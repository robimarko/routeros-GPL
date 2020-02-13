#include <linux/module.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/seq_file.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <mm/mmu_decl.h>
#include <asm/machdep.h>
#include <asm/mpic.h>
#include <asm/mtvic.h>
#include <asm/vm.h>
#include <asm/rb_aux.h>

#define GPIO(x) (0x80000000 >> (x))
static unsigned *gpio_data = NULL;
static unsigned *picr = NULL;

const unsigned *beep1;
const unsigned *beep2;

#define GT0_BASE_COUNT (picr + (0x1110 / 4))

hypercall(hv_yield, 16, void);

static void ioremap_from_node(const unsigned *property, unsigned **ptr) {
    struct resource res;
    struct device_node *nd;
    
    nd = of_find_node_by_phandle(property[0]);
    if (!nd || of_address_to_resource(nd, 0, &res)) return;
    of_node_put(nd);

    *ptr = ioremap_nocache(res.start, res.end - res.start + 1);
}

irqreturn_t beeper_irq(int irq, void *ptr)
{
	static int toggle = 1;
	if (toggle) {
	    out_be32(gpio_data, in_be32(gpio_data) & ~GPIO(beep1[0]));
	    if (beep2) {
		out_be32(gpio_data, in_be32(gpio_data) & ~GPIO(beep2[0]));
	    }
	}
	else {
	    out_be32(gpio_data, in_be32(gpio_data) | GPIO(beep1[0]));
	    if (beep2) {
		out_be32(gpio_data, in_be32(gpio_data) | GPIO(beep2[0]));
	    }
	}
	toggle ^= 1; 
	return IRQ_HANDLED;
}

void consume(int x) { }

extern unsigned long ppc_tb_freq;
static unsigned long ppc_tb_freq_kHz;

static void adjust_gpio_data_register(void) {
    unsigned pvr = mfspr(SPRN_PVR) & ~0xff;
    printk("pvr = %08x\n", pvr);
    if (pvr != 0x80210000) {
	/* aim pointer at GPIO data register */
	gpio_data = gpio_data + 2;
    }
}

static void __init rb_beeper_init(void)
{
	struct device_node *beeper;
	unsigned interrupt;
	const unsigned *int_p;
	const unsigned *gpio;
	
	beeper = of_find_node_by_name(NULL, "beeper");
	if (!beeper)
		return;

	beep1 = of_get_property(beeper, "beep1", NULL);
	beep2 = of_get_property(beeper, "beep2", NULL);
	gpio  = of_get_property(beeper, "gpio", NULL);
	int_p = of_get_property(beeper, "interrupt-parent", NULL);
	
	ioremap_from_node(gpio, &gpio_data);
	ioremap_from_node(int_p, &picr);
	
	ppc_tb_freq_kHz = (ppc_tb_freq / 1000);
	
	if (gpio_data) {
	    adjust_gpio_data_register();

	    interrupt = irq_of_parse_and_map(beeper, 0);
	    if (interrupt != NO_IRQ)
		consume(request_irq(interrupt, beeper_irq, 
				    IRQF_TRIGGER_RISING,
				    "beeper", NULL));
	}
}

void router_beep(unsigned freq) {
	if (!ppc_tb_freq_kHz)
		return;

	out_be32(GT0_BASE_COUNT,
		 freq ? (500 * ppc_tb_freq_kHz) / freq : 0x80000000);
}
EXPORT_SYMBOL(router_beep);

static struct resource rb_led_resources[2] = {
	[0] = {
		.flags		= IORESOURCE_IO,
	},
	[1] = {
		.name		= "user-led",
	},
};

static const unsigned rb_uled[2] = { 0x400, 0x1c };

