#ifndef _TRKE_ENG_INT_REGISTER_H_
#define _TRKE_ENG_INT_REGISTER_H_

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
    TRKE_RST  :    [0x0, 0x1],           bits : 0
    TRKE_START:    [0x0, 0x1],           bits : 1
*/
#define ENGINE_CONTROL_REGISTER_OFS 0x0000
REGDEF_BEGIN(ENGINE_CONTROL_REGISTER)
REGDEF_BIT(TRKE_RST,        1)
REGDEF_BIT(TRKE_START,        1)
REGDEF_BIT(        ,        26)
REGDEF_BIT(LL_FIRE,        1)
REGDEF_END(ENGINE_CONTROL_REGISTER)


/*
    INTE_FRM_END :    [0x0, 0x1],           bits : 0
*/
#define TRKE_INTERRUPT_ENABLE_REGISTER_OFS 0x0004
REGDEF_BEGIN(TRKE_INTERRUPT_ENABLE_REGISTER)
REGDEF_BIT(INTE_FRM_END,        1)
REGDEF_BIT(                ,        7)
REGDEF_BIT(INTE_LLEND      ,        1)
REGDEF_BIT(INTE_LLERROR    ,        1)
REGDEF_BIT(INTE_LL_JOB_END ,        1)

REGDEF_END(TRKE_INTERRUPT_ENABLE_REGISTER)


/*
    INT_FRM_END:    [0x0, 0x1],         bits : 0
*/
#define TRKE_INTERRUPT_STATUS_REGISTER_OFS 0x0008
REGDEF_BEGIN(TRKE_INTERRUPT_STATUS_REGISTER)
REGDEF_BIT(INT_FRM_END     ,        1)
REGDEF_BIT(                ,        7)
REGDEF_BIT(INT_LLEND       ,        1)
REGDEF_BIT(INT_LLERROR     ,        1)
REGDEF_BIT(INT_LL_JOB_END  ,        1)
REGDEF_END(TRKE_INTERRUPT_STATUS_REGISTER)


/*
    LL_TERMINATE:    [0x0, 0xffffffff],         bits : 31_0
*/
#define TRKE_TERMINATE_REGISTER_OFS 0x000c
REGDEF_BEGIN(TRKE_TERMINATE_REGISTER)
REGDEF_BIT(LL_TERMINATE,        1)
REGDEF_END(TRKE_TERMINATE_REGISTER)

/*
    DRAM_LL_SAI:    [0x0, 0xffffffff],         bits : 31_0
*/
#define TRKE_LINKEDLIST_ADDRESS_REGISTER_OFS 0x0010
REGDEF_BEGIN(TRKE_LINKEDLIST_ADDRESS_REGISTER)
REGDEF_BIT(DRAM_LL_SAI,        32)
REGDEF_END(TRKE_LINKEDLIST_ADDRESS_REGISTER)



/*
    POINT_NUM        :    [0x0, 0x1],           bits : 8_0
    MAX_SEARCH_RANGE :    [0x0, 0x1],           bits : 18_12
    ITER_CNT         :    [0x0, 0x3],           bits : 24_20  
	IS_INITIAL_FLOW  :    [0x0, 0x1],           bits : 28
	IS_SKIP_POINT    :    [0x0, 0x1],           bits : 29
    
*/
#define TRKE_FUNCTION_CONTROL_REGISTER0_OFS 0x0014
REGDEF_BEGIN(TRKE_FUNCTION_CONTROL_REGISTER0)
REGDEF_BIT(POINT_NUM,        9)
REGDEF_BIT(,                 3)
REGDEF_BIT(MAX_SEARCH_RANGE, 8)
REGDEF_BIT(ITER_CNT,         5)
REGDEF_BIT(,                 3)
REGDEF_BIT(IS_INITIAL_FLOW,  1)
REGDEF_BIT(IS_SKIP_POINT,    1)
REGDEF_BIT(SEARCH_LIMIT_DISABLE,    1)
REGDEF_BIT(,    1)
REGDEF_END(TRKE_FUNCTION_CONTROL_REGISTER0)


