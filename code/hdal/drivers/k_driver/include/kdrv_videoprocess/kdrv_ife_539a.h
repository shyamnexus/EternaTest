/**
    Public header file for dal_ife

    This file is the header file that define the API and data type for dal_ife.

    @file       kdrv_ife.h
    @ingroup    mILibIPLCom
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _KDRV_IFE_H_
#define _KDRV_IFE_H_

#include "kdrv_videoprocess/kdrv_ipp_utility_539a.h"

#define IFE_SSDRV_SUPPORT (1)
#define IFE_538_KDRV_FEATURE (1)
#define IFE_538_KLOFW_SUPPORT (0)
#define IFE_538_KLOFW_REMOVE (1)
#define IFE_538_OLD_HANDLER_SUPPORT (0)

#define IFE_VA_SUPPORT (0)

typedef enum {
	KDRV_IFE_PARAM_CFG_PROCESS_CPU = 0, // [set] cpu_mode job config.               data_dype: KDRV_IFE_JOB_CFG
	KDRV_IFE_PARAM_CFG_PROCESS_LL,      // [set] linklist_mode job config.          data_dype: KDRV_IFE_JOB_CFG
	KDRV_IFE_PARAM_TRIG_SINGLE,         // [set] trigger ife start.                 data_type: NULL
	KDRV_IFE_PARAM_TRIG_LL,             // [set] trigger ife start by linklist.     data_type: UINT32 *
	KDRV_IFE_PARAM_TRIG_DIR,			// [set] trigger ife start by direct.     data_type: UINT32 *
	KDRV_IFE_PARAM_SET_CB,              // [set] set isr callback.                  data_type: KDRV_IPP_ISR_CB
	KDRV_IFE_PARAM_STOP_SINGLE,			// [set] stop ife.                          data_type: NULL
	KDRV_IFE_PARAM_HARD_RESET,			// [set] hard reset.                        data_type: NULL
	KDRV_IFE_PARAM_CLR_RING_BUF_LINE_CNT,//[set] clear ring buffer line counter.    data_type: NULL
	KDRV_IFE_PARAM_DMA_CH_ABORT,		// [set] set dma abort at anytime.          data_type: UINT32 *
	KDRV_IFE_PARAM_NN_ISP_DO_NEXT_SLICE,// [set] set nn isp do next slice           data_type: UINT32 *
	KDRV_IFE_PARAM_NN_ISP_SLICE_DONE,	// [set] set nn isp slice done              data_type: UINT32 *
	KDRV_IFE_PARAM_NN_ISP_SLICE_CLR,	// [set] set nn isp slice clear             data_type: UINT32 *
	KDRV_IFE_PARAM_DMA_CH_STS,			// [get] get dma abort status				data_type: UINT32 *
	KDRV_IFE_PARAM_GET_REG_BASE_ADDR,	// [get] get reg base addr					data_type: UINT32 *
	KDRV_IFE_PARAM_GET_REG_BASE_ADDR_LL,// [get] get reg base addr ll mode			data_type: UINT32 *
	KDRV_IFE_PARAM_GET_DBG_INFO,        // [get] get dbg into						data_type: KDRV_IFE_DBG_INFO *
	KDRV_IFE_PARAM_HIST_RESULT,         // [get] get hist result					data_type: KDRV_IFE_HIST_RSLT *
	KDRV_IFE_PARAM_VA_RESULT,           // [get] get va result					    data_type: KDRV_IFE_VA_RST *
	KDRV_IFE_PARAM_VA_RESULT_DUAL,      // [get] get va dual result					data_type: KDRV_IFE_VA_RST_DUAL *
	KDRV_IFE_PARAM_INDE_VA_RESULT,      // [get] get va indp result					data_type: KDRV_IFE_VA_INDEP_RSLT *
	KDRV_IFE_PARAM_GET_MERGED_INDP_VA,  // [get] get merged va indp result			data_type: KDRV_IPP_CB_STCS_OUTPUT *
	KDRV_IFE_PARAM_GET_MERGED_HIST_STSC,// [get] get merged va indp result			data_type: KDRV_IPP_CB_STCS_OUTPUT *
	KDRV_IFE_PARAM_ID_MAX,
} KDRV_IFE_PARAM_ID;

typedef enum  {//1,2,7,8,10~13 not support
	KDRV_IFE_INTERRUPT_FMD				= 0x00000001,  // interrupt enable/status: frame end
	KDRV_IFE_INTERRUPT_DEC_ERR1			= 0x00000002,  // interrupt enable/status: rde decode error1 interrupt
	KDRV_IFE_INTERRUPT_DEC_ERR2			= 0x00000004,  // interrupt enable/status: rde decode error2 interrupt
	KDRV_IFE_INTERRUPT_LLEND			= 0x00000008,  // interrupt enable/status: LinkedList end interrupt
	KDRV_IFE_INTERRUPT_LLERR			= 0x00000010,  // interrupt enable/status: LinkedList error  interrupt (linklist format error)
	KDRV_IFE_INTERRUPT_LLERR2			= 0x00000020,  // interrupt enable/status: LinkedList error2 interrupt (direct mode only, SIE send frame start before KDRV_IFE_INTERRUPT_LLEND)
	KDRV_IFE_INTERRUPT_LLJOBEND			= 0x00000040,  // interrupt enable/status: LinkedList job end interrupt
	KDRV_IFE_INTERRUPT_BUFOVFL			= 0x00000080,  // interrupt enable/status: buffer overflow interrupt
	KDRV_IFE_INTERRUPT_RING_BUF_ERR1	= 0x00000100,  // interrupt enable/status: ring buffer error1 interrupt
	KDRV_IFE_INTERRUPT_FRAME_ERR		= 0x00000200,  // interrupt enable/status: frame error interrupt (direct mode only)
	KDRV_IFE_INTERRUPT_DEC_ERR3			= 0x00000400,  // interrupt enable/status: rde decode error3 interrupt
	KDRV_IFE_INTERRUPT_RING_BUF_ERR2	= 0x00000800,  // interrupt enable/status: ring buffer error1 interrupt
	KDRV_IFE_INTERRUPT_SIE_FRAME_START	= 0x00001000,  // interrupt enable/status: SIE frame start interrupt (direct mode only)
	KDRV_IFE_INTERRUPT_SIE2_FRAME_START = 0x00002000,  // interrupt enable/status: SIE2 frame start interrupt (direct mode only)
	KDRV_IFE_INTERRUPT_IFE_FRAME_START	= 0x00004000,  // interrupt enable/status: IFE frame start interrupt (direct mode only)
	KDRV_IFE_INTERRUPT_IFE_P0_SET_SLICE_RDY = 0x00010000,  // interrupt enable/status: IFE p0 set slice ready interrupt
	KDRV_IFE_INTERRUPT_IFE_P0_GET_SLICE_CLR = 0x00020000,  // interrupt enable/status: IFE p0 get slice clear interrupt
	KDRV_IFE_INTERRUPT_IFE_P1_SET_SLICE_RDY = 0x00040000,  // interrupt enable/status: IFE p0 set slice ready interrupt
	KDRV_IFE_INTERRUPT_IFE_P1_GET_SLICE_CLR = 0x00080000,  // interrupt enable/status: IFE p0 get slice clear interrupt
	KDRV_IFE_INTERRUPT_ALL				= (KDRV_IFE_INTERRUPT_FMD | KDRV_IFE_INTERRUPT_DEC_ERR1 | KDRV_IFE_INTERRUPT_DEC_ERR2 | KDRV_IFE_INTERRUPT_DEC_ERR3 | KDRV_IFE_INTERRUPT_LLEND | KDRV_IFE_INTERRUPT_LLERR |
											KDRV_IFE_INTERRUPT_LLERR2 | KDRV_IFE_INTERRUPT_LLJOBEND | KDRV_IFE_INTERRUPT_BUFOVFL | KDRV_IFE_INTERRUPT_RING_BUF_ERR1 | KDRV_IFE_INTERRUPT_RING_BUF_ERR2 |
											KDRV_IFE_INTERRUPT_FRAME_ERR | KDRV_IFE_INTERRUPT_SIE_FRAME_START | KDRV_IFE_INTERRUPT_SIE2_FRAME_START | KDRV_IFE_INTERRUPT_IFE_FRAME_START | KDRV_IFE_INTERRUPT_IFE_P0_SET_SLICE_RDY |
											KDRV_IFE_INTERRUPT_IFE_P0_GET_SLICE_CLR | KDRV_IFE_INTERRUPT_IFE_P1_SET_SLICE_RDY | KDRV_IFE_INTERRUPT_IFE_P1_GET_SLICE_CLR),  // all interrupt enable/status mask
} KDRV_IFE_INTERRUPT;

typedef enum {
	KDRV_IFE_UPDATE_NRS0				= 0x00000001,
	//KDRV_IFE_UPDATE_NRS1				= 0x00000002,
	KDRV_IFE_UPDATE_FCURVE				= 0x00000004,
	KDRV_IFE_UPDATE_FUSION				= 0x00000008,
	KDRV_IFE_UPDATE_OUTL				= 0x00000010,
	KDRV_IFE_UPDATE_FILT				= 0x00000020,
	KDRV_IFE_UPDATE_CGAIN				= 0x00000040,
	KDRV_IFE_UPDATE_VIG					= 0x00000080,
	KDRV_IFE_UPDATE_GBAL				= 0x00000100,
	KDRV_IFE_UPDATE_DGAIN               = 0x00000200,
	KDRV_IFE_UPDATE_WDR                 = 0x00000400,
	KDRV_IFE_UPDATE_VA                  = 0x00000800,
	KDRV_IFE_UPDATE_WDR_TONE_CRV        = 0x00001000,
	KDRV_IFE_UPDATE_WDR_BLD_LUT         = 0x00002000,
	KDRV_IFE_UPDATE_WDR_SUB             = 0x00004000,
	KDRV_IFE_UPDATE_SUBISP              = 0x00008000,
	KDRV_IFE_UPDATE_FPN                 = 0x00010000,
	KDRV_IFE_UPDATE_ALL					= (KDRV_IFE_UPDATE_NRS0 | KDRV_IFE_UPDATE_FCURVE | KDRV_IFE_UPDATE_FUSION | KDRV_IFE_UPDATE_OUTL | KDRV_IFE_UPDATE_FILT | KDRV_IFE_UPDATE_CGAIN |
											KDRV_IFE_UPDATE_VIG | KDRV_IFE_UPDATE_GBAL | KDRV_IFE_UPDATE_DGAIN | KDRV_IFE_UPDATE_WDR | KDRV_IFE_UPDATE_VA | KDRV_IFE_UPDATE_WDR_TONE_CRV |
											KDRV_IFE_UPDATE_WDR_BLD_LUT | KDRV_IFE_UPDATE_WDR_SUB | KDRV_IFE_UPDATE_SUBISP | KDRV_IFE_UPDATE_FPN),
} KDRV_IFE_UPDATE;


/*******************************************************************************
*	Following #if 1 ... #endif block include all IPP IQ parameter.             *
*	These content must sync with kflow IQ parameter. (see ctl_ife_isp.h)       *
*                                                                              *
*   How to modify the following IQ parameter:                                  *
*   1. main structure                                                          *
*      (i)   add item in KDRV_IFE_IQ_CFG                                       *
*               KDRV_IFE_IQ_XXX xxx;                                           *
*      (ii)  add item in CTL_IFE_ISP_IQ_ALL                                    *
*               CTL_IFE_ISP_XXX *p_xxx;                                        *
*      (iii) add memcpy in ctl_ipp_isp_set_ife_iq()                            *
*               if (p_iq_ife->p_xxx != NULL) {                                 *
*					memcpy((void *)&p_kdrv_iq_ife->xxx, (void *)p_iq_ife->p_xxx, sizeof(p_kdrv_iq_ife->xxx)); *
*					p_kdrv_iq_xxx->update |= KDRV_IFE_UPDATE_XXX;              *
*				}                                                              *
*      (iv)  add static assert before (iii) to check structure size            *
*               STATIC_ASSERT(sizeof(CTL_IFE_ISP_XXX) == sizeof(KDRV_IFE_IQ_XXX)); *
*               if (p_iq_ife->p_xxx != NULL) { ...                             *
*                                                                              *
*   2. sub structure                                                           *
*      (i)   modify content in #if 1 ... #endif of kdrv_ife.h                  *
*      (ii)  modify content in #if 1 ... #endif of ctl_ife_isp.h               *
*      (iii) compare #if 1 ... #endif are the same                             *
*               copy #if 1 ... #endif of ctl_ife_isp.h to compare tool left    *
*               copy #if 1 ... #endif of kdrv_ife.h to compare tool right      *
*               replace 'KDRV_IFE_IQ' to 'CTL_IFE_ISP'                         *
*               do compare! (the sub structure part should be exactly the same)*
*******************************************************************************/
#if 1

