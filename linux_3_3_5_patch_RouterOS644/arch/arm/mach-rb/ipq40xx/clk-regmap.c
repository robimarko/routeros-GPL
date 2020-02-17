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

#include <linux/of_address.h>
#include <linux/device.h>
#include <linux/regmap.h>
#include <linux/export.h>
#include <linux/slab.h>

//#include "clk.h"
//#include "clk-provider.h"
#include "clk-regmap.h"


#define CLOCK_BASE	0x1800000


struct regmap*
qcom_regmap_init_by_name(struct device *dev, const struct qcom_cc_desc *desc,
			 const char *name)
{
	struct regmap *map;
	struct resource res;
	struct device_node *nd;

	nd = of_find_compatible_node(NULL, NULL, name);
	if (!nd) {
                pr_err("%s: can't find node\n", __func__);
                return -EINVAL;
        }

        if (of_address_to_resource(nd, 0, &res)) {
                of_node_put(nd);
                return -EINVAL;
        }
	
	map = (struct regmap*)devm_kzalloc(dev, sizeof(struct regmap), GFP_KERNEL);

	map->regmap_phys_base = CLOCK_BASE;
	map->regmap_virt_base = (unsigned)of_iomap(nd, 0);
	return map;
}

struct regmap*
qcom_regmap_init(struct device *dev, const struct qcom_cc_desc *desc)
{
	return qcom_regmap_init_by_name(dev, desc, "qcom,gcc-ipq40xx");
}

int clk_is_enabled_regmap(struct clk_hw *hw)
{
	struct clk_regmap *rclk = to_clk_regmap(hw);
	unsigned int val;

	reg_read(rclk->regmap, rclk->enable_reg, &val);

	if (rclk->enable_is_inverted)
		return (val & rclk->enable_mask) == 0;
	else
		return (val & rclk->enable_mask) != 0;
}

int clk_enable_regmap(struct clk_hw *hw)
{
	struct clk_regmap *rclk = to_clk_regmap(hw);
	unsigned int val;

	if (rclk->enable_is_inverted)
		val = 0;
	else
		val = rclk->enable_mask;

	reg_update_bits(rclk->regmap, rclk->enable_reg,
				  rclk->enable_mask, val);
	return 0;
}

void clk_disable_regmap(struct clk_hw *hw)
{
	struct clk_regmap *rclk = to_clk_regmap(hw);
	unsigned int val;

	if (rclk->enable_is_inverted)
		val = rclk->enable_mask;
	else
		val = 0;

	reg_update_bits(rclk->regmap, rclk->enable_reg, rclk->enable_mask,
			   val);
}


void reg_read(struct regmap *map, unsigned int reg, unsigned int *val)
{
	*val = readl(map->regmap_virt_base + reg);
}

void reg_write(struct regmap *map, unsigned int reg, unsigned int val)
{
	writel(val, map->regmap_virt_base + reg);
}

void reg_update_bits(struct regmap *map, unsigned int reg,
                            unsigned int mask, unsigned int val)
{
	unsigned new_val = readl(map->regmap_virt_base + reg);
	new_val &= ~mask;
	new_val |= val & mask;
	writel(new_val, map->regmap_virt_base + reg);
}
