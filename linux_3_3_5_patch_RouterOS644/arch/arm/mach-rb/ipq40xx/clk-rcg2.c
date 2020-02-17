/*
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
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

#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/bug.h>
#include <linux/export.h>
#include <linux/delay.h>
#include <linux/regmap.h>
#include <linux/math64.h>

#include <asm/div64.h>

#include <mach/clk-provider-dk.h>
#include "clk-rcg.h"
#include "clk-regmap.h"
#include "common.h"
#include "clk.h"

#define CMD_REG			0x0
#define CMD_UPDATE		BIT(0)
#define CMD_ROOT_EN		BIT(1)
#define CMD_DIRTY_CFG		BIT(4)
#define CMD_DIRTY_N		BIT(5)
#define CMD_DIRTY_M		BIT(6)
#define CMD_DIRTY_D		BIT(7)
#define CMD_ROOT_OFF		BIT(31)

#define CFG_REG			0x4
#define CFG_SRC_DIV_SHIFT	0
#define CFG_SRC_SEL_SHIFT	8
#define CFG_SRC_SEL_MASK	(0x7 << CFG_SRC_SEL_SHIFT)
#define CFG_MODE_SHIFT		12
#define CFG_MODE_MASK		(0x3 << CFG_MODE_SHIFT)
#define CFG_MODE_DUAL_EDGE	(0x2 << CFG_MODE_SHIFT)

#define M_REG			0x8
#define N_REG			0xc
#define D_REG			0x10
#define FEPLL_500_SRC		0x2

static int clk_rcg2_is_enabled(struct clk_hw *hw)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	u32 cmd;

	reg_read(rcg->clkr.regmap, rcg->cmd_rcgr + CMD_REG, &cmd);

	return (cmd & CMD_ROOT_OFF) == 0;
}

static u8 clk_rcg2_get_parent(struct clk_hw *hw)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	int num_parents = hw->clk->num_parents;
	u32 cfg;
	int i;

	reg_read(rcg->clkr.regmap, rcg->cmd_rcgr + CFG_REG, &cfg);

	cfg &= CFG_SRC_SEL_MASK;
	cfg >>= CFG_SRC_SEL_SHIFT;

	for (i = 0; i < num_parents; i++)
		if (cfg == rcg->parent_map[i])
			return i;

	return -EINVAL;
}

static int update_config(struct clk_rcg2 *rcg)
{
	int count;
	u32 cmd;
	struct clk_hw *hw = &rcg->clkr.hw;
	const char *name = hw->clk->name;
	u32 flags;

	flags = clk_get_flags(hw->clk);

	reg_update_bits(rcg->clkr.regmap, rcg->cmd_rcgr + CMD_REG,
				 CMD_UPDATE, CMD_UPDATE);

	if (flags && CLK_RCG2_NO_WAIT) {
		return 0;
	} else {
		/* Wait for update to take effect */
		for (count = 500; count > 0; count--) {
			reg_read(rcg->clkr.regmap, rcg->cmd_rcgr +
								CMD_REG, &cmd);
			if (!(cmd & CMD_UPDATE))
				return 0;
			udelay(1);
		}

		WARN(1, "%s: rcg didn't update its configuration.", name);
	}
	return 0;
}

static int clk_rcg2_set_parent(struct clk_hw *hw, u8 index)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);

	reg_update_bits(rcg->clkr.regmap, rcg->cmd_rcgr + CFG_REG,
				 CFG_SRC_SEL_MASK,
				 rcg->parent_map[index] << CFG_SRC_SEL_SHIFT);

	return update_config(rcg);
}

/*
 * Calculate m/n:d rate
 *
 *          parent_rate     m
 *   rate = ----------- x  ---
 *            hid_div       n
 */
static unsigned long
calc_rate(unsigned long rate, u32 m, u32 n, u32 mode, u32 hid_div)
{
	if (hid_div) {
		rate *= 2;
		rate /= hid_div + 1;
	}

	if (mode) {
		u64 tmp = rate;
		tmp *= m;
		do_div(tmp, n);
		rate = tmp;
	}

	return rate;
}

