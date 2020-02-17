#include <linux/init.h>
#include <linux/leds.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <asm/rb.h>

struct ccr_led_data {
	unsigned long value;
	struct led_classdev cdev;
	void (*set_fn)(unsigned long value, unsigned long mask);
};

static struct ccr_led_data *get_led_data(struct led_classdev *led_cdev)
{
	return container_of(led_cdev, struct ccr_led_data, cdev);
}

static void ccr_led_set(struct led_classdev *led_cdev,
			enum led_brightness brightness)
{
	struct ccr_led_data *led_data = get_led_data(led_cdev);
	led_data->set_fn(brightness ? led_data->value : 0, led_data->value);
}

static void set_latch(unsigned long value, unsigned long mask) {
	access_latch(0, value, mask);
}

static void inv_latch(unsigned long value, unsigned long mask) {
	access_latch(0, value ^ mask, mask);
}

static void inv_gpio(unsigned long value, unsigned long mask) {
	set_gpio(value == mask ? 0 : mask, mask);
}

#define LED_ENTRY(led_name, fn, val) \
	{ .cdev = { .name = led_name }, .set_fn = fn, .value = val }

#define EMPTY_ENTRY() LED_ENTRY(NULL, NULL, 0)

#define GPIO_LED(led_name, val) LED_ENTRY(led_name, &set_gpio, val)
#define GPIO_INV(led_name, val) LED_ENTRY(led_name, &inv_gpio, val)
#define LATCH_LED(led_name, val) LED_ENTRY(led_name, &set_latch, val)
#define LATCH_INV(led_name, val) LED_ENTRY(led_name, &inv_latch, val)

static struct ccr_led_data ccr_led[] = {
	GPIO_LED("lcd", GPO_LCD_LED),
	GPIO_LED("user-led", GPO_USER_LED),
	GPIO_INV("monitor-select", GPO_MON_SEL),
	GPIO_LED("usb-power-off", GPO_USB_POWER),
	EMPTY_ENTRY(),
};

static struct ccr_led_data ccr1009c_led[] = {
	GPIO_LED("lcd", GPO_LCD_LED),
	GPIO_LED("user-led", GPO_USER_LED),
	GPIO_INV("monitor-select", GPO_MON_SEL),
	GPIO_INV("usb-power-off", GPO_USB_POWER),
	EMPTY_ENTRY(),
};

static struct ccr_led_data ccr1072_led[] = {
	GPIO_LED("lcd", GPO_LCD_LED),
	GPIO_LED("user-led", GPO_USER_LED),
	GPIO_LED("monitor-select", GPO_MON_SEL72),
	LATCH_LED("usb-select", BIT(0)),
	LATCH_INV("fault-led", BIT(8)),
	EMPTY_ENTRY(),
};

static struct ccr_led_data ccr1072_r3_led[] = {
	GPIO_LED("lcd", GPO_LCD_LED),
	GPIO_LED("user-led", GPO_USER_LED),
	GPIO_LED("monitor-select", GPO_MON_SEL72),
	GPIO_LED("usb-power-off-0", GPO_USB_RST(0)),
	GPIO_LED("usb-power-off-1", GPO_USB_RST(1)),
	LATCH_INV("fault-led", BIT(8)),
	EMPTY_ENTRY(),
};

static struct ccr_led_data *get_board_led_info(void) {
	if (is_board_type("ccr1009c"))
		return ccr1009c_led;
	else if (is_CCR1072_r3())
		return ccr1072_r3_led;
	else if (is_name_prefix("CCR1072"))
		return ccr1072_led;
	else
		return ccr_led;
}

static int ccr_led_probe(struct platform_device *pdev)
{
	struct ccr_led_data *led_data = get_board_led_info();
	while (led_data->cdev.name) {
	    led_data->cdev.brightness_set = ccr_led_set;
	    led_classdev_register(&pdev->dev, &led_data->cdev);
	    led_data++;
	}
	return 0;
}

static struct platform_driver ccr_led_driver = {
	.probe	= ccr_led_probe,
	.driver	= {
		.name = "ccr-led",
		.owner = THIS_MODULE,
	},
};

static int __init ccr_led_init(void)
{
	return platform_driver_register(&ccr_led_driver);
}

static void __exit ccr_led_exit(void)
{
	platform_driver_unregister(&ccr_led_driver);
}

module_init(ccr_led_init);
module_exit(ccr_led_exit);
