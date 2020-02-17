#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/spi/mmc_spi.h>
#include <linux/spi/spi.h>
#include <linux/mmc/host.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <asm/io.h>

#define SD_DEBUG 0

#define GPIO_BASE_16_23	0xfd889000
#define GPO_SCK		237	/* PBS[21] */
#define    SCK_0		0
#define    SCK_1		(1 << (21 - 16))
#define GPI_MISO	238	/* PBS[22] */
#define    MISO_1		(1 << (22 - 16))
#define GPO_MOSI	239	/* PBS[23] */
#define    MOSI_0		(0 << (23 - 16))
#define    MOSI_1		(1 << (23 - 16))
#define GPO_nCS		(224 + 1)	/* PBS[25] */
#define GPIO_SD_PWR_EN	(224 + 2)	/* PSB[26] */
#define GPIO_SD_CD	(224 + 3)	/* PSB[27] */

struct alpine_spi {
	int			cs_wait;
};
static volatile unsigned *sck = 0;
static volatile unsigned *miso = 0;
static volatile unsigned *mosi_sck = 0;

static unsigned ns_delay = 0;

static void set_speed(unsigned hz) {
	// one gpio cycle takes >= 66.7 ns
	if (hz > 0 && hz < 7500000) {
		ns_delay = 500000000 / hz - 66;
	}
	else {
		ns_delay = 0;
	}
#if SD_DEBUG
	printk("hz %u -> %u ns\n", hz, ns_delay);
#endif
}

static void inline do_spi_init(struct alpine_spi *rbspi,
			       struct spi_device *spi) {
	// prepare for transaction, make correct CS low
	if (!(spi->mode & SPI_CS_HIGH)) {
		gpio_set_value(GPO_nCS, 0);
	}
#if SD_DEBUG
	printk("do_spi_init %d\n", spi->chip_select);
#endif
}

static void spi_cs_terminate(struct alpine_spi *rbspi) {
	// finish transaction, make all CS high
	gpio_set_value(GPO_nCS, 1);
#if SD_DEBUG
	printk("spi_cs_terminate\n");
#endif
}

static inline void hclk_delay(void) {
	if (ns_delay) {
		ndelay(ns_delay);
	}
}

static unsigned char spi_txrx8(unsigned char data, int rx) {
	unsigned char res = 0;
	int i;
	for (i = 7; i >= 0; --i) {
		*mosi_sck = ((data >> i) & 1) ? MOSI_1 : MOSI_0;;
		hclk_delay();
		*sck = SCK_1;
		hclk_delay();
		if (rx) {
			res <<= 1;
			if (MISO_1 & *miso) res |= 1;
		}
	}
	return res;
}

static int alpine_spi_txrx(struct spi_transfer *t)
{
	const unsigned char *tx_ptr = t->tx_buf;
	unsigned char *rx_ptr = t->rx_buf;
	unsigned i;

#if SD_DEBUG
	printk("spi_txrx len %u tx %u rx %u\n",
	       t->len,
	       (t->tx_buf ? 1 : 0),
	       (t->rx_buf ? 1 : 0));
#endif

        // Give time for CS setup needed by LCD
        hclk_delay();

	for (i = 0; i < t->len; ++i) {
		unsigned char txdata = tx_ptr ? tx_ptr[i] : 0;
		if (rx_ptr) {
			rx_ptr[i] = spi_txrx8(txdata, 1);
		}
		else {
			spi_txrx8(txdata, 0);
		}
	}
	return i;
}

