/**
    Public header file for kdrv_sie

    This file is the header file that define the API and data type for kdrv_sie.

    @file      kdrv_sie.h
    @ingroup    mILibIPLCom
    @note      Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2009.  All rights reserved.
*/
#ifndef _KDRV_SIE_H_
#define _KDRV_SIE_H_

#include "kdrv_type.h"
#include "kwrap/file.h"

#define KDRV_SIE_MAX_ENG                5   //maximum support engine number for all chip
#define KDRV_SIE_MAX_ENG_NT98538        5
#define KDRV_SIE_MAX_ENG_NT98539A       5

#define KDRV_SIE_DBG_DRAMOUT_SZ (4 * 20) // units : bytes, 32bits * 20

//function/statistic out maximum value
#define KDRV_SIE_DPC_MAX_PXCNT          4096
#define KDRV_SIE_ECS_DITHER_MAX_LVL     7   // IQ

#define KDRV_SIE_COMPANDING_FL_MAX_LEN       65  // IQ (left part of f-curve LUT for companding)
#define KDRV_SIE_COMPANDING_FM_MAX_LEN       17  // IQ (middle part of f-curve LUT for companding)
#define KDRV_SIE_COMPANDING_FR_MAX_LEN       16  // IQ (right part of f-curve LUT for companding)
#define KDRV_SIE_DECOMPANDING_KPX_MAX_LEN    32  // IQ (knee-point X for de-companding)
#define KDRV_SIE_DECOMPANDING_KPY_MAX_LEN    32  // IQ (knee-point Y for de-companding)
#define KDRV_SIE_DECOMPANDING_GAIN_MAX_LEN   32  // IQ (gain for de-companding)
#define KDRV_SIE_DECOMPANDING_SB_MAX_LEN     32  // IQ (shift-bit for de-companding)
#define KDRV_SIE_COMPANDING_YWEIGHT_MAX_LEN  17  // IQ (y weighting LUT for companding)

#define KDRV_SIE_VAR2STR(x)   (#x)
#define KDRV_SIE_DUMPD(x)     DBG_DUMP("%s = %d\r\n", KDRV_SIE_VAR2STR(x), x)
#define KDRV_SIE_DUMPH(x)     DBG_DUMP("%s = 0x%.8x\r\n", KDRV_SIE_VAR2STR(x), x)

/**
    sie support function.
*/
typedef enum {
	KDRV_SIE_FUNC_SPT_NONE          = 0x00000000, // none
	KDRV_SIE_FUNC_SPT_DIRECT        = 0x00000001, // direct to ipp
	KDRV_SIE_FUNC_SPT_PATGEN        = 0x00000002, // pattern gen
	KDRV_SIE_FUNC_SPT_DVI           = 0x00000004, // N.S.
	KDRV_SIE_FUNC_SPT_OB_AVG        = 0x00000008, // ob average
	KDRV_SIE_FUNC_SPT_OB_BYPASS     = 0x00000010, // ob bypass
	KDRV_SIE_FUNC_SPT_OB_FRAME_AVG  = 0x00000020, // ob frame average
	KDRV_SIE_FUNC_SPT_OB_PLANE_SUB  = 0x00000040, // ob plane sub
	KDRV_SIE_FUNC_SPT_YOUT          = 0x00000080, // y out
	KDRV_SIE_FUNC_SPT_CGAIN         = 0x00000100, // color gain
	KDRV_SIE_FUNC_SPT_GRID_LINE     = 0x00000200, // grid line
	KDRV_SIE_FUNC_SPT_DPC           = 0x00000400, // defect pixel compensation
	KDRV_SIE_FUNC_SPT_PFPC          = 0x00000800, // period fixed pattern compensation
	KDRV_SIE_FUNC_SPT_ROI_ACC		= 0x00001000, // roi acc
	KDRV_SIE_FUNC_SPT_ECS           = 0x00002000, // ecs
	KDRV_SIE_FUNC_SPT_DGAIN         = 0x00004000, // digital gain
	KDRV_SIE_FUNC_SPT_BS_H          = 0x00008000, // horizontal bayer scale
	KDRV_SIE_FUNC_SPT_BS_V          = 0x00010000, // vertical bayer scale
	KDRV_SIE_FUNC_SPT_FLIP_H        = 0x00020000, // horizontal flip
	KDRV_SIE_FUNC_SPT_FLIP_V        = 0x00040000, // vertical flip
	KDRV_SIE_FUNC_SPT_RAWENC        = 0x00080000, // raw compress
	KDRV_SIE_FUNC_SPT_LA            = 0x00100000, // luminance accumulation output
	KDRV_SIE_FUNC_SPT_LA_HISTO      = 0x00200000, // luminance accumulation histogram output
	KDRV_SIE_FUNC_SPT_CA            = 0x00400000, // color accumulation output
	KDRV_SIE_FUNC_SPT_VA            = 0x00800000, // variation accumulation output
	KDRV_SIE_FUNC_SPT_COMPANDING    = 0x01000000, // companding function
	KDRV_SIE_FUNC_SPT_RGBIR_FMT_SEL = 0x02000000, // RGBIR format Selection
	KDRV_SIE_FUNC_SPT_RCCB_FMT_SEL  = 0x04000000, // RCCB format Selection
	KDRV_SIE_FUNC_SPT_RGGB_FMT_SEL  = 0x08000000, // RGGB format Selection
	KDRV_SIE_FUNC_SPT_SINGLE_OUT    = 0x10000000, // single out
	KDRV_SIE_FUNC_SPT_RING_BUF      = 0x20000000, // ring buffer
	KDRV_SIE_FUNC_SPT_EVS           = 0x40000000, // evs data
	KDRV_SIE_FUNC_SPT_DVS           = 0x80000000, // dvs data parser
	KDRV_SIE_FUNC_SPT_VD_DLY        = 0x0000000100000000LLU, // vd delay
	KDRV_SIE_FUNC_SPT_EXTDATA       = 0x0000000200000000LLU, // extraction data (ex:pdaf)
															 // [NOTE] ssdrv N.S. act2_spt_func & data_format_ch3_spt_func,
															 //        act2 & data_format_ch3 are also used KDRV_SIE_FUNC_SPT_EXTDATA temporarily
	KDRV_SIE_FUNC_SPT_STCS_DGAIN    = 0x0000000400000000LLU, // statistics path dgain
	KDRV_SIE_FUNC_SPT_MASK          = 0x0000000800000000LLU, // mask
} KDRV_SIE_FUNC_SUPPORT;

/**
    sie clk source select
*/
typedef enum {
	KDRV_SIE_CLKSRC_CURR = 0,   ///< SIE clock source/rate keep previous setting
	KDRV_SIE_CLKSRC_480,        ///< SIE clock source as 480MHz
	KDRV_SIE_CLKSRC_PLL17,      ///< SIE clock source as PLL17, 538 not supported, 539A supported
	KDRV_SIE_CLKSRC_PLL5,       ///< SIE clock source as PLL5
	KDRV_SIE_CLKSRC_PLL13,      ///< SIE clock source as PLL13
	KDRV_SIE_CLKSRC_PLL12,      ///< SIE clock source as PLL12
	KDRV_SIE_CLKSRC_320,        ///< SIE clock source as 320MHz
	KDRV_SIE_CLKSRC_192,        ///< SIE clock source as 192MHz
	KDRV_SIE_CLKSRC_PLL10,      ///< SIE clock source as PLL10

	KDRV_SIE_CLKSRC_PLL19,      ///< FixMe, reserve for kflow build error, please remove it
	KDRV_SIE_CLKSRC_PLL23,      ///< FixMe, reserve for kflow build error, please remove it

	ENUM_DUMMY4WORD(KDRV_SIE_CLKSRC_SEL)
} KDRV_SIE_CLKSRC_SEL;

typedef struct {
	UINT32 rate;
	KDRV_SIE_CLKSRC_SEL clk_src_sel;        ///< SIE clock source selection
} KDRV_SIE_CLK_INFO;

/**
    sie pclk source select
*/
typedef enum {
	KDRV_SIE_PXCLKSRC_OFF,      ///< SIE pixel-clock disable
	KDRV_SIE_PXCLKSRC_PAD,      ///< SIE pixel-clock enabled, source as pixel-clock-pad
	KDRV_SIE_PXCLKSRC_MCLK,     ///< SIE pixel-clock enabled, source as MCLK (SIE_PXCLK from SIE_MCLK, SIE2_PXCLK from SIE_MCLK2, SIE3_PXCLK from SIE_MCLK2, SIE4_PXCLK from SIE_MCLK2)
	///< Usually for Pattern Gen Only
	KDRV_SIE_PXCLKSRC_VX1_1X,   ///< SIE pixel-clock enabled, source as Vx1 1x clock (only for SIE2/3)
	KDRV_SIE_PXCLKSRC_VX1_2X,   ///< SIE pixel-clock enabled, source as Vx1 2x clock (only for SIE2/3)
	ENUM_DUMMY4WORD(KDRV_SIE_PXCLKSRC_SEL)
} KDRV_SIE_PXCLKSRC_SEL;

/**
    SIE M-Clock Source Selection

    Structure of M-Clock source SIE configuration.
*/
//@{
typedef enum {
	KDRV_SIE_MCLKSRC_CURR = 0, ///< SIE M-clock source as current setting(no setting, no changing)
	KDRV_SIE_MCLKSRC_480,      ///< SIE M-clock source as 480MHz
	KDRV_SIE_MCLKSRC_PLL5,     ///< SIE M-clock source as PLL5
	KDRV_SIE_MCLKSRC_PLL6,     ///< SIE M-clock source as PLL6
	KDRV_SIE_MCLKSRC_PLL12,    ///< SIE M-clock source as PLL12
} KDRV_SIE_MCLKSRC_SEL;

typedef enum {
	KDRV_SIE_MCLK = 0,
	KDRV_SIE_MCLK2,
	KDRV_SIE_MCLK3,
	KDRV_SIE_MCLK4,
} KDRV_SIE_MCLK_ID;
/**
    type for KDRV_SIE_MCLK
*/
typedef struct {
	BOOL                clk_en;
	KDRV_SIE_MCLKSRC_SEL mclk_src_sel;
	UINT32              clk_rate;
	KDRV_SIE_MCLK_ID    mclk_id_sel;
} KDRV_SIE_MCLK_INFO;

typedef enum {
	KDRV_SIE_BCC_CLK_SRC_480,      ///< source as 480MHz
	KDRV_SIE_BCC_CLK_SRC_PLL23,    ///< source as PLL23
} KDRV_SIE_BCC_CLK_SRC_SEL;

