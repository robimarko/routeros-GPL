/* Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/cpumask.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <mach/clk-provider-dk.h>

#include "common.h"
#include "clk-rcg.h"
#include "clk.h"

#ifdef CONFIG_ARCH_RB
#include <mach/system.h>
#endif

#define CPUFREQ_TABLE_END 	~1
#define IPQ40XX_DEFAULT_FREQ	3

static unsigned int transition_latency;
static DEFINE_PER_CPU(struct clk *, cpu_clks);
struct cpufreq_frequency_table *ftbl;

static unsigned int max_scalable_frequency;
static unsigned int default_frequency;

static struct cpufreq_frequency_table *ipq40xx_fill_freq_tbl(struct device *dev, int cpu)
{
	u32 i, nf = 0xb;
	unsigned long f;

	ftbl = devm_kzalloc(dev, (nf + 1) * sizeof(*ftbl), GFP_KERNEL);
	if (!ftbl)
		return ERR_PTR(-ENOMEM);

	for (i = 0; i < nf; i++) {
		f = ipq40xx_get_freq_index(nf - i);
		if (IS_ERR_VALUE(f))
			break;

		ftbl[i].index = i;
		ftbl[i].frequency = f / 1000;
	}

	ftbl[i].index = i;
	ftbl[i].frequency = CPUFREQ_TABLE_END;

	return ftbl;
}

static struct cpufreq_frequency_table *cpufreq_parse_dt(struct device *dev,
						char *tbl_name, int cpu)
{
	u32 ret, nf, i;
	u32 *data;

	/* Parse list of usable CPU frequencies. */
	if (!of_find_property(dev->of_node, tbl_name, &nf))
		return ERR_PTR(-EINVAL);
	nf /= sizeof(*data);

	if (nf == 0)
		return ERR_PTR(-EINVAL);

	data = devm_kzalloc(dev, nf * sizeof(*data), GFP_KERNEL);
	if (!data)
		return ERR_PTR(-ENOMEM);

	ret = of_property_read_u32_array(dev->of_node, tbl_name, data, nf);
	if (ret)
		return ERR_PTR(ret);

	ftbl = devm_kzalloc(dev, (nf + 1) * sizeof(*ftbl), GFP_KERNEL);
	if (!ftbl)
		return ERR_PTR(-ENOMEM);

	for (i = 0; i < nf; i++) {
		unsigned long f;
		struct clk *cpu_clk;

		cpu_clk = per_cpu(cpu_clks, cpu);

		f =data[i]*1000;
		if (IS_ERR_VALUE(f))
			break;
		f /= 1000;

		ftbl[i].index = i;
		ftbl[i].frequency = f;
	}

	ftbl[i].index = i;
	ftbl[i].frequency = CPUFREQ_TABLE_END;

	devm_kfree(dev, data);

	return ftbl;
}

static unsigned int ipq40xx_get_target(unsigned int cpu)
{
	struct clk *cpu_clk;
	unsigned freq = 0;

	cpu_clk = per_cpu(cpu_clks, cpu);
	if (cpu_clk)
		freq = clk_get_rate_dk(cpu_clk) / 1000;

	return freq;
}

static int ipq40xx_cpufreq_init(struct cpufreq_policy *policy)
{
	unsigned int min_freq = ~0;
	unsigned int freq;
	struct cpufreq_frequency_table *pos;
	struct clk *clk = per_cpu(cpu_clks, policy->cpu);
	struct device_node *node;
	unsigned long frequency;

	default_frequency = ftbl[IPQ40XX_DEFAULT_FREQ].frequency;

	node = of_find_compatible_node(NULL, NULL, "ipq40xx,default_cpu_freq");
	if (node) {
		if (!of_property_read_u32(node, "default_freq", &frequency))
			default_frequency = frequency;
	}
	
	for (pos = ftbl; pos->frequency != CPUFREQ_TABLE_END; pos++) {
		if (pos->frequency == CPUFREQ_ENTRY_INVALID)
			continue;
		else {
			freq = pos->frequency;
			if (freq < min_freq)
				min_freq = freq;
			if (freq > max_scalable_frequency)
				max_scalable_frequency = freq;
		}
	}

	cpufreq_frequency_table_get_attr(ftbl, 0);

	policy->cpuinfo.min_freq = min_freq; 
	policy->min = min_freq;
	policy->cpuinfo.max_freq = default_frequency;
	policy->max = default_frequency;
	policy->cur = clk_get_rate_dk(clk);
	policy->cpuinfo.transition_latency = transition_latency;
	cpumask_setall(policy->cpus);
	return 0;
}

