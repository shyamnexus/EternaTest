/**
    NVT clock management module
    To handle Linux clock framework API interface
    @file nvt-clk-provider.c
    @ingroup
    @note
    Copyright Novatek Microelectronics Corp. 2022. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/soc/nvt/nvt-clk-ext.h>
#include <plat/cg-reg.h>
#include <plat/efuse_protected.h>
#include <linux/console.h>

#include "nvt-clk-provider.h"
#define div_mask(width)	((1 << (width)) - 1)

unsigned int is_ARMPLL = 0x0;
unsigned int is_fastboot = 0x0;

static inline uint64_t roundup_64(uint64_t x, uint32_t y)
{
	x += y - 1;
	do_div(x, y);
	return x * y;
}

static int __init early_param_rtos_boot(char *p)
{
	if (p == NULL) {
		return 0;
	}
	if (strncmp(p, "2", 2) == 0) {
		is_fastboot = 1;
	}
	return 0;
}

early_param("rtos_boot", early_param_rtos_boot);

void fastboot_determination(void)
{
	struct device_node *of_node = of_find_node_by_path("/fastboot");

	if (of_node) {
		of_property_read_u32(of_node, "enable", &is_fastboot);
	}
}

#ifdef CONFIG_NVT_IVOT_PLAT_NS02301

#define PR_PLL_FLL_6M_MAGNI_REG0_REG_OFS	0x4008
#define PR_PLL_FLL_6M_MAGNI_REG1_REG_OFS	0x400C
#define TRIM_FIELD				0x4010
#define PR_PLL_FLL_OPEN_SLOW_DOWN_REG_OFS	0x4044

struct nvt_composite_mux_clk *prfll;

int pll_get_fll_is_6m(void)
{
	unsigned int fll_6m_magni_0, fll_6m_magni_1, fll_open_slow_down;
	struct nvt_composite_mux_clk *mux = prfll;

	fll_6m_magni_0 = readl(mux->cg_base + mux->gate_reg_offset + PR_PLL_FLL_6M_MAGNI_REG0_REG_OFS);
	fll_6m_magni_1 = readl(mux->cg_base + mux->gate_reg_offset + PR_PLL_FLL_6M_MAGNI_REG1_REG_OFS);
	fll_open_slow_down = readl(mux->cg_base + mux->gate_reg_offset + PR_PLL_FLL_OPEN_SLOW_DOWN_REG_OFS);

	if ((fll_6m_magni_0 == 0x71) && (fll_6m_magni_1 == 0x0b) && (fll_open_slow_down == 0x51)) {
		/* FLL is running on 6M */
		return 0;
	} else {
		if ((fll_6m_magni_0 == 0x8d) && (fll_6m_magni_1 == 0x5b) && (fll_open_slow_down == 0x41)) {
			/* FLL is running on 48M */
		} else {
			pr_err("FLL need to check 48M setting\r\n");
		}

		return -EINVAL;
	}

}
EXPORT_SYMBOL(pll_get_fll_is_6m);

void FLL_change_to_6M(unsigned int en)
{
	unsigned long flags;
	unsigned int fll_power;
	unsigned int fll_6m_magni_0, fll_6m_magni_1, fll_open_slow_down;
	struct nvt_composite_mux_clk *mux = prfll;

	nvt_spin_lock_irqsave(mux->lock, flags);

	fll_power = readl(mux->cg_base + mux->gate_reg_offset);
	fll_6m_magni_0 = readl(mux->cg_base + mux->gate_reg_offset + PR_PLL_FLL_6M_MAGNI_REG0_REG_OFS);
	fll_6m_magni_1 = readl(mux->cg_base + mux->gate_reg_offset + PR_PLL_FLL_6M_MAGNI_REG1_REG_OFS);
	fll_open_slow_down = readl(mux->cg_base + mux->gate_reg_offset + PR_PLL_FLL_OPEN_SLOW_DOWN_REG_OFS);

	if (fll_power & 0x10) {
		/* disable FLL */
		fll_power |= 0x2;
		writel(fll_power, mux->cg_base + mux->gate_reg_offset);
		udelay(10);
		/* set 6m magnification */
		fll_6m_magni_0 &= 0xFFFFFF00;
		fll_6m_magni_0 |= 0x71;
		writel(fll_6m_magni_0, mux->cg_base + mux->gate_reg_offset + PR_PLL_FLL_6M_MAGNI_REG0_REG_OFS);

		/* set 6m magnification */
		fll_6m_magni_1 &= 0xFFFFFF00;
		fll_6m_magni_1 |= 0xb;
		writel(fll_6m_magni_1, mux->cg_base + mux->gate_reg_offset + PR_PLL_FLL_6M_MAGNI_REG1_REG_OFS);
		/* set open slow_en */
		fll_open_slow_down &= 0xFFFFFF00;
		fll_open_slow_down |= 0x51;
		writel(fll_open_slow_down, mux->cg_base + mux->gate_reg_offset + PR_PLL_FLL_OPEN_SLOW_DOWN_REG_OFS);
		pr_info("FLL is not disable for chagne 6M\r\n");
	} else {
		/* set 6m magnification */
		fll_6m_magni_0 &= 0xFFFFFF00;
		fll_6m_magni_0 |= 0x71;
		writel(fll_6m_magni_0, mux->cg_base + mux->gate_reg_offset + PR_PLL_FLL_6M_MAGNI_REG0_REG_OFS);

		/* set 6m magnification */
		fll_6m_magni_1 &= 0xFFFFFF00;
		fll_6m_magni_1 |= 0xb;
		writel(fll_6m_magni_1, mux->cg_base + mux->gate_reg_offset + PR_PLL_FLL_6M_MAGNI_REG1_REG_OFS);

		/* set open slow_en */
		fll_open_slow_down &= 0xFFFFFF00;
		fll_open_slow_down |= 0x51;
		writel(fll_open_slow_down, mux->cg_base + mux->gate_reg_offset + PR_PLL_FLL_OPEN_SLOW_DOWN_REG_OFS);

	}
	nvt_spin_unlock_irqrestore(mux->lock, flags);
	pr_info("FLL 6M change\r\n");

}
EXPORT_SYMBOL(FLL_change_to_6M);

