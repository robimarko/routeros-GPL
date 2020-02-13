#ifndef __LINUX_POEOUT_H
#define __LINUX_POEOUT_H

struct poeout_port {
    int eth_port;
    int gpo_on;
    int gpi_status;
    int gpi_status_valid;
    int invert_gpo_on:1;
    int has_low_current_sense:1;
};

#define POE_MAX_CHANNELS	8

#define POE_PIN_UNUSED	0xff
#define POE_PIN_SPI_CS	0xfe
#define POE_PIN_SPI_CLK	0xfd
#define POE_PIN_GPIO_INVERT	0x80
struct poe_platform_data {
	unsigned char reset;
	unsigned swdelay;
	unsigned char swcs;
	unsigned char swclk;
	unsigned char swdio;
	unsigned char psuCnt;
	unsigned char chCnt;
	unsigned char chMap[POE_MAX_CHANNELS];
};

#endif
