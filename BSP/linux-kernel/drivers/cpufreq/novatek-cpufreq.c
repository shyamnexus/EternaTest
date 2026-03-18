/**
    Novateck CPUFreq module
    To handle DVFS
    @file Novateck-cpufreq.c
    @ingroup
    @note
    Copyright Novatek Microelectronics Corp. 2022. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <linux/clk.h>
#include <linux/cpu.h>
#include <linux/clk-provider.h>
#include <linux/cpufreq.h>
#include <linux/cpumask.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_opp.h>
#include <linux/slab.h>
#include <plat/efuse_protected.h>



#define DRV_VERSION "1.00.007"

struct nvt_cpufreq_config {
	long int cpu_lower_freq_max;
	long int cpu_normal_freq_max;
	long int cpu_over_clking_freq_max;
	int cpu_mult;
	char pll_name[6];
};

struct ntk_cpu_dvfs_info {
	struct cpumask cpus;
	struct device *cpu_dev;
	struct clk *cpu_clk;
	struct clk *cpu_clk_parent;
	struct clk *cpu_clk_fix480;
	struct list_head list_head;
	long int freq_max;
	int cpu_mult;
	const struct nvt_cpufreq_config *soc_config;
};

static LIST_HEAD(dvfs_info_list);

static struct ntk_cpu_dvfs_info *ntk_cpu_dvfs_info_lookup(int cpu)
{
	struct ntk_cpu_dvfs_info *info;

	list_for_each_entry(info, &dvfs_info_list, list_head) {
		if (cpumask_test_cpu(cpu, &info->cpus)) {
			return info;
		}
	}

	return NULL;
}


static int ntk_cpufreq_set_target(struct cpufreq_policy *policy,
								  unsigned int index)
{
	struct cpufreq_frequency_table *freq_table = policy->freq_table;
	struct ntk_cpu_dvfs_info *info;
	struct clk *cpu_clk = policy->clk;
	long freq_hz, old_freq_hz;
	int ret;

	struct clk *cpu_clk_parent;
	info = ntk_cpu_dvfs_info_lookup(policy->cpu);

	cpu_clk_parent = info->cpu_clk_parent;

	old_freq_hz = clk_get_rate(cpu_clk);

	freq_hz = freq_table[index].frequency;

	if(freq_hz > info->freq_max) {
		pr_err("freq_hz %lu is over max!\n", freq_hz);
		return -1;
	}

	if (freq_hz == 480000) {
		if (clk_set_parent(cpu_clk_parent, info->cpu_clk_fix480)) {
			pr_err("cpu%d: set fixed 480MHz error!\n", policy->cpu);
		} else {
			if (__clk_is_enabled(cpu_clk)) {
				clk_disable(cpu_clk);
			}
		}
		return 0;
	} else {
		if (!__clk_is_enabled(info->cpu_clk)) {
			clk_enable(info->cpu_clk);
		}

		if (clk_set_parent(cpu_clk_parent, info->cpu_clk)) {
			pr_err("cpu%d: set pll error\n", policy->cpu);
		}
	
		ret = clk_set_rate(cpu_clk, freq_hz * 1000 / info->cpu_mult);
		if (ret) {
			pr_err("cpu%d: failed to scale cpu clock rate!\n",
				   policy->cpu);
			return ret;
		}

		return 0;
	}

	return 0;

}

static int ntk_cpu_dvfs_info_init(struct ntk_cpu_dvfs_info *info, int cpu)
{
	struct device *cpu_dev;
	struct clk *cpu_clk = ERR_PTR(-ENODEV);
	struct clk *cpu_clk_parent = ERR_PTR(-ENODEV);
	struct clk *cpu_clk_fix480 = ERR_PTR(-ENODEV);
	int ret;

	/*Force downgrade*/
