#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/hardirq.h>
#include <asm/rb/boards.h>
#include <asm/rb/rb400.h>

#include <linux/spi/spi.h>

#define RB900_DEBUG 0
#define DDR_PLL_CONFIG		0x18050004
#define CPU_DDR_CLOCK_CONTROL	0x18050008
#define RST_BOOTSTRAP		0x180600b0

#define SPI_BASE	0x1F000000
#define   SPI_FUNC	  0x000000
#define     SBIT_FUNC_GPIO	0x00000001
#define   SPI_CTRL	  0x000004
#define	    SPI_CTRL_FASTEST	0x140	/* TODO: adjust TSHSL_CNT to 100ns */
#define     SPI_CTRL_DIV_MAX		0x3f
#define   SPI_IO_CTRL	  0x000008
#define     SBIT_CS_ALL		0x00070000
#define     SBIT_CS_0		0x00010000
#define     SBIT_CLK		0x00000100
#define   SPI_RDATA	  0x00000C
#define   SPI_SHIFT_DATAOUT_ADDR 0x10
#define   SPI_SHIFT_CNT_ADDR	0x14
#define     SPI_SHIFT_EN		(1 << 31)
#define     SPI_CS0			(1 << 28)
#define     SPI_CLK_INIT_HIGH		(1 << 27)
#define     SPI_TERMINATE		(1 << 26)
#define   SPI_SHIFT_DATAIN_ADDR	0x0c	/* data at 0x18 is missing last bit */

#define GPIO_BASE	0x18040000
#define   GPIO_INPUT_ENABLE   0x00
#define     GPO_LCD_MOSI	(1 << 4)
#define   GPIO_OUTPUT	      0x08
#define   GPIO_SET	      0x0c
#define     GPO_LCD_DCX		(1 << 0)
#define     GPO_ATTINY_nCS	(1 << 12)
#define   GPIO_CLEAR	      0x10
#define   GPIO_IN_ENABLE_0    0x44
#define     GPIN0_DEFAULT	0x00000908
#define     GPIN0_MISC(x)	(0x00000900 | (x))
#define     GPIN0_TS		0x0000090d
#define     GPIN0_LCD_READ	0x00000904

#define ATTINY_UDELAY	66

#define SPI_REG(x)	(*(volatile unsigned *)((unsigned) spi_base + (x)))
#define GPIO_REG(x)	(*(volatile unsigned *)((unsigned) gpio_base + (x)))

extern unsigned mips_hpt_frequency;
static unsigned ahb_clock = 0;
static unsigned spi_cmd_base = 0;
static unsigned spi_hclk_delay = 0;
static unsigned spi_clk_low = SBIT_CS_ALL;
static void __iomem *spi_base;
static void __iomem *gpio_base;

struct rb900_spi {
	spinlock_t		lock;
	struct list_head	queue;
	struct work_struct	wq;
	char			cs_wait;
	int			busy:1;
	int			attiny:1;
};

static void set_speed(unsigned hz, struct rb900_spi *rbspi) {
	int div = (ahb_clock - 1) / hz;
	spi_hclk_delay = 0;
	if (div < 0) div = 0;
	else if (div <= SPI_CTRL_DIV_MAX) {
		/* correct div, do nothing */
	}
	else if (rbspi->attiny) {
		div = 0;
		// coefficients are calculated from experiments with AHB=100MHz
		spi_hclk_delay = 432758015 / hz - 40;
	}
	else {
		/* FIXME: use spi_hclk_delay method with correct CS and delay */
		static int warned = 0;
		if (!warned) {
			warned = true;
			printk("spi_rb900 WARNING: "
			       "unable to set speed %u hz, use %u hz instead\n",
			       hz, ahb_clock / SPI_CTRL_DIV_MAX);
		}
		div = SPI_CTRL_DIV_MAX;
	}
#if RB900_DEBUG
	printk("set_speed %u => div %d hclk_delay %u\n",
	       hz, div, spi_hclk_delay);
#endif
	SPI_REG(SPI_CTRL) = SPI_CTRL_FASTEST | div;
}

