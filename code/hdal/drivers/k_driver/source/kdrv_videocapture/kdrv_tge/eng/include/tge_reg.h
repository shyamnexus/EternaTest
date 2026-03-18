/*
    TGE register header.

    @file       tge_reg.h
    @ingroup    mIIPPTGE

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#ifndef _TGE_REG_H_
#define _TGE_REG_H_

#include "kwrap/type.h"
#include "rcw_macro.h"

/*
    TGE_RST       :     [0x0, 0x1],     bits : 0
    FLSHA_LOAD    :     [0x0, 0x1],     bits : 1
    MSHA_LOAD     :     [0x0, 0x1],     bits : 2

    VD_LOAD       :     [0x0, 0x1],     bits : 8
    VD2_LOAD      :     [0x0, 0x1],     bits : 9
    VD3_LOAD      :     [0x0, 0x1],     bits :10
    VD4_LOAD      :     [0x0, 0x1],     bits :11

    VD_RST        :     [0x0, 0x1],     bits :16
    VD2_RST       :     [0x0, 0x1],     bits :17
    VD3_RST       :     [0x0, 0x1],     bits :18
    VD4_RST       :     [0x0, 0x1],     bits :19
*/
#define R0_ENGINE_CONTROL_OFS 0x0000
REGDEF_BEGIN(R0_ENGINE_CONTROL)
REGDEF_BIT(TGE_RST,         1)
REGDEF_BIT(FLSHA_LOAD,      1)
REGDEF_BIT(MSHA_LOAD,       1)
REGDEF_BIT(,                5)
REGDEF_BIT(VD_LOAD,         1)
REGDEF_BIT(VD2_LOAD,        1)
REGDEF_BIT(VD3_LOAD,        1)
REGDEF_BIT(VD4_LOAD,        1)
REGDEF_BIT(,                4)
REGDEF_BIT(VD_RST,          1)
REGDEF_BIT(VD2_RST,         1)
REGDEF_BIT(VD3_RST,         1)
REGDEF_BIT(VD4_RST,         1)
REGDEF_END(R0_ENGINE_CONTROL)

/*
    VD_PAUSE          :     [0x0, 0x1],     bits : 0
    HD_PAUSE          :     [0x0, 0x1],     bits : 1
    VD2_PAUSE         :     [0x0, 0x1],     bits : 2
    HD2_PAUSE         :     [0x0, 0x1],     bits : 3
    VD3_PAUSE         :     [0x0, 0x1],     bits : 4
    HD3_PAUSE         :     [0x0, 0x1],     bits : 5
    VD4_PAUSE         :     [0x0, 0x1],     bits : 6
    HD4_PAUSE         :     [0x0, 0x1],     bits : 7

    FLSHA_VD_HD_IN_SEL:     [0x0, 0xf],     bits :19_16
    MSHA_VD_HD_IN_SEL :     [0x0, 0xf],     bits :23_20
*/
#define R4_ENGINE_CONTROL_OFS 0x0004
REGDEF_BEGIN(R4_ENGINE_CONTROL)
REGDEF_BIT(VD_PAUSE,           1)
REGDEF_BIT(HD_PAUSE,           1)
REGDEF_BIT(VD2_PAUSE,          1)
REGDEF_BIT(HD2_PAUSE,          1)
REGDEF_BIT(VD3_PAUSE,          1)
REGDEF_BIT(HD3_PAUSE,          1)
REGDEF_BIT(VD4_PAUSE,          1)
REGDEF_BIT(HD4_PAUSE,          1)
REGDEF_BIT(,                   8)
REGDEF_BIT(FLSHA_VD_HD_IN_SEL, 4)
REGDEF_BIT(MSHA_VD_HD_IN_SEL,  4)
REGDEF_END(R4_ENGINE_CONTROL)


/*
    VD_PHASE  :    [0x0, 0x1],      bits : 0
    HD_PHASE  :    [0x0, 0x1],      bits : 1
    VD_INV    :    [0x0, 0x1],      bits : 2
    HD_INV    :    [0x0, 0x1],      bits : 3
    VD2_PHASE :    [0x0, 0x1],      bits : 4
    HD2_PHASE :    [0x0, 0x1],      bits : 5
    VD2_INV   :    [0x0, 0x1],      bits : 6
    HD2_INV   :    [0x0, 0x1],      bits : 7
    VD3_PHASE :    [0x0, 0x1],      bits : 8
    HD3_PHASE :    [0x0, 0x1],      bits : 9
    VD3_INV   :    [0x0, 0x1],      bits :10
    HD3_INV   :    [0x0, 0x1],      bits :11
    VD4_PHASE :    [0x0, 0x1],      bits :12
    HD4_PHASE :    [0x0, 0x1],      bits :13
    VD4_INV   :    [0x0, 0x1],      bits :14
    HD4_INV   :    [0x0, 0x1],      bits :15
*/
#define R8_ENGINE_CONTROL_OFS 0x0008
REGDEF_BEGIN(R8_ENGINE_CONTROL)
REGDEF_BIT(VD_PHASE,     1)
REGDEF_BIT(HD_PHASE,     1)
REGDEF_BIT(VD_INV,       1)
REGDEF_BIT(HD_INV,       1)
REGDEF_BIT(VD2_PHASE,    1)
REGDEF_BIT(HD2_PHASE,    1)
REGDEF_BIT(VD2_INV,      1)
REGDEF_BIT(HD2_INV,      1)
REGDEF_BIT(VD3_PHASE,    1)
REGDEF_BIT(HD3_PHASE,    1)
REGDEF_BIT(VD3_INV,      1)
REGDEF_BIT(HD3_INV,      1)
REGDEF_BIT(VD4_PHASE,    1)
REGDEF_BIT(HD4_PHASE,    1)
REGDEF_BIT(VD4_INV,      1)
REGDEF_BIT(HD4_INV,      1)
REGDEF_END(R8_ENGINE_CONTROL)


