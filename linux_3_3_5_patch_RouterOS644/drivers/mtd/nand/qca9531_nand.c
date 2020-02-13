#include <linux/init.h>
#include <linux/mtd/nand.h>
#include <linux/platform_device.h>
#include <linux/module.h>

#define DEBUG_SPEED	0

extern int rb_nand_probe(struct nand_chip *nand, int booter);

static struct nand_chip rnand;
static DEFINE_SPINLOCK(lock);

#define GPIO_BASE	0x18040000
#define GPIO_IE_REG	    0x0000
#define GPIO_IN_REG	    0x0004
#define GPIO_O1_REG	    0x0008
#define GPIO_S1_REG	    0x000c
#define GPIO_S0_REG	    0x0010

#define GPIO_REG(x)	(*(volatile unsigned *)((unsigned) gpio_base + (x)))

static void __iomem *gpio_base;

/* ------------------ QCA9531 nand driver -------------------- */

#define GPO_NAND_nCE		(1 << 13)
#define GPO_NAND_READ		(1 << 3)
#define GPO_NAND_ALE		(1 << 15)
#define GPO_NAND_CLE		(1 << 14)
#define GPIO_NAND_RDY		(1 << 4)
#define GPO_NAND_nRW		(1 << 12)
#define GPO_LATCH_EN		(1 << 11)

#define GPO_NAND_DATA(x) (((x) & 0x1f) | (((x) & 0xe0) << 8))
#define GPI_NAND_DATA(x) (((x) & 0x1f) | (((x) >> 8) & 0xe0))

static int qca9531_dev_ready(struct mtd_info *mtd) {
	return GPIO_REG(GPIO_IN_REG) & GPIO_NAND_RDY;
}

static void qca9531_write_bytes_gpio(const uint8_t *data, unsigned cnt) {
	int i;
	unsigned gpo_orig;
	unsigned long flags;
	unsigned s1_prev = 0xffffffff;

	spin_lock_irqsave(&lock, flags);

	GPIO_REG(GPIO_S0_REG) = GPO_LATCH_EN;
	gpo_orig = GPIO_REG(GPIO_O1_REG);
	GPIO_REG(GPIO_IE_REG) &= ~GPO_NAND_DATA(0xff);

	for (i = 0; i < cnt; ++i) {
		unsigned s1 = GPO_NAND_DATA(data[i]);
		unsigned s0 = s1 ^ GPO_NAND_DATA(0xff);

		GPIO_REG(GPIO_S0_REG) = s0 | GPO_NAND_nRW;
		if (s1_prev != s1) {
			GPIO_REG(GPIO_S1_REG) = s1;
			s1_prev = s1;
		}
		GPIO_REG(GPIO_S1_REG) = GPO_NAND_nRW;
	}

	GPIO_REG(GPIO_IE_REG) |= GPIO_NAND_RDY;
	GPIO_REG(GPIO_S1_REG) = gpo_orig & GPO_NAND_DATA(0xff);
	GPIO_REG(GPIO_S0_REG) = (~gpo_orig) & GPO_NAND_DATA(0xff);
	GPIO_REG(GPIO_S1_REG) = GPO_LATCH_EN;
	spin_unlock_irqrestore(&lock, flags);
}

