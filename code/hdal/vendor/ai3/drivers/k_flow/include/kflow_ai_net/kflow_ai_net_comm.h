/**
    @brief Header file of common definition of vendor net flow sample.

    @file net_common.h

    @ingroup net_flow_sample

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _NET_COMMON_H_
#define _NET_COMMON_H_

/********************************************************************
    INCLUDE FILES
********************************************************************/
#if defined(__LINUX)
#include <linux/kernel.h>   // for snprintf
#elif defined(__FREERTOS)
#include <stdio.h>          // for snprintf
#include "kwrap/type.h"     // for snprintf
#else
#include "kwrap/type.h"     // for snprintf
#endif

/********************************************************************
    MACRO CONSTANT DEFINITIONS
********************************************************************/
#define DBG_MSG_DUMP            0

//dump output buffer
#define DBG_OUT_DUMP			0
#define STR_MAX_LENGTH          256
#define DBG_OUT_PATH			"/mnt/sd/ai_dumpbuf"

//dump job register
#define DBG_REG_DUMP			0

//dump job time
#define DBG_TIME_DUMP			0

#define NN_IMEM_NUM             20
#define NN_OMEM_NUM             3

#define USE_HTE                 1

#define _CNN30_TODO_            1

/********************************************************************
    TYPE DEFINITION
********************************************************************/

#if (!defined(__KERNEL__))   // only allow USER SPACE

typedef enum {
	BUF_IN_IDX = 0,
	BUF_PARM_IDX0 = 1,
	BUF_INTERM_IDX,
	BUF_PARM_IDX1,
	BUF_PARM_IDX2,

	BUF_TOTAL_IDX_INMEM,
	ENUM_DUMMY4WORD(BUF_INMEM_IDX_TYPE)
} BUF_INMEM_IDX_TYPE;

typedef enum {
	BUF_OUT_IDX0 = 0,
	BUF_OUT_IDX1 = 1,

	BUF_TOTAL_IDX_OUTMEM,
	ENUM_DUMMY4WORD(BUF_OUTMEM_IDX_TYPE)
} BUF_OUTMEM_IDX_TYPE;

typedef enum {
	// input
	CONV_IN0 = 0, // input
	CONV_IN1 = 1, // eltwise
	CONV_IN_CH_STRIPE = 2,

	// weight
	CONV_W_STREAM0_OCH0 = 3,
	CONV_W_STREAM0_OCH1 = 4,
	CONV_W_STREAM0_OCH2 = 5,
	CONV_W_STREAM0_OCH3 = 6,
	CONV_BIAS_QCONVOUT = 7, // BIAS + per-ch quan
	CONV_BN_W = 8, // BN packed
	CONV_ACT_W6 = 9, // ACT Weight
	CONV_ACT_W7 = 10, // ACT LUT
    CONV_PP_DW_W8 = 11, // DW (weight + bias + ACT) + per_ch quan

	// pre input
	CONV_PRE_IN0 = 12,

	// wcd
	CONV_WCD_STREAM1_OCH0 = 15,
	CONV_WCD_STREAM1_OCH1 = 16,
	CONV_WCD_STREAM1_OCH2 = 17,
	CONV_WCD_STREAM1_OCH3 = 18,
	CONV_WCD_KMEANS_TBL = 19,

	CONV_TOTAL_IDX_INMEM,
	ENUM_DUMMY4WORD(CONV_INMEM_IDX_TYPE)
} CONV_INMEM_IDX_TYPE;

typedef enum {
	CONV_OUT0 = 0,

	CONV_TOTAL_IDX_OUTMEM,
	ENUM_DUMMY4WORD(CONV_OUTMEM_IDX_TYPE)
} CONV_OUTMEM_IDX_TYPE;

typedef enum {
	// input
	ROU_IN0 = 0, // input
	ROU_IN1 = 1, // roi corrdinate for ROIPool / mask for ACTNorm / alpha&beta for Norm
	ROU_IN_NORM_MEAN = 2, // mean for Norm

	ROU_TOTAL_IDX_INMEM,
	ENUM_DUMMY4WORD(ROU_INMEM_IDX_TYPE)
} ROU_INMEM_IDX_TYPE;

typedef enum {
	ROU_OUT0 = 0,

	ROU_TOTAL_IDX_OUTMEM,
	ENUM_DUMMY4WORD(ROU_OUTMEM_IDX_TYPE)
} ROU_OUTMEM_IDX_TYPE;

typedef enum {
	NUE2_IN_IDX0 = BUF_IN_IDX,
	NUE2_IN_IDX1 = 1,
	NUE2_IN_IDX2 = 2,

	NUE2_TOTAL_IDX_INMEM,
	ENUM_DUMMY4WORD(NUE2_INMEM_IDX_TYPE)
} NUE2_INMEM_IDX_TYPE;

