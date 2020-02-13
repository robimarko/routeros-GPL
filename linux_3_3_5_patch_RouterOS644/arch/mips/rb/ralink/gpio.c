#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <asm/rb/rb400.h>

#define GPIO_BASE	0x1E000600
#define GPIO_CTRL_0	    0x0000
#define GPIO_POL_0	    0x0010
#define GPIO_DATA_0	    0x0020
#define GPIO_DSET_0	    0x0030
#define GPIO_DCLR_0	    0x0040

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


static int ralink_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	unsigned ofs = (offset / 32) * 4;
	unsigned mask = BIT(offset & 0x1f);
	int val = !!(gpio_readl(GPIO_DATA_0 + ofs) & mask);
//	printk("ralink_gpio_get %u(%u) %d %08x\n",
//	       offset, ofs, val, gpio_readl(GPIO_DATA_0 + ofs));
	return val;
}

static void ralink_gpio_set(struct gpio_chip *chip,
				unsigned offset, int value)
{
	unsigned ofs = (offset / 32) * 4;
	unsigned mask = BIT(offset & 0x1f);
//	printk("ralink_gpio_set %u(%u) %d\n", offset, offset, value);
	gpio_writel(mask, ofs + (value ? GPIO_DSET_0 : GPIO_DCLR_0));
//	printk("set ok\n");
}

static int ralink_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	unsigned ofs = (offset / 32) * 4;
	unsigned mask = BIT(offset & 0x1f);
//	printk("ralink_gpio_direction_input %u(%u)\n", offset, offset);
	gpio_rmw(GPIO_CTRL_0 + ofs, mask, 0);
	return 0;
}

static int ralink_gpio_direction_output(struct gpio_chip *chip,
					unsigned offset, int value)
{
	unsigned ofs = (offset / 32) * 4;
	unsigned mask = BIT(offset & 0x1f);
//	printk("ralink_gpio_direction_output %u(%u) %d\n",
//	       offset, offset, value);
	gpio_rmw(GPIO_CTRL_0 + ofs, 0, mask);
	gpio_writel(mask, ofs + (value ? GPIO_DSET_0 : GPIO_DCLR_0));
	return 0;
}

//static int ralink_gpio_to_irq(struct gpio_chip *chip, unsigned offset)
//{
//	return GPIO_IRQ_BASE + offset;
//}

static struct gpio_chip ralink_gpio_chip = {
	.label			= "cpu-gpio-pins",
	.direction_input	= ralink_gpio_direction_input,
	.direction_output	= ralink_gpio_direction_output,
	.get			= ralink_gpio_get,
	.set			= ralink_gpio_set,
//	.to_irq			= ralink_gpio_to_irq,
	.base			= 0,
	.ngpio			= 49,
};

static int ralink_gpio_probe(struct platform_device *pdev) {
	gpio_base = ioremap_nocache(GPIO_BASE, PAGE_SIZE);
	if (!gpio_base) {
		printk("could not remap GPIO_BASE\n");
		return -ENOMEM;
	}

	gpiochip_add(&ralink_gpio_chip);
	return 0;
}

static struct platform_driver ralink_gpio_driver = {
	.probe	= ralink_gpio_probe,
	.driver	= {
		.name = "ralink-gpio",
		.owner = THIS_MODULE,
	},
};


static int __init ralink_gpio_init(void)
{
	int ret;

	ret = platform_driver_register(&ralink_gpio_driver);
	if (ret)
		printk(KERN_ERR "ralink-gpio: probe failed: %d\n", ret);

	return ret;
}
subsys_initcall(ralink_gpio_init);

static void __exit ralink_gpio_exit(void)
{
	platform_driver_unregister(&ralink_gpio_driver);
}
module_exit(ralink_gpio_exit);
