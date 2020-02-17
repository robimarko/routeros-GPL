/* Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
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
#include <linux/of_gpio.h>
#include <asm/mach/pci.h>
#include "qcom_pcie.h"

/* Root Complex Port vendor/device IDs */
#define PCIE_VENDOR_ID_RCP             0x17cb
#ifdef CONFIG_ARCH_MDM9630
#define PCIE_DEVICE_ID_RCP             0x300
#else
#define PCIE_DEVICE_ID_RCP             0x0101
#endif

#define PCIE20_PARF_SYS_CTRL           0x00
#define PCIE20_PARF_PM_CTRL            0x20
#define PCIE20_PARF_PM_STTS            0x24
#define PCIE20_PARF_PCS_DEEMPH         0x34
#define PCIE20_PARF_PCS_SWING          0x38
#define PCIE20_PARF_PHY_CTRL           0x40
#define PCIE20_PARF_PHY_REFCLK         0x4C
#define PCIE20_PARF_CONFIG_BITS        0x50
#define PCIE20_PARF_DBI_BASE_ADDR      0x168
#define PCIE20_PARF_AXI_MSTR_WR_ADDR_HALT   0x178
#define PCIE20_PARF_Q2A_FLUSH          0x1AC
#define PCIE20_PARF_LTSSM              0x1B0

#define PCIE20_ELBI_VERSION            0x00
#define PCIE20_ELBI_SYS_CTRL           0x04
#define PCIE20_ELBI_SYS_STTS	       0x08

#define PCIE20_CAP                     0x70
#define PCIE20_CAP_LINKCTRLSTATUS      (PCIE20_CAP + 0x10)
#define PCIE20_CAP_LINK_CAPABILITIES   (PCIE20_CAP + 0xC)
#define PCIE20_CAP_LINK_1		(PCIE20_CAP + 0x14)

#define PCIE20_COMMAND_STATUS          0x04
#define PCIE20_BUSNUMBERS              0x18
#define PCIE20_MEMORY_BASE_LIMIT       0x20
#define PCIE20_L1SUB_CONTROL1          0x158
#define PCIE20_EP_L1SUB_CTL1_OFFSET    0x30
#define PCIE20_DEVICE_CONTROL2_STATUS2 0x98

#define PCIE20_ACK_F_ASPM_CTRL_REG     0x70C
#define PCIE20_ACK_N_FTS               0xff00
#define PCIE20_GEN2_CTRL_REG           0x80C
#define PCIE20_MISC_CONTROL_1_REG      0x8BC

#define PCIE20_PLR_IATU_VIEWPORT       0x900
#define PCIE20_PLR_IATU_CTRL1          0x904
#define PCIE20_PLR_IATU_CTRL2          0x908
#define PCIE20_PLR_IATU_LBAR           0x90C
#define PCIE20_PLR_IATU_UBAR           0x910
#define PCIE20_PLR_IATU_LAR            0x914
#define PCIE20_PLR_IATU_LTAR           0x918
#define PCIE20_PLR_IATU_UTAR           0x91c

#define RD 0
#define WR 1

/* Timing Delays */
#define PERST_PROPAGATION_DELAY_US_MIN        1000
#define PERST_PROPAGATION_DELAY_US_MAX        1005
#define REFCLK_STABILIZATION_DELAY_US_MIN     1000
#define REFCLK_STABILIZATION_DELAY_US_MAX     1500
#define LINK_RETRY_TIMEOUT_US_MIN             20000
#define LINK_RETRY_TIMEOUT_US_MAX             25000
#define LINK_UP_TIMEOUT_US_MIN                5000
#define LINK_UP_TIMEOUT_US_MAX                5100
#define LINK_UP_CHECK_MAX_COUNT               20
#define PHY_STABILIZATION_DELAY_US_MIN        995
#define PHY_STABILIZATION_DELAY_US_MAX        1005
#define REQ_EXIT_L1_DELAY_US                  1

#define PHY_READY_TIMEOUT_COUNT               10
#define XMLH_LINK_UP                          0x400
#define MAX_LINK_RETRIES 5
#define MAX_BUS_NUM 4
#define MAX_PROP_SIZE 32
#define MAX_RC_NAME_LEN 15

#define CMD_BME_VAL					0x4
#define DBI_RO_WR_EN					1
#define PCIE_CAP_CPL_TIMEOUT_DISABLE			0x10
#define LTSSM_EN					(1 << 8)
#define PCIE_CAP_ACTIVE_STATE_LINK_PM_SUPPORT_MASK	0xc00
#define PCIE_CAP_LINK1_VAL				0x2fd7f

/* Config Space Offsets */
#define BDF_OFFSET(bus, device, function) \
	((bus << 24) | (device << 15) | (function << 8))

/**
 *  PCIe driver state
 */
struct pcie_drv_sta {
	u32 rc_num;
	u32 rc_expected;
	u32 current_rc;
	struct mutex drv_lock;
} pcie_drv;

static struct msm_pcie_dev_t msm_pcie_dev;

static const struct msm_pcie_res_info_t msm_pcie_res_info[MSM_PCIE_MAX_RES] = {
	{"parf",	0, 0},
	{"phy",     0, 0},
	{"dm_core",	0, 0},
	{"elbi",	0, 0},
	{"conf",	0, 0},
	{"io",		0, 0},
	{"bars",	0, 0}
};

static const struct msm_pcie_irq_info_t msm_pcie_irq_info[MSM_PCIE_MAX_IRQ] = {
	{"int_msi",	0},
	{"int_a",	0},
	{"int_b",	0},
	{"int_c",	0},
	{"int_d",	0},
	{"int_pls_pme",		0},
	{"int_pme_legacy",	0},
	{"int_pls_err",		0},
	{"int_aer_legacy",	0},
	{"int_pls_link_up",	0},
	{"int_pls_link_down",	0},
	{"int_bridge_flush_n",	0},
	{"int_wake",	0}
};

#define CLK_REG_BASE 0x01800000
#define TLMM_BCR				0x5000
#define TLMM_AHB_CBCR				0x5004
#define PCIE_AXI_M_CBCR				0x1D004
#define PCIE_AXI_S_CBCR				0x1D008
#define PCIE_AHB_CBCR				0x1D00C
#define PCIE_RST_CTRL				0x1D010

#define PCIE_RST_AXI_M		BIT(0)
#define PCIE_RST_AXI_S		BIT(1)
#define PCIE_RST_PIPE		BIT(2)
#define PCIE_RST_AXI_M_VMIDMT	BIT(3)
#define PCIE_RST_AXI_S_XPU	BIT(4)
#define PCIE_RST_PARF_XPU	BIT(5)
#define PCIE_RST_PHY		BIT(6)
#define PCIE_RST_AXI_M_STICKY	BIT(7)
#define PCIE_RST_PIPE_STICKY	BIT(8)
#define PCIE_RST_PWR		BIT(9)
#define PCIE_RST_AHB		BIT(10)
#define PCIE_RST_PHY_AHB	BIT(11)

#define TLMM_BASE 0x01000000
#define TLMM_GPIO_CFG(n) (TLMM_BASE + (n) * 0x1000 + 0x0)
#define TLMM_GPIO_IN_OUT(n) (TLMM_BASE + (n) * 0x1000 + 0x4)

static unsigned ioremap_reg(unsigned long addr, unsigned clear, unsigned set) {
    unsigned *reg = ioremap(addr, 0x4);
    unsigned old_val = *reg;
    unsigned val = old_val;
//    printk("%08x = %08x\n", (unsigned)addr, val);
    val &= ~clear;
    val |= set;
    if (val != old_val) {
	*reg = val;
	val = *reg;
//	printk("%08x = %08x\n", (unsigned)addr, val);
    }
    iounmap(reg);
    return val;
}

