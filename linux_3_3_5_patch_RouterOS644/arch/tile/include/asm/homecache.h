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
 * Handle issues around the Tile "home cache" model of coherence.
 */

#ifndef _ASM_TILE_HOMECACHE_H
#define _ASM_TILE_HOMECACHE_H

#include <asm/page.h>
#include <linux/cpumask.h>

struct page;
struct task_struct;
struct vm_area_struct;
struct zone;

/*
 * Coherence point for the page is its memory controller.
 * It is not present in any cache (L1 or L2).
 */
#define PAGE_HOME_UNCACHED -1

/*
 * Is this page immutable (unwritable) and thus able to be cached more
 * widely than would otherwise be possible?  On tile64 this means we
 * mark the PTE to cache locally; on tilepro it means we have "nc" set.
 */
#define PAGE_HOME_IMMUTABLE -2

/*
 * Each cpu considers its own cache to be the home for the page,
 * which makes it incoherent.
 */
#define PAGE_HOME_INCOHERENT -3

#if CHIP_HAS_CBOX_HOME_MAP()
/* Home for the page is distributed via hash-for-home. */
#define PAGE_HOME_HASH -4
#endif

/* Homing is unknown or unspecified.  Not valid for page_home(). */
#define PAGE_HOME_UNKNOWN -5

/* Home on the current cpu.  Not valid for page_home(). */
#define PAGE_HOME_HERE -6

/* Support wrapper to use instead of explicit hv_flush_remote(). */
extern void flush_remote(unsigned long cache_pfn, unsigned long cache_length,
			 const struct cpumask *cache_cpumask,
			 HV_VirtAddr tlb_va, unsigned long tlb_length,
			 unsigned long tlb_pgsize,
			 const struct cpumask *tlb_cpumask,
			 HV_Remote_ASID *asids, int asidcount);

/* Set homing-related bits in a PTE (can also pass a pgprot_t). */
extern pte_t pte_set_home(pte_t pte, int home);

/* Do a cache eviction on the specified cpus. */
extern void homecache_evict(const struct cpumask *mask);

/*
 * Change a kernel page's homecache.  It must not be mapped in user space.
 * If !CONFIG_HOMECACHE, only usable on LOWMEM, and can only be called when
 * no other cpu can reference the page, and causes a full-chip cache/TLB flush.
 */
extern void homecache_change_page_home(struct page *, int order, int home);

/*
 * Flush a page out of whatever cache(s) it is in.
 * This is more than just finv, since it properly handles waiting
 * for the data to reach memory, but it can be quite
 * heavyweight, particularly on incoherent or immutable memory.
 */
extern void homecache_finv_page(struct page *);

/*
 * Flush a page out of the specified home cache.
 * Note that the specified home need not be the actual home of the page,
 * as for example might be the case when coordinating with I/O devices.
 */
extern void homecache_finv_map_page(struct page *, int home);

/*
 * Allocate a page with the given GFP flags, home, and optionally
 * node.  These routines are actually just wrappers around the normal
 * alloc_pages() / alloc_pages_node() functions, which set and clear
 * a per-cpu variable to communicate with homecache_new_kernel_page().
 * If !CONFIG_HOMECACHE, uses homecache_change_page_home().
 */
extern struct page *homecache_alloc_pages(gfp_t gfp_mask,
					  unsigned int order, int home);
extern struct page *homecache_alloc_pages_node(int nid, gfp_t gfp_mask,
					       unsigned int order, int home);
#define homecache_alloc_page(gfp_mask, home) \
  homecache_alloc_pages(gfp_mask, 0, home)

/*
 * These routines are just pass-throughs to free_pages() when
 * we support full homecaching.  If !CONFIG_HOMECACHE, then these
 * routines use homecache_change_page_home() to reset the home
 * back to the default before returning the page to the allocator.
 */
void __homecache_free_pages(struct page *, unsigned int order);
void homecache_free_pages(unsigned long addr, unsigned int order);
#define __homecache_free_page(page) __homecache_free_pages((page), 0)
#define homecache_free_page(page) homecache_free_pages((page), 0)

/* Get the home cache of a page. */
extern int page_home(struct page *page);

#ifdef CONFIG_HOMECACHE

/*
 * The homecache system saves some free pages with each kind of
 * desired homing.  There is one list for each cpu home, and
 * additional lists for uncached, hash-for-home (if supported),
 * and incoherent pages (including freed "immutable" pages).
 */
#define ZONE_HOMECACHE_UNCACHED_INDEX (NR_CPUS)
#if CHIP_HAS_CBOX_HOME_MAP()
#define ZONE_HOMECACHE_HFH_INDEX (NR_CPUS + 1)
#define NR_COHERENT_ZONE_HOMECACHE_LISTS (NR_CPUS + 2)
#else
#define NR_COHERENT_ZONE_HOMECACHE_LISTS (NR_CPUS + 1)
#endif
#define ZONE_HOMECACHE_INCOHERENT_INDEX (NR_COHERENT_ZONE_HOMECACHE_LISTS)
#define NR_ZONE_HOMECACHE_LISTS (NR_COHERENT_ZONE_HOMECACHE_LISTS + 1)