static int __init rb_leds_init(void)
{
	struct device_node *np;
	const unsigned *uled = rb_uled;

	np = of_find_node_by_name(NULL, "led");
	if (np) {
		uled = of_get_property(np, "user_led", NULL);
		of_node_put(np);
		if (!uled) {
			printk("rbppc led error: "
			       "user_led property is missing\n");
			return -1;
		}
	}

	rb_led_resources[1].start = uled[1];
	rb_led_resources[1].end = uled[1];

	np = of_find_node_by_phandle(uled[0]);
	if (!np) {
		printk("rbppc led error: no gpio<%x> node found\n", *uled);
		return -1;
	}
	if (of_address_to_resource(np, 0, &rb_led_resources[0])) {
		of_node_put(np);
		printk("rbppc led error: no reg property in gpio found\n");
		return -1;
	}
	of_node_put(np);

	platform_device_register_simple("rbppc-led", 0,
					rb_led_resources, 2);
	return 0;
}

static struct of_device_id __initdata of_bus_ids[] = {
	{ .type = "soc", },
	{ .compatible = "soc", },
	{ .compatible = "fsl,dpaa", },
	{ .compatible = "simple-bus", },
	{},
};

static int __init rb_declare_of_platform_devices(void)
{
	struct device_node *np;
	unsigned idx;

	np = of_find_node_by_name(NULL, "nand");
	if (np) of_platform_device_create(np, "nand", NULL);

	np = of_find_node_by_name(NULL, "nand_fcm");
	if (np) of_platform_device_create(np, "nand_fcm", NULL);

	np = of_find_node_by_name(NULL, "spi");
	if (np) of_platform_device_create(np, "spi", NULL);

	idx = 0;
	for_each_node_by_type(np, "rb,cf") {
		char dev_name[12];
		snprintf(dev_name, sizeof(dev_name), "cf.%u", idx);
		of_platform_device_create(np, dev_name, NULL);
		++idx;		
	}

	rb_beeper_init();
	rb_leds_init();

	of_platform_bus_probe(NULL, of_bus_ids, NULL);

	return 0;
}

device_initcall(rb_declare_of_platform_devices);

void __init rb_pic_init(void)
{
	struct device_node *np;
	struct resource r;
	struct mpic *mpic;
	void *gcr;
	unsigned i;

	mtvic_init(0);

	np = of_find_node_by_type(NULL, "open-pic");

	if (!np)
		return;

	if (of_address_to_resource(np, 0, &r)) {
		printk(KERN_ERR "mpic error: no region specified\n");
		of_node_put(np);
		return;
	}

	gcr = ioremap(r.start + 0x1020, 4);
	out_be32(gcr, in_be32(gcr) | (1 << 29));
	iounmap(gcr);

	mpic = mpic_alloc(np, r.start, MPIC_WANTS_RESET | MPIC_BIG_ENDIAN,
			  4, 0, " OpenPIC ");
	for (i = 0; i < 80; i += 4) {
		/* 
		 * each mpic_assign_isu call assigns 4 isus at once
		 * (as it is specified in mpic_alloc call 4th argument)
		 * therefore base address must be incremented by 0x80
		 */
		mpic_assign_isu(mpic, i / 4, r.start + 0x10000 + i * 0x20);
	}
	mpic_assign_isu(mpic, 80 / 4, r.start + 0x1120);

	of_node_put(np);
	mpic_init(mpic);
}

void rb_show_cpuinfo(struct seq_file *m)
{
	seq_printf(m, "Vendor\t\t: Mikrotik\n");
	seq_printf(m, "Machine\t\t: %s\n", ppc_md.name);
	seq_printf(m, "Memory\t\t: %u MB\n", total_memory / (1024 * 1024));
}

void rb_restart(char *cmd)
{
	local_irq_disable();

	mtmsr(mfmsr() | 0x00000200);
	mtspr(0x134, mfspr(0x134) | 0x70000000);
}

void rb_power_off(void)
{
	printk(KERN_EMERG "System Halted, OK to turn off power\n");
	while (1) ;
}

