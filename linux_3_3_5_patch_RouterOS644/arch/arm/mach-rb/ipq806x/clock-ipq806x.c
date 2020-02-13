/* * Copyright (c) 2012 The Linux Foundation. All rights reserved.* */
/* Copyright (c) 2009-2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/ctype.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/clk.h>

#include <asm/clkdev.h>
#include <asm/mach-types.h>

#include <mach/msm_iomap.h>
#include <mach/socinfo.h>

#include "clock.h"
#include "clock-local.h"
#include "clock-rpm.h"
#include "clock-voter.h"
#include "devices.h"
#include "clock-pll.h"

#define REG(off)	(MSM_CLK_CTL_BASE + (off))
#define REG_GCC(off)	(MSM_APCS_GCC_BASE + (off))
#define REG_LPA(off)	(MSM_LPASS_CLK_CTL_BASE + (off))

/* Peripheral clock registers. */
#define GCC_RPU_CR				REG(0x0F80)
#define GCC_RPU_EAR				REG(0x0F84)
#define GCC_RPU_ESR				REG(0x0F88)
#define GCC_RPU_ESRRESTORE			REG(0x0F8C)
#define GCC_RPU_ESYNR0				REG(0x0F90)
#define GCC_RPU_ESYNR1				REG(0x0F94)
#define GCC_RPU_REV				REG(0x0FF4)
#define GCC_RPU_IDR				REG(0x0FF8)
#define GCC_RPU_RPU_ACR				REG(0x0FFC)
#define AFAB_CLK_SRC_ACR			REG(0x1000)
#define QDSS_STM_CLK_ACR			REG(0x100C)
#define AFAB_CORE_CLK_ACR			REG(0x1010)
#define SCSS_ACR				REG(0x1014)
#define AFAB_EBI1_S_ACLK_ACR			REG(0x1018)
#define AFAB_AXI_S_FCLK_ACR			REG(0x1020)
#define SFAB_CORE_CLK_ACR			REG(0x1024)
#define SFAB_AXI_S_FCLK_ACR			REG(0x1028)
#define SFAB_AHB_S_FCLK_ACR			REG(0x102C)
#define QDSS_AT_CLK_ACR				REG(0x1030)
#define QDSS_TRACECLKIN_CLK_ACR			REG(0x1034)
#define QDSS_TSCTR_CLK_ACR			REG(0x1038)
#define SFAB_ADM0_M_ACLK_ACR			REG(0x103C)
#define ADM0_ACR				REG(0x1040)
#define QDSS_RESETS_ACR				REG(0x104C)
#define IMEM0_ACLK_ACR				REG(0x1050)
#define QDSS_HCLK_ACR				REG(0x1054)
#define PCIE_ACR				REG(0x1058)
#define SFAB_CLK_SRC_ACR			REG(0x105C)
#define SFAB_LPASS_Q6_ACLK_ACR			REG(0x1074)
#define SFAB_AFAB_M_ACLK_ACR			REG(0x107C)
#define AFAB_SFAB_M_ACLK_ACR			REG(0x1084)
#define SFAB_SATA_S_HCLK_ACR			REG(0x1090)
#define DFAB_ACR				REG(0x1094)
#define SFAB_DFAB_M_ACLK_ACR			REG(0x10A0)
#define DFAB_SFAB_M_ACLK_ACR			REG(0x10A4)
#define DFAB_SWAY_ACR				REG(0x10A8)
#define DFAB_ARB_ACR				REG(0x10AC)
#define PPSS_ACR				REG(0x10B0)
#define PMEM_ACR				REG(0x10B4)
#define DMA_BAM_ACR				REG(0x10B8)
#define SIC_HCLK_ACR				REG(0x10BC)
#define SPS_TIC_HCLK_ACR			REG(0x10C0)
#define CFPB_ACR				REG(0x10C8)
#define SFAB_CFPB_M_HCLK_ACR			REG(0x10D0)
#define CFPB_MASTER_HCLK_ACR			REG(0x10D4)
#define SFAB_CFPB_S_HCLK_ACR			REG(0x10D8)
#define CFPB_SPLITTER_HCLK_ACR			REG(0x10DC)
#define TSIF_ACR				REG(0x10E0)
#define CE1_HCLK_ACR				REG(0x10E4)
#define CE2_HCLK_ACR				REG(0x10E8)
#define SFPB_HCLK_ACR				REG(0x10EC)
#define SFAB_SFPB_M_HCLK_ACR			REG(0x10F0)
#define SFAB_SFPB_S_HCLK_ACR			REG(0x10F4)
#define RPM_ACR					REG(0x10F8)
#define RPM_MSG_RAM_ACR				REG(0x10FC)
#define PMIC_ACR				REG(0x1100)
#define SDCn_ACR(n)				REG(0x1104+4*(n-1))
#define SDC1_ACR				REG(0x1104)
#define ACC_ACR					REG(0x111C)
#define USB_HS1_ACR				REG(0x1120)
#define USB_HSIC_ACR				REG(0x1124)
#define DIM_BUS_ACR				REG(0x1128)
#define USB_FS1_ACR				REG(0x112C)
#define GSBI_COMMON_SIM_CLK_ACR			REG(0x1134)
#define GSBIn_ACR(n)				REG(0x1138+4*(n-1))
#define GSBI1_ACR				REG(0x1138)
#define USB_HSIC_HSIC_CLK_CTL_ACR		REG(0x1168)
#define SPDM_ACR				REG(0x116C)
#define SEC_CTRL_ACR				REG(0x1170)
#define TLMM_ACR				REG(0x1174)
#define SATA_ACR				REG(0x1180)
#define SATA_ACLK_ACR				REG(0x1184)
#define SATA_PHY_CFG_ACR			REG(0x1188)
#define GSS_ACR					REG(0x118C)
#define TSSC_CLK_ACR				REG(0x1194)
#define PDM_ACR					REG(0x1198)
#define GPn_ACR(n)				REG(0x11A0+4*(n))
#define GP0_ACR					REG(0x11A0)
#define MPM_ACR					REG(0x11B4)
#define RINGOSC_ACR				REG(0x11B8)
#define EBI1_ACR				REG(0x11BC)
#define SFAB_SMPSS_S_HCLK_ACR			REG(0x11C0)
#define SCSS_DBG_STATUS_ACR			REG(0x11CC)
#define PRNG_ACR				REG(0x11D0)
#define PXO_SRC_CLK_CTL_ACR			REG(0x11D4)
#define LPASS_XO_SRC_CLK_CTL_ACR		REG(0x11D8)
#define GLOBAL_BUS_NS_ACR			REG(0x11DC)
#define PLL11_DIV_SRC_ACR			REG(0x11E4)
#define SPDM_CY_CLK_CTL_ACR			REG(0x11EC)
#define RESET_ACR				REG(0x11F0)
#define CLK_DBG_ACR				REG(0x11F4)
#define CLK_HALT_ACR				REG(0x11F8)
#define RPM_CLK_VOTE_ACR			REG(0x11FC)
#define LPA_Q6_CLK_VOTE_ACR			REG(0x1204)
#define APCS_CLK_VOTE_ACR			REG(0x1208)
#define SPARE_CLK_VOTE_ACR			REG(0x120C)
#define APCS_U_CLK_VOTE_ACR			REG(0x1210)
#define PLL0_ACR				REG(0x1218)
#define PLL5_ACR				REG(0x121C)
#define PLL8_ACR				REG(0x1228)
#define GPLL1_ACR				REG(0x122C)
#define EBI1_PLL_ACR				REG(0x1230)
#define PLL18_ACR				REG(0x1234)
#define PLL14_ACR				REG(0x1238)
#define SC_PLL0_ACR				REG(0x1240)
#define SC_PLL1_ACR				REG(0x1248)
#define SC_PLL2_ACR				REG(0x1250)
#define SC_PLL3_ACR				REG(0x1258)
#define SC_L2_PLL_ACR				REG(0x1260)
#define PLL_LOCK_DET_ACR			REG(0x1284)
#define PLL_ENA_SPARE_ACR			REG(0x128C)
#define PLL_ENA_GSS_ACR				REG(0x1290)
#define PLL_ENA_RPM_ACR				REG(0x1294)
#define PLL_ENA_APCS_ACR			REG(0x1298)
#define PLL_ENA_APCS_U_ACR			REG(0x129C)
#define PLL_ENA_RIVA_ACR			REG(0x12A0)
#define PLL_ENA_LPASS_DSP_ACR			REG(0x12A4)
#define PLL_ENA_SPS_ACR				REG(0x12A8)
#define FABS_RESET_ACR				REG(0x12AC)
#define RIVA_ACR				REG(0x12BC)
#define XPU_ACR					REG(0x12C0)
#define TSENS_ACR				REG(0x12C4)
#define TSENS_CONFIG_ACR			REG(0x12C8)
#define TSENS_STATUS_ACR			REG(0x12CC)
#define CE5_CORE_CLK_ACR			REG(0x12D8)
#define SFAB_AHB_S_FCLK2_ACR			REG(0x12DC)
#define USB_HS3_ACR				REG(0x12E0)
#define USB_HS4_ACR				REG(0x12E4)
#define GSS_CLK_VOTE_ACR			REG(0x12E8)
#define RIVA_CLK_VOTE_ACR			REG(0x12FC)
#define SPDM_CY_CLK_CTL2_ACR			REG(0x1300)
#define APCS_WDOG_EXPIRED_ENA_ACR		REG(0x1320)
#define RPM_WDOG_EXPIRED_ENA_ACR		REG(0x1340)
#define PCIE_ALT_REF_CLK_ACR			REG(0x1344)
#define SPARE0_ACR				REG(0x1348)
#define SPARE1_ACR				REG(0x134C)
#define PCIE_1_ACR				REG(0x1350)
#define PCIE_1_ALT_REF_CLK_ACR			REG(0x1354)
#define PCIE_2_ACR				REG(0x1358)
#define PCIE_2_ALT_REF_CLK_ACR			REG(0x135C)
#define EBI2_CLK_ACR				REG(0x1360)
#define USB30_CLK_ACR				REG(0x1364)
#define USB30_1_CLK_ACR				REG(0x1368)
#define NSS_RESET_ACR				REG(0x136C)
#define NSS_FAB0_CLK_SRC_ACR			REG(0x1370)
#define NSS_FB0_CLK_CTL_ACR			REG(0x1374)
#define NSS_FAB_GLOBAL_BUS_NS_ACR		REG(0x1378)
#define NSS_FAB1_CLK_SRC_ACR			REG(0x137C)
#define NSS_FB1_CLK_CTL_ACR			REG(0x1380)
#define CLK_HALT_1_ACR				REG(0x1384)
#define UBI32_MPT_CLK_CTL_ACR			REG(0x1388)
#define CE5_HCLK_ACR				REG(0x138C)
#define NSS_FPB_CLK_CTL_ACR			REG(0x1390)
#define GMAC_COREn_ACR(n)			REG(0x1394+4*(n-1))
#define GMAC_CORE1_ACR				REG(0x1394)
#define AFAB_CLK_SRC0_NS			REG(0x2000)
#define AFAB_CLK_SRC1_NS			REG(0x2004)
#define AFAB_CLK_SRC_CTL			REG(0x2008)
#define QDSS_STM_CLK_CTL			REG(0x2060)
#define QDSS_REQ_STATUS				REG(0x2064)
#define AFAB_CORE_CLK_CTL			REG(0x2080)
#define SCSS_ACLK_CTL				REG(0x20A0)
#define SCSS_HCLK_CTL				REG(0x20A4)
#define SCSS_XO_SRC_CLK_CTL			REG(0x20AC)
#define SCSS_AFAB_PORT_RESET			REG(0x20B8)
#define AFAB_EBI1_CH0_ACLK_CTL			REG(0x20C0)
#define AFAB_EBI1_CH1_ACLK_CTL			REG(0x20C4)
#define AFAB_AXI_S0_FCLK_CTL			REG(0x2100)
#define AFAB_AXI_S1_FCLK_CTL			REG(0x2104)
#define AFAB_AXI_S2_FCLK_CTL			REG(0x2108)
#define AFAB_AXI_S3_FCLK_CTL			REG(0x210C)
#define AFAB_AXI_S4_FCLK_CTL			REG(0x2110)
#define SFAB_CORE_CLK_CTL			REG(0x2120)
#define SFAB_AXI_S0_FCLK_CTL			REG(0x2140)
#define SFAB_AXI_S1_FCLK_CTL			REG(0x2144)
#define SFAB_AXI_S2_FCLK_CTL			REG(0x2148)
#define SFAB_AXI_S3_FCLK_CTL			REG(0x214C)
#define SFAB_AXI_S4_FCLK_CTL			REG(0x2150)
#define SFAB_AXI_S5_FCLK_CTL			REG(0x2154)
#define SFAB_AHB_S0_FCLK_CTL			REG(0x2160)
#define SFAB_AHB_S1_FCLK_CTL			REG(0x2164)
#define SFAB_AHB_S2_FCLK_CTL			REG(0x2168)
#define SFAB_AHB_S3_FCLK_CTL			REG(0x216C)
#define SFAB_AHB_S4_FCLK_CTL			REG(0x2170)
#define SFAB_AHB_S5_FCLK_CTL			REG(0x2174)
#define SFAB_AHB_S6_FCLK_CTL			REG(0x2178)
#define SFAB_AHB_S7_FCLK_CTL			REG(0x217C)
#define QDSS_AT_CLK_SRC0_NS			REG(0x2180)
#define QDSS_AT_CLK_SRC1_NS			REG(0x2184)
#define QDSS_AT_CLK_SRC_CTL			REG(0x2188)
#define QDSS_AT_CLK_NS				REG(0x218C)
#define QDSS_AT_CLK_FS				REG(0x2190)
#define QDSS_TRACECLKIN_CLK_SRC0_NS		REG(0x21A0)
#define QDSS_TRACECLKIN_CLK_SRC1_NS		REG(0x21A4)
#define QDSS_TRACECLKIN_CLK_SRC_CTL		REG(0x21A8)
#define QDSS_TRACECLKIN_CTL			REG(0x21AC)
#define QDSS_TSCTR_CLK_SRC0_NS			REG(0x21C0)
#define QDSS_TSCTR_CLK_SRC1_NS			REG(0x21C4)
#define QDSS_TSCTR_CLK_SRC_CTL			REG(0x21C8)
#define QDSS_TSCTR_CTL				REG(0x21CC)
#define SFAB_ADM0_M0_ACLK_CTL			REG(0x21E0)
#define SFAB_ADM0_M1_ACLK_CTL			REG(0x21E4)
#define SFAB_ADM0_M2_HCLK_CTL			REG(0x21E8)
#define ADM0_CLK_CTL				REG(0x2200)
#define ADM0_CLK_FS				REG(0x2204)
#define ADM0_PBUS_CLK_CTL_REG			REG(0x2208)
#define ADM0_RESET				REG(0x220C)
#define QDSS_RESETS				REG(0x2260)
#define IMEM0_ACLK_CTL				REG(0x2280)
#define IMEM0_ACLK_FS				REG(0x2284)
#define QDSS_HCLK_CTL				REG(0x22A0)
#define PCIE_ACLK_CTL_REG			REG(0x22C0)
#define PCIE_ACLK_FS				REG(0x22C4)
#define PCIE_AUX_CLK_CTL			REG(0x22C8)
#define PCIE_HCLK_CTL_REG			REG(0x22CC)
#define PCIE_PCLK_CTL_REG			REG(0x22D0)
#define PCIE_PCLK_FS				REG(0x22D4)
#define PCIE_SFAB_PORT_RESET			REG(0x22D8)
#define PCIE_RESET				REG(0x22DC)
#define SFAB_CLK_SRC0_NS			REG(0x22E0)
#define SFAB_CLK_SRC1_NS			REG(0x22E4)
#define SFAB_CLK_SRC_CTL			REG(0x22E8)
#define SFAB_LPASS_Q6_ACLK_CTL			REG(0x23A0)
#define SFAB_AFAB_M_ACLK_CTL			REG(0x23E0)
#define AFAB_SFAB_M0_ACLK_CTL			REG(0x2420)
#define AFAB_SFAB_M1_ACLK_CTL			REG(0x2424)
#define SFAB_SATA_S_HCLK_CTL_REG		REG(0x2480)
#define DFAB_CLK_SRC0_NS			REG(0x24A0)
#define DFAB_CLK_SRC1_NS			REG(0x24A4)
#define DFAB_CLK_SRC_CTL			REG(0x24A8)
#define DFAB_CORE_CLK_CTL			REG(0x24AC)
#define SFAB_DFAB_M_ACLK_CTL			REG(0x2500)
#define DFAB_SFAB_M_ACLK_CTL			REG(0x2520)
#define DFAB_SWAY0_HCLK_CTL			REG(0x2540)
#define DFAB_SWAY1_HCLK_CTL			REG(0x2544)
#define DFAB_ARB0_HCLK_CTL			REG(0x2560)
#define DFAB_ARB1_HCLK_CTL			REG(0x2564)
#define PPSS_HCLK_CTL				REG(0x2580)
#define PPSS_HCLK_FS				REG(0x2584)
#define PPSS_PROC_CLK_CTL			REG(0x2588)
#define PPSS_TIMER0_CLK_CTL			REG(0x258C)
#define PPSS_TIMER1_CLK_CTL			REG(0x2590)
#define PPSS_RESET				REG(0x2594)
#define PMEM_ACLK_CTL_REG			REG(0x25A0)
#define PMEM_ACLK_FS				REG(0x25A4)
#define DMA_BAM_HCLK_CTL			REG(0x25C0)
#define DMA_BAM_HCLK_FS				REG(0x25C4)
#define SIC_HCLK_CTL				REG(0x25E0)
#define SPS_TIC_HCLK_CTL			REG(0x2600)
#define CFPB_2X_CLK_SRC0_NS			REG(0x2640)
#define CFPB_2X_CLK_SRC1_NS			REG(0x2644)
#define CFPB_2X_CLK_SRC_CTL			REG(0x2648)
#define CFPB_CLK_NS				REG(0x264C)
#define CFPB0_HCLK_CTL				REG(0x2650)
#define CFPB1_HCLK_CTL				REG(0x2654)
#define CFPB2_HCLK_CTL				REG(0x2658)
#define SFAB_CFPB_M_HCLK_CTL			REG(0x2680)
#define CFPB_MASTER_HCLK_CTL			REG(0x26A0)
#define SFAB_CFPB_S_HCLK_CTL			REG(0x26C0)
#define CFPB_SPLITTER_HCLK_CTL			REG(0x26E0)
#define TSIF_HCLK_CTL_REG			REG(0x2700)
#define TSIF_HCLK_FS				REG(0x2704)
#define TSIF_INACTIVITY_TIMERS_CLK_CTL		REG(0x2708)
#define TSIF_REF_CLK_MD_REG			REG(0x270C)
#define TSIF_REF_CLK_NS_REG			REG(0x2710)
#define CE1_HCLK_CTL_REG			REG(0x2720)
#define CE1_CORE_CLK_CTL_REG			REG(0x2724)
#define CE1_SLEEP_CLK_CTL			REG(0x2728)
#define CE2_HCLK_CTL				REG(0x2740)
#define CE2_CORE_CLK_CTL			REG(0x2744)
#define SFPB_HCLK_SRC0_NS			REG(0x2760)
#define SFPB_HCLK_SRC1_NS			REG(0x2764)
#define SFPB_HCLK_SRC_CTL			REG(0x2768)
#define SFPB_HCLK_CTL				REG(0x276C)
#define SFAB_SFPB_M_HCLK_CTL			REG(0x2780)
#define SFAB_SFPB_S_HCLK_CTL			REG(0x27A0)
#define RPM_PROC_CLK_CTL			REG(0x27C0)
#define RPM_BUS_HCLK_CTL			REG(0x27C4)
#define RPM_BUS_HCLK_FS				REG(0x27C8)
#define RPM_SLEEP_CLK_CTL			REG(0x27CC)
#define RPM_TIMER_CLK_CTL			REG(0x27D0)
#define RPM_MSG_RAM_HCLK_CTL_REG		REG(0x27E0)
#define RPM_MSG_RAM_HCLK_FS			REG(0x27E4)
#define PMIC_ARB0_HCLK_CTL			REG(0x2800)
#define PMIC_ARB1_HCLK_CTL			REG(0x2804)
#define PMIC_ARB1_HCLK_FS			REG(0x2808)
#define PMIC_SSBI2_NS				REG(0x280C)
#define PMIC_SSBI2_CLK_FS			REG(0x280D)
#define SDCn_HCLK_CTL_REG(n)			REG(0x2820+32*(n-1))
#define SDC1_HCLK_CTL				REG(0x2820)
#define SDCn_HCLK_FS(n)				REG(0x2824+32*(n-1))
#define SDC1_HCLK_FS				REG(0x2824)
#define SDCn_APPS_CLK_MD_REG(n)			REG(0x2828+32*(n-1))
#define SDC1_APPS_CLK_MD			REG(0x2828)
#define SDCn_APPS_CLK_NS_REG(n)			REG(0x282C+32*(n-1))
#define SDC1_APPS_CLK_NS			REG(0x282C)
#define SDCn_RESET_REG(n)			REG(0x2830+32*(n-1))
#define SDC1_RESET				REG(0x2830)
#define SDCn_APPS_CLK_FS(n)			REG(0x2834+32*(n-1))
#define SDC1_APPS_CLK_FS			REG(0x2834)
#define QMC_ACC					REG(0x28E0)
#define ACC_HPIMEM_RF8441			REG(0x28E4)
#define ARR_STBY_N				REG(0x28E8)
#define USB_HS1_HCLK_CTL_REG			REG(0x2900)
#define USB_HS1_HCLK_FS_REG			REG(0x2904)
#define USB_HS1_XCVR_FS_CLK_MD_REG		REG(0x2908)
#define USB_HS1_XCVR_FS_CLK_NS_REG		REG(0x290C)
#define USB_HS1_RESET_REG			REG(0x2910)
#define USB_HSIC_HCLK_CTL_REG			REG(0x2920)
#define USB_HSIC_XCVR_FS_CLK_MD_REG		REG(0x2924)
#define USB_HSIC_XCVR_FS_CLK_NS_REG		REG(0x2928)
#define USB_HSIC_SYSTEM_CLK_CTL_REG		REG(0x292C)
#define USB_HSIC_SYSTEM_CLK_FS			REG(0x2930)
#define USB_HSIC_RESET_REG			REG(0x2934)
#define VDD_USB_HSIC_GFS_CTL			REG(0x2938)
#define VDD_USB_HSIC_GFS_CTL_STATUS		REG(0x293C)
#define CFPB0_C0_HCLK_CTL			REG(0x2940)
#define CFPB0_D0_HCLK_CTL			REG(0x2944)
#define CFPB0_C1_HCLK_CTL			REG(0x2948)
#define CFPB0_D1_HCLK_CTL			REG(0x294C)
#define USB_FS1_HCLK_CTL_REG			REG(0x2960)
#define USB_FS1_XCVR_FS_CLK_MD_REG		REG(0x2964)
#define USB_FS1_XCVR_FS_CLK_NS_REG		REG(0x2968)
#define USB_FS1_SYSTEM_CLK_CTL_REG		REG(0x296C)
#define USB_FS1_SYSTEM_CLK_FS_REG		REG(0x2970)
#define USB_FS1_RESET_REG			REG(0x2974)
#define GSBI_COMMON_SIM_CLK_NS			REG(0x29A0)
#define GSBIn_HCLK_CTL_REG(n)			REG(0x29C0+32*(n-1))
#define GSBI1_HCLK_CTL				REG(0x29C0)
#define GSBIn_HCLK_FS(n)			REG(0x29C4+32*(n-1))
#define GSBI1_HCLK_FS				REG(0x29C4)
#define GSBIn_QUP_APPS_MD_REG(n)		REG(0x29C8+32*(n-1))
#define GSBI1_QUP_APPS_MD			REG(0x29C8)
#define GSBIn_QUP_APPS_NS_REG(n)		REG(0x29CC+32*(n-1))
#define GSBI1_QUP_APPS_NS			REG(0x29CC)
#define GSBIn_UART_APPS_MD_REG(n)		REG(0x29D0+32*(n-1))
#define GSBI1_UART_APPS_MD			REG(0x29D0)
#define GSBIn_UART_APPS_NS_REG(n)		REG(0x29D4+32*(n-1))
#define GSBI1_UART_APPS_NS			REG(0x29D4)
#define GSBIn_SIM_CLK_CTL(n)			REG(0x29D8+32*(n-1))
#define GSBI1_SIM_CLK_CTL			REG(0x29D8)
#define GSBIn_RESET_REG(n)			REG(0x29DC+32*(n-1))
#define GSBI1_RESET				REG(0x29DC)
#define USB_HSIC_HSIC_CLK_SRC_CTL_REG		REG(0x2B40)
#define USB_HSIC_HSIC_CLK_CTL_REG		REG(0x2B44)
#define USB_HSIC_HSIO_CAL_CLK_CTL_REG		REG(0x2B48)
#define SPDM_CFG_HCLK_CTL			REG(0x2B60)
#define SPDM_MSTR_HCLK_CTL			REG(0x2B64)
#define SPDM_FF_CLK_CTL				REG(0x2B68)
#define SPDM_RESET				REG(0x2B6C)
#define SEC_CTRL_CLK_CTL			REG(0x2B80)
#define SEC_CTRL_CLK_FS				REG(0x2B84)
#define SEC_CTRL_ACC_CLK_SRC0_NS		REG(0x2B88)
#define SEC_CTRL_ACC_CLK_SRC1_NS		REG(0x2B8C)
#define SEC_CTRL_ACC_CLK_SRC_CTL		REG(0x2B90)
#define SEC_CTRL_ACC_CLK_CTL			REG(0x2B94)
#define SEC_CTRL_ACC_CLK_FS			REG(0x2B98)
#define TLMM_HCLK_CTL				REG(0x2BA0)
#define TLMM_CLK_CTL				REG(0x2BA4)
#define SATA_HCLK_CTL_REG			REG(0x2C00)
#define SATA_HCLK_FS				REG(0x2C04)
#define SATA_CLK_SRC_NS_REG			REG(0x2C08)
#define SATA_RXOOB_CLK_CTL_REG			REG(0x2C0C)
#define SATA_PMALIVE_CLK_CTL_REG		REG(0x2C10)
#define SATA_PHY_REF_CLK_CTL_REG		REG(0x2C14)
#define SATA_SFAB_M_PORT_RESET			REG(0x2C18)
#define SATA_RESET				REG(0x2C1C)
#define SATA_ACLK_CTL_REG			REG(0x2C20)
#define SATA_ACLK_FS				REG(0x2C24)
#define SATA_PHY_CFG_CLK_CTL_REG		REG(0x2C40)
#define GSS_SLP_CLK_CTL				REG(0x2C60)
#define GSS_RESET				REG(0x2C64)
#define GSS_CLAMP_ENA				REG(0x2C68)
#define GSS_CXO_SRC_CTL				REG(0x2C74)
#define TSSC_CLK_CTL_REG			REG(0x2CA0)
#define PDM_CLK_NS_REG				REG(0x2CC0)
#define GPn_MD_REG(n)				REG(0x2D00+32*(n))
#define GP0_MD					REG(0x2D00)
#define GPn_NS_REG(n)				REG(0x2D24+32*(n))
#define GP0_NS					REG(0x2D24)
#define MPM_CLK_CTL				REG(0x2DA0)
#define MPM_RESET				REG(0x2DA4)
#define RINGOSC_NS_REG				REG(0x2DC0)
#define RINGOSC_TCXO_CTL_REG			REG(0x2DC4)
#define RINGOSC_STATUS_REG			REG(0x2DCC)
#define EBI1_CLK_SRC0_NS			REG(0x2DE0)
#define EBI1_CLK_SRC1_NS			REG(0x2DE4)
#define EBI1_CLK_SRC_CTL			REG(0x2DE8)
#define EBI1_CLK_CTL				REG(0x2DEC)
#define EBI1_FRQSW_CTL				REG(0x2DF0)
#define EBI1_FRQSW_STATUS			REG(0x2DF4)
#define EBI1_FRQSW_REQ_ACK_TIMER		REG(0x2DF8)
#define EBI1_XO_SRC_CTL				REG(0x2DFC)
#define SFAB_SMPSS_S_HCLK_CTL			REG(0x2E00)
#define SCSS_DBG_STATUS_REQ			REG(0x2E60)
#define SCSS_DBG_STATUS_CORE_PWRDUP		REG(0x2E64)
#define USB_PHY0_RESET_REG			REG(0x2E20)	// This is not there in the flat file??
#define PRNG_CLK_NS_REG				REG(0x2E80)
#define PXO_SRC_CLK_CTL_REG			REG(0x2EA0)
#define LPASS_XO_SRC_CLK_CTL			REG(0x2EC0)
#define GLOBAL_BUS_NS				REG(0x2EE0)
#define PLL11_DIV_SRC_CTL			REG(0x2F20)
#define SPDM_CY_PORT0_CLK_CTL			REG(0x2F60)
#define SPDM_CY_PORT1_CLK_CTL			REG(0x2F64)
#define SPDM_CY_PORT2_CLK_CTL			REG(0x2F68)
#define SPDM_CY_PORT3_CLK_CTL			REG(0x2F6C)
#define SPDM_CY_PORT4_CLK_CTL			REG(0x2F70)
#define SPDM_CY_PORT5_CLK_CTL			REG(0x2F74)
#define SPDM_CY_PORT6_CLK_CTL			REG(0x2F78)
#define SPDM_CY_PORT7_CLK_CTL			REG(0x2F7C)
#define RESET_ALL				REG(0x2F80)
#define RESET_STATUS				REG(0x2F84)
#define PLLTEST_PAD_CFG_REG			REG(0x2FA4)
#define CLKTEST_PAD_CFG				REG(0x2FA8)
#define JITTER_PROBE				REG(0x2FAC)
#define JITTER_PROBE_VAL			REG(0x2FB0)
#define CLK_HALT_AFAB_SFAB_STATEA_REG		REG(0x2FC0)
#define CLK_HALT_AFAB_SFAB_STATEB_REG		REG(0x2FC4)
#define CLK_HALT_DFAB_STATE_REG			REG(0x2FC8)
#define CLK_HALT_CFPB_STATEA_REG		REG(0x2FCC)
#define CLK_HALT_CFPB_STATEB_REG		REG(0x2FD0)
#define CLK_HALT_CFPB_STATEC_REG		REG(0x2FD4)
#define CLK_HALT_SFPB_MISC_STATE_REG		REG(0x2FD8)
#define CLK_HALT_GSS_KPSS_MISC_STATE		REG(0x2FDC)
#define CLK_HALT_MSS_SMPSS_MISC_STATE_REG	CLK_HALT_GSS_KPSS_MISC_STATE
#define RPM_CLK_BRANCH_ENA_VOTE			REG(0x2FE0)
#define RPM_CLK_SLEEP_ENA_VOTE			REG(0x2FE4)
#define LPA_Q6_CLK_BRANCH_ENA_VOTE		REG(0x3020)
#define LPA_Q6_CLK_SLEEP_ENA_VOTE		REG(0x3024)
#define APCS_CLK_BRANCH_ENA_VOTE		REG(0x3040)
#define APCS_CLK_SLEEP_ENA_VOTE			REG(0x3044)
#define SPARE_CLK_BRANCH_ENA_VOTE		REG(0x3060)
#define SC0_U_CLK_BRANCH_ENA_VOTE_REG		REG(0x3080)
#define APCS_U_CLK_SLEEP_ENA_VOTE		REG(0x3084)
#define PLL0_MODE				REG(0x30C0)
#define PLL0_L_VAL				REG(0x30C4)
#define PLL0_M_VAL				REG(0x30C8)
#define PLL0_N_VAL				REG(0x30CC)
#define PLL0_TEST_CTL				REG(0x30D0)
#define PLL0_CONFIG				REG(0x30D4)
#define PLL0_STATUS				REG(0x30D8)
#define PLL5_MODE				REG(0x30E0)
#define PLL5_L_VAL				REG(0x30E4)
#define PLL5_M_VAL				REG(0x30E8)
#define PLL5_N_VAL				REG(0x30EC)
#define PLL5_TEST_CTL				REG(0x30F0)
#define PLL5_CONFIG				REG(0x30F4)
#define PLL5_STATUS				REG(0x30F8)
#define PLL8_MODE				REG(0x3140)
#define PLL8_L_VAL				REG(0x3144)
#define PLL8_M_VAL				REG(0x3148)
#define PLL8_N_VAL				REG(0x314C)
#define PLL8_TEST_CTL				REG(0x3150)
#define PLL8_CONFIG				REG(0x3154)
#define BB_PLL8_STATUS_REG			REG(0x3158)
#define GPLL1_MODE				REG(0x3160)
#define GPLL1_L_VAL				REG(0x3164)
#define GPLL1_M_VAL				REG(0x3168)
#define GPLL1_N_VAL				REG(0x316C)
#define GPLL1_TEST_CTL				REG(0x3170)
#define GPLL1_CONFIG				REG(0x3174)
#define GPLL1_STATUS				REG(0x3178)
#define EBI1_PLL_MODE				REG(0x3180)
#define EBI1_PLL_L_VAL				REG(0x3184)
#define EBI1_PLL_M_VAL				REG(0x3188)
#define EBI1_PLL_N_VAL				REG(0x318C)
#define EBI1_PLL_TEST_CTL			REG(0x3190)
#define EBI1_PLL_CONFIG				REG(0x3194)
#define EBI1_PLL_STATUS				REG(0x3198)
#define PLL18_MODE				REG(0x31A0)
#define PLL18_L_VAL				REG(0x31A4)
#define PLL18_M_VAL				REG(0x31A8)
#define PLL18_N_VAL				REG(0x31AC)
#define PLL18_TEST_CTL				REG(0x31B0)
#define PLL18_CONFIG				REG(0x31B4)
#define PLL18_STATUS				REG(0x31B8)
#define BB_PLL14_MODE_REG			REG(0x31C0)
#define BB_PLL14_L_VAL_REG			REG(0x31C4)
#define BB_PLL14_M_VAL_REG			REG(0x31C8)
#define BB_PLL14_N_VAL_REG			REG(0x31CC)
#define PLL14_TEST_CTL				REG(0x31D0)
#define BB_PLL14_CONFIG_REG			REG(0x31D4)
#define BB_PLL14_STATUS_REG			REG(0x31D8)
#define SC_PLL0_MODE				REG(0x3200)
#define SC_PLL0_CONFIG_CTL			REG(0x3204)
#define SC_PLL0_L_VAL				REG(0x3208)
#define SC_PLL0_M_VAL				REG(0x320C)
#define SC_PLL0_N_VAL				REG(0x3210)
#define SC_PLL0_DROOP_CTL			REG(0x3214)
#define SC_PLL0_TEST_CTL			REG(0x3218)
#define SC_PLL0_STATUS				REG(0x321C)
#define SC_PLL1_MODE				REG(0x3240)
#define SC_PLL1_CONFIG_CTL			REG(0x3244)
#define SC_PLL1_L_VAL				REG(0x3248)
#define SC_PLL1_M_VAL				REG(0x324C)
#define SC_PLL1_N_VAL				REG(0x3250)
#define SC_PLL1_DROOP_CTL			REG(0x3254)
#define SC_PLL1_TEST_CTL			REG(0x3258)
#define SC_PLL1_STATUS				REG(0x325C)
#define SC_PLL2_MODE				REG(0x3280)
#define SC_PLL2_CONFIG_CTL			REG(0x3284)
#define SC_PLL2_L_VAL				REG(0x3288)
#define SC_PLL2_M_VAL				REG(0x328C)
#define SC_PLL2_N_VAL				REG(0x3290)
#define SC_PLL2_DROOP_CTL			REG(0x3294)
#define SC_PLL2_TEST_CTL			REG(0x3298)
#define SC_PLL2_STATUS				REG(0x329C)
#define SC_PLL3_MODE				REG(0x32C0)
#define SC_PLL3_CONFIG_CTL			REG(0x32C4)
#define SC_PLL3_L_VAL				REG(0x32C8)
#define SC_PLL3_M_VAL				REG(0x32CC)
#define SC_PLL3_N_VAL				REG(0x32D0)
#define SC_PLL3_DROOP_CTL			REG(0x32D4)
#define SC_PLL3_TEST_CTL			REG(0x32D8)
#define SC_PLL3_STATUS				REG(0x32DC)
#define SC_L2_PLL_MODE				REG(0x3300)
#define SC_L2_PLL_CONFIG_CTL			REG(0x3304)
#define SC_L2_PLL_L_VAL				REG(0x3308)
#define SC_L2_PLL_M_VAL				REG(0x330C)
#define SC_L2_PLL_N_VAL				REG(0x3310)
#define SC_L2_PLL_DROOP_CTL			REG(0x3314)
#define SC_L2_PLL_TEST_CTL			REG(0x3318)
#define SC_L2_PLL_STATUS			REG(0x331C)
#define PLL_LOCK_DET_STATUS			REG(0x3420)
#define PLL_LOCK_DET_MASK			REG(0x3424)
#define PLL_ENA_SPARE				REG(0x3460)
#define PLL_ENA_GSS				REG(0x3480)
#define PLL_ENA_RPM				REG(0x34A0)
#define BB_PLL_ENA_SC0_REG			REG(0x34C0)
#define PLL_ENA_APCS_U				REG(0x34E0)
#define PLL_ENA_RIVA				REG(0x3500)
#define PLL_ENA_LPASS_DSP			REG(0x3520)
#define PLL_ENA_SPS				REG(0x3540)
#define FABS_RESET				REG(0x3560)
#define RIVA_RESET				REG(0x35E0)
#define XPU_RESET				REG(0x3600)
#define TSENS_CNTL				REG(0x3620)
#define TSENS_THRESHOLD				REG(0x3624)
#define TSENS_S0_STATUS				REG(0x3628)
#define TSENS_S1_STATUS				REG(0x362C)
#define TSENS_S2_STATUS				REG(0x3630)
#define TSENS_S3_STATUS				REG(0x3634)
#define TSENS_S4_STATUS				REG(0x3638)
#define TSENS_INT_STATUS			REG(0x363C)
#define TSENS_CONFIG				REG(0x3640)
#define TSENS_TEST_CNTL				REG(0x3644)
#define TSENS_STATUS_CNTL			REG(0x3660)
#define TSENS_S5_STATUS				REG(0x3664)
#define TSENS_S6_STATUS				REG(0x3668)
#define TSENS_S7_STATUS				REG(0x366C)
#define TSENS_S8_STATUS				REG(0x3670)
#define TSENS_S9_STATUS				REG(0x3674)
#define TSENS_S10_STATUS			REG(0x3678)
#define CE5_CORE_CLK_SRC_CTL			REG(0x36C0)
#define CE5_CORE_CLK_SRC0_NS			REG(0x36C4)
#define CE5_CLK_SRC_NS_REG			CE5_CORE_CLK_SRC0_NS
#define CE5_CORE_CLK_SRC1_NS			REG(0x36C8)
#define CE5_CORE_CLK_CTL_REG			REG(0x36CC)
#define CE5_CORE_CLK_FS				REG(0x36D0)
#define CE3_SLEEP_CLK_CTL			REG(0x36D0)
#define SFAB_AHB_S8_FCLK_CTL			REG(0x36E0)
#define USB_HS3_HCLK_CTL_REG			REG(0x3700)
#define USB_HS3_HCLK_FS_REG			REG(0x3704)
#define USB_HS3_XCVR_FS_CLK_MD_REG		REG(0x3708)
#define USB_HS3_XCVR_FS_CLK_NS_REG		REG(0x370C)
#define USB_HS3_RESET_REG			REG(0x3710)
#define USB_HS4_HCLK_CTL_REG			REG(0x3720)
#define USB_HS4_HCLK_FS_REG			REG(0x3724)
#define USB_HS4_XCVR_FS_CLK_MD_REG		REG(0x3728)
#define USB_HS4_XCVR_FS_CLK_NS_REG		REG(0x372C)
#define USB_HS4_RESET_REG			REG(0x3730)
#define GSS_CLK_BRANCH_ENA_VOTE			REG(0x3740)
#define GSS_CLK_SLEEP_ENA_VOTE			REG(0x3744)
#define RIVA_CLK_BRANCH_ENA_VOTE		REG(0x37E4)
#define SPDM_CY_PORT8_CLK_CTL			REG(0x3800)
#define APCS_WDT0_CPU0_WDOG_EXPIRED_ENABLE	REG(0x3820)
#define APCS_WDT1_CPU0_WDOG_EXPIRED_ENABLE	REG(0x3824)
#define APCS_WDT0_CPU1_WDOG_EXPIRED_ENABLE	REG(0x3828)
#define APCS_WDT1_CPU1_WDOG_EXPIRED_ENABLE	REG(0x382C)
#define APCS_WDT0_CPU2_WDOG_EXPIRED_ENABLE	REG(0x3830)
#define APCS_WDT1_CPU2_WDOG_EXPIRED_ENABLE	REG(0x3834)
#define APCS_WDT0_CPU3_WDOG_EXPIRED_ENABLE	REG(0x3838)
#define APCS_WDT1_CPU3_WDOG_EXPIRED_ENABLE	REG(0x383C)
#define RPM_WDOG_EXPIRED_ENABLE			REG(0x3840)
#define PCIE_ALT_REF_CLK_NS			REG(0x3860)
#define SPARE0					REG(0x3880)
#define SPARE1					REG(0x3884)
#define SPARE2					REG(0x3888)
#define SPARE3					REG(0x388C)
#define SPARE4					REG(0x38A0)
#define SPARE5					REG(0x38A4)
#define SPARE6					REG(0x38A8)
#define SPARE7					REG(0x38AC)
#define PCIE_1_ACLK_CTL_REG			REG(0x3A80)
#define PCIE_1_ACLK_FS				REG(0x3A84)
#define PCIE_1_AUX_CLK_CTL			REG(0x3A88)
#define PCIE_1_HCLK_CTL_REG			REG(0x3A8C)
#define PCIE_1_PCLK_CTL_REG			REG(0x3A90)
#define PCIE_1_PCLK_FS				REG(0x3A94)
#define PCIE_1_SFAB_PORT_RESET			REG(0x3A98)
#define PCIE_1_RESET				REG(0x3A9C)
#define PCIE_1_ALT_REF_CLK_NS			REG(0x3AA0)
#define PCIE_2_ACLK_CTL_REG			REG(0x3AC0)
#define PCIE_2_ACLK_FS				REG(0x3AC4)
#define PCIE_2_AUX_CLK_CTL			REG(0x3AC8)
#define PCIE_2_HCLK_CTL_REG			REG(0x3ACC)
#define PCIE_2_PCLK_CTL_REG			REG(0x3AD0)
#define PCIE_2_PCLK_FS				REG(0x3AD4)
#define PCIE_2_SFAB_PORT_RESET			REG(0x3AD8)
#define PCIE_2_RESET				REG(0x3ADC)
#define PCIE_2_ALT_REF_CLK_NS			REG(0x3AE0)
#define EBI2_CLK_CTL				REG(0x3B00)
#define EBI2_CLK_FS				REG(0x3B04)
#define USB30_0_ACLK_FS				REG(0x3B20)
#define USB30_0_MASTER_CLK_CTL			REG(0x3B24)
#define USB30_MASTER_CLK_MD			REG(0x3B28)
#define USB30_MASTER_CLK_NS			REG(0x3B2C)
#define USB30_1_ACLK_FS				REG(0x3B30)
#define USB30_1_MASTER_CLK_CTL			REG(0x3B34)
#define USB30_0_SLEEP_CLK_CTL			REG(0x3B38)
#define USB30_1_SLEEP_CLK_CTL			REG(0x3B3C)
#define USB30_MOC_UTMI_CLK_MD			REG(0x3B40)
#define USB30_MOC_UTMI_CLK_NS			REG(0x3B44)
#define USB30_0_MOC_UTMI_CLK_CTL		REG(0x3B48)
#define USB30_1_MOC_UTMI_CLK_CTL		REG(0x3B4C)
#define USB30_0_RESET				REG(0x3B50)
#define USB30_0_SFAB_PORT_RESET			REG(0x3B54)
#define USB30_1_RESET				REG(0x3B58)
#define USB30_1_SFAB_PORT_RESET			REG(0x3B5C)
#define NSS_RESET				REG(0x3B60)
#define NSS_RESET_SPARE				REG(0x3B64)
#define NSS_FB0_CLK_SRC_CTL			REG(0x3B80)
#define NSS_FAB0_CLK_SRC0_NS			REG(0x3B84)
#define NSS_FAB0_CLK_SRC1_NS			REG(0x3B88)
#define NSS_FB0_CLK_CTL				REG(0x3BA0)
#define NSS_FAB_GLOBAL_BUS_NS			REG(0x3BC0)
#define NSS_FB1_CLK_SRC_CTL			REG(0x3BE0)
#define NSS_FAB1_CLK_SRC0_NS			REG(0x3BE4)
#define NSS_FAB1_CLK_SRC1_NS			REG(0x3BE8)
#define NSS_FB1_CLK_CTL				REG(0x3C00)
#define CLK_HALT_NSS_FAB0_NSS_FAB1_STATEA_REG	REG(0x3C20)
#define UBI32_MPT_CLK_CTL			REG(0x3C40)
#define CE5_HCLK_SRC_CTL			REG(0x3C60)
#define CE5_HCLK_SRC0_NS			REG(0x3C64)
#define CE5_HCLK_SRC1_NS			REG(0x3C68)
#define CE5_HCLK_CTL_REG			REG(0x3C6C)
#define NSS_FPB_CLK_CTL				REG(0x3C80)
#define NSS_FPB_CLK_SRC_CTL			REG(0x3C84)
#define NSS_FPB_CLK_SRC0_NS			REG(0x3C88)
#define NSS_FPB_CLK_SRC1_NS			REG(0x3C88)
#define GMAC_COREn_CLK_SRC_CTL(n)		REG(0x3CA0+32*(n-1))
#define GMAC_CORE1_CLK_SRC_CTL			REG(0x3CA0)
#define GMAC_COREn_CLK_MD(n)			REG(0x3CA4+32*(n-1))
#define GMAC_CORE1_CLK_MD			REG(0x3CA4)
#define GMAC_COREn_DUAL_MN8_CRC_CTL(n)		REG(0x3CA8+32*(n-1))
#define GMAC_CORE1_DUAL_MN8_CRC_CTL		REG(0x3CA8)
#define GMAC_COREn_CLK_CTL(n)			REG(0x3CAC+32*(n-1))
#define GMAC_CORE1_CLK_CTL			REG(0x3CAC)
#define GMAC_COREn_CLK_FS(n)			REG(0x3CB0+32*(n-1))
#define GMAC_CORE1_CLK_FS			REG(0x3CB0)
#define UBI32_COREn_CLK_SRC_CTL(n)		REG(0x3D20+32*(n))
#define UBI32_CORE1_CLK_SRC_CTL			REG(0x3D20)
#define UBI32_COREn_CLK_SRC0_MD(n)		REG(0x3D24+32*(n))
#define UBI32_CORE1_CLK_SRC0_MD			REG(0x3D24)
#define UBI32_COREn_CLK_SRC1_MD(n)		REG(0x3D28+32*(n))
#define UBI32_CORE1_CLK_SRC1_MD			REG(0x3D28)
#define UBI32_COREn_CLK_SRC0_NS(n)		REG(0x3D2C+32*(n))
#define UBI32_CORE1_CLK_SRC0_NS			REG(0x3D2C)
#define UBI32_COREn_CLK_SRC1_NS(n)		REG(0x3D30+32*(n))
#define UBI32_CORE1_CLK_SRC1_NS			REG(0x3D30)
#define UBI32_COREn_CLK_CTL(n)			REG(0x3D34+32*(n))
#define UBI32_CORE1_CLK_CTL			REG(0x3D34)
#define UBI32_COREn_DUAL_MN8_CRC_CTL(n)		REG(0x3D28+32*(n-1))
#define UBI32_CORE1_DUAL_MN8_CRC_CTL		REG(0x3D28)
#define UBI32_COREn_CLK_FS(n)			REG(0x3D30+32*(n-1))
#define UBI32_CORE1_CLK_FS			REG(0x3D30)
#define NSS_250MHZ_CLK_SRC_CTL			REG(0x3D60)
#define NSS_250MHZ_CLK_SRC0_NS			REG(0x3D64)
#define NSS_250MHZ_CLK_SRC1_NS			REG(0x3D68)
#define NSS_250MHZ_CLK_CTL			REG(0x3D6C)
#define CE5_ACLK_SRC_CTL			REG(0x3D80)
#define CE5_ACLK_SRC0_NS			REG(0x3D84)
#define CE5_ACLK_SRC1_NS			REG(0x3D88)
#define CE5_ACLK_CTL				REG(0x3D8C)
#define PLL_ENA_NSS				REG(0x3DA0)
#define NSS_TCM_CLK_SRC_CTL			REG(0x3DC0)
#define NSS_TCM_CLK_SRC0_NS			REG(0x3DC4)
#define NSS_TCM_CLK_SRC1_NS			REG(0x3DC8)
#define NSS_TCM_CLK_FS				REG(0x3DCC)
#define NSS_TCM_CLK_CTL				REG(0x3DD0)
#define MVS_CNTRL				REG(0x3DF0)
#define CLK_TEST_REG				REG(0x2FA0)
#define GCC_APCS_CLK_DIAG			REG_GCC(0x001C)

