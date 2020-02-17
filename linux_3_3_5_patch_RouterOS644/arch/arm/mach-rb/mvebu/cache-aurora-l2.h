/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File under the following licensing terms. 
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer. 

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution. 

    *   Neither the name of Marvell nor the names of its contributors may be 
        used to endorse or promote products derived from this software without 
        specific prior written permission. 
    
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY PER_CPU, INPER_CPU, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/


#ifndef __INCmvCpuIfL2Regsh
#define __INCmvCpuIfL2Regsh

#define L2_MAX_COUNTERS	2

/**********************************
* Discovery Level-2 Cache registers
***********************************/

/* Cache Controls and Configurations */
#define L2_CTRL_REG                     0x100
#define L2_AUX_CTRL_REG		        0x104
#define L2_CNTR_CTRL_REG		0x200
#define L2_CNTR_CONFIG_REG(cntrNum)     (0x204 + cntrNum * 0xc)
#define L2_CNTR_VAL_LOW_REG(cntrNum)    (0x208 + cntrNum * 0xc)
#define L2_CNTR_VAL_HIGH_REG(cntrNum)   (0x20c + cntrNum * 0xc)
#define L2_INT_CAUSE_REG         	0x220
#define L2_INT_MASK_REG         	0x224
#define L2_ERR_INJECT_CTRL_REG          0x228
#define L2_ECC_ERR_INJECT_MASK_REG      0x22c
#define WAY_SRAM_CONFIG_REG(wayNum)     (0x500 + wayNum * 0x4)
#define L2_ECC_ERR_COUNT_REG            0x600
#define L2_ECC_ERR_THRESHOLD_REG        0x604
#define L2_ECC_ERR_ADDR_CAPTURE_REG     0x608
#define L2_PARITY_ERR_WAY_SET_LATCH_REG 0x60c
#define L2_ECC_ERR_WAY_SET_LATCH_REG    0x610
#define L2_RANGE_BASE_ADDR_REG(cpuNum)  (0x710 + cpuNum * 0x4)
#define L2_CACHE_SYNC_REG               0x730
#define L2_INVAL_PHY_ADDR_REG           0x770
#define L2_INVAL_RANGE_REG              0x774
#define L2_INVAL_INDEX_WAY_REG          0x778
#define L2_INVAL_WAY_REG                0x77c
#define L2_CLEAN_PHY_ADDR_REG           0x7b0
#define L2_CLEAN_RANGE_REG              0x7b4
#define L2_CLEAN_INDEX_WAY_REG          0x7b8
#define L2_CLEAN_WAY_REG                0x7bc
#define L2_FLUSH_PHY_ADDR_REG           0x7f0
#define L2_FLUSH_RANGE_REG              0x7f4
#define L2_FLUSH_INDEX_WAY_REG          0x7f8
#define L2_FLUSH_WAY_REG                0x7fc
#define L2_DATA_LOC_REG(cpuNum)         (0x900 + cpuNum * 0x8)
#define L2_INST_LOC_REG(cpuNum)         (0x904 + cpuNum * 0x8)
#define L2_PFU_LOCK_REG                 0x980
#define IO_BRIDGE_LOCK_REG              0x984

/*  L2_CTRL_REG (L2CR) */
#define L2CR_ENABLE			BIT0