#define KDRV_IFE_IQ_NRS_STR_NUM						6
#define KDRV_IFE_IQ_FCURVE_Y_W_NUM					17
#define KDRV_IFE_IQ_FCURVE_END_NUM					16
#define KDRV_IFE_IQ_FCURVE_L_NUM					65
#define KDRV_IFE_IQ_FCURVE_R_NUM					17
#define KDRV_IFE_IQ_FUSION_MC_DIFF_W_NUM			16
#define KDRV_IFE_IQ_FUSION_BLD_CUR_KNEE_NUM			1
#define KDRV_IFE_IQ_FUSION_DARK_SAT_NUM				2
#define KDRV_IFE_IQ_OUTL_BRI_TH_NUM					5
#define KDRV_IFE_IQ_OUTL_DARK_TH_NUM				5
#define KDRV_IFE_IQ_OUTL_CNT_NUM					2
#define KDRV_IFE_IQ_OUTL_ORD_W_NUM					8
#define KDRV_IFE_IQ_SPATIAL_W_LEN					6
#define KDRV_IFE_IQ_RANGE_A_TH_NUM					6
#define KDRV_IFE_IQ_RANGE_B_TH_NUM					6
#define KDRV_IFE_IQ_RANGE_A_LUT_SIZE				17
#define KDRV_IFE_IQ_RANGE_B_LUT_SIZE				17
#define KDRV_IFE_IQ_RBFILL_LUMA_NUM					17
#define KDRV_IFE_IQ_RBFILL_RATIO_NUM				32
#define KDRV_IFE_IQ_VIG_CH0_LUT_SIZE				17
#define KDRV_IFE_IQ_VIG_CH1_LUT_SIZE				17
#define KDRV_IFE_IQ_VIG_CH2_LUT_SIZE				17
#define KDRV_IFE_IQ_VIG_CH3_LUT_SIZE				17
#define KDRV_IFE_IQ_GBAL_OFS_NUM					17
#define KDRV_IFE_IQ_WDR_SUBIMG_FILT_NUM				3
#define KDRV_IFE_IQ_WDR_COEF_NUM					4
#define KDRV_IFE_IQ_WDR_INPUT_BLD_NUM				17
#define KDRV_IFE_IQ_WDR_INPUT_YV_BLD_NUM			9
#define KDRV_IFE_IQ_WDR_NEQ_TABLE_L_NUM				65
#define KDRV_IFE_IQ_WDR_NEQ_TABLE_R_NUM				17
#define KDRV_IFE_IQ_WDR_FBC_TH_NUM					6
#define KDRV_IFE_IQ_VA_INDEP_NUM					5
#define KDRV_IFE_IQ_VA_GAMMA_LEN					17
#define KDRV_IFE_IQ_SUBISP_COEF_LEN					9
#define KDRV_IFE_IQ_SUBISP_GAMMA_LEN				17
#define KDRV_IFE_IQ_FPN_PATH_MAX					1

/************************
    IFE nrs func
************************/
typedef struct {
	UINT8 enable;							///<  Enable/Disable
	UINT16 str[KDRV_IFE_IQ_NRS_STR_NUM];	///<  NRS strength. range 0~9.
} KDRV_IFE_IQ_NRS;

/************************
    IFE fcurve func
************************/
typedef enum {
	KDRV_IFE_IQ_F_CURVE_Y_8G4R4B = 0,
	KDRV_IFE_IQ_F_CURVE_Y_4G = 1,
	KDRV_IFE_IQ_F_CURVE_Y_ORI = 2,
	ENUM_DUMMY4WORD(KDRV_IFE_IQ_F_CURVE_Y_SEL)
} KDRV_IFE_IQ_F_CURVE_Y_SEL;

typedef struct {
	KDRV_IFE_IQ_F_CURVE_Y_SEL	y_mean_sel;	///< Rnage 0~2. Fcurve Y Mean Method selection.
	UINT8	yv_w;							///< Range 0~8. Fcurve Y mean and V weight.
	UINT8	ev_fmt;
} KDRV_IFE_IQ_FCURVE_CTRL;

typedef struct {
	UINT8	y_w_lut[KDRV_IFE_IQ_FCURVE_Y_W_NUM];	///< Range 0~255. Fcurve Y weight.
} KDRV_IFE_IQ_Y_W;

typedef struct {
	UINT32	fcur_l_lut[KDRV_IFE_IQ_FCURVE_L_NUM];	///< Range 0~1048576. Fcurve Left Part Value.
} KDRV_IFE_IQ_FCURVE_L_LUT;

typedef struct {
	UINT32	fcur_r_lut[KDRV_IFE_IQ_FCURVE_R_NUM];	///< Range 0~1048576. Fcurve Right Part Value.
} KDRV_IFE_IQ_FCURVE_R_LUT;

typedef struct {
	UINT8						enable;		///< Enable/Disable of f curve in IFE.
	KDRV_IFE_IQ_FCURVE_CTRL		fcur_ctrl;	///< Fcurve control Y Mean Method and V weight.
	KDRV_IFE_IQ_Y_W				y_weight;	///< Fcurve Y weight
	KDRV_IFE_IQ_FCURVE_L_LUT	fcurve_l;	///< Fcurve Left Part
	KDRV_IFE_IQ_FCURVE_R_LUT	fcurve_r;	///< Fcurve Right Part
} KDRV_IFE_IQ_FCURVE;

/************************
    IFE fusion func
************************/
/**
    range of color gain coefficients
*/
typedef enum {
	KDRV_IFE_IQ__2_8 = 0,		///< 2bits decimal, 8bits fraction
	KDRV_IFE_IQ__3_7 = 1,		///< 3bits decimal, 7bits fraction
	ENUM_DUMMY4WORD(KDRV_IFE_IQ_GAIN_FIELD)
} KDRV_IFE_IQ_GAIN_FIELD;

typedef enum {
	KDRV_IFE_IQ__8_8 = 0,		///< 8bits decimal, 8bits fraction
	KDRV_IFE_IQ__9_7 = 1,		///< 9bits decimal, 7bits fraction
	ENUM_DUMMY4WORD(KDRV_IFE_IQ_FCGAIN_FIELD)
} KDRV_IFE_IQ_FCGAIN_FIELD;

typedef enum {
	KDRV_IFE_IQ_Y_SEL_8G4R4B = 0,
	KDRV_IFE_IQ_Y_SEL_4G = 1,
	KDRV_IFE_IQ_Y_SEL_RGB_MAX = 2,
	KDRV_IFE_IQ_Y_SEL_ORI = 3,
} KDRV_IFE_IQ_Y_SEL;

typedef enum {
	KDRV_IFE_IQ_MODE_FUSION = 0,
	KDRV_IFE_IQ_MODE_SHORT_EXP = 1,
	KDRV_IFE_IQ_MODE_LONG_EXP = 2,
	//KDRV_IFE_IQ_MODE_VERY_SHORT_EXP = 3,
} KDRV_IFE_IQ_MODE_SEL;

/**
    IFE fusion ctrl func
*/
typedef struct {
	KDRV_IFE_IQ_Y_SEL		y_mean_sel;	// Y mean option. range 0~3.
	KDRV_IFE_IQ_MODE_SEL	mode;		// Fusion mode. range 0~3. (0: Fusion; 1: Short frame; 2: Long frame; 3: Long real)
	UINT16					ev_ratio;	// EV ratio. range 0~511. (4EV=16*16, 3EV = 8*16, 2EV=2*16, 1EV=1*16)
} KDRV_IFE_IQ_FUSION_CTRL;