/*
    Reserved:       [0x0, 0xffffffff],      bits : 31_0
*/
#define RC_ENGINE_CONTROL_OFS 0x000C
REGDEF_BEGIN(RC_ENGINE_CONTROL)
REGDEF_BIT(RESERVED,    32)
REGDEF_END(RC_ENGINE_CONTROL)

/*
    FLSHA_CTRL_INV   :      [0x0, 0x1],     bits :  1

    MSHA_CTRL_INV    :      [0x0, 0x1],     bits :  3

    MSHA_SWAP        :      [0x0, 0x1],     bits :  5

    MSHA_CTRL_MODE   :      [0x0, 0x1],     bits :  7

    FLSHA_EXT_TRG_INV:      [0x0, 0x1],     bits :  9
    MSHA_EXT_TRG_INV :      [0x0, 0x1],     bits : 10
*/
#define R10_ENGINE_CONTROL_OFS 0x0010
REGDEF_BEGIN(R10_ENGINE_CONTROL)
REGDEF_BIT(,                   1)
REGDEF_BIT(FLSHA_CTRL_INV,     1)
REGDEF_BIT(,                   1)
REGDEF_BIT(MSHA_CTRL_INV,      1)
REGDEF_BIT(,                   1)
REGDEF_BIT(MSHA_SWAP,          1)
REGDEF_BIT(,                   1)
REGDEF_BIT(MSHA_CTRL_MODE,     1)
REGDEF_BIT(,                   1)
REGDEF_BIT(FLSHA_EXT_TRG_INV,  1)
REGDEF_BIT(MSHA_EXT_TRG_INV,   1)
REGDEF_END(R10_ENGINE_CONTROL)


/*
    INTE_VD             :       [0x0, 0x1],     bits : 0
    INTE_VD2            :       [0x0, 0x1],     bits : 1
    INTE_VD3            :       [0x0, 0x1],     bits : 2
    INTE_VD4            :       [0x0, 0x1],     bits : 3

    INTE_VD_BP          :       [0x0, 0x1],     bits : 8
    INTE_VD2_BP         :       [0x0, 0x1],     bits : 9
    INTE_VD3_BP         :       [0x0, 0x1],     bits :10
    INTE_VD4_BP         :       [0x0, 0x1],     bits :11

    INTE_FLSHA_TRG      :       [0x0, 0x1],     bits :16
    INTE_MSHA_CLOSE_TRG :       [0x0, 0x1],     bits :17
    INTE_MSHA_OPEN_TRG  :       [0x0, 0x1],     bits :18

    INTE_FLSHA_END      :       [0x0, 0x1],     bits :21
    INTE_MSHA_CLOSE_END :       [0x0, 0x1],     bits :22
    INTE_MSHA_OPEN_END  :       [0x0, 0x1],     bits :23
*/
#define R14_INTERRUPT_EN_OFS 0x0014
REGDEF_BEGIN(R14_INTERRUPT_EN)
REGDEF_BIT(INTE_VD,              1)
REGDEF_BIT(INTE_VD2,             1)
REGDEF_BIT(INTE_VD3,             1)
REGDEF_BIT(INTE_VD4,             1)
REGDEF_BIT(,                     4)
REGDEF_BIT(INTE_VD_BP,           1)
REGDEF_BIT(INTE_VD2_BP,          1)
REGDEF_BIT(INTE_VD3_BP,          1)
REGDEF_BIT(INTE_VD4_BP,          1)
REGDEF_BIT(,                     4)
REGDEF_BIT(INTE_FLSHA_TRG,       1)
REGDEF_BIT(INTE_MSHA_CLOSE_TRG,  1)
REGDEF_BIT(INTE_MSHA_OPEN_TRG,   1)
REGDEF_BIT(,                     2)
REGDEF_BIT(INTE_FLSHA_END,       1)
REGDEF_BIT(INTE_MSHA_CLOSE_END,  1)
REGDEF_BIT(INTE_MSHA_OPEN_END,   1)
REGDEF_END(R14_INTERRUPT_EN)


