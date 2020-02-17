 /* * Copyright (c) 2013 Qualcomm Atheros, Inc. * */
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
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/pm_runtime.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/list.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/msm_hsusb.h>

#include <mach/msm_bus.h>
#include <mach/msm_iomap.h>
#include <mach/msm_usb30.h>
#include "core.h"
#include "gadget.h"

#define REG(off)				(MSM_CLK_CTL_BASE + (off))
#define USB30_RESET				REG(0x3B50)
#define USB30_1_RESET                           REG(0x3B58)

#define DWC3_SSUSB_REG_GCTL			0xC110
#define DWC3_SSUSB_REG_GUSB2PHYCFG(n)		(0xC200 + ((n) * 0x16))
#define DWC3_SSUSB_REG_GUCTL			0xC12C
#define DWC3_SSUSB_REG_GUSB3PIPECTL(n)		(0xC2C0 + ((n) * 0x16))

#define DWC3_SSUSB_XHCI_REV_10			1
#define DWC3_SSUSB_XHCI_REV_096			0

#define DWC3_SSUSB_REG_GUSB2PHYCFG_PHYSOFTRST			(1 << 31)
#define DWC3_SSUSB_REG_GUSB2PHYCFG_RSVD(n)			((n) << 19)
#define DWC3_SSUSB_REG_GUSB2PHYCFG_ULPIEXTVBUSINDIACTOR 	(1 << 18)
#define DWC3_SSUSB_REG_GUSB2PHYCFG_ULPIEXTVBUSDRV		(1 << 17)
#define DWC3_SSUSB_REG_GUSB2PHYCFG_ULPICLKSUSM			(1 << 16)
#define DWC3_SSUSB_REG_GUSB2PHYCFG_ULPIAUTORES			(1 << 15)
#define DWC3_SSUSB_REG_GUSB2PHYCFG_RSVD9			(1 << 14)
#define DWC3_SSUSB_REG_GUSB2PHYCFG_USBTRDTIM(n)			((n) << 10)
#define DWC3_SSUSB_REG_GUSB2PHYCFG_RSVD0			(1 << 9)
#define DWC3_SSUSB_REG_GUSB2PHYCFG_ENBLSLPM			(1 << 8)
#define DWC3_SSUSB_REG_GUSB2PHYCFG_PHYSEL			(1 << 7)
#define DWC3_SSUSB_REG_GUSB2PHYCFG_SUSPENDUSB20			(1 << 6)
#define DWC3_SSUSB_REG_GUSB2PHYCFG_FSINTF			(1 << 5)
#define DWC3_SSUSB_REG_GUSB2PHYCFG_ULPI_UTMI_SEL		(1 << 4)
#define DWC3_SSUSB_REG_GUSB2PHYCFG_PHYIF			(1 << 3)
#define DWC3_SSUSB_REG_GUSB2PHYCFG_B1L(n)			((n) << 18)

#define DWC3_SSUSB_REG_GUSB3PIPECTL_PHYSOFTRST				(1 << 31)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_HSTPRTCMPL				(1 << 30)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_U2SSINACTP3OK			(1 << 29)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_DISRXDETP3				(1 << 28)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_UX_EXIT_IN_PX			(1 << 27)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_PING_ENHANCEMENT_EN			(1 << 26)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_U1U2EXITFAIL_TO_RECOV 		(1 << 25)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_REQUEST_P1P2P3			(1 << 24)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_STARTRXDETU3RXDET			(1 << 23)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_DISRXDETU3RXDET			(1 << 22)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_DELAYP1P2P3(n)			((n) << 19)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_DELAYP1TRANS			(1 << 18)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_SUSPENDENABLE			(1 << 17)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_DATWIDTH(n)				((n) << 15)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_ABORTRXDETINU2			(1 << 14)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_SKIPRXDET				(1 << 13)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_LFPSP0ALGN				(1 << 12)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_P3P2TRANOK				(1 << 11)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_P3EXSIGP2				(1 << 10)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_LFPSFILTER				(1 << 9)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_RX_DETECT_TO_POLLING_LFPS_CONTRO	(1 << 8)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_RESERVED_BIT7			(1 << 7)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_TX_SWING				(1 << 6)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_TX_MARGIN(n)			((n) << 3)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_TX_DE_EPPHASIS(n)			((n) << 1)
#define DWC3_SSUSB_REG_GUSB3PIPECTL_ELASTIC_BUFFER_MODE			(1 << 0)

/* PHY Registers */
#define DWC3_SSUSB_PHY_TX_DEEMPH_MASK			0x3F80
#define DWC3_SSUSB_PHY_AMP_MASK				0x7F
#define DWC3_SSUSB_PHY_AMP_EN				(1 << 14)
#define DWC3_SSUSB_PHY_TX_DEEMPH_SHIFT			7

#define DWC3_SSUSB_PHY_RX_OVRD_IN_HI_RX_EQ_EN		6
#define DWC3_SSUSB_PHY_RX_OVRD_IN_HI_RX_EQ_EN_OVRD	7
#define DWC3_SSUSB_PHY_RX_OVRD_IN_HI_RX_EQ		8
#define DWC3_SSUSB_PHY_RX_OVRD_IN_HI_RX_EQ_OVRD		11
#define DWC3_SSUSB_PHY_RX_OVRD_IN_HI_RX_EQ_MASK		0x7
#define DWC3_SSUSB_PHY_TX_DEBUG_RXDET_MEAS_TIME_VAL	0x400

#define DWC3_SSUSB_PHY_TX_OVRD_DRV_LO_REG		0x1002
#define DWC3_SSUSB_PHY_RX_OVRD_IN_HI_REG		0x1006
#define DWC3_SSUSB_PHY_TX_ALT_BLOCK_REG			0x102D
#define DWC3_SSUSB_PHY_RTUNE_RTUNE_CTRL_REG		0x34
#define DWC3_SSUSB_PHY_RTUNE_DEBUG_REG			0x3
#define DWC3_SSUSB_PHY_TX_DEBUG_REG			0x1010

#define DWC3_SSUSB_PHY_TX_ALT_BLOCK_EN_ALT_BUS		(1 << 7)

#define DWC3_GCTL_CORESOFTRESET	(1 << 11)
#define DWC3_GCTL_SCALEDOWN(n)	((n) << 4)
#define DWC3_GCTL_SCALEDOWN_MASK DWC3_GCTL_SCALEDOWN(3)
#define DWC3_GCTL_DISSCRAMBLE	(1 << 3)
#define DWC3_GCTL_DSBLCLKGTNG	(1 << 0)
#define DWC3_GCTL_U2EXIT_LFPS	(1 << 2)
#define DWC3_GCTL_SOFITPSYNC	(1 << 10)

/* USB3.0 default values provided by VI */

