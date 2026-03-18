/**
    NVT clock management module for NA51090 SoC
    @file na51090-clk-ep-common.c

    Copyright Novatek Microelectronics Corp. 2024. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <linux/spinlock.h>
#include <linux/of_address.h>
#include <plat/cg-reg.h>
#include "../v2/nvt-im-clk-fr.h"
#include "na51090-maxfreq.h"
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <plat/top.h>

#define PROG_SOURCE_MAX 16 /* Maximum clock source(parent) */
static spinlock_t cg_lock;
int ep_idx = 0;

#define DEBUG 0

static int nvt_fix_clk_init(struct device_node *node)
{
	unsigned int value;
	const char *clk_name = NULL;
	char name[CLK_NAME_STR_SIZE] = {0};
	u32 rate;
	struct clk *clk;
	int ret;

	if (of_property_read_u32(node, "chip_id", &value) < 0) {
		pr_err("%s: chip_id not find\n", __func__);
		return -EINVAL;
	}

	if (value != ep_idx) {
		return 0;
	}

	of_property_read_string_index(node, "clock-output-names", 0, &clk_name);
	if (NULL == clk_name) {
		snprintf(name, CLK_NAME_STR_SIZE, "%s", node->name);
	} else {
		snprintf(name, CLK_NAME_STR_SIZE, "%s", clk_name);
	}

	if (of_property_read_u32(node, "clock-frequency", &rate))
		return -EIO;


	clk = clk_register_fixed_rate(NULL, name, NULL, 0, rate);
	if (IS_ERR(clk)) {
		pr_err("%s: failed to register fixed rate clock \"%s\"\n", __func__, name);
		return -EPERM;
	}

	if (clk_register_clkdev(clk, name, NULL)) {
		pr_err("%s: failed to register lookup %s!\n", __func__, name);
		return -EPERM;
	}

	ret = of_clk_add_provider(node, of_clk_src_simple_get, clk_get_sys(node->name, name));
	if (ret) {
		pr_err("%s Failed to add <%s>.\n", __func__, node->name);
	}

	return 0;
}

static int nvt_pll_clk_init(struct device_node *node)
{
	int ret;
	unsigned int value;
	struct nvt_pll_clk pll_clk;
	const char *clk_name = NULL;

	if (of_property_read_u32(node, "chip_id", &value) < 0) {
		pr_err("%s: chip_id not find\n", __func__);
		return -EINVAL;
	}
	pll_clk.is_ep_base = value;

	if (value != ep_idx) {
		return 0;
	}

	of_property_read_string_index(node, "clock-output-names", 0, &clk_name);
	if (NULL == clk_name) {
		snprintf(pll_clk.name, CLK_NAME_STR_SIZE, "%s", node->name);
	} else {
		snprintf(pll_clk.name, CLK_NAME_STR_SIZE, "%s", clk_name);
	}

	if (of_property_read_u32(node, "pll_ratio", &value)) {
		pr_err("%s: pll_ratio not find!\n", __func__);
		return -EPERM;
	} else {
		pll_clk.pll_ratio = value;
	}

	if (of_property_read_u32(node, "rate_reg_offset", &value)) {
		pr_err("%s: rate_reg_offset not find!\n", __func__);
		return -EPERM;
	} else {
		pll_clk.rate_reg_offset = value;
	}


	if (of_property_read_u32(node, "gate_reg_offset", &value)) {
		pr_err("%s: gate_reg_offset not find!\n", __func__);
		return -EPERM;
	} else {
		pll_clk.gate_reg_offset = value;
	}

	if (of_property_read_u32(node, "gate_bit_idx", &value)) {
		pr_err("%s: gate_bit_idx not find!\n", __func__);
		return -EPERM;
	} else {
		pll_clk.gate_bit_idx = value;
	}

	if (of_property_read_u32(node, "rate_bit_idx", &value)) {
		pr_err("%s: rate_bit_idx not find!\n", __func__);
		return -EPERM;
	} else {
		pll_clk.rate_bit_idx = value;
	}

	if (of_property_read_u32(node, "rate_bit_width", &value)) {
		pr_err("%s: rate_bit_width not find!\n", __func__);
		return -EPERM;
	} else {
		pll_clk.rate_bit_width = value;
	}

	pll_clk.status_reg_offset = pll_clk.gate_reg_offset+0x04;
	pll_clk.status_bit_idx = pll_clk.gate_bit_idx;

	of_property_read_u32(node, "default_value", &value);

#if DEBUG
	pr_info("==== %s: %s ====\n", __func__, node->name);
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
		pr_err("%s: Failed to register cg_ep pll clk!\n", __func__);
		return ret;
	}

	ret = of_clk_add_provider(node, of_clk_src_simple_get, clk_get_sys(node->name, pll_clk.name));
	if (ret) {
		pr_err("%s Failed to add <%s>.\n", __func__, node->name);
	}

	return ret;
}

