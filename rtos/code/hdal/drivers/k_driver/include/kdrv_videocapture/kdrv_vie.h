/**
    Public header file for kdrv_vie

    This file is the header file that define the API and data type for kdrv_vie.

    @file      kdrv_vie.h
    @ingroup    mILibIPLCom
    @note      Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2009.  All rights reserved.
*/
#ifndef _KDRV_VIE_H_
#define _KDRV_VIE_H_

#include "kwrap/type.h"
#include "kdrv_type.h"

#define KDRV_VIE_READY DISABLE
#define SSDRV_VIE_READY DISABLE

#define KDRV_VIE_MAX_ENG        2 // VIE_ENGINE_ID_MAX   //maximum support engine
#define KDRV_VIE_MAX_VDO_CH     4 // VIE_CHANNEL_NUM     //maximum support input video channel for each engine

#define KDRV_VIE_VAR2STR(x)   (#x)
#define KDRV_VIE_DUMPD(x)     DBG_DUMP("%s = %d\r\n", KDRV_VIE_VAR2STR(x), x)
#define KDRV_VIE_DUMPH(x)     DBG_DUMP("%s = 0x%.8x\r\n", KDRV_VIE_VAR2STR(x), x)

#define KDRV_VIE_DBG_DRAMOUT_SZ (4 * 14) // units : bytes, 32bits * 14, per input channel


/**
    vie support function.
*/
typedef enum {
	KDRV_VIE_FUNC_SPT_NONE          = 0x00000000, // none
	KDRV_VIE_FUNC_SPT_DVI           = 0x00000001, // dvi
	KDRV_VIE_FUNC_SPT_FLIP_H        = 0x00000002, // horizontal flip
	KDRV_VIE_FUNC_SPT_FLIP_V        = 0x00000004, // vertical flip
	KDRV_VIE_FUNC_SPT_YCCENC        = 0x00000008, // ycc encode
	KDRV_VIE_FUNC_SPT_SINGLE_OUT    = 0x00000010, // single out
	KDRV_VIE_FUNC_SPT_PATGEN        = 0x00000020, // patgen
	KDRV_VIE_FUNC_SPT_MASK			= 0x00000040, // mask
} KDRV_VIE_FUNC_SUPPORT;

/**
    vie clk source select
*/
typedef enum {
	KDRV_VIE_CLKSRC_CURR = 0,   ///< VIE clock source/rate keep previous setting
	KDRV_VIE_CLKSRC_192,        ///< VIE clock source as 192MHz
	KDRV_VIE_CLKSRC_320,        ///< VIE clock source as 320MHz
	KDRV_VIE_CLKSRC_480,        ///< VIE clock source as 480MHz
	KDRV_VIE_CLKSRC_PLL19,      ///< VIE clock source as PLL19
	KDRV_VIE_CLKSRC_PLL23,      ///< VIE clock source as PLL23
	KDRV_VIE_CLKSRC_MAX,

	ENUM_DUMMY4WORD(KDRV_VIE_CLKSRC_SEL)
} KDRV_VIE_CLKSRC_SEL;

typedef struct {
	UINT32 rate;
	KDRV_VIE_CLKSRC_SEL clk_src_sel;        ///< VIE clock source selection
} KDRV_VIE_CLK_INFO;

/**
    vie pclk source select
*/
typedef enum {
	KDRV_VIE_PXCLKSRC_OFF,      ///< VIE pixel-clock disable
	KDRV_VIE_PXCLKSRC_PAD_A,    ///< VIE pixel-clock enabled, source as pixel-clock-pad-A
	KDRV_VIE_PXCLKSRC_PAD_B,	///< VIE pixel-clock enabled, source as pixel-clock-pad-B
	KDRV_VIE_PXCLKSRC_PAD_AB,	///< VIE pixel-clock enabled, source as pixel-clock-pad-A&B

	ENUM_DUMMY4WORD(KDRV_VIE_PXCLKSRC_SEL)
} KDRV_VIE_PXCLKSRC_SEL;


typedef struct {
	KDRV_VIE_CLK_INFO       clk_info;
} KDRV_VIE_CLK_HDL;

//@}

/**
    YUV channel index
*/
//@{
typedef enum {
	KDRV_VIE_YUV_Y = 0, ///< process channel Y
	KDRV_VIE_YUV_U,     ///< process channel U
	KDRV_VIE_YUV_V,     ///< process channel V
	KDRV_VIE_YUV_MAX_CH,
	ENUM_DUMMY4WORD(KDRV_VIE_YUV_CH)
} KDRV_VIE_YUV_CH;


/**
    VIE flip information
*/
typedef enum {
	KDRV_VIE_FLIP_NONE =    0x00000000,      ///< no flip
	KDRV_VIE_FLIP_H =       0x00000001,      ///< H flip
	KDRV_VIE_FLIP_V =       0x00000002,      ///< V flip
	KDRV_VIE_FLIP_H_V  =    0x00000003,      ///< H & V flip(Flip_H | Flip_V) don't modify
	KDRV_VIE_FLIP_MAX,
	ENUM_DUMMY4WORD(KDRV_VIE_FLIP)
} KDRV_VIE_FLIP;

