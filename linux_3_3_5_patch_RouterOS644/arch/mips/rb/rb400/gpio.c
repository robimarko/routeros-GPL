#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <asm/rb/rb400.h>

#define GPIO_BASE	0x18040000
#define GPIO_BASE2	0x18080000
#define GPIO_OE_REG	    0x0000
#define GPIO_IN_REG	    0x0004
#define GPIO_OUT_REG	    0x0008
#define GPIO_SET_REG	    0x000c
#define GPIO_CLEAR_REG	    0x0010

static void __iomem *gpio_base;

static void gpio_writel(unsigned val, unsigned reg) {
	rb400_writel(val, (unsigned) gpio_base + reg);
}

static unsigned gpio_readl(unsigned reg) {
	return rb400_readl((unsigned) gpio_base + reg);
}

static void gpio_rmw(unsigned reg, unsigned off, unsigned on) {
	unsigned long flags;
	unsigned val;
	local_irq_save(flags);
	val = gpio_readl(reg);
//	printk("rmw %08x %08x->%08x\n", reg, val, (val | on) & ~off);
	val = (val | on) & ~off;
	gpio_writel(val, reg);
	local_irq_restore(flags);
}


static int rb400_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	unsigned mask = BIT(offset);
	int val = !!(gpio_readl(GPIO_IN_REG) & mask);
//	printk("rb400_gpio_get %u(%u) %d %08x\n",
//	       offset, offset, val, gpio_readl(GPIO_IN_REG));
	return val;
}

static void rb400_gpio_set(struct gpio_chip *chip,
				unsigned offset, int value)
{
	unsigned mask = BIT(offset);
//	printk("rb400_gpio_set %u(%u) %d\n", offset, offset, value);
	gpio_writel(mask, value ? GPIO_SET_REG : GPIO_CLEAR_REG);
//	printk("set ok\n");
}

static int rb400_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	unsigned mask = BIT(offset);
//	printk("rb400_gpio_direction_input %u(%u)\n", offset, offset);
	gpio_rmw(GPIO_OE_REG, mask, 0);
	return 0;
}

static int rb400_gpio_direction_output(struct gpio_chip *chip,
					unsigned offset, int value)
{
	unsigned mask = BIT(offset);
//	printk("rb400_gpio_direction_output %u(%u) %d\n",
//	       offset, offset, value);
	gpio_writel(mask, value ? GPIO_SET_REG : GPIO_CLEAR_REG);
	gpio_rmw(GPIO_OE_REG, 0, mask);
	return 0;
}

static int ar9340_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	unsigned mask = BIT(offset);
//	printk("rb400_gpio_direction_input %u(%u)\n", offset, offset);
	gpio_rmw(GPIO_OE_REG, 0, mask);
	return 0;
}

static int ar9340_gpio_direction_output(struct gpio_chip *chip,
					unsigned offset, int value)
{
	unsigned mask = BIT(offset);
//	printk("rb400_gpio_direction_output %u(%u) %d\n",
//	       offset, offset, value);
	gpio_writel(mask, value ? GPIO_SET_REG : GPIO_CLEAR_REG);
	gpio_rmw(GPIO_OE_REG, mask, 0);
	return 0;
}

static int rb400_gpio_to_irq(struct gpio_chip *chip, unsigned offset)
{
	return GPIO_IRQ_BASE + offset;
}

static struct gpio_chip rb400_gpio_chip = {
	.label			= "cpu-gpio-pins",
	.direction_input	= rb400_gpio_direction_input,
	.direction_output	= rb400_gpio_direction_output,
	.get			= rb400_gpio_get,
	.set			= rb400_gpio_set,
	.to_irq			= rb400_gpio_to_irq,
	.base			= 0,
	.ngpio			= 28,
};

static int rb400_gpio_probe(struct platform_device *pdev) {
	long flags = (long)pdev->dev.platform_data;
	long base = GPIO_BASE;
	if (flags & RB400_GPIO_BASE2) base = GPIO_BASE2;
	gpio_base = ioremap_nocache(base, PAGE_SIZE);
	if (!gpio_base) {
		printk("could not remap GPIO_BASE\n");
		return -ENOMEM;
	}

	if (flags & RB400_GPIO_INVERT_OE) {
		rb400_gpio_chip.direction_output = ar9340_gpio_direction_output;
		rb400_gpio_chip.direction_input = ar9340_gpio_direction_input;
	}
	if (flags & RB400_GPIO11_RESET_FN) {
		// reset GPIO11 output function to gpio, needed for RB2011 SFP
		gpio_rmw(0x34, 0xff000000, 0);
	}
	gpiochip_add(&rb400_gpio_chip);
	return 0;
}

static struct platform_driver rb400_gpio_driver = {
	.probe	= rb400_gpio_probe,
	.driver	= {
		.name = "rb400-gpio",
		.owner = THIS_MODULE,
	},
};


static int __init rb400_gpio_init(void)
{
	int ret;

	ret = platform_driver_register(&rb400_gpio_driver);
	if (ret)
		printk(KERN_ERR "rb400-gpio: probe failed: %d\n", ret);

	return ret;
}
subsys_initcall(rb400_gpio_init);

static void __exit rb400_gpio_exit(void)
{
	platform_driver_unregister(&rb400_gpio_driver);
}
module_exit(rb400_gpio_exit);

//module_platform_driver(rb400_gpio_driver);