static int alpine_spi_msg(struct alpine_spi *rbspi,
			 struct spi_message *m, int cs) {
	struct spi_transfer *t = NULL;
	int cs_terminate = cs < 0 ? 1 : 0;

	m->status = 0;
	m->actual_length = 0;
	if (list_empty(&m->transfers)) {
		if (cs >= 0) {
			spi_cs_terminate(rbspi);
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
		set_speed(hz);

		len = alpine_spi_txrx(t);
		if (len != t->len) {
			m->status = -EMSGSIZE;
			spi_cs_terminate(rbspi);
			return -1;
		}
		m->actual_length += len;

		if (cs_terminate) {
			spi_cs_terminate(rbspi);
		}
	}

	return cs_terminate ? -1 : m->spi->chip_select;
}

static int alpine_spi_transfer(struct spi_device *spi,
			      struct spi_message *m) {
	struct alpine_spi *rbspi = spi_master_get_devdata(spi->master);

	rbspi->cs_wait = alpine_spi_msg(rbspi, m, rbspi->cs_wait);
	m->complete(m->context);
	return 0;
}

static int alpine_spi_setup(struct spi_device *spi)
{
	return 0;
}
#if SD_DEBUG
static void gpio_test_speed(void) {
	unsigned long ticks;
	unsigned kb;

	/* wait for "start of" clock tick */
	kb = 0;
	ticks = jiffies;
	while (ticks == jiffies)
		/* nothing */;
	ticks = jiffies + HZ / 10;

	while ((long)(jiffies - ticks) < 0) {
	    int i;
	    for (i = 0; i < 1024 / 8; ++i) {
		    *miso;
		    *miso;
		    *miso;
		    *miso;
		    *miso;
		    *miso;
		    *miso;
		    *miso;
	    }
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
	    int i;
	    for (i = 0; i < 1024 / 8; ++i) {
		*sck = SCK_0;
		*sck = SCK_1;
		*sck = SCK_0;
		*sck = SCK_1;
		*sck = SCK_0;
		*sck = SCK_1;
		*sck = SCK_0;
		*sck = SCK_1;
	    }
		++kb;
	}
	printk("write speed is %u KB/s\n", kb * 10);
}
#endif
static int alpine_spi_gpio_init(void) {
	unsigned *gpio_base_16_23;
	if (gpio_request(GPO_MOSI, "mmc_spi:mosi") ||
	    gpio_request(GPO_nCS, "mmc_spi:ncs") ||
	    gpio_request(GPO_SCK, "mmc_spi:sck") ||
	    gpio_request(GPI_MISO, "mmc_spi:miso") ||
	    gpio_request(GPIO_SD_PWR_EN, "mmc_spi:pwr_en") ||
	    gpio_request(GPIO_SD_CD, "mmc_spi:cd")) {
		printk("Alpine SD gpio request failed\n");
		return -EIO;
	}
	gpio_direction_output(GPIO_SD_PWR_EN, 0);
	gpio_direction_output(GPO_MOSI, 0);
	gpio_direction_output(GPO_SCK, 0);
	gpio_direction_output(GPO_nCS, 1);
	gpio_direction_input(GPI_MISO);
	gpio_direction_input(GPIO_SD_CD);

	gpio_base_16_23 = ioremap(GPIO_BASE_16_23, 0x1000);
	if (gpio_base_16_23 == 0) {
		return -EIO;
	}
	sck = (gpio_base_16_23 + (1 << (21 - 16)));
	miso = (gpio_base_16_23 + (1 << (22 - 16)));
	mosi_sck = (gpio_base_16_23 + (1 << (23 - 16)) + (1 << (21 - 16)));

#if SD_DEBUG
	gpio_test_speed();
#endif
	return 0;
}

static int alpine_sd_get_cd(struct device *pdev) {
	int cd = !gpio_get_value(GPIO_SD_CD);
#if SD_DEBUG
	printk("alpine sd cd %d\n", cd);
#endif
	return cd;
}
static void alpine_sd_setpower(struct device *pdev, unsigned int maskval) {
#if SD_DEBUG
	printk("alpine sd setpower %u\n", maskval);
#endif
	gpio_set_value(GPIO_SD_PWR_EN, maskval ? 1 : 0);
}
static struct mmc_spi_platform_data alpine_spi_sd_data = {
	.get_cd		= alpine_sd_get_cd,
	.caps		= MMC_CAP_NEEDS_POLL,
	.ocr_mask	= MMC_VDD_32_33 | MMC_VDD_33_34,
	.setpower	= alpine_sd_setpower,
};
static struct spi_board_info alpine_spi_sd = {
	.modalias = "mmc_spi",
	.max_speed_hz = 50 * 1000 * 1000,
	.bus_num = 4,
	.chip_select = 0,
	.mode = SPI_MODE_0,
	.platform_data = &alpine_spi_sd_data,
	.controller_data = (void *) GPO_nCS,
};

static int alpine_sd_probe(struct platform_device *pdev)
{
	struct spi_master *master;
	struct alpine_spi *rbspi;
	int err = 0;

	printk("Alpine SD SPI\n");
	err = alpine_spi_gpio_init();
	if (err) {
		return err;
	}

	master = spi_alloc_master(&pdev->dev, sizeof(*rbspi));
	if (master == NULL) {
		dev_err(&pdev->dev, "No memory for spi_master\n");
		return -ENOMEM;
	}
	master->bus_num = 4;
	master->num_chipselect = 1;
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH;
	master->setup    = alpine_spi_setup;
	master->transfer = alpine_spi_transfer;

	rbspi = spi_master_get_devdata(master);
	memset(rbspi, 0, sizeof(*rbspi));
	rbspi->cs_wait = -1;

	err = spi_register_master(master);
	if (err) {
		dev_err(&pdev->dev, "Failed to register SPI master\n");
		spi_master_put(master);
		return err;
	}
	spi_new_device(master, &alpine_spi_sd);
	return 0;
}

static struct platform_driver alpine_sd_drv = {
	.probe		= alpine_sd_probe,
        .driver		= {
		.name	= "alpine-sd",
		.owner	= THIS_MODULE,
        },
};

static int __init alpine_sd_init(void)
{
        return platform_driver_register(&alpine_sd_drv);
}

module_init(alpine_sd_init);