#define RX_TERM_VALUE           0   /* RTUNE_DEBUG register value */
#define RX_EQ_VALUE             3   /* Override value for rx_eq */
#define AMPLITUDE_VALUE         127 /* Override value for transmit amplitude */
#define TX_DEEMPH_3_5DB         22  /* Override value for transmit preemphasis */


/**
 *  USB DBM Hardware registers.
 *
 */
#define DBM_BASE		0x000F8000
#define DBM_EP_CFG(n)		(DBM_BASE + (0x00 + 4 * (n)))
#define DBM_DATA_FIFO(n)	(DBM_BASE + (0x10 + 4 * (n)))
#define DBM_DATA_FIFO_SIZE(n)	(DBM_BASE + (0x20 + 4 * (n)))
#define DBM_DATA_FIFO_EN	(DBM_BASE + (0x30))
#define DBM_GEVNTADR		(DBM_BASE + (0x34))
#define DBM_GEVNTSIZ		(DBM_BASE + (0x38))
#define DBM_DBG_CNFG		(DBM_BASE + (0x3C))
#define DBM_HW_TRB0_EP(n)	(DBM_BASE + (0x40 + 4 * (n)))
#define DBM_HW_TRB1_EP(n)	(DBM_BASE + (0x50 + 4 * (n)))
#define DBM_HW_TRB2_EP(n)	(DBM_BASE + (0x60 + 4 * (n)))
#define DBM_HW_TRB3_EP(n)	(DBM_BASE + (0x70 + 4 * (n)))
#define DBM_PIPE_CFG		(DBM_BASE + (0x80))
#define DBM_SOFT_RESET		(DBM_BASE + (0x84))
#define DBM_GEN_CFG		(DBM_BASE + (0x88))

/**
 *  USB DBM  Hardware registers bitmask.
 *
 */
/* DBM_EP_CFG */
#define DBM_EN_EP		0x00000001
#define USB3_EPNUM		0x0000003E
#define DBM_BAM_PIPE_NUM	0x000000C0
#define DBM_PRODUCER		0x00000100
#define DBM_DISABLE_WB		0x00000200
#define DBM_INT_RAM_ACC		0x00000400

/* DBM_DATA_FIFO_SIZE */
#define DBM_DATA_FIFO_SIZE_MASK	0x0000ffff

/* DBM_GEVNTSIZ */
#define DBM_GEVNTSIZ_MASK	0x0000ffff

/* DBM_DBG_CNFG */
#define DBM_ENABLE_IOC_MASK	0x0000000f

/* DBM_SOFT_RESET */
#define DBM_SFT_RST_EP0		0x00000001
#define DBM_SFT_RST_EP1		0x00000002
#define DBM_SFT_RST_EP2		0x00000004
#define DBM_SFT_RST_EP3		0x00000008
#define DBM_SFT_RST_EPS_MASK	0x0000000F
#define DBM_SFT_RST_MASK	0x80000000
#define DBM_EN_MASK		0x00000002

#define DBM_MAX_EPS		4

/* DBM TRB configurations */
#define DBM_TRB_BIT		0x80000000
#define DBM_TRB_DATA_SRC	0x40000000
#define DBM_TRB_DMA		0x20000000
#define DBM_TRB_EP_NUM(ep)	(ep<<24)

struct dwc3_ipq_req_complete {
	struct list_head list_item;
	struct usb_request *req;
	void (*orig_complete)(struct usb_ep *ep,
			      struct usb_request *req);
};

struct dwc3_ipq {
	struct platform_device *dwc3;
	struct device *dev;
	void __iomem *base;
	u32 resource_size;
	int dbm_num_eps;
	u8 ep_num_mapping[DBM_MAX_EPS];
	struct list_head req_complete_list;
	struct clk		*core_clk;
	struct clk		*iface0_clk;
	struct clk              *iface1_clk;
	struct clk		*sleep_clk;
	struct clk		*utmi_clk;
	struct clk 		*utmi_b0_clk;
	struct clk              *utmi_b1_clk;
	bool			resume_pending;
	atomic_t                pm_suspended;
	atomic_t		in_lpm;
	struct delayed_work	resume_work;
	struct delayed_work	chg_work;
	enum usb_chg_state	chg_state;
	u8			dcd_retries;
	u8			mode;
	u32			bus_perf_client;
	struct msm_bus_scale_pdata	*bus_scale_table;
};

static struct dwc3_ipq *context;
static u64 dwc3_ipq_dma_mask = DMA_BIT_MASK(64);

static u16 dwc3_ipq_ssusb_read_phy_reg(unsigned int  addr, void __iomem *ipq_base)
{
	u16 tmp_phy[3], i;
	do {
		for (i = 0; i < 3; i++) {
			writel(addr, ipq_base +
				IPQ_SSUSB_REG_QSCRATCH_SS_CR_PROTOCOL_DATA_IN);
			writel(0x1, ipq_base +
				IPQ_SSUSB_REG_QSCRATCH_SS_CR_PROTOCOL_CAP_ADDR);
			while (0 != readl(ipq_base +
				IPQ_SSUSB_REG_QSCRATCH_SS_CR_PROTOCOL_CAP_ADDR));
			writel(0x1, ipq_base +
				IPQ_SSUSB_REG_QSCRATCH_SS_CR_PROTOCOL_READ);
			while (0 != readl(ipq_base +
				IPQ_SSUSB_REG_QSCRATCH_SS_CR_PROTOCOL_READ));
			tmp_phy[i] = (u16)readl(ipq_base +
				IPQ_SSUSB_REG_QSCRATCH_SS_CR_PROTOCOL_DATA_OUT);
		}
	} while (tmp_phy[1] != tmp_phy[2]);
	return tmp_phy[2];
}

static void dwc3_ipq_ssusb_write_phy_reg(u32 addr, u16 data, void __iomem *ipq_base)
{
	writel(addr, ipq_base + IPQ_SSUSB_REG_QSCRATCH_SS_CR_PROTOCOL_DATA_IN);
	writel(0x1, ipq_base + IPQ_SSUSB_REG_QSCRATCH_SS_CR_PROTOCOL_CAP_ADDR);
	while (0 != readl(ipq_base +
		IPQ_SSUSB_REG_QSCRATCH_SS_CR_PROTOCOL_CAP_ADDR));
	writel(data, ipq_base + IPQ_SSUSB_REG_QSCRATCH_SS_CR_PROTOCOL_DATA_IN);
	writel(0x1, ipq_base + IPQ_SSUSB_REG_QSCRATCH_SS_CR_PROTOCOL_CAP_DATA);
	while (0 != readl(ipq_base +
		IPQ_SSUSB_REG_QSCRATCH_SS_CR_PROTOCOL_CAP_DATA));
	writel(0x1, ipq_base + IPQ_SSUSB_REG_QSCRATCH_SS_CR_PROTOCOL_WRITE);
	while (0 != readl(ipq_base +
		IPQ_SSUSB_REG_QSCRATCH_SS_CR_PROTOCOL_WRITE));
}