/* LPASS CLOCK REGISTERS */
#define LCC_CLK_HS_DEBUG_CFG_REG		REG_LPA(0x00A4)
#define LCC_CLK_LS_DEBUG_CFG_REG		REG_LPA(0x00A8)
#define LCC_CODEC_I2S_MIC_MD_REG		REG_LPA(0x0064)
#define LCC_CODEC_I2S_MIC_NS_REG		REG_LPA(0x0060)
#define LCC_CODEC_I2S_MIC_STATUS_REG		REG_LPA(0x0068)
#define LCC_CODEC_I2S_SPKR_MD_REG		REG_LPA(0x0070)
#define LCC_CODEC_I2S_SPKR_NS_REG		REG_LPA(0x006C)
#define LCC_CODEC_I2S_SPKR_STATUS_REG		REG_LPA(0x0074)
#define LCC_MI2S_MD_REG				REG_LPA(0x004C)
#define LCC_MI2S_NS_REG				REG_LPA(0x0048)
#define LCC_MI2S_STATUS_REG			REG_LPA(0x0050)
#define LCC_PCM_MD_REG				REG_LPA(0x0058)
#define LCC_PCM_NS_REG				REG_LPA(0x0054)
#define LCC_PCM_STATUS_REG			REG_LPA(0x005C)
#define LCC_PLL0_MODE_REG			REG_LPA(0x0000)
#define LCC_PLL0_L_VAL_REG			REG_LPA(0x0004)
#define LCC_PLL0_M_VAL_REG			REG_LPA(0x0008)
#define LCC_PLL0_N_VAL_REG			REG_LPA(0x000C)
#define LCC_PLL0_CONFIG_REG			REG_LPA(0x0014)
#define LCC_PLL0_STATUS_REG			REG_LPA(0x0018)
#define LCC_SPARE_I2S_MIC_MD_REG		REG_LPA(0x007C)
#define LCC_SPARE_I2S_MIC_NS_REG		REG_LPA(0x0078)
#define LCC_SPARE_I2S_MIC_STATUS_REG		REG_LPA(0x0080)
#define LCC_SPARE_I2S_SPKR_MD_REG		REG_LPA(0x0088)
#define LCC_SPARE_I2S_SPKR_NS_REG		REG_LPA(0x0084)
#define LCC_SPARE_I2S_SPKR_STATUS_REG		REG_LPA(0x008C)
#define LCC_SLIMBUS_NS_REG			REG_LPA(0x00CC)
#define LCC_SLIMBUS_MD_REG			REG_LPA(0x00D0)
#define LCC_SLIMBUS_STATUS_REG			REG_LPA(0x00D4)
#define LCC_AHBEX_BRANCH_CTL_REG		REG_LPA(0x00E4)
#define LCC_PRI_PLL_CLK_CTL_REG			REG_LPA(0x00C4)
#define LCC_AHBIX_NS_REG			REG_LPA(0x0038)
#define LCC_AHBIX_MD_REG			REG_LPA(0x003C)
#define LCC_AHBIX_STATUS_REG			REG_LPA(0x0044)
#define LCC_PXO_SRC_CTL_REG			REG_LPA(0x00B4)
#define LCC_PRI_PLL_CLK_CTL			REG_LPA(0x00C4)
#define LCC_PLL0_MODE_MASK			(0)
#define LCC_PLL0_MODE_OUTCTRL			(1 << 0)
#define LCC_PLL0_MODE_BYPASSNL			(1 << 1)
#define LCC_PLL0_MODE_RESET_N			(1 << 2)
#define LCC_PXO_SRC_CTL_MASK			(0)
#define LCC_PXO_SRC_CTL_GFM_CXO_SRC_SEL		(1 << 0)
#define LCC_PRI_PLL_CLK_CTL_MASK		(0)
#define LCC_PRI_PLL_CLK_CTL_GFM_PRI_PLL_SRC_SEL	(1 << 0)


