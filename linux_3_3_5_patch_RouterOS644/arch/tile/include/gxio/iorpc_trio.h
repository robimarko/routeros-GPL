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
#ifndef __GXIO_TRIO_LINUX_RPC_H__
#define __GXIO_TRIO_LINUX_RPC_H__

#include <hv/iorpc.h>

#include <hv/drv_trio_intf.h>
#include <gxio/trio.h>
#include <gxio/kiorpc.h>
#include <linux/string.h>
#include <linux/module.h>
#include <asm/pgtable.h>

#define GXIO_TRIO_OP_DEALLOC_ASID      IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1400)
#define GXIO_TRIO_OP_ALLOC_ASIDS       IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1401)
#ifndef TILERA_PUBLIC

#define GXIO_TRIO_OP_UNREGISTER_PAGE_AUX IORPC_OPCODE(IORPC_FORMAT_KERNEL_MEM, 0x1402)
#define GXIO_TRIO_OP_REGISTER_PAGE_AUX IORPC_OPCODE(IORPC_FORMAT_KERNEL_MEM, 0x1403)

#endif /* TILERA_PUBLIC */

#define GXIO_TRIO_OP_ALLOC_MEMORY_MAPS IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1404)
#ifndef TILERA_PUBLIC

#define GXIO_TRIO_OP_FREE_MEMORY_MAP_AUX IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1405)
#define GXIO_TRIO_OP_INIT_MEMORY_MAP_AUX IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1406)
#define GXIO_TRIO_OP_READ_ISR_STATUS_AUX IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1407)
#define GXIO_TRIO_OP_WRITE_ISR_STATUS_AUX IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1408)
#define GXIO_TRIO_OP_ENABLE_MMI        IORPC_OPCODE(IORPC_FORMAT_KERNEL_INTERRUPT, 0x1409)
#define GXIO_TRIO_OP_MASK_MMI_AUX      IORPC_OPCODE(IORPC_FORMAT_NONE, 0x140a)
#define GXIO_TRIO_OP_UNMASK_MMI_AUX    IORPC_OPCODE(IORPC_FORMAT_NONE, 0x140b)
#define GXIO_TRIO_OP_READ_MMI_BITS_AUX IORPC_OPCODE(IORPC_FORMAT_NONE, 0x140c)
#define GXIO_TRIO_OP_WRITE_MMI_BITS_AUX IORPC_OPCODE(IORPC_FORMAT_NONE, 0x140d)
#define GXIO_TRIO_OP_ALLOC_SCATTER_QUEUES IORPC_OPCODE(IORPC_FORMAT_NONE, 0x140e)
#define GXIO_TRIO_OP_FREE_SCATTER_QUEUE_AUX IORPC_OPCODE(IORPC_FORMAT_NONE, 0x140f)
#define GXIO_TRIO_OP_INIT_SCATTER_QUEUE_AUX IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1410)
#define GXIO_TRIO_OP_ENABLE_SQI        IORPC_OPCODE(IORPC_FORMAT_KERNEL_INTERRUPT, 0x1411)

#endif /* TILERA_PUBLIC */

#define GXIO_TRIO_OP_ALLOC_SCATTER_QUEUES IORPC_OPCODE(IORPC_FORMAT_NONE, 0x140e)
#define GXIO_TRIO_OP_ALLOC_PIO_REGIONS IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1412)
#ifndef TILERA_PUBLIC

#define GXIO_TRIO_OP_FREE_PIO_REGION_AUX IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1413)

#endif /* TILERA_PUBLIC */

#define GXIO_TRIO_OP_INIT_PIO_REGION_AUX IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1414)
#ifndef TILERA_PUBLIC

#define GXIO_TRIO_OP_FREE_PUSH_DMA_RING_AUX IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1415)
#define GXIO_TRIO_OP_ALLOC_PUSH_DMA_RING IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1416)
#define GXIO_TRIO_OP_FREE_PULL_DMA_RING_AUX IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1417)
#define GXIO_TRIO_OP_ALLOC_PULL_DMA_RING IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1418)
#define GXIO_TRIO_OP_INIT_PUSH_DMA_RING_AUX IORPC_OPCODE(IORPC_FORMAT_KERNEL_MEM, 0x1419)
#define GXIO_TRIO_OP_ENABLE_PUSH_DMA_ISR IORPC_OPCODE(IORPC_FORMAT_KERNEL_INTERRUPT, 0x141a)
#define GXIO_TRIO_OP_INIT_PULL_DMA_RING_AUX IORPC_OPCODE(IORPC_FORMAT_KERNEL_MEM, 0x141b)
#define GXIO_TRIO_OP_ENABLE_PULL_DMA_ISR IORPC_OPCODE(IORPC_FORMAT_KERNEL_INTERRUPT, 0x141c)
#define GXIO_TRIO_OP_REGISTER_CLIENT_MEMORY IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x141d)

