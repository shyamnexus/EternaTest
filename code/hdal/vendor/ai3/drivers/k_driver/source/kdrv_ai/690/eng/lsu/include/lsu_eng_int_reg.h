#ifndef _LSU_ENG_INT_REGISTER_H_
#define _LSU_ENG_INT_REGISTER_H_

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
#include "kwrap/type.h"
#include "kwrap/nvt_type.h"
#else
#endif

#define REGVALUE        uint32_t
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
    LSU_SW_RST   :    [0x0, 0x1],			bits : 0
    LSU_START    :    [0x0, 0x1],			bits : 1
    JOB_START    :    [0x0, 0x1],			bits : 28
    JOB_TERMINATE:    [0x0, 0x1],			bits : 29
*/

#define LSU_CONTROL_REGISTER_OFS 0x0000
REGDEF_BEGIN(LSU_CONTROL_REGISTER_OFS)
    REGDEF_BIT(LSU_SW_RST   ,        1)
    REGDEF_BIT(LSU_START    ,        1)
    REGDEF_BIT(             ,        26)
    REGDEF_BIT(JOB_START    ,        1)
    REGDEF_BIT(JOB_TERMINATE,        1)
    REGDEF_BIT(             ,        2)
REGDEF_END(LSU_CONTROL_REGISTER_OFS)


/*
    INTE_FRM_DONE   :    [0x0, 0x1],		bits : 0
    INTE_ACD_BS_LEN_ERR :    [0x0, 0x1],	bits : 4
    INTE_JOB_END   :    [0x0, 0x1],			bits : 24
    INTE_JOB_ERR   :    [0x0, 0x1],			bits : 25
    INTE_JOB_WR_END:    [0x0, 0x1],			bits : 26
*/

#define LSU_INTERRUPT_ENABLE_REGISTER_OFS 0x0004
REGDEF_BEGIN(LSU_INTERRUPT_ENABLE_REGISTER_OFS)
    REGDEF_BIT(INTE_FRM_DONE   ,       1)
    REGDEF_BIT(               ,        3)
    REGDEF_BIT(INTE_ACD_BS_LEN_ERR ,   1)
    REGDEF_BIT(               ,        19)
    REGDEF_BIT(INTE_JOB_END   ,        1)
    REGDEF_BIT(INTE_JOB_ERR   ,        1)
    REGDEF_BIT(INTE_JOB_WR_END,        1)
REGDEF_END(LSU_INTERRUPT_ENABLE_REGISTER_OFS)


/*
    INTS_FRM_DONE  :    [0x0, 0x1],			bits : 0
    INTS_ACD_BS_LEN_ERR :    [0x0, 0x1],	bits : 4
    INTS_JOB_END   :    [0x0, 0x1],			bits : 24
    INTS_JOB_ERR   :    [0x0, 0x1],			bits : 25
    INTS_JOB_WR_END:    [0x0, 0x1],			bits : 26
*/

#define LSU_INTERRUPT_STATUS_REGISTER_OFS 0x0008
REGDEF_BEGIN(LSU_INTERRUPT_STATUS_REGISTER_OFS)
    REGDEF_BIT(INTS_FRM_DONE   ,       1)
    REGDEF_BIT(               ,        3)
	REGDEF_BIT(INTS_ACD_BS_LEN_ERR   , 1)
    REGDEF_BIT(               ,        19)
    REGDEF_BIT(INTS_JOB_END   ,        1)
    REGDEF_BIT(INTS_JOB_ERR   ,        1)
    REGDEF_BIT(INTS_JOB_WR_END,        1)
REGDEF_END(LSU_INTERRUPT_STATUS_REGISTER_OFS)


/*
    LSU_MODE         :    [0x0, 0x1],                     bits : 0
*/

#define LSU_MAIN_MODE_CONTROL_REGISTER_OFS 0x000c
REGDEF_BEGIN(LSU_MAIN_MODE_CONTROL_REGISTER_OFS)
    REGDEF_BIT(LSU_MODE,              1)
REGDEF_END(LSU_MAIN_MODE_CONTROL_REGISTER_OFS)


#define LSU_SUB_MODE_REGISTER0_OFS 0x0010
REGDEF_BEGIN(LSU_SUB_MODE_REGISTER0_OFS)
    REGDEF_BIT(AC_ENC_KLOSSY_EN,       1)     // bits :0
    REGDEF_BIT(		      ,         3)
    REGDEF_BIT(AC_BC_OFS_EN,           1)     // bits :4
    REGDEF_BIT(		      ,         3)
    REGDEF_BIT(AC_BC_MODE,             1)     // bits :8
REGDEF_END(LSU_SUB_MODE_REGISTER0_OFS)


#define LSU_FORMAT_REGISTER0_OFS 0x0014
REGDEF_BEGIN(LSU_FORMAT_REGISTER0_OFS)
    REGDEF_BIT(IN_BIT_DEPTH ,           3)    // bits :2_0
    REGDEF_BIT(                       , 1)    
    REGDEF_BIT(IN_DATA_FMT ,            2)    // bits :5_4
REGDEF_END(LSU_FORMAT_REGISTER0_OFS)


#define LSU_RESERVED_REGISTER_0_OFS 0x0018
REGDEF_BEGIN(LSU_RESERVED_REGISTER_0_OFS)
	REGDEF_BIT(    	, 32)	// bits , 31_0
REGDEF_END(LSU_RESERVED_REGISTER_0_OFS)


#define LSU_RESERVED_REGISTER_1_OFS 0x001c
REGDEF_BEGIN(LSU_RESERVED_REGISTER_1_OFS)
	REGDEF_BIT(    	, 32)	// bits , 31_0
REGDEF_END(LSU_RESERVED_REGISTER_1_OFS)