#if defined(CONFIG_NVT_IVOT_PLAT_NS02201)
	if (quary_secure_boot(SECUREBOOT_CPU_LOWER_CLOCKING)) {
		info->freq_max = info->soc_config->cpu_lower_freq_max;
	} else {
		if(quary_secure_boot(SECUREBOOT_CPU_OVER_CLOCKING)) {
			info->freq_max = info->soc_config->cpu_over_clking_freq_max;
		} else {
			info->freq_max = info->soc_config->cpu_normal_freq_max;
		}
	}
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02302)
	if (efuse_check_available_extend(EFUSE_ABILITY_CPU_PLL_FREQ, info->soc_config->cpu_over_clking_freq_max * 1000)) {
		info->freq_max = info->soc_config->cpu_over_clking_freq_max;
	} else {
		info->freq_max = info->soc_config->cpu_normal_freq_max;
	}
#else
	info->freq_max = info->soc_config->cpu_normal_freq_max;
#endif

	info->cpu_mult = info->soc_config->cpu_mult;

	cpu_dev = get_cpu_device(cpu);
	if (!cpu_dev) {
		pr_err("failed to get cpu%d device\n", cpu);
		return -ENODEV;
	}

	cpu_clk = clk_get(cpu_dev, info->soc_config->pll_name);
	if (IS_ERR(cpu_clk)) {
		if (PTR_ERR(cpu_clk) == -EPROBE_DEFER) {
			pr_warn("cpu clk for cpu%d not ready, retry.\n", cpu);
		} else {
			pr_err("failed to get cpu clk for cpu%d\n", cpu);
		}

		ret = PTR_ERR(cpu_clk);
		return ret;
	}

	cpu_clk_fix480 = clk_get(cpu_dev, "fix480m");
	if (IS_ERR(cpu_clk_fix480)) {
		if (PTR_ERR(cpu_clk_fix480) == -EPROBE_DEFER) {
			pr_warn("cpu clk for cpu%d not ready, retry.\n", cpu);
		} else {
			pr_err("failed to get cpu clk for cpu%d\n", cpu);
		}

		ret = PTR_ERR(cpu_clk_fix480);
		return ret;
	}

	cpu_clk_parent = clk_get(cpu_dev, "cpu_clk");
	if (IS_ERR(cpu_clk_parent)) {
		if (PTR_ERR(cpu_clk_parent) == -EPROBE_DEFER) {
			pr_warn("cpu clk for cpu%d not ready, retry.\n", cpu);
		} else {
			pr_err("failed to get cpu parent clk for cpu%d\n", cpu);
		}

		ret = PTR_ERR(cpu_clk_parent);
		return ret;
	}


	/* Get OPP-sharing information from "operating-points-v2" bindings */
	ret = dev_pm_opp_of_get_sharing_cpus(cpu_dev, &info->cpus);
	if (ret) {
		pr_err("failed to get OPP-sharing information for cpu%d\n",
			   cpu);
		goto out_free_resources;
	}

	ret = dev_pm_opp_of_cpumask_add_table(&info->cpus);
	if (ret) {
		pr_warn("no OPP table for cpu%d\n", cpu);
		goto out_free_resources;
	}

	info->cpu_dev = cpu_dev;
	info->cpu_clk = cpu_clk;
	info->cpu_clk_parent = cpu_clk_parent;
	info->cpu_clk_fix480 = cpu_clk_fix480;

	return 0;

out_free_resources:
	if (!IS_ERR(cpu_clk)) {
		clk_put(cpu_clk);
	}
	if (!IS_ERR(cpu_clk_fix480)) {
		clk_put(cpu_clk_fix480);
	}
	if (!IS_ERR(cpu_clk_parent)) {
		clk_put(cpu_clk_parent);
	}
	return ret;
}