typedef enum {
	NUE2_OUT_IDX0 = BUF_OUT_IDX0,
	NUE2_OUT_IDX1 = 1,
	NUE2_OUT_IDX2 = 2,

	NUE2_TOTAL_IDX_OUTMEM,
	ENUM_DUMMY4WORD(NUE2_OUTMEM_IDX_TYPE)
} NUE2_OUTMEM_IDX_TYPE;

typedef enum {
	// input
	CAL_IN0 = 0,
	CAL_IN1 = 1,
	CAL_IN2 = 2,

	CAL_TOTAL_IDX_INMEM,
	ENUM_DUMMY4WORD(CAL_INMEM_IDX_TYPE)
} CAL_INMEM_IDX_TYPE;

typedef enum {
	CAL_OUT0 = 0,
	CAL_OUT1 = 1,

	CAL_TOTAL_IDX_OUTMEM,
	ENUM_DUMMY4WORD(CAL_OUTMEM_IDX_TYPE)
} CAL_OUTMEM_IDX_TYPE;

typedef enum {
	// input
	UTIL_IN0 = 0,
	UTIL_IN1 = 1,
	UTIL_IN2 = 2,

	UTIL_TOTAL_IDX_INMEM,
	ENUM_DUMMY4WORD(UTIL_INMEM_IDX_TYPE)
} UTIL_INMEM_IDX_TYPE;

typedef enum {
	UTIL_OUT0 = 0,
	UTIL_OUT1 = 1,

	UTIL_TOTAL_IDX_OUTMEM,
	ENUM_DUMMY4WORD(UTIL_OUTMEM_IDX_TYPE)
} UTIL_OUTMEM_IDX_TYPE;

typedef enum {
	// input
	LSU_IN0 = 0,

	LSU_TOTAL_IDX_INMEM,
	ENUM_DUMMY4WORD(LSU_INMEM_IDX_TYPE)
} LSU_INMEM_IDX_TYPE;

typedef enum {
	LSU_OUT0 = 0,

	LSU_TOTAL_IDX_OUTMEM,
	ENUM_DUMMY4WORD(LSU_OUTMEM_IDX_TYPE)
} LSU_OUTMEM_IDX_TYPE;

typedef enum {
	PPU_IN_IDX0 = BUF_IN_IDX,
	PPU_IN_IDX1 = 1,
	PPU_IN_IDX2 = 2,

	PPU_TOTAL_IDX_INMEM,
	ENUM_DUMMY4WORD(PPU_INMEM_IDX_TYPE)
} PPU_INMEM_IDX_TYPE;

typedef enum {
	PPU_OUT_IDX0 = BUF_OUT_IDX0,
	PPU_OUT_IDX1 = 1,
	PPU_OUT_IDX2 = 2,

	PPU_TOTAL_IDX_OUTMEM,
	ENUM_DUMMY4WORD(PPU_OUTMEM_IDX_TYPE)
} PPU_OUTMEM_IDX_TYPE;

//========== for first layer linked list mode ==========
typedef struct {
	unsigned : 32;
	unsigned : 32;
} ENG_NULL_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned addr : 32;
	} bit;
} ENG_ADDR_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned ofs : 29;
	} bit;
} ENG_OFS_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned width : 12;
		unsigned : 4;
		unsigned height : 12;
	} bit;
} CONV_SIZE0_REG;

/*
batch_num of 539A has been increased to 12bits,
but 538/539A cannot be separated at compile time,
beacuse sdk does not modify this value,
we just comment here, no need of modification.
*/
typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned channel : 16;
		unsigned : 4;
#if defined(_BSP_NS02401_) 
		unsigned batch_num : 12;
#else
		unsigned batch_num : 7; //539A => 12bits
#endif
	} bit;
} CONV_SIZE1_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned width : 17;
	} bit;
} ROU_SIZE0_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned height : 17;
	} bit;
} ROU_SIZE1_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned channel : 17;
	} bit;
} ROU_SIZE2_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned width : 13;
		unsigned : 3;
		unsigned height : 13;
	} bit;
} NUE2_SIZE0_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned ofs : 17;
	} bit;
} NUE2_LOFS_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned h_rate : 7;
		unsigned : 1;
		unsigned v_rate : 7;
	} bit;
} NUE2_SCL0_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned h_sfact : 16;
		unsigned v_sfact : 16;
	} bit;
} NUE2_SCL1_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned h_scl_size : 13;
		unsigned : 3;
		unsigned v_scl_size : 13;
	} bit;
} NUE2_SCL_SIZE_REG;

