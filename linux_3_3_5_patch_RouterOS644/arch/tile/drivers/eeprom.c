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
 * I2C EEPROM driver
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
#include <linux/mutex.h>
#include <linux/hugetlb.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <hv/hypervisor.h>
#include <hv/drv_eeprom_intf.h>

struct eeprom_dev {
	struct cdev cdev;		/* Character device structure */
	int hv_devhdl;			/* Handle for hypervisor device */
	u32 size;			/* Size of this device */
	struct mutex lock;		/* Serialize the chip access */
};

static int eeprom_major;		/* Dynamic major by default */
static int eeprom_devs =    I2C_NUM_EEPROMS; /* Max number of eeprom devices */

module_param(eeprom_major, int, 0);
module_param(eeprom_devs, int, 0);
MODULE_AUTHOR("Tilera Corporation");
MODULE_LICENSE("GPL");

static struct eeprom_dev *eeprom_devices; /* allocated in eeprom_init */


/**
 * eeprom_open() - Device open routine.
 * @inode: Inode for this device.
 * @filp: File for this specific open of the device.
 *
 * Returns zero, or an error code.
 */
static int eeprom_open(struct inode *inode, struct file *filp)
{
	struct eeprom_dev *dev;

	/* Find the device */
	dev = container_of(inode->i_cdev, struct eeprom_dev, cdev);

	mutex_lock(&dev->lock);

	/* Now open the hypervisor device if we haven't already. */
	if (dev->hv_devhdl < 0) {
		char buf[20];
		int instance = iminor(inode);

		sprintf(buf, "eeprom/%d", instance);
		dev->hv_devhdl = hv_dev_open((HV_VirtAddr)buf, 0);
		if (dev->hv_devhdl >= 0) {
			hv_dev_pread(dev->hv_devhdl, 0,
				     (HV_VirtAddr)&dev->size,
				     sizeof(dev->size),
				     EEPROM_DEVICE_SIZE_OFF);
		}
	}

	mutex_unlock(&dev->lock);

	/* If we tried and failed to open it, fail. */
	if (dev->hv_devhdl < 0) {
		switch (dev->hv_devhdl)	{
		case HV_ENODEV:
			return -ENODEV;
		default:
			return (ssize_t)dev->hv_devhdl;
		}
	}

	filp->private_data = dev;

	return 0;          /* success */
}


/**
 * eeprom_release() - Device release routine.
 * @inode: Inode for this device.
 * @filp: File for this specific open of the device.
 *
 * Returns zero.
 */
static int eeprom_release(struct inode *inode, struct file *filp)
{
	filp->private_data = NULL;

	return 0;
}


/**
 * eeprom_read() - Read data from the device.
 * @filp: File for this specific open of the device.
 * @buf: User's data buffer.
 * @count: Number of bytes requested.
 * @f_pos: File position.
 *
 * Returns number of bytes read, or an error code.
 */
static ssize_t eeprom_read(struct file *filp, char __user *buf,
			   size_t count, loff_t *f_pos)
{
	int retval = 0;
	void *kernbuf;
	struct eeprom_dev *dev = filp->private_data;

	if (dev->hv_devhdl < 0)
		return -EINVAL;

	if (unlikely(!count))
		return count;

	kernbuf = kmalloc(EEPROM_CHUNK_SIZE, GFP_KERNEL);
	if (!kernbuf)
		return -ENOMEM;

	mutex_lock(&dev->lock);

	while (count) {
		int hv_retval;
		int bytes_this_pass = count;
		if (bytes_this_pass > EEPROM_CHUNK_SIZE)
			bytes_this_pass = EEPROM_CHUNK_SIZE;

		hv_retval = hv_dev_pread(dev->hv_devhdl, 0,
					 (HV_VirtAddr) kernbuf,
					 bytes_this_pass, *f_pos);

		if (hv_retval > 0) {
			if (copy_to_user(buf, kernbuf, hv_retval) != 0) {
				retval = -EFAULT;
				break;
			}
		} else if (hv_retval == 0) {
			break;
		} else {
			pr_err("eeprom: hv_dev_pread failed, "
			       "error %d\n", hv_retval);
			retval = -EIO;
			break;
		}

		retval += hv_retval;
		*f_pos += hv_retval;
		buf += hv_retval;
		count -= hv_retval;
	}

	mutex_unlock(&dev->lock);

	kfree(kernbuf);

	return retval;
}

/**
 * eeprom_write() - Write data to the device.
 * @filp: File for this specific open of the device.
 * @buf: User's data buffer.
 * @count: Number of bytes requested.
 * @f_pos: File position.
 *
 * Returns number of bytes written, or an error code.
 */
