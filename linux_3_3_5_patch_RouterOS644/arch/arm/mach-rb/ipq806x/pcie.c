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

/*
 * MSM PCIe controller driver.
 */

#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <asm/mach/pci.h>
#include <mach/msm_iomap.h>
#include <mach/system.h>
#include <asm/mach/arch.h>

#include "pcie.h"

/* Root Complex Port vendor/device IDs */
#define PCIE_VENDOR_ID_RCP             0x17cb
#define PCIE_DEVICE_ID_RCP             0x0101

#define __mask(a, b)	(((1 << ((a) + 1)) - 1) & ~((1 << (b)) - 1))
#define __set(v, a, b)	(((v) << (b)) & __mask(a, b))

#define PCIE20_PARF_PCS_DEEMPH         0x34
#define PCIE20_PARF_PCS_DEEMPH_TX_DEEMPH_GEN1(x)	__set(x, 21, 16)
#define PCIE20_PARF_PCS_DEEMPH_TX_DEEMPH_GEN2_3_5DB(x)	__set(x, 13, 8)
#define PCIE20_PARF_PCS_DEEMPH_TX_DEEMPH_GEN2_6DB(x)	__set(x, 5, 0)

#define PCIE20_PARF_PCS_SWING          0x38
#define PCIE20_PARF_PCS_SWING_TX_SWING_FULL(x)		__set(x, 14, 8)
#define PCIE20_PARF_PCS_SWING_TX_SWING_LOW(x)		__set(x, 6, 0)

#define PCIE20_PARF_PHY_CTRL           0x40
#define PCIE20_PARF_PHY_CTRL_PHY_TX0_TERM_OFFST(x)	__set(x, 20, 16)
#define PCIE20_PARF_PHY_CTRL_PHY_LOS_LEVEL(x)		__set(x, 12, 8)
#define PCIE20_PARF_PHY_CTRL_PHY_RTUNE_REQ		(1 << 4)
#define PCIE20_PARF_PHY_CTRL_PHY_TEST_BURNIN		(1 << 2)
#define PCIE20_PARF_PHY_CTRL_PHY_TEST_BYPASS		(1 << 1)
#define PCIE20_PARF_PHY_CTRL_PHY_TEST_PWR_DOWN		(1 << 0)

#define PCIE20_PARF_PHY_REFCLK         0x4C
#define PCIE20_PARF_CONFIG_BITS        0x50

#define PCIE20_ELBI_SYS_CTRL           0x04

#define PCIE20_CAP                     0x70
#define PCIE20_CAP_LINKCTRLSTATUS      (PCIE20_CAP + 0x10)

#define PCIE20_COMMAND_STATUS          0x04
#define PCIE20_BUSNUMBERS              0x18
#define PCIE20_MEMORY_BASE_LIMIT       0x20
#define PCIE20_LNK_CAS2                0xA0

#define PCIE20_DEV_CAS			0x78
#define PCIE20_MRRS_MASK		__mask(14, 12)
#define PCIE20_MRRS(x)			__set(x, 14, 12)
#define PCIE20_MPS_MASK			__mask(7, 5)
#define PCIE20_MPS(x)			__set(x, 7, 5)
#define PCIE20_AXI_MSTR_RESP_COMP_CTRL0 0x818
#define PCIE20_AXI_MSTR_RESP_COMP_CTRL1 0x81c
#define PCIE20_PLR_IATU_VIEWPORT        0x900
#define PCIE20_PLR_IATU_CTRL1           0x904
#define PCIE20_PLR_IATU_CTRL2           0x908
#define PCIE20_PLR_IATU_LBAR            0x90C
#define PCIE20_PLR_IATU_UBAR            0x910
#define PCIE20_PLR_IATU_LAR             0x914
#define PCIE20_PLR_IATU_LTAR            0x918
#define PCIE20_PLR_IATU_UTAR            0x91c

#define MSM_PCIE_DEV_BAR_ADDR	PCIBIOS_MIN_MEM
#define MSM_PCIE_DEV_CFG_ADDR	0x01000000

#define RD 0
#define WR 1

/* PCIE AXI address space */
#define PCIE_AXI_CONF_SIZE   SZ_1M

/* debug mask sys interface */
static int msm_pcie_debug_mask;
module_param_named(debug_mask, msm_pcie_debug_mask,
			    int, S_IRUGO | S_IWUSR | S_IWGRP);

/* msm pcie device data */
static struct msm_pcie_dev_t msm_pcie_rc_dev[MSM_NUM_PCIE];

int msm_pcie_get_debug_mask(void)
{
	return msm_pcie_debug_mask;
}

