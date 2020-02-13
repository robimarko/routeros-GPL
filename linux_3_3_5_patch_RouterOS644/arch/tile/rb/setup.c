#include <linux/mmc/host.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <asm/io.h>
#include <asm/rb.h>
#include <gxio/gpio.h>

#include <hv/drv_srom_intf.h>

EXPORT_SYMBOL(hv_simcard_io);

static int devhdl;
static DEFINE_SPINLOCK(nor_lock);

static unsigned long mmc_pins = MMC_PINS;
static unsigned long gpi_sd_cdn = GPI_SD_CDn;
static unsigned long gpo_sd_pwr = GPO_SD_PWR;
static unsigned _fan_sense_count = 2;
static unsigned _fan_sense_gpios = GPI_SENSE(1) | GPI_SENSE(2);

#define FN_ARG int (*fn)(int, __hv32, HV_VirtAddr, __hv32, __hv64)

static int nor_fn(void *buf, int count, unsigned long offset, FN_ARG) {
    int retval;
    int retries = 1000;
    unsigned long flags;

    spin_lock_irqsave(&nor_lock, flags);
    for (;;) {
	retval = fn(devhdl, 0, (HV_VirtAddr) buf, count, offset);
	if (retval == HV_EBUSY && --retries > 0) {
	    mdelay(20);
	    continue;
	}
	else if (retval < 0) {
	    continue;
	}
	else break;
    }
    spin_unlock_irqrestore(&nor_lock, flags);

    return retval;
}  

int nor_read(void *buf, int count, unsigned long offset) {
    return nor_fn(buf, count, offset, hv_dev_pread);
}  
EXPORT_SYMBOL(nor_read);

static int nor_write(void *buf, int count, unsigned long offset) {
    return nor_fn(buf, count, offset, hv_dev_pwrite);
}

int nor_program(void *buf, int count, unsigned long offset) {
    char dummy;
    int retval;
    retval = nor_write(buf, count, offset);
    nor_write(&dummy, 1, SROM_FLUSH_OFF);
    return retval;
}
EXPORT_SYMBOL(nor_program);

static unsigned read_unsigned(unsigned *offset) {
    unsigned val = 0;
    nor_read(&val, sizeof(unsigned), *offset);
    *offset += sizeof(unsigned);
    return val;    
}

int read_booter_cfg(unsigned id, void *buf, int amount) {
    unsigned offset = 0x30000;
    if (read_unsigned(&offset) != 0x64726148) return 0;
    while (offset < 0x31000) {
	unsigned data = read_unsigned(&offset);
	unsigned tag = data & 0xffff;	
	int len = data >> 16;	
	if (len == 0 || len > 0x1000 || tag == 0) {
	    break;
	} else if (tag == id) {
	    amount = min(len, amount);
	    nor_read(buf, amount, offset);
	    return amount;
	}
	offset = offset + len;
    }
    return 0;
}
EXPORT_SYMBOL(read_booter_cfg);

static gxio_gpio_context_t gpio_ctx;

void set_gpio(unsigned long value, unsigned long mask) {
    gxio_gpio_set(&gpio_ctx, value, mask);
}
EXPORT_SYMBOL(set_gpio);

unsigned long get_gpio(void) {
    return gxio_gpio_get(&gpio_ctx);
}
EXPORT_SYMBOL(get_gpio);

static spinlock_t latch_lock;
static unsigned hw_options = 0;

int is_CCR1072_r3(void) {
    return hw_options & (1 << 4);
}
EXPORT_SYMBOL(is_CCR1072_r3);

void __init ccr_init(void) {
    char name[64];
    int voltage = hv_get_voltage();
    spin_lock_init(&latch_lock);
    devhdl = hv_dev_open((HV_VirtAddr) "srom/0/0", 0);
    read_booter_cfg(ID_BOARD_NAME, name, sizeof(name));
    read_booter_cfg(ID_HW_OPTIONS, &hw_options, sizeof(unsigned));
    printk("Hello [%s] HW=%08x\n", name, hw_options);
    if (voltage) {
	printk("Core Voltage is %d.%04d\n",
	       (int) (voltage / 1000000),
	       (voltage % 1000000) / 100);
    }
}

