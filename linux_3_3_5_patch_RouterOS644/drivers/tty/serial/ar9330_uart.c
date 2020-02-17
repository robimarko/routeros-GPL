/*
 *	Serial driver for AR9330 SoC
 *
 *	Derived from drivers/serial/uart00.c
 *	Copyright 2001 Altera Corporation
 *
 *	Some pieces are derived from the ADMtek 2.4 serial driver.
 *	Copyright (C) ADMtek Incorporated, 2003
 *		daniell@admtek.com.tw
 *	Which again was derived from drivers/char/serial.c
 *	Copyright (C) Linus Torvalds et al.
 *
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/console.h>
#include <linux/platform_device.h>
#include <linux/serial_reg.h>
#include <asm/bootinfo.h>
#include <asm/rb/boards.h>

#define PORT_AR9330	250

#define MEM(x) *((volatile unsigned *) (x))

unsigned modreg(struct uart_port *port,
		unsigned reg,
		unsigned set,
		unsigned clr) {
	writel((readl(port->membase + reg) & ~clr) | set, port->membase + reg);
	return readl(port->membase + reg);
}

static unsigned uart_regs = 0;
void ar9330_prom_putchar(const char c) {
	while ((MEM(uart_regs) & 0x200) == 0) { }
	MEM(uart_regs) = 0x200 | (unsigned) c;	
}

static void ar9330ser_flip(struct uart_port *port,
			   int state,
			   unsigned reg,
			   unsigned bit) {
	if (state) {
		modreg(port, reg, bit, 0);
	}
	else {
		modreg(port, reg, 0, bit);
	}
}

static void ar9330ser_set_tx_irq(struct uart_port *port, int state) {
	ar9330ser_flip(port, state, 0x10, (1 << 1));
}

static void ar9330ser_set_rx_irq(struct uart_port *port, int state) {
	ar9330ser_flip(port, state, 0x10, (1 << 0));
}

static void ar9330ser_stop_tx(struct uart_port *port)
{
	ar9330ser_set_tx_irq(port, 0);
}

static unsigned int ar9330ser_tx_empty(struct uart_port *port)
{
	return (readl(port->membase) & 0x200) ? TIOCSER_TEMT : 0;
}

static void ar9330ser_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
}

static unsigned int ar9330ser_get_mctrl(struct uart_port *port)
{
	return TIOCM_CTS | TIOCM_CAR | TIOCM_DSR;
}

static void ar9330ser_putchar(struct uart_port *port, const char c) {
	while ((readl(port->membase) & 0x200) == 0) { }
	writel(0x200 | (unsigned) c, port->membase);
}

static void ar9330ser_tx_chars(struct uart_port *port) {
	struct uart_state *info = port->state;
	if (info->xmit.head == info->xmit.tail
	    || info->port.tty->stopped
	    || info->port.tty->hw_stopped) {
		ar9330ser_stop_tx(port);
		return;
	}
	if (port->x_char) {
		ar9330ser_putchar(port, port->x_char);
		port->x_char = 0;
		return;
	}
	ar9330ser_putchar(port, info->xmit.buf[info->xmit.tail]);
	info->xmit.tail++;
	info->xmit.tail %= UART_XMIT_SIZE;

	if (CIRC_CNT(info->xmit.head,
		     info->xmit.tail,
		     UART_XMIT_SIZE)
	    < WAKEUP_CHARS) {
		uart_write_wakeup(port);
	}
	if (info->xmit.head == info->xmit.tail) {
		ar9330ser_stop_tx(port);
	}
}

static void ar9330ser_rx_chars(struct uart_port *port) {
	unsigned int c = 0;
	do {
		c = readl(port->membase);
		if (c & 0x100) {
			port->icount.rx++;
			writel(0x100, port->membase);
			uart_insert_char(port, 0, 0, c & 0xff, TTY_NORMAL);
		}
	} while (c & 0x100);
	tty_flip_buffer_push(port->state->port.tty);
}


static irqreturn_t ar9330ser_irq(int irq, void *dev_id)
{
	struct uart_port *port = dev_id;	
	if (readl(port->membase + 0x0c) | (1 << 1)) {
		ar9330ser_tx_chars(port);		
		writel((1 << 1), port->membase + 0x0c);
	}
	if (readl(port->membase + 0x0c) | (1 << 0)) {
		ar9330ser_rx_chars(port);		
		writel((1 << 0), port->membase + 0x0c);
	}
	return IRQ_HANDLED;
}

static void ar9330ser_start_tx(struct uart_port *port)
{	
	ar9330ser_set_tx_irq(port, 1);
}

static void ar9330ser_stop_rx(struct uart_port *port)
{
}

static void ar9330ser_enable_ms(struct uart_port *port)
{
}

static void ar9330ser_break_ctl(struct uart_port *port, int break_state)
{
}

static void ar9330ser_set_irq(struct uart_port *port, int state) {
	ar9330ser_flip(port, state, 0x04, (1 << 13));
}

static int ar9330ser_startup(struct uart_port *port)
{
	int ret;

	if (request_mem_region(port->iobase, 0x20, "ar9330-uart") != NULL) {
		port->membase = ioremap_nocache(port->iobase, 0x20);
	}
	else {
		return -EBUSY;
	}

	ret = request_irq(port->irq, ar9330ser_irq, 0, "AR9330 UART", port);
	if (ret) {
		printk(KERN_ERR "Couldn't get irq %d\n", port->irq);
		return ret;
	}

	ar9330ser_set_irq(port, 1);
	ar9330ser_set_rx_irq(port, 1);
	return 0;
}

static void ar9330ser_shutdown(struct uart_port *port)
{
	ar9330ser_set_rx_irq(port, 0);
	ar9330ser_set_irq(port, 0);
	free_irq(port->irq, port);
	iounmap(port->membase);
	release_mem_region(port->iobase, 0x20);	
}

static void ar9330ser_set_termios(struct uart_port *port,
    struct ktermios *termios, struct ktermios *old)
{
}

static const char *ar9330ser_type(struct uart_port *port)
{
	return port->type == PORT_AR9330 ? "AR9330" : NULL;
}

static void ar9330ser_config_port(struct uart_port *port, int flags)
{
	if (flags & UART_CONFIG_TYPE) port->type = PORT_AR9330;
}

static void ar9330ser_release_port(struct uart_port *port)
{
}

static int ar9330ser_request_port(struct uart_port *port)
{
	return 0;
}

static struct uart_ops ar9330ser_ops = {
	.tx_empty =	ar9330ser_tx_empty,
	.set_mctrl =	ar9330ser_set_mctrl,
	.get_mctrl =	ar9330ser_get_mctrl,
	.stop_tx =	ar9330ser_stop_tx,
	.start_tx =	ar9330ser_start_tx,
	.stop_rx =	ar9330ser_stop_rx,
	.enable_ms =	ar9330ser_enable_ms,
	.break_ctl =	ar9330ser_break_ctl,
	.startup =	ar9330ser_startup,
	.shutdown =	ar9330ser_shutdown,
	.set_termios =	ar9330ser_set_termios,
	.type =		ar9330ser_type,
	.config_port =	ar9330ser_config_port,
	.release_port =	ar9330ser_release_port,
	.request_port =	ar9330ser_request_port,
};

static struct uart_port ar9330ser_ports[] = {
	{
		.iobase   =	0x18020000,
		.irq      =	19,
		.line     =	0,
		.fifosize =	16,
		.ops =		&ar9330ser_ops,
		.flags    =	ASYNC_BOOT_AUTOCONF,
	},
};

static void ar9330console_write(struct console *con, const char *s,
    unsigned int count)
{
	while (count--) {
		if (*s == '\n')
			ar9330_prom_putchar('\r');
		ar9330_prom_putchar(*s);
		s++;
	}
}

static int ar9330console_setup(struct console *con, char *options)
{
	struct uart_port *port;
	int baud = 115200;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';
	int ret = 0;

	port = &ar9330ser_ports[0];

	if (options) {
		uart_parse_options(options, &baud, &parity, &bits, &flow);
		ret = uart_set_options(port, con, baud, parity, bits, flow);
	}

	return ret;
}

static struct uart_driver ar9330ser_reg;

static struct console ar9330_serconsole = {
	.name =		"ttyS",
	.write =	ar9330console_write,
	.device =	uart_console_device,
	.setup =	ar9330console_setup,
	.flags =	CON_PRINTBUFFER,
	.index =	0,
	.data =		&ar9330ser_reg,
};

static int is_ar9330(void) {
	if (mips_machtype == MACH_MT_RB951) return 1;
	if (mips_machtype == MACH_MT_CM2N) return 1;
	if (mips_machtype == MACH_MT_mAP) return 1;
	return 0;
}

static int __init ar9330console_init(void)
{
	if (is_ar9330()) {
	        uart_regs = (unsigned) ioremap_nocache(0xb8020000, 0x20);
		register_console(&ar9330_serconsole);
	}
	return 0;
}

console_initcall(ar9330console_init);


static struct uart_driver ar9330ser_reg = {
	.owner	=	THIS_MODULE,
	.driver_name =	"ar9330_uart",
	.dev_name =	"ttyS",
	.major =	TTY_MAJOR,
	.minor =	64,
	.nr =		1,
	.cons =		&ar9330_serconsole,
};

static int ar9330ser_probe(struct platform_device *pdev)
{
	uart_add_one_port(&ar9330ser_reg, &ar9330ser_ports[0]);
	platform_set_drvdata(pdev, &ar9330ser_ports[0]);
	return 0;
}

static struct platform_driver ar9330ser_driver = {
	.probe	= ar9330ser_probe,
	.driver	= {
		.name	= "ar9330-uart",
		.owner	= THIS_MODULE,
	},
};

static int __init ar9330ser_init(void)
{
	int ret;
	extern unsigned uart_enabled;
	if (!is_ar9330() || !uart_enabled)
		return 0;

	printk("AR9330 UART\n");

	ret = uart_register_driver(&ar9330ser_reg);
	if (ret == 0) {
		ret = platform_driver_register(&ar9330ser_driver);
		if (ret) uart_unregister_driver(&ar9330ser_reg);
	}
	return ret;
}

module_init(ar9330ser_init);
