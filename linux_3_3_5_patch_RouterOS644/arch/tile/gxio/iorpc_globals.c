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
#include "gxio/iorpc_globals.h"

typedef struct {
  HV_PTE base;
} get_mmio_base_param_t;

int __iorpc_get_mmio_base(int fd, HV_PTE *base)
{
  uint64_t __offset;
  int __result;
  get_mmio_base_param_t temp;
  get_mmio_base_param_t *params = &temp;
  size_t __size = sizeof(*params);

  __offset = IORPC_OP_GET_MMIO_BASE;
  __result = hv_dev_pread(fd, 0, (HV_VirtAddr)  params, __size, __offset);
  *base = params->base;

  return __result;
}


EXPORT_SYMBOL(__iorpc_get_mmio_base);

typedef struct {
  unsigned long offset;
  unsigned long size;
} check_mmio_offset_param_t;

int __iorpc_check_mmio_offset(int fd, unsigned long offset, unsigned long size)
{
  uint64_t __offset;
  int __result;
  check_mmio_offset_param_t temp;
  check_mmio_offset_param_t *params = &temp;
  size_t __size = sizeof(*params);

  params->offset = offset;
  params->size = size;

  __offset = IORPC_OP_CHECK_MMIO_OFFSET;
  __result = hv_dev_pwrite(fd, 0, (HV_VirtAddr)  params, __size, __offset);
  return __result;
}


EXPORT_SYMBOL(__iorpc_check_mmio_offset);

