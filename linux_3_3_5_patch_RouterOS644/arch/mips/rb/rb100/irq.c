#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/irq_cpu.h>
#include <asm/irq.h>

#define MAX_IRQ_NR 10
#define IRQ_BASE 8

#define INTC_BASE 0x12200000

#define ADM5120_INTC_REG(reg) \
	(*((volatile unsigned *) (KSEG1ADDR(INTC_BASE + (reg)))))

#define INTC_IRQ_STATUS 0x00
#define INTC_IRQ_ENABLE 0x08
#define INTC_IRQ_DISABLE 0x0c

extern asmlinkage void rb100_IRQ(void);

static void rb100_mask_irq(struct irq_data *d)
{
	ADM5120_INTC_REG(INTC_IRQ_DISABLE) = 1 << (d->irq - IRQ_BASE);
}

static void rb100_unmask_irq(struct irq_data *d)
{
	ADM5120_INTC_REG(INTC_IRQ_ENABLE) = 1 << (d->irq - IRQ_BASE);
}

void rb100_irqdispatch(void)
{
	unsigned pending = read_c0_status() & read_c0_cause() & 0xfc00;
	unsigned long intsrc;
	int i;

	if (pending & CAUSEF_IP7) {
	    do_IRQ(7);
	    return;
	}
	if (!(pending & CAUSEF_IP2))
	    return;

	intsrc = ADM5120_INTC_REG(INTC_IRQ_STATUS) & ((1 << MAX_IRQ_NR) - 1);

	/* do not run softirqs on timer & OProfile irq */
	if (intsrc & 0x1) generic_handle_irq(IRQ_BASE + 0);

	for (i = IRQ_BASE + 1; intsrc; intsrc >>= 1, i++)
		if (intsrc & 0x2)
			do_IRQ(i);
}

static struct irq_chip rb100_irq_controller = {
	.name		= "ADM5120",
	.irq_ack		= rb100_mask_irq,
	.irq_mask		= rb100_mask_irq,
	.irq_mask_ack	        = rb100_mask_irq,
	.irq_unmask		= rb100_unmask_irq,
};

static struct irqaction cascade  = {
	.handler = no_action,
	.name = "cascade",
};

static struct irqaction beeper  = {
	.handler = no_action,
	.name = "beeper",
};

void __init rb100_init_irq(void)
{
	int i;

	mips_cpu_irq_init();
	set_except_vector(0, rb100_IRQ);
	
	for (i = IRQ_BASE; i < IRQ_BASE + MAX_IRQ_NR; i++)
		irq_set_chip_and_handler(i, &rb100_irq_controller,
					 handle_level_irq);

	setup_irq(2, &cascade);
	setup_irq(3, &beeper);
}
