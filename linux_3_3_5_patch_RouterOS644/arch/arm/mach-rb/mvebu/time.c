/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.


********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
*******************************************************************************/

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/clockchips.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/mach/time.h>
//#include <mach/hardware.h>
#include <asm/localtimer.h>
#include <asm/sched_clock.h>

#include <linux/clk.h>
#include <linux/clockchips.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/export.h>

//#include "boardEnv/mvBoardEnvLib.h"
//#include "cpu/mvCpu.h"
//#include "irq.h"
//#include "time.h"

#ifdef CONFIG_SMP
static struct clock_event_device __percpu **msys_local_clockevent;
#endif

unsigned __iomem cntmr_regs_offset = 0;
unsigned __iomem lcl_timer_base = 0;
unsigned __iomem bridge_regs_base = 0;
void __iomem *wd_rstoutn_regs_offset = 0;
//#define MV_CNTMR_REGS_OFFSET			(0x20300)
//#define MV_MBUS_REGS_OFFSET			(0x20000)

EXPORT_SYMBOL(cntmr_regs_offset);

#define TIMER_CTRL		(cntmr_regs_offset + 0x0000)
#define TIMER_WD_RELOAD		(cntmr_regs_offset + 0x0020)
#define TIMER_WD_VAL		(cntmr_regs_offset + 0x0034)
#define TIMER_CAUSE		(cntmr_regs_offset + 0x0028)

#define TIMER_EN(x)		(0x0001 << (2 * x))
#define TIMER_RELOAD_EN(x)	(0x0002 << (2 * x))
#define TIMER_RELOAD(x)		(cntmr_regs_offset + 0x0010 + (8 * x))
#define TIMER_VAL(x)		(cntmr_regs_offset + 0x0014 + (8 * x))
#define INT_TIMER_CLR(x)	(~(1 << (8*x)))

//#define LCL_TIMER_BASE		(0x21000 | 0x40)
#define LCL_TIMER_CTRL		(lcl_timer_base + 0x0000)
#define LCL_TIMER0_EN		0x0001
#define LCL_TIMER0_RELOAD_EN	0x0002
#define LCL_TIMER1_EN		0x0004
#define LCL_TIMER1_RELOAD_EN	0x0008
#define LCL_TIMER0_RELOAD	(lcl_timer_base + 0x0010)
#define LCL_TIMER0_VAL		(lcl_timer_base + 0x0014)
#define LCL_TIMER1_RELOAD	(lcl_timer_base + 0x0018)
#define LCL_TIMER1_VAL		(lcl_timer_base + 0x001c)
#define LCL_TIMER_WD_RELOAD	(lcl_timer_base + 0x0020)
#define LCL_TIMER_WD_VAL	(lcl_timer_base + 0x0024)
#define LCL_TIMER_CAUSE		(lcl_timer_base + 0x0028)
#define LCL_INT_TIMER0_CLR	~(1 << 0)
#define LCL_INT_TIMER1_CLR	~(1 << 8)
#define LCL_TIMER_TURN_25MHZ	(1 << 11)

#define TIMER_TURN_25MHZ(x)	(1 << (11 + x))
//#define BRIDGE_CAUSE		(MV_MBUS_REGS_OFFSET | 0x0260)
//#define BRIDGE_MASK		(MV_MBUS_REGS_OFFSET | 0x10c4)
#define BRIDGE_MASK		(bridge_regs_base)
#define BRIDGE_INT_TIMER(x)	(1 << (24 + x))

#define IRQ_AURORA_TIMER0		5
#define IRQ_AURORA_TIMER1		6
#define IRQ_LOCALTIMER			IRQ_AURORA_TIMER0

#define MV_BOARD_REFCLK_25MHZ        25000000

#define MV_MEMIO32_READ(addr)           \
        ((*((volatile unsigned int*)(addr))))
#define MV_MEMIO32_WRITE(addr, data)    \
        ((*((volatile unsigned int*)(addr))) = ((unsigned int)(data)))
#define MV_REG_READ(offset)             \
        (MV_MEMIO32_READ((offset)))
#define MV_REG_WRITE(offset, val)    \
        MV_MEMIO32_WRITE(((offset)), (val));

/*
 * Number of timer ticks per jiffy.
 */
