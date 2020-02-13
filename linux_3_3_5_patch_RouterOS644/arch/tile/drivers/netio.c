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
 * The netio character device driver.
 *
 * This source code is derived from code provided in "Linux Device
 * Drivers, Third Edition", by Jonathan Corbet, Alessandro Rubini, and
 * Greg Kroah-Hartman, published by O'Reilly Media, Inc.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/aio.h>
#include <linux/pagemap.h>
#include <linux/hugetlb.h>
#include <linux/uaccess.h>
#include <linux/smp.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <asm/homecache.h>
#include <hv/hypervisor.h>
#include <hv/drv_xgbe_intf.h>
#include <hv/drv_pcie_channel_intf.h>
#include <hv/netio_errors.h>

/* xgbe0 and xgbe1, then gbe0 and gbe1, and then pcie0 and pcie1 */
#define NETIO_DEVS (4 + MAX_PCIE_LINKS_PER_CHIP)

#define NETIO_GBE_MINOR 0x2  /* This bit means we're a GbE device */

#define NETIO_PCIE_MINOR 0x4  /* This bit means we're a PCIe device */

/* Our PCI device structure. */
struct pcie_dev {
	int link_index;
	struct tlr_bios_info bios_info;
};

/*
 * Locking rules:
 *
 * ipp_lock protects us against multiple simultaneous incompatible
 * operations targeting the IPP, and against the IPP being started or
 * stopped while an operation is outstanding.  This lock must be held
 * for read while doing a hypervisor operation against the hypervisor
 * xgbe driver, and must be held for write while doing a non-parallel
 * operation against the netio driver (e.g., opening the device, starting
 * the IPP, stopping the IPP).  With regard to data, the lock must be
 * held for write when writing hv_devhdl; that value is only ever set
 * once so it need not be locked while reading.  pages serves as the
 * "IPP is initialized" flag, so ipp_lock must be held for read when
 * reading pages, and for write when writing pages.
 *
 * Lock ordering:
 *
 * We always acquire the locks used by the driver in this order:
 *
 * 1. mm->mmap_sem
 * 2. dev->ipp_lock
 * 3. xfp->file_lock
 *
 * This is slightly counterintuitive; it makes us hold mmap_sem throughout
 * all of netio_init_ipp(), even though we only really need to hold it while
 * we call get_user_pages().  However, netio_mmap(), which needs ipp_lock,
 * is already entered with mmap_sem held, so to maintain the lock ordering
 * (and prevent deadlock), we really have no choice.
 */

struct netio_dev {
	/* Character device structure */
	struct cdev cdev;

	/* See locking rules above */
	struct rw_semaphore ipp_lock;

	/* Handle for hypervisor device */
	int hv_devhdl;

	/* If nonzero, only control operations are allowed */
	u8 ctl_only:1;

	/* If nonzero, this is a PCIe interface */
	u8 is_pcie:1;

	/* The address of the fixed VA */
	u32 va_addr;

	/* The size of the fixed VA */
	u32 va_size;

	/* List of memory arenas */
	struct page **pages;

	/* Number of pages in pages[] */
	int npages;

	/* Number of active fd's for which did_register (not is_registered!)
	   is 1 */
	atomic_t n_did_register;

	/* If this is a PCIe packet interface, save its info here */
	struct pcie_dev pcie_dev;
};

/*
 * Entry in the per-file/per-tile list of I/O mem-registered pages.
 */
struct netio_iomem_page {
	void *va;			/* VA (NULL means no page in slot) */
	struct page *page;		/* Registered page pointer */
};

/*
 * The registration state is more complex than might initially seem needed,
 * so here's the rationale behind it.
 *
 * - We don't want to permit multiple registrations on the same fd/tile.
 *
 * - We want to keep track of which tiles registrations were done on, so
 *   that when the file is closed, we can make sure the unregistration is
 *   done on the same tile where the original registration happened.  Since
 *   the NetIO library binds you to a CPU before calling input_register,
 *   you'd think this would be easy, and that a registration and
 *   subsequent unregistration would always just happen on the same tile.
 *   Unfortunately, pthreaded programs don't close their fds until the
 *   very last thread exits, so in that case all of the releases happen
 *   on one tile.  To handle this we need to ask the original CPU to do
 *   the unregister, which means we have to remember which CPU that was.
 *   (As a bonus, this also handles the case where an application un-sets
 *   its processor affinity after registering.)
 *
 * - We want to stop the shim/IPP, and free up any associated memory
 *   resources, when applications aren't using it anymore.  However, we do
 *   have cases where applications register and unregister multiple times.
 *   We don't want to stop the IPP while an app is running, since that
 *   could lose state which it had set up.  So, we keep track of every
 *   fd which was _ever_ registered, even if it isn't at the moment,
 *   and only when all of those are closed do we perform the stop.
 */