/*  L2_AUX_CTRL_REG (L2ACR) */
#define L2ACR_FORCE_WRITE_POLICY_OFFSET	0
#define L2ACR_FORCE_WRITE_POLICY_MASK	(0x3 << L2ACR_FORCE_WRITE_POLICY_OFFSET)
#define L2ACR_FORCE_WRITE_POLICY_DIS	(0 << L2ACR_FORCE_WRITE_POLICY_OFFSET)
#define L2ACR_FORCE_WRITE_BACK_POLICY	(1 << L2ACR_FORCE_WRITE_POLICY_OFFSET)
#define L2ACR_FORCE_WRITE_THRO_POLICY	(2 << L2ACR_FORCE_WRITE_POLICY_OFFSET)
#define L2ACR_ASSOCIATIVITY_OFFSET	13
#define L2ACR_ASSOCIATIVITY_MASK	(0xF << L2ACR_ASSOCIATIVITY_OFFSET)
#define L2ACR_ASSOCIATIVITY_4WAY	(3 << L2ACR_ASSOCIATIVITY_OFFSET)
#define L2ACR_ASSOCIATIVITY_8WAY	(7 << L2ACR_ASSOCIATIVITY_OFFSET)
#define L2ACR_WAY_SIZE_OFFSET		17			
#define L2ACR_WAY_SIZE_MASK		(0x7 << L2ACR_WAY_SIZE_OFFSE)
#ifdef CONFIG_ARMADA_XP_REV_Z1
#define L2ACR_WAY_SIZE_16KB		(1 << L2ACR_WAY_SIZE_OFFSET)
#define L2ACR_WAY_SIZE_32KB		(2 << L2ACR_WAY_SIZE_OFFSET)
#define L2ACR_WAY_SIZE_64KB		(3 << L2ACR_WAY_SIZE_OFFSET)
#define L2ACR_WAY_SIZE_128KB		(4 << L2ACR_WAY_SIZE_OFFSET)
#define L2ACR_WAY_SIZE_256KB		(5 << L2ACR_WAY_SIZE_OFFSET)
#define L2ACR_WAY_SIZE_512KB		(6 << L2ACR_WAY_SIZE_OFFSET)
#else  /* A0 and over */
#define L2ACR_WAY_SIZE_16KB		(2 << L2ACR_WAY_SIZE_OFFSET)
#define L2ACR_WAY_SIZE_32KB		(3 << L2ACR_WAY_SIZE_OFFSET)
#define L2ACR_WAY_SIZE_64KB		(4 << L2ACR_WAY_SIZE_OFFSET)
#define L2ACR_WAY_SIZE_128KB		(5 << L2ACR_WAY_SIZE_OFFSET)
#define L2ACR_WAY_SIZE_256KB		(6 << L2ACR_WAY_SIZE_OFFSET)
#define L2ACR_WAY_SIZE_512KB		(7 << L2ACR_WAY_SIZE_OFFSET)
#endif
#define L2ACR_ECC_ENABLE		BIT20
#define L2ACR_PARITY_ENABLE		BIT21
#define L2ACR_INV_EVIC_LINE_UC_ERR	BIT22
#define L2ACR_FORCE_WA_OFFSET		23
#define L2ACR_FORCE_WA_MASK		(0x3 << L2ACR_FORCE_WA_OFFSET)
#define L2ACR_FORCE_WA_REQ_ATTRIB	(0 << L2ACR_FORCE_WA_OFFSET)	/* WA is fixed based on page table attribute */
#define L2ACR_FORCE_WA_NONE		(1 << L2ACR_FORCE_WA_OFFSET)	/* No WA regardless of page table attribute */
#define L2ACR_FORCE_WA_ALL		(2 << L2ACR_FORCE_WA_OFFSET)	/* Always WA regardless of PT attribute */
#define L2ACR_REPLACEMENT_OFFSET	27
#define L2ACR_REPLACEMENT_MASK		(0x3 << L2ACR_REPLACEMENT_OFFSET)
#define L2ACR_REPLACEMENT_TYPE_WAYRR	(0 << L2ACR_REPLACEMENT_OFFSET)
#define L2ACR_REPLACEMENT_TYPE_LFSR	(1 << L2ACR_REPLACEMENT_OFFSET)
#ifdef CONFIG_ARMADA_XP_REV_Z1
#define L2ACR_REPLACEMENT_TYPE_SEMIPLRU	(2 << L2ACR_REPLACEMENT_OFFSET)
#else
#define L2ACR_REPLACEMENT_TYPE_SEMIPLRU	(3 << L2ACR_REPLACEMENT_OFFSET)
#endif

/* L2_CNTR_CTRL_REG (L2CCR) */
#define L2CCR_CPU0_EVENT_ENABLE		(1 << 16)
#define L2CCR_CPU1_EVENT_ENABLE		(1 << 17)
#define L2CCR_FPU_EVENT_ENABLE		(1 << 30)
#define L2CCR_IO_BRIDGE_EVENT_ENABLE	(1 << 31)


extern int __init aurora_l2_init(void __iomem *base);
inline void l2_clean_pa(unsigned int);
//#ifdef CONFIG_CACHE_AURORA_L2
int aurora_l2_pm_enter(void);
int aurora_l2_pm_exit(void);
void auroraL2_flush_all(void);
//#else
//static inline int aurora_l2_pm_enter(void) { return 0;};
//static inline int aurora_l2_pm_exit(void) { return 0;};
//static inline void auroraL2_flush_all(void) {};
//#endif
#endif /* __INCmvCpuIfL2Regsh */
