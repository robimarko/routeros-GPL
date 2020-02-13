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
#include "gxio/iorpc_trio.h"

struct dealloc_asid_param {
	unsigned int asid;
};

int gxio_trio_dealloc_asid(gxio_trio_context_t *context, unsigned int asid)
{
	struct dealloc_asid_param temp;
	struct dealloc_asid_param *params = &temp;

	params->asid = asid;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_DEALLOC_ASID);
}

EXPORT_SYMBOL(gxio_trio_dealloc_asid);

struct alloc_asids_param {
	unsigned int count;
	unsigned int first;
	unsigned int flags;
};

int gxio_trio_alloc_asids(gxio_trio_context_t *context, unsigned int count,
			  unsigned int first, unsigned int flags)
{
	struct alloc_asids_param temp;
	struct alloc_asids_param *params = &temp;

	params->count = count;
	params->first = first;
	params->flags = flags;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_ALLOC_ASIDS);
}

EXPORT_SYMBOL(gxio_trio_alloc_asids);

#ifndef TILERA_PUBLIC

struct unregister_page_aux_param {
	union iorpc_mem_buffer buffer;
	unsigned int stack;
	uint64_t vpn;
};

int gxio_trio_unregister_page_aux(gxio_trio_context_t *context, void *page_va,
				  size_t page_size, unsigned int page_flags,
				  unsigned int stack, uint64_t vpn)
{
	int __result;
	unsigned long long __cpa;
	pte_t __pte;
	struct unregister_page_aux_param temp;
	struct unregister_page_aux_param *params = &temp;

	__result = va_to_cpa_and_pte(page_va, &__cpa, &__pte);
	if (__result != 0)
		return __result;
	params->buffer.kernel.cpa = __cpa;
	params->buffer.kernel.size = page_size;
	params->buffer.kernel.pte = __pte;
	params->buffer.kernel.flags = page_flags;
	params->stack = stack;
	params->vpn = vpn;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_UNREGISTER_PAGE_AUX);
}

EXPORT_SYMBOL(gxio_trio_unregister_page_aux);

struct register_page_aux_param {
	union iorpc_mem_buffer buffer;
	unsigned int stack;
	uint64_t vpn;
};

int gxio_trio_register_page_aux(gxio_trio_context_t *context, void *page_va,
				size_t page_size, unsigned int page_flags,
				unsigned int stack, uint64_t vpn)
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
	params->stack = stack;
	params->vpn = vpn;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_REGISTER_PAGE_AUX);
}

EXPORT_SYMBOL(gxio_trio_register_page_aux);

#endif /* TILERA_PUBLIC */

struct alloc_memory_maps_param {
	unsigned int count;
	unsigned int first;
	unsigned int flags;
};

int gxio_trio_alloc_memory_maps(gxio_trio_context_t *context,
				unsigned int count, unsigned int first,
				unsigned int flags)
{
	struct alloc_memory_maps_param temp;
	struct alloc_memory_maps_param *params = &temp;

	params->count = count;
	params->first = first;
	params->flags = flags;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_ALLOC_MEMORY_MAPS);
}

EXPORT_SYMBOL(gxio_trio_alloc_memory_maps);

#ifndef TILERA_PUBLIC

struct free_memory_map_aux_param {
	unsigned int map;
};

int gxio_trio_free_memory_map_aux(gxio_trio_context_t *context,
				  unsigned int map)
{
	struct free_memory_map_aux_param temp;
	struct free_memory_map_aux_param *params = &temp;

	params->map = map;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_FREE_MEMORY_MAP_AUX);
}

EXPORT_SYMBOL(gxio_trio_free_memory_map_aux);

struct init_memory_map_aux_param {
	unsigned int map;
	uint64_t vpn;
	uint64_t size;
	unsigned int asid;
	unsigned int mac;
	uint64_t bus_address;
	unsigned int order_mode;
};

