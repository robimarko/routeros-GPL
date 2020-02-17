/*
 * Copyright 2012 Tilera Corporation. All Rights Reserved.
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation, version 2.
 *
 *   This program is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, GOOD TITLE or
 *   NON INFRINGEMENT.  See the GNU General Public License for
 *   more details.
 */

#include <linux/oprofile.h>
#include <linux/init.h>
#include <linux/smp.h>
#include <linux/kernel.h>
#include <linux/percpu.h>
#include <linux/delay.h>
#include <linux/ptrace.h>
#include <linux/timex.h>
#include <asm/system.h>
#include <asm/pmc.h>

#include <arch/chip.h>
#include <arch/interrupts.h>
#include <arch/spr_def.h>

#include "op_impl.h"

#if CHIP_HAS_AUX_PERF_COUNTERS()
#define TILE_NUM_PERF_COUNTERS	4
#else
#define TILE_NUM_PERF_COUNTERS	2
#endif

extern int __init hrtimer_oprofile_init(struct oprofile_operations *);

/* per-cpu configurations are copies of this master */
static struct op_counter_config master_ctr[TILE_NUM_PERF_COUNTERS];

static DEFINE_PER_CPU(struct op_counter_config[TILE_NUM_PERF_COUNTERS], ctr);
static DEFINE_PER_CPU(u32, op_counters_enabled);

static char op_tile_cpu_type[] = "tile/" CHIP_ARCH_NAME;

static atomic_t op_stop_cpus, op_start_cpus, op_setup_cpus;

/*
 * For each cpu, track how many cycles have been spent in the interrupt
 * handler, and when we started profiling.  This lets us compute if we
 * are pushing the limits of profiling.
 */
struct profile_time {
	u64 interrupt_cycles;
	u64 profile_start_time;
};
static DEFINE_PER_CPU(struct profile_time, profile_time);

/*
 * By just tracking time spent in the C code portion of the interrupt,
 * we don't track the assembler overhead, e.g. register save/restore.
 * Provide an approximation of that cost here, in cycles.
 */
#define ASM_INTERRUPT_OVERHEAD 200

/* How much time in oprofile is too much? */
static inline int too_much_oprofiling(u64 interrupt_cycles, u64 total_cycles)
{
	return (total_cycles > 1000000000ULL &&
		(interrupt_cycles > total_cycles / 2));
}

/*
 * Compute how much time we spent in this interrupt and aggregate it
 * to the total so far.  If this exceeds 50% of the total time that has
 * elapsed, turn off profiling.
 */
static int ratelimit_oprofile(u64 entry_time)
{
	struct profile_time *p = &__get_cpu_var(profile_time);
	u64 now = get_cycles();
	u64 total_cycles = now - p->profile_start_time;

	p->interrupt_cycles += (now - entry_time) + ASM_INTERRUPT_OVERHEAD;
	if (too_much_oprofiling(p->interrupt_cycles, total_cycles)) {
		pr_err("Disabling oprofiling on cpu %d;"
		       " more than %llu interrupt cycles out of %llu\n",
		       smp_processor_id(), p->interrupt_cycles, total_cycles);
		__get_cpu_var(op_counters_enabled) = 0;
		mask_pmc_interrupts();
		return -EINVAL;
	}
	return 0;
}

int op_handle_perf_interrupt(struct pt_regs *regs, int fault_num)
{
	unsigned long status;
	u64 entry_time = get_cycles();

	status = pmc_get_overflow();
	pmc_ack_overflow(status);

	if (!__get_cpu_var(op_counters_enabled))
		return -EINVAL;

	if (status & 1) {
		oprofile_add_sample(regs, 0);
		__insn_mtspr(SPR_PERF_COUNT_0,
			     __get_cpu_var(ctr)[0].reset_value);
	}
	if (status & 2) {
		oprofile_add_sample(regs, 1);
		__insn_mtspr(SPR_PERF_COUNT_1,
			     __get_cpu_var(ctr)[1].reset_value);
	}

#if CHIP_HAS_AUX_PERF_COUNTERS()
	if (status & 4) {
		oprofile_add_sample(regs, 2);
		__insn_mtspr(SPR_AUX_PERF_COUNT_0,
			     __get_cpu_var(ctr)[2].reset_value);
	}
	if (status & 8) {
		oprofile_add_sample(regs, 3);
		__insn_mtspr(SPR_AUX_PERF_COUNT_1,
			     __get_cpu_var(ctr)[3].reset_value);
	}
#endif
	return ratelimit_oprofile(entry_time);
}

static inline void op_tile_cpu_stop(void *dummy)
{
	unsigned long perf_count_ctl;

	get_cpu_var(op_counters_enabled) = 0;
	mask_pmc_interrupts();
	put_cpu_var(op_counters_enabled);
	atomic_dec(&op_stop_cpus);

	/* stop both kernel counting and user countering */
#ifndef __tilegx__
	perf_count_ctl = 3 << 7;
#else
	perf_count_ctl = 3 << 10;
#endif
	perf_count_ctl = (perf_count_ctl << 16) | perf_count_ctl;

	__insn_mtspr(SPR_PERF_COUNT_CTL, perf_count_ctl);
#if CHIP_HAS_AUX_PERF_COUNTERS()
	__insn_mtspr(SPR_AUX_PERF_COUNT_CTL, perf_count_ctl);
#endif
}

