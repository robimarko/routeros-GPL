#include <linux/init.h>
#include <linux/linkage.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/bitops.h>
#include <linux/module.h>
#include <asm/signal.h>
#include <asm/mipsregs.h>
#include <asm/irq_cpu.h>
#include <asm/bootinfo.h>
#include <asm/vm.h>
#include <asm/rb/boards.h>

extern void rb500_init_irq(void);
extern void rb100_init_irq(void);
extern void rb400_init_irq(void);
extern void mmips_init_irq(void);
extern void music_arch_init_irq(void);
extern void cr_init_irq(void);

#ifndef CONFIG_MMIPS_MIKROTIK
asmlinkage void plat_irq_dispatch(void) {
	unsigned pending = read_c0_status() & read_c0_cause() & 0xfe00;

	if (pending)
		do_IRQ(fls(pending) - (9 - MIPS_CPU_IRQ_BASE));
}
#endif

volatile unsigned long virqs;
EXPORT_SYMBOL(virqs);

static void ack_virq(struct irq_data *d)
{
	clear_bit(d->irq - VIRQ_BASE, &virqs);
}

static inline void unmask_virq(struct irq_data *d)
{
}

static inline void mask_virq(struct irq_data *d)
{
}

static struct irq_chip virq_controller = {
	.name	= "virq",
	.irq_ack	= ack_virq,
	.irq_unmask     = unmask_virq,
	.irq_mask	= mask_virq,
};

static irqreturn_t virq_cascade_irq(int irq, void *dev_id)
{
	unsigned i;
	unsigned irqs = virqs;

	for (i = 0; irqs; ++i) {
		if (irqs & (1 << i)) {
			do_IRQ(i + VIRQ_BASE);
			irqs ^= (1 << i);
		}
	}
	return IRQ_HANDLED;
}

static struct irqaction virq_cascade  = {
	.handler = virq_cascade_irq,
	.name = "virq-cascade",
};

static void soft_irq_ack(struct irq_data *d)
{
	clear_c0_cause(0x100 << (d->irq - MIPS_CPU_IRQ_BASE));
}

static inline void unmask_soft_irq(struct irq_data *d)
 {
	set_c0_status(0x100 << (d->irq - MIPS_CPU_IRQ_BASE));
	irq_enable_hazard();
 }
 
static inline void mask_soft_irq(struct irq_data *d)
{
	clear_c0_status(0x100 << (d->irq - MIPS_CPU_IRQ_BASE));
	irq_disable_hazard();
}

static struct irq_chip soft_irq_controller = {
	.name	= "MIPS",
	.irq_ack	= soft_irq_ack,
	.irq_unmask     = unmask_soft_irq,
	.irq_mask	= mask_soft_irq,
};

extern void ont_arch_init_irq(void);

void __init arch_init_irq(void)
{
	unsigned i;

	switch (mips_machgroup) {
	case MACH_GROUP_MT_RB500:
		rb500_init_irq();
		break;
	case MACH_GROUP_MT_RB100:
		rb100_init_irq();
		break;
	case MACH_GROUP_MT_RB400:
	case MACH_GROUP_MT_RB700:
		rb400_init_irq();
		break;
	case MACH_GROUP_MT_CR:
		cr_init_irq();
		break;
	case MACH_GROUP_MT_MUSIC:
		music_arch_init_irq();
		break;
#ifdef CONFIG_MMIPS_MIKROTIK
	case MACH_GROUP_MT_MMIPS:
		mmips_init_irq();
		break;
#endif
	case MACH_GROUP_MT_VM:
		mips_cpu_irq_init();
		break;
	}

	if (mips_machgroup != MACH_GROUP_MT_RB500 &&
	    mips_machgroup != MACH_GROUP_MT_RB100) {
		irq_set_chip_and_handler(1, &soft_irq_controller, handle_percpu_irq);
		setup_irq(1, &virq_cascade);
		
		for (i = VIRQ_BASE;  i < VIRQ_BASE + 32; ++i)
			irq_set_chip_and_handler(i, &virq_controller,
						 handle_edge_irq);
	}
}