/*
    INT_VD             :        [0x0, 0x1],     bits : 0
    INT_VD2            :        [0x0, 0x1],     bits : 1
    INT_VD3            :        [0x0, 0x1],     bits : 2
    INT_VD4            :        [0x0, 0x1],     bits : 3

    INT_VD_BP          :        [0x0, 0x1],     bits : 8
    INT_VD2_BP         :        [0x0, 0x1],     bits : 9
    INT_VD3_BP         :        [0x0, 0x1],     bits :10
    INT_VD4_BP         :        [0x0, 0x1],     bits :11

    INT_FLSHA_TRG      :        [0x0, 0x1],     bits :16
    INT_MSHA_CLOSE_TRG :        [0x0, 0x1],     bits :17
    INT_MSHA_OPEN_TRG  :        [0x0, 0x1],     bits :18

    INT_FLSHA_END      :        [0x0, 0x1],     bits :21
    INT_MSHA_CLOSE_END :        [0x0, 0x1],     bits :22
    INT_MSHA_OPEN_END  :        [0x0, 0x1],     bits :23
*/
#define R18_INTERRUPT_OFS 0x0018
REGDEF_BEGIN(R18_INTERRUPT)
REGDEF_BIT(INT_VD,              1)
REGDEF_BIT(INT_VD2,             1)
REGDEF_BIT(INT_VD3,             1)
REGDEF_BIT(INT_VD4,             1)
REGDEF_BIT(,                    4)
REGDEF_BIT(INT_VD_BP,           1)
REGDEF_BIT(INT_VD2_BP,          1)
REGDEF_BIT(INT_VD3_BP,          1)
REGDEF_BIT(INT_VD4_BP,          1)
REGDEF_BIT(,                    4)
REGDEF_BIT(INT_FLSHA_TRG,       1)
REGDEF_BIT(INT_MSHA_CLOSE_TRG,  1)
REGDEF_BIT(INT_MSHA_OPEN_TRG,   1)
REGDEF_BIT(,                    2)
REGDEF_BIT(INT_FLSHA_END,       1)
REGDEF_BIT(INT_MSHA_CLOSE_END,  1)
REGDEF_BIT(INT_MSHA_OPEN_END,   1)
REGDEF_END(R18_INTERRUPT)


/*
    DEBUG_PORT  :       [0x0, 0xf],             bits : 0_3
    RTL_VERSION :       [0x0, 0xffff],          bits : 31_16
*/
#define R1C_DEBUG_OFS 0x001C
REGDEF_BEGIN(R1C_DEBUG)
REGDEF_BIT(DEBUG_PORT,   4)
REGDEF_BIT(RESERVED,    12)
REGDEF_BIT(RTL_VERSION, 16)
REGDEF_END(R1C_DEBUG)


/*
    FLSHA_CTRL:         [0x0, 0x1],             bits : 16
*/
#define R20_FLASH_STATUS_OFS 0x0020
REGDEF_BEGIN(R20_FLASH_STATUS)
REGDEF_BIT(,            16)
REGDEF_BIT(FLSHA_CTRL,   1)
REGDEF_END(R20_FLASH_STATUS)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define R24_RESERVED_OFS 0x0024
REGDEF_BEGIN(R24_RESERVED)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(R24_RESERVED)


/*
    VD_PERIOD:      [0x0, 0xfffffff],           bits : 27_0
*/
#define R28_VERTICAL_SYNC_OFS 0x0028
REGDEF_BEGIN(R28_VERTICAL_SYNC)
REGDEF_BIT(VD_PERIOD,       28)
REGDEF_END(R28_VERTICAL_SYNC)


/*
    VD_ASSERT:      [0x0, 0xfffffff],           bits : 27_0
*/
#define R2C_VERTICAL_SYNC_OFS 0x002C
REGDEF_BEGIN(R2C_VERTICAL_SYNC)
REGDEF_BIT(VD_ASSERT,       28)
REGDEF_END(R2C_VERTICAL_SYNC)


/*
    VD_FRONTBLANK:  [0x0, 0xfffffff],           bits : 27_0
*/
#define R30_VERTICAL_SYNC_OFS 0x0030
REGDEF_BEGIN(R30_VERTICAL_SYNC)
REGDEF_BIT(VD_FRONTBLANK,       28)
REGDEF_END(R30_VERTICAL_SYNC)


/*
    HD_PERIOD:      [0x0, 0xffff],              bits : 15_0
    HD_ASSERT:      [0x0, 0xffff],              bits : 31_16
*/
#define R34_HORIZONTAL_SYNC_OFS 0x0034
REGDEF_BEGIN(R34_HORIZONTAL_SYNC)
REGDEF_BIT(HD_PERIOD,       16)
REGDEF_BIT(HD_ASSERT,       16)
REGDEF_END(R34_HORIZONTAL_SYNC)


/*
    HD_COUNT:       [0x0, 0xfffffff],          bits : 27_0
*/
#define R38_HORIZONTAL_SYNC_OFS 0x0038
REGDEF_BEGIN(R38_HORIZONTAL_SYNC)
REGDEF_BIT(HD_COUNT,       28)
REGDEF_END(R38_HORIZONTAL_SYNC)


/*
    VD_BP:          [0x0, 0xfffffff],           bits : 27_0
*/
#define R3C_BREAKPOINT_OFS 0x003C
REGDEF_BEGIN(R3C_BREAKPOINT)
REGDEF_BIT(VD_BP,       28)
REGDEF_END(R3C_BREAKPOINT)

/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define R40_RESERVED_OFS 0x0040
REGDEF_BEGIN(R40_RESERVED)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(R40_RESERVED)

/*
    VD2_PERIOD:     [0x0, 0xfffffff],           bits : 27_0
*/
#define R44_VERTICAL_SYNC_OFS 0x0044
REGDEF_BEGIN(R44_VERTICAL_SYNC)
REGDEF_BIT(VD2_PERIOD,       28)
REGDEF_END(R44_VERTICAL_SYNC)


/*
    VD2_ASSERT:     [0x0, 0xfffffff],           bits : 27_0
*/
#define R48_VERTICAL_SYNC_OFS 0x0048
REGDEF_BEGIN(R48_VERTICAL_SYNC)
REGDEF_BIT(VD2_ASSERT,       28)
REGDEF_END(R48_VERTICAL_SYNC)


/*
    VD2_FRONTBLANK: [0x0, 0xfffffff],           bits : 27_0
*/
#define R4C_VERTICAL_SYNC_OFS 0x004C
REGDEF_BEGIN(R4C_VERTICAL_SYNC)
REGDEF_BIT(VD2_FRONTBLANK,       28)
REGDEF_END(R4C_VERTICAL_SYNC)


