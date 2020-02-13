#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/serial.h>
#include <linux/types.h>
#include <linux/module.h>

#include <asm/bootinfo.h>
#include <asm/reboot.h>
#include <asm/io.h>
#include <asm/time.h>
#include <asm/pgtable.h>
#include <asm/processor.h>
#include <asm/system.h>

#define INTC_BASE   0x12200000
#define SWCTRL_BASE 0x12000000

#define ADM5120_SW_REG(reg) \
        (*((volatile unsigned *) (KSEG1ADDR(SWCTRL_BASE + (reg)))))
#define ADM5120_INTC_REG(reg) \
        (*((volatile unsigned *) (KSEG1ADDR(INTC_BASE + (reg)))))

#define IRQ_TIMER 0

#define INTC_IRQ_ENABLE		0x08
#define INTC_IRQ_DISABLE	0x0c
#define INTC_IRQ_MODE		0x14

#define SW_SOFT_RESET		0x0004
#define SW_CPU_PORT_CONF	0x0024
#define SW_PORT_CONF0		0x0028
#define SW_GPIO			0x00b8
#define SW_TIMER_INT		0x00f0
#define SW_TIMER		0x00f4

#define GPIO6_OUTPUT_EN		0x00400000
#define GPIO6_OUTPUT_HI		0x40000000

#define SW_SOFTWARE_RESET 1

#define SW_CPU_PORT_DISABLE 0x00000001

#define SW_PORT_DISABLE_MASK 0x0000003F

static int beeper_enabled = 1;
unsigned beep_delay = 0;

void rb100_restart(char *command) {
	/* Disable All ports*/
	ADM5120_SW_REG(SW_PORT_CONF0) |= SW_PORT_DISABLE_MASK;

	/* Disable CPU port */
	ADM5120_SW_REG(SW_CPU_PORT_CONF) |= SW_CPU_PORT_DISABLE;

	/* Wait until switch DMA idle. At least 1ms is required! */
	mdelay(1);

	ADM5120_SW_REG(SW_SOFT_RESET) = SW_SOFTWARE_RESET;
}

void rb100_halt(void) {
	local_irq_disable();
        while (1);
}

void rb100_beep(unsigned freq) {
    if (!beeper_enabled) return;

    if (freq) {
	/* limit to 23Hz-10kHz */
	beep_delay =
	    max_t(unsigned, min_t(unsigned, 1562500 / 2 / freq, 0xffff), 78u) | (1u << 16);

	ADM5120_INTC_REG(INTC_IRQ_MODE) = (1 << IRQ_TIMER); /* enable FIQ */
	ADM5120_INTC_REG(INTC_IRQ_ENABLE) = (1 << IRQ_TIMER);
	
	ADM5120_SW_REG(SW_TIMER) = beep_delay;
	ADM5120_SW_REG(SW_TIMER_INT) = 1;

	ADM5120_SW_REG(SW_GPIO) |= GPIO6_OUTPUT_EN | GPIO6_OUTPUT_HI;
    } else {
	beep_delay = 0;

	ADM5120_INTC_REG(INTC_IRQ_MODE) = 0; /* disable FIQ */
	ADM5120_INTC_REG(INTC_IRQ_DISABLE) = (1 << IRQ_TIMER);

	ADM5120_SW_REG(SW_TIMER) = beep_delay;
	ADM5120_SW_REG(SW_TIMER_INT) = 1 << 16;

	ADM5120_SW_REG(SW_GPIO) &= ~(GPIO6_OUTPUT_EN | GPIO6_OUTPUT_HI);
    }
}

void rb100_enable_beeper(int enable)
{
	if (!enable && beep_delay) rb100_beep(0);
	beeper_enabled = enable;
}
EXPORT_SYMBOL(rb100_enable_beeper);

void __init rb100_setup(void) {
	_machine_restart = rb100_restart;
	_machine_halt = rb100_halt;
}

EXPORT_SYMBOL(rb100_beep);
