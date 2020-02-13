#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <gxio/gpio.h>

#include <linux/spi/spi.h>
#include <linux/delay.h>

#define TILERA_DEBUG 0

#define   GPO_TS_nCS	(1ULL << 0)
//#define   GPI_TS_IRQ	(1ULL << 1)
#define   SHIFT_TS_SOUT	2
#define   GPI_TS_SOUT	(1ULL << 2)
//#define   GPO_LCD_LED	(1ULL << 3)
#define   GPO_LCD_nCS	(1ULL << 4)
#define   GPO_LCD_DCX	(1ULL << 5)
#define   GPO_LCD_SCK	(1ULL << 6)
#define   SHIFT_LCD_SIN		7
#define   GPIO_LCD_SIN	(1ULL << 7)

struct tilera_spi {
	spinlock_t		lock;
	struct list_head	queue;
	struct work_struct	wq;
	int			busy:1;
	int			cs_wait;
};

static gxio_gpio_context_t	gpio_ctx;
static unsigned ns_delay = 0;
static unsigned gpio_delay = 0;

static void set_speed(unsigned hz) {
	// one gpio cycle takes >= 12.5 ns
	if (hz > 0 && hz < 40000000) {
		ns_delay = 500000000 / hz - 12;
		if (ns_delay < 48) {
			gpio_delay = (ns_delay + 11) / 12;
			ns_delay = 0;
		}
		else {
			gpio_delay = 0;
		}
	}
	else {
		ns_delay = 0;
		gpio_delay = 0;
	}
#if TILERA_DEBUG
	printk("hz %u -> %u ns, gpio_delay %d\n", hz, ns_delay, gpio_delay);
#endif
}

static void inline do_spi_init(struct tilera_spi *rbspi,
			       struct spi_device *spi) {
	// prepare for transaction, make correct CS low
	unsigned cs = spi->chip_select ? GPO_TS_nCS : GPO_LCD_nCS;
	gxio_gpio_set(&gpio_ctx, 0, cs);
#if TILERA_DEBUG
	printk("do_spi_init %d\n", spi->chip_select);
#endif
}

static void spi_cs_terminate(struct tilera_spi *rbspi) {
	// finish transaction, make all CS high
	gxio_gpio_set(&gpio_ctx,
		      GPO_LCD_nCS | GPO_TS_nCS,
		      GPO_LCD_nCS | GPO_TS_nCS);
#if TILERA_DEBUG
	printk("spi_cs_terminate\n");
#endif
}

static inline void hclk_delay(void) {
	int i;
	for (i = 0; i < gpio_delay; ++i) {
		gxio_gpio_set(&gpio_ctx, 0, 0);
	}
	if (ns_delay) {
		__insn_mf();
		ndelay(ns_delay);
	}
}

static void spi_tx8(unsigned char data) {
	int i;
	for (i = 7; i >= 0; --i) {
		unsigned bit = ((data >> i) & 1) ? GPIO_LCD_SIN : 0;
		gxio_gpio_set(&gpio_ctx, bit, GPO_LCD_SCK | GPIO_LCD_SIN);
		hclk_delay();
		gxio_gpio_toggle(&gpio_ctx, GPO_LCD_SCK);
		hclk_delay();
	}
}

static unsigned char spi_txrx8(unsigned char data, unsigned gpi_shift) {
	unsigned char res = 0;
	int i;
	for (i = 7; i >= 0; --i) {
		unsigned bit = ((data >> i) & 1) ? GPIO_LCD_SIN : 0;
		gxio_gpio_set(&gpio_ctx, bit, GPO_LCD_SCK | GPIO_LCD_SIN);
		hclk_delay();
		gxio_gpio_toggle(&gpio_ctx, GPO_LCD_SCK);
		hclk_delay();
		res <<= 1;
		res |= (gxio_gpio_get(&gpio_ctx) >> gpi_shift) & 1;
	}
	return res;
}

