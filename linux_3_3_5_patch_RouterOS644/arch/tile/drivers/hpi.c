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
 * HPI driver
 *
 * This source code is derived from code provided in "Linux Device
 * Drivers, Third Edition", by Jonathan Corbet, Alessandro Rubini, and
 * Greg Kroah-Hartman, published by O'Reilly Media, Inc.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/poll.h>
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <asm/hv_driver.h>
#include <hv/hypervisor.h>
#include <hv/drv_hpi_intf.h>

struct hpi_dev {
	struct cdev cdev;		/* Character device structure */
	int hv_devhdl;		        /* Handle for hypervisor device */
	char *info_string;		/* String returned by the info device
					   if this is it; NULL otherwise */
	struct hv_driver_cb driver_cb;  /* Driver callback for poll routine */
	unsigned int poll_mask;         /* Mask returned by poll routine */
	int is_memory_file;	        /* This is the memory file */
	wait_queue_head_t int_queue;    /* Queue for interrupts from host */
	int hv_poll_outstanding;        /* Hv has not delivered poll yet */
	struct mutex mutex;             /* Guarantee one accessor at a time */
	char *data_buffer;              /* Holds kernel's temp copy of data */
};

/* Dynamic major by default */
static int hpi_major;

/* One device for memory access, one for host interrupt, one for info */
static int hpi_devs =    3;

/* Minor number of the info file */
static inline int hpi_info_minor(void)
{
	return hpi_devs - 1;
}

static char driver_name[] = "hpi";

module_param(hpi_major, int, 0);
module_param(hpi_devs, int, 0);
MODULE_AUTHOR("Tilera Corporation");
MODULE_LICENSE("GPL");

#ifdef HPI_DEBUG
#define HPI_TRACE(...) pr_info("hpi:" __VA_ARGS__)
#else
#define HPI_TRACE(...)
#endif

static struct hpi_dev *hpi_devices;

/** irq_handler() - The IRQ handler routine. */
static void irq_handler(struct hv_driver_cb *cb, __hv32 result)
{
	struct hpi_dev *dev = (struct hpi_dev *)cb->dev;

	HPI_TRACE("INTERRUPT dev->hv_devhdl=0x%x result=%d(0x%x)\n",
		  dev->hv_devhdl, result, result);

	dev->hv_poll_outstanding = 1;

	if (result & HV_DEVPOLL_READ) {
		dev->poll_mask |= POLLIN | POLLRDNORM;
		wake_up_interruptible(&dev->int_queue);
	}
}

/**
 * hpi_init_device() - Device initialization routine.
 * @dev: Device to be initialized.
 * @instance: Minor inode for device.
 */
static int hpi_init_device(struct hpi_dev *dev, int instance)
{
	if (instance == 0) {
		dev->is_memory_file = 1;
		dev->data_buffer = kmalloc(HPI_DATA_BUFFER_SIZE,
					   GFP_KERNEL);
		if (!dev->data_buffer) {
			HPI_TRACE("Exit hpi_init_device -ENOMEM\n");
			return -ENOMEM;
		}
	} else {
		dev->is_memory_file = 0;
		dev->data_buffer = kmalloc(HPI_INT_BUFFER_SIZE,
					   GFP_KERNEL);
		if (!dev->data_buffer) {
			HPI_TRACE("Exit hpi_init_device -ENOMEM\n");
			return -ENOMEM;
		}
	}
	return 0;
}

/**
 * hpi_open() - Device open routine.
 * @inode: Inode for this device.
 * @filp: File for this specific open of the device.
 *
 * Returns zero, or an error code.
 */
static int hpi_open(struct inode *inode, struct file *filp)
{
	struct hpi_dev *dev;

	/* Find the device */
	dev = container_of(inode->i_cdev, struct hpi_dev, cdev);

	/* Now open the hypervisor device if we haven't already. */
	if (dev->hv_devhdl == 0) {
		char buf[20];
		int instance = iminor(inode);
		if (instance != hpi_info_minor()) {
			sprintf(buf, "hpi/0/%d", instance);
			dev->hv_devhdl = hv_dev_open((HV_VirtAddr)buf, 0);
			if (dev->hv_devhdl > 0) {
				int retval = hpi_init_device(dev, instance);
				if (retval)
					return retval;
			}
		} else {
			dev->info_string = kmalloc(80, GFP_KERNEL);
			if (!dev->info_string) {
				HPI_TRACE("Exit hpi_open -ENOMEM\n");
				return -ENOMEM;
			}
			sprintf(dev->info_string, "memory_size: %d\n",
				HPI_DATA_BUFFER_SIZE);
		}
	}
	/* If we tried and failed to open it, fail. */
	if (dev->hv_devhdl < 0)
		return (ssize_t)dev->hv_devhdl;

	filp->private_data = dev;

	return 0;          /* success */
}