typedef enum {
	KDRV_IFE_IQ_FUSION_BY_L_EXP = 0,
	KDRV_IFE_IQ_FUSION_BY_S_EXP = 1,
	KDRV_IFE_IQ_FUSION_BOTH_EXP = 2,
} KDRV_IFE_IQ_FUSION_NOR_SEL;

/**
    IFE fusion blend curve
*/
typedef struct {
	KDRV_IFE_IQ_FUSION_NOR_SEL nor_sel;							///< Range 0~2. Normal blending curve weighting option.
	KDRV_IFE_IQ_FUSION_NOR_SEL dif_sel;							///< Range 0~2. Difference blending curve weighting option.

	/* normal blending - long exposure param. */
	UINT16	l_nor_knee[KDRV_IFE_IQ_FUSION_BLD_CUR_KNEE_NUM];	///< range 0~4095.  Knee point of normal blending curve for long exposure.
	UINT16	l_nor_range;										///< range 16~4095. Normal blending curve range for long  exposure. Actually range = (1<<l_nor_range).
	UINT16	l_nor_slope;										///< range 0~65535. Weighting function slope.

	/* normal blending - short exposure param. */
	UINT16	s_nor_knee[KDRV_IFE_IQ_FUSION_BLD_CUR_KNEE_NUM];	///< range 0~4095.  Knee point of normal blending curve for short exposure.
	UINT16	s_nor_range;										///< range 16~4095. Normal blending curve range for short  exposure.
	UINT16	s_nor_slope;										///< range 0~65535. Weighting function slope.

	/* difference blending - long exposure param. */
	UINT16	l_dif_knee[KDRV_IFE_IQ_FUSION_BLD_CUR_KNEE_NUM];	///< range 0~4095.  Knee point of difference blending curve for long exposure.
	UINT16	l_dif_range;										///< range 16~4095. Difference blending curve range for long  exposure. Actually range = (1<<l_dif_range).
	UINT16	l_dif_slope;										///< range 0~65535. Weighting function slope.

	/* difference blending - short exposure param. */
	UINT16	s_dif_knee[KDRV_IFE_IQ_FUSION_BLD_CUR_KNEE_NUM];	///< range 0~4095.  Knee point of difference blending curve for short exposure.
	UINT16	s_dif_range;										///< range 16~4095. Difference blending curve range for short  exposure. Actually range = (1<<l_dif_range).
	UINT16	s_dif_slope;										///< range 0~65535. Weighting function slope.

} KDRV_IFE_IQ_BLEND_CURVE;

/**
    IFE fusion mc
*/
typedef struct {
	UINT8 lum_th;											///< Range 0~255. Lower than this threshold using normal blending curve.
	UINT8 diff_ratio;										///< Range 0~3   . Control ratio.
	UINT8 diff_w[KDRV_IFE_IQ_FUSION_MC_DIFF_W_NUM];			///< Range 0~16. Difference weighting for difference > 0.
	UINT8 dwd;												///< Range 0~16. Using this ratio if difference less than lum_th.
} KDRV_IFE_IQ_MC;

/**
    IFE fusion dark sat reduce
*/
typedef struct {
	UINT16 th[KDRV_IFE_IQ_FUSION_DARK_SAT_NUM];				///< Range 0~4095. Threshold of DS.
	UINT8 step[KDRV_IFE_IQ_FUSION_DARK_SAT_NUM];			///< Range 0~255 . Weighting step of DS.
	UINT8 low_bound[KDRV_IFE_IQ_FUSION_DARK_SAT_NUM];		///< range 0~255 . Lower bound of DS.
} KDRV_IFE_IQ_DARK_SAT;

/**
    IFE fusion color gain
*/
typedef enum {
	KDRV_IFE_IQ_CGAIN_CH_R = 0,					// R  channel for RGGB & RGBIr
	KDRV_IFE_IQ_CGAIN_CH_GR,					// Gr channel for RGGB & RGBIr
	KDRV_IFE_IQ_CGAIN_CH_GB,					// Gb channel for RGGB & RGBIr
	KDRV_IFE_IQ_CGAIN_CH_B,						// B  channel for RGGB & RGBIr
	KDRV_IFE_IQ_CGAIN_CH_IR,					// Ir channel for        RGBIr
	KDRV_IFE_IQ_CGAIN_CH_MAX,
} KDRV_IFE_IQ_CGAIN_CH;

typedef struct {
	UINT8					enable;					// Enable/Disable.
	KDRV_IFE_IQ_FCGAIN_FIELD	bit_field;			// select Fusion color gain decimal range. range 0~1.

	/* short exposure param. */
	UINT16  fcgain_s[KDRV_IFE_IQ_CGAIN_CH_MAX];		// SHDR short exposure path color gain. range 0~1023. please used KDRV_IFE_IQ_CGAIN_CH to assign KDRV_IFE_IQ_CGAIN_CH_MAX index
	UINT16  fcofs_s[KDRV_IFE_IQ_CGAIN_CH_MAX];		// SHDR short exposure path color offset. range 0~1023. please used KDRV_IFE_IQ_CGAIN_CH to assign KDRV_IFE_IQ_CGAIN_CH_MAX index

	/* long exposure param. */
	UINT16  fcgain_l[KDRV_IFE_IQ_CGAIN_CH_MAX];		// SHDR long exposure path color gain. range 0~1023. please used KDRV_IFE_IQ_CGAIN_CH to assign KDRV_IFE_IQ_CGAIN_CH_MAX index
	UINT16  fcofs_l[KDRV_IFE_IQ_CGAIN_CH_MAX];		// SHDR long exposure path color offset. range 0~1023. please used KDRV_IFE_IQ_CGAIN_CH to assign KDRV_IFE_IQ_CGAIN_CH_MAX index
} KDRV_IFE_IQ_FCGAIN;

/**
    IFE fusion dbg mode
*/
typedef enum {
	KDRV_IFE_IQ_DISABLE_DBG_MODE        = 0,     // Disable dbg mode (same as enable=0)
	KDRV_IFE_IQ_L_EXPOS_WEIGHT          = 1,     // show long exposure weight
	KDRV_IFE_IQ_S_EXPOS_WEIGHT          = 2,     // show short exposure weight
	KDRV_IFE_IQ_N_CURVE_WEIGHT          = 3,     // show n curve weight
	KDRV_IFE_IQ_D_CURVE_WEIGHT          = 4,     // show d curve weight
	//KDRV_IFE_IQ_FIRST_FU_WEIGHT_RESULT  = 5,     // show weighting of first fusion result
	//KDRV_IFE_IQ_VERY_S_FRAME_WEIGHT     = 6,     // show weighting of very short frame
	KDRV_IFE_IQ_DBG_MODE_UNKNOWN,
} KDRV_IFE_IQ_FU_DBG_MODE;

typedef struct {
	BOOL enable;						//enable=0 disable=1
	KDRV_IFE_IQ_FU_DBG_MODE mode;		//mode 0~6
} KDRV_IFE_IQ_FUSION_DBG;

/**
    IFE fusion func
*/
typedef struct {
	KDRV_IFE_IQ_FUSION_CTRL   fu_ctrl;
	KDRV_IFE_IQ_BLEND_CURVE   bld_cur;
	KDRV_IFE_IQ_MC            mc_para;
	KDRV_IFE_IQ_DARK_SAT      dk_sat;
	KDRV_IFE_IQ_FCGAIN        fu_cgain;
	KDRV_IFE_IQ_FUSION_DBG    dbg;
} KDRV_IFE_IQ_FUSION;

/************************
    IFE outlier func
************************/
typedef struct {
	UINT8	enable;											///< Enable/Disable
	UINT16	bright_th[KDRV_IFE_IQ_OUTL_BRI_TH_NUM];			///< range: 0~4095
                                                            ///< outlier bright case threshold
	UINT16	dark_th [KDRV_IFE_IQ_OUTL_DARK_TH_NUM];			///< range: 0~4095
                                                            ///< outlier dark case threshold
	UINT8	outl_cnt[KDRV_IFE_IQ_OUTL_CNT_NUM];				///< range: 0~16  , UINT32[2], outlier neighbor point counter
	UINT8	outl_weight;									///< range: 0~255 , outlier weighting
	UINT16	dark_ofs;										///< range: 0~4095, outlier dark th offset
	UINT16	bright_ofs;										///< range: 0~4095, outlier bright th offset

	UINT8	outl_comp_mode;

	UINT8	ord_rng_bri;									///< range: 0~7   , control the strength of order method for bright defect pixel.
	UINT8	ord_rng_dark;									///< range: 0~7   , control the strength of order method for dark defect pixel.
	UINT16	ord_protect_th;									///< range: 0~1023, order method protect artifact th
	UINT8	ord_blend_w;									///< range: 0~255 , outlier and order result blending weight
	UINT8	ord_bri_w[KDRV_IFE_IQ_OUTL_ORD_W_NUM];			///< range: 0~8   , order bright defect pixel compensate weight
	UINT8	ord_dark_w[KDRV_IFE_IQ_OUTL_ORD_W_NUM];			///< range: 0~8   , order dark defect pixel compensate weight
	UINT8	outl_rgbir_rb_w;
	UINT8	ord_rgbir_rb_w;
} KDRV_IFE_IQ_OUTL;

/************************
    IFE bilateral filter
************************/
typedef enum{
	KDRV_IFE_SPTL_FILT_5X5 = 0,
	KDRV_IFE_SPTL_FILT_7x7 = 1,
}KDRV_IFE_IQ_FILT_MODE;


typedef struct {
	KDRV_IFE_IQ_FILT_MODE  ife_filt_mode;           ///< spatial filter mode. 5x5 or 7x7
	UINT8  weight[KDRV_IFE_IQ_SPATIAL_W_LEN];		///< range:0~31, UINT32[6], filter spatial weight,
} KDRV_IFE_IQ_SPATIAL;

