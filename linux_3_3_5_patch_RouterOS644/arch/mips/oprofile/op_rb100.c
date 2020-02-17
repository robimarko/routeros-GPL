#include <linux/interrupt.h>
#include <linux/oprofile.h>

#define IRQ_TIMER 0

#define SWCTRL_BASE 0x12000000
#define ADM5120_SW_REG(reg) \
        (*((volatile unsigned *) (KSEG1ADDR(SWCTRL_BASE + (reg)))))

#define SW_TIMER_INT		0x00f0
#define SW_TIMER		0x00f4

#define CYCLES 100000
#define OPROFILE_FREQ (175000000 / CYCLES)

extern void rb100_enable_beeper(int enable);

static irqreturn_t rb100_oprofile_irq(int irq, void *p)
{
	oprofile_add_sample(get_irq_regs(), 0);

	ADM5120_SW_REG(SW_TIMER) = (1 << 16) | (1562500 / OPROFILE_FREQ);
	ADM5120_SW_REG(SW_TIMER_INT) = 1;

	return IRQ_HANDLED;
}

static int rb100_oprofile_start(void)
{
	rb100_enable_beeper(0);
	request_irq(IRQ_TIMER, rb100_oprofile_irq, 0, "oprofile", 0);

	ADM5120_SW_REG(SW_TIMER) = (1 << 16) | (1562500 / OPROFILE_FREQ);
	ADM5120_SW_REG(SW_TIMER_INT) = 1;

	return 0;
}

static void rb100_oprofile_stop(void)
{
	ADM5120_SW_REG(SW_TIMER_INT) = 1 << 16;
	ADM5120_SW_REG(SW_TIMER) = 0;

	free_irq(IRQ_TIMER, 0);
	rb100_enable_beeper(1);
}

int __init rb100_oprofile_init(struct oprofile_operations *ops)
{
	ops->create_files = NULL;
	ops->setup = NULL;
	ops->shutdown = NULL;
	ops->start = rb100_oprofile_start;
	ops->stop = rb100_oprofile_stop;
	ops->cpu_type = "timer";
	return 0;
}
