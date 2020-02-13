/*******************************************************************************
Copyright (C) 2013 Annapurna Labs Ltd.

This file may be licensed under the terms of the Annapurna Labs Commercial
License Agreement.

Alternatively, this file can be distributed under the terms of the GNU General
Public License V2 or V3 as published by the Free Software Foundation and can be
found at http://www.gnu.org/licenses/gpl-2.0.html

Alternatively, redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following conditions are
met:

    *     Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.

    *     Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in
	  the documentation and/or other materials provided with the
	  distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

/**
 * @defgroup group_raid_api API
 * @ingroup group_raid
 * RAID API
 * @{
 * @file   al_hal_raid.h
 *
 * @brief Header file for RAID acceleration unit HAL driver
 *
 */

#ifndef __AL_HAL_RAID_H__
#define __AL_HAL_RAID_H__

#include "al_hal_common.h"
#include "al_hal_udma.h"
#include "al_hal_m2m_udma.h"

/* *INDENT-OFF* */
#ifdef __cplusplus
extern "C" {
#endif
/* *INDENT-ON* */

/* PCI Adapter Device/Revision ID */
#define AL_RAID_DEV_ID			0x0021
#define AL_RAID_REV_ID_0	0
#define AL_RAID_REV_ID_1	1

/* RAID DMA private data structure */
struct al_raid_dma {
	uint16_t dev_id; /**<PCI adapter device ID */
	uint8_t rev_id; /**<PCI adapter revision ID */
	struct al_m2m_udma m2m_udma;
	struct raid_accelerator_regs __iomem *app_regs;
};

/* RAID DMA parameters from upper layer */
struct al_raid_dma_params {
	uint16_t dev_id; /**<PCI adapter device ID */
	uint8_t rev_id; /**<PCI adapter revision ID */
	void __iomem *udma_regs_base;
	void __iomem *app_regs_base;
	char *name; /* the upper layer must keep the string area */
	uint8_t num_of_queues;/* number of queues */
	uint8_t init_gf_tables; /* initialize gflog and gfilog tables */
};

enum al_raid_op {
	AL_RAID_OP_MEM_CPY = 0, /* memory copy */
	AL_RAID_OP_MEM_SET, /* memory set with 64b data */
	AL_RAID_OP_MEM_SCRUB, /* memory read */
	AL_RAID_OP_MEM_CMP, /* compare block with 128 bit pattern */
	AL_RAID_OP_NOP, /* no-operation */
	AL_RAID_OP_P_CALC, /* RAID5/6 Parity (xor) calculation */
	AL_RAID_OP_Q_CALC, /* RAID6 Q calculation */
	AL_RAID_OP_PQ_CALC, /* P and Q calculation */
	AL_RAID_OP_P_VAL, /* Parity validation */
	AL_RAID_OP_Q_VAL, /* Q validation */
	AL_RAID_OP_PQ_VAL, /* P and Q validation */
	AL_RAID_PARALLEL_MEM_CPY, /* Parallel memory copy */
};

enum al_raid_op_flags {
	AL_RAID_INTERRUPT = AL_BIT(0), /* enable interrupt when the xaction
					  completes */
	AL_RAID_BARRIER = AL_BIT(1), /* data memory barrier, subsequent xactions
					will be served only when the current one
					completes */
	AL_RAID_SRC_NO_SNOOP = AL_BIT(2), /* set no snoop on source buffers */
	AL_RAID_DEST_NO_SNOOP = AL_BIT(3), /* set no snoop on destination buffers */
};

/* transaction completion status */
#define AL_RAID_P_VAL_ERROR	AL_BIT(0)
#define AL_RAID_Q_VAL_ERROR	AL_BIT(1)
#define	AL_RAID_BUS_PARITY_ERROR	AL_BIT(2)
#define	AL_RAID_SOURCE_LEN_ERROR	AL_BIT(3)
#define	AL_RAID_CMD_DECODE_ERROR	AL_BIT(4)
#define	AL_RAID_INTERNAL_ERROR	AL_BIT(5)

#define	AL_RAID_REDIRECTED_TRANSACTION	AL_BIT(16)
#define	AL_RAID_REDIRECTED_SRC_UDMA	(AL_BIT(13) | AL_BIT(12))
#define	AL_RAID_REDIRECTED_SRC_QUEUE	(AL_BIT(9) | AL_BIT(8))

#define AL_RAID_MAX_SRC_DESCS	20

#define AL_RAID_MAX_DST_DESCS	AL_UDMA_DEFAULT_MAX_ACTN_DESCS


struct al_raid_transaction {
	enum al_raid_op op;
	enum al_raid_op_flags flags;
	struct al_block *srcs_blocks;
	uint32_t num_of_srcs;
	uint32_t total_src_bufs; /* total number of buffers of all source blocks */
	struct al_block *dsts_blocks;
	uint32_t num_of_dsts;
	uint32_t total_dst_bufs; /* total number of buffers of all destination blocks */
	uint32_t tx_descs_count; /* number of tx descriptors created for this */
				/* transaction, this field set by the hal */

	/* the following fields are operation specific */
	uint8_t *coefs; /* RAID6 Q coefficients of source blocks */
	uint8_t q_index; /* RAID6 PQ_VAL: index of q src block, the parity*/
			 /* calculation will ignore that buffer */
	uint32_t data[2]; /* MEM SET data */

	uint32_t mem_set_flags; /* MEM SET special flags, should be zero */
	uint32_t pattern_data[4]; /* MEM CMP pattern data */
	uint32_t pattern_mask[4]; /* MEM CMP pattern mask */
};

/* Init RAID DMA */
int al_raid_dma_init(struct al_raid_dma *raid_dma, struct al_raid_dma_params *params);

/* Initialize the m2s(tx) and s2m(rx) components of the queue */
int al_raid_dma_q_init(struct al_raid_dma *raid_dma, uint32_t qid,
			   struct al_udma_q_params *tx_params,
			   struct al_udma_q_params *rx_params);

/* Set RAID DMA state */
int al_raid_dma_state_set(struct al_raid_dma *dma, enum al_udma_state dma_state);

/* Get udma dma handle, this handle can be used to call misc configuration */
/* functions defined at al_udma_config.h. TODO: add more explanation */
int al_raid_dma_handle_get(struct al_raid_dma *raid_dma, enum al_udma_type type,
			    struct al_udma **udma);

/**
 * Get TX udma handle
 *
 * This handle can be used to call misc configuration functions defined at
 * al_udma_config.h
 *
 * @param	raid_dma
 *
 * @returns	TX UDMA handle upon success, or NULL upon failure
 */
struct al_udma *al_raid_dma_tx_udma_handle_get(
	struct al_raid_dma *raid_dma);

/**
 * Get TX queue handle
 *
 * @param	raid_dma - RAID DMA context
 * @param	qid - Queue ID
 *
 * @returns	TX queue handle upon success, or NULL upon failure
 */
struct al_udma_q *al_raid_dma_tx_queue_handle_get(
	struct al_raid_dma	*raid_dma,
	unsigned int		qid);

/**
 * Get RX udma handle
 *
 * This handle can be used to call misc configuration functions defined at
 * al_udma_config.h
 *
 * @param	raid_dma
 *
 * @returns	RX UDMA handle upon success, or NULL upon failure
 */
struct al_udma *al_raid_dma_rx_udma_handle_get(
	struct al_raid_dma *raid_dma);

/**
 * Get RX queue handle
 *
 * @param	raid_dma - RAID DMA context
 * @param	qid - Queue ID
 *
 * @returns	RX queue handle upon success, or NULL upon failure
 */
struct al_udma_q *al_raid_dma_rx_queue_handle_get(
	struct al_raid_dma	*raid_dma,
	unsigned int		qid);

/* prepare raid transaction */
int al_raid_dma_prepare(struct al_raid_dma *raid_dma, uint32_t qid,
			    struct al_raid_transaction *xaction);

/* add previously prepared transaction to engine */
int al_raid_dma_action(struct al_raid_dma *raid_dma, uint32_t qid,
			   uint32_t tx_descs);

/* get new completed transaction */
int al_raid_dma_completion(struct al_raid_dma *raid_dma, uint32_t qid,
			       uint32_t *comp_status);
/* *INDENT-OFF* */
#ifdef __cplusplus
}
#endif
/* *INDENT-ON* */
/** @} end of RAID group */
#endif		/* __AL_HAL_RAID_H__ */
