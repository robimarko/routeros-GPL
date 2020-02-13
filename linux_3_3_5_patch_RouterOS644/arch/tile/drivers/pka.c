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
 * PKA driver
 *
 * This source code is derived from code provided in "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published by
 * O'Reilly & Associates.
 */

#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/poll.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <linux/irq.h>
#include <linux/jiffies.h>

#include <asm/pka.h>

#include <hv/hypervisor.h>
#include <hv/drv_pka_intf.h>

/* Module parameters, with defaults */
static int pka_num_rings = PKA_NUM_RINGS;

static bool pka_ring0_is_priority;

static int pka_ring0_elem_size = 2048;
static int pka_ring1_elem_size = 2048;
static int pka_ring2_elem_size = 2048;
static int pka_ring3_elem_size = 2048;

module_param_named(num_hw_queues, pka_num_rings, int, 0644);
MODULE_PARM_DESC(num_hw_queues, "number of queues in the device");

module_param_named(q0_high_priority, pka_ring0_is_priority, bool, 0644);
MODULE_PARM_DESC(q0_high_priority, "queue 0 has high priority");

module_param_named(q0_elem_size, pka_ring0_elem_size, int, 0644);
MODULE_PARM_DESC(q0_elem_size, "number of bytes in each element in hw queue 0");
module_param_named(q1_elem_size, pka_ring1_elem_size, int, 0644);
MODULE_PARM_DESC(q1_elem_size, "number of bytes in each element in hw queue 1");
module_param_named(q2_elem_size, pka_ring2_elem_size, int, 0644);
MODULE_PARM_DESC(q2_elem_size, "number of bytes in each element in hw queue 2");
module_param_named(q3_elem_size, pka_ring3_elem_size, int, 0644);
MODULE_PARM_DESC(q3_elem_size, "number of bytes in each element in hw queue 3");

#define PKA_NUM_VECTORS_PER_CMD 5
#define PKA_VECTOR_SIZE 512

struct pka_dev {
	gxio_mica_pka_context_t *context; /* Pointer back to our context */
	struct cdev cdev;	         /* Character device structure */
	int ring;		/* The hw ring number for this device */
	wait_queue_head_t cmd_wq;        /* Waiters for cmd queue */
	wait_queue_head_t res_wq;        /* Waiters for res queue */
	int poll_mask;		/* Mask returned by poll routine */
	struct mutex mutex;             /* Guarantee one accessor at a time */
	int cmds_outstanding; /* Number of commands still on the queue */
	uint32_t* vector[PKA_NUM_VECTORS_PER_CMD]; /* Buffers for vectors */
};

struct pka_shim {
	gxio_mica_pka_context_t context; /* Context object to manage
					    PKA hw resources */
	int irq;		         /* IRQ for when we need to send
					    more cmds or get results */
	struct pka_dev *devices;	/* Devices associated with this shim */
	dev_t dev;
};

MODULE_AUTHOR("Tilera Corporation");
MODULE_LICENSE("Dual BSD/GPL");

#define DRIVER_NAME_STRING "pka"

#define NUM_PKA_DEVS_PER_SHIM PKA_NUM_RINGS
#define NUM_PKA_SHIMS 2