static void msm_pcie_write_mask(void __iomem *addr,
				uint32_t clear_mask, uint32_t set_mask)
{
	uint32_t val;

	val = (readl_relaxed(addr) & ~clear_mask) | set_mask;
	writel_relaxed(val, addr);
	wmb();  /* ensure data is written to hardware register */
}

static int msm_pcie_is_link_up(struct msm_pcie_dev_t *msm_pcie_dev)
{
	return readl_relaxed(msm_pcie_dev->pcie20 + PCIE20_CAP_LINKCTRLSTATUS) &
				BIT(29);
}

inline struct msm_pcie_dev_t *pcie_bus_to_msm_pcie_dev(struct pci_bus *bus)
{
	return ((struct pci_sys_data *)(bus->sysdata))->private_data;
}
#define bus_to_mpdev		pcie_bus_to_msm_pcie_dev

inline struct msm_pcie_dev_t *pdev_id_to_msm_pcie_dev(int id)
{
	return &msm_pcie_rc_dev[id];
}
#define pdev_id_to_mpdev	pdev_id_to_msm_pcie_dev

inline int is_msm_pcie_rc(struct pci_bus *bus)
{
	struct msm_pcie_dev_t *dev = bus_to_mpdev(bus);
	return (bus->number == dev->bus);
}

/* PCIe TLP types that we are interested in */
#define PCI_CFG0_RDWR	0x4
#define PCI_CFG1_RDWR	0x5

inline int msm_pcie_get_cfgtype(struct pci_bus *bus)
{
	int root = bus_to_mpdev(bus)->bus;

	/*
	 * http://www.tldp.org/LDP/tlk/dd/pci.html
	 * Pass it onto the secondary bus interface unchanged if the
	 * bus number specified is greater than the secondary bus
	 * number and less than or equal to the subordinate bus
	 * number.
	 *
	 * Read/Write to the RC and Device/Switch connected to the RC
	 * are CFG0 type transactions. Rest have to be forwarded
	 * down stream as CFG1 transactions.
	 *
	 */
	if (bus->number == root || bus->primary == root)
		return PCI_CFG0_RDWR;

	return PCI_CFG1_RDWR;
}

void msm_pcie_config_cfgtype(struct pci_bus *bus, u32 devfn)
{
	uint32_t bdf, cfgtype;
	struct msm_pcie_dev_t *dev = bus_to_mpdev(bus);

	cfgtype = msm_pcie_get_cfgtype(bus);

	if (cfgtype == PCI_CFG0_RDWR) {
		bdf = MSM_PCIE_DEV_CFG_ADDR;
	} else {
		/*
		 * iATU Lower Target Address Register
		 *	Bits	Description
		 *	*-1:0	Forms bits [*:0] of the
		 *		start address of the new
		 *		address of the translated
		 *		region. The start address
		 *		must be aligned to a
		 *		CX_ATU_MIN_REGION_SIZE kB
		 *		boundary, so these bits are
		 *		always 0. A write to this
		 *		location is ignored by the
		 *		PCIe core.
		 *	31:*1	Forms bits [31:*] of the of
		 *		the new address of the
		 *		translated region.
		 *
		 *	* is log2(CX_ATU_MIN_REGION_SIZE)
		 */
		bdf = (((bus->number & 0xff) << 24) & 0xff000000) |
				(((devfn & 0xff) << 16) & 0x00ff0000);
	}

	writel_relaxed(0, dev->pcie20 + PCIE20_PLR_IATU_VIEWPORT);
	wmb();

	/* Program Bdf Address */
	writel_relaxed(bdf, dev->pcie20 + PCIE20_PLR_IATU_LTAR);
	wmb();

	/* Write Config Request Type */
	writel_relaxed(cfgtype, dev->pcie20 + PCIE20_PLR_IATU_CTRL1);
	wmb();
}