unsigned long gpio_irq_refresh(unsigned long pin) {
    uint64_t s1 = pin, s2 = pin;
    gxio_gpio_report_reset_interrupt(&gpio_ctx, &s1, &s2);
    return (s1 | s2);
}
EXPORT_SYMBOL(gpio_irq_refresh);

void gpio_irq(unsigned irq, unsigned long pin) {
    int my_cpu = smp_processor_id();
    int x = cpu_x(my_cpu);
    int y = cpu_y(my_cpu);
    gxio_gpio_cfg_interrupt(&gpio_ctx, x, y, KERNEL_PL, irq, pin, pin);
    gpio_irq_refresh(pin);
}
EXPORT_SYMBOL(gpio_irq);

static struct platform_device gpio_beep_device = {
	.name	= "gpio-beep",
	.id	= -1,
	.dev	= {
		.platform_data = (void*)15,
	},
};

static struct spi_board_info ccr_spi_microsd = {
	.modalias = "mmc_spi",
	.max_speed_hz = 10 * 1000 * 1000,
	.bus_num = 0,
	.chip_select = 0,
	.controller_data = (void*)56,
	.mode = SPI_MODE_3,
};

static struct spi_board_info *ccr_spi_info[] = {
	&ccr_spi_microsd, NULL
};

static struct platform_device tile_mmc_device = {
	.name	= "ccr-mmc-spi",
	.id	= -1,
	.dev	= {
		.platform_data = ccr_spi_info,
	},
};

static struct sim_gpio ccr1009_sim_gpio = {
	.io	= GPIO_SIM_IO,
	.clk	= GPO_SIM_CLK,
	.rst	= GPO_SIM_RST,
	.en	= GPO_SIM_EN,
	.vsel	= GPO_SIM_3V_1V8n,
};
static struct platform_device ccr1009_sim_device = {
	.name	= "ccr-sim",
	.id	= -1,
	.dev	= {
		.platform_data = &ccr1009_sim_gpio,
	},
};

static struct sim_gpio ccr1072_sim_gpio = {
	.io	= (1ULL << 35),
	.clk	= (1ULL << 36),
	.rst	= (1ULL << 37),
	.en	= (1ULL << 38),
	.vsel	= (1ULL << 39),
};
static struct platform_device ccr1072_sim_device = {
	.name	= "ccr-sim",
	.id	= -1,
	.dev	= {
		.platform_data = &ccr1072_sim_gpio,
	},
};

static struct platform_device tile_led_device = {
	.name	= "ccr-led",
	.id	= -1,
};

static struct platform_device tile_uart_device = {
	.name	= "TILEGx_Serial",
	.id	= -1,
};

static struct platform_device tile_crypto_device = {
	.name	= "tile",
	.id	= -1,
};

static struct platform_device tile_eth_device = {
	.name	= "tilegx",
	.id	= -1,
};

static struct platform_device *tile_devices[] = {
	&tile_led_device,
	&tile_uart_device,
	&tile_crypto_device,
	&tile_eth_device,
	&gpio_beep_device,
};

static struct platform_device *tile_mmc_devices[] = {
	&tile_mmc_device,
};

static int tile_gpio_get(struct gpio_chip *chip, unsigned offset) {
    return (get_gpio() & BIT(offset)) != 0;
}

static void tile_gpio_set(struct gpio_chip *chip, unsigned offset, int value) {    
    unsigned long bit = BIT(offset);
    set_gpio(value ? bit : 0, bit);
}

static int tile_direction_input(struct gpio_chip *chip, unsigned offset) {
    gxio_gpio_set_dir(&gpio_ctx, 0, BIT(offset), 0, 0);
    return 0;
}

