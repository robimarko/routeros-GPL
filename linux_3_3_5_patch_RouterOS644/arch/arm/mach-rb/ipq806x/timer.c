/* * Copyright (c) 2012 Qualcomm Atheros, Inc. * */
/*
 * Copyright (C) 2007 Google, Inc.
 * Copyright (c) 2009-2013, The Linux Foundation. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/percpu.h>

#include <asm/localtimer.h>
#include <asm/mach/time.h>
#include <asm/hardware/gic.h>
#include <asm/sched_clock.h>
#include <asm/smp_plat.h>
#include <mach/msm_iomap.h>
#include <mach/irqs.h>
#include <mach/socinfo.h>
#include <mach/system.h>

#include "timer.h"

enum {
	MSM_TIMER_DEBUG_SYNC = 1U << 0,
};
static int msm_timer_debug_mask;
module_param_named(debug_mask, msm_timer_debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);

#define DG_TIMER_RATING 100

#ifndef MSM_TMR0_BASE
#define MSM_TMR0_BASE MSM_TMR_BASE
#endif

#define MSM_DGT_SHIFT (5)

#define TIMER_MATCH_VAL         0x0000
#define TIMER_COUNT_VAL         0x0004
#define TIMER_ENABLE            0x0008
#define TIMER_CLEAR             0x000C
#define DGT_CLK_CTL             0x0034
enum {
	DGT_CLK_CTL_DIV_1 = 0,
	DGT_CLK_CTL_DIV_2 = 1,
	DGT_CLK_CTL_DIV_3 = 2,
	DGT_CLK_CTL_DIV_4 = 3,
};
#define TIMER_STATUS            0x0088
#define TIMER_ENABLE_EN              1
#define TIMER_ENABLE_CLR_ON_MATCH_EN 2

#define LOCAL_TIMER 0
#define GLOBAL_TIMER 1

/*
 * global_timer_offset is added to the regbase of a timer to force the memory
 * access to come from the CPU0 region.
 */
static int global_timer_offset;
static int msm_global_timer;

#define NR_TIMERS ARRAY_SIZE(msm_clocks)

unsigned int gpt_hz = 32768;
unsigned int sclk_hz = 32768;

static struct msm_clock *clockevent_to_clock(struct clock_event_device *evt);
static irqreturn_t msm_timer_interrupt(int irq, void *dev_id);
static cycle_t msm_gpt_read(struct clocksource *cs);
static cycle_t msm_dgt_read(struct clocksource *cs);
static void msm_timer_set_mode(enum clock_event_mode mode,
			       struct clock_event_device *evt);
static int msm_timer_set_next_event(unsigned long cycles,
				    struct clock_event_device *evt);

enum {
	MSM_CLOCK_FLAGS_UNSTABLE_COUNT = 1U << 0,
	MSM_CLOCK_FLAGS_ODD_MATCH_WRITE = 1U << 1,
	MSM_CLOCK_FLAGS_DELAYED_WRITE_POST = 1U << 2,
};

struct msm_clock {
	struct clock_event_device   clockevent;
	struct clocksource          clocksource;
	unsigned int		    irq;
	void __iomem                *regbase;
	uint32_t                    freq;
	uint32_t                    shift;
	uint32_t                    flags;
	uint32_t                    write_delay;
	uint32_t                    rollover_offset;
	uint32_t                    index;
	void __iomem                *global_counter;
	void __iomem                *local_counter;
	uint32_t		    status_mask;
	union {
		struct clock_event_device		*evt;
		struct clock_event_device __percpu	**percpu_evt;
	};
};

enum {
	MSM_CLOCK_GPT,
	MSM_CLOCK_DGT,
};

struct msm_clock_percpu_data {
	uint32_t                  last_set;
	uint32_t                  sleep_offset;
	uint32_t                  alarm_vtime;
	uint32_t                  alarm;
	uint32_t                  non_sleep_offset;
	uint32_t                  in_sync;
	cycle_t                   stopped_tick;
	int                       stopped;
	uint32_t                  last_sync_gpt;
	u64                       last_sync_jiffies;
};

