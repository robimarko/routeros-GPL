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
 * This code maintains the "home" for each page in the system.
 */

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/bootmem.h>
#include <linux/rmap.h>
#include <linux/pagemap.h>
#include <linux/interrupt.h>
#include <linux/sysctl.h>
#include <linux/pagevec.h>
#include <linux/ptrace.h>
#include <linux/timex.h>
#include <linux/cache.h>
#include <linux/smp.h>
#include <linux/module.h>
#include <linux/hugetlb.h>

#include <asm/page.h>
#include <asm/sections.h>
#include <asm/tlbflush.h>
#include <asm/pgalloc.h>
#include <asm/homecache.h>
#ifdef CONFIG_DATAPLANE
#include <asm/dataplane.h>
#endif

#include <arch/sim.h>

#include "migrate.h"

#ifdef CONFIG_HOMECACHE

/* Extra hook function in mm/page_alloc.c, just for homecache. */
extern struct page *homecache_rmqueue(struct zone *, unsigned int order,
				      int migratetype);

/* Forward declarations. */
static int homecache_get_desired_home(int home, pgprot_t, int writable);

/* Various statistics exposed through /proc/sys/tile/homecache/. */

/* Pages sequestered at "free" time (e.g. immutable). */
static int homecache_proc_sequestered_free;

/* Times we have dumped sequestered coherent pages back to the allocator. */
static int homecache_proc_coherent_sequestered_purge;

/* Times we have done a chip-wide flush and unsequestered everything. */
static int homecache_proc_sequestered_purge;

/* Number of unmapped pages migrated (i.e. no PTEs yet). */
static int homecache_proc_migrated_unmapped;

/* Number of mapped pages we have migrated. */
static int homecache_proc_migrated_mapped;

/* Number of incoherent pages finv'ed. */
static int homecache_proc_incoherent_finv_pages;

/* Number of tasks that have been migrated. */
static int homecache_proc_migrated_tasks;

#endif /* CONFIG_HOMECACHE */

#if CHIP_HAS_COHERENT_LOCAL_CACHE()

/*
 * The noallocl2 option suppresses all use of the L2 cache to cache
 * locally from a remote home.  There's no point in using it if we
 * don't have coherent local caching, though.
 */
static int __write_once noallocl2;
static int __init set_noallocl2(char *str)
{
	noallocl2 = 1;
	return 0;
}
early_param("noallocl2", set_noallocl2);

#else

#define noallocl2 0

#endif
#ifdef CONFIG_HOMECACHE

/* Return the cache home of a page. */
int page_home(struct page *page)
{
	return page->home;
}
EXPORT_SYMBOL(page_home);

/*
 * Optimize access to page home in this file.  We use the function
 * definition to make it possible to use modules that reference
 * page_home() regardless of how the kernel is built.
 */
#define page_home(page)			((page)->home)

#endif /* CONFIG_HOMECACHE */
#ifdef CONFIG_DATAPLANE

static atomic_t remote_flush_counts[NR_CPUS];

/*
 * Update the number of remote TLB flushes targeting this cpu.
 * Barrier to ensure that any remote cpus will be able to see the
 * new value in dataplane_return_to_user().
 */
static void set_remote_flush_count(struct cpumask *mask, int delta)
{
	if (mask) {
		int cpu;
		for_each_cpu(cpu, mask)
			atomic_add(delta, &remote_flush_counts[cpu]);
		mb();
	}
}

/* Return the number of remote TLB flushes targeting this cpu. */
int homecache_get_remote_flush_count(void)
{
	return atomic_read(&remote_flush_counts[smp_processor_id()]);
}

/* State of TLB deferral on a dataplane cpu. */
static DEFINE_PER_CPU(atomic_t, dataplane_tlb_state);

/* Provide constants for dataplane TLB deferral. */
#define TLB_DEFER_KERNEL   0    /* in kernel space */
#define TLB_DEFER_USER     1    /* in user space */
#define TLB_DEFER_PENDING  2    /* in user space with a TLB flush pending */

/*
 * This routine is called on kernel entry from userspace for dataplane
 * tiles, so we can properly adjust our state to be TLB_DEFER_KERNEL,
 * and run a TLB flush if necessary.
 *
 * Note that using the atomic routines here implies that the atomic
 * locks are all available to us.  This is true only because if the
 * hypervisor recursively punches through ICS (when an atomic lock
 * might be held, e.g. for a cmpxchg() to vmalloc'ed memory), we will
 * see it very early in the interrupt handler path and call
 * do_page_fault_ics(), which will release the lock before continuing
 * on to the code that will invoke this routine.
 */
void homecache_tlb_defer_enter(void)
{
	atomic_t *state = &__get_cpu_var(dataplane_tlb_state);
	if (atomic_xchg(state, TLB_DEFER_KERNEL) == TLB_DEFER_PENDING) {
		unsigned long size = KERNEL_HIGH_VADDR - PAGE_OFFSET;
		int rc = hv_flush_pages(PAGE_OFFSET, PAGE_SIZE, size);
		rc |= hv_flush_pages(PAGE_OFFSET, HPAGE_SIZE, size);
		BUG_ON(rc != 0);
	}
}

/*
 * This routine is called on kernel exit to userspace for dataplane
 * tiles, so we can properly adjust our state to be TLB_DEFER_USER.
 */
void homecache_tlb_defer_exit(void)
{
	atomic_t *state = &__get_cpu_var(dataplane_tlb_state);

	/*
	 * Note that we could write directly to state->counter here
	 * instead of using the normal serializing atomic_set(),
	 * since we shouldn't be able to race with the other deferral
	 * routines any more than we already do.  But it makes it easier
	 * to reason about this code to use the serializing version,
	 * and I'm not concerned about performance in this context.
	 */
	atomic_set(state, TLB_DEFER_USER);
}

/*
 * This routine determines if we can defer a TLB flush.
 * It must be called with interrupts disabled.
 */
static int homecache_tlb_can_defer(int cpu)
{
	atomic_t *state = &per_cpu(dataplane_tlb_state, cpu);
	int old = atomic_cmpxchg(state, TLB_DEFER_USER, TLB_DEFER_PENDING);
	return (old != TLB_DEFER_KERNEL);
}

/*
 * Return whether the remote cpu is actively running dataplane code
 * and is not in the kernel.
 */
static int dataplane_hvflush_debug(int cpu)
{
	return dataplane_debug(cpu) &&
		(atomic_read(&per_cpu(dataplane_tlb_state, cpu)) !=
		 TLB_DEFER_KERNEL);
}

/*
 * Remove cpus from the passed mask that can defer TLB flushes.
 * Adjust the remote flush counts as well.
 */
static void clear_user_cpus(struct cpumask *tlb_cpumask)
{
	struct cpumask maybe_defer;
	int cpu;

	cpumask_and(&maybe_defer, tlb_cpumask, &dataplane_map);
	if (cpumask_empty(&maybe_defer))
		return;

	for_each_cpu(cpu, &maybe_defer) {
		if (homecache_tlb_can_defer(cpu)) {
			cpumask_clear_cpu(cpu, tlb_cpumask);
			atomic_add(-1, &remote_flush_counts[cpu]);
		}
	}
}
#endif /* CONFIG_DATAPLANE */

/*
 * Update the irq_stat for cpus that we are going to interrupt
 * with TLB or cache flushes.  Also handle removing dataplane cpus
 * from the TLB flush set, and setting dataplane_tlb_state instead.
 */
static void hv_flush_update(const struct cpumask *cache_cpumask,
			    struct cpumask *tlb_cpumask,
			    unsigned long tlb_va, unsigned long tlb_length,
			    HV_Remote_ASID *asids, int asidcount)
{
	struct cpumask mask;
	int i, cpu;
#ifdef CONFIG_DATAPLANE
	int dataplane_stack = 0;
#endif

	cpumask_clear(&mask);
	if (cache_cpumask) {
		cpumask_or(&mask, &mask, cache_cpumask);
#ifdef CONFIG_DATAPLANE
		for_each_cpu(cpu, cache_cpumask) {
			if (dataplane_hvflush_debug(cpu)) {
				dataplane_stack = 1;
				pr_err("cpu %d: cache flush to"
				       " dataplane cpu %d\n",
				       smp_processor_id(), cpu);
			}
		}
#endif
	}

	if (tlb_cpumask && tlb_length) {
#ifdef CONFIG_DATAPLANE
		/*
		 * If we are doing TLB flushes to kernel addresses we
		 * check to see if there are any dataplane tiles that
		 * are currently running user code and if so, mark
		 * them as pending a TLB flush, and remove them from
		 * tlb_cpumask.  We only proceed with the flush on
		 * such tiles if they are in the kernel.
		 */
		if (tlb_va >= PAGE_OFFSET &&
		    tlb_va + tlb_length < KERNEL_HIGH_VADDR)
			clear_user_cpus(tlb_cpumask);
#endif
		cpumask_or(&mask, &mask, tlb_cpumask);
	}

	for (i = 0; i < asidcount; ++i) {
		int cpu = xy_to_cpu(asids[i].x, asids[i].y);
#ifdef CONFIG_DATAPLANE
		if (dataplane_hvflush_debug(cpu)) {
			dataplane_stack = 1;
			pr_err("cpu %d: ASID flush to dataplane cpu %d\n",
			       smp_processor_id(), cpu);
		}
#endif		
		cpumask_set_cpu(cpu, &mask);
	}

	/*
	 * Don't bother to update atomically; losing a count
	 * here is not that critical.
	 */
	for_each_cpu(cpu, &mask)
		++per_cpu(irq_stat, cpu).irq_hv_flush_count;

#ifdef CONFIG_DATAPLANE
	if (dataplane_stack)
		dump_stack();
#endif
}

/*
 * This wrapper function around hv_flush_remote() does several things:
 *
 *  - Provides a return value error-checking panic path, since
 *    there's never any good reason for hv_flush_remote() to fail.
 *  - Accepts a 32-bit PFN rather than a 64-bit PA, which generally
 *    is the type that Linux wants to pass around anyway.
 *  - Canonicalizes that lengths of zero make cpumasks NULL.
 *  - Handles deferring TLB flushes for dataplane tiles.
 *  - Tracks remote interrupts in the per-cpu irq_cpustat_t.
 *
 * Note that we have to wait until the cache flush completes before
 * updating the per-cpu last_cache_flush word, since otherwise another
 * concurrent flush can race, conclude the flush has already
 * completed, and start to use the page while it's still dirty
 * remotely (running concurrently with the actual evict, presumably).
 */
