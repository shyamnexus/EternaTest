/*
    This file is the header file that define the API and data type for VIE
    module.

    @file       viep_lib.h
    @ingroup    mIDrvIPPVIE
    @note

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/

#ifndef _VIEP_LIB_H
#define _VIEP_LIB_H

#include "kwrap/type.h"

#define VIE_MASK_NUM 4
//#define VIE_INTERRUPT_STATUS_NEW (1)

/**
    @addtogroup mIIPPVIE
*/
//@{

/**
    @name Define VIE function.
*/
//@{
#define VIE_CH0_EN                      0x00000001 ///< Enable function
#define VIE_CH1_EN                      0x00000002 ///< Enable function
#define VIE_CH2_EN                      0x00000004 ///< Enable function
#define VIE_CH3_EN                      0x00000008 ///< Enable function
#define VIE_CH0_MASK0_EN                0x00000010 ///< Enable function
#define VIE_CH0_MASK1_EN                0x00000020 ///< Enable function
#define VIE_CH0_MASK2_EN                0x00000040 ///< Enable function
#define VIE_CH0_MASK3_EN                0x00000080 ///< Enable function
#define VIE_CH1_MASK0_EN                0x00000100 ///< Enable function
#define VIE_CH1_MASK1_EN                0x00000200 ///< Enable function
#define VIE_CH1_MASK2_EN                0x00000400 ///< Enable function
#define VIE_CH1_MASK3_EN                0x00000800 ///< Enable function
#define VIE_CH2_MASK0_EN                0x00001000 ///< Enable function
#define VIE_CH2_MASK1_EN                0x00002000 ///< Enable function
#define VIE_CH2_MASK2_EN                0x00004000 ///< Enable function
#define VIE_CH2_MASK3_EN                0x00008000 ///< Enable function
#define VIE_CH3_MASK0_EN                0x00010000 ///< Enable function
#define VIE_CH3_MASK1_EN                0x00020000 ///< Enable function
#define VIE_CH3_MASK2_EN                0x00040000 ///< Enable function
#define VIE_CH3_MASK3_EN                0x00080000 ///< Enable function
#define VIE_CH0_YCC_ENC_EN              0x00100000 ///< Enable function
#define VIE_CH1_YCC_ENC_EN              0x00200000 ///< Enable function
#define VIE_CH2_YCC_ENC_EN              0x00400000 ///< Enable function
#define VIE_CH3_YCC_ENC_EN              0x00800000 ///< Enable function
#define VIE_CH0_PATGEN_EN               0x10000000 ///< Enable function
#define VIE_CH1_PATGEN_EN               0x20000000 ///< Enable function
#define VIE_CH2_PATGEN_EN               0x40000000 ///< Enable function
#define VIE_CH3_PATGEN_EN               0x80000000 ///< Enable function
//@}



