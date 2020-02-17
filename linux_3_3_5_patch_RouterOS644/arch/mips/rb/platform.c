#include <linux/platform_device.h>
#include <linux/serial.h>
#include <linux/serial_8250.h>
#include <linux/spi/spi.h>
#include <linux/mtd/partitions.h>
#include <linux/spi/flash.h>
#include <linux/i2c-gpio.h>
#include <linux/mdio-gpio.h>
#include <linux/poeout-rb.h>
#include <linux/leds-rb.h>
#include <asm/bootinfo.h>
#include <asm/rb/boards.h>
#include <asm/rb/rb400.h>
#include <asm/rb/booter.h>
#include <asm/serial.h>
#include <asm/mach-ralink/surfboardint.h>
#include <asm/mach-ralink/rt_mmap.h>

struct hwopt_device {
	struct platform_device *plat;
	unsigned val;
	unsigned mask;
};

static int platform_add_hwopt_devices(struct hwopt_device *devs, int cnt) {
	int i;
	unsigned hw_opt = read_hw_opt();
	printk("hw_options %x\n", hw_opt);

	for (i = 0; i < cnt; ++i) {
		struct hwopt_device *dev = &devs[i];
		if ((hw_opt & (dev->val | dev->mask)) == dev->val) {
			printk("adding platform device %s\n", dev->plat->name);
			platform_device_register(dev->plat);
		}
	}
	return 0;
}

static struct platform_device rb100_uart_device = {
	.name	= "rb100-uart",
	.id	= -1,
};

static struct platform_device rb500_nand_device = {
	.name	= "rb500-nand",
	.id	= -1,
};

static struct platform_device rb500r5_nand_device = {
	.name	= "rb500r5-nand",
	.id	= -1,
};

static struct platform_device rb700_nand_device = {
	.name	= "rb700-nand",
	.id	= -1,
	.dev	= {
		.platform_data = (void *)0,
	},
};

static struct platform_device rb700_nolatch_nand_device = {
	.name	= "rb700-nand",
	.id	= -1,
	.dev	= {
		.platform_data = (void *)1,
	},
};

static struct platform_device rb700_tiny_nand_device = {
	.name	= "rb700-nand",
	.id	= -1,
	.dev	= {
		.platform_data = (void *)2,
	},
};

static struct platform_device rb750g_nand_device = {
	.name	= "rb750g-nand",
	.id	= -1,
};

static struct platform_device rb100_nand_device = {
	.name	= "rb100-nand",
	.id	= -1,
};

static struct platform_device cr_nand_device = {
	.name	= "cr-nand",
	.id	= -1,
};

static struct platform_device mr_nand_device = {
	.name	= "mr-nand",
	.id	= -1,
};

static struct platform_device ar9342_nand_device = {
	.name	= "ar9342-nand",
	.id	= -1,
};

static struct platform_device ar9344_nand_device = {
	.name	= "ar9344-nand",
	.id	= -1,
};

static struct spi_board_info rb400_spi_misc = {
	.modalias = "rb400-spi-misc",
	.max_speed_hz = 10 * 1000 * 1000,
	.bus_num = 0,
	.chip_select = 0,
	.mode = SPI_MODE_0,
};

static struct rb400_ssr_info rb400_ssr_data = {
	.strobe = 6,
	.bytes = 1,
	.val = (SSR_BIT_nLED1 | SSR_BIT_nLED2 |
		SSR_BIT_nLED3 | SSR_BIT_nLED4 |
		SSR_BIT_nLED5 |
		SSR_BIT_FAN_ON | SSR_BIT_USB_EN),
};
static struct spi_board_info rb400l_spi_misc = {
	.modalias = "rb400-spi-misc",
	.max_speed_hz = 10 * 1000 * 1000,
	.bus_num = 0,
	.chip_select = 0,
	.mode = SPI_MODE_0,
	.platform_data = &rb400_ssr_data,
};

// NOTE: partition sizes are changed on some CRS3xx boards
static struct mtd_partition nor_part_table[] = {
    {
	.name = "RouterOS",
	.offset = 128 * 1024,
	.size = MTDPART_SIZ_FULL
    },
    {
	.name = "RouterBoot",
	.offset = 0,
	.size = 128 * 1024,
    },
    {
	.name = "SwOS",
    }
};
static struct flash_platform_data nor_data = {
        .parts = nor_part_table,
        .nr_parts = 2,
};

static struct spi_board_info rb400_spi_nand = {
	.modalias = "rb400-spi-nand",
	.max_speed_hz = 33 * 1000 * 1000,
	.bus_num = 0,
	.chip_select = 1,
	.mode = SPI_MODE_0,
};
static struct spi_board_info rb400_spi_microsd = {
	.modalias = "mmc_spi",
	.max_speed_hz = 10 * 1000 * 1000,
	.bus_num = 0,
	.chip_select = 2,
	.mode = SPI_MODE_0,
};
static struct spi_board_info rb700_spi_attiny = {
	.modalias = "rb700-spi-attiny",
	.max_speed_hz = 2 * 1000 * 1000,
	.bus_num = 0,
	.chip_select = 3,
	.mode = SPI_MODE_0,
};
static struct spi_board_info rb700_spi_nand = {
	.modalias = "rb700-spi-nand",
	.max_speed_hz = 0,
	.bus_num = 0,
	.chip_select = 1,
	.mode = SPI_CS_HIGH,
};

static struct spi_board_info *rb400_spi_info[] = {
	&rb400_spi_misc,
	&rb400_spi_nand,
	NULL
};

static struct spi_board_info *rb433_spi_info[] = {
	&rb400_spi_misc,
	&rb400_spi_nand,
	&rb400_spi_microsd,
	NULL
};

static struct spi_board_info *rb400l_spi_info[] = {
	&rb400l_spi_misc,
	NULL
};

static struct spi_board_info *rb750_spi_info[] = {
	&rb400_spi_misc,
	NULL
};

static struct spi_board_info *rb711_spi_info[] = {
	&rb400_spi_misc,
	&rb700_spi_nand,
	NULL
};

static struct spi_board_info *omni_spi_info[] = {
	&rb400_spi_misc,
	&rb700_spi_attiny,
	&rb700_spi_nand,
	NULL
};

static struct platform_device omni_spi_device = {
	.name	= "rb400-spi",
	.id	= -1,
	.dev	= {
		.platform_data = omni_spi_info,
	},
};

static struct platform_device rb400_spi_device = {
	.name	= "rb400-spi",
	.id	= -1,
	.dev	= {
		.platform_data = rb400_spi_info,
	},
};

static struct platform_device rb400l_spi_device = {
	.name	= "rb400-spi",
	.id	= -1,
	.dev	= {
		.platform_data = rb400l_spi_info,
	},
};

static struct platform_device rb433_spi_device = {
	.name	= "rb400-spi",
	.id	= -1,
	.dev	= {
		.platform_data = rb433_spi_info,
	},
};

static struct platform_device rb750_spi_device = {
	.name	= "rb400-spi",
	.id	= -1,
	.dev	= {
		.platform_data = rb750_spi_info,
	},
};

static struct platform_device rb711_spi_device = {
	.name	= "rb400-spi",
	.id	= -1,
	.dev	= {
		.platform_data = rb711_spi_info,
	},
};

static struct rb400_ssr_info rb900_ssr_data = {
	.strobe = 17,
	.bytes = 2,
	.val = 0x0c00
};
static struct spi_board_info rb900_spi_misc = {
	.modalias = "rb400-spi-misc",
	.max_speed_hz = 33333334,
	.bus_num = 0,
	.chip_select = 0,
	.mode = SPI_MODE_3,
	.platform_data = &rb900_ssr_data,
};

static struct spi_board_info rb2011_spi_lcd = {
	.modalias = "rb2011-spi-lcd",
	.max_speed_hz = 15 * 1000 * 1000,
	.bus_num = 0,
	.chip_select = 1,
	.mode = SPI_MODE_0,
};

static struct spi_board_info rb2011_spi_ts = {
	.modalias = "rb2011-spi-ts",
	.max_speed_hz = 2000 * 1000,
	.bus_num = 0,
	.chip_select = 2,
	.mode = SPI_MODE_3,
	.platform_data = (void *) 16,
};

static struct spi_board_info *rb2011_spi_info[] = {
	&rb900_spi_misc,
	&rb2011_spi_lcd,
	&rb2011_spi_ts,
	NULL
};

static struct platform_device rb2011_spi_device = {
	.name	= "rb900-spi",
	.id	= -1,
	.dev	= {
		.platform_data = rb2011_spi_info,
	},
};

static struct spi_board_info *rb2011L_spi_info[] = {
	&rb900_spi_misc,
	NULL
};

static struct platform_device rb2011L_spi_device = {
	.name	= "rb900-spi",
	.id	= -1,
	.dev	= {
		.platform_data = rb2011L_spi_info,
	},
};

static struct spi_board_info ar9342_spi_ssr = {
	.modalias = "ar9342-spi-ssr",
	.max_speed_hz = 33333334,
	.bus_num = 0,
	.chip_select = 4,
	.mode = SPI_MODE_3,
};
static struct spi_board_info *ar9342_spi_info[] = {
	&rb900_spi_misc,
	&ar9342_spi_ssr,
	NULL
};
static struct platform_device ar9342_spi_device = {
	.name	= "rb900-spi",
	.id	= -1,
	.dev	= {
		.platform_data = ar9342_spi_info,
	},
};
static struct platform_device ar9342_ts_spi_device = {
	.name	= "rb900-spi",
	.id	= -1,
	.dev	= {
		.platform_data = &(struct spi_board_info *[]) {
			&rb900_spi_misc,
			&ar9342_spi_ssr,
			&rb2011_spi_ts,
			NULL
		},
	},
};

static struct spi_board_info music_spi_nand = {
	.modalias = "music-spi-nand",
	.max_speed_hz = 33333334,
	.bus_num = 0,
	.chip_select = 2,
	.mode = SPI_MODE_0,
};
static struct spi_board_info music_spi_lcd = {
	.modalias = "rb2011-spi-lcd",
	.max_speed_hz = 15 * 1000 * 1000,
	.bus_num = 0,
	.chip_select = 5,
	.mode = SPI_MODE_0,
};
static struct rb400_spi_ctrl music_ts_ctrl = {
	.cmd_prefix = 1 << 2,	// CMD_SEL_TS for Xilinx
};
static struct spi_board_info music_spi_ts = {
	.modalias = "rb2011-spi-ts",
	.max_speed_hz = 2000 * 1000,
	.bus_num = 0,
	.chip_select = 6,
	.mode = SPI_MODE_0,
	.platform_data = (void *) 2,
	.controller_data = &music_ts_ctrl,
};

static struct spi_board_info *music_spi_info[] = {
	&rb400_spi_misc,
	&music_spi_nand,
	&music_spi_lcd,
	&music_spi_ts,
	NULL
};
static struct platform_device music_spi_device = {
	.name	= "rb400-spi",
	.id	= -1,
	.dev	= {
		.platform_data = music_spi_info,
	},
};

static struct rb400_ssr_info crs212_ssr_data = {
	.bytes = 3,
	.val = 0xffffff
};
static struct rb400_spi_ctrl crs212_ssr_ctrl = {
	.cmd_prefix = 1 << 3,	// CMD_SEL_SSR for Xilinx
	.cs_as_strobe = 1,
};
static struct spi_board_info crs212_spi_ssr = {
	.modalias = "ccs-spi-ssr",
	.max_speed_hz = 10 * 1000 * 1000,
	.bus_num = 0,
	.chip_select = 10,
	.mode = SPI_MODE_0,
	.platform_data = &crs212_ssr_data,
	.controller_data = &crs212_ssr_ctrl,
};
static struct spi_board_info *crs212_spi_info[] = {
	&rb400_spi_misc,
	&music_spi_nand,
	&music_spi_lcd,
	&music_spi_ts,
	&crs212_spi_ssr,
	NULL
};
static struct platform_device crs212_spi_device = {
	.name	= "rb400-spi",
	.id	= -1,
	.dev	= {
		.platform_data = crs212_spi_info,
	},
};

static struct spi_board_info large_spi_nor = {
	.modalias = "m25p80",
	.max_speed_hz = 80 * 1000 * 1000,
	.bus_num = 0,
	.chip_select = 4,
	.mode = SPI_MODE_0,
        .platform_data = &nor_data,
};

static struct spi_board_info slow_large_spi_nor = {
	.modalias = "m25p80",
	.max_speed_hz = 30 * 1000 * 1000,
	.bus_num = 0,
	.chip_select = 4,
	.mode = SPI_MODE_0,
        .platform_data = &nor_data,
};

static struct spi_board_info *rb900l_spi_info[] = {
	&rb400_spi_misc,
	&large_spi_nor,
	NULL
};
static struct platform_device rb900l_spi_device = {
	.name	= "rb900-spi",
	.id	= -1,
	.dev	= {
		.platform_data = rb900l_spi_info,
	},
};

static struct spi_board_info *waspL_spi_info[] = {
	&large_spi_nor,
	NULL
};
static struct platform_device waspL_spi_device = {
	.name	= "rb900-spi",
	.id	= -1,
	.dev	= {
		.platform_data = waspL_spi_info,
	},
};

static struct spi_board_info *ar9340L_spi_info[] = {
	&slow_large_spi_nor,
	&rb2011_spi_ts,
	NULL
};
static struct platform_device ar9340L_spi_device = {
	.name	= "rb900-spi",
	.id	= -1,
	.dev	= {
		.platform_data = ar9340L_spi_info,
	},
};

static struct spi_board_info *slow_waspL_spi_info[] = {
	&slow_large_spi_nor,
	NULL
};
static struct platform_device slow_waspL_spi_device = {
	.name	= "rb900-spi",
	.id	= -1,
	.dev	= {
		.platform_data = slow_waspL_spi_info,
	},
};

static struct spi_board_info *scorpionL_spi_info[] = {
	&slow_large_spi_nor,
	&rb2011_spi_ts,
	NULL
};
static struct platform_device scorpionL_spi_device = {
	.name	= "rb900-spi",
	.id	= -1,
	.dev	= {
		.platform_data = scorpionL_spi_info,
	},
};

static struct platform_device flash_nor_device = {
	.name	= "flash-nor",
	.id	= -1,
};

static struct platform_device flash_spi_device = {
	.name	= "flash-spi",
	.id	= -1,
};

static struct platform_device korina_device = {
	.name	= "korina",
	.id	= -1,
};

static struct platform_device rb100_led_device = {
	.name	= "rb100-led",
	.id	= -1,
};

static struct platform_device rb112_led_device = {
	.name	= "rb112-led",
	.id	= -1,
};

static struct platform_device rb400_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 4, GPIO),
			PLD(led1, 0, SHARED_RB400),
			PLD(led2, 1, SHARED_RB400),
			PLD(led3, 2, SHARED_RB400),
			PLD(led4, 3, SHARED_RB400),
			PLD(led5, 8, SHARED_RB400),
			0
		},
	},
};

static struct platform_device rb411l_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 4, GPIO),
			PLDI(led1, 4, SSR_RB400),
			PLDI(led2, 3, SSR_RB400),
			PLDI(led3, 2, SSR_RB400),
			PLDI(led4, 1, SSR_RB400),
			PLDI(led5, 0, SSR_RB400),
			PLDI(usb_power_off, 7, SSR_RB400),
			0
		},
	},
};

static struct platform_device rb450g_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 4, GPIO),
			PLD(led1, 8, SHARED_RB400),
			0
		},
	},
};

static struct platform_device rb493g_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 4, GPIO),
			PLD(led1, 0, SHARED_RB400),
			PLD(led2, 2, SHARED_RB400),
			PLD(led3, 1, SHARED_RB400),
			PLD(led4, 3, SHARED_RB400),
			PLD(led5, 8, SHARED_RB400),
			0
		},
	},
};

static struct platform_device rb700_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 12, SHARED_RB700),
			0
		},
	},
};