typedef struct {
	UINT16  a_th [KDRV_IFE_IQ_RANGE_A_TH_NUM];		///< range:0~1023, UINT32[6] , range filter A threshold
	UINT16  a_lut[KDRV_IFE_IQ_RANGE_A_LUT_SIZE];	///< range:0~1023, UINT32[17], range filter A threshold adjustment
	UINT16  b_th [KDRV_IFE_IQ_RANGE_B_TH_NUM];		///< range:0~1023, UINT32[6] , range filter B threshold
	UINT16  b_lut[KDRV_IFE_IQ_RANGE_B_LUT_SIZE];	///< range:0~1023, UINT32[17], range filter B threshold adjustment
} KDRV_IFE_IQ_RANGE_FILTER_R;

typedef struct {
	UINT16  a_th [KDRV_IFE_IQ_RANGE_A_TH_NUM];		///< range:0~1023, UINT32[6] , range filter A threshold
	UINT16  a_lut[KDRV_IFE_IQ_RANGE_A_LUT_SIZE];	///< range:0~1023, UINT32[17], range filter A threshold adjustment
	UINT16  b_th [KDRV_IFE_IQ_RANGE_B_TH_NUM];		///< range:0~1023, UINT32[6] , range filter B threshold
	UINT16  b_lut[KDRV_IFE_IQ_RANGE_B_LUT_SIZE];	///< range:0~1023, UINT32[17], range filter B threshold adjustment
} KDRV_IFE_IQ_RANGE_FILTER_GR;

typedef struct {
	UINT16  a_th [KDRV_IFE_IQ_RANGE_A_TH_NUM];		///< range:0~1023, UINT32[6] , range filter A threshold
	UINT16  a_lut[KDRV_IFE_IQ_RANGE_A_LUT_SIZE];	///< range:0~1023, UINT32[17], range filter A threshold adjustment
	UINT16  b_th [KDRV_IFE_IQ_RANGE_B_TH_NUM];		///< range:0~1023, UINT32[6] , range filter B threshold
	UINT16  b_lut[KDRV_IFE_IQ_RANGE_B_LUT_SIZE];	///< range:0~1023, UINT32[17], range filter B threshold adjustment
} KDRV_IFE_IQ_RANGE_FILTER_GB;

typedef struct {
	UINT16  a_th [KDRV_IFE_IQ_RANGE_A_TH_NUM];		///< range:0~1023, UINT32[6] , range filter A threshold
	UINT16  a_lut[KDRV_IFE_IQ_RANGE_A_LUT_SIZE];	///< range:0~1023, UINT32[17], range filter A threshold adjustment
	UINT16  b_th [KDRV_IFE_IQ_RANGE_B_TH_NUM];		///< range:0~1023, UINT32[6] , range filter B threshold
	UINT16  b_lut[KDRV_IFE_IQ_RANGE_B_LUT_SIZE];	///< range:0~1023, UINT32[17], range filter B threshold adjustment
} KDRV_IFE_IQ_RANGE_FILTER_B;

typedef struct {
	UINT16  a_th [KDRV_IFE_IQ_RANGE_A_TH_NUM];		///< range:0~1023, UINT32[6] , range filter A threshold
	UINT16  a_lut[KDRV_IFE_IQ_RANGE_A_LUT_SIZE];	///< range:0~1023, UINT32[17], range filter A threshold adjustment
	UINT16  b_th [KDRV_IFE_IQ_RANGE_B_TH_NUM];		///< range:0~1023, UINT32[6] , range filter B threshold
	UINT16  b_lut[KDRV_IFE_IQ_RANGE_B_LUT_SIZE];	///< range:0~1023, UINT32[17], range filter B threshold adjustment
} KDRV_IFE_IQ_RANGE_FILTER_IR;

typedef enum {
	KDRV_IFE_IQ_BILAT_CEN_DPC = 0,		// 52x algorithm
	KDRV_IFE_IQ_BILAT_CEN_BILATERAL,	// bypass center pixel
} KDRV_IFE_IQ_BILAT_CEN_SEL;

typedef struct {
	UINT8	enable;						///<               3x3 center mofify bilateral filter threshold enable
	UINT16  th1;						///< range:0~1023, 3x3 center mofify bilateral filter threshold 1
	UINT16  th2;						///< range:0~1023, 3x3 center mofify bilateral filter threshold 2
	KDRV_IFE_IQ_BILAT_CEN_SEL cen_sel;	///<               3x3 center mofify bilateral filter threshold algorithm
} KDRV_IFE_IQ_CENTER_MODIFY;

/**
    clamp and weighting setting
*/
typedef struct {
	UINT16  th;			///< range:0~4095, threshold
	UINT8   mul;		///< range:0~255 , weighting multiplier
	UINT16  dlt;		///< range:0~4095, threshold adjustment
} KDRV_IFE_IQ_CLAMP;

/**
    IFE structure - IFE RB fill parameter set.
*/
typedef struct {
	UINT8   enable;									///< range: 0~1 , Enable/Disable
	UINT8   luma [KDRV_IFE_IQ_RBFILL_LUMA_NUM];		///< range: 0~31,
	UINT8   ratio[KDRV_IFE_IQ_RBFILL_RATIO_NUM];	///< range: 0~31,
	UINT8   ratio_mode;								///< range: 0~2 , n3.2, n2.3, n1.4
} KDRV_IFE_IQ_RBFILL_PARAM;

typedef struct {
	UINT8 motion_en;            ///< 2dnr consider motion
	UINT8 static_symbol;        ///< define static area value of motion map
	UINT8 transition_symbol;    ///< define transition area value of motion map
	UINT8 motion_symbol;        ///< define motion area value of motion map
}  KDRV_IFE_IQ_MOTIONSET;

typedef struct {
	UINT8							enable;			///< Enable/Disable
	KDRV_IFE_IQ_SPATIAL				spatial;		///< filter spatial weight table
	KDRV_IFE_IQ_RANGE_FILTER_R		rng_filt_r;		///< range filter ch0
	KDRV_IFE_IQ_RANGE_FILTER_GR		rng_filt_gr;	///< range filter ch1
	KDRV_IFE_IQ_RANGE_FILTER_GB		rng_filt_gb;	///< range filter ch2
	KDRV_IFE_IQ_RANGE_FILTER_B		rng_filt_b;		///< range filter ch3
	KDRV_IFE_IQ_RANGE_FILTER_IR		rng_filt_ir;	///< range filter ch2
	//Transition RTH1
	KDRV_IFE_IQ_RANGE_FILTER_R		rng_filt_r_1;	///< range filter ch0
	KDRV_IFE_IQ_RANGE_FILTER_GR		rng_filt_gr_1;	///< range filter ch1
	KDRV_IFE_IQ_RANGE_FILTER_GB		rng_filt_gb_1;	///< range filter ch2
	KDRV_IFE_IQ_RANGE_FILTER_B		rng_filt_b_1;	///< range filter ch3
	KDRV_IFE_IQ_RANGE_FILTER_IR		rng_filt_ir_1;	///< range filter ch2
	//Motion RTH2
	KDRV_IFE_IQ_RANGE_FILTER_R		rng_filt_r_2;	///< range filter ch0
	KDRV_IFE_IQ_RANGE_FILTER_GR		rng_filt_gr_2;	///< range filter ch1
	KDRV_IFE_IQ_RANGE_FILTER_GB		rng_filt_gb_2;	///< range filter ch2
	KDRV_IFE_IQ_RANGE_FILTER_B		rng_filt_b_2;	///< range filter ch3
	KDRV_IFE_IQ_RANGE_FILTER_IR		rng_filt_ir_2;	///< range filter ch2

	KDRV_IFE_IQ_CENTER_MODIFY		center_mod;		///< center modify
	KDRV_IFE_IQ_CLAMP				clamp;			///< clamp and weighting setting
	KDRV_IFE_IQ_RBFILL_PARAM		rbfill;			///< RGBIr fill r/b pixel
	UINT8							blend_w;		///< range:0~255, range filter A and B weighting
	UINT8							rng_th_w;		///< range:0~15 ,range th weight
	UINT8							bin;			///< range:0~7  , ( Denominator: 2<<bin )

	//539A add
	UINT8							range_sel;		///< FILTER_RANGE
	KDRV_IFE_IQ_MOTIONSET			motion_set;		///< motion set

} KDRV_IFE_IQ_FILTER;

/************************
    IFE digital gain
************************/
typedef struct {
	UINT8 enable;		// Enable/Disable. range:0~1.
	UINT16 dgain;		// digital gain. range 0~65535.
} KDRV_IFE_IQ_DGAIN;

/************************
    IFE color gain
************************/
typedef struct {
	UINT8					enable;			///< range:0~1   , Enable/Disable
	UINT8					inv;			///< range:0~1   , color gain invert
	UINT8					hinv;			///< range:0~1   , color gain H-invert
	KDRV_IFE_IQ_GAIN_FIELD	bit_field;		///< range:0~1   , color gain bit field
	UINT16					mask;			///< range:0~4095, color gain mask

	UINT16					cgain_r;		///< range 0~1023, R  channel color gain.
	UINT16					cgain_gr;		///< range 0~1023, Gr channel color gain.
	UINT16					cgain_gb;		///< range 0~1023, Gb channel color gain.
	UINT16					cgain_b;		///< range 0~1023, B  channel color gain.
	UINT16					cgain_ir;		///< range 0~1023, IR channel color gain.

	UINT16					cofs_r;			///< range 0~1023, R  channel color offset.
	UINT16					cofs_gr;		///< range 0~1023, Gr channel color offset.
	UINT16					cofs_gb;		///< range 0~1023, Gb channel color offset.
	UINT16					cofs_b;			///< range 0~1023, B  channel color offset.
	UINT16					cofs_ir;		///< range 0~1023, IR channel color offset.
} KDRV_IFE_IQ_CGAIN;