void flush_remote(unsigned long cache_pfn, unsigned long cache_control,
		  const struct cpumask *cache_cpumask_orig,
		  HV_VirtAddr tlb_va, unsigned long tlb_length,
		  unsigned long tlb_pgsize,
		  const struct cpumask *tlb_cpumask_orig,
		  HV_Remote_ASID *asids, int asidcount)
{
	int rc;
	struct cpumask cache_cpumask_copy, tlb_cpumask_copy;
	struct cpumask *cache_cpumask, *tlb_cpumask;
	HV_PhysAddr cache_pa;
	char cache_buf[NR_CPUS*5], tlb_buf[NR_CPUS*5];

	mb();   /* provided just to simplify "magic hypervisor" mode */

	/*
	 * Canonicalize and copy the cpumasks.
	 */
	if (cache_cpumask_orig && cache_control) {
		cpumask_copy(&cache_cpumask_copy, cache_cpumask_orig);
		cache_cpumask = &cache_cpumask_copy;
	} else {
		cpumask_clear(&cache_cpumask_copy);
		cache_cpumask = NULL;
	}
	if (cache_cpumask == NULL)
		cache_control = 0;
	if (tlb_cpumask_orig && tlb_length) {
		cpumask_copy(&tlb_cpumask_copy, tlb_cpumask_orig);
		tlb_cpumask = &tlb_cpumask_copy;
	} else {
		cpumask_clear(&tlb_cpumask_copy);
		tlb_cpumask = NULL;
	}
#ifdef CONFIG_DATAPLANE
	set_remote_flush_count(tlb_cpumask, +1);
#endif
	hv_flush_update(cache_cpumask, tlb_cpumask,
			tlb_va, tlb_length, asids, asidcount);
	cache_pa = (HV_PhysAddr)cache_pfn << PAGE_SHIFT;
	rc = hv_flush_remote(cache_pa, cache_control,
			     cpumask_bits(cache_cpumask),
			     tlb_va, tlb_length, tlb_pgsize,
			     cpumask_bits(tlb_cpumask),
			     asids, asidcount);
#ifdef CONFIG_DATAPLANE
	set_remote_flush_count(tlb_cpumask, -1);
#endif
	if (rc == 0)
		return;
	cpumask_scnprintf(cache_buf, sizeof(cache_buf), &cache_cpumask_copy);
	cpumask_scnprintf(tlb_buf, sizeof(tlb_buf), &tlb_cpumask_copy);

	pr_err("hv_flush_remote(%#llx, %#lx, %p [%s],"
	       " %#lx, %#lx, %#lx, %p [%s], %p, %d) = %d\n",
	       cache_pa, cache_control, cache_cpumask, cache_buf,
	       (unsigned long)tlb_va, tlb_length, tlb_pgsize,
	       tlb_cpumask, tlb_buf,
	       asids, asidcount, rc);
	panic("Unsafe to continue.");
}

static void homecache_finv_page_va(void* va, int home)
{
	if (home == smp_processor_id()) {
		finv_buffer_local(va, PAGE_SIZE);
	} else if (home == PAGE_HOME_HASH) {
		finv_buffer_remote(va, PAGE_SIZE, 1);
	} else {
			BUG_ON(home < 0 || home >= NR_CPUS);
		finv_buffer_remote(va, PAGE_SIZE, 0);
	}
}

void homecache_finv_map_page(struct page *page, int home)
{
	unsigned long flags;
	unsigned long va;
	pte_t *ptep;
	pte_t pte;

	if (home == PAGE_HOME_UNCACHED)
		return;
	local_irq_save(flags);
#ifdef CONFIG_HIGHMEM
	va = __fix_to_virt(FIX_KMAP_BEGIN + kmap_atomic_idx_push() +
			   (KM_TYPE_NR * smp_processor_id()));
#else
	va = __fix_to_virt(FIX_HOMECACHE_BEGIN + smp_processor_id());
#endif
	ptep = virt_to_pte(NULL, (unsigned long)va);
	pte = pfn_pte(page_to_pfn(page), PAGE_KERNEL);
	__set_pte(ptep, pte_set_home(pte, home));
	homecache_finv_page_va((void *)va, home);
	__pte_clear(ptep);
	hv_flush_page(va, PAGE_SIZE);
#ifdef CONFIG_HIGHMEM
	kmap_atomic_idx_pop();
#endif
	local_irq_restore(flags);
}

static void homecache_finv_page_home(struct page *page, int home)
{
	if (!PageHighMem(page) && home == page_home(page))
		homecache_finv_page_va(page_address(page), home);
	else
		homecache_finv_map_page(page, home);
}

static inline bool incoherent_home(int home)
{
	return home == PAGE_HOME_IMMUTABLE || home == PAGE_HOME_INCOHERENT;
}

static void homecache_finv_page_internal(struct page *page, int force_map)
{
	int home = page_home(page);
	if (home == PAGE_HOME_UNCACHED)
		return;
	if (incoherent_home(home)) {
		int cpu;
		for_each_cpu(cpu, &cpu_cacheable_map)
			homecache_finv_map_page(page, cpu);
#ifdef CONFIG_HOMECACHE
		++homecache_proc_incoherent_finv_pages;
#endif
	} else if (force_map) {
		/* Force if, e.g., the normal mapping is migrating. */
		homecache_finv_map_page(page, home);
	} else {
		homecache_finv_page_home(page, home);
	}
	sim_validate_lines_evicted(PFN_PHYS(page_to_pfn(page)), PAGE_SIZE);
}

void homecache_finv_page(struct page *page)
{
	homecache_finv_page_internal(page, 0);
}

void homecache_evict(const struct cpumask *mask)
{
	flush_remote(0, HV_FLUSH_EVICT_L2, mask, 0, 0, 0, NULL, NULL, 0);
}

/* Report the home corresponding to a given PTE. */
static int pte_to_home(pte_t pte)
{
	if (hv_pte_get_nc(pte))
		return PAGE_HOME_IMMUTABLE;
	switch (hv_pte_get_mode(pte)) {
	case HV_PTE_MODE_CACHE_TILE_L3:
		return get_remote_cache_cpu(pte);
	case HV_PTE_MODE_CACHE_NO_L3:
		return PAGE_HOME_INCOHERENT;
	case HV_PTE_MODE_UNCACHED:
		return PAGE_HOME_UNCACHED;
#if CHIP_HAS_CBOX_HOME_MAP()
	case HV_PTE_MODE_CACHE_HASH_L3:
		return PAGE_HOME_HASH;
#endif
	}
	panic("Bad PTE %#llx\n", pte.val);
}

/* Update the home of a PTE if necessary (can also be used for a pgprot_t). */
pte_t pte_set_home(pte_t pte, int home)
{
	/* Check for non-linear file mapping "PTEs" and pass them through. */
	if (pte_file(pte))
		return pte;

#if CHIP_HAS_MMIO()
	/* Check for MMIO mappings and pass them through. */
	if (hv_pte_get_mode(pte) == HV_PTE_MODE_MMIO)
		return pte;
#endif


	/*
	 * Only immutable pages get NC mappings.  If we have a
	 * non-coherent PTE, but the underlying page is not
	 * immutable, it's likely the result of a forced
	 * caching setting running up against ptrace setting
	 * the page to be writable underneath.  In this case,
	 * just keep the PTE coherent.
	 */
	if (hv_pte_get_nc(pte) && home != PAGE_HOME_IMMUTABLE) {
		pte = hv_pte_clear_nc(pte);
		pr_err("non-immutable page incoherently referenced: %#llx\n",
		       pte.val);
	}

	switch (home) {

	case PAGE_HOME_UNCACHED:
		pte = hv_pte_set_mode(pte, HV_PTE_MODE_UNCACHED);
		break;

	case PAGE_HOME_INCOHERENT:
		pte = hv_pte_set_mode(pte, HV_PTE_MODE_CACHE_NO_L3);
		break;

	case PAGE_HOME_IMMUTABLE:
		/*
		 * We could home this page anywhere, since it's immutable,
		 * but by default just home it to follow "hash_default".
		 */
		BUG_ON(hv_pte_get_writable(pte));
		if (pte_get_forcecache(pte)) {
			/* Upgrade "force any cpu" to "No L3" for immutable. */
			if (hv_pte_get_mode(pte) == HV_PTE_MODE_CACHE_TILE_L3
			    && pte_get_anyhome(pte)) {
				pte = hv_pte_set_mode(pte,
						      HV_PTE_MODE_CACHE_NO_L3);
			}
		} else
#if CHIP_HAS_CBOX_HOME_MAP()
		if (hash_default)
			pte = hv_pte_set_mode(pte, HV_PTE_MODE_CACHE_HASH_L3);
		else
#endif
			pte = hv_pte_set_mode(pte, HV_PTE_MODE_CACHE_NO_L3);
		pte = hv_pte_set_nc(pte);
		break;

#if CHIP_HAS_CBOX_HOME_MAP()
	case PAGE_HOME_HASH:
		pte = hv_pte_set_mode(pte, HV_PTE_MODE_CACHE_HASH_L3);
		break;
#endif

	default:
		BUG_ON(home < 0 || home >= NR_CPUS ||
		       !cpu_is_valid_lotar(home));
		pte = hv_pte_set_mode(pte, HV_PTE_MODE_CACHE_TILE_L3);
		pte = set_remote_cache_cpu(pte, home);
		break;
	}

#if CHIP_HAS_NC_AND_NOALLOC_BITS()
	if (noallocl2)
		pte = hv_pte_set_no_alloc_l2(pte);

	/* Simplify "no local and no l3" to "uncached" */
	if (hv_pte_get_no_alloc_l2(pte) && hv_pte_get_no_alloc_l1(pte) &&
	    hv_pte_get_mode(pte) == HV_PTE_MODE_CACHE_NO_L3) {
		pte = hv_pte_set_mode(pte, HV_PTE_MODE_UNCACHED);
	}
#endif

	/* Checking this case here gives a better panic than from the hv. */
	BUG_ON(hv_pte_get_mode(pte) == 0);

	return pte;
}
EXPORT_SYMBOL(pte_set_home);
#ifndef CONFIG_HOMECACHE

/*
 * The routines in this section are the "static" versions of the normal
 * dynamic homecaching routines; they just set the home cache
 * of a kernel page once, and require a full-chip cache/TLB flush,
 * so they're not suitable for anything but infrequent use.
 */

#if CHIP_HAS_CBOX_HOME_MAP()
static inline int initial_page_home(void) { return PAGE_HOME_HASH; }
#else
static inline int initial_page_home(void) { return 0; }
#endif

int page_home(struct page *page)
{
	if (PageHighMem(page)) {
		return initial_page_home();
	} else {
		unsigned long kva = (unsigned long)page_address(page);
		return pte_to_home(*virt_to_pte(NULL, kva));
	}
}
EXPORT_SYMBOL(page_home);

void homecache_change_page_home(struct page *page, int order, int home)
{
	int i, pages = (1 << order);
	unsigned long kva;

	BUG_ON(PageHighMem(page));
	BUG_ON(page_count(page) > 1);
	BUG_ON(page_mapcount(page) != 0);
	kva = (unsigned long) page_address(page);
	flush_remote(0, HV_FLUSH_EVICT_L2, &cpu_cacheable_map,
		     kva, pages * PAGE_SIZE, PAGE_SIZE, cpu_online_mask,
		     NULL, 0);

	for (i = 0; i < pages; ++i, kva += PAGE_SIZE) {
		pte_t *ptep = virt_to_pte(NULL, kva);
		pte_t pteval = *ptep;
		BUG_ON(!pte_present(pteval) || pte_huge(pteval));
		__set_pte(ptep, pte_set_home(pteval, home));
	}
}

