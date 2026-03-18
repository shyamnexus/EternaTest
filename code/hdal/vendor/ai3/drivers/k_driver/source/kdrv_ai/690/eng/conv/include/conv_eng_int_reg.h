#ifndef _CONV_ENG_INT_REGISTER_H_
#define _CONV_ENG_INT_REGISTER_H_

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
    CONV_SW_RST  :    [0x0, 0x1],			bits : 0
    CONV_START   :    [0x0, 0x1],			bits : 1
    JOB_START    :    [0x0, 0x1],			bits : 28
    JOB_TERMINATE:    [0x0, 0x1],			bits : 29
*/
#define CONV_CONTROL_REGISTER_OFS 0x0000
REGDEF_BEGIN(CONV_CONTROL_REGISTER)
    REGDEF_BIT(CONV_SW_RST  ,        1)
    REGDEF_BIT(CONV_START   ,        1)
    REGDEF_BIT(             ,        26)
    REGDEF_BIT(JOB_START    ,        1)
    REGDEF_BIT(JOB_TERMINATE,        1)
REGDEF_END(CONV_CONTROL_REGISTER)


/*
    INTE_FRM_DONE                 :    [0x0, 0x1],			bits : 0
    INTE_WCD_DECODE_WEIGHT_NUM_ERR:    [0x0, 0x1],			bits : 4
    INTE_WCD_VLC_BITSTREAM_ERR    :    [0x0, 0x1],			bits : 8
    INTE_WCD_VLC_VAL_RANGE_ERR    :    [0x0, 0x1],			bits : 9
    INTE_ACT_LUT_ERR              :    [0x0, 0x1],			bits : 12
    INTE_JOB_END                  :    [0x0, 0x1],			bits : 24
    INTE_JOB_ERR                  :    [0x0, 0x1],			bits : 25
    INTE_JOB_WR_END               :    [0x0, 0x1],			bits : 26
*/
#define CONV_INTERRUPT_ENABLE_REGISTER_OFS 0x0004
REGDEF_BEGIN(CONV_INTERRUPT_ENABLE_REGISTER)
    REGDEF_BIT(INTE_FRM_DONE                 ,        1)
    REGDEF_BIT(                              ,        3)
    REGDEF_BIT(INTE_WCD_DECODE_WEIGHT_NUM_ERR,        1)
    REGDEF_BIT(                              ,        3)
    REGDEF_BIT(INTE_WCD_VLC_BITSTREAM_ERR    ,        1)
    REGDEF_BIT(INTE_WCD_VLC_VAL_RANGE_ERR    ,        1)
    REGDEF_BIT(                              ,        2)
    REGDEF_BIT(INTE_ACT_LUT_ERR              ,        1)
    REGDEF_BIT(                              ,        11)
    REGDEF_BIT(INTE_JOB_END                  ,        1)
    REGDEF_BIT(INTE_JOB_ERR                  ,        1)
    REGDEF_BIT(INTE_JOB_WR_END               ,        1)
REGDEF_END(CONV_INTERRUPT_ENABLE_REGISTER)


/*
    INTS_FRM_DONE                 :    [0x0, 0x1],			bits : 0
    INTS_WCD_DECODE_WEIGHT_NUM_ERR:    [0x0, 0x1],			bits : 4
    INTS_WCD_VLC_BITSTREAM_ERR    :    [0x0, 0x1],			bits : 8
    INTS_WCD_VLC_VAL_RANGE_ERR    :    [0x0, 0x1],			bits : 9
    INTS_ACT_LUT_ERR              :    [0x0, 0x1],			bits : 12
    INTS_JOB_END                  :    [0x0, 0x1],			bits : 24
    INTS_JOB_ERR                  :    [0x0, 0x1],			bits : 25
    INTS_JOB_WR_END               :    [0x0, 0x1],			bits : 26
*/
#define CONV_INTERRUPT_STATUS_REGISTER_OFS 0x0008
REGDEF_BEGIN(CONV_INTERRUPT_STATUS_REGISTER)
    REGDEF_BIT(INTS_FRM_DONE                 ,        1)
    REGDEF_BIT(                              ,        3)
    REGDEF_BIT(INTS_WCD_DECODE_WEIGHT_NUM_ERR,        1)
    REGDEF_BIT(                              ,        3)
    REGDEF_BIT(INTS_WCD_VLC_BITSTREAM_ERR    ,        1)
    REGDEF_BIT(INTS_WCD_VLC_VAL_RANGE_ERR    ,        1)
    REGDEF_BIT(                              ,        2)
    REGDEF_BIT(INTS_ACT_LUT_ERR              ,        1)
    REGDEF_BIT(                              ,        11)
    REGDEF_BIT(INTS_JOB_END                  ,        1)
    REGDEF_BIT(INTS_JOB_ERR                  ,        1)
    REGDEF_BIT(INTS_JOB_WR_END               ,        1)
REGDEF_END(CONV_INTERRUPT_STATUS_REGISTER)


/*
    CONV_CONV_EN      :    [0x0, 0x1],			bits : 0
    CONV_BIAS_EN      :    [0x0, 0x1],			bits : 4
    CONV_ROUTER_ORDER :    [0x0, 0x7],			bits : 10_8
    CONV_BN_EN        :    [0x0, 0x1],			bits : 12
    CONV_ELT_EN       :    [0x0, 0x1],			bits : 13
    CONV_ACT_EN       :    [0x0, 0x1],			bits : 14
    CONV_POOL_EN      :    [0x0, 0x1],			bits : 15
    CONV_WCD_VLC_EN   :    [0x0, 0x1],			bits : 25
    CONV_WCD_KMEANS_EN:    [0x0, 0x1],			bits : 26
    CONV_DW_EN        :    [0x0, 0x1],			bits : 28
    CONV_DW_BIAS_EN   :    [0x0, 0x1],			bits : 29
    CONV_DW_RELU_EN   :    [0x0, 0x1],			bits : 30
    CONV_DIM_MODE     :    [0x0, 0x1],			bits : 31
*/
#define CONV_MAIN_FLOW_CONTROL_REGISTER0_OFS 0x000c
REGDEF_BEGIN(CONV_MAIN_FLOW_CONTROL_REGISTER0)
    REGDEF_BIT(CONV_CONV_EN      ,        1)
    REGDEF_BIT(                  ,        3)
    REGDEF_BIT(CONV_BIAS_EN      ,        1)
    REGDEF_BIT(                  ,        3)
    REGDEF_BIT(CONV_ROUTER_ORDER ,        3)
    REGDEF_BIT(                  ,        1)
    REGDEF_BIT(CONV_BN_EN        ,        1)
    REGDEF_BIT(CONV_ELT_EN       ,        1)
    REGDEF_BIT(CONV_ACT_EN       ,        1)
    REGDEF_BIT(CONV_POOL_EN      ,        1)
    REGDEF_BIT(                  ,        9)
    REGDEF_BIT(CONV_WCD_VLC_EN   ,        1)
    REGDEF_BIT(CONV_WCD_KMEANS_EN,        1)
    REGDEF_BIT(                  ,        1)
    REGDEF_BIT(CONV_DW_EN        ,        1)
    REGDEF_BIT(CONV_DW_BIAS_EN   ,        1)
    REGDEF_BIT(CONV_DW_RELU_EN   ,        1)
    REGDEF_BIT(CONV_DIM_MODE     ,        1)
REGDEF_END(CONV_MAIN_FLOW_CONTROL_REGISTER0)


/*
    CONV_CONV_MODE         :    [0x0, 0x7],			bits : 2_0
    CONV_BN_W_MODE         :    [0x0, 0x1],			bits : 4
    CONV_ELT_MODE          :    [0x0, 0x1],			bits : 8
    CONV_POOL_MODE         :    [0x0, 0x1],			bits : 12
    CONV_IN1_BROADCAST_MODE:    [0x0, 0x7],			bits : 18_16
    CONV_ACT_A1_MODE       :    [0x0, 0x1],			bits : 20
    CONV_ACT_D_MODE        :    [0x0, 0x1],			bits : 21
    CONV_ACT_LUT_UPDATE_EN :    [0x0, 0x1],			bits : 22
    CONV_DW_RELU_MODE      :    [0x0, 0x1],			bits : 24
*/
#define CONV_SUB_FUNCTION_FLOW_CONTROL_REGISTER0_OFS 0x0010
REGDEF_BEGIN(CONV_SUB_FUNCTION_FLOW_CONTROL_REGISTER0)
    REGDEF_BIT(CONV_CONV_MODE         ,        3)
    REGDEF_BIT(                       ,        1)
    REGDEF_BIT(CONV_BN_W_MODE         ,        1)
    REGDEF_BIT(                       ,        3)
    REGDEF_BIT(CONV_ELT_MODE          ,        1)
    REGDEF_BIT(                       ,        3)
    REGDEF_BIT(CONV_POOL_MODE         ,        1)
    REGDEF_BIT(                       ,        3)
    REGDEF_BIT(CONV_IN1_BROADCAST_MODE,        3)
    REGDEF_BIT(                       ,        1)
    REGDEF_BIT(CONV_ACT_A1_MODE       ,        1)
    REGDEF_BIT(CONV_ACT_D_MODE        ,        1)
    REGDEF_BIT(CONV_ACT_LUT_UPDATE_EN ,        1)
    REGDEF_BIT(                       ,        1)
    REGDEF_BIT(CONV_DW_RELU_MODE      ,        1)
REGDEF_END(CONV_SUB_FUNCTION_FLOW_CONTROL_REGISTER0)


/*
    CONV_IN0_REMAINSRC_EN        :    [0x0, 0x1],			bits : 0
    CONV_IN0_SWITCHSRC_EN        :    [0x0, 0x1],			bits : 1
    CONV_IN0_MERGESRC_EN         :    [0x0, 0x1],			bits : 2
    CONV_PRE_IN0_LOAD_EN         :    [0x0, 0x1],			bits : 3
    CONV_IN0_PERMUTE_WC_EN       :    [0x0, 0x1],			bits : 4
    CONV_IN0_FLATTEN2CH_EN       :    [0x0, 0x1],			bits : 5
    CONV_IN_CH_STRIPE_EN         :    [0x0, 0x1],			bits : 6
    CONV_OUT0_DATA_MODE          :    [0x0, 0x3],			bits : 9_8
    CONV_WCD_VLC_VAL_CONVERT_EN  :    [0x0, 0x1],			bits : 20
    CONV_WCD_KMEANS_TBL_UPDATE_EN:    [0x0, 0x1],			bits : 21
    CONV_WCD_VLC_BIT_EN          :    [0x0, 0xf],			bits : 27_24
    CONV_WCD_KMEANS_ENTRY_MODE   :    [0x0, 0x3],			bits : 29_28
*/
#define CONV_SUB_FUNCTION_FLOW_CONTROL_REGISTER1_OFS 0x0014
REGDEF_BEGIN(CONV_SUB_FUNCTION_FLOW_CONTROL_REGISTER1)
    REGDEF_BIT(CONV_IN0_REMAINSRC_EN        ,        1)
    REGDEF_BIT(CONV_IN0_SWITCHSRC_EN        ,        1)
    REGDEF_BIT(CONV_IN0_MERGESRC_EN         ,        1)
    REGDEF_BIT(CONV_PRE_IN0_LOAD_EN         ,        1)
    REGDEF_BIT(CONV_IN0_PERMUTE_WC_EN       ,        1)
    REGDEF_BIT(CONV_IN0_FLATTEN2CH_EN       ,        1)
    REGDEF_BIT(CONV_IN_CH_STRIPE_EN         ,        1)
    REGDEF_BIT(                             ,        1)
    REGDEF_BIT(CONV_OUT0_DATA_MODE          ,        2)
    REGDEF_BIT(                             ,        10)
    REGDEF_BIT(CONV_WCD_VLC_VAL_CONVERT_EN  ,        1)
    REGDEF_BIT(CONV_WCD_KMEANS_TBL_UPDATE_EN,        1)
    REGDEF_BIT(                             ,        2)
    REGDEF_BIT(CONV_WCD_VLC_BIT_EN          ,        4)
    REGDEF_BIT(CONV_WCD_KMEANS_ENTRY_MODE   ,        2)
REGDEF_END(CONV_SUB_FUNCTION_FLOW_CONTROL_REGISTER1)


/*
    CONV_QUAN_CONVOUT_MODE:    [0x0, 0x1],			bits : 16
    CONV_QUAN_OUT0_MODE   :    [0x0, 0x1],			bits : 20
*/
#define CONV_SUB_FUNCTION_FLOW_CONTROL_REGISTER2_OFS 0x0018
REGDEF_BEGIN(CONV_SUB_FUNCTION_FLOW_CONTROL_REGISTER2)
    REGDEF_BIT(                      ,        16)
    REGDEF_BIT(CONV_QUAN_CONVOUT_MODE,        1)
    REGDEF_BIT(                      ,        3)
    REGDEF_BIT(CONV_QUAN_OUT0_MODE   ,        1)
REGDEF_END(CONV_SUB_FUNCTION_FLOW_CONTROL_REGISTER2)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED0_OFS 0x001c
REGDEF_BEGIN(RESERVED0)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED0)


/*
    BASE_ADDR0:    [0x0, 0xffffffff],			bits : 31_0
*/
#define LL_BASE_ADDRESS_REGISTER0_OFS 0x0020
REGDEF_BEGIN(LL_BASE_ADDRESS_REGISTER0)
    REGDEF_BIT(BASE_ADDR0,        32)
REGDEF_END(LL_BASE_ADDRESS_REGISTER0)


/*
    BASE_MSB_ADDR0:    [0x0, 0xf],			bits : 3_0
*/
#define LL_BASE_ADDRESS_REGISTER1_OFS 0x0024
REGDEF_BEGIN(LL_BASE_ADDRESS_REGISTER1)
    REGDEF_BIT(BASE_MSB_ADDR0,        4)
REGDEF_END(LL_BASE_ADDRESS_REGISTER1)


/*
    BASE_ADDR1:    [0x0, 0xffffffff],			bits : 31_0
*/
#define LL_BASE_ADDRESS_REGISTER2_OFS 0x0028
REGDEF_BEGIN(LL_BASE_ADDRESS_REGISTER2)
    REGDEF_BIT(BASE_ADDR1,        32)
REGDEF_END(LL_BASE_ADDRESS_REGISTER2)


/*
    BASE_MSB_ADDR1:    [0x0, 0xf],			bits : 3_0
*/
#define LL_BASE_ADDRESS_REGISTER3_OFS 0x002c
REGDEF_BEGIN(LL_BASE_ADDRESS_REGISTER3)
    REGDEF_BIT(BASE_MSB_ADDR1,        4)
REGDEF_END(LL_BASE_ADDRESS_REGISTER3)


/*
    BASE_ADDR2:    [0x0, 0xffffffff],			bits : 31_0
*/
#define LL_BASE_ADDRESS_REGISTER4_OFS 0x0030
REGDEF_BEGIN(LL_BASE_ADDRESS_REGISTER4)
    REGDEF_BIT(BASE_ADDR2,        32)
REGDEF_END(LL_BASE_ADDRESS_REGISTER4)


/*
    BASE_MSB_ADDR2:    [0x0, 0xf],			bits : 3_0
*/
#define LL_BASE_ADDRESS_REGISTER5_OFS 0x0034
REGDEF_BEGIN(LL_BASE_ADDRESS_REGISTER5)
    REGDEF_BIT(BASE_MSB_ADDR2,        4)
REGDEF_END(LL_BASE_ADDRESS_REGISTER5)


/*
    BASE_ADDR3:    [0x0, 0xffffffff],			bits : 31_0
*/
#define LL_BASE_ADDRESS_REGISTER6_OFS 0x0038
REGDEF_BEGIN(LL_BASE_ADDRESS_REGISTER6)
    REGDEF_BIT(BASE_ADDR3,        32)
REGDEF_END(LL_BASE_ADDRESS_REGISTER6)


/*
    BASE_MSB_ADDR3:    [0x0, 0xf],			bits : 3_0
*/
#define LL_BASE_ADDRESS_REGISTER7_OFS 0x003c
REGDEF_BEGIN(LL_BASE_ADDRESS_REGISTER7)
    REGDEF_BIT(BASE_MSB_ADDR3,        4)
REGDEF_END(LL_BASE_ADDRESS_REGISTER7)


/*
    DRAMUB_SAIJOB:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_JOB_CHANNEL_REGISTER0_OFS 0x0040
REGDEF_BEGIN(DMA_TO_CONV_JOB_CHANNEL_REGISTER0)
    REGDEF_BIT(DRAMUB_SAIJOB,        32)
REGDEF_END(DMA_TO_CONV_JOB_CHANNEL_REGISTER0)


/*
    DRAMUB_MSB_SAIJOB:    [0x0, 0xf],			bits : 3_0
*/
#define DMA_TO_CONV_JOB_CHANNEL_REGISTER1_OFS 0x0044
REGDEF_BEGIN(DMA_TO_CONV_JOB_CHANNEL_REGISTER1)
    REGDEF_BIT(DRAMUB_MSB_SAIJOB,        4)
REGDEF_END(DMA_TO_CONV_JOB_CHANNEL_REGISTER1)


/*
    BASE_ADDR4:    [0x0, 0xffffffff],			bits : 31_0
*/
#define LL_BASE_ADDRESS_REGISTER8_OFS 0x0048
REGDEF_BEGIN(LL_BASE_ADDRESS_REGISTER8)
    REGDEF_BIT(BASE_ADDR4,        32)
REGDEF_END(LL_BASE_ADDRESS_REGISTER8)


/*
    BASE_MSB_ADDR4:    [0x0, 0xf],			bits : 3_0
*/
#define LL_BASE_ADDRESS_REGISTER9_OFS 0x004c
REGDEF_BEGIN(LL_BASE_ADDRESS_REGISTER9)
    REGDEF_BIT(BASE_MSB_ADDR4,        4)
REGDEF_END(LL_BASE_ADDRESS_REGISTER9)


/*
    DRAMUB_SAI0:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER0_OFS 0x0050
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER0)
    REGDEF_BIT(DRAMUB_SAI0,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER0)


/*
    DRAMUB_MSB_SAI0 :    [0x0, 0xf],			bits : 3_0
    DRAMUB_MODE_SAI0:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_CONV_CHANNEL_REGISTER1_OFS 0x0054
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER1)
    REGDEF_BIT(DRAMUB_MSB_SAI0 ,        4)
    REGDEF_BIT(                ,        24)
    REGDEF_BIT(DRAMUB_MODE_SAI0,        4)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER1)


/*
    DRAMUB_SAI1:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER2_OFS 0x0058
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER2)
    REGDEF_BIT(DRAMUB_SAI1,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER2)


/*
    DRAMUB_MSB_SAI1 :    [0x0, 0xf],			bits : 3_0
    DRAMUB_MODE_SAI1:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_CONV_CHANNEL_REGISTER3_OFS 0x005c
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER3)
    REGDEF_BIT(DRAMUB_MSB_SAI1 ,        4)
    REGDEF_BIT(                ,        24)
    REGDEF_BIT(DRAMUB_MODE_SAI1,        4)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER3)


/*
    DRAMUB_SAI2:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER4_OFS 0x0060
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER4)
    REGDEF_BIT(DRAMUB_SAI2,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER4)


/*
    DRAMUB_MSB_SAI2 :    [0x0, 0xf],			bits : 3_0
    DRAMUB_MODE_SAI2:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_CONV_CHANNEL_REGISTER5_OFS 0x0064
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER5)
    REGDEF_BIT(DRAMUB_MSB_SAI2 ,        4)
    REGDEF_BIT(                ,        24)
    REGDEF_BIT(DRAMUB_MODE_SAI2,        4)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER5)


/*
    DRAMUB_SAI3:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER6_OFS 0x0068
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER6)
    REGDEF_BIT(DRAMUB_SAI3,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER6)


/*
    DRAMUB_MSB_SAI3 :    [0x0, 0xf],			bits : 3_0
    DRAMUB_MODE_SAI3:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_CONV_CHANNEL_REGISTER7_OFS 0x006c
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER7)
    REGDEF_BIT(DRAMUB_MSB_SAI3 ,        4)
    REGDEF_BIT(                ,        24)
    REGDEF_BIT(DRAMUB_MODE_SAI3,        4)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER7)


/*
    DRAMUB_SAI4:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER8_OFS 0x0070
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER8)
    REGDEF_BIT(DRAMUB_SAI4,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER8)


/*
    DRAMUB_MSB_SAI4 :    [0x0, 0xf],			bits : 3_0
    DRAMUB_MODE_SAI4:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_CONV_CHANNEL_REGISTER9_OFS 0x0074
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER9)
    REGDEF_BIT(DRAMUB_MSB_SAI4 ,        4)
    REGDEF_BIT(                ,        24)
    REGDEF_BIT(DRAMUB_MODE_SAI4,        4)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER9)


/*
    DRAMUB_SAI5:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER10_OFS 0x0078
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER10)
    REGDEF_BIT(DRAMUB_SAI5,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER10)


/*
    DRAMUB_MSB_SAI5 :    [0x0, 0xf],			bits : 3_0
    DRAMUB_MODE_SAI5:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_CONV_CHANNEL_REGISTER11_OFS 0x007c
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER11)
    REGDEF_BIT(DRAMUB_MSB_SAI5 ,        4)
    REGDEF_BIT(                ,        24)
    REGDEF_BIT(DRAMUB_MODE_SAI5,        4)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER11)


/*
    DRAMUB_SAI6:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER12_OFS 0x0080
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER12)
    REGDEF_BIT(DRAMUB_SAI6,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER12)


/*
    DRAMUB_MSB_SAI6 :    [0x0, 0xf],			bits : 3_0
    DRAMUB_MODE_SAI6:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_CONV_CHANNEL_REGISTER13_OFS 0x0084
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER13)
    REGDEF_BIT(DRAMUB_MSB_SAI6 ,        4)
    REGDEF_BIT(                ,        24)
    REGDEF_BIT(DRAMUB_MODE_SAI6,        4)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER13)


/*
    DRAMUB_SAI7:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER14_OFS 0x0088
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER14)
    REGDEF_BIT(DRAMUB_SAI7,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER14)


/*
    DRAMUB_MSB_SAI7 :    [0x0, 0xf],			bits : 3_0
    DRAMUB_MODE_SAI7:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_CONV_CHANNEL_REGISTER15_OFS 0x008c
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER15)
    REGDEF_BIT(DRAMUB_MSB_SAI7 ,        4)
    REGDEF_BIT(                ,        24)
    REGDEF_BIT(DRAMUB_MODE_SAI7,        4)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER15)


/*
    DRAMUB_SAI8:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER16_OFS 0x0090
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER16)
    REGDEF_BIT(DRAMUB_SAI8,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER16)


/*
    DRAMUB_MSB_SAI8 :    [0x0, 0xf],			bits : 3_0
    DRAMUB_MODE_SAI8:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_CONV_CHANNEL_REGISTER17_OFS 0x0094
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER17)
    REGDEF_BIT(DRAMUB_MSB_SAI8 ,        4)
    REGDEF_BIT(                ,        24)
    REGDEF_BIT(DRAMUB_MODE_SAI8,        4)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER17)


/*
    DRAMUB_SAI9:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER18_OFS 0x0098
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER18)
    REGDEF_BIT(DRAMUB_SAI9,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER18)


/*
    DRAMUB_MSB_SAI9 :    [0x0, 0xf],			bits : 3_0
    DRAMUB_MODE_SAI9:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_CONV_CHANNEL_REGISTER19_OFS 0x009c
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER19)
    REGDEF_BIT(DRAMUB_MSB_SAI9 ,        4)
    REGDEF_BIT(                ,        24)
    REGDEF_BIT(DRAMUB_MODE_SAI9,        4)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER19)


/*
    DRAMUB_SAI10:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER20_OFS 0x00a0
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER20)
    REGDEF_BIT(DRAMUB_SAI10,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER20)


/*
    DRAMUB_MSB_SAI10 :    [0x0, 0xf],			bits : 3_0
    DRAMUB_MODE_SAI10:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_CONV_CHANNEL_REGISTER21_OFS 0x00a4
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER21)
    REGDEF_BIT(DRAMUB_MSB_SAI10 ,        4)
    REGDEF_BIT(                 ,        24)
    REGDEF_BIT(DRAMUB_MODE_SAI10,        4)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER21)


/*
    DRAMUB_SAI11:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER22_OFS 0x00a8
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER22)
    REGDEF_BIT(DRAMUB_SAI11,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER22)


/*
    DRAMUB_MSB_SAI11 :    [0x0, 0xf],			bits : 3_0
    DRAMUB_MODE_SAI11:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_CONV_CHANNEL_REGISTER23_OFS 0x00ac
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER23)
    REGDEF_BIT(DRAMUB_MSB_SAI11 ,        4)
    REGDEF_BIT(                 ,        24)
    REGDEF_BIT(DRAMUB_MODE_SAI11,        4)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER23)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER24_OFS 0x00b0
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER24)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER24)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER25_OFS 0x00b4
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER25)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER25)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER26_OFS 0x00b8
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER26)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER26)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER27_OFS 0x00bc
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER27)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER27)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER28_OFS 0x00c0
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER28)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER28)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER29_OFS 0x00c4
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER29)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER29)


/*
    DRAMUB_SAI15:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER30_OFS 0x00c8
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER30)
    REGDEF_BIT(DRAMUB_SAI15,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER30)


/*
    DRAMUB_MSB_SAI15 :    [0x0, 0xf],			bits : 3_0
    DRAMUB_MODE_SAI15:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_CONV_CHANNEL_REGISTER31_OFS 0x00cc
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER31)
    REGDEF_BIT(DRAMUB_MSB_SAI15 ,        4)
    REGDEF_BIT(                 ,        24)
    REGDEF_BIT(DRAMUB_MODE_SAI15,        4)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER31)


/*
    DRAMUB_SAI16:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER32_OFS 0x00d0
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER32)
    REGDEF_BIT(DRAMUB_SAI16,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER32)


/*
    DRAMUB_MSB_SAI16 :    [0x0, 0xf],			bits : 3_0
    DRAMUB_MODE_SAI16:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_CONV_CHANNEL_REGISTER33_OFS 0x00d4
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER33)
    REGDEF_BIT(DRAMUB_MSB_SAI16 ,        4)
    REGDEF_BIT(                 ,        24)
    REGDEF_BIT(DRAMUB_MODE_SAI16,        4)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER33)


/*
    DRAMUB_SAI17:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER34_OFS 0x00d8
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER34)
    REGDEF_BIT(DRAMUB_SAI17,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER34)


/*
    DRAMUB_MSB_SAI17 :    [0x0, 0xf],			bits : 3_0
    DRAMUB_MODE_SAI17:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_CONV_CHANNEL_REGISTER35_OFS 0x00dc
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER35)
    REGDEF_BIT(DRAMUB_MSB_SAI17 ,        4)
    REGDEF_BIT(                 ,        24)
    REGDEF_BIT(DRAMUB_MODE_SAI17,        4)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER35)


/*
    DRAMUB_SAI18:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER36_OFS 0x00e0
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER36)
    REGDEF_BIT(DRAMUB_SAI18,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER36)


/*
    DRAMUB_MSB_SAI18 :    [0x0, 0xf],			bits : 3_0
    DRAMUB_MODE_SAI18:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_CONV_CHANNEL_REGISTER37_OFS 0x00e4
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER37)
    REGDEF_BIT(DRAMUB_MSB_SAI18 ,        4)
    REGDEF_BIT(                 ,        24)
    REGDEF_BIT(DRAMUB_MODE_SAI18,        4)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER37)


/*
    DRAMUB_SAI19:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER38_OFS 0x00e8
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER38)
    REGDEF_BIT(DRAMUB_SAI19,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER38)


/*
    DRAMUB_MSB_SAI19 :    [0x0, 0xf],			bits : 3_0
    DRAMUB_MODE_SAI19:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_CONV_CHANNEL_REGISTER39_OFS 0x00ec
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER39)
    REGDEF_BIT(DRAMUB_MSB_SAI19 ,        4)
    REGDEF_BIT(                 ,        24)
    REGDEF_BIT(DRAMUB_MODE_SAI19,        4)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER39)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER40_OFS 0x00f0
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER40)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER40)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER41_OFS 0x00f4
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER41)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER41)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER42_OFS 0x00f8
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER42)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER42)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER43_OFS 0x00fc
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER43)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER43)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER44_OFS 0x0100
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER44)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER44)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER45_OFS 0x0104
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER45)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER45)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER46_OFS 0x0108
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER46)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER46)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER47_OFS 0x010c
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER47)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER47)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER48_OFS 0x0110
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER48)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER48)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER49_OFS 0x0114
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER49)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER49)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER50_OFS 0x0118
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER50)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER50)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER51_OFS 0x011c
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER51)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER51)


/*
    DRAMUB_SAO0:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER52_OFS 0x0120
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER52)
    REGDEF_BIT(DRAMUB_SAO0,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER52)


/*
    DRAMUB_MSB_SAO0 :    [0x0, 0xf],			bits : 3_0
    DRAMUB_MODE_SAO0:    [0x0, 0xf],			bits : 31_28
*/
#define DMA_TO_CONV_CHANNEL_REGISTER53_OFS 0x0124
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER53)
    REGDEF_BIT(DRAMUB_MSB_SAO0 ,        4)
    REGDEF_BIT(                ,        24)
    REGDEF_BIT(DRAMUB_MODE_SAO0,        4)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER53)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER54_OFS 0x0128
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER54)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER54)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER55_OFS 0x012c
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER55)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER55)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER56_OFS 0x0130
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER56)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER56)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define DMA_TO_CONV_CHANNEL_REGISTER57_OFS 0x0134
REGDEF_BEGIN(DMA_TO_CONV_CHANNEL_REGISTER57)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(DMA_TO_CONV_CHANNEL_REGISTER57)


