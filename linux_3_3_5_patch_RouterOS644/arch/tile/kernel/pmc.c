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

#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/atomic.h>
#include <linux/interrupt.h>

#include <asm/processor.h>
#include <asm/pmc.h>

/*
 * Default is "0", which means perf_counters is in Linux client protection level.
 * "1" means perf_counters is in user protection level.
 */
unsigned int userspace_perf_counters = 0;

int userspace_perf_handler(struct pt_regs *regs, int fault)
{
	panic("Unexpected PERF_COUNT interrupt %d\n", fault);
}
EXPORT_SYMBOL(userspace_perf_handler);

perf_irq_t perf_irq = NULL;
int handle_perf_interrupt(struct pt_regs *regs, int fault)
{
	int retval;

	if (!perf_irq)
		panic("Unexpected PERF_COUNT interrupt %d\n", fault);

	nmi_enter();
	retval = perf_irq(regs, fault);
	nmi_exit();
	unmask_pmc_interrupts();
	return retval;
}

/* Reserve PMC hardware if it is available. */
perf_irq_t reserve_pmc_hardware(perf_irq_t new_perf_irq)
{
	return cmpxchg(&perf_irq, NULL, new_perf_irq);
}
EXPORT_SYMBOL(reserve_pmc_hardware);

/* Release PMC hardware whatever. */
void release_pmc_hardware(void)
{
	perf_irq = NULL;
}
EXPORT_SYMBOL(release_pmc_hardware);


/*
 * Get current overflow status of each performance counter,
 * and auxiliary performance counter.
 */
unsigned long
pmc_get_overflow(void)
{
	unsigned long status;

	/*
	 * merge base+aux into a single vector
	 */
	status = __insn_mfspr(SPR_PERF_COUNT_STS);
#if CHIP_HAS_AUX_PERF_COUNTERS()
	status |= __insn_mfspr(SPR_AUX_PERF_COUNT_STS) << TILE_BASE_COUNTERS;
#endif
	return status;
}
EXPORT_SYMBOL(pmc_get_overflow);

/*
 * Clear the status bit for the corresponding counter, if written
 * with a one.
 */
void
pmc_ack_overflow(unsigned long status)
{
	/*
	 * clear overflow status by writing ones
	 */
	__insn_mtspr(SPR_PERF_COUNT_STS, status);
#if CHIP_HAS_AUX_PERF_COUNTERS()
	__insn_mtspr(SPR_AUX_PERF_COUNT_STS, status >> TILE_BASE_COUNTERS);
#endif
}
EXPORT_SYMBOL(pmc_ack_overflow);

/*
 * The perf count interrupts are masked and unmasked explicitly,
 * and only here.  The normal irq_enable() does not enable them,
 * and irq_disable() does not disable them.  That lets these
 * routines drive the perf count interrupts orthogonally.
 *
 * We also mask the perf count interrupts on entry to the perf count
 * interrupt handler in assembly code, and by default unmask them
 * again (with interrupt critical section protection) just before
 * returning from the interrupt.  If the perf count handler returns
 * a non-zero error code, then we don't re-enable them before returning.
 *
 * For Pro, we rely on both interrupts being in the same word to update
 * them atomically so we never have one enabled and one disabled.
 */

#if CHIP_HAS_SPLIT_INTR_MASK()
# if INT_PERF_COUNT < 32 || INT_AUX_PERF_COUNT < 32
#  error Fix assumptions about which word PERF_COUNT interrupts are in
# endif
#endif

static inline unsigned long long pmc_mask(void)
{
	unsigned long long mask = INT_MASK(INT_PERF_COUNT);
#if CHIP_HAS_AUX_PERF_COUNTERS()
	mask |= INT_MASK(INT_AUX_PERF_COUNT);
#endif
	return mask;
}

void unmask_pmc_interrupts(void)
{
	interrupt_mask_reset_mask(pmc_mask());
}
EXPORT_SYMBOL(unmask_pmc_interrupts);

void mask_pmc_interrupts(void)
{
	interrupt_mask_set_mask(pmc_mask());
}
EXPORT_SYMBOL(mask_pmc_interrupts);

bool pmc_used_by_kernel(void)
{
	return perf_irq && !userspace_perf_counters;
}

/*
 * Helper macro for SET_MPL; allows use of macros as the second argument
 *  to SET_MPL(). Copied from hv.
 */
#define _SET_MPL(name, pl)  __insn_mtspr(SPR_MPL_ ## name ## _SET_ ## pl , 1)
/* Set an MPL to a specific value. */
#define SET_MPL(name, pl)    _SET_MPL(name, pl)

/* Set MPL_PERF_COUNT_SET_X and clear PERF_COUNT_X. */
static void set_perf_count_sprs(void *unused)
{
	/* Clear counters. */
	__insn_mtspr(SPR_PERF_COUNT_CTL, TILE_PLM_MASK | TILE_PLM_MASK << 16);
	__insn_mtspr(SPR_AUX_PERF_COUNT_CTL,
		TILE_PLM_MASK | TILE_PLM_MASK << 16);
	__insn_mtspr(SPR_PERF_COUNT_0, 0);
	__insn_mtspr(SPR_PERF_COUNT_1, 0);
	__insn_mtspr(SPR_AUX_PERF_COUNT_0, 0);
	__insn_mtspr(SPR_AUX_PERF_COUNT_1, 0);

	pmc_ack_overflow(0xf);

	if (userspace_perf_counters == 1) {
		/* Set PERF_COUNT to user protection level. */
		SET_MPL(PERF_COUNT, 0);
		SET_MPL(AUX_PERF_COUNT, 0);
	} else {
		/* Set PERF_COUNT to Linux client protection level. */
#if CONFIG_KERNEL_PL == 1
		SET_MPL(PERF_COUNT, 1);
		SET_MPL(AUX_PERF_COUNT, 1);
#else
		SET_MPL(PERF_COUNT, 2);
		SET_MPL(AUX_PERF_COUNT, 2);
#endif
	}
}

/*
 * proc handler for /proc/sys/tile/userspace_perf_counters
 */
int userspace_perf_counters_handler(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp,
		loff_t *ppos)
{
	int ret, changed;
	unsigned int old_userspace_perf_counters;

	/* Read /proc/sys/tile/userspace_perf_counters */
	if (!write) {
		ret = proc_dointvec_minmax(table, write, buffer, lenp, ppos);
		return ret;
	}

	/* Write /proc/sys/tile/userspace_perf_counters */
	old_userspace_perf_counters = userspace_perf_counters;
	ret = proc_dointvec_minmax(table, write, buffer, lenp, ppos);
	changed = userspace_perf_counters != old_userspace_perf_counters;

	/*
	 * Do something only if the value of userspace_perf_counters
	 * is changed.
	 */
	if  (ret == 0 && changed) {

		if (userspace_perf_counters == 1) {
			if (reserve_pmc_hardware(userspace_perf_handler)) {
				pr_warning("PMC hardware busy (reserved "
					"by perf_event or oprofile)\n");
				userspace_perf_counters =
					old_userspace_perf_counters;
				return -EBUSY;
			}
		} else
			release_pmc_hardware();

		/* Set MPL_PERF_COUNT_SET_X on each tile. */
		on_each_cpu(set_perf_count_sprs, NULL, 1);
	}

	return ret;
}