static int nvt_fll_enable(struct nvt_composite_mux_clk *mux)
{
	unsigned long flags;
	unsigned int FLL_6M, FLL_48M, FLL_Ready, Ready_timeout;
	unsigned int result, fll_sts, reg_data, trim_reg;
	u16 data;

	result = efuse_readParamOps(EFUSE_FLL_TRIM_DATA, &data);
	if (result) {
		pr_info("%s: Trim value does not exist, use default value\n", __func__);
	}

	FLL_6M  = (data & 0x7);
	FLL_48M = ((data >> 3) & 0x7);
	reg_data = readl(mux->cg_base + mux->gate_reg_offset);
	fll_sts = (reg_data & 0x10)>>4;

	nvt_spin_lock_irqsave(mux->lock, flags);

	if (fll_sts == 1) {
		reg_data = readl(mux->cg_base + mux->gate_reg_offset);
		reg_data |= 0x2;
		writel(reg_data, mux->cg_base + mux->gate_reg_offset);

		trim_reg = readl(mux->cg_base + mux->gate_reg_offset + TRIM_FIELD);
		trim_reg &= ~0x1c;
		if(pll_get_fll_is_6m() == 0){
			trim_reg |= ((FLL_6M&0x3f) << 2);
		}
		else{
			trim_reg |= ((FLL_48M&0x3f) << 2);
		}
		writel(trim_reg, mux->cg_base + mux->gate_reg_offset + TRIM_FIELD);

		reg_data = readl(mux->cg_base + mux->gate_reg_offset);
		reg_data |= 0x1;
		writel(reg_data, mux->cg_base + mux->gate_reg_offset);

	} else {
		trim_reg = readl(mux->cg_base + mux->gate_reg_offset + TRIM_FIELD);
		trim_reg &= ~0x1c;
		if(pll_get_fll_is_6m() == 0){
			trim_reg |= ((FLL_6M&0x3f) << 2);
		}
		else{
			trim_reg |= ((FLL_48M&0x3f) << 2);
		}
		writel(trim_reg, mux->cg_base + mux->gate_reg_offset + TRIM_FIELD);

		reg_data = readl(mux->cg_base + mux->gate_reg_offset);
		reg_data |= 0x1;
		writel(reg_data, mux->cg_base + mux->gate_reg_offset);
	}

	FLL_Ready = 0;
	Ready_timeout = 0;
	while (!FLL_Ready) {
		reg_data = readl(mux->cg_base + mux->gate_reg_offset);
		FLL_Ready = ((reg_data&0x4)>>2);
		udelay(30);
		Ready_timeout++;
		if (Ready_timeout > 1000) { 
			pr_err("Change FLL timeout!!!\r\n");
			break;
		}
	}
	nvt_spin_unlock_irqrestore(mux->lock, flags);

	pr_info("FLL 6M Trim val[%d], FLL 48M Trim val[%d]\r\n", (int)FLL_6M, (int)FLL_48M);

	return 0;
}
#endif

static int nvt_pll_clk_enable(struct clk_hw *hw)
{
	unsigned int reg_val;
	unsigned long flags;
	struct nvt_pll_clk *pll = container_of(hw, struct nvt_pll_clk, pll_hw);

	/* race condition protect. enter critical section */
	nvt_spin_lock_irqsave(pll->lock, flags);
	reg_val = readl(pll->cg_base + pll->gate_reg_offset);
	reg_val |= (0x1 << pll->gate_bit_idx);
	writel(reg_val, pll->cg_base + pll->gate_reg_offset);

	if (!(readl(pll->cg_base + pll->status_reg_offset) & (0x1 << pll->gate_bit_idx)))
		mdelay(3);
	/* race condition protect. leave critical section */
	nvt_spin_unlock_irqrestore(pll->lock, flags);

	return 0;
}

static void nvt_pll_clk_disable(struct clk_hw *hw)
{
	unsigned int reg_val;
	unsigned long flags;
	struct nvt_pll_clk *pll = container_of(hw, struct nvt_pll_clk, pll_hw);
	
	/* race condition protect. enter critical section */
	nvt_spin_lock_irqsave(pll->lock, flags);
	reg_val = readl(pll->cg_base + pll->gate_reg_offset);
	reg_val &= ~(0x1 << pll->gate_bit_idx);
	writel(reg_val, pll->cg_base + pll->gate_reg_offset);
	/* race condition protect. leave critical section */
	nvt_spin_unlock_irqrestore(pll->lock, flags);
}

static int nvt_pll_clk_is_enabled(struct clk_hw *hw)
{
	unsigned long flags;
	bool is_enabled;
	struct nvt_pll_clk *pll = container_of(hw, struct nvt_pll_clk, pll_hw);

	/* race condition protect. enter critical section */
	nvt_spin_lock_irqsave(pll->lock, flags);
	is_enabled = readl(pll->cg_base + pll->gate_reg_offset) & (0x1 << pll->gate_bit_idx);
	/* race condition protect. leave critical section */
	nvt_spin_unlock_irqrestore(pll->lock, flags);

	return is_enabled;
}

static long nvt_pll_clk_round_rate(struct clk_hw *hw, unsigned long rate,
		unsigned long *parent_rate)
{
#ifndef CONFIG_NVT_FPGA_EMULATION
	struct nvt_pll_clk *pll = container_of(hw, struct nvt_pll_clk, pll_hw);
	unsigned long long temp_rate, ratio;

	temp_rate = rate;

	ratio = (temp_rate * PLL_CAL_FACTOR);

	do_div(ratio, (*parent_rate));

	if (!ratio)
		pll->pll_ratio = 1;
	else
		pll->pll_ratio = ratio;

	if (strncmp(pll->name, "cpu_clk", strlen("cpu_clk")) == 0) {
		do_div(ratio, CG_CPU_RATIO);
		pll->pll_ratio = ratio;
	}

	temp_rate = pll->pll_ratio * (*parent_rate);

	do_div(temp_rate, PLL_CAL_FACTOR);

	pll->current_rate = roundup_64(temp_rate, 1000);
#endif
	return rate;
}

static unsigned long nvt_pll_clk_recalc_rate(struct clk_hw *hw,
		unsigned long parent_rate)
{
	struct nvt_pll_clk *pll = container_of(hw, struct nvt_pll_clk, pll_hw);
	unsigned long long temp_rate;
	unsigned int ratio0, ratio1, ratio2;

	ratio0 = readl(pll->cg_base + pll->rate_reg_offset) & 0xFF;
	ratio1 = readl(pll->cg_base + pll->rate_reg_offset + 0x4) & 0xFF;
	ratio2 = readl(pll->cg_base + pll->rate_reg_offset + 0x8) & 0xFF;

	pll->pll_ratio = ratio0 | (ratio1 << 8) | (ratio2 << 16);

	temp_rate = pll->pll_ratio * parent_rate;

	do_div(temp_rate, PLL_CAL_FACTOR);

	pll->current_rate = roundup_64(temp_rate, 1000);

	if (strncmp(pll->name, "cpu_clk", strlen("cpu_clk")) == 0) {
		pll->current_rate = pll->current_rate * CG_CPU_RATIO;
	}

#ifdef CONFIG_NVT_FPGA_EMULATION
	pll->current_rate = 12000000;
	pr_debug("%s: use fixed 12M on FPGA\n", __func__);
#endif

	return pll->current_rate;
}