static void ipq_ssusb_clear_and_set_bits32(u32 offset, u32 clear_bits, u32 set_bits, void __iomem *ipq_base)
{
	u32 data;
	data = readl(ipq_base + offset);
	data = (data & ~clear_bits) | set_bits;
	writel(data, ipq_base + offset);
}

static void ipq_ssusb_clear_bits32(u32 offset, u32 bits, void __iomem *ipq_base)
{
	u32 data;
	data = readl(ipq_base+offset);
	data = data & ~bits;
	writel(data, ipq_base + offset);
}

static void partial_rx_reset_init(void __iomem *ipq_base)
{
        u32 addr = DWC3_SSUSB_PHY_TX_ALT_BLOCK_REG;
        u16 data = dwc3_ipq_ssusb_read_phy_reg(addr, ipq_base);
        data |= DWC3_SSUSB_PHY_TX_ALT_BLOCK_EN_ALT_BUS;
        dwc3_ipq_ssusb_write_phy_reg(addr, data, ipq_base);
        return;
}


static void  uw_ssusb_pre_init(void __iomem *ipq_base)
{
	u32 clear_bits, set_bits, tmp;
	u16 data;
	/* GCTL Reset ON */
	writel(0x800, ipq_base + DWC3_SSUSB_REG_GCTL);
	/* Config SS PHY CTRL */
	clear_bits = 0;
	set_bits = 0;
	writel(0x80, ipq_base + IPQ_SS_PHY_CTRL_REG);
	udelay(5);
	ipq_ssusb_clear_bits32(IPQ_SS_PHY_CTRL_REG, 0x80, ipq_base);
	udelay(5);
	/* REF_USE_PAD */
	set_bits = 0x0000000;  /* USE Internal clock */
	set_bits |= IPQ_SSUSB_QSCRATCH_SS_PHY_CTRL_LANE0_PWR_PRESENT;
	set_bits |= IPQ_SSUSB_QSCRATCH_SS_PHY_CTRL_REF_SS_PHY_EN;
	writel(set_bits, ipq_base + IPQ_SS_PHY_CTRL_REG);
	/* Config HS PHY CTRL */
	clear_bits = 0;
	/* HS PHY UTMI_OTG_VBUS_VALID | AUTORESUME | COMMONN*/
	set_bits = IPQ_SSUSB_REG_QSCRATCH_HS_PHY_CTRL_AUTORESUME |
			IPQ_SSUSB_REG_QSCRATCH_HS_PHY_CTRL_UTMI_OTG_VBUS_VALID;
	/*
	 * COMMONONN forces xo, bias and pll to stay on during suspend;
	 * Allowing suspend (writing 1) kills Aragorn V1
	 */
	set_bits |= IPQ_SSUSB_REG_QSCRATCH_HS_PHY_CTRL_COMMONONN;
	set_bits |= IPQ_SSUSB_REG_QSCRATCH_HS_PHY_CTRL_USE_CLKCORE;
	set_bits |= IPQ_SSUSB_REG_QSCRATCH_HS_PHY_CTRL_FSEL_VAL;
	/*
	 * If the configuration of clocks is not bypassed in Host mode,
	 * HS PHY suspend needs to be prohibited, otherwise - SS connection fails
	 */
	ipq_ssusb_clear_and_set_bits32(IPQ_SSUSB_REG_QSCRATCH_HS_PHY_CTRL,
					clear_bits, set_bits, ipq_base);
	/* USB2 PHY Reset ON */
	writel(DWC3_SSUSB_REG_GUSB2PHYCFG_PHYSOFTRST, ipq_base +
		DWC3_SSUSB_REG_GUSB2PHYCFG(0));
	/* USB3 PHY Reset ON */
	writel(DWC3_SSUSB_REG_GUSB3PIPECTL_PHYSOFTRST, ipq_base +
		DWC3_SSUSB_REG_GUSB3PIPECTL(0));
	udelay(5);
	/* USB3 PHY Reset OFF */
	ipq_ssusb_clear_bits32(DWC3_SSUSB_REG_GUSB3PIPECTL(0),
				DWC3_SSUSB_REG_GUSB3PIPECTL_PHYSOFTRST, ipq_base);
	ipq_ssusb_clear_bits32(DWC3_SSUSB_REG_GUSB2PHYCFG(0),
				DWC3_GUSB2PHYCFG_PHYSOFTRST, ipq_base);
	udelay(5);
	/* GCTL Reset OFF */
	ipq_ssusb_clear_bits32(DWC3_SSUSB_REG_GCTL, DWC3_GCTL_CORESOFTRESET,
				ipq_base);
	udelay(5);
	/* HACK - override RX detect value */
	data = dwc3_ipq_ssusb_read_phy_reg(DWC3_SSUSB_PHY_TX_DEBUG_REG,
						ipq_base);
	data &= ~0xFF0;
	data |= DWC3_SSUSB_PHY_TX_DEBUG_RXDET_MEAS_TIME_VAL;
	dwc3_ipq_ssusb_write_phy_reg(DWC3_SSUSB_PHY_TX_DEBUG_REG,
					data, ipq_base);
	if (RX_TERM_VALUE) {
		dwc3_ipq_ssusb_write_phy_reg(DWC3_SSUSB_PHY_RTUNE_RTUNE_CTRL_REG,
						0, ipq_base);
		dwc3_ipq_ssusb_write_phy_reg(DWC3_SSUSB_PHY_RTUNE_DEBUG_REG,
						0x0448, ipq_base);
		dwc3_ipq_ssusb_write_phy_reg(DWC3_SSUSB_PHY_RTUNE_DEBUG_REG,
						RX_TERM_VALUE, ipq_base);
	}
	if (4 != RX_EQ_VALUE) { /* Values from 1 to 7 */
		tmp =0;
		/*
		 * 1. Fixed EQ setting. This can be achieved as follows:
		 * LANE0.RX_OVRD_IN_HI. RX_EQ_EN set to 0 - address 1006 bit 6
		 * LANE0.RX_OVRD_IN_HI.RX_EQ_EN_OVRD set to 1 0- address 1006 bit 7
		 * LANE0.RX_OVRD_IN_HI.RX_EQ set to 4 (also try setting 3 if possible) -
		 * address 1006 bits 10:8 - please make this a variable, if unchanged the section is not executed
		 * LANE0.RX_OVRD_IN_HI.RX_EQ_OVRD set to 1 - address 1006 bit 11
		 */
		tmp = dwc3_ipq_ssusb_read_phy_reg(DWC3_SSUSB_PHY_RX_OVRD_IN_HI_REG,
							ipq_base);
		tmp &= ~((u16)1 << DWC3_SSUSB_PHY_RX_OVRD_IN_HI_RX_EQ_EN);
		tmp |= ((u16)1 << DWC3_SSUSB_PHY_RX_OVRD_IN_HI_RX_EQ_EN_OVRD);
		tmp &= ~((u16) DWC3_SSUSB_PHY_RX_OVRD_IN_HI_RX_EQ_MASK <<
			DWC3_SSUSB_PHY_RX_OVRD_IN_HI_RX_EQ);
		tmp |= RX_EQ_VALUE << DWC3_SSUSB_PHY_RX_OVRD_IN_HI_RX_EQ;
		tmp |= 1 << DWC3_SSUSB_PHY_RX_OVRD_IN_HI_RX_EQ_OVRD;
		dwc3_ipq_ssusb_write_phy_reg(DWC3_SSUSB_PHY_RX_OVRD_IN_HI_REG,
						tmp, ipq_base);
	}
	if ((113 != AMPLITUDE_VALUE) || (21 != TX_DEEMPH_3_5DB)) {
		tmp = dwc3_ipq_ssusb_read_phy_reg(DWC3_SSUSB_PHY_TX_OVRD_DRV_LO_REG,
			ipq_base);
		tmp &= ~DWC3_SSUSB_PHY_TX_DEEMPH_MASK;
		tmp |= (TX_DEEMPH_3_5DB << DWC3_SSUSB_PHY_TX_DEEMPH_SHIFT);
		tmp &= ~DWC3_SSUSB_PHY_AMP_MASK;
		tmp |= AMPLITUDE_VALUE;
		tmp |= DWC3_SSUSB_PHY_AMP_EN;
		dwc3_ipq_ssusb_write_phy_reg(DWC3_SSUSB_PHY_TX_OVRD_DRV_LO_REG,
						tmp, ipq_base);
	}
	ipq_ssusb_clear_and_set_bits32(IPQ_SS_PHY_PARAM_CTRL_1_REG,
					0x7, 0x5, ipq_base);
	/* XHCI REV */
	writel((1 << 2), ipq_base + IPQ_QSCRATCH_GENERAL_CFG);
	writel(0x0c80c010, ipq_base + DWC3_SSUSB_REG_GUCTL);
	partial_rx_reset_init(ipq_base);
	clear_bits = 0;
	set_bits = 0;
	/* Test  U2EXIT_LFPS */
	set_bits |= IPQ_SSUSB_REG_GCTL_U2EXIT_LFPS;
	ipq_ssusb_clear_and_set_bits32(DWC3_SSUSB_REG_GCTL, clear_bits,
					set_bits, ipq_base);
	set_bits = 0;
	set_bits |= IPQ_SSUSB_REG_GCTL_U2RSTECN;
	set_bits |= IPQ_SSUSB_REG_GCTL_U2EXIT_LFPS;
	ipq_ssusb_clear_and_set_bits32(DWC3_SSUSB_REG_GCTL, clear_bits,
					set_bits, ipq_base);

	writel(DWC3_GCTL_U2EXIT_LFPS | DWC3_GCTL_SOFITPSYNC |
		DWC3_GCTL_PRTCAPDIR(1) |
		DWC3_GCTL_U2RSTECN | DWC3_GCTL_PWRDNSCALE(2),
		ipq_base + DWC3_GCTL);
	writel((IPQ_SSUSB_QSCRATCH_SS_PHY_CTRL_MPLL_MULTI(0x19) |
		IPQ_SSUSB_QSCRATCH_SS_PHY_CTRL_REF_SS_PHY_EN |
		IPQ_SSUSB_QSCRATCH_SS_PHY_CTRL_LANE0_PWR_PRESENT),
		ipq_base + IPQ_SS_PHY_CTRL_REG);
	writel((DWC3_SSUSB_REG_GUSB2PHYCFG_SUSPENDUSB20 |
		DWC3_SSUSB_REG_GUSB2PHYCFG_ENBLSLPM |
		DWC3_SSUSB_REG_GUSB2PHYCFG_USBTRDTIM(9)),
		ipq_base + DWC3_SSUSB_REG_GUSB2PHYCFG(0));
	writel(DWC3_SSUSB_REG_GUSB3PIPECTL_ELASTIC_BUFFER_MODE |
		DWC3_SSUSB_REG_GUSB3PIPECTL_TX_DE_EPPHASIS(1) |
		DWC3_SSUSB_REG_GUSB3PIPECTL_TX_MARGIN(0)|
		DWC3_SSUSB_REG_GUSB3PIPECTL_DELAYP1TRANS |
		DWC3_SSUSB_REG_GUSB3PIPECTL_DELAYP1P2P3(1) |
		DWC3_SSUSB_REG_GUSB3PIPECTL_U1U2EXITFAIL_TO_RECOV |
		DWC3_SSUSB_REG_GUSB3PIPECTL_REQUEST_P1P2P3,
		ipq_base + DWC3_SSUSB_REG_GUSB3PIPECTL(0));
	writel(IPQ_SSUSB_REG_QSCRATCH_SS_PHY_PARAM_CTRL_1_LOS_LEVEL(0x9) |
		IPQ_SSUSB_REG_QSCRATCH_SS_PHY_PARAM_CTRL_1_TX_DEEMPH_3_5DB(0x15) |
		IPQ_SSUSB_REG_QSCRATCH_SS_PHY_PARAM_CTRL_1_TX_DEEMPH_6DB(0x20) |
		IPQ_SSUSB_REG_QSCRATCH_SS_PHY_PARAM_CTRL_1_TX_SWING_FULL(0x61),
		ipq_base + IPQ_SS_PHY_PARAM_CTRL_1_REG);
	writel(IPQ_SSUSB_REG_QSCRATCH_GENERAL_CFG_XHCI_REV(DWC3_SSUSB_XHCI_REV_10),
		ipq_base + IPQ_QSCRATCH_GENERAL_CFG);
}

