/*
 * Copyright 2011 Tilera Corporation. All Rights Reserved.
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

#ifndef OP_IMPL_H
#define OP_IMPL_H	1

int op_handle_perf_interrupt(struct pt_regs *regs, int fault_num);

/* Per-counter configuration as set via oprofilefs.  */
struct op_counter_config {
	unsigned long enabled;
	unsigned long event;
	unsigned long count;
	unsigned long kernel;
	unsigned long user;
	/* Dummies because I am too lazy to hack the userspace tools.  */
	unsigned long unit_mask;
	/* extra stuff */
	unsigned long reset_value;
	unsigned long perf_count_ctl;
};

/* Backtrace from regs->pc. */
void tile_backtrace(struct pt_regs *const regs, unsigned int depth);

#endif