static int tile_direction_output(struct gpio_chip *chip, unsigned offset, int value) {
    tile_gpio_set(chip, offset, value);
    gxio_gpio_set_dir(&gpio_ctx, 0, 0, BIT(offset), 0);
    tile_gpio_set(chip, offset, value);
    return 0;
}

static struct gpio_chip tile_gpio_chip = {
    .label			= "gpio",
    .direction_input		= tile_direction_input,
    .direction_output		= tile_direction_output,
    .set			= tile_gpio_set,
    .get			= tile_gpio_get,
    .base			= 0,
    .ngpio			= 64,
};

static void micro_sd_power_toggle(void) {
    int card_in = (get_gpio() & gpi_sd_cdn) == 0;
//    printk("MicroSD power %s\n", card_in ? "on" : "off");
    set_gpio(card_in ? gpo_sd_pwr : 0, gpo_sd_pwr);
}

irqreturn_t mmc_cd_irq(int irq, void *ptr) {
    struct spi_device *spi = ptr;
    void *mmc_host = dev_get_drvdata(&spi->dev);
    if (gpio_irq_refresh(gpi_sd_cdn) & gpi_sd_cdn && mmc_host) {
	micro_sd_power_toggle();
	mmc_detect_change(mmc_host, msecs_to_jiffies(100));
    }
    return IRQ_HANDLED;
}

void init_ccr_spi(struct spi_master *master) {
    int irq_num;
    struct spi_device *spi_dev;
    spi_dev = spi_new_device(master, &ccr_spi_microsd);

    irq_num = create_irq();
    if (irq_num <= 0) return;
    tile_irq_activate(irq_num, TILE_IRQ_PERCPU);
    if (request_irq(irq_num, &mmc_cd_irq, IRQF_SHARED, "MicroSD", spi_dev)) {
	return;
    }
    gpio_irq(irq_num, gpi_sd_cdn);

}
EXPORT_SYMBOL(init_ccr_spi);

static int is_prefix(char *str, char *base) {
    unsigned len = strlen(str);
    if (strlen(base) < len) return 0;    
    return (memcmp(base, str, len) == 0) ? 1 : 0;
}

#define FETCH_NAME(id) \
    static int fetched = 0; \
    static char name[64]; \
    if (!fetched) { \
	read_booter_cfg(id, name, sizeof(name)); \
	fetched = 1; \
    }

int is_name_prefix(char *str) {
    FETCH_NAME(ID_BOARD_NAME);
    return is_prefix(str, name);
}
EXPORT_SYMBOL(is_name_prefix);

int is_board_type(char *str) {
    FETCH_NAME(ID_BOARD_TYPE_NAME);
    return strcmp(str, name) == 0;
}
EXPORT_SYMBOL(is_board_type);

int fan_sense_count(void) {
    return _fan_sense_count;
}
EXPORT_SYMBOL(fan_sense_count);

unsigned long fan_sense_gpios(void) {
    return _fan_sense_gpios;
}
EXPORT_SYMBOL(fan_sense_gpios);

struct latch_info {
    unsigned long strobe;
    unsigned long state;
    int input;
    int output;
};

#define latch_delay() __insn_mf(); ndelay(15)
unsigned long access_latch(int num, unsigned long value, unsigned long mask) {
    static struct latch_info info[] = {
	{ .strobe = GPO_PR_STROBE1,
	  .state = RST_USB | RST_PCI | RST_ETH | RST_LCD | RST_LED | RST_I2C,
	  .input = 0,
	  .output = 16,
	},
	{ .strobe = GPO_PR_STROBE2,
	  .state = 0,
	  .input = 32,
	  .output = 40,
	},
    };

    int i;
    unsigned long flags;
    unsigned long ret = 0;
    struct latch_info *latch = &info[num];
    spin_lock_irqsave(&latch_lock, flags);
    latch->state = (latch->state & ~mask) | (value & mask);
    set_gpio(0, GPO_AUX_RESET | latch->strobe);
    for (i = 0; i < latch->output; ++i) {
	unsigned long pin = (latch->state & (1ULL << i)) ? GPO_PR_MOSI : 0;
	set_gpio(pin, GPO_PR_MOSI | GPO_PR_CLK);
	latch_delay();
	set_gpio(pin | GPO_PR_CLK, GPO_PR_MOSI | GPO_PR_CLK);
	latch_delay();
    }
    set_gpio(latch->strobe, latch->strobe);
    for (i = 0; i < latch->input; ++i) {
	set_gpio(0, GPO_PR_CLK);
	latch_delay();
	ret = (ret << 1) | ((get_gpio() & GPI_PR_MISO) ? 1 : 0);
	set_gpio(GPO_PR_CLK, GPO_PR_CLK);
	latch_delay();
    }
    spin_unlock_irqrestore(&latch_lock, flags);
    return ret;
}
EXPORT_SYMBOL(access_latch);