static void msm_pcie_dev_reset(struct msm_pcie_dev_t *dev, int assert) {
    unsigned val;
    if (!dev->of_defined_device) {
	if (assert)
	    ioremap_reg(TLMM_GPIO_IN_OUT(dev->gpio_rst), 0, BIT(1));
	else
	    ioremap_reg(TLMM_GPIO_IN_OUT(dev->gpio_rst), BIT(1), 0);
    } else {
	struct msm_pci_devices_t *curr = dev->children;
	while (curr->name || curr->gpio) {
	    val = !!assert == !!curr->polarity;
	    gpio_set_value(curr->gpio, val);
	    curr++;
	}
    }
}

static bool msm_pcie_confirm_linkup(struct msm_pcie_dev_t *dev,
						bool check_sw_stts,
						bool check_ep)
{
	u32 val;

	if (check_sw_stts && (dev->link_status != MSM_PCIE_LINK_ENABLED)) {
		PCIE_DBG(dev, "PCIe: The link of RC %d is not enabled.\n",
			dev->rc_idx);
		return false;
	}

	if (!(readl_relaxed(dev->dm_core + 0x80) & BIT(29))) {
		PCIE_DBG(dev, "PCIe: The link of RC %d is not up.\n",
			dev->rc_idx);
		return false;
	}

	val = readl_relaxed(dev->dm_core);
	PCIE_DBG(dev, "PCIe: device ID and vender ID of RC %d are 0x%x.\n",
		dev->rc_idx, val);
	if (val == PCIE_LINK_DOWN) {
		PCIE_ERR(dev,
			"PCIe: The link of RC %d is not really up; device ID and vender ID of RC %d are 0x%x.\n",
			dev->rc_idx, dev->rc_idx, val);
		return false;
	}

	if (check_ep) {
		val = readl_relaxed(dev->conf);
		PCIE_DBG(dev,
			"PCIe: device ID and vender ID of EP of RC %d are 0x%x.\n",
			dev->rc_idx, val);
		if (val == PCIE_LINK_DOWN) {
			PCIE_ERR(dev,
				"PCIe: The link of RC %d is not really up; device ID and vender ID of EP of RC %d are 0x%x.\n",
				dev->rc_idx, dev->rc_idx, val);
			return false;
		}
	}

	return true;
}

static void msm_pcie_write_mask(void __iomem *addr,
				uint32_t clear_mask, uint32_t set_mask)
{
	uint32_t val;

	val = (readl_relaxed(addr) & ~clear_mask) | set_mask;
	writel_relaxed(val, addr);
	wmb();  /* ensure data is written to hardware register */
}

static int msm_pcie_is_link_up(struct msm_pcie_dev_t *dev)
{
	return readl_relaxed(dev->dm_core +
			PCIE20_CAP_LINKCTRLSTATUS) & BIT(29);
}

static void atu_outbound(struct msm_pcie_dev_t *dev, u32 type, u32 cpu_addr, u32 pci_addr, u32 size) {
	writel(0, 		dev->dm_core + PCIE20_PLR_IATU_VIEWPORT);
	writel(type, 		dev->dm_core + PCIE20_PLR_IATU_CTRL1);
	writel(cpu_addr, 	dev->dm_core + PCIE20_PLR_IATU_LBAR);
	writel(0, 		dev->dm_core + PCIE20_PLR_IATU_UBAR);
	writel(cpu_addr + size - 1, dev->dm_core + PCIE20_PLR_IATU_LAR);
	writel(pci_addr, 	dev->dm_core + PCIE20_PLR_IATU_LTAR);
	writel(0, 		dev->dm_core + PCIE20_PLR_IATU_UTAR);
	writel(BIT(31), 	dev->dm_core + PCIE20_PLR_IATU_CTRL2);
}

static inline int msm_pcie_oper_conf(struct pci_bus *bus, u32 devfn, int oper,
				     int where, int size, u32 *val)
{
	uint32_t word_offset, byte_offset, mask;
	uint32_t rd_val, wr_val;
	struct msm_pcie_dev_t *dev = &msm_pcie_dev;
	void __iomem *config_base;
	bool rc = false;
	u32 rc_idx;
	int rv = 0;
	u32 busdev, type = 5;


	if (!dev) {
		pr_err("PCIe: No device found for this bus.\n");
		*val = ~0;
		rv = PCIBIOS_DEVICE_NOT_FOUND;
		goto out;
	}

	if (!bus->number)
		rc = true;
	rc_idx = dev->rc_idx;

	if ((bus->number > MAX_BUS_NUM) ||
		((devfn != 0) && (bus->number <= 1))) {
		PCIE_DBG3(dev, "RC%d invalid %s - bus %d devfn %d\n", rc_idx,
			 (oper == RD) ? "rd" : "wr", bus->number, devfn);
		*val = ~0;
		rv = PCIBIOS_DEVICE_NOT_FOUND;
		goto out;
	}

	spin_lock_irqsave(&dev->cfg_lock, dev->irqsave_flags);

	if (!dev->cfg_access) {
		PCIE_DBG3(dev,
			"Access denied for RC%d %d:0x%02x + 0x%04x[%d]\n",
			rc_idx, bus->number, devfn, where, size);
		*val = ~0;
		rv = PCIBIOS_DEVICE_NOT_FOUND;
		goto unlock;
	}

	if (dev->link_status != MSM_PCIE_LINK_ENABLED) {
		PCIE_DBG3(dev,
			"Access to RC%d %d:0x%02x + 0x%04x[%d] is denied because link is down\n",
			rc_idx, bus->number, devfn, where, size);
		*val = ~0;
		rv = PCIBIOS_DEVICE_NOT_FOUND;
		goto unlock;
	}

	/* check if the link is up for endpoint */
	if (!rc && !msm_pcie_is_link_up(dev)) {
		PCIE_ERR(dev,
			"PCIe: RC%d %s fail, link down - bus %d devfn %d\n",
				rc_idx, (oper == RD) ? "rd" : "wr",
				bus->number, devfn);
			*val = ~0;
			rv = PCIBIOS_DEVICE_NOT_FOUND;
			goto unlock;
	}

	word_offset = where & ~0x3;
	byte_offset = where & 0x3;
	mask = (~0 >> (8 * (4 - size))) << (8 * byte_offset);

	config_base = rc ? dev->dm_core : dev->conf;

	busdev = (((bus->number) & 0xff) << 24) |
		((PCI_SLOT(devfn) & 0x1f) << 19) |
		((PCI_FUNC(devfn) & 0x7) << 16);

	if (bus->number == 1) {
		type = 4;
	}

	if (!rc) {
		atu_outbound(dev, type, dev->res[MSM_PCIE_RES_CONF].resource->start, busdev, 
			dev->res[MSM_PCIE_RES_CONF].resource->end - 
				dev->res[MSM_PCIE_RES_CONF].resource->start);
	}

	rd_val = readl_relaxed(config_base + word_offset);

	if (oper == RD) {
		*val = ((rd_val & mask) >> (8 * byte_offset));
		PCIE_DBG3(dev, 
			"RC%d %d:0x%02x + 0x%04x[%d] -> 0x%08x; rd 0x%08x\n",
			rc_idx, bus->number, devfn, where, size, *val, rd_val);
	} else {

		wr_val = (rd_val & ~mask) |
				((*val << (8 * byte_offset)) & mask);

		writel_relaxed(wr_val, config_base + word_offset);
		wmb(); /* ensure config data is written to hardware register */
		rd_val = readl_relaxed(dev->elbi);

		if (rd_val == PCIE_LINK_DOWN) {
			PCIE_ERR(dev,
				"Read of RC%d %d:0x%02x + 0x%04x[%d] is all FFs\n",
				rc_idx, bus->number, devfn, where, size);
		} else if (dev->shadow_en) {
			if (rc)
				dev->rc_shadow[word_offset / 4] = wr_val;
			else
				dev->ep_shadow[word_offset / 4] = wr_val;
		}

		PCIE_DBG3(dev,
			"RC%d %d:0x%02x + 0x%04x[%d] <- 0x%08x; rd 0x%08x val 0x%08x\n",
			rc_idx, bus->number, devfn, where, size,
			wr_val, rd_val, *val);
	}

unlock:
	spin_unlock_irqrestore(&dev->cfg_lock, dev->irqsave_flags);
out:
	return rv;
}

