#include <linux/init.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/irq.h>
#include <linux/ioport.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/serial_8250.h>
#include <asm/bootinfo.h>
#include <asm/rb/boards.h>
#include <asm/io.h>
#include <asm/mipsregs.h>
#include <asm/pgtable.h>
#include <asm/reboot.h>
#include <asm/addrspace.h>

#define RB500_UART_BASE 0x18058000
#define RB500_UART_IRQ 104

#define TIMER_BASE     ((unsigned *) KSEG1ADDR(0x18028000))
#define TIMER_COUNT0   (TIMER_BASE + 0)
#define TIMER_COMPARE0 (TIMER_BASE + 1)
#define TIMER_CTC0     (TIMER_BASE + 2)
#define TIMER_CTCSEL0  (TIMER_BASE + 3)

extern unsigned int mips_hpt_frequency;

extern void __init rb500_init_latch(void);

int rb500_beeper_enabled = 1;

static void rb_machine_restart(char *command)
{
	/* just jump to the reset vector */
	* (volatile unsigned *) KSEG1ADDR(0x18008000) = 0x80000001;
	
	((void (*)(void)) KSEG1ADDR(0x1FC00000u))();
}

static void rb_machine_halt(void)
{
	local_irq_disable();
	for(;;) continue;
}

static irqreturn_t rb500_beep_irq(int irq, void *p) {
	return IRQ_HANDLED;
}

static int __init rb500_init_beep_irq(void) {
	if (mips_machgroup == MACH_GROUP_MT_RB500) {
		return request_irq(8, rb500_beep_irq, 0, "beeper", 0);
	}
	return 0;
}

static void rb500_free_beep_irq(void) {
	if (mips_machgroup == MACH_GROUP_MT_RB500) {
		free_irq(8, 0);
	}
}
arch_initcall(rb500_init_beep_irq);

void rb500_beep(unsigned freq) {
	if (!rb500_beeper_enabled) return;

	if (freq) {
		/* limit to 23Hz-10kHz */
		unsigned delay = mips_hpt_frequency / 2 / min(max(freq, 23u), 10000u);

		writel(0, TIMER_COUNT0);
		writel(delay, TIMER_COMPARE0);
		writel(0, TIMER_CTCSEL0);
		writel(1, TIMER_CTC0);
	} else {
		writel(0, TIMER_CTC0);
	}
}

EXPORT_SYMBOL(rb500_beep);

void rb500_enable_beeper(int enable) {
	if (rb500_beeper_enabled != enable) {
		rb500_beeper_enabled = enable;
		
		if (enable) {
			rb500_init_beep_irq();
		} else {
			rb500_beep(0);
			rb500_free_beep_irq();
		}
	}
}
EXPORT_SYMBOL(rb500_enable_beeper);

void __init rb500_serial_console_init(void)
{
	struct uart_port port;

	memset(&port, 0, sizeof(port));
	port.type = PORT_16550;
	port.uartclk =  mips_hpt_frequency;
	port.membase = (unsigned char *) KSEG1ADDR(RB500_UART_BASE);
	port.irq = RB500_UART_IRQ;
	port.regshift = 2;
	port.iotype = UPIO_MEM;
	port.flags = UPF_BOOT_AUTOCONF | UPF_SKIP_TEST;

	early_serial_setup(&port);
}

void __init rb500_setup(void)
{
	rb500_serial_console_init();
	rb500_init_latch();

	_machine_restart = rb_machine_restart;
	_machine_halt = rb_machine_halt;

	ioport_resource.start = 0x18800000;
	ioport_resource.end = 0x188FFFFF;
	iomem_resource.start = 0x50000000;
	iomem_resource.end = 0x5FFFFFFF;
}