typedef enum {
    KDRV_SIE_TSEN_RXCLKSRC_CURR = 0,
    KDRV_SIE_TSEN_RXCLKSRC_PXCLK,            ///< from sensor pixel clock
    KDRV_SIE_TSEN_RXCLKSRC_SN_MCLK3,         ///< SENSOR_MCLK3, loopback from sensor mclk
    KDRV_SIE_TSEN_RXCLKSRC_MAX
} KDRV_SIE_TSEN_RXCLKSRC;

/**
    BCC clock info
*/
typedef struct {
	KDRV_SIE_BCC_CLK_SRC_SEL 	bcc_clk_src_sel;
	UINT32              		clk_rate;
} KDRV_SIE_BCC_CLK_INFO;

typedef struct {
	KDRV_SIE_MCLK_INFO      mclk_info;
	KDRV_SIE_CLK_INFO       clk_info;
	KDRV_SIE_PXCLKSRC_SEL   pxclk_info;
	KDRV_SIE_BCC_CLK_INFO 	bcc_clk_info;
	KDRV_SIE_TSEN_RXCLKSRC  tsen_rx_clk_sel;
	UINT32 cur_clk_rate;
	UINT32 int_clk_src;						///<internal clock src
} KDRV_SIE_CLK_HDL;

/**
    RGB channel index
*/
//@{
typedef enum {
	KDRV_SIE_RGB_R = 0,     ///< process channel R
	KDRV_SIE_RGB_G,         ///< process channel G
	KDRV_SIE_RGB_B,         ///< process channel B
	KDRV_SIE_RGB_MAX_CH,
	ENUM_DUMMY4WORD(KDRV_SIE_RGB_CH)
} KDRV_SIE_RGB_CH;

/**
    IPL flip information
*/
typedef enum _KDRV_SIE_FLIP {
	KDRV_SIE_FLIP_NONE =    0x00000000,      ///< no flip
	KDRV_SIE_FLIP_H =       0x00000001,      ///< H flip
	KDRV_SIE_FLIP_V =       0x00000002,      ///< V flip
	KDRV_SIE_FLIP_H_V  =    0x00000003,      ///<H & V flip(Flip_H | Flip_V) don't modify
	KDRV_SIE_FLIP_MAX,
	ENUM_DUMMY4WORD(KDRV_SIE_FLIP)
} KDRV_SIE_FLIP;

/**
    IPL RAW Image start pixel
*/
typedef enum {
	// RGB start pixel
	KDRV_SIE_RGGB_PIX_R = 0,    ///< start pixel R
	KDRV_SIE_RGGB_PIX_GR,       ///< start pixel Gr
	KDRV_SIE_RGGB_PIX_GB,       ///< start pixel Gb
	KDRV_SIE_RGGB_PIX_B,        ///< start pixel B
	KDRV_SIE_RGGB_PIX_MAX,

	// RGBIR data format
	KDRV_SIE_RGBIR_PIX_RG_GI,
	KDRV_SIE_RGBIR_PIX_GB_IG,
	KDRV_SIE_RGBIR_PIX_GI_BG,
	KDRV_SIE_RGBIR_PIX_IG_GR,
	KDRV_SIE_RGBIR_PIX_BG_GI,
	KDRV_SIE_RGBIR_PIX_GR_IG,
	KDRV_SIE_RGBIR_PIX_GI_RG,
	KDRV_SIE_RGBIR_PIX_IG_GB,
	KDRV_SIE_RGBIR_PIX_MAX,

	// RCCB start pixel
	KDRV_SIE_RCCB_PIX_RC,   ///< start pixel RC
	KDRV_SIE_RCCB_PIX_CR,   ///< start pixel CR
	KDRV_SIE_RCCB_PIX_CB,   ///< start pixel CB
	KDRV_SIE_RCCB_PIX_BC,   ///< start pixel BC
	KDRV_SIE_RCCB_PIX_MAX,

	ENUM_DUMMY4WORD(KDRV_SIE_PIX)
} KDRV_SIE_PIX;

/**
    IPL process ID
*/
typedef enum {
	KDRV_SIE_ID_1 = 0,      ///< process id 1
	KDRV_SIE_ID_2,          ///< process id 2
	KDRV_SIE_ID_3,          ///< process id 3
	KDRV_SIE_ID_4,          ///< process id 4,
	KDRV_SIE_ID_5,          ///< process id 5,
	KDRV_SIE_ID_6,          ///< process id 6,
	KDRV_SIE_ID_7,          ///< process id 7,
	KDRV_SIE_ID_8,          ///< process id 8,
	KDRV_SIE_ID_9,          ///< process id 9,
	KDRV_SIE_ID_10,         ///< process id 10,
	KDRV_SIE_ID_11,         ///< process id 11,
	KDRV_SIE_ID_12,         ///< process id 12,
	KDRV_SIE_ID_MAX_NUM,
	ENUM_DUMMY4WORD(KDRV_SIE_PROC_ID)
} KDRV_SIE_PROC_ID;


/**
    sie signal receive mode
*/
typedef enum {
	KDRV_SIE_IN_PARA_MSTR_SNR = 0,  ///< Parallel Master Sensor
	KDRV_SIE_IN_PARA_SLAV_SNR,      ///< Parallel Slave Sensor
	KDRV_SIE_IN_PATGEN,             ///< Self Pattern-Generator
	KDRV_SIE_IN_VX1_IF0_SNR,        ///< Vx1 Sensor
	KDRV_SIE_IN_CSI_1,              ///< Serial Sensor from CSI-1
	KDRV_SIE_IN_CSI_2,              ///< Serial Sensor from CSI-2
	KDRV_SIE_IN_CSI_3,              ///< Serial Sensor from CSI-3
	KDRV_SIE_IN_CSI_4,              ///< Serial Sensor from CSI-4
	KDRV_SIE_IN_CSI_5,              ///< Serial Sensor from CSI-5
	KDRV_SIE_IN_CSI_6,              ///< Serial Sensor from CSI-6
	KDRV_SIE_IN_CSI_7,              ///< Serial Sensor from CSI-7
	KDRV_SIE_IN_CSI_8,              ///< Serial Sensor from CSI-8
	KDRV_SIE_IN_2PPATGEN,			///< 2-pixel pattern gen mode, use sie clock
	KDRV_SIE_IN_SLVS_EC_1,          ///< Serial Sensor from SLVS-EC 1
	KDRV_SIE_IN_VX1_IF1_SNR,        ///< Vx1 Sensor, interface 1
	KDRV_SIE_IN_PARA_THERMAL_SNR,	//parallel thermal sensor
	ENUM_DUMMY4WORD(KDRV_SIE_ACT_MODE)
} KDRV_SIE_ACT_MODE;

/**
    sie kdrv interrupt type, must sync proposal bit !!!!!!!!!
*/
typedef enum {
	KDRV_SIE_INT_CLR            = 0,
	KDRV_SIE_INT_VD             = 0x00000001,   ///< interrupt: VD
	KDRV_SIE_INT_BP1            = 0x00000002,   ///< interrupt: Break point 1
	KDRV_SIE_INT_BP2            = 0x00000004,   ///< interrupt: Break point 2
	KDRV_SIE_INT_BP3            = 0x00000008,   ///< interrupt: Break point 3
	KDRV_SIE_INT_ACTST          = 0x00000010,   ///< interrupt: Active window start
	KDRV_SIE_INT_CRPST          = 0x00000020,   ///< interrupt: Crop window start
	KDRV_SIE_INT_ERR_DRAMIO     = 0x00000040,   ///< interrupt: dram in/out error
	KDRV_SIE_INT_DRAM_OUT0_END  = 0x00000080,   ///< interrupt: Dram output channel 0 end
	KDRV_SIE_INT_DRAM_OUT1_END  = 0x00000100,   ///< interrupt: Dram output channel 1 end
	KDRV_SIE_INT_DRAM_OUT2_END  = 0x00000200,   ///< interrupt: Dram output channel 2 end
	KDRV_SIE_INT_DRAM_OUT3_END  = 0x00000400,   ///< interrupt: Dram output channel 3 end
	KDRV_SIE_INT_DRAM_OUTD_END  = 0x00000800,   ///< interrupt: Dram output channel dbg end
	KDRV_SIE_INT_ERR_DPC     	= 0x00002000,   ///< interrupt: dpc fail
	KDRV_SIE_INT_ERR_SIECLK     = 0x00004000,   ///< interrupt: sie clk err
	KDRV_SIE_INT_ERR_RAWENC     = 0x00008000,   ///< interrupt: raw encode overflow
	KDRV_SIE_INT_ACTEND         = 0x00010000,   ///< interrupt: Active window send
	KDRV_SIE_INT_CROPEND        = 0x00020000,   ///< interrupt: Crop window send
	KDRV_SIE_INT_BEHAVIOR       = 0x00040000,   ///< interrupt: vd -> dramend -> vd : miss dramend (sie module magic: must error in direct mode)
	KDRV_SIE_INT_SRC_BUF_OVFL   = 0x00080000,	///< only SIE2~12
	KDRV_SIE_INT_HISTO_END 	    = 0x00100000,
	KDRV_SIE_INT_ROI_ACC_END    = 0x00200000,  	///< only SIE1/4/5/7/8
	KDRV_SIE_INT_RAWENC_FIFO_OVFL 			= 0x00400000,	///< BCC_CLK and SIE_CLK setting incorrect
	KDRV_SIE_INT_RAWENC_DATA_OVLAP			= 0x00800000,  	///< BCC share and process data overlap
	KDRV_SIE_INT_RAWENC_DATA_BEHAVIOR_ERR	= 0x01000000,  	///< BCC input data behavior does not meet expectations
	KDRV_SIE_INT_TSEN_OOC_VD                = 0x02000000,   ///< Thermal sensor VD of OOC data, only SIE5
	KDRV_SIE_INT_TSEN_END                   = 0x04000000,   ///< Thermal sensor end processing, only SIE5
	KDRV_SIE_INT_TSEN_DRAM_IN3_UDFL         = 0x08000000,   ///< Thermal sensor ooc data dram in3 underflow, only SIE5
	KDRV_SIE_INT_TSEN_FIFO_OVFL             = 0x10000000,   ///< Thermal sensor module clock < rx clock,     only SIE5
	KDRV_SIE_INT_ALL            = 0xffffffff,   ///< interrupt: all
} KDRV_SIE_INT;

/**
    struct for kdrv_sie_open
*/
typedef struct {
	KDRV_SIE_ACT_MODE act_mode;         ///< SIE active mode, based on sensor type
	KDRV_SIE_CLKSRC_SEL clk_src_sel;    ///< SIE clock source selection, for patgen using
	KDRV_SIE_PXCLKSRC_SEL pclk_src_sel; ///< SIE pixel clk source selection
	UINT32 data_rate;                   ///< Sensor output data rate(bytes per seconds), for patgen using
} KDRV_SIE_OPENCFG;

