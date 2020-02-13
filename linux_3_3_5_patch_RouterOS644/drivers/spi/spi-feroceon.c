/*
 * feroceon_spi.c -- Marvell Orion SPI controller driver
 *
 * Author: Shadi Ammouri <shadi@marvell.com>
 * Copyright (C) 2007-2008 Marvell Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/spi/spi.h>
#include <linux/spi/orion_spi.h>
#include <asm/unaligned.h>
#include <linux/module.h>

#define DRIVER_NAME			"feroceon_spi"

#define ORION_NUM_CHIPSELECTS		3 /* only one slave is supported*/
#define ORION_SPI_WAIT_RDY_MAX_LOOP	2000 /* in usec */

#define ORION_SPI_IF_CTRL_REG		0x00
#define ORION_SPI_IF_CONFIG_REG		0x04
#define ORION_SPI_DATA_OUT_REG		0x08
#define ORION_SPI_DATA_IN_REG		0x0c
#define ORION_SPI_INT_CAUSE_REG		0x10

#define ORION_SPI_IF_8_16_BIT_MODE	(1 << 5)
#define ORION_SPI_CLK_PRESCALE_MASK	0x1F

struct feroceon_spi {
	struct work_struct	work;

	/* Lock access to transfer list.	*/
	spinlock_t		lock;

	struct list_head	msg_queue;
	struct spi_master	*master;
	void __iomem		*base;
	unsigned int		max_speed;
	unsigned int		min_speed;
	struct orion_spi_info	*spi_info;
};

static struct workqueue_struct *feroceon_spi_wq;

static inline void __iomem *spi_reg(struct feroceon_spi *feroceon_spi, u32 reg)
{
	return feroceon_spi->base + reg;
}

static inline void
feroceon_spi_setbits(struct feroceon_spi *feroceon_spi, u32 reg, u32 mask)
{
	void __iomem *reg_addr = spi_reg(feroceon_spi, reg);
	u32 val;

	val = readl(reg_addr);
	val |= mask;
	writel(val, reg_addr);
}

static inline void
feroceon_spi_clrbits(struct feroceon_spi *feroceon_spi, u32 reg, u32 mask)
{
	void __iomem *reg_addr = spi_reg(feroceon_spi, reg);
	u32 val;

	val = readl(reg_addr);
	val &= ~mask;
	writel(val, reg_addr);
}

static int feroceon_spi_set_transfer_size(struct feroceon_spi *feroceon_spi, int size)
{
	if (size == 16) {
		feroceon_spi_setbits(feroceon_spi, ORION_SPI_IF_CONFIG_REG,
				  ORION_SPI_IF_8_16_BIT_MODE);
	} else if (size == 8) {
		feroceon_spi_clrbits(feroceon_spi, ORION_SPI_IF_CONFIG_REG,
				  ORION_SPI_IF_8_16_BIT_MODE);
	} else {
		pr_debug("Bad bits per word value %d (only 8 or 16 are "
			 "allowed).\n", size);
		return -EINVAL;
	}

	return 0;
}

static int feroceon_spi_baudrate_set(struct spi_device *spi, unsigned int speed)
{
	u32 tclk_hz;
	u32 rate;
	u32 prescale;
	u32 reg;
	struct feroceon_spi *feroceon_spi;

	feroceon_spi = spi_master_get_devdata(spi->master);

	tclk_hz = feroceon_spi->spi_info->tclk;

	/*
	 * the supported rates are: 4,6,8...30
	 * round up as we look for equal or less speed
	 */
	rate = DIV_ROUND_UP(tclk_hz, speed);
	rate = roundup(rate, 2);

	/* check if requested speed is too small */
	if (rate > 30)
		return -EINVAL;

	if (rate < 4)
		rate = 4;

	/* Convert the rate to SPI clock divisor value.	*/
	prescale = 0x10 + rate/2;

	reg = readl(spi_reg(feroceon_spi, ORION_SPI_IF_CONFIG_REG));
	reg = ((reg & ~ORION_SPI_CLK_PRESCALE_MASK) | prescale);
	writel(reg, spi_reg(feroceon_spi, ORION_SPI_IF_CONFIG_REG));

	return 0;
}

/*
 * called only when no transfer is active on the bus
 */