/**
    VIE YUV Image type
*/
typedef enum {
	KDRV_VIE_YUV444 = 0,    ///< 3 channel, yuv 444 planar
	KDRV_VIE_YUV422,        ///< 3 channel, yuv 422 planar
	KDRV_VIE_YUV420,        ///< 3 channel, yuv 420 planar
	KDRV_VIE_Y_PACK_UV444,  ///< 2 channel, y planar uv pack, 444 format
	KDRV_VIE_Y_PACK_UV422,  ///< 2 channel, y planar uv pack, 422 format
	KDRV_VIE_Y_PACK_UV420,  ///< 2 channel, y planar uv pack, 420 format
	KDRV_VIE_PACK_YUV444,   ///< 1 channel, yuv pack, 444 format
	KDRV_VIE_Y_ONLY,        ///< 1 channel, y only
	ENUM_DUMMY4WORD(KDRV_VIE_YUV_TYPE)
} KDRV_VIE_YUV_TYPE;

/**
    VIE process ID
*/
typedef enum {
	KDRV_VIE_ID_1 = 0,      ///< process id 1
	KDRV_VIE_ID_2,          ///< process id 2
	KDRV_VIE_ID_MAX_NUM = KDRV_VIE_MAX_ENG,
	ENUM_DUMMY4WORD(KDRV_VIE_PROC_ID)
} KDRV_VIE_PROC_ID;

typedef enum {
	KDRV_VIE_VDO_CH0,
	KDRV_VIE_VDO_CH1,
	KDRV_VIE_VDO_CH2,
	KDRV_VIE_VDO_CH3,
	KDRV_VIE_VDO_CH_MAX_NUM = KDRV_VIE_MAX_VDO_CH,
	ENUM_DUMMY4WORD(KDRV_VIE_VDO_CH)
} KDRV_VIE_VDO_CH;

/**
    vie signal receive mode
*/
typedef enum {
	KDRV_VIE_IN_PARA_MSTR_SNR = 0,  ///< Parallel Master Sensor
	KDRV_VIE_IN_PATGEN,             ///< Self Pattern-Generator
	KDRV_VIE_IN_CSI_1,              ///< Serial Sensor from CSI-1
	KDRV_VIE_IN_CSI_2,              ///< Serial Sensor from CSI-2
	KDRV_VIE_IN_CSI_3,              ///< Serial Sensor from CSI-3
	KDRV_VIE_IN_CSI_4,              ///< Serial Sensor from CSI-4
	KDRV_VIE_IN_CSI_5,              ///< Serial Sensor from CSI-5
	KDRV_VIE_IN_CSI_6,              ///< Serial Sensor from CSI-6
	KDRV_VIE_IN_CSI_7,              ///< Serial Sensor from CSI-7
	KDRV_VIE_IN_CSI_8,              ///< Serial Sensor from CSI-8

	KDRV_VIE_IN_MAX,
	ENUM_DUMMY4WORD(KDRV_VIE_ACT_MODE)
} KDRV_VIE_ACT_MODE;

