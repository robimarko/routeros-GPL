#include <linux/kernel.h>
#include <linux/module.h>	/* for EXPORT_SYMBOL() */
#include <linux/string.h>	/* for memcpy() */
#include <linux/hardirq.h>	/* for in_atomic() */
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/gpio.h>
#include <asm/bootinfo.h>
#include <asm/rb/boards.h>
#include <asm/rb/rb400.h>

static struct spi_device *spi = 0;
static struct spi_device *spi_ssr = 0;

struct spi_device *rb400_spi_get(void) {
	return spi;
}
EXPORT_SYMBOL(rb400_spi_get);

static void rb400_spi_async_complete(void *msg_buf) {
	kfree(msg_buf);
}

static void rb400_spi_write_async(struct spi_device *spi,
				  const uint8_t *buf, unsigned len, int ssr) {
	struct msg_async {
		struct spi_message m;
		struct spi_transfer t;
		uint8_t data[];
	};

	struct msg_async *ma;
		
	ma = kzalloc(sizeof(*ma) + len, GFP_KERNEL);
	if (ma == NULL) {
		printk(KERN_ERR "rb400_spi_write_async failed: OOM\n");
		return;
	}
	spi_message_init(&ma->m);
	spi_message_add_tail(&ma->t, &ma->m);

	ma->t.tx_buf = ma->data;
	ma->t.len = len;
	memcpy(ma->data, buf, len);

	ma->m.ssr_strobe = ssr;
	ma->m.context = ma;
	ma->m.complete = rb400_spi_async_complete;

	spi_async(spi, &ma->m);
}

void rb400_change_cfg(unsigned off, unsigned on) {
	static unsigned cfg = (CFG_BIT_nLED1 | CFG_BIT_nLED2 |
			       CFG_BIT_nLED3 | CFG_BIT_nLED4 |
			       CFG_BIT_nLED5);
	unsigned ocfg;
	unsigned ncfg;

	static DEFINE_SPINLOCK(lock);
	unsigned long flags;

	spin_lock_irqsave(&lock, flags);
	ocfg = cfg;
	cfg = (cfg & ~off) | on;
	ncfg = cfg;
	spin_unlock_irqrestore(&lock, flags);
	/*
	 * FIXME: it is possible to send CFG changes in wrong order
	 *	  in case if task switch happens here:
	 *		1) LED change gets interrupted here
	 *		2) NAND_ALE change is finished, it gets interrupted
	 *		3) LED change finishes, ALE change is lost at moment
	 *		4) NAND write is done with missing ALE!!!
	 */

	if ((ncfg ^ ocfg) & 0xff) {
		unsigned char buf[2] = { SPI_CMD_WRITE_CFG, ncfg };

		if (in_atomic())
			rb400_spi_write_async(spi, buf, 2, 0);
		else
			spi_write(spi, buf, 2);
	}
	if ((ncfg ^ ocfg) & CFG_BIT_nLED5) {
		uint8_t cmd = ((ncfg & CFG_BIT_nLED5) ?
			       SPI_CMD_LED5_OFF :
			       SPI_CMD_LED5_ON);
		rb400_spi_write_async(spi, &cmd, 1, 0);
	}
}
EXPORT_SYMBOL(rb400_change_cfg);

int rb_change_or_get_cfg(unsigned off, unsigned on, unsigned *get) {
	static unsigned cfg = -1u;
	unsigned ocfg;
	unsigned ncfg;
	struct rb400_ssr_info *ssr = NULL;
	unsigned changed = 0;

	static DEFINE_SPINLOCK(lock);
	unsigned long flags;

	if (spi_ssr) {
		ssr = (struct rb400_ssr_info *)spi_ssr->dev.platform_data;
	}
	if (ssr == NULL) {
		printk("ERROR: rb_change_cfg(%x,%x) without ssr!\n",
		       off, on);
		return -ENODEV;
	}

	if (cfg == -1u) {
		cfg = ssr->val;
		changed = 1;
	}
	spin_lock_irqsave(&lock, flags);
	ocfg = cfg;
	cfg = (cfg & ~off) | on;
	ncfg = cfg;
	spin_unlock_irqrestore(&lock, flags);

	if (get) {
		*get = ncfg;
		return 0;
	}
	if (ncfg != ocfg) {
		changed = 1;
	}
	if (((on | off) != 0) && !changed) {
		return -EEXIST;
	}
	{
		unsigned char buf[4] = { 5 /*SPI_CMD_RD_STATUS*/,
					 ncfg >> ((ssr->bytes - 1) * 8),
					 ncfg };
		unsigned char rxbuf[4];
		int len = ssr->bytes + 1;
		if (ssr->bytes == 3) {
			buf[0] = ncfg >> 16;
			buf[1] = ncfg >> 8;
			buf[2] = ncfg >> 0;
			len = 3;
		}
		if (!in_atomic()) {
			struct spi_message m;
			struct spi_transfer t = {
				.tx_buf = buf,
				.rx_buf = rxbuf,
				.len = len,
			};
			int res;

			spi_message_init(&m);
			spi_message_add_tail(&t, &m);
			m.ssr_strobe = ssr->strobe;
			res = spi_sync(spi_ssr, &m);
			if (res) return res;

			res = rxbuf[0];
			if (ssr->bytes > 1) res = (res << 8) | rxbuf[1];
			if (ssr->bytes > 2) res = (res << 8) | rxbuf[2];
//			printk("rb_change_cfg %02x, read %02x strobe %d\n",
//			       ncfg, res, ssr->strobe);
			return res;
		}
		else {
//			printk("rb_change_cfg async %02x\n", ncfg);
			rb400_spi_write_async(spi_ssr, buf, len, ssr->strobe);
			return -EAGAIN;
		}
	}
}
EXPORT_SYMBOL(rb_change_or_get_cfg);

