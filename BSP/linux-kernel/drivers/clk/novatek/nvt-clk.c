/**
    NVT clock management module
    @file nvt-clk.c

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
#include <linux/syscore_ops.h>
#include "nvt-clk-provider.h"

static spinlock_t cg_lock;
int ep_idx = 0;
void __iomem *nvt_clk_ptr = NULL;
struct list_head nvt_clk_pll_list;
struct list_head nvt_clk_mux_list;
#define div_mask(width) ((1 << (width)) - 1)

static int nvt_suspend_mode(void)
{
	char *pstr = NULL;
	char *psep = NULL;
	char symbol;
	int ret = 0;
	static unsigned long suspend_mode = 0;

	pstr = strstr(saved_command_line, "suspend_mode=");
	if (pstr) {
		pstr += strlen("suspend_mode=");
	}
	while (pstr) {
		psep = strpbrk(pstr, " ");
		if (NULL == psep) {
			break;
		}

		symbol = *psep;
		*psep = '\0';
		suspend_mode = 0;
		ret = kstrtoul(pstr, 10, &suspend_mode);
		if (ret < 0) {
			pr_info("Error parsing power setting\n");
			return -1;
		}
		*psep = symbol;

		if (' ' == symbol) {
			break;
		}

		pstr = psep + 1;
	}

	return suspend_mode;
}

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
#ifdef DEBUG
		pr_info("%s: clock-output-names not find, use default name %s\n", __func__, np->name);
#endif
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

static int nvt_fix_factor_clk_init(struct device_node *node)
{
	struct clk *clk;
	const char *clk_name = node->name;
	u32 div, mult;

	if (of_property_read_u32(node, "clock-div", &div)) {
		pr_err("%s Fixed factor clock <%pOFn> must have a clock-div property\n",
				__func__, node);
		return -EIO;
	}

	if (of_property_read_u32(node, "clock-mult", &mult)) {
		pr_err("%s Fixed factor clock <%pOFn> must have a clock-mult property\n",
				__func__, node);
		return -EIO;
	}

	of_property_read_string(node, "clock-output-names", &clk_name);

	clk = clk_register_fixed_factor(NULL, clk_name, of_clk_get_parent_name(node, 0), 0, mult, div);
	if (IS_ERR(clk)) {
		pr_err("%s: failed to register fixed factor rate clock \"%s\"\n", __func__, clk_name);
		return -EINVAL;
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
#ifdef DEBUG
		pr_info("%s: clock-output-names not find, use default name %s\n", __func__, np->name);
#endif
		snprintf(pll_clk->name, CLK_NAME_STR_SIZE, "%s", np->name);
	} else {
		snprintf(pll_clk->name, CLK_NAME_STR_SIZE, "%s", clk_name);
	}

	if (of_property_read_u32(np, "pll_ratio", &value)) {
		pr_err("%s: pll_ratio not find!\n", pll_clk->name);
		ret = -EINVAL;
		goto pll_init_fail;
	} else {
		pll_clk->pll_ratio = value;
	}

	/* Avoid CPU or DRAM being disabled by user */
	if (pll_clk->pll_ratio > 0) {
		pll_clk->pll_fixed = false;
	} else {
		pll_clk->pll_fixed = true;
	}

	if (of_property_read_u32(np, "rate_reg_offset", &value)) {
		pr_err("%s: rate_reg_offset not find!\n", pll_clk->name);
		ret = -EINVAL;
		goto pll_init_fail;
	} else {
		pll_clk->rate_reg_offset = value;
	}


	if (of_property_read_u32(np, "gate_reg_offset", &value)) {
		pr_err("%s: gate_reg_offset not find!\n", pll_clk->name);
		ret = -EINVAL;
		goto pll_init_fail;
	} else {
		pll_clk->gate_reg_offset = value;
	}

	if (of_property_read_u32(np, "gate_bit_idx", &value)) {
		pr_err("%s: gate_bit_idx not find!\n", pll_clk->name);
		ret = -EINVAL;
		goto pll_init_fail;
	} else {
		pll_clk->gate_bit_idx = value;
	}

	if (of_property_read_u32(np, "rate_bit_idx", &value)) {
		pr_err("%s: rate_bit_idx not find!\n", pll_clk->name);
		ret = -EINVAL;
		goto pll_init_fail;
	} else {
		pll_clk->rate_bit_idx = value;
	}

	if (of_property_read_u32(np, "rate_bit_width", &value)) {
		pr_err("%s: rate_bit_width not find!\n", pll_clk->name);
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

	of_property_read_u32(np, "default_value", &value);

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
		pr_err("%s: Failed to register cg pll clk!\n", pll_clk->name);
		goto pll_init_fail;
	}

	ret = of_clk_add_provider(np, of_clk_src_simple_get, clk_get_sys(np->name, pll_clk->name));
	if (ret) {
		pr_err("%s Failed to add <%s>.\n", __func__, np->name);
		goto pll_init_fail;
	}

	list_add_tail(&pll_clk->list, &nvt_clk_pll_list);

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

	of_property_read_string_index(np, "clock-output-names", 0, &clk_name);
	if (NULL == clk_name) {
#ifdef DEBUG
		pr_info("%s: clock-output-names not find, use default name %s\n", __func__, np->name);
#endif
		snprintf(mux_clk->name, CLK_NAME_STR_SIZE, "%s", np->name);
	} else {
		snprintf(mux_clk->name, CLK_NAME_STR_SIZE, "%s", clk_name);
	}

	mux_clk->num_parents = of_clk_get_parent_count(np);
	if (mux_clk->num_parents == 0 || mux_clk->num_parents > PROG_SOURCE_MAX) {
		pr_err("%s: parent number(%d) is wrong!\n", mux_clk->name, mux_clk->num_parents);
		ret = -EINVAL;
		goto mux_init_fail;
	}
	of_clk_parent_fill(np, parent_names, mux_clk->num_parents);
	mux_clk->parent_names = parent_names;

	if (of_property_read_u32(np, "parent_idx", &value)) {
#ifdef DEBUG
		pr_info("%s: parent_idx not find, force set to 0\n", mux_clk->name);
#endif
		mux_clk->parent_idx = 0;
	} else {
		mux_clk->parent_idx = value;
	}

	if (of_property_read_u32(np, "current_rate", &value)) {
		mux_clk->current_rate = 0;
	} else {
		mux_clk->current_rate = value;
	}
	
	/* Some clock gate without multi clock source */
	if (of_property_read_u32(np, "mux_reg_offset", &value)) {
#ifdef DEBUG
		pr_info("%s: mux_reg_offset not find, force set to 0\n", mux_clk->name);
#endif
		mux_clk->mux_reg_offset = 0;
	} else {
		mux_clk->mux_reg_offset = value;
	}

	if (of_property_read_u32(np, "mux_bit_idx", &value)) {
#ifdef DEBUG
		pr_info("%s: mux_bit_idx not findm force set to 0\n", mux_clk->name);
#endif
		mux_clk->mux_bit_idx = 0;
	} else {
		mux_clk->mux_bit_idx = value;
	}

	if (of_property_read_u32(np, "mux_bit_width", &value)) {
#ifdef DEBUG
		pr_info("%s: mux_bit_width not find, force set to 0\n", mux_clk->name);
#endif
		mux_clk->mux_bit_width = 0;
	} else {
		mux_clk->mux_bit_width = value;
	}

	if (of_property_read_u32(np, "div_reg_offset", &value)) {
		mux_clk->div_reg_offset = 0;
	} else {
		mux_clk->div_reg_offset = value;
	}

	if (mux_clk->div_reg_offset) {
		if (of_property_read_u32(np, "div_bit_idx", &value)) {
			pr_err("%s: div_bit_idx not find!\n", mux_clk->name);
			ret = -EINVAL;
			goto mux_init_fail;
		} else {
			mux_clk->div_bit_idx = value;
		}

		if (of_property_read_u32(np, "div_bit_width", &value)) {
			pr_err("%s: div_bit_width not find!\n", mux_clk->name);
			ret = -EINVAL;
			goto mux_init_fail;
		} else {
			mux_clk->div_bit_width = value;
		}
	} else {
		mux_clk->div_bit_idx = 0;
		mux_clk->div_bit_width = 0;
	}

	if (of_property_read_u32(np, "gate_reg_offset", &value)) {
		mux_clk->gate_reg_offset = 0;
	} else {
		mux_clk->gate_reg_offset = value;
	}

	if (mux_clk->gate_reg_offset) {
		if (of_property_read_u32(np, "gate_bit_idx", &value)) {
			pr_err("%s: gate_bit_idx not find!\n", mux_clk->name);
			ret = -EINVAL;
			goto mux_init_fail;
		} else {
			mux_clk->gate_bit_idx = value;
		}

		if (of_property_read_u32(np, "do_enable", &value)) {
			mux_clk->do_enable = 0;
		} else {
			mux_clk->do_enable = value;
		}
	} else {
		mux_clk->gate_bit_idx = 0;
		mux_clk->do_enable = 0;
	}

	if (of_property_read_u32(np, "reset_reg_offset", &value)) {
		mux_clk->reset_reg_offset = 0;
	} else {
		mux_clk->reset_reg_offset = value;
	}

	if (mux_clk->reset_reg_offset) {
		if (of_property_read_u32(np, "reset_bit_idx", &value)) {
			pr_err("%s: reset_bit_idx not find!\n", mux_clk->name);
			ret = -EINVAL;
			goto mux_init_fail;
		} else {
			mux_clk->reset_bit_idx = value;
		}

		if (of_property_read_u32(np, "do_reset", &value)) {
			mux_clk->do_reset = false;
		} else {
			mux_clk->do_reset = value ? true : false;
		}
	} else {
		mux_clk->reset_bit_idx = 0;
		mux_clk->do_reset = false;
	}

	if (of_property_read_u32(np, "autogating_reg_offset", &value)) {
		mux_clk->autogating_reg_offset = 0;
	} else {
		mux_clk->autogating_reg_offset = value;
	}

	if (mux_clk->autogating_reg_offset) {
		if (of_property_read_u32(np, "autogating_bit_idx", &value)) {
			pr_err("%s: autogating_bit_idx not find!\n", mux_clk->name);
			ret = -EINVAL;
			goto mux_init_fail;
		} else {
			mux_clk->autogating_bit_idx = value;
		}

		if (of_property_read_u32(np, "do_autogating", &value)) {
			mux_clk->do_autogating = false;
		} else {
			mux_clk->do_autogating = value ? true : false;
		}
	} else {
		mux_clk->autogating_bit_idx = 0;
		mux_clk->do_autogating = false;
	}

	if (of_property_read_u32(np, "keep_rate", &value)) {
#ifdef DEBUG
		pr_info("%s: keep_rate not find, force set to 0\n", mux_clk->name);
#endif
		mux_clk->keep_rate = 0;
	} else {
		mux_clk->keep_rate = value;
	}