static struct platform_device rb751_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 12, SHARED_RB700),
			PLDI(wlan_led, 9, WIFI),
			0
		},
	},
};

static struct platform_device sxt5d_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 12, SHARED_RB700),
			PLD(led1, 5, WIFI),
			PLD(led2, 6, WIFI),
			PLD(led3, 7, WIFI),
			PLD(led4, 8, WIFI),
			PLD(led5, 9, WIFI),
			PLD(usb_power_off, 3, WIFI),
			0
		},
	},
};

static struct platform_device groove_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 12, SHARED_RB700),
			PLD(led1, 9, WIFI),
			PLD(led2, 8, WIFI),
			PLD(led3, 7, WIFI),
			PLD(led4, 6, WIFI),
			PLD(led5, 5, WIFI),
			PLD(usb_power_off, 3, WIFI),
			0
		},
	},
};

static struct platform_device rb711_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 1, SHARED_RB700),
			PLDI(led1, 12, SHARED_RB700),
			PLDI(led2, 13, SHARED_RB700),
			PLDI(led3, 14, SHARED_RB700),
			PLDI(led4, 15, SHARED_RB700),
			PLDI(led5, 16, SHARED_RB700),
			0
		},
	},
};

static struct platform_device rb711r3_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 1, SHARED_RB700),
			PLDI(led1, 12, SHARED_RB700),
			PLDI(led2, 14, SHARED_RB700),
			PLDI(led3, 15, SHARED_RB700),
			PLDI(led4, 16, SHARED_RB700),
			PLD(usb_power_off, 4, WIFI),
			0
		},
	},
};

static struct platform_device rb711g_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 1, SHARED_RB700),
			PLDI(led1, 12, SHARED_RB700),
			PLDI(led2, 14, SHARED_RB700),
			PLDI(led3, 15, SHARED_RB700),
			PLDI(led4, 16, SHARED_RB700),
			PLDI(usb_power_off, 13, SHARED_RB700),
			0
		},
	},
};

static struct platform_device rb711gt_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 12, SHARED_RB700),
			PLD(led1, 10, WIFI),
			PLD(led2, 12, WIFI),
			PLD(led3, 13, WIFI),
			PLD(led4, 14, WIFI),
			PLD(led5, 15, WIFI),
			PLD(usb_power_off, 0, WIFI),
			PLED(power_led, 1, WIFI, PLED_CFG_INV | PLED_CFG_ON),
			0
		},
	},
};

static struct platform_device rb951g_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 4, WIFI),
			PLDI(usb_power_off, 20, GPIO),
			PLD(wlan_led, 0, WIFI),
			PLDI(button, 15, GPIO),
			PLDI(pin_hole, 14, SHARED_GPIO),
			0
		},
	},
};

static struct platform_device rb2011L_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 4, WIFI),
			PLDI(button, 15, GPIO),
			PLDI(pin_hole, 14, SHARED_GPIO),
			0
		},
	},
};

static struct platform_device rb2011_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(sfp_led, 4, WIFI),
			PLDI(usb_power_off, 20, GPIO_OE),
			PLED(lcd, 2, GPIO, PLED_CFG_KEEP),
			PLDI(button, 15, GPIO),
			PLDI(pin_hole, 14, SHARED_GPIO),
			0
		},
	},
};

static struct platform_device crs125_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led2, 11, GPIO),
			PLDI(usb_power_off, 20, GPIO_OE),
			PLDI(button, 15, GPIO),
			PLED(lcd, 2, GPIO, PLED_CFG_KEEP),
			0
		},
	},
};

static struct platform_device crs125_wifi_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(wlan_led, 11, GPIO),
			PLDI(usb_power_off, 20, GPIO_OE),
			PLDI(button, 15, GPIO),
			PLED(lcd, 2, GPIO, PLED_CFG_KEEP),
			0
		},
	},
};

static struct platform_device rb2011r5_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(sfp_led, 4, WIFI),
			PLDI(usb_power_off, 20, GPIO_OE),
			PLDI(lcd, 0, WIFI),
			PLDI(button, 15, GPIO),
			PLDI(pin_hole, 14, SHARED_GPIO),
			0
		},
	},
};

static struct platform_device rb2011r5_user_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 4, WIFI),
			PLDI(usb_power_off, 20, GPIO_OE),
			PLDI(lcd, 0, WIFI),
			PLDI(button, 15, GPIO),
			PLDI(pin_hole, 14, SHARED_GPIO),
			0
		},
	},
};

static struct platform_device sxtg_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 12, SHARED_RB700),
			PLD(led1, 5, WIFI),
			PLD(led2, 6, WIFI),
			PLD(led3, 7, WIFI),
			PLD(led4, 8, WIFI),
			PLD(led5, 9, WIFI),
			PLD(usb_power_off, 3, WIFI),
			PLD(power_led, 1, WIFI),
			0
		},
	},
};

static struct platform_device rb750g_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 4, GPIO),
			0
		},
	},
};

static struct platform_device rb750gl_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 12, SHARED_RB700),
			PLDI(usb_power_off, 13, SHARED_RB700),
			0
		},
	},
};

static struct platform_device rb751g_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 12, SHARED_RB700),
			PLDI(usb_power_off, 13, SHARED_RB700),
			PLDI(wlan_led, 9, WIFI),
			0
		},
	},
};

static struct platform_device rb912g_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 2, SHARED_RB900),
			PLD(led1, 0, SSR_RB900),
			PLD(led2, 1, SSR_RB900),
			PLD(led3, 2, SSR_RB900),
			PLD(led4, 3, SSR_RB900),
			PLD(led5, 4, SSR_RB900),
			PLDI(usb_power_off, 6, SSR_RB900),
			PLED(power_led, 1, SHARED_RB900, PLED_CFG_ON),
			PLDI(pcie_power_off, 7, SSR_RB900),
			PLDI(button, 15, SHARED_RB900),
			PLDI(pin_hole, 14, SHARED_RB900),
			0
		},
	},
};

static struct platform_device wasp_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLED(user_led, 3, GPIO, PLED_CFG_ON), // also powerled
			PLDI(led1, 13, GPIO),
			PLDI(led2, 12, GPIO),
			PLDI(led3, 4, GPIO),
			PLDI(led4, 21, GPIO),
			PLDI(led5, 16, GPIO),
			PLDI(button, 15, GPIO),
			PLDI(usb_power_off, 20, GPIO),
			0
		},
	},
};

static struct platform_device rb922g_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 12, GPIO),
			PLDI(led1, 1, WIFI),
			PLDI(led2, 8, WIFI),
			PLDI(led3, 9, WIFI),
			PLDI(led4, 10, WIFI),
			PLDI(led5, 11, WIFI),
			PLD(usb_power_off, 13, GPIO),
			PLED(power_led, 0, WIFI, PLED_CFG_ON),
			PLDI(pcie_power_off, 17, WIFI),
			PLDI(button, 20, GPIO),
			PLDI(fan_off, 14, GPIO),
			0
		},
	},
};

static struct platform_device sxtg5c_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 16, GPIO),
			PLDI(led1, 1, WIFI),
			PLDI(led2, 8, WIFI),
			PLDI(led3, 9, WIFI),
			PLDI(led4, 10, WIFI),
			PLDI(led5, 11, WIFI),
			PLDI(usb_power_off, 11, GPIO),
			PLED(power_led, 7, WIFI, PLED_CFG_ON),
			PLDI(button, 20, GPIO),
			PLDI(pin_hole, 23, SHARED_GPIO),
			0
		},
	},
};

static struct platform_device rb951HnD_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 3, GPIO_OE),
			PLDI(led1, 13, GPIO),
			PLDI(led2, 12, GPIO),
			PLDI(led3, 4, GPIO),
			PLDI(led4, 21, GPIO),
			PLDI(led5, 16, GPIO),
			PLDI(usb_power_off, 20, GPIO),
			PLDI(wlan_led, 11, GPIO),
			PLDI(button, 15, GPIO),
			PLDI(pin_hole, 14, SHARED_GPIO),
			0
		},
	},
};

static struct platform_device sxt2d_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 3, GPIO_OE),
			PLDI(led1, 16, GPIO),
			PLDI(led2, 21, GPIO),
			PLDI(led3, 4, GPIO),
			PLDI(led4, 12, GPIO),
			PLDI(led5, 13, GPIO),
			PLDI(power_led, 11, GPIO_OE),
			PLDI(button, 15, GPIO),
			PLDI(pin_hole, 14, SHARED_GPIO),
			0
		},
	},
};

static struct platform_device sxt5n_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 3, GPIO_OE),
			PLDI(led1, 13, GPIO),
			PLDI(led2, 12, GPIO),
			PLDI(led3, 4, GPIO),
			PLDI(led4, 21, GPIO),
			PLDI(led5, 18, GPIO),
			PLDI(usb_power_off, 20, GPIO),
			PLDI(power_led, 11, GPIO_OE),
			PLDI(button, 15, GPIO),
			PLDI(pin_hole, 14, SHARED_GPIO),
			0
		},
	},
};

static struct platform_device rb911L_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 3, GPIO_OE),
			PLDI(led1, 13, GPIO),
			PLDI(led2, 12, GPIO),
			PLDI(led3, 4, GPIO),
			PLDI(led4, 21, GPIO),
			PLDI(led5, 18, GPIO),
			PLDI(usb_power_off, 20, GPIO),
			PLDI(power_led, 11, GPIO_OE),
			PLDI(button, 15, GPIO),
			PLDI(pin_hole, 14, GPIO),
			0
		},
	},
};

static struct platform_device rb953_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 16, GPIO),
			PLDI(led1, 4, SSR_RB400),
			PLDI(led2, 5, SSR_RB400),
			PLDI(led3, 6, SSR_RB400),
			PLDI(led4, 7, SSR_RB400),
			PLDI(led5, 8, SSR_RB400),
			PLDI(usb_power_off, 12, SSR_RB400),
			PLED(power_led, 9, SSR_RB400,
					PLED_CFG_INV | PLED_CFG_KEEP),
			PLDI(pcie_power_off, 10, SSR_RB400),
			PLDI(pcie2_power_off, 11, SSR_RB400),
			PLDI(button, 20, GPIO),
			PLDI(pin_hole, 23, SHARED_GPIO),
			0
		},
	},
};

static struct platform_device crs226_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLED(lcd, 7, SSR_MUSIC, PLED_CFG_INV | PLED_CFG_KEEP),
			PLDI(button, 0, GPIO),
			0
		},
	},
};

static struct platform_device ccs112_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(button, 0, GPIO),
			0
		},
	},
};

static struct platform_device rb500_led_device = {
	.name	= "rb500-led",
	.id	= -1,
};

static struct platform_device cr_led_device = {
	.name	= "cr-led",
	.id	= -1,
};

static struct platform_device mr_led_device = {
	.name	= "mr-led",
	.id	= -1,
};

static struct platform_device rb400_gpio_device = {
	.name	= "rb400-gpio",
	.id	= -1,
};

static struct platform_device ar9340_gpio_device = {
	.name	= "rb400-gpio",
	.id	= -1,
	.dev	= {
		.platform_data = (void *)RB400_GPIO_INVERT_OE,
	},
};
static struct platform_device ar9344_gpio_device = {
	.name	= "rb400-gpio",
	.id	= -1,
	.dev	= {
		.platform_data = (void *)(RB400_GPIO_INVERT_OE |
					  RB400_GPIO11_RESET_FN),
	},
};

static struct platform_device ccs_gpio_device = {
	.name	= "rb400-gpio",
	.id	= -1,
	.dev	= {
		.platform_data = (void *)RB400_GPIO_BASE2,
	},
};

static struct i2c_gpio_platform_data rb2011_i2c_gpio_data = {
    .sda_pin = 11,
    .scl_pin = 21,
    .udelay = 5,
    .timeout = 1,
    .sda_is_open_drain = 0,
    .scl_is_open_drain = 0,
    .scl_is_output_only = 0,
};
static struct platform_device rb2011_i2c_gpio_device = {
	.name	= "i2c-gpio",
	.id	= -1,
	.dev	= {
		.platform_data = &rb2011_i2c_gpio_data,
	},
};

static struct i2c_gpio_platform_data rb953_i2c_gpio_data = {
    .sda_pin = 18,
    .scl_pin = 19,
    .udelay = 5,
    .timeout = 1,
    .sda_is_open_drain = 1,
    .scl_is_open_drain = 1,
    .scl_is_output_only = 0,
};
static struct platform_device rb953_i2c_gpio_device = {
	.name	= "i2c-gpio",
	.id	= -1,
	.dev	= {
		.platform_data = &rb953_i2c_gpio_data,
	},
};

static struct platform_device rb2011_gpio_i2c_sfp_device = {
	.name	= "rb400-gpio-i2c-sfp",
	.id	= -1,
};


static int rb411_eth_port_map[] = { 1, 0, 1, -1 };
static struct platform_device rb411_eth_device = {
	.name	= "ag7100",
	.id	= -1,
	.dev	= {
		.platform_data = rb411_eth_port_map,
	},
};

static int rb411g_eth_port_map[] = { 1, 0, 0, -1 };
static struct platform_device rb411g_eth_device = {
	.name	= "ag7100",
	.id	= -1,
	.dev	= {
		.platform_data = rb411g_eth_port_map,
	},
};

static int rb433_eth_port_map[] = { 2, 1, 4, -1, 0, 175, 1, 2, -1 };
static struct platform_device rb433_eth_device = {
	.name	= "ag7100",
	.id	= -1,
	.dev	= {
		.platform_data = rb433_eth_port_map,
	},
};

static int rb433gl_eth_port_map[] = { 1, 0, 8327, 4, 1, 2, -1 };
static struct platform_device rb433gl_eth_device = {
	.name	= "ag7100",
	.id	= -1,
	.dev	= {
		.platform_data = rb433gl_eth_port_map,
	},
};

static int rb953_eth_port_map[] = { 2, 0, 8327, 0, 1, 2, 5, -1, 1, -1, -1 };
static struct platform_device rb953_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = rb953_eth_port_map,
	},
};

static int rb433ul_eth_port_map[] = { 2, 1, 4, -1, 0, 8236, 0, 1, -1 };
static struct platform_device rb433ul_eth_device = {
	.name	= "ag7100",
	.id	= -1,
	.dev	= {
		.platform_data = rb433ul_eth_port_map,
	},
};

static int rb435g_eth_port_map[] = { 2, 1, 4, -1, 0, 8316, 0, 1, -1 };
static struct platform_device rb435g_eth_device = {
	.name	= "ag7100",
	.id	= -1,
	.dev	= {
		.platform_data = rb435g_eth_port_map,
	},
};

static int rb450_eth_port_map[] = { 2, 1, 4, -1, 0, 175, 3, 2, 1, 0, -1 };
static struct platform_device rb450_eth_device = {
	.name	= "ag7100",
	.id	= -1,
	.dev	= {
		.platform_data = rb450_eth_port_map,
	},
};

static int rb450g_eth_port_map[] = { 2, 1, 4, -1, 0, 8316, 0, 3, 2, 1, -1 };
static struct platform_device rb450g_eth_device = {
	.name	= "ag7100",
	.id	= -1,
	.dev	= {
		.platform_data = rb450g_eth_port_map,
	},
};

static int rb711_eth_port_map[] = { 1, 0, 4, -1 };
static struct platform_device rb711_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = rb711_eth_port_map,
	},
};

static int rb711r3_eth_port_map[] = { 1, 1, 7240, 0, -1 };
static struct platform_device rb711r3_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = rb711r3_eth_port_map,
	},
};

static int mAP_eth_port_map[] = { 1, 1, 7240, 0, 1, -1 };
static struct platform_device mAP_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = mAP_eth_port_map,
	},
};

static int rb750_eth_port_map[] = { 2, 0, 4, -1, 1, 7240, 3, 2, 1, 0, -1 };
static struct platform_device rb750_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = rb750_eth_port_map,
	},
};

static int rb493g_eth_port_map[] = { 3, 0, 8316, 4, -1,
				     1, 8316, 1, 2, 3, 0, -1,
				     0, 8316, 1, 3, 2, 0, -1 };