/*
    CONV_IN0_LOFS_EN          :    [0x0, 0x1],			bits : 0
    CONV_IN0_CHOFS_EN         :    [0x0, 0x1],			bits : 1
    CONV_IN0_BOFS_EN          :    [0x0, 0x1],			bits : 2
    CONV_IN1_LOFS_EN          :    [0x0, 0x1],			bits : 4
    CONV_IN1_CHOFS_EN         :    [0x0, 0x1],			bits : 5
    CONV_IN1_BOFS_EN          :    [0x0, 0x1],			bits : 6
    CONV_IN_CH_STRIPE_LOFS_EN :    [0x0, 0x1],			bits : 8
    CONV_IN_CH_STRIPE_CHOFS_EN:    [0x0, 0x1],			bits : 9
    CONV_IN_CH_STRIPE_BOFS_EN :    [0x0, 0x1],			bits : 10
    CONV_W_LOFS_EN            :    [0x0, 0x1],			bits : 12
    CONV_W_CHOFS_EN           :    [0x0, 0x1],			bits : 13
    CONV_W_BOFS_EN            :    [0x0, 0x1],			bits : 14
    CONV_W_DOFS_EN            :    [0x0, 0x1],			bits : 15
    CONV_OUT0_LOFS_EN         :    [0x0, 0x1],			bits : 16
    CONV_OUT0_CHOFS_EN        :    [0x0, 0x1],			bits : 17
    CONV_OUT0_BOFS_EN         :    [0x0, 0x1],			bits : 18
*/
#define DMA_TO_CONV_OFFSET_ENABLE_REGISTER0_OFS 0x0138
REGDEF_BEGIN(DMA_TO_CONV_OFFSET_ENABLE_REGISTER0)
    REGDEF_BIT(CONV_IN0_LOFS_EN          ,        1)
    REGDEF_BIT(CONV_IN0_CHOFS_EN         ,        1)
    REGDEF_BIT(CONV_IN0_BOFS_EN          ,        1)
    REGDEF_BIT(                          ,        1)
    REGDEF_BIT(CONV_IN1_LOFS_EN          ,        1)
    REGDEF_BIT(CONV_IN1_CHOFS_EN         ,        1)
    REGDEF_BIT(CONV_IN1_BOFS_EN          ,        1)
    REGDEF_BIT(                          ,        1)
    REGDEF_BIT(CONV_IN_CH_STRIPE_LOFS_EN ,        1)
    REGDEF_BIT(CONV_IN_CH_STRIPE_CHOFS_EN,        1)
    REGDEF_BIT(CONV_IN_CH_STRIPE_BOFS_EN ,        1)
    REGDEF_BIT(                          ,        1)
    REGDEF_BIT(CONV_W_LOFS_EN            ,        1)
    REGDEF_BIT(CONV_W_CHOFS_EN           ,        1)
    REGDEF_BIT(CONV_W_BOFS_EN            ,        1)
    REGDEF_BIT(CONV_W_DOFS_EN            ,        1)
    REGDEF_BIT(CONV_OUT0_LOFS_EN         ,        1)
    REGDEF_BIT(CONV_OUT0_CHOFS_EN        ,        1)
    REGDEF_BIT(CONV_OUT0_BOFS_EN         ,        1)
REGDEF_END(DMA_TO_CONV_OFFSET_ENABLE_REGISTER0)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED1_OFS 0x013c
REGDEF_BEGIN(RESERVED1)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED1)


/*
    CONV_IN0_LOFS:    [0x0, 0x1fffffff],			bits : 28_0
*/
#define DMA_TO_CONV_OFFSET_REGISTER0_OFS 0x0140
REGDEF_BEGIN(DMA_TO_CONV_OFFSET_REGISTER0)
    REGDEF_BIT(CONV_IN0_LOFS,        29)
REGDEF_END(DMA_TO_CONV_OFFSET_REGISTER0)


/*
    CONV_IN0_CHOFS:    [0x0, 0x1fffffff],			bits : 28_0
*/
#define DMA_TO_CONV_OFFSET_REGISTER1_OFS 0x0144
REGDEF_BEGIN(DMA_TO_CONV_OFFSET_REGISTER1)
    REGDEF_BIT(CONV_IN0_CHOFS,        29)
REGDEF_END(DMA_TO_CONV_OFFSET_REGISTER1)


/*
    CONV_IN0_BOFS:    [0x0, 0x1fffffff],			bits : 28_0
*/
#define DMA_TO_CONV_OFFSET_REGISTER2_OFS 0x0148
REGDEF_BEGIN(DMA_TO_CONV_OFFSET_REGISTER2)
    REGDEF_BIT(CONV_IN0_BOFS,        29)
REGDEF_END(DMA_TO_CONV_OFFSET_REGISTER2)


/*
    CONV_IN1_LOFS:    [0x0, 0x1fffffff],			bits : 28_0
*/
#define DMA_TO_CONV_OFFSET_REGISTER3_OFS 0x014c
REGDEF_BEGIN(DMA_TO_CONV_OFFSET_REGISTER3)
    REGDEF_BIT(CONV_IN1_LOFS,        29)
REGDEF_END(DMA_TO_CONV_OFFSET_REGISTER3)


/*
    CONV_IN1_CHOFS:    [0x0, 0x1fffffff],			bits : 28_0
*/
#define DMA_TO_CONV_OFFSET_REGISTER4_OFS 0x0150
REGDEF_BEGIN(DMA_TO_CONV_OFFSET_REGISTER4)
    REGDEF_BIT(CONV_IN1_CHOFS,        29)
REGDEF_END(DMA_TO_CONV_OFFSET_REGISTER4)


/*
    CONV_IN1_BOFS:    [0x0, 0x1fffffff],			bits : 28_0
*/
#define DMA_TO_CONV_OFFSET_REGISTER5_OFS 0x0154
REGDEF_BEGIN(DMA_TO_CONV_OFFSET_REGISTER5)
    REGDEF_BIT(CONV_IN1_BOFS,        29)
REGDEF_END(DMA_TO_CONV_OFFSET_REGISTER5)


/*
    CONV_IN_CH_STRIPE_LOFS:    [0x0, 0x1fffffff],			bits : 28_0
*/
#define DMA_TO_CONV_OFFSET_REGISTER6_OFS 0x0158
REGDEF_BEGIN(DMA_TO_CONV_OFFSET_REGISTER6)
    REGDEF_BIT(CONV_IN_CH_STRIPE_LOFS,        29)
REGDEF_END(DMA_TO_CONV_OFFSET_REGISTER6)


/*
    CONV_IN_CH_STRIPE_CHOFS:    [0x0, 0x1fffffff],			bits : 28_0
*/
#define DMA_TO_CONV_OFFSET_REGISTER7_OFS 0x015c
REGDEF_BEGIN(DMA_TO_CONV_OFFSET_REGISTER7)
    REGDEF_BIT(CONV_IN_CH_STRIPE_CHOFS,        29)
REGDEF_END(DMA_TO_CONV_OFFSET_REGISTER7)


/*
    CONV_IN_CH_STRIPE_BOFS:    [0x0, 0x1fffffff],			bits : 28_0
*/
#define DMA_TO_CONV_OFFSET_REGISTER8_OFS 0x0160
REGDEF_BEGIN(DMA_TO_CONV_OFFSET_REGISTER8)
    REGDEF_BIT(CONV_IN_CH_STRIPE_BOFS,        29)
REGDEF_END(DMA_TO_CONV_OFFSET_REGISTER8)


/*
    CONV_W_LOFS:    [0x0, 0x1fffffff],			bits : 28_0
*/
#define DMA_TO_CONV_OFFSET_REGISTER9_OFS 0x0164
REGDEF_BEGIN(DMA_TO_CONV_OFFSET_REGISTER9)
    REGDEF_BIT(CONV_W_LOFS,        29)
REGDEF_END(DMA_TO_CONV_OFFSET_REGISTER9)


/*
    CONV_W_CHOFS:    [0x0, 0x1fffffff],			bits : 28_0
*/
#define DMA_TO_CONV_OFFSET_REGISTER10_OFS 0x0168
REGDEF_BEGIN(DMA_TO_CONV_OFFSET_REGISTER10)
    REGDEF_BIT(CONV_W_CHOFS,        29)
REGDEF_END(DMA_TO_CONV_OFFSET_REGISTER10)


/*
    CONV_W_BOFS:    [0x0, 0x1fffffff],			bits : 28_0
*/
#define DMA_TO_CONV_OFFSET_REGISTER11_OFS 0x016c
REGDEF_BEGIN(DMA_TO_CONV_OFFSET_REGISTER11)
    REGDEF_BIT(CONV_W_BOFS,        29)
REGDEF_END(DMA_TO_CONV_OFFSET_REGISTER11)


/*
    CONV_OUT0_LOFS:    [0x0, 0x1fffffff],			bits : 28_0
*/
#define DMA_TO_CONV_OFFSET_REGISTER12_OFS 0x0170
REGDEF_BEGIN(DMA_TO_CONV_OFFSET_REGISTER12)
    REGDEF_BIT(CONV_OUT0_LOFS,        29)
REGDEF_END(DMA_TO_CONV_OFFSET_REGISTER12)


/*
    CONV_OUT0_CHOFS:    [0x0, 0x1fffffff],			bits : 28_0
*/
#define DMA_TO_CONV_OFFSET_REGISTER13_OFS 0x0174
REGDEF_BEGIN(DMA_TO_CONV_OFFSET_REGISTER13)
    REGDEF_BIT(CONV_OUT0_CHOFS,        29)
REGDEF_END(DMA_TO_CONV_OFFSET_REGISTER13)


/*
    CONV_OUT0_BOFS:    [0x0, 0x1fffffff],			bits : 28_0
*/
#define DMA_TO_CONV_OFFSET_REGISTER14_OFS 0x0178
REGDEF_BEGIN(DMA_TO_CONV_OFFSET_REGISTER14)
    REGDEF_BIT(CONV_OUT0_BOFS,        29)
REGDEF_END(DMA_TO_CONV_OFFSET_REGISTER14)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED2_OFS 0x017c
REGDEF_BEGIN(RESERVED2)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED2)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED3_OFS 0x0180
REGDEF_BEGIN(RESERVED3)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED3)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED4_OFS 0x0184
REGDEF_BEGIN(RESERVED4)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED4)


/*
    CONV_W_DOFS:    [0x0, 0x1fffffff],			bits : 28_0
*/
#define DMA_TO_CONV_OFFSET_REGISTER15_OFS 0x0188
REGDEF_BEGIN(DMA_TO_CONV_OFFSET_REGISTER15)
    REGDEF_BIT(CONV_W_DOFS,        29)
REGDEF_END(DMA_TO_CONV_OFFSET_REGISTER15)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED5_OFS 0x018c
REGDEF_BEGIN(RESERVED5)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED5)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED6_OFS 0x0190
REGDEF_BEGIN(RESERVED6)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED6)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED7_OFS 0x0194
REGDEF_BEGIN(RESERVED7)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED7)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED8_OFS 0x0198
REGDEF_BEGIN(RESERVED8)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED8)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED9_OFS 0x019c
REGDEF_BEGIN(RESERVED9)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED9)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED10_OFS 0x01a0
REGDEF_BEGIN(RESERVED10)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED10)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED11_OFS 0x01a4
REGDEF_BEGIN(RESERVED11)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED11)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED12_OFS 0x01a8
REGDEF_BEGIN(RESERVED12)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED12)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED13_OFS 0x01ac
REGDEF_BEGIN(RESERVED13)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED13)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED14_OFS 0x01b0
REGDEF_BEGIN(RESERVED14)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED14)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED15_OFS 0x01b4
REGDEF_BEGIN(RESERVED15)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED15)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED16_OFS 0x01b8
REGDEF_BEGIN(RESERVED16)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED16)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED17_OFS 0x01bc
REGDEF_BEGIN(RESERVED17)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED17)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED18_OFS 0x01c0
REGDEF_BEGIN(RESERVED18)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED18)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED19_OFS 0x01c4
REGDEF_BEGIN(RESERVED19)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED19)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED20_OFS 0x01c8
REGDEF_BEGIN(RESERVED20)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED20)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED21_OFS 0x01cc
REGDEF_BEGIN(RESERVED21)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED21)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED22_OFS 0x01d0
REGDEF_BEGIN(RESERVED22)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED22)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED23_OFS 0x01d4
REGDEF_BEGIN(RESERVED23)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED23)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED24_OFS 0x01d8
REGDEF_BEGIN(RESERVED24)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED24)


/*
    CONV_IN0_WIDTH :    [0x0, 0xfff],			bits : 11_0
    CONV_IN0_HEIGHT:    [0x0, 0xfff],			bits : 27_16
*/
#define CONV_SIZE_REGISTER0_OFS 0x01dc
REGDEF_BEGIN(CONV_SIZE_REGISTER0)
    REGDEF_BIT(CONV_IN0_WIDTH ,        12)
    REGDEF_BIT(               ,        4)
    REGDEF_BIT(CONV_IN0_HEIGHT,        12)
REGDEF_END(CONV_SIZE_REGISTER0)


/*
    CONV_IN0_CHANNEL:    [0x0, 0xffff],			bits : 15_0
    CONV_IN0_BATCH  :    [0x0, 0x7f],			bits : 26_20
*/
#define CONV_SIZE_REGISTER1_OFS 0x01e0
REGDEF_BEGIN(CONV_SIZE_REGISTER1)
    REGDEF_BIT(CONV_IN0_CHANNEL,        16)
    REGDEF_BIT(                ,        4)
    REGDEF_BIT(CONV_IN0_BATCH  ,        7)
REGDEF_END(CONV_SIZE_REGISTER1)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED25_OFS 0x01e4
REGDEF_BEGIN(RESERVED25)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED25)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED26_OFS 0x01e8
REGDEF_BEGIN(RESERVED26)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED26)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED27_OFS 0x01ec
REGDEF_BEGIN(RESERVED27)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED27)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED28_OFS 0x01f0
REGDEF_BEGIN(RESERVED28)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED28)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED29_OFS 0x01f4
REGDEF_BEGIN(RESERVED29)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED29)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED30_OFS 0x01f8
REGDEF_BEGIN(RESERVED30)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED30)


/*
    CONV_IN0_BIT_DEPTH :    [0x0, 0x3],			bits : 1_0
    CONV_IN0_DATA_FMT  :    [0x0, 0x1],			bits : 4
    CONV_IN1_BIT_DEPTH :    [0x0, 0x3],			bits : 9_8
    CONV_IN1_DATA_FMT  :    [0x0, 0x1],			bits : 12
    CONV_OUT0_BIT_DEPTH:    [0x0, 0x7],			bits : 18_16
    CONV_OUT0_DATA_FMT :    [0x0, 0x3],			bits : 21_20
*/
#define CONV_DATA_TYPE_REGISTER0_OFS 0x01fc
REGDEF_BEGIN(CONV_DATA_TYPE_REGISTER0)
    REGDEF_BIT(CONV_IN0_BIT_DEPTH ,        2)
    REGDEF_BIT(                   ,        2)
    REGDEF_BIT(CONV_IN0_DATA_FMT  ,        1)
    REGDEF_BIT(                   ,        3)
    REGDEF_BIT(CONV_IN1_BIT_DEPTH ,        2)
    REGDEF_BIT(                   ,        2)
    REGDEF_BIT(CONV_IN1_DATA_FMT  ,        1)
    REGDEF_BIT(                   ,        3)
    REGDEF_BIT(CONV_OUT0_BIT_DEPTH,        3)
    REGDEF_BIT(                   ,        1)
    REGDEF_BIT(CONV_OUT0_DATA_FMT ,        2)
REGDEF_END(CONV_DATA_TYPE_REGISTER0)


/*
    CONV_W_BIT_DEPTH          :    [0x0, 0x3],			bits : 1_0
    CONV_W_DATA_FMT           :    [0x0, 0x1],			bits : 4
    CONV_IN_CH_STRIPE_DATA_FMT:    [0x0, 0x3],			bits : 9_8
    CONV_BIAS_DATA_FMT        :    [0x0, 0x1],			bits : 12
    CONV_DW_IN_FMT            :    [0x0, 0x1],			bits : 16
    CONV_DW_W_FMT             :    [0x0, 0x1],			bits : 20
*/
#define CONV_DATA_TYPE_REGISTER1_OFS 0x0200
REGDEF_BEGIN(CONV_DATA_TYPE_REGISTER1)
    REGDEF_BIT(CONV_W_BIT_DEPTH          ,        2)
    REGDEF_BIT(                          ,        2)
    REGDEF_BIT(CONV_W_DATA_FMT           ,        1)
    REGDEF_BIT(                          ,        3)
    REGDEF_BIT(CONV_IN_CH_STRIPE_DATA_FMT,        2)
    REGDEF_BIT(                          ,        2)
    REGDEF_BIT(CONV_BIAS_DATA_FMT        ,        1)
    REGDEF_BIT(                          ,        3)
    REGDEF_BIT(CONV_DW_IN_FMT            ,        1)
    REGDEF_BIT(                          ,        3)
    REGDEF_BIT(CONV_DW_W_FMT             ,        1)
REGDEF_END(CONV_DATA_TYPE_REGISTER1)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED31_OFS 0x0204
REGDEF_BEGIN(RESERVED31)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED31)


/*
    CONV_IN0_C0_MODE   :    [0x0, 0x1],			bits : 0
    CONV_OUT0_C0_MODE  :    [0x0, 0x1],			bits : 4
    CONV_W_C0_MODE     :    [0x0, 0x3],			bits : 9_8
    CONV_IN0_NC_LAYOUT :    [0x0, 0x1],			bits : 12
    CONV_OUT0_NC_LAYOUT:    [0x0, 0x1],			bits : 16
*/
#define CONV_DATA_LAYOUT_REGISTER0_OFS 0x0208
REGDEF_BEGIN(CONV_DATA_LAYOUT_REGISTER0)
    REGDEF_BIT(CONV_IN0_C0_MODE   ,        1)
    REGDEF_BIT(                   ,        3)
    REGDEF_BIT(CONV_OUT0_C0_MODE  ,        1)
    REGDEF_BIT(                   ,        3)
    REGDEF_BIT(CONV_W_C0_MODE     ,        2)
    REGDEF_BIT(                   ,        2)
    REGDEF_BIT(CONV_IN0_NC_LAYOUT ,        1)
    REGDEF_BIT(                   ,        3)
    REGDEF_BIT(CONV_OUT0_NC_LAYOUT,        1)
REGDEF_END(CONV_DATA_LAYOUT_REGISTER0)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED32_OFS 0x020c
REGDEF_BEGIN(RESERVED32)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED32)


/*
    CONV_CONV_KERNEL_W:    [0x0, 0xff],			bits : 7_0
    CONV_CONV_KERNEL_H:    [0x0, 0xff],			bits : 15_8
    CONV_CONV_DILATE_W:    [0x0, 0xff],			bits : 23_16
    CONV_CONV_DILATE_H:    [0x0, 0xff],			bits : 31_24
*/
#define CONV_CONVOLUTION_REGISTER0_OFS 0x0210
REGDEF_BEGIN(CONV_CONVOLUTION_REGISTER0)
    REGDEF_BIT(CONV_CONV_KERNEL_W,        8)
    REGDEF_BIT(CONV_CONV_KERNEL_H,        8)
    REGDEF_BIT(CONV_CONV_DILATE_W,        8)
    REGDEF_BIT(CONV_CONV_DILATE_H,        8)
REGDEF_END(CONV_CONVOLUTION_REGISTER0)


/*
    CONV_CONV_STRIDE_W   :    [0x0, 0xff],			bits : 7_0
    CONV_CONV_STRIDE_H   :    [0x0, 0xff],			bits : 15_8
    CONV_CONV_OUT_CHANNEL:    [0x0, 0xffff],			bits : 31_16
*/
#define CONV_CONVOLUTION_REGISTER1_OFS 0x0214
REGDEF_BEGIN(CONV_CONVOLUTION_REGISTER1)
    REGDEF_BIT(CONV_CONV_STRIDE_W   ,        8)
    REGDEF_BIT(CONV_CONV_STRIDE_H   ,        8)
    REGDEF_BIT(CONV_CONV_OUT_CHANNEL,        16)
REGDEF_END(CONV_CONVOLUTION_REGISTER1)


/*
    CONV_CONV_KERNEL_D:    [0x0, 0x7f],			bits : 6_0
    CONV_CONV_DILATE_D:    [0x0, 0x7f],			bits : 14_8
    CONV_CONV_STRIDE_D:    [0x0, 0x7f],			bits : 22_16
*/
#define CONV_CONVOLUTION_REGISTER2_OFS 0x0218
REGDEF_BEGIN(CONV_CONVOLUTION_REGISTER2)
    REGDEF_BIT(CONV_CONV_KERNEL_D,        7)
    REGDEF_BIT(                  ,        1)
    REGDEF_BIT(CONV_CONV_DILATE_D,        7)
    REGDEF_BIT(                  ,        1)
    REGDEF_BIT(CONV_CONV_STRIDE_D,        7)