struct msm_timer_sync_data_t {
	struct msm_clock *clock;
	uint32_t         timeout;
	int              exit_sleep;
};

static struct msm_clock msm_clocks[] = {
	[MSM_CLOCK_GPT] = {
		.clockevent = {
			.name           = "gp_timer",
			.features       = CLOCK_EVT_FEAT_ONESHOT,
			.shift          = 32,
			.rating         = 200,
			.set_next_event = msm_timer_set_next_event,
			.set_mode       = msm_timer_set_mode,
		},
		.clocksource = {
			.name           = "gp_timer",
			.rating         = 200,
			.read           = msm_gpt_read,
			.mask           = CLOCKSOURCE_MASK(32),
			.flags          = CLOCK_SOURCE_IS_CONTINUOUS,
		},
		.irq = INT_GP_TIMER_EXP,
		.regbase = MSM_TMR_BASE + 0x4,
		.freq = 32768,
		.index = MSM_CLOCK_GPT,
		.write_delay = 9,
	},
	[MSM_CLOCK_DGT] = {
		.clockevent = {
			.name           = "dg_timer",
			.features       = CLOCK_EVT_FEAT_ONESHOT,
			.shift          = 32,
			.rating         = DG_TIMER_RATING,
			.set_next_event = msm_timer_set_next_event,
			.set_mode       = msm_timer_set_mode,
		},
		.clocksource = {
			.name           = "dg_timer",
			.rating         = DG_TIMER_RATING,
			.read           = msm_dgt_read,
			.mask           = CLOCKSOURCE_MASK(32),
			.flags          = CLOCK_SOURCE_IS_CONTINUOUS,
		},
		.irq = INT_DEBUG_TIMER_EXP,
		.regbase = MSM_TMR_BASE + 0x24,
		.index = MSM_CLOCK_DGT,
		.write_delay = 9,
	}
};

static DEFINE_PER_CPU(struct msm_clock_percpu_data[NR_TIMERS],
    msm_clocks_percpu);

static DEFINE_PER_CPU(struct msm_clock *, msm_active_clock);

static irqreturn_t msm_timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = *(struct clock_event_device **)dev_id;
	if (evt->event_handler == NULL)
		return IRQ_HANDLED;
	evt->event_handler(evt);
	return IRQ_HANDLED;
}

static uint32_t msm_read_timer_count(struct msm_clock *clock, int global)
{
	uint32_t t1, t2, t3;
	int loop_count = 0;
	void __iomem *addr = clock->regbase + TIMER_COUNT_VAL +
		global*global_timer_offset;

	if (!(clock->flags & MSM_CLOCK_FLAGS_UNSTABLE_COUNT))
		return __raw_readl_no_log(addr);

	t1 = __raw_readl_no_log(addr);
	t2 = __raw_readl_no_log(addr);
	if ((t2-t1) <= 1)
		return t2;
	while (1) {
		t1 = __raw_readl_no_log(addr);
		t2 = __raw_readl_no_log(addr);
		t3 = __raw_readl_no_log(addr);
		cpu_relax();
		if ((t3-t2) <= 1)
			return t3;
		if ((t2-t1) <= 1)
			return t2;
		if ((t2 >= t1) && (t3 >= t2))
			return t2;
		if (++loop_count == 5) {
			pr_err("msm_read_timer_count timer %s did not "
			       "stabilize: %u -> %u -> %u\n",
			       clock->clockevent.name, t1, t2, t3);
			return t3;
		}
	}
}

static cycle_t msm_gpt_read(struct clocksource *cs)
{
	struct msm_clock *clock = &msm_clocks[MSM_CLOCK_GPT];
	struct msm_clock_percpu_data *clock_state =
		&per_cpu(msm_clocks_percpu, 0)[MSM_CLOCK_GPT];

	if (clock_state->stopped)
		return clock_state->stopped_tick;

	return msm_read_timer_count(clock, GLOBAL_TIMER) +
		clock_state->sleep_offset;
}

