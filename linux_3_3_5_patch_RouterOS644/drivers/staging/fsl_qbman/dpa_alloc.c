/* Copyright 2009-2012 Freescale Semiconductor, Inc.
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

#include "dpa_sys.h"
#include <linux/fsl_qman.h>
#include <linux/fsl_bman.h>

/* Qman and Bman APIs are front-ends to the common code; */

static DECLARE_DPA_ALLOC(bpalloc); /* BPID allocator */
static DECLARE_DPA_ALLOC(fqalloc); /* FQID allocator */
static DECLARE_DPA_ALLOC(qpalloc); /* pool-channel allocator */
static DECLARE_DPA_ALLOC(cgralloc); /* CGR ID allocator */

/* This is a sort-of-conditional dpa_alloc_free() routine. Eg. when releasing
 * FQIDs (probably from user-space), it can filter out those that aren't in the
 * OOS state (better to leak a h/w resource than to crash). This function
 * returns the number of invalid IDs that were not released. */
static u32 release_id_range(struct dpa_alloc *alloc, u32 id, u32 count,
			     int (*is_valid)(u32 id))
{
	int valid_mode = 0;
	u32 loop = id, total_invalid = 0;
	while (loop < (id + count)) {
		int isvalid = is_valid ? is_valid(loop) : 1;
		if (!valid_mode) {
			/* We're looking for a valid ID to terminate an invalid
			 * range */
			if (isvalid) {
				/* We finished a range of invalid IDs, a valid
				 * range is now underway */
				valid_mode = 1;
				count -= (loop - id);
				id = loop;
			} else
				total_invalid++;
		} else {
			/* We're looking for an invalid ID to terminate a
			 * valid range */
			if (!isvalid) {
				/* Release the range of valid IDs, an unvalid
				 * range is now underway */
				if (loop > id)
					dpa_alloc_free(alloc, id, loop - id);
				valid_mode = 0;
			}
		}
		loop++;
	}
	/* Release any unterminated range of valid IDs */
	if (valid_mode && count)
		dpa_alloc_free(alloc, id, count);
	return total_invalid;
}

/* BPID allocator front-end */

int bman_alloc_bpid_range(u32 *result, u32 count, u32 align, int partial)
{
	return dpa_alloc_new(&bpalloc, result, count, align, partial);
}
EXPORT_SYMBOL(bman_alloc_bpid_range);

static int bp_valid(u32 bpid)
{
	struct bm_pool_state state;
	int ret = bman_query_pools(&state);
	BUG_ON(ret);
	if (bman_depletion_get(&state.as.state, bpid))
		/* "Available==1" means unavailable, go figure. Ie. it has no
		 * buffers, which is means it is valid for deallocation. (So
		 * true means false, which means true...) */
		return 1;
	return 0;
}
void bman_release_bpid_range(u32 bpid, u32 count)
{
	u32 total_invalid = release_id_range(&bpalloc, bpid, count, bp_valid);
	if (total_invalid)
		pr_err("BPID range [%d..%d] (%d) had %d leaks\n",
			bpid, bpid + count - 1, count, total_invalid);
}
EXPORT_SYMBOL(bman_release_bpid_range);

/* FQID allocator front-end */

int qman_alloc_fqid_range(u32 *result, u32 count, u32 align, int partial)
{
	return dpa_alloc_new(&fqalloc, result, count, align, partial);
}
EXPORT_SYMBOL(qman_alloc_fqid_range);

static int fq_valid(u32 fqid)
{
	struct qman_fq fq = {
		.fqid = fqid
	};
	struct qm_mcr_queryfq_np np;
	int err = qman_query_fq_np(&fq, &np);
	BUG_ON(err);
	return ((np.state & QM_MCR_NP_STATE_MASK) == QM_MCR_NP_STATE_OOS);
}
void qman_release_fqid_range(u32 fqid, u32 count)
{
	u32 total_invalid = release_id_range(&fqalloc, fqid, count, fq_valid);
	if (total_invalid)
		pr_err("FQID range [%d..%d] (%d) had %d leaks\n",
			fqid, fqid + count - 1, count, total_invalid);
}
EXPORT_SYMBOL(qman_release_fqid_range);

/* Pool-channel allocator front-end */

int qman_alloc_pool_range(u32 *result, u32 count, u32 align, int partial)
{
	return dpa_alloc_new(&qpalloc, result, count, align, partial);
}
EXPORT_SYMBOL(qman_alloc_pool_range);

