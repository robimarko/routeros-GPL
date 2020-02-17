#include <linux/spinlock.h>
#include <linux/memblock.h>
#include <asm/time.h>
#include <asm/machdep.h>
#include <asm/mpic.h>
#include <asm/mtvic.h>
#include <sysdev/fsl_pci.h>
#include <asm/rb_aux.h>

#ifdef CONFIG_SMP
extern void __init mpc85xx_smp_init(void);
#endif

extern int rb_big_boot_partition;

spinlock_t localbus_lock = __SPIN_LOCK_UNLOCKED(localbus_lock);
EXPORT_SYMBOL(localbus_lock);

static void __init rb1120_setup_arch(void)
{
	rb_big_boot_partition = 1;

	mtspr(SPRN_HID0, 1 << 14); /* set TBEN */
	mb();

	ppc_md.power_save = rb_idle;

#ifdef CONFIG_PCI
	rb_init_pci();
#endif

#ifdef CONFIG_SMP
	mpc85xx_smp_init();
#endif
}

static int is_board(const char *str) {
	char *model;
	model = of_get_flat_dt_prop(of_get_flat_dt_root(), "model", NULL);
	return model ? (strcmp(model, str) == 0) : 0;    
}

#ifdef CONFIG_FSL_QMAN_CONFIG
void qman_init_early(void);
#endif
#ifdef CONFIG_FSL_BMAN_CONFIG
void bman_init_early(void);
#endif
static void __init rb1120_init_early(void) {
	if (!is_board("RB850G")) {
		add_second_serial_of_node();
	}
#ifdef CONFIG_FSL_QMAN_CONFIG
	qman_init_early();
#endif
#ifdef CONFIG_FSL_BMAN_CONFIG
	bman_init_early();
#endif
}

static int __init rb1120_probe(void)
{
	int is_rb_p_type = is_board("RB1120");
	if (is_board("RB850G")) is_rb_p_type = true;

	if (is_rb_p_type) {
	    unsigned top = memblock_end_of_DRAM();
	    memblock_add(top, 0x1000);
	    memblock_reserve(top, 0x1000);
	}

	return is_rb_p_type;
}

void rb_power_off(void);

void rb1120_restart(char *cmd)
{
	static phys_addr_t immrbase = -1;
	struct device_node *soc = of_find_node_by_type(NULL, "soc");
	unsigned *addr;
	int size;
    
	if (soc) {
	    const void *prop = of_get_property(soc, "reg", &size);
	    immrbase = of_translate_address(soc, prop);
	    of_node_put(soc);

	    addr = (unsigned *) ioremap_nocache(immrbase + 0xe0000, 0x100);
	    local_irq_disable();
	    out_be32(addr + 44, 2); 
	    iounmap(addr);
	}
	else {
	    printk("This is emergency!\n");
	    local_irq_disable();
	    rb_restart(NULL);
	}
}

define_machine(rb1120) {
	.name			= "RB1120",
	.probe			= rb1120_probe,
	.setup_arch		= rb1120_setup_arch,
	.init_early		= rb1120_init_early,
	.init_IRQ		= rb_pic_init,
	.get_irq		= rb_get_irq,
	.show_cpuinfo		= rb_show_cpuinfo,
	.restart		= rb1120_restart,
	.calibrate_decr		= generic_calibrate_decr,
	.pcibios_fixup_bus	= fsl_pcibios_fixup_bus,
	.power_off		= rb_power_off,
};
