#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/hardirq.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <asm/rb/rb400.h>
#include <asm/bootinfo.h>
#include <asm/rb/boards.h>

#include <linux/spi/spi.h>

#define RB400_DEBUG 0
#define REG_PLL_CONFIG	0x18050000
#define REG_RESET_BASE	0x18060000
#define   RST_REVISION_ID     0x90

#if RB400_DEBUG
#define CLK_HALF_DELAY() ndelay(20000)
#else
#define CLK_HALF_DELAY() while(0) {}
#endif

#define SPI_BASE	0x1f000000
#define SPI_BASE_MUSICL	0x19000000
#define   SPI_FUNC	  0x000000
#define     SBIT_FUNC_GPIO	0x00000001
#define   SPI_CTRL	  0x000004
#define	    SPI_CTRL_FASTEST	0x40
#define     SPI_FLASH_HZ	33333334
#define     SPI_CPLD_HZ		33333334
#define   SPI_IO_CTRL	  0x000008
#define     SBIT_DO2_SHIFT	18
#define     SBIT_DO2		(1u << SBIT_DO2_SHIFT)
#define     SBIT_CS_ALL		0x00070000
#define     SBIT_CS_2		0x00040000
#define     SBIT_CS_1		0x00020000
#define     SBIT_CS_0		0x00010000
#define     SBIT_CLK		0x00000100
#define     SBIT_DO_SHIFT	0
#define     SBIT_DO		(1u << SBIT_DO_SHIFT)
#define   SPI_RDATA	  0x00000C

#define	 SPI1_CMD_LCD		(1 << 1)
#define   CFG_BIT_LCD_READ	(1 << 4)
#define   CFG_BIT_LCD_nCMD	(1 << 6)
#define   CFG_BIT_LCD_nCS	(1 << 7)

#define ATTINY_UDELAY	100

#define SPI_REG(x)	(*(volatile unsigned *)((unsigned) spi_base + (x)))

extern unsigned mips_hpt_frequency;
static void __iomem *spi_base;

struct rb400_spi {
	unsigned		spi_ctrl_flash;
	unsigned		spi_ctrl_fread;

	spinlock_t		lock;
	struct list_head	queue;
	int			busy:1;
	int			attiny:1;
	int			lcd:1;
	int			ccs112_lcd:1;
	int			cs_wait;
};
static unsigned spi_clk_low = SBIT_CS_ALL;
static unsigned spi_hclk_delay = 0;
static const unsigned spi_cs_map[] = {
	SBIT_CS_ALL ^ SBIT_CS_0,
	SBIT_CS_ALL ^ SBIT_CS_0 ^ SBIT_CS_2,
	SBIT_CS_ALL ^ SBIT_CS_1,		// uSD or NAND
	SBIT_CS_ALL ^ SBIT_CS_2,		// CS3 = ATtiny
	SBIT_CS_ALL ^ SBIT_CS_0 ^ SBIT_CS_2,	// large spi nor
	SBIT_CS_ALL ^ SBIT_CS_1,		// CS5 = LCD
	SBIT_CS_ALL ^ SBIT_CS_1,		// CS6 = TS on CS1
	SBIT_CS_ALL ^ SBIT_CS_2,		// CS7 = TS on CS2
	SBIT_CS_ALL ^ SBIT_CS_1,		// CS8 = CCS112 LCD
	SBIT_CS_ALL ^ SBIT_CS_1,		// CS9 = CCS112 TS
	SBIT_CS_ALL ^ SBIT_CS_1,		// CS10 = PoE or SSR
	SBIT_CS_ALL ^ SBIT_CS_1,		// CS11 = CCS112 PoE
	SBIT_CS_ALL
};

static void set_speed(unsigned hz) {
	unsigned gpio_hz_max = 10000000;
	if (mips_machgroup == MACH_GROUP_MT_MUSIC) {
		gpio_hz_max = 20000000;
	}
	if (hz == 0 || hz >= gpio_hz_max) {
		spi_hclk_delay = 0;
	}
	else {
		spi_hclk_delay = 50 * gpio_hz_max / hz - 50;
	}
#if RB400_DEBUG
	if (spi_hclk_delay < 20000) spi_hclk_delay = 20000;
#endif
}

