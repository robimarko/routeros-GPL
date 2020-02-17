/*
 * Marvell Orion SPI controller driver
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
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <asm/unaligned.h>
#include <linux/gpio.h>

#define DRIVER_NAME			"orion_spi"

#define ORION_NUM_CHIPSELECTS		32 /* only one slave is supported*/
#define ORION_SPI_WAIT_RDY_MAX_LOOP	2000 /* in usec */

#define ORION_SPI_IF_CTRL_REG		0x00
#define ORION_SPI_IF_CONFIG_REG		0x04
#define ORION_SPI_DATA_OUT_REG		0x08
#define ORION_SPI_DATA_IN_REG		0x0c
#define ORION_SPI_INT_CAUSE_REG		0x10

#define ORION_SPI_MODE_CPOL		(1 << 11)
#define ORION_SPI_MODE_CPHA		(1 << 12)
#define ORION_SPI_IF_8_16_BIT_MODE	(1 << 5)
#define ORION_SPI_CLK_PRESCALE_MASK	0x1F
#define ARMADA_SPI_CLK_PRESCALE_MASK	0xDF
#define ORION_SPI_MODE_MASK		(ORION_SPI_MODE_CPOL | \
					 ORION_SPI_MODE_CPHA)
#define ORION_SPI_CS_MASK	0x1C
#define ORION_SPI_CS_SHIFT	2
#define ORION_SPI_CS(cs)	((cs << ORION_SPI_CS_SHIFT) & \
					ORION_SPI_CS_MASK)

enum orion_spi_type {
	ORION_SPI,
	ARMADA_SPI,
};

struct orion_spi_dev {
	enum orion_spi_type	typ;
	unsigned int		min_divisor;
	unsigned int		max_divisor;
	u32			prescale_mask;
	u32			enable_clock_fix;
};

struct orion_spi {
	struct work_struct	work;

	/* Lock access to transfer list.	*/
	spinlock_t		lock;

	struct list_head	msg_queue;
	struct spi_master	*master;
	void __iomem		*base;
	struct clk              *clk;
	unsigned int		max_speed;
	unsigned int		min_speed;
	int			ext_cs_sel;
	int			current_cs;
	const struct orion_spi_dev *devdata;
};

static struct workqueue_struct *orion_spi_wq;

static inline void __iomem *spi_reg(struct orion_spi *orion_spi, u32 reg)
{
	return orion_spi->base + reg;
}

static inline void
orion_spi_setbits(struct orion_spi *orion_spi, u32 reg, u32 mask)
{
	void __iomem *reg_addr = spi_reg(orion_spi, reg);
	u32 val;

	val = readl(reg_addr);
	val |= mask;
	writel(val, reg_addr);
}

static inline void
orion_spi_clrbits(struct orion_spi *orion_spi, u32 reg, u32 mask)
{
	void __iomem *reg_addr = spi_reg(orion_spi, reg);
	u32 val;

	val = readl(reg_addr);
	val &= ~mask;
	writel(val, reg_addr);
}

static int orion_spi_baudrate_set(struct spi_device *spi, unsigned int speed)
{
	u32 tclk_hz;
	u32 rate;
	u32 prescale;
	u32 reg;
	struct orion_spi *orion_spi;
	const struct orion_spi_dev *devdata;

	orion_spi = spi_master_get_devdata(spi->master);
	devdata = orion_spi->devdata;

	tclk_hz = clk_get_rate(orion_spi->clk);

	if (devdata->typ == ARMADA_SPI) {
		/*
		 * Given the core_clk (tclk_hz) and the target rate (speed) we
		 * determine the best values for SPR (in [0 .. 15]) and SPPR (in
		 * [0..7]) such that
		 *
		 * 	core_clk / (SPR * 2 ** SPPR)
		 *
		 * is as big as possible but not bigger than speed.
		 */

		/* best integer divider: */
		unsigned divider = DIV_ROUND_UP(tclk_hz, speed);
		unsigned spr, sppr;

		if (divider < 16) {
			/* This is the easy case, divider is less than 16 */
			spr = divider;
			sppr = 0;

		} else {
			unsigned two_pow_sppr;
			/*
			 * Find the highest bit set in divider. This and the
			 * three next bits define SPR (apart from rounding).
			 * SPPR is then the number of zero bits that must be
			 * appended:
			 */
			sppr = fls(divider) - 4;

			/*
			 * As SPR only has 4 bits, we have to round divider up
			 * to the next multiple of 2 ** sppr.
			 */
			two_pow_sppr = 1 << sppr;
			divider = (divider + two_pow_sppr - 1) & -two_pow_sppr;

			/*
			 * recalculate sppr as rounding up divider might have
			 * increased it enough to change the position of the
			 * highest set bit. In this case the bit that now
			 * doesn't make it into SPR is 0, so there is no need to
			 * round again.
			 */
			sppr = fls(divider) - 4;
			spr = divider >> sppr;

			/*
			 * Now do range checking. SPR is constructed to have a
			 * width of 4 bits, so this is fine for sure. So we
			 * still need to check for sppr to fit into 3 bits:
			 */
			if (sppr > 7)
				return -EINVAL;
		}

		prescale = ((sppr & 0x6) << 5) | ((sppr & 0x1) << 4) | spr;
	} else {
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
	}

	reg = readl(spi_reg(orion_spi, ORION_SPI_IF_CONFIG_REG));
	reg = ((reg & ~devdata->prescale_mask) | prescale);
	writel(reg, spi_reg(orion_spi, ORION_SPI_IF_CONFIG_REG));

	return 0;
}

