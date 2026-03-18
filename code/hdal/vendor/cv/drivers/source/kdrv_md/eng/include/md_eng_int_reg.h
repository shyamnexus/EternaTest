#ifndef _MD_ENG_INT_REGISTER_H_
#define _MD_ENG_INT_REGISTER_H_

#if defined (__LINUX)
/*
#if defined(__aarch64__)
#include "linux/soc/nvt/rcw_macro.h"
#else
#include "rcw_macro.h"
#endif
*/
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

//#define OUTW(addr, value)    	nvt_writel(value, addr)
//#define INW(addr)           	nvt_readl(addr)

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
    MDBC_SW_RST:    [0x0, 0x1],			bits : 0
    MDBC_START :    [0x0, 0x1],			bits : 1
*/
#define MDBC_CONTROL_REGISTER_OFS 0x0000
REGDEF_BEGIN(MDBC_CONTROL_REGISTER)
    REGDEF_BIT(MDBC_SW_RST,        1)
    REGDEF_BIT(MDBC_START ,        1)
REGDEF_END(MDBC_CONTROL_REGISTER)


/*
    MDBC_MODE       :    [0x0, 0x1],			bits : 0
    BC_UPDATE_NEI_EN:    [0x0, 0x1],			bits : 4
    BC_DEGHOST_EN   :    [0x0, 0x1],			bits : 8
    ROI_EN0         :    [0x0, 0x1],			bits : 12
    ROI_EN1         :    [0x0, 0x1],			bits : 13
    ROI_EN2         :    [0x0, 0x1],			bits : 14
    ROI_EN3         :    [0x0, 0x1],			bits : 15
    ROI_EN4         :    [0x0, 0x1],			bits : 16
    ROI_EN5         :    [0x0, 0x1],			bits : 17
    ROI_EN6         :    [0x0, 0x1],			bits : 18
    ROI_EN7         :    [0x0, 0x1],			bits : 19
    CHKSUM_EN       :    [0x0, 0x1],			bits : 20
    BGMW_SAVE_BW_EN :    [0x0, 0x1],			bits : 24
    BC_Y_ONLY_EN    :    [0x0, 0x1],			bits : 28
    BC_MODEL_MODE   :    [0x0, 0x1],			bits : 29
    MD_OUT_BIT_DEPTH:    [0x0, 0x1],			bits : 30
*/
#define MDBC_MODE_REGISTER0_OFS 0x0004
REGDEF_BEGIN(MDBC_MODE_REGISTER0)
    REGDEF_BIT(MDBC_MODE       ,        1)
    REGDEF_BIT(                ,        3)
    REGDEF_BIT(BC_UPDATE_NEI_EN,        1)
    REGDEF_BIT(                ,        3)
    REGDEF_BIT(BC_DEGHOST_EN   ,        1)
    REGDEF_BIT(                ,        3)
    REGDEF_BIT(ROI_EN0         ,        1)
    REGDEF_BIT(ROI_EN1         ,        1)
    REGDEF_BIT(ROI_EN2         ,        1)
    REGDEF_BIT(ROI_EN3         ,        1)
    REGDEF_BIT(ROI_EN4         ,        1)
    REGDEF_BIT(ROI_EN5         ,        1)
    REGDEF_BIT(ROI_EN6         ,        1)
    REGDEF_BIT(ROI_EN7         ,        1)
    REGDEF_BIT(CHKSUM_EN       ,        1)
    REGDEF_BIT(                ,        3)
    REGDEF_BIT(BGMW_SAVE_BW_EN ,        1)
    REGDEF_BIT(                ,        3)
    REGDEF_BIT(BC_Y_ONLY_EN    ,        1)
    REGDEF_BIT(BC_MODEL_MODE   ,        1)
    REGDEF_BIT(MD_OUT_BIT_DEPTH,        1)
    REGDEF_BIT(MD_LBSP_DISABLE ,        1)
REGDEF_END(MDBC_MODE_REGISTER0)


/*
    INTE_FRM_END:    [0x0, 0x1],			bits : 0
*/
#define MDBC_INTERRUPT_ENABLE_REGISTER_OFS 0x0008
REGDEF_BEGIN(MDBC_INTERRUPT_ENABLE_REGISTER)
    REGDEF_BIT(INTE_FRM_END,        1)
REGDEF_END(MDBC_INTERRUPT_ENABLE_REGISTER)


/*
    INTS_FRM_END:    [0x0, 0x1],			bits : 0
*/
#define MDBC_INTERRUPT_STATUS_REGISTER_OFS 0x000c
REGDEF_BEGIN(MDBC_INTERRUPT_STATUS_REGISTER)
    REGDEF_BIT(INTS_FRM_END,        1)
REGDEF_END(MDBC_INTERRUPT_STATUS_REGISTER)


/*
    DRAM_SAI0:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define DMA_TO_MDBC_REGISTER0_OFS 0x0010
REGDEF_BEGIN(DMA_TO_MDBC_REGISTER0)
    REGDEF_BIT(         ,        2)
    REGDEF_BIT(DRAM_SAI0,        30)
REGDEF_END(DMA_TO_MDBC_REGISTER0)


/*
    DRAM_SAI1:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define DMA_TO_MDBC_REGISTER1_OFS 0x0014
REGDEF_BEGIN(DMA_TO_MDBC_REGISTER1)
    REGDEF_BIT(         ,        2)
    REGDEF_BIT(DRAM_SAI1,        30)
REGDEF_END(DMA_TO_MDBC_REGISTER1)


/*
    DRAM_OFSI0:    [0x0, 0x3ffff],			bits : 19_2
*/
#define DMA_TO_MDBC_REGISTER2_OFS 0x0018
REGDEF_BEGIN(DMA_TO_MDBC_REGISTER2)
    REGDEF_BIT(          ,        2)
    REGDEF_BIT(DRAM_OFSI0,        18)
REGDEF_END(DMA_TO_MDBC_REGISTER2)


/*
    DRAM_OFSI1:    [0x0, 0x3ffff],			bits : 19_2
*/
#define DMA_TO_MDBC_REGISTER3_OFS 0x001c
REGDEF_BEGIN(DMA_TO_MDBC_REGISTER3)
    REGDEF_BIT(          ,        2)
    REGDEF_BIT(DRAM_OFSI1,        18)
REGDEF_END(DMA_TO_MDBC_REGISTER3)


/*
    DRAM_SAI2:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define DMA_TO_MDBC_REGISTER4_OFS 0x0020
REGDEF_BEGIN(DMA_TO_MDBC_REGISTER4)
    REGDEF_BIT(         ,        2)
    REGDEF_BIT(DRAM_SAI2,        30)
REGDEF_END(DMA_TO_MDBC_REGISTER4)


/*
    DRAM_SAI3:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define DMA_TO_MDBC_REGISTER5_OFS 0x0024
REGDEF_BEGIN(DMA_TO_MDBC_REGISTER5)
    REGDEF_BIT(         ,        2)
    REGDEF_BIT(DRAM_SAI3,        30)
REGDEF_END(DMA_TO_MDBC_REGISTER5)


/*
    DRAM_SAI4:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define DMA_TO_MDBC_REGISTER6_OFS 0x0028
REGDEF_BEGIN(DMA_TO_MDBC_REGISTER6)
    REGDEF_BIT(         ,        2)
    REGDEF_BIT(DRAM_SAI4,        30)
REGDEF_END(DMA_TO_MDBC_REGISTER6)


/*
    DRAM_SAI5:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define DMA_TO_MDBC_REGISTER7_OFS 0x002c
REGDEF_BEGIN(DMA_TO_MDBC_REGISTER7)
    REGDEF_BIT(         ,        2)
    REGDEF_BIT(DRAM_SAI5,        30)
REGDEF_END(DMA_TO_MDBC_REGISTER7)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER3_OFS 0x0030
REGDEF_BEGIN(RESERVED_REGISTER3)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER3)


/*
    DRAM_SAO0:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define MDBC_TO_DMA_REGISTER0_OFS 0x0034
REGDEF_BEGIN(MDBC_TO_DMA_REGISTER0)
    REGDEF_BIT(         ,        2)
    REGDEF_BIT(DRAM_SAO0,        30)
REGDEF_END(MDBC_TO_DMA_REGISTER0)


/*
    DRAM_SAO1:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define MDBC_TO_DMA_REGISTER1_OFS 0x0038
REGDEF_BEGIN(MDBC_TO_DMA_REGISTER1)
    REGDEF_BIT(         ,        2)
    REGDEF_BIT(DRAM_SAO1,        30)
REGDEF_END(MDBC_TO_DMA_REGISTER1)


/*
    DRAM_SAO2:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define MDBC_TO_DMA_REGISTER2_OFS 0x003c
REGDEF_BEGIN(MDBC_TO_DMA_REGISTER2)
    REGDEF_BIT(         ,        2)
    REGDEF_BIT(DRAM_SAO2,        30)
REGDEF_END(MDBC_TO_DMA_REGISTER2)


/*
    DRAM_SAO3:    [0x0, 0x3fffffff],			bits : 31_2
*/
#define MDBC_TO_DMA_REGISTER3_OFS 0x0040
REGDEF_BEGIN(MDBC_TO_DMA_REGISTER3)
    REGDEF_BIT(         ,        2)
    REGDEF_BIT(DRAM_SAO3,        30)
REGDEF_END(MDBC_TO_DMA_REGISTER3)


/*
    IN_BURST_LENGTH :    [0x0, 0x1],			bits : 0
    OUT_BURST_LENGTH:    [0x0, 0x1],			bits : 1
    VERSION_CODE    :    [0x0, 0xffff],			bits : 31_16
*/
#define DESIGN_DEBUG_REGISTER0_OFS 0x0044
REGDEF_BEGIN(DESIGN_DEBUG_REGISTER0)
    REGDEF_BIT(IN_BURST_LENGTH ,        1)
    REGDEF_BIT(OUT_BURST_LENGTH,        1)
    REGDEF_BIT(                ,        14)
    REGDEF_BIT(VERSION_CODE    ,        16)
REGDEF_END(DESIGN_DEBUG_REGISTER0)


/*
    WIDTH :    [0x0, 0x1ff],			bits : 9_1
    HEIGHT:    [0x0, 0x1ff],			bits : 21_13
*/
#define INPUT_SIZE_REGISTER_OFS 0x0048
REGDEF_BEGIN(INPUT_SIZE_REGISTER)
    REGDEF_BIT(      ,        1)
    REGDEF_BIT(WIDTH ,        9)
    REGDEF_BIT(      ,        3)
    REGDEF_BIT(HEIGHT,        9)
REGDEF_END(INPUT_SIZE_REGISTER)