static void op_tile_stop(void)
{
	unsigned int timeout;

	atomic_set(&op_stop_cpus, num_online_cpus() - 1);

	(void)smp_call_function(op_tile_cpu_stop, NULL, 1);
	op_tile_cpu_stop(NULL);

	timeout = 1000;
	while ((atomic_read(&op_stop_cpus) > 0) && timeout) {
		mdelay(1);
		timeout--;
	}

	if (!timeout)
		printk(KERN_ERR "Not all tiles are stopped after "
		       "oprofile stops");
}

static void op_tile_cpu_start(void *dummy)
{
	get_cpu();

	/* enable the perf counter interrupt */
	__insn_mtspr(SPR_PERF_COUNT_0, 0);
	__insn_mtspr(SPR_PERF_COUNT_1, 0);
	__insn_mtspr(SPR_PERF_COUNT_STS, 0x3);
	__insn_mtspr(SPR_PERF_COUNT_0, __get_cpu_var(ctr)[0].reset_value);
	__insn_mtspr(SPR_PERF_COUNT_1, __get_cpu_var(ctr)[1].reset_value);
	__insn_mtspr(SPR_PERF_COUNT_CTL,
		     (__get_cpu_var(ctr)[1].perf_count_ctl << 16) |
		     __get_cpu_var(ctr)[0].perf_count_ctl);

#if CHIP_HAS_AUX_PERF_COUNTERS()
	/* enable the auxilliary perf counter interrupt */
	__insn_mtspr(SPR_AUX_PERF_COUNT_0, 0);
	__insn_mtspr(SPR_AUX_PERF_COUNT_1, 0);
	__insn_mtspr(SPR_AUX_PERF_COUNT_STS, 0x3);
	__insn_mtspr(SPR_AUX_PERF_COUNT_0, __get_cpu_var(ctr)[2].reset_value);
	__insn_mtspr(SPR_AUX_PERF_COUNT_1, __get_cpu_var(ctr)[3].reset_value);
	__insn_mtspr(SPR_AUX_PERF_COUNT_CTL,
		     (__get_cpu_var(ctr)[3].perf_count_ctl << 16) |
		     __get_cpu_var(ctr)[2].perf_count_ctl);
#endif

	__get_cpu_var(op_counters_enabled) = 1;

	/*
	 * Save away the time that we enabled profiling, and clear
	 * the count of interrupt cycles seen to date.
	 */
	__get_cpu_var(profile_time).profile_start_time = get_cycles();
	__get_cpu_var(profile_time).interrupt_cycles = 0;

	unmask_pmc_interrupts();

	put_cpu();

	atomic_dec(&op_start_cpus);
}

static int op_tile_start(void)
{
	unsigned int timeout;
	
	atomic_set(&op_start_cpus, num_online_cpus() - 1);

	(void)smp_call_function(op_tile_cpu_start, NULL, 1);
	op_tile_cpu_start(NULL);

	timeout = 1000;
	while ((atomic_read(&op_start_cpus) > 0) && timeout) {
		mdelay(1);
		timeout--;
	}

	if (!timeout)
		printk(KERN_ERR "Not all tiles are started after "
		       "oprofile starts");

	return 0;
}

static void op_tile_shutdown(void)
{
	release_pmc_hardware();
}

static void op_get_init_vals(struct op_counter_config master_ctr,
			     unsigned long *reset_val,
			     unsigned long *perf_count_ctl,
			     unsigned long *init_val,
			     int cpu)
{
	*perf_count_ctl = 0;
	*init_val = 0;
	*reset_val = 0;
	if (master_ctr.enabled) {
#ifndef __tilegx__
		unsigned long ctl = (master_ctr.event) & 0x7f;
		if (!master_ctr.user)
			ctl |= (1 << 7);
		/* Should we also disable HV counting in the following case? */
		if (!master_ctr.kernel)
			ctl |= (2 << 7);
#else
		unsigned long ctl = (master_ctr.event) & 0x1ff;
		if (!master_ctr.user)
			ctl |= (1 << 10);
		/* Should we also disable HV counting in the following case? */
		if (!master_ctr.kernel)
			ctl |= (2 << 10);
#endif
		/* We take the interrupt on wrap from 0Xffffffff -> 0 */
		*reset_val = 0 - master_ctr.count;
		/*
		 * Add a stagger the first time the event is triggered
		 * so that the cycle counter event doesn't fire in
		 * lock step and cause a bursty load on oprofiled.
		 */
		*init_val = 0 - (master_ctr.count +
				 (cpu * (master_ctr.count / NR_CPUS)));
		*reset_val &= 0xffffFFFFul;
		*init_val &= 0xffffFFFFul;
		*perf_count_ctl = ctl;
	}
}