struct netio_file {
	/* Device structure */
	struct netio_dev *dev;

	/* CPU on which fd did input_register */
	int registered_cpu;

	/* Nonzero if this fd ever did input_register */
	u8 did_register:1;

	/* Nonzero if this fd is currently registered */
	u8 is_registered:1;

	/* The address of the fixed VA */
	u32 fixed_addr;

	/* The size of the fixed VA */
	u32 fixed_size;

	/* List of I/O mem-registered pages */
	struct netio_iomem_page iomem_pages[1 << (32 - HPAGE_SHIFT)];
};

static int netio_major;			/* Dynamic major by default */
static int netio_devs = NETIO_DEVS;	/* Number of netio devices. */

module_param(netio_major, int, 0);
module_param(netio_devs, int, 0);

MODULE_AUTHOR("Tilera Corporation");
MODULE_LICENSE("GPL");


/* Allocated in netio_init(). */
static struct netio_dev *netio_devices;


/**
 * netio_open() - Device open routine.
 * @inode: Inode for this device.
 * @filp: File for this specific open of the device.
 *
 * Returns zero on success, or a negative error code.
 */
static int netio_open(struct inode *inode, struct file *filp)
{
	struct netio_dev *dev;
	struct netio_file *xfp;
	int instance = iminor(inode);

	/* Find the device. */
	dev = container_of(inode->i_cdev, struct netio_dev, cdev);

	/* NOTE: Indentation due to race condition removal. */
	{
		down_write(&dev->ipp_lock);

		/* Open the hypervisor device if we haven't already. */
		if (dev->hv_devhdl == 0) {

			char buf[32];

			if (instance & NETIO_PCIE_MINOR) {
				dev->is_pcie = 1;
				dev->pcie_dev.link_index =
					instance & ~NETIO_PCIE_MINOR;
				sprintf(buf, "pcie/%d/epp",
					dev->pcie_dev.link_index);
				dev->hv_devhdl =
					hv_dev_open((HV_VirtAddr)buf, 0);

				/*
				 * If the host isn't ready or the PCIe link
				 * isn't up yet, return EBUSY to let the app
				 * decide if it wants retry. Need to reset
				 * hv_devhdl so that the next open is possible.
				 */
				if (dev->hv_devhdl == HV_ENOTREADY) {
					dev->hv_devhdl = 0;
					up_write(&dev->ipp_lock);
					return (ssize_t)-EBUSY;
				}
			} else {
				/*
				 * First we try to open the device which
				 * gives us full NetIO functionality.
				 * If that doesn't work, we try the
				 * control-only device, and remember that
				 * we won't be able to do non-control
				 * operations.
				 */
				char *xgbechar =
					(instance & NETIO_GBE_MINOR) ? "" : "x";
				int baseinst = instance & ~NETIO_GBE_MINOR;
				sprintf(buf, "%sgbe/%d/netio", xgbechar,
					baseinst);
				dev->hv_devhdl =
					hv_dev_open((HV_VirtAddr)buf, 0);

				if (dev->hv_devhdl < 0) {
					dev->ctl_only = 1;
					sprintf(buf, "%sgbe/%d/ctl", xgbechar,
						baseinst);
					dev->hv_devhdl =
						hv_dev_open((HV_VirtAddr)buf,
							    0);
				}
			}
		}

		up_write(&dev->ipp_lock);
	}

	/* If we tried and failed to open it, fail. */
	if (dev->hv_devhdl < 0) {
		if (dev->hv_devhdl == HV_ENODEV)
			return (ssize_t)-ENODEV;
		else
			return (ssize_t)dev->hv_devhdl;
	}

	/*
	 * Create our per-file data structure, save a pointer to the
	 * device information in it, and then attach it to our file struct.
	 */
	xfp = kmalloc(sizeof(*xfp), GFP_KERNEL);
	memset(xfp, 0, sizeof(*xfp));
	xfp->dev = dev;
	filp->private_data = xfp;

	return 0;
}


/**
 * netio_hv_unregister() - Execute a netio unregistration request.
 * @arg: Pointer to per-file/tile state structure.
 *
 * The unregistration request is passed to the hypervisor.
 * This routine's parameters allow it to be called by smp_call_function().
 */
