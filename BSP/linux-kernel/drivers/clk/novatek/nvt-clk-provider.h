/**
    NVT clock header file
    This file will provide NVT clock related structure & API
    @file       nvt-clk-provider.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#ifndef __NVT_CLK_PROVIDER_H__
#define __NVT_CLK_PROVIDER_H__

#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/soc/nvt/nvt-pcie-lib.h>

#define PROG_SOURCE_MAX 16 /* Maximum clock source(parent) */

#define CLK_NAME_STR_SIZE 32

#define PLL_CAL_FACTOR  0x20000

#ifdef CONFIG_NVT_CC
#include <plat/nvt_cc.h>
#define nvt_spin_lock_irqsave(lock, flags)                                      \
	do {                                                                    \
		spin_lock_irqsave(lock, flags);                                 \
		cc_hwlock_resource(CC_CORE_CPU1, CC_RESOURCE_CKG);        \
	} while (0)

#define nvt_spin_unlock_irqrestore(lock, flags)                                 \
	do {                                                                    \
		cc_hwunlock_resource(CC_CORE_CPU1, CC_RESOURCE_CKG);      \
		spin_unlock_irqrestore(lock, flags);                            \
	} while (0)

#else
#define nvt_spin_lock_irqsave(lock, flags)                                      \
	do {                                                                    \
		spin_lock_irqsave(lock, flags);                                 \
	} while (0)

#define nvt_spin_unlock_irqrestore(lock, flags)                                 \
	do {                                                                    \
		spin_unlock_irqrestore(lock, flags);                            \
	} while (0)
#endif /* CONFIG_NVT_CC */

struct nvt_pll_clk {
	void __iomem *cg_base;
	struct clk_hw pll_hw;
	char name[CLK_NAME_STR_SIZE];
	const char **parent_names;
	unsigned long long pll_ratio;
	unsigned long rate_reg_offset;
	unsigned long gate_reg_offset;
	unsigned char gate_bit_idx;
	unsigned char rate_bit_idx;
	unsigned char rate_bit_width;
	unsigned long status_reg_offset;
	unsigned char status_bit_idx;
	unsigned long current_rate;
	unsigned char num_parents;
	bool pll_fixed;
	struct clk_hw *fixed_factor;
	spinlock_t *lock;
	/* Used to PM */
	unsigned char is_enabled;
	struct list_head list;
};

struct nvt_composite_mux_clk {
	void __iomem *cg_base;
	struct clk_mux mux;
	char name[CLK_NAME_STR_SIZE];
	const char **parent_names;
	unsigned long current_rate;	/* HZ */
	unsigned char parent_idx;
	unsigned long mux_reg_offset;
	unsigned char mux_bit_idx;
	unsigned char mux_bit_width;
	unsigned long div_reg_offset;
	unsigned char div_bit_idx;
	unsigned char div_bit_width;
	unsigned long gate_reg_offset;
	unsigned char gate_bit_idx;
	unsigned char do_enable;
	unsigned long reset_reg_offset;
	unsigned char reset_bit_idx;
	bool do_reset;
	unsigned long autogating_reg_offset;
	unsigned char autogating_bit_idx;
	bool do_autogating;
	unsigned char num_parents;
	unsigned char mux_flags;
	unsigned long parent_rate;	/* HZ */
	unsigned long max_rate;	/* HZ */
	int keep_rate;
	spinlock_t *lock;
	/* Used to PM */
	unsigned char is_enabled;
	struct list_head list;
};

int nvt_pll_clk_register(struct nvt_pll_clk *p_pll_clks, unsigned long pll_div_value);
int nvt_composite_mux_clk_register(struct nvt_composite_mux_clk *p_cmux_clk);
void fastboot_determination(void);
bool nvt_check_child(struct clk *clk);
void nvt_clkpll_pm_resume(struct nvt_pll_clk *pll);
void nvt_clkpll_pm_suspend(struct nvt_pll_clk *pll);
void nvt_clkmux_pm_resume(struct nvt_composite_mux_clk *mux);
void nvt_clkmux_pm_suspend(struct nvt_composite_mux_clk *mux);
#endif
