/*
    Register definition header file for timer module.

    This file is the header file that define the address offset and bit
    definition of registers of timer module.

    @file       timer_reg.h
    @ingroup    miDrvTimer_Timer
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
*/

#ifndef _HRTIMER_REG_H
#define _HRTIMER_REG_H

#include "rcw_macro.h"
#include "io_address.h"
#include "kwrap/type.h"

// HRTimer Status Register
REGDEF_OFFSET(HRTIMER_STS_REG, 0x00)
REGDEF_BEGIN(HRTIMER_STS0_REG)            // --> Register "HRTIMER_STS_REG" begin ---
REGDEF_BIT(TimerStsCPU, 1)
REGDEF_BIT(, 31)
REGDEF_END(HRTIMER_STS_REG)              // --- Register "HRTIMER_STS_REG" end   <--

// HRTimer Interrupt Register
REGDEF_OFFSET(HRTIMER_INT_REG, 0x04)
REGDEF_BEGIN(HRTIMER_INT_REG)            // --> Register "HRTIMER_INT_REG" begin ---
REGDEF_BIT(TimerINTCPU, 1)
REGDEF_BIT(, 31)
REGDEF_END(HRTIMER_INT_REG)              // --- Register "HRTIMER_INT_REG" end   <--

// HRTimer Control Register
REGDEF_OFFSET(HRTIMER_CTRL_REG, 0x08)
REGDEF_BEGIN(HRTIMER_CTRL_REG)           // --> Register "HRTIMER_CTRL_REG" begin ---
REGDEF_BIT(Enable, 1)
REGDEF_BIT(Mode, 1)
REGDEF_BIT(INIT_CNT_EN, 1)
REGDEF_BIT(, 29)
REGDEF_END(HRTIMER_CTRL_REG)             // --- Register "HRTIMER_CTRL_REG" end   <--

// HRTimer Target Register Lower 4 bytes
REGDEF_OFFSET(HRTIMER_TARGET_LB_REG, 0x0C)
REGDEF_BEGIN(HRTIMER_TARGET_LB_REG)         // --> Register "HRTIMER_TARGET_LB_REG" begin ---
REGDEF_BIT(TargetLB, 32)
REGDEF_END(HRTIMER_TARGET_LB_REG)           // --- Register "HRTIMER_TARGET_LB_REG" end   <--

// HRTimer Target Register Upper 4 bytes
REGDEF_OFFSET(HRTIMER_TARGET_UB_REG, 0x10)
REGDEF_BEGIN(HRTIMER_TARGET_UB_REG)         // --> Register "HRTIMER_TARGET_UB_REG" begin ---
REGDEF_BIT(TargetUB, 32)
REGDEF_END(HRTIMER_TARGET_UB_REG)           // --- Register "HRTIMER_TARGET_UB_REG" end   <--

// HRTimer Counter Register Lower 4 bytes
REGDEF_OFFSET(HRTIMER_COUNTER_LB_REG, 0x14)
REGDEF_BEGIN(HRTIMER_COUNTER_LB_REG)        // --> Register "HRTIMER_COUNTER_LB_REG" begin ---
REGDEF_BIT(CounterLB, 32)
REGDEF_END(HRTIMER_COUNTER_LB_REG)          // --- Register "HRTIMER_COUNTER_LB_REG" end   <--

// HRTimer Counter Register Upper 4 bytes
REGDEF_OFFSET(HRTIMER_COUNTER_UB_REG, 0x18)
REGDEF_BEGIN(HRTIMER_COUNTER_UB_REG)        // --> Register "HRTIMER_COUNTER_UB_REG" begin ---
REGDEF_BIT(CounterUB, 32)
REGDEF_END(HRTIMER_COUNTER_UB_REG)          // --- Register "HRTIMER_COUNTER_UB_REG" end   <--

// HRTimer Reload Register
REGDEF_OFFSET(HRTIMER_RELOAD_REG, 0x1C)
REGDEF_BEGIN(HRTIMER_RELOAD_REG)        // --> Register "HRTIMER_RELOAD_REG" begin ---
REGDEF_BIT(RELOAD, 1)
REGDEF_BIT(, 31)
REGDEF_END(HRTIMER_RELOAD_REG)          // --- Register "HRTIMER_RELOAD_REG" end   <--