static int
feroceon_spi_setup_transfer(struct spi_device *spi, struct spi_transfer *t)
{
	struct feroceon_spi *feroceon_spi;
	unsigned int speed = spi->max_speed_hz;
	unsigned int bits_per_word = spi->bits_per_word;
	int	rc;

	feroceon_spi = spi_master_get_devdata(spi->master);

	if ((t != NULL) && t->speed_hz)
		speed = t->speed_hz;

	if ((t != NULL) && t->bits_per_word)
		bits_per_word = t->bits_per_word;

	rc = feroceon_spi_baudrate_set(spi, speed);
	if (rc)
		return rc;

	return feroceon_spi_set_transfer_size(feroceon_spi, bits_per_word);
}

static void feroceon_spi_set_cs(struct feroceon_spi *feroceon_spi, int enable)
{
	if (enable)
		feroceon_spi_setbits(feroceon_spi, ORION_SPI_IF_CTRL_REG, 0x1);
	else
		feroceon_spi_clrbits(feroceon_spi, ORION_SPI_IF_CTRL_REG, 0x1);
}

static inline int feroceon_spi_wait_till_ready(struct feroceon_spi *feroceon_spi)
{
	int i;

	for (i = 0; i < ORION_SPI_WAIT_RDY_MAX_LOOP; i++) {
		if (readl(spi_reg(feroceon_spi, ORION_SPI_INT_CAUSE_REG)))
			return 1;
		else
			udelay(1);
	}

	return -1;
}

static inline int
feroceon_spi_write_read_8bit(struct spi_device *spi,
			  const u8 **tx_buf, u8 **rx_buf)
{
	void __iomem *tx_reg, *rx_reg, *int_reg;
	struct feroceon_spi *feroceon_spi;

	feroceon_spi = spi_master_get_devdata(spi->master);
	tx_reg = spi_reg(feroceon_spi, ORION_SPI_DATA_OUT_REG);
	rx_reg = spi_reg(feroceon_spi, ORION_SPI_DATA_IN_REG);
	int_reg = spi_reg(feroceon_spi, ORION_SPI_INT_CAUSE_REG);

	/* clear the interrupt cause register */
	writel(0x0, int_reg);

	if (tx_buf && *tx_buf)
		writel(*(*tx_buf)++, tx_reg);
	else
		writel(0, tx_reg);

	if (feroceon_spi_wait_till_ready(feroceon_spi) < 0) {
		dev_err(&spi->dev, "TXS timed out\n");
		return -1;
	}

	if (rx_buf && *rx_buf)
		*(*rx_buf)++ = readl(rx_reg);

	return 1;
}

static inline int
feroceon_spi_write_read_16bit(struct spi_device *spi,
			   const u16 **tx_buf, u16 **rx_buf)
{
	void __iomem *tx_reg, *rx_reg, *int_reg;
	struct feroceon_spi *feroceon_spi;

	feroceon_spi = spi_master_get_devdata(spi->master);
	tx_reg = spi_reg(feroceon_spi, ORION_SPI_DATA_OUT_REG);
	rx_reg = spi_reg(feroceon_spi, ORION_SPI_DATA_IN_REG);
	int_reg = spi_reg(feroceon_spi, ORION_SPI_INT_CAUSE_REG);

	/* clear the interrupt cause register */
	writel(0x0, int_reg);

	if (tx_buf && *tx_buf)
		writel(__cpu_to_le16(get_unaligned((*tx_buf)++)), tx_reg);
	else
		writel(0, tx_reg);

	if (feroceon_spi_wait_till_ready(feroceon_spi) < 0) {
		dev_err(&spi->dev, "TXS timed out\n");
		return -1;
	}

	if (rx_buf && *rx_buf)
		put_unaligned(__le16_to_cpu(readl(rx_reg)), (*rx_buf)++);

	return 1;
}

static unsigned int
feroceon_spi_write_read(struct spi_device *spi, struct spi_transfer *xfer)
{
	struct feroceon_spi *feroceon_spi;
	unsigned int count;
	int word_len;

	feroceon_spi = spi_master_get_devdata(spi->master);
	word_len = spi->bits_per_word;
	count = xfer->len;

	if (word_len == 8) {
		const u8 *tx = xfer->tx_buf;
		u8 *rx = xfer->rx_buf;

		do {
			if (feroceon_spi_write_read_8bit(spi, &tx, &rx) < 0)
				goto out;
			count--;
		} while (count);
	} else if (word_len == 16) {
		const u16 *tx = xfer->tx_buf;
		u16 *rx = xfer->rx_buf;

		do {
			if (feroceon_spi_write_read_16bit(spi, &tx, &rx) < 0)
				goto out;
			count -= 2;
		} while (count);
	}

out:
	return xfer->len - count;
}