static void do_spi_byte(uint8_t byte);

#define CCS112_GPIO_CS 1
static void change_ccs112_ssr(unsigned off, unsigned on) {
	unsigned ssr;
	__gpio_set_value(CCS112_GPIO_CS, 0);
	__gpio_get_value(CCS112_GPIO_CS);
	rb_change_or_get_cfg(off, on, &ssr);
	do_spi_byte(ssr & 0xff);
	__gpio_set_value(CCS112_GPIO_CS, 1);
	__gpio_get_value(CCS112_GPIO_CS);
	__gpio_set_value(CCS112_GPIO_CS, 0);
	__gpio_get_value(CCS112_GPIO_CS);
	__gpio_set_value(CCS112_GPIO_CS, 1);
}

static void do_spi_init(struct rb400_spi *rbspi, struct spi_device *spi) {
	set_speed(spi->max_speed_hz);
	if (unlikely(spi->mode & SPI_CS_HIGH)) {
		spi_clk_low = SBIT_CS_ALL;
	}
	else {
		spi_clk_low = spi_cs_map[spi->chip_select];
		if (unlikely(spi->mode & SPI_CPOL)) {
			SPI_REG(SPI_IO_CTRL) = spi_clk_low | SBIT_CLK;
		}
		else {
			SPI_REG(SPI_IO_CTRL) = spi_clk_low;
		}
#if RB400_DEBUG
		printk("do_spi_init CS %x\n",
		       (spi_clk_low ^ SBIT_CS_ALL) >> 16);
#endif
	}
	if (spi->controller_data) {
		const struct rb400_spi_ctrl *ctrl =
			(const struct rb400_spi_ctrl *)spi->controller_data;
		if (ctrl->gpio_as_cs) {
			__gpio_set_value(ctrl->gpio_as_cs, 0);
		}
		if (ctrl->cmd_prefix) {
			do_spi_byte(ctrl->cmd_prefix);
		}
	}
	else if (spi->chip_select == 8) {
		change_ccs112_ssr(BIT(1), BIT(0) | BIT(2));
		rbspi->ccs112_lcd = 1;
	}
	else if (spi->chip_select == 9) {
		change_ccs112_ssr(BIT(2), BIT(0) | BIT(1));
	}
	rbspi->attiny = (spi->chip_select == 3) || (spi->chip_select == 11);
	rbspi->lcd = (spi->chip_select == 5);
}

static void inline do_spi_finish(struct rb400_spi *rbspi,
				 struct spi_device *spi,
				 struct spi_message *m) {
	if (unlikely(spi_hclk_delay)) ndelay(spi_hclk_delay);
	if (rbspi->attiny) udelay(ATTINY_UDELAY);
	if (rbspi->lcd) do_spi_byte(SPI1_CMD_LCD | CFG_BIT_LCD_nCS);
	if (spi->controller_data) {
		const struct rb400_spi_ctrl *ctrl =
			(const struct rb400_spi_ctrl *)spi->controller_data;
		if (ctrl->gpio_as_strobe) {
			__gpio_set_value(ctrl->gpio_as_strobe, 1);
			__gpio_get_value(ctrl->gpio_as_strobe);		// flush
			__gpio_set_value(ctrl->gpio_as_strobe, 0);
			__gpio_get_value(ctrl->gpio_as_strobe);		// flush
		}
		if (ctrl->gpio_as_cs) {
			__gpio_set_value(ctrl->gpio_as_cs, 1);
		}
		if (ctrl->cs_as_strobe) {
			SPI_REG(SPI_IO_CTRL) |= SBIT_CS_ALL;
			do_spi_init(rbspi, spi);
		}
	}
	if (m && m->ssr_strobe) {
		__gpio_set_value(m->ssr_strobe, 0);
		__gpio_get_value(m->ssr_strobe);
		__gpio_set_value(m->ssr_strobe, 1);
	}
	else if (spi->chip_select == 8 || spi->chip_select == 9) {
		change_ccs112_ssr(0, BIT(0) | BIT(1) | BIT(2));
		rbspi->ccs112_lcd = 0;
	}
	SPI_REG(SPI_IO_CTRL) = (SBIT_CS_ALL | (SPI_REG(SPI_IO_CTRL) & 1));
#if RB400_DEBUG
	printk("do_spi_finish (CS %x)\n", SBIT_CS_ALL >> 16);
#endif
	if (rbspi->attiny) udelay(ATTINY_UDELAY);
}