static int nvt_composite_gate_clk_init(struct device_node *node)
{
	int ret;
	struct nvt_composite_gate_clk gate_clk;
	unsigned int value; 
	const char *clk_name = NULL;

	if (of_property_read_u32(node, "chip_id", &value) < 0) {
		pr_err("%s: chip_id not find\n", __func__);
		return -EINVAL;
	}
	gate_clk.is_ep_base = value;

	if (value != ep_idx) {
		return 0;
	}

	of_property_read_string_index(node, "clock-output-names", 0, &clk_name);
	if (NULL == clk_name) {
		snprintf(gate_clk.name, CLK_NAME_STR_SIZE, "%s", node->name);
	} else {
		snprintf(gate_clk.name, CLK_NAME_STR_SIZE, "%s", clk_name);
	}

	gate_clk.parent_name = of_clk_get_parent_name(node, 0);

	if (of_property_read_u32(node, "current_rate", &value)) {
		pr_err("%s: current_rate not find!\n", __func__);
		return -EPERM;
	} else {
		gate_clk.current_rate = value;
	}

	if (of_property_read_u32(node, "div_reg_offset", &value)) {
		pr_err("%s: div_reg_offset not find!\n", __func__);
		return -EPERM;
	} else {
		gate_clk.div_reg_offset = value;
	}

	if (of_property_read_u32(node, "div_bit_idx", &value)) {
		pr_err("%s: div_bit_idx not find!\n", __func__);
		return -EPERM;
	} else {
		gate_clk.div_bit_idx = value;
	}

	if (of_property_read_u32(node, "div_bit_width", &value)) {
		pr_err("%s: div_bit_width not find!\n", __func__);
		return -EPERM;
	} else {
		gate_clk.div_bit_width = value;
	}

	if (of_property_read_u32(node, "gate_reg_offset", &value)) {
		pr_err("%s: gate_reg_offset not find!\n", __func__);
		return -EPERM;
	} else {
		gate_clk.gate_reg_offset = value;
	}

	if (of_property_read_u32(node, "gate_bit_idx", &value)) {
		pr_err("%s: gate_bit_idx not find!\n", __func__);
		return -EPERM;
	} else {
		gate_clk.gate_bit_idx = value;
	}

	if (of_property_read_u32(node, "do_enable", &value)) {
		pr_err("%s: do_enable not find!\n", __func__);
		return -EPERM;
	} else {
		gate_clk.do_enable = value ? true : false;
	}

	if (of_property_read_u32(node, "reset_reg_offset", &value)) {
		pr_err("%s: reset_reg_offset not find!\n", __func__);
		return -EPERM;
	} else {
		gate_clk.reset_reg_offset = value;
	}

	if (of_property_read_u32(node, "reset_bit_idx", &value)) {
		pr_err("%s: reset_bit_idx not find!\n", __func__);
		return -EPERM;
	} else {
		gate_clk.reset_bit_idx = value;
	}

	if (of_property_read_u32(node, "do_reset", &value)) {
		pr_err("%s: do_reset not find!\n", __func__);
		return -EPERM;
	} else {
		gate_clk.do_reset = value ? true : false;
	}

	if (of_property_read_u32(node, "autogating_reg_offset", &value)) {
		pr_err("%s: autogating_reg_offset not find!\n", __func__);
		return -EPERM;
	} else {
		gate_clk.autogating_reg_offset = value;
	}

	if (of_property_read_u32(node, "autogating_bit_idx", &value)) {
		pr_err("%s: autogating_bit_idx not find!\n", __func__);
		return -EPERM;
	} else {
		gate_clk.autogating_bit_idx = value;
	}

	if (of_property_read_u32(node, "do_autogating", &value)) {
		pr_err("%s: do_autogating not find!\n", __func__);
		return -EPERM;
	} else {
		gate_clk.do_autogating = value ? true : false;
	}

#ifndef CONFIG_NVT_FPGA_EMULATION
	if (of_property_read_u32(node, "max_rate", &value) == 0) {
		gate_clk.max_rate = value;
	}
#endif

	gate_clk.div_flags = CLK_DIVIDER_ALLOW_ZERO;

#if DEBUG
	pr_info("==== %s %s ====\n", __func__, node->name);
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
		pr_err("%s: Failed to register cg_ep composite gate clk!\n", __func__);
		return ret;
	}

	ret = of_clk_add_provider(node, of_clk_src_simple_get, clk_get_sys(node->name, gate_clk.name));
	if (ret) {
		pr_err("%s Failed to add <%s>.\n", __func__, node->name);
	}

	return ret;
}

