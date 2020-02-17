#include <linux/init.h>
#include <linux/leds.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <asm/rb/rb500.h>

static void rb500_led_set_user(struct led_classdev *led_cdev,
			       enum led_brightness brightness)
{
	if (brightness)
		changeLatchU5(RB500_LO_ULED, 0);
	else
		changeLatchU5(0, RB500_LO_ULED);
}

static struct led_classdev rb500_led = {
       .name = "user-led",
       .brightness_set = rb500_led_set_user,
};

static int rb500_led_probe(struct platform_device *pdev)
{
	return led_classdev_register(&pdev->dev, &rb500_led);
}

static struct platform_driver rb500_led_driver = {
	.probe	= rb500_led_probe,
	.driver	= {
		.name = "rb500-led",
		.owner = THIS_MODULE,
	},
};

static int __init rb500_led_init(void)
{
	return platform_driver_register(&rb500_led_driver);
}

static void __exit rb500_led_exit(void)
{
	platform_driver_unregister(&rb500_led_driver);
}

module_init(rb500_led_init);
module_exit(rb500_led_exit);