static void inline do_spi_clk(int bit) {
	unsigned bval = spi_clk_low | (bit & 1);
	CLK_HALF_DELAY();
	SPI_REG(SPI_IO_CTRL) = bval;
	CLK_HALF_DELAY();
	SPI_REG(SPI_IO_CTRL) = bval | SBIT_CLK;
}

static void do_spi_byte(uint8_t byte) {
	do_spi_clk(byte >> 7);
	do_spi_clk(byte >> 6);
	do_spi_clk(byte >> 5);
	do_spi_clk(byte >> 4);
	do_spi_clk(byte >> 3);
	do_spi_clk(byte >> 2);
	do_spi_clk(byte >> 1);
	do_spi_clk(byte);
#if RB400_DEBUG
	printk("spi_byte sent 0x%x got 0x%x\n",
	       (unsigned)byte,
	       SPI_REG(SPI_RDATA));
#endif
}

static void inline do_spi_clk_delay(int bit) {
	unsigned bval = spi_clk_low | (bit & 1);
	ndelay(spi_hclk_delay);
	SPI_REG(SPI_IO_CTRL) = bval;
	ndelay(spi_hclk_delay);
	SPI_REG(SPI_IO_CTRL) = bval | SBIT_CLK;
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
#if RB400_DEBUG
	printk("spi_byte_delay sent 0x%x got 0x%x\n",
	       (unsigned)byte,
	       SPI_REG(SPI_RDATA));
#endif
}

static void inline do_spi_clk_fast(int bit1, int bit2) {
	unsigned bval = (spi_clk_low |
			 ((bit1 << SBIT_DO_SHIFT) & SBIT_DO) |
			 ((bit2 << SBIT_DO2_SHIFT) & SBIT_DO2));
	CLK_HALF_DELAY();
	SPI_REG(SPI_IO_CTRL) = bval;
	CLK_HALF_DELAY();
	SPI_REG(SPI_IO_CTRL) = bval | SBIT_CLK;
}

static void do_spi_byte_fast(uint8_t byte) {
	do_spi_clk_fast(byte >> 7, byte >> 6);
	do_spi_clk_fast(byte >> 5, byte >> 4);
	do_spi_clk_fast(byte >> 3, byte >> 2);
	do_spi_clk_fast(byte >> 1, byte >> 0);
#if RB400_DEBUG
	printk("spi_byte_fast sent 0x%x got 0x%x\n",
	       (unsigned)byte,
	       SPI_REG(SPI_RDATA));
#endif
}