static struct platform_device rb493g_eth_device = {
	.name	= "ag7100",
	.id	= -1,
	.dev	= {
		.platform_data = rb493g_eth_port_map,
	},
};

static int rb750g_eth_port_map[] = { 1, 0, 8316, 0, 1, 2, 3, 4, -1 };
static struct platform_device rb750g_eth_device = {
	.name	= "ag7100",
	.id	= -1,
	.dev	= {
		.platform_data = rb750g_eth_port_map,
	},
};

static int rb711g_eth_port_map[] = { 1, 0, 0, -1 };
static struct platform_device rb711g_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = rb711g_eth_port_map,
	},
};

static int rb922g_eth_port_map[] = { 2, 0, 4, -1, 1, -1, -1 };
static struct platform_device rb922g_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = rb922g_eth_port_map,
	},
};

static int rb951HnD_eth_port_map[] = { 1, 1, 8227, 4, 3, 2, 1, 0, -1 };
static struct platform_device rb951HnD_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = rb951HnD_eth_port_map,
	},
};

static int sxt2d_eth_port_map[] = { 1, 1, 8227, 0, -1 };
static struct platform_device sxt2d_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = sxt2d_eth_port_map,
	},
};

static struct platform_device plchb_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			1, 1, 8227, 0, BIT(16 + 3) | 4, -1
		},
	},
};

static int rb2011S_eth_port_map[] = { 2,
				      0, 8327, 5, 0, 1, 2, 3, 4, -1,
				      1, 8227, 4, 3, 2, 1, 0, -1 };
static struct platform_device rb2011S_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = rb2011S_eth_port_map,
	},
};

static int rb2011L_eth_port_map[] = { 2,
				      0, 8327, 0, 1, 2, 3, 4, -1,
				      1, 8227, 4, 3, 2, 1, 0, -1 };
static struct platform_device rb2011L_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = rb2011L_eth_port_map,
	},
};

static int rb951G_eth_port_map[] = { 1,
				     0, 8327, 0, 1, 2, 3, 4, -1, };
static struct platform_device rb951G_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = rb951G_eth_port_map,
	},
};

static int rb750gl_eth_port_map[] = { 1, 0, 8327, 0, 1, 2, 3, 4, -1 };
static struct platform_device rb750gl_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = rb750gl_eth_port_map,
	},
};

static int groove52_eth_port_map[] = { 1, 0, 0, -1 };
static struct platform_device groove52_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = groove52_eth_port_map,
	},
};

static int rb912g_eth_port_map[] = { 1, 0, 0, -1 };
static struct platform_device rb912g_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = rb912g_eth_port_map,
	},
};

static int rb493_eth_port_map[] = { 2, 0, 1, -1,
				    1, 178, 0, 1, 2, 3, 4, 5, 6, 7, -1 };
static struct platform_device rb493_eth_device = {
	.name	= "ag7100",
	.id	= -1,
	.dev	= {
		.platform_data = rb493_eth_port_map,
	},
};

static int rb112_eth_port_map[] = { -1, 0, -1 };

static struct platform_device rb112_eth_device = {
	.name	= "admtek",
	.id	= -1,
	.dev	= {
		.platform_data = rb112_eth_port_map,
	},
};

static int rb153_eth_port_map[] = { -1, 0, 4, 3, 2, 1, -1 };

static struct platform_device rb153_eth_device = {
	.name	= "admtek",
	.id	= -1,
	.dev	= {
		.platform_data = rb153_eth_port_map,
	},
};

static int rb133_eth_port_map[] = { -1, 2, 1, 0, -1 };

static struct platform_device rb133_eth_device = {
	.name	= "admtek",
	.id	= -1,
	.dev	= {
		.platform_data = rb133_eth_port_map,
	},
};

static int rb133c_eth_port_map[] = { -1, 2, -1 };

static struct platform_device rb133c_eth_device = {
	.name	= "admtek",
	.id	= -1,
	.dev	= {
		.platform_data = rb133c_eth_port_map,
	},
};

static int rb192_eth_port_map[] = { 5, 0, 4, 3, 2, 1, 101, 102, 103, 104, -1 };

static struct platform_device rb192_eth_device = {
	.name	= "admtek",
	.id	= -1,
	.dev	= {
		.platform_data = rb192_eth_port_map,
	},
};

static int mr_eth_port_map[] = { -1, 4, 3, 2, 1, 0, -1 };

static struct platform_device mr_eth_device = {
	.name	= "admtek",
	.id	= -1,
	.dev	= {
		.platform_data = mr_eth_port_map,
	},
};

static struct platform_device cr_eth_device = {
	.name	= "cr-ether",
	.id	= -1,
};

static struct platform_device cr_wlan_device = {
	.name	= "cr-wlan",
	.id	= -1,
};

static struct platform_device ar9330_wlan_device = {
	.name	= "rb750-wlan",
	.id	= -1,
};

static struct platform_device pci_wlan_device = {
	.name	= "wlan-data-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(struct rb_wlan_device []) {
			{ .pci_domain_nr = 1, .caldata_instance = 1 },
			{ -1, -1 }},
	},
};

static struct platform_device rb500_cf_device = {
	.name	= "rb500-cf",
	.id	= -1,
};

static struct platform_device rb100_cf_device = {
	.name	= "rb100-cf",
	.id	= -1,
};

static u64 dmamask = ~(u32) 0;

static struct resource rb400_ohci_resources[] = {
	[0] = {
		.start		= 0x1c000000,
		.end		= 0x1cffffff,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= 22,
		.end		= 22,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct platform_device rb400_ohci_device = {
	.name		= "rb400-ohci",
	.id		= 0,
	.dev		= {
		.dma_mask		= &dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(rb400_ohci_resources),
	.resource	= rb400_ohci_resources,
};

static struct resource rb400_ehci_resources[] = {
	[0] = {
		.start		= 0x1b000000,
		.end		= 0x1bffffff,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= 3,
		.end		= 3,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct platform_device rb400_ehci_device = {
	.name		= "rb400-ehci",
	.id		= 0,
	.dev		= {
		.dma_mask		= &dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(rb400_ehci_resources),
	.resource	= rb400_ehci_resources,
};

static struct poeout_port rb951HnD_poeout_port = {
	.eth_port = 4,
	.gpo_on = 2,
	.gpi_status = 17,
};
static struct platform_device rb951HnD_poeout_device = {
	.name	= "poe-out-simple",
	.id	= -1,
	.dev	= {
		.platform_data = &rb951HnD_poeout_port,
	},
};

static struct poeout_port rb952_poeout_port = {
	.eth_port = 4,
	.gpo_on = 14,
	.gpi_status = 12,
};
static struct platform_device rb952_poeout_device = {
	.name	= "poe-out-simple",
	.id	= -1,
	.dev	= {
		.platform_data = &rb952_poeout_port,
	},
};

static struct poeout_port rb2011r5_poeout_port = {
	.eth_port = 10,
	.gpo_on = 2,
	.gpi_status = 13,
	.gpi_status_valid = 3,
};
static struct platform_device rb2011r5_poeout_device = {
	.name	= "poe-out-simple",
	.id	= -1,
	.dev	= {
		.platform_data = &rb2011r5_poeout_port,
	},
};

static struct poeout_port rb2011Lr5_poeout_port = {
	.eth_port = 9,
	.gpo_on = 2,
	.gpi_status = 13,
	.gpi_status_valid = 3,
};
static struct platform_device rb2011Lr5_poeout_device = {
	.name	= "poe-out-simple",
	.id	= -1,
	.dev	= {
		.platform_data = &rb2011Lr5_poeout_port,
	},
};

static struct poeout_port mAP_poeout_port = {
        .eth_port = 1,
        .gpo_on = 26,
        .gpi_status = 27,
};
static struct platform_device mAP_poeout_device = {
        .name   = "poe-out-simple",
        .id     = -1,
        .dev    = {
                .platform_data = &mAP_poeout_port,
        },
};

static struct platform_device *rb112_devices[] = {
	&rb100_uart_device,
	&rb100_nand_device,
	&rb112_eth_device,
	&flash_nor_device,
	&rb112_led_device,
};

static struct platform_device *rb153_devices[] = {
	&rb100_uart_device,
	&rb100_nand_device,
	&rb153_eth_device,
	&rb100_cf_device,
	&flash_nor_device,
	&rb100_led_device,
};

static struct platform_device *rb133_devices[] = {
	&rb100_uart_device,
	&rb100_nand_device,
	&rb133_eth_device,
	&flash_nor_device,
	&rb100_led_device,
};

static struct platform_device *rb133c_devices[] = {
	&rb100_uart_device,
	&rb100_nand_device,
	&rb133c_eth_device,
	&flash_nor_device,
	&rb100_led_device,
};

static struct platform_device *rb192_devices[] = {
	&rb100_uart_device,
	&rb100_nand_device,
	&rb192_eth_device,
	&flash_nor_device,
	&rb100_led_device,
};

static struct platform_device *mr_devices[] = {
	&rb100_uart_device,
	&mr_nand_device,
	&mr_eth_device,
	&flash_nor_device,
	&mr_led_device,
};

static struct platform_device *rb500_devices[] = {
	&rb500_nand_device,
	&korina_device,
	&rb500_cf_device,
	&flash_nor_device,
	&rb500_led_device,
};

static struct platform_device *rb500r5_devices[] = {
	&rb500r5_nand_device,
	&korina_device,
	&rb500_cf_device,
	&flash_nor_device,
	&rb500_led_device,
};

static struct platform_device *rb411_devices[] = {
	&rb400_spi_device,
	&rb411_eth_device,
	&rb400_led_device,
};

static struct platform_device *rb411ul_devices[] = {
	&rb400_ehci_device,
	&rb400_ohci_device,
	&rb400_gpio_device,
	&rb400l_spi_device,
	&rb750g_nand_device,
	&rb411_eth_device,
	&rb411l_led_device,
};

static struct platform_device *rb411l_devices[] = {
	&rb400_gpio_device,
	&rb400l_spi_device,
	&rb750g_nand_device,
	&rb411_eth_device,
	&rb411l_led_device,
};

static struct platform_device *rb411g_devices[] = {
	&rb400_ehci_device,
	&rb400_ohci_device,
	&rb400_gpio_device,
	&rb400l_spi_device,
	&rb750g_nand_device,
	&rb411g_eth_device,
	&rb411l_led_device,
};

static struct platform_device *rb411u_devices[] = {
	&rb400_ehci_device,
	&rb400_ohci_device,
	&rb400_spi_device,
	&rb411_eth_device,
	&rb400_led_device,
};

static struct platform_device *rb433_devices[] = {
	&rb433_spi_device,
	&rb433_eth_device,
	&rb400_led_device,
};

static struct platform_device *rb433u_devices[] = {
	&rb400_ehci_device,
	&rb400_ohci_device,
	&rb433_spi_device,
	&rb433_eth_device,
	&rb400_led_device,
};

static struct platform_device *rb433gl_devices[] = {
	&rb400_ehci_device,
	&rb400_ohci_device,
	&rb400_gpio_device,
	&rb400l_spi_device,
	&rb433gl_eth_device,
	&rb750g_nand_device,
	&rb411l_led_device,
};

static struct platform_device *rb433ul_devices[] = {
	&rb400_ehci_device,
	&rb400_ohci_device,
	&rb400_gpio_device,
	&rb400l_spi_device,
	&rb433ul_eth_device,
	&rb750g_nand_device,
	&rb411l_led_device,
};

static struct platform_device *rb433l_devices[] = {
	&rb400_gpio_device,
	&rb400l_spi_device,
	&rb433ul_eth_device,
	&rb750g_nand_device,
	&rb411l_led_device,
};

static struct platform_device *rb435g_devices[] = {
	&rb400_ehci_device,
	&rb400_ohci_device,
	&rb433_spi_device,
	&rb435g_eth_device,
	&rb400_led_device,
};

static struct platform_device *rb450_devices[] = {
	&rb400_spi_device,
	&rb450_eth_device,
	&rb400_led_device,
};

static struct platform_device *rb450g_devices[] = {
	&rb433_spi_device,
	&rb450g_eth_device,
	&rb450g_led_device,
};

static struct platform_device *rb493_devices[] = {
	&rb400_spi_device,
	&rb493_eth_device,
	&rb400_led_device,
};

static struct platform_device *rb493g_devices[] = {
	&rb400_ehci_device,
	&rb400_ohci_device,
	&rb433_spi_device,
	&rb493g_eth_device,
	&rb493g_led_device,
};

static struct platform_device *rb750g_devices[] = {
	&rb750_spi_device,
	&rb750g_nand_device,
	&rb750g_eth_device,
	&rb750g_led_device,
};

static struct platform_device *cr_devices[] = {
	&cr_nand_device,
	&cr_eth_device,
	&cr_wlan_device,
	&flash_spi_device,
	&cr_led_device,
};

static struct resource rb700_ohci_resources[] = {
	[0] = {
		.start		= 0x1b000000,
		.end		= 0x1bffffff,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= 3,
		.end		= 3,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct platform_device rb700_ohci_device = {
	.name		= "rb400-ohci",
	.id		= 0,
	.dev		= {
		.dma_mask		= &dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(rb700_ohci_resources),
	.resource	= rb700_ohci_resources,
};

static struct resource rb700_ehci_resources[] = {
	[0] = {
		.start		= 0x1b000000,
		.end		= 0x1b3fffff,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= 3,
		.end		= 3,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct resource rb900_ehci_resources[] = {
	[0] = {
		.start		= 0x1b400000,
		.end		= 0x1b7fffff,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= 3,
		.end		= 3,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct platform_device rb700_ehci_device = {
	.name		= "rb400-ehci",
	.id		= 0,
	.dev		= {
		.dma_mask		= &dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(rb700_ehci_resources),
	.resource	= rb700_ehci_resources,
};

static struct platform_device rb900_ehci_device = {
	.name		= "rb900-ehci",
	.id		= 1,
	.dev		= {
		.dma_mask		= &dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(rb900_ehci_resources),
	.resource	= rb900_ehci_resources,
};

static struct platform_device *rb711_devices[] = {
	&rb700_ehci_device,
	&rb700_ohci_device,
	&rb711_spi_device,
	&rb700_nand_device,
	&rb711_led_device,
	&rb711_eth_device,
};

static struct platform_device *rb711r3_devices[] = {
	&rb700_ehci_device,
	&rb700_ohci_device,
	&rb711_spi_device,
	&rb700_nand_device,
	&rb711r3_led_device,
	&rb711r3_eth_device,
};

static struct platform_device *rb_omni_5fe_devices[] = {
	&rb700_ehci_device,
	&rb700_ohci_device,
	&omni_spi_device,
	&rb700_nolatch_nand_device,
	&sxt5d_led_device,
	&rb750_eth_device,
};

static struct platform_device *rb751g_devices[] = {
	&rb700_ehci_device,
	&rb700_ohci_device,
	&rb711_spi_device,
	&rb750gl_eth_device,
	&rb700_nolatch_nand_device,
	&rb751g_led_device,
};

static struct platform_device *rb711g_devices[] = {
	&rb700_ehci_device,
	&rb700_ohci_device,
	&rb711_spi_device,
	&rb700_nand_device,
	&rb711g_eth_device,
	&rb711g_led_device,
};

static struct platform_device *rb711gt_devices[] = {
	&rb700_ehci_device,
	&rb700_ohci_device,
	&rb711_spi_device,
	&rb700_nolatch_nand_device,
	&rb711g_eth_device,
	&rb711gt_led_device,
};

static struct platform_device *rb_sxtg_devices[] = {
	&rb700_ehci_device,
	&rb700_ohci_device,
	&rb711_spi_device,
	&rb700_nolatch_nand_device,
	&rb711g_eth_device,
	&sxtg_led_device,
};

static struct hwopt_device rb912g_devices[] = {
	{ &rb700_ehci_device, 0, 0 },
	{ &ar9342_spi_device, 0, HW_OPT_HAS_TS_FOR_ADC },
	{ &ar9342_ts_spi_device, HW_OPT_HAS_TS_FOR_ADC, 0 },
	{ &rb912g_eth_device, 0, 0 },
	{ &ar9342_nand_device, 0, 0 },
	{ &rb912g_led_device, 0, 0 },
	{ &ar9330_wlan_device, 0, 0 },
};

static struct hwopt_device rb750gl_devices[] = {
	{ &rb700_ehci_device, HW_OPT_HAS_USB, 0 },
	{ &rb700_ohci_device, HW_OPT_HAS_USB, 0 },
	{ &rb711_spi_device, 0, 0 },
	{ &rb700_nolatch_nand_device, 0, 0 },
	{ &rb750gl_led_device, 0, 0 },
	{ &rb750gl_eth_device, 0, 0 },
};

static struct spi_board_info ar9344_spi_attiny = {
	.modalias = "rb700-spi-attiny",
	.max_speed_hz = 2 * 1000 * 1000,
	.bus_num = 0,
	.chip_select = 6,
	.mode = SPI_MODE_0,
	.platform_data = (void *) 1,	/* poe chan map to eth port: ch1=eth2 */
};
static struct spi_board_info *wasp_spi_info[] = {
	&rb900_spi_misc,
	&ar9344_spi_attiny,
	&slow_large_spi_nor,
	NULL
};

static struct platform_device wasp_spi_device = {
	.name	= "rb900-spi",
	.id	= -1,
	.dev	= {
		.platform_data = wasp_spi_info,
	},
};

static struct platform_device *omnitik_wasp_devices[] = {
	&rb700_ehci_device,
	&ar9340_gpio_device,
	&wasp_spi_device,
	&wasp_led_device,
	&rb951HnD_eth_device,
	&ar9330_wlan_device,
};

static struct platform_device qrtg_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 2, GPIO),
			PLD(led1, 0, GPIO),
			PLD(led2, 3, GPIO),
			PLD(led3, 12, GPIO),
			PLD(led4, 13, GPIO),
			PLD(led5, 14, GPIO),
			PLED(power_led, 1, GPIO, PLED_CFG_ON),
			PLDI(button, 15, GPIO),
			0
		},
	},
};
static struct platform_device *qrtg_devices[] = {
	&ar9330_wlan_device,
	&ar9340L_spi_device,
	&rb711g_eth_device,
	&qrtg_led_device,
};

static struct hwopt_device rb951hnd_devices[] = {
	{ &rb700_ehci_device, HW_OPT_HAS_USB, 0 },
	{ &rb2011L_spi_device, 0, 0 },
	{ &ar9340_gpio_device, 0, 0 },
	{ &rb951HnD_poeout_device, HW_OPT_HAS_POE_OUT, 0 },
	{ &rb951HnD_eth_device, 0, 0 },
	{ &rb951HnD_led_device, 0, 0 },
	{ &ar9344_nand_device, 0, 0 },
	{ &ar9330_wlan_device, 0, 0 },
};

static struct platform_device *sxt2d_devices[] = {
	&rb700_ehci_device,
	&rb2011L_spi_device,
	&sxt2d_led_device,
	&sxt2d_eth_device,
	&ar9344_nand_device,
	&ar9330_wlan_device,
};

static struct platform_device *rb911L_devices[] = {
	&rb700_ehci_device,
	&slow_waspL_spi_device,
	&rb911L_led_device,
	&sxt2d_eth_device,
	&ar9330_wlan_device,
};

static struct platform_device lhg_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 20, GPIO),
			PLDI(led1, 13, GPIO),
			PLDI(led2, 12, GPIO),
			PLDI(led3, 4, GPIO),
			PLDI(led4, 21, GPIO),
			PLDI(led5, 18, GPIO),
			PLDI(link_act_led, 14, GPIO),
			PLED(power_led, 11, GPIO, PLED_CFG_ON),
			PLDI(button, 15, GPIO),
			0
		},
	},
};

static struct platform_device *lhg_devices[] = {
	&slow_waspL_spi_device,
	&lhg_led_device,
	&sxt2d_eth_device,
	&ar9330_wlan_device,
};

static struct hwopt_device sxt5n_devices[] = {
	{ &rb700_ehci_device, HW_OPT_HAS_USB, 0 },
	{ &rb2011_spi_device, HW_OPT_HAS_TS_FOR_ADC, HW_OPT_NO_NAND },
	{ &rb2011L_spi_device, 0, HW_OPT_HAS_TS_FOR_ADC | HW_OPT_NO_NAND },
	{ &waspL_spi_device, HW_OPT_NO_NAND, 0 },
	{ &sxt5n_led_device, 0, 0 },
	{ &sxt2d_eth_device, 0, 0 },
	{ &ar9344_nand_device, 0, HW_OPT_NO_NAND },
	{ &ar9330_wlan_device, 0, 0 },
};

static struct hwopt_device sxtg5p_devices[] = {
	{ &rb700_ehci_device, 0, 0 },
	{ &rb2011_spi_device, 0, HW_OPT_NO_NAND },
	{ &scorpionL_spi_device, HW_OPT_NO_NAND, 0 },
	{ &sxt5n_led_device,  0, HW_OPT_NO_NAND },
	{ &rb911L_led_device, HW_OPT_NO_NAND, 0 },
	{ &rb711g_eth_device, 0, 0 },
	{ &ar9344_nand_device, 0, HW_OPT_NO_NAND },
	{ &ar9330_wlan_device, 0, 0 },
};

static struct platform_device *rb2011G_devices[] = {
	&rb700_ehci_device,
	&rb2011_spi_device,
	&rb2011S_eth_device,
	&ar9344_nand_device,
	&ar9330_wlan_device,
	&rb2011_led_device,
};

static struct platform_device *rb2011US_devices[] = {
	&rb700_ehci_device,
	&rb2011_spi_device,
	&rb2011S_eth_device,
	&ar9344_nand_device,
	&rb2011_led_device,
};

static struct platform_device *rb2011L_devices[] = {
	&rb2011L_spi_device,
	&rb2011L_led_device,
	&rb2011L_eth_device,
	&ar9344_nand_device,
};

static struct platform_device *rb2011LS_devices[] = {
	&rb2011L_spi_device,
	&rb2011S_eth_device,
	&ar9344_nand_device,
	&rb2011_led_device,
};

static struct hwopt_device rb2011r5_devices[] = {
	{ &rb700_ehci_device, HW_OPT_HAS_USB, 0 },
	{ &rb2011_spi_device, HW_OPT_HAS_LCD, 0 },
	{ &rb2011L_spi_device, 0, HW_OPT_HAS_LCD },
	{ &rb2011r5_poeout_device, HW_OPT_HAS_POE_OUT | HW_OPT_HAS_SFP, 0 },
	{ &rb2011Lr5_poeout_device, HW_OPT_HAS_POE_OUT, HW_OPT_HAS_SFP },
	{ &rb2011S_eth_device, HW_OPT_HAS_SFP, 0 },
	{ &rb2011L_eth_device, 0, HW_OPT_HAS_SFP },
	{ &ar9344_nand_device, 0, 0 },
	{ &ar9330_wlan_device, HW_OPT_HAS_WIFI, 0 },
	{ &rb2011r5_led_device, HW_OPT_HAS_SFP, 0 },
	{ &rb2011r5_user_led_device, 0, HW_OPT_HAS_SFP },
	{ &ar9344_gpio_device, 0, 0 },
	{ &rb2011_i2c_gpio_device, HW_OPT_HAS_SFP, 0 },
	{ &rb2011_gpio_i2c_sfp_device, HW_OPT_HAS_SFP, 0 },
};

static struct platform_device *rb951G_devices[] = {
	&rb700_ehci_device,
	&rb2011_spi_device,
	&rb951G_eth_device,
	&ar9344_nand_device,
	&ar9330_wlan_device,
	&rb951g_led_device,
};

static struct platform_device *rb953gs_devices[] = {
	&rb700_ehci_device,
	&rb900_ehci_device,
	&rb2011L_spi_device,
	&ar9344_nand_device,
	&ar9330_wlan_device,
	&rb953_eth_device,
	&ar9340_gpio_device,
	&rb953_i2c_gpio_device,
	&rb2011_gpio_i2c_sfp_device,
	&rb953_led_device,
};

static struct spi_board_info *rb941_spi_info[] = {
	&rb900_spi_misc,
	&slow_large_spi_nor,
	NULL
};
static struct platform_device rb941_spi_device = {
	.name	= "rb900-spi",
	.id	= -1,
	.dev	= {
		.platform_data = rb941_spi_info,
	},
};

static struct rb400_ssr_info hb_ssr_data = {
	.strobe = 11,
	.bytes = 1,
	.val = 0xff
};
static struct spi_board_info hb_ssr_spi_misc = {
	.modalias = "rb400-spi-misc",
	.max_speed_hz = 15000000,
	.bus_num = 0,
	.chip_select = 0,
	.mode = SPI_MODE_3,
	.platform_data = &hb_ssr_data,
};

static struct spi_board_info *hb_ssr_spi_info[] = {
	&hb_ssr_spi_misc,
	&slow_large_spi_nor,
	NULL
};
static struct platform_device hb_ssr_spi_device = {
	.name	= "rb900-spi",
	.id	= -1,
	.dev	= {
		.platform_data = hb_ssr_spi_info,
	},
};

static struct platform_device rb750r2_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 4, GPIO),
			PLDI(led1, 0, SSR_RB400),
			PLDI(led2, 1, SSR_RB400),
			PLDI(led3, 2, SSR_RB400),
			PLDI(led4, 3, SSR_RB400),
			PLDI(led5, 4, SSR_RB400),
			PLDI(usb_power_off, 5, SSR_RB400),
			PLDI(wlan_led, 6, SSR_RB400),
			PLDI(button, 16, GPIOLIB),
			0
		},
	},
};

static struct platform_device rb952_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 4, GPIO),
			PLDI(led1, 0, SSR_RB400),
			PLDI(led2, 1, SSR_RB400),
			PLDI(led3, 2, SSR_RB400),
			PLDI(led4, 3, SSR_RB400),
			PLDI(led5, 4, SSR_RB400),
			PLDI(usb_power_off, 5, SSR_RB400),
			PLDI(button, 16, GPIOLIB),
			0
		},
	},
};