static unsigned long
clk_rcg2_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	u32 cfg, hid_div, m = 0, n = 0, mode = 0, mask;

	reg_read(rcg->clkr.regmap, rcg->cmd_rcgr + CFG_REG, &cfg);

	if (rcg->mnd_width) {
		mask = BIT(rcg->mnd_width) - 1;
		reg_read(rcg->clkr.regmap, rcg->cmd_rcgr + M_REG, &m);
		m &= mask;
		reg_read(rcg->clkr.regmap, rcg->cmd_rcgr + N_REG, &n);
		n =  ~n;
		n &= mask;
		n += m;
		mode = cfg & CFG_MODE_MASK;
		mode >>= CFG_MODE_SHIFT;
	}

	mask = BIT(rcg->hid_width) - 1;
	hid_div = cfg >> CFG_SRC_DIV_SHIFT;
	hid_div &= mask;

	return calc_rate(parent_rate, m, n, mode, hid_div);
}

static long _freq_tbl_determine_rate(struct clk_hw *hw,
		const struct freq_tbl *f, unsigned long rate,
		unsigned long *p_rate, struct clk **p)
{
	unsigned long clk_flags;

	f = qcom_find_freq(f, rate);
	if (!f)
		return -EINVAL;

	clk_flags = clk_get_flags(hw->clk);
	*p = clk_get_parent_by_index(hw->clk, f->src);
	if (clk_flags & CLK_SET_RATE_PARENT) {
		if (f->pre_div) {
			rate /= 2;
			rate *= f->pre_div + 1;
		}

		if (f->n) {
			u64 tmp = rate;
			tmp = tmp * f->n;
			do_div(tmp, f->m);
			rate = tmp;
		}
	} else {
		rate =  clk_get_rate_dk(*p);
	}
	*p_rate = rate;

	return f->freq;
}

static long clk_rcg2_determine_rate(struct clk_hw *hw, unsigned long rate,
		unsigned long *p_rate, struct clk **p)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);

	return _freq_tbl_determine_rate(hw, rcg->freq_tbl, rate, p_rate, p);
}

static int clk_rcg2_configure(struct clk_rcg2 *rcg, const struct freq_tbl *f)
{
	u32 cfg, mask;

	if (rcg->mnd_width && f->n) {
		mask = BIT(rcg->mnd_width) - 1;
		reg_update_bits(rcg->clkr.regmap,
				rcg->cmd_rcgr + M_REG, mask, f->m);
		reg_update_bits(rcg->clkr.regmap,
				rcg->cmd_rcgr + N_REG, mask, ~(f->n - f->m));
		reg_update_bits(rcg->clkr.regmap,
				rcg->cmd_rcgr + D_REG, mask, ~f->n);
	}

	mask = BIT(rcg->hid_width) - 1;
	mask |= CFG_SRC_SEL_MASK | CFG_MODE_MASK;
	cfg = f->pre_div << CFG_SRC_DIV_SHIFT;
	cfg |= rcg->parent_map[f->src] << CFG_SRC_SEL_SHIFT;
	if (rcg->mnd_width && f->n && (f->m != f->n))
		cfg |= CFG_MODE_DUAL_EDGE;
	reg_update_bits(rcg->clkr.regmap,
			rcg->cmd_rcgr + CFG_REG, mask, cfg);

	return update_config(rcg);
}

static int __clk_rcg2_set_rate(struct clk_hw *hw, unsigned long rate)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	const struct freq_tbl *f;

	f = qcom_find_freq(rcg->freq_tbl, rate);
	if (!f)
		return -EINVAL;

	return clk_rcg2_configure(rcg, f);
}

static int clk_rcg2_set_rate(struct clk_hw *hw, unsigned long rate,
			    unsigned long parent_rate)
{
	return __clk_rcg2_set_rate(hw, rate);
}

static int clk_rcg2_set_rate_and_parent(struct clk_hw *hw,
		unsigned long rate, unsigned long parent_rate, u8 index)
{
	return __clk_rcg2_set_rate(hw, rate);
}

const struct clk_ops clk_rcg2_ops = {
	.is_enabled = clk_rcg2_is_enabled,
	.get_parent = clk_rcg2_get_parent,
	.set_parent = clk_rcg2_set_parent,
	.recalc_rate = clk_rcg2_recalc_rate,
	.determine_rate = clk_rcg2_determine_rate,
	.set_rate = clk_rcg2_set_rate,
	.set_rate_and_parent = clk_rcg2_set_rate_and_parent,
};

static int clk_cpu_rcg2_is_enabled(struct clk_hw *hw)
{
	struct clk_cdiv_rcg2 *rcg = to_clk_cdiv_rcg2(hw);
	u32 cmd;

	reg_read(rcg->clkr.regmap, rcg->cmd_rcgr + CMD_REG, &cmd);

	return (cmd & CMD_ROOT_OFF) == 0;

}

