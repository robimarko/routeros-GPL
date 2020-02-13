/*
 * Copyright (c) 2013-2014, The Linux Foundation. All rights reserved.
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

#include <linux/export.h>
#include <linux/regmap.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <mach/clk-provider-dk.h>
#include <linux/err.h>

#include "common.h"
#include "clk-rcg.h"
#include "clk-regmap.h"

#include <linux/delay.h>

#define IS_ERR(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)

struct qcom_cc {
	struct clk_onecell_data data;
	struct clk *clks[];
};

const
struct freq_tbl *qcom_find_freq(const struct freq_tbl *f, unsigned long rate)
{
	if (!f)
		return NULL;

	for (; f->freq; f++)
		if (rate <= f->freq)
			return f;

	/* Default to our fastest rate */
	return f - 1;
}

struct regmap *
qcom_cc_map(struct platform_device *pdev, const struct qcom_cc_desc *desc)
{
	return qcom_regmap_init(&pdev->dev, desc);
}

int qcom_cc_really_probe(struct platform_device *pdev,
			 const struct qcom_cc_desc *desc, struct regmap *regmap)
{
	int i, ret;
	struct device *dev = &pdev->dev;
	struct clk *clk;
	struct clk_onecell_data *data;
	struct clk **clks;
	struct qcom_cc *cc;
	size_t num_clks = desc->num_clks;
	struct clk_regmap **rclks = desc->clks;

	cc = devm_kzalloc(dev, sizeof(*cc) + sizeof(*clks) * num_clks,
			  GFP_KERNEL);
	if (!cc)
		return -ENOMEM;

	clks = cc->clks;
	data = &cc->data;
	data->clks = clks;
	data->clk_num = num_clks;

	for (i = 0; i < num_clks; i++) {
		if (!rclks[i]) {
			clks[i] = -ENOENT;
			continue;
		}
//		clk = devm_clk_register_regmap(dev, rclks[i]);
		rclks[i]->regmap = regmap;
		clk = devm_clk_register(dev, &rclks[i]->hw);
		clks[i] = clk;
	}

	ret = of_clk_add_provider(dev->of_node, of_clk_src_onecell_get, data);
	if (ret)
		return ret;

	return ret;
}

int qcom_cc_probe(struct platform_device *pdev, const struct qcom_cc_desc *desc)
{
	struct regmap *regmap;

	regmap = qcom_cc_map(pdev, desc);
	if (IS_ERR(regmap)){
		printk("regmap init failed\n");
		return regmap;
	}

	return qcom_cc_really_probe(pdev, desc, regmap);
}

void qcom_cc_remove(struct platform_device *pdev)
{
	of_clk_del_provider(pdev->dev.of_node);
}