static void netio_hv_unregister(void *arg)
{
	struct netio_file *xfp = (struct netio_file *) arg;
	struct netio_dev *dev = xfp->dev;

	char *dummy;
	int i;

	/*
	 * First unregister any I/O memory.
	 */
	for (i = 0;
	     i < sizeof(xfp->iomem_pages) / sizeof(xfp->iomem_pages[0]); i++) {
		if (xfp->iomem_pages[i].va != NULL) {
			netio_ipp_address_t ia;

			ia.va = (HV_VirtAddr) xfp->iomem_pages[i].va;
			ia.size = HPAGE_SIZE;

			(void) hv_dev_pwrite(dev->hv_devhdl, 0,
					     (HV_VirtAddr)&ia, sizeof(ia),
					     NETIO_IPP_IOMEM_UNREGISTER_OFF);

			page_cache_release(xfp->iomem_pages[i].page);
			xfp->iomem_pages[i].va = NULL;
		}
	}

	/*
	 * Now do the IPP unregistration.
	 */
	hv_dev_pwrite(dev->hv_devhdl, 0, (HV_VirtAddr)&dummy,
		      sizeof(dummy), NETIO_IPP_INPUT_UNREGISTER_OFF);
}


/**
 * netio_do_unregister() - Process an unregistration request.
 * @xfp: Per-file NETIO state.
 * @dev: Per-device NETIO state.
 * @buf: Unregistration command from user.
 * @count: Number of bytes in unregistration command.
 *
 * Returns zero on success, or a negative error code.
 */
static int netio_do_unregister(struct netio_file *xfp, struct netio_dev *dev,
			       char *buf, size_t count)
{
	/* If we're currently registered, do the unregistration.  */
	if (xfp->is_registered) {
		/*
		 * We have to do the unregister on the tile that originally
		 * registered.  If that was us, fine; if not, we have to do
		 * a remote SMP function call to make it happen.
		 */
		smp_call_function_single(xfp->registered_cpu,
					 netio_hv_unregister, (void *) xfp, 1);

		xfp->is_registered = 0;
	}
	return 0;
}


/**
 * netio_final_unregister() - Handle device cleanup on close.
 * @xfp: Per-file NETIO state.
 * @dev: Per-device NETIO state.
 */
static void netio_final_unregister(struct netio_file *xfp, struct netio_dev *dev)
{
	/* If we never registered, nothing to do. */
	if (!xfp->did_register)
		return;

	/*
	 * Decrement the count of registered files; if zero, we shut down
	 * the IPP.
	 */
	xfp->did_register = 0;
	if (atomic_dec_and_test(&dev->n_did_register)) {
		char dummy;
		int retval;

		up_read(&dev->ipp_lock);
		down_write(&dev->ipp_lock);
		/*
		 * Since we didn't have the write lock when we checked the
		 * count, we could have gotten a new registration since then,
		 * so we need to check again.
		 */
		if (atomic_read(&dev->n_did_register)) {
			/* Yes, we did. */
			downgrade_write(&dev->ipp_lock);
			return;
		}

		/* Okay, we're really ready.  First shut down the IPP. */
		retval = hv_dev_pwrite(dev->hv_devhdl, 0,
				       (HV_VirtAddr)&dummy, sizeof(dummy),
				       NETIO_IPP_STOP_SHIM_OFF);

		if (retval >= 0) {
			/* It stopped; now free up the buffer pages. */

			if (dev->pages) {
				int i;
				for (i = 0; i < dev->npages; i++)
					page_cache_release(dev->pages[i]);
				kfree(dev->pages);
				dev->pages = NULL;
				dev->npages = 0;
				dev->va_addr = 0;
				dev->va_size = 0;
			}
		}

		downgrade_write(&dev->ipp_lock);
	}

	return;
}


/**
 * netio_release() - Device release routine.
 * @inode: Inode for this device.
 * @filp: File for this specific open of the device.
 *
 * Returns zero on success, or a negative error code.
 */
static int netio_release(struct inode *inode, struct file *filp)
{
	struct netio_file *xfp = filp->private_data;
	struct netio_dev *dev = xfp->dev;
	char dummy;

	down_read(&dev->ipp_lock);
	netio_do_unregister(xfp, dev, &dummy, 1);
	netio_final_unregister(xfp, dev);
	up_read(&dev->ipp_lock);

	kfree(xfp);
	filp->private_data = NULL;

	return 0;
}