static int qp_valid(u32 qp)
{
	/* TBD: when resource-management improves, we may be able to find
	 * something better than this. Currently we query all FQDs starting from
	 * FQID 1 until we get an "invalid FQID" error, looking for non-OOS FQDs
	 * whose destination channel is the pool-channel being released. */
	struct qman_fq fq = {
		.fqid = 1
	};
	int err;
	do {
		struct qm_mcr_queryfq_np np;
		err = qman_query_fq_np(&fq, &np);
		if (err)
			/* FQID range exceeded, found no problems */
			return 1;
		if ((np.state & QM_MCR_NP_STATE_MASK) != QM_MCR_NP_STATE_OOS) {
			struct qm_fqd fqd;
			err = qman_query_fq(&fq, &fqd);
			BUG_ON(err);
			if (fqd.dest.channel == qp)
				/* The channel is the FQ's target, can't free */
				return 0;
		}
		/* Move to the next FQID */
		fq.fqid++;
	} while (1);
}
void qman_release_pool_range(u32 qp, u32 count)
{
	u32 total_invalid = release_id_range(&qpalloc, qp, count, qp_valid);
	if (total_invalid) {
		/* Pool channels are almost always used individually */
		if (count == 1)
			pr_err("Pool channel 0x%x had %d leaks\n",
				qp, total_invalid);
		else
			pr_err("Pool channels [%d..%d] (%d) had %d leaks\n",
				qp, qp + count - 1, count, total_invalid);
	}
}
EXPORT_SYMBOL(qman_release_pool_range);

/* CGR ID allocator front-end */

int qman_alloc_cgrid_range(u32 *result, u32 count, u32 align, int partial)
{
	return dpa_alloc_new(&cgralloc, result, count, align, partial);
}
EXPORT_SYMBOL(qman_alloc_cgrid_range);

void qman_release_cgrid_range(u32 cgrid, u32 count)
{
	u32 total_invalid = release_id_range(&cgralloc, cgrid, count, NULL);
	if (total_invalid)
		pr_err("CGRID range [%d..%d] (%d) had %d leaks\n",
			cgrid, cgrid + count - 1, count, total_invalid);
}
EXPORT_SYMBOL(qman_release_cgrid_range);

/* Everything else is the common backend to all the allocators */

/* The allocator is a (possibly-empty) list of these; */
struct alloc_node {
	struct list_head list;
	u32 base;
	u32 num;
};

/* #define DPA_ALLOC_DEBUG */

#ifdef DPA_ALLOC_DEBUG
#define DPRINT pr_info
static void DUMP(struct dpa_alloc *alloc)
{
	int off = 0;
	char buf[256];
	struct alloc_node *p;
	list_for_each_entry(p, &alloc->list, list) {
		if (off < 255)
			off += snprintf(buf + off, 255-off, "{%d,%d}",
				p->base, p->base + p->num - 1);
	}
	pr_info("%s\n", buf);
}
#else
#define DPRINT(x...)	do { ; } while (0)
#define DUMP(a)		do { ; } while (0)
#endif

int dpa_alloc_new(struct dpa_alloc *alloc, u32 *result, u32 count, u32 align,
		  int partial)
{
	struct alloc_node *i = NULL, *next_best = NULL;
	u32 base, next_best_base = 0, num = 0, next_best_num = 0;
	struct alloc_node *margin_left, *margin_right;

	*result = (u32)-1;
	DPRINT("alloc_range(%d,%d,%d)\n", count, align, partial);
	DUMP(alloc);
	/* If 'align' is 0, it should behave as though it was 1 */
	if (!align)
		align = 1;
	margin_left = kmalloc(sizeof(*margin_left), GFP_KERNEL);
	if (!margin_left)
		goto err;
	margin_right = kmalloc(sizeof(*margin_right), GFP_KERNEL);
	if (!margin_right) {
		kfree(margin_left);
		goto err;
	}
	spin_lock_irq(&alloc->lock);
	list_for_each_entry(i, &alloc->list, list) {
		base = (i->base + align - 1) / align;
		base *= align;
		if ((base - i->base) >= i->num)
			/* alignment is impossible, regardless of count */
			continue;
		num = i->num - (base - i->base);
		if (num >= count) {
			/* this one will do nicely */
			num = count;
			goto done;
		}
		if (num > next_best_num) {
			next_best = i;
			next_best_base = base;
			next_best_num = num;
		}
	}
	if (partial && next_best) {
		i = next_best;
		base = next_best_base;
		num = next_best_num;
	} else
		i = NULL;
done:
	if (i) {
		if (base != i->base) {
			margin_left->base = i->base;
			margin_left->num = base - i->base;
			list_add_tail(&margin_left->list, &i->list);
		} else
			kfree(margin_left);
		if ((base + num) < (i->base + i->num)) {
			margin_right->base = base + num;
			margin_right->num = (i->base + i->num) -
						(base + num);
			list_add(&margin_right->list, &i->list);
		} else
			kfree(margin_right);
		list_del(&i->list);
		kfree(i);
		*result = base;
	}
	spin_unlock_irq(&alloc->lock);
err:
	DPRINT("returning %d\n", i ? num : -ENOMEM);
	DUMP(alloc);
	return i ? (int)num : -ENOMEM;
}

