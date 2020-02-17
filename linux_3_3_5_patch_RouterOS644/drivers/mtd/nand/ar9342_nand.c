#include <linux/init.h>
#include <linux/mtd/nand.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <asm/rb/rb400.h>

extern int rb_nand_probe(struct nand_chip *nand, int booter);

static struct nand_chip rnand;

#define GPIO_BASE	0x18040000
#define GPIO_IE_REG	    0x0000
#define GPIO_IN_REG	    0x0004
#define GPIO_O1_REG	    0x0008
#define GPIO_S1_REG	    0x000c
#define GPIO_S0_REG	    0x0010

#define GPIO_REG(x)	(*(volatile unsigned *)((unsigned) gpio_base + (x)))

static void __iomem *gpio_base;
static struct spi_device *spi = NULL;

/* ------------------ AR9342 nand driver -------------------- */

#define   GPO_AR9342_SSR_STROBE	(1 << 0)
#define   GPO_AR9342_NAND_READ	(1 << 3)
#define   GPI_AR9342_NAND_RDY	(1 << 4)
#define   GPO_AR9342_LATCH_EN	(1 << 11)
#define   GPO_AR9342_NAND_nRW	(1 << 12)
#define   GPO_AR9342_NAND_nCE	(1 << 13)
#define   GPO_AR9342_NAND_CLE	(1 << 14)
#define   GPO_AR9342_NAND_ALE	(1 << 15)

#define   GPO_NAND_DATA(x)	((((x) & 0x1f) << 0) | (((x) & 0xe0) << 8))
#define   GPI_NAND_DATA(x)	((((x) >> 0) & 0x1f) | (((x) >> 8) & 0xe0))

static inline void set_gpo(unsigned bits) {
    GPIO_REG(GPIO_S1_REG) = bits;
    GPIO_REG(GPIO_S1_REG);
}

static inline void clear_gpo(unsigned bits) {
    GPIO_REG(GPIO_S0_REG) = bits;
    GPIO_REG(GPIO_S0_REG);
}

static void fn_dummy_void(void *x) {
}

static void ar9342_ssr_cfg_callback(void *ptr) {
	static struct spi_transfer tx = {
		.tx_buf = NULL,
		.len = 2,
		.cs_change = 1,
	};
	static struct spi_message m_upd;
	static struct spi_message m_finish;

	unsigned char *data = (unsigned char *)ptr;

	if (spi == NULL)
		return;

	if (tx.tx_buf == NULL) {
		tx.tx_buf = data;

		spi_message_init(&m_upd);
		m_upd.context = data;
		m_upd.complete = ar9342_ssr_cfg_callback;
		spi_message_add_tail(&tx, &m_upd);

		spi_message_init(&m_finish);
		m_finish.complete = fn_dummy_void;
	}

	if (data[1] != data[2]) {
		// value has changed, start new transaction
		data[2] = data[1];
		spi_async(spi, &m_upd);
		return;
	}
	if (data[2] != data[3]) {
		// finish transaction with strobe
		int ok = ar9342_change_gpo(GPO_AR9342_SSR_STROBE, 0);
		ar9342_change_gpo(0, GPO_AR9342_SSR_STROBE);
		if (ok) {
			data[3] = data[2];	// active value updated
		}
		else {
			/*
			 * update failed - NAND is busy
			 * cancel spi transaction
			 * wait for NAND transaction to finish
			 * then spi transaction will be restarted
			 */
			data[2] = data[3];
		}
	}
	// release cs for other purpose
	spi_async(spi, &m_finish);
}