static unsigned get_gpin0_ts(void) {
	switch (mips_machtype) {
	case MACH_MT_RB2011G:
	case MACH_MT_RB2011US:
	case MACH_MT_RB2011R5:
	case MACH_MT_RB951G:
	case MACH_MT_CRS109:
	case MACH_MT_CRS125G:
		return GPIN0_TS;
	case MACH_MT_SXTG5P:
	case MACH_MT_SXT5N:
		return GPIN0_MISC(10);
	}
	return GPIN0_DEFAULT;
}

static void do_spi_init(struct rb900_spi *rbspi, struct spi_device *spi) {
	int cs = spi->chip_select % 4;
	SPI_REG(SPI_FUNC) = SBIT_FUNC_GPIO;
	spi_cmd_base = SPI_SHIFT_EN | (SPI_CS0 << cs);
	spi_clk_low = SBIT_CS_ALL;
	if (!(spi->mode & SPI_CS_HIGH)) {
		spi_clk_low ^= (SBIT_CS_0 << cs);
	}
	if (spi->chip_select == 2) {
		GPIO_REG(GPIO_IN_ENABLE_0) = get_gpin0_ts();
	}
	if (spi->chip_select == 3) {
		GPIO_REG(GPIO_CLEAR) = GPO_ATTINY_nCS;
		GPIO_REG(GPIO_OUTPUT);
		spi_clk_low |= SBIT_CS_ALL;
		rbspi->attiny = 1;
	}
	if (spi->chip_select == 6) {
		rbspi->attiny = 1;
	}
	if (spi->mode & SPI_CPOL) {
		spi_cmd_base |= SPI_CLK_INIT_HIGH;
		// force CLK high before CS
		SPI_REG(SPI_SHIFT_CNT_ADDR) = SPI_CLK_INIT_HIGH | SPI_SHIFT_EN;
		// assert CS & CLK, deassert MOSI
		SPI_REG(SPI_IO_CTRL) = spi_clk_low | SBIT_CLK;
	}
	else {
		// assert CS, deassert MOSI & CLK
		SPI_REG(SPI_IO_CTRL) = spi_clk_low;
	}
	SPI_REG(SPI_IO_CTRL);
#if RB900_DEBUG
	printk("do_spi_init %x\n", spi_cmd_base);
#endif
}

static void spi_cs_terminate(struct rb900_spi *rbspi, struct spi_message *m) {
	SPI_REG(SPI_IO_CTRL) = (SBIT_CS_ALL | (SPI_REG(SPI_IO_CTRL) & 1));
	GPIO_REG(GPIO_IN_ENABLE_0) = GPIN0_DEFAULT;
#if RB900_DEBUG
	printk("spi_cs_terminate %x\n", spi_cmd_base);
#endif
	if (rbspi->attiny) {
		if (m->spi->chip_select == 3) {
			GPIO_REG(GPIO_SET) = GPO_ATTINY_nCS;
			GPIO_REG(GPIO_OUTPUT);
		}
		rbspi->attiny = 0;
		udelay(ATTINY_UDELAY);
	}
	if (m && m->ssr_strobe) {
		__gpio_set_value(m->ssr_strobe, 0);
		__gpio_get_value(m->ssr_strobe);
		__gpio_set_value(m->ssr_strobe, 1);
	}
	SPI_REG(SPI_FUNC) = 0;
}

static void inline do_spi_clk_delay(int bit) {
	unsigned bval = spi_clk_low | (bit & 1);
	ndelay(spi_hclk_delay);
	SPI_REG(SPI_IO_CTRL) = bval;
	SPI_REG(SPI_IO_CTRL);
	ndelay(spi_hclk_delay);
	SPI_REG(SPI_IO_CTRL) = bval | SBIT_CLK;
	SPI_REG(SPI_IO_CTRL);
}

static void do_spi_byte_delay(uint8_t byte) {
	do_spi_clk_delay(byte >> 7);
	do_spi_clk_delay(byte >> 6);
	do_spi_clk_delay(byte >> 5);
	do_spi_clk_delay(byte >> 4);
	do_spi_clk_delay(byte >> 3);
	do_spi_clk_delay(byte >> 2);
	do_spi_clk_delay(byte >> 1);
	do_spi_clk_delay(byte);
#if RB900_DEBUG
	printk("spi_byte_delay sent 0x%x got 0x%x\n",
	       (unsigned)byte,
	       SPI_REG(SPI_RDATA));
#endif
}

