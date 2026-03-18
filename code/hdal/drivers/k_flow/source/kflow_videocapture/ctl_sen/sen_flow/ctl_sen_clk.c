/*
   sensor control - clock

    @file       ctl_sen_clk.c
    @ingroup
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
*/
#include "ctl_sen_clk_int.h"

INT32 __set_clk_uti(CTL_SEN_UTI_CLK param_id, UINT32 p_param)
{
	INT32 rt = CTL_SEN_E_OK;

	switch (param_id) {
	case CTL_SEN_UTI_CLK_SIEMCLK_ENABLE: {
#ifndef __KERNEL__

			if ((p_param) > 0) {
				pll_enable_clock(SIE_MCLK);
			} else {
				pll_disable_clock(SIE_MCLK);
			}
#else
			struct clk *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			if ((p_param) > 0) {
				clk_prepare(src_clk);
				clk_enable(src_clk);
			} else {
				clk_disable(src_clk);
				clk_unprepare(src_clk);
			}

			clk_put(src_clk);
#endif
		}
		break;
	case CTL_SEN_UTI_CLK_SIEMCLK2_ENABLE: {
#ifndef __KERNEL__

			if ((p_param) > 0) {
				pll_enable_clock(SIE_MCLK2);
			} else {
				pll_disable_clock(SIE_MCLK2);
			}
#else
			struct clk *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK2);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			if ((p_param) > 0) {
				clk_prepare(src_clk);
				clk_enable(src_clk);
			} else {
				clk_disable(src_clk);
				clk_unprepare(src_clk);
			}

			clk_put(src_clk);
#endif
		}
		break;
	case CTL_SEN_UTI_CLK_SIEMCLK3_ENABLE: {
#ifndef __KERNEL__

			if ((p_param) > 0) {
				pll_enable_clock(SIE_MCLK3);
			} else {
				pll_disable_clock(SIE_MCLK3);
			}
#else
			struct clk *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK3);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			if ((p_param) > 0) {
				clk_prepare(src_clk);
				clk_enable(src_clk);
			} else {
				clk_disable(src_clk);
				clk_unprepare(src_clk);
			}

			clk_put(src_clk);
#endif
		}
		break;
	case CTL_SEN_UTI_CLK_SIEMCLK4_ENABLE: {
#ifndef __KERNEL__

			if ((p_param) > 0) {
				pll_enable_clock(SIE_MCLK4);
			} else {
				pll_disable_clock(SIE_MCLK4);
			}
#else
			struct clk *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK4);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			if ((p_param) > 0) {
				clk_prepare(src_clk);
				clk_enable(src_clk);
			} else {
				clk_disable(src_clk);
				clk_unprepare(src_clk);
			}

			clk_put(src_clk);
#endif
		}
		break;