static inline int msm_pcie_oper_conf(struct pci_bus *bus, u32 devfn, int oper,
				     int where, int size, u32 *val)
{
	uint32_t word_offset, byte_offset, mask;
	uint32_t rd_val, wr_val;
	struct msm_pcie_dev_t *dev = bus_to_mpdev(bus);
	void __iomem *config_base;
	int rc;

	/*
	 * Each RC has 2 Linux PCIe bus numbers associated with it.
	 * Linux PCIe bus numbering starts at 0. Hence, first RC will
	 * take bus 0 & 1, second RC will take bus 2 & 3 and so on.
	 * The lower numbered bus is for the RC and the other is for EP.
	 */
	rc = (bus->number % 2) ? 0 : 1;

	/*
	 * For downstream bus, make sure link is up
	 */
	if ((bus->number > ((MSM_NUM_PCIE * 2) - 1)) || (devfn != 0)) {
		PCIE_DBG("invalid %s - bus %d devfn %d\n",
			 (oper == RD) ? "rd" : "wr", bus->number, devfn);
		*val = ~0;
		return PCIBIOS_DEVICE_NOT_FOUND;
	} else if ((!rc) && (!msm_pcie_is_link_up(dev))) {
		PCIE_DBG("%s fail, link down - bus %d devfn %d\n",
			 (oper == RD) ? "rd" : "wr", bus->number, devfn);
		*val = ~0;
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	msm_pcie_config_cfgtype(bus, devfn);

	word_offset = where & ~0x3;
	byte_offset = where & 0x3;
	mask = (~0 >> (8 * (4 - size))) << (8 * byte_offset);

	config_base = (rc) ? dev->pcie20 : dev->axi_conf;
	rd_val = readl_relaxed(config_base + word_offset);

	if (oper == RD) {
		*val = ((rd_val & mask) >> (8 * byte_offset));

		PCIE_DBG("%d:0x%02x + 0x%04x[%d] -> 0x%08x; rd 0x%08x\n",
			 bus->number, devfn, where, size, *val, rd_val);
	} else {
		wr_val = (rd_val & ~mask) |
				((*val << (8 * byte_offset)) & mask);
		writel_relaxed(wr_val, config_base + word_offset);
		wmb(); /* ensure config data is written to hardware register */

		PCIE_DBG("%d:0x%02x + 0x%04x[%d] <- 0x%08x;"
			 " rd 0x%08x val 0x%08x\n", bus->number,
			 devfn, where, size, wr_val, rd_val, *val);
	}

	return 0;
}

static int msm_pcie_rd_conf(struct pci_bus *bus, u32 devfn, int where,
			    int size, u32 *val)
{
	return msm_pcie_oper_conf(bus, devfn, RD, where, size, val);
}

static int msm_pcie_wr_conf(struct pci_bus *bus, u32 devfn,
			    int where, int size, u32 val)
{
	/*
	 *Attempt to reset secondary bus is causing PCIE core to reset.
	 *Disable secondary bus reset functionality.
	 */
	if ((bus->number == 0) && (where == PCI_BRIDGE_CONTROL) &&
	    (val & PCI_BRIDGE_CTL_BUS_RESET)) {
		pr_info("PCIE secondary bus reset not supported\n");
		val &= ~PCI_BRIDGE_CTL_BUS_RESET;
	}

	return msm_pcie_oper_conf(bus, devfn, WR, where, size, &val);
}

static struct pci_ops msm_pcie_ops = {
	.read = msm_pcie_rd_conf,
	.write = msm_pcie_wr_conf,
};

static int __init msm_pcie_gpio_init(struct msm_pcie_dev_t *msm_pcie_dev)
{
	int rc, i;
	struct msm_pcie_gpio_info_t *info;

	for (i = 0; i < MSM_PCIE_MAX_GPIO; i++) {
		info = &msm_pcie_dev->gpio[i];

		if (!gpio_is_valid(info->num))
			continue;

		rc = gpio_request(info->num, info->name);
		if (rc) {
			pr_err("can't get gpio %s; %d\n", info->name, rc);
			break;
		}

		rc = gpio_direction_output(info->num, 0);
		if (rc) {
			pr_err("can't set gpio direction %s; %d\n",
			       info->name, rc);
			gpio_free(info->num);
			break;
		}
	}

	if (rc)
		while (i--)
			gpio_free(msm_pcie_dev->gpio[i].num);

	return rc;
}

static void msm_pcie_gpio_deinit(struct msm_pcie_dev_t *msm_pcie_dev)
{
	int i;

	for (i = 0; i < MSM_PCIE_MAX_GPIO; i++)
		if (gpio_is_valid(msm_pcie_dev->gpio[i].num))
			gpio_free(msm_pcie_dev->gpio[i].num);
}

static int __init msm_pcie_vreg_init(struct msm_pcie_dev_t *msm_pcie_dev)
{
	int i, rc = 0;
	struct regulator *vreg;
	struct msm_pcie_vreg_info_t *info;
	struct device *dev = &msm_pcie_dev->pdev->dev;

	for (i = 0; i < msm_pcie_dev->vreg_n; i++) {
		info = &msm_pcie_dev->vreg[i];
		vreg = regulator_get(dev, info->name);
		if (!vreg || IS_ERR(vreg)) {
			rc = (PTR_ERR(vreg)) ? PTR_ERR(vreg) : -ENODEV;
			pr_err("can't get %s; %d\n", info->name, rc);
			break;
		}

		if (info->max_v) {
			rc = regulator_set_voltage(vreg,
						   info->min_v, info->max_v);
			if (rc) {
				pr_err("can't set voltage %s; %d\n",
				       info->name, rc);
				regulator_put(vreg);
				break;
			}
		}

		if (info->opt_mode) {
			rc = regulator_set_optimum_mode(vreg, info->opt_mode);
			if (rc < 0) {
				pr_err("can't set mode %s; %d\n",
				       info->name, rc);
				regulator_put(vreg);
				break;
			}
		}

		rc = regulator_enable(vreg);
		if (rc) {
			pr_err("can't enable %s, %d\n", info->name, rc);
			regulator_put(vreg);
			break;
		}
		info->hdl = vreg;
	}

	if (rc)
		while (i--) {
			regulator_disable(msm_pcie_dev->vreg[i].hdl);
			regulator_put(msm_pcie_dev->vreg[i].hdl);
			msm_pcie_dev->vreg[i].hdl = NULL;
		}

	return rc;
}

static void msm_pcie_vreg_deinit(struct msm_pcie_dev_t *msm_pcie_dev)
{
	int i;

	for (i = 0; i < msm_pcie_dev->vreg_n; i++) {
		regulator_disable(msm_pcie_dev->vreg[i].hdl);
		regulator_put(msm_pcie_dev->vreg[i].hdl);
		msm_pcie_dev->vreg[i].hdl = NULL;
	}
}

static int __init msm_pcie_clk_init(struct msm_pcie_dev_t *msm_pcie_dev)
{
	int i, rc = 0;
	struct clk *clk_hdl;
	struct msm_pcie_clk_info_t *info;
	struct device *dev = &msm_pcie_dev->pdev->dev;

	for (i = 0; i < msm_pcie_dev->clk_n; i++) {
		info = &msm_pcie_dev->clk[i];

		clk_hdl = clk_get(dev, info->name);
		if (!clk_hdl || IS_ERR(clk_hdl)) {
			rc = (PTR_ERR(clk_hdl)) ? PTR_ERR(clk_hdl) : -ENODEV;
			pr_err("can't get clk %s; %d\n", info->name, rc);
			break;
		}
		clk_prepare_enable(clk_hdl);
		info->hdl = clk_hdl;
	}

	if (rc)
		while (i--) {
			clk_disable_unprepare(msm_pcie_dev->clk[i].hdl);
			clk_put(msm_pcie_dev->clk[i].hdl);
			msm_pcie_dev->clk[i].hdl = NULL;
		}

	return rc;
}

static void msm_pcie_clk_deinit(struct msm_pcie_dev_t *msm_pcie_dev)
{
	int i;

	for (i = 0; i < msm_pcie_dev->clk_n; i++) {
		clk_disable_unprepare(msm_pcie_dev->clk[i].hdl);
		clk_put(msm_pcie_dev->clk[i].hdl);
		msm_pcie_dev->clk[i].hdl = NULL;
	}
}

static void __init msm_pcie_config_controller(struct msm_pcie_dev_t *dev)
{
	struct resource *axi_conf = dev->res[MSM_PCIE_RES_AXI_CONF].resource;

	/*
	 * program and enable address translation region 0 (device config
	 * address space); region type config;
	 * axi config address range to device config address range
	 */
	writel_relaxed(0, dev->pcie20 + PCIE20_PLR_IATU_VIEWPORT);
	/* ensure that hardware locks the region before programming it */
	wmb();

	writel_relaxed(4, dev->pcie20 + PCIE20_PLR_IATU_CTRL1);
	writel_relaxed(BIT(31), dev->pcie20 + PCIE20_PLR_IATU_CTRL2);
	writel_relaxed(axi_conf->start, dev->pcie20 + PCIE20_PLR_IATU_LBAR);
	writel_relaxed(0, dev->pcie20 + PCIE20_PLR_IATU_UBAR);
	writel_relaxed(axi_conf->end, dev->pcie20 + PCIE20_PLR_IATU_LAR);
	writel_relaxed(MSM_PCIE_DEV_CFG_ADDR,
		       dev->pcie20 + PCIE20_PLR_IATU_LTAR);
	writel_relaxed(0, dev->pcie20 + PCIE20_PLR_IATU_UTAR);
	/* ensure that hardware registers the configuration */
	wmb();

	/*
	 * program and enable address translation region 2 (device resource
	 * address space); region type memory;
	 * axi device bar address range to device bar address range
	 */
	writel_relaxed(2, dev->pcie20 + PCIE20_PLR_IATU_VIEWPORT);
	/* ensure that hardware locks the region before programming it */
	wmb();

	writel_relaxed(0, dev->pcie20 + PCIE20_PLR_IATU_CTRL1);
	writel_relaxed(BIT(31), dev->pcie20 + PCIE20_PLR_IATU_CTRL2);
	writel_relaxed(dev->axi_bar_start, dev->pcie20 + PCIE20_PLR_IATU_LBAR);
	writel_relaxed(0, dev->pcie20 + PCIE20_PLR_IATU_UBAR);
	writel_relaxed(dev->axi_bar_end, dev->pcie20 + PCIE20_PLR_IATU_LAR);
	writel_relaxed(MSM_PCIE_DEV_BAR_ADDR,
		       dev->pcie20 + PCIE20_PLR_IATU_LTAR);
	writel_relaxed(0, dev->pcie20 + PCIE20_PLR_IATU_UTAR);
	/* ensure that hardware registers the configuration */
	wmb();

	/* 256B PCIE buffer setting */
	writel_relaxed(0x1, dev->pcie20 + PCIE20_AXI_MSTR_RESP_COMP_CTRL0);
	writel_relaxed(0x1, dev->pcie20 + PCIE20_AXI_MSTR_RESP_COMP_CTRL1);
	/* ensure that hardware registers the configuration */
	wmb();

}

static int __init msm_pcie_get_resources(struct platform_device *pdev)
{
	int i, rc = 0;
	struct resource *res;
	struct msm_pcie_res_info_t *info;
	struct msm_pcie_dev_t *dev = pdev_id_to_mpdev(pdev->id);

	for (i = 0; i < MSM_PCIE_MAX_RES; i++) {
		info = &dev->res[i];

		if (i < MSM_PCIE_MAX_PLATFORM_RES) {
			res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
							   info->name);
		} else {
			res = dev->res[i].resource;
			if (request_resource(&iomem_resource, res))
				res = NULL;
		}

		if (!res) {
			pr_err("can't get %s resource\n", info->name);
			rc = -ENOMEM;
			break;
		}

		info->base = ioremap(res->start, resource_size(res));
		if (!info->base) {
			pr_err("can't remap %s\n", info->name);
			rc = -ENOMEM;
			break;
		}

		info->resource = res;
	}

	if (rc) {
		while (i--) {
			iounmap(dev->res[i].base);
			dev->res[i].base = NULL;
			if (i >= MSM_PCIE_MAX_PLATFORM_RES)
				release_resource(dev->res[i].resource);
		}
	} else {
		dev->parf = dev->res[MSM_PCIE_RES_PARF].base;
		dev->elbi = dev->res[MSM_PCIE_RES_ELBI].base;
		dev->pcie20 = dev->res[MSM_PCIE_RES_PCIE20].base;
		dev->axi_conf = dev->res[MSM_PCIE_RES_AXI_CONF].base;
	}

	return rc;
}