static int rb900_spi_txrx(struct rb900_spi *rbspi, struct spi_transfer *t)
{
	const unsigned char *rxv_ptr = NULL;
	const unsigned char *tx_ptr = t->tx_buf;
	unsigned char *rx_ptr = t->rx_buf;
	unsigned i = 0;

#if RB900_DEBUG
	printk("spi_txrx len %u tx %u rx %u\n",
	       t->len,
	       (t->tx_buf ? 1 : 0),
	       (t->rx_buf ? 1 : 0));
#endif
	if (t->verify) {
		rxv_ptr = tx_ptr;
		tx_ptr = NULL;
	}
	if (t->lcd_write_cmd) {
		GPIO_REG(GPIO_CLEAR) = GPO_LCD_DCX;
	}
	if (t->lcd_read) {
		GPIO_REG(GPIO_IN_ENABLE_0) = GPIN0_LCD_READ;
		GPIO_REG(GPIO_INPUT_ENABLE) |= GPO_LCD_MOSI;
	}
	if (tx_ptr == NULL) {
		SPI_REG(SPI_SHIFT_DATAOUT_ADDR) = 0;
	}

	if (spi_hclk_delay) {
		for (i = 0; i < t->len; ++i) {
			unsigned char rdata;
			unsigned char sdata = tx_ptr ? tx_ptr[i] : 0;

			if (rbspi->attiny) udelay(ATTINY_UDELAY);
			do_spi_byte_delay(sdata);

			rdata = SPI_REG(SPI_RDATA);
			if (rx_ptr) {
				rx_ptr[i] = rdata;
			}
			else if (rxv_ptr) {
				if (rxv_ptr[i] != rdata) goto err;
			}
		}
	}
	else if (t->len >= 4 && !rbspi->attiny && ((unsigned)rx_ptr & 3) == 0) {
		unsigned len4 = t->len / 4;
		const unsigned *tx32 = (const unsigned *)tx_ptr;
		const unsigned *rv32 = (const unsigned *)rxv_ptr;
		unsigned *rx32 = (unsigned *)rx_ptr;
		for (i = 0; i < len4; ++i) {
			unsigned rdata;
			if (tx_ptr) SPI_REG(SPI_SHIFT_DATAOUT_ADDR) = tx32[i];
			SPI_REG(SPI_SHIFT_CNT_ADDR) = spi_cmd_base | 32;
			rdata = SPI_REG(SPI_SHIFT_DATAIN_ADDR);
			if (rx32) {
				rx32[i] = rdata;
			}
			else if (rv32) {
				if (rv32[i] != rdata) {
					i *= 4;
					goto err;
				}
			}
		}
		i *= 4;
	}

	for (; i < t->len; ++i) {
		unsigned char rdata;
		if (rbspi->attiny) udelay(ATTINY_UDELAY);
		if (tx_ptr) SPI_REG(SPI_SHIFT_DATAOUT_ADDR) = tx_ptr[i];
		SPI_REG(SPI_SHIFT_CNT_ADDR) = spi_cmd_base | 8;
		rdata = SPI_REG(SPI_SHIFT_DATAIN_ADDR);
		if (rx_ptr) {
			rx_ptr[i] = rdata;
		}
		else if (rxv_ptr) {
			if (rxv_ptr[i] != rdata) break;
		}
	}
err:
	if (t->lcd_write_cmd) {
		GPIO_REG(GPIO_SET) = GPO_LCD_DCX;
	}
	if (t->lcd_read) {
		GPIO_REG(GPIO_INPUT_ENABLE) &= ~GPO_LCD_MOSI;
		GPIO_REG(GPIO_IN_ENABLE_0) = GPIN0_DEFAULT;
	}
	return i;
}

