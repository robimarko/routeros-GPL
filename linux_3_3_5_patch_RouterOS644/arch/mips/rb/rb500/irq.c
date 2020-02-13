/*
 * Copyright 2002 MontaVista Software Inc.
 * Author: MontaVista Software, Inc.
 *		stevel@mvista.com or source@mvista.com
 *
 *  This program is free software; you can redistribute	 it and/or modify it
 *  under  the terms of	 the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the	License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED	  ``AS	IS'' AND   ANY	EXPRESS OR IMPLIED
 *  WARRANTIES,	  INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO	EVENT  SHALL   THE AUTHOR  BE	 LIABLE FOR ANY	  DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED	  TO, PROCUREMENT OF  SUBSTITUTE GOODS	OR SERVICES; LOSS OF
 *  USE, DATA,	OR PROFITS; OR	BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN	 CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel_stat.h>
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/timex.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/delay.h>

#include <asm/bitops.h>
#include <asm/bootinfo.h>
#include <asm/io.h>
#include <asm/mipsregs.h>
#include <asm/system.h>
#include <asm/irq_cpu.h>
#include <asm/irq.h>

#define GPIO_BASE 0x18050000
#define GPIO_REG(reg) (*(volatile u32 *) KSEG1ADDR(GPIO_BASE + (reg)))

#define GPIO_INT_STATUS 0x10

#define GROUP0_IRQ_BASE 8
#define GROUP4_IRQ_BASE 136

#define RB500_NR_IRQS  168

#define INT_PEND(base) (*(volatile u32 *) base)
#define INT_MASK(base) (*((volatile u32 *) base + 2))

#define IRQ_TIMER 8

extern asmlinkage void rbIRQ(void);

struct intr_group {
	u32 mask;
	volatile u32 *base_addr;
};

static const struct intr_group intr_group[] = {
	{ 0x0000efff, (u32 *) KSEG1ADDR(0x18038000) },
	{ 0x00001fff, (u32 *) KSEG1ADDR(0x18038000 + 1 * 12) },
	{ 0x00000007, (u32 *) KSEG1ADDR(0x18038000 + 2 * 12) },
	{ 0x0003ffff, (u32 *) KSEG1ADDR(0x18038000 + 3 * 12) },
	{ 0xffffffff, (u32 *) KSEG1ADDR(0x18038000 + 4 * 12) }
};

static void rb500_unmask_irq(struct irq_data *d)
{
	volatile unsigned int *addr;

	unsigned irq = d->irq - GROUP0_IRQ_BASE;
	addr = intr_group[irq >> 5].base_addr;
	irq &= (1 << 5) - 1;
	
	INT_MASK(addr) &= ~(1 << irq);
}

static void rb500_mask_irq(struct irq_data *d)
{
	volatile unsigned int *addr;

	unsigned irq = d->irq - GROUP0_IRQ_BASE;
	addr = intr_group[irq >> 5].base_addr;
	irq &= (1 << 5) - 1;
	
	INT_MASK(addr) |= 1 << irq;
}

static void rb500_gpio_unmask_irq(struct irq_data *d) {
	GPIO_REG(GPIO_INT_STATUS) &= ~(1 << (d->irq - GROUP4_IRQ_BASE));
	rb500_unmask_irq(d);
}

static struct irq_chip rb500_irq_controller = {
	.name		= "RB500",
	.irq_ack		= rb500_mask_irq,
	.irq_mask		= rb500_mask_irq,
	.irq_mask_ack		= rb500_mask_irq,
	.irq_unmask		= rb500_unmask_irq,
};

static struct irq_chip rb500_gpio_irq_controller = {
	.name		= "RB500 GPIO",
	.irq_ack		= rb500_mask_irq,
	.irq_mask		= rb500_mask_irq,
	.irq_mask_ack		= rb500_mask_irq,
	.irq_unmask		= rb500_gpio_unmask_irq,
};

static struct irqaction cascade  = {
	.handler = no_action,
	.name = "cascade",
};

void rb500_irqdispatch(void)
{
	unsigned group;
	volatile unsigned *addr;
	unsigned irq;
	unsigned pending = read_c0_status() & read_c0_cause() & 0xfc00;

	if (!pending) return;

	group = fls(pending) - 11;
	irq = group + 2;

	if (irq == 7) {
	    do_IRQ(irq);
	    return;
	}

	addr = intr_group[group].base_addr;

	pending = INT_PEND(addr) & ~INT_MASK(addr);
	if (!pending) return;
	
	irq = (group << 5) + (ffs(pending) - 1) + 8;
	
	if (irq != IRQ_TIMER)
		do_IRQ(irq);
	else
		/* do not enable softirqs & friends for OProfile */
		generic_handle_irq(irq);
}

void __init rb500_init_irq(void)
{
	int i;

	mips_cpu_irq_init();
	set_except_vector(0, rbIRQ);

	for (i = 2; i <= 6; ++i)
		setup_irq(i, &cascade);
	for (i = GROUP0_IRQ_BASE; i < RB500_NR_IRQS; ++i)
		irq_set_chip_and_handler(i, &rb500_irq_controller,
					 handle_level_irq);
	for (i = GROUP4_IRQ_BASE; i < GROUP4_IRQ_BASE + 14; ++i)
		irq_set_chip_and_handler(i, &rb500_gpio_irq_controller,
					 handle_level_irq);
}