static int msm_pcie_rd_conf(struct pci_bus *bus, u32 devfn, int where,
			    int size, u32 *val)
{
	int ret = msm_pcie_oper_conf(bus, devfn, RD, where, size, val);

	if ((bus->number == 0) && (where == PCI_CLASS_REVISION)) {
		*val = (*val & 0xff) | (PCI_CLASS_BRIDGE_PCI << 16);
		pr_debug("change class for RC:0x%x\n", *val);
		ret = PCIBIOS_SUCCESSFUL;
	}

	return ret;
}

static int msm_pcie_wr_conf(struct pci_bus *bus, u32 devfn,
			    int where, int size, u32 val)
{
	return msm_pcie_oper_conf(bus, devfn, WR, where, size, &val);
}

static struct pci_ops msm_pcie_ops = {
	.read = msm_pcie_rd_conf,
	.write = msm_pcie_wr_conf,
};

static void msm_pcie_clk_init(struct msm_pcie_dev_t *dev)
{
	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);
	ioremap_reg(CLK_REG_BASE + TLMM_AHB_CBCR, 0, BIT(0));

	ioremap_reg(CLK_REG_BASE + PCIE_AHB_CBCR, 0, BIT(0));
	ioremap_reg(CLK_REG_BASE + PCIE_AXI_M_CBCR, 0, BIT(0));
	ioremap_reg(CLK_REG_BASE + PCIE_AXI_S_CBCR, 0, BIT(0));
}

static void msm_pcie_clk_deinit(struct msm_pcie_dev_t *dev)
{
	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);
	ioremap_reg(CLK_REG_BASE + PCIE_AHB_CBCR, BIT(0), 0);
	ioremap_reg(CLK_REG_BASE + PCIE_AXI_M_CBCR, BIT(0), 0);
	ioremap_reg(CLK_REG_BASE + PCIE_AXI_S_CBCR, BIT(0), 0);
}

static void msm_pcie_controller_reset(struct msm_pcie_dev_t *dev)
{
	ioremap_reg(CLK_REG_BASE + PCIE_RST_CTRL, 0, PCIE_RST_AXI_M);
	ioremap_reg(CLK_REG_BASE + PCIE_RST_CTRL, 0, PCIE_RST_AXI_S);
	usleep_range(10000, 12000); /* wait 12ms */

	ioremap_reg(CLK_REG_BASE + PCIE_RST_CTRL, 0, PCIE_RST_PIPE);
	ioremap_reg(CLK_REG_BASE + PCIE_RST_CTRL, 0, PCIE_RST_PIPE_STICKY);
	ioremap_reg(CLK_REG_BASE + PCIE_RST_CTRL, 0, PCIE_RST_PHY);
	ioremap_reg(CLK_REG_BASE + PCIE_RST_CTRL, 0, PCIE_RST_PHY_AHB);
	usleep_range(10000, 12000); /* wait 12ms */

	ioremap_reg(CLK_REG_BASE + PCIE_RST_CTRL, 0, PCIE_RST_AXI_M_STICKY);
	ioremap_reg(CLK_REG_BASE + PCIE_RST_CTRL, 0, PCIE_RST_PWR);
	ioremap_reg(CLK_REG_BASE + PCIE_RST_CTRL, 0, PCIE_RST_AHB);
	usleep_range(10000, 12000); /* wait 12ms */

	ioremap_reg(CLK_REG_BASE + PCIE_RST_CTRL, PCIE_RST_PHY_AHB, 0);
	ioremap_reg(CLK_REG_BASE + PCIE_RST_CTRL, PCIE_RST_PHY, 0);
	ioremap_reg(CLK_REG_BASE + PCIE_RST_CTRL, PCIE_RST_PIPE, 0);
	ioremap_reg(CLK_REG_BASE + PCIE_RST_CTRL, PCIE_RST_PIPE_STICKY, 0);
	usleep_range(10000, 12000); /* wait 12ms */

	ioremap_reg(CLK_REG_BASE + PCIE_RST_CTRL, PCIE_RST_AXI_M, 0);
	ioremap_reg(CLK_REG_BASE + PCIE_RST_CTRL, PCIE_RST_AXI_M_STICKY, 0);
	ioremap_reg(CLK_REG_BASE + PCIE_RST_CTRL, PCIE_RST_AXI_S, 0);
	ioremap_reg(CLK_REG_BASE + PCIE_RST_CTRL, PCIE_RST_PWR, 0);
	ioremap_reg(CLK_REG_BASE + PCIE_RST_CTRL, PCIE_RST_AHB, 0);
	usleep_range(10000, 12000); /* wait 12ms */
	wmb(); /* ensure data is written to hw register */
}