static cycle_t msm_dgt_read(struct clocksource *cs)
{
	struct msm_clock *clock = &msm_clocks[MSM_CLOCK_DGT];
	struct msm_clock_percpu_data *clock_state =
		&per_cpu(msm_clocks_percpu, 0)[MSM_CLOCK_DGT];

	if (clock_state->stopped)
		return clock_state->stopped_tick >> clock->shift;

	return (msm_read_timer_count(clock, GLOBAL_TIMER) +
		clock_state->sleep_offset) >> clock->shift;
}

static struct msm_clock *clockevent_to_clock(struct clock_event_device *evt)
{
	int i;

	if (!is_smp())
		return container_of(evt, struct msm_clock, clockevent);

	for (i = 0; i < NR_TIMERS; i++)
		if (evt == &(msm_clocks[i].clockevent))
			return &msm_clocks[i];
	return &msm_clocks[msm_global_timer];
}

static int msm_timer_set_next_event(unsigned long cycles,
				    struct clock_event_device *evt)
{
	int i;
	struct msm_clock *clock;
	struct msm_clock_percpu_data *clock_state;
	uint32_t now;
	uint32_t alarm;
	int late;

	clock = clockevent_to_clock(evt);
	clock_state = &__get_cpu_var(msm_clocks_percpu)[clock->index];
	now = msm_read_timer_count(clock, LOCAL_TIMER);
	alarm = now + (cycles << clock->shift);
	if (clock->flags & MSM_CLOCK_FLAGS_ODD_MATCH_WRITE)
		while (now == clock_state->last_set)
			now = msm_read_timer_count(clock, LOCAL_TIMER);

	clock_state->alarm = alarm;
	__raw_writel(alarm, clock->regbase + TIMER_MATCH_VAL);

	if (clock->flags & MSM_CLOCK_FLAGS_DELAYED_WRITE_POST) {
		/* read the counter four extra times to make sure write posts
		   before reading the time */
		for (i = 0; i < 4; i++)
			__raw_readl_no_log(clock->regbase + TIMER_COUNT_VAL);
	}
	now = msm_read_timer_count(clock, LOCAL_TIMER);
	clock_state->last_set = now;
	clock_state->alarm_vtime = alarm + clock_state->sleep_offset;
	late = now - alarm;
	if (late >= (int)(-clock->write_delay << clock->shift) &&
	    late < clock->freq*5)
		return -ETIME;

	return 0;
}

static void msm_timer_set_mode(enum clock_event_mode mode,
			       struct clock_event_device *evt)
{
	struct msm_clock *clock;
	struct msm_clock **cur_clock;
	struct msm_clock_percpu_data *clock_state, *gpt_state;
	unsigned long irq_flags;
	struct irq_chip *chip;

	clock = clockevent_to_clock(evt);
	clock_state = &__get_cpu_var(msm_clocks_percpu)[clock->index];
	gpt_state = &__get_cpu_var(msm_clocks_percpu)[MSM_CLOCK_GPT];

	local_irq_save(irq_flags);

	switch (mode) {
	case CLOCK_EVT_MODE_RESUME:
	case CLOCK_EVT_MODE_PERIODIC:
		break;
	case CLOCK_EVT_MODE_ONESHOT:
		clock_state->stopped = 0;
		clock_state->sleep_offset =
			-msm_read_timer_count(clock, LOCAL_TIMER) +
			clock_state->stopped_tick;
		get_cpu_var(msm_active_clock) = clock;
		put_cpu_var(msm_active_clock);
		__raw_writel(TIMER_ENABLE_EN, clock->regbase + TIMER_ENABLE);
		chip = irq_get_chip(clock->irq);
		if (chip && chip->irq_unmask)
			chip->irq_unmask(irq_get_irq_data(clock->irq));
		if (clock != &msm_clocks[MSM_CLOCK_GPT])
			__raw_writel(TIMER_ENABLE_EN,
				msm_clocks[MSM_CLOCK_GPT].regbase +
			       TIMER_ENABLE);
		break;
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
		cur_clock = &get_cpu_var(msm_active_clock);
		if (*cur_clock == clock)
			*cur_clock = NULL;
		put_cpu_var(msm_active_clock);
		clock_state->in_sync = 0;
		clock_state->stopped = 1;
		clock_state->stopped_tick =
			msm_read_timer_count(clock, LOCAL_TIMER) +
			clock_state->sleep_offset;
		__raw_writel(0, clock->regbase + TIMER_MATCH_VAL);
		chip = irq_get_chip(clock->irq);
		if (chip && chip->irq_mask)
			chip->irq_mask(irq_get_irq_data(clock->irq));

		if (!is_smp() || clock != &msm_clocks[MSM_CLOCK_DGT]
				|| smp_processor_id())
			__raw_writel(0, clock->regbase + TIMER_ENABLE);

		if (msm_global_timer == MSM_CLOCK_DGT &&
		    clock != &msm_clocks[MSM_CLOCK_GPT]) {
			gpt_state->in_sync = 0;
			__raw_writel(0, msm_clocks[MSM_CLOCK_GPT].regbase +
			       TIMER_ENABLE);
		}
		break;
	}
	wmb();
	local_irq_restore(irq_flags);
}