int gxio_trio_init_memory_map_aux(gxio_trio_context_t *context,
				  unsigned int map, uint64_t vpn, uint64_t size,
				  unsigned int asid, unsigned int mac,
				  uint64_t bus_address, unsigned int order_mode)
{
	struct init_memory_map_aux_param temp;
	struct init_memory_map_aux_param *params = &temp;

	params->map = map;
	params->vpn = vpn;
	params->size = size;
	params->asid = asid;
	params->mac = mac;
	params->bus_address = bus_address;
	params->order_mode = order_mode;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_INIT_MEMORY_MAP_AUX);
}

EXPORT_SYMBOL(gxio_trio_init_memory_map_aux);

struct read_isr_status_aux_param {
	unsigned int vec_num;
};

int gxio_trio_read_isr_status_aux(gxio_trio_context_t *context,
				  unsigned int vec_num)
{
	struct read_isr_status_aux_param temp;
	struct read_isr_status_aux_param *params = &temp;

	params->vec_num = vec_num;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_READ_ISR_STATUS_AUX);
}

EXPORT_SYMBOL(gxio_trio_read_isr_status_aux);

struct write_isr_status_aux_param {
	unsigned int vec_num;
	uint32_t bits_to_clear;
};

int gxio_trio_write_isr_status_aux(gxio_trio_context_t *context,
				   unsigned int vec_num, uint32_t bits_to_clear)
{
	struct write_isr_status_aux_param temp;
	struct write_isr_status_aux_param *params = &temp;

	params->vec_num = vec_num;
	params->bits_to_clear = bits_to_clear;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params),
			     GXIO_TRIO_OP_WRITE_ISR_STATUS_AUX);
}

EXPORT_SYMBOL(gxio_trio_write_isr_status_aux);

struct enable_mmi_param {
	union iorpc_interrupt interrupt;
	unsigned int map;
	unsigned int mode;
};

int gxio_trio_enable_mmi(gxio_trio_context_t *context, int bind_x, int bind_y,
			 int bind_ipi, int bind_event, unsigned int map,
			 unsigned int mode)
{
	struct enable_mmi_param temp;
	struct enable_mmi_param *params = &temp;

	params->interrupt.kernel.x = bind_x;
	params->interrupt.kernel.y = bind_y;
	params->interrupt.kernel.ipi = bind_ipi;
	params->interrupt.kernel.event = bind_event;
	params->map = map;
	params->mode = mode;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_ENABLE_MMI);
}

EXPORT_SYMBOL(gxio_trio_enable_mmi);

struct mask_mmi_aux_param {
	unsigned int map;
	unsigned int mask;
};

int gxio_trio_mask_mmi_aux(gxio_trio_context_t *context, unsigned int map,
			   unsigned int mask)
{
	struct mask_mmi_aux_param temp;
	struct mask_mmi_aux_param *params = &temp;

	params->map = map;
	params->mask = mask;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_MASK_MMI_AUX);
}

EXPORT_SYMBOL(gxio_trio_mask_mmi_aux);

struct unmask_mmi_aux_param {
	unsigned int map;
	unsigned int mask;
};

int gxio_trio_unmask_mmi_aux(gxio_trio_context_t *context, unsigned int map,
			     unsigned int mask)
{
	struct unmask_mmi_aux_param temp;
	struct unmask_mmi_aux_param *params = &temp;

	params->map = map;
	params->mask = mask;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_UNMASK_MMI_AUX);
}

EXPORT_SYMBOL(gxio_trio_unmask_mmi_aux);

struct read_mmi_bits_aux_param {
	unsigned int map;
};

int gxio_trio_read_mmi_bits_aux(gxio_trio_context_t *context, unsigned int map)
{
	struct read_mmi_bits_aux_param temp;
	struct read_mmi_bits_aux_param *params = &temp;

	params->map = map;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_READ_MMI_BITS_AUX);
}

EXPORT_SYMBOL(gxio_trio_read_mmi_bits_aux);

struct write_mmi_bits_aux_param {
	unsigned int map;
	unsigned int bits;
	unsigned int mode;
};

int gxio_trio_write_mmi_bits_aux(gxio_trio_context_t *context, unsigned int map,
				 unsigned int bits, unsigned int mode)
{
	struct write_mmi_bits_aux_param temp;
	struct write_mmi_bits_aux_param *params = &temp;

	params->map = map;
	params->bits = bits;
	params->mode = mode;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_WRITE_MMI_BITS_AUX);
}

