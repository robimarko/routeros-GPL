/* * Copyright (c) 2012 The Linux Foundation. All rights reserved.* */
/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/module.h>
#include <mach/msm_bus.h>
#include <mach/msm_bus_board.h>
#include <mach/rpm.h>
#include "msm_bus_core.h"

#define NMASTERS	MSM_BUS_MASTER_COUNT
#define NSLAVES		MSM_BUS_SLAVE_COUNT
#define NFAB_IPQ806X	6

enum msm_bus_fabric_tiered_slave_type {
	MSM_BUS_SYSTEM_TIERED_SLAVE_FAB_APPSS_0 = 1,
	MSM_BUS_SYSTEM_TIERED_SLAVE_FAB_APPSS_1,
	MSM_BUS_TIERED_SLAVE_SYSTEM_IMEM,

	MSM_BUS_TIERED_SLAVE_NSS_TCM = 1,
	MSM_BUS_NSS_TIERED_SLAVE_FAB_APPS_0,
	MSM_BUS_NSS_TIERED_SLAVE_FAB_APPS_1,

	MSM_BUS_TIERED_SLAVE_EBI1_CH0 = 1,
	MSM_BUS_TIERED_SLAVE_EBI1_CH1,
	MSM_BUS_TIERED_SLAVE_KMPSS_L2,
};

enum msm_bus_ipq806x_master_ports_type {
	MSM_BUS_SYSTEM_MASTER_PORT_APPSS_FAB = 0,
	MSM_BUS_MASTER_PORT_SPS,
	MSM_BUS_MASTER_PORT_ADM_PORT0,
	MSM_BUS_MASTER_PORT_ADM_PORT1,
	MSM_BUS_MASTER_PORT_PCIE_1,
	MSM_BUS_MASTER_PORT_USB3,
	MSM_BUS_MASTER_PORT_PCIE_2,
	MSM_BUS_MASTER_PORT_PCIE_3,
	MSM_BUS_MASTER_PORT_SATA,
	MSM_BUS_MASTER_PORT_LPASS,
	MSM_BUS_SYSTEM_MASTER_PORT_CPSS_FPB,
	MSM_BUS_SYSTEM_MASTER_PORT_SYSTEM_FPB,
	MSM_BUS_SYSTEM_MASTER_PORT_NSS_FPB,
	MSM_BUS_SYSTEM_MASTER_PORT_ADM_AHB_CI,

	/* NSS Port should come here */
	MSM_BUS_MASTER_PORT_UBI32_0 = 0,
	MSM_BUS_MASTER_PORT_UBI32_1,
	MSM_BUS_NSS_MASTER_PORT_NSS_FAB1,

	MSM_BUS_NSS_MASTER_PORT_APPS_FAB = 0,
	MSM_BUS_MASTER_PORT_UBI32,
	MSM_BUS_MASTER_PORT_GMAC_0,
	MSM_BUS_MASTER_PORT_GMAC_1,
	MSM_BUS_MASTER_PORT_GMAC_2,
	MSM_BUS_MASTER_PORT_GMAC_3,
	MSM_BUS_MASTER_PORT_CRYPTO5_0,
	MSM_BUS_MASTER_PORT_CRYPTO5_1,
	MSM_BUS_MASTER_PORT_CRYPTO5_2,
	MSM_BUS_MASTER_PORT_CRYPTO5_3,

	MSM_BUS_MASTER_PORT_KMPSS_M0 = 0,
	MSM_BUS_MASTER_PORT_KMPSS_M1,
	MSM_BUS_APPSS_MASTER_PORT_FAB_NSS_0,
	MSM_BUS_APPSS_MASTER_PORT_FAB_NSS_1,
	MSM_BUS_APPSS_MASTER_PORT_FAB_SYSTEM_0,
	MSM_BUS_APPSS_MASTER_PORT_FAB_SYSTEM_1,

};

enum msm_bus_ipq806x_slave_ports_type {
	MSM_BUS_NSS_SLAVE_PORT_APPS_FAB_0 = 0,
	MSM_BUS_SLAVE_PORT_NSS_TCM,

	MSM_BUS_NSS_SLAVE_PORT_APPS_FAB_1 = 0,
	MSM_BUS_NSS_SLAVE_PORT_NSS_FAB0,
	MSM_BUS_SLAVE_PORT_PCIE_2,
	MSM_BUS_SLAVE_PORT_PCIE_3,

