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

#ifndef __ARCH_ARM_MACH_MSM_PCIE_H
#define __ARCH_ARM_MACH_MSM_PCIE_H

#include <linux/clk.h>
#include <linux/compiler.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/types.h>
#include <linux/pm_wakeup.h>
#include <linux/pci.h>

enum msm_pcie_config {
	MSM_PCIE_CONFIG_INVALID = 0,
	MSM_PCIE_CONFIG_NO_CFG_RESTORE = 0x1,
	MSM_PCIE_CONFIG_LINKDOWN = 0x2,
	MSM_PCIE_CONFIG_NO_RECOVERY = 0x4,
};

enum msm_pcie_pm_opt {
	MSM_PCIE_SUSPEND,
	MSM_PCIE_RESUME,
	MSM_PCIE_REQ_EXIT_L1,
};

enum msm_pcie_event {
	MSM_PCIE_EVENT_INVALID = 0,
	MSM_PCIE_EVENT_LINKDOWN = 0x1,
	MSM_PCIE_EVENT_LINKUP = 0x2,
	MSM_PCIE_EVENT_WAKEUP = 0x4,
	MSM_PCIE_EVENT_WAKE_RECOVERY = 0x8,
	MSM_PCIE_EVENT_NO_ACCESS = 0x10,
};

enum msm_pcie_trigger {
	MSM_PCIE_TRIGGER_CALLBACK,
	MSM_PCIE_TRIGGER_COMPLETION,
};

struct msm_pcie_notify {
	enum msm_pcie_event event;
	void *user;
	void *data;
	u32 options;
};

#define MAX_RC_NUM 1

#ifdef CONFIG_ARM_LPAE
#define PCIE_UPPER_ADDR(addr) ((u32)((addr) >> 32))
#else
#define PCIE_UPPER_ADDR(addr) (0x0)
#endif
#define PCIE_LOWER_ADDR(addr) ((u32)((addr) & 0xffffffff))

#define PCIE_MSI_NR_IRQS 256

#define PCIE_LOG_PAGES (50)

#define PCIE_DBG(dev, fmt, arg...) do {			 \
	if (0)   \
		pr_alert("%s: " fmt, __func__, arg);              \
	} while (0)

#define PCIE_DBG2(dev, fmt, arg...) do {			 \
	if (0)   \
		pr_alert("%s: " fmt, __func__, arg);              \
	} while (0)

#define PCIE_DBG3(dev, fmt, arg...) do {			 \
	if (0)   \
		pr_alert("%s: " fmt, __func__, arg);              \
	} while (0)

#define PCIE_INFO(dev, fmt, arg...) do {			 \
	pr_info("%s: " fmt, __func__, arg);  \
	} while (0)

#define PCIE_ERR(dev, fmt, arg...) do {			 \
	pr_err("%s: " fmt, __func__, arg);  \
	} while (0)

#define PCIE_BUS_PRIV_DATA(pdev) \
	(((struct pci_sys_data *)pdev->bus->sysdata)->private_data)

/* PM control options */
#define PM_IRQ                   0x1
#define PM_CLK                   0x2
#define PM_GPIO                  0x4
#define PM_VREG                  0x8
#define PM_PIPE_CLK              0x10
#define PM_ALL (PM_IRQ | PM_CLK | PM_GPIO | PM_VREG | PM_PIPE_CLK)

#define PCIE_CONF_SPACE_DW		      1024
#define PCIE_CLEAR			      0xDEADBEEF
#define PCIE_LINK_DOWN                        0xFFFFFFFF

enum msm_pcie_res {
	MSM_PCIE_RES_PARF,
	MSM_PCIE_RES_PHY,
	MSM_PCIE_RES_DM_CORE,
	MSM_PCIE_RES_ELBI,
	MSM_PCIE_RES_CONF,
	MSM_PCIE_RES_IO,
	MSM_PCIE_RES_BARS,
	MSM_PCIE_MAX_RES,
};

enum msm_pcie_irq {
	MSM_PCIE_INT_MSI,
	MSM_PCIE_INT_A,
	MSM_PCIE_INT_B,
	MSM_PCIE_INT_C,
	MSM_PCIE_INT_D,
	MSM_PCIE_INT_PLS_PME,
	MSM_PCIE_INT_PME_LEGACY,
	MSM_PCIE_INT_PLS_ERR,
	MSM_PCIE_INT_AER_LEGACY,
	MSM_PCIE_INT_LINK_UP,
	MSM_PCIE_INT_LINK_DOWN,
	MSM_PCIE_INT_BRIDGE_FLUSH_N,
	MSM_PCIE_INT_WAKE,
	MSM_PCIE_MAX_IRQ,
};

enum msm_pcie_link_status {
	MSM_PCIE_LINK_DEINIT,
	MSM_PCIE_LINK_ENABLED,
	MSM_PCIE_LINK_DISABLED
};

/* resource info structure */
struct msm_pcie_res_info_t {
	char            *name;
	struct resource *resource;
	void __iomem    *base;
};

/* irq info structrue */
struct msm_pcie_irq_info_t {
	char              *name;
	uint32_t          num;
};

struct msm_pci_devices_t {
	unsigned char *name;
	unsigned gpio;
	unsigned polarity;
};

/* msm pcie device structure */
struct msm_pcie_dev_t {
	struct platform_device       *pdev;
	struct pci_dev *dev;
	struct msi_chip *msi_chip;
	unsigned of_defined_device;
	struct msm_pci_devices_t *children;
	unsigned gpio_rst;
	unsigned gpio_booster;
	
//	unsigned gpio_booster_on;
	struct msm_pcie_res_info_t   res[MSM_PCIE_MAX_RES];
	struct msm_pcie_irq_info_t   irq[MSM_PCIE_MAX_IRQ];

	void __iomem                 *parf;
	void __iomem                 *phy;
	void __iomem                 *elbi;
	void __iomem                 *dm_core;
	void __iomem                 *va_dm_core;
	void __iomem                 *conf;
	void __iomem                 *bars;
	void __iomem                 *dev_io;

	uint32_t                      axi_bar_start;
	uint32_t                      axi_bar_end;

	struct resource               *dev_mem_res;
	struct resource               *dev_io_res;

	uint32_t                      wake_n;
	uint32_t                      parf_deemph;
	uint32_t                      parf_swing;

	bool                         cfg_access;
	spinlock_t                   cfg_lock;
	unsigned long                irqsave_flags;
	struct mutex                 setup_lock;

	bool                         use_msi;

	enum msm_pcie_link_status    link_status;
	bool                         user_suspend;
	struct pci_saved_state	     *saved_state;

	struct wakeup_source	     ws;

	uint32_t                     n_fts;
	uint32_t                     ep_latency;
	bool                         ep_wakeirq;

	uint32_t                     rc_idx;
	bool                         enumerated;
	struct work_struct	     handle_wake_work;
	struct mutex                 recovery_lock;
	spinlock_t                   linkdown_lock;
	spinlock_t                   wakeup_lock;
	ulong                        linkdown_counter;
	bool                         suspending;
	ulong                        wake_counter;
	ulong                        req_exit_l1_counter;
	u32			     ep_shadow[PCIE_CONF_SPACE_DW];
	u32                          rc_shadow[PCIE_CONF_SPACE_DW];
	bool                         shadow_en;
	bool                         power_on;
	void                         *ipc_log;
	void                         *ipc_log_long;
	struct pci_bus               *pci_bus;
};

extern void msm_pcie_config_msi_controller(struct msm_pcie_dev_t *dev);
extern int32_t ipq40xx_irq_init(struct msm_pcie_dev_t *dev);
#endif
