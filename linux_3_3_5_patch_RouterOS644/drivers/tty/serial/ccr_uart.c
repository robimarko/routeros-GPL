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

#define CONSOLE_PORT 0

#define UART_COUNT 2

#define PORT_CCR 251

#define UART_PUT_CHAR		1
#define UART_GET_CHAR		2
#define UART_SET_BAUDRATE	3

static void ccr_uart_put_char(struct uart_port *port, char c) {
	hv_uart_ctrl(port->line, UART_PUT_CHAR, c);
	port->icount.tx++;
}

static void ccr_uart_stop_tx(struct uart_port *port) { }

static unsigned int ccr_uart_tx_empty(struct uart_port *port) {
	return TIOCSER_TEMT;
}

static void ccr_uart_set_mctrl(struct uart_port *port, unsigned int mctrl) { }

static unsigned int ccr_uart_get_mctrl(struct uart_port *port) {
	return 0;
}

static void ccr_uart_start_tx(struct uart_port *port) {
	struct circ_buf *xmit = &port->state->xmit;

	if (port->x_char) {
		ccr_uart_put_char(port, port->x_char);
		port->x_char = 0;
	}

	while (!uart_circ_empty(xmit)) {
		ccr_uart_put_char(port, xmit->buf[xmit->tail]);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
	}
}

static void ccr_uart_stop_rx(struct uart_port *port) { }

static void ccr_uart_enable_ms(struct uart_port *port) { }

static void ccr_uart_break_ctl(struct uart_port *port, int break_state) { }

static struct timer_list uart_timer[UART_COUNT];
static unsigned uart_timeout[UART_COUNT];

#define MIN_TIMEOUT (HZ/100)
#define MAX_TIMEOUT (HZ/5)

static void ccr_uart_timeout(int index, int reset) {
	uart_timeout[index] = (reset ? 0 : uart_timeout[index]) + MIN_TIMEOUT;
	uart_timeout[index] = min(uart_timeout[index], MAX_TIMEOUT);
	mod_timer(&uart_timer[index], jiffies + uart_timeout[index]);
}

static void ccr_uart_rx_chars(unsigned long data) {
	int byte, count = 0;
	struct uart_port *port = (struct uart_port *) data;
	spin_lock(&port->lock);
	while ((byte = hv_uart_ctrl(port->line, UART_GET_CHAR, 0)) != -1) {
		uart_insert_char(port, 0, 0, byte & 0xff, TTY_NORMAL);
		port->icount.rx++;
		count++;
	}
	tty_flip_buffer_push(port->state->port.tty);
	ccr_uart_timeout(port->line, count);
	spin_unlock(&port->lock);
}

static int ccr_uart_startup(struct uart_port *port) {
	struct timer_list *timer = &uart_timer[port->line];
	init_timer(timer);
	timer->data = (unsigned long) port;
	timer->function = ccr_uart_rx_chars;
	ccr_uart_timeout(port->line, 1);
	return 0;
}

static void ccr_uart_shutdown(struct uart_port *port) {
	del_timer(&uart_timer[port->line]);
}

static void ccr_set_baudrate(int index, int baudrate) {
    static unsigned baudrate_table[UART_COUNT] = { 0, 0 };
    if (baudrate_table[index] != baudrate) {
	hv_uart_ctrl(index, UART_SET_BAUDRATE, baudrate);
	baudrate_table[index] = baudrate;
    }
}

static void ccr_uart_set_termios(struct uart_port *port,
				 struct ktermios *termios,
				 struct ktermios *old)
{
	unsigned long flags;
	int baudrate = uart_get_baud_rate(port, termios, old, 9600, 115200);

	spin_lock_irqsave(&port->lock, flags);
	ccr_set_baudrate(port->line, baudrate);
	uart_update_timeout(port, termios->c_cflag, baudrate);
	spin_unlock_irqrestore(&port->lock, flags);
}

static const char *ccr_uart_type(struct uart_port *port) {
	return port->type == PORT_CCR ? "CCR-UART" : NULL;
}

static void ccr_uart_config_port(struct uart_port *port, int flags) {
	if (flags & UART_CONFIG_TYPE) port->type = PORT_CCR;
}

