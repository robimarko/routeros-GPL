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
 * MSM PCIe controller IRQ driver.
 */

#include <linux/bitops.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/msi.h>
#include <linux/pci.h>
#include <linux/irqdomain.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include "qcom_pcie.h"

/* Any address will do here, as it won't be dereferenced */
#define MSM_PCIE_MSI_PHY 0xa0000000

#define PCIE20_MSI_CTRL_ADDR            (0x820)
#define PCIE20_MSI_CTRL_UPPER_ADDR      (0x824)
#define PCIE20_MSI_CTRL_INTR_EN         (0x828)
#define PCIE20_MSI_CTRL_INTR_MASK       (0x82C)
#define PCIE20_MSI_CTRL_INTR_STATUS     (0x830)

#define PCIE20_MSI_CTRL_MAX 8

static DECLARE_BITMAP(msi_irq_in_use, NR_PCIE_MSI_IRQS);

static irqreturn_t handle_msi_irq(int irq, void *data)
{
	int i, j;
	unsigned long val;
	struct msm_pcie_dev_t *dev = data;
	void __iomem *ctrl_status;

	PCIE_DBG(dev, "irq=%d\n", irq);

	/* check for set bits, clear it by setting that bit
	   and trigger corresponding irq */
	for (i = 0; i < PCIE20_MSI_CTRL_MAX; i++) {
		ctrl_status = dev->dm_core +
				PCIE20_MSI_CTRL_INTR_STATUS + (i * 12);

		val = readl_relaxed(ctrl_status);
		while (val) {
			j = find_first_bit(&val, 32);
			writel_relaxed(BIT(j), ctrl_status);
			/* ensure that interrupt is cleared (acked) */
			wmb();

			generic_handle_irq(MSM_PCIE_MSI_INT(j + (32 * i)));
			val = readl_relaxed(ctrl_status);
		}
	}

	return IRQ_HANDLED;
}

void ipq40xx_teardown_msi_irq(unsigned int irq)
{
	int pos = irq - MSM_PCIE_MSI_INT(0);

	dynamic_irq_cleanup(irq);
	clear_bit(pos, msi_irq_in_use);
}

static int msm_pcie_create_irq(struct pci_dev *pdev)
{
	int irq, pos;

again:
	pos = find_first_zero_bit(msi_irq_in_use, NR_PCIE_MSI_IRQS);
	/*
	 * MSI IRQs are assigned at the end of the list (of all IRQs).
	 * We know that RC takes even numbered bus and EP takes
	 * odd numbered bus. We need MSI IRQs for the EPs. Allot
	 * a bunch of 32 IRQs for each EP.
	 */
	pos = (pdev->bus->number / 2) * 32 + pos;
	irq = MSM_PCIE_MSI_INT(pos);
	if (irq >= (MSM_PCIE_MSI_INT(0) + NR_PCIE_MSI_IRQS))
		return -ENOSPC;

	if (test_and_set_bit(pos, msi_irq_in_use))
		goto again;

	dynamic_irq_init(irq);
	return irq;
}

int ipq40xx_setup_msi_irq(struct pci_dev *pdev, struct msi_desc *desc)
{
	struct msm_pcie_dev_t *dev = PCIE_BUS_PRIV_DATA(pdev);
	int irq;
	struct msi_msg msg;

	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);

	irq = msm_pcie_create_irq(pdev);

	PCIE_DBG(dev, "IRQ %d is allocated.\n", irq);

	if (irq < 0)
		return irq;

	PCIE_DBG(dev, "irq %d allocated\n", irq);

	irq_set_msi_desc(irq, desc);

	/* write msi vector and data */
	msg.address_hi = 0;
	msg.address_lo = MSM_PCIE_MSI_PHY;
	msg.data = irq - MSM_PCIE_MSI_INT(0);
	write_msi_msg(irq, &msg);
	return 0;
}

int32_t ipq40xx_irq_init(struct msm_pcie_dev_t *dev)
{
	int rc;
	struct device *pdev = &dev->pdev->dev;

	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);

	/* register handler for physical MSI interrupt line */
	rc = devm_request_irq(pdev,
		dev->irq[MSM_PCIE_INT_MSI].num, handle_msi_irq,
		IRQF_TRIGGER_RISING, dev->irq[MSM_PCIE_INT_MSI].name, dev);
	if (rc) {
		PCIE_ERR(dev, "PCIe: RC%d: Unable to request MSI interrupt\n",
			dev->rc_idx);
		return rc;
	}

	return 0;
}
void msm_pcie_config_msi_controller(struct msm_pcie_dev_t *dev)
{
	int i;

	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);

	/* program MSI controller and enable all interrupts */
	writel_relaxed(MSM_PCIE_MSI_PHY, dev->dm_core + PCIE20_MSI_CTRL_ADDR);
	writel_relaxed(0, dev->dm_core + PCIE20_MSI_CTRL_UPPER_ADDR);

	for (i = 0; i < PCIE20_MSI_CTRL_MAX; i++)
		writel_relaxed(~0, dev->dm_core +
			       PCIE20_MSI_CTRL_INTR_EN + (i * 12));

	/* ensure that hardware is configured before proceeding */
	wmb();
}
