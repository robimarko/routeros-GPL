/*
 * Copyright 2010 Tilera Corporation. All Rights Reserved.
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
 * This file contains various random system calls that
 * have a non-standard calling sequence on the Linux/TILE
 * platform.
 */

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/syscalls.h>
#include <linux/mman.h>
#include <linux/file.h>
#include <linux/mempolicy.h>
#include <linux/binfmts.h>
#include <linux/fs.h>
#include <linux/compat.h>
#include <linux/uaccess.h>
#include <linux/signal.h>
#include <asm/syscalls.h>
#include <asm/pgtable.h>
#include <asm/homecache.h>
#include <asm/cachectl.h>
#include <arch/chip.h>

SYSCALL_DEFINE3(cacheflush, unsigned long, addr, unsigned long, len,
		unsigned long, flags)
{
	if (flags & DCACHE)
	homecache_evict(cpumask_of(smp_processor_id()));
	if (flags & ICACHE)
		flush_remote(0, HV_FLUSH_EVICT_L1I, mm_cpumask(current->mm),
			     0, 0, 0, NULL, NULL, 0);
	return 0;
}

/*
 * Syscalls that pass 64-bit values on 32-bit systems normally
 * pass them as (low,high) word packed into the immediately adjacent
 * registers.  If the low word naturally falls on an even register,
 * our ABI makes it work correctly; if not, we adjust it here.
 * Handling it here means we don't have to fix uclibc AND glibc AND
 * any other standard libcs we want to support.
 */

#if !defined(__tilegx__) || defined(CONFIG_COMPAT)

ssize_t sys32_readahead(int fd, u32 offset_lo, u32 offset_hi, u32 count)
{
	return sys_readahead(fd, ((loff_t)offset_hi << 32) | offset_lo, count);
}

int sys32_fadvise64_64(int fd, u32 offset_lo, u32 offset_hi,
		       u32 len_lo, u32 len_hi, int advice)
{
	return sys_fadvise64_64(fd, ((loff_t)offset_hi << 32) | offset_lo,
				((loff_t)len_hi << 32) | len_lo, advice);
}

#endif /* 32-bit syscall wrappers */

/* Note: used by the compat code even in 64-bit Linux. */
SYSCALL_DEFINE6(mmap2, unsigned long, addr, unsigned long, len,
		unsigned long, prot, unsigned long, flags,
		unsigned long, fd, unsigned long, off_4k)
{
#define PAGE_ADJUST (PAGE_SHIFT - 12)
	if (off_4k & ((1 << PAGE_ADJUST) - 1))
		return -EINVAL;
	return sys_mmap_pgoff(addr, len, prot, flags, fd,
			      off_4k >> PAGE_ADJUST);
}

#ifdef __tilegx__
SYSCALL_DEFINE6(mmap, unsigned long, addr, unsigned long, len,
		unsigned long, prot, unsigned long, flags,
		unsigned long, fd, off_t, offset)
{
	if (offset & ((1 << PAGE_SHIFT) - 1))
		return -EINVAL;
	return sys_mmap_pgoff(addr, len, prot, flags, fd,
			      offset >> PAGE_SHIFT);
}
#endif

#ifdef CONFIG_HOMECACHE

#if CHIP_HAS_CBOX_HOME_MAP()

/* Set up the default, which is allbutstack. */
static int __write_once utext_hash = 1;
static int __write_once udata_hash = 1;
static int __write_once ustack_hash = 0;
int __write_once uheap_hash = 1;

/* Allow overriding the defaults. */
int __init setup_ucache_hash(char *str)
{
	if (str == NULL)
		return -EINVAL;

	if (strcmp(str, "all") == 0) {
		utext_hash = 1;
		udata_hash = 1;
		uheap_hash = 1;
		ustack_hash = 1;
	} else if (strcmp(str, "allbutstack") == 0) {
		utext_hash = 1;
		udata_hash = 1;
		uheap_hash = 1;
		ustack_hash = 0;
	} else if (strcmp(str, "static") == 0) {
		utext_hash = 1;
		udata_hash = 1;
		uheap_hash = 0;
		ustack_hash = 0;
	} else if (strcmp(str, "ro") == 0) {
		utext_hash = 1;
		udata_hash = 0;
		uheap_hash = 0;
		ustack_hash = 0;
	} else if (strcmp(str, "none") == 0) {
		utext_hash = 0;
		udata_hash = 0;
		uheap_hash = 0;
		ustack_hash = 0;
	} else {
		return -EINVAL;
	}

	return 0;
}
early_param("ucache_hash", setup_ucache_hash);