/*
    DRAM_IN_SADDR0:    [0x0, 0x1fffffff],            bits : 31_2
*/
#define TRKE_DMA_INPUT_REGISTER0_OFS 0x0018
REGDEF_BEGIN(TRKE_DMA_INPUT_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(DRAM_IN_SADDR0,        30)
REGDEF_END(TRKE_DMA_INPUT_REGISTER0)


/*
    DRAM_IN_SADDR1:    [0x0, 0x1fffffff],            bits : 31_2
*/
#define TRKE_DMA_INPUT_REGISTER1_OFS 0x001c
REGDEF_BEGIN(TRKE_DMA_INPUT_REGISTER1)
REGDEF_BIT(,        2)
REGDEF_BIT(DRAM_IN_SADDR1,        30)
REGDEF_END(TRKE_DMA_INPUT_REGISTER1)




/*
    DRAM_IN_SADDR2:    [0x0, 0x1fffffff],            bits : 31_2
*/
#define TRKE_DMA_INPUT_REGISTER2_OFS 0x0020
REGDEF_BEGIN(TRKE_DMA_INPUT_REGISTER2)
REGDEF_BIT(,        2)
REGDEF_BIT(DRAM_IN_SADDR2,        30)
REGDEF_END(TRKE_DMA_INPUT_REGISTER2)




/*
    DRAM_IN_SADDR3:    [0x0, 0x1fffffff],            bits : 31_2
*/
#define TRKE_DMA_INPUT_REGISTER3_OFS 0x0024
REGDEF_BEGIN(TRKE_DMA_INPUT_REGISTER3)
REGDEF_BIT(,        2)
REGDEF_BIT(DRAM_IN_SADDR3,        30)
REGDEF_END(TRKE_DMA_INPUT_REGISTER3)



/*
    DRAM_IN_LOFST0:    [0x0, 0x3ffff],           bits : 19_2
*/
#define TRKE_DMA_INPUT_LINE_OFFSET_REGISTER0_OFS 0x0028
REGDEF_BEGIN(TRKE_DMA_INPUT_LINE_OFFSET_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(DRAM_IN_LOFST0,        18)
REGDEF_END(TRKE_DMA_INPUT_LINE_OFFSET_REGISTER0)


/*
    DRAM_IN_LOFST1:    [0x0, 0x3ffff],           bits : 19_2
*/
#define TRKE_DMA_INPUT_LINE_OFFSET_REGISTER1_OFS 0x002c
REGDEF_BEGIN(TRKE_DMA_INPUT_LINE_OFFSET_REGISTER1)
REGDEF_BIT(,        2)
REGDEF_BIT(DRAM_IN_LOFST1,        18)
REGDEF_END(TRKE_DMA_INPUT_LINE_OFFSET_REGISTER1)




/*
    DRAM_OUT_SADDR0:    [0x0, 0x1fffffff],           bits : 31_2
*/
#define TRKE_DMA_OUTPUT_RESULT_REGISTER0_OFS 0x0030
REGDEF_BEGIN(TRKE_DMA_OUTPUT_RESULT_REGISTER0)
REGDEF_BIT(,        2)
REGDEF_BIT(DRAM_OUT_SADDR0,        30)
REGDEF_END(TRKE_DMA_OUTPUT_RESULT_REGISTER0)


/*
    DRAM_OUT_SADDR1:    [0x0, 0x1fffffff],           bits : 31_2
*/
#define TRKE_DMA_OUTPUT_RESULT_REGISTER1_OFS 0x0034
REGDEF_BEGIN(TRKE_DMA_OUTPUT_RESULT_REGISTER1)
REGDEF_BIT(,        2)
REGDEF_BIT(DRAM_OUT_SADDR1,        30)
REGDEF_END(TRKE_DMA_OUTPUT_RESULT_REGISTER1)




/*
    IM_PYR_WIDTH :    [0x0, 0x3fff],           bits : 10_0
    IM_PYR_HEIGHT:    [0x0, 0x1fff],           bits : 26_16
*/
#define INPUT_IMAGE_SIZE_REGISTER_OFS 0x0038
REGDEF_BEGIN(INPUT_IMAGE_SIZE_REGISTER)
REGDEF_BIT(IMG_WIDTH,        12)
REGDEF_BIT(,        4)
REGDEF_BIT(IMG_HEIGHT,        12)
REGDEF_END(INPUT_IMAGE_SIZE_REGISTER)


/*
    EPS:    			  [0x0, 0xf],         bits : 7_0
    MIN_EIG_VAL_THRES:    [0x0, 0xf],         bits : 23_16
    PYR_INDEX:    		  [0x0, 0xf],         bits : 25_24
    MAX_PYR_LEVEL:    	  [0x0, 0xf],         bits : 29_28
    
*/
#define TRKE_FUNCTION_CONTROL_REGISTER1_OFS 0x003c
REGDEF_BEGIN(TRKE_FUNCTION_CONTROL_REGISTER1)
REGDEF_BIT(EPS              ,        8)
REGDEF_BIT(TRKE_PATCHSIZE   ,        2)
REGDEF_BIT(                 ,        6)
REGDEF_BIT(MIN_EIG_VAL_THRES,        8)
REGDEF_BIT(PYR_INDEX        ,        3)
REGDEF_BIT(                 ,        1)
REGDEF_BIT(MAX_PYR_LEVEL    ,        3)
REGDEF_BIT(                 ,        1)
REGDEF_END(TRKE_FUNCTION_CONTROL_REGISTER1)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER0_OFS 0x0040
REGDEF_BEGIN(RESERVED_REGISTER0)
REGDEF_BIT(PYR_SEARCH_RANGE,        16)
REGDEF_BIT(LAYER_SEARCH_RANGE,        16)
REGDEF_END(RESERVED_REGISTER0)


/*
    reserved:    [0x0, 0xffffffff],			bits : 31_0
*/
#define RESERVED_REGISTER1_OFS 0x0044
REGDEF_BEGIN(RESERVED_REGISTER1)
    REGDEF_BIT(reserved,        32)
REGDEF_END(RESERVED_REGISTER1)

/*
    LL_BASE_ADDRESS_REGISTER:    [0x0, 0xffffffff],			bits : 31_0
*/
#define TRKE_LL_BASE_ADDRESS_REGISTER_OFS 0x0048
REGDEF_BEGIN(TRKE_LL_BASE_ADDRESS_REGISTER)
    REGDEF_BIT(LL_BASE_ADDR,        32)
REGDEF_END(TRKE_LL_BASE_ADDRESS_REGISTER)


/*
    IN0_CH_EN:    [0x0, 0x1],			bits : 0
	IN1_CH_EN:    [0x0, 0x1],			bits : 1
	IN2_CH_EN:    [0x0, 0x1],			bits : 2
	IN3_CH_EN:    [0x0, 0x1],			bits : 3
	OUT0_CH_EN:   [0x0, 0x1],			bits : 5
	OUT1_CH_EN:   [0x0, 0x1],			bits : 6
	LLC_CH_EN:    [0x0, 0x1],			bits : 8	
	IN0_LOCK_DIS:    [0x0, 0x1],			bits : 9
	IN1_LOCK_DIS:    [0x0, 0x1],			bits : 10
	IN2_LOCK_DIS:    [0x0, 0x1],			bits : 11
	IN3_LOCK_DIS:    [0x0, 0x1],			bits : 12
	WRITE_CH_OUTSTANDING_NUM:   [0x0, 0xff],			bits : 23_16
	READ_CH_OUTSTANDING_NUM:    [0x0, 0xff],			bits : 31_24
	
*/
#define AXI_REGISTER0_OFS 0x004c
REGDEF_BEGIN(AXI_REGISTER0)
REGDEF_BIT(IN0_CH_EN,        1)
REGDEF_BIT(IN1_CH_EN,        1)
REGDEF_BIT(IN2_CH_EN,        1)
REGDEF_BIT(IN3_CH_EN,        1)
REGDEF_BIT(         ,        1)
REGDEF_BIT(OUT0_CH_EN,        1)
REGDEF_BIT(OUT1_CH_EN,        1)
REGDEF_BIT(          ,        1)
REGDEF_BIT(LLC_CH_EN,        1)
REGDEF_BIT(IN0_LOCK_DIS,        1)
REGDEF_BIT(IN1_LOCK_DIS,        1)
REGDEF_BIT(IN2_LOCK_DIS,        1)
REGDEF_BIT(IN3_LOCK_DIS,        1)
REGDEF_BIT(            ,        3)
REGDEF_BIT(WRITE_CH_OUTSTANDING_NUM  ,        8)
REGDEF_BIT(READ_CH_OUTSTANDING_NUM   ,        8)
REGDEF_END(AXI_REGISTER0)



/*
    OUT0_LOCK_DIS:    [0x0, 0x1],			bits : 0
	OUT1_LOCK_DIS:    [0x0, 0x1],			bits : 1
	LLC_LOCK_DIS:    [0x0, 0x1],			bits : 2

	
*/
#define AXI_REGISTER1_OFS 0x0050
REGDEF_BEGIN(AXI_REGISTER1)
REGDEF_BIT(OUT0_LOCK_DIS,        1)
REGDEF_BIT(OUT1_LOCK_DIS,        1)
REGDEF_BIT(LLC_LOCK_DIS,         1)
REGDEF_END(AXI_REGISTER1)



/*
    AXI_BUS_DISABLE:    [0x0, 0x1],			bits : 0
	AXI_BUS_IDLE:       [0x0, 0x1],			bits : 27
	
*/
#define AXI_REGISTER2_OFS 0x0054
REGDEF_BEGIN(AXI_REGISTER2)
REGDEF_BIT(AXI_BUS_DISABLE,        1)
REGDEF_BIT(,                       26)
REGDEF_BIT(AXI_BUS_IDLE,           1)
REGDEF_END(AXI_REGISTER2)


/*

	AXI_CH_STA:    [0x0, 0xffffffff],			bits : 31_0
	
*/
#define AXI_REGISTER3_OFS 0x0058
REGDEF_BEGIN(AXI_REGISTER3)
REGDEF_BIT(AXI_CH_STA,                      32)
REGDEF_END(AXI_REGISTER3)



/*
    HDMACH_DIS:    [0x0, 0x1],			bits : 30
	HDMACH_DIS:    [0x0, 0x1],			bits : 31
	
*/
#define TRKE_DMACH_REGISTER_OFS 0x005c   //HDMACH_REGISTER_OFS
REGDEF_BEGIN(TRKE_DMACH_REGISTER)
REGDEF_BIT(,                      30)
REGDEF_BIT(HDMACH_DIS,            1)
REGDEF_BIT(HDMACH_IDLE,           1)
REGDEF_END(TRKE_DMACH_REGISTER)



/*
    INDATA_BURST_MODE:    [0x0, 0x1],			bits : 1_0
	OUTRST_BURST_MODE:    [0x0, 0x1],			bits : 3_2
	
*/
#define TRKE_BURST_MODE_REGISTER_OFS 0x0060
REGDEF_BEGIN(TRKE_BURST_MODE_REGISTER)
REGDEF_BIT(INDATA_BURST_MODE,              2)
REGDEF_BIT(OUTRST_BURST_MODE,              2)
REGDEF_BIT(,                               28)
REGDEF_END(TRKE_BURST_MODE_REGISTER)




/*
    LL_TABLE_IDX0:    [0x0, 0xff],			bits : 7_0
	LL_TABLE_IDX1:    [0x0, 0xff],			bits : 15_8
	LL_TABLE_IDX2:    [0x0, 0xff],			bits : 23_16
	LL_TABLE_IDX3:    [0x0, 0xff],			bits : 31_24
	
	
*/
#define LL_TABLE_REGISTER0_OFS 0x0064
REGDEF_BEGIN(LL_TABLE_REGISTER0)
REGDEF_BIT(LL_TABLE_IDX0,              8)
REGDEF_BIT(LL_TABLE_IDX1,              8)
REGDEF_BIT(LL_TABLE_IDX2,              8)
REGDEF_BIT(LL_TABLE_IDX3,              8)
REGDEF_END(LL_TABLE_REGISTER0)



/*
    LL_TABLE_IDX4:    [0x0, 0xff],			bits : 7_0
	LL_TABLE_IDX5:    [0x0, 0xff],			bits : 15_8
	LL_TABLE_IDX6:    [0x0, 0xff],			bits : 23_16
	LL_TABLE_IDX7:    [0x0, 0xff],			bits : 31_24	
	
*/
#define LL_TABLE_REGISTER1_OFS 0x0068
REGDEF_BEGIN(LL_TABLE_REGISTER1)
REGDEF_BIT(LL_TABLE_IDX4,              8)
REGDEF_BIT(LL_TABLE_IDX5,              8)
REGDEF_BIT(LL_TABLE_IDX6,              8)
REGDEF_BIT(LL_TABLE_IDX7,              8)
REGDEF_END(LL_TABLE_REGISTER1)


/*
    LL_TABLE_IDX8:    [0x0, 0xff],			bits : 7_0
	LL_TABLE_IDX9:    [0x0, 0xff],			bits : 15_8
	LL_TABLE_IDX10:    [0x0, 0xff],			bits : 23_16
	LL_TABLE_IDX11:    [0x0, 0xff],			bits : 31_24		
*/

#define LL_TABLE_REGISTER2_OFS 0x006c
REGDEF_BEGIN(LL_TABLE_REGISTER2)
REGDEF_BIT(LL_TABLE_IDX8,              8)
REGDEF_BIT(LL_TABLE_IDX9,              8)
REGDEF_BIT(LL_TABLE_IDX10,              8)
REGDEF_BIT(LL_TABLE_IDX11,              8)
REGDEF_END(LL_TABLE_REGISTER2)

/*
    LL_TABLE_IDX12:    [0x0, 0xff],			bits : 7_0
	LL_TABLE_IDX13:    [0x0, 0xff],			bits : 15_8
	LL_TABLE_IDX14:    [0x0, 0xff],			bits : 23_16
	LL_TABLE_IDX15:    [0x0, 0xff],			bits : 31_24		
*/

#define LL_TABLE_REGISTER3_OFS 0x0070
REGDEF_BEGIN(LL_TABLE_REGISTER3)
REGDEF_BIT(LL_TABLE_IDX12,              8)
REGDEF_BIT(LL_TABLE_IDX13,              8)
REGDEF_BIT(LL_TABLE_IDX14,              8)
REGDEF_BIT(LL_TABLE_IDX15,              8)
REGDEF_END(LL_TABLE_REGISTER3)


/*
    LLC_CHECKSUM:    [0x0, 0xff],			bits : 31_0
	
*/
#define DESIGN_DEBUG_REGISTER0_OFS 0x0074
REGDEF_BEGIN(DESIGN_DEBUG_REGISTER0)
REGDEF_BIT(LLC_CHECKSUM,              32)
REGDEF_END(DESIGN_DEBUG_REGISTER0)


/*
    LLC_CMD_CNT:    [0x0, 0xff],			bits : 31_0
	
*/
#define DESIGN_DEBUG_REGISTER1_OFS 0x0078
REGDEF_BEGIN(DESIGN_DEBUG_REGISTER1)
REGDEF_BIT(LLC_CMD_CNT,              32)
REGDEF_END(DESIGN_DEBUG_REGISTER1)



/*
    LLC_CMD_ADDR:    [0x0, 0xff],			bits : 31_0
	
*/
#define DESIGN_DEBUG_REGISTER2_OFS 0x007c
REGDEF_BEGIN(DESIGN_DEBUG_REGISTER2)
REGDEF_BIT(LLC_CMD_ADDR,              32)
REGDEF_END(DESIGN_DEBUG_REGISTER2)


/*
    LLC_CMD_MSB_ADDR:    [0x0, 0xff],			bits : 31_0
	
*/
#define DESIGN_DEBUG_REGISTER3_OFS 0x0080
REGDEF_BEGIN(DESIGN_DEBUG_REGISTER3)
REGDEF_BIT(LLC_CMD_MSB_ADDR,              32)
REGDEF_END(DESIGN_DEBUG_REGISTER3)


/*
    LLC_CMD_MSB_ADDR:    [0x0, 0xff],			bits : 31_0
	
*/
#define DESIGN_DEBUG_REGISTER4_OFS 0x0084
REGDEF_BEGIN(DESIGN_DEBUG_REGISTER4)
REGDEF_BIT(TRKE_ENG_CNT,              32)
REGDEF_END(DESIGN_DEBUG_REGISTER4)


/*
    LLC_CMD_MSB_ADDR:    [0x0, 0xff],			bits : 31_0
	
*/
#define DESIGN_DEBUG_REGISTER5_OFS 0x0088
REGDEF_BEGIN(DESIGN_DEBUG_REGISTER5)
REGDEF_BIT(TRKE_LL_CNT,              32)
REGDEF_END(DESIGN_DEBUG_REGISTER5)


/*
    STATUS0 :    [0x0, 0x1],          bits : 0
    STATUS1 :    [0x0, 0x1],          bits : 1
    STATUS2 :    [0x0, 0x1],          bits : 2
    STATUS3 :    [0x0, 0x1],          bits : 3
    STATUS4 :    [0x0, 0x1],          bits : 4
    STATUS5 :    [0x0, 0x1],          bits : 5
    STATUS6 :    [0x0, 0x1],          bits : 6
	STATUS7 :    [0x0, 0x1],          bits : 7
    STATUS8 :    [0x0, 0x1],          bits : 8
    STATUS9 :    [0x0, 0x1],          bits : 9
    STATUS10:    [0x0, 0x1],          bits : 10
    STATUS11:    [0x0, 0x1],          bits : 11
    STATUS12:    [0x0, 0x1],          bits : 12
    STATUS13:    [0x0, 0x1],          bits : 13
    STATUS14:    [0x0, 0x1],          bits : 14
    STATUS15:    [0x0, 0x1],          bits : 15
    STATUS16:    [0x0, 0x1],          bits : 16
    STATUS17:    [0x0, 0x1],          bits : 17
    STATUS18:    [0x0, 0x1],          bits : 18
    STATUS19:    [0x0, 0x1],          bits : 19
    STATUS20:    [0x0, 0x1],          bits : 20
    STATUS21:    [0x0, 0x1],          bits : 21
    STATUS22:    [0x0, 0x1],          bits : 22
    STATUS23:    [0x0, 0x1],          bits : 23
	  STATUS24:    [0x0, 0x1],          bits : 24
    STATUS25:    [0x0, 0x1],          bits : 25
    STATUS26:    [0x0, 0x1],          bits : 26
	  STATUS27:    [0x0, 0x1],          bits : 27
    STATUS28:    [0x0, 0x1],          bits : 28
    STATUS29:    [0x0, 0x1],          bits : 29
    STATUS30:    [0x0, 0x1],          bits : 30
    STATUS31:    [0x0, 0x1],          bits : 31
*/

#define TRKE_STATUS_REGISTER0_OFS 0x008c
REGDEF_BEGIN(TRKE_STATUS_REGISTER0)
REGDEF_BIT(STATUS0 ,        1)
REGDEF_BIT(STATUS1 ,        1)
REGDEF_BIT(STATUS2 ,        1)
REGDEF_BIT(STATUS3 ,        1)
REGDEF_BIT(STATUS4 ,        1)
REGDEF_BIT(STATUS5 ,        1)
REGDEF_BIT(STATUS6 ,        1)
REGDEF_BIT(STATUS7 ,        1)
REGDEF_BIT(STATUS8 ,        1)
REGDEF_BIT(STATUS9 ,        1)
REGDEF_BIT(STATUS10,        1)
REGDEF_BIT(STATUS11,        1)
REGDEF_BIT(STATUS12,        1)
REGDEF_BIT(STATUS13,        1)
REGDEF_BIT(STATUS14,        1)
REGDEF_BIT(STATUS15,        1)
REGDEF_BIT(STATUS16,        1)
REGDEF_BIT(STATUS17,        1)
REGDEF_BIT(STATUS18,        1)
REGDEF_BIT(STATUS19,        1)
REGDEF_BIT(STATUS20,        1)
REGDEF_BIT(STATUS21,        1)
REGDEF_BIT(STATUS22,        1)
REGDEF_BIT(STATUS23,        1)
REGDEF_BIT(STATUS24,        1)
REGDEF_BIT(STATUS25,        1)
REGDEF_BIT(STATUS26,        1)
REGDEF_BIT(STATUS27,        1)
REGDEF_BIT(STATUS28,        1)
REGDEF_BIT(STATUS29,        1)
REGDEF_BIT(STATUS30,        1)
REGDEF_BIT(STATUS31,        1)
REGDEF_END(TRKE_STATUS_REGISTER0)


/*
    STATUS32:    [0x0, 0x1],          bits : 0
    STATUS33:    [0x0, 0x1],          bits : 1
    STATUS34:    [0x0, 0x1],          bits : 2
    STATUS35:    [0x0, 0x1],          bits : 3
    STATUS36:    [0x0, 0x1],          bits : 4
    STATUS37:    [0x0, 0x1],          bits : 5
    STATUS38:    [0x0, 0x1],          bits : 6
	  STATUS39:    [0x0, 0x1],          bits : 7
    STATUS40:    [0x0, 0x1],          bits : 8
    STATUS41:    [0x0, 0x1],          bits : 9
    STATUS42:    [0x0, 0x1],          bits : 10
    STATUS43:    [0x0, 0x1],          bits : 11
    STATUS44:    [0x0, 0x1],          bits : 12
    STATUS45:    [0x0, 0x1],          bits : 13
    STATUS46:    [0x0, 0x1],          bits : 14
    STATUS47:    [0x0, 0x1],          bits : 15
    STATUS48:    [0x0, 0x1],          bits : 16
    STATUS49:    [0x0, 0x1],          bits : 17
    STATUS50:    [0x0, 0x1],          bits : 18
    STATUS51:    [0x0, 0x1],          bits : 19
    STATUS52:    [0x0, 0x1],          bits : 20
    STATUS53:    [0x0, 0x1],          bits : 21
    STATUS54:    [0x0, 0x1],          bits : 22
    STATUS55:    [0x0, 0x1],          bits : 23
	  STATUS56:    [0x0, 0x1],          bits : 24
    STATUS57:    [0x0, 0x1],          bits : 25
    STATUS58:    [0x0, 0x1],          bits : 26
	  STATUS59:    [0x0, 0x1],          bits : 27
    STATUS60:    [0x0, 0x1],          bits : 28
    STATUS61:    [0x0, 0x1],          bits : 29
    STATUS62:    [0x0, 0x1],          bits : 30
    STATUS63:    [0x0, 0x1],          bits : 31
*/

#define TRKE_STATUS_REGISTER1_OFS 0x0090
REGDEF_BEGIN(TRKE_STATUS_REGISTER1)
REGDEF_BIT(STATUS32,        1)
REGDEF_BIT(STATUS33,        1)
REGDEF_BIT(STATUS34,        1)
REGDEF_BIT(STATUS35,        1)
REGDEF_BIT(STATUS36,        1)
REGDEF_BIT(STATUS37,        1)
REGDEF_BIT(STATUS38,        1)
REGDEF_BIT(STATUS39,        1)
REGDEF_BIT(STATUS40,        1)
REGDEF_BIT(STATUS41,        1)
REGDEF_BIT(STATUS42,        1)
REGDEF_BIT(STATUS43,        1)
REGDEF_BIT(STATUS44,        1)
REGDEF_BIT(STATUS45,        1)
REGDEF_BIT(STATUS46,        1)
REGDEF_BIT(STATUS47,        1)
REGDEF_BIT(STATUS48,        1)
REGDEF_BIT(STATUS49,        1)
REGDEF_BIT(STATUS50,        1)
REGDEF_BIT(STATUS51,        1)
REGDEF_BIT(STATUS52,        1)
REGDEF_BIT(STATUS53,        1)
REGDEF_BIT(STATUS54,        1)
REGDEF_BIT(STATUS55,        1)
REGDEF_BIT(STATUS56,        1)
REGDEF_BIT(STATUS57,        1)
REGDEF_BIT(STATUS58,        1)
REGDEF_BIT(STATUS59,        1)
REGDEF_BIT(STATUS60,        1)
REGDEF_BIT(STATUS61,        1)
REGDEF_BIT(STATUS62,        1)
REGDEF_BIT(STATUS63,        1)
REGDEF_END(TRKE_STATUS_REGISTER1)


/*
    STATUS64:    [0x0, 0x1],          bits : 0
    STATUS65:    [0x0, 0x1],          bits : 1
    STATUS66:    [0x0, 0x1],          bits : 2
    STATUS67:    [0x0, 0x1],          bits : 3
    STATUS68:    [0x0, 0x1],          bits : 4
    STATUS69:    [0x0, 0x1],          bits : 5
    STATUS70:    [0x0, 0x1],          bits : 6
	  STATUS71:    [0x0, 0x1],          bits : 7
    STATUS72:    [0x0, 0x1],          bits : 8
    STATUS73:    [0x0, 0x1],          bits : 9
    STATUS74:    [0x0, 0x1],          bits : 10
    STATUS75:    [0x0, 0x1],          bits : 11
    STATUS76:    [0x0, 0x1],          bits : 12
    STATUS77:    [0x0, 0x1],          bits : 13
    STATUS78:    [0x0, 0x1],          bits : 14
    STATUS79:    [0x0, 0x1],          bits : 15
    STATUS80:    [0x0, 0x1],          bits : 16
    STATUS81:    [0x0, 0x1],          bits : 17
    STATUS82:    [0x0, 0x1],          bits : 18
    STATUS83:    [0x0, 0x1],          bits : 19
    STATUS84:    [0x0, 0x1],          bits : 20
    STATUS85:    [0x0, 0x1],          bits : 21
    STATUS86:    [0x0, 0x1],          bits : 22
    STATUS87:    [0x0, 0x1],          bits : 23
	  STATUS88:    [0x0, 0x1],          bits : 24
    STATUS89:    [0x0, 0x1],          bits : 25
    STATUS90:    [0x0, 0x1],          bits : 26
	  STATUS91:    [0x0, 0x1],          bits : 27
    STATUS92:    [0x0, 0x1],          bits : 28
    STATUS93:    [0x0, 0x1],          bits : 29
    STATUS94:    [0x0, 0x1],          bits : 30
    STATUS95:    [0x0, 0x1],          bits : 31
*/

#define TRKE_STATUS_REGISTER2_OFS 0x0094
REGDEF_BEGIN(TRKE_STATUS_REGISTER2)
REGDEF_BIT(STATUS64,        1)
REGDEF_BIT(STATUS65,        1)
REGDEF_BIT(STATUS66,        1)
REGDEF_BIT(STATUS67,        1)
REGDEF_BIT(STATUS68,        1)
REGDEF_BIT(STATUS69,        1)
REGDEF_BIT(STATUS70,        1)
REGDEF_BIT(STATUS71,        1)
REGDEF_BIT(STATUS72,        1)
REGDEF_BIT(STATUS73,        1)
REGDEF_BIT(STATUS74,        1)
REGDEF_BIT(STATUS75,        1)
REGDEF_BIT(STATUS76,        1)
REGDEF_BIT(STATUS77,        1)
REGDEF_BIT(STATUS78,        1)
REGDEF_BIT(STATUS79,        1)
REGDEF_BIT(STATUS80,        1)
REGDEF_BIT(STATUS81,        1)
REGDEF_BIT(STATUS82,        1)
REGDEF_BIT(STATUS83,        1)
REGDEF_BIT(STATUS84,        1)
REGDEF_BIT(STATUS85,        1)
REGDEF_BIT(STATUS86,        1)
REGDEF_BIT(STATUS87,        1)
REGDEF_BIT(STATUS88,        1)
REGDEF_BIT(STATUS89,        1)
REGDEF_BIT(STATUS90,        1)
REGDEF_BIT(STATUS91,        1)
REGDEF_BIT(STATUS92,        1)
REGDEF_BIT(STATUS93,        1)
REGDEF_BIT(STATUS94,        1)
REGDEF_BIT(STATUS95,        1)
REGDEF_END(TRKE_STATUS_REGISTER2)


/*
    STATUS96 :    [0x0, 0x1],          bits : 0
    STATUS97 :    [0x0, 0x1],          bits : 1
    STATUS98 :    [0x0, 0x1],          bits : 2
    STATUS99 :    [0x0, 0x1],          bits : 3
    STATUS100:    [0x0, 0x1],          bits : 4
    STATUS101:    [0x0, 0x1],          bits : 5
    STATUS102:    [0x0, 0x1],          bits : 6
	  STATUS103:    [0x0, 0x1],          bits : 7
    STATUS104:    [0x0, 0x1],          bits : 8
    STATUS105:    [0x0, 0x1],          bits : 9
    STATUS106:    [0x0, 0x1],          bits : 10
    STATUS107:    [0x0, 0x1],          bits : 11
    STATUS108:    [0x0, 0x1],          bits : 12
    STATUS109:    [0x0, 0x1],          bits : 13
    STATUS110:    [0x0, 0x1],          bits : 14
    STATUS111:    [0x0, 0x1],          bits : 15
    STATUS112:    [0x0, 0x1],          bits : 16
    STATUS113:    [0x0, 0x1],          bits : 17
    STATUS114:    [0x0, 0x1],          bits : 18
    STATUS115:    [0x0, 0x1],          bits : 19
    STATUS116:    [0x0, 0x1],          bits : 20
    STATUS117:    [0x0, 0x1],          bits : 21
    STATUS118:    [0x0, 0x1],          bits : 22
    STATUS119:    [0x0, 0x1],          bits : 23
	  STATUS120:    [0x0, 0x1],          bits : 24
    STATUS121:    [0x0, 0x1],          bits : 25
    STATUS122:    [0x0, 0x1],          bits : 26
	  STATUS123:    [0x0, 0x1],          bits : 27
    STATUS124:    [0x0, 0x1],          bits : 28
    STATUS125:    [0x0, 0x1],          bits : 29
    STATUS126:    [0x0, 0x1],          bits : 30
    STATUS127:    [0x0, 0x1],          bits : 31
*/

#define TRKE_STATUS_REGISTER3_OFS 0x0098
REGDEF_BEGIN(TRKE_STATUS_REGISTER3)
REGDEF_BIT(STATUS96 ,        1)
REGDEF_BIT(STATUS97 ,        1)
REGDEF_BIT(STATUS98 ,        1)
REGDEF_BIT(STATUS99 ,        1)
REGDEF_BIT(STATUS100,        1)
REGDEF_BIT(STATUS101,        1)
REGDEF_BIT(STATUS102,        1)
REGDEF_BIT(STATUS103,        1)
REGDEF_BIT(STATUS104,        1)
REGDEF_BIT(STATUS105,        1)
REGDEF_BIT(STATUS106,        1)
REGDEF_BIT(STATUS107,        1)
REGDEF_BIT(STATUS108,        1)
REGDEF_BIT(STATUS109,        1)
REGDEF_BIT(STATUS110,        1)
REGDEF_BIT(STATUS111,        1)
REGDEF_BIT(STATUS112,        1)
REGDEF_BIT(STATUS113,        1)
REGDEF_BIT(STATUS114,        1)
REGDEF_BIT(STATUS115,        1)
REGDEF_BIT(STATUS116,        1)
REGDEF_BIT(STATUS117,        1)
REGDEF_BIT(STATUS118,        1)
REGDEF_BIT(STATUS119,        1)
REGDEF_BIT(STATUS120,        1)
REGDEF_BIT(STATUS121,        1)
REGDEF_BIT(STATUS122,        1)
REGDEF_BIT(STATUS123,        1)
REGDEF_BIT(STATUS124,        1)
REGDEF_BIT(STATUS125,        1)
REGDEF_BIT(STATUS126,        1)
REGDEF_BIT(STATUS127,        1)
REGDEF_END(TRKE_STATUS_REGISTER3)

/*
    STATUS128:    [0x0, 0x1],          bits : 0
    STATUS129:    [0x0, 0x1],          bits : 1
    STATUS130:    [0x0, 0x1],          bits : 2
    STATUS131:    [0x0, 0x1],          bits : 3
    STATUS132:    [0x0, 0x1],          bits : 4
    STATUS133:    [0x0, 0x1],          bits : 5
    STATUS134:    [0x0, 0x1],          bits : 6
	STATUS135:    [0x0, 0x1],          bits : 7
    STATUS136:    [0x0, 0x1],          bits : 8
    STATUS137:    [0x0, 0x1],          bits : 9
    STATUS138:    [0x0, 0x1],          bits : 10
    STATUS139:    [0x0, 0x1],          bits : 11
    STATUS140:    [0x0, 0x1],          bits : 12
    STATUS141:    [0x0, 0x1],          bits : 13
    STATUS142:    [0x0, 0x1],          bits : 14
    STATUS143:    [0x0, 0x1],          bits : 15
    STATUS144:    [0x0, 0x1],          bits : 16
    STATUS145:    [0x0, 0x1],          bits : 17
    STATUS146:    [0x0, 0x1],          bits : 18
    STATUS147:    [0x0, 0x1],          bits : 19
    STATUS148:    [0x0, 0x1],          bits : 20
    STATUS149:    [0x0, 0x1],          bits : 21
    STATUS150:    [0x0, 0x1],          bits : 22
    STATUS151:    [0x0, 0x1],          bits : 23
	STATUS152:    [0x0, 0x1],          bits : 24
    STATUS153:    [0x0, 0x1],          bits : 25
    STATUS154:    [0x0, 0x1],          bits : 26
	STATUS155:    [0x0, 0x1],          bits : 27
    STATUS156:    [0x0, 0x1],          bits : 28
    STATUS157:    [0x0, 0x1],          bits : 29
    STATUS158:    [0x0, 0x1],          bits : 30
    STATUS159:    [0x0, 0x1],          bits : 31
*/

#define TRKE_STATUS_REGISTER4_OFS 0x009c
REGDEF_BEGIN(TRKE_STATUS_REGISTER4)
REGDEF_BIT(STATUS128,        1)
REGDEF_BIT(STATUS129,        1)
REGDEF_BIT(STATUS130,        1)
REGDEF_BIT(STATUS131,        1)
REGDEF_BIT(STATUS132,        1)
REGDEF_BIT(STATUS133,        1)
REGDEF_BIT(STATUS134,        1)
REGDEF_BIT(STATUS135,        1)
REGDEF_BIT(STATUS136,        1)
REGDEF_BIT(STATUS137,        1)
REGDEF_BIT(STATUS138,        1)
REGDEF_BIT(STATUS139,        1)
REGDEF_BIT(STATUS140,        1)
REGDEF_BIT(STATUS141,        1)
REGDEF_BIT(STATUS142,        1)
REGDEF_BIT(STATUS143,        1)
REGDEF_BIT(STATUS144,        1)
REGDEF_BIT(STATUS145,        1)
REGDEF_BIT(STATUS146,        1)
REGDEF_BIT(STATUS147,        1)
REGDEF_BIT(STATUS148,        1)
REGDEF_BIT(STATUS149,        1)
REGDEF_BIT(STATUS150,        1)
REGDEF_BIT(STATUS151,        1)
REGDEF_BIT(STATUS152,        1)
REGDEF_BIT(STATUS153,        1)
REGDEF_BIT(STATUS154,        1)
REGDEF_BIT(STATUS155,        1)
REGDEF_BIT(STATUS156,        1)
REGDEF_BIT(STATUS157,        1)
REGDEF_BIT(STATUS158,        1)
REGDEF_BIT(STATUS159,        1)
REGDEF_END(TRKE_STATUS_REGISTER4)


/*
    STATUS160:    [0x0, 0x1],          bits : 0
    STATUS161:    [0x0, 0x1],          bits : 1
    STATUS162:    [0x0, 0x1],          bits : 2
    STATUS163:    [0x0, 0x1],          bits : 3
    STATUS164:    [0x0, 0x1],          bits : 4
    STATUS165:    [0x0, 0x1],          bits : 5
    STATUS166:    [0x0, 0x1],          bits : 6
	STATUS167:    [0x0, 0x1],          bits : 7
    STATUS168:    [0x0, 0x1],          bits : 8
    STATUS169:    [0x0, 0x1],          bits : 9
    STATUS170:    [0x0, 0x1],          bits : 10
    STATUS171:    [0x0, 0x1],          bits : 11
    STATUS172:    [0x0, 0x1],          bits : 12
    STATUS173:    [0x0, 0x1],          bits : 13
    STATUS174:    [0x0, 0x1],          bits : 14
    STATUS175:    [0x0, 0x1],          bits : 15
    STATUS176:    [0x0, 0x1],          bits : 16
    STATUS177:    [0x0, 0x1],          bits : 17
    STATUS178:    [0x0, 0x1],          bits : 18
    STATUS179:    [0x0, 0x1],          bits : 19
    STATUS180:    [0x0, 0x1],          bits : 20
    STATUS181:    [0x0, 0x1],          bits : 21
    STATUS182:    [0x0, 0x1],          bits : 22
    STATUS183:    [0x0, 0x1],          bits : 23
	STATUS184:    [0x0, 0x1],          bits : 24
    STATUS185:    [0x0, 0x1],          bits : 25
    STATUS186:    [0x0, 0x1],          bits : 26
	STATUS187:    [0x0, 0x1],          bits : 27
    STATUS188:    [0x0, 0x1],          bits : 28
    STATUS189:    [0x0, 0x1],          bits : 29
    STATUS190:    [0x0, 0x1],          bits : 30
    STATUS191:    [0x0, 0x1],          bits : 31
*/

#define TRKE_STATUS_REGISTER5_OFS 0x00a0
REGDEF_BEGIN(TRKE_STATUS_REGISTER5)
REGDEF_BIT(STATUS160,        1)
REGDEF_BIT(STATUS161,        1)
REGDEF_BIT(STATUS162,        1)
REGDEF_BIT(STATUS163,        1)
REGDEF_BIT(STATUS164,        1)
REGDEF_BIT(STATUS165,        1)
REGDEF_BIT(STATUS166,        1)
REGDEF_BIT(STATUS167,        1)
REGDEF_BIT(STATUS168,        1)
REGDEF_BIT(STATUS169,        1)
REGDEF_BIT(STATUS170,        1)
REGDEF_BIT(STATUS171,        1)
REGDEF_BIT(STATUS172,        1)
REGDEF_BIT(STATUS173,        1)
REGDEF_BIT(STATUS174,        1)
REGDEF_BIT(STATUS175,        1)
REGDEF_BIT(STATUS176,        1)
REGDEF_BIT(STATUS177,        1)
REGDEF_BIT(STATUS178,        1)
REGDEF_BIT(STATUS179,        1)
REGDEF_BIT(STATUS180,        1)
REGDEF_BIT(STATUS181,        1)
REGDEF_BIT(STATUS182,        1)
REGDEF_BIT(STATUS183,        1)
REGDEF_BIT(STATUS184,        1)
REGDEF_BIT(STATUS185,        1)
REGDEF_BIT(STATUS186,        1)
REGDEF_BIT(STATUS187,        1)
REGDEF_BIT(STATUS188,        1)
REGDEF_BIT(STATUS189,        1)
REGDEF_BIT(STATUS190,        1)
REGDEF_BIT(STATUS191,        1)
REGDEF_END(TRKE_STATUS_REGISTER5)



/*
    STATUS192:    [0x0, 0x1],          bits : 0
    STATUS193:    [0x0, 0x1],          bits : 1
    STATUS194:    [0x0, 0x1],          bits : 2
    STATUS195:    [0x0, 0x1],          bits : 3
    STATUS196:    [0x0, 0x1],          bits : 4
    STATUS197:    [0x0, 0x1],          bits : 5
    STATUS198:    [0x0, 0x1],          bits : 6
	STATUS199:    [0x0, 0x1],          bits : 7
    STATUS200:    [0x0, 0x1],          bits : 8
    STATUS201:    [0x0, 0x1],          bits : 9
    STATUS202:    [0x0, 0x1],          bits : 10
    STATUS203:    [0x0, 0x1],          bits : 11
    STATUS204:    [0x0, 0x1],          bits : 12
    STATUS205:    [0x0, 0x1],          bits : 13
    STATUS206:    [0x0, 0x1],          bits : 14
    STATUS207:    [0x0, 0x1],          bits : 15
    STATUS208:    [0x0, 0x1],          bits : 16
    STATUS209:    [0x0, 0x1],          bits : 17
    STATUS210:    [0x0, 0x1],          bits : 18
    STATUS211:    [0x0, 0x1],          bits : 19
    STATUS212:    [0x0, 0x1],          bits : 20
    STATUS213:    [0x0, 0x1],          bits : 21
    STATUS214:    [0x0, 0x1],          bits : 22
    STATUS215:    [0x0, 0x1],          bits : 23
	STATUS216:    [0x0, 0x1],          bits : 24
    STATUS217:    [0x0, 0x1],          bits : 25
    STATUS218:    [0x0, 0x1],          bits : 26
	STATUS219:    [0x0, 0x1],          bits : 27
    STATUS220:    [0x0, 0x1],          bits : 28
    STATUS221:    [0x0, 0x1],          bits : 29
    STATUS222:    [0x0, 0x1],          bits : 30
    STATUS223:    [0x0, 0x1],          bits : 31
*/

#define TRKE_STATUS_REGISTER6_OFS 0x00a4
REGDEF_BEGIN(TRKE_STATUS_REGISTER6)
REGDEF_BIT(STATUS192,        1)
REGDEF_BIT(STATUS193,        1)
REGDEF_BIT(STATUS194,        1)
REGDEF_BIT(STATUS195,        1)
REGDEF_BIT(STATUS196,        1)
REGDEF_BIT(STATUS197,        1)
REGDEF_BIT(STATUS198,        1)
REGDEF_BIT(STATUS199,        1)
REGDEF_BIT(STATUS200,        1)
REGDEF_BIT(STATUS201,        1)
REGDEF_BIT(STATUS202,        1)
REGDEF_BIT(STATUS203,        1)
REGDEF_BIT(STATUS204,        1)
REGDEF_BIT(STATUS205,        1)
REGDEF_BIT(STATUS206,        1)
REGDEF_BIT(STATUS207,        1)
REGDEF_BIT(STATUS208,        1)
REGDEF_BIT(STATUS209,        1)
REGDEF_BIT(STATUS210,        1)
REGDEF_BIT(STATUS211,        1)
REGDEF_BIT(STATUS212,        1)
REGDEF_BIT(STATUS213,        1)
REGDEF_BIT(STATUS214,        1)
REGDEF_BIT(STATUS215,        1)
REGDEF_BIT(STATUS216,        1)
REGDEF_BIT(STATUS217,        1)
REGDEF_BIT(STATUS218,        1)
REGDEF_BIT(STATUS219,        1)
REGDEF_BIT(STATUS220,        1)
REGDEF_BIT(STATUS221,        1)
REGDEF_BIT(STATUS222,        1)
REGDEF_BIT(STATUS223,        1)
REGDEF_END(TRKE_STATUS_REGISTER6)


/*
    STATUS224:    [0x0, 0x1],          bits : 0
    STATUS225:    [0x0, 0x1],          bits : 1
    STATUS226:    [0x0, 0x1],          bits : 2
    STATUS227:    [0x0, 0x1],          bits : 3
    STATUS228:    [0x0, 0x1],          bits : 4
    STATUS229:    [0x0, 0x1],          bits : 5
    STATUS230:    [0x0, 0x1],          bits : 6
	STATUS231:    [0x0, 0x1],          bits : 7
    STATUS232:    [0x0, 0x1],          bits : 8
    STATUS233:    [0x0, 0x1],          bits : 9
    STATUS234:    [0x0, 0x1],          bits : 10
    STATUS235:    [0x0, 0x1],          bits : 11
    STATUS236:    [0x0, 0x1],          bits : 12
    STATUS237:    [0x0, 0x1],          bits : 13
    STATUS238:    [0x0, 0x1],          bits : 14
    STATUS239:    [0x0, 0x1],          bits : 15
    STATUS240:    [0x0, 0x1],          bits : 16
    STATUS241:    [0x0, 0x1],          bits : 17
    STATUS242:    [0x0, 0x1],          bits : 18
    STATUS243:    [0x0, 0x1],          bits : 19
    STATUS244:    [0x0, 0x1],          bits : 20
    STATUS245:    [0x0, 0x1],          bits : 21
    STATUS246:    [0x0, 0x1],          bits : 22
    STATUS247:    [0x0, 0x1],          bits : 23
	STATUS248:    [0x0, 0x1],          bits : 24
    STATUS249:    [0x0, 0x1],          bits : 25
    STATUS250:    [0x0, 0x1],          bits : 26
	STATUS251:    [0x0, 0x1],          bits : 27
    STATUS252:    [0x0, 0x1],          bits : 28
    STATUS253:    [0x0, 0x1],          bits : 29
    STATUS254:    [0x0, 0x1],          bits : 30
    STATUS255:    [0x0, 0x1],          bits : 31
*/

#define TRKE_STATUS_REGISTER7_OFS 0x00a8
REGDEF_BEGIN(TRKE_STATUS_REGISTER7)
REGDEF_BIT(STATUS224,        1)
REGDEF_BIT(STATUS225,        1)
REGDEF_BIT(STATUS226,        1)
REGDEF_BIT(STATUS227,        1)
REGDEF_BIT(STATUS228,        1)
REGDEF_BIT(STATUS229,        1)
REGDEF_BIT(STATUS230,        1)
REGDEF_BIT(STATUS231,        1)
REGDEF_BIT(STATUS232,        1)
REGDEF_BIT(STATUS233,        1)
REGDEF_BIT(STATUS234,        1)
REGDEF_BIT(STATUS235,        1)
REGDEF_BIT(STATUS236,        1)
REGDEF_BIT(STATUS237,        1)
REGDEF_BIT(STATUS238,        1)
REGDEF_BIT(STATUS239,        1)
REGDEF_BIT(STATUS240,        1)
REGDEF_BIT(STATUS241,        1)
REGDEF_BIT(STATUS242,        1)
REGDEF_BIT(STATUS243,        1)
REGDEF_BIT(STATUS244,        1)
REGDEF_BIT(STATUS245,        1)
REGDEF_BIT(STATUS246,        1)
REGDEF_BIT(STATUS247,        1)
REGDEF_BIT(STATUS248,        1)
REGDEF_BIT(STATUS249,        1)
REGDEF_BIT(STATUS250,        1)
REGDEF_BIT(STATUS251,        1)
REGDEF_BIT(STATUS252,        1)
REGDEF_BIT(STATUS253,        1)
REGDEF_BIT(STATUS254,        1)
REGDEF_BIT(STATUS255,        1)
REGDEF_END(TRKE_STATUS_REGISTER7)



/*
    STATUS256:    [0x0, 0x1],          bits : 0
    STATUS257:    [0x0, 0x1],          bits : 1
    STATUS258:    [0x0, 0x1],          bits : 2
    STATUS259:    [0x0, 0x1],          bits : 3
    STATUS260:    [0x0, 0x1],          bits : 4
    STATUS261:    [0x0, 0x1],          bits : 5
    STATUS262:    [0x0, 0x1],          bits : 6
	STATUS263:    [0x0, 0x1],          bits : 7
    STATUS264:    [0x0, 0x1],          bits : 8
    STATUS265:    [0x0, 0x1],          bits : 9
    STATUS266:    [0x0, 0x1],          bits : 10
    STATUS267:    [0x0, 0x1],          bits : 11
    STATUS268:    [0x0, 0x1],          bits : 12
    STATUS269:    [0x0, 0x1],          bits : 13
    STATUS270:    [0x0, 0x1],          bits : 14
    STATUS271:    [0x0, 0x1],          bits : 15
    STATUS272:    [0x0, 0x1],          bits : 16
    STATUS273:    [0x0, 0x1],          bits : 17
    STATUS274:    [0x0, 0x1],          bits : 18
    STATUS275:    [0x0, 0x1],          bits : 19
    STATUS276:    [0x0, 0x1],          bits : 20
    STATUS277:    [0x0, 0x1],          bits : 21
    STATUS278:    [0x0, 0x1],          bits : 22
    STATUS279:    [0x0, 0x1],          bits : 23
	STATUS280:    [0x0, 0x1],          bits : 24
    STATUS281:    [0x0, 0x1],          bits : 25
    STATUS282:    [0x0, 0x1],          bits : 26
	STATUS283:    [0x0, 0x1],          bits : 27
    STATUS284:    [0x0, 0x1],          bits : 28
    STATUS285:    [0x0, 0x1],          bits : 29
    STATUS286:    [0x0, 0x1],          bits : 30
    STATUS287:    [0x0, 0x1],          bits : 31
*/

#define TRKE_STATUS_REGISTER8_OFS 0x00ac
REGDEF_BEGIN(TRKE_STATUS_REGISTER8)
REGDEF_BIT(STATUS256,        1)
REGDEF_BIT(STATUS257,        1)
REGDEF_BIT(STATUS258,        1)
REGDEF_BIT(STATUS259,        1)
REGDEF_BIT(STATUS260,        1)
REGDEF_BIT(STATUS261,        1)
REGDEF_BIT(STATUS262,        1)
REGDEF_BIT(STATUS263,        1)
REGDEF_BIT(STATUS264,        1)
REGDEF_BIT(STATUS265,        1)
REGDEF_BIT(STATUS266,        1)
REGDEF_BIT(STATUS267,        1)
REGDEF_BIT(STATUS268,        1)
REGDEF_BIT(STATUS269,        1)
REGDEF_BIT(STATUS270,        1)
REGDEF_BIT(STATUS271,        1)
REGDEF_BIT(STATUS272,        1)
REGDEF_BIT(STATUS273,        1)
REGDEF_BIT(STATUS274,        1)
REGDEF_BIT(STATUS275,        1)
REGDEF_BIT(STATUS276,        1)
REGDEF_BIT(STATUS277,        1)
REGDEF_BIT(STATUS278,        1)
REGDEF_BIT(STATUS279,        1)
REGDEF_BIT(STATUS280,        1)
REGDEF_BIT(STATUS281,        1)
REGDEF_BIT(STATUS282,        1)
REGDEF_BIT(STATUS283,        1)
REGDEF_BIT(STATUS284,        1)
REGDEF_BIT(STATUS285,        1)
REGDEF_BIT(STATUS286,        1)
REGDEF_BIT(STATUS287,        1)
REGDEF_END(TRKE_STATUS_REGISTER8)



/*
    STATUS288:    [0x0, 0x1],          bits : 0
    STATUS289:    [0x0, 0x1],          bits : 1
    STATUS290:    [0x0, 0x1],          bits : 2
    STATUS291:    [0x0, 0x1],          bits : 3
    STATUS292:    [0x0, 0x1],          bits : 4
    STATUS293:    [0x0, 0x1],          bits : 5
    STATUS294:    [0x0, 0x1],          bits : 6
	STATUS295:    [0x0, 0x1],          bits : 7
    STATUS296:    [0x0, 0x1],          bits : 8
    STATUS297:    [0x0, 0x1],          bits : 9
    STATUS298:    [0x0, 0x1],          bits : 10
    STATUS299:    [0x0, 0x1],          bits : 11
    STATUS300:    [0x0, 0x1],          bits : 12
    STATUS301:    [0x0, 0x1],          bits : 13
    STATUS302:    [0x0, 0x1],          bits : 14
    STATUS303:    [0x0, 0x1],          bits : 15
    STATUS304:    [0x0, 0x1],          bits : 16
    STATUS305:    [0x0, 0x1],          bits : 17
    STATUS306:    [0x0, 0x1],          bits : 18
    STATUS307:    [0x0, 0x1],          bits : 19
    STATUS308:    [0x0, 0x1],          bits : 20
    STATUS309:    [0x0, 0x1],          bits : 21
    STATUS310:    [0x0, 0x1],          bits : 22
    STATUS311:    [0x0, 0x1],          bits : 23
	STATUS312:    [0x0, 0x1],          bits : 24
    STATUS313:    [0x0, 0x1],          bits : 25
    STATUS314:    [0x0, 0x1],          bits : 26
	STATUS315:    [0x0, 0x1],          bits : 27
    STATUS316:    [0x0, 0x1],          bits : 28
    STATUS317:    [0x0, 0x1],          bits : 29
    STATUS318:    [0x0, 0x1],          bits : 30
    STATUS319:    [0x0, 0x1],          bits : 31
*/

#define TRKE_STATUS_REGISTER9_OFS 0x00b0
REGDEF_BEGIN(TRKE_STATUS_REGISTER9)
REGDEF_BIT(STATUS288,        1)
REGDEF_BIT(STATUS289,        1)
REGDEF_BIT(STATUS290,        1)
REGDEF_BIT(STATUS291,        1)
REGDEF_BIT(STATUS292,        1)
REGDEF_BIT(STATUS293,        1)
REGDEF_BIT(STATUS294,        1)
REGDEF_BIT(STATUS295,        1)
REGDEF_BIT(STATUS296,        1)
REGDEF_BIT(STATUS297,        1)
REGDEF_BIT(STATUS298,        1)
REGDEF_BIT(STATUS299,        1)
REGDEF_BIT(STATUS300,        1)
REGDEF_BIT(STATUS301,        1)
REGDEF_BIT(STATUS302,        1)
REGDEF_BIT(STATUS303,        1)
REGDEF_BIT(STATUS304,        1)
REGDEF_BIT(STATUS305,        1)
REGDEF_BIT(STATUS306,        1)
REGDEF_BIT(STATUS307,        1)
REGDEF_BIT(STATUS308,        1)
REGDEF_BIT(STATUS309,        1)
REGDEF_BIT(STATUS310,        1)
REGDEF_BIT(STATUS311,        1)
REGDEF_BIT(STATUS312,        1)
REGDEF_BIT(STATUS313,        1)
REGDEF_BIT(STATUS314,        1)
REGDEF_BIT(STATUS315,        1)
REGDEF_BIT(STATUS316,        1)
REGDEF_BIT(STATUS317,        1)
REGDEF_BIT(STATUS318,        1)
REGDEF_BIT(STATUS319,        1)
REGDEF_END(TRKE_STATUS_REGISTER9)


/*
    STATUS320:    [0x0, 0x1],          bits : 0
    STATUS321:    [0x0, 0x1],          bits : 1
    STATUS322:    [0x0, 0x1],          bits : 2
    STATUS323:    [0x0, 0x1],          bits : 3
    STATUS324:    [0x0, 0x1],          bits : 4
    STATUS325:    [0x0, 0x1],          bits : 5
    STATUS326:    [0x0, 0x1],          bits : 6
	STATUS327:    [0x0, 0x1],          bits : 7
    STATUS328:    [0x0, 0x1],          bits : 8
    STATUS329:    [0x0, 0x1],          bits : 9
    STATUS330:    [0x0, 0x1],          bits : 10
    STATUS331:    [0x0, 0x1],          bits : 11
    STATUS332:    [0x0, 0x1],          bits : 12
    STATUS333:    [0x0, 0x1],          bits : 13
    STATUS334:    [0x0, 0x1],          bits : 14
    STATUS335:    [0x0, 0x1],          bits : 15
    STATUS336:    [0x0, 0x1],          bits : 16
    STATUS337:    [0x0, 0x1],          bits : 17
    STATUS338:    [0x0, 0x1],          bits : 18
    STATUS339:    [0x0, 0x1],          bits : 19
    STATUS340:    [0x0, 0x1],          bits : 20
    STATUS341:    [0x0, 0x1],          bits : 21
    STATUS342:    [0x0, 0x1],          bits : 22
    STATUS343:    [0x0, 0x1],          bits : 23
	STATUS344:    [0x0, 0x1],          bits : 24
    STATUS345:    [0x0, 0x1],          bits : 25
    STATUS346:    [0x0, 0x1],          bits : 26
	STATUS347:    [0x0, 0x1],          bits : 27
    STATUS348:    [0x0, 0x1],          bits : 28
    STATUS349:    [0x0, 0x1],          bits : 29
    STATUS350:    [0x0, 0x1],          bits : 30
    STATUS351:    [0x0, 0x1],          bits : 31
*/

#define TRKE_STATUS_REGISTER10_OFS 0x00b4
REGDEF_BEGIN(TRKE_STATUS_REGISTER10)
REGDEF_BIT(STATUS320,        1)
REGDEF_BIT(STATUS321,        1)
REGDEF_BIT(STATUS322,        1)
REGDEF_BIT(STATUS323,        1)
REGDEF_BIT(STATUS324,        1)
REGDEF_BIT(STATUS325,        1)
REGDEF_BIT(STATUS326,        1)
REGDEF_BIT(STATUS327,        1)
REGDEF_BIT(STATUS328,        1)
REGDEF_BIT(STATUS329,        1)
REGDEF_BIT(STATUS330,        1)
REGDEF_BIT(STATUS331,        1)
REGDEF_BIT(STATUS332,        1)
REGDEF_BIT(STATUS333,        1)
REGDEF_BIT(STATUS334,        1)
REGDEF_BIT(STATUS335,        1)
REGDEF_BIT(STATUS336,        1)
REGDEF_BIT(STATUS337,        1)
REGDEF_BIT(STATUS338,        1)
REGDEF_BIT(STATUS339,        1)
REGDEF_BIT(STATUS340,        1)
REGDEF_BIT(STATUS341,        1)
REGDEF_BIT(STATUS342,        1)
REGDEF_BIT(STATUS343,        1)
REGDEF_BIT(STATUS344,        1)
REGDEF_BIT(STATUS345,        1)
REGDEF_BIT(STATUS346,        1)
REGDEF_BIT(STATUS347,        1)
REGDEF_BIT(STATUS348,        1)
REGDEF_BIT(STATUS349,        1)
REGDEF_BIT(STATUS350,        1)
REGDEF_BIT(STATUS351,        1)
REGDEF_END(TRKE_STATUS_REGISTER10)


/*
    STATUS352:    [0x0, 0x1],          bits : 0
    STATUS353:    [0x0, 0x1],          bits : 1
    STATUS354:    [0x0, 0x1],          bits : 2
    STATUS355:    [0x0, 0x1],          bits : 3
    STATUS356:    [0x0, 0x1],          bits : 4
    STATUS357:    [0x0, 0x1],          bits : 5
    STATUS358:    [0x0, 0x1],          bits : 6
	STATUS359:    [0x0, 0x1],          bits : 7
    STATUS360:    [0x0, 0x1],          bits : 8
    STATUS361:    [0x0, 0x1],          bits : 9
    STATUS362:    [0x0, 0x1],          bits : 10
    STATUS363:    [0x0, 0x1],          bits : 11
    STATUS364:    [0x0, 0x1],          bits : 12
    STATUS365:    [0x0, 0x1],          bits : 13
    STATUS366:    [0x0, 0x1],          bits : 14
    STATUS367:    [0x0, 0x1],          bits : 15
    STATUS368:    [0x0, 0x1],          bits : 16
    STATUS369:    [0x0, 0x1],          bits : 17
    STATUS370:    [0x0, 0x1],          bits : 18
    STATUS371:    [0x0, 0x1],          bits : 19
    STATUS372:    [0x0, 0x1],          bits : 20
    STATUS373:    [0x0, 0x1],          bits : 21
    STATUS374:    [0x0, 0x1],          bits : 22
    STATUS375:    [0x0, 0x1],          bits : 23
	STATUS376:    [0x0, 0x1],          bits : 24
    STATUS377:    [0x0, 0x1],          bits : 25
    STATUS378:    [0x0, 0x1],          bits : 26
	STATUS379:    [0x0, 0x1],          bits : 27
    STATUS380:    [0x0, 0x1],          bits : 28
    STATUS381:    [0x0, 0x1],          bits : 29
    STATUS382:    [0x0, 0x1],          bits : 30
    STATUS383:    [0x0, 0x1],          bits : 31
*/

#define TRKE_STATUS_REGISTER11_OFS 0x00b8
REGDEF_BEGIN(TRKE_STATUS_REGISTER11)
REGDEF_BIT(STATUS352,        1)
REGDEF_BIT(STATUS353,        1)
REGDEF_BIT(STATUS354,        1)
REGDEF_BIT(STATUS355,        1)
REGDEF_BIT(STATUS356,        1)
REGDEF_BIT(STATUS357,        1)
REGDEF_BIT(STATUS358,        1)
REGDEF_BIT(STATUS359,        1)
REGDEF_BIT(STATUS360,        1)
REGDEF_BIT(STATUS361,        1)
REGDEF_BIT(STATUS362,        1)
REGDEF_BIT(STATUS363,        1)
REGDEF_BIT(STATUS364,        1)
REGDEF_BIT(STATUS365,        1)
REGDEF_BIT(STATUS366,        1)
REGDEF_BIT(STATUS367,        1)
REGDEF_BIT(STATUS368,        1)
REGDEF_BIT(STATUS369,        1)
REGDEF_BIT(STATUS370,        1)
REGDEF_BIT(STATUS371,        1)
REGDEF_BIT(STATUS372,        1)
REGDEF_BIT(STATUS373,        1)
REGDEF_BIT(STATUS374,        1)
REGDEF_BIT(STATUS375,        1)
REGDEF_BIT(STATUS376,        1)
REGDEF_BIT(STATUS377,        1)
REGDEF_BIT(STATUS378,        1)
REGDEF_BIT(STATUS379,        1)
REGDEF_BIT(STATUS380,        1)
REGDEF_BIT(STATUS381,        1)
REGDEF_BIT(STATUS382,        1)
REGDEF_BIT(STATUS383,        1)
REGDEF_END(TRKE_STATUS_REGISTER11)


/*
    STATUS384:    [0x0, 0x1],          bits : 0
    STATUS385:    [0x0, 0x1],          bits : 1
    STATUS386:    [0x0, 0x1],          bits : 2
    STATUS387:    [0x0, 0x1],          bits : 3
    STATUS388:    [0x0, 0x1],          bits : 4
    STATUS389:    [0x0, 0x1],          bits : 5
    STATUS390:    [0x0, 0x1],          bits : 6
	STATUS391:    [0x0, 0x1],          bits : 7
    STATUS392:    [0x0, 0x1],          bits : 8
    STATUS393:    [0x0, 0x1],          bits : 9
    STATUS394:    [0x0, 0x1],          bits : 10
    STATUS395:    [0x0, 0x1],          bits : 11
    STATUS396:    [0x0, 0x1],          bits : 12
    STATUS397:    [0x0, 0x1],          bits : 13
    STATUS398:    [0x0, 0x1],          bits : 14
    STATUS399:    [0x0, 0x1],          bits : 15
    STATUS400:    [0x0, 0x1],          bits : 16
    STATUS401:    [0x0, 0x1],          bits : 17
    STATUS402:    [0x0, 0x1],          bits : 18
    STATUS403:    [0x0, 0x1],          bits : 19
    STATUS404:    [0x0, 0x1],          bits : 20
    STATUS405:    [0x0, 0x1],          bits : 21
    STATUS406:    [0x0, 0x1],          bits : 22
    STATUS407:    [0x0, 0x1],          bits : 23
	STATUS408:    [0x0, 0x1],          bits : 24
    STATUS409:    [0x0, 0x1],          bits : 25
    STATUS410:    [0x0, 0x1],          bits : 26
	STATUS411:    [0x0, 0x1],          bits : 27
    STATUS412:    [0x0, 0x1],          bits : 28
    STATUS413:    [0x0, 0x1],          bits : 29
    STATUS414:    [0x0, 0x1],          bits : 30
    STATUS415:    [0x0, 0x1],          bits : 31
*/

#define TRKE_STATUS_REGISTER12_OFS 0x00bc
REGDEF_BEGIN(TRKE_STATUS_REGISTER12)
REGDEF_BIT(STATUS384,        1)
REGDEF_BIT(STATUS385,        1)
REGDEF_BIT(STATUS386,        1)
REGDEF_BIT(STATUS387,        1)
REGDEF_BIT(STATUS388,        1)
REGDEF_BIT(STATUS389,        1)
REGDEF_BIT(STATUS390,        1)
REGDEF_BIT(STATUS391,        1)
REGDEF_BIT(STATUS392,        1)
REGDEF_BIT(STATUS393,        1)
REGDEF_BIT(STATUS394,        1)
REGDEF_BIT(STATUS395,        1)
REGDEF_BIT(STATUS396,        1)
REGDEF_BIT(STATUS397,        1)
REGDEF_BIT(STATUS398,        1)
REGDEF_BIT(STATUS399,        1)
REGDEF_BIT(STATUS400,        1)
REGDEF_BIT(STATUS401,        1)
REGDEF_BIT(STATUS402,        1)
REGDEF_BIT(STATUS403,        1)
REGDEF_BIT(STATUS404,        1)
REGDEF_BIT(STATUS405,        1)
REGDEF_BIT(STATUS406,        1)
REGDEF_BIT(STATUS407,        1)
REGDEF_BIT(STATUS408,        1)
REGDEF_BIT(STATUS409,        1)
REGDEF_BIT(STATUS410,        1)
REGDEF_BIT(STATUS411,        1)
REGDEF_BIT(STATUS412,        1)
REGDEF_BIT(STATUS413,        1)
REGDEF_BIT(STATUS414,        1)
REGDEF_BIT(STATUS415,        1)
REGDEF_END(TRKE_STATUS_REGISTER12)


/*
    STATUS416:    [0x0, 0x1],          bits : 0
    STATUS417:    [0x0, 0x1],          bits : 1
    STATUS418:    [0x0, 0x1],          bits : 2
    STATUS419:    [0x0, 0x1],          bits : 3
    STATUS420:    [0x0, 0x1],          bits : 4
    STATUS421:    [0x0, 0x1],          bits : 5
    STATUS422:    [0x0, 0x1],          bits : 6
	STATUS423:    [0x0, 0x1],          bits : 7
    STATUS424:    [0x0, 0x1],          bits : 8
    STATUS425:    [0x0, 0x1],          bits : 9
    STATUS426:    [0x0, 0x1],          bits : 10
    STATUS427:    [0x0, 0x1],          bits : 11
    STATUS428:    [0x0, 0x1],          bits : 12
    STATUS429:    [0x0, 0x1],          bits : 13
    STATUS430:    [0x0, 0x1],          bits : 14
    STATUS431:    [0x0, 0x1],          bits : 15
    STATUS432:    [0x0, 0x1],          bits : 16
    STATUS433:    [0x0, 0x1],          bits : 17
    STATUS434:    [0x0, 0x1],          bits : 18
    STATUS435:    [0x0, 0x1],          bits : 19
    STATUS436:    [0x0, 0x1],          bits : 20
    STATUS437:    [0x0, 0x1],          bits : 21
    STATUS438:    [0x0, 0x1],          bits : 22
    STATUS439:    [0x0, 0x1],          bits : 23
	STATUS440:    [0x0, 0x1],          bits : 24
    STATUS441:    [0x0, 0x1],          bits : 25
    STATUS442:    [0x0, 0x1],          bits : 26
	STATUS443:    [0x0, 0x1],          bits : 27
    STATUS444:    [0x0, 0x1],          bits : 28
    STATUS445:    [0x0, 0x1],          bits : 29
    STATUS446:    [0x0, 0x1],          bits : 30
    STATUS447:    [0x0, 0x1],          bits : 31
*/

#define TRKE_STATUS_REGISTER13_OFS 0x00c0
REGDEF_BEGIN(TRKE_STATUS_REGISTER13)
REGDEF_BIT(STATUS416,        1)
REGDEF_BIT(STATUS417,        1)
REGDEF_BIT(STATUS418,        1)
REGDEF_BIT(STATUS419,        1)
REGDEF_BIT(STATUS420,        1)
REGDEF_BIT(STATUS421,        1)
REGDEF_BIT(STATUS422,        1)
REGDEF_BIT(STATUS423,        1)
REGDEF_BIT(STATUS424,        1)
REGDEF_BIT(STATUS425,        1)
REGDEF_BIT(STATUS426,        1)
REGDEF_BIT(STATUS427,        1)
REGDEF_BIT(STATUS428,        1)
REGDEF_BIT(STATUS429,        1)
REGDEF_BIT(STATUS430,        1)
REGDEF_BIT(STATUS431,        1)
REGDEF_BIT(STATUS432,        1)
REGDEF_BIT(STATUS433,        1)
REGDEF_BIT(STATUS434,        1)
REGDEF_BIT(STATUS435,        1)
REGDEF_BIT(STATUS436,        1)
REGDEF_BIT(STATUS437,        1)
REGDEF_BIT(STATUS438,        1)
REGDEF_BIT(STATUS439,        1)
REGDEF_BIT(STATUS440,        1)
REGDEF_BIT(STATUS441,        1)
REGDEF_BIT(STATUS442,        1)
REGDEF_BIT(STATUS443,        1)
REGDEF_BIT(STATUS444,        1)
REGDEF_BIT(STATUS445,        1)
REGDEF_BIT(STATUS446,        1)
REGDEF_BIT(STATUS447,        1)
REGDEF_END(TRKE_STATUS_REGISTER13)


/*
    STATUS448:    [0x0, 0x1],          bits : 0
    STATUS449:    [0x0, 0x1],          bits : 1
    STATUS450:    [0x0, 0x1],          bits : 2
    STATUS451:    [0x0, 0x1],          bits : 3
    STATUS452:    [0x0, 0x1],          bits : 4
    STATUS453:    [0x0, 0x1],          bits : 5
    STATUS454:    [0x0, 0x1],          bits : 6
	STATUS455:    [0x0, 0x1],          bits : 7
    STATUS456:    [0x0, 0x1],          bits : 8
    STATUS457:    [0x0, 0x1],          bits : 9
    STATUS458:    [0x0, 0x1],          bits : 10
    STATUS459:    [0x0, 0x1],          bits : 11
    STATUS460:    [0x0, 0x1],          bits : 12
    STATUS461:    [0x0, 0x1],          bits : 13
    STATUS462:    [0x0, 0x1],          bits : 14
    STATUS463:    [0x0, 0x1],          bits : 15
    STATUS464:    [0x0, 0x1],          bits : 16
    STATUS465:    [0x0, 0x1],          bits : 17
    STATUS466:    [0x0, 0x1],          bits : 18
    STATUS467:    [0x0, 0x1],          bits : 19
    STATUS468:    [0x0, 0x1],          bits : 20
    STATUS469:    [0x0, 0x1],          bits : 21
    STATUS470:    [0x0, 0x1],          bits : 22
    STATUS471:    [0x0, 0x1],          bits : 23
	STATUS472:    [0x0, 0x1],          bits : 24
    STATUS473:    [0x0, 0x1],          bits : 25
    STATUS474:    [0x0, 0x1],          bits : 26
	STATUS475:    [0x0, 0x1],          bits : 27
    STATUS476:    [0x0, 0x1],          bits : 28
    STATUS477:    [0x0, 0x1],          bits : 29
    STATUS478:    [0x0, 0x1],          bits : 30
    STATUS479:    [0x0, 0x1],          bits : 31
*/

#define TRKE_STATUS_REGISTER14_OFS 0x00c4
REGDEF_BEGIN(TRKE_STATUS_REGISTER14)
REGDEF_BIT(STATUS448,        1)
REGDEF_BIT(STATUS449,        1)
REGDEF_BIT(STATUS450,        1)
REGDEF_BIT(STATUS451,        1)
REGDEF_BIT(STATUS452,        1)
REGDEF_BIT(STATUS453,        1)
REGDEF_BIT(STATUS454,        1)
REGDEF_BIT(STATUS455,        1)
REGDEF_BIT(STATUS456,        1)
REGDEF_BIT(STATUS457,        1)
REGDEF_BIT(STATUS458,        1)
REGDEF_BIT(STATUS459,        1)
REGDEF_BIT(STATUS460,        1)
REGDEF_BIT(STATUS461,        1)
REGDEF_BIT(STATUS462,        1)
REGDEF_BIT(STATUS463,        1)
REGDEF_BIT(STATUS464,        1)
REGDEF_BIT(STATUS465,        1)
REGDEF_BIT(STATUS466,        1)
REGDEF_BIT(STATUS467,        1)
REGDEF_BIT(STATUS468,        1)
REGDEF_BIT(STATUS469,        1)
REGDEF_BIT(STATUS470,        1)
REGDEF_BIT(STATUS471,        1)
REGDEF_BIT(STATUS472,        1)
REGDEF_BIT(STATUS473,        1)
REGDEF_BIT(STATUS474,        1)
REGDEF_BIT(STATUS475,        1)
REGDEF_BIT(STATUS476,        1)
REGDEF_BIT(STATUS477,        1)
REGDEF_BIT(STATUS478,        1)
REGDEF_BIT(STATUS479,        1)
REGDEF_END(TRKE_STATUS_REGISTER14)


/*
    STATUS480:    [0x0, 0x1],          bits : 0
    STATUS481:    [0x0, 0x1],          bits : 1
    STATUS482:    [0x0, 0x1],          bits : 2
    STATUS483:    [0x0, 0x1],          bits : 3
    STATUS484:    [0x0, 0x1],          bits : 4
    STATUS485:    [0x0, 0x1],          bits : 5
    STATUS486:    [0x0, 0x1],          bits : 6
	STATUS487:    [0x0, 0x1],          bits : 7
    STATUS488:    [0x0, 0x1],          bits : 8
    STATUS489:    [0x0, 0x1],          bits : 9
    STATUS490:    [0x0, 0x1],          bits : 10
    STATUS491:    [0x0, 0x1],          bits : 11
    STATUS492:    [0x0, 0x1],          bits : 12
    STATUS493:    [0x0, 0x1],          bits : 13
    STATUS494:    [0x0, 0x1],          bits : 14
    STATUS495:    [0x0, 0x1],          bits : 15
    STATUS496:    [0x0, 0x1],          bits : 16
    STATUS497:    [0x0, 0x1],          bits : 17
    STATUS498:    [0x0, 0x1],          bits : 18
    STATUS499:    [0x0, 0x1],          bits : 19   
*/        

#define TRKE_STATUS_REGISTER15_OFS 0x00c8
REGDEF_BEGIN(TRKE_STATUS_REGISTER15)
REGDEF_BIT(STATUS480,        1)
REGDEF_BIT(STATUS481,        1)
REGDEF_BIT(STATUS482,        1)
REGDEF_BIT(STATUS483,        1)
REGDEF_BIT(STATUS484,        1)
REGDEF_BIT(STATUS485,        1)
REGDEF_BIT(STATUS486,        1)
REGDEF_BIT(STATUS487,        1)
REGDEF_BIT(STATUS488,        1)
REGDEF_BIT(STATUS489,        1)
REGDEF_BIT(STATUS490,        1)
REGDEF_BIT(STATUS491,        1)
REGDEF_BIT(STATUS492,        1)
REGDEF_BIT(STATUS493,        1)
REGDEF_BIT(STATUS494,        1)
REGDEF_BIT(STATUS495,        1)
REGDEF_BIT(STATUS496,        1)
REGDEF_BIT(STATUS497,        1)
REGDEF_BIT(STATUS498,        1)
REGDEF_BIT(STATUS499,        1)
REGDEF_END(TRKE_STATUS_REGISTER15)


#define TRKE_RESERVED_REGISTER_OFS 0x0cc
REGDEF_BEGIN(TRKE_RESERVED_REGISTER87)
REGDEF_BIT(TRKE_BLOCK_LOAD_CNT,        32)
REGDEF_END(TRKE_RESERVED_REGISTER87)


#define TRKE_DMA_INPUT_REGISTER0_MSB_OFS 0x0d0
REGDEF_BEGIN(TRKE_DMA_INPUT_REGISTER0_MSB)
REGDEF_BIT(DRAM_IN_SADDR0_MSB,        32)
REGDEF_END(TRKE_DMA_INPUT_REGISTER0_MSB)

#define TRKE_DMA_INPUT_REGISTER1_MSB_OFS 0x0d4
REGDEF_BEGIN(TRKE_DMA_INPUT_REGISTER1_MSB)
REGDEF_BIT(DRAM_IN_SADDR1_MSB,        32)
REGDEF_END(TRKE_DMA_INPUT_REGISTER1_MSB)

#define TRKE_DMA_INPUT_REGISTER2_MSB_OFS 0x0d8
REGDEF_BEGIN(TRKE_DMA_INPUT_REGISTER2_MSB)
REGDEF_BIT(DRAM_IN_SADDR2_MSB,        32)
REGDEF_END(TRKE_DMA_INPUT_REGISTER2_MSB)

#define TRKE_DMA_INPUT_REGISTER3_MSB_OFS 0x0dc
REGDEF_BEGIN(TRKE_DMA_INPUT_REGISTER3_MSB)
REGDEF_BIT(DRAM_IN_SADDR3_MSB,        32)
REGDEF_END(TRKE_DMA_INPUT_REGISTER3_MSB)


#define TRKE_DMA_OUTPUT_REGISTER0_MSB_OFS 0x0e0
REGDEF_BEGIN(TRKE_DMA_OUTPUT_REGISTER0_MSB)
REGDEF_BIT(DRAM_OUTPUT_SADDR0_MSB,        32)
REGDEF_END(TRKE_DMA_OUTPUT_REGISTER0_MSB)

#define TRKE_DMA_OUTPUT_REGISTER1_MSB_OFS 0x0e4
REGDEF_BEGIN(TRKE_DMA_OUTPUT_REGISTER1_MSB)
REGDEF_BIT(DRAM_OUTPUT_SADDR1_MSB,        32)
REGDEF_END(TRKE_DMA_OUTPUT_REGISTER1_MSB)

#define TRKE_LL_ADDRESS_REGISTER_MSB_OFS 0x0e8
REGDEF_BEGIN(TRKE_LL_ADDRESS_REGISTER_MSB)
REGDEF_BIT(DRAM_LL_SAI_MSB,        32)
REGDEF_END(TRKE_LL_ADDRESS_REGISTER_MSB)

#define TRKE_LL_BASE_ADDRESS_REGISTER_MSB_OFS 0x0ec
REGDEF_BEGIN(TRKE_LL_BASE_ADDRESS_REGISTER_MSB)
REGDEF_BIT(LL_BASE_ADDR_MSB,        32)
REGDEF_END(TRKE_LL_BASE_ADDRESS_REGISTER_MSB)




typedef struct
{

  union
  {
    struct
    {
      unsigned TRKE_RST          : 1;        // bits : 0
      unsigned TRKE_START        : 1;        // bits : 1
      unsigned                   : 26;
      unsigned LL_FIRE           : 1;
    } Bit;
    UINT32 Word;
  } TRKE_Register_0; // 0x0000

  union
  {
    struct
    {
      unsigned INTE_FRM_END       : 1;     // bits : 0
      unsigned                    : 7;
      unsigned INTE_LL_END        : 1;     // bits : 8
      unsigned INTE_LL_ERR        : 1;     // bits : 9
      unsigned INTE_LL_JOB_END    : 1;     // bits : 10
    } Bit;
    UINT32 Word;
  } TRKE_Register_1; // 0x0004

  union
  {
    struct
    {
      unsigned INT_FRM_END        : 1;      // bits : 0
      unsigned                    : 7;
      unsigned INT_LL_END         : 1;     // bits : 8
      unsigned INT_LL_ERR         : 1;     // bits : 9
      unsigned INT_LL_JOB_END     : 1;     // bits : 10
    } Bit;
    UINT32 Word;
  } TRKE_Register_2; // 0x0008

  union
  {
    struct
    {
      unsigned LL_TERMINATE        : 1  ;

    } Bit;
    UINT32 Word;
  } TRKE_Register_3; // 0x000c

  union
  {
    struct
    {
      unsigned DRAM_LL_SAI        : 32;

    } Bit;
    UINT32 Word;
  } TRKERegister_4; // 0x0010

  union
  {
    struct
    {
      unsigned POINT_NUM                : 9;        // bits : 0_8
      unsigned                          : 3;        // bits : 9_11
      unsigned MAX_SEARCH_RANGE         : 8;        // bits : 12_19
      unsigned ITER_CNT                 : 5;        // bits : 20_24
      unsigned                          : 3;        // bits : 25_27
      unsigned IS_INITIAL_FLOW          : 1;        // bits : 28
      unsigned IS_SKIP_POINT            : 1;        // bits : 29
      unsigned SEARCH_LIMIT_DISABLE     : 1;              
      unsigned                          : 1;
    } Bit;
    UINT32 Word;
  } TRKE_Register_5; // 0x0014

  union
  {
    struct
    {
      unsigned                       : 2;
      unsigned DRAM_IN_SADDR0        : 30;       // bits : 30_2
    } Bit;
    UINT32 Word;
  } TRKE_Register_6; // 0x0018

  union
  {
    struct
    {
      unsigned                       : 2;
      unsigned DRAM_IN_SADDR1        : 30;       // bits : 30_2
    } Bit;
    UINT32 Word;
  } TRKE_Register_7; // 0x001C


  union
  {
    struct
    {
      unsigned                       : 2;
      unsigned DRAM_IN_SADDR2        : 30;       // bits : 30_2
    } Bit;
    UINT32 Word;
  } TRKE_Register_8; // 0x0020

   union
  {
    struct
    {
      unsigned                       : 2;
      unsigned DRAM_IN_SADDR3        : 30;       // bits : 30_2
    } Bit;
    UINT32 Word;
  } TRKE_Register_9; // 0x0024

  union
  {
    struct
    {
      unsigned                      : 2;
      unsigned DRAM_IN_LOFST0       : 18;       // bits : 19_2
    } Bit;
    UINT32 Word;
  } TRKE_Register_10; // 0x0028

  union
  {
    struct
    {
      unsigned                       : 2;
      unsigned DRAM_IN_LOFST1        : 18;      // bits : 19_2
    } Bit;
    UINT32 Word;
  } TRKE_Register_11; // 0x002c

  union
  {
    struct
    {
      unsigned                       : 2;
      unsigned DRAM_OUT_SADDR0       : 30;      // bits : 30_2
    } Bit;
    UINT32 Word;
  } TRKE_Register_12; // 0x0030

   union
  {
    struct
    {
      unsigned                       : 2;
      unsigned DRAM_OUT_SADDR1       : 30;      // bits : 30_2
    } Bit;
    UINT32 Word;
  } TRKE_Register_13; // 0x0034

  union
  {
    struct
    {
      unsigned IM_PYR_WIDTH         : 12;      // bits : 13_0
      unsigned                      : 4;
      unsigned IM_PYR_HEIGHT        : 12;      // bits : 27_16
    } Bit;
    UINT32 Word;
  } TRKE_Register_14; // 0x0038

  union
  {
    struct
    {
      unsigned EPS                   : 8;      // bits : 7_0
      unsigned TRKE_PATCHSIZE        : 2;      // bits : 9_8
      unsigned                       : 6;      // bits : 15_10
      unsigned MIN_EIG_VAL_THRES     : 8;      // bits : 23_16
      unsigned PYR_INDEX             : 3;      // bits : 26_24
      unsigned                       : 1;      // bits : 27_27
      unsigned MAX_PYR_LEVEL         : 3;      // bits : 30_28
      unsigned                       : 1;      // bits : 31_31  
    } Bit;
    UINT32 Word;
  } TRKE_Register_15; // 0x003c

  union
  {
    struct
    {
      unsigned PYR_SEARCH_RANGE       :16;
      unsigned LAYER_SEARCH_RANGE     :16;      
    } Bit;
    UINT32 Word;
  } TRKE_Register_16; // 0x0040

  union
  {
    struct
    {
      unsigned                      : 32;      // bits : 31_0
    } Bit;
    UINT32 Word;
  } TRKE_Register_17; // 0x0044

  union
  {
    struct
    {
      unsigned                      : 32;      // bits : 31_0
    } Bit;
    UINT32 Word;
  } TRKE_Register_18; // 0x0048

  union
  {
    struct
    {
            unsigned in0_ch_en         : 1;      // bits : 0
			unsigned in1_ch_en         : 1;      // bits : 1
			unsigned in2_ch_en         : 1;      // bits : 2
			unsigned in3_ch_en         : 1;      // bits : 3
			unsigned                   : 1;      // bits : 4
			unsigned out0_ch_en        : 1;      // bits : 5
            unsigned out1_ch_en        : 1;      // bits : 6
            unsigned                   : 1;      // bits : 7
			unsigned llc_ch_en         : 1;      // bits : 8
			unsigned in0_lock_dis      : 1;      // bits : 9
			unsigned in1_lock_dis      : 1;      // bits : 10
			unsigned in2_lock_dis      : 1;      // bits : 11
			unsigned in3_lock_dis      : 1;      // bits : 12
            unsigned                   : 1;      // bits : 13
			unsigned                   : 2;      // bits : 15_14
			unsigned write_ch_outstand_num : 8;      // bits : 23_16
			unsigned read_ch_outstand_num : 8;      // bits : 31_24
    } Bit;
    UINT32 Word;
  } TRKE_Register_19; // 0x004c

  union
  {
    struct
    {
        unsigned out0_lock_dis   : 1;      // bits : 0
		unsigned out1_lock_dis   : 1;      // bits : 1
		unsigned llc_lock_dis    : 1;      // bits : 2
		unsigned                 : 29;     // bits : 31_3
    } Bit;
    UINT32 Word;
  } TRKE_Register_20; // 0x0050

    union
  {
    struct
    {
        unsigned axi_bus_disable   : 1;      // bits : 0
		unsigned                   : 26;     // bits : 26_1
		unsigned axi_bus_idle      : 1;      // bits : 27
		unsigned                   : 4;      // bits : 31_28
    } Bit;
    UINT32 Word;
  } TRKE_Register_21; // 0x0054

    union
  {
    struct
    {
      unsigned                      : 32;      // bits : 31_0
    } Bit;
    UINT32 Word;
  } TRKE_Register_22; // 0x0058

    union
  {
    struct
    {
      unsigned                      : 32;      // bits : 31_0
    } Bit;
    UINT32 Word;
  } TRKE_Register_23; // 0x005C


      union
  {
    struct
    {
      unsigned INDATA_BURST_MODE        : 2;      // bits : 1_0
      unsigned OUTRST_BURST_MODE        : 2;      // bits : 3_2
      unsigned                          : 28; 

    } Bit;
    UINT32 Word;
  } TRKE_Register_24; // 0x0060


   union
  {
    struct
    {
      unsigned                      : 32;      // bits : 31_0
    } Bit;
    UINT32 Word;
  } TRKE_Register_25; // 0x0064



    union
  {
    struct
    {
      unsigned                      : 32;      // bits : 31_0
    } Bit;
    UINT32 Word;
  } TRKE_Register_26; // 0x0068

    union
  {
    struct
    {
      unsigned                      : 32;      // bits : 31_0
    } Bit;
    UINT32 Word;
  } TRKE_Register_27; // 0x006C

    union
  {
    struct
    {
      unsigned                      : 32;      // bits : 31_0
    } Bit;
    UINT32 Word;
  } TRKE_Register_28; // 0x0070

    union
  {
    struct
    {
      unsigned                      : 32;      // bits : 31_0
    } Bit;
    UINT32 Word;
  } TRKE_Register_29; // 0x0074


      union
  {
    struct
    {
      unsigned                      : 32;      // bits : 31_0
    } Bit;
    UINT32 Word;
  } TRKE_Register_30; // 0x0078

      union
  {
    struct
    {
      unsigned                      : 32;      // bits : 31_0
    } Bit;
    UINT32 Word;
  } TRKE_Register_31; // 0x007c

      union
  {
    struct
    {
      unsigned                      : 32;      // bits : 31_0
    } Bit;
    UINT32 Word;
  } TRKE_Register_32; // 0x0080

      union
  {
    struct
    {
      unsigned                      : 32;      // bits : 31_0
    } Bit;
    UINT32 Word;
  } TRKE_Register_33; // 0x0084



   union
  {
    struct
    {
      unsigned                      : 32;      // bits : 31_0
    } Bit;
    UINT32 Word;
  } TRKE_Register_34; // 0x0088

  union
  {
    struct
    {
      unsigned Status0        : 1;
      unsigned Status1        : 1;
      unsigned Status2        : 1;
      unsigned Status3        : 1;
      unsigned Status4        : 1;
      unsigned Status5        : 1;
      unsigned Status6        : 1;
      unsigned Status7        : 1;
      unsigned Status8        : 1;
      unsigned Status9        : 1;
      unsigned Status10       : 1;
      unsigned Status11       : 1;
      unsigned Status12        : 1;
      unsigned Status13        : 1;
      unsigned Status14        : 1;
      unsigned Status15        : 1;
      unsigned Status16        : 1;
      unsigned Status17        : 1;
      unsigned Status18        : 1;
      unsigned Status19        : 1;
      unsigned Status20        : 1;
      unsigned Status21        : 1;
      unsigned Status22        : 1;
      unsigned Status23        : 1;
      unsigned Status24        : 1;
      unsigned Status25        : 1;
      unsigned Status26        : 1;
      unsigned Status27        : 1;
      unsigned Status28        : 1;
      unsigned Status29        : 1;
      unsigned Status30        : 1;
      unsigned Status31        : 1;

    } Bit;
    UINT32 Word;
  } TRKE_Register_35; // 0x008c


    union
  {
    struct
    {
      unsigned Status32        : 1;
      unsigned Status33        : 1;
      unsigned Status34        : 1;
      unsigned Status35        : 1;
      unsigned Status36        : 1;
      unsigned Status37        : 1;
      unsigned Status38        : 1;
      unsigned Status39        : 1;
      unsigned Status40        : 1;
      unsigned Status41        : 1;
      unsigned Status42        : 1;
      unsigned Status43        : 1;
      unsigned Status44        : 1;
      unsigned Status45        : 1;
      unsigned Status46        : 1;
      unsigned Status47        : 1;
      unsigned Status48        : 1;
      unsigned Status49        : 1;
      unsigned Status50        : 1;
      unsigned Status51        : 1;
      unsigned Status52        : 1;
      unsigned Status53        : 1;
      unsigned Status54        : 1;
      unsigned Status55        : 1;
      unsigned Status56        : 1;
      unsigned Status57        : 1;
      unsigned Status58        : 1;
      unsigned Status59        : 1;
      unsigned Status60        : 1;
      unsigned Status61        : 1;
      unsigned Status62       : 1;
      unsigned Status63       : 1;

    } Bit;
    UINT32 Word;
  } TRKE_Register_36; // 0x0090

   union
  {
    struct
    {
      unsigned Status64        : 1;
      unsigned Status65        : 1;
      unsigned Status66        : 1;
      unsigned Status67        : 1;
      unsigned Status68        : 1;
      unsigned Status69        : 1;
      unsigned Status70        : 1;
      unsigned Status71        : 1;
      unsigned Status72        : 1;
      unsigned Status73        : 1;
      unsigned Status74        : 1;
      unsigned Status75        : 1;
      unsigned Status76        : 1;
      unsigned Status77        : 1;
      unsigned Status78        : 1;
      unsigned Status79        : 1;
      unsigned Status80        : 1;
      unsigned Status81        : 1;
      unsigned Status82        : 1;
      unsigned Status83        : 1;
      unsigned Status84        : 1;
      unsigned Status85        : 1;
      unsigned Status86        : 1;
      unsigned Status87        : 1;
      unsigned Status88        : 1;
      unsigned Status89        : 1;
      unsigned Status90        : 1;
      unsigned Status91        : 1;
      unsigned Status92        : 1;
      unsigned Status93        : 1;
	  unsigned Status94        : 1;
      unsigned Status95        : 1;

    } Bit;
    UINT32 Word;
  } TRKE_Register_37; // 0x0094


  union
  {
    struct
    {
      unsigned Status96        : 1;
      unsigned Status97        : 1;
      unsigned Status98        : 1;
      unsigned Status99        : 1;
      unsigned Status100        : 1;
      unsigned Status101        : 1;
      unsigned Status102        : 1;
      unsigned Status103        : 1;
      unsigned Status104        : 1;
      unsigned Status105        : 1;
      unsigned Status106        : 1;
      unsigned Status107        : 1;
      unsigned Status108        : 1;
      unsigned Status109        : 1;
      unsigned Status110        : 1;
      unsigned Status111        : 1;
      unsigned Status112        : 1;
      unsigned Status113        : 1;
      unsigned Status114        : 1;
      unsigned Status115        : 1;
      unsigned Status116        : 1;
      unsigned Status117        : 1;
      unsigned Status118        : 1;
      unsigned Status119        : 1;
      unsigned Status120        : 1;
      unsigned Status121        : 1;
      unsigned Status122        : 1;
      unsigned Status123        : 1;
	  unsigned Status124        : 1;
      unsigned Status125        : 1;
	  unsigned Status126        : 1;
      unsigned Status127        : 1;
    } Bit;
    UINT32 Word;
  } TRKE_Register_38; // 0x0098

  union
  {
    struct
    {
      unsigned Status128        : 1;
      unsigned Status129        : 1;
      unsigned Status130        : 1;
      unsigned Status131        : 1;
      unsigned Status132        : 1;
      unsigned Status133        : 1;
      unsigned Status134        : 1;
      unsigned Status135        : 1;
      unsigned Status136        : 1;
      unsigned Status137        : 1;
      unsigned Status138        : 1;
      unsigned Status139        : 1;
      unsigned Status140        : 1;
      unsigned Status141        : 1;
      unsigned Status142        : 1;
      unsigned Status143        : 1;
      unsigned Status144        : 1;
      unsigned Status145        : 1;
      unsigned Status146        : 1;
      unsigned Status147        : 1;
      unsigned Status148        : 1;
      unsigned Status149        : 1;
      unsigned Status150        : 1;
      unsigned Status151        : 1;
      unsigned Status152        : 1;
      unsigned Status153        : 1;
	  unsigned Status154        : 1;
      unsigned Status155        : 1;
	  unsigned Status156        : 1;
      unsigned Status157        : 1;
	  unsigned Status158        : 1;
      unsigned Status159        : 1;
    } Bit;
    UINT32 Word;
  } TRKE_Register_39; // 0x009c


  union
  {
    struct
    {
      unsigned Status160        : 1;
      unsigned Status161        : 1;
      unsigned Status162        : 1;
      unsigned Status163        : 1;
      unsigned Status164        : 1;
      unsigned Status165        : 1;
      unsigned Status166        : 1;
      unsigned Status167        : 1;
      unsigned Status168        : 1;
      unsigned Status169        : 1;
      unsigned Status170        : 1;
      unsigned Status171        : 1;
      unsigned Status172        : 1;
      unsigned Status173        : 1;
      unsigned Status174        : 1;
      unsigned Status175        : 1;
      unsigned Status176        : 1;
      unsigned Status177        : 1;
      unsigned Status178        : 1;
      unsigned Status179        : 1;
      unsigned Status180        : 1;
      unsigned Status181        : 1;
      unsigned Status182        : 1;
      unsigned Status183        : 1;
	  unsigned Status184        : 1;
      unsigned Status185        : 1;
	  unsigned Status186        : 1;
      unsigned Status187        : 1;
	  unsigned Status188        : 1;
      unsigned Status189        : 1;
      unsigned Status190        : 1;
      unsigned Status191        : 1;
    } Bit;
    UINT32 Word;
  } TRKE_Register_40; // 0x00a0


  union
  {
    struct
    {
      unsigned Status192        : 1;
      unsigned Status193        : 1;
      unsigned Status194        : 1;
      unsigned Status195        : 1;
      unsigned Status196        : 1;
      unsigned Status197        : 1;
      unsigned Status198        : 1;
      unsigned Status199        : 1;
      unsigned Status200        : 1;
      unsigned Status201        : 1;
      unsigned Status202        : 1;
      unsigned Status203        : 1;
      unsigned Status204        : 1;
      unsigned Status205        : 1;
      unsigned Status206        : 1;
      unsigned Status207        : 1;
      unsigned Status208        : 1;
      unsigned Status209        : 1;
      unsigned Status210        : 1;
      unsigned Status211        : 1;
      unsigned Status212        : 1;
      unsigned Status213        : 1;
	  unsigned Status214        : 1;
      unsigned Status215        : 1;
	  unsigned Status216        : 1;
      unsigned Status217        : 1;
	  unsigned Status218        : 1;
      unsigned Status219        : 1;
      unsigned Status220        : 1;
      unsigned Status221        : 1;
	  unsigned Status222        : 1;
      unsigned Status223        : 1;

    } Bit;
    UINT32 Word;
  } TRKE_Register_41; // 0x00a4


  union
  {
    struct
    {
      unsigned Status224        : 1;
      unsigned Status225        : 1;
      unsigned Status226        : 1;
      unsigned Status227        : 1;
      unsigned Status228        : 1;
      unsigned Status229        : 1;
      unsigned Status230        : 1;
      unsigned Status231        : 1;
      unsigned Status232        : 1;
      unsigned Status233        : 1;
      unsigned Status234        : 1;
      unsigned Status235        : 1;
      unsigned Status236        : 1;
      unsigned Status237        : 1;
      unsigned Status238        : 1;
      unsigned Status239        : 1;
      unsigned Status240        : 1;
      unsigned Status241        : 1;
      unsigned Status242        : 1;
      unsigned Status243        : 1;
	  unsigned Status244        : 1;
      unsigned Status245        : 1;
	  unsigned Status246        : 1;
      unsigned Status247        : 1;
	  unsigned Status248        : 1;
      unsigned Status249        : 1;
      unsigned Status250        : 1;
      unsigned Status251        : 1;
	  unsigned Status252        : 1;
      unsigned Status253        : 1;
	  unsigned Status254        : 1;
      unsigned Status255        : 1;

    } Bit;
    UINT32 Word;
  } TRKE_Register_42; // 0x00a8

  union
  {
    struct
    {
      unsigned Status256        : 1;
      unsigned Status257        : 1;
      unsigned Status258        : 1;
      unsigned Status259        : 1;
      unsigned Status260        : 1;
      unsigned Status261        : 1;
      unsigned Status262        : 1;
      unsigned Status263        : 1;
      unsigned Status264        : 1;
      unsigned Status265        : 1;
      unsigned Status266        : 1;
      unsigned Status267        : 1;
      unsigned Status268        : 1;
      unsigned Status269        : 1;
      unsigned Status270        : 1;
      unsigned Status271        : 1;
      unsigned Status272        : 1;
      unsigned Status273        : 1;
	  unsigned Status274        : 1;
      unsigned Status275        : 1;
	  unsigned Status276        : 1;
      unsigned Status277        : 1;
	  unsigned Status278        : 1;
      unsigned Status279        : 1;
      unsigned Status280        : 1;
      unsigned Status281        : 1;
	  unsigned Status282        : 1;
      unsigned Status283        : 1;
	  unsigned Status284        : 1;
      unsigned Status285        : 1;
	  unsigned Status286        : 1;
      unsigned Status287        : 1;

    } Bit;
    UINT32 Word;
  } TRKE_Register_43; // 0x00ac

  union
  {
    struct
    {
      unsigned Status288        : 1;
      unsigned Status289        : 1;
      unsigned Status290        : 1;
      unsigned Status291        : 1;
      unsigned Status292        : 1;
      unsigned Status293        : 1;
      unsigned Status294        : 1;
      unsigned Status295        : 1;
      unsigned Status296        : 1;
      unsigned Status297        : 1;
      unsigned Status298        : 1;
      unsigned Status299        : 1;
      unsigned Status300        : 1;
      unsigned Status301        : 1;
      unsigned Status302        : 1;
      unsigned Status303        : 1;
	  unsigned Status304        : 1;
      unsigned Status305        : 1;
	  unsigned Status306        : 1;
      unsigned Status307        : 1;
	  unsigned Status308        : 1;
      unsigned Status309        : 1;
      unsigned Status310        : 1;
      unsigned Status311        : 1;
	  unsigned Status312        : 1;
      unsigned Status313        : 1;
	  unsigned Status314        : 1;
      unsigned Status315        : 1;
	  unsigned Status316        : 1;
      unsigned Status317        : 1;
	  unsigned Status318        : 1;
      unsigned Status319        : 1;

    } Bit;
    UINT32 Word;
  } TRKE_Register_44; // 0x00b0


  union
  {
    struct
    {
      unsigned Status320        : 1;
      unsigned Status321        : 1;
      unsigned Status322        : 1;
      unsigned Status323        : 1;
      unsigned Status324        : 1;
      unsigned Status325        : 1;
      unsigned Status326        : 1;
      unsigned Status327        : 1;
      unsigned Status328        : 1;
      unsigned Status329        : 1;
      unsigned Status330        : 1;
      unsigned Status331        : 1;
      unsigned Status332        : 1;
      unsigned Status333        : 1;
	  unsigned Status334        : 1;
      unsigned Status335        : 1;
	  unsigned Status336        : 1;
      unsigned Status337        : 1;
	  unsigned Status338        : 1;
      unsigned Status339        : 1;
      unsigned Status340        : 1;
      unsigned Status341        : 1;
	  unsigned Status342        : 1;
      unsigned Status343        : 1;
	  unsigned Status344        : 1;
      unsigned Status345        : 1;
	  unsigned Status346        : 1;
      unsigned Status347        : 1;
	  unsigned Status348        : 1;
      unsigned Status349        : 1;
	  unsigned Status350        : 1;
      unsigned Status351        : 1;

    } Bit;
    UINT32 Word;
  } TRKE_Register_45; // 0x00b4


  union
  {
    struct
    {
      unsigned Status352        : 1;
      unsigned Status353        : 1;
      unsigned Status354        : 1;
      unsigned Status355        : 1;
      unsigned Status356        : 1;
      unsigned Status357        : 1;
      unsigned Status358        : 1;
      unsigned Status359        : 1;
      unsigned Status360        : 1;
      unsigned Status361        : 1;
      unsigned Status362        : 1;
      unsigned Status363        : 1;
	  unsigned Status364        : 1;
      unsigned Status365        : 1;
	  unsigned Status366        : 1;
      unsigned Status367        : 1;
	  unsigned Status368        : 1;
      unsigned Status369        : 1;
      unsigned Status370        : 1;
      unsigned Status371        : 1;
	  unsigned Status372        : 1;
      unsigned Status373        : 1;
	  unsigned Status374        : 1;
      unsigned Status375        : 1;
	  unsigned Status376        : 1;
      unsigned Status377        : 1;
	  unsigned Status378        : 1;
      unsigned Status379        : 1;
	  unsigned Status380        : 1;
      unsigned Status381        : 1;
	  unsigned Status382        : 1;
      unsigned Status383        : 1;

    } Bit;
    UINT32 Word;
  } TRKE_Register_46; // 0x00b8


  union
  {
    struct
    {
      unsigned Status384        : 1;
      unsigned Status385        : 1;
      unsigned Status386        : 1;
      unsigned Status387        : 1;
      unsigned Status388        : 1;
      unsigned Status389        : 1;
      unsigned Status390        : 1;
      unsigned Status391        : 1;
      unsigned Status392        : 1;
      unsigned Status393        : 1;
	  unsigned Status394        : 1;
      unsigned Status395        : 1;
	  unsigned Status396        : 1;
      unsigned Status397        : 1;
	  unsigned Status398        : 1;
      unsigned Status399        : 1;
      unsigned Status400        : 1;
      unsigned Status401        : 1;
	  unsigned Status402        : 1;
      unsigned Status403        : 1;
	  unsigned Status404        : 1;
      unsigned Status405        : 1;
	  unsigned Status406        : 1;
      unsigned Status407        : 1;
	  unsigned Status408        : 1;
      unsigned Status409        : 1;
	  unsigned Status410        : 1;
      unsigned Status411        : 1;
	  unsigned Status412        : 1;
      unsigned Status413        : 1;
	  unsigned Status414        : 1;
      unsigned Status415        : 1;

    } Bit;
    UINT32 Word;
  } TRKE_Register_47; // 0x00bc


  union
  {
    struct
    {
	  unsigned Status416        : 1;
      unsigned Status417        : 1;
      unsigned Status418        : 1;
      unsigned Status419        : 1;
      unsigned Status420        : 1;
      unsigned Status421        : 1;
      unsigned Status422        : 1;
      unsigned Status423        : 1;
	  unsigned Status424        : 1;
      unsigned Status425        : 1;
	  unsigned Status426        : 1;
      unsigned Status427        : 1;
	  unsigned Status428        : 1;
      unsigned Status429        : 1;
      unsigned Status430        : 1;
      unsigned Status431        : 1;
	  unsigned Status432        : 1;
      unsigned Status433        : 1;
	  unsigned Status434        : 1;
      unsigned Status435        : 1;
	  unsigned Status436        : 1;
      unsigned Status437        : 1;
	  unsigned Status438        : 1;
      unsigned Status439        : 1;
	  unsigned Status440        : 1;
      unsigned Status441        : 1;
	  unsigned Status442        : 1;
      unsigned Status443        : 1;
	  unsigned Status444        : 1;
      unsigned Status445        : 1;
	  unsigned Status446        : 1;
      unsigned Status447        : 1;

    } Bit;
    UINT32 Word;
  } TRKE_Register_48; // 0x00c0

  union
  {
    struct
    {
	  unsigned Status448        : 1;
      unsigned Status449        : 1;
      unsigned Status450        : 1;
      unsigned Status451        : 1;
      unsigned Status452        : 1;
      unsigned Status453        : 1;
	  unsigned Status454        : 1;
      unsigned Status455        : 1;
	  unsigned Status456        : 1;
      unsigned Status457        : 1;
	  unsigned Status458        : 1;
      unsigned Status459        : 1;
      unsigned Status460        : 1;
      unsigned Status461        : 1;
	  unsigned Status462        : 1;
      unsigned Status463        : 1;
	  unsigned Status464        : 1;
      unsigned Status465        : 1;
	  unsigned Status466        : 1;
      unsigned Status467        : 1;
	  unsigned Status468        : 1;
      unsigned Status469        : 1;
	  unsigned Status470        : 1;
      unsigned Status471        : 1;
	  unsigned Status472        : 1;
      unsigned Status473        : 1;
	  unsigned Status474        : 1;
      unsigned Status475        : 1;
	  unsigned Status476        : 1;
      unsigned Status477        : 1;
	  unsigned Status478        : 1;
      unsigned Status479        : 1;

    } Bit;
    UINT32 Word;
  } TRKE_Register_49; // 0x00c4


    union
  {
    struct
    {
	  unsigned Status480        : 1;
      unsigned Status481        : 1;
      unsigned Status482        : 1;
      unsigned Status483        : 1;
	  unsigned Status484        : 1;
      unsigned Status485        : 1;
	  unsigned Status486        : 1;
      unsigned Status487        : 1;
	  unsigned Status488        : 1;
      unsigned Status489        : 1;
      unsigned Status490        : 1;
      unsigned Status491        : 1;
	  unsigned Status492        : 1;
      unsigned Status493        : 1;
	  unsigned Status494        : 1;
      unsigned Status495        : 1;
	  unsigned Status496        : 1;
      unsigned Status497        : 1;
	  unsigned Status498        : 1;
      unsigned Status499        : 1;
      unsigned                  : 12;

    } Bit;
    UINT32 Word;
  } TRKE_Register_50; // 0x00c8

  union
  {
    struct
    {
      unsigned TRKE_BLOCK_LOAD_CNT        : 32;
    } Bit;
    UINT32 Word;
  } TRKE_Register_51; // 0x00cc

   union
  {
    struct
    {
      unsigned DRAM_IN_SADDR0_MSB        : 32;
    } Bit;
    UINT32 Word;
  } TRKE_Register_52; // 0x00d0

  union
  {
    struct
    {
      unsigned DRAM_IN_SADDR1_MSB        : 32;
    } Bit;
    UINT32 Word;
  } TRKE_Register_53; // 0x00d4

  union
  {
    struct
    {
      unsigned DRAM_IN_SADDR2_MSB        : 32;
    } Bit;
    UINT32 Word;
  } TRKE_Register_54; // 0x00d8


  union
  {
    struct
    {
      unsigned DRAM_IN_SADDR3_MSB        : 32;
    } Bit;
    UINT32 Word;
  } TRKE_Register_55; // 0x00dc

  union
  {
    struct
    {
      unsigned DRAM_OUT_SADDR0_MSB        : 32;
    } Bit;
    UINT32 Word;
  } TRKE_Register_56; // 0x00e0

  union
  {
    struct
    {
      unsigned DRAM_OUT_SADDR1_MSB        : 32;
    } Bit;
    UINT32 Word;
  } TRKE_Register_57; // 0x00e4

  union
  {
    struct
    {
      unsigned DRAM_LL_SAI_MSB        : 32;
    } Bit;
    UINT32 Word;
  } TRKE_Register_58; // 0x00e8

  union
  {
    struct
    {
      unsigned LL_BASE_ADDR_MSB        : 32;
    } Bit;
    UINT32 Word;
  } TRKE_Register_59; // 0x00ec



} NT98538_TRKE_REGISTER_STRUCT;



#ifdef __cplusplus
}
#endif


#endif