static void msm_pcie_config_controller(struct msm_pcie_dev_t *dev)
{
	struct resource *axi_conf = dev->res[MSM_PCIE_RES_CONF].resource;
	u32 dev_conf, upper, lower, limit;

	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);

	if (IS_ENABLED(CONFIG_ARM_LPAE)) {
		lower = PCIE_LOWER_ADDR(axi_conf->start);
		upper = PCIE_UPPER_ADDR(axi_conf->start);
		limit = PCIE_LOWER_ADDR(axi_conf->end);
	} else {
		lower = axi_conf->start;
		upper = 0;
		limit = axi_conf->end;
	}

	dev_conf = BDF_OFFSET(1, 0, 0);

	if (dev->shadow_en) {
		dev->rc_shadow[PCIE20_PLR_IATU_VIEWPORT / 4] = 0;
		dev->rc_shadow[PCIE20_PLR_IATU_CTRL1 / 4] = 4;
		dev->rc_shadow[PCIE20_PLR_IATU_LBAR / 4] = lower;
		dev->rc_shadow[PCIE20_PLR_IATU_UBAR / 4] = upper;
		dev->rc_shadow[PCIE20_PLR_IATU_LAR / 4] = limit;
		dev->rc_shadow[PCIE20_PLR_IATU_LTAR / 4] = dev_conf;
		dev->rc_shadow[PCIE20_PLR_IATU_UTAR / 4] = 0;
		dev->rc_shadow[PCIE20_PLR_IATU_CTRL2 / 4] = BIT(31);
	}

	/*
	 * program and enable address translation region 0 (device config
	 * address space); region type config;
	 * axi config address range to device config address range
	 */
	writel_relaxed(0, dev->dm_core + PCIE20_PLR_IATU_VIEWPORT);
	/* ensure that hardware locks the region before programming it */
	wmb();

	writel_relaxed(4, dev->dm_core + PCIE20_PLR_IATU_CTRL1);
	writel_relaxed(lower, dev->dm_core + PCIE20_PLR_IATU_LBAR);
	writel_relaxed(upper, dev->dm_core + PCIE20_PLR_IATU_UBAR);
	writel_relaxed(limit, dev->dm_core + PCIE20_PLR_IATU_LAR);
	writel_relaxed(dev_conf, dev->dm_core + PCIE20_PLR_IATU_LTAR);
	writel_relaxed(0, dev->dm_core + PCIE20_PLR_IATU_UTAR);
	writel_relaxed(BIT(31), dev->dm_core + PCIE20_PLR_IATU_CTRL2);
	/* ensure that hardware registers the configuration */
	wmb();
	PCIE_DBG2(dev, "PCIE20_PLR_IATU_VIEWPORT:0x%x\n",
		readl_relaxed(dev->dm_core + PCIE20_PLR_IATU_VIEWPORT));
	PCIE_DBG2(dev, "PCIE20_PLR_IATU_CTRL1:0x%x\n",
		readl_relaxed(dev->dm_core + PCIE20_PLR_IATU_CTRL1));
	PCIE_DBG2(dev, "PCIE20_PLR_IATU_LBAR:0x%x\n",
		readl_relaxed(dev->dm_core + PCIE20_PLR_IATU_LBAR));
	PCIE_DBG2(dev, "PCIE20_PLR_IATU_UBAR:0x%x\n",
		readl_relaxed(dev->dm_core + PCIE20_PLR_IATU_UBAR));
	PCIE_DBG2(dev, "PCIE20_PLR_IATU_LAR:0x%x\n",
		readl_relaxed(dev->dm_core + PCIE20_PLR_IATU_LAR));
	PCIE_DBG2(dev, "PCIE20_PLR_IATU_LTAR:0x%x\n",
		readl_relaxed(dev->dm_core + PCIE20_PLR_IATU_LTAR));
	PCIE_DBG2(dev, "PCIE20_PLR_IATU_UTAR:0x%x\n",
		readl_relaxed(dev->dm_core + PCIE20_PLR_IATU_UTAR));
	PCIE_DBG2(dev, "PCIE20_PLR_IATU_CTRL2:0x%x\n",
		readl_relaxed(dev->dm_core + PCIE20_PLR_IATU_CTRL2));

	/* configure N_FTS */
	PCIE_DBG2(dev, "Original PCIE20_ACK_F_ASPM_CTRL_REG:0x%x\n",
		readl_relaxed(dev->dm_core + PCIE20_ACK_F_ASPM_CTRL_REG));
	if (!dev->n_fts)
		msm_pcie_write_mask(dev->dm_core + PCIE20_ACK_F_ASPM_CTRL_REG,
					0, BIT(15));
	else
		msm_pcie_write_mask(dev->dm_core + PCIE20_ACK_F_ASPM_CTRL_REG,
					PCIE20_ACK_N_FTS,
					dev->n_fts << 8);
	readl_relaxed(dev->elbi);

	if (dev->shadow_en)
		dev->rc_shadow[PCIE20_ACK_F_ASPM_CTRL_REG / 4] =
			readl_relaxed(dev->dm_core +
			PCIE20_ACK_F_ASPM_CTRL_REG);

	PCIE_DBG2(dev, "Updated PCIE20_ACK_F_ASPM_CTRL_REG:0x%x\n",
		readl_relaxed(dev->dm_core + PCIE20_ACK_F_ASPM_CTRL_REG));
}

static int msm_pcie_get_resources(struct msm_pcie_dev_t *dev,
					struct platform_device *pdev)
{
	int i, ret = 0;
	struct resource *res;
	struct msm_pcie_res_info_t *res_info;
	struct msm_pcie_irq_info_t *irq_info;

	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);

	for (i = 0; i < MSM_PCIE_MAX_RES; i++) {
		res_info = &dev->res[i];

		res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
							   res_info->name);

		if (!res) {
			PCIE_ERR(dev, "PCIe: RC%d can't get %s resource.\n",
				dev->rc_idx, res_info->name);
			ret = -ENOMEM;
			goto out;
		} else
			PCIE_DBG(dev, "start addr for %s is %pa.\n",
				res_info->name,	&res->start);

		res_info->base = devm_ioremap(&pdev->dev,
						res->start, resource_size(res));
		if (!res_info->base) {
			PCIE_ERR(dev, "PCIe: RC%d can't remap %s.\n",
				dev->rc_idx, res_info->name);
			ret = -ENOMEM;
			goto out;
		}
		res_info->resource = res;
	}

	for (i = 0; i < MSM_PCIE_MAX_IRQ; i++) {
		irq_info = &dev->irq[i];

		if ((i == MSM_PCIE_INT_WAKE))
			continue;

		res = platform_get_resource_byname(pdev, IORESOURCE_IRQ,
							   irq_info->name);

		if (!res) {
			int j;
			for (j = 0; j < MSM_PCIE_MAX_RES; j++) {
				iounmap(dev->res[j].base);
				dev->res[j].base = NULL;
			}
			PCIE_ERR(dev, "PCIe: RC%d can't find IRQ # for %s.\n",
				dev->rc_idx, irq_info->name);
			ret = -ENODEV;
			goto out;
		} else {
			irq_info->num = res->start;
			PCIE_DBG(dev, "IRQ # for %s is %d.\n", irq_info->name,
					irq_info->num);
		}
	}

	/* All allocations succeeded */

	dev->wake_n = dev->irq[MSM_PCIE_INT_WAKE].num;

	dev->parf = dev->res[MSM_PCIE_RES_PARF].base;
	dev->phy = dev->res[MSM_PCIE_RES_PHY].base;
	dev->elbi = dev->res[MSM_PCIE_RES_ELBI].base;
	dev->dm_core = dev->res[MSM_PCIE_RES_DM_CORE].base;
	dev->conf = dev->res[MSM_PCIE_RES_CONF].base;
	dev->bars = dev->res[MSM_PCIE_RES_BARS].base;
	dev->dev_mem_res = dev->res[MSM_PCIE_RES_BARS].resource;
	dev->dev_io_res = dev->res[MSM_PCIE_RES_IO].resource;
	dev->dev_io = dev->res[MSM_PCIE_RES_IO].base;
	dev->dev_io_res->flags = IORESOURCE_IO;

out:
	return ret;
}

static void msm_pcie_release_resources(struct msm_pcie_dev_t *dev)
{
	dev->parf = NULL;
	dev->elbi = NULL;
	dev->dm_core = NULL;
	dev->conf = NULL;
	dev->bars = NULL;
	dev->dev_mem_res = NULL;
	dev->dev_io_res = NULL;
}

#define PCIE_PHY_PCS_STATUS	0x728
static bool pcie_phy_is_ready(struct msm_pcie_dev_t *dev)
{
	if (readl_relaxed(dev->phy + PCIE_PHY_PCS_STATUS) & BIT(6))
		return false;
	else
		return true;
}

static int msm_pcie_enable(struct msm_pcie_dev_t *dev, u32 options)
{
	int ret = 0;
	uint32_t val;
	uint32_t val_last = -1u;
	long int retries = 0;
	int link_check_count = 0;
	unsigned long jiffies_end = 0;

	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);

	mutex_lock(&dev->setup_lock);

	if (dev->link_status == MSM_PCIE_LINK_ENABLED) {
		PCIE_ERR(dev, "PCIe: the link of RC%d is already enabled\n",
			dev->rc_idx);
		goto out;
	}


	/* assert PCIe reset link to keep EP in reset */

	PCIE_INFO(dev, "PCIe: trigger the reset of endpoint of RC%d.\n",
		dev->rc_idx);
	msm_pcie_dev_reset(dev, 1);