static int nvt_composite_mux_clk_init(struct device_node *node)
{
	int ret;
	struct nvt_composite_mux_clk mux_clk;
	const char *parent_names[PROG_SOURCE_MAX] = {0};
	unsigned int value; 
	const char *clk_name = NULL;

	if (of_property_read_u32(node, "chip_id", &value)) {
		pr_err("%s: chip_id not find\n", __func__);
		return -EINVAL;
	}
	mux_clk.is_ep_base = value;

	if (value != ep_idx) {
		return 0;
	}

	mux_clk.num_parents = of_clk_get_parent_count(node);
	if (mux_clk.num_parents == 0 || mux_clk.num_parents > PROG_SOURCE_MAX) {
		pr_err("%s: parent number(%d) is wrong!\n", __func__, mux_clk.num_parents);
		return -EINVAL;
	}
	of_clk_parent_fill(node, parent_names, mux_clk.num_parents);
	mux_clk.parent_names = parent_names;

	of_property_read_string_index(node, "clock-output-names", 0, &clk_name);
	if (NULL == clk_name) {
		snprintf(mux_clk.name, CLK_NAME_STR_SIZE, "%s", node->name);
	} else {
		snprintf(mux_clk.name, CLK_NAME_STR_SIZE, "%s", clk_name);
	}

	if (of_property_read_u32(node, "parent_idx", &value)) {
		pr_err("%s: parent_idx not find!\n", __func__);
		return -EPERM;
	} else {
		mux_clk.parent_idx = value;
	}

	if (of_property_read_u32(node, "current_rate", &value)) {
		pr_err("%s: current_rate not find!\n", __func__);
		return -EPERM;
	} else {
		mux_clk.current_rate = value;
	}

	if (of_property_read_u32(node, "mux_reg_offset", &value)) {
		pr_err("%s: mux_reg_offset not find!\n", __func__);
		return -EPERM;
	} else {
		mux_clk.mux_reg_offset = value;
	}

	if (of_property_read_u32(node, "mux_bit_idx", &value)) {
		pr_err("%s: mux_bit_idx not find!\n", __func__);
		return -EPERM;
	} else {
		mux_clk.mux_bit_idx = value;
	}

	if (of_property_read_u32(node, "mux_bit_width", &value)) {
		pr_err("%s: mux_bit_width not find!\n", __func__);
		return -EPERM;
	} else {
		mux_clk.mux_bit_width = value;
	}

	if (of_property_read_u32(node, "div_reg_offset", &value)) {
		pr_err("%s: div_reg_offset not find!\n", __func__);
		return -EPERM;
	} else {
		mux_clk.div_reg_offset = value;
	}

	if (of_property_read_u32(node, "div_bit_idx", &value)) {
		pr_err("%s: div_bit_idx not find!\n", __func__);
		return -EPERM;
	} else {
		mux_clk.div_bit_idx = value;
	}

	if (of_property_read_u32(node, "div_bit_width", &value)) {
		pr_err("%s: div_bit_width not find!\n", __func__);
		return -EPERM;
	} else {
		mux_clk.div_bit_width = value;
	}

	if (of_property_read_u32(node, "gate_reg_offset", &value)) {
		pr_err("%s: gate_reg_offset not find!\n", __func__);
		return -EPERM;
	} else {
		mux_clk.gate_reg_offset = value;
	}

	if (of_property_read_u32(node, "gate_bit_idx", &value)) {
		pr_err("%s: gate_bit_idx not find!\n", __func__);
		return -EPERM;
	} else {
		mux_clk.gate_bit_idx = value;
	}

	if (of_property_read_u32(node, "do_enable", &value)) {
		pr_err("%s: do_enable not find!\n", __func__);
		return -EPERM;
	} else {
		mux_clk.do_enable = value ? true : false;
	}

	if (of_property_read_u32(node, "reset_reg_offset", &value)) {
		pr_err("%s: reset_reg_offset not find!\n", __func__);
		return -EPERM;
	} else {
		mux_clk.reset_reg_offset = value;
	}

	if (of_property_read_u32(node, "reset_bit_idx", &value)) {
		pr_err("%s: reset_bit_idx not find!\n", __func__);
		return -EPERM;
	} else {
		mux_clk.reset_bit_idx = value;
	}

	if (of_property_read_u32(node, "do_reset", &value)) {
		pr_err("%s: do_reset not find!\n", __func__);
		return -EPERM;
	} else {
		mux_clk.do_reset = value ? true : false;
	}

	if (of_property_read_u32(node, "autogating_reg_offset", &value)) {
		pr_err("%s: autogating_reg_offset not find!\n", __func__);
		return -EPERM;
	} else {
		mux_clk.autogating_reg_offset = value;
	}

	if (of_property_read_u32(node, "autogating_bit_idx", &value)) {
		pr_err("%s: autogating_bit_idx not find!\n", __func__);
		return -EPERM;
	} else {
		mux_clk.autogating_bit_idx = value;
	}

	if (of_property_read_u32(node, "do_autogating", &value)) {
		pr_err("%s: do_autogating not find!\n", __func__);
		return -EPERM;
	} else {
		mux_clk.do_autogating = value ? true : false;
	}

#ifndef CONFIG_NVT_FPGA_EMULATION
	if (of_property_read_u32(node, "max_rate", &value) == 0) {
		mux_clk.max_rate = value;
	}
#endif

	mux_clk.mux_flags = CLK_SET_RATE_PARENT | CLK_SET_RATE_NO_REPARENT | CLK_GET_RATE_NOCACHE;

#if DEBUG
	pr_info("==== %s %s ====\n", __func__, node->name);
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
		pr_err("%s: Failed to register cg_ep composite mux clk!\n", __func__);
		return ret;
	}

	ret = of_clk_add_provider(node, of_clk_src_simple_get, clk_get_sys(node->name, mux_clk.name));
	if (ret) {
		pr_err("%s Failed to add <%s>.\n", __func__, node->name);
	}

	return ret;
}

