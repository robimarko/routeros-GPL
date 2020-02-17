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
 * Driver to allow memory sharing with BME tiles.
 *
 * This source code is derived from code provided in "Linux Device
 * Drivers, Third Edition", by Jonathan Corbet, Alessandro Rubini, and
 * Greg Kroah-Hartman, published by O'Reilly Media, Inc.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/pagemap.h>
#include <linux/init.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/fs.h>		/* everything... */
#include <linux/gfp.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/hugetlb.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <asm/page.h>
#include <asm/bme.h>
#include <hv/hypervisor.h>

struct bme_locked_pages {
	struct page *page;
};

struct bme_locked_pages_list {
	struct list_head list;
	int npages;
	struct bme_locked_pages *pageblock;
};

struct bme_mem_dev {
	struct list_head plist;
	struct rw_semaphore lock;
};

MODULE_AUTHOR("Tilera Corporation");
MODULE_LICENSE("GPL");

#ifdef BME_MEM_DEBUG
#define BME_MEM_TRACE(...) pr_info("bme_mem:" __VA_ARGS__)
#else
#define BME_MEM_TRACE(...)
#endif


static struct bme_mem_dev bme_mem_dev;

/* Figure out how many pages of memory underlie this user address range. */
static int get_num_pages(struct bme_user_mem_desc *range)
{
	struct mm_struct *mm = current->mm;
	unsigned long addr, end_addr;
	int count = 0;

	down_read(&mm->mmap_sem);

	addr = (unsigned long)range->va;
	end_addr = (unsigned long)(range->va + range->len);

	while (addr < end_addr) {
		unsigned long page_size;
		struct vm_area_struct *vma = find_vma(mm, (unsigned long)addr);
		if (!vma) {
			count = 0;
			break;
		}
		page_size = vma_kernel_pagesize(vma);

		/* advance to next page */
		addr = (addr & ~(page_size - 1)) + page_size;
		count++;
	}

	up_read(&mm->mmap_sem);

	return count;
}


/* Helper function to release all locked pages and free storage. */
static void bme_release_pages(struct bme_mem_dev *dev)
{
	struct bme_locked_pages_list *pglist;
	struct list_head *ptr, *ptrnext;

	BME_MEM_TRACE("Releasing pages\n");

	down_read(&dev->lock);

	/* Release all pages, free storage. */
	list_for_each_safe(ptr, ptrnext, &dev->plist) {
		int i;

		pglist = list_entry(ptr, struct bme_locked_pages_list, list);
		BME_MEM_TRACE("Releasing %d pages in list elem 0x%lx\n",
			      pglist->npages, (unsigned long)pglist);
		for (i = 0; i < pglist->npages; i++) {
			if (pglist->pageblock[i].page)
				page_cache_release(pglist->pageblock[i].page);
		}

		/* Free storage. */
		list_del(&pglist->list);
		kfree(pglist);
	}

	up_read(&dev->lock);
}


/*
 * Lock range of user memory specified by user portion of mem_desc,
 * return info regarding underlying real physical memory in phys portion
 * of mem_desc.
 */
static int lock_user_mem(struct bme_mem_dev *dev,
			 struct bme_phys_mem_desc_io *mem_desc)
{
	struct bme_user_mem_desc *user_range;
	struct bme_locked_pages_list *pglist;
	struct bme_locked_pages *pageblock;
	struct mm_struct *mm = current->mm;
	unsigned long addr, end_addr;
	int npages, index, retval;

	if (!mem_desc)
		return -EINVAL;

	if (!access_ok(VERIFY_WRITE, mem_desc->user.va,
		       mem_desc->num_pages * sizeof(struct bme_phys_mem_desc)))
		return -EINVAL;

	user_range = &mem_desc->user;

	/* Find out how many pages this memory covers */
	npages = get_num_pages(user_range);

	if (npages <= 0) {
		BME_MEM_TRACE("No pages relate to this address\n");
		return -EINVAL;
	}

	/*
	 * Check that user has allocated room for return of that
	 * many page results.
	 */
	if (npages > mem_desc->num_pages) {
		BME_MEM_TRACE("Not enough space for return values, "
			      "not locking pages\n");
		return -EINVAL;
	}

	/* Allocate list element and space for record of pages */
	pglist = kzalloc(sizeof(struct bme_locked_pages_list) +
			 (sizeof(struct bme_locked_pages) * npages),
			 GFP_KERNEL);
	if (!pglist) {
		BME_MEM_TRACE("Couldn't allocate page list\n");
		return -ENOMEM;
	}

	/* Set pointer to storage for driver record of pages */
	pageblock = (struct bme_locked_pages *)
		((unsigned long)pglist + sizeof(struct bme_locked_pages_list));

	pglist->npages = npages;
	pglist->pageblock = pageblock;

	/* Loop to lock down pages, get physical info */
	addr = (unsigned long)user_range->va;
	end_addr = (unsigned long)(user_range->va + user_range->len);
	index = 0;
	retval = 0;

	/*
	 * We use get_user_pages to grab and lock the pages, so we need
	 * this semaphore.
	 */
	down_read(&mm->mmap_sem);

