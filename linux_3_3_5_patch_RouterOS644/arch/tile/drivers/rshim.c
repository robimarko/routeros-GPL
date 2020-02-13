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
 * RShim driver
 *
 * This source code is derived from code provided in "Linux Device
 * Drivers, Third Edition", by Jonathan Corbet, Alessandro Rubini, and
 * Greg Kroah-Hartman, published by O'Reilly Media, Inc.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/aio.h>
#include <linux/pagemap.h>
#include <linux/hugetlb.h>
#include <linux/uaccess.h>
#include <hv/hypervisor.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <hv/drv_rshim_intf.h>

struct rshim_dev {
	struct cdev cdev;		/* Character device structure */
	int hv_devhdl;			/* Handle for hypervisor device */
};

static int rshim_major;		/* Dynamic major by default */
static int rshim_devs =    1;	/* One for each rshim partition */

module_param(rshim_major, int, 0);
module_param(rshim_devs, int, 0);
MODULE_AUTHOR("Tilera Corporation");
MODULE_LICENSE("GPL");

static struct rshim_dev *rshim_devices; /* allocated in rshim_init */


/**
 * rshim_open() - Device open routine.
 * @inode: Inode for this device.
 * @filp: File for this specific open of the device.
 *
 * Returns zero, or an error code.
 */
static int rshim_open(struct inode *inode, struct file *filp)
{
	struct rshim_dev *dev;
	char *filename = "rshim/0";

	/* Find the device */
	dev = container_of(inode->i_cdev, struct rshim_dev, cdev);

	/* Open the hypervisor device. */
	dev->hv_devhdl = hv_dev_open((HV_VirtAddr)filename, 0);
	if (dev->hv_devhdl < 0) {
		pr_err("rshim: Couldn't open %s.\n", filename);
		return -ENODEV;
	}

	filp->private_data = dev;

	return 0;          /* success */
}



/**
 * rshim_read() - Read (control) data from the device.
 * @filp: File for this specific open of the device.
 * @buf: User's data buffer.
 * @count: Number of bytes requested.
 * @f_pos: File position.
 *
 * Returns number of bytes read, or an error code.
 */
static ssize_t rshim_read(struct file *filp, char __user *buf,
			  size_t count, loff_t *f_pos)
{
	u32 kernbuf;
	struct rshim_dev *dev = filp->private_data;

	if (dev->hv_devhdl < 0)
		return -EINVAL;

	if (count == sizeof(kernbuf)) {
		int hv_retval;

		hv_retval = hv_dev_pread(dev->hv_devhdl, 0,
					 (HV_VirtAddr)&kernbuf,
					 count, *f_pos);

		if (hv_retval > 0) {
			if (copy_to_user(buf, &kernbuf, hv_retval) != 0)
				return -EFAULT;
		} else {
			pr_err("rshim: hv_dev_pread failed, "
			       "error %d\n", hv_retval);
			return -EIO;
		}

		return hv_retval;
	} else {
		return -EINVAL;
	}
}

/**
 * rshim_write() - Write (control) data to the device.
 * @filp: File for this specific open of the device.
 * @buf: User's data buffer.
 * @count: Number of bytes requested.
 * @f_pos: File position.
 *
 * Returns number of bytes written, or an error code.
 */
static ssize_t rshim_write(struct file *filp, const char __user *buf,
			   size_t count, loff_t *f_pos)
{
	struct rshim_dev *dev = filp->private_data;
	char kernbuf[sizeof(rshim_down_counter_config_t)];

	if (dev->hv_devhdl < 0)
		return -EINVAL;

	if (count == sizeof(rshim_down_counter_config_t)) {
		int hv_retval;

		if (copy_from_user(kernbuf, buf, count) != 0)
			return -EFAULT;

		hv_retval = hv_dev_pwrite(dev->hv_devhdl, 0,
					  (HV_VirtAddr) kernbuf,
					  count, *f_pos);
		if (hv_retval < 0)
			return -EIO;

		return hv_retval;
	}

	return -EINVAL;
}



/*
 * The fops
 */
static const struct file_operations rshim_fops = {
	.owner =     THIS_MODULE,
	.read =	     rshim_read,
	.write =     rshim_write,
	.open =	     rshim_open,
};

/**
 * rshim_setup_cdev() - Set up a device instance in the cdev table.
 * @dev: Per-device RSHIM state.
 * @index: Device to set up.
 */
static void rshim_setup_cdev(struct rshim_dev *dev, int index)
{
	int err, devno = MKDEV(rshim_major, index);

	cdev_init(&dev->cdev, &rshim_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &rshim_fops;
	err = cdev_add(&dev->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		pr_notice("Error %d adding rshim%d", err, index);
}

/** rshim_init() - Initialize the driver's module. */
static int rshim_init(void)
{
	int result, i;
	dev_t dev = MKDEV(rshim_major, 0);

	/*
	 * Register our major, and accept a dynamic number.
	 */
	if (rshim_major)
		result = register_chrdev_region(dev, rshim_devs, "rshim");
	else {
		result = alloc_chrdev_region(&dev, 0, rshim_devs, "rshim");
		rshim_major = MAJOR(dev);
	}
	if (result < 0)
		return result;


	/*
	 * Allocate the devices -- we can't have them static, as the number
	 * can be specified at load time.
	 */
	rshim_devices = kzalloc(rshim_devs * sizeof(struct rshim_dev),
				GFP_KERNEL);
	if (!rshim_devices) {
		unregister_chrdev_region(dev, rshim_devs);
		return -ENOMEM;
	}
	for (i = 0; i < rshim_devs; i++)
		rshim_setup_cdev(rshim_devices + i, i);

	return 0; /* succeed */
}

/** rshim_cleanup() - Clean up the driver's module. */
static void rshim_cleanup(void)
{
	int i;

	for (i = 0; i < rshim_devs; i++)
		cdev_del(&rshim_devices[i].cdev);
	kfree(rshim_devices);
	unregister_chrdev_region(MKDEV(rshim_major, 0), rshim_devs);
}

module_init(rshim_init);
module_exit(rshim_cleanup);