static int rb400_spi_txrx(struct rb400_spi *rbspi, struct spi_transfer *t)
{
	const unsigned char *rxv_ptr = NULL;
	const unsigned char *tx_ptr = t->tx_buf;
	unsigned char *rx_ptr = t->rx_buf;
	unsigned i;

#if RB400_DEBUG
	printk("spi_txrx len %u tx %u rx %u\n",
	       t->len,
	       (t->tx_buf ? 1 : 0),
	       (t->rx_buf ? 1 : 0));
#endif
	if (t->verify) {
		rxv_ptr = tx_ptr;
		tx_ptr = NULL;
	}
	if (rbspi->lcd) {
		unsigned char lcd_cfg = SPI1_CMD_LCD;
		if (!t->lcd_write_cmd) {
			lcd_cfg |= CFG_BIT_LCD_nCMD;
		}
		if (t->lcd_read) {
			lcd_cfg |= CFG_BIT_LCD_READ;
		}
		do_spi_byte(lcd_cfg);
	}
	if (rbspi->ccs112_lcd && t->lcd_write_cmd) {
		change_ccs112_ssr(BIT(4), 0);
	}
	if (t->speed_hz) {
		set_speed(t->speed_hz);
	}

	for (i = 0; i < t->len; ++i) {
		unsigned char rdata;
		unsigned char sdata = tx_ptr ? tx_ptr[i] : 0;

		if (rbspi->attiny && !t->speed_hz) {
			udelay(ATTINY_UDELAY);
		}
		if (t->fast_write)
			do_spi_byte_fast(sdata);
		else if (unlikely(spi_hclk_delay))
			do_spi_byte_delay(sdata);
		else
			do_spi_byte(sdata);

		rdata = SPI_REG(SPI_RDATA);
		if (rx_ptr) {
			rx_ptr[i] = rdata;
		}
		else if (rxv_ptr) {
			if (rxv_ptr[i] != rdata) break;
		}
	}
	if (rbspi->ccs112_lcd && t->lcd_write_cmd) {
		change_ccs112_ssr(0, BIT(4));
	}
	if (rbspi->lcd) {
		SPI_REG(SPI_IO_CTRL) |= SBIT_CS_ALL;
	}
	if (t->leave_clk_low) {
		SPI_REG(SPI_IO_CTRL) = spi_clk_low;
	}
	if (t->leave_clk_hi) {
		SPI_REG(SPI_IO_CTRL) = spi_clk_low | SBIT_CLK;
	}
	return i;
}

static int rb400_spi_read_fast(struct rb400_spi *rbspi,
			       struct spi_message *m) {
	struct spi_transfer *t;
	const unsigned char *tx_ptr;
	unsigned addr;

	/* check for exactly two transfers */
	if (list_empty(&m->transfers) ||
	    list_is_last(m->transfers.next, &m->transfers) ||
	    !list_is_last(m->transfers.next->next, &m->transfers)) {
		return -1;
	}

	/* first transfer contains command and address  */
	t = list_entry(m->transfers.next,
		       struct spi_transfer, transfer_list);
	if (t->len != 5 || t->tx_buf == NULL) {
		return -1;
	}
	tx_ptr = t->tx_buf;
	if (tx_ptr[0] != SPI_CMD_READ_FAST) {
		return -1;
	}
	addr = tx_ptr[1];
	addr = tx_ptr[2] | (addr << 8);
	addr = tx_ptr[3] | (addr << 8);
	addr += (unsigned) spi_base;

	m->actual_length += t->len;

	/* second transfer contains data itself */
	t = list_entry(m->transfers.next->next,
		       struct spi_transfer, transfer_list);

	if (t->tx_buf && !t->verify) {
		return -1;
	}

	SPI_REG(SPI_FUNC) = SBIT_FUNC_GPIO;
	SPI_REG(SPI_CTRL) = rbspi->spi_ctrl_fread;
	SPI_REG(SPI_FUNC) = 0;

	if (t->rx_buf) {
		memcpy(t->rx_buf, (const void *)addr, t->len);
	}
	else if (t->tx_buf) {
		unsigned char buf[t->len];
		memcpy(buf, (const void *)addr, t->len);
		if (memcmp(t->tx_buf, buf, t->len) != 0) {
			m->status = -EMSGSIZE;
		}
	}
	m->actual_length += t->len;

	if (rbspi->spi_ctrl_flash != rbspi->spi_ctrl_fread) {
		SPI_REG(SPI_FUNC) = SBIT_FUNC_GPIO;
		SPI_REG(SPI_CTRL) = rbspi->spi_ctrl_flash;
		SPI_REG(SPI_FUNC) = 0;
	}
	
	return 0;
}