REGDEF_END(CONV_CONVOLUTION_REGISTER2)


/*
    CONV_CONV_TOP_PAD_NUM   :    [0x0, 0xff],			bits : 7_0
    CONV_CONV_BOTTOM_PAD_NUM:    [0x0, 0xff],			bits : 15_8
    CONV_CONV_LEFT_PAD_NUM  :    [0x0, 0xff],			bits : 23_16
    CONV_CONV_RIGHT_PAD_NUM :    [0x0, 0xff],			bits : 31_24
*/
#define CONV_CONVOLUTION_REGISTER3_OFS 0x021c
REGDEF_BEGIN(CONV_CONVOLUTION_REGISTER3)
    REGDEF_BIT(CONV_CONV_TOP_PAD_NUM   ,        8)
    REGDEF_BIT(CONV_CONV_BOTTOM_PAD_NUM,        8)
    REGDEF_BIT(CONV_CONV_LEFT_PAD_NUM  ,        8)
    REGDEF_BIT(CONV_CONV_RIGHT_PAD_NUM ,        8)
REGDEF_END(CONV_CONVOLUTION_REGISTER3)


/*
    CONV_CONV_PAD_VAL_LSB  :    [0x0, 0xffff],			bits : 15_0
    CONV_CONV_FRONT_PAD_NUM:    [0x0, 0xff],			bits : 23_16
    CONV_CONV_BACK_PAD_NUM :    [0x0, 0xff],			bits : 31_24
*/
#define CONV_CONVOLUTION_REGISTER4_OFS 0x0220
REGDEF_BEGIN(CONV_CONVOLUTION_REGISTER4)
    REGDEF_BIT(CONV_CONV_PAD_VAL_LSB  ,        16)
    REGDEF_BIT(CONV_CONV_FRONT_PAD_NUM,        8)
    REGDEF_BIT(CONV_CONV_BACK_PAD_NUM ,        8)
REGDEF_END(CONV_CONVOLUTION_REGISTER4)


/*
    CONV_CONV_MAC_PARALLELISM:    [0x0, 0x7],			bits : 2_0
    CONV_CONV_MAC_MODE       :    [0x0, 0x1],			bits : 4
    CONV_CONV_ACC_PRECISION  :    [0x0, 0x1],			bits : 8
    CONV_CONV_ACC_SHIFT      :    [0x0, 0x1f],			bits : 16_12
    CONV_CONV_ACC_OUT_SHIFT  :    [0x0, 0x3f],			bits : 25_20
*/
#define CONV_CONVOLUTION_REGISTER5_OFS 0x0224
REGDEF_BEGIN(CONV_CONVOLUTION_REGISTER5)
    REGDEF_BIT(CONV_CONV_MAC_PARALLELISM,        3)
    REGDEF_BIT(                         ,        1)
    REGDEF_BIT(CONV_CONV_MAC_MODE       ,        1)
    REGDEF_BIT(                         ,        3)
    REGDEF_BIT(CONV_CONV_ACC_PRECISION  ,        1)
    REGDEF_BIT(                         ,        3)
    REGDEF_BIT(CONV_CONV_ACC_SHIFT      ,        5)
    REGDEF_BIT(                         ,        3)
    REGDEF_BIT(CONV_CONV_ACC_OUT_SHIFT  ,        6)
REGDEF_END(CONV_CONVOLUTION_REGISTER5)


/*
    CONV_CONV_PAD_VAL_MSB:    [0x0, 0x1],			bits : 0
*/
#define CONV_CONVOLUTION_REGISTER6_OFS 0x0228
REGDEF_BEGIN(CONV_CONVOLUTION_REGISTER6)
    REGDEF_BIT(CONV_CONV_PAD_VAL_MSB,        1)
REGDEF_END(CONV_CONVOLUTION_REGISTER6)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CONV_CONVOLUTION_REGISTER7_OFS 0x022c
REGDEF_BEGIN(CONV_CONVOLUTION_REGISTER7)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(CONV_CONVOLUTION_REGISTER7)


/*
    CONV_BIAS_SHIFT        :    [0x0, 0x1f],			bits : 4_0
    CONV_BIAS_OUT_SHIFT_DIR:    [0x0, 0x1],			bits : 8
    CONV_BIAS_OUT_SHIFT    :    [0x0, 0x3f],			bits : 17_12
*/
#define CONV_BIAS_REGISTER0_OFS 0x0230
REGDEF_BEGIN(CONV_BIAS_REGISTER0)
    REGDEF_BIT(CONV_BIAS_SHIFT        ,        5)
    REGDEF_BIT(                       ,        3)
    REGDEF_BIT(CONV_BIAS_OUT_SHIFT_DIR,        1)
    REGDEF_BIT(                       ,        3)
    REGDEF_BIT(CONV_BIAS_OUT_SHIFT    ,        6)
REGDEF_END(CONV_BIAS_REGISTER0)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED33_OFS 0x0234
REGDEF_BEGIN(RESERVED33)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED33)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED34_OFS 0x0238
REGDEF_BEGIN(RESERVED34)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED34)


/*
    CONV_BN_MEAN_SHIFT:    [0x0, 0x1f],			bits : 4_0
    CONV_BN_BETA_SHIFT:    [0x0, 0x3f],			bits : 21_16
    CONV_BN_OUT_SHIFT :    [0x0, 0x3f],			bits : 29_24
*/
#define CONV_BN_REGISTER0_OFS 0x023c
REGDEF_BEGIN(CONV_BN_REGISTER0)
    REGDEF_BIT(CONV_BN_MEAN_SHIFT,        5)
    REGDEF_BIT(                  ,        11)
    REGDEF_BIT(CONV_BN_BETA_SHIFT,        6)
    REGDEF_BIT(                  ,        2)
    REGDEF_BIT(CONV_BN_OUT_SHIFT ,        6)
REGDEF_END(CONV_BN_REGISTER0)


/*
    CONV_BN_OUT_DIR:    [0x0, 0x1],			bits : 1
*/
#define CONV_BN_REGISTER1_OFS 0x0240
REGDEF_BEGIN(CONV_BN_REGISTER1)
    REGDEF_BIT(               ,        1)
    REGDEF_BIT(CONV_BN_OUT_DIR,        1)
REGDEF_END(CONV_BN_REGISTER1)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED35_OFS 0x0244
REGDEF_BEGIN(RESERVED35)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED35)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED36_OFS 0x0248
REGDEF_BEGIN(RESERVED36)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED36)


/*
    CONV_ELT_COEFF0    :    [0x0, 0xffff],			bits : 15_0
    CONV_ELT_SHIFT0    :    [0x0, 0x3f],			bits : 25_20
    CONV_ELT_SHIFT0_DIR:    [0x0, 0x1],			bits : 31
*/
#define CONV_ELTWISE_REGISTER0_OFS 0x024c
REGDEF_BEGIN(CONV_ELTWISE_REGISTER0)
    REGDEF_BIT(CONV_ELT_COEFF0    ,        16)
    REGDEF_BIT(                   ,        4)
    REGDEF_BIT(CONV_ELT_SHIFT0    ,        6)
    REGDEF_BIT(                   ,        5)
    REGDEF_BIT(CONV_ELT_SHIFT0_DIR,        1)
REGDEF_END(CONV_ELTWISE_REGISTER0)


/*
    CONV_ELT_COEFF1    :    [0x0, 0xffff],			bits : 15_0
    CONV_ELT_SHIFT1    :    [0x0, 0x3f],			bits : 25_20
    CONV_ELT_SHIFT1_DIR:    [0x0, 0x1],			bits : 31
*/
#define CONV_ELTWISE_REGISTER1_OFS 0x0250
REGDEF_BEGIN(CONV_ELTWISE_REGISTER1)
    REGDEF_BIT(CONV_ELT_COEFF1    ,        16)
    REGDEF_BIT(                   ,        4)
    REGDEF_BIT(CONV_ELT_SHIFT1    ,        6)
    REGDEF_BIT(                   ,        5)
    REGDEF_BIT(CONV_ELT_SHIFT1_DIR,        1)
REGDEF_END(CONV_ELTWISE_REGISTER1)


/*
    CONV_ELT_OUT_SHIFT    :    [0x0, 0x3f],			bits : 5_0
    CONV_ELT_OUT_SHIFT_DIR:    [0x0, 0x1],			bits : 8
*/
#define CONV_ELTWISE_REGISTER2_OFS 0x0254
REGDEF_BEGIN(CONV_ELTWISE_REGISTER2)
    REGDEF_BIT(CONV_ELT_OUT_SHIFT    ,        6)
    REGDEF_BIT(                      ,        2)
    REGDEF_BIT(CONV_ELT_OUT_SHIFT_DIR,        1)
REGDEF_END(CONV_ELTWISE_REGISTER2)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED37_OFS 0x0258
REGDEF_BEGIN(RESERVED37)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED37)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED38_OFS 0x025c
REGDEF_BEGIN(RESERVED38)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED38)


/*
    CONV_ACT_CONDI0            :    [0x0, 0x1],			bits : 0
    CONV_ACT_CONDI1            :    [0x0, 0x1],			bits : 1
    CONV_ACT_STATE0_EN         :    [0x0, 0x1],			bits : 4
    CONV_ACT_STATE1_EN         :    [0x0, 0x1],			bits : 5
    CONV_ACT_STATE2_EN         :    [0x0, 0x1],			bits : 6
    CONV_ACT_LUT_EN            :    [0x0, 0x1],			bits : 8
    CONV_ACT_LUT_SYMMETRIC_MODE:    [0x0, 0x1],			bits : 12
    CONV_ACT_LUT_FMT           :    [0x0, 0x1],			bits : 16
*/
#define CONV_ACT_REGISTER0_OFS 0x0260
REGDEF_BEGIN(CONV_ACT_REGISTER0)
    REGDEF_BIT(CONV_ACT_CONDI0            ,        1)
    REGDEF_BIT(CONV_ACT_CONDI1            ,        1)
    REGDEF_BIT(                           ,        2)
    REGDEF_BIT(CONV_ACT_STATE0_EN         ,        1)
    REGDEF_BIT(CONV_ACT_STATE1_EN         ,        1)
    REGDEF_BIT(CONV_ACT_STATE2_EN         ,        1)
    REGDEF_BIT(                           ,        1)
    REGDEF_BIT(CONV_ACT_LUT_EN            ,        1)
    REGDEF_BIT(                           ,        3)
    REGDEF_BIT(CONV_ACT_LUT_SYMMETRIC_MODE,        1)
    REGDEF_BIT(                           ,        3)
    REGDEF_BIT(CONV_ACT_LUT_FMT           ,        1)
REGDEF_END(CONV_ACT_REGISTER0)


/*
    CONV_ACT_CONDI_UB:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CONV_ACT_REGISTER1_OFS 0x0264
REGDEF_BEGIN(CONV_ACT_REGISTER1)
    REGDEF_BIT(CONV_ACT_CONDI_UB,        32)
REGDEF_END(CONV_ACT_REGISTER1)


/*
    CONV_ACT_CONDI_LB:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CONV_ACT_REGISTER2_OFS 0x0268
REGDEF_BEGIN(CONV_ACT_REGISTER2)
    REGDEF_BIT(CONV_ACT_CONDI_LB,        32)
REGDEF_END(CONV_ACT_REGISTER2)


/*
    CONV_ACT_A0_MUL  :    [0x0, 0xffff],			bits : 15_0
    CONV_ACT_A0_SHIFT:    [0x0, 0x3f],			bits : 29_24
*/
#define CONV_ACT_REGISTER3_OFS 0x026c
REGDEF_BEGIN(CONV_ACT_REGISTER3)
    REGDEF_BIT(CONV_ACT_A0_MUL  ,        16)
    REGDEF_BIT(                 ,        8)
    REGDEF_BIT(CONV_ACT_A0_SHIFT,        6)
REGDEF_END(CONV_ACT_REGISTER3)


/*
    CONV_ACT_B0      :    [0x0, 0xffff],			bits : 15_0
    CONV_ACT_B0_SHIFT:    [0x0, 0x3f],			bits : 29_24
*/
#define CONV_ACT_REGISTER4_OFS 0x0270
REGDEF_BEGIN(CONV_ACT_REGISTER4)
    REGDEF_BIT(CONV_ACT_B0      ,        16)
    REGDEF_BIT(                 ,        8)
    REGDEF_BIT(CONV_ACT_B0_SHIFT,        6)
REGDEF_END(CONV_ACT_REGISTER4)


/*
    CONV_ACT_A1_MUL  :    [0x0, 0xffff],			bits : 15_0
    CONV_ACT_A1_SHIFT:    [0x0, 0x3f],			bits : 29_24
*/
#define CONV_ACT_REGISTER5_OFS 0x0274
REGDEF_BEGIN(CONV_ACT_REGISTER5)
    REGDEF_BIT(CONV_ACT_A1_MUL  ,        16)
    REGDEF_BIT(                 ,        8)
    REGDEF_BIT(CONV_ACT_A1_SHIFT,        6)
REGDEF_END(CONV_ACT_REGISTER5)


/*
    CONV_ACT_B1      :    [0x0, 0xffff],			bits : 15_0
    CONV_ACT_B1_SHIFT:    [0x0, 0x3f],			bits : 29_24
*/
#define CONV_ACT_REGISTER6_OFS 0x0278
REGDEF_BEGIN(CONV_ACT_REGISTER6)
    REGDEF_BIT(CONV_ACT_B1      ,        16)
    REGDEF_BIT(                 ,        8)
    REGDEF_BIT(CONV_ACT_B1_SHIFT,        6)
REGDEF_END(CONV_ACT_REGISTER6)


/*
    CONV_ACT_C_MUL  :    [0x0, 0xffff],			bits : 15_0
    CONV_ACT_C_SHIFT:    [0x0, 0x3f],			bits : 29_24
*/
#define CONV_ACT_REGISTER7_OFS 0x027c
REGDEF_BEGIN(CONV_ACT_REGISTER7)
    REGDEF_BIT(CONV_ACT_C_MUL  ,        16)
    REGDEF_BIT(                ,        8)
    REGDEF_BIT(CONV_ACT_C_SHIFT,        6)
REGDEF_END(CONV_ACT_REGISTER7)


/*
    CONV_ACT_D      :    [0x0, 0xffff],			bits : 15_0
    CONV_ACT_D_SHIFT:    [0x0, 0x3f],			bits : 29_24
*/
#define CONV_ACT_REGISTER8_OFS 0x0280
REGDEF_BEGIN(CONV_ACT_REGISTER8)
    REGDEF_BIT(CONV_ACT_D      ,        16)
    REGDEF_BIT(                ,        8)
    REGDEF_BIT(CONV_ACT_D_SHIFT,        6)
REGDEF_END(CONV_ACT_REGISTER8)


/*
    CONV_ACT_OUT0_SHIFT    :    [0x0, 0x3f],			bits : 5_0
    CONV_ACT_OUT1_SHIFT    :    [0x0, 0x3f],			bits : 13_8
    CONV_ACT_OUT2_SHIFT    :    [0x0, 0x3f],			bits : 21_16
    CONV_ACT_OUT0_SHIFT_DIR:    [0x0, 0x1],			bits : 24
    CONV_ACT_OUT1_SHIFT_DIR:    [0x0, 0x1],			bits : 25
    CONV_ACT_OUT2_SHIFT_DIR:    [0x0, 0x1],			bits : 26
*/
#define CONV_ACT_REGISTER9_OFS 0x0284
REGDEF_BEGIN(CONV_ACT_REGISTER9)
    REGDEF_BIT(CONV_ACT_OUT0_SHIFT    ,        6)
    REGDEF_BIT(                       ,        2)
    REGDEF_BIT(CONV_ACT_OUT1_SHIFT    ,        6)
    REGDEF_BIT(                       ,        2)
    REGDEF_BIT(CONV_ACT_OUT2_SHIFT    ,        6)
    REGDEF_BIT(                       ,        2)
    REGDEF_BIT(CONV_ACT_OUT0_SHIFT_DIR,        1)
    REGDEF_BIT(CONV_ACT_OUT1_SHIFT_DIR,        1)
    REGDEF_BIT(CONV_ACT_OUT2_SHIFT_DIR,        1)
REGDEF_END(CONV_ACT_REGISTER9)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CONV_ACT_REGISTER10_OFS 0x0288
REGDEF_BEGIN(CONV_ACT_REGISTER10)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(CONV_ACT_REGISTER10)


/*
    ACT_LUT_CHKSUM:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CONV_ACT_REGISTER11_OFS 0x028c
REGDEF_BEGIN(CONV_ACT_REGISTER11)
    REGDEF_BIT(ACT_LUT_CHKSUM,        32)
REGDEF_END(CONV_ACT_REGISTER11)


/*
    CONV_ACT_TOTAL_ENTRY_NUM     :    [0x0, 0x7f],			bits : 6_0
    CONV_ACT_LUT1_START_ENTRY_IDX:    [0x0, 0x7f],			bits : 22_16
*/
#define CONV_ACT_REGISTER12_OFS 0x0290
REGDEF_BEGIN(CONV_ACT_REGISTER12)
    REGDEF_BIT(CONV_ACT_TOTAL_ENTRY_NUM     ,        7)
    REGDEF_BIT(                             ,        9)
    REGDEF_BIT(CONV_ACT_LUT1_START_ENTRY_IDX,        7)
REGDEF_END(CONV_ACT_REGISTER12)


/*
    CONV_ACT_LUT2_START_ENTRY_IDX:    [0x0, 0x7f],			bits : 6_0
*/
#define CONV_ACT_REGISTER13_OFS 0x0294
REGDEF_BEGIN(CONV_ACT_REGISTER13)
    REGDEF_BIT(CONV_ACT_LUT2_START_ENTRY_IDX,        7)
REGDEF_END(CONV_ACT_REGISTER13)


/*
    CONV_ACT_LUT_BOUNDARY0:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CONV_ACT_REGISTER14_OFS 0x0298
REGDEF_BEGIN(CONV_ACT_REGISTER14)
    REGDEF_BIT(CONV_ACT_LUT_BOUNDARY0,        32)
REGDEF_END(CONV_ACT_REGISTER14)


/*
    CONV_ACT_LUT_BOUNDARY1:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CONV_ACT_REGISTER15_OFS 0x029c
REGDEF_BEGIN(CONV_ACT_REGISTER15)
    REGDEF_BIT(CONV_ACT_LUT_BOUNDARY1,        32)
REGDEF_END(CONV_ACT_REGISTER15)


/*
    CONV_ACT_LUT_BOUNDARY2:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CONV_ACT_REGISTER16_OFS 0x02a0
REGDEF_BEGIN(CONV_ACT_REGISTER16)
    REGDEF_BIT(CONV_ACT_LUT_BOUNDARY2,        32)
REGDEF_END(CONV_ACT_REGISTER16)


/*
    CONV_ACT_LUT_BOUNDARY3:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CONV_ACT_REGISTER17_OFS 0x02a4
REGDEF_BEGIN(CONV_ACT_REGISTER17)
    REGDEF_BIT(CONV_ACT_LUT_BOUNDARY3,        32)
REGDEF_END(CONV_ACT_REGISTER17)


/*
    CONV_ACT_LUT_IN_SHIFT    :    [0x0, 0x1f],			bits : 4_0
    CONV_ACT_LUT_IN_SHIFT_DIR:    [0x0, 0x1],			bits : 24
*/
#define CONV_ACT_REGISTER18_OFS 0x02a8
REGDEF_BEGIN(CONV_ACT_REGISTER18)
    REGDEF_BIT(CONV_ACT_LUT_IN_SHIFT    ,        5)
    REGDEF_BIT(                         ,        19)
    REGDEF_BIT(CONV_ACT_LUT_IN_SHIFT_DIR,        1)
REGDEF_END(CONV_ACT_REGISTER18)


/*
    CONV_ACT_LUT0_OUT_SHIFT    :    [0x0, 0x1f],			bits : 4_0
    CONV_ACT_LUT1_OUT_SHIFT    :    [0x0, 0x1f],			bits : 12_8
    CONV_ACT_LUT2_OUT_SHIFT    :    [0x0, 0x1f],			bits : 20_16
    CONV_ACT_LUT0_OUT_SHIFT_DIR:    [0x0, 0x1],			bits : 24
    CONV_ACT_LUT1_OUT_SHIFT_DIR:    [0x0, 0x1],			bits : 25
    CONV_ACT_LUT2_OUT_SHIFT_DIR:    [0x0, 0x1],			bits : 26
*/
#define CONV_ACT_REGISTER19_OFS 0x02ac
REGDEF_BEGIN(CONV_ACT_REGISTER19)
    REGDEF_BIT(CONV_ACT_LUT0_OUT_SHIFT    ,        5)
    REGDEF_BIT(                           ,        3)
    REGDEF_BIT(CONV_ACT_LUT1_OUT_SHIFT    ,        5)
    REGDEF_BIT(                           ,        3)
    REGDEF_BIT(CONV_ACT_LUT2_OUT_SHIFT    ,        5)
    REGDEF_BIT(                           ,        3)
    REGDEF_BIT(CONV_ACT_LUT0_OUT_SHIFT_DIR,        1)
    REGDEF_BIT(CONV_ACT_LUT1_OUT_SHIFT_DIR,        1)
    REGDEF_BIT(CONV_ACT_LUT2_OUT_SHIFT_DIR,        1)
REGDEF_END(CONV_ACT_REGISTER19)


/*
    CONV_ACT_LUT0_GAP_BIT_NUM:    [0x0, 0x1f],			bits : 4_0
    CONV_ACT_LUT1_GAP_BIT_NUM:    [0x0, 0x1f],			bits : 12_8
    CONV_ACT_LUT2_GAP_BIT_NUM:    [0x0, 0x1f],			bits : 20_16
*/
#define CONV_ACT_REGISTER20_OFS 0x02b0
REGDEF_BEGIN(CONV_ACT_REGISTER20)
    REGDEF_BIT(CONV_ACT_LUT0_GAP_BIT_NUM,        5)
    REGDEF_BIT(                         ,        3)
    REGDEF_BIT(CONV_ACT_LUT1_GAP_BIT_NUM,        5)
    REGDEF_BIT(                         ,        3)
    REGDEF_BIT(CONV_ACT_LUT2_GAP_BIT_NUM,        5)
REGDEF_END(CONV_ACT_REGISTER20)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED39_OFS 0x02b4
REGDEF_BEGIN(RESERVED39)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED39)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED40_OFS 0x02b8
REGDEF_BEGIN(RESERVED40)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED40)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED41_OFS 0x02bc
REGDEF_BEGIN(RESERVED41)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED41)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED42_OFS 0x02c0
REGDEF_BEGIN(RESERVED42)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED42)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED43_OFS 0x02c4
REGDEF_BEGIN(RESERVED43)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED43)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED44_OFS 0x02c8
REGDEF_BEGIN(RESERVED44)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED44)


/*
    CONV_POOL_OP               :    [0x0, 0x1],			bits : 0
    CONV_POOL_KERNEL_W         :    [0x0, 0x3],			bits : 5_4
    CONV_POOL_KERNEL_H         :    [0x0, 0x3],			bits : 9_8
    CONV_POOL_STRIDE_W         :    [0x0, 0x3],			bits : 13_12
    CONV_POOL_STRIDE_H         :    [0x0, 0x3],			bits : 17_16
    CONV_POOL_ROUND_MODE       :    [0x0, 0x1],			bits : 20
    CONV_POOL_COUNT_INCLUDE_PAD:    [0x0, 0x1],			bits : 21
    CONV_POOL_DIVISOR_OVERRIDE :    [0x0, 0x1],			bits : 22
*/
#define CONV_POOL_REGISTER0_OFS 0x02cc
REGDEF_BEGIN(CONV_POOL_REGISTER0)
    REGDEF_BIT(CONV_POOL_OP               ,        1)
    REGDEF_BIT(                           ,        3)
    REGDEF_BIT(CONV_POOL_KERNEL_W         ,        2)
    REGDEF_BIT(                           ,        2)
    REGDEF_BIT(CONV_POOL_KERNEL_H         ,        2)
    REGDEF_BIT(                           ,        2)
    REGDEF_BIT(CONV_POOL_STRIDE_W         ,        2)
    REGDEF_BIT(                           ,        2)
    REGDEF_BIT(CONV_POOL_STRIDE_H         ,        2)
    REGDEF_BIT(                           ,        2)
    REGDEF_BIT(CONV_POOL_ROUND_MODE       ,        1)
    REGDEF_BIT(CONV_POOL_COUNT_INCLUDE_PAD,        1)
    REGDEF_BIT(CONV_POOL_DIVISOR_OVERRIDE ,        1)
REGDEF_END(CONV_POOL_REGISTER0)


/*
    CONV_POOL_TOP_PAD_NUM   :    [0x0, 0x1],			bits : 0
    CONV_POOL_LEFT_PAD_NUM  :    [0x0, 0x1],			bits : 4
    CONV_POOL_BOTTOM_PAD_NUM:    [0x0, 0x1],			bits : 8
    CONV_POOL_RIGHT_PAD_NUM :    [0x0, 0x1],			bits : 12
    CONV_POOL_OUT_SHIFT_DIR :    [0x0, 0x1],			bits : 17
*/
#define CONV_POOL_REGISTER1_OFS 0x02d0
REGDEF_BEGIN(CONV_POOL_REGISTER1)
    REGDEF_BIT(CONV_POOL_TOP_PAD_NUM   ,        1)
    REGDEF_BIT(                        ,        3)
    REGDEF_BIT(CONV_POOL_LEFT_PAD_NUM  ,        1)
    REGDEF_BIT(                        ,        3)
    REGDEF_BIT(CONV_POOL_BOTTOM_PAD_NUM,        1)
    REGDEF_BIT(                        ,        3)
    REGDEF_BIT(CONV_POOL_RIGHT_PAD_NUM ,        1)
    REGDEF_BIT(                        ,        4)
    REGDEF_BIT(CONV_POOL_OUT_SHIFT_DIR ,        1)