static struct platform_device *rb952_devices[] = {
	&rb700_ehci_device,
	&ar9340_gpio_device,
	&hb_ssr_spi_device,
	&rb952_led_device,
	&rb952_poeout_device,
	&rb951HnD_eth_device,
	&ar9330_wlan_device,
};

static struct platform_device *rb951Ui2nD_devices[] = {
	&rb700_ehci_device,
	&ar9340_gpio_device,
	&hb_ssr_spi_device,
	&rb750r2_led_device,
	&rb952_poeout_device,
	&rb951HnD_eth_device,
	&ar9330_wlan_device,
};

static struct platform_device wsAP_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 4, GPIO),
			PLDI(led1, 0, SSR_RB400),
			PLDI(led2, 1, SSR_RB400),
			PLDI(led3, 2, SSR_RB400),
			PLDI(led4, 3, SSR_RB400),
			PLDI(led5, 4, SSR_RB400),
			PLDI(usb_power_off, 5, SSR_RB400),
			PLED(power_led, 6, SSR_RB400, PLED_CFG_ON),
			PLDI(button, 16, GPIO),
			PLDI(mode_button, 9, GPIO),
			0
		},
	},
};
static struct platform_device wsAP_poeout_device = {
	.name	= "poe-out-simple",
	.id	= -1,
	.dev	= {
		.platform_data = &(struct poeout_port){
			.eth_port = 2,
			.gpo_on = 14,
			.gpi_status = 12,
		},
	},
};
static int wsAP_eth_port_map[] = { 1, 1, 8227, 0, 1, 2, -1 };
static struct platform_device wsAP_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = wsAP_eth_port_map,
	},
};

static struct platform_device *wsAP_devices[] = {
	&rb700_ehci_device,
	&ar9340_gpio_device,
	&hb_ssr_spi_device,
	&wsAP_led_device,
	&wsAP_poeout_device,
	&wsAP_eth_device,
	&ar9330_wlan_device,
};

static struct platform_device hex_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 4, GPIO),
			PLDI(led1, 0, SSR_RB400),
			PLDI(led2, 1, SSR_RB400),
			PLDI(led3, 2, SSR_RB400),
			PLDI(led4, 3, SSR_RB400),
			PLDI(led5, 4, SSR_RB400),
			PLDI(usb_power_off, 5, SSR_RB400),
			0
		},
	},
};

static struct spi_board_info *rb750r2_spi_info[] = {
	&hb_ssr_spi_misc,
	&slow_large_spi_nor,
	&rb700_spi_attiny,
	NULL
};
static struct platform_device rb750r2_spi_device = {
	.name	= "rb900-spi",
	.id	= -1,
	.dev	= {
		.platform_data = rb750r2_spi_info,
	},
};

static struct hwopt_device rb750r2_devices[] = {
	{ &rb700_ehci_device, HW_OPT_HAS_USB, 0 },
	{ &ar9340_gpio_device, 0, 0 },
	{ &rb750r2_spi_device, HW_OPT_HAS_ATTINY, 0 },
	{ &hb_ssr_spi_device, 0, HW_OPT_HAS_ATTINY },
	{ &rb750r2_led_device, HW_OPT_HAS_WIFI, 0 },
	{ &hex_led_device, 0, HW_OPT_HAS_WIFI },
	{ &rb951HnD_eth_device, 0, 0 },
	{ &ar9330_wlan_device, HW_OPT_HAS_WIFI, 0 },
};

static int rbHAP_eth_port_map[] = { 1, 1, 8227, 3, 2, 1, 0, -1 };
static struct platform_device rbHAP_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = rbHAP_eth_port_map,
	},
};

static struct platform_device rbHAPplc_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			1, 1, 8227, 3, 2, 1, 0, BIT(16 + 3) | 4, -1
		},
	},
};

static struct platform_device rbHAP_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 14, GPIO),
			PLDI(button, 16, GPIOLIB),
			PLDI(mode_button, 9, GPIO),
			0
		},
	},
};

static struct hwopt_device rb941HL_devices[] = {
	{ &ar9340_gpio_device, 0, 0 },
	{ &rbHAP_led_device, 0, 0 },
	{ &rb941_spi_device, 0, 0 },
	{ &ar9330_wlan_device, 0, 0 },
	{ &rbHAP_eth_device, 0, HW_OPT_HAS_PLC },
	{ &rbHAPplc_eth_device, HW_OPT_HAS_PLC, 0 },
};

static struct platform_device rb931_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			1, 1, 8227, 2, 1, 0, -1
		},
	},
};
static struct platform_device rb931plc_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			1, 1, 8227, 2, 1, 0, BIT(16 + 3) | 4, -1
		},
	},
};

static struct platform_device rb931_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 1, GPIO),
			PLDI(button, 0, GPIOLIB),
			0
		},
	},
};
static struct platform_device rb931mb_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 1, GPIO),
			PLDI(button, 0, GPIOLIB),
			PLDI(mode_button, 9, GPIOLIB),
			0
		},
	},
};
static struct hwopt_device rb931_devices[] = {
	{ &ar9340_gpio_device, 0, 0 },
	{ &rb931_led_device, 0, HW_OPT_HAS_PLC },
	{ &rb931mb_led_device, HW_OPT_HAS_PLC, 0 },
	{ &rb941_spi_device, 0, 0 },
	{ &ar9330_wlan_device, 0, 0 },
	{ &rb931_eth_device, 0, HW_OPT_HAS_PLC },
	{ &rb931plc_eth_device, HW_OPT_HAS_PLC, 0 },
};

static struct platform_device wap_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 14, GPIO),
			PLDI(wlan_led, 11, GPIO),
			0
		},
	},
};

static struct platform_device *wap_devices[] = {
	&wap_led_device,
	&hb_ssr_spi_device,
	&ar9330_wlan_device,
	&sxt2d_eth_device
};