/**
    vie kdrv interrupt type
*/
typedef enum {
	KDRV_VIE_INT_CLR                = 0,

	KDRV_VIE_INT_CH0_VD             = 0x00000001UL,   ///< interrupt: input video ch0 VD
	KDRV_VIE_INT_CH1_VD             = 0x00000002UL,   ///< interrupt: input video ch1 VD
	KDRV_VIE_INT_CH2_VD             = 0x00000004UL,   ///< interrupt: input video ch2 VD
	KDRV_VIE_INT_CH3_VD             = 0x00000008UL,   ///< interrupt: input video ch3 VD

	KDRV_VIE_INT_CH0_BP             = 0x00000010UL,   ///< interrupt: input video ch0 BP
	KDRV_VIE_INT_CH1_BP             = 0x00000020UL,   ///< interrupt: input video ch1 BP
	KDRV_VIE_INT_CH2_BP             = 0x00000040UL,   ///< interrupt: input video ch2 BP
	KDRV_VIE_INT_CH3_BP             = 0x00000080UL,   ///< interrupt: input video ch3 BP

	KDRV_VIE_INT_CH0_DRAM_OUT0_END  = 0x00000100UL,   ///< interrupt: input video ch0 dram output channel 0 end
	KDRV_VIE_INT_CH1_DRAM_OUT0_END  = 0x00000200UL,   ///< interrupt: input video ch1 dram output channel 0 end
	KDRV_VIE_INT_CH2_DRAM_OUT0_END  = 0x00000400UL,   ///< interrupt: input video ch2 dram output channel 0 end
	KDRV_VIE_INT_CH3_DRAM_OUT0_END  = 0x00000800UL,   ///< interrupt: input video ch3 dram output channel 0 end

	KDRV_VIE_INT_CH0_DRAM_OUT1_END  = 0x00001000UL,   ///< interrupt: input video ch0 dram output channel 1 end
	KDRV_VIE_INT_CH1_DRAM_OUT1_END  = 0x00002000UL,   ///< interrupt: input video ch1 dram output channel 1 end
	KDRV_VIE_INT_CH2_DRAM_OUT1_END  = 0x00004000UL,   ///< interrupt: input video ch2 dram output channel 1 end
	KDRV_VIE_INT_CH3_DRAM_OUT1_END  = 0x00008000UL,   ///< interrupt: input video ch3 dram output channel 1 end

	KDRV_VIE_INT_CH0_CRPST          = 0x00010000UL,   ///< interrupt: input video ch0 crop window start
	KDRV_VIE_INT_CH1_CRPST          = 0x00020000UL,   ///< interrupt: input video ch1 crop window start
	KDRV_VIE_INT_CH2_CRPST          = 0x00040000UL,   ///< interrupt: input video ch2 crop window start
	KDRV_VIE_INT_CH3_CRPST          = 0x00080000UL,   ///< interrupt: input video ch3 crop window start

	KDRV_VIE_INT_CH0_CROPEND        = 0x00100000UL,   ///< interrupt: input video ch0 crop window end
	KDRV_VIE_INT_CH1_CROPEND        = 0x00200000UL,   ///< interrupt: input video ch1 crop window end
	KDRV_VIE_INT_CH2_CROPEND        = 0x00400000UL,   ///< interrupt: input video ch2 crop window end
	KDRV_VIE_INT_CH3_CROPEND        = 0x00800000UL,   ///< interrupt: input video ch3 crop window end

	KDRV_VIE_INT_CH0_ERR_YCCOVFL    = 0x01000000UL,   ///< interrupt: input video ch0 ycc overflow
	KDRV_VIE_INT_CH1_ERR_YCCOVFL    = 0x02000000UL,   ///< interrupt: input video ch1 ycc overflow
	KDRV_VIE_INT_CH2_ERR_YCCOVFL    = 0x04000000UL,   ///< interrupt: input video ch2 ycc overflow
	KDRV_VIE_INT_CH3_ERR_YCCOVFL    = 0x08000000UL,   ///< interrupt: input video ch3 ycc overflow

	KDRV_VIE_INT_DRAM_DBG_END       = 0x10000000UL,   ///< interrupt: debug information output
	KDRV_VIE_INT_ERR_LINEBUF_OVFL   = 0x20000000UL,   ///< interrupt: line buffer overflow
	KDRV_VIE_INT_CCIR656_HEAD_HIT   = 0x40000000UL,   ///< interrupt: ccir656 header hit

	KDRV_VIE_INT_ALL                = 0xffffffffUL,   ///< interrupt: all

} KDRV_VIE_INT;

#define KDRV_VIE_INT_CH0 (KDRV_VIE_INT_CH0_VD|KDRV_VIE_INT_CH0_BP|KDRV_VIE_INT_CH0_DRAM_OUT0_END|KDRV_VIE_INT_CH0_DRAM_OUT1_END|KDRV_VIE_INT_CH0_CRPST|KDRV_VIE_INT_CH0_CROPEND|KDRV_VIE_INT_CH0_ERR_YCCOVFL)
#define KDRV_VIE_INT_CH1 (KDRV_VIE_INT_CH1_VD|KDRV_VIE_INT_CH1_BP|KDRV_VIE_INT_CH1_DRAM_OUT0_END|KDRV_VIE_INT_CH1_DRAM_OUT1_END|KDRV_VIE_INT_CH1_CRPST|KDRV_VIE_INT_CH1_CROPEND|KDRV_VIE_INT_CH1_ERR_YCCOVFL)
#define KDRV_VIE_INT_CH2 (KDRV_VIE_INT_CH2_VD|KDRV_VIE_INT_CH2_BP|KDRV_VIE_INT_CH2_DRAM_OUT0_END|KDRV_VIE_INT_CH2_DRAM_OUT1_END|KDRV_VIE_INT_CH2_CRPST|KDRV_VIE_INT_CH2_CROPEND|KDRV_VIE_INT_CH2_ERR_YCCOVFL)
#define KDRV_VIE_INT_CH3 (KDRV_VIE_INT_CH3_VD|KDRV_VIE_INT_CH3_BP|KDRV_VIE_INT_CH3_DRAM_OUT0_END|KDRV_VIE_INT_CH3_DRAM_OUT1_END|KDRV_VIE_INT_CH3_CRPST|KDRV_VIE_INT_CH3_CROPEND|KDRV_VIE_INT_CH3_ERR_YCCOVFL)

#define KDRV_VIE_INT_VD (KDRV_VIE_INT_CH0_VD|KDRV_VIE_INT_CH1_VD|KDRV_VIE_INT_CH2_VD|KDRV_VIE_INT_CH3_VD)
#define KDRV_VIE_INT_CHK (KDRV_VIE_INT_CH0_ERR_YCCOVFL|KDRV_VIE_INT_CH1_ERR_YCCOVFL|KDRV_VIE_INT_CH2_ERR_YCCOVFL|KDRV_VIE_INT_CH3_ERR_YCCOVFL)

