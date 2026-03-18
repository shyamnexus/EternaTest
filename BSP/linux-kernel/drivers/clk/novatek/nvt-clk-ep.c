/**
    NVT clock management module for Novatek SoC
    @file nvt-clk-ep.c

    Copyright Novatek Microelectronics Corp. 2022. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <linux/spinlock.h>
#include <linux/of_address.h>
#include <plat/cg-reg.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <plat/top.h>
#include "nvt-clk-provider.h"


static spinlock_t cg_lock;
int ep_idx = 0;
void __iomem *nvt_clk_ptr = NULL;

static int nvt_fix_clk_init(struct device_node *np)
{
	unsigned int value;
	const char *clk_name = NULL;
	char name[CLK_NAME_STR_SIZE] = {0};
	u32 rate;
	struct clk *clk;
	int ret;

	if (of_property_read_u32(np, "chip_id", &value))
		value = 0;

	if (value != ep_idx) {
		return 0;
	}

	of_property_read_string_index(np, "clock-output-names", 0, &clk_name);
	if (NULL == clk_name) {
		pr_info("%s: clock-output-names not find, use default name %s\n", __func__, np->name);
		snprintf(name, CLK_NAME_STR_SIZE, "%s", np->name);
	} else {
		snprintf(name, CLK_NAME_STR_SIZE, "%s", clk_name);
	}

	if (of_property_read_u32(np, "clock-frequency", &rate)) {
		pr_err("%s: failed to get clock-frequency\n", __func__);
		return -EINVAL;
	}


	clk = clk_register_fixed_rate(NULL, name, NULL, 0, rate);
	if (IS_ERR(clk)) {
		pr_err("%s: failed to register fixed rate clock \"%s\"\n", __func__, name);
		return -EINVAL;
	}

	ret = clk_register_clkdev(clk, name, NULL);
	if (ret < 0) {
		pr_err("%s: failed to register lookup %s!\n", __func__, name);
		return ret;
	}

	ret = of_clk_add_provider(np, of_clk_src_simple_get, clk_get_sys(np->name, name));
	if (ret < 0) {
		pr_err("%s Failed to add <%s>.\n", __func__, np->name);
		return ret;
	}

	return 0;
}


static int nvt_pll_clk_init(struct device_node *np)
{
	int ret, is_ep_base;
	unsigned int value;
	struct nvt_pll_clk *pll_clk;
	const char *clk_name = NULL;
	const char *parent_names[PROG_SOURCE_MAX] = {0};

	if (of_property_read_u32(np, "chip_id", &value))
		is_ep_base = 0;
	else
		is_ep_base = value;

	if (is_ep_base != ep_idx) {
		return 0;
	}

	pll_clk = kzalloc(sizeof(struct nvt_pll_clk), GFP_KERNEL);
	if (!pll_clk) {
		pr_err("%s: failed to alloc pll_clk!\n", __func__);
		return -EPERM;
	}

	of_property_read_string_index(np, "clock-output-names", 0, &clk_name);
	if (NULL == clk_name) {
		pr_info("%s: clock-output-names not find, use default name %s\n", __func__, np->name);
		snprintf(pll_clk->name, CLK_NAME_STR_SIZE, "%s", np->name);
	} else {
		snprintf(pll_clk->name, CLK_NAME_STR_SIZE, "%s", clk_name);
	}

	if (of_property_read_u32(np, "pll_ratio", &value)) {
		pr_err("%s: pll_ratio not find!\n", __func__);
		ret = -EINVAL;
		goto pll_init_fail;
	} else {
		pll_clk->pll_ratio = value;
	}

	if (of_property_read_u32(np, "rate_reg_offset", &value)) {
		pr_err("%s: rate_reg_offset not find!\n", __func__);
		ret = -EINVAL;
		goto pll_init_fail;
	} else {
		pll_clk->rate_reg_offset = value;
	}


	if (of_property_read_u32(np, "gate_reg_offset", &value)) {
		pr_err("%s: gate_reg_offset not find!\n", __func__);
		ret = -EINVAL;
		goto pll_init_fail;
	} else {
		pll_clk->gate_reg_offset = value;
	}

	if (of_property_read_u32(np, "gate_bit_idx", &value)) {
		pr_err("%s: gate_bit_idx not find!\n", __func__);
		ret = -EINVAL;
		goto pll_init_fail;
	} else {
		pll_clk->gate_bit_idx = value;
	}

	if (of_property_read_u32(np, "rate_bit_idx", &value)) {
		pr_err("%s: rate_bit_idx not find!\n", __func__);
		ret = -EINVAL;
		goto pll_init_fail;
	} else {
		pll_clk->rate_bit_idx = value;
	}

	if (of_property_read_u32(np, "rate_bit_width", &value)) {
		pr_err("%s: rate_bit_width not find!\n", __func__);
		ret = -EINVAL;
		goto pll_init_fail;
	} else {
		pll_clk->rate_bit_width = value;
	}

	pll_clk->num_parents = 1;
	of_clk_parent_fill(np, parent_names, pll_clk->num_parents);
	pll_clk->parent_names = parent_names;

	pll_clk->status_reg_offset = pll_clk->gate_reg_offset+0x04;
	pll_clk->status_bit_idx = pll_clk->gate_bit_idx;

	if (of_property_read_u32(np, "default_value", &value)) {
		pr_err("%s: default_value not find!\n", __func__);
		ret = -EINVAL;
		goto pll_init_fail;
	} 

#ifdef DEBUG
	pr_info("==== %s: %s ====\n", __func__, np->name);
	pr_info("clock name: %s\n", pll_clk->name);
	pr_info("pll_ratio: %llu\n", pll_clk->pll_ratio);
	pr_info("rate_reg_offset: %lu\n", pll_clk->rate_reg_offset);
	pr_info("gate_reg_offset: %lu\n", pll_clk->gate_reg_offset);
	pr_info("gate_bit_idx: %u\n", pll_clk->gate_bit_idx);
	pr_info("rate_bit_idx: %u\n", pll_clk->rate_bit_idx);
	pr_info("rate_bit_width: %u\n", pll_clk->rate_bit_width);
	pr_info("is_ep_base: %u\n", is_ep_base);
	pr_info("default_value: %u\n", value);
#endif

	pll_clk->cg_base = nvt_clk_ptr;
	pll_clk->lock = &cg_lock;

	ret = nvt_pll_clk_register(pll_clk, value);
	if (ret < 0) {
		pr_err("%s: Failed to register cg pll clk!\n", __func__);
		goto pll_init_fail;
	}

	ret = of_clk_add_provider(np, of_clk_src_simple_get, clk_get_sys(np->name, pll_clk->name));
	if (ret) {
		pr_err("%s Failed to add <%s>.\n", __func__, np->name);
		goto pll_init_fail;
	}

	//coverity[leaked_storage]: clock will be used later
	return 0;

pll_init_fail:
	if (pll_clk)
		kfree(pll_clk);
	return ret;
}

static int nvt_composite_mux_clk_init(struct device_node *np)
{
	int ret, is_ep_base;
	struct nvt_composite_mux_clk *mux_clk;
	const char *parent_names[PROG_SOURCE_MAX] = {0};
	unsigned int value; 
	const char *clk_name = NULL;

	if (of_property_read_u32(np, "chip_id", &value))
		is_ep_base = 0;
	else
		is_ep_base = value;

	if (is_ep_base != ep_idx) {
		return 0;
	}

	mux_clk = kzalloc(sizeof(struct nvt_composite_mux_clk), GFP_KERNEL);
	if (!mux_clk) {
		pr_err("%s: failed to alloc mux_clk\n", __func__);
		ret = -EPERM;
		goto mux_init_fail;
	}

	mux_clk->num_parents = of_clk_get_parent_count(np);
	if (mux_clk->num_parents == 0 || mux_clk->num_parents > PROG_SOURCE_MAX) {
		pr_err("%s: parent number(%d) is wrong!\n", __func__, mux_clk->num_parents);
		ret = -EINVAL;
		goto mux_init_fail;
	}
	of_clk_parent_fill(np, parent_names, mux_clk->num_parents);
	mux_clk->parent_names = parent_names;

	of_property_read_string_index(np, "clock-output-names", 0, &clk_name);
	if (NULL == clk_name) {
		pr_info("%s: clock-output-names not find, use default name %s\n", __func__, np->name);
		snprintf(mux_clk->name, CLK_NAME_STR_SIZE, "%s", np->name);
	} else {
		snprintf(mux_clk->name, CLK_NAME_STR_SIZE, "%s", clk_name);
	}

	if (of_property_read_u32(np, "parent_idx", &value)) {
		pr_info("%s: parent_idx not find, force set to 0\n", __func__);
		mux_clk->parent_idx = 0;
	} else {
		mux_clk->parent_idx = value;
	}

	if (of_property_read_u32(np, "current_rate", &value)) {
		pr_err("%s: current_rate not find!\n", __func__);
		ret = -EINVAL;
		goto mux_init_fail;
	} else {
		mux_clk->current_rate = value;
	}

	/* Some clock gate without multi clock source */
	if (of_property_read_u32(np, "mux_reg_offset", &value)) {
		pr_info("%s: mux_reg_offset not find, force set to 0\n", __func__);
		mux_clk->mux_reg_offset = 0;
	} else {
		mux_clk->mux_reg_offset = value;
	}

	if (of_property_read_u32(np, "mux_bit_idx", &value)) {
		pr_info("%s: mux_bit_idx not findm force set to 0\n", __func__);
		mux_clk->mux_bit_idx = 0;
	} else {
		mux_clk->mux_bit_idx = value;
	}

	if (of_property_read_u32(np, "mux_bit_width", &value)) {
		pr_info("%s: mux_bit_width not find, force set to 0\n", __func__);
		mux_clk->mux_bit_width = 0;
	} else {
		mux_clk->mux_bit_width = value;
	}

	if (of_property_read_u32(np, "div_reg_offset", &value)) {
		pr_err("%s: div_reg_offset not find!\n", __func__);
		ret = -EINVAL;
		goto mux_init_fail;
	} else {
		mux_clk->div_reg_offset = value;
	}

	if (of_property_read_u32(np, "div_bit_idx", &value)) {
		pr_err("%s: div_bit_idx not find!\n", __func__);
		ret = -EINVAL;
		goto mux_init_fail;
	} else {
		mux_clk->div_bit_idx = value;
	}

	if (of_property_read_u32(np, "div_bit_width", &value)) {
		pr_err("%s: div_bit_width not find!\n", __func__);
		ret = -EINVAL;
		goto mux_init_fail;
	} else {
		mux_clk->div_bit_width = value;
	}

	if (of_property_read_u32(np, "gate_reg_offset", &value)) {
		pr_err("%s: gate_reg_offset not find!\n", __func__);
		ret = -EINVAL;
		goto mux_init_fail;
	} else {
		mux_clk->gate_reg_offset = value;
	}

	if (of_property_read_u32(np, "gate_bit_idx", &value)) {
		pr_err("%s: gate_bit_idx not find!\n", __func__);
		ret = -EINVAL;
		goto mux_init_fail;
	} else {
		mux_clk->gate_bit_idx = value;
	}

	if (of_property_read_u32(np, "do_enable", &value)) {
		pr_err("%s: do_enable not find!\n", __func__);
		ret = -EINVAL;
		goto mux_init_fail;
	} else {
		mux_clk->do_enable = value;
	}

	if (of_property_read_u32(np, "reset_reg_offset", &value)) {
		pr_err("%s: reset_reg_offset not find!\n", __func__);
		ret = -EINVAL;
		goto mux_init_fail;
	} else {
		mux_clk->reset_reg_offset = value;
	}

	if (of_property_read_u32(np, "reset_bit_idx", &value)) {
		pr_err("%s: reset_bit_idx not find!\n", __func__);
		ret = -EINVAL;
		goto mux_init_fail;
	} else {
		mux_clk->reset_bit_idx = value;
	}

	if (of_property_read_u32(np, "do_reset", &value)) {
		pr_err("%s: do_reset not find!\n", __func__);
		ret = -EINVAL;
		goto mux_init_fail;
	} else {
		mux_clk->do_reset = value ? true : false;
	}

	if (of_property_read_u32(np, "autogating_reg_offset", &value)) {
		pr_err("%s: autogating_reg_offset not find!\n", __func__);
		ret = -EINVAL;
		goto mux_init_fail;
	} else {
		mux_clk->autogating_reg_offset = value;
	}

	if (of_property_read_u32(np, "autogating_bit_idx", &value)) {
		pr_err("%s: autogating_bit_idx not find!\n", __func__);
		ret = -EINVAL;
		goto mux_init_fail;
	} else {
		mux_clk->autogating_bit_idx = value;
	}

	if (of_property_read_u32(np, "do_autogating", &value)) {
		pr_err("%s: do_autogating not find!\n", __func__);
		ret = -EINVAL;
		goto mux_init_fail;
	} else {
		mux_clk->do_autogating = value ? true : false;
	}

