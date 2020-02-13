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
 */

#ifndef _ASM_TILE_MMAN_H
#define _ASM_TILE_MMAN_H

#include <asm-generic/mman-common.h>
#include <arch/chip.h>

/* Standard Linux flags */

#define MAP_POPULATE	0x0040		/* populate (prefault) pagetables */
#define MAP_NONBLOCK	0x0080		/* do not block on IO */
#define MAP_GROWSDOWN	0x0100		/* stack-like segment */
#define MAP_STACK	MAP_GROWSDOWN	/* provide convenience alias */
#define MAP_LOCKED	0x0200		/* pages are locked */
#define MAP_NORESERVE	0x0400		/* don't check for reservations */
#define MAP_DENYWRITE	0x0800		/* ETXTBSY */
#define MAP_EXECUTABLE	0x1000		/* mark it as an executable */
#define MAP_HUGETLB	0x4000		/* create a huge page mapping */

#ifdef CONFIG_HOMECACHE

/*
 * Specify the "home cache" for the page explicitly.  The home cache is
 * the cache of one particular "home" cpu, which is used as a coherence
 * point for normal cached operations.  Normally the kernel chooses for
 * you, but you can use the MAP_CACHE_HOME_xxx flags to override.
 *
 * User code should not use any symbols with a leading "_" as they are
 * implementation specific and may change from release to release
 * without warning.
 *
 * See the Tilera mmap(2) man page for more details (e.g. "tile-man mmap").
 */

/* Implementation details; do not use directly. */
#define _MAP_CACHE_INCOHERENT   0x40000
#define _MAP_CACHE_HOME         0x80000
#define _MAP_CACHE_HOME_SHIFT   20
#define _MAP_CACHE_HOME_MASK    0x3ff
#define _MAP_CACHE_MKHOME(n) \
  (_MAP_CACHE_HOME | (((n) & _MAP_CACHE_HOME_MASK) << _MAP_CACHE_HOME_SHIFT))

/* Set the home cache to the specified cpu. */
#define MAP_CACHE_HOME(n)       _MAP_CACHE_MKHOME(n)

/* Set the home cache to the current cpu. */
#define _MAP_CACHE_HOME_HERE    (_MAP_CACHE_HOME_MASK - 0)
#define MAP_CACHE_HOME_HERE     _MAP_CACHE_MKHOME(_MAP_CACHE_HOME_HERE)

/*
 * Request no on-chip home, i.e. read/write direct to memory.  Invalid if both
 * PROT_WRITE and any local caching are specified; see MAP_CACHE_NONE below.
 */
#define _MAP_CACHE_HOME_NONE    (_MAP_CACHE_HOME_MASK - 1)
#define MAP_CACHE_HOME_NONE     _MAP_CACHE_MKHOME(_MAP_CACHE_HOME_NONE)

/* Request no on-chip home, and allow incoherent cached PROT_WRITE mappings. */
#define MAP_CACHE_INCOHERENT    (_MAP_CACHE_INCOHERENT | MAP_CACHE_HOME_NONE)

/* Force the system to choose a single home cache, on a cpu of its choice. */
#define _MAP_CACHE_HOME_SINGLE  (_MAP_CACHE_HOME_MASK - 2)
#define MAP_CACHE_HOME_SINGLE   _MAP_CACHE_MKHOME(_MAP_CACHE_HOME_SINGLE)

/* Create a mapping that follows the task when it migrates. */
#define _MAP_CACHE_HOME_TASK    (_MAP_CACHE_HOME_MASK - 3)
#define MAP_CACHE_HOME_TASK     _MAP_CACHE_MKHOME(_MAP_CACHE_HOME_TASK)

#if CHIP_HAS_CBOX_HOME_MAP()
/* Create a hash-for-home mapping. */
#define _MAP_CACHE_HOME_HASH    (_MAP_CACHE_HOME_MASK - 4)
#define MAP_CACHE_HOME_HASH     _MAP_CACHE_MKHOME(_MAP_CACHE_HOME_HASH)
#endif

/*
 * Specify local caching attributes for the mapping.  Normally the kernel
 * chooses whether to use the local cache, but these flags can be used
 * to override the kernel.
 */

/* Disable use of local L2 (ignored on tile64). */
#define MAP_CACHE_NO_L2         0x20000

/* Disable use of local L1 (ignored on tile64). */
#define MAP_CACHE_NO_L1         0x08000

/* Convenience alias that should be used for forward compatibility. */
#define MAP_CACHE_NO_LOCAL      (MAP_CACHE_NO_L1 | MAP_CACHE_NO_L2)

/* Convenience alias for direct-to-RAM mappings. */
#define MAP_CACHE_NONE          (MAP_CACHE_HOME_NONE | MAP_CACHE_NO_LOCAL)

/* Arrange for this mapping to take priority in the cache. */
#define MAP_CACHE_PRIORITY      0x02000

/*
 * Environment variable that controls hash-for-home in user programs.
 */
#define MAP_CACHE_HASH_ENV_VAR "LD_CACHE_HASH"

#endif

/*
 * Flags for mlockall
 */
#define MCL_CURRENT	1		/* lock all current mappings */
#define MCL_FUTURE	2		/* lock all future mappings */

#if defined(CONFIG_HOMECACHE) && defined(__KERNEL__) && !defined(__ASSEMBLY__)

#include <asm/page.h>

struct vm_area_struct;
struct address_space;
extern int arch_vm_area_flags(struct mm_struct *mm, unsigned long flags,
			      unsigned long vm_flags, pid_t *, pgprot_t *);
extern int arch_vm_area_validate(struct vm_area_struct *,
				 struct address_space *);

#endif /* kernel C code */

#endif /* _ASM_TILE_MMAN_H */