static void feroceon_spi_work(struct work_struct *work)
{
	struct feroceon_spi *feroceon_spi =
		container_of(work, struct feroceon_spi, work);

	spin_lock_irq(&feroceon_spi->lock);
	while (!list_empty(&feroceon_spi->msg_queue)) {
		struct spi_message *m;
		struct spi_device *spi;
		struct spi_transfer *t = NULL;
		int par_override = 0;
		int status = 0;
		int cs_active = 0;

		m = container_of(feroceon_spi->msg_queue.next, struct spi_message,
				 queue);

		list_del_init(&m->queue);
		spin_unlock_irq(&feroceon_spi->lock);

		spi = m->spi;

		/* Load defaults */
		status = feroceon_spi_setup_transfer(spi, NULL);

		if (status < 0)
			goto msg_done;

		list_for_each_entry(t, &m->transfers, transfer_list) {
			if (par_override || t->speed_hz || t->bits_per_word) {
				par_override = 1;
				status = feroceon_spi_setup_transfer(spi, t);
				if (status < 0)
					break;
				if (!t->speed_hz && !t->bits_per_word)
					par_override = 0;
			}

			if (!cs_active) {
				feroceon_spi_set_cs(feroceon_spi, 1);
				cs_active = 1;
			}

			if (t->len)
				m->actual_length +=
					feroceon_spi_write_read(spi, t);

			if (t->delay_usecs)
				udelay(t->delay_usecs);

			if (t->cs_change) {
				feroceon_spi_set_cs(feroceon_spi, 0);
				cs_active = 0;
			}
		}

msg_done:
		if (cs_active)
			feroceon_spi_set_cs(feroceon_spi, 0);

		m->status = status;
		m->complete(m->context);

		spin_lock_irq(&feroceon_spi->lock);
	}

	spin_unlock_irq(&feroceon_spi->lock);
}

static int __init feroceon_spi_reset(struct feroceon_spi *feroceon_spi)
{
	/* Verify that the CS is deasserted */
	feroceon_spi_set_cs(feroceon_spi, 0);

	return 0;
}

static int feroceon_spi_setup(struct spi_device *spi)
{
	struct feroceon_spi *feroceon_spi;

	feroceon_spi = spi_master_get_devdata(spi->master);

	/* Fix ac timing if required.   */
	if (feroceon_spi->spi_info->enable_clock_fix)
		feroceon_spi_setbits(feroceon_spi, ORION_SPI_IF_CONFIG_REG,
				  (1 << 14));

	if ((spi->max_speed_hz == 0)
			|| (spi->max_speed_hz > feroceon_spi->max_speed))
		spi->max_speed_hz = feroceon_spi->max_speed;

	if (spi->max_speed_hz < feroceon_spi->min_speed) {
		dev_err(&spi->dev, "setup: requested speed too low %d Hz\n",
			spi->max_speed_hz);
		return -EINVAL;
	}

	/*
	 * baudrate & width will be set feroceon_spi_setup_transfer
	 */
	return 0;
}

static int feroceon_spi_transfer(struct spi_device *spi, struct spi_message *m)
{
	struct feroceon_spi *feroceon_spi;
	struct spi_transfer *t = NULL;
	unsigned long flags;

	m->actual_length = 0;
	m->status = 0;

	/* reject invalid messages and transfers */
	if (list_empty(&m->transfers) || !m->complete)
		return -EINVAL;

	feroceon_spi = spi_master_get_devdata(spi->master);

	list_for_each_entry(t, &m->transfers, transfer_list) {
		unsigned int bits_per_word = spi->bits_per_word;

		if (t->tx_buf == NULL && t->rx_buf == NULL && t->len) {
			dev_err(&spi->dev,
				"message rejected : "
				"invalid transfer data buffers\n");
			goto msg_rejected;
		}

		if ((t != NULL) && t->bits_per_word)
			bits_per_word = t->bits_per_word;

		if ((bits_per_word != 8) && (bits_per_word != 16)) {
			dev_err(&spi->dev,
				"message rejected : "
				"invalid transfer bits_per_word (%d bits)\n",
				bits_per_word);
			goto msg_rejected;
		}
		/*make sure buffer length is even when working in 16 bit mode*/
		if ((t != NULL) && (t->bits_per_word == 16) && (t->len & 1)) {
			dev_err(&spi->dev,
				"message rejected : "
				"odd data length (%d) while in 16 bit mode\n",
				t->len);
			goto msg_rejected;
		}

		if (t->speed_hz && t->speed_hz < feroceon_spi->min_speed) {
			dev_err(&spi->dev,
				"message rejected : "
				"device min speed (%d Hz) exceeds "
				"required transfer speed (%d Hz)\n",
				feroceon_spi->min_speed, t->speed_hz);
			goto msg_rejected;
		}
	}


	spin_lock_irqsave(&feroceon_spi->lock, flags);
	list_add_tail(&m->queue, &feroceon_spi->msg_queue);
	queue_work(feroceon_spi_wq, &feroceon_spi->work);
	spin_unlock_irqrestore(&feroceon_spi->lock, flags);

	return 0;
msg_rejected:
	/* Message rejected and not queued */
	m->status = -EINVAL;
	if (m->complete)
		m->complete(m->context);
	return -EINVAL;
}

