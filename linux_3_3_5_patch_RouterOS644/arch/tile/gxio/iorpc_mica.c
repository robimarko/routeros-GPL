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
#include "gxio/iorpc_mica.h"

struct register_page_aux_param {
	union iorpc_mem_buffer buffer;
	uint64_t vpn;
};

int __gxio_mica_register_page_aux(int fd, void *page_va, size_t page_size,
				  unsigned int page_flags, uint64_t vpn)
{
	int __result;
	unsigned long long __cpa;
	pte_t __pte;
	struct register_page_aux_param temp;
	struct register_page_aux_param *params = &temp;

	__result = va_to_cpa_and_pte(page_va, &__cpa, &__pte);
	if (__result != 0)
		return __result;
	params->buffer.kernel.cpa = __cpa;
	params->buffer.kernel.size = page_size;
	params->buffer.kernel.pte = __pte;
	params->buffer.kernel.flags = page_flags;
	params->vpn = vpn;

	return hv_dev_pwrite(fd, 0, (HV_VirtAddr) params, sizeof(*params),
			     GXIO_MICA_OP_REGISTER_PAGE_AUX);
}

EXPORT_SYMBOL(__gxio_mica_register_page_aux);

struct cfg_completion_interrupt_param {
	union iorpc_interrupt interrupt;
};

int __gxio_mica_cfg_completion_interrupt(int fd, int intr_x, int intr_y,
					 int intr_ipi, int intr_event)
{
	struct cfg_completion_interrupt_param temp;
	struct cfg_completion_interrupt_param *params = &temp;

	params->interrupt.kernel.x = intr_x;
	params->interrupt.kernel.y = intr_y;
	params->interrupt.kernel.ipi = intr_ipi;
	params->interrupt.kernel.event = intr_event;

	return hv_dev_pwrite(fd, 0, (HV_VirtAddr) params, sizeof(*params),
			     GXIO_MICA_OP_CFG_COMPLETION_INTERRUPT);
}

EXPORT_SYMBOL(__gxio_mica_cfg_completion_interrupt);

struct register_client_memory_param {
	unsigned int iotlb;
	HV_PTE pte;
	unsigned int flags;
};

int __gxio_mica_register_client_memory(int fd, unsigned int iotlb, HV_PTE pte,
				       unsigned int flags)
{
	struct register_client_memory_param temp;
	struct register_client_memory_param *params = &temp;

	params->iotlb = iotlb;
	params->pte = pte;
	params->flags = flags;

	return hv_dev_pwrite(fd, 0, (HV_VirtAddr) params, sizeof(*params),
			     GXIO_MICA_OP_REGISTER_CLIENT_MEMORY);
}

EXPORT_SYMBOL(__gxio_mica_register_client_memory);

struct get_mmio_base_param {
	HV_PTE base;
};

int __gxio_mica_get_mmio_base(int fd, HV_PTE *base)
{
	int __result;
	struct get_mmio_base_param temp;
	struct get_mmio_base_param *params = &temp;

	__result =
	    hv_dev_pread(fd, 0, (HV_VirtAddr) params, sizeof(*params),
			 GXIO_MICA_OP_GET_MMIO_BASE);
	*base = params->base;

	return __result;
}

EXPORT_SYMBOL(__gxio_mica_get_mmio_base);

struct check_mmio_offset_param {
	unsigned long offset;
	unsigned long size;
};

int __gxio_mica_check_mmio_offset(int fd, unsigned long offset,
				  unsigned long size)
{
	struct check_mmio_offset_param temp;
	struct check_mmio_offset_param *params = &temp;

	params->offset = offset;
	params->size = size;

	return hv_dev_pwrite(fd, 0, (HV_VirtAddr) params, sizeof(*params),
			     GXIO_MICA_OP_CHECK_MMIO_OFFSET);
}

EXPORT_SYMBOL(__gxio_mica_check_mmio_offset);

int __gxio_mica_get_sys_status(int fd)
{
	return hv_dev_pread(fd, 0, (HV_VirtAddr) 0, 0,
			 GXIO_MICA_OP_GET_SYS_STATUS);
}

EXPORT_SYMBOL(__gxio_mica_get_sys_status);

struct pause_ctx_param {
    int pause;
};

int __gxio_mica_pause_ctx(int fd, int pause)
{
	struct pause_ctx_param temp = { pause };
	struct pause_ctx_param *params = &temp;

	return hv_dev_pwrite(fd, 0, (HV_VirtAddr) params, sizeof(*params),
			 GXIO_MICA_OP_PAUSE_CTX);
}

EXPORT_SYMBOL(__gxio_mica_pause_ctx);