void ccr_deassert_usb_reset(unsigned i) {
    static int nr[] = { 13, 12 };
    if (is_name_prefix("CCR1072") && i < ARRAY_SIZE(nr)) {
	printk("CCR1072: deasserting USB%d reset (pin%d)\n", i, nr[i]);
	access_latch(0, 0, BIT(nr[i]));
	udelay(1000);
    }
}
EXPORT_SYMBOL(ccr_deassert_usb_reset);

struct class_attribute_gpio {
    struct class_attribute class_attr;
    unsigned long gpio_bit;
};

static ssize_t show_gpio_state(struct class *class,
			struct class_attribute *attr,
			char *output_buffer)
{
    struct class_attribute_gpio *info;
    info = container_of(attr, struct class_attribute_gpio, class_attr);
    return scnprintf(output_buffer, PAGE_SIZE, "%d\n",
		     !!(get_gpio() & info->gpio_bit));
}

static ssize_t store_nothing(struct class *class,
			     struct class_attribute *attr,
			     const char *buf, size_t count)
{
    return count;
}

unsigned long add_sysfs_gpio_state_file(struct class *hw_info,
					const char *filename,
					int gpio_num) {
	static struct class_attribute_gpio infos[2];
	static unsigned infos_used = 0;
	struct class_attribute_gpio *info = &infos[infos_used++];
	BUG_ON(infos_used > ARRAY_SIZE(infos));
	memset(info, 0, sizeof(struct class_attribute_gpio));

	info->class_attr.attr.name = filename;
	info->class_attr.attr.mode = S_IWUSR | S_IRUGO;
	info->class_attr.show = &show_gpio_state;
	info->class_attr.store = &store_nothing;
	info->gpio_bit = BIT(gpio_num);

	if (class_create_file(hw_info, &info->class_attr) < 0) {
	    printk("add_sysfs_gpio_state_file: failed\n");
	}

	return info->gpio_bit;
}

unsigned long add_psu_state(unsigned gpio_numA, unsigned gpio_numB) {
    unsigned long pins = 0;
    struct class *hw_info = class_create(THIS_MODULE, "hw_info");
    pins |= add_sysfs_gpio_state_file(hw_info, "psu1_state", gpio_numA);
    pins |= add_sysfs_gpio_state_file(hw_info, "psu2_state", gpio_numB);
    return pins;
}

void usb_power_enable(void) {
    set_gpio(is_board_type("ccr1009c") ? GPO_USB_POWER : 0, GPO_USB_POWER);
}
EXPORT_SYMBOL(usb_power_enable);

static void ccr_add_uSD_pins(ulong *in, ulong *out, ulong *all) {
    if (hw_options & HW_OPT_HAS_uSD) {
	*in |= gpi_sd_cdn;
	*out |= gpo_sd_pwr;
	*all |= mmc_pins;
    }
}

static void ccr_add_uSD_device(void) {
    if (hw_options & HW_OPT_HAS_uSD) {
	printk("CCR with MicroSD slot!\n");
	set_gpio(0, gpo_sd_pwr);
	udelay(10000);
	micro_sd_power_toggle();
	platform_add_devices(tile_mmc_devices, ARRAY_SIZE(tile_mmc_devices));
    }
}