REGDEF_END(CONV_POOL_REGISTER1)


/*
    CONV_POOL_AVG_MUL:    [0x0, 0xffff],			bits : 15_0
*/
#define CONV_POOL_REGISTER2_OFS 0x02d4
REGDEF_BEGIN(CONV_POOL_REGISTER2)
    REGDEF_BIT(CONV_POOL_AVG_MUL,        16)
REGDEF_END(CONV_POOL_REGISTER2)


/*
    CONV_POOL_OUT_SHIFT:    [0x0, 0x3f],			bits : 13_8
*/
#define CONV_POOL_REGISTER3_OFS 0x02d8
REGDEF_BEGIN(CONV_POOL_REGISTER3)
    REGDEF_BIT(                   ,        8)
    REGDEF_BIT(CONV_POOL_OUT_SHIFT,        6)
REGDEF_END(CONV_POOL_REGISTER3)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED45_OFS 0x02dc
REGDEF_BEGIN(RESERVED45)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED45)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED46_OFS 0x02e0
REGDEF_BEGIN(RESERVED46)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED46)


/*
    CONV_DW_KERNEL_W:    [0x0, 0x3],			bits : 1_0
    CONV_DW_KERNEL_H:    [0x0, 0x3],			bits : 5_4
    CONV_DW_STRIDE_W:    [0x0, 0x3],			bits : 9_8
    CONV_DW_STRIDE_H:    [0x0, 0x3],			bits : 13_12
*/
#define CONV_DW_CONV_REGISTER0_OFS 0x02e4
REGDEF_BEGIN(CONV_DW_CONV_REGISTER0)
    REGDEF_BIT(CONV_DW_KERNEL_W,        2)
    REGDEF_BIT(                ,        2)
    REGDEF_BIT(CONV_DW_KERNEL_H,        2)
    REGDEF_BIT(                ,        2)
    REGDEF_BIT(CONV_DW_STRIDE_W,        2)
    REGDEF_BIT(                ,        2)
    REGDEF_BIT(CONV_DW_STRIDE_H,        2)
REGDEF_END(CONV_DW_CONV_REGISTER0)


/*
    CONV_DW_TOP_PAD_NUM   :    [0x0, 0x1],			bits : 0
    CONV_DW_BOTTOM_PAD_NUM:    [0x0, 0x1],			bits : 8
    CONV_DW_LEFT_PAD_NUM  :    [0x0, 0x1],			bits : 16
    CONV_DW_RIGHT_PAD_NUM :    [0x0, 0x1],			bits : 24
*/
#define CONV_DW_CONV_REGISTER1_OFS 0x02e8
REGDEF_BEGIN(CONV_DW_CONV_REGISTER1)
    REGDEF_BIT(CONV_DW_TOP_PAD_NUM   ,        1)
    REGDEF_BIT(                      ,        7)
    REGDEF_BIT(CONV_DW_BOTTOM_PAD_NUM,        1)
    REGDEF_BIT(                      ,        7)
    REGDEF_BIT(CONV_DW_LEFT_PAD_NUM  ,        1)
    REGDEF_BIT(                      ,        7)
    REGDEF_BIT(CONV_DW_RIGHT_PAD_NUM ,        1)
REGDEF_END(CONV_DW_CONV_REGISTER1)


/*
    CONV_DW_ACC_OUT_SHIFT    :    [0x0, 0x1f],			bits : 12_8
    CONV_DW_ACC_OUT_SHIFT_DIR:    [0x0, 0x1],			bits : 28
*/
#define CONV_DW_CONV_REGISTER2_OFS 0x02ec
REGDEF_BEGIN(CONV_DW_CONV_REGISTER2)
    REGDEF_BIT(                         ,        8)
    REGDEF_BIT(CONV_DW_ACC_OUT_SHIFT    ,        5)
    REGDEF_BIT(                         ,        15)
    REGDEF_BIT(CONV_DW_ACC_OUT_SHIFT_DIR,        1)
REGDEF_END(CONV_DW_CONV_REGISTER2)


/*
    CONV_DW_BIAS_SHIFT        :    [0x0, 0x1f],			bits : 4_0
    CONV_DW_BIAS_OUT_SHIFT_DIR:    [0x0, 0x1],			bits : 8
    CONV_DW_BIAS_OUT_SHIFT    :    [0x0, 0x3f],			bits : 17_12
*/
#define CONV_DW_CONV_REGISTER3_OFS 0x02f0
REGDEF_BEGIN(CONV_DW_CONV_REGISTER3)
    REGDEF_BIT(CONV_DW_BIAS_SHIFT        ,        5)
    REGDEF_BIT(                          ,        3)
    REGDEF_BIT(CONV_DW_BIAS_OUT_SHIFT_DIR,        1)
    REGDEF_BIT(                          ,        3)
    REGDEF_BIT(CONV_DW_BIAS_OUT_SHIFT    ,        6)
REGDEF_END(CONV_DW_CONV_REGISTER3)


/*
    CONV_DW_RELU_MUL      :    [0x0, 0xffff],			bits : 15_0
    CONV_DW_RELU_SHIFT    :    [0x0, 0x3f],			bits : 29_24
    CONV_DW_RELU_SHIFT_DIR:    [0x0, 0x1],			bits : 31
*/
#define CONV_DW_CONV_REGISTER4_OFS 0x02f4
REGDEF_BEGIN(CONV_DW_CONV_REGISTER4)
    REGDEF_BIT(CONV_DW_RELU_MUL      ,        16)
    REGDEF_BIT(                      ,        8)
    REGDEF_BIT(CONV_DW_RELU_SHIFT    ,        6)
    REGDEF_BIT(                      ,        1)
    REGDEF_BIT(CONV_DW_RELU_SHIFT_DIR,        1)
REGDEF_END(CONV_DW_CONV_REGISTER4)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED47_OFS 0x02f8
REGDEF_BEGIN(RESERVED47)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED47)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED48_OFS 0x02fc
REGDEF_BEGIN(RESERVED48)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED48)


/*
    CONV_QUAN_IN0_OFS:    [0x0, 0xffff],			bits : 15_0
    CONV_QUAN_IN1_OFS:    [0x0, 0xffff],			bits : 31_16
*/
#define QUANTIZATION_OFFSET_REGISTER0_OFS 0x0300
REGDEF_BEGIN(QUANTIZATION_OFFSET_REGISTER0)
    REGDEF_BIT(CONV_QUAN_IN0_OFS,        16)
    REGDEF_BIT(CONV_QUAN_IN1_OFS,        16)
REGDEF_END(QUANTIZATION_OFFSET_REGISTER0)


/*
    CONV_QUAN_OUT0_OFS:    [0x0, 0xffff],			bits : 15_0
    CONV_QUAN_W_OFS   :    [0x0, 0xffff],			bits : 31_16
*/
#define QUANTIZATION_OFFSET_REGISTER1_OFS 0x0304
REGDEF_BEGIN(QUANTIZATION_OFFSET_REGISTER1)
    REGDEF_BIT(CONV_QUAN_OUT0_OFS,        16)
    REGDEF_BIT(CONV_QUAN_W_OFS   ,        16)
REGDEF_END(QUANTIZATION_OFFSET_REGISTER1)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED49_OFS 0x0308
REGDEF_BEGIN(RESERVED49)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED49)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED50_OFS 0x030c
REGDEF_BEGIN(RESERVED50)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED50)


/*
    CONV_QUAN_CONVOUT_SHIFT_DIR:    [0x0, 0x1],			bits : 0
    CONV_QUAN_CONVOUT_SHIFT    :    [0x0, 0x3f],			bits : 9_4
    CONV_QUAN_CONVOUT_SCALE    :    [0x0, 0xffff],			bits : 27_12
*/
#define QUANTIZATION_SCALE_SHIFT_REGISTER0_OFS 0x0310
REGDEF_BEGIN(QUANTIZATION_SCALE_SHIFT_REGISTER0)
    REGDEF_BIT(CONV_QUAN_CONVOUT_SHIFT_DIR,        1)
    REGDEF_BIT(                           ,        3)
    REGDEF_BIT(CONV_QUAN_CONVOUT_SHIFT    ,        6)
    REGDEF_BIT(                           ,        2)
    REGDEF_BIT(CONV_QUAN_CONVOUT_SCALE    ,        16)
REGDEF_END(QUANTIZATION_SCALE_SHIFT_REGISTER0)


/*
    CONV_QUAN_OUT0_SHIFT_DIR:    [0x0, 0x1],			bits : 0
    CONV_QUAN_OUT0_SHIFT    :    [0x0, 0x3f],			bits : 9_4
    CONV_QUAN_OUT0_SCALE    :    [0x0, 0xffff],			bits : 27_12
*/
#define QUANTIZATION_SCALE_SHIFT_REGISTER1_OFS 0x0314
REGDEF_BEGIN(QUANTIZATION_SCALE_SHIFT_REGISTER1)
    REGDEF_BIT(CONV_QUAN_OUT0_SHIFT_DIR,        1)
    REGDEF_BIT(                        ,        3)
    REGDEF_BIT(CONV_QUAN_OUT0_SHIFT    ,        6)
    REGDEF_BIT(                        ,        2)
    REGDEF_BIT(CONV_QUAN_OUT0_SCALE    ,        16)
REGDEF_END(QUANTIZATION_SCALE_SHIFT_REGISTER1)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED51_OFS 0x0318
REGDEF_BEGIN(RESERVED51)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED51)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED52_OFS 0x031c
REGDEF_BEGIN(RESERVED52)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED52)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED53_OFS 0x0320
REGDEF_BEGIN(RESERVED53)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED53)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED54_OFS 0x0324
REGDEF_BEGIN(RESERVED54)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED54)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED55_OFS 0x0328
REGDEF_BEGIN(RESERVED55)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED55)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED56_OFS 0x032c
REGDEF_BEGIN(RESERVED56)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED56)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED57_OFS 0x0330
REGDEF_BEGIN(RESERVED57)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED57)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED58_OFS 0x0334
REGDEF_BEGIN(RESERVED58)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED58)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED59_OFS 0x0338
REGDEF_BEGIN(RESERVED59)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED59)


/*
    CONV_WCD_BITSTREAM_LENGTH_0_OCH0:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CONV_WCD_REGISTER0_OFS 0x033c
REGDEF_BEGIN(CONV_WCD_REGISTER0)
    REGDEF_BIT(CONV_WCD_BITSTREAM_LENGTH_0_OCH0,        32)
REGDEF_END(CONV_WCD_REGISTER0)


/*
    CONV_WCD_BITSTREAM_LENGTH_1_OCH0:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CONV_WCD_REGISTER1_OFS 0x0340
REGDEF_BEGIN(CONV_WCD_REGISTER1)
    REGDEF_BIT(CONV_WCD_BITSTREAM_LENGTH_1_OCH0,        32)
REGDEF_END(CONV_WCD_REGISTER1)


/*
    CONV_WCD_BITSTREAM_LENGTH_0_OCH1:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CONV_WCD_REGISTER2_OFS 0x0344
REGDEF_BEGIN(CONV_WCD_REGISTER2)
    REGDEF_BIT(CONV_WCD_BITSTREAM_LENGTH_0_OCH1,        32)
REGDEF_END(CONV_WCD_REGISTER2)


/*
    CONV_WCD_BITSTREAM_LENGTH_1_OCH1:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CONV_WCD_REGISTER3_OFS 0x0348
REGDEF_BEGIN(CONV_WCD_REGISTER3)
    REGDEF_BIT(CONV_WCD_BITSTREAM_LENGTH_1_OCH1,        32)
REGDEF_END(CONV_WCD_REGISTER3)


/*
    CONV_WCD_BITSTREAM_LENGTH_0_OCH2:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CONV_WCD_REGISTER4_OFS 0x034c
REGDEF_BEGIN(CONV_WCD_REGISTER4)
    REGDEF_BIT(CONV_WCD_BITSTREAM_LENGTH_0_OCH2,        32)
REGDEF_END(CONV_WCD_REGISTER4)


/*
    CONV_WCD_BITSTREAM_LENGTH_1_OCH2:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CONV_WCD_REGISTER5_OFS 0x0350
REGDEF_BEGIN(CONV_WCD_REGISTER5)
    REGDEF_BIT(CONV_WCD_BITSTREAM_LENGTH_1_OCH2,        32)
REGDEF_END(CONV_WCD_REGISTER5)


/*
    CONV_WCD_BITSTREAM_LENGTH_0_OCH3:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CONV_WCD_REGISTER6_OFS 0x0354
REGDEF_BEGIN(CONV_WCD_REGISTER6)
    REGDEF_BIT(CONV_WCD_BITSTREAM_LENGTH_0_OCH3,        32)
REGDEF_END(CONV_WCD_REGISTER6)


/*
    CONV_WCD_BITSTREAM_LENGTH_1_OCH3:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CONV_WCD_REGISTER7_OFS 0x0358
REGDEF_BEGIN(CONV_WCD_REGISTER7)
    REGDEF_BIT(CONV_WCD_BITSTREAM_LENGTH_1_OCH3,        32)
REGDEF_END(CONV_WCD_REGISTER7)


/*
    CONV_WCD_VLC_TBL_BASE_0   :    [0x0, 0xffff],			bits : 15_0
    CONV_WCD_VLC_TBL_BIT_LEN_0:    [0x0, 0x1f],			bits : 28_24
*/
#define CONV_WCD_REGISTER8_OFS 0x035c
REGDEF_BEGIN(CONV_WCD_REGISTER8)
    REGDEF_BIT(CONV_WCD_VLC_TBL_BASE_0   ,        16)
    REGDEF_BIT(                          ,        8)
    REGDEF_BIT(CONV_WCD_VLC_TBL_BIT_LEN_0,        5)
REGDEF_END(CONV_WCD_REGISTER8)


/*
    CONV_WCD_VLC_TBL_BASE_1   :    [0x0, 0xffff],			bits : 15_0
    CONV_WCD_VLC_TBL_BIT_LEN_1:    [0x0, 0x1f],			bits : 28_24
*/
#define CONV_WCD_REGISTER9_OFS 0x0360
REGDEF_BEGIN(CONV_WCD_REGISTER9)
    REGDEF_BIT(CONV_WCD_VLC_TBL_BASE_1   ,        16)
    REGDEF_BIT(                          ,        8)
    REGDEF_BIT(CONV_WCD_VLC_TBL_BIT_LEN_1,        5)
REGDEF_END(CONV_WCD_REGISTER9)


/*
    CONV_WCD_VLC_TBL_BASE_2   :    [0x0, 0xffff],			bits : 15_0
    CONV_WCD_VLC_TBL_BIT_LEN_2:    [0x0, 0x1f],			bits : 28_24
*/
#define CONV_WCD_REGISTER10_OFS 0x0364
REGDEF_BEGIN(CONV_WCD_REGISTER10)
    REGDEF_BIT(CONV_WCD_VLC_TBL_BASE_2   ,        16)
    REGDEF_BIT(                          ,        8)
    REGDEF_BIT(CONV_WCD_VLC_TBL_BIT_LEN_2,        5)
REGDEF_END(CONV_WCD_REGISTER10)


/*
    CONV_WCD_VLC_TBL_BASE_3   :    [0x0, 0xffff],			bits : 15_0
    CONV_WCD_VLC_TBL_BIT_LEN_3:    [0x0, 0x1f],			bits : 28_24
*/
#define CONV_WCD_REGISTER11_OFS 0x0368
REGDEF_BEGIN(CONV_WCD_REGISTER11)
    REGDEF_BIT(CONV_WCD_VLC_TBL_BASE_3   ,        16)
    REGDEF_BIT(                          ,        8)
    REGDEF_BIT(CONV_WCD_VLC_TBL_BIT_LEN_3,        5)
REGDEF_END(CONV_WCD_REGISTER11)


/*
    CONV_WCD_VLC_TBL_BASE_4   :    [0x0, 0xffff],			bits : 15_0
    CONV_WCD_VLC_TBL_BIT_LEN_4:    [0x0, 0x1f],			bits : 28_24
*/
#define CONV_WCD_REGISTER12_OFS 0x036c
REGDEF_BEGIN(CONV_WCD_REGISTER12)
    REGDEF_BIT(CONV_WCD_VLC_TBL_BASE_4   ,        16)
    REGDEF_BIT(                          ,        8)
    REGDEF_BIT(CONV_WCD_VLC_TBL_BIT_LEN_4,        5)
REGDEF_END(CONV_WCD_REGISTER12)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED60_OFS 0x0370
REGDEF_BEGIN(RESERVED60)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED60)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED61_OFS 0x0374
REGDEF_BEGIN(RESERVED61)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED61)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED62_OFS 0x0378
REGDEF_BEGIN(RESERVED62)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED62)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED63_OFS 0x037c
REGDEF_BEGIN(RESERVED63)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED63)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED64_OFS 0x0380
REGDEF_BEGIN(RESERVED64)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED64)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED65_OFS 0x0384
REGDEF_BEGIN(RESERVED65)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED65)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED66_OFS 0x0388
REGDEF_BEGIN(RESERVED66)
    REGDEF_BIT(HIDDEN_CONV_DRAM_BW,  32)
REGDEF_END(RESERVED66)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED67_OFS 0x038c
REGDEF_BEGIN(RESERVED67)
    REGDEF_BIT(HIDDEN_CONV_UB_BW,    32)
REGDEF_END(RESERVED67)


/*
    CONV_ENG_CYCLE:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CYCLE_COUNT_REGISTER0_OFS 0x0390
REGDEF_BEGIN(CYCLE_COUNT_REGISTER0)
    REGDEF_BIT(CONV_ENG_CYCLE,        32)
REGDEF_END(CYCLE_COUNT_REGISTER0)


/*
    CONV_LL_CYCLE:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CYCLE_COUNT_REGISTER1_OFS 0x0394
REGDEF_BEGIN(CYCLE_COUNT_REGISTER1)
    REGDEF_BIT(CONV_LL_CYCLE,        32)
REGDEF_END(CYCLE_COUNT_REGISTER1)


/*
    CONV_WAIT_DMA_CYCLE:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CYCLE_COUNT_REGISTER2_OFS 0x0398
REGDEF_BEGIN(CYCLE_COUNT_REGISTER2)
    REGDEF_BIT(CONV_WAIT_DMA_CYCLE,        32)
REGDEF_END(CYCLE_COUNT_REGISTER2)


/*
    CONV_WAIT_UB_CYCLE:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CYCLE_COUNT_REGISTER3_OFS 0x039c
REGDEF_BEGIN(CYCLE_COUNT_REGISTER3)
    REGDEF_BIT(CONV_WAIT_UB_CYCLE,        32)
REGDEF_END(CYCLE_COUNT_REGISTER3)


/*
    CONV_WAIT_DRAM_CYCLE:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CYCLE_COUNT_REGISTER4_OFS 0x03a0
REGDEF_BEGIN(CYCLE_COUNT_REGISTER4)
    REGDEF_BIT(CONV_WAIT_DRAM_CYCLE,        32)
REGDEF_END(CYCLE_COUNT_REGISTER4)


/*
    LL_TABLE_IDX0_3:    [0x0, 0xffffffff],			bits : 31_0
*/
#define LL_FRAME_REGISTER0_OFS 0x03a4
REGDEF_BEGIN(LL_FRAME_REGISTER0)
    REGDEF_BIT(LL_TABLE_IDX0_3,        32)
REGDEF_END(LL_FRAME_REGISTER0)


/*
    LL_TABLE_IDX4_7:    [0x0, 0xffffffff],			bits : 31_0
*/
#define LL_FRAME_REGISTER1_OFS 0x03a8
REGDEF_BEGIN(LL_FRAME_REGISTER1)
    REGDEF_BIT(LL_TABLE_IDX4_7,        32)
REGDEF_END(LL_FRAME_REGISTER1)


/*
    LL_TABLE_IDX8_11:    [0x0, 0xffffffff],			bits : 31_0
*/
#define LL_FRAME_REGISTER2_OFS 0x03ac
REGDEF_BEGIN(LL_FRAME_REGISTER2)
    REGDEF_BIT(LL_TABLE_IDX8_11,        32)
REGDEF_END(LL_FRAME_REGISTER2)


/*
    LL_TABLE_IDX12_15:    [0x0, 0xffffffff],			bits : 31_0
*/
#define LL_FRAME_REGISTER3_OFS 0x03b0
REGDEF_BEGIN(LL_FRAME_REGISTER3)
    REGDEF_BIT(LL_TABLE_IDX12_15,        32)
REGDEF_END(LL_FRAME_REGISTER3)


/*
    DMA_DISABLE:    [0x0, 0x1],			bits : 0
    CONV_IDLE  :    [0x0, 0x1],			bits : 15
*/
#define DMA_DISABLE_REGISTER0_OFS 0x03b4
REGDEF_BEGIN(DMA_DISABLE_REGISTER0)
    REGDEF_BIT(DMA_DISABLE,        1)
    REGDEF_BIT(           ,        14)
    REGDEF_BIT(CONV_IDLE  ,        1)
REGDEF_END(DMA_DISABLE_REGISTER0)


/*
    CHECK_SUM_ENABLE:    [0x0, 0x1],			bits : 0
    CYCLE_COUNT_EN  :    [0x0, 0x1],			bits : 1
    DBG_PORT_SEL    :    [0x0, 0x3],			bits : 5_4
*/
#define DESIGN_DEBUG_REGISTER0_OFS 0x03b8
REGDEF_BEGIN(DESIGN_DEBUG_REGISTER0)
    REGDEF_BIT(CHECK_SUM_ENABLE,        1)
    REGDEF_BIT(CYCLE_COUNT_EN  ,        1)
    REGDEF_BIT(                ,        2)
    REGDEF_BIT(DBG_PORT_SEL    ,        2)
REGDEF_END(DESIGN_DEBUG_REGISTER0)


/*
    DMACH0_BURST_LENGTH:    [0x0, 0x3],			bits : 1_0
    DMACH1_BURST_LENGTH:    [0x0, 0x3],			bits : 5_4
    DMACH2_BURST_LENGTH:    [0x0, 0x3],			bits : 9_8
    DMACH3_BURST_LENGTH:    [0x0, 0x3],			bits : 13_12
    DMACH4_BURST_LENGTH:    [0x0, 0x3],			bits : 17_16
    DMACH5_BURST_LENGTH:    [0x0, 0x3],			bits : 21_20
    DMACH6_BURST_LENGTH:    [0x0, 0x3],			bits : 25_24
    DMACH7_BURST_LENGTH:    [0x0, 0x3],			bits : 29_28
*/
#define DESIGN_DEBUG_REGISTER1_OFS 0x03bc
REGDEF_BEGIN(DESIGN_DEBUG_REGISTER1)
    REGDEF_BIT(DMACH0_BURST_LENGTH,        2)
    REGDEF_BIT(                   ,        2)
    REGDEF_BIT(DMACH1_BURST_LENGTH,        2)
    REGDEF_BIT(                   ,        2)
    REGDEF_BIT(DMACH2_BURST_LENGTH,        2)
    REGDEF_BIT(                   ,        2)
    REGDEF_BIT(DMACH3_BURST_LENGTH,        2)
    REGDEF_BIT(                   ,        2)
    REGDEF_BIT(DMACH4_BURST_LENGTH,        2)
    REGDEF_BIT(                   ,        2)
    REGDEF_BIT(DMACH5_BURST_LENGTH,        2)
    REGDEF_BIT(                   ,        2)
    REGDEF_BIT(DMACH6_BURST_LENGTH,        2)
    REGDEF_BIT(                   ,        2)
    REGDEF_BIT(DMACH7_BURST_LENGTH,        2)
REGDEF_END(DESIGN_DEBUG_REGISTER1)


/*
    DMACH8_BURST_LENGTH :    [0x0, 0x3],			bits : 1_0
    DMACH9_BURST_LENGTH :    [0x0, 0x3],			bits : 5_4
    DMACH13_BURST_LENGTH:    [0x0, 0x3],			bits : 21_20
    DMACH14_BURST_LENGTH:    [0x0, 0x3],			bits : 25_24
    DMACH15_BURST_LENGTH:    [0x0, 0x3],			bits : 29_28
*/
#define DESIGN_DEBUG_REGISTER2_OFS 0x03c0
REGDEF_BEGIN(DESIGN_DEBUG_REGISTER2)
    REGDEF_BIT(DMACH8_BURST_LENGTH ,        2)
    REGDEF_BIT(                    ,        2)
    REGDEF_BIT(DMACH9_BURST_LENGTH ,        2)
    REGDEF_BIT(                    ,        14)
    REGDEF_BIT(DMACH13_BURST_LENGTH,        2)
    REGDEF_BIT(                    ,        2)
    REGDEF_BIT(DMACH14_BURST_LENGTH,        2)
    REGDEF_BIT(                    ,        2)
    REGDEF_BIT(DMACH15_BURST_LENGTH,        2)
REGDEF_END(DESIGN_DEBUG_REGISTER2)