#if defined(CONFIG_NVT_IVOT_PLAT_NS02201)
static int nvt_pll_cpuclk_set_rate(struct nvt_pll_clk *pll)
{
	unsigned long flags;
	unsigned int cpu_arm_pll;

	cpu_arm_pll = 1;
	nvt_spin_lock_irqsave(pll->lock, flags);
	if (pll->pll_ratio < 0x28000) {
		pll->pll_ratio *= CG_CPU_RATIO;                                               
		cpu_arm_pll = 0;                                                              
		//Prepare use normal PLL                                                      
		//0xF0028000 bit[0] = 0 //switch to 1x mode                                   
		writel(readl(pll->cg_base + 0x8000) & (~(0x1)), pll->cg_base + 0x8000);

		//resetb_armpll --> disable PLL & BG LDO OFF                                  
		writel(readl(pll->cg_base + 0x8000 + 0x10) & (~(0x3)), pll->cg_base + 0x8000 + 0x10);
	}

	writel(pll->pll_ratio & 0xFF, pll->cg_base + pll->rate_reg_offset);
	writel(((pll->pll_ratio >> 8) & 0xFF), pll->cg_base + pll->rate_reg_offset + 0x4);
	writel((pll->pll_ratio >> 16) & 0xFF, pll->cg_base + pll->rate_reg_offset + 0x8);

	if (cpu_arm_pll) {
		udelay( 300);
		//resetb_armpll --> enable PLL & BG LDO ON
		writel(readl(pll->cg_base + 0x8000 + 0x10) | 0x3, pll->cg_base + 0x8000 + 0x10);
		udelay(500);
		//select ti ARMPLL
		writel(readl(pll->cg_base + 0x8000) | 0x1, pll->cg_base + 0x8000);
	}
	nvt_spin_unlock_irqrestore(pll->lock, flags);

	return 0;
}
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02302)
static int nvt_pll_cpuclk_set_rate(struct nvt_pll_clk *pll)
{
	unsigned long flags;
	unsigned int cpu_arm_pll;

	cpu_arm_pll = 1;
	nvt_spin_lock_irqsave(pll->lock, flags);
	if (pll->pll_ratio < 0xA0000) {
		pll->pll_ratio *= CG_CPU_RATIO;                                               
		cpu_arm_pll = 0;                                                              
		//Prepare use normal PLL                                                      
		//0x2F0234340 bit[0] = 0 //switch to 1x mode                                   
		writel(readl(pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x340) & (~(0x1)), pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x340);

		//resetb_armpll 0x344 bit[4] = 0 --> disable PLL 
		writel(readl(pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x344) & (~(0x10)), pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x344);

		//& BG LDO OFF (0x3D0 bit[2] = 0)                              
		writel(readl(pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x3D0) & (~(0x4)), pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x3D0);

		//0x100 bit[6] = 0
 		writel(readl(pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x100) & (~(0x40)), pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x100);
	}
	writel(pll->pll_ratio & 0xFF, pll->cg_base + pll->rate_reg_offset);
	writel(((pll->pll_ratio >> 8) & 0xFF), pll->cg_base + pll->rate_reg_offset + 0x4);
	writel((pll->pll_ratio >> 16) & 0xFF, pll->cg_base + pll->rate_reg_offset + 0x8);

	if (cpu_arm_pll) {
		//0x100 bit[6] = 1
 		writel(readl(pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x100) | (0x40), pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x100);
		//& BG LDO OFF (0x3D0 bit[2] = 0)                              
		writel(readl(pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x3D0) | (0x4), pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x3D0);
		udelay( 300);
		// Select band select 0x3C8 bit[7..5] = 0x10 (1000~1399)
		writel(((readl(pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x3C8) & (~(0xE0))) | 0x40), (pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x3C8));
		// de-assert resetb_armpll to 1: Write 0xD1 = 0x10 (0x344) bit[4] = 1		
		writel((readl(pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x344) | (0x10)), (pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x344));
		udelay(500);
		//select ti ARMPLL	
		// Set ARMPLL output. Write 0xD0 = 0x03 (0x340)
		writel(readl(pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x340) | (0x1), pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x340);
	}
	nvt_spin_unlock_irqrestore(pll->lock, flags);

	return 0;
}
#endif

static int nvt_pll_clk_set_rate(struct clk_hw *hw, unsigned long rate,
		unsigned long parent_rate)
{
	struct nvt_pll_clk *pll = container_of(hw, struct nvt_pll_clk, pll_hw);
	unsigned long flags;

#if defined(CONFIG_NVT_IVOT_PLAT_NS02201)
	if (!strncmp(pll->name, "pll16", strlen("pll16"))) {
		nvt_pll_cpuclk_set_rate(pll);
		return 0;
	}
#endif

#if defined(CONFIG_NVT_IVOT_PLAT_NS02302)
	//printk("PLL clock set rate name[%s]\r\n", pll->name);
	if (!strncmp(pll->name, "pll8", strlen("pll8"))) {
		nvt_pll_cpuclk_set_rate(pll);
		return 0;
	}
#endif

#ifdef CONFIG_NVT_NT98567_EVB
	/* Set the pll11 clock change bit to support pll11 clock change */
	if (strncmp(pll->name, "pll11", 5) == 0) {
		switch (rate) {
			case 480000000 :
				writel(0x0, pll->cg_base + 0x4008);
				break;
			case 320000000 :
				writel(0x1, pll->cg_base + 0x4008);
				break;
			default :
				writel(0x2, pll->cg_base + 0x4008);
				break;
		}
	}
#endif

	nvt_spin_lock_irqsave(pll->lock, flags);
	writel(pll->pll_ratio & 0xFF, pll->cg_base + pll->rate_reg_offset);
	writel(((pll->pll_ratio >> 8) & 0xFF), pll->cg_base + pll->rate_reg_offset + 0x4);
	writel((pll->pll_ratio >> 16) & 0xFF, pll->cg_base + pll->rate_reg_offset + 0x8);
	nvt_spin_unlock_irqrestore(pll->lock, flags);

	return 0;
}

static const struct clk_ops nvt_pll_clk_ops = {
	.enable = nvt_pll_clk_enable,
	.disable = nvt_pll_clk_disable,
	.is_enabled = nvt_pll_clk_is_enabled,
	.round_rate = nvt_pll_clk_round_rate,
	.recalc_rate = nvt_pll_clk_recalc_rate,
	.set_rate = nvt_pll_clk_set_rate,
};

int nvt_pll_clk_register(struct nvt_pll_clk *pll_clks, unsigned long pll_div_value)
{
	struct clk *clk;
	struct clk_init_data init;
	int ret = 0;
	unsigned long long rate, pll_ratio;

	init.name = pll_clks->name;
	init.ops = &nvt_pll_clk_ops;
	init.flags = CLK_IGNORE_UNUSED | CLK_GET_RATE_NOCACHE;
	init.num_parents = 1;
	init.parent_names = pll_clks->parent_names;
	pll_clks->pll_hw.init = &init;
	/* To avoid being changed on the clock framework */
	pll_ratio = pll_clks->pll_ratio;

	clk = clk_register(NULL, &pll_clks->pll_hw);
	if (IS_ERR(clk)) {
		pr_err("%s: failed to register clk %s!\n", __func__,
				pll_clks->name);
		ret = -EPERM;
		goto err;
	}

	if (pll_clks->rate_reg_offset && (!pll_clks->pll_fixed)) {
		if (!is_fastboot) {
			rate = clk_get_rate(clk_get_parent(clk));
			rate = rate * pll_ratio;
			do_div(rate, pll_div_value);
			clk_set_rate(clk, rate);
		}
	}

	if (clk_register_clkdev(clk, pll_clks->name, NULL)) {
		pr_err("%s: failed to register lookup %s!\n", __func__,
				pll_clks->name);
		ret = -EPERM;
		goto err;
	}


	if (__clk_is_enabled(clk)) {
		ret = clk_prepare_enable(clk);
		if (ret < 0)
			pr_err("%s: %s prepare & enable failed!\n", __func__,
					pll_clks->name);
	}

err:
	return ret;
}
EXPORT_SYMBOL(nvt_pll_clk_register);