static void ntk_cpu_dvfs_info_release(struct ntk_cpu_dvfs_info *info)
{
	if (!IS_ERR(info->cpu_clk)) {
		clk_put(info->cpu_clk);
	}

	if (!IS_ERR(info->cpu_clk_fix480)) {
		clk_put(info->cpu_clk_fix480);
	}

	if (!IS_ERR(info->cpu_clk_parent)) {
		clk_put(info->cpu_clk_parent);
	}

	dev_pm_opp_of_cpumask_remove_table(&info->cpus);
}

static int ntk_cpufreq_init(struct cpufreq_policy *policy)
{
	struct ntk_cpu_dvfs_info *info;
	struct cpufreq_frequency_table *freq_table;
	int ret;

	info = ntk_cpu_dvfs_info_lookup(policy->cpu);

	if (!info) {
		pr_err("dvfs info for cpu%d is not initialized.\n",
			   policy->cpu);
		return -EINVAL;
	}

	ret = dev_pm_opp_init_cpufreq_table(info->cpu_dev, &freq_table);
	if (ret) {
		pr_err("failed to init cpufreq table for cpu%d: %d\n",
			   policy->cpu, ret);
		return ret;
	}

	cpumask_copy(policy->cpus, &info->cpus);
	policy->freq_table = freq_table;
	policy->driver_data = info;
	policy->clk = info->cpu_clk;
	return 0;
}

static int ntk_cpufreq_exit(struct cpufreq_policy *policy)
{
	struct ntk_cpu_dvfs_info *info = policy->driver_data;

	//cpufreq_cooling_unregister(info->cdev);
	dev_pm_opp_free_cpufreq_table(info->cpu_dev, &policy->freq_table);

	return 0;
}

static unsigned int ntk_cpufreq_get(unsigned int cpu)
{
	struct ntk_cpu_dvfs_info *info;
	struct cpufreq_policy *policy = cpufreq_cpu_get_raw(cpu);

	if (!policy || IS_ERR(policy->clk)) {
		pr_err("%s: No %s associated to cpu: %d\n",
			   __func__, policy ? "clk" : "policy", cpu);
		return 0;
	}

	info = ntk_cpu_dvfs_info_lookup(policy->cpu);

	if (!info) {
		pr_err("dvfs info for cpu%d is not initialized.\n",
			   policy->cpu);
		return -EINVAL;
	}

	return (clk_get_rate(clk_get_parent(info->cpu_clk_parent))) / 1000 * info->cpu_mult;
}

static struct cpufreq_driver ntk_cpufreq_driver = {
	.flags = CPUFREQ_STICKY | CPUFREQ_NEED_INITIAL_FREQ_CHECK |
	CPUFREQ_HAVE_GOVERNOR_PER_POLICY,
	.verify = cpufreq_generic_frequency_table_verify,
	.target_index = ntk_cpufreq_set_target,
	.get = ntk_cpufreq_get,
	.init = ntk_cpufreq_init,
	.exit = ntk_cpufreq_exit,
	.name = "ntk-cpufreq",
	.attr = cpufreq_generic_attr,
};

static const struct nvt_cpufreq_config ns02201_soc_config = {
	.cpu_lower_freq_max = 1400000,
	.cpu_normal_freq_max = 1500000,
	.cpu_over_clking_freq_max = 1600000,
	.cpu_mult = 32,
	.pll_name = "pll16",
};

static const struct nvt_cpufreq_config ns02302_soc_config = {
	.cpu_lower_freq_max = 1100000,
	.cpu_normal_freq_max = 1100000,
	.cpu_over_clking_freq_max = 1300000,
	.cpu_mult = 8,
	.pll_name = "pll8",
};

static const struct nvt_cpufreq_config na51102_soc_config = {
	.cpu_lower_freq_max = 1400000,
	.cpu_normal_freq_max = 1400000,
	.cpu_over_clking_freq_max = 1400000,
	.cpu_mult = 1,
	.pll_name = "pll16",
};