static ssize_t eeprom_write(struct file *filp, const char __user *buf,
			    size_t count, loff_t *f_pos)
{
	int retval = 0;
	void *kernbuf;
	struct eeprom_dev *dev = filp->private_data;

	if (dev->hv_devhdl < 0)
		return -EINVAL;

	if (unlikely(!count))
		return count;

	kernbuf = kmalloc(EEPROM_CHUNK_SIZE, GFP_KERNEL);
	if (!kernbuf)
		return -ENOMEM;

	mutex_lock(&dev->lock);

	while (count) {
		int hv_retval;
		int bytes_this_pass = count;
		if (bytes_this_pass > EEPROM_CHUNK_SIZE)
			bytes_this_pass = EEPROM_CHUNK_SIZE;

		if (copy_from_user(kernbuf, buf, bytes_this_pass) != 0) {
			retval = -EFAULT;
			break;
		}

		hv_retval = hv_dev_pwrite(dev->hv_devhdl, 0,
					  (HV_VirtAddr) kernbuf,
					  bytes_this_pass, *f_pos);
		if (hv_retval < 0) {
			if (hv_retval == HV_EPERM) {
				retval = -EPERM;
			} else {
				pr_err("eeprom: hv_dev_pwrite failed,"
					" error %d\n", hv_retval);
				retval = -EIO;
			}
			break;
		} else if (hv_retval == 0) {
			break;
		}

		retval += hv_retval;
		*f_pos += hv_retval;
		buf += hv_retval;
		count -= hv_retval;
	}

	mutex_unlock(&dev->lock);

	kfree(kernbuf);

	return retval;
}

/**
 * eeprom_llseek() - Change the current device offset.
 * @filp: File for this specific open of the device.
 * @off: New offset value.
 * @whence: Base for new offset value.
 *
 * Returns new offset, or an error code.
 */
static loff_t eeprom_llseek(struct file *filp, loff_t off, int whence)
{
	struct eeprom_dev *dev = filp->private_data;
	long newpos;

	switch (whence) {
	case 0: /* SEEK_SET */
		newpos = off;
		break;

	case 1: /* SEEK_CUR */
		newpos = filp->f_pos + off;
		break;

	case 2: /* SEEK_END */
		newpos = dev->size + off;
		break;

	default: /* can't happen */
		return -EINVAL;
	}

	if (newpos < 0 || newpos > dev->size)
		return -EINVAL;

	filp->f_pos = newpos;
	return newpos;
}

/*
 * The fops
 */
static const struct file_operations eeprom_fops = {
	.owner =     THIS_MODULE,
	.llseek =    eeprom_llseek,
	.read =	     eeprom_read,
	.write =     eeprom_write,
	.open =	     eeprom_open,
	.release =   eeprom_release,
};

/**
 * eeprom_setup_cdev() - Set up a device instance in the cdev table.
 * @dev: Per-device EEPROM state.
 * @index: Device to set up.
 */
static void eeprom_setup_cdev(struct eeprom_dev *dev, int index)
{
	int err, devno = MKDEV(eeprom_major, index);

	mutex_init(&dev->lock);
	cdev_init(&dev->cdev, &eeprom_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &eeprom_fops;
	err = cdev_add(&dev->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		pr_notice("Error %d adding eeprom%d\n", err, index);
}

/** eeprom_init() - Initialize the driver's module. */
static int eeprom_init(void)
{
	int result, i;
	dev_t dev = MKDEV(eeprom_major, 0);

	/*
	 * Register our major, and accept a dynamic number.
	 */
	if (eeprom_major)
		result = register_chrdev_region(dev, eeprom_devs, "eeprom");
	else {
		result = alloc_chrdev_region(&dev, 0, eeprom_devs, "eeprom");
		eeprom_major = MAJOR(dev);
	}
	if (result < 0)
		return result;

	/*
	 * Allocate the devices -- we can't have them static, as the number
	 * can be specified at load time.
	 */
	eeprom_devices = kzalloc(eeprom_devs * sizeof(struct eeprom_dev),
			       GFP_KERNEL);
	if (!eeprom_devices) {
		unregister_chrdev_region(dev, eeprom_devs);
		return -ENOMEM;
	}
	for (i = 0; i < eeprom_devs; i++) {
		eeprom_devices[i].hv_devhdl = -1;
		eeprom_setup_cdev(eeprom_devices + i, i);
	}

	return 0; /* succeed */
}

/** eeprom_cleanup() - Clean up the driver's module. */
static void eeprom_cleanup(void)
{
	int i;

	for (i = 0; i < eeprom_devs; i++)
		cdev_del(&eeprom_devices[i].cdev);
	kfree(eeprom_devices);
	unregister_chrdev_region(MKDEV(eeprom_major, 0), eeprom_devs);
}

module_init(eeprom_init);
module_exit(eeprom_cleanup);