static int nvt_composite_mux_clk_is_enabled(struct clk_hw *hw)
{
	unsigned long flags;
	bool is_enabled = false;
	struct clk_mux *cmux = container_of(hw, struct clk_mux, hw);
	struct nvt_composite_mux_clk *mux =
		container_of(cmux, struct nvt_composite_mux_clk, mux);

#ifdef CONFIG_NVT_IVOT_PLAT_NS02301
	if (strncmp(mux->name, "prfll", strlen("prfll")) == 0) {
		nvt_spin_lock_irqsave(mux->lock, flags);
		is_enabled = readl(mux->cg_base + mux->gate_reg_offset) & (0x10);
		nvt_spin_unlock_irqrestore(mux->lock, flags);
		return is_enabled;
	}
#endif

	/* race condition protect. enter critical section */
	nvt_spin_lock_irqsave(mux->lock, flags);

	if (mux->gate_reg_offset != 0)
		is_enabled = readl(mux->cg_base + mux->gate_reg_offset) & (0x1 << mux->gate_bit_idx);

	/* race condition protect. leave critical section */
	nvt_spin_unlock_irqrestore(mux->lock, flags);

	return is_enabled;
}

static int nvt_composite_mux_clk_prepare(struct clk_hw *hw)
{
	unsigned int reg_val;
	unsigned long flags;
	struct clk_mux *cmux = container_of(hw, struct clk_mux, hw);
	struct nvt_composite_mux_clk *mux =
	    container_of(cmux, struct nvt_composite_mux_clk, mux);

	/* Warning LCD and HDMI rates have been changed for debugging */
	if (strstr(mux->name, "lcd") != NULL || strstr(mux->name, "hdmi") != NULL) {
		/* Reset always changes from 0 -> 1 or 1 -> 0 -> 1 */
		pr_info("%s: %s reset\n", __func__, mux->name);
	}

	if (mux->reset_reg_offset != 0) {
#ifdef CONFIG_NVT_IVOT_PLAT_NS02301
		if ((strstr(mux->name, "spi") != NULL) || (strstr(mux->name, "f0510000.mmc") != NULL))
			return 0;
#endif
		nvt_spin_lock_irqsave(mux->lock, flags);
		/* race condition protect. enter critical section */
		reg_val = readl(mux->cg_base + mux->reset_reg_offset);
		if (mux->do_reset) {
			reg_val &= ~(0x1 << mux->reset_bit_idx);
			writel(reg_val, mux->cg_base + mux->reset_reg_offset);
			udelay(10);
		}
		reg_val |= 0x1 << mux->reset_bit_idx;
		writel(reg_val, mux->cg_base + mux->reset_reg_offset);
		udelay(10);
		/* race condition protect. leave critical section */
		nvt_spin_unlock_irqrestore(mux->lock, flags);
	}

	return 0;
}

static void nvt_composite_mux_clk_unprepare(struct clk_hw *hw)
{
	/* Do nothing */
}

static int nvt_composite_mux_clk_is_prepared(struct clk_hw *hw)
{
	unsigned int reg_val;
	unsigned long flags;
	struct clk_mux *cmux = container_of(hw, struct clk_mux, hw);
	struct nvt_composite_mux_clk *mux =
		container_of(cmux, struct nvt_composite_mux_clk, mux);

	if (mux->reset_reg_offset != 0) {
		/* race condition protect. enter critical section */
		nvt_spin_lock_irqsave(mux->lock, flags);
		/* Check if RSTN bit is released */
		reg_val = readl(mux->cg_base + mux->reset_reg_offset);
		/* race condition protect. leave critical section */
		nvt_spin_unlock_irqrestore(mux->lock, flags);
		return (reg_val & (0x1 << mux->reset_bit_idx));
	} else
		return 0;
}

static int nvt_composite_mux_clk_enable(struct clk_hw *hw)
{
	unsigned int reg_val;
	unsigned long flags;
	struct clk_mux *cmux = container_of(hw, struct clk_mux, hw);
	struct nvt_composite_mux_clk *mux =
	    container_of(cmux, struct nvt_composite_mux_clk, mux);

#ifdef CONFIG_NVT_IVOT_PLAT_NS02301
	if (mux->gate_reg_offset != 0) {
		if (nvt_composite_mux_clk_is_enabled(hw)) {
			return 0;
		}
	}

	if (strncmp(mux->name, "prfll", strlen("prfll")) == 0) {
		return nvt_fll_enable(mux);
	}
#endif

	if ((mux->max_rate) && (mux->current_rate > mux->max_rate)) {
		pr_err("!!!! %s: %s current_freq(%ld) exceed max_freq(%ld), return !!!!\n",
			__func__, mux->name, mux->current_rate, mux->max_rate);
		return -1;
	}

	if (mux->gate_reg_offset != 0) {
		/* race condition protect. enter critical section */
		nvt_spin_lock_irqsave(mux->lock, flags);
		reg_val = readl(mux->cg_base + mux->gate_reg_offset);
		reg_val |= (0x1 << mux->gate_bit_idx);
		writel(reg_val, mux->cg_base + mux->gate_reg_offset);
		/* race condition protect. leave critical section */
		nvt_spin_unlock_irqrestore(mux->lock, flags);
	}

	return 0;
}

static void nvt_composite_mux_clk_disable(struct clk_hw *hw)
{
	unsigned int reg_val;
	unsigned long flags;
	struct clk_mux *cmux = container_of(hw, struct clk_mux, hw);
	struct nvt_composite_mux_clk *mux =
	    container_of(cmux, struct nvt_composite_mux_clk, mux);

#ifdef CONFIG_NVT_IVOT_PLAT_NS02301
	if (strncmp(mux->name, "prfll", strlen("prfll")) == 0) {
		nvt_spin_lock_irqsave(mux->lock, flags);
		reg_val = readl(mux->cg_base + mux->gate_reg_offset);
		reg_val |= 0x2;
		writel(reg_val, mux->cg_base + mux->gate_reg_offset);
		/* race condition protect. leave critical section */
		nvt_spin_unlock_irqrestore(mux->lock, flags);
		return;
	}
#endif

	if (mux->gate_reg_offset != 0) {
		/* race condition protect. enter critical section */
		nvt_spin_lock_irqsave(mux->lock, flags);
		reg_val = readl(mux->cg_base + mux->gate_reg_offset);
		reg_val &= ~(0x1 << mux->gate_bit_idx);
		writel(reg_val, mux->cg_base + mux->gate_reg_offset);
		/* race condition protect. leave critical section */
		nvt_spin_unlock_irqrestore(mux->lock, flags);
	}
}

