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
 * Tilera-specific I2C driver.
 *
 * This source code is derived from the following driver:
 *
 * i2c Support for Atmel's AT91 Two-Wire Interface (TWI)
 *
 * Copyright (C) 2004 Rick Bronson
 * Converted to 2.6 by Andrew Victor <andrew@sanpeople.com>
 *
 * Borrowed heavily from original work by:
 * Copyright (C) 2000 Philip Edelbrock <phil@stimpy.netroedge.com>
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/i2c/pca954x.h>
#include <hv/hypervisor.h>
#include <hv/drv_i2cm_intf.h>

#define DRV_NAME	"i2c-tile"

/* Handle for hypervisor device. */
static int i2cm_hv_devhdl[HV_I2CM_MAX_MASTERS];

static int xfer_msg(struct i2c_adapter *adap, struct i2c_msg *pmsg)
{
	int retval = 0;
	int data_offset = 0;
	int addr = pmsg->addr;
	int length = pmsg->len;
	char *buf = pmsg->buf;

	/* The hypervisor uses 8-bit slave addresses. */
	addr <<= 1;

	while (length) {
		int hv_retval;
		tile_i2c_addr_desc_t hv_offset = {{
			.addr = addr,
			.data_offset = data_offset,
		}};

		int bytes_this_pass = length;
		if (bytes_this_pass > HV_I2CM_CHUNK_SIZE)
			bytes_this_pass = HV_I2CM_CHUNK_SIZE;

		if (pmsg->flags & I2C_M_RD)
			hv_retval = hv_dev_pread(i2cm_hv_devhdl[adap->nr], 0,
					(HV_VirtAddr) buf,
					bytes_this_pass,
					hv_offset.word);
		else
			hv_retval = hv_dev_pwrite(i2cm_hv_devhdl[adap->nr], 0,
					(HV_VirtAddr) buf,
					bytes_this_pass,
					hv_offset.word);
		if (hv_retval < 0) {
			if (hv_retval == HV_ENODEV) {
				pr_err(DRV_NAME "%d: %s failed, invalid I2C"
					" address or access denied.\n",
					adap->nr,
					(pmsg->flags & I2C_M_RD) ?
					"hv_dev_pread" : "hv_dev_pwrite");
				retval = -ENODEV;
			} else
				retval = -EIO;
			break;
		}

		buf += hv_retval;
		data_offset += hv_retval;
		length -= hv_retval;
	}

	return retval;
}

/*
 * Generic I2C master transfer routine.
 */
static int tile_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg *pmsg,
			 int num)
{
	int ret, i;

	for (i = 0; i < num; i++) {
		if (pmsg->len && pmsg->buf) {

			/* We don't support ten bit chip address. */
			if (pmsg->flags & I2C_M_TEN)
				return -EINVAL;

			ret = xfer_msg(adap, pmsg);
			if (ret)
				return ret;

			pmsg++;
		} else
			return -EINVAL;
	}

	return i;
}

/*
 * Return list of supported functionality.
 */
