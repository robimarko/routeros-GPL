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
 * Tile watchdog driver
 *
 * This source code is derived from the following driver:
 *
 * Watchdog driver for Atmel AT91RM9200 (Thunder)
 *
 *  Copyright (C) 2003 SAN People (Pty) Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 */

#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/watchdog.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <hv/hypervisor.h>
#include <hv/drv_watchdog_intf.h>

MODULE_LICENSE("GPL");

#define DRV_NAME		"tile_wdt"

/* Default timeout = 20 seconds */
#define WDT_DEFAULT_TIME	20

static int wdt_time = WDT_DEFAULT_TIME;
module_param(wdt_time, int, 0);
MODULE_PARM_DESC(wdt_time, "Watchdog time in seconds. "
	"(default="__MODULE_STRING(WDT_DEFAULT_TIME) ")");

/*
 * If CONFIG_WATCHDOG_NOWAYOUT is defined,  there is no way
 * to disable the watchdog.
 */
static int nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started "
	"(default=" __MODULE_STRING(WATCHDOG_NOWAYOUT) ")");

/*
 * The HV watchdog device is in the format of "watchdog/N",
 * where N is the instance number.
 */
static char watchdog_device[16] = "watchdog/0";

/* Handle for hypervisor device. */
static int watchdog_hv_devhdl;

/* Serialize the watchdog access. */
static spinlock_t wdt_lock;

/* Watchdog active flag. */
static int wdt_armed;

/*
 * The watchdog platform device.
 */
static struct platform_device *wdt_platform_device;

/* The watchdog can only be opened once. */
static unsigned long tile_wdt_busy;

/*
 * This is the variable to record if the magic character 'V' has been sent to
 * /dev/watchdog just before closing the file. If 'V' is written, the watchdog
 * will be disabled when /dev/watchdog is closed. For details, see
 * Documentation/watchdog/watchdog-api.txt
 */
static char expect_close;

/*
 * Disable the watchdog.
 */
static int tile_wdt_stop(void)
{
	int ret = 0;
	int hv_ret;
	int dummy;

	spin_lock(&wdt_lock);

	hv_ret = hv_dev_pwrite(watchdog_hv_devhdl, 0, (HV_VirtAddr)&dummy,
			sizeof(dummy), WATCHDOG_DISABLE_OFF);
	if (hv_ret < 0) {
		pr_err("Tile watchdog: hv_dev_pwrite(WATCHDOG_DISABLE_OFF)"
		       " failed, error %d\n", hv_ret);
		ret = -EIO;
	}

	wdt_armed = 0;

	spin_unlock(&wdt_lock);

	return ret;
}

/*
 * Enable and start the watchdog.
 */
static int tile_wdt_start(int countdown)
{
	int ret = 0;
	int hv_ret;

	spin_lock(&wdt_lock);

	hv_ret = hv_dev_pwrite(watchdog_hv_devhdl, 0, (HV_VirtAddr)&countdown,
			sizeof(countdown), WATCHDOG_ENABLE_OFF);
	if (hv_ret < 0) {
		pr_err("Tile watchdog: hv_dev_pwrite(WATCHDOG_ENABLE_OFF)"
		       " failed, error %d\n", hv_ret);
		ret = -EIO;
		goto out;
	}
	hv_ret = hv_dev_pread(watchdog_hv_devhdl, 0, (HV_VirtAddr)&wdt_time,
			sizeof(wdt_time), WATCHDOG_GET_COUNTDOWN_OFF);
	if (hv_ret < 0) {
		pr_err("Tile watchdog:"
		       " hv_dev_pread(WATCHDOG_GET_COUNTDOWN_OFF)"
		       " failed, error %d\n", hv_ret);
		ret = -EIO;
	}

	wdt_armed = 1;

out:
	spin_unlock(&wdt_lock);
	return ret;
}

static void noop(void *data) { }

/*
 * Reload the watchdog timer, ie, pat the watchdog.
 */