struct page *homecache_alloc_pages(gfp_t gfp_mask,
				   unsigned int order, int home)
{
	struct page *page;
	BUG_ON(gfp_mask & __GFP_HIGHMEM);   /* must be lowmem */
	page = alloc_pages(gfp_mask, order);
	if (page)
		homecache_change_page_home(page, order, home);
	return page;
}
EXPORT_SYMBOL(homecache_alloc_pages);

struct page *homecache_alloc_pages_node(int nid, gfp_t gfp_mask,
					unsigned int order, int home)
{
	struct page *page;
	BUG_ON(gfp_mask & __GFP_HIGHMEM);   /* must be lowmem */
	page = alloc_pages_node(nid, gfp_mask, order);
	if (page)
		homecache_change_page_home(page, order, home);
	return page;
}

void __homecache_free_pages(struct page *page, unsigned int order)
{
	if (put_page_testzero(page)) {
		homecache_change_page_home(page, order, initial_page_home());
		if (order == 0) {
			free_hot_cold_page(page, 0);
		} else {
			init_page_count(page);
			__free_pages(page, order);
		}
	}
}
EXPORT_SYMBOL(__homecache_free_pages);

void homecache_free_pages(unsigned long addr, unsigned int order)
{
	if (addr != 0) {
		VM_BUG_ON(!virt_addr_valid((void *)addr));
		__homecache_free_pages(virt_to_page((void *)addr), order);
	}
}
EXPORT_SYMBOL(homecache_free_pages);

#else  /* dynamic homecaching support hooked into the Linux internals */

/*
 * When we free a page, in addition to marking the pages as free,
 * we check the homing to determine what to do with it.
 *
 * Pages that are compatible with the buddy allocator get freed
 * normally.  With hash_default, this means hash-for-home pages only,
 * since we expect most pages to come back that way.  Otherwise, we
 * do all pages that are "cheap" to re-home (i.e. pages cached on
 * a single cpu, plus uncached pages).
 *
 * Pages that are potentially cached on every cpu are put onto
 * a special global "sequestered" list, since we don't anticipate
 * being able to easily reuse them for anything without doing
 * a global round of cache flushing first.
 *
 * Other pages (the kind that didn't go in the buddy allocator)
 * are stored on a special per-zone free list and checked whenever
 * we do an allocation from that zone that requests a home override,
 * so we can re-use them without having to do a remote flush
 * on a page that we pulled from the buddy allocator.
 */

/* Do we want to free this page back to the buddy allocator? */
static int home_is_freeable(int home)
{
	/* For hash-default heap, we only free back hash pages. */
#if CHIP_HAS_CBOX_HOME_MAP()
	if (hash_default)
		return home == PAGE_HOME_HASH;
#endif

	/* Otherwise, we only free back things that are easy to re-home. */
	return (home == PAGE_HOME_UNCACHED || home >= 0);
}

/*
 * When resetting a page's homecache (e.g. when freshly allocating the
 * page, flushing out the homecache free list, or converting an
 * immutable page) what home should we reset it to?
 */
static int default_page_home(void)
{
#if CHIP_HAS_CBOX_HOME_MAP()
	if (hash_default)
		return PAGE_HOME_HASH;
#endif
	/* Arbitrarily home the page back on this cpu. */
	return smp_processor_id();
}

/* When true, the homecache checker passes all pages to the buddy allocator. */
static DEFINE_PER_CPU(int, homecache_is_unsequestering);

/* Check if this page should be returned to homecache, not page_alloc. */
int homecache_check_free_page(struct page *page, int order)
{
	int pages, i;
	int home = page_home(page);

	/*
	 * Clear the homecache_nomigrate bit; it would only have been
	 * set if we had vmapped the page and now have unmapped it and
	 * are freeing it.
	 */
	pages = (1 << order);
	for (i = 0; i < pages; ++i)
		__ClearPageHomecacheNomigrate(&page[i]);

	/* Validate that the whole allocation was identically homed. */
	pages = (1 << order);
	for (i = 1; i < pages; ++i) {
		BUG_ON(page_home(&page[i]) != home);
	}

	/*
	 * In kdata=huge mode, any lowmem page mapped by a small
	 * kernel PTE goes onto the per-zone freelists, unless the
	 * "homecache_is_unsequestering" flag is set, in which case we
	 * have to put them all back into the buddy allocator anyway.
	 */
	if (__get_cpu_var(homecache_is_unsequestering) ||
	    ((!kdata_huge ||
	      PageHighMem(page) ||
	      pte_huge(*virt_to_pte(NULL, (ulong)page_address(page)))) &&
	     home_is_freeable(home)))
		return 0;

	/* Otherwise, the homecache code will keep it. */
	return 1;
}

/*
 * Translate a cache home to the list within the zone where we keep
 * similarly-homed free pages.
 */
static struct list_head *homecache_zone_list(struct zone *zone, int home)
{
	if (home >= 0)
		return &zone->homecache_list[home];
	else if (home == PAGE_HOME_UNCACHED)
		return &zone->homecache_list[ZONE_HOMECACHE_UNCACHED_INDEX];
#if CHIP_HAS_CBOX_HOME_MAP()
	else if (home == PAGE_HOME_HASH)
		return &zone->homecache_list[ZONE_HOMECACHE_HFH_INDEX];
#endif
	else if (incoherent_home(home))
		return &zone->homecache_list[ZONE_HOMECACHE_INCOHERENT_INDEX];
	else
		return NULL;
}

/*
 * Make sure we're not putting a page on the free list that a later
 * allocation is going to complain about.
 */
static void homecache_add_to_free_list(struct page *page,
				       struct list_head *list)
{
	if (likely((page->flags & PAGE_FLAGS_CHECK_AT_PREP) == 0)) {
		list_add(&page->lru, list);
		return;
	}

	/* Leak the page. */
	printk(KERN_ALERT "BUG: Bad page state in process %s  pfn:%05lx\n",
	       current->comm, page_to_pfn(page));
	dump_page(page);
	dump_stack();
}

/*
 * Actually free a page back to the homecache lists.
 * Should only be called if homecache_check_free_page() is true.
 */
void homecache_keep_free_page(struct page *page, int order)
{
	int home = page_home(page);
	struct zone *zone = page_zone(page);
	struct list_head *list = homecache_zone_list(zone, home);
	unsigned long flags;
	int pages = (1 << order);
	int i;

	BUG_ON(!list);
	spin_lock_irqsave(&zone->lock, flags);
	homecache_proc_sequestered_free += pages;
	for (i = 0; i < pages; ++i)
		homecache_add_to_free_list(&page[i], list);
	if (incoherent_home(home))
		zone->homecache_incoherent_count += pages;
	else
		zone->homecache_coherent_count += pages;
	spin_unlock_irqrestore(&zone->lock, flags);
}

/* Check that the page was allocated properly. */
static void check_page_home(struct page *page, int home)
{
	pte_t pte;
	if (home == PAGE_HOME_UNKNOWN)
		return;
#ifdef CONFIG_PREEMPT
#warning Consider just testing that the page is not hfh here?
#else
	if (home == PAGE_HOME_HERE)
		home = smp_processor_id();
#endif
	if (page_home(page) != home)
		panic("Allocated page PFN %#lx should have home %d, has %d\n",
		      page_to_pfn(page), home, page_home(page));
	if (PageHighMem(page))
		return;
	pte = *virt_to_pte(NULL, (unsigned long)page_address(page));
	BUG_ON(pte_to_home(pte) != home);
}

struct desired_home {
	int home;
	int depth;
	struct desired_home *next;
};

#define PUSH_DESIRED_HOME(home)					      \
	{							      \
		struct desired_home __dh = {			      \
			home,					      \
			irq_count(),				      \
			current->thread.homecache_desired_home	      \
		};						      \
		current->thread.homecache_desired_home = &__dh
		
#define POP_DESIRED_HOME()					      \
		current->thread.homecache_desired_home = __dh.next;   \
	}

/*
 * Return the desired home set by the current interrupt level.
 * If we were interrupted after setting the home, any allocations
 * in the interrupt context will have a different irq_count()
 * and so will return default_page_home().
 */
static inline int current_desired_home(void)
{
	struct desired_home *dh = current->thread.homecache_desired_home;
	if (dh && dh->depth == irq_count())
		return dh->home;
	else
		return default_page_home();
}

struct page *homecache_alloc_pages(gfp_t gfp_mask,
				   unsigned int order, int home)
{
	struct page *page;
	PUSH_DESIRED_HOME(home);
	page = alloc_pages(gfp_mask, order);
	POP_DESIRED_HOME();
	if (page)
		check_page_home(page, home);
	return page;
}
EXPORT_SYMBOL(homecache_alloc_pages);

struct page *homecache_alloc_pages_node(int nid, gfp_t gfp_mask,
					unsigned int order, int home)
{
	struct page *page;
	PUSH_DESIRED_HOME(home);
	page = alloc_pages_node(nid, gfp_mask, order);
	POP_DESIRED_HOME();
	if (page)
		check_page_home(page, home);
	return page;
}

struct page *homecache_alloc_page_vma(gfp_t gfp_mask,
				      struct vm_area_struct *vma,
				      unsigned long addr)
{
	pgprot_t prot = vma->vm_page_prot;
	if (!pte_get_forcecache(prot)) {
		return alloc_page_vma(gfp_mask, vma, addr);
	} else {
		struct page *page;
		int home = default_page_home();

		home = homecache_get_desired_home(home, prot, 1);
		PUSH_DESIRED_HOME(home);
		page = alloc_page_vma(gfp_mask, vma, addr);
		POP_DESIRED_HOME();
		if (page == NULL)
			return NULL;
		check_page_home(page, home);

#if CHIP_HAS_NC_AND_NOALLOC_BITS()
		/*
		 * If we are allocating a page with noalloc attributes,
		 * we should ensure it starts with a clean local cache.
		 * Normal coherence won't necessarily have flushed the
		 * local cache.
		 */
		if (hv_pte_get_no_alloc_l2(prot) ||
		    hv_pte_get_no_alloc_l1(prot))
			homecache_finv_page(page);
#endif

		return page;
	}
}

void __homecache_free_pages(struct page *page, unsigned int order)
{
	__free_pages(page, order);
}
EXPORT_SYMBOL(__homecache_free_pages);

void homecache_free_pages(unsigned long addr, unsigned int order)
{
	free_pages(addr, order);
}
EXPORT_SYMBOL(homecache_free_pages);

/* Called with interrupts disabled but the zone unlocked. */
struct page *homecache_get_cached_page(struct zone *zone, gfp_t gfp_flags)
{
	struct page *page;
	int home = current_desired_home();
	int require_homecache = kdata_huge && !is_highmem(zone);
	int order;
	struct list_head *list;

	/* Don't bother to look up incoherent pages. */
	if (incoherent_home(home))
		return NULL;

	/* Safe to convert here since we have interrupts disabled. */
	if (home == PAGE_HOME_HERE)
		home = smp_processor_id();