typedef union {
       UINT64 reg;
       struct {
			unsigned : 32;
            unsigned yuv2rgb_en: 1;
			unsigned sub_en: 1;
			unsigned pad_en: 1;
			unsigned : 1;
			unsigned rotate_en: 1;
			unsigned reserved: 7;
			unsigned in_fmt: 2;
			unsigned out_signedness: 1;
			unsigned sub_mode: 1;
       } bit;
} NUE2_FUNC_EN_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned width : 11;
	} bit;
} PPU_SIZE0_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned height : 11;
	} bit;
} PPU_SIZE1_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned width : 17;
	} bit;
} CAL_SIZE0_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned height : 17;
	} bit;
} CAL_SIZE1_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned channel : 12;
	} bit;
} CAL_SIZE2_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned width : 12;
		unsigned : 4;
		unsigned height : 12;
	} bit;
} UTIL_SIZE0_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned channel : 12;
	} bit;
} UTIL_SIZE1_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned width : 20;
	} bit;
} LSU_SIZE0_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned height : 17;
	} bit;
} LSU_SIZE1_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned channel : 17;
	} bit;
} LSU_SIZE2_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned batch : 7;
	} bit;
} LSU_SIZE3_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned in_los_en : 1;
		unsigned in_chofs_en : 1;
		unsigned in_bofs_en : 1;
		unsigned : 13;
		unsigned out_los_en : 1;
		unsigned out_chofs_en : 1;
		unsigned out_bofs_en : 1;
	} bit;
} LSU_OFFSET_EN_REG;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned ofs : 29;
	} bit;
} LSU_LOFS_REG;