#ifndef CONFIG_NVT_FPGA_EMULATION
	if (of_property_read_u32(np, "max_rate", &value) == 0) {
		mux_clk->max_rate = value;
	}
#endif
	mux_clk->cg_base = nvt_clk_ptr;
	mux_clk->lock = &cg_lock;


#ifdef DEBUG
	pr_info("==== %s %s ====\n", __func__, np->name);
	pr_info("clock name: %s\n", mux_clk->name);
	pr_info("parent_names: %s\n", mux_clk->parent_names[0]);
	pr_info("current_rate: %ld\n", mux_clk->current_rate);
	pr_info("parent_idx: %d\n", mux_clk->parent_idx);
	pr_info("mux_reg_offset: %ld\n", mux_clk->mux_reg_offset);
	pr_info("mux_bit_idx: %d\n", mux_clk->mux_bit_idx);
	pr_info("mux_bit_width: %d\n", mux_clk->mux_bit_width);
	pr_info("div_reg_offset: %ld\n", mux_clk->div_reg_offset);
	pr_info("div_bit_idx: %d\n", mux_clk->div_bit_idx);
	pr_info("div_bit_width: %d\n", mux_clk->div_bit_width);
	pr_info("gate_reg_offset: %ld\n", mux_clk->gate_reg_offset);
	pr_info("gate_bit_idx: %d\n", mux_clk->gate_bit_idx);
	pr_info("do_enable: %d\n", mux_clk->do_enable);
	pr_info("reset_reg_offset: %ld\n", mux_clk->reset_reg_offset);
	pr_info("reset_bit_idx: %d\n", mux_clk->reset_bit_idx);
	pr_info("do_reset: %d\n", mux_clk->do_reset);
	pr_info("autogating_reg_offset: %ld\n", mux_clk->autogating_reg_offset);
	pr_info("autogating_bit_idx: %d\n", mux_clk->autogating_bit_idx);
	pr_info("do_autogating: %d\n", mux_clk->do_autogating);
	pr_info("num_parents: %d\n", mux_clk->num_parents);
	pr_info("max_rate: %ld\n", mux_clk->max_rate);
	pr_info("is_ep_base: %u\n", is_ep_base);