#if 0 // 538 remove
	case CTL_SEN_UTI_CLK_SIEMCLK5_ENABLE: {
#ifndef __KERNEL__

			if ((p_param) > 0) {
				pll_enable_clock(SIE_MCLK5);
			} else {
				pll_disable_clock(SIE_MCLK5);
			}
#else
			struct clk *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK5);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			if ((p_param) > 0) {
				clk_prepare(src_clk);
				clk_enable(src_clk);
			} else {
				clk_disable(src_clk);
				clk_unprepare(src_clk);
			}

			clk_put(src_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_SIEMCLK6_ENABLE: {
#ifndef __KERNEL__

			if ((p_param) > 0) {
				pll_enable_clock(SENSOR_MCLK6);
			} else {
				pll_disable_clock(SENSOR_MCLK6);
			}
#else
			struct clk *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK6);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			if ((p_param) > 0) {
				clk_prepare(src_clk);
				clk_enable(src_clk);
			} else {
				clk_disable(src_clk);
				clk_unprepare(src_clk);
			}

			clk_put(src_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_SIEMCLK7_ENABLE: {
#ifndef __KERNEL__

			if ((p_param) > 0) {
				pll_enable_clock(SENSOR_MCLK7);
			} else {
				pll_disable_clock(SENSOR_MCLK7);
			}
#else
			struct clk *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK7);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			if ((p_param) > 0) {
				clk_prepare(src_clk);
				clk_enable(src_clk);
			} else {
				clk_disable(src_clk);
				clk_unprepare(src_clk);
			}

			clk_put(src_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_SIEMCLK8_ENABLE: {
#ifndef __KERNEL__

			if ((p_param) > 0) {
				pll_enable_clock(SENSOR_MCLK8);
			} else {
				pll_disable_clock(SENSOR_MCLK8);
			}
#else
			struct clk *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK8);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			if ((p_param) > 0) {
				clk_prepare(src_clk);
				clk_enable(src_clk);
			} else {
				clk_disable(src_clk);
				clk_unprepare(src_clk);
			}

			clk_put(src_clk);
#endif
		}
		break;
#endif

	case CTL_SEN_UTI_CLK_SPCLK_ENABLE: {
#ifndef __KERNEL__

			if ((p_param) > 0) {
				pll_enable_clock(SP_CLK);
			} else {
				pll_disable_clock(SP_CLK);
			}
#else
			struct clk *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_SPCLK);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			if ((p_param) > 0) {
				clk_prepare(src_clk);
				clk_enable(src_clk);
			} else {
				clk_disable(src_clk);
				clk_unprepare(src_clk);
			}

			clk_put(src_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_SPCLK2_ENABLE: {
#ifndef __KERNEL__

			if ((p_param) > 0) {
				pll_enable_clock(SP2_CLK);
			} else {
				pll_disable_clock(SP2_CLK);
			}
#else
			struct clk *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_SPCLK2);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			if ((p_param) > 0) {
				clk_prepare(src_clk);
				clk_enable(src_clk);
			} else {
				clk_disable(src_clk);
				clk_unprepare(src_clk);
			}

			clk_put(src_clk);
#endif
		}
		break;
	case CTL_SEN_UTI_CLK_SIEMCLK_SOURCE: {
#ifndef __KERNEL__
			if ((p_param) == CTL_SEN_CLKSRC_480) {
				pll_set_clock_rate(PLL_CLKSEL_SIE_MCLKSRC, PLL_CLKSEL_SIE_MCLKSRC_480);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL5) {
				pll_set_clock_rate(PLL_CLKSEL_SIE_MCLKSRC, PLL_CLKSEL_SIE_MCLKSRC_PLL5);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL6) {
				pll_set_clock_rate(PLL_CLKSEL_SIE_MCLKSRC, PLL_CLKSEL_SIE_MCLKSRC_PLL6);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL12) {
				pll_set_clock_rate(PLL_CLKSEL_SIE_MCLKSRC, PLL_CLKSEL_SIE_MCLKSRC_PLL12);
			} else {
				ctl_sen_dbg_err("not support %d=%d\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}
#else
			struct clk *parent_clk = NULL, *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			if ((p_param) == CTL_SEN_CLKSRC_480) {
				parent_clk = clk_get(NULL, CG_STR_FIX480);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL5) {
				parent_clk = clk_get(NULL, CG_STR_PLL5);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL6) {
				parent_clk = clk_get(NULL, CG_STR_PLL6);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL12) {
				parent_clk = clk_get(NULL, CG_STR_PLL12);
			} else {
				ctl_sen_dbg_err("not support %d=%d\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}

			if (!IS_ERR(parent_clk)) {
				clk_set_parent(src_clk, parent_clk);
			} else {
				ctl_sen_dbg_err("%d,%d: get parent clk fail\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}

			clk_put(parent_clk);
			clk_put(src_clk);
#endif
		}
		break;
	case CTL_SEN_UTI_CLK_SIEMCLK2_SOURCE: {
#ifndef __KERNEL__
			if ((p_param) == CTL_SEN_CLKSRC_480) {
				pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK2SRC, PLL_CLKSEL_SIE_MCLK2SRC_480);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL5) {
				pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK2SRC, PLL_CLKSEL_SIE_MCLK2SRC_PLL5);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL6) {
				pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK2SRC, PLL_CLKSEL_SIE_MCLK2SRC_PLL6);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL12) {
				pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK2SRC, PLL_CLKSEL_SIE_MCLK2SRC_PLL12);
			} else {
				ctl_sen_dbg_err("not support %d=%d\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}
#else
			struct clk *parent_clk = NULL, *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK2);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			if ((p_param) == CTL_SEN_CLKSRC_480) {
				parent_clk = clk_get(NULL, CG_STR_FIX480);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL5) {
				parent_clk = clk_get(NULL, CG_STR_PLL5);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL6) {
				parent_clk = clk_get(NULL, CG_STR_PLL6);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL12) {
				parent_clk = clk_get(NULL, CG_STR_PLL12);
			} else {
				ctl_sen_dbg_err("not support %d=%d\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}

			if (!IS_ERR(parent_clk)) {
				clk_set_parent(src_clk, parent_clk);
			} else {
				ctl_sen_dbg_err("%d,%d: get parent clk fail\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_SYS;
			}

			clk_put(parent_clk);
			clk_put(src_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_SIEMCLK3_SOURCE: {
#ifndef __KERNEL__
			if ((p_param) == CTL_SEN_CLKSRC_480) {
				pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK3SRC, PLL_CLKSEL_SIE_MCLK3SRC_480);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL5) {
				pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK3SRC, PLL_CLKSEL_SIE_MCLK3SRC_PLL5);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL6) {
				pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK3SRC, PLL_CLKSEL_SIE_MCLK3SRC_PLL6);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL12) {
				pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK3SRC, PLL_CLKSEL_SIE_MCLK3SRC_PLL12);
			} else {
				ctl_sen_dbg_err("not support %d=%d\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}
#else
			struct clk *parent_clk = NULL, *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK3);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			if ((p_param) == CTL_SEN_CLKSRC_480) {
				parent_clk = clk_get(NULL, CG_STR_FIX480);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL5) {
				parent_clk = clk_get(NULL, CG_STR_PLL5);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL6) {
				parent_clk = clk_get(NULL, CG_STR_PLL6);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL12) {
				parent_clk = clk_get(NULL, CG_STR_PLL12);
			} else {
				ctl_sen_dbg_err("not support %d=%d\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}

			if (!IS_ERR(parent_clk)) {
				clk_set_parent(src_clk, parent_clk);
			} else {
				ctl_sen_dbg_err("%d,%d: get parent clk fail\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_SYS;
			}

			clk_put(parent_clk);
			clk_put(src_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_SIEMCLK4_SOURCE: {
#ifndef __KERNEL__
			if ((p_param) == CTL_SEN_CLKSRC_480) {
				pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK4SRC, PLL_CLKSEL_SIE_MCLK4SRC_480);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL5) {
				pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK4SRC, PLL_CLKSEL_SIE_MCLK4SRC_PLL5);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL6) {
				pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK4SRC, PLL_CLKSEL_SIE_MCLK4SRC_PLL6);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL12) {
				pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK4SRC, PLL_CLKSEL_SIE_MCLK4SRC_PLL12);
			} else {
				ctl_sen_dbg_err("not support %d=%d\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}
#else
			struct clk *parent_clk = NULL, *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK4);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			if ((p_param) == CTL_SEN_CLKSRC_480) {
				parent_clk = clk_get(NULL, CG_STR_FIX480);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL5) {
				parent_clk = clk_get(NULL, CG_STR_PLL5);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL6) {
				parent_clk = clk_get(NULL, CG_STR_PLL6);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL12) {
				parent_clk = clk_get(NULL, CG_STR_PLL12);
			} else {
				ctl_sen_dbg_err("not support %d=%d\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}

			if (!IS_ERR(parent_clk)) {
				clk_set_parent(src_clk, parent_clk);
			} else {
				ctl_sen_dbg_err("%d,%d: get parent clk fail\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_SYS;
			}

			clk_put(parent_clk);
			clk_put(src_clk);
#endif
		}
		break;

#if 0 // 538 remove
	case CTL_SEN_UTI_CLK_SIEMCLK5_SOURCE: {
#ifndef __KERNEL__
			if ((p_param) == CTL_SEN_CLKSRC_480) {
				pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK5SRC, PLL_CLKSEL_SIE_MCLK5SRC_480);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL5) {
				pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK5SRC, PLL_CLKSEL_SIE_MCLK5SRC_PLL5);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL11) {
				pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK5SRC, PLL_CLKSEL_SIE_MCLK5SRC_PLL11);
			} else {
				ctl_sen_dbg_err("not support %d=%d\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}
#else
			struct clk *parent_clk = NULL, *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK5);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			if ((p_param) == CTL_SEN_CLKSRC_480) {
				parent_clk = clk_get(NULL, CG_STR_FIX480);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL5) {
				parent_clk = clk_get(NULL, CG_STR_PLL5);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL11) {
				parent_clk = clk_get(NULL, CG_STR_PLL11);
			} else {
				ctl_sen_dbg_err("not support %d=%d\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}

			if (!IS_ERR(parent_clk)) {
				clk_set_parent(src_clk, parent_clk);
			} else {
				ctl_sen_dbg_err("%d,%d: get parent clk fail\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_SYS;
			}

			clk_put(parent_clk);
			clk_put(src_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_SIEMCLK6_SOURCE: {
#ifndef __KERNEL__
			if ((p_param) == CTL_SEN_CLKSRC_480) {
				pll_set_clock_rate(PLL_CLKSEL_SENSOR_MCLK6SRC, PLL_CLKSEL_SENSOR_MCLK6SRC_480);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL20) {
				pll_set_clock_rate(PLL_CLKSEL_SENSOR_MCLK6SRC, PLL_CLKSEL_SENSOR_MCLK6SRC_PLL20);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL23) {
				pll_set_clock_rate(PLL_CLKSEL_SENSOR_MCLK6SRC, PLL_CLKSEL_SENSOR_MCLK6SRC_PLL23);
			} else {
				ctl_sen_dbg_err("not support %d=%d\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}
#else
			struct clk *parent_clk = NULL, *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK6);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			if ((p_param) == CTL_SEN_CLKSRC_480) {
				parent_clk = clk_get(NULL, CG_STR_FIX480);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL20) {
				parent_clk = clk_get(NULL, CG_STR_PLL20);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL23) {
				parent_clk = clk_get(NULL, CG_STR_PLL23);
			} else {
				ctl_sen_dbg_err("not support %d=%d\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}

			if (!IS_ERR(parent_clk)) {
				clk_set_parent(src_clk, parent_clk);
			} else {
				ctl_sen_dbg_err("%d,%d: get parent clk fail\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_SYS;
			}

			clk_put(parent_clk);
			clk_put(src_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_SIEMCLK7_SOURCE: {
#ifndef __KERNEL__
			if ((p_param) == CTL_SEN_CLKSRC_480) {
				pll_set_clock_rate(PLL_CLKSEL_SENSOR_MCLK7SRC, PLL_CLKSEL_SENSOR_MCLK7SRC_480);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL20) {
				pll_set_clock_rate(PLL_CLKSEL_SENSOR_MCLK7SRC, PLL_CLKSEL_SENSOR_MCLK7SRC_PLL20);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL23) {
				pll_set_clock_rate(PLL_CLKSEL_SENSOR_MCLK7SRC, PLL_CLKSEL_SENSOR_MCLK7SRC_PLL23);
			} else {
				ctl_sen_dbg_err("not support %d=%d\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}
#else
			struct clk *parent_clk = NULL, *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK7);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			if ((p_param) == CTL_SEN_CLKSRC_480) {
				parent_clk = clk_get(NULL, CG_STR_FIX480);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL20) {
				parent_clk = clk_get(NULL, CG_STR_PLL20);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL23) {
				parent_clk = clk_get(NULL, CG_STR_PLL23);
			} else {
				ctl_sen_dbg_err("not support %d=%d\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}

			if (!IS_ERR(parent_clk)) {
				clk_set_parent(src_clk, parent_clk);
			} else {
				ctl_sen_dbg_err("%d,%d: get parent clk fail\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_SYS;
			}

			clk_put(parent_clk);
			clk_put(src_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_SIEMCLK8_SOURCE: {
#ifndef __KERNEL__
			if ((p_param) == CTL_SEN_CLKSRC_480) {
				pll_set_clock_rate(PLL_CLKSEL_SENSOR_MCLK8SRC, PLL_CLKSEL_SENSOR_MCLK8SRC_480);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL20) {
				pll_set_clock_rate(PLL_CLKSEL_SENSOR_MCLK8SRC, PLL_CLKSEL_SENSOR_MCLK8SRC_PLL20);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL23) {
				pll_set_clock_rate(PLL_CLKSEL_SENSOR_MCLK8SRC, PLL_CLKSEL_SENSOR_MCLK8SRC_PLL23);
			} else {
				ctl_sen_dbg_err("not support %d=%d\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}
#else
			struct clk *parent_clk = NULL, *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK8);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			if ((p_param) == CTL_SEN_CLKSRC_480) {
				parent_clk = clk_get(NULL, CG_STR_FIX480);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL20) {
				parent_clk = clk_get(NULL, CG_STR_PLL20);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL23) {
				parent_clk = clk_get(NULL, CG_STR_PLL23);
			} else {
				ctl_sen_dbg_err("not support %d=%d\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}

			if (!IS_ERR(parent_clk)) {
				clk_set_parent(src_clk, parent_clk);
			} else {
				ctl_sen_dbg_err("%d,%d: get parent clk fail\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_SYS;
			}

			clk_put(parent_clk);
			clk_put(src_clk);
#endif
		}
		break;
#endif

	case CTL_SEN_UTI_CLK_SPCLK_SOURCE: {
#ifndef __KERNEL__
			if ((p_param) == CTL_SEN_CLKSRC_480) {
				pll_set_clock_rate(PLL_CLKSEL_SP, PLL_CLKSEL_SP_480);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL4) {
				pll_set_clock_rate(PLL_CLKSEL_SP, PLL_CLKSEL_SP_PLL4);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL5) {
				pll_set_clock_rate(PLL_CLKSEL_SP, PLL_CLKSEL_SP_PLL5);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL6) {
				pll_set_clock_rate(PLL_CLKSEL_SP, PLL_CLKSEL_SP_PLL6);
			} else {
				ctl_sen_dbg_err("not support %d=%d\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}
#else
			struct clk *parent_clk = NULL, *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_SPCLK);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
			}

			if ((p_param) == CTL_SEN_CLKSRC_480) {
				parent_clk = clk_get(NULL, CG_STR_FIX480);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL4) {
				parent_clk = clk_get(NULL, CG_STR_PLL4);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL5) {
				parent_clk = clk_get(NULL, CG_STR_PLL5);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL6) {
				parent_clk = clk_get(NULL, CG_STR_PLL6);
			} else {
				ctl_sen_dbg_err("not support %d=%d\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}

			if (!IS_ERR(parent_clk)) {
				clk_set_parent(src_clk, parent_clk);
			} else {
				ctl_sen_dbg_err("%d,%d: get parent clk fail\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_SYS;
			}

			clk_put(parent_clk);
			clk_put(src_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_SPCLK2_SOURCE: {
#ifndef __KERNEL__
			if ((p_param) == CTL_SEN_CLKSRC_480) {
				pll_set_clock_rate(PLL_CLKSEL_SP2, PLL_CLKSEL_SP2_480);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL4) {
				pll_set_clock_rate(PLL_CLKSEL_SP2, PLL_CLKSEL_SP2_PLL4);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL5) {
				pll_set_clock_rate(PLL_CLKSEL_SP2, PLL_CLKSEL_SP2_PLL5);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL6) {
				pll_set_clock_rate(PLL_CLKSEL_SP2, PLL_CLKSEL_SP2_PLL6);
			} else {
				ctl_sen_dbg_err("not support %d=%d\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}
#else
			struct clk *parent_clk = NULL, *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_SPCLK2);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
			}

			if ((p_param) == CTL_SEN_CLKSRC_480) {
				parent_clk = clk_get(NULL, CG_STR_FIX480);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL4) {
				parent_clk = clk_get(NULL, CG_STR_PLL4);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL5) {
				parent_clk = clk_get(NULL, CG_STR_PLL5);
			} else if ((p_param) == CTL_SEN_CLKSRC_PLL6) {
				parent_clk = clk_get(NULL, CG_STR_PLL6);
			} else {
				ctl_sen_dbg_err("not support %d=%d\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_IN_PARAM;
			}

			if (!IS_ERR(parent_clk)) {
				clk_set_parent(src_clk, parent_clk);
			} else {
				ctl_sen_dbg_err("%d,%d: get parent clk fail\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_SYS;
			}

			clk_put(parent_clk);
			clk_put(src_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_SIEMCLK_FREQUENCY: {
#ifndef __KERNEL__
			pll_set_clock_freq(SIEMCLK_FREQ, p_param);
#else
			struct clk *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			clk_set_rate(src_clk, (ULONG)p_param);

			clk_put(src_clk);
#endif
		}
		break;
	case CTL_SEN_UTI_CLK_SIEMCLK2_FREQUENCY: {
#ifndef __KERNEL__
			pll_set_clock_freq(SIEMCLK2_FREQ, p_param);
#else
			struct clk *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK2);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			clk_set_rate(src_clk, (ULONG)p_param);

			clk_put(src_clk);
#endif
		}
		break;
	case CTL_SEN_UTI_CLK_SIEMCLK3_FREQUENCY: {
#ifndef __KERNEL__
			pll_set_clock_freq(SIEMCLK3_FREQ, p_param);
#else
			struct clk *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK3);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			clk_set_rate(src_clk, (ULONG)p_param);

			clk_put(src_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_SIEMCLK4_FREQUENCY: {
#ifndef __KERNEL__
			pll_set_clock_freq(SIEMCLK4_FREQ, p_param);
#else
			struct clk *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK4);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			clk_set_rate(src_clk, (ULONG)p_param);

			clk_put(src_clk);
#endif
		}
		break;

#if 0 // 538 remove
	case CTL_SEN_UTI_CLK_SIEMCLK5_FREQUENCY: {
#ifndef __KERNEL__
			pll_set_clock_freq(SIEMCLK5_FREQ, p_param);
#else
			struct clk *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK5);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			clk_set_rate(src_clk, (ULONG)p_param);

			clk_put(src_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_SIEMCLK6_FREQUENCY: {
#ifndef __KERNEL__
			pll_set_clock_freq(SENSORMCLK6_FREQ, p_param);
#else
			struct clk *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK6);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			clk_set_rate(src_clk, (ULONG)p_param);

			clk_put(src_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_SIEMCLK7_FREQUENCY: {
#ifndef __KERNEL__
			pll_set_clock_freq(SENSORMCLK7_FREQ, p_param);
#else
			struct clk *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK7);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			clk_set_rate(src_clk, (ULONG)p_param);

			clk_put(src_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_SIEMCLK8_FREQUENCY: {
#ifndef __KERNEL__
			pll_set_clock_freq(SENSORMCLK8_FREQ, p_param);
#else
			struct clk *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_MCLK8);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			clk_set_rate(src_clk, (ULONG)p_param);

			clk_put(src_clk);
#endif
		}
		break;
#endif

	case CTL_SEN_UTI_CLK_SPCLK_FREQUENCY: {
#ifndef __KERNEL__
			pll_set_clock_freq(SPCLK_FREQ, p_param);
#else
			struct clk *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_SPCLK);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			clk_set_rate(src_clk, (ULONG)p_param);

			clk_put(src_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_SPCLK2_FREQUENCY: {
#ifndef __KERNEL__
			pll_set_clock_freq(SPCLK2_FREQ, p_param);
#else
			struct clk *src_clk = NULL;

			src_clk = clk_get(NULL, CG_STR_SPCLK2);
			if (IS_ERR(src_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			clk_set_rate(src_clk, (ULONG)p_param);

			clk_put(src_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_PLL4_ENABLE: {
#ifndef __KERNEL__
			pll_set_pll_enable(PLL_ID_4, (p_param) > 0);
#endif
		}
		break;
	case CTL_SEN_UTI_CLK_PLL5_ENABLE: {
#ifndef __KERNEL__
			pll_set_pll_enable(PLL_ID_5, (p_param) > 0);
#endif
		}
		break;
	case CTL_SEN_UTI_CLK_PLL6_ENABLE: {
#ifndef __KERNEL__
			pll_set_pll_enable(PLL_ID_6, (p_param) > 0);
#endif
		}
		break;
	case CTL_SEN_UTI_CLK_PLL12_ENABLE: {
#ifndef __KERNEL__
			pll_set_pll_enable(PLL_ID_12, (p_param) > 0);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_PLL4_FREQUENCY: {
#ifndef __KERNEL__
			pll_set_pll_freq(PLL_ID_4, p_param);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_PLL4);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d,%d: get parent clk fail\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_SYS;
			}

			clk_set_rate(parent_clk, (ULONG)p_param);
			clk_put(parent_clk);
#endif
		}
		break;
	case CTL_SEN_UTI_CLK_PLL5_FREQUENCY: {
#ifndef __KERNEL__
			pll_set_pll_freq(PLL_ID_5, p_param);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_PLL5);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d,%d: get parent clk fail\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_SYS;
			}

			clk_set_rate(parent_clk, (ULONG)p_param);
			clk_put(parent_clk);
#endif
		}
		break;
	case CTL_SEN_UTI_CLK_PLL6_FREQUENCY: {
#ifndef __KERNEL__
			pll_set_pll_freq(PLL_ID_6, p_param);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_PLL6);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d,%d: get parent clk fail\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_SYS;
			}

			clk_set_rate(parent_clk, (ULONG)p_param);
			clk_put(parent_clk);
#endif
		}
		break;
	case CTL_SEN_UTI_CLK_PLL12_FREQUENCY: {
#ifndef __KERNEL__
			pll_set_pll_freq(PLL_ID_12, p_param);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_PLL12);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d,%d: get parent clk fail\r\n", (UINT32)param_id, p_param);
				rt = CTL_SEN_E_SYS;
			}

			clk_set_rate(parent_clk, (ULONG)p_param);
			clk_put(parent_clk);
#endif
		}
		break;
	default:
		ctl_sen_dbg_err("Err param_id 0x%08X\r\n", param_id);
		return CTL_SEN_E_IN_PARAM;
	}
	return rt;
}

INT32 __get_clk_uti(CTL_SEN_UTI_CLK param_id, VOID *p_param)
{
	UINT32 *ret;
	INT32 rt = CTL_SEN_E_OK;

	ret = (UINT32 *)p_param;

	switch (param_id) {
	case CTL_SEN_UTI_CLK_SIEMCLK_ENABLE: {
#ifndef __KERNEL__
			*ret = pll_is_clock_enabled(SIE_MCLK);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_MCLK);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			*ret = __clk_is_enabled(parent_clk);
			clk_put(parent_clk);
#endif
	}
	break;

	case CTL_SEN_UTI_CLK_SIEMCLK2_ENABLE: {
#ifndef __KERNEL__
			*ret = pll_is_clock_enabled(SIE_MCLK2);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_MCLK2);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			*ret = __clk_is_enabled(parent_clk);
			clk_put(parent_clk);
#endif
	}
	break;

	case CTL_SEN_UTI_CLK_SIEMCLK3_ENABLE: {
#ifndef __KERNEL__
			*ret = pll_is_clock_enabled(SIE_MCLK3);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_MCLK3);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			*ret = __clk_is_enabled(parent_clk);
			clk_put(parent_clk);
#endif
	}
	break;

	case CTL_SEN_UTI_CLK_SIEMCLK4_ENABLE: {
#ifndef __KERNEL__
			*ret = pll_is_clock_enabled(SIE_MCLK4);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_MCLK4);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			*ret = __clk_is_enabled(parent_clk);
			clk_put(parent_clk);
#endif
	}
	break;

#if 0 // 538 remove
	case CTL_SEN_UTI_CLK_SIEMCLK5_ENABLE: {
#ifndef __KERNEL__
			*ret = pll_is_clock_enabled(SIE_MCLK5);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_MCLK5);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			*ret = __clk_is_enabled(parent_clk);
			clk_put(parent_clk);
#endif
	}
	break;

	case CTL_SEN_UTI_CLK_SIEMCLK6_ENABLE: {
#ifndef __KERNEL__
			*ret = pll_is_clock_enabled(SENSOR_MCLK6);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_MCLK6);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			*ret = __clk_is_enabled(parent_clk);
			clk_put(parent_clk);
#endif
	}
	break;

	case CTL_SEN_UTI_CLK_SIEMCLK7_ENABLE: {
#ifndef __KERNEL__
			*ret = pll_is_clock_enabled(SENSOR_MCLK7);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_MCLK7);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			*ret = __clk_is_enabled(parent_clk);
			clk_put(parent_clk);
#endif
	}
	break;

	case CTL_SEN_UTI_CLK_SIEMCLK8_ENABLE: {
#ifndef __KERNEL__
			*ret = pll_is_clock_enabled(SENSOR_MCLK8);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_MCLK8);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			*ret = __clk_is_enabled(parent_clk);
			clk_put(parent_clk);
#endif
	}
	break;
#endif

	case CTL_SEN_UTI_CLK_SPCLK_ENABLE: {
#ifndef __KERNEL__
			*ret = pll_is_clock_enabled(SP_CLK);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_SPCLK);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			*ret = __clk_is_enabled(parent_clk);
			clk_put(parent_clk);
#endif
	}
	break;

	case CTL_SEN_UTI_CLK_SPCLK2_ENABLE: {
#ifndef __KERNEL__
			*ret = pll_is_clock_enabled(SP2_CLK);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_SPCLK2);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			*ret = __clk_is_enabled(parent_clk);
			clk_put(parent_clk);
#endif
	}
	break;

	case CTL_SEN_UTI_CLK_PLL4_FREQUENCY: {
#ifndef __KERNEL__
			*ret = pll_get_pll_freq(PLL_ID_4);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_PLL4);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			*ret = clk_get_rate(parent_clk);
			clk_put(parent_clk);
#endif
		}
		break;
	case CTL_SEN_UTI_CLK_PLL5_FREQUENCY: {
#ifndef __KERNEL__
			*ret = pll_get_pll_freq(PLL_ID_5);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_PLL5);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			*ret = clk_get_rate(parent_clk);
			clk_put(parent_clk);
#endif
		}
		break;
	case CTL_SEN_UTI_CLK_PLL6_FREQUENCY: {
#ifndef __KERNEL__
			*ret = pll_get_pll_freq(PLL_ID_6);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_PLL6);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			*ret = clk_get_rate(parent_clk);
			clk_put(parent_clk);
#endif
		}
		break;
	case CTL_SEN_UTI_CLK_PLL12_FREQUENCY: {
#ifndef __KERNEL__
			*ret = pll_get_pll_freq(PLL_ID_12);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_PLL12);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			*ret = clk_get_rate(parent_clk);
			clk_put(parent_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_PLL4_ENABLE: {
#ifndef __KERNEL__
			*ret = pll_get_pll_enable(PLL_ID_4);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_PLL4);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			*ret = __clk_is_enabled(parent_clk);

			clk_put(parent_clk);
#endif
		}
		break;
	case CTL_SEN_UTI_CLK_PLL5_ENABLE: {
#ifndef __KERNEL__
			*ret = pll_get_pll_enable(PLL_ID_5);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_PLL5);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			*ret = __clk_is_enabled(parent_clk);

			clk_put(parent_clk);
#endif
		}
		break;
	case CTL_SEN_UTI_CLK_PLL6_ENABLE: {
#ifndef __KERNEL__
			*ret = pll_get_pll_enable(PLL_ID_6);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_PLL6);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			*ret = __clk_is_enabled(parent_clk);

			clk_put(parent_clk);
#endif
		}
		break;
	case CTL_SEN_UTI_CLK_PLL12_ENABLE: {
#ifndef __KERNEL__
			*ret = pll_get_pll_enable(PLL_ID_12);
#else
			struct clk *parent_clk = NULL;

			parent_clk = clk_get(NULL, CG_STR_PLL12);
			if (IS_ERR(parent_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			*ret = __clk_is_enabled(parent_clk);

			clk_put(parent_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_MIPILVDS_FREQUENCY: {
#ifndef __KERNEL__
			if (pll_get_clock_rate(PLL_CLKSEL_CSI) == PLL_CLKSEL_CSI_60) {
				*ret = CTL_SEN_60M_HZ;
			} else if (pll_get_clock_rate(PLL_CLKSEL_CSI) == PLL_CLKSEL_CSI_120) {
				*ret = CTL_SEN_120M_HZ;
			} else if (pll_get_clock_rate(PLL_CLKSEL_CSI) == PLL_CLKSEL_CSI_240) {
				*ret = CTL_SEN_240M_HZ;
			} else {
				ctl_sen_dbg_err("%d: get clk outofrange 0x%lx\r\n", (UINT32)param_id, (ULONG)pll_get_clock_rate(PLL_CLKSEL_CSI));
				*ret = CTL_SEN_60M_HZ;
				rt = CTL_SEN_E_SYS;
			}
#else
			struct clk *parent;
			struct clk *csi_clk;

			csi_clk = clk_get(NULL, CG_STR_CSI);
			if (IS_ERR(csi_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			parent = clk_get_parent(csi_clk);
			*ret = clk_get_rate(parent);
			clk_put(csi_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_MIPILVDS2_FREQUENCY: {
#ifndef __KERNEL__
			if (pll_get_clock_rate(PLL_CLKSEL_CSI2) == PLL_CLKSEL_CSI2_60) {
				*ret = CTL_SEN_60M_HZ;
			} else if (pll_get_clock_rate(PLL_CLKSEL_CSI2) == PLL_CLKSEL_CSI2_120) {
				*ret = CTL_SEN_120M_HZ;
			} else if (pll_get_clock_rate(PLL_CLKSEL_CSI2) == PLL_CLKSEL_CSI2_240) {
				*ret = CTL_SEN_240M_HZ;
			} else {
				ctl_sen_dbg_err("%d: get clk outofrange 0x%lx\r\n", (UINT32)param_id, (ULONG)pll_get_clock_rate(PLL_CLKSEL_CSI2));
				*ret = CTL_SEN_60M_HZ;
				rt = CTL_SEN_E_SYS;
			}
#else
			struct clk *parent;
			struct clk *csi_clk;

			csi_clk = clk_get(NULL, CG_STR_CSI2);
			if (IS_ERR(csi_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			parent = clk_get_parent(csi_clk);
			*ret = clk_get_rate(parent);
			clk_put(csi_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_MIPILVDS3_FREQUENCY: {
#ifndef __KERNEL__
			if (pll_get_clock_rate(PLL_CLKSEL_CSI3) == PLL_CLKSEL_CSI3_60) {
				*ret = CTL_SEN_60M_HZ;
			} else if (pll_get_clock_rate(PLL_CLKSEL_CSI3) == PLL_CLKSEL_CSI3_120) {
				*ret = CTL_SEN_120M_HZ;
			} else if (pll_get_clock_rate(PLL_CLKSEL_CSI3) == PLL_CLKSEL_CSI3_240) {
				*ret = CTL_SEN_240M_HZ;
			} else {
				ctl_sen_dbg_err("%d: get clk outofrange 0x%lx\r\n", (UINT32)param_id, (ULONG)pll_get_clock_rate(PLL_CLKSEL_CSI3));
				*ret = CTL_SEN_60M_HZ;
				rt = CTL_SEN_E_SYS;
			}
#else
			struct clk *parent;
			struct clk *csi_clk;

			csi_clk = clk_get(NULL, CG_STR_CSI3);
			if (IS_ERR(csi_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			parent = clk_get_parent(csi_clk);
			*ret = clk_get_rate(parent);
			clk_put(csi_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_MIPILVDS4_FREQUENCY: {
#ifndef __KERNEL__
			if (pll_get_clock_rate(PLL_CLKSEL_CSI4) == PLL_CLKSEL_CSI4_60) {
				*ret = CTL_SEN_60M_HZ;
			} else if (pll_get_clock_rate(PLL_CLKSEL_CSI4) == PLL_CLKSEL_CSI4_120) {
				*ret = CTL_SEN_120M_HZ;
			} else if (pll_get_clock_rate(PLL_CLKSEL_CSI4) == (UINT32)PLL_CLKSEL_CSI4_240) {
				*ret = CTL_SEN_240M_HZ;
			} else {
				ctl_sen_dbg_err("%d: get clk outofrange 0x%lx\r\n", (UINT32)param_id, (ULONG)pll_get_clock_rate(PLL_CLKSEL_CSI4));
				*ret = CTL_SEN_60M_HZ;
				rt = CTL_SEN_E_SYS;
			}
#else
			struct clk *parent;
			struct clk *csi_clk;

			csi_clk = clk_get(NULL, CG_STR_CSI4);
			if (IS_ERR(csi_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			parent = clk_get_parent(csi_clk);
			*ret = clk_get_rate(parent);
			clk_put(csi_clk);
#endif
		}
		break;

#if 0 // 538 remove
	case CTL_SEN_UTI_CLK_MIPILVDS5_FREQUENCY: {
#ifndef __KERNEL__
			if (pll_get_clock_rate(PLL_CLKSEL_CSI5) == PLL_CLKSEL_CSI5_60) {
				*ret = CTL_SEN_60M_HZ;
			} else if (pll_get_clock_rate(PLL_CLKSEL_CSI5) == PLL_CLKSEL_CSI5_120) {
				*ret = CTL_SEN_120M_HZ;
			} else if (pll_get_clock_rate(PLL_CLKSEL_CSI5) == PLL_CLKSEL_CSI5_240) {
				*ret = CTL_SEN_240M_HZ;
			} else {
				ctl_sen_dbg_err("%d: get clk outofrange 0x%lx\r\n", (UINT32)param_id, (ULONG)pll_get_clock_rate(PLL_CLKSEL_CSI5));
				*ret = CTL_SEN_60M_HZ;
				rt = CTL_SEN_E_SYS;
			}
#else
			struct clk *parent;
			struct clk *csi_clk;

			csi_clk = clk_get(NULL, CG_STR_CSI5);
			if (IS_ERR(csi_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			parent = clk_get_parent(csi_clk);
			*ret = clk_get_rate(parent);
			clk_put(csi_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_MIPILVDS6_FREQUENCY: {
#ifndef __KERNEL__
			if (pll_get_clock_rate(PLL_CLKSEL_CSI6) == PLL_CLKSEL_CSI6_60) {
				*ret = CTL_SEN_60M_HZ;
			} else if (pll_get_clock_rate(PLL_CLKSEL_CSI6) == PLL_CLKSEL_CSI6_120) {
				*ret = CTL_SEN_120M_HZ;
			} else if (pll_get_clock_rate(PLL_CLKSEL_CSI6) == PLL_CLKSEL_CSI6_240) {
				*ret = CTL_SEN_240M_HZ;
			} else {
				ctl_sen_dbg_err("%d: get clk outofrange 0x%lx\r\n", (UINT32)param_id, (ULONG)pll_get_clock_rate(PLL_CLKSEL_CSI6));
				*ret = CTL_SEN_60M_HZ;
				rt = CTL_SEN_E_SYS;
			}
#else
			struct clk *parent;
			struct clk *csi_clk;

			csi_clk = clk_get(NULL, CG_STR_CSI6);
			if (IS_ERR(csi_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			parent = clk_get_parent(csi_clk);
			*ret = clk_get_rate(parent);
			clk_put(csi_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_MIPILVDS7_FREQUENCY: {
#ifndef __KERNEL__
			if (pll_get_clock_rate(PLL_CLKSEL_CSI7) == PLL_CLKSEL_CSI7_60) {
				*ret = CTL_SEN_60M_HZ;
			} else if (pll_get_clock_rate(PLL_CLKSEL_CSI7) == PLL_CLKSEL_CSI7_120) {
				*ret = CTL_SEN_120M_HZ;
			} else if (pll_get_clock_rate(PLL_CLKSEL_CSI7) == PLL_CLKSEL_CSI7_240) {
				*ret = CTL_SEN_240M_HZ;
			} else {
				ctl_sen_dbg_err("%d: get clk outofrange 0x%lx\r\n", (UINT32)param_id, (ULONG)pll_get_clock_rate(PLL_CLKSEL_CSI7));
				*ret = CTL_SEN_60M_HZ;
				rt = CTL_SEN_E_SYS;
			}
#else
			struct clk *parent;
			struct clk *csi_clk;

			csi_clk = clk_get(NULL, CG_STR_CSI7);
			if (IS_ERR(csi_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			parent = clk_get_parent(csi_clk);
			*ret = clk_get_rate(parent);
			clk_put(csi_clk);
#endif
		}
		break;

	case CTL_SEN_UTI_CLK_MIPILVDS8_FREQUENCY: {
#ifndef __KERNEL__
			if (pll_get_clock_rate(PLL_CLKSEL_CSI8) == PLL_CLKSEL_CSI8_60) {
				*ret = CTL_SEN_60M_HZ;
			} else if (pll_get_clock_rate(PLL_CLKSEL_CSI8) == PLL_CLKSEL_CSI8_120) {
				*ret = CTL_SEN_120M_HZ;
			} else if (pll_get_clock_rate(PLL_CLKSEL_CSI8) == (UINT32)PLL_CLKSEL_CSI8_240) {
				*ret = CTL_SEN_240M_HZ;
			} else {
				ctl_sen_dbg_err("%d: get clk outofrange 0x%lx\r\n", (UINT32)param_id, (ULONG)pll_get_clock_rate(PLL_CLKSEL_CSI8));
				*ret = CTL_SEN_60M_HZ;
				rt = CTL_SEN_E_SYS;
			}
#else
			struct clk *parent;
			struct clk *csi_clk;

			csi_clk = clk_get(NULL, CG_STR_CSI8);
			if (IS_ERR(csi_clk)) {
				ctl_sen_dbg_err("%d: get src clk fail\r\n", (UINT32)param_id);
				rt = CTL_SEN_E_SYS;
			}

			parent = clk_get_parent(csi_clk);
			*ret = clk_get_rate(parent);
			clk_put(csi_clk);
#endif
		}
		break;
#endif

	default:
		ctl_sen_dbg_err("Err param_id 0x%08X\r\n", param_id);
		return CTL_SEN_E_IN_PARAM;
	}
	return rt;
}

static INT32 _pll_set_en(CTL_SEN_CLKSRC_SEL clksel, UINT32 en)
{
	CTL_SEN_UTI_CLK item;

	ctl_sen_dbg_ind("CTL_SEN_CLKSRC_SEL %d en %d GO\r\n", clksel, en);

	if (clksel == CTL_SEN_CLKSRC_480) {
		// fixed 480MHz is always enabled
		return 0;
	}
	if (clksel == CTL_SEN_CLKSRC_320) {
		// fixed 320MHz is always enabled
		return 0;
	}

	switch (clksel) {
	case CTL_SEN_CLKSRC_PLL4:
		item = CTL_SEN_UTI_CLK_PLL4_ENABLE;
		break;
	case CTL_SEN_CLKSRC_PLL5:
		item = CTL_SEN_UTI_CLK_PLL5_ENABLE;
		break;
	case CTL_SEN_CLKSRC_PLL6:
		item = CTL_SEN_UTI_CLK_PLL6_ENABLE;
		break;
	case CTL_SEN_CLKSRC_PLL12:
		item = CTL_SEN_UTI_CLK_PLL12_ENABLE;
		break;
	default:
		ctl_sen_dbg_err("CTL_SEN_CLKSRC_SEL %d N.S.\r\n", clksel);
		return CTL_SEN_E_IN_PARAM;
	}
	return _set_clk(item, en);
}

INT32 _pll_get_en(CTL_SEN_CLKSRC_SEL clksel, UINT32 *en)
{
	CTL_SEN_UTI_CLK item;

	if (clksel == CTL_SEN_CLKSRC_480) {
		// fixed 480MHz is always enabled
		*en = 1;
		return CTL_SEN_E_OK;
	}
	if (clksel == CTL_SEN_CLKSRC_320) {
		// fixed 320MHz is always enabled
		*en = 1;
		return CTL_SEN_E_OK;
	}

	switch (clksel) {
	case CTL_SEN_CLKSRC_PLL4:
		item = CTL_SEN_UTI_CLK_PLL4_ENABLE;
		break;
	case CTL_SEN_CLKSRC_PLL5:
		item = CTL_SEN_UTI_CLK_PLL5_ENABLE;
		break;
	case CTL_SEN_CLKSRC_PLL6:
		item = CTL_SEN_UTI_CLK_PLL6_ENABLE;
		break;
	case CTL_SEN_CLKSRC_PLL12:
		item = CTL_SEN_UTI_CLK_PLL12_ENABLE;
		break;
	default:
		ctl_sen_dbg_err("CTL_SEN_CLKSRC_SEL %d N.S.\r\n", clksel);
		return CTL_SEN_E_IN_PARAM;
	}

	return _get_clk(item, en);
}

INT32 _pll_set_freq(CTL_SEN_CLKSRC_SEL clksel, UINT32 freq)
{
	CTL_SEN_UTI_CLK item;

	ctl_sen_dbg_ind("CTL_SEN_CLKSRC_SEL %d GO\r\n", clksel);

	switch (clksel) {
	case CTL_SEN_CLKSRC_PLL4:
		item = CTL_SEN_UTI_CLK_PLL4_FREQUENCY;
		break;
	case CTL_SEN_CLKSRC_PLL5:
		item = CTL_SEN_UTI_CLK_PLL5_FREQUENCY;
		break;
	case CTL_SEN_CLKSRC_PLL6:
		item = CTL_SEN_UTI_CLK_PLL6_FREQUENCY;
		break;
	case CTL_SEN_CLKSRC_PLL12:
		item = CTL_SEN_UTI_CLK_PLL12_FREQUENCY;
		break;
	default:
		ctl_sen_dbg_err("CTL_SEN_CLKSRC_SEL %d N.S.\r\n", clksel);
		return CTL_SEN_E_IN_PARAM;
	}
	return _set_clk(item, freq);
}

static INT32 _pll_get_freq(CTL_SEN_CLKSRC_SEL clksel, UINT32 *freq)
{
	CTL_SEN_UTI_CLK item;

	ctl_sen_dbg_ind("CTL_SEN_CLKSRC_SEL %d GO\r\n", clksel);

	switch (clksel) {
	case CTL_SEN_CLKSRC_480:
		*freq = CTL_SEN_480M_HZ;
		return 0;
	case CTL_SEN_CLKSRC_320:
		*freq = CTL_SEN_320M_HZ;
		return 0;
	case CTL_SEN_CLKSRC_PLL4:
		item = CTL_SEN_UTI_CLK_PLL4_FREQUENCY;
		break;
	case CTL_SEN_CLKSRC_PLL5:
		item = CTL_SEN_UTI_CLK_PLL5_FREQUENCY;
		break;
	case CTL_SEN_CLKSRC_PLL6:
		item = CTL_SEN_UTI_CLK_PLL6_FREQUENCY;
		break;
	case CTL_SEN_CLKSRC_PLL12:
		item = CTL_SEN_UTI_CLK_PLL12_FREQUENCY;
		break;
	default:
		ctl_sen_dbg_err("CTL_SEN_CLKSRC_SEL %d N.S.\r\n", clksel);
		return CTL_SEN_E_IN_PARAM;
	}

	return _get_clk(item, freq);
}

static INT32 _mclk_set_src(CTL_SEN_CLK_SEL mclksel, CTL_SEN_CLKSRC_SEL mclksrc)
{
	CTL_SEN_UTI_CLK item;

	ctl_sen_dbg_ind("CTL_SEN_CLK_SEL %d CTL_SEN_CLKSRC_SEL %d GO\r\n", mclksel, mclksrc);

	switch (mclksel) {
	case CTL_SEN_CLK_SEL_SIEMCLK:
		item = CTL_SEN_UTI_CLK_SIEMCLK_SOURCE;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK2:
		item = CTL_SEN_UTI_CLK_SIEMCLK2_SOURCE;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK3:
		item = CTL_SEN_UTI_CLK_SIEMCLK3_SOURCE;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK4:
		item = CTL_SEN_UTI_CLK_SIEMCLK4_SOURCE;
		break;
#if 0 // 538 remove
	case CTL_SEN_CLK_SEL_SIEMCLK5:
		item = CTL_SEN_UTI_CLK_SIEMCLK5_SOURCE;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK6:
		item = CTL_SEN_UTI_CLK_SIEMCLK6_SOURCE;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK7:
		item = CTL_SEN_UTI_CLK_SIEMCLK7_SOURCE;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK8:
		item = CTL_SEN_UTI_CLK_SIEMCLK8_SOURCE;
		break;
#endif
	case CTL_SEN_CLK_SEL_SPCLK:
		item = CTL_SEN_UTI_CLK_SPCLK_SOURCE;
		break;
	case CTL_SEN_CLK_SEL_SPCLK2:
		item = CTL_SEN_UTI_CLK_SPCLK2_SOURCE;
		break;
	default:
		ctl_sen_dbg_err("CTL_SEN_CLK_SEL %d N.S.\r\n", mclksel);
		return CTL_SEN_E_IN_PARAM;
	}

	return _set_clk(item, mclksrc);
}

static INT32 _mclk_set_freq(CTL_SEN_CLK_SEL mclksel, UINT32 freq)
{
	CTL_SEN_UTI_CLK item;

	ctl_sen_dbg_ind("CTL_SEN_CLK_SEL %d freq %d GO\r\n", mclksel, freq);

	switch (mclksel) {
	case CTL_SEN_CLK_SEL_SIEMCLK:
		item = CTL_SEN_UTI_CLK_SIEMCLK_FREQUENCY;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK2:
		item = CTL_SEN_UTI_CLK_SIEMCLK2_FREQUENCY;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK3:
		item = CTL_SEN_UTI_CLK_SIEMCLK3_FREQUENCY;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK4:
		item = CTL_SEN_UTI_CLK_SIEMCLK4_FREQUENCY;
		break;
#if 0 // 538 remove
	case CTL_SEN_CLK_SEL_SIEMCLK5:
		item = CTL_SEN_UTI_CLK_SIEMCLK5_FREQUENCY;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK6:
		item = CTL_SEN_UTI_CLK_SIEMCLK6_FREQUENCY;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK7:
		item = CTL_SEN_UTI_CLK_SIEMCLK7_FREQUENCY;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK8:
		item = CTL_SEN_UTI_CLK_SIEMCLK8_FREQUENCY;
		break;
#endif
	case CTL_SEN_CLK_SEL_SPCLK:
		item = CTL_SEN_UTI_CLK_SPCLK_FREQUENCY;
		break;
	case CTL_SEN_CLK_SEL_SPCLK2:
		item = CTL_SEN_UTI_CLK_SPCLK2_FREQUENCY;
		break;
	default:
		ctl_sen_dbg_err("CTL_SEN_CLK_SEL %d N.S.\r\n", mclksel);
		return CTL_SEN_E_IN_PARAM;
	}

	return _set_clk(item, freq);
}

INT32 ctl_sen_get_sendrv_mclk_info(CTL_SEN_MAP_HDL *map_hdl, CTL_SEN_MODE senmode, CTL_SEN_CLK_SEL *mclk_sel, UINT32 *mclk_freq)
{
	ER rt_sendrv;
	CTL_SEN_DRV_TAB *sendrv_tbl = NULL;
	CTL_SENDRV_GET_SPEED_PARAM sendrv_param;

	sendrv_tbl = map_hdl->chip_hdl->sendrv->drv_tab;

	memset((void *)&sendrv_param, 0, sizeof(CTL_SENDRV_GET_SPEED_PARAM));

	sendrv_param.mode = senmode;

	if ((rt_sendrv = sendrv_tbl->get_cfg(conv_senid(map_hdl), CTL_SENDRV_CFGID_GET_SPEED, (void *)&sendrv_param)) != E_OK) {
		ctl_sen_dbg_err("sen_id=%d (%s) access sendrv fail (%d)\r\n", map_hdl->sen_id, map_hdl->chip_name, rt_sendrv);
		return CTL_SEN_E_SENDRV;
	}

	*mclk_freq = sendrv_param.mclk;
	if (sendrv_param.mclk_src == CTL_SEN_SIEMCLK_SRC_DFT) {
		switch (map_hdl->chip_id) {
		case CTL_SEN_ID_1:
			*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK;
			break;
		case CTL_SEN_ID_2:
		case CTL_SEN_ID_3:
		case CTL_SEN_ID_4:
		case CTL_SEN_ID_5:
		case CTL_SEN_ID_6:
		case CTL_SEN_ID_7:
		case CTL_SEN_ID_8:
			*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK2;
			break;
		default:
			*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK2;
			break;
		}
	} else if (sendrv_param.mclk_src == CTL_SEN_SIEMCLK_SRC_MCLK) {
		*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK;
	} else if (sendrv_param.mclk_src == CTL_SEN_SIEMCLK_SRC_MCLK2) {
		*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK2;
	} else if (sendrv_param.mclk_src == CTL_SEN_SIEMCLK_SRC_MCLK3) {
		*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK3;
	} else if (sendrv_param.mclk_src == CTL_SEN_SIEMCLK_SRC_MCLK4) {
		*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK4;
#if 0 // 538 remove
	} else if (sendrv_param.mclk_src == CTL_SEN_SIEMCLK_SRC_MCLK5) {
		*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK5;
	} else if (sendrv_param.mclk_src == CTL_SEN_SIEMCLK_SRC_MCLK6) {
		*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK6;
	} else if (sendrv_param.mclk_src == CTL_SEN_SIEMCLK_SRC_MCLK7) {
		*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK7;
	} else if (sendrv_param.mclk_src == CTL_SEN_SIEMCLK_SRC_MCLK8) {
		*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK8;
#endif
	} else if (sendrv_param.mclk_src == CTL_SEN_SIEMCLK_SRC_SPCLK) {
		*mclk_sel = CTL_SEN_CLK_SEL_SPCLK;
	} else if (sendrv_param.mclk_src == CTL_SEN_SIEMCLK_SRC_SPCLK2) {
		*mclk_sel = CTL_SEN_CLK_SEL_SPCLK2;
	} else if (sendrv_param.mclk_src == CTL_SEN_SIEMCLK_SRC_IGNORE) {
		*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK_IGNORE;
	} else {
		ctl_sen_dbg_err("sen_id=%d (%s) mclk src error %d\r\n", map_hdl->sen_id, map_hdl->chip_name, sendrv_param.mclk_src);
		*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK;
	}

	return CTL_SEN_E_OK;
}

/* for 530 tge engine limit */

static KDRV_TGE_CH_SFT _conv2_tge_ch_sft(CTL_SEN_DRVDEV drvdev)
{
	KDRV_TGE_CH_SFT ch_sft_tbl[CTL_SEN_DRVDEV_IDX_TGE(CTL_SEN_DRVDEV_TGE_MAX) + 1] = {
		KDRV_TGE_CH_SFT0, KDRV_TGE_CH_SFT1, KDRV_TGE_CH_SFT2, KDRV_TGE_CH_SFT3,
		KDRV_TGE_CH_SFT4, KDRV_TGE_CH_SFT5, KDRV_TGE_CH_SFT6, KDRV_TGE_CH_SFT7
	};

	if ((CTL_SEN_DRVDEV_MASK_TGE(drvdev) >= CTL_SEN_DRVDEV_TGE_BASE) && (CTL_SEN_DRVDEV_MASK_TGE(drvdev) <= CTL_SEN_DRVDEV_TGE_MAX)) {
		return ch_sft_tbl[CTL_SEN_DRVDEV_IDX_TGE(drvdev)];
	}

	ctl_sen_dbg_err("drvdev 0x%.8x error\r\n", (unsigned int)drvdev);
	return KDRV_TGE_CH_SFT_MAX;
}

INT32 ctl_sen_get_tge_mclk_info(CTL_SEN_MAP_HDL *map_hdl, CTL_SEN_CLK_SEL *mclk_sel)
{
	CTL_SEN_DRVDEV drvdev = map_hdl->chip_hdl->init_cfg_obj.drvdev;

	if (CTL_SEN_DRVDEV_MASK_TGE(drvdev) == 0) {
		*mclk_sel = CTL_SEN_CLK_SEL_MAX;
		return CTL_SEN_E_OK;
	}

	switch (_conv2_tge_ch_sft(drvdev)) {
		case KDRV_TGE_CH_SFT0:
			*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK;
			break;
		case KDRV_TGE_CH_SFT1:
			*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK2;
			break;
		case KDRV_TGE_CH_SFT2:
			*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK3;
			break;
		case KDRV_TGE_CH_SFT3:
			*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK4;
			break;
#if 0 // 538 remove
		case KDRV_TGE_CH_SFT4:
			*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK5;
			break;
		case KDRV_TGE_CH_SFT5:
			*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK6;
			break;
		case KDRV_TGE_CH_SFT6:
			*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK7;
			break;
		case KDRV_TGE_CH_SFT7:
			*mclk_sel = CTL_SEN_CLK_SEL_SIEMCLK8;
			break;
#endif
		default:
			ctl_sen_dbg_wrn("sen_id=%d (%s) drvdev 0x%x out-of-range\r\n", map_hdl->sen_id, map_hdl->chip_name, drvdev);
			*mclk_sel = CTL_SEN_CLK_SEL_MAX;
			break;
		}

	return CTL_SEN_E_OK;
}

INT32 ctl_sen_mclk_prepare(void *hdl, CTL_SEN_MODE senmode)
{
	// MCLK PLL selection order
#define SEN_RANK_TBL_LEN 5

	CTL_SEN_MAP_HDL *map_hdl = (CTL_SEN_MAP_HDL *)hdl;
	static CTL_SEN_CLKSRC_SEL sen_clk_srcsel_rank[CTL_SEN_CLK_SEL_MAX][SEN_RANK_TBL_LEN] = {
		{CTL_SEN_CLKSRC_480,    CTL_SEN_CLKSRC_PLL5,    CTL_SEN_CLKSRC_PLL12,   CTL_SEN_CLKSRC_PLL6,   CTL_SEN_CLKSRC_MAX}, // CTL_SEN_CLK_SEL_SIEMCLK
		{CTL_SEN_CLKSRC_480,    CTL_SEN_CLKSRC_PLL5,    CTL_SEN_CLKSRC_PLL12,   CTL_SEN_CLKSRC_PLL6,   CTL_SEN_CLKSRC_MAX}, // CTL_SEN_CLK_SEL_SIEMCLK2
		{CTL_SEN_CLKSRC_480,    CTL_SEN_CLKSRC_PLL5,    CTL_SEN_CLKSRC_PLL12,   CTL_SEN_CLKSRC_PLL6,   CTL_SEN_CLKSRC_MAX}, // CTL_SEN_CLK_SEL_SIEMCLK3
		{CTL_SEN_CLKSRC_480,    CTL_SEN_CLKSRC_PLL5,    CTL_SEN_CLKSRC_PLL12,   CTL_SEN_CLKSRC_PLL6,   CTL_SEN_CLKSRC_MAX}, // CTL_SEN_CLK_SEL_SIEMCLK4
		{CTL_SEN_CLKSRC_MAX,    CTL_SEN_CLKSRC_MAX,     CTL_SEN_CLKSRC_MAX,     CTL_SEN_CLKSRC_MAX,    CTL_SEN_CLKSRC_MAX}, // CTL_SEN_CLK_SEL_SIEMCLK5
		{CTL_SEN_CLKSRC_MAX,    CTL_SEN_CLKSRC_MAX,     CTL_SEN_CLKSRC_MAX,     CTL_SEN_CLKSRC_MAX,    CTL_SEN_CLKSRC_MAX}, // CTL_SEN_CLK_SEL_SIEMCLK6
		{CTL_SEN_CLKSRC_MAX,    CTL_SEN_CLKSRC_MAX,     CTL_SEN_CLKSRC_MAX,     CTL_SEN_CLKSRC_MAX,    CTL_SEN_CLKSRC_MAX}, // CTL_SEN_CLK_SEL_SIEMCLK7
		{CTL_SEN_CLKSRC_MAX,    CTL_SEN_CLKSRC_MAX,     CTL_SEN_CLKSRC_MAX,     CTL_SEN_CLKSRC_MAX,    CTL_SEN_CLKSRC_MAX}, // CTL_SEN_CLK_SEL_SIEMCLK8
		{CTL_SEN_CLKSRC_480,    CTL_SEN_CLKSRC_PLL5,    CTL_SEN_CLKSRC_PLL4,    CTL_SEN_CLKSRC_PLL6,   CTL_SEN_CLKSRC_MAX}, // CTL_SEN_CLK_SEL_SPCLK
		{CTL_SEN_CLKSRC_480,    CTL_SEN_CLKSRC_PLL5,    CTL_SEN_CLKSRC_PLL4,    CTL_SEN_CLKSRC_PLL6,   CTL_SEN_CLKSRC_MAX}, // CTL_SEN_CLK_SEL_SPCLK2
	};

	BOOL update_mclk = 0; // chk which mclksel must be same source (480Hz,pll5,pll12,..)
	UINT32 i, mclk_freq, freq, pll_en;
	INT32 kdrv_rt, rt = CTL_SEN_E_OK;
	CTL_SEN_CLK_SEL mclk_sel_sendrv, mclk_sel_tge = CTL_SEN_CLK_SEL_MAX;
	CTL_SEN_CLKSRC_SEL *mclk_src_sel_rank = NULL;
	CTL_SEN_CLKSRC_SEL mclk_src_sel_tmp;

	if ((rt = ctl_sen_get_sendrv_mclk_info(map_hdl, senmode, &mclk_sel_sendrv, &mclk_freq)) != 0) {
		return rt;
	}

	ctl_sen_dbg_ind("sen_id=%d (%s) mode %d CTL_SEN_CLK_SEL %d mclk_freq %d\r\n", map_hdl->sen_id, map_hdl->chip_name, senmode, mclk_sel_sendrv, mclk_freq);

	map_hdl->chip_hdl->mclk_sen_mode = senmode;

	if (mclk_sel_sendrv == CTL_SEN_CLK_SEL_SIEMCLK_IGNORE) {
		// ext mclk
		return CTL_SEN_E_OK;
	}
	if (mclk_freq == 0) {
		ctl_sen_dbg_err("sen_id=%d (%s) mode %d mclk_freq zero\r\n", map_hdl->sen_id, map_hdl->chip_name, senmode);
		return CTL_SEN_E_SENDRV;
	}

	mclk_src_sel_rank = &sen_clk_srcsel_rank[mclk_sel_sendrv][0];

	/*
	    cfg mclk src and corresponding pllx
	*/
	for (i = 0; i < SEN_RANK_TBL_LEN; i++) {
		mclk_src_sel_tmp = mclk_src_sel_rank[i];
		if (mclk_src_sel_tmp == CTL_SEN_CLKSRC_MAX) {
			// rank array end
			ctl_sen_dbg_err("sen_id=%d cannot find pll for CTL_SEN_CLK_SEL %d, freq=%d\r\n", map_hdl->sen_id, mclk_sel_sendrv, mclk_freq);
			rt = CTL_SEN_E_CLK;
			break;
		} else {
			// get pll current enable
			pll_en = 0;
			if (_pll_get_en(mclk_src_sel_tmp, &pll_en) != 0) {
				ctl_sen_dbg_err("sen_id=%d sen_pll_get_en (%d) fail\r\n", map_hdl->sen_id, mclk_src_sel_tmp);
				rt = CTL_SEN_E_CLK;
				break;
			}

			// change frequency for current using if no one enable this pll
			if (pll_en == 0) {
				if (_pll_set_freq(mclk_src_sel_tmp, mclk_freq) != 0) {
					ctl_sen_dbg_err("sen_id=%d CTL_SEN_CLKSRC_SEL %d sen_pll_set_freq %d\r\n", map_hdl->sen_id, mclk_src_sel_tmp, mclk_freq);
					rt = CTL_SEN_E_CLK;
				} else {
					// set freq directly, no matter it can be divided by "pllx current frequency", thus cg api will find the nearest freq
					ctl_sen_dbg_ind("sen_id=%d CTL_SEN_CLKSRC_SEL %d sen_pll_set_freq2 %d\r\n", map_hdl->sen_id, mclk_src_sel_tmp, mclk_freq);
					update_mclk = 1;
				}
				break;
			}

			// get clk_src current frequency
			freq = 0;
			if (_pll_get_freq(mclk_src_sel_tmp, &freq) != 0) {
				ctl_sen_dbg_err("CTL_SEN_CLKSRC_SEL %d sen_pll_get_freq\r\n", mclk_src_sel_tmp);
				rt = CTL_SEN_E_CLK;
				break;
			}
			ctl_sen_dbg_ind("CTL_SEN_CLKSRC_SEL %d = %d hz\r\n", mclk_src_sel_tmp, freq);
			if ((freq) && ((freq % mclk_freq) == 0)) {
				// "mclk frequency" is divisible by "pllx current frequency"
				update_mclk = 1;
				break;
			}
		}
	}

	if (update_mclk) {
		kdrv_rt = _mclk_set_freq(mclk_sel_sendrv, mclk_freq); // set src->freq will hit linux drv check max rate
		if (kdrv_rt) {
			ctl_sen_dbg_err("sen_id=%d sen_mclk_set_freq fail\r\n", map_hdl->sen_id);
			rt = CTL_SEN_E_CLK;
		}
		kdrv_rt = _mclk_set_src(mclk_sel_sendrv, mclk_src_sel_tmp); // linux drv will modify frequency
		if (kdrv_rt) {
			ctl_sen_dbg_err("sen_id=%d sen_mclk_set_src fail\r\n", map_hdl->sen_id);
			rt = CTL_SEN_E_CLK;
		}
		kdrv_rt = _mclk_set_freq(mclk_sel_sendrv, mclk_freq); // linux drv will modify frequency in set_src, need set frequency again
		if (kdrv_rt) {
			ctl_sen_dbg_err("sen_id=%d sen_mclk_set_freq fail\r\n", map_hdl->sen_id);
			rt = CTL_SEN_E_CLK;
		}
		kdrv_rt = _pll_set_en(mclk_src_sel_tmp, ENABLE);
		if (kdrv_rt) {
			ctl_sen_dbg_err("sen_id=%d sen_pll_set_en fail\r\n", map_hdl->sen_id);
			rt = CTL_SEN_E_CLK;
		}
		if (rt == CTL_SEN_E_OK) {
			map_hdl->chip_hdl->mclk_src_sel = mclk_src_sel_tmp;
			map_hdl->chip_hdl->mclk_freq = mclk_freq;
		}
	}

	/*
		check tge mclk, will not disable in ctl_sen_mclk_unprepare (for pinmux sync case, PIN_SENSORSYNC_CFG)
	*/
	if ((rt = ctl_sen_get_tge_mclk_info(map_hdl, &mclk_sel_tge)) != CTL_SEN_E_OK) {
		return rt;
	}
	if ((mclk_sel_tge != CTL_SEN_CLK_SEL_MAX) && (mclk_sel_tge != mclk_sel_sendrv)) {
		ctl_sen_dbg_wrn("sen_id=%d force set tge mclk_src %d (sensor mclk_src : %d)\r\n", map_hdl->sen_id, mclk_sel_tge, mclk_sel_sendrv); // refer [NA51102-1049]
		kdrv_rt = _mclk_set_src(mclk_sel_tge, mclk_src_sel_tmp);
		if (kdrv_rt) {
			ctl_sen_dbg_err("sen_id=%d sen_mclk_set_src fail [tge src %d]\r\n", map_hdl->sen_id, mclk_sel_tge);
			rt = CTL_SEN_E_CLK;
		}
		kdrv_rt = _mclk_set_freq(mclk_sel_tge, mclk_freq);
		if (kdrv_rt) {
			ctl_sen_dbg_err("sen_id=%d sen_mclk_set_freq fail [tge src %d]\r\n", map_hdl->sen_id, mclk_sel_tge);
			rt = CTL_SEN_E_CLK;
		}
		ctl_sen_mclk_set_en(mclk_sel_tge, TRUE);
	}

	return rt;
}

INT32 ctl_sen_mclk_unprepare(void *hdl, CTL_SEN_MODE senmode)
{
	CTL_SEN_MAP_HDL *map_hdl = (CTL_SEN_MAP_HDL *)hdl;
	CTL_SEN_CLK_SEL mclk_sel_sendrv;
	UINT32 mclk_freq;
	INT32 kdrv_rt, rt = CTL_SEN_E_OK;

	if ((rt = ctl_sen_get_sendrv_mclk_info(map_hdl, senmode, &mclk_sel_sendrv, &mclk_freq)) != 0) {
		return rt;
	}

	ctl_sen_dbg_ind("sen_id=%d (%s) mode %d CTL_SEN_CLK_SEL %d mclk_freq %d\r\n", map_hdl->sen_id, map_hdl->chip_name, senmode, mclk_sel_sendrv, mclk_freq);

	map_hdl->chip_hdl->mclk_sen_mode = CTL_SEN_MAP_NULL;

	if (mclk_sel_sendrv == CTL_SEN_CLK_SEL_SIEMCLK_IGNORE) {
		// ext mclk
		return CTL_SEN_E_OK;
	}
	if (mclk_freq == 0) {
		ctl_sen_dbg_err("sen_id=%d (%s) mode %d mclk_freq zero\r\n", map_hdl->sen_id, map_hdl->chip_name, senmode);
		return CTL_SEN_E_SENDRV;
	}

	// disable pllx
	kdrv_rt = _pll_set_en(map_hdl->chip_hdl->mclk_src_sel, DISABLE);
	if (kdrv_rt) {
		ctl_sen_dbg_err("CTL_SEN_CLK_SEL %d disable fail\r\n", mclk_sel_sendrv);
		rt = CTL_SEN_E_CLK;
	}

	return rt;
}

BOOL ctl_sen_mclk_freq_cmp(CTL_SEN_MAP_HDL *map_hdl, CTL_SEN_MODE senmode_0, CTL_SEN_MODE senmode_1)
{
	CTL_SEN_CLK_SEL mclk_sel_sendrv[2];
	UINT32 mclk_freq[2];

	if (ctl_sen_get_sendrv_mclk_info(map_hdl, senmode_0, &mclk_sel_sendrv[0], &mclk_freq[0]) != 0) {
		return FALSE;
	}

	if (ctl_sen_get_sendrv_mclk_info(map_hdl, senmode_1, &mclk_sel_sendrv[1], &mclk_freq[1]) != 0) {
		return FALSE;
	}

	if (mclk_freq[0] == mclk_freq[1]) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void ctl_sen_mclk_set_en(CTL_SEN_CLK_SEL mclksel, BOOL en)
{
	CTL_SEN_UTI_CLK item;
	INT32 rt;

	ctl_sen_dbg_ind("CTL_SEN_CLK_SEL %d enable %d GO\r\n", mclksel, en);
	if (mclksel == CTL_SEN_CLK_SEL_SIEMCLK_IGNORE) {
		return;
	}

	switch (mclksel) {
	case CTL_SEN_CLK_SEL_SIEMCLK:
		item = CTL_SEN_UTI_CLK_SIEMCLK_ENABLE;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK2:
		item = CTL_SEN_UTI_CLK_SIEMCLK2_ENABLE;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK3:
		item = CTL_SEN_UTI_CLK_SIEMCLK3_ENABLE;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK4:
		item = CTL_SEN_UTI_CLK_SIEMCLK4_ENABLE;
		break;
#if 0 // 538 remove
	case CTL_SEN_CLK_SEL_SIEMCLK5:
		item = CTL_SEN_UTI_CLK_SIEMCLK5_ENABLE;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK6:
		item = CTL_SEN_UTI_CLK_SIEMCLK6_ENABLE;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK7:
		item = CTL_SEN_UTI_CLK_SIEMCLK7_ENABLE;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK8:
		item = CTL_SEN_UTI_CLK_SIEMCLK8_ENABLE;
		break;
#endif
	case CTL_SEN_CLK_SEL_SPCLK:
		item = CTL_SEN_UTI_CLK_SPCLK_ENABLE;
		break;
	case CTL_SEN_CLK_SEL_SPCLK2:
		item = CTL_SEN_UTI_CLK_SPCLK2_ENABLE;
		break;
	default:
		ctl_sen_dbg_err("CTL_SEN_CLK_SEL %d N.S.\r\n", mclksel);
		return;
	}

	rt = _set_clk(item, en);
	if (rt) {
		ctl_sen_dbg_err("CTL_SEN_CLK_SEL %d enable %d fail.\r\n", mclksel, en);
	}
}

INT32 ctl_sen_mclk_get_en(CTL_SEN_CLK_SEL mclksel, BOOL *en)
{
	CTL_SEN_UTI_CLK item;
	INT32 rt;

	ctl_sen_dbg_ind("CTL_SEN_CLK_SEL %d GO\r\n", mclksel);
	if (mclksel == CTL_SEN_CLK_SEL_SIEMCLK_IGNORE) {
		*en = FALSE;
		return CTL_SEN_E_OK;
	}

	switch (mclksel) {
	case CTL_SEN_CLK_SEL_SIEMCLK:
		item = CTL_SEN_UTI_CLK_SIEMCLK_ENABLE;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK2:
		item = CTL_SEN_UTI_CLK_SIEMCLK2_ENABLE;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK3:
		item = CTL_SEN_UTI_CLK_SIEMCLK3_ENABLE;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK4:
		item = CTL_SEN_UTI_CLK_SIEMCLK4_ENABLE;
		break;
#if 0 // 538 remove
	case CTL_SEN_CLK_SEL_SIEMCLK5:
		item = CTL_SEN_UTI_CLK_SIEMCLK5_ENABLE;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK6:
		item = CTL_SEN_UTI_CLK_SIEMCLK6_ENABLE;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK7:
		item = CTL_SEN_UTI_CLK_SIEMCLK7_ENABLE;
		break;
	case CTL_SEN_CLK_SEL_SIEMCLK8:
		item = CTL_SEN_UTI_CLK_SIEMCLK8_ENABLE;
		break;
#endif
	case CTL_SEN_CLK_SEL_SPCLK:
		item = CTL_SEN_UTI_CLK_SPCLK_ENABLE;
		break;
	case CTL_SEN_CLK_SEL_SPCLK2:
		item = CTL_SEN_UTI_CLK_SPCLK2_ENABLE;
		break;
	default:
		ctl_sen_dbg_err("CTL_SEN_CLK_SEL %d N.S.\r\n", mclksel);
		return CTL_SEN_E_IN_PARAM;
	}

	rt = _get_clk(item, en);
	if (rt) {
		ctl_sen_dbg_err("CTL_SEN_CLK_SEL %d fail.\r\n", mclksel);
	}
	return rt;
}

INT32 ctl_sen_mclk_chk_en(void *hdl, CTL_SEN_MODE senmode, BOOL *mclk_en)
{
	CTL_SEN_MAP_HDL *map_hdl = (CTL_SEN_MAP_HDL *)hdl;
	UINT32 mclk_freq;
	INT32 rt = CTL_SEN_E_OK;
	CTL_SEN_CLK_SEL mclk_sel_sendrv;

	if ((rt = ctl_sen_get_sendrv_mclk_info(map_hdl, senmode, &mclk_sel_sendrv, &mclk_freq)) != 0) {
		ctl_sen_dbg_err("sen_id=%d (%s) mode %d get mclk info fail\r\n", map_hdl->sen_id, map_hdl->chip_name, senmode);
		return rt;
	}

	ctl_sen_dbg_ind("sen_id=%d (%s) mode %d CTL_SEN_CLK_SEL %d mclk_freq %d\r\n", map_hdl->sen_id, map_hdl->chip_name, senmode, mclk_sel_sendrv, mclk_freq);

	if (mclk_sel_sendrv == CTL_SEN_CLK_SEL_SIEMCLK_IGNORE) {
		// ext mclk
		*mclk_en = FALSE;
		return CTL_SEN_E_OK;
	}

	if ((rt = ctl_sen_mclk_get_en(mclk_sel_sendrv, mclk_en)) != 0) {
		ctl_sen_dbg_err("sen_id=%d (%s) mode %d get clk fail\r\n", map_hdl->sen_id, map_hdl->chip_name, senmode);
		return rt;
	}

	return CTL_SEN_E_OK;
}