/**
    @name Define VIE interrput.
*/
//@{
#define VIE_INT_CH0_VD                 0x00000001 ///< Enable interrupt
#define VIE_INT_CH1_VD                 0x00000002 ///< Enable interrupt
#define VIE_INT_CH2_VD                 0x00000004 ///< Enable interrupt
#define VIE_INT_CH3_VD                 0x00000008 ///< Enable interrupt
#define VIE_INT_CH0_BP                 0x00000010 ///< Enable interrupt
#define VIE_INT_CH1_BP                 0x00000020 ///< Enable interrupt
#define VIE_INT_CH2_BP                 0x00000040 ///< Enable interrupt
#define VIE_INT_CH3_BP                 0x00000080 ///< Enable interrupt
#define VIE_INT_CH0_DRAM_OUT0_END      0x00000100 ///< Enable interrupt
#define VIE_INT_CH1_DRAM_OUT0_END      0x00000200 ///< Enable interrupt
#define VIE_INT_CH2_DRAM_OUT0_END      0x00000400 ///< Enable interrupt
#define VIE_INT_CH3_DRAM_OUT0_END      0x00000800 ///< Enable interrupt
#define VIE_INT_CH0_DRAM_OUT1_END      0x00001000 ///< Enable interrupt
#define VIE_INT_CH1_DRAM_OUT1_END      0x00002000 ///< Enable interrupt
#define VIE_INT_CH2_DRAM_OUT1_END      0x00004000 ///< Enable interrupt
#define VIE_INT_CH3_DRAM_OUT1_END      0x00008000 ///< Enable interrupt
#define VIE_INT_CH0_CRPSTR             0x00010000 ///< Enable interrupt
#define VIE_INT_CH1_CRPSTR             0x00020000 ///< Enable interrupt
#define VIE_INT_CH2_CRPSTR             0x00040000 ///< Enable interrupt
#define VIE_INT_CH3_CRPSTR             0x00080000 ///< Enable interrupt
#define VIE_INT_CH0_CRPEND             0x00100000 ///< Enable interrupt
#define VIE_INT_CH1_CRPEND             0x00200000 ///< Enable interrupt
#define VIE_INT_CH2_CRPEND             0x00400000 ///< Enable interrupt
#define VIE_INT_CH3_CRPEND             0x00800000 ///< Enable interrupt
#define VIE_INT_CH0_YCC_OVFL           0x01000000 ///< Enable interrupt
#define VIE_INT_CH1_YCC_OVFL           0x02000000 ///< Enable interrupt
#define VIE_INT_CH2_YCC_OVFL           0x04000000 ///< Enable interrupt
#define VIE_INT_CH3_YCC_OVFL           0x08000000 ///< Enable interrupt
#define VIE_INT_DBGOUT_END             0x10000000 ///< Enable interrupt
#define VIE_INT_LINEBUF_OVFL           0x20000000 ///< Enable interrupt
#define VIE_INT_CCIR656_HEADER_HIT     0x40000000 ///< Enable interrupt
#define VIE_INT_ALL                    0xffffffff ///< Enable interrupt
//@}

#define VIE_INT_CH0 (VIE_INT_CH0_VD|VIE_INT_CH0_BP|VIE_INT_CH0_DRAM_OUT0_END|VIE_INT_CH0_DRAM_OUT1_END|VIE_INT_CH0_CRPSTR| \
	                 VIE_INT_CH0_CRPEND|VIE_INT_CH0_YCC_OVFL|VIE_INT_DBGOUT_END|VIE_INT_LINEBUF_OVFL|VIE_INT_CCIR656_HEADER_HIT)

#define VIE_INT_CH1 (VIE_INT_CH1_VD|VIE_INT_CH1_BP|VIE_INT_CH1_DRAM_OUT0_END|VIE_INT_CH1_DRAM_OUT1_END|VIE_INT_CH1_CRPSTR| \
	                 VIE_INT_CH1_CRPEND|VIE_INT_CH1_YCC_OVFL|VIE_INT_DBGOUT_END|VIE_INT_LINEBUF_OVFL|VIE_INT_CCIR656_HEADER_HIT)	

#define VIE_INT_CH2 (VIE_INT_CH2_VD|VIE_INT_CH2_BP|VIE_INT_CH2_DRAM_OUT0_END|VIE_INT_CH2_DRAM_OUT1_END|VIE_INT_CH2_CRPSTR| \
	                 VIE_INT_CH2_CRPEND|VIE_INT_CH2_YCC_OVFL|VIE_INT_DBGOUT_END|VIE_INT_LINEBUF_OVFL|VIE_INT_CCIR656_HEADER_HIT)

#define VIE_INT_CH3 (VIE_INT_CH3_VD|VIE_INT_CH3_BP|VIE_INT_CH3_DRAM_OUT0_END|VIE_INT_CH3_DRAM_OUT1_END|VIE_INT_CH3_CRPSTR| \
	                 VIE_INT_CH3_CRPEND|VIE_INT_CH3_YCC_OVFL|VIE_INT_DBGOUT_END|VIE_INT_LINEBUF_OVFL|VIE_INT_CCIR656_HEADER_HIT)