static long nvt_composite_mux_clk_round_rate(struct clk_hw *hw,
					     unsigned long rate,
					     unsigned long *prate)
{
	struct clk_mux *cmux = container_of(hw, struct clk_mux, hw);
	struct nvt_composite_mux_clk *mux =
	    container_of(cmux, struct nvt_composite_mux_clk, mux);
	u32 best_div, max_div;

	if (!(mux->div_bit_width))
		return mux->current_rate;

	max_div = div_mask(mux->div_bit_width);

	best_div = DIV_ROUND_UP(*prate, rate);

	if (best_div < 1)
		best_div = 1;
	if (best_div > max_div)
		best_div = max_div;

	mux->current_rate = *prate/best_div;

	if (mux->gate_reg_offset != 0) {
		if ((mux->max_rate) && (mux->current_rate > mux->max_rate) && nvt_composite_mux_clk_is_enabled(hw)) {
			pr_err("!!!! %s: %s current_freq(%ld) exceed max_freq(%ld), return !!!!\n",
					__func__, mux->name, mux->current_rate, mux->max_rate);
		}
	} else {
		if ((mux->max_rate) && (mux->current_rate > mux->max_rate)) {
			pr_err("!!!! %s: %s current_freq(%ld) exceed max_freq(%ld), return !!!!\n",
					__func__, mux->name, mux->current_rate, mux->max_rate);
		}
	}

	return mux->current_rate;
}

static unsigned long nvt_composite_mux_clk_recalc_rate(struct clk_hw *hw,
						       unsigned long parent_rate)
{
	struct clk_mux *cmux = container_of(hw, struct clk_mux, hw);
	struct nvt_composite_mux_clk *mux =
	    container_of(cmux, struct nvt_composite_mux_clk, mux);
	unsigned long flags;
	u32 divider, reg_val;

#ifdef CONFIG_NVT_IVOT_PLAT_NS02301
	if (strncmp(mux->name, "prfll", strlen("prfll")) == 0) {
		if (pll_get_fll_is_6m() == 0)
			mux->current_rate = 6000000;
		else
			mux->current_rate = 48000000;
		return mux->current_rate;
	}
#endif

	/*If there is no divider, parent rate equals to current rate*/
	if (!(mux->div_bit_width))
		mux->current_rate = parent_rate;
	else {
		/*Update current rate with hardware runtime value*/
		nvt_spin_lock_irqsave(mux->lock, flags);
		reg_val = readl(mux->cg_base + mux->div_reg_offset);
		reg_val &= (div_mask(mux->div_bit_width) << mux->div_bit_idx);
		divider = (reg_val >> mux->div_bit_idx);
		nvt_spin_unlock_irqrestore(mux->lock, flags);

		mux->current_rate = parent_rate/(divider+1);
	}

	if (mux->gate_reg_offset != 0) {
		if ((mux->max_rate) && (mux->current_rate > mux->max_rate) && nvt_composite_mux_clk_is_enabled(hw)) {
			pr_err("!!!! %s: %s current_freq(%ld) exceed max_freq(%ld), return !!!!\n",
					__func__, mux->name, mux->current_rate, mux->max_rate);
		}
	} else {
		if ((mux->max_rate) && (mux->current_rate > mux->max_rate)) {
			pr_err("!!!! %s: %s current_freq(%ld) exceed max_freq(%ld), return !!!!\n",
					__func__, mux->name, mux->current_rate, mux->max_rate);
		}
	}

	return mux->current_rate;
}

static int nvt_composite_mux_clk_set_rate(struct clk_hw *hw,
					  unsigned long rate,
					  unsigned long parent_rate)
{
	struct clk_mux *cmux = container_of(hw, struct clk_mux, hw);
	struct nvt_composite_mux_clk *mux =
	    container_of(cmux, struct nvt_composite_mux_clk, mux);
	unsigned long flags;
	u32 divider, reg_val;

	if (!(mux->div_reg_offset))
		return 0;

	if (mux->gate_reg_offset != 0) {
		if ((mux->max_rate) && (mux->current_rate > mux->max_rate) && nvt_composite_mux_clk_is_enabled(hw)) {
			pr_err("!!!! %s: %s current_freq(%ld) exceed max_freq(%ld), return !!!!\n",
					__func__, mux->name, mux->current_rate, mux->max_rate);
			return -1;
		}
	} else {
		if ((mux->max_rate) && (mux->current_rate > mux->max_rate)) {
			pr_err("!!!! %s: %s current_freq(%ld) exceed max_freq(%ld), return !!!!\n",
					__func__, mux->name, mux->current_rate, mux->max_rate);
			return -1;
		}
	}

	divider = (parent_rate / rate);
	if (!divider)
		divider = 1;

	nvt_spin_lock_irqsave(mux->lock, flags);
	reg_val = readl(mux->cg_base + mux->div_reg_offset);
	reg_val &= ~(div_mask(mux->div_bit_width) << mux->div_bit_idx);
	reg_val |= ((divider - 1) << mux->div_bit_idx);
	writel(reg_val, mux->cg_base + mux->div_reg_offset);
	nvt_spin_unlock_irqrestore(mux->lock, flags);

	return 0;
}

