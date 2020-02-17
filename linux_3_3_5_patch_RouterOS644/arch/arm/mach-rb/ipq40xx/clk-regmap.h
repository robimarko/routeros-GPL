/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __QCOM_CLK_REGMAP_H__
#define __QCOM_CLK_REGMAP_H__

#include <mach/clk-provider-dk.h>
#include "common.h"


#define to_clk_regmap(_hw) container_of(_hw, struct clk_regmap, hw)

struct clk_regmap_div {
	u32			reg;
	u32			shift;
	u32			width;
	struct clk_regmap	clkr;
};

extern const struct clk_ops clk_regmap_div_ops;

int clk_is_enabled_regmap(struct clk_hw *hw);
int clk_enable_regmap(struct clk_hw *hw);
void clk_disable_regmap(struct clk_hw *hw);
void reg_write(struct regmap *map, unsigned int reg, unsigned int val);
void reg_read(struct regmap *map, unsigned int reg, unsigned int *val);
void reg_update_bits(struct regmap *map, unsigned int reg,
		       unsigned int mask, unsigned int val);

struct regmap*
qcom_regmap_init(struct device *dev, const struct qcom_cc_desc *desc);

#endif
