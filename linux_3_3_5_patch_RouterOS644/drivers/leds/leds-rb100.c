#include <linux/init.h>
#include <linux/leds.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <asm/rb/rb100.h>

static void rb100_led_set_user(struct led_classdev *led_cdev,
			       enum led_brightness brightness)
{
	if (brightness)
		RB100_GPIO() |= RB100_GPOUT(RB100_GPIO_ULED);
	else
		RB100_GPIO() &= ~RB100_GPOUT(RB100_GPIO_ULED);
}

static void rb112_led_set_user(struct led_classdev *led_cdev,
			       enum led_brightness brightness)
{
	if (brightness)
		RB100_GPIO() |= RB100_GPOUT(RB112_GPIO_ULED);
	else
		RB100_GPIO() &= ~RB100_GPOUT(RB112_GPIO_ULED);
}

static struct led_classdev rb100_led = {
       .name = "user-led",
       .brightness_set = rb100_led_set_user,
};

static struct led_classdev rb112_led = {
       .name = "user-led",
       .brightness_set = rb112_led_set_user,
};

static int rb100_led_probe(struct platform_device *pdev)
{
	return led_classdev_register(&pdev->dev, &rb100_led);
}

static int rb112_led_probe(struct platform_device *pdev)
{
	return led_classdev_register(&pdev->dev, &rb112_led);
}

static struct platform_driver rb100_led_driver = {
	.probe	= rb100_led_probe,
	.driver	= {
		.name = "rb100-led",
		.owner = THIS_MODULE,
	},
};

static struct platform_driver rb112_led_driver = {
	.probe	= rb112_led_probe,
	.driver	= {
		.name = "rb112-led",
		.owner = THIS_MODULE,
	},
};

static int __init rb100_led_init(void)
{
	platform_driver_register(&rb100_led_driver);
	platform_driver_register(&rb112_led_driver);
	return 0;
}

static void __exit rb100_led_exit(void)
{
	platform_driver_unregister(&rb100_led_driver);
	platform_driver_unregister(&rb112_led_driver);
}

module_init(rb100_led_init);
module_exit(rb100_led_exit);
