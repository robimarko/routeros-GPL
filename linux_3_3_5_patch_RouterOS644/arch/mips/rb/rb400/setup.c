#include <linux/kernel.h>
#include <linux/irq.h>
#include <linux/serial_8250.h>
#include <linux/serial.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <asm/bootinfo.h>
#include <asm/dma-mapping.h>
#include <asm/reboot.h>
#include <asm/addrspace.h>
#include <asm/traps.h>
#include <asm/rb/boards.h>
#include <asm/rb/rb400.h>
#include <asm/time.h>

#define RESET_BASE	0x18060000
#define RESET_REG(x)	((unsigned long) reset_base + (x))
static unsigned RST_RESET = 0x24;
#define REG_RESET		RESET_REG(RST_RESET)
#define REG_TIMER		RESET_REG(0)
#define REG_TIMER_RELOAD	RESET_REG(4)
#define TIMER_IRQ_NUM	16
#define RST_RESET_RB750		0x1C
#define RESET_CPU		(1 << 24)
#define RESET_USB_OHCI_DLL	(1 << 6)
#define RESET_USB_HOST		(1 << 5)
#define RESET_USB_PHY		(1 << 4)
#define RESET_USB	(RESET_USB_OHCI_DLL | RESET_USB_HOST | RESET_USB_PHY)

#define PLL_BASE	0x18050000

#define USB_BASE	0x18030000
#define USB_REG(x)	((unsigned long) usb_base + (x))
#define REG_USB_FLADJ	USB_REG(0x00)
#define REG_USB_CONFIG	USB_REG(0x04)

#define SLIC_BASE	0x18090000
#define SLIC_REG(x)	((unsigned long) slic_base + (x))
#define REG_SLIC_CTRL	SLIC_REG(0x04)
#define SLIC_CTRL_INIT	0x7
#define REG_SLIC_FREQ	SLIC_REG(0x30)

#define DMA_BASE	0x180a0000
#define DMA_REG(x)	((unsigned long) dma_base + (x))
#define REG_DMA_ADDR	DMA_REG(0x3c)
#define REG_DMA_CFG	DMA_REG(0x40)
#define DMA_CFG_INIT	0x170
#define REG_DMA_UPD	DMA_REG(0x44)
#define DMA_START	0x3
#define REG_DMA_INT	DMA_REG(0x48)
#define ENABLE_INT	0x2000

#define GPIO_BASE	0x18040000
#define GPIO_REG(x)	((unsigned long) gpio_base + (x))
#define REG_GPIO_OUT	GPIO_REG(0x08)
#define REG_GPIO_SET	GPIO_REG(0x0c)
#define REG_GPIO_CLEAR	GPIO_REG(0x10)
#define GPO_BEEP_RB700	(1 << 13)
#define REG_GPIO_FN	GPIO_REG(0x28)
#define GPIO_EN_I2S	0x20000

#define STEREO_BASE	0x180b0000
#define STEREO_REG(x)	((unsigned long) stereo_base + (x))
#define REG_STEREO		STEREO_REG(0x00)
#define  STEREO_ENABLE_AR71  0x010101ff
#define REG_STEREO_VOL_AR71	STEREO_REG(0x04)
#define  DMA_DESC_OWN	     (1<<31)

#define SLIC_IRQ_NUM	23

struct dma_beep_desc {
	u32 size;
	u32 addr;
	u32 next;
};

static void __iomem *dma_base;
static void __iomem *reset_base;
static void __iomem *gpio_base;

extern unsigned mips_hpt_frequency;
extern struct plat_serial8250_port rb400_uart_data[];
static unsigned ahb_freq = -1u;

static int no_buzzer = 0;
static char beep_xor;
static unsigned beep_buf_size;       /* bufer size */
static unsigned char *beep_buf;      /* for rewrite bufer and stop beep */
struct dma_beep_desc *dma_audio;

int is_wasp(void);
int is_scorpion(void);
int is_honeybee(void);

int rb400_be_handler(struct pt_regs *regs, int is_fixup);
int rb700_be_handler(struct pt_regs *regs, int is_fixup);

extern int is_crs_type(void);

