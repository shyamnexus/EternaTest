/**
    Public header file for SIE utility

    This file is the header file that define the API and data type for SIE utility tool.

    @file      ctl_sie_utility.h
    @ingroup    mILibSIECom
    @note      Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2009.  All rights reserved.
*/
#ifndef _CTL_SIE_UTILITY_H_
#define _CTL_SIE_UTILITY_H_

#include "kflow_videocapture/ctl_sen.h"
#include "kwrap/type.h"
#include "kwrap/error_no.h"
#include "kflow_videocapture/ctl_sie_event.h"
#include "kflow_videocapture/ctl_sie_isp.h"
#include "kflow_common/type_vdo.h"
#include "kflow_common/isp_if.h"
#include "comm/gyro_drv.h"

/**
    CTL SIE error type
*/
#define CTL_SIE_E_OK            (0)
#define CTL_SIE_E_ID            (-1)    //illegal sie handle
#define CTL_SIE_E_SYS           (-2)    //sysem error
#define CTL_SIE_E_HDL           (-3)    //null handle
#define CTL_SIE_E_NULL_FP       (-4)    //null fp
#define CTL_SIE_E_PAR           (-5)    //parameter error
#define CTL_SIE_E_NOSPT         (-6)    //not support function
#define CTL_SIE_E_TMOUT         (-7)    //time out
#define CTL_SIE_E_QOVR          (-8)    //queue overflow
#define CTL_SIE_E_NOMEM         (-9)    //no memory
#define CTL_SIE_E_STATE         (-10)   //illegal state
#define CTL_SIE_E_IPP           (-11)   //ipp error
#define CTL_SIE_E_KDRV_GET      (-100)  //kdrv get error
#define CTL_SIE_E_KDRV_SET      (-200)  //kdrv set error
#define CTL_SIE_E_KDRV_TRIG     (-300)  //kdrv trigger error
#define CTL_SIE_E_KDRV_CLOSE    (-400)  //kdrv close error
#define CTL_SIE_E_SEN           (-1000) //sensor error


#define CTL_SIE_PARAM_NONE          0xffffffff
#define CTL_SIE_RATIO(w, h) (((UINT32)(UINT16)(w) << 16) | (UINT32)(UINT16)(h))
//for CTL_SIE default parameters, only mark [CTL_SIE_DFT] parameters can used
#define CTL_SIE_DFT 0xffffcccc

typedef struct {
	ULONG va;  // virtual address
	ULONG pa;  // physical address
} CTL_SIE_ADDR;

/**
    SIE Engine ID
*/
typedef enum _CTL_SIE_ID {
	CTL_SIE_ID_1 = 0,   ///< process id 1
	CTL_SIE_ID_2,      ///< process id 2
	CTL_SIE_ID_3,      ///< process id 3
	CTL_SIE_ID_4,      ///< process id 4
	CTL_SIE_ID_5,      ///< process id 5
	CTL_SIE_ID_6,      ///< process id 6
	CTL_SIE_ID_7,      ///< process id 7
	CTL_SIE_ID_8,      ///< process id 8
	CTL_SIE_ID_9,      ///< process id 9
	CTL_SIE_ID_10,      ///< process id 10
	CTL_SIE_ID_11,      ///< process id 11
	CTL_SIE_ID_12,      ///< process id 12
	CTL_SIE_ID_MAX_NUM,
	ENUM_DUMMY4WORD(CTL_SIE_ID)
} CTL_SIE_ID;

/**
    SIE Engine ID index
*/
typedef enum {
	CTL_SIE_ID_IDX_NONE = 0x00000000,
	CTL_SIE_ID_IDX_1    = 0x00000001,   ///< process idx 1
	CTL_SIE_ID_IDX_2    = 0x00000002,   ///< process idx 2
	CTL_SIE_ID_IDX_3    = 0x00000004,   ///< process idx 3
	CTL_SIE_ID_IDX_4    = 0x00000008,   ///< process idx 4
	CTL_SIE_ID_IDX_5    = 0x00000010,   ///< process idx 5
	CTL_SIE_ID_IDX_6    = 0x00000020,   ///< process idx 6
	CTL_SIE_ID_IDX_7    = 0x00000040,   ///< process idx 7
	CTL_SIE_ID_IDX_8    = 0x00000080,   ///< process idx 8
	CTL_SIE_ID_IDX_9    = 0x00000100,   ///< process idx 9
	CTL_SIE_ID_IDX_10   = 0x00000200,   ///< process idx 10
	CTL_SIE_ID_IDX_11   = 0x00000400,   ///< process idx 11
	CTL_SIE_ID_IDX_12   = 0x00000800,   ///< process idx 12
	CTL_SIE_ID_IDX_MAX = (CTL_SIE_ID_IDX_1 | CTL_SIE_ID_IDX_2 | CTL_SIE_ID_IDX_3 | CTL_SIE_ID_IDX_4 | CTL_SIE_ID_IDX_5 | CTL_SIE_ID_IDX_6 | CTL_SIE_ID_IDX_7 | CTL_SIE_ID_IDX_8 | CTL_SIE_ID_IDX_9 | CTL_SIE_ID_IDX_10 | CTL_SIE_ID_IDX_11 | CTL_SIE_ID_IDX_12),   ///< process idx max
	ENUM_DUMMY4WORD(CTL_SIE_ID_IDX)
} CTL_SIE_ID_IDX;

/* dram output channel */
typedef enum {
	CTL_SIE_DRAM_CH0,
	CTL_SIE_DRAM_CH1,
	CTL_SIE_DRAM_CH2,
	CTL_SIE_DRAM_CH3,
	CTL_SIE_DRAM_CHDBG,
	CTL_SIE_DRAM_CH_MAX,
	ENUM_DUMMY4WORD(CTL_SIE_DRAM_CH)
} CTL_SIE_DRAM_CH;