/*
    LBSP_TH :    [0x0, 0xff],			bits : 7_0
    D_COLOUR:    [0x0, 0xff],			bits : 15_8
    R_COLOUR:    [0x0, 0xff],			bits : 23_16
    D_LBSP  :    [0x0, 0xf],			bits : 27_24
    R_LBSP  :    [0x0, 0xf],			bits : 31_28
*/
#define MODEL_MATCH_REGISTER0_OFS 0x004c
REGDEF_BEGIN(MODEL_MATCH_REGISTER0)
    REGDEF_BIT(LBSP_TH ,        8)
    REGDEF_BIT(D_COLOUR,        8)
    REGDEF_BIT(R_COLOUR,        8)
    REGDEF_BIT(D_LBSP  ,        4)
    REGDEF_BIT(R_LBSP  ,        4)
REGDEF_END(MODEL_MATCH_REGISTER0)


/*
    BG_MODEL_NUM:    [0x0, 0xf],			bits : 3_0
    T_ALPHA     :    [0x0, 0xff],			bits : 11_4
    DW_SHIFT    :    [0x0, 0x7],			bits : 14_12
    D_LAST_ALPHA:    [0x0, 0x3ff],			bits : 25_16
*/
#define MODEL_MATCH_REGISTER1_OFS 0x0050
REGDEF_BEGIN(MODEL_MATCH_REGISTER1)
    REGDEF_BIT(BG_MODEL_NUM,        4)
    REGDEF_BIT(T_ALPHA     ,        8)
    REGDEF_BIT(DW_SHIFT    ,        3)
    REGDEF_BIT(BG_MODEL_NUM_EXT,    1)
    REGDEF_BIT(D_LAST_ALPHA,        10)
REGDEF_END(MODEL_MATCH_REGISTER1)


/*
    BC_MIN_MATCH:    [0x0, 0xf],			bits : 3_0
    DLT_ALPHA   :    [0x0, 0x3ff],			bits : 13_4
    DST_ALPHA   :    [0x0, 0x3ff],			bits : 25_16
*/
#define MODEL_MATCH_REGISTER2_OFS 0x0054
REGDEF_BEGIN(MODEL_MATCH_REGISTER2)
    REGDEF_BIT(BC_MIN_MATCH,        4)
    REGDEF_BIT(DLT_ALPHA   ,        10)
    REGDEF_BIT(            ,        2)
    REGDEF_BIT(DST_ALPHA   ,        10)
REGDEF_END(MODEL_MATCH_REGISTER2)


/*
    BC_UV_THRES:    [0x0, 0xff],			bits : 7_0
    S_ALPHA    :    [0x0, 0x3ff],			bits : 21_12
*/
#define MODEL_MATCH_REGISTER3_OFS 0x0058
REGDEF_BEGIN(MODEL_MATCH_REGISTER3)
    REGDEF_BIT(BC_UV_THRES,        8)
    REGDEF_BIT(           ,        4)
    REGDEF_BIT(S_ALPHA    ,        10)
REGDEF_END(MODEL_MATCH_REGISTER3)


/*
    DBG_LUM_DIFF   :    [0x0, 0xfffffff],			bits : 27_0
    DBG_LUM_DIFF_EN:    [0x0, 0x1],			bits : 28
*/
#define MODEL_MATCH_REGISTER4_OFS 0x005c
REGDEF_BEGIN(MODEL_MATCH_REGISTER4)
    REGDEF_BIT(DBG_LUM_DIFF   ,        28)
    REGDEF_BIT(DBG_LUM_DIFF_EN,        1)
REGDEF_END(MODEL_MATCH_REGISTER4)


/*
    LUM_DIFF:    [0x0, 0xfffffff],			bits : 27_0
*/
#define MODEL_MATCH_REGISTER5_OFS 0x0060
REGDEF_BEGIN(MODEL_MATCH_REGISTER5)
    REGDEF_BIT(LUM_DIFF,        28)
REGDEF_END(MODEL_MATCH_REGISTER5)


/*
    MOR_TH0       :    [0x0, 0xf],			bits : 3_0
    MOR_TH1       :    [0x0, 0xf],			bits : 7_4
    MOR_TH2       :    [0x0, 0xf],			bits : 11_8
    MOR_TH3       :    [0x0, 0xf],			bits : 15_12
    MOR_TH_DIL    :    [0x0, 0xf],			bits : 19_16
    MOR_SEL0      :    [0x0, 0x1],			bits : 20
    MOR_SEL1      :    [0x0, 0x1],			bits : 21
    MOR_SEL2      :    [0x0, 0x1],			bits : 22
    MOR_SEL3      :    [0x0, 0x1],			bits : 23
    BC_TEMPORAL_TH:    [0x0, 0xf],			bits : 27_24
*/
#define MORPHOLOGICAL_PROCESS_REGISTER_OFS 0x0064
REGDEF_BEGIN(MORPHOLOGICAL_PROCESS_REGISTER)
    REGDEF_BIT(MOR_TH0       ,        4)
    REGDEF_BIT(MOR_TH1       ,        4)
    REGDEF_BIT(MOR_TH2       ,        4)
    REGDEF_BIT(MOR_TH3       ,        4)
    REGDEF_BIT(MOR_TH_DIL    ,        4)
    REGDEF_BIT(MOR_SEL0      ,        1)
    REGDEF_BIT(MOR_SEL1      ,        1)
    REGDEF_BIT(MOR_SEL2      ,        1)
    REGDEF_BIT(MOR_SEL3      ,        1)
    REGDEF_BIT(BC_TEMPORAL_TH,        4)
REGDEF_END(MORPHOLOGICAL_PROCESS_REGISTER)


/*
    BC_MIN_T     :    [0x0, 0xff],			bits : 7_0
    BC_MAX_T     :    [0x0, 0xff],			bits : 15_8
    BC_MAX_FG_FRM:    [0x0, 0xff],			bits : 23_16
*/
#define UPDATE_REGISTER0_OFS 0x0068
REGDEF_BEGIN(UPDATE_REGISTER0)
    REGDEF_BIT(BC_MIN_T     ,        8)
    REGDEF_BIT(BC_MAX_T     ,        8)
    REGDEF_BIT(BC_MAX_FG_FRM,        8)
REGDEF_END(UPDATE_REGISTER0)


/*
    BC_DEGHOST_DTH:    [0x0, 0x1ff],			bits : 8_0
    BC_DEGHOST_STH:    [0x0, 0xff],			bits : 23_16
*/
#define UPDATE_REGISTER1_OFS 0x006c
REGDEF_BEGIN(UPDATE_REGISTER1)
    REGDEF_BIT(BC_DEGHOST_DTH,        9)
    REGDEF_BIT(              ,        7)
    REGDEF_BIT(BC_DEGHOST_STH,        8)
REGDEF_END(UPDATE_REGISTER1)


/*
    BC_STABLE_FRAME:    [0x0, 0xff],			bits : 7_0
    BC_UPDATE_DYN  :    [0x0, 0xff],			bits : 15_8
    BC_VA_DISTTH   :    [0x0, 0xff],			bits : 23_16
    BC_T_DISTTH    :    [0x0, 0xff],			bits : 31_24
*/
#define UPDATE_REGISTER2_OFS 0x0070
REGDEF_BEGIN(UPDATE_REGISTER2)
    REGDEF_BIT(BC_STABLE_FRAME,        8)
    REGDEF_BIT(BC_UPDATE_DYN  ,        8)
    REGDEF_BIT(BC_VA_DISTTH   ,        8)
    REGDEF_BIT(BC_T_DISTTH    ,        8)
REGDEF_END(UPDATE_REGISTER2)


/*
    DBG_FRM_ID   :    [0x0, 0xff],			bits : 7_0
    DBG_FRM_ID_EN:    [0x0, 0x1],			bits : 8
    FRM_ID       :    [0x0, 0xff],			bits : 19_12
*/
#define UPDATE_REGISTER3_OFS 0x0074
REGDEF_BEGIN(UPDATE_REGISTER3)
    REGDEF_BIT(DBG_FRM_ID   ,        8)
    REGDEF_BIT(DBG_FRM_ID_EN,        1)
    REGDEF_BIT(             ,        3)
    REGDEF_BIT(FRM_ID       ,        8)
REGDEF_END(UPDATE_REGISTER3)


/*
    DBG_RND   :    [0x0, 0x7fff],			bits : 14_0
    DBG_RND_EN:    [0x0, 0x1],			bits : 15
    RND       :    [0x0, 0x7fff],			bits : 30_16
*/
#define UPDATE_REGISTER4_OFS 0x0078
REGDEF_BEGIN(UPDATE_REGISTER4)
    REGDEF_BIT(DBG_RND   ,        15)
    REGDEF_BIT(DBG_RND_EN,        1)
    REGDEF_BIT(RND       ,        15)
REGDEF_END(UPDATE_REGISTER4)


/*
    ROI_X0:    [0x0, 0x3ff],			bits : 9_0
    ROI_Y0:    [0x0, 0x3ff],			bits : 21_12
*/
#define ROI0_REGISTER0_OFS 0x007c
REGDEF_BEGIN(ROI0_REGISTER0)
    REGDEF_BIT(ROI_X0,        10)
    REGDEF_BIT(      ,        2)
    REGDEF_BIT(ROI_Y0,        10)
REGDEF_END(ROI0_REGISTER0)


/*
    ROI_W0       :    [0x0, 0x3ff],			bits : 9_0
    ROI_H0       :    [0x0, 0x3ff],			bits : 21_12
    ROI_UV_THRES0:    [0x0, 0xff],			bits : 31_24
*/
#define ROI0_REGISTER1_OFS 0x0080
REGDEF_BEGIN(ROI0_REGISTER1)
    REGDEF_BIT(ROI_W0       ,        10)
    REGDEF_BIT(             ,        2)
    REGDEF_BIT(ROI_H0       ,        10)
    REGDEF_BIT(             ,        2)
    REGDEF_BIT(ROI_UV_THRES0,        8)
REGDEF_END(ROI0_REGISTER1)


/*
    ROI_LBSP_TH0 :    [0x0, 0xff],			bits : 7_0
    ROI_D_COLOUR0:    [0x0, 0xff],			bits : 15_8
    ROI_R_COLOUR0:    [0x0, 0xff],			bits : 23_16
    ROI_D_LBSP0  :    [0x0, 0xf],			bits : 27_24
    ROI_R_LBSP0  :    [0x0, 0xf],			bits : 31_28
*/
#define ROI0_REGISTER2_OFS 0x0084
REGDEF_BEGIN(ROI0_REGISTER2)
    REGDEF_BIT(ROI_LBSP_TH0 ,        8)
    REGDEF_BIT(ROI_D_COLOUR0,        8)
    REGDEF_BIT(ROI_R_COLOUR0,        8)
    REGDEF_BIT(ROI_D_LBSP0  ,        4)
    REGDEF_BIT(ROI_R_LBSP0  ,        4)
REGDEF_END(ROI0_REGISTER2)