	MSM_BUS_SLAVE_PORT_UBI32,
	MSM_BUS_SLAVE_PORT_GMAC,
	MSM_BUS_SLAVE_PORT_CRYPTO5,

	MSM_BUS_SLAVE_PORT_EBI1_CH0 = 0,
	MSM_BUS_SLAVE_PORT_KMPSS_L2,
	MSM_BUS_APPSS_SLAVE_PORT_NSS_FAB,
	MSM_BUS_SLAVE_PORT_SYSTEM_FAB,

	MSM_BUS_SYSTEM_SLAVE_PORT_APPSS_FAB_0 = 0,
	MSM_BUS_SYSTEM_SLAVE_PORT_APPSS_FAB_1,
	MSM_BUS_SLAVE_PORT_SPS,
	MSM_BUS_SLAVE_PORT_SYSTEM_IMEM,
	MSM_BUS_SLAVE_PORT_CORESIGHT,
	MSM_BUS_SLAVE_PORT_PCIE_1,
	MSM_BUS_SLAVE_PORT_KMPSS,
	MSM_BUS_SLAVE_PORT_USB3_1,
	MSM_BUS_SLAVE_PORT_LPASS,
	MSM_BUS_SYSTEM_SLAVE_PORT_CPSS_FPB,
	MSM_BUS_SYSTEM_SLAVE_PORT_SYSTEM_FPB,
	MSM_BUS_SYSTEM_SLAVE_PORT_NSS_FPB,
	MSM_BUS_SLAVE_PORT_SATA,
	MSM_BUS_SLAVE_PORT_USB3_2,
};

static int tier2[] = {MSM_BUS_BW_TIER2,};
static uint32_t master_iids[NMASTERS];
static uint32_t slave_iids[NSLAVES];

static int mport_kmpss_m0[] = {MSM_BUS_MASTER_PORT_KMPSS_M0,};
static int mport_kmpss_m1[] = {MSM_BUS_MASTER_PORT_KMPSS_M1,};

static int nss_mport_apps_fab[] = {MSM_BUS_NSS_MASTER_PORT_APPS_FAB,};
static int system_mport_appss_fab[] = {MSM_BUS_SYSTEM_MASTER_PORT_APPSS_FAB,};
static int sport_ebi1_ch0[] = {
	MSM_BUS_SLAVE_PORT_EBI1_CH0,
};
static int sport_kmpss_l2[] = {MSM_BUS_SLAVE_PORT_KMPSS_L2,};
static int appss_sport_nss_fab[] = {MSM_BUS_APPSS_SLAVE_PORT_NSS_FAB,};
static int sport_system_fab[] = {MSM_BUS_SLAVE_PORT_SYSTEM_FAB,};

static int tiered_slave_ebi1_ch0[] = {MSM_BUS_TIERED_SLAVE_EBI1_CH0,};

static int tiered_slave_kmpss[] = {MSM_BUS_TIERED_SLAVE_KMPSS_L2,};

static struct msm_bus_node_info apps_fabric_info[] = {
	{
		.id = MSM_BUS_MASTER_AMPSS_M0,
		.masterp = mport_kmpss_m0,
		.num_mports = ARRAY_SIZE(mport_kmpss_m0),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_MASTER_AMPSS_M1,
		.masterp = mport_kmpss_m1,
		.num_mports = ARRAY_SIZE(mport_kmpss_m1),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_SLAVE_EBI_CH0,
		.slavep = sport_ebi1_ch0,
		.num_sports = ARRAY_SIZE(sport_ebi1_ch0),
		.tier = tiered_slave_ebi1_ch0,
		.num_tiers = ARRAY_SIZE(tiered_slave_ebi1_ch0),
		.buswidth = 8,
		.slaveclk[DUAL_CTX] = "mem_clk",
		.slaveclk[ACTIVE_CTX] = "mem_a_clk",
	},