#define LSU_LL_BASE_ADDR_REGISTER_0_OFS 0x0020
REGDEF_BEGIN(LSU_LL_BASE_ADDR_REGISTER_0_OFS)
	REGDEF_BIT(BASE_ADDR0             , 32)    // bits :31_0
REGDEF_END(LSU_LL_BASE_ADDR_REGISTER_0_OFS)


#define LSU_LL_BASE_ADDR_REGISTER_1_OFS 0x0024
REGDEF_BEGIN(LSU_LL_BASE_ADDR_REGISTER_1_OFS)
	REGDEF_BIT(BASE_MSB_ADDR0	       , 4)    // bits :3_0	
REGDEF_END(LSU_LL_BASE_ADDR_REGISTER_1_OFS)


#define LSU_LL_BASE_ADDR_REGISTER_2_OFS 0x0028
REGDEF_BEGIN(LSU_LL_BASE_ADDR_REGISTER_2_OFS)
	REGDEF_BIT(BASE_ADDR1             , 32)    // bits :31_0
REGDEF_END(LSU_LL_BASE_ADDR_REGISTER_2_OFS)


#define LSU_LL_BASE_ADDR_REGISTER_3_OFS 0x002c
REGDEF_BEGIN(LSU_LL_BASE_ADDR_REGISTER_3_OFS)
	REGDEF_BIT(BASE_MSB_ADDR1	       , 4)    // bits :3_0	
REGDEF_END(LSU_LL_BASE_ADDR_REGISTER_3_OFS)


#define LSU_LL_BASE_ADDR_REGISTER_4_OFS 0x0030
REGDEF_BEGIN(LSU_LL_BASE_ADDR_REGISTER_4_OFS)
	REGDEF_BIT(BASE_ADDR2             , 32)    // bits :31_0
REGDEF_END(LSU_LL_BASE_ADDR_REGISTER_4_OFS)


#define LSU_LL_BASE_ADDR_REGISTER_5_OFS 0x0034
REGDEF_BEGIN(LSU_LL_BASE_ADDR_REGISTER_5_OFS)
	REGDEF_BIT(BASE_MSB_ADDR2	       , 4)    // bits :3_0	
REGDEF_END(LSU_LL_BASE_ADDR_REGISTER_5_OFS)


#define LSU_LL_BASE_ADDR_REGISTER_6_OFS 0x0038
REGDEF_BEGIN(LSU_LL_BASE_ADDR_REGISTER_6_OFS)
	REGDEF_BIT(BASE_ADDR3             , 32)    // bits :31_0
REGDEF_END(LSU_LL_BASE_ADDR_REGISTER_6_OFS)


#define LSU_LL_BASE_ADDR_REGISTER_7_OFS 0x003c
REGDEF_BEGIN(LSU_LL_BASE_ADDR_REGISTER_7_OFS)
	REGDEF_BIT(BASE_MSB_ADDR3	       , 4)    // bits :3_0	
REGDEF_END(LSU_LL_BASE_ADDR_REGISTER_7_OFS)


#define LSU_LL_JOB_CHANNEL_REGISTER_0_OFS 0x0040
REGDEF_BEGIN(LSU_LL_JOB_CHANNEL_REGISTER_0_OFS)
	REGDEF_BIT(DRAMUB_SAIJOB	, 32)	       // bits :31_0
REGDEF_END(LSU_LL_JOB_CHANNEL_REGISTER_0_OFS)

	
#define LSU_LL_JOB_CHANNEL_REGISTER_1_OFS 0x0044
REGDEF_BEGIN(LSU_LL_JOB_CHANNEL_REGISTER_1_OFS)
	REGDEF_BIT(DRAMUB_MSB_SAIJOB	, 4)	   // bits :3_0
REGDEF_END(LSU_LL_JOB_CHANNEL_REGISTER_1_OFS)

	
#define LSU_LL_BASE_ADDR_REGISTER_8_OFS 0x0048
REGDEF_BEGIN(LSU_LL_BASE_ADDR_REGISTER_8_OFS)
	REGDEF_BIT(BASE_ADDR4	, 32)	           // bits :31_0
REGDEF_END(LSU_LL_BASE_ADDR_REGISTER_8_OFS)

	
#define LSU_LL_BASE_ADDR_REGISTER_9_OFS 0x004c
REGDEF_BEGIN(LSU_LL_BASE_ADDR_REGISTER_9_OFS)
	REGDEF_BIT(BASE_MSB_ADDR4	, 4)	       // bits :3_0
REGDEF_END(LSU_LL_BASE_ADDR_REGISTER_9_OFS)


#define LSU_DRAMUB_IO_REGISTER_0_OFS 0x0050
REGDEF_BEGIN(LSU_DRAMUB_IO_REGISTER_0_OFS)
	REGDEF_BIT(DRAMUB_SAI0	, 32)	           // bits :31_0
REGDEF_END(LSU_DRAMUB_IO_REGISTER_0_OFS)

	
#define LSU_DRAMUB_IO_REGISTER_1_OFS 0x0054
REGDEF_BEGIN(LSU_DRAMUB_IO_REGISTER_1_OFS)
	REGDEF_BIT(DRAMUB_MSB_SAI0	, 4)	// bits :3_0
	REGDEF_BIT(					, 24)	// bits :27_4
	REGDEF_BIT(DRAMUB_MODE_SAI0	, 4)	// bits :31_28
REGDEF_END(LSU_DRAMUB_IO_REGISTER_1_OFS)

	
#define LSU_DRAMUB_IO_REGISTER_2_OFS 0x0058
REGDEF_BEGIN(LSU_DRAMUB_IO_REGISTER_2_OFS)
	REGDEF_BIT(DRAMUB_SAO0	, 32)	// bits :31_0