/*
    ROI_MORPH_EN0:    [0x0, 0x1],			bits : 0
    ROI_MIN_T0   :    [0x0, 0xff],			bits : 11_4
    ROI_MAX_T0   :    [0x0, 0xff],			bits : 19_12
*/
#define ROI0_REGISTER3_OFS 0x0088
REGDEF_BEGIN(ROI0_REGISTER3)
    REGDEF_BIT(ROI_MORPH_EN0,        1)
    REGDEF_BIT(             ,        3)
    REGDEF_BIT(ROI_MIN_T0   ,        8)
    REGDEF_BIT(ROI_MAX_T0   ,        8)
REGDEF_END(ROI0_REGISTER3)


/*
    ROI_X1:    [0x0, 0x3ff],			bits : 9_0
    ROI_Y1:    [0x0, 0x3ff],			bits : 21_12
*/
#define ROI1_REGISTER0_OFS 0x008c
REGDEF_BEGIN(ROI1_REGISTER0)
    REGDEF_BIT(ROI_X1,        10)
    REGDEF_BIT(      ,        2)
    REGDEF_BIT(ROI_Y1,        10)
REGDEF_END(ROI1_REGISTER0)


/*
    ROI_W1       :    [0x0, 0x3ff],			bits : 9_0
    ROI_H1       :    [0x0, 0x3ff],			bits : 21_12
    ROI_UV_THRES1:    [0x0, 0xff],			bits : 31_24
*/
#define ROI1_REGISTER1_OFS 0x0090
REGDEF_BEGIN(ROI1_REGISTER1)
    REGDEF_BIT(ROI_W1       ,        10)
    REGDEF_BIT(             ,        2)
    REGDEF_BIT(ROI_H1       ,        10)
    REGDEF_BIT(             ,        2)
    REGDEF_BIT(ROI_UV_THRES1,        8)
REGDEF_END(ROI1_REGISTER1)


/*
    ROI_LBSP_TH1 :    [0x0, 0xff],			bits : 7_0
    ROI_D_COLOUR1:    [0x0, 0xff],			bits : 15_8
    ROI_R_COLOUR1:    [0x0, 0xff],			bits : 23_16
    ROI_D_LBSP1  :    [0x0, 0xf],			bits : 27_24
    ROI_R_LBSP1  :    [0x0, 0xf],			bits : 31_28
*/
#define ROI1_REGISTER2_OFS 0x0094
REGDEF_BEGIN(ROI1_REGISTER2)
    REGDEF_BIT(ROI_LBSP_TH1 ,        8)
    REGDEF_BIT(ROI_D_COLOUR1,        8)
    REGDEF_BIT(ROI_R_COLOUR1,        8)
    REGDEF_BIT(ROI_D_LBSP1  ,        4)
    REGDEF_BIT(ROI_R_LBSP1  ,        4)
REGDEF_END(ROI1_REGISTER2)


/*
    ROI_MORPH_EN1:    [0x0, 0x1],			bits : 0
    ROI_MIN_T1   :    [0x0, 0xff],			bits : 11_4
    ROI_MAX_T1   :    [0x0, 0xff],			bits : 19_12
*/
#define ROI1_REGISTER3_OFS 0x0098
REGDEF_BEGIN(ROI1_REGISTER3)
    REGDEF_BIT(ROI_MORPH_EN1,        1)
    REGDEF_BIT(             ,        3)
    REGDEF_BIT(ROI_MIN_T1   ,        8)
    REGDEF_BIT(ROI_MAX_T1   ,        8)
REGDEF_END(ROI1_REGISTER3)


/*
    ROI_X2:    [0x0, 0x3ff],			bits : 9_0
    ROI_Y2:    [0x0, 0x3ff],			bits : 21_12
*/
#define ROI2_REGISTER0_OFS 0x009c
REGDEF_BEGIN(ROI2_REGISTER0)
    REGDEF_BIT(ROI_X2,        10)
    REGDEF_BIT(      ,        2)
    REGDEF_BIT(ROI_Y2,        10)
REGDEF_END(ROI2_REGISTER0)


/*
    ROI_W2       :    [0x0, 0x3ff],			bits : 9_0
    ROI_H2       :    [0x0, 0x3ff],			bits : 21_12
    ROI_UV_THRES2:    [0x0, 0xff],			bits : 31_24
*/
#define ROI2_REGISTER1_OFS 0x00a0
REGDEF_BEGIN(ROI2_REGISTER1)
    REGDEF_BIT(ROI_W2       ,        10)
    REGDEF_BIT(             ,        2)
    REGDEF_BIT(ROI_H2       ,        10)
    REGDEF_BIT(             ,        2)
    REGDEF_BIT(ROI_UV_THRES2,        8)
REGDEF_END(ROI2_REGISTER1)


/*
    ROI_LBSP_TH2 :    [0x0, 0xff],			bits : 7_0
    ROI_D_COLOUR2:    [0x0, 0xff],			bits : 15_8
    ROI_R_COLOUR2:    [0x0, 0xff],			bits : 23_16
    ROI_D_LBSP2  :    [0x0, 0xf],			bits : 27_24
    ROI_R_LBSP2  :    [0x0, 0xf],			bits : 31_28
*/
#define ROI2_REGISTER2_OFS 0x00a4
REGDEF_BEGIN(ROI2_REGISTER2)
    REGDEF_BIT(ROI_LBSP_TH2 ,        8)
    REGDEF_BIT(ROI_D_COLOUR2,        8)
    REGDEF_BIT(ROI_R_COLOUR2,        8)
    REGDEF_BIT(ROI_D_LBSP2  ,        4)
    REGDEF_BIT(ROI_R_LBSP2  ,        4)
REGDEF_END(ROI2_REGISTER2)


/*
    ROI_MORPH_EN2:    [0x0, 0x1],			bits : 0
    ROI_MIN_T2   :    [0x0, 0xff],			bits : 11_4
    ROI_MAX_T2   :    [0x0, 0xff],			bits : 19_12
*/
#define ROI2_REGISTER3_OFS 0x00a8
REGDEF_BEGIN(ROI2_REGISTER3)
    REGDEF_BIT(ROI_MORPH_EN2,        1)
    REGDEF_BIT(             ,        3)
    REGDEF_BIT(ROI_MIN_T2   ,        8)
    REGDEF_BIT(ROI_MAX_T2   ,        8)
REGDEF_END(ROI2_REGISTER3)


/*
    ROI_X3:    [0x0, 0x3ff],			bits : 9_0
    ROI_Y3:    [0x0, 0x3ff],			bits : 21_12
*/
#define ROI3_REGISTER0_OFS 0x00ac
REGDEF_BEGIN(ROI3_REGISTER0)
    REGDEF_BIT(ROI_X3,        10)
    REGDEF_BIT(      ,        2)
    REGDEF_BIT(ROI_Y3,        10)
REGDEF_END(ROI3_REGISTER0)


/*
    ROI_W3       :    [0x0, 0x3ff],			bits : 9_0
    ROI_H3       :    [0x0, 0x3ff],			bits : 21_12
    ROI_UV_THRES3:    [0x0, 0xff],			bits : 31_24
*/
#define ROI3_REGISTER1_OFS 0x00b0
REGDEF_BEGIN(ROI3_REGISTER1)
    REGDEF_BIT(ROI_W3       ,        10)
    REGDEF_BIT(             ,        2)
    REGDEF_BIT(ROI_H3       ,        10)
    REGDEF_BIT(             ,        2)
    REGDEF_BIT(ROI_UV_THRES3,        8)
REGDEF_END(ROI3_REGISTER1)


/*
    ROI_LBSP_TH3 :    [0x0, 0xff],			bits : 7_0
    ROI_D_COLOUR3:    [0x0, 0xff],			bits : 15_8
    ROI_R_COLOUR3:    [0x0, 0xff],			bits : 23_16
    ROI_D_LBSP3  :    [0x0, 0xf],			bits : 27_24
    ROI_R_LBSP3  :    [0x0, 0xf],			bits : 31_28
*/
#define ROI3_REGISTER2_OFS 0x00b4
REGDEF_BEGIN(ROI3_REGISTER2)
    REGDEF_BIT(ROI_LBSP_TH3 ,        8)
    REGDEF_BIT(ROI_D_COLOUR3,        8)
    REGDEF_BIT(ROI_R_COLOUR3,        8)
    REGDEF_BIT(ROI_D_LBSP3  ,        4)
    REGDEF_BIT(ROI_R_LBSP3  ,        4)
REGDEF_END(ROI3_REGISTER2)


/*
    ROI_MORPH_EN3:    [0x0, 0x1],			bits : 0
    ROI_MIN_T3   :    [0x0, 0xff],			bits : 11_4
    ROI_MAX_T3   :    [0x0, 0xff],			bits : 19_12
*/
#define ROI3_REGISTER3_OFS 0x00b8
REGDEF_BEGIN(ROI3_REGISTER3)
    REGDEF_BIT(ROI_MORPH_EN3,        1)
    REGDEF_BIT(             ,        3)
    REGDEF_BIT(ROI_MIN_T3   ,        8)
    REGDEF_BIT(ROI_MAX_T3   ,        8)
REGDEF_END(ROI3_REGISTER3)


/*
    ROI_X4:    [0x0, 0x3ff],			bits : 9_0
    ROI_Y4:    [0x0, 0x3ff],			bits : 21_12
*/
#define ROI4_REGISTER0_OFS 0x00bc
REGDEF_BEGIN(ROI4_REGISTER0)
    REGDEF_BIT(ROI_X4,        10)
    REGDEF_BIT(      ,        2)
    REGDEF_BIT(ROI_Y4,        10)
REGDEF_END(ROI4_REGISTER0)


/*
    ROI_W4       :    [0x0, 0x3ff],			bits : 9_0
    ROI_H4       :    [0x0, 0x3ff],			bits : 21_12
    ROI_UV_THRES4:    [0x0, 0xff],			bits : 31_24
*/
#define ROI4_REGISTER1_OFS 0x00c0
REGDEF_BEGIN(ROI4_REGISTER1)
    REGDEF_BIT(ROI_W4       ,        10)
    REGDEF_BIT(             ,        2)
    REGDEF_BIT(ROI_H4       ,        10)
    REGDEF_BIT(             ,        2)
    REGDEF_BIT(ROI_UV_THRES4,        8)
REGDEF_END(ROI4_REGISTER1)


/*
    ROI_LBSP_TH4 :    [0x0, 0xff],			bits : 7_0
    ROI_D_COLOUR4:    [0x0, 0xff],			bits : 15_8
    ROI_R_COLOUR4:    [0x0, 0xff],			bits : 23_16
    ROI_D_LBSP4  :    [0x0, 0xf],			bits : 27_24
    ROI_R_LBSP4  :    [0x0, 0xf],			bits : 31_28
*/
#define ROI4_REGISTER2_OFS 0x00c4
REGDEF_BEGIN(ROI4_REGISTER2)
    REGDEF_BIT(ROI_LBSP_TH4 ,        8)
    REGDEF_BIT(ROI_D_COLOUR4,        8)
    REGDEF_BIT(ROI_R_COLOUR4,        8)
    REGDEF_BIT(ROI_D_LBSP4  ,        4)
    REGDEF_BIT(ROI_R_LBSP4  ,        4)