	{
		.id = MSM_BUS_SLAVE_AMPSS_L2,
		.slavep = sport_kmpss_l2,
		.num_sports = ARRAY_SIZE(sport_kmpss_l2),
		.tier = tiered_slave_kmpss,
		.num_tiers = ARRAY_SIZE(tiered_slave_kmpss),
		.buswidth = 8,
	},
	{
		.id = MSM_BUS_FAB_NSS_1,
		.gateway = 1,
		.slavep = appss_sport_nss_fab,
		.num_sports = ARRAY_SIZE(appss_sport_nss_fab),
		.masterp = nss_mport_apps_fab,
		.num_mports = ARRAY_SIZE(nss_mport_apps_fab),
		.buswidth = 8,
	},
	{
		.id = MSM_BUS_FAB_SYSTEM,
		.gateway = 1,
		.slavep = sport_system_fab,
		.num_sports = ARRAY_SIZE(sport_system_fab),
		.masterp = system_mport_appss_fab,
		.num_mports = ARRAY_SIZE(system_mport_appss_fab),
		.buswidth = 8,
	},
};

static int mport_sps[] = {MSM_BUS_MASTER_PORT_SPS,};
static int mport_adm_port0[] = {MSM_BUS_MASTER_PORT_ADM_PORT0,};
static int mport_adm_port1[] = {MSM_BUS_MASTER_PORT_ADM_PORT1,};
static int mport_usb30[] = {MSM_BUS_MASTER_PORT_USB3,};
static int mport_pcie[] = {
	MSM_BUS_MASTER_PORT_PCIE_1,
	MSM_BUS_MASTER_PORT_PCIE_2,
	MSM_BUS_MASTER_PORT_PCIE_3
};

static int mport_sata[] = {MSM_BUS_MASTER_PORT_SATA,};

static int mport_lpass[] = {MSM_BUS_MASTER_PORT_LPASS,};
static int system_mport_nss_fpb[] = {MSM_BUS_SYSTEM_MASTER_PORT_NSS_FPB,};
static int system_mport_adm_ahb_ci[] = {MSM_BUS_SYSTEM_MASTER_PORT_ADM_AHB_CI,};
static int appss_mport_fab_system[] = {
	MSM_BUS_APPSS_MASTER_PORT_FAB_SYSTEM_0,
	MSM_BUS_APPSS_MASTER_PORT_FAB_SYSTEM_1
};
static int mport_system_fpb[] = {MSM_BUS_SYSTEM_MASTER_PORT_SYSTEM_FPB,};
static int system_mport_cpss_fpb[] = {MSM_BUS_SYSTEM_MASTER_PORT_CPSS_FPB,};

static int system_sport_appss_fab[] = {
	MSM_BUS_SYSTEM_SLAVE_PORT_APPSS_FAB_0,
	MSM_BUS_SYSTEM_SLAVE_PORT_APPSS_FAB_1
};
static int system_sport_system_fpb[] = {MSM_BUS_SYSTEM_SLAVE_PORT_SYSTEM_FPB,};
static int system_sport_cpss_fpb[] = {MSM_BUS_SYSTEM_SLAVE_PORT_CPSS_FPB,};
static int sport_sps[] = {MSM_BUS_SLAVE_PORT_SPS,};
static int sport_system_imem[] = {MSM_BUS_SLAVE_PORT_SYSTEM_IMEM,};
static int sport_coresight[] = {MSM_BUS_SLAVE_PORT_CORESIGHT,};

static int sport_usb30[] = {
	MSM_BUS_SLAVE_PORT_USB3_1,
	MSM_BUS_SLAVE_PORT_USB3_2
};
static int sport_sata[] = {MSM_BUS_SLAVE_PORT_SATA,};
static int sport_kmpss[] = {MSM_BUS_SLAVE_PORT_KMPSS,};
static int sport_pcie1[] = {MSM_BUS_SLAVE_PORT_PCIE_1,};

static int sport_lpass[] = {MSM_BUS_SLAVE_PORT_LPASS,};
static int sport_nss_fpb[] = {MSM_BUS_SYSTEM_SLAVE_PORT_NSS_FPB,};

static int tiered_slave_system_imem[] = {MSM_BUS_TIERED_SLAVE_SYSTEM_IMEM,};
static int system_tiered_slave_fab_appss[] = {
	MSM_BUS_SYSTEM_TIERED_SLAVE_FAB_APPSS_0,
	MSM_BUS_SYSTEM_TIERED_SLAVE_FAB_APPSS_1,
};