static int tile_wdt_reload(void)
{
	int ret = 0;
	int hv_ret;
	int dummy;
	
	on_each_cpu(noop, NULL, 1);

	spin_lock(&wdt_lock);

	hv_ret = hv_dev_pwrite(watchdog_hv_devhdl, 0, (HV_VirtAddr)&dummy,
			sizeof(dummy), WATCHDOG_PAT_OFF);
	if (hv_ret < 0) {
		pr_err("Tile watchdog: hv_dev_pwrite(WATCHDOG_PAT_OFF) failed,"
		       " error %d\n", hv_ret);
		ret = -EIO;
	}

	spin_unlock(&wdt_lock);

	return ret;
}

/*
 * tile_wdt_open() - Device open routine.
 * @inode: Inode for this device.
 * @filp: File for this specific open of the device.
 *
 * Watchdog device is opened, and watchdog starts running.
 * Returns zero, or an error code.
 */
static int tile_wdt_open(struct inode *inode, struct file *file)
{
	int ret = 0;

	/* Watchdog can only be opened once. */
	if (test_and_set_bit(0, &tile_wdt_busy))
		return -EBUSY;

	/* Reload and activate timer. */
	ret = tile_wdt_start(wdt_time);
	if (ret < 0) {
		clear_bit(0, &tile_wdt_busy);
		return ret;
	}
	return nonseekable_open(inode, file);
}

/*
 * Close the watchdog device.
 */
static int tile_wdt_release(struct inode *inode, struct file *file)
{
	int ret = 0;

	/*
	 * Disable the watchdog if 'V' is written to /dev/watchdog.
	 */
	if (expect_close == 42) {
		ret = tile_wdt_stop();
	} else if (wdt_armed) {
		pr_crit("Tilera watchdog:"
			" Unexpected close, not stopping watchdog!\n");
		ret = tile_wdt_reload();
	}

	clear_bit(0, &tile_wdt_busy);
	expect_close = 0;
	return ret;
}

static struct watchdog_info tile_wdt_info = {
	.identity	= DRV_NAME,
	.options	= (WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING |
			   WDIOF_MAGICCLOSE),
};

#define RESET_CAUSE_IOCTL _IO('R', 1)

#define RESET_COLD	0
#define RESET_WARM	1
#define RESET_WATCHDOG	2

static int get_reset_cause(void) {
	int ret, hv_ret;
	unsigned long cause = 0;

	spin_lock(&wdt_lock);
	hv_ret = hv_dev_pread(watchdog_hv_devhdl, 0, (HV_VirtAddr)&cause,
			sizeof(cause), WATCHDOG_GET_CAUSE);
	printk("hv_ret = %d, cause = %016lx\n", hv_ret, cause);
	if (hv_ret < 0) {
		ret = -1;
	}
	else if (cause == 0x5741544348444F47) {
		ret = RESET_WATCHDOG;
	}
	else if (cause == 0x0) {
		ret = RESET_COLD;
	}
	else if (cause == 0x1) {
		ret = RESET_WARM;
	}
	else {
		ret = -1;
	}

	spin_unlock(&wdt_lock);

	return ret;
}

/*
 * Handle commands from user-space.
 */
static long tile_wdt_ioctl(struct file *file,
			   unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int __user *p = argp;
	int new_value;
	long ret = 0;

	switch (cmd) {
	case RESET_CAUSE_IOCTL:
		return get_reset_cause();
	case WDIOC_KEEPALIVE:
		ret = tile_wdt_reload();
		if (ret < 0)
			return ret;
		return 0;

	case WDIOC_GETSUPPORT:
		return copy_to_user(argp, &tile_wdt_info,
				    sizeof(tile_wdt_info)) ? -EFAULT : 0;

	case WDIOC_SETTIMEOUT:
		if (get_user(new_value, p))
			return -EFAULT;

		/* Start timer and possibly modify wdt_time. */
		ret = tile_wdt_start(new_value);
		if (ret < 0)
			return ret;

		/* Return current value */
		return put_user(wdt_time, p);

	case WDIOC_GETTIMEOUT:
		return put_user(wdt_time, p);

	case WDIOC_GETSTATUS:
	case WDIOC_GETBOOTSTATUS:
		return put_user(0, p);

	case WDIOC_SETOPTIONS:
		if (get_user(new_value, p))
			return -EFAULT;

		if (new_value & WDIOS_DISABLECARD) {
			ret = tile_wdt_stop();
			if (ret < 0)
				return ret;
		}
		if (new_value & WDIOS_ENABLECARD) {
			ret = tile_wdt_start(wdt_time);
			if (ret < 0)
				return ret;
		}
		return 0;

	default:
		return -ENOTTY;
	}
}

