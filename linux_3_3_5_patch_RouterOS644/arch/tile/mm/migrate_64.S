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
 * This routine is a helper for migrating the home of a set of pages to
 * a new cpu.  See the documentation in homecache.c for more information.
 */

#include <linux/linkage.h>
#include <linux/threads.h>
#include <asm/page.h>
#include <asm/thread_info.h>
#include <asm/types.h>
#include <asm/asm-offsets.h>
#include <hv/hypervisor.h>

	.text

/*
 * First, some definitions that apply to all the code in the file.
 */

/* Locals (caller-save) */
#define r_tmp		r10
#define r_save_sp	r11

/* What we save where in the stack frame; must include all callee-saves. */
#define FRAME_SP	8
#define FRAME_R30	16
#define FRAME_R31	24
#define FRAME_R32	32
#define FRAME_SIZE	40


#ifdef CONFIG_HOMECACHE
/*
 * On entry:
 *
 *   r0 new stack PTE to use (moved to r_stack_pte)
 *   r1 virtual address
 *   r2 length of virtual address range
 *   r3 pointer to stack PTE to use (moved to r_stack_ptep)
 *   r4 cache cpumask pointer
 *   r5 tlb cpumask pointer
 *   r6 HV_Remote_ASID array pointer
 *   r7 HV_Remote_ASID count
 *
 * If THREAD_SIZE > PAGE_SIZE, we assume that the stack is in
 * contiguous pages (which should be true if allocated via alloc_pages)
 * and that the PTEs that map it don't span page table boundaries.
 */

/* Arguments (caller-save) */
#define r_stack_pte_in	r0
#define r_va		r1
#define r_length	r2
#define r_stack_ptep_in	r3
#define r_cache_cpumask_in r4
#define r_tlb_cpumask   r5
#define r_rem_asids     r6
#define r_num_rem_asids r7

/* Locals (callee-save); must not be more than FRAME_xxx above. */
#define r_cache_cpumask	r30
#define r_stack_ptep	r31
#define r_stack_pte	r32

STD_ENTRY(homecache_migrate_stack_and_flush)

	/* Create a stack frame; we can't touch it once we set the
	 * migrating bit on the stack PTE until we clear it at the end.
	 */
	{
	 move r_save_sp, sp
	 st sp, lr
	 addi sp, sp, -FRAME_SIZE
	}
	addi r_tmp, sp, FRAME_SP
	{
	 st r_tmp, r_save_sp
	 addi r_tmp, sp, FRAME_R30
	}
	{
	 st r_tmp, r30
	 addi r_tmp, sp, FRAME_R31
	}
	{
	 st r_tmp, r31
	 addi r_tmp, sp, FRAME_R32
	}
	st r_tmp, r32

	/* Move some arguments to callee-save registers. */
	{
	 move r_cache_cpumask, r_cache_cpumask_in
	 move r_stack_ptep, r_stack_ptep_in
	}
	{
	 move r_stack_pte, r_stack_pte_in
	}

	/* Make sure our stack writes have reached the remote cache. */
	mf

	/* Clear the present bit and set the migrating bit on the stack. */
	ld r_tmp, r_stack_ptep
	andi r_tmp, r_tmp, ~HV_PTE_PRESENT
	ori r_tmp, r_tmp, HV_PTE_MIGRATING
	st r_stack_ptep, r_tmp
#if THREAD_SIZE == PAGE_SIZE
	/* No further work required. */
#elif THREAD_SIZE == (2 * PAGE_SIZE)
	addi r_stack_ptep, r_stack_ptep, 8
	ld r_tmp, r_stack_ptep
	andi r_tmp, r_tmp, ~HV_PTE_PRESENT
	ori r_tmp, r_tmp, HV_PTE_MIGRATING
	{
	 st r_stack_ptep, r_tmp
	 addi r_stack_ptep, r_stack_ptep, -8
	}
#else
# error Only support 1- or 2-page stacks
#endif
	mf

	/*
	 * Now we do a global TLB flush:
	 *
	 *   hv_flush_remote(0ULL, 0, NULL,
	 *                   va, length, PAGE_SIZE, tlb_cpumask,
	 *                   rem_asids, num_rem_asids);
	 *
	 */
	{
	 move r8, r_num_rem_asids
	 move r7, r_rem_asids
	}
	{
	 move r6, r_tlb_cpumask
	 moveli r5, hw1_last(PAGE_SIZE)
	}
	{
	 shl16insli r5, r5, hw0(PAGE_SIZE)
	 move r4, r_length
	}
	{
	 move r3, r_va
	 move r2, zero
	}
	{
	 move r1, zero
	 move r0, zero
	}
	jal hv_flush_remote
	bnez r0, .Lwrite_stack_pte

	/*
	 * And now a cache flush on the old cpus:
	 *
	 *   hv_flush_remote(0ULL, HV_FLUSH_EVICT_L2, cache_cpumask,
	 *                   NULL, 0, 0, 0, NULL, 0)
	 *
	 */
	{
	 move r0, zero
	 moveli r1, hw2_last(HV_FLUSH_EVICT_L2)
	}
	{
	 shl16insli r1, r1, hw1(HV_FLUSH_EVICT_L2)
	 move r2, r_cache_cpumask
	}
	{
	 shl16insli r1, r1, hw0(HV_FLUSH_EVICT_L2)
	 move r3, zero
	}
	{
	 move r4, zero
	 move r5, zero
	}
	{
	 move r6, zero
	 move r7, zero
	}
	{
	 move r8, zero
	 jal hv_flush_remote
	}