static struct platform_device cap_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(led1, 14, GPIO),
			PLDI(led2, 12, GPIO),
			PLDI(led3, 11, GPIO),
			PLDI(led4, 04, GPIO),
			PLED(all_leds, 13, GPIO, PLED_CFG_ON),
			0
		},
	},
};

static struct platform_device *cap_devices[] = {
	&cap_led_device,
	&hb_ssr_spi_device,
	&ar9330_wlan_device,
	&sxt2d_eth_device
};

static struct platform_device lhg_hb_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 14, GPIO),
			PLDI(led1, 11, GPIO),
			PLDI(led2,  9, GPIO),
			PLDI(led3, 12, GPIO),
			PLDI(led4, 13, GPIO),
			PLDI(led5, 16, GPIO),
			PLED(power_led, 17, GPIO, PLED_CFG_ON),
			0
		},
	},
};

static struct platform_device *lhg_hb_devices[] = {
	&lhg_hb_led_device,
	&hb_ssr_spi_device,
	&ar9330_wlan_device,
	&sxt2d_eth_device
};

static struct platform_device ltap_hb_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 14, GPIO),
			PLD(wlan_led, 11, GPIO),
			PLD(sfp_led, 4, GPIO),
			PLDI(button, 16, GPIOLIB),
			PLDI(mode_button, 3, GPIOLIB),
			PLDI(pcie_power_off, 13, GPIOLIB),
			PLD(sim_select, 15, GPIOLIB),
                        PLED(gps_mux, 0, GPIOLIB, PLED_CFG_KEEP),
                        PLD(gps_ant_select, 1, GPIOLIB),
                        PLD(gps_reset, 2, GPIOLIB),
			0
		},
	},
};

static struct hwopt_device ltap_hb_devices[] = {
	{ &rb700_ehci_device, HW_OPT_HAS_USB, 0 },
	{ &ar9340_gpio_device, 0, 0 },
	{ &ltap_hb_led_device, 0, 0 },
	{ &slow_waspL_spi_device, 0, 0 },
	{ &ar9330_wlan_device, HW_OPT_HAS_WIFI, 0 },
	{ &sxt2d_eth_device, 0, HW_OPT_HAS_PLC },
	{ &plchb_eth_device, HW_OPT_HAS_PLC, 0 },
	{ &(struct platform_device) {
	    .name = "gpio-uart-gps",
	    .id = -1,
	}, 0, 0 },
};

static struct platform_device wap_lte_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 14, GPIO),
			PLDI(led1, 12, GPIO),
			PLDI(led2, 13, GPIO),
			PLD(led3, 3, GPIO),
			PLDI(pcie_power_off, 15, GPIO),
			PLED(control, 10, GPIO, PLED_CFG_INV | PLED_CFG_ON),
			0
		},
	},
};

static struct platform_device *wap_lte_devices[] = {
	&rb700_ehci_device,
	&wap_lte_led_device,
	&ar9340L_spi_device,
	&ar9330_wlan_device,
	&sxt2d_eth_device
};

static struct platform_device mapl_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 14, GPIO),
			PLD(wlan_led, 11, GPIO),
			PLD(sfp_led, 4, GPIO),
			PLED(power_led, 17, GPIO, PLED_CFG_ON),
			PLDI(button, 16, GPIOLIB),
			0
		},
	},
};

static struct hwopt_device mapl_devices[] = {
	{ &rb700_ehci_device, HW_OPT_HAS_USB, 0 },
	{ &ar9340_gpio_device, 0, 0 },
	{ &mapl_led_device, 0, 0 },
	{ &hb_ssr_spi_device, 0, 0 },
	{ &ar9330_wlan_device, HW_OPT_HAS_WIFI, 0 },
	{ &sxt2d_eth_device, 0, HW_OPT_HAS_PLC },
	{ &plchb_eth_device, HW_OPT_HAS_PLC, 0 },
};

static struct spi_board_info *maphb_spi_info[] = {
	&slow_large_spi_nor,
	&hb_ssr_spi_misc,
	&rb2011_spi_ts,
	NULL
};
static struct platform_device maphb_spi_device = {
	.name	= "rb900-spi",
	.id	= -1,
	.dev	= {
		.platform_data = maphb_spi_info,
	},
};

static struct platform_device maphb_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLED(power_led, 4, GPIO, PLED_CFG_INV | PLED_CFG_ON),
			PLDI(led1, 0, SSR_RB400),
			PLDI(led2, 1, SSR_RB400),
			PLDI(led3, 2, SSR_RB400),
			PLDI(user_led, 3, SSR_RB400),
			PLDI(led4, 4, SSR_RB400),
			PLD(usb_power_off, 5, SSR_RB400),
			PLDI(led5, 6, SSR_RB400),
			0
		},
	},
};

static struct platform_device maphbplc_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLED(power_led, 4, GPIO, PLED_CFG_INV | PLED_CFG_ON),
			PLDI(led1, 0, SSR_RB400),
			PLDI(led2, 1, SSR_RB400),
			PLDI(led3, 2, SSR_RB400),
			PLDI(user_led, 3, SSR_RB400),
			PLDI(led4, 4, SSR_RB400),
			PLDI(usb_power_off, 5, SSR_RB400),
			PLDI(led5, 6, SSR_RB400),
			0
		},
	},
};

static struct poeout_port mAP2_poeout_port = {
        .eth_port = 1,
        .gpo_on = 14,
        .gpi_status = 12,
};
static struct platform_device mAP2_poeout_device = {
        .name   = "poe-out-simple",
        .id     = -1,
        .dev    = {
                .platform_data = &mAP2_poeout_port,
        },
};

static int maphb_eth_port_map[] = { 1, 1, 8227, 0, 1, -1 };
static struct platform_device maphb_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = maphb_eth_port_map,
	},
};

static struct platform_device maphbplc_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			1, 1, 8227, 0, 1, BIT(16 + 3) | 4, -1
		},
	},
};

static struct hwopt_device map2_devices[] = {
	{ &rb700_ehci_device, 0, 0 },
	{ &maphb_led_device, 0, HW_OPT_HAS_PLC },
	{ &maphbplc_led_device, HW_OPT_HAS_PLC, 0 },
	{ &maphb_spi_device, 0, 0 },
	{ &ar9330_wlan_device, 0, 0 },
	{ &ar9340_gpio_device, 0, 0 },
	{ &mAP2_poeout_device, 0, 0 },
	{ &maphb_eth_device, 0, HW_OPT_HAS_PLC },
	{ &maphbplc_eth_device, HW_OPT_HAS_PLC, 0 },
};

static struct platform_device sxtr_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 1, GPIOLIB),
			PLDI(mode_button, 3, GPIOLIB),
			PLED(power_led, 4, GPIOLIB, PLED_CFG_INV | PLED_CFG_ON),
			PLDI(usb_power_off, 13, GPIOLIB),
			PLD(lte_reset, RB400_SSR_GPIO_BASE + 0, GPIOLIB),
			PLD(w_disable, RB400_SSR_GPIO_BASE + 1, GPIOLIB),
			PLDI(led1, RB400_SSR_GPIO_BASE + 2, GPIOLIB),
			PLDI(led2, RB400_SSR_GPIO_BASE + 3, GPIOLIB),
			PLDI(led3, RB400_SSR_GPIO_BASE + 4, GPIOLIB),
			PLDI(led4, RB400_SSR_GPIO_BASE + 5, GPIOLIB),
			PLDI(led5, RB400_SSR_GPIO_BASE + 6, GPIOLIB),
			PLD(sim_select, RB400_SSR_GPIO_BASE + 7, GPIOLIB),
			0
		},
	},
};

static struct platform_device sxtr_hb_poeout_device = {
	.name   = "poe-out-simple",
	.id     = -1,
	.dev    = {
		.platform_data = &(struct poeout_port) {
			.eth_port = 1,
			.gpo_on = 14,
			.gpi_status = 12,
			.has_low_current_sense = 1,
		},
	},
};

static struct platform_device *sxtr_hb_devices[] = {
	&rb700_ehci_device,
	&ar9340_gpio_device,
	&sxtr_led_device,
	&hb_ssr_spi_device,
	&ar9330_wlan_device,
	&sxtr_hb_poeout_device,
	&maphb_eth_device,
};

static struct platform_device crs332_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(led1, 2, GPIOLIB),
			PLDI(led2, 3, GPIOLIB),
			PLDI(fault, 13, GPIOLIB),
			PLDI(user_led, 14, GPIOLIB),
			0
		},
	},
};

static struct platform_device crs332_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			32 << 16 | 1 << 8 | 1, 1, 8227, 0, -1,
		},
	},
};

static struct platform_device *crs332_devices[] = {
	&ar9340_gpio_device,
	&crs332_led_device,
	&slow_waspL_spi_device,
	&crs332_eth_device,
};

static struct platform_device crs312_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(led1, 4, GPIOLIB),
			PLD(led2, 3, GPIOLIB),
			PLDI(fault, 13, GPIOLIB),
			PLDI(user_led, 14, GPIOLIB),
			0
		},
	},
};

static struct platform_device crs312_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			12 << 16 | 1 << 8 | 1, 1, 8227, 0, -1,
		},
	},
};

static struct platform_device *crs312_devices[] = {
	&ar9340_gpio_device,
	&crs312_led_device,
	&slow_waspL_spi_device,
	&crs312_eth_device,
};

static int crs354_eth_port_map[] = { 54 << 16 | 1 << 8 | 1, 1, 8227, 1, -1 };
static struct platform_device crs354_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = crs354_eth_port_map,
	},
};

static struct platform_device *crs354_devices[] = {
	&ar9340_gpio_device,
	&slow_waspL_spi_device,
	&crs354_eth_device,
};


static struct platform_device plc_hb_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 14, GPIOLIB),
			PLDI(wlan_led, 11, GPIOLIB),
//			PLD(ether_led, 4, FUNCTION),
			PLDI(button, 16, GPIOLIB),
			PLDI(mode_button, 0, GPIOLIB),
			0
		},
	},
};

static struct platform_device *plc_hb_devices[] = {
	&ar9340_gpio_device,
	&plc_hb_led_device,
	&hb_ssr_spi_device,
	&ar9330_wlan_device,
	&plchb_eth_device,
};

static struct platform_device lhg60_hb_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(led5, 0, GPIOLIB),
			PLDI(led4, 1, GPIOLIB),
			PLDI(led3, 2, GPIOLIB),
			PLDI(led2, 3, GPIOLIB),
			PLDI(led1, 11, GPIOLIB),
			PLED(power_led, 12, GPIOLIB, PLED_CFG_ON),
			PLD(pcie_power_off, 13, GPIOLIB),
			PLDI(user_led, 14, GPIOLIB),
			PLDI(button, 16, GPIOLIB),
			0
		},
	},
};

static struct platform_device *lhg60_hb_devices[] = {
	&ar9340_gpio_device,
	&lhg60_hb_led_device,
	&slow_waspL_spi_device,
	&sxt2d_eth_device,
};

static struct platform_device sxt_ac_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 14, GPIO),
			PLDI(led1, 1, WIFI),
			PLDI(led2, 8, WIFI),
			PLDI(led3, 9, WIFI),
			PLDI(led4, 10, WIFI),
			PLDI(led5, 11, WIFI),
			PLED(power_led, 7, WIFI, PLED_CFG_ON),
			PLDI(button, 16, GPIO),	// shared with SPI_CS2
			PLDI(sfp_led, 4, GPIO),	// PHY_LED
			0
		},
	},
};
static struct platform_device *sxt_ac_devices[] = {
	&sxt_ac_led_device,
	&hb_ssr_spi_device,
	&ar9330_wlan_device,
	&sxt2d_eth_device
};

static struct hwopt_device sxtg5c_devices[] = {
	{ &rb900_ehci_device, 0, 0 },
	{ &rb2011_spi_device, 0, HW_OPT_NO_NAND },
	{ &ar9344_nand_device, 0, HW_OPT_NO_NAND },
	{ &ar9340L_spi_device, HW_OPT_NO_NAND, HW_OPT_NO_NAND },
	{ &rb711g_eth_device, 0, 0 },
	{ &sxtg5c_led_device, 0, 0 },
	{ &pci_wlan_device, 0, 0},
};

static struct hwopt_device rb922gs_devices[] = {
	{ &rb700_ehci_device, HW_OPT_HAS_USB, 0 },
	{ &rb900_ehci_device, HW_OPT_HAS_USB, 0 },
	{ &rb2011_spi_device, 0, 0 },
	{ &ar9344_nand_device, 0, 0 },
	{ &rb922g_eth_device, HW_OPT_HAS_SFP, 0 },
	{ &rb711_eth_device, 0, HW_OPT_HAS_SFP },
	{ &rb922g_led_device, 0, 0 },
	{ &ar9340_gpio_device, 0, 0 },
	{ &rb953_i2c_gpio_device, 0, 0 },
	{ &rb2011_gpio_i2c_sfp_device, 0, 0 },
};

static struct mdio_gpio_platform_data g52c_mdio_platform_data = {
	.mdc = 12,
	.mdio = 11,
	.phy_mask = 0,
};

struct platform_device g52c_mdio_device = {
	.name   = "mdio-gpio",
	.id     = 1,
	.dev    = {
		.platform_data = &g52c_mdio_platform_data,
	},
};

static struct platform_device g52c_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 16, GPIO),
			PLDI(led1, 0, WIFI),
			PLDI(led2, 1, WIFI),
			PLDI(led3, 2, WIFI),
			PLDI(led4, 3, WIFI),
			PLDI(led5, 4, WIFI),
			PLDI(button, 1, GPIO),
			0
		},
	},
};

static int g52c_eth_port_map[] = { 1, 1, 0 | BIT(30), -1 };
static struct platform_device g52c_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = g52c_eth_port_map,
	},
};
static struct platform_device *g52c_devices[] = {
	&scorpionL_spi_device,
	&ar9340_gpio_device,
	&g52c_mdio_device,
	&g52c_eth_device,
	&g52c_led_device,
};

static struct platform_device wapg_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(led1, 1, WIFI),
			PLDI(led2, 8, WIFI),
			PLDI(led3, 9, WIFI),
			PLED(power_led, 16, GPIO, PLED_CFG_ON | PLED_CFG_INV),
			PLDI(button, 1, GPIO),
			0
		},
	},
};
static struct platform_device *wapg_devices[] = {
	&scorpionL_spi_device,
	&ar9340_gpio_device,
	&g52c_mdio_device,
	&g52c_eth_device,
	&wapg_led_device,
	&ar9330_wlan_device,
};

static struct mdio_gpio_platform_data rb750gv2_mdio_platform_data = {
	.mdc = 17,
	.mdio = 19,
	.phy_mask = 0,
};

struct platform_device rb750gv2_mdio_device = {
	.name   = "mdio-gpio",
	.id     = 1,
	.dev    = {
		.platform_data = &rb750gv2_mdio_platform_data,
	},
};

static struct platform_device rb750gv2_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 14, GPIO),
			PLDI(usb_power_off, 13, GPIO),
			PLED(power_led, 16, GPIO, PLED_CFG_ON | PLED_CFG_INV),
			PLDI(button, 1, GPIO),
			0
		},
	},
};

static int rb750gv2_eth_port_map[] = {
    1, 1, BIT(29) | BIT(30) | 8327, // use bitbanging (bit 30), switch specific config follows (bit 29)
    BIT(8) | BIT(17), // ath8327 specific_config: cpu port is sgmii (bit 8) | sgmii in phy mode (bit 17)
    0, 1, 2, 3, 4, -1 };
static struct platform_device rb750gv2_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = rb750gv2_eth_port_map,
	},
};
static struct platform_device *rb750Gv2_devices[] = {
	&scorpionL_spi_device,
	&ar9340_gpio_device,
	&rb750gv2_eth_device,
	&rb750gv2_mdio_device,
	&rb750gv2_led_device,
	&rb700_ehci_device,
};