/*
    HD2_PERIOD:     [0x0, 0xffff],              bits : 15_0
    HD2_ASSERT:     [0x0, 0xffff],              bits : 31_16
*/
#define R50_HORIZONTAL_SYNC_OFS 0x0050
REGDEF_BEGIN(R50_HORIZONTAL_SYNC)
REGDEF_BIT(HD2_PERIOD,       16)
REGDEF_BIT(HD2_ASSERT,       16)
REGDEF_END(R50_HORIZONTAL_SYNC)


/*
    HD2_COUNT:      [0x0, 0xfffffff],           bits : 27_0
*/
#define R54_HORIZONTAL_SYNC_OFS 0x0054
REGDEF_BEGIN(R54_HORIZONTAL_SYNC)
REGDEF_BIT(HD2_COUNT,       28)
REGDEF_END(R54_HORIZONTAL_SYNC)


/*
    VD2_BP:         [0x0, 0xfffffff],           bits : 27_0
*/
#define R58_BREAKPOINT_OFS 0x0058
REGDEF_BEGIN(R58_BREAKPOINT)
REGDEF_BIT(VD2_BP,       28)
REGDEF_END(R58_BREAKPOINT)

/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define R5C_RESERVED_OFS 0x005C
REGDEF_BEGIN(R5C_RESERVED)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(R5C_RESERVED)


/*
    VD3_PERIOD:     [0x0, 0xfffffff],           bits : 27_0
*/
#define R60_VERTICAL_SYNC_OFS 0x0060
REGDEF_BEGIN(R60_VERTICAL_SYNC)
REGDEF_BIT(VD3_PERIOD,       28)
REGDEF_END(R60_VERTICAL_SYNC)


/*
    VD3_ASSERT:     [0x0, 0xfffffff],           bits : 27_0
*/
#define R64_VERTICAL_SYNC_OFS 0x0064
REGDEF_BEGIN(R64_VERTICAL_SYNC)
REGDEF_BIT(VD3_ASSERT,       28)
REGDEF_END(R64_VERTICAL_SYNC)


/*
    VD3_FRONTBLANK: [0x0, 0xfffffff],           bits : 27_0
*/
#define R68_VERTICAL_SYNC_OFS 0x0068
REGDEF_BEGIN(R68_VERTICAL_SYNC)
REGDEF_BIT(VD3_FRONTBLANK,       28)
REGDEF_END(R68_VERTICAL_SYNC)


/*
    HD3_PERIOD:     [0x0, 0xffff],              bits : 15_0
    HD3_ASSERT:     [0x0, 0xffff],              bits : 31_16
*/
#define R6C_HORIZONTAL_SYNC_OFS 0x006C
REGDEF_BEGIN(R6C_HORIZONTAL_SYNC)
REGDEF_BIT(HD3_PERIOD,       16)
REGDEF_BIT(HD3_ASSERT,       16)
REGDEF_END(R6C_HORIZONTAL_SYNC)


/*
    HD3_COUNT:      [0x0, 0xfffffff],           bits : 27_0
*/
#define R70_HORIZONTAL_SYNC_OFS 0x0070
REGDEF_BEGIN(R70_HORIZONTAL_SYNC)
REGDEF_BIT(HD3_COUNT,       28)
REGDEF_END(R70_HORIZONTAL_SYNC)


/*
    VD3_BP:         [0x0, 0xfffffff],           bits : 27_0
*/
#define R74_BREAKPOINT_OFS 0x0074
REGDEF_BEGIN(R74_BREAKPOINT)
REGDEF_BIT(VD3_BP,       28)
REGDEF_END(R74_BREAKPOINT)

/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define R78_RESERVED_OFS 0x0078
REGDEF_BEGIN(R78_RESERVED)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(R78_RESERVED)


/*
    VD4_PERIOD:     [0x0, 0xfffffff],           bits : 27_0
*/
#define R7C_VERTICAL_SYNC_OFS 0x007C
REGDEF_BEGIN(R7C_VERTICAL_SYNC)
REGDEF_BIT(VD4_PERIOD,       28)
REGDEF_END(R7C_VERTICAL_SYNC)


/*
    VD4_ASSERT:     [0x0, 0xfffffff],           bits : 27_0
*/
#define R80_VERTICAL_SYNC_OFS 0x0080
REGDEF_BEGIN(R80_VERTICAL_SYNC)
REGDEF_BIT(VD4_ASSERT,       28)
REGDEF_END(R80_VERTICAL_SYNC)


/*
    VD4_FRONTBLANK: [0x0, 0xfffffff],           bits : 27_0
*/
#define R84_VERTICAL_SYNC_OFS 0x0084
REGDEF_BEGIN(R84_VERTICAL_SYNC)
REGDEF_BIT(VD4_FRONTBLANK,       28)
REGDEF_END(R84_VERTICAL_SYNC)


/*
    HD4_PERIOD:     [0x0, 0xffff],              bits : 15_0
    HD4_ASSERT:     [0x0, 0xffff],              bits : 31_16
*/
#define R88_HORIZONTAL_SYNC_OFS 0x0088
REGDEF_BEGIN(R88_HORIZONTAL_SYNC)
REGDEF_BIT(HD4_PERIOD,       16)
REGDEF_BIT(HD4_ASSERT,       16)
REGDEF_END(R88_HORIZONTAL_SYNC)