static void
orion_spi_mode_set(struct spi_device *spi)
{
	u32 reg;
	struct orion_spi *orion_spi;

	orion_spi = spi_master_get_devdata(spi->master);

	reg = readl(spi_reg(orion_spi, ORION_SPI_IF_CONFIG_REG));
	reg &= ~ORION_SPI_MODE_MASK;
	if (spi->mode & SPI_CPOL)
		reg |= ORION_SPI_MODE_CPOL;
	if (spi->mode & SPI_CPHA)
		reg |= ORION_SPI_MODE_CPHA;
	writel(reg, spi_reg(orion_spi, ORION_SPI_IF_CONFIG_REG));
}

/*
 * called only when no transfer is active on the bus
 */
static int
orion_spi_setup_transfer(struct spi_device *spi, struct spi_transfer *t)
{
	struct orion_spi *orion_spi;
	unsigned int speed = spi->max_speed_hz;
	unsigned int bits_per_word = spi->bits_per_word;
	int	rc;

	orion_spi = spi_master_get_devdata(spi->master);

	if ((t != NULL) && t->speed_hz)
		speed = t->speed_hz;

	if ((t != NULL) && t->bits_per_word)
		bits_per_word = t->bits_per_word;

	orion_spi_mode_set(spi);

	rc = orion_spi_baudrate_set(spi, speed);
	if (rc)
		return rc;

	if (bits_per_word == 16)
		orion_spi_setbits(orion_spi, ORION_SPI_IF_CONFIG_REG,
				  ORION_SPI_IF_8_16_BIT_MODE);
	else
		orion_spi_clrbits(orion_spi, ORION_SPI_IF_CONFIG_REG,
				  ORION_SPI_IF_8_16_BIT_MODE);

	return 0;
}

static void orion_set_cs (struct orion_spi *orion_spi, unsigned cs, bool enable) {
	if (enable) {
		orion_spi->current_cs = cs;
	}

	// XXX: We need to assert CS from the SPI interface, otherwise ready bit
	// won't be set - transaction will timeout...
	// This means we cannot use kernel SPI "cs_gpios"
	if (cs > 1) {
		if (orion_spi->ext_cs_sel >=0) {
			gpio_set_value(orion_spi->ext_cs_sel, !enable);
		}
		gpio_set_value(cs + 3, !enable);
		cs = 2;
		udelay(2);
	}
	else {
		if (enable) {
			if (orion_spi->ext_cs_sel >=0) {
				gpio_set_value(orion_spi->ext_cs_sel, 1);
			}
		}
	}

	orion_spi_clrbits(orion_spi, ORION_SPI_IF_CTRL_REG, ORION_SPI_CS_MASK);
	orion_spi_setbits(orion_spi, ORION_SPI_IF_CTRL_REG, ORION_SPI_CS(cs));

	/* Chip select logic is inverted from spi_set_cs */
	if (enable)
		orion_spi_setbits(orion_spi, ORION_SPI_IF_CTRL_REG, 0x1);
	else
		orion_spi_clrbits(orion_spi, ORION_SPI_IF_CTRL_REG, 0x1);
	if (!enable) {
		orion_spi->current_cs = -1;
	}
}

static void orion_spi_set_cs(struct spi_device *spi, bool enable)
{
	struct orion_spi *orion_spi = spi_master_get_devdata(spi->master);
	unsigned cs = spi->chip_select;
	if (orion_spi->current_cs != -1) {
	    if (orion_spi->current_cs != cs) {
		orion_set_cs(orion_spi, orion_spi->current_cs, 0);
	    }
	}
	if (enable && (orion_spi->current_cs == cs)) {
	    // XXX: cs already set, no need to change it
	    return;
	}

	if (!enable && (orion_spi->current_cs == -1)) {
	    // XXX: cs already cleared, no need to change it
	    return;
	}
	orion_set_cs(orion_spi, cs, enable);

}

