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

#ifndef __ASM_ARCH_MSM_PCIE_H
#define __ASM_ARCH_MSM_PCIE_H

#include <linux/types.h>

/* gpios */
enum msm_pcie_gpio {
	MSM_PCIE_GPIO_RST_N,
	MSM_PCIE_GPIO_PWR_EN,
	MSM_PCIE_MAX_GPIO
};

/* gpio info structure */
struct msm_pcie_gpio_info_t {
	char      *name;
	uint32_t   num;
	uint32_t   on;
};

/* Port Enable info */
typedef struct {
	void __iomem  *reg;
	uint32_t      bits;
} msm_pcie_port_en_t;

/* msm pcie platfrom data */
struct msm_pcie_platform {
	struct msm_pcie_gpio_info_t  *gpio;

	uint32_t                     axi_addr;
	uint32_t                     axi_size;
	uint32_t                     wake_n;
	uint32_t                     vreg_n;
	uint32_t                     clk_n;
	struct platform_device       *pdev;

	struct msm_pcie_vreg_info_t  *vreg;
	struct msm_pcie_clk_info_t   *clk;
	struct msm_pcie_res_info_t   *res;

	void __iomem                 *parf;
	void __iomem                 *elbi;
	void __iomem                 *pcie20;
	void __iomem                 *axi_conf;
	void __iomem                 *reset_reg;
	void __iomem                 *porten_reg;
	uint32_t                     axi_bar_start;
	uint32_t                     axi_bar_end;

	struct resource              dev_mem_res;

	uint32_t                     msi_irq;
	uint32_t                     inta;
        msm_pcie_port_en_t           *port_en;
	uint32_t                     term_offset;
};

#endif