static struct platform_device rb962_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 12, GPIO),
			PLDI(usb_power_off, 13, GPIO),
			PLED(power_led, 0, WIFI, PLED_CFG_ON),
			PLDI(led1, 1, WIFI),
			PLDI(led2, 2, WIFI),
			0
		},
	},
};

static struct poeout_port rb962_poeout_port = {
        .eth_port = 4,
        .gpo_on = 3,
        .gpi_status = 2,
	.invert_gpo_on = 1,
};
static struct platform_device rb962_poeout_device = {
        .name   = "poe-out-simple",
        .id     = -1,
        .dev    = {
                .platform_data = &rb962_poeout_port,
        },
};
static int rb962_eth_port_map[] = { 2,
				    0, 8327, 0, 1, 2, 3, 4, -1,
				    1, -1, -1 };
static struct platform_device rb962_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = rb962_eth_port_map,
	},
};

static struct spi_board_info *rb962nand_spi_info[] = {
	&rb900_spi_misc,
	&rb2011_spi_lcd,
	&rb2011_spi_ts,
	NULL
};

static struct platform_device rb962nand_spi_device = {
	.name	= "rb900-spi",
	.id	= -1,
	.dev	= {
		.platform_data = rb962nand_spi_info,
	},
};

static struct spi_board_info *rb962nonand_spi_info[] = {
	&rb900_spi_misc,
	&rb2011_spi_lcd,
	&rb2011_spi_ts,
	&large_spi_nor,
	NULL
};

static struct platform_device rb962nonand_spi_device = {
	.name	= "rb900-spi",
	.id	= -1,
	.dev	= {
		.platform_data = rb962nonand_spi_info,
	},
};

static struct hwopt_device rb962_devices[] = {
	{ &rb700_ehci_device, 0, 0 },
	{ &rb962_led_device, 0, 0 },
	{ &rb962nand_spi_device, 0, HW_OPT_NO_NAND },
	{ &rb962nonand_spi_device, HW_OPT_NO_NAND, HW_OPT_NO_NAND },
	{ &ar9344_nand_device, 0, HW_OPT_NO_NAND },
	{ &ar9330_wlan_device, 0, 0 },
	{ &ar9340_gpio_device, 0, 0 },
	{ &rb953_i2c_gpio_device, 0, 0 },
	{ &rb2011_gpio_i2c_sfp_device, 0, 0 },
	{ &rb962_poeout_device, 0, 0 },
	{ &rb962_eth_device, 0, 0 },
};

static struct platform_device rb960_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(sfp_led, 2,  GPIO),
			PLD(power_led, 3,  GPIO),
			PLDI(user_led, 12, GPIO),
			PLDI(usb_power_off, 13, GPIO),
			PLDI(button, 20, GPIO),
			0
		},
	},
};

static struct poe_platform_data rb960_poe_pd = {
	.reset = 1,
	.swdelay = 10000,
	.swcs = POE_PIN_UNUSED,
	.swclk = 0,
	.swdio = 3,
	.psuCnt = 1,
	.chCnt = 4,
	.chMap = { 4, 3, 2, 1, },
};

static struct spi_board_info rb960_spi_poe = {
	.modalias = "spi-poe-pse",
	.max_speed_hz = 2 * 1000 * 1000,
	.bus_num = 0,
	.chip_select = 6,
	.mode = SPI_MODE_3,
	.platform_data = &rb960_poe_pd,
};

static struct spi_board_info *rb960nand_spi_info[] = {
	&rb960_spi_poe,
	NULL
};

static struct platform_device rb960nand_spi_device = {
	.name	= "rb900-spi",
	.id	= -1,
	.dev	= {
		.platform_data = rb960nand_spi_info,
	},
};

static struct spi_board_info *rb960nonand_spi_info[] = {
	&slow_large_spi_nor,
	&rb960_spi_poe,
	NULL
};

static struct platform_device rb960nonand_spi_device = {
	.name	= "rb900-spi",
	.id	= -1,
	.dev	= {
		.platform_data = rb960nonand_spi_info,
	},
};

static struct platform_device rb960_poeout_device = {
        .name   = "poe-pse-ghost",
        .id     = -1,
};

static struct hwopt_device rb960_devices[] = {
	{ &rb700_ehci_device, 0, 0 },
	{ &rb960_led_device, 0, 0 },
	{ &rb960nand_spi_device, 0, HW_OPT_NO_NAND },
	{ &rb960nonand_spi_device, HW_OPT_NO_NAND, HW_OPT_NO_NAND },
	{ &ar9344_nand_device, 0, HW_OPT_NO_NAND },
	{ &ar9330_wlan_device, 0, 0 },
	{ &ar9340_gpio_device, 0, 0 },
	{ &rb953_i2c_gpio_device, 0, 0 },
	{ &rb2011_gpio_i2c_sfp_device, 0, 0 },
	{ &rb960_poeout_device, 0, 0 },
	{ &rb962_eth_device, 0, 0 },
};

static struct platform_device omni_sc_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLED(user_led, 3, GPIO, PLED_CFG_ON), // also powerled
			PLDI(usb_power_off, 13, GPIO),
			PLDI(button, 20, GPIO),
			0
		},
	},
};

static struct hwopt_device omni_sc_devices[] = {
	{ &rb700_ehci_device, 0, 0 },
	{ &omni_sc_led_device, 0, 0 },
	{ &rb960nonand_spi_device, 0, 0 },
	{ &ar9330_wlan_device, 0, 0 },
	{ &ar9340_gpio_device, 0, 0 },
	{ &rb953_i2c_gpio_device, 0, 0 },
	{ &rb2011_gpio_i2c_sfp_device, 0, 0 },
	{ &rb960_poeout_device, 0, 0 },
	{ &rb951G_eth_device, 0, 0 },
};

static int crs125g_eth_port_map[] = {
    1, 0, 8513,
    1, 2, 3, 4, 5, 6, 7, 8,
    9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24,
    26 | BIT(17),
    -1
};
static struct platform_device crs125g_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = crs125g_eth_port_map,
	},
};

static struct hwopt_device crs125g_devices[] = {
	{ &rb700_ehci_device, HW_OPT_HAS_USB, 0 },
	{ &rb2011_spi_device, HW_OPT_HAS_LCD, 0 },
	{ &rb2011L_spi_device, 0, HW_OPT_HAS_LCD },
	{ &ar9344_nand_device, 0, 0 },
	{ &ar9330_wlan_device, HW_OPT_HAS_WIFI, 0 },
	{ &crs125g_eth_device, 0, 0 },
	{ &crs125_wifi_led_device, HW_OPT_HAS_WIFI, 0 },
	{ &crs125_led_device, 0, HW_OPT_HAS_WIFI },
};

static int crs109_eth_port_map[] = {
    1, 0, 8513,
    1, 2, 3, 4, 5, 6, 7, 8,
    26 | BIT(17),
    -1
};
static struct platform_device crs109_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = crs109_eth_port_map,
	},
};

static int crs110_eth_port_map[] = {
    1, 0, 8513,
    1, 2, 3, 4, 5, 6, 7, 8,
    26 | BIT(17),
    27 | BIT(17),
    -1
};
static struct platform_device crs110_eth_device = {
	.name	= "ag7240",
	.id	= -1,
	.dev	= {
		.platform_data = crs110_eth_port_map,
	},
};

static struct hwopt_device crs109_devices[] = {
	{ &rb700_ehci_device, HW_OPT_HAS_USB, 0 },
	{ &rb2011_spi_device, HW_OPT_HAS_LCD, 0 },
	{ &rb2011L_spi_device, 0, HW_OPT_HAS_LCD },
	{ &ar9344_nand_device, 0, 0 },
	{ &ar9330_wlan_device, HW_OPT_HAS_WIFI, 0 },
	{ &crs109_eth_device, HW_OPT_HAS_USB, 0 },
	{ &crs110_eth_device, 0, HW_OPT_HAS_USB },
	{ &crs125_wifi_led_device, HW_OPT_HAS_WIFI, 0 },
	{ &crs125_led_device, 0, HW_OPT_HAS_WIFI },
};

static struct platform_device groove52_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(led1, 0, WIFI),
			PLD(led2, 0, SHARED_RB900),
			PLD(led3, 1, SHARED_RB900),
			PLD(led4, 2, SHARED_RB900),
			PLDI(button, 15, SHARED_RB900),
			PLDI(pin_hole, 14, SHARED_RB900),
			0
		},
	},
};

static struct platform_device groove52L_led_device = {
        .name   = "leds-rb",
        .id     = -1,
        .dev    = {
                .platform_data = &(unsigned[]) {
                        PLD(led1, 0, WIFI),
                        PLD(led2, 0, GPIO),
                        PLD(led3, 1, GPIO),
                        PLD(led4, 2, GPIO),
                        PLDI(button, 15, GPIO),
                        PLDI(pin_hole, 14, GPIO),
                        0
                },
        },
};

static struct hwopt_device groove52_devices[] = {
	{ &rb750_spi_device, 0, HW_OPT_NO_NAND },
	{ &ar9342_nand_device, 0, HW_OPT_NO_NAND },
	{ &ar9340L_spi_device, HW_OPT_NO_NAND, HW_OPT_NO_NAND },
	{ &groove52_eth_device, 0, 0 },
	{ &ar9330_wlan_device, 0, 0 },
	{ &groove52_led_device, 0, HW_OPT_NO_NAND },
	{ &groove52L_led_device, HW_OPT_NO_NAND, HW_OPT_NO_NAND },
};

static struct platform_device ar9330_uart_device = {
	.name	= "ar9330-uart",
	.id	= -1,
};
static struct platform_device ar9330_nand_device = {
	.name	= "ar9330-nand",
	.id	= -1,
};
static struct platform_device rb951_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 0, GPIO),
			PLD(usb_power_off, 9, GPIO),
			PLDI(button, 6, GPIO),
			PLDI(pin_hole, 7, GPIO),
			0
		},
	},
};

static struct platform_device mAP_led_device = {
        .name   = "leds-rb",
        .id     = -1,
        .dev    = {
                .platform_data = &(unsigned[]) {
			PLDI(user_led, 21, GPIO),
			PLDI(led1, 18, GPIO),
			PLDI(led2, 20, GPIO),
			PLDI(led3, 19, GPIO),
			PLDI(led4, 22, GPIO),
			PLDI(led5, 23, GPIO),
			PLD(power_led, 28, GPIO),
			PLDI(button, 6, GPIO),
			PLDI(pin_hole, 7, GPIO),
			0
		},
        },
};

static struct platform_device *rb951_devices[] = {
	&rb700_ehci_device,
	&rb750_spi_device,
	&ar9330_nand_device,
	&ar9330_uart_device,
	&rb750_eth_device,
	&rb951_led_device,
	&ar9330_wlan_device,
};

static struct platform_device cm2n_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(led1, 14, GPIO),
			PLDI(led2, 15, GPIO),
			PLDI(led3, 16, GPIO),
			PLDI(led4, 17, GPIO),
			PLDI(button, 6, GPIO),
			PLDI(pin_hole, 7, GPIO),
			PLED(all_leds, 28, GPIO, PLED_CFG_ON),
			0
		},
	},
};

static struct platform_device *cm2n_devices[] = {
	&rb900l_spi_device,
	&ar9330_uart_device,
	&cm2n_led_device,
	&rb711r3_eth_device,
	&ar9330_wlan_device,
};

static struct platform_device *mAP_devices[] = {
	&rb400_ehci_device,
	&rb400_ohci_device,
	&rb900l_spi_device,
	&ar9330_uart_device,
	&rb400_gpio_device,
	&mAP_poeout_device,
	&mAP_eth_device,
	&ar9330_wlan_device,
	&mAP_led_device,
};

static struct platform_device *rb_sxt5d_devices[] = {
	&rb700_ehci_device,
	&rb700_ohci_device,
	&rb711_spi_device,
	&rb700_nolatch_nand_device,
	&sxt5d_led_device,
	&rb711r3_eth_device
};

static struct platform_device *rb_groove_devices[] = {
	&rb700_ehci_device,
	&rb700_ohci_device,
	&rb711_spi_device,
	&rb700_nolatch_nand_device,
	&groove_led_device,
	&rb711r3_eth_device
};

static struct platform_device *rb750_devices[] = {
	&rb700_ehci_device,
	&rb700_ohci_device,
	&omni_spi_device,
	&rb700_tiny_nand_device,
	&rb700_led_device,
	&rb750_eth_device
};

static struct platform_device *rb751_devices[] = {
	&rb700_ehci_device,
	&rb700_ohci_device,
	&rb711_spi_device,
	&rb700_tiny_nand_device,
	&rb751_led_device,
	&rb750_eth_device
};

static int ccs226_eth_port_map[] = {
    8519,
    1, 2, 3, 4, 5, 6, 7, 8,
    9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24,
    26 | BIT(16) | BIT(17),
    25 | BIT(16),
    -1
};
static struct platform_device ccs226_eth_device = {
	.name	= "music",
	.id	= -1,
	.dev	= {
		.platform_data = ccs226_eth_port_map,
	},
};

static int crs210_eth_port_map[] = {
    8513,
    1, 2, 3, 4, 5, 6, 7, 8,
    26 | BIT(16) | BIT(17),
    25 | BIT(16),
    -1
};
static struct platform_device crs210_eth_device = {
	.name	= "music",
	.id	= -1,
	.dev	= {
		.platform_data = crs210_eth_port_map,
	},
};
static struct spi_board_info *music_large_spi_info[] = {
	&rb400_spi_misc,
	&large_spi_nor,
	&music_spi_nand,
	&music_spi_lcd,
	&music_spi_ts,
	NULL
};
static struct platform_device music_large_spi_device = {
	.name	= "rb400-spi",
	.id	= -1,
	.dev	= {
		.platform_data = music_large_spi_info,
	},
};
static struct spi_board_info *crs212_large_spi_info[] = {
	&rb400_spi_misc,
	&large_spi_nor,
	&music_spi_nand,
	&music_spi_lcd,
	&music_spi_ts,
	&crs212_spi_ssr,
	NULL
};
static struct platform_device crs212_large_spi_device = {
	.name	= "rb400-spi",
	.id	= -1,
	.dev	= {
		.platform_data = crs212_large_spi_info,
	},
};
static struct hwopt_device ccs226_devices[] = {
	{ &music_large_spi_device, HW_OPT_NO_NAND, 0 },
	{ &music_spi_device, 0, HW_OPT_NO_NAND },
	{ &ccs226_eth_device, 0, 0 },
	{ &crs226_led_device, 0, 0 },
};

static struct hwopt_device crs210_devices[] = {
	{ &music_large_spi_device, HW_OPT_NO_NAND, 0 },
	{ &music_spi_device, 0, HW_OPT_NO_NAND },
	{ &crs210_eth_device, 0, 0 },
	{ &crs226_led_device, 0, 0 },
};

static int crs212_eth_port_map[] = {
    8719,
    5 | BIT(17),
    1 | BIT(17),
    13 | BIT(17),
    9 | BIT(17),
    21 | BIT(17),
    17 | BIT(17),
    27 | BIT(17),
    26 | BIT(17),
    29 | BIT(17),
    28 | BIT(17),
    25 | BIT(16),
    24,
    -1
};
static struct platform_device crs212_eth_device = {
	.name	= "music",
	.id	= -1,
	.dev	= {
		.platform_data = crs212_eth_port_map,
	},
};
static struct hwopt_device crs212_devices[] = {
	{ &crs212_large_spi_device, HW_OPT_NO_NAND, 0 },
	{ &crs212_spi_device, 0, HW_OPT_NO_NAND },
	{ &crs212_eth_device, 0, 0 },
	{ &crs226_led_device, 0, 0 },
};

static int ccs112_eth_port_map[] = {
    8511,
    1, 2, 3, 4, 5, 6, 7, 8,
    27 | BIT(17),
    26 | BIT(17),
    29 | BIT(17),
    28 | BIT(17),
    -1
};
static struct platform_device ccs112_eth_device = {
	.name	= "music",
	.id	= -1,
	.dev	= {
		.platform_data = ccs112_eth_port_map,
	},
};