EXPORT_SYMBOL(gxio_trio_write_mmi_bits_aux);

struct alloc_scatter_queues_param {
	unsigned int count;
	unsigned int first;
	unsigned int flags;
};

int gxio_trio_alloc_scatter_queues(gxio_trio_context_t *context,
				   unsigned int count, unsigned int first,
				   unsigned int flags)
{
	struct alloc_scatter_queues_param temp;
	struct alloc_scatter_queues_param *params = &temp;

	params->count = count;
	params->first = first;
	params->flags = flags;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params),
			     GXIO_TRIO_OP_ALLOC_SCATTER_QUEUES);
}

EXPORT_SYMBOL(gxio_trio_alloc_scatter_queues);

struct free_scatter_queue_aux_param {
	unsigned int queue;
};

int gxio_trio_free_scatter_queue_aux(gxio_trio_context_t *context,
				     unsigned int queue)
{
	struct free_scatter_queue_aux_param temp;
	struct free_scatter_queue_aux_param *params = &temp;

	params->queue = queue;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params),
			     GXIO_TRIO_OP_FREE_SCATTER_QUEUE_AUX);
}

EXPORT_SYMBOL(gxio_trio_free_scatter_queue_aux);

struct init_scatter_queue_aux_param {
	unsigned int queue;
	uint64_t size;
	unsigned int asid;
	unsigned int mac;
	uint64_t bus_address;
	unsigned int order_mode;
};

int gxio_trio_init_scatter_queue_aux(gxio_trio_context_t *context,
				     unsigned int queue, uint64_t size,
				     unsigned int asid, unsigned int mac,
				     uint64_t bus_address,
				     unsigned int order_mode)
{
	struct init_scatter_queue_aux_param temp;
	struct init_scatter_queue_aux_param *params = &temp;

	params->queue = queue;
	params->size = size;
	params->asid = asid;
	params->mac = mac;
	params->bus_address = bus_address;
	params->order_mode = order_mode;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params),
			     GXIO_TRIO_OP_INIT_SCATTER_QUEUE_AUX);
}

EXPORT_SYMBOL(gxio_trio_init_scatter_queue_aux);

struct enable_sqi_param {
	union iorpc_interrupt interrupt;
	unsigned int queue;
};

int gxio_trio_enable_sqi(gxio_trio_context_t *context, int bind_x, int bind_y,
			 int bind_ipi, int bind_event, unsigned int queue)
{
	struct enable_sqi_param temp;
	struct enable_sqi_param *params = &temp;

	params->interrupt.kernel.x = bind_x;
	params->interrupt.kernel.y = bind_y;
	params->interrupt.kernel.ipi = bind_ipi;
	params->interrupt.kernel.event = bind_event;
	params->queue = queue;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_ENABLE_SQI);
}

EXPORT_SYMBOL(gxio_trio_enable_sqi);

#endif /* TILERA_PUBLIC */

struct alloc_pio_regions_param {
	unsigned int count;
	unsigned int first;
	unsigned int flags;
};

int gxio_trio_alloc_pio_regions(gxio_trio_context_t *context,
				unsigned int count, unsigned int first,
				unsigned int flags)
{
	struct alloc_pio_regions_param temp;
	struct alloc_pio_regions_param *params = &temp;

	params->count = count;
	params->first = first;
	params->flags = flags;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_ALLOC_PIO_REGIONS);
}

EXPORT_SYMBOL(gxio_trio_alloc_pio_regions);

#ifndef TILERA_PUBLIC

struct free_pio_region_aux_param {
	unsigned int pio_region;
};

int gxio_trio_free_pio_region_aux(gxio_trio_context_t *context,
				  unsigned int pio_region)
{
	struct free_pio_region_aux_param temp;
	struct free_pio_region_aux_param *params = &temp;

	params->pio_region = pio_region;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_FREE_PIO_REGION_AUX);
}

EXPORT_SYMBOL(gxio_trio_free_pio_region_aux);

#endif /* TILERA_PUBLIC */

struct init_pio_region_aux_param {
	unsigned int pio_region;
	unsigned int mac;
	uint32_t bus_address_hi;
	unsigned int flags;
};

