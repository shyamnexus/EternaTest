/**
    NVT clock management module
    @file nvt-clk.c

    Copyright Novatek Microelectronics Corp. 2020. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <linux/spinlock.h>
#include <linux/of_address.h>
#include <plat/cg-reg.h>
#include "../v2/nvt-im-clk-fr.h"
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <plat/top.h>
#include <linux/syscore_ops.h>

#define PROG_SOURCE_MAX 16 /* Maximum clock source(parent) */
static spinlock_t cg_lock;
int ep_idx = 0;
#define DEBUG 0

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


static void __init nvt_fix_clk_init(struct device_node *np)
{
	unsigned int value;
	const char *clk_name = NULL;
	char name[CLK_NAME_STR_SIZE] = {0};
	u32 rate;
	struct clk *clk;
	int ret;

	if (of_property_read_u32(np, "chip_id", &value)) {
		value = 0;
	}

	if (value != ep_idx) {
		return;
	}

	of_property_read_string_index(np, "clock-output-names", 0, &clk_name);
	if (NULL == clk_name) {
		pr_debug("%s: clock-output-names not find, use default name %s\n", __func__, np->name);
		snprintf(name, CLK_NAME_STR_SIZE, "%s", np->name);
	} else {
		snprintf(name, CLK_NAME_STR_SIZE, "%s", clk_name);
	}

	if (of_property_read_u32(np, "clock-frequency", &rate)) {
		return;
	}


	clk = clk_register_fixed_rate(NULL, name, NULL, 0, rate);
	if (IS_ERR(clk)) {
		pr_err("%s: failed to register fixed rate clock \"%s\"\n", __func__, name);
		return;
	}

	if (clk_register_clkdev(clk, name, NULL)) {
		pr_err("%s: failed to register lookup %s!\n", __func__, name);
		return;
	}

	ret = of_clk_add_provider(np, of_clk_src_simple_get, clk_get_sys(np->name, name));
	if (ret) {
		pr_err("%s Failed to add <%s>.\n", __func__, np->name);
	}

	return;
}
CLK_OF_DECLARE(nvt_clk_fixed_clock, "nvt,nvt-fixed-clock", nvt_fix_clk_init);


static void __init nvt_pll_clk_init(struct device_node *np)
{
	int ret;
	unsigned int value;
	struct nvt_pll_clk pll_clk;
	const char *clk_name = NULL;

	if (of_property_read_u32(np, "chip_id", &value)) {
		pll_clk.is_ep_base = 0;
	} else {
		pll_clk.is_ep_base = value;
	}

	if (pll_clk.is_ep_base != ep_idx) {
		return;
	}

	of_property_read_string_index(np, "clock-output-names", 0, &clk_name);
	if (NULL == clk_name) {
		pr_debug("%s: clock-output-names not find, use default name %s\n", __func__, np->name);
		snprintf(pll_clk.name, CLK_NAME_STR_SIZE, "%s", np->name);
	} else {
		snprintf(pll_clk.name, CLK_NAME_STR_SIZE, "%s", clk_name);
	}

	if (of_property_read_u32(np, "pll_ratio", &value)) {
		pr_err("%s: pll_ratio not find!\n", __func__);
		return;
	} else {
		pll_clk.pll_ratio = value;
	}

	if (pll_clk.pll_ratio > 0) {
		pll_clk.pll_fixed = false;
	} else {
		pll_clk.pll_fixed = true;
	}

	if (of_property_read_u32(np, "rate_reg_offset", &value)) {
		pr_err("%s: rate_reg_offset not find!\n", __func__);
		return;
	} else {
		pll_clk.rate_reg_offset = value;
	}


	if (of_property_read_u32(np, "gate_reg_offset", &value)) {
		pr_err("%s: gate_reg_offset not find!\n", __func__);
		return;
	} else {
		pll_clk.gate_reg_offset = value;
	}

	if (of_property_read_u32(np, "gate_bit_idx", &value)) {
		pr_err("%s: gate_bit_idx not find!\n", __func__);
		return;
	} else {
		pll_clk.gate_bit_idx = value;
	}

	if (of_property_read_u32(np, "rate_bit_idx", &value)) {
		pr_err("%s: rate_bit_idx not find!\n", __func__);
		return;
	} else {
		pll_clk.rate_bit_idx = value;
	}

	if (of_property_read_u32(np, "rate_bit_width", &value)) {
		pr_err("%s: rate_bit_width not find!\n", __func__);
		return;
	} else {
		pll_clk.rate_bit_width = value;
	}

	pll_clk.status_reg_offset = pll_clk.gate_reg_offset + 0x04;
	pll_clk.status_bit_idx = pll_clk.gate_bit_idx;

	of_property_read_u32(np, "default_value", &value);

#if DEBUG
	pr_info("==== %s: %s ====\n", __func__, np->name);
	pr_info("clock name: %s\n", pll_clk.name);
	pr_info("pll_ratio: %llu\n", pll_clk.pll_ratio);
	pr_info("rate_reg_offset: %lu\n", pll_clk.rate_reg_offset);
	pr_info("gate_reg_offset: %lu\n", pll_clk.gate_reg_offset);
	pr_info("gate_bit_idx: %u\n", pll_clk.gate_bit_idx);
	pr_info("rate_bit_idx: %u\n", pll_clk.rate_bit_idx);
	pr_info("rate_bit_width: %u\n", pll_clk.rate_bit_width);
	pr_info("is_ep_base: %lu\n", pll_clk.is_ep_base);
	pr_info("default_value: %u\n", value);
#endif

	ret = nvt_pll_clk_register(&pll_clk, 1, value, &cg_lock);
	if (ret < 0) {
		pr_err("%s: Failed to register cg pll clk!\n", __func__);
		return;
	}

	ret = of_clk_add_provider(np, of_clk_src_simple_get, clk_get_sys(np->name, pll_clk.name));
	if (ret) {
		pr_err("%s Failed to add <%s>.\n", __func__, np->name);
	}

	return;
}
CLK_OF_DECLARE(nvt_clk_pll, "nvt,nvt-clk-pll", nvt_pll_clk_init);