static struct msm_bus_node_info system_fabric_info[]  = {
	{
		.id = MSM_BUS_MASTER_SPS,
		.masterp = mport_sps,
		.num_mports = ARRAY_SIZE(mport_sps),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_MASTER_ADM_PORT0,
		.masterp = mport_adm_port0,
		.num_mports = ARRAY_SIZE(mport_adm_port0),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_MASTER_ADM_PORT1,
		.masterp = mport_adm_port1,
		.num_mports = ARRAY_SIZE(mport_adm_port1),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_MASTER_USB3,
		.masterp = mport_usb30,
		.num_mports = ARRAY_SIZE(mport_usb30),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_MASTER_PCIE,
		.masterp = mport_pcie,
		.num_mports = ARRAY_SIZE(mport_pcie),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},

	{
		.id = MSM_BUS_MASTER_SATA,
		.masterp = mport_sata,
		.num_mports = ARRAY_SIZE(mport_sata),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},

	{
		.id = MSM_BUS_MASTER_LPASS,
		.masterp = mport_lpass,
		.num_mports = ARRAY_SIZE(mport_lpass),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_SYSTEM_MASTER_NSS_FPB,
		.masterp = system_mport_nss_fpb,
		.num_mports = ARRAY_SIZE(system_mport_nss_fpb),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_MASTER_ADM0_CI,
		.masterp = system_mport_adm_ahb_ci,
		.num_mports = ARRAY_SIZE(system_mport_adm_ahb_ci),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_FAB_APPSS,
		.gateway = 1,
		.slavep = system_sport_appss_fab,
		.num_sports = ARRAY_SIZE(system_sport_appss_fab),
		.masterp = appss_mport_fab_system,
		.num_mports = ARRAY_SIZE(appss_mport_fab_system),
		.tier = system_tiered_slave_fab_appss,
		.num_tiers = ARRAY_SIZE(system_tiered_slave_fab_appss),
		.buswidth = 8,
	},
	{
		.id = MSM_BUS_FAB_SYSTEM_FPB,
		.gateway = 1,
		.slavep = system_sport_system_fpb,
		.num_sports = ARRAY_SIZE(system_sport_system_fpb),
		.masterp = mport_system_fpb,
		.num_mports = ARRAY_SIZE(mport_system_fpb),
		.buswidth = 4,
	},
	{
		.id = MSM_BUS_FAB_CPSS_FPB,
		.gateway = 1,
		.slavep = system_sport_cpss_fpb,
		.num_sports = ARRAY_SIZE(system_sport_cpss_fpb),
		.masterp = system_mport_cpss_fpb,
		.num_mports = ARRAY_SIZE(system_mport_cpss_fpb),
		.buswidth = 4,
	},
	{
		.id = MSM_BUS_SLAVE_SPS,
		.slavep = sport_sps,
		.num_sports = ARRAY_SIZE(sport_sps),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
		.buswidth = 8,
		.slaveclk[DUAL_CTX] = "dfab_clk",
		.slaveclk[ACTIVE_CTX] = "dfab_a_clk",
	},
	{
		.id = MSM_BUS_SLAVE_SYSTEM_IMEM,
		.slavep = sport_system_imem,
		.num_sports = ARRAY_SIZE(sport_system_imem),
		.tier = tiered_slave_system_imem,
		.num_tiers = ARRAY_SIZE(tiered_slave_system_imem),
		.buswidth = 8,
	},
	{
		.id = MSM_BUS_SLAVE_CORESIGHT,
		.slavep = sport_coresight,
		.num_sports = ARRAY_SIZE(sport_coresight),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
		.buswidth = 8,
	},
	{
		.id = MSM_BUS_SLAVE_SYS_PCIE_1,
		.slavep = sport_pcie1,
		.num_sports = ARRAY_SIZE(sport_pcie1),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
		.buswidth = 8,
	},
	{
		.id = MSM_BUS_SLAVE_USB3,
		.slavep = sport_usb30,
		.num_sports = ARRAY_SIZE(sport_usb30),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
		.buswidth = 8,
	},
	{
		.id = MSM_BUS_SLAVE_SATA,
		.slavep = sport_sata,
		.num_sports = ARRAY_SIZE(sport_sata),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
		.buswidth = 8,
	},
	{
		.id = MSM_BUS_SLAVE_AMPSS,
		.slavep = sport_kmpss,
		.num_sports = ARRAY_SIZE(sport_kmpss),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
		.buswidth = 8,
	},