static struct rb400_ssr_info ccs112_ssr_data = {
	.bytes = 1,
	.val = 0xff
};
static struct rb400_spi_ctrl ccs112_ssr_ctrl = {
	.gpio_as_cs = 1,
	.gpio_as_strobe = 1,
};
static struct spi_board_info ccs_spi_ssr = {
	.modalias = "ccs-spi-ssr",
	.max_speed_hz = 20 * 1000 * 1000,
	.bus_num = 0,
	.chip_select = 2,
	.mode = SPI_MODE_3,
	.platform_data = &ccs112_ssr_data,
	.controller_data = &ccs112_ssr_ctrl,
};
static struct spi_board_info ccs112_spi_lcd = {
	.modalias = "rb2011-spi-lcd",
	.max_speed_hz = 15 * 1000 * 1000,
	.bus_num = 0,
	.chip_select = 8,
	.mode = SPI_MODE_0,
};
static struct spi_board_info ccs112_spi_ts = {
	.modalias = "rb2011-spi-ts",
	.max_speed_hz = 2000 * 1000,
	.bus_num = 0,
	.chip_select = 9,
	.mode = SPI_MODE_0,
	.platform_data = (void *) 0,
};

static struct spi_board_info *ccs112_spi_info[] = {
	&rb400_spi_misc,
	&ccs_spi_ssr,
	&large_spi_nor,
	&ccs112_spi_lcd,
	&ccs112_spi_ts,
	NULL
};
static struct platform_device ccs112_spi_device = {
	.name	= "rb400-spi",
	.id	= -1,
	.dev	= {
		.platform_data = ccs112_spi_info,
	},
};

static struct platform_device *ccs112_devices[] = {
	&ccs_gpio_device,
	&ccs112_spi_device,
	&ccs112_eth_device,
	&ccs112_led_device,
};

static struct spi_board_info ccs112r4_spi_ssr = {
	.modalias = "ccs-spi-ssr",
	.max_speed_hz = 2 * 1000 * 1000,
	.bus_num = 0,
	.chip_select = 10,
	.mode = SPI_MODE_3,
	.platform_data = &ccs112_ssr_data,
};
static struct rb400_spi_ctrl ccs112r4_ts_ctrl = {
	.gpio_as_cs = 2,
};
static struct spi_board_info ccs112r4_spi_ts = {
	.modalias = "rb2011-spi-ts",
	.max_speed_hz = 2000 * 1000,
	.bus_num = 0,
	.chip_select = 6,
	.mode = SPI_MODE_0,
	.platform_data = (void *) 0,
	.controller_data = &ccs112r4_ts_ctrl,
};

static struct rb400_spi_ctrl ccs112r4_poe_ctrl = {
	.gpio_as_cs = 1,
};

static struct poe_platform_data ccs112r4_poe_pd = {
	.reset = (2 | POE_PIN_GPIO_INVERT),
	.swdelay = 10000,
	.swcs = POE_PIN_SPI_CS,
	.swclk = POE_PIN_SPI_CLK,
	.swdio = 1,
	.psuCnt = 2,
	.chCnt = 8,
	.chMap = { 7, 6, 5, 4, 3, 2, 1, 0, },
};

static struct spi_board_info ccs112r4_spi_poe = {
	.modalias = "spi-poe-pse",
	.max_speed_hz = 2 * 1000 * 1000,
	.bus_num = 0,
	.chip_select = 11,
	.mode = SPI_MODE_0,
	.controller_data = &ccs112r4_poe_ctrl,
	.platform_data = &ccs112r4_poe_pd,
};

static struct spi_board_info *ccs112r4_poe_spi_info[] = {
	&rb400_spi_misc,
	&ccs112r4_spi_ssr,
	&large_spi_nor,
	&ccs112r4_spi_poe,
	NULL
};
static struct platform_device ccs112r4_poe_spi_device = {
	.name	= "rb400-spi",
	.id	= -1,
	.dev	= {
		.platform_data = ccs112r4_poe_spi_info,
	},
};

static struct spi_board_info *ccs112r4_ts_spi_info[] = {
	&rb400_spi_misc,
	&ccs112r4_spi_ssr,
	&large_spi_nor,
	&ccs112r4_spi_ts,
	NULL
};
static struct platform_device ccs112r4_ts_spi_device = {
	.name	= "rb400-spi",
	.id	= -1,
	.dev	= {
		.platform_data = ccs112r4_ts_spi_info,
	},
};

static struct hwopt_device ccs112r4_devices[] = {
	{ &ccs_gpio_device, 0, 0 },
	{ &ccs112r4_poe_spi_device, HW_OPT_HAS_POE_OUT, 0 },
	{ &ccs112r4_ts_spi_device, 0, HW_OPT_HAS_POE_OUT },
	{ &ccs112_eth_device, 0, 0 },
	{ &ccs112_led_device, 0, 0 },
};

static int crs106_eth_port_map[] = {
    8511,
    5 | BIT(17) | BIT(18),
    1 | BIT(17),
    26 | BIT(17),
    27 | BIT(17),
    28 | BIT(17),
    29 | BIT(17),
    -1
};
static struct platform_device crs106_eth_device = {
	.name	= "music",
	.id	= -1,
	.dev	= {
		.platform_data = crs106_eth_port_map,
	},
};
static struct rb400_ssr_info crs106_ssr_data = {
	.bytes = 2,
	.val = 0xffff
};

static struct spi_board_info crs106_spi_ssr = {
	.modalias = "ccs-spi-ssr",
	.max_speed_hz = 1 * 1000 * 1000,
	.bus_num = 0,
	.chip_select = 2,
	.mode = SPI_MODE_3,
	.platform_data = &crs106_ssr_data,
};
static struct spi_board_info *crs106_spi_info[] = {
	&rb400_spi_misc,
	&crs106_spi_ssr,
	&large_spi_nor,
	NULL
};
static struct platform_device crs106_spi_device = {
	.name	= "rb400-spi",
	.id	= -1,
	.dev	= {
		.platform_data = crs106_spi_info,
	},
};
static struct platform_device crs106_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(button, 0, GPIO),
			0
		},
	},
};
static struct platform_device *crs106_devices[] = {
	&ccs_gpio_device,
	&crs106_spi_device,
	&crs106_eth_device,
	&crs106_led_device,
};

static int crs105_eth_port_map[] = {
    8511,
    1 | BIT(17),
    26 | BIT(17),
    27 | BIT(17),
    28 | BIT(17),
    29 | BIT(17),
    -1
};
static struct platform_device crs105_eth_device = {
	.name	= "music",
	.id	= -1,
	.dev	= {
		.platform_data = crs105_eth_port_map,
	},
};
static struct platform_device *crs105_devices[] = {
	&ccs_gpio_device,
	&crs106_spi_device,
	&crs105_eth_device,
	&crs106_led_device,
};

static struct platform_driver fake_serial8250_driver = {
	.driver = {
		.name = "serial8250",
	},
};

static struct rb400_ssr_info sxt_lte_ssr_data = {
	.strobe = 15,
	.bytes = 1,
	.val = 0xff,
};
static struct spi_board_info sxt_lte_spi_misc = {
	.modalias = "rb400-spi-misc",
	.max_speed_hz = 10 * 1000 * 1000,
	.bus_num = 0,
	.chip_select = 0,
	.mode = SPI_MODE_0,
	.platform_data = &sxt_lte_ssr_data,
};

static struct spi_board_info sxt_lte_spi_ts = {
	.modalias = "rb2011-spi-ts",
	.max_speed_hz = 2000 * 1000,
	.bus_num = 0,
	.chip_select = 7,
	.mode = SPI_MODE_0,
	.platform_data = (void *) 16,
};

static struct spi_board_info *sxt_lte_spi_info[] = {
	&sxt_lte_spi_misc,
	&rb700_spi_nand,
	&sxt_lte_spi_ts,
	NULL
};

static struct platform_device sxt_lte_spi_device = {
	.name	= "rb400-spi",
	.id	= -1,
	.dev	= {
		.platform_data = sxt_lte_spi_info,
	},
};

static struct platform_device sxt_lte_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLDI(user_led, 12, SHARED_RB700),
			PLDI(led1, 2, SSR_RB400),
			PLDI(led2, 3, SSR_RB400),
			PLDI(led3, 4, SSR_RB400),
			PLDI(led4, 5, SSR_RB400),
			PLDI(led5, 6, SSR_RB400),
			PLED(power_led, 14, SHARED_RB700,
			       PLED_CFG_ON | PLED_CFG_INV),
			PLDI(usb_power_off, 16, SHARED_RB700),	// USB sel LTE
			PLDI(pcie_power_off, 1, SSR_RB400),	// mPCIe pwr off
			PLDI(pcie2_power_off, 0, SSR_RB400),	// LTE power off
			PLDI(link_act_led, 13, SHARED_RB700),
			0
		},
	},
};

static struct platform_device *sxt_lte_devices[] = {
	&rb700_ehci_device,
	&rb700_ohci_device,
	&rb400_gpio_device,
	&sxt_lte_spi_device,
	&rb700_tiny_nand_device,	// CS2 is used by ADC chip
	&sxt_lte_led_device,
	&rb711r3_eth_device,
};

static struct platform_device ralink_gpio_device = {
	.name	= "ralink-gpio",
	.id	= -1,
};
static struct platform_device ralink_crypto_device = {
	.name	= "mt7621-crypto",
	.id	= -1,
};
static struct platform_device ralink_ts_device = {
	.name	= "ts-ralink",
	.id	= -1,
};

static struct resource ralink_xhci_resources[] = {
	[0] = {
		.start  = 0x1e1c0000,
		.end    = 0x1e1cffff,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = 22,
		.end    = 22,
		.flags  = IORESOURCE_IRQ,
	},
};
static struct platform_device ralink_xhci_device = {
	.name		= "ralink-xhci",
	.id		= -1,
	.dev		= {
		.dma_mask		= &dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(ralink_xhci_resources),
	.resource	= ralink_xhci_resources,
};

static struct resource ralink_sd_resources[] = {
	[0] = {
		.start  = 0xBE130000,
		.end    = 0xBE133fff,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = 20,
		.end    = 20,
		.flags  = IORESOURCE_IRQ,
	},
};
static struct platform_device ralink_sd_device = {
	.name           = "ralink-sd",
	.id             = 0,
	.dev		= {
		.platform_data = (void *)1,	/* disable write protect */
	},
	.num_resources  = ARRAY_SIZE(ralink_sd_resources),
	.resource       = ralink_sd_resources,
};

static struct platform_device gpio_15_beep_device = {
	.name	= "gpio-beep",
	.id	= -1,
	.dev	= {
		.platform_data = (void*)15,
	},
};

static struct platform_device rb750gr3_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 0, GPIOLIB),
			PLDI(usb_power_off, 12, GPIOLIB),
			PLED(power_led, 16, GPIOLIB, PLED_CFG_ON),
			PLDI(button, 18, GPIOLIB),
			PLDI(mode_button, 13, GPIOLIB),
			0
		},
	},
};
static struct platform_device ralink_poeout_device = {
        .name   = "poe-out-simple",
        .id     = -1,
        .dev    = {
                .platform_data = &(struct poeout_port){
			.eth_port = 4,
			.gpo_on = 17,
			.gpi_status = 7,
		},
        },
};
static struct platform_device rb750gr4_poeout_device = {
        .name   = "poe-out-simple",
        .id     = -1,
        .dev    = {
                .platform_data = &(struct poeout_port){
			.eth_port = 4,
			.gpo_on = 17,
			.gpi_status = 7,
			.has_low_current_sense = 1,
		},
        },
};
static struct platform_device rb750gr3_eth_device = {
	.name = "mt7621-eth",
	.id = -1,
	.dev = {
		.platform_data = (int[]){0, 1, 2, 3, 4, -1},
	},
};
static struct platform_device rb760_sfp_eth_device = {
	.name = "mt7621-eth",
	.id = -1,
	.dev = {
		.platform_data = (int[]){0, 1, 2, 3, 4, (1 << 17) | (1 << 20) | 5, -1},
	},
};
static struct platform_device rb760_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(sfp_led, 9, GPIOLIB),
			PLDI(usb_power_off, 12, GPIOLIB),
			PLED(power_led, 16, GPIOLIB, PLED_CFG_ON),
			PLDI(button, 18, GPIOLIB),
			PLDI(mode_button, 13, GPIOLIB),
			0
		},
	},
};
static struct hwopt_device rb750gr3_devices[] = {
	{ &ralink_gpio_device, 0, 0 },
	{ &ralink_poeout_device, HW_OPT_HAS_POE_OUT, 0 },
	{ &rb750gr3_eth_device, 0, HW_OPT_HAS_SFP },
	{ &rb760_sfp_eth_device, HW_OPT_HAS_SFP, 0 },
	{ &ralink_crypto_device, 0, 0 },
	{ &ralink_xhci_device, HW_OPT_HAS_USB, 0 },
	{ &ralink_sd_device, HW_OPT_HAS_uSD, 0 },
	{ &rb750gr3_led_device, 0, HW_OPT_HAS_SFP },
	{ &rb760_led_device, HW_OPT_HAS_SFP, 0 },
	{ &gpio_15_beep_device, 0, 0 },
	{ &ralink_ts_device, 0, 0 },
};
static struct hwopt_device rb750gr4_devices[] = {
	{ &ralink_gpio_device, 0, 0 },
	{ &rb750gr4_poeout_device, HW_OPT_HAS_POE_OUT, 0 },
	{ &rb750gr3_eth_device, 0, HW_OPT_HAS_SFP },
	{ &rb760_sfp_eth_device, HW_OPT_HAS_SFP, 0 },
	{ &ralink_crypto_device, 0, 0 },
	{ &ralink_xhci_device, HW_OPT_HAS_USB, 0 },
	{ &ralink_sd_device, HW_OPT_HAS_uSD, 0 },
	{ &rb750gr3_led_device, 0, HW_OPT_HAS_SFP },
	{ &rb760_led_device, HW_OPT_HAS_SFP, 0 },
	{ &gpio_15_beep_device, 0, 0 },
	{ &ralink_ts_device, 0, 0 },
};

static struct platform_device m11_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 0, GPIOLIB),
			PLDI(button, 18, GPIOLIB),
			PLDI(led1, 26, GPIOLIB),
			PLDI(led2, 25, GPIOLIB),
			PLDI(led3, 24, GPIOLIB),
			PLDI(led4, 23, GPIOLIB),
			PLDI(led5, 22, GPIOLIB),
			PLDI(pcie_power_off, 9, GPIOLIB),
			0
		},
	},
};
static struct platform_device *m11_devices[] = {
	&(struct platform_device){
	    .name = "mt7621-eth",
	    .id = -1,
	    .dev = {
		.platform_data = (int[]){0, -1},
	    },
	},
	&ralink_gpio_device,
	&ralink_crypto_device,
	&ralink_xhci_device,
	&m11_led_device,
};

static struct platform_device m33_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, 0, GPIOLIB),
			PLDI(pcie_power_off, 9, GPIOLIB),
			PLDI(pcie2_power_off, 10, GPIOLIB),
			PLDI(usb_power_off, 12, GPIOLIB),
			PLDI(button, 18, GPIOLIB),
			0
		},
	},
};
static struct platform_device *m33_devices[] = {
	&(struct platform_device){
	    .name = "mt7621-eth",
	    .id = -1,
	    .dev = {
		.platform_data = (int[]){0, 1, 2, -1},
	    },
	},
	&ralink_gpio_device,
	&ralink_crypto_device,
	&ralink_xhci_device,
	&ralink_sd_device,
	&m33_led_device,
	&ralink_ts_device,
};