/**
    sie kdrv trig type, start/stop sie
*/
typedef enum {
	KDRV_SIE_TRIG_STOP = 0, ///< Stop sie
	KDRV_SIE_TRIG_START,    ///< Start sie
	ENUM_DUMMY4WORD(KDRV_SIE_TRIG_TYPE)
} KDRV_SIE_TRIG_TYPE;

/**
    struct for kdrv_sie_open
*/
typedef struct {
	KDRV_SIE_TRIG_TYPE trig_type;   ///< trigger operation
	BOOL wait_end;                  ///< wait trigger end
} KDRV_SIE_TRIG_INFO;

/**
    struct for kdrv_sie_crp and kdrv_sie_act
*/
typedef struct {
	URECT win;
	KDRV_SIE_PIX cfa_pat;   ///< raw cfa start pixel for active and crop window
} KDRV_SIE_ACT_CRP_WIN;

/*
    dram output channel
*/
typedef enum  {
	KDRV_SIE_DRAM_CH0,
	KDRV_SIE_DRAM_CH1,
	KDRV_SIE_DRAM_CH2,
	KDRV_SIE_DRAM_CH3,
	KDRV_SIE_DRAM_CHDBG,
	KDRV_SIE_DRAM_CH_MAX,
	KDRV_SIE_DRAM_CH_ALL = 0xff,
	ENUM_DUMMY4WORD(KDRV_SIE_DRAM_CH)
} KDRV_SIE_DRAM_CH;

/**
    callback function prototype
    UINT32 id
    UINT32 msg_type
    void* in_data
    void* out_data
*/
typedef INT32(*KDRV_SIE_ISRCB)(UINT32, UINT32, void *, void *);

/**
    sie channel 0 output destination
*/
typedef enum {
	KDRV_SIE_OUT_DEST_DIRECT = 0,   ///< ch0 output direct to ipl_in
	KDRV_SIE_OUT_DEST_DRAM,         ///< ch0 output to dram
	KDRV_SIE_OUT_DEST_BOTH,         ///< ch0 output to both ipl_in and dram
	KDRV_SIE_OUT_DEST_MAX,
	ENUM_DUMMY4WORD(KDRV_SIE_OUT_DEST)
} KDRV_SIE_OUT_DEST;

/**
    type for KDRV_SIE_SERIAL_RAW_BITDEPTH
	sie input bitdepth for serial interface, MIPI/LVDS/PATGEN
	538 not supported, 539A supported
*/
typedef enum {
    KDRV_SIE_SERIAL_RAW_BIT_12 = 0,      ///< 12 bits input
    KDRV_SIE_SERIAL_RAW_BIT_14,          ///< 14 bits input
    KDRV_SIE_SERIAL_RAW_BIT_16,          ///< 16 bits input
    KDRV_SIE_SERIAL_RAW_BIT_MAX,
    ENUM_DUMMY4WORD(KDRV_SIE_SERIAL_RAW_BITDEPTH)
} KDRV_SIE_SERIAL_RAW_BITDEPTH;      ///< for serial interface, MIPI/LVDS/PATGEN

typedef struct {
	KDRV_SIE_SERIAL_RAW_BITDEPTH serial_raw_bitdepth;
} KDRV_SIE_SERIAL_RAW_PARAM;

/**
    type for KDRV_SIE_DATA_FMT
		Bayer
	    select output packbud, 8/10/12/16 decide sie_ch0 output bit

	    When raw_encode enable, need to select 12bit
	    Compress rate is 70%, lofs should be width*12/8*(0.7)
*/
typedef enum {
	KDRV_SIE_BAYER_8,
	KDRV_SIE_BAYER_10,
	KDRV_SIE_BAYER_12,
	KDRV_SIE_BAYER_16,
	KDRV_SIE_BAYER_16_MSB = KDRV_SIE_BAYER_16,
	KDRV_SIE_BAYER_16_LSB,

	/** CCIR
		select ccir input format
		sie always output y to ch0, uv to ch1
	*/
	KDRV_SIE_YUV_422_SPT,
	KDRV_SIE_YUV_422_NOSPT,
	KDRV_SIE_YUV_420_SPT,

	/*
	    Y ONLY
	    sie output y to ch0
	*/
	KDRV_SIE_Y_8,

	KDRV_SIE_DATA_FMT_MAX,

} KDRV_SIE_DATA_FMT;

/**
	type for KDRV_SIE_SET_YUV_ORDER
*/
typedef enum {
	KDRV_SIE_YUYV = 0,
	KDRV_SIE_YVYU,
	KDRV_SIE_UYVY,
	KDRV_SIE_VYUY,
	KDRV_SIE_YYU_YYV,
	KDRV_SIE_YUY_YVY,
	KDRV_SIE_UYY_VYY,
	ENUM_DUMMY4WORD(KDRV_SIE_YUV_ORDER)
} KDRV_SIE_YUV_ORDER;


typedef enum {
	KDRV_SIE_DVI_FORMAT_CCIR601  = 0,    ///< CCIR 601
	KDRV_SIE_DVI_FORMAT_CCIR656_EAV,     ///< CCIR 656 EAV
	KDRV_SIE_DVI_FORMAT_CCIR656_ACT,     ///< CCIR 656 ACT
	KDRV_SIE_DVI_FORMAT_LEGACY_YUV420,
	ENUM_DUMMY4WORD(KDRV_SIE_DVI_FORMAT_SEL)
} KDRV_SIE_DVI_FORMAT_SEL;

typedef enum {
	KDRV_SIE_DVI_MODE_SD  = 0,   ///< SD mode (8 bits)
	KDRV_SIE_DVI_MODE_HD,        ///< HD mode (16bits)
	KDRV_SIE_DVI_MODE_HD_INV,    ///< HD mode (16bits) with Byte Inverse
	ENUM_DUMMY4WORD(KDRV_SIE_DVI_IN_MODE_SEL)
} KDRV_SIE_DVI_IN_MODE_SEL;

/**
    type for KDRV_SIE_SET_SIGNAL
*/
typedef enum {
	KDRV_SIE_PHASE_RISING = 0,
	KDRV_SIE_PHASE_FALLING,
	ENUM_DUMMY4WORD(KDRV_SIE_SIGNAL_PHASE)
} KDRV_SIE_SIGNAL_PHASE;

typedef struct {
	KDRV_SIE_SIGNAL_PHASE vd_phase;
	KDRV_SIE_SIGNAL_PHASE hd_phase;
	KDRV_SIE_SIGNAL_PHASE data_phase;
	BOOL vd_inverse;
	BOOL hd_inverse;
} KDRV_SIE_SIGNAL;

typedef enum {
	KDRV_SIE_PAT_COLORBAR = 1,  ///< Color bar
	KDRV_SIE_PAT_RANDOM,        ///< Random
	KDRV_SIE_PAT_FIXED,         ///< Fixed
	KDRV_SIE_PAT_HINCREASE,     ///< 1D Increment
	KDRV_SIE_PAT_HVINCREASE,    ///< 2D increment
	ENUM_DUMMY4WORD(KDRV_SIE_PATGEN_SEL)
} KDRV_SIE_PATGEN_SEL;

typedef struct {
	KDRV_SIE_PATGEN_SEL mode;
	UINT32              val;
	USIZE               src_win;
} KDRV_SIE_PATGEN_INFO;

/**
    type for KDRV_SIE_SET_OB
*/
typedef struct {
	BOOL bypass_enable;     ///< ob bypass enable, note that ECS/DGain will substract ob_val before function and add it back while end of the function
	UINT32 ob_ofs;          ///< ob offset
} KDRV_SIE_OB_PARAM;

/**
    type for KDRV_SIE_SET_CA_ROI
*/
typedef struct {
	UINT32 roi_base;
	URECT roi;              ///< ca region of interest, source is sie scale out
	///< when setting, roi need be 'ratio', and roi change to 'size' for getting
} KDRV_SIE_CA_ROI;

typedef enum {
	KDRV_SIE_CA_SRC_AFTER_OB = 0,
	KDRV_SIE_CA_SRC_BEFORE_OB,
	ENUM_DUMMY4WORD(KDRV_SIE_CA_SRC_SEL)
} KDRV_SIE_CA_SRC_SEL;

/**
    type for KDRV_SIE_SET_CA

    buffer calculation:
    CA Output R/G/B/Cnt/IRth/Rth/Gth/Bth @16bit for each window
    buffer_size = win_num_w * win_num_h * 8 * (16/8) bytes
    max_buffer_size = 32 * 32 * 8 * 16/8
*/
typedef struct {
	BOOL enable;            ///< ca enable, ca output use sie channel 1
	USIZE win_num;          ///< ca window number, 1x1 ~ 32x32

	BOOL th_enable;         ///< ca threshold enable
	UINT16 g_th_l;          ///< G threshold lower bound
	UINT16 g_th_u;          ///< G threshold upper bound
	UINT16 r_th_l;          ///< R threshold lower bound
	UINT16 r_th_u;          ///< R threshold upper bound
	UINT16 b_th_l;          ///< B threshold lower bound
	UINT16 b_th_u;          ///< B threshold upper bound
	UINT16 p_th_l;          ///< P threshold lower bound
	UINT16 p_th_u;          ///< P threshold upper bound

	UINT32 irsub_r_weight;  ///< IR weight for R channel, 0~255
	UINT32 irsub_g_weight;  ///< IR weight for G channel, 0~255
	UINT32 irsub_b_weight;  ///< IR weight for B channel, 0~255
	UINT32 ca_ob_ofs;
	KDRV_SIE_CA_SRC_SEL ca_src;
	UINT16 rang_shift;		///< right shift bit when companding is disable, 0~8, 538 not supported, 539A supported
} KDRV_SIE_CA_PARAM;
/**
    type for KDRV_SIE_GET_CA_RST
    ca result, array size should be window num_x * num_y
    user need to prepare memory for these pointer
*/
typedef struct {
	ULONG src_addr;
	UINT16 *buf_r;  //[KDRV_SIE_CA_MAX_WINNUM * KDRV_SIE_CA_MAX_WINNUM];
	UINT16 *buf_g;  //[KDRV_SIE_CA_MAX_WINNUM * KDRV_SIE_CA_MAX_WINNUM];
	UINT16 *buf_b;  //[KDRV_SIE_CA_MAX_WINNUM * KDRV_SIE_CA_MAX_WINNUM];
	UINT16 *buf_ir; //[KDRV_SIE_CA_MAX_WINNUM * KDRV_SIE_CA_MAX_WINNUM];
	UINT16 *acc_cnt;//[KDRV_SIE_CA_MAX_WINNUM * KDRV_SIE_CA_MAX_WINNUM];
} KDRV_SIE_CA_RST;