REGDEF_END(ROI4_REGISTER2)


/*
    ROI_MORPH_EN4:    [0x0, 0x1],			bits : 0
    ROI_MIN_T4   :    [0x0, 0xff],			bits : 11_4
    ROI_MAX_T4   :    [0x0, 0xff],			bits : 19_12
*/
#define ROI4_REGISTER3_OFS 0x00c8
REGDEF_BEGIN(ROI4_REGISTER3)
    REGDEF_BIT(ROI_MORPH_EN4,        1)
    REGDEF_BIT(             ,        3)
    REGDEF_BIT(ROI_MIN_T4   ,        8)
    REGDEF_BIT(ROI_MAX_T4   ,        8)
REGDEF_END(ROI4_REGISTER3)


/*
    ROI_X5:    [0x0, 0x3ff],			bits : 9_0
    ROI_Y5:    [0x0, 0x3ff],			bits : 21_12
*/
#define ROI5_REGISTER0_OFS 0x00cc
REGDEF_BEGIN(ROI5_REGISTER0)
    REGDEF_BIT(ROI_X5,        10)
    REGDEF_BIT(      ,        2)
    REGDEF_BIT(ROI_Y5,        10)
REGDEF_END(ROI5_REGISTER0)


/*
    ROI_W5       :    [0x0, 0x3ff],			bits : 9_0
    ROI_H5       :    [0x0, 0x3ff],			bits : 21_12
    ROI_UV_THRES5:    [0x0, 0xff],			bits : 31_24
*/
#define ROI5_REGISTER1_OFS 0x00d0
REGDEF_BEGIN(ROI5_REGISTER1)
    REGDEF_BIT(ROI_W5       ,        10)
    REGDEF_BIT(             ,        2)
    REGDEF_BIT(ROI_H5       ,        10)
    REGDEF_BIT(             ,        2)
    REGDEF_BIT(ROI_UV_THRES5,        8)
REGDEF_END(ROI5_REGISTER1)


/*
    ROI_LBSP_TH5 :    [0x0, 0xff],			bits : 7_0
    ROI_D_COLOUR5:    [0x0, 0xff],			bits : 15_8
    ROI_R_COLOUR5:    [0x0, 0xff],			bits : 23_16
    ROI_D_LBSP5  :    [0x0, 0xf],			bits : 27_24
    ROI_R_LBSP5  :    [0x0, 0xf],			bits : 31_28
*/
#define ROI5_REGISTER2_OFS 0x00d4
REGDEF_BEGIN(ROI5_REGISTER2)
    REGDEF_BIT(ROI_LBSP_TH5 ,        8)
    REGDEF_BIT(ROI_D_COLOUR5,        8)
    REGDEF_BIT(ROI_R_COLOUR5,        8)
    REGDEF_BIT(ROI_D_LBSP5  ,        4)
    REGDEF_BIT(ROI_R_LBSP5  ,        4)
REGDEF_END(ROI5_REGISTER2)


/*
    ROI_MORPH_EN5:    [0x0, 0x1],			bits : 0
    ROI_MIN_T5   :    [0x0, 0xff],			bits : 11_4
    ROI_MAX_T5   :    [0x0, 0xff],			bits : 19_12
*/
#define ROI5_REGISTER3_OFS 0x00d8
REGDEF_BEGIN(ROI5_REGISTER3)
    REGDEF_BIT(ROI_MORPH_EN5,        1)
    REGDEF_BIT(             ,        3)
    REGDEF_BIT(ROI_MIN_T5   ,        8)
    REGDEF_BIT(ROI_MAX_T5   ,        8)
REGDEF_END(ROI5_REGISTER3)


/*
    ROI_X6:    [0x0, 0x3ff],			bits : 9_0
    ROI_Y6:    [0x0, 0x3ff],			bits : 21_12
*/
#define ROI6_REGISTER0_OFS 0x00dc
REGDEF_BEGIN(ROI6_REGISTER0)
    REGDEF_BIT(ROI_X6,        10)
    REGDEF_BIT(      ,        2)
    REGDEF_BIT(ROI_Y6,        10)
REGDEF_END(ROI6_REGISTER0)


/*
    ROI_W6       :    [0x0, 0x3ff],			bits : 9_0
    ROI_H6       :    [0x0, 0x3ff],			bits : 21_12
    ROI_UV_THRES6:    [0x0, 0xff],			bits : 31_24
*/
#define ROI6_REGISTER1_OFS 0x00e0
REGDEF_BEGIN(ROI6_REGISTER1)
    REGDEF_BIT(ROI_W6       ,        10)
    REGDEF_BIT(             ,        2)
    REGDEF_BIT(ROI_H6       ,        10)
    REGDEF_BIT(             ,        2)
    REGDEF_BIT(ROI_UV_THRES6,        8)
REGDEF_END(ROI6_REGISTER1)


/*
    ROI_LBSP_TH6 :    [0x0, 0xff],			bits : 7_0
    ROI_D_COLOUR6:    [0x0, 0xff],			bits : 15_8
    ROI_R_COLOUR6:    [0x0, 0xff],			bits : 23_16
    ROI_D_LBSP6  :    [0x0, 0xf],			bits : 27_24
    ROI_R_LBSP6  :    [0x0, 0xf],			bits : 31_28
*/
#define ROI6_REGISTER2_OFS 0x00e4
REGDEF_BEGIN(ROI6_REGISTER2)
    REGDEF_BIT(ROI_LBSP_TH6 ,        8)
    REGDEF_BIT(ROI_D_COLOUR6,        8)
    REGDEF_BIT(ROI_R_COLOUR6,        8)
    REGDEF_BIT(ROI_D_LBSP6  ,        4)
    REGDEF_BIT(ROI_R_LBSP6  ,        4)
REGDEF_END(ROI6_REGISTER2)


/*
    ROI_MORPH_EN6:    [0x0, 0x1],			bits : 0
    ROI_MIN_T6   :    [0x0, 0xff],			bits : 11_4
    ROI_MAX_T6   :    [0x0, 0xff],			bits : 19_12
*/
#define ROI6_REGISTER3_OFS 0x00e8
REGDEF_BEGIN(ROI6_REGISTER3)
    REGDEF_BIT(ROI_MORPH_EN6,        1)
    REGDEF_BIT(             ,        3)
    REGDEF_BIT(ROI_MIN_T6   ,        8)
    REGDEF_BIT(ROI_MAX_T6   ,        8)
REGDEF_END(ROI6_REGISTER3)


/*
    ROI_X7:    [0x0, 0x3ff],			bits : 9_0
    ROI_Y7:    [0x0, 0x3ff],			bits : 21_12
*/
#define ROI7_REGISTER0_OFS 0x00ec
REGDEF_BEGIN(ROI7_REGISTER0)
    REGDEF_BIT(ROI_X7,        10)
    REGDEF_BIT(      ,        2)
    REGDEF_BIT(ROI_Y7,        10)
REGDEF_END(ROI7_REGISTER0)


/*
    ROI_W7       :    [0x0, 0x3ff],			bits : 9_0
    ROI_H7       :    [0x0, 0x3ff],			bits : 21_12
    ROI_UV_THRES7:    [0x0, 0xff],			bits : 31_24
*/
#define ROI7_REGISTER1_OFS 0x00f0
REGDEF_BEGIN(ROI7_REGISTER1)
    REGDEF_BIT(ROI_W7       ,        10)
    REGDEF_BIT(             ,        2)
    REGDEF_BIT(ROI_H7       ,        10)
    REGDEF_BIT(             ,        2)
    REGDEF_BIT(ROI_UV_THRES7,        8)
REGDEF_END(ROI7_REGISTER1)


/*
    ROI_LBSP_TH7 :    [0x0, 0xff],			bits : 7_0
    ROI_D_COLOUR7:    [0x0, 0xff],			bits : 15_8
    ROI_R_COLOUR7:    [0x0, 0xff],			bits : 23_16
    ROI_D_LBSP7  :    [0x0, 0xf],			bits : 27_24
    ROI_R_LBSP7  :    [0x0, 0xf],			bits : 31_28
*/
#define ROI7_REGISTER2_OFS 0x00f4
REGDEF_BEGIN(ROI7_REGISTER2)
    REGDEF_BIT(ROI_LBSP_TH7 ,        8)
    REGDEF_BIT(ROI_D_COLOUR7,        8)
    REGDEF_BIT(ROI_R_COLOUR7,        8)
    REGDEF_BIT(ROI_D_LBSP7  ,        4)
    REGDEF_BIT(ROI_R_LBSP7  ,        4)
REGDEF_END(ROI7_REGISTER2)


/*
    ROI_MORPH_EN7:    [0x0, 0x1],			bits : 0
    ROI_MIN_T7   :    [0x0, 0xff],			bits : 11_4
    ROI_MAX_T7   :    [0x0, 0xff],			bits : 19_12
*/
#define ROI7_REGISTER3_OFS 0x00f8
REGDEF_BEGIN(ROI7_REGISTER3)
    REGDEF_BIT(ROI_MORPH_EN7,        1)
    REGDEF_BIT(             ,        3)
    REGDEF_BIT(ROI_MIN_T7   ,        8)
    REGDEF_BIT(ROI_MAX_T7   ,        8)
REGDEF_END(ROI7_REGISTER3)


/*
    MDBC_AXI_CH_EN   :    [0x0, 0x3ff],			bits : 9_0
    MDBC_AXI_LOCK_DIS:    [0x0, 0x3ff],			bits : 21_12
*/
#define AXI_REGISTER0_OFS 0x00fc
REGDEF_BEGIN(AXI_REGISTER0)
    REGDEF_BIT(MDBC_AXI_CH_EN   ,        10)
    REGDEF_BIT(                 ,        2)
    REGDEF_BIT(MDBC_AXI_LOCK_DIS,        10)
REGDEF_END(AXI_REGISTER0)


/*
    AXI_BUS_DISABLE:    [0x0, 0x1],			bits : 0
    AXI_BUS_IDLE   :    [0x0, 0x1],			bits : 16
*/
#define AXI_REGISTER1_OFS 0x0100
REGDEF_BEGIN(AXI_REGISTER1)
    REGDEF_BIT(AXI_BUS_DISABLE,        1)
    REGDEF_BIT(               ,        15)
    REGDEF_BIT(AXI_BUS_IDLE   ,        1)
REGDEF_END(AXI_REGISTER1)


/*
    AXI_CH_STA:    [0x0, 0xffffffff],			bits : 31_0
*/
#define AXI_REGISTER2_OFS 0x0104
REGDEF_BEGIN(AXI_REGISTER2)
    REGDEF_BIT(AXI_CH_STA,        32)