static int ipq40xx_set_target(struct cpufreq_policy *policy, unsigned int target_freq, unsigned int relation)
{
	int ret;
	long freq = target_freq;
	struct clk *cpu_clk;

	if (!strcmp(policy->governor->name, "userspace")) 
		policy->max = max_scalable_frequency;
	else
		policy->max = default_frequency;

	if (target_freq < policy->min)
		freq = policy->min;
	if (target_freq > policy->max)
		freq = policy->max;

	cpu_clk = per_cpu(cpu_clks, policy->cpu);
	ret = clk_set_rate_dk(cpu_clk, freq * 1000);
	policy->cur = freq;
	cpumask_setall(policy->cpus);

	if (ret)
		pr_err("failed to set clock rate: %d\n", ret);

	return ret;
}

static int ipq40xx_verify (struct cpufreq_policy *policy)
{
	unsigned int tmp;

	if (policy->max < policy->min) {
		tmp = policy->max;
		policy->max = policy->min;
		policy->min = tmp;
	}
	if (policy->cur < policy->min)
		policy->cur = policy->min;
	if (policy->cur > policy->max)
		policy->cur = policy->max;
	return 0;
}

static struct freq_attr *ipq40xx_attributes[] = {
	&cpufreq_freq_attr_scaling_available_freqs,
	NULL,
};

static struct cpufreq_driver qca_ipq40xx_cpufreq_driver = {
	.flags = CPUFREQ_CONST_LOOPS,
	.target = ipq40xx_set_target,
	.get = ipq40xx_get_target,
	.init = ipq40xx_cpufreq_init,
	.name = "ipq40xx_freq",
	.verify = ipq40xx_verify,
	.attr = ipq40xx_attributes,
};

static int __init ipq40xx_cpufreq_probe(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	struct device *dev;
	struct clk *clk;
	unsigned int cpu;
	int ret;

	if (rb_mach != RB_MACH_IPQ40XX) 
		return -1;
	

	for_each_possible_cpu(cpu) {
		dev = get_cpu_device(cpu);
		if (!dev) {
			pr_err("failed to get A7 device\n");
			ret = -ENOENT;
			goto out_put_node;
		}
		per_cpu(cpu_clks, cpu) = clk = clk_get_by_name("apps_clk_src");
		if (IS_ERR(clk)) {
			pr_err("failed to get clk device\n");
			ret = PTR_ERR(clk);
			goto out_put_node;
		}
	}

//	ftbl = cpufreq_parse_dt(&pdev->dev, "qcom,cpufreq-table", 0);
	ftbl = ipq40xx_fill_freq_tbl(&pdev->dev, 0);


	np = of_node_get(pdev->dev.of_node);
	of_property_read_u32(np, "clock-latency", &transition_latency);

	if (!transition_latency) {
		pr_info("%s: Clock latency not found. Defaults...\n"
			, __func__);
		transition_latency = CPUFREQ_ETERNAL;
	}
	ret = cpufreq_register_driver(&qca_ipq40xx_cpufreq_driver);
	if (ret) {
		pr_err("failed register driver: %d\n", ret);
		goto out_put_node;
	}
	return 0;

out_put_node:
	of_node_put(np);
	return ret;
}

static int __exit ipq40xx_cpufreq_remove(struct platform_device *pdev)
{
	cpufreq_unregister_driver(&qca_ipq40xx_cpufreq_driver);
	return 0;
}

static struct of_device_id ipq40xx_match_table[] = {
	{.compatible = "qca,ipq40xx_freq"},
	{},
};

static struct platform_driver qca_ipq40xx_cpufreq_platdrv = {
	.probe		= ipq40xx_cpufreq_probe,
	.remove		= ipq40xx_cpufreq_remove,
	.driver = {
		.name	= "cpufreq-ipq40xx",
		.owner	= THIS_MODULE,
		.of_match_table = ipq40xx_match_table,
	},
};

module_platform_driver(qca_ipq40xx_cpufreq_platdrv);

MODULE_DESCRIPTION("QCA IPQ40XX CPUfreq driver");