/**
 * netio_init_ipp_inner() - Do the actual work of starting the IPP.
 * @dev: Per-device NETIO state.
 * @xfp: Per-file NETIO state.
 * @mm: mm_struct being used for the buffer pages.
 *
 * Returns zero on success, or a negative error code.
 */
static inline int netio_init_ipp_inner(struct netio_dev *dev,
				       struct netio_file *xfp,
				       struct mm_struct *mm)
{
	int i;
	int retval, error;
	char dummy;
	struct page **pages;
	netio_ipp_address_t *ipp_address;
	u32 va_addr, va_size, npages;

	/*
	 * This routine is entered holding mm->mmap_sem, and dev->ipp_lock
	 * (for write). Doing this work in two routines instead of one
	 * means that we don't have to worry about the locks in this
	 * routine, which makes error returns easier.
	 */

	/*
	 * If dev->pages is nonzero, someone else already started the IPP,
	 * so we have no work to do.  However, before we return, we need to
	 * make sure that the memory supplied by this caller matches what
	 * we used to configure the IPP; if it doesn't, the application may
	 * get network packet addresses which point at the wrong memory.
	 */
	if (dev->pages) {
		/* First verify that the VA/length matches. */

		if (xfp->fixed_addr != dev->va_addr ||
		    xfp->fixed_size != dev->va_size)
			return NETIO_FAULT;

		/* Now verify that the physical pages are the same. */

		npages = dev->va_size / HPAGE_SIZE;

		for (i = 0; i < npages; i++) {
			u32 addr = dev->va_addr + i * HPAGE_SIZE;
			struct page *page = NULL;

			retval = get_user_pages(current, mm, addr,
						1, 1, 0, &page, NULL);
			if (retval <= 0)
				return retval;

			page_cache_release(page);

			if (dev->pages[i] != page)
				return NETIO_FAULT;
		}

		return 0;
	}

	/* You must specify these values first. */
	if (xfp->fixed_addr == 0 || xfp->fixed_size == 0)
		return -EDQUOT;

	/* Set up the VA/PA mapping. */

	va_addr = xfp->fixed_addr;
	va_size = xfp->fixed_size;

	npages = va_size / HPAGE_SIZE;

	ipp_address = kzalloc(npages * sizeof(*ipp_address), GFP_KERNEL);
	if (!ipp_address)
		return -ENOMEM;

	pages = kzalloc(npages * sizeof(*dev->pages), GFP_KERNEL);
	if (!pages) {
		kfree(ipp_address);
		return -ENOMEM;
	}

	/* We use get_user_pages to grab and lock the pages. */

	for (i = 0; i < npages; i++) {
		u32 addr = va_addr + i * HPAGE_SIZE;
		retval = get_user_pages(current, mm, addr,
					1, 1, 0, &pages[i], NULL);
		if (retval <= 0) {
			int j;
			printk(KERN_DEBUG "netio: warning, "
			       "get_user_pages %d failed, code=%d\n",
			       i, retval);
			for (j = 0; j < i; j++)
				page_cache_release(pages[j]);
			kfree(ipp_address);
			kfree(pages);
			return retval;
		}
		ipp_address[i].size = HPAGE_SIZE;
		ipp_address[i].pa =
			((HV_PhysAddr)page_to_pfn(pages[i])) <<
			PAGE_SHIFT;
		ipp_address[i].va = addr;
		ipp_address[i].pte =
			pte_set_home(PAGE_KERNEL, page_home(pages[i]));

		printk(KERN_DEBUG "Passing page %d to IPP: va=0x%lx, "
		       "pa=0x%llx, size=0x%x\n", i,
		       (unsigned long)(ipp_address[i].va), ipp_address[i].pa,
		       ipp_address[i].size);
	}

	/* Commit. */
	dev->pages = pages;
	dev->npages = npages;
	dev->va_addr = va_addr;
	dev->va_size = va_size;

	/* OK, pass the memory to the shim. */
	error = hv_dev_pwrite(dev->hv_devhdl, 0,
			      (HV_VirtAddr)ipp_address,
			      npages * sizeof(*ipp_address),
			      NETIO_IPP_ADDRESS_OFF);
	if (error < 0)
		panic("netio: IPP couldn't take memory, code %d", error);

	kfree(ipp_address);