/**
    SIE flip information
*/
typedef enum _CTL_SIE_FLIP_TYPE {
	CTL_SIE_FLIP_NONE   = 0x00000000,        ///< no flip
	CTL_SIE_FLIP_H      = 0x00000001,        ///< H flip
	CTL_SIE_FLIP_V      = 0x00000002,        ///< V flip
	CTL_SIE_FLIP_H_V    = 0x00000003,        ///< H & V flip(Flip_H | Flip_V) don't modify
	CTL_SIE_FLIP_MAX,
	ENUM_DUMMY4WORD(CTL_SIE_FLIP_TYPE)
} CTL_SIE_FLIP_TYPE;

/**
    SIE RAW Image start pixel
*/
typedef enum _CTL_SIE_RAW_PIX {
	// RGB start pixel
	CTL_SIE_RGGB_PIX_R  = 0,    ///< start pixel R
	CTL_SIE_RGGB_PIX_GR = 1,    ///< start pixel Gr
	CTL_SIE_RGGB_PIX_GB = 2,    ///< start pixel Gb
	CTL_SIE_RGGB_PIX_B  = 3,    ///< start pixel B
	CTL_SIE_RGGB_PIX_MAX_CNT = 4,

	// RGBIR data format
	CTL_SIE_RGBIR_PIX_OFFSET =  0x00010000,
	CTL_SIE_RGBIR_PIX_RGBG_GIGI = 0 + CTL_SIE_RGBIR_PIX_OFFSET,   ///< start pixel RGBG/GIrGIr
	CTL_SIE_RGBIR_PIX_GBGR_IGIG = 1 + CTL_SIE_RGBIR_PIX_OFFSET,   ///< start pixel GBGR/IrGIrG
	CTL_SIE_RGBIR_PIX_GIGI_BGRG = 2 + CTL_SIE_RGBIR_PIX_OFFSET,   ///< start pixel GIrGIr/BGRG
	CTL_SIE_RGBIR_PIX_IGIG_GRGB = 3 + CTL_SIE_RGBIR_PIX_OFFSET,   ///< start pixel IrGIrG/GRGB
	CTL_SIE_RGBIR_PIX_BGRG_GIGI = 4 + CTL_SIE_RGBIR_PIX_OFFSET,   ///< start pixel BGRG/GIrGIr
	CTL_SIE_RGBIR_PIX_GRGB_IGIG = 5 + CTL_SIE_RGBIR_PIX_OFFSET,   ///< start pixel GRGB/IrGIrG
	CTL_SIE_RGBIR_PIX_GIGI_RGBG = 6 + CTL_SIE_RGBIR_PIX_OFFSET,   ///< start pixel GIrGIr/RGBG
	CTL_SIE_RGBIR_PIX_IGIG_GBGR = 7 + CTL_SIE_RGBIR_PIX_OFFSET,   ///< start pixel IrGIrG/GBGR
	CTL_SIE_RGBIR_PIX_MAX_CNT = 8 + CTL_SIE_RGBIR_PIX_OFFSET,

	// RCCB start pixel
	CTL_SIE_RCCB_PIX_OFFSET = 0x00020000,
	CTL_SIE_RCCB_PIX_RC = 0 + CTL_SIE_RCCB_PIX_OFFSET,      ///< start pixel RC
	CTL_SIE_RCCB_PIX_CR = 1 + CTL_SIE_RCCB_PIX_OFFSET,      ///< start pixel CR
	CTL_SIE_RCCB_PIX_CB = 2 + CTL_SIE_RCCB_PIX_OFFSET,      ///< start pixel CB
	CTL_SIE_RCCB_PIX_BC = 3 + CTL_SIE_RCCB_PIX_OFFSET,      ///< start pixel BC
	CTL_SIE_RCCB_PIX_MAX_CNT = 4 + CTL_SIE_RCCB_PIX_OFFSET,

	CTL_SIE_PIX_MAX_CNT,
	ENUM_DUMMY4WORD(CTL_SIE_RAW_PIX)
} CTL_SIE_RAW_PIX;

/**
    SIE RAW Image bits
*/
typedef enum _CTL_SIE_RAW_BIT {
	CTL_SIE_RAW_BIT_8 = 8,    ///< 8 bits
	CTL_SIE_RAW_BIT_10 = 10,    ///< 10 bits
	CTL_SIE_RAW_BIT_12 = 12,    ///< 12 bits
	CTL_SIE_RAW_BIT_16 = 16,    ///< 16 bits
	CTL_SIE_RAW_BIT_MAX_CNT,
	ENUM_DUMMY4WORD(CTL_SIE_RAW_BIT)
} CTL_SIE_RAW_BIT;

/**
    CTL SIE M-Clock Source Selection
*/
typedef enum {
	CTL_SIE_MCLK_SRC_CURR = 0, ///< SIE M-clock source as current setting(no setting, no changing)
	CTL_SIE_MCLK_SRC_480,      ///< SIE M-clock source as 480MHz
	CTL_SIE_MCLK_SRC_PLL5,     ///< SIE M-clock source as PLL5
} CTL_SIE_MCLK_SRC_SEL;

typedef enum {
	CTL_SIE_ID_MCLK = 0,
	CTL_SIE_ID_MCLK2,
	CTL_SIE_ID_MCLK3,
	CTL_SIE_ID_MCLK4,
	CTL_SIE_ID_MCLK5,
} CTL_SIE_MCLK_ID;

typedef struct {
	BOOL mclk_en;
	CTL_SIE_MCLK_ID mclk_id_sel;    //kflow will auto select mclk_id in pattern gen mode
} CTL_SIE_MCLK;