int gxio_trio_init_pio_region_aux(gxio_trio_context_t *context,
				  unsigned int pio_region, unsigned int mac,
				  uint32_t bus_address_hi, unsigned int flags)
{
	struct init_pio_region_aux_param temp;
	struct init_pio_region_aux_param *params = &temp;

	params->pio_region = pio_region;
	params->mac = mac;
	params->bus_address_hi = bus_address_hi;
	params->flags = flags;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_INIT_PIO_REGION_AUX);
}

EXPORT_SYMBOL(gxio_trio_init_pio_region_aux);

#ifndef TILERA_PUBLIC

struct free_push_dma_ring_aux_param {
	unsigned int ring;
};

int gxio_trio_free_push_dma_ring_aux(gxio_trio_context_t *context,
				     unsigned int ring)
{
	struct free_push_dma_ring_aux_param temp;
	struct free_push_dma_ring_aux_param *params = &temp;

	params->ring = ring;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params),
			     GXIO_TRIO_OP_FREE_PUSH_DMA_RING_AUX);
}

EXPORT_SYMBOL(gxio_trio_free_push_dma_ring_aux);

struct alloc_push_dma_ring_param {
	unsigned int count;
	unsigned int first;
	unsigned int flags;
};

int gxio_trio_alloc_push_dma_ring(gxio_trio_context_t *context,
				  unsigned int count, unsigned int first,
				  unsigned int flags)
{
	struct alloc_push_dma_ring_param temp;
	struct alloc_push_dma_ring_param *params = &temp;

	params->count = count;
	params->first = first;
	params->flags = flags;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_ALLOC_PUSH_DMA_RING);
}

EXPORT_SYMBOL(gxio_trio_alloc_push_dma_ring);

struct free_pull_dma_ring_aux_param {
	unsigned int ring;
};

int gxio_trio_free_pull_dma_ring_aux(gxio_trio_context_t *context,
				     unsigned int ring)
{
	struct free_pull_dma_ring_aux_param temp;
	struct free_pull_dma_ring_aux_param *params = &temp;

	params->ring = ring;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params),
			     GXIO_TRIO_OP_FREE_PULL_DMA_RING_AUX);
}

EXPORT_SYMBOL(gxio_trio_free_pull_dma_ring_aux);

struct alloc_pull_dma_ring_param {
	unsigned int count;
	unsigned int first;
	unsigned int flags;
};

int gxio_trio_alloc_pull_dma_ring(gxio_trio_context_t *context,
				  unsigned int count, unsigned int first,
				  unsigned int flags)
{
	struct alloc_pull_dma_ring_param temp;
	struct alloc_pull_dma_ring_param *params = &temp;

	params->count = count;
	params->first = first;
	params->flags = flags;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_ALLOC_PULL_DMA_RING);
}

EXPORT_SYMBOL(gxio_trio_alloc_pull_dma_ring);

struct init_push_dma_ring_aux_param {
	union iorpc_mem_buffer buffer;
	unsigned int ring;
	unsigned int mac;
	unsigned int asid;
	unsigned int flags;
};

int gxio_trio_init_push_dma_ring_aux(gxio_trio_context_t *context, void *mem_va,
				     size_t mem_size, unsigned int mem_flags,
				     unsigned int ring, unsigned int mac,
				     unsigned int asid, unsigned int flags)
{
	int __result;
	unsigned long long __cpa;
	pte_t __pte;
	struct init_push_dma_ring_aux_param temp;
	struct init_push_dma_ring_aux_param *params = &temp;

	__result = va_to_cpa_and_pte(mem_va, &__cpa, &__pte);
	if (__result != 0)
		return __result;
	params->buffer.kernel.cpa = __cpa;
	params->buffer.kernel.size = mem_size;
	params->buffer.kernel.pte = __pte;
	params->buffer.kernel.flags = mem_flags;
	params->ring = ring;
	params->mac = mac;
	params->asid = asid;
	params->flags = flags;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params),
			     GXIO_TRIO_OP_INIT_PUSH_DMA_RING_AUX);
}