/**
    type for KDRV_SIE_SET_LA

    buffer calculation:
    LA Output PreGamma Lum/PostGamma Lum @16bit for each window, Histogram 64bin@16bit
    buffer_size = win_num_w * win_num_h * 2 * (16/8) + 64 * (16/8) bytes
    max_buffer_size = (32 * 32 * 2 * 16/8) + (64 * 16/8)
*/
typedef enum {
	KDRV_SIE_LA_SRC_POST_CG = 0,
	KDRV_SIE_LA_SRC_PRE_CG,
	ENUM_DUMMY4WORD(KDRV_SIE_LA_SRC_SEL)
} KDRV_SIE_LA_SRC_SEL;

typedef enum {
	KDRV_SIE_LA_HIST_SRC_POST_GMA = 0,
	KDRV_SIE_LA_HIST_SRC_PRE_GMA,
	ENUM_DUMMY4WORD(KDRV_SIE_LA_HIST_SRC_SEL)
} KDRV_SIE_LA_HIST_SRC_SEL;

typedef struct {
	UINT32 roi_base;
	URECT roi;                  ///< la region of interest, source is sie ca crop window
	///< when setting, roi need be 'ratio', and roi change to 'size' for getting
} KDRV_SIE_LA_ROI;

typedef struct {
	UINT32 roi_base;
	URECT roi;                  ///< roi acc region of interest, source is sie crop window
	///< when setting, roi need be 'ratio', and roi change to 'size' for getting
} KDRV_SIE_ROI_ACC_ROI;


typedef enum {
	STCS_LA_RGB     = 0,        ///< use RGB channel, transfer to Y
	STCS_LA_G,                  ///< use G channel as Y
	ENUM_DUMMY4WORD(KDRV_SIE_LA_RGB2Y_MOD_SEL)
} KDRV_SIE_LA_RGB2Y_MOD_SEL;

typedef struct {
	BOOL enable;                ///< la enable, la output use sie channel 2
	USIZE win_num;              ///< la window number, 1x1 ~ 32x32
	KDRV_SIE_LA_SRC_SEL la_src; ///< la source selection
	KDRV_SIE_LA_RGB2Y_MOD_SEL la_rgb2y1mod;
	KDRV_SIE_LA_RGB2Y_MOD_SEL la_rgb2y2mod;

	BOOL cg_enable;             ///< la color gain enable, in 3.7 bits format
	UINT16 r_gain;
	UINT16 g_gain;
	UINT16 b_gain;

	BOOL gamma_enable;          ///< la gamma enable
	ULONG gamma_tbl_addr;       ///< la gamma table, size 65

	BOOL histogram_enable;                  ///< la histogram enable
	KDRV_SIE_LA_HIST_SRC_SEL histogram_src; ///< la histogram src, TRUE --> data before gamma, FALSE --> data after gamma
	UINT32 irsub_r_weight;  ///< IR weight for R channel, 0~255
	UINT32 irsub_g_weight;  ///< IR weight for G channel, 0~255
	UINT32 irsub_b_weight;  ///< IR weight for B channel, 0~255
	UINT32 la_ob_ofs;
	BOOL lath_enable;
	UINT8  lathy1lower;
	UINT8  lathy1upper;
	UINT8  lathy2lower;
	UINT8  lathy2upper;
	UINT16 rang_shift;		///< right shift bit when companding is disable, 0~8, 538 not supported, 539A supported
} KDRV_SIE_LA_PARAM;

/**
    type for KDRV_SIE_GET_LA_RST
    la result, array size should be window num_x * num_y
    histogram array size = 64
    user need to prepare memory for these pointer
*/
typedef struct {
	ULONG src_addr;
	UINT16 *buf_lum_1;      //[KDRV_SIE_LA_MAX_WINNUM * KDRV_SIE_LA_MAX_WINNUM];  // pre gamma result
	UINT16 *buf_lum_2;      //[KDRV_SIE_LA_MAX_WINNUM * KDRV_SIE_LA_MAX_WINNUM];  // post-gamma result
	UINT16 *buf_histogram;  //[KDRV_SIE_LA_HIST_BIN];
} KDRV_SIE_LA_RST;

typedef struct {
	BOOL enable;
} KDRV_SIE_ROI_ACC_PARAM;

typedef struct {
    UINT32  *buf_acc_rslt;      ///< pointer to result buffer, 5x5 window accumulation result, 32bit counter for each window
} KDRV_SIE_ROI_ACC_RST;

/**
    type for KDRV_SIE_GET_MD_RST
    md_th_rslt, array size should be 32x32x8bit
    user need to prepare memory for these pointer
*/
typedef struct {
	UINT8       *md_th_rslt;
	UINT16      blk_dif_cnt;
	UINT32      total_blk_diff;
} KDRV_SIE_MD_RST;

/**
	type for KDRV_SIE_SET_CCIR
*/
typedef struct {
	KDRV_SIE_YUV_ORDER yuv_order;
	KDRV_SIE_DVI_FORMAT_SEL fmt;
	KDRV_SIE_DVI_IN_MODE_SEL dvi_mode;
	BOOL filed_enable;				///< for CCIR interlaced
	BOOL filed_sel;					///< select data for CCIR interlaced
	BOOL ccir656_vd_sel;			///< 656 vd mode, 0 --> interlaced(field change as VD), 1 --> progressive(SAV + V Blank status)
#if 0
	BOOL auto_align;				///< CCIR656 mux sensor data auto alignment
	UINT8 data_period;				///< for mux sensor, 0 --> 1 input(YUYV), 1 --> 2 input(YYUUYYVV)
	UINT8 data_idx;					///< select data idx 0/1 when mux sensor input
#endif
} KDRV_SIE_CCIR_INFO;

/**
    KDRV_SIE_SET_ENCODE

    Only control Enable/Disable
    Enable will cause ch5 output sideinfo, and Channel lineoffset can set to 70%
    note that encode can only enable when data_fmt = KDRV_SIE_BAYER_12

    Channel 5 Setting
    lineoffset = align_ceil_32(scl_size.w) * 16/8
    buffer_size = lineoffset * scl_size.h
*/

typedef enum {
	KDRV_SIE_ENC_50 = 0,
	KDRV_SIE_ENC_58,
	KDRV_SIE_ENC_MAX,
	ENUM_DUMMY4WORD(KDRV_SIE_ENC_RATE_SEL)
} KDRV_SIE_ENC_RATE_SEL;

typedef struct {
	BOOL enable;
	BOOL aggres_mode_en;
	KDRV_SIE_ENC_RATE_SEL enc_rate;
} KDRV_SIE_RAW_ENCODE;
/**
    KDRV_SIE_SET_DGAIN

    digital gain in 3.7 bits format
*/

typedef struct {
	BOOL enable;        // digital gain enable
	BOOL enable_stcs;   // digital gain eanble for statistics path , only valid when enable=1
	UINT32 gain;
	UINT32 gain_stcs;
} KDRV_SIE_DGAIN;

/**
    KDRV_SIE_SET_CGAIN: deprecated after 539A
*/
typedef struct {
	BOOL enable;
	BOOL sel_37_fmt;    // 0 --> 2.8 bit gain format, 1 --> 3.7 bit gain format
	UINT16 r_gain;      // r/gr/gb/b/Ir
	UINT16 gr_gain;
	UINT16 gb_gain;
	UINT16 b_gain;
	UINT16 ir_gain;
	//UINT16 gr_gain;
	//UINT16 gb_gain;
	//UINT16 b_gain;
} KDRV_SIE_CGAIN;

/**
    KDRV_SIE_SET_DPC
*/
typedef enum {
	KDRV_SIE_50F00_PERCENT = 0,     ///< cross-channel weighting = 50% for defect concealment
	KDRV_SIE_25F00_PERCENT,         ///< cross-channel weighting = 25% for defect concealment
	KDRV_SIE_12F50_PERCENT,         ///< cross-channel weighting = 12.5% for defect concealment
	KDRV_SIE_6F25_PERCENT,          ///< cross-channel weighting = 6.25% for defect concealment
	ENUM_DUMMY4WORD(KDRV_SIE_DPC_WEIGTH)
} KDRV_SIE_DPC_WEIGTH;

typedef enum {
    KDRV_SIE_DP_MODE_NORMAL = 0,        ///< normal mode for DPC
    KDRV_SIE_DP_MODE_DEBUG,             ///< debug  mode for DPC data replace
    KDRV_SIE_DP_MODE_MAX,
    ENUM_DUMMY4WORD(KDRV_SIE_DP_MODE_SEL)
} KDRV_SIE_DP_MODE_SEL;

typedef struct {
	BOOL enable;                            ///< dpc function enable
    KDRV_SIE_DP_MODE_SEL mode;
	ULONG tbl_addr_va;                      ///< dpc table virtual addr
	ULONG tbl_addr_pa;                      ///< dpc table physical addr
	KDRV_SIE_DPC_WEIGTH weight;             ///< dpc weigth select
	UINT32 dp_total_size;                   //defect pixel total size
    UINT16 dbg_value;      					///< DPC replace value when mode = DEF_MODE_DEBUG
	BOOL def_same_ch_enb;					///< deffect cross channel weighting, 538 not supported, 539A supported
} KDRV_SIE_DPC;

/**
    KDRV_SIE_SET_ECS
*/
typedef enum {
	KDRV_SIE_ECS_MAP_65x65 = 0,
	KDRV_SIE_ECS_MAP_49x49,
	KDRV_SIE_ECS_MAP_33x33,
	ENUM_DUMMY4WORD(KDRV_SIE_ECS_MAP_SEL)
} KDRV_SIE_ECS_MAP_SEL;

/**
    KDRV_SIE_SET_ECS
*/
typedef enum {
	KDRV_SIE_ECS_3CH_10B = 0,
	KDRV_SIE_ECS_4CH_8B,
	ENUM_DUMMY4WORD(KDRV_SIE_ECS_BAYER_MODE_SEL)
} KDRV_SIE_ECS_BAYER_MODE_SEL;

typedef struct {
	BOOL enable;
	BOOL sel_37_fmt;
	ULONG map_tbl_addr_va;
	ULONG map_tbl_addr_pa;
	KDRV_SIE_ECS_MAP_SEL map_sel;

	BOOL dthr_enable;
	BOOL dthr_reset;
	UINT32 dthr_level;  ///< dithering level; 0x0=bit[1:0], 0x7=bit[8:7]
	KDRV_SIE_ECS_BAYER_MODE_SEL bayer_mode;
} KDRV_SIE_ECS;

typedef struct {
	UINT32 decomp_kpx[KDRV_SIE_DECOMPANDING_KPX_MAX_LEN];   // knee-point X for de-companding
	UINT32 decomp_kpy[KDRV_SIE_DECOMPANDING_KPY_MAX_LEN];   // knee-point Y for de-companding
	UINT32 decomp_gain[KDRV_SIE_DECOMPANDING_GAIN_MAX_LEN]; // gain for de-companding
	UINT32 decomp_sb[KDRV_SIE_DECOMPANDING_SB_MAX_LEN];     // shift-bit for de-companding
} KDRV_SIE_DECOMPANDING_INFO;