#ifndef CONFIG_NVT_FPGA_EMULATION
	if (of_property_read_u32(np, "max_rate", &value)) {
		/* Skip max rate check */
		mux_clk->max_rate = 0;
	} else {
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
	pr_info("keep_rate: %u\n", mux_clk->keep_rate);
#endif

	ret = nvt_composite_mux_clk_register(mux_clk);
	if (ret < 0) {
		pr_err("%s: Failed to register cg composite mux clk!\n", mux_clk->name);
		goto mux_init_fail;
	}

	ret = of_clk_add_provider(np, of_clk_src_simple_get, clk_get_sys(np->name, mux_clk->name));
	if (ret) {
		pr_err("%s Failed to add <%s>.\n", __func__, np->name);
		goto mux_init_fail;
	}

	list_add_tail(&mux_clk->list, &nvt_clk_mux_list);

	return 0;
mux_init_fail:
	if (mux_clk)
		kfree(mux_clk);
	return ret;
}

static int __maybe_unused nvt_clk_pm_suspend(void)
{
	struct nvt_pll_clk *pll, *_list;
	struct nvt_composite_mux_clk *mux, *_list_mux;

	list_for_each_entry_safe(mux, _list_mux, &nvt_clk_mux_list, list) {
		nvt_clkmux_pm_suspend(mux);
	}

	list_for_each_entry_safe(pll, _list, &nvt_clk_pll_list, list) {
		nvt_clkpll_pm_suspend(pll);
	}

	pr_info("%s: successfully\n", __func__);
	return 0;
}