/**
    struct for kdrv_vie_open
*/
typedef struct {
	KDRV_VIE_CLKSRC_SEL     clk_src_sel;    // by engine, VIE clock source selection
	KDRV_VIE_PXCLKSRC_SEL   pclk_src_sel;   // by engine, VIE pixel clk source selection
	UINT32                  data_rate;      // by engine, Sensor output data rate(bytes per seconds)
} KDRV_VIE_OPENCFG;

/**
    vie kdrv trig type, start/stop vie
*/
typedef enum {
	KDRV_VIE_TRIG_STOP = 0, ///< Stop vie
	KDRV_VIE_TRIG_START,    ///< Start vie
	ENUM_DUMMY4WORD(KDRV_VIE_TRIG_TYPE)
} KDRV_VIE_TRIG_TYPE;

/**
    struct for kdrv_vie_open
*/
typedef struct {
	KDRV_VIE_TRIG_TYPE trig_type;   ///< trigger operation
	BOOL wait_end;                  ///< wait trigger end
} KDRV_VIE_TRIG_INFO;

/**
    struct for kdrv_vie_bp
*/
typedef struct {
	UINT32 bp3;
} KDRV_VIE_BP_INFO;

/*
    vie dram channel
*/
typedef enum  {
	KDRV_VIE_DRAM_CH0,
	KDRV_VIE_DRAM_CH1,
	KDRV_VIE_DRAM_CHDBG,
	KDRV_VIE_DRAM_CH_MAX,
	KDRV_VIE_DRAM_CH_ALL = 0xff,
	ENUM_DUMMY4WORD(KDRV_VIE_DRAM_CH)
} KDRV_VIE_DRAM_CH;

/**
    callback function prototype
    UINT32 id
    UINT32 msg_type
    void* in_data
    void* out_data
*/
typedef INT32(*KDRV_VIE_ISRCB)(UINT32, UINT32, void *, void *);

/**
    type for KDRV_VIE_DATA_FMT
    vie always output y to ch0, uv to ch1
    When CH0_YCC_EN/CH1_YCC_EN/..., the output split will be effective automatically.
*/
typedef enum {
	KDRV_VIE_YUV_422_NOSPT,
	KDRV_VIE_YUV_422_SPT,
	KDRV_VIE_YUV_422_SPT_YCCENC,

	KDRV_VIE_YUV_420_SPT,
	KDRV_VIE_YUV_420_SPT_YCCENC,

	KDRV_VIE_DATA_FMT_MAX,
} KDRV_VIE_DATA_FMT;

/**
    type for KDRV_VIE_SET_SIGNAL
*/
typedef enum {
	KDRV_VIE_PAD_SEL_OFF,
	KDRV_VIE_PAD_SEL_A,	// CCIR601 must select PAD_A, 16bit must select PAD_A
	KDRV_VIE_PAD_SEL_B,
	KDRV_VIE_PAD_SEL_AB,
	KDRV_VIE_PAD_SEL_MAX,
	ENUM_DUMMY4WORD(KDRV_VIE_PAD_SEL)
} KDRV_VIE_PAD_SEL;

typedef enum {
	KDRV_VIE_SMP_SINGLE_EDGE = 0,   // clock sampling with single edge of pxclk
	KDRV_VIE_SMP_DUAL_EDGE,         // clock sampling with dual edge of pxclk
	ENUM_DUMMY4WORD(KDRV_VIE_SMP_EDGE)
} KDRV_VIE_SMP_EDGE;

typedef enum {
	KDRV_VIE_PHASE_RISING = 0,
	KDRV_VIE_PHASE_FALLING,
	ENUM_DUMMY4WORD(KDRV_VIE_SIGNAL_PHASE)
} KDRV_VIE_SIGNAL_PHASE;

typedef enum {
	KDRV_VIE_SIGNAL_INV_OFF = 0, 	// signal no-inverse
	KDRV_VIE_SIGNAL_INV_ON,			// signal inverse
	ENUM_DUMMY4WORD(KDRV_VIE_SIGNAL_INV)
} KDRV_VIE_SIGNAL_INV;

typedef enum {
	KDRV_VIE_SIGNAL_SWAP_OFF = 0, 	// signal no-swap
	KDRV_VIE_SIGNAL_SWAP_ON,		// signal swap
	ENUM_DUMMY4WORD(KDRV_VIE_SIGNAL_SWAP)
} KDRV_VIE_SIGNAL_SWAP;

typedef enum {
	KDRV_VIE_DATA_SWITCH_MODE_NONE 		= 0x00UL,
	KDRV_VIE_DATA_SWITCH_MODE_A 		= 0x01UL, // HI_LO_BYTE_SWAP	, input = [15:8][7:0], the swap result = [7:0][15:8]
	KDRV_VIE_DATA_SWITCH_MODE_B 		= 0x02UL, // LO_BYTE_INV		, input = [15:8][7:0], the swap result = [15:8][0:7]
	KDRV_VIE_DATA_SWITCH_MODE_C 		= 0x04UL, // HI_BYTE_INV		, input = [15:8][7:0], the swap result = [8:15][7:0]
	KDRV_VIE_DATA_SWITCH_MODE_D 		= 0x08UL, // LO_4BIT_SWAP		, input = [15:12][11:8][7:4][3:0], the swap result = [15:12][11:8][3:0][7:4]
	KDRV_VIE_DATA_SWITCH_MODE_E 		= 0x10UL, // HI_4BIT_SWAP		, input = [15:12][11:8][7:4][3:0], the swap result = [11:8][15:12][7:4][3:0]
	ENUM_DUMMY4WORD(KDRV_VIE_DATA_SWITCH_MODE)
} KDRV_VIE_DATA_SWITCH_MODE;