static void usb30_common_pre_init(int id, void __iomem *ipq_base)
{
	void __iomem *reg;
	if (id == 0)
		reg = USB30_RESET;
	else
		reg = USB30_1_RESET;

	writel(IPQ_USB30_RESET_PHY_ASYNC_RESET, reg);
	writel(IPQ_USB30_RESET_POWERON_ASYNC_RESET |
		IPQ_USB30_RESET_PHY_ASYNC_RESET, reg);
	writel(IPQ_USB30_RESET_MOC_UTMI_ASYNC_RESET |
		IPQ_USB30_RESET_POWERON_ASYNC_RESET |
		IPQ_USB30_RESET_PHY_ASYNC_RESET, reg);
	writel(IPQ_USB30_RESET_SLEEP_ASYNC_RESET |
		IPQ_USB30_RESET_MOC_UTMI_ASYNC_RESET |
		IPQ_USB30_RESET_POWERON_ASYNC_RESET |
		IPQ_USB30_RESET_PHY_ASYNC_RESET, reg);
	writel(IPQ_USB30_RESET_MASTER_ASYNC_RESET |
		IPQ_USB30_RESET_SLEEP_ASYNC_RESET |
		IPQ_USB30_RESET_MOC_UTMI_ASYNC_RESET |
		IPQ_USB30_RESET_POWERON_ASYNC_RESET |
		IPQ_USB30_RESET_PHY_ASYNC_RESET, reg);
	if (id == 0) {
		writel(IPQ_USB30_RESET_PORT2_HS_PHY_ASYNC_RESET |
			IPQ_USB30_RESET_MASTER_ASYNC_RESET |
			IPQ_USB30_RESET_SLEEP_ASYNC_RESET |
			IPQ_USB30_RESET_MOC_UTMI_ASYNC_RESET |
			IPQ_USB30_RESET_POWERON_ASYNC_RESET |
			IPQ_USB30_RESET_PHY_ASYNC_RESET, reg);
	}
	udelay(5);
	writel(IPQ_USB30_RESET_MASK & ~(IPQ_USB30_RESET_PHY_ASYNC_RESET), reg);
	writel(IPQ_USB30_RESET_MASK & ~(IPQ_USB30_RESET_POWERON_ASYNC_RESET |
		IPQ_USB30_RESET_PHY_ASYNC_RESET), reg);
	writel(IPQ_USB30_RESET_MASK & ~(IPQ_USB30_RESET_MOC_UTMI_ASYNC_RESET |
		IPQ_USB30_RESET_POWERON_ASYNC_RESET |
		IPQ_USB30_RESET_PHY_ASYNC_RESET), reg);
	writel(IPQ_USB30_RESET_MASK & ~(IPQ_USB30_RESET_SLEEP_ASYNC_RESET |
		IPQ_USB30_RESET_MOC_UTMI_ASYNC_RESET |
		IPQ_USB30_RESET_POWERON_ASYNC_RESET |
		IPQ_USB30_RESET_PHY_ASYNC_RESET), reg);
	writel(IPQ_USB30_RESET_MASK & ~(IPQ_USB30_RESET_MASTER_ASYNC_RESET |
		IPQ_USB30_RESET_SLEEP_ASYNC_RESET|
		IPQ_USB30_RESET_MOC_UTMI_ASYNC_RESET |
		IPQ_USB30_RESET_POWERON_ASYNC_RESET |
		IPQ_USB30_RESET_PHY_ASYNC_RESET), reg);
	if (id == 0) {
		writel(IPQ_USB30_RESET_MASK &
			~(IPQ_USB30_RESET_PORT2_HS_PHY_ASYNC_RESET |
			IPQ_USB30_RESET_MASTER_ASYNC_RESET |
			IPQ_USB30_RESET_SLEEP_ASYNC_RESET |
			IPQ_USB30_RESET_MOC_UTMI_ASYNC_RESET |
			IPQ_USB30_RESET_POWERON_ASYNC_RESET |
			IPQ_USB30_RESET_PHY_ASYNC_RESET), reg);
		reg = ioremap(IPQ_TCSR_USB_CONTROLLER_TYPE_SEL, 4);
		if (reg) {
			writel(0x3, reg);
			iounmap(reg);
		}
	}
	writel((IPQ_SSUSB_REG_QSCRATCH_CGCTL_RAM1112_EN |
		IPQ_SSUSB_REG_QSCRATCH_CGCTL_RAM13_EN),
		ipq_base + IPQ_SSUSB_REG_QSCRATCH_CGCTL);
	writel((IPQ_SSUSB_REG_QSCRATCH_RAM1_RAM13_EN |
		IPQ_SSUSB_REG_QSCRATCH_RAM1RAM12_EN |
		IPQ_SSUSB_REG_QSCRATCH_RAM1_RAM11_EN),
		ipq_base + IPQ_SSUSB_REG_QSCRATCH_RAM1);
}


