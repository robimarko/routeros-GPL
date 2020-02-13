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
#ifndef __GXIO_MPIPE_LINUX_RPC_H__
#define __GXIO_MPIPE_LINUX_RPC_H__

#include <hv/iorpc.h>

#include <hv/drv_mpipe_intf.h>
#include <asm/page.h>
#include <gxio/kiorpc.h>
#include <gxio/mpipe.h>
#include <linux/string.h>
#include <linux/module.h>
#include <asm/pgtable.h>

#define GXIO_MPIPE_OP_ALLOC_BUFFER_STACKS_AUX IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1200)
#define GXIO_MPIPE_OP_INIT_BUFFER_STACK_AUX IORPC_OPCODE(IORPC_FORMAT_KERNEL_MEM, 0x1201)
#ifndef TILERA_PUBLIC

#define GXIO_MPIPE_OP_REGISTER_PAGE_AUX IORPC_OPCODE(IORPC_FORMAT_KERNEL_MEM, 0x1202)

#endif /* TILERA_PUBLIC */

#define GXIO_MPIPE_OP_ALLOC_NOTIF_RINGS IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1203)
#define GXIO_MPIPE_OP_INIT_NOTIF_RING_AUX IORPC_OPCODE(IORPC_FORMAT_KERNEL_MEM, 0x1204)
#define GXIO_MPIPE_OP_REQUEST_NOTIF_RING_INTERRUPT IORPC_OPCODE(IORPC_FORMAT_KERNEL_INTERRUPT, 0x1205)
#define GXIO_MPIPE_OP_ENABLE_NOTIF_RING_INTERRUPT IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1206)
#define GXIO_MPIPE_OP_ALLOC_NOTIF_GROUPS IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1207)
#define GXIO_MPIPE_OP_INIT_NOTIF_GROUP IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1208)
#define GXIO_MPIPE_OP_ALLOC_BUCKETS    IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1209)
#define GXIO_MPIPE_OP_INIT_BUCKET      IORPC_OPCODE(IORPC_FORMAT_NONE, 0x120a)
#define GXIO_MPIPE_OP_ALLOC_EDMA_RINGS IORPC_OPCODE(IORPC_FORMAT_NONE, 0x120b)
#define GXIO_MPIPE_OP_INIT_EDMA_RING_AUX IORPC_OPCODE(IORPC_FORMAT_KERNEL_MEM, 0x120c)
#ifndef TILERA_PUBLIC

#define GXIO_MPIPE_OP_CLASSIFIER_LOAD_FROM_BYTES IORPC_OPCODE(IORPC_FORMAT_NONE, 0x120d)
#define GXIO_MPIPE_OP_CLASSIFIER_SET_MEMORY IORPC_OPCODE(IORPC_FORMAT_NONE, 0x120e)

#endif /* TILERA_PUBLIC */

#define GXIO_MPIPE_OP_COMMIT_RULES     IORPC_OPCODE(IORPC_FORMAT_NONE, 0x120f)
#define GXIO_MPIPE_OP_REGISTER_CLIENT_MEMORY IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x1210)
#define GXIO_MPIPE_OP_LINK_OPEN_AUX    IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1211)
#define GXIO_MPIPE_OP_LINK_CLOSE_AUX   IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1212)
#define GXIO_MPIPE_OP_LINK_SET_ATTR_AUX IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1213)
#ifndef TILERA_PUBLIC

#define GXIO_MPIPE_OP_LINK_GET_ATTR_AUX IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1214)

#endif /* TILERA_PUBLIC */

#define GXIO_MPIPE_OP_LINK_MDIO_RD_AUX IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1215)
#define GXIO_MPIPE_OP_LINK_MDIO_WR_AUX IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1216)
#ifndef TILERA_PUBLIC

#define GXIO_MPIPE_OP_LINK_MAC_RD_AUX  IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1217)
#define GXIO_MPIPE_OP_LINK_MAC_WR_AUX  IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1218)
#define GXIO_MPIPE_OP_GET_SQN          IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1219)
#define GXIO_MPIPE_OP_CONFIG_STATS     IORPC_OPCODE(IORPC_FORMAT_NONE, 0x121a)
#define GXIO_MPIPE_OP_GET_STATS        IORPC_OPCODE(IORPC_FORMAT_NONE, 0x121b)
#define GXIO_MPIPE_OP_GET_COUNTER      IORPC_OPCODE(IORPC_FORMAT_NONE, 0x121c)
#define GXIO_MPIPE_OP_LINK_CFG_POLLFD  IORPC_OPCODE(IORPC_FORMAT_KERNEL_POLLFD_SETUP, 0x121d)