static u8 clk_cpu_rcg2_get_parent(struct clk_hw *hw)
{
	struct clk_cdiv_rcg2 *rcg = to_clk_cdiv_rcg2(hw);
	int num_parents = hw->clk->num_parents;
	u32 cfg;
	int i;

	reg_read(rcg->clkr.regmap, rcg->cmd_rcgr + CFG_REG, &cfg);

	cfg &= CFG_SRC_SEL_MASK;
	cfg >>= CFG_SRC_SEL_SHIFT;

	for (i = 0; i < num_parents; i++)
		if (cfg == rcg->parent_map[i])
			return i;

	return -EINVAL;
}

static int cpu_rcg2_update_config(struct clk_cdiv_rcg2 *rcg)
{
	int count;
	u32 cmd;
	struct clk_hw *hw = &rcg->clkr.hw;
	const char *name = hw->clk->name;

	reg_update_bits(rcg->clkr.regmap, rcg->cmd_rcgr + CMD_REG,
				 CMD_UPDATE, CMD_UPDATE);

	/* Wait for update to take effect */
	for (count = 500; count > 0; count--) {
		reg_read(rcg->clkr.regmap, rcg->cmd_rcgr + CMD_REG, &cmd);
		if (!(cmd & CMD_UPDATE))
			return 0;
		udelay(1);
	}

	WARN(1, "%s: rcg didn't update its configuration.", name);
	return 0;
}
static int clk_cpu_rcg2_set_parent(struct clk_hw *hw, u8 index)
{
	struct clk_cdiv_rcg2 *rcg = to_clk_cdiv_rcg2(hw);

	reg_update_bits(rcg->clkr.regmap, rcg->cmd_rcgr + CFG_REG,
				 CFG_SRC_SEL_MASK,
				 rcg->parent_map[index] << CFG_SRC_SEL_SHIFT);

	return cpu_rcg2_update_config(rcg);
}


/*
 * These are used for looking up the actual divider ratios
 * the divider used for DDR PLL Post divider is not linear,
 * hence we need this look up table
 */
static const unsigned char ddrpll_div[] = {
		12,
		13,
		14,
		15,
		16,
		17,
		18,
		19,
		20,
		21,
		22,
		24,
		26,
		28
};

static unsigned long
clk_cpu_rcg2_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct clk_cdiv_rcg2 *rcg = to_clk_cdiv_rcg2(hw);
	u32 cfg, hid_div , m = 0 , n = 0 , mode = 0 , mask , cdiv;
	unsigned long rate;
	u32 src;

	reg_read(rcg->clkr.regmap, rcg->cmd_rcgr + CFG_REG, &cfg);

	if (rcg->mnd_width) {
		mask = BIT(rcg->mnd_width) - 1;
		reg_read(rcg->clkr.regmap, rcg->cmd_rcgr + M_REG, &m);
		m &= mask;
		reg_read(rcg->clkr.regmap, rcg->cmd_rcgr + N_REG, &n);
		n =  ~n;
		n &= mask;
		n += m;
		mode = cfg & CFG_MODE_MASK;
		mode >>= CFG_MODE_SHIFT;
	}

	mask = BIT(rcg->hid_width) - 1;
	hid_div = cfg >> CFG_SRC_DIV_SHIFT;
	hid_div &= mask;
	rate = calc_rate(parent_rate, m, n, mode, hid_div);
	src = (cfg >> CFG_SRC_SEL_SHIFT) & 0xf;
	if (src == 0x1) {
		reg_read(rcg->clkr.regmap, rcg->cdiv.offset, &cdiv);
		cdiv &= (rcg->cdiv.mask << rcg->cdiv.shift);
		cdiv = cdiv >> rcg->cdiv.shift;
		do_div(rate , ddrpll_div[cdiv]);
		rate *= 16;
		do_div(rate , 1000000);
		rate = rate * 1000000;
	}
	return rate;
}

static long _cpu_rcg2_freq_tbl_determine_rate(struct clk_hw *hw,
		const struct freq_tbl *f, unsigned long rate,
		unsigned long *p_rate, struct clk **p)
{
	unsigned long clk_flags;

	f = qcom_find_freq(f, rate);
	if (!f)
		return -EINVAL;

	clk_flags = clk_get_flags(hw->clk);
	*p = clk_get_parent_by_index(hw->clk, f->src);
	rate = clk_get_rate_dk(*p);
	*p_rate = rate;

	return f->freq;
}