int ar9342_change_gpo(unsigned _off, unsigned _on) {
	static unsigned on = (GPO_AR9342_LATCH_EN | GPO_AR9342_NAND_nCE |
			      (7 << 25));
	static unsigned off = (GPO_AR9342_NAND_ALE |
			       GPO_AR9342_NAND_CLE |
			       GPO_AR9342_NAND_READ);
	static unsigned char ssr[4] = { 5 /*SPI_CMD_RD_STATUS*/,
					0xe0,	// newest value
					0xe0,	// last sent value
					0xe0 };	// currently active value
	static unsigned ie = 0;
	static DEFINE_SPINLOCK(lock);	// TODO: use atomic instead
	unsigned long flags;
	int res = 0;

	spin_lock_irqsave(&lock, flags);

	on = (on | _on) & ~_off;
	off = (off | _off) & ~_on;
	ssr[1] = (on >> 20) & 0xff;
	if (!ie) ie = GPIO_REG(GPIO_IE_REG);

	if (_on == 0 && _off == 0
		&& (GPIO_REG(GPIO_O1_REG) & GPO_AR9342_NAND_nRW)) {
		GPIO_REG(GPIO_IE_REG) |= (GPO_AR9342_NAND_ALE |
					  GPO_AR9342_NAND_CLE);
		GPIO_REG(GPIO_IE_REG);
		udelay(200);
		res = GPIO_REG(GPIO_IN_REG);
		GPIO_REG(GPIO_IE_REG) &= ~(GPO_AR9342_NAND_ALE |
					   GPO_AR9342_NAND_CLE);
		GPIO_REG(GPIO_IE_REG);
	}
	else if (on & GPO_AR9342_LATCH_EN) {
		GPIO_REG(GPIO_IE_REG) &= ie & ~(on | off);
		GPIO_REG(GPIO_S0_REG) = off;
		GPIO_REG(GPIO_S1_REG) = on;
		if (ssr[1] != ssr[2] && ssr[2] == ssr[3]) {
		    // start new spi transaction
		    ar9342_ssr_cfg_callback(ssr);
		}
		res = 1;
	}
	else if (_off & GPO_AR9342_LATCH_EN) {
		ie = GPIO_REG(GPIO_IE_REG);
		clear_gpo(GPO_AR9342_LATCH_EN);
	}

	spin_unlock_irqrestore(&lock, flags);
	return res;
}
EXPORT_SYMBOL(ar9342_change_gpo);

static int ar9342_dev_ready(struct mtd_info *mtd) {
	return GPIO_REG(GPIO_IN_REG) & GPI_AR9342_NAND_RDY;
}

static void ar9342_write_bytes_gpio(const uint8_t *data, unsigned cnt) {
	int i;
	unsigned gpo;
	unsigned gpo_orig;
	unsigned gpie_orig = GPIO_REG(GPIO_IE_REG);

	ar9342_change_gpo(GPO_AR9342_LATCH_EN, 0);
	gpo_orig = GPIO_REG(GPIO_O1_REG);
	GPIO_REG(GPIO_IE_REG) = gpie_orig & ~GPO_NAND_DATA(0xff);
	gpo = gpo_orig & ~(GPO_AR9342_NAND_nRW | GPO_NAND_DATA(0xff));
	/* XXX: any GPIO output val change in parallel (for led) may get lost */

	for (i = 0; i < cnt; ++i) {
		unsigned val = data[i];
		unsigned gpov = gpo | GPO_NAND_DATA(val);

		GPIO_REG(GPIO_O1_REG) = gpov;
		GPIO_REG(GPIO_S1_REG) = GPO_AR9342_NAND_nRW;
	}

	GPIO_REG(GPIO_IE_REG) = gpie_orig;
	GPIO_REG(GPIO_O1_REG) = gpo_orig;
	ar9342_change_gpo(0, GPO_AR9342_LATCH_EN);
}

static int ar9342_read_bytes_gpio(uint8_t *data, unsigned cnt,
				  const uint8_t *verify) {
	int i;
	int ret = 0;
	unsigned gpo_orig;
	unsigned gpie_orig = GPIO_REG(GPIO_IE_REG);

	ar9342_change_gpo(0, GPO_AR9342_NAND_READ);
	ar9342_change_gpo(GPO_AR9342_LATCH_EN, 0);
	gpo_orig = GPIO_REG(GPIO_O1_REG);
	GPIO_REG(GPIO_IE_REG) = gpie_orig | GPO_NAND_DATA(0xff);

	for (i = 0; i < cnt; ++i) {
		uint8_t val;

		clear_gpo(GPO_AR9342_NAND_nRW);

		val = GPI_NAND_DATA(GPIO_REG(GPIO_IN_REG));

		GPIO_REG(GPIO_S1_REG) = GPO_AR9342_NAND_nRW;

		if (data) {
			data[i] = val;
		}
		else if (verify) {
			if (verify[i] != val) {
				ret = -EFAULT;
				break;
			}
		}
	}

	GPIO_REG(GPIO_IE_REG) = gpie_orig;
	GPIO_REG(GPIO_O1_REG) = gpo_orig;
	ar9342_change_gpo(GPO_AR9342_NAND_READ, GPO_AR9342_LATCH_EN);
	return ret;
}