typedef struct {
	/* latch signal egde sel */
	KDRV_VIE_SMP_EDGE       smp_edge;   	// by engine & KDRV_VIE_PAD_SEL
	KDRV_VIE_SIGNAL_SWAP	swap_edge;		// by engine & KDRV_VIE_PAD_SEL

	/* latch signal phase sel */
	KDRV_VIE_SIGNAL_PHASE   vd_phase;   	// by engine & KDRV_VIE_PAD_SEL, only valid when smp_edge=KDRV_VIE_SMP_SINGLE_EDGE, support CCIR601 (530 only PAD_A support CCIR601)
	KDRV_VIE_SIGNAL_PHASE   hd_phase;   	// by engine & KDRV_VIE_PAD_SEL, only valid when smp_edge=KDRV_VIE_SMP_SINGLE_EDGE, support CCIR601 (530 only PAD_A support CCIR601)
	KDRV_VIE_SIGNAL_PHASE   data_phase; 	// by engine & KDRV_VIE_PAD_SEL, only valid when smp_edge=KDRV_VIE_SMP_SINGLE_EDGE, support CCIR601 & CCIR656
	KDRV_VIE_SIGNAL_PHASE   filed_phase; 	// by engine & KDRV_VIE_PAD_SEL, only valid when smp_edge=KDRV_VIE_SMP_SINGLE_EDGE, support CCIR601 (530 only PAD_A support CCIR601)

	/* latch signal inverse sel */
	KDRV_VIE_SIGNAL_INV		vd_inv;   		// by engine & KDRV_VIE_PAD_SEL, support CCIR601 (530 only PAD_A support CCIR601)
	KDRV_VIE_SIGNAL_INV		hd_inv;   		// by engine & KDRV_VIE_PAD_SEL, support CCIR601 (530 only PAD_A support CCIR601)
	KDRV_VIE_SIGNAL_INV		filed_inv;   	// by engine & KDRV_VIE_PAD_SEL, support CCIR601 (530 only PAD_A support CCIR601)

	/* data switch sel */
	KDRV_VIE_DATA_SWITCH_MODE data_switch_mode;

} KDRV_VIE_PAD_INFO;

typedef struct {
	KDRV_VIE_ACT_MODE	act_mode;	// by engine
	KDRV_VIE_PAD_SEL    pad_sel;    // by channel. vie pad selection
	KDRV_VIE_PAD_INFO	pad_info; 	// by engine & KDRV_VIE_PAD_SEL. pad information.
} KDRV_VIE_SIGNAL;

/*
    pattern gen
*/
typedef enum {
	KDRV_VIE_PAT_REV = 1,      	///< N.S.
	KDRV_VIE_PAT_RANDOM,    	///< Random
	KDRV_VIE_PAT_FIXED,         ///< Fixed
	KDRV_VIE_PAT_HINCREASE,     ///< 1D Increment
	KDRV_VIE_PAT_HVINCREASE,    ///< 2D increment
	ENUM_DUMMY4WORD(KDRV_VIE_PATGEN_SEL)
} KDRV_VIE_PATGEN_SEL;

typedef struct {
	KDRV_VIE_PATGEN_SEL mode;
	UINT32              val;
	USIZE               src_win;
} KDRV_VIE_PATGEN_INFO;

/**
    type for KDRV_VIE_SET_YUV_ORDER
*/
typedef enum {
	KDRV_VIE_YUYV = 0,
	KDRV_VIE_YVYU,
	KDRV_VIE_UYVY,
	KDRV_VIE_VYUY,
	KDRV_VIE_YYU_YYV,
	KDRV_VIE_YUY_YVY,
	KDRV_VIE_UYY_VYY,
	ENUM_DUMMY4WORD(KDRV_VIE_YUV_ORDER)
} KDRV_VIE_YUV_ORDER;

typedef enum {
	KDRV_VIE_DVI_FMT_CCIR601  = 0,    			// CCIR 601
	KDRV_VIE_DVI_FMT_CCIR656_EAV,     			// CCIR 656 EAV
	KDRV_VIE_DVI_FMT_CCIR656_EAV_DUAL_HEADER,	// CCIR 656 EAV with dual header
	KDRV_VIE_DVI_FMT_LEGACY_YUV420,
	ENUM_DUMMY4WORD(KDRV_VIE_DVI_FMT_SEL)
} KDRV_VIE_DVI_FMT_SEL;

typedef enum {
	KDRV_DVI_MODE_SD  = 0,   ///< SD mode (8 bits)
	KDRV_DVI_MODE_HD,        ///< HD mode (16bits)
	ENUM_DUMMY4WORD(KDRV_VIE_DVI_IN_MODE_SEL)
} KDRV_VIE_DVI_IN_MODE_SEL;