static inline int orion_spi_wait_till_ready(struct orion_spi *orion_spi)
{
	int i;

	for (i = 0; i < ORION_SPI_WAIT_RDY_MAX_LOOP; i++) {
		if (readl(spi_reg(orion_spi, ORION_SPI_INT_CAUSE_REG)))
			return 1;

			udelay(1);
	}

	return -1;
}

static inline int
orion_spi_write_read_8bit(struct spi_device *spi,
			  const u8 **tx_buf, u8 **rx_buf)
{
	void __iomem *tx_reg, *rx_reg, *int_reg;
	struct orion_spi *orion_spi;

	orion_spi = spi_master_get_devdata(spi->master);
	tx_reg = spi_reg(orion_spi, ORION_SPI_DATA_OUT_REG);
	rx_reg = spi_reg(orion_spi, ORION_SPI_DATA_IN_REG);
	int_reg = spi_reg(orion_spi, ORION_SPI_INT_CAUSE_REG);

	/* clear the interrupt cause register */
	writel(0x0, int_reg);

	if (tx_buf && *tx_buf)
		writel(*(*tx_buf)++, tx_reg);
	else
		writel(0, tx_reg);

	if (orion_spi_wait_till_ready(orion_spi) < 0) {
		dev_err(&spi->dev, "TXS timed out\n");
		return -1;
	}

	if (rx_buf && *rx_buf)
		*(*rx_buf)++ = readl(rx_reg);

	return 1;
}

static inline int
orion_spi_write_read_16bit(struct spi_device *spi,
			   const u16 **tx_buf, u16 **rx_buf)
{
	void __iomem *tx_reg, *rx_reg, *int_reg;
	struct orion_spi *orion_spi;

	orion_spi = spi_master_get_devdata(spi->master);
	tx_reg = spi_reg(orion_spi, ORION_SPI_DATA_OUT_REG);
	rx_reg = spi_reg(orion_spi, ORION_SPI_DATA_IN_REG);
	int_reg = spi_reg(orion_spi, ORION_SPI_INT_CAUSE_REG);

	/* clear the interrupt cause register */
	writel(0x0, int_reg);

	if (tx_buf && *tx_buf)
		writel(__cpu_to_le16(get_unaligned((*tx_buf)++)), tx_reg);
	else
		writel(0, tx_reg);

	if (orion_spi_wait_till_ready(orion_spi) < 0) {
		dev_err(&spi->dev, "TXS timed out\n");
		return -1;
	}

	if (rx_buf && *rx_buf)
		put_unaligned(__le16_to_cpu(readl(rx_reg)), (*rx_buf)++);

	return 1;
}

static unsigned int
orion_spi_write_read(struct spi_device *spi, struct spi_transfer *xfer)
{
	struct orion_spi *orion_spi;
	unsigned int count;
	int word_len;

	orion_spi = spi_master_get_devdata(spi->master);
	word_len = spi->bits_per_word;
	count = xfer->len;

	if (word_len == 8) {
		const u8 *tx = xfer->tx_buf;
		u8 *rx = xfer->rx_buf;

		do {
			if (orion_spi_write_read_8bit(spi, &tx, &rx) < 0)
				goto out;
			count--;
		} while (count);
	} else if (word_len == 16) {
		const u16 *tx = xfer->tx_buf;
		u16 *rx = xfer->rx_buf;

		do {
			if (orion_spi_write_read_16bit(spi, &tx, &rx) < 0)
				goto out;
			count -= 2;
		} while (count);
	}

out:
	return xfer->len - count;
}


