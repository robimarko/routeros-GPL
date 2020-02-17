#include <linux/init.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/hardirq.h>
#include <linux/preempt.h>

#include <asm/irq_cpu.h>
#include <asm/mipsregs.h>

#include <asm/irq.h>
#include <asm/mach-ralink/surfboard.h>
#include <asm/mach-ralink/surfboardint.h>
#include <asm/mach-ralink/rt_mmap.h>

#include <asm/mach-ralink/eureka_ep430.h>

#if defined (CONFIG_IRQ_GIC)
#include <asm/gic.h>
#include <asm/gcmpregs.h>
#endif

extern int cp0_compare_irq;
void __init ralink_gpio_init_irq(void);

extern unsigned int gic_present;
int gcmp_present;
unsigned long _gcmp_base;

#define X GIC_UNUSED
/*
 * This GIC specific tabular array defines the association between External
 * Interrupts and CPUs/Core Interrupts. The nature of the External
 * Interrupts is also defined here - polarity/trigger.
 */
static struct gic_intr_map gic_intr_map[GIC_NUM_INTRS] = {
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI }, //0
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI }, 
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI }, 
        { 0, GIC_CPU_INT3, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI }, //FE
        { 0, GIC_CPU_INT4, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI }, //PCIE0

#if defined (CONFIG_RALINK_SYSTICK)
	{ 0, GIC_CPU_INT5, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },//5, aux timer(system tick)
#else
	{ X, X,            X,           X,              0 }, //5
#endif
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI }, 
	{ X, X,            X,           X,              GIC_UNUSED }, 
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },

        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI }, //10
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI }, 
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI }, 
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI }, //15
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },

        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI }, //20
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },
        { 0, GIC_CPU_INT4, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },

        { 0, GIC_CPU_INT4, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI }, //25
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },

        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI }, //30
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },

        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_EDGE, GIC_FLAG_IPI },//32: PCIE_P0_LINT_DOWN_RST
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_EDGE, GIC_FLAG_IPI },//33: PCIE_P1_LINT_DOWN_RST
        { 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_EDGE, GIC_FLAG_IPI },//34: PCIE_P2_LINT_DOWN_RST
        /* The remainder of this table is initialised by fill_ipi_map */
};
#undef X

/*
 * GCMP needs to be detected before any SMP initialisation
 */
int __init gcmp_probe(unsigned long addr, unsigned long size) {
	_gcmp_base = (unsigned long) ioremap_nocache(GCMP_BASE_ADDR, GCMP_ADDRSPACE_SZ);
        gcmp_present = (GCMPGCB(GCMPB) & GCMP_GCB_GCMPB_GCMPBASE_MSK) == GCMP_BASE_ADDR;

        if (gcmp_present)
                printk("GCMP present\n");

        return gcmp_present;

}

/* Return the number of IOCU's present */
int __init gcmp_niocu(void) {
	return gcmp_present ?
		(GCMPGCB(GC) & GCMP_GCB_GC_NUMIOCU_MSK) >> GCMP_GCB_GC_NUMIOCU_SHF :
		0;
}

void gic_irq_ack(struct irq_data *d) {
	int irq = (d->irq - gic_irq_base);

	GIC_CLR_INTR_MASK(irq);
	if (gic_irq_flags[irq] & GIC_TRIG_EDGE)
		GICWRITE(GIC_REG(SHARED, GIC_SH_WEDGE), irq);
}

void gic_finish_irq(struct irq_data *d) {
        /* Enable interrupts. */
        GIC_SET_INTR_MASK(d->irq - gic_irq_base);
}

void __init gic_platform_init(int irqs, struct irq_chip *irq_controller) {
	int i;
	for (i = gic_irq_base; i < (gic_irq_base + irqs); i++){
		irq_set_chip(i, irq_controller);
        }
}                                

static int gic_resched_int_base;
static int gic_call_int_base;

#define GIC_RESCHED_INT(cpu) (gic_resched_int_base+(cpu))
#define GIC_CALL_INT(cpu) (gic_call_int_base+(cpu))

static unsigned int ipi_map[NR_CPUS];
static irqreturn_t ipi_resched_interrupt(int irq, void *dev_id) {
        scheduler_ipi();
        return IRQ_HANDLED;
}

static irqreturn_t ipi_call_interrupt(int irq, void *dev_id) {
	smp_call_function_interrupt();
	return IRQ_HANDLED;
}

static struct irqaction irq_resched = {
        .handler        = ipi_resched_interrupt,
        .flags          = IRQF_DISABLED|IRQF_PERCPU,
        .name           = "IPI_resched"
};

static struct irqaction irq_call = {
        .handler        = ipi_call_interrupt,
        .flags          = IRQF_DISABLED|IRQF_PERCPU,
        .name           = "IPI_call"
};

unsigned int plat_ipi_call_int_xlate(unsigned int cpu) {
	return GIC_CALL_INT(cpu);
}

unsigned int plat_ipi_resched_int_xlate(unsigned int cpu) {
	return GIC_RESCHED_INT(cpu);
}