#define ucache_flags(which)						\
	((current_thread_info()->status & TS_EXEC_HASH_ ## which) ?	\
	 MAP_CACHE_HOME_HASH : MAP_CACHE_HOME_SINGLE)

#endif /* CHIP_HAS_CBOX_HOME_MAP() */

int arch_vm_area_flags(struct mm_struct *mm, unsigned long flags,
		       unsigned long vm_flags,
		       pid_t *pid_ptr, pgprot_t *prot_ptr)
{
	pgprot_t prot = __pgprot(0);
	pid_t pid = 0;

#if CHIP_HAS_NC_AND_NOALLOC_BITS()
	if (flags & MAP_CACHE_NO_L1)
		prot = hv_pte_set_no_alloc_l1(prot);
	if (flags & MAP_CACHE_NO_L2)
		prot = hv_pte_set_no_alloc_l2(prot);
#endif


#if CHIP_HAS_CBOX_HOME_MAP()
	/* Certain types of mapping have standard hash-for-home defaults. */
	if (!(flags & _MAP_CACHE_HOME)) {
		if (flags & MAP_ANONYMOUS)
			flags |= (flags & MAP_GROWSDOWN) ?
				ucache_flags(STACK) : ucache_flags(HEAP);
		else if (flags & MAP_PRIVATE)
			flags |= (vm_flags & PROT_WRITE) ?
				ucache_flags(DATA) : ucache_flags(TEXT);
	}
#endif

	/*
	 * If the only request is for what the kernel does naturally,
	 * remove it, to avoid unnecessary use of VM_DONTMERGE.
	 */
	if (flags & MAP_ANONYMOUS) {
		switch (flags & _MAP_CACHE_MKHOME(_MAP_CACHE_HOME_MASK)) {
#if CHIP_HAS_CBOX_HOME_MAP()
		case MAP_CACHE_HOME_HASH:
			if (hash_default)
				flags &= ~MAP_CACHE_HOME_HASH;
			break;
#endif
		case MAP_CACHE_HOME_SINGLE:
			if (!hash_default)
				flags &= ~MAP_CACHE_HOME_SINGLE;
			break;
		}
	}

	if (flags & _MAP_CACHE_HOME)
		prot = pte_set_forcecache(prot);

	if ((flags & _MAP_CACHE_MKHOME(_MAP_CACHE_HOME_MASK)) ==
	    MAP_CACHE_HOME_NONE) {

		/*
		 * We special-case setting the home cache to "none".
		 * If the user isn't indicating willingness to tolerate
		 * incoherence, and is caching locally on the cpu, we
		 * fail a writable mapping, or enforce a readonly mapping.
		 */
		if (!(flags & _MAP_CACHE_INCOHERENT) &&
		    (flags & MAP_CACHE_NO_LOCAL) != MAP_CACHE_NO_LOCAL) {
			if (vm_flags & VM_WRITE)
				return -EINVAL;
		}
		if ((flags & MAP_CACHE_NO_LOCAL) == MAP_CACHE_NO_LOCAL)
			prot = hv_pte_set_mode(prot, HV_PTE_MODE_UNCACHED);
		else
			prot = hv_pte_set_mode(prot, HV_PTE_MODE_CACHE_NO_L3);

	} else if (flags & _MAP_CACHE_HOME) {

		/* Extract the cpu (or magic cookie). */
		int cpu = (flags >> _MAP_CACHE_HOME_SHIFT) &
			_MAP_CACHE_HOME_MASK;

		switch (cpu) {

		case _MAP_CACHE_HOME_SINGLE:
			/*
			 * This is the default case; we set "anyhome"
			 * and the OS will pick the cpu for us in pfn_pte()
			 * by examining the page_home() of the page.
			 */
			prot = hv_pte_set_mode(prot, HV_PTE_MODE_CACHE_TILE_L3);
			prot = pte_set_anyhome(prot);
			break;

#if CHIP_HAS_CBOX_HOME_MAP()
		case _MAP_CACHE_HOME_HASH:
			/* Mark this page for home-map hash caching. */
			prot = hv_pte_set_mode(prot, HV_PTE_MODE_CACHE_HASH_L3);
			break;
#endif

		case _MAP_CACHE_HOME_TASK:
			pid = current->pid;
			/*FALLTHROUGH*/

		case _MAP_CACHE_HOME_HERE:
			cpu = smp_processor_id();
			/*FALLTHROUGH*/

		default:
			if (cpu < 0 || cpu >= nr_cpu_ids ||
			    !cpu_is_valid_lotar(cpu))
				return -EINVAL;
			prot = hv_pte_set_mode(prot, HV_PTE_MODE_CACHE_TILE_L3);
			prot = set_remote_cache_cpu(prot, cpu);
		}
	}

	/*
	 * If we get a request for priority, we have to start checking
	 * this mm from now on when we switch to it.  We could do things
	 * that are more efficient: for example, hack mmap and munmap
	 * to reset a more definitive flag saying whether there is or
	 * is not a priority mapping by rescanning; or even by tracking
	 * the vm_area_structs themselves with a counter.  But this
	 * technique seems most foolproof, and doesn't involve messing
	 * with architecture-independent code at all.
	 *
	 * Note that if we implement VA<->PA coloring, we could then
	 * also usefully implement tracking exactly which priority
	 * pages are present, and use the hv_set_caching() argument to
	 * only partially flip the cache out of red/black mode.
	 * But this is an optimization for another day.
	 */
	if (flags & MAP_CACHE_PRIORITY) {
		start_mm_caching(mm);
		prot = hv_pte_set_cached_priority(prot);
	}

	prot_ptr->val |= prot.val;
	*pid_ptr = pid;
	return 0;
}

/*
 * If we are setting up a shared-writable mapping that forces homing
 * of part of a file, ensure that there are no other shared-writable
 * mappings that force the homing of an overlapping part of the
 * same file in an incompatible way.  This implies that the caching
 * mode matches, and if it is a "tile L3" mode, that the specified
 * remote cpus match (or one of them is an "anyhome" mapping).  Note
 * that we don't care about the NO_L1, etc., parts of the pgprot_t,
 * since those may differ without causing re-homecaching.
 */
int arch_vm_area_validate(struct vm_area_struct *new_vma,
			  struct address_space *mapping)
{
	size_t len = new_vma->vm_end - new_vma->vm_start;
	pgprot_t prot = new_vma->vm_page_prot;
	unsigned long end = new_vma->vm_pgoff + (len >> PAGE_SHIFT);
	struct vm_area_struct *vma;
	struct prio_tree_iter iter;

	/* No existing writable mappings means we must be OK. */
	if (mapping->i_mmap_writable == 0)
		return 0;

	/* If we're not trying to set up a shared mapping, we're OK. */
	if (!(new_vma->vm_flags & VM_SHARED))
		return 0;

	/* If we're not forcing the caching, we're OK. */
	if (!pte_get_forcecache(prot))
		return 0;

	vma_prio_tree_foreach(vma, &iter, &mapping->i_mmap,
			      new_vma->vm_pgoff, end) {

		/* If we are updating our own mapping, ignore it. */
		if (vma == new_vma)
			continue;

		/*
		 * The prio_tree is apparently conservative and will
		 * report a mapping immediately following our proposed
		 * new mapping as overlapping with ours, erroneously.
		 * Watch for this and discard it.
		 */
		if (vma->vm_pgoff == end)
			continue;

		/* Ignore other private mappings or non-forcing mappings. */
		if (!(vma->vm_flags & VM_SHARED) ||
		    !pte_get_forcecache(vma->vm_page_prot))
			continue;

		if (hv_pte_get_mode(vma->vm_page_prot) !=
		    hv_pte_get_mode(prot))
			return -EINVAL;

		if (hv_pte_get_mode(prot) == HV_PTE_MODE_CACHE_TILE_L3 &&
		    !pte_get_anyhome(vma->vm_page_prot) &&
		    !pte_get_anyhome(prot) &&
		    hv_pte_get_lotar(vma->vm_page_prot) !=
		    hv_pte_get_lotar(prot))
			return -EINVAL;

	}

	return 0;
}

#if CHIP_HAS_CBOX_HOME_MAP()

#define CACHE_VAR MAP_CACHE_HASH_ENV_VAR "="

static void _arch_exec_env(const char *val)
{
	unsigned flags;

	if (val == NULL)
		goto no_env;

	/* Set flags based on the environment variable string value. */
	if (strcmp(val, "all") == 0)
		flags = TS_EXEC_HASH_STACK | TS_EXEC_HASH_HEAP |
			TS_EXEC_HASH_DATA | TS_EXEC_HASH_TEXT;
	else if (strcmp(val, "allbutstack") == 0)
		flags = TS_EXEC_HASH_HEAP |
			TS_EXEC_HASH_DATA | TS_EXEC_HASH_TEXT;
	else if (strcmp(val, "static") == 0)
		flags = TS_EXEC_HASH_DATA | TS_EXEC_HASH_TEXT;
	else if (strcmp(val, "ro") == 0)
		flags = TS_EXEC_HASH_TEXT;
	else if (strcmp(val, "none") == 0)
		flags = 0;
	else {
		/* Could issue a warning, but we don't.  Set to defaults. */
	no_env:
		flags = (ustack_hash ? TS_EXEC_HASH_STACK : 0) |
			(uheap_hash ? TS_EXEC_HASH_HEAP : 0) |
			(udata_hash ? TS_EXEC_HASH_DATA : 0) |
			(utext_hash ? TS_EXEC_HASH_TEXT : 0);
	}

	/* Remember for later. */
	current_thread_info()->status &= ~TS_EXEC_HASH_FLAGS;
	current_thread_info()->status |= flags;
}

void arch_exec_env(const char __user *const __user *envp)
{
	char buf[64];
	const char *val = NULL;
	int i;

	for (i = 0; ; ++i) {
		const char __user *str;
		int len;

		/* Copy in the next env string, with validity checking. */
		if (get_user(str, &envp[i]) || str == NULL)
			break;     /* Corrupt envp array, or end of array. */
		len = strnlen_user(str, sizeof(buf));
		if (len > sizeof(buf))
			continue;  /* Too long; ignore. */
		if (len == 0 || _copy_from_user(buf, str, len))
			break;     /* Corrupt envp[] pointer; give up. */

		/* See if it is the one we're interested in. */
		if (strncmp(buf, CACHE_VAR, sizeof(CACHE_VAR)-1) == 0) {
			val = &buf[sizeof(CACHE_VAR)-1];
			break;
		}
	}

	_arch_exec_env(val);
}

#ifdef CONFIG_COMPAT
void compat_arch_exec_env(void __user *_envp)
{
	char buf[64];
	const char *val = NULL;
	int i;
	compat_uptr_t __user *envp = (compat_uptr_t __user *) _envp;

	for (i = 0; ; ++i) {
		compat_uptr_t str;
		int len;

		/* Copy in the next env string, with validity checking. */
		if (get_user(str, &envp[i]) || str == 0)
			break;     /* Corrupt envp array, or end of array. */
		len = strnlen_user(compat_ptr(str), sizeof(buf));
		if (len > sizeof(buf))
			continue;  /* Too long; ignore. */
		if (len == 0 || _copy_from_user(buf, compat_ptr(str), len))
			break;     /* Corrupt envp[] pointer; give up. */

		/* See if it is the one we're interested in. */
		if (strncmp(buf, CACHE_VAR, sizeof(CACHE_VAR)-1) == 0) {
			val = &buf[sizeof(CACHE_VAR)-1];
			break;
		}
	}

	_arch_exec_env(val);
}
#endif

void arch_exec_vma(struct vm_area_struct *vma)
{
	unsigned long flags = current_thread_info()->status;
	unsigned long vm_flags;
	int use_hash;
	pgprot_t prot;

	vm_flags = vma->vm_flags;
	if (vm_flags & VM_GROWSDOWN)
		use_hash = !!(flags & TS_EXEC_HASH_STACK);
	else if (vm_flags & VM_WRITE)
		use_hash = !!(flags & TS_EXEC_HASH_DATA);
	else
		use_hash = !!(flags & TS_EXEC_HASH_TEXT);

	prot = vma->vm_page_prot;
	if (use_hash) {
		/* Use hash-for-home caching for this mapping. */
		prot = hv_pte_set_mode(prot, HV_PTE_MODE_CACHE_HASH_L3);
	} else {
		/*
		 * Cache this mapping all on one cpu.  For immutable
		 * pages (r/o, or r/w pages before they are COW'ed) this
		 * will always be homed on the local tile.
		 */
		prot = hv_pte_set_mode(prot, HV_PTE_MODE_CACHE_TILE_L3);
		prot = pte_set_anyhome(prot);
	}

	prot = pte_set_forcecache(prot);
	vma->vm_page_prot = prot;
	vma->vm_flags |= VM_DONTMERGE;
}

void arch_exec_map(unsigned long addr)
{
	struct vm_area_struct *vma = find_vma(current->mm, addr);
	BUG_ON(!vma || addr < vma->vm_start);
	arch_exec_vma(vma);
}

#endif /* CHIP_HAS_CBOX_HOME_MAP() */

#endif /* CONFIG_HOMECACHE */

/* Provide the actual syscall number to call mapping. */
#undef __SYSCALL
#define __SYSCALL(nr, call) [nr] = (call),

#ifndef __tilegx__
/* See comments at the top of the file. */
#define sys_fadvise64_64 sys32_fadvise64_64
#define sys_readahead sys32_readahead
#endif

/* Call the trampolines to manage pt_regs where necessary. */
#define sys_execve _sys_execve
#define sys_sigaltstack _sys_sigaltstack
#define sys_rt_sigreturn _sys_rt_sigreturn
#define sys_clone _sys_clone
#ifndef __tilegx__
#define sys_cmpxchg_badaddr _sys_cmpxchg_badaddr
#endif

/*
 * Note that we can't include <linux/unistd.h> here since the header
 * guard will defeat us; <asm/unistd.h> checks for __SYSCALL as well.
 */
void *sys_call_table[__NR_syscalls] = {
	[0 ... __NR_syscalls-1] = sys_ni_syscall,
#include <asm/unistd.h>
};