typedef struct {
	UINT32 comp_fcurve_l[KDRV_SIE_COMPANDING_FL_MAX_LEN];   // left part of f-curve LUT for companding
	UINT32 comp_fcurve_m[KDRV_SIE_COMPANDING_FM_MAX_LEN];   // middle part of f-curve LUT for companding
	UINT32 comp_fcurve_r[KDRV_SIE_COMPANDING_FR_MAX_LEN];   // right part of f-curve LUT for companding (only comp_fcurve_ev_fmt=1 valid)
	UINT8  comp_fcurve_ev_fmt;                              // format option of f-curve LUT (0 : 16bit mode, 1 : 20bit mode)
} KDRV_SIE_COMPANDING_INFO;

typedef struct {
    BOOL   fcurve_y_en;      ///< bayer scaling must disable when fcurve_y enable
    UINT32 y_gain_max;        ///< 0 ~ 0xfff
    UINT32 y_gain_shift;      ///< 0 ~ 0xf
} KDRV_SIE_COMPANDING_Y_PARAM;

typedef struct {
    UINT8 yweight_lut[KDRV_SIE_COMPANDING_YWEIGHT_MAX_LEN];   ///< 0 ~ 0xff
} KDRV_SIE_COMPANDING_Y_WEIGHT_PARAM;

typedef struct {
	BOOL enable;
	KDRV_SIE_DECOMPANDING_INFO  decomp_info;
	KDRV_SIE_COMPANDING_INFO    comp_info;
	UINT16 comp_shift;
	KDRV_SIE_COMPANDING_Y_PARAM y_param;
	KDRV_SIE_COMPANDING_Y_WEIGHT_PARAM y_weight_param;
} KDRV_SIE_COMPANDING;

typedef struct {
	BOOL singleout_enable[KDRV_SIE_DRAM_CH_MAX];
} KDRV_SIE_SINGLE_OUT_CTRL;

typedef enum {
	KDRV_SIE_NORMAL_OUT = 0,
	KDRV_SIE_SINGLE_OUT,
	ENUM_DUMMY4WORD(KDRV_SIE_OUTPUT_MODE_TYPE)
} KDRV_SIE_OUTPUT_MODE_TYPE;

typedef struct {
	KDRV_SIE_OUTPUT_MODE_TYPE  out_mode[KDRV_SIE_DRAM_CH_MAX];
} KDRV_SIE_DRAM_OUT_CTRL;

typedef struct {
	BOOL enable;
	UINT32 ring_buf_len;
} KDRV_SIE_RINGBUF_INFO;

typedef struct {
	UINT32 VdIntervLowerThr;
	UINT32 VdIntervUpperThr;
	UINT32 HdIntervLowerThr;
	UINT32 HdIntervUpperThr;
} KDRV_SIE_VDHD_INTERVAL_INFO;

typedef struct {
	UINT32 kdrv_sie_ir_level;   ///< Legal range : 0~255
	UINT32 kdrv_sie_ir_sat;     ///< Legal range : 0~1023
} KDRV_SIE_RGBIR_INFO;

typedef enum {
	KDRV_SIE_SPT_BAYER_8         = (1 << KDRV_SIE_BAYER_8),
	KDRV_SIE_SPT_BAYER_10        = (1 << KDRV_SIE_BAYER_10),
	KDRV_SIE_SPT_BAYER_12        = (1 << KDRV_SIE_BAYER_12),
	KDRV_SIE_SPT_BAYER_16        = (1 << KDRV_SIE_BAYER_16),
	KDRV_SIE_SPT_BAYER_16_MSB    = (1 << KDRV_SIE_BAYER_16_MSB),
	KDRV_SIE_SPT_BAYER_16_LSB    = (1 << KDRV_SIE_BAYER_16_LSB),
	KDRV_SIE_SPT_Y_8             = (1 << KDRV_SIE_Y_8),
	KDRV_SIE_DATAFORMAT_SPT_ALL  = (KDRV_SIE_SPT_BAYER_8|KDRV_SIE_SPT_BAYER_10|KDRV_SIE_SPT_BAYER_12|KDRV_SIE_SPT_BAYER_16|KDRV_SIE_SPT_BAYER_16_MSB|KDRV_SIE_SPT_BAYER_16_LSB|KDRV_SIE_SPT_Y_8),
	ENUM_DUMMY4WORD(KDRV_SIE_DATAFORMAT_SPT)
} KDRV_SIE_DATAFORMAT_SPT;

/**
    SIE limitation infor
    sync to CTL_SIE_LIMIT
*/
typedef struct {
	CHAR	func_name[5];
	UINT32 	func; // SIE_FUNCTION_SEL
	UINT32  val;
} KDRV_SIE_LIMIT_INFO;

typedef struct {
	UINT32 max_clk_rate;        //maximum sie clock rate
	UINT32 max_mclk_rate;       //maximum sie mclk rate
	UINT32 max_spt_id;          //maximum sie id
	UINT32 out_max_ch;          //output maximum channel number
	USIZE ca_win_max_num;       //ca window maximum number, proposal[CA_WIN_NUMX & CA_WIN_NUMY]
	USIZE la_win_max_num;       //la window maximum number, proposal[LA_WIN_NUMX & LA_WIN_NUMY]
	UINT32 la_hist_bin_num;     //la histogram maximum bin number, proposal[HISTO_Y_RSLT_0~63]
	UINT32 roi_acc_max_win_num;	//roi acc maximum window number, fixed
	URECT roi_acc_win_align;	//roi acc window align
	URECT roi_acc_max_win;		//roi acc max window size
	USIZE pat_gen_src_win_align; // for src width/height, proposal[SRC_WIDTH & SRC_HEIGHT]
	USIZE pat_gen_src_win_min;
	USIZE pat_gen_src_win_max;
	UINT32 self_bcc_ability;	// support self BCC hardware
	UINT32 bcc_share_src_id;	// share with other SIE
	UINT32 bcc_in_win_w;		// min width when bcc enable

	URECT 				act_win_align;        // active window limit
	UINT32 				act_win_min_w;
	KDRV_SIE_LIMIT_INFO act_win_min_w_info[1];
	UINT32 				act_win_min_h;
	KDRV_SIE_LIMIT_INFO act_win_min_h_info[1];
	USIZE 				act_win_max;

	UINT32 				crp_win_align_x;
	UINT32 				crp_win_align_y;
	UINT32 				crp_win_align_w;
	UINT32 				crp_win_align_h;
	UINT32 				legacy420_crp_win_align_w;
	UINT32 				legacy420_crp_win_align_h;
	KDRV_SIE_LIMIT_INFO crp_win_align_h_info[2];
	USIZE 				crp_win_min;
	USIZE 				crp_win_max[KDRV_SIE_DATA_FMT_MAX];

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
	UINT32 out_lofs_align[KDRV_SIE_DRAM_CH_MAX]; //output lineoofset align, proposal[DRAM_OUT0_OFSO & DRAM_OUT1_OFSO & DRAM_OUT2_OFSO]
	UINT32 out0_lofs_align_h_flip[KDRV_SIE_DATA_FMT_MAX];	//output lineoofset for all bitdepth align when h flip enable
	UINT64 support_func;        //reference to KDRV_SIE_FUNC_SUPPORT
	UINT32 ring_buf_len_max;    //ring buffer length maximum, proposal[DRAM_OUT0_RINGBUF_LEN]
	UINT32 support_output_fmt;  // KDRV_SIE_DATAFORMAT_SPT

	/* active window 2 limit */
	URECT act_win2_align;
	USIZE act_win2_min;
	USIZE act_win2_max[KDRV_SIE_DATA_FMT_MAX];

	/* extraction limit */
	UINT32 extract_ini_len_min; 		// initial len min, units : line
	UINT32 extract_ini_len_max; 		// initial len max, units : line
	UINT32 extract_loop_keep_len_min; 	// loop keep len min, units : line
	UINT32 extract_loop_keep_len_max; 	// loop keep len max, units : line
	UINT32 extract_loop_drop_len_min; 	// loop drop len min, units : line
	UINT32 extract_loop_drop_len_max; 	// loop drop len max, units : line

} KDRV_SIE_LIMIT;

/**
    KDRV SIE all IQ function enable/disable operation for IPL
*/
typedef struct _KDRV_SIE_IPL_FUNC_EN_ {
	UINT32 ipl_ctrl_func;   ///<It labels which function will be controlled by KFLOW
	BOOL   enable;          ///
} KDRV_SIE_IPL_FUNC_EN;

/**
    type for KDRV_SIE_DVS_CODE
*/
typedef struct {
	UINT8 positive;
	UINT8 negative;
	UINT8 nochange;
} KDRV_SIE_DVS_CODE;

/*
    type of KDRV_SIE_EVS_INFO
*/
typedef struct {
	UINT32 max_out_pixel_cnt;	// need private buf = max_out_pixel_cnt * 2(byte) * 2(buf num)
    UINT16 padding_val;         // padding value when output size not 4 byte align, 538 not supported, 539A supported
} KDRV_SIE_EVS_INFO;

/**
    type for KDRV_SIE_VD_DLY
*/
typedef enum {
	KDRV_SIE_VD_DLY_NONE,
	KDRV_SIE_VD_DLY_AUTO,
	KDRV_SIE_VD_DLY_MAX,
	ENUM_DUMMY4WORD(KDRV_SIE_VD_DLY)
} KDRV_SIE_VD_DLY;

/**
    type for KDRV_SIE_MASK
*/
#define KDRV_SIE_MASK_COL_CHK 0xFFF // hw only 12bit

typedef enum {
	KDRV_SIE_MASK_IDX0,
	KDRV_SIE_MASK_IDX1,
	KDRV_SIE_MASK_IDX2,
	KDRV_SIE_MASK_IDX3,
	KDRV_SIE_MASK_IDX_MAX,
	ENUM_DUMMY4WORD(KDRV_SIE_MASK_IDX)
} KDRV_SIE_MASK_IDX;

typedef struct {
	BOOL en;
	URECT win;
	UINT16 r;   // 12bits
	UINT16 g;   // 12bits
	UINT16 b;   // 12bits
	UINT16 ir;  // 12bits
} KDRV_SIE_MASK_INFO;

typedef struct {
	BOOL                dvs_en;                                 // dvs sensor enable
	USIZE               dvs_size;                               // dvs sensor only
} KDRV_SIE_DVS_INFO;

typedef enum {
	KDRV_SID_TSEN_TX_LEVEL_LOW = 0,					///< unused data output level low
	KDRV_SIE_TSEN_TX_LEVEL_HIGH,					///< unused data output level high
	KDRV_SIE_TSEN_TX_LEVEL_MAX,
} KDRV_SIE_TSEN_TX_LEVEL_SEL;