static void flush_hornet_uart_fifo(void) {
	unsigned uart_cs = (unsigned) ioremap_nocache(0x18020004, PAGE_SIZE);
	unsigned counter = read_c0_count();
	do {
	    if ((rb400_readl(uart_cs) & BIT(14)) == 0) break;
	} while	(read_c0_count() - counter < mips_hpt_frequency); // wait a sec
}

static void rb400_restart(char *command)
{
	int is_hornet(void);
#ifdef CONFIG_SPI_RB400
	if (mips_machgroup == MACH_GROUP_MT_RB400
	    || mips_machgroup == MACH_GROUP_MT_RB700) {
	    void rb400_exit_4byte_spi_addressing(void);
	    rb400_exit_4byte_spi_addressing();
	}
#endif
	if (is_hornet()) {
	    flush_hornet_uart_fifo();
	}
	while (1)
		rb400_writel(RESET_CPU, REG_RESET);
}

static void rb400_halt(void)
{
	local_irq_disable();
	while (1);
}

static irqreturn_t rb400_beep_int(int irq, void *data)
{
	if (mips_machgroup == MACH_GROUP_MT_RB400) {
		if (mips_machtype == MACH_MT_RB750G) return IRQ_NONE;
		rb400_writel(rb400_readl(REG_DMA_INT) & (~0xff), REG_DMA_INT);
		rb400_writel(DMA_START, REG_DMA_UPD);    
		return IRQ_HANDLED;
	} else {
		return IRQ_NONE;
	}
}

static void set_nth_bit(unsigned char *buf, unsigned bit)
{
	buf[(bit >> 3) ^ beep_xor] |= (0x80 >> (bit & 0x7));
}

static void set_bit_range(unsigned char *buf, unsigned offset, unsigned bits)
{
	int i;
	for (i = 0; i < bits; ++i) {
		set_nth_bit(buf, offset + i);
	}
}

static void fill_buffer(unsigned char *buf, const unsigned buf_size,
			unsigned freq, const unsigned sample_freq)
{
	const unsigned num_samples = buf_size * 8;
	unsigned loops = ((freq * num_samples + (sample_freq / 2))
			  / sample_freq);
	unsigned samples_added = 0;
	unsigned i = 0;

	memset(buf, 0, buf_size);
	for (i = 1; i <= loops; ++i) {
		unsigned samples_till_loop = i * num_samples / loops;
		unsigned samples_to_add = samples_till_loop - samples_added;

		set_bit_range(buf, samples_added, samples_to_add / 2);
		samples_added += samples_to_add;
	}
}

static int is_4xx_gpio_beep(void) {
	if (mips_machtype == MACH_MT_RB411G) return 1;
	if (mips_machtype == MACH_MT_RB411L) return 1;
	if (mips_machtype == MACH_MT_RB411UL) return 1;
	if (mips_machtype == MACH_MT_RB433GL) return 1;
	if (mips_machtype == MACH_MT_RB433UL) return 1;
	if (mips_machtype == MACH_MT_RB433L) return 1;
	return 0;
}

static int is_750(void) {
    return (mips_machtype == MACH_MT_RB750 ||
	    mips_machtype == MACH_MT_RB751);
}

static unsigned get_pin_num(void) {
    if (is_4xx_gpio_beep()) {
	return (1 << 9);
    }
    else if (mips_machtype == MACH_MT_RB912G
	     || mips_machtype == MACH_MT_GROOVE52
	     || mips_machtype == MACH_MT_SXT2D
	     || mips_machtype == MACH_MT_RB951G) {
	return (1 << 17);
    }
    else if (mips_machtype == MACH_MT_SXT5N
	     || mips_machtype == MACH_MT_RB911L
	     || mips_machtype == MACH_MT_SXTG5P) {
	return (1 << 19);
    }
    else if (mips_machtype == MACH_MT_SXTG5C
	     || mips_machtype == MACH_MT_RB953GS
	     || mips_machtype == MACH_MT_RB962
	     || mips_machtype == MACH_MT_QRTG
	     || mips_machtype == MACH_MT_RB960
	     || mips_machtype == MACH_MT_OMNI_SC
	     || mips_machtype == MACH_MT_GROOVE_52AC
	     || mips_machtype == MACH_MT_RB750Gv2
	     || mips_machtype == MACH_MT_RB922GS) {
	return (1 << 4);
    }
    else if (mips_machtype == MACH_MT_CRS332) {
	    return (1 << 15);
    }
    else if (mips_machtype == MACH_MT_CRS312) {
	    return (1 << 15);
    }
    else if (is_crs_type()
	     || mips_machtype == MACH_MT_951HND
	     || mips_machtype == MACH_MT_OMNITIK_WASP
	     || mips_machtype == MACH_MT_RB2011R5) {
	return (1 << 22);
    }
    else {
	return GPO_BEEP_RB700;
    }
}