/*
    DMACH16_BURST_LENGTH:    [0x0, 0x3],			bits : 1_0
    DMACH17_BURST_LENGTH:    [0x0, 0x3],			bits : 5_4
*/
#define DESIGN_DEBUG_REGISTER3_OFS 0x03c4
REGDEF_BEGIN(DESIGN_DEBUG_REGISTER3)
    REGDEF_BIT(DMACH16_BURST_LENGTH,        2)
    REGDEF_BIT(                    ,        2)
    REGDEF_BIT(DMACH17_BURST_LENGTH,        2)
REGDEF_END(DESIGN_DEBUG_REGISTER3)


/*
    DMACH0_MAX_OUTSTANDING_NUM:    [0x0, 0xff],			bits : 7_0
    DMACH1_MAX_OUTSTANDING_NUM:    [0x0, 0xff],			bits : 15_8
*/
#define DESIGN_DEBUG_REGISTER4_OFS 0x03c8
REGDEF_BEGIN(DESIGN_DEBUG_REGISTER4)
    REGDEF_BIT(DMACH0_MAX_OUTSTANDING_NUM,        8)
    REGDEF_BIT(DMACH1_MAX_OUTSTANDING_NUM,        8)
REGDEF_END(DESIGN_DEBUG_REGISTER4)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED68_OFS 0x03cc
REGDEF_BEGIN(RESERVED68)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED68)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED69_OFS 0x03d0
REGDEF_BEGIN(RESERVED69)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED69)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED70_OFS 0x03d4
REGDEF_BEGIN(RESERVED70)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED70)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED71_OFS 0x03d8
REGDEF_BEGIN(RESERVED71)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED71)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED72_OFS 0x03dc
REGDEF_BEGIN(RESERVED72)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED72)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED73_OFS 0x03e0
REGDEF_BEGIN(RESERVED73)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED73)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED74_OFS 0x03e4
REGDEF_BEGIN(RESERVED74)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED74)


/*
    DMACH0_EN:    [0x0, 0x1],			bits : 0
    DMACH1_EN:    [0x0, 0x1],			bits : 4
    DMACH2_EN:    [0x0, 0x1],			bits : 8
    DMACH3_EN:    [0x0, 0x1],			bits : 12
    DMACH4_EN:    [0x0, 0x1],			bits : 16
    DMACH5_EN:    [0x0, 0x1],			bits : 20
    DMACH6_EN:    [0x0, 0x1],			bits : 24
    DMACH7_EN:    [0x0, 0x1],			bits : 28
*/
#define AXI_REGISTER0_OFS 0x03e8
REGDEF_BEGIN(AXI_REGISTER0)
    REGDEF_BIT(DMACH0_EN,        1)
    REGDEF_BIT(         ,        3)
    REGDEF_BIT(DMACH1_EN,        1)
    REGDEF_BIT(         ,        3)
    REGDEF_BIT(DMACH2_EN,        1)
    REGDEF_BIT(         ,        3)
    REGDEF_BIT(DMACH3_EN,        1)
    REGDEF_BIT(         ,        3)
    REGDEF_BIT(DMACH4_EN,        1)
    REGDEF_BIT(         ,        3)
    REGDEF_BIT(DMACH5_EN,        1)
    REGDEF_BIT(         ,        3)
    REGDEF_BIT(DMACH6_EN,        1)
    REGDEF_BIT(         ,        3)
    REGDEF_BIT(DMACH7_EN,        1)
REGDEF_END(AXI_REGISTER0)


/*
    DMACH8_EN :    [0x0, 0x1],			bits : 0
    DMACH9_EN :    [0x0, 0x1],			bits : 4
    DMACH13_EN:    [0x0, 0x1],			bits : 20
    DMACH14_EN:    [0x0, 0x1],			bits : 24
    DMACH15_EN:    [0x0, 0x1],			bits : 28
*/
#define AXI_REGISTER1_OFS 0x03ec
REGDEF_BEGIN(AXI_REGISTER1)
    REGDEF_BIT(DMACH8_EN ,        1)
    REGDEF_BIT(          ,        3)
    REGDEF_BIT(DMACH9_EN ,        1)
    REGDEF_BIT(          ,        15)
    REGDEF_BIT(DMACH13_EN,        1)
    REGDEF_BIT(          ,        3)
    REGDEF_BIT(DMACH14_EN,        1)
    REGDEF_BIT(          ,        3)
    REGDEF_BIT(DMACH15_EN,        1)
REGDEF_END(AXI_REGISTER1)


/*
    DMACH16_EN  :    [0x0, 0x1],			bits : 0
    DMACH17_EN  :    [0x0, 0x1],			bits : 4
    LLC_IN_CH_EN:    [0x0, 0x1],			bits : 24
*/
#define AXI_REGISTER2_OFS 0x03f0
REGDEF_BEGIN(AXI_REGISTER2)
    REGDEF_BIT(DMACH16_EN  ,        1)
    REGDEF_BIT(            ,        3)
    REGDEF_BIT(DMACH17_EN  ,        1)
    REGDEF_BIT(            ,        19)
    REGDEF_BIT(LLC_IN_CH_EN,        1)
REGDEF_END(AXI_REGISTER2)


/*
    DMACH0_LOCK_DISABLE:    [0x0, 0x1],			bits : 0
    DMACH1_LOCK_DISABLE:    [0x0, 0x1],			bits : 4
    DMACH2_LOCK_DISABLE:    [0x0, 0x1],			bits : 8
    DMACH3_LOCK_DISABLE:    [0x0, 0x1],			bits : 12
    DMACH4_LOCK_DISABLE:    [0x0, 0x1],			bits : 16
    DMACH5_LOCK_DISABLE:    [0x0, 0x1],			bits : 20
    DMACH6_LOCK_DISABLE:    [0x0, 0x1],			bits : 24
    DMACH7_LOCK_DISABLE:    [0x0, 0x1],			bits : 28
*/
#define AXI_REGISTER3_OFS 0x03f4
REGDEF_BEGIN(AXI_REGISTER3)
    REGDEF_BIT(DMACH0_LOCK_DISABLE,        1)
    REGDEF_BIT(                   ,        3)
    REGDEF_BIT(DMACH1_LOCK_DISABLE,        1)
    REGDEF_BIT(                   ,        3)
    REGDEF_BIT(DMACH2_LOCK_DISABLE,        1)
    REGDEF_BIT(                   ,        3)
    REGDEF_BIT(DMACH3_LOCK_DISABLE,        1)
    REGDEF_BIT(                   ,        3)
    REGDEF_BIT(DMACH4_LOCK_DISABLE,        1)
    REGDEF_BIT(                   ,        3)
    REGDEF_BIT(DMACH5_LOCK_DISABLE,        1)
    REGDEF_BIT(                   ,        3)
    REGDEF_BIT(DMACH6_LOCK_DISABLE,        1)
    REGDEF_BIT(                   ,        3)
    REGDEF_BIT(DMACH7_LOCK_DISABLE,        1)
REGDEF_END(AXI_REGISTER3)


/*
    DMACH8_LOCK_DISABLE :    [0x0, 0x1],			bits : 0
    DMACH9_LOCK_DISABLE :    [0x0, 0x1],			bits : 4
    DMACH13_LOCK_DISABLE:    [0x0, 0x1],			bits : 20
    DMACH14_LOCK_DISABLE:    [0x0, 0x1],			bits : 24
    DMACH15_LOCK_DISABLE:    [0x0, 0x1],			bits : 28
*/
#define AXI_REGISTER4_OFS 0x03f8
REGDEF_BEGIN(AXI_REGISTER4)
    REGDEF_BIT(DMACH8_LOCK_DISABLE ,        1)
    REGDEF_BIT(                    ,        3)
    REGDEF_BIT(DMACH9_LOCK_DISABLE ,        1)
    REGDEF_BIT(                    ,        15)
    REGDEF_BIT(DMACH13_LOCK_DISABLE,        1)
    REGDEF_BIT(                    ,        3)
    REGDEF_BIT(DMACH14_LOCK_DISABLE,        1)
    REGDEF_BIT(                    ,        3)
    REGDEF_BIT(DMACH15_LOCK_DISABLE,        1)
REGDEF_END(AXI_REGISTER4)


/*
    DMACH16_LOCK_DISABLE:    [0x0, 0x1],			bits : 0
    DMACH17_LOCK_DISABLE:    [0x0, 0x1],			bits : 4
    LLC_IN_LOCK_DISABLE :    [0x0, 0x1],			bits : 24
*/
#define AXI_REGISTER5_OFS 0x03fc
REGDEF_BEGIN(AXI_REGISTER5)
    REGDEF_BIT(DMACH16_LOCK_DISABLE,        1)
    REGDEF_BIT(                    ,        3)
    REGDEF_BIT(DMACH17_LOCK_DISABLE,        1)
    REGDEF_BIT(                    ,        19)
    REGDEF_BIT(LLC_IN_LOCK_DISABLE ,        1)
REGDEF_END(AXI_REGISTER5)


/*
    WRITE_CH_OUTSTANDING_NUM:    [0x0, 0xff],			bits : 7_0
    READ_CH_OUTSTANDING_NUM :    [0x0, 0xff],			bits : 15_8
*/
#define AXI_REGISTER6_OFS 0x0400
REGDEF_BEGIN(AXI_REGISTER6)
    REGDEF_BIT(WRITE_CH_OUTSTANDING_NUM,        8)
    REGDEF_BIT(READ_CH_OUTSTANDING_NUM ,        8)
REGDEF_END(AXI_REGISTER6)


/*
    AXI_BUS_DISABLE:    [0x0, 0x1],			bits : 0
    AXI_BUS_IDLE   :    [0x0, 0x1],			bits : 16
*/
#define AXI_REGISTER7_OFS 0x0404
REGDEF_BEGIN(AXI_REGISTER7)
    REGDEF_BIT(AXI_BUS_DISABLE,        1)
    REGDEF_BIT(               ,        15)
    REGDEF_BIT(AXI_BUS_IDLE   ,        1)
REGDEF_END(AXI_REGISTER7)


/*
    AXI_STATUS:    [0x0, 0xffffffff],			bits : 31_0
*/
#define AXI_REGISTER8_OFS 0x0408
REGDEF_BEGIN(AXI_REGISTER8)
    REGDEF_BIT(AXI_STATUS,        32)
REGDEF_END(AXI_REGISTER8)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED75_OFS 0x040c
REGDEF_BEGIN(RESERVED75)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED75)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED76_OFS 0x0410
REGDEF_BEGIN(RESERVED76)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED76)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED77_OFS 0x0414
REGDEF_BEGIN(RESERVED77)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED77)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED78_OFS 0x0418
REGDEF_BEGIN(RESERVED78)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED78)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED79_OFS 0x041c
REGDEF_BEGIN(RESERVED79)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED79)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED80_OFS 0x0420
REGDEF_BEGIN(RESERVED80)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED80)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED81_OFS 0x0424
REGDEF_BEGIN(RESERVED81)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED81)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED82_OFS 0x0428
REGDEF_BEGIN(RESERVED82)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED82)


/*
    LLC_ERR_CMD_ADR0:    [0x0, 0xffffffff],			bits : 31_0
*/
#define LLC_DEBUG_RESISTER0_OFS 0x042c
REGDEF_BEGIN(LLC_DEBUG_RESISTER0)
    REGDEF_BIT(LLC_ERR_CMD_ADR0,        32)
REGDEF_END(LLC_DEBUG_RESISTER0)


/*
    LLC_ERR_CMD_ADR1:    [0x0, 0xf],			bits : 3_0
*/
#define LLC_DEBUG_RESISTER1_OFS 0x0430
REGDEF_BEGIN(LLC_DEBUG_RESISTER1)
    REGDEF_BIT(LLC_ERR_CMD_ADR1,        4)
REGDEF_END(LLC_DEBUG_RESISTER1)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED83_OFS 0x0434
REGDEF_BEGIN(RESERVED83)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED83)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED84_OFS 0x0438
REGDEF_BEGIN(RESERVED84)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED84)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED85_OFS 0x043c
REGDEF_BEGIN(RESERVED85)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED85)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED86_OFS 0x0440
REGDEF_BEGIN(RESERVED86)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED86)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED87_OFS 0x0444
REGDEF_BEGIN(RESERVED87)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED87)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED88_OFS 0x0448
REGDEF_BEGIN(RESERVED88)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED88)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED89_OFS 0x044c
REGDEF_BEGIN(RESERVED89)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED89)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED90_OFS 0x0450
REGDEF_BEGIN(RESERVED90)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED90)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECKSUM_REGISTER0_OFS 0x0454
REGDEF_BEGIN(CHECKSUM_REGISTER0)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(CHECKSUM_REGISTER0)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECKSUM_REGISTER1_OFS 0x0458
REGDEF_BEGIN(CHECKSUM_REGISTER1)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(CHECKSUM_REGISTER1)


/*
    CONV_CHKSUM:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECKSUM_REGISTER2_OFS 0x045c
REGDEF_BEGIN(CHECKSUM_REGISTER2)
    REGDEF_BIT(CONV_CHKSUM,        32)
REGDEF_END(CHECKSUM_REGISTER2)


/*
    QUAN_CONVOUT_CHKSUM:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECKSUM_REGISTER3_OFS 0x0460
REGDEF_BEGIN(CHECKSUM_REGISTER3)
    REGDEF_BIT(QUAN_CONVOUT_CHKSUM,        32)
REGDEF_END(CHECKSUM_REGISTER3)


/*
    BIAS_CHKSUM:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECKSUM_REGISTER4_OFS 0x0464
REGDEF_BEGIN(CHECKSUM_REGISTER4)
    REGDEF_BIT(BIAS_CHKSUM,        32)
REGDEF_END(CHECKSUM_REGISTER4)


/*
    BN_CHKSUM:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECKSUM_REGISTER5_OFS 0x0468
REGDEF_BEGIN(CHECKSUM_REGISTER5)
    REGDEF_BIT(BN_CHKSUM,        32)
REGDEF_END(CHECKSUM_REGISTER5)


/*
    ELTWISE_CHKSUM:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECKSUM_REGISTER6_OFS 0x046c
REGDEF_BEGIN(CHECKSUM_REGISTER6)
    REGDEF_BIT(ELTWISE_CHKSUM,        32)
REGDEF_END(CHECKSUM_REGISTER6)


/*
    ACT_CHKSUM:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECKSUM_REGISTER7_OFS 0x0470
REGDEF_BEGIN(CHECKSUM_REGISTER7)
    REGDEF_BIT(ACT_CHKSUM,        32)
REGDEF_END(CHECKSUM_REGISTER7)


/*
    QUAN_OUT0_INPUT_CHKSUM:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECKSUM_REGISTER8_OFS 0x0474
REGDEF_BEGIN(CHECKSUM_REGISTER8)
    REGDEF_BIT(QUAN_OUT0_INPUT_CHKSUM,        32)
REGDEF_END(CHECKSUM_REGISTER8)


/*
    QUAN_OUT0_CHKSUM:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECKSUM_REGISTER9_OFS 0x0478
REGDEF_BEGIN(CHECKSUM_REGISTER9)
    REGDEF_BIT(QUAN_OUT0_CHKSUM,        32)
REGDEF_END(CHECKSUM_REGISTER9)


/*
    WCD_CHKSUM:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECKSUM_REGISTER10_OFS 0x047c
REGDEF_BEGIN(CHECKSUM_REGISTER10)
    REGDEF_BIT(WCD_CHKSUM,        32)
REGDEF_END(CHECKSUM_REGISTER10)


/*
    ACT_LUT_HW_CUR_CHKSUM:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECKSUM_REGISTER11_OFS 0x0480
REGDEF_BEGIN(CHECKSUM_REGISTER11)
    REGDEF_BIT(ACT_LUT_HW_CUR_CHKSUM,        32)
REGDEF_END(CHECKSUM_REGISTER11)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED91_OFS 0x0484
REGDEF_BEGIN(RESERVED91)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED91)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED92_OFS 0x0488
REGDEF_BEGIN(RESERVED92)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED92)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED93_OFS 0x048c
REGDEF_BEGIN(RESERVED93)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED93)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED94_OFS 0x0490
REGDEF_BEGIN(RESERVED94)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED94)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED95_OFS 0x0494
REGDEF_BEGIN(RESERVED95)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED95)


/*
    CONV_IN0_CUR_SRCBUF:    [0x0, 0x1],			bits : 0
*/
#define CONV_DEBUG_REGISTER0_OFS 0x0498
REGDEF_BEGIN(CONV_DEBUG_REGISTER0)
    REGDEF_BIT(CONV_IN0_CUR_SRCBUF,        1)
REGDEF_END(CONV_DEBUG_REGISTER0)


/*
    CONV_OUT0_WIDTH :    [0x0, 0xfff],			bits : 11_0
    CONV_OUT0_HEIGHT:    [0x0, 0xfff],			bits : 27_16
*/
#define CONV_DEBUG_REGISTER1_OFS 0x049c
REGDEF_BEGIN(CONV_DEBUG_REGISTER1)
    REGDEF_BIT(CONV_OUT0_WIDTH ,        12)
    REGDEF_BIT(                ,        4)
    REGDEF_BIT(CONV_OUT0_HEIGHT,        12)
REGDEF_END(CONV_DEBUG_REGISTER1)


/*
    CONV_OUT0_CHANNEL:    [0x0, 0xffff],			bits : 15_0
    CONV_OUT0_BATCH  :    [0x0, 0x7f],			bits : 26_20
*/
#define CONV_DEBUG_REGISTER2_OFS 0x04a0
REGDEF_BEGIN(CONV_DEBUG_REGISTER2)
    REGDEF_BIT(CONV_OUT0_CHANNEL,        16)
    REGDEF_BIT(                 ,        4)
    REGDEF_BIT(CONV_OUT0_BATCH  ,        7)
REGDEF_END(CONV_DEBUG_REGISTER2)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED96_OFS 0x04a4
REGDEF_BEGIN(RESERVED96)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED96)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED97_OFS 0x04a8
REGDEF_BEGIN(RESERVED97)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED97)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED98_OFS 0x04ac
REGDEF_BEGIN(RESERVED98)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED98)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED99_OFS 0x04b0
REGDEF_BEGIN(RESERVED99)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED99)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED100_OFS 0x04b4
REGDEF_BEGIN(RESERVED100)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED100)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED101_OFS 0x04b8
REGDEF_BEGIN(RESERVED101)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED101)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED102_OFS 0x04bc
REGDEF_BEGIN(RESERVED102)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED102)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED103_OFS 0x04c0
REGDEF_BEGIN(RESERVED103)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED103)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED104_OFS 0x04c4
REGDEF_BEGIN(RESERVED104)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED104)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED105_OFS 0x04c8
REGDEF_BEGIN(RESERVED105)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED105)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED106_OFS 0x04cc
REGDEF_BEGIN(RESERVED106)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED106)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED107_OFS 0x04d0
REGDEF_BEGIN(RESERVED107)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED107)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED108_OFS 0x04d4
REGDEF_BEGIN(RESERVED108)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED108)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED109_OFS 0x04d8
REGDEF_BEGIN(RESERVED109)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED109)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED110_OFS 0x04dc
REGDEF_BEGIN(RESERVED110)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED110)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED111_OFS 0x04e0
REGDEF_BEGIN(RESERVED111)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED111)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED112_OFS 0x04e4
REGDEF_BEGIN(RESERVED112)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED112)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED113_OFS 0x04e8
REGDEF_BEGIN(RESERVED113)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED113)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED114_OFS 0x04ec
REGDEF_BEGIN(RESERVED114)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED114)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED115_OFS 0x04f0
REGDEF_BEGIN(RESERVED115)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED115)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED116_OFS 0x04f4
REGDEF_BEGIN(RESERVED116)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED116)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED117_OFS 0x04f8
REGDEF_BEGIN(RESERVED117)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED117)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED118_OFS 0x04fc
REGDEF_BEGIN(RESERVED118)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED118)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED119_OFS 0x0500
REGDEF_BEGIN(RESERVED119)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED119)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED120_OFS 0x0504
REGDEF_BEGIN(RESERVED120)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED120)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED121_OFS 0x0508
REGDEF_BEGIN(RESERVED121)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED121)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED122_OFS 0x050c
REGDEF_BEGIN(RESERVED122)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED122)


/*
    CONV_PRE_IN0_CONV_MODE:    [0x0, 0x7],			bits : 2_0
*/
#define PRE_IN0_CONV_SUB_FUNCTION_FLOW_CONTROL_REGISTER0_OFS 0x0510
REGDEF_BEGIN(PRE_IN0_CONV_SUB_FUNCTION_FLOW_CONTROL_REGISTER0)
    REGDEF_BIT(CONV_PRE_IN0_CONV_MODE,        3)
REGDEF_END(PRE_IN0_CONV_SUB_FUNCTION_FLOW_CONTROL_REGISTER0)


/*
    CONV_PRE_IN0_PERMUTE_WC_EN:    [0x0, 0x1],			bits : 4
    CONV_PRE_IN0_FLATTEN2CH_EN:    [0x0, 0x1],			bits : 5
*/
#define PRE_IN0_CONV_SUB_FUNCTION_FLOW_CONTROL_REGISTER1_OFS 0x0514
REGDEF_BEGIN(PRE_IN0_CONV_SUB_FUNCTION_FLOW_CONTROL_REGISTER1)
    REGDEF_BIT(                          ,        4)
    REGDEF_BIT(CONV_PRE_IN0_PERMUTE_WC_EN,        1)
    REGDEF_BIT(CONV_PRE_IN0_FLATTEN2CH_EN,        1)
REGDEF_END(PRE_IN0_CONV_SUB_FUNCTION_FLOW_CONTROL_REGISTER1)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED123_OFS 0x0518
REGDEF_BEGIN(RESERVED123)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED123)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED124_OFS 0x051c
REGDEF_BEGIN(RESERVED124)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED124)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED125_OFS 0x0520
REGDEF_BEGIN(RESERVED125)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED125)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED126_OFS 0x0524
REGDEF_BEGIN(RESERVED126)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED126)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED127_OFS 0x0528
REGDEF_BEGIN(RESERVED127)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED127)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED128_OFS 0x052c
REGDEF_BEGIN(RESERVED128)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED128)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED129_OFS 0x0530
REGDEF_BEGIN(RESERVED129)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED129)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED130_OFS 0x0534
REGDEF_BEGIN(RESERVED130)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED130)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED131_OFS 0x0538
REGDEF_BEGIN(RESERVED131)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED131)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED132_OFS 0x053c
REGDEF_BEGIN(RESERVED132)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED132)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED133_OFS 0x0540
REGDEF_BEGIN(RESERVED133)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED133)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED134_OFS 0x0544
REGDEF_BEGIN(RESERVED134)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED134)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED135_OFS 0x0548
REGDEF_BEGIN(RESERVED135)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED135)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED136_OFS 0x054c
REGDEF_BEGIN(RESERVED136)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED136)


/*
    DRAMUB_PRE_SAI0:    [0x0, 0xffffffff],			bits : 31_0
*/
#define PRE_IN0_DMA_TO_CONV_CHANNEL_REGISTER0_OFS 0x0550
REGDEF_BEGIN(PRE_IN0_DMA_TO_CONV_CHANNEL_REGISTER0)
    REGDEF_BIT(DRAMUB_PRE_SAI0,        32)
REGDEF_END(PRE_IN0_DMA_TO_CONV_CHANNEL_REGISTER0)


/*
    DRAMUB_MSB_PRE_SAI0 :    [0x0, 0xf],			bits : 3_0
    DRAMUB_MODE_PRE_SAI0:    [0x0, 0xf],			bits : 31_28
*/
#define PRE_IN0_DMA_TO_CONV_CHANNEL_REGISTER1_OFS 0x0554
REGDEF_BEGIN(PRE_IN0_DMA_TO_CONV_CHANNEL_REGISTER1)
    REGDEF_BIT(DRAMUB_MSB_PRE_SAI0 ,        4)
    REGDEF_BIT(                    ,        24)
    REGDEF_BIT(DRAMUB_MODE_PRE_SAI0,        4)