#if defined(_BSP_NS02201_) 
typedef struct _CONV_LL_PARM {
	// addr
	ENG_ADDR_REG input;
	ENG_ADDR_REG input_msb;
	ENG_ADDR_REG elt_in;
	ENG_ADDR_REG elt_in_msb;
	ENG_ADDR_REG inter_in;
	ENG_ADDR_REG inter_in_msb;
	ENG_ADDR_REG weight_O0_in;
	ENG_ADDR_REG weight_O0_in_msb;
	ENG_ADDR_REG weight_O1_in;
	ENG_ADDR_REG weight_O1_in_msb;
	ENG_ADDR_REG weight_O2_in;
	ENG_ADDR_REG weight_O2_in_msb;
	ENG_ADDR_REG weight_O3_in;
	ENG_ADDR_REG weight_O3_in_msb;
	ENG_ADDR_REG bias_in;
	ENG_ADDR_REG bias_in_msb;
	ENG_ADDR_REG bn_in;
	ENG_ADDR_REG bn_in_msb;
	ENG_ADDR_REG act_weight_in;
	ENG_ADDR_REG act_weight_in_msb;
	ENG_ADDR_REG act_LUT_in;
	ENG_ADDR_REG act_LUT_in_msb;
	ENG_ADDR_REG DW_weight_in;
	ENG_ADDR_REG DW_weight_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O0_in;
	ENG_ADDR_REG VLC_bitstram1_O0_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O1_in;
	ENG_ADDR_REG VLC_bitstram1_O1_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O2_in;
	ENG_ADDR_REG VLC_bitstram1_O2_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O3_in;
	ENG_ADDR_REG VLC_bitstram1_O3_in_msb;
	ENG_ADDR_REG kmeans_in;
	ENG_ADDR_REG kmeans_in_msb;
	ENG_ADDR_REG preIn0_in;
	ENG_ADDR_REG preIn0_in_msb;
	ENG_ADDR_REG output;
	ENG_ADDR_REG output_msb;
	// input size
	CONV_SIZE0_REG size0; // in0 width height
	CONV_SIZE1_REG size1; // in0 channel batch
	CONV_SIZE0_REG size2; // prein0 width height
	CONV_SIZE1_REG size3; // prein0 channel batch
} CONV_LL_PARM;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned in0_los_en : 1;
		unsigned in0_chofs_en : 1;
		unsigned in0_bofs_en : 1;
		unsigned : 1;
		unsigned in1_los_en : 1;
		unsigned in1_chofs_en : 1;
		unsigned in1_bofs_en : 1;
		unsigned : 1;
		unsigned inch_los_en : 1;
		unsigned inch_chofs_en : 1;
		unsigned inch_bofs_en : 1;
		unsigned : 1;
		unsigned w_los_en : 1;
		unsigned w_chofs_en : 1;
		unsigned w_bofs_en : 1;
		unsigned w_dofs_en : 1;
		unsigned out0_los_en : 1;
		unsigned out0_chofs_en : 1;
		unsigned out0_bofs_en : 1;
	} bit;
} CONV_OFFSET_EN_REG;
typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned ofs : 29;
	} bit;
} CONV_LOFS_REG;
typedef struct _CONV_LL_PARM_WITH_WIDTH_STRIDE {
	// addr
	ENG_ADDR_REG input;
	ENG_ADDR_REG input_msb;
	ENG_ADDR_REG elt_in;
	ENG_ADDR_REG elt_in_msb;
	ENG_ADDR_REG inter_in;
	ENG_ADDR_REG inter_in_msb;
	ENG_ADDR_REG weight_O0_in;
	ENG_ADDR_REG weight_O0_in_msb;
	ENG_ADDR_REG weight_O1_in;
	ENG_ADDR_REG weight_O1_in_msb;
	ENG_ADDR_REG weight_O2_in;
	ENG_ADDR_REG weight_O2_in_msb;
	ENG_ADDR_REG weight_O3_in;
	ENG_ADDR_REG weight_O3_in_msb;
	ENG_ADDR_REG bias_in;
	ENG_ADDR_REG bias_in_msb;
	ENG_ADDR_REG bn_in;
	ENG_ADDR_REG bn_in_msb;
	ENG_ADDR_REG act_weight_in;
	ENG_ADDR_REG act_weight_in_msb;
	ENG_ADDR_REG act_LUT_in;
	ENG_ADDR_REG act_LUT_in_msb;
	ENG_ADDR_REG DW_weight_in;
	ENG_ADDR_REG DW_weight_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O0_in;
	ENG_ADDR_REG VLC_bitstram1_O0_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O1_in;
	ENG_ADDR_REG VLC_bitstram1_O1_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O2_in;
	ENG_ADDR_REG VLC_bitstram1_O2_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O3_in;
	ENG_ADDR_REG VLC_bitstram1_O3_in_msb;
	ENG_ADDR_REG kmeans_in;
	ENG_ADDR_REG kmeans_in_msb;
	ENG_ADDR_REG preIn0_in;
	ENG_ADDR_REG preIn0_in_msb;
	ENG_ADDR_REG output;
	ENG_ADDR_REG output_msb;
	// input size
	CONV_SIZE0_REG size0; // in0 width height 
	CONV_SIZE1_REG size1; // in0 channel batch
	CONV_SIZE0_REG size2; // prein0 width height
	CONV_SIZE1_REG size3; // prein0 channel batch
	// offsetEn
	CONV_OFFSET_EN_REG offset_en;
	// in_offset
	CONV_LOFS_REG in0_lofs[3];
	CONV_LOFS_REG in1_lofs[3];
	CONV_LOFS_REG w_lofs[4]; // los dofs chofs bofs
} CONV_LL_PARM_WITH_WIDTH_STRIDE;
#endif
#if defined(_BSP_NS02302_)
typedef struct _CONV_LL_PARM {
	// addr
	ENG_ADDR_REG input;
	ENG_ADDR_REG input_msb;
	ENG_ADDR_REG elt_in;
	ENG_ADDR_REG elt_in_msb;
	ENG_ADDR_REG inter_in;
	ENG_ADDR_REG inter_in_msb;
	ENG_ADDR_REG weight_O0_in;
	ENG_ADDR_REG weight_O0_in_msb;
	ENG_ADDR_REG weight_O1_in;
	ENG_ADDR_REG weight_O1_in_msb;
	ENG_ADDR_REG weight_O2_in;
	ENG_ADDR_REG weight_O2_in_msb;
	ENG_ADDR_REG weight_O3_in;
	ENG_ADDR_REG weight_O3_in_msb;
	ENG_ADDR_REG bias_in;
	ENG_ADDR_REG bias_in_msb;
	ENG_ADDR_REG bn_in;
	ENG_ADDR_REG bn_in_msb;
	ENG_ADDR_REG act_weight_in;
	ENG_ADDR_REG act_weight_in_msb;
	ENG_ADDR_REG act_LUT_in;
	ENG_ADDR_REG act_LUT_in_msb;
	ENG_ADDR_REG DW_weight_in;
	ENG_ADDR_REG DW_weight_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O0_in;
	ENG_ADDR_REG VLC_bitstram1_O0_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O1_in;
	ENG_ADDR_REG VLC_bitstram1_O1_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O2_in;
	ENG_ADDR_REG VLC_bitstram1_O2_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O3_in;
	ENG_ADDR_REG VLC_bitstram1_O3_in_msb;
	ENG_ADDR_REG kmeans_in;
	ENG_ADDR_REG kmeans_in_msb;
	ENG_ADDR_REG preIn0_in;
	ENG_ADDR_REG preIn0_in_msb;
	ENG_ADDR_REG output;
	ENG_ADDR_REG output_msb;
	ENG_ADDR_REG output1;
	ENG_ADDR_REG output1_msb;
	// input size
	CONV_SIZE0_REG size0; // in0 width height
	CONV_SIZE1_REG size1; // in0 channel batch
	CONV_SIZE0_REG size2; // prein0 width height
	CONV_SIZE1_REG size3; // prein0 channel batch
} CONV_LL_PARM;
typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned in0_los_en : 1;
		unsigned in0_chofs_en : 1;
		unsigned in0_bofs_en : 1;
		unsigned : 1;
		unsigned in1_los_en : 1;
		unsigned in1_chofs_en : 1;
		unsigned in1_bofs_en : 1;
		unsigned : 1;
		unsigned inch_los_en : 1;
		unsigned inch_chofs_en : 1;
		unsigned inch_bofs_en : 1;
		unsigned : 1;
		unsigned w_los_en : 1;
		unsigned w_chofs_en : 1;
		unsigned w_bofs_en : 1;
		unsigned w_dofs_en : 1;
		unsigned out0_los_en : 1;
		unsigned out0_chofs_en : 1;
		unsigned out0_bofs_en : 1;
		unsigned : 1;
		unsigned out1_los_en : 1;
		unsigned out1_chofs_en : 1;
		unsigned out1_bofs_en : 1;

	} bit;
} CONV_OFFSET_EN_REG;
typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned ofs : 29;
	} bit;
} CONV_LOFS_REG;
typedef struct _CONV_LL_PARM_WITH_WIDTH_STRIDE {
	// addr
	ENG_ADDR_REG input;
	ENG_ADDR_REG input_msb;
	ENG_ADDR_REG elt_in;
	ENG_ADDR_REG elt_in_msb;
	ENG_ADDR_REG inter_in;
	ENG_ADDR_REG inter_in_msb;
	ENG_ADDR_REG weight_O0_in;
	ENG_ADDR_REG weight_O0_in_msb;
	ENG_ADDR_REG weight_O1_in;
	ENG_ADDR_REG weight_O1_in_msb;
	ENG_ADDR_REG weight_O2_in;
	ENG_ADDR_REG weight_O2_in_msb;
	ENG_ADDR_REG weight_O3_in;
	ENG_ADDR_REG weight_O3_in_msb;
	ENG_ADDR_REG bias_in;
	ENG_ADDR_REG bias_in_msb;
	ENG_ADDR_REG bn_in;
	ENG_ADDR_REG bn_in_msb;
	ENG_ADDR_REG act_weight_in;
	ENG_ADDR_REG act_weight_in_msb;
	ENG_ADDR_REG act_LUT_in;
	ENG_ADDR_REG act_LUT_in_msb;
	ENG_ADDR_REG DW_weight_in;
	ENG_ADDR_REG DW_weight_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O0_in;
	ENG_ADDR_REG VLC_bitstram1_O0_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O1_in;
	ENG_ADDR_REG VLC_bitstram1_O1_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O2_in;
	ENG_ADDR_REG VLC_bitstram1_O2_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O3_in;
	ENG_ADDR_REG VLC_bitstram1_O3_in_msb;
	ENG_ADDR_REG kmeans_in;
	ENG_ADDR_REG kmeans_in_msb;
	ENG_ADDR_REG preIn0_in;
	ENG_ADDR_REG preIn0_in_msb;
	ENG_ADDR_REG output;
	ENG_ADDR_REG output_msb;
	ENG_ADDR_REG output1;
	ENG_ADDR_REG output1_msb;
	// input size
	CONV_SIZE0_REG size0; // in0 width height
	CONV_SIZE1_REG size1; // in0 channel batch
	CONV_SIZE0_REG size2; // prein0 width height
	CONV_SIZE1_REG size3; // prein0 channel batch
	// offsetEn
	CONV_OFFSET_EN_REG offset_en;
	// in_offset
	CONV_LOFS_REG in0_lofs[3];
	CONV_LOFS_REG in1_lofs[3];
	CONV_LOFS_REG w_lofs[4]; // los dofs chofs bofs
} CONV_LL_PARM_WITH_WIDTH_STRIDE;
#endif
#if defined(_BSP_NS02401_) 
typedef struct _CONV_LL_PARM {
	// addr
	ENG_ADDR_REG input;
	ENG_ADDR_REG input_msb;
	ENG_ADDR_REG elt_in;
	ENG_ADDR_REG elt_in_msb;
	ENG_ADDR_REG inter_in;
	ENG_ADDR_REG inter_in_msb;
	ENG_ADDR_REG weight_O0_in;
	ENG_ADDR_REG weight_O0_in_msb;
	ENG_ADDR_REG weight_O1_in;
	ENG_ADDR_REG weight_O1_in_msb;
	ENG_ADDR_REG weight_O2_in;
	ENG_ADDR_REG weight_O2_in_msb;
	ENG_ADDR_REG weight_O3_in;
	ENG_ADDR_REG weight_O3_in_msb;
	ENG_ADDR_REG bias_in;
	ENG_ADDR_REG bias_in_msb;
	ENG_ADDR_REG bn_in;
	ENG_ADDR_REG bn_in_msb;
	ENG_ADDR_REG act_weight_in;
	ENG_ADDR_REG act_weight_in_msb;
	ENG_ADDR_REG act_LUT_in;
	ENG_ADDR_REG act_LUT_in_msb;
	ENG_ADDR_REG DW_weight_in;
	ENG_ADDR_REG DW_weight_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O0_in;
	ENG_ADDR_REG VLC_bitstram1_O0_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O1_in;
	ENG_ADDR_REG VLC_bitstram1_O1_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O2_in;
	ENG_ADDR_REG VLC_bitstram1_O2_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O3_in;
	ENG_ADDR_REG VLC_bitstram1_O3_in_msb;
	ENG_ADDR_REG kmeans_in;
	ENG_ADDR_REG kmeans_in_msb;
	ENG_ADDR_REG preIn0_in;
	ENG_ADDR_REG preIn0_in_msb;
	ENG_ADDR_REG output;
	ENG_ADDR_REG output_msb;
	ENG_ADDR_REG output1;
	ENG_ADDR_REG output1_msb;
	// input size
	CONV_SIZE0_REG size0; // in0 width height
	CONV_SIZE1_REG size1; // in0 channel batch
	CONV_SIZE0_REG size2; // prein0 width height
	CONV_SIZE1_REG size3; // prein0 channel batch
} CONV_LL_PARM;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned in0_los_en : 1;
		unsigned in0_chofs_en : 1;
		unsigned in0_bofs_en : 1;
		unsigned : 1;
		unsigned in1_los_en : 1;
		unsigned in1_chofs_en : 1;
		unsigned in1_bofs_en : 1;
		unsigned : 1;
		unsigned inch_los_en : 1;
		unsigned inch_chofs_en : 1;
		unsigned inch_bofs_en : 1;
		unsigned : 1;
		unsigned w_los_en : 1;
		unsigned w_chofs_en : 1;
		unsigned w_bofs_en : 1;
		unsigned w_dofs_en : 1;
		unsigned out0_los_en : 1;
		unsigned out0_chofs_en : 1;
		unsigned out0_bofs_en : 1;
		unsigned : 1;
		unsigned out1_los_en : 1;
		unsigned out1_chofs_en : 1;
		unsigned out1_bofs_en : 1;

	} bit;
} CONV_OFFSET_EN_REG;
typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned ofs : 29;
	} bit;
} CONV_LOFS_REG;
typedef struct _CONV_LL_PARM_WITH_WIDTH_STRIDE {
	// addr
	ENG_ADDR_REG input;
	ENG_ADDR_REG input_msb;
	ENG_ADDR_REG elt_in;
	ENG_ADDR_REG elt_in_msb;
	ENG_ADDR_REG inter_in;
	ENG_ADDR_REG inter_in_msb;
	ENG_ADDR_REG weight_O0_in;
	ENG_ADDR_REG weight_O0_in_msb;
	ENG_ADDR_REG weight_O1_in;
	ENG_ADDR_REG weight_O1_in_msb;
	ENG_ADDR_REG weight_O2_in;
	ENG_ADDR_REG weight_O2_in_msb;
	ENG_ADDR_REG weight_O3_in;
	ENG_ADDR_REG weight_O3_in_msb;
	ENG_ADDR_REG bias_in;
	ENG_ADDR_REG bias_in_msb;
	ENG_ADDR_REG bn_in;
	ENG_ADDR_REG bn_in_msb;
	ENG_ADDR_REG act_weight_in;
	ENG_ADDR_REG act_weight_in_msb;
	ENG_ADDR_REG act_LUT_in;
	ENG_ADDR_REG act_LUT_in_msb;
	ENG_ADDR_REG DW_weight_in;
	ENG_ADDR_REG DW_weight_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O0_in;
	ENG_ADDR_REG VLC_bitstram1_O0_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O1_in;
	ENG_ADDR_REG VLC_bitstram1_O1_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O2_in;
	ENG_ADDR_REG VLC_bitstram1_O2_in_msb;
	ENG_ADDR_REG VLC_bitstram1_O3_in;
	ENG_ADDR_REG VLC_bitstram1_O3_in_msb;
	ENG_ADDR_REG kmeans_in;
	ENG_ADDR_REG kmeans_in_msb;
	ENG_ADDR_REG preIn0_in;
	ENG_ADDR_REG preIn0_in_msb;
	ENG_ADDR_REG output;
	ENG_ADDR_REG output_msb;
	ENG_ADDR_REG output1;
	ENG_ADDR_REG output1_msb;
	// input size
	CONV_SIZE0_REG size0; // in0 width height
	CONV_SIZE1_REG size1; // in0 channel batch
	CONV_SIZE0_REG size2; // prein0 width height
	CONV_SIZE1_REG size3; // prein0 channel batch
	// offsetEn
	CONV_OFFSET_EN_REG offset_en;
	// in_offset
	CONV_LOFS_REG in0_lofs[3];
	CONV_LOFS_REG in1_lofs[3];
	CONV_LOFS_REG w_lofs[4]; // los dofs chofs bofs
} CONV_LL_PARM_WITH_WIDTH_STRIDE;
#endif

