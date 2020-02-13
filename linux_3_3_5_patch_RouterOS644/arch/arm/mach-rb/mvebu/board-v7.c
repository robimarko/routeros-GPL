/*
 * Device Tree support for Armada 370 and XP platforms.
 *
 * Copyright (C) 2012 Marvell
 *
 * Lior Amsalem <alior@marvell.com>
 * Gregory CLEMENT <gregory.clement@free-electrons.com>
 * Thomas Petazzoni <thomas.petazzoni@free-electrons.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include <linux/init.h>
//#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/of_address.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/io.h>
#include <linux/clocksource.h>
#include <linux/dma-mapping.h>
#include <linux/memblock.h>
#include <linux/mbus.h>
#include <linux/signal.h>
#include <linux/slab.h>
//#include <linux/irqchip.h>
#include <asm/hardware/cache-l2x0.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>
#include <asm/smp_scu.h>
#include <mach/clk-provider.h>
#include <mach/system.h>
#include <mach/memory.h>
#include "armada-370-xp.h"
#include "common.h"
#include "coherency.h"
#include "mvebu-soc-id.h"
#include "cache-aurora-l2.h"

static void __iomem *scu_base;

/*
 * Enables the SCU when available. Obviously, this is only useful on
 * Cortex-A based SOCs, not on PJ4B based ones.
 */
static void __init mvebu_scu_enable(void)
{
	struct device_node *np =
		of_find_compatible_node(NULL, NULL, "arm,cortex-a9-scu");
	if (np) {
		scu_base = of_iomap(np, 0);
		scu_enable(scu_base);
		of_node_put(np);
	}
}

void __iomem *mvebu_get_scu_base(void)
{
	return scu_base;
}

/*
 * When returning from suspend, the platform goes through the
 * bootloader, which executes its DDR3 training code. This code has
 * the unfortunate idea of using the first 10 KB of each DRAM bank to
 * exercise the RAM and calculate the optimal timings. Therefore, this
 * area of RAM is overwritten, and shouldn't be used by the kernel if
 * suspend/resume is supported.
 */

//#ifdef CONFIG_SUSPEND
//#define MVEBU_DDR_TRAINING_AREA_SZ (10 * SZ_1K)
//static int __init mvebu_scan_mem(unsigned long node, const char *uname,
//				 int depth, void *data)
//{
//	const char *type = of_get_flat_dt_prop(node, "device_type", NULL);
//	const __be32 *reg, *endp;
//	int l;
//
//	if (type == NULL || strcmp(type, "memory"))
//		return 0;
//
//	reg = of_get_flat_dt_prop(node, "linux,usable-memory", &l);
//	if (reg == NULL)
//		reg = of_get_flat_dt_prop(node, "reg", &l);
//	if (reg == NULL)
//		return 0;
//
//	endp = reg + (l / sizeof(__be32));
//	while ((endp - reg) >= (dt_root_addr_cells + dt_root_size_cells)) {
//		u64 base, size;
//
//		base = dt_mem_next_cell(dt_root_addr_cells, &reg);
//		size = dt_mem_next_cell(dt_root_size_cells, &reg);
//
//		memblock_reserve(base, MVEBU_DDR_TRAINING_AREA_SZ);
//	}
//
//	return 0;
//}
//
//static void __init mvebu_memblock_reserve(void)
//{
//	of_scan_flat_dt(mvebu_scan_mem, NULL);
//}
//#else
//static void __init mvebu_memblock_reserve(void) {}
//#endif

/*
 * Early versions of Armada 375 SoC have a bug where the BootROM
 * leaves an external data abort pending. The kernel is hit by this
 * data abort as soon as it enters userspace, because it unmasks the
 * data aborts at this moment. We register a custom abort handler
 * below to ignore the first data abort to work around this
 * problem.
 */
//static int armada_375_external_abort_wa(unsigned long addr, unsigned int fsr,
//					struct pt_regs *regs)
//{
//	static int ignore_first;
//
//	if (!ignore_first && fsr == 0x1406) {
//		ignore_first = 1;
//		return 0;
//	}
//
//	return 1;
//}

/* Auxiliary Debug Modes Control 1 Register */
#define PJ4B_STATIC_BP (1 << 2) /* Enable Static BP */
#define PJ4B_INTER_PARITY (1 << 8) /* Disable Internal Parity Handling */
#define PJ4B_CLEAN_LINE (1 << 16) /* Disable data transfer for clean line */

/* Auxiliary Debug Modes Control 2 Register */
#define PJ4B_FAST_LDR (1 << 23) /* Disable fast LDR */
#define PJ4B_SNOOP_DATA (1 << 25) /* Do not interleave write and snoop data */
#define PJ4B_CWF (1 << 27) /* Disable Critical Word First feature */
#define PJ4B_OUTSDNG_NC (1 << 29) /* Disable outstanding non cacheable rqst */
#define PJ4B_L1_REP_RR (1 << 30) /* L1 replacement - Strict round robin */
#define PJ4B_AUX_DBG_CTRL2 (PJ4B_SNOOP_DATA | PJ4B_CWF |\
		PJ4B_OUTSDNG_NC | PJ4B_L1_REP_RR)