typedef enum {
    KDRV_SIE_TSEN_TX_FS_IDLE_MODE_HILO_END = 0,          ///< frame sync signal go to idle when tx high/low clock cycle end
    KDRV_SIE_TSEN_TX_FS_IDLE_MODE_DATA_END,              ///< frame sync signal go to idle when tx config/stream data end
    KDRV_SIE_TSEN_TX_FS_IDLE_MODE_MAX,
    ENUM_DUMMY4WORD(KDRV_SIE_TSEN_TX_FS_IDLE_MODE_SEL)
} KDRV_SIE_TSEN_TX_FS_IDLE_MODE_SEL;                     ///< 538 not supported, 539A supported

typedef enum {
    KDRV_SIE_TSEN_TX_FS_INV_NONE = 0,                    ///< frame sync signal invert disable
    KDRV_SIE_TSEN_TX_FS_INV_OUT,                         ///< frame sync signal invert output(high/low/delay/idle)
    KDRV_SIE_TSEN_TX_FS_INV_HILO,                        ///< frame sync signal invert high/low only, delay/idle keep low
    KDRV_SIE_TSEN_TX_FS_INV_HILO_OUT,                    ///< frame sync signal invert high/low + output
    KDRV_SIE_TSEN_TX_FS_INV_MAX,
    ENUM_DUMMY4WORD(KDRV_SIE_TSEN_TX_FS_INV_SEL)
} KDRV_SIE_TSEN_TX_FS_INV_SEL;                           ///< 538 not supported, 539A supported

typedef enum {
	KDRV_SIE_TSEN_TX_CFG_MODE_SD0 = 0,			///< output port 1-bit, SD0 => TX_CONFIG_DATA bit0~511
	KDRV_SIE_TSEN_TX_CFG_MODE_SD0_1,			///< output port 2-bit, SD0 => TX_CONFIG_DATA bit0~255, SD1 => TX_CONFIG_DATA bit256~511
	KDRV_SIE_TSEN_TX_CFG_MODE_SD0_2,			///< output port 3-bit, SD0 => TX_CONFIG_DATA bit0~127, SD1 => TX_CONFIG_DATA bit128~255, SD2 => TX_CONFIG_DATA bit256~383
	KDRV_SIE_TSEN_TX_CFG_MODE_SD0_3,			///< output port 7-bit, SD0 => TX_CONFIG_DATA bit0~127, SD1 => TX_CONFIG_DATA bit128~255, SD2 => TX_CONFIG_DATA bit256~383, SD3 => TX_CONFIG_DATA bit384~511
    KDRV_SIE_TSEN_TX_CFG_MODE_SD0_5_M1,			///< output port 6-bit, SD0 => TX_CONFIG_DATA bit0~95,  SD1 => TX_CONFIG_DATA bit96~191,  SD2 => TX_CONFIG_DATA bit192~287, SD3 => TX_CONFIG_DATA bit288~383, SD4 => TX_CONFIG_DATA bit384~479, SD5 => TX_CONFIG_DATA bit480~575, 538 not supported, 539A supported
    KDRV_SIE_TSEN_TX_CFG_MODE_SD0_M2,			///< output port 1-bit, SD0 => TX_CONFIG_DATA bit0~639, 538 not supported, 539A supported
    KDRV_SIE_TSEN_TX_CFG_MODE_SD0_5_M2,			///< output port 6-bit, SD0 => TX_CONFIG_DATA bit0~319, SD1 => TX_CONFIG_DATA bit320~383, SD2 => TX_CONFIG_DATA bit384~447, SD3 => TX_CONFIG_DATA bit448~511, SD4 => TX_CONFIG_DATA bit512~575, SD5 => TX_CONFIG_DATA bit576~639, 538 not supported, 539A supported
	KDRV_SIE_TSEN_TX_CFG_MODE_MAX,
} KDRV_SIE_TSEN_TX_CFG_MODE;

typedef enum {
	KDRV_SIE_TSEN_TX_SYNC_CODE_MODE_SD0 = 0,		///< output port 1-bit
	KDRV_SIE_TSEN_TX_SYNC_CODE_MODE_SD0_1,		///< output port 2-bit
	KDRV_SIE_TSEN_TX_SYNC_CODE_MODE_SD0_2,		///< output port 3-bit
	KDRV_SIE_TSEN_TX_SYNC_CODE_MODE_SD0_6,		///< output port 7-bit
	KDRV_SIE_TSEN_TX_SYNC_CODE_MODE_MAX,
} KDRV_SIE_TSEN_TX_SYNC_CODE_MODE;

typedef enum {
	KDRV_SIE_TSEN_RX_DECODE_MODE_DVP_8BIT = 0,	///< PCLK + VD + HD + D0~7
	KDRV_SIE_TSEN_RX_DECODE_MODE_DVP_14BIT,		///< PCLK + VD + HD + D0~13
	KDRV_SIE_TSEN_RX_DECODE_MODE_SYNC_CODE,		///< base on sync code setting
	KDRV_SIE_TSEN_RX_DECODE_MODE_TX_FS_LS_4BIT,	///< base on OOC TX FS/LS loopback signal with FS/LS_Delay setting, PCLK + D0~3
	KDRV_SIE_TSEN_RX_DECODE_MODE_MAX,
} KDRV_SIE_TSEN_RX_DECODE_MODE;

typedef enum {
	KDRV_SIE_TSEN_RX_OUT_FMT_MSB = 0,	///< output data for raw 14bit from buffer[15:0] MSB => [15:2]
	KDRV_SIE_TSEN_RX_OUT_FMT_LSB,		///< output data for raw 14bit from buffer[15:0] LSB => [13:0]
	KDRV_SIE_TSEN_RX_OUT_FMT_MAX,
} KDRV_SIE_TSEN_RX_OUT_FMT;

typedef enum {
	KDRV_SIE_TSEN_TX_OOC_MODE_SD0 = 0,			///< output port 1-bit
	KDRV_SIE_TSEN_TX_OOC_MODE_SD0_1_2PXL_M1,		///< output port 2-bit, 2-pixel mode-1, SD0=>Pixel(N,0), SD1=>Pixel(N,1)
	KDRV_SIE_TSEN_TX_OOC_MODE_SD0_1_2PXL_M2,		///< output port 2-bit, 2-pixel mode-2, SD0=>Pixel(N,1), SD1=>Pixel(N,0)
	KDRV_SIE_TSEN_TX_OOC_MODE_SD0_1,				///< output port 2-bit, 1-pixel mode
	KDRV_SIE_TSEN_TX_OOC_MODE_SD0_2_M1,				///< output port 3-bit, 1-pixel mode-1, SD[0 1 2]=>T0:[0 1 2] T1:[3 4 5]
	KDRV_SIE_TSEN_TX_OOC_MODE_SD0_6,				///< output port 7-bit
    KDRV_SIE_TSEN_TX_OOC_MODE_SD0_2_M2,				///< output port 3-bit, 1-pixel mode-2, SD[0 1 2]=>T0:[2 1 0] T1:[5 4 3], 538 not supported, 539A supported
	KDRV_SIE_TSEN_TX_OOC_MODE_MAX,
} KDRV_SIE_TSEN_TX_OOC_MODE_SEL;

typedef enum {
	KDRV_SIE_TSEN_TX_OOC_BITDEPTH_6 = 0,
	KDRV_SIE_TSEN_TX_OOC_BITDEPTH_7,
	KDRV_SIE_TSEN_TX_OOC_BITDEPTH_8,
	KDRV_SIE_TSEN_TX_OOC_BITDEPTH_MAX,
} KDRV_SIE_TSEN_TX_OOC_BITDEPTH_SEL;

typedef struct {
    UINT32  tx_dly_cnt;                             ///< clock count of delay for frame sync signal,      from 0 ~ 0xffff
    UINT32  tx_high_cnt;                            ///< clock count of level high for frame sync signal, from 1 ~ 0xffffffff
    UINT32  tx_low_cnt;                             ///< clock count of level low  for frame sync signal, from 1 ~ 0xffffffff
} KDRV_SIE_TSEN_TX_FS_CLKCNT;                       ///< 538 not supported, 539A supported, SD6 as FS output pin when enable tsen_tx_fs

#define KDRV_SIE_TSEN_CFG_NUM 		20
#define KDRV_SIE_TSEN_LVL_NUM 		7
#define KDRV_SIE_TSEN_FSCODE_NUM 	8
#define KDRV_SIE_TSEN_LSCODE_NUM 	8
#define KDRV_SIE_TSEN_BNK_NUM 		3
typedef struct {
	BOOL                		thermal_en;						///< thermal sensor enable
	KDRV_SIE_TSEN_TX_CFG_MODE 	tx_mode;						///< config data output port mode
	UINT16 tx_len;												///< from 4 ~ 512 clock cycle
	UINT32 tx_data[KDRV_SIE_TSEN_CFG_NUM];						///< config data, 538 max up to 512bit, 539A max up to 640bit
    UINT32 tx_period;       									///< config data output period, clock cycle, 538 not supported, 539A supported, from 0 ~ 31 => means 1~32 clock cycle output 1 data
    UINT32 tx_blanking[2];  									///< blanking clock cycle for tx config,     538 not supported, 539A supported, | blanking[0] | tx_config | blanking[1] |, from 0 ~ 0xffff
	KDRV_SIE_TSEN_TX_LEVEL_SEL tx_lvl[KDRV_SIE_TSEN_LVL_NUM];	///< SD#0~6 output level

	KDRV_SIE_TSEN_TX_SYNC_CODE_MODE sync_mode;					///< sync_code data output port mode
	UINT16 fs_code_len;											///< FS code length, 4 ~ 32 clock cycle
	UINT16 ls_code_len;											///< LS code length, 4 ~ 32 clock cycle
	UINT32 fs_code[KDRV_SIE_TSEN_FSCODE_NUM];					///< FS code data
	UINT32 ls_code[KDRV_SIE_TSEN_LSCODE_NUM];					///< LS code data

	UINT16 fs_delay;											///< frame start delay cycle when rx_mode=CTL_SEN_THERMAL_RX_DECODE_MODE_TX_FS_LS
	UINT16 ls_delay;											///< line  start delay cycle when rx_mode=CTL_SEN_THERMAL_RX_DECODE_MODE_TX_FS_LS

	BOOL tx_combine_en;											//538 not supported, 539A supported

	/* FS output signal CFG */
	BOOL tx_fs_en;												//538 not supported, 539A supported
	KDRV_SIE_TSEN_TX_FS_IDLE_MODE_SEL	fs_idle_mode_sel;		//538 not supported, 539A supported
	KDRV_SIE_TSEN_TX_FS_INV_SEL			fs_inv_sel;				//538 not supported, 539A supported
	KDRV_SIE_TSEN_TX_FS_CLKCNT			fs_clk_cnt;				//538 not supported, 539A supported

	KDRV_SIE_TSEN_TX_OOC_MODE_SEL ooc_mode;						///< OOC data output port mode
	KDRV_SIE_TSEN_TX_OOC_BITDEPTH_SEL ooc_bitdepth;				///< support on CTL_SEN_THERMAL_TX_OOC_MODE_SD0_1_2PXL_M1 and CTL_SEN_THERMAL_TX_OOC_MODE_SD0_1_2PXL_M2
	BOOL ooc_data_swap;											///< data swap, none(MSB->LSB), do-swap(LSB->MSB)
	UINT8 ooc_data_r_shift;										///< data right-shift 0 ~ 7
	UINT8 ooc_err_det_code;										///< OOC illegal code detection
	UINT8 ooc_err_rep_code;										///< OOC replace code for illegal data
	UINT16 ooc_width;											///< OOC frame width
	UINT16 ooc_height;											///< OOC frame height
	UINT8 ooc_dummy_top_cnt;									///< OOC frame dummy top    line count
	UINT8 ooc_dummy_bot_cnt;									///< OOC frame dummy bottom line count
	UINT8 ooc_dummy_val;
	UINT16 ooc_vblanking[KDRV_SIE_TSEN_BNK_NUM];				///< OOC frame vertical   blanking0,1,2, unit: clock cycle
	UINT16 ooc_hblanking[KDRV_SIE_TSEN_BNK_NUM];				///< OOC frame horizontal blanking0,1,2, unit: clock cycle

	KDRV_SIE_TSEN_RX_DECODE_MODE rx_mode;						///< receive frame data mode
	KDRV_SIE_TSEN_RX_OUT_FMT 	out_fmt;						///< receive frame data output format
	BOOL 	hi_byte_inv;										///< [15:8][7:0] => [8:15][7:0]
	BOOL 	lo_byte_inv;										///< [15:8][7:0] => [15:8][0:7]
	UINT8 	hi_byte_r_shift;									///< [15:8][7:0] => 2bit shift, [15:10][0][0][7:0]
	UINT8 	lo_byte_r_shift;									///< [15:8][7:0] => 2bit shift, [15 :8][7:2][0][0]
	BOOL 	two_byte_swap;										///< [15:8][7:0] => [7:0][15:8]
	BOOL 	two_byte_inv;										///< [15:0]      => [0:15]
	UINT8 	two_byte_r_shift;									///< [15:0]      => 3bit shift, [15:3][0][0][0]
} KDRV_SIE_TSEN_INFO;