/**
 *
 * Read register with debug info.
 *
 * @base - DWC3 base virtual address.
 * @offset - register offset.
 *
 * @return u32
 */
static inline u32 dwc3_ipq_read_reg(void *base, u32 offset)
{
	u32 val = ioread32(base + offset);
	return val;
}

/**
 * Read register masked field with debug info.
 *
 * @base - DWC3 base virtual address.
 * @offset - register offset.
 * @mask - register bitmask.
 *
 * @return u32
 */
static inline u32 dwc3_ipq_read_reg_field(void *base,
					  u32 offset,
					  const u32 mask)
{
	u32 shift = find_first_bit((void *)&mask, 32);
	u32 val = ioread32(base + offset);
	val &= mask;		/* clear other bits */
	val >>= shift;
	return val;
}

/**
 *
 * Write register with debug info.
 *
 * @base - DWC3 base virtual address.
 * @offset - register offset.
 * @val - value to write.
 *
 */
static inline void dwc3_ipq_write_reg(void *base, u32 offset, u32 val)
{
	iowrite32(val, base + offset);
}

/**
 * Write register masked field with debug info.
 *
 * @base - DWC3 base virtual address.
 * @offset - register offset.
 * @mask - register bitmask.
 * @val - value to write.
 *
 */
static inline void dwc3_ipq_write_reg_field(void *base, u32 offset,
					    const u32 mask, u32 val)
{
	u32 shift = find_first_bit((void *)&mask, 32);
	u32 tmp = ioread32(base + offset);

	tmp &= ~mask;		/* clear written bits */
	val = tmp | (val << shift);
	iowrite32(val, base + offset);
}

/**
 * Write register and read back masked value to confirm it is written
 *
 * @base - DWC3 base virtual address.
 * @offset - register offset.
 * @mask - register bitmask specifying what should be updated
 * @val - value to write.
 *
 */
static inline void dwc3_ipq_write_readback(void *base, u32 offset,
					    const u32 mask, u32 val)
{
	u32 write_val, tmp = ioread32(base + offset);

	tmp &= ~mask;		/* retain other bits */
	write_val = tmp | val;

	iowrite32(write_val, base + offset);

	/* Read back to see if val was written */
	tmp = ioread32(base + offset);
	tmp &= mask;		/* clear other bits */

	if (tmp != val)
		dev_err(context->dev, "%s: write: %x to QSCRATCH: %x FAILED\n",
						__func__, val, offset);
}

/**
 * Configure the DBM with the USB3 core event buffer.
 * This function is called by the SNPS UDC upon initialization.
 *
 * @addr - address of the event buffer.
 * @size - size of the event buffer.
 *
 */
