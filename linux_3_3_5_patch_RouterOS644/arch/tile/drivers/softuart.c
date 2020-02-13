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
 * Soft UART driver.
 *
 * This source code is derived from code provided in "Linux Device
 * Drivers, Third Edition", by Jonathan Corbet, Alessandro Rubini, and
 * Greg Kroah-Hartman, published by O'Reilly Media, Inc.
 */

#include <linux/console.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/sysrq.h>
#include <linux/termios.h>
#include <asm/hv_driver.h>
#include <hv/hypervisor.h>
#include <hv/drv_softuart_intf.h>

static DEFINE_SPINLOCK(softuart_callback_lock);

#define MAX_SOFTUART_DEVICES 1

struct softuart_private {
	struct tty_struct *tty;         /* Passed from the tty layer */
	spinlock_t lock;
	struct hv_driver_cb driver_cb;  /* Driver callback for client int */
};

static int softuart_fd;

/*
 * Open the descriptors to the HV end of the device.
 */
static int softuart_open_hv(int devnum)
{
	char filename[32];

	sprintf(filename, "softuart/%d", devnum);
	softuart_fd = hv_dev_open((HV_VirtAddr)filename, 0);
	if (softuart_fd < 0) {
		pr_err("softuart: Couldn't open %s.\n", filename);
		return -ENODEV;
	}
	return 0;
}

/*
 * Pass the downcall handle to the HV so it can interrupt us.
 */
static int softuart_configure_downcall(u32 intarg)
{
	return hv_dev_pwrite(softuart_fd, 0, (HV_VirtAddr)&intarg,
			     sizeof(u32), SOFTUART_INTARG);
}

/* Called with callback_lock held */
static int softuart_do_receive_chars(struct tty_struct *tty)
{
	int len = 0;
	char buf[1024];
	int n;

	/* Need to check to see if anything's available */
	len = hv_dev_pread(softuart_fd, 0, (HV_VirtAddr)(buf),
			   1024, SOFTUART_DATA_READ);
	if (len > 0) {
		for (n = 0; n < len; n++)
			tty_insert_flip_char(tty, buf[n], 0);
		tty_schedule_flip(tty);
	} else if (len < 0)
		pr_err("softuart:  Couldn't read from HV: %d\n", len);

	return len;
}

/*
 * This asks the HV how much transmit buffer space is available.
 */
static int softuart_write_room(struct tty_struct *tty)
{
	u32 write_room;
	int len;

	len = hv_dev_pread(softuart_fd, 0, (HV_VirtAddr)(&write_room),
			   sizeof(u32), SOFTUART_WRITEROOM);

	if (len != sizeof(u32)) {
		pr_err("softuart:  Unable to read from HV: %d\n", len);
		write_room = 0;
	}

	return write_room;
}

/*
 * This is called when the HV needs to signal that receive data is
 * available to read or that transmit buffer space has become
 * available.
 */
static void softuart_downcall(struct hv_driver_cb *cb, __hv32 reason)
{
	unsigned long flags;

	local_irq_save(flags);
	if (spin_trylock(&softuart_callback_lock)) {
		struct softuart_private *softuartp =
			(struct softuart_private *)cb->dev;
		struct tty_struct *tty = softuartp->tty;
		if (tty) {
			if (reason == SOFTUART_CLIENTINT_RX) {
				/*
				 * Some data has been received by the
				 * HV driver and is ready to read.
				 */
				softuart_do_receive_chars(tty);
			} else if (reason == SOFTUART_CLIENTINT_WRITEROOM) {
				/*
				 * If the output buffer had filled up then the
				 * tty has been put to sleep.  The HV has
				 * indicated that there's buffer space
				 * available, so we wake the tty up.
				 */
				if (waitqueue_active(&tty->write_wait) &&
				    softuart_write_room(tty))
					tty_wakeup(tty);
			}
		}
		spin_unlock(&softuart_callback_lock);
	}
	local_irq_restore(flags);
}

/*
 * If the termios OPOST bit is set then the write_room method will be
 * called to find out how much space should be passed in one write.
 * If the OPOST bit is not set then softuart_write will be called with
 * as much as the higher level has to write, and any unwritten data
 * will be immediately retried.
 */
