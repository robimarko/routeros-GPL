#ifndef __ASM_ARCH_MSM_BOARD_H
#define __ASM_ARCH_MSM_BOARD_H

struct msm_i2c_platform_data {
	int clk_freq;
	uint32_t rmutex;
	const char *rsl_id;
	uint32_t pm_lat;
	int pri_clk;
	int pri_dat;
	int aux_clk;
	int aux_dat;
	int src_clk_rate;
	int use_gsbi_shared_mode;
	int keep_ahb_clk_on;
	void (*msm_i2c_config_gpio)(int iface, int config_type);
};

#endif