/**
 * hpi_release() - Device release routine.
 * @inode: Inode for this device.
 * @filp: File for this specific open of the device.
 *
 * Returns zero, or an error code.
 */
static int hpi_release(struct inode *inode, struct file *filp)
{
	struct hpi_dev *dev = filp->private_data;

	/*
	 * Free our data structures.
	 */
	if (dev) {
		kfree(dev->data_buffer);
		hv_dev_poll_cancel(dev->hv_devhdl);
	}
	filp->private_data = NULL;

	return 0;
}

/**
 * hpi_read() - Read data from the device.
 * @filp: File for this specific open of the device.
 * @buf: User's data buffer.
 * @count: Number of bytes requested.
 * @f_pos: File position.
 *
 * Returns number of bytes read, or an error code.
 */
static ssize_t hpi_read(struct file *filp, char __user *buf,
			size_t count, loff_t *f_pos)
{
	int retval;
	struct hpi_dev *dev = filp->private_data;

	HPI_TRACE("hv_devhdl=0x%x count=%d offset=%lld(0x%llx)\n",
		  dev->hv_devhdl, count, *f_pos, *f_pos);

	/*
	 * The info file is a special case.  Just return the info
	 * string that was already constructed from hypervisor information.
	 */
	if (dev->info_string) {
		int info_len = strlen(dev->info_string);
		int bytes_avail = info_len - *f_pos;
		int xfer_len = (bytes_avail < count) ? bytes_avail : count;

		if (xfer_len <= 0)
			return 0;

		if (copy_to_user(buf, dev->info_string + *f_pos, xfer_len))
			return -EFAULT;
		*f_pos += xfer_len;
		return xfer_len;
	}

	/* Grab the lock. */
	if (mutex_lock_interruptible(&dev->mutex)) {
		HPI_TRACE("Exit hpi_read -ERESTARTSYS\n");
		return -ERESTARTSYS;
	}

	if (!dev->is_memory_file) {
		if (dev->hv_poll_outstanding == 0) {
			if (hv_dev_poll(dev->hv_devhdl,
					HV_DEVPOLL_READ,
					(HV_IntArg)&dev->driver_cb) < 0) {
				mutex_unlock(&dev->mutex);
				HPI_TRACE("Exit hpi_read "
					  "-ERESTARTSYS\n");
				return -ERESTARTSYS;
			}
		}

		while (dev->hv_poll_outstanding == 0) {
			mutex_unlock(&dev->mutex);
			/*
			 * If we are doing a non-blocking read of the interrupt
			 * file, return immediately if we haven't received an
			 * interrupt yet.
			 */
			if (filp->f_flags & O_NONBLOCK)	{
				if (dev->hv_poll_outstanding == 0) {
					HPI_TRACE("Exit hpi_read -EAGAIN\n");
					return -EAGAIN;
				}
			}

			if (wait_event_interruptible(dev->int_queue,
					     dev->hv_poll_outstanding)) {
				HPI_TRACE("Exit hpi_read -ERESTARTSYS\n");
				return -ERESTARTSYS;
			}
			if (mutex_lock_interruptible(&dev->mutex)) {
				HPI_TRACE("Exit hpi_read -ERESTARTSYS\n");
				return -ERESTARTSYS;
			}
		}

		/* Reset poll variables */
		dev->hv_poll_outstanding = 0;
		dev->poll_mask = POLLOUT | POLLWRNORM;
	}

	retval = hv_dev_pread(dev->hv_devhdl, 0,
			      (HV_VirtAddr) dev->data_buffer, count,
			      *f_pos);

	if (retval >= 0 && copy_to_user(buf, dev->data_buffer, count) != 0) {
		mutex_unlock(&dev->mutex);
		HPI_TRACE("Exit hpi_read -EFAULT");
		retval = -EFAULT;
	}

	mutex_unlock(&dev->mutex);

	return retval;
}

/**
 * hpi_write() - Write data to the device.
 * @filp: File for this specific open of the device.
 * @buf: User's data buffer.
 * @count: Number of bytes requested.
 * @f_pos: File position.
 *
 * Returns number of bytes written, or an error code.
 */