//	gpio_set_value(dev->gpio_rst, 1);
	usleep_range(PERST_PROPAGATION_DELAY_US_MIN,
				 PERST_PROPAGATION_DELAY_US_MAX);

	if (options & PM_CLK) {
		msm_pcie_clk_init(dev);
		wmb();
	}

	/* enable PCIe clocks and resets */
	msm_pcie_write_mask(dev->parf + PCIE20_PARF_PHY_CTRL, BIT(0), 0);

	/* change DBI base address */
	writel_relaxed(0, dev->parf + PCIE20_PARF_DBI_BASE_ADDR);

	if (dev->rc_idx)
		writel_relaxed(0x361c, dev->parf + PCIE20_PARF_SYS_CTRL);
	else
		writel_relaxed(0x3656, dev->parf + PCIE20_PARF_SYS_CTRL);

	writel_relaxed(0, dev->parf + PCIE20_PARF_Q2A_FLUSH);

	if (dev->use_msi) {
		PCIE_DBG(dev, "RC%d: enable WR halt.\n", dev->rc_idx);
		msm_pcie_write_mask(dev->parf +
			PCIE20_PARF_AXI_MSTR_WR_ADDR_HALT, 0, BIT(31));
	}

	writel_relaxed(CMD_BME_VAL, dev->dm_core + PCIE20_COMMAND_STATUS);
	writel_relaxed(DBI_RO_WR_EN,
				 dev->dm_core + PCIE20_MISC_CONTROL_1_REG);
	writel(1, dev->dm_core + PCIE20_MISC_CONTROL_1_REG);
	writel_relaxed(PCIE_CAP_LINK1_VAL, dev->dm_core + PCIE20_CAP_LINK_1);
	msm_pcie_write_mask(dev->dm_core + PCIE20_CAP_LINK_CAPABILITIES,
						 BIT(10) | BIT(11), 0);
	writel_relaxed(PCIE_CAP_CPL_TIMEOUT_DISABLE,
			 dev->dm_core + PCIE20_DEVICE_CONTROL2_STATUS2);
	writel_relaxed(LTSSM_EN, dev->parf + PCIE20_PARF_LTSSM);

	PCIE_DBG(dev, "RC%d: waiting for phy ready...\n", dev->rc_idx);

	do {
		if (pcie_phy_is_ready(dev))
			break;
		retries++;
		usleep_range(REFCLK_STABILIZATION_DELAY_US_MIN,
					 REFCLK_STABILIZATION_DELAY_US_MAX);
	} while (retries < PHY_READY_TIMEOUT_COUNT);

	PCIE_DBG(dev, "RC%d: number of PHY retries:%ld.\n",
		dev->rc_idx, retries);

	if (pcie_phy_is_ready(dev))
		PCIE_INFO(dev, "PCIe RC%d PHY is ready!\n", dev->rc_idx);
	else {
		PCIE_ERR(dev, "PCIe PHY RC%d failed to come up!\n",
			dev->rc_idx);
		ret = -ENODEV;
		goto link_fail;
	}

	if (dev->ep_latency)
		msleep(dev->ep_latency);

	/* de-assert PCIe reset link to bring EP out of reset */

	PCIE_INFO(dev, "PCIe: Release the reset of endpoint of RC%d.\n",
		dev->rc_idx);
	msm_pcie_dev_reset(dev, 0);
//	gpio_set_value(dev->gpio_rst, 0);
	usleep_range(PERST_PROPAGATION_DELAY_US_MIN,
				 PERST_PROPAGATION_DELAY_US_MAX);

	/* enable link training */
	msm_pcie_write_mask(dev->elbi + PCIE20_ELBI_SYS_CTRL, 0, BIT(0));
	unsigned reg_val;
	reg_val = readl(dev->parf + 0x1b0);
	reg_val |= BIT(8);
	writel(reg_val, dev->parf + 0x1b0); 

	PCIE_DBG(dev, "%s", "check if link is up\n");

	if (dev->rc_idx) {
		PCIE_DBG(dev, "optimized link training for RC%d\n",
			dev->rc_idx);
		/* Wait for up to 100ms for the link to come up */
		do {
			usleep_range(LINK_UP_TIMEOUT_US_MIN,
					LINK_UP_TIMEOUT_US_MAX);
			val =  readl_relaxed(dev->elbi + PCIE20_ELBI_SYS_STTS);
		} while ((!(val & XMLH_LINK_UP) ||
			!msm_pcie_confirm_linkup(dev, false, false))
			&& (link_check_count++ < LINK_UP_CHECK_MAX_COUNT));

		if ((val & XMLH_LINK_UP) &&
			msm_pcie_confirm_linkup(dev, false, false))
			PCIE_DBG(dev, "Link is up after %d checkings\n",
				link_check_count);
		else
			PCIE_DBG(dev, "Initial link training failed for RC%d\n",
				dev->rc_idx);
	} else {
		PCIE_DBG(dev, "non-optimized link training for RC%d\n",
			dev->rc_idx);
		usleep_range(LINK_RETRY_TIMEOUT_US_MIN * 5 ,
			LINK_RETRY_TIMEOUT_US_MAX * 5);
		val =  readl_relaxed(dev->elbi + PCIE20_ELBI_SYS_STTS);
	}

	retries = 0;

	while ((!(val & XMLH_LINK_UP) ||
		!msm_pcie_confirm_linkup(dev, false, false))
		&& (retries < MAX_LINK_RETRIES)) {
		if (val_last != val) {
			val_last = val;
			PCIE_ERR(dev, "RC%d:No. %ld:LTSSM_STATE:0x%x\n",
				 dev->rc_idx,
				 retries + 1, (val >> 0xC) & 0x1f);
		}
		msm_pcie_dev_reset(dev, 1);
//		gpio_set_value(dev->gpio_rst, 1);
		usleep_range(PERST_PROPAGATION_DELAY_US_MIN,
					 PERST_PROPAGATION_DELAY_US_MAX);
		msm_pcie_dev_reset(dev, 0);
//		gpio_set_value(dev->gpio_rst, 0);
		usleep_range(LINK_RETRY_TIMEOUT_US_MIN,
				LINK_RETRY_TIMEOUT_US_MAX);
		retries++;
		if (retries == MAX_LINK_RETRIES && dev->gpio_booster) {
			if (jiffies_end == 0) {
				jiffies_end = jiffies + 150 * HZ;
				gpio_set_value(dev->gpio_booster, 1);
				printk("enable pcie booster\n");
			}
			else if (time_after(jiffies, jiffies_end)) {
				break;
			}
			--retries;
		}
		val =  readl_relaxed(dev->elbi + PCIE20_ELBI_SYS_STTS);
	}
	if (jiffies_end) {
		if (!time_after(jiffies, jiffies_end)) {
			int secsBoosted = (jiffies + 150*HZ - jiffies_end) / HZ;
			msleep(secsBoosted * 1000 / 3);
		}
		printk("disable pcie booster after %lds, val 0x%x\n",
		       (jiffies + 150 * HZ - jiffies_end) / HZ, val);
		gpio_set_value(dev->gpio_booster, 0);
	}

	PCIE_DBG(dev, "number of link training retries: %ld\n", retries);

	if ((val & XMLH_LINK_UP) &&
		msm_pcie_confirm_linkup(dev, false, false)) {
		PCIE_INFO(dev, "PCIe RC%d link initialized\n", dev->rc_idx);
	} else {
		PCIE_INFO(dev, "PCIe: trigger the reset of endpoint of RC%d.\n",
			dev->rc_idx);
		msm_pcie_dev_reset(dev, 1);
//		gpio_set_value(dev->gpio_rst, 1);
		PCIE_ERR(dev, "PCIe RC%d link initialization failed\n",
			dev->rc_idx);
		ret = -1;
		goto link_fail;
	}

	msm_pcie_config_controller(dev);
	msm_pcie_config_msi_controller(dev);

	dev->link_status = MSM_PCIE_LINK_ENABLED;
	dev->power_on = true;
	dev->suspending = false;
	goto out;

