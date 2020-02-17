#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/spi/spi.h>


static DEFINE_SPINLOCK(spi_gpio_lock);

struct gpio_spi_dev {
	struct gpio_chip gpio_chip;
	unsigned char last_written_val;
	unsigned char last_read_val;
	unsigned char recieve_buf;
	int nr_gpo;
	int nr_gpi;
	struct spi_device *spi;
};

#define GPO_BASE(dev)		(0)
#define GPI_BASE(dev)		((dev)->nr_gpo)
#define IS_GPO(dev, gpio)	(gpio > GPO_BASE(dev) && \
				gpio < GPI_BASE(dev))
#define IS_GPI(dev, gpio)	(gpio >= GPI_BASE(dev) && \
				(gpio < (GPI_BASE(dev) + (dev)->nr_gpi)))

#define LAST_WRITTEN_VAL(dev, gpio)	((((dev)->last_written_val) >> gpio) & 1)
#define LAST_READ_VAL(dev, gpio)	((((dev)->last_read_val) >> gpio) & 1)

static void gpio_spi_async_transfer_complete(void *msg_buf) {
	kfree(msg_buf);
}

static int write_spi_gpio(struct gpio_spi_dev *spi_gpio, unsigned char val)
{
	struct msg_async {
		struct spi_message m;
		struct spi_transfer t;
		uint8_t data[];
	};
	struct msg_async *ma;

	ma = kzalloc(sizeof(*ma) + 1, GFP_KERNEL);
	if (ma == NULL) {
		pr_err("gpio_spi_write_async failed\n");
		return - 1;
	}
	spi_message_init(&ma->m);
	spi_message_add_tail(&ma->t, &ma->m);

	ma->t.tx_buf = ma->data;
	ma->t.len = sizeof(unsigned char);
	memcpy(ma->data, &val, sizeof(unsigned char));

	ma->m.context = ma;
	ma->m.complete = gpio_spi_async_transfer_complete;
	spi_gpio->spi->mode &= ~SPI_NO_CS;

	return spi_async(spi_gpio->spi, &ma->m);

}

static unsigned read_spi_gpio(struct gpio_spi_dev *spi_gpio)
{
	struct msg_async {
		struct spi_message m;
		struct spi_transfer t;
		unsigned char cmd;
	};
	struct msg_async *ma;

	ma = kzalloc(sizeof(*ma) + 1, GFP_KERNEL);
	if (ma == NULL) {
		pr_err("gpio_spi_write_async failed\n");
		return - 1;
	}

	spi_message_init(&ma->m);
	spi_message_add_tail(&ma->t, &ma->m);

	ma->cmd = 0x7f;
	ma->t.tx_buf = &ma->cmd;
	ma->t.rx_buf = &spi_gpio->recieve_buf;
	ma->t.len = sizeof(unsigned char);

	ma->m.context = ma;
	ma->m.complete = gpio_spi_async_transfer_complete;
	spi_gpio->spi->mode |= SPI_NO_CS;

	return spi_async(spi_gpio->spi, &ma->m);

}

static int gpio_spi_get(struct gpio_chip *chip, unsigned gpio)
{
	struct gpio_spi_dev *gpio_spi = container_of(chip, struct gpio_spi_dev, gpio_chip);
	unsigned char val;
	unsigned long flags;

	spin_lock_irqsave(&spi_gpio_lock, flags);
	if (IS_GPO(gpio_spi, gpio)) {
		val = LAST_WRITTEN_VAL(gpio_spi, gpio);
	} else if (IS_GPI(gpio_spi, gpio)) {
		/* return value from previous read */
		gpio_spi->last_read_val = gpio_spi->recieve_buf;
		val = LAST_READ_VAL(gpio_spi, (gpio - gpio_spi->nr_gpo));
		read_spi_gpio(gpio_spi);
	}
	spin_unlock_irqrestore(&spi_gpio_lock, flags);

	return val;
}

