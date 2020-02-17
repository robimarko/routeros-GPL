/*
 * Copyright 2013 Tilera Corporation. All Rights Reserved.
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
 */

/* This file is machine-generated; DO NOT EDIT! */
#ifndef __GXIO_USB_DEV_LINUX_RPC_H__
#define __GXIO_USB_DEV_LINUX_RPC_H__

#include <hv/iorpc.h>

#include <hv/drv_usb_dev_intf.h>
#include <asm/page.h>
#include <gxio/kiorpc.h>
#include <gxio/usb_dev.h>
#include <linux/string.h>
#include <linux/module.h>
#include <asm/pgtable.h>

#define GXIO_USB_DEV_OP_CFG_INTERRUPT  IORPC_OPCODE(IORPC_FORMAT_KERNEL_INTERRUPT, 0x2100)
#define GXIO_USB_DEV_OP_RESET_INTERRUPT IORPC_OPCODE(IORPC_FORMAT_NONE, 0x2101)
#define GXIO_USB_DEV_OP_GET_MMIO_BASE  IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x8000)
#define GXIO_USB_DEV_OP_CHECK_MMIO_OFFSET IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x8001)

int gxio_usb_dev_cfg_interrupt(gxio_usb_dev_context_t * context, int inter_x,
			       int inter_y, int inter_ipi, int inter_event);

int gxio_usb_dev_reset_interrupt(gxio_usb_dev_context_t * context);

int gxio_usb_dev_get_mmio_base(gxio_usb_dev_context_t * context, HV_PTE *base);

int gxio_usb_dev_check_mmio_offset(gxio_usb_dev_context_t * context,
				   unsigned long offset, unsigned long size);

#endif /* !__GXIO_USB_DEV_LINUX_RPC_H__ */