REGDEF_END(PRE_IN0_DMA_TO_CONV_CHANNEL_REGISTER1)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED137_OFS 0x0558
REGDEF_BEGIN(RESERVED137)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED137)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED138_OFS 0x055c
REGDEF_BEGIN(RESERVED138)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED138)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED139_OFS 0x0560
REGDEF_BEGIN(RESERVED139)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED139)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED140_OFS 0x0564
REGDEF_BEGIN(RESERVED140)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED140)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED141_OFS 0x0568
REGDEF_BEGIN(RESERVED141)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED141)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED142_OFS 0x056c
REGDEF_BEGIN(RESERVED142)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED142)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED143_OFS 0x0570
REGDEF_BEGIN(RESERVED143)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED143)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED144_OFS 0x0574
REGDEF_BEGIN(RESERVED144)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED144)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED145_OFS 0x0578
REGDEF_BEGIN(RESERVED145)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED145)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED146_OFS 0x057c
REGDEF_BEGIN(RESERVED146)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED146)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED147_OFS 0x0580
REGDEF_BEGIN(RESERVED147)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED147)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED148_OFS 0x0584
REGDEF_BEGIN(RESERVED148)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED148)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED149_OFS 0x0588
REGDEF_BEGIN(RESERVED149)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED149)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED150_OFS 0x058c
REGDEF_BEGIN(RESERVED150)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED150)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED151_OFS 0x0590
REGDEF_BEGIN(RESERVED151)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED151)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED152_OFS 0x0594
REGDEF_BEGIN(RESERVED152)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED152)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED153_OFS 0x0598
REGDEF_BEGIN(RESERVED153)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED153)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED154_OFS 0x059c
REGDEF_BEGIN(RESERVED154)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED154)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED155_OFS 0x05a0
REGDEF_BEGIN(RESERVED155)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED155)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED156_OFS 0x05a4
REGDEF_BEGIN(RESERVED156)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED156)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED157_OFS 0x05a8
REGDEF_BEGIN(RESERVED157)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED157)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED158_OFS 0x05ac
REGDEF_BEGIN(RESERVED158)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED158)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED159_OFS 0x05b0
REGDEF_BEGIN(RESERVED159)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED159)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED160_OFS 0x05b4
REGDEF_BEGIN(RESERVED160)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED160)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED161_OFS 0x05b8
REGDEF_BEGIN(RESERVED161)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED161)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED162_OFS 0x05bc
REGDEF_BEGIN(RESERVED162)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED162)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED163_OFS 0x05c0
REGDEF_BEGIN(RESERVED163)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED163)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED164_OFS 0x05c4
REGDEF_BEGIN(RESERVED164)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED164)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED165_OFS 0x05c8
REGDEF_BEGIN(RESERVED165)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED165)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED166_OFS 0x05cc
REGDEF_BEGIN(RESERVED166)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED166)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED167_OFS 0x05d0
REGDEF_BEGIN(RESERVED167)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED167)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED168_OFS 0x05d4
REGDEF_BEGIN(RESERVED168)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED168)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED169_OFS 0x05d8
REGDEF_BEGIN(RESERVED169)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED169)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED170_OFS 0x05dc
REGDEF_BEGIN(RESERVED170)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED170)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED171_OFS 0x05e0
REGDEF_BEGIN(RESERVED171)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED171)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED172_OFS 0x05e4
REGDEF_BEGIN(RESERVED172)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED172)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED173_OFS 0x05e8
REGDEF_BEGIN(RESERVED173)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED173)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED174_OFS 0x05ec
REGDEF_BEGIN(RESERVED174)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED174)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED175_OFS 0x05f0
REGDEF_BEGIN(RESERVED175)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED175)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED176_OFS 0x05f4
REGDEF_BEGIN(RESERVED176)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED176)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED177_OFS 0x05f8
REGDEF_BEGIN(RESERVED177)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED177)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED178_OFS 0x05fc
REGDEF_BEGIN(RESERVED178)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED178)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED179_OFS 0x0600
REGDEF_BEGIN(RESERVED179)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED179)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED180_OFS 0x0604
REGDEF_BEGIN(RESERVED180)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED180)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED181_OFS 0x0608
REGDEF_BEGIN(RESERVED181)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED181)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED182_OFS 0x060c
REGDEF_BEGIN(RESERVED182)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED182)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED183_OFS 0x0610
REGDEF_BEGIN(RESERVED183)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED183)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED184_OFS 0x0614
REGDEF_BEGIN(RESERVED184)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED184)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED185_OFS 0x0618
REGDEF_BEGIN(RESERVED185)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED185)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED186_OFS 0x061c
REGDEF_BEGIN(RESERVED186)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED186)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED187_OFS 0x0620
REGDEF_BEGIN(RESERVED187)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED187)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED188_OFS 0x0624
REGDEF_BEGIN(RESERVED188)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED188)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED189_OFS 0x0628
REGDEF_BEGIN(RESERVED189)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED189)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED190_OFS 0x062c
REGDEF_BEGIN(RESERVED190)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED190)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED191_OFS 0x0630
REGDEF_BEGIN(RESERVED191)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED191)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED192_OFS 0x0634
REGDEF_BEGIN(RESERVED192)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED192)


/*
    CONV_PRE_IN0_LOFS_EN :    [0x0, 0x1],			bits : 0
    CONV_PRE_IN0_CHOFS_EN:    [0x0, 0x1],			bits : 1
    CONV_PRE_IN0_BOFS_EN :    [0x0, 0x1],			bits : 2
*/
#define PRE_IN0_DMA_TO_CONV_OFFSET_ENABLE_REGISTER0_OFS 0x0638
REGDEF_BEGIN(PRE_IN0_DMA_TO_CONV_OFFSET_ENABLE_REGISTER0)
    REGDEF_BIT(CONV_PRE_IN0_LOFS_EN ,        1)
    REGDEF_BIT(CONV_PRE_IN0_CHOFS_EN,        1)
    REGDEF_BIT(CONV_PRE_IN0_BOFS_EN ,        1)
REGDEF_END(PRE_IN0_DMA_TO_CONV_OFFSET_ENABLE_REGISTER0)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED193_OFS 0x063c
REGDEF_BEGIN(RESERVED193)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED193)


/*
    CONV_PRE_IN0_LOFS:    [0x0, 0x1fffffff],			bits : 28_0
*/
#define PRE_IN0_DMA_TO_CONV_OFFSET_REGISTER0_OFS 0x0640
REGDEF_BEGIN(PRE_IN0_DMA_TO_CONV_OFFSET_REGISTER0)
    REGDEF_BIT(CONV_PRE_IN0_LOFS,        29)
REGDEF_END(PRE_IN0_DMA_TO_CONV_OFFSET_REGISTER0)


/*
    CONV_PRE_IN0_CHOFS:    [0x0, 0x1fffffff],			bits : 28_0
*/
#define PRE_IN0_DMA_TO_CONV_OFFSET_REGISTER1_OFS 0x0644
REGDEF_BEGIN(PRE_IN0_DMA_TO_CONV_OFFSET_REGISTER1)
    REGDEF_BIT(CONV_PRE_IN0_CHOFS,        29)
REGDEF_END(PRE_IN0_DMA_TO_CONV_OFFSET_REGISTER1)


/*
    CONV_PRE_IN0_BOFS:    [0x0, 0x1fffffff],			bits : 28_0
*/
#define PRE_IN0_DMA_TO_CONV_OFFSET_REGISTER2_OFS 0x0648
REGDEF_BEGIN(PRE_IN0_DMA_TO_CONV_OFFSET_REGISTER2)
    REGDEF_BIT(CONV_PRE_IN0_BOFS,        29)
REGDEF_END(PRE_IN0_DMA_TO_CONV_OFFSET_REGISTER2)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED194_OFS 0x064c
REGDEF_BEGIN(RESERVED194)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED194)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED195_OFS 0x0650
REGDEF_BEGIN(RESERVED195)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED195)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED196_OFS 0x0654
REGDEF_BEGIN(RESERVED196)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED196)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED197_OFS 0x0658
REGDEF_BEGIN(RESERVED197)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED197)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED198_OFS 0x065c
REGDEF_BEGIN(RESERVED198)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED198)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED199_OFS 0x0660
REGDEF_BEGIN(RESERVED199)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED199)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED200_OFS 0x0664
REGDEF_BEGIN(RESERVED200)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED200)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED201_OFS 0x0668
REGDEF_BEGIN(RESERVED201)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED201)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED202_OFS 0x066c
REGDEF_BEGIN(RESERVED202)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED202)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED203_OFS 0x0670
REGDEF_BEGIN(RESERVED203)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED203)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED204_OFS 0x0674
REGDEF_BEGIN(RESERVED204)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED204)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED205_OFS 0x0678
REGDEF_BEGIN(RESERVED205)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED205)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED206_OFS 0x067c
REGDEF_BEGIN(RESERVED206)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED206)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED207_OFS 0x0680
REGDEF_BEGIN(RESERVED207)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED207)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED208_OFS 0x0684
REGDEF_BEGIN(RESERVED208)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED208)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED209_OFS 0x0688
REGDEF_BEGIN(RESERVED209)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED209)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED210_OFS 0x068c
REGDEF_BEGIN(RESERVED210)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED210)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED211_OFS 0x0690
REGDEF_BEGIN(RESERVED211)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED211)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED212_OFS 0x0694
REGDEF_BEGIN(RESERVED212)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED212)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED213_OFS 0x0698
REGDEF_BEGIN(RESERVED213)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED213)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED214_OFS 0x069c
REGDEF_BEGIN(RESERVED214)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED214)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED215_OFS 0x06a0
REGDEF_BEGIN(RESERVED215)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED215)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED216_OFS 0x06a4
REGDEF_BEGIN(RESERVED216)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED216)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED217_OFS 0x06a8
REGDEF_BEGIN(RESERVED217)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED217)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED218_OFS 0x06ac
REGDEF_BEGIN(RESERVED218)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED218)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED219_OFS 0x06b0
REGDEF_BEGIN(RESERVED219)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED219)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED220_OFS 0x06b4
REGDEF_BEGIN(RESERVED220)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED220)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED221_OFS 0x06b8
REGDEF_BEGIN(RESERVED221)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED221)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED222_OFS 0x06bc
REGDEF_BEGIN(RESERVED222)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED222)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED223_OFS 0x06c0
REGDEF_BEGIN(RESERVED223)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED223)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED224_OFS 0x06c4
REGDEF_BEGIN(RESERVED224)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED224)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED225_OFS 0x06c8
REGDEF_BEGIN(RESERVED225)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED225)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED226_OFS 0x06cc
REGDEF_BEGIN(RESERVED226)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED226)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED227_OFS 0x06d0
REGDEF_BEGIN(RESERVED227)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED227)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED228_OFS 0x06d4
REGDEF_BEGIN(RESERVED228)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED228)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED229_OFS 0x06d8
REGDEF_BEGIN(RESERVED229)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED229)


/*
    CONV_PRE_IN0_WIDTH :    [0x0, 0xfff],			bits : 11_0
    CONV_PRE_IN0_HEIGHT:    [0x0, 0xfff],			bits : 27_16
*/
#define PRE_IN0_CONV_SIZE_REGISTER0_OFS 0x06dc
REGDEF_BEGIN(PRE_IN0_CONV_SIZE_REGISTER0)
    REGDEF_BIT(CONV_PRE_IN0_WIDTH ,        12)
    REGDEF_BIT(                   ,        4)
    REGDEF_BIT(CONV_PRE_IN0_HEIGHT,        12)
REGDEF_END(PRE_IN0_CONV_SIZE_REGISTER0)


/*
    CONV_PRE_IN0_CHANNEL:    [0x0, 0xffff],			bits : 15_0
    CONV_PRE_IN0_BATCH  :    [0x0, 0x7f],			bits : 26_20
*/
#define PRE_IN0_CONV_SIZE_REGISTER1_OFS 0x06e0
REGDEF_BEGIN(PRE_IN0_CONV_SIZE_REGISTER1)
    REGDEF_BIT(CONV_PRE_IN0_CHANNEL,        16)
    REGDEF_BIT(                    ,        4)
    REGDEF_BIT(CONV_PRE_IN0_BATCH  ,        7)
REGDEF_END(PRE_IN0_CONV_SIZE_REGISTER1)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED230_OFS 0x06e4
REGDEF_BEGIN(RESERVED230)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED230)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED231_OFS 0x06e8
REGDEF_BEGIN(RESERVED231)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED231)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED232_OFS 0x06ec
REGDEF_BEGIN(RESERVED232)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED232)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED233_OFS 0x06f0
REGDEF_BEGIN(RESERVED233)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED233)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED234_OFS 0x06f4
REGDEF_BEGIN(RESERVED234)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED234)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED235_OFS 0x06f8
REGDEF_BEGIN(RESERVED235)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED235)


/*
    CONV_PRE_IN0_BIT_DEPTH:    [0x0, 0x3],			bits : 1_0
    CONV_PRE_IN0_DATA_FMT :    [0x0, 0x1],			bits : 4
*/
#define PRE_IN0_CONV_DATA_TYPE_REGISTER0_OFS 0x06fc
REGDEF_BEGIN(PRE_IN0_CONV_DATA_TYPE_REGISTER0)
    REGDEF_BIT(CONV_PRE_IN0_BIT_DEPTH,        2)
    REGDEF_BIT(                      ,        2)
    REGDEF_BIT(CONV_PRE_IN0_DATA_FMT ,        1)
REGDEF_END(PRE_IN0_CONV_DATA_TYPE_REGISTER0)


/*
    CONV_PRE_IN0_W_BIT_DEPTH:    [0x0, 0x3],			bits : 1_0
*/
#define PRE_IN0_CONV_DATA_TYPE_REGISTER1_OFS 0x0700
REGDEF_BEGIN(PRE_IN0_CONV_DATA_TYPE_REGISTER1)
    REGDEF_BIT(CONV_PRE_IN0_W_BIT_DEPTH,        2)
REGDEF_END(PRE_IN0_CONV_DATA_TYPE_REGISTER1)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED236_OFS 0x0704
REGDEF_BEGIN(RESERVED236)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED236)


/*
    CONV_PRE_IN0_C0_MODE  :    [0x0, 0x1],			bits : 0
    CONV_PRE_IN0_NC_LAYOUT:    [0x0, 0x1],			bits : 12
*/
#define PRE_IN0_CONV_DATA_LAYOUT_REGISTER0_OFS 0x0708
REGDEF_BEGIN(PRE_IN0_CONV_DATA_LAYOUT_REGISTER0)
    REGDEF_BIT(CONV_PRE_IN0_C0_MODE  ,        1)
    REGDEF_BIT(                      ,        11)
    REGDEF_BIT(CONV_PRE_IN0_NC_LAYOUT,        1)
REGDEF_END(PRE_IN0_CONV_DATA_LAYOUT_REGISTER0)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED237_OFS 0x070c
REGDEF_BEGIN(RESERVED237)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED237)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED238_OFS 0x0710
REGDEF_BEGIN(RESERVED238)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED238)


/*
    CONV_PRE_IN0_CONV_STRIDE_W:    [0x0, 0xff],			bits : 7_0
*/
#define PRE_IN0_CONV_CONVOLUTION_REGISTER1_OFS 0x0714
REGDEF_BEGIN(PRE_IN0_CONV_CONVOLUTION_REGISTER1)
    REGDEF_BIT(CONV_PRE_IN0_CONV_STRIDE_W,        8)
REGDEF_END(PRE_IN0_CONV_CONVOLUTION_REGISTER1)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED239_OFS 0x0718
REGDEF_BEGIN(RESERVED239)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED239)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED240_OFS 0x071c
REGDEF_BEGIN(RESERVED240)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED240)


/*
    Reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED241_OFS 0x0720
REGDEF_BEGIN(RESERVED241)
    REGDEF_BIT(Reserved,        32)
REGDEF_END(RESERVED241)


/*
    CONV_PRE_IN0_CONV_MAC_PARALLELISM:    [0x0, 0x7],			bits : 2_0
*/
#define PRE_IN0_CONV_CONVOLUTION_REGISTER4_OFS 0x0724
REGDEF_BEGIN(PRE_IN0_CONV_CONVOLUTION_REGISTER4)
    REGDEF_BIT(CONV_PRE_IN0_CONV_MAC_PARALLELISM,        3)
REGDEF_END(PRE_IN0_CONV_CONVOLUTION_REGISTER4)