typedef struct _CONV_LL_DW_PARM {
	// addr
	ENG_ADDR_REG input;
	ENG_ADDR_REG elt_in;
	ENG_ADDR_REG inter_in;
	ENG_ADDR_REG weight_O0_in;
	ENG_ADDR_REG weight_O1_in;
	ENG_ADDR_REG weight_O2_in;
	ENG_ADDR_REG weight_O3_in;
	ENG_ADDR_REG bias_in;
	ENG_ADDR_REG bias_in_msb;
	ENG_ADDR_REG bn_in;
	ENG_ADDR_REG bn_in_msb;
	ENG_ADDR_REG act_weight_in;
	ENG_ADDR_REG act_LUT_in;
	ENG_ADDR_REG DW_weight_in;
	ENG_ADDR_REG VLC_bitstram1_O0_in;
	ENG_ADDR_REG VLC_bitstram1_O1_in;
	ENG_ADDR_REG VLC_bitstram1_O2_in;
	ENG_ADDR_REG VLC_bitstram1_O3_in;
	ENG_ADDR_REG kmeans_in;
	ENG_ADDR_REG preIn0_in;
	ENG_ADDR_REG output;
	// input size
	CONV_SIZE0_REG size0; // in0 width height
	CONV_SIZE1_REG size1; // in0 channel batch
	CONV_SIZE0_REG size2; // prein0 width height
	CONV_SIZE1_REG size3; // prein0 channel batch
} CONV_LL_DW_PARM; // for depthwise shrink llcmd

