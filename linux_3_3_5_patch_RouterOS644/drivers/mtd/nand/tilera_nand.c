#include <linux/init.h>
#include <linux/mtd/nand.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <gxio/gpio.h>

#define DEBUG_SPEED

extern int rb_nand_probe(struct nand_chip *nand, int booter);

static struct nand_chip rnand;

/* ------------------ TILERA nand driver -------------------- */

#define   GPO_TILERA_NAND_nCE	(1ULL << 29)
#define   GPO_TILERA_NAND_CLE	(1ULL << 28)
#define   GPO_TILERA_NAND_ALE	(1ULL << 27)
#define   GPO_TILERA_NAND_nWE	(1ULL << 26)
#define   GPO_TILERA_NAND_nRE	(1ULL << 30)
#define   GPI_TILERA_NAND_RDY	(1ULL << 31)

#define   GPO_NAND_DATA(x)	(((x) & 0xff) << 18)
#define   GPI_NAND_DATA(x)	(((x) >> 18) & 0xff)

static gxio_gpio_context_t gpio_ctx;

static int tilera_dev_ready(struct mtd_info *mtd) {
	return gxio_gpio_get(&gpio_ctx) & GPI_TILERA_NAND_RDY;
}

static void tilera_write_bytes_gpio(const uint8_t *data, unsigned cnt) {
	int i;
	unsigned val_last = 0xff;
	__gxio_mmio_write(gpio_ctx.mmio_base + GPIO_PIN_SET,
			  GPO_NAND_DATA(0xff) | GPO_TILERA_NAND_nWE);
	for (i = 0; i < cnt; ++i) {
		unsigned val = data[i];
		__gxio_mmio_write(gpio_ctx.mmio_base + GPIO_PIN_OUTPUT_TGL,
				  GPO_NAND_DATA(val ^ val_last) |
				  GPO_TILERA_NAND_nWE);
		val_last = val;

		__gxio_mmio_write(gpio_ctx.mmio_base + GPIO_PIN_SET,
				  GPO_TILERA_NAND_nWE);
	}
}

static int tilera_read_bytes_gpio(uint8_t *data, unsigned cnt,
				  const uint8_t *verify) {
	int ret = 0;
	int i;
	__gxio_mmio_write(gpio_ctx.mmio_base + GPIO_PIN_RELEASE,
			  GPO_NAND_DATA(0xff));

	for (i = 0; i < cnt; ++i) {
		uint8_t val;

		__gxio_mmio_write(gpio_ctx.mmio_base + GPIO_PIN_CLR,
				  GPO_TILERA_NAND_nRE);
		__insn_mf();
		ndelay(20);

		val = GPI_NAND_DATA(
			__gxio_mmio_read(gpio_ctx.mmio_base + GPIO_PIN_STATE));

		__gxio_mmio_write(gpio_ctx.mmio_base + GPIO_PIN_SET,
				  GPO_TILERA_NAND_nRE);

		if (data) {
			data[i] = val;
		}
		else if (verify) {
			if (verify[i] != val) {
				ret = -EFAULT;
			}
		}
	}
	__gxio_mmio_write(gpio_ctx.mmio_base + GPIO_PIN_RELEASE, 0);
	return ret;
}

static void tilera_hwcontrol(struct mtd_info *mtd, int cmd,
			     unsigned int ctrl) {
	if (ctrl & NAND_CTRL_CHANGE) {
		unsigned gpo = 0;
		if (!(ctrl & NAND_NCE)) {
			gpo |= GPO_TILERA_NAND_nCE;
		}
		if (ctrl & NAND_CLE) {
			gpo |= GPO_TILERA_NAND_CLE;
		}
		if (ctrl & NAND_ALE) {
			gpo |= GPO_TILERA_NAND_ALE;
		}
		gxio_gpio_set(&gpio_ctx, gpo,
			      GPO_TILERA_NAND_nCE |
			      GPO_TILERA_NAND_CLE |
			      GPO_TILERA_NAND_ALE);
		__insn_mf();
	}

	if (cmd != NAND_CMD_NONE) {
		uint8_t data = cmd;
		tilera_write_bytes_gpio(&data, 1);
	}
}

static uint8_t tilera_read_byte(struct mtd_info *mtd)
{
	uint8_t data;
	tilera_read_bytes_gpio(&data, 1, NULL);
	return data;
}

static void tilera_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	tilera_write_bytes_gpio(buf, len);
}

static void tilera_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	tilera_read_bytes_gpio(buf, len, NULL);
}

static int tilera_verify_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	return tilera_read_bytes_gpio(NULL, len, buf);
}