typedef struct
{

  union
  {
    struct
    {
      unsigned CONV_SW_RST          : 1;		// bits : 0
      unsigned CONV_START           : 1;		// bits : 1
      unsigned                      : 26;
      unsigned JOB_START            : 1;		// bits : 28
      unsigned JOB_TERMINATE        : 1;		// bits : 29
    } Bit;
    UINT32 Word;
  } CONV_Register_0; // 0x0000

  union
  {
    struct
    {
      unsigned INTE_FRM_DONE                         : 1;		// bits : 0
      unsigned                                       : 3;
      unsigned INTE_WCD_DECODE_WEIGHT_NUM_ERR        : 1;		// bits : 4
      unsigned                                       : 3;
      unsigned INTE_WCD_VLC_BITSTREAM_ERR            : 1;		// bits : 8
      unsigned INTE_WCD_VLC_VAL_RANGE_ERR            : 1;		// bits : 9
      unsigned                                       : 2;
      unsigned INTE_ACT_LUT_ERR                      : 1;		// bits : 12
      unsigned                                       : 11;
      unsigned INTE_JOB_END                          : 1;		// bits : 24
      unsigned INTE_JOB_ERR                          : 1;		// bits : 25
      unsigned INTE_JOB_WR_END                       : 1;		// bits : 26
    } Bit;
    UINT32 Word;
  } CONV_Register_1; // 0x0004

  union
  {
    struct
    {
      unsigned INTS_FRM_DONE                         : 1;		// bits : 0
      unsigned                                       : 3;
      unsigned INTS_WCD_DECODE_WEIGHT_NUM_ERR        : 1;		// bits : 4
      unsigned                                       : 3;
      unsigned INTS_WCD_VLC_BITSTREAM_ERR            : 1;		// bits : 8
      unsigned INTS_WCD_VLC_VAL_RANGE_ERR            : 1;		// bits : 9
      unsigned                                       : 2;
      unsigned INTS_ACT_LUT_ERR                      : 1;		// bits : 12
      unsigned                                       : 11;
      unsigned INTS_JOB_END                          : 1;		// bits : 24
      unsigned INTS_JOB_ERR                          : 1;		// bits : 25
      unsigned INTS_JOB_WR_END                       : 1;		// bits : 26
    } Bit;
    UINT32 Word;
  } CONV_Register_2; // 0x0008

  union
  {
    struct
    {
      unsigned CONV_CONV_EN              : 1;		// bits : 0
      unsigned                           : 3;
      unsigned CONV_BIAS_EN              : 1;		// bits : 4
      unsigned                           : 3;
      unsigned CONV_ROUTER_ORDER         : 3;		// bits : 10_8
      unsigned                           : 1;
      unsigned CONV_BN_EN                : 1;		// bits : 12
      unsigned CONV_ELT_EN               : 1;		// bits : 13
      unsigned CONV_ACT_EN               : 1;		// bits : 14
      unsigned CONV_POOL_EN              : 1;		// bits : 15
      unsigned                           : 9;
      unsigned CONV_WCD_VLC_EN           : 1;		// bits : 25
      unsigned CONV_WCD_KMEANS_EN        : 1;		// bits : 26
      unsigned                           : 1;
      unsigned CONV_DW_EN                : 1;		// bits : 28
      unsigned CONV_DW_BIAS_EN           : 1;		// bits : 29
      unsigned CONV_DW_RELU_EN           : 1;		// bits : 30
      unsigned CONV_DIM_MODE             : 1;		// bits : 31
    } Bit;
    UINT32 Word;
  } CONV_Register_3; // 0x000c

  union
  {
    struct
    {
      unsigned CONV_CONV_MODE                 : 3;		// bits : 2_0
      unsigned                                : 1;
      unsigned CONV_BN_W_MODE                 : 1;		// bits : 4
      unsigned                                : 3;
      unsigned CONV_ELT_MODE                  : 1;		// bits : 8
      unsigned                                : 3;
      unsigned CONV_POOL_MODE                 : 1;		// bits : 12
      unsigned                                : 3;
      unsigned CONV_IN1_BROADCAST_MODE        : 3;		// bits : 18_16
      unsigned                                : 1;
      unsigned CONV_ACT_A1_MODE               : 1;		// bits : 20
      unsigned CONV_ACT_D_MODE                : 1;		// bits : 21
      unsigned CONV_ACT_LUT_UPDATE_EN         : 1;		// bits : 22
      unsigned                                : 1;
      unsigned CONV_DW_RELU_MODE              : 1;		// bits : 24
    } Bit;
    UINT32 Word;
  } CONV_Register_4; // 0x0010

  union
  {
    struct
    {
      unsigned CONV_IN0_REMAINSRC_EN                : 1;		// bits : 0
      unsigned CONV_IN0_SWITCHSRC_EN                : 1;		// bits : 1
      unsigned CONV_IN0_MERGESRC_EN                 : 1;		// bits : 2
      unsigned CONV_PRE_IN0_LOAD_EN                 : 1;		// bits : 3
      unsigned CONV_IN0_PERMUTE_WC_EN               : 1;		// bits : 4
      unsigned CONV_IN0_FLATTEN2CH_EN               : 1;		// bits : 5
      unsigned CONV_IN_CH_STRIPE_EN                 : 1;		// bits : 6
      unsigned                                      : 1;
      unsigned CONV_OUT0_DATA_MODE                  : 2;		// bits : 9_8
      unsigned                                      : 10;
      unsigned CONV_WCD_VLC_VAL_CONVERT_EN          : 1;		// bits : 20
      unsigned CONV_WCD_KMEANS_TBL_UPDATE_EN        : 1;		// bits : 21
      unsigned                                      : 2;
      unsigned CONV_WCD_VLC_BIT_EN                  : 4;		// bits : 27_24
      unsigned CONV_WCD_KMEANS_ENTRY_MODE           : 2;		// bits : 29_28
    } Bit;
    UINT32 Word;
  } CONV_Register_5; // 0x0014

  union
  {
    struct
    {
      unsigned                               : 16;
      unsigned CONV_QUAN_CONVOUT_MODE        : 1;		// bits : 16
      unsigned                               : 3;
      unsigned CONV_QUAN_OUT0_MODE           : 1;		// bits : 20
    } Bit;
    UINT32 Word;
  } CONV_Register_6; // 0x0018

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_7; // 0x001c

  union
  {
    struct
    {
      unsigned BASE_ADDR0        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_8; // 0x0020

  union
  {
    struct
    {
      unsigned BASE_MSB_ADDR0        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } CONV_Register_9; // 0x0024

  union
  {
    struct
    {
      unsigned BASE_ADDR1        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_10; // 0x0028

  union
  {
    struct
    {
      unsigned BASE_MSB_ADDR1        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } CONV_Register_11; // 0x002c

  union
  {
    struct
    {
      unsigned BASE_ADDR2        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_12; // 0x0030

  union
  {
    struct
    {
      unsigned BASE_MSB_ADDR2        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } CONV_Register_13; // 0x0034

  union
  {
    struct
    {
      unsigned BASE_ADDR3        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_14; // 0x0038

  union
  {
    struct
    {
      unsigned BASE_MSB_ADDR3        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } CONV_Register_15; // 0x003c

  union
  {
    struct
    {
      unsigned DRAMUB_SAIJOB        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_16; // 0x0040

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAIJOB        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } CONV_Register_17; // 0x0044

  union
  {
    struct
    {
      unsigned BASE_ADDR4        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_18; // 0x0048

  union
  {
    struct
    {
      unsigned BASE_MSB_ADDR4        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } CONV_Register_19; // 0x004c

  union
  {
    struct
    {
      unsigned DRAMUB_SAI0        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_20; // 0x0050

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAI0         : 4;		// bits : 3_0
      unsigned                         : 24;
      unsigned DRAMUB_MODE_SAI0        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CONV_Register_21; // 0x0054

  union
  {
    struct
    {
      unsigned DRAMUB_SAI1        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_22; // 0x0058

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAI1         : 4;		// bits : 3_0
      unsigned                         : 24;
      unsigned DRAMUB_MODE_SAI1        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CONV_Register_23; // 0x005c

  union
  {
    struct
    {
      unsigned DRAMUB_SAI2        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_24; // 0x0060

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAI2         : 4;		// bits : 3_0
      unsigned                         : 24;
      unsigned DRAMUB_MODE_SAI2        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CONV_Register_25; // 0x0064

  union
  {
    struct
    {
      unsigned DRAMUB_SAI3        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_26; // 0x0068

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAI3         : 4;		// bits : 3_0
      unsigned                         : 24;
      unsigned DRAMUB_MODE_SAI3        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CONV_Register_27; // 0x006c

  union
  {
    struct
    {
      unsigned DRAMUB_SAI4        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_28; // 0x0070

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAI4         : 4;		// bits : 3_0
      unsigned                         : 24;
      unsigned DRAMUB_MODE_SAI4        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CONV_Register_29; // 0x0074

  union
  {
    struct
    {
      unsigned DRAMUB_SAI5        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_30; // 0x0078

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAI5         : 4;		// bits : 3_0
      unsigned                         : 24;
      unsigned DRAMUB_MODE_SAI5        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CONV_Register_31; // 0x007c

  union
  {
    struct
    {
      unsigned DRAMUB_SAI6        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_32; // 0x0080

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAI6         : 4;		// bits : 3_0
      unsigned                         : 24;
      unsigned DRAMUB_MODE_SAI6        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CONV_Register_33; // 0x0084

  union
  {
    struct
    {
      unsigned DRAMUB_SAI7        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_34; // 0x0088

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAI7         : 4;		// bits : 3_0
      unsigned                         : 24;
      unsigned DRAMUB_MODE_SAI7        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CONV_Register_35; // 0x008c

  union
  {
    struct
    {
      unsigned DRAMUB_SAI8        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_36; // 0x0090

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAI8         : 4;		// bits : 3_0
      unsigned                         : 24;
      unsigned DRAMUB_MODE_SAI8        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CONV_Register_37; // 0x0094

  union
  {
    struct
    {
      unsigned DRAMUB_SAI9        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_38; // 0x0098

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAI9         : 4;		// bits : 3_0
      unsigned                         : 24;
      unsigned DRAMUB_MODE_SAI9        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CONV_Register_39; // 0x009c

  union
  {
    struct
    {
      unsigned DRAMUB_SAI10        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_40; // 0x00a0

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAI10         : 4;		// bits : 3_0
      unsigned                          : 24;
      unsigned DRAMUB_MODE_SAI10        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CONV_Register_41; // 0x00a4

  union
  {
    struct
    {
      unsigned DRAMUB_SAI11        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_42; // 0x00a8

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAI11         : 4;		// bits : 3_0
      unsigned                          : 24;
      unsigned DRAMUB_MODE_SAI11        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CONV_Register_43; // 0x00ac

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_44; // 0x00b0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_45; // 0x00b4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_46; // 0x00b8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_47; // 0x00bc

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_48; // 0x00c0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_49; // 0x00c4

  union
  {
    struct
    {
      unsigned DRAMUB_SAI15        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_50; // 0x00c8

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAI15         : 4;		// bits : 3_0
      unsigned                          : 24;
      unsigned DRAMUB_MODE_SAI15        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CONV_Register_51; // 0x00cc

  union
  {
    struct
    {
      unsigned DRAMUB_SAI16        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_52; // 0x00d0

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAI16         : 4;		// bits : 3_0
      unsigned                          : 24;
      unsigned DRAMUB_MODE_SAI16        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CONV_Register_53; // 0x00d4

  union
  {
    struct
    {
      unsigned DRAMUB_SAI17        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_54; // 0x00d8

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAI17         : 4;		// bits : 3_0
      unsigned                          : 24;
      unsigned DRAMUB_MODE_SAI17        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CONV_Register_55; // 0x00dc

  union
  {
    struct
    {
      unsigned DRAMUB_SAI18        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_56; // 0x00e0

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAI18         : 4;		// bits : 3_0
      unsigned                          : 24;
      unsigned DRAMUB_MODE_SAI18        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CONV_Register_57; // 0x00e4

  union
  {
    struct
    {
      unsigned DRAMUB_SAI19        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_58; // 0x00e8

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAI19         : 4;		// bits : 3_0
      unsigned                          : 24;
      unsigned DRAMUB_MODE_SAI19        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CONV_Register_59; // 0x00ec

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_60; // 0x00f0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_61; // 0x00f4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_62; // 0x00f8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_63; // 0x00fc

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_64; // 0x0100

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_65; // 0x0104

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_66; // 0x0108

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_67; // 0x010c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_68; // 0x0110

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_69; // 0x0114

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_70; // 0x0118

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_71; // 0x011c

  union
  {
    struct
    {
      unsigned DRAMUB_SAO0        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_72; // 0x0120

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAO0         : 4;		// bits : 3_0
      unsigned                         : 24;
      unsigned DRAMUB_MODE_SAO0        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CONV_Register_73; // 0x0124

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_74; // 0x0128

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_75; // 0x012c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_76; // 0x0130

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_77; // 0x0134

  union
  {
    struct
    {
      unsigned CONV_IN0_LOFS_EN                  : 1;		// bits : 0
      unsigned CONV_IN0_CHOFS_EN                 : 1;		// bits : 1
      unsigned CONV_IN0_BOFS_EN                  : 1;		// bits : 2
      unsigned                                   : 1;
      unsigned CONV_IN1_LOFS_EN                  : 1;		// bits : 4
      unsigned CONV_IN1_CHOFS_EN                 : 1;		// bits : 5
      unsigned CONV_IN1_BOFS_EN                  : 1;		// bits : 6
      unsigned                                   : 1;
      unsigned CONV_IN_CH_STRIPE_LOFS_EN         : 1;		// bits : 8
      unsigned CONV_IN_CH_STRIPE_CHOFS_EN        : 1;		// bits : 9
      unsigned CONV_IN_CH_STRIPE_BOFS_EN         : 1;		// bits : 10
      unsigned                                   : 1;
      unsigned CONV_W_LOFS_EN                    : 1;		// bits : 12
      unsigned CONV_W_CHOFS_EN                   : 1;		// bits : 13
      unsigned CONV_W_BOFS_EN                    : 1;		// bits : 14
      unsigned CONV_W_DOFS_EN                    : 1;		// bits : 15
      unsigned CONV_OUT0_LOFS_EN                 : 1;		// bits : 16
      unsigned CONV_OUT0_CHOFS_EN                : 1;		// bits : 17
      unsigned CONV_OUT0_BOFS_EN                 : 1;		// bits : 18
    } Bit;
    UINT32 Word;
  } CONV_Register_78; // 0x0138

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_79; // 0x013c

  union
  {
    struct
    {
      unsigned CONV_IN0_LOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CONV_Register_80; // 0x0140

  union
  {
    struct
    {
      unsigned CONV_IN0_CHOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CONV_Register_81; // 0x0144

  union
  {
    struct
    {
      unsigned CONV_IN0_BOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CONV_Register_82; // 0x0148

  union
  {
    struct
    {
      unsigned CONV_IN1_LOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CONV_Register_83; // 0x014c

  union
  {
    struct
    {
      unsigned CONV_IN1_CHOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CONV_Register_84; // 0x0150

  union
  {
    struct
    {
      unsigned CONV_IN1_BOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CONV_Register_85; // 0x0154

  union
  {
    struct
    {
      unsigned CONV_IN_CH_STRIPE_LOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CONV_Register_86; // 0x0158

  union
  {
    struct
    {
      unsigned CONV_IN_CH_STRIPE_CHOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CONV_Register_87; // 0x015c

  union
  {
    struct
    {
      unsigned CONV_IN_CH_STRIPE_BOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CONV_Register_88; // 0x0160

  union
  {
    struct
    {
      unsigned CONV_W_LOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CONV_Register_89; // 0x0164

  union
  {
    struct
    {
      unsigned CONV_W_CHOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CONV_Register_90; // 0x0168

  union
  {
    struct
    {
      unsigned CONV_W_BOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CONV_Register_91; // 0x016c

  union
  {
    struct
    {
      unsigned CONV_OUT0_LOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CONV_Register_92; // 0x0170

  union
  {
    struct
    {
      unsigned CONV_OUT0_CHOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CONV_Register_93; // 0x0174

  union
  {
    struct
    {
      unsigned CONV_OUT0_BOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CONV_Register_94; // 0x0178

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_95; // 0x017c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_96; // 0x0180

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_97; // 0x0184

  union
  {
    struct
    {
      unsigned CONV_W_DOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CONV_Register_98; // 0x0188

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_99; // 0x018c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_100; // 0x0190

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_101; // 0x0194

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_102; // 0x0198

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_103; // 0x019c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_104; // 0x01a0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_105; // 0x01a4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_106; // 0x01a8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_107; // 0x01ac

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_108; // 0x01b0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_109; // 0x01b4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_110; // 0x01b8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_111; // 0x01bc

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_112; // 0x01c0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_113; // 0x01c4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_114; // 0x01c8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_115; // 0x01cc

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_116; // 0x01d0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_117; // 0x01d4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_118; // 0x01d8

  union
  {
    struct
    {
      unsigned CONV_IN0_WIDTH         : 12;		// bits : 11_0
      unsigned                        : 4;
      unsigned CONV_IN0_HEIGHT        : 12;		// bits : 27_16
    } Bit;
    UINT32 Word;
  } CONV_Register_119; // 0x01dc

  union
  {
    struct
    {
      unsigned CONV_IN0_CHANNEL        : 16;		// bits : 15_0
      unsigned                         : 4;
      unsigned CONV_IN0_BATCH          : 7;		// bits : 26_20
    } Bit;
    UINT32 Word;
  } CONV_Register_120; // 0x01e0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_121; // 0x01e4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_122; // 0x01e8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_123; // 0x01ec

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_124; // 0x01f0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_125; // 0x01f4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_126; // 0x01f8

  union
  {
    struct
    {
      unsigned CONV_IN0_BIT_DEPTH         : 2;		// bits : 1_0
      unsigned                            : 2;
      unsigned CONV_IN0_DATA_FMT          : 1;		// bits : 4
      unsigned                            : 3;
      unsigned CONV_IN1_BIT_DEPTH         : 2;		// bits : 9_8
      unsigned                            : 2;
      unsigned CONV_IN1_DATA_FMT          : 1;		// bits : 12
      unsigned                            : 3;
      unsigned CONV_OUT0_BIT_DEPTH        : 3;		// bits : 18_16
      unsigned                            : 1;
      unsigned CONV_OUT0_DATA_FMT         : 2;		// bits : 21_20
    } Bit;
    UINT32 Word;
  } CONV_Register_127; // 0x01fc

  union
  {
    struct
    {
      unsigned CONV_W_BIT_DEPTH                  : 2;		// bits : 1_0
      unsigned                                   : 2;
      unsigned CONV_W_DATA_FMT                   : 1;		// bits : 4
      unsigned                                   : 3;
      unsigned CONV_IN_CH_STRIPE_DATA_FMT        : 2;		// bits : 9_8
      unsigned                                   : 2;
      unsigned CONV_BIAS_DATA_FMT                : 1;		// bits : 12
      unsigned                                   : 3;
      unsigned CONV_DW_IN_FMT                    : 1;		// bits : 16
      unsigned                                   : 3;
      unsigned CONV_DW_W_FMT                     : 1;		// bits : 20
    } Bit;
    UINT32 Word;
  } CONV_Register_128; // 0x0200

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_129; // 0x0204

  union
  {
    struct
    {
      unsigned CONV_IN0_C0_MODE           : 1;		// bits : 0
      unsigned                            : 3;
      unsigned CONV_OUT0_C0_MODE          : 1;		// bits : 4
      unsigned                            : 3;
      unsigned CONV_W_C0_MODE             : 2;		// bits : 9_8
      unsigned                            : 2;
      unsigned CONV_IN0_NC_LAYOUT         : 1;		// bits : 12
      unsigned                            : 3;
      unsigned CONV_OUT0_NC_LAYOUT        : 1;		// bits : 16
    } Bit;
    UINT32 Word;
  } CONV_Register_130; // 0x0208

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_131; // 0x020c

  union
  {
    struct
    {
      unsigned CONV_CONV_KERNEL_W        : 8;		// bits : 7_0
      unsigned CONV_CONV_KERNEL_H        : 8;		// bits : 15_8
      unsigned CONV_CONV_DILATE_W        : 8;		// bits : 23_16
      unsigned CONV_CONV_DILATE_H        : 8;		// bits : 31_24
    } Bit;
    UINT32 Word;
  } CONV_Register_132; // 0x0210

  union
  {
    struct
    {
      unsigned CONV_CONV_STRIDE_W           : 8;		// bits : 7_0
      unsigned CONV_CONV_STRIDE_H           : 8;		// bits : 15_8
      unsigned CONV_CONV_OUT_CHANNEL        : 16;		// bits : 31_16
    } Bit;
    UINT32 Word;
  } CONV_Register_133; // 0x0214

  union
  {
    struct
    {
      unsigned CONV_CONV_KERNEL_D        : 7;		// bits : 6_0
      unsigned                           : 1;
      unsigned CONV_CONV_DILATE_D        : 7;		// bits : 14_8
      unsigned                           : 1;
      unsigned CONV_CONV_STRIDE_D        : 7;		// bits : 22_16
    } Bit;
    UINT32 Word;
  } CONV_Register_134; // 0x0218

  union
  {
    struct
    {
      unsigned CONV_CONV_TOP_PAD_NUM           : 8;		// bits : 7_0
      unsigned CONV_CONV_BOTTOM_PAD_NUM        : 8;		// bits : 15_8
      unsigned CONV_CONV_LEFT_PAD_NUM          : 8;		// bits : 23_16
      unsigned CONV_CONV_RIGHT_PAD_NUM         : 8;		// bits : 31_24
    } Bit;
    UINT32 Word;
  } CONV_Register_135; // 0x021c

  union
  {
    struct
    {
      unsigned CONV_CONV_PAD_VAL_LSB          : 16;		// bits : 15_0
      unsigned CONV_CONV_FRONT_PAD_NUM        : 8;		// bits : 23_16
      unsigned CONV_CONV_BACK_PAD_NUM         : 8;		// bits : 31_24
    } Bit;
    UINT32 Word;
  } CONV_Register_136; // 0x0220

  union
  {
    struct
    {
      unsigned CONV_CONV_MAC_PARALLELISM        : 3;		// bits : 2_0
      unsigned                                  : 1;
      unsigned CONV_CONV_MAC_MODE               : 1;		// bits : 4
      unsigned                                  : 3;
      unsigned CONV_CONV_ACC_PRECISION          : 1;		// bits : 8
      unsigned                                  : 3;
      unsigned CONV_CONV_ACC_SHIFT              : 5;		// bits : 16_12
      unsigned                                  : 3;
      unsigned CONV_CONV_ACC_OUT_SHIFT          : 6;		// bits : 25_20
    } Bit;
    UINT32 Word;
  } CONV_Register_137; // 0x0224

  union
  {
    struct
    {
      unsigned CONV_CONV_PAD_VAL_MSB        : 1;		// bits : 0
    } Bit;
    UINT32 Word;
  } CONV_Register_138; // 0x0228

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_139; // 0x022c

  union
  {
    struct
    {
      unsigned CONV_BIAS_SHIFT                : 5;		// bits : 4_0
      unsigned                                : 3;
      unsigned CONV_BIAS_OUT_SHIFT_DIR        : 1;		// bits : 8
      unsigned                                : 3;
      unsigned CONV_BIAS_OUT_SHIFT            : 6;		// bits : 17_12
    } Bit;
    UINT32 Word;
  } CONV_Register_140; // 0x0230

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_141; // 0x0234

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_142; // 0x0238

  union
  {
    struct
    {
      unsigned CONV_BN_MEAN_SHIFT        : 5;		// bits : 4_0
      unsigned                           : 11;
      unsigned CONV_BN_BETA_SHIFT        : 6;		// bits : 21_16
      unsigned                           : 2;
      unsigned CONV_BN_OUT_SHIFT         : 6;		// bits : 29_24
    } Bit;
    UINT32 Word;
  } CONV_Register_143; // 0x023c

  union
  {
    struct
    {
      unsigned                        : 1;
      unsigned CONV_BN_OUT_DIR        : 1;		// bits : 1
    } Bit;
    UINT32 Word;
  } CONV_Register_144; // 0x0240

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_145; // 0x0244

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_146; // 0x0248

  union
  {
    struct
    {
      unsigned CONV_ELT_COEFF0            : 16;		// bits : 15_0
      unsigned                            : 4;
      unsigned CONV_ELT_SHIFT0            : 6;		// bits : 25_20
      unsigned                            : 5;
      unsigned CONV_ELT_SHIFT0_DIR        : 1;		// bits : 31
    } Bit;
    UINT32 Word;
  } CONV_Register_147; // 0x024c

  union
  {
    struct
    {
      unsigned CONV_ELT_COEFF1            : 16;		// bits : 15_0
      unsigned                            : 4;
      unsigned CONV_ELT_SHIFT1            : 6;		// bits : 25_20
      unsigned                            : 5;
      unsigned CONV_ELT_SHIFT1_DIR        : 1;		// bits : 31
    } Bit;
    UINT32 Word;
  } CONV_Register_148; // 0x0250

  union
  {
    struct
    {
      unsigned CONV_ELT_OUT_SHIFT            : 6;		// bits : 5_0
      unsigned                               : 2;
      unsigned CONV_ELT_OUT_SHIFT_DIR        : 1;		// bits : 8
    } Bit;
    UINT32 Word;
  } CONV_Register_149; // 0x0254

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_150; // 0x0258

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_151; // 0x025c

  union
  {
    struct
    {
      unsigned CONV_ACT_CONDI0                    : 1;		// bits : 0
      unsigned CONV_ACT_CONDI1                    : 1;		// bits : 1
      unsigned                                    : 2;
      unsigned CONV_ACT_STATE0_EN                 : 1;		// bits : 4
      unsigned CONV_ACT_STATE1_EN                 : 1;		// bits : 5
      unsigned CONV_ACT_STATE2_EN                 : 1;		// bits : 6
      unsigned                                    : 1;
      unsigned CONV_ACT_LUT_EN                    : 1;		// bits : 8
      unsigned                                    : 3;
      unsigned CONV_ACT_LUT_SYMMETRIC_MODE        : 1;		// bits : 12
      unsigned                                    : 3;
      unsigned CONV_ACT_LUT_FMT                   : 1;		// bits : 16
    } Bit;
    UINT32 Word;
  } CONV_Register_152; // 0x0260

  union
  {
    struct
    {
      unsigned CONV_ACT_CONDI_UB        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_153; // 0x0264

  union
  {
    struct
    {
      unsigned CONV_ACT_CONDI_LB        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_154; // 0x0268

  union
  {
    struct
    {
      unsigned CONV_ACT_A0_MUL          : 16;		// bits : 15_0
      unsigned                          : 8;
      unsigned CONV_ACT_A0_SHIFT        : 6;		// bits : 29_24
    } Bit;
    UINT32 Word;
  } CONV_Register_155; // 0x026c

  union
  {
    struct
    {
      unsigned CONV_ACT_B0              : 16;		// bits : 15_0
      unsigned                          : 8;
      unsigned CONV_ACT_B0_SHIFT        : 6;		// bits : 29_24
    } Bit;
    UINT32 Word;
  } CONV_Register_156; // 0x0270

  union
  {
    struct
    {
      unsigned CONV_ACT_A1_MUL          : 16;		// bits : 15_0
      unsigned                          : 8;
      unsigned CONV_ACT_A1_SHIFT        : 6;		// bits : 29_24
    } Bit;
    UINT32 Word;
  } CONV_Register_157; // 0x0274

  union
  {
    struct
    {
      unsigned CONV_ACT_B1              : 16;		// bits : 15_0
      unsigned                          : 8;
      unsigned CONV_ACT_B1_SHIFT        : 6;		// bits : 29_24
    } Bit;
    UINT32 Word;
  } CONV_Register_158; // 0x0278

  union
  {
    struct
    {
      unsigned CONV_ACT_C_MUL          : 16;		// bits : 15_0
      unsigned                         : 8;
      unsigned CONV_ACT_C_SHIFT        : 6;		// bits : 29_24
    } Bit;
    UINT32 Word;
  } CONV_Register_159; // 0x027c

  union
  {
    struct
    {
      unsigned CONV_ACT_D              : 16;		// bits : 15_0
      unsigned                         : 8;
      unsigned CONV_ACT_D_SHIFT        : 6;		// bits : 29_24
    } Bit;
    UINT32 Word;
  } CONV_Register_160; // 0x0280

  union
  {
    struct
    {
      unsigned CONV_ACT_OUT0_SHIFT            : 6;		// bits : 5_0
      unsigned                                : 2;
      unsigned CONV_ACT_OUT1_SHIFT            : 6;		// bits : 13_8
      unsigned                                : 2;
      unsigned CONV_ACT_OUT2_SHIFT            : 6;		// bits : 21_16
      unsigned                                : 2;
      unsigned CONV_ACT_OUT0_SHIFT_DIR        : 1;		// bits : 24
      unsigned CONV_ACT_OUT1_SHIFT_DIR        : 1;		// bits : 25
      unsigned CONV_ACT_OUT2_SHIFT_DIR        : 1;		// bits : 26
    } Bit;
    UINT32 Word;
  } CONV_Register_161; // 0x0284

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_162; // 0x0288

  union
  {
    struct
    {
      unsigned ACT_LUT_CHKSUM        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_163; // 0x028c

  union
  {
    struct
    {
      unsigned CONV_ACT_TOTAL_ENTRY_NUM             : 7;		// bits : 6_0
      unsigned                                      : 9;
      unsigned CONV_ACT_LUT1_START_ENTRY_IDX        : 7;		// bits : 22_16
    } Bit;
    UINT32 Word;
  } CONV_Register_164; // 0x0290

  union
  {
    struct
    {
      unsigned CONV_ACT_LUT2_START_ENTRY_IDX        : 7;		// bits : 6_0
    } Bit;
    UINT32 Word;
  } CONV_Register_165; // 0x0294

  union
  {
    struct
    {
      unsigned CONV_ACT_LUT_BOUNDARY0        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_166; // 0x0298

  union
  {
    struct
    {
      unsigned CONV_ACT_LUT_BOUNDARY1        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_167; // 0x029c

  union
  {
    struct
    {
      unsigned CONV_ACT_LUT_BOUNDARY2        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_168; // 0x02a0

  union
  {
    struct
    {
      unsigned CONV_ACT_LUT_BOUNDARY3        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_169; // 0x02a4

  union
  {
    struct
    {
      unsigned CONV_ACT_LUT_IN_SHIFT            : 5;		// bits : 4_0
      unsigned                                  : 19;
      unsigned CONV_ACT_LUT_IN_SHIFT_DIR        : 1;		// bits : 24
    } Bit;
    UINT32 Word;
  } CONV_Register_170; // 0x02a8

  union
  {
    struct
    {
      unsigned CONV_ACT_LUT0_OUT_SHIFT            : 5;		// bits : 4_0
      unsigned                                    : 3;
      unsigned CONV_ACT_LUT1_OUT_SHIFT            : 5;		// bits : 12_8
      unsigned                                    : 3;
      unsigned CONV_ACT_LUT2_OUT_SHIFT            : 5;		// bits : 20_16
      unsigned                                    : 3;
      unsigned CONV_ACT_LUT0_OUT_SHIFT_DIR        : 1;		// bits : 24
      unsigned CONV_ACT_LUT1_OUT_SHIFT_DIR        : 1;		// bits : 25
      unsigned CONV_ACT_LUT2_OUT_SHIFT_DIR        : 1;		// bits : 26
    } Bit;
    UINT32 Word;
  } CONV_Register_171; // 0x02ac

  union
  {
    struct
    {
      unsigned CONV_ACT_LUT0_GAP_BIT_NUM        : 5;		// bits : 4_0
      unsigned                                  : 3;
      unsigned CONV_ACT_LUT1_GAP_BIT_NUM        : 5;		// bits : 12_8
      unsigned                                  : 3;
      unsigned CONV_ACT_LUT2_GAP_BIT_NUM        : 5;		// bits : 20_16
    } Bit;
    UINT32 Word;
  } CONV_Register_172; // 0x02b0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_173; // 0x02b4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_174; // 0x02b8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_175; // 0x02bc

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_176; // 0x02c0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_177; // 0x02c4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_178; // 0x02c8

  union
  {
    struct
    {
      unsigned CONV_POOL_OP                       : 1;		// bits : 0
      unsigned                                    : 3;
      unsigned CONV_POOL_KERNEL_W                 : 2;		// bits : 5_4
      unsigned                                    : 2;
      unsigned CONV_POOL_KERNEL_H                 : 2;		// bits : 9_8
      unsigned                                    : 2;
      unsigned CONV_POOL_STRIDE_W                 : 2;		// bits : 13_12
      unsigned                                    : 2;
      unsigned CONV_POOL_STRIDE_H                 : 2;		// bits : 17_16
      unsigned                                    : 2;
      unsigned CONV_POOL_ROUND_MODE               : 1;		// bits : 20
      unsigned CONV_POOL_COUNT_INCLUDE_PAD        : 1;		// bits : 21
      unsigned CONV_POOL_DIVISOR_OVERRIDE         : 1;		// bits : 22
    } Bit;
    UINT32 Word;
  } CONV_Register_179; // 0x02cc

  union
  {
    struct
    {
      unsigned CONV_POOL_TOP_PAD_NUM           : 1;		// bits : 0
      unsigned                                 : 3;
      unsigned CONV_POOL_LEFT_PAD_NUM          : 1;		// bits : 4
      unsigned                                 : 3;
      unsigned CONV_POOL_BOTTOM_PAD_NUM        : 1;		// bits : 8
      unsigned                                 : 3;
      unsigned CONV_POOL_RIGHT_PAD_NUM         : 1;		// bits : 12
      unsigned                                 : 4;
      unsigned CONV_POOL_OUT_SHIFT_DIR         : 1;		// bits : 17
    } Bit;
    UINT32 Word;
  } CONV_Register_180; // 0x02d0

  union
  {
    struct
    {
      unsigned CONV_POOL_AVG_MUL        : 16;		// bits : 15_0
    } Bit;
    UINT32 Word;
  } CONV_Register_181; // 0x02d4

  union
  {
    struct
    {
      unsigned                            : 8;
      unsigned CONV_POOL_OUT_SHIFT        : 6;		// bits : 13_8
    } Bit;
    UINT32 Word;
  } CONV_Register_182; // 0x02d8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_183; // 0x02dc

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_184; // 0x02e0

  union
  {
    struct
    {
      unsigned CONV_DW_KERNEL_W        : 2;		// bits : 1_0
      unsigned                         : 2;
      unsigned CONV_DW_KERNEL_H        : 2;		// bits : 5_4
      unsigned                         : 2;
      unsigned CONV_DW_STRIDE_W        : 2;		// bits : 9_8
      unsigned                         : 2;
      unsigned CONV_DW_STRIDE_H        : 2;		// bits : 13_12
    } Bit;
    UINT32 Word;
  } CONV_Register_185; // 0x02e4

  union
  {
    struct
    {
      unsigned CONV_DW_TOP_PAD_NUM           : 1;		// bits : 0
      unsigned                               : 7;
      unsigned CONV_DW_BOTTOM_PAD_NUM        : 1;		// bits : 8
      unsigned                               : 7;
      unsigned CONV_DW_LEFT_PAD_NUM          : 1;		// bits : 16
      unsigned                               : 7;
      unsigned CONV_DW_RIGHT_PAD_NUM         : 1;		// bits : 24
    } Bit;
    UINT32 Word;
  } CONV_Register_186; // 0x02e8

  union
  {
    struct
    {
      unsigned                                  : 8;
      unsigned CONV_DW_ACC_OUT_SHIFT            : 5;		// bits : 12_8
      unsigned                                  : 15;
      unsigned CONV_DW_ACC_OUT_SHIFT_DIR        : 1;		// bits : 28
    } Bit;
    UINT32 Word;
  } CONV_Register_187; // 0x02ec

  union
  {
    struct
    {
      unsigned CONV_DW_BIAS_SHIFT                : 5;		// bits : 4_0
      unsigned                                   : 3;
      unsigned CONV_DW_BIAS_OUT_SHIFT_DIR        : 1;		// bits : 8
      unsigned                                   : 3;
      unsigned CONV_DW_BIAS_OUT_SHIFT            : 6;		// bits : 17_12
    } Bit;
    UINT32 Word;
  } CONV_Register_188; // 0x02f0

  union
  {
    struct
    {
      unsigned CONV_DW_RELU_MUL              : 16;		// bits : 15_0
      unsigned                               : 8;
      unsigned CONV_DW_RELU_SHIFT            : 6;		// bits : 29_24
      unsigned                               : 1;
      unsigned CONV_DW_RELU_SHIFT_DIR        : 1;		// bits : 31
    } Bit;
    UINT32 Word;
  } CONV_Register_189; // 0x02f4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_190; // 0x02f8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_191; // 0x02fc

  union
  {
    struct
    {
      unsigned CONV_QUAN_IN0_OFS        : 16;		// bits : 15_0
      unsigned CONV_QUAN_IN1_OFS        : 16;		// bits : 31_16
    } Bit;
    UINT32 Word;
  } CONV_Register_192; // 0x0300

  union
  {
    struct
    {
      unsigned CONV_QUAN_OUT0_OFS        : 16;		// bits : 15_0
      unsigned CONV_QUAN_W_OFS           : 16;		// bits : 31_16
    } Bit;
    UINT32 Word;
  } CONV_Register_193; // 0x0304

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_194; // 0x0308

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_195; // 0x030c

  union
  {
    struct
    {
      unsigned CONV_QUAN_CONVOUT_SHIFT_DIR        : 1;		// bits : 0
      unsigned                                    : 3;
      unsigned CONV_QUAN_CONVOUT_SHIFT            : 6;		// bits : 9_4
      unsigned                                    : 2;
      unsigned CONV_QUAN_CONVOUT_SCALE            : 16;		// bits : 27_12
    } Bit;
    UINT32 Word;
  } CONV_Register_196; // 0x0310

  union
  {
    struct
    {
      unsigned CONV_QUAN_OUT0_SHIFT_DIR        : 1;		// bits : 0
      unsigned                                 : 3;
      unsigned CONV_QUAN_OUT0_SHIFT            : 6;		// bits : 9_4
      unsigned                                 : 2;
      unsigned CONV_QUAN_OUT0_SCALE            : 16;		// bits : 27_12
    } Bit;
    UINT32 Word;
  } CONV_Register_197; // 0x0314

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_198; // 0x0318

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_199; // 0x031c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_200; // 0x0320

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_201; // 0x0324

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_202; // 0x0328

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_203; // 0x032c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_204; // 0x0330

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_205; // 0x0334

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_206; // 0x0338

  union
  {
    struct
    {
      unsigned CONV_WCD_BITSTREAM_LENGTH_0_OCH0        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_207; // 0x033c

  union
  {
    struct
    {
      unsigned CONV_WCD_BITSTREAM_LENGTH_1_OCH0        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_208; // 0x0340

  union
  {
    struct
    {
      unsigned CONV_WCD_BITSTREAM_LENGTH_0_OCH1        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_209; // 0x0344

  union
  {
    struct
    {
      unsigned CONV_WCD_BITSTREAM_LENGTH_1_OCH1        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_210; // 0x0348

  union
  {
    struct
    {
      unsigned CONV_WCD_BITSTREAM_LENGTH_0_OCH2        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_211; // 0x034c

  union
  {
    struct
    {
      unsigned CONV_WCD_BITSTREAM_LENGTH_1_OCH2        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_212; // 0x0350

  union
  {
    struct
    {
      unsigned CONV_WCD_BITSTREAM_LENGTH_0_OCH3        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_213; // 0x0354

  union
  {
    struct
    {
      unsigned CONV_WCD_BITSTREAM_LENGTH_1_OCH3        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_214; // 0x0358

  union
  {
    struct
    {
      unsigned CONV_WCD_VLC_TBL_BASE_0           : 16;		// bits : 15_0
      unsigned                                   : 8;
      unsigned CONV_WCD_VLC_TBL_BIT_LEN_0        : 5;		// bits : 28_24
    } Bit;
    UINT32 Word;
  } CONV_Register_215; // 0x035c

  union
  {
    struct
    {
      unsigned CONV_WCD_VLC_TBL_BASE_1           : 16;		// bits : 15_0
      unsigned                                   : 8;
      unsigned CONV_WCD_VLC_TBL_BIT_LEN_1        : 5;		// bits : 28_24
    } Bit;
    UINT32 Word;
  } CONV_Register_216; // 0x0360

  union
  {
    struct
    {
      unsigned CONV_WCD_VLC_TBL_BASE_2           : 16;		// bits : 15_0
      unsigned                                   : 8;
      unsigned CONV_WCD_VLC_TBL_BIT_LEN_2        : 5;		// bits : 28_24
    } Bit;
    UINT32 Word;
  } CONV_Register_217; // 0x0364

  union
  {
    struct
    {
      unsigned CONV_WCD_VLC_TBL_BASE_3           : 16;		// bits : 15_0
      unsigned                                   : 8;
      unsigned CONV_WCD_VLC_TBL_BIT_LEN_3        : 5;		// bits : 28_24
    } Bit;
    UINT32 Word;
  } CONV_Register_218; // 0x0368

  union
  {
    struct
    {
      unsigned CONV_WCD_VLC_TBL_BASE_4           : 16;		// bits : 15_0
      unsigned                                   : 8;
      unsigned CONV_WCD_VLC_TBL_BIT_LEN_4        : 5;		// bits : 28_24
    } Bit;
    UINT32 Word;
  } CONV_Register_219; // 0x036c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_220; // 0x0370

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_221; // 0x0374

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_222; // 0x0378

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_223; // 0x037c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_224; // 0x0380

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_225; // 0x0384

  union
  {
    struct
    {
      unsigned HIDDEN_CONV_DRAM_BW: 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_226; // 0x0388

  union
  {
    struct
    {
      unsigned HIDDEN_CONV_UB_BW  : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_227; // 0x038c

  union
  {
    struct
    {
      unsigned CONV_ENG_CYCLE        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_228; // 0x0390

  union
  {
    struct
    {
      unsigned CONV_LL_CYCLE        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_229; // 0x0394

  union
  {
    struct
    {
      unsigned CONV_WAIT_DMA_CYCLE        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_230; // 0x0398

  union
  {
    struct
    {
      unsigned CONV_WAIT_UB_CYCLE        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_231; // 0x039c

  union
  {
    struct
    {
      unsigned CONV_WAIT_DRAM_CYCLE        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_232; // 0x03a0

  union
  {
    struct
    {
      unsigned LL_TABLE_IDX0_3        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_233; // 0x03a4

  union
  {
    struct
    {
      unsigned LL_TABLE_IDX4_7        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_234; // 0x03a8

  union
  {
    struct
    {
      unsigned LL_TABLE_IDX8_11        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_235; // 0x03ac

  union
  {
    struct
    {
      unsigned LL_TABLE_IDX12_15        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_236; // 0x03b0

  union
  {
    struct
    {
      unsigned DMA_DISABLE        : 1;		// bits : 0
      unsigned                    : 14;
      unsigned CONV_IDLE          : 1;		// bits : 15
    } Bit;
    UINT32 Word;
  } CONV_Register_237; // 0x03b4

  union
  {
    struct
    {
      unsigned CHECK_SUM_ENABLE        : 1;		// bits : 0
      unsigned CYCLE_COUNT_EN          : 1;		// bits : 1
      unsigned                         : 2;
      unsigned DBG_PORT_SEL            : 2;		// bits : 5_4
    } Bit;
    UINT32 Word;
  } CONV_Register_238; // 0x03b8

  union
  {
    struct
    {
      unsigned DMACH0_BURST_LENGTH        : 2;		// bits : 1_0
      unsigned                            : 2;
      unsigned DMACH1_BURST_LENGTH        : 2;		// bits : 5_4
      unsigned                            : 2;
      unsigned DMACH2_BURST_LENGTH        : 2;		// bits : 9_8
      unsigned                            : 2;
      unsigned DMACH3_BURST_LENGTH        : 2;		// bits : 13_12
      unsigned                            : 2;
      unsigned DMACH4_BURST_LENGTH        : 2;		// bits : 17_16
      unsigned                            : 2;
      unsigned DMACH5_BURST_LENGTH        : 2;		// bits : 21_20
      unsigned                            : 2;
      unsigned DMACH6_BURST_LENGTH        : 2;		// bits : 25_24
      unsigned                            : 2;
      unsigned DMACH7_BURST_LENGTH        : 2;		// bits : 29_28
    } Bit;
    UINT32 Word;
  } CONV_Register_239; // 0x03bc

  union
  {
    struct
    {
      unsigned DMACH8_BURST_LENGTH         : 2;		// bits : 1_0
      unsigned                             : 2;
      unsigned DMACH9_BURST_LENGTH         : 2;		// bits : 5_4
      unsigned                             : 14;
      unsigned DMACH13_BURST_LENGTH        : 2;		// bits : 21_20
      unsigned                             : 2;
      unsigned DMACH14_BURST_LENGTH        : 2;		// bits : 25_24
      unsigned                             : 2;
      unsigned DMACH15_BURST_LENGTH        : 2;		// bits : 29_28
    } Bit;
    UINT32 Word;
  } CONV_Register_240; // 0x03c0

  union
  {
    struct
    {
      unsigned DMACH16_BURST_LENGTH        : 2;		// bits : 1_0
      unsigned                             : 2;
      unsigned DMACH17_BURST_LENGTH        : 2;		// bits : 5_4
    } Bit;
    UINT32 Word;
  } CONV_Register_241; // 0x03c4

  union
  {
    struct
    {
      unsigned DMACH0_MAX_OUTSTANDING_NUM        : 8;		// bits : 7_0
      unsigned DMACH1_MAX_OUTSTANDING_NUM        : 8;		// bits : 15_8
    } Bit;
    UINT32 Word;
  } CONV_Register_242; // 0x03c8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_243; // 0x03cc

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_244; // 0x03d0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_245; // 0x03d4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_246; // 0x03d8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_247; // 0x03dc

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_248; // 0x03e0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_249; // 0x03e4

  union
  {
    struct
    {
      unsigned DMACH0_EN        : 1;		// bits : 0
      unsigned                  : 3;
      unsigned DMACH1_EN        : 1;		// bits : 4
      unsigned                  : 3;
      unsigned DMACH2_EN        : 1;		// bits : 8
      unsigned                  : 3;
      unsigned DMACH3_EN        : 1;		// bits : 12
      unsigned                  : 3;
      unsigned DMACH4_EN        : 1;		// bits : 16
      unsigned                  : 3;
      unsigned DMACH5_EN        : 1;		// bits : 20
      unsigned                  : 3;
      unsigned DMACH6_EN        : 1;		// bits : 24
      unsigned                  : 3;
      unsigned DMACH7_EN        : 1;		// bits : 28
    } Bit;
    UINT32 Word;
  } CONV_Register_250; // 0x03e8

  union
  {
    struct
    {
      unsigned DMACH8_EN         : 1;		// bits : 0
      unsigned                   : 3;
      unsigned DMACH9_EN         : 1;		// bits : 4
      unsigned                   : 15;
      unsigned DMACH13_EN        : 1;		// bits : 20
      unsigned                   : 3;
      unsigned DMACH14_EN        : 1;		// bits : 24
      unsigned                   : 3;
      unsigned DMACH15_EN        : 1;		// bits : 28
    } Bit;
    UINT32 Word;
  } CONV_Register_251; // 0x03ec

  union
  {
    struct
    {
      unsigned DMACH16_EN          : 1;		// bits : 0
      unsigned                     : 3;
      unsigned DMACH17_EN          : 1;		// bits : 4
      unsigned                     : 19;
      unsigned LLC_IN_CH_EN        : 1;		// bits : 24
    } Bit;
    UINT32 Word;
  } CONV_Register_252; // 0x03f0

  union
  {
    struct
    {
      unsigned DMACH0_LOCK_DISABLE        : 1;		// bits : 0
      unsigned                            : 3;
      unsigned DMACH1_LOCK_DISABLE        : 1;		// bits : 4
      unsigned                            : 3;
      unsigned DMACH2_LOCK_DISABLE        : 1;		// bits : 8
      unsigned                            : 3;
      unsigned DMACH3_LOCK_DISABLE        : 1;		// bits : 12
      unsigned                            : 3;
      unsigned DMACH4_LOCK_DISABLE        : 1;		// bits : 16
      unsigned                            : 3;
      unsigned DMACH5_LOCK_DISABLE        : 1;		// bits : 20
      unsigned                            : 3;
      unsigned DMACH6_LOCK_DISABLE        : 1;		// bits : 24
      unsigned                            : 3;
      unsigned DMACH7_LOCK_DISABLE        : 1;		// bits : 28
    } Bit;
    UINT32 Word;
  } CONV_Register_253; // 0x03f4

  union
  {
    struct
    {
      unsigned DMACH8_LOCK_DISABLE         : 1;		// bits : 0
      unsigned                             : 3;
      unsigned DMACH9_LOCK_DISABLE         : 1;		// bits : 4
      unsigned                             : 15;
      unsigned DMACH13_LOCK_DISABLE        : 1;		// bits : 20
      unsigned                             : 3;
      unsigned DMACH14_LOCK_DISABLE        : 1;		// bits : 24
      unsigned                             : 3;
      unsigned DMACH15_LOCK_DISABLE        : 1;		// bits : 28
    } Bit;
    UINT32 Word;
  } CONV_Register_254; // 0x03f8

  union
  {
    struct
    {
      unsigned DMACH16_LOCK_DISABLE        : 1;		// bits : 0
      unsigned                             : 3;
      unsigned DMACH17_LOCK_DISABLE        : 1;		// bits : 4
      unsigned                             : 19;
      unsigned LLC_IN_LOCK_DISABLE         : 1;		// bits : 24
    } Bit;
    UINT32 Word;
  } CONV_Register_255; // 0x03fc

  union
  {
    struct
    {
      unsigned WRITE_CH_OUTSTANDING_NUM        : 8;		// bits : 7_0
      unsigned READ_CH_OUTSTANDING_NUM         : 8;		// bits : 15_8
    } Bit;
    UINT32 Word;
  } CONV_Register_256; // 0x0400

  union
  {
    struct
    {
      unsigned AXI_BUS_DISABLE        : 1;		// bits : 0
      unsigned                        : 15;
      unsigned AXI_BUS_IDLE           : 1;		// bits : 16
    } Bit;
    UINT32 Word;
  } CONV_Register_257; // 0x0404

  union
  {
    struct
    {
      unsigned AXI_STATUS        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_258; // 0x0408

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_259; // 0x040c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_260; // 0x0410

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_261; // 0x0414

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_262; // 0x0418

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_263; // 0x041c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_264; // 0x0420

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_265; // 0x0424

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_266; // 0x0428

  union
  {
    struct
    {
      unsigned LLC_ERR_CMD_ADR0        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_267; // 0x042c

  union
  {
    struct
    {
      unsigned LLC_ERR_CMD_ADR1        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } CONV_Register_268; // 0x0430

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_269; // 0x0434

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_270; // 0x0438

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_271; // 0x043c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_272; // 0x0440

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_273; // 0x0444

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_274; // 0x0448

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_275; // 0x044c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_276; // 0x0450

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_277; // 0x0454

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_278; // 0x0458

  union
  {
    struct
    {
      unsigned CONV_CHKSUM        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_279; // 0x045c

  union
  {
    struct
    {
      unsigned QUAN_CONVOUT_CHKSUM        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_280; // 0x0460

  union
  {
    struct
    {
      unsigned BIAS_CHKSUM        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_281; // 0x0464

  union
  {
    struct
    {
      unsigned BN_CHKSUM        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_282; // 0x0468

  union
  {
    struct
    {
      unsigned ELTWISE_CHKSUM        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_283; // 0x046c

  union
  {
    struct
    {
      unsigned ACT_CHKSUM        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_284; // 0x0470

  union
  {
    struct
    {
      unsigned QUAN_OUT0_INPUT_CHKSUM        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_285; // 0x0474

  union
  {
    struct
    {
      unsigned QUAN_OUT0_CHKSUM        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_286; // 0x0478

  union
  {
    struct
    {
      unsigned WCD_CHKSUM        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_287; // 0x047c

  union
  {
    struct
    {
      unsigned ACT_LUT_HW_CUR_CHKSUM        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_288; // 0x0480

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_289; // 0x0484

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_290; // 0x0488

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_291; // 0x048c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_292; // 0x0490

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_293; // 0x0494

  union
  {
    struct
    {
      unsigned CONV_IN0_CUR_SRCBUF        : 1;		// bits : 0
    } Bit;
    UINT32 Word;
  } CONV_Register_294; // 0x0498

  union
  {
    struct
    {
      unsigned CONV_OUT0_WIDTH         : 12;		// bits : 11_0
      unsigned                         : 4;
      unsigned CONV_OUT0_HEIGHT        : 12;		// bits : 27_16
    } Bit;
    UINT32 Word;
  } CONV_Register_295; // 0x049c

  union
  {
    struct
    {
      unsigned CONV_OUT0_CHANNEL        : 16;		// bits : 15_0
      unsigned                          : 4;
      unsigned CONV_OUT0_BATCH          : 7;		// bits : 26_20
    } Bit;
    UINT32 Word;
  } CONV_Register_296; // 0x04a0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_297; // 0x04a4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_298; // 0x04a8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_299; // 0x04ac

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_300; // 0x04b0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_301; // 0x04b4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_302; // 0x04b8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_303; // 0x04bc

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_304; // 0x04c0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_305; // 0x04c4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_306; // 0x04c8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_307; // 0x04cc

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_308; // 0x04d0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_309; // 0x04d4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_310; // 0x04d8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_311; // 0x04dc

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_312; // 0x04e0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_313; // 0x04e4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_314; // 0x04e8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_315; // 0x04ec

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_316; // 0x04f0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_317; // 0x04f4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_318; // 0x04f8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_319; // 0x04fc

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_320; // 0x0500

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_321; // 0x0504

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_322; // 0x0508

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_323; // 0x050c

  union
  {
    struct
    {
      unsigned CONV_PRE_IN0_CONV_MODE        : 3;		// bits : 2_0
    } Bit;
    UINT32 Word;
  } CONV_Register_324; // 0x0510

  union
  {
    struct
    {
      unsigned                                   : 4;
      unsigned CONV_PRE_IN0_PERMUTE_WC_EN        : 1;		// bits : 4
      unsigned CONV_PRE_IN0_FLATTEN2CH_EN        : 1;		// bits : 5
    } Bit;
    UINT32 Word;
  } CONV_Register_325; // 0x0514

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_326; // 0x0518

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_327; // 0x051c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_328; // 0x0520

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_329; // 0x0524

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_330; // 0x0528

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_331; // 0x052c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_332; // 0x0530

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_333; // 0x0534

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_334; // 0x0538

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_335; // 0x053c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_336; // 0x0540

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_337; // 0x0544

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_338; // 0x0548

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_339; // 0x054c

  union
  {
    struct
    {
      unsigned DRAMUB_PRE_SAI0        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_340; // 0x0550

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_PRE_SAI0         : 4;		// bits : 3_0
      unsigned                             : 24;
      unsigned DRAMUB_MODE_PRE_SAI0        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CONV_Register_341; // 0x0554

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_342; // 0x0558

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_343; // 0x055c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_344; // 0x0560

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_345; // 0x0564

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_346; // 0x0568

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_347; // 0x056c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_348; // 0x0570

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_349; // 0x0574

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_350; // 0x0578

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_351; // 0x057c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_352; // 0x0580

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_353; // 0x0584

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_354; // 0x0588

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_355; // 0x058c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_356; // 0x0590

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_357; // 0x0594

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_358; // 0x0598

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_359; // 0x059c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_360; // 0x05a0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_361; // 0x05a4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_362; // 0x05a8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_363; // 0x05ac

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_364; // 0x05b0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_365; // 0x05b4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_366; // 0x05b8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_367; // 0x05bc

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_368; // 0x05c0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_369; // 0x05c4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_370; // 0x05c8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_371; // 0x05cc

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_372; // 0x05d0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_373; // 0x05d4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_374; // 0x05d8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_375; // 0x05dc

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_376; // 0x05e0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_377; // 0x05e4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_378; // 0x05e8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_379; // 0x05ec

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_380; // 0x05f0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_381; // 0x05f4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_382; // 0x05f8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_383; // 0x05fc

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_384; // 0x0600

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_385; // 0x0604

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_386; // 0x0608

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_387; // 0x060c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_388; // 0x0610

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_389; // 0x0614

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_390; // 0x0618

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_391; // 0x061c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_392; // 0x0620

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_393; // 0x0624

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_394; // 0x0628

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_395; // 0x062c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_396; // 0x0630

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_397; // 0x0634

  union
  {
    struct
    {
      unsigned CONV_PRE_IN0_LOFS_EN         : 1;		// bits : 0
      unsigned CONV_PRE_IN0_CHOFS_EN        : 1;		// bits : 1
      unsigned CONV_PRE_IN0_BOFS_EN         : 1;		// bits : 2
    } Bit;
    UINT32 Word;
  } CONV_Register_398; // 0x0638

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_399; // 0x063c

  union
  {
    struct
    {
      unsigned CONV_PRE_IN0_LOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CONV_Register_400; // 0x0640

  union
  {
    struct
    {
      unsigned CONV_PRE_IN0_CHOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CONV_Register_401; // 0x0644

  union
  {
    struct
    {
      unsigned CONV_PRE_IN0_BOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CONV_Register_402; // 0x0648

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_403; // 0x064c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_404; // 0x0650

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_405; // 0x0654

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_406; // 0x0658

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_407; // 0x065c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_408; // 0x0660

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_409; // 0x0664

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_410; // 0x0668

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_411; // 0x066c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_412; // 0x0670

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_413; // 0x0674

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_414; // 0x0678

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_415; // 0x067c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_416; // 0x0680

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_417; // 0x0684

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_418; // 0x0688

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_419; // 0x068c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_420; // 0x0690

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_421; // 0x0694

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_422; // 0x0698

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_423; // 0x069c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_424; // 0x06a0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_425; // 0x06a4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_426; // 0x06a8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_427; // 0x06ac

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_428; // 0x06b0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_429; // 0x06b4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_430; // 0x06b8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_431; // 0x06bc

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_432; // 0x06c0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_433; // 0x06c4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_434; // 0x06c8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_435; // 0x06cc

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_436; // 0x06d0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_437; // 0x06d4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_438; // 0x06d8

  union
  {
    struct
    {
      unsigned CONV_PRE_IN0_WIDTH         : 12;		// bits : 11_0
      unsigned                            : 4;
      unsigned CONV_PRE_IN0_HEIGHT        : 12;		// bits : 27_16
    } Bit;
    UINT32 Word;
  } CONV_Register_439; // 0x06dc

  union
  {
    struct
    {
      unsigned CONV_PRE_IN0_CHANNEL        : 16;		// bits : 15_0
      unsigned                             : 4;
      unsigned CONV_PRE_IN0_BATCH          : 7;		// bits : 26_20
    } Bit;
    UINT32 Word;
  } CONV_Register_440; // 0x06e0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_441; // 0x06e4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_442; // 0x06e8

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_443; // 0x06ec

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_444; // 0x06f0

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_445; // 0x06f4

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_446; // 0x06f8

  union
  {
    struct
    {
      unsigned CONV_PRE_IN0_BIT_DEPTH        : 2;		// bits : 1_0
      unsigned                               : 2;
      unsigned CONV_PRE_IN0_DATA_FMT         : 1;		// bits : 4
    } Bit;
    UINT32 Word;
  } CONV_Register_447; // 0x06fc

  union
  {
    struct
    {
      unsigned CONV_PRE_IN0_W_BIT_DEPTH        : 2;		// bits : 1_0
    } Bit;
    UINT32 Word;
  } CONV_Register_448; // 0x0700

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_449; // 0x0704

  union
  {
    struct
    {
      unsigned CONV_PRE_IN0_C0_MODE          : 1;		// bits : 0
      unsigned                               : 11;
      unsigned CONV_PRE_IN0_NC_LAYOUT        : 1;		// bits : 12
    } Bit;
    UINT32 Word;
  } CONV_Register_450; // 0x0708

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_451; // 0x070c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_452; // 0x0710

  union
  {
    struct
    {
      unsigned CONV_PRE_IN0_CONV_STRIDE_W        : 8;		// bits : 7_0
    } Bit;
    UINT32 Word;
  } CONV_Register_453; // 0x0714

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_454; // 0x0718

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_455; // 0x071c

  union
  {
    struct
    {
      unsigned Reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CONV_Register_456; // 0x0720

  union
  {
    struct
    {
      unsigned CONV_PRE_IN0_CONV_MAC_PARALLELISM        : 3;		// bits : 2_0
    } Bit;
    UINT32 Word;
  } CONV_Register_457; // 0x0724

} NT98690_CONV_REGISTER_STRUCT;

#ifdef __cplusplus
}
#endif


#endif