static void ccr_uart_release_port(struct uart_port *port) { }

static int ccr_uart_request_port(struct uart_port *port) {
	return 0;
}

static struct uart_ops ccr_uart_ops = {
	.tx_empty =	ccr_uart_tx_empty,
	.set_mctrl =	ccr_uart_set_mctrl,
	.get_mctrl =	ccr_uart_get_mctrl,
	.stop_tx =	ccr_uart_stop_tx,
	.start_tx =	ccr_uart_start_tx,
	.stop_rx =	ccr_uart_stop_rx,
	.enable_ms =	ccr_uart_enable_ms,
	.break_ctl =	ccr_uart_break_ctl,
	.startup =	ccr_uart_startup,
	.shutdown =	ccr_uart_shutdown,
	.set_termios =	ccr_uart_set_termios,
	.type =		ccr_uart_type,
	.config_port =	ccr_uart_config_port,
	.release_port =	ccr_uart_release_port,
	.request_port =	ccr_uart_request_port,
};

static struct uart_port ccr_uart_ports[] = {
	{
		.line     =	0,
		.fifosize =	16,
		.type     =	PORT_CCR,
		.ops	  =	&ccr_uart_ops,
		.flags    =	ASYNC_BOOT_AUTOCONF,
	},
	{
		.line     =	1,
		.fifosize =	16,
		.type     =	PORT_CCR,
		.ops      =	&ccr_uart_ops,
		.flags    =	ASYNC_BOOT_AUTOCONF,
	},
};

static void ccr_console_write(struct console *con,
			      const char *str, 
			      unsigned int count)
{
	while (count--) {
		if (*str == '\n') {
			hv_uart_ctrl(CONSOLE_PORT, UART_PUT_CHAR, '\r');
		}
		hv_uart_ctrl(CONSOLE_PORT, UART_PUT_CHAR, *str);
		str++;
	}
}

extern void disable_early_printk(void);
static int ccr_console_setup(struct console *con, char *options)
{
	struct uart_port *port;
	int baud = 115200;
	int parity = 'n';
	int flow = 'n';
	int bits = 8;
	int ret = 0;

	port = &ccr_uart_ports[CONSOLE_PORT];

	if (options) {
		uart_parse_options(options, &baud, &parity, &bits, &flow);
		ret = uart_set_options(port, con, baud, parity, bits, flow);
	}

	disable_early_printk();

	return ret;
}

static struct uart_driver ccr_uart_reg;

static struct console ccr_serial_console = {
	.name =		"ttyS",
	.write =	ccr_console_write,
	.device =	uart_console_device,
	.setup =	ccr_console_setup,
	.flags =	CON_PRINTBUFFER,
	.index =	CONSOLE_PORT,
	.data =		&ccr_uart_reg,
};

static int __init ccr_console_init(void)
{
	register_console(&ccr_serial_console);
	return 0;
}

console_initcall(ccr_console_init);

static struct uart_driver ccr_uart_reg = {
	.owner	=	THIS_MODULE,
	.driver_name =	"ccr-uart",
	.dev_name =	"hvc",
	.major =	TTY_MAJOR,
	.minor =	64,
	.nr =		UART_COUNT,
	.cons =		&ccr_serial_console,
};

static int ccr_uart_probe(struct platform_device *pdev)
{
	int i;
	for (i = 0; i < UART_COUNT; i++) {
		uart_add_one_port(&ccr_uart_reg, &ccr_uart_ports[i]);
		platform_set_drvdata(pdev, &ccr_uart_ports[i]);
	}
	return 0;
}

static struct platform_driver ccr_uart_driver = {
	.probe	= ccr_uart_probe,
	.driver	= {
		.name	= "ccr-uart",
		.owner	= THIS_MODULE,
	},
};

static int __init ccr_uart_init(void)
{
	int ret;
	ret = uart_register_driver(&ccr_uart_reg);
	if (ret == 0) {
		ret = platform_driver_register(&ccr_uart_driver);
		if (ret) uart_unregister_driver(&ccr_uart_reg);
	}
	return ret;
}

module_init(ccr_uart_init);