#ifdef DEBUG_SPEED
static void tilera_test_speed(void) {
	char buf[1024];
	unsigned long ticks;
	unsigned kb;

	/* wait for "start of" clock tick */
	kb = 0;
	ticks = jiffies;
	while (ticks == jiffies)
		/* nothing */;
	ticks = jiffies + HZ / 10;

	while ((long)(jiffies - ticks) < 0) {
		tilera_read_bytes_gpio(buf, 1024, NULL);
		++kb;
	}
	printk("read speed is %u KB/s\n", kb * 10);

	/* wait for "start of" clock tick */
	for (kb = 0; kb < sizeof(buf); ++kb) buf[kb] = kb;
	kb = 0;
	ticks = jiffies;
	while (ticks == jiffies)
		/* nothing */;
	ticks = jiffies + HZ / 10;

	while ((long)(jiffies - ticks) < 0) {
		tilera_write_bytes_gpio(buf, 1024);
		++kb;
	}
	printk("write speed is %u KB/s\n", kb * 10);
}
#endif

static int tilera_nand_probe(struct platform_device *pdev)
{
	memset(&rnand, 0, sizeof(rnand));

	rnand.cmd_ctrl = tilera_hwcontrol;
	rnand.dev_ready = tilera_dev_ready;
	rnand.read_byte = tilera_read_byte;
	rnand.write_buf = tilera_write_buf;
	rnand.read_buf = tilera_read_buf;
	rnand.verify_buf = tilera_verify_buf;

	return rb_nand_probe(&rnand, 1);
}

static int __init tilera_nand_gpio_init(void) {
	GPIO_GCLK_MODE_t gclk_mode;
	int src_mhz, div;
	int ret = gxio_gpio_init(&gpio_ctx, 0);
	if (ret) {
		printk("gxio_gpio_init() failed: %d\n", ret);
		return -ENOMEM;
	}
	ret = gxio_gpio_attach(&gpio_ctx,
			       GPO_TILERA_NAND_nCE |
			       GPO_TILERA_NAND_CLE |
			       GPO_TILERA_NAND_ALE |
			       GPO_TILERA_NAND_nWE |
			       GPO_TILERA_NAND_nRE |
			       GPI_TILERA_NAND_RDY |
			       GPO_NAND_DATA(0xff));
	if (ret) {
		printk("gxio_gpio_attach() failed: %d\n", ret);
		return -EIO;
	}
	ret = gxio_gpio_set_dir(&gpio_ctx,
				0,			// disabled
				GPI_TILERA_NAND_RDY,	// input
				GPO_NAND_DATA(0xff) |
				GPO_TILERA_NAND_nCE |
				GPO_TILERA_NAND_CLE |
				GPO_TILERA_NAND_ALE |
				GPO_TILERA_NAND_nWE |
				GPO_TILERA_NAND_nRE,	// output push
				0);			// output od
	if (ret) {
		printk("gxio_gpio_set_dir() failed: %d\n", ret);
		return -EIO;
	}
	gxio_gpio_set(&gpio_ctx,
		      GPO_TILERA_NAND_nCE |
		      GPO_TILERA_NAND_nWE |
		      GPO_TILERA_NAND_nRE,	// output 1
		      GPO_NAND_DATA(0xff) |
		      GPO_TILERA_NAND_nCE |
		      GPO_TILERA_NAND_CLE |
		      GPO_TILERA_NAND_ALE |
		      GPO_TILERA_NAND_nWE |
		      GPO_TILERA_NAND_nRE);	// output mask

	gxio_gpio_get_gclk_mode(&gpio_ctx, &gclk_mode);
#if 1
	gclk_mode.cmode = 1;		// gpio clock source is CPU / 2
	src_mhz = (get_clock_rate() / 1000000) / 2;
#else
	gclk_mode.cmode = 0;		// gpio clock source is REFCLK = 125 MHz
	src_mhz = 125;
#endif
	div = (src_mhz + 79) / 80;	// GPIO CLK <= 80 MHz
	gclk_mode.divide = div - 1;	// one gpio i/o takes >= 12.5ns
	gxio_gpio_set_gclk_mode(&gpio_ctx, gclk_mode);

#ifdef DEBUG_SPEED
	{
		int ns10 = div * 10 * 1000 / src_mhz;
		printk("gpio clk src %d MHz, div %d, clock cycle %u.%u ns\n",
		       src_mhz, div, ns10 / 10, ns10 % 10);
	}
	tilera_test_speed();
#endif
	return 0;
}

static int __init tilera_nand_init(void)
{
	int ret;
	int try = 0;
	printk("TILERA nand\n");

	ret = tilera_nand_gpio_init();
	if (ret) {
		if (ret == -EIO)
			gxio_gpio_destroy(&gpio_ctx);
		return -ENOMEM;
	}

	ret = tilera_nand_probe(NULL);
	if (ret) {
		printk("tilera_nand_probe() failed: %d (try %d)\n", ret, ++try);
		gxio_gpio_destroy(&gpio_ctx);
		return -EIO;
	}
	return 0;
}

module_init(tilera_nand_init);