REGDEF_END(LSU_DRAMUB_IO_REGISTER_2_OFS)

	
#define LSU_DRAMUB_IO_REGISTER_3_OFS 0x005c
REGDEF_BEGIN(LSU_DRAMUB_IO_REGISTER_3_OFS)
	REGDEF_BIT(DRAMUB_MSB_SAO0	, 4)	// bits :3_0
	REGDEF_BIT(					, 24)	// bits :27_4
	REGDEF_BIT(DRAMUB_MODE_SAO0, 4)	// bits :31_28
REGDEF_END(LSU_DRAMUB_IO_REGISTER_3_OFS)


#define LSU_RESERVED_REGISTER_2_OFS 0x0060
REGDEF_BEGIN(LSU_RESERVED_REGISTER_2_OFS)
	REGDEF_BIT(    	, 32)	// bits , 31_0
REGDEF_END(LSU_RESERVED_REGISTER_2_OFS)


#define LSU_RESERVED_REGISTER_3_OFS 0x0064
REGDEF_BEGIN(LSU_RESERVED_REGISTER_3_OFS)
	REGDEF_BIT(    	, 32)	// bits , 31_0
REGDEF_END(LSU_RESERVED_REGISTER_3_OFS)


#define LSU_RESERVED_REGISTER_4_OFS 0x0068
REGDEF_BEGIN(LSU_RESERVED_REGISTER_4_OFS)
	REGDEF_BIT(    	, 32)	// bits , 31_0
REGDEF_END(LSU_RESERVED_REGISTER_4_OFS)


#define LSU_RESERVED_REGISTER_5_OFS 0x006c
REGDEF_BEGIN(LSU_RESERVED_REGISTER_5_OFS)
	REGDEF_BIT(    	, 32)	// bits , 31_0
REGDEF_END(LSU_RESERVED_REGISTER_5_OFS)

	
#define LSU_OFFSET_ENABLE_REGISTER_OFS 0x0070
REGDEF_BEGIN(LSU_OFFSET_ENABLE_REGISTER_OFS)
	REGDEF_BIT(LSU_IN0_LOFS_EN	, 1)	// bits :0
	REGDEF_BIT(LSU_IN0_CHOFS_EN	, 1)	// bits :1
	REGDEF_BIT(LSU_IN0_BOFS_EN	, 1)	// bits :2
	REGDEF_BIT(					, 13)	// bits :15_3
	REGDEF_BIT(LSU_OUT0_LOFS_EN	, 1)	// bits :16
	REGDEF_BIT(LSU_OUT0_CHOFS_EN, 1)	// bits :17
	REGDEF_BIT(LSU_OUT0_BOFS_EN	, 1)	// bits :18
REGDEF_END(LSU_OFFSET_ENABLE_REGISTER_OFS)

	
#define LSU_OFFSET_REGISTER_0_OFS 0x0074
REGDEF_BEGIN(LSU_OFFSET_REGISTER_0_OFS)
	REGDEF_BIT(LSU_IN0_LOFS	, 	 29)	// bits :28_0
REGDEF_END(LSU_OFFSET_REGISTER_0_OFS)

	
#define LSU_OFFSET_REGISTER_1_OFS 0x0078
REGDEF_BEGIN(LSU_OFFSET_REGISTER_1_OFS)
	REGDEF_BIT(LSU_IN0_CHOFS,	 29)	// bits :28_0
REGDEF_END(LSU_OFFSET_REGISTER_1_OFS)


#define LSU_OFFSET_REGISTER_2_OFS 0x007c
REGDEF_BEGIN(LSU_OFFSET_REGISTER_2_OFS)
	REGDEF_BIT(LSU_IN0_BOFS	,	 29)	// bits :28_0
REGDEF_END(LSU_OFFSET_REGISTER_2_OFS)


#define LSU_OFFSET_REGISTER_3_OFS 0x0080
REGDEF_BEGIN(LSU_OFFSET_REGISTER_3_OFS)
	REGDEF_BIT(LSU_OUT0_LOFS,	 29)	// bits :28_0
REGDEF_END(LSU_OFFSET_REGISTER_3_OFS)


#define LSU_OFFSET_REGISTER_4_OFS 0x0084
REGDEF_BEGIN(LSU_OFFSET_REGISTER_4_OFS)
	REGDEF_BIT(LSU_OUT0_CHOFS,	 29)	// bits :28_0
REGDEF_END(LSU_OFFSET_REGISTER_4_OFS)


#define LSU_OFFSET_REGISTER_5_OFS 0x0088
REGDEF_BEGIN(LSU_OFFSET_REGISTER_5_OFS)
	REGDEF_BIT(LSU_OUT0_BOFS,	 29)	// bits :28_0
REGDEF_END(LSU_OFFSET_REGISTER_5_OFS)


#define LSU_RESERVED_REGISTER_6_OFS 0x008c
REGDEF_BEGIN(LSU_RESERVED_REGISTER_6_OFS)
	REGDEF_BIT(    	, 32)	// bits , 31_0
REGDEF_END(LSU_RESERVED_REGISTER_6_OFS)


#define LSU_RESERVED_REGISTER_7_OFS 0x0090
REGDEF_BEGIN(LSU_RESERVED_REGISTER_7_OFS)
	REGDEF_BIT(    	, 32)	// bits , 31_0
REGDEF_END(LSU_RESERVED_REGISTER_7_OFS)


#define LSU_RESERVED_REGISTER_8_OFS 0x0094
REGDEF_BEGIN(LSU_RESERVED_REGISTER_8_OFS)
	REGDEF_BIT(    	, 32)	// bits , 31_0
REGDEF_END(LSU_RESERVED_REGISTER_8_OFS)


#define LSU_RESERVED_REGISTER_9_OFS 0x0098
REGDEF_BEGIN(LSU_RESERVED_REGISTER_9_OFS)
	REGDEF_BIT(    	, 32)	// bits , 31_0