static void __maybe_unused nvt_clk_pm_resume(void)
{
	struct nvt_pll_clk *pll, *_list;
	struct nvt_composite_mux_clk *mux, *_list_mux;

	list_for_each_entry_safe(pll, _list, &nvt_clk_pll_list, list) {
		nvt_clkpll_pm_resume(pll);
	}

	list_for_each_entry_safe(mux, _list_mux, &nvt_clk_mux_list, list) {
		nvt_clkmux_pm_resume(mux);
	}

	pr_info("%s: successfully\n", __func__);
}

static struct syscore_ops __maybe_unused nvt_pm_ops = {
	.suspend = nvt_clk_pm_suspend,
	.resume = nvt_clk_pm_resume,
};

static void clk_nvt_init_lowpower(struct device_node *np)
{
	struct device_node *nodeoffset, *subnode;
	const char *name = NULL;
	int addr_cells, size_cells, i;
	unsigned long long addr, size, value;

	for_each_of_allnodes_from(np, nodeoffset)
		if (of_node_name_eq(nodeoffset, "init") && of_node_get(nodeoffset))
			break;

	if (!nodeoffset) {
		pr_info("skip to run %s\n", __func__);
		return;
	}

	for_each_child_of_node(nodeoffset, subnode) {
		const __be32 *reg;
		int len;
		name = of_node_full_name(subnode);

		addr_cells = of_n_addr_cells(subnode);
		size_cells = of_n_size_cells(subnode);

		reg = of_get_property(subnode, "reg", &len);
		if (!reg) {
			pr_err("%s : can not find reg on device tree\n", name);
			if (nodeoffset)
				of_node_put(nodeoffset);
			return;
		}

		addr = of_read_number(reg, addr_cells);
		size = of_read_number(reg + addr_cells, size_cells - 1);
		value = of_read_number(reg + addr_cells + 1, size_cells - 1);

		for (i = 0; i < size / sizeof(unsigned int); i++) {
#ifdef DEBUG
			pr_info("%s: Set offset[0x%llx] to valude[0x%llx]\n", name, addr + (i * sizeof(unsigned int)), value);
#endif
			writel(value, nvt_clk_ptr + addr + (i * sizeof(unsigned int)));
#ifdef DEBUG
			pr_info("%s: Read Back offset[0x%llx] : valude[0x%x]\n\n", name, addr + (i * sizeof(unsigned int)), readl(nvt_clk_ptr + addr + (i * sizeof(unsigned int))));
#endif
		}
	}

	if (nodeoffset)
		of_node_put(nodeoffset);
	return;
}

