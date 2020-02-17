#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <asm/rb/rb400.h>


static struct i2c_board_info gpio_i2c_devices[2] = {
	{
		.type = "sfp-eeprom",
		.addr = 0xa0 >> 1,
	},
	{
		.type = "sfp-diagnostics",
		.addr = 0xa2 >> 1,
	},
};

static int rb400_gpio_i2c_sfp_probe(struct platform_device *pdev) {
	int ret;
	ret = i2c_register_board_info(0, gpio_i2c_devices, 2);
	printk("rb400_gpio_i2c_sfp_probe %d\n", ret);
	if (ret < 0) {
		printk("rb400_gpio_i2c_sfp_probe: i2c_register_board_info_failed: %d\n", ret);
	}
	return 0;
}

static struct platform_driver rb400_gpio_i2c_sfp_driver = {
	.probe	= rb400_gpio_i2c_sfp_probe,
	.driver	= {
		.name = "rb400-gpio-i2c-sfp",
		.owner = THIS_MODULE,
	},
};

static int __init rb400_gpio_i2c_sfp_init(void)
{
	int ret;

	ret = platform_driver_register(&rb400_gpio_i2c_sfp_driver);
	if (ret)
		printk(KERN_ERR "rb400-gpio-i2c-sfp: probe failed: %d\n", ret);

	return ret;
}
subsys_initcall(rb400_gpio_i2c_sfp_init);

static void __exit rb400_gpio_i2c_sfp_exit(void)
{
	platform_driver_unregister(&rb400_gpio_i2c_sfp_driver);
}
module_exit(rb400_gpio_i2c_sfp_exit);

//module_platform_driver(rb400_gpio_driver);
