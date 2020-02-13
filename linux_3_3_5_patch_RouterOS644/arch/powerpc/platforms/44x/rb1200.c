#include <asm/machdep.h>
#include <asm/pci-bridge.h>
#include <asm/ppc4xx.h>
#include <asm/prom.h>
#include <asm/time.h>
#include <asm/udbg.h>
#include <asm/uic.h>
#include <asm/dcr-native.h>

#include <linux/interrupt.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/of_platform.h>
#include <linux/leds.h>

static __initdata struct of_device_id ppc44x_of_bus[] = {
	{ .compatible = "ibm,plb4", },
	{ .compatible = "ibm,opb", },
	{ .compatible = "ibm,ebc", },
	{ .compatible = "simple-bus", },
	{},
};

static char *board[] __initdata = {
	"RB1200",
};

unsigned get_gpio_def(const char *name) {
    unsigned pin = 0;
    struct device_node *np, *child;	
    np = of_find_node_by_name(NULL, "gpio-definitions");
    if (!np) return 0;
    for_each_child_of_node(np, child) {
	if (strcmp(child->name, name) == 0) {
	    pin = of_get_gpio(child, 0);
	}
    }
    of_node_put(np);
    return pin;
}

void switch_monitor(unsigned state) {
    static unsigned monsel0 = 0;
    static unsigned monsel1 = 0;
    if (!monsel0 || !monsel1) {
	monsel0 = get_gpio_def("mon-sel0");
	monsel1 = get_gpio_def("mon-sel1");
	if (!monsel0 || !monsel1) return;	
    }
    gpio_set_value(monsel0, (state & 1) ? 1 : 0);
    gpio_set_value(monsel1, (state & 2) ? 1 : 0);
}
EXPORT_SYMBOL(switch_monitor);

void switch_mdio(int flip) {
    static unsigned mdio_gpio = 0;
    if (!mdio_gpio) {
	mdio_gpio = get_gpio_def("mdio_switch");
	if (!mdio_gpio) return;
    }
    gpio_set_value(mdio_gpio, flip);
}
EXPORT_SYMBOL(switch_mdio);

static int __init ppc44x_probe(void)
{
	unsigned long root = of_get_flat_dt_root();
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(board); i++) {
		if (of_flat_dt_is_compatible(root, board[i])) {
			pci_set_flags(PCI_REASSIGN_ALL_RSRC);
			return 1;
		}
	}

	return 0;
}

struct hrtimer beep_timer;
static unsigned beep_gpio = 0;
static unsigned beep_interval = 0;
static enum hrtimer_restart beep_callback(struct hrtimer *t) {
    static unsigned state = 1;
    gpio_set_value(beep_gpio, state);
    state = !state;
    hrtimer_forward_now(&beep_timer, ns_to_ktime(beep_interval));
    return HRTIMER_RESTART;
}

void router_beep(unsigned freq) {
    if (!beep_gpio) {
	beep_gpio = get_gpio_def("beep");
	printk("beep_gpio = 0x%08x\n", beep_gpio);
	if (!beep_gpio) return;

	hrtimer_init(&beep_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	beep_timer.function = beep_callback;
    }
    if (freq > 0) {
	beep_interval = 1000000000 / (2 * freq);
	printk("beep freq = %u\n", freq);
	hrtimer_start(&beep_timer, ns_to_ktime(0), HRTIMER_MODE_REL_PINNED);
    }
    else {
	hrtimer_cancel(&beep_timer);
	gpio_set_value(beep_gpio, 0);
    }
}
EXPORT_SYMBOL(router_beep);

void export_gpio_by_name(const char *name, int active_low) {
    unsigned gpio_pin = get_gpio_def(name);
    if (gpio_pin) {
	gpio_request(gpio_pin, NULL);
	gpio_sysfs_set_active_low(gpio_pin, active_low);
	gpio_export(gpio_pin, 1);
    }
}

static int __init ppc44x_device_probe(void)
{
	export_gpio_by_name("button", 1);
	export_gpio_by_name("pin-hole", 1);
	export_gpio_by_name("fan-on", 0);
	export_gpio_by_name("sense", 0);
	of_platform_bus_probe(NULL, ppc44x_of_bus, NULL);
	return 0;
}
machine_device_initcall(rb1200, ppc44x_device_probe);

static void rb_power_off(void)
{
	printk(KERN_EMERG "System Halted, OK to turn off power\n");
	while (1) ;
}

define_machine(rb1200) {
	.name = "RB1200",
	.probe = ppc44x_probe,
	.progress = udbg_progress,
	.init_IRQ = uic_init_tree,
	.get_irq = uic_get_irq,
	.restart = ppc4xx_reset_system,
	.calibrate_decr = generic_calibrate_decr,
	.power_off = rb_power_off,
};
