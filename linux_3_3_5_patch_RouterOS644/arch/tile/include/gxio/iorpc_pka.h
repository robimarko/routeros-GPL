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
#ifndef __GXIO_MICA_PKA_LINUX_RPC_H__
#define __GXIO_MICA_PKA_LINUX_RPC_H__

#include <hv/iorpc.h>

#include <gxio/kiorpc.h>
#include <gxio/pka.h>
#include <linux/string.h>
#include <linux/module.h>
#include <asm/pgtable.h>

#define GXIO_MICA_PKA_OP_CFG_CMD_QUEUE_EMPTY_INTERRUPT IORPC_OPCODE(IORPC_FORMAT_KERNEL_INTERRUPT, 0x2200)
#define GXIO_MICA_PKA_OP_CFG_RES_QUEUE_FULL_INTERRUPT IORPC_OPCODE(IORPC_FORMAT_KERNEL_INTERRUPT, 0x2201)
#define GXIO_MICA_PKA_OP_GET_MMIO_BASE IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x8000)
#define GXIO_MICA_PKA_OP_CHECK_MMIO_OFFSET IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x8001)

int gxio_mica_pka_cfg_cmd_queue_empty_interrupt(gxio_mica_pka_context_t *
						context, int intr_x, int intr_y,
						int intr_ipi, int intr_event,
						int ring);

int gxio_mica_pka_cfg_res_queue_full_interrupt(gxio_mica_pka_context_t *
					       context, int intr_x, int intr_y,
					       int intr_ipi, int intr_event,
					       int ring);

int gxio_mica_pka_get_mmio_base(gxio_mica_pka_context_t * context,
				HV_PTE *base);

int gxio_mica_pka_check_mmio_offset(gxio_mica_pka_context_t * context,
				    unsigned long offset, unsigned long size);

#endif /* !__GXIO_MICA_PKA_LINUX_RPC_H__ */