#endif /* TILERA_PUBLIC */

#define GXIO_MPIPE_OP_GET_TIMESTAMP_AUX IORPC_OPCODE(IORPC_FORMAT_NONE, 0x121e)
#define GXIO_MPIPE_OP_SET_TIMESTAMP_AUX IORPC_OPCODE(IORPC_FORMAT_NONE, 0x121f)
#define GXIO_MPIPE_OP_ADJUST_TIMESTAMP_AUX IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1220)
#define GXIO_MPIPE_OP_CONFIG_EDMA_RING_BLKS IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1221)
#define GXIO_MPIPE_OP_ADJUST_TIMESTAMP_FREQ IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1222)
#define GXIO_MPIPE_OP_EDMA_RING_SET_PRIORITY IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1223)
#define GXIO_MPIPE_OP_EDMA_RING_GET_PRIORITY IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1224)
#define GXIO_MPIPE_OP_GET_BUFFER_COUNT IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1225)
#define GXIO_MPIPE_OP_ARM_POLLFD       IORPC_OPCODE(IORPC_FORMAT_KERNEL_POLLFD, 0x9000)
#define GXIO_MPIPE_OP_CLOSE_POLLFD     IORPC_OPCODE(IORPC_FORMAT_KERNEL_POLLFD, 0x9001)
#define GXIO_MPIPE_OP_GET_MMIO_BASE    IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x8000)
#define GXIO_MPIPE_OP_CHECK_MMIO_OFFSET IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x8001)

int gxio_mpipe_alloc_buffer_stacks_aux(gxio_mpipe_context_t *context,
				       unsigned int count, unsigned int first,
				       unsigned int flags);

int gxio_mpipe_init_buffer_stack_aux(gxio_mpipe_context_t *context,
				     void *mem_va, size_t mem_size,
				     unsigned int mem_flags, unsigned int stack,
				     unsigned int buffer_size_enum);

#ifndef TILERA_PUBLIC

int gxio_mpipe_register_page_aux(gxio_mpipe_context_t *context, void *page_va,
				 size_t page_size, unsigned int page_flags,
				 unsigned int stack, uint64_t vpn);

#endif /* TILERA_PUBLIC */

int gxio_mpipe_alloc_notif_rings(gxio_mpipe_context_t *context,
				 unsigned int count, unsigned int first,
				 unsigned int flags);

int gxio_mpipe_init_notif_ring_aux(gxio_mpipe_context_t *context, void *mem_va,
				   size_t mem_size, unsigned int mem_flags,
				   unsigned int ring);

int gxio_mpipe_request_notif_ring_interrupt(gxio_mpipe_context_t *context,
					    int inter_x, int inter_y,
					    int inter_ipi, int inter_event,
					    unsigned int ring);

int gxio_mpipe_enable_notif_ring_interrupt(gxio_mpipe_context_t *context,
					   unsigned int ring);

int gxio_mpipe_alloc_notif_groups(gxio_mpipe_context_t *context,
				  unsigned int count, unsigned int first,
				  unsigned int flags);

int gxio_mpipe_init_notif_group(gxio_mpipe_context_t *context,
				unsigned int group,
				gxio_mpipe_notif_group_bits_t bits);

int gxio_mpipe_alloc_buckets(gxio_mpipe_context_t *context, unsigned int count,
			     unsigned int first, unsigned int flags);

int gxio_mpipe_init_bucket(gxio_mpipe_context_t *context, unsigned int bucket,
			   MPIPE_LBL_INIT_DAT_BSTS_TBL_t bucket_info);

int gxio_mpipe_alloc_edma_rings(gxio_mpipe_context_t *context,
				unsigned int count, unsigned int first,
				unsigned int flags);

int gxio_mpipe_init_edma_ring_aux(gxio_mpipe_context_t *context, void *mem_va,
				  size_t mem_size, unsigned int mem_flags,
				  unsigned int ring, unsigned int channel);

#ifndef TILERA_PUBLIC