/**
    sie channel 0 output destination
    SIE2: support only DRAM and DIRECT(when shdr case+ SIE1 direct) mode
    SIE3: support only DRAM mode
*/
typedef enum {
	CTL_SIE_OUT_DEST_DIRECT = 0,    ///< ch0 output direct to ipl_in
	CTL_SIE_OUT_DEST_DRAM,          ///< ch0 output to dram only
	CTL_SIE_OUT_DEST_BOTH,          ///< ch0 output to both ipl_in and dram
	CTL_SIE_OUT_DEST_MAX,
	ENUM_DUMMY4WORD(CTL_SIE_OUT_DEST)
} CTL_SIE_OUT_DEST;

typedef enum {
	CTL_SIE_CRP_OFF = 0,    ///< SIE not crop
	CTL_SIE_CRP_ON,         ///< SIE crop
	CTL_SIE_CRP_SEL_MAX,
	ENUM_DUMMY4WORD(CTL_SIE_CRP_SEL)
} CTL_SIE_CRP_SEL;

typedef enum {
	CTL_SIE_IOSIZE_AUTO = 0, ///< skip CTL_SIE_SIZE_INFO set information, ctl_sie auto gen sie crop size(according to CTL_SIE_IOSIZE_AUTO_INFO)
	CTL_SIE_IOSIZE_MANUAL,   ///< sie crop size according to CTL_SIE_SIZE_INFO
	CTL_SIE_IOSIZE_DVS_AUTO, ///< sie active size & crop size, same as sensor driver setting
	CTL_SIE_IOSIZE_SEL_MAX,
	ENUM_DUMMY4WORD(CTL_SIE_IOSIZE_SEL)
} CTL_SIE_IOSIZE_SEL;

typedef struct {
	CTL_SIE_CRP_SEL crp_sel;
	UINT32 ratio_h_v;           ///< hv ratio (H:bit[31:16], V:bit[15:0]), set [CTL_SIE_DFT] for sensor ori ratio
	UINT32 factor;              ///< max 1000, for no digital zoom effect (w = w * factor / 1000, h = h * factor / 1000)
	IPOINT sft;                 ///< based on sie active window center, unit: pixel
	USIZE sie_crp_min;          ///< set 0 for always sie crp. if sie_crp size < sie_crp_min, sie not crp (dest need to crp).
	///< only support when CTL_SIE_CRP_ON.
	USIZE sie_scl_max_sz;       ///< scale out maximum size
	USIZE dest_ext_crp_prop;    ///< destination extend crop proportion, crop win = dest_crp * (100 - dest_ext_crp_prop) / 100
//	UINT32 sie_crp_proportion;  ///< set 100, for sie crop to sie_crp size, unit: percent. only support when CTL_SIE_CRP_ON.
} CTL_SIE_IOSIZE_AUTO_INFO;

typedef struct {
	URECT   sie_crp;        ///< sie crop window
	USIZE   sie_scl_out;    ///< sie scale out size
	URECT   dest_crp;       ///< dest crop window
} CTL_SIE_SIZE_INFO;

typedef struct {
	CTL_SIE_IOSIZE_SEL iosize_sel;
	CTL_SIE_IOSIZE_AUTO_INFO auto_info;     ///< only CTL_SIE_IOSIZE_AUTO need to set
	CTL_SIE_SIZE_INFO size_info;            ///< ctl_sie_set only support when CTL_SIE_IOSIZE_MANUAL.
	USIZE align;                            ///< [OPTION] Must be aligned to a multiple of 4, set [CTL_SIE_DFT] for default align 4.
	USIZE dest_align;                       ///< destination align
} CTL_SIE_IO_SIZE_INFO;

typedef enum {
	CTL_SIE_PAT_COLORBAR    = 1,    ///< Color bar, set each color width by pat_gen_val, width should be 2's multiples
	CTL_SIE_PAT_RANDOM,             ///< Random, frame based
	CTL_SIE_PAT_FIXED,              ///< Fixed, set pixel value by pat_gen_val
	CTL_SIE_PAT_HINCREASE,          ///< 1D Increment, set pixel value by pat_gen_val and reset for every line head
	CTL_SIE_PAT_HVINCREASE,         ///< 2D increment, set to line number count for every line head
	CTL_SIE_PAT_MAX,
	ENUM_DUMMY4WORD(CTL_SIE_PATGEN_SEL)
} CTL_SIE_PATGEN_SEL;

/**
    sie pattern gen info
    Pattern Gen MCLK fixed to 240Mhz(520/525)
    pattern gen source window always start with cfa_r
    pattern gen frame time = 240Mhz / ((act_win.x + act_win_w + 8) * (act_win.y + act_win_h))
    CTL_SIE_PAT_COLORBAR: pat_gen_val = width of each color
    CTL_SIE_PAT_HINCREASE: pat_gen_val = 12bit value of start luminance
*/
typedef struct {
	URECT               act_win;        //sie active window
	URECT               crp_win;        //sie crop window
	CTL_SIE_PATGEN_SEL  pat_gen_mode;
	UINT32              pat_gen_val;
	UINT32              frame_rate;     //fps * 100
	UINT32              out_dest;		//group sie index, ref. to CTL_SIE_ID_IDX
} CTL_SIE_PAG_GEN_INFO;

typedef enum {
    CTL_SIE_IN_CH3_DATA_PACKBUS_6 = 0,     ///< 6 bits packing
    CTL_SIE_IN_CH3_DATA_PACKBUS_7,         ///< 7 bits packing
    CTL_SIE_IN_CH3_DATA_PACKBUS_8,         ///< 8 bits packing
    CTL_SIE_IN_CH3_DATA_PACKBUS_MAX,
    ENUM_DUMMY4WORD(CTL_SIE_IN_CH3_DATA_PACKBUS_SEL)
} CTL_SIE_IN_CH3_DATA_PACKBUS_SEL;

#define CTL_SIE_TSEN_CFG_NUM 		20

typedef struct {
	UINT32 ooc_id;							///< valid range: 0~255, only valid when thermal sensor
	CTL_SIE_ADDR addr;
	UINT32 lofs;
	CTL_SIE_IN_CH3_DATA_PACKBUS_SEL pack_bus_sel;
} CTL_SIE_IN_CH3_INFO;

