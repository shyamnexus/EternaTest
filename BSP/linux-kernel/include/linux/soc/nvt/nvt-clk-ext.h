/**
    NVT clock external api header file
    This file will provide NVT clock external API
    @file       nvt-clk-ext.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#ifndef __NVT_CLK_EXT_H__
#define __NVT_CLK_EXT_H__
int nvt_mux_clk_reset(struct clk *clock, bool reset);
int nvt_spread_spectrum(struct clk *clock, unsigned int steps, unsigned long ssc_step, unsigned long ssc_period_value);

#ifdef CONFIG_NVT_IVOT_PLAT_NS02301
int pll_get_fll_is_6m(void);
void FLL_change_to_6M(unsigned int en);
#endif

#endif