static void msm_pcie_release_resources(struct msm_pcie_dev_t *msm_pcie_dev)
{
	int i;

	for (i = 0; i < MSM_PCIE_MAX_RES; i++) {
		iounmap(msm_pcie_dev->res[i].base);
		msm_pcie_dev->res[i].base = NULL;
		if (i >= MSM_PCIE_MAX_PLATFORM_RES)
			release_resource(msm_pcie_dev->res[i].resource);
	}

	msm_pcie_dev->parf = NULL;
	msm_pcie_dev->elbi = NULL;
	msm_pcie_dev->pcie20 = NULL;
	msm_pcie_dev->axi_conf = NULL;
}

static void msm_pcie_parf_reset(struct msm_pcie_dev_t *dev, int domain, int assert)

{
	if (assert)
		msm_pcie_write_mask(dev->reset_reg, 0, domain);
	else
		msm_pcie_write_mask(dev->reset_reg, domain, 0);
}

static void msm_pcie_port_enable(struct msm_pcie_dev_t *msm_pcie_dev)
{
        msm_pcie_port_en_t *port_en = msm_pcie_dev->port_en;

	if (port_en->reg)
		writel_relaxed(port_en->bits, port_en->reg);
}

static int __init msm_pcie_setup(int nr, struct pci_sys_data *sys)
{
	int rc;
	struct msm_pcie_dev_t *dev;
        struct msm_pcie_platform *pcie_pdata;
	uint32_t val;

	PCIE_DBG("bus %d\n", nr);
	if (nr < 0 && nr >= MSM_NUM_PCIE)
		return 0;

	sys->private_data =
	dev = pdev_id_to_mpdev(nr);
        pcie_pdata = (struct msm_pcie_platform *)dev->pdev->dev.platform_data;
	/*
	 * specify linux PCI framework to allocate device memory (BARs)
	 * from msm_pcie_dev->dev_mem_res resource.
	 */
	sys->mem_offset = 0;
	pci_add_resource(&sys->resources, &dev->dev_mem_res);

	dev->bus = sys->busnr; /* Save the RC bus number */

	/* assert PCIe reset link to keep EP in reset */
	if (gpio_is_valid(dev->gpio[MSM_PCIE_GPIO_RST_N].num)) {
		gpio_set_value_cansleep(dev->gpio[MSM_PCIE_GPIO_RST_N].num,
				dev->gpio[MSM_PCIE_GPIO_RST_N].on);
	}
	/* enable power */
	rc = msm_pcie_vreg_init(dev);
	if (rc)
		goto out;


	/* assert PCIe PARF reset while powering the core */
	msm_pcie_parf_reset(dev, BIT(2), 1);


	/* enable clocks */
	rc = msm_pcie_clk_init(dev);
	if (rc)
		goto clk_fail;

	if (gpio_is_valid(dev->gpio[MSM_PCIE_GPIO_PWR_EN].num)) {
		/* enable pcie power; wait 3ms for clock to stabilize */
		gpio_set_value_cansleep(dev->gpio[MSM_PCIE_GPIO_PWR_EN].num,
					dev->gpio[MSM_PCIE_GPIO_PWR_EN].on);
		usleep(3000);
	}

	/*
	 * de-assert PCIe PARF reset;
	 * wait 1us before accessing PARF registers
	 */
	msm_pcie_parf_reset(dev, BIT(2), 0);
	udelay(1);

	/* enable PCIe clocks and resets */
	msm_pcie_write_mask(dev->parf + PCIE20_PARF_PHY_CTRL, BIT(0), 0);

	/* Set Tx Termination Offset */
	val = readl_relaxed(dev->parf + PCIE20_PARF_PHY_CTRL) |
		PCIE20_PARF_PHY_CTRL_PHY_TX0_TERM_OFFST(pcie_pdata->term_offset);

	writel_relaxed(val, dev->parf + PCIE20_PARF_PHY_CTRL);

	/* PARF programming */
	writel_relaxed(PCIE20_PARF_PCS_DEEMPH_TX_DEEMPH_GEN1(0x18) |
			PCIE20_PARF_PCS_DEEMPH_TX_DEEMPH_GEN2_3_5DB(0x18) |
			PCIE20_PARF_PCS_DEEMPH_TX_DEEMPH_GEN2_6DB(0x22),
			dev->parf + PCIE20_PARF_PCS_DEEMPH);
	writel_relaxed(PCIE20_PARF_PCS_SWING_TX_SWING_FULL(0x78) |
			PCIE20_PARF_PCS_SWING_TX_SWING_LOW(0x78),
			dev->parf + PCIE20_PARF_PCS_SWING);
	writel_relaxed((4 << 24), dev->parf + PCIE20_PARF_CONFIG_BITS);
	/* ensure that hardware registers the PARF configuration */
	wmb();

	/* enable reference clock */
	msm_pcie_write_mask(dev->parf + PCIE20_PARF_PHY_REFCLK, BIT(12), BIT(16));

	/* enable access to PCIe slave port on system fabric */
	msm_pcie_port_enable(dev);
	/* ensure that access is enabled before proceeding */
	wmb();

	/* de-assert PICe PHY, Core, POR and AXI clk domain resets */
	msm_pcie_parf_reset(dev, BIT(5), 0);
	msm_pcie_parf_reset(dev, BIT(4), 0);
	msm_pcie_parf_reset(dev, BIT(3), 0);
	msm_pcie_parf_reset(dev, BIT(0), 0);

	/* wait 150ms for clock acquisition */
	udelay(150);

	/* de-assert PCIe reset link to bring EP out of reset */
	if (gpio_is_valid(dev->gpio[MSM_PCIE_GPIO_RST_N].num)) {
		gpio_set_value_cansleep(dev->gpio[MSM_PCIE_GPIO_RST_N].num,
					!dev->gpio[MSM_PCIE_GPIO_RST_N].on);
	}

	/* enable link training */
	msm_pcie_write_mask(dev->elbi + PCIE20_ELBI_SYS_CTRL, 0, BIT(0));

	/* poll for link to come up for upto 100ms */
	rc = readl_poll_timeout(
			(dev->pcie20 + PCIE20_CAP_LINKCTRLSTATUS),
			val, (val & BIT(29)), 10000, 100000);
	if (rc) {
		pr_err("link initialization failed\n");
		dev->bus = -1;
		goto link_fail;
	} else
		pr_info("link initialized\n");

	msm_pcie_config_controller(dev);
	rc = msm_pcie_irq_init(dev);
	if (!rc)
		goto out;

link_fail:
        // crypto engine depends on some of them
	//msm_pcie_clk_deinit(dev);
clk_fail:
	msm_pcie_vreg_deinit(dev);
out:
	return (rc) ? 0 : 1;
}