REGDEF_END(LSU_RESERVED_REGISTER_9_OFS)


#define LSU_RESERVED_REGISTER_10_OFS 0x009c
REGDEF_BEGIN(LSU_RESERVED_REGISTER_10_OFS)
	REGDEF_BIT(    	, 32)	// bits , 31_0
REGDEF_END(LSU_RESERVED_REGISTER_10_OFS)


#define LSU_SIZE_REGISTER_0_OFS 0x00a0
REGDEF_BEGIN(LSU_SIZE_REGISTER_0_OFS)
	REGDEF_BIT(LSU_WIDTH,        20)	// bits :19_0
REGDEF_END(LSU_SIZE_REGISTER_0_OFS)

		
#define LSU_SIZE_REGISTER_1_OFS 0x00a4
REGDEF_BEGIN(LSU_SIZE_REGISTER_1_OFS)
	REGDEF_BIT(LSU_HEIGHT,       17)	// bits :16_0
REGDEF_END(LSU_SIZE_REGISTER_1_OFS)

	
#define LSU_SIZE_REGISTER_2_OFS 0x00a8
REGDEF_BEGIN(LSU_SIZE_REGISTER_2_OFS)
	REGDEF_BIT(LSU_CHANNEL,      17)	// bits :16_0
REGDEF_END(LSU_SIZE_REGISTER_2_OFS)


#define LSU_SIZE_REGISTER_3_OFS 0x00ac
REGDEF_BEGIN(LSU_SIZE_REGISTER_3_OFS)
	REGDEF_BIT(LSU_BATCH,         7)	// bits :6_0
REGDEF_END(LSU_SIZE_REGISTER_3_OFS)


#define LSU_ACT_COMPRESS_REGISTER_0_OFS 0x00b0
REGDEF_BEGIN(LSU_ACT_COMPRESS_REGISTER_0_OFS)
	REGDEF_BIT(AC_OFS_VAL,       16)	// bits :15_0
REGDEF_END(LSU_ACT_COMPRESS_REGISTER_0_OFS)


#define LSU_ACT_COMPRESS_REGISTER_1_OFS 0x00b4
REGDEF_BEGIN(LSU_ACT_COMPRESS_REGISTER_1_OFS)
	REGDEF_BIT(AC_KLOSSY_VAL,    	32)	// bits :31_0
REGDEF_END(LSU_ACT_COMPRESS_REGISTER_1_OFS)


#define LSU_RESERVED_REGISTER_11_OFS 0x00b8
REGDEF_BEGIN(LSU_RESERVED_REGISTER_11_OFS)
	REGDEF_BIT(    	, 32)	// bits , 31_0
REGDEF_END(LSU_RESERVED_REGISTER_11_OFS)


#define LSU_RESERVED_REGISTER_12_OFS 0x00bc
REGDEF_BEGIN(LSU_RESERVED_REGISTER_12_OFS)
	REGDEF_BIT(    	, 32)	// bits , 31_0
REGDEF_END(LSU_RESERVED_REGISTER_12_OFS)


#define LSU_CYCLE_COUNT_REGISTER_0_OFS 0x00c0
REGDEF_BEGIN(LSU_CYCLE_COUNT_REGISTER_0_OFS)
	REGDEF_BIT(LSU_ENG_CYCLE, 		32)	// bits :31_0
REGDEF_END(LSU_CYCLE_COUNT_REGISTER_0_OFS)


#define LSU_CYCLE_COUNT_REGISTER_1_OFS 0x00c4
REGDEF_BEGIN(LSU_CYCLE_COUNT_REGISTER_1_OFS)
	REGDEF_BIT(LSU_LL_CYCLE	, 		32)	// bits :31_0
REGDEF_END(LSU_CYCLE_COUNT_REGISTER_1_OFS)


#define LSU_CYCLE_COUNT_REGISTER_2_OFS 0x00c8
REGDEF_BEGIN(LSU_CYCLE_COUNT_REGISTER_2_OFS)
	REGDEF_BIT(LSU_WAIT_CYCLE	, 32)	// bits :31_0
REGDEF_END(LSU_CYCLE_COUNT_REGISTER_2_OFS)


#define LSU_RESERVED_REGISTER_13_OFS 0x00cc
REGDEF_BEGIN(LSU_RESERVED_REGISTER_13_OFS)
	REGDEF_BIT(    	, 32)	// bits , 31_0
REGDEF_END(LSU_RESERVED_REGISTER_13_OFS)


#define LSU_RESERVED_REGISTER_14_OFS 0x00d0
REGDEF_BEGIN(LSU_RESERVED_REGISTER_14_OFS)
	REGDEF_BIT(    	, 32)	// bits , 31_0
REGDEF_END(LSU_RESERVED_REGISTER_14_OFS)


#define LSU_RESERVED_REGISTER_15_OFS 0x00d4
REGDEF_BEGIN(LSU_RESERVED_REGISTER_15_OFS)
	REGDEF_BIT(    	, 32)	// bits , 31_0
REGDEF_END(LSU_RESERVED_REGISTER_15_OFS)


#define LSU_LL_FRAME_REGISTER_0_OFS 0x00d8
REGDEF_BEGIN(LSU_LL_FRAME_REGISTER_0_OFS)
    REGDEF_BIT(LL_TABLE_IDX0,        8)
    REGDEF_BIT(LL_TABLE_IDX1,        8)
    REGDEF_BIT(LL_TABLE_IDX2,        8)
    REGDEF_BIT(LL_TABLE_IDX3,        8)
REGDEF_END(LSU_LL_FRAME_REGISTER_0_OFS)