int ccr_platform_init(void) {
    unsigned long in_pins, out_pins, od_pins, all_pins = 0;
    struct platform_device *pdev_sim = NULL;

    in_pins = GPI_BUTTON
	| fan_sense_gpios()
	| GPI_PIN_HOLE;
    out_pins = GPO_BEEP
	| GPO_USER_LED
	| GPO_LCD_LED
	| GPO_USB_POWER;
    od_pins = GPO_FAN_ON(0) | GPO_MON_SEL;
    
    if (is_name_prefix("CCR1036-8G")
	|| is_name_prefix("CCR1036-12S")
	|| is_name_prefix("CCR1016-12S")) {
	od_pins |= GPO_FAN_ON(1);
    }

    if (is_name_prefix("CCR1072")) {
	struct spi_board_info **spi_info;
	in_pins = GPI_PR_MISO | GPI_BUTTON;
	out_pins = GPO_PR_STROBE1
	    | GPO_PR_STROBE2
	    | GPO_PR_CLK
	    | GPO_PR_MOSI
	    | GPO_BEEP
	    | GPO_USER_LED
	    | GPO_MON_SEL72
	    | GPO_LCD_LED
	    | GPO_AUX_RESET;
	od_pins = GPO_FAN_ON(0);

	if (is_CCR1072_r3()) out_pins |= GPO_USB_RST(0) | GPO_USB_RST(1);

	gpi_sd_cdn = (1ULL << 40);
	gpo_sd_pwr = (1ULL << 41);
	mmc_pins = (0x3FULL << 42);

	tile_mmc_device.name = "ccr-mmc-spi-ccr1072";
	spi_info = tile_mmc_device.dev.platform_data;
	spi_info[0]->controller_data = (void *) 44;
    }
    
    if (is_name_prefix("CCR1009-8G-1S-1S+")
	    || is_name_prefix("CCR1009-7G-1C")
	    || is_name_prefix("CCR1009-7G-1C-1S+")) {
	in_pins |= add_psu_state(35, 34);
	_fan_sense_count = 1;
	_fan_sense_gpios = GPI_SENSE(1);
    }

    if (is_name_prefix("CCR1016-12S-1S+")) {
	in_pins |= add_psu_state(48, 50);
    }

    ccr_add_uSD_pins(&in_pins, &out_pins, &all_pins);

    if (hw_options & HW_OPT_HAS_SIM) {
	struct sim_gpio *gpio;
	pdev_sim = &ccr1009_sim_device;
	if (is_name_prefix("CCR1072")) pdev_sim = &ccr1072_sim_device;

	gpio = (struct sim_gpio *)pdev_sim->dev.platform_data;
	od_pins |= gpio->io;
	out_pins |= gpio->clk | gpio->rst | gpio->en | gpio->vsel;
    }

    all_pins |= out_pins | in_pins | od_pins;

    gxio_gpio_init(&gpio_ctx, 0);
    gxio_gpio_attach(&gpio_ctx, all_pins);
    gxio_gpio_set_dir(&gpio_ctx, 0, in_pins, out_pins, od_pins);
    usb_power_enable();

    gpiochip_add(&tile_gpio_chip);

    ccr_add_uSD_device();

    if (pdev_sim) {
	struct sim_gpio *gpio = (struct sim_gpio *)pdev_sim->dev.platform_data;
	printk("CCR with SIM card slot!\n");
	set_gpio(gpio->vsel,
		 gpio->io | gpio->clk | gpio->rst | gpio->en | gpio->vsel);
	platform_device_register(pdev_sim);
    }
    if (is_name_prefix("CCR1072")) {
	/* use USB type A connector by default */
	access_latch(0, BIT(0), BIT(0));
    }

    return platform_add_devices(tile_devices, ARRAY_SIZE(tile_devices));
}

arch_initcall(ccr_platform_init);