static int rb400_spi_msg(struct rb400_spi *rbspi,
			 struct spi_message *m, int cs) {
	struct spi_transfer *t = NULL;

	m->status = 0;
	if (list_empty(&m->transfers)) {
		if (cs >= 0) goto finish;
		return -1;
	}
	if (m->fast_read) {
		if (rb400_spi_read_fast(rbspi, m) == 0)
			return -1;
	}

	if (cs != m->spi->chip_select) {
		SPI_REG(SPI_FUNC) = SBIT_FUNC_GPIO;
		SPI_REG(SPI_CTRL) = SPI_CTRL_FASTEST;
		do_spi_init(rbspi, m->spi);
	}

	list_for_each_entry (t, &m->transfers, transfer_list) {
		int len = rb400_spi_txrx(rbspi, t);
		if (len != t->len) {
			m->status = -EMSGSIZE;
			break;
		}
		m->actual_length += len;

		if (t->cs_change) {
			if (list_is_last(&t->transfer_list, &m->transfers)) {
				/* wait for continuation */
				return m->spi->chip_select;
			}
			do_spi_finish(rbspi, m->spi, m);
			ndelay(100);
			do_spi_init(rbspi, m->spi);
		}
	}

finish:
	do_spi_finish(rbspi, m->spi, m);
	SPI_REG(SPI_CTRL) = rbspi->spi_ctrl_flash;
	SPI_REG(SPI_FUNC) = 0;
	return -1;
}

static void rb400_spi_process_queue_locked(struct rb400_spi *rbspi,
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

		cs = rb400_spi_msg(rbspi, m, cs);
		m->complete(m->context);

		spin_lock_irqsave(&rbspi->lock, *flags);
	}
	rbspi->cs_wait = cs;
	rbspi->busy = 0;
	if (cs >= 0) {
		// TODO: add timer to unlock cs after 1s inactivity
	}
}

void spi_complete(void *arg);
static int rb400_spi_transfer(struct spi_device *spi,
			      struct spi_message *m) {
	struct rb400_spi *rbspi = spi_master_get_devdata(spi->master);
	unsigned long flags;

	m->actual_length = 0;
	m->status = -EINPROGRESS;

	spin_lock_irqsave(&rbspi->lock, flags);
	list_add_tail(&m->queue, &rbspi->queue);
	if (m->complete == spi_complete && in_atomic()) {
//		printk("rb400_spi: atomic with busy %d cs_wait %d\n",
//		       rbspi->busy, rbspi->cs_wait);
		rbspi->cs_wait = -1;
	}
	else if (rbspi->busy ||
	    (rbspi->cs_wait >= 0 && rbspi->cs_wait != m->spi->chip_select)) {
		/* job will be done later */
//		printk("rb400_spi: delay non-atomic with busy %d cs_wait %d\n",
//		       rbspi->busy, rbspi->cs_wait);
		spin_unlock_irqrestore(&rbspi->lock, flags);
		return 0;
	}

	/* process job in current context */
	rb400_spi_process_queue_locked(rbspi, &flags);
	spin_unlock_irqrestore(&rbspi->lock, flags);

	return 0;
}

static int rb400_spi_setup(struct spi_device *spi)
{
	struct rb400_spi *rbspi = spi_master_get_devdata(spi->master);
	unsigned long flags;
	unsigned mode = spi->mode & ~(SPI_CS_HIGH);

	if (mode != SPI_MODE_0 && mode != SPI_MODE_3) {
		printk("RB400 SPI: mode %x not supported\n",
		       (unsigned)spi->mode);
		return -EINVAL;
	}
	if (spi->bits_per_word != 8 && spi->bits_per_word != 0) {
		printk("RB400 SPI: bits_per_word %u not supported\n",
		       (unsigned)spi->bits_per_word);
		return -EINVAL;
	}

	spin_lock_irqsave(&rbspi->lock, flags);
	if (rbspi->cs_wait == spi->chip_select && !rbspi->busy) {
		do_spi_finish(rbspi, spi, NULL);
		SPI_REG(SPI_CTRL) = rbspi->spi_ctrl_flash;
		SPI_REG(SPI_FUNC) = 0;

		rbspi->cs_wait = -1;
		rb400_spi_process_queue_locked(rbspi, &flags);
	}
	spin_unlock_irqrestore(&rbspi->lock, flags);
	return 0;
}