	/* Start the IPP. */
	error = hv_dev_pwrite(dev->hv_devhdl, 0,
			      (HV_VirtAddr)&dummy, sizeof(dummy),
			      NETIO_IPP_START_SHIM_OFF);
	if (error < 0)
		panic("netio: couldn't start IPP, code %d", error);

	return 0;
}


/**
 * netio_init_ipp() - Start the IPP.
 * @dev: Per-device NETIO state.
 * @xfp: Per-file NETIO state.
 *
 * This routine is basically a wrapper which handles all of the locking
 * rules and then calls netio_start_ipp_inner() to do the real work.
 * Returns zero on success, or a negative error code.
 */
static int netio_init_ipp(struct netio_dev *dev, struct netio_file *xfp)
{
	struct mm_struct *mm = current->mm;
	int retval;

	/*
	 * We're entered holding ipp_lock for read, but we need it for write,
	 * and there's no upgrade call for rwsems, so we have to drop it.
	 */
	up_read(&dev->ipp_lock);

	/*
	 * We're eventually going to need to get the mmap_sem to call
	 * get_user_pages().  We take it right here, before we retake
	 * ipp_lock, so that we maintain consistent lock order.  See
	 * the long "Locking rules" comment higher up in this file.
	 */
	down_read(&mm->mmap_sem);

	/*
	 * We dropped ipp_lock up above, so we need to get it back for write.
	 */
	down_write(&dev->ipp_lock);

	/* Now we call the routine which does all of the work. */
	retval = netio_init_ipp_inner(dev, xfp, mm);

	/*
	 * Now release the locks.  Note that we must return holding ipp_lock
	 * for read, so downgrade it instead of releasing it.
	 */
	downgrade_write(&dev->ipp_lock);
	up_read(&mm->mmap_sem);

	return retval;
}


/**
 * netio_iomem_register() - Process an I/O memory registration request.
 * @xfp: Per-file NETIO state.
 * @dev: Per-device NETIO state.
 * @buf: Command from user.
 * @count: Number of bytes in command.
 *
 * Note that this function must be called with mmap_sem held.
 *
 * Returns zero on success, or a negative error code.
 */
static int netio_iomem_register(struct netio_file *xfp, struct netio_dev *dev,
				char *buf, size_t count)
{
	netio_ipp_address_t *iap = (netio_ipp_address_t *) buf;
	int i;

	/*
	 * Sanity-check the request.
	 */
	if (count != sizeof(*iap))
		return -EINVAL;

	if (iap->va & (HPAGE_SIZE - 1) || iap->size & (HPAGE_SIZE - 1) ||
	    iap->va + iap->size <= iap->va)
		return NETIO_FAULT;

	/*
	 * Get and register our pages, one at a time.
	 */
	for (i = 0; i < iap->size >> HPAGE_SHIFT; i++) {
		int index = (iap->va >> HPAGE_SHIFT) + i;
		struct vm_area_struct *vma;
		int retval;

		/*
		 * First get the page.
		 */
		xfp->iomem_pages[index].page = NULL;
		retval = get_user_pages(current, current->mm,
					index << HPAGE_SHIFT, 1, 1, 0,
					&xfp->iomem_pages[index].page, &vma);

		if (retval != 1 || vma_kernel_pagesize(vma) != HPAGE_SIZE) {
			/* If we didn't get a default-size huge page, fail. */
			retval = NETIO_FAULT;
		} else {
			/* We got a page, see if we can register it. */
			netio_ipp_address_t reg_ia;
			struct page *page = xfp->iomem_pages[index].page;
			reg_ia.va = (HV_VirtAddr) (index << HPAGE_SHIFT);
			reg_ia.pa =
				((HV_PhysAddr)page_to_pfn(page)) << PAGE_SHIFT;
			reg_ia.pte =
				pte_set_home(PAGE_KERNEL, page_home(page));
			reg_ia.size = HPAGE_SIZE;

			retval = hv_dev_pwrite(dev->hv_devhdl, 0,
					       (HV_VirtAddr)&reg_ia,
					       sizeof(reg_ia),
					       NETIO_IPP_IOMEM_REGISTER_OFF);
		}

		/*
		 * If the last page failed, we need to back out everything
		 * we've done so far, and return an error.
		 */
		if (retval < 0) {
			struct page *page = xfp->iomem_pages[index].page;
			if (page)
				page_cache_release(page);
			if (i > 0) {
				/*
				 * Unregister and release everything we've
				 * successfully registered up to this point.
				 */
				netio_ipp_address_t unreg_ia;
				int j;

				unreg_ia.va = iap->va;
				unreg_ia.size = i << HPAGE_SHIFT;
				hv_dev_pwrite(dev->hv_devhdl, 0,
					      (HV_VirtAddr)&unreg_ia,
					      sizeof(unreg_ia),
					      NETIO_IPP_IOMEM_UNREGISTER_OFF);

				for (j = 0; j < i; j++) {
					int jindex =
						(iap->va >> HPAGE_SHIFT) + j;
					page_cache_release(
						xfp->iomem_pages[jindex].page);
					xfp->iomem_pages[jindex].va = NULL;
				}
			}

			return retval;
		}

		/*
		 * Remember that we've successfully gotten this page.
		 */
		xfp->iomem_pages[index].va =
			(void *)((long)index << HPAGE_SHIFT);
	}

	return count;
}