typedef enum {
	CTL_SIE_TSEN_TX_CFG_MODE_SD0 = 0,			///< output port 1-bit, SD0 => TX_CONFIG_DATA bit0~511
	CTL_SIE_TSEN_TX_CFG_MODE_SD0_1,				///< output port 2-bit, SD0 => TX_CONFIG_DATA bit0~255, SD1 => TX_CONFIG_DATA bit256~511
	CTL_SIE_TSEN_TX_CFG_MODE_SD0_2,				///< output port 3-bit, SD0 => TX_CONFIG_DATA bit0~127, SD1 => TX_CONFIG_DATA bit128~255, SD2 => TX_CONFIG_DATA bit256~383
	CTL_SIE_TSEN_TX_CFG_MODE_SD0_3,				///< output port 7-bit, SD0 => TX_CONFIG_DATA bit0~127, SD1 => TX_CONFIG_DATA bit128~255, SD2 => TX_CONFIG_DATA bit256~383, SD3 => TX_CONFIG_DATA bit384~511
	CTL_SIE_TSEN_TX_CFG_MODE_MAX,
} CTL_SIE_TSEN_TX_CFG_MODE;

typedef struct {
	UINT32 cfg_id;							///< valid 0~255
	UINT32 tx_len;							///< from 4 ~ 512 clock cycle
	UINT32 tx_data[CTL_SIE_TSEN_CFG_NUM];	///< config data
} CTL_SIE_TSEN_TX_CFG;

/**
    sie interrupt type
*/
typedef enum {
	CTL_SIE_CLR                 = 0,
	CTL_SIE_INTE_VD             = 0x00000001,   ///< enable interrupt: VD
	CTL_SIE_INTE_BP1            = 0x00000002,   ///< enable interrupt: Break point 1
	CTL_SIE_INTE_BP2            = 0x00000004,   ///< enable interrupt: Break point 2
	CTL_SIE_INTE_BP3            = 0x00000008,   ///< enable interrupt: Break point 3
	CTL_SIE_INTE_ACTST          = 0x00000010,   ///< enable interrupt: Active window start
	CTL_SIE_INTE_CRPST          = 0x00000020,   ///< enable interrupt: Crop window start
	CTL_SIE_INTE_ERR_DRAMIO     = 0x00000040,   ///< enable interrupt: dram in/out error
	CTL_SIE_INTE_DRAM_OUT0_END  = 0x00000080,   ///< enable interrupt: Dram output channel 0 end
	CTL_SIE_INTE_DRAM_OUT1_END  = 0x00000100,   ///< enable interrupt: Dram output channel 1 end
	CTL_SIE_INTE_DRAM_OUT2_END  = 0x00000200,   ///< enable interrupt: Dram output channel 2 end
	CTL_SIE_INTE_DRAM_OUT3_END  = 0x00000400,   ///< enable interrupt: Dram output channel 3 end
	CTL_SIE_INTE_DRAM_OUTD_END  = 0x00000800,   ///< enable interrupt: Dram output channel debug end
	CTL_SIE_INTE_ERR_RAWENC     = 0x00008000,   ///< enable interrupt: raw encode overflow
	CTL_SIE_INTE_ERR_DPC        = 0x00002000,   ///< enable interrupt: dpc fail
	CTL_SIE_INTE_ACTEND         = 0x00010000,   ///< enable interrupt: Active window end
	CTL_SIE_INTE_CROPEND        = 0x00020000,   ///< enable interrupt: Crop window end
	CTL_SIE_INTE_ERR_BEHAVIOR   = 0x00040000, 	///< enable interrupt: behavior error
	CTL_SIE_INTE_SRC_BUF_OVFL   = 0x00080000,	///< only SIE2~12
	CTL_SIE_INTE_HISTO_END 	    = 0x00100000,
	CTL_SIE_INTE_ROI_ACC_END    = 0x00200000,  	///< only SIE1/4/5/7/8
	CTL_SIE_INTE_RAWENC_FIFO_OVFL 			= 0x00400000,	///< BCC_CLK and SIE_CLK setting incorrect
	CTL_SIE_INTE_RAWENC_DATA_OVLAP			= 0x00800000,  	///< BCC share and process data overlap
	CTL_SIE_INTE_RAWENC_DATA_BEHAVIOR_ERR	= 0x01000000,  	///< BCC input data behavior does not meet expectations
	CTL_SIE_INT_TSEN_OOC_VD                 = 0x02000000,   ///< Thermal sensor VD of OOC data, only SIE5
	CTL_SIE_INT_TSEN_END                    = 0x04000000,   ///< Thermal sensor end processing, only SIE5
	CTL_SIE_INT_TSEN_DRAM_IN3_UDFL          = 0x08000000,   ///< Thermal sensor ooc data dram in3 underflow, only SIE5
	CTL_SIE_INT_TSEN_FIFO_OVFL              = 0x10000000,   ///< Thermal sensor module clock < rx clock,     only SIE5
	CTL_SIE_INTE_ALL            			= 0xffffffff,   ///< enable interrupt: all
} CTL_SIE_INTE;

/**
	### DESCRIPTION:
	Input type `SIE_SERIAL_RAW_BITDEPTH` (MIPI/LVDS/PATGEN)
    ### SUPPORT: 539A only
*/
typedef enum {
	CTL_SIE_SERIAL_RAW_BIT_12 = 0,      ///< 12 bits input
	CTL_SIE_SERIAL_RAW_BIT_14,	      	///< 14 bits input
	CTL_SIE_SERIAL_RAW_BIT_16,    		///< 16 bits input
	CTL_SIE_SERIAL_RAW_BIT_MAX,
	ENUM_DUMMY4WORD(CTL_SIE_SERIAL_RAW_BIT)
} CTL_SIE_SERIAL_RAW_BIT;