REGDEF_END(AXI_REGISTER2)


/*
    MDBC_R_OSTD_NUM:    [0x0, 0xff],			bits : 7_0
    MDBC_W_OSTD_NUM:    [0x0, 0xff],			bits : 15_8
*/
#define AXI_REGISTER3_OFS 0x0108
REGDEF_BEGIN(AXI_REGISTER3)
    REGDEF_BIT(MDBC_R_OSTD_NUM,        8)
    REGDEF_BIT(MDBC_W_OSTD_NUM,        8)
REGDEF_END(AXI_REGISTER3)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER5_OFS 0x010c
REGDEF_BEGIN(RESERVED_REGISTER5)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER5)


/*
    DMA_CH_DISABLE:    [0x0, 0x1],			bits : 0
    DMA_IDLE      :    [0x0, 0x1],			bits : 15
*/
#define DMA_DISABLE_REGISTER0_OFS 0x0110
REGDEF_BEGIN(DMA_DISABLE_REGISTER0)
    REGDEF_BIT(DMA_CH_DISABLE,        1)
    REGDEF_BIT(              ,        14)
    REGDEF_BIT(DMA_IDLE      ,        1)
REGDEF_END(DMA_DISABLE_REGISTER0)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER6_OFS 0x0114
REGDEF_BEGIN(RESERVED_REGISTER6)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER6)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER7_OFS 0x0118
REGDEF_BEGIN(RESERVED_REGISTER7)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER7)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER8_OFS 0x011c
REGDEF_BEGIN(RESERVED_REGISTER8)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER8)


/*
    CHKSUM_SRCY:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECK_SUM_REGISTER0_OFS 0x0120
REGDEF_BEGIN(CHECK_SUM_REGISTER0)
    REGDEF_BIT(CHKSUM_SRCY,        32)
REGDEF_END(CHECK_SUM_REGISTER0)


/*
    CHKSUM_SRCUV:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECK_SUM_REGISTER1_OFS 0x0124
REGDEF_BEGIN(CHECK_SUM_REGISTER1)
    REGDEF_BIT(CHKSUM_SRCUV,        32)
REGDEF_END(CHECK_SUM_REGISTER1)


/*
    CHKSUM_PREUV:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECK_SUM_REGISTER2_OFS 0x0128
REGDEF_BEGIN(CHECK_SUM_REGISTER2)
    REGDEF_BIT(CHKSUM_PREUV,        32)
REGDEF_END(CHECK_SUM_REGISTER2)


/*
    CHKSUM_BGMR:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECK_SUM_REGISTER3_OFS 0x012c
REGDEF_BEGIN(CHECK_SUM_REGISTER3)
    REGDEF_BIT(CHKSUM_BGMR,        32)
REGDEF_END(CHECK_SUM_REGISTER3)


/*
    CHKSUM_VAR1:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECK_SUM_REGISTER4_OFS 0x0130
REGDEF_BEGIN(CHECK_SUM_REGISTER4)
    REGDEF_BIT(CHKSUM_VAR1,        32)
REGDEF_END(CHECK_SUM_REGISTER4)


/*
    CHKSUM_FVAR2:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECK_SUM_REGISTER5_OFS 0x0134
REGDEF_BEGIN(CHECK_SUM_REGISTER5)
    REGDEF_BIT(CHKSUM_FVAR2,        32)
REGDEF_END(CHECK_SUM_REGISTER5)


/*
    CHKSUM_BGMW:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECK_SUM_REGISTER6_OFS 0x0138
REGDEF_BEGIN(CHECK_SUM_REGISTER6)
    REGDEF_BIT(CHKSUM_BGMW,        32)
REGDEF_END(CHECK_SUM_REGISTER6)


/*
    CHKSUM_UV1:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECK_SUM_REGISTER7_OFS 0x013c
REGDEF_BEGIN(CHECK_SUM_REGISTER7)
    REGDEF_BIT(CHKSUM_UV1,        32)
REGDEF_END(CHECK_SUM_REGISTER7)


/*
    CHKSUM_BVAR2:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECK_SUM_REGISTER8_OFS 0x0140
REGDEF_BEGIN(CHECK_SUM_REGISTER8)
    REGDEF_BIT(CHKSUM_BVAR2,        32)
REGDEF_END(CHECK_SUM_REGISTER8)


/*
    CHKSUM_UPD:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECK_SUM_REGISTER9_OFS 0x0144
REGDEF_BEGIN(CHECK_SUM_REGISTER9)
    REGDEF_BIT(CHKSUM_UPD,        32)
REGDEF_END(CHECK_SUM_REGISTER9)


/*
    CHKSUM_PASS:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECK_SUM_REGISTER10_OFS 0x0148
REGDEF_BEGIN(CHECK_SUM_REGISTER10)
    REGDEF_BIT(CHKSUM_PASS,        32)
REGDEF_END(CHECK_SUM_REGISTER10)


/*
    CHKSUM_MPH:    [0x0, 0xffffffff],			bits : 31_0
*/
#define CHECK_SUM_REGISTER11_OFS 0x014c
REGDEF_BEGIN(CHECK_SUM_REGISTER11)
    REGDEF_BIT(CHKSUM_MPH,        32)
REGDEF_END(CHECK_SUM_REGISTER11)


/*
    MDBC_ENG_CYCLE:    [0x0, 0xffffffff],			bits : 31_0
*/
#define ENGINE_CYCLE0_OFS 0x0150
REGDEF_BEGIN(ENGINE_CYCLE0)
    REGDEF_BIT(MDBC_ENG_CYCLE,        32)
REGDEF_END(ENGINE_CYCLE0)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER9_OFS 0x0154
REGDEF_BEGIN(RESERVED_REGISTER9)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER9)


/*
    MDBC_WAIT_DMA_CYCLE:    [0x0, 0xffffffff],			bits : 31_0
*/
#define ENGINE_CYCLE1_OFS 0x0158
REGDEF_BEGIN(ENGINE_CYCLE1)
    REGDEF_BIT(MDBC_WAIT_DMA_CYCLE,        32)
REGDEF_END(ENGINE_CYCLE1)

#define MDBC_RESERVED_OFS_0 0x015C
REGDEF_BEGIN(MDBC_RESERVED_0)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_0) //0x015C


#define MDBC_RESERVED_OFS_1 0x0160
REGDEF_BEGIN(MDBC_RESERVED_1)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_1) //0x0160


#define MDBC_RESERVED_OFS_2 0x0164
REGDEF_BEGIN(MDBC_RESERVED_2)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_2) //0x0164


#define MDBC_RESERVED_OFS_3 0x0168
REGDEF_BEGIN(MDBC_RESERVED_3)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_3) //0x0168


#define MDBC_RESERVED_OFS_4 0x016C
REGDEF_BEGIN(MDBC_RESERVED_4)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_4) //0x016C


#define MDBC_RESERVED_OFS_5 0x0170
REGDEF_BEGIN(MDBC_RESERVED_5)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_5) //0x0170


#define MDBC_RESERVED_OFS_6 0x0174
REGDEF_BEGIN(MDBC_RESERVED_6)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_6) //0x0174


#define MDBC_RESERVED_OFS_7 0x0178
REGDEF_BEGIN(MDBC_RESERVED_7)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_7) //0x0178


#define MDBC_RESERVED_OFS_8 0x017C
REGDEF_BEGIN(MDBC_RESERVED_8)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_8) //0x017C


#define MDBC_RESERVED_OFS_9 0x0180
REGDEF_BEGIN(MDBC_RESERVED_9)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_9) //0x0180


#define MDBC_RESERVED_OFS_10 0x0184
REGDEF_BEGIN(MDBC_RESERVED_10)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_10) //0x0184


#define MDBC_RESERVED_OFS_11 0x0188
REGDEF_BEGIN(MDBC_RESERVED_11)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_11) //0x0188


#define MDBC_RESERVED_OFS_12 0x018C
REGDEF_BEGIN(MDBC_RESERVED_12)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_12) //0x018C


#define MDBC_RESERVED_OFS_13 0x0190
REGDEF_BEGIN(MDBC_RESERVED_13)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_13) //0x0190


#define MDBC_RESERVED_OFS_14 0x0194
REGDEF_BEGIN(MDBC_RESERVED_14)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_14) //0x0194


#define MDBC_RESERVED_OFS_15 0x0198
REGDEF_BEGIN(MDBC_RESERVED_15)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_15) //0x0198


#define MDBC_RESERVED_OFS_16 0x019C
REGDEF_BEGIN(MDBC_RESERVED_16)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_16) //0x019C


#define MDBC_RESERVED_OFS_17 0x01A0
REGDEF_BEGIN(MDBC_RESERVED_17)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_17) //0x01A0


#define MDBC_RESERVED_OFS_18 0x01A4
REGDEF_BEGIN(MDBC_RESERVED_18)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_18) //0x01A4


#define MDBC_RESERVED_OFS_19 0x01A8
REGDEF_BEGIN(MDBC_RESERVED_19)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_19) //0x01A8


#define MDBC_RESERVED_OFS_20 0x01AC
REGDEF_BEGIN(MDBC_RESERVED_20)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_20) //0x01AC


#define MDBC_RESERVED_OFS_21 0x01B0
REGDEF_BEGIN(MDBC_RESERVED_21)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_21) //0x01B0


#define MDBC_RESERVED_OFS_22 0x01B4
REGDEF_BEGIN(MDBC_RESERVED_22)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_22) //0x01B4


#define MDBC_RESERVED_OFS_23 0x01B8
REGDEF_BEGIN(MDBC_RESERVED_23)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_23) //0x01B8


#define MDBC_RESERVED_OFS_24 0x01BC
REGDEF_BEGIN(MDBC_RESERVED_24)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_24) //0x01BC


#define MDBC_RESERVED_OFS_25 0x01C0
REGDEF_BEGIN(MDBC_RESERVED_25)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_25) //0x01C0


#define MDBC_RESERVED_OFS_26 0x01C4
REGDEF_BEGIN(MDBC_RESERVED_26)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_26) //0x01C4


#define MDBC_RESERVED_OFS_27 0x01C8
REGDEF_BEGIN(MDBC_RESERVED_27)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_27) //0x01C8


#define MDBC_RESERVED_OFS_28 0x01CC
REGDEF_BEGIN(MDBC_RESERVED_28)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_28) //0x01CC


#define MDBC_RESERVED_OFS_29 0x01D0
REGDEF_BEGIN(MDBC_RESERVED_29)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_29) //0x01D0


#define MDBC_RESERVED_OFS_30 0x01D4
REGDEF_BEGIN(MDBC_RESERVED_30)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_30) //0x01D4