/**
    type for KDRV_VIE_SET_CCIR
*/
typedef struct {
	KDRV_VIE_YUV_ORDER yuv_order;
	KDRV_VIE_DVI_FMT_SEL fmt;
	KDRV_VIE_DVI_IN_MODE_SEL dvi_mode;
	BOOL filed_enable;              ///< for CCIR interlaced
	BOOL filed_sel;                 ///< select data for CCIR interlaced
	BOOL ccir656_vd_sel;			///< 656 vd mode, 0 --> interlaced(field change as VD), 1 --> progressive(SAV + V Blank status)
	UINT8 data_period;              ///< for mux sensor, 0 --> 1 input(YUYV), 1 --> 2 input(YYUUYYVV), only parallel sensor support, mipi sensor not support
	UINT8 data_idx;                 ///< select data idx 0/1 when mux sensor input
} KDRV_VIE_CCIR_INFO;

typedef struct {
	URECT win;
} KDRV_VIE_CRP_WIN;

typedef struct {
	BOOL singleout_enable[KDRV_VIE_DRAM_CH_MAX];
} KDRV_VIE_SINGLE_OUT_CTRL;

typedef enum {
	KDRV_VIE_NORMAL_OUT = 0,
	KDRV_VIE_SINGLE_OUT,
	ENUM_DUMMY4WORD(KDRV_VIE_OUTPUT_MODE_TYPE)
} KDRV_VIE_OUTPUT_MODE_TYPE;

typedef struct {
	KDRV_VIE_OUTPUT_MODE_TYPE  out_mode[KDRV_VIE_DRAM_CH_MAX];
} KDRV_VIE_DRAM_OUT_CTRL;

typedef enum {
	KDRV_VIE_SPT_YUV_422_NOSPT          = (1 << KDRV_VIE_YUV_422_NOSPT),
	KDRV_VIE_SPT_YUV_422_SPT            = (1 << KDRV_VIE_YUV_422_SPT),
	KDRV_VIE_SPT_YUV_422_SPT_YCCENC     = (1 << KDRV_VIE_YUV_422_SPT_YCCENC),

	KDRV_VIE_SPT_YUV_420_SPT            = (1 << KDRV_VIE_YUV_420_SPT),
	KDRV_VIE_SPT_YUV_420_SPT_YCCENC     = (1 << KDRV_VIE_YUV_420_SPT_YCCENC),

	KDRV_VIE_DATAFORMAT_SPT_ALL = (KDRV_VIE_SPT_YUV_422_NOSPT | KDRV_VIE_SPT_YUV_422_SPT | KDRV_VIE_SPT_YUV_422_SPT_YCCENC | KDRV_VIE_SPT_YUV_420_SPT | KDRV_VIE_SPT_YUV_420_SPT_YCCENC),

	ENUM_DUMMY4WORD(KDRV_VIE_DATAFORMAT_SPT)
} KDRV_VIE_DATAFORMAT_SPT;

/**
    VIE limitation infor
    sync to CTL_VIE_LIMIT
*/
typedef struct {
	UINT32 max_clk_rate;        // maximum vie clock rate
	UINT32 max_spt_id;          // [for ctl layer] maximum vie engine * vie input video channel
	UINT32 max_spt_egine;       // maximum vie engine
	UINT32 max_spt_vdo_ch;      // maximum vie input video channel, per vie engine
	UINT32 max_dramout_ch;      // maximum dram output channel
	USIZE pat_gen_src_win_align;
	URECT crp_win_align_nosplit;      // crop window limit with split=0 (ycc_encode=0)
	URECT crp_win_align_split;        // crop window limit with split=1 (ycc_encode=0)
	URECT crp_win_align_split_yccenc; // crop window limit with ycc_encode=1 (ycc_encode=1 only support split)
	USIZE crp_win_min;
	USIZE crp_win_max;
	USIZE crp_win_max_four_k;
	UINT32 out_lofs_align[KDRV_VIE_DRAM_CH_MAX];    //output lineoofset align, proposal[DRAM_OUT0_OFSO & DRAM_OUT1_OFSO & DRAM_OUT2_OFSO]
	UINT64 support_func;        //reference to KDRV_VIE_FUNC_SUPPORT or CTL_VIE_FUNC_SPT
	UINT32 support_output_fmt;  // KDRV_VIE_DATAFORMAT_SPT or CTL_VIE_DATAFORMAT_SPT
} KDRV_VIE_LIMIT;


/**
    type for KDRV_VIE_DMA_STS
*/
typedef enum {
	KDRV_VIE_DMA_STS_IDLE = 0,
	KDRV_VIE_DMA_STS_RUN = 1,
	ENUM_DUMMY4WORD(KDRV_VIE_DMA_STS)
} KDRV_VIE_DMA_STS;