/**
 * netio_iomem_unregister() - Process an I/O memory unregistration request.
 * @xfp: Per-file NETIO state.
 * @dev: Per-device NETIO state.
 * @buf: Command from user.
 * @count: Number of bytes in command.
 *
 * Returns zero on success, or a negative error code.
 */
static int netio_iomem_unregister(struct netio_file *xfp, struct netio_dev *dev,
				  char *buf, size_t count)
{
	int retval;
	int i;

	netio_ipp_address_t *iap = (netio_ipp_address_t *) buf;
	if (count != sizeof(*iap))
		return -EINVAL;

	/*
	 * Do the unregistration.
	 */
	retval = hv_dev_pwrite(dev->hv_devhdl, 0, (HV_VirtAddr) buf,
			       count, NETIO_IPP_IOMEM_UNREGISTER_OFF);

	if (retval < 0)
		return retval;

	/*
	 * Since the unregistration was successful, unlock the pages.
	 */
	for (i = 0; i < iap->size >> HPAGE_SHIFT; i++) {
		int index = (iap->va >> HPAGE_SHIFT) + i;

		if (xfp->iomem_pages[index].va != NULL) {
			page_cache_release(xfp->iomem_pages[index].page);
			xfp->iomem_pages[index].va = NULL;
		}
	}

	return count;
}


/**
 * netio_do_register() - Process a registration request.
 * @xfp: Per-file NETIO state.
 * @dev: Per-device NETIO state.
 * @buf: Registration command from user.
 * @count: Number of bytes in registration command.
 *
 * Returns zero on success, or a negative error code.
 */
static int netio_do_register(struct netio_file *xfp, struct netio_dev *dev,
			     char *buf, size_t count)
{
	int retval;

	/*
	 * We don't allow more than one active registration per
	 * file at any one time, so that we don't get the reference
	 * count messed up.
	 */
	if (xfp->is_registered)
		return -EBUSY;

	/* Start IPP if needed. */
	if (!dev->is_pcie) {
		retval = netio_init_ipp(dev, xfp);
		if (retval)
			return retval;
	}

	/*
	 * Remember which tile we're on.  Note that preemption needs to be
	 * disabled until we finish the hypervisor call so that we know we
	 * weren't rescheduled between getting the CPU and doing the register.
	 */
	xfp->registered_cpu = get_cpu();

	/*
	 * Do the registration.  Note that we're called with ipp_lock held
	 * for read.
	 */
	retval = hv_dev_pwrite(dev->hv_devhdl, 0, (HV_VirtAddr) buf,
			       count, NETIO_IPP_INPUT_REGISTER_OFF);
	put_cpu();

	/*
	 * If the registration was okay, set flags and bump the registered
	 * count.
	 */
	if (retval >= 0) {
		xfp->is_registered = 1;
		if (!xfp->did_register) {
			xfp->did_register = 1;
			atomic_inc(&dev->n_did_register);
		}
	}

	return retval;
}


/**
 * netio_read() - Read (control) data from the device.
 * @filp: File for this specific open of the device.
 * @buf: User's data buffer.
 * @count: Number of bytes requested.
 * @f_pos: File position.
 *
 * Returns number of bytes read on success, or a negative error code.
 */