.Lwrite_stack_pte:
	/* Finally, write the new stack PTE. */
	st r_stack_ptep, r_stack_pte
#if THREAD_SIZE == PAGE_SIZE
	/* No further work required. */
#elif THREAD_SIZE == (2 * PAGE_SIZE)
	{
	 addi r_stack_ptep, r_stack_ptep, 8
	 movei r_tmp, 1
	}
	shli r_tmp, r_tmp, HV_PTE_INDEX_PFN
	add r_stack_pte, r_stack_pte, r_tmp
	st r_stack_ptep, r_stack_pte
#else
# error Only support 1- or 2-page stacks
#endif
	mf

.Ldone:
	/* Restore the callee-saved registers and return. */
	addli lr, sp, FRAME_SIZE
	{
	 ld lr, lr
	 addli r_tmp, sp, FRAME_R30
	}
	{
	 ld r30, r_tmp
	 addli r_tmp, sp, FRAME_R31
	}
	{
	 ld r31, r_tmp
	 addli r_tmp, sp, FRAME_R32
	}
	{
	 ld r32, r_tmp
	 addi sp, sp, FRAME_SIZE
	}
	jrp lr
	STD_ENDPROC(homecache_migrate_stack_and_flush)
#endif


/*
 * On entry:
 *
 *   r0 the new context PA to install (moved to r_context)
 *   r1 PTE to use for context access (moved to r_access)
 *   r2 ASID to use for new context (moved to r_asid)
 *   r3 pointer to cpumask with just this cpu set in it (r_my_cpumask)
 */

/* Arguments (caller-save) */
#define r_context_in	r0
#define r_access_in	r1
#define r_asid_in	r2
#define r_my_cpumask	r3

/* Locals (callee-save); must not be more than FRAME_xxx above. */
#define r_context	r30
#define r_access	r31
#define r_asid		r32

/*
 * Caller-save locals and frame constants are the same as
 * for homecache_migrate_stack_and_flush.
 */

STD_ENTRY(flush_and_install_context)
	/*
	 * Create a stack frame; we can't touch it once we flush the
	 * cache until we install the new page table and flush the TLB.
	 */
	{
	 move r_save_sp, sp
	 st sp, lr
	 addi sp, sp, -FRAME_SIZE
	}
	addi r_tmp, sp, FRAME_SP
	{
	 st r_tmp, r_save_sp
	 addi r_tmp, sp, FRAME_R30
	}
	{
	 st r_tmp, r30
	 addi r_tmp, sp, FRAME_R31
	}
	{
	 st r_tmp, r31
	 addi r_tmp, sp, FRAME_R32
	}
	 st r_tmp, r32

	/* Move some arguments to callee-save registers. */
	{
	 move r_context, r_context_in
	 move r_access, r_access_in
	}
	move r_asid, r_asid_in

	/* First, flush our L2 cache. */
	{
	 move r0, zero  /* cache_pa */
	 moveli r1, hw2_last(HV_FLUSH_EVICT_L2)  /* cache_control */
	}
	{
	 shl16insli r1, r1, hw1(HV_FLUSH_EVICT_L2)
	 move r2, r_my_cpumask  /* cache_cpumask */
	}
	{
	 shl16insli r1, r1, hw0(HV_FLUSH_EVICT_L2)
	 move r3, zero  /* tlb_va */
	}
	{
	 move r4, zero  /* tlb_length */
	 move r5, zero  /* tlb_pgsize */
	}
	{
	 move r6, zero  /* tlb_cpumask */
	 move r7, zero  /* asids */
	}
	{
	 move r8, zero  /* asidcount */
	 jal hv_flush_remote
	}
	bnez r0, 1f

	/* Now install the new page table. */
	{
	 move r0, r_context
	 move r1, r_access
	}
	{
	 move r2, r_asid
	 moveli r3, HV_CTX_DIRECTIO | CTX_PAGE_FLAG
	}
	jal hv_install_context
	bnez r0, 1f

	/* Finally, flush the TLB. */
	{
	 movei r0, 0   /* preserve_global */
	 jal hv_flush_all
	}

1:	/* Restore the callee-saved registers and return. */
	addli lr, sp, FRAME_SIZE
	{
	 ld lr, lr
	 addli r_tmp, sp, FRAME_R30
	}
	{
	 ld r30, r_tmp
	 addli r_tmp, sp, FRAME_R31
	}
	{
	 ld r31, r_tmp
	 addli r_tmp, sp, FRAME_R32
	}
	{
	 ld r32, r_tmp
	 addi sp, sp, FRAME_SIZE
	}
	jrp lr
	STD_ENDPROC(flush_and_install_context)
