/*
 * SMP support: Entry point for secondary CPUs
 *
 * Copyright (C) 2012 Marvell
 *
 * Yehuda Yitschak <yehuday@marvell.com>
 * Gregory CLEMENT <gregory.clement@free-electrons.com>
 * Thomas Petazzoni <thomas.petazzoni@free-electrons.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 *
 * This file implements the assembly entry point for secondary CPUs in
 * an SMP kernel. The only thing we need to do is to add the CPU to
 * the coherency fabric by writing to 2 registers. Currently the base
 * register addresses are hard coded due to the early initialisation
 * problems.
 */

#include <linux/linkage.h>
#include <linux/init.h>

#include <asm/assembler.h>

/* Select code for any configuration running in BE8 mode */
#ifdef CONFIG_CPU_ENDIAN_BE8
#define ARM_BE8(code...) code
#else
#define ARM_BE8(code...)
#endif

/* Auxiliary Debug Modes Control 1 Register */
#define PJ4B_STATIC_BP (1 << 2) /* Enable Static BP */
#define PJ4B_INTER_PARITY (1 << 8) /* Disable Internal Parity Handling */
#define PJ4B_CLEAN_LINE (1 << 16) /* Disable data transfer for clean line */

/* Auxiliary Debug Modes Control 2 Register */
#define PJ4B_FAST_LDR (1 << 23) /* Disable fast LDR */
#define PJ4B_SNOOP_DATA (1 << 25) /* Do not interleave write and snoop data */
#define PJ4B_CWF (1 << 27) /* Disable Critical Word First feature */
#define PJ4B_OUTSDNG_NC (1 << 29) /* Disable outstanding non cacheable rqst */
#define PJ4B_L1_REP_RR (1 << 30) /* L1 replacement - Strict round robin */
#define PJ4B_AUX_DBG_CTRL2 (PJ4B_SNOOP_DATA | PJ4B_CWF |\
			    PJ4B_OUTSDNG_NC | PJ4B_L1_REP_RR)

/* Auxiliary Functional Modes Control Register 0 */
#define PJ4B_SMP_CFB (1 << 1) /* Set SMP mode. Join the coherency fabric */
#define PJ4B_L1_PAR_CHK (1 << 2) /* Support L1 parity checking */
#define PJ4B_BROADCAST_CACHE (1 << 8) /* Broadcast Cache and TLB maintenance */

/* Auxiliary Debug Modes Control 0 Register */
#define PJ4B_WFI_WFE (1 << 22) /* WFI/WFE - serve the DVM and back to idle */

/*
 * Armada XP specific entry point for secondary CPUs.
 * We add the CPU to the coherency fabric and then jump to secondary
 * startup
 */
ENTRY(armada_xp_secondary_startup)
 ARM_BE8(setend	be )			@ go BE8 if entered LE

	bl	ll_add_cpu_to_smp_group

	bl	ll_enable_coherency

	/* Auxiliary Debug Modes Control 1 Register */
	mrc	p15, 1,	r0, c15, c1, 1
	orr     r0, r0, #PJ4B_CLEAN_LINE
	orr     r0, r0, #PJ4B_INTER_PARITY
	bic	r0, r0, #PJ4B_STATIC_BP
	mcr	p15, 1,	r0, c15, c1, 1

	/* Auxiliary Debug Modes Control 2 Register */
	mrc	p15, 1,	r0, c15, c1, 2
	bic	r0, r0, #PJ4B_FAST_LDR
	orr	r0, r0, #PJ4B_AUX_DBG_CTRL2
	mcr	p15, 1,	r0, c15, c1, 2

	/* Auxiliary Functional Modes Control Register 0 */
	mrc	p15, 1,	r0, c15, c2, 0
#ifdef CONFIG_SMP
	orr	r0, r0, #PJ4B_SMP_CFB
#endif
	orr	r0, r0, #PJ4B_L1_PAR_CHK
	orr	r0, r0, #PJ4B_BROADCAST_CACHE
	mcr	p15, 1,	r0, c15, c2, 0

	/* Auxiliary Debug Modes Control 0 Register */
	mrc	p15, 1,	r0, c15, c1, 0
	orr	r0, r0, #PJ4B_WFI_WFE
	mcr	p15, 1,	r0, c15, c1, 0

	b	secondary_startup

ENDPROC(armada_xp_secondary_startup)