static void orion_spi_work(struct work_struct *work)
{
	struct orion_spi *orion_spi =
		container_of(work, struct orion_spi, work);

	spin_lock_irq(&orion_spi->lock);
	while (!list_empty(&orion_spi->msg_queue)) {
		struct spi_message *m;
		struct spi_device *spi;
		struct spi_transfer *t = NULL;
		int par_override = 0;
		int status = 0;

		m = container_of(orion_spi->msg_queue.next, struct spi_message,
				 queue);

		list_del_init(&m->queue);
		spin_unlock_irq(&orion_spi->lock);

		spi = m->spi;

		/* Load defaults */
		status = orion_spi_setup_transfer(spi, NULL);

		if (status < 0)
			goto msg_done;

		list_for_each_entry(t, &m->transfers, transfer_list) {
			if (par_override || t->speed_hz || t->bits_per_word) {
				par_override = 1;
				status = orion_spi_setup_transfer(spi, t);
				if (status < 0)
					break;
				if (!t->speed_hz && !t->bits_per_word)
					par_override = 0;
			}

			orion_spi_set_cs(spi, 1);

			if (t->len)
				m->actual_length +=
					orion_spi_write_read(spi, t);

			if (t->delay_usecs)
				udelay(t->delay_usecs);

			if (t->cs_change) {
				if (!list_is_last(&t->transfer_list, &m->transfers)) {
					orion_spi_set_cs(spi, 0);
					ndelay(100);
					orion_spi_set_cs(spi, 1);
				}
			}
			else {
				if (list_is_last(&t->transfer_list, &m->transfers)) {
					orion_spi_set_cs(spi, 0);
				}
			}
		}

msg_done:
		m->status = status;
		m->complete(m->context);

		spin_lock_irq(&orion_spi->lock);
	}

	spin_unlock_irq(&orion_spi->lock);
}

static int __init orion_spi_reset(struct orion_spi *orion_spi)
{
	/* Verify that the CS is deasserted */
	orion_spi_clrbits(orion_spi, ORION_SPI_IF_CTRL_REG, 0x1);

	return 0;
}

static int orion_spi_setup(struct spi_device *spi)
{
	struct orion_spi *orion_spi;

	orion_spi = spi_master_get_devdata(spi->master);

	/* Fix ac timing if required.   */
	if (orion_spi->devdata->enable_clock_fix)
		orion_spi_setbits(orion_spi, ORION_SPI_IF_CONFIG_REG,
				  (1 << 14));

	if ((spi->max_speed_hz == 0)
			|| (spi->max_speed_hz > orion_spi->max_speed))
		spi->max_speed_hz = orion_spi->max_speed;

	if (spi->max_speed_hz < orion_spi->min_speed) {
		dev_err(&spi->dev, "setup: requested speed too low %d Hz\n",
			spi->max_speed_hz);
		return -EINVAL;
	}

	/*
	 * baudrate & width will be set orion_spi_setup_transfer
	 */
	return 0;
}

static int orion_spi_transfer(struct spi_device *spi, struct spi_message *m)
{
	struct orion_spi *orion_spi;
	struct spi_transfer *t = NULL;
	unsigned long flags;

	m->actual_length = 0;
	m->status = 0;

	/* reject invalid messages and transfers */
	if (list_empty(&m->transfers) || !m->complete)
		return -EINVAL;

	orion_spi = spi_master_get_devdata(spi->master);

	list_for_each_entry(t, &m->transfers, transfer_list) {
		unsigned int bits_per_word = spi->bits_per_word;

		if (t->tx_buf == NULL && t->rx_buf == NULL && t->len) {
			dev_err(&spi->dev,
				"message rejected : "
				"invalid transfer data buffers\n");
			goto msg_rejected;
		}

		if (t->bits_per_word)
			bits_per_word = t->bits_per_word;

		if ((bits_per_word != 8) && (bits_per_word != 16)) {
			dev_err(&spi->dev,
				"message rejected : "
				"invalid transfer bits_per_word (%d bits)\n",
				bits_per_word);
			goto msg_rejected;
		}
		/*make sure buffer length is even when working in 16 bit mode*/
		if ((t->bits_per_word == 16) && (t->len & 1)) {
			dev_err(&spi->dev,
				"message rejected : "
				"odd data length (%d) while in 16 bit mode\n",
				t->len);
			goto msg_rejected;
		}

		if (t->speed_hz && t->speed_hz < orion_spi->min_speed) {
			dev_err(&spi->dev,
				"message rejected : "
				"device min speed (%d Hz) exceeds "
				"required transfer speed (%d Hz)\n",
				orion_spi->min_speed, t->speed_hz);
			goto msg_rejected;
		}
	}


	spin_lock_irqsave(&orion_spi->lock, flags);
	list_add_tail(&m->queue, &orion_spi->msg_queue);
	queue_work(orion_spi_wq, &orion_spi->work);
	spin_unlock_irqrestore(&orion_spi->lock, flags);

	return 0;
msg_rejected:
	/* Message rejected and not queued */
	m->status = -EINVAL;
	if (m->complete)
		m->complete(m->context);
	return -EINVAL;
}

static struct orion_spi_dev orion_spi_dev_data = {
	.typ = ORION_SPI,
	.min_divisor = 4,
	.max_divisor = 30,
	.prescale_mask = ORION_SPI_CLK_PRESCALE_MASK,
};

