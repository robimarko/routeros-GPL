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
 * Tile-monitor FIFO driver.
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
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/poll.h>

#include <asm/atomic.h>

#include <hv/hypervisor.h>
#include <hv/drv_tmfifo_intf.h>

struct tmfifo_dev {
	struct cdev cdev;		/* Character device structure */
	int hv_devhdl;			/* Handle for hypervisor device */
	struct mutex mutex;		/* Allow only one accessor at a time */
	int dev_open;			/* Nonzero if device is open */
	void *chunk_buf;		/* Buffer for staging hypervisor I/O */

	int is_readable;		/* Nonzero if known to be readable */
	int readable_irq;		/* IRQ for readable interrupts */
	int last_readable_intrs;	/* Number of dev-is-readable events
					   before most recent read */
	atomic_t readable_intrs;	/* Number of dev-is-readable events */
	wait_queue_head_t readable_wq;	/* Waiters for readability */

	int is_writable;		/* Nonzero if known to be writable */
	int writable_irq;		/* IRQ for writable interrupts */
	int last_writable_intrs;	/* Number of dev-is-writable events
					   before most recent write */
	atomic_t writable_intrs;	/* Number of dev-is-writable events */
	wait_queue_head_t writable_wq;	/* Waiters for writability */
};

#define CHUNK_SIZE 4096		/* Maximum size I/O we do to the hypervisor */

static int tmfifo_major;	/* Dynamic major by default */
static int tmfifo_devs = 1;	/* One device; note we don't allow this to
				   be modified. */

module_param(tmfifo_major, int, 0);
MODULE_AUTHOR("Tilera Corporation");
MODULE_LICENSE("GPL");