/* MUX source input identifiers. */
#define pxo_to_bb_mux		0
#define cxo_to_bb_mux		5
#define pll0_to_bb_mux		2
#define pll8_to_bb_mux		3
#define pll6_to_bb_mux		4
#define gnd_to_bb_mux		5
#define pll3_to_bb_mux		6
#define pxo_to_mm_mux		0
#define cxo_to_xo_mux		0
#define pxo_to_xo_mux		1
#define gnd_to_xo_mux		3
#define pxo_to_lpa_mux		0
#define pll4_to_lpa_mux		2
#define gnd_to_lpa_mux		6
#define pll3_to_pcie_mux	1
#define pxo_to_pcie_mux		1

/* Test Vector Macros */
#define TEST_TYPE_PER_LS	1
#define TEST_TYPE_PER_HS	2
#define TEST_TYPE_MM_LS		3
#define TEST_TYPE_MM_HS		4
#define TEST_TYPE_LPA		5
#define TEST_TYPE_CPUL2		6
#define TEST_TYPE_LPA_HS	7
#define TEST_TYPE_SHIFT		24
#define TEST_CLK_SEL_MASK	BM(23, 0)
#define TEST_VECTOR(s, t)	(((t) << TEST_TYPE_SHIFT) | BVAL(23, 0, (s)))
#define TEST_PER_LS(s)		TEST_VECTOR((s), TEST_TYPE_PER_LS)
#define TEST_PER_HS(s)		TEST_VECTOR((s), TEST_TYPE_PER_HS)
#define TEST_MM_LS(s)		TEST_VECTOR((s), TEST_TYPE_MM_LS)
#define TEST_LPA(s)		TEST_VECTOR((s), TEST_TYPE_LPA)
#define TEST_LPA_HS(s)		TEST_VECTOR((s), TEST_TYPE_LPA_HS)
#define TEST_CPUL2(s)		TEST_VECTOR((s), TEST_TYPE_CPUL2)

#define CLK_IPQ_AFAB_IDLE_FREQ		(133 * 1000 * 1000)
#define CLK_IPQ_AFAB_NOMINAL_FREQ	(400 * 1000 * 1000)
#define CLK_IPQ_AFAB_TURBO_FREQ		(533 * 1000 * 1000)
#define CLK_IPQ_SFAB_IDLE_FREQ		(80 * 1000 * 1000)
#define CLK_IPQ_SFAB_NOMINAL_FREQ	(133 * 1000 * 1000)
#define CLK_IPQ_SFAB_TURBO_FREQ		(177.667 * 1000 * 1000)
#define CLK_IPQ_DFAB_IDLE_FREQ		(32 * 1000 * 1000)
#define CLK_IPQ_DFAB_NOMINAL_FREQ	(64 * 1000 * 1000)
#define CLK_IPQ_DFAB_TURBO_FREQ		(64 * 1000 * 1000)
#define CLK_IPQ_SFPB_IDLE_FREQ		(32 * 1000 * 1000)
#define CLK_IPQ_SFPB_NOMINAL_FREQ	(64 * 1000 * 1000)
#define CLK_IPQ_SFPB_TURBO_FREQ		(64 * 1000 * 1000)
#define CLK_IPQ_CFPB_IDLE_FREQ		(32 * 1000 * 1000)
#define CLK_IPQ_CFPB_NOMINAL_FREQ	(64 * 1000 * 1000)
#define CLK_IPQ_CFPB_TURBO_FREQ		(64 * 1000 * 1000)

#define MN_MODE_DUAL_EDGE 0x2

#define PLL_VAL 0x00C1248B

#define CLK_USB_SS(name, n, h_b) \
	static struct rcg_clk name = { \
	.b = { \
		.ctl_reg = USB30_MASTER_CLK_NS, \
		.en_mask = BIT(11), \
		.halt_check = NOCHECK, \
	}, \
	.ns_reg = USB30_MASTER_CLK_NS, \
	.md_reg = USB30_MASTER_CLK_MD, \
	.ns_mask = (BM(23, 16) | BM(6, 0)), \
	.root_en_mask = BIT(11), \
	.mnd_en_mask = BIT(8), \
	.set_rate = set_rate_mnd, \
	.freq_tbl = clk_tbl_usb30, \
	.current_freq = &rcg_dummy_freq, \
	.c = { \
		.dbg_name = "USB30_clk", \
		.ops = &clk_ops_rcg, \
		CLK_INIT(name.c), \
	}, \
}