// HRTimer Clock Divider Register
REGDEF_OFFSET(HRTIMER_CLKDIV_REG, 0x20)
REGDEF_BEGIN(HRTIMER_CLKDIV_REG)          // --> Register "HRTIMER_CLKDIV_REG" begin ---
REGDEF_BIT(Divider, 8)
REGDEF_BIT(, 24)
REGDEF_END(HRTIMER_CLKDIV_REG)            // --- Register "HRTIMER_CLKDIV_REG" end   <--

// HRTimer Init Count For Lower 4 Bytes
REGDEF_OFFSET(HRTIMER_INIT_CNT_LB_REG, 0x24)
REGDEF_BEGIN(HRTIMER_INIT_CNT_LB_REG)          // --> Register "HRTIMER_INIT_CNT_LB_REG" begin ---
REGDEF_BIT(InitcntLB, 32)
REGDEF_END(HRTIMER_INIT_CNT_LB_REG)            // --- Register "HRTIMER_INIT_CNT_LB_REG" end   <--

// HRTimer Init Count For Upper 4 Bytes
REGDEF_OFFSET(HRTIMER_INIT_CNT_UB_REG, 0x28)
REGDEF_BEGIN(HRTIMER_INIT_CNT_UB_REG)          // --> Register "HRTIMER_INIT_CNT_UB_REG" begin ---
REGDEF_BIT(InitcntUB, 32)
REGDEF_END(HRTIMER_INIT_CNT_UB_REG)            // --- Register "HRTIMER_INIT_CNT_UB_REG" end   <--

// HRTimer External Counter Enable
REGDEF_OFFSET(HRTIMER_EXTCNT_EN_REG, 0x2C)
REGDEF_BEGIN(HRTIMER_EXTCNT_EN_REG)          // --> Register "HRTIMER_EXTCNT_EN_REG" begin ---
REGDEF_BIT(Cnt_en, 8)
REGDEF_BIT(, 24)
REGDEF_END(HRTIMER_EXTCNT_EN_REG)            // --- Register "HRTIMER_EXTCNT_EN_REG" end   <--

// HRTimer External Counter Test Bit
REGDEF_OFFSET(HRTIMER_EXTCNT_TST_REG, 0x30)
REGDEF_BEGIN(HRTIMER_EXTCNT_TST_REG)          // --> Register "HRTIMER_EXTCNT_TST_REG" begin ---
REGDEF_BIT(Cnt_tst_set, 8)
REGDEF_BIT(, 24)
REGDEF_END(HRTIMER_EXTCNT_TST_REG)            // --- Register "HRTIMER_EXTCNT_TST_REG" end   <--


// HRTimer External Counter Index
REGDEF_OFFSET(HRTIMER_EXTCNT_IDX_REG, 0x34)
REGDEF_BEGIN(HRTIMER_EXTCNT_IDX_REG)          // --> Register "HRTIMER_EXTCNT_IDX_REG" begin ---
REGDEF_BIT(Cnt_set1_idx, 1)
REGDEF_BIT(Cnt_set2_idx, 1)
REGDEF_BIT(, 30)
REGDEF_END(HRTIMER_EXTCNT_IDX_REG)            // --- Register "HRTIMER_EXTCNT_IDX_REG" end   <--


// HRTimer Signal Count For Lower 4 Bytes of CNT0
REGDEF_OFFSET(HRTIMER_EXT_CNT_LB_REG, 0x38)
REGDEF_BEGIN(HRTIMER_EXT_CNT_LB_REG)          // --> Register "HRTIMER_EXT_CNT_LB_REG" begin ---
REGDEF_BIT(ExtcntLB, 32)
REGDEF_END(HRTIMER_EXT_CNT_LB_REG)            // --- Register "HRTIMER_EXT_CNT_LB_REG" end   <--

// HRTimer Signal Count For Upper 4 Bytes of CNT0
REGDEF_OFFSET(HRTIMER_EXT_CNT_UB_REG, 0x3C)
REGDEF_BEGIN(HRTIMER_EXT_CNT_UB_REG)          // --> Register "HRTIMER_EXT_CNT_UB_REG" begin ---
REGDEF_BIT(ExtcntUB, 32)
REGDEF_END(HRTIMER_EXT_CNT_UB_REG)            // --- Register "HRTIMER_EXT_CNT_UB_REG" end   <--

#endif