static int dwc3_ipq_event_buffer_config(u32 addr, u16 size)
{
	dev_dbg(context->dev, "%s\n", __func__);

	dwc3_ipq_write_reg(context->base, DBM_GEVNTADR, addr);
	dwc3_ipq_write_reg_field(context->base, DBM_GEVNTSIZ,
		DBM_GEVNTSIZ_MASK, size);

	return 0;
}

/**
 * Reset the DBM registers upon initialization.
 *
 */
static int dwc3_ipq_dbm_soft_reset(int enter_reset)
{
	dev_dbg(context->dev, "%s\n", __func__);
	if (enter_reset) {
		dev_dbg(context->dev, "enter DBM reset\n");
		dwc3_ipq_write_reg_field(context->base, DBM_SOFT_RESET,
			DBM_SFT_RST_MASK, 1);
	} else {
		dev_dbg(context->dev, "exit DBM reset\n");
		dwc3_ipq_write_reg_field(context->base, DBM_SOFT_RESET,
			DBM_SFT_RST_MASK, 0);
		/*enable DBM*/
		dwc3_ipq_write_reg_field(context->base, IPQ_QSCRATCH_GENERAL_CFG,
			DBM_EN_MASK, 0x1);
	}

	return 0;
}

/**
 * Configure the DBM with the BAM's data fifo.
 * This function is called by the USB BAM Driver
 * upon initialization.
 *
 * @ep - pointer to usb endpoint.
 * @addr - address of data fifo.
 * @size - size of data fifo.
 *
 */
int ipq_data_fifo_config(struct usb_ep *ep, u32 addr, u32 size, u8 dst_pipe_idx)
{
	u8 dbm_ep;
	struct dwc3_ep *dep = to_dwc3_ep(ep);
	u8 bam_pipe = dst_pipe_idx;

	dev_dbg(context->dev, "%s\n", __func__);

	dbm_ep = bam_pipe;
	context->ep_num_mapping[dbm_ep] = dep->number;

	dwc3_ipq_write_reg(context->base, DBM_DATA_FIFO(dbm_ep), addr);
	dwc3_ipq_write_reg_field(context->base, DBM_DATA_FIFO_SIZE(dbm_ep),
		DBM_DATA_FIFO_SIZE_MASK, size);

	return 0;
}

static void dwc3_chg_enable_secondary_det(struct dwc3_ipq *mdwc)
{
	u32 chg_ctrl;

	/* Turn off VDP_SRC */
	dwc3_ipq_write_reg(mdwc->base, IPQ_CHARGING_DET_CTRL_REG, 0x0);
	msleep(20);

	/* Before proceeding make sure VDP_SRC is OFF */
	chg_ctrl = dwc3_ipq_read_reg(mdwc->base, IPQ_CHARGING_DET_CTRL_REG);
	if (chg_ctrl & 0x3F)
		dev_err(mdwc->dev, "%s Unable to reset chg_det block: %x\n",
						 __func__, chg_ctrl);
	/*
	 * Configure DM as current source, DP as current sink
	 * and enable battery charging comparators.
	 */
	dwc3_ipq_write_readback(mdwc->base, IPQ_CHARGING_DET_CTRL_REG, 0x3F, 0x34);
}

static bool dwc3_chg_det_check_output(struct dwc3_ipq *mdwc)
{
	u32 chg_det;
	bool ret = false;

	chg_det = dwc3_ipq_read_reg(mdwc->base, IPQ_CHARGING_DET_OUTPUT_REG);
	ret = chg_det & 1;

	return ret;
}

static void dwc3_chg_enable_primary_det(struct dwc3_ipq *mdwc)
{
	/*
	 * Configure DP as current source, DM as current sink
	 * and enable battery charging comparators.
	 */
	dwc3_ipq_write_readback(mdwc->base, IPQ_CHARGING_DET_CTRL_REG, 0x3F, 0x30);
}

static inline bool dwc3_chg_check_dcd(struct dwc3_ipq *mdwc)
{
	u32 chg_state;
	bool ret = false;

	chg_state = dwc3_ipq_read_reg(mdwc->base, IPQ_CHARGING_DET_OUTPUT_REG);
	ret = chg_state & 2;

	return ret;
}

static inline void dwc3_chg_disable_dcd(struct dwc3_ipq *mdwc)
{
	dwc3_ipq_write_readback(mdwc->base, IPQ_CHARGING_DET_CTRL_REG, 0x3F, 0x0);
}

static inline void dwc3_chg_enable_dcd(struct dwc3_ipq *mdwc)
{
	/* Data contact detection enable, DCDENB */
	dwc3_ipq_write_readback(mdwc->base, IPQ_CHARGING_DET_CTRL_REG, 0x3F, 0x2);
}

static void dwc3_chg_block_reset(struct dwc3_ipq *mdwc)
{
	u32 chg_ctrl;

	/* Clear charger detecting control bits */
	dwc3_ipq_write_reg(mdwc->base, IPQ_CHARGING_DET_CTRL_REG, 0x0);

	/* Clear alt interrupt latch and enable bits */
	dwc3_ipq_write_reg(mdwc->base, IPQ_HS_PHY_IRQ_STAT_REG, 0xFFF);
	dwc3_ipq_write_reg(mdwc->base, IPQ_ALT_INTERRUPT_EN_REG, 0x0);

	udelay(100);

	/* Before proceeding make sure charger block is RESET */
	chg_ctrl = dwc3_ipq_read_reg(mdwc->base, IPQ_CHARGING_DET_CTRL_REG);
	if (chg_ctrl & 0x3F)
		dev_err(mdwc->dev, "%s Unable to reset chg_det block: %x\n",
						 __func__, chg_ctrl);
}

#define DWC3_CHG_DCD_POLL_TIME		(100 * HZ/1000) /* 100 msec */
#define DWC3_CHG_DCD_MAX_RETRIES	6 /* Tdcd_tmout = 6 * 100 msec */
#define DWC3_CHG_PRIMARY_DET_TIME	(50 * HZ/1000) /* TVDPSRC_ON */
#define DWC3_CHG_SECONDARY_DET_TIME	(50 * HZ/1000) /* TVDMSRC_ON */

