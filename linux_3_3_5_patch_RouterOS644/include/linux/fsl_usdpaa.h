/* Copyright 2011-2012 Freescale Semiconductor, Inc.
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2.  This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#ifndef FSL_USDPAA_H
#define FSL_USDPAA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/uaccess.h>
#include <linux/ioctl.h>

#ifdef CONFIG_FSL_USDPAA

/* Allocation of resource IDs uses a generic interface. This enum is used to
 * distinguish between the type of underlying object being manipulated. */
enum usdpaa_id_type {
	usdpaa_id_fqid,
	usdpaa_id_bpid,
	usdpaa_id_qpool,
	usdpaa_id_cgrid,
	usdpaa_id_max /* <-- not a valid type, represents the number of types */
};
#define USDPAA_IOCTL_MAGIC 'u'
struct usdpaa_ioctl_id_alloc {
	uint32_t base; /* Return value, the start of the allocated range */
	enum usdpaa_id_type id_type; /* what kind of resource(s) to allocate */
	uint32_t num; /* how many IDs to allocate (and return value) */
	uint32_t align; /* must be a power of 2, 0 is treated like 1 */
	int partial; /* whether to allow less than 'num' */
};
struct usdpaa_ioctl_id_release {
	/* Input; */
	enum usdpaa_id_type id_type;
	uint32_t base;
	uint32_t num;
};
/* Maximum length for a map name, including NULL-terminator */
#define USDPAA_DMA_NAME_MAX 16
/* Flags for requesting DMA maps. Maps are private+unnamed or sharable+named.
 * For a sharable and named map, specify _SHARED (whether creating one or
 * binding to an existing one). If _SHARED is specified and _CREATE is not, then
 * the mapping must already exist. If _SHARED and _CREATE are specified and the
 * mapping doesn't already exist, it will be created. If _SHARED and _CREATE are
 * specified and the mapping already exists, the mapping will fail unless _LAZY
 * is specified. When mapping to a pre-existing sharable map, the length must be
 * an exact match. Lengths must be a power-of-4 multiple of page size.
 *
 * Note that this does not actually map the memory to user-space, that is done
 * by a subsequent mmap() using the page offset returned from this ioctl(). The
 * ioctl() is what gives the process permission to do this, and a page-offset
 * with which to do so.
 */
#define USDPAA_DMA_FLAG_SHARE    0x01
#define USDPAA_DMA_FLAG_CREATE   0x02
#define USDPAA_DMA_FLAG_LAZY     0x04
struct usdpaa_ioctl_dma_map {
	/* If the map succeeds, pa_offset is returned and can be used in a
	 * subsequent call to mmap(). */
	uint64_t pa_offset;
	/* Input parameter, the length of the region to be created (or if
	 * mapping an existing region, this must match it). Must be a power-of-4
	 * multiple of page size. */
	uint64_t len;
	/* Input parameter, the USDPAA_DMA_FLAG_* settings. */
	uint32_t flags;
	/* If _FLAG_SHARE is specified, the name of the region to be created (or
	 * of the existing mapping to use). */
	char name[USDPAA_DMA_NAME_MAX];
	/* If this ioctl() creates the mapping, this is an input parameter
	 * stating whether the region supports locking. If mapping an existing
	 * region, this is a return value indicating the same thing. */
	int has_locking;
	/* In the case of a successful map with _CREATE and _LAZY, this return
	 * value indicates whether we created the mapped region or whether it
	 * already existed. */
	int did_create;
};
#define USDPAA_IOCTL_ID_ALLOC \
	_IOWR(USDPAA_IOCTL_MAGIC, 0x01, struct usdpaa_ioctl_id_alloc)
#define USDPAA_IOCTL_ID_RELEASE \
	_IOW(USDPAA_IOCTL_MAGIC, 0x02, struct usdpaa_ioctl_id_release)
#define USDPAA_IOCTL_DMA_MAP \
	_IOWR(USDPAA_IOCTL_MAGIC, 0x03, struct usdpaa_ioctl_dma_map)
/* We implement a cross-process locking scheme per DMA map. Call this ioctl()
 * with a mmap()'d address, and the process will (interruptible) sleep if the
 * lock is already held by another process. Process destruction will
 * automatically clean up any held locks. */
#define USDPAA_IOCTL_DMA_LOCK \
	_IOW(USDPAA_IOCTL_MAGIC, 0x04, unsigned char)
#define USDPAA_IOCTL_DMA_UNLOCK \
	_IOW(USDPAA_IOCTL_MAGIC, 0x05, unsigned char)

#ifdef __KERNEL__

/* Early-boot hook */
void __init fsl_usdpaa_init_early(void);

/* Fault-handling in arch/powerpc/mm/mem.c gives USDPAA an opportunity to detect
 * faults within its ranges via this hook. */
int usdpaa_test_fault(unsigned long pfn, u64 *phys_addr, u64 *size);

#endif /* __KERNEL__ */

#endif /* CONFIG_FSL_USDPAA */

#ifdef __KERNEL__
/* This interface is needed in a few places and though it's not specific to
 * USDPAA as such, creating a new header for it doesn't make any sense. The
 * qbman kernel driver implements this interface and uses it as the backend for
 * both the FQID and BPID allocators. The fsl_usdpaa driver also uses this
 * interface for tracking per-process allocations handed out to user-space. */
struct dpa_alloc {
	struct list_head list;
	spinlock_t lock;
};
#define DECLARE_DPA_ALLOC(name) \
	struct dpa_alloc name = { \
		.list = { \
			.prev = &name.list, \
			.next = &name.list \
		}, \
		.lock = __SPIN_LOCK_UNLOCKED(name.lock) \
	}
static inline void dpa_alloc_init(struct dpa_alloc *alloc)
{
	INIT_LIST_HEAD(&alloc->list);
	spin_lock_init(&alloc->lock);
}
int dpa_alloc_new(struct dpa_alloc *alloc, u32 *result, u32 count, u32 align,
		  int partial);
void dpa_alloc_free(struct dpa_alloc *alloc, u32 base_id, u32 count);
/* Like 'new' but specifies the desired range, returns -ENOMEM if the entire
 * desired range is not available, or 0 for success. */
int dpa_alloc_reserve(struct dpa_alloc *alloc, u32 base_id, u32 count);
/* Pops and returns contiguous ranges from the allocator. Returns -ENOMEM when
 * 'alloc' is empty. */
int dpa_alloc_pop(struct dpa_alloc *alloc, u32 *result, u32 *count);
#endif /* __KERNEL__ */

#ifdef __cplusplus
}
#endif

#endif /* FSL_USDPAA_H */