#define LSU_LL_FRAME_REGISTER_1_OFS 0x00dc
REGDEF_BEGIN(LSU_LL_FRAME_REGISTER_1_OFS)
    REGDEF_BIT(LL_TABLE_IDX4,        8)
    REGDEF_BIT(LL_TABLE_IDX5,        8)
    REGDEF_BIT(LL_TABLE_IDX6,        8)
    REGDEF_BIT(LL_TABLE_IDX7,        8)
REGDEF_END(LSU_LL_FRAME_REGISTER_1_OFS)


#define LSU_LL_FRAME_REGISTER_2_OFS 0x00e0
REGDEF_BEGIN(LSU_LL_FRAME_REGISTER_2_OFS)
    REGDEF_BIT(LL_TABLE_IDX8,        8)
    REGDEF_BIT(LL_TABLE_IDX9,        8)
    REGDEF_BIT(LL_TABLE_IDX10,       8)
    REGDEF_BIT(LL_TABLE_IDX11,       8)
REGDEF_END(LSU_LL_FRAME_REGISTER_2_OFS)


#define LSU_LL_FRAME_REGISTER_3_OFS 0x00e4
REGDEF_BEGIN(LSU_LL_FRAME_REGISTER_3_OFS)
    REGDEF_BIT(LL_TABLE_IDX12,       8)
    REGDEF_BIT(LL_TABLE_IDX13,       8)
    REGDEF_BIT(LL_TABLE_IDX14,       8)
    REGDEF_BIT(LL_TABLE_IDX15,       8)
REGDEF_END(LSU_LL_FRAME_REGISTER_3_OFS)


#define LSU_DMA_DISABLE_REGISTER_OFS 0x00e8
REGDEF_BEGIN(LSU_DMA_DISABLE_REGISTER_OFS)
	REGDEF_BIT(DMA_DISABLE 	, 		1)	// bits : 0
	REGDEF_BIT(					, 	14)	// bits : 14_1
	REGDEF_BIT(DMA_IDLE 	, 		1)	// bits : 15
REGDEF_END(LSU_DMA_DISABLE_REGISTER_OFS)


#define LSU_DISIGN_DEBUG_REGISTER_OFS 0x00ec
REGDEF_BEGIN(LSU_DISIGN_DEBUG_REGISTER_OFS)
	REGDEF_BIT(CHECK_SUM_ENABLE , 	1)	// bits : 0
	REGDEF_BIT(CYCLE_COUNT_EN , 	1)	// bits : 1
	REGDEF_BIT(					, 	2)	// bits : 3_2
	REGDEF_BIT(IN0_BURST_LENGTH	, 	2)	// bits : 5_4
	REGDEF_BIT(					, 	2)	// bits : 7_6
	REGDEF_BIT(OUT0_BURST_LENGTH, 	2)	// bits : 9_8
	REGDEF_BIT(					, 	2)	// bits : 11_10
	REGDEF_BIT(DBG_PORT_SEL, 		2)	// bits : 13_12
REGDEF_END(LSU_DISIGN_DEBUG_REGISTER_OFS)


#define LSU_AXI_REGISTER_0_OFS 0x00f0
REGDEF_BEGIN(LSU_AXI_REGISTER_0_OFS)
	REGDEF_BIT(IN0_CH_EN 	, 		1)	// bits : 0
	REGDEF_BIT(					, 	3)	// bits : 3_1
	REGDEF_BIT(OUT0_CH_EN 	, 		1)	// bits : 4
	REGDEF_BIT( 				, 	1)	// bits : 5
	REGDEF_BIT(LLC_IN_CH_EN , 		1)	// bits : 6
	REGDEF_BIT( 				, 	1)	// bits : 7
	REGDEF_BIT(IN0_LOCK_DISABLE , 	1)	// bits : 8
	REGDEF_BIT(					, 	3)	// bits : 11_9
	REGDEF_BIT(OUT0_LOCK_DISABLE , 	1)	// bits : 12
	REGDEF_BIT( 				, 	1)	// bits : 13
	REGDEF_BIT(LLC_IN_LOCK_DISABLE ,1)	// bits : 14
	REGDEF_BIT( 				, 	1)	// bits : 15
	REGDEF_BIT(WRITE_CH_OUTSTANDING_NUM ,8)	// bits : 23_16
	REGDEF_BIT(READ_CH_OUTSTANDING_NUM  ,8)	// bits : 31_24
REGDEF_END(LSU_AXI_REGISTER_0_OFS)


#define LSU_AXI_REGISTER_1_OFS 0x00f4
REGDEF_BEGIN(LSU_AXI_REGISTER_1_OFS)
	REGDEF_BIT(AXI_BUS_DISABLE, 	1)	// bits : 0
	REGDEF_BIT(					, 	15)	// bits : 15_1
	REGDEF_BIT(AXI_BUS_IDLE , 		1)	// bits : 16
REGDEF_END(LSU_AXI_REGISTER_1_OFS)


#define LSU_AXI_REGISTER_2_OFS 0x00f8
REGDEF_BEGIN(LSU_AXI_REGISTER_2_OFS)
	REGDEF_BIT(AXI_STATUS, 			32)	// bits : 31_0
REGDEF_END(LSU_AXI_REGISTER_2_OFS)


#define LSU_RESERVED_REGISTER_16_OFS 0x00fc
REGDEF_BEGIN(LSU_RESERVED_REGISTER_16_OFS)
	REGDEF_BIT(    	, 32)	// bits , 31_0
REGDEF_END(LSU_RESERVED_REGISTER_16_OFS)


#define LSU_LLC_DEBUG_REGISTER_0_OFS 0x0100
REGDEF_BEGIN(LSU_LLC_DEBUG_REGISTER_0_OFS)
	REGDEF_BIT(LLC_ERR_CMD_ADR0, 	32)	// bits : 31_0
REGDEF_END(LSU_LLC_DEBUG_REGISTER_0_OFS)