/************************
    ife vignette shading compensation
************************/
typedef struct {
	UINT8	enable;										///< range: 0~1   , Enable/Disable
	UINT16	dist_th;									///< range: 0~1023, disabled area distance threshold

	UINT16	ch_r_lut[KDRV_IFE_IQ_VIG_CH0_LUT_SIZE];		///< range: 0~1023, VIG LUT of R  channel
	UINT16	ch_gr_lut[KDRV_IFE_IQ_VIG_CH1_LUT_SIZE];	///< range: 0~1023, VIG LUT of Gr channel
	UINT16	ch_gb_lut[KDRV_IFE_IQ_VIG_CH2_LUT_SIZE];	///< range: 0~1023, VIG LUT of Gb channel
	UINT16	ch_b_lut[KDRV_IFE_IQ_VIG_CH3_LUT_SIZE];		///< range: 0~1023, VIG LUT of B  channel
	UINT16	ch_ir_lut[KDRV_IFE_IQ_VIG_CH2_LUT_SIZE];	///< range: 0~1023, VIG LUT of Ir channel

	UINT8	dither_enable;								///< range: 0~1   , dithering enable/disable
	UINT8	dither_rst_enable;							///< range: 0~1   , dithering reset enable
} KDRV_IFE_IQ_VIG;

/************************
    IFE structure - IFE gbalance parameter set.
************************/
typedef struct {
	UINT8	enable;				///< range: 0~1   , enable/disable
	UINT8	protect_enable;		///< range: 0~1   , enable/disable preserve edge or feature
	UINT16	diff_th_str;		///< range: 0~4095, gbal strength
	UINT8	diff_w_max;			///< range: 0~15  , difference weight max
	UINT16	edge_protect_th1;	///< range: 0~4095, edge judgement th 1
	UINT16	edge_protect_th0;	///< range: 0~4095, edge judgement th 0
	UINT8	edge_w_max;			///< range: 0~255 , edge weight max
	UINT8	edge_w_min;			///< range: 0~255 , edge weight min
	UINT8	gbal_ofs[KDRV_IFE_IQ_GBAL_OFS_NUM];///< range: 0~63  , weight transision region
	UINT16	str_luma_low_bnd;
	UINT16	edge_luma_low_bnd;
} KDRV_IFE_IQ_GBAL;

/************************
    IFE KDRV WDR/TONE CURVE Param and Enable/Disable
************************/
/**
    IFE structure - WDR input blending parameters
*/
typedef enum {
	KDRV_IFE_IQ_INPUT_BLD_3x3_Y  = 0,   ///< use Y from bayer 3x3 as the input blending source selection
	KDRV_IFE_IQ_INPUT_BLD_GMEAN  = 1,   ///< use Gmean from bayer 3x3 as the input blending source selection
	KDRV_IFE_IQ_INPUT_BLD_BAYER  = 2,   ///< use Bayer as the input blending source selection
	ENUM_DUMMY4WORD(KDRV_IFE_IQ_WDR_INPUT_BLDSEL)
} KDRV_IFE_IQ_WDR_INPUT_BLDSEL;

typedef struct {
	KDRV_IFE_IQ_WDR_INPUT_BLDSEL bld_sel;
	UINT8 blend_lut[KDRV_IFE_IQ_WDR_INPUT_BLD_NUM];				///< blending of Y and raw [0~255], 0: Y, 255: raw
	UINT8 in_yv_blend_lut[KDRV_IFE_IQ_WDR_INPUT_YV_BLD_NUM];	///< lut of YV blending [0~63]
} KDRV_IFE_IQ_WDR_IN_BLD;

/**
    IFE structure - WDR non-equal table parameters
*/
typedef struct {
	UINT16 left_table[KDRV_IFE_IQ_WDR_NEQ_TABLE_L_NUM];		///< non equatable table value [0~4095]
	UINT16 right_table[KDRV_IFE_IQ_WDR_NEQ_TABLE_R_NUM];	///< non equatable table value [0~4095]
} KDRV_IFE_IQ_WDR_NEQ_TABLE;

typedef enum {
	KDRV_IFE_IQ_WDR_NORMAL_MODE = 0,	///< wdr normal mode
	KDRV_IFE_IQ_WDR_ANTI_HALO_MODE = 1,	///< wdr anti halo mode
} KDRV_IFE_IQ_WDR_MODE;

typedef enum {
	KDRV_IFE_IQ_ANTI_HALO_BLENDING = 0, ///< blending anti halo method
	KDRV_IFE_IQ_ANTI_HALO_ADAPTIVE = 1,	///< adaptive anti halo method
} KDRV_IFE_IQ_ANTI_HALO_OPT;

/**
    IFE structure - WDR strength parameters
*/
typedef struct {
	INT16 wdr_coeff[KDRV_IFE_IQ_WDR_COEF_NUM];	///< wdr coefficient [-4096~4095]
	UINT8 strength;								///< wdr strength [0~255]
	KDRV_IFE_IQ_WDR_MODE wdr_mode;				///< wdr mode, 0:Normal mode, 1:Anti-halo mode
	KDRV_IFE_IQ_ANTI_HALO_OPT wdr_anti_halo_opt;///< wdr anti halo option, 0:blending, 1:adaptive
	UINT8  wdr_halo_ratio;						///< wdr halo ratio [0~255]
	UINT8  wdr_halo_slope;						///< wdr halo slop [0~255]
	UINT16 wdr_b2p_var;							///< wdr b2p var [0~4095]
} KDRV_IFE_IQ_WDR_STRENGTH;

/**
    IFE structure - WDR gain control parameters
*/
typedef struct {
	BOOL gainctrl_en;							///< enable gain control for wdr
	UINT8 max_gain;								///< maximum gain of wdr [1~255]
	UINT8 min_gain;								///< minimum gain of wdr [1~255], mapping to 1/256, 2/256, 255/256
} KDRV_IFE_IQ_WDR_GAINCTRL;

/**
    IFE structure - WDR output blending parameters
*/
typedef struct {
	BOOL outbld_en;								///< enable output blending for wdr
	KDRV_IFE_IQ_WDR_NEQ_TABLE outbld_lut;		///< wdr output blending table
} KDRV_IFE_IQ_WDR_OUTBLD;

/**
    IFE structure - WDR saturation reduction parameters
*/
typedef struct {
	UINT16 sat_th;								///< threshold of wdr saturation reduction [0~4095]
	UINT8 sat_wt_low;							///< lower weight of wdr saturation reduction [0~255]
	UINT8 sat_delta;							///< delta of wdr saturation reduction [0~255]
} KDRV_IFE_IQ_WDR_SAT_REDUCT;

typedef enum {
	KDRV_IFE_IQ_WDR_ROUNDING        = 0,  ///< rounding
	KDRV_IFE_IQ_WDR_HALFTONING      = 1,  ///< halftone rounding
	KDRV_IFE_IQ_WDR_RANDOMLSB       = 2,  ///< random rounding
	ENUM_DUMMY4WORD(KDRV_IFE_IQ_WDR_DITHER_MODE)
} KDRV_IFE_IQ_WDR_DITHER_MODE;

/**
    IFE structure - WDR dither parameters.
*/
typedef struct {
	BOOL wdr_dithering_en;
	BOOL wdr_rand_rst;
	KDRV_IFE_IQ_WDR_DITHER_MODE wdr_rand_sel;
} KDRV_IFE_IQ_WDR_DITHER;

/**
    IFE structure - WDR foreground brightness compensation parameters
*/
typedef struct {
	BOOL    fbc_en;									///< wdr fbc enable, 0:disable, 1:enable
	UINT8   fbc_ratio;								///< wdr fbc ratio [0~255]
	UINT16  fbc_th[KDRV_IFE_IQ_WDR_FBC_TH_NUM];		///< wdr th [0~8191]
} KDRV_IFE_IQ_WDR_FBC;

typedef struct {
	BOOL wdr_enable;
	BOOL tonecurve_enable;
	UINT8 ftrcoef[KDRV_IFE_IQ_WDR_SUBIMG_FILT_NUM];			///< wdr sub-image low pass filter coefficients, 3 entries
	KDRV_IFE_IQ_WDR_IN_BLD		input_bld;					///< wdr input blending
	KDRV_IFE_IQ_WDR_NEQ_TABLE	tonecurve;					///< wdr tonecurve configuration
	KDRV_IFE_IQ_WDR_STRENGTH	wdr_str;					///< wdr strength
	KDRV_IFE_IQ_WDR_GAINCTRL	gainctrl;					///< wdr gain control
	KDRV_IFE_IQ_WDR_OUTBLD		outbld;						///< wdr output blending settings
	KDRV_IFE_IQ_WDR_SAT_REDUCT	sat_reduct;					///< wdr saturation reduction
	KDRV_IFE_IQ_WDR_DITHER		dither;						///< wdr sub-img dither
	KDRV_IFE_IQ_WDR_FBC			fbc;						///< wdr foreground brightness compensation
	UINT16						wdr_gain_prot_str;			///< wdr gain prot str
} KDRV_IFE_IQ_WDR;

/************************
    IFE WDR subimg size Information
************************/
typedef struct {
	UINT8	subimg_size_h;			///< wdr sub-image size [8~48]
	UINT8	subimg_size_v;			///< wdr sub-image size [8~48]
	//UINT16	subimg_lofs_in;			///< wdr subin lineoffset, lofs >= (subimg_size_h * 8)
	//UINT16	subimg_lofs_out;		///< wdr subout lineoffset, lofs >= (subimg_size_h * 8)
} KDRV_IFE_IQ_WDR_SUBIMG;

/************************
    IFE Histogram Param and Enable/Disable
************************/
typedef enum {
	KDRV_IFE_IQ_BEFORE_WDR	= 0,	///< histogram statistics before wdr algorithm
	KDRV_IFE_IQ_AFTER_WDR	= 1,	///< histogram statistics after wdr algorithm
	ENUM_DUMMY4WORD(KDRV_IFE_IQ_HIST_SEL)
} KDRV_IFE_IQ_HIST_SEL;

typedef struct {
	BOOL hist_enable;				///< Enable histogram statistics
	KDRV_IFE_IQ_HIST_SEL hist_sel;	///< selection of statistics input
	UINT8 step_h;					///< h step size of histogram [0~31]
	UINT8 step_v;					///< v step size of histogram [0~31]
} KDRV_IFE_IQ_HIST;

/************************
    IFE structure - VA paramater settingss
************************/
typedef enum {
	KDRV_IFE_IQ_VA_FILTER_SYM_MIRROR = 0,
	KDRV_IFE_IQ_VA_FILTER_SYM_INVERSE,
} KDRV_IFE_IQ_VA_FILTER_SYM_SEL;

