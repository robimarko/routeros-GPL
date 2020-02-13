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
#include "gxio/iorpc_pka.h"

struct cfg_cmd_queue_empty_interrupt_param {
	union iorpc_interrupt interrupt;
	int ring;
};

int gxio_mica_pka_cfg_cmd_queue_empty_interrupt(gxio_mica_pka_context_t *
						context, int intr_x, int intr_y,
						int intr_ipi, int intr_event,
						int ring)
{
	struct cfg_cmd_queue_empty_interrupt_param temp;
	struct cfg_cmd_queue_empty_interrupt_param *params = &temp;

	params->interrupt.kernel.x = intr_x;
	params->interrupt.kernel.y = intr_y;
	params->interrupt.kernel.ipi = intr_ipi;
	params->interrupt.kernel.event = intr_event;
	params->ring = ring;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params),
			     GXIO_MICA_PKA_OP_CFG_CMD_QUEUE_EMPTY_INTERRUPT);
}

EXPORT_SYMBOL(gxio_mica_pka_cfg_cmd_queue_empty_interrupt);

struct cfg_res_queue_full_interrupt_param {
	union iorpc_interrupt interrupt;
	int ring;
};

int gxio_mica_pka_cfg_res_queue_full_interrupt(gxio_mica_pka_context_t *
					       context, int intr_x, int intr_y,
					       int intr_ipi, int intr_event,
					       int ring)
{
	struct cfg_res_queue_full_interrupt_param temp;
	struct cfg_res_queue_full_interrupt_param *params = &temp;

	params->interrupt.kernel.x = intr_x;
	params->interrupt.kernel.y = intr_y;
	params->interrupt.kernel.ipi = intr_ipi;
	params->interrupt.kernel.event = intr_event;
	params->ring = ring;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params),
			     GXIO_MICA_PKA_OP_CFG_RES_QUEUE_FULL_INTERRUPT);
}

EXPORT_SYMBOL(gxio_mica_pka_cfg_res_queue_full_interrupt);

struct get_mmio_base_param {
	HV_PTE base;
};

int gxio_mica_pka_get_mmio_base(gxio_mica_pka_context_t * context, HV_PTE *base)
{
	int __result;
	struct get_mmio_base_param temp;
	struct get_mmio_base_param *params = &temp;

	__result =
	    hv_dev_pread(context->fd, 0, (HV_VirtAddr) params, sizeof(*params),
			 GXIO_MICA_PKA_OP_GET_MMIO_BASE);
	*base = params->base;

	return __result;
}

EXPORT_SYMBOL(gxio_mica_pka_get_mmio_base);

struct check_mmio_offset_param {
	unsigned long offset;
	unsigned long size;
};

int gxio_mica_pka_check_mmio_offset(gxio_mica_pka_context_t * context,
				    unsigned long offset, unsigned long size)
{
	struct check_mmio_offset_param temp;
	struct check_mmio_offset_param *params = &temp;

	params->offset = offset;
	params->size = size;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params),
			     GXIO_MICA_PKA_OP_CHECK_MMIO_OFFSET);
}

EXPORT_SYMBOL(gxio_mica_pka_check_mmio_offset);