#define LSU_LLC_DEBUG_REGISTER_1_OFS 0x0104
REGDEF_BEGIN(LSU_LLC_DEBUG_REGISTER_1_OFS)
	REGDEF_BIT(LLC_ERR_CMD_ADR1, 	32)	// bits : 31_0
REGDEF_END(LSU_LLC_DEBUG_REGISTER_1_OFS)


#define LSU_RESERVED_REGISTER_17_OFS 0x0108
REGDEF_BEGIN(LSU_RESERVED_REGISTER_17_OFS)
	REGDEF_BIT(    	, 32)	// bits , 31_0
REGDEF_END(LSU_RESERVED_REGISTER_17_OFS)


#define LSU_CODE_VERSION_REGISTER_OFS 0x010c
REGDEF_BEGIN(LSU_CODE_VERSION_REGISTER_OFS)
	REGDEF_BIT(RTL_VERSION, 		32)	// bits : 31_0
REGDEF_END(LSU_CODE_VERSION_REGISTER_OFS)


#define LSU_CHECKSUM_REGISTER_0_OFS 0x0110
REGDEF_BEGIN(LSU_CHECKSUM_REGISTER_0_OFS)
	REGDEF_BIT(CHKSUM_DMAI,		 32)	// bits , 31_0
REGDEF_END(LSU_CHECKSUM_REGISTER_0_OFS)


#define LSU_CHECKSUM_REGISTER_1_OFS 0x0114
REGDEF_BEGIN(LSU_CHECKSUM_REGISTER_1_OFS)
	REGDEF_BIT(CHKSUM_DPK,		 32)	// bits , 31_0
REGDEF_END(LSU_CHECKSUM_REGISTER_1_OFS)


#define LSU_CHECKSUM_REGISTER_2_OFS 0x0118
REGDEF_BEGIN(LSU_CHECKSUM_REGISTER_2_OFS)
	REGDEF_BIT(CHKSUM_PACK,		 32)	// bits , 31_0
REGDEF_END(LSU_CHECKSUM_REGISTER_2_OFS)


#define LSU_CHECKSUM_REGISTER_3_OFS 0x011c
REGDEF_BEGIN(LSU_CHECKSUM_REGISTER_3_OFS)
	REGDEF_BIT(CHKSUM_DMAO,		 32)	// bits , 31_0
REGDEF_END(LSU_CHECKSUM_REGISTER_3_OFS)



