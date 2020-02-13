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
#ifndef __GXIO_MICA_LINUX_RPC_H__
#define __GXIO_MICA_LINUX_RPC_H__

#include <hv/iorpc.h>

#include <gxio/mica.h>
#include <gxio/kiorpc.h>
#include <linux/string.h>
#include <linux/module.h>
#include <asm/pgtable.h>

#define GXIO_MICA_OP_REGISTER_PAGE_AUX IORPC_OPCODE(IORPC_FORMAT_KERNEL_MEM, 0x2000)
#define GXIO_MICA_OP_CFG_COMPLETION_INTERRUPT IORPC_OPCODE(IORPC_FORMAT_KERNEL_INTERRUPT, 0x2001)
#define GXIO_MICA_OP_REGISTER_CLIENT_MEMORY IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x2002)
#define GXIO_MICA_OP_GET_MMIO_BASE     IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x8000)
#define GXIO_MICA_OP_CHECK_MMIO_OFFSET IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x8001)
#define GXIO_MICA_OP_GET_SYS_STATUS     IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x8002)
#define GXIO_MICA_OP_PAUSE_CTX          IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x8003)

int __gxio_mica_register_page_aux(int fd, void *page_va, size_t page_size,
				  unsigned int page_flags, uint64_t vpn);

int __gxio_mica_cfg_completion_interrupt(int fd, int intr_x, int intr_y,
					 int intr_ipi, int intr_event);

int __gxio_mica_register_client_memory(int fd, unsigned int iotlb, HV_PTE pte,
				       unsigned int flags);

int __gxio_mica_get_mmio_base(int fd, HV_PTE *base);

int __gxio_mica_check_mmio_offset(int fd, unsigned long offset,
				  unsigned long size);

int __gxio_mica_get_sys_status(int fd);

int __gxio_mica_pause_ctx(int fd, int pause);

#endif /* !__GXIO_MICA_LINUX_RPC_H__ */