static int qca9531_read_bytes_gpio(uint8_t *data, unsigned cnt,
				  const uint8_t *verify) {

	int i;
	int ret = 0;
	unsigned gpo_orig;
	unsigned long flags;

	GPIO_REG(GPIO_S1_REG) = GPO_NAND_READ;
	GPIO_REG(GPIO_O1_REG);
	spin_lock_irqsave(&lock, flags);
	
	GPIO_REG(GPIO_S0_REG) = GPO_LATCH_EN;
	gpo_orig = GPIO_REG(GPIO_O1_REG) & ~GPO_NAND_READ;
	GPIO_REG(GPIO_IE_REG) |= GPO_NAND_DATA(0xff);
	
	for (i = 0; i < cnt; ++i) {
		unsigned char val;
		unsigned gpi;
		GPIO_REG(GPIO_S0_REG) = GPO_NAND_nRW;
		GPIO_REG(GPIO_O1_REG);
		gpi = GPIO_REG(GPIO_IN_REG);
		GPIO_REG(GPIO_S1_REG) = GPO_NAND_nRW;
		val = GPI_NAND_DATA(gpi);
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

	GPIO_REG(GPIO_IE_REG) &= ~(GPO_NAND_DATA(0xff) ^ GPIO_NAND_RDY);
	GPIO_REG(GPIO_S1_REG) = gpo_orig & GPO_NAND_DATA(0xff);
	GPIO_REG(GPIO_S0_REG) = (~gpo_orig) & GPO_NAND_DATA(0xff);
	GPIO_REG(GPIO_S1_REG) = GPO_LATCH_EN;

	spin_unlock_irqrestore(&lock, flags);
	return ret;
}

static void qca9531_hwcontrol(struct mtd_info *mtd, int cmd,
			     unsigned int ctrl) {
	if (ctrl & NAND_CTRL_CHANGE) {
		unsigned s0 = 0;

		if (!(ctrl & NAND_CLE)) {
			s0 |= GPO_NAND_CLE;
		}
		if (!(ctrl & NAND_ALE)) {
			s0 |= GPO_NAND_ALE;
		}
		if (ctrl & NAND_NCE) {
			s0 |= GPO_NAND_nCE;
		}
		GPIO_REG(GPIO_S0_REG) = s0;
		GPIO_REG(GPIO_S1_REG) = s0 ^ (GPO_NAND_CLE |
					      GPO_NAND_ALE |
					      GPO_NAND_nCE);
		GPIO_REG(GPIO_O1_REG);		/* flush */
	}

	if (cmd != NAND_CMD_NONE) {
		uint8_t data = cmd;
		qca9531_write_bytes_gpio(&data, 1);
	}
}

static uint8_t qca9531_read_byte(struct mtd_info *mtd)
{
	uint8_t data;
	qca9531_read_bytes_gpio(&data, 1, NULL);
	return data;
}

static void qca9531_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	qca9531_write_bytes_gpio(buf, len);
}

static void qca9531_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	qca9531_read_bytes_gpio(buf, len, NULL);
}

static int qca9531_verify_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	return qca9531_read_bytes_gpio(NULL, len, buf);
}

#if DEBUG_SPEED
static void qca9531_test_speed(void) {
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
		qca9531_read_bytes_gpio(buf, 1024, NULL);
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
		qca9531_write_bytes_gpio(buf, 1024);
		++kb;
	}
	printk("write speed is %u KB/s\n", kb * 10);
}
#endif

static int qca9531_nand_probe(struct platform_device *pdev)
{
	printk("QCA9531 nand\n");
#if DEBUG_SPEED
	qca9531_test_speed();
#endif
	memset(&rnand, 0, sizeof(rnand));

	rnand.cmd_ctrl = qca9531_hwcontrol;
	rnand.dev_ready = qca9531_dev_ready;
	rnand.read_byte = qca9531_read_byte;
	rnand.write_buf = qca9531_write_buf;
	rnand.read_buf = qca9531_read_buf;
	rnand.verify_buf = qca9531_verify_buf;

	return rb_nand_probe(&rnand, 1);
}

static struct platform_driver qca9531_nand_driver = {
	.probe	= qca9531_nand_probe,
	.driver	= {
		.name = "qca9531-nand",
		.owner = THIS_MODULE,
	},
};

/* ------------------ common init/exit code -------------------- */

static int __init qca9531_nand_init(void)
{
	gpio_base = ioremap_nocache(GPIO_BASE, PAGE_SIZE);
	if (!gpio_base)
		return -ENOMEM;

	return platform_driver_register(&qca9531_nand_driver);
}

static void __exit qca9531_nand_exit(void)
{
	iounmap(gpio_base);

	platform_driver_unregister(&qca9531_nand_driver);
}

module_init(qca9531_nand_init);
module_exit(qca9531_nand_exit);