typedef struct
{

  union
  {
    struct
    {
      unsigned LSU_SW_RST          : 1;		// bits : 0
      unsigned LSU_START           : 1;		// bits : 1
      unsigned                     : 26;
      unsigned JOB_START           : 1;		// bits : 28
      unsigned JOB_TERMINATE       : 1;		// bits : 29
    } Bit;
    UINT32 Word;
  } LSU_Register_0; // 0x0000

  union
  {
    struct
    {
      unsigned INTE_FRM_DONE         : 1;		// bits : 0
	  unsigned                       : 3;
	  unsigned INTE_ACD_BS_LEN_ERR   : 1;		// bits : 4
      unsigned                       : 19;
      unsigned INTE_JOB_END          : 1;		// bits : 24
      unsigned INTE_JOB_ERR          : 1;		// bits : 25
      unsigned INTE_JOB_WR_END       : 1;		// bits : 26
    } Bit;
    UINT32 Word;
  } LSU_Register_1; // 0x0004

  union
  {
    struct
    {
      unsigned INTS_FRM_DONE         : 1;		// bits : 0
	  unsigned                       : 3;
	  unsigned INTS_ACD_BS_LEN_ERR   : 1;		// bits : 4
      unsigned                       : 19;
      unsigned INTS_JOB_END          : 1;		// bits : 24
      unsigned INTS_JOB_ERR          : 1;		// bits : 25
      unsigned INTS_JOB_WR_END        : 1;		// bits : 26
    } Bit;
    UINT32 Word;
  } LSU_Register_2; // 0x0008

  union
  {
    struct
    {
      unsigned LSU_MODE              : 2;		// bits : 1_0
    } Bit;
    UINT32 Word;
  } LSU_Register_3; // 0x000c

  union
  {
    struct
    {
      unsigned AC_ENC_KLOSSY_EN      : 1;		// bits : 0
	  unsigned                       : 3;
      unsigned AC_BC_OFS_EN          : 1;		// bits : 4
	  unsigned                       : 3;
      unsigned AC_BC_MODE            : 1;		// bits : 8
    } Bit;
    UINT32 Word;
  } LSU_Register_4; // 0x0010

  union
  {
    struct
    {
      unsigned IN_BIT_DEPTH          : 3;		// bits : 2_0
	  unsigned                       : 1;
      unsigned IN_DATA_FMT           : 2;		// bits : 5_4
    } Bit;
    UINT32 Word;
  } LSU_Register_5; // 0x0014

  union
  {
    struct
    {
      unsigned                      : 32;
    } Bit;
    UINT32 Word;
  } LSU_Register_6; // 0x0018

  union
  {
    struct
    {
      unsigned                      : 32;
    } Bit;
    UINT32 Word;
  } LSU_Register_7; // 0x001c

  union
  {
    struct
    {
      unsigned BASE_ADDR0            : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } LSU_Register_8; // 0x0020

  union
  {
    struct
    {
      unsigned BASE_MSB_ADDR0        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } LSU_Register_9; // 0x0024

  union
  {
    struct
    {
      unsigned BASE_ADDR1            : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } LSU_Register_10; // 0x0028

  union
  {
    struct
    {
      unsigned BASE_MSB_ADDR1        : 4;		// bits : 3_0
	} Bit;
    UINT32 Word;
  } LSU_Register_11; // 0x002c

  union
  {
    struct
    {
      unsigned BASE_ADDR2            : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } LSU_Register_12; // 0x0030

  union
  {
    struct
    {
      unsigned BASE_MSB_ADDR2        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } LSU_Register_13; // 0x0034

  union
  {
    struct
    {
      unsigned BASE_ADDR3            : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } LSU_Register_14; // 0x0038

  union
  {
    struct
    {
      unsigned BASE_MSB_ADDR3        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } LSU_Register_15; // 0x003c

  union
  {
    struct
    {
      unsigned DRAMUB_SAIJOB         : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } LSU_Register_16; // 0x0040

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAIJOB    : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } LSU_Register_17; // 0x0044

  union
  {
    struct
    {
      unsigned BASE_ADDR4            : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } LSU_Register_18; // 0x0048

  union
  {
    struct
    {
      unsigned BASE_MSB_ADDR4        : 4;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } LSU_Register_19; // 0x004c

  union
  {
    struct
    {
      unsigned DRAMUB_SAI0           : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } LSU_Register_20; // 0x0050

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAI0       : 4;		// bits : 3_0
      unsigned                       : 24;		// bits : 27_4
      unsigned DRAMUB_MODE_SAI0      : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } LSU_Register_21; // 0x0054

  union
  {
    struct
    {
      unsigned DRAMUB_SAO0          : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } LSU_Register_22; // 0x0058

  union
  {
    struct
    {
      unsigned DRAMUB_MSB_SAO0      : 4;		// bits : 3_0
      unsigned                       : 24;		// bits : 27_4
      unsigned DRAMUB_MODE_SAO0      : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } LSU_Register_23; // 0x005c

  union
  {
    struct
    {
      unsigned                      : 32;
    } Bit;
    UINT32 Word;
  } LSU_Register_24; // 0x0060

  union
  {
    struct
    {
      unsigned                      : 32;
    } Bit;
    UINT32 Word;
  } LSU_Register_25; // 0x0064

  union
  {
    struct
    {
      unsigned                      : 32;
    } Bit;
    UINT32 Word;
  } LSU_Register_26; // 0x0068

  union
  {
    struct
    {
      unsigned                      : 32;
    } Bit;
    UINT32 Word;
  } LSU_Register_27; // 0x006c

  union
  {
    struct
    {
      unsigned LSU_IN0_LOFS_EN       : 1;		// bits : 0
	  unsigned LSU_IN0_CHOFS_EN      : 1;		// bits : 1
	  unsigned LSU_IN0_BOFS_EN       : 1;		// bits : 2
	  unsigned                       : 13;		// bits : 15_3
	  unsigned LSU_OUT0_LOFS_EN      : 1;		// bits : 16
	  unsigned LSU_OUT0_CHOFS_EN     : 1;		// bits : 17
	  unsigned LSU_OUT0_BOFS_EN      : 1;		// bits : 18

      unsigned Result7        : 16;		// bits : 31_16
    } Bit;
    UINT32 Word;
  } LSU_Register_28; // 0x0070

  union
  {
    struct
    {
      unsigned LSU_IN0_LOFS          : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } LSU_Register_29; // 0x0074

  union
  {
    struct
    {
      unsigned LSU_IN0_CHOFS         : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } LSU_Register_30; // 0x0078

  union
  {
    struct
    {
      unsigned LSU_IN0_BOFS          : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } LSU_Register_31; // 0x007c

  union
  {
    struct
    {
      unsigned LSU_OUT0_LOFS         : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } LSU_Register_32; // 0x0080

  union
  {
    struct
    {
      unsigned LSU_OUT0_CHOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } LSU_Register_33; // 0x0084

  union
  {
    struct
    {
      unsigned LSU_OUT0_BOFS         : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } LSU_Register_34; // 0x0088

  union
  {
    struct
    {
      unsigned                      : 32;
    } Bit;
    UINT32 Word;
  } LSU_Register_35; // 0x008c

  union
  {
    struct
    {
      unsigned                      : 32;
    } Bit;
    UINT32 Word;
  } LSU_Register_36; // 0x0090

  union
  {
    struct
    {
      unsigned                      : 32;
    } Bit;
    UINT32 Word;
  } LSU_Register_37; // 0x0094

  union
  {
    struct
    {
      unsigned                      : 32;
    } Bit;
    UINT32 Word;
  } LSU_Register_38; // 0x0098

  union
  {
    struct
    {
      unsigned                      : 32;
    } Bit;
    UINT32 Word;
  } LSU_Register_39; // 0x009c

  union
  {
    struct
    {
      unsigned LSU_WIDTH             : 20;		// bits : 19_0
    } Bit;
    UINT32 Word;
  } LSU_Register_40; // 0x00a0

  union
  {
    struct
    {
      unsigned LSU_HEIGHT            : 17;		// bits : 16_0
    } Bit;
    UINT32 Word;
  } LSU_Register_41; // 0x00a4

  union
  {
    struct
    {
      unsigned LSU_CHANNEL           : 17;		// bits : 16_0
    } Bit;
    UINT32 Word;
  } LSU_Register_42; // 0x00a8

  union
  {
    struct
    {
      unsigned LSU_BATCH             : 7;		// bits : 6_0
    } Bit;
    UINT32 Word;
  } LSU_Register_43; // 0x00ac

  union
  {
    struct
    {
      unsigned AC_OFS_VAL            : 16;		// bits : 15_0
    } Bit;
    UINT32 Word;
  } LSU_Register_44; // 0x00b0

  union
  {
    struct
    {
      unsigned AC_KLOSSY_VAL         : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } LSU_Register_45; // 0x00b4
  
  union
  {
    struct
    {
      unsigned                      : 32;
    } Bit;
    UINT32 Word;
  } LSU_Register_46; // 0x00b8
  
  union
  {
    struct
    {
      unsigned                      : 32;
    } Bit;
    UINT32 Word;
  } LSU_Register_47; // 0x00bc
  
  union
  {
    struct
    {
      unsigned LSU_ENG_CYCLE         : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } LSU_Register_48; // 0x00c0
  
  union
  {
    struct
    {
      unsigned LSU_LL_CYCLE          : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } LSU_Register_49; // 0x00c4
  
  union
  {
    struct
    {
      unsigned LSU_WAIT_CYCLE        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } LSU_Register_50; // 0x00c8
  
  union
  {
    struct
    {
      unsigned                      : 32;
    } Bit;
    UINT32 Word;
  } LSU_Register_51; // 0x00cc
  
  union
  {
    struct
    {
      unsigned                      : 32;
    } Bit;
    UINT32 Word;
  } LSU_Register_52; // 0x00d0
  
  union
  {
    struct
    {
      unsigned                      : 32;
    } Bit;
    UINT32 Word;
  } LSU_Register_53; // 0x00d4
  
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
  } LSU_Register_54; // 0x00d8
  
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
  } LSU_Register_55; // 0x00dc
  
  union
  {
    struct
    {
      unsigned LL_TABLE_IDX8        : 8;		// bits : 7_0
      unsigned LL_TABLE_IDX9        : 8;		// bits : 15_8
      unsigned LL_TABLE_IDX10       : 8;		// bits : 23_16
      unsigned LL_TABLE_IDX11       : 8;		// bits : 31_24
    } Bit;
    UINT32 Word;
  } LSU_Register_56; // 0x00e0
  
  union
  {
    struct
    {
      unsigned LL_TABLE_IDX12       : 8;		// bits : 7_0
      unsigned LL_TABLE_IDX13       : 8;		// bits : 15_8
      unsigned LL_TABLE_IDX14       : 8;		// bits : 23_16
      unsigned LL_TABLE_IDX15       : 8;		// bits : 31_24
    } Bit;
    UINT32 Word;
  } LSU_Register_57; // 0x00e4
  
  union
  {
    struct
    {
      unsigned DMA_DISABLE          : 1;		// bits : 0
	  unsigned                      : 14;		// bits : 14_1
      unsigned DMA_IDLE             : 1;		// bits : 15
    } Bit;
    UINT32 Word;
  } LSU_Register_58; // 0x00e8
  
  union
  {
    struct
    {
      unsigned CHECK_SUM_ENABLE     : 1;		// bits : 0
      unsigned CYCLE_COUNT_EN       : 1;		// bits : 1
	  unsigned                      : 2;		// bits : 3_2
      unsigned IN0_BURST_LENGTH     : 2;		// bits : 5_4
	  unsigned                      : 2;		// bits : 7_6
      unsigned OUT0_BURST_LENGTH    : 2;		// bits : 9_8
	  unsigned                      : 2;		// bits : 11_10
      unsigned DBG_PORT_SEL         : 2;		// bits : 13_12
    } Bit;
    UINT32 Word;
  } LSU_Register_59; // 0x00ec
  
  union
  {
    struct
    {
      unsigned IN0_CH_EN            : 1;		// bits : 0
	  unsigned                      : 3;		// bits : 3_1
      unsigned OUT0_CH_EN           : 1;		// bits : 4
	  unsigned                      : 1;		// bits : 5
      unsigned LLC_IN_CH_EN         : 1;		// bits : 6
	  unsigned                      : 1;		// bits : 7
      unsigned IN0_LOCK_DISABLE     : 1;		// bits : 8
	  unsigned                      : 3;		// bits : 11_9
      unsigned OUT0_LOCK_DISABLE    : 1;		// bits : 12
	  unsigned                      : 1;		// bits : 13
      unsigned LLC_IN_LOCK_DISABLE  : 1;		// bits : 14
	  unsigned                      : 1;		// bits : 15
      unsigned WRITE_CH_OUTSTANDING_NUM  : 8;	// bits : 23_16
      unsigned READ_CH_OUTSTANDING_NUM   : 8;	// bits : 31_24
    } Bit;
    UINT32 Word;
  } LSU_Register_60; // 0x00f0
  
  union
  {
    struct
    {
      unsigned AXI_BUS_DISABLE      : 1;		// bits : 0
	  unsigned                      : 15;		// bits : 15_1
      unsigned AXI_BUS_IDLE         : 1;		// bits : 16
    } Bit;
    UINT32 Word;
  } LSU_Register_61; // 0x00f4
  
  union
  {
    struct
    {
      unsigned AXI_STATUS           : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } LSU_Register_62; // 0x00f8
  
  union
  {
    struct
    {
      unsigned                      : 32;
    } Bit;
    UINT32 Word;
  } LSU_Register_63; // 0x00fc
  
  union
  {
    struct
    {
      unsigned LLC_ERR_CMD_ADR0     : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } LSU_Register_64; // 0x0100
  
  union
  {
    struct
    {
      unsigned LLC_ERR_CMD_ADR1     : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } LSU_Register_65; // 0x0104
  
  union
  {
    struct
    {
      unsigned                      : 32;
    } Bit;
    UINT32 Word;
  } LSU_Register_66; // 0x0108
  
  union
  {
    struct
    {
      unsigned RTL_VERSION          : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } LSU_Register_67; // 0x010c
  
  union
  {
    struct
    {
      unsigned CHKSUM_DMAI          : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } LSU_Register_68; // 0x0110
  
  union
  {
    struct
    {
      unsigned CHKSUM_DPK           : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } LSU_Register_69; // 0x0114
  
  union
  {
    struct
    {
      unsigned CHKSUM_PACK          : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } LSU_Register_70; // 0x0118
  
  union
  {
    struct
    {
      unsigned CHKSUM_DMAO          : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } LSU_Register_71; // 0x011c
  
} NT98690_LSU_REGISTER_STRUCT;


#ifdef __cplusplus
}
#endif

#endif