	{
		.id = MSM_BUS_SLAVE_LPASS,
		.slavep = sport_lpass,
		.num_sports = ARRAY_SIZE(sport_lpass),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
		.buswidth = 8,
	},
	{
		.id = MSM_BUS_SYSTEM_SLAVE_NSS_FPB,
		.slavep = sport_nss_fpb,
		.num_sports = ARRAY_SIZE(sport_nss_fpb),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
		.buswidth = 8,
	},
};

static int appss_mport_fab_0_nss[] = {MSM_BUS_APPSS_MASTER_PORT_FAB_NSS_0,};

static int nss_sport_apps_fab_0[] = {MSM_BUS_NSS_SLAVE_PORT_APPS_FAB_0,};

static int mport_nss_fab_0_ubi32_core0[] = {MSM_BUS_MASTER_PORT_UBI32_0,};
static int mport_nss_fab_0_ubi32_core1[] = {MSM_BUS_MASTER_PORT_UBI32_1,};

static int sport_nss_tcm[] = {MSM_BUS_SLAVE_PORT_NSS_TCM,};
static int nss_tiered_slave_fab_0_apps[] = {
	MSM_BUS_NSS_TIERED_SLAVE_FAB_APPS_0,
};
static int tiered_slave_nss_tcm[] = {MSM_BUS_TIERED_SLAVE_NSS_TCM,};


static struct msm_bus_node_info nss_fabric_0_info[]  = {
	{
		.id = MSM_BUS_MASTER_UBI32_0,
		.masterp = mport_nss_fab_0_ubi32_core0,
		.num_mports = ARRAY_SIZE(mport_nss_fab_0_ubi32_core0),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_MASTER_UBI32_1,
		.masterp = mport_nss_fab_0_ubi32_core1,
		.num_mports = ARRAY_SIZE(mport_nss_fab_0_ubi32_core1),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_FAB_APPSS,
		.gateway = 1,
		.slavep = nss_sport_apps_fab_0,
		.num_sports = ARRAY_SIZE(nss_sport_apps_fab_0),
		.masterp = appss_mport_fab_0_nss,
		.num_mports = ARRAY_SIZE(appss_mport_fab_0_nss),
		.tier = nss_tiered_slave_fab_0_apps,
		.num_tiers = ARRAY_SIZE(nss_tiered_slave_fab_0_apps),
		.buswidth = 16,
	},
	{
		.id = MSM_BUS_SLAVE_NSS_TCM,
		.slavep = sport_nss_tcm,
		.num_sports = ARRAY_SIZE(sport_nss_tcm),
		.tier = tiered_slave_nss_tcm,
		.num_tiers = ARRAY_SIZE(tiered_slave_nss_tcm),
		.buswidth = 8,
	},
};

static int appss_mport_fab_1_nss[] = {MSM_BUS_APPSS_MASTER_PORT_FAB_NSS_1,};
static int nss_sport_apps_fab_1[] = {MSM_BUS_NSS_SLAVE_PORT_APPS_FAB_1,};
static int nss1_sport_nss0[] = {MSM_BUS_NSS_SLAVE_PORT_NSS_FAB0,};
static int nss0_mport_nss1[] = {MSM_BUS_NSS_MASTER_PORT_NSS_FAB1,};

static int nss_tiered_slave_fab_1_apps[] = {
	MSM_BUS_NSS_TIERED_SLAVE_FAB_APPS_1,
};

static int mport_nss_fab_1_gmac_0[] = {MSM_BUS_MASTER_PORT_GMAC_0,};
static int mport_nss_fab_1_gmac_1[] = {MSM_BUS_MASTER_PORT_GMAC_1,};
static int mport_nss_fab_1_gmac_2[] = {MSM_BUS_MASTER_PORT_GMAC_2,};
static int mport_nss_fab_1_gmac_3[] = {MSM_BUS_MASTER_PORT_GMAC_3,};

static int mport_nss_fab_1_crypto_0[] = {MSM_BUS_MASTER_PORT_CRYPTO5_0,};
static int mport_nss_fab_1_crypto_1[] = {MSM_BUS_MASTER_PORT_CRYPTO5_1,};
static int mport_nss_fab_1_crypto_2[] = {MSM_BUS_MASTER_PORT_CRYPTO5_2,};
static int mport_nss_fab_1_crypto_3[] = {MSM_BUS_MASTER_PORT_CRYPTO5_3,};