	/*
	 * The __GFP_HIGHMEM flag is a hint to this code that, if it
	 * is in a "require_homecache" zone (lowmem with kdata_huge)
	 * it should go to the homecache free list even if the request
	 * does not appear to merit homecaching.  This is because
	 * such pages are for user space or the page cache, both of
	 * which are prone to homecache adjustment.
	 */
	if (!((gfp_flags & __GFP_HIGHMEM) && require_homecache)) {

		/* Don't bother looking on the list if there's no override. */
		if (home == PAGE_HOME_UNKNOWN)
			return NULL;

		/* Don't bother for types that match the allocator. */
		if (home_is_freeable(home))
			return NULL;
	}

	/*
	 * See if there are any pages on the freelist for pages with the
	 * desired home.
	 */
	list = homecache_zone_list(zone, home);
	spin_lock(&zone->lock);
	if (list && !list_empty(list)) {
		page = list_first_entry(list, struct page, lru);
		check_page_home(page, home);
		list_del(&page->lru);
		--zone->homecache_coherent_count;
		goto unlock;
	}

	/*
	 * If we are using huge pages in the buddy allocator, and this
	 * is a LOWMEM zone, we would rather re-homecache an existing
	 * small page than shatter a new huge page.  So see if we
	 * have anything that is usable if we re-homecache it.
	 * We ignore the "migratetype", which is basically about
	 * defragmentation, and segregating the homecache pages is
	 * too, so it's plausible to ignore migratetype if necessary.
	 */
	if (require_homecache) {
		int i;

		/*
		 * We keep track of the last place we found a page, and
		 * start the search there.
		 */
		i = zone->lru_homecache_list;
		do {
			list = &zone->homecache_list[i];
			if (!list_empty(list)) {
				page = list_first_entry(list, struct page, lru);
				list_del(&page->lru);
				--zone->homecache_coherent_count;
				zone->lru_homecache_list = i;
				goto unlock;
			}
			i = i ? i - 1 : NR_COHERENT_ZONE_HOMECACHE_LISTS - 1;
		} while (i != zone->lru_homecache_list);

		/*
		 * We need to shatter a new huge page.  Ideally we get
		 * an entire huge page and shatter it.  But we work
		 * our way down to order-0 anyway, to be robust.
		 */
		for (order = HUGETLB_PAGE_ORDER; order >= 0; --order) {
			int migratetype = allocflags_to_migratetype(gfp_flags);
			page = homecache_rmqueue(zone, order, migratetype);
			if (page)
				break;
		}
		if (page) {
			int i, pages = 1 << order;
			unsigned long kaddr =
				(unsigned long)page_address(page);
			shatter_huge_page(kaddr);
			for (i = 1; i < pages; ++i) {
				list = homecache_zone_list(zone,
							   page_home(&page[i]));
				BUG_ON(!list);
				homecache_add_to_free_list(&page[i], list);
			}
			zone->homecache_coherent_count += pages - 1;
			goto unlock;
		}
	}

	/* No luck; just get something from the buddy allocator. */
	page = NULL;

 unlock:
	spin_unlock(&zone->lock);
	return page;
}

unsigned long homecache_count_sequestered_pages(bool total_mem,
						bool include_incoherent)
{
	unsigned long npages = 0;
	pg_data_t *pgdat;

	for_each_online_pgdat(pgdat) {
		int i;
		for (i = 0; i < MAX_NR_ZONES; i++) {
			struct zone *zone;
#ifdef CONFIG_HIGHMEM
			if (!total_mem &&
			    (i == ZONE_HIGHMEM ||
			     (zone_movable_is_highmem() && i == ZONE_MOVABLE)))
				continue;
#endif
			zone = &pgdat->node_zones[i];
			npages += zone->homecache_coherent_count;
			if (include_incoherent)
				npages += zone->homecache_incoherent_count;
		}
	}

	return npages;
}

/*
 * Collect all the pages that we mean to return to the allocator.
 * Any pages that were freed but homed on a non-cacheable cpu
 * (e.g. a hypervisor driver cpu) we ignore, but just count up
 * so we can reset the totals appropriately.
 */
static void homecache_collect_free_pages(struct zone *zone,
					 struct list_head *work_list,
					 bool flush_whole_chip)
{
	int i, npages = 0;
	spin_lock(&zone->lock);
	for (i = 0; i < NR_COHERENT_ZONE_HOMECACHE_LISTS; ++i) {
		if (i < NR_CPUS && !cpu_cacheable(i)) {
			struct list_head *list;
			list_for_each(list, &zone->homecache_list[i])
				++npages;
			continue;
		}
		list_splice_init(&zone->homecache_list[i], work_list);
	}
	zone->homecache_coherent_count = npages;
	if (flush_whole_chip) {
		struct list_head *list =
			&zone->homecache_list[ZONE_HOMECACHE_INCOHERENT_INDEX];
		list_splice_init(list, work_list);
		zone->homecache_incoherent_count = 0;
	}
	spin_unlock(&zone->lock);
}

/*
 * Walk a list of pages and reset their homes to the default, reset
 * their lowmem PTE (if any) to the new home, and do a full cache flush
 * and TLB flush of all cpus.  Return the number of pages on the list.
 */
static int homecache_reset_free_pages(struct list_head *work_list)
{
	unsigned long low_kaddr = ~0UL, high_kaddr = 0;
	struct page *page;
	int npages = 0;

	list_for_each_entry(page, work_list, lru) {
		unsigned long kva = (unsigned long)page_address(page);
		set_page_home(page, default_page_home());
		if (kva != 0) {
			pte_t *pte = virt_to_pte(NULL, kva);
			BUG_ON(pte == NULL || pte_huge(*pte));
			set_pte(pte, mk_pte(page, PAGE_KERNEL));
			if (kva < low_kaddr)
				low_kaddr = kva;
			if (kva + PAGE_SIZE > high_kaddr)
				high_kaddr = kva + PAGE_SIZE;
		}
		++npages;
	}

	if (low_kaddr > high_kaddr) {
		low_kaddr = 0;
		high_kaddr = 0;
	}

	/* Flush caches and probably TLBs everywhere */
	flush_remote(0, HV_FLUSH_EVICT_L2, &cpu_cacheable_map,
		     low_kaddr, high_kaddr - low_kaddr, PAGE_SIZE,
		     cpu_online_mask, NULL, 0);

	return npages;
}

int homecache_recover_free_pages(void)
{
	unsigned long flags;
	bool flush_whole_chip;
	LIST_HEAD(work_list);
	struct page *page, *tmp;
	struct zone *zone;

	/* Disable interrupts so we don't uselessly re-enter this routine. */
	local_irq_save(flags);

	/*
	 * If the coherent freelists have sufficient pages, just free
	 * them back to the allocator without resetting their homes
	 * and without doing a disruptive whole-chip cache flush.
	 * We can safely return pages from the coherent freelists with no
	 * need to do anything else for now, since we can flush those
	 * pages easily as we start re-allocating them.
	 */
	flush_whole_chip = (homecache_count_sequestered_pages(1, 0) < 256);

	/* Collect free pages from each zone. */
	for_each_zone(zone)
		homecache_collect_free_pages(zone, &work_list,
					     flush_whole_chip);

	/* If there are no pages, we're done. */
	if (list_empty(&work_list)) {
		local_irq_restore(flags);
		return 0;
	}

	/* If we're going to flush everything, go update homes and PTEs. */
	if (flush_whole_chip) {
		int pages_freed = homecache_reset_free_pages(&work_list);
		int do_printk = (homecache_proc_sequestered_purge == 0);
#ifdef CONFIG_DATAPLANE
		if (cpumask_empty(&dataplane_map))
			do_printk = 0;
#endif
		if (do_printk) {
			pr_info("cpu %d (%s/%d): note: homecache freed"
				" %d sequestered pages\n",
				smp_processor_id(), current->comm,
				current->pid, pages_freed);
		}
		++homecache_proc_sequestered_purge;
	} else {
		++homecache_proc_coherent_sequestered_purge;
	}

	/* Free all the pages back to the buddy allocator. */
	__get_cpu_var(homecache_is_unsequestering) = 1;
	list_for_each_entry_safe(page, tmp, &work_list, lru)
		free_hot_cold_page(page, 1);
	__get_cpu_var(homecache_is_unsequestering) = 0;

	/* TODO: this would be an obvious point to unshatter any huge pages. */

	local_irq_restore(flags);
	return 1;
}


/*
 * Provide a lock and two accessors to lock when we are doing
 * page migration and when we are trying to add new kernel mappings.
 */

static DEFINE_SPINLOCK(kpte_lock);

unsigned long homecache_kpte_lock_irqsave(void)
	__acquires(kpte_lock)
{
	unsigned long flags;
	spin_lock_irqsave(&kpte_lock, flags);
	return flags;
}

void homecache_kpte_unlock_irqrestore(unsigned long flags)
	__releases(kpte_lock)
{
	spin_unlock_irqrestore(&kpte_lock, flags);
}

void homecache_kpte_lock(void)
	__acquires(kpte_lock)
{
	spin_lock(&kpte_lock);
}

void homecache_kpte_unlock(void)
	__releases(kpte_lock)
{
	spin_unlock(&kpte_lock);
}


/*
 * Find the kernel PTE mapping this page (either lowmem or kmap) and
 * adjust it as follows.  If "finished" is false, we mark it as
 * migrating; otherwise, we rebuild it "from scratch".  In
 * the normal migration model (mark PTEs migrating; flush TLBs; flush
 * caches; rewrite PTEs) the finished=0 and finished=1 modes
 * correspond to the first and last phases, respectively.
 *
 * FIXME: ptrace writes on huge pages will create temporary mappings
 * of sub-pages within the huge page, and we will not see it since we
 * are only checking for the vaddr of the beginning of the huge page.
 * We could loop calling kmap_fix_kpte() or pass "pages" to kmap_fix_kpte,
 * but either way is still likely pretty inefficient, and we might end
 * up with a set of unrelated kernel VAs that we need to flush.
 */
static unsigned long homecache_fix_kpte(struct page *page, int pages,
					int finished)
{
	int i, pfn;
	int home = page->home;
	unsigned long vaddr;
	pte_t *ptep;
	pgprot_t prot;

#ifdef CONFIG_HIGHMEM
	if (PageHighMem(page)) {
		kmap_atomic_fix_kpte(page, finished);
		vaddr = (unsigned long) kmap_fix_kpte(page, finished);
		return vaddr;
	}
#endif
	pfn = page_to_pfn(page);
	vaddr = (unsigned long) lowmem_page_address(page);
	shatter_huge_page(vaddr);
	ptep = virt_to_pte(NULL, vaddr);
	BUG_ON(ptep == NULL || pte_huge(*ptep));
	prot = (home == PAGE_HOME_IMMUTABLE) ? PAGE_KERNEL_RO : PAGE_KERNEL;
	prot = pte_set_home(prot, home);
	for (i = 0; i < pages; ++i, ++pfn, ++ptep) {
		if (!finished) {
			pte_t pte = *ptep;
			set_pte(ptep, pte_mkmigrate(pte));
			BUG_ON(!pte_same(pte, pfn_pte(pfn, prot)));
		} else {
			set_pte(ptep, pfn_pte(pfn, prot));
		}
	}
	return vaddr;
}