EXPORT_SYMBOL(gxio_trio_init_push_dma_ring_aux);

struct enable_push_dma_isr_param {
	union iorpc_interrupt interrupt;
	unsigned int ring;
};

int gxio_trio_enable_push_dma_isr(gxio_trio_context_t *context, int bind_x,
				  int bind_y, int bind_ipi, int bind_event,
				  unsigned int ring)
{
	struct enable_push_dma_isr_param temp;
	struct enable_push_dma_isr_param *params = &temp;

	params->interrupt.kernel.x = bind_x;
	params->interrupt.kernel.y = bind_y;
	params->interrupt.kernel.ipi = bind_ipi;
	params->interrupt.kernel.event = bind_event;
	params->ring = ring;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_ENABLE_PUSH_DMA_ISR);
}

EXPORT_SYMBOL(gxio_trio_enable_push_dma_isr);

struct init_pull_dma_ring_aux_param {
	union iorpc_mem_buffer buffer;
	unsigned int ring;
	unsigned int mac;
	unsigned int asid;
	unsigned int flags;
};

int gxio_trio_init_pull_dma_ring_aux(gxio_trio_context_t *context, void *mem_va,
				     size_t mem_size, unsigned int mem_flags,
				     unsigned int ring, unsigned int mac,
				     unsigned int asid, unsigned int flags)
{
	int __result;
	unsigned long long __cpa;
	pte_t __pte;
	struct init_pull_dma_ring_aux_param temp;
	struct init_pull_dma_ring_aux_param *params = &temp;

	__result = va_to_cpa_and_pte(mem_va, &__cpa, &__pte);
	if (__result != 0)
		return __result;
	params->buffer.kernel.cpa = __cpa;
	params->buffer.kernel.size = mem_size;
	params->buffer.kernel.pte = __pte;
	params->buffer.kernel.flags = mem_flags;
	params->ring = ring;
	params->mac = mac;
	params->asid = asid;
	params->flags = flags;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params),
			     GXIO_TRIO_OP_INIT_PULL_DMA_RING_AUX);
}

EXPORT_SYMBOL(gxio_trio_init_pull_dma_ring_aux);

struct enable_pull_dma_isr_param {
	union iorpc_interrupt interrupt;
	unsigned int ring;
};

int gxio_trio_enable_pull_dma_isr(gxio_trio_context_t *context, int bind_x,
				  int bind_y, int bind_ipi, int bind_event,
				  unsigned int ring)
{
	struct enable_pull_dma_isr_param temp;
	struct enable_pull_dma_isr_param *params = &temp;

	params->interrupt.kernel.x = bind_x;
	params->interrupt.kernel.y = bind_y;
	params->interrupt.kernel.ipi = bind_ipi;
	params->interrupt.kernel.event = bind_event;
	params->ring = ring;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_ENABLE_PULL_DMA_ISR);
}

EXPORT_SYMBOL(gxio_trio_enable_pull_dma_isr);

struct register_client_memory_param {
	unsigned int iotlb;
	HV_PTE pte;
	unsigned int flags;
};

int gxio_trio_register_client_memory(gxio_trio_context_t *context,
				     unsigned int iotlb, HV_PTE pte,
				     unsigned int flags)
{
	struct register_client_memory_param temp;
	struct register_client_memory_param *params = &temp;

	params->iotlb = iotlb;
	params->pte = pte;
	params->flags = flags;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params),
			     GXIO_TRIO_OP_REGISTER_CLIENT_MEMORY);
}

EXPORT_SYMBOL(gxio_trio_register_client_memory);

#endif /* TILERA_PUBLIC */

struct init_memory_map_mmu_aux_param {
	unsigned int map;
	unsigned long va;
	uint64_t size;
	unsigned int asid;
	unsigned int mac;
	uint64_t bus_address;
	unsigned int node;
	unsigned int order_mode;
};

