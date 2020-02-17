#include <linux/of_platform.h>
#include <asm/time.h>
#include <asm/machdep.h>
#include <asm/mpic.h>
#include <asm/mtvic.h>
#include <sysdev/fsl_pci.h>
#include <asm/rb_aux.h>

#ifdef MT_DEBUG
void rb1000_putc(char c)
{

        while (!(*(volatile unsigned char *) 0xe0004505 & 0x20));

	*(char *) 0xe0004500 = c;
}

void rb1000_puts(char *str)
{
        while (*str) {
	        if (*str == '\n') rb1000_putc('\r');
		rb1000_putc(*str);
		++str;
	}
}

void rb1000_printk(const char *fmt, ...) {
	va_list args;
	char buf[256];

	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	rb1000_puts(buf);
}

void rb1000_init(void)
{
#if 0
        if (inited) return;

        settlbcam(3, 0xf0000000, 0xe0000000, 0x10000, _PAGE_IO, 0);
	inited = 1;
#endif
}
#endif

static void __init rb1000_pic_init(void)
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
			  1, 0, " OpenPIC ");
	
	for (i = 0; i < 31; ++i) {
		if (i == 11 || i == 12) {
		    /* Ext IRQ4 and IRQ5 is mapped to 11 & 12 respectively */
			mpic_assign_isu(mpic, i,
					r.start + 0x10000 + (i - 11 + 4) * 0x20);
		} else if (i == 30) {
			mpic_assign_isu(mpic, i, r.start + 0x10000 + 7 * 0x20);
		} else {
			mpic_assign_isu(mpic, i, r.start + 0x10200 + i * 0x20);
		}
	}
	mpic_assign_isu(mpic, 31, r.start + 0x1120);

	of_node_put(np);
	mpic_init(mpic);
}

static void __init rb1000_setup_arch(void)
{
	mtspr(SPRN_HID0, 1 << 14); /* set TBEN */
	mb();

	/* matches MPC8548E, MPC8547E, MPC8545E */
	if ((mfspr(SPRN_SVR) >> 16) == 0x8039)
		add_crypto_of_node(0x1d);

	ppc_md.power_save = rb_idle;
}

static int __init rb1000_probe(void)
{
	char *model;

	model = of_get_flat_dt_prop(of_get_flat_dt_root(), "model", NULL);

	if (!model)
		return 0;

	return strcmp(model, "RB1000") == 0;
}

void rb_power_off(void);

define_machine(rb1000) {
	.name			= "RB1000",
	.probe			= rb1000_probe,
	.setup_arch		= rb1000_setup_arch,
	.init_IRQ		= rb1000_pic_init,
	.show_cpuinfo		= rb_show_cpuinfo,
	.get_irq		= rb_get_irq,
	.restart		= rb_restart,
	.calibrate_decr		= generic_calibrate_decr,
	.pcibios_fixup_bus	= fsl_pcibios_fixup_bus,
	.power_off		= rb_power_off,
};