typedef enum {
	KDRV_IFE_IQ_VA_FILTER_SIZE_1 = 0,
	KDRV_IFE_IQ_VA_FILTER_SIZE_3,
	KDRV_IFE_IQ_VA_FILTER_SIZE_5,
	KDRV_IFE_IQ_VA_FILTER_SIZE_7,
} KDRV_IFE_IQ_VA_FILTER_SIZE_SEL;

typedef struct {
	KDRV_IFE_IQ_VA_FILTER_SYM_SEL symmetry;		///< select filter kernel symmetry
	KDRV_IFE_IQ_VA_FILTER_SIZE_SEL filter_size;	///< select filter kernel size
	UINT8 tap_a;								///< filter coefficent A, range: 0~31
	INT8  tap_b;								///< filter coefficent B, range: -16~15
	INT8  tap_c;								///< filter coefficent C, range: -8~7
	INT8  tap_d;								///< filter coefficent D, range: -8~7
	UINT8 div;									///< filter normalized term, range: 0~15
	UINT16 th_l;								///< lower threshold, range: 0~4095
	UINT16 th_u;								///< upper threshold, range: 0~4095
} KDRV_IFE_IQ_VA_FILTER;

typedef struct {
	KDRV_IFE_IQ_VA_FILTER_SYM_SEL symmetry_iir2;///< select filter kernel symmetry
	KDRV_IFE_IQ_VA_FILTER_SYM_SEL symmetry_iir3;///< select filter kernel symmetry
	INT16 tap_iir1_e;							///<
	INT16 tap_iir1_f;							///<
    UINT8 iir1_shift_bit;
    BOOL  iir2_enable;
	INT16 tap_iir2_a;							///<
	INT16 tap_iir2_b;							///<
	INT16 tap_iir2_e;							///<
	INT16 tap_iir2_f;							///<
    UINT8 iir2_shift_bit;
    BOOL  iir3_enable;
	INT16 tap_iir3_a;							///<
	INT16 tap_iir3_b;							///<
	INT16 tap_iir3_e;							///<
	INT16 tap_iir3_f;							///<
    UINT8 iir3_shift_bit;
} KDRV_IFE_IQ_VA_IIR_FILTER;

typedef struct {
	KDRV_IFE_IQ_VA_FILTER h_filt;				///< horizontal filter
	KDRV_IFE_IQ_VA_FILTER v_filt;				///< vertical filter
	KDRV_IFE_IQ_VA_IIR_FILTER iir_filt;			///< vertical filter
	BOOL count_enable;							///< count the nonzero pixels
} KDRV_IFE_IQ_VA_GROUP_INFO;

typedef struct {
	BOOL enable;								///< va independent window enable
} KDRV_IFE_IQ_VA_INDEP_OPT;

typedef struct {
	UINT8 ldg_low_th;          ///< Darkness threshold
	UINT8 ldg_high_th;         ///< Brightness threshold
	UINT8 ldg_low_gain;        ///< Dark region minima gain
	UINT8 ldg_high_gain;       ///< Bright region minima gain
	UINT8 ldg_low_slope;       ///< Dark region gain slope
	UINT8 ldg_high_slope;      ///< Bright region gain slope
} KDRV_IFE_IQ_VA_LDG_PARAM;

typedef enum {
	KDRV_IFE_IQ_NO_PRE_FILTER     = 0,   ///<
	KDRV_IFE_IQ_PRE_FILTER_3x3_1  = 1,   ///<
	KDRV_IFE_IQ_PRE_FILTER_3x3_2  = 2,   ///<
} KDRV_IFE_IQ_VA_PRE_FILTER_MODE;

typedef enum {
	KDRV_IFE_IQ_VA_ENERGY_COUNT     = 0,   ///<
	KDRV_IFE_IQ_VA_HIGH_LUMA_COUNT  = 1,   ///<
} KDRV_IFE_IQ_VA_CNT_OUTSEL;

typedef enum {
	KDRV_IFE_IQ_VA_VDETGH_FIR     = 0,   ///< Horizontal Filter Selection
	KDRV_IFE_IQ_VA_VDETGH_IIR  	= 1,
} KDRV_IFE_IQ_VA_VDETGH_FILTER_SEL;

typedef enum {
	KDRV_IFE_IQ_VA_VDETGH_IIR_INPUT_ORIGINAL     = 0,   ///< G1/2H IIR input select
	KDRV_IFE_IQ_VA_VDETGH_IIR_INPUT_JUMP_PIXEL   = 1,
} KDRV_IFE_IQ_VA_VDETGH_IIR_INPUT_SEL;

typedef struct {
	KDRV_IFE_IQ_VA_VDETGH_FILTER_SEL vdetgh1_filter_sel;
	KDRV_IFE_IQ_VA_VDETGH_IIR_INPUT_SEL vdetgh1_iir_input_sel;  //538 add
	KDRV_IFE_IQ_VA_VDETGH_FILTER_SEL vdetgh2_filter_sel;
	KDRV_IFE_IQ_VA_VDETGH_IIR_INPUT_SEL vdetgh2_iir_input_sel;  //538 add
} KDRV_IFE_IQ_VA_FILTER_SEL;

typedef struct {
	BOOL enable;								///< va function enable
	BOOL indep_va_enable;						///< independent va function enable
	BOOL ldg_enable;							///< va level dependent gain enable
	BOOL gamma_enable;							///< va gamma enable

	//filter
	KDRV_IFE_IQ_VA_GROUP_INFO group_1;			///< va group 1 info
	KDRV_IFE_IQ_VA_GROUP_INFO group_2;			///< va group 2 info
	KDRV_IFE_IQ_VA_FILTER_SEL fltr_sel;

	//VA
	BOOL va_out_grp1_2;							///< 0 --> output only group 1, 1 --> output group 1 and 2
	//UINT32 va_lofs;								///< va output lineoffset
	USIZE win_num;								///< va window number, 1x1 ~ 8x8

	//independent VA
	KDRV_IFE_IQ_VA_INDEP_OPT indep_win[KDRV_IFE_IQ_VA_INDEP_NUM]; ///< va independent settings

	//ldg
	KDRV_IFE_IQ_VA_LDG_PARAM ldg_para;

	//pre
	KDRV_IFE_IQ_VA_PRE_FILTER_MODE pre_filter_mode;

	//gamma
	UINT16 va_gamma_lut[KDRV_IFE_IQ_VA_GAMMA_LEN];

	KDRV_IFE_IQ_VA_CNT_OUTSEL win_cnt_out_sel;
	UINT8 high_luma_th;
	UINT8 energy_w;

} KDRV_IFE_IQ_VA;

/************************
    IFE Subisp
************************/
typedef struct {
	BOOL subisp_cst_en;
	INT16 coef[KDRV_IFE_IQ_SUBISP_COEF_LEN];

	BOOL subisp_gamma_en;
	UINT8 subisp_gamma_lut[KDRV_IFE_IQ_SUBISP_GAMMA_LEN];

	BOOL subisp_cfa_en;
} KDRV_IFE_IQ_SUBISP_IQ;

/************************
    IFE FPN
************************/
typedef struct {
	BOOL   fpn_en;
	UINT8  fpn_cgain_range;
	UINT16  fpn_cgain_r[KDRV_IFE_IQ_FPN_PATH_MAX];
	UINT16  fpn_cgain_gr[KDRV_IFE_IQ_FPN_PATH_MAX];
	UINT16  fpn_cgain_gb[KDRV_IFE_IQ_FPN_PATH_MAX];
	UINT16  fpn_cgain_b[KDRV_IFE_IQ_FPN_PATH_MAX];
	UINT16  fpn_cgain_ir[KDRV_IFE_IQ_FPN_PATH_MAX];
	UINT16  fpn_cofs_r[KDRV_IFE_IQ_FPN_PATH_MAX];
	UINT16  fpn_cofs_gr[KDRV_IFE_IQ_FPN_PATH_MAX];
	UINT16  fpn_cofs_gb[KDRV_IFE_IQ_FPN_PATH_MAX];
	UINT16  fpn_cofs_b[KDRV_IFE_IQ_FPN_PATH_MAX];
	UINT16  fpn_cofs_ir[KDRV_IFE_IQ_FPN_PATH_MAX];
} KDRV_IFE_IQ_FPN;

/************************
    misc structure
************************/
typedef struct {
	UINT32 ratio_base;
	USIZE winsz_ratio;									///< va window size of each window, max 1024x1024
	URECT indep_roi_ratio[KDRV_IFE_IQ_VA_INDEP_NUM];	///< va indep-window size, max 1024x1024
} KDRV_IFE_IQ_VA_WIN;

/************************
    main structure
************************/
typedef struct {
	KDRV_IFE_IQ_NRS			nrs0;
	KDRV_IFE_IQ_FCURVE		fcurve;
	KDRV_IFE_IQ_FUSION		fusion;
	KDRV_IFE_IQ_OUTL		outl;
	KDRV_IFE_IQ_FILTER		filt;
	KDRV_IFE_IQ_DGAIN		dgain;
	KDRV_IFE_IQ_CGAIN		cgain;
	KDRV_IFE_IQ_VIG			vig;
	KDRV_IFE_IQ_GBAL		gbal;
	KDRV_IFE_IQ_WDR			wdr;
	KDRV_IFE_IQ_WDR_SUBIMG	wdr_subimg;
	KDRV_IFE_IQ_HIST		hist;
	KDRV_IFE_IQ_VA			va;
	KDRV_IFE_IQ_SUBISP_IQ	subisp_iq;
	KDRV_IFE_IQ_FPN			fpn;

	KDRV_IFE_IQ_VA_WIN		va_window;
	KDRV_IFE_UPDATE 		update;
} KDRV_IFE_IQ_CFG;

#endif
/*******************************************************************************
*	                          IPP IQ parameter End                             *
*******************************************************************************/