static int tilera_spi_txrx(struct spi_transfer *t)
{
	const unsigned char *rxv_ptr = NULL;
	const unsigned char *tx_ptr = t->tx_buf;
	unsigned char *rx_ptr = t->rx_buf;
	unsigned i;
	unsigned gpi_shift = SHIFT_TS_SOUT;

#if TILERA_DEBUG
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
		gxio_gpio_set(&gpio_ctx, 0, GPO_LCD_DCX);
	}
	if (t->lcd_read) {
		gxio_gpio_set_dir(&gpio_ctx, 0, GPIO_LCD_SIN, 0, 0);
		gpi_shift = SHIFT_LCD_SIN;
	}

        // Give time for CS setup needed by LCD
        hclk_delay();

	for (i = 0; i < t->len; ++i) {
		unsigned char txdata = tx_ptr ? tx_ptr[i] : 0;
		if (rx_ptr) {
			rx_ptr[i] = spi_txrx8(txdata, gpi_shift);
		}
		else if (rxv_ptr) {
			if (rxv_ptr[i] != spi_txrx8(txdata, gpi_shift)) break;
		}
		else {
			spi_tx8(txdata);
		}
	}
	if (t->lcd_write_cmd) {
		gxio_gpio_set(&gpio_ctx, GPO_LCD_DCX, GPO_LCD_DCX);
	}
	if (t->lcd_read) {
		gxio_gpio_set_dir(&gpio_ctx, 0, 0, GPIO_LCD_SIN, 0);
	}
	return i;
}