static ssize_t netio_read(struct file *filp, char __user *buf,
			  size_t count, loff_t *f_pos)
{
	int retval;
	void *kernbuf;
	struct netio_file *xfp = filp->private_data;
	struct netio_dev *dev = xfp->dev;

	if (*f_pos > NETIO_IPP_USER_MAX_OFF ||
	    *f_pos + count > NETIO_IPP_USER_MAX_OFF)
		return -EPERM;

	if (dev->hv_devhdl <= 0)
		return -EINVAL;

	kernbuf = kmalloc(count, GFP_KERNEL);
	if (!kernbuf)
		return -ENOMEM;

	down_read(&dev->ipp_lock);
	/*
	 * If the IPP isn't initialized yet, and this isn't a get/set
	 * operation, fail.
	 */
	if (!dev->is_pcie && !dev->pages &&
	    NETIO_BASE_OFFSET(*f_pos) != NETIO_IPP_PARAM_OFF) {
		up_read(&dev->ipp_lock);
		kfree(kernbuf);
		return -EBUSY;
	}

	retval = hv_dev_pread(dev->hv_devhdl, 0, (HV_VirtAddr) kernbuf, count,
			      *f_pos);
	up_read(&dev->ipp_lock);

	if (retval >= 0 && copy_to_user(buf, kernbuf, retval) != 0)
		retval = -EFAULT;

	kfree(kernbuf);

	return retval;
}


/**
 * netio_write() - Write (control) data to the device.
 * @filp: File for this specific open of the device.
 * @buf: User's data buffer.
 * @count: Number of bytes requested.
 * @f_pos: File position.
 *
 * Returns number of bytes written on success, or a negative error code.
 */
static ssize_t netio_write(struct file *filp, const char __user *buf,
			   size_t count, loff_t *f_pos)
{
	int retval;
	void *kernbuf;
	struct netio_file *xfp = filp->private_data;
	struct netio_dev *dev = xfp->dev;
	loff_t baseoff = NETIO_BASE_OFFSET(*f_pos);
	struct rw_semaphore *mmap_sem;

	if (dev->hv_devhdl <= 0)
		return -EINVAL;

	kernbuf = kmalloc(count, GFP_KERNEL);
	if (!kernbuf)
		return -ENOMEM;

	if (copy_from_user(kernbuf, buf, count) != 0) {
		kfree(kernbuf);
		return -EFAULT;
	}

	/*
	 * netio_iomem_register() uses get_user_pages(), so we need to
	 * take mmap_sem before ipp_lock to obey our locking rules.
	 */
	if (*f_pos == NETIO_IPP_IOMEM_REGISTER_OFF) {
		mmap_sem = &current->mm->mmap_sem;
		down_read(mmap_sem);
	} else {
		mmap_sem = NULL;
	}

	down_read(&dev->ipp_lock);

	/*
	 * If the IPP isn't initialized yet, and this isn't one of the
	 * operations which don't need a running IPP, then fail.
	 */
	if (!dev->is_pcie && !dev->pages &&
	    baseoff != NETIO_IPP_PARAM_OFF &&
	    baseoff != NETIO_IPP_INPUT_REGISTER_OFF &&
	    baseoff != NETIO_FIXED_ADDR &&
	    baseoff != NETIO_FIXED_SIZE) {
		up_read(&dev->ipp_lock);
		kfree(kernbuf);
		if (mmap_sem)
			up_read(mmap_sem);
		return -EBUSY;
	}

	/* TODO: Combine "NETIO_FIXED_ADDR" and "NETIO_FIXED_SIZE". */

	/*
	 * Registration and unregistration require special handling, to
	 * start or stop the IPP and maintain the reference count.
	 * We don't allow registration if we're in control-only mode.
	 */
	if (*f_pos == NETIO_IPP_INPUT_REGISTER_OFF) {
		if (dev->ctl_only)
			retval = -EPERM;
		else
			retval = netio_do_register(xfp, dev, kernbuf, count);
	} else if (*f_pos == NETIO_IPP_INPUT_UNREGISTER_OFF) {
		retval = netio_do_unregister(xfp, dev, kernbuf, count);
	} else if (*f_pos == NETIO_IPP_IOMEM_REGISTER_OFF) {
		retval = netio_iomem_register(xfp, dev, kernbuf, count);
	} else if (*f_pos == NETIO_IPP_IOMEM_UNREGISTER_OFF) {
		retval = netio_iomem_unregister(xfp, dev, kernbuf, count);
	} else if (*f_pos == NETIO_FIXED_ADDR) {
		if (count != sizeof(xfp->fixed_addr)) {
			retval = -EINVAL;
		} else {
			memcpy(&xfp->fixed_addr, kernbuf, count);
			retval = count;
		}
	} else if (*f_pos == NETIO_FIXED_SIZE) {
		if (count != sizeof(xfp->fixed_size)) {
			retval = -EINVAL;
		} else {
			memcpy(&xfp->fixed_size, kernbuf, count);
			retval = count;
		}
	} else if (*f_pos > NETIO_IPP_USER_MAX_OFF ||
		   *f_pos + count > NETIO_IPP_USER_MAX_OFF) {
		retval = -EPERM;
	} else {
		retval = hv_dev_pwrite(dev->hv_devhdl, 0,
				       (HV_VirtAddr) kernbuf,
				       count, *f_pos);
	}

	up_read(&dev->ipp_lock);

	if (mmap_sem)
		up_read(mmap_sem);

	kfree(kernbuf);

	return retval;
}