static unsigned get_ahb_clock(void) {
	void __iomem *pll_cfg;
	void __iomem *reset_cfg;
	unsigned revid;
	unsigned ahb_div = 1;

	if (mips_machgroup == MACH_GROUP_MT_MUSIC) {
		return mips_hpt_frequency;
	}
	pll_cfg = ioremap(REG_PLL_CONFIG, PAGE_SIZE);
	reset_cfg = ioremap(REG_RESET_BASE, PAGE_SIZE);
	revid = rb400_readl((unsigned) reset_cfg + RST_REVISION_ID);

	switch (revid & 0xff0) {
	case 0x0a0:
		ahb_div = ((rb400_readl((unsigned) pll_cfg) >> 20) & 0x7) + 1;
		break;
	case 0x0b0:
	case 0x0c0:
	case 0x100:
		ahb_div = ((rb400_readl((unsigned) pll_cfg) >> 19) & 1) + 1;
		break;
	case 0x110:
		ahb_div = ((rb400_readl((unsigned) pll_cfg + 8) >> 15) & 3) + 1;
		break;
	default:
		printk(KERN_WARNING "RB400 SPI FIXME: unknown ahb_clock"
		       " for CPU revision id %x\n", revid);
		break;
	}
	iounmap(reset_cfg);
	iounmap(pll_cfg);
	return (mips_hpt_frequency  + ahb_div - 1) / ahb_div;
}

static unsigned get_spi_ctrl(unsigned hz_max, const char *name) {
	unsigned ahb_clock = get_ahb_clock();
	unsigned div = (ahb_clock - 1) / (2 * hz_max);
	if (div == 0) {
		// AR7100 CPU has a bug at (div == 0) - first bit read is random
		++div;
	}
	if (name) {
		unsigned ahb_khz = (ahb_clock + 500) / 1000;
		unsigned div_real = 2 * (div + 1);
		printk(KERN_INFO "%s SPI clock %u kHz (AHB %u kHz / %u)\n",
		       name,
		       ahb_khz / div_real,
		       ahb_khz, div_real);
	}
	return SPI_CTRL_FASTEST + div;
}

static void rb400_spi_add_devices(struct spi_master *master, const void *data) {
	struct spi_board_info **info;
	for (info = (struct spi_board_info **)data; *info != NULL; ++info) {
		(void) spi_new_device(master, *info);
	}
}

static int rb400_spi_probe(struct platform_device *pdev)
{
	struct spi_master *master;
	struct rb400_spi *rbspi;
	int err = 0;

	master = spi_alloc_master(&pdev->dev, sizeof(*rbspi));
	if (master == NULL) {
		dev_err(&pdev->dev, "No memory for spi_master\n");
		err = -ENOMEM;
		goto err_nomem;
	}
	master->bus_num = 0;
	master->num_chipselect = ARRAY_SIZE(spi_cs_map) - 1;
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH;
	master->setup    = rb400_spi_setup;
	master->transfer = rb400_spi_transfer;

	rbspi = spi_master_get_devdata(master);
	memset(rbspi, 0, sizeof(*rbspi));
	rbspi->spi_ctrl_flash = get_spi_ctrl(SPI_FLASH_HZ, "FLASH");
	rbspi->spi_ctrl_fread = get_spi_ctrl(SPI_CPLD_HZ, "CPLD");
	rbspi->cs_wait = -1;

	spin_lock_init(&rbspi->lock);
	INIT_LIST_HEAD(&rbspi->queue);

	err = spi_register_master(master);
	if (err) {
		dev_err(&pdev->dev, "Failed to register SPI master\n");
		goto err_register;
	}
	rb400_spi_add_devices(master, pdev->dev.platform_data);
	return 0;

err_register:
	spi_master_put(master);;
err_nomem:
	return err;
}

static struct platform_driver rb400_spi_drv = {
	.probe		= rb400_spi_probe,
        .driver		= {
		.name	= "rb400-spi",
		.owner	= THIS_MODULE,
        },
};

static int __init rb400_spi_init(void)
{
	unsigned long baddr = SPI_BASE;
	if (is_music_lite()) baddr = SPI_BASE_MUSICL;
	spi_base = ioremap(baddr, 0x01000000);
	if (!spi_base)
		return -ENOMEM;

        return platform_driver_register(&rb400_spi_drv);
}

static void __exit rb400_spi_exit(void)
{
        platform_driver_unregister(&rb400_spi_drv);
	iounmap(spi_base);
}

module_init(rb400_spi_init);
module_exit(rb400_spi_exit);
