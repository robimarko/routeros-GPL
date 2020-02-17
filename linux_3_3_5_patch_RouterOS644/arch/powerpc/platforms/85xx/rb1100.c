#include <linux/of_platform.h>
#include <asm/time.h>
#include <asm/machdep.h>
#include <asm/mpic.h>
#include <asm/mtvic.h>
#include <sysdev/fsl_pci.h>
#include <asm/rb_aux.h>

static unsigned char latch_status = 0x0C; // the way booter sets latch

static u64 get_latch_address(void) {
    u64 size;
    u64 ret = 0;
    unsigned flags;
    struct device_node *latch_dev_node;
    latch_dev_node = of_find_node_by_name(NULL, "latch");
    if (latch_dev_node) {
	const u32 *tmp = of_get_address(latch_dev_node, 0, &size, &flags);
	ret = of_translate_address(latch_dev_node, tmp);	
	printk("rb1100 latch address = %08x\n", (u32) ret);
	of_node_put(latch_dev_node);
    }
    return ret;
}

static unsigned char *latch = NULL;
void change_latch(unsigned char set, unsigned char clear) {
    latch_status = (latch_status & ~clear) | set;
    out_8(latch, latch_status);    
}

EXPORT_SYMBOL(change_latch);

static void __init rb1100_setup_arch(void)
{
	latch = ioremap_nocache(get_latch_address(), 1);

	mtspr(SPRN_HID0, 1 << 14); /* set TBEN */
	mb();

	if ((mfspr(SPRN_SVR) >> 16) == 0x803c)
		add_crypto_of_node(0x2d);

	ppc_md.power_save = rb_idle;

#ifdef CONFIG_PCI
	rb_init_pci();
#endif
}

static void __init rb1100_init_early(void) {
	add_second_serial_of_node();
}

static int __init rb1100_probe(void)
{
	char *model;

	model = of_get_flat_dt_prop(of_get_flat_dt_root(), "model", NULL);

	if (!model)
		return 0;

	return strcmp(model, "RB1100") == 0;
}

void rb_power_off(void);

define_machine(rb1100) {
	.name			= "RB1100",
	.probe			= rb1100_probe,
	.setup_arch		= rb1100_setup_arch,
	.init_early		= rb1100_init_early,
	.init_IRQ		= rb_pic_init,
	.get_irq		= rb_get_irq,
	.show_cpuinfo		= rb_show_cpuinfo,
	.restart		= rb_restart,
	.calibrate_decr		= generic_calibrate_decr,
	.pcibios_fixup_bus	= fsl_pcibios_fixup_bus,
	.power_off		= rb_power_off,
};