typedef enum _VIE_ENGINE_ID {
	VIE_ENGINE_ID_1 = 0,    ///< VIE1
	VIE_ENGINE_ID_2 = 1,    ///< VIE2
	VIE_ENGINE_ID_MAX,
	ENUM_DUMMY4WORD(VIE_ENGINE_ID)
} VIE_ENGINE_ID;


typedef enum _VIE_CHANNEL_ID {
	VIE_CHANNEL_ID_1 = 0,    ///< CH0
	VIE_CHANNEL_ID_2 = 1,    ///< CH1
	VIE_CHANNEL_ID_3 = 2,    ///< CH2
	VIE_CHANNEL_ID_4 = 3,    ///< CH3
	VIE_CHANNEL_ID_MAX,
	ENUM_DUMMY4WORD(VIE_CHANNEL_ID)
} VIE_CHANNEL_ID;


typedef enum {
	VIE_ISR_CB_IMD = 0,   // immediately callback
	VIE_ISR_CB_VDLATCH,   // va-latch callback
} VIE_ISR_CB_TYPE;

/**
VIE Enumeration

@name   VIE_Enumeration
*/
//@{

typedef enum {
	MAIN_IN_PARA_MSTR_SNR = 0,   ///< Parallel Master Sensor
	MAIN_IN_CSI_2,               ///< Serial Sensor from CSI-2
	MAIN_IN_CSI_4,               ///< Serial Sensor from CSI-4
	ENUM_DUMMY4WORD(VIE_MAIN_INPUT_SEL)
} VIE_MAIN_INPUT_SEL;

typedef enum {
	VIE_CLKSMPL_SINGLE  = 0,  ///< clk single-edge sampling of pxclk
	VIE_CLKSMPL_DUAL,         ///< clk dual-edge sampling of pxclk
	ENUM_DUMMY4WORD(VIE_CLK_SAMPLE_SEL)
} VIE_CLK_SAMPLE_SEL;

typedef enum {
	VIE_PHASE_RISING  = 0,  ///< rising edge latch/trigger
	VIE_PHASE_FALLING,      ///< falling edge latch/trigger
	ENUM_DUMMY4WORD(VIE_DATA_PHASE_SEL)
} VIE_DATA_PHASE_SEL;

typedef enum {
	VIE_FUNC_DISABLE  = 0,  ///< Those bit=1 would be set, others remain
	VIE_FUNC_ENABLE,        ///< Those bit=1 would be clear, others remain
	VIE_FUNC_SET,           ///< All bit would be configured as 0/1 ther are
	ENUM_DUMMY4WORD(VIE_FUNC_SWITCH_SEL)
} VIE_FUNC_SWITCH_SEL;

typedef enum {
	BURST_MODE_NORMAL       =  0,  ///< normal
	BURST_MODE_BURST_CAP,          ///< for high-data-rate capture
	BURST_MODE_REMAIN,             ///< for Driver only
	ENUM_DUMMY4WORD(VIE_DRAM_BURST_MODE_SEL)
} VIE_DRAM_BURST_MODE_SEL;


typedef enum {
	VIE_BURST_64W  = 0,   ///< 64  words
	VIE_BURST_48W  = 1,   ///< 48  words
	VIE_BURST_32W  = 2,   ///< 32  words
	VIE_BURST_16W  = 3,   ///< 16  words
	ENUM_DUMMY4WORD(VIE_BURST_SEL)
} VIE_BURST_SEL;

typedef enum {
	DVI_FORMAT_BT601  = 0,     ///< CCIR 601, 8bit
	DVI_FORMAT_BT656,          ///< CCIR 656
	DVI_FORMAT_BT656_2MUX,     ///< CCIR 656 2mux
	DVI_FORMAT_BT656_3MUX,     ///< CCIR 656 3mux
	DVI_FORMAT_BT656_4MUX,     ///< CCIR 656 4mux
	DVI_FORMAT_BT1120,         ///< CCIR 601, 16bit
	DVI_FORMAT_BT1120_16BIT_BT656, ///< BT1120 16bit(IO)
	DVI_FORMAT_BT1120_DUAL_HEADER, ///< BT1120 8bit(IO) with dual header
	ENUM_DUMMY4WORD(VIE_DVI_FORMAT_SEL)
} VIE_DVI_FORMAT_SEL;