/**
 * netio_llseek() - Change the current device offset.
 * @filp: File for this specific open of the device.
 * @off: New offset value.
 * @whence: Base for new offset value.
 *
 * Returns new offset on success, or a negative error code.
 */
static loff_t netio_llseek(struct file *filp, loff_t off, int whence)
{
	long newpos;

	switch (whence) {
	case 0: /* SEEK_SET */
		newpos = off;
		break;

	case 1: /* SEEK_CUR */
		newpos = filp->f_pos + off;
		break;

	case 2: /* SEEK_END */
		/* We have no size, so this is the same as SEEK_SET. */
		newpos = off;
		break;

	default: /* can't happen */
		return -EINVAL;
	}
	if (newpos < 0)
		return -EINVAL;
	filp->f_pos = newpos;
	return newpos;
}


/**
 * netio_mmap() - Map the device into user address space.
 * @filp: File for this specific open of the device.
 * @vma: Virtual memory area.
 *
 * Returns va of the mapped area on success, or a negative error code.
 */
static int netio_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct netio_file *xfp = filp->private_data;

	/* PCIe device doesn't support mmap. */
	if (xfp->dev->is_pcie)
		return -ENODEV;

	/* Perform the mapping. */
	return hugetlbfs_file_operations.mmap(filp, vma);
}


/*
 * The fops.
 */
static const struct file_operations netio_fops = {
	.owner =     THIS_MODULE,
	.llseek =    netio_llseek,
	.read =	     netio_read,
	.write =     netio_write,
	.mmap =	     netio_mmap,
	.open =	     netio_open,
	.release =   netio_release,
	.get_unmapped_area = hugetlb_get_unmapped_area,
};


/**
 * netio_setup_cdev() - Set up a device instance in the cdev table.
 * @dev: Per-device NETIO state.
 * @index: Device to set up.
 */
static void netio_setup_cdev(struct netio_dev *dev, int index)
{
	int err, devno = MKDEV(netio_major, index);

	cdev_init(&dev->cdev, &netio_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &netio_fops;
	err = cdev_add(&dev->cdev, devno, 1);
	/* Fail gracefully if needed. */
	if (err)
		pr_notice("Error %d adding netio%d\n", err, index);
}


/**
 * netio_init() - Initialize the driver's module.
 *
 * Returns zero on success, or a negative error code.
 */
static int netio_init(void)
{
	int result, i;
	dev_t dev = MKDEV(netio_major, 0);

	/*
	 * Register your major (if non-zero), or allocate one (if zero).
	 */
	if (netio_major) {
		result = register_chrdev_region(dev, netio_devs, "netio");
	} else {
		result = alloc_chrdev_region(&dev, 0, netio_devs, "netio");
		/* ISSUE: Is this legal if "result < 0"? */
		netio_major = MAJOR(dev);
	}
	if (result < 0)
		return result;

	/*
	 * Allocate the devices.  We can't have them static, as the number
	 * can be specified at load time.
	 */
	netio_devices = kzalloc(netio_devs * sizeof(struct netio_dev),
				GFP_KERNEL);
	if (!netio_devices) {
		unregister_chrdev_region(dev, netio_devs);
		return -ENOMEM;
	}
	for (i = 0; i < netio_devs; i++) {
		atomic_set(&netio_devices[i].n_did_register, 0);
		init_rwsem(&netio_devices[i].ipp_lock);
		netio_setup_cdev(netio_devices + i, i);
	}

	return 0;
}


/** netio_cleanup() - Clean up the driver's module. */
static void netio_cleanup(void)
{
	int i;

	for (i = 0; i < netio_devs; i++) {
		kfree(netio_devices[i].pages);
		cdev_del(&netio_devices[i].cdev);
	}
	kfree(netio_devices);
	unregister_chrdev_region(MKDEV(netio_major, 0), netio_devs);
}


module_init(netio_init);
module_exit(netio_cleanup);