static int rb900_spi_msg(struct rb900_spi *rbspi,
			 struct spi_message *m, int cs) {
	struct spi_transfer *t = NULL;
	int cs_terminate = cs < 0 ? 1 : 0;

	m->status = 0;
	if (list_empty(&m->transfers)) {
		if (cs >= 0) {
			spi_cs_terminate(rbspi, m);
		}
		return -1;
	}

	list_for_each_entry (t, &m->transfers, transfer_list) {
		int last = list_is_last(&t->transfer_list, &m->transfers);
		unsigned hz = t->speed_hz ?: m->spi->max_speed_hz;
		int len;

		if (cs_terminate) {
			do_spi_init(rbspi, m->spi);
		}
		cs_terminate = t->cs_change ? !last : last;
		set_speed(hz, rbspi);

		len = rb900_spi_txrx(rbspi, t);
		if (len != t->len) {
			m->status = -EMSGSIZE;
			spi_cs_terminate(rbspi, m);
			return -1;
		}
		m->actual_length += len;

		if (cs_terminate) {
			spi_cs_terminate(rbspi, m);
		}
	}

	return cs_terminate ? -1 : m->spi->chip_select;
}

static void rb900_spi_process_queue_locked(struct rb900_spi *rbspi,
					   unsigned long *flags) {
	int cs = rbspi->cs_wait;
	rbspi->busy = 1;
	while (!list_empty(&rbspi->queue)) {
		struct spi_message *m;
		list_for_each_entry(m, &rbspi->queue, queue) {
			if (cs < 0 || cs == m->spi->chip_select) break;
		}
		if (&m->queue == &rbspi->queue) break;

		list_del_init(&m->queue);
		spin_unlock_irqrestore(&rbspi->lock, *flags);

		cs = rb900_spi_msg(rbspi, m, cs);
		m->complete(m->context);

		spin_lock_irqsave(&rbspi->lock, *flags);
	}
	rbspi->cs_wait = cs;
	rbspi->busy = 0;
	if (cs >= 0) {
		// TODO: add timer to unlock cs after 1s inactivity
	}
}

static void rb900_spi_process_queue(struct work_struct *wq) {
	struct rb900_spi *rbspi = container_of(wq, struct rb900_spi, wq);
	unsigned long flags;

	spin_lock_irqsave(&rbspi->lock, flags);
	if (!rbspi->busy) {
		rb900_spi_process_queue_locked(rbspi, &flags);
	}
	spin_unlock_irqrestore(&rbspi->lock, flags);
}

void spi_complete(void *arg);
static int rb900_spi_transfer(struct spi_device *spi,
			      struct spi_message *m) {
	struct rb900_spi *rbspi = spi_master_get_devdata(spi->master);
	unsigned long flags;

	m->actual_length = 0;
	m->status = -EINPROGRESS;

	spin_lock_irqsave(&rbspi->lock, flags);
	if (m->complete == spi_complete && in_atomic()) {
//		printk("rb900_spi: atomic with busy %d cs_wait %d\n",
//		       rbspi->busy, rbspi->cs_wait);
		// process queue from this context, otherwise panic save fails
		if (rbspi->busy || rbspi->cs_wait != -1) {
			// cleanup incomplete spi transfer
			rbspi->cs_wait = -1;
			spi_cs_terminate(rbspi, NULL);
		}
		rb900_spi_msg(rbspi, m, -1);	// process this message
	}
	else {
		list_add_tail(&m->queue, &rbspi->queue);
		if (rbspi->busy) {
			// queue processing already in progress
		}
		else {
			queue_work(system_unbound_wq, &rbspi->wq);
		}
	}
	spin_unlock_irqrestore(&rbspi->lock, flags);

	return 0;
}

static int rb900_spi_setup(struct spi_device *spi)
{
	struct rb900_spi *rbspi = spi_master_get_devdata(spi->master);
	unsigned long flags;
	unsigned mode = spi->mode & ~(SPI_CS_HIGH);

	if (mode != SPI_MODE_0 && mode != SPI_MODE_3) {
		printk("RB900 SPI: mode %x not supported\n",
		       (unsigned)spi->mode);
		return -EINVAL;
	}
	if (spi->bits_per_word != 8 && spi->bits_per_word != 0) {
		printk("RB900 SPI: bits_per_word %u not supported\n",
		       (unsigned)spi->bits_per_word);
		return -EINVAL;
	}

	spin_lock_irqsave(&rbspi->lock, flags);
	if (rbspi->cs_wait == spi->chip_select && !rbspi->busy) {
		rbspi->cs_wait = -1;
		spi_cs_terminate(rbspi, NULL);
		queue_work(system_unbound_wq, &rbspi->wq);
	}
	spin_unlock_irqrestore(&rbspi->lock, flags);
	return 0;
}