/*
    HD4_COUNT:      [0x0, 0xfffffff],           bits : 27_0
*/
#define R8C_HORIZONTAL_SYNC_OFS 0x008C
REGDEF_BEGIN(R8C_HORIZONTAL_SYNC)
REGDEF_BIT(HD4_COUNT,       28)
REGDEF_END(R8C_HORIZONTAL_SYNC)


/*
    VD4_BP:         [0x0, 0xfffffff],           bits : 27_0
*/
#define R90_BREAKPOINT_OFS 0x0090
REGDEF_BEGIN(R90_BREAKPOINT)
REGDEF_BIT(VD4_BP,       28)
REGDEF_END(R90_BREAKPOINT)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define R94_RESERVED_OFS 0x0094
REGDEF_BEGIN(R94_RESERVED)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(R94_RESERVED)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define R98_VERTICAL_SYNC_OFS 0x0098
REGDEF_BEGIN(R98_VERTICAL_SYNC)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(R98_VERTICAL_SYNC)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define R9C_VERTICAL_SYNC_OFS 0x009C
REGDEF_BEGIN(R9C_VERTICAL_SYNC)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(R9C_VERTICAL_SYNC)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RA0_VERTICAL_SYNC_OFS 0x00A0
REGDEF_BEGIN(RA0_VERTICAL_SYNC)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RA0_VERTICAL_SYNC)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RA4_HORIZONTAL_SYNC_OFS 0x00A4
REGDEF_BEGIN(RA4_HORIZONTAL_SYNC)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RA4_HORIZONTAL_SYNC)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RA8_HORIZONTAL_SYNC_OFS 0x00A8
REGDEF_BEGIN(RA8_HORIZONTAL_SYNC)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RA8_HORIZONTAL_SYNC)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RAC_BREAKPOINT_OFS 0x00AC
REGDEF_BEGIN(RAC_BREAKPOINT)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RAC_BREAKPOINT)

/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RB0_RESERVED_OFS 0x00B0
REGDEF_BEGIN(RB0_RESERVED)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RB0_RESERVED)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RB4_VERTICAL_SYNC_OFS 0x00B4
REGDEF_BEGIN(RB4_VERTICAL_SYNC)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RB4_VERTICAL_SYNC)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RB8_VERTICAL_SYNC_OFS 0x00B8
REGDEF_BEGIN(RB8_VERTICAL_SYNC)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RB8_VERTICAL_SYNC)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RBC_VERTICAL_SYNC_OFS 0x00BC
REGDEF_BEGIN(RBC_VERTICAL_SYNC)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RBC_VERTICAL_SYNC)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RC0_HORIZONTAL_SYNC_OFS 0x00C0
REGDEF_BEGIN(RC0_HORIZONTAL_SYNC)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RC0_HORIZONTAL_SYNC)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RC4_HORIZONTAL_SYNC_OFS 0x00C4
REGDEF_BEGIN(RC4_HORIZONTAL_SYNC)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RC4_HORIZONTAL_SYNC)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RC8_BREAKPOINT_OFS 0x00C8
REGDEF_BEGIN(RC8_BREAKPOINT)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RC8_BREAKPOINT)

/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RCC_RESERVED_OFS 0x00CC
REGDEF_BEGIN(RCC_RESERVED)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RCC_RESERVED)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RD0_VERTICAL_SYNC_OFS 0x00D0
REGDEF_BEGIN(RD0_VERTICAL_SYNC)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RD0_VERTICAL_SYNC)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RD4_VERTICAL_SYNC_OFS 0x00D4
REGDEF_BEGIN(RD4_VERTICAL_SYNC)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RD4_VERTICAL_SYNC)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RD8_VERTICAL_SYNC_OFS 0x00D8
REGDEF_BEGIN(RD8_VERTICAL_SYNC)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RD8_VERTICAL_SYNC)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RDC_HORIZONTAL_SYNC_OFS 0x00DC
REGDEF_BEGIN(RDC_HORIZONTAL_SYNC)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RDC_HORIZONTAL_SYNC)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RE0_HORIZONTAL_SYNC_OFS 0x00E0
REGDEF_BEGIN(RE0_HORIZONTAL_SYNC)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RE0_HORIZONTAL_SYNC)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RE4_BREAKPOINT_OFS 0x00E4
REGDEF_BEGIN(RE4_BREAKPOINT)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RE4_BREAKPOINT)

/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RE8_RESERVED_OFS 0x00E8
REGDEF_BEGIN(RE8_RESERVED)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RE8_RESERVED)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define REC_VERTICAL_SYNC_OFS 0x00EC
REGDEF_BEGIN(REC_VERTICAL_SYNC)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(REC_VERTICAL_SYNC)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RF0_VERTICAL_SYNC_OFS 0x00F0
REGDEF_BEGIN(RF0_VERTICAL_SYNC)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RF0_VERTICAL_SYNC)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RF4_VERTICAL_SYNC_OFS 0x00F4
REGDEF_BEGIN(RF4_VERTICAL_SYNC)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RF4_VERTICAL_SYNC)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RF8_HORIZONTAL_SYNC_OFS 0x00F8
REGDEF_BEGIN(RF8_HORIZONTAL_SYNC)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RF8_HORIZONTAL_SYNC)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define RFC_HORIZONTAL_SYNC_OFS 0x00FC
REGDEF_BEGIN(RFC_HORIZONTAL_SYNC)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(RFC_HORIZONTAL_SYNC)


/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define R100_BREAKPOINT_OFS 0x0100
REGDEF_BEGIN(R100_BREAKPOINT)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(R100_BREAKPOINT)

