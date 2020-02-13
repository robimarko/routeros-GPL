#ifndef _RB_H
#define _RB_H

#define ID_MAC_ADDRESS_PACK     0x00000004
#define ID_BOARD_NAME           0x00000005
#define ID_HW_OPTIONS           0x00000015
#define ID_BOARD_TYPE_NAME      0x00000017

#define HW_OPT_HAS_uSD		(1 << 17)
#define HW_OPT_HAS_SIM		(1 << 18)

#define MMC_PINS		(0x3FULL << 54)

#define GPO_SD_PWR		(1ULL << 53)
#define GPO_USB_POWER		(1ULL << 47)
#define GPO_BEEP		(1ULL << 15)
#define GPO_USER_LED		(1ULL << 14)
#define GPO_FAN_ON(x)		(1ULL << ((x == 0) ? 10 : 61))
#define GPO_MON_SEL		(1ULL << 8)
#define GPO_MON_SEL72		(1ULL << 16)
#define GPO_LCD_LED	        (1ULL << 3)

#define GPI_SD_CDn		(1ULL << 52)
#define GPI_BUTTON		(1ULL << 17)
#define GPI_PIN_HOLE		(1ULL << 16)

#define GPIO_SIM_IO		(1ULL << 38)
#define GPO_SIM_CLK		(1ULL << 39)
#define GPO_SIM_RST		(1ULL << 40)
#define GPO_SIM_EN		(1ULL << 41)
#define GPO_SIM_3V_1V8n		(1ULL << 42)

#define GPI_PR_MISO		(1ULL << 8)
#define GPO_PR_STROBE1		(1ULL << 9)
#define GPO_PR_STROBE2		(1ULL << 34)
#define GPO_PR_CLK		(1ULL << 10)
#define GPO_PR_MOSI		(1ULL << 11)
#define GPO_AUX_RESET		(1ULL << 13)

#define GPO_USB_RST(x)		(1ULL << (32 + x))

#define RST_I2C			BIT(2)
#define RST_LED			BIT(8)
#define RST_LCD			BIT(10)
#define RST_ETH			BIT(11)
#define RST_PCI			BIT(0) | BIT(1) | BIT(9)
#define RST_USB			BIT(12) | BIT(13)

// GPI_SENSE(1) == GPIO9
// GPI_SENSE(2) == GPIO11
#define GPI_SENSE(x)		(1ULL << (7 + ((x) << 1)))

struct sim_gpio {
    unsigned long long io;
    unsigned long long clk;
    unsigned long long rst;
    unsigned long long en;
    unsigned long long vsel;
};

void __init ccr_init(void);
int nor_read(void *buf, int count, unsigned long offset);
int read_booter_cfg(unsigned id, void *buf, int amount);
int nor_program(void *buf, int count, unsigned long offset);
void set_gpio(unsigned long value, unsigned long mask);
unsigned long gpio_irq_refresh(unsigned long pin);
void gpio_irq(unsigned irq, unsigned long pin);
int is_name_prefix(char *str);
int is_board_type(char *str);
void ccr_deassert_usb_reset(unsigned i);
unsigned long get_gpio(void);
int fan_sense_count(void);
unsigned long fan_sense_gpios(void);
void usb_power_enable(void);
int is_CCR1072_r3(void);

unsigned long access_latch(int num, unsigned long value, unsigned long mask);

#endif
