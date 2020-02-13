#include <asm/irq_regs.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/hrtimer.h>
#include <linux/module.h>
#include <linux/oprofile.h>

static int polling;
static int interval;
module_param(polling, int, 0);
module_param(interval, int, 0);

static DEFINE_PER_CPU(bool, paused);
static DEFINE_PER_CPU(struct hrtimer, op_hrtimer);

static enum hrtimer_restart hrtimer_oprofile_timer(struct hrtimer *t)
{
	if (__get_cpu_var(paused)) {
		return HRTIMER_RESTART;
	}
	oprofile_add_sample(get_irq_regs(), 0);

	hrtimer_forward_now(t, ns_to_ktime(interval));

	return HRTIMER_RESTART;
}

static void hrtimer_oprofile_start_one(void *d)
{
	struct hrtimer *ht = &__get_cpu_var(op_hrtimer);

	hrtimer_init(ht, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	ht->function = hrtimer_oprofile_timer;

	hrtimer_start(ht, ns_to_ktime(interval), HRTIMER_MODE_REL_PINNED);
}
void hrtimer_oprofile_resume(void) {
	__get_cpu_var(paused) = false;
}
void hrtimer_oprofile_pause(void) {
	__get_cpu_var(paused) = true;
}
EXPORT_SYMBOL(hrtimer_oprofile_resume);
EXPORT_SYMBOL(hrtimer_oprofile_pause);

static int hrtimer_oprofile_start(void)
{
	on_each_cpu(hrtimer_oprofile_start_one, NULL, 1);
	return 0;
}

static void hrtimer_oprofile_stop(void)
{
	int cpu;

	for_each_online_cpu(cpu)
		hrtimer_cancel(&per_cpu(op_hrtimer, cpu));
}

int __init hrtimer_oprofile_init(struct oprofile_operations *ops)
{
	if (!polling) 
		return -ENODEV;
	if (!interval) {
		interval = 5000000;
	}

	ops->create_files = NULL;
	ops->setup = NULL;
	ops->shutdown = NULL;
	ops->start = hrtimer_oprofile_start;
	ops->stop = hrtimer_oprofile_stop;
	ops->cpu_type = "timer";
	return 0;
}