#define ERROR(fmt, ...) \
	printk(KERN_ERR "tmfifo: " fmt "\n" , ## __VA_ARGS__)

#define INFO(fmt, ...) \
	pr_info("tmfifo: " fmt "\n" , ## __VA_ARGS__)

#define TRACE(fmt, ...) \
	pr_debug("tmfifo: " fmt "\n" , ## __VA_ARGS__)

static struct tmfifo_dev *tmfifo_devices; /* allocated in tmfifo_init */

/** Handle a readable interrupt on the current CPU. */
static irqreturn_t tmfifo_handle_readable_irq(int irq, void *dev_id)
{
	struct tmfifo_dev *dev = dev_id;
	atomic_inc(&dev->readable_intrs);
	wake_up_interruptible(&dev->readable_wq);
	TRACE("got readable intr");
	return IRQ_HANDLED;
}

/** Handle a writable interrupt on the current CPU. */
static irqreturn_t tmfifo_handle_writable_irq(int irq, void *dev_id)
{
	struct tmfifo_dev *dev = dev_id;
	atomic_inc(&dev->writable_intrs);
	wake_up_interruptible(&dev->writable_wq);
	TRACE("got writable intr");
	return IRQ_HANDLED;
}

/**
 * tmfifo_open() - Device open routine.
 * @inode: Inode for this device.
 * @filp: File for this specific open of the device.
 *
 * Returns zero, or an error code.
 */
static int tmfifo_open(struct inode *inode, struct file *filp)
{
	int retval = 0;
	struct tmfifo_intr_config tic;
	struct tmfifo_dev *dev = container_of(inode->i_cdev, struct
					      tmfifo_dev, cdev);
	int cpu = smp_processor_id();
	HV_LOTAR my_lotar = HV_XY_TO_LOTAR(cpu % smp_width, cpu / smp_width);

	mutex_lock(&dev->mutex);

	if (dev->dev_open) {
		retval = -EBUSY;
		goto err;
	}

	/* Allocate our I/O buffer, if needed. */
	if (!dev->chunk_buf) {
		dev->chunk_buf = kmalloc(CHUNK_SIZE, GFP_KERNEL);
		if (!dev->chunk_buf) {
			retval = -ENOMEM;
			goto err;
		}
	}

	/* Now open the hypervisor device if we haven't already. */
	if (dev->hv_devhdl == 0)
		dev->hv_devhdl = hv_dev_open((HV_VirtAddr) "tmfifo/0", 0);

	/* If we tried and failed to open it, fail. */
	if (dev->hv_devhdl < 0) {
		retval = (dev->hv_devhdl == HV_ENODEV) ? -ENODEV :
							 dev->hv_devhdl;
		goto err_free_buf;
	}

	/* Create our IRQs and register them. */
	dev->readable_irq = create_irq();
	if (dev->readable_irq < 0) {
		retval = -ENXIO;
		goto err_close_hv;
	}
	dev->writable_irq = create_irq();
	if (dev->writable_irq < 0) {
		retval = -ENXIO;
		goto err_dest_r_irq;
	}

	tile_irq_activate(dev->readable_irq, TILE_IRQ_PERCPU);
	tile_irq_activate(dev->writable_irq, TILE_IRQ_PERCPU);

	if (request_irq(dev->readable_irq, tmfifo_handle_readable_irq, 0,
			"tmfifo_r", dev)) {
		retval = -ENXIO;
		goto err_dest_w_irq;
	}
	if (request_irq(dev->writable_irq, tmfifo_handle_writable_irq, 0,
			"tmfifo_w", dev)) {
		retval = -ENXIO;
		goto err_free_r_irq;
	}

	/* Request that the hypervisor start sending us interrupts. */

	tic.readable_event = dev->readable_irq;
	tic.writable_event = dev->writable_irq;
	tic.readable_lotar = my_lotar;
	tic.writable_lotar = my_lotar;

	if (hv_dev_pwrite(dev->hv_devhdl, 0, (HV_VirtAddr)&tic,
			  sizeof(tic), TMFIFO_CONFIGURE_INTR) < 0) {
		retval = -EIO;
		goto err_free_w_irq;
	}

	dev->dev_open = 1;

	mutex_unlock(&dev->mutex);

	filp->private_data = dev;

	return 0;


err_free_w_irq:
	free_irq(dev->writable_irq, dev);
err_free_r_irq:
	free_irq(dev->readable_irq, dev);
err_dest_w_irq:
	destroy_irq(dev->writable_irq);
err_dest_r_irq:
	destroy_irq(dev->readable_irq);
err_close_hv:
	hv_dev_close(dev->hv_devhdl);
err_free_buf:
	kfree(dev->chunk_buf);
	dev->chunk_buf = NULL;
err:

	mutex_unlock(&dev->mutex);

	return retval;
}


/**
 * tmfifo_release() - Device release routine.
 * @inode: Inode for this device.
 * @filp: File for this specific open of the device.
 *
 * Returns zero, or an error code.
 */
static int tmfifo_release(struct inode *inode, struct file *filp)
{
	struct tmfifo_intr_config tic;
	struct tmfifo_dev *dev = filp->private_data;
	int retval = 0;

	mutex_lock(&dev->mutex);

	kfree(dev->chunk_buf);
	dev->chunk_buf = NULL;

	/*
	 * Tell the hypervisor to stop sending us interrupts, then close
	 * the device.
	 */
	tic.readable_event = -1;
	tic.writable_event = -1;

	hv_dev_pwrite(dev->hv_devhdl, 0, (HV_VirtAddr)&tic, sizeof(tic),
		      TMFIFO_CONFIGURE_INTR);

	hv_dev_close(dev->hv_devhdl);
	dev->hv_devhdl = 0;

	/* Unregister and destroy our IRQs. */
	free_irq(dev->readable_irq, dev);
	free_irq(dev->writable_irq, dev);

	destroy_irq(dev->readable_irq);
	destroy_irq(dev->writable_irq);

	/* Mark the device as no longer open. */

	dev->dev_open = 0;

	filp->private_data = NULL;

	mutex_unlock(&dev->mutex);

	return retval;
}


/**
 * tmfifo_read() - Read data from the device.
 * @filp: File for this specific open of the device.
 * @buf: User's data buffer.
 * @count: Number of bytes requested.
 * @f_pos: File position.
 *
 * Returns number of bytes read, or an error code.
 */
static ssize_t tmfifo_read(struct file *filp, char __user *buf,
			   size_t count, loff_t *f_pos)
{
	struct tmfifo_dev *dev = filp->private_data;
	size_t rd_cnt = 0;

	if (dev->hv_devhdl < 0)
		return -EINVAL;

	if (mutex_lock_interruptible(&dev->mutex))
		return -ERESTARTSYS;

	while (count) {
		int hv_retval;
		int hv_retval_bytes;
		int readsize = min(count, (size_t) CHUNK_SIZE);

		if (!dev->is_readable && dev->last_readable_intrs ==
		    atomic_read(&dev->readable_intrs)) {
			TRACE("tmfifo_read: not readable");
			if (rd_cnt || (filp->f_flags & O_NONBLOCK)) {
				mutex_unlock(&dev->mutex);
				TRACE("tmfifo_read: returning %zd/EAGAIN",
				      rd_cnt);
				return rd_cnt ? rd_cnt : -EAGAIN;
			} else {
				mutex_unlock(&dev->mutex);
				TRACE("tmfifo_read: waiting for readable");
				if (wait_event_interruptible(dev->readable_wq,
					(dev->last_readable_intrs !=
					 atomic_read(&dev->readable_intrs)))) {
					TRACE("tmfifo_read: returning "
					      "%zd/ERESTARTSYS", rd_cnt);
					return rd_cnt ? rd_cnt : -ERESTARTSYS;
				}
				mutex_lock(&dev->mutex);
				/*
				 * Since we dropped the mutex, let's
				 * restart the loop in case some other
				 * state changed while we were sleeping.
				 */
				continue;
			}
		}

		/*
		 * Before we read, save the current interrupt count.  We
		 * have to do this here, since otherwise we could miss a
		 * readable interrupt which showed up concurrently with the
		 * hypervisor pread routine returning.
		 */
		dev->last_readable_intrs = atomic_read(&dev->readable_intrs);

		/* Since we're reading a FIFO, the offset is always 0. */
		hv_retval = hv_dev_pread(dev->hv_devhdl, 0,
					 (HV_VirtAddr) dev->chunk_buf,
					 readsize, 0);

		if (hv_retval < 0) {
			mutex_unlock(&dev->mutex);
			return rd_cnt ? rd_cnt : -EIO;
		}

		hv_retval_bytes = TMFIFO_RETVAL_BYTES(hv_retval);
		dev->is_readable = TMFIFO_RETVAL_MORE(hv_retval);

		if (hv_retval_bytes && copy_to_user(buf, dev->chunk_buf,
						    hv_retval_bytes) != 0) {
			mutex_unlock(&dev->mutex);
			return rd_cnt ? rd_cnt : -EFAULT;
		}

		rd_cnt += hv_retval_bytes;
		*f_pos += hv_retval_bytes;
		buf += hv_retval_bytes;
		count -= hv_retval_bytes;
	}

	mutex_unlock(&dev->mutex);

	return rd_cnt;
}

/**
 * tmfifo_write() - Write data to the device.
 * @filp: File for this specific open of the device.
 * @buf: User's data buffer.
 * @count: Number of bytes requested.
 * @f_pos: File position.
 *
 * Returns number of bytes written, or an error code.
 */
static ssize_t tmfifo_write(struct file *filp, const char __user *buf,
			    size_t count, loff_t *f_pos)
{
	struct tmfifo_dev *dev = filp->private_data;
	size_t wr_cnt = 0;

	if (dev->hv_devhdl < 0)
		return -EINVAL;

	if (mutex_lock_interruptible(&dev->mutex))
		return -ERESTARTSYS;

	while (count) {
		int hv_retval;
		int hv_retval_bytes;
		int writesize = min(count, (size_t) CHUNK_SIZE);

		if (!dev->is_writable && dev->last_writable_intrs ==
		    atomic_read(&dev->writable_intrs)) {
			TRACE("tmfifo_write: not writable");
			if (filp->f_flags & O_NONBLOCK) {
				mutex_unlock(&dev->mutex);
				TRACE("tmfifo_write: returning %zd/EAGAIN",
				      wr_cnt);
				return wr_cnt ? wr_cnt : -EAGAIN;
			} else {
				mutex_unlock(&dev->mutex);
				TRACE("tmfifo_write: waiting for writable");
				if (wait_event_interruptible(dev->writable_wq,
					(dev->last_writable_intrs !=
					 atomic_read(&dev->writable_intrs)))) {
					TRACE("tmfifo_write: returning "
					      "%zd/ERESTARTSYS", wr_cnt);
					return wr_cnt ? wr_cnt : -ERESTARTSYS;
				}
				mutex_lock(&dev->mutex);
				/*
				 * Since we dropped the mutex, let's
				 * restart the loop in case some other
				 * state changed while we were sleeping.
				 */
				continue;
			}
		}

		if (copy_from_user(dev->chunk_buf, buf, writesize) != 0) {
			mutex_unlock(&dev->mutex);
			TRACE("tmfifo_write: returning %zd/EFAULT", wr_cnt);
			return wr_cnt ? wr_cnt : -EFAULT;
		}

		/*
		 * Before we write, save the current interrupt count.  We
		 * have to do this here, since otherwise we could miss a
		 * writable interrupt which showed up concurrently with the
		 * hypervisor pwrite routine returning.
		 */
		dev->last_writable_intrs = atomic_read(&dev->writable_intrs);

		/* Since we're writing a FIFO, the offset is always 0. */
		hv_retval = hv_dev_pwrite(dev->hv_devhdl, 0,
					  (HV_VirtAddr) dev->chunk_buf,
					  writesize, 0);

		if (hv_retval < 0) {
			mutex_unlock(&dev->mutex);
			return wr_cnt ? wr_cnt : -EIO;
		}

		hv_retval_bytes = TMFIFO_RETVAL_BYTES(hv_retval);
		dev->is_writable = TMFIFO_RETVAL_MORE(hv_retval);

		wr_cnt += hv_retval_bytes;
		*f_pos += hv_retval_bytes;
		buf += hv_retval_bytes;
		count -= hv_retval_bytes;
	}

	mutex_unlock(&dev->mutex);

	return wr_cnt;
}

static unsigned int tmfifo_poll(struct file *filp, poll_table *wait)
{
	struct tmfifo_dev *dev = filp->private_data;
	unsigned int retval = 0;

	mutex_lock(&dev->mutex);

	poll_wait(filp, &dev->readable_wq, wait);
	poll_wait(filp, &dev->writable_wq, wait);

	if (dev->is_readable || dev->last_readable_intrs !=
	    atomic_read(&dev->readable_intrs))
		retval |= POLLIN | POLLRDNORM;
	if (dev->is_writable || dev->last_writable_intrs !=
	    atomic_read(&dev->writable_intrs))
		retval |= POLLOUT | POLLWRNORM;

	mutex_unlock(&dev->mutex);

	return retval;
}

/*
 * The fops
 */
static const struct file_operations tmfifo_fops = {
	.owner =     THIS_MODULE,
	.open =      tmfifo_open,
	.read =      tmfifo_read,
	.write =     tmfifo_write,
	.poll =      tmfifo_poll,
	.release =   tmfifo_release,
};

/**
 * tmfifo_setup_cdev() - Set up a device instance in the cdev table.
 * @dev: Per-device SROM state.
 * @index: Device to set up.
 */
static void tmfifo_setup_cdev(struct tmfifo_dev *dev, int index)
{
	int err, devno = MKDEV(tmfifo_major, index);

	cdev_init(&dev->cdev, &tmfifo_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &tmfifo_fops;
	err = cdev_add(&dev->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		pr_notice("Error %d adding tmfifo%d", err, index);
}

/** tmfifo_init() - Initialize the driver's module. */
static int tmfifo_init(void)
{
	int result, i;
	dev_t dev = MKDEV(tmfifo_major, 0);

	/*
	 * Register our major, and accept a dynamic number.
	 */
	if (tmfifo_major)
		result = register_chrdev_region(dev, tmfifo_devs, "tmfifo");
	else {
		result = alloc_chrdev_region(&dev, 0, tmfifo_devs, "tmfifo");
		tmfifo_major = MAJOR(dev);
	}
	if (result < 0)
		return result;


	/*
	 * Allocate the devices -- we can't have them static, as the number
	 * can be specified at load time.
	 */
	tmfifo_devices = kzalloc(tmfifo_devs * sizeof(struct tmfifo_dev),
			       GFP_KERNEL);
	if (!tmfifo_devices) {
		unregister_chrdev_region(dev, tmfifo_devs);
		return -ENOMEM;
	}
	for (i = 0; i < tmfifo_devs; i++) {
		tmfifo_setup_cdev(tmfifo_devices + i, i);
		mutex_init(&tmfifo_devices[i].mutex);
		init_waitqueue_head(&tmfifo_devices[i].readable_wq);
		init_waitqueue_head(&tmfifo_devices[i].writable_wq);
		atomic_set(&tmfifo_devices[i].readable_intrs, 0);
		atomic_set(&tmfifo_devices[i].writable_intrs, 0);
	}

	return 0; /* succeed */
}

/** tmfifo_cleanup() - Clean up the driver's module. */
static void tmfifo_cleanup(void)
{
	int i;

	for (i = 0; i < tmfifo_devs; i++)
		cdev_del(&tmfifo_devices[i].cdev);
	kfree(tmfifo_devices);
	unregister_chrdev_region(MKDEV(tmfifo_major, 0), tmfifo_devs);
}

module_init(tmfifo_init);
module_exit(tmfifo_cleanup);