static int nss_sport_gmac_fab_1[] = {MSM_BUS_SLAVE_PORT_GMAC,};
static int nss_sport_crypto_fab_1[] = {MSM_BUS_SLAVE_PORT_CRYPTO5,};
static int nss_sport_pcie2[] = {MSM_BUS_SLAVE_PORT_PCIE_2,};
static int nss_sport_pcie3[] = {MSM_BUS_SLAVE_PORT_PCIE_3,};

static struct msm_bus_node_info nss_fabric_1_info[]  = {
	{
		.id = MSM_BUS_FAB_APPSS,
		.gateway = 1,
		.slavep = nss_sport_apps_fab_1,
		.num_sports = ARRAY_SIZE(nss_sport_apps_fab_1),
		.masterp = appss_mport_fab_1_nss,
		.num_mports = ARRAY_SIZE(appss_mport_fab_1_nss),
		.tier = nss_tiered_slave_fab_1_apps,
		.num_tiers = ARRAY_SIZE(nss_tiered_slave_fab_1_apps),
		.buswidth = 16,
	},
	{
		.id = MSM_BUS_FAB_NSS_0,
		.gateway = 1,
		.slavep = nss1_sport_nss0,
		.num_sports = ARRAY_SIZE(nss1_sport_nss0),
		.masterp = nss0_mport_nss1,
		.num_mports = ARRAY_SIZE(nss0_mport_nss1),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
		.buswidth = 16,
	},
	{
		.id = MSM_BUS_MASTER_NSS_GMAC_0,
		.masterp = mport_nss_fab_1_gmac_0,
		.num_mports = ARRAY_SIZE(mport_nss_fab_1_gmac_0),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_MASTER_NSS_GMAC_1,
		.masterp = mport_nss_fab_1_gmac_1,
		.num_mports = ARRAY_SIZE(mport_nss_fab_1_gmac_1),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_MASTER_NSS_GMAC_2,
		.masterp = mport_nss_fab_1_gmac_2,
		.num_mports = ARRAY_SIZE(mport_nss_fab_1_gmac_2),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_MASTER_NSS_GMAC_3,
		.masterp = mport_nss_fab_1_gmac_3,
		.num_mports = ARRAY_SIZE(mport_nss_fab_1_gmac_3),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_MASTER_NSS_CRYPTO5_0,
		.masterp = mport_nss_fab_1_crypto_0,
		.num_mports = ARRAY_SIZE(mport_nss_fab_1_crypto_0),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_MASTER_NSS_CRYPTO5_1,
		.masterp = mport_nss_fab_1_crypto_1,
		.num_mports = ARRAY_SIZE(mport_nss_fab_1_crypto_1),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_MASTER_NSS_CRYPTO5_2,
		.masterp = mport_nss_fab_1_crypto_2,
		.num_mports = ARRAY_SIZE(mport_nss_fab_1_crypto_2),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_MASTER_NSS_CRYPTO5_3,
		.masterp = mport_nss_fab_1_crypto_3,
		.num_mports = ARRAY_SIZE(mport_nss_fab_1_crypto_3),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_SLAVE_NSS_GMAC,
		.slavep = nss_sport_gmac_fab_1,
		.num_sports = ARRAY_SIZE(nss_sport_gmac_fab_1),
		.buswidth = 8,
	},
	{
		.id = MSM_BUS_SLAVE_NSS_CRYPTO5,
		.slavep = nss_sport_crypto_fab_1,
		.num_sports = ARRAY_SIZE(nss_sport_crypto_fab_1),
		.buswidth = 8,
	},
	{
		.id = MSM_BUS_SLAVE_NSS_PCIE_2,
		.slavep = nss_sport_pcie2,
		.num_sports = ARRAY_SIZE(nss_sport_pcie2),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
		.buswidth = 8,
	},
	{
		.id = MSM_BUS_SLAVE_NSS_PCIE_3,
		.slavep = nss_sport_pcie3,
		.num_sports = ARRAY_SIZE(nss_sport_pcie3),
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
		.buswidth = 8,
	},

};