typedef enum {
	DVI_IN_YUV422 = 0,
	DVI_IN_YUV420,
	ENUM_DUMMY4WORD(VIE_DVI_IN_FMT)
} VIE_DVI_IN_FMT;

typedef enum {
	DVI_OUT_SWAP_YUYV  = 0,   ///<
	DVI_OUT_SWAP_YVYU,        ///<
	DVI_OUT_SWAP_UYVY,        ///<
	DVI_OUT_SWAP_VYUY,        ///<
	ENUM_DUMMY4WORD(VIE_DVI_OUT_SWAP_SEL)
} VIE_DVI_OUT_SWAP_SEL;


typedef enum {
	DVI_420OUT_SWAP_YYU  = 0,   ///<
	DVI_420OUT_SWAP_YUY,        ///<
	DVI_420OUT_SWAP_UYY,        ///<
	ENUM_DUMMY4WORD(VIE_DVI_420OUT_SWAP_SEL)
} VIE_DVI_420OUT_SWAP_SEL;



typedef enum {
	//PAT_COLORBAR    = 1,    ///< Color bar
	PAT_RANDOM       =2,             ///< Random
	PAT_FIXED,              ///< Fixed
	PAT_HINCREASE,          ///< 1D Increment
	PAT_HVINCREASE,         ///< 2D increment
	ENUM_DUMMY4WORD(VIE_PATGEN_SEL)
} VIE_PATGEN_SEL;

typedef enum {
	VIE_PXCLKSRC_OFF = 0,  ///< VIE pixel-clock disabled
	VIE_PXCLKSRC_PAD_A,    ///< VIE pixel-clock enabled, source as pixel-clock-pad A
	VIE_PXCLKSRC_PAD_B,    ///< VIE pixel-clock enabled, source as pixel-clock-pad B
	VIE_PXCLKSRC_PAD_AB,    ///< VIE pixel-clock enabled, source as pixel-clock-pad A
} VIE_PXCLKSRC;

typedef enum {
	VIE_PADA_SRC_SGPIO1,    ///< VIE, source of pad A
	VIE_PADA_SRC_PGPIO0,    ///< VIE, source of pad A
} VIE_PADA_SRC;


typedef enum {
	DELAY_CHAINA_SRC_SIE,
	DELAY_CHAINA_SRC_VIE,    
} VIE_DELAYCHAINA_SRC;



/**
    VIE Pixel-Clock Source Selection

    Structure of Pixel-Clock source VIE configuration.
*/
//@{
typedef enum {
	VIE_CLKSRC_CURR = 0, ///< VIE clock source as current setting(no setting, no changing)
	VIE_CLKSRC_480,      ///< VIE clock source as 480MHz
	VIE_CLKSRC_192,
	VIE_CLKSRC_320,
	VIE_CLKSRC_PLL19,     ///< VIE clock source as PLL19
	VIE_CLKSRC_PLL23,    ///< VIE clock source as PLL23
	VIE_CLKSRC_PLL5,
} VIE_CLKSRC_SEL;
//@}