/**
    type for CTL_SIE_SET_DATAFORMAT
    Bayer:
        bayer input is automatically handle by sie
        select SIE output packed, 8/10/12/16 decide sie_ch0 output bit

        When raw_encode enable, need to select 12bit
        Compress rate is 70%, lofs should be width*12/8*(0.7)
    y only:
        sie ouptut y to ch0
*/
typedef enum {
	/*
	    bayer
	*/
	CTL_SIE_BAYER_8,
	CTL_SIE_BAYER_10,
	CTL_SIE_BAYER_12,
	CTL_SIE_BAYER_16,
	CTL_SIE_BAYER_16_MSB = CTL_SIE_BAYER_16,
	CTL_SIE_BAYER_16_LSB,

	/*
		ccir
	*/
	CTL_SIE_YUV_422_SPT,
	CTL_SIE_YUV_422_NOSPT,
	CTL_SIE_YUV_420_SPT,	// YUV420 only support split output

	/*
	    y only
	*/
	CTL_SIE_Y_8,            // dvs sensor support output 8 bit data
	CTL_SIE_DATAFORMAT_MAX,
	ENUM_DUMMY4WORD(CTL_SIE_DATAFORMAT)
} CTL_SIE_DATAFORMAT;

/**
    struct for ctl_sie_open
*/
typedef enum {
	CTL_SIE_FLOW_UNKNOWN = 0,
	CTL_SIE_FLOW_SEN_IN,    //sensor input
	CTL_SIE_FLOW_PATGEN,    //SIE pattern gen mode
	CTL_SIE_FLOW_SIG_DUPL,  //SIE Signal duplicate from dupl_id
	CTL_SIE_FLOW_MAX,
	ENUM_DUMMY4WORD(CTL_SIE_FLOW_TYPE)
} CTL_SIE_FLOW_TYPE;

/**
    sie clk source select
*/
typedef enum {
	CTL_SIE_CLKSRC_CURR,        ///< SIE clock source as current setting(no setting, no changing)
	CTL_SIE_CLKSRC_192,         ///< SIE clock source as 192MHz
	CTL_SIE_CLKSRC_320,         ///< SIE clock source as 320MHz
	CTL_SIE_CLKSRC_480,         ///< SIE clock source as 480MHz, PATGEN force use 480
	CTL_SIE_CLKSRC_PLL5,        ///< SIE clock source as PLL5
	CTL_SIE_CLKSRC_PLL19,       ///< SIE clock source as PLL19
	CTL_SIE_CLKSRC_PLL23,       ///< SIE clock source as PLL23
	CTL_SIE_CLKSRC_MAX,
	CTL_SIE_CLKSRC_DEFAULT = CTL_SIE_CLKSRC_320,    ///< SIE clock source use default setting
	ENUM_DUMMY4WORD(CTL_SIE_CLKSRC_SEL)
} CTL_SIE_CLKSRC_SEL;

typedef struct {
	CTL_SIE_ID          id;
	CTL_SIE_FLOW_TYPE   flow_type;
	CTL_SIE_ID          dupl_src_id;    //source duplicate id, valid only when flow_type is CTL_SIE_FLOW_SIG_DUPL
	UINT32              sen_id;
	ISP_ID              isp_id;
	CTL_SIE_CLKSRC_SEL  clk_src_sel;    //select sie clock source
} CTL_SIE_OPEN_CFG;

/**
    struct for sie cb fp
*/
typedef struct {
	UINT32 sts;
	CTL_SIE_EVENT_FP cb_fp;
} CTL_SIE_CB_INFO;

/**
    type for CTL_SIE_CHGSENMODE_INFO
*/
typedef enum {
	CTL_SIE_OUTPUT_DEST_SIE1    = (1 << CTL_SIE_ID_1),
	CTL_SIE_OUTPUT_DEST_SIE2    = (1 << CTL_SIE_ID_2),
	CTL_SIE_OUTPUT_DEST_SIE3    = (1 << CTL_SIE_ID_3),
	CTL_SIE_OUTPUT_DEST_SIE4    = (1 << CTL_SIE_ID_4),
	CTL_SIE_OUTPUT_DEST_SIE5    = (1 << CTL_SIE_ID_5),
	CTL_SIE_OUTPUT_DEST_SIE6    = (1 << CTL_SIE_ID_6),
	CTL_SIE_OUTPUT_DEST_SIE7    = (1 << CTL_SIE_ID_7),
	CTL_SIE_OUTPUT_DEST_SIE8    = (1 << CTL_SIE_ID_8),
	CTL_SIE_OUTPUT_DEST_ALL     = 0xff,
	ENUM_DUMMY4WORD(CTL_SIE_OUTPUT_DEST)
} CTL_SIE_OUTPUT_DEST;

typedef struct {
	CTL_SIE_OUTPUT_DEST         output_dest;
	CTL_SEN_MODE                sen_mode;
} CTL_SIE_CHGSENMODE_INFO;

typedef enum {
	CTL_SIE_ENC_50 = 0,
	CTL_SIE_ENC_58,
	CTL_SIE_ENC_66,	//N.S. for NT98538
	ENUM_DUMMY4WORD(CTL_SIE_ENC_RATE_SEL)
} CTL_SIE_ENC_RATE_SEL;

typedef struct {
	BOOL enc_en;
	BOOL aggres_mode_en;
	CTL_SIE_ENC_RATE_SEL enc_rate;
} CTL_SIE_ENC_INFO;

/**
	type for CTL_SIE_SET_CCIR
*/
typedef enum {
	CTL_SIE_FIELD_DISABLE = 0,
	CTL_SIE_FIELD_EN_0,
	CTL_SIE_FIELD_EN_1,
	ENUM_DUMMY4WORD(CTL_SIE_FIELD_SEL)
} CTL_SIE_FIELD_SEL;