void __iomem *msm_timer_get_timer0_base(void)
{
	return MSM_TMR_BASE + global_timer_offset;
}

#define MPM_SCLK_COUNT_VAL    0x0024


static u32 notrace msm_read_sched_clock(void)
{
	struct msm_clock *clock = &msm_clocks[msm_global_timer];
	struct clocksource *cs = &clock->clocksource;
	return cs->read(NULL);
}

int read_current_timer(unsigned long *timer_val)
{
	if (rb_mach != RB_MACH_IPQ806X) {
		*timer_val = 0;
		return -ENODEV;
	}
	struct msm_clock *dgt = &msm_clocks[MSM_CLOCK_DGT];
	*timer_val = msm_read_timer_count(dgt, GLOBAL_TIMER);
	return 0;
}

static void __init msm_sched_clock_init(void)
{
	struct msm_clock *clock = &msm_clocks[msm_global_timer];

	setup_sched_clock(msm_read_sched_clock, 32 - clock->shift, clock->freq);
}

#ifdef CONFIG_LOCAL_TIMERS
static int __cpuinit local_timer_setup(struct clock_event_device *evt)
{
	static DEFINE_PER_CPU(bool, first_boot) = true;
	struct msm_clock *clock = &msm_clocks[msm_global_timer];

	/* Use existing clock_event for cpu 0 */
	if (!smp_processor_id())
		return 0;

	__raw_writel(DGT_CLK_CTL_DIV_4, MSM_TMR_BASE + DGT_CLK_CTL);

	if (__get_cpu_var(first_boot)) {
		__raw_writel(0, clock->regbase  + TIMER_ENABLE);
		__raw_writel(0, clock->regbase + TIMER_CLEAR);
		__raw_writel(~0, clock->regbase + TIMER_MATCH_VAL);
		__get_cpu_var(first_boot) = false;
		if (clock->status_mask)
			while (__raw_readl(MSM_TMR_BASE + TIMER_STATUS) &
			       clock->status_mask)
				;
	}
	evt->irq = clock->irq;
	evt->name = "local_timer";
	evt->features = CLOCK_EVT_FEAT_ONESHOT;
	evt->rating = clock->clockevent.rating;
	evt->set_mode = msm_timer_set_mode;
	evt->set_next_event = msm_timer_set_next_event;
	evt->shift = clock->clockevent.shift;
	evt->mult = div_sc(clock->freq, NSEC_PER_SEC, evt->shift);
	evt->max_delta_ns =
		clockevent_delta2ns(0xf0000000 >> clock->shift, evt);
	evt->min_delta_ns = clockevent_delta2ns(4, evt);

	*__this_cpu_ptr(clock->percpu_evt) = evt;

	clockevents_register_device(evt);
	enable_percpu_irq(evt->irq, IRQ_TYPE_EDGE_RISING);

	return 0;
}

static void local_timer_stop(struct clock_event_device *evt)
{
	evt->set_mode(CLOCK_EVT_MODE_UNUSED, evt);
	disable_percpu_irq(evt->irq);
}

