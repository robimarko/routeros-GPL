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
 */

/* This file is machine-generated; DO NOT EDIT! */
#ifndef __IORPC_LINUX_RPC_H__
#define __IORPC_LINUX_RPC_H__

#include <hv/iorpc.h>

#include <linux/string.h>
#include <linux/module.h>
#include <asm/pgtable.h>
#include <hv/iorpc.h>


#define IORPC_OP_GET_MMIO_BASE         IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x8000)
#define IORPC_OP_CHECK_MMIO_OFFSET     IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x8001)

int __iorpc_get_mmio_base(int fd, HV_PTE *base);

int __iorpc_check_mmio_offset(int fd, unsigned long offset, unsigned long size);

#endif  /* !__IORPC_LINUX_RPC_H__ */