/* Evict a contiguous set of pages from cache. */
static void homecache_finv_pages(struct page *page, int pages, int force_map)
{
	int i;
	for (i = 0; i < pages; ++i, ++page)
		homecache_finv_page_internal(page, force_map);
}

/* Mark a group of pages with their new homecache. */
static void set_pages_home(struct page *page, int pages, int home)
{
	int i;
	for (i = 0; i < pages; ++i)
		set_page_home(&page[i], home);
}

/*
 * Remember that we allocated this page on this processor,
 * so that we can set up our PTEs to always reference that home.
 * Arguably we might want to be able to batch page allocations
 * here so we can avoid multiple IPI round-trips.  TBD.
 * However, note that we have per-cpu freelists, so that it is
 * at least plausible that we will get mostly same-cpu homed
 * pages once we get into a steady state.
 *
 * Locking requirements are substantially eased in this code
 * because we are guaranteeing that the page(s) are not mapped
 * into user-space anywhere.
 *
 * The "home" argument is the requested new setting for the
 * specified block of pages.
 */
void homecache_change_page_home(struct page *page, int order, int new_home)
{
	int pages = 1 << order;
	unsigned long vaddr;
	int i;

	/*
	 * Validate the assumption that the page is unmapped
	 * and is available for migration.
	 */
	for (i = 0; i < pages; ++i) {
		BUG_ON(page_mapcount(&page[i]) != 0);
		BUG_ON(PageHomecacheNomigrate(&page[i]));
	}

	/* Do a quick check if migration is needed at all. */
	for (i = 0; i < pages; ++i) {
		if (page_home(&page[i]) != new_home)
			break;
	}
	if (i == pages)
		return;

	/* Flush page out of cache(s) and reset its TLBs, if any. */
	homecache_proc_migrated_unmapped += pages;
	homecache_finv_pages(page, pages, 0);
	set_pages_home(page, pages, new_home);
	vaddr = homecache_fix_kpte(page, pages, 1);
	if (vaddr)
		flush_remote(0, 0, NULL, vaddr, pages * PAGE_SIZE, PAGE_SIZE,
			     cpu_online_mask, NULL, 0);

	sim_validate_lines_evicted(PFN_PHYS(page_to_pfn(page)),
				   pages * PAGE_SIZE);
}

void homecache_new_kernel_page(struct page *page, int order)
{
	int home = current_desired_home();
	if (home == PAGE_HOME_UNKNOWN)
		home = default_page_home();
	if (home == PAGE_HOME_HERE)
		home = smp_processor_id();
	homecache_change_page_home(page, order, home);
}

void homecache_update_migrating_pte(struct page *page, pte_t *ptep,
				    struct vm_area_struct *vma,
				    unsigned long address)
{
	pte_t oldpte = *ptep;
	unsigned long pfn = pte_pfn(oldpte);
	pte_t pte = pfn_pte(pfn, vma->vm_page_prot);
	pte.val = (pte.val & ~_PAGE_ALL) | (oldpte.val & _PAGE_ALL);
	pte = hv_pte_clear_nc(pte_donemigrate(pte));
	pte = pte_set_home(pte, page_home(page));
	set_pte_at(vma->vm_mm, address, ptep, pte);
}

/*
 * Change the homing of a mapped page, flushing any stale PTEs.
 * The page must be locked on entry.
 */
static void homecache_home_mapped_page(struct page *page, int order,
				       int new_home)
{
	struct thread_struct *ts = &current->thread;
	int pages = 1 << order;
	void *vaddr;
	int ttu_flags = TTU_IGNORE_MLOCK | TTU_IGNORE_ACCESS;
	unsigned long flags;

	/* Check some requirements. */
	BUG_ON(!PageLocked(page));

	/* Check if we need to do anything. */
	if (page_home(page) == new_home)
		return;

	/*
	 * If the page is not migratable (e.g. a vmalloc page being
	 * provided by a driver to user space) just ignore this request.
	 * We leave it cached however it was cached before.
	 */
	if (PageHomecacheNomigrate(page))
		return;

	ts->homecache_is_migrating = true;

	homecache_proc_migrated_mapped += pages;

	/*
	 * Now, find all the places where this PTE used to be set,
	 * mark them all as migrating, and flush the page out of
	 * TLB in all the mm's that are referencing the page,
	 * and out of the kernel lowmem or kmap area (if any).
	 * We flip anonymous PTEs to "migrating" (resetting them below),
	 * but just clear file PTEs outright.
	 */
	if (pages == 1) {
		int rc = try_to_unmap(page, ttu_flags | TTU_HOMECACHE_START);
		BUG_ON(!PageAnon(page) && rc != SWAP_SUCCESS);
	}

	/*
	 * Lock out any new kmaps so no new kernel PTEs can be created until
	 * we have finished migration; this also disables interrupts
	 * while the spinlock is held, to avoid self-deadlock.
	 */
	local_irq_save(flags);
	homecache_kpte_lock();
	vaddr = (void *)homecache_fix_kpte(page, pages, 0);
	if (vaddr) {
		flush_remote(0, 0, NULL,
			     (HV_VirtAddr) vaddr, pages * PAGE_SIZE, PAGE_SIZE,
			     cpu_online_mask, NULL, 0);
	}

	/*
	 * Flush the caches, since no cpu can touch the caches that we
	 * are migrating away from now.
	 */
	homecache_finv_pages(page, pages, 1);

	/* Mark the pages with their new cache info. */
	set_pages_home(page, pages, new_home);

	/* Release the kpte lock since new kmaps can be created now. */
	homecache_kpte_unlock();

	/* Make any anonymous user PTEs assume their correct value. */
	if (PageAnon(page) && pages == 1)
		try_to_unmap(page, ttu_flags | TTU_HOMECACHE_FINISH);

	/* Fix the kernel PTE. */
	homecache_fix_kpte(page, pages, 1);

	ts->homecache_is_migrating = false;

	/*
	 * We need to enable irq after migrating done to prevent potential
	 * deadlock in page fault hander which can wait for pte migrating
	 * done.
	 */
	local_irq_restore(flags);
}

/*
 * This method checks the given home against the passed pgprot (and
 * whether we intend to write to it) and returns an appropriate new home.
 */
static int homecache_get_desired_home(int home, pgprot_t prot, int writable)
{
	if (home == PAGE_HOME_IMMUTABLE) {
		/*
		 * Immutable pages are treated specially.  If we are
		 * writing to them, we convert them to normal pages
		 * following the pgprot.  Otherwise, we do nothing,
		 * since any pgprot is compatible with an immutable page.
		 */
		if (!writable)
			return home;
		home = default_page_home();
	}

	/* If the pgprot isn't intended to force the mapping, we're done. */
	if (!pte_get_forcecache(prot))
		return home;

	switch (hv_pte_get_mode(prot)) {
	case HV_PTE_MODE_UNCACHED:
		home = PAGE_HOME_UNCACHED;
		break;
	case HV_PTE_MODE_CACHE_NO_L3:
		/*
		 * If we are just caching locally, we must be
		 * either incoherent or immutable.  Tolerate a
		 * read-only mapping of incoherent memory.
		 */
		if (home != PAGE_HOME_INCOHERENT)
			home = writable ? PAGE_HOME_INCOHERENT :
				PAGE_HOME_IMMUTABLE;
		break;
	case HV_PTE_MODE_CACHE_TILE_L3:
		/* Set the page home if requested by the pgprot. */
		if (!pte_get_anyhome(prot)) {
			/*
			 * Get requested CPU.  Note that users can't
			 * mmap() with a hypervisor lotar, so if one were
			 * in the pgprot, we would correctly assert
			 * in get_remote_cache_cpu().
			 */
			home = get_remote_cache_cpu(prot);
		} else {
			/* A lotar with anyhome is confused. */
			BUG_ON(hv_pte_get_lotar(prot));
			if (home < 0)
				home = smp_processor_id();
		}
		/* Writable pages can't be immutable. */
		if (!writable && hv_pte_get_nc(prot))
			home = PAGE_HOME_IMMUTABLE;
		break;
#if CHIP_HAS_CBOX_HOME_MAP()
	case HV_PTE_MODE_CACHE_HASH_L3:
		home = PAGE_HOME_HASH;
		BUG_ON(hv_pte_get_lotar(prot) != 0);
		/* Writable pages can't be immutable. */
		if (!writable && hv_pte_get_nc(prot))
			home = PAGE_HOME_IMMUTABLE;
		break;
#endif
	default:
		panic("Invalid mode in pte %#llx", hv_pte_val(prot));
		break;
	}
	return home;
}

void homecache_home_page_here(struct page *page, int order, pgprot_t prot)
{
	int home = page_home(page);

	/*
	 * If this pgprot forces the page to be homed somewhere specific,
	 * just return and don't try to move it around.
	 */
	if (home != PAGE_HOME_IMMUTABLE &&
	    pte_get_forcecache(prot) &&
	    (hv_pte_get_mode(prot) == HV_PTE_MODE_UNCACHED ||
	     hv_pte_get_mode(prot) == HV_PTE_MODE_CACHE_NO_L3 ||
#if CHIP_HAS_CBOX_HOME_MAP()
	     hv_pte_get_mode(prot) == HV_PTE_MODE_CACHE_HASH_L3 ||
#endif
	     !pte_get_anyhome(prot)))
		return;

	/* Make sure the page is actually homed on a single cpu. */
	if (home < 0 && home != PAGE_HOME_IMMUTABLE)
		return;

	/* Change this page to be coherently cached on this cpu. */
	home = homecache_get_desired_home(default_page_home(), prot, 1);

	/* Re-home the page. */
	homecache_home_mapped_page(page, order, home);
}

void homecache_update_page(struct page *page, int order,
			   struct vm_area_struct *vma, int writable)
{
	int home = page_home(page);
	pgprot_t prot = vma->vm_page_prot;

	/*
	 * If there is already a shared writable mapping for this file, it
	 * will own the caching of its pages, so just return early.
	 *
	 * FIXME: walk through the vmas with vma_prio_tree_foreach()
	 * and if we overlap with a shared one, force its homing here,
	 * and if not, use our requested homing.  This would also give
	 * us better granularity, since there might be a non-overlapping
	 * shared-writable mapping that this mapping could then ignore.
	 */
	if (!(vma->vm_flags & VM_SHARED) &&
	    vma->vm_file && vma->vm_file->f_mapping->i_mmap_writable > 0)
		return;

	/*
	 * If we are setting up a shared writable mapping, we may not
	 * come into this path via an actual write, but we still want
	 * to set up the mapping as writable.
	 */
	if (hv_pte_get_writable(prot))
		writable = 1;

	/*
	 * If the access is for read, and the mapping is private,
	 * and the page is from a file and is not shared writably,
	 * we ignore "prot" and make it immutable instead.
	 *
	 * If noallocl2 is set, we never cache pages locally, so
	 * there's no point in claiming they are immutable.
	 */
	if (!writable && !(vma->vm_flags & VM_SHARED) && !noallocl2 &&
	    vma->vm_file && vma->vm_file->f_mapping->i_mmap_writable == 0) {
		home = PAGE_HOME_IMMUTABLE;
	} else {
		home = homecache_get_desired_home(home, prot, writable);
	}

