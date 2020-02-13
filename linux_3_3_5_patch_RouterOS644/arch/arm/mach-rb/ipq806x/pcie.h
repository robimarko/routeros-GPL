/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
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

#ifndef __ARCH_ARM_MACH_MSM_PCIE_H
#define __ARCH_ARM_MACH_MSM_PCIE_H

#include <linux/clk.h>
#include <linux/compiler.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/types.h>
#include <mach/msm_pcie.h>

#define MSM_PCIE_MAX_VREG 4
#define MSM_PCIE_MAX_CLK  4

#define PCIE_DBG(x...) do {              \
	if (msm_pcie_get_debug_mask())   \
		pr_info(x);              \
	} while (0)

#define PCIE_RESET		(MSM_CLK_CTL_BASE + 0x22DC)
#define PCIE_1_RESET		(MSM_CLK_CTL_BASE + 0x3A9C)
#define PCIE_2_RESET		(MSM_CLK_CTL_BASE + 0x3ADC)

#define PCIE_SFAB_AXI_S5_FCLK_CTL	(MSM_CLK_CTL_BASE + 0x2154)
#define PCIE_1_ACLK_CTL			(MSM_CLK_CTL_BASE + 0x3A80)
#define PCIE_2_ACLK_CTL			(MSM_CLK_CTL_BASE + 0x3AC0)

/* resources from device file */
enum msm_pcie_res {
	/* platform defined resources */
	MSM_PCIE_RES_PARF,
	MSM_PCIE_RES_ELBI,
	MSM_PCIE_RES_PCIE20,
	MSM_PCIE_MAX_PLATFORM_RES,

	/* other resources */
	MSM_PCIE_RES_AXI_CONF = MSM_PCIE_MAX_PLATFORM_RES,
	MSM_PCIE_MAX_RES,
};

/* voltage regulator info structrue */
struct msm_pcie_vreg_info_t {
	struct regulator  *hdl;
	char              *name;
	uint32_t           max_v;
	uint32_t           min_v;
	uint32_t           opt_mode;
};

/* clock info structure */
struct msm_pcie_clk_info_t {
	struct clk  *hdl;
	char        *name;
};

/* resource info structure */
struct msm_pcie_res_info_t {
	char            *name;
	struct resource *resource;
	void __iomem    *base;
};

/* msm pcie device structure */
struct msm_pcie_dev_t {
	struct platform_device		*pdev;

	struct msm_pcie_vreg_info_t	*vreg;
	struct msm_pcie_gpio_info_t	*gpio;
	struct msm_pcie_clk_info_t	*clk;
	struct msm_pcie_res_info_t	*res;
        msm_pcie_port_en_t              *port_en;

	void __iomem			*parf;
	void __iomem			*elbi;
	void __iomem			*pcie20;
	void __iomem			*axi_conf;
	void __iomem			*reset_reg;
	void __iomem			*porten_reg;
	uint32_t			axi_bar_start;
	uint32_t			axi_bar_end;

	struct resource			dev_mem_res;

	uint32_t			wake_n;
	uint32_t			vreg_n;
	uint32_t			clk_n;
	uint32_t			msi_irq;
	uint32_t			inta;
	uint32_t			axi_addr;
	uint32_t			axi_size;
	uint32_t			bus;
};

extern uint32_t msm_pcie_irq_init(struct msm_pcie_dev_t *dev);
extern void msm_pcie_irq_deinit(struct msm_pcie_dev_t *dev);
extern int msm_pcie_get_debug_mask(void);

#endif