static void dwc3_chg_detect_work(struct work_struct *w)
{
	struct dwc3_ipq *mdwc = container_of(w, struct dwc3_ipq, chg_work.work);
	bool is_dcd = false, tmout, vout;
	unsigned long delay;

	dev_dbg(mdwc->dev, "chg detection work\n");
	switch (mdwc->chg_state) {
	case USB_CHG_STATE_UNDEFINED:
		dwc3_chg_block_reset(mdwc);
		dwc3_chg_enable_dcd(mdwc);
		mdwc->chg_state = USB_CHG_STATE_WAIT_FOR_DCD;
		mdwc->dcd_retries = 0;
		delay = DWC3_CHG_DCD_POLL_TIME;
		break;
	case USB_CHG_STATE_WAIT_FOR_DCD:
		is_dcd = dwc3_chg_check_dcd(mdwc);
		tmout = ++mdwc->dcd_retries == DWC3_CHG_DCD_MAX_RETRIES;
		if (is_dcd || tmout) {
			dwc3_chg_disable_dcd(mdwc);
			dwc3_chg_enable_primary_det(mdwc);
			delay = DWC3_CHG_PRIMARY_DET_TIME;
			mdwc->chg_state = USB_CHG_STATE_DCD_DONE;
		} else {
			delay = DWC3_CHG_DCD_POLL_TIME;
		}
		break;
	case USB_CHG_STATE_DCD_DONE:
		vout = dwc3_chg_det_check_output(mdwc);
		if (vout) {
			dwc3_chg_enable_secondary_det(mdwc);
			delay = DWC3_CHG_SECONDARY_DET_TIME;
			mdwc->chg_state = USB_CHG_STATE_PRIMARY_DONE;
		} else {
			mdwc->chg_state = USB_CHG_STATE_DETECTED;
			delay = 0;
		}
		break;
	case USB_CHG_STATE_PRIMARY_DONE:
		vout = dwc3_chg_det_check_output(mdwc);
		mdwc->chg_state = USB_CHG_STATE_SECONDARY_DONE;
		/* fall through */
	case USB_CHG_STATE_SECONDARY_DONE:
		mdwc->chg_state = USB_CHG_STATE_DETECTED;
		/* fall through */
	case USB_CHG_STATE_DETECTED:
		dwc3_chg_block_reset(mdwc);
		return;
	default:
		return;
	}

	queue_delayed_work(system_nrt_wq, &mdwc->chg_work, delay);
}

static void dwc3_resume_work(struct work_struct *w)
{
	struct dwc3_ipq *mdwc = container_of(w, struct dwc3_ipq,
							resume_work.work);

	dev_dbg(mdwc->dev, "%s: dwc3 resume work\n", __func__);
	/* handle any event that was queued while work was already running */
	if (!atomic_read(&mdwc->in_lpm)) {
		dev_dbg(mdwc->dev, "%s: notifying xceiv event\n", __func__);
		return;
	}

	/* bail out if system resume in process, else initiate RESUME */
	if (atomic_read(&mdwc->pm_suspended)) {
		mdwc->resume_pending = true;
	} else {
		pm_runtime_get_sync(mdwc->dev);
		pm_runtime_put_sync(mdwc->dev);
	}
}

static bool debug_id, debug_bsv, debug_connect;

static int dwc3_connect_show(struct seq_file *s, void *unused)
{
	if (debug_connect)
		seq_printf(s, "true\n");
	else
		seq_printf(s, "false\n");

	return 0;
}

static int dwc3_connect_open(struct inode *inode, struct file *file)
{
	return single_open(file, dwc3_connect_show, inode->i_private);
}

static ssize_t dwc3_connect_write(struct file *file, const char __user *ubuf,
				size_t count, loff_t *ppos)
{
	struct seq_file *s = file->private_data;
	struct dwc3_ipq *mdwc = s->private;
	char buf[8];

	memset(buf, 0x00, sizeof(buf));

	if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;

	if (!strncmp(buf, "enable", 6) || !strncmp(buf, "true", 4)) {
		debug_connect = true;
	} else {
		debug_connect = debug_bsv = false;
		debug_id = true;
	}

	if (atomic_read(&mdwc->in_lpm)) {
		dev_dbg(mdwc->dev, "%s: calling resume_work\n", __func__);
		dwc3_resume_work(&mdwc->resume_work.work);
	} else {
		dev_dbg(mdwc->dev, "%s: notifying xceiv event\n", __func__);
	}

	return count;
}