/*
    Reserved:       [0x0, 0xffffffff],          bits : 31_0
*/
#define R104_RESERVED_OFS 0x0104
REGDEF_BEGIN(R104_RESERVED)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(R104_RESERVED)


/*
    CLK_CNT      :  [0x0, 0xff],               bits : 7
    VD           :  [0x0, 0x1],                bits : 8
    HD           :  [0x0, 0x1],                bits : 9
    VD2          :  [0x0, 0x1],                bits :10
    HD2          :  [0x0, 0x1],                bits :11
    VD3          :  [0x0, 0x1],                bits :12
    HD3          :  [0x0, 0x1],                bits :13
    VD4          :  [0x0, 0x1],                bits :14
    HD4          :  [0x0, 0x1],                bits :15

    FLSHA_EXT_TRG:  [0x0, 0x1],                bits :24
    MSHA_EXT_TRG :  [0x0, 0x1],                bits :25

    MSH_CTRL_A0  :  [0x0, 0x1],                bits :28
    MSH_CTRL_A1  :  [0x0, 0x1],                bits :29
*/
#define R108_STATUS_OFS 0x0108
REGDEF_BEGIN(R108_STATUS)
REGDEF_BIT(CLK_CNT,       8)
REGDEF_BIT(VD,            1)
REGDEF_BIT(HD,            1)
REGDEF_BIT(VD2,           1)
REGDEF_BIT(HD2,           1)
REGDEF_BIT(VD3,           1)
REGDEF_BIT(HD3,           1)
REGDEF_BIT(VD4,           1)
REGDEF_BIT(HD4,           1)
REGDEF_BIT(,              8)
REGDEF_BIT(FLSHA_EXT_TRG, 1)
REGDEF_BIT(MSHA_EXT_TRG,  1)
REGDEF_BIT(,              2)
REGDEF_BIT(MSHA_CTRL_0,   1)
REGDEF_BIT(MSHA_CTRL_1,   1)
REGDEF_END(R108_STATUS)


/*
    FLSHA_IMD_TRG       : [0x0, 0x1],           bits : 0
    FLSHA_WAT_TRG       : [0x0, 0x1],           bits : 1
    FLSHA_WAT_MODE      : [0x0, 0x1],           bits : 2
    FLSHA_ASSERT        : [0x0, 0xfffff],       bits : 23_4
    FLSHA_CONT_WAT_TRG  : [0x0, 0x1],           bits : 24
*/
#define R10C_FLASH_A_LIGHT_OFS 0x010C
REGDEF_BEGIN(R10C_FLASH_A_LIGHT)
REGDEF_BIT(FLSHA_IMD_TRG,       1)
REGDEF_BIT(FLSHA_WAT_TRG,       1)
REGDEF_BIT(FLSHA_WAT_MODE,      1)
REGDEF_BIT(,                    1)
REGDEF_BIT(FLSHA_ASSERT,       20)
REGDEF_BIT(FLSHA_CONT_WAT_TRG,  1)
REGDEF_END(R10C_FLASH_A_LIGHT)


/*
    FLSHA_DELAY:    [0x0, 0xfffffff],           bits : 27_0
*/
#define R110_FLASH_A_LIGHT_OFS 0x0110
REGDEF_BEGIN(R110_FLASH_A_LIGHT)
REGDEF_BIT(FLSHA_DELAY,        28)
REGDEF_END(R110_FLASH_A_LIGHT)


/*
    FLSHA_PERIOD:    [0x0, 0xff],               bits : 7_0
*/
#define R114_FLASH_A_LIGHT_OFS 0x0114
REGDEF_BEGIN(R114_FLASH_A_LIGHT)
REGDEF_BIT(FLSHA_PERIOD,       8)
REGDEF_END(R114_FLASH_A_LIGHT)


/*
    Reserved:    [0x0, 0xffffffff],             bits : 31_0
*/
#define R118_RESERVED_OFS 0x0118
REGDEF_BEGIN(R118_RESERVED)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(R118_RESERVED)


/*
    Reserved:    [0x0, 0xffffffff],             bits : 31_0
*/
#define R11C_RESERVED_OFS 0x011C
REGDEF_BEGIN(R11C_RESERVED)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(R11C_RESERVED)


/*
    MSHA_CLOSE_IMD_TRG      :   [0x0, 0x1],         bits : 0
    MSHA_CLOSE_WAT_TRG      :   [0x0, 0x1],         bits : 1
    MSHA_CLOSE_WAT_MODE     :   [0x0, 0x1],         bits : 2
    MSHA_CLOSE_ASSERT       :   [0x0, 0x7fffff],    bits : 26_4
    MSHA_CLOSE_CONT_WAT_TRG :   [0x0, 0x1],         bits : 28
*/
#define R120_MECHANICAL_SHUTTER_A_OFS 0x0120
REGDEF_BEGIN(R120_MECHANICAL_SHUTTER_A)
REGDEF_BIT(MSHA_CLOSE_IMD_TRG,       1)
REGDEF_BIT(MSHA_CLOSE_WAT_TRG,       1)
REGDEF_BIT(MSHA_CLOSE_WAT_MODE,      1)
REGDEF_BIT(,                         1)
REGDEF_BIT(MSHA_CLOSE_ASSERT,       23)
REGDEF_BIT(,                         1)
REGDEF_BIT(MSHA_CLOSE_CONT_WAT_TRG,  1)
REGDEF_END(R120_MECHANICAL_SHUTTER_A)