/* Set home cache of a page. */
#define set_page_home(page, _home) \
	do { \
		int __home = (_home); \
		BUG_ON(__home <= PAGE_HOME_UNKNOWN || __home >= NR_CPUS); \
		(page)->home = __home; \
	} while (0)

/*
 * Allocate a page intended for user-space with suitable homecaching.
 */
struct page *homecache_alloc_page_vma(gfp_t gfp, struct vm_area_struct *vma,
				      unsigned long addr);

/*
 * Regenerate a PTE that has been migrated by taking the vm_page_prot
 * values for caching and the PTE's own read/write/access/dirty bits,
 * then rewriting the PTE.  This will cause various components (e.g.
 * the home, whether it's coherent, etc.) to be filled in correctly.
 * In addition, reset the PTE to match the page.
 */
extern void homecache_update_migrating_pte(struct page *, pte_t *,
					   struct vm_area_struct *,
					   unsigned long address);

/*
 * Make a freshly-allocated page be homed on the current cpu,
 * or some other home if requested by homecache_alloc_pages() et al.
 */
extern void homecache_new_kernel_page(struct page *, int order);

/*
 * Called by the page_alloc allocation code prior to checking the
 * per-cpu free lists.  If there's a hit for the type of page that
 * we're currently looking for here, we return that page and
 * short-circuit any futher allocation work.
 * Must be called with interrupts disabled.
 */
extern struct page *homecache_get_cached_page(struct zone *zone, gfp_t);

/*
 * Called by the page_alloc free code when just about to return a page
 * to the free pool.  If it returns "1", the generic code should call
 * homecache_keep_free_page() instead of handling the page itself.
 */
extern int homecache_check_free_page(struct page *, int order);

/*
 * Place the specified page into the homecache lists that track
 * pages with interesting homecache settings.
 */
extern void homecache_keep_free_page(struct page *page, int order);

/*
 * Report the number of pages sequestered by
 * homecache_check_free_page().  If total_mem is true, the total
 * amount of memory is reported, otherwise just the lowmem.
 * If include_incoherent is true, the returned value includes
 * incoherent pages (including immutable pages); otherwise the
 * value is only coherent pages.
 */
extern unsigned long homecache_count_sequestered_pages(bool total_mem,
	bool include_incoherent);

/*
 * Recover any free pages that were sequestered by homecache_free_page()
 * by doing a global cache flush and returning them to the free pool.
 * Called from the page allocator when free pool is empty.
 */
extern int homecache_recover_free_pages(void);

/*
 * Take a user page and try to associate it with the current cpu.
 * Called from do_wp_page() when un-cow'ing a page with only one reference.
 * The page must be locked.
 */
extern void homecache_home_page_here(struct page *, int order, pgprot_t);

/*
 * Update caching to match a pgprot, and unmap any other mappings of
 * this page in other address spaces.  Called when we are mapping a page
 * into an address space, before any page table locks are taken.
 * If the page is a file mapping with no shared writers and we are setting
 * up a read-only mapping, we ignore vm_page_prot and make it immutable.
 * The page must be locked.
 */
extern void homecache_update_page(struct page *, int order,
				  struct vm_area_struct *, int writable);

/*
 * Make an immutable page writable by giving it default cache homing.
 * This may only last as long as it takes to complete the action
 * (e.g. page writeout) that required it to be locked in the first place,
 * since if the page is mapped not shared-writable it will be reset to
 * immutable when the page gets faulted back in again.
 * The page must be locked.
 */
extern void homecache_make_writable(struct page *page, int order);

/*
 * Fix the caching on a new page that we are about to map into user space.
 * The page is freshly-allocated, so should not be locked.
 * This is currently only used by the hugepage code; small pages
 * come through homecache_alloc_page_vma().
 */
extern void homecache_new_user_page(struct page *, int order,
				    pgprot_t prot, int writable);

/* Migrate the current user-space process to the current cpu. */
extern void homecache_migrate(void);

/* Migrate the current kernel thread to the current cpu. */
extern void homecache_migrate_kthread(void);

/* Acquire/release the lock needed to create new kernel PTE mappings. */
extern unsigned long homecache_kpte_lock_irqsave(void);
extern void homecache_kpte_unlock_irqrestore(unsigned long);
extern void homecache_kpte_lock(void);
extern void homecache_kpte_unlock(void);

/* Allow setting homecache defaults for user mappings. */
extern int setup_ucache_hash(char *str);

/* Set up homecache-related data in a new zone struct. */
extern void homecache_init_zone_lists(struct zone *zone);

#else

/*
 * Report the page home for LOWMEM pages by examining their kernel PTE,
 * or for highmem pages as the default home.
 */
extern int page_home(struct page *);

#define homecache_migrate_kthread() do {} while (0)

#define homecache_kpte_lock_irqsave() 0
#define homecache_kpte_unlock_irqrestore(flags) do {} while (0)
#define homecache_kpte_lock() 0
#define homecache_kpte_unlock() do {} while (0)

#endif /* CONFIG_HOMECACHE */


#endif /* _ASM_TILE_HOMECACHE_H */
