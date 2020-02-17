/* * Copyright (c) 2012 Qualcomm Atheros, Inc. * */
/*
 *  Copyright (C) 2002 ARM Ltd.
 *  All Rights Reserved
 *  Copyright (c) 2010-2012, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cpumask.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>

#include <asm/hardware/gic.h>
#include <asm/cacheflush.h>
#include <asm/cputype.h>
#include <asm/mach-types.h>
#include <asm/smp_plat.h>

#include <mach/socinfo.h>
#include <mach/msm_iomap.h>
#include <soc/qcom/scm.h>

#define SCM_BOOT_ADDR				0x1
#define SCM_FLAG_COLDBOOT_CPU1		0x01
#define SCM_FLAG_COLDBOOT_CPU2		0x08
#define SCM_FLAG_COLDBOOT_CPU3		0x20

#include "pm.h"

#define VDD_SC1_ARRAY_CLAMP_GFS_CTL 0x15A0
#define SCSS_CPU1CORE_RESET 0xD80
#define SCSS_DBG_STATUS_CORE_PWRDUP 0xE64

extern void secondary_startup(void);

static int __cpuinit krait_release_secondary(int cpu)
{
	void *base_ptr = ioremap_nocache(0x02088000 + (cpu * 0x10000), SZ_4K);
	if (!base_ptr)
		return -ENODEV;

	writel_relaxed(0x109, base_ptr+0x04);
	writel_relaxed(0x101, base_ptr+0x04);
	mb();
	ndelay(300);

	writel_relaxed(0x121, base_ptr+0x04);
	mb();
	udelay(2);

	writel_relaxed(0x120, base_ptr+0x04);
	mb();
	udelay(2);

	writel_relaxed(0x100, base_ptr+0x04);
	mb();
	udelay(100);

	writel_relaxed(0x180, base_ptr+0x04);
	mb();
	iounmap(base_ptr);
	return 0;
}

DEFINE_PER_CPU(int, cold_boot_done);
static int cold_boot_flags[] = {
	0,
	SCM_FLAG_COLDBOOT_CPU1,
	SCM_FLAG_COLDBOOT_CPU2,
	SCM_FLAG_COLDBOOT_CPU3,
};

static int scm_set_boot_addr(phys_addr_t addr, int flags)
{
	struct {
		unsigned int flags;
		phys_addr_t  addr;
	} cmd;

	cmd.addr = addr;
	cmd.flags = flags;
	return scm_call(SCM_SVC_BOOT, SCM_BOOT_ADDR,
			&cmd, sizeof(cmd), NULL, 0);
}

void ipq806x_boot_secondary(unsigned cpu)
{
	int ret;
	unsigned int flag = 0;

	pr_debug("Starting secondary CPU %d\n", cpu);

	/* Set preset_lpj to avoid subsequent lpj recalculations */
	preset_lpj = loops_per_jiffy;

	if (cpu > 0 && cpu < ARRAY_SIZE(cold_boot_flags))
		flag = cold_boot_flags[cpu];
	else
		__WARN();

	if (per_cpu(cold_boot_done, cpu) == false) {
		ret = scm_set_boot_addr(virt_to_phys(secondary_startup),
					flag);
		if (ret == 0)
			krait_release_secondary(cpu);
		else
			printk(KERN_DEBUG "Failed to set secondary core boot "
					  "address\n");
		per_cpu(cold_boot_done, cpu) = true;
	}

	/*
	 * Send the secondary CPU a soft interrupt, thereby causing
	 * the boot monitor to read the system wide flags register,
	 * and branch to the address found there.
	 */
	gic_raise_softirq(cpumask_of(cpu), 1);
}