#define F_USB30(f, s, d, m, n) \
	{ \
		.freq_hz = f, \
		.src_clk = &s##_clk.c, \
		.md_val = MD8(16, m, 0, n), \
		.ns_val = NS(23, 16, n, m, 5, 4, 3, d, 2, 0, s##_to_bb_mux), \
	}

#define F_AIF_MI2S(f, s, d, m, n) \
	{ \
		.freq_hz = f, \
		.src_clk = &s##_clk.c, \
		.md_val = MD8(8, m, 0, n), \
		.ns_val = NS(31, 24, n, m, 5, 4, 3, d, 2, 0, s##_to_lpa_mux), \
	}

#define F_AIF_SPDIF(f, s, d, m, n) \
	{ \
		.freq_hz = f, \
		.src_clk = &s##_clk.c, \
		.md_val = MD16(m, n), \
		.ns_val = NS(31, 16, n, m, 5, 4, 3, d, 2, 0, s##_to_lpa_mux), \
	}

#define F_PCM(f, s, d, m, n) \
	{ \
		.freq_hz = f, \
		.src_clk = &s##_clk.c, \
		.md_val = MD16(m, n), \
		.ns_val = NS(31, 16, n, m, 5, 4, 3, d, 2, 0, s##_to_lpa_mux), \
	}

#define F_AHBIX(f, s, d, m, n) \
	{ \
		.freq_hz = f, \
		.src_clk = &s##_clk.c, \
		.md_val = MD8(8, m, 0, n), \
		.ns_val = NS(31, 24, n, m, 5, 4, 3, d, 2, 0, s##_to_lpa_mux),\
	}

#define IPQ_PXO_FREQ		(25 * 1000 * 1000)	/* MHz */
#define IPQ_CXO_FREQ		IPQ_PXO_FREQ

DEFINE_CLK_RPM_BRANCH(pxo_clk, pxo_a_clk, PXO, IPQ_PXO_FREQ);
DEFINE_CLK_RPM_BRANCH(cxo_clk, cxo_a_clk, CXO, IPQ_CXO_FREQ);

static struct pll_vote_clk pll0_clk = {
	.en_reg = BB_PLL_ENA_SC0_REG,
	.en_mask = BIT(0),
	.status_reg = PLL_LOCK_DET_STATUS,
	.status_mask = BIT(0),
	.parent = &pxo_clk.c,
	.c = {
		.dbg_name = "pll0_clk",
		.rate = 800000000,
		.ops = &clk_ops_pll_vote,
		CLK_INIT(pll0_clk.c),
	},
};

static struct clk_freq_tbl clk_tbl_usb30[] = {
	F_USB30(125000000, pll0, 1, 5, 32),
	F_END
};

CLK_USB_SS(usb30_0_master_clk, 0, 0);
static struct branch_clk usb30_0_branch_clk = {
	.b = {
		.ctl_reg = USB30_0_MASTER_CLK_CTL,
		.en_mask =  BIT(4),
		.halt_reg = CLK_HALT_AFAB_SFAB_STATEB_REG,
		.halt_bit = 22,
	},
	.parent = &usb30_0_master_clk.c,
	.c = {
		.dbg_name = "usb30_0_branch_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(usb30_0_branch_clk.c),
	},
};

static struct branch_clk usb30_1_branch_clk = {
	.b = {
		.ctl_reg = USB30_1_MASTER_CLK_CTL,
		.en_mask =  BIT(4),
		.halt_reg = CLK_HALT_AFAB_SFAB_STATEB_REG,
		.halt_bit = 17,
	},
	.parent = &usb30_0_master_clk.c,
	.c = {
		.dbg_name = "usb30_1_branch_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(usb30_1_branch_clk.c),
	},
};

#define CLK_USB_UTMI(name, n, h_b) \
	static struct rcg_clk name = { \
	.b = { \
		.ctl_reg = USB30_MOC_UTMI_CLK_NS, \
		.en_mask = BIT(11), \
		.halt_check = NOCHECK,\
	}, \
	.ns_reg = USB30_MOC_UTMI_CLK_NS, \
	.md_reg = USB30_MOC_UTMI_CLK_MD, \
	.ns_mask = (BM(23, 16) | BM(6, 0)), \
	.mnd_en_mask = BIT(8), \
	.set_rate = set_rate_mnd, \
	.freq_tbl = clk_tbl_usb30_utmi, \
	.current_freq = &rcg_dummy_freq, \
	.c = { \
		.dbg_name = #name, \
		.ops = &clk_ops_rcg, \
		CLK_INIT(usb30_utmi_clk.c), \
	}, \
}

static struct clk_freq_tbl clk_tbl_usb30_utmi[] = {
	F_USB30(60000000, pll0, 1, 1, 40),
	F_END
};

CLK_USB_UTMI(usb30_utmi_clk, 0, 0);
static struct branch_clk usb30_0_utmi_clk_ctl = {
	.b = {
		.ctl_reg = USB30_0_MOC_UTMI_CLK_CTL,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_AFAB_SFAB_STATEB_REG,
		.halt_bit = 21,
	},
	.parent = &usb30_utmi_clk.c,
	.c = {
		.dbg_name = "usb30_0_utmi_clk_ctl",
		.ops = &clk_ops_branch,
		CLK_INIT(usb30_0_utmi_clk_ctl.c),
	},
};

static struct branch_clk usb30_1_utmi_clk_ctl = {
	.b = {
		.ctl_reg = USB30_1_MOC_UTMI_CLK_CTL,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_AFAB_SFAB_STATEB_REG,
		.halt_bit = 15,
	},
	.parent = &usb30_utmi_clk.c,
	.c = {
		.dbg_name = "usb30_1_utmi_clk_ctl",
		.ops = &clk_ops_branch,
		CLK_INIT(usb30_1_utmi_clk_ctl.c),
	},
};

static struct clk_lookup msm_usb30_clocks_ipq806x[] = {
	CLK_LOOKUP("core_clk",		usb30_0_master_clk.c,	"msm-dwc3.0"),
	CLK_LOOKUP("iface_clk",		usb30_0_branch_clk.c,	"msm-dwc3.0"),
	CLK_LOOKUP("iface1_clk",	usb30_1_branch_clk.c,	"msm-dwc3.0"),
	CLK_LOOKUP("utmi_clk",		usb30_utmi_clk.c,	"msm-dwc3.0"),
	CLK_LOOKUP("utmi_b0_clk",	usb30_0_utmi_clk_ctl.c,	"msm-dwc3.0"),
	CLK_LOOKUP("utmi_b1_clk",	usb30_1_utmi_clk_ctl.c,	"msm-dwc3.0"),
};

struct clock_init_data ipq806x_usb30_clocks __initdata = {
	.table = msm_usb30_clocks_ipq806x,
	.size = ARRAY_SIZE(msm_usb30_clocks_ipq806x),
};

struct pll_rate{
	const uint32_t	l_val;
	const uint32_t	m_val;
	const uint32_t	n_val;
	const uint32_t	vco;
	const uint32_t	post_div;
	const uint32_t	i_bits;
};

#define PLL_RATE(l, m, n, v, d, i) { l, m, n, v, (d>>1), i }

enum vdd_dig_levels {
	VDD_DIG_NONE,
	VDD_DIG_LOW,
	VDD_DIG_NOMINAL,
	VDD_DIG_HIGH,
	VDD_DIG_NUM
};

static int set_vdd_dig_ipq806x(struct clk_vdd_class *vdd_class, int level)
{
	return 0;
}

static DEFINE_VDD_CLASS(vdd_dig, set_vdd_dig_ipq806x, VDD_DIG_NUM);

#define VDD_DIG_FMAX_MAP1(l1, f1)		\
	.vdd_class = &vdd_dig,			\
	.fmax = (unsigned long[VDD_DIG_NUM]) {	\
		[VDD_DIG_##l1] = (f1),		\
	}, \
	.num_fmax = VDD_DIG_NUM
#define VDD_DIG_FMAX_MAP2(l1, f1, l2, f2)	\
	.vdd_class = &vdd_dig,			\
	.fmax = (unsigned long[VDD_DIG_NUM]) {	\
		[VDD_DIG_##l1] = (f1),		\
		[VDD_DIG_##l2] = (f2),		\
	}, \
	.num_fmax = VDD_DIG_NUM
#define VDD_DIG_FMAX_MAP3(l1, f1, l2, f2, l3, f3) \
	.vdd_class = &vdd_dig,			\
	.fmax = (unsigned long[VDD_DIG_NUM]) {	\
		[VDD_DIG_##l1] = (f1),		\
		[VDD_DIG_##l2] = (f2),		\
		[VDD_DIG_##l3] = (f3),		\
	},					\
	.num_fmax = VDD_DIG_NUM

static int set_vdd_dig_nss_core(struct clk_vdd_class *vdd_class, int level)
{
	return 0;
}

static DEFINE_VDD_CLASS(vdd_nss_dig, set_vdd_dig_nss_core, VDD_DIG_NUM);

#define VDD_NSS_DIG_FMAX_MAP2(l1, f1, l2, f2)	\
	.vdd_class = &vdd_nss_dig,			\
	.fmax = (unsigned long[VDD_DIG_NUM]) {	\
		[VDD_DIG_##l1] = (f1),		\
		[VDD_DIG_##l2] = (f2),		\
	}, \
	.num_fmax = VDD_DIG_NUM

/*
 * Clock Descriptions
 */


static struct pll_clk pll3_clk = {
	.mode_reg = GPLL1_MODE,
	.parent = &pxo_clk.c,
	.c = {
		.dbg_name = "pll3_clk",
		.rate = 1200000000,
		.ops = &clk_ops_local_pll,
		CLK_INIT(pll3_clk.c),
	},
};

static struct pll_vote_clk pll8_clk = {
	.en_reg = BB_PLL_ENA_SC0_REG,
	.en_mask = BIT(8),
	.status_reg = BB_PLL8_STATUS_REG,
	.status_mask = BIT(16),
	.parent = &pxo_clk.c,
	.c = {
		.dbg_name = "pll8_clk",
		.rate = 384000000,
		.ops = &clk_ops_pll_vote,
		CLK_INIT(pll8_clk.c),
	},
};

static struct pll_vote_clk pll14_clk = {
	.en_reg = BB_PLL_ENA_SC0_REG,
	.en_mask = BIT(14),
	.status_reg = BB_PLL14_STATUS_REG,
	.status_mask = BIT(16),
	.parent = &pxo_clk.c,
	.c = {
		.dbg_name = "pll14_clk",
		.rate = 480000000,
		.ops = &clk_ops_pll_vote,
		CLK_INIT(pll14_clk.c),
	},
};

static struct pll_vote_clk pll4_clk = {
	.en_reg = BB_PLL_ENA_SC0_REG,
	.en_mask = BIT(4),
	.status_reg = PLL_LOCK_DET_STATUS,
	.status_mask = BIT(4),
	.parent = &pxo_clk.c,
	.c = {
		.dbg_name = "pll4_clk",
		.rate = 393216000,
		.ops = &clk_ops_pll_vote,
		CLK_INIT(pll4_clk.c),
	},
};

static struct clk_freq_tbl clk_tbl_aif_mi2s[] = {
	F_AIF_MI2S(0,		gnd,	1,	0,	0),
	/*
	 * 16 bit, stereo, 8KHz
	 * 16 bit, 7.1 channels, 8KHz
	 */
	F_AIF_MI2S(1024000,	pll4,	4,	1,	96),
	/*
	 * 16 bit, stereo, 11.025KHz
	 * 16 bit, 7.1 channels, 11.025KHz
	 */
	F_AIF_MI2S(1411200,	pll4,	4,	2,	139),
	/*
	 * 24 bit, stereo, 8KHz
	 * 16 bit, 5.1 channels, 8KHz
	 */
	F_AIF_MI2S(1536000,	pll4,	4,	1,	64),
	/*
	 * 16 bit, stereo, 16KHz
	 * 32 bit, stereo, 8KHz
	 * 16 bit, 7.1 channels, 16KHz
	 * 32 bit, 7.1 channels, 8KHz
	 */
	F_AIF_MI2S(2048000,	pll4,	4,	1,	48),
	/*
	 * 24 bit, stereo, 11.025KHz
	 * 16 bit, 5.1 channels, 11.025KHz
	 */
	F_AIF_MI2S(2116800,	pll4,	4,	2,	93),
	/*
	 * 24 bit, 5.1 channels, 8KHz
	 */
	F_AIF_MI2S(2304000,	pll4,	4,	2,	85),
	/*
	 * 16 bit, stereo, 22.05KHz
	 * 32  bit, stereo, 11.025KHz
	 * 16 bit, 7.1 channels, 22.05KHz
	 * 32 bit, 7.1 channels, 11.025KHz
	 */
	F_AIF_MI2S(2822400,	pll4,	4,	6,	209),
	/*
	 * 24 bit, stereo, 16KHz
	 * 16 bit, 5.1 channels, 16KHz
	 * 32 bit, 5.1 channels, 8KHz
	 * 24 bit, 7.1 channels, 8KHz
	 */
	F_AIF_MI2S(3072000,	pll4,	4,	1,	32),
	/*
	 * 24 bit, 5.1 channels, 11.025KHz
	 */
	F_AIF_MI2S(3175200,	pll4,	4,	1,	31),
	/*
	 * 16 bit, stereo, 32KHz
	 * 32 bit, stereo, 16KHz
	 * 16 bit, 7.1 channels, 32KHz
	 * 32 bit, 7.1 channels, 16KHz
	 */
	F_AIF_MI2S(4096000,	pll4,	4,	1,	24),
	/*
	 * 24 bit, stereo, 22.05KHz
	 * 16 bit, 5.1 channels, 22.05KHz
	 * 32 bit, 5.1 channels, 11.025KHz
	 * 24 bit, 7.1 channels, 11.025KHz
	 */
	F_AIF_MI2S(4233600,	pll4,	4,	9,	209),
	/*
	 * 24 bit, 5.1 channels, 16KHz
	 */
	F_AIF_MI2S(4608000,	pll4,	4,	3,	64),
	/*
	 * 16 bit, stereo, 44.1KHz
	 * 32 bit, stereo, 22.05KHz
	 * 16 bit, 7.1 channels, 44.1KHz
	 * 32 bit, 7.1 channels, 22.05KHz
	 */
	F_AIF_MI2S(5644800,	pll4,	4,	12,	209),
	/*
	 * 16 bit, stereo, 48KHz
	 * 24 bit, stereo, 32KHz
	 * 24 bit, stereo, 64KHz
	 * 16 bit, 5.1 channels, 32KHz
	 * 32 bit, 5.1 channels, 16KHz
	 * 16 bit, 7.1 channels, 48KHz
	 * 24 bit, 7.1 channels, 16KHz
	 */
	F_AIF_MI2S(6144000,	pll4,	4,	1,	16),
	/*
	 * 24 bit, 5.1 channels, 22.05KHz
	 */
	F_AIF_MI2S(6350400,	pll4,	4,	2,	31),
	/*
	 * 16 bit, stereo, 64KHz
	 * 32 bit, stereo, 32KHz
	 * 16 bit, 7.1 channels, 64KHz
	 * 32 bit, 7.1 channels, 32KHz
	 */
	F_AIF_MI2S(8192000,	pll4,	4,	1,	12),
	/*
	 * 24 bit, stereo, 44.1KHz
	 * 24 bit, stereo, 88.2KHz
	 * 16 bit, 5.1 channels, 44.1KHz
	 * 32 bit, 5.1 channels, 22.05KHz
	 * 24 bit, 7.1 channels, 22.05KHz
	 */
	F_AIF_MI2S(8467200,	pll4,	4,	18,	209),
	/*
	 * 24 bit, stereo, 48KHz
	 * 24 bit, stereo, 96KHz
	 * 16 bit, 5.1 channels, 48KHz
	 * 24 bit, 5.1 channels, 32KHz
	 */
	F_AIF_MI2S(9216000,	pll4,	4,	3,	32),
	/*
	 * 16bit, stereo, 88.2KHz
	 * 16bit, stereo, 176.4KHz
	 * 32bit, stereo, 44.1KHz
	 * 16bit, 7.1 channels, 88.2KHz
	 * 32bit, 7.1 channels, 44.1KHz
	 */
	F_AIF_MI2S(11289600,	pll4,	4,	24,	209),
	/*
	 * 32bit, stereo, 48KHz
	 * 16bit, 5.1 channels, 64KHz
	 * 32bit, 5.1 channels, 32KHz
	 * 16bit, 7.1 channels, 96KHz
	 * 24bit, 7.1 channels, 32KHz
	 * 32bit, 7.1 channels, 48KHz
	 */
	F_AIF_MI2S(12288000,	pll4,	4,	1,	8),
	/*
	 * 24bit, 7.1 channels, 44.1KHz
	 * 24bit, 7.1 channels, 88.2KHz
	 */
	F_AIF_MI2S(12700800,	pll4,	4,	27,	209),
	/*
	 * 24bit, 6ch, 48KHz
	 */
	F_AIF_MI2S(13824000,	pll4,	4,	9,	64),
	/*
	 * 32bit, stereo, 64KHz
	 * 32bit, 7.1 channels, 64KHz
	 */
	F_AIF_MI2S(16384000,	pll4,	4,	1,	6),
	/*
	 * 24bit, stereo, 176.4KHz
	 * 16bit, 5.1 channels, 88.2KHz
	 * 32bit, 5.1 channels, 44.1KHz
	 * 24bit, 7.1 channels, 44.1KHz
	 */
	F_AIF_MI2S(16934400,	pll4,	4,	41,	238),
	/*
	 * 24bit, stereo, 192KHz
	 * 16bit, 5.1 channels, 96KHz
	 * 24bit, 5.1 channels, 64KHz
	 * 32bit, 5.1 channels, 48KHz
	 * 24bit, 7.1 channels, 48KHz
	 */
	F_AIF_MI2S(18432000,	pll4,	4,	3,	16),
	/*
	 * 32bit, stereo, 176.4KHz
	 * 32bit, stereo, 88.2KHz
	 * 32bit, stereo, 88.2KHz
	 * 32bit, stereo, 176.4KHz
	 * 16bit, 7.1 channels, 176.4KHz
	 * 32bit, 7.1 channels, 88.2KHz
	 */
	F_AIF_MI2S(22579200,	pll4,	2,	24,	209),
	/*
	 * 16bit, stereo, 96KHz
	 * 32bit, stereo, 96KHz
	 * 32bit, stereo,  96KHz
	 * 32bit, 5.1 channels, 64KHz
	 * 16bit, 7.1 channels, 192KHz
	 * 24bit, 7.1 channels, 64KHz
	 * 32bit, 7.1 channels, 96KHz
	 */
	F_AIF_MI2S(24576000,	pll4,	4,	1,	4),
	/*
	 * 24bit, 5.1 channels, 96KHz
	 * 24bit, 5.1 channels, 192KHz
	 */
	F_AIF_MI2S(27648000,	pll4,	4,	9,	32),
	/*
	 * 16bit, 5.1 channels, 176.4KHz
	 * 32bit, 5.1 channels, 88.2KHz
	 * 32bit, 5.1 channels, 176.4KHz
	 * 24bit, 7.1 channels, 88.2KHz
	 * 24bit, 7.1 channels, 176.4KHz
	 */
	F_AIF_MI2S(33868800,	pll4,	4,	41,	119),
	/*
	 * 16bit, 5.1 channels, 192KHz
	 * 32bit, 5.1 channels, 96KHz
	 * 32bit, 5.1 channels, 192KHz
	 * 24bit, 7.1 channels, 96KHz
	 * 24bit, 7.1 channels, 192KHz
	 */
	F_AIF_MI2S(36864000,	pll4,	4,	3,	8),
	/*
	 * 32bit, 7.1 channels, 176.4KHz
	 */
	F_AIF_MI2S(45158400,	pll4,	1,	24,	209),
	/*
	 * 16bit, stereo, 192KHz
	 * 32bit, stereo, 192KHz
	 * 32bit, stereo, 192KHz
	 * 32bit, 7.1 channels, 192KHz
	 */
	F_AIF_MI2S(49152000,	pll4,	4,	1,	2),
	/*
	 * 24bit, 5.1 channels, 176.4KHz
	 */
	F_AIF_MI2S(50803200,	pll4,	1,	27,	209),
	F_END
};

#define CLK_AIF_MI2S_BIT(i, ns, h_r) \
struct cdiv_clk i##_clk = { \
	.b = { \
		.ctl_reg = ns, \
		.en_mask = BIT(15), \
		.halt_reg = h_r, \
		.halt_check = DELAY, \
	}, \
	.ns_reg = ns, \
	.ext_mask = BIT(14), \
	.div_offset = 10, \
	.max_div = 16, \
	.c = { \
		.dbg_name = #i "_clk", \
		.ops = &clk_ops_cdiv, \
		CLK_INIT(i##_clk.c), \
		.rate = ULONG_MAX, \
	}, \
}

#define CLK_AIF_MI2S_OSR(i, ns, md, h_r) \
	struct rcg_clk i##_clk = { \
		.b = { \
			.ctl_reg = ns, \
			.en_mask = BIT(17), \
			.reset_reg = ns, \
			.reset_mask = BIT(19), \
			.halt_reg = h_r, \
			.halt_check = ENABLE, \
			.halt_bit = 1 , \
		}, \
		.ns_reg = ns, \
		.md_reg = md, \
		.root_en_mask = BIT(9), \
		.ns_mask = (BM(31, 24) | BM(6, 0)), \
		.mnd_en_mask = BIT(8), \
		.set_rate = set_rate_mnd, \
		.freq_tbl = clk_tbl_aif_mi2s, \
		.current_freq = &rcg_dummy_freq, \
		.c = { \
			.dbg_name = #i "_clk", \
			.ops = &clk_ops_rcg, \
			CLK_INIT(i##_clk.c), \
		}, \
	}

static CLK_AIF_MI2S_OSR(mi2s_osr, LCC_MI2S_NS_REG, LCC_MI2S_MD_REG,
	LCC_MI2S_STATUS_REG);

static CLK_AIF_MI2S_BIT(mi2s_bit, LCC_MI2S_NS_REG, LCC_MI2S_STATUS_REG);

static struct clk_freq_tbl clk_tbl_pcm[] = {
	{ .ns_val = BIT(10) },
	F_PCM(64000,	pll4,	4,	1,	1536),
	F_PCM(128000,	pll4,	4,	1,	768),
	F_PCM(256000,	pll4,	4,	1,	384),
	F_PCM(512000,	pll4,	4,	1,	192),
	F_PCM(1024000,	pll4,	4,	1,	96),
	F_PCM(2048000,	pll4,	4,	1,	48),
	F_END
};

static struct rcg_clk pcm_clk = {
	.b = {
		.ctl_reg = LCC_PCM_NS_REG,
		.en_mask = BIT(11),
		.reset_reg = LCC_PCM_NS_REG,
		.reset_mask = BIT(13),
		.halt_reg = LCC_PCM_STATUS_REG,
		.halt_check = ENABLE,
		.halt_bit = 0,
	},
	.ns_reg = LCC_PCM_NS_REG,
	.md_reg = LCC_PCM_MD_REG,
	.root_en_mask = BIT(9),
	.ns_mask = BM(31, 16) | BM(6, 0),
	.mnd_en_mask = BIT(8),
	.set_rate = set_rate_mnd,
	.freq_tbl = clk_tbl_pcm,
	.current_freq = &rcg_dummy_freq,
	.c = {
		.dbg_name = "pcm_clk",
		.ops = &clk_ops_rcg,
		CLK_INIT(pcm_clk.c),
		.rate = ULONG_MAX,
	},
};

static struct clk_freq_tbl clk_tbl_aif_osr_492[] = {
	F_AIF_SPDIF(0,		gnd,	1,	0,	0),
	F_AIF_SPDIF(22050,	pll4,	1,	147,	20480),
	F_AIF_SPDIF(32000,	pll4,	1,	1,	96),
	F_AIF_SPDIF(44100,	pll4,	1,	147,	10240),
	F_AIF_SPDIF(48000,	pll4,	1,	1,	64),
	F_AIF_SPDIF(88200,	pll4,	1,	147,	5120),
	F_AIF_SPDIF(176400,	pll4,	1,	147,	2560),
	F_AIF_SPDIF(96000,	pll4,	1,	1,	32),
	F_AIF_SPDIF(192000,	pll4,	1,	1,	16),
	F_END
};

static struct rcg_clk spdif_clk = {
	.b = {
		.ctl_reg = LCC_SLIMBUS_NS_REG,
		.en_mask = BIT(12),
		.reset_reg = LCC_SLIMBUS_NS_REG,
		.reset_mask = BIT(7),
		.halt_reg = LCC_SLIMBUS_STATUS_REG,
		.halt_check = ENABLE,
		.halt_bit = 1,
	},
	.ns_reg = LCC_SLIMBUS_NS_REG,
	.md_reg = LCC_SLIMBUS_MD_REG,
	.root_en_mask = BIT(10) | BIT(8),
	.ns_mask = (BM(31, 16) | BM(6, 0)),
	.mnd_en_mask = BIT(9),
	.set_rate = set_rate_mnd,
	.freq_tbl = clk_tbl_aif_osr_492,
	.current_freq = &rcg_dummy_freq,
	.c = {
		.dbg_name = "SPDIF_CLK",
		.ops = &clk_ops_rcg,
		CLK_INIT(spdif_clk.c),
	},
};

static struct clk_freq_tbl clk_tbl_ahbix[] = {
	F_AHBIX(131072, pll4, 1, 1, 3),
	F_END
};

static struct rcg_clk ahbix_clk = {
	.b = {
		.ctl_reg = LCC_AHBIX_NS_REG,
		.reset_reg = LCC_AHBIX_NS_REG,
		.reset_mask = BIT(10),
		.halt_reg = LCC_AHBIX_STATUS_REG,
		.halt_check = ENABLE,
		.halt_bit = 0,
	},
	.ns_reg = LCC_AHBIX_NS_REG,
	.md_reg = LCC_AHBIX_MD_REG,
	.root_en_mask = BIT(8),
	.ns_mask = (BM(31, 24) | BM(6, 0)),
	.mnd_en_mask = BIT(8) | BIT(10),
	.set_rate = set_rate_mnd,
	.freq_tbl = clk_tbl_ahbix,
	.current_freq = &rcg_dummy_freq,
	.c = {
		.dbg_name = "AHBIX_CLK",
		.ops = &clk_ops_rcg,
		CLK_INIT(ahbix_clk.c),
	},
};

static struct branch_clk ahbex_clk = {
	.b = {
		.ctl_reg = LCC_AHBEX_BRANCH_CTL_REG,
		.en_mask = BIT(15) | BIT(16),
		.halt_reg = LCC_AHBIX_STATUS_REG,
		.halt_check = ENABLE,
		.halt_bit = 0,
	},
	.c = {
		.dbg_name = "AHBEX_CLK",
		.ops = &clk_ops_branch,
		CLK_INIT(ahbex_clk.c),
	},
};

/* AXI Interfaces */
/*
 * Peripheral Clocks
 */
#define CLK_GP(i, n, h_r, h_b) \
	struct rcg_clk i##_clk = { \
		.b = { \
			.ctl_reg = GPn_NS_REG(n), \
			.en_mask = BIT(9), \
			.halt_reg = h_r, \
			.halt_bit = h_b, \
		}, \
		.ns_reg = GPn_NS_REG(n), \
		.md_reg = GPn_MD_REG(n), \
		.root_en_mask = BIT(11), \
		.ns_mask = (BM(23, 16) | BM(6, 0)), \
		.mnd_en_mask = BIT(8), \
		.set_rate = set_rate_mnd, \
		.freq_tbl = clk_tbl_gp, \
		.current_freq = &rcg_dummy_freq, \
		.c = { \
			.dbg_name = #i "_clk", \
			.ops = &clk_ops_rcg, \
			VDD_DIG_FMAX_MAP2(LOW, 100000000, NOMINAL, 200000000), \
			CLK_INIT(i##_clk.c), \
		}, \
	}
#define F_GP(f, s, d, m, n) \
	{ \
		.freq_hz = f, \
		.src_clk = &s##_clk.c, \
		.md_val = MD8(16, m, 0, n), \
		.ns_val = NS(23, 16, n, m, 5, 4, 3, d, 2, 0, s##_to_bb_mux), \
	}
static struct clk_freq_tbl clk_tbl_gp[] = {
	F_GP(        0, gnd,  1, 0, 0),
	F_GP( 12500000, pxo,  2, 0, 0),
	F_GP( 25000000, pxo,  1, 0, 0),
	F_GP( 64000000, pll8, 2, 1, 3),
	F_GP( 76800000, pll8, 1, 1, 5),
	F_GP( 96000000, pll8, 4, 0, 0),
	F_GP(128000000, pll8, 3, 0, 0),
	F_GP(192000000, pll8, 2, 0, 0),
	F_END
};

static CLK_GP(gp0, 0, CLK_HALT_SFPB_MISC_STATE_REG, 7);
static CLK_GP(gp1, 1, CLK_HALT_SFPB_MISC_STATE_REG, 6);
static CLK_GP(gp2, 2, CLK_HALT_SFPB_MISC_STATE_REG, 5);

#define CLK_GSBI_UART(i, n, h_r, h_b) \
	struct rcg_clk i##_clk = { \
		.b = { \
			.ctl_reg = GSBIn_UART_APPS_NS_REG(n), \
			.en_mask = BIT(9), \
			.reset_reg = GSBIn_RESET_REG(n), \
			.reset_mask = BIT(0), \
			.halt_reg = h_r, \
			.halt_bit = h_b, \
		}, \
		.ns_reg = GSBIn_UART_APPS_NS_REG(n), \
		.md_reg = GSBIn_UART_APPS_MD_REG(n), \
		.root_en_mask = BIT(11), \
		.ns_mask = (BM(31, 16) | BM(6, 0)), \
		.mnd_en_mask = BIT(8), \
		.set_rate = set_rate_mnd, \
		.freq_tbl = clk_tbl_gsbi_uart, \
		.current_freq = &rcg_dummy_freq, \
		.c = { \
			.dbg_name = #i "_clk", \
			.ops = &clk_ops_rcg, \
			VDD_DIG_FMAX_MAP2(LOW, 32000000, NOMINAL, 64000000), \
			CLK_INIT(i##_clk.c), \
		}, \
	}
#define F_GSBI_UART(f, s, d, m, n) \
	{ \
		.freq_hz = f, \
		.src_clk = &s##_clk.c, \
		.md_val = MD16(m, n), \
		.ns_val = NS(31, 16, n, m, 5, 4, 3, d, 2, 0, s##_to_bb_mux), \
	}
static struct clk_freq_tbl clk_tbl_gsbi_uart[] = {
	F_GSBI_UART(       0, gnd,  1,  0,   0),
	F_GSBI_UART( 1843200, pll8, 2,  6, 625),
	F_GSBI_UART( 3686400, pll8, 2, 12, 625),
	F_GSBI_UART( 7372800, pll8, 2, 24, 625),
	F_GSBI_UART(14745600, pll8, 2, 48, 625),
	F_GSBI_UART(16000000, pll8, 4,  1,   6),
	F_GSBI_UART(24000000, pll8, 4,  1,   4),
	F_GSBI_UART(32000000, pll8, 4,  1,   3),
	F_GSBI_UART(40000000, pll8, 1,  5,  48),
	F_GSBI_UART(46400000, pll8, 1, 29, 240),
	F_GSBI_UART(48000000, pll8, 4,  1,   2),
	F_GSBI_UART(51200000, pll8, 1,  2,  15),
	F_GSBI_UART(56000000, pll8, 1,  7,  48),
	F_GSBI_UART(58982400, pll8, 1, 96, 625),
	F_GSBI_UART(64000000, pll8, 2,  1,   3),
	F_END
};

static CLK_GSBI_UART(gsbi1_uart,   1, CLK_HALT_CFPB_STATEA_REG, 12);
static CLK_GSBI_UART(gsbi2_uart,   2, CLK_HALT_CFPB_STATEA_REG,  8);
static CLK_GSBI_UART(gsbi4_uart,   4, CLK_HALT_CFPB_STATEB_REG, 26);
static CLK_GSBI_UART(gsbi5_uart,   5, CLK_HALT_CFPB_STATEB_REG, 22);
static CLK_GSBI_UART(gsbi6_uart,   6, CLK_HALT_CFPB_STATEB_REG, 18);
static CLK_GSBI_UART(gsbi7_uart,   7, CLK_HALT_CFPB_STATEB_REG, 14);

#define CLK_GSBI_QUP(i, n, h_r, h_b) \
	struct rcg_clk i##_clk = { \
		.b = { \
			.ctl_reg = GSBIn_QUP_APPS_NS_REG(n), \
			.en_mask = BIT(9), \
			.reset_reg = GSBIn_RESET_REG(n), \
			.reset_mask = BIT(0), \
			.halt_reg = h_r, \
			.halt_bit = h_b, \
		}, \
		.ns_reg = GSBIn_QUP_APPS_NS_REG(n), \
		.md_reg = GSBIn_QUP_APPS_MD_REG(n), \
		.root_en_mask = BIT(11), \
		.ns_mask = (BM(23, 16) | BM(6, 0)), \
		.mnd_en_mask = BIT(8), \
		.set_rate = set_rate_mnd, \
		.freq_tbl = clk_tbl_gsbi_qup, \
		.current_freq = &rcg_dummy_freq, \
		.c = { \
			.dbg_name = #i "_clk", \
			.ops = &clk_ops_rcg, \
			VDD_DIG_FMAX_MAP2(LOW, 24000000, NOMINAL, 52000000), \
			CLK_INIT(i##_clk.c), \
		}, \
	}
#define F_GSBI_QUP(f, s, d, m, n) \
	{ \
		.freq_hz = f, \
		.src_clk = &s##_clk.c, \
		.md_val = MD8(16, m, 0, n), \
		.ns_val = NS(23, 16, n, m, 5, 4, 3, d, 2, 0, s##_to_bb_mux), \
	}
static struct clk_freq_tbl clk_tbl_gsbi_qup[] = {
	F_GSBI_QUP(                 0, gnd,  1, 0,   0),
	F_GSBI_QUP( IPQ_PXO_FREQ / 25, pxo,  1, 1,  25),
	F_GSBI_QUP(  IPQ_PXO_FREQ / 5, pxo,  1, 1,   5),
	F_GSBI_QUP(IPQ_PXO_FREQ / 2.5, pxo,  1, 2,   5),
	F_GSBI_QUP(          15060000, pll8, 1, 2,  51),
	F_GSBI_QUP(          24000000, pll8, 4, 1,   4),
	F_GSBI_QUP(          25600000, pll8, 1, 1,  15),
	F_GSBI_QUP(          27000000, pll8, 1, 9, 128),
	F_GSBI_QUP(          48000000, pll8, 4, 1,   2),
	F_GSBI_QUP(          51200000, pll8, 1, 2,  15),
	F_END
};

static CLK_GSBI_QUP(gsbi1_qup,   1, CLK_HALT_CFPB_STATEA_REG, 11);
static CLK_GSBI_QUP(gsbi2_qup,   2, CLK_HALT_CFPB_STATEA_REG,  6);
static CLK_GSBI_QUP(gsbi5_qup,   5, CLK_HALT_CFPB_STATEB_REG, 20);
static CLK_GSBI_QUP(gsbi6_qup,   6, CLK_HALT_CFPB_STATEB_REG, 16);
static CLK_GSBI_QUP(gsbi7_qup,   7, CLK_HALT_CFPB_STATEB_REG, 12);

#define F_PDM(f, s, d) \
	{ \
		.freq_hz = f, \
		.src_clk = &s##_clk.c, \
		.ns_val = NS_SRC_SEL(1, 0, s##_to_xo_mux), \
	}
static struct clk_freq_tbl clk_tbl_pdm[] = {
	F_PDM(       0, gnd, 1),
	F_PDM(IPQ_PXO_FREQ, pxo, 1),
	F_END
};

static struct rcg_clk pdm_clk = {
	.b = {
		.ctl_reg = PDM_CLK_NS_REG,
		.en_mask = BIT(9),
		.reset_reg = PDM_CLK_NS_REG,
		.reset_mask = BIT(12),
		.halt_reg = CLK_HALT_CFPB_STATEC_REG,
		.halt_bit = 3,
	},
	.ns_reg = PDM_CLK_NS_REG,
	.root_en_mask = BIT(11),
	.ns_mask = BM(1, 0),
	.set_rate = set_rate_nop,
	.freq_tbl = clk_tbl_pdm,
	.current_freq = &rcg_dummy_freq,
	.c = {
		.dbg_name = "pdm_clk",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP1(LOW, IPQ_PXO_FREQ),
		CLK_INIT(pdm_clk.c),
	},
};

static struct branch_clk pmem_clk = {
	.b = {
		.ctl_reg = PMEM_ACLK_CTL_REG,
		.en_mask = BIT(4),
		.hwcg_reg = PMEM_ACLK_CTL_REG,
		.hwcg_mask = BIT(6),
		.halt_reg = CLK_HALT_DFAB_STATE_REG,
		.halt_bit = 20,
	},
	.c = {
		.dbg_name = "pmem_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(pmem_clk.c),
	},
};

#define F_PRNG(f, s) \
	{ \
		.freq_hz = f, \
		.src_clk = &s##_clk.c, \
	}
static struct clk_freq_tbl clk_tbl_prng_32[] = {
	F_PRNG(32000000, pll8),
	F_END
};

static struct clk_freq_tbl clk_tbl_prng_64[] = {
	F_PRNG(64000000, pll8),
	F_END
};

static struct rcg_clk prng_clk = {
	.b = {
		.ctl_reg = SC0_U_CLK_BRANCH_ENA_VOTE_REG,
		.en_mask = BIT(10),
		.halt_reg = CLK_HALT_SFPB_MISC_STATE_REG,
		.halt_check = HALT_VOTED,
		.halt_bit = 10,
	},
	.set_rate = set_rate_nop,
	.freq_tbl = clk_tbl_prng_32,
	.current_freq = &rcg_dummy_freq,
	.c = {
		.dbg_name = "prng_clk",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP2(LOW, 32000000, NOMINAL, 64000000),
		CLK_INIT(prng_clk.c),
	},
};

#define CLK_SDC(name, n, h_b, fmax_low, fmax_nom) \
	struct rcg_clk name = { \
		.b = { \
			.ctl_reg = SDCn_APPS_CLK_NS_REG(n), \
			.en_mask = BIT(9), \
			.reset_reg = SDCn_RESET_REG(n), \
			.reset_mask = BIT(0), \
			.halt_reg = CLK_HALT_DFAB_STATE_REG, \
			.halt_bit = h_b, \
		}, \
		.ns_reg = SDCn_APPS_CLK_NS_REG(n), \
		.md_reg = SDCn_APPS_CLK_MD_REG(n), \
		.root_en_mask = BIT(11), \
		.ns_mask = (BM(23, 16) | BM(6, 0)), \
		.mnd_en_mask = BIT(8), \
		.set_rate = set_rate_mnd, \
		.freq_tbl = clk_tbl_sdc, \
		.current_freq = &rcg_dummy_freq, \
		.c = { \
			.dbg_name = #name, \
			.ops = &clk_ops_rcg, \
			VDD_DIG_FMAX_MAP2(LOW, fmax_low, NOMINAL, fmax_nom), \
			CLK_INIT(name.c), \
		}, \
	}
#define F_SDC(f, s, d, m, n) \
	{ \
		.freq_hz = f, \
		.src_clk = &s##_clk.c, \
		.md_val = MD8(16, m, 0, n), \
		.ns_val = NS(23, 16, n, m, 5, 4, 3, d, 2, 0, s##_to_bb_mux), \
	}
static struct clk_freq_tbl clk_tbl_sdc[] = {
	F_SDC(        0, gnd,   1, 0,   0),
	F_SDC(   144000, pxo,   5, 18, 625),
	F_SDC(   400000, pll8,  4, 1, 240),
	F_SDC( 16000000, pll8,  4, 1,   6),
	F_SDC( 17070000, pll8,  1, 2,  45),
	F_SDC( 20210000, pll8,  1, 1,  19),
	F_SDC( 24000000, pll8,  4, 1,   4),
	F_SDC( 48000000, pll8,  4, 1,   2),
	F_SDC( 64000000, pll8,  3, 1,   2),
	F_SDC( 96000000, pll8,  4, 0,   0),
	F_SDC(192000000, pll8,  2, 0,   0),
	F_END
};

static CLK_SDC(sdc1_clk, 1, 6,  52000000, 104000000);
static CLK_SDC(sdc3_clk, 3, 4, 104000000, 208000000);

#define F_TSIF_REF(f, s, d, m, n) \
	{ \
		.freq_hz = f, \
		.src_clk = &s##_clk.c, \
		.md_val = MD16(m, n), \
		.ns_val = NS(31, 16, n, m, 5, 4, 3, d, 2, 0, s##_to_bb_mux), \
	}
static struct clk_freq_tbl clk_tbl_tsif_ref[] = {
	F_TSIF_REF(     0, gnd,  1, 0,   0),
	F_TSIF_REF(105000, pxo,  1, 1, 256),
	F_END
};

static struct rcg_clk tsif_ref_clk = {
	.b = {
		.ctl_reg = TSIF_REF_CLK_NS_REG,
		.en_mask = BIT(9),
		.halt_reg = CLK_HALT_CFPB_STATEC_REG,
		.halt_bit = 5,
	},
	.ns_reg = TSIF_REF_CLK_NS_REG,
	.md_reg = TSIF_REF_CLK_MD_REG,
	.root_en_mask = BIT(11),
	.ns_mask = (BM(31, 16) | BM(6, 0)),
	.mnd_en_mask = BIT(8),
	.set_rate = set_rate_mnd,
	.freq_tbl = clk_tbl_tsif_ref,
	.current_freq = &rcg_dummy_freq,
	.c = {
		.dbg_name = "tsif_ref_clk",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP2(LOW, IPQ_PXO_FREQ, NOMINAL, (2 * IPQ_PXO_FREQ)),
		CLK_INIT(tsif_ref_clk.c),
	},
};

#define F_TSSC(f, s) \
	{ \
		.freq_hz = f, \
		.src_clk = &s##_clk.c, \
		.ns_val = NS_SRC_SEL(1, 0, s##_to_xo_mux), \
	}
static struct clk_freq_tbl clk_tbl_tssc[] = {
	F_TSSC(       0, gnd),
	F_TSSC(IPQ_PXO_FREQ, pxo),
	F_END
};

static struct rcg_clk tssc_clk = {
	.b = {
		.ctl_reg = TSSC_CLK_CTL_REG,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_CFPB_STATEC_REG,
		.halt_bit = 4,
	},
	.ns_reg = TSSC_CLK_CTL_REG,
	.ns_mask = BM(1, 0),
	.set_rate = set_rate_nop,
	.freq_tbl = clk_tbl_tssc,
	.current_freq = &rcg_dummy_freq,
	.c = {
		.dbg_name = "tssc_clk",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP1(LOW, IPQ_PXO_FREQ),
		CLK_INIT(tssc_clk.c),
	},
};

#define CLK_USB_HS(name, n, h_b) \
	static struct rcg_clk name = { \
	.b = { \
		.ctl_reg = USB_HS##n##_XCVR_FS_CLK_NS_REG, \
		.en_mask = BIT(9), \
		.reset_reg = USB_HS##n##_RESET_REG, \
		.reset_mask = BIT(0), \
		.halt_reg = CLK_HALT_DFAB_STATE_REG, \
		.halt_bit = h_b, \
	}, \
	.ns_reg = USB_HS##n##_XCVR_FS_CLK_NS_REG, \
	.md_reg = USB_HS##n##_XCVR_FS_CLK_MD_REG, \
	.root_en_mask = BIT(11), \
	.ns_mask = (BM(23, 16) | BM(6, 0)), \
	.mnd_en_mask = BIT(8), \
	.set_rate = set_rate_mnd, \
	.freq_tbl = clk_tbl_usb, \
	.current_freq = &rcg_dummy_freq, \
	.c = { \
		.dbg_name = #name, \
		.ops = &clk_ops_rcg, \
		VDD_DIG_FMAX_MAP1(NOMINAL, 64000000), \
		CLK_INIT(name.c), \
	}, \
}

#define F_USB(f, s, d, m, n) \
	{ \
		.freq_hz = f, \
		.src_clk = &s##_clk.c, \
		.md_val = MD8(16, m, 0, n), \
		.ns_val = NS(23, 16, n, m, 5, 4, 3, d, 2, 0, s##_to_bb_mux), \
	}
static struct clk_freq_tbl clk_tbl_usb[] = {
	F_USB(       0, gnd,  1, 0,  0),
	F_USB(60000000, pll8, 1, 5, 32),
	F_END
};

CLK_USB_HS(usb_hs1_xcvr_clk, 1, 0);

static struct clk_freq_tbl clk_tbl_usb_hsic[] = {
	F_USB(       0, gnd,  1, 0,  0),
	F_USB(60000000, pll8, 1, 5, 32),
	F_END
};

static struct rcg_clk usb_hsic_xcvr_fs_clk = {
	.b = {
		.ctl_reg = USB_HSIC_XCVR_FS_CLK_NS_REG,
		.en_mask = BIT(9),
		.halt_reg = CLK_HALT_CFPB_STATEA_REG,
		.halt_bit = 28,
	},
	.ns_reg = USB_HSIC_XCVR_FS_CLK_NS_REG,
	.md_reg = USB_HSIC_XCVR_FS_CLK_MD_REG,
	.root_en_mask = BIT(11),
	.ns_mask = (BM(23, 16) | BM(6, 0)),
	.mnd_en_mask = BIT(8),
	.set_rate = set_rate_mnd,
	.freq_tbl = clk_tbl_usb_hsic,
	.current_freq = &rcg_dummy_freq,
	.c = {
		.dbg_name = "usb_hsic_xcvr_fs_clk",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP1(LOW, 60000000),
		CLK_INIT(usb_hsic_xcvr_fs_clk.c),
	},
};

static struct branch_clk usb_hsic_system_clk = {
	.b = {
		.ctl_reg = USB_HSIC_SYSTEM_CLK_CTL_REG,
		.en_mask = BIT(4),
		.reset_reg = USB_HSIC_RESET_REG,
		.reset_mask = BIT(0),
		.halt_reg = CLK_HALT_CFPB_STATEA_REG,
		.halt_bit = 26,
	},
	.parent = &usb_hsic_xcvr_fs_clk.c,
	.c = {
		.dbg_name = "usb_hsic_system_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(usb_hsic_system_clk.c),
	},
};

#define F_USB_HSIC(f, s) \
	{ \
		.freq_hz = f, \
		.src_clk = &s##_clk.c, \
	}
static struct clk_freq_tbl clk_tbl_usb2_hsic[] = {
	F_USB_HSIC(480000000, pll14),
	F_END
};

static struct rcg_clk usb_hsic_hsic_src_clk = {
	.b = {
		.ctl_reg = USB_HSIC_HSIC_CLK_SRC_CTL_REG,
		.halt_check = NOCHECK,
	},
	.root_en_mask = BIT(0),
	.set_rate = set_rate_nop,
	.freq_tbl = clk_tbl_usb2_hsic,
	.current_freq = &rcg_dummy_freq,
	.c = {
		.dbg_name = "usb_hsic_hsic_src_clk",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP1(LOW, 480000000),
		CLK_INIT(usb_hsic_hsic_src_clk.c),
	},
};

static struct branch_clk usb_hsic_hsic_clk = {
	.b = {
		.ctl_reg = USB_HSIC_HSIC_CLK_CTL_REG,
		.en_mask = BIT(0),
		.halt_reg = CLK_HALT_CFPB_STATEA_REG,
		.halt_bit = 21,
	},
	.parent = &usb_hsic_hsic_src_clk.c,
	.c = {
		.dbg_name = "usb_hsic_hsic_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(usb_hsic_hsic_clk.c),
	},
};

#define F_USB_HSIO_CAL(f, s) \
	{ \
		.freq_hz = f, \
		.src_clk = &s##_clk.c, \
	}
static struct clk_freq_tbl clk_tbl_usb_hsio_cal[] = {
	F_USB_HSIO_CAL(9000000, pxo),
	F_END
};

static struct rcg_clk usb_hsic_hsio_cal_clk = {
	.b = {
		.ctl_reg = USB_HSIC_HSIO_CAL_CLK_CTL_REG,
		.en_mask = BIT(0),
		.halt_reg = CLK_HALT_CFPB_STATEA_REG,
		.halt_bit = 25,
	},
	.set_rate = set_rate_nop,
	.freq_tbl = clk_tbl_usb_hsio_cal,
	.current_freq = &rcg_dummy_freq,
	.c = {
		.dbg_name = "usb_hsic_hsio_cal_clk",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP1(LOW, 10000000),
		CLK_INIT(usb_hsic_hsio_cal_clk.c),
	},
};

static struct branch_clk usb_phy0_clk = {
	.b = {
		.reset_reg = USB_PHY0_RESET_REG,
		.reset_mask = BIT(0),
	},
	.c = {
		.dbg_name = "usb_phy0_clk",
		.ops = &clk_ops_reset,
		CLK_INIT(usb_phy0_clk.c),
	},
};

#define CLK_USB_FS(i, n, fmax_nom) \
	struct rcg_clk i##_clk = { \
		.ns_reg = USB_FS1_XCVR_FS_CLK_NS_REG, \
		.b = { \
			.ctl_reg = USB_FS1_XCVR_FS_CLK_NS_REG, \
			.halt_check = NOCHECK, \
		}, \
		.md_reg = USB_FS1_XCVR_FS_CLK_MD_REG, \
		.root_en_mask = BIT(11), \
		.ns_mask = (BM(23, 16) | BM(6, 0)), \
		.mnd_en_mask = BIT(8), \
		.set_rate = set_rate_mnd, \
		.freq_tbl = clk_tbl_usb, \
		.current_freq = &rcg_dummy_freq, \
		.c = { \
			.dbg_name = #i "_clk", \
			.ops = &clk_ops_rcg, \
			VDD_DIG_FMAX_MAP1(NOMINAL, fmax_nom), \
			CLK_INIT(i##_clk.c), \
		}, \
	}

static CLK_USB_FS(usb_fs1_src, 1, 64000000);
static struct branch_clk usb_fs1_xcvr_clk = {
	.b = {
		.ctl_reg = USB_FS1_XCVR_FS_CLK_NS_REG,
		.en_mask = BIT(9),
		.reset_reg = USB_FS1_RESET_REG,
		.reset_mask = BIT(1),
		.halt_reg = CLK_HALT_CFPB_STATEA_REG,
		.halt_bit = 17,
	},
	.parent = &usb_fs1_src_clk.c,
	.c = {
		.dbg_name = "usb_fs1_xcvr_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(usb_fs1_xcvr_clk.c),
	},
};

static struct branch_clk usb_fs1_sys_clk = {
	.b = {
		.ctl_reg = USB_FS1_SYSTEM_CLK_CTL_REG,
		.en_mask = BIT(4),
		.reset_reg = USB_FS1_RESET_REG,
		.reset_mask = BIT(0),
		.halt_reg = CLK_HALT_CFPB_STATEA_REG,
		.halt_bit = 18,
	},
	.parent = &usb_fs1_src_clk.c,
	.c = {
		.dbg_name = "usb_fs1_sys_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(usb_fs1_sys_clk.c),
	},
};

/* Fast Peripheral Bus Clocks */
static struct branch_clk ce1_core_clk = {
	.b = {
		.ctl_reg = CE1_CORE_CLK_CTL_REG,
		.en_mask = BIT(4),
		.hwcg_reg = CE1_CORE_CLK_CTL_REG,
		.hwcg_mask = BIT(6),
		.halt_reg = CLK_HALT_CFPB_STATEC_REG,
		.halt_bit = 27,
	},
	.c = {
		.dbg_name = "ce1_core_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(ce1_core_clk.c),
	},
};

static struct branch_clk ce1_p_clk = {
	.b = {
		.ctl_reg = CE1_HCLK_CTL_REG,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_CFPB_STATEC_REG,
		.halt_bit = 1,
	},
	.c = {
		.dbg_name = "ce1_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(ce1_p_clk.c),
	},
};

#define F_CE5(f, s, d) \
	{ \
		.freq_hz = f, \
		.src_clk = &s##_clk.c, \
		.ns_val = NS_DIVSRC(6, 3, d, 2, 0, s##_to_bb_mux), \
	}

static struct clk_freq_tbl clk_tbl_ce5[] = {
	F_CE5(        0, gnd,   1),
	F_CE5( 48000000, pll8,  8),
	F_END
};

static struct rcg_clk ce5_src_clk = {
	.b = {
		.ctl_reg = CE5_CLK_SRC_NS_REG,
		.halt_check = NOCHECK,
	},
	.ns_reg = CE5_CLK_SRC_NS_REG,
	.root_en_mask = BIT(7),
	.ns_mask = BM(6, 0),
	.set_rate = set_rate_nop,
	.freq_tbl = clk_tbl_ce5,
	.current_freq = &rcg_dummy_freq,
	.c = {
		.dbg_name = "ce5_src_clk",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP2(LOW, 50000000, NOMINAL, 100000000),
		CLK_INIT(ce5_src_clk.c),
	},
};

static struct branch_clk ce5_core_clk = {
	.b = {
		.ctl_reg = CE5_CORE_CLK_CTL_REG,
		.en_mask = BIT(4),
		.reset_reg = CE5_CORE_CLK_CTL_REG,
		.reset_mask = BIT(7),
		.halt_reg = CLK_HALT_MSS_SMPSS_MISC_STATE_REG,
		.halt_bit = 5,
	},
	.parent = &ce5_src_clk.c,
	.c = {
		.dbg_name = "ce5_core_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(ce5_core_clk.c),
	}
};

static struct branch_clk ce5_p_clk = {
	.b = {
		.ctl_reg = CE5_HCLK_CTL_REG,
		.en_mask = BIT(4),
		.reset_reg = CE5_HCLK_CTL_REG,
		.reset_mask = BIT(7),
		.halt_reg = CLK_HALT_NSS_FAB0_NSS_FAB1_STATEA_REG,
		.halt_bit = 11,
	},
	.parent = &ce5_src_clk.c,
	.c = {
		.dbg_name = "ce5_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(ce5_p_clk.c),
	}
};

#define F_SATA(f, s, d) \
	{ \
		.freq_hz = f, \
		.src_clk = &s##_clk.c, \
		.ns_val = NS_DIVSRC(6, 3, d, 2, 0, s##_to_bb_mux), \
	}

static struct clk_freq_tbl clk_tbl_sata[] = {
	F_SATA(        0,  gnd, 1),
	F_SATA(100000000, pll3, 12),
	F_END
};

static struct rcg_clk sata_src_clk = {
	.b = {
		.ctl_reg = SATA_CLK_SRC_NS_REG,
		.halt_check = NOCHECK,
	},
	.ns_reg = SATA_CLK_SRC_NS_REG,
	.root_en_mask = BIT(7),
	.ns_mask = BM(6, 0),
	.set_rate = set_rate_nop,
	.freq_tbl = clk_tbl_sata,
	.current_freq = &rcg_dummy_freq,
	.c = {
		.dbg_name = "sata_src_clk",
		.ops = &clk_ops_rcg,
		VDD_DIG_FMAX_MAP2(LOW, 50000000, NOMINAL, 100000000),
		CLK_INIT(sata_src_clk.c),
	},
};

static struct branch_clk sata_rxoob_clk = {
	.b = {
		.ctl_reg = SATA_RXOOB_CLK_CTL_REG,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_MSS_SMPSS_MISC_STATE_REG,
		.halt_bit = 20,
	},
	.parent = &sata_src_clk.c,
	.c = {
		.dbg_name = "sata_rxoob_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(sata_rxoob_clk.c),
	},
};

static struct branch_clk sata_pmalive_clk = {
	.b = {
		.ctl_reg = SATA_PMALIVE_CLK_CTL_REG,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_MSS_SMPSS_MISC_STATE_REG,
		.halt_bit = 19,
	},
	.parent = &sata_src_clk.c,
	.c = {
		.dbg_name = "sata_pmalive_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(sata_pmalive_clk.c),
	},
};

static struct branch_clk sata_phy_ref_clk = {
	.b = {
		.ctl_reg = SATA_PHY_REF_CLK_CTL_REG,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_MSS_SMPSS_MISC_STATE_REG,
		.halt_bit = 18,
	},
	.parent = &pxo_clk.c,
	.c = {
		.dbg_name = "sata_phy_ref_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(sata_phy_ref_clk.c),
	},
};

static struct branch_clk sata_a_clk = {
	.b = {
		.ctl_reg = SATA_ACLK_CTL_REG,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_AFAB_SFAB_STATEA_REG,
		.halt_bit = 12,
	},
	.c = {
		.dbg_name = "sata_a_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(sata_a_clk.c),
	},
};

static struct branch_clk sata_p_clk = {
	.b = {
		.ctl_reg = SATA_HCLK_CTL_REG,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_MSS_SMPSS_MISC_STATE_REG,
		.halt_bit = 21,
                .reset_reg = SATA_RESET,
		.reset_mask = BIT(0),
	},
	.c = {
		.dbg_name = "sata_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(sata_p_clk.c),
	},
};

static struct branch_clk sfab_sata_s_p_clk = {
	.b = {
		.ctl_reg = SFAB_SATA_S_HCLK_CTL_REG,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_AFAB_SFAB_STATEB_REG,
		.halt_bit = 14,
	},
	.c = {
		.dbg_name = "sfab_sata_s_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(sfab_sata_s_p_clk.c),
	},
};

static struct branch_clk pcie_p_clk = {
	.b = {
		.ctl_reg = PCIE_HCLK_CTL_REG,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_CFPB_STATEC_REG,
		.halt_bit = 8,
	},
	.c = {
		.dbg_name = "pcie_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(pcie_p_clk.c),
	},
};

static struct branch_clk pcie_aux_clk = {
	.b = {
		.ctl_reg = PCIE_AUX_CLK_CTL,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_MSS_SMPSS_MISC_STATE_REG,
		.halt_bit = 31,
	},
	.c = {
		.dbg_name = "pcie_aux_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(pcie_aux_clk.c),
	},
};

static struct branch_clk pcie_phy_ref_clk = {
	.b = {
		.ctl_reg = PCIE_PCLK_CTL_REG,
		.en_mask = BIT(4),
		.halt_check = NOCHECK,
		.halt_reg = CLK_HALT_MSS_SMPSS_MISC_STATE_REG,
		.halt_bit = 29,
	},
	.c = {
		.dbg_name = "pcie_phy_ref_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(pcie_phy_ref_clk.c),
	},
};

static struct branch_clk pcie_a_clk = {
	.b = {
		.ctl_reg = PCIE_ACLK_CTL_REG,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_AFAB_SFAB_STATEA_REG,
		.halt_bit = 13,
	},
	.c = {
		.dbg_name = "pcie_a_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(pcie_a_clk.c),
	},
};

static struct branch_clk pcie1_p_clk = {
	.b = {
		.ctl_reg = PCIE_1_HCLK_CTL_REG,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_CFPB_STATEC_REG,
		.halt_bit = 9,
	},
	.c = {
		.dbg_name = "pcie1_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(pcie1_p_clk.c),
	},
};

static struct branch_clk pcie1_aux_clk = {
	.b = {
		.ctl_reg = PCIE_1_AUX_CLK_CTL,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_MSS_SMPSS_MISC_STATE_REG,
		.halt_bit = 28,
	},
	.c = {
		.dbg_name = "pcie1_aux_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(pcie1_aux_clk.c),
	},
};

static struct branch_clk pcie1_phy_ref_clk = {
	.b = {
		.ctl_reg = PCIE_1_PCLK_CTL_REG,
		.en_mask = BIT(4),
		.halt_check = NOCHECK,
		.halt_reg = CLK_HALT_MSS_SMPSS_MISC_STATE_REG,
		.halt_bit = 26,
	},
	.c = {
		.dbg_name = "pcie1_phy_ref_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(pcie1_phy_ref_clk.c),
	},
};

static struct branch_clk pcie1_a_clk = {
	.b = {
		.ctl_reg = PCIE_1_ACLK_CTL_REG,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_AFAB_SFAB_STATEA_REG,
		.halt_bit = 10,
	},
	.c = {
		.dbg_name = "pcie1_a_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(pcie1_a_clk.c),
	},
};

static struct branch_clk pcie2_p_clk = {
	.b = {
		.ctl_reg = PCIE_2_HCLK_CTL_REG,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_CFPB_STATEC_REG,
		.halt_bit = 10,
	},
	.c = {
		.dbg_name = "pcie2_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(pcie2_p_clk.c),
	},
};

static struct branch_clk pcie2_aux_clk = {
	.b = {
		.ctl_reg = PCIE_2_AUX_CLK_CTL,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_MSS_SMPSS_MISC_STATE_REG,
		.halt_bit = 25,
	},
	.c = {
		.dbg_name = "pcie2_aux_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(pcie2_aux_clk.c),
	},
};

static struct branch_clk pcie2_phy_ref_clk = {
	.b = {
		.ctl_reg = PCIE_2_PCLK_CTL_REG,
		.en_mask = BIT(4),
		.halt_check = NOCHECK,
		.halt_reg = CLK_HALT_MSS_SMPSS_MISC_STATE_REG,
		.halt_bit = 23,
	},
	.c = {
		.dbg_name = "pcie2_phy_ref_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(pcie2_phy_ref_clk.c),
	},
};

static struct branch_clk pcie2_a_clk = {
	.b = {
		.ctl_reg = PCIE_2_ACLK_CTL_REG,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_AFAB_SFAB_STATEA_REG,
		.halt_bit = 9,
	},
	.c = {
		.dbg_name = "pcie2_a_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(pcie2_a_clk.c),
	},
};

#define F_PCIe(f, s, d) \
	{ \
		.freq_hz = f, \
		.src_clk = &s##_clk.c, \
		.ns_val = NS_DIVSRC(6, 3, d, 2, 0, s##_to_pcie_mux), \
	}

static struct clk_freq_tbl clk_tbl_pcie[] = {
	F_PCIe( 100000000, pll3, 12),
	F_END
};

static struct rcg_clk pcie_src_clk = {
	.b = {
		.ctl_reg = PCIE_ALT_REF_CLK_NS,
		.halt_check = NOCHECK,
	},
	.ns_reg = PCIE_ALT_REF_CLK_NS,
	.root_en_mask = BIT(11),
	.ns_mask = BM(6, 3) | BIT(0),
	.set_rate = set_rate_nop,
	.freq_tbl = clk_tbl_pcie,
	.current_freq = &rcg_dummy_freq,
	.c = {
		.dbg_name = "pcie_src_clk",
		.ops = &clk_ops_rcg,
		CLK_INIT(pcie_src_clk.c),
	},
};

static struct rcg_clk pcie1_src_clk = {
	.b = {
		.ctl_reg = PCIE_1_ALT_REF_CLK_NS,
		.halt_check = NOCHECK,
	},
	.ns_reg = PCIE_1_ALT_REF_CLK_NS,
	.root_en_mask = BIT(11),
	.ns_mask = BM(6, 3) | BIT(0),
	.set_rate = set_rate_nop,
	.freq_tbl = clk_tbl_pcie,
	.current_freq = &rcg_dummy_freq,
	.c = {
		.dbg_name = "pcie1_src_clk",
		.ops = &clk_ops_rcg,
		CLK_INIT(pcie1_src_clk.c),
	},
};

static struct rcg_clk pcie2_src_clk = {
	.b = {
		.ctl_reg = PCIE_2_ALT_REF_CLK_NS,
		.halt_check = NOCHECK,
	},
	.ns_reg = PCIE_2_ALT_REF_CLK_NS,
	.root_en_mask = BIT(11),
	.ns_mask = BM(6, 3) | BIT(0),
	.set_rate = set_rate_nop,
	.freq_tbl = clk_tbl_pcie,
	.current_freq = &rcg_dummy_freq,
	.c = {
		.dbg_name = "pcie2_src_clk",
		.ops = &clk_ops_rcg,
		CLK_INIT(pcie2_src_clk.c),
	},
};

static struct branch_clk dma_bam_p_clk = {
	.b = {
		.ctl_reg = DMA_BAM_HCLK_CTL,
		.en_mask = BIT(4),
		.hwcg_reg = DMA_BAM_HCLK_CTL,
		.hwcg_mask = BIT(6),
		.halt_reg = CLK_HALT_DFAB_STATE_REG,
		.halt_bit = 12,
	},
	.c = {
		.dbg_name = "dma_bam_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(dma_bam_p_clk.c),
	},
};

static struct branch_clk gsbi1_p_clk = {
	.b = {
		.ctl_reg = GSBIn_HCLK_CTL_REG(1),
		.en_mask = BIT(4),
		.hwcg_reg = GSBIn_HCLK_CTL_REG(1),
		.hwcg_mask = BIT(6),
		.halt_reg = CLK_HALT_CFPB_STATEA_REG,
		.halt_bit = 13,
	},
	.c = {
		.dbg_name = "gsbi1_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gsbi1_p_clk.c),
	},
};

static struct branch_clk gsbi2_p_clk = {
	.b = {
		.ctl_reg = GSBIn_HCLK_CTL_REG(2),
		.en_mask = BIT(4),
		.hwcg_reg = GSBIn_HCLK_CTL_REG(2),
		.hwcg_mask = BIT(6),
		.halt_reg = CLK_HALT_CFPB_STATEA_REG,
		.halt_bit = 9,
	},
	.c = {
		.dbg_name = "gsbi2_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gsbi2_p_clk.c),
	},
};

static struct branch_clk gsbi4_p_clk = {
	.b = {
		.ctl_reg = GSBIn_HCLK_CTL_REG(4),
		.en_mask = BIT(4),
		.hwcg_reg = GSBIn_HCLK_CTL_REG(4),
		.hwcg_mask = BIT(6),
		.halt_reg = CLK_HALT_CFPB_STATEB_REG,
		.halt_bit = 27,
	},
	.c = {
		.dbg_name = "gsbi4_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gsbi4_p_clk.c),
	},
};

static struct branch_clk gsbi5_p_clk = {
	.b = {
		.ctl_reg = GSBIn_HCLK_CTL_REG(5),
		.en_mask = BIT(4),
		.hwcg_reg = GSBIn_HCLK_CTL_REG(5),
		.hwcg_mask = BIT(6),
		.halt_reg = CLK_HALT_CFPB_STATEB_REG,
		.halt_bit = 23,
	},
	.c = {
		.dbg_name = "gsbi5_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gsbi5_p_clk.c),
	},
};

static struct branch_clk gsbi6_p_clk = {
	.b = {
		.ctl_reg = GSBIn_HCLK_CTL_REG(6),
		.en_mask = BIT(4),
		.hwcg_reg = GSBIn_HCLK_CTL_REG(6),
		.hwcg_mask = BIT(6),
		.halt_reg = CLK_HALT_CFPB_STATEB_REG,
		.halt_bit = 19,
	},
	.c = {
		.dbg_name = "gsbi6_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gsbi6_p_clk.c),
	},
};

static struct branch_clk gsbi7_p_clk = {
	.b = {
		.ctl_reg = GSBIn_HCLK_CTL_REG(7),
		.en_mask = BIT(4),
		.hwcg_reg = GSBIn_HCLK_CTL_REG(7),
		.hwcg_mask = BIT(6),
		.halt_reg = CLK_HALT_CFPB_STATEB_REG,
		.halt_bit = 15,
	},
	.c = {
		.dbg_name = "gsbi7_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(gsbi7_p_clk.c),
	},
};

static struct branch_clk sata_phy_cfg_clk = {
	.b = {
		.ctl_reg = SATA_PHY_CFG_CLK_CTL_REG,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_CFPB_STATEA_REG,
		.halt_bit = 14,
	},
	.c = {
		.dbg_name = "sata_phy_cfg_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(sata_phy_cfg_clk.c),
	},
};

static struct branch_clk tsif_p_clk = {
	.b = {
		.ctl_reg = TSIF_HCLK_CTL_REG,
		.en_mask = BIT(4),
		.hwcg_reg = TSIF_HCLK_CTL_REG,
		.hwcg_mask = BIT(6),
		.halt_reg = CLK_HALT_CFPB_STATEC_REG,
		.halt_bit = 7,
	},
	.c = {
		.dbg_name = "tsif_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(tsif_p_clk.c),
	},
};

static struct branch_clk usb_fs1_p_clk = {
	.b = {
		.ctl_reg = USB_FS1_HCLK_CTL_REG,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_CFPB_STATEA_REG,
		.halt_bit = 19,
	},
	.c = {
		.dbg_name = "usb_fs1_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(usb_fs1_p_clk.c),
	},
};


static struct branch_clk usb_hs1_p_clk = {
	.b = {
		.ctl_reg = USB_HS1_HCLK_CTL_REG,
		.en_mask = BIT(4),
		.hwcg_reg = USB_HS1_HCLK_CTL_REG,
		.hwcg_mask = BIT(6),
		.halt_reg = CLK_HALT_DFAB_STATE_REG,
		.halt_bit = 1,
	},
	.c = {
		.dbg_name = "usb_hs1_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(usb_hs1_p_clk.c),
	},
};

static struct branch_clk usb_hsic_p_clk = {
	.b = {
		.ctl_reg = USB_HSIC_HCLK_CTL_REG,
		.en_mask = BIT(4),
		.halt_reg = CLK_HALT_CFPB_STATEA_REG,
		.halt_bit = 30,
	},
	.c = {
		.dbg_name = "usb_hsic_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(usb_hsic_p_clk.c),
	},
};

static struct branch_clk sdc1_p_clk = {
	.b = {
		.ctl_reg = SDCn_HCLK_CTL_REG(1),
		.en_mask = BIT(4),
		.hwcg_reg = SDCn_HCLK_CTL_REG(1),
		.hwcg_mask = BIT(6),
		.halt_reg = CLK_HALT_DFAB_STATE_REG,
		.halt_bit = 11,
	},
	.c = {
		.dbg_name = "sdc1_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(sdc1_p_clk.c),
	},
};

static struct branch_clk sdc3_p_clk = {
	.b = {
		.ctl_reg = SDCn_HCLK_CTL_REG(3),
		.en_mask = BIT(4),
		.hwcg_reg = SDCn_HCLK_CTL_REG(3),
		.hwcg_mask = BIT(6),
		.halt_reg = CLK_HALT_DFAB_STATE_REG,
		.halt_bit = 9,
	},
	.c = {
		.dbg_name = "sdc3_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(sdc3_p_clk.c),
	},
};

/* HW-Voteable Clocks */
static struct branch_clk adm0_clk = {
	.b = {
		.ctl_reg = SC0_U_CLK_BRANCH_ENA_VOTE_REG,
		.en_mask = BIT(2),
		.halt_reg = CLK_HALT_MSS_SMPSS_MISC_STATE_REG,
		.halt_check = HALT_VOTED,
		.halt_bit = 12,
	},
	.c = {
		.dbg_name = "adm0_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(adm0_clk.c),
	},
};

static struct branch_clk adm0_p_clk = {
	.b = {
		.ctl_reg = SC0_U_CLK_BRANCH_ENA_VOTE_REG,
		.en_mask = BIT(3),
		.hwcg_reg = ADM0_PBUS_CLK_CTL_REG,
		.hwcg_mask = BIT(6),
		.halt_reg = CLK_HALT_MSS_SMPSS_MISC_STATE_REG,
		.halt_check = HALT_VOTED,
		.halt_bit = 11,
	},
	.c = {
		.dbg_name = "adm0_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(adm0_p_clk.c),
	},
};

static struct branch_clk pmic_arb0_p_clk = {
	.b = {
		.ctl_reg = SC0_U_CLK_BRANCH_ENA_VOTE_REG,
		.en_mask = BIT(8),
		.halt_reg = CLK_HALT_SFPB_MISC_STATE_REG,
		.halt_check = HALT_VOTED,
		.halt_bit = 22,
	},
	.c = {
		.dbg_name = "pmic_arb0_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(pmic_arb0_p_clk.c),
	},
};

static struct branch_clk pmic_arb1_p_clk = {
	.b = {
		.ctl_reg = SC0_U_CLK_BRANCH_ENA_VOTE_REG,
		.en_mask = BIT(9),
		.halt_reg = CLK_HALT_SFPB_MISC_STATE_REG,
		.halt_check = HALT_VOTED,
		.halt_bit = 21,
	},
	.c = {
		.dbg_name = "pmic_arb1_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(pmic_arb1_p_clk.c),
	},
};

static struct branch_clk pmic_ssbi2_clk = {
	.b = {
		.ctl_reg = SC0_U_CLK_BRANCH_ENA_VOTE_REG,
		.en_mask = BIT(7),
		.halt_reg = CLK_HALT_SFPB_MISC_STATE_REG,
		.halt_check = HALT_VOTED,
		.halt_bit = 23,
	},
	.c = {
		.dbg_name = "pmic_ssbi2_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(pmic_ssbi2_clk.c),
	},
};

static struct branch_clk rpm_msg_ram_p_clk = {
	.b = {
		.ctl_reg = SC0_U_CLK_BRANCH_ENA_VOTE_REG,
		.en_mask = BIT(6),
		.hwcg_reg = RPM_MSG_RAM_HCLK_CTL_REG,
		.hwcg_mask = BIT(6),
		.halt_reg = CLK_HALT_SFPB_MISC_STATE_REG,
		.halt_check = HALT_VOTED,
		.halt_bit = 12,
	},
	.c = {
		.dbg_name = "rpm_msg_ram_p_clk",
		.ops = &clk_ops_branch,
		CLK_INIT(rpm_msg_ram_p_clk.c),
	},
};

/*
 * Multimedia Clocks
 */


struct pix_rdi_clk {
	bool prepared;
	bool enabled;
	unsigned long cur_rate;

	void __iomem *const s_reg;
	u32 s_mask;

	void __iomem *const s2_reg;
	u32 s2_mask;

	struct branch b;
	struct clk c;
};

static inline struct pix_rdi_clk *to_pix_rdi_clk(struct clk *c)
{
	return container_of(c, struct pix_rdi_clk, c);
}

DEFINE_CLK_RPM(afab_clk, afab_a_clk, APPS_FABRIC, NULL);
DEFINE_CLK_RPM(cfpb_clk, cfpb_a_clk, CFPB, NULL);
DEFINE_CLK_RPM(dfab_clk, dfab_a_clk, DAYTONA_FABRIC, NULL);
DEFINE_CLK_RPM(ebi1_clk, ebi1_a_clk, EBI1, NULL);
DEFINE_CLK_RPM(sfab_clk, sfab_a_clk, SYSTEM_FABRIC, NULL);
DEFINE_CLK_RPM(sfpb_clk, sfpb_a_clk, SFPB, NULL);
DEFINE_CLK_RPM_QDSS(qdss_clk, qdss_a_clk);
DEFINE_CLK_RPM(nssfab0_clk, nssfab0_a_clk, NSS_FABRIC_0, NULL);
DEFINE_CLK_RPM(nssfab1_clk, nssfab1_a_clk, NSS_FABRIC_1, NULL);

static DEFINE_CLK_VOTER(sfab_msmbus_a_clk, &sfab_a_clk.c, 0);
static DEFINE_CLK_VOTER(sfab_tmr_a_clk, &sfab_a_clk.c, 0);

static DEFINE_CLK_VOTER(dfab_dsps_clk, &dfab_clk.c, 0);
static DEFINE_CLK_VOTER(dfab_usb_hs_clk, &dfab_clk.c, 0);
static DEFINE_CLK_VOTER(dfab_sdc1_clk, &dfab_clk.c, 0);
static DEFINE_CLK_VOTER(dfab_sdc3_clk, &dfab_clk.c, 0);
static DEFINE_CLK_VOTER(dfab_sps_clk, &dfab_clk.c, 0);
static DEFINE_CLK_VOTER(dfab_bam_dmux_clk, &dfab_clk.c, 0);
static DEFINE_CLK_VOTER(dfab_scm_clk, &dfab_clk.c, 0);
static DEFINE_CLK_VOTER(dfab_msmbus_clk, &dfab_clk.c, 0);
static DEFINE_CLK_VOTER(dfab_msmbus_a_clk, &dfab_a_clk.c, 0);

static DEFINE_CLK_VOTER(ebi1_msmbus_clk, &ebi1_clk.c, LONG_MAX);
static DEFINE_CLK_VOTER(ebi1_adm_clk, &ebi1_clk.c, 0);

static DEFINE_CLK_VOTER(ebi1_acpu_a_clk, &ebi1_a_clk.c, LONG_MAX);
static DEFINE_CLK_VOTER(ebi1_msmbus_a_clk, &ebi1_a_clk.c, LONG_MAX);
static DEFINE_CLK_VOTER(afab_acpu_a_clk, &afab_a_clk.c, LONG_MAX);
static DEFINE_CLK_VOTER(afab_msmbus_a_clk, &afab_a_clk.c, LONG_MAX);

#ifdef CONFIG_DEBUG_FS
struct measure_sel {
	u32 test_vector;
	struct clk *c;
};

static DEFINE_CLK_MEASURE(l2_m_clk);
static DEFINE_CLK_MEASURE(krait0_m_clk);
static DEFINE_CLK_MEASURE(krait1_m_clk);

static struct measure_sel measure_mux[] = {
	{ TEST_PER_LS(0x12), &sdc1_p_clk.c },
	{ TEST_PER_LS(0x13), &sdc1_clk.c },
	{ TEST_PER_LS(0x16), &sdc3_p_clk.c },
	{ TEST_PER_LS(0x17), &sdc3_clk.c },
	{ TEST_PER_LS(0x1F), &gp0_clk.c },
	{ TEST_PER_LS(0x20), &gp1_clk.c },
	{ TEST_PER_LS(0x21), &gp2_clk.c },
	{ TEST_PER_LS(0x25), &dfab_clk.c },
	{ TEST_PER_LS(0x25), &dfab_a_clk.c },
	{ TEST_PER_LS(0x26), &pmem_clk.c },
	{ TEST_PER_LS(0x32), &dma_bam_p_clk.c },
	{ TEST_PER_LS(0x33), &cfpb_clk.c },
	{ TEST_PER_LS(0x33), &cfpb_a_clk.c },
	{ TEST_PER_LS(0x3D), &gsbi1_p_clk.c },
	{ TEST_PER_LS(0x3E), &gsbi1_uart_clk.c },
	{ TEST_PER_LS(0x3F), &gsbi1_qup_clk.c },
	{ TEST_PER_LS(0x41), &gsbi2_p_clk.c },
	{ TEST_PER_LS(0x42), &gsbi2_uart_clk.c },
	{ TEST_PER_LS(0x44), &gsbi2_qup_clk.c },
	{ TEST_PER_LS(0x49), &gsbi4_p_clk.c },
	{ TEST_PER_LS(0x4A), &gsbi4_uart_clk.c },
	{ TEST_PER_LS(0x4D), &gsbi5_p_clk.c },
	{ TEST_PER_LS(0x4E), &gsbi5_uart_clk.c },
	{ TEST_PER_LS(0x50), &gsbi5_qup_clk.c },
	{ TEST_PER_LS(0x51), &gsbi6_p_clk.c },
	{ TEST_PER_LS(0x52), &gsbi6_uart_clk.c },
	{ TEST_PER_LS(0x54), &gsbi6_qup_clk.c },
	{ TEST_PER_LS(0x55), &gsbi7_p_clk.c },
	{ TEST_PER_LS(0x56), &gsbi7_uart_clk.c },
	{ TEST_PER_LS(0x58), &gsbi7_qup_clk.c },
	{ TEST_PER_LS(0x59), &sfab_sata_s_p_clk.c },
	{ TEST_PER_LS(0x5A), &sata_p_clk.c },
	{ TEST_PER_LS(0x5B), &sata_rxoob_clk.c },
	{ TEST_PER_LS(0x5C), &sata_pmalive_clk.c },
	{ TEST_PER_LS(0x5D), &pcie_src_clk.c },
	{ TEST_PER_LS(0x5E), &pcie_p_clk.c },
	{ TEST_PER_LS(0x5F), &ce5_p_clk.c },
	{ TEST_PER_LS(0x60), &ce5_core_clk.c },
	{ TEST_PER_LS(0x6B), &sata_phy_ref_clk.c },
	{ TEST_PER_LS(0x6C), &sata_phy_cfg_clk.c },
	{ TEST_PER_LS(0x78), &sfpb_clk.c },
	{ TEST_PER_LS(0x78), &sfpb_a_clk.c },
	{ TEST_PER_LS(0x7A), &pmic_ssbi2_clk.c },
	{ TEST_PER_LS(0x7B), &pmic_arb0_p_clk.c },
	{ TEST_PER_LS(0x7C), &pmic_arb1_p_clk.c },
	{ TEST_PER_LS(0x7D), &prng_clk.c },
	{ TEST_PER_LS(0x7F), &rpm_msg_ram_p_clk.c },
	{ TEST_PER_LS(0x80), &adm0_p_clk.c },
	{ TEST_PER_LS(0x84), &usb_hs1_p_clk.c },
	{ TEST_PER_LS(0x85), &usb_hs1_xcvr_clk.c },
	{ TEST_PER_LS(0x86), &usb_hsic_p_clk.c },
	{ TEST_PER_LS(0x87), &usb_hsic_system_clk.c },
	{ TEST_PER_LS(0x88), &usb_hsic_xcvr_fs_clk.c },
	{ TEST_PER_LS(0x89), &usb_fs1_p_clk.c },
	{ TEST_PER_LS(0x8A), &usb_fs1_sys_clk.c },
	{ TEST_PER_LS(0x8B), &usb_fs1_xcvr_clk.c },
	{ TEST_PER_LS(0x8F), &tsif_p_clk.c },
	{ TEST_PER_LS(0x91), &tsif_ref_clk.c },
	{ TEST_PER_LS(0x92), &ce1_p_clk.c },
	{ TEST_PER_LS(0x94), &tssc_clk.c },
	{ TEST_PER_LS(0x9D), &usb_hsic_hsio_cal_clk.c },
	{ TEST_PER_LS(0xA4), &ce1_core_clk.c },
	{ TEST_PER_LS(0xB0), &pcie1_p_clk.c },
	{ TEST_PER_LS(0xB1), &pcie1_src_clk.c },
	{ TEST_PER_LS(0xB2), &pcie2_p_clk.c },
	{ TEST_PER_LS(0xB3), &pcie2_src_clk.c },

	{ TEST_PER_HS(0x07), &afab_clk.c },
	{ TEST_PER_HS(0x07), &afab_a_clk.c },
	{ TEST_PER_HS(0x18), &sfab_clk.c },
	{ TEST_PER_HS(0x18), &sfab_a_clk.c },
	{ TEST_PER_HS(0x2A), &adm0_clk.c },
	{ TEST_PER_HS(0x31), &sata_a_clk.c },
	{ TEST_PER_HS(0x2B), &pcie_aux_clk.c },
	{ TEST_PER_HS(0x2D), &pcie_phy_ref_clk.c },
	{ TEST_PER_HS(0x32), &pcie_a_clk.c },
	{ TEST_PER_HS(0x34), &ebi1_clk.c },
	{ TEST_PER_HS(0x34), &ebi1_a_clk.c },
	{ TEST_PER_HS(0x50), &usb_hsic_hsic_clk.c },
	{ TEST_PER_HS(0x55), &pcie1_aux_clk.c },
	{ TEST_PER_HS(0x56), &pcie1_phy_ref_clk.c },
	{ TEST_PER_HS(0x57), &pcie2_aux_clk.c },
	{ TEST_PER_HS(0x58), &pcie2_phy_ref_clk.c },
	{ TEST_PER_HS(0x66), &pcie1_a_clk.c },
	{ TEST_PER_HS(0x67), &pcie2_a_clk.c },

	{ TEST_CPUL2(0x2), &l2_m_clk },
	{ TEST_CPUL2(0x0), &krait0_m_clk },
	{ TEST_CPUL2(0x1), &krait1_m_clk },
};

static struct measure_sel *find_measure_sel(struct clk *c)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(measure_mux); i++)
		if (measure_mux[i].c == c)
			return &measure_mux[i];
	return NULL;
}

static int measure_clk_set_parent(struct clk *c, struct clk *parent)
{
	int ret = 0;
	u32 clk_sel;
	struct measure_sel *p;
	struct measure_clk *measure = to_measure_clk(c);
	unsigned long flags;

	if (!parent)
		return -EINVAL;

	p = find_measure_sel(parent);
	if (!p)
		return -EINVAL;

	spin_lock_irqsave(&local_clock_reg_lock, flags);

	/*
	 * Program the test vector, measurement period (sample_ticks)
	 * and scaling multiplier.
	 */
	measure->sample_ticks = 0x10000;
	clk_sel = p->test_vector & TEST_CLK_SEL_MASK;
	measure->multiplier = 1;
	switch (p->test_vector >> TEST_TYPE_SHIFT) {
	case TEST_TYPE_PER_LS:
		writel_relaxed(0x4030D00|BVAL(7, 0, clk_sel), CLK_TEST_REG);
		break;
	case TEST_TYPE_PER_HS:
		writel_relaxed(0x4020000|BVAL(16, 10, clk_sel), CLK_TEST_REG);
		break;
	case TEST_TYPE_CPUL2:
		writel_relaxed(0x4030400, CLK_TEST_REG);
		writel_relaxed(0x80|BVAL(5, 3, clk_sel), GCC_APCS_CLK_DIAG);
		measure->sample_ticks = 0x4000;
		measure->multiplier = 2;
		if (cpu_is_krait_v3())
			measure->multiplier = 8;
		break;
	default:
		ret = -EPERM;
	}
	/* Make sure test vector is set before starting measurements. */
	mb();

	spin_unlock_irqrestore(&local_clock_reg_lock, flags);

	return ret;
}

/* Sample clock for 'ticks' reference clock ticks. */
static u32 run_measurement(unsigned ticks)
{
	/* Stop counters and set the XO4 counter start value. */
	writel_relaxed(ticks, RINGOSC_TCXO_CTL_REG);

	/* Wait for timer to become ready. */
	while ((readl_relaxed(RINGOSC_STATUS_REG) & BIT(25)) != 0)
		cpu_relax();

	/* Run measurement and wait for completion. */
	writel_relaxed(BIT(20)|ticks, RINGOSC_TCXO_CTL_REG);
	while ((readl_relaxed(RINGOSC_STATUS_REG) & BIT(25)) == 0)
		cpu_relax();

	/* Stop counters. */
	writel_relaxed(0x0, RINGOSC_TCXO_CTL_REG);

	/* Return measured ticks. */
	return readl_relaxed(RINGOSC_STATUS_REG) & BM(24, 0);
}


/* Perform a hardware rate measurement for a given clock.
   FOR DEBUG USE ONLY: Measurements take ~15 ms! */
static unsigned long measure_clk_get_rate(struct clk *c)
{
	unsigned long flags;
	u32 pdm_reg_backup, ringosc_reg_backup;
	u64 raw_count_short, raw_count_full;
	struct measure_clk *measure = to_measure_clk(c);
	unsigned ret;

	ret = clk_prepare_enable(&cxo_clk.c);
	if (ret) {
		pr_warning("CXO clock failed to enable. Can't measure\n");
		return 0;
	}

	spin_lock_irqsave(&local_clock_reg_lock, flags);

	/* Enable CXO/4 and RINGOSC branch and root. */
	pdm_reg_backup = readl_relaxed(PDM_CLK_NS_REG);
	ringosc_reg_backup = readl_relaxed(RINGOSC_NS_REG);
	writel_relaxed(0x2898, PDM_CLK_NS_REG);
	writel_relaxed(0xA00, RINGOSC_NS_REG);

	/*
	 * The ring oscillator counter will not reset if the measured clock
	 * is not running.  To detect this, run a short measurement before
	 * the full measurement.  If the raw results of the two are the same
	 * then the clock must be off.
	 */

	/* Run a short measurement. (~1 ms) */
	raw_count_short = run_measurement(0x1000);
	/* Run a full measurement. (~14 ms) */
	raw_count_full = run_measurement(measure->sample_ticks);

	writel_relaxed(ringosc_reg_backup, RINGOSC_NS_REG);
	writel_relaxed(pdm_reg_backup, PDM_CLK_NS_REG);

	/* Return 0 if the clock is off. */
	if (raw_count_full == raw_count_short)
		ret = 0;
	else {
		/* Compute rate in Hz. */
		raw_count_full = ((raw_count_full * 10) + 15) *
							(IPQ_CXO_FREQ / 4);
		do_div(raw_count_full, ((measure->sample_ticks * 10) + 35));
		ret = (raw_count_full * measure->multiplier);
	}

	/* Route dbg_hs_clk to PLLTEST.  300mV single-ended amplitude. */
	writel_relaxed(0x38F8, PLLTEST_PAD_CFG_REG);
	spin_unlock_irqrestore(&local_clock_reg_lock, flags);

	clk_disable_unprepare(&cxo_clk.c);

	return ret;
}
#else /* !CONFIG_DEBUG_FS */
static int measure_clk_set_parent(struct clk *c, struct clk *parent)
{
	return -EINVAL;
}

static unsigned long measure_clk_get_rate(struct clk *c)
{
	return 0;
}
#endif /* CONFIG_DEBUG_FS */

static struct clk_ops clk_ops_measure = {
	.set_parent = measure_clk_set_parent,
	.get_rate = measure_clk_get_rate,
};

static struct measure_clk measure_clk = {
	.c = {
		.dbg_name = "measure_clk",
		.ops = &clk_ops_measure,
		CLK_INIT(measure_clk.c),
	},
	.multiplier = 1,
};

#define NSS_PLL_RATE(l, m, n, v, d, i) \
	{  \
		.l_val = l, \
		.m_val = m, \
		.n_val = n, \
		.vco = v, \
		.post_div = d, \
		.i_bits = i, \
	}

/*
 * Currently unable to use Macros for Bitwise operator
 * TODO: Waiting for framework to be checked and clean up
 */
#define F_NSS_CORE(f, s, m, n, p_r) \
	{ \
		.freq_hz = f, \
		.src_clk = &s##_clk.c, \
		.md_val = m, \
		.ns_val = n, \
		.extra_freq_data = p_r, \
	}

static struct pll_clk pll18_clk = {
	.mode_reg = PLL18_MODE,
	.parent = &pxo_clk.c,
	.c = {
		.dbg_name = "pll18_clk",
		.rate = 0, /* TODO: Detect rate dynamically */
		.ops = &clk_ops_local_pll,
		CLK_INIT(pll18_clk.c),
	},
};

static struct pll_rate pll18_rate[] = {
	[0] = NSS_PLL_RATE( 0x4000042C, 0x00, 0x01, 0x1, 0x2, 0x01495625),	/*  PLL Values for 550Mhz */
	[1] = NSS_PLL_RATE( 0x4000043A, 0x10, 0x19, 0x1, 0x2, 0x014B5625),	/*  PLL Values for 733Mhz */
};

static struct clk_freq_tbl clk_tbl_nss[] = {
	F_NSS_CORE( 110000000, pll18, 0x0100fa, 0xfb0141, &pll18_rate[0]),	/* 110Mhz */
	F_NSS_CORE( 275000000, pll18, 0x0100fd, 0xfe0141, &pll18_rate[0]),	/* 275Mhz */
	F_NSS_CORE( 550000000, pll18, 0xff00ff, 0xff0001, &pll18_rate[0]),	/* 550Mhz */
	F_NSS_CORE( 733000000, pll18, 0xff00ff, 0xff0001, &pll18_rate[1]),	/* 733Mhz */
	F_END
};

/*
 * Custom Table to handle programming both cores at same time
 * TODO: Waiting for framework to be checked and clean up
 */
struct nss_core_clk_tbl {
	bool prepared;
	bool enabled;
	unsigned long cur_rate;

	void __iomem *const ctl0_src_reg;
	void __iomem *const ctl1_src_reg;

	uint32_t ns_mask;
	void __iomem *const ns0_reg;
	void __iomem *const ns1_reg;

	uint32_t md_mask;
	void __iomem *const md0_reg;
	void __iomem *const md1_reg;

	uint32_t mnd_en_mask;
	uint32_t root_en_mask;

	void   (*set_rate)(struct nss_core_clk_tbl *, struct clk_freq_tbl *);

	struct clk_freq_tbl *freq_tbl;
        struct clk_freq_tbl *current_freq;

	struct branch b;
	struct branch b1;
	struct clk c;
};

static inline struct nss_core_clk_tbl *to_nss_core_clk(struct clk *c)
{
	return container_of(c, struct nss_core_clk_tbl, c);
}

static int nss_core_clk_set_rate(struct clk *c, unsigned long rate)
{
	struct nss_core_clk_tbl *ncc = to_nss_core_clk(c);
	struct clk_freq_tbl *nf, *cf;

	for (nf = ncc->freq_tbl; nf->freq_hz != FREQ_END
			&& nf->freq_hz != rate; nf++)
		;

	if (nf->freq_hz == FREQ_END)
		return -EINVAL;

	cf = ncc->current_freq;

	spin_lock(&local_clock_reg_lock);

	ncc->set_rate(ncc, nf);
	ncc->current_freq = nf;

	spin_unlock(&local_clock_reg_lock);

	return 0;
}

static unsigned long nss_core_clk_get_rate(struct clk *c)
{
	printk("nss_core_clk_get_rate() not implemented\n");
	return 0;
}

static int nss_core_clk_prepare(struct clk *c)
{
	struct nss_core_clk_tbl *ncc = to_nss_core_clk(c);

	WARN(ncc->current_freq == &rcg_dummy_freq,
		"Attempting to prepare %s before setting its rate. "
		"Set the rate first!\n", ncc->c.dbg_name);
	return 0;
}

static int nss_core_clk_enable(struct clk *c)
{
	unsigned long flags;
	struct nss_core_clk_tbl *ncc = to_nss_core_clk(c);

	spin_lock_irqsave(&local_clock_reg_lock, flags);
	ncc->enabled = true;
	spin_unlock_irqrestore(&local_clock_reg_lock, flags);

	return 0;
}

static void nss_core_clk_disable(struct clk *c)
{
	printk("nss_core_clk_disable() not implemented \n");
}

static void nss_core_clk_unprepare(struct clk *c)
{
	struct nss_core_clk_tbl *ncc = to_nss_core_clk(c);
	ncc->prepared = false;
}

static struct clk *nss_core_clk_get_parent(struct clk *c)
{
	return to_nss_core_clk(c)->current_freq->src_clk;
}

static int nss_core_clk_list_rate(struct clk *c, unsigned n)
{
	printk("nss_core_clk_list_rate() not implemented \n");
	return -ENXIO;
}

static int nss_core_clk_reset(struct clk *c, enum clk_reset_action action)
{
	return branch_reset(&to_nss_core_clk(c)->b, action);
}

static enum handoff nss_core_clk_handoff(struct clk *c)
{
	return HANDOFF_DISABLED_CLK;
}

/*
 * TODO - Clean up code to use banking
 */
static void set_rate_nss(struct nss_core_clk_tbl *ncc, struct clk_freq_tbl *nf)
{
	struct pll_rate *rate = nf->extra_freq_data;

	uint32_t ctl_reg0;
	uint32_t ctl_reg1;

	const uint32_t pll_cl_mask = 0x1;
	const uint32_t pll11_mask = 0x1;
	const uint32_t pll18_mask = 0x0;

	uint32_t wait_cycles = 10000000;
	volatile uint32_t value;
	volatile uint32_t en_mask = (1 << 18);
	volatile uint32_t status_mask = (1 << 2);

	/*
	 * Switch to Stable PLL
	 */
	writel_relaxed(0x0100fd, UBI32_COREn_CLK_SRC1_MD(1));
	writel_relaxed(0x0100fd, UBI32_COREn_CLK_SRC1_MD(2));
	writel_relaxed(0xfe014a, UBI32_COREn_CLK_SRC1_NS(1));
	writel_relaxed(0xfe014a, UBI32_COREn_CLK_SRC1_NS(2));

	ctl_reg0 = readl_relaxed(ncc->ctl0_src_reg);
	ctl_reg1 = readl_relaxed(ncc->ctl1_src_reg);
	ctl_reg0 &= ~pll_cl_mask;
	ctl_reg1 &= ~pll_cl_mask;
	ctl_reg0 |= pll11_mask;
	ctl_reg1 |= pll11_mask;
	writel_relaxed(ctl_reg0, ncc->ctl0_src_reg);
	writel_relaxed(ctl_reg1, ncc->ctl1_src_reg);

	/*
	 * Start with clean slate
	 * 	Programming Sequence Provided by HW Team
	 */
	writel_relaxed(0, PLL18_MODE);

	/*
	 * Program PLL
	 */
	writel_relaxed(rate->l_val, PLL18_L_VAL);
	writel_relaxed(rate->m_val, PLL18_M_VAL);
	writel_relaxed(rate->n_val, PLL18_N_VAL);

	writel_relaxed(rate->i_bits, PLL18_CONFIG);
	writel_relaxed(0x00003080, PLL18_TEST_CTL);

	/*
	 * Enable PLL18 output
	 *	mb() added to ensure writes complete - Following /arch/arm/mach-msm without any other write functions
	 */
	writel_relaxed(0x2, PLL18_MODE);
	mb();
	writel_relaxed(0x6, PLL18_MODE);
	mb();
	writel_relaxed(0x7, PLL18_MODE);

	/*
	 * Enable NSS Vote for PLL18 and acquire LOCK
	 */
	writel_relaxed((readl_relaxed(PLL_ENA_NSS) | en_mask), PLL_ENA_NSS);
	do {
		value = readl_relaxed(PLL_LOCK_DET_STATUS);
		if (value & status_mask) {
			break;
		}
		udelay(1);
	} while (wait_cycles-- > 0);

	/*
	 * Program MND NS Registers
	 */
	writel_relaxed(nf->md_val, ncc->md0_reg);
	writel_relaxed(nf->md_val, ncc->md1_reg);

	writel_relaxed(nf->ns_val, ncc->ns0_reg);
	writel_relaxed(nf->ns_val, ncc->ns1_reg);

	/*
	 * Switch to New PLL
	 */
	ctl_reg0 = readl_relaxed(ncc->ctl0_src_reg);
	ctl_reg1 = readl_relaxed(ncc->ctl1_src_reg);
	ctl_reg0 &= ~pll_cl_mask;
	ctl_reg1 &= ~pll_cl_mask;
	ctl_reg0 |= pll18_mask;
	ctl_reg1 |= pll18_mask;
	writel_relaxed(ctl_reg0, ncc->ctl0_src_reg);
	writel_relaxed(ctl_reg1, ncc->ctl1_src_reg);
}

static struct clk_ops clk_ops_nss_core = {
	.prepare = nss_core_clk_prepare,
	.enable = nss_core_clk_enable,
	.disable = nss_core_clk_disable,
	.unprepare = nss_core_clk_unprepare,
	.handoff = nss_core_clk_handoff,
	.set_rate = nss_core_clk_set_rate,
	.get_rate = nss_core_clk_get_rate,
	.list_rate = nss_core_clk_list_rate,
	.reset = nss_core_clk_reset,
	.get_parent = nss_core_clk_get_parent,
};

static struct nss_core_clk_tbl nss_core_clk = {
	.b = {
		.ctl_reg = UBI32_COREn_CLK_CTL(0),
		.en_mask = BIT(1),
		.halt_check = NOCHECK,
	},
	.b1 = {
		.ctl_reg = UBI32_COREn_CLK_CTL(1),
		.en_mask = BIT(1),
		.halt_check = NOCHECK,
	},
	.ctl0_src_reg = UBI32_COREn_CLK_SRC_CTL(0),
	.ctl1_src_reg = UBI32_COREn_CLK_SRC_CTL(1),
	.ns0_reg = UBI32_COREn_CLK_SRC0_NS(0),
	.ns1_reg = UBI32_COREn_CLK_SRC0_NS(1),
	.md0_reg = UBI32_COREn_CLK_SRC0_MD(0),
	.md1_reg = UBI32_COREn_CLK_SRC0_MD(1),

	.ns_mask = 0xff01ff,
	.md_mask = 0xff00ff,

	.root_en_mask = BIT(11),
	.mnd_en_mask = BIT(8),
	.set_rate = set_rate_nss,
	.freq_tbl = clk_tbl_nss,
	.current_freq = &rcg_dummy_freq,
	.c = {
		.dbg_name = "nss_core_clk",
		.ops = &clk_ops_nss_core,
		CLK_INIT(nss_core_clk.c),
		VDD_NSS_DIG_FMAX_MAP2(NOMINAL, 550000000, HIGH, 733000000),
	},
};

static struct clk_lookup msm_clocks_gsbi4_uart[] = {
	CLK_LOOKUP("core_clk",	gsbi4_uart_clk.c,	"msm_serial_hsl.2"),
	CLK_LOOKUP("iface_clk",	gsbi4_p_clk.c,		"msm_serial_hsl.2"),
};

static struct clk_lookup msm_clocks_gsbi2_uart[] = {
	CLK_LOOKUP("core_clk",	gsbi2_uart_clk.c,	"msm_serial_hsl.2"),
	CLK_LOOKUP("iface_clk",	gsbi2_p_clk.c,		"msm_serial_hsl.2"),
};

struct clock_init_data ipq806x_gsbi4_uart_clks  __initdata = {
	.table = msm_clocks_gsbi4_uart,
	.size = ARRAY_SIZE(msm_clocks_gsbi4_uart),
};

struct clock_init_data ipq806x_gsbi2_uart_clks  __initdata = {
	.table = msm_clocks_gsbi2_uart,
	.size = ARRAY_SIZE(msm_clocks_gsbi2_uart),
};

static struct clk_lookup msm_clocks_ipq806x[] = {
	CLK_LOOKUP("xo",		cxo_a_clk.c,	""),
	CLK_LOOKUP("xo",		pxo_a_clk.c,	""),
	CLK_LOOKUP("pwm_clk",		cxo_clk.c,	"0-0048"),
	CLK_LOOKUP("xo",		cxo_clk.c,	"BAM_RMNT"),
	CLK_LOOKUP("xo",		cxo_clk.c,	"msm_xo"),
	CLK_LOOKUP("vref_buff",		cxo_clk.c,	"rpm-regulator"),
	CLK_LOOKUP("pll8",		pll8_clk.c,	NULL),
	CLK_LOOKUP("pll0",		pll0_clk.c,	NULL),
	CLK_LOOKUP("measure",		measure_clk.c,	"debug"),

	CLK_LOOKUP("bus_clk",		afab_clk.c,	""),
	CLK_LOOKUP("bus_clk",		afab_a_clk.c,	""),
	CLK_LOOKUP("bus_clk",		cfpb_clk.c,	""),
	CLK_LOOKUP("bus_clk",		cfpb_a_clk.c,	""),
	CLK_LOOKUP("bus_clk",		dfab_clk.c,	""),
	CLK_LOOKUP("bus_clk",		dfab_a_clk.c,	""),
	CLK_LOOKUP("mem_clk",		ebi1_clk.c,	""),
	CLK_LOOKUP("mem_clk",		ebi1_a_clk.c,	""),
	CLK_LOOKUP("bus_clk",		sfab_clk.c,	""),
	CLK_LOOKUP("bus_clk",		sfab_a_clk.c,	""),
	CLK_LOOKUP("bus_clk",		sfpb_clk.c,	""),
	CLK_LOOKUP("bus_clk",		sfpb_a_clk.c,	""),
	CLK_LOOKUP("bus_clk",		nssfab0_clk.c,	""),
	CLK_LOOKUP("bus_clk",		nssfab0_a_clk.c,	""),
	CLK_LOOKUP("bus_clk",		nssfab1_clk.c,	""),
	CLK_LOOKUP("bus_clk",		nssfab1_a_clk.c,	""),

	CLK_LOOKUP("bus_clk",		afab_clk.c,		"msm_apps_fab"),
	CLK_LOOKUP("bus_a_clk",		afab_msmbus_a_clk.c,	"msm_apps_fab"),
	CLK_LOOKUP("bus_clk",		cfpb_clk.c,		"msm_cpss_fpb"),
	CLK_LOOKUP("bus_a_clk",		cfpb_a_clk.c,		"msm_cpss_fpb"),
	CLK_LOOKUP("bus_clk",		sfab_clk.c,		"msm_sys_fab"),
	CLK_LOOKUP("bus_a_clk",		sfab_msmbus_a_clk.c,	"msm_sys_fab"),
	CLK_LOOKUP("bus_clk",		sfpb_clk.c,		"msm_sys_fpb"),
	CLK_LOOKUP("bus_a_clk",		sfpb_a_clk.c,		"msm_sys_fpb"),
	CLK_LOOKUP("bus_clk",		nssfab0_clk.c,		"msm_nss_fab_0"),
	CLK_LOOKUP("bus_a_clk",		nssfab0_a_clk.c,	"msm_nss_fab_0"),
	CLK_LOOKUP("bus_clk",		nssfab1_clk.c,		"msm_nss_fab_1"),
	CLK_LOOKUP("bus_a_clk",		nssfab1_a_clk.c,	"msm_nss_fab_1"),
	CLK_LOOKUP("nss_core_clk",	nss_core_clk.c,		"qca-nss.0"),
	CLK_LOOKUP("mem_clk",		ebi1_msmbus_clk.c,	"msm_bus"),
	CLK_LOOKUP("mem_a_clk",		ebi1_msmbus_a_clk.c,	"msm_bus"),
	CLK_LOOKUP("dfab_clk",		dfab_msmbus_clk.c,	"msm_bus"),
	CLK_LOOKUP("dfab_a_clk",	dfab_msmbus_a_clk.c,	"msm_bus"),
	CLK_LOOKUP("core_a_clk",	qdss_a_clk.c,		""),
	CLK_LOOKUP("core_clk",		qdss_clk.c,	"coresight-tpiu.0"),
	CLK_LOOKUP("core_clk",		qdss_clk.c,	"coresight-etb.0"),
	CLK_LOOKUP("core_clk",		qdss_clk.c,	"coresight-funnel.0"),
	CLK_LOOKUP("core_clk",		qdss_clk.c,	"coresight-etm.0"),
	CLK_LOOKUP("core_clk",		qdss_clk.c,	"coresight-etm.1"),

	CLK_LOOKUP("ebi1_clk",		ebi1_clk.c,		""),
	CLK_LOOKUP("cfpb_a_clk",	cfpb_a_clk.c,		"clock-ipq806x"),

	CLK_LOOKUP("core_clk",		gp0_clk.c,		""),
	CLK_LOOKUP("core_clk",		gp1_clk.c,		""),
	CLK_LOOKUP("core_clk",		gp2_clk.c,		""),
	CLK_LOOKUP("core_clk",		gsbi1_uart_clk.c,	""),
	CLK_LOOKUP("core_clk",		gsbi2_uart_clk.c,	"msm_serial_hsl.1"),
	//CLK_LOOKUP("core_clk",		gsbi3_uart_clk.c,	""),
	CLK_LOOKUP("core_clk",		gsbi4_uart_clk.c,	"msm_serial_hsl.1"),
	CLK_LOOKUP("core_clk",		gsbi5_uart_clk.c,	""),
	CLK_LOOKUP("core_clk",		gsbi6_uart_clk.c, "msm_serial_hs.0"),
	CLK_LOOKUP("core_clk",		gsbi7_uart_clk.c, "msm_serial_hsl.0"),
	CLK_LOOKUP("core_clk",		gsbi1_qup_clk.c,	"qup_i2c.0"),
	CLK_LOOKUP("core_clk",		gsbi2_qup_clk.c,	"qup_i2c.1"),
	CLK_LOOKUP("core_clk",		gsbi2_qup_clk.c,	"spi_qsd.2"),
	/*CLK_LOOKUP("core_clk",	gsbi3_qup_clk.c,	"qup_i2c.2"),*/
	CLK_LOOKUP("core_clk",		gsbi5_qup_clk.c,	"spi_qsd.5"),
	CLK_LOOKUP("core_clk",		gsbi5_qup_clk.c,	"qup_i2c.4"),
	CLK_LOOKUP("core_clk",		gsbi6_qup_clk.c,	"spi_qsd.6"),
	CLK_LOOKUP("core_clk",		gsbi7_qup_clk.c,	""),
	CLK_LOOKUP("core_clk",		pdm_clk.c,		""),
	CLK_LOOKUP("mem_clk",		pmem_clk.c,		"msm_sps"),
	CLK_LOOKUP("core_clk",          prng_clk.c,		"msm_rng.0"),
	CLK_LOOKUP("core_clk",		sdc1_clk.c,		"msm_sdcc.1"),
	CLK_LOOKUP("core_clk",		sdc3_clk.c,		"msm_sdcc.3"),
	CLK_LOOKUP("core_clk",		tssc_clk.c,		""),
	CLK_LOOKUP("alt_core_clk",	usb_hs1_xcvr_clk.c,	"msm_otg"),
	CLK_LOOKUP("src_clk",		usb_fs1_src_clk.c,	""),
	CLK_LOOKUP("alt_core_clk",	usb_fs1_xcvr_clk.c,	""),
	CLK_LOOKUP("sys_clk",		usb_fs1_sys_clk.c,	""),
	CLK_LOOKUP("ref_clk",		sata_phy_ref_clk.c,	"msm_sata.0"),
	CLK_LOOKUP("cfg_clk",		sata_phy_cfg_clk.c,	"msm_sata.0"),
	CLK_LOOKUP("src_clk",		sata_src_clk.c,		"msm_sata.0"),
	CLK_LOOKUP("core_rxoob_clk",	sata_rxoob_clk.c,	"msm_sata.0"),
	CLK_LOOKUP("core_pmalive_clk",	sata_pmalive_clk.c,	"msm_sata.0"),
	CLK_LOOKUP("bus_clk",		sata_a_clk.c,		"msm_sata.0"),
	CLK_LOOKUP("iface_clk",		sata_p_clk.c,		"msm_sata.0"),
	CLK_LOOKUP("slave_iface_clk",	sfab_sata_s_p_clk.c,	"msm_sata.0"),
	CLK_LOOKUP("iface_clk",		ce5_p_clk.c,		"qce.0"),
	CLK_LOOKUP("iface_clk",		ce5_p_clk.c,		"qcrypto.0"),
	CLK_LOOKUP("core_clk",		ce5_core_clk.c,		"qce.0"),
	CLK_LOOKUP("core_clk",		ce5_core_clk.c,		"qcrypto.0"),
	CLK_LOOKUP("ce5_core_src_clk",	ce5_src_clk.c,		"qce.0"),
	CLK_LOOKUP("ce5_core_src_clk",	ce5_src_clk.c,		"qcrypto.0"),
	CLK_LOOKUP("dma_bam_pclk",	dma_bam_p_clk.c,	NULL),
	CLK_LOOKUP("iface_clk",		gsbi1_p_clk.c,		"qup_i2c.0"),
	CLK_LOOKUP("iface_clk",		gsbi2_p_clk.c,		"msm_serial_hsl.1"),
	CLK_LOOKUP("iface_clk",		gsbi2_p_clk.c,		"qup_i2c.1"),
	CLK_LOOKUP("iface_clk",		gsbi2_p_clk.c,		"spi_qsd.2"),
	/*CLK_LOOKUP("iface_clk",	gsbi3_p_clk.c,		"qup_i2c.2"),*/
	CLK_LOOKUP("iface_clk",		gsbi4_p_clk.c,		"msm_serial_hsl.1"),
	CLK_LOOKUP("iface_clk",		gsbi4_p_clk.c,		"qup_i2c.3"),
	CLK_LOOKUP("iface_clk",		gsbi5_p_clk.c,		"spi_qsd.5"),
	CLK_LOOKUP("iface_clk",		gsbi5_p_clk.c,		"qup_i2c.4"),
	CLK_LOOKUP("iface_clk",		gsbi6_p_clk.c,		"spi_qsd.6"),
	CLK_LOOKUP("iface_clk",		gsbi7_p_clk.c,		"msm_serial_hsl.0"),
	CLK_LOOKUP("ref_clk",		tsif_ref_clk.c,		"msm_tspp.0"),
	CLK_LOOKUP("iface_clk",		tsif_p_clk.c,		"msm_tspp.0"),
	CLK_LOOKUP("iface_clk",		usb_fs1_p_clk.c,	""),
	CLK_LOOKUP("iface_clk",		usb_hs1_p_clk.c,	"msm_otg"),
	CLK_LOOKUP("iface_clk",		sdc1_p_clk.c,		"msm_sdcc.1"),
	CLK_LOOKUP("iface_clk",		sdc3_p_clk.c,		"msm_sdcc.3"),
	CLK_LOOKUP("iface_clk",		pcie_p_clk.c,		"msm_pcie.0"),
	CLK_LOOKUP("aux_clk",		pcie_aux_clk.c,	"msm_pcie.0"),
	CLK_LOOKUP("ref_clk",		pcie_phy_ref_clk.c,	"msm_pcie.0"),
	CLK_LOOKUP("bus_clk",		pcie_a_clk.c,		"msm_pcie.0"),
	CLK_LOOKUP("alt_ref_clk",	pcie_src_clk.c,		"msm_pcie.0"),
	CLK_LOOKUP("iface_clk",		pcie1_p_clk.c,		"msm_pcie.1"),
	CLK_LOOKUP("aux_clk",		pcie1_aux_clk.c,	"msm_pcie.1"),
	CLK_LOOKUP("ref_clk",		pcie1_phy_ref_clk.c,	"msm_pcie.1"),
	CLK_LOOKUP("bus_clk",		pcie1_a_clk.c,		"msm_pcie.1"),
	CLK_LOOKUP("alt_ref_clk",	pcie1_src_clk.c,	"msm_pcie.1"),
	CLK_LOOKUP("iface_clk",		pcie2_p_clk.c,		"msm_pcie.2"),
	CLK_LOOKUP("aux_clk",		pcie2_aux_clk.c,	"msm_pcie.2"),
	CLK_LOOKUP("ref_clk",		pcie2_phy_ref_clk.c,	"msm_pcie.2"),
	CLK_LOOKUP("bus_clk",		pcie2_a_clk.c,		"msm_pcie.2"),
	CLK_LOOKUP("alt_ref_clk",	pcie2_src_clk.c,	"msm_pcie.2"),
	CLK_LOOKUP("core_clk",		adm0_clk.c,		"msm_dmov"),
	CLK_LOOKUP("iface_clk",		adm0_p_clk.c,		"msm_dmov"),
	CLK_LOOKUP("iface_clk",		pmic_arb0_p_clk.c,	""),
	CLK_LOOKUP("iface_clk",		pmic_arb1_p_clk.c,	""),
	CLK_LOOKUP("core_clk",		pmic_ssbi2_clk.c,	""),
	CLK_LOOKUP("mem_clk",		rpm_msg_ram_p_clk.c,	""),

	CLK_LOOKUP("core_clk",		dfab_usb_hs_clk.c,	"msm_otg"),
	CLK_LOOKUP("bus_clk",		dfab_sdc1_clk.c, "msm_sdcc.1"),
	CLK_LOOKUP("bus_clk",		dfab_sdc3_clk.c, "msm_sdcc.3"),
	CLK_LOOKUP("dfab_clk",		dfab_sps_clk.c,	"msm_sps"),
	CLK_LOOKUP("bus_clk",		dfab_bam_dmux_clk.c,	"BAM_RMNT"),
	CLK_LOOKUP("bus_clk",		dfab_scm_clk.c,	"scm"),

	CLK_LOOKUP("alt_core_clk",    usb_hsic_xcvr_fs_clk.c,  "msm_hsic_host"),
	CLK_LOOKUP("phy_clk",	      usb_hsic_hsic_clk.c,     "msm_hsic_host"),
	CLK_LOOKUP("cal_clk",	      usb_hsic_hsio_cal_clk.c, "msm_hsic_host"),
	CLK_LOOKUP("core_clk",	      usb_hsic_system_clk.c,   "msm_hsic_host"),
	CLK_LOOKUP("iface_clk",	      usb_hsic_p_clk.c,        "msm_hsic_host"),

	CLK_LOOKUP("mem_clk",		ebi1_adm_clk.c, "msm_dmov"),
	CLK_LOOKUP("mem_clk",		ebi1_acpu_a_clk.c, ""),
	CLK_LOOKUP("bus_clk",		afab_acpu_a_clk.c, ""),

	CLK_LOOKUP("l2_mclk",		l2_m_clk,     ""),
	CLK_LOOKUP("krait0_mclk",	krait0_m_clk, ""),
	CLK_LOOKUP("krait1_mclk",	krait1_m_clk, ""),

	CLK_LOOKUP("core_clk",		usb30_0_master_clk.c,	"ipq-dwc3.0"),
	CLK_LOOKUP("iface0_clk",	usb30_0_branch_clk.c,	"ipq-dwc3.0"),
	CLK_LOOKUP("iface1_clk",	usb30_1_branch_clk.c,	"ipq-dwc3.0"),
	CLK_LOOKUP("utmi_clk",		usb30_utmi_clk.c,	"ipq-dwc3.0"),
	CLK_LOOKUP("utmi_b0_clk",	usb30_0_utmi_clk_ctl.c,	"ipq-dwc3.0"),
	CLK_LOOKUP("utmi_b1_clk",	usb30_1_utmi_clk_ctl.c,	"ipq-dwc3.0"),

	CLK_LOOKUP("PLL4",		pll4_clk.c,		NULL),
	CLK_LOOKUP("ahbex_clk",		ahbex_clk.c,	"ipq-lpass-clk"),
	CLK_LOOKUP("ahbix_clk",		ahbix_clk.c,	"ipq-lpass-clk"),
	CLK_LOOKUP("mi2s_osr_clk",	mi2s_osr_clk.c,	"ipq-cpu-dai"),
	CLK_LOOKUP("mi2s_bit_clk",	mi2s_bit_clk.c,	"ipq-cpu-dai"),
	CLK_LOOKUP("spdif_bit_clk",	spdif_clk.c,	"ipq-cpu-dai"),
	CLK_LOOKUP("pcm_bit_clk",	pcm_clk.c,	"ipq-lpaif"),
};


static struct clk_lookup msm_clocks_ipq806x_dummy[] = {
        CLK_DUMMY("pll2",               PLL2,           NULL, 0),
        CLK_DUMMY("pll8",               PLL8,           NULL, 0),
        CLK_DUMMY("pll4",               PLL4,           NULL, 0),

	CLK_DUMMY("bus_clk",		afab_clk.c,		"msm_apps_fab", 0),
	CLK_DUMMY("bus_a_clk",		afab_msmbus_a_clk.c,	"msm_apps_fab", 0),
	CLK_DUMMY("bus_clk",		cfpb_clk.c,		"msm_cpss_fpb", 0),
	CLK_DUMMY("bus_a_clk",		cfpb_a_clk.c,		"msm_cpss_fpb", 0),
	CLK_DUMMY("bus_clk",		sfab_clk.c,		"msm_sys_fab", 0),
	CLK_DUMMY("bus_a_clk",		sfab_msmbus_a_clk.c,	"msm_sys_fab", 0),
	CLK_DUMMY("bus_clk",		sfpb_clk.c,		"msm_sys_fpb", 0),
	CLK_DUMMY("bus_a_clk",		sfpb_a_clk.c,		"msm_sys_fpb", 0),
	CLK_DUMMY("mem_clk",		ebi1_msmbus_clk.c,	"msm_bus", 0),
	CLK_DUMMY("mem_a_clk",		ebi1_msmbus_a_clk.c,	"msm_bus", 0),
	CLK_DUMMY("dfab_clk",		dfab_msmbus_clk.c,	"msm_bus", 0),
	CLK_DUMMY("dfab_a_clk",		dfab_msmbus_a_clk.c,	"msm_bus", 0),

	/*
	CLK_LOOKUP("core_clk",          gp0_clk.c,              NULL),
        CLK_LOOKUP("core_clk",          gp1_clk.c,              NULL),
	CLK_LOOKUP("core_clk",          gp2_clk.c,              NULL),
        CLK_LOOKUP("core_clk",          gsbi1_uart_clk.c,       "msm_serial_hsl.1"),
	CLK_LOOKUP("core_clk",          gsbi2_uart_clk.c,       NULL),
	CLK_LOOKUP("core_clk",          gsbi3_uart_clk.c,       NULL),
        CLK_LOOKUP("core_clk",          gsbi4_uart_clk.c,       NULL),
	CLK_LOOKUP("core_clk",          gsbi5_uart_clk.c,       NULL),
	CLK_LOOKUP("core_clk",          gsbi6_uart_clk.c,       NULL),
	CLK_LOOKUP("core_clk",          gsbi7_uart_clk.c,       NULL),
	*/

	CLK_DUMMY("xo",	cxo_clk.c,	"BAM_RMNT", 0),
	CLK_DUMMY("core_clk",	qdss_clk.c,	"coresight-tpiu.0", 0),
	CLK_DUMMY("core_clk",	qdss_clk.c,	"coresight-etb.0", 0),
	CLK_DUMMY("core_clk",	qdss_clk.c,	"coresight-funnel.0", 0),
	CLK_DUMMY("core_clk",	qdss_clk.c,	"coresight-etm.0", 0),
	CLK_DUMMY("core_clk",	qdss_clk.c,	"coresight-etm.1", 0),

	CLK_DUMMY("mem_clk",	pmem_clk.c,	"msm_sps", 0),
	CLK_DUMMY("core_clk",   prng_clk.c,	"msm_rng.0", 0),

	CLK_DUMMY("dfab_clk",	dfab_sps_clk.c,	"msm_sps", 0),
	CLK_DUMMY("bus_clk",	dfab_bam_dmux_clk.c,	"BAM_RMNT", 0),
	CLK_DUMMY("bus_clk",	dfab_scm_clk.c,	"scm", 0),

	CLK_DUMMY("mem_clk",	ebi1_adm_clk.c, "msm_dmov", 0),

	CLK_DUMMY("core_clk",		gp0_clk.c,		"", 0),
	CLK_DUMMY("core_clk",		gp1_clk.c,		"", 0),
	CLK_DUMMY("core_clk",		gp2_clk.c,		"", 0),
	CLK_DUMMY("core_clk",		gsbi1_uart_clk.c, "msm_serial_hsl.1", 0),
	CLK_DUMMY("core_clk",		gsbi2_uart_clk.c,	"", 0),
	//CLK_DUMMY("core_clk",		gsbi3_uart_clk.c,	"", 0),
	CLK_DUMMY("core_clk",		gsbi4_uart_clk.c,	"", 0),
	CLK_DUMMY("core_clk",		gsbi5_uart_clk.c,	"", 0),
	CLK_DUMMY("core_clk",		gsbi6_uart_clk.c, "msm_serial_hs.0", 0),
	CLK_DUMMY("core_clk",		gsbi7_uart_clk.c, "msm_serial_hsl.0", 0),

	CLK_DUMMY("core_clk",		gsbi1_qup_clk.c,	"qup_i2c.0", 0),
	CLK_DUMMY("core_clk",		gsbi2_qup_clk.c,	"qup_i2c.1", 0),
	//CLK_DUMMY("core_clk",		gsbi3_qup_clk.c,	"qup_i2c.3", 0),
	CLK_DUMMY("core_clk",		gsbi5_qup_clk.c,	"spi_qsd.5", 0),
	CLK_DUMMY("core_clk",		gsbi5_qup_clk.c,	"qup_i2c.4", 0),
	CLK_DUMMY("core_clk",		gsbi6_qup_clk.c,	"", 0),
	CLK_DUMMY("core_clk",		gsbi7_qup_clk.c,	"", 0),
	CLK_DUMMY("core_clk",		pdm_clk.c,		"", 0),

	//CLK_LOOKUP("iface_clk",		gsbi1_p_clk.c,	"msm_serial_hsl.1"),
	CLK_DUMMY("iface_clk",		gsbi1_p_clk.c,	"msm_serial_hsl.1", 0),
	CLK_DUMMY("iface_clk",		gsbi1_p_clk.c,	"qup_i2c.0", 0),
	CLK_DUMMY("iface_clk",		gsbi2_p_clk.c,	"qup_i2c.1", 0),
	/*CLK_DUMMY("iface_clk",	gsbi3_p_clk.c,	"qup_i2c.2", 0),*/
	CLK_DUMMY("iface_clk",		gsbi4_p_clk.c,	"qup_i2c.3", 0),
	CLK_DUMMY("iface_clk",		gsbi5_p_clk.c,		"spi_qsd.5", 0),
	CLK_DUMMY("iface_clk",		gsbi5_p_clk.c,	"qup_i2c.4", 0),
	CLK_DUMMY("iface_clk",		gsbi6_p_clk.c,	"msm_serial_hs.0", 0),
	CLK_DUMMY("iface_clk",		gsbi7_p_clk.c,	"msm_serial_hsl.0", 0),
	CLK_DUMMY("iface_clk",      sdc1_p_clk.c,       "msm_sdcc.1", 0),
	CLK_DUMMY("bus_clk",        dfab_sdc1_clk.c,    "msm_sdcc.1", 0),
	CLK_DUMMY("core_clk",       sdc1_clk.c,         "msm_sdcc.1", 0),
	CLK_DUMMY("iface_clk",      sdc3_p_clk.c,       "msm_sdcc.3", 0),
	CLK_DUMMY("bus_clk",        dfab_sdc3_clk.c,    "msm_sdcc.3", 0),
	CLK_DUMMY("core_clk",       sdc3_clk.c,         "msm_sdcc.3", 0),
        CLK_DUMMY("gsbi_uart_clk",      GSBI1_UART_CLK,         NULL, OFF),
        CLK_DUMMY("gsbi_uart_clk",      GSBI2_UART_CLK,         NULL, OFF),
        CLK_DUMMY("gsbi_uart_clk",      GSBI3_UART_CLK,
                                                  "msm_serial_hsl.0", OFF),
        CLK_DUMMY("gsbi_uart_clk",      GSBI4_UART_CLK,         NULL, OFF),
        CLK_DUMMY("gsbi_uart_clk",      GSBI5_UART_CLK,         NULL, OFF),
	CLK_DUMMY("uartdm_clk",         GSBI6_UART_CLK,         NULL, OFF),
        CLK_DUMMY("gsbi_uart_clk",      GSBI7_UART_CLK,         NULL, OFF),
        CLK_DUMMY("gsbi_qup_clk",       GSBI2_QUP_CLK,          NULL, OFF),
        CLK_DUMMY("gsbi_qup_clk",       GSBI3_QUP_CLK,          NULL, OFF),
        CLK_DUMMY("gsbi_qup_clk",       GSBI6_QUP_CLK,          NULL, OFF),
        CLK_DUMMY("gsbi_qup_clk",       GSBI7_QUP_CLK,          NULL, OFF),
        CLK_DUMMY("pdm_clk",            PDM_CLK,                NULL, OFF),
        CLK_DUMMY("pmem_clk",           PMEM_CLK,               NULL, OFF),
        CLK_DUMMY("prng_clk",           PRNG_CLK,               NULL, OFF),
        CLK_DUMMY("tsif_ref_clk",       TSIF_REF_CLK,           NULL, OFF),
        CLK_DUMMY("tssc_clk",           TSSC_CLK,               NULL, OFF),
	CLK_DUMMY("sleep_clk",		USB30_SLEEP_CLK,
							"msm-dwc3.0", OFF),
	CLK_DUMMY("sleep_a_clk",	USB2a_SLEEP_CLK,
							"msm-dwc3.0", OFF),
	CLK_DUMMY("sleep_b_clk",	USB2b_SLEEP_CLK,
							"msm-dwc3.0", OFF),
	CLK_DUMMY("ref_clk",		USB30_DIFF_CLK,
							"msm-dwc3.0", OFF),
	CLK_DUMMY("sleep_clk",		USB30_SLEEP_CLK,
							"msm-dwc3.1", OFF),
	CLK_DUMMY("sleep_a_clk",	USB2a_SLEEP_CLK,
							"msm-dwc3.1", OFF),
	CLK_DUMMY("sleep_b_clk",	USB2b_SLEEP_CLK,
							"msm-dwc3.1", OFF),
	CLK_DUMMY("ref_clk",		USB30_DIFF_CLK,
							"msm-dwc3.1", OFF),
        CLK_DUMMY("usb_hs_clk",         USB_HS1_XCVR_CLK,       NULL, OFF),
        CLK_DUMMY("usb_hs_clk",         USB_HS3_XCVR_CLK,       NULL, OFF),
        CLK_DUMMY("usb_hs_clk",         USB_HS4_XCVR_CLK,       NULL, OFF),
        CLK_DUMMY("usb_phy_clk",        USB_PHY0_CLK,           NULL, OFF),
        CLK_DUMMY("usb_fs_src_clk",     USB_FS1_SRC_CLK,        NULL, OFF),
	CLK_DUMMY("usb_fs_clk",         USB_FS1_XCVR_CLK,       NULL, OFF),
        CLK_DUMMY("usb_fs_sys_clk",     USB_FS1_SYS_CLK,        NULL, OFF),
	CLK_DUMMY("ce_pclk",            CE2_CLK,                NULL, OFF),
        CLK_DUMMY("ce_clk",             CE1_CORE_CLK,           NULL, OFF),
	CLK_DUMMY("ce_clk",             CE5_CORE_CLK,           NULL, OFF),
	CLK_DUMMY("ce_pclk",            CE5_P_CLK,              NULL, OFF),
	CLK_DUMMY("iface_clk",		pcie_p_clk.c,		"msm_pcie.0", OFF),
	CLK_DUMMY("ref_clk",		pcie_phy_ref_clk.c,	"msm_pcie.0", OFF),
	CLK_DUMMY("aux_clk",		pcie_aux_clk.c,	    "msm_pcie.0", OFF),
	CLK_DUMMY("bus_clk",		pcie_a_clk.c,		"msm_pcie.0", OFF),
	CLK_DUMMY("alt_ref_clk",	pcie_src_clk.c,		"msm_pcie.0", OFF),
	CLK_DUMMY("iface_clk",		pcie_p_clk.c,		"msm_pcie.1", OFF),
	CLK_DUMMY("aux_clk",		pcie_aux_clk.c,	    "msm_pcie.1", OFF),
	CLK_DUMMY("ref_clk",		pcie_phy_ref_clk.c,	"msm_pcie.1", OFF),
	CLK_DUMMY("bus_clk",		pcie_a_clk.c,		"msm_pcie.1", OFF),
	CLK_DUMMY("alt_ref_clk",	pcie1_src_clk.c,	"msm_pcie.1", OFF),
	CLK_DUMMY("iface_clk",		pcie_p_clk.c,		"msm_pcie.2", OFF),
	CLK_DUMMY("aux_clk",		pcie_aux_clk.c,	    "msm_pcie.2", OFF),
	CLK_DUMMY("ref_clk",		pcie_phy_ref_clk.c,	"msm_pcie.2", OFF),
	CLK_DUMMY("bus_clk",		pcie_a_clk.c,		"msm_pcie.2", OFF),
	CLK_DUMMY("alt_ref_clk",	pcie2_src_clk.c,	"msm_pcie.2", OFF),
        CLK_DUMMY("sata_rxoob_clk",     SATA_RXOOB_CLK,         NULL, OFF),
        CLK_DUMMY("sata_pmalive_clk",   SATA_PMALIVE_CLK,       NULL, OFF),
        CLK_DUMMY("sata_phy_ref_clk",   SATA_PHY_REF_CLK,       NULL, OFF),
        CLK_DUMMY("spi_pclk",           GSBI1_P_CLK,            NULL, OFF),
        CLK_DUMMY("gsbi_pclk",          GSBI2_P_CLK,            NULL, OFF),
        CLK_DUMMY("gsbi_pclk",          GSBI3_P_CLK,
                                                  "msm_serial_hsl.0", OFF),
        CLK_DUMMY("gsbi_pclk",          GSBI4_P_CLK,            NULL, OFF),
        CLK_DUMMY("spi_pclk",           GSBI5_P_CLK,     "spi_qsd.0", OFF),
        CLK_DUMMY("uartdm_pclk",        GSBI6_P_CLK,            NULL, OFF),
        CLK_DUMMY("gsbi_pclk",          GSBI7_P_CLK,            NULL, OFF),
        CLK_DUMMY("tsif_pclk",          TSIF_P_CLK,             NULL, OFF),
        CLK_DUMMY("usb_fs_pclk",        USB_FS1_P_CLK,          NULL, OFF),
        CLK_DUMMY("usb_hs_pclk",        USB_HS1_P_CLK,          NULL, OFF),
        CLK_DUMMY("usb_hs_pclk",        USB_HS3_P_CLK,          NULL, OFF),
        CLK_DUMMY("usb_hs_pclk",        USB_HS4_P_CLK,          NULL, OFF),
        CLK_DUMMY("adm_clk",            ADM0_CLK,               NULL, OFF),
        CLK_DUMMY("adm_pclk",           ADM0_P_CLK,             NULL, OFF),
        CLK_DUMMY("pmic_arb_pclk",      PMIC_ARB0_P_CLK,        NULL, OFF),
        CLK_DUMMY("pmic_arb_pclk",      PMIC_ARB1_P_CLK,        NULL, OFF),
        CLK_DUMMY("pmic_ssbi2",         PMIC_SSBI2_CLK,         NULL, OFF),
        CLK_DUMMY("rpm_msg_ram_pclk",   RPM_MSG_RAM_P_CLK,      NULL, OFF),
        CLK_DUMMY("amp_clk",            AMP_CLK,                NULL, OFF),
        CLK_DUMMY("cam_clk",            CAM0_CLK,               NULL, OFF),
        CLK_DUMMY("cam_clk",            CAM1_CLK,               NULL, OFF),
        CLK_DUMMY("csi_src_clk",        CSI0_SRC_CLK,           NULL, OFF),
        CLK_DUMMY("csi_src_clk",        CSI1_SRC_CLK,           NULL, OFF),
        CLK_DUMMY("csi_clk",            CSI0_CLK,               NULL, OFF),
        CLK_DUMMY("csi_clk",            CSI1_CLK,               NULL, OFF),
        CLK_DUMMY("csi_pix_clk",        CSI_PIX_CLK,            NULL, OFF),
        CLK_DUMMY("csi_rdi_clk",        CSI_RDI_CLK,            NULL, OFF),
        CLK_DUMMY("csiphy_timer_src_clk", CSIPHY_TIMER_SRC_CLK, NULL, OFF),
        CLK_DUMMY("csi0phy_timer_clk",  CSIPHY0_TIMER_CLK,      NULL, OFF),
        CLK_DUMMY("csi1phy_timer_clk",  CSIPHY1_TIMER_CLK,      NULL, OFF),
        CLK_DUMMY("dsi_byte_div_clk",   DSI1_BYTE_CLK,          NULL, OFF),
        CLK_DUMMY("dsi_byte_div_clk",   DSI2_BYTE_CLK,          NULL, OFF),
        CLK_DUMMY("dsi_esc_clk",        DSI1_ESC_CLK,           NULL, OFF),
        CLK_DUMMY("dsi_esc_clk",        DSI2_ESC_CLK,           NULL, OFF),
        CLK_DUMMY("vcap_clk",           VCAP_CLK,               NULL, OFF),
        CLK_DUMMY("vcap_npl_clk",       VCAP_NPL_CLK,           NULL, OFF),
        CLK_DUMMY("gfx3d_clk",          GFX3D_CLK,              NULL, OFF),
        CLK_DUMMY("ijpeg_clk",          IJPEG_CLK,              NULL, OFF),
        CLK_DUMMY("imem_clk",           IMEM_CLK,               NULL, OFF),
        CLK_DUMMY("jpegd_clk",          JPEGD_CLK,              NULL, OFF),
        CLK_DUMMY("mdp_clk",            MDP_CLK,                NULL, OFF),
        CLK_DUMMY("mdp_vsync_clk",      MDP_VSYNC_CLK,          NULL, OFF),
        CLK_DUMMY("lut_mdp",            LUT_MDP_CLK,            NULL, OFF),
        CLK_DUMMY("tv_src_clk",         TV_SRC_CLK,             NULL, OFF),
        CLK_DUMMY("vcodec_clk",         VCODEC_CLK,             NULL, OFF),
        CLK_DUMMY("mdp_tv_clk",         MDP_TV_CLK,             NULL, OFF),
        CLK_DUMMY("rgb_tv_clk",         RGB_TV_CLK,             NULL, OFF),
        CLK_DUMMY("npl_tv_clk",         NPL_TV_CLK,             NULL, OFF),
        CLK_DUMMY("hdmi_clk",           HDMI_TV_CLK,            NULL, OFF),
        CLK_DUMMY("hdmi_app_clk",       HDMI_APP_CLK,           NULL, OFF),
        CLK_DUMMY("vpe_clk",            VPE_CLK,                NULL, OFF),
        CLK_DUMMY("vfe_clk",            VFE_CLK,                NULL, OFF),
        CLK_DUMMY("csi_vfe_clk",        CSI0_VFE_CLK,           NULL, OFF),
        CLK_DUMMY("vfe_axi_clk",        VFE_AXI_CLK,            NULL, OFF),
        CLK_DUMMY("ijpeg_axi_clk",      IJPEG_AXI_CLK,          NULL, OFF),
        CLK_DUMMY("mdp_axi_clk",        MDP_AXI_CLK,            NULL, OFF),
        CLK_DUMMY("rot_axi_clk",        ROT_AXI_CLK,            NULL, OFF),
        CLK_DUMMY("vcodec_axi_clk",     VCODEC_AXI_CLK,         NULL, OFF),
        CLK_DUMMY("vcodec_axi_a_clk",   VCODEC_AXI_A_CLK,       NULL, OFF),
        CLK_DUMMY("vcodec_axi_b_clk",   VCODEC_AXI_B_CLK,       NULL, OFF),
        CLK_DUMMY("vpe_axi_clk",        VPE_AXI_CLK,            NULL, OFF),
        CLK_DUMMY("gfx3d_axi_clk",      GFX3D_AXI_CLK,          NULL, OFF),
        CLK_DUMMY("vcap_axi_clk",       VCAP_AXI_CLK,           NULL, OFF),
        CLK_DUMMY("vcap_ahb_clk",       VCAP_AHB_CLK,           NULL, OFF),
        CLK_DUMMY("amp_pclk",           AMP_P_CLK,              NULL, OFF),
        CLK_DUMMY("csi_pclk",           CSI0_P_CLK,             NULL, OFF),
        CLK_DUMMY("dsi_m_pclk",         DSI1_M_P_CLK,           NULL, OFF),
        CLK_DUMMY("dsi_s_pclk",         DSI1_S_P_CLK,           NULL, OFF),
        CLK_DUMMY("dsi_m_pclk",         DSI2_M_P_CLK,           NULL, OFF),
        CLK_DUMMY("dsi_s_pclk",         DSI2_S_P_CLK,           NULL, OFF),
        CLK_DUMMY("lvds_clk",           LVDS_CLK,               NULL, OFF),
        CLK_DUMMY("mdp_p2clk",          MDP_P2CLK,              NULL, OFF),
        CLK_DUMMY("dsi2_pixel_clk",     DSI2_PIXEL_CLK,         NULL, OFF),
        CLK_DUMMY("lvds_ref_clk",       LVDS_REF_CLK,           NULL, OFF),
        CLK_DUMMY("gfx3d_pclk",         GFX3D_P_CLK,            NULL, OFF),
        CLK_DUMMY("hdmi_m_pclk",        HDMI_M_P_CLK,           NULL, OFF),
        CLK_DUMMY("hdmi_s_pclk",        HDMI_S_P_CLK,           NULL, OFF),
        CLK_DUMMY("ijpeg_pclk",         IJPEG_P_CLK,            NULL, OFF),
        CLK_DUMMY("jpegd_pclk",         JPEGD_P_CLK,            NULL, OFF),
        CLK_DUMMY("imem_pclk",          IMEM_P_CLK,             NULL, OFF),
        CLK_DUMMY("mdp_pclk",           MDP_P_CLK,              NULL, OFF),
        CLK_DUMMY("smmu_pclk",          SMMU_P_CLK,             NULL, OFF),
        CLK_DUMMY("rotator_pclk",       ROT_P_CLK,              NULL, OFF),
        CLK_DUMMY("vcodec_pclk",        VCODEC_P_CLK,           NULL, OFF),
        CLK_DUMMY("vfe_pclk",           VFE_P_CLK,              NULL, OFF),
        CLK_DUMMY("vpe_pclk",           VPE_P_CLK,              NULL, OFF),
        CLK_DUMMY("mi2s_osr_clk",       MI2S_OSR_CLK,           NULL, OFF),
        CLK_DUMMY("mi2s_bit_clk",       MI2S_BIT_CLK,           NULL, OFF),
        CLK_DUMMY("i2s_mic_osr_clk",    CODEC_I2S_MIC_OSR_CLK,  NULL, OFF),
        CLK_DUMMY("i2s_mic_bit_clk",    CODEC_I2S_MIC_BIT_CLK,  NULL, OFF),
        CLK_DUMMY("i2s_mic_osr_clk",    SPARE_I2S_MIC_OSR_CLK,  NULL, OFF),
        CLK_DUMMY("i2s_mic_bit_clk",    SPARE_I2S_MIC_BIT_CLK,  NULL, OFF),
        CLK_DUMMY("i2s_spkr_osr_clk",   CODEC_I2S_SPKR_OSR_CLK, NULL, OFF),
        CLK_DUMMY("i2s_spkr_bit_clk",   CODEC_I2S_SPKR_BIT_CLK, NULL, OFF),
        CLK_DUMMY("i2s_spkr_osr_clk",   SPARE_I2S_SPKR_OSR_CLK, NULL, OFF),
        CLK_DUMMY("i2s_spkr_bit_clk",   SPARE_I2S_SPKR_BIT_CLK, NULL, OFF),
        CLK_DUMMY("pcm_clk",            PCM_CLK,                NULL, OFF),
        CLK_DUMMY("iommu_clk",          JPEGD_AXI_CLK,          NULL, 0),
        CLK_DUMMY("iommu_clk",          VFE_AXI_CLK,            NULL, 0),
        CLK_DUMMY("iommu_clk",          VCODEC_AXI_CLK,         NULL, 0),
        CLK_DUMMY("iommu_clk",          GFX3D_CLK,              NULL, 0),
        CLK_DUMMY("iommu_clk",          GFX2D0_CLK,             NULL, 0),
        CLK_DUMMY("iommu_clk",          GFX2D1_CLK,             NULL, 0),

        CLK_DUMMY("dfab_dsps_clk",      DFAB_DSPS_CLK,          NULL, 0),
        CLK_DUMMY("dfab_usb_hs_clk",    DFAB_USB_HS_CLK,        NULL, 0),
        CLK_DUMMY("dfab_clk",           DFAB_CLK,               NULL, 0),
        CLK_DUMMY("dma_bam_pclk",       DMA_BAM_P_CLK,          NULL, 0),
	CLK_DUMMY("core_clk",		adm0_clk.c,	"msm_dmov", 0),
	CLK_DUMMY("iface_clk",		adm0_p_clk.c,	"msm_dmov", 0),
	CLK_DUMMY("alt_core_clk",    usb_hsic_xcvr_fs_clk.c,  "msm_hsic_host",0),
	CLK_DUMMY("phy_clk",         usb_hsic_hsic_clk.c,     "msm_hsic_host",0),
	CLK_DUMMY("cal_clk",         usb_hsic_hsio_cal_clk.c, "msm_hsic_host",0),
	CLK_DUMMY("core_clk",        usb_hsic_system_clk.c,   "msm_hsic_host",0),
	CLK_DUMMY("iface_clk",       usb_hsic_p_clk.c,        "msm_hsic_host",0),

	CLK_DUMMY("ref_clk",		REF_CLK,		NULL, "msm_sata.0"),
	CLK_DUMMY("cfg_clk",		CFG_CLK,		NULL, "msm_sata.0"),
	CLK_DUMMY("src_clk",		SRC_CLK,		NULL, "msm_sata.0"),
	CLK_DUMMY("core_rxoob_clk",	CORE_RXOOB_CLK,		NULL, "msm_sata.0"),
	CLK_DUMMY("core_pmalive_clk",	CORE_PMALIVE_CLK,	NULL, "msm_sata.0"),
	CLK_DUMMY("bus_clk",		BUS_CLK,		NULL, "msm_sata.0"),
	CLK_DUMMY("iface_clk",		IFACE_CLK,		NULL, "msm_sata.0"),
	CLK_DUMMY("slave_iface_clk",	SLAVE_IFACE_CLK,	NULL, "msm_sata.0"),
};

/*
 * Miscellaneous clock register initializations
 */

/* Read, modify, then write-back a register. */
static void __init rmwreg(uint32_t val, void *reg, uint32_t mask)
{
	uint32_t regval = readl_relaxed(reg);
	regval &= ~mask;
	regval |= val;
	writel_relaxed(regval, reg);
}

static struct pll_config_regs pll14_regs __initdata = {
	.l_reg = BB_PLL14_L_VAL_REG,
	.m_reg = BB_PLL14_M_VAL_REG,
	.n_reg = BB_PLL14_N_VAL_REG,
	.config_reg = BB_PLL14_CONFIG_REG,
	.mode_reg = BB_PLL14_MODE_REG,
};

static struct pll_config pll14_config __initdata = {
	.l = (0x11 | BVAL(31, 7, 0x620)),
	.m = 0x7,
	.n = 0x9,
	.vco_val = 0x0,
	.vco_mask = BM(17, 16),
	.pre_div_val = 0x0,
	.pre_div_mask = BIT(19),
	.post_div_val = 0x0,
	.post_div_mask = BM(21, 20),
	.mn_ena_val = BIT(22),
	.mn_ena_mask = BIT(22),
	.main_output_val = BIT(23),
	.main_output_mask = BIT(23),
};

static struct pll_config_regs pll4_regs __initdata = {
	.l_reg = LCC_PLL0_L_VAL_REG,
	.m_reg = LCC_PLL0_M_VAL_REG,
	.n_reg = LCC_PLL0_N_VAL_REG,
	.config_reg = LCC_PLL0_CONFIG_REG,
	.mode_reg = LCC_PLL0_MODE_REG,
};

static struct pll_config pll4_config_393 __initdata = {
	.l = 0xF,
	.m = 0x91,
	.n = 0xC7,
	.vco_val = 0x0,
	.vco_mask = BM(17, 16),
	.pre_div_val = 0x0,
	.pre_div_mask = BIT(19),
	.post_div_val = 0x0,
	.post_div_mask = BM(21, 20),
	.mn_ena_val = BIT(22),
	.mn_ena_mask = BIT(22),
	.main_output_val = BIT(23),
	.main_output_mask = BIT(23),
};

static void __init reg_init(void)
{
	uint32_t cfg_val;

	/*
	 * Initialize USB_HS_HCLK_FS registers: Set FORCE_C_ON bits so that
	 * core remain active during halt state of the clk. Also, set sleep
	 * and wake-up value to max.
	 */
	rmwreg(0x0000004F, USB_HS1_HCLK_FS_REG, 0x0000007F);
	rmwreg(0x0000004F, USB_HS3_HCLK_FS_REG, 0x0000007F);
	rmwreg(0x0000004F, USB_HS4_HCLK_FS_REG, 0x0000007F);

	/* Enable TSSC and PDM PXO sources. */
	writel_relaxed(BIT(11), TSSC_CLK_CTL_REG);
	writel_relaxed(BIT(15), PDM_CLK_NS_REG);

	/*
	 * Source the sata_phy_ref_clk from PXO and set predivider of
	 * sata_pmalive_clk to 1.
	 */
	rmwreg(0, SATA_PHY_REF_CLK_CTL_REG, 0x1);
	rmwreg(0, SATA_PMALIVE_CLK_CTL_REG, 0x3);

	/*
	 * TODO: Programming below PLLs and prng_clk is temporary and
	 *	 needs to be removed after bootloaders program them.
	 */
		u32 is_pll_enabled;

		/* Program pxo_src_clk to source from PXO */
		rmwreg(0x1, PXO_SRC_CLK_CTL_REG, 0x7);

		/* Check if PLL14 is active */
		is_pll_enabled = readl_relaxed(BB_PLL14_STATUS_REG) & BIT(16);
		if (!is_pll_enabled)
			/* Ref clk = 27MHz and program pll14 to 480MHz */
			configure_sr_pll(&pll14_config, &pll14_regs, 1);

		/* Program prng_clk to 64MHz if it isn't configured */
		if (!readl_relaxed(PRNG_CLK_NS_REG))
			writel_relaxed(0x2B, PRNG_CLK_NS_REG);

		/* Check whether Pll4 is active */

		is_pll_enabled = readl_relaxed(PLL_LOCK_DET_STATUS) & BIT(4);

		if (!is_pll_enabled) {
			configure_sr_pll(&pll4_config_393, &pll4_regs, 1);
			writel_relaxed(0x1, LCC_PRI_PLL_CLK_CTL_REG);
		}
		/* Enable Lpass pll src ctrl registers */
		cfg_val = readl_relaxed(LCC_PLL0_MODE_REG);
		cfg_val &= LCC_PLL0_MODE_MASK;
		cfg_val |= (LCC_PLL0_MODE_OUTCTRL |
				LCC_PLL0_MODE_BYPASSNL |
				LCC_PLL0_MODE_RESET_N);
		writel_relaxed(cfg_val, LCC_PLL0_MODE_REG);

		cfg_val = readl_relaxed(LCC_PXO_SRC_CTL_REG);
		cfg_val &= LCC_PXO_SRC_CTL_MASK;
		cfg_val |= LCC_PXO_SRC_CTL_GFM_CXO_SRC_SEL;
		writel_relaxed(cfg_val, LCC_PXO_SRC_CTL_REG);

		cfg_val = readl_relaxed(LCC_PRI_PLL_CLK_CTL);
		cfg_val &= LCC_PRI_PLL_CLK_CTL_MASK;
		cfg_val |= LCC_PRI_PLL_CLK_CTL_GFM_PRI_PLL_SRC_SEL;
		writel_relaxed(cfg_val, LCC_PRI_PLL_CLK_CTL);
		writel_relaxed(PLL_VAL, LCC_PLL0_CONFIG_REG);
}

struct clock_init_data ipq806x_clock_init_data __initdata;
static void __init ipq806x_clock_pre_init(void)
{
	/* Initialize clock registers. */
	reg_init();

	if ((readl_relaxed(PRNG_CLK_NS_REG) & 0x7F) == 0x2B)
		prng_clk.freq_tbl = clk_tbl_prng_64;

	clk_ops_local_pll.enable = sr_pll_clk_enable;

	nss_core_clk.freq_tbl = clk_tbl_nss;
	printk("clk_tbl_nss - loaded\n");
}

static void __init ipq806x_clock_post_init(void)
{
	/* Keep PXO on whenever APPS cpu is active */
	clk_prepare_enable(&pxo_a_clk.c);

	/* Initialize rates for clocks that only support one. */
	clk_set_rate(&pdm_clk.c, IPQ_PXO_FREQ);
	clk_set_rate(&prng_clk.c, prng_clk.freq_tbl->freq_hz);
	clk_set_rate(&tsif_ref_clk.c, 105000);
	clk_set_rate(&tssc_clk.c, IPQ_PXO_FREQ);
	clk_set_rate(&usb_hs1_xcvr_clk.c, 60000000);
	clk_set_rate(&usb_fs1_src_clk.c, 60000000);
	clk_set_rate(&usb_hsic_xcvr_fs_clk.c, 60000000);
	clk_set_rate(&usb_hsic_hsic_src_clk.c, 480000000);
	clk_set_rate(&usb_hsic_hsio_cal_clk.c, 9000000);
	clk_set_rate(&usb_hsic_system_clk.c, 60000000);
	clk_set_rate(&pcie_src_clk.c, 100000000);
	clk_set_rate(&pcie1_src_clk.c, 100000000);
	clk_set_rate(&pcie2_src_clk.c, 100000000);

	/*
	 * Keep sfab floor @ 133MHz @ nominal frequency
	 */
	clk_set_rate(&sfab_tmr_a_clk.c, CLK_IPQ_SFAB_NOMINAL_FREQ);
	clk_prepare_enable(&sfab_tmr_a_clk.c);
	/*
	 * Keep dfab floor @ 64MHz @ nominal frequency
	 */
	clk_set_rate(&dfab_msmbus_a_clk.c, CLK_IPQ_DFAB_NOMINAL_FREQ);
	clk_prepare_enable(&dfab_msmbus_a_clk.c);
	/*
	 * Keep sfpb floor @ 64MHz @ nominal frequency
	 */
	clk_set_rate(&sfpb_a_clk.c, CLK_IPQ_SFPB_NOMINAL_FREQ);
	clk_prepare_enable(&sfpb_a_clk.c);
}

static int __init ipq806x_clock_late_init(void)
{
	int rc;
	struct clk *cfpb_a_clk = clk_get_sys("clock-ipq806x", "cfpb_a_clk");

	/* Vote for CFPB to be on when Apps is active. */
	if (WARN(IS_ERR(cfpb_a_clk), "cfpb_a_clk not found (%ld)\n",
			PTR_ERR(cfpb_a_clk)))
		return PTR_ERR(cfpb_a_clk);
	rc = clk_set_rate(cfpb_a_clk, CLK_IPQ_CFPB_NOMINAL_FREQ);
	if (WARN(rc, "cfpb_a_clk rate was not set (%d)\n", rc))
		return rc;
	rc = clk_prepare_enable(cfpb_a_clk);
	if (WARN(rc, "cfpb_a_clk not enabled (%d)\n", rc))
		return rc;

	return 0;
}

struct clock_init_data ipq806x_clock_init_data __initdata = {
	.table = msm_clocks_ipq806x,
	.size = ARRAY_SIZE(msm_clocks_ipq806x),
	.pre_init = ipq806x_clock_pre_init,
	.post_init = ipq806x_clock_post_init,
	.late_init = ipq806x_clock_late_init,
};

struct clock_init_data ipq806x_dummy_clock_init_data __initdata = {
	.table = msm_clocks_ipq806x_dummy,
	.size = ARRAY_SIZE(msm_clocks_ipq806x_dummy),
};