static long clk_cpu_rcg2_determine_rate(struct clk_hw *hw, unsigned long rate,
		unsigned long *p_rate, struct clk **p)
{
	struct clk_cdiv_rcg2 *rcg = to_clk_cdiv_rcg2(hw);

	return _cpu_rcg2_freq_tbl_determine_rate(hw, rcg->freq_tbl,
							rate, p_rate, p);
}


static int clk_cpu_rcg2_configure(struct clk_cdiv_rcg2 *rcg,
						const struct freq_tbl *f)
{
	u32 cfg, mask;

	if (rcg->mnd_width && f->n) {
		mask = BIT(rcg->mnd_width) - 1;
		reg_update_bits(rcg->clkr.regmap,
				rcg->cmd_rcgr + M_REG, mask, f->m);

		reg_update_bits(rcg->clkr.regmap,
				rcg->cmd_rcgr + N_REG, mask, ~(f->n - f->m));

		reg_update_bits(rcg->clkr.regmap,
				rcg->cmd_rcgr + D_REG, mask, ~f->n);
	}

	if ((rcg->parent_map[f->src] == 0x01)) {
		mask = (BIT(rcg->hid_width) - 1);
		mask |= CFG_SRC_SEL_MASK | CFG_MODE_MASK;
		cfg = FEPLL_500_SRC << CFG_SRC_SEL_SHIFT;
		cfg |= (1 << CFG_SRC_DIV_SHIFT);
		reg_update_bits(rcg->clkr.regmap,
					rcg->cmd_rcgr + CFG_REG, mask, cfg);
		cpu_rcg2_update_config(rcg);
		mask = (rcg->cdiv.mask)<<rcg->cdiv.shift;
		reg_update_bits(rcg->clkr.regmap,
					rcg->cdiv.offset, mask,
				(f->pre_div << rcg->cdiv.shift) & mask);
		udelay(1);
		mask = BIT(rcg->hid_width) - 1;
		mask |= CFG_SRC_SEL_MASK | CFG_MODE_MASK;
		cfg = 1 << CFG_SRC_DIV_SHIFT;
	} else {
		mask = BIT(rcg->hid_width) - 1;
		mask |= CFG_SRC_SEL_MASK | CFG_MODE_MASK;
		cfg = f->pre_div << CFG_SRC_DIV_SHIFT;
	}

	cfg |= rcg->parent_map[f->src] << CFG_SRC_SEL_SHIFT;
	if (rcg->mnd_width && f->n)
		cfg |= CFG_MODE_DUAL_EDGE;
	reg_update_bits(rcg->clkr.regmap,
					rcg->cmd_rcgr + CFG_REG, mask, cfg);

	return cpu_rcg2_update_config(rcg);
}

static long __clk_cpu_rcg2_set_rate(struct clk_hw *hw, unsigned long rate)
{
	struct clk_cdiv_rcg2 *rcg = to_clk_cdiv_rcg2(hw);
	const struct freq_tbl *f;

	f = qcom_find_freq(rcg->freq_tbl, rate);
	if (!f)
		return -EINVAL;

	return clk_cpu_rcg2_configure(rcg, f);
}

static int clk_cpu_rcg2_set_rate(struct clk_hw *hw, unsigned long rate,
			    unsigned long parent_rate)
{
	return __clk_cpu_rcg2_set_rate(hw, rate);
}

static int clk_cpu_rcg2_set_rate_and_parent(struct clk_hw *hw,
		unsigned long rate, unsigned long parent_rate, u8 index)
{
	return __clk_cpu_rcg2_set_rate(hw, rate);
}

const struct clk_ops clk_cpu_rcg2_ops = {
	.is_enabled	=	clk_cpu_rcg2_is_enabled,
	.get_parent	=	clk_cpu_rcg2_get_parent,
	.set_parent	=	clk_cpu_rcg2_set_parent,
	.recalc_rate	=	clk_cpu_rcg2_recalc_rate,
	.determine_rate	=	clk_cpu_rcg2_determine_rate,
	.set_rate	=	clk_cpu_rcg2_set_rate,
	.set_rate_and_parent	=	clk_cpu_rcg2_set_rate_and_parent,
};
