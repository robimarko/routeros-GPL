#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/serial.h>
#include <linux/serial_8250.h>

#include <asm/time.h>
#include <asm/reboot.h>
#include <asm/rb/rb400.h>

extern int vsprintf(char *buf, const char *fmt, va_list args);

#define UART0_BASE 	0xb8060000

#define UART(reg) (* (volatile unsigned *) (UART0_BASE + (reg)))
#define MEM(reg) (* (volatile unsigned *) (reg))

static char sprint_buf[1024] = { 0 };

#define OFS_SEND_BUFFER   0x00
#define OFS_LINE_STATUS   0x14

static void music_uart_putc(char byte) {
    while ((UART(OFS_LINE_STATUS) & 0x20) == 0x0) { }
    UART(OFS_SEND_BUFFER) = byte;
}

static void write_serial(char *str, int count) {
    int i;

    for (i = 0; i <= count; i++) {
        music_uart_putc(str[i]);
    }

    music_uart_putc('\r');
    memset(str, '\0', 1024);

    return;
}

void serial_print(char *fmt, ...) {
    va_list args;
    int n;
    
    va_start(args, fmt);
    n = vsprintf(sprint_buf, fmt, args);
    va_end(args);
    write_serial(sprint_buf, n);
}

int get_cpu_freq(void) {
    static int cpu_speed_mhz[8] = { 200, 300, 400, 480, 533, 600, 300, 300 };
    return cpu_speed_mhz[MEM(0xb8800010) & 0x7] * 1000 * 1000;
}

static void music_restart(char *command) {
    while (1) { MEM(0xb8000000) = 0x80000000; }
}

extern void (*beep_misc)(unsigned);
extern int (*misc_beep_init)(void);

#define TIMER_REG(x)		((unsigned long) timer_base + (x))
#define REG_TIMER_CTRL		TIMER_REG(0x00)
#define REG_TIMER_COUNT		TIMER_REG(0x04)
#define REG_TIMER_RELOAD	TIMER_REG(0x08)
#define REG_TIMER_STATUS	TIMER_REG(0x24)

#define TIMER_IRQ_NUM		8

#define GPIO_BASE	0x18040000
#define GPIO_REG(x)	((unsigned long) gpio_base + (x))
#define REG_GPIO_OE	GPIO_REG(0x00)
#define REG_GPIO_OUT	GPIO_REG(0x08)

static void __iomem *timer_base;
static void __iomem *gpio_base;

void beep_music(unsigned freq) {
    if (freq == 0) {
	rb400_modreg(REG_TIMER_CTRL, 0, BIT(0));
    }
    else {
	int timeout = mips_hpt_frequency / freq;
	rb400_writel(timeout, REG_TIMER_RELOAD);
	rb400_modreg(REG_TIMER_CTRL, BIT(0), 0);
    }
}

static irqreturn_t music_beep_int(int irq, void *data) {
    if (rb400_readl(REG_TIMER_STATUS) & BIT(0)) {
	rb400_writel(rb400_readl(REG_GPIO_OUT) ^ BIT(1), REG_GPIO_OUT);
	rb400_writel(BIT(0), REG_TIMER_STATUS);
	return IRQ_HANDLED;
    }
    else {
	return IRQ_NONE;
    }
}

int music_beep_init(void) {
    int err = request_irq(TIMER_IRQ_NUM, &music_beep_int, 0, "beeper", NULL);
    if (err) printk("ERROR: could not request beeper irq\n");
    timer_base = ioremap(0x18030000, PAGE_SIZE);
    gpio_base = ioremap(0x18080000, PAGE_SIZE);
    rb400_modreg(REG_GPIO_OE, BIT(1), 0);
    return 0;
}

void music_setup(void) {
    struct uart_port port;
    mips_hpt_frequency = get_cpu_freq() / 2;
    _machine_restart = music_restart;

    memset(&port, 0, sizeof(port));
    port.type = PORT_16550;
    port.mapbase = 0x18060000;
    port.uartclk =  mips_hpt_frequency;
    port.flags = UPF_BOOT_AUTOCONF | UPF_SKIP_TEST | UPF_IOREMAP;
    port.iotype = UPIO_MEM;
    port.regshift = 2;
    port.irq = 13;
    
    early_serial_setup(&port);

    beep_misc = &beep_music;
    misc_beep_init = &music_beep_init;
}