static void __init nvt_composite_gate_clk_init(struct device_node *np)
{
	int ret;
	struct nvt_composite_gate_clk gate_clk;
	unsigned int value;
	const char *clk_name = NULL;

	if (of_property_read_u32(np, "chip_id", &value)) {
		gate_clk.is_ep_base = 0;
	} else {
		gate_clk.is_ep_base = value;
	}

	if (gate_clk.is_ep_base != ep_idx) {
		return;
	}

	of_property_read_string_index(np, "clock-output-names", 0, &clk_name);
	if (NULL == clk_name) {
		pr_debug("%s: clock-output-names not find, use default name %s\n", __func__, np->name);
		snprintf(gate_clk.name, CLK_NAME_STR_SIZE, "%s", np->name);
	} else {
		snprintf(gate_clk.name, CLK_NAME_STR_SIZE, "%s", clk_name);
	}

	gate_clk.parent_name = of_clk_get_parent_name(np, 0);

	if (of_property_read_u32(np, "current_rate", &value)) {
		pr_err("%s: current_rate not find!\n", __func__);
		return;
	} else {
		gate_clk.current_rate = value;
	}

	if (of_property_read_u32(np, "div_reg_offset", &value)) {
		pr_err("%s: div_reg_offset not find!\n", __func__);
		return;
	} else {
		gate_clk.div_reg_offset = value;
	}

	if (of_property_read_u32(np, "div_bit_idx", &value)) {
		pr_err("%s: div_bit_idx not find!\n", __func__);
		return;
	} else {
		gate_clk.div_bit_idx = value;
	}

	if (of_property_read_u32(np, "div_bit_width", &value)) {
		pr_err("%s: div_bit_width not find!\n", __func__);
		return;
	} else {
		gate_clk.div_bit_width = value;
	}

	if (of_property_read_u32(np, "gate_reg_offset", &value)) {
		pr_err("%s: gate_reg_offset not find!\n", __func__);
		return;
	} else {
		gate_clk.gate_reg_offset = value;
	}

	if (of_property_read_u32(np, "gate_bit_idx", &value)) {
		pr_err("%s: gate_bit_idx not find!\n", __func__);
		return;
	} else {
		gate_clk.gate_bit_idx = value;
	}

	if (of_property_read_u32(np, "do_enable", &value)) {
		pr_err("%s: do_enable not find!\n", __func__);
		return;
	} else {
		gate_clk.do_enable = value ? true : false;
	}

	if (of_property_read_u32(np, "reset_reg_offset", &value)) {
		pr_err("%s: reset_reg_offset not find!\n", __func__);
		return;
	} else {
		gate_clk.reset_reg_offset = value;
	}

	if (of_property_read_u32(np, "reset_bit_idx", &value)) {
		pr_err("%s: reset_bit_idx not find!\n", __func__);
		return;
	} else {
		gate_clk.reset_bit_idx = value;
	}

	if (of_property_read_u32(np, "do_reset", &value)) {
		pr_err("%s: do_reset not find!\n", __func__);
		return;
	} else {
		gate_clk.do_reset = value ? true : false;
	}

	if (of_property_read_u32(np, "autogating_reg_offset", &value)) {
		pr_err("%s: autogating_reg_offset not find!\n", __func__);
		return;
	} else {
		gate_clk.autogating_reg_offset = value;
	}

	if (of_property_read_u32(np, "autogating_bit_idx", &value)) {
		pr_err("%s: autogating_bit_idx not find!\n", __func__);
		return;
	} else {
		gate_clk.autogating_bit_idx = value;
	}

	if (of_property_read_u32(np, "do_autogating", &value)) {
		pr_err("%s: do_autogating not find!\n", __func__);
		return;
	} else {
		gate_clk.do_autogating = value ? true : false;
	}

#ifndef CONFIG_NVT_FPGA_EMULATION
	if (of_property_read_u32(np, "max_rate", &value)) {
		/* Skip max rate check */
		gate_clk.max_rate = 0;
	} else {
		gate_clk.max_rate = value;
	}
#endif

	gate_clk.div_flags = CLK_DIVIDER_ALLOW_ZERO;

#if DEBUG
	pr_info("==== %s %s ====\n", __func__, np->name);
	pr_info("clock name: %s\n", gate_clk.name);
	pr_info("parent_name: %s\n", gate_clk.parent_name);
	pr_info("current_rate: %ld\n", gate_clk.current_rate);
	pr_info("div_reg_offset: %ld\n", gate_clk.div_reg_offset);
	pr_info("div_bit_idx: %d\n", gate_clk.div_bit_idx);
	pr_info("div_bit_width: %d\n", gate_clk.div_bit_width);
	pr_info("gate_reg_offset: %ld\n", gate_clk.gate_reg_offset);
	pr_info("gate_bit_idx: %d\n", gate_clk.gate_bit_idx);
	pr_info("do_enable: %d\n", gate_clk.do_enable);
	pr_info("reset_reg_offset: %ld\n", gate_clk.reset_reg_offset);
	pr_info("reset_bit_idx: %d\n", gate_clk.reset_bit_idx);
	pr_info("do_reset: %d\n", gate_clk.do_reset);
	pr_info("autogating_reg_offset: %ld\n", gate_clk.autogating_reg_offset);
	pr_info("autogating_bit_idx: %d\n", gate_clk.autogating_bit_idx);
	pr_info("do_autogating: %d\n", gate_clk.do_autogating);
	pr_info("is_ep_base: %ld\n", gate_clk.is_ep_base);
	pr_info("max_rate: %ld\n", gate_clk.max_rate);
#endif

	ret = nvt_composite_gate_clk_register(&gate_clk, 1, &cg_lock);
	if (ret < 0) {
		pr_err("%s: Failed to register cg composite gate clk!\n", __func__);
		return;
	}

	ret = of_clk_add_provider(np, of_clk_src_simple_get, clk_get_sys(np->name, gate_clk.name));
	if (ret) {
		pr_err("%s Failed to add <%s>.\n", __func__, np->name);
	}

	return;
}
CLK_OF_DECLARE(nvt_clk_gate, "nvt,nvt-clk-gate", nvt_composite_gate_clk_init);