#define MDBC_RESERVED_OFS_31 0x01D8
REGDEF_BEGIN(MDBC_RESERVED_31)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_31) //0x01D8


#define MDBC_RESERVED_OFS_32 0x01DC
REGDEF_BEGIN(MDBC_RESERVED_32)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_32) //0x01DC


#define MDBC_RESERVED_OFS_33 0x01E0
REGDEF_BEGIN(MDBC_RESERVED_33)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_33) //0x01E0


#define MDBC_RESERVED_OFS_34 0x01E4
REGDEF_BEGIN(MDBC_RESERVED_34)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_34) //0x01E4


#define MDBC_RESERVED_OFS_35 0x01E8
REGDEF_BEGIN(MDBC_RESERVED_35)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_35) //0x01E8


#define MDBC_RESERVED_OFS_36 0x01EC
REGDEF_BEGIN(MDBC_RESERVED_36)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_36) //0x01EC


#define MDBC_RESERVED_OFS_37 0x01F0
REGDEF_BEGIN(MDBC_RESERVED_37)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_37) //0x01F0


#define MDBC_RESERVED_OFS_38 0x01F4
REGDEF_BEGIN(MDBC_RESERVED_38)
    REGDEF_BIT(RESERVED,        32)
REGDEF_END(MDBC_RESERVED_38) //0x01F4


/*
    DRAM_MSB_SAI0:    [0x0, 0xf],			bits : 3_0
*/
#define DMA_TO_MDBC_REGISTER8_OFS 0x01f8
REGDEF_BEGIN(DMA_TO_MDBC_REGISTER8)
    REGDEF_BIT(DRAM_MSB_SAI0,        4)
REGDEF_END(DMA_TO_MDBC_REGISTER8)


/*
    DRAM_MSB_SAI1:    [0x0, 0xf],			bits : 3_0
*/
#define DMA_TO_MDBC_REGISTER9_OFS 0x01fc
REGDEF_BEGIN(DMA_TO_MDBC_REGISTER9)
    REGDEF_BIT(DRAM_MSB_SAI1,        4)
REGDEF_END(DMA_TO_MDBC_REGISTER9)


/*
    DRAM_MSB_SAI2:    [0x0, 0xf],			bits : 3_0
*/
#define DMA_TO_MDBC_REGISTER10_OFS 0x0200
REGDEF_BEGIN(DMA_TO_MDBC_REGISTER10)
    REGDEF_BIT(DRAM_MSB_SAI2,        4)
REGDEF_END(DMA_TO_MDBC_REGISTER10)


/*
    DRAM_MSB_SAI3:    [0x0, 0xf],			bits : 3_0
*/
#define DMA_TO_MDBC_REGISTER11_OFS 0x0204
REGDEF_BEGIN(DMA_TO_MDBC_REGISTER11)
    REGDEF_BIT(DRAM_MSB_SAI3,        4)
REGDEF_END(DMA_TO_MDBC_REGISTER11)


/*
    DRAM_MSB_SAI4:    [0x0, 0xf],			bits : 3_0
*/
#define DMA_TO_MDBC_REGISTER12_OFS 0x0208
REGDEF_BEGIN(DMA_TO_MDBC_REGISTER12)
    REGDEF_BIT(DRAM_MSB_SAI4,        4)
REGDEF_END(DMA_TO_MDBC_REGISTER12)


/*
    DRAM_MSB_SAI5:    [0x0, 0xf],			bits : 3_0
*/
#define DMA_TO_MDBC_REGISTER13_OFS 0x020c
REGDEF_BEGIN(DMA_TO_MDBC_REGISTER13)
    REGDEF_BIT(DRAM_MSB_SAI5,        4)
REGDEF_END(DMA_TO_MDBC_REGISTER13)


/*
    DRAM_MSB_SAO0:    [0x0, 0xf],			bits : 3_0
*/
#define MDBC_TO_DMA_REGISTER4_OFS 0x0210
REGDEF_BEGIN(MDBC_TO_DMA_REGISTER4)
    REGDEF_BIT(DRAM_MSB_SAO0,        4)
REGDEF_END(MDBC_TO_DMA_REGISTER4)


/*
    DRAM_MSB_SAO1:    [0x0, 0xf],			bits : 3_0
*/
#define MDBC_TO_DMA_REGISTER5_OFS 0x0214
REGDEF_BEGIN(MDBC_TO_DMA_REGISTER5)
    REGDEF_BIT(DRAM_MSB_SAO1,        4)
REGDEF_END(MDBC_TO_DMA_REGISTER5)


/*
    DRAM_MSB_SAO2:    [0x0, 0xf],			bits : 3_0
*/
#define MDBC_TO_DMA_REGISTER6_OFS 0x0218
REGDEF_BEGIN(MDBC_TO_DMA_REGISTER6)
    REGDEF_BIT(DRAM_MSB_SAO2,        4)
REGDEF_END(MDBC_TO_DMA_REGISTER6)


/*
    DRAM_MSB_SAO3:    [0x0, 0xf],			bits : 3_0
*/
#define MDBC_TO_DMA_REGISTER7_OFS 0x021c
REGDEF_BEGIN(MDBC_TO_DMA_REGISTER7)
    REGDEF_BIT(DRAM_MSB_SAO3,        4)
REGDEF_END(MDBC_TO_DMA_REGISTER7)