/**
    KDRV_VIE_SYS_INFO
    vd_cnt_clk / vd_cnt_clk_base = vd time (s)
    hd_cnt_clk / hd_cnt_clk_base = hd time (s)
*/
typedef struct {
	BOOL pxclk_a_in;          	// check pixel clock in,  for parallel sensor debug only
	BOOL pxclk_b_in;          	// check pixel clock in,  for parallel sensor debug only
	BOOL vieclk_in;         	// check vie module clock in

	/* vd */
	UINT32 vd_cnt_clk_base;     // units : Hz
	UINT32 vd_cnt_clk;          // units : vd_cnt_clk_base (apb clk)
	UINT32 vd_cnt_pxl;          // units : pixel, previeus frame (update next vd)

	/* hd */
	UINT32 hd_cnt_clk_base;     // units : Hz
	UINT32 hd_cnt_clk;          // units : hd_cnt_clk_base (apb clk)
	UINT32 hd_cnt_line;         // units : line, previeus frame (update next vd)

} KDRV_VIE_SYS_INFO;

typedef enum {
	KDRV_VIE_MASK_IDX0,
	KDRV_VIE_MASK_IDX1,
	KDRV_VIE_MASK_IDX2,
	KDRV_VIE_MASK_IDX3,
	KDRV_VIE_MASK_IDX_MAX,
	ENUM_DUMMY4WORD(KDRV_VIE_MASK_IDX)
} KDRV_VIE_MASK_IDX;

typedef struct {
	BOOL en;
	URECT win;
	UINT8 y;   // 8bits
	UINT8 cb;  // 8bits
	UINT8 cr;  // 8bits
} KDRV_VIE_MASK_INFO;

typedef struct {
	KDRV_VIE_MASK_INFO info[KDRV_VIE_MASK_IDX_MAX];
} KDRV_VIE_MASK;

typedef struct {
	ULONG addr_va;  // virtual address
	ULONG addr_pa;  // physical address
} KDRV_VIE_ADDR;

typedef enum {
	KDRV_VIE_DELAY_CHAINA_SRC_SIE,
	KDRV_VIE_DELAY_CHAINA_SRC_VIE,
} KDRV_VIE_DELAYCHAINA_SRC;

typedef struct {
	UINT8 srca_delay_val;
	UINT8 srcb_delay_val;
	KDRV_VIE_DELAYCHAINA_SRC srca_delay_chain_src;
} KDRV_VIE_DELAYCHAIN_INFO;

/**
    data for kdrv_vie Set/Get Item
*/

typedef enum {
	KDRV_VIE_ITEM_OPENCFG = 0,  ///< data_type: [Set]     KDRV_VIE_OPENCFG, it can be used before kdrv_vie_open, by engine
	KDRV_VIE_ITEM_ISRCB,        ///< data_type: [Set]     KDRV_VIE_ISRCB,[Get]N.S., by engine
	KDRV_VIE_ITEM_VIECLK,       ///< data_type: [Set/Get] KDRV_VIE_CLK_INFO, it can be used before kdrv_vie_open, by engine
	KDRV_VIE_ITEM_CROP_WIN,     ///< data_type: [Set/Get] KDRV_VIE_CRP_WIN
	KDRV_VIE_ITEM_DATA_FMT,     ///< data_type: [Set/Get] KDRV_VIE_DATA_FMT
	KDRV_VIE_ITEM_YUV_ORDER,    ///< data_type: [Set/Get] KDRV_VIE_YUV_ORDER
	KDRV_VIE_ITEM_SIGNAL,       ///< data_type: [Set/Get] KDRV_VIE_SIGNAL
	KDRV_VIE_ITEM_FLIP,         ///< data_type: [Set/Get] KDRV_VIE_FLIP
	KDRV_VIE_ITEM_INTE,         ///< data_type: [Set/Get] KDRV_VIE_INT, set, by engine
	KDRV_VIE_ITEM_INTE_CH,      ///< data_type: [Set]     KDRV_VIE_INT, set specified channel
	KDRV_VIE_ITEM_CH0_LOF,      ///< data_type: [Set/Get] UINT32
	KDRV_VIE_ITEM_CH1_LOF,      ///< data_type: [Set/Get] UINT32
	KDRV_VIE_ITEM_CH0_ADDR,     ///< data_type: [Set/Get] KDRV_VIE_ADDR
	KDRV_VIE_ITEM_CH1_ADDR,     ///< data_type: [Set/Get] KDRV_VIE_ADDR
	KDRV_VIE_ITEM_CHDBG_ADDR,   ///< data_type: [Set/Get] KDRV_VIE_ADDR
	KDRV_VIE_ITEM_CCIR,         ///< data_type: [Set/Get] KDRV_VIE_CCIR_INFO
	KDRV_VIE_ITEM_LOAD,         ///< data_type: [Set]     NULL
	KDRV_VIE_ITEM_BP3,          ///< data_type: [Set/Get] UINT32
	KDRV_VIE_ITEM_LIMIT,        ///< data_type:     [Get] KDRV_VIE_LIMIT, return vie size align limitation, it can be used before kdrv_vie_open
	KDRV_VIE_ITEM_SINGLEOUT,    ///< data_type: [Set/Get] KDRV_VIE_SINGLE_OUT_CTRL, kdrv will return current status of single-out enable(getting from register)
	KDRV_VIE_ITEM_OUTPUT_MODE,  ///< data_type: [Set/Get] KDRV_VIE_DRAM_OUT_CTRL
	KDRV_VIE_ITEM_SYS_INFO,     ///< data_type:     [Get] KDRV_VIE_SYS_INFO, get vie hw debug information (for debug check only)
	KDRV_VIE_ITEM_CCIR_H,       ///< data_type: [Set]     UINT16, set ccir header (for debug check only)
	KDRV_VIE_ITEM_CCIR_H_STS,   ///< data_type:     [Get] UINT32, get ccir header status (for debug check only)
	KDRV_VIE_ITEM_PATGEN_INFO,  ///< data_type: [Set/Get] KDRV_VIE_PATGEN_INFO
	KDRV_VIE_ITEM_MASK,         ///< data_type: [Set/Get] KDRV_VIE_MASK
	KDRV_VIE_ITEM_FOUR_K_MODE,  ///< data_type: [Set/Get] BOOL, enable/disable 4K mode
												// 530 only surppot KDRV_VIE_VDO_CH0 & KDRV_VIE_VDO_CH1 enable
												// if KDRV_VIE_VDO_CH0 enable, cannot use KDRV_VIE_VDO_CH2 at the same time
												// if KDRV_VIE_VDO_CH1 enable, cannot use KDRV_VIE_VDO_CH3 at the same time
	KDRV_VIE_ITEM_DBG_EN,       ///< data_type: [Set/Get] BOOL
	KDRV_VIE_ITEM_DELAYCHAIN,	///< data_type: [Set/Get] KDRV_VIE_DELAYCHAIN_INFO
	KDRV_VIE_PARAM_MAX,
	KDRV_VIE_PARAM_REV = 0x80000000,
	ENUM_DUMMY4WORD(KDRV_VIE_PARAM_ID)
} KDRV_VIE_PARAM_ID;
STATIC_ASSERT((KDRV_VIE_PARAM_MAX &KDRV_VIE_PARAM_REV) == 0);
#define KDRV_VIE_IGN_CHK KDRV_VIE_PARAM_REV //only support set/get function

