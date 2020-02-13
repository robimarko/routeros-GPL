/*******************************************************************************
Copyright (C) 2013 Annapurna Labs Ltd.

This file may be licensed under the terms of the Annapurna Labs Commercial
License Agreement.

Alternatively, this file can be distributed under the terms of the GNU General
Public License V2 as published by the Free Software Foundation and can be
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
 * @defgroup group_udma_interrupts UDMA I/O Fabric Interrupt Controller
 * @ingroup group_udma_api
 *  UDMA IOFIC API
 *  @{
 * @file   al_hal_udma_iofic.h
 *
 * @brief C Header file for programming the interrupt controller that found
 * in UDMA based units. These APIs rely and use some the Interrupt controller
 * API under al_hal_iofic.h
 */

#ifndef __AL_HAL_UDMA_IOFIC_H__
#define __AL_HAL_UDMA_IOFIC_H__

#include "al_hal_common.h"
#include "al_hal_iofic.h"
#include "al_hal_udma_regs.h"

/* *INDENT-OFF* */
#ifdef __cplusplus
extern "C" {
#endif
/* *INDENT-ON* */

	/** interrupt mode */
enum al_iofic_mode {
	AL_IOFIC_MODE_LEGACY, /**< level-sensitive interrupt wire */
	AL_IOFIC_MODE_MSIX_PER_Q, /**< per UDMA queue MSI-X interrupt */
	AL_IOFIC_MODE_MSIX_PER_GROUP
};

/** interrupt controller level (primary/secondary) */
enum al_udma_iofic_level {
	AL_UDMA_IOFIC_LEVEL_PRIMARY,
	AL_UDMA_IOFIC_LEVEL_SECONDARY
};

#define AL_INT_GROUP_A		0 /**< summary of the below events */
#define AL_INT_GROUP_B		1 /**< RX completion queues */
#define AL_INT_GROUP_C		2 /**< TX completion queues */
#define AL_INT_GROUP_D		3 /**< Misc */

/* Group A bits which are just summary bits of GROUP B, C and D */
#define AL_INT_GROUP_A_GROUP_B_SUM	AL_BIT(0)
#define AL_INT_GROUP_A_GROUP_C_SUM	AL_BIT(1)
#define AL_INT_GROUP_A_GROUP_D_SUM	AL_BIT(2)

/* MSIX entries definitions */
#define AL_INT_MSIX_GROUP_A_SUM_D_IDX 	2
#define AL_INT_MSIX_RX_COMPLETION_START	3

/* Group D bits */
#define AL_INT_GROUP_D_CROSS_MAIL_BOXES	\
			(AL_BIT(0) | AL_BIT(1) | AL_BIT(2) | AL_BIT(3))
#define AL_INT_GROUP_D_M2S	AL_BIT(8)
#define AL_INT_GROUP_D_S2M	AL_BIT(9)
#define AL_INT_GROUP_D_SW_TIMER_INT	AL_BIT(10)
#define AL_INT_GROUP_D_APP_EXT_INT	AL_BIT(11)
#define AL_INT_GROUP_D_ALL			\
			AL_INT_GROUP_D_CROSS_MAIL_BOXES | \
			AL_INT_GROUP_D_M2S | \
			AL_INT_GROUP_D_S2M | \
			AL_INT_GROUP_D_SW_TIMER_INT | \
			AL_INT_GROUP_D_APP_EXT_INT

/*
 * Configurations
 */

/**
 * configure the UDMA interrupt controller registers, interrupts will are kept masked
 *
 * @param regs pointer to unit registers
 * @param mode interrupt scheme mode (legacy, MSI-X..)
 * @param m2s_errors_disable mask of m2s errors to disable. recommended value:0 (enable all errors).
 * @param m2s_aborts_disable mask of m2s aborts to disable. recommended value:0 (enable all aborts).
 * @param s2m_errors_disable mask of s2m errors to disable. recommended value:0xE0 (disable hint errors).
 * @param s2m_aborts_disable mask of s2m aborts to disable. recommended value:0xE0 (disable hint aborts).
 *
 * @return 0 on success. -EINVAL otherwise.
 */
int al_udma_iofic_config(struct unit_regs __iomem *regs,
			enum al_iofic_mode mode,
			uint32_t	m2s_errors_disable,
			uint32_t	m2s_aborts_disable,
			uint32_t	s2m_errors_disable,
			uint32_t	s2m_aborts_disable);
/**
 * return the offset of the unmask register for a given group.
 * this function can be used when the upper layer wants to directly
 * access the unmask regiter and bypass the al_udma_iofic_unmask() API.
 *
 * @param regs pointer to udma registers
 * @param group the interrupt group
 * @return the offset of the unmask register.
 */
uint32_t __iomem * al_udma_iofic_unmask_offset_get(struct unit_regs __iomem *regs, int group);

/**
 * unmask specific interrupts for a given group
 * this functions uses the interrupt mask clear register to guarantee atomicity
 * it's safe to call it while the mask is changed by the HW (auto mask) or another cpu.
 *
 * @param regs pointer to udma unit registers
 * @param group the interrupt group
 * @param mask bitwise of interrupts to unmask, set bits will be unmasked.
 */
static INLINE void al_udma_iofic_unmask(
	struct unit_regs __iomem *regs,
	int group,
	uint32_t mask)
{
	al_iofic_unmask(&regs->gen.interrupt_regs.main_iofic, group, mask);
}

/**
 * mask specific interrupts for a given group
 * this functions modifies interrupt mask register, the callee must make sure
 * the mask is not changed by another cpu.
 *
 * @param regs pointer to udma unit registers
 * @param group the interrupt group
 * @param mask bitwise of interrupts to mask, set bits will be masked.
 */
static INLINE void al_udma_iofic_mask(struct unit_regs __iomem *regs, int group, uint32_t mask)
{
	al_iofic_mask(&regs->gen.interrupt_regs.main_iofic, group, mask);
}

/**
 * read interrupt cause register for a given group
 * this will clear the set bits if the Clear on Read mode enabled.
 * @param regs pointer to udma unit registers
 * @param group the interrupt group
 */
static INLINE uint32_t al_udma_iofic_read_cause(struct unit_regs __iomem *regs, int group)
{
	return al_iofic_read_cause(&regs->gen.interrupt_regs.main_iofic, group);
}


#endif
/** @} end of UDMA group */