typedef struct {
	UINT32 tx_len;							///< from 4 ~ 512 clock cycle
	UINT32 tx_data[KDRV_SIE_TSEN_CFG_NUM];	///< config data
} KDRV_SIE_TSEN_TX_CFG;

typedef struct {
	KDRV_SIE_MASK_INFO info[KDRV_SIE_MASK_IDX_MAX];
} KDRV_SIE_MASK;


/**
    type for KDRV_SIE_ITEM_EXTDATA

case A : KDRV_SIE_EXTDATA_LOOP_FIRST_DROP
	 -------------------------------------------
	|											|
    |		* initial 							|
    |			- ini_sel : drop or keep		|
    |			- ini_len : line num			|
	 -------------------------------------------
    |								^			|
    |								|			|
	|		* drop (1)			loop_drop_len	|
    |								|			|
    |								v			|
	 -------------------------------------------
    |								^			|
    |								|			|
	|		* keep (1)			loop_keep_len	|
    |								|			|
    |								v			|
	 -------------------------------------------
    |								^			|
    |								|			|
	|		* drop (2)			loop_drop_len	|
    |								|			|
    |								v			|
	 -------------------------------------------
    |								^			|
    |								|			|
	|		* keep (2)			loop_keep_len	|
    |								|			|
    |								v			|
	 -------------------------------------------
	.			.								.
	.			.								.
	.			.								.
	.			.								.
	 -------------------------------------------
    |								^			|
    |								|			|
	|		* drop (N)			loop_drop_len	|
    |								|			|
    |								v			|
	 -------------------------------------------
    |								^			|
    |								|			|
	|		* keep (N)			loop_keep_len	|
    |								|			|
    |								v			|
	 -------------------------------------------
	.			.								.
	--------------------------------------------........> decide by act/act2 window h

case B : KDRV_SIE_EXTDATA_LOOP_FIRST_KEEP
	 -------------------------------------------
	|											|
    |		* initial 							|
    |			- ini_sel : drop or keep		|
    |			- ini_len : line num			|
	 -------------------------------------------
    |								^			|
    |								|			|
	|		* keep(1)			loop_keep_len	|
    |								|			|
    |								v			|
	 -------------------------------------------
    |								^			|
    |								|			|
	|		* drop (1)			loop_drop_len	|
    |								|			|
    |								v			|
	 -------------------------------------------
    |								^			|
    |								|			|
	|		* keep (2)			loop_keep_len	|
    |								|			|
    |								v			|
	 -------------------------------------------
    |								^			|
    |								|			|
	|		* drop (2)			loop_drop_len	|
    |								|			|
    |								v			|
	 -------------------------------------------
	.			.								.
	.			.								.
	.			.								.
	.			.								.
	 -------------------------------------------
    |								^			|
    |								|			|
	|		* keep (N)			loop_keep_len	|
    |								|			|
    |								v			|
	 -------------------------------------------
    |								^			|
    |								|			|
	|		* drop (N)			loop_drop_len	|
    |								|			|
    |								v			|
	 -------------------------------------------
	.			.								.
	--------------------------------------------........> decide by act/act2 window h

*/

typedef enum {
	KDRV_SIE_EXTDATA_INI_DROP,	// drop initial line (drop line number : ini_len)
	KDRV_SIE_EXTDATA_INI_KEEP,	// keep initial line (keep line number : ini_len)
	KDRV_SIE_EXTDATA_INI_MAX,
	ENUM_DUMMY4WORD(KDRV_SIE_EXTDATA_INI)
} KDRV_SIE_EXTDATA_INI;

typedef enum {
	KDRV_SIE_EXTDATA_LOOP_FIRST_DROP,	// loop drop first (initial -> drop -> keep -> drop -> keep -> ... -> drop -> keep)
	KDRV_SIE_EXTDATA_LOOP_FIRST_KEEP,	// loop keep first (initial -> keep -> drop -> keep -> drop -> ... -> keep -> drop)
	KDRV_SIE_EXTDATA_LOOP_FIRST_MAX,
	ENUM_DUMMY4WORD(KDRV_SIE_EXTDATA_LOOP)
} KDRV_SIE_EXTDATA_LOOP;

typedef struct {
	/* extraction enable */
	BOOL					enable;			// enable intraction

	/* initial data information */
	KDRV_SIE_EXTDATA_INI	ini_sel;		// initial select
	UINT32 					ini_len;		// initial len, units : line

	/* loop data information */
	KDRV_SIE_EXTDATA_LOOP	loop_sel;		// loop select
	UINT32					loop_drop_len;	// loop drop len, units : line
	UINT32					loop_keep_len;	// loop keep len, units : line
} KDRV_SIE_EXTDATA_INFO;

typedef struct {
	KDRV_SIE_EXTDATA_INFO extraction1; 	// the extracted data will be output to act
	KDRV_SIE_EXTDATA_INFO extraction2;	// the extracted data will be output to act2
} KDRV_SIE_EXTDATA;

/**
    KDRV_SIE_SYS_INFO
    vd_cnt_clk / vd_cnt_clk_base = vd time (s)
    hd_cnt_clk / hd_cnt_clk_base = hd time (s)
*/
typedef struct {
	/* vd */
	UINT32 vd_cnt_clk_base;     // units : Hz
	UINT32 vd_cnt_clk;          // units : vd_cnt_clk_base (apb clk) [0x1e0]
	UINT32 vd_cnt_pxl;          // units : pixel [0x368]

	/* hd */
	UINT32 hd_cnt_clk_base;     // units : Hz
	UINT32 hd_cnt_clk;          // units : hd_cnt_clk_base (apb clk) [0x1e4]
	UINT32 hd_cnt_line;         // units : line [0x364,bit[15..0]], update by next vd
	UINT32 line_end_cnt;	    // units : line, update by next vd
	UINT32 hd_cur_cnt_line;		// units : line update by each hd

} KDRV_SIE_SYS_INFO;

typedef struct {
	ULONG va;  // virtual address
	ULONG pa;  // physical address
} KDRV_SIE_ADDR;

typedef struct {
	KDRV_SIE_ADDR addr;
	UINT32 size; // for flush info
} KDRV_SIE_ADDR_INFO;

typedef enum {
    IN_CH3_DATA_PACKBUS_6 = 0,     ///< 6 bits packing
    IN_CH3_DATA_PACKBUS_7,         ///< 7 bits packing
    IN_CH3_DATA_PACKBUS_8,         ///< 8 bits packing
    IN_CH3_DATA_PACKBUS_MAX,
    ENUM_DUMMY4WORD(KDRV_SIE_IN_CH3_DATA_PACKBUS_SEL)
} KDRV_SIE_IN_CH3_DATA_PACKBUS_SEL;

typedef struct {
	KDRV_SIE_ADDR addr;
	UINT32 lofs;
	KDRV_SIE_IN_CH3_DATA_PACKBUS_SEL pack_bus_sel;
} KDRV_SIE_IN_CH3_INFO;

typedef struct {
	BOOL act;	// active window
	BOOL act2;	// active window 2
} KDRV_SIE_DMA_OUT_EN;

typedef enum {
	KDRV_SIE_BP_1 = 0x1,
	KDRV_SIE_BP_2 = 0x2,
	KDRV_SIE_BP_3 = 0x4,
	ENUM_DUMMY4WORD(KDRV_SIE_BP_IDX)
} KDRV_SIE_BP_IDX;

typedef struct {
	KDRV_SIE_BP_IDX bp_idx;
	UINT32 bp1_value;
	UINT32 bp2_value;
	UINT32 bp3_value;
} KDRV_SIE_BP_PARAM;