const struct file_operations dwc3_connect_fops = {
	.open = dwc3_connect_open,
	.read = seq_read,
	.write = dwc3_connect_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct dentry *dwc3_debugfs_root;

static void dwc3_debugfs_init(struct dwc3_ipq *mdwc)
{
	dwc3_debugfs_root = debugfs_create_dir("ipq_dwc3", NULL);

	if (!dwc3_debugfs_root || IS_ERR(dwc3_debugfs_root))
		return;

	if (!debugfs_create_bool("id", S_IRUGO | S_IWUSR, dwc3_debugfs_root,
				 (u32 *)&debug_id))
		goto error;

	if (!debugfs_create_bool("bsv", S_IRUGO | S_IWUSR, dwc3_debugfs_root,
				 (u32 *)&debug_bsv))
		goto error;

	if (!debugfs_create_file("connect", S_IRUGO | S_IWUSR,
				dwc3_debugfs_root, mdwc, &dwc3_connect_fops))
		goto error;

	return;

error:
	debugfs_remove_recursive(dwc3_debugfs_root);
}

static int __devinit dwc3_ipq_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	struct platform_device *dwc3;
	struct dwc3_ipq *ipq;
	struct resource *res;
	void __iomem *tcsr;
	int ret = 0;
	struct dwc3_platform_data *pd = pdev->dev.platform_data;
	ipq = devm_kzalloc(&pdev->dev, sizeof(*ipq), GFP_KERNEL);
	if (!ipq) {
		dev_err(&pdev->dev, "not enough memory\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, ipq);
	context = ipq;
	ipq->dev = &pdev->dev;
	INIT_LIST_HEAD(&ipq->req_complete_list);
	INIT_DELAYED_WORK(&ipq->chg_work, dwc3_chg_detect_work);
	INIT_DELAYED_WORK(&ipq->resume_work, dwc3_resume_work);

	/* Initialize clocks */
	if (pdev->id == 0) {
		ipq->core_clk = clk_get(&pdev->dev, "core_clk");
		if (IS_ERR(ipq->core_clk)) {
			dev_err(&pdev->dev, "failed to get core_clk\n");
			return PTR_ERR(ipq->core_clk);
		}
		clk_set_rate(ipq->core_clk, 125000000);
		clk_prepare_enable(ipq->core_clk);

		ipq->iface0_clk = clk_get(&pdev->dev, "iface0_clk");
		if (IS_ERR(ipq->iface0_clk)) {
			dev_err(&pdev->dev, "failed to get iface0_clk\n");
			ret = PTR_ERR(ipq->iface0_clk);
			goto disable_core_clk;
		}
		clk_prepare_enable(ipq->iface0_clk);

		ipq->iface1_clk = clk_get(&pdev->dev, "iface1_clk");
		if (IS_ERR(ipq->iface1_clk)) {
			dev_err(&pdev->dev, "failed to get iface1_clk\n");
			ret = PTR_ERR(ipq->iface1_clk);
			goto disable_iface0_clk;
		}
		clk_prepare_enable(ipq->iface1_clk);

		ipq->utmi_clk = clk_get(&pdev->dev, "utmi_clk");
		if (IS_ERR(ipq->utmi_clk)) {
			dev_err(&pdev->dev, "failed to get utmi_clk\n");
			goto disable_iface1_clk;
		}
		clk_set_rate(ipq->utmi_clk, 60000000);
		clk_prepare_enable(ipq->utmi_clk);

		ipq->utmi_b0_clk = clk_get(&pdev->dev, "utmi_b0_clk");
		if (IS_ERR(ipq->utmi_b0_clk)) {
			dev_err(&pdev->dev, "failed to get utmi_b0_clk \n");
			ret = PTR_ERR(ipq->utmi_b0_clk);
			goto disable_utmi_clk;
		}
		clk_prepare_enable(ipq->utmi_b0_clk);

		ipq->utmi_b1_clk = clk_get(&pdev->dev, "utmi_b1_clk");
		if (IS_ERR(ipq->utmi_b1_clk)) {
			dev_err(&pdev->dev, "failed to get utmi_b0_clk\n");
			ret = PTR_ERR(ipq->utmi_b1_clk);
			goto disable_utmi_b0__clk;
		}
		clk_prepare_enable(ipq->utmi_b1_clk);
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!res) {
		dev_dbg(&pdev->dev, "missing TCSR memory resource\n");
	} else {
		tcsr = devm_ioremap_nocache(&pdev->dev, res->start,
			resource_size(res));
		if (!tcsr) {
			dev_err(&pdev->dev, "tcsr ioremap failed\n");
			ret = -ENODEV;
			return ret;
		} else {
			/* Enable USB3 on the primary USB port. */
			writel_relaxed(0x1, tcsr);
			/*
			 * Ensure that TCSR write is completed before
			 * USB registers initialization.
			 */
			mb();
		}
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "missing memory base resource\n");
		ret = -ENODEV;
		return ret;
	}

	ipq->base = devm_ioremap_nocache(&pdev->dev, res->start,
		resource_size(res));
	if (!ipq->base) {
		dev_err(&pdev->dev, "ioremap failed\n");
		ret = -ENODEV;
		return ret;
	}

	dwc3 = platform_device_alloc("dwc3", pdev->id);
	if (!dwc3) {
		dev_err(&pdev->dev, "couldn't allocate dwc3 device\n");
		ret = -ENODEV;
		return ret;
	}

	dwc3->dev.parent = &pdev->dev;
	dwc3->dev.coherent_dma_mask = DMA_BIT_MASK(32);
	dwc3->dev.dma_mask = &dwc3_ipq_dma_mask;
	dwc3->dev.dma_parms = pdev->dev.dma_parms;
	dwc3->dev.id = pdev->id;
	ipq->resource_size = resource_size(res);
	ipq->dwc3 = dwc3;
	ipq->mode = pd->usb_mode;
	dwc3->dev.platform_data = &ipq->mode;
	if(pdev->id == 0)
		usb30_common_pre_init(pdev->id, ipq->base);
	uw_ssusb_pre_init(ipq->base);
	if (pd->pwr_en) {
		writel((0 << IPQ_GPIO_IN_OUTn_GPIO_OUT_SHFT),
			(IPQ_GPIO_IN_OUTn(pd->pwr_en_gpio1)));
		writel((0 << IPQ_GPIO_IN_OUTn_GPIO_OUT_SHFT),
			(IPQ_GPIO_IN_OUTn(pd->pwr_en_gpio2)));
	}
	pm_runtime_set_active(ipq->dev);

	if (of_property_read_u32(node, "qcom,dwc-usb3-ipq-dbm-eps",
				 &ipq->dbm_num_eps)) {
		dev_err(&pdev->dev,
			"unable to read platform data num of dbm eps\n");
		ipq->dbm_num_eps = DBM_MAX_EPS;
	}

	if (ipq->dbm_num_eps > DBM_MAX_EPS) {
		dev_err(&pdev->dev,
			"Driver doesn't support number of DBM EPs. "
			"max: %d, dbm_num_eps: %d\n",
			DBM_MAX_EPS, ipq->dbm_num_eps);
		ret = -ENODEV;
		goto put_pdev;
	}

	ret = platform_device_add_resources(dwc3, pdev->resource,
		pdev->num_resources);
	if (ret) {
		dev_err(&pdev->dev, "couldn't add resources to dwc3 device\n");
		goto put_pdev;
	}

	ret = platform_device_add(dwc3);
	if (ret) {
		dev_err(&pdev->dev, "failed to register dwc3 device\n");
		goto put_pdev;
	}

	/* Reset the DBM */
	dwc3_ipq_dbm_soft_reset(1);
	usleep_range(1000, 1200);
	dwc3_ipq_dbm_soft_reset(0);
	dwc3_ipq_event_buffer_config(dwc3_readl(ipq->base, DWC3_GEVNTADRLO(0)),
		dwc3_readl(ipq->base, DWC3_GEVNTSIZ(0)));
	dwc3_debugfs_init(ipq);

	return 0;

put_pdev:
	platform_device_put(dwc3);
disable_utmi_b0__clk:
	clk_disable_unprepare(ipq->utmi_b0_clk);
disable_utmi_clk:
	clk_disable_unprepare(ipq->utmi_clk);
disable_iface1_clk:
	clk_disable_unprepare(ipq->iface1_clk);
disable_iface0_clk:
	clk_disable_unprepare(ipq->iface0_clk);
disable_core_clk:
	clk_disable_unprepare(ipq->core_clk);
	return ret;
}

static int __devexit dwc3_ipq_remove(struct platform_device *pdev)
{
	struct dwc3_ipq	*ipq = platform_get_drvdata(pdev);

	if (dwc3_debugfs_root)
		debugfs_remove_recursive(dwc3_debugfs_root);
	pm_runtime_disable(ipq->dev);
	platform_device_unregister(ipq->dwc3);

	return 0;
}

static const struct of_device_id of_dwc3_matach[] = {
	{
		.compatible = "qcom,dwc-usb3-ipq",
	},
	{ },
};
MODULE_DEVICE_TABLE(of, of_dwc3_matach);

static struct platform_driver dwc3_ipq_driver = {
	.probe		= dwc3_ipq_probe,
	.remove		= __devexit_p(dwc3_ipq_remove),
	.driver		= {
		.name	= "ipq-dwc3",
		.of_match_table	= of_dwc3_matach,
	},
};

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("DesignWare USB3 IPQ Glue Layer");

static int __devinit dwc3_ipq_init(void)
{
	return platform_driver_register(&dwc3_ipq_driver);
}
module_init(dwc3_ipq_init);

static void __exit dwc3_ipq_exit(void)
{
	platform_driver_unregister(&dwc3_ipq_driver);
}

module_exit(dwc3_ipq_exit);