#endif /* TILERA_PUBLIC */

#define GXIO_TRIO_OP_INIT_MEMORY_MAP_MMU_AUX IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x141e)
#define GXIO_TRIO_OP_GET_PORT_PROPERTY IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x141f)
#define GXIO_TRIO_OP_CONFIG_LEGACY_INTR IORPC_OPCODE(IORPC_FORMAT_KERNEL_INTERRUPT, 0x1420)
#define GXIO_TRIO_OP_CONFIG_MSI_INTR   IORPC_OPCODE(IORPC_FORMAT_KERNEL_INTERRUPT, 0x1421)
#ifndef TILERA_PUBLIC

#define GXIO_TRIO_OP_CONFIG_CHAR_INTR  IORPC_OPCODE(IORPC_FORMAT_KERNEL_INTERRUPT, 0x1422)

#endif /* TILERA_PUBLIC */

#define GXIO_TRIO_OP_SET_MPS_MRS       IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x1423)
#define GXIO_TRIO_OP_FORCE_RC_LINK_UP  IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x1424)
#define GXIO_TRIO_OP_FORCE_EP_LINK_UP  IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x1425)
#define GXIO_TRIO_OP_UNCONFIG_SIO_MAC_ERR_INTR IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1426)
#define GXIO_TRIO_OP_CONFIG_SIO_MAC_ERR_INTR IORPC_OPCODE(IORPC_FORMAT_KERNEL_INTERRUPT, 0x1427)
#define GXIO_TRIO_OP_GET_MMIO_BASE     IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x8000)
#define GXIO_TRIO_OP_CHECK_MMIO_OFFSET IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x8001)

int gxio_trio_dealloc_asid(gxio_trio_context_t *context, unsigned int asid);

int gxio_trio_alloc_asids(gxio_trio_context_t *context, unsigned int count,
			  unsigned int first, unsigned int flags);

#ifndef TILERA_PUBLIC

int gxio_trio_unregister_page_aux(gxio_trio_context_t *context, void *page_va,
				  size_t page_size, unsigned int page_flags,
				  unsigned int stack, uint64_t vpn);

int gxio_trio_register_page_aux(gxio_trio_context_t *context, void *page_va,
				size_t page_size, unsigned int page_flags,
				unsigned int stack, uint64_t vpn);

#endif /* TILERA_PUBLIC */

int gxio_trio_alloc_memory_maps(gxio_trio_context_t *context,
				unsigned int count, unsigned int first,
				unsigned int flags);

#ifndef TILERA_PUBLIC

int gxio_trio_free_memory_map_aux(gxio_trio_context_t *context,
				  unsigned int map);

int gxio_trio_init_memory_map_aux(gxio_trio_context_t *context,
				  unsigned int map, uint64_t vpn, uint64_t size,
				  unsigned int asid, unsigned int mac,
				  uint64_t bus_address,
				  unsigned int order_mode);

int gxio_trio_read_isr_status_aux(gxio_trio_context_t *context,
				  unsigned int vec_num);

int gxio_trio_write_isr_status_aux(gxio_trio_context_t *context,
				   unsigned int vec_num,
				   uint32_t bits_to_clear);

int gxio_trio_enable_mmi(gxio_trio_context_t *context, int bind_x, int bind_y,
			 int bind_ipi, int bind_event, unsigned int map,
			 unsigned int mode);

int gxio_trio_mask_mmi_aux(gxio_trio_context_t *context, unsigned int map,
			   unsigned int mask);

int gxio_trio_unmask_mmi_aux(gxio_trio_context_t *context, unsigned int map,
			     unsigned int mask);

int gxio_trio_read_mmi_bits_aux(gxio_trio_context_t *context, unsigned int map);

int gxio_trio_write_mmi_bits_aux(gxio_trio_context_t *context, unsigned int map,
				 unsigned int bits, unsigned int mode);

int gxio_trio_alloc_scatter_queues(gxio_trio_context_t *context,
				   unsigned int count, unsigned int first,
				   unsigned int flags);

int gxio_trio_free_scatter_queue_aux(gxio_trio_context_t *context,
				     unsigned int queue);

int gxio_trio_init_scatter_queue_aux(gxio_trio_context_t *context,
				     unsigned int queue, uint64_t size,
				     unsigned int asid, unsigned int mac,
				     uint64_t bus_address,
				     unsigned int order_mode);