	homecache_home_mapped_page(page, order, home);
}

void homecache_make_writable(struct page *page, int order)
{
	int home = page_home(page);
	if (home == PAGE_HOME_IMMUTABLE) {
		home = homecache_get_desired_home(home, PAGE_KERNEL, 1);
		homecache_home_mapped_page(page, order, home);
	}
}

void homecache_new_user_page(struct page *page, int order,
			     pgprot_t prot, int writable)
{
	int home = page_home(page);

	home = homecache_get_desired_home(home, prot, writable);
	homecache_change_page_home(page, order, home);
}


/* Information needed to migrate user-space PTEs. */
struct migrating_pte {
	pte_t pteval;           /* copy of PTE (with migrating bit set) */
	spinlock_t *ptl;        /* non-NULL if this entry locked the PTE */
	pmd_t *pmd;             /* PMD that this pte is on */
	unsigned long va;       /* address for this PTE */
	struct page *page_lock; /* non-NULL if this entry locked the page */
};

/*
 * Number of entries to migrate per iteration.
 * This is just a tradeoff of stack memory used vs number of times
 * we need to walk through the outer migration loop.
 */
#define MIGRATING_COUNT 32

/* State for the migration algorithm, kept on-stack. */
struct migrate_state {
	cpumask_t *cache_cpumask;		/* cpus to flush cache on */
	cpumask_t *tlb_cpumask;			/* cpus to flush TLB on */
	int cpu;				/* cpu that owns this struct */
	int num_rem_asids;			/* remote ASID count */
	HV_Remote_ASID *rem_asids;		/* remote ASIDs */
	unsigned long low_kaddr, high_kaddr;	/* bounds of kaddrs to flush */
	int migrating_index;			/* next entry in migrating[] */
	struct migrating_pte migrating[MIGRATING_COUNT];
};

/* Add information for a new migrating_pte to the list. */
static void add_migrating_pte(struct migrate_state *ms, pte_t pteval,
			      spinlock_t *ptl, pmd_t *pmd, unsigned long va,
			      struct page *page_lock)
{
	struct migrating_pte *mpte;
	BUG_ON(ms->migrating_index >= MIGRATING_COUNT);
	mpte = &ms->migrating[ms->migrating_index++];
	mpte->pteval = pteval;
	mpte->ptl = ptl;
	mpte->pmd = pmd;
	mpte->va = va;
	mpte->page_lock = page_lock;
}

/* Check to see if we're already locked a given page. */
static int is_page_locked(struct migrate_state *ms, struct page *page)
{
	int i, count = ms->migrating_index;
	for (i = 0; i < count; ++i)
		if (ms->migrating[i].page_lock == page)
			return 1;
	return 0;
}

/* Check to see if we're already locked a given page table lock. */
static int is_page_table_locked(struct migrate_state *ms, spinlock_t *ptl)
{
	int i, count = ms->migrating_index;
	for (i = 0; i < count; ++i)
		if (ms->migrating[i].ptl == ptl)
			return 1;
	return 0;
}

/*
 * Add information on a region of kernel VAs that we need to flush.
 * Right now we end up just passing a single (start,size) argument to
 * the hypervisor, but we structure it as an API here so that we
 * can use a modified hypervisor API more easily at a later date.
 */
static void add_kaddr_flush(struct migrate_state *ms,
			    unsigned long kaddr, unsigned long size)
{
	unsigned long end = kaddr + size;
	if (kaddr < ms->low_kaddr)
		ms->low_kaddr = kaddr;
	if (end > ms->high_kaddr)
		ms->high_kaddr = end;
}

/*
 * Get a PTE pointer for a small or huge page in the current process
 * from a PMD and address.  Note that this relies on the tile
 * architecture using the same format for PTEs and PGDs (and thus PMDs).
 */
static pte_t *map_pte(pmd_t *pmd, unsigned long address)
{
	if (pmd_huge_page(*pmd))
		return (pte_t *) pmd;
	else
		return pte_offset_map(pmd, address);
}

/* Unmap a small or huge PTE (only necessary for small PTEs). */
static inline void unmap_pte(pmd_t *pmd, pte_t *ptep)
{
	if (!pmd_huge_page(*pmd))
		pte_unmap(ptep);
}

/*
 * Set the migrating bit on the page and PTE (and any kernel PTE),
 * and update the TLB flush info and cache flush info in the migrate_state.
 * Return the pteval that we should expect to find when we finish migrating.
 */
static pte_t migrate_start_page(struct migrate_state *ms,
				pte_t *ptep, struct page *page,
				unsigned long va)
{
	pte_t pteval = *ptep;

	/*
	 * Rewrite the PTE as migrating so any attempt to use it will
	 * cause a hang.  We use ptep_get_and_clear() to avoid racing
	 * with the hypervisor's dirty/accessed bit setting.  Note that
	 * there is a brief window of vulnerability where the pte is
	 * zero-valued, but this is true elsewhere too, e.g. mprotect.
	 */
	pteval = pte_mkmigrate(ptep_get_and_clear(current->mm, va, ptep));
	set_pte_at(current->mm, va, ptep, pteval);

	/* Record that we need to flush the old cpu's cache. */
	cpumask_set_cpu(page_home(page), ms->cache_cpumask);

	return pteval;
}

/* Poison any kernel PTEs for the page and track any TLB flushes. */
static void migrate_start_kpte(struct migrate_state *ms,
			       struct migrating_pte *mpte)
{
	pte_t pteval = mpte->pteval;
	struct page *page = pfn_to_page(pte_pfn(pteval));
	int npages = pte_huge(pteval) ? (1 << HUGETLB_PAGE_ORDER) : 1;
	unsigned long kva = homecache_fix_kpte(page, npages, 0);
	if (kva != 0)
		add_kaddr_flush(ms, kva, npages * PAGE_SIZE);
}

/* Adjust the page so it is ready to go with its new cpu home. */
static void migrate_finish_page(struct migrating_pte *mpte, int cpu)
{
	pte_t pteval = mpte->pteval;
	struct page *page = pfn_to_page(pte_pfn(pteval));
	int npages = pte_huge(pteval) ? (1 << HUGETLB_PAGE_ORDER) : 1;
	int i;

	/* Fix the page attributes. */
	for (i = 0; i < npages; i++)
		set_page_home(&page[i], cpu);
}

/*
 * Adjust the pte(s) so they are ready to go with their new cpu home.
 * On exit, any cpus that were spinning in page fault are now
 * released, get the updated pte and reset their TLBs appropriately.
 */
static void migrate_finish_pte(struct migrating_pte *mpte, int cpu)
{
	pmd_t *pmd = mpte->pmd;
	pte_t pteval = mpte->pteval;
	struct page *page = pfn_to_page(pte_pfn(pteval));
	pte_t *ptep;
	int npages = pte_huge(pteval) ? (1 << HUGETLB_PAGE_ORDER) : 1;

	/* Adjust the user PTE. */
	ptep = map_pte(pmd, mpte->va);
	pteval = pte_donemigrate(set_remote_cache_cpu(pteval, cpu));
	set_pte_at(current->mm, mpte->va, ptep, pteval);
	unmap_pte(pmd, ptep);

	/* Adjust any kernel PTEs referencing this page. */
	homecache_fix_kpte(page, npages, 1);
}

/*
 * Given a PTE, inspect it to see if it's one we can migrate; if
 * so, return a pointer to the page so we can try to lock it.
 */
static struct page *pte_to_migratable_page(pte_t pteval, int cpu)
{
	struct page *page;
	int home;

	if (!pte_present(pteval))
		return NULL;

	/* Only migrate pages that are coherently cached on a single cpu. */
	if (hv_pte_get_mode(pteval) != HV_PTE_MODE_CACHE_TILE_L3 ||
	    hv_pte_get_nc(pteval))
		return NULL;

	/* Sanity-check the PTE against the page info. */
	page = pfn_to_page(pte_pfn(pteval));
	home = page_home(page);
	if (home != get_remote_cache_cpu(pteval))
		panic("Candidate PTE %#llx (home %d) has PFN %#lx (home %d)",
		      pteval.val, get_remote_cache_cpu(pteval),
		      pte_pfn(pteval), home);

	/* If we're already homed on this cpu, no need to migrate! */
	if (home == cpu)
		return NULL;

	/* If the cpu is not one the hypervisor can cache-flush, skip it. */
	BUG_ON(home < 0 || home >= NR_CPUS);
	if (!cpu_cacheable(home))
		return NULL;

	/* If no page flags are set, it's probably a device mapping. */
	if ((page->flags & ((1 << __NR_PAGEFLAGS) - 1)) == 0)
		return NULL;

	return page;
}

/* Check that the page is one that we want to migrate. */
static int page_migrates_with_process(pte_t pteval, struct page *page)
{
	/*
	 * If the page is mapped into multiple mm's, we don't migrate
	 * it, since we don't provide try_to_unmap() functionality.
	 * There's also not a clear advantage to migrating it.
	 *
	 * NOTE: This also excludes pages that are mapped twice into
	 * the same mm, but this is a rare case, so we don't worry.
	 * We actually do support migrating a page mapped more than once
	 * (see the is_page_locked() calls in maybe_migrate(), below)
	 * so if we do need to do this later it may not be that hard.
	 *
	 * If the mapcount is zero, it may be a page of memory mapped
	 * by a device driver, in which case we also don't want to try
	 * to migrate it.
	 */
	if (page_mapcount(page) != 1)
		return 0;

	/* Unlikely to map one of these, but might as well check. */
	if (PageHomecacheNomigrate(page))
		return 0;

	return 1;
}

/*
 * We enter with a candidate VA and a flag indicating whether we should
 * use "trylock" instead of lock, and no locks held (other than the
 * mmap_sem held for read).  We return 0 if things went OK, and 1 if
 * we were in "trylock" mode and failed to acquire a lock.
 *
 * First we validate that the PTE is plausible, and return early if not.
 * Then we try to get a lock on the page, and then map and lock the page
 * table.  This is a bit tricky because we have to lock the page before
 * the page table to respect the ordering in mm/rmap.c.  This means we
 * get a tentative page from the pte, then lock it, lock the page table,
 * and validate the PTE.  If the PTE has changed (perhaps because
 * another thread upgraded a zero-page ref to writable while we were
 * working) we try again until the PTE value is stable.  Once we have a
 * stable, migratable PTE, we call migrate_start_page() on it, and return.
 *
 * Prior to taking any page or page table locks, we scan the list of
 * locks we are currently holding to avoid double-taking any locks.
 * Note that this means that if we already have a page table lock for
 * some page, we will end up trying to take the page lock after the page
 * table lock, in violation of the rmap.c ordering; but since at that
 * point we must already be in trylock mode, and have already made some
 * progress, it doesn't matter.
 *
 * Note that we must have interrupts enabled during this routine
 * since we are acquiring the page lock and the page table lock.
 */