/**
    @name SIE enum - Change Funciton Parameter Selection
*/
//@{
typedef enum {
	VIE_CHG_MAIN_IN = 0,
	VIE_CHG_SRC_WIN,                ///< refer to "VIE_SRC_WIN_INFO",       , change souce size
	VIE_CHG_CROP_WIN,	   			///< refer to "VIE_CRP_WIN_INFO"        , change crop size
	VIE_CHG_PAT_GEN,				///< refer to "VIE_PATGEN_INFO"
	VIE_CHG_OUT0_ADDR_FLIP,         ///< change output channel 0 address + Flip
	VIE_CHG_OUT1_ADDR_FLIP,         ///< change output channel 1 address + Flip
	VIE_CHG_OUT0_LOFS,              ///< change output channel 0 lineoffset
	VIE_CHG_OUT1_LOFS,              ///< change output channel 1 lineoffset
	VIE_CHG_DVI,                    ///< refer to "VIE_CHG_DVI_INFO"        , change DVI, not support @NT98530
	VIE_CHG_FLIP,                   ///< refer to "VIE_FLIP_INFO"           , change H/V flip, not support @NT98530
	VIE_CHG_DRAM_BURST,             ///< refer to "VIE_DRAM_BURST_INFO"     , change Dram Burst-length
	VIE_CHG_YCC_ADJ,                ///< refer to "VIE_YCC_ADJ_INFO"        , change YCC
	VIE_CHG_BP,
	VIE_CHG_INTE,
	VIE_CHG_SINGLEOUT,
	VIE_CHG_OUTMODE,
	VIE_CHG_CCIR656_HEADER,   
	VIE_CHG_PATGEN,
	VIE_CHG_MASK,
	VIE_CHG_DBG_OUT_ADDR,
	VIE_CHG_DBGEN,
	VIE_CHG_DELAY_CHAIN,
	ENUM_DUMMY4WORD(VIE_CHANGE_FUN_PARAM_SEL)
} VIE_CHANGE_FUN_PARAM_SEL;
//@}




typedef enum {
	VIE_WAIT_CH0_VD         =  0,  ///<
	VIE_WAIT_CH1_VD,
	VIE_WAIT_CH2_VD,
	VIE_WAIT_CH3_VD,
	VIE_WAIT_CH0_BP,              ///< ch0 break point
	VIE_WAIT_CH1_BP,              ///< ch1 break point
	VIE_WAIT_CH2_BP,              ///< ch2 break point
	VIE_WAIT_CH3_BP,              ///< ch3 break point
	VIE_WAIT_CH0_DO0_END,          ///< ch0 Dram Output-0 END
	VIE_WAIT_CH1_DO0_END,          ///< ch0 Dram Output-0 END
	VIE_WAIT_CH2_DO0_END,          ///< ch0 Dram Output-0 END
	VIE_WAIT_CH3_DO0_END,          ///< ch0 Dram Output-0 END
	VIE_WAIT_CH0_DO1_END,          ///< ch0 Dram Output-1 END
	VIE_WAIT_CH1_DO1_END,          ///< ch0 Dram Output-1 END
	VIE_WAIT_CH2_DO1_END,          ///< ch0 Dram Output-1 END
	VIE_WAIT_CH3_DO1_END,          ///< ch0 Dram Output-1 END
	VIE_WAIT_CH0_CRPSTR,              ///< ch0 crop start
	VIE_WAIT_CH1_CRPSTR,              ///< ch1 crop start
	VIE_WAIT_CH2_CRPSTR,              ///< ch2 crop start
	VIE_WAIT_CH3_CRPSTR,              ///< ch3 crop start
	VIE_WAIT_CH0_CRPEND,              ///< ch0 crop end
	VIE_WAIT_CH1_CRPEND,              ///< ch1 crop end
	VIE_WAIT_CH2_CRPEND,              ///< ch2 crop end
	VIE_WAIT_CH3_CRPEND,              ///< ch3 crop end
	VIE_WAIT_CH0_YCC_OVFL,              ///< ch0 yc compression overflow
	VIE_WAIT_CH1_YCC_OVFL,              ///< ch1 yc compression overflow
	VIE_WAIT_CH2_YCC_OVFL,              ///< ch2 yc compression overflow
	VIE_WAIT_CH3_YCC_OVFL,              ///< ch3 yc compression overflow
	VIE_WAIT_DBGOUT_END,                ///< debug channel end
	VIE_WAIT_LINEBUF_OVFL,              ///< VIE linebuffer overflow
	VIE_WAIT_CCIR656_HEADER_HIT,        ///< ccir656 header hit
	ENUM_DUMMY4WORD(VIE_WAIT_EVENT_SEL)
} VIE_WAIT_EVENT_SEL;

