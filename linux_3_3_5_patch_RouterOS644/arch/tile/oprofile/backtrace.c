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
 *
 * Tile specific backtracing code for oprofile
 */

#include <linux/kernel.h>
#include <linux/oprofile.h>
#include <linux/uaccess.h>
#include <asm/stack.h>
#include "op_impl.h"

unsigned long handle_syscall_link_address(void);

void tile_backtrace(struct pt_regs *const regs, unsigned int depth)
{
	struct KBacktraceIterator kbt;
	unsigned int i;

	/*
	 * Get the address just after the "jalr" instruction that
	 * jumps to the handler for a syscall.  When we find this
	 * address in a backtrace, we silently ignore it, which gives
	 * us a one-step backtrace connection from the sys_xxx()
	 * function in the kernel to the xxx() function in libc.
	 * Otherwise, we lose the ability to properly attribute time
	 * from the libc calls to the kernel implementations, since
	 * oprofile only considers PCs from backtraces a pair at a time.
	 */
	unsigned long handle_syscall_pc = handle_syscall_link_address();

	KBacktraceIterator_init(&kbt, NULL, regs);
	kbt.profile = 1;

	/*
	 * The sample for the pc is already recorded.  Now we are adding the
	 * address of the callsites on the stack.  Our iterator starts
	 * with the frame of the (already sampled) call site.  If our
	 * iterator contained a "return address" field, we could have just
	 * used it and wouldn't have needed to skip the first
	 * frame.  That's in effect what the arm and x86 versions do.
	 * Instead we peel off the first iteration to get the equivalent
	 * behavior.
	 */

	if (KBacktraceIterator_end(&kbt))
		return;
	KBacktraceIterator_next(&kbt);

	for (i = 0; i < depth; ++i) {
		int is_kernel;
		unsigned long pc;
		if (KBacktraceIterator_end(&kbt))
			break;
		pc = kbt.it.pc;
		is_kernel = (pc >= PAGE_OFFSET &&
			     !is_arch_mappable_range(pc, 1));
		if (pc != handle_syscall_pc)
			oprofile_add_pc(pc, is_kernel, 0);
		KBacktraceIterator_next(&kbt);
	}
}