static int maybe_migrate(struct migrate_state *ms,
			 unsigned long va, int try_lock)
{
	pte_t *ptep;
	pte_t pteval;
	spinlock_t *ptl;  /* page table lock for "va" */
	struct page *page;
	struct mm_struct *mm = current->mm;
	int took_page_lock, took_page_table_lock;

	/* Map in the PTE. */
	pmd_t *pmd = pmd_offset(pud_offset(pgd_offset(mm, va), va), va);
	pmd_t pmdval = *pmd;
	if (!pmd_present(pmdval))
		return 0;
	ptep = map_pte(pmd, va);

	/*
	 * Lock the page table (unless we locked it for a previous page).
	 * We have to do this so it's safe to examine the PTE's page struct.
	 */
	took_page_table_lock = 0;
	if (pmd_huge_page(pmdval))
		ptl = &mm->page_table_lock;
	else
		ptl = pte_lockptr(mm, pmd);
	if (!is_page_table_locked(ms, ptl)) {
		if (!spin_trylock(ptl)) {
			if (try_lock) {
				unmap_pte(pmd, ptep);
				return 1;
			}
			spin_lock(ptl);
		}
		took_page_table_lock = 1;
	}

 retry:
	/* See if we are interested in this PTE. */
	pteval = *ptep;
	page = pte_to_migratable_page(pteval, ms->cpu);
	if (page == NULL || !page_migrates_with_process(pteval, page)) {
		if (took_page_table_lock)
			spin_unlock(ptl);
		unmap_pte(pmd, ptep);
		return 0;
	}

	/* Now try to take the page lock. */
	took_page_lock = 0;
	if (!is_page_locked(ms, page)) {
		if (TestSetPageLocked(page)) {
			if (try_lock) {
				if (took_page_table_lock)
					spin_unlock(ptl);
				unmap_pte(pmd, ptep);
				return 1;
			}

			/*
			 * This is the first page we're trying to acquire,
			 * so we have to take the page lock first to avoid
			 * deadlock with (e.g.)  the swapper.  But this
			 * means we have to drop the existing page table
			 * lock, which means we have to bump up the
			 * reference count on the page beforehand, so we
			 * can still validly look at it when we try to lock
			 * it.  Then we have to check the PTE to make sure
			 * it didn't change while we had the PTL dropped.
			 */
			BUG_ON(!took_page_table_lock);
			get_page(page);
			spin_unlock(ptl);
			lock_page(page);
			spin_lock(ptl);
			if (unlikely(!pte_same(*ptep, pteval))) {
				unlock_page(page);
				put_page(page);
				goto retry;
			}

			/*
			 * Drop the extra refcount; we don't need it since
			 * we will leave the PTL locked from now on.
			 */
			put_page(page);
		}

		/* Now that we have the lock, recheck the page. */
		if (!page_migrates_with_process(pteval, page)) {
			unlock_page(page);
			if (took_page_table_lock)
				spin_unlock(ptl);
			unmap_pte(pmd, ptep);
			return 0;
		}

		took_page_lock = 1;
	}

	/* Mark the page for migrating and unmap the PTE. */
	pteval = migrate_start_page(ms, ptep, page, va);
	unmap_pte(pmd, ptep);

	/* Record what we migrated and what locks we took out. */
	if (!took_page_lock)
		page = NULL;
	if (!took_page_table_lock)
		ptl = NULL;
	add_migrating_pte(ms, pteval, ptl, pmd, va, page);
	if (page)
		++homecache_proc_migrated_mapped;

	return 0;
}

/*
 * Walk the user pages and try to start migrating the ones that need
 * it.  We enter holding the mmap_sem for read.  We return 0 if we
 * were able to migrate every page we were interested in, and the VA
 * to restart at if we need to complete this migration pass and then
 * try again.  On exit, the passed migrate_state structure is updated
 * with the list of user PTEs chosen to migrate, and the kernel VA
 * range is updated with any kernel addresses that have to be
 * explicitly flushed.
 *
 * Marking all the pages for migrating is tricky since we have to
 * worry about ABBA deadlock.  If we've already locked some pages and
 * marked them as migrating, then try to lock a new page or a page
 * table, it's possible that some other thread already holds that
 * lock, but is blocked trying to lock, or create a PTE for, a page
 * that we have already started to migrate.  This would be a deadlock,
 * but instead maybe_migrate() bails out (returning a non-zero start
 * va), we short-circuit this routine, complete the whole migration
 * pass for the pages we've already marked for migration, then loop
 * back in homecache_migrate() and retry.  This way we allow the other
 * task to make forward progress, thus allowing us to eventually be
 * able to acquire the lock that we need as well.
 */
static unsigned long migrate_start_user(struct migrate_state *ms,
					unsigned long start_va)
{
	struct task_struct *p = current;
	struct mm_struct *mm = p->mm;
	int is_threaded = (atomic_read(&mm->mm_users) > 1);
	pid_t mypid = current->pid;
	unsigned long usp0 = p->thread.usp0;
	struct vm_area_struct *vm;

	/* Walk user pages and discover which should be migrated. */
	for (vm = mm->mmap; vm != NULL; vm = vm->vm_next) {
		unsigned long va;
		int page_size = vma_kernel_pagesize(vm);

		/* Handle MAP_CACHE_HOME_TASK regions. */
		if (vm->vm_pid != 0) {
			/* Skip regions owned by another task. */
			if (vm->vm_pid != mypid)
				continue;

			/* Update vm_page_prot for subsequent faults. */
			vm->vm_page_prot =
				set_remote_cache_cpu(vm->vm_page_prot,
						     ms->cpu);
		} else {
			/* Don't try to migrate regions with explicit homes */
			if (pte_get_forcecache(vm->vm_page_prot) &&
			    !pte_get_anyhome(vm->vm_page_prot))
				continue;

			/* If threaded, we only migrate the stack. */
			if (is_threaded &&
			    (usp0 < vm->vm_start || usp0 >= vm->vm_end))
				continue;
		}

		/* Walk each page in the region. */
		va = vm->vm_start > start_va ? vm->vm_start : start_va;
		for (; va < vm->vm_end; va += page_size) {
			int try_lock;

			/* If we can't store any more PTE info, retry. */
			if (ms->migrating_index >= MIGRATING_COUNT)
				return va;

			/*
			 * Check this address to see if it needs to
			 * migrate.  If we've already marked page(s) for
			 * migration, use "trylock" to avoid deadlock.
			 * If we get a trylock failure notification,
			 * give up and indicate we should retry.
			 */
			try_lock = (ms->migrating_index != 0);
			if (maybe_migrate(ms, va, try_lock) != 0)
				return va;
		}
	}

	return 0;
}

/*
 * When migrating the kernel stack pages in a 4KB system, we have
 * to migrate multiple PTEs.  To accommodate this, we must guarantee
 * prior to entering the assembly code that does migration that
 * the stack is mapped by PTEs on the same leaf page table, and
 * that the actual PTEs differ only by having contiguous PFNs.
 * Rather than just corrupt them, we validate the preconditions here.
 */
static void homecache_validate_stack_ptes(unsigned long stack_va,
					  pte_t* stack_ptep)
{
#if THREAD_SIZE > PAGE_SIZE
	pte_t stack_pteval = *stack_ptep;
	unsigned long stack_pfn = pte_pfn(stack_pteval);
	int i;
	for (i = 1; i < THREAD_SIZE / PAGE_SIZE; ++i) {
		unsigned long va = stack_va + (i * PAGE_SIZE);
		pte_t *ptep = virt_to_pte(NULL, va);
		pte_t pteval = pfn_pte(stack_pfn + i, stack_pteval);
		BUG_ON(ptep - stack_ptep != i);
		BUG_ON(pteval.val != ptep->val);
	}
#endif
}

static void migrate_stack_and_flush(const struct cpumask *cache_cpumask,
				    struct cpumask *tlb_cpumask,
				    unsigned long va, unsigned long size,
				    HV_Remote_ASID* rem_asids,
				    int num_rem_asids,
				    pte_t stack_pte, pte_t *stack_ptep)
{
	int rc;
	unsigned long long irqmask;

#ifdef CONFIG_DATAPLANE
	set_remote_flush_count(tlb_cpumask, +1);
#endif
	hv_flush_update(cache_cpumask, tlb_cpumask, va, size,
			rem_asids, num_rem_asids);
	irqmask = interrupt_mask_save_mask();
	interrupt_mask_set_mask(-1ULL);
	rc = homecache_migrate_stack_and_flush(stack_pte, va, size,
					       stack_ptep, cache_cpumask,
					       tlb_cpumask, rem_asids,
					       num_rem_asids);
	interrupt_mask_restore_mask(irqmask);
	if (rc != 0)
		panic("homecache_migrate_stack_and_flush: %d", rc);
#ifdef CONFIG_DATAPLANE
	set_remote_flush_count(tlb_cpumask, -1);
#endif
}

/*
 * Kernel tasks only migrate their stack, at most.  So for kernel
 * tasks, we run a minimal version of homecache_trymigrate().
 */
void homecache_migrate_kthread(void)
{
	struct thread_info *ti = current_thread_info();
	struct task_struct *p = current;
	struct thread_struct *ts = &p->thread;
	unsigned long stack_va = (unsigned long) p->stack;
	unsigned long stack_pfn = kaddr_to_pfn((void *)stack_va);
	pte_t *stack_ptep = virt_to_pte(NULL, stack_va);
	pte_t stack_pte = *stack_ptep;
	struct page *stack_page = pfn_to_page(stack_pfn);
	int stack_home = page_home(stack_page);
	const struct cpumask *stack_cachemask;
	struct cpumask *stack_tlbmask;
	int cpu = smp_processor_id();
	int i;

	/* Set the homecache_cpu to reflect that we have migrated. */
	ti->homecache_cpu = cpu;

	/* See if we actually need to do anything. */
#if CHIP_HAS_CBOX_HOME_MAP()
	if (unlikely(stack_home == PAGE_HOME_HASH)) {
		/*
		 * Possible only for the boot idle task during init
		 * before we move it to a properly-homed stack.
		 */
		return;
	}
#endif
	if (unlikely(stack_home == cpu))
		return;

	ts->homecache_is_migrating = true;
	BUG_ON(stack_home != pte_to_home(stack_pte));
	BUG_ON(stack_home < 0 || stack_home > NR_CPUS);
	homecache_validate_stack_ptes(stack_va, stack_ptep);
	stack_cachemask = &cpumask_of_cpu(stack_home);
	stack_pte = set_remote_cache_cpu(stack_pte, cpu);
	stack_tlbmask = (struct cpumask *) p->thread.homecache_tlb_cpumask;
	memcpy(stack_tlbmask, cpu_online_map.bits, sizeof(cpu_online_map));
	migrate_stack_and_flush(stack_cachemask, stack_tlbmask,
				stack_va, THREAD_SIZE, NULL, 0,
				stack_pte, stack_ptep);
	for (i = 0; i < THREAD_SIZE_PAGES; ++i)
		set_page_home(stack_page + i, cpu);
	homecache_proc_migrated_mapped += THREAD_SIZE / PAGE_SIZE;
	homecache_proc_migrated_tasks++;
	ts->homecache_is_migrating = false;
}