int gxio_trio_init_memory_map_mmu_aux(gxio_trio_context_t *context,
				      unsigned int map, unsigned long va,
				      uint64_t size, unsigned int asid,
				      unsigned int mac, uint64_t bus_address,
				      unsigned int node,
				      unsigned int order_mode)
{
	struct init_memory_map_mmu_aux_param temp;
	struct init_memory_map_mmu_aux_param *params = &temp;

	params->map = map;
	params->va = va;
	params->size = size;
	params->asid = asid;
	params->mac = mac;
	params->bus_address = bus_address;
	params->node = node;
	params->order_mode = order_mode;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params),
			     GXIO_TRIO_OP_INIT_MEMORY_MAP_MMU_AUX);
}

EXPORT_SYMBOL(gxio_trio_init_memory_map_mmu_aux);

struct get_port_property_param {
	struct pcie_trio_ports_property trio_ports;
};

int gxio_trio_get_port_property(gxio_trio_context_t *context,
				struct pcie_trio_ports_property *trio_ports)
{
	int __result;
	struct get_port_property_param temp;
	struct get_port_property_param *params = &temp;

	__result =
	    hv_dev_pread(context->fd, 0, (HV_VirtAddr) params, sizeof(*params),
			 GXIO_TRIO_OP_GET_PORT_PROPERTY);
	*trio_ports = params->trio_ports;

	return __result;
}

EXPORT_SYMBOL(gxio_trio_get_port_property);

struct config_legacy_intr_param {
	union iorpc_interrupt interrupt;
	unsigned int mac;
	unsigned int intx;
};

int gxio_trio_config_legacy_intr(gxio_trio_context_t *context, int inter_x,
				 int inter_y, int inter_ipi, int inter_event,
				 unsigned int mac, unsigned int intx)
{
	struct config_legacy_intr_param temp;
	struct config_legacy_intr_param *params = &temp;

	params->interrupt.kernel.x = inter_x;
	params->interrupt.kernel.y = inter_y;
	params->interrupt.kernel.ipi = inter_ipi;
	params->interrupt.kernel.event = inter_event;
	params->mac = mac;
	params->intx = intx;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_CONFIG_LEGACY_INTR);
}

EXPORT_SYMBOL(gxio_trio_config_legacy_intr);

struct config_msi_intr_param {
	union iorpc_interrupt interrupt;
	unsigned int mac;
	unsigned int mem_map;
	uint64_t mem_map_base;
	uint64_t mem_map_limit;
	unsigned int asid;
};

int gxio_trio_config_msi_intr(gxio_trio_context_t *context, int inter_x,
			      int inter_y, int inter_ipi, int inter_event,
			      unsigned int mac, unsigned int mem_map,
			      uint64_t mem_map_base, uint64_t mem_map_limit,
			      unsigned int asid)
{
	struct config_msi_intr_param temp;
	struct config_msi_intr_param *params = &temp;

	params->interrupt.kernel.x = inter_x;
	params->interrupt.kernel.y = inter_y;
	params->interrupt.kernel.ipi = inter_ipi;
	params->interrupt.kernel.event = inter_event;
	params->mac = mac;
	params->mem_map = mem_map;
	params->mem_map_base = mem_map_base;
	params->mem_map_limit = mem_map_limit;
	params->asid = asid;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_CONFIG_MSI_INTR);
}

EXPORT_SYMBOL(gxio_trio_config_msi_intr);

#ifndef TILERA_PUBLIC

struct config_char_intr_param {
	union iorpc_interrupt interrupt;
	unsigned int mac;
	unsigned int mem_map;
	unsigned int push_dma_ring;
	unsigned int pull_dma_ring;
	pcie_stream_intr_config_sel_t conf;
};

int gxio_trio_config_char_intr(gxio_trio_context_t *context, int inter_x,
			       int inter_y, int inter_ipi, int inter_event,
			       unsigned int mac, unsigned int mem_map,
			       unsigned int push_dma_ring,
			       unsigned int pull_dma_ring,
			       pcie_stream_intr_config_sel_t conf)
{
	struct config_char_intr_param temp;
	struct config_char_intr_param *params = &temp;

	params->interrupt.kernel.x = inter_x;
	params->interrupt.kernel.y = inter_y;
	params->interrupt.kernel.ipi = inter_ipi;
	params->interrupt.kernel.event = inter_event;
	params->mac = mac;
	params->mem_map = mem_map;
	params->push_dma_ring = push_dma_ring;
	params->pull_dma_ring = pull_dma_ring;
	params->conf = conf;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_CONFIG_CHAR_INTR);
}