#define STRINGIFY(x) #x
#define TOSTRING(x)	 STRINGIFY(x)
#define SIMPLE_MSG_LINE    DRIVER_NAME_STRING "(" TOSTRING(__LINE__) "): "
#define INFO(FMT, ...) \
	pr_info(SIMPLE_MSG_LINE FMT, ## __VA_ARGS__)
#define WARNING(FMT, ...) \
	pr_warning(SIMPLE_MSG_LINE FMT, ## __VA_ARGS__)
#define TRACE(FMT, ...) \
	pr_debug(SIMPLE_MSG_LINE FMT, ## __VA_ARGS__)

static struct pka_shim pka_shims[NUM_PKA_SHIMS];

#ifdef DEBUG_PKA
static void pka_debug_dump_cmd_desc(pka_cmd_desc_t *cmd)
{
	printk(KERN_DEBUG "va_a = %p\n", cmd->va_a);
	printk(KERN_DEBUG "va_b = %p\n", cmd->va_b);
	printk(KERN_DEBUG "va_c = %p\n", cmd->va_c);
	printk(KERN_DEBUG "va_d = %p\n", cmd->va_d);
	printk(KERN_DEBUG "va_e = %p\n", cmd->va_e);
	printk(KERN_DEBUG "tag = %d\n", cmd->tag);
	printk(KERN_DEBUG "len_a = %d\n", cmd->len_a);
	printk(KERN_DEBUG "len_b = %d\n", cmd->len_b);
	printk(KERN_DEBUG "len_c = %d\n", cmd->len_c);
	printk(KERN_DEBUG "len_d = %d\n", cmd->len_d);
	printk(KERN_DEBUG "len_e = %d\n", cmd->len_e);
	printk(KERN_DEBUG "rsvd_0 = %d\n", cmd->rsvd_0);
	printk(KERN_DEBUG "hw_len_a = %d\n", cmd->hw_len_a);
	printk(KERN_DEBUG "rsvd_1 = %d\n", cmd->rsvd_1);
	printk(KERN_DEBUG "hw_len_b = %d\n", cmd->hw_len_b);
	printk(KERN_DEBUG "rsvd_2 = %d\n", cmd->rsvd_2);
	printk(KERN_DEBUG "command = 0x%x\n", cmd->command);
	printk(KERN_DEBUG "rsvd_3 = %d\n", cmd->rsvd_3);
	printk(KERN_DEBUG "enc_vectors_bitmask = 0x%x\n",
	       cmd->enc_vectors_bitmask);
	printk(KERN_DEBUG "kdr = %d\n", cmd->kdr);
	printk(KERN_DEBUG "odd_powers = %d\n", cmd->odd_powers);
	printk(KERN_DEBUG "driver_status = %d\n", cmd->driver_status);
	printk(KERN_DEBUG "linked = %d\n", cmd->linked);
}

static void pka_debug_dump_res_desc(pka_res_desc_t *res)
{
	printk(KERN_DEBUG "Result descriptor at %p\n", res);
	printk(KERN_DEBUG "va_a = %p\n", res->va_a);
	printk(KERN_DEBUG "va_b = %p\n", res->va_b);
	printk(KERN_DEBUG "va_c = %p\n", res->va_c);
	printk(KERN_DEBUG "va_d = %p\n", res->va_d);
	printk(KERN_DEBUG "tag = 0x%08x\n", res->tag);
	printk(KERN_DEBUG "len_a = %d\n", res->len_a);
	printk(KERN_DEBUG "len_b = %d\n", res->len_b);
	printk(KERN_DEBUG "len_c = %d\n", res->len_c);
	printk(KERN_DEBUG "len_d = %d\n", res->len_d);

	printk(KERN_DEBUG "modulo_is_0 = 0x%x\n", res->modulo_is_0);
	printk(KERN_DEBUG "rsvd_0 = 0x%x\n", res->rsvd_0);
	printk(KERN_DEBUG "ms_offset = 0x%x\n", res->ms_offset);
	printk(KERN_DEBUG "rsvd_1 = 0x%x\n", res->rsvd_1);
	printk(KERN_DEBUG "result_is_0 = 0x%x\n", res->result_is_0);
	printk(KERN_DEBUG "rsvd_2 = 0x%x\n", res->rsvd_2);
	printk(KERN_DEBUG "main_result_msw_offset = 0x%x\n"
	       , res->main_result_msw_offset);
	printk(KERN_DEBUG "rsvd_3 = 0x%x\n", res->rsvd_3);

	printk(KERN_DEBUG "cmp_res = 0x%x\n", res->cmp_res);
	printk(KERN_DEBUG "rsvd_4 = 0x%x\n", res->rsvd_4);
	printk(KERN_DEBUG "len_b  = 0x%x\n", res->len_b);
	printk(KERN_DEBUG "rsvd_5 = 0x%x\n", res->rsvd_5);
	printk(KERN_DEBUG "len_a  = 0x%x\n", res->len_a);
	printk(KERN_DEBUG "rsvd_6 = 0x%x\n", res->rsvd_6);

	printk(KERN_DEBUG "linked = 0x%x\n", res->linked);
	printk(KERN_DEBUG "written_zero = 0x%x\n", res->written_zero);
	printk(KERN_DEBUG "odd_powers = 0x%x\n", res->odd_powers);
	printk(KERN_DEBUG "kdr = 0x%x\n", res->kdr);
	printk(KERN_DEBUG "enc_vectors_bitmask = 0x%x\n",
	       res->enc_vectors_bitmask);
	printk(KERN_DEBUG "result_code = 0x%x\n", res->result_code);
	printk(KERN_DEBUG "command = 0x%x\n", res->command);
}
#endif

/** Handle a command or result interrupt. */
static irqreturn_t pka_handle_irq(int irq, void *arg)
{
	struct pka_shim *shim = arg;
	gxio_mica_pka_context_t *context = &shim->context;
	uint32_t interrupts = 0;
	struct pka_dev *dev;
	int i;

	TRACE("got pka intr");

	gxio_mica_pka_read_interrupts(context, &interrupts);

	for (i = 0; i < context->num_rings; i++) {
		dev = &shim->devices[i];

		/* check result interrupts */
		if (interrupts & (1 << i << 4)) {
			TRACE("Result irq for device %d\n", i);
			dev->poll_mask |= POLLIN;
			wake_up_interruptible(&dev->res_wq);
		}
		/* check command interrupts */
		if (interrupts & (1 << i)) {
			TRACE("Command irq for device %d\n", i);
			dev->poll_mask |= POLLOUT;
			wake_up_interruptible(&dev->cmd_wq);
		}
	}

	return IRQ_HANDLED;
}

static int setup_rings(gxio_mica_pka_context_t *context, int shim_no)
{
	gxio_mica_pka_ring_config_t ring_config = { 0 };
	int err;

	ring_config.num_rings = pka_num_rings;
	ring_config.ring_0_is_high_priority = pka_ring0_is_priority;

	ring_config.ring_elem_size[0] = pka_ring0_elem_size;
	ring_config.ring_elem_size[1] = pka_ring1_elem_size;
	ring_config.ring_elem_size[2] = pka_ring2_elem_size;
	ring_config.ring_elem_size[3] = pka_ring3_elem_size;

	err = gxio_mica_pka_init(context, shim_no, &ring_config);

	return err;
}

static int setup_irqs(struct pka_shim *shim)
{
	int irq;
	int cpu = smp_processor_id();
	int ring;
	gxio_mica_pka_context_t *context = &shim->context;

	/* Create our IRQs and register them. */
	irq = create_irq();
	if (irq < 0)
		return -ENXIO;

	tile_irq_activate(irq, TILE_IRQ_SW_CLEAR);

	if (request_irq(irq, pka_handle_irq, 0, "pka", context)) {
		destroy_irq(irq);
		return -ENXIO;
	}

	/* Request that the hardware start sending us interrupts. */
	/* Point all interrupts at the same IRQ and handler. */
	for (ring = 0; ring < context->num_rings; ring++) {
		gxio_mica_pka_cfg_cmd_queue_empty_interrupt(context,
							    cpu % smp_width,
							    cpu / smp_width,
							    CONFIG_KERNEL_PL,
							    irq,
							    ring);
		gxio_mica_pka_cfg_res_queue_full_interrupt(context,
							   cpu % smp_width,
							   cpu / smp_width,
							   CONFIG_KERNEL_PL,
							   irq,
							   ring);
	}

	return irq;
}

static int pka_open(struct inode *inode, struct file *filp)
{
	unsigned int minor = MINOR(inode->i_rdev);
	struct pka_dev *dev = container_of(inode->i_cdev, struct
					   pka_dev, cdev);
	int ring = minor;
	int retval = 0;
	int i;

	mutex_lock(&dev->mutex);

	filp->private_data = dev;

	for (i = 0; i < PKA_NUM_VECTORS_PER_CMD; i++)
	if (!dev->vector[i]) {
		dev->vector[i] = (uint32_t*)kmalloc(PKA_VECTOR_SIZE,
						    GFP_KERNEL);
		if (!dev->vector[i]) {
			retval = -ENOMEM;
			goto err;
		}
	}

	if (gxio_mica_pka_command_slots_available(dev->context, ring))
		dev->poll_mask = POLLOUT;

	mutex_unlock(&dev->mutex);
	return 0;
err:
	for (i = 0; i < PKA_NUM_VECTORS_PER_CMD; i++)
		if (dev->vector[i]) {
			kfree(dev->vector[i]);
			dev->vector[i] = NULL;
		}

	mutex_unlock(&dev->mutex);
	return retval;
}


/**
 * pka_ioctl() - Device ioctl routine.
 * @filp: File for this specific open of the device.
 * @cmd: ioctl command.
 * @arg: Pointer to user argument.
 *
 * Returns zero, or an error code.
 */
static long pka_ioctl(struct file *filp,
			  unsigned int cmd, unsigned long arg)
{
	struct pka_dev *dev = filp->private_data;
	gxio_mica_pka_context_t *context = dev->context;
	int ring = dev->ring;
	pka_cmd_desc_t cmd_desc = {{ 0 }};
	pka_res_desc_t res_desc = {{ 0 }};
	pka_res_desc_t kern_res = {{ 0 }};
	int err = 0;

	if (mutex_lock_interruptible(&dev->mutex)) {
		TRACE("Exit pka_ioctl -ERESTARTSYS\n");
		return -ERESTARTSYS;
	}

	switch (cmd) {
	case PKA_QUEUE_CMD:
		if (gxio_mica_pka_command_slots_available(context,
							  ring) == 0) {
			mutex_unlock(&dev->mutex);
			return -EBUSY;
		}

		err |= copy_from_user(&cmd_desc, (void __user *)arg,
				     sizeof(cmd_desc));

		if (cmd_desc.vec_a.len) {
			err |= copy_from_user(dev->vector[0], cmd_desc.vec_a.va,
					      cmd_desc.vec_a.len * 4);
			cmd_desc.vec_a.va = dev->vector[0];
		}
		if (cmd_desc.vec_b.len) {
			err |= copy_from_user(dev->vector[1], cmd_desc.vec_b.va,
					      cmd_desc.vec_b.len * 4);
			cmd_desc.vec_b.va = dev->vector[1];
		}
		if (cmd_desc.vec_c.len) {
			err |= copy_from_user(dev->vector[2], cmd_desc.vec_c.va,
					      cmd_desc.vec_c.len * 4);
			cmd_desc.vec_c.va = dev->vector[2];
		}
		if (cmd_desc.vec_d.len) {
			err |= copy_from_user(dev->vector[3], cmd_desc.vec_d.va,
					      cmd_desc.vec_d.len * 4);
			cmd_desc.vec_d.va = dev->vector[3];
		}
		if (cmd_desc.vec_e.len) {
			err |= copy_from_user(dev->vector[4], cmd_desc.vec_e.va,
					      cmd_desc.vec_e.len * 4);
			cmd_desc.vec_e.va = dev->vector[4];
		}

		if (err != 0) {
			mutex_unlock(&dev->mutex);
			return -EFAULT;
		}

		if (gxio_mica_pka_put_command_on_ring(context, ring,
						      &cmd_desc)) {
			mutex_unlock(&dev->mutex);
			return -EFAULT;
		}
		dev->cmds_outstanding++;

		/* Reset poll mask.
		 * NOTE: if we decide to go with level interrupts for commands
		 * then we can unconditionally unset POLLOUT.  If the commands
		 * are under the threshold we'll get an interrupt immediately.
		 * Here we are asking the hardware again because even if the
		 * first time we checked there was only one slot, one may well
		 * have become available since then.
		 */
		if (gxio_mica_pka_command_slots_available(context, ring) == 0)
			dev->poll_mask &= ~POLLOUT;

		/* Can re-arm interrupt now that we've added a command to
		   the queue. */
		gxio_mica_pka_reset_interrupts(context, 1 << 4);

		break;
	case PKA_DEQUEUE_RES:
		if (gxio_mica_pka_results_available(context, ring) <= 0) {
			mutex_unlock(&dev->mutex);
			return -EAGAIN;
		}

		err |= copy_from_user(&res_desc, (void __user *)arg,
				      sizeof(res_desc));

		memcpy(&kern_res, &res_desc, sizeof(kern_res));
		kern_res.vec_a.va = dev->vector[0];
		kern_res.vec_b.va = dev->vector[1];
		kern_res.vec_c.va = dev->vector[2];
		kern_res.vec_d.va = dev->vector[3];
		if (gxio_mica_pka_get_result_from_ring(context, ring,
						       &kern_res)) {
			mutex_unlock(&dev->mutex);
			return -EFAULT;
		}
		dev->cmds_outstanding--;

		/* We can re-arm the interrupt now that we've taken a
		   result from the queue. */
		gxio_mica_pka_reset_interrupts(context,
					       1 << ring << 4);

		dev->poll_mask &= ~POLLIN;

		if (res_desc.vec_a.len)
			err |= copy_to_user(res_desc.vec_a.va, dev->vector[0],
					    res_desc.vec_a.len * 4);

		if (res_desc.vec_b.len)
			err |= copy_to_user(res_desc.vec_b.va, dev->vector[1],
					    res_desc.vec_b.len * 4);

		if (res_desc.vec_c.len)
			err |= copy_to_user(res_desc.vec_c.va, dev->vector[2],
					    res_desc.vec_c.len * 4);

		if (res_desc.vec_d.len)
			err |= copy_to_user(res_desc.vec_d.va, dev->vector[3],
					    res_desc.vec_d.len * 4);

		if (err != 0) {
			mutex_unlock(&dev->mutex);
			return -EFAULT;
		}

		break;
	default:
		mutex_unlock(&dev->mutex);
		return -EINVAL;
	}

	mutex_unlock(&dev->mutex);
	return 0;
}

static unsigned int pka_poll(struct file *filp, poll_table *table)
{
	struct pka_dev *dev = filp->private_data;

	if (mutex_lock_interruptible(&dev->mutex)) {
		TRACE("Exit pka_poll -ERESTARTSYS\n");
		return -ERESTARTSYS;
	}

	poll_wait(filp, &dev->res_wq, table);
	poll_wait(filp, &dev->cmd_wq, table);

	mutex_unlock(&dev->mutex);

	return dev->poll_mask;
}

static int pka_release(struct inode *inode, struct file *filp)
{
	struct pka_dev *dev = filp->private_data;
	pka_res_desc_t res = {{ 0 }};
	int i;
	int start_jiffies;

	mutex_lock(&dev->mutex);

	start_jiffies = jiffies;
	TRACE("draining results from ring %d: cmds_outstanding = %d, "
	      "results_available = %d\n", dev->ring, dev->cmds_outstanding,
	      gxio_mica_pka_results_available(dev->context, dev->ring));

	/* Drain and discard any outstanding results from the queue. */
	for (i = 0; i < dev->cmds_outstanding; i++) {
		while (gxio_mica_pka_results_available(dev->context,
						       dev->ring) == 0) {
			if (start_jiffies + jiffies > usecs_to_jiffies(1000))
			{
				mutex_unlock(&dev->mutex);
				return -1;
			}
		}

		gxio_mica_pka_get_result_from_ring(dev->context, dev->ring,
						   &res);
		dev->cmds_outstanding--;
	}

	for (i = 0; i < PKA_NUM_VECTORS_PER_CMD; i++)
		if (dev->vector[i]) {
			kfree(dev->vector[i]);
			dev->vector[i] = NULL;
		}

	mutex_unlock(&dev->mutex);

	return 0;
}

/*
 * The fops
 */
static const struct file_operations pka_fops = {
	.owner =     THIS_MODULE,
	.open =	     pka_open,
	.poll =      pka_poll,
	.unlocked_ioctl = pka_ioctl,
	.release =   pka_release,
};

static int pka_setup_shim(struct pka_shim *shim, int shim_no)
{
	struct pka_dev *pka_devices;
	char driver_name[32];
	int retval;
	int i;

	snprintf(driver_name, sizeof(driver_name), "%s%d", DRIVER_NAME_STRING,
		 shim_no);

	/* Allocate some major/minor numbers. */
	retval = alloc_chrdev_region(&shim->dev, 0, NUM_PKA_DEVS_PER_SHIM,
				     driver_name);
	if (retval != 0) {
		WARNING("Could not allocate major device number for "
			"pka shim %d.\n", shim_no);
		return retval;
	}
	/* Allocate and register the devices. */
	pka_devices = kzalloc(NUM_PKA_DEVS_PER_SHIM * sizeof(struct pka_dev),
			      GFP_KERNEL);
	if (!pka_devices) {
		TRACE("Exit pka_init -ENOMEM\n");
		retval = -ENOMEM;
		goto err_alloc;
	}
	retval = setup_rings(&shim->context, shim_no);
	if (retval != 0) {
		TRACE("Could not initialize queues for pka shim %d.\n",
		      shim_no);
		goto err_alloc;
	}
	shim->irq = setup_irqs(shim);
	if (shim->irq < 0) {
		TRACE("Could not initialize interrupts for pka shim %d.\n",
		      shim_no);
		retval = shim->irq;
		goto err_irq;
	}
	for (i = 0; i < NUM_PKA_DEVS_PER_SHIM; i++) {
		int devno = MKDEV(MAJOR(shim->dev), i);
		cdev_init(&pka_devices[i].cdev, &pka_fops);
		pka_devices[i].cdev.owner = THIS_MODULE;
		retval = cdev_add(&pka_devices[i].cdev, devno, 1);
		if (retval != 0) {
			WARNING("Failed to add pka cdev.\n");
			goto err_irq;
		}
		pka_devices[i].context = &shim->context;
		pka_devices[i].ring = i;
		mutex_init(&pka_devices[i].mutex);
		init_waitqueue_head(&pka_devices[i].cmd_wq);
		init_waitqueue_head(&pka_devices[i].res_wq);
	}

	shim->devices = pka_devices;

	return 0;

err_irq:
	destroy_irq(shim->irq);
err_alloc:
	kfree(pka_devices);
	unregister_chrdev_region(shim->dev, NUM_PKA_DEVS_PER_SHIM);

	return retval;
}

/** pka_init() - Initialize the driver's module. */
static int pka_init(void)
{
	int i;

	for (i = 0; i < NUM_PKA_SHIMS; i++)
		pka_setup_shim(&pka_shims[i], i);
	return 0;
}

/** pka_cleanup() - Cleanup function for PKA driver. */
static void pka_cleanup(void)
{
	int i, j;
	for (i = 0; i < NUM_PKA_SHIMS; i++) {
		struct pka_shim *shim = &pka_shims[i];

		for (j = 0; j < NUM_PKA_DEVS_PER_SHIM; j++) {
			cdev_del(&shim->devices[j].cdev);
			unregister_chrdev_region(shim->dev,
						 NUM_PKA_DEVS_PER_SHIM);
			destroy_irq(shim->irq);
		}
	}
}

module_init(pka_init);
module_exit(pka_cleanup);