static int tilera_spi_msg(struct tilera_spi *rbspi,
			 struct spi_message *m, int cs) {
	struct spi_transfer *t = NULL;
	int cs_terminate = cs < 0 ? 1 : 0;

	m->status = 0;
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

		len = tilera_spi_txrx(t);
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

static void tilera_spi_process_queue(struct work_struct *wq) {
	struct tilera_spi *rbspi = container_of(wq, struct tilera_spi, wq);
	unsigned long flags;
	int cs;

	spin_lock_irqsave(&rbspi->lock, flags);
	if (rbspi->busy) {
		// BUG: some other workqueue is already processing queue
		goto out;
	}
	cs = rbspi->cs_wait;
	rbspi->busy = 1;
	while (!list_empty(&rbspi->queue)) {
		struct spi_message *m;
		list_for_each_entry(m, &rbspi->queue, queue) {
			if (cs < 0 || cs == m->spi->chip_select) break;
		}
		if (&m->queue == &rbspi->queue) break;

		list_del_init(&m->queue);
		spin_unlock_irqrestore(&rbspi->lock, flags);

		cs = tilera_spi_msg(rbspi, m, cs);
		m->complete(m->context);

		spin_lock_irqsave(&rbspi->lock, flags);
	}
	rbspi->cs_wait = cs;
	rbspi->busy = 0;
	if (cs >= 0) {
		// TODO: add timer to unlock cs after 1s inactivity
	}
out:
	spin_unlock_irqrestore(&rbspi->lock, flags);
}

static int tilera_spi_transfer(struct spi_device *spi,
			      struct spi_message *m) {
	struct tilera_spi *rbspi = spi_master_get_devdata(spi->master);
	unsigned long flags;

	m->actual_length = 0;
	m->status = -EINPROGRESS;

	spin_lock_irqsave(&rbspi->lock, flags);
	list_add_tail(&m->queue, &rbspi->queue);
	if (!rbspi->busy) {
		queue_work(system_unbound_wq, &rbspi->wq);
	}
	spin_unlock_irqrestore(&rbspi->lock, flags);

	return 0;
}

static int tilera_spi_setup(struct spi_device *spi)
{
	struct tilera_spi *rbspi = spi_master_get_devdata(spi->master);
	unsigned long flags;

	if (spi->mode & ~(SPI_CS_HIGH)) {
		printk("TILERA SPI: mode %x not supported\n",
		       (unsigned)spi->mode);
		return -EINVAL;
	}
	if (spi->bits_per_word != 8 && spi->bits_per_word != 0) {
		printk("TILERA SPI: bits_per_word %u not supported\n",
		       (unsigned)spi->bits_per_word);
		return -EINVAL;
	}

	spin_lock_irqsave(&rbspi->lock, flags);
	if (rbspi->cs_wait == spi->chip_select && !rbspi->busy) {
		rbspi->cs_wait = -1;
		queue_work(system_unbound_wq, &rbspi->wq);
	}
	spin_unlock_irqrestore(&rbspi->lock, flags);
	return 0;
}

static int tilera_spi_gpio_init(void) {
	int ret = gxio_gpio_init(&gpio_ctx, 0);
	if (ret) {
		printk("gxio_gpio_init() failed: %d\n", ret);
		return -ENOMEM;
	}
	ret = gxio_gpio_attach(&gpio_ctx,
			       GPO_TS_nCS |
			       GPI_TS_SOUT |
			       GPO_LCD_nCS |
			       GPO_LCD_DCX |
			       GPO_LCD_SCK |
			       GPIO_LCD_SIN);
	if (ret) {
		printk("gxio_gpio_attach() failed: %d\n", ret);
		return -EIO;
	}
	ret = gxio_gpio_set_dir(&gpio_ctx,
				0,			// disabled
				GPI_TS_SOUT,		// input
				GPO_TS_nCS |
				GPO_LCD_nCS |
				GPO_LCD_DCX |
				GPO_LCD_SCK |
				GPIO_LCD_SIN,		// output push
				0);			// output od
	if (ret) {
		printk("gxio_gpio_set_dir() failed: %d\n", ret);
		return -EIO;
	}
	gxio_gpio_set(&gpio_ctx,
		      GPO_TS_nCS |
		      GPO_LCD_nCS |
		      GPO_LCD_DCX,	// output 1
		      GPO_TS_nCS |
		      GPO_LCD_nCS |
		      GPO_LCD_DCX |
		      GPO_LCD_SCK |
		      GPIO_LCD_SIN);	// output mask
	return 0;
}

static int tilera_spi_probe(struct platform_device *pdev)
{
	struct spi_master *master;
	struct tilera_spi *rbspi;
	struct spi_board_info spi_lcd = {
		.modalias = "rb2011-spi-lcd",
		.max_speed_hz = 15 * 1000 * 1000,
		.bus_num = 0,
		.chip_select = 0,
		.mode = SPI_MODE_0,
	};
	struct spi_board_info spi_ts = {
		.modalias = "rb2011-spi-ts",
		.max_speed_hz = 2000 * 1000,
		.bus_num = 0,
		.chip_select = 1,
		.mode = SPI_MODE_0,
	};
	int err = 0;

	printk("Tilera SPI\n");
	err = tilera_spi_gpio_init();
	if (err) {
		if (err != ENOMEM) goto err_nomem;
		return err;
	}

	master = spi_alloc_master(&pdev->dev, sizeof(*rbspi));
	if (master == NULL) {
		dev_err(&pdev->dev, "No memory for spi_master\n");
		err = -ENOMEM;
		goto err_nomem;
	}
	master->bus_num = 0;
	master->num_chipselect = 2;
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH;
	master->setup    = tilera_spi_setup;
	master->transfer = tilera_spi_transfer;

	rbspi = spi_master_get_devdata(master);
	memset(rbspi, 0, sizeof(*rbspi));
	rbspi->cs_wait = -1;

	spin_lock_init(&rbspi->lock);
	INIT_LIST_HEAD(&rbspi->queue);
	INIT_WORK(&rbspi->wq, tilera_spi_process_queue);

	err = spi_register_master(master);
	if (err) {
		dev_err(&pdev->dev, "Failed to register SPI master\n");
		goto err_register;
	}
	spi_new_device(master, &spi_lcd);
	spi_new_device(master, &spi_ts);
	return 0;

err_register:
	spi_master_put(master);;
err_nomem:
	gxio_gpio_destroy(&gpio_ctx);
	return err;
}

static struct platform_driver tilera_spi_drv = {
	.probe		= tilera_spi_probe,
        .driver		= {
		.name	= "tilera-gpio-spi",
		.owner	= THIS_MODULE,
        },
};

static int __init tilera_spi_init(void)
{
	platform_device_register_simple("tilera-gpio-spi", 0, NULL, 0);
        return platform_driver_register(&tilera_spi_drv);
}

module_init(tilera_spi_init);