typedef enum {
	VIE_NO_FLIP             =  0,
	VIE_H_FLIP,                    ///< enable selected function bits
	VIE_V_FLIP,                    ///< enable selected function bits
	VIE_HV_FLIP,                   ///< enable selected function bits
	ENUM_DUMMY4WORD(VIE_FLIP_SEL)
} VIE_FLIP_SEL;

typedef enum {
	DRAM_OUT_MODE_NORMAL = 0,
	DRAM_OUT_MODE_SINGLE,
	ENUM_DUMMY4WORD(SIE_DRAM_OUT_MODE)
} VIE_DRAM_OUT_MODE;

typedef struct {
	VIE_DRAM_OUT_MODE  out0mode; ///< out0 mode sel
	VIE_DRAM_OUT_MODE  out1mode; ///< out1 mode sel
	VIE_DRAM_OUT_MODE  dbgoutmode;///< debug out mode sel
} VIE_DRAM_OUT_CTRL;

typedef struct {
	VIE_MAIN_INPUT_SEL  MainInSrc[4];   ///< Main Input Source
	BOOL                FourKMODE[4];   ///< 4k mode enable, Only CH0/1 are available !!! 
	VIE_CLK_SAMPLE_SEL  SrcAClkSmpSel;  ///< clock sample selection
	VIE_DATA_PHASE_SEL  SrcAVdPhase;    ///< VD phase of Source A, only valid for SDR mode
	VIE_DATA_PHASE_SEL  SrcAHdPhase;    ///< HD phase of Source A, only valid for SDR mode
	VIE_DATA_PHASE_SEL  SrcADataPhase;  ///< Data phase of Source A, only valid for SDR mode
	VIE_DATA_PHASE_SEL  SrcAFidledPhase;///< Field phase of Source A, for B601 only
	BOOL                SrcAVdInv;      ///< VD Invert of Source A, for B601 only
	BOOL                SrcAHdInv;      ///< HD Invert of Source A, for BT601 only
	BOOL                SrcAFieldInv;   ///< Field Invert of Source A, for BT601 only
	BOOL                SrcAHiByteInv;  ///< bit swap of MSB 8bit of Source A
	BOOL                SrcALoByteInv;  ///< bit swap of LSB 8bit of Source A
	BOOL                SrcAHi4bitSwap; ///< bit swap of MSB 4bit of Source A
	BOOL                SrcALo4bitSwap; ///< bit swap of LSB 4bit  of Source A
	BOOL                SrcAHiLoByteSwap;///< bit swap between MSB 8bit and LSB 8bit of Source A
	BOOL                SrcAEdgeSwap;   ///< edge swap of Source A
	VIE_CLK_SAMPLE_SEL  SrcBClkSmpSel;  ///< clock sample selection
	//VIE_DATA_PHASE_SEL  SrcBVdPhase;    ///< VD phase of Source B, only valid for SDR mode
	//VIE_DATA_PHASE_SEL  SrcBHdPhase;    ///< HD phase of Source B, only valid for SDR mode
	VIE_DATA_PHASE_SEL  SrcBDataPhase;  ///< Data phase of Source B, only valid for SDR mode
	//VIE_DATA_PHASE_SEL  SrcBFidledPhase;///< Field phase of Source B, for B601 only
	//BOOL                SrcBVdInv;      ///< VD Invert of Source B, for B601 only
	//BOOL                SrcBHdInv;      ///< HD Invert of Source B, for BT601 only
	//BOOL                SrcBFieldInv;   ///< Field Invert of Source B, for BT601 only
	BOOL                SrcBHiByteInv;  ///< bit swap of MSB 8bit of Source B
	BOOL                SrcBLoByteInv;  ///< bit swap of LSB 8bit of Source B
	BOOL                SrcBHi4bitSwap; ///< bit swap of MSB 4bit of Source B
	BOOL                SrcBLo4bitSwap; ///< bit swap of LSB 4bit  of Source B
	BOOL                SrcBHiLoByteSwap;///< bit swap between MSB 8bit and LSB 8bit of Source B
	BOOL                SrcBEdgeSwap;   ///< edge swap of Source B
} VIE_MAIN_INPUT_INFO;