static void op_tile_cpu_setup(void *dummy)
{
	int cpu = get_cpu();
	unsigned long init_value[TILE_NUM_PERF_COUNTERS];
	int i;

	for (i = 0; i < TILE_NUM_PERF_COUNTERS; i++) {
		unsigned long reset_val;
		unsigned long perf_count_ctl;

		op_get_init_vals(master_ctr[i], &reset_val, &perf_count_ctl,
				 &init_value[i], cpu);

		__get_cpu_var(ctr)[i].perf_count_ctl = perf_count_ctl;
		__get_cpu_var(ctr)[i].reset_value = reset_val;
	}
	/* just so we don't have any accidental interrupts */
	__insn_mtspr(SPR_PERF_COUNT_0, 0);
	__insn_mtspr(SPR_PERF_COUNT_1, 0);
	__insn_mtspr(SPR_PERF_COUNT_STS, 0x3);
	__insn_mtspr(SPR_PERF_COUNT_0, init_value[0]);
	__insn_mtspr(SPR_PERF_COUNT_1, init_value[1]);

#if CHIP_HAS_AUX_PERF_COUNTERS()
	__insn_mtspr(SPR_AUX_PERF_COUNT_0, 0);
	__insn_mtspr(SPR_AUX_PERF_COUNT_1, 0);
	__insn_mtspr(SPR_AUX_PERF_COUNT_STS, 0x3);
	__insn_mtspr(SPR_AUX_PERF_COUNT_0, init_value[2]);
	__insn_mtspr(SPR_AUX_PERF_COUNT_1, init_value[3]);

	// Some profiling events on the network switch (i.e.,
	// TDN_CONGESTION, MDN_CONGESTION, VDN_BUBBLE) can occur on
	// each of the 5 ouput ports of the switch. The direction is
	// set with the *_EVT_PORT_SEL field, which is a 5-bit field
	// in the SPR_PERF_COUNT_DN_CTL register. Each bit masks one
	// of the directions. Here we set all the masks as enabled.
	__insn_mtspr(SPR_PERF_COUNT_DN_CTL, 0xFFFFFFFF);
#endif

	__get_cpu_var(op_counters_enabled) = 0;

	put_cpu();

	atomic_dec(&op_setup_cpus);
}

static int op_tile_setup(void)
{
	unsigned int timeout;
	perf_irq_t old_irq_handler;

	old_irq_handler = reserve_pmc_hardware(op_handle_perf_interrupt);
	if (old_irq_handler) {
		if (old_irq_handler == userspace_perf_handler) {
			pr_warning("Can not start perf_event. PERF_COUNT is not"
			  " in Linux client protection level. Please set /proc/"
			  "sys/tile/userspace_perf_counters to 0.\n");
		} else {
			pr_warning("PMC hardware busy (reserved "
			  "by perf_event)\n");
		}
		return -EBUSY;
	}

	atomic_set(&op_setup_cpus, num_online_cpus() - 1);

	/* Configure the registers on all cpus.  */
	(void)smp_call_function(op_tile_cpu_setup, NULL, 1);
	op_tile_cpu_setup(NULL);

	timeout = 1000;
	while ((atomic_read(&op_setup_cpus) > 0) && timeout) {
		mdelay(1);
		timeout--;
	}

	if (!timeout)
		printk(KERN_ERR "Not all tiles are setup for oprofile");

	return 0;
}

static int op_tile_create_files(struct super_block *sb, struct dentry *root)
{
	int i;

	for (i = 0; i < TILE_NUM_PERF_COUNTERS; ++i) {
		struct dentry *dir;
		char buf[3];

		snprintf(buf, sizeof(buf), "%d", i);
		dir = oprofilefs_mkdir(sb, root, buf);

		oprofilefs_create_ulong(sb, dir, "enabled",
			&master_ctr[i].enabled);
		oprofilefs_create_ulong(sb, dir, "event",
			&master_ctr[i].event);
		oprofilefs_create_ulong(sb, dir, "count",
			&master_ctr[i].count);
		oprofilefs_create_ulong(sb, dir, "kernel",
			&master_ctr[i].kernel);
		oprofilefs_create_ulong(sb, dir, "user",
			&master_ctr[i].user);
		/* We don't use the unit mask */
		oprofilefs_create_ulong(sb, dir, "unit_mask",
			&master_ctr[i].unit_mask);
	}

	return 0;
}

int __init oprofile_arch_init(struct oprofile_operations *ops)
{
	ops->backtrace = tile_backtrace;

	if (hrtimer_oprofile_init(ops) == 0)
		return 0;

	ops->create_files = op_tile_create_files;
	ops->setup = op_tile_setup;
	ops->shutdown = op_tile_shutdown;
	ops->start = op_tile_start;
	ops->stop = op_tile_stop;
	ops->cpu_type = op_tile_cpu_type;
	return 0;
}

void oprofile_arch_exit(void)
{
}