static int nvt_composite_mux_clk_set_parent(struct clk_hw *hw, u8 index)
{
	struct clk_mux *cmux = container_of(hw, struct clk_mux, hw);
	struct nvt_composite_mux_clk *mux =
		container_of(cmux, struct nvt_composite_mux_clk, mux);
	unsigned long flags;
	u32 reg_val;
#if defined(CONFIG_NVT_IVOT_PLAT_NS02302)
	struct nvt_pll_clk *pll = container_of(hw, struct nvt_pll_clk, pll_hw);
#endif
	/* Warning LCD and HDMI rates have been changed for debugging */
	if (strstr(mux->name, "lcd") != NULL || strstr(mux->name, "hdmi") != NULL) {
		reg_val = readl(mux->cg_base + mux->mux_reg_offset);
		reg_val &= (div_mask(mux->mux_bit_width) << mux->mux_bit_idx);
		if (reg_val != (index << mux->mux_bit_idx)) {
			pr_info("%s: Change %s clock parent to 0x%x\n", __func__, mux->name, index);
		}
	}

	if (mux->mux_reg_offset != 0) {
		nvt_spin_lock_irqsave(mux->lock, flags);
		reg_val = readl(mux->cg_base + mux->mux_reg_offset);
		reg_val &= ~(div_mask(mux->mux_bit_width) << mux->mux_bit_idx);
		reg_val |= (index << mux->mux_bit_idx);
		writel(reg_val, mux->cg_base + mux->mux_reg_offset);
		nvt_spin_unlock_irqrestore(mux->lock, flags);
	}

#if defined(CONFIG_NVT_IVOT_PLAT_NS02201) || defined (CONFIG_NVT_IVOT_PLAT_NS02301)
	if (mux->mux_reg_offset == CG_SYS_CLK_RATE_REG_OFFSET && mux->mux_bit_idx == 0) {
		writel(0x34, mux->cg_base + CG_SLEEP_MODE_REG_OFFSET);
		while ((readl(mux->cg_base + CG_SLEEP_MODE_REG_OFFSET) & (0x1 << 3)) != (0x1 << 3)) {
		}
		writel(0x28, mux->cg_base + CG_SLEEP_MODE_REG_OFFSET);
	}
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02302)
	if (mux->mux_reg_offset == CG_SYS_CLK_RATE_REG_OFFSET && mux->mux_bit_idx == 0) {
//		printk("offset[0x%02x], mux_bit_idx[%d], index[%02x], reg_val[0x%08x] CG_SLEEP_MODE_REG_OFFSET = 0x%08x\r\n", mux->mux_reg_offset, mux->mux_bit_idx, index, reg_val, CG_SLEEP_MODE_REG_OFFSET);
		nvt_spin_lock_irqsave(mux->lock, flags);
		writel(0x34, mux->cg_base + CG_SLEEP_MODE_REG_OFFSET);
		while ((readl(mux->cg_base + CG_SLEEP_MODE_REG_OFFSET) & (0x1 << 3)) != (0x1 << 3)) {
		}
		writel(0x28, mux->cg_base + CG_SLEEP_MODE_REG_OFFSET);		

		if(index != 1) {			
			//0x2F0234340 bit[0] = 0 //switch to 1x mode    
			if((readl(pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x340) & 0x1) == 0x1) {
				is_ARMPLL = 1;
			} else {
				is_ARMPLL = 0;
			}                               
			writel(readl(pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x340) & (~(0x1)), pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x340);

			//resetb_armpll 0x344 bit[4] = 0 --> disable PLL 
			writel(readl(pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x344) & (~(0x10)), pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x344);

			//& BG LDO OFF (0x3D0 bit[2] = 0)                              
			writel(readl(pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x3D0) & (~(0x4)), pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x3D0);

			//0x100 bit[6] = 0
			writel(readl(pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x100) & (~(0x40)), pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x100);
		} else {
			if(is_ARMPLL) {
				//0x100 bit[6] = 1
				writel(readl(pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x100) | (0x40), pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x100);
				//& BG LDO OFF (0x3D0 bit[2] = 0)                              
				writel(readl(pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x3D0) | (0x4), pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x3D0);
				udelay( 300);
				// Select band select 0x3C8 bit[7..5] = 0x10 (1000~1399)
				writel(((readl(pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x3C8) & (~(0xE0))) | 0x40), (pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x3C8));
				// de-assert resetb_armpll to 1: Write 0xD1 = 0x10 (0x344) bit[4] = 1		
				writel((readl(pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x344) | (0x10)), (pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x344));
				udelay(500);
				//select ti ARMPLL	
				// Set ARMPLL output. Write 0xD0 = 0x03 (0x340)
				writel(readl(pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x340) | (0x1), pll->cg_base + STBC_BASE_OFFSET + 0x4000 + 0x340);
			}
		}
		nvt_spin_unlock_irqrestore(mux->lock, flags);
	}
#endif

	return 0;
}

static u8 nvt_composite_mux_clk_get_parent(struct clk_hw *hw)
{
	struct clk_mux *cmux = container_of(hw, struct clk_mux, hw);
	struct nvt_composite_mux_clk *mux =
	    container_of(cmux, struct nvt_composite_mux_clk, mux);
	u32 reg_val, index;

	if (mux->mux_reg_offset != 0) {
		reg_val = readl(mux->cg_base + mux->mux_reg_offset);
		index = reg_val & (div_mask(mux->mux_bit_width) << mux->mux_bit_idx);
		return index >> mux->mux_bit_idx;
	}
	return 0;
}

static int nvt_composite_mux_clk_set_phase(struct clk_hw *hw, int enable)
{
	unsigned int reg_val;
	unsigned long flags;
	struct clk_mux *cmux = container_of(hw, struct clk_mux, hw);
	struct nvt_composite_mux_clk *mux =
	    container_of(cmux, struct nvt_composite_mux_clk, mux);

	if (mux->autogating_reg_offset != 0) {
		/* race condition protect. enter critical section */
		nvt_spin_lock_irqsave(mux->lock, flags);
		reg_val = readl(mux->cg_base + mux->autogating_reg_offset);

		if (enable == 0)
			reg_val &= ~(0x1 << mux->autogating_bit_idx);
		else
			reg_val |= (0x1 << mux->autogating_bit_idx);

		writel(reg_val, mux->cg_base + mux->autogating_reg_offset);
		/* race condition protect. leave critical section */
		nvt_spin_unlock_irqrestore(mux->lock, flags);
	}

	return 0;
}

static int nvt_composite_mux_clk_get_phase(struct clk_hw *hw)
{
	unsigned long flags;
	int is_enabled = 0;
	struct clk_mux *cmux = container_of(hw, struct clk_mux, hw);
	struct nvt_composite_mux_clk *mux =
	    container_of(cmux, struct nvt_composite_mux_clk, mux);

	if (mux->autogating_reg_offset != 0) {
		/* race condition protect. enter critical section */
		nvt_spin_lock_irqsave(mux->lock, flags);
		is_enabled = (readl(mux->cg_base + mux->autogating_reg_offset) & (0x1 << mux->autogating_bit_idx)) ? 1 : 0;
		/* race condition protect. leave critical section */
		nvt_spin_unlock_irqrestore(mux->lock, flags);
	}

	return is_enabled;
}