#define KDRV_IFE_NRS1_STR_NUM		6
/* NT98538 Remove
#define KDRV_IFE_FCURVE_END_NUM		16
*/
/* NT98530 Remove
#define KDRV_IFE_FCURVE_IDX_NUM		32
#define KDRV_IFE_FCURVE_SPLIT_NUM	32
#define KDRV_IFE_FCURVE_VAL_NUM		65
*/
#define KDRV_IFE_RINGBUF_NUM		(1)
#define HISTOGRAM_STCS_NUM      (128)
#define KDRV_IFE_WDR_BUF_SIZE   (18432) // 48 x 48 x 8 (64bit)
#define KDRV_IFE_SUBISP_SIZE       (0)
#define KDRV_IFE_VA_INDEP_WIN_MAX  (1024) // 538: 1024
#define KDRV_IFE_VA_MAX_WINNUM     (8*8)
#define KDRV_IFE_STRP_NUM_MAX      (8)
#define KDRV_IFE_COMBINE_NUM_MAX   (8)
#define KDRV_IFE_VA_BUF_SIZE       (2048) // 8(h-blks) x 8(v-blks) x 2(g1/g2)x 2(h/v) x 8(byte) = 2048

typedef enum {
	KDRV_IFE_PROC_MODE_LINKLIST = 0,	/* linlist mode */
	KDRV_IFE_PROC_MODE_CPU,				/* cpu write register, only for debug */
	KDRV_IFE_PROC_MODE_MAX,
} KDRV_IFE_PROC_MODE;

typedef enum {
	KDRV_IFE_QUERY_REG_NUM = 0,         // [get] get reg num for memory alloc.           data_type: UINT32
	KDRV_IFE_QUERY_STRP_INFO = 1,       // [get] get overlap info for stripe calculation data_type: KDRV_IFE_STRP_INFO
	KDRV_IFE_QUERY_MOTION_BUF_SIZE = 2,	// [get] get MOTION buffer size					 data_type: KDRV_IFE_BNR_BUF_SIZE_INFO
	KDRV_IFE_QUERY_ID_MAX,
} KDRV_IFE_QUERY_ID;

typedef enum {
	KDRV_IFE_VIG_CH_R = 0,						// R  channel for RGGB & RGBIr
	KDRV_IFE_VIG_CH_GR = 1,						// Gr channel for RGGB
	KDRV_IFE_VIG_CH_GB = 2,						// Gb channel for RGGB
	KDRV_IFE_VIG_CH_G = KDRV_IFE_VIG_CH_GR,		// G  channel for        RGBIr
	KDRV_IFE_VIG_CH_IR = KDRV_IFE_VIG_CH_GB,	// Ir channel for        RGBIr
	KDRV_IFE_VIG_CH_B = 3,						// B  channel for RGGB & RGBIr
	KDRV_IFE_VIG_CH_MAX = 4,
} KDRV_IFE_VIG_CH;

typedef enum{
	KDRV_IFE_HDR_REF_SIE1   = 0x00000001, //reference channel: sie1
	KDRV_IFE_HDR_REF_SIE2   = 0x00000002, //reference channel: sie2
	//KDRV_IFE_HDR_REF_SIE3   = 0x00000004, //reference channel: sie3
	KDRV_IFE_HDR_REF_UNKOWN = 0x00000008, //reference channel: unknown
}KDRV_IFE_HDR_REF_CHK;

typedef enum {
	KDRV_IFE_WDR_OUT         = 0,        ///< WDR out
	KDRV_IFE_SUBISP_OUT      = 1,        ///< SUBISP out
	ENUM_DUMMY4WORD(KDRV_IFE_OUT_SEL)
} KDRV_IFE_OUT_SEL;

#if 0
#endif

/**************************************/
/*                                    */
/*    ISR CB private data             */
/*                                    */
/**************************************/
typedef struct{
	UINT32 ife_clk;
}KDRV_IFE_ISR_CB_DATA;

/**************************************/
/*                                    */
/*    JOB CFG -  IO CFG structure     */
/*                                    */
/**************************************/

typedef struct {
	UINT8 en; 		// 0 : original, 1 : output image horizontal mirror
} KDRV_IFE_MIRROR;

typedef enum {
	KDRV_IFE_ENCODE_RATE_50     = 0,     // RDE encode rate 50
	KDRV_IFE_ENCODE_RATE_58     = 1,     // RDE encode rate 58
	//KDRV_IFE_ENCODE_RATE_66     = 2,     // RDE encode rate 66
	//KDRV_IFE_ENCODE_RATE_41     = 3,     // RDE encode rate 41
	KDRV_IFE_ENCODE_RATE_UNKNOWN,
} KDRV_IFE_ENCODE_RATE;

typedef enum {
	KDRV_IFE_ENCODE_ADJMODE_NORMAL = 0,
	KDRV_IFE_ENCODE_ADJMODE_AGGRESIVE,
	KDRV_IFE_ENCODE_ADJMODE_UNKNOWN,
} KDRV_IFE_ENCODE_ADJMODE;

typedef struct {
	KDRV_IFE_ENCODE_RATE  encode_rate;  	// RDE encode rate of bitstream per segment
	KDRV_IFE_ENCODE_ADJMODE encode_adjmode;	// RDE encode adjmode
} KDRV_IFE_RDE;

typedef struct{
	UINT8   en;         // 0 : used full buffer, 1: enable ring buffer (only direct mode with multi-frames support)
	UINT16  line_num;   // frame1 ring buffer line number
}KDRV_IFE_RINGBUF_CTRL;

typedef struct {
	KDRV_IFE_RINGBUF_CTRL ring[KDRV_IFE_RINGBUF_NUM];
	UINT8   imm_start;		// 0: ring buffer line counter will start counting after receiving SIE2 sync signal
							// 1: ring buffer line counter will start counting immediately
} KDRV_IFE_RING_BUF;

typedef struct {
	IPOINT center[KDRV_IFE_VIG_CH_MAX]; // VIG center position. range : -8192~8191. please used KDRV_IFE_VIG_CH to assign KDRV_IFE_VIG_CH_MAX index
} KDRV_IFE_VIG_POS;

typedef enum{
	KDRV_IFE_MOTION_TYPE_1_64 = 0,
	KDRV_IFE_MOTION_TYPE_1_16,
	KDRV_IFE_MOTION_TYPE_1_4,
}KDRV_IFE_MOTION_TYPE;

#if 0
#endif

/**************************************/
/*                                    */
/*    JOB CFG -  IQ CFG structure     */
/*                                    */
/**************************************/


/**
    IFE dbg info
*/
typedef struct{

	UINT32  ife_overflow_cnt;
	UINT32  ife_buf_overflow_status;
	UINT32 *ife_buf_rec_status;
	UINT32  ife_buf_current_status;
	UINT32  ife_record_idx;
	UINT32  ife_record_num;
	UINT32  ife_frm_ed_cnt;
	UINT32  ife_frm_st_cnt;
	UINT32  ife_r_dec_err1_cnt;
	UINT32  ife_r_dec_err2_cnt;
	UINT32  ife_r_dec_err3_cnt;
	UINT32  ife_ll_ed_cnt;
	UINT32  ife_ll_error_cnt;
	UINT32  ife_ll_error2_cnt;
	UINT32  ife_bufovfl_cnt;
	UINT32  ife_ring_buf_err1_cnt;
	UINT32  ife_ring_buf_err2_cnt;
	UINT32  ife_frame_err_cnt;
	UINT32  ife_p0_rdy;
	UINT32  ife_p0_clr;
	UINT32  ife_p1_rdy;
	UINT32  ife_p1_clr;

}KDRV_IFE_DBG_INFO;
#if(IFE_538_KDRV_FEATURE == 0)
/**
    IFE nrs1 strength func
*/

typedef struct {
	UINT8   en;								// NRS1 enable
	UINT16  str[KDRV_IFE_NRS1_STR_NUM];		// NRS1 strength. range 0~9.
} KDRV_IFE_NRS1_PARAM;
#endif
/* NT98530 remove
typedef struct {
	UINT16  val_lut[KDRV_IFE_FCURVE_VAL_NUM];	// Fcurve Value. range 0~4095.
} KDRV_IFE_VAL_LUT;

typedef struct {
	UINT8   split_lut[KDRV_IFE_FCURVE_SPLIT_NUM];	// Fcurve Split. range 0~3.
} KDRV_IFE_SPLIT_LUT;

typedef struct {
	UINT8   idx_lut[KDRV_IFE_FCURVE_IDX_NUM];	// Fcurve Index. range 0~63.
} KDRV_IFE_IDX_LUT;
*/

typedef struct {
	BOOL random_lsb_enable; // used in WDR for 8bit -> 12bit
	BOOL random_reset;
	BOOL uv_lpf_enable; //lpf for 444 to 422 or 420
	BOOL io_stop;
} KDRV_IFE_YUV_IN_PARAM;

/**
	IFE WDR DRAM ADDR
 */
typedef struct{
	ULONG va;
	ULONG pa;
}KDRV_IFE_BUF_ADDR;


/**
    IFE KDRV Histogram Output Rslt
*/
typedef struct _KDRV_IFE_HIST_RSLT {
	UINT16 hist_stcs[HISTOGRAM_STCS_NUM];       ///< histogram statistics, 128 entries
} KDRV_IFE_HIST_RSLT ;



/**
    IFE KDRV Subisp
*/
typedef struct _KDRV_IFE_SUBISP_PARAM {
	BOOL subisp_out_en;
	UINT16 crop_width;
	UINT16 crop_height;
	UINT8 subsample_rate_x;
	UINT8 subsample_rate_y;
	KDRV_IPP_FRAME subisp_frm;
} KDRV_IFE_SUBISP_PARAM;

typedef struct {
	UINT16 alignment;
	UINT16 overlap;
	KDRV_IPP_FMT in_format;
} KDRV_IFE_STRP_INFO;

typedef struct {
	/* input parameter */
	UINT32 pre_in_width;
	UINT32 pre_in_height;
	KDRV_IFE_MOTION_TYPE motion_type;

	/* buffer size result */
	UINT32 get_motion_map_size;    ///< get gamma map buffer size
	UINT32 get_motion_map_lofs;    ///< get gamma map buffer lineoffset
} KDRV_IFE_MOTION_BUF_SIZE_INFO;


typedef struct {
	BOOL enable;
	UINT8 va_en;                ////<DMA channel for VA
} IFE_ENG_DMA_OUT_INFO;

