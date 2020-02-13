/* Copyright 2008-2012 Freescale Semiconductor, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Freescale Semiconductor nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *
 * ALTERNATIVELY, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") as published by the Free Software
 * Foundation, either version 2 of that License or (at your option) any
 * later version.
 *
 * THIS SOFTWARE IS PROVIDED BY Freescale Semiconductor ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Freescale Semiconductor BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DPA_SYS_H
#define DPA_SYS_H

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/bootmem.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/of_platform.h>
#include <linux/kthread.h>
#include <linux/memblock.h>
#include <linux/completion.h>
#include <linux/log2.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/device.h>
#include <linux/uio_driver.h>
#include <linux/smp.h>
#include <sysdev/fsl_soc.h>
#include <linux/fsl_hypervisor.h>
#include <linux/vmalloc.h>
#include <linux/ctype.h>

#include <linux/fsl_usdpaa.h>

/* When copying aligned words or shorts, try to avoid memcpy() */
#define CONFIG_TRY_BETTER_MEMCPY

/* Handle portals destined for USDPAA (user-space).
 *
 * The UIO handling is mostly in dpa_uio.c which is common to qman and bman, but
 * there are some specifics to each case, and they have independent data
 * structures. The "pcfg"s for qman and bman portals are maintained in lists in
 * their respective drivers, and they're detached from those lists when they are
 * to be registered as UIO devices, so we have dpa_uio.c store them in a
 * mixed-type list, and use this vtable of callbacks to let the qman+bman
 * drivers container_of() the list item to their respective object wrappers and
 * implement whatever logic distinguishes them.
 */
struct dpa_uio_vtable {
	/* This callback should fill in 'name', 'mem', and 'irq'. The rest will
	 * be filled in by dpa_uio.c */
	int (*init_uio)(const struct list_head *pcfg, struct uio_info *info);
	/* Free up whatever object contains 'pcfg' */
	void (*destroy)(const struct list_head *pcfg, struct uio_info *info);
	/* Called when the portal is opened (Qman uses this for rerouting
	 * stashing to the current cpu) */
	int (*on_open)(const struct list_head *pcfg);
	void (*on_close)(const struct list_head *pcfg);
	/* Called when an interrupt fires - must disable interrupts */
	void (*on_interrupt)(const struct list_head *pcfg);
};
int __init dpa_uio_register(struct list_head *new_pcfg,
			    const struct dpa_uio_vtable *vtable);

/* For 2-element tables related to cache-inhibited and cache-enabled mappings */
#define DPA_PORTAL_CE 0
#define DPA_PORTAL_CI 1

/* These stubs are re-mapped to hypervisor+failover features in kernel trees
 * that contain that support. */
static inline int pamu_enable_liodn(struct device_node *n, int i)
{
	return 0;
}
/***********************/
/* Misc inline assists */
/***********************/

/* TODO: NB, we currently assume that hwsync() and lwsync() imply compiler
 * barriers and that dcb*() won't fall victim to compiler or execution
 * reordering with respect to other code/instructions that manipulate the same
 * cacheline. */
#define hwsync() \
	do { \
		__asm__ __volatile__ ("sync" : : : "memory"); \
	} while(0)
#define lwsync() \
	do { \
		__asm__ __volatile__ (stringify_in_c(LWSYNC) : : : "memory"); \
	} while(0)
#define dcbf(p) \
	do { \
		__asm__ __volatile__ ("dcbf 0,%0" : : "r" (p) : "memory"); \
	} while(0)
#define dcbt_ro(p) \
	do { \
		__asm__ __volatile__ ("dcbt 0,%0" : : "r" (p)); \
	} while(0)
#define dcbt_rw(p) \
	do { \
		__asm__ __volatile__ ("dcbtst 0,%0" : : "r" (p)); \
	} while(0)
#define dcbi(p) dcbf(p)
#ifdef CONFIG_PPC_E500MC
#define dcbzl(p) \
	do { \
		__asm__ __volatile__ ("dcbzl 0,%0" : : "r" (p)); \
	} while (0)
#define dcbz_64(p) \
	do { \
		dcbzl(p); \
	} while (0)
#define dcbf_64(p) \
	do { \
		dcbf(p); \
	} while (0)
/* Commonly used combo */
#define dcbit_ro(p) \
	do { \
		dcbi(p); \
		dcbt_ro(p); \
	} while (0)
#else
#define dcbz(p) \
	do { \
		__asm__ __volatile__ ("dcbz 0,%0" : : "r" (p)); \
	} while (0)
#define dcbz_64(p) \
	do { \
		dcbz((u32)p + 32);	\
		dcbz(p);	\
	} while (0)
#define dcbf_64(p) \
	do { \
		dcbf((u32)p + 32); \
		dcbf(p); \
	} while (0)
/* Commonly used combo */
#define dcbit_ro(p) \
	do { \
		dcbi(p); \
		dcbi((u32)p + 32); \
		dcbt_ro(p); \
		dcbt_ro((u32)p + 32); \
	} while (0)
#endif /* CONFIG_PPC_E500MC */

static inline u64 mfatb(void)
{
	u32 hi, lo, chk;
	do {
		hi = mfspr(SPRN_ATBU);
		lo = mfspr(SPRN_ATBL);
		chk = mfspr(SPRN_ATBU);
	} while (unlikely(hi != chk));
	return ((u64)hi << 32) | (u64)lo;
}