#endif
	ret = nvt_composite_mux_clk_register(mux_clk);
	if (ret < 0) {
		pr_err("%s: Failed to register cg composite mux clk!\n", __func__);
		goto mux_init_fail;
	}

	ret = of_clk_add_provider(np, of_clk_src_simple_get, clk_get_sys(np->name, mux_clk->name));
	if (ret) {
		pr_err("%s Failed to add <%s>.\n", __func__, np->name);
		goto mux_init_fail;
	}

	//coverity[leaked_storage]: clock will be used later
	return 0;
mux_init_fail:
	if (mux_clk)
		kfree(mux_clk);
	return ret;
}

static int clk_nvt_ep_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct resource res;
	struct device_node *refnp;
	int ret;

	if (of_property_read_u32(np, "chip_id", &ep_idx)) {
		ep_idx = 0;
	}

	if (of_address_to_resource(np, 0, &res)) {
		pr_err("%s: Failed to get resource\n", __func__);
		ret = -EINVAL;
		goto probe_fail;
	}

	if (CHIP_RC != ep_idx) {
		if (nvtpcie_conv_resource(ep_idx, &res)) {
			pr_err("%s: Failed to Convert resource\n", __func__);
			ret = -EINVAL;
			goto probe_fail;
		}
	}

	nvt_clk_ptr = ioremap(res.start, resource_size(&res));
	if (!nvt_clk_ptr) {
		pr_err("%s: Failed to remap cg register\n", __func__);
		ret = -EINVAL;
		goto probe_fail;
	}

	for_each_compatible_node(refnp, NULL, "nvt,nvt-fixed-clock") {
		ret = nvt_fix_clk_init(refnp);
		if (ret) 
			goto probe_fail;
	}

	for_each_compatible_node(refnp, NULL, "nvt,nvt-clk-pll") {
		ret = nvt_pll_clk_init(refnp);
		if (ret) 
			goto probe_fail;
	}

	for_each_compatible_node(refnp, NULL, "nvt,nvt-clk-gate") {
		ret = nvt_composite_mux_clk_init(refnp);
		if (ret) 
			goto probe_fail;
	}

	for_each_compatible_node(refnp, NULL, "nvt,nvt-clk-mux") {
		ret = nvt_composite_mux_clk_init(refnp);
		if (ret) 
			goto probe_fail;
	}

	spin_lock_init(&cg_lock);

	pr_info("%s: CG clock init done\n", __func__);

	return 0;

probe_fail:
	if (nvt_clk_ptr)
		iounmap(nvt_clk_ptr);
	return ret;
}

static const struct of_device_id of_match_clk_nvt_ep[] = {
	{
		.compatible = "nvt,ep_clk",
	}, {
	}
};

static struct platform_driver clk_nvt_ep_drv = {
	.probe = clk_nvt_ep_probe,
	.driver = {
		.name = "clk-nvt-ep",
		.owner = THIS_MODULE,
		.of_match_table = of_match_clk_nvt_ep,
	},
};

static int __init clk_nvt_ep_init(void)
{
	return platform_driver_register(&clk_nvt_ep_drv);
}

arch_initcall(clk_nvt_ep_init);

static void __exit clk_nvt_ep_exit(void)
{
	platform_driver_unregister(&clk_nvt_ep_drv);
}

module_exit(clk_nvt_ep_exit);

MODULE_AUTHOR("Novatek");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CLK EP driver for NOVATEK CG controller");
MODULE_ALIAS("nvt_clk_ep");
MODULE_VERSION("1.00.000");