link_fail:
	msm_pcie_clk_deinit(dev);
out:
	mutex_unlock(&dev->setup_lock);

	return ret;
}


static void msm_pcie_disable(struct msm_pcie_dev_t *dev, u32 options)
{
	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);

	mutex_lock(&dev->setup_lock);

	if (!dev->power_on) {
		PCIE_DBG(dev,
			"PCIe: the link of RC%d is already power down.\n",
			dev->rc_idx);
		mutex_unlock(&dev->setup_lock);
		return;
	}

	dev->link_status = MSM_PCIE_LINK_DISABLED;
	dev->power_on = false;

	PCIE_INFO(dev, "PCIe: trigger the reset of endpoint of RC%d.\n",
		dev->rc_idx);

	gpio_set_value(dev->gpio_rst, 1);
	msm_pcie_dev_reset(dev, 1);

	if (options & PM_CLK) {
		msm_pcie_write_mask(dev->parf + PCIE20_PARF_PHY_CTRL, 0,
					BIT(0));
		msm_pcie_clk_deinit(dev);
	}

	mutex_unlock(&dev->setup_lock);
}

static int msm_pcie_setup(int nr, struct pci_sys_data *sys)
{
	struct msm_pcie_dev_t *dev = &msm_pcie_dev;

	PCIE_DBG(dev, "bus %d\n", nr);
	/*
	 * specify linux PCI framework to allocate device memory (BARs)
	 * from msm_pcie_dev.dev_mem_res resource.
	 */
	sys->private_data = dev;
	sys->mem_offset = 0;
	sys->io_offset = 0;

	pci_add_resource(&sys->resources, dev->dev_io_res);
	pci_add_resource(&sys->resources, dev->dev_mem_res);
	return 1;
}

static struct pci_bus *msm_pcie_scan_bus(int nr, struct pci_sys_data *sys)
{
	struct pci_bus *child, *bus = NULL;
	struct msm_pcie_dev_t *dev = &msm_pcie_dev;

	PCIE_DBG(dev, "bus %d\n", nr);

	bus = pci_create_root_bus(NULL, sys->busnr, &msm_pcie_ops, sys,
					&sys->resources);
	pci_scan_child_bus(bus);
	pci_bus_size_bridges(bus);
	pci_bus_assign_resources(bus);
	list_for_each_entry(child, &bus->children, node)
		pcie_bus_configure_settings(child, 0);
	pci_bus_add_devices(bus);


	dev->pci_bus = bus;

	return bus;
}

static int msm_pcie_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	struct msm_pcie_dev_t *pcie_dev = PCIE_BUS_PRIV_DATA(dev);
	int ret = 0;

	PCIE_DBG(pcie_dev, "rc %s slot %d pin %d\n", pcie_dev->pdev->name,
		slot, pin);

	switch (pin) {
	case 1:
		ret = pcie_dev->irq[MSM_PCIE_INT_A].num;
		break;
	case 2:
		ret = pcie_dev->irq[MSM_PCIE_INT_B].num;
		break;
	case 3:
		ret = pcie_dev->irq[MSM_PCIE_INT_C].num;
		break;
	case 4:
		ret = pcie_dev->irq[MSM_PCIE_INT_D].num;
		break;
	default:
		PCIE_ERR(pcie_dev, "PCIe: RC%d: unsupported pin number.\n",
			pcie_dev->rc_idx);
	}

	return ret;
}

static struct hw_pci msm_pci = {
	.domain = 0,
	.nr_controllers	= 1,
	.swizzle	= pci_common_swizzle,
	.setup		= msm_pcie_setup,
	.scan		= msm_pcie_scan_bus,
	.map_irq	= msm_pcie_map_irq,
};

static int msm_pcie_enumerate(u32 rc_idx)
{
	int ret = 0;
	struct msm_pcie_dev_t *dev = &msm_pcie_dev;

	PCIE_DBG(dev, "Enumerate RC%d\n", rc_idx);

	if (!dev->enumerated) {
		ret = msm_pcie_enable(dev, PM_ALL);

		/* kick start ARM PCI configuration framework */
		if (!ret) {
			struct pci_dev *pcidev = NULL;
			bool found = false;
			u32 ids = readl_relaxed(msm_pcie_dev.dm_core);
			u32 vendor_id = ids & 0xffff;
			u32 device_id = (ids & 0xffff0000) >> 16;

			PCIE_DBG(dev, "vendor-id:0x%x device_id:0x%x\n",
					vendor_id, device_id);

			pci_common_init(&msm_pci);

			/* This has to happen only once */
			dev->enumerated = true;

			do {
				pcidev = pci_get_device(vendor_id,
					device_id, pcidev);
				if (pcidev && (&msm_pcie_dev ==
					(struct msm_pcie_dev_t *)
					PCIE_BUS_PRIV_DATA(pcidev))) {
					msm_pcie_dev.dev = pcidev;
					found = true;
					PCIE_DBG(&msm_pcie_dev,
						"PCI device is found for RC%d\n",
						rc_idx);
				}
			} while (!found && pcidev);

			if (!pcidev) {
				PCIE_ERR(dev,
					"PCIe: Did not find PCI device for RC%d.\n",
					dev->rc_idx);
				return -ENODEV;
			}
		} else {
			PCIE_ERR(dev, "PCIe: failed to enable RC%d.\n",
				dev->rc_idx);
		}
	} else {
		PCIE_ERR(dev, "PCIe: RC%d has already been enumerated.\n",
			dev->rc_idx);
	}

	return ret;
}

static int of_dev_node_match(struct device *dev, void *data)
{
	return dev->of_node == data;
}