static ssize_t hpi_write(struct file *filp, const char __user *buf,
			 size_t count, loff_t *f_pos)
{
	int retval;
	struct hpi_dev *dev = filp->private_data;

	HPI_TRACE("hpi_write: hv_devhdl=0x%x\n", dev->hv_devhdl);

	if (dev->info_string)
		return -EINVAL;

	/* Grab the lock. */
	if (mutex_lock_interruptible(&dev->mutex)) {
		HPI_TRACE("Exit hpi_write -ERESTARTSYS\n");
		return -ERESTARTSYS;
	}

	if (copy_from_user(dev->data_buffer, buf, count) != 0) {
		mutex_unlock(&dev->mutex);
		HPI_TRACE("Exit hpi_write -EFAULT\n");
		return -EFAULT;
	}

	retval = hv_dev_pwrite(dev->hv_devhdl, 0,
			       (HV_VirtAddr) dev->data_buffer,
			       count, *f_pos);

	mutex_unlock(&dev->mutex);

	return retval;
}

static unsigned int hpi_poll(struct file *filp, poll_table *table)
{
	struct hpi_dev *dev = filp->private_data;

	HPI_TRACE("hpi_poll: hv_devhdl=0x%x\n", dev->hv_devhdl);

	/*
	 * Grab the semaphore so that this operation is ordered with respect
	 * to any other processes that may be reading or writing.
	 */
	if (mutex_lock_interruptible(&dev->mutex)) {
		HPI_TRACE("Exit hpi_poll -ERESTARTSYS\n");
		return -ERESTARTSYS;
	}
	poll_wait(filp, &dev->int_queue, table);

	if (dev->hv_poll_outstanding == 0) {
		if (hv_dev_poll(dev->hv_devhdl, HV_DEVPOLL_READ,
				(HV_IntArg)&dev->driver_cb) < 0) {
			mutex_unlock(&dev->mutex);
			HPI_TRACE("Exit hpi_poll -ERESTARTSYS\n");
			return -ERESTARTSYS;
		}
	}

	/* The memory file is always readable. */
	if (dev->is_memory_file)
		dev->poll_mask |= POLLIN | POLLRDNORM;

	/* Both devices are always writeable. */
	dev->poll_mask |= POLLOUT | POLLWRNORM;

	mutex_unlock(&dev->mutex);

	HPI_TRACE("POLL returning poll_mask = 0x%x\n", dev->poll_mask);
	return dev->poll_mask;
}

/*
 * The fops
 */
static const struct file_operations hpi_fops = {
	.owner =     THIS_MODULE,
	.read =	     hpi_read,
	.write =     hpi_write,
	.open =	     hpi_open,
	.poll =      hpi_poll,
	.release =   hpi_release,
};


/**
 * hpi_setup_cdev() - Set up a device instance in the cdev table.
 * @dev: Per-device HPI state.
 * @index: Device to set up.
 */
static void hpi_setup_cdev(struct hpi_dev *dev, int index)
{
	int err, devno = MKDEV(hpi_major, index);

	cdev_init(&dev->cdev, &hpi_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &hpi_fops;
	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
		HPI_TRACE("hpi_setup_cdev: Error %d adding hpi%d", err, index);
}

/** hpi_init() - Initialize the driver's module. */
static int hpi_init(void)
{
	int result, i;
	dev_t dev = MKDEV(hpi_major, 0);

	/*
	 * Register our major, and accept a dynamic number.
	 */
	if (hpi_major)
		result = register_chrdev_region(dev, hpi_devs, driver_name);
	else {
		result = alloc_chrdev_region(&dev, 0, hpi_devs, driver_name);
		hpi_major = MAJOR(dev);
	}
	if (result < 0)
		return result;

	/*
	 * Allocate the devices.
	 */
	hpi_devices = kzalloc(hpi_devs * sizeof(struct hpi_dev),
			      GFP_KERNEL);
	if (!hpi_devices) {
		unregister_chrdev_region(dev, hpi_devs);
		HPI_TRACE("Exit hpi_init -ENOMEM\n");
		return -ENOMEM;
	}
	for (i = 0; i < hpi_devs; i++) {
		hpi_setup_cdev(hpi_devices + i, i);

		/*
		 * Set up callback and initialize wait queue for finding out
		 * when interrupts arrive from the host.
		 */
		hpi_devices[i].driver_cb.dev = &hpi_devices[i];
		hpi_devices[i].driver_cb.callback = irq_handler;
		mutex_init(&hpi_devices[i].mutex);
		init_waitqueue_head(&hpi_devices[i].int_queue);
		hpi_devices[i].hv_poll_outstanding = 0;
	}

	return 0;
}

/** hpi_cleanup() - Cleanup function for HPI driver. */
static void hpi_cleanup(void)
{
	int i;

	for (i = 0; i < hpi_devs; i++) {
		cdev_del(&hpi_devices[i].cdev);
		kfree(hpi_devices[i].info_string);
		kfree(hpi_devices[i].data_buffer);
	}
	kfree(hpi_devices);
	unregister_chrdev_region(MKDEV(hpi_major, 0), hpi_devs);
}

module_init(hpi_init);
module_exit(hpi_cleanup);