	while ((addr < end_addr) && (retval >= 0)) {
		struct vm_area_struct *vma;

		retval = get_user_pages(current, mm, addr,
					1, 1, 0, &pageblock[index].page,
					&vma);

		if (retval != 1) {
			BME_MEM_TRACE("Couldn't get page\n");
			retval = -EFAULT;
		} else {
			/* Figure out what size page we're on. */
			int page_size = vma_kernel_pagesize(vma);
			HV_PhysAddr cpfn;
			s64 pa;
			/*
			 * Calculate the length of the specified user memory
			 * on this page (may end before page does).
			 */
			unsigned long next_page_addr = (addr & ~(page_size - 1))
				+ page_size;
			int len_rest_of_range = end_addr - addr;
			int len_rest_of_page = next_page_addr - addr;
			int len_rest = len_rest_of_range < len_rest_of_page ?
				len_rest_of_range : len_rest_of_page;

			BME_MEM_TRACE("addr = 0x%lx next_page_addr = 0x%lx "
				      "len_rest = 0x%x\n",
				      addr, next_page_addr, len_rest);

			/*
			 * Now get realpa and make sure that we have valid
			 * memory underlying this.  It would be a horrible
			 * error if we didn't.
			 */

			cpfn = page_to_pfn(pageblock[index].page);
			pa = hv_inquire_realpa(cpfn << PAGE_SHIFT, len_rest);
			if (pa < 0) {
				BME_MEM_TRACE("Not valid real memory\n");
				retval = -EFAULT;
			} else {
				struct bme_phys_mem_desc *phys = 
					(struct bme_phys_mem_desc *)(uintptr_t)
					mem_desc->phys;

				/* Fill in pte, pa, and length info. */
				pte_t *ptep = virt_to_pte(mm, addr);
				phys[index].pte = ptep->val;
				phys[index].pa = pa;
				phys[index].len = len_rest;

				/* advance to next page */
				addr = next_page_addr;
				index++;
			}
		}

		/*
		 * If the last page failed, we need to back out everything
		 * we've done so far, and return an error.
		 */
		if (retval < 0) {
			int j;
			for (j = 0; j < index; j++) {
				if (pageblock[j].page)
					page_cache_release(pageblock[j].page);
			}

			up_read(&mm->mmap_sem);

			/* Free storage */
			kfree(pglist);

			return retval;
		}
	}

	/* Success.  Add the pageblock to the list of locked pages. */
	down_read(&dev->lock);
	list_add_tail(&pglist->list, &dev->plist);
	up_read(&dev->lock);

	up_read(&mm->mmap_sem);

	return 0;
}


/**
 * bme_mem_ioctl() - Device ioctl routine.
 * @filp: File for this specific open of the device.
 * @cmd: ioctl command.
 * @arg: Pointer to user argument.
 *
 * Returns zero, or an error code.
 */
static long bme_mem_ioctl(struct file *filp,
			  unsigned int cmd, unsigned long arg)
{
	struct bme_user_mem_desc_io user_mem_desc;
	struct bme_phys_mem_desc_io phys_mem_desc;

	switch (cmd) {
	case BME_IOC_GET_NUM_PAGES:
		if (copy_from_user(&user_mem_desc, (void __user *)arg,
				   sizeof(struct bme_user_mem_desc_io)))
			return -EFAULT;
		user_mem_desc.num_pages = get_num_pages(&user_mem_desc.user);
		if (copy_to_user((void __user *)arg, &user_mem_desc,
				 sizeof(struct bme_user_mem_desc_io)))
			return -EFAULT;
		break;

	case BME_IOC_LOCK_MEMORY:
		if (copy_from_user(&phys_mem_desc, (void __user *)arg,
				   sizeof(struct bme_phys_mem_desc_io)))
			return -EFAULT;

		phys_mem_desc.results_are_valid =
			lock_user_mem(&bme_mem_dev, &phys_mem_desc);
		if (copy_to_user((void __user *)arg, &phys_mem_desc,
				 sizeof(struct bme_phys_mem_desc_io)))
			return -EFAULT;
		break;
	default:
		BME_MEM_TRACE("BME: Unknown IOCTL\n");
		return -EINVAL;
	}

	return 0;
}

/**
 * bme_mem_release() - Device release routine.
 * @inode: Inode for this device.
 * @filp: File for this specific open of the device.
 *
 * Returns zero on success, or a negative error code.
 */
static int bme_mem_release(struct inode *inode, struct file *filp)
{
	bme_release_pages(&bme_mem_dev);

	return 0;
}

/*
 * The fops
 */
static const struct file_operations bme_mem_fops = {
	.owner          = THIS_MODULE,
	.open           = nonseekable_open,
	.llseek         = no_llseek,
	.release        = bme_mem_release,
	.unlocked_ioctl = bme_mem_ioctl,
};

static struct miscdevice bme_device = {
	MISC_DYNAMIC_MINOR,
	"bme_mem",
	&bme_mem_fops
};

/** bme_mem_init() - Initialize the driver's module. */
static int bme_mem_init(void)
{
	int result;

	/*
	 * Register as a misc device, and accept a dynamic minor number.
	 */
	result = misc_register(&bme_device);
	if (result < 0)
		return result;

	/* Initialize list. */
	INIT_LIST_HEAD(&bme_mem_dev.plist);

	/* Initialize lock. */
	init_rwsem(&bme_mem_dev.lock);

	return 0;
}

/** bme_mem_cleanup() - Cleanup function for the BME memory driver. */
static void bme_mem_cleanup(void)
{
	misc_deregister(&bme_device);
}

module_init(bme_mem_init);
module_exit(bme_mem_cleanup);