typedef struct _ROU_LL_PARM {
	// addr
	ENG_ADDR_REG input;
	ENG_ADDR_REG input_msb;
	ENG_ADDR_REG refer_in;
	ENG_ADDR_REG refer_in_msb;
	ENG_ADDR_REG norm_mean;
	ENG_ADDR_REG norm_mean_msb;
	ENG_ADDR_REG output;
	ENG_ADDR_REG output_msb;
	// input size
	ROU_SIZE0_REG size0; // in0 width
	ROU_SIZE1_REG size1; // in0 height
	ROU_SIZE2_REG size2; // in0 channel
	ROU_SIZE0_REG size3; // out0 width
	ROU_SIZE1_REG size4; // out0 height
	ROU_SIZE2_REG size5; // out0 channel
} ROU_LL_PARM;

typedef struct _ROU_LL_DW_PARM {
	// addr
	ENG_ADDR_REG input;
	ENG_ADDR_REG refer_in;
	ENG_ADDR_REG norm_mean;
	ENG_ADDR_REG output;
	// input size
	ROU_SIZE0_REG size0; // in0 width
	ROU_SIZE1_REG size1; // in0 height
	ROU_SIZE2_REG size2; // in0 channel
	ROU_SIZE0_REG size3; // out0 width
	ROU_SIZE1_REG size4; // out0 height
	ROU_SIZE2_REG size5; // out0 channel
} ROU_LL_DW_PARM;