/* Auxiliary Functional Modes Control Register 0 */
#define PJ4B_SMP_CFB (1 << 1) /* Set SMP mode. Join the coherency fabric */
#define PJ4B_L1_PAR_CHK (1 << 2) /* Support L1 parity checking */
#define PJ4B_BROADCAST_CACHE (1 << 8) /* Broadcast Cache and TLB maintenance */

/* Auxiliary Debug Modes Control 0 Register */
#define PJ4B_WFI_WFE (1 << 22) /* WFI/WFE - serve the DVM and back to idle */

static void cpu_fabric_common_init(void)
{
	unsigned reg;
	void __iomem *aurora_base;
	//void __iomem *coherency_base;
	//void __iomem *coherency_base2;
	struct device_node *np;

	np = of_find_compatible_node(NULL, NULL, "marvell,aurora-system-cache");
	if (!np) {
		return;
	}
	aurora_base = of_iomap(np, 0);
	of_node_put(np);

	//np = of_find_compatible_node(NULL, NULL, "marvell,coherency-fabric");
	//if (!np) {
	//	return;
	//}
	//coherency_base = of_iomap(np, 0);
	//coherency_base2 = of_iomap(np, 1);
	//of_node_put(np);

	// Taken from v7 proc-v7.S (__v7_pj4b_setup)
	/* Auxiliary Debug Modes Control 1 Register */
	__asm__("mrc p15, 1, %0, c15, c1, 1" : "=r" (reg));
	reg |= PJ4B_CLEAN_LINE;
	reg |= PJ4B_INTER_PARITY;
	reg &= ~PJ4B_STATIC_BP;
	__asm__("mcr p15, 1, %0, c15, c1, 1" : : "r" (reg));

	/* Auxiliary Debug Modes Control 2 Register */
	__asm__("mrc p15, 1, %0, c15, c1, 2" : "=r" (reg));
	reg &= ~PJ4B_FAST_LDR;
	reg |= PJ4B_AUX_DBG_CTRL2;
	__asm__("mcr p15, 1, %0, c15, c1, 2" : : "r" (reg));

	/* Auxiliary Functional Modes Control Register 0 */
	__asm__("mrc p15, 1, %0, c15, c2, 0" : "=r" (reg));
	reg |= PJ4B_SMP_CFB;
	reg |= PJ4B_L1_PAR_CHK;
	reg |= PJ4B_BROADCAST_CACHE;
	__asm__("mcr p15, 1, %0, c15, c2, 0" : : "r" (reg));

	__asm__("mrc p15, 1, %0, c15, c1, 0" : "=r" (reg));
	reg |= PJ4B_WFI_WFE;
	__asm__("mcr p15, 1, %0, c15, c1, 0" : : "r" (reg));

	// CONFIG_CACHE_AURORA_L2
	aurora_l2_init(aurora_base);

	// for arch_is_coherent
	hwcc = 1;
}

int __init mvebu_mbus_dt_init(bool is_coherent);
extern struct of_device_id msys_irq_match[];

static void __init mvebu_init_irq(void)
{
	of_irq_init(msys_irq_match);
	mvebu_scu_enable();
	cpu_fabric_common_init();
	coherency_init();
	BUG_ON(mvebu_mbus_dt_init(coherency_available()));
}