EXPORT_SYMBOL(gxio_trio_config_char_intr);

#endif /* TILERA_PUBLIC */

struct set_mps_mrs_param {
	uint16_t mps;
	uint16_t mrs;
	unsigned int mac;
};

int gxio_trio_set_mps_mrs(gxio_trio_context_t *context, uint16_t mps,
			  uint16_t mrs, unsigned int mac)
{
	struct set_mps_mrs_param temp;
	struct set_mps_mrs_param *params = &temp;

	params->mps = mps;
	params->mrs = mrs;
	params->mac = mac;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_SET_MPS_MRS);
}

EXPORT_SYMBOL(gxio_trio_set_mps_mrs);

struct force_rc_link_up_param {
	unsigned int mac;
};

int gxio_trio_force_rc_link_up(gxio_trio_context_t *context, unsigned int mac)
{
	struct force_rc_link_up_param temp;
	struct force_rc_link_up_param *params = &temp;

	params->mac = mac;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_FORCE_RC_LINK_UP);
}

EXPORT_SYMBOL(gxio_trio_force_rc_link_up);

struct force_ep_link_up_param {
	unsigned int mac;
};

int gxio_trio_force_ep_link_up(gxio_trio_context_t *context, unsigned int mac)
{
	struct force_ep_link_up_param temp;
	struct force_ep_link_up_param *params = &temp;

	params->mac = mac;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_FORCE_EP_LINK_UP);
}

EXPORT_SYMBOL(gxio_trio_force_ep_link_up);

struct unconfig_sio_mac_err_intr_param {
	unsigned int mac;
};

int gxio_trio_unconfig_sio_mac_err_intr(gxio_trio_context_t *context,
					unsigned int mac)
{
	struct unconfig_sio_mac_err_intr_param temp;
	struct unconfig_sio_mac_err_intr_param *params = &temp;

	params->mac = mac;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params),
			     GXIO_TRIO_OP_UNCONFIG_SIO_MAC_ERR_INTR);
}

EXPORT_SYMBOL(gxio_trio_unconfig_sio_mac_err_intr);

struct config_sio_mac_err_intr_param {
	union iorpc_interrupt interrupt;
	unsigned int mac;
};

int gxio_trio_config_sio_mac_err_intr(gxio_trio_context_t *context, int inter_x,
				      int inter_y, int inter_ipi,
				      int inter_event, unsigned int mac)
{
	struct config_sio_mac_err_intr_param temp;
	struct config_sio_mac_err_intr_param *params = &temp;

	params->interrupt.kernel.x = inter_x;
	params->interrupt.kernel.y = inter_y;
	params->interrupt.kernel.ipi = inter_ipi;
	params->interrupt.kernel.event = inter_event;
	params->mac = mac;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params),
			     GXIO_TRIO_OP_CONFIG_SIO_MAC_ERR_INTR);
}

EXPORT_SYMBOL(gxio_trio_config_sio_mac_err_intr);

struct get_mmio_base_param {
	HV_PTE base;
};

int gxio_trio_get_mmio_base(gxio_trio_context_t *context, HV_PTE *base)
{
	int __result;
	struct get_mmio_base_param temp;
	struct get_mmio_base_param *params = &temp;

	__result =
	    hv_dev_pread(context->fd, 0, (HV_VirtAddr) params, sizeof(*params),
			 GXIO_TRIO_OP_GET_MMIO_BASE);
	*base = params->base;

	return __result;
}

EXPORT_SYMBOL(gxio_trio_get_mmio_base);

struct check_mmio_offset_param {
	unsigned long offset;
	unsigned long size;
};

int gxio_trio_check_mmio_offset(gxio_trio_context_t *context,
				unsigned long offset, unsigned long size)
{
	struct check_mmio_offset_param temp;
	struct check_mmio_offset_param *params = &temp;

	params->offset = offset;
	params->size = size;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_TRIO_OP_CHECK_MMIO_OFFSET);
}

EXPORT_SYMBOL(gxio_trio_check_mmio_offset);