static struct msm_bus_node_info sys_fpb_fabric_info[]  = {
	{
		.id = MSM_BUS_FAB_SYSTEM,
		.gateway = 1,
		.slavep = system_sport_system_fpb,
		.num_sports = ARRAY_SIZE(system_sport_system_fpb),
		.masterp = mport_system_fpb,
		.num_mports = ARRAY_SIZE(mport_system_fpb),
		.buswidth = 4,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_MASTER_SPDM,
		.ahb = 1,
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_MASTER_RPM,
		.ahb = 1,
		.tier = tier2,
		.num_tiers = ARRAY_SIZE(tier2),
	},
	{
		.id = MSM_BUS_SLAVE_SPDM,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_RPM,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_RPM_MSG_RAM,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_MPM,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_PMIC1_SSBI1_A,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_PMIC1_SSBI1_B,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_PMIC1_SSBI1_C,
		.buswidth = 8,
		.ahb = 1,
	},
};

static struct msm_bus_node_info cpss_fpb_fabric_info[] = {
	{
		.id = MSM_BUS_FAB_SYSTEM,
		.gateway = 1,
		.slavep = system_sport_cpss_fpb,
		.num_sports = ARRAY_SIZE(system_sport_cpss_fpb),
		.masterp = system_mport_cpss_fpb,
		.num_mports = ARRAY_SIZE(system_mport_cpss_fpb),
		.buswidth = 4,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_GSBI1_UART,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_GSBI2_UART,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_GSBI4_UART,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_GSBI5_UART,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_GSBI6_UART,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_GSBI7_UART,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_GSBI1_QUP,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_GSBI2_QUP,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_GSBI4_QUP,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_GSBI5_QUP,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_GSBI6_QUP,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_GSBI7_QUP,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_EBI2_NAND,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_EBI2_CS0,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_EBI2_CS1,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_EBI2_CS2,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_EBI2_CS3,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_EBI2_CS4,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_EBI2_CS5,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_USB_FS1,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_TSIF,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_MSM_TSSC,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_MSM_PDM,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_MSM_DIMEM,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_MSM_TCSR,
		.buswidth = 8,
		.ahb = 1,
	},
	{
		.id = MSM_BUS_SLAVE_MSM_PRNG,
		.buswidth = 4,
		.ahb = 1,
	},
};

static void msm_bus_board_assign_iids(struct msm_bus_fabric_registration
	*fabreg, int fabid)
{
	int i;
	for (i = 0; i < fabreg->len; i++) {
		if (!fabreg->info[i].gateway) {
			fabreg->info[i].priv_id = fabid + fabreg->info[i].id;
			if (fabreg->info[i].id < SLAVE_ID_KEY) {
				WARN(fabreg->info[i].id >= NMASTERS,
					"id %d exceeds array size!\n",
					fabreg->info[i].id);
				master_iids[fabreg->info[i].id] =
					fabreg->info[i].priv_id;
			} else {
				WARN((fabreg->info[i].id - SLAVE_ID_KEY) >=
					NSLAVES, "id %d exceeds array size!\n",
					fabreg->info[i].id);
				slave_iids[fabreg->info[i].id - (SLAVE_ID_KEY)]
					= fabreg->info[i].priv_id;
			}
		} else
			fabreg->info[i].priv_id = fabreg->info[i].id;
	}
}

static int msm_bus_board_ipq806x_get_iid(int id)
{
	if ((id < SLAVE_ID_KEY && id >= NMASTERS) ||
		id >= (SLAVE_ID_KEY + NSLAVES)) {
		MSM_BUS_ERR("Cannot get iid. Invalid id %d passed\n", id);
		return -EINVAL;
	}

	return CHECK_ID(((id < SLAVE_ID_KEY) ? master_iids[id] :
		slave_iids[id - SLAVE_ID_KEY]), id);
}

static struct msm_bus_board_algorithm msm_bus_board_algo = {
	.board_nfab = NFAB_IPQ806X,
	.get_iid = msm_bus_board_ipq806x_get_iid,
	.assign_iids = msm_bus_board_assign_iids,
};