typedef struct _UTIL_LL_PARM {
	// addr
	ENG_ADDR_REG input;
	ENG_ADDR_REG input_msb;
	ENG_ADDR_REG elt_in;
	ENG_ADDR_REG elt_in_msb;
	ENG_ADDR_REG pool_mask_in;
	ENG_ADDR_REG pool_mask_in_msb;
	ENG_ADDR_REG act_weight_in;
	ENG_ADDR_REG act_weight_in_msb;
	ENG_ADDR_REG act_LUT_in;
	ENG_ADDR_REG act_LUT_in_msb;
	ENG_ADDR_REG output0;
	ENG_ADDR_REG output0_msb;
	ENG_ADDR_REG output1;
	ENG_ADDR_REG output1_msb;
	// input size
	UTIL_SIZE0_REG size0; // in0 width height
	UTIL_SIZE1_REG size1; // in0 channel
} UTIL_LL_PARM;


typedef struct _NUE2_LL_PARM {
	ENG_ADDR_REG input[3];
	ENG_ADDR_REG input_msb[3];
	ENG_ADDR_REG output[3];
	ENG_ADDR_REG output_msb[3];
	NUE2_SIZE0_REG size0;
	NUE2_LOFS_REG ilofs[3];
	NUE2_LOFS_REG olofs[3];
	NUE2_SCL0_REG scale0;
	NUE2_SCL1_REG scale1;
	NUE2_SCL_SIZE_REG scale_size;
	NUE2_FUNC_EN_REG func_en;
} NUE2_LL_PARM;

