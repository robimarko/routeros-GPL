/* Copyright 2011 Freescale Semiconductor, Inc.
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

#include "bman_private.h"
#include "qman_private.h"

static const char dpa_uio_version[] = "USDPAA UIO portal driver v0.2";

static LIST_HEAD(dpa_uio_list);

struct dpa_uio_info {
	const struct dpa_uio_vtable *vtable;
	const struct list_head *pcfg;
	atomic_t ref; /* exclusive, only one open() at a time */
	struct uio_info uio;
	struct platform_device *pdev;
	struct list_head node;
};

static int dpa_uio_open(struct uio_info *info, struct inode *inode)
{
	struct dpa_uio_info *i = container_of(info, struct dpa_uio_info, uio);
	int ret = 0;

	if (!atomic_dec_and_test(&i->ref)) {
		atomic_inc(&i->ref);
		return -EBUSY;
	}
	if (i->vtable->on_open) {
		ret = i->vtable->on_open(i->pcfg);
		if (ret)
			atomic_inc(&i->ref);
	}
	return ret;
}

static int dpa_uio_release(struct uio_info *info, struct inode *inode)
{
	struct dpa_uio_info *i = container_of(info, struct dpa_uio_info, uio);
	if (i->vtable->on_close)
		i->vtable->on_close(i->pcfg);
	atomic_inc(&i->ref);
	return 0;
}

static pgprot_t dpa_uio_pgprot(struct uio_info *info, unsigned int mem_idx,
				   pgprot_t prot)
{
	if (mem_idx == DPA_PORTAL_CE)
		/* It's the cache-enabled portal region. NB, we shouldn't use
		 * pgprot_cached() here because it includes _PAGE_COHERENT. The
		 * region is cachable but *not* coherent - stashing (if enabled)
		 * leads to "coherent-like" behaviour, otherwise the driver
		 * explicitly invalidates/prefetches. */
		return pgprot_cached_noncoherent(prot);
	/* Otherwise it's the cache-inhibited portal region */
	return pgprot_noncached(prot);
}

static irqreturn_t dpa_uio_irq_handler(int irq, struct uio_info *info)
{
	struct dpa_uio_info *i = container_of(info, struct dpa_uio_info, uio);
	i->vtable->on_interrupt(i->pcfg);
	return IRQ_HANDLED;
}

static int __init dpa_uio_portal_init(struct dpa_uio_info *info)
{
	int ret;

	/* Fill in qbman-specific fields of uio_info */
	ret = info->vtable->init_uio(info->pcfg, &info->uio);
	if (ret) {
		pr_err("dpa_uio_portal: qbman parameter setup failed\n");
		return -ENODEV;
	}

	/* Fill in common fields of uio_info */
	info->uio.version = dpa_uio_version;
	info->uio.handler = dpa_uio_irq_handler;
	info->uio.set_pgprot = dpa_uio_pgprot;
	info->uio.open = dpa_uio_open;
	info->uio.release = dpa_uio_release;

	/* Fill in state private to this file */
	atomic_set(&info->ref, 1);
	info->pdev = platform_device_alloc(info->uio.name, -1);
	if (!info->pdev) {
		info->vtable->destroy(info->pcfg, &info->uio);
		pr_err("dpa_uio_portal: platform_device_alloc() failed\n");
		return -ENOMEM;
	}
	ret = platform_device_add(info->pdev);
	if (ret) {
		platform_device_put(info->pdev);
		info->vtable->destroy(info->pcfg, &info->uio);
		pr_err("dpa_uio_portal: platform_device_add() failed\n");
		return -ENOMEM;
	}

	/* Register the device */
	ret = uio_register_device(&info->pdev->dev, &info->uio);
	if (ret) {
		platform_device_del(info->pdev);
		platform_device_put(info->pdev);
		info->vtable->destroy(info->pcfg, &info->uio);
		pr_err("dpa_uio_portal: UIO registration failed\n");
		return -EBUSY;
	}
	pr_info("USDPAA portal initialised, %s\n", info->uio.name);
	return 0;
}

static void __init dpa_uio_portal_finish(struct dpa_uio_info *info)
{
	info->vtable->destroy(info->pcfg, &info->uio);
	uio_unregister_device(&info->uio);
	platform_device_del(info->pdev);
	platform_device_put(info->pdev);
	pr_info("USDPAA portal removed, %s\n", info->uio.name);
}

static int __init dpa_uio_init(void)
{
	struct dpa_uio_info *info, *tmp;
	list_for_each_entry_safe(info, tmp, &dpa_uio_list, node) {
		int ret = dpa_uio_portal_init(info);
		if (ret) {
			list_del(&info->node);
			kfree(info);
		}
	}
	pr_info("USDPAA portal layer loaded\n");
	return 0;
}

static void __exit dpa_uio_exit(void)
{
	struct dpa_uio_info *info, *tmp;
	list_for_each_entry_safe(info, tmp, &dpa_uio_list, node) {
		dpa_uio_portal_finish(info);
		list_del(&info->node);
		kfree(info);
	}
	pr_info("USDPAA portal layer unloaded\n");
}

int __init dpa_uio_register(struct list_head *new_pcfg,
			    const struct dpa_uio_vtable *vtable)
{
	struct dpa_uio_info *info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;
	info->vtable = vtable;
	info->pcfg = new_pcfg;
	list_add_tail(&info->node, &dpa_uio_list);
	return 0;
}

module_init(dpa_uio_init)
module_exit(dpa_uio_exit)
MODULE_LICENSE("GPL");
