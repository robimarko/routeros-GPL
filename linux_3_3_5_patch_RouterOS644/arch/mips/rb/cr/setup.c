#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/serial.h>
#include <linux/types.h>
#include <linux/string.h>	/* for memset */
#include <linux/ioport.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/serial_8250.h>

#include <asm/reboot.h>
#include <asm/io.h>
#include <asm/time.h>
#include <asm/pgtable.h>
#include <asm/processor.h>
#include <asm/system.h>
#include <asm/rb/cr.h>
#include <asm/bootinfo.h>
#include <asm/rb/boards.h>

static struct timer_list wdi_timer;
#define WDI_INTERVAL	(10 * HZ)

void cr_restart(char *command) {
	CR_GPOUT() &= ~CR_GPIO_CRST;
	CR_GPDIR() |= CR_GPIO_CRST;
}

void cr_halt(void) {
	local_irq_disable();
        while (1);
}

void __init cr_serial_console_init(void)
{
	struct uart_port port;

	memset(&port, 0, sizeof(port));
	port.type = PORT_16550;
	port.uartclk =  mips_hpt_frequency;
	port.membase = (unsigned char *) KSEG1ADDR(CR_UART_BASE);
	port.irq = CR_UART_IRQ;
	port.regshift = 2;
	port.iotype = UPIO_MEM;
	port.flags = UPF_BOOT_AUTOCONF | UPF_SKIP_TEST;

	early_serial_setup(&port);
}

void __init cr_setup(void)
{
	cr_serial_console_init();

	_machine_restart = cr_restart;
	_machine_halt = cr_halt;
}

static void wdi_interrupt(unsigned long unused)
{
	local_irq_disable();
	CR_GPOUT() ^= CR_GPIO_WDI;
	local_irq_enable();

	wdi_timer.expires = jiffies + WDI_INTERVAL;
	add_timer(&wdi_timer);
}

static int __init wdi_setup(void)
{
	if (mips_machgroup != MACH_GROUP_MT_CR) return 0;

	CR_GPDIR() |= CR_GPIO_WDI;	// enable as an output
	memset(&wdi_timer, 0, sizeof(wdi_timer));
	init_timer(&wdi_timer);
	wdi_timer.function = wdi_interrupt;
	wdi_interrupt(0);
	return 0;
}

arch_initcall(wdi_setup);
