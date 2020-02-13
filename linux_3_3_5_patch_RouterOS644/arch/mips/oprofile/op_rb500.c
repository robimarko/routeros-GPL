#include <linux/interrupt.h>
#include <linux/oprofile.h>
#include <asm/io.h>
#include <asm/addrspace.h>

#define TIMER_BASE     ((unsigned *) KSEG1ADDR(0x18028000))
#define TIMER_COUNT0   (TIMER_BASE + 0)
#define TIMER_COMPARE0 (TIMER_BASE + 1)
#define TIMER_CTC0     (TIMER_BASE + 2)
#define TIMER_CTCSEL0  (TIMER_BASE + 3)

#define IRQ_TIMER 8

#define CYCLES 100000

extern unsigned int mips_hpt_frequency;

extern void rb500_enable_beeper(int enable);

static irqreturn_t rb500_oprofile_irq(int irq, void *p)
{
	oprofile_add_sample(get_irq_regs(), 0);

	writel(1, TIMER_CTC0);

	return IRQ_HANDLED;
}

static int rb500_oprofile_start(void)
{
	unsigned delay = mips_hpt_frequency / (CYCLES / 2);
	
	rb500_enable_beeper(0);
	request_irq(IRQ_TIMER, rb500_oprofile_irq, 0, "oprofile", 0);

	writel(0, TIMER_COUNT0);
	writel(delay, TIMER_COMPARE0);
	writel(0, TIMER_CTCSEL0);
	writel(1, TIMER_CTC0);

	return 0;
}

static void rb500_oprofile_stop(void)
{
	writel(0, TIMER_CTC0);

	free_irq(IRQ_TIMER, 0);
	rb500_enable_beeper(1);
}

int __init rb500_oprofile_init(struct oprofile_operations *ops)
{
	ops->create_files = NULL;
	ops->setup = NULL;
	ops->shutdown = NULL;
	ops->start = rb500_oprofile_start;
	ops->stop = rb500_oprofile_stop;
	ops->cpu_type = "timer";
	return 0;
}