static irqreturn_t rb700_beep_int(int irq, void *data)
{
	unsigned beep_pin = get_pin_num();
	if (rb400_readl(REG_GPIO_OUT) & beep_pin) {
		rb400_writel(beep_pin, REG_GPIO_CLEAR);
	}
	else {
		rb400_writel(beep_pin, REG_GPIO_SET);
	}
	return IRQ_HANDLED;
}

static int has_no_beeper(void) {
    return (mips_machtype == MACH_MT_RB750G
	    || mips_machtype == MACH_MT_RB750GL
	    || mips_machtype == MACH_MT_RB951
	    || mips_machtype == MACH_MT_RB941HL
	    || mips_machtype == MACH_MT_cAP
	    || mips_machtype == MACH_MT_CM2N
	    || mips_machtype == MACH_MT_CCS112
	    || mips_machtype == MACH_MT_CCS112R4
	    || mips_machtype == MACH_MT_mAP
	    || mips_machtype == MACH_MT_mAPL
	    || mips_machtype == MACH_MT_LHG_hb
	    || mips_machtype == MACH_MT_LHG60_hb
	    || mips_machtype == MACH_MT_LTAP_hb
	    || no_buzzer
	    || is_750());
}

static void rb700_beeper_off(void) {
	unsigned reg = REG_GPIO_CLEAR;
	if (mips_machtype == MACH_MT_RB951G) {
		reg = REG_GPIO_SET;
	}
	rb400_writel(get_pin_num(), reg);
}

static int __init rb700_beep_init(void) {
	int err;
	rb400_writel(ahb_freq / 1000, REG_TIMER_RELOAD);
	err = request_irq(TIMER_IRQ_NUM, &rb700_beep_int, 0, "beeper", NULL);
	if (err)
		printk("beeper: could not request irq %d\n", TIMER_IRQ_NUM);
	else
		disable_irq_nosync(TIMER_IRQ_NUM);
	rb700_beeper_off();
	return err;
}

static int __init rb400_beep_init(void) {
	int err;
	void *cookie = (void *) 1;
	void __iomem *slic_base = ioremap(SLIC_BASE, PAGE_SIZE);
	void __iomem *stereo_base  = ioremap(STEREO_BASE, PAGE_SIZE);

	rb400_writel(rb400_readl(REG_GPIO_FN) | GPIO_EN_I2S, REG_GPIO_FN);
	rb400_writel(0x2, REG_SLIC_FREQ);
	rb400_writel(SLIC_CTRL_INIT, REG_SLIC_CTRL);

	beep_xor = 1;
	beep_buf_size = 0x400; /* bufer size for AR71 */
	rb400_writel(STEREO_ENABLE_AR71, REG_STEREO);
	rb400_writel(0, REG_STEREO_VOL_AR71);
	rb400_writel(DMA_CFG_INIT, REG_DMA_CFG); 

	iounmap(stereo_base);
	iounmap(slic_base);

	beep_buf = kmalloc(beep_buf_size, GFP_KERNEL);

	err = request_irq(SLIC_IRQ_NUM, &rb400_beep_int, 0, "beeper", cookie);

	if (err) printk("beeper: could not request irq %d\n", SLIC_IRQ_NUM);
	return err;
}

void (*beep_misc)(unsigned) = NULL;
int (*misc_beep_init)(void) = NULL;

static int is_gpio_beep(void) {
    return mips_machgroup == MACH_GROUP_MT_RB700 || is_4xx_gpio_beep();
}

int __init rb_beep_init(void)
{
	if (has_no_beeper()) return 0;

	if (misc_beep_init) {
		return misc_beep_init();
	} else if (is_gpio_beep()) {
		return rb700_beep_init();
	} else if (mips_machgroup == MACH_GROUP_MT_RB400) {
		return rb400_beep_init();
	}	
	return 0;
}