int gxio_trio_enable_sqi(gxio_trio_context_t *context, int bind_x, int bind_y,
			 int bind_ipi, int bind_event, unsigned int queue);

#endif /* TILERA_PUBLIC */

int gxio_trio_alloc_pio_regions(gxio_trio_context_t *context,
				unsigned int count, unsigned int first,
				unsigned int flags);

#ifndef TILERA_PUBLIC

int gxio_trio_free_pio_region_aux(gxio_trio_context_t *context,
				  unsigned int pio_region);

#endif /* TILERA_PUBLIC */

int gxio_trio_init_pio_region_aux(gxio_trio_context_t *context,
				  unsigned int pio_region, unsigned int mac,
				  uint32_t bus_address_hi, unsigned int flags);

#ifndef TILERA_PUBLIC

int gxio_trio_free_push_dma_ring_aux(gxio_trio_context_t *context,
				     unsigned int ring);

int gxio_trio_alloc_push_dma_ring(gxio_trio_context_t *context,
				  unsigned int count, unsigned int first,
				  unsigned int flags);

int gxio_trio_free_pull_dma_ring_aux(gxio_trio_context_t *context,
				     unsigned int ring);

int gxio_trio_alloc_pull_dma_ring(gxio_trio_context_t *context,
				  unsigned int count, unsigned int first,
				  unsigned int flags);

int gxio_trio_init_push_dma_ring_aux(gxio_trio_context_t *context, void *mem_va,
				     size_t mem_size, unsigned int mem_flags,
				     unsigned int ring, unsigned int mac,
				     unsigned int asid, unsigned int flags);

int gxio_trio_enable_push_dma_isr(gxio_trio_context_t *context, int bind_x,
				  int bind_y, int bind_ipi, int bind_event,
				  unsigned int ring);

int gxio_trio_init_pull_dma_ring_aux(gxio_trio_context_t *context, void *mem_va,
				     size_t mem_size, unsigned int mem_flags,
				     unsigned int ring, unsigned int mac,
				     unsigned int asid, unsigned int flags);

int gxio_trio_enable_pull_dma_isr(gxio_trio_context_t *context, int bind_x,
				  int bind_y, int bind_ipi, int bind_event,
				  unsigned int ring);

int gxio_trio_register_client_memory(gxio_trio_context_t *context,
				     unsigned int iotlb, HV_PTE pte,
				     unsigned int flags);

#endif /* TILERA_PUBLIC */

int gxio_trio_init_memory_map_mmu_aux(gxio_trio_context_t *context,
				      unsigned int map, unsigned long va,
				      uint64_t size, unsigned int asid,
				      unsigned int mac, uint64_t bus_address,
				      unsigned int node,
				      unsigned int order_mode);

int gxio_trio_get_port_property(gxio_trio_context_t *context,
				struct pcie_trio_ports_property *trio_ports);

int gxio_trio_config_legacy_intr(gxio_trio_context_t *context, int inter_x,
				 int inter_y, int inter_ipi, int inter_event,
				 unsigned int mac, unsigned int intx);

int gxio_trio_config_msi_intr(gxio_trio_context_t *context, int inter_x,
			      int inter_y, int inter_ipi, int inter_event,
			      unsigned int mac, unsigned int mem_map,
			      uint64_t mem_map_base, uint64_t mem_map_limit,
			      unsigned int asid);

#ifndef TILERA_PUBLIC

int gxio_trio_config_char_intr(gxio_trio_context_t *context, int inter_x,
			       int inter_y, int inter_ipi, int inter_event,
			       unsigned int mac, unsigned int mem_map,
			       unsigned int push_dma_ring,
			       unsigned int pull_dma_ring,
			       pcie_stream_intr_config_sel_t conf);

#endif /* TILERA_PUBLIC */

int gxio_trio_set_mps_mrs(gxio_trio_context_t *context, uint16_t mps,
			  uint16_t mrs, unsigned int mac);

int gxio_trio_force_rc_link_up(gxio_trio_context_t *context, unsigned int mac);

int gxio_trio_force_ep_link_up(gxio_trio_context_t *context, unsigned int mac);

int gxio_trio_unconfig_sio_mac_err_intr(gxio_trio_context_t *context,
					unsigned int mac);

int gxio_trio_config_sio_mac_err_intr(gxio_trio_context_t *context, int inter_x,
				      int inter_y, int inter_ipi,
				      int inter_event, unsigned int mac);

int gxio_trio_get_mmio_base(gxio_trio_context_t *context, HV_PTE *base);

int gxio_trio_check_mmio_offset(gxio_trio_context_t *context,
				unsigned long offset, unsigned long size);

#endif /* !__GXIO_TRIO_LINUX_RPC_H__ */
