/* * Copyright (c) 2012 The Linux Foundation. All rights reserved.* */
/*
 * Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
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
#include <linux/module.h>
#include <linux/platform_device.h>
#include <mach/msm_bus_board.h>
#include <mach/msm_bus.h>

#include "mach/socinfo.h"
#include "acpuclock.h"
#include "acpuclock-krait.h"

static struct hfpll_data hfpll_data __initdata = {
	.mode_offset = 0x00,
	.l_offset = 0x08,
	.m_offset = 0x0C,
	.n_offset = 0x10,
	.config_offset = 0x04,
	.status_offset = 0x1C,
	.config_val = 0x7845C665,
	.has_droop_ctl = true,
	.droop_offset = 0x14,
	.droop_val = 0x0108C000,
	.low_vdd_l_max = 22,
	.nom_vdd_l_max = 42,
	.vdd[HFPLL_VDD_NONE] =       0,
	.vdd[HFPLL_VDD_LOW]  = 1100000,
	.vdd[HFPLL_VDD_NOM]  = 1100000,
	.vdd[HFPLL_VDD_HIGH] = 1150000,
};

static struct scalable scalable[] __initdata = {
	[CPU0] = {
		.hfpll_phys_base = 0x00903200,
		.aux_clk_sel_phys = 0x02088014,
		.aux_clk_sel = 3,
		.sec_clk_sel = 2,
		.l2cpmr_iaddr = 0x4501,
		.vreg[VREG_CORE] = { "krait0", 1250000 },
		.vreg[VREG_MEM]  = { "krait0_mem", 1150000 },
		.vreg[VREG_DIG]  = { "krait0_dig", 1150000 },
		.vreg[VREG_HFPLL_A] = { "krait0_hfpll", 1100000 },
		.avs_enabled = false,
		.vddmx_scale_en = true,
	},
	[CPU1] = {
		.hfpll_phys_base = 0x00903240,
		.aux_clk_sel_phys = 0x02098014,
		.aux_clk_sel = 3,
		.sec_clk_sel = 2,
		.l2cpmr_iaddr = 0x5501,
		.vreg[VREG_CORE] = { "krait1", 1250000 },
		.vreg[VREG_MEM]  = { "krait1_mem", 1150000 },
		.vreg[VREG_DIG]  = { "krait1_dig", 1150000 },
		.vreg[VREG_HFPLL_A] = { "krait1_hfpll", 1100000 },
		.avs_enabled = false,
		.vddmx_scale_en = true,
	},
	[L2] = {
		.hfpll_phys_base = 0x00903300,
		.aux_clk_sel_phys = 0x02011028,
		.aux_clk_sel = 3,
		.sec_clk_sel = 2,
		.l2cpmr_iaddr = 0x0500,
		.vreg[VREG_HFPLL_A] = { "l2_hfpll", 1100000 },
		.avs_enabled = false,
		.vddmx_scale_en = true,
	},
};

/*
 * Apps Fabric/DDR BW levels
 */
static struct msm_bus_paths bw_level_tbl[] __initdata = {
	[0] =  BW_MBPS(640), /* At least  80 MHz on bus. */
	[1] = BW_MBPS(1064), /* At least 133 MHz on bus. */
	[2] = BW_MBPS(1600), /* At least 200 MHz on bus. */
	[3] = BW_MBPS(2128), /* At least 266 MHz on bus. */
	[4] = BW_MBPS(3200), /* At least 400 MHz on bus. */
	[5] = BW_MBPS(4264), /* At least 533 MHz on bus. */
};

static struct msm_bus_scale_pdata bus_scale_data __initdata = {
	.usecase = bw_level_tbl,
	.num_usecases = ARRAY_SIZE(bw_level_tbl),
	.active_only = 1,
	.name = "acpuclk-ipq806x",
};

/*
 * PVS tables for Akronite based platforms
 *
 * These are based on PVS Characterization results for the 19x19 Package across Voltage and Temperature on the PVS Fused parts
 * These are based on +/-5% Margin on the VDD_APCx that is advertised in our Datasheet across Temperature
 */
static struct l2_level l2_freq_tbl[] __initdata = {
//	[0] = { {  384000, PLL_8, 0, 0x00 }, 1100000, 1100000, 4 },
	[1] = { { 1000000, HFPLL, 1, 0x28 }, 1100000, 1100000, 4 },
	[2] = { { 1200000, HFPLL, 1, 0x30 }, 1150000, 1150000, 5 },
	{ }
};

static struct acpu_level tbl_slow[] __initdata = {
//	{ 1, {   384000, PLL_8, 0, 0x00 }, L2(0), 1000000 },
	{ 1, {   600000, HFPLL, 1, 0x18 }, L2(1), 1050000 },
	{ 1, {   800000, HFPLL, 1, 0x20 }, L2(1), 1100000 },
	{ 1, {  1000000, HFPLL, 1, 0x28 }, L2(1), 1150000 },
	{ 1, {  1200000, HFPLL, 1, 0x30 }, L2(2), 1200000 },
	{ 1, {  1400000, HFPLL, 1, 0x38 }, L2(2), 1250000 },
	{ 0, { 0 } }
};

static struct acpu_level tbl_nominal[] __initdata = {
//	{ 1, {   384000, PLL_8, 0, 0x00 }, L2(0),  925000 },
	{ 1, {   600000, HFPLL, 1, 0x18 }, L2(1),  975000 },
	{ 1, {   800000, HFPLL, 1, 0x20 }, L2(1), 1025000 },
	{ 1, {  1000000, HFPLL, 1, 0x28 }, L2(1), 1075000 },
	{ 1, {  1200000, HFPLL, 1, 0x30 }, L2(2), 1125000 },
	{ 1, {  1400000, HFPLL, 1, 0x38 }, L2(2), 1200000 },
	{ 0, { 0 } }
};