//static void __init external_abort_quirk(void)
//{
//	u32 dev, rev;
//
//	if (mvebu_get_soc_id(&dev, &rev) == 0 && rev > ARMADA_375_Z1_REV)
//		return;
//
//	hook_fault_code(16 + 6, armada_375_external_abort_wa, SIGBUS, 0,
//			"imprecise external abort");
//}
//
//static void __init i2c_quirk(void)
//{
//	//struct device_node *np;
//	u32 dev, rev;
//
//	/*
//	 * Only revisons more recent than A0 support the offload
//	 * mechanism. We can exit only if we are sure that we can
//	 * get the SoC revision and it is more recent than A0.
//	 */
//	if (mvebu_get_soc_id(&dev, &rev) == 0 && rev > MV78XX0_A0_REV)
//		return;
//
//        /*
//	for_each_compatible_node(np, NULL, "marvell,mv78230-i2c") {
//		struct property *new_compat;
//
//		new_compat = kzalloc(sizeof(*new_compat), GFP_KERNEL);
//
//		new_compat->name = kstrdup("compatible", GFP_KERNEL);
//		new_compat->length = sizeof("marvell,mv78230-a0-i2c");
//		new_compat->value = kstrdup("marvell,mv78230-a0-i2c",
//						GFP_KERNEL);
//
//		of_update_property(np, new_compat);
//	}
//        */
//	return;
//}
//
//static void __init mvebu_dt_init(void)
//{
//	if (of_machine_is_compatible("marvell,armadaxp"))
//		i2c_quirk();
//	if (of_machine_is_compatible("marvell,a375-db"))
//		external_abort_quirk();
//
//	of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);
//}
//
//static const char * const armada_370_xp_dt_compat[] __initconst = {
//	"marvell,armada-370-xp",
//	NULL,
//};
//
//DT_MACHINE_START(ARMADA_370_XP_DT, "Marvell Armada 370/XP (Device Tree)")
//	.l2c_aux_val	= 0,
//	.l2c_aux_mask	= ~0,
///*
// * The following field (.smp) is still needed to ensure backward
// * compatibility with old Device Trees that were not specifying the
// * cpus enable-method property.
// */
//	.smp		= smp_ops(armada_xp_smp_ops),
//	.init_machine	= mvebu_dt_init,
//	.init_irq       = mvebu_init_irq,
//	.restart	= mvebu_restart,
//	.reserve        = mvebu_memblock_reserve,
//	.dt_compat	= armada_370_xp_dt_compat,
//MACHINE_END
//
//static const char * const armada_375_dt_compat[] __initconst = {
//	"marvell,armada375",
//	NULL,
//};
//
//DT_MACHINE_START(ARMADA_375_DT, "Marvell Armada 375 (Device Tree)")
//	.l2c_aux_val	= 0,
//	.l2c_aux_mask	= ~0,
//	.init_irq       = mvebu_init_irq,
//	.init_machine	= mvebu_dt_init,
//	.restart	= mvebu_restart,
//	.dt_compat	= armada_375_dt_compat,
//MACHINE_END
//
//static const char * const armada_38x_dt_compat[] __initconst = {
//	"marvell,armada380",
//	"marvell,armada385",
//	NULL,
//};
//
//DT_MACHINE_START(ARMADA_38X_DT, "Marvell Armada 380/385 (Device Tree)")
//	.l2c_aux_val	= 0,
//	.l2c_aux_mask	= ~0,
//	.init_irq       = mvebu_init_irq,
//	.restart	= mvebu_restart,
//	.dt_compat	= armada_38x_dt_compat,
//MACHINE_END
//
//static const char * const armada_39x_dt_compat[] __initconst = {
//	"marvell,armada390",
//	"marvell,armada398",
//	NULL,
//};
//
//DT_MACHINE_START(ARMADA_39X_DT, "Marvell Armada 39x (Device Tree)")
//	.l2c_aux_val	= 0,
//	.l2c_aux_mask	= ~0,
//	.init_irq       = mvebu_init_irq,
//	.restart	= mvebu_restart,
//	.dt_compat	= armada_39x_dt_compat,
//MACHINE_END

static struct clk core_clk = {
	.rate = 200000000,
};

static struct clk_lookup lookup[] = {
	{
		.dev_id		= "f1010600.spi",
		.con_id 	= NULL,
		.clk		= &core_clk,
	},
	{
		.dev_id		= "f1011000.i2c",
		.con_id 	= NULL,
		.clk		= &core_clk,
	},
};

// Added from armada-370-xp.c
static void __init armada_370_xp_map_io(void)
{
	//debug_ll_io_init();
        // see debug-macro.S
	struct map_desc map;
	map.virtual = 0xFB012000;
	map.pfn = __phys_to_pfn(0xF1012000);
	map.length = PAGE_SIZE;
	map.type = MT_DEVICE;
	iotable_init(&map, 1);

	rb_mach = RB_MACH_MSYS;
	clkdev_add_table(lookup, ARRAY_SIZE(lookup));
}

static void __init armada_370_xp_spi_fix(void)
{
	struct device_node *np;
	struct property *old_prop;
	for_each_compatible_node(np, NULL, "marvell,orion-spi") {
		struct property *new_compat;

		new_compat = kzalloc(sizeof(*new_compat), GFP_KERNEL);

		new_compat->name = kstrdup("compatible", GFP_KERNEL);
		new_compat->length = sizeof("marvell,armada-370-spi");
		new_compat->value = kstrdup("marvell,armada-370-spi",
				GFP_KERNEL);

		old_prop = of_find_property(np, new_compat->name, NULL);
		prom_update_property(np, new_compat, old_prop);
	}
}

static void __init armada_370_xp_dt_init(void)
{
	armada_370_xp_spi_fix();
	of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);
}

static const char * const msys_dt_compat[] = {
	"marvell,msys",
	NULL,
};

void armada_370_xp_handle_irq(struct pt_regs *regs);

void __init msys_timer_init(void);
static struct sys_timer msys_timer = {
	.init	= msys_timer_init,
};

DT_MACHINE_START(MSYS_DT, "Marvell Msys (Device Tree)")
	.init_machine	= armada_370_xp_dt_init,
	.map_io		= armada_370_xp_map_io,
	.init_irq	= mvebu_init_irq,
	.timer		= &msys_timer,
        .handle_irq     = armada_370_xp_handle_irq,
        .nr_irqs        = 128 + 34 + 16,
	.restart	= mvebu_restart,
	.dt_compat	= msys_dt_compat,
MACHINE_END