void rb_idle(void) {
	extern void e500_idle(void);
	int err;

	local_irq_enable();
	err = hv_yield();
	local_irq_disable();
	if (err < 0) e500_idle();
}

struct of_prop {
    const char *name;
    unsigned size;
    unsigned values[2];
    struct property prop;
};

void add_of_property(struct device_node *np, struct property *pp,
		     const char *name, unsigned size, void *value)
{
	memset(pp, 0, sizeof(struct property));
	pp->name = (char *) name;
	pp->length = size;
	pp->value = value;

	prom_add_property(np, pp);
}

struct of_prop serial_properties[] = {
    { "reg", 8, { 0x4500, 0x100 } },
    { "interrupts", 8,  { 0x2a, 2 } },
    { "interrupt-parent", 4, { 0x700 } },
    { NULL, 0, { 0 } }
};

/*
 * The device tree is traversed in some funny order. If we just add second
 * serial node (0x4600) then it is found as first and onboard serial connector
 * is found as second which is no good because debug and console will
 * show up on serial that is attaced to header pins. Therefore we add new node
 * with offset (0x4500) and replace original node's offset with (0x4600)
 */
void add_second_serial_of_node(void)
{
	static struct device_node serial_node;
	static struct property comp_prop;
	static struct property clock_prop;
	static unsigned clock = 0;

	struct device_node *np;
	struct device_node *sp;
	struct of_prop *p;

	sp = of_find_node_by_type(NULL, "serial");
	if (!sp) return;

	clock = * (unsigned *) of_get_property(sp, "clock-frequency", NULL);

	np = &serial_node;
	memset(np, 0, sizeof(struct device_node));
	kref_init(&np->kref);

	np->name = "serial-port";
	np->type = of_get_property(sp, "device_type", NULL);

	add_of_property(np, &comp_prop,  "compatible",      8, "ns16550");
	add_of_property(np, &clock_prop, "clock-frequency", 4, &clock);

	for (p = serial_properties; p->name; ++p) {
		add_of_property(np, &p->prop, p->name, p->size, p->values);
	}

	/* change original node to 0x4600 */
	np->full_name = sp->full_name;
	sp->full_name = "/soc8544@e0000000/serial@4600";	
	* (unsigned *) of_get_property(sp, "reg", NULL) = 0x4600;

	np->parent = sp->parent;
	of_attach_node(np);
}

struct of_prop crypto_properties[] = {
    { "reg", 8, { 0x30000, 0x10000 } },
    { "interrupts", 8,  { 0x1d, 2 } },
    { "interrupt-parent", 4, { 0x700 } },
    { "fsl,num-channels", 4, { 4 } },
    { "fsl,channel-fifo-len", 4, { 24 } },
    { "fsl,exec-units-mask", 4, { 0xfe } },
    { "fsl,descriptor-types-mask", 4, { 0x12b0ebf } },
    { NULL, 0, { 0 } }
};

void add_crypto_of_node(unsigned irq)
{
	static struct device_node crypto_node;
	static struct property comp_prop;

	struct device_node *np;
	struct device_node *sp;
	struct of_prop *p;

	sp = of_find_node_by_type(NULL, "serial");
	if (!sp)
		return;

	np = &crypto_node;
	memset(np, 0, sizeof(struct device_node));
	np->full_name = "crypto@30000";
	kref_init(&np->kref);
	
	add_of_property(np, &comp_prop, "compatible", 11, "fsl,sec2.0");

	for (p = crypto_properties; p->name; ++p) {
		if (strcmp(p->name, "interrupts") == 0) {
		    p->values[0] = irq;
		}
		add_of_property(np, &p->prop, p->name, p->size, p->values);
	}

	np->parent = sp->parent;
	of_attach_node(np);
}

/* 
 * this function is copy pasted from rb1200.c
 * black belt told me to leave it like that for now
 */
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

EXPORT_SYMBOL(get_gpio_def);