static void __init nvt_composite_mux_clk_init(struct device_node *np)
{
	int ret;
	struct nvt_composite_mux_clk mux_clk;
	const char *parent_names[PROG_SOURCE_MAX] = {0};
	unsigned int value;
	const char *clk_name = NULL;

	mux_clk.keep_rate = 0;
	if (of_property_read_u32(np, "chip_id", &value)) {
		mux_clk.is_ep_base = 0;
	} else {
		mux_clk.is_ep_base = value;
	}

	if (mux_clk.is_ep_base != ep_idx) {
		return;
	}

	mux_clk.num_parents = of_clk_get_parent_count(np);
	if (mux_clk.num_parents == 0 || mux_clk.num_parents > PROG_SOURCE_MAX) {
		pr_err("%s: parent number(%d) is wrong!\n", __func__, mux_clk.num_parents);
		return;
	}
	of_clk_parent_fill(np, parent_names, mux_clk.num_parents);
	mux_clk.parent_names = parent_names;

	of_property_read_string_index(np, "clock-output-names", 0, &clk_name);
	if (NULL == clk_name) {
		pr_debug("%s: clock-output-names not find, use default name %s\n", __func__, np->name);
		snprintf(mux_clk.name, CLK_NAME_STR_SIZE, "%s", np->name);
	} else {
		snprintf(mux_clk.name, CLK_NAME_STR_SIZE, "%s", clk_name);
	}

	if (of_property_read_u32(np, "parent_idx", &value)) {
		pr_err("%s: parent_idx not find!\n", __func__);
		return;
	} else {
		mux_clk.parent_idx = value;
	}

	if (of_property_read_u32(np, "current_rate", &value)) {
		pr_err("%s: current_rate not find!\n", __func__);
		return;
	} else {
		mux_clk.current_rate = value;
	}

	if (of_property_read_u32(np, "keep_rate", &value)) {
		pr_debug("%s: keep_rate not find!\n", __func__);
		// return;   keep_rate is optional, not necessary
	} else {
		mux_clk.keep_rate = value;
	}

	if (of_property_read_u32(np, "mux_reg_offset", &value)) {
		pr_err("%s: mux_reg_offset not find!\n", __func__);
		return;
	} else {
		mux_clk.mux_reg_offset = value;
	}

	if (of_property_read_u32(np, "mux_bit_idx", &value)) {
		pr_err("%s: mux_bit_idx not find!\n", __func__);
		return;
	} else {
		mux_clk.mux_bit_idx = value;
	}

	if (of_property_read_u32(np, "mux_bit_width", &value)) {
		pr_err("%s: mux_bit_width not find!\n", __func__);
		return;
	} else {
		mux_clk.mux_bit_width = value;
	}

	if (of_property_read_u32(np, "div_reg_offset", &value)) {
		pr_err("%s: div_reg_offset not find!\n", __func__);
		return;
	} else {
		mux_clk.div_reg_offset = value;
	}

	if (of_property_read_u32(np, "div_bit_idx", &value)) {
		pr_err("%s: div_bit_idx not find!\n", __func__);
		return;
	} else {
		mux_clk.div_bit_idx = value;
	}

	if (of_property_read_u32(np, "div_bit_width", &value)) {
		pr_err("%s: div_bit_width not find!\n", __func__);
		return;
	} else {
		mux_clk.div_bit_width = value;
	}

	if (of_property_read_u32(np, "gate_reg_offset", &value)) {
		pr_err("%s: gate_reg_offset not find!\n", __func__);
		return;
	} else {
		mux_clk.gate_reg_offset = value;
	}

	if (of_property_read_u32(np, "gate_bit_idx", &value)) {
		pr_err("%s: gate_bit_idx not find!\n", __func__);
		return;
	} else {
		mux_clk.gate_bit_idx = value;
	}

	if (of_property_read_u32(np, "do_enable", &value)) {
		pr_err("%s: do_enable not find!\n", __func__);
		return;
	} else {
		mux_clk.do_enable = value ? true : false;
	}

	if (of_property_read_u32(np, "reset_reg_offset", &value)) {
		pr_err("%s: reset_reg_offset not find!\n", __func__);
		return;
	} else {
		mux_clk.reset_reg_offset = value;
	}

	if (of_property_read_u32(np, "reset_bit_idx", &value)) {
		pr_err("%s: reset_bit_idx not find!\n", __func__);
		return;
	} else {
		mux_clk.reset_bit_idx = value;
	}

	if (of_property_read_u32(np, "do_reset", &value)) {
		pr_err("%s: do_reset not find!\n", __func__);
		return;
	} else {
		mux_clk.do_reset = value ? true : false;
	}

	if (of_property_read_u32(np, "autogating_reg_offset", &value)) {
		pr_err("%s: autogating_reg_offset not find!\n", __func__);
		return;
	} else {
		mux_clk.autogating_reg_offset = value;
	}

	if (of_property_read_u32(np, "autogating_bit_idx", &value)) {
		pr_err("%s: autogating_bit_idx not find!\n", __func__);
		return;
	} else {
		mux_clk.autogating_bit_idx = value;
	}

	if (of_property_read_u32(np, "do_autogating", &value)) {
		pr_err("%s: do_autogating not find!\n", __func__);
		return;
	} else {
		mux_clk.do_autogating = value ? true : false;
	}

#ifndef CONFIG_NVT_FPGA_EMULATION
	if (of_property_read_u32(np, "max_rate", &value)) {
		/* Skip max rate check */
		mux_clk.max_rate = 0;
	} else {
		mux_clk.max_rate = value;
	}

#endif
	mux_clk.mux_flags = CLK_SET_RATE_PARENT | CLK_SET_RATE_NO_REPARENT | CLK_GET_RATE_NOCACHE;

#if DEBUG
	pr_info("==== %s %s ====\n", __func__, np->name);
	pr_info("clock name: %s\n", mux_clk.name);
	pr_info("parent_names: %s\n", mux_clk.parent_names[0]);
	pr_info("current_rate: %ld\n", mux_clk.current_rate);
	pr_info("parent_idx: %d\n", mux_clk.parent_idx);
	pr_info("mux_reg_offset: %ld\n", mux_clk.mux_reg_offset);
	pr_info("mux_bit_idx: %d\n", mux_clk.mux_bit_idx);
	pr_info("mux_bit_width: %d\n", mux_clk.mux_bit_width);
	pr_info("div_reg_offset: %ld\n", mux_clk.div_reg_offset);
	pr_info("div_bit_idx: %d\n", mux_clk.div_bit_idx);
	pr_info("div_bit_width: %d\n", mux_clk.div_bit_width);
	pr_info("gate_reg_offset: %ld\n", mux_clk.gate_reg_offset);
	pr_info("gate_bit_idx: %d\n", mux_clk.gate_bit_idx);
	pr_info("do_enable: %d\n", mux_clk.do_enable);
	pr_info("reset_reg_offset: %ld\n", mux_clk.reset_reg_offset);
	pr_info("reset_bit_idx: %d\n", mux_clk.reset_bit_idx);
	pr_info("do_reset: %d\n", mux_clk.do_reset);
	pr_info("autogating_reg_offset: %ld\n", mux_clk.autogating_reg_offset);
	pr_info("autogating_bit_idx: %d\n", mux_clk.autogating_bit_idx);
	pr_info("do_autogating: %d\n", mux_clk.do_autogating);
	pr_info("num_parents: %d\n", mux_clk.num_parents);
	pr_info("is_ep_base: %ld\n", mux_clk.is_ep_base);
	pr_info("max_rate: %ld\n", mux_clk.max_rate);
#endif

	ret = nvt_composite_mux_clk_register(&mux_clk, 1, &cg_lock);
	if (ret < 0) {
		pr_err("%s: Failed to register cg composite mux clk!\n", __func__);
		return;
	}

	ret = of_clk_add_provider(np, of_clk_src_simple_get, clk_get_sys(np->name, mux_clk.name));
	if (ret) {
		pr_err("%s Failed to add <%s>.\n", __func__, np->name);
	}

	return;
}
CLK_OF_DECLARE(nvt_clk_mux, "nvt,nvt-clk-mux", nvt_composite_mux_clk_init);