#ifdef CONFIG_COMPAT
static long tile_wdt_compat_ioctl(struct file *filp,
				  unsigned int a, unsigned long b)
{
	/* Sign-extend the argument so it can be used as a pointer. */
	return tile_wdt_ioctl(filp, a, (int)(long)b);
}
#endif

/*
 * Pat the watchdog whenever device is written to.
 * If CONFIG_WATCHDOG_NOWAYOUT is defined,  there is no way
 * to disabling the watchdog.
 */
static ssize_t tile_wdt_write(struct file *file, const char __user *data,
			      size_t len, loff_t *ppos)
{
	int ret;
	/* See if we got the magic character 'V' and reload the timer */
	if (len) {
		if (!nowayout) {
			size_t i;

			expect_close = 0;

			for (i = 0; i != len; i++) {
				char c;
				if (get_user(c, data + i))
					return -EFAULT;
				if (c == 'V')
					expect_close = 42;
			}
		}

		ret = tile_wdt_reload();
		if (ret < 0)
			return ret;
	}
	return len;
}

static const struct file_operations tile_wdt_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.unlocked_ioctl	= tile_wdt_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= tile_wdt_compat_ioctl,
#endif
	.open		= tile_wdt_open,
	.release	= tile_wdt_release,
	.write		= tile_wdt_write,
};

static struct miscdevice tile_wdt_miscdev = {
	.minor		= WATCHDOG_MINOR,
	.name		= "watchdog",
	.fops		= &tile_wdt_fops,
};

static int __devinit tile_wdt_probe(struct platform_device *pdev)
{
	int res;

	if (tile_wdt_miscdev.parent)
		return -EBUSY;
	tile_wdt_miscdev.parent = &pdev->dev;

	/* Open the HV watchdog device if it exists. */
	watchdog_hv_devhdl = hv_dev_open((HV_VirtAddr)watchdog_device, 0);
	if (watchdog_hv_devhdl < 0) {

		switch (watchdog_hv_devhdl) {
		case HV_ENODEV:
			return -ENODEV;
		default:
			return (ssize_t)watchdog_hv_devhdl;
		}
	}
	spin_lock_init(&wdt_lock);

	res = misc_register(&tile_wdt_miscdev);
	if (res) {
		pr_err("Tile watchdog: cannot register miscdev, err=%d\n",
		       res);
		return res;
	}

	pr_info("Tilera watchdog found."
		" Default timeout=%d sec, nowayout=%d\n", wdt_time, nowayout);

	return 0;
}

static int __devexit tile_wdt_remove(struct platform_device *pdev)
{
	int res;

	/* Stop the timer before we leave, if needed. */
	if (!nowayout) {
		res = tile_wdt_stop();
		if (res < 0)
			return res;
	}

	res = misc_deregister(&tile_wdt_miscdev);
	if (!res)
		tile_wdt_miscdev.parent = NULL;

	return res;
}

static void tile_wdt_shutdown(struct platform_device *pdev)
{
	tile_wdt_stop();
}

static struct platform_driver tile_wdt_driver = {
	.probe		= tile_wdt_probe,
	.remove		= __devexit_p(tile_wdt_remove),
	.shutdown	= tile_wdt_shutdown,
	.driver		= {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init tile_wdt_init(void)
{
	int err;

	err = platform_driver_register(&tile_wdt_driver);
	if (err)
		return err;

	wdt_platform_device =
		platform_device_register_simple(DRV_NAME, -1, NULL, 0);
	if (IS_ERR(wdt_platform_device)) {
		err = PTR_ERR(wdt_platform_device);
		goto unreg_platform_driver;
	}

	return 0;

unreg_platform_driver:
	platform_driver_unregister(&tile_wdt_driver);
	return err;
}

static void __exit tile_wdt_exit(void)
{
	platform_device_unregister(wdt_platform_device);
	platform_driver_unregister(&tile_wdt_driver);
}

module_init(tile_wdt_init);
module_exit(tile_wdt_exit);