static unsigned read32_ioremap(unsigned addr) {
	void __iomem *cfg = ioremap(addr & ~(PAGE_SIZE - 1), PAGE_SIZE);
	unsigned val = rb400_readl((unsigned)(cfg + (addr & (PAGE_SIZE - 1))));
	iounmap(cfg);
	return val;
}

static int get_ahb_source_clock(void) {
    if (read32_ioremap(CPU_DDR_CLOCK_CONTROL) & BIT(24)) {
	unsigned nint = (read32_ioremap(DDR_PLL_CONFIG) >> 10) & 0x3f;
	unsigned refdiv = (read32_ioremap(DDR_PLL_CONFIG) >> 16) & 0x1f;
	unsigned outdiv = (read32_ioremap(DDR_PLL_CONFIG) >> 23) & 0x7;
	unsigned is_xtal40 = read32_ioremap(RST_BOOTSTRAP) & BIT(4);
	unsigned refclk = is_xtal40 ? 40000000 : 25000000;
	unsigned freq = nint * refclk / refdiv / (1 << outdiv) / 2;
	printk("ahb_clock_source is DDR (%dHz)\n", freq);
	return freq;
    }
    else {
	printk("ahb_clock_source is CORE (%dHz)\n", mips_hpt_frequency);
	return mips_hpt_frequency;
    }
}

static unsigned get_ahb_clock(void) {
	unsigned clk_ctrl = read32_ioremap(CPU_DDR_CLOCK_CONTROL);
	unsigned ahb_div = ((clk_ctrl >> 15) & 0x1f) + 1;
	return (get_ahb_source_clock() + ahb_div - 1) / ahb_div;
}

static void rb900_spi_add_devices(struct spi_master *master, const void *data) {
	struct spi_board_info **info;
	for (info = (struct spi_board_info **)data; *info != NULL; ++info) {
		(void) spi_new_device(master, *info);
	}
}

static int rb900_spi_probe(struct platform_device *pdev)
{
	struct spi_master *master;
	struct rb900_spi *rbspi;
	int err = 0;

	printk("RB900 SPI\n");
	master = spi_alloc_master(&pdev->dev, sizeof(*rbspi));
	if (master == NULL) {
		dev_err(&pdev->dev, "No memory for spi_master\n");
		err = -ENOMEM;
		goto err_nomem;
	}
	master->bus_num = 0;
	master->num_chipselect = 7;
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH;
	master->setup    = rb900_spi_setup;
	master->transfer = rb900_spi_transfer;

	rbspi = spi_master_get_devdata(master);
	memset(rbspi, 0, sizeof(*rbspi));
	rbspi->cs_wait = -1;

	ahb_clock = get_ahb_clock();
	printk("AHB clock %u Hz\n", ahb_clock);

	spin_lock_init(&rbspi->lock);
	INIT_LIST_HEAD(&rbspi->queue);
	INIT_WORK(&rbspi->wq, rb900_spi_process_queue);

	err = spi_register_master(master);
	if (err) {
		dev_err(&pdev->dev, "Failed to register SPI master\n");
		goto err_register;
	}
	rb900_spi_add_devices(master, pdev->dev.platform_data);
	return 0;

err_register:
	spi_master_put(master);;
err_nomem:
	return err;
}

static struct platform_driver rb900_spi_drv = {
	.probe		= rb900_spi_probe,
        .driver		= {
		.name	= "rb900-spi",
		.owner	= THIS_MODULE,
        },
};

static int __init rb900_spi_init(void)
{
	spi_base = ioremap(SPI_BASE, PAGE_SIZE);
	if (!spi_base)
		return -ENOMEM;

	gpio_base = ioremap(GPIO_BASE, PAGE_SIZE);
	if (!gpio_base) {
		iounmap(spi_base);
		return -ENOMEM;
	}

        return platform_driver_register(&rb900_spi_drv);
}

static void __exit rb900_spi_exit(void)
{
        platform_driver_unregister(&rb900_spi_drv);
	iounmap(spi_base);
	iounmap(gpio_base);
}

module_init(rb900_spi_init);
module_exit(rb900_spi_exit);