static void __init clk_nvt_init(struct device_node *np)
{
	struct resource res;
	struct device_node *refnp;

	if (of_property_read_u32(np, "chip_id", &ep_idx)) {
		ep_idx = 0;
	}

	if (of_address_to_resource(np, 0, &res)) {
		pr_err("%s: Failed to get resource\n", __func__);
		return;
	}

	nvt_clk_ptr = ioremap(res.start, resource_size(&res));
	if (!nvt_clk_ptr) {
		pr_err("%s: Failed to remap cg register\n", __func__);
		return;
	}

	fastboot_determination();

	clk_nvt_init_lowpower(np);
	spin_lock_init(&cg_lock);

	INIT_LIST_HEAD(&nvt_clk_pll_list);
	INIT_LIST_HEAD(&nvt_clk_mux_list);

	for_each_compatible_node(refnp, NULL, "nvt,nvt-fixed-clock") {
		if (nvt_fix_clk_init(refnp))
			return;
	}

	for_each_compatible_node(refnp, NULL, "nvt,nvt-factor-clock") {
		if (nvt_fix_factor_clk_init(refnp))
			return;
	}

	for_each_compatible_node(refnp, NULL, "nvt,nvt-clk-pll") {
		if (nvt_pll_clk_init(refnp))
			return;
	}

	for_each_compatible_node(refnp, NULL, "nvt,nvt-clk-gate") {
		if (nvt_composite_mux_clk_init(refnp) == -EPERM)
			return;
	}

	for_each_compatible_node(refnp, NULL, "nvt,nvt-clk-mux") {
		if (nvt_composite_mux_clk_init(refnp) == -EPERM)
			return;
	}

	if (nvt_suspend_mode()) {
		register_syscore_ops(&nvt_pm_ops);
	}

	pr_info("%s: CG clock init done\n", __func__);

	return;
}
CLK_OF_DECLARE_DRIVER(nvt_clk, "nvt,core_clk", clk_nvt_init);