typedef struct _PPU_LL_PARM {
	ENG_ADDR_REG input0;
	ENG_ADDR_REG input0_msb;
	ENG_ADDR_REG input1;
	ENG_ADDR_REG input1_msb;
	ENG_ADDR_REG input2;
	ENG_ADDR_REG input2_msb;
	ENG_ADDR_REG output0;
	ENG_ADDR_REG output0_msb;
	ENG_ADDR_REG output;
	ENG_ADDR_REG output_msb;
	ENG_ADDR_REG output2;
	ENG_ADDR_REG output2_msb;
	PPU_SIZE0_REG size0;
	PPU_SIZE1_REG size1;
} PPU_LL_PARM;

typedef struct _CAL_LL_PARM {
	ENG_ADDR_REG input0;
	ENG_ADDR_REG input0_msb;
	ENG_ADDR_REG input1;
	ENG_ADDR_REG input1_msb;
	ENG_ADDR_REG input2;
	ENG_ADDR_REG input2_msb;
	ENG_ADDR_REG output0;
	ENG_ADDR_REG output0_msb;
	ENG_ADDR_REG output1;
	ENG_ADDR_REG output1_msb;
	CAL_SIZE0_REG in0_width;
	CAL_SIZE1_REG in0_height;
	CAL_SIZE2_REG in0_channel;
	CAL_SIZE0_REG in1_width;
	CAL_SIZE1_REG in1_height;
	CAL_SIZE2_REG in1_channel;
	CAL_SIZE0_REG in2_width;
	CAL_SIZE1_REG in2_height;
	CAL_SIZE2_REG in2_channel;
	CAL_SIZE0_REG group0_in0_width;
	CAL_SIZE1_REG group0_in0_height;
	CAL_SIZE2_REG group0_in0_channel;
	CAL_SIZE0_REG group1_in0_width;
	CAL_SIZE1_REG group1_in0_height;
	CAL_SIZE2_REG group1_in0_channel;
	CAL_SIZE0_REG group2_in0_width;
	CAL_SIZE1_REG group2_in0_height;
	CAL_SIZE2_REG group2_in0_channel;
	CAL_SIZE0_REG group3_in0_width;
	CAL_SIZE1_REG group3_in0_height;
	CAL_SIZE2_REG group3_in0_channel;
	CAL_SIZE0_REG group0_in1_width;
	CAL_SIZE1_REG group0_in1_height;
	CAL_SIZE2_REG group0_in1_channel;
} CAL_LL_PARM;

typedef struct _LSU_LL_PARM {
	ENG_ADDR_REG input;
	ENG_ADDR_REG input_msb;
	ENG_ADDR_REG output;
	ENG_ADDR_REG output_msb;
	LSU_SIZE0_REG size0;
	LSU_SIZE1_REG size1;
	LSU_SIZE2_REG size2;
	LSU_SIZE3_REG size3;
	LSU_OFFSET_EN_REG offset_en;
	LSU_LOFS_REG ilofs[3];
	LSU_LOFS_REG olofs[3];
} LSU_LL_PARM;

#endif

/********************************************************************
    EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
//int nn_write_file(char *filename, void *data, size_t count, unsigned long long *offset);
//int nn_read_file(char *filename, void *data, size_t count, unsigned long long *offset);
//UINT32 get_mid_val(UINT32 *p_times, UINT32 length);
extern uintptr_t nvt_ai_user2user_va_in_kerl(uintptr_t addr, UINT32 net_id);
extern UINT32 kflow_ai_get_net_supported_num(VOID);

#endif  /* _NET_COMMON_H_ */