static const struct of_device_id nvt_clk_match[] = {
	{
		.compatible = "nvt,nvt-fixed-clock",
		.data = nvt_fix_clk_init,
	}, {
		.compatible = "nvt,ep-clk-pll",
		.data = nvt_pll_clk_init,
	}, {
		.compatible = "nvt,ep-clk-gate",
		.data = nvt_composite_gate_clk_init,
	}, {
		.compatible = "nvt,ep-clk-mux",
		.data = nvt_composite_mux_clk_init,
	}, {

	}
};

static void ep_ext_clkout_src_set(int ep_num)
{
	struct clk* extclk;
	struct clk *source_clk;
	u32 m_ext_clk = 0x0;
	struct device_node* of_node;
	char ext_name[4][16];
	char node_name[16];
	char clk_name[16];

	// ep0
	sprintf(node_name, "%s%d", "extclk_src_ep", ep_num);
	sprintf(ext_name[0], "%s%d", "ext_ep", ep_num);
	sprintf(ext_name[1], "%s%d", "ext2_ep", ep_num);
	sprintf(ext_name[2], "%s%d", "ext3_ep", ep_num);
	sprintf(ext_name[3], "%s%d", "ext4_ep", ep_num);
	of_node = of_find_node_by_name(NULL, node_name);
	
	pr_err("*** %s Get %s\n", __func__, node_name);

	if (of_node) {
		if (of_property_read_u32(of_node, "extclk_from", &m_ext_clk) == 0) {
			
			sprintf(clk_name, "%s%d", "ext_from_ep", ep_num);
			extclk = clk_get(NULL, clk_name);
			if (!IS_ERR(extclk)) {
				source_clk = clk_get(NULL, ext_name[m_ext_clk]);
				if (IS_ERR(source_clk)) {
					pr_err("*** %s Get ext source error\n", __func__);
				}
				clk_set_parent(extclk, source_clk);
			} else {
				pr_err("*** %s Get extclk_from_ep%d Clock error\n", __func__, ep_num);
			}
		}

		if (of_property_read_u32(of_node, "extclk2_from", &m_ext_clk) == 0) {

			sprintf(clk_name, "%s%d", "ext2_from_ep", ep_num);
			extclk = clk_get(NULL, clk_name);
			if (!IS_ERR(extclk)) {
				source_clk = clk_get(NULL, ext_name[m_ext_clk]);
				if (IS_ERR(source_clk)) {
					pr_err("*** %s Get ext2 source error\n", __func__);
				}
				clk_set_parent(extclk, source_clk);
			} else {
				pr_err("*** %s Get extclk2_from_ep%d Clock error\n", __func__, ep_num);
			}
		}

		if (of_property_read_u32(of_node, "extclk3_from", &m_ext_clk) == 0) {

			sprintf(clk_name, "%s%d", "ext3_from_ep", ep_num);
			extclk = clk_get(NULL, clk_name);
			if (!IS_ERR(extclk)) {
				source_clk = clk_get(NULL, ext_name[m_ext_clk]);
				if (IS_ERR(source_clk)) {
					pr_err("*** %s Get ext3 source error\n", __func__);
				}
				clk_set_parent(extclk, source_clk);
			} else {
				pr_err("*** %s Get extclk3_from_ep%d Clock error\n", __func__, ep_num);
			}
		}

		if (of_property_read_u32(of_node, "extclk4_from", &m_ext_clk) == 0) {

			sprintf(clk_name, "%s%d", "ext4_from_ep", ep_num);
			extclk = clk_get(NULL, clk_name);
			if (!IS_ERR(extclk)) {
				source_clk = clk_get(NULL, ext_name[m_ext_clk]);
				if (IS_ERR(source_clk)) {
					pr_err("*** %s Get ext4 source error\n", __func__);
				}
				clk_set_parent(extclk, source_clk);
			} else {
				pr_err("*** %s Get extclk4_from_ep%d Clock error\n", __func__, ep_num);
			}
		}

	} else {
		pr_err("%s no extclk_src_ep%d node found in dtsi\n", __func__, ep_num);
	}
}

