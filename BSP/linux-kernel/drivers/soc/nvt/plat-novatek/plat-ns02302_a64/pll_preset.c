/**
    NVT pll preset function
    This file will preset PLL value
    @file       pll_preset.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/clk-provider.h>
#include <plat/nvt-sramctl.h>

static void ext_clkout_src_set(void)
{
	struct clk* extclk;
	struct clk *source_clk;
	u32 m_ext_clk = 0x0;
	struct device_node* of_node = of_find_node_by_name(NULL, "extclk_src");
	char *ext_name[4] = {"ext", "ext2", "ext3", "ext4"};

	// rc
	if (of_node) {
		if (of_property_read_u32(of_node, "extclk_from", &m_ext_clk) == 0) {

			extclk = clk_get(NULL, "extclk_from");
			if (!IS_ERR(extclk)) {
				source_clk = clk_get(NULL, ext_name[m_ext_clk]);
				if (IS_ERR(source_clk)) {
					pr_err("*** %s Get ext source error\n", __func__);
				}
				clk_set_parent(extclk, source_clk);
			} else {
				pr_err("*** %s Get extclk_from Clock error\n", __func__);
			}
		}

		if (of_property_read_u32(of_node, "extclk2_from", &m_ext_clk) == 0) {

			extclk = clk_get(NULL, "extclk2_from");
			if (!IS_ERR(extclk)) {
				source_clk = clk_get(NULL, ext_name[m_ext_clk]);
				if (IS_ERR(source_clk)) {
					pr_err("*** %s Get ext2 source error\n", __func__);
				}
				clk_set_parent(extclk, source_clk);
			} else {
				pr_err("*** %s Get extclk2_from Clock error\n", __func__);
			}
		}

		if (of_property_read_u32(of_node, "extclk3_from", &m_ext_clk) == 0) {

			extclk = clk_get(NULL, "extclk3_from");
			if (!IS_ERR(extclk)) {
				source_clk = clk_get(NULL, ext_name[m_ext_clk]);
				if (IS_ERR(source_clk)) {
					pr_err("*** %s Get ext3 source error\n", __func__);
				}
				clk_set_parent(extclk, source_clk);
			} else {
				pr_err("*** %s Get extclk3_from Clock error\n", __func__);
			}
		}

		if (of_property_read_u32(of_node, "extclk4_from", &m_ext_clk) == 0) {

			extclk = clk_get(NULL, "extclk4_from");
			if (!IS_ERR(extclk)) {
				source_clk = clk_get(NULL, ext_name[m_ext_clk]);
				if (IS_ERR(source_clk)) {
					pr_err("*** %s Get ext4 source error\n", __func__);
				}
				clk_set_parent(extclk, source_clk);
			} else {
				pr_err("*** %s Get extclk4_from Clock error\n", __func__);
			}
		}

	} else {
		pr_err("%s no extclk_src node found in dtsi\n", __func__);
	}

	// ep0
	of_node = of_find_node_by_name(NULL, "extclk_src_ep0");

	if (of_node) {
		if (of_property_read_u32(of_node, "extclk_from", &m_ext_clk) == 0) {

			extclk = clk_get(NULL, "extclk_from_ep0");
			if (!IS_ERR(extclk)) {
				source_clk = clk_get(NULL, ext_name[m_ext_clk]);
				if (IS_ERR(source_clk)) {
					pr_err("*** %s Get ext source error\n", __func__);
				}
				clk_set_parent(extclk, source_clk);
			} else {
				pr_err("*** %s Get extclk_from Clock error\n", __func__);
			}
		}

		if (of_property_read_u32(of_node, "extclk2_from", &m_ext_clk) == 0) {

			extclk = clk_get(NULL, "extclk2_from_ep0");
			if (!IS_ERR(extclk)) {
				source_clk = clk_get(NULL, ext_name[m_ext_clk]);
				if (IS_ERR(source_clk)) {
					pr_err("*** %s Get ext2 source error\n", __func__);
				}
				clk_set_parent(extclk, source_clk);
			} else {
				pr_err("*** %s Get extclk2_from Clock error\n", __func__);
			}
		}

		if (of_property_read_u32(of_node, "extclk3_from", &m_ext_clk) == 0) {

			extclk = clk_get(NULL, "extclk3_from_ep0");
			if (!IS_ERR(extclk)) {
				source_clk = clk_get(NULL, ext_name[m_ext_clk]);
				if (IS_ERR(source_clk)) {
					pr_err("*** %s Get ext3 source error\n", __func__);
				}
				clk_set_parent(extclk, source_clk);
			} else {
				pr_err("*** %s Get extclk3_from Clock error\n", __func__);
			}
		}

		if (of_property_read_u32(of_node, "extclk4_from", &m_ext_clk) == 0) {

			extclk = clk_get(NULL, "extclk4_from_ep0");
			if (!IS_ERR(extclk)) {
				source_clk = clk_get(NULL, ext_name[m_ext_clk]);
				if (IS_ERR(source_clk)) {
					pr_err("*** %s Get ext4 source error\n", __func__);
				}
				clk_set_parent(extclk, source_clk);
			} else {
				pr_err("*** %s Get extclk4_from Clock error\n", __func__);
			}
		}

	} else {
		pr_err("%s no extclk_src_ep0 node found in dtsi\n", __func__);
	}
}

static void disable_cnn(void)
{
}

static int __init nvt_preset_pll(void)
{
	struct device_node *node, *pll;
	struct clk* pll_clk;
	char pll_name[10] = {};
	u32 value[3] = {};

	node = of_find_node_by_path("/pll_preset@0");
	if (node) {
		for_each_child_of_node(node, pll) {
			if (!of_property_read_u32_array(pll, "pll_config", value, 3)) {
				if (value[1]) {
					snprintf(pll_name, 10, "pll%d", value[0]);
					pll_clk = clk_get(NULL, pll_name);
					if (!IS_ERR(pll_clk))
						clk_set_rate(pll_clk, value[1]);
					else
						pr_err("*** %s Get PLL%d Clock error\n", __func__, value[0]);
				}

				if (value[2]) {
					snprintf(pll_name, 10, "pll%d", value[0]);
					pll_clk = clk_get(NULL, pll_name);
					if (!IS_ERR(pll_clk)) {
						if (!__clk_is_enabled(pll_clk))
							clk_prepare_enable(pll_clk);
					} else
						pr_err("*** %s Get PLL%d Clock error\n", __func__, value[0]);
				}
			}
		}
	} else
		pr_err("*** %s not get dts node ***\n", __func__);

	disable_cnn();
	ext_clkout_src_set();

	return 0;
}

arch_initcall_sync(nvt_preset_pll);