/* Allocate the list node using GFP_ATOMIC, because we *really* want to avoid
 * forcing error-handling on to users in the deallocation path. */
void dpa_alloc_free(struct dpa_alloc *alloc, u32 base_id, u32 count)
{
	struct alloc_node *i, *node = kmalloc(sizeof(*node), GFP_ATOMIC);
	BUG_ON(!node);
	DPRINT("release_range(%d,%d)\n", base_id, count);
	DUMP(alloc);
	BUG_ON(!count);
	spin_lock_irq(&alloc->lock);
	node->base = base_id;
	node->num = count;
	list_for_each_entry(i, &alloc->list, list) {
		if (i->base >= node->base) {
			/* BUG_ON(any overlapping) */
			BUG_ON(i->base < (node->base + node->num));
			list_add_tail(&node->list, &i->list);
			goto done;
		}
	}
	list_add_tail(&node->list, &alloc->list);
done:
	/* Merge to the left */
	i = list_entry(node->list.prev, struct alloc_node, list);
	if (node->list.prev != &alloc->list) {
		BUG_ON((i->base + i->num) > node->base);
		if ((i->base + i->num) == node->base) {
			node->base = i->base;
			node->num += i->num;
			list_del(&i->list);
			kfree(i);
		}
	}
	/* Merge to the right */
	i = list_entry(node->list.next, struct alloc_node, list);
	if (node->list.next != &alloc->list) {
		BUG_ON((node->base + node->num) > i->base);
		if ((node->base + node->num) == i->base) {
			node->num += i->num;
			list_del(&i->list);
			kfree(i);
		}
	}
	spin_unlock_irq(&alloc->lock);
	DUMP(alloc);
}

int dpa_alloc_reserve(struct dpa_alloc *alloc, u32 base, u32 num)
{
	struct alloc_node *i = NULL;
	struct alloc_node *margin_left, *margin_right;

	DPRINT("alloc_reserve(%d,%d)\n", base_id, count);
	DUMP(alloc);
	margin_left = kmalloc(sizeof(*margin_left), GFP_KERNEL);
	if (!margin_left)
		goto err;
	margin_right = kmalloc(sizeof(*margin_right), GFP_KERNEL);
	if (!margin_right) {
		kfree(margin_left);
		goto err;
	}
	spin_lock_irq(&alloc->lock);
	list_for_each_entry(i, &alloc->list, list)
		if ((i->base <= base) && ((i->base + i->num) >= (base + num)))
			/* yep, the reservation is within this node */
			goto done;
	i = NULL;
done:
	if (i) {
		if (base != i->base) {
			margin_left->base = i->base;
			margin_left->num = base - i->base;
			list_add_tail(&margin_left->list, &i->list);
		} else
			kfree(margin_left);
		if ((base + num) < (i->base + i->num)) {
			margin_right->base = base + num;
			margin_right->num = (i->base + i->num) -
						(base + num);
			list_add(&margin_right->list, &i->list);
		} else
			kfree(margin_right);
		list_del(&i->list);
		kfree(i);
	}
	spin_unlock_irq(&alloc->lock);
err:
	DPRINT("returning %d\n", i ? 0 : -ENOMEM);
	DUMP(alloc);
	return i ? 0 : -ENOMEM;
}

int dpa_alloc_pop(struct dpa_alloc *alloc, u32 *result, u32 *count)
{
	struct alloc_node *i = NULL;
	DPRINT("alloc_pop()\n");
	DUMP(alloc);
	spin_lock_irq(&alloc->lock);
	if (!list_empty(&alloc->list)) {
		i = list_entry(alloc->list.next, struct alloc_node, list);
		list_del(&i->list);
	}
	spin_unlock_irq(&alloc->lock);
	DPRINT("returning %d\n", i ? 0 : -ENOMEM);
	DUMP(alloc);
	if (!i)
		return -ENOMEM;
	*result = i->base;
	*count = i->num;
	kfree(i);
	return 0;
}