int rb_platform_init(void)
{
	switch (mips_machgroup) {
	case MACH_GROUP_MT_RB100:
		platform_driver_register(&fake_serial8250_driver);

		switch (mips_machtype) {
		case MACH_MT_RB100:
			return platform_add_devices(
			    rb112_devices, ARRAY_SIZE(rb112_devices));
		case MACH_MT_RB150:
			return platform_add_devices(
			    rb153_devices, ARRAY_SIZE(rb153_devices));
		case MACH_MT_RB133:
			return platform_add_devices(
			    rb133_devices, ARRAY_SIZE(rb133_devices));
		case MACH_MT_RB133C:
			return platform_add_devices(
			    rb133c_devices, ARRAY_SIZE(rb133c_devices));
		case MACH_MT_RB192:
			return platform_add_devices(
			    rb192_devices, ARRAY_SIZE(rb192_devices));
		case MACH_MT_MR:
			return platform_add_devices(
			    mr_devices, ARRAY_SIZE(mr_devices));
		}
		break;
	case MACH_GROUP_MT_RB500:
		switch (mips_machtype) {
		case MACH_MT_RB500:
			return platform_add_devices(
			    rb500_devices, ARRAY_SIZE(rb500_devices));
		case MACH_MT_RB500R5:
			return platform_add_devices(
			    rb500r5_devices, ARRAY_SIZE(rb500r5_devices));
		}
		break;
	case MACH_GROUP_MT_RB400:
		switch (mips_machtype) {
		case MACH_MT_RB411:
			return platform_add_devices(
				rb411_devices,  ARRAY_SIZE(rb411_devices));
		case MACH_MT_RB411L:
			return platform_add_devices(
				rb411l_devices,  ARRAY_SIZE(rb411l_devices));
		case MACH_MT_RB411UL:
			return platform_add_devices(
				rb411ul_devices,  ARRAY_SIZE(rb411ul_devices));
		case MACH_MT_RB411G:
			return platform_add_devices(
				rb411g_devices,  ARRAY_SIZE(rb411g_devices));
		case MACH_MT_RB411U:
			return platform_add_devices(
				rb411u_devices,  ARRAY_SIZE(rb411u_devices));
		case MACH_MT_RB433:
			return platform_add_devices(
				rb433_devices,  ARRAY_SIZE(rb433_devices));
		case MACH_MT_RB433U:
			return platform_add_devices(
				rb433u_devices,  ARRAY_SIZE(rb433u_devices));
		case MACH_MT_RB433GL:
			return platform_add_devices(
				rb433gl_devices,  ARRAY_SIZE(rb433gl_devices));
		case MACH_MT_RB433UL:
			return platform_add_devices(
				rb433ul_devices,  ARRAY_SIZE(rb433ul_devices));
		case MACH_MT_RB433L:
			return platform_add_devices(
				rb433l_devices,  ARRAY_SIZE(rb433l_devices));
		case MACH_MT_RB435G:
			return platform_add_devices(
				rb435g_devices,  ARRAY_SIZE(rb435g_devices));
		case MACH_MT_RB450:
			return platform_add_devices(
				rb450_devices,  ARRAY_SIZE(rb450_devices));
		case MACH_MT_RB450G:
			return platform_add_devices(
				rb450g_devices,  ARRAY_SIZE(rb450g_devices));
		case MACH_MT_RB493:
			return platform_add_devices(
				rb493_devices,  ARRAY_SIZE(rb493_devices));
		case MACH_MT_RB493G:
			return platform_add_devices(
				rb493g_devices,  ARRAY_SIZE(rb493g_devices));
		case MACH_MT_RB750G:
			return platform_add_devices(
				rb750g_devices,  ARRAY_SIZE(rb750g_devices));
		}
		break;
	case MACH_GROUP_MT_RB700:
		switch (mips_machtype) {
		case MACH_MT_RB_OMNI:
		case MACH_MT_RB711:
			return platform_add_devices(
				rb711_devices,  ARRAY_SIZE(rb711_devices));
		case MACH_MT_RB711R3:
			return platform_add_devices(
				rb711r3_devices,  ARRAY_SIZE(rb711r3_devices));
		case MACH_MT_RB_GROOVE_5S:
		case MACH_MT_RB_SXT5D:
			return platform_add_devices(
			    rb_sxt5d_devices,  ARRAY_SIZE(rb_sxt5d_devices));
		case MACH_MT_RB751G:
			return platform_add_devices(
			    rb751g_devices,  ARRAY_SIZE(rb751g_devices));
		case MACH_MT_RB711G:
			return platform_add_devices(
			    rb711g_devices,  ARRAY_SIZE(rb711g_devices));
		case MACH_MT_RB711GT:
			return platform_add_devices(
			    rb711gt_devices,  ARRAY_SIZE(rb711gt_devices));
		case MACH_MT_RB_SXTG:
			return platform_add_devices(
			    rb_sxtg_devices,  ARRAY_SIZE(rb_sxtg_devices));
		case MACH_MT_RB912G:
			return platform_add_hwopt_devices(
			    rb912g_devices, ARRAY_SIZE(rb912g_devices));
		case MACH_MT_RB750GL:
			return platform_add_hwopt_devices(
			    rb750gl_devices,  ARRAY_SIZE(rb750gl_devices));
		case MACH_MT_OMNITIK_WASP:
			return platform_add_devices(
			    omnitik_wasp_devices,  ARRAY_SIZE(omnitik_wasp_devices));
		case MACH_MT_RB911L:
			return platform_add_devices(
			    rb911L_devices,  ARRAY_SIZE(rb911L_devices));
		case MACH_MT_LHG:
			return platform_add_devices(
			    lhg_devices,  ARRAY_SIZE(lhg_devices));
		case MACH_MT_SXT2D:
			return platform_add_devices(
			    sxt2d_devices,  ARRAY_SIZE(sxt2d_devices));
		case MACH_MT_SXT5N:
			return platform_add_hwopt_devices(
			    sxt5n_devices,  ARRAY_SIZE(sxt5n_devices));
		case MACH_MT_SXTG5P:
			return platform_add_hwopt_devices(
			    sxtg5p_devices,  ARRAY_SIZE(sxtg5p_devices));
		case MACH_MT_SXTG5C:
			return platform_add_hwopt_devices(
			    sxtg5c_devices,  ARRAY_SIZE(sxtg5c_devices));
		case MACH_MT_SXT_LTE:
			return platform_add_devices(
			    sxt_lte_devices,  ARRAY_SIZE(sxt_lte_devices));
		case MACH_MT_RB922GS:
			return platform_add_hwopt_devices(
			    rb922gs_devices,  ARRAY_SIZE(rb922gs_devices));
		case MACH_MT_GROOVE_52AC:
			return platform_add_devices(
			    g52c_devices,  ARRAY_SIZE(g52c_devices));
		case MACH_MT_RB750Gv2:
			return platform_add_devices(
			    rb750Gv2_devices,  ARRAY_SIZE(rb750Gv2_devices));
		case MACH_MT_wAPG:
			return platform_add_devices(
			    wapg_devices,  ARRAY_SIZE(wapg_devices));
		case MACH_MT_RB962:
			return platform_add_hwopt_devices(
			    rb962_devices,  ARRAY_SIZE(rb962_devices));
		case MACH_MT_RB960:
			return platform_add_hwopt_devices(
			    rb960_devices,  ARRAY_SIZE(rb960_devices));
		case MACH_MT_OMNI_SC:
			return platform_add_hwopt_devices(
			    omni_sc_devices,  ARRAY_SIZE(omni_sc_devices));
		case MACH_MT_951HND:
			return platform_add_hwopt_devices(
			    rb951hnd_devices,  ARRAY_SIZE(rb951hnd_devices));
		case MACH_MT_RB951G:
			return platform_add_devices(
			    rb951G_devices,  ARRAY_SIZE(rb951G_devices));
		case MACH_MT_CM2N:
			return platform_add_devices(
			    cm2n_devices,  ARRAY_SIZE(cm2n_devices));
		case MACH_MT_mAP:
			return platform_add_devices(
			    mAP_devices,  ARRAY_SIZE(mAP_devices));
		case MACH_MT_RB2011G:
			return platform_add_devices(
			    rb2011G_devices,  ARRAY_SIZE(rb2011G_devices));
		case MACH_MT_RB2011US:
			return platform_add_devices(
			    rb2011US_devices,  ARRAY_SIZE(rb2011US_devices));
		case MACH_MT_RB2011L:
			return platform_add_devices(
			    rb2011L_devices,  ARRAY_SIZE(rb2011L_devices));
		case MACH_MT_RB2011LS:
			return platform_add_devices(
			    rb2011LS_devices,  ARRAY_SIZE(rb2011LS_devices));
		case MACH_MT_RB2011R5:
			return platform_add_hwopt_devices(
			    rb2011r5_devices,  ARRAY_SIZE(rb2011r5_devices));
		case MACH_MT_RB953GS:
			return platform_add_devices(
			    rb953gs_devices,  ARRAY_SIZE(rb953gs_devices));
		case MACH_MT_SXTR_hb:
			return platform_add_devices(
			    sxtr_hb_devices,  ARRAY_SIZE(sxtr_hb_devices));
		case MACH_MT_CRS332:
			nor_part_table[0].offset = (192 + 256) * 1024;
			nor_part_table[1].size = 192 * 1024;
			nor_part_table[2].offset = 192 * 1024;
			nor_part_table[2].size = 256 * 1024;
			nor_data.nr_parts = 3;
			return platform_add_devices(
			    crs332_devices,  ARRAY_SIZE(crs332_devices));
		case MACH_MT_CRS312:
			nor_part_table[0].offset = (320 + 256) * 1024;
			nor_part_table[1].size = 320 * 1024;
			nor_part_table[2].offset = 320 * 1024;
			nor_part_table[2].size = 256 * 1024;
			nor_data.nr_parts = 3;
			return platform_add_devices(
			    crs312_devices,  ARRAY_SIZE(crs312_devices));
		case MACH_MT_CRS354:
			nor_part_table[0].offset = (192 + 256) * 1024;
			nor_part_table[1].size = 192 * 1024;
			nor_part_table[2].offset = 192 * 1024;
			nor_part_table[2].size = 256 * 1024;
			nor_data.nr_parts = 3;
			return platform_add_devices(
			    crs354_devices,  ARRAY_SIZE(crs354_devices));
		case MACH_MT_SXT_AC:
			return platform_add_devices(
			    sxt_ac_devices,  ARRAY_SIZE(sxt_ac_devices));
		case MACH_MT_RB941HL:
			return platform_add_hwopt_devices(
			    rb941HL_devices,  ARRAY_SIZE(rb941HL_devices));
		case MACH_MT_RB931:
			return platform_add_hwopt_devices(
			    rb931_devices,  ARRAY_SIZE(rb931_devices));
		case MACH_MT_wAP_LTE:
			return platform_add_devices(
			    wap_lte_devices,  ARRAY_SIZE(wap_lte_devices));
		case MACH_MT_wAP:
			return platform_add_devices(
			    wap_devices,  ARRAY_SIZE(wap_devices));
		case MACH_MT_cAP:
			return platform_add_devices(
			    cap_devices,  ARRAY_SIZE(cap_devices));
		case MACH_MT_LHG_hb:
			return platform_add_devices(
			    lhg_hb_devices,  ARRAY_SIZE(lhg_hb_devices));
		case MACH_MT_LTAP_hb:
			return platform_add_hwopt_devices(
			    ltap_hb_devices,  ARRAY_SIZE(ltap_hb_devices));
		case MACH_MT_PLC_hb:
			return platform_add_devices(
			    plc_hb_devices,  ARRAY_SIZE(plc_hb_devices));
		case MACH_MT_LHG60_hb:
			return platform_add_devices(
			    lhg60_hb_devices,  ARRAY_SIZE(lhg60_hb_devices));
		case MACH_MT_mAPL:
			return platform_add_hwopt_devices(
			    mapl_devices,  ARRAY_SIZE(mapl_devices));
		case MACH_MT_mAP2:
			return platform_add_hwopt_devices(
			    map2_devices,  ARRAY_SIZE(map2_devices));
		case MACH_MT_wsAP:
			return platform_add_devices(
			    wsAP_devices,  ARRAY_SIZE(wsAP_devices));
		case MACH_MT_RB952: {
			if (!strcmp(read_rb_name(), "951Ui-2nD"))
				return platform_add_devices(
				    rb951Ui2nD_devices,  ARRAY_SIZE(rb951Ui2nD_devices));
			else
				return platform_add_devices(
				    rb952_devices,  ARRAY_SIZE(rb952_devices));
			}
		case MACH_MT_RB750r2:
			return platform_add_hwopt_devices(
			    rb750r2_devices,  ARRAY_SIZE(rb750r2_devices));
		case MACH_MT_CRS125G:
			return platform_add_hwopt_devices(
			    crs125g_devices,  ARRAY_SIZE(crs125g_devices));
		case MACH_MT_CRS109:
			return platform_add_hwopt_devices(
			    crs109_devices,  ARRAY_SIZE(crs109_devices));
		case MACH_MT_GROOVE52:
			return platform_add_hwopt_devices(
			    groove52_devices,  ARRAY_SIZE(groove52_devices));
		case MACH_MT_QRTG:
			return platform_add_devices(
			    qrtg_devices,  ARRAY_SIZE(qrtg_devices));
		case MACH_MT_RB951:
			return platform_add_devices(
			    rb951_devices,  ARRAY_SIZE(rb951_devices));
		case MACH_MT_RB_GROOVE:
			return platform_add_devices(
			    rb_groove_devices,  ARRAY_SIZE(rb_groove_devices));
		case MACH_MT_RB750:
			return platform_add_devices(
				rb750_devices,  ARRAY_SIZE(rb750_devices));
		case MACH_MT_RB751:
			return platform_add_devices(
				rb751_devices,  ARRAY_SIZE(rb751_devices));
		case MACH_MT_RB_OMNI_5FE:
			return platform_add_devices(
				rb_omni_5fe_devices,
				ARRAY_SIZE(rb_omni_5fe_devices));
		}
		break;
	case MACH_GROUP_MT_CR:
		return platform_add_devices(
		    cr_devices, ARRAY_SIZE(cr_devices));
	case MACH_GROUP_MT_VM:
		return 0;
	case MACH_GROUP_MT_MUSIC:
		switch (mips_machtype) {
		case MACH_MT_CCS226:
			return platform_add_hwopt_devices(
				ccs226_devices,  ARRAY_SIZE(ccs226_devices));
		case MACH_MT_CCS112:
			return platform_add_devices(
				ccs112_devices,  ARRAY_SIZE(ccs112_devices));
		case MACH_MT_CCS112R4:
			return platform_add_hwopt_devices(
				ccs112r4_devices,  ARRAY_SIZE(ccs112r4_devices));
		case MACH_MT_CRS106:
			return platform_add_devices(
				crs106_devices,  ARRAY_SIZE(crs106_devices));
		case MACH_MT_CRS105:
			return platform_add_devices(
				crs105_devices,  ARRAY_SIZE(crs105_devices));
		case MACH_MT_CRS210:
			return platform_add_hwopt_devices(
				crs210_devices,  ARRAY_SIZE(crs210_devices));
		case MACH_MT_CRS212:
			return platform_add_hwopt_devices(
				crs212_devices,  ARRAY_SIZE(crs212_devices));
		}
		break;
	case MACH_GROUP_MT_MMIPS:
		switch (mips_machtype) {
		case MACH_MT_RB750Gr3:
			return platform_add_hwopt_devices(
			    rb750gr3_devices, ARRAY_SIZE(rb750gr3_devices));
		case MACH_MT_RB750Gr4:
			return platform_add_hwopt_devices(
			    rb750gr4_devices, ARRAY_SIZE(rb750gr4_devices));
		case MACH_MT_M11:
			return platform_add_devices(
			    m11_devices, ARRAY_SIZE(m11_devices));
		case MACH_MT_M33:
			return platform_add_devices(
			    m33_devices, ARRAY_SIZE(m33_devices));
		}
		break;
	}

	printk("ERROR: unknown RB version\n");
	return 0;
}

arch_initcall(rb_platform_init);