/*
    MSHA_CLOSE_DELAY:   [0x0, 0xfffffff],       bits : 27_0
*/
#define R124_MECHANICAL_SHUTTER_A_OFS 0x0124
REGDEF_BEGIN(R124_MECHANICAL_SHUTTER_A)
REGDEF_BIT(MSHA_CLOSE_DELAY,        28)
REGDEF_END(R124_MECHANICAL_SHUTTER_A)


/*
    MSHA_OPEN_IMD_TRG     :    [0x0, 0x1],      bits : 0
    MSHA_OPEN_WAT_TRG     :    [0x0, 0x1],      bits : 1
    MSHA_OPEN_WAT_MODE    :    [0x0, 0x1],      bits : 2
    MSHA_OPEN_ASSERT      :    [0x0, 0x7fffff], bits : 26_4
    MSHA_OPEN_CONT_WAT_TRG:    [0x0, 0x1],      bits : 28
*/
#define R128_MECHANICAL_SHUTTER_A_OFS 0x0128
REGDEF_BEGIN(R128_MECHANICAL_SHUTTER_A)
REGDEF_BIT(MSHA_OPEN_IMD_TRG,        1)
REGDEF_BIT(MSHA_OPEN_WAT_TRG,        1)
REGDEF_BIT(MSHA_OPEN_WAT_MODE,       1)
REGDEF_BIT(,                         1)
REGDEF_BIT(MSHA_OPEN_ASSERT,        23)
REGDEF_BIT(,                         1)
REGDEF_BIT(MSHA_OPEN_CONT_WAT_TRG,   1)
REGDEF_END(R128_MECHANICAL_SHUTTER_A)


/*
    MSHA_OPEN_DELAY:    [0x0, 0xfffffff],       bits : 27_0
*/
#define R12C_MECHANICAL_SHUTTER_A_OFS 0x012C
REGDEF_BEGIN(R12C_MECHANICAL_SHUTTER_A)
REGDEF_BIT(MSHA_OPEN_DELAY,        28)
REGDEF_END(R12C_MECHANICAL_SHUTTER_A)


/*
    MSHA_CLOSE_PERIOD:    [0x0, 0xff],          bits : 7_0
    MSHA_OPEN_PERIOD:     [0x0, 0xff],          bits : 23_16
*/
#define R130_MECHANICAL_SHUTTER_A_OFS 0x0130
REGDEF_BEGIN(R130_MECHANICAL_SHUTTER_A)
REGDEF_BIT(MSHA_CLOSE_PERIOD,  8)
REGDEF_BIT(,                   8)
REGDEF_BIT(MSHA_OPEN_PERIOD,   8)
REGDEF_END(R130_MECHANICAL_SHUTTER_A)

/*
    Reserved:    [0x0, 0xffffffff],             bits : 31_0
*/
#define R134_RESERVED_OFS 0x0134
REGDEF_BEGIN(R134_RESERVED)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(R134_RESERVED)


/*
    Reserved:    [0x0, 0xffffffff],             bits : 31_0
*/
#define R138_RESERVED_OFS 0x0138
REGDEF_BEGIN(R138_RESERVED)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(R138_RESERVED)


/*
    Reserved:    [0x0, 0xffffffff],             bits : 31_0
*/
#define R13C_RESERVED_OFS 0x013C
REGDEF_BEGIN(R13C_RESERVED)
REGDEF_BIT(RESERVED,        32)
REGDEF_END(R13C_RESERVED)