/*
 * Migrate the caching of the current task's pages to its new cpu.
 * Return 0 if we completed successfully, otherwise the VA we should
 * restart at if we faced possible deadlock and gave up part way through.
 * The first invocation must be passed start_va as "0", because this
 * indicates the invocation that will migrate the kernel stack as well.
 */
static unsigned long homecache_trymigrate(struct migrate_state *ms,
					  unsigned long start_va)
{
	struct task_struct *p = current;
	struct page *uninitialized_var(stack_page);
	pte_t *stack_ptep;
	pte_t stack_pte;
	int stack_home;
	int cpu = ms->cpu;
	unsigned long end_va;
	unsigned long flags;
	int i, other_cpu;
	int migrate_stack;

	/*
	 * For vfork'ed children, just return immediately; the parent
	 * still owns the pages, so we don't want to move any of them.
	 */
	if (p->vfork_done != NULL)
		return 0;

	/* Initialize the migrating_state */
	cpumask_clear(ms->cache_cpumask);
	cpumask_clear(ms->tlb_cpumask);
	ms->num_rem_asids = 0;
	ms->migrating_index = 0;
	ms->high_kaddr = 0;
	ms->low_kaddr = -1UL;

	/*
	 * This should only ever be called just before returning
	 * a task to user-space, but be paranoid and check.
	 */
	BUG_ON(in_interrupt());

	/* Mark user PTEs for migration. */
	down_read(&p->mm->mmap_sem);
	end_va = migrate_start_user(ms, start_va);
	up_read(&p->mm->mmap_sem);

	if (ms->migrating_index == 0) {
#if CHIP_HAS_CBOX_HOME_MAP()
		/*
		 * In kstack_hash mode, we won't migrate any
		 * kernel pages, and if we didn't find any
		 * user pages to migrate either, we're done.
		 */
		if (kstack_hash)
			return end_va;
#endif
	} else {
		/*
		 * Construct the cpu/ASID vector to flush,
		 * based on what other threads are sharing
		 * this mm.  Once we have real ASID support we
		 * will probably have something like a
		 * cpu/ASID vector in the mm.  For now, we
		 * just construct one manually.
		 */
		for_each_cpu(other_cpu, mm_cpumask(p->mm)) {
			int index = ms->num_rem_asids++;
			HV_Remote_ASID *rem_asid =
				&ms->rem_asids[index];
			rem_asid->x = other_cpu % smp_width;
			rem_asid->y = other_cpu / smp_width;
			rem_asid->asid =
				per_cpu(current_asid, other_cpu);
		}
	}

	/*
	 * On our first pass, mark kernel stack for migration.
	 * For kstack_hash, the kernel stack is hash-for-home,
	 * so we never migrate it.
	 */
#if CHIP_HAS_CBOX_HOME_MAP()
	if (kstack_hash)
		migrate_stack = 0;
	else
#endif
	migrate_stack = (start_va == 0);

	if (migrate_stack) {
		/* See comments above in homecache_migrate_kthread(). */
		unsigned long stack_va = (unsigned long)(p->stack);
		unsigned long stack_pfn = kaddr_to_pfn(p->stack);
		stack_ptep = virt_to_pte(NULL, stack_va);
		stack_pte = *stack_ptep;
		stack_page = pfn_to_page(stack_pfn);
		stack_home = page_home(stack_page);
		homecache_validate_stack_ptes(stack_va, stack_ptep);
		BUG_ON(stack_home != pte_to_home(stack_pte));
		if (unlikely(stack_home == cpu)) {
			migrate_stack = 0;
		} else {
			cpumask_set_cpu(stack_home, ms->cache_cpumask);
			stack_pte = set_remote_cache_cpu(stack_pte, cpu);
			homecache_proc_migrated_mapped +=
				THREAD_SIZE / PAGE_SIZE;
			add_kaddr_flush(ms, stack_va, THREAD_SIZE);
		}
	} else {
		/* Provide something for the assembly helper to scribble on. */
		stack_ptep = &stack_pte;
		__pte_clear(stack_ptep);  /* avoid uninitialized data */
	}

	/*
	 * Take out the kpte lock, and disable interrupts, to avoid
	 * any new kernel PTEs being created while we run this code.
	 * Then poison any kernel ptes.
	 * Also note that we need interrupts disabled around the call
	 * to homecache_migrate_stack_and_flush(), if migrate_stack is true,
	 * since we are them marking this task's own stack as migrating.
	 */
	local_irq_save(flags);
	homecache_kpte_lock();
	for (i = 0; i < ms->migrating_index; ++i)
		migrate_start_kpte(ms, &ms->migrating[i]);

	/*
	 * Call homecache_migrate_stack_and_flush() to ensure the pages that
	 * we're migrating are flushed from all TLBs and caches,
	 * then finally write the revised stack_pte to *stack_ptep.
	 */
	if (ms->low_kaddr > ms->high_kaddr) {
		ms->low_kaddr = 0;
		ms->high_kaddr = 0;
	} else {
		cpumask_copy(ms->tlb_cpumask, cpu_online_mask);
	}
	migrate_stack_and_flush(ms->cache_cpumask, ms->tlb_cpumask,
				ms->low_kaddr,
				ms->high_kaddr - ms->low_kaddr,
				ms->rem_asids, ms->num_rem_asids,
				stack_pte, stack_ptep);
	if (migrate_stack) {
		for (i = 0; i < THREAD_SIZE_PAGES; ++i)
			set_page_home(stack_page + i, cpu);
	}

	/* Mark all the page structures as finished migrating. */
	for (i = 0; i < ms->migrating_index; ++i)
		migrate_finish_page(&ms->migrating[i], cpu);

	/* Release the kpte lock, now that we can safely create new kmaps. */
	homecache_kpte_unlock();

	/*
	 * Finish migrating.  We loop in reverse
	 * order since that way we release any shared locks
	 * after all the PTEs that referenced them.
	 */
	for (i = ms->migrating_index - 1; i >= 0; --i) {
		struct migrating_pte *mpte = &ms->migrating[i];

		/*
		 * Validate that we really evicted the page.
		 * FIXME: need to capture length or page-table depth earlier
		 * since pte_huge() alone isn't enough to give the length.
		 * We may not be testing a long enough length here.
		 */
		unsigned long pfn = pte_pfn(mpte->pteval);
		int length = pte_huge(mpte->pteval) ?
			HPAGE_SIZE : PAGE_SIZE;
		sim_validate_lines_evicted(PFN_PHYS(pfn), length);

		/* Write the new PTE (and kernel PTE). */
		migrate_finish_pte(mpte, cpu);

		/* Unlock the page and the page table, if necessary. */
		if (mpte->page_lock)
			unlock_page(mpte->page_lock);
		if (mpte->ptl)
			spin_unlock(mpte->ptl);
	}

	/*
	 * We need to enable irq after migrating done to prevent potential
	 * deadlock in page fault hander which can wait for pte migrating
	 * done.
	 */
	local_irq_restore(flags);

	return end_va;
}

/*
 * Called to migrate the home cache of any pages associated with the
 * task if the cpu has changed and we are resuming back to userspace.
 */
void homecache_migrate(void)
{
	struct migrate_state ms;
	unsigned long start_va, next_va;
	struct thread_struct *ts = &current->thread;

	/* kthreadd takes this path, so redirect it to kernel task path. */
	if (current->mm == NULL) {
		homecache_migrate_kthread();
		return;
	}

	/*
	 * Initialize migrate_state to point to some thread_struct state.
	 * This is memory that we need to pass to the hypervisor to
	 * describe the migration, so can't be on the stack itself.
	 */
	BUILD_BUG_ON(sizeof(ts->homecache_cache_cpumask) !=
		     sizeof(struct cpumask));
	ms.cache_cpumask = (struct cpumask *) &ts->homecache_cache_cpumask;
	ms.tlb_cpumask = (struct cpumask *) &ts->homecache_tlb_cpumask;
	ms.rem_asids = ts->homecache_rem_asids;
	ts->homecache_is_migrating = true;

restart:
	/*
	 * We need interrupts enabled throughout the actual migration
	 * process, in particular so we can handle IPIs to avoid
	 * deadlocks while we are trying to acquire page table locks.
	 */
	local_irq_enable();

	/*
	 * Track the cpu number that we are migrating to.  We don't
	 * use smp_processor_id() during migration, so we perform a
	 * consistent migration, but we recheck at the end in case
	 * we ended up rescheduled to a new cpu before we returned.
	 */
	current_thread_info()->homecache_cpu = ms.cpu = smp_processor_id();
	homecache_proc_migrated_tasks++;

	/*
	 * If we hit a potential deadlock (a page or page table
	 * locked while we had other pages marked for migration) we
	 * just complete migrating the pages we were holding, then
	 * go back and rescan and try to pick up some more pages.
	 */
	start_va = 0;
	while ((next_va = homecache_trymigrate(&ms, start_va)) != 0 &&
	       smp_processor_id() == ms.cpu) {
		BUG_ON(next_va <= start_va);
		start_va = next_va;
	}

	if (unlikely(current->ptrace & PT_TRACE_MIGRATE)) {
		current->ptrace_message = ms.cpu;
		ptrace_notify((PTRACE_EVENT_MIGRATE << 8) | SIGTRAP);
	}

	/* Once we've disabled irqs we won't change cpus. */
	local_irq_disable();

	if (unlikely(ms.cpu != smp_processor_id()))
		goto restart;

	ts->homecache_is_migrating = false;
}

/* Set up homecache-related data in a new zone struct. */
void homecache_init_zone_lists(struct zone *zone)
{
	int i;

	for (i = 0; i < NR_ZONE_HOMECACHE_LISTS; i++)
		INIT_LIST_HEAD(&zone->homecache_list[i]);
	zone->lru_homecache_list = 0;
}

static ctl_table homecache_table[] = {
	{
		.procname	= "migrated_tasks",
		.data		= &homecache_proc_migrated_tasks,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec
	},
	{
		.procname	= "migrated_mapped_pages",
		.data		= &homecache_proc_migrated_mapped,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec
	},
	{
		.procname	= "migrated_unmapped_pages",
		.data		= &homecache_proc_migrated_unmapped,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec
	},
	{
		.procname	= "sequestered_pages_at_free",
		.data		= &homecache_proc_sequestered_free,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec
	},
	{
		.procname	= "coherent_sequestered_purges",
		.data		= &homecache_proc_coherent_sequestered_purge,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec
	},
	{
		.procname	= "sequestered_purges",
		.data		= &homecache_proc_sequestered_purge,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec
	},
	{
		.procname	= "incoherent_finv_pages",
		.data		= &homecache_proc_incoherent_finv_pages,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec
	},
	{}
};

static ctl_table tile_table[] = {
	{
		.procname	= "homecache",
		.mode		= 0555,
		.child		= homecache_table,
	},
	{},
};

static ctl_table root[] = {
	{
		.procname	= "tile",
		.child		= tile_table,
	},
	{},
};

static int __init homecache_proc_init(void)
{
	register_sysctl_table(root);
	return 0;
}
subsys_initcall(homecache_proc_init);

#endif /* CONFIG_HOMECACHE */
