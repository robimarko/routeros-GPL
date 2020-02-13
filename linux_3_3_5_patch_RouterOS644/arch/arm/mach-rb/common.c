#include <linux/export.h>
#include <linux/io.h>
#include <linux/of.h>
#include <asm/smp.h>
#include <asm/hardware/gic.h>
#include <mach/clk-provider.h>
#include <mach/system.h>

int rb_mach;
EXPORT_SYMBOL(rb_mach);

/*
int clk_enable(struct clk *clk) {
	return 0;
}
EXPORT_SYMBOL(clk_enable);

void clk_disable(struct clk *clk) {
}
EXPORT_SYMBOL(clk_disable);

unsigned long clk_get_rate(struct clk *clk) {
	return clk->rate;
}
EXPORT_SYMBOL(clk_get_rate);
*/


static DEFINE_SPINLOCK(boot_lock);

void __cpuinit armada_xp_secondary_init(unsigned int cpu);
void __cpuinit platform_secondary_init(unsigned cpu) {
	if (rb_mach == RB_MACH_MSYS) {
		armada_xp_secondary_init(cpu);
	} else {
		gic_secondary_init(0);
		spin_lock(&boot_lock);
		spin_unlock(&boot_lock);
	}
}

void alpine_boot_secondary(unsigned cpu);
void qcom_boot_secondary(unsigned cpu);
int armada_xp_boot_secondary(unsigned int cpu, struct task_struct *idle);
void ipq806x_boot_secondary(unsigned cpu);
int __cpuinit boot_secondary(unsigned cpu, struct task_struct *idle) {
	spin_lock(&boot_lock);

	switch (rb_mach) {
	case RB_MACH_ALPINE:
		alpine_boot_secondary(cpu);
		break;
	case RB_MACH_IPQ40XX:
		qcom_boot_secondary(cpu);
		break;
	case RB_MACH_IPQ806X:
		ipq806x_boot_secondary(cpu);
		break;
	case RB_MACH_MSYS:
		armada_xp_boot_secondary(cpu, idle);
		break;
	};

	if (rb_mach != RB_MACH_MSYS)
		gic_raise_softirq(cpumask_of(cpu), 0);
	spin_unlock(&boot_lock);
	return 0;
}

int __init msys_get_core_count(void) {
	struct device_node *np = of_find_node_by_name(NULL, "rb");
	u32 ncores = 1;
	if (!np)
		return 1;
	of_property_read_u32(np, "core-count", &ncores);
	of_node_put(np);
	return ncores;
}

void __init smp_init_cpus(void) {
	unsigned i, ncores = 1;

	switch (rb_mach) {
	case RB_MACH_ALPINE:
	case RB_MACH_IPQ40XX:
		asm volatile("mrc p15, 1, %0, c9, c0, 2\n" : "=r" (ncores));
		ncores = ((ncores >> 24) & 3) + 1;
		break;
	case RB_MACH_IPQ806X:
		ncores = 2;
		break;
	case RB_MACH_MSYS:
		ncores = msys_get_core_count();
		break;
	};

	for (i = 0; i < ncores; i++)
		set_cpu_possible(i, true);

	if (rb_mach != RB_MACH_MSYS)
		set_smp_cross_call(gic_raise_softirq);
}

void __init alpine_smp_prepare_cpus(void);
void __init qcom_smp_prepare_cpus(void);
void __init armada_xp_smp_prepare_cpus(unsigned int max_cpus);
void __init platform_smp_prepare_cpus(unsigned int max_cpus) {
	int i;
	switch (rb_mach) {
	case RB_MACH_ALPINE:
		alpine_smp_prepare_cpus();
		break;
	case RB_MACH_IPQ40XX:
		qcom_smp_prepare_cpus();
		break;
	case RB_MACH_MSYS:
		armada_xp_smp_prepare_cpus(max_cpus);
		break;
	};

	for (i = 0; i < max_cpus; i++)
		set_cpu_present(i, true);
}

void alpine_teardown_msi_irq(unsigned int irq);
void ipq806x_teardown_msi_irq(unsigned int irq);
void ipq40xx_teardown_msi_irq(unsigned int irq);
int alpine_setup_msi_irq(struct pci_dev *pdev, struct msi_desc *desc);
int ipq806x_setup_msi_irq(struct pci_dev *pdev, struct msi_desc *desc);
int ipq40xx_setup_msi_irq(struct pci_dev *pdev, struct msi_desc *desc);
void arch_teardown_msi_irq(unsigned int irq) {
	switch (rb_mach) {
	case RB_MACH_ALPINE:
		return alpine_teardown_msi_irq(irq);
	case RB_MACH_IPQ806X:
		return ipq806x_teardown_msi_irq(irq);
	case RB_MACH_IPQ40XX:
		return ipq40xx_teardown_msi_irq(irq);
	case RB_MACH_MSYS:
		break;
	};
}
int arch_setup_msi_irq(struct pci_dev *pdev, struct msi_desc *desc) {
	switch (rb_mach) {
	case RB_MACH_ALPINE:
		return alpine_setup_msi_irq(pdev, desc);
	case RB_MACH_IPQ806X:
		return ipq806x_setup_msi_irq(pdev, desc);
	case RB_MACH_IPQ40XX:
		return ipq40xx_setup_msi_irq(pdev, desc);
	case RB_MACH_MSYS:
		break;
	};
	return -1;
}