typedef struct {
	CTL_SIE_FIELD_SEL field_sel;	///< for CCIR field select
} CTL_SIE_CCIR_INFO;

/**
    type for CTL_SIE_SET_SIGNAL
*/
typedef enum {
	CTL_SIE_PHASE_RISING = 0,
	CTL_SIE_PHASE_FALLING,
	ENUM_DUMMY4WORD(CTL_SIE_SIGNAL_PHASE)
} CTL_SIE_SIGNAL_PHASE;

typedef struct {
	CTL_SIE_SIGNAL_PHASE vd_phase;
	CTL_SIE_SIGNAL_PHASE hd_phase;
	CTL_SIE_SIGNAL_PHASE data_phase;
	BOOL b_vd_inverse;
	BOOL b_hd_inverse;
} CTL_SIE_SIGNAL;

/*
    DVS CODE
*/
typedef struct {
	UINT8 positive;
	UINT8 negative;
	UINT8 nochange;
} CTL_SIE_DVS_CODE;

/**
 * @brief EVS_INFO
 * 
 * @related `VENDOR_VIDEOCAP_EVS_INFO`, `VDOCAP_EVS_INFO`, `KDRV_SIE_EVS_INFO`
 */
typedef struct {
	UINT32 max_out_pixel_cnt;	// evs out event data common buf = max_out_pixel_cnt * 2(byte)
	UINT16 padding_val;         // padding value when output size not 4 byte align, 538 not supported, 539A supported
} CTL_SIE_EVS_INFO;

/*
    MASK
*/
typedef enum {
	CTL_SIE_MASK_IDX0,
	CTL_SIE_MASK_IDX1,
	CTL_SIE_MASK_IDX2,
	CTL_SIE_MASK_IDX3,
	CTL_SIE_MASK_IDX_MAX,
	ENUM_DUMMY4WORD(CTL_SIE_MASK_IDX)
} CTL_SIE_MASK_IDX;

typedef struct {
	BOOL en;
	URECT win;
	UINT16 r;   // 12bits
	UINT16 g;   // 12bits
	UINT16 b;   // 12bits
	UINT16 ir;  // 12bits
} CTL_SIE_MASK_INFO;

/*
    poririty: CTL_SIE_MASK_IDX3 > CTL_SIE_MASK_IDX2 > CTL_SIE_MASK_IDX1 > CTL_SIE_MASK_IDX0 for window region overlap
*/
typedef struct {
	CTL_SIE_MASK_INFO info[CTL_SIE_MASK_IDX_MAX];
} CTL_SIE_MASK;

/*
    ISP
*/
typedef CTL_SIE_ISP_CA_RSLT CTL_SIE_CA_RSLT;
typedef CTL_SIE_ISP_LA_RSLT CTL_SIE_LA_RSLT;

/**
    sie trig type, start/stop sie
*/
typedef enum {
	CTL_SIE_TRIG_STOP = 0,      ///< Stop sie
	CTL_SIE_TRIG_START,         ///< Start sie
	ENUM_DUMMY4WORD(CTL_SIE_TRIG_TYPE)
} CTL_SIE_TRIG_TYPE;


typedef struct {
	CTL_SIE_TRIG_TYPE trig_type;
	UINT32 trig_frame_num;  ///< user define sie output frame, sie will stop after reach target frame, only effect when trig_type = CTL_SIE_TRIG_START
	///< set to 0xffffffff for continuous mode
	BOOL b_wait_end;        ///< wait trigger end
} CTL_SIE_TRIG_INFO;

/**
    type for CTL_SIE_SUS_RES_LVL
*/
typedef enum {
	CTL_SIE_SUS_RES_NONE    =   0x00000000, //SUSPEND: stop sie     /RESUME: start sie
	CTL_SIE_SUS_RES_PXCLK   =   0x00000001, //SUSPEND: disable pxclk/RESUME: enable pxclk
	CTL_SIE_SUS_RES_ALL     =   0xffffffff,
	ENUM_DUMMY4WORD(CTL_SIE_SUS_RES_LVL)
} CTL_SIE_SUS_RES_LVL;

typedef enum {
	CTL_SIE_ALG_TYPE_AWB = 0,   //enable SIE ch1 CA, max buffer: 32*32*8*2 for 680, R/G/B/Cnt/IRth/Rth/Gth/Bth @16bit
	CTL_SIE_ALG_TYPE_AE,        //enable SIE ch2 LA, max buffer: 32*32*2*2+64*2 for 680, PreGamma Lum/PostGamma Lum @16bit, Histogram 64bin@16bit
	CTL_SIE_ALG_TYPE_AF,        //enable SIE ch3 or ch4 VA, max buffer: 16*16*2*4 for 680, 2 group va, 4 words per window
	CTL_SIE_ALG_TYPE_SHDR,      //Sensor HDR effect (multi frame), if enable SIE ch3 Y out, max buffer: 128*128@12bit for 680, 12bit per window
	CTL_SIE_ALG_TYPE_WDR,       //WDR effect (single frame), if enable SIE ch3 Y out, max buffer: 128*128@12bit for 680, 12bit per window
	CTL_SIE_ALG_TYPE_ETH,       //enable SIE ch4 Edge threshold out, max buffer: eth_roi.w*eth_roi.h
	CTL_SIE_ALG_TYPE_ROIACC,    //enable SIE roi_acc, out to ch3, buffer size: 5*5@32bit
	CTL_SIE_ALG_TYPE_EXTDATA,   //N.S. for 690 enable SIE ch3 extraction data (ex:pdaf)
	CTL_SIE_ALG_TYPE_LOWLATENCY,//vcap out low latency mode, push buf out when bp3
	CTL_SIE_ALG_TYPE_ONE_BUF_MODE,	//vcap output with one buffer mode , for fastboot ref.
	CTL_SIE_ALG_TYPE_MAX,
} CTL_SIE_ALG_TYPE;