device_initcall(rb_beep_init);

static void beep_rb400(unsigned freq) {
	static const unsigned sample_freq = 156800;
	dma_addr_t phys;

	if (freq == 0) {
	    rb400_writel(0, REG_DMA_INT);
	    memset(beep_buf, 0, beep_buf_size);
	    dma_map_single(NULL, beep_buf, beep_buf_size, DMA_TO_DEVICE);
	}
	else {
	    fill_buffer(beep_buf, beep_buf_size, freq, sample_freq);
	    phys = dma_map_single(NULL, beep_buf, beep_buf_size, DMA_TO_DEVICE);
	    rb400_writel((unsigned) phys, REG_DMA_ADDR);
	    rb400_writel(ENABLE_INT, REG_DMA_INT);
	    rb400_writel(DMA_START, REG_DMA_UPD);
	}
}

#ifdef CONFIG_MTD_NAND_RB400
static void rb711_switch(unsigned freq) {
    if (mips_machtype == MACH_MT_RB711R3 ||
	mips_machtype == MACH_MT_SXT_LTE ||
	mips_machtype == MACH_MT_RB711G) {
	if (freq == 0)
	    rb700_change_gpo(GPO_RB700_nLINK4, 0);
	else
	    rb700_change_gpo(0, GPO_RB700_nLINK4);
    }
}

static void rb912g_switch(unsigned freq) {
	/* TODO: use led triggers instead */
	if (mips_machtype == MACH_MT_RB912G) {
	    unsigned func = rb400_readl(GPIO_REG(0x3c));
	    if (freq) {
		func &= 0xffff00ff;
		ar9342_change_gpo(1 << 25, 0);
	    }
	    else {
		ar9342_change_gpo(0, 1 << 25);
		func |= 0x00002f00;
	    }
	    rb400_writel(func, GPIO_REG(0x3c));
	}
}
#endif

static void beep_rb700(unsigned freq) {
	static int running = 0;
#ifdef CONFIG_MTD_NAND_RB400
	rb711_switch(freq);
	rb912g_switch(freq);
#endif
	if (freq == 0) {
	    if (running) disable_irq_nosync(TIMER_IRQ_NUM);
	    rb700_beeper_off();
	    running = 0;
	}
	else {
	    rb400_writel(ahb_freq / (freq * 2), REG_TIMER_RELOAD);
	    rb400_writel(0, REG_TIMER);
	    enable_irq(TIMER_IRQ_NUM);
	    running = 1;
	}
}

void rb400_beep(unsigned freq)
{
	if (has_no_beeper()) return;

	printk("rb400_beep %u\n", freq);
	if (beep_misc) {
		beep_misc(freq);
	}
	else if (is_gpio_beep()) {
		beep_rb700(freq);
	}
	else if (mips_machgroup == MACH_GROUP_MT_RB400) {
		beep_rb400(freq);
	}
}
EXPORT_SYMBOL(rb400_beep);

static void __init rb400_ahb_freq_init(void)
{
	unsigned ahb_div;
	void __iomem *pll;

	if (mips_machtype == MACH_MT_RB2011L
	    || mips_machtype == MACH_MT_SXT5N
	    || mips_machtype == MACH_MT_RB2011R5
	    || is_crs_type()
	    || mips_machtype == MACH_MT_RB2011LS
	    || mips_machtype == MACH_MT_RB2011US
	    || mips_machtype == MACH_MT_RB2011G) {
		rb400_writel(BIT(20), REG_GPIO_SET);
	}

	if (is_wasp() || is_scorpion() || is_honeybee()) {
	    /* takes clock direcly from crystal */
	    if (rb400_readl(RESET_REG(0xb0)) & (1 << 4)) {
		    ahb_freq = 40000000;
	    }
	    else {
		    ahb_freq = 25000000;
	    }
	    return;
	}

	pll = ioremap(PLL_BASE, PAGE_SIZE);
	if (mips_machgroup != MACH_GROUP_MT_RB400) {
		ahb_div = ((rb400_readl((unsigned long) pll) >> 19) & 0x1) + 1;
	}
	else {
		ahb_div = ((rb400_readl((unsigned long) pll) >> 20) & 0x7) + 1;
	}
	ahb_freq = mips_hpt_frequency / ahb_div;

	if (mips_machtype == MACH_MT_RB_SXT5D) {
		/* lower PCI-E frequency by 24% to avoid interference on SXT */
		rb400_writel(0x00040c00, 0x10 + (unsigned long) pll);
		rb400_writel(0x000b7147, 0x18 + (unsigned long) pll);
		rb400_writel(0xc00b8000, 0x14 + (unsigned long) pll);
	}
	iounmap(pll);
}