typedef struct {
    //0x0000
    T_R0_ENGINE_CONTROL
    TGE_Register_0000;

    //0x0004
    T_R4_ENGINE_CONTROL
    TGE_Register_0004;

    //0x0008
    T_R8_ENGINE_CONTROL
    TGE_Register_0008;

    //0x000c
    T_RC_ENGINE_CONTROL
    TGE_Register_000c;

    //0x0010
    T_R10_ENGINE_CONTROL
    TGE_Register_0010;

    //0x0014
    T_R14_INTERRUPT_EN
    TGE_Register_0014;

    //0x0018
    T_R18_INTERRUPT
    TGE_Register_0018;

    //0x001c
    T_R1C_DEBUG
    TGE_Register_001c;

    //0x0020
    T_R20_FLASH_STATUS
    TGE_Register_0020;

    //0x0024
    T_R24_RESERVED
    TGE_Register_0024;

    //0x0028
    T_R28_VERTICAL_SYNC
    TGE_Register_0028;

    //0x002c
    T_R2C_VERTICAL_SYNC
    TGE_Register_002c;

    //0x0030
    T_R30_VERTICAL_SYNC
    TGE_Register_0030;

    //0x0034
    T_R34_HORIZONTAL_SYNC
    TGE_Register_0034;

    //0x0038
    T_R38_HORIZONTAL_SYNC
    TGE_Register_0038;

    //0x003c
    T_R3C_BREAKPOINT
    TGE_Register_003c;

    //0x0040
    T_R40_RESERVED
    TGE_Register_0040;

    //0x0044
    T_R44_VERTICAL_SYNC
    TGE_Register_0044;

    //0x0048
    T_R48_VERTICAL_SYNC
    TGE_Register_0048;

    //0x004c
    T_R4C_VERTICAL_SYNC
    TGE_Register_004c;

    //0x0050
    T_R50_HORIZONTAL_SYNC
    TGE_Register_0050;

    //0x0054
    T_R54_HORIZONTAL_SYNC
    TGE_Register_0054;

    //0x0058
    T_R58_BREAKPOINT
    TGE_Register_0058;

    //0x005c
    T_R5C_RESERVED
    TGE_Register_005c;

    //0x0060
    T_R60_VERTICAL_SYNC
    TGE_Register_0060;

    //0x0064
    T_R64_VERTICAL_SYNC
    TGE_Register_0064;

    //0x0068
    T_R68_VERTICAL_SYNC
    TGE_Register_0068;

    //0x006c
    T_R6C_HORIZONTAL_SYNC
    TGE_Register_006c;

    //0x0070
    T_R70_HORIZONTAL_SYNC
    TGE_Register_0070;

    //0x0074
    T_R74_BREAKPOINT
    TGE_Register_0074;

    //0x0078
    T_R78_RESERVED
    TGE_Register_0078;

    //0x007c
    T_R7C_VERTICAL_SYNC
    TGE_Register_007c;

    //0x0080
    T_R80_VERTICAL_SYNC
    TGE_Register_0080;

    //0x0084
    T_R84_VERTICAL_SYNC
    TGE_Register_0084;

    //0x0088
    T_R88_HORIZONTAL_SYNC
    TGE_Register_0088;

    //0x008c
    T_R8C_HORIZONTAL_SYNC
    TGE_Register_008c;

    //0x0090
    T_R90_BREAKPOINT
    TGE_Register_0090;

    //0x0094
    T_R94_RESERVED
    TGE_Register_0094;

    //0x0098
    T_R98_VERTICAL_SYNC
    TGE_Register_0098;

    //0x009c
    T_R9C_VERTICAL_SYNC
    TGE_Register_009c;

    //0x00a0
    T_RA0_VERTICAL_SYNC
    TGE_Register_00a0;

    //0x00a4
    T_RA4_HORIZONTAL_SYNC
    TGE_Register_00a4;

    //0x00a8
    T_RA8_HORIZONTAL_SYNC
    TGE_Register_00a8;

    //0x00ac
    T_RAC_BREAKPOINT
    TGE_Register_00ac;

    //0x00b0
    T_RB0_RESERVED
    TGE_Register_00b0;

    //0x00b4
    T_RB4_VERTICAL_SYNC
    TGE_Register_00b4;

    //0x00b8
    T_RB8_VERTICAL_SYNC
    TGE_Register_00b8;

    //0x00bc
    T_RBC_VERTICAL_SYNC
    TGE_Register_00bc;

    //0x00c0
    T_RC0_HORIZONTAL_SYNC
    TGE_Register_00c0;

    //0x00c4
    T_RC4_HORIZONTAL_SYNC
    TGE_Register_00c4;

    //0x00c8
    T_RC8_BREAKPOINT
    TGE_Register_00c8;

    //0x00cc
    T_RCC_RESERVED
    TGE_Register_00cc;

    //0x00d0
    T_RD0_VERTICAL_SYNC
    TGE_Register_00d0;

    //0x00d4
    T_RD4_VERTICAL_SYNC
    TGE_Register_00d4;

    //0x00d8
    T_RD8_VERTICAL_SYNC
    TGE_Register_00d8;

    //0x00dc
    T_RDC_HORIZONTAL_SYNC
    TGE_Register_00dc;

    //0x00e0
    T_RE0_HORIZONTAL_SYNC
    TGE_Register_00e0;

    //0x00e4
    T_RE4_BREAKPOINT
    TGE_Register_00e4;

    //0x00e8
    T_RE8_RESERVED
    TGE_Register_00e8;

    //0x00ec
    T_REC_VERTICAL_SYNC
    TGE_Register_00ec;

    //0x00f0
    T_RF0_VERTICAL_SYNC
    TGE_Register_00f0;

    //0x00f4
    T_RF4_VERTICAL_SYNC
    TGE_Register_00f4;

    //0x00f8
    T_RF8_HORIZONTAL_SYNC
    TGE_Register_00f8;

    //0x00fc
    T_RFC_HORIZONTAL_SYNC
    TGE_Register_00fc;

    //0x0100
    T_R100_BREAKPOINT
    TGE_Register_0100;

    //0x0104
    T_R104_RESERVED
    TGE_Register_0104;

    //0x0108
    T_R108_STATUS
    TGE_Register_0108;

    //0x010c
    T_R10C_FLASH_A_LIGHT
    TGE_Register_010c;

    //0x0110
    T_R110_FLASH_A_LIGHT
    TGE_Register_0110;

    //0x0114
    T_R114_FLASH_A_LIGHT
    TGE_Register_0114;

    //0x0118
    T_R118_RESERVED
    TGE_Register_0118;

    //0x011c
    T_R11C_RESERVED
    TGE_Register_011c;

    //0x0120
    T_R120_MECHANICAL_SHUTTER_A
    TGE_Register_0120;

    //0x0124
    T_R124_MECHANICAL_SHUTTER_A
    TGE_Register_00124;

    //0x0128
    T_R128_MECHANICAL_SHUTTER_A
    TGE_Register_0128;

    //0x012C
    T_R12C_MECHANICAL_SHUTTER_A
    TGE_Register_012c;

    //0x0130
    T_R130_MECHANICAL_SHUTTER_A
    TGE_Register_0130;

    //0x0134
    T_R134_RESERVED
    TGE_Register_0134;

    //0x0138
    T_R138_RESERVED
    TGE_Register_0138;

    //0x013C
    T_R13C_RESERVED
    TGE_Register_013c;

} TGE_REG_STRUCT;

#endif /* _TGE_REG_H_ */