static u32 ticks_per_jiffy;
static unsigned int soc_timer_id;

static u32 notrace msys_clksrc_read32(void)
{
	u32 cyc = ~MV_REG_READ(TIMER_VAL(soc_timer_id));
	return cyc;
}

static void __init msys_setup_sched_clock(unsigned long tclk)
{
	setup_sched_clock(msys_clksrc_read32, 32, tclk);
}

/*
 * Clocksource handling.
 */
static cycle_t msys_clksrc_read(struct clocksource *cs)
{
	return 0xffffffff - MV_REG_READ(TIMER_VAL(soc_timer_id));
}

static struct clocksource msys_clksrc = {
	.name		= "msys_clocksource",
	.shift		= 20,
	.rating		= 300,
	.read		= msys_clksrc_read,
	.mask		= CLOCKSOURCE_MASK(32),
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

void armada_370_xp_irq_mask(struct irq_data *d);
void armada_370_xp_irq_unmask(struct irq_data *d);

/*
 * Clockevent handling.
 */
int msys_clkevt_next_event(unsigned long delta, struct clock_event_device *dev)
{
	unsigned long flags;
	u32 u;

	if (delta == 0)
		return -ETIME;

	local_irq_save(flags);

	/* Clear and enable clockevent timer interrupt */
	MV_REG_WRITE(LCL_TIMER_CAUSE, LCL_INT_TIMER0_CLR);

	/*msys_irq_unmask(IRQ_LOCALTIMER);*/
	armada_370_xp_irq_unmask(irq_get_irq_data(IRQ_LOCALTIMER));

	/* Setup new clockevent timer value */
	MV_REG_WRITE(LCL_TIMER0_VAL, delta);

	/* Enable the timer */
	u = MV_REG_READ(LCL_TIMER_CTRL);
	u = (u & ~LCL_TIMER0_RELOAD_EN) | LCL_TIMER0_EN;
	MV_REG_WRITE(LCL_TIMER_CTRL, u);

	local_irq_restore(flags);

	return 0;
}

static void msys_clkevt_mode(enum clock_event_mode mode, struct clock_event_device *dev)
{
	unsigned long flags;
	u32 u;

	local_irq_save(flags);

	if ((mode == CLOCK_EVT_MODE_PERIODIC) ||
	    (mode == CLOCK_EVT_MODE_ONESHOT)) {
		/* Setup timer to fire at 1/HZ intervals */
		MV_REG_WRITE(LCL_TIMER0_RELOAD, (ticks_per_jiffy - 1));
		MV_REG_WRITE(LCL_TIMER0_VAL, (ticks_per_jiffy - 1));

		/* Enable timer interrupt */
		//msys_irq_unmask(irq_get_irq_data(IRQ_LOCALTIMER));
		armada_370_xp_irq_unmask(irq_get_irq_data(IRQ_LOCALTIMER));

		/* Enable timer */
		u = MV_REG_READ(LCL_TIMER_CTRL);

		if (mode == CLOCK_EVT_MODE_PERIODIC)
			u |= (LCL_TIMER0_EN | LCL_TIMER0_RELOAD_EN | LCL_TIMER_TURN_25MHZ);
		else
			u |= (LCL_TIMER0_EN | LCL_TIMER_TURN_25MHZ);

		MV_REG_WRITE(LCL_TIMER_CTRL, u);
	} else {
		/* Disable timer */
		u = MV_REG_READ(LCL_TIMER_CTRL);
		u &= ~LCL_TIMER0_EN;
		MV_REG_WRITE(LCL_TIMER_CTRL, u);

		/* Disable timer interrupt */
		//msys_irq_mask(irq_get_irq_data(IRQ_LOCALTIMER));
		armada_370_xp_irq_mask(irq_get_irq_data(IRQ_LOCALTIMER));

		/* ACK pending timer interrupt */
		MV_REG_WRITE(LCL_TIMER_CAUSE, LCL_INT_TIMER0_CLR);
	}

	local_irq_restore(flags);
}

static struct clock_event_device msys_clkevt;
static irqreturn_t msys_timer_interrupt(int irq, void *dev_id)
{
	/* ACK timer interrupt and call event handler */
	MV_REG_WRITE(LCL_TIMER_CAUSE, LCL_INT_TIMER0_CLR);
	msys_clkevt.event_handler(&msys_clkevt);

	return IRQ_HANDLED;
}

static struct irqaction msys_timer_irq = {
	.name		= "msys_tick",
	.flags		= IRQF_DISABLED | IRQF_TIMER,
	.handler	= msys_timer_interrupt,
	.dev_id		= &msys_clkevt,
};


/*
 * Setup the local clock events for a CPU.
 */
void __cpuinit mv_timer_setup(struct clock_event_device *clk, unsigned int fabric_clk)
{
	unsigned int cpu = smp_processor_id();

	clk->features		= (CLOCK_EVT_FEAT_ONESHOT | CLOCK_EVT_FEAT_PERIODIC),
	clk->shift		= 32,
	clk->rating		= 300,
	clk->set_next_event	= msys_clkevt_next_event,
	clk->set_mode		= msys_clkevt_mode,
	clk->cpumask		= cpumask_of(cpu);
	clk->mult		= div_sc(fabric_clk, NSEC_PER_SEC, clk->shift);
	clk->max_delta_ns	= clockevent_delta2ns(0xffffffff, clk);
	clk->min_delta_ns	= clockevent_delta2ns(0x1, clk);
}

/*
 * Resume timer from suspend to RAM
 * TODO - need to implement kernel hooks for suspend/resume
 */
void msys_timer_resume(void)
{
	u32 u;

	pr_info("Resuming MSYS SOC Timer %d\n", soc_timer_id);

	MV_REG_WRITE(TIMER_VAL(soc_timer_id), 0xffffffff);
	MV_REG_WRITE(TIMER_RELOAD(soc_timer_id), 0xffffffff);

	u = MV_REG_READ(BRIDGE_MASK);
	u &= ~BRIDGE_INT_TIMER(soc_timer_id);
	MV_REG_WRITE(BRIDGE_MASK, u);

	u = MV_REG_READ(TIMER_CTRL);
	u |= (TIMER_EN(soc_timer_id) | TIMER_RELOAD_EN(soc_timer_id) |
			TIMER_TURN_25MHZ(soc_timer_id));
	MV_REG_WRITE(TIMER_CTRL, u);
}

#if defined(CONFIG_SMP) && defined(CONFIG_LOCAL_TIMERS)
/*
 * local_timer_ack: checks for a local timer interrupt.
 *
 * If a local timer interrupt has occurred, acknowledge and return 1.
 * Otherwise, return 0.
 */

int local_timer_ack(void)
{
	if (MV_REG_READ(LCL_TIMER_CAUSE) & ~LCL_INT_TIMER0_CLR) {
		MV_REG_WRITE(LCL_TIMER_CAUSE, LCL_INT_TIMER0_CLR);
		return 1;
	}
	return 0;
}

static irqreturn_t msys_localtimer_handler(int irq, void *dev_id)
{
	struct clock_event_device *evt = *(struct clock_event_device **)dev_id;

	if (local_timer_ack()) {
		evt->event_handler(evt);
		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

/*
 * Setup the local clock events for a CPU.
 */
int __cpuinit msys_local_timer_setup(struct clock_event_device *clk)
{
	unsigned int fabric_clk = MV_BOARD_REFCLK_25MHZ;
	static int cpu0_flag = -1;
	int cpu = smp_processor_id();
	struct clock_event_device **this_cpu_clk;

	if (!msys_local_clockevent) {
		int err;

		msys_local_clockevent = alloc_percpu(struct clock_event_device *);
		if (!msys_local_clockevent) {
			pr_err("msys_local_clockevent: can't allocate memory\n");
			return 0;
		}
		err = request_percpu_irq(IRQ_LOCALTIMER, msys_localtimer_handler,
				"msys_local_clockevent", msys_local_clockevent);
		if (err) {
			pr_err("msys_local_clockevent: can't register interrupt %d (%d)\n",
				IRQ_LOCALTIMER, err);
			return 0;
		}
	}

	if ((cpu) || (!cpu && (-1 == cpu0_flag))) {
		ticks_per_jiffy = (fabric_clk + HZ/2) / HZ;
		clk->name = "local_timer";
		clk->irq = IRQ_LOCALTIMER;
		mv_timer_setup(clk, fabric_clk);
		this_cpu_clk = __this_cpu_ptr(msys_local_clockevent);
		*this_cpu_clk = clk;
		clockevents_register_device(clk);

		if (!cpu)
			cpu0_flag++;
	}
	enable_percpu_irq(clk->irq, 0);
	return 0;
}

#ifdef CONFIG_HOTPLUG_CPU
/*
 * take a local timer down
 */
void msys_local_timer_stop(struct clock_event_device *evt)
{
	unsigned long flags;
	u32 u;

	local_irq_save(flags);

	/* Disable timer */
	u = MV_REG_READ(LCL_TIMER_CTRL);
	u &= ~LCL_TIMER0_EN;
	MV_REG_WRITE(LCL_TIMER_CTRL, u);
	MV_REG_WRITE(LCL_TIMER_CAUSE, LCL_INT_TIMER0_CLR);

	/* Disable timer interrupt */
	msys_irq_mask(irq_get_irq_data(IRQ_LOCALTIMER));

	local_irq_restore(flags);
}
#endif

static struct local_timer_ops msys_local_timer_ops __cpuinitdata = {
	.setup	= msys_local_timer_setup,
#ifdef CONFIG_HOTPLUG_CPU
	.stop	= msys_local_timer_stop,
#endif
};
#endif	/* CONFIG_LOCAL_TIMERS && CONFIG_SMP */

void __init msys_time_init(unsigned int fabric_clk)
{
	struct device_node *np;
	u32 u;

	soc_timer_id = 0;

	printk(KERN_INFO "Initializing MSYS SOC Timer %d\n", soc_timer_id);

	np = of_find_compatible_node(
			NULL, NULL, "marvell,armada-xp-timer");
	cntmr_regs_offset = (unsigned) of_iomap(np, 0);
	lcl_timer_base = (unsigned) of_iomap(np, 1);
        bridge_regs_base = (unsigned) ioremap(0xF10210c4, 4);
        if (!cntmr_regs_offset || !lcl_timer_base || !bridge_regs_base) {
            WARN(1, "failed to iomap timer addreses\n");
            return;
        }

	ticks_per_jiffy = (fabric_clk + HZ/2) / HZ;

	msys_setup_sched_clock(fabric_clk);

	/* Setup free-running clocksource timer (interrupts disabled) */
	MV_REG_WRITE(TIMER_VAL(soc_timer_id), 0xffffffff);
	MV_REG_WRITE(TIMER_RELOAD(soc_timer_id), 0xffffffff);
	u = MV_REG_READ(BRIDGE_MASK);
	u &= ~BRIDGE_INT_TIMER(soc_timer_id);
	MV_REG_WRITE(BRIDGE_MASK, u);
	u = MV_REG_READ(TIMER_CTRL);
	u |= (TIMER_EN(soc_timer_id) | TIMER_RELOAD_EN(soc_timer_id) | TIMER_TURN_25MHZ(soc_timer_id));
	u &= ~TIMER_EN(4); // disable early watch dog

	MV_REG_WRITE(TIMER_CTRL, u);
	msys_clksrc.mult = clocksource_hz2mult(fabric_clk, msys_clksrc.shift);
	clocksource_register(&msys_clksrc);

#ifdef CONFIG_SMP
	{
#ifdef CONFIG_LOCAL_TIMERS
		local_timer_register(&msys_local_timer_ops);
#endif
		percpu_timer_setup();
		return;
	}
#endif
	/* Setup clockevent timer (interrupt-driven) */
	msys_clkevt.name = "msys_tick";
	msys_clkevt.irq = IRQ_LOCALTIMER;
	mv_timer_setup(&msys_clkevt, fabric_clk);
	setup_irq(IRQ_LOCALTIMER, &msys_timer_irq);
	clockevents_register_device(&msys_clkevt);

	wd_rstoutn_regs_offset = ioremap(0xf1020704, 0x4);
	if (wd_rstoutn_regs_offset) {
		u = MV_REG_READ(wd_rstoutn_regs_offset);
		u &= ~BIT(8);
		MV_REG_WRITE(wd_rstoutn_regs_offset, u);
		iounmap(wd_rstoutn_regs_offset);
	}
}

void __init msys_timer_init(void)
{
	msys_time_init(MV_BOARD_REFCLK_25MHZ);
}
