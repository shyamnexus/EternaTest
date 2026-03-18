/**
    NVT TrustZone Protection Controller
    We define the nvt 51055 based Protection Controller register here.
    @file           nvt_51055_tzpc.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2020.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the SPDX-License-Identifier: BSD-2-Clause as
    published by the Free Software Foundation.
*/

#ifndef __DRIVERS_PLL_H
#define __DRIVERS_PLL_H

#include <stdint.h>
#include <trace_levels.h>
#include <types_ext.h>
#include <util.h>
#include <plat/io_address.h>
#include <kwrap/nvt_type.h>

#include "nvt_sce_cfg.h"

extern ER pll_set_pll_enable(PLL_ID id, BOOL b_enable);
extern BOOL pll_get_pll_enable(PLL_ID id);

extern void pll_enable_clock(CG_EN num);
extern void pll_get_trng_ro_sel(uint32_t *pui_trng_ro_sel, uint32_t *pui_divider);

#if defined(SCE_USE_STBC_CLK) || defined(TRNG_USE_STBC_CLK)
extern ER pll_set_STBC_pll_enable(PLL_ID id, BOOL b_enable);
#endif

#endif /* __DRIVERS_PLL_H */