/**
    data for kdrv_sie Set/Get Item
*/
typedef enum {
	KDRV_SIE_ITEM_OPENCFG = 0,  ///< data_type: [Set]     KDRV_SIE_OPENCFG, it can be used before kdrv_sie_open
	KDRV_SIE_ITEM_ISRCB,        ///< data_type: [Set]     KDRV_SIE_ISRCB,[Get]N.S.
	KDRV_SIE_ITEM_MCLK,         ///< data_type: [Set/Get] KDRV_SIE_MCLK_INFO, it can be used before kdrv_sie_open
	KDRV_SIE_ITEM_PXCLK,        ///< data_type: [Set/Get] KDRV_SIE_PXCLKSRC_SEL
	KDRV_SIE_ITEM_SIECLK,       ///< data_type: [Set/Get] KDRV_SIE_CLK_INFO, it can be used before kdrv_sie_open
	KDRV_SIE_ITEM_ACT_WIN,      ///< data_type: [Set/Get] KDRV_SIE_ACT_CRP_WIN
	KDRV_SIE_ITEM_CROP_WIN,     ///< data_type: [Set/Get] KDRV_SIE_ACT_CRP_WIN
	KDRV_SIE_ITEM_OUT_DEST,     ///< data_type: [Set/Get] KDRV_SIE_OUT_DEST
	KDRV_SIE_ITEM_SERIAL_PARAM, ///< data_type: [Set/Get] KDRV_SIE_SERIAL_RAW_BITDEPTH, for sie serial input bitdepth, 538 not supported, 539A supported
	KDRV_SIE_ITEM_DATA_FMT,     ///< data_type: [Set/Get] KDRV_SIE_DATA_FMT
	KDRV_SIE_ITEM_YUV_ORDER,	///< data_type: [Set/Get] KDRV_SIE_YUV_ORDER
	KDRV_SIE_ITEM_SIGNAL,       ///< data_type: [Set/Get] KDRV_SIE_SIGNAL
	KDRV_SIE_ITEM_FLIP,         ///< data_type: [Set/Get] KDRV_SIE_FLIP
	KDRV_SIE_ITEM_INTE,         ///< data_type: [Set/Get] KDRV_SIE_INT
	KDRV_SIE_ITEM_IN_CH3_INFO,  ///< data_type: [Set/Get] KDRV_SIE_IN_CH3_INFO
	KDRV_SIE_ITEM_CH0_LOF,      ///< data_type: [Set/Get] UINT32
	KDRV_SIE_ITEM_CH1_LOF,      ///< data_type: [Set/Get] UINT32
	KDRV_SIE_ITEM_CH2_LOF,      ///< data_type: [Set/Get] UINT32
	KDRV_SIE_ITEM_CH0_ADDR,     ///< data_type: [Set/Get] KDRV_SIE_ADDR_INFO
	KDRV_SIE_ITEM_CH1_ADDR,     ///< data_type: [Set/Get] KDRV_SIE_ADDR_INFO
	KDRV_SIE_ITEM_CH2_ADDR,     ///< data_type: [Set/Get] KDRV_SIE_ADDR_INFO
	KDRV_SIE_ITEM_CHDBG_ADDR,   ///< data_type: [Set/Get] KDRV_SIE_ADDR_INFO
	KDRV_SIE_ITEM_PATGEN_INFO,  ///< data_type: [Set/Get] KDRV_SIE_PATGEN_INFO
	KDRV_SIE_ITEM_SCALEOUT,     ///< data_type: [Set/Get] USIZE
	KDRV_SIE_ITEM_OB,           ///< data_type: [Set/Get] KDRV_SIE_OB_PARAM
	KDRV_SIE_ITEM_CA,           ///< data_type: [Set/Get] KDRV_SIE_CA_PARAM
	KDRV_SIE_ITEM_CA_ROI,       ///< data_type: [Set/Get] KDRV_SIE_CA_ROI
	KDRV_SIE_ITEM_CA_RSLT,      ///< data_type:     [Get] KDRV_SIE_CA_RST
	KDRV_SIE_ITEM_LA,           ///< data_type: [Set/Get] KDRV_SIE_LA_PARAM
	KDRV_SIE_ITEM_LA_ROI,       ///< data_type: [Set/Get] KDRV_SIE_LA_ROI
	KDRV_SIE_ITEM_LA_RSLT,      ///< data_type:     [Get] KDRV_SIE_LA_RST
	KDRV_SIE_ITEM_ROI_ACC,  	///< data_type: [Set/Get] KDRV_SIE_ROI_ACC_PARAM
	KDRV_SIE_ITEM_ROI_ACC_ROI,  ///< data_type: [Set/Get] KDRV_SIE_ROI_ACC
	KDRV_SIE_ITEM_ROI_ACC_RSLT,	///< data_type:     [Get] KDRV_SIE_ROI_ACC_RST
	KDRV_SIE_ITEM_ENCODE,       ///< data_type: [Set/Get] KDRV_SIE_RAW_ENCODE
	KDRV_SIE_ITEM_DGAIN,        ///< data_type: [Set/Get] KDRV_SIE_DGAIN
	KDRV_SIE_ITEM_CGAIN,        ///< data_type: [Set/Get] KDRV_SIE_CGAIN
	KDRV_SIE_ITEM_DPC,          ///< data_type: [Set/Get] KDRV_SIE_DPC
	KDRV_SIE_ITEM_ECS,          ///< data_type: [Set/Get] KDRV_SIE_ECS
	KDRV_SIE_ITEM_CCIR,			///< data_type: [Set/Get] KDRV_SIE_CCIR_INFO
	KDRV_SIE_ITEM_LOAD,         ///< data_type: [Set]     NULL
	KDRV_SIE_ITEM_DMA_OUT_EN,   ///< data_type: [Set/Get] KDRV_SIE_DMA_OUT_EN
	KDRV_SIE_ITEM_BP,           ///< data_type: [Set/Get] KDRV_SIE_BP_PARAM
	KDRV_SIE_ITEM_LIMIT,        ///< data_type:     [Get] KDRV_SIE_LIMIT, return sie size align limitation, it can be used before kdrv_sie_open
	KDRV_SIE_ITEM_COMPANDING,   ///< data_type: [Set/Get] KDRV_SIE_COMPANDING
	KDRV_SIE_ITEM_SINGLEOUT,    ///< data_type: [Set/Get] KDRV_SIE_SINGLE_OUT_CTRL, kdrv will return current status of single-out enable(getting from register)
	KDRV_SIE_ITEM_OUTPUT_MODE,  ///< data_type: [Set/Get] KDRV_SIE_DRAM_OUT_CTRL
	KDRV_SIE_ITEM_RING_BUF,     ///< data_type: [Set/Get] KDRV_SIE_RINGBUF_INFO
	KDRV_SIE_ITEM_IR_INFO,      ///< data_type:     [Get] KDRV_SIE_RGBIR_INFO, output Ir-Level and saturation gain for rgbir 4x4 pattern
	KDRV_SIE_ITEM_REF_LOAD_ID,  ///< data_type: [Set]     KDRV_SIE_PROC_ID
	KDRV_SIE_ITEM_DMA_ABORT,    ///< data_type: [Set]     BOOL
	KDRV_SIE_ITEM_DVS_CODE,     ///< data_type: [Set/Get] KDRV_SIE_DVS_CODE, dvs code for dvs data (cannot runtime change, must set before CTL_SIE_TRIG_START)
	KDRV_SIE_ITEM_DVS,       	///< data_type: [Set/Get] KDRV_SIE_DVS_INFO, dvs data dram output size
	KDRV_SIE_ITEM_EVS,     		///< data_type: [Set/Get] KDRV_SIE_EVS, evs data for deam out size (cannot runtime change, must set before CTL_SIE_TRIG_START)
	KDRV_SIE_ITEM_VD_DLY,       ///< data_type: [Set/Get] KDRV_SIE_VD_DLY
	KDRV_SIE_ITEM_SYS_INFO,     ///< data_type:     [Get] KDRV_SIE_SYS_INFO, get sie hw debug information (for debug check only)
	KDRV_SIE_ITEM_MASK,         ///< data_type: [Set/Get] KDRV_SIE_MASK
	KDRV_SIE_ITEM_EXTDATA,      ///< data_type: [Set/Get] KDRV_SIE_EXTDATA
	KDRV_SIE_ITEM_DBG_EN,       ///< data_type: [Set/Get] BOOL
	KDRV_SIE_ITEM_TSEN_INFO,   	///< data_type: [Set/Get] KDRV_SIE_TSEN_INFO
	KDRV_SIE_ITEM_TSEN_TX_CFG,  ///< data_type: [Set]     KDRV_SIE_TSEN_TX_CFG
	KDRV_SIE_PARAM_MAX,
	KDRV_SIE_PARAM_REV = 0x80000000,
	ENUM_DUMMY4WORD(KDRV_SIE_PARAM_ID)
} KDRV_SIE_PARAM_ID;
STATIC_ASSERT((KDRV_SIE_PARAM_MAX &KDRV_SIE_PARAM_REV) == 0);
#define KDRV_SIE_IGN_CHK KDRV_SIE_PARAM_REV //only support set/get function

typedef enum {
	RESERVED_SIE = 0,
	ENUM_DUMMY4WORD(KDRV_SIE_TRIGGER_PARAM)
} KDRV_SIE_TRIGGER_PARAM;

typedef struct {
	INT32(*callback)(VOID *callback_info, VOID *user_data);
	INT32(*reserve_buf)(ULONG phy_addr);
	INT32(*free_buf)(ULONG phy_addr);
} KDRV_SIE_CALLBACK_FUNC;

typedef struct {
	BOOL pxclk_en;
	BOOL wait_end;              ///< wait vd-come, then disable sie_clk
} KDRV_SIE_SUSPEND_PARAM;

typedef struct {
	BOOL pxclk_en;
} KDRV_SIE_RESUME_PARAM;

INT32 kdrv_sie_open(UINT32 chip, UINT32 engine);
INT32 kdrv_sie_close(UINT32 chip, UINT32 engine);
INT32 kdrv_sie_resume(UINT32 dev_id, void *param);
INT32 kdrv_sie_suspend(UINT32 dev_id, void *param);
INT32 kdrv_sie_set(UINT32 dev_id, KDRV_SIE_PARAM_ID parm_id, VOID *param);
INT32 kdrv_sie_get(UINT32 dev_id, KDRV_SIE_PARAM_ID parm_id, VOID *param);
INT32 kdrv_sie_trigger(UINT32 dev_id, KDRV_SIE_TRIGGER_PARAM *rpc_param, KDRV_SIE_CALLBACK_FUNC *cb_func, VOID *user_data);

void kdrv_sie_init(void);
void kdrv_sie_uninit(void);

UINT32 kdrv_sie_buf_query(UINT32 engine_num);
INT32 kdrv_sie_buf_init(ULONG input_addr, UINT32 buf_size);
INT32 kdrv_sie_buf_uninit(void);
INT32 kdrv_sie_get_sie_limit(KDRV_SIE_PROC_ID id, void *data); // KDRV_SIE_LIMIT
INT32 kdrv_sie_dump_fb_info(VOS_FILE fd);
void kdrv_sie_dbg_skip_chk_limit(UINT32 id, BOOL skip);
void kdrv_sie_dump_intrpt_sts(ULONG dump_id_bit, int (*dump)(const char *fmt, ...));
void kdrv_sie_set_err_log_rate(KDRV_SIE_PROC_ID id, UINT32 err_log_rate);

#endif //_KDRV_SIE_H_