typedef struct {
	UINT32 uiBp1;             ///< break point 1
	UINT32 uiBp2;             ///< break point 2
	UINT32 uiBp3;             ///< break point 3
} VIE_BREAKPOINT_INFO;

typedef struct {
	BOOL        bch0linebufovfl;
	BOOL        bch1linebufovfl;
	BOOL        bch2linebufovfl;
	BOOL        bch3linebufovfl;
} VIE_ENGINE_STATUS_INFO_CB;

typedef struct {
	BOOL        bch0ccir656headerhit;
	BOOL        bch1ccir656headerhit;
	BOOL        bch2ccir656headerhit;
	BOOL        bch3ccir656headerhit;
	BOOL        bch0dbgoutend;
	BOOL        bch1dbgoutend;
	BOOL        bch2dbgoutend;
	BOOL        bch3dbgoutend;
	BOOL        bch0linebufovfl;
	BOOL        bch1linebufovfl;
	BOOL        bch2linebufovfl;
	BOOL        bch3linebufovfl;
} VIE_ENGINE_STATUS_INFO;

typedef struct {
	UINT32      uiSzX; ///< horizontal size
	UINT32      uiSzY; ///< vertical size
} VIE_SRC_WIN_INFO;

typedef struct {
	UINT16      uiStX; ///< horizontal start
	UINT16      uiStY; ///< vertical start
	UINT32      uiSzX; ///< horizontal size
	UINT32      uiSzY; ///< vertical size
} VIE_CRP_WIN_INFO;

typedef struct {
	BOOL             SingleOut0En;
	BOOL             SingleOut1En;
	BOOL             SingleOutDbgEn;
} VIE_DRAM_SINGLE_OUT;

typedef struct _VIE_BUF_ADDR_INFO_ {
	ULONG va;
	ULONG pa;
} VIE_BUF_ADDR_INFO;

// for emulation only
typedef struct {
	ULONG             uiAddr;           ///< address
	UINT32            uiLofs;           ///< line-offset
	BOOL              bMirror;          ///< H flip
	BOOL              bFlip;            ///< V flip
} VIE_DRAM_OUT_INFO;


typedef struct _VIE_BURST_LENGTH {
	VIE_BURST_SEL  BurstLenOut0; ///< 
	VIE_BURST_SEL  BurstLenOut1; ///< 
} VIE_BURST_LENGTH;

typedef struct {
	BOOL            bOut0Mirror;     ///<
	BOOL            bOut0Flip;     ///<
	BOOL            bOut1Mirror;     ///<
	BOOL            bOut1Flip;     ///<
} VIE_FLIP_INFO;

typedef struct _VIE_MASK_INFO{
	UINT16          MaskX0[VIE_MASK_NUM];
	UINT16          MaskY0[VIE_MASK_NUM];
	UINT16          MaskX1[VIE_MASK_NUM];
	UINT16          MaskY1[VIE_MASK_NUM];
	UINT8           MaskColY[VIE_MASK_NUM];
	UINT8           MaskColCb[VIE_MASK_NUM];
	UINT8           MaskColCr[VIE_MASK_NUM];
} VIE_MASK_INFO;