int rb_change_cfg(unsigned off, unsigned on) {
	return rb_change_or_get_cfg(off, on, NULL);
}
EXPORT_SYMBOL(rb_change_cfg);

void rb400_exit_4byte_spi_addressing(void) {
	unsigned id = 0;
	uint8_t jedec_read_id[] = { 0x9f };
	if (spi == NULL) {
		return;
	}
	spi_write_then_read(spi, jedec_read_id, 1, &id, 4);
	if (id == 0xef401900) {
		uint8_t exit_4b[] = { 0xe9 };
		uint8_t wren[] = { 0x06 };
		uint8_t zero[] = { 0xc5, 0x00 };
		spi_write(spi, exit_4b, 1);
		spi_write(spi, wren, 1);
		spi_write(spi, zero, 2);
	}
}

int rb400_spiflash_read_verify(unsigned addr,
			       uint8_t *rdata, const uint8_t *vdata,
			       unsigned cnt, unsigned addr_size) {
	const uint8_t cmd[6] = { SPI_CMD_READ_FAST,
				 addr >> (addr_size * 8 -  8),
				 addr >> (addr_size * 8 - 16),
				 addr >> (addr_size * 8 - 24),
				 addr >> (addr_size * 8 - 32),
				 0
	};
	struct spi_transfer t[2] = {
		{
			.tx_buf = &cmd,
			.len = addr_size + 2,
		},
		{
			.tx_buf = vdata,
			.rx_buf = rdata,
			.len = cnt,
			.verify = (vdata != NULL),
		},
	};
	struct spi_message m;

	spi_message_init(&m);
	m.fast_read = 1;
	spi_message_add_tail(&t[0], &m);
	spi_message_add_tail(&t[1], &m);
	if (in_atomic()) {
	    return spi_sync_locked(spi, &m);
	}
	return spi_sync(spi, &m);
}
EXPORT_SYMBOL(rb400_spiflash_read_verify);

int rb400_spi_sync(struct spi_message *message) {
	if (in_atomic()) {
	    return spi_sync_locked(spi, message);
	}
	return spi_sync(spi, message);
}
EXPORT_SYMBOL(rb400_spi_sync);

static int ssr_gpio_get(struct gpio_chip *chip, unsigned offset) {
	unsigned val = 0;
	rb_change_or_get_cfg(0, 0, &val);
	return (val & BIT(offset)) != 0;
}

static void ssr_gpio_set(struct gpio_chip *chip, unsigned offset, int value) {
	if (value) {
		rb_change_or_get_cfg(0, BIT(offset), NULL);
	}
	else {
		rb_change_or_get_cfg(BIT(offset), 0, NULL);
	}
}

static struct gpio_chip ssr_gpio_chip = {
	.label			= "ssr-gpio",
	.set			= ssr_gpio_set,
	.get			= ssr_gpio_get,
	.base			= RB400_SSR_GPIO_BASE,
	.ngpio			= 8,
};

static int ccs_spi_ssr_probe(struct spi_device *_spi)
{
	spi_ssr = _spi;
	ssr_gpio_chip.ngpio =
		((struct rb400_ssr_info*)spi_ssr->dev.platform_data)->bytes * 8;
	gpiochip_add(&ssr_gpio_chip);
	return 0;
}

static int rb400_spi_misc_probe(struct spi_device *_spi)
{
	printk("RB400 spi misc\n");
	spi = _spi;
	if (spi->dev.platform_data) {
		ccs_spi_ssr_probe(_spi);
	}
	return 0;
}

static struct spi_driver rb400_spi_misc = {
	.driver	= {
		.name = "rb400-spi-misc",
		.owner = THIS_MODULE,
	},
	.probe	= rb400_spi_misc_probe,
};

static struct spi_driver ccs_spi_ssr = {
	.driver	= {
		.name = "ccs-spi-ssr",
		.owner = THIS_MODULE,
	},
	.probe	= ccs_spi_ssr_probe,
};

int __init rb400_spiboard_init(void) {
	spi_register_driver(&ccs_spi_ssr);
	return spi_register_driver(&rb400_spi_misc);
}

module_init(rb400_spiboard_init);