static int softuart_write(struct tty_struct *tty,
	     const unsigned char *buf, int count)
{
	struct softuart_private *softuartp = tty->driver_data;
	unsigned long flags;
	int retval = 0;

	spin_lock_irqsave(&softuartp->lock, flags);

	retval = hv_dev_pwrite(softuart_fd, 0, (HV_VirtAddr)buf, count, 0);
	if (retval < 0) {
		pr_err("softuart:  Unable to write to HV: %d\n", retval);
		count = 0;
	}

	spin_unlock_irqrestore(&softuartp->lock, flags);

	return retval;
}

static int softuart_get_private_struct(struct softuart_private **ps)
{
	static struct softuart_private *softuartp;
	static DEFINE_SPINLOCK(softuartp_lock);
	unsigned long flags;
	int retval = 0;

	if (softuartp == NULL) {
		struct softuart_private *new_softuartp =
			kmalloc(sizeof(*softuartp), GFP_KERNEL);

		spin_lock_irqsave(&softuartp_lock, flags);
		if (softuartp != NULL) {
			/* We lost a race */
			kfree(new_softuartp);
		} else if (new_softuartp == NULL) {
			retval = -ENOMEM;
		} else {
			new_softuartp->tty = NULL;
			spin_lock_init(&new_softuartp->lock);
			mb();  /* barrier before exposing content */
			softuartp = new_softuartp;
		}

		spin_unlock_irqrestore(&softuartp_lock, flags);
	}

	*ps = softuartp;
	return retval;
}

static int softuart_open(struct tty_struct *tty, struct file *filp)
{
	struct softuart_private *softuartp;
	unsigned long flags;
	int retval = 0;

	retval = softuart_get_private_struct(&softuartp);
	if (retval)
		return retval;

	spin_lock_irqsave(&softuartp->lock, flags);

	retval = softuart_open_hv(tty->index);
	if (retval < 0)
		pr_err("softuart:  Couldn't open HV device: %d.\n", retval);

	if (!softuartp->tty) {
		tty->driver_data = softuartp;

		softuartp->tty = tty;

		/* Make the tty raw. */
		tty->termios->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
					|INLCR|IGNCR|ICRNL|IXON);
		tty->termios->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
		tty->termios->c_cflag &= ~(CSIZE|PARENB);
		tty->termios->c_cflag |= CS8;

		softuartp->driver_cb.dev = softuartp;
		softuartp->driver_cb.callback = softuart_downcall;
		softuart_configure_downcall((u32)&softuartp->driver_cb);
	}

	spin_unlock_irqrestore(&softuartp->lock, flags);

	return retval;
}

/*
 * The HV driver could still be transmitting buffered data after this
 * call.
 */
static void softuart_close(struct tty_struct *tty, struct file *filp)
{
	struct softuart_private *softuartp = tty->driver_data;
	unsigned long flags;

	if (!softuartp) /* This means the open failed */
		return;

	spin_lock_irqsave(&softuartp->lock, flags);

	if (tty->count == 1) {
		/* This is the last device close. */
		softuartp->tty = NULL;
		softuart_configure_downcall(0);
	}

	spin_unlock_irqrestore(&softuartp->lock, flags);

	hv_dev_close(softuart_fd);
}

static struct tty_driver *softuart_driver;

static const struct tty_operations softuart_ops = {
	.open           = softuart_open,
	.close          = softuart_close,
	.write          = softuart_write,
	.write_room     = softuart_write_room,
};

static int __init softuart_init(void)
{
	struct tty_driver *driver;
	int err;

	driver = alloc_tty_driver(MAX_SOFTUART_DEVICES);
	if (!driver)
		return -ENOMEM;
	driver->driver_name = "softuart";
	driver->name = "softuart";
	driver->major = 0;      /* dynamic */
	driver->minor_start = 0;
	driver->type = TTY_DRIVER_TYPE_SYSTEM;
	driver->subtype = SYSTEM_TYPE_SYSCONS;
	driver->init_termios = tty_std_termios;
	tty_set_operations(driver, &softuart_ops);
	err = tty_register_driver(driver);
	if (err) {
		put_tty_driver(driver);
		return err;
	}
	softuart_driver = driver;

	return 0;
}

module_init(softuart_init);