typedef struct
{

  union
  {
    struct
    {
      unsigned MDBC_SW_RST        : 1;		// bits : 0
      unsigned MDBC_START         : 1;		// bits : 1
    } Bit;
    UINT32 Word;
  } MDBC_Register_0; // 0x0000

  union
  {
    struct
    {
      unsigned MDBC_MODE               : 1;		// bits : 0
      unsigned                         : 3;
      unsigned BC_UPDATE_NEI_EN        : 1;		// bits : 4
      unsigned                         : 3;
      unsigned BC_DEGHOST_EN           : 1;		// bits : 8
      unsigned                         : 3;
      unsigned ROI_EN0                 : 1;		// bits : 12
      unsigned ROI_EN1                 : 1;		// bits : 13
      unsigned ROI_EN2                 : 1;		// bits : 14
      unsigned ROI_EN3                 : 1;		// bits : 15
      unsigned ROI_EN4                 : 1;		// bits : 16
      unsigned ROI_EN5                 : 1;		// bits : 17
      unsigned ROI_EN6                 : 1;		// bits : 18
      unsigned ROI_EN7                 : 1;		// bits : 19
      unsigned CHKSUM_EN               : 1;		// bits : 20
      unsigned                         : 3;
      unsigned BGMW_SAVE_BW_EN         : 1;		// bits : 24
      unsigned                         : 3;
      unsigned BC_Y_ONLY_EN            : 1;		// bits : 28
      unsigned BC_MODEL_MODE           : 1;		// bits : 29
      unsigned MD_OUT_BIT_DEPTH        : 1;		// bits : 30
      unsigned MD_LBSP_DISABLE         : 1;		// bits : 31
    } Bit;
    UINT32 Word;
  } MDBC_Register_1; // 0x0004

  union
  {
    struct
    {
      unsigned INTE_FRM_END        : 1;		// bits : 0
    } Bit;
    UINT32 Word;
  } MDBC_Register_2; // 0x0008

  union
  {
    struct
    {
      unsigned INTS_FRM_END        : 1;		// bits : 0
    } Bit;
    UINT32 Word;
  } MDBC_Register_3; // 0x000c

  union
  {
    struct
    {
      unsigned                  : 2;
      unsigned DRAM_SAI0        : 30;		// bits : 31_2
    } Bit;
    UINT32 Word;
  } MDBC_Register_4; // 0x0010

  union
  {
    struct
    {
      unsigned                  : 2;
      unsigned DRAM_SAI1        : 30;		// bits : 31_2
    } Bit;
    UINT32 Word;
  } MDBC_Register_5; // 0x0014

  union
  {
    struct
    {
      unsigned                   : 2;
      unsigned DRAM_OFSI0        : 18;		// bits : 19_2
    } Bit;
    UINT32 Word;
  } MDBC_Register_6; // 0x0018

  union
  {
    struct
    {
      unsigned                   : 2;
      unsigned DRAM_OFSI1        : 18;		// bits : 19_2
    } Bit;
    UINT32 Word;
  } MDBC_Register_7; // 0x001c

  union
  {
    struct
    {
      unsigned                  : 2;
      unsigned DRAM_SAI2        : 30;		// bits : 31_2
    } Bit;
    UINT32 Word;
  } MDBC_Register_8; // 0x0020

  union
  {
    struct
    {
      unsigned                  : 2;
      unsigned DRAM_SAI3        : 30;		// bits : 31_2
    } Bit;
    UINT32 Word;
  } MDBC_Register_9; // 0x0024

  union
  {
    struct
    {
      unsigned                  : 2;
      unsigned DRAM_SAI4        : 30;		// bits : 31_2
    } Bit;
    UINT32 Word;
  } MDBC_Register_10; // 0x0028

  union
  {
    struct
    {
      unsigned                  : 2;
      unsigned DRAM_SAI5        : 30;		// bits : 31_2
    } Bit;
    UINT32 Word;
  } MDBC_Register_11; // 0x002c

  union
  {
    struct
    {
      unsigned reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_12; // 0x0030

  union
  {
    struct
    {
      unsigned                  : 2;
      unsigned DRAM_SAO0        : 30;		// bits : 31_2
    } Bit;
    UINT32 Word;
  } MDBC_Register_13; // 0x0034

  union
  {
    struct
    {
      unsigned                  : 2;
      unsigned DRAM_SAO1        : 30;		// bits : 31_2
    } Bit;
    UINT32 Word;
  } MDBC_Register_14; // 0x0038

  union
  {
    struct
    {
      unsigned                  : 2;
      unsigned DRAM_SAO2        : 30;		// bits : 31_2
    } Bit;
    UINT32 Word;
  } MDBC_Register_15; // 0x003c

  union
  {
    struct
    {
      unsigned                  : 2;
      unsigned DRAM_SAO3        : 30;		// bits : 31_2
    } Bit;
    UINT32 Word;
  } MDBC_Register_16; // 0x0040

  union
  {
    struct
    {
      unsigned IN_BURST_LENGTH         : 1;		// bits : 0
      unsigned OUT_BURST_LENGTH        : 1;		// bits : 1
      unsigned                         : 14;
      unsigned VERSION_CODE            : 16;		// bits : 31_16
    } Bit;
    UINT32 Word;
  } MDBC_Register_17; // 0x0044

  union
  {
    struct
    {
      unsigned               : 1;
      unsigned WIDTH         : 9;		// bits : 9_1
      unsigned               : 3;
      unsigned HEIGHT        : 9;		// bits : 21_13
    } Bit;
    UINT32 Word;
  } MDBC_Register_18; // 0x0048

  union
  {
    struct
    {
      unsigned LBSP_TH         : 8;		// bits : 7_0
      unsigned D_COLOUR        : 8;		// bits : 15_8
      unsigned R_COLOUR        : 8;		// bits : 23_16
      unsigned D_LBSP          : 4;		// bits : 27_24
      unsigned R_LBSP          : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } MDBC_Register_19; // 0x004c

  union
  {
    struct
    {
      unsigned BG_MODEL_NUM        : 4;		// bits : 3_0
      unsigned T_ALPHA             : 8;		// bits : 11_4
      unsigned DW_SHIFT            : 3;		// bits : 14_12
      unsigned BG_MODEL_NUM_EXT    : 1;		// bits : 15
      unsigned D_LAST_ALPHA        : 10;		// bits : 25_16
    } Bit;
    UINT32 Word;
  } MDBC_Register_20; // 0x0050

  union
  {
    struct
    {
      unsigned BC_MIN_MATCH        : 4;		// bits : 3_0
      unsigned DLT_ALPHA           : 10;		// bits : 13_4
      unsigned                     : 2;
      unsigned DST_ALPHA           : 10;		// bits : 25_16
    } Bit;
    UINT32 Word;
  } MDBC_Register_21; // 0x0054

  union
  {
    struct
    {
      unsigned BC_UV_THRES        : 8;		// bits : 7_0
      unsigned                    : 4;
      unsigned S_ALPHA            : 10;		// bits : 21_12
    } Bit;
    UINT32 Word;
  } MDBC_Register_22; // 0x0058

  union
  {
    struct
    {
      unsigned DBG_LUM_DIFF           : 28;		// bits : 27_0
      unsigned DBG_LUM_DIFF_EN        : 1;		// bits : 28
    } Bit;
    UINT32 Word;
  } MDBC_Register_23; // 0x005c

  union
  {
    struct
    {
      unsigned LUM_DIFF        : 28;		// bits : 27_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_24; // 0x0060

  union
  {
    struct
    {
      unsigned MOR_TH0               : 4;		// bits : 3_0
      unsigned MOR_TH1               : 4;		// bits : 7_4
      unsigned MOR_TH2               : 4;		// bits : 11_8
      unsigned MOR_TH3               : 4;		// bits : 15_12
      unsigned MOR_TH_DIL            : 4;		// bits : 19_16
      unsigned MOR_SEL0              : 1;		// bits : 20
      unsigned MOR_SEL1              : 1;		// bits : 21
      unsigned MOR_SEL2              : 1;		// bits : 22
      unsigned MOR_SEL3              : 1;		// bits : 23
      unsigned BC_TEMPORAL_TH        : 4;		// bits : 27_24
    } Bit;
    UINT32 Word;
  } MDBC_Register_25; // 0x0064

  union
  {
    struct
    {
      unsigned BC_MIN_T             : 8;		// bits : 7_0
      unsigned BC_MAX_T             : 8;		// bits : 15_8
      unsigned BC_MAX_FG_FRM        : 8;		// bits : 23_16
    } Bit;
    UINT32 Word;
  } MDBC_Register_26; // 0x0068

  union
  {
    struct
    {
      unsigned BC_DEGHOST_DTH        : 9;		// bits : 8_0
      unsigned                       : 7;
      unsigned BC_DEGHOST_STH        : 8;		// bits : 23_16
    } Bit;
    UINT32 Word;
  } MDBC_Register_27; // 0x006c

  union
  {
    struct
    {
      unsigned BC_STABLE_FRAME        : 8;		// bits : 7_0
      unsigned BC_UPDATE_DYN          : 8;		// bits : 15_8
      unsigned BC_VA_DISTTH           : 8;		// bits : 23_16
      unsigned BC_T_DISTTH            : 8;		// bits : 31_24
    } Bit;
    UINT32 Word;
  } MDBC_Register_28; // 0x0070

  union
  {
    struct
    {
      unsigned DBG_FRM_ID           : 8;		// bits : 7_0
      unsigned DBG_FRM_ID_EN        : 1;		// bits : 8
      unsigned                      : 3;
      unsigned FRM_ID               : 8;		// bits : 19_12
    } Bit;
    UINT32 Word;
  } MDBC_Register_29; // 0x0074

  union
  {
    struct
    {
      unsigned DBG_RND           : 15;		// bits : 14_0
      unsigned DBG_RND_EN        : 1;		// bits : 15
      unsigned RND               : 15;		// bits : 30_16
    } Bit;
    UINT32 Word;
  } MDBC_Register_30; // 0x0078

  union
  {
    struct
    {
      unsigned ROI_X0        : 10;		// bits : 9_0
      unsigned               : 2;
      unsigned ROI_Y0        : 10;		// bits : 21_12
    } Bit;
    UINT32 Word;
  } MDBC_Register_31; // 0x007c

  union
  {
    struct
    {
      unsigned ROI_W0               : 10;		// bits : 9_0
      unsigned                      : 2;
      unsigned ROI_H0               : 10;		// bits : 21_12
      unsigned                      : 2;
      unsigned ROI_UV_THRES0        : 8;		// bits : 31_24
    } Bit;
    UINT32 Word;
  } MDBC_Register_32; // 0x0080

  union
  {
    struct
    {
      unsigned ROI_LBSP_TH0         : 8;		// bits : 7_0
      unsigned ROI_D_COLOUR0        : 8;		// bits : 15_8
      unsigned ROI_R_COLOUR0        : 8;		// bits : 23_16
      unsigned ROI_D_LBSP0          : 4;		// bits : 27_24
      unsigned ROI_R_LBSP0          : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } MDBC_Register_33; // 0x0084

  union
  {
    struct
    {
      unsigned ROI_MORPH_EN0        : 1;		// bits : 0
      unsigned                      : 3;
      unsigned ROI_MIN_T0           : 8;		// bits : 11_4
      unsigned ROI_MAX_T0           : 8;		// bits : 19_12
    } Bit;
    UINT32 Word;
  } MDBC_Register_34; // 0x0088

  union
  {
    struct
    {
      unsigned ROI_X1        : 10;		// bits : 9_0
      unsigned               : 2;
      unsigned ROI_Y1        : 10;		// bits : 21_12
    } Bit;
    UINT32 Word;
  } MDBC_Register_35; // 0x008c

  union
  {
    struct
    {
      unsigned ROI_W1               : 10;		// bits : 9_0
      unsigned                      : 2;
      unsigned ROI_H1               : 10;		// bits : 21_12
      unsigned                      : 2;
      unsigned ROI_UV_THRES1        : 8;		// bits : 31_24
    } Bit;
    UINT32 Word;
  } MDBC_Register_36; // 0x0090

  union
  {
    struct
    {
      unsigned ROI_LBSP_TH1         : 8;		// bits : 7_0
      unsigned ROI_D_COLOUR1        : 8;		// bits : 15_8
      unsigned ROI_R_COLOUR1        : 8;		// bits : 23_16
      unsigned ROI_D_LBSP1          : 4;		// bits : 27_24
      unsigned ROI_R_LBSP1          : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } MDBC_Register_37; // 0x0094

  union
  {
    struct
    {
      unsigned ROI_MORPH_EN1        : 1;		// bits : 0
      unsigned                      : 3;
      unsigned ROI_MIN_T1           : 8;		// bits : 11_4
      unsigned ROI_MAX_T1           : 8;		// bits : 19_12
    } Bit;
    UINT32 Word;
  } MDBC_Register_38; // 0x0098

  union
  {
    struct
    {
      unsigned ROI_X2        : 10;		// bits : 9_0
      unsigned               : 2;
      unsigned ROI_Y2        : 10;		// bits : 21_12
    } Bit;
    UINT32 Word;
  } MDBC_Register_39; // 0x009c

  union
  {
    struct
    {
      unsigned ROI_W2               : 10;		// bits : 9_0
      unsigned                      : 2;
      unsigned ROI_H2               : 10;		// bits : 21_12
      unsigned                      : 2;
      unsigned ROI_UV_THRES2        : 8;		// bits : 31_24
    } Bit;
    UINT32 Word;
  } MDBC_Register_40; // 0x00a0

  union
  {
    struct
    {
      unsigned ROI_LBSP_TH2         : 8;		// bits : 7_0
      unsigned ROI_D_COLOUR2        : 8;		// bits : 15_8
      unsigned ROI_R_COLOUR2        : 8;		// bits : 23_16
      unsigned ROI_D_LBSP2          : 4;		// bits : 27_24
      unsigned ROI_R_LBSP2          : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } MDBC_Register_41; // 0x00a4

  union
  {
    struct
    {
      unsigned ROI_MORPH_EN2        : 1;		// bits : 0
      unsigned                      : 3;
      unsigned ROI_MIN_T2           : 8;		// bits : 11_4
      unsigned ROI_MAX_T2           : 8;		// bits : 19_12
    } Bit;
    UINT32 Word;
  } MDBC_Register_42; // 0x00a8

  union
  {
    struct
    {
      unsigned ROI_X3        : 10;		// bits : 9_0
      unsigned               : 2;
      unsigned ROI_Y3        : 10;		// bits : 21_12
    } Bit;
    UINT32 Word;
  } MDBC_Register_43; // 0x00ac

  union
  {
    struct
    {
      unsigned ROI_W3               : 10;		// bits : 9_0
      unsigned                      : 2;
      unsigned ROI_H3               : 10;		// bits : 21_12
      unsigned                      : 2;
      unsigned ROI_UV_THRES3        : 8;		// bits : 31_24
    } Bit;
    UINT32 Word;
  } MDBC_Register_44; // 0x00b0

  union
  {
    struct
    {
      unsigned ROI_LBSP_TH3         : 8;		// bits : 7_0
      unsigned ROI_D_COLOUR3        : 8;		// bits : 15_8
      unsigned ROI_R_COLOUR3        : 8;		// bits : 23_16
      unsigned ROI_D_LBSP3          : 4;		// bits : 27_24
      unsigned ROI_R_LBSP3          : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } MDBC_Register_45; // 0x00b4

  union
  {
    struct
    {
      unsigned ROI_MORPH_EN3        : 1;		// bits : 0
      unsigned                      : 3;
      unsigned ROI_MIN_T3           : 8;		// bits : 11_4
      unsigned ROI_MAX_T3           : 8;		// bits : 19_12
    } Bit;
    UINT32 Word;
  } MDBC_Register_46; // 0x00b8

  union
  {
    struct
    {
      unsigned ROI_X4        : 10;		// bits : 9_0
      unsigned               : 2;
      unsigned ROI_Y4        : 10;		// bits : 21_12
    } Bit;
    UINT32 Word;
  } MDBC_Register_47; // 0x00bc

  union
  {
    struct
    {
      unsigned ROI_W4               : 10;		// bits : 9_0
      unsigned                      : 2;
      unsigned ROI_H4               : 10;		// bits : 21_12
      unsigned                      : 2;
      unsigned ROI_UV_THRES4        : 8;		// bits : 31_24
    } Bit;
    UINT32 Word;
  } MDBC_Register_48; // 0x00c0

  union
  {
    struct
    {
      unsigned ROI_LBSP_TH4         : 8;		// bits : 7_0
      unsigned ROI_D_COLOUR4        : 8;		// bits : 15_8
      unsigned ROI_R_COLOUR4        : 8;		// bits : 23_16
      unsigned ROI_D_LBSP4          : 4;		// bits : 27_24
      unsigned ROI_R_LBSP4          : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } MDBC_Register_49; // 0x00c4

  union
  {
    struct
    {
      unsigned ROI_MORPH_EN4        : 1;		// bits : 0
      unsigned                      : 3;
      unsigned ROI_MIN_T4           : 8;		// bits : 11_4
      unsigned ROI_MAX_T4           : 8;		// bits : 19_12
    } Bit;
    UINT32 Word;
  } MDBC_Register_50; // 0x00c8

  union
  {
    struct
    {
      unsigned ROI_X5        : 10;		// bits : 9_0
      unsigned               : 2;
      unsigned ROI_Y5        : 10;		// bits : 21_12
    } Bit;
    UINT32 Word;
  } MDBC_Register_51; // 0x00cc

  union
  {
    struct
    {
      unsigned ROI_W5               : 10;		// bits : 9_0
      unsigned                      : 2;
      unsigned ROI_H5               : 10;		// bits : 21_12
      unsigned                      : 2;
      unsigned ROI_UV_THRES5        : 8;		// bits : 31_24
    } Bit;
    UINT32 Word;
  } MDBC_Register_52; // 0x00d0

  union
  {
    struct
    {
      unsigned ROI_LBSP_TH5         : 8;		// bits : 7_0
      unsigned ROI_D_COLOUR5        : 8;		// bits : 15_8
      unsigned ROI_R_COLOUR5        : 8;		// bits : 23_16
      unsigned ROI_D_LBSP5          : 4;		// bits : 27_24
      unsigned ROI_R_LBSP5          : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } MDBC_Register_53; // 0x00d4

  union
  {
    struct
    {
      unsigned ROI_MORPH_EN5        : 1;		// bits : 0
      unsigned                      : 3;
      unsigned ROI_MIN_T5           : 8;		// bits : 11_4
      unsigned ROI_MAX_T5           : 8;		// bits : 19_12
    } Bit;
    UINT32 Word;
  } MDBC_Register_54; // 0x00d8

  union
  {
    struct
    {
      unsigned ROI_X6        : 10;		// bits : 9_0
      unsigned               : 2;
      unsigned ROI_Y6        : 10;		// bits : 21_12
    } Bit;
    UINT32 Word;
  } MDBC_Register_55; // 0x00dc

  union
  {
    struct
    {
      unsigned ROI_W6               : 10;		// bits : 9_0
      unsigned                      : 2;
      unsigned ROI_H6               : 10;		// bits : 21_12
      unsigned                      : 2;
      unsigned ROI_UV_THRES6        : 8;		// bits : 31_24
    } Bit;
    UINT32 Word;
  } MDBC_Register_56; // 0x00e0

  union
  {
    struct
    {
      unsigned ROI_LBSP_TH6         : 8;		// bits : 7_0
      unsigned ROI_D_COLOUR6        : 8;		// bits : 15_8
      unsigned ROI_R_COLOUR6        : 8;		// bits : 23_16
      unsigned ROI_D_LBSP6          : 4;		// bits : 27_24
      unsigned ROI_R_LBSP6          : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } MDBC_Register_57; // 0x00e4

  union
  {
    struct
    {
      unsigned ROI_MORPH_EN6        : 1;		// bits : 0
      unsigned                      : 3;
      unsigned ROI_MIN_T6           : 8;		// bits : 11_4
      unsigned ROI_MAX_T6           : 8;		// bits : 19_12
    } Bit;
    UINT32 Word;
  } MDBC_Register_58; // 0x00e8

  union
  {
    struct
    {
      unsigned ROI_X7        : 10;		// bits : 9_0
      unsigned               : 2;
      unsigned ROI_Y7        : 10;		// bits : 21_12
    } Bit;
    UINT32 Word;
  } MDBC_Register_59; // 0x00ec

  union
  {
    struct
    {
      unsigned ROI_W7               : 10;		// bits : 9_0
      unsigned                      : 2;
      unsigned ROI_H7               : 10;		// bits : 21_12
      unsigned                      : 2;
      unsigned ROI_UV_THRES7        : 8;		// bits : 31_24
    } Bit;
    UINT32 Word;
  } MDBC_Register_60; // 0x00f0

  union
  {
    struct
    {
      unsigned ROI_LBSP_TH7         : 8;		// bits : 7_0
      unsigned ROI_D_COLOUR7        : 8;		// bits : 15_8
      unsigned ROI_R_COLOUR7        : 8;		// bits : 23_16
      unsigned ROI_D_LBSP7          : 4;		// bits : 27_24
      unsigned ROI_R_LBSP7          : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } MDBC_Register_61; // 0x00f4

  union
  {
    struct
    {
      unsigned ROI_MORPH_EN7        : 1;		// bits : 0
      unsigned                      : 3;
      unsigned ROI_MIN_T7           : 8;		// bits : 11_4
      unsigned ROI_MAX_T7           : 8;		// bits : 19_12
    } Bit;
    UINT32 Word;
  } MDBC_Register_62; // 0x00f8

  union
  {
    struct
    {
      unsigned MDBC_AXI_CH_EN           : 10;		// bits : 9_0
      unsigned                          : 2;
      unsigned MDBC_AXI_LOCK_DIS        : 10;		// bits : 21_12
    } Bit;
    UINT32 Word;
  } MDBC_Register_63; // 0x00fc

  union
  {
    struct
    {
      unsigned AXI_BUS_DISABLE        : 1;		// bits : 0
      unsigned                        : 15;
      unsigned AXI_BUS_IDLE           : 1;		// bits : 16
    } Bit;
    UINT32 Word;
  } MDBC_Register_64; // 0x0100

  union
  {
    struct
    {
      unsigned AXI_CH_STA        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_65; // 0x0104

  union
  {
    struct
    {
      unsigned MDBC_R_OSTD_NUM        : 8;		// bits : 7_0
      unsigned MDBC_W_OSTD_NUM        : 8;		// bits : 15_8
    } Bit;
    UINT32 Word;
  } MDBC_Register_66; // 0x0108

  union
  {
    struct
    {
      unsigned reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_67; // 0x010c

  union
  {
    struct
    {
      unsigned DMA_CH_DISABLE        : 1;		// bits : 0
      unsigned                       : 14;
      unsigned DMA_IDLE              : 1;		// bits : 15
    } Bit;
    UINT32 Word;
  } MDBC_Register_68; // 0x0110

  union
  {
    struct
    {
      unsigned reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_69; // 0x0114

  union
  {
    struct
    {
      unsigned reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_70; // 0x0118

  union
  {
    struct
    {
      unsigned reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_71; // 0x011c

  union
  {
    struct
    {
      unsigned CHKSUM_SRCY        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_72; // 0x0120

  union
  {
    struct
    {
      unsigned CHKSUM_SRCUV        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_73; // 0x0124

  union
  {
    struct
    {
      unsigned CHKSUM_PREUV        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_74; // 0x0128

  union
  {
    struct
    {
      unsigned CHKSUM_BGMR        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_75; // 0x012c

  union
  {
    struct
    {
      unsigned CHKSUM_VAR1        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_76; // 0x0130

  union
  {
    struct
    {
      unsigned CHKSUM_FVAR2        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_77; // 0x0134

  union
  {
    struct
    {
      unsigned CHKSUM_BGMW        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_78; // 0x0138

  union
  {
    struct
    {
      unsigned CHKSUM_UV1        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_79; // 0x013c

  union
  {
    struct
    {
      unsigned CHKSUM_BVAR2        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_80; // 0x0140

  union
  {
    struct
    {
      unsigned CHKSUM_UPD        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_81; // 0x0144

  union
  {
    struct
    {
      unsigned CHKSUM_PASS        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_82; // 0x0148

  union
  {
    struct
    {
      unsigned CHKSUM_MPH        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_83; // 0x014c

  union
  {
    struct
    {
      unsigned MDBC_ENG_CYCLE        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_84; // 0x0150

  union
  {
    struct
    {
      unsigned reserved        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_85; // 0x0154

  union
  {
    struct
    {
      unsigned MDBC_WAIT_DMA_CYCLE        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_86; // 0x0158
  
  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_87; // 0x015C

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_88; // 0x0160

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_89; // 0x0164

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_90; // 0x0168

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_91; // 0x016C

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_92; // 0x0170

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_93; // 0x0174

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_94; // 0x0178

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_95; // 0x017C

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_96; // 0x0180

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_97; // 0x0184

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_98; // 0x0188

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_99; // 0x018C

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_100; // 0x0190

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_101; // 0x0194

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_102; // 0x0198

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_103; // 0x019C

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_104; // 0x01A0

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_105; // 0x01A4

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_106; // 0x01A8

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_107; // 0x01AC

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_108; // 0x01B0

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_109; // 0x01B4

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_110; // 0x01B8

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_111; // 0x01BC

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_112; // 0x01C0

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_113; // 0x01C4

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_114; // 0x01C8

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_115; // 0x01CC

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_116; // 0x01D0

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_117; // 0x01D4

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_118; // 0x01D8

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_119; // 0x01DC

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_120; // 0x01E0

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_121; // 0x01E4

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_122; // 0x01E8

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_123; // 0x01EC

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_124; // 0x01F0

  union
  {
    struct
    {
      unsigned RESERVED         : 32;    // 31_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_125; // 0x01F4
  
  union
  {
    struct
    {
      unsigned DRAM_MSB_SAI0        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_126; // 0x01f8

  union
  {
    struct
    {
      unsigned DRAM_MSB_SAI1        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_127; // 0x01fc

  union
  {
    struct
    {
      unsigned DRAM_MSB_SAI2        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_128; // 0x0200

  union
  {
    struct
    {
      unsigned DRAM_MSB_SAI3        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_129; // 0x0204

  union
  {
    struct
    {
      unsigned DRAM_MSB_SAI4        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_130; // 0x0208

  union
  {
    struct
    {
      unsigned DRAM_MSB_SAI5        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_131; // 0x020c

  union
  {
    struct
    {
      unsigned DRAM_MSB_SAO0        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_132; // 0x0210

  union
  {
    struct
    {
      unsigned DRAM_MSB_SAO1        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_133; // 0x0214

  union
  {
    struct
    {
      unsigned DRAM_MSB_SAO2        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_134; // 0x0218

  union
  {
    struct
    {
      unsigned DRAM_MSB_SAO3        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } MDBC_Register_135; // 0x021c

} NT98538_MDBC_REG_STRUCT;



#ifdef __cplusplus
}
#endif


#endif
