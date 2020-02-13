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
 * SPI Flash ROM driver for MTD
 *
 * Based on sst25l.c, which is:
 *
 * Copyright © 2009 Bluewater Systems Ltd
 * Author: Andre Renaud <andre@bluewatersys.com>
 * Author: Ryan Mallon <ryan@bluewatersys.com>
 *
 * Based on m25p80.c
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/sched.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <hv/hypervisor.h>
#include <hv/drv_srom_intf.h>

struct tile_srom_dev {
	int alive;
	struct mtd_info mtd;
	int hv_devhdl;
};

static struct tile_srom_dev *srom_device;

#define to_tile_srom_dev(x) container_of(x, struct tile_srom_dev, mtd)

static int tile_srom_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct tile_srom_dev *dev = to_tile_srom_dev(mtd);
	u32 addr, end;

	/* Sanity checks */
	if (instr->addr + instr->len > mtd->size)
		return -EINVAL;

	if ((u32)instr->len & (mtd->erasesize - 1)) {
		pr_err("tile_srom: erase length invalid: erase size %#x, "
		       "length %#x\n", mtd->erasesize, (u32)instr->len);
		return -EINVAL;
	}

	if ((u32)instr->addr & (mtd->erasesize - 1)) {
		pr_err("tile_srom: erase addr invalid: erase size %#X, "
		       "addr %#x\n", mtd->erasesize, (u32)instr->addr);
		return -EINVAL;
	}

	addr = instr->addr;
	end = addr + instr->len;

	while (addr < end) {
		int hv_retval;
		char dummy = 0;

		hv_retval = hv_dev_pwrite(dev->hv_devhdl, 0,
					  (HV_VirtAddr)&dummy, sizeof(dummy),
					  SROM_ERASE_OFF | (u32) addr);
		if (hv_retval != sizeof(dummy)) {
			pr_debug("hv_dev_pwrite for erase"
			      " failed, error %d\n", hv_retval);
			return -EIO;
		}

		addr += mtd->erasesize;
	}

	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);
	return 0;
}

static int tile_srom_read(struct mtd_info *mtd, loff_t from, size_t len,
			 size_t *retlen, unsigned char *buf)
{
	struct tile_srom_dev *dev = to_tile_srom_dev(mtd);
	int curoff = 0;
	char rd_buf[mtd->writesize];
	int returned = 0;

	/* Sanity checking */
	if (len == 0)
		return 0;

	if (from + len > mtd->size)
		return -EINVAL;

	/* Align to a page, since the HV requires that. */
	if (from & (mtd->writesize - 1)) {
		curoff = from & (mtd->writesize - 1);
		from -= curoff;
	}

	while (len) {
		int thislen;
		int hv_retval;

		/* We always read the same length. */
		hv_retval = hv_dev_pread(dev->hv_devhdl, 0,
					 (HV_VirtAddr) rd_buf,
					 mtd->writesize, from);
		if (hv_retval < mtd->writesize) {
			pr_debug("hv_dev_pread failed,"
			      " error %d\n", hv_retval);
			if (retlen)
				*retlen = returned;
			return -EIO;
		}

		/* (We can do the copy length computation afterwards.) */
		thislen = (mtd->writesize - curoff > len)
			     ? len
			     : mtd->writesize - curoff;

		/* If this is found to be performance-critical, a copy can
		 * be eliminated by fastpathing the case in which thislen ==
		 * mtd->write_size.  I think that SPI performance will be
		 * the bottleneck here, though, not memory bandwidth...
		 */
		memcpy(buf, rd_buf + curoff, thislen);

		returned += thislen;
		len -= thislen;
		from += mtd->writesize;
		buf += thislen;

		/* We only have an offset once. */
		curoff = 0;
	}

	if (retlen)
		*retlen = returned;

	return 0;
}