#ifdef CONFIG_EARLY_PRINTK
#define UART_LSR_TEMT             0x40 /* Transmitter empty */
#define UART_LSR_THRE             0x20 /* Transmit-hold-register empty */
#define BOTH_EMPTY (UART_LSR_TEMT | UART_LSR_THRE)
void prom_putchar(char c)
{
	for (;;) {
		if ((readb((void *)0xb8020014) & BOTH_EMPTY) == BOTH_EMPTY)
			break;
	}
	writeb(c, (void *)0xb8020000);
}
#endif

static void __init rb400_serial_console_init(void)
{
	struct uart_port port;
	memset(&port, 0, sizeof(port));
	port.type = PORT_16550;
	port.uartclk =  ahb_freq;
	port.mapbase = 0x18020000;
	port.irq = 19;
	port.regshift = 2;
	port.iotype = UPIO_MEM;
	port.flags = UPF_BOOT_AUTOCONF | UPF_SKIP_TEST | UPF_IOREMAP;

	early_serial_setup(&port);
}

static void __init rb400_usb_reset(void)
{
	void __iomem *usb_base;
	
	if (mips_machgroup != MACH_GROUP_MT_RB400)
		return;
	if (mips_machtype == MACH_MT_RB433U) {
	} else if (mips_machtype == MACH_MT_RB435G) {
	} else if (mips_machtype == MACH_MT_RB493G) {
	} else if (mips_machtype == MACH_MT_RB411UL) {
	} else if (mips_machtype == MACH_MT_RB411G) {
	} else if (mips_machtype == MACH_MT_RB433GL) {
	} else if (mips_machtype == MACH_MT_RB433UL) {
	} else if (mips_machtype == MACH_MT_RB411U) {
	} else {
		return;
	}

	usb_base = ioremap(USB_BASE, PAGE_SIZE);

	rb400_writel(rb400_readl(REG_RESET) | RESET_USB, REG_RESET);
	rb400_readl(REG_RESET);
	mdelay(1000);
	rb400_writel(rb400_readl(REG_RESET) & ~RESET_USB, REG_RESET);
	rb400_readl(REG_RESET);

	/* swap descriptors & data to big endian */
	rb400_writel(0xf0000, REG_USB_CONFIG);
	/* Adjust the duration between two SOFS */
	rb400_writel(0x20c00, REG_USB_FLADJ);
	rb400_readl(REG_USB_CONFIG);

	mdelay(900);

	iounmap(usb_base);
}

static int __init setup_no_buzzer(char *s) {
    no_buzzer = 1;
    return 1;
}
__setup("no-buzzer", setup_no_buzzer);

unsigned uart_enabled = 1;

static int __init disable_uart(char *s) {
    uart_enabled = 0;
    return 1;
}

__setup("no-uart", disable_uart);

#if 0
static irqreturn_t rb400_perfirq(int irq, void *arg) {
    return perf_irq();
}

static int __init rb400_setupperfirq(void) {
	(void)request_irq(21, &rb400_perfirq, 0, "perf", NULL);
	return 0;
}
device_initcall(rb400_setupperfirq);
#endif