typedef struct {
	VIE_DVI_FORMAT_SEL   DviFormat;
	VIE_DVI_OUT_SWAP_SEL DviOutSwap;
	//BOOL             bHdInSwap;
	BOOL             bOutSplit;
	BOOL             bFieldEn;
	BOOL             bFieldSel;
	BOOL             bCCIR656VdSel;
	UINT8            uiDataPeriod;
	UINT8            uiDataIdx;
	BOOL             bAutoAlign;
	BOOL             bOutYUV420;
	VIE_DVI_420OUT_SWAP_SEL   Dvi420OutSwap; 
	VIE_DVI_IN_FMT   DviInFmt;
	BOOL             SrcSel; // 0: source A, 1: source B
} VIE_DVI_INFO;

typedef struct {
	VIE_PATGEN_SEL  PatGenMode;
	UINT32          uiPatGenVal;
} VIE_PATGEN_INFO;

typedef struct {
	UINT32              uiDctTblIdx[16];
	UINT32              uiDctThr[8];
	BOOL                bShiftEn[4];
} VIE_YCC_PARAM_INFO;


/**
    VIE Open Object.

    Open Object is used for opening VIE module.
    \n Used for sie_open()

*/
//@{
typedef struct _VIE_OPENOBJ {
	void (*pfVieIsrCb)(UINT32 uiIntpStatus, VIE_ENGINE_STATUS_INFO_CB *info); ///< isr callback function
	UINT32 uiVieClockRate;  ///< VIE clock rate in Hz; configure '0' to skip set-up
	VIE_CLKSRC_SEL VieClkSel;   ///< VIE clock source selection
	VIE_PXCLKSRC   PxClkSel;    ///<  VIE pixel-clock source selection; valid in Parallel(Master/Slave) modes, invalid in Serial mode
} VIE_OPENOBJ;
//@}

#if 0
typedef struct {
	VIE_BUF_ADDR_INFO        uiOut0Addr; ///< SIE output ch0 dram addr; "zero value" for null configuration
	VIE_BUF_ADDR_INFO        uiOut1Addr; ///< SIE output ch1 dram addr; "zero value" for null configuration
	UINT32                   uiOut0Size; ///< SIE output ch0 dram size, in bytes, for cache-handle; "zero value" for null configuration
	UINT32                   uiOut1Size; ///< SIE output ch1 dram size, in bytes, for cache-handle; "zero value" for null configuration
} VIE_CHG_OUT_ADDR_INFO;


typedef struct {
	UINT32                   uiOut0Lofs; ///< SIE output ch0 dram line-offset, in bytes; "zero value" for null configuration
	UINT32                   uiOut1Lofs; ///< SIE output ch1 dram line-offset, in bytes; "zero value" for null configuration
} VIE_CHG_IO_LOFS_INFO;
#endif


typedef struct {
	BOOL                    pxclkAIn;         ///< check pixel clock A in,  for parallel sensor debug only
	BOOL                    pxclkBIn;         ///< check pixel clock B in,  for parallel sensor debug only
	BOOL                    vieclkIn;         ///< check vie module clock in
	UINT32                  uiMaxPixelCnt;    ///< Max pixel count
	UINT32                  uiChxMaxLineCnt;     ///< max line count
	UINT32                  uiChxLastFrmHdCnt;   ///< Hd count for last frame, by ch0~3	
	UINT32                  uiChxLastFrmPixCnt;  ///< Pixel count of each line for last frame, by ch0~3 
	UINT32                  uiChxVdpxclkCnt;     ///< VD time, count by apb clock, by ch0~3
	UINT32                  uiChxHdpxclkCnt;     ///< HD time, count by apb clock, by ch0~3
	UINT32                  uiChxVdCnt;          ///< VD count, count start from SW_RST from 1 to 0, by ch0~3
	UINT32                  uiChxCrpEndCnt;      ///< Crop-end count, by ch0~3
} VIE_SYS_DEBUG_INFO;

typedef struct {
	UINT8 srca_delay_val;
	UINT8 srcb_delay_val;
	VIE_DELAYCHAINA_SRC srca_delay_chain_src;
} VIE_DELAYCHAIN_INFO;

#endif//_vIEP_LIB_H
//@}

//@}