static struct pci_bus __init *msm_pcie_scan_bus(int nr,
						struct pci_sys_data *sys)
{
	struct pci_bus *bus = NULL;

	PCIE_DBG("bus %d\n", nr);
	if (nr >= 0 && nr < (MSM_NUM_PCIE * 2))
		bus = pci_scan_root_bus(NULL, sys->busnr, &msm_pcie_ops, sys,
					&sys->resources);

	return bus;
}

static int __init msm_pcie_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	struct msm_pcie_dev_t *msm_pcie_dev = bus_to_mpdev(dev->bus);

	PCIE_DBG("slot %d pin %d\n", slot, pin);
	return (pin <= 4) ? (msm_pcie_dev->inta + pin - 1) : 0;
}

static struct hw_pci msm_pci __initdata = {
	.swizzle = pci_std_swizzle,
	.setup = msm_pcie_setup,
	.scan = msm_pcie_scan_bus,
	.map_irq = msm_pcie_map_irq,
};

int __init msm_pcibios_init(void)
{
	/* kick start ARM PCI configuration framework */
	pci_common_init(&msm_pci);
	return 0;
}

static int __init msm_pcie_probe(struct platform_device *pdev)
{
	const struct msm_pcie_platform *pdata;
	struct resource *res;
	int rc;
	struct msm_pcie_dev_t *msm_pcie_dev = pdev_id_to_mpdev(pdev->id);

	PCIE_DBG("\n");

	msm_pcie_dev->pdev = pdev;
	pdata = pdev->dev.platform_data;
	msm_pcie_dev->gpio = pdata->gpio;
	msm_pcie_dev->wake_n = pdata->wake_n;
	msm_pcie_dev->vreg = pdata->vreg;
	msm_pcie_dev->clk = pdata->clk;
	msm_pcie_dev->res = pdata->res;
	msm_pcie_dev->inta = pdata->inta;
	msm_pcie_dev->msi_irq = pdata->msi_irq;
	msm_pcie_dev->reset_reg = pdata->reset_reg;
	msm_pcie_dev->vreg_n = pdata->vreg_n;
	msm_pcie_dev->clk_n = pdata->clk_n;
	msm_pcie_dev->port_en = pdata->port_en;
	/* device memory resource */
	res = &msm_pcie_dev->dev_mem_res;
	res->name = "pcie_dev_mem";
	res->start = MSM_PCIE_DEV_BAR_ADDR;
	res->end = res->start + pdata->axi_size - 1;
	res->flags = IORESOURCE_MEM;

	/* axi address space = axi bar space + axi config space */
	msm_pcie_dev->axi_bar_start = pdata->axi_addr;
	msm_pcie_dev->axi_bar_end = pdata->axi_addr + pdata->axi_size -
					PCIE_AXI_CONF_SIZE - 1;

	/* axi config space resource */
	res = kzalloc(sizeof(*res), GFP_KERNEL);
	if (!res) {
		pr_err("can't allocate memory\n");
		return -ENOMEM;
	}

	msm_pcie_dev->res[MSM_PCIE_RES_AXI_CONF].resource = res;
	res->name = msm_pcie_dev->res[MSM_PCIE_RES_AXI_CONF].name;
	res->start = msm_pcie_dev->axi_bar_end + 1;
	res->end = res->start + PCIE_AXI_CONF_SIZE - 1;
	res->flags = IORESOURCE_MEM;

	rc = msm_pcie_get_resources(msm_pcie_dev->pdev);
	if (rc)
		return rc;

	rc = msm_pcie_gpio_init(msm_pcie_dev);
	if (rc) {
		msm_pcie_release_resources(msm_pcie_dev);
		return rc;
	}
	msm_pci.nr_controllers ++;
	return 0;
}