static int __init rb400_postsetup(void)
{
	if (mips_machgroup == MACH_GROUP_MT_RB400) {
	} else if (mips_machgroup == MACH_GROUP_MT_RB700) {
		RST_RESET = RST_RESET_RB750;
	} else {
		return 0;
	}

	dma_base = ioremap(DMA_BASE, PAGE_SIZE);
	reset_base = ioremap(RESET_BASE, PAGE_SIZE);
	gpio_base = ioremap(GPIO_BASE, PAGE_SIZE);

	rb400_ahb_freq_init();
	if (mips_machtype == MACH_MT_GROOVE52) {
	    printk("Force UART disable!\n");
	    uart_enabled = 0;
	}
	if (mips_machtype == MACH_MT_CM2N) {
	    unsigned enabled = rb400_readl(GPIO_REG(0x00));
	    rb400_writel(enabled | BIT(28), GPIO_REG(0x00));
	    rb400_writel(BIT(28), GPIO_REG(0x0c));
	}
	if (mips_machtype == MACH_MT_RB750Gv2) {
	    rb400_modreg(GPIO_REG(0x00), 0, BIT(14) | BIT(13));
	}
	if (mips_machtype == MACH_MT_SXT_AC) {
	    rb400_modreg(GPIO_REG(0x00), 0, BIT(14));
	    rb400_modreg(GPIO_REG(0x30), 0, 0xff);
	}
	if (mips_machtype == MACH_MT_wAP_LTE) {
	    rb400_modreg(GPIO_REG(0x00), 0, BIT(15));
	}
	if (mips_machtype == MACH_MT_mAPL) {
	    rb400_modreg(GPIO_REG(0x00), 0, BIT(14));
	    rb400_modreg(GPIO_REG(0x30), 0, (0xff << 0));  // fix ethernet led
	    rb400_modreg(GPIO_REG(0x3c), 0, (0xff << 8));  // fix power led
	}
	if (mips_machtype == MACH_MT_LTAP_hb) {
	    rb400_modreg(GPIO_REG(0x30), 0, (0xff << 0));  // fix ethernet led
	}
	if (mips_machtype == MACH_MT_OMNITIK_WASP) {
	    unsigned outputs = BIT(16) | BIT(13) | BIT(12) | BIT(3);
	    rb400_modreg(GPIO_REG(0x00), 0, outputs);
	    rb400_modreg(GPIO_REG(0x3c), 0, 0xff);
	}
	if (uart_enabled) rb400_serial_console_init();
	rb400_usb_reset();

	return 0;
}
arch_initcall(rb400_postsetup);

void __init rb400_setup(void)
{
	_machine_restart = rb400_restart;
	_machine_halt = rb400_halt;
	switch (mips_machgroup) {
	case MACH_GROUP_MT_RB400:
		board_be_handler = rb400_be_handler;
		break;
	case MACH_GROUP_MT_RB700:
		board_be_handler = rb700_be_handler;
		break;
	}

	set_io_port_base(0);
}

#define RST_VERSION		0x00000090

static int is_rev(unsigned id) {
    if (mips_machgroup == MACH_GROUP_MT_RB400
	|| mips_machgroup == MACH_GROUP_MT_RB700) {
	unsigned rev = rb400_readl(RESET_REG(RST_VERSION)) & 0xfff0;
	return rev == id;
    }
    else {
	return 0;
    }
}

static unsigned get_minor(void) {
    if (mips_machgroup == MACH_GROUP_MT_VM) {
	return 0;
    }
    else {	
	return rb400_readl(RESET_REG(RST_VERSION)) & 0xf;
    }
}

int is_ar7240(void) {
    return is_rev(0xc0);
}
EXPORT_SYMBOL(is_ar7240);

int is_scorpion(void) {
    return is_rev(0x1130) || is_rev(0x0130);
}
EXPORT_SYMBOL(is_scorpion);

int is_honeybee(void) {
    return is_rev(0x140) || is_rev(0x160);
}
EXPORT_SYMBOL(is_honeybee);

int is_wasp(void) {
    return is_rev(0x2120) || is_rev(0x1120);
}
EXPORT_SYMBOL(is_wasp);

int is_hornet(void) {
    return is_rev(0x0110) || is_rev(0x1110);
}
EXPORT_SYMBOL(is_hornet);

int is_new_wasp(void) {
    return is_wasp() && get_minor() >= 3;
}
EXPORT_SYMBOL(is_new_wasp);

int register_ag7100_driver(struct platform_driver *drv)
{
	return platform_driver_register(drv);
}
EXPORT_SYMBOL(register_ag7100_driver);

void unregister_ag7100_driver(struct platform_driver *drv)
{
	return platform_driver_unregister(drv);
}
EXPORT_SYMBOL(unregister_ag7100_driver);

void (*music_reset_notify)(void) = NULL;
EXPORT_SYMBOL(music_reset_notify);

int ar_has_second_pcie_bus(void) {
    if (mips_machtype == MACH_MT_GROOVE_52AC) return false;
    return is_scorpion();
}