#ifdef CONFIG_FSL_DPA_CHECKING
#define DPA_ASSERT(x) \
	do { \
		if (!(x)) { \
			pr_crit("ASSERT: (%s:%d) %s\n", __FILE__, __LINE__, \
				__stringify_1(x)); \
			dump_stack(); \
			panic("assertion failure"); \
		} \
	} while(0)
#else
#define DPA_ASSERT(x)
#endif

/* memcpy() stuff - when you know alignments in advance */
#ifdef CONFIG_TRY_BETTER_MEMCPY
static inline void copy_words(void *dest, const void *src, size_t sz)
{
	u32 *__dest = dest;
	const u32 *__src = src;
	size_t __sz = sz >> 2;
	BUG_ON((unsigned long)dest & 0x3);
	BUG_ON((unsigned long)src & 0x3);
	BUG_ON(sz & 0x3);
	while (__sz--)
		*(__dest++) = *(__src++);
}
static inline void copy_shorts(void *dest, const void *src, size_t sz)
{
	u16 *__dest = dest;
	const u16 *__src = src;
	size_t __sz = sz >> 1;
	BUG_ON((unsigned long)dest & 0x1);
	BUG_ON((unsigned long)src & 0x1);
	BUG_ON(sz & 0x1);
	while (__sz--)
		*(__dest++) = *(__src++);
}
static inline void copy_bytes(void *dest, const void *src, size_t sz)
{
	u8 *__dest = dest;
	const u8 *__src = src;
	while (sz--)
		*(__dest++) = *(__src++);
}
#else
#define copy_words memcpy
#define copy_shorts memcpy
#define copy_bytes memcpy
#endif

/************/
/* RB-trees */
/************/

/* We encapsulate RB-trees so that its easier to use non-linux forms in
 * non-linux systems. This also encapsulates the extra plumbing that linux code
 * usually provides when using RB-trees. This encapsulation assumes that the
 * data type held by the tree is u32. */

struct dpa_rbtree {
	struct rb_root root;
};
#define DPA_RBTREE { .root = RB_ROOT }

static inline void dpa_rbtree_init(struct dpa_rbtree *tree)
{
	tree->root = RB_ROOT;
}

#define IMPLEMENT_DPA_RBTREE(name, type, node_field, val_field) \
static inline int name##_push(struct dpa_rbtree *tree, type *obj) \
{ \
	struct rb_node *parent = NULL, **p = &tree->root.rb_node; \
	while (*p) { \
		u32 item; \
		parent = *p; \
		item = rb_entry(parent, type, node_field)->val_field; \
		if (obj->val_field < item) \
			p = &parent->rb_left; \
		else if (obj->val_field > item) \
			p = &parent->rb_right; \
		else \
			return -EBUSY; \
	} \
	rb_link_node(&obj->node_field, parent, p); \
	rb_insert_color(&obj->node_field, &tree->root); \
	return 0; \
} \
static inline void name##_del(struct dpa_rbtree *tree, type *obj) \
{ \
	rb_erase(&obj->node_field, &tree->root); \
} \
static inline type *name##_find(struct dpa_rbtree *tree, u32 val) \
{ \
	type *ret; \
	struct rb_node *p = tree->root.rb_node; \
	while (p) { \
		ret = rb_entry(p, type, node_field); \
		if (val < ret->val_field) \
			p = p->rb_left; \
		else if (val > ret->val_field) \
			p = p->rb_right; \
		else \
			return ret; \
	} \
	return NULL; \
}

/************/
/* Bootargs */
/************/

/* Qman has "qportals=" and Bman has "bportals=", they use the same syntax
 * though; a comma-separated list of items, each item being a cpu index and/or a
 * range of cpu indices, and each item optionally be prefixed by "s" to indicate
 * that the portal associated with that cpu should be shared. See bman_driver.c
 * for more specifics. */
static int __parse_portals_cpu(const char **s, int *cpu)
{
	*cpu = 0;
	if (!isdigit(**s))
		return -EINVAL;
	while (isdigit(**s))
		*cpu = *cpu * 10 + (*((*s)++) - '0');
	return 0;
}
static inline int parse_portals_bootarg(char *str, struct cpumask *want_shared,
					struct cpumask *want_unshared,
					const char *argname)
{
	const char *s = str;
	unsigned int shared, cpu1, cpu2, loop;

keep_going:
	if (*s == 's') {
		shared = 1;
		s++;
	} else
		shared = 0;
	if (__parse_portals_cpu(&s, &cpu1))
		goto err;
	if (*s == '-') {
		s++;
		if (__parse_portals_cpu(&s, &cpu2))
			goto err;
		if (cpu2 < cpu1)
			goto err;
	} else
		cpu2 = cpu1;
	for (loop = cpu1; loop <= cpu2; loop++)
		cpumask_set_cpu(loop, shared ? want_shared : want_unshared);
	if (*s == ',') {
		s++;
		goto keep_going;
	} else if ((*s == '\0') || isspace(*s))
		return 0;
err:
	pr_crit("Malformed %s argument: %s, offset: %lu\n", argname, str,
		(unsigned long)s - (unsigned long)str);
	return -EINVAL;
}

#endif /* DPA_SYS_H */