typedef enum {
	KDRV_VIE_PARAM_DEPEND_CH,               // Dependent on the vie input video channel
	KDRV_VIE_PARAM_DEPEND_ENGINE,           // Dependent on the vie engine
	ENUM_DUMMY4WORD(KDRV_VIE_PARAM_DEPEND)
} KDRV_VIE_PARAM_DEPEND;

typedef struct {
	KDRV_VIE_PARAM_ID	  param_id;
	KDRV_VIE_PARAM_DEPEND depend;
} KDRV_VIE_PARAM_PROP;

typedef enum {
	RESERVED_VIE = 0,
	ENUM_DUMMY4WORD(KDRV_VIE_TRIGGER_PARAM)
} KDRV_VIE_TRIGGER_PARAM;

typedef struct {
	INT32(*callback)(VOID *callback_info, VOID *user_data);
	INT32(*reserve_buf)(ULONG phy_addr);
	INT32(*free_buf)(ULONG phy_addr);
} KDRV_VIE_CALLBACK_FUNC;

typedef struct {
	BOOL pxclk_en;
	BOOL wait_end;              ///< wait vd-come, then disable vie_clk
} KDRV_VIE_SUSPEND_PARAM;

typedef struct {
	BOOL pxclk_en;
} KDRV_VIE_RESUME_PARAM;

INT32 kdrv_vie_open(UINT32 kdrv_id);
INT32 kdrv_vie_close(UINT32 kdrv_id);
INT32 kdrv_vie_resume(UINT32 kdrv_id, void *param);
INT32 kdrv_vie_suspend(UINT32 kdrv_id, void *param);
INT32 kdrv_vie_set(UINT32 kdrv_id, KDRV_VIE_PARAM_ID parm_id, VOID *param);
INT32 kdrv_vie_get(UINT32 kdrv_id, KDRV_VIE_PARAM_ID parm_id, VOID *param);
INT32 kdrv_vie_trigger(UINT32 kdrv_id, KDRV_VIE_TRIGGER_PARAM *rpc_param, KDRV_VIE_CALLBACK_FUNC *cb_func, VOID *user_data);

void kdrv_vie_init(void);
void kdrv_vie_uninit(void);

UINT32 kdrv_vie_buf_query(UINT32 engine_num);
INT32 kdrv_vie_buf_init(ULONG input_addr, UINT32 buf_size);
INT32 kdrv_vie_buf_uninit(void);
INT32 kdrv_vie_get_vie_limit(KDRV_VIE_PROC_ID id, KDRV_VIE_VDO_CH ch, void *data); // KDRV_VIE_LIMIT
void kdrv_vie_set_checksum_en(KDRV_VIE_PROC_ID id, UINT32 en); // set check sum enable, for get debug info
void kdrv_vie_dump_intrpt_sts(ULONG dump_id_bit, int (*dump)(const char *fmt, ...));
void kdrv_vie_set_err_log_rate(KDRV_VIE_PROC_ID id, UINT32 err_log_rate);

#endif //_KDRV_VIE_H_