static int __exit msm_pcie_remove(struct platform_device *pdev)
{
	struct msm_pcie_dev_t *msm_pcie_dev;

	PCIE_DBG("\n");
	msm_pcie_dev = pdev_id_to_mpdev(pdev->id);

	msm_pcie_irq_deinit(msm_pcie_dev);
	msm_pcie_vreg_deinit(msm_pcie_dev);
	msm_pcie_clk_deinit(msm_pcie_dev);
	msm_pcie_gpio_deinit(msm_pcie_dev);
	msm_pcie_release_resources(msm_pcie_dev);

	msm_pcie_dev->pdev = NULL;
	msm_pcie_dev->vreg = NULL;
	msm_pcie_dev->clk = NULL;
	msm_pcie_dev->gpio = NULL;
	return 0;
}

static struct platform_driver msm_pcie_driver = {
	.remove = __exit_p(msm_pcie_remove),
	.driver = {
		.name = "msm_pcie",
		.owner = THIS_MODULE,
	},
};

static int __init msm_pcie_init(void)
{
	if (rb_mach != RB_MACH_IPQ806X) {
		return -1;
	}
	PCIE_DBG("\n");
	pcibios_min_mem = 0x10000000;
	platform_driver_probe(&msm_pcie_driver, msm_pcie_probe);
	return msm_pcibios_init();
}
subsys_initcall(msm_pcie_init);