static const struct nvt_cpufreq_config na51089_soc_config = {
	.cpu_lower_freq_max = 960000,
	.cpu_normal_freq_max = 960000,
	.cpu_over_clking_freq_max = 960000,
	.cpu_mult = 1,
	.pll_name = "pll8",
};

static const struct nvt_cpufreq_config na02301_soc_config = {
	.cpu_lower_freq_max = 800000,
	.cpu_normal_freq_max = 800000,
	.cpu_over_clking_freq_max = 800000,
	.cpu_mult = 1,
	.pll_name = "pll8",
};



/* List of machines supported by this driver */
static const struct of_device_id ntk_cpufreq_machines[] = {
	{ .compatible = "novatek,ns02201", .data = &ns02201_soc_config },
	{ .compatible = "novatek,ns02302", .data = &ns02302_soc_config },
	{ .compatible = "novatek,na51102", .data = &na51102_soc_config },
	{ .compatible = "novatek,na51089", .data = &na51089_soc_config },
	{ .compatible = "novatek,ns02301", .data = &na02301_soc_config },
	{ }
};

static int ntk_cpufreq_probe(struct platform_device *pdev)
{
	struct ntk_cpu_dvfs_info *info, *tmp;
	int cpu, ret;
	const struct of_device_id *match;
	match = pdev->dev.platform_data;

	for_each_possible_cpu(cpu) {
		info = ntk_cpu_dvfs_info_lookup(cpu);

		if (info) {
			continue;
		}

		info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
		if (!info) {
			ret = -ENOMEM;
			goto release_dvfs_info_list;
		}

		info->soc_config = match->data;

		ret = ntk_cpu_dvfs_info_init(info, cpu);
		if (ret) {
			dev_err(&pdev->dev,
					"failed to initialize dvfs info for cpu%d\n",
					cpu);
			goto release_dvfs_info_list;
		}

		list_add(&info->list_head, &dvfs_info_list);
	}

	ret = cpufreq_register_driver(&ntk_cpufreq_driver);
	if (ret) {
		dev_err(&pdev->dev, "failed to register ntk cpufreq driver\n");
		goto release_dvfs_info_list;
	}

	return 0;

release_dvfs_info_list:
	list_for_each_entry_safe(info, tmp, &dvfs_info_list, list_head) {
		ntk_cpu_dvfs_info_release(info);
		list_del(&info->list_head);
	}

	return ret;
}


static struct platform_driver ntk_cpufreq_platdrv = {
	.driver = {
		.name   = "ntk-cpufreq",
		.of_match_table = ntk_cpufreq_machines,
	},
	.probe      = ntk_cpufreq_probe,
};


static int __init ntk_cpufreq_driver_init(void)
{
	struct device_node *np;
	const struct of_device_id *match;
	struct platform_device *pdev;
	int err;

	np = of_find_node_by_path("/");
	if (!np) {
		return -ENODEV;
	}

	match = of_match_node(ntk_cpufreq_machines, np);
	of_node_put(np);
	if (!match) {
		pr_debug("Machine is not compatible with ntk-cpufreq\n");
		return -ENODEV;
	}

	err = platform_driver_register(&ntk_cpufreq_platdrv);
	if (err) {
		return err;
	}

	/*
	 * Since there's no place to hold device registration code and no
	 * device tree based way to match cpufreq driver yet, both the driver
	 * and the device registration codes are put here to handle defer
	 * probing.
	 */
	pdev = platform_device_register_data(NULL, "ntk-cpufreq",
						     -1, match, sizeof(*match));

	if (IS_ERR(pdev)) {
		pr_err("failed to register ntk-cpufreq platform device\n");
		return PTR_ERR(pdev);
	}

	return 0;
}
device_initcall(ntk_cpufreq_driver_init);

MODULE_DESCRIPTION("Novatek CPUFreq driver");
MODULE_AUTHOR("NTK>");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(DRV_VERSION);
MODULE_DEVICE_TABLE(of, ntk_cpufreq_machines);