static void fastboot_determination(void)
{
	u32 m_fastboot = 0x0;
	struct device_node *of_node = of_find_node_by_path("/fastboot");

	if (of_node) {
		of_property_read_u32(of_node, "enable", &m_fastboot);
	}

	nvt_fastboot_detect(m_fastboot);
}

static struct syscore_ops __maybe_unused nvt_pm_ops = {
	.suspend = nvt_clk_pm_suspend,
	.resume = nvt_clk_pm_resume,
};

static void __init clk_nvt_init(struct device_node *np)
{
	struct resource res;
	void __iomem *remap_base = NULL;

	if (of_property_read_u32(np, "chip_id", &ep_idx)) {
		ep_idx = 0;
	}

	if (CHIP_RC != ep_idx) {
		pr_err("%s: Chip id is wrong(%d)\n", __func__, ep_idx);
		return;
	}

	fastboot_determination();

	spin_lock_init(&cg_lock);

	if (of_address_to_resource(np, 0, &res)) {
		pr_err("%s: Failed to get resource of cg_ep\n", __func__);
		return;
	} else {
		remap_base = ioremap(res.start, resource_size(&res));
		if (NULL == remap_base) {
			pr_err("%s: Failed to remap cg register\n", __func__);
			return;
		} else {
			nvt_cg_base_remap(remap_base);
		}
	}

	if (nvt_suspend_mode()) {
		register_syscore_ops(&nvt_pm_ops);
	}

	pr_info("%s: CG clock init done\n", __func__);

	return;
}
CLK_OF_DECLARE(nvt_clk, "nvt,core_clk", clk_nvt_init);