static int clk_nvt_ep_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	struct resource res;
	int ep_count;
	const struct of_device_id *match;
	struct device_node *np;
	void (*clk_init_cb)(struct device_node *);
	void __iomem *remap_ep_base = NULL;


	if (of_property_read_u32(node, "chip_id", &ep_idx)) {
		dev_err(&pdev->dev, "%s: chip_id not find, probe fail\n", __func__);
		return -EINVAL;
	}

	ep_count = nvtpcie_get_ep_count();
	if (ep_count < ep_idx){
		dev_info(&pdev->dev, "%s: skip ep[%d]\n", __func__, ep_idx - CHIP_EP0);
		return 0;
	}

	spin_lock_init(&cg_lock);

	if (of_address_to_resource(node, 0, &res)) {
		dev_err(&pdev->dev, "%s: Failed to get resource of cg_ep\n", __func__);
		return -ENXIO;
	} else {
		nvtpcie_conv_resource(ep_idx, &res);
		remap_ep_base = ioremap(res.start, resource_size(&res));
		if (NULL == remap_ep_base)
			dev_err(&pdev->dev, "%s: Failed to remap cg_ep register\n", __func__);
		else
			nvt_cg_ep_base_remap(remap_ep_base, ep_idx - CHIP_EP0);
	}

	for_each_matching_node_and_match(np, nvt_clk_match, &match) {
		clk_init_cb = match->data;
		clk_init_cb(np);
	}

	dev_info(&pdev->dev, "%s: cg_ep[%d](0x%llx), ep_count(%d)\n", __func__, ep_idx - CHIP_EP0, res.start, ep_count);

	ep_ext_clkout_src_set(ep_idx - CHIP_EP0);
	return 0;
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
MODULE_VERSION("1.00.001");