static u32 tile_i2c_functionality(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static const struct i2c_algorithm tile_i2c_algorithm = {
	.master_xfer	= tile_i2c_xfer,
	.functionality	= tile_i2c_functionality,
};

static struct pca954x_platform_mode tile_i2c_plat_modes[4] = {
	{
		.adap_id = 0,
		.deselect_on_exit = 1,
	},
	{
		.adap_id = 0,
		.deselect_on_exit = 1,
	},
	{
		.adap_id = 0,
		.deselect_on_exit = 1,
	},
	{
		.adap_id = 0,
		.deselect_on_exit = 1,
	}
};

static struct pca954x_platform_data tile_i2c_plat_data = {
	.modes = tile_i2c_plat_modes,
	.num_modes = 4,
};


/*
 * This routine is called to register all I2C devices that are connected to
 * the I2C bus. This should be done at arch_initcall time, before declaring
 * the I2C adapter. This function does the following:
 *
 * 1. Retrieve the I2C device lists from the hypervisor, which describe
 *    the sets of available I2C devices for each controller, and build
 *    arrays of struct i2c_board_info.
 *
 * 2. Statically declare these I2C devices by calling
 *    i2c_register_board_info().
 */
static int __init tile_i2c_dev_init(void)
{
	struct i2c_board_info *tile_i2c_devices;
	tile_i2c_desc_t *tile_i2c_desc;
	int i2c_desc_size;
	int i2c_devs = 0;
	int ret;
	int i;
	int instance;

	/*
	 * Later on we'll use a negative device handle as a sign to
	 * not instantiate that controller, so make them all negative
	 * up front in case we return early.
	 */
	for (instance = 0; instance < HV_I2CM_MAX_MASTERS; instance++)
		i2cm_hv_devhdl[instance] = -1;

	/*
	 * Now get the device list for each controller from the hypervisor,
	 * and register them all.
	 */
	for (instance = 0; instance < HV_I2CM_MAX_MASTERS; instance++) {
		char i2cm_device[32];

		/* Open the hypervisor's i2cm device. */
		sprintf(i2cm_device, "i2cm/%d", instance);
		i2cm_hv_devhdl[instance] =
			hv_dev_open((HV_VirtAddr)i2cm_device, 0);
		if (i2cm_hv_devhdl[instance] < 0) {
			if (i2cm_hv_devhdl[instance] == HV_ENODEV)
				printk(KERN_DEBUG DRV_NAME "%d: hypervisor "
				       "device %s not configured\n", instance,
				       i2cm_device);
			else
				printk(KERN_WARNING DRV_NAME "%d: failed to "
				       "open %s: error %d\n", instance,
				       i2cm_device, i2cm_hv_devhdl[instance]);
			continue;
		}

		ret = hv_dev_pread(i2cm_hv_devhdl[instance], 0,
				   (HV_VirtAddr)&i2c_devs, sizeof(i2c_devs),
				   I2C_GET_NUM_DEVS_OFF);
		if (ret <= 0) {
			pr_err(DRV_NAME "%d: hv_dev_pread(I2C_GET_NUM_DEVS_OFF)"
			       " failed, error %d\n", instance, ret);
			return -EIO;
		}

		if (i2c_devs <= 0) {
			/* If no devices, no point in creating the adapter. */
			i2cm_hv_devhdl[instance] = -1;
			continue;
		}

		pr_info(DRV_NAME "%d: detected %d I2C devices.\n",
			instance, i2c_devs);

		i2c_desc_size = i2c_devs * sizeof(tile_i2c_desc_t);
		tile_i2c_desc = kzalloc(i2c_desc_size, GFP_KERNEL);
		if (!tile_i2c_desc)
			return -ENOMEM;

		ret = hv_dev_pread(i2cm_hv_devhdl[instance], 0,
				   (HV_VirtAddr)tile_i2c_desc,
				   i2c_desc_size, I2C_GET_DEV_INFO_OFF);
		if (ret <= 0) {
			pr_err(DRV_NAME ": hv_dev_pread(I2C_GET_DEV_INFO_OFF)"
			       " failed, error %d\n", ret);
			return -EIO;
		}

		i2c_desc_size = i2c_devs * sizeof(struct i2c_board_info);
		tile_i2c_devices = kzalloc(i2c_desc_size, GFP_KERNEL);
		if (!tile_i2c_devices)
			return -ENOMEM;

		for (i = 0; i < i2c_devs; i++) {
			strncpy(tile_i2c_devices[i].type,
				tile_i2c_desc[i].name, I2C_NAME_SIZE);
			/*
			 * The hypervisor uses 8-bit slave addresses,
			 * convert to 7-bit for Linux.
			 */
			tile_i2c_devices[i].addr = tile_i2c_desc[i].addr >> 1;
			if (!strcmp(tile_i2c_devices[i].type, "pca9546")) {
				tile_i2c_devices[i].platform_data = &tile_i2c_plat_data;
			}
		}

		ret = i2c_register_board_info(instance, tile_i2c_devices,
					      i2c_devs);

		kfree(tile_i2c_desc);
		kfree(tile_i2c_devices);

		if (ret)
			return ret;
	}

	return 0;
}
arch_initcall(tile_i2c_dev_init);

/*
 * I2C adapter probe routine which registers the I2C adapter with the I2C core.
 */
static int __devinit tile_i2c_probe(struct platform_device *dev)
{
	struct i2c_adapter *adapter;
	int ret;

	adapter = kzalloc(sizeof(struct i2c_adapter), GFP_KERNEL);
	if (adapter == NULL) {
		ret = -ENOMEM;
		goto malloc_err;
	}

	adapter->owner = THIS_MODULE;
	adapter->nr = dev->id;
	snprintf(adapter->name, sizeof(adapter->name), "%s.%u",
		 dev_name(&dev->dev), adapter->nr);

	adapter->algo = &tile_i2c_algorithm;
	adapter->class = I2C_CLASS_HWMON;
	adapter->dev.parent = &dev->dev;

	ret = i2c_add_numbered_adapter(adapter);
	if (ret < 0) {
		dev_err(&dev->dev, "registration failed\n");
		goto add_adapter_err;
	}

	platform_set_drvdata(dev, adapter);

	return 0;

add_adapter_err:
	kfree(adapter);
malloc_err:
	return ret;
}

/*
 * I2C adapter cleanup routine.
 */
static int __devexit tile_i2c_remove(struct platform_device *dev)
{
	struct i2c_adapter *adapter = platform_get_drvdata(dev);
	int rc;

	rc = i2c_del_adapter(adapter);
	platform_set_drvdata(dev, NULL);

	kfree(adapter);

	return rc;
}

static struct platform_driver tile_i2c_driver = {
	.driver		= {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	},
	.probe		= tile_i2c_probe,
	.remove		= __devexit_p(tile_i2c_remove),
};

/*
 * Driver init routine.
 */
static int __init tile_i2c_init(void)
{
	struct platform_device *dev;
	int err;
	int instance;

	err = platform_driver_register(&tile_i2c_driver);
	if (err)
		return err;

	for (instance = 0; instance < HV_I2CM_MAX_MASTERS; instance++) {
		if (i2cm_hv_devhdl[instance] >= 0) {
			dev = platform_device_register_simple(DRV_NAME,
							      instance,
							      NULL, 0);
			if (IS_ERR(dev)) {
				err = PTR_ERR(dev);
				pr_warning("i2c-tile%d init failed: %d\n",
					   instance, err);
			}
		}
	}

	return 0;
}

/*
 * Driver cleanup routine.
 */
static void __exit tile_i2c_exit(void)
{
	platform_driver_unregister(&tile_i2c_driver);
}

module_init(tile_i2c_init);
module_exit(tile_i2c_exit);