int nvt_composite_mux_clk_register(struct nvt_composite_mux_clk *p_cmux_clk)
{
	struct clk *clk;
	struct clk_init_data init;
	int ret = 0;
	//For RingOSC calibration flow(only for 690)
#ifdef CONFIG_NVT_IVOT_PLAT_NS02201	
	unsigned long flags;
	unsigned long uiReg;
	int input_osc = 12;
	int div_cal = 0;
	int div = 0;
#endif

	/* To avoid being changed on the clock framework */
	unsigned long current_rate = p_cmux_clk->current_rate; 
	struct clk_ops *nvt_composite_mux_clk_ops = kzalloc(sizeof(struct clk_ops), GFP_KERNEL);
	if (!nvt_composite_mux_clk_ops) {
		pr_err("%s: failed to alloc nvt_composite_mux_clk_ops\n", __func__);
		ret = -EPERM;
		goto err;
	}

	nvt_composite_mux_clk_ops->prepare = &nvt_composite_mux_clk_prepare;
	nvt_composite_mux_clk_ops->unprepare = &nvt_composite_mux_clk_unprepare;
	nvt_composite_mux_clk_ops->is_prepared = &nvt_composite_mux_clk_is_prepared;
	nvt_composite_mux_clk_ops->enable = &nvt_composite_mux_clk_enable;
	nvt_composite_mux_clk_ops->disable = &nvt_composite_mux_clk_disable;
	if (p_cmux_clk->gate_reg_offset != 0)
		nvt_composite_mux_clk_ops->is_enabled = &nvt_composite_mux_clk_is_enabled;
	else
		nvt_composite_mux_clk_ops->is_enabled = NULL;
	nvt_composite_mux_clk_ops->round_rate = &nvt_composite_mux_clk_round_rate;
	nvt_composite_mux_clk_ops->recalc_rate = &nvt_composite_mux_clk_recalc_rate;
	nvt_composite_mux_clk_ops->set_rate = &nvt_composite_mux_clk_set_rate;
	nvt_composite_mux_clk_ops->set_parent = &nvt_composite_mux_clk_set_parent;
	nvt_composite_mux_clk_ops->get_parent = &nvt_composite_mux_clk_get_parent;
	nvt_composite_mux_clk_ops->set_phase = &nvt_composite_mux_clk_set_phase;
	nvt_composite_mux_clk_ops->get_phase = &nvt_composite_mux_clk_get_phase;

	p_cmux_clk->mux_flags = CLK_SET_RATE_PARENT | CLK_SET_RATE_NO_REPARENT | CLK_GET_RATE_NOCACHE;

	/* struct clk_mux assignments */
	p_cmux_clk->mux.reg = p_cmux_clk->cg_base;
	p_cmux_clk->mux.shift = p_cmux_clk->mux_bit_idx;
	p_cmux_clk->mux.mask = (u32)(BIT(p_cmux_clk->mux_bit_width) - 1);
	p_cmux_clk->mux.hw.init = &init;
	p_cmux_clk->mux.lock = p_cmux_clk->lock;

	init.name = p_cmux_clk->name;
	init.ops = nvt_composite_mux_clk_ops;
	init.flags = p_cmux_clk->mux_flags;
	init.num_parents = p_cmux_clk->num_parents;
	init.parent_names = p_cmux_clk->parent_names;

#ifdef CONFIG_NVT_IVOT_PLAT_NS02301
	if (strncmp(p_cmux_clk->name, "prfll", strlen("prfll")) == 0) {
		prfll = p_cmux_clk;
	}
#endif

	/* Set parent first, avoid default clock rate exceed max rate on recalc rate */
	if (p_cmux_clk->keep_rate == 0)
		nvt_composite_mux_clk_set_parent(&p_cmux_clk->mux.hw, p_cmux_clk->parent_idx);

	clk = clk_register(NULL, &(p_cmux_clk->mux.hw));
	if (IS_ERR(clk)) {
		pr_err("%s: failed to register clk %s\n",__func__, p_cmux_clk->name);
		ret = -EPERM;
		goto err;
	}

	if (clk_register_clkdev(clk, p_cmux_clk->name, NULL)) {
		pr_err("%s: failed to register lookup %s\n",__func__, p_cmux_clk->name);
		ret = -EPERM;
		goto err;
	}

#ifdef CONFIG_NVT_IVOT_PLAT_NS02302
	if (strncmp(p_cmux_clk->name, "cpu_clk", strlen("cpu_clk")) == 0) {
		if (clk_get_rate(clk) == 480000000) {
			p_cmux_clk->parent_idx = 1;
		} else if (clk_get_rate(clk) == 400000000) {
			p_cmux_clk->parent_idx = 0;
		} else if (clk_get_rate(clk) == 1000000000) {
			p_cmux_clk->parent_idx = 2;
		} else {
			p_cmux_clk->parent_idx = 3;
		}

	}
#endif	
	/* Skip changing clock parent if you want to keep the clock frequency */
	if (p_cmux_clk->keep_rate == 0)
		clk_set_parent(clk, __clk_lookup(p_cmux_clk->parent_names[p_cmux_clk->parent_idx]));

	p_cmux_clk->parent_rate = clk_get_rate(clk_get_parent(clk));

	if (((p_cmux_clk->div_bit_width) && current_rate) && (p_cmux_clk->keep_rate == 0)) {
		if (!is_fastboot) {
			clk_set_rate(clk, current_rate);
		}
	}

	//For RingOSC calibration flow(only for 690)
#ifdef CONFIG_NVT_IVOT_PLAT_NS02201	
	if (strncmp(p_cmux_clk->name, "ro_32k_clk", strlen("ro_32k_clk")) == 0) {
		pr_info("%s: ro_32k_clk parent index= %d parent name %s\n", __func__, p_cmux_clk->parent_idx, p_cmux_clk->parent_names[p_cmux_clk->parent_idx]);
		if (strncmp(p_cmux_clk->parent_names[p_cmux_clk->parent_idx], "osc_ring", strlen("osc_ring")) == 0) {
			
			nvt_spin_lock_irqsave(p_cmux_clk->lock, flags);
			//TRNG_RO clock enable bit
			uiReg = readl(p_cmux_clk->cg_base + 0x7C);
			uiReg |= (0x1<<4);
			writel(uiReg, p_cmux_clk->cg_base + 0x7C);
			//RO clock enable bit[2] = 1
			uiReg = readl(p_cmux_clk->cg_base + 0x140);
			uiReg |= (0x1<<2);
			writel(uiReg, p_cmux_clk->cg_base + 0x140);
			//Detect enable
			uiReg = readl(p_cmux_clk->cg_base + 0x100);
			uiReg |= 0x1;
			writel(uiReg, p_cmux_clk->cg_base + 0x100);
			/*wait done*/
			while(1) {
				uiReg = readl(p_cmux_clk->cg_base + 0x100);
				if ((uiReg & 0x1) == 0x0)
					break;
			}
			//Get calibration value
			uiReg = readl(p_cmux_clk->cg_base + 0x104);
			/*Set divider reuslt*/
			div_cal = 768 / input_osc;
			div = ((uiReg + (0x1 << 5)) / div_cal) - 1;

			writel(div << 4, p_cmux_clk->cg_base + 0x110);			
			nvt_spin_unlock_irqrestore(p_cmux_clk->lock, flags);
//			pr_info("%s: ro_32k_clk parent index= %d parent name %s need calibration base 0x%08x\n", __func__, p_cmux_clk->parent_idx, p_cmux_clk->parent_names[p_cmux_clk->parent_idx], p_cmux_clk->cg_base);
		}
	}
#endif
	p_cmux_clk->current_rate = clk_get_rate(clk);

	/* Increase enable and prepare counts if clock was previously enabled*/
	if (__clk_is_enabled(clk)) {
		ret = clk_prepare_enable(clk);
		if (ret < 0)
			pr_err("%s prepare & enable failed!\n", p_cmux_clk->name);
	}


	if (p_cmux_clk->do_enable == DO_ENABLE) {
		if (!__clk_is_enabled(clk) && !is_fastboot) {
			ret = clk_prepare_enable(clk);
			if (ret < 0)
				pr_err("%s prepare & enable failed!\n", p_cmux_clk->name);
		}
	} else if (p_cmux_clk->do_enable == DO_DISABLE) {
		if (__clk_is_enabled(clk)) {
			clk_disable_unprepare(clk);
		}
	}

	ret = clk_set_phase(clk, p_cmux_clk->do_autogating);
	if (ret < 0)
		pr_err("%s enable autogating failed!\n", p_cmux_clk->name);

err:
	if ((ret != 0) && (nvt_composite_mux_clk_ops != NULL))
		kfree(nvt_composite_mux_clk_ops);

	return ret;
}
EXPORT_SYMBOL(nvt_composite_mux_clk_register);