static int __init feroceon_spi_probe(struct platform_device *pdev)
{
	struct spi_master *master;
	struct feroceon_spi *spi;
	struct resource *r;
	struct orion_spi_info *spi_info;
	int status = 0;

	spi_info = pdev->dev.platform_data;

	master = spi_alloc_master(&pdev->dev, sizeof *spi);
	if (master == NULL) {
		dev_dbg(&pdev->dev, "master allocation failed\n");
		return -ENOMEM;
	}

	if (pdev->id != -1)
		master->bus_num = pdev->id;

	/* we support only mode 0, and no options */
	master->mode_bits = 0;

	master->setup = feroceon_spi_setup;
	master->transfer = feroceon_spi_transfer;
	master->num_chipselect = ORION_NUM_CHIPSELECTS;

	dev_set_drvdata(&pdev->dev, master);

	spi = spi_master_get_devdata(master);
	spi->master = master;
	spi->spi_info = spi_info;

	spi->max_speed = DIV_ROUND_UP(spi_info->tclk, 4);
	spi->min_speed = DIV_ROUND_UP(spi_info->tclk, 30);

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (r == NULL) {
		status = -ENODEV;
		goto out;
	}

	if (!request_mem_region(r->start, (r->end - r->start) + 1,
				dev_name(&pdev->dev))) {
		status = -EBUSY;
		goto out;
	}
	spi->base = ioremap(r->start, SZ_1K);

	INIT_WORK(&spi->work, feroceon_spi_work);

	spin_lock_init(&spi->lock);
	INIT_LIST_HEAD(&spi->msg_queue);

	if (feroceon_spi_reset(spi) < 0)
		goto out_rel_mem;

	status = spi_register_master(master);
	if (status < 0)
		goto out_rel_mem;

	return status;

out_rel_mem:
	release_mem_region(r->start, (r->end - r->start) + 1);

out:
	spi_master_put(master);
	return status;
}


static int __exit feroceon_spi_remove(struct platform_device *pdev)
{
	struct spi_master *master;
	struct feroceon_spi *spi;
	struct resource *r;

	master = dev_get_drvdata(&pdev->dev);
	spi = spi_master_get_devdata(master);

	cancel_work_sync(&spi->work);

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(r->start, (r->end - r->start) + 1);

	spi_unregister_master(master);

	return 0;
}

MODULE_ALIAS("platform:" DRIVER_NAME);

static struct platform_driver feroceon_spi_driver = {
	.driver = {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
	.probe		= feroceon_spi_probe,
	.remove		= feroceon_spi_remove,
};
module_platform_driver(feroceon_spi_driver);

static int __init feroceon_spi_init(void)
{
	feroceon_spi_wq = create_singlethread_workqueue(
				feroceon_spi_driver.driver.name);
	if (feroceon_spi_wq == NULL)
		return -ENOMEM;

	return platform_driver_probe(&feroceon_spi_driver, feroceon_spi_probe);
}
module_init(feroceon_spi_init);

static void __exit feroceon_spi_exit(void)
{
	flush_workqueue(feroceon_spi_wq);
	platform_driver_unregister(&feroceon_spi_driver);

	destroy_workqueue(feroceon_spi_wq);
}
module_exit(feroceon_spi_exit);

MODULE_DESCRIPTION("Feroceon SPI driver");
MODULE_AUTHOR("Shadi Ammouri <shadi@marvell.com>");
MODULE_LICENSE("GPL");