/* RC do not represent the right class; set it to PCI_CLASS_BRIDGE_PCI */
static void __devinit msm_pcie_fixup_early(struct pci_dev *dev)
{
	PCIE_DBG("hdr_type %d\n", dev->hdr_type);
	if (dev->hdr_type == 1)
		dev->class = (dev->class & 0xff) | (PCI_CLASS_BRIDGE_PCI << 8);
}
DECLARE_PCI_FIXUP_EARLY(PCIE_VENDOR_ID_RCP, PCIE_DEVICE_ID_RCP,
			msm_pcie_fixup_early);

/* enable wake_n interrupt during suspend */
static void msm_pcie_fixup_suspend(struct pci_dev *dev)
{
	struct msm_pcie_dev_t *msm_pcie_dev = bus_to_mpdev(dev->bus);

	PCIE_DBG("enabling wake_n\n");
	if (dev->pcie_type == PCI_EXP_TYPE_ROOT_PORT)
		enable_irq(msm_pcie_dev->wake_n);
}
DECLARE_PCI_FIXUP_SUSPEND(PCIE_VENDOR_ID_RCP, PCIE_DEVICE_ID_RCP,
			  msm_pcie_fixup_suspend);

/* disable wake_n interrupt when system is not in suspend */
static void msm_pcie_fixup_resume(struct pci_dev *dev)
{
	struct msm_pcie_dev_t *msm_pcie_dev = bus_to_mpdev(dev->bus);

	PCIE_DBG("disabling wake_n\n");
	if (dev->pcie_type == PCI_EXP_TYPE_ROOT_PORT)
		disable_irq(msm_pcie_dev->wake_n);
}
DECLARE_PCI_FIXUP_RESUME(PCIE_VENDOR_ID_RCP, PCIE_DEVICE_ID_RCP,
			 msm_pcie_fixup_resume);