static int msm_pcie_probe(struct platform_device *pdev)
{
	int ret = 0;
	int rc_idx = -1;
	int i;
	enum of_gpio_flags flags;
	struct device_node *np = pdev->dev.of_node, *child, *gchild;

	pr_debug("%s\n", __func__);

	mutex_lock(&pcie_drv.drv_lock);

	ret = of_property_read_u32(np, "qcom,ctrl-amt", &pcie_drv.rc_expected);
	if (ret) {
		pr_err("PCIe: does not find controller amount.\n");
		goto out;
	} else {
		if (pcie_drv.rc_expected > MAX_RC_NUM) {
			pr_debug("Expected number of devices %d\n",
							pcie_drv.rc_expected);
			pr_debug("Exceeded max supported devices %d\n",
							MAX_RC_NUM);
			goto out;
		}
		pr_debug("Target has %d RC(s).\n", pcie_drv.rc_expected);
	}

	ret = of_property_read_u32(np, "cell-index", &rc_idx);
	if (ret) {
		pr_debug("Did not find RC index.\n");
		goto out;
	} else {
		if (rc_idx >= MAX_RC_NUM) {
			pr_err(
				"PCIe: Invalid RC Index %d (max supported = %d)\n",
				rc_idx, MAX_RC_NUM);
			goto out;
		}
		pcie_drv.rc_num++;
		PCIE_DBG(&msm_pcie_dev, "PCIe: RC index is %d.\n",
			rc_idx);
	}

	msm_pcie_dev.n_fts = 0;
	ret = of_property_read_u32(np, "qcom,n-fts", &msm_pcie_dev.n_fts);

	if (ret)
		PCIE_DBG(&msm_pcie_dev,
			"n-fts does not exist. ret=%d\n", ret);
	else
		PCIE_DBG(&msm_pcie_dev, "n-fts: 0x%x.\n",
				msm_pcie_dev.n_fts);

	msm_pcie_dev.ep_latency = 0;
	ret = of_property_read_u32(np, "qcom,ep-latency", &msm_pcie_dev.ep_latency);
	if (ret)
		PCIE_DBG(&msm_pcie_dev,
			"RC%d: ep-latency does not exist.\n",
			rc_idx);
	else
		PCIE_DBG(&msm_pcie_dev, "RC%d: ep-latency: 0x%x.\n",
			rc_idx, msm_pcie_dev.ep_latency);

	msm_pcie_dev.rc_idx = rc_idx;
	msm_pcie_dev.pdev = pdev;
	msm_pcie_dev.parf_deemph = 0;
	msm_pcie_dev.parf_swing = 0;
	msm_pcie_dev.link_status = MSM_PCIE_LINK_DEINIT;
	msm_pcie_dev.user_suspend = false;
	msm_pcie_dev.saved_state = NULL;
	msm_pcie_dev.enumerated = false;
	msm_pcie_dev.linkdown_counter = 0;
	msm_pcie_dev.suspending = false;
	msm_pcie_dev.wake_counter = 0;
	msm_pcie_dev.req_exit_l1_counter = 0;
	msm_pcie_dev.power_on = false;
	msm_pcie_dev.use_msi = false;

	memcpy(msm_pcie_dev.res, msm_pcie_res_info,
				sizeof(msm_pcie_res_info));
	memcpy(msm_pcie_dev.irq, msm_pcie_irq_info,
				sizeof(msm_pcie_irq_info));
//	msm_pcie_dev.shadow_en = true;
//	for (i = 0; i < PCIE_CONF_SPACE_DW; i++) {
//		msm_pcie_dev.rc_shadow[i] = PCIE_CLEAR;
//		msm_pcie_dev.ep_shadow[i] = PCIE_CLEAR;
//	}

	ret = msm_pcie_get_resources(&msm_pcie_dev,
				msm_pcie_dev.pdev);

	if (ret)
		goto decrease_rc_num;

	ret = of_get_named_gpio_flags(np, "gpio-booster", 0, &flags);
	if (ret >= 0) {
		unsigned gpio_num = ret;
		unsigned val_on = !(flags & OF_GPIO_ACTIVE_LOW);
		ret = gpio_request(gpio_num, pdev->name);
		if (ret >= 0) {
			msm_pcie_dev.gpio_booster = gpio_num;
//			msm_pcie_dev.gpio_booster_on = val_on;
			gpio_direction_output(gpio_num, !val_on);
			printk("pcie booster on gpio %d\n", gpio_num);
		}
	}

	for_each_child_of_node(np, child) {
		const char *name;
		int gchild_count = 0;
		struct msm_pci_devices_t *curr;
		enum of_gpio_flags fl;

		name = of_get_property(child, "label", NULL) ? : child->name;
		if (!strcmp(name, "switch")) {
			struct device *dev;
			msm_pcie_dev.of_defined_device = 1;
		} else
			continue;

		for_each_child_of_node(child, gchild) {
			gchild_count++;
		}
		gchild_count += 2;
		msm_pcie_dev.children = 
			kzalloc(gchild_count * sizeof(struct msm_pci_devices_t), GFP_KERNEL);
		gchild = 0;
		curr = msm_pcie_dev.children;
		curr->name = of_get_property(child, "label", NULL) ? : child->name;
		curr->gpio = of_get_gpio_flags(child, 0, &fl);
		curr->polarity = !(fl & OF_GPIO_ACTIVE_LOW);
		curr++;

		for_each_child_of_node(child, gchild) {
			curr->gpio = of_get_gpio_flags(gchild, 0, &fl);
			curr->name = of_get_property(gchild, "label", NULL) ? : gchild->name;
			curr->polarity = !(fl & OF_GPIO_ACTIVE_LOW);
			curr++;
		}

		curr = msm_pcie_dev.children;
		while (curr->name || curr->gpio) {
			printk("%s : %d %d\n", curr->name, curr->gpio, curr->polarity);
			gpio_request(curr->gpio, curr->name);
			gpio_direction_output(curr->gpio, 1);
			curr++;
		}
	}

	msm_pcie_dev.gpio_rst = 0x2a;		// GPIO 42, active-high level
	ioremap_reg(TLMM_GPIO_CFG(msm_pcie_dev.gpio_rst), 0, BIT(9)); // gpio oe
	msm_pcie_dev_reset(&msm_pcie_dev, 1);

	/* s/w reset of pcie */
	msm_pcie_controller_reset(&msm_pcie_dev);

	ret = ipq40xx_irq_init(&msm_pcie_dev);
	if (ret) {
		msm_pcie_release_resources(&msm_pcie_dev);
		goto decrease_rc_num;
	}

	ret = msm_pcie_enumerate(rc_idx);

	if (ret) {
		PCIE_ERR(&msm_pcie_dev,
			"PCIe: RC%d is not enabled during bootup; it will be enumerated upon WAKE signal.\n",
			rc_idx);
		goto decrease_rc_num;
	} else {
		PCIE_ERR(&msm_pcie_dev, "RC%d is enabled in bootup\n",
			rc_idx);
	}

	PCIE_DBG(&msm_pcie_dev, "PCIE probed %s\n",
		dev_name(&(pdev->dev)));

	mutex_unlock(&pcie_drv.drv_lock);
	return 0;

decrease_rc_num:
	pcie_drv.rc_num--;
out:
	PCIE_ERR(&msm_pcie_dev,
		"PCIe: Driver probe failed for RC%d:%d\n",
		rc_idx, ret);
	mutex_unlock(&pcie_drv.drv_lock);

	return ret;
}

static int __exit msm_pcie_remove(struct platform_device *pdev)
{
	int ret = 0;
	int rc_idx;

	pr_debug("PCIe:%s.\n", __func__);

	mutex_lock(&pcie_drv.drv_lock);

	ret = of_property_read_u32((&pdev->dev)->of_node,
				"cell-index", &rc_idx);
	if (ret) {
		pr_err("%s: Did not find RC index.\n", __func__);
		goto out;
	} else {
		pcie_drv.rc_num--;
		pr_debug("%s: RC index is 0x%x.", __func__, rc_idx);
	}

	msm_pcie_clk_deinit(&msm_pcie_dev);
	msm_pcie_release_resources(&msm_pcie_dev);

out:
	mutex_unlock(&pcie_drv.drv_lock);

	return ret;
}

static struct of_device_id msm_pcie_match[] = {
	{	.compatible = "qcom,msm_pcie",
	},
	{}
};

static struct platform_driver msm_pcie_driver = {
	.probe	= msm_pcie_probe,
	.remove	= msm_pcie_remove,
	.driver	= {
		.name		= "msm_pcie",
		.owner		= THIS_MODULE,
		.of_match_table	= msm_pcie_match,
	},
};

static int __init pcie_init(void)
{
	pcie_drv.rc_num = 0;
	pcie_drv.rc_expected = 0;
	mutex_init(&pcie_drv.drv_lock);

	spin_lock_init(&msm_pcie_dev.cfg_lock);
	msm_pcie_dev.cfg_access = true;
	mutex_init(&msm_pcie_dev.setup_lock);
	mutex_init(&msm_pcie_dev.recovery_lock);
	spin_lock_init(&msm_pcie_dev.linkdown_lock);
	spin_lock_init(&msm_pcie_dev.wakeup_lock);

	return platform_driver_register(&msm_pcie_driver);
}

static void __exit pcie_exit(void)
{
	platform_driver_unregister(&msm_pcie_driver);
}

subsys_initcall_sync(pcie_init);
module_exit(pcie_exit);