static struct local_timer_ops msm_lt_ops = {
	local_timer_setup,
	local_timer_stop,
};
#endif /* CONFIG_LOCAL_TIMERS */

static void __init msm_timer_init(void)
{
	int i;
	int res;
	struct irq_chip *chip;
	struct msm_clock *dgt = &msm_clocks[MSM_CLOCK_DGT];
	struct msm_clock *gpt = &msm_clocks[MSM_CLOCK_GPT];

	global_timer_offset = MSM_TMR0_BASE - MSM_TMR_BASE;
	dgt->freq = 6750000;
	__raw_writel(DGT_CLK_CTL_DIV_4, MSM_TMR_BASE + DGT_CLK_CTL);
	gpt->status_mask = BIT(10);
	dgt->status_mask = BIT(2);
	gpt->freq = 32010;
	gpt_hz = 32010;
	sclk_hz = 32010;

	if (msm_clocks[MSM_CLOCK_GPT].clocksource.rating > DG_TIMER_RATING)
		msm_global_timer = MSM_CLOCK_GPT;
	else
		msm_global_timer = MSM_CLOCK_DGT;

	for (i = 0; i < ARRAY_SIZE(msm_clocks); i++) {
		struct msm_clock *clock = &msm_clocks[i];
		struct clock_event_device *ce = &clock->clockevent;
		struct clocksource *cs = &clock->clocksource;
		__raw_writel(0, clock->regbase + TIMER_ENABLE);
		__raw_writel(0, clock->regbase + TIMER_CLEAR);
		__raw_writel(~0, clock->regbase + TIMER_MATCH_VAL);

		if ((clock->freq << clock->shift) == gpt_hz) {
			clock->rollover_offset = 0;
		} else {
			uint64_t temp;

			temp = clock->freq << clock->shift;
			temp <<= 32;
			do_div(temp, gpt_hz);

			clock->rollover_offset = (uint32_t) temp;
		}

		ce->mult = div_sc(clock->freq, NSEC_PER_SEC, ce->shift);
		/* allow at least 10 seconds to notice that the timer wrapped */
		ce->max_delta_ns =
			clockevent_delta2ns(0xf0000000 >> clock->shift, ce);
		/* ticks gets rounded down by one */
		ce->min_delta_ns =
			clockevent_delta2ns(clock->write_delay + 4, ce);
		ce->cpumask = cpumask_of(0);

		res = clocksource_register_hz(cs, clock->freq);
		if (res)
			printk(KERN_ERR "msm_timer_init: clocksource_register "
			       "failed for %s\n", cs->name);

		ce->irq = clock->irq;
		clock->percpu_evt = alloc_percpu(struct clock_event_device *);
		if (!clock->percpu_evt) {
			pr_err("msm_timer_init: memory allocation "
			       "failed for %s\n", ce->name);
			continue;
		}

		*__this_cpu_ptr(clock->percpu_evt) = ce;
		res = request_percpu_irq(ce->irq, msm_timer_interrupt,
					 ce->name, clock->percpu_evt);
		if (!res)
			enable_percpu_irq(ce->irq,
					  IRQ_TYPE_EDGE_RISING);

		if (res)
			pr_err("msm_timer_init: request_irq failed for %s\n",
			       ce->name);

		chip = irq_get_chip(clock->irq);
		if (chip && chip->irq_mask)
			chip->irq_mask(irq_get_irq_data(clock->irq));

		if (clock->status_mask)
			while (__raw_readl(MSM_TMR_BASE + TIMER_STATUS) &
			       clock->status_mask)
				;

		clockevents_register_device(ce);
	}
	msm_sched_clock_init();

#ifdef ARCH_HAS_READ_CURRENT_TIMER
	if (is_smp()) {
		__raw_writel(1,
			msm_clocks[MSM_CLOCK_DGT].regbase + TIMER_ENABLE);
		set_delay_fn(read_current_timer_delay_loop);
	}
#endif

#ifdef CONFIG_LOCAL_TIMERS
	local_timer_register(&msm_lt_ops);
#endif
}

struct sys_timer msm_timer = {
	.init = msm_timer_init
};
