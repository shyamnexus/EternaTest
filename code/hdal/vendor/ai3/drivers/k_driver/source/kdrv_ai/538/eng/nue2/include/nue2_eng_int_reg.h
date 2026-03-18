#ifndef _NUE2_ENG_INT_REGISTER_H_
#define _NUE2_ENG_INT_REGISTER_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined (__LINUX)
#include <linux/types.h>
#include <linux/soc/nvt/nvt-io.h>
#include "plat/top.h"
#include "kwrap/type.h"
#include "kwrap/nvt_type.h"

//=========================================================================
#elif defined (__FREERTOS)

#include "rcw_macro.h"
#include "top.h"
#include "kwrap/type.h"

#include "kwrap/nvt_type.h"
#else
#endif

#define REGVALUE                uint32_t
#define UBITFIELD		unsigned int 	/* Unsigned bit field */
#define BITFIELD 		signed int	/* Signed bit field */

#define ASSERT_CONCAT_(a, b)    a##b
#define ASSERT_CONCAT(a, b)     ASSERT_CONCAT_(a, b)

#if defined(__COUNTER__)
#define STATIC_ASSERT(expr) \
enum { ASSERT_CONCAT(FAILED_STATIC_ASSERT_, __COUNTER__) = 1/(expr) }
#else
#define STATIC_ASSERT(expr) \
enum { ASSERT_CONCAT(FAILED_STATIC_ASSERT_, __LINE__) = 1/(expr) }
#endif

