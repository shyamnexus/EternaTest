/*
    Internal header file for high-resolution timer module.

    This file is the internal header file that defines the variables, structures
    and function prototypes of timer module.

    @file       timer_int.h
    @ingroup    miDrvTimer_Timer
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2010.  All rights reserved.
*/

#ifndef _HRTIMER_INT_H
#define _HRTIMER_INT_H

#include <kwrap/nvt_type.h>
#include <kwrap/task.h>
#include <kwrap/flag.h>
#include <kwrap/debug.h>

// Register setting
#define HRTIMER_REG_STS                   1

#define HRTIMER_REG_INTEN_DISABLE         0
#define HRTIMER_REG_INTEN_ENABLE          1

#define HRTIMER_REG_MODE_ONE_SHOT         0
#define HRTIMER_REG_MODE_FREE_RUN         1

#define HRTIMER_REG_EN_DISABLE            0
#define HRTIMER_REG_EN_ENABLE             1
#define HRTIMER_REG_EN_RELOAD             1

#define HRTIMER_DIVIDER_MIN               0
#define HRTIMER_DIVIDER_MAX               255

#ifdef _NVT_EMULATION_
#define _EMULATION_             ENABLE
#else
#define _EMULATION_             DISABLE
#endif

#if defined(_NVT_FPGA_) || defined(CONFIG_NVT_FPGA_EMULATION)
#ifndef _FPGA_PLL_OSC_
#define _FPGA_PLL_OSC_  24000000
#endif
#define _FPGA_EMULATION_        ENABLE
#else
#define _FPGA_EMULATION_        DISABLE
#endif

#if (_FPGA_EMULATION_ == ENABLE)
/*
FPGA Real chip
pll2*2 CLK480 ==> 480 is use pll2 @ FPGA Y 12 fixed @ 24MHz
pll1/2 CLK192
pll1*2 PLL2
x PLL3
pll1*2 PLL4
pll1*2 PLL5
pll1 PLL6
pll3 PLL7
pll2*2 PLL8
pll1*2/5 PLL9
pll1*2/4 PLL10
pll1*2 PLL11
pll3*5 PLL12
pll1*2/3 PLL13
x PLL14

*/
// OSC * 2 / 160
#define HRTIMER_SOURCE_CLOCK              (_FPGA_PLL_OSC_ / 80)
#else
// 3 MHz (480 MHz / 160)
#define HRTIMER_SOURCE_CLOCK              3000000
#endif

// Register default value
#define HRTIMER_STATUS_REG_DEFAULT        0x00000000
#define HRTIMER_INT_REG_DEFAULT           0x00000000
#define HRTIMER_CTRL_REG_DEFAULT         0x00000000
#define HRTIMER_TARGET_LB_REG_DEFAULT       0xFFFFFFFF
#define HRTIMER_TARGET_UB_REG_DEFAULT       0xFFFFFFFF
#define HRTIMER_COUNTER_LB_REG_DEFAULT      0x00000000
#define HRTIMER_COUNTER_UB_REG_DEFAULT      0x00000000
#define HRTIMER_RELOAD_REG_DEFAULT       0x00000000
#define HRTIMER_CLKDIV_REG_DEFAULT        0x00000077
#define HRTIMER_INITCNT_LB_REG_DEFAULT       0x00000000
#define HRTIMER_INITCNT_UB_REG_DEFAULT        0x00000000

#define CNT_SHIFT 0x10
#define BUF1_SHIFT 0x8

/*
    Timer register default value

    Timer register default value.
*/
typedef struct {
	UINT32  uiOffset;
	UINT32  uiValue;
	CHAR    *pName;
} HRTIMER_REG_DEFAULT;

#endif