typedef enum {
	KDRV_IFE_LEFT   = 0,
	KDRV_IFE_RIGHT  = 1,
	KDRV_IFE_UNKNOWN = 2,
}KDRV_IFE_DUAL_IDX;

typedef enum {
	KDRV_SIE1   = 0,
	KDRV_SIE4   = 1,
	KDRV_SIE7   = 2,
	KDRV_SIE10  = 3,
}KDRV_IFE_COMBINE_SRC;

typedef struct{
    BOOL combine_mode_en;
	UINT32 strp_num;
	KDRV_IFE_COMBINE_SRC src_sel[KDRV_IFE_COMBINE_NUM_MAX];
	UINT32 strp_size[KDRV_IFE_COMBINE_NUM_MAX];
	UINT32 left_ovlp[KDRV_IFE_COMBINE_NUM_MAX];
	UINT32 right_ovlp[KDRV_IFE_COMBINE_NUM_MAX];
}KDRV_IFE_COMBINE_MODE_PARAM;

typedef enum _KDRV_IFE_NN_ISP_PATH_ {
	KDRV_IFE_NN_ISP_PATH0 = 0,
	KDRV_IFE_NN_ISP_PATH1 = 1,
	ENUM_DUMMY4WORD(KDRV_IFE_NN_ISP_PATH_SEL)
} KDRV_IFE_NN_ISP_PATH_SEL;

typedef struct{
	BOOL update;

	BOOL path_en;
	BOOL fw_handshake_en;
	UINT32 proc_id;

	KDRV_IFE_BUF_ADDR ringbuf_addr;
	KDRV_IFE_BUF_ADDR outbuf_addr;
	UINT32 lofs;

	UINT32 slice_height;
	UINT32 slice_ovlp;

	UINT32 outbuf_height;
	UINT32 ringbuf_height;

	UINT32 nn_max_strp;

}KDRV_IFE_NN_ISP_PATH;

typedef struct{
	BOOL glb_nn_isp_en;
	KDRV_IFE_NN_ISP_PATH nn_isp_p0;
	KDRV_IFE_NN_ISP_PATH nn_isp_p1;
}KDRV_IFE_NN_ISP_PARAM;

typedef struct {
	BOOL va_en;
	UINT32 win_num_x;
	UINT32 win_num_y;
    UINT32 win_start_x;
    UINT32 win_size_x;
    UINT32 win_skip_x;
    UINT32 dual_cut_pos;
    BOOL outsel;
	ULONG address_pipe1;
	ULONG address_pipe2;
	ULONG address_out;
	ULONG lineoffset;
} KDRV_IFE_VA_RST_SETTING_DUAL;

typedef struct {
	UINT32* g1_h;
	UINT32* g1_v;
	UINT32* g2_h;
	UINT32* g2_v;
	UINT32* g1_h_cnt;
	UINT32* g1_v_cnt;
	UINT32* g2_h_cnt;
	UINT32* g2_v_cnt;
	KDRV_IFE_VA_RST_SETTING_DUAL va_setting;
} KDRV_IFE_VA_RST_DUAL;

typedef struct {
	BOOL va_en;
	UINT32 win_num_x;
	UINT32 win_num_y;
	BOOL outsel;
	KDRV_IFE_BUF_ADDR address;
	UINT32 lineoffset;
} KDRV_IFE_VA_RST_SETTING;

typedef struct {
	UINT32 p_luma[KDRV_IFE_VA_MAX_WINNUM];
	UINT32 g1_h[KDRV_IFE_VA_MAX_WINNUM];
	UINT32 g1_v[KDRV_IFE_VA_MAX_WINNUM];
	UINT32 g2_h[KDRV_IFE_VA_MAX_WINNUM];
	UINT32 g2_v[KDRV_IFE_VA_MAX_WINNUM];
	UINT32 g1_h_cnt[KDRV_IFE_VA_MAX_WINNUM];
	UINT32 g1_v_cnt[KDRV_IFE_VA_MAX_WINNUM];
	UINT32 g2_h_cnt[KDRV_IFE_VA_MAX_WINNUM];
	UINT32 g2_v_cnt[KDRV_IFE_VA_MAX_WINNUM];
	KDRV_IFE_VA_RST_SETTING va_setting;
} KDRV_IFE_VA_RST;

typedef struct {
	UINT32 va_luma[KDRV_IFE_IQ_VA_INDEP_NUM];
	UINT32 g1_h[KDRV_IFE_IQ_VA_INDEP_NUM];
	UINT32 g1_v[KDRV_IFE_IQ_VA_INDEP_NUM];
	UINT32 g2_h[KDRV_IFE_IQ_VA_INDEP_NUM];
	UINT32 g2_v[KDRV_IFE_IQ_VA_INDEP_NUM];
	UINT32 g1_h_cnt[KDRV_IFE_IQ_VA_INDEP_NUM];
	UINT32 g1_v_cnt[KDRV_IFE_IQ_VA_INDEP_NUM];
	UINT32 g2_h_cnt[KDRV_IFE_IQ_VA_INDEP_NUM];
	UINT32 g2_v_cnt[KDRV_IFE_IQ_VA_INDEP_NUM];
} KDRV_IFE_VA_INDEP_RSLT;

typedef struct {
	KDRV_IPP_OPMODE mode;
	KDRV_IPP_OP_STATUS ipp_status; // [0]: ipp is start [1]: ipp is stop
	KDRV_IPP_FRAME in_frm;		// KDRV_IPP_PLANE ([0]:frame0, [1]:frame1,only multi-frames valid)
	KDRV_IPP_FRAME out_frm;		// KDRV_IPP_PLANE ([0]:frame0,only frame0 valid)
	UINT8 in_sel;				// IFE_INPUT_SEL only available when IPP mode 0: IFE input from DRAM, 1: IFE input from PRE
	UINT8 out_sel;				// IFE_OUTPUT_SEL only available when IPP mode 0: IFE output to DRAM, 1: IFE output to IPE
	URECT crp_window;
	UINT32 in_job_num;			// internal reference
	UINT32 inte_en;				// data type: KDRV_IFE_INTERRUPT
	UINT8 thermal_mode_en;
	//539A add
	UINT8 motion_sel;			// IFE_INPUT_MOTION_SEL
	KDRV_IFE_MOTION_TYPE motion_type;   ///Diff size type of motion map

	KDRV_IFE_MIRROR       mirror;       // mirror information.
	KDRV_IFE_RDE          rde;          // raw decode information.
	KDRV_IFE_RING_BUF     ring_buf;     // ring buffer information.
	KDRV_IFE_VIG_POS      vig_pos;      // vig center position.
	KDRV_IFE_HDR_REF_CHK  hdr_ref_chk;  // hdr reference channel check
	KDRV_IFE_YUV_IN_PARAM yuv_in_info;  // yuv in
	BOOL wdr_sub_out_en;                // subisp out enable or wdr subimg out enable
	//538 added
	UINT16	subimg_lofs_in;				///< wdr subin lineoffset, lofs >= (subimg_size_h * 8)
	UINT16	subimg_lofs_out;			///< wdr subout lineoffset, lofs >= (subimg_size_h * 8)

	KDRV_IFE_BUF_ADDR wdr_sub_out_addr; // wdr subout addr
	KDRV_IFE_BUF_ADDR wdr_sub_in_addr;  // wdr subin addr

	KDRV_IFE_SUBISP_PARAM subisp_param;

	KDRV_IFE_OUT_SEL dram_out_sel;      // subisp out or main flow out
	BOOL dram_out_en;

	KDRV_IFE_BUF_ADDR va_out_addr;
	UINT32 va_lofs;
	ULONG va_out_addr_ofs;

	BOOL multi_ipp_en;

    UINT32 ife_strp[KDRV_IFE_STRP_NUM_MAX];
    UINT8  ife_strp_num;
    UINT32 ife_ovlp;

    UINT32 va_win_stx;    // internal reference
    UINT32 va_win_size_x; // internal reference
    UINT32 va_win_skip_x; // internal reference

    KDRV_IFE_COMBINE_MODE_PARAM combine_param;

    BOOL dual_en;
    KDRV_IFE_DUAL_IDX dual_idx;
    UINT16 dual_start_ofs;
    UINT32 dual_left_width;
    UINT32 dual_right_width;
    UINT16 dual_left_ovlp;
    UINT16 dual_right_ovlp;

    KDRV_IFE_NN_ISP_PARAM nn_isp_param;
    BOOL nn_isp_drop_frm_flg;

} KDRV_IFE_IO_CFG;

/*
    KDRV IFE JOB CFG
*/
typedef struct {
	KDRV_IFE_IO_CFG *p_iocfg;	// [set][get] IFE engine in/out configuration
	KDRV_IFE_IQ_CFG *p_iqcfg;	// [set][get] IFE engine IQ configuration
	void *p_ll_blk;				// [set] ipp queue information
} KDRV_IFE_JOB_CFG;

INT32 kdrv_ife_module_init(void);
INT32 kdrv_ife_module_uninit(void);

INT32 kdrv_ife_open(UINT32 chip, UINT32 engine);
INT32 kdrv_ife_close(UINT32 chip, UINT32 engine);

#ifdef CONFIG_PM
INT32 kdrv_ife_suspend(UINT32 chip, UINT32 engine);
INT32 kdrv_ife_resume(UINT32 chip, UINT32 engine);
#endif

INT32 kdrv_ife_query(UINT32 id, KDRV_IFE_QUERY_ID qid, void *p_param);
INT32 kdrv_ife_set(UINT32 id, KDRV_IFE_PARAM_ID param_id, void *p_param);
INT32 kdrv_ife_get(UINT32 id, KDRV_IFE_PARAM_ID param_id, void *p_param);

INT32 kdrv_ife_rtos_init(void);
INT32 kdrv_ife_rtos_uninit(void);

void kdrv_ife_dump(void);
void kdrv_ife_dump_cfg(KDRV_IFE_JOB_CFG *p_cfg, int (*KDRV_IFE_DUMP)(const char *fmt, ...));
void kdrv_ife_dump_register(void);
void kdrv_ife_dump_register_log(void);
UINT32 kdrv_ife_dbg_mode(void *p_hdl, UINT32 param_id, void *data);

#endif