typedef struct {
	CTL_SIE_ALG_TYPE type;
	BOOL func_en;
} CTL_SIE_ALG_FUNC;

typedef struct {
	UINT32 mode;            //0:disable, 1: rec, 2:rec + sync(main)
	UINT32 sync_id;         //frame start sync source sie id, only valid at mode 2
	UINT32 sync_diff;    	//frame start sync diff ts(us), only valid at mode 2
	UINT32 det_frm_int;     //det frame interval (frame counter), only valid at mode 2
	UINT32 adj_thres;       //adjust threshold ts (us), only valid at mode 2
	UINT32 adj_auto;        //1:auto, 0: adj_self, only valid at mode 2
} CTL_SIE_SYNC_INFO;

typedef struct {
	PGYRO_OBJ p_gyro_obj;
	BOOL en;
	UINT32 data_num; 		//out buf need (data_num)*(CTL_SIE_GYRO_DATA_NUM)*(sizof(INT32):4 byte)
	BOOL   latency_en;		//enable when first frame get gyro data fail
} CTL_SIE_GYRO_CFG;

typedef enum {
	CTL_SIE_GROUP_OFF = 0,	//disable group
	CTL_SIE_GROUP_COMBINE,	//combine multi sie output, set grp_order and grp_size for output buf/lofs calc.
	CTL_SIE_GROUP_FS,		//frame sync only, multi sie with same v_sync, set grp_id_idx for frame sync
	CTL_SIE_GROUP_TYPE_MAX,
} CTL_SIE_GROUP_TYPE;

/*
	group info

	grp_type:
		COMBINE:	multi SIE outuput raw to combine image(or ycc combine when direct mode)
		FS:			frme count sync only, streaming out when all group id trigger start and reset fc

	grp_id_idx; 	total index with gropu id
*/
#define CTL_SIE_GROUP_MAX_NUM          2
typedef struct {
	CTL_SIE_GROUP_TYPE 	grp_type;							//ref. to CTL_SIE_GROUP_TYPE
    CTL_SIE_ID_IDX 		grp_id_idx;       					//ex:  CTL_SIE_ID_IDX_1 | CTL_SIE_ID_IDX_2
} CTL_SIE_GROUP_INFO;

/**
    SIE limitation infor
    sync to KDRV_SIE_LIMIT
*/
typedef struct {
	CHAR	func_name[5];
	UINT32 	func; // SIE_FUNCTION_SEL
	UINT32  val;
} CTL_SIE_LIMIT_INFO;

typedef struct {
	UINT32 max_clk_rate;        	//maximum sie clock rate
	UINT32 max_mclk_rate;       	//maximum sie mclk rate
	UINT32 max_spt_id;          	//maximum sie id
	UINT32 out_max_ch;          	//output maximum channel number
	USIZE  ca_win_max_num;       	//ca window maximum number, proposal[CA_WIN_NUMX & CA_WIN_NUMY]
	USIZE  la_win_max_num;       	//la window maximum number, proposal[LA_WIN_NUMX & LA_WIN_NUMY]
	UINT32 la_hist_bin_num;     	//la histogram maximum bin number, proposal[HISTO_Y_RSLT_0~63]
	UINT32 roi_acc_max_win_num;		//roi acc maximum window number, fixed
	URECT  roi_acc_win_align;		//roi acc window align
	URECT  roi_acc_max_win;			//roi acc max window size
	USIZE  pat_gen_src_win_align; 	// for src width/height, proposal[SRC_WIDTH & SRC_HEIGHT]
	USIZE  pat_gen_src_win_min;
	USIZE  pat_gen_src_win_max;
	UINT32 self_bcc_ability;		// support self BCC hardware
	UINT32 bcc_share_src_id;		// share with other SIE
	UINT32 bcc_in_win_w;			// min width when bcc enable

	URECT 				act_win_align;        // active window limit
	UINT32 				act_win_min_w;
	CTL_SIE_LIMIT_INFO act_win_min_w_info[1];
	UINT32 				act_win_min_h;
	CTL_SIE_LIMIT_INFO act_win_min_h_info[1];
	USIZE 				act_win_max;

	UINT32 				crp_win_align_x;
	UINT32 				crp_win_align_y;
	UINT32 				crp_win_align_w;
	UINT32 				crp_win_align_h;
	UINT32 				legacy420_crp_win_align_w;
	UINT32 				legacy420_crp_win_align_h;
	CTL_SIE_LIMIT_INFO crp_win_align_h_info[2];
	USIZE 				crp_win_min;
	USIZE 				crp_win_max[CTL_SIE_DATAFORMAT_MAX];

	USIZE scale_in_align;       // scale input limit
	USIZE scale_in_max;
	USIZE scale_out_align;      // scale output limit
	USIZE scale_out_min;
	USIZE scale_out_max;
	USIZE scale_out_ratio_max;

	USIZE ca_crp_win_min;     	//ca crop window minimum size w/h
	USIZE ca_crp_win_align;     //ca crop window size w/h size align, proposal[CA_CROP_SZX & CA_CROP_SZY]
	USIZE la_crp_win_min;     	//la crop window minimum size w/h
	USIZE la_crp_win_align;     //la crop window size w/h size align, proposal[LA_CROP_SZX & LA_CROP_SZY]
	UINT32 out_lofs_align[CTL_SIE_DRAM_CH_MAX]; //output lineoofset align, proposal[DRAM_OUT0_OFSO & DRAM_OUT1_OFSO & DRAM_OUT2_OFSO]
	UINT32 out0_lofs_align_h_flip[CTL_SIE_DATAFORMAT_MAX];	//output lineoofset for all bitdepth align when h flip enable
	UINT64 support_func;        //reference to KDRV_SIE_FUNC_SUPPORT
	UINT32 ring_buf_len_max;    //ring buffer length maximum, proposal[DRAM_OUT0_RINGBUF_LEN]
	UINT32 support_output_fmt;  // KDRV_SIE_DATAFORMAT_SPT

	/* active window 2 limit */
	URECT act_win2_align;
	USIZE act_win2_min;
	USIZE act_win2_max[CTL_SIE_DATAFORMAT_MAX];

	/* extraction limit */
	UINT32 extract_ini_len_min; 		// initial len min, units : line
	UINT32 extract_ini_len_max; 		// initial len max, units : line
	UINT32 extract_loop_keep_len_min; 	// loop keep len min, units : line
	UINT32 extract_loop_keep_len_max; 	// loop keep len max, units : line
	UINT32 extract_loop_drop_len_min; 	// loop drop len min, units : line
	UINT32 extract_loop_drop_len_max; 	// loop drop len max, units : line

} CTL_SIE_LIMIT;