static int tile_srom_write(struct mtd_info *mtd, loff_t to, size_t len,
			  size_t *retlen, const unsigned char *buf)
{
	struct tile_srom_dev *dev = to_tile_srom_dev(mtd);
	int curoff = 0;
	int returned = 0;

	/* Sanity checks */
	if (!len)
		return 0;

	if (to + len > mtd->size)
		return -EINVAL;

	curoff = (u32) to & (mtd->writesize - 1);
	to -= curoff;

	while (len) {
		int thislen;
		int hv_retval;
		char wr_buf[mtd->writesize];

		thislen = (mtd->writesize - curoff > len)
			     ? len
			     : mtd->writesize - curoff;

		memset(wr_buf, 0xFF, mtd->writesize);
		memcpy(wr_buf + curoff, buf, thislen);

		hv_retval = hv_dev_pwrite(dev->hv_devhdl, 0,
					  (HV_VirtAddr) wr_buf,
					  mtd->writesize, (u32) to);
		if (hv_retval != mtd->writesize) {
			pr_debug("hv_dev_pwrite failed,"
			      " error %d\n", hv_retval);
			if (retlen)
				*retlen = returned;
			return -EIO;
		}

		len -= thislen;
		buf += thislen;
		returned += thislen;
		to += mtd->writesize;

		/* We only have an offset once. */
		curoff = 0;
	}

	if (retlen)
		*retlen = returned;

	return 0;
}

static int __init tile_srom_setup(struct tile_srom_dev *dev)
{
	int rv;
	const char *fn = "srom/0/mtd";
	u32 tmp;

	/*
	 * No mutex needed, because the hypervisor will serialize accesses
	 * for us.
	 */

	/* Start off dead. */
	dev->alive = 0;

	/* Light it up -- if it's not there, then it's not our problem now. */
	dev->hv_devhdl = hv_dev_open((HV_VirtAddr)fn, 0);
	if (dev->hv_devhdl < 0) {
		switch (dev->hv_devhdl)	{
		case HV_ENODEV:
			pr_info("tile_srom: %s not present\n", fn);
			return -ENODEV;
		default:
			pr_info("tile_srom: open %s failed with other %d\n",
				fn, dev->hv_devhdl);
			return dev->hv_devhdl;
		}
	}

	/* Populate parameters from HV. */
	dev->mtd.name      = "srom";
	dev->mtd.type      = MTD_NORFLASH;
	dev->mtd.flags     = MTD_CAP_NORFLASH;

	hv_dev_pread(dev->hv_devhdl, 0, (HV_VirtAddr)&(dev->mtd.erasesize),
		     sizeof(dev->mtd.erasesize), SROM_SECTOR_SIZE_OFF);
	hv_dev_pread(dev->hv_devhdl, 0, (HV_VirtAddr)&(dev->mtd.writesize),
		     sizeof(dev->mtd.writesize), SROM_PAGE_SIZE_OFF);
	hv_dev_pread(dev->hv_devhdl, 0, (HV_VirtAddr)&tmp,
		     sizeof(tmp), SROM_TOTAL_SIZE_OFF);
	dev->mtd.size = tmp;

	dev->mtd.erase     = tile_srom_erase;
	dev->mtd.read      = tile_srom_read;
	dev->mtd.write     = tile_srom_write;

	pr_debug(
	      "mtd .name = %s, .size = 0x%llx (%lld MiB) "
	      ".erasesize = 0x%.8x (%uKiB) .numeraseregions = %d\n",
	      dev->mtd.name,
	      (long long)dev->mtd.size, (long long)(dev->mtd.size >> 20),
	      dev->mtd.erasesize, dev->mtd.erasesize / 1024,
	      dev->mtd.numeraseregions);

	/* Now that it's all populated... */
	rv = mtd_device_register(&dev->mtd, NULL, 0);
	if (rv != 0)
		return -ENODEV;

	/* And once we've succeeded... */
	dev->alive = 1;
	pr_info("tile_srom: found SROM, %lld bytes\n",
		(long long)dev->mtd.size);

	return 0;
}

static int __init tile_srom_init(void)
{
	/*
	 * Allocate our data structure.
	 */
	srom_device = kzalloc(sizeof(struct tile_srom_dev), GFP_KERNEL);
	if (!srom_device)
		return -ENOMEM;

	/* Set up the device. */
	if (tile_srom_setup(srom_device) != 0) {
		kfree(srom_device);
		return -ENODEV;
	}

	return 0;
}

static void __exit tile_srom_exit(void)
{
	if (srom_device->alive)
		mtd_device_unregister(&srom_device->mtd);

	kfree(srom_device);
}

module_init(tile_srom_init);
module_exit(tile_srom_exit);

MODULE_DESCRIPTION("MTD driver for SROM interface on Tile");
MODULE_AUTHOR("Tilera Corporation");
MODULE_LICENSE("GPL");