#define REGDEF_BEGIN(name)      \
typedef union                   \
{                               \
    REGVALUE    reg;            \
    struct                      \
    {

#define REGDEF_BIT(field, bits) \
    UBITFIELD   field : bits;

#define REGDEF_END(name)        \
    } bit;                      \
} T_##name;                     \
STATIC_ASSERT(sizeof(T_##name) == sizeof(REGVALUE));


/*
    NUE2_RST  :    [0x0, 0x1],			bits : 0
    NUE2_START:    [0x0, 0x1],			bits : 1
    LL_FIRE   :    [0x0, 0x1],			bits : 28
*/
#define NUE2_CONTROL_REGISTER_OFS 0x0000
REGDEF_BEGIN(NUE2_CONTROL_REGISTER)
    REGDEF_BIT(NUE2_RST  ,        1)
    REGDEF_BIT(NUE2_START,        1)
    REGDEF_BIT(          ,        26)
    REGDEF_BIT(LL_FIRE   ,        1)
REGDEF_END(NUE2_CONTROL_REGISTER)


/*
    NUE2_YUV2RGB_EN           :    [0x0, 0x1],			bits : 0
    NUE2_SUB_EN               :    [0x0, 0x1],			bits : 1
    NUE2_PAD_EN               :    [0x0, 0x1],			bits : 2
    NUE2_ROTATE_EN            :    [0x0, 0x1],			bits : 4
    NUE2_IN_FMT               :    [0x0, 0x3],			bits : 13_12
    NUE2_OUT_SIGNEDNESS       :    [0x0, 0x1],			bits : 14
    NUE2_SUB_MODE             :    [0x0, 0x1],			bits : 15
    NUE2_ROTATE_MODE          :    [0x0, 0x3],			bits : 18_17
    NUE2_FLIP_MODE            :    [0x0, 0x3],			bits : 20_19
    NUE2_YUV_MODE             :    [0x0, 0x7],			bits : 23_21
    NUE2_SCALE_H_MODE         :    [0x0, 0x1],			bits : 24
    NUE2_SCALE_V_MODE         :    [0x0, 0x1],			bits : 25
    NUE2_MEAN_SCALE_SHIFT_MODE:    [0x0, 0x1],			bits : 26
*/
#define NUE2_FUNCTION_ENABLE_REGISTER0_OFS 0x0004
REGDEF_BEGIN(NUE2_FUNCTION_ENABLE_REGISTER0)
    REGDEF_BIT(NUE2_YUV2RGB_EN           ,        1)
    REGDEF_BIT(NUE2_SUB_EN               ,        1)
    REGDEF_BIT(NUE2_PAD_EN               ,        1)
    REGDEF_BIT(NUE2_HSV_EN               ,        1)
    REGDEF_BIT(NUE2_ROTATE_EN            ,        1)
    REGDEF_BIT(                          ,        3)
    REGDEF_BIT(NUE2_HSV_OUT_FMT          ,        1)
    REGDEF_BIT(                          ,        3)
    REGDEF_BIT(NUE2_IN_FMT               ,        2)
    REGDEF_BIT(NUE2_OUT_SIGNEDNESS       ,        1)
    REGDEF_BIT(NUE2_SUB_MODE             ,        1)
    REGDEF_BIT(NUE2_HSV_OUT_MODE         ,        1)
    REGDEF_BIT(NUE2_ROTATE_MODE          ,        2)
    REGDEF_BIT(NUE2_FLIP_MODE            ,        2)
    REGDEF_BIT(NUE2_YUV_MODE             ,        3)
    REGDEF_BIT(NUE2_SCALE_H_MODE         ,        1)
    REGDEF_BIT(NUE2_SCALE_V_MODE         ,        1)
    REGDEF_BIT(NUE2_MEAN_SCALE_SHIFT_MODE,        1)
REGDEF_END(NUE2_FUNCTION_ENABLE_REGISTER0)


/*
    DRAM_SAI0:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_NUE2_REGISTER0_OFS 0x0008
REGDEF_BEGIN(DMA_TO_NUE2_REGISTER0)
    REGDEF_BIT(DRAM_SAI0,        32)
REGDEF_END(DMA_TO_NUE2_REGISTER0)


/*
    DRAM_SAI1:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_NUE2_REGISTER1_OFS 0x000c
REGDEF_BEGIN(DMA_TO_NUE2_REGISTER1)
    REGDEF_BIT(DRAM_SAI1,        32)
REGDEF_END(DMA_TO_NUE2_REGISTER1)


/*
    DRAM_SAI2:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_NUE2_REGISTER2_OFS 0x0010
REGDEF_BEGIN(DMA_TO_NUE2_REGISTER2)
    REGDEF_BIT(DRAM_SAI2,        32)
REGDEF_END(DMA_TO_NUE2_REGISTER2)


/*
    DRAM_SAILL:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_NUE2_REGISTER3_OFS 0x0014
REGDEF_BEGIN(DMA_TO_NUE2_REGISTER3)
    REGDEF_BIT(DRAM_SAILL,        32)
REGDEF_END(DMA_TO_NUE2_REGISTER3)


/*
    DRAM_SAO0:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_TO_DMA_RESULT_REGISTER0_OFS 0x0018
REGDEF_BEGIN(NUE2_TO_DMA_RESULT_REGISTER0)
    REGDEF_BIT(DRAM_SAO0,        32)
REGDEF_END(NUE2_TO_DMA_RESULT_REGISTER0)


/*
    DRAM_SAO1:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_TO_DMA_RESULT_REGISTER1_OFS 0x001c
REGDEF_BEGIN(NUE2_TO_DMA_RESULT_REGISTER1)
    REGDEF_BIT(DRAM_SAO1,        32)
REGDEF_END(NUE2_TO_DMA_RESULT_REGISTER1)


/*
    DRAM_SAO2:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_TO_DMA_RESULT_REGISTER2_OFS 0x0020
REGDEF_BEGIN(NUE2_TO_DMA_RESULT_REGISTER2)
    REGDEF_BIT(DRAM_SAO2,        32)
REGDEF_END(NUE2_TO_DMA_RESULT_REGISTER2)


/*
    DRAM_OFSI0:    [0x0, 0x1ffff],			bits : 16_0
*/
#define NUE2_INPUT_LINE_OFFSET_REGISTER0_OFS 0x0024
REGDEF_BEGIN(NUE2_INPUT_LINE_OFFSET_REGISTER0)
    REGDEF_BIT(DRAM_OFSI0,        17)
REGDEF_END(NUE2_INPUT_LINE_OFFSET_REGISTER0)


/*
    DRAM_OFSI1:    [0x0, 0x1ffff],			bits : 16_0
*/
#define NUE2_INPUT_LINE_OFFSET_REGISTER1_OFS 0x0028
REGDEF_BEGIN(NUE2_INPUT_LINE_OFFSET_REGISTER1)
    REGDEF_BIT(DRAM_OFSI1,        17)
REGDEF_END(NUE2_INPUT_LINE_OFFSET_REGISTER1)


/*
    DRAM_OFSI2:    [0x0, 0x1ffff],			bits : 16_0
*/
#define NUE2_INPUT_LINE_OFFSET_REGISTER2_OFS 0x002c
REGDEF_BEGIN(NUE2_INPUT_LINE_OFFSET_REGISTER2)
    REGDEF_BIT(DRAM_OFSI2,        17)
REGDEF_END(NUE2_INPUT_LINE_OFFSET_REGISTER2)


/*
    DRAM_OFSO0:    [0x0, 0x1ffff],			bits : 16_0
*/
#define NUE2_OUTPUT_LINE_OFFSET_REGISTER0_OFS 0x0030
REGDEF_BEGIN(NUE2_OUTPUT_LINE_OFFSET_REGISTER0)
    REGDEF_BIT(DRAM_OFSO0,        17)
REGDEF_END(NUE2_OUTPUT_LINE_OFFSET_REGISTER0)


/*
    DRAM_OFSO1:    [0x0, 0x1ffff],			bits : 16_0
*/
#define NUE2_OUTPUT_LINE_OFFSET_REGISTER1_OFS 0x0034
REGDEF_BEGIN(NUE2_OUTPUT_LINE_OFFSET_REGISTER1)
    REGDEF_BIT(DRAM_OFSO1,        17)
REGDEF_END(NUE2_OUTPUT_LINE_OFFSET_REGISTER1)


/*
    DRAM_OFSO2:    [0x0, 0x1ffff],			bits : 16_0
*/
#define NUE2_OUTPUT_LINE_OFFSET_REGISTER2_OFS 0x0038
REGDEF_BEGIN(NUE2_OUTPUT_LINE_OFFSET_REGISTER2)
    REGDEF_BIT(DRAM_OFSO2,        17)
REGDEF_END(NUE2_OUTPUT_LINE_OFFSET_REGISTER2)


/*
    INTE_FRM_END           :    [0x0, 0x1],			bits : 0
    INTE_DMAIN0END         :    [0x0, 0x1],			bits : 1
    INTE_DMAIN1END         :    [0x0, 0x1],			bits : 2
    INTE_DMAIN2END         :    [0x0, 0x1],			bits : 3
    INTE_LLEND             :    [0x0, 0x1],			bits : 8
    INTE_LLERROR           :    [0x0, 0x1],			bits : 9
    INTE_LLJOBEND          :    [0x0, 0x1],			bits : 10
    INTE_SW_RESET          :    [0x0, 0x1],			bits : 16
    INTE_CHECKSUM_MISMATCH0:    [0x0, 0x1],			bits : 17
    INTE_CHECKSUM_MISMATCH1:    [0x0, 0x1],			bits : 18
    INTE_CHECKSUM_MISMATCH2:    [0x0, 0x1],			bits : 19
*/
#define NUE2_INTERRUPT_ENABLE_REGISTER_OFS 0x003c
REGDEF_BEGIN(NUE2_INTERRUPT_ENABLE_REGISTER)
    REGDEF_BIT(INTE_FRM_END           ,        1)
    REGDEF_BIT(INTE_DMAIN0END         ,        1)
    REGDEF_BIT(INTE_DMAIN1END         ,        1)
    REGDEF_BIT(INTE_DMAIN2END         ,        1)
    REGDEF_BIT(                       ,        4)
    REGDEF_BIT(INTE_LLEND             ,        1)
    REGDEF_BIT(INTE_LLERROR           ,        1)
    REGDEF_BIT(INTE_LLJOBEND          ,        1)
    REGDEF_BIT(                       ,        5)
    REGDEF_BIT(INTE_SW_RESET          ,        1)
    REGDEF_BIT(INTE_CHECKSUM_MISMATCH0,        1)
    REGDEF_BIT(INTE_CHECKSUM_MISMATCH1,        1)
    REGDEF_BIT(INTE_CHECKSUM_MISMATCH2,        1)
REGDEF_END(NUE2_INTERRUPT_ENABLE_REGISTER)


/*
    INT_FRM_END           :    [0x0, 0x1],			bits : 0
    INT_DMAIN0END         :    [0x0, 0x1],			bits : 1
    INT_DMAIN1END         :    [0x0, 0x1],			bits : 2
    INT_DMAIN2END         :    [0x0, 0x1],			bits : 3
    INT_LLEND             :    [0x0, 0x1],			bits : 8
    INT_LLERROR           :    [0x0, 0x1],			bits : 9
    INT_LLJOBEND          :    [0x0, 0x1],			bits : 10
    INT_SW_RESET          :    [0x0, 0x1],			bits : 16
    INT_CHECKSUM_MISMATCH0:    [0x0, 0x1],			bits : 17
    INT_CHECKSUM_MISMATCH1:    [0x0, 0x1],			bits : 18
    INT_CHECKSUM_MISMATCH2:    [0x0, 0x1],			bits : 19
*/
#define NUE2_INTERRUPT_STATUS_REGISTER_OFS 0x0040
REGDEF_BEGIN(NUE2_INTERRUPT_STATUS_REGISTER)
    REGDEF_BIT(INT_FRM_END           ,        1)
    REGDEF_BIT(INT_DMAIN0END         ,        1)
    REGDEF_BIT(INT_DMAIN1END         ,        1)
    REGDEF_BIT(INT_DMAIN2END         ,        1)
    REGDEF_BIT(                      ,        4)
    REGDEF_BIT(INT_LLEND             ,        1)
    REGDEF_BIT(INT_LLERROR           ,        1)
    REGDEF_BIT(INT_LLJOBEND          ,        1)
    REGDEF_BIT(                      ,        5)
    REGDEF_BIT(INT_SW_RESET          ,        1)
    REGDEF_BIT(INT_CHECKSUM_MISMATCH0,        1)
    REGDEF_BIT(INT_CHECKSUM_MISMATCH1,        1)
    REGDEF_BIT(INT_CHECKSUM_MISMATCH2,        1)
REGDEF_END(NUE2_INTERRUPT_STATUS_REGISTER)


/*
    NUE2_IN_WIDTH :    [0x0, 0x1fff],			bits : 12_0
    NUE2_IN_HEIGHT:    [0x0, 0x1fff],			bits : 28_16
*/
#define NUE2_INPUT_SIZE_REGISTER0_OFS 0x0044
REGDEF_BEGIN(NUE2_INPUT_SIZE_REGISTER0)
    REGDEF_BIT(NUE2_IN_WIDTH ,        13)
    REGDEF_BIT(              ,        3)
    REGDEF_BIT(NUE2_IN_HEIGHT,        13)
REGDEF_END(NUE2_INPUT_SIZE_REGISTER0)


/*
    NUE2_H_DNRATE  :    [0x0, 0x7f],			bits : 6_0
    NUE2_V_DNRATE  :    [0x0, 0x7f],			bits : 14_8
    NUE2_H_FILTMODE:    [0x0, 0x1],			bits : 16
    NUE2_H_FILTCOEF:    [0x0, 0x3f],			bits : 22_17
    NUE2_V_FILTMODE:    [0x0, 0x1],			bits : 24
    NUE2_V_FILTCOEF:    [0x0, 0x3f],			bits : 30_25
*/
#define SCALING_RATE_REGISTER0_OFS 0x0048
REGDEF_BEGIN(SCALING_RATE_REGISTER0)
    REGDEF_BIT(NUE2_H_DNRATE  ,        7)
    REGDEF_BIT(               ,        1)
    REGDEF_BIT(NUE2_V_DNRATE  ,        7)
    REGDEF_BIT(               ,        1)
    REGDEF_BIT(NUE2_H_FILTMODE,        1)
    REGDEF_BIT(NUE2_H_FILTCOEF,        6)
    REGDEF_BIT(               ,        1)
    REGDEF_BIT(NUE2_V_FILTMODE,        1)
    REGDEF_BIT(NUE2_V_FILTCOEF,        6)
REGDEF_END(SCALING_RATE_REGISTER0)


/*
    NUE2_H_SFACT:    [0x0, 0xffff],			bits : 15_0
    NUE2_V_SFACT:    [0x0, 0xffff],			bits : 31_16
*/
#define SCALING_RATE_REGISTER1_OFS 0x004c
REGDEF_BEGIN(SCALING_RATE_REGISTER1)
    REGDEF_BIT(NUE2_H_SFACT,        16)
    REGDEF_BIT(NUE2_V_SFACT,        16)
REGDEF_END(SCALING_RATE_REGISTER1)


/*
    NUE2_INI_H_DNRATE:    [0x0, 0x7f],			bits : 6_0
    NUE2_INI_H_SFACT :    [0x0, 0x1ffff],			bits : 31_15
*/
#define SCALING_RATE_REGISTER2_OFS 0x0050
REGDEF_BEGIN(SCALING_RATE_REGISTER2)
    REGDEF_BIT(NUE2_INI_H_DNRATE,        7)
    REGDEF_BIT(                 ,        8)
    REGDEF_BIT(NUE2_INI_H_SFACT ,        17)
REGDEF_END(SCALING_RATE_REGISTER2)


/*
    NUE2_FINAL_H_DNRATE:    [0x0, 0x7f],			bits : 6_0
    NUE2_FINAL_H_SFACT :    [0x0, 0x1ffff],			bits : 31_15
*/
#define SCALING_RATE_REGISTER3_OFS 0x0054
REGDEF_BEGIN(SCALING_RATE_REGISTER3)
    REGDEF_BIT(NUE2_FINAL_H_DNRATE,        7)
    REGDEF_BIT(                   ,        8)
    REGDEF_BIT(NUE2_FINAL_H_SFACT ,        17)
REGDEF_END(SCALING_RATE_REGISTER3)


/*
    NUE2_H_SCL_SIZE:    [0x0, 0x1fff],			bits : 12_0
    NUE2_V_SCL_SIZE:    [0x0, 0x1fff],			bits : 28_16
*/
#define SCALING_OUTPUT_SIZE_REGISTER0_OFS 0x0058
REGDEF_BEGIN(SCALING_OUTPUT_SIZE_REGISTER0)
    REGDEF_BIT(NUE2_H_SCL_SIZE,        13)
    REGDEF_BIT(               ,        3)
    REGDEF_BIT(NUE2_V_SCL_SIZE,        13)
REGDEF_END(SCALING_OUTPUT_SIZE_REGISTER0)


/*
    NUE2_SUB_IN_WIDTH :    [0x0, 0x1fff],			bits : 12_0
    NUE2_SUB_IN_HEIGHT:    [0x0, 0x1fff],			bits : 28_16
*/
#define MEAN_SUBTRACTION_REGISTER0_OFS 0x005c
REGDEF_BEGIN(MEAN_SUBTRACTION_REGISTER0)
    REGDEF_BIT(NUE2_SUB_IN_WIDTH ,        13)
    REGDEF_BIT(                  ,        3)
    REGDEF_BIT(NUE2_SUB_IN_HEIGHT,        13)
REGDEF_END(MEAN_SUBTRACTION_REGISTER0)


/*
    NUE2_SUB_COEF_0:    [0x0, 0xff],			bits : 7_0
    NUE2_SUB_COEF_1:    [0x0, 0xff],			bits : 15_8
    NUE2_SUB_COEF_2:    [0x0, 0xff],			bits : 23_16
    NUE2_SUB_DUP   :    [0x0, 0x3],			bits : 25_24
*/
#define MEAN_SUBTRACTION_REGISTER1_OFS 0x0060
REGDEF_BEGIN(MEAN_SUBTRACTION_REGISTER1)
    REGDEF_BIT(NUE2_SUB_COEF_0,        8)
    REGDEF_BIT(NUE2_SUB_COEF_1,        8)
    REGDEF_BIT(NUE2_SUB_COEF_2,        8)
    REGDEF_BIT(NUE2_SUB_DUP   ,        2)
REGDEF_END(MEAN_SUBTRACTION_REGISTER1)


/*
    NUE2_PAD_CROP_X:    [0x0, 0x1fff],			bits : 12_0
    NUE2_PAD_CROP_Y:    [0x0, 0x1fff],			bits : 28_16
*/
#define PADDING_REGISTER0_OFS 0x0064
REGDEF_BEGIN(PADDING_REGISTER0)
    REGDEF_BIT(NUE2_PAD_CROP_X,        13)
    REGDEF_BIT(               ,        3)
    REGDEF_BIT(NUE2_PAD_CROP_Y,        13)
REGDEF_END(PADDING_REGISTER0)


/*
    NUE2_PAD_CROP_WIDTH :    [0x0, 0x1fff],			bits : 12_0
    NUE2_PAD_CROP_HEIGHT:    [0x0, 0x1fff],			bits : 28_16
*/
#define PADDING_REGISTER1_OFS 0x0068
REGDEF_BEGIN(PADDING_REGISTER1)
    REGDEF_BIT(NUE2_PAD_CROP_WIDTH ,        13)
    REGDEF_BIT(                    ,        3)
    REGDEF_BIT(NUE2_PAD_CROP_HEIGHT,        13)
REGDEF_END(PADDING_REGISTER1)


/*
    NUE2_PAD_OUT_X:    [0x0, 0x1fff],			bits : 12_0
    NUE2_PAD_OUT_Y:    [0x0, 0x1fff],			bits : 28_16
*/
#define PADDING_REGISTER2_OFS 0x006c
REGDEF_BEGIN(PADDING_REGISTER2)
    REGDEF_BIT(NUE2_PAD_OUT_X,        13)
    REGDEF_BIT(              ,        3)
    REGDEF_BIT(NUE2_PAD_OUT_Y,        13)
REGDEF_END(PADDING_REGISTER2)


/*
    NUE2_PAD_OUT_WIDTH :    [0x0, 0x1fff],			bits : 12_0
    NUE2_PAD_OUT_HEIGHT:    [0x0, 0x1fff],			bits : 28_16
*/
#define PADDING_REGISTER3_OFS 0x0070
REGDEF_BEGIN(PADDING_REGISTER3)
    REGDEF_BIT(NUE2_PAD_OUT_WIDTH ,        13)
    REGDEF_BIT(                   ,        3)
    REGDEF_BIT(NUE2_PAD_OUT_HEIGHT,        13)
REGDEF_END(PADDING_REGISTER3)


/*
    NUE2_PAD_VAL_0:    [0x0, 0xff],			bits : 7_0
    NUE2_PAD_VAL_1:    [0x0, 0xff],			bits : 15_8
    NUE2_PAD_VAL_2:    [0x0, 0xff],			bits : 23_16
*/
#define PADDING_REGISTER4_OFS 0x0074
REGDEF_BEGIN(PADDING_REGISTER4)
    REGDEF_BIT(NUE2_PAD_VAL_0,        8)
    REGDEF_BIT(NUE2_PAD_VAL_1,        8)
    REGDEF_BIT(NUE2_PAD_VAL_2,        8)
REGDEF_END(PADDING_REGISTER4)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER1_OFS 0x0078
REGDEF_BEGIN(RESERVED_REGISTER1)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER1)


/*
    MEAN_SHIFT_DIR:    [0x0, 0x1],			bits : 0
    MEAN_SHIFT    :    [0x0, 0x1f],			bits : 8_4
    MEAN_SCALE    :    [0x0, 0xffff],			bits : 27_12
*/
#define MEAN_SHIFT_REGISTER0_OFS 0x007c
REGDEF_BEGIN(MEAN_SHIFT_REGISTER0)
    REGDEF_BIT(MEAN_SHIFT_DIR,        1)
    REGDEF_BIT(              ,        3)
    REGDEF_BIT(MEAN_SHIFT    ,        5)
    REGDEF_BIT(              ,        3)
    REGDEF_BIT(MEAN_SCALE    ,        16)
REGDEF_END(MEAN_SHIFT_REGISTER0)


/*
    INDATA_BURST_MODE:    [0x0, 0x3],			bits : 1_0
    OUTRST_BURST_MODE:    [0x0, 0x3],			bits : 3_2
    DEBUGPORTSEL     :    [0x0, 0x3],			bits : 5_4
    CHECKSUM_EN      :    [0x0, 0x1],			bits : 11
    CYCLE_COUNT_EN   :    [0x0, 0x1],			bits : 12
*/
#define NUE2_DEBUG_DESIGN_REGISTER_OFS 0x0080
REGDEF_BEGIN(NUE2_DEBUG_DESIGN_REGISTER)
    REGDEF_BIT(INDATA_BURST_MODE,        2)
    REGDEF_BIT(OUTRST_BURST_MODE,        2)
    REGDEF_BIT(DEBUGPORTSEL     ,        2)
    REGDEF_BIT(                 ,        5)
    REGDEF_BIT(CHECKSUM_EN      ,        1)
    REGDEF_BIT(CYCLE_COUNT_EN   ,        1)
REGDEF_END(NUE2_DEBUG_DESIGN_REGISTER)


/*
    LL_TABLE_IDX0:    [0x0, 0xff],			bits : 7_0
    LL_TABLE_IDX1:    [0x0, 0xff],			bits : 15_8
    LL_TABLE_IDX2:    [0x0, 0xff],			bits : 23_16
    LL_TABLE_IDX3:    [0x0, 0xff],			bits : 31_24
*/
#define NUE2_LL_FRAME_REGISTER0_OFS 0x0084
REGDEF_BEGIN(NUE2_LL_FRAME_REGISTER0)
    REGDEF_BIT(LL_TABLE_IDX0,        8)
    REGDEF_BIT(LL_TABLE_IDX1,        8)
    REGDEF_BIT(LL_TABLE_IDX2,        8)
    REGDEF_BIT(LL_TABLE_IDX3,        8)
REGDEF_END(NUE2_LL_FRAME_REGISTER0)


/*
    LL_TABLE_IDX4:    [0x0, 0xff],			bits : 7_0
    LL_TABLE_IDX5:    [0x0, 0xff],			bits : 15_8
    LL_TABLE_IDX6:    [0x0, 0xff],			bits : 23_16
    LL_TABLE_IDX7:    [0x0, 0xff],			bits : 31_24
*/
#define NUE2_LL_FRAME_REGISTER1_OFS 0x0088
REGDEF_BEGIN(NUE2_LL_FRAME_REGISTER1)
    REGDEF_BIT(LL_TABLE_IDX4,        8)
    REGDEF_BIT(LL_TABLE_IDX5,        8)
    REGDEF_BIT(LL_TABLE_IDX6,        8)
    REGDEF_BIT(LL_TABLE_IDX7,        8)
REGDEF_END(NUE2_LL_FRAME_REGISTER1)


/*
    LL_TABLE_IDX8 :    [0x0, 0xff],			bits : 7_0
    LL_TABLE_IDX9 :    [0x0, 0xff],			bits : 15_8
    LL_TABLE_IDX10:    [0x0, 0xff],			bits : 23_16
    LL_TABLE_IDX11:    [0x0, 0xff],			bits : 31_24
*/
#define NUE2_LL_FRAME_REGISTER2_OFS 0x008c
REGDEF_BEGIN(NUE2_LL_FRAME_REGISTER2)
    REGDEF_BIT(LL_TABLE_IDX8 ,        8)
    REGDEF_BIT(LL_TABLE_IDX9 ,        8)
    REGDEF_BIT(LL_TABLE_IDX10,        8)
    REGDEF_BIT(LL_TABLE_IDX11,        8)
REGDEF_END(NUE2_LL_FRAME_REGISTER2)


/*
    LL_TABLE_IDX12:    [0x0, 0xff],			bits : 7_0
    LL_TABLE_IDX13:    [0x0, 0xff],			bits : 15_8
    LL_TABLE_IDX14:    [0x0, 0xff],			bits : 23_16
    LL_TABLE_IDX15:    [0x0, 0xff],			bits : 31_24
*/
#define NUE2_LL_FRAME_REGISTER3_OFS 0x0090
REGDEF_BEGIN(NUE2_LL_FRAME_REGISTER3)
    REGDEF_BIT(LL_TABLE_IDX12,        8)
    REGDEF_BIT(LL_TABLE_IDX13,        8)
    REGDEF_BIT(LL_TABLE_IDX14,        8)
    REGDEF_BIT(LL_TABLE_IDX15,        8)
REGDEF_END(NUE2_LL_FRAME_REGISTER3)


/*
    LL_TERMINATE:    [0x0, 0x1],			bits : 0
*/
#define NUE2_LL_TERMINATE_RESISTER0_OFS 0x0094
REGDEF_BEGIN(NUE2_LL_TERMINATE_RESISTER0)
    REGDEF_BIT(LL_TERMINATE,        1)
REGDEF_END(NUE2_LL_TERMINATE_RESISTER0)


/*
    DMA_DISABLE:    [0x0, 0x1],			bits : 0
    NUE2_IDLE  :    [0x0, 0x1],			bits : 15
*/
#define NUE2_DMA_DISABLE_REGISTER0_OFS 0x0098
REGDEF_BEGIN(NUE2_DMA_DISABLE_REGISTER0)
    REGDEF_BIT(DMA_DISABLE,        1)
    REGDEF_BIT(           ,        14)
    REGDEF_BIT(NUE2_IDLE  ,        1)
REGDEF_END(NUE2_DMA_DISABLE_REGISTER0)


/*
    LL_BASE_ADDR0:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_LL_BASE_ADDRESS_REGISTER0_OFS 0x009c
REGDEF_BEGIN(NUE2_LL_BASE_ADDRESS_REGISTER0)
    REGDEF_BIT(LL_BASE_ADDR0,        32)
REGDEF_END(NUE2_LL_BASE_ADDRESS_REGISTER0)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER2_OFS 0x00a0
REGDEF_BEGIN(RESERVED_REGISTER2)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER2)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER3_OFS 0x00a4
REGDEF_BEGIN(RESERVED_REGISTER3)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER3)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER4_OFS 0x00a8
REGDEF_BEGIN(RESERVED_REGISTER4)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER4)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER5_OFS 0x00ac
REGDEF_BEGIN(RESERVED_REGISTER5)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER5)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER6_OFS 0x00b0
REGDEF_BEGIN(RESERVED_REGISTER6)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER6)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER7_OFS 0x00b4
REGDEF_BEGIN(RESERVED_REGISTER7)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER7)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER8_OFS 0x00b8
REGDEF_BEGIN(RESERVED_REGISTER8)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER8)


/*
    NUE2_CODE_VERSION:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_VERSION_REGISTER0_OFS 0x00bc
REGDEF_BEGIN(NUE2_VERSION_REGISTER0)
    REGDEF_BIT(NUE2_CODE_VERSION,        32)
REGDEF_END(NUE2_VERSION_REGISTER0)


/*
    NUE2_ENG_CYCLE:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_CYCLE_COUNT_REGISTER0_OFS 0x00c0
REGDEF_BEGIN(NUE2_CYCLE_COUNT_REGISTER0)
    REGDEF_BIT(NUE2_ENG_CYCLE,        32)
REGDEF_END(NUE2_CYCLE_COUNT_REGISTER0)


/*
    NUE2_LL_CYCLE:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_CYCLE_COUNT_REGISTER1_OFS 0x00c4
REGDEF_BEGIN(NUE2_CYCLE_COUNT_REGISTER1)
    REGDEF_BIT(NUE2_LL_CYCLE,        32)
REGDEF_END(NUE2_CYCLE_COUNT_REGISTER1)


/*
    NUE2_WAIT_DMA_CYCLE:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_CYCLE_COUNT_REGISTER2_OFS 0x00c8
REGDEF_BEGIN(NUE2_CYCLE_COUNT_REGISTER2)
    REGDEF_BIT(NUE2_WAIT_DMA_CYCLE,        32)
REGDEF_END(NUE2_CYCLE_COUNT_REGISTER2)


/*
    NUE2_LLC_ERR_CMD_CNT:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_LLC_DEBUG_REGISTER0_OFS 0x00cc
REGDEF_BEGIN(NUE2_LLC_DEBUG_REGISTER0)
    REGDEF_BIT(NUE2_LLC_ERR_CMD_CNT,        32)
REGDEF_END(NUE2_LLC_DEBUG_REGISTER0)


/*
    NUE2_LLC_ERR_CMD_ADDR:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_LLC_DEBUG_REGISTER1_OFS 0x00d0
REGDEF_BEGIN(NUE2_LLC_DEBUG_REGISTER1)
    REGDEF_BIT(NUE2_LLC_ERR_CMD_ADDR,        32)
REGDEF_END(NUE2_LLC_DEBUG_REGISTER1)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER9_OFS 0x00d4
REGDEF_BEGIN(RESERVED_REGISTER9)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER9)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER10_OFS 0x00d8
REGDEF_BEGIN(RESERVED_REGISTER10)
    REGDEF_BIT(HIDDEN_NUE2_DRAM_BW,   32)
REGDEF_END(RESERVED_REGISTER10)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER11_OFS 0x00dc
REGDEF_BEGIN(RESERVED_REGISTER11)
    REGDEF_BIT(HIDDEN_NUE2_UB_BW,    32)
REGDEF_END(RESERVED_REGISTER11)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER12_OFS 0x00e0
REGDEF_BEGIN(RESERVED_REGISTER12)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER12)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER13_OFS 0x00e4
REGDEF_BEGIN(RESERVED_REGISTER13)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER13)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER14_OFS 0x00e8
REGDEF_BEGIN(RESERVED_REGISTER14)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER14)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER15_OFS 0x00ec
REGDEF_BEGIN(RESERVED_REGISTER15)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER15)


/*
    AXI_DIS    :    [0x0, 0x1],			bits : 0
    CHANNEL_EN :    [0x0, 0x3f],			bits : 6_1
    R_OSTD_NUM :    [0x0, 0xff],			bits : 15_8
    W_OSTD_NUM :    [0x0, 0xff],			bits : 23_16
    LOCK_DIS   :    [0x0, 0x3f],			bits : 29_24
    AXI_CH_IDLE:    [0x0, 0x1],			bits : 31
*/
#define NUE2_AXI_REGISTER0_OFS 0x00f0
REGDEF_BEGIN(NUE2_AXI_REGISTER0)
    REGDEF_BIT(AXI_DIS    ,        1)
    REGDEF_BIT(CHANNEL_EN ,        6)
    REGDEF_BIT(           ,        1)
    REGDEF_BIT(R_OSTD_NUM ,        8)
    REGDEF_BIT(W_OSTD_NUM ,        8)
    REGDEF_BIT(LOCK_DIS   ,        6)
    REGDEF_BIT(           ,        1)
    REGDEF_BIT(AXI_CH_IDLE,        1)
REGDEF_END(NUE2_AXI_REGISTER0)


/*
    AXI_CH_STA:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_AXI_REGISTER1_OFS 0x00f4
REGDEF_BEGIN(NUE2_AXI_REGISTER1)
    REGDEF_BIT(AXI_CH_STA,        32)
REGDEF_END(NUE2_AXI_REGISTER1)


/*
    NUE2_OUTPUT_CHECKSUM0:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_OUTPUT_CHECKSUM_REGISTER0_OFS 0x00f8
REGDEF_BEGIN(NUE2_OUTPUT_CHECKSUM_REGISTER0)
    REGDEF_BIT(NUE2_OUTPUT_CHECKSUM0,        32)
REGDEF_END(NUE2_OUTPUT_CHECKSUM_REGISTER0)


/*
    NUE2_OUTPUT_CHECKSUM1:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_OUTPUT_CHECKSUM_REGISTER1_OFS 0x00fc
REGDEF_BEGIN(NUE2_OUTPUT_CHECKSUM_REGISTER1)
    REGDEF_BIT(NUE2_OUTPUT_CHECKSUM1,        32)
REGDEF_END(NUE2_OUTPUT_CHECKSUM_REGISTER1)


/*
    NUE2_OUTPUT_CHECKSUM2:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_OUTPUT_CHECKSUM_REGISTER2_OFS 0x0100
REGDEF_BEGIN(NUE2_OUTPUT_CHECKSUM_REGISTER2)
    REGDEF_BIT(NUE2_OUTPUT_CHECKSUM2,        32)
REGDEF_END(NUE2_OUTPUT_CHECKSUM_REGISTER2)


/*
    NUE2_COMPARE_CHECKSUM0:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_COMPARE_CHECKSUM_REGISTER0_OFS 0x0104
REGDEF_BEGIN(NUE2_COMPARE_CHECKSUM_REGISTER0)
    REGDEF_BIT(NUE2_COMPARE_CHECKSUM0,        32)
REGDEF_END(NUE2_COMPARE_CHECKSUM_REGISTER0)


/*
    NUE2_COMPARE_CHECKSUM1:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_COMPARE_CHECKSUM_REGISTER1_OFS 0x0108
REGDEF_BEGIN(NUE2_COMPARE_CHECKSUM_REGISTER1)
    REGDEF_BIT(NUE2_COMPARE_CHECKSUM1,        32)
REGDEF_END(NUE2_COMPARE_CHECKSUM_REGISTER1)


/*
    NUE2_COMPARE_CHECKSUM2:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_COMPARE_CHECKSUM_REGISTER2_OFS 0x010c
REGDEF_BEGIN(NUE2_COMPARE_CHECKSUM_REGISTER2)
    REGDEF_BIT(NUE2_COMPARE_CHECKSUM2,        32)
REGDEF_END(NUE2_COMPARE_CHECKSUM_REGISTER2)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER16_OFS 0x0110
REGDEF_BEGIN(RESERVED_REGISTER16)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER16)


/*
    NUE2_CHKSUM_MISMATCH_IDX_CNT:    [0x0, 0xffff],			bits : 15_0
*/
#define NUE2_CHKSUM_MISMATCH_COUNTER_REGISTER0_OFS 0x0114
REGDEF_BEGIN(NUE2_CHKSUM_MISMATCH_COUNTER_REGISTER0)
    REGDEF_BIT(NUE2_CHKSUM_MISMATCH_IDX_CNT,        16)
REGDEF_END(NUE2_CHKSUM_MISMATCH_COUNTER_REGISTER0)


/*
    NUE2_YUV2RGB_COEF0:    [0x0, 0x7fff],			bits : 14_0
*/
#define YUV2RGB_COEF_REGISTER0_OFS 0x0118
REGDEF_BEGIN(YUV2RGB_COEF_REGISTER0)
    REGDEF_BIT(NUE2_YUV2RGB_COEF0,        15)
REGDEF_END(YUV2RGB_COEF_REGISTER0)


/*
    NUE2_YUV2RGB_COEF1:    [0x0, 0x7fff],			bits : 14_0
*/
#define YUV2RGB_COEF_REGISTER1_OFS 0x011c
REGDEF_BEGIN(YUV2RGB_COEF_REGISTER1)
    REGDEF_BIT(NUE2_YUV2RGB_COEF1,        15)
REGDEF_END(YUV2RGB_COEF_REGISTER1)


/*
    NUE2_YUV2RGB_COEF2:    [0x0, 0x7fff],			bits : 14_0
*/
#define YUV2RGB_COEF_REGISTER2_OFS 0x0120
REGDEF_BEGIN(YUV2RGB_COEF_REGISTER2)
    REGDEF_BIT(NUE2_YUV2RGB_COEF2,        15)
REGDEF_END(YUV2RGB_COEF_REGISTER2)


/*
    NUE2_YUV2RGB_COEF3:    [0x0, 0x7fff],			bits : 14_0
*/
#define YUV2RGB_COEF_REGISTER3_OFS 0x0124
REGDEF_BEGIN(YUV2RGB_COEF_REGISTER3)
    REGDEF_BIT(NUE2_YUV2RGB_COEF3,        15)
REGDEF_END(YUV2RGB_COEF_REGISTER3)


/*
    NUE2_YUV2RGB_COEF4:    [0x0, 0x7fff],			bits : 14_0
*/
#define YUV2RGB_COEF_REGISTER4_OFS 0x0128
REGDEF_BEGIN(YUV2RGB_COEF_REGISTER4)
    REGDEF_BIT(NUE2_YUV2RGB_COEF4,        15)
REGDEF_END(YUV2RGB_COEF_REGISTER4)


/*
    NUE2_YUV2RGB_COEF5:    [0x0, 0x7fff],			bits : 14_0
*/
#define YUV2RGB_COEF_REGISTER5_OFS 0x012c
REGDEF_BEGIN(YUV2RGB_COEF_REGISTER5)
    REGDEF_BIT(NUE2_YUV2RGB_COEF5,        15)
REGDEF_END(YUV2RGB_COEF_REGISTER5)


/*
    NUE2_YUV2RGB_COEF6:    [0x0, 0x7fff],			bits : 14_0
*/
#define YUV2RGB_COEF_REGISTER6_OFS 0x0130
REGDEF_BEGIN(YUV2RGB_COEF_REGISTER6)
    REGDEF_BIT(NUE2_YUV2RGB_COEF6,        15)
REGDEF_END(YUV2RGB_COEF_REGISTER6)


/*
    NUE2_YUV2RGB_COEF7:    [0x0, 0x7fff],			bits : 14_0
*/
#define YUV2RGB_COEF_REGISTER7_OFS 0x0134
REGDEF_BEGIN(YUV2RGB_COEF_REGISTER7)
    REGDEF_BIT(NUE2_YUV2RGB_COEF7,        15)
REGDEF_END(YUV2RGB_COEF_REGISTER7)


/*
    NUE2_YUV2RGB_COEF8:    [0x0, 0x7fff],			bits : 14_0
*/
#define YUV2RGB_COEF_REGISTER8_OFS 0x0138
REGDEF_BEGIN(YUV2RGB_COEF_REGISTER8)
    REGDEF_BIT(NUE2_YUV2RGB_COEF8,        15)
REGDEF_END(YUV2RGB_COEF_REGISTER8)


/*
    NUE2_YUV2RGB_BIAS0:    [0x0, 0x1ff],			bits : 8_0
*/
#define YUV2RGB_BIAS_REGISTER0_OFS 0x013c
REGDEF_BEGIN(YUV2RGB_BIAS_REGISTER0)
    REGDEF_BIT(NUE2_YUV2RGB_BIAS0,        9)
REGDEF_END(YUV2RGB_BIAS_REGISTER0)


/*
    NUE2_YUV2RGB_BIAS1:    [0x0, 0x1ff],			bits : 8_0
*/
#define YUV2RGB_BIAS_REGISTER1_OFS 0x0140
REGDEF_BEGIN(YUV2RGB_BIAS_REGISTER1)
    REGDEF_BIT(NUE2_YUV2RGB_BIAS1,        9)
REGDEF_END(YUV2RGB_BIAS_REGISTER1)


/*
    NUE2_YUV2RGB_BIAS2:    [0x0, 0x1ff],			bits : 8_0
*/
#define YUV2RGB_BIAS_REGISTER2_OFS 0x0144
REGDEF_BEGIN(YUV2RGB_BIAS_REGISTER2)
    REGDEF_BIT(NUE2_YUV2RGB_BIAS2,        9)
REGDEF_END(YUV2RGB_BIAS_REGISTER2)


/*
    DRAM_MSB_SAI0 :    [0x0, 0xf],			bits : 3_0
    DRAM_MODE_SAI0:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_NUE2_MSB_REGISTER0_OFS 0x0148
REGDEF_BEGIN(DMA_TO_NUE2_MSB_REGISTER0)
    REGDEF_BIT(DRAM_MSB_SAI0 ,        4)
    REGDEF_BIT(              ,        24)
    REGDEF_BIT(DRAM_MODE_SAI0,        4)
REGDEF_END(DMA_TO_NUE2_MSB_REGISTER0)


/*
    DRAM_MSB_SAI1 :    [0x0, 0xf],			bits : 3_0
    DRAM_MODE_SAI1:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_NUE2_MSB_REGISTER1_OFS 0x014c
REGDEF_BEGIN(DMA_TO_NUE2_MSB_REGISTER1)
    REGDEF_BIT(DRAM_MSB_SAI1 ,        4)
    REGDEF_BIT(              ,        24)
    REGDEF_BIT(DRAM_MODE_SAI1,        4)
REGDEF_END(DMA_TO_NUE2_MSB_REGISTER1)


/*
    DRAM_MSB_SAI2 :    [0x0, 0xf],			bits : 3_0
    DRAM_MODE_SAI2:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_NUE2_MSB_REGISTER2_OFS 0x0150
REGDEF_BEGIN(DMA_TO_NUE2_MSB_REGISTER2)
    REGDEF_BIT(DRAM_MSB_SAI2 ,        4)
    REGDEF_BIT(              ,        24)
    REGDEF_BIT(DRAM_MODE_SAI2,        4)
REGDEF_END(DMA_TO_NUE2_MSB_REGISTER2)


/*
    DRAM_MSB_SAILL:    [0x0, 0xf],			bits : 3_0
*/
#define DMA_TO_NUE2_MSB_REGISTER3_OFS 0x0154
REGDEF_BEGIN(DMA_TO_NUE2_MSB_REGISTER3)
    REGDEF_BIT(DRAM_MSB_SAILL,        4)
REGDEF_END(DMA_TO_NUE2_MSB_REGISTER3)


/*
    DRAM_MSB_SAO0 :    [0x0, 0xf],			bits : 3_0
    DRAM_MODE_SAO0:    [0x0, 0xf],			bits : 31_28
*/
#define NUE2_TO_DMA_RESULT_MSB_REGISTER0_OFS 0x0158
REGDEF_BEGIN(NUE2_TO_DMA_RESULT_MSB_REGISTER0)
    REGDEF_BIT(DRAM_MSB_SAO0 ,        4)
    REGDEF_BIT(              ,        24)
    REGDEF_BIT(DRAM_MODE_SAO0,        4)
REGDEF_END(NUE2_TO_DMA_RESULT_MSB_REGISTER0)


/*
    DRAM_MSB_SAO1 :    [0x0, 0xf],			bits : 3_0
    DRAM_MODE_SAO1:    [0x0, 0xf],			bits : 31_28
*/
#define NUE2_TO_DMA_RESULT_MSB_REGISTER1_OFS 0x015c
REGDEF_BEGIN(NUE2_TO_DMA_RESULT_MSB_REGISTER1)
    REGDEF_BIT(DRAM_MSB_SAO1 ,        4)
    REGDEF_BIT(              ,        24)
    REGDEF_BIT(DRAM_MODE_SAO1,        4)
REGDEF_END(NUE2_TO_DMA_RESULT_MSB_REGISTER1)


/*
    DRAM_MSB_SAO2 :    [0x0, 0xf],			bits : 3_0
    DRAM_MODE_SAO2:    [0x0, 0xf],			bits : 31_28
*/
#define NUE2_TO_DMA_RESULT_MSB_REGISTER2_OFS 0x0160
REGDEF_BEGIN(NUE2_TO_DMA_RESULT_MSB_REGISTER2)
    REGDEF_BIT(DRAM_MSB_SAO2 ,        4)
    REGDEF_BIT(              ,        24)
    REGDEF_BIT(DRAM_MODE_SAO2,        4)
REGDEF_END(NUE2_TO_DMA_RESULT_MSB_REGISTER2)


/*
    LL_BASE_MSB_ADDR0:    [0x0, 0xf],			bits : 3_0
*/
#define NUE2_LL_BASE_ADDRESS_MSB_REGISTER0_OFS 0x0164
REGDEF_BEGIN(NUE2_LL_BASE_ADDRESS_MSB_REGISTER0)
    REGDEF_BIT(LL_BASE_MSB_ADDR0,        4)
REGDEF_END(NUE2_LL_BASE_ADDRESS_MSB_REGISTER0)


/*
    MEAN_CH_SHIFT_DIR0:    [0x0, 0x1],			bits : 0
    MEAN_CH_SHIFT0    :    [0x0, 0x1f],			bits : 8_4
    MEAN_CH_SCALE0    :    [0x0, 0xffff],			bits : 27_12
*/
#define MEAN_SHIFT_REGISTER1_OFS 0x0168
REGDEF_BEGIN(MEAN_SHIFT_REGISTER1)
    REGDEF_BIT(MEAN_CH_SHIFT_DIR0,        1)
    REGDEF_BIT(                  ,        3)
    REGDEF_BIT(MEAN_CH_SHIFT0    ,        5)
    REGDEF_BIT(                  ,        3)
    REGDEF_BIT(MEAN_CH_SCALE0    ,        16)
REGDEF_END(MEAN_SHIFT_REGISTER1)


/*
    MEAN_CH_SHIFT_DIR1:    [0x0, 0x1],			bits : 0
    MEAN_CH_SHIFT1    :    [0x0, 0x1f],			bits : 8_4
    MEAN_CH_SCALE1    :    [0x0, 0xffff],			bits : 27_12
*/
#define MEAN_SHIFT_REGISTER2_OFS 0x016c
REGDEF_BEGIN(MEAN_SHIFT_REGISTER2)
    REGDEF_BIT(MEAN_CH_SHIFT_DIR1,        1)
    REGDEF_BIT(                  ,        3)
    REGDEF_BIT(MEAN_CH_SHIFT1    ,        5)
    REGDEF_BIT(                  ,        3)
    REGDEF_BIT(MEAN_CH_SCALE1    ,        16)
REGDEF_END(MEAN_SHIFT_REGISTER2)


/*
    MEAN_CH_SHIFT_DIR2:    [0x0, 0x1],			bits : 0
    MEAN_CH_SHIFT2    :    [0x0, 0x1f],			bits : 8_4
    MEAN_CH_SCALE2    :    [0x0, 0xffff],			bits : 27_12
*/
#define MEAN_SHIFT_REGISTER3_OFS 0x0170
REGDEF_BEGIN(MEAN_SHIFT_REGISTER3)
    REGDEF_BIT(MEAN_CH_SHIFT_DIR2,        1)
    REGDEF_BIT(                  ,        3)
    REGDEF_BIT(MEAN_CH_SHIFT2    ,        5)
    REGDEF_BIT(                  ,        3)
    REGDEF_BIT(MEAN_CH_SCALE2    ,        16)
REGDEF_END(MEAN_SHIFT_REGISTER3)


/*
    LL_BASE_ADDR1:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_LL_BASE_ADDRESS_REGISTER1_OFS 0x0174
REGDEF_BEGIN(NUE2_LL_BASE_ADDRESS_REGISTER1)
    REGDEF_BIT(LL_BASE_ADDR1,        32)
REGDEF_END(NUE2_LL_BASE_ADDRESS_REGISTER1)


/*
    LL_BASE_MSB_ADDR1:    [0x0, 0xf],			bits : 3_0
*/
#define NUE2_LL_BASE_ADDRESS_MSB_REGISTER1_OFS 0x0178
REGDEF_BEGIN(NUE2_LL_BASE_ADDRESS_MSB_REGISTER1)
    REGDEF_BIT(LL_BASE_MSB_ADDR1,        4)
REGDEF_END(NUE2_LL_BASE_ADDRESS_MSB_REGISTER1)


/*
    LL_BASE_ADDR2:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_LL_BASE_ADDRESS_REGISTER2_OFS 0x017c
REGDEF_BEGIN(NUE2_LL_BASE_ADDRESS_REGISTER2)
    REGDEF_BIT(LL_BASE_ADDR2,        32)
REGDEF_END(NUE2_LL_BASE_ADDRESS_REGISTER2)


/*
    LL_BASE_MSB_ADDR2:    [0x0, 0xf],			bits : 3_0
*/
#define NUE2_LL_BASE_ADDRESS_MSB_REGISTER2_OFS 0x0180
REGDEF_BEGIN(NUE2_LL_BASE_ADDRESS_MSB_REGISTER2)
    REGDEF_BIT(LL_BASE_MSB_ADDR2,        4)
REGDEF_END(NUE2_LL_BASE_ADDRESS_MSB_REGISTER2)


/*
    LL_BASE_ADDR3:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_LL_BASE_ADDRESS_REGISTER3_OFS 0x0184
REGDEF_BEGIN(NUE2_LL_BASE_ADDRESS_REGISTER3)
    REGDEF_BIT(LL_BASE_ADDR3,        32)
REGDEF_END(NUE2_LL_BASE_ADDRESS_REGISTER3)


/*
    LL_BASE_MSB_ADDR3:    [0x0, 0xf],			bits : 3_0
*/
#define NUE2_LL_BASE_ADDRESS_MSB_REGISTER3_OFS 0x0188
REGDEF_BEGIN(NUE2_LL_BASE_ADDRESS_MSB_REGISTER3)
    REGDEF_BIT(LL_BASE_MSB_ADDR3,        4)
REGDEF_END(NUE2_LL_BASE_ADDRESS_MSB_REGISTER3)


/*
    LL_BASE_ADDR4:    [0x0, 0xffffffff],			bits : 31_0
*/
#define NUE2_LL_BASE_ADDRESS_REGISTER4_OFS 0x018c
REGDEF_BEGIN(NUE2_LL_BASE_ADDRESS_REGISTER4)
    REGDEF_BIT(LL_BASE_ADDR4,        32)
REGDEF_END(NUE2_LL_BASE_ADDRESS_REGISTER4)


/*
    LL_BASE_MSB_ADDR4:    [0x0, 0xf],			bits : 3_0
*/
#define NUE2_LL_BASE_ADDRESS_MSB_REGISTER4_OFS 0x0190
REGDEF_BEGIN(NUE2_LL_BASE_ADDRESS_MSB_REGISTER4)
    REGDEF_BIT(LL_BASE_MSB_ADDR4,        4)
REGDEF_END(NUE2_LL_BASE_ADDRESS_MSB_REGISTER4)

typedef struct
{

	union
	{
		struct
		{
			unsigned NUE2_RST          : 1;		// bits : 0
			unsigned NUE2_START        : 1;		// bits : 1
			unsigned                   : 26;
			unsigned LL_FIRE           : 1;		// bits : 28
		} Bit;
		UINT32 Word;
	} NUE2_Register_0; // 0x0000

	union
	{
		struct
		{
			unsigned NUE2_YUV2RGB_EN                   : 1;		// bits : 0
			unsigned NUE2_SUB_EN                       : 1;		// bits : 1
			unsigned NUE2_PAD_EN                       : 1;		// bits : 2
			unsigned NUE2_HSV_EN                       : 1;		// bits : 3
			unsigned NUE2_ROTATE_EN                    : 1;		// bits : 4
			unsigned                                   : 3;
			unsigned NUE2_HSV_OUT_FMT                  : 1;		// bits : 8
			unsigned                                   : 3;
			unsigned NUE2_IN_FMT                       : 2;		// bits : 13_12
			unsigned NUE2_OUT_SIGNEDNESS               : 1;		// bits : 14
			unsigned NUE2_SUB_MODE                     : 1;		// bits : 15
			unsigned NUE2_HSV_OUT_MODE                 : 1;		// bits : 16
			unsigned NUE2_ROTATE_MODE                  : 2;		// bits : 18_17
			unsigned NUE2_FLIP_MODE                    : 2;		// bits : 20_19
			unsigned NUE2_YUV_MODE                     : 3;		// bits : 23_21
			unsigned NUE2_SCALE_H_MODE                 : 1;		// bits : 24
			unsigned NUE2_SCALE_V_MODE                 : 1;		// bits : 25
			unsigned NUE2_MEAN_SCALE_SHIFT_MODE        : 1;		// bits : 26
		} Bit;
		UINT32 Word;
	} NUE2_Register_1; // 0x0004

	union
	{
		struct
		{
			unsigned DRAM_SAI0        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_2; // 0x0008

	union
	{
		struct
		{
			unsigned DRAM_SAI1        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_3; // 0x000c

	union
	{
		struct
		{
			unsigned DRAM_SAI2        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_4; // 0x0010

	union
	{
		struct
		{
			unsigned DRAM_SAILL        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_5; // 0x0014

	union
	{
		struct
		{
			unsigned DRAM_SAO0        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_6; // 0x0018

	union
	{
		struct
		{
			unsigned DRAM_SAO1        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_7; // 0x001c

	union
	{
		struct
		{
			unsigned DRAM_SAO2        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_8; // 0x0020

	union
	{
		struct
		{
			unsigned DRAM_OFSI0        : 17;		// bits : 16_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_9; // 0x0024

	union
	{
		struct
		{
			unsigned DRAM_OFSI1        : 17;		// bits : 16_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_10; // 0x0028

	union
	{
		struct
		{
			unsigned DRAM_OFSI2        : 17;		// bits : 16_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_11; // 0x002c

	union
	{
		struct
		{
			unsigned DRAM_OFSO0        : 17;		// bits : 16_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_12; // 0x0030

	union
	{
		struct
		{
			unsigned DRAM_OFSO1        : 17;		// bits : 16_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_13; // 0x0034

	union
	{
		struct
		{
			unsigned DRAM_OFSO2        : 17;		// bits : 16_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_14; // 0x0038

	union
	{
		struct
		{
			unsigned INTE_FRM_END                   : 1;		// bits : 0
			unsigned INTE_DMAIN0END                 : 1;		// bits : 1
			unsigned INTE_DMAIN1END                 : 1;		// bits : 2
			unsigned INTE_DMAIN2END                 : 1;		// bits : 3
			unsigned                                : 4;
			unsigned INTE_LLEND                     : 1;		// bits : 8
			unsigned INTE_LLERROR                   : 1;		// bits : 9
			unsigned INTE_LLJOBEND                  : 1;		// bits : 10
			unsigned                                : 5;
			unsigned INTE_SW_RESET                  : 1;		// bits : 16
			unsigned INTE_CHECKSUM_MISMATCH0        : 1;		// bits : 17
			unsigned INTE_CHECKSUM_MISMATCH1        : 1;		// bits : 18
			unsigned INTE_CHECKSUM_MISMATCH2        : 1;		// bits : 19
		} Bit;
		UINT32 Word;
	} NUE2_Register_15; // 0x003c

	union
	{
		struct
		{
			unsigned INT_FRM_END                   : 1;		// bits : 0
			unsigned INT_DMAIN0END                 : 1;		// bits : 1
			unsigned INT_DMAIN1END                 : 1;		// bits : 2
			unsigned INT_DMAIN2END                 : 1;		// bits : 3
			unsigned                               : 4;
			unsigned INT_LLEND                     : 1;		// bits : 8
			unsigned INT_LLERROR                   : 1;		// bits : 9
			unsigned INT_LLJOBEND                  : 1;		// bits : 10
			unsigned                               : 5;
			unsigned INT_SW_RESET                  : 1;		// bits : 16
			unsigned INT_CHECKSUM_MISMATCH0        : 1;		// bits : 17
			unsigned INT_CHECKSUM_MISMATCH1        : 1;		// bits : 18
			unsigned INT_CHECKSUM_MISMATCH2        : 1;		// bits : 19
		} Bit;
		UINT32 Word;
	} NUE2_Register_16; // 0x0040

	union
	{
		struct
		{
			unsigned NUE2_IN_WIDTH         : 13;		// bits : 12_0
			unsigned                       : 3;
			unsigned NUE2_IN_HEIGHT        : 13;		// bits : 28_16
		} Bit;
		UINT32 Word;
	} NUE2_Register_17; // 0x0044

	union
	{
		struct
		{
			unsigned NUE2_H_DNRATE          : 7;		// bits : 6_0
			unsigned                        : 1;
			unsigned NUE2_V_DNRATE          : 7;		// bits : 14_8
			unsigned                        : 1;
			unsigned NUE2_H_FILTMODE        : 1;		// bits : 16
			unsigned NUE2_H_FILTCOEF        : 6;		// bits : 22_17
			unsigned                        : 1;
			unsigned NUE2_V_FILTMODE        : 1;		// bits : 24
			unsigned NUE2_V_FILTCOEF        : 6;		// bits : 30_25
		} Bit;
		UINT32 Word;
	} NUE2_Register_18; // 0x0048

	union
	{
		struct
		{
			unsigned NUE2_H_SFACT        : 16;		// bits : 15_0
			unsigned NUE2_V_SFACT        : 16;		// bits : 31_16
		} Bit;
		UINT32 Word;
	} NUE2_Register_19; // 0x004c

	union
	{
		struct
		{
			unsigned NUE2_INI_H_DNRATE        : 7;		// bits : 6_0
			unsigned                          : 8;
			unsigned NUE2_INI_H_SFACT         : 17;		// bits : 31_15
		} Bit;
		UINT32 Word;
	} NUE2_Register_20; // 0x0050

	union
	{
		struct
		{
			unsigned NUE2_FINAL_H_DNRATE        : 7;		// bits : 6_0
			unsigned                            : 8;
			unsigned NUE2_FINAL_H_SFACT         : 17;		// bits : 31_15
		} Bit;
		UINT32 Word;
	} NUE2_Register_21; // 0x0054

	union
	{
		struct
		{
			unsigned NUE2_H_SCL_SIZE        : 13;		// bits : 12_0
			unsigned                        : 3;
			unsigned NUE2_V_SCL_SIZE        : 13;		// bits : 28_16
		} Bit;
		UINT32 Word;
	} NUE2_Register_22; // 0x0058

	union
	{
		struct
		{
			unsigned NUE2_SUB_IN_WIDTH         : 13;		// bits : 12_0
			unsigned                           : 3;
			unsigned NUE2_SUB_IN_HEIGHT        : 13;		// bits : 28_16
		} Bit;
		UINT32 Word;
	} NUE2_Register_23; // 0x005c

	union
	{
		struct
		{
			unsigned NUE2_SUB_COEF_0        : 8;		// bits : 7_0
			unsigned NUE2_SUB_COEF_1        : 8;		// bits : 15_8
			unsigned NUE2_SUB_COEF_2        : 8;		// bits : 23_16
			unsigned NUE2_SUB_DUP           : 2;		// bits : 25_24
		} Bit;
		UINT32 Word;
	} NUE2_Register_24; // 0x0060

	union
	{
		struct
		{
			unsigned NUE2_PAD_CROP_X        : 13;		// bits : 12_0
			unsigned                        : 3;
			unsigned NUE2_PAD_CROP_Y        : 13;		// bits : 28_16
		} Bit;
		UINT32 Word;
	} NUE2_Register_25; // 0x0064

	union
	{
		struct
		{
			unsigned NUE2_PAD_CROP_WIDTH         : 13;		// bits : 12_0
			unsigned                             : 3;
			unsigned NUE2_PAD_CROP_HEIGHT        : 13;		// bits : 28_16
		} Bit;
		UINT32 Word;
	} NUE2_Register_26; // 0x0068

	union
	{
		struct
		{
			unsigned NUE2_PAD_OUT_X        : 13;		// bits : 12_0
			unsigned                       : 3;
			unsigned NUE2_PAD_OUT_Y        : 13;		// bits : 28_16
		} Bit;
		UINT32 Word;
	} NUE2_Register_27; // 0x006c

	union
	{
		struct
		{
			unsigned NUE2_PAD_OUT_WIDTH         : 13;		// bits : 12_0
			unsigned                            : 3;
			unsigned NUE2_PAD_OUT_HEIGHT        : 13;		// bits : 28_16
		} Bit;
		UINT32 Word;
	} NUE2_Register_28; // 0x0070

	union
	{
		struct
		{
			unsigned NUE2_PAD_VAL_0        : 8;		// bits : 7_0
			unsigned NUE2_PAD_VAL_1        : 8;		// bits : 15_8
			unsigned NUE2_PAD_VAL_2        : 8;		// bits : 23_16
		} Bit;
		UINT32 Word;
	} NUE2_Register_29; // 0x0074

	union
	{
		struct
		{
			unsigned NUE2_HUE_SFT        : 7;		// bits : 6_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_30; // 0x0078

	union
	{
		struct
		{
			unsigned MEAN_SHIFT_DIR        : 1;		// bits : 0
			unsigned                       : 3;
			unsigned MEAN_SHIFT            : 5;		// bits : 8_4
			unsigned                       : 3;
			unsigned MEAN_SCALE            : 16;		// bits : 27_12
		} Bit;
		UINT32 Word;
	} NUE2_Register_31; // 0x007c

	union
	{
		struct
		{
			unsigned INDATA_BURST_MODE        : 2;		// bits : 1_0
			unsigned OUTRST_BURST_MODE        : 2;		// bits : 3_2
			unsigned DEBUGPORTSEL             : 2;		// bits : 5_4
			unsigned                          : 5;
			unsigned CHECKSUM_EN              : 1;		// bits : 11
			unsigned CYCLE_COUNT_EN           : 1;		// bits : 12
		} Bit;
		UINT32 Word;
	} NUE2_Register_32; // 0x0080

	union
	{
		struct
		{
			unsigned LL_TABLE_IDX0        : 8;		// bits : 7_0
			unsigned LL_TABLE_IDX1        : 8;		// bits : 15_8
			unsigned LL_TABLE_IDX2        : 8;		// bits : 23_16
			unsigned LL_TABLE_IDX3        : 8;		// bits : 31_24
		} Bit;
		UINT32 Word;
	} NUE2_Register_33; // 0x0084

	union
	{
		struct
		{
			unsigned LL_TABLE_IDX4        : 8;		// bits : 7_0
			unsigned LL_TABLE_IDX5        : 8;		// bits : 15_8
			unsigned LL_TABLE_IDX6        : 8;		// bits : 23_16
			unsigned LL_TABLE_IDX7        : 8;		// bits : 31_24
		} Bit;
		UINT32 Word;
	} NUE2_Register_34; // 0x0088

	union
	{
		struct
		{
			unsigned LL_TABLE_IDX8         : 8;		// bits : 7_0
			unsigned LL_TABLE_IDX9         : 8;		// bits : 15_8
			unsigned LL_TABLE_IDX10        : 8;		// bits : 23_16
			unsigned LL_TABLE_IDX11        : 8;		// bits : 31_24
		} Bit;
		UINT32 Word;
	} NUE2_Register_35; // 0x008c

	union
	{
		struct
		{
			unsigned LL_TABLE_IDX12        : 8;		// bits : 7_0
			unsigned LL_TABLE_IDX13        : 8;		// bits : 15_8
			unsigned LL_TABLE_IDX14        : 8;		// bits : 23_16
			unsigned LL_TABLE_IDX15        : 8;		// bits : 31_24
		} Bit;
		UINT32 Word;
	} NUE2_Register_36; // 0x0090

	union
	{
		struct
		{
			unsigned LL_TERMINATE        : 1;		// bits : 0
		} Bit;
		UINT32 Word;
	} NUE2_Register_37; // 0x0094

	union
	{
		struct
		{
			unsigned DMA_DISABLE        : 1;		// bits : 0
			unsigned                    : 14;
			unsigned NUE2_IDLE          : 1;		// bits : 15
		} Bit;
		UINT32 Word;
	} NUE2_Register_38; // 0x0098

	union
	{
		struct
		{
			unsigned LL_BASE_ADDR0        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_39; // 0x009c

	union
	{
		struct
		{
			unsigned reserved        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_40; // 0x00a0

	union
	{
		struct
		{
			unsigned reserved        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_41; // 0x00a4

	union
	{
		struct
		{
			unsigned reserved        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_42; // 0x00a8

	union
	{
		struct
		{
			unsigned reserved        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_43; // 0x00ac

	union
	{
		struct
		{
			unsigned reserved        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_44; // 0x00b0

	union
	{
		struct
		{
			unsigned reserved        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_45; // 0x00b4

	union
	{
		struct
		{
			unsigned reserved        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_46; // 0x00b8

	union
	{
		struct
		{
			unsigned NUE2_CODE_VERSION        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_47; // 0x00bc

	union
	{
		struct
		{
			unsigned NUE2_ENG_CYCLE        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_48; // 0x00c0

	union
	{
		struct
		{
			unsigned NUE2_LL_CYCLE        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_49; // 0x00c4

	union
	{
		struct
		{
			unsigned NUE2_WAIT_DMA_CYCLE        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_50; // 0x00c8

	union
	{
		struct
		{
			unsigned NUE2_LLC_ERR_CMD_CNT        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_51; // 0x00cc

	union
	{
		struct
		{
			unsigned NUE2_LLC_ERR_CMD_ADDR        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_52; // 0x00d0

	union
	{
		struct
		{
			unsigned reserved        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_53; // 0x00d4

	union
	{
		struct
		{
			unsigned reserved        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_54; // 0x00d8

	union
	{
		struct
		{
			unsigned reserved        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_55; // 0x00dc

	union
	{
		struct
		{
			unsigned reserved        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_56; // 0x00e0

	union
	{
		struct
		{
			unsigned reserved        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_57; // 0x00e4

	union
	{
		struct
		{
			unsigned reserved        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_58; // 0x00e8

	union
	{
		struct
		{
			unsigned reserved        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_59; // 0x00ec

	union
	{
		struct
		{
			unsigned AXI_DIS            : 1;		// bits : 0
			unsigned CHANNEL_EN         : 6;		// bits : 6_1
			unsigned                    : 1;
			unsigned R_OSTD_NUM         : 8;		// bits : 15_8
			unsigned W_OSTD_NUM         : 8;		// bits : 23_16
			unsigned LOCK_DIS           : 6;		// bits : 29_24
			unsigned                    : 1;
			unsigned AXI_CH_IDLE        : 1;		// bits : 31
		} Bit;
		UINT32 Word;
	} NUE2_Register_60; // 0x00f0

	union
	{
		struct
		{
			unsigned AXI_CH_STA        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_61; // 0x00f4

	union
	{
		struct
		{
			unsigned NUE2_OUTPUT_CHECKSUM0        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_62; // 0x00f8

	union
	{
		struct
		{
			unsigned NUE2_OUTPUT_CHECKSUM1        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_63; // 0x00fc

	union
	{
		struct
		{
			unsigned NUE2_OUTPUT_CHECKSUM2        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_64; // 0x0100

	union
	{
		struct
		{
			unsigned NUE2_COMPARE_CHECKSUM0        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_65; // 0x0104

	union
	{
		struct
		{
			unsigned NUE2_COMPARE_CHECKSUM1        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_66; // 0x0108

	union
	{
		struct
		{
			unsigned NUE2_COMPARE_CHECKSUM2        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_67; // 0x010c

	union
	{
		struct
		{
			unsigned reserved        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_68; // 0x0110

	union
	{
		struct
		{
			unsigned NUE2_CHKSUM_MISMATCH_IDX_CNT        : 16;		// bits : 15_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_69; // 0x0114

	union
	{
		struct
		{
			unsigned NUE2_YUV2RGB_COEF0        : 15;		// bits : 14_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_70; // 0x0118

	union
	{
		struct
		{
			unsigned NUE2_YUV2RGB_COEF1        : 15;		// bits : 14_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_71; // 0x011c

	union
	{
		struct
		{
			unsigned NUE2_YUV2RGB_COEF2        : 15;		// bits : 14_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_72; // 0x0120

	union
	{
		struct
		{
			unsigned NUE2_YUV2RGB_COEF3        : 15;		// bits : 14_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_73; // 0x0124

	union
	{
		struct
		{
			unsigned NUE2_YUV2RGB_COEF4        : 15;		// bits : 14_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_74; // 0x0128

	union
	{
		struct
		{
			unsigned NUE2_YUV2RGB_COEF5        : 15;		// bits : 14_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_75; // 0x012c

	union
	{
		struct
		{
			unsigned NUE2_YUV2RGB_COEF6        : 15;		// bits : 14_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_76; // 0x0130

	union
	{
		struct
		{
			unsigned NUE2_YUV2RGB_COEF7        : 15;		// bits : 14_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_77; // 0x0134

	union
	{
		struct
		{
			unsigned NUE2_YUV2RGB_COEF8        : 15;		// bits : 14_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_78; // 0x0138

	union
	{
		struct
		{
			unsigned NUE2_YUV2RGB_BIAS0        : 9;		// bits : 8_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_79; // 0x013c

	union
	{
		struct
		{
			unsigned NUE2_YUV2RGB_BIAS1        : 9;		// bits : 8_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_80; // 0x0140

	union
	{
		struct
		{
			unsigned NUE2_YUV2RGB_BIAS2        : 9;		// bits : 8_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_81; // 0x0144

	union
	{
		struct
		{
			unsigned DRAM_MSB_SAI0         : 4;		// bits : 3_0
			unsigned                       : 24;
			unsigned DRAM_MODE_SAI0        : 4;		// bits : 31_28
		} Bit;
		UINT32 Word;
	} NUE2_Register_82; // 0x0148

	union
	{
		struct
		{
			unsigned DRAM_MSB_SAI1         : 4;		// bits : 3_0
			unsigned                       : 24;
			unsigned DRAM_MODE_SAI1        : 4;		// bits : 31_28
		} Bit;
		UINT32 Word;
	} NUE2_Register_83; // 0x014c

	union
	{
		struct
		{
			unsigned DRAM_MSB_SAI2         : 4;		// bits : 3_0
			unsigned                       : 24;
			unsigned DRAM_MODE_SAI2        : 4;		// bits : 31_28
		} Bit;
		UINT32 Word;
	} NUE2_Register_84; // 0x0150

	union
	{
		struct
		{
			unsigned DRAM_MSB_SAILL        : 4;		// bits : 3_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_85; // 0x0154

	union
	{
		struct
		{
			unsigned DRAM_MSB_SAO0         : 4;		// bits : 3_0
			unsigned                       : 24;
			unsigned DRAM_MODE_SAO0        : 4;		// bits : 31_28
		} Bit;
		UINT32 Word;
	} NUE2_Register_86; // 0x0158

	union
	{
		struct
		{
			unsigned DRAM_MSB_SAO1         : 4;		// bits : 3_0
			unsigned                       : 24;
			unsigned DRAM_MODE_SAO1        : 4;		// bits : 31_28
		} Bit;
		UINT32 Word;
	} NUE2_Register_87; // 0x015c

	union
	{
		struct
		{
			unsigned DRAM_MSB_SAO2         : 4;		// bits : 3_0
			unsigned                       : 24;
			unsigned DRAM_MODE_SAO2        : 4;		// bits : 31_28
		} Bit;
		UINT32 Word;
	} NUE2_Register_88; // 0x0160

	union
	{
		struct
		{
			unsigned LL_BASE_MSB_ADDR0        : 4;		// bits : 3_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_89; // 0x0164

	union
	{
		struct
		{
			unsigned MEAN_CH_SHIFT_DIR0        : 1;		// bits : 0
			unsigned                           : 3;
			unsigned MEAN_CH_SHIFT0            : 5;		// bits : 8_4
			unsigned                           : 3;
			unsigned MEAN_CH_SCALE0            : 16;		// bits : 27_12
		} Bit;
		UINT32 Word;
	} NUE2_Register_90; // 0x0168

	union
	{
		struct
		{
			unsigned MEAN_CH_SHIFT_DIR1        : 1;		// bits : 0
			unsigned                           : 3;
			unsigned MEAN_CH_SHIFT1            : 5;		// bits : 8_4
			unsigned                           : 3;
			unsigned MEAN_CH_SCALE1            : 16;		// bits : 27_12
		} Bit;
		UINT32 Word;
	} NUE2_Register_91; // 0x016c

	union
	{
		struct
		{
			unsigned MEAN_CH_SHIFT_DIR2        : 1;		// bits : 0
			unsigned                           : 3;
			unsigned MEAN_CH_SHIFT2            : 5;		// bits : 8_4
			unsigned                           : 3;
			unsigned MEAN_CH_SCALE2            : 16;		// bits : 27_12
		} Bit;
		UINT32 Word;
	} NUE2_Register_92; // 0x0170

	union
	{
		struct
		{
			unsigned LL_BASE_ADDR1        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_93; // 0x0174

	union
	{
		struct
		{
			unsigned LL_BASE_MSB_ADDR1        : 4;		// bits : 3_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_94; // 0x0178

	union
	{
		struct
		{
			unsigned LL_BASE_ADDR2        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_95; // 0x017c

	union
	{
		struct
		{
			unsigned LL_BASE_MSB_ADDR2        : 4;		// bits : 3_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_96; // 0x0180

	union
	{
		struct
		{
			unsigned LL_BASE_ADDR3        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_97; // 0x0184

	union
	{
		struct
		{
			unsigned LL_BASE_MSB_ADDR3        : 4;		// bits : 3_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_98; // 0x0188

	union
	{
		struct
		{
			unsigned LL_BASE_ADDR4        : 32;		// bits : 31_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_99; // 0x018c

	union
	{
		struct
		{
			unsigned LL_BASE_MSB_ADDR4        : 4;		// bits : 3_0
		} Bit;
		UINT32 Word;
	} NUE2_Register_100; // 0x0190

} NT98538_NUE2_REGISTER_STRUCT;

#ifdef __cplusplus
}
#endif


#endif
