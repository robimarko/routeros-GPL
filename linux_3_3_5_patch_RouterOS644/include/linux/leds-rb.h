#ifndef __LINUX_LEDS_RB_H_INCLUDED
#define __LINUX_LEDS_RB_H_INCLUDED

void rb_beepled(int on);
unsigned register_wifi_gpo(void *obj,
			   void (*set_gpo)(void *, unsigned, unsigned));

enum pled_name {
    PLED_NAME_user_led,
    PLED_NAME_led1,
    PLED_NAME_led2,
    PLED_NAME_led3,
    PLED_NAME_led4,
    PLED_NAME_led5,
    PLED_NAME_usb_power_off,
    PLED_NAME_power_led,
    PLED_NAME_wlan_led,
    PLED_NAME_pcie_power_off,		// mpcie-power-off
    PLED_NAME_pcie2_power_off,		// mpcie2-power-off
    PLED_NAME_lcd,
    PLED_NAME_button,
    PLED_NAME_pin_hole,
    PLED_NAME_fan_off,
    PLED_NAME_user_led2,
    PLED_NAME_sfp_led,
    PLED_NAME_link_act_led,
    PLED_NAME_all_leds,
    PLED_NAME_omni_led,
    PLED_NAME_ap_omni_led,
    PLED_NAME_ap_dir_led,
    PLED_NAME_control,
    PLED_NAME_heater,
    PLED_NAME_mode_button,
    PLED_NAME_sim_select,
    PLED_NAME_gps_mux,
    PLED_NAME_gps_ant_select,
    PLED_NAME_gps_reset,		// active low level
    PLED_NAME_monitor_select,
    PLED_NAME_fault,
    PLED_NAME_psu1_state,
    PLED_NAME_psu2_state,
    PLED_NAME_lte_reset,
    PLED_NAME_w_disable,
    PLED_NAME_lte_led,
    PLED_NAME_led_up,
    PLED_NAME_led_down,
    PLED_NAME_led_left,
    PLED_NAME_led_right,
    PLED_NAME_led_ok,
    PLED_NAME_eth_led,
    /* add name in drivers/leds/leds-rb.c as well */
};

enum pled_type {
    PLED_TYPE_GPIO = 0,
    PLED_TYPE_GPIO_OE = 1,
    PLED_TYPE_SHARED_GPIO = 2,
    PLED_TYPE_SHARED_RB400 = 4,
    PLED_TYPE_SSR_RB400 = 5,
    PLED_TYPE_SHARED_RB700 = 7,
    PLED_TYPE_SSR_MUSIC = 8,
    PLED_TYPE_SHARED_RB900 = 9,
    PLED_TYPE_SSR_RB900 = 10,
    PLED_TYPE_GPIOLIB = 11,
    PLED_TYPE_RGB7 = 13,
    PLED_TYPE_VIRTUAL = 14,
    PLED_TYPE_WIFI = 15,
};



#define PLED_CFG_INV		(1U << 31)	/* active-low ('on' == 0) */
#define PLED_CFG_ON		(1 << 30)	/* 'on' by default */
#define PLED_CFG_KEEP		(1 << 29)	/* keep val from RouterBOOT */
#define PLED_VALID		(1 << 28)	/* for internal use */
#define PLED_CFG_INPUT		(1 << 27)	/* input by default */
#define PLED_CFG_IO		(1 << 26)	/* allow GPIO direction change*/
#define PLED_CFG_DARK		(1 << 25)	/* affected when dark mode is on */

#define PLED_WHITE		0
#define PLED_RED		1
#define PLED_GREEN		2
#define PLED_BLUE		3

#define PLED_TYPE_MASK		0x3f
#define PLED_GET_BIT_NUM(val)	(((val) >> 8) & 0xff)
#define PLED_GET_BIT(val)	(1ULL << PLED_GET_BIT_NUM(val))
#define PLED_GET_TYPE(val)	((val) & PLED_TYPE_MASK)
#define PLED_GET_COLOUR(val)	(((val) >> 6) & 0x3)

#define PLED_GET_NAME_IDX(val)	(((val) >> 16) & 0xff)

#define PLEDN(nidx,bit,type,cfg,col)(((PLED_TYPE_##type) & PLED_TYPE_MASK) |	\
				 (((col) & 3) << 6) |		\
				 (((bit) & 0xff) << 8) |	\
				 (((nidx) & 0xff) << 16) |	\
				 (cfg) | PLED_VALID)
#define PLEDC(name,bit,type,cfg,col)	PLEDN(PLED_NAME_##name,bit,type,cfg,col)
#define PLED(name,bit,type,cfg)	PLEDC(name, bit, type, cfg, PLED_WHITE)
#define PLDI(name,bit,type)	PLED(name, bit, type, PLED_CFG_INV)
#define PLD(name,bit,type)	PLED(name, bit, type, 0)

#endif