static void gpio_spi_set(struct gpio_chip *chip, unsigned gpio, int val)
{
	struct gpio_spi_dev *gpio_spi = container_of(chip, struct gpio_spi_dev, gpio_chip);
	unsigned long flags;
	spin_lock_irqsave(&spi_gpio_lock, flags);
	if (IS_GPO(gpio_spi, gpio)) {

		if (val)
			gpio_spi->last_written_val |= 1 << gpio;
		else
			gpio_spi->last_written_val &= ~(1 << gpio);

		write_spi_gpio(gpio_spi, gpio_spi->last_written_val);
	}
	spin_unlock_irqrestore(&spi_gpio_lock, flags);
}

static int gpio_spi_direction_input(struct gpio_chip *chip, unsigned gpio)
{
	struct gpio_spi_dev *gpio_spi = container_of(chip, struct gpio_spi_dev, gpio_chip);
	if (IS_GPI(gpio_spi, gpio))
		return 0;
	return -EIO;
}

static int gpio_spi_direction_output(struct gpio_chip *chip, unsigned gpio, int val)
{
	struct gpio_spi_dev *gpio_spi = container_of(chip, struct gpio_spi_dev, gpio_chip);
	if (IS_GPO(gpio_spi, gpio))
		return 0;
	return -EIO;
}

static struct gpio_spi_dev gpio_spi = {
	.gpio_chip = {
		.label			= "gpio-spi",
		.base			= -1,
		.direction_input	= gpio_spi_direction_input,
		.direction_output	= gpio_spi_direction_output,
		.get			= gpio_spi_get,
		.set			= gpio_spi_set,
	},
};

static int gpio_spi_probe(struct spi_device *spi)
{
	struct device_node *np = spi->dev.of_node;
	struct timespec tmspc;
	int ret, timer, nr_gpo, nr_gpi;

#ifndef CONFIG_OF_GPIO
	return -EINVAL;
#else
	if (!np) {
		pr_err("no dt node found\n");
		return -ENODEV;
	}

	if (of_property_read_u32(np, "nr-gpo", &nr_gpo)) {
		pr_err("failed to get number of output pins(nr-gpo)\n");
		return -EINVAL;
	}
	if (of_property_read_u32(np, "nr-gpi", &nr_gpi)) {
		pr_err("failed to get snumber of input pins(nr-gpi)\n");
		return -EINVAL;
	}
#endif
	gpio_spi.spi = spi;
	gpio_spi.nr_gpo = nr_gpo;
	gpio_spi.nr_gpi = nr_gpi;
	gpio_spi.gpio_chip.ngpio = gpio_spi.nr_gpo + gpio_spi.nr_gpi;
	gpio_spi.gpio_chip.of_node = np;

	ret = gpiochip_add(&gpio_spi.gpio_chip);
	if (ret < 0)
		return ret;

	return 0;
}

static int gpio_spi_remove(struct spi_device *spi)
{
	int ret = gpiochip_remove(&gpio_spi.gpio_chip);
	if (ret < 0) {
		pr_err("Failed with error: %d\n", ret);
		return ret;
	}
	return 0;
}

#ifdef CONFIG_OF_GPIO
static const struct of_device_id gpio_spi_of_match[] = {
	{ .compatible = "gpio-spi", },
	{},
};
#endif

static struct spi_driver gpio_spi_driver = {
	.probe	= gpio_spi_probe,
	.remove	= gpio_spi_remove,
	.driver	= {
		.name = "gpio-spi",
		.owner = THIS_MODULE,
#ifdef CONFIG_OF_GPIO
		.of_match_table = gpio_spi_of_match,
#endif
	},
};

static int gpio_spi_init(void) {
	return spi_register_driver(&gpio_spi_driver);
}

static void gpio_spi_exit(void) {
	spi_unregister_driver(&gpio_spi_driver);
}

module_init(gpio_spi_init);
module_exit(gpio_spi_exit);