/* RC do not represent the right class; set it to PCI_CLASS_BRIDGE_PCI */
static void msm_pcie_fixup_early(struct pci_dev *dev)
{
	if (dev->hdr_type == 1)
		dev->class = (dev->class & 0xff) | (PCI_CLASS_BRIDGE_PCI << 8);
}
DECLARE_PCI_FIXUP_EARLY(PCIE_VENDOR_ID_RCP, PCIE_DEVICE_ID_RCP,
			msm_pcie_fixup_early);

/* Suspend the PCIe link */
static int msm_pcie_pm_suspend(struct pci_dev *dev,
			void *user, void *data, u32 options)
{
	int ret = 0;
	u32 val = 0;
	int ret_l23;
	struct msm_pcie_dev_t *pcie_dev = PCIE_BUS_PRIV_DATA(dev);

	pcie_dev->suspending = true;
	PCIE_DBG(pcie_dev, "RC%d\n", pcie_dev->rc_idx);

	if (!pcie_dev->power_on) {
		PCIE_DBG(pcie_dev,
			"PCIe: power of RC%d has been turned off.\n",
			pcie_dev->rc_idx);
		return ret;
	}

	if (dev && !(options & MSM_PCIE_CONFIG_NO_CFG_RESTORE)
		&& msm_pcie_confirm_linkup(pcie_dev, true, true)) {
		ret = pci_save_state(dev);
		pcie_dev->saved_state =	pci_store_saved_state(dev);
	}
	if (ret) {
		PCIE_ERR(pcie_dev, "PCIe: fail to save state of RC%d:%d.\n",
			pcie_dev->rc_idx, ret);
		pcie_dev->suspending = false;
		return ret;
	}

	spin_lock_irqsave(&pcie_dev->cfg_lock,
				pcie_dev->irqsave_flags);
	pcie_dev->cfg_access = false;
	spin_unlock_irqrestore(&pcie_dev->cfg_lock,
				pcie_dev->irqsave_flags);

	msm_pcie_write_mask(pcie_dev->elbi + PCIE20_ELBI_SYS_CTRL, 0,
				BIT(4));

	PCIE_DBG(pcie_dev, "RC%d: PME_TURNOFF_MSG is sent out\n",
		pcie_dev->rc_idx);

	ret_l23 = readl_poll_timeout((pcie_dev->parf
		+ PCIE20_PARF_PM_STTS), val, (val & BIT(6)), 10000, 100000);

	/* check L23_Ready */
	if (!ret_l23)
		PCIE_DBG(pcie_dev, "RC%d: PM_Enter_L23 is received\n",
			pcie_dev->rc_idx);
	else
		PCIE_DBG(pcie_dev, "RC%d: PM_Enter_L23 is NOT received\n",
			pcie_dev->rc_idx);

	msm_pcie_disable(pcie_dev, PM_PIPE_CLK | PM_CLK | PM_VREG);

	return ret;
}

static void msm_pcie_fixup_suspend(struct pci_dev *dev)
{
	int ret;
	struct msm_pcie_dev_t *pcie_dev = PCIE_BUS_PRIV_DATA(dev);

	PCIE_DBG(pcie_dev, "RC%d\n", pcie_dev->rc_idx);

	if (pcie_dev->link_status != MSM_PCIE_LINK_ENABLED)
		return;

	mutex_lock(&pcie_dev->recovery_lock);

	ret = msm_pcie_pm_suspend(dev, NULL, NULL, 0);
	if (ret)
		PCIE_ERR(pcie_dev, "PCIe: RC%d got failure in suspend:%d.\n",
			pcie_dev->rc_idx, ret);

	mutex_unlock(&pcie_dev->recovery_lock);
}
DECLARE_PCI_FIXUP_SUSPEND(PCIE_VENDOR_ID_RCP, PCIE_DEVICE_ID_RCP,
			  msm_pcie_fixup_suspend);

/* Resume the PCIe link */
static int msm_pcie_pm_resume(struct pci_dev *dev,
			void *user, void *data, u32 options)
{
	int ret;
	struct msm_pcie_dev_t *pcie_dev = PCIE_BUS_PRIV_DATA(dev);

	PCIE_DBG(pcie_dev, "RC%d\n", pcie_dev->rc_idx);

	spin_lock_irqsave(&pcie_dev->cfg_lock,
				pcie_dev->irqsave_flags);
	pcie_dev->cfg_access = true;
	spin_unlock_irqrestore(&pcie_dev->cfg_lock,
				pcie_dev->irqsave_flags);

	ret = msm_pcie_enable(pcie_dev, PM_PIPE_CLK | PM_CLK | PM_VREG);
	if (ret) {
		PCIE_ERR(pcie_dev,
			"PCIe: RC%d fail to enable PCIe link in resume.\n",
			pcie_dev->rc_idx);
		return ret;
	} else {
		pcie_dev->suspending = false;
		PCIE_DBG(pcie_dev,
			"dev->bus->number = %d dev->bus->primary = %d\n",
			 dev->bus->number, dev->bus->primary);

		if (!(options & MSM_PCIE_CONFIG_NO_CFG_RESTORE)) {
			pci_load_and_free_saved_state(dev,
					&pcie_dev->saved_state);
			pci_restore_state(dev);
		}
	}

	return ret;
}

static void msm_pcie_fixup_resume(struct pci_dev *dev)
{
	int ret;
	struct msm_pcie_dev_t *pcie_dev = PCIE_BUS_PRIV_DATA(dev);

	PCIE_DBG(pcie_dev, "RC%d\n", pcie_dev->rc_idx);

	if ((pcie_dev->link_status != MSM_PCIE_LINK_DISABLED) ||
		pcie_dev->user_suspend)
		return;

	mutex_lock(&pcie_dev->recovery_lock);
	ret = msm_pcie_pm_resume(dev, NULL, NULL, 0);
	if (ret)
		PCIE_ERR(pcie_dev,
			"PCIe: RC%d got failure in fixup resume:%d.\n",
			pcie_dev->rc_idx, ret);

	mutex_unlock(&pcie_dev->recovery_lock);
}
DECLARE_PCI_FIXUP_RESUME(PCIE_VENDOR_ID_RCP, PCIE_DEVICE_ID_RCP,
				 msm_pcie_fixup_resume);

static void msm_pcie_fixup_resume_early(struct pci_dev *dev)
{
	int ret;
	struct msm_pcie_dev_t *pcie_dev = PCIE_BUS_PRIV_DATA(dev);

	PCIE_DBG(pcie_dev, "RC%d\n", pcie_dev->rc_idx);

	if ((pcie_dev->link_status != MSM_PCIE_LINK_DISABLED) ||
		pcie_dev->user_suspend)
		return;

	mutex_lock(&pcie_dev->recovery_lock);
	ret = msm_pcie_pm_resume(dev, NULL, NULL, 0);
	if (ret)
		PCIE_ERR(pcie_dev, "PCIe: RC%d got failure in resume:%d.\n",
			pcie_dev->rc_idx, ret);

	mutex_unlock(&pcie_dev->recovery_lock);
}
DECLARE_PCI_FIXUP_RESUME_EARLY(PCIE_VENDOR_ID_RCP, PCIE_DEVICE_ID_RCP,
				 msm_pcie_fixup_resume_early);

static void msm_pcie_fixup_final(struct pci_dev *dev)
{
	struct msm_pcie_dev_t *pcie_dev = PCIE_BUS_PRIV_DATA(dev);
	PCIE_DBG(pcie_dev, "RC%d\n", pcie_dev->rc_idx);
	pcie_drv.current_rc++;
}
DECLARE_PCI_FIXUP_FINAL(PCI_ANY_ID, PCI_ANY_ID, msm_pcie_fixup_final);