/**
    data for ctl_sie Set/Get Item
        CTL_SIE_ITEM_ENCODE:
        Only control Enable/Disable
        ch0 Channel lineoffset can set to 70% when enable
        note that encode can only enable when data_fmt = CTL_SIE_BAYER_12
*/

typedef enum {
	CTL_SIE_ITEM_REV = 0,      		///< for ctl_sie_module_update_load_flg / kdf_sie_get / ...
	CTL_SIE_ITEM_MCLK_IMM,      	///< [set/get],  data_type: CTL_SIE_MCLK
	CTL_SIE_ITEM_OUT_DEST,          ///< [set/get],  data_type: CTL_SIE_OUT_DEST
	CTL_SIE_ITEM_SERIAL_RAW_BIT,	///< [set/get],  data_type: CTL_SIE_SERIAL_RAW_BIT, for sie input serial raw bitdepth (MIPI/LVDS/PATGEN)
	CTL_SIE_ITEM_DATAFORMAT,        ///< [set/get],  data_type: CTL_SIE_DATAFORMAT, for dramout ch0
	CTL_SIE_ITEM_CHGSENMODE,        ///< [set/get],  data_type: CTL_SIE_CHGSENMODE_INFO
	CTL_SIE_ITEM_FLIP,              ///< [set/get],  data_type: CTL_SIE_FLIP_TYPE
	CTL_SIE_ITEM_CH0_LOF,           ///<     [get],  data_type: UINT32, need 4 byte align
	CTL_SIE_ITEM_CH1_LOF,           ///<     [get],  data_type: UINT32, need 4 byte align
	CTL_SIE_ITEM_IO_SIZE,           ///< [set/get],  data_type: CTL_SIE_IO_SIZE_INFO
	CTL_SIE_ITEM_PATGEN_INFO,       ///< [set/get],  data_type: CTL_SIE_PAG_GEN_INFO
	CTL_SIE_ITEM_CA_RSLT,           ///<     [get],  data_type: CTL_SIE_CA_RSLT
	CTL_SIE_ITEM_LA_RSLT,           ///<     [get],  data_type: CTL_SIE_LA_RSLT
	CTL_SIE_ITEM_ENCODE,            ///< [set/get],  data_type: CTL_SIE_ENC_INFO, only valid when sensor output RAW
	CTL_SIE_ITEM_TRIG_IMM,          ///< [set],      data_type: CTL_SIE_TRIG_INFO
	CTL_SIE_ITEM_REG_CB_IMM,        ///< [set],      data_type: CTL_SIE_REG_CB_INFO
	CTL_SIE_ITEM_ALG_FUNC_IMM,      ///< [set/get],  data_type: CTL_SIE_ALG_FUNC, SIE ALG(3A+IQ...) function enable
	CTL_SIE_ITEM_RESET_FC_IMM,      ///< [set],      data_type: CTL_SIE_ID_IDX, reset frame counter(for shdr using)
	CTL_SIE_ITEM_DMA_ABORT,         ///< [set],      data_type: NONE, set DMA ABORT, need to check CTL_SIE_ITEM_DMA_STS, not need to load
	CTL_SIE_ITEM_DVS_CODE,          ///< [set/get],  data_type: CTL_SIE_DVS_CODE, dvs code for dvs data (cannot runtime change, must set before CTL_SIE_TRIG_START)
	CTL_SIE_ITEM_EVS_INFO,          ///< [set/get],  data_type: CTL_SIE_EVS_INFO, evs info for evs data (cannot runtime change, must set before CTL_SIE_TRIG_START)
	CTL_SIE_ITEM_MASK,              ///< [set/get],  data_type: CTL_SIE_MASK
	CTL_SIE_ITEM_LOAD,              ///< [set],      data_type: NULL or CTL_SIE_SET_ITEM
	CTL_SIE_ITEM_BP3_RATIO,         ///< [set/get],  data_type: UINT32, dram/both mode act_start to bp ratio(1~100),
	CTL_SIE_ITEM_SW_SYNC,           ///< [set],      data_type: CTL_SIE_SYNC_INFO
	CTL_SIE_ITEM_GYRO_CFG,			///< [set/get],  data_type: CTL_SIE_GYRO_CFG
	CTL_SIE_ITEM_GROUP,				///< [set/get],  data_type: CTL_SIE_GROUP_INFO
	CTL_SIE_ITEM_IN_CH3,			///< [set],      data_type: CTL_SIE_IN_CH3_INFO
	CTL_SIE_ITEM_TSEN_TXCFG,		///< [set],      data_type: CTL_SIE_TSEN_TX_CFG
	CTL_SIE_ITEM_LINECNT_CHK,      ///<  [get],      data_type: BOOL, check line count still update
	CTL_SIE_ITEM_MAX,
	ENUM_DUMMY4WORD(CTL_SIE_ITEM)
} CTL_SIE_ITEM;

extern BOOL ctl_sie_get_is_open(CTL_SIE_ID id);

//@}
#endif //_CTL_SIE_UTILITY_H_