static struct orion_spi_dev armada_spi_dev_data = {
	.typ = ARMADA_SPI,
	.min_divisor = 4,
	.max_divisor = 1920,
	.prescale_mask = ARMADA_SPI_CLK_PRESCALE_MASK,
};

static const struct of_device_id orion_spi_of_match_table[] = {
	{ .compatible = "marvell,orion-spi", .data = &orion_spi_dev_data, },
	{ .compatible = "marvell,armada-370-spi", .data = &armada_spi_dev_data, },
	{}
};
MODULE_DEVICE_TABLE(of, orion_spi_of_match_table);

static int orion_spi_probe(struct platform_device *pdev)
{
	const struct of_device_id *of_id;
	struct orion_spi_dev *devdata;
	struct spi_master *master;
	struct orion_spi *spi;
	struct resource *r;
	unsigned long tclk_hz;
	int status = 0;

	orion_spi_wq = create_singlethread_workqueue(DRIVER_NAME);
	if (orion_spi_wq == NULL)
		return -ENOMEM;

	master = spi_alloc_master(&pdev->dev, sizeof *spi);
	if (master == NULL) {
		dev_dbg(&pdev->dev, "master allocation failed\n");
		destroy_workqueue(orion_spi_wq);
		return -ENOMEM;
	}

	if (pdev->id != -1)
		master->bus_num = pdev->id;
	if (pdev->dev.of_node) {
		u32 cell_index;

		if (!of_property_read_u32(pdev->dev.of_node, "cell-index",
					  &cell_index))
			master->bus_num = cell_index;
	}

	/* we support only mode 0, and no options */
	master->mode_bits = SPI_CPHA | SPI_CPOL;

	master->setup = orion_spi_setup;
	master->transfer = orion_spi_transfer;
	master->num_chipselect = ORION_NUM_CHIPSELECTS;

	dev_set_drvdata(&pdev->dev, master);

	spi = spi_master_get_devdata(master);
	spi->master = master;

	of_id = of_match_device(orion_spi_of_match_table, &pdev->dev);
	devdata = (of_id) ? of_id->data : &orion_spi_dev_data;
	spi->devdata = devdata;

	spi->clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(spi->clk)) {
		status = PTR_ERR(spi->clk);
		goto out;
	}

	status = clk_prepare_enable(spi->clk);
	if (status)
		goto out;

	tclk_hz = clk_get_rate(spi->clk);
	spi->max_speed = DIV_ROUND_UP(tclk_hz, devdata->min_divisor);
	spi->min_speed = DIV_ROUND_UP(tclk_hz, devdata->max_divisor);
	spi->current_cs = -1;
	spi->ext_cs_sel = -1;
	if (pdev->dev.of_node) {
		u32 ext_cs_sel;
		if (!of_property_read_u32(pdev->dev.of_node, "ext-cs-sel",
					  &ext_cs_sel)) {
			spi->ext_cs_sel = ext_cs_sel;
		}
	}

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	spi->base = devm_request_and_ioremap(&pdev->dev, r);
	if (!spi->base) {
		status = -EBUSY;
		goto out_rel_clk;
	}

	INIT_WORK(&spi->work, orion_spi_work);

	spin_lock_init(&spi->lock);
	INIT_LIST_HEAD(&spi->msg_queue);

	status = orion_spi_reset(spi);
	if (status < 0)
		goto out_rel_clk;

	master->dev.of_node = pdev->dev.of_node;
	status = spi_register_master(master);
	if (status < 0)
		goto out_rel_clk;

	return status;

out_rel_clk:
	clk_disable_unprepare(spi->clk);
out:
	spi_master_put(master);
	destroy_workqueue(orion_spi_wq);
	return status;
}


static int orion_spi_remove(struct platform_device *pdev)
{
	struct spi_master *master = dev_get_drvdata(&pdev->dev);
	struct orion_spi *spi = spi_master_get_devdata(master);

	clk_disable_unprepare(spi->clk);

	cancel_work_sync(&spi->work);

	spi_unregister_master(master);

	flush_workqueue(orion_spi_wq);
	destroy_workqueue(orion_spi_wq);

	return 0;
}

MODULE_ALIAS("platform:" DRIVER_NAME);

static struct platform_driver orion_spi_driver = {
	.driver = {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(orion_spi_of_match_table),
	},
	.probe		= orion_spi_probe,
	.remove		= orion_spi_remove,
};

module_platform_driver(orion_spi_driver);

MODULE_DESCRIPTION("Orion SPI driver");
MODULE_AUTHOR("Shadi Ammouri <shadi@marvell.com>");
MODULE_LICENSE("GPL");