static void ar9342_hwcontrol(struct mtd_info *mtd, int cmd,
			     unsigned int ctrl) {
	if (ctrl & NAND_CTRL_CHANGE) {
		unsigned gpo = 0;
		if (!(ctrl & NAND_NCE)) {
			gpo |= GPO_AR9342_NAND_nCE;
		}
		if (ctrl & NAND_CLE) {
			gpo |= GPO_AR9342_NAND_CLE;
		}
		if (ctrl & NAND_ALE) {
			gpo |= GPO_AR9342_NAND_ALE;
		}
		ar9342_change_gpo(gpo ^ (GPO_AR9342_NAND_nCE |
					 GPO_AR9342_NAND_ALE |
					 GPO_AR9342_NAND_CLE), gpo);
	}

	if (cmd != NAND_CMD_NONE) {
		uint8_t data = cmd;
		ar9342_write_bytes_gpio(&data, 1);
	}
}

static uint8_t ar9342_read_byte(struct mtd_info *mtd)
{
	uint8_t data;
	ar9342_read_bytes_gpio(&data, 1, NULL);
	return data;
}

static void ar9342_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	ar9342_write_bytes_gpio(buf, len);
}

static void ar9342_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	ar9342_read_bytes_gpio(buf, len, NULL);
}

static int ar9342_verify_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	return ar9342_read_bytes_gpio(NULL, len, buf);
}

#ifdef DEBUG_SPEED
static void ar9342_test_speed(void) {
	char buf[1024];
	unsigned long ticks;
	unsigned kb;

	set_gpo(GPO_AR9342_NAND_nCE);

	/* wait for "start of" clock tick */
	kb = 0;
	ticks = jiffies;
	while (ticks == jiffies)
		/* nothing */;
	ticks = jiffies + HZ / 10;

	while ((long)(jiffies - ticks) < 0) {
		ar9342_read_bytes_gpio(buf, 1024, NULL);
		++kb;
	}
	printk("read speed is %u KB/s\n", kb * 10);

	/* wait for "start of" clock tick */
	kb = 0;
	ticks = jiffies;
	while (ticks == jiffies)
		/* nothing */;
	ticks = jiffies + HZ / 10;

	while ((long)(jiffies - ticks) < 0) {
		ar9342_write_bytes_gpio(buf, 1024);
		++kb;
	}
	printk("write speed is %u KB/s\n", kb * 10);
}
#endif

static int ar9342_nand_probe(struct platform_device *pdev)
{
	printk("AR9342 nand\n");
#ifdef DEBUG_SPEED
	ar9342_test_speed();
#endif
	memset(&rnand, 0, sizeof(rnand));

	rnand.cmd_ctrl = ar9342_hwcontrol;
	rnand.dev_ready = ar9342_dev_ready;
	rnand.read_byte = ar9342_read_byte;
	rnand.write_buf = ar9342_write_buf;
	rnand.read_buf = ar9342_read_buf;
	rnand.verify_buf = ar9342_verify_buf;

	return rb_nand_probe(&rnand, 1);
}

static struct platform_driver ar9342_nand_driver = {
	.probe	= ar9342_nand_probe,
	.driver	= {
		.name = "ar9342-nand",
		.owner = THIS_MODULE,
	},
};

static int ar9342_spi_ssr_probe(struct spi_device *_spi)
{
	printk("AR9342 spi ssr\n");
	spi = _spi;
	return 0;
}

static struct spi_driver ar9342_spi_ssr = {
	.driver	= {
		.name = "ar9342-spi-ssr",
		.owner = THIS_MODULE,
	},
	.probe	= ar9342_spi_ssr_probe,
};

/* ------------------ common init/exit code -------------------- */

static int __init ar9342_nand_init(void)
{
	gpio_base = ioremap_nocache(GPIO_BASE, PAGE_SIZE);
	if (!gpio_base)
		return -ENOMEM;

	spi_register_driver(&ar9342_spi_ssr);
	return platform_driver_register(&ar9342_nand_driver);
}

static void __exit ar9342_nand_exit(void)
{
	iounmap(gpio_base);

	platform_driver_unregister(&ar9342_nand_driver);
}

module_init(ar9342_nand_init);
module_exit(ar9342_nand_exit);