static struct acpu_level tbl_fast[] __initdata = {
//	{ 1, {   384000, PLL_8, 0, 0x00 }, L2(0),  875000 },
	{ 1, {   600000, HFPLL, 1, 0x18 }, L2(1),  925000 },
	{ 1, {   800000, HFPLL, 1, 0x20 }, L2(1),  995000 },
	{ 1, {  1000000, HFPLL, 1, 0x28 }, L2(1), 1025000 },
	{ 1, {  1200000, HFPLL, 1, 0x30 }, L2(2), 1075000 },
	{ 1, {  1400000, HFPLL, 1, 0x38 }, L2(2), 1125000 },
	{ 0, { 0 } }
};

static struct acpu_level tbl_superfast[] __initdata = {
//	{ 1, {   384000, PLL_8, 0, 0x00 }, L2(0),  800000 },
	{ 1, {   600000, HFPLL, 1, 0x18 }, L2(1),  850000 },
	{ 1, {   800000, HFPLL, 1, 0x20 }, L2(1),  900000 },
	{ 1, {  1000000, HFPLL, 1, 0x28 }, L2(1),  950000 },
	{ 1, {  1200000, HFPLL, 1, 0x30 }, L2(2), 1000000 },
	{ 1, {  1400000, HFPLL, 1, 0x38 }, L2(2), 1050000 },
	{ 0, { 0 } }
};

static struct pvs_table pvs_tables[NUM_SPEED_BINS][NUM_PVS] __initdata = {
	[0][PVS_SLOW] = {tbl_slow, sizeof(tbl_slow), 0},
	[0][PVS_NOMINAL] = {tbl_nominal, sizeof(tbl_nominal), 0},
	[0][PVS_FAST] = {tbl_fast, sizeof(tbl_fast), 0},
	[0][PVS_FASTER] = {tbl_superfast, sizeof(tbl_superfast), 0},
};

/*
 * PVS tables for Akronite-lite based platforms
 */
static struct acpu_level tbl_slow_lite[] __initdata = {
//	{ 1, {   384000, PLL_8, 0, 0x00 }, L2(0),  950000},
	{ 1, {   600000, HFPLL, 1, 0x18 }, L2(1), 1000000 },
	{ 1, {   800000, HFPLL, 1, 0x20 }, L2(1), 1050000 },
	{ 1, {  1000000, HFPLL, 1, 0x28 }, L2(1), 1100000 },
	{ 0, { 0 } }
};

static struct acpu_level tbl_nominal_lite[] __initdata = {
//	{ 1, {   384000, PLL_8, 0, 0x00 }, L2(0),  900000 },
	{ 1, {   600000, HFPLL, 1, 0x18 }, L2(1),  950000 },
	{ 1, {   800000, HFPLL, 1, 0x20 }, L2(1), 1000000 },
	{ 1, {  1000000, HFPLL, 1, 0x28 }, L2(1), 1050000 },
	{ 0, { 0 } }
};

static struct acpu_level tbl_fast_lite[] __initdata = {
//	{ 1, {   384000, PLL_8, 0, 0x00 }, L2(0),  850000 },
	{ 1, {   600000, HFPLL, 1, 0x18 }, L2(1),  900000 },
	{ 1, {   800000, HFPLL, 1, 0x20 }, L2(1),  950000 },
	{ 1, {  1000000, HFPLL, 1, 0x28 }, L2(1), 1000000 },
	{ 0, { 0 } }
};

static struct acpu_level tbl_superfast_lite[] __initdata = {
//	{ 1, {   384000, PLL_8, 0, 0x00 }, L2(0), 800000 },
	{ 1, {   600000, HFPLL, 1, 0x18 }, L2(1), 850000 },
	{ 1, {   800000, HFPLL, 1, 0x20 }, L2(1), 900000 },
	{ 1, {  1000000, HFPLL, 1, 0x28 }, L2(1), 950000 },
	{ 0, { 0 } }
};

static struct pvs_table pvs_tables_lite[NUM_SPEED_BINS][NUM_PVS] __initdata = {
	[0][PVS_SLOW]    = {tbl_slow_lite, sizeof(tbl_slow_lite),     0 },
	[0][PVS_NOMINAL] = {tbl_nominal_lite, sizeof(tbl_nominal_lite), 0},
	[0][PVS_FAST] = {tbl_fast_lite, sizeof(tbl_fast_lite), 0},
	[0][PVS_FASTER] = {tbl_superfast_lite, sizeof(tbl_superfast_lite), 0},
};

static struct acpuclk_krait_params acpuclk_ipq806x_params __initdata = {
	.scalable = scalable,
	.scalable_size = sizeof(scalable),
	.hfpll_data = &hfpll_data,
	.pvs_tables = pvs_tables,
	.l2_freq_tbl = l2_freq_tbl,
	.l2_freq_tbl_size = sizeof(l2_freq_tbl),
	.bus_scale = &bus_scale_data,
	.pte_efuse_phys = 0x007000C0,
	.stby_khz = 384000,
};

static int __init acpuclk_ipq806x_probe(struct platform_device *pdev)
{
	return acpuclk_krait_init(&pdev->dev, &acpuclk_ipq806x_params);
}

static struct platform_driver acpuclk_ipq806x_driver = {
	.driver = {
		.name = "acpuclk-ipq806x",
		.owner = THIS_MODULE,
	},
};

static int __init acpuclk_ipq806x_init(void)
{
	return platform_driver_probe(&acpuclk_ipq806x_driver,
				     acpuclk_ipq806x_probe);
}
device_initcall(acpuclk_ipq806x_init);