/*
 * actual physical (BAR) address of the device resources starts from
 * MSM_PCIE_DEV_BAR_ADDR; the system axi address for the device resources starts
 * from msm_pcie_dev->axi_bar_start; correct the device resource structure here;
 * address translation unit handles the required translations
 */
static void __devinit msm_pcie_fixup_final(struct pci_dev *dev)
{
	int i;
	struct resource *res;
	struct msm_pcie_dev_t *msm_pcie_dev;

	PCIE_DBG("vendor 0x%x 0x%x\n", dev->vendor, dev->device);
	msm_pcie_dev = bus_to_mpdev(dev->bus);

	for (i = 0; i < PCI_NUM_RESOURCES; i++) {
		res = &dev->resource[i];
		if (res->start & MSM_PCIE_DEV_BAR_ADDR) {
			res->start -= MSM_PCIE_DEV_BAR_ADDR;
			res->start += msm_pcie_dev->axi_bar_start;
			res->end -= MSM_PCIE_DEV_BAR_ADDR;
			res->end += msm_pcie_dev->axi_bar_start;

			/* If Root Port, request for the changed resource */
			if ((dev->vendor == PCIE_VENDOR_ID_RCP) &&
			    (dev->device == PCIE_DEVICE_ID_RCP)) {
				insert_resource(&iomem_resource, res);
			}
		}
	}
}
DECLARE_PCI_FIXUP_FINAL(PCI_ANY_ID, PCI_ANY_ID, msm_pcie_fixup_final);
