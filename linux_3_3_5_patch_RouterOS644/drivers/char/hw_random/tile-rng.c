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
 * Tilera-specific Hardware random number generator driver.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hw_random.h>
#include <linux/slab.h>

#include <gxio/pka.h>

static int tile_rng_data_present(struct hwrng *rng, int wait)
{
	gxio_mica_pka_trng_context_t* context =
		(gxio_mica_pka_trng_context_t*)rng->priv;

	return gxio_mica_pka_trng_data_present(context);
}

static int tile_rng_data_read(struct hwrng *rng, u32 *data)
{
	int err;
	gxio_mica_pka_trng_context_t* context =
		(gxio_mica_pka_trng_context_t*)rng->priv;

	err = gxio_mica_pka_trng_read32(context, data);
	if (err) {
		printk(KERN_ERR "RNG read failed (%d)\n", err);
		return 0;
	}

	return sizeof(*data);
}

static struct hwrng tile_rng = {
	.name		= "tile",
	.data_present	= tile_rng_data_present,
	.data_read	= tile_rng_data_read,
};

static int __init mod_init(void)
{
	int err;
	int found = 0;
	int i;
	gxio_mica_pka_trng_context_t* rng;

	rng = kzalloc(sizeof(*rng), GFP_KERNEL);
	if (!rng)
		return -ENOMEM;

	for (i = 0; i < HV_PKA_NUM_SHIMS; i++) {
		if (gxio_mica_pka_trng_init(rng, i) == 0)
		{
			found = 1;
			break;
		}
	}
	
	if (!found) {
		kfree(rng);
		return -ENODEV;
	}
	
	tile_rng.priv = (unsigned long)rng;

	err = hwrng_register(&tile_rng);
	if (err) {
		printk(KERN_ERR "RNG registering failed (%d)\n", err);
		return err;
	}

	return 0;
}

static void __exit mod_exit(void)
{
	gxio_mica_pka_trng_context_t* rng = 
		(gxio_mica_pka_trng_context_t*)tile_rng.priv;
	hwrng_unregister(&tile_rng);
	kfree(rng);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_DESCRIPTION("Tilera-specific Hardware Random Number Generator Driver");
MODULE_LICENSE("GPL");