static void __init fill_ipi_map1(int baseintr, int cpu, int cpupin) {
        int intr = baseintr + cpu;
        gic_intr_map[intr].cpunum = cpu;
        gic_intr_map[intr].pin = cpupin;
        gic_intr_map[intr].polarity = GIC_POL_POS;
        gic_intr_map[intr].trigtype = GIC_TRIG_EDGE;
        gic_intr_map[intr].flags = GIC_FLAG_IPI;
        ipi_map[cpu] |= (1 << (cpupin + 2));
}

static void __init fill_ipi_map(void) {
        int cpu;

        for (cpu = 0; cpu < NR_CPUS; cpu++) {
                fill_ipi_map1(gic_resched_int_base, cpu, GIC_CPU_INT1);
                fill_ipi_map1(gic_call_int_base, cpu, GIC_CPU_INT2);
        }
}

static void __init arch_init_ipiirq(int irq, struct irqaction *action) {
	setup_irq(irq, action);
	irq_set_handler(irq, handle_percpu_irq);
}

void __init mmips_init_irq(void) {
	int i;
	/*
	 * Mask out all interrupt by writing "1" to all bit position in
	 * the interrupt reset reg.
	 */
	mips_cpu_irq_init();

	if (gcmp_present)  {
		GCMPGCB(GICBA) = GIC_BASE_ADDR | GCMP_GCB_GICBA_EN_MSK;
                gic_present = 1;
	}

	if (gic_present) {
                gic_call_int_base = GIC_NUM_INTRS - NR_CPUS;
                gic_resched_int_base = gic_call_int_base - NR_CPUS;
                fill_ipi_map();
		gic_init(GIC_BASE_ADDR, GIC_ADDRSPACE_SZ, gic_intr_map,
				ARRAY_SIZE(gic_intr_map), MIPS_GIC_IRQ_BASE);
		set_c0_status(STATUSF_IP7 | STATUSF_IP6 | STATUSF_IP5 | STATUSF_IP4 | STATUSF_IP3 | STATUSF_IP2);
                
		/* set up ipi interrupts */
                for (i = 0; i < NR_CPUS; i++) {
                        arch_init_ipiirq(MIPS_GIC_IRQ_BASE + GIC_RESCHED_INT(i), &irq_resched);
                        arch_init_ipiirq(MIPS_GIC_IRQ_BASE + GIC_CALL_INT(i), &irq_call);
                }
	}

	irq_set_handler(SURFBOARDINT_PCIE0, handle_level_irq);
	irq_set_handler(SURFBOARDINT_PCIE1, handle_level_irq);
	irq_set_handler(SURFBOARDINT_PCIE2, handle_level_irq);
	irq_set_handler(SURFBOARDINT_FE, handle_level_irq);
	irq_set_handler(SURFBOARDINT_USB, handle_level_irq);
	irq_set_handler(SURFBOARDINT_SYSCTL, handle_level_irq);
	irq_set_handler(SURFBOARDINT_DRAMC, handle_level_irq);
	irq_set_handler(SURFBOARDINT_PCM, handle_level_irq);
	irq_set_handler(SURFBOARDINT_HSGDMA, handle_level_irq);
	irq_set_handler(SURFBOARDINT_GPIO, handle_level_irq);
	irq_set_handler(SURFBOARDINT_DMA, handle_level_irq);
	irq_set_handler(SURFBOARDINT_NAND, handle_level_irq);
	irq_set_handler(SURFBOARDINT_I2S, handle_level_irq);
	irq_set_handler(SURFBOARDINT_SPI, handle_level_irq);
	irq_set_handler(SURFBOARDINT_SPDIF, handle_level_irq);
	irq_set_handler(SURFBOARDINT_CRYPTO, handle_level_irq);
	irq_set_handler(SURFBOARDINT_SDXC, handle_level_irq);
	irq_set_handler(SURFBOARDINT_PCTRL, handle_level_irq);
	irq_set_handler(SURFBOARDINT_ESW, handle_level_irq);
	irq_set_handler(SURFBOARDINT_UART_LITE1, handle_level_irq);
	irq_set_handler(SURFBOARDINT_UART_LITE2, handle_level_irq);
	irq_set_handler(SURFBOARDINT_UART_LITE3, handle_level_irq);
	irq_set_handler(SURFBOARDINT_NAND_ECC, handle_level_irq);
	irq_set_handler(SURFBOARDINT_I2C, handle_level_irq);
	irq_set_handler(SURFBOARDINT_WDG, handle_level_irq);
	irq_set_handler(SURFBOARDINT_TIMER0, handle_level_irq);
	irq_set_handler(SURFBOARDINT_TIMER1, handle_level_irq);

#ifdef CONFIG_RALINK_GPIO
	ralink_gpio_init_irq();
#endif
}

asmlinkage void plat_irq_dispatch(void) {
	unsigned int pending = read_c0_status() & read_c0_cause() & ST0_IM;
	if(pending & CAUSEF_IP7 ){
		do_IRQ(cp0_compare_irq);
	}
	if (pending & (CAUSEF_IP2 | CAUSEF_IP3 | CAUSEF_IP4 | CAUSEF_IP5 | CAUSEF_IP6)) {
		unsigned int irq = gic_get_int();
		if (likely(irq < GIC_NUM_INTRS))  {
			do_IRQ(MIPS_GIC_IRQ_BASE + irq);  
		}
	}	
}