struct msm_bus_fabric_registration msm_bus_ipq806x_apps_fabric_pdata = {
	.id = MSM_BUS_FAB_APPSS,
	.name = "msm_apps_fab",
	.info = apps_fabric_info,
	.len = ARRAY_SIZE(apps_fabric_info),
	.ahb = 0,
	.fabclk[DUAL_CTX] = "bus_clk",
	.fabclk[ACTIVE_CTX] = "bus_a_clk",
	.haltid = MSM_RPM_ID_APPS_FABRIC_CFG_HALT_0,
	.offset = MSM_RPM_ID_APPS_FABRIC_ARB_0,
	.nmasters = 6,
	.nslaves = 5,
	.ntieredslaves = 3,
	.board_algo = &msm_bus_board_algo,
};

struct msm_bus_fabric_registration msm_bus_ipq806x_sys_fabric_pdata = {
	.id = MSM_BUS_FAB_SYSTEM,
	.name = "msm_sys_fab",
	system_fabric_info,
	ARRAY_SIZE(system_fabric_info),
	.ahb = 0,
	.fabclk[DUAL_CTX] = "bus_clk",
	.fabclk[ACTIVE_CTX] = "bus_a_clk",
	.haltid = MSM_RPM_ID_SYS_FABRIC_CFG_HALT_0,
	.offset = MSM_RPM_ID_SYSTEM_FABRIC_ARB_0,
	.nmasters = 14,
	.nslaves = 15,
	.ntieredslaves = 3,
	.board_algo = &msm_bus_board_algo,
};

struct msm_bus_fabric_registration msm_bus_ipq806x_nss_fabric_0_pdata = {
	.id = MSM_BUS_FAB_NSS_0,
	.name = "msm_nss_fab_0",
	nss_fabric_0_info,
	ARRAY_SIZE(nss_fabric_0_info),
	.ahb = 0,
	.fabclk[DUAL_CTX] = "bus_clk",
	.fabclk[ACTIVE_CTX] = "bus_a_clk",
	.haltid = MSM_RPM_ID_NSS_FABRIC_0_CFG_HALT_0,
	.offset = MSM_RPM_ID_NSS_FABRIC_0_ARB_0,
	.nmasters = 3,
	.nslaves = 2,
	.ntieredslaves = 3,
	.board_algo = &msm_bus_board_algo,
};

struct msm_bus_fabric_registration msm_bus_ipq806x_nss_fabric_1_pdata = {
	.id = MSM_BUS_FAB_NSS_1,
	.name = "msm_nss_fab_1",
	nss_fabric_1_info,
	ARRAY_SIZE(nss_fabric_1_info),
	.ahb = 0,
	.fabclk[DUAL_CTX] = "bus_clk",
	.fabclk[ACTIVE_CTX] = "bus_a_clk",
	.haltid = MSM_RPM_ID_NSS_FABRIC_1_CFG_HALT_0,
	.offset = MSM_RPM_ID_NSS_FABRIC_1_ARB_0,
	.nmasters = 6,
	.nslaves = 6,
	.ntieredslaves = 3,
	.board_algo = &msm_bus_board_algo,
};
struct msm_bus_fabric_registration msm_bus_ipq806x_sys_fpb_pdata = {
	.id = MSM_BUS_FAB_SYSTEM_FPB,
	.name = "msm_sys_fpb",
	sys_fpb_fabric_info,
	ARRAY_SIZE(sys_fpb_fabric_info),
	.ahb = 1,
	.fabclk[DUAL_CTX] = "bus_clk",
	.fabclk[ACTIVE_CTX] = "bus_a_clk",
	.nmasters = 0,
	.nslaves = 0,
	.ntieredslaves = 0,
	.board_algo = &msm_bus_board_algo,
};

struct msm_bus_fabric_registration msm_bus_ipq806x_cpss_fpb_pdata = {
	.id = MSM_BUS_FAB_CPSS_FPB,
	.name = "msm_cpss_fpb",
	cpss_fpb_fabric_info,
	ARRAY_SIZE(cpss_fpb_fabric_info),
	.ahb = 1,
	.fabclk[DUAL_CTX] = "bus_clk",
	.fabclk[ACTIVE_CTX] = "bus_a_clk",
	.nmasters = 0,
	.nslaves = 0,
	.ntieredslaves = 0,
	.board_algo = &msm_bus_board_algo,
};

int msm_bus_board_rpm_get_il_ids(uint16_t id[])
{
	return -ENXIO;
}