int gxio_mpipe_classifier_load_from_bytes(gxio_mpipe_context_t *context,
					  const void *blob, size_t blob_size);

int gxio_mpipe_classifier_set_memory(gxio_mpipe_context_t *context,
				     _gxio_mpipe_symbol_name_t name,
				     const void *blob, size_t blob_size);

#endif /* TILERA_PUBLIC */

int gxio_mpipe_commit_rules(gxio_mpipe_context_t *context, const void *blob,
			    size_t blob_size);

int gxio_mpipe_register_client_memory(gxio_mpipe_context_t *context,
				      unsigned int iotlb, HV_PTE pte,
				      unsigned int flags);

int gxio_mpipe_link_open_aux(gxio_mpipe_context_t *context,
			     _gxio_mpipe_link_name_t name, unsigned int flags);

int gxio_mpipe_link_close_aux(gxio_mpipe_context_t *context, int mac);

int gxio_mpipe_link_set_attr_aux(gxio_mpipe_context_t *context, int mac,
				 uint32_t attr, int64_t val);

#ifndef TILERA_PUBLIC

int gxio_mpipe_link_get_attr_aux(gxio_mpipe_context_t *context,
				 unsigned int off, int64_t *data);

#endif /* TILERA_PUBLIC */

int gxio_mpipe_link_mdio_rd_aux(gxio_mpipe_context_t *context, int mac, int phy,
				int dev, int addr);

int gxio_mpipe_link_mdio_wr_aux(gxio_mpipe_context_t *context, int mac, int phy,
				int dev, int addr, uint16_t val);

#ifndef TILERA_PUBLIC

int gxio_mpipe_link_mac_rd_aux(gxio_mpipe_context_t *context, unsigned int off,
			       int64_t *data);

int gxio_mpipe_link_mac_wr_aux(gxio_mpipe_context_t *context, int mac, int addr,
			       uint32_t val);

int gxio_mpipe_get_sqn(gxio_mpipe_context_t *context, int idx);

int gxio_mpipe_config_stats(gxio_mpipe_context_t *context, uint32_t command,
			    uint64_t val);

int gxio_mpipe_get_stats(gxio_mpipe_context_t *context,
			 gxio_mpipe_stats_t * result);

int gxio_mpipe_get_counter(gxio_mpipe_context_t *context, unsigned int off,
			   uint64_t *result);

int gxio_mpipe_link_cfg_pollfd(gxio_mpipe_context_t *context, int inter_x,
			       int inter_y, int inter_ipi, int inter_event,
			       uint64_t linkmask);

#endif /* TILERA_PUBLIC */

int gxio_mpipe_get_timestamp_aux(gxio_mpipe_context_t *context, uint64_t *sec,
				 uint64_t *nsec, uint64_t *cycles);

int gxio_mpipe_set_timestamp_aux(gxio_mpipe_context_t *context, uint64_t sec,
				 uint64_t nsec, uint64_t cycles);

int gxio_mpipe_adjust_timestamp_aux(gxio_mpipe_context_t *context,
				    int64_t nsec);

int gxio_mpipe_config_edma_ring_blks(gxio_mpipe_context_t *context,
				     unsigned int ering, unsigned int max_blks,
				     unsigned int min_snf_blks,
				     unsigned int db);

int gxio_mpipe_adjust_timestamp_freq(gxio_mpipe_context_t *context,
				     int32_t ppb);

int gxio_mpipe_edma_ring_set_priority(gxio_mpipe_context_t *context,
				      unsigned int ering,
				      unsigned int ering_priority);

int gxio_mpipe_edma_ring_get_priority(gxio_mpipe_context_t *context,
				      unsigned int ering);

int gxio_mpipe_get_buffer_count(gxio_mpipe_context_t *context, uint32_t stack);

int gxio_mpipe_arm_pollfd(gxio_mpipe_context_t *context, int pollfd_cookie);

int gxio_mpipe_close_pollfd(gxio_mpipe_context_t *context, int pollfd_cookie);

int gxio_mpipe_get_mmio_base(gxio_mpipe_context_t *context, HV_PTE *base);

int gxio_mpipe_check_mmio_offset(gxio_mpipe_context_t *context,
				 unsigned long offset, unsigned long size);

#endif /* !__GXIO_MPIPE_LINUX_RPC_H__ */
