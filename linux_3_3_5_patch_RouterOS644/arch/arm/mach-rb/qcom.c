#include <linux/init.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/clocksource.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/module.h>
#include <linux/clkdev.h>
#include <linux/memblock.h>
#include <soc/qcom/scm.h>
#include <asm/hardware/gic.h>
#include <asm/mach/time.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <mach/system.h>
#include <mach/clk-provider.h>
#include <linux/gpio.h>
#include <linux/leds.h>
#include <mach/msm_iomap.h>
#include <linux/ioport.h>
#include <linux/poeout-rb.h>

#define SCM_BOOT_ADDR			0x1
#define SCM_FLAG_COLDBOOT_CPU1		0x01
#define SCM_FLAG_COLDBOOT_CPU2		0x08
#define SCM_FLAG_COLDBOOT_CPU3		0x20

void qcom_boot_secondary(unsigned int cpu) {
	struct device_node *node;
	void __iomem *base;
	struct resource res;

	node = of_find_compatible_node(NULL, NULL, "qcom,arm-cortex-acc");
	if (!node) {
		pr_err("%s: can't find node\n", __func__);
		return;
	}

	if (of_address_to_resource(node, 0, &res)) {
		of_node_put(node);
		return;
	}

	res.start += cpu * 0x10000;
	base = ioremap(res.start, 0x1000);
	of_node_put(node);

	if (!base)
		return;

	/* Enable Clamp signal and assert core reset */
	writel_relaxed(0x00000033, base + 0x04);
	mb();
	/* Set GDHS and delay counter */
	writel_relaxed(0x20000001, base + 0x14);
	mb();
	udelay(2);
	/* Enable Core memory HS */
	writel_relaxed(0x00020008, base + 0x04);
	mb();
	/* Report that the CPU is powered up */
	writel_relaxed(0x00020088, base + 0x04);
	mb();

	iounmap(base);
}

extern void secondary_startup(void);
void __init qcom_smp_prepare_cpus(void) {
	struct {
		unsigned int flags;
		phys_addr_t  addr;
	} cmd;
	cmd.addr = virt_to_phys(secondary_startup);
	cmd.flags = SCM_FLAG_COLDBOOT_CPU1 | SCM_FLAG_COLDBOOT_CPU2 | SCM_FLAG_COLDBOOT_CPU3;
	if (scm_call(SCM_SVC_BOOT, SCM_BOOT_ADDR, &cmd, sizeof(cmd), NULL, 0)) {
		pr_warn("Failed to set CPU boot address, disabling SMP\n");
	}
}

static const char * const qcom_dt_match[] __initconst = {
	"qcom,ipq40xx",
	NULL
};

void init_qcom_clock(void);

static void __init global_counter_enable(void) {
	struct device_node *node;
	void __iomem *base;

	node = of_find_compatible_node(NULL, NULL, "qcom,qca-gcnt");
	if (!node)
		pr_err("%s:can't find node\n", __func__);

	base = of_iomap(node, 0);
	of_node_put(node);
	if (!base)
		pr_err("%s:no regs for global counter\n", __func__);

	writel_relaxed(1, base);
	mb();
	iounmap(base);

#ifdef CONFIG_COMMON_CLK
	of_clk_init(NULL);
#endif
	clocksource_of_init();
}

static struct sys_timer qcom_timer = {
	.init = global_counter_enable,
};

extern struct clk dummy_clk;

static struct clk_lookup lookup[] = {
	{
		.con_id		= "core_clk",
		.dev_id		= "78b5000.spi",
		.clk		= &dummy_clk,
	},
	{
	    .con_id		= "iface_clk",
		.dev_id		= "78b5000.spi",
		.clk		= &dummy_clk,
	},
};

static void __init qcom_map_io(void) {
	/* Needed for early printk to work */
	struct map_desc map2[2];
	map2[0].pfn = 0x078af000;
	map2[0].virtual = 0xf78af000;
	map2[0].pfn = __phys_to_pfn(map2[0].pfn);
	map2[0].virtual &= PAGE_MASK;
	map2[0].length = PAGE_SIZE;
	map2[0].type = MT_DEVICE;

	/* TLMM */
	map2[1].pfn = __phys_to_pfn(0x1000000);
	map2[1].virtual = (unsigned)MSM_TLMM_BASE & PAGE_MASK;
	map2[1].length = SZ_4M;
	map2[1].type = MT_DEVICE_NONSHARED;

	iotable_init(map2, ARRAY_SIZE(map2));


	/* use max available DMA size (14 MB currently) */
	init_consistent_dma_size(CONSISTENT_END - VMALLOC_END);

	rb_mach = RB_MACH_IPQ40XX;
	init_qcom_clock();
	clkdev_add_table(lookup, ARRAY_SIZE(lookup));
}

static const struct of_device_id irq_match[] = {
	{ .compatible = "qcom,msm-qgic2", .data = gic_of_init, },
	{}
};

static void __init qcom_init_irq(void) {
	of_irq_init(irq_match);
}

static void qcom_restart(char mode, const char *cmd) {
	u32 pshold_base, wdt_base;
	struct device_node *node, *node_wdt;
	struct resource res, res_wdt;
	node = of_find_compatible_node(NULL, NULL, "qcom,pshold");
	node_wdt = of_find_compatible_node(NULL, NULL, "qcom,kpss-wdt-ipq40xx");

	if (!node || !node_wdt) {
		pr_err("%s: can't find node\n", __func__);
		return;
	}
	if (of_address_to_resource(node, 0, &res)) {
		of_node_put(node);
		return;
	}
	if (of_address_to_resource(node_wdt, 0, &res_wdt)) {
		of_node_put(node_wdt);
		return;
	}

	if(!request_mem_region(res.start, resource_size(&res),"pshold")
	|| !request_mem_region(res_wdt.start, resource_size(&res_wdt),"wdt")){
		pr_err("%s: failed to map specific region\n", __func__);
	}

	pshold_base = (unsigned)of_iomap(node, 0);
	wdt_base = (unsigned)of_iomap(node_wdt, 0);

	writel(0, pshold_base); //pshold

	msleep(1000);

	writel(0x00000000, wdt_base + 0x8); //disable
	writel(0x00000001, wdt_base + 0x4); //reset
	writel(0xffffffff, wdt_base + 0x10); //bark
	writel(0x00001000, wdt_base + 0x14); //bite
	writel(0x00000001, wdt_base + 0x8); //enable

	while (1) { }
}

static void disable_early_watchdog(void) {
	unsigned wdt_base;
	struct device_node *node;
	struct resource res;
	node = of_find_compatible_node(NULL, NULL, "qcom,kpss-wdt-ipq40xx");
	if (!node) return;
	if (of_address_to_resource(node, 0, &res)) {
		of_node_put(node);
		return;
	}
	request_mem_region(res.start, resource_size(&res),"wdt");
	wdt_base = (unsigned)of_iomap(node, 0);
	writel(1, wdt_base + 0x4); //reset
	writel(0, wdt_base + 0x8); //disable
}

static void __init qcom_init(void) {
	arm_pm_restart = qcom_restart;
	of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);
	disable_early_watchdog();
}

static void __init qcom_reserve(void) {
	memblock_reserve(0x87000000, 0x01000000);
}

DT_MACHINE_START(QCOM_QCA_DT, "Qualcomm (Flattened Device Tree)")
	.map_io = qcom_map_io,
	.dt_compat = qcom_dt_match,
	.init_irq = qcom_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &qcom_timer,
	.init_machine = qcom_init,
	.reserve = qcom_reserve,
MACHINE_END