int nvt_spread_spectrum(struct clk *clock, unsigned int steps, unsigned long ssc_step, unsigned long ssc_period_value)
{
	struct clk_hw *hw = __clk_get_hw(clock);
	unsigned long flags;
	unsigned int reg_val;
	struct nvt_pll_clk *pll;

	if (!hw) {
		pr_err("%s: clock not found \n", __func__);
		return -EINVAL;
	}
	pll = container_of(hw, struct nvt_pll_clk, pll_hw);

	nvt_spin_lock_irqsave(pll->lock, flags);

	/*Disable PLLx*/
	if (steps) {
		reg_val = readl(pll->cg_base + pll->gate_reg_offset);
		reg_val &= ~(0x1 << pll->gate_bit_idx);
		writel(reg_val, pll->cg_base + pll->gate_reg_offset);
	}

	/* Setup period vlaue */
	writel(ssc_period_value, pll->cg_base + pll->rate_reg_offset - SPREAD_SPECTRUM_REG2_MINUS_OFFSET);

	/* Setup MULFAC */
	writel(steps, pll->cg_base + pll->rate_reg_offset - SPREAD_SPECTRUM_REG1_MINUS_OFFSET);

	/* Setup SET_SEL */
	reg_val = readl(pll->cg_base + pll->rate_reg_offset - SPREAD_SPECTRUM_REG0_MINUS_OFFSET);
	reg_val &= ~(0x3 << SPREAD_SPECTRUM_STEP_OFFSET);
	reg_val |= (ssc_step << SPREAD_SPECTRUM_STEP_OFFSET);
	writel(reg_val, pll->cg_base + pll->rate_reg_offset - SPREAD_SPECTRUM_REG0_MINUS_OFFSET);

	/*Write 1 to SSC_NEW_MODE, DSSC*/
	reg_val &= ~((0x1 << SPREAD_SPECTRUM_NEW_MODE_OFFSET) | (0x1 << SPREAD_SPECTRUM_DSSC_OFFSET));

	reg_val |= (0x1 << SPREAD_SPECTRUM_NEW_MODE_OFFSET) | (0x1 << SPREAD_SPECTRUM_DSSC_OFFSET);
	writel(reg_val, pll->cg_base + pll->rate_reg_offset - SPREAD_SPECTRUM_REG0_MINUS_OFFSET);

	/*Write 1 to SSC_EN*/
	reg_val = readl(pll->cg_base + pll->rate_reg_offset - SPREAD_SPECTRUM_REG0_MINUS_OFFSET);
	if (steps == 0) {
		reg_val &= ~0x1;
	} else {
		reg_val |= 0x1;
	}
	writel(reg_val, pll->cg_base + pll->rate_reg_offset - SPREAD_SPECTRUM_REG0_MINUS_OFFSET);

	/*Enable PLLx*/
	if (steps) {
		reg_val = readl(pll->cg_base + pll->gate_reg_offset);
		reg_val |= (0x1 << pll->gate_bit_idx);
		writel(reg_val, pll->cg_base + pll->gate_reg_offset);

		if (!(readl(pll->cg_base + pll->status_reg_offset) & (0x1 << pll->gate_bit_idx)))
			mdelay(3);
	}

	nvt_spin_unlock_irqrestore(pll->lock, flags);
	return 0;
}
EXPORT_SYMBOL(nvt_spread_spectrum);

int nvt_mux_clk_reset(struct clk *clock, bool reset)
{
	unsigned int reg_val;
	unsigned long flags;
	struct clk_mux *cmux;
	struct nvt_composite_mux_clk *mux;
	struct clk_hw *hw = __clk_get_hw(clock);

	if (!hw) {
		pr_err("%s: clock not found \n", __func__);
		return -EINVAL;
	}

	cmux = container_of(hw, struct clk_mux, hw);
	mux = container_of(cmux, struct nvt_composite_mux_clk, mux);

	if (mux->reset_reg_offset != 0) {
		nvt_spin_lock_irqsave(mux->lock, flags);
		/* race condition protect. enter critical section */
		reg_val = readl(mux->cg_base + mux->reset_reg_offset);
		if (reset) {
			reg_val &= ~(0x1 << mux->reset_bit_idx);
			writel(reg_val, mux->cg_base + mux->reset_reg_offset);
			udelay(10);
		} else {
			reg_val |= 0x1 << mux->reset_bit_idx;
			writel(reg_val, mux->cg_base + mux->reset_reg_offset);
			udelay(10);
		}
		/* race condition protect. leave critical section */
		nvt_spin_unlock_irqrestore(mux->lock, flags);
	}
	return 0;
}
EXPORT_SYMBOL(nvt_mux_clk_reset);

void nvt_clkmux_pm_suspend(struct nvt_composite_mux_clk *mux)
{
	if (!console_suspend_enabled && (strstr(mux->name, "uart") != NULL))
		return;

	mux->parent_idx = nvt_composite_mux_clk_get_parent(&mux->mux.hw);
	mux->do_autogating = nvt_composite_mux_clk_get_phase(&mux->mux.hw);
	if (mux->gate_reg_offset != 0) {
		mux->is_enabled = nvt_composite_mux_clk_is_enabled(&mux->mux.hw);
		if (mux->is_enabled) {
			nvt_composite_mux_clk_disable(&mux->mux.hw);
			nvt_composite_mux_clk_unprepare(&mux->mux.hw);
		}
	}
}

void nvt_clkpll_pm_suspend(struct nvt_pll_clk *pll)
{
	pll->is_enabled = nvt_pll_clk_is_enabled(&pll->pll_hw);
	if (pll->is_enabled) {
		if (!pll->pll_fixed) {
			nvt_pll_clk_disable(&pll->pll_hw);
		}
	}
}

void nvt_clkmux_pm_resume(struct nvt_composite_mux_clk *mux)
{
	if (!console_suspend_enabled && (strstr(mux->name, "uart") != NULL))
		return;

	if (nvt_composite_mux_clk_set_parent(&mux->mux.hw, mux->parent_idx))
		return;

	if (nvt_composite_mux_clk_set_phase(&mux->mux.hw, mux->do_autogating))
		return;

	if (nvt_composite_mux_clk_set_rate(&mux->mux.hw, mux->current_rate, mux->parent_rate))
		return;

	if (!mux->is_enabled)
		return;

	if (nvt_composite_mux_clk_prepare(&mux->mux.hw))
		return;

	if (nvt_composite_mux_clk_enable(&mux->mux.hw))
		return;
}

void nvt_clkpll_pm_resume(struct nvt_pll_clk *pll)
{
	if (nvt_pll_clk_set_rate(&pll->pll_hw, pll->current_rate, 12000000))
		return;

	if (!pll->is_enabled)
		return;

	if (nvt_pll_clk_enable(&pll->pll_hw))
		return;
}
