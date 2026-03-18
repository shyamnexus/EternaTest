/**
    IPL Ctrl Layer, ISP Interface

    @file       ctl_ipp_isp.h
    @ingroup    mIIPL_Ctrl
    @note       None

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/


#ifndef _CTL_IFE_ISP_H
#define _CTL_IFE_ISP_H

/*******************************************************************************
*	Following #if 1 ... #endif block include all IPP IQ parameter.             *
*	These content must sync with kdrv IQ parameter. (see kdrv_ife.h)           *
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

#define CTL_IFE_ISP_NRS_STR_NUM						6
#define CTL_IFE_ISP_FCURVE_Y_W_NUM					17
#define CTL_IFE_ISP_FCURVE_L_NUM					65
#define CTL_IFE_ISP_FCURVE_R_NUM					17
#define CTL_IFE_ISP_FUSION_MC_DIFF_W_NUM			16
#define CTL_IFE_ISP_FUSION_BLD_CUR_KNEE_NUM			1
#define CTL_IFE_ISP_FUSION_DARK_SAT_NUM				2
#define CTL_IFE_ISP_OUTL_BRI_TH_NUM					5
#define CTL_IFE_ISP_OUTL_DARK_TH_NUM				5
#define CTL_IFE_ISP_OUTL_CNT_NUM					2
#define CTL_IFE_ISP_OUTL_ORD_W_NUM					8
#define CTL_IFE_ISP_SPATIAL_W_LEN					6
#define CTL_IFE_ISP_RANGE_A_TH_NUM					6
#define CTL_IFE_ISP_RANGE_B_TH_NUM					6
#define CTL_IFE_ISP_RANGE_A_LUT_SIZE				17
#define CTL_IFE_ISP_RANGE_B_LUT_SIZE				17
#define CTL_IFE_ISP_RBFILL_LUMA_NUM					17
#define CTL_IFE_ISP_RBFILL_RATIO_NUM				32
#define CTL_IFE_ISP_VIG_CH0_LUT_SIZE				17
#define CTL_IFE_ISP_VIG_CH1_LUT_SIZE				17
#define CTL_IFE_ISP_VIG_CH2_LUT_SIZE				17
#define CTL_IFE_ISP_VIG_CH3_LUT_SIZE				17
#define CTL_IFE_ISP_GBAL_OFS_NUM					17
#define CTL_IFE_ISP_WDR_SUBIMG_FILT_NUM				3
#define CTL_IFE_ISP_WDR_COEF_NUM					4
#define CTL_IFE_ISP_WDR_INPUT_BLD_NUM				17
#define CTL_IFE_ISP_WDR_INPUT_YV_BLD_NUM			9
#define CTL_IFE_ISP_WDR_NEQ_TABLE_L_NUM				65
#define CTL_IFE_ISP_WDR_NEQ_TABLE_R_NUM				17
#define CTL_IFE_ISP_WDR_FBC_TH_NUM					6
#define CTL_IFE_ISP_VA_INDEP_NUM					5
#define CTL_IFE_ISP_VA_GAMMA_LEN					17
#define CTL_IFE_ISP_SUBISP_COEF_LEN					9
#define CTL_IFE_ISP_SUBISP_GAMMA_LEN				17
#define CTL_IFE_ISP_FPN_PATH_MAX					1

/************************
    IFE nrs func
************************/
typedef struct {
	UINT8 enable;							///<  Enable/Disable
	UINT16 str[CTL_IFE_ISP_NRS_STR_NUM];	///<  NRS strength. range 0~9.
} CTL_IFE_ISP_NRS;

/************************
    IFE fcurve func
************************/
typedef enum {
	CTL_IFE_ISP_F_CURVE_Y_8G4R4B = 0,
	CTL_IFE_ISP_F_CURVE_Y_4G = 1,
	CTL_IFE_ISP_F_CURVE_Y_ORI = 2,
	ENUM_DUMMY4WORD(CTL_IFE_ISP_F_CURVE_Y_SEL)
} CTL_IFE_ISP_F_CURVE_Y_SEL;

typedef struct {
	CTL_IFE_ISP_F_CURVE_Y_SEL	y_mean_sel;	///< Rnage 0~2. Fcurve Y Mean Method selection.
	UINT8	yv_w;							///< Range 0~8. Fcurve Y mean and V weight.
	UINT8	ev_fmt;
} CTL_IFE_ISP_FCURVE_CTRL;

typedef struct {
	UINT8	y_w_lut[CTL_IFE_ISP_FCURVE_Y_W_NUM];	///< Range 0~255. Fcurve Y weight.
} CTL_IFE_ISP_Y_W;

typedef struct {
	UINT32	fcur_l_lut[CTL_IFE_ISP_FCURVE_L_NUM];	///< Range 0~1048576. Fcurve Left Part Value.
} CTL_IFE_ISP_FCURVE_L_LUT;

typedef struct {
	UINT32	fcur_r_lut[CTL_IFE_ISP_FCURVE_R_NUM];	///< Range 0~1048576. Fcurve Right Part Value.
} CTL_IFE_ISP_FCURVE_R_LUT;

typedef struct {
	UINT8						enable;		///< Enable/Disable of f curve in IFE.
	CTL_IFE_ISP_FCURVE_CTRL		fcur_ctrl;	///< Fcurve control Y Mean Method and V weight.
	CTL_IFE_ISP_Y_W				y_weight;	///< Fcurve Y weight
	CTL_IFE_ISP_FCURVE_L_LUT	fcurve_l;	///< Fcurve Left Part
	CTL_IFE_ISP_FCURVE_R_LUT	fcurve_r;	///< Fcurve Right Part
} CTL_IFE_ISP_FCURVE;

/************************
    IFE fusion func
************************/
/**
    range of color gain coefficients
*/
typedef enum {
	CTL_IFE_ISP__2_8 = 0,		///< 2bits decimal, 8bits fraction
	CTL_IFE_ISP__3_7 = 1,		///< 3bits decimal, 7bits fraction
	ENUM_DUMMY4WORD(CTL_IFE_ISP_GAIN_FIELD)
} CTL_IFE_ISP_GAIN_FIELD;

typedef enum {
	CTL_IFE_ISP__8_8 = 0,		///< 8bits decimal, 8bits fraction
	CTL_IFE_ISP__9_7 = 1,		///< 9bits decimal, 7bits fraction
	ENUM_DUMMY4WORD(CTL_IFE_ISP_FCGAIN_FIELD)
} CTL_IFE_ISP_FCGAIN_FIELD;

typedef enum {
	CTL_IFE_ISP_Y_SEL_8G4R4B = 0,
	CTL_IFE_ISP_Y_SEL_4G = 1,
	CTL_IFE_ISP_Y_SEL_RGB_MAX = 2,
	CTL_IFE_ISP_Y_SEL_ORI = 3,
} CTL_IFE_ISP_Y_SEL;

typedef enum {
	CTL_IFE_ISP_MODE_FUSION = 0,
	CTL_IFE_ISP_MODE_SHORT_EXP = 1,
	CTL_IFE_ISP_MODE_LONG_EXP = 2,
	//CTL_IFE_ISP_MODE_VERY_SHORT_EXP = 3,
} CTL_IFE_ISP_MODE_SEL;

/**
    IFE fusion ctrl func
*/
typedef struct {
	CTL_IFE_ISP_Y_SEL		y_mean_sel;	// Y mean option. range 0~3.
	CTL_IFE_ISP_MODE_SEL	mode;		// Fusion mode. range 0~3. (0: Fusion; 1: Short frame; 2: Long frame; 3: Long real)
	UINT16					ev_ratio;	// EV ratio. range 0~511. (4EV=16*16, 3EV = 8*16, 2EV=2*16, 1EV=1*16)
} CTL_IFE_ISP_FUSION_CTRL;

typedef enum {
	CTL_IFE_ISP_FUSION_BY_L_EXP = 0,
	CTL_IFE_ISP_FUSION_BY_S_EXP = 1,
	CTL_IFE_ISP_FUSION_BOTH_EXP = 2,
} CTL_IFE_ISP_FUSION_NOR_SEL;

/**
    IFE fusion blend curve
*/
typedef struct {
	CTL_IFE_ISP_FUSION_NOR_SEL nor_sel;							///< Range 0~2. Normal blending curve weighting option.
	CTL_IFE_ISP_FUSION_NOR_SEL dif_sel;							///< Range 0~2. Difference blending curve weighting option.

	/* normal blending - long exposure param. */
	UINT16	l_nor_knee[CTL_IFE_ISP_FUSION_BLD_CUR_KNEE_NUM];	///< range 0~4095.  Knee point of normal blending curve for long exposure.
	UINT16	l_nor_range;										///< range 16~4095. Normal blending curve range for long  exposure. Actually range = (1<<l_nor_range).
	UINT16	l_nor_slope;										///< range 0~65535. Weighting function slope.

	/* normal blending - short exposure param. */
	UINT16	s_nor_knee[CTL_IFE_ISP_FUSION_BLD_CUR_KNEE_NUM];	///< range 0~4095.  Knee point of normal blending curve for short exposure.
	UINT16	s_nor_range;										///< range 16~4095. Normal blending curve range for short  exposure.
	UINT16	s_nor_slope;										///< range 0~65535. Weighting function slope.

	/* difference blending - long exposure param. */
	UINT16	l_dif_knee[CTL_IFE_ISP_FUSION_BLD_CUR_KNEE_NUM];	///< range 0~4095.  Knee point of difference blending curve for long exposure.
	UINT16	l_dif_range;										///< range 16~4095. Difference blending curve range for long  exposure. Actually range = (1<<l_dif_range).
	UINT16	l_dif_slope;										///< range 0~65535. Weighting function slope.

	/* difference blending - short exposure param. */
	UINT16	s_dif_knee[CTL_IFE_ISP_FUSION_BLD_CUR_KNEE_NUM];	///< range 0~4095.  Knee point of difference blending curve for short exposure.
	UINT16	s_dif_range;										///< range 16~4095. Difference blending curve range for short  exposure. Actually range = (1<<l_dif_range).
	UINT16	s_dif_slope;										///< range 0~65535. Weighting function slope.

} CTL_IFE_ISP_BLEND_CURVE;

/**
    IFE fusion mc
*/
typedef struct {
	UINT8 lum_th;											///< Range 0~255. Lower than this threshold using normal blending curve.
	UINT8 diff_ratio;										///< Range 0~3   . Control ratio.
	UINT8 diff_w[CTL_IFE_ISP_FUSION_MC_DIFF_W_NUM];			///< Range 0~16. Difference weighting for difference > 0.
	UINT8 dwd;												///< Range 0~16. Using this ratio if difference less than lum_th.
} CTL_IFE_ISP_MC;

/**
    IFE fusion dark sat reduce
*/
typedef struct {
	UINT16 th[CTL_IFE_ISP_FUSION_DARK_SAT_NUM];				///< Range 0~4095. Threshold of DS.
	UINT8 step[CTL_IFE_ISP_FUSION_DARK_SAT_NUM];			///< Range 0~255 . Weighting step of DS.
	UINT8 low_bound[CTL_IFE_ISP_FUSION_DARK_SAT_NUM];		///< range 0~255 . Lower bound of DS.
} CTL_IFE_ISP_DARK_SAT;

/**
    IFE fusion color gain
*/
typedef enum {
	CTL_IFE_ISP_CGAIN_CH_R = 0,					// R  channel for RGGB & RGBIr
	CTL_IFE_ISP_CGAIN_CH_GR,					// Gr channel for RGGB & RGBIr
	CTL_IFE_ISP_CGAIN_CH_GB,					// Gb channel for RGGB & RGBIr
	CTL_IFE_ISP_CGAIN_CH_B,						// B  channel for RGGB & RGBIr
	CTL_IFE_ISP_CGAIN_CH_IR,					// Ir channel for        RGBIr
	CTL_IFE_ISP_CGAIN_CH_MAX,
} CTL_IFE_ISP_CGAIN_CH;

typedef struct {
	UINT8					enable;					// Enable/Disable.
	CTL_IFE_ISP_FCGAIN_FIELD	bit_field;				// select Fusion color gain decimal range. range 0~1.

	/* short exposure param. */
	UINT16  fcgain_s[CTL_IFE_ISP_CGAIN_CH_MAX];		// SHDR short exposure path color gain. range 0~1023. please used CTL_IFE_ISP_CGAIN_CH to assign CTL_IFE_ISP_CGAIN_CH_MAX index
	UINT16  fcofs_s[CTL_IFE_ISP_CGAIN_CH_MAX];		// SHDR short exposure path color offset. range 0~1023. please used CTL_IFE_ISP_CGAIN_CH to assign CTL_IFE_ISP_CGAIN_CH_MAX index

	/* long exposure param. */
	UINT16  fcgain_l[CTL_IFE_ISP_CGAIN_CH_MAX];		// SHDR long exposure path color gain. range 0~1023. please used CTL_IFE_ISP_CGAIN_CH to assign CTL_IFE_ISP_CGAIN_CH_MAX index
	UINT16  fcofs_l[CTL_IFE_ISP_CGAIN_CH_MAX];		// SHDR long exposure path color offset. range 0~1023. please used CTL_IFE_ISP_CGAIN_CH to assign CTL_IFE_ISP_CGAIN_CH_MAX index
} CTL_IFE_ISP_FCGAIN;

/**
    IFE fusion dbg mode
*/
typedef enum {
	CTL_IFE_ISP_DISABLE_DBG_MODE        = 0,     // Disable dbg mode (same as enable=0)
	CTL_IFE_ISP_L_EXPOS_WEIGHT          = 1,     // show long exposure weight
	CTL_IFE_ISP_S_EXPOS_WEIGHT          = 2,     // show short exposure weight
	CTL_IFE_ISP_N_CURVE_WEIGHT          = 3,     // show n curve weight
	CTL_IFE_ISP_D_CURVE_WEIGHT          = 4,     // show d curve weight
	//CTL_IFE_ISP_FIRST_FU_WEIGHT_RESULT  = 5,     // show weighting of first fusion result
	//CTL_IFE_ISP_VERY_S_FRAME_WEIGHT     = 6,     // show weighting of very short frame
	CTL_IFE_ISP_DBG_MODE_UNKNOWN,
} CTL_IFE_ISP_FU_DBG_MODE;

typedef struct {
	BOOL enable;						//enable=0 disable=1
	CTL_IFE_ISP_FU_DBG_MODE mode;		//mode 0~6
} CTL_IFE_ISP_FUSION_DBG;

/**
    IFE fusion func
*/
typedef struct {
	CTL_IFE_ISP_FUSION_CTRL   fu_ctrl;
	CTL_IFE_ISP_BLEND_CURVE   bld_cur;
	CTL_IFE_ISP_MC            mc_para;
	CTL_IFE_ISP_DARK_SAT      dk_sat;
	CTL_IFE_ISP_FCGAIN        fu_cgain;
	CTL_IFE_ISP_FUSION_DBG    dbg;
} CTL_IFE_ISP_FUSION;

/************************
    IFE outlier func
************************/
typedef struct {
	UINT8	enable;											///< Enable/Disable
	UINT16	bright_th[CTL_IFE_ISP_OUTL_BRI_TH_NUM];			///< range: 0~4095
                                                            ///< outlier bright case threshold
	UINT16	dark_th [CTL_IFE_ISP_OUTL_DARK_TH_NUM];			///< range: 0~4095
                                                            ///< outlier dark case threshold
	UINT8	outl_cnt[CTL_IFE_ISP_OUTL_CNT_NUM];				///< range: 0~16  , UINT32[2], outlier neighbor point counter
	UINT8	outl_weight;									///< range: 0~255 , outlier weighting
	UINT16	dark_ofs;										///< range: 0~4095, outlier dark th offset
	UINT16	bright_ofs;										///< range: 0~4095, outlier bright th offset

	UINT8	outl_comp_mode;

	UINT8	ord_rng_bri;									///< range: 0~7   , control the strength of order method for bright defect pixel.
	UINT8	ord_rng_dark;									///< range: 0~7   , control the strength of order method for dark defect pixel.
	UINT16	ord_protect_th;									///< range: 0~1023, order method protect artifact th
	UINT8	ord_blend_w;									///< range: 0~255 , outlier and order result blending weight
	UINT8	ord_bri_w[CTL_IFE_ISP_OUTL_ORD_W_NUM];			///< range: 0~8   , order bright defect pixel compensate weight
	UINT8	ord_dark_w[CTL_IFE_ISP_OUTL_ORD_W_NUM];			///< range: 0~8   , order dark defect pixel compensate weight
	UINT8	outl_rgbir_rb_w;
	UINT8	ord_rgbir_rb_w;
} CTL_IFE_ISP_OUTL;

/************************
    IFE bilateral filter
************************/
typedef struct {
	UINT8  weight[CTL_IFE_ISP_SPATIAL_W_LEN];		///< range:0~31, UINT32[6], filter spatial weight,
} CTL_IFE_ISP_SPATIAL;

typedef struct {
	UINT16  a_th [CTL_IFE_ISP_RANGE_A_TH_NUM];		///< range:0~1023, UINT32[6] , range filter A threshold
	UINT16  a_lut[CTL_IFE_ISP_RANGE_A_LUT_SIZE];	///< range:0~1023, UINT32[17], range filter A threshold adjustment
	UINT16  b_th [CTL_IFE_ISP_RANGE_B_TH_NUM];		///< range:0~1023, UINT32[6] , range filter B threshold
	UINT16  b_lut[CTL_IFE_ISP_RANGE_B_LUT_SIZE];	///< range:0~1023, UINT32[17], range filter B threshold adjustment
} CTL_IFE_ISP_RANGE_FILTER_R;

typedef struct {
	UINT16  a_th [CTL_IFE_ISP_RANGE_A_TH_NUM];		///< range:0~1023, UINT32[6] , range filter A threshold
	UINT16  a_lut[CTL_IFE_ISP_RANGE_A_LUT_SIZE];	///< range:0~1023, UINT32[17], range filter A threshold adjustment
	UINT16  b_th [CTL_IFE_ISP_RANGE_B_TH_NUM];		///< range:0~1023, UINT32[6] , range filter B threshold
	UINT16  b_lut[CTL_IFE_ISP_RANGE_B_LUT_SIZE];	///< range:0~1023, UINT32[17], range filter B threshold adjustment
} CTL_IFE_ISP_RANGE_FILTER_GR;

typedef struct {
	UINT16  a_th [CTL_IFE_ISP_RANGE_A_TH_NUM];		///< range:0~1023, UINT32[6] , range filter A threshold
	UINT16  a_lut[CTL_IFE_ISP_RANGE_A_LUT_SIZE];	///< range:0~1023, UINT32[17], range filter A threshold adjustment
	UINT16  b_th [CTL_IFE_ISP_RANGE_B_TH_NUM];		///< range:0~1023, UINT32[6] , range filter B threshold
	UINT16  b_lut[CTL_IFE_ISP_RANGE_B_LUT_SIZE];	///< range:0~1023, UINT32[17], range filter B threshold adjustment
} CTL_IFE_ISP_RANGE_FILTER_GB;

typedef struct {
	UINT16  a_th [CTL_IFE_ISP_RANGE_A_TH_NUM];		///< range:0~1023, UINT32[6] , range filter A threshold
	UINT16  a_lut[CTL_IFE_ISP_RANGE_A_LUT_SIZE];	///< range:0~1023, UINT32[17], range filter A threshold adjustment
	UINT16  b_th [CTL_IFE_ISP_RANGE_B_TH_NUM];		///< range:0~1023, UINT32[6] , range filter B threshold
	UINT16  b_lut[CTL_IFE_ISP_RANGE_B_LUT_SIZE];	///< range:0~1023, UINT32[17], range filter B threshold adjustment
} CTL_IFE_ISP_RANGE_FILTER_B;

typedef struct {
	UINT16  a_th [CTL_IFE_ISP_RANGE_A_TH_NUM];		///< range:0~1023, UINT32[6] , range filter A threshold
	UINT16  a_lut[CTL_IFE_ISP_RANGE_A_LUT_SIZE];	///< range:0~1023, UINT32[17], range filter A threshold adjustment
	UINT16  b_th [CTL_IFE_ISP_RANGE_B_TH_NUM];		///< range:0~1023, UINT32[6] , range filter B threshold
	UINT16  b_lut[CTL_IFE_ISP_RANGE_B_LUT_SIZE];	///< range:0~1023, UINT32[17], range filter B threshold adjustment
} CTL_IFE_ISP_RANGE_FILTER_IR;

typedef enum {
	CTL_IFE_ISP_BILAT_CEN_DPC = 0,		// 52x algorithm
	CTL_IFE_ISP_BILAT_CEN_BILATERAL,	// bypass center pixel
} CTL_IFE_ISP_BILAT_CEN_SEL;

typedef struct {
	UINT8	enable;						///<               3x3 center mofify bilateral filter threshold enable
	UINT16  th1;						///< range:0~1023, 3x3 center mofify bilateral filter threshold 1
	UINT16  th2;						///< range:0~1023, 3x3 center mofify bilateral filter threshold 2
	CTL_IFE_ISP_BILAT_CEN_SEL cen_sel;	///<               3x3 center mofify bilateral filter threshold algorithm
} CTL_IFE_ISP_CENTER_MODIFY;

/**
    clamp and weighting setting
*/
typedef struct {
	UINT16  th;			///< range:0~4095, threshold
	UINT8   mul;		///< range:0~255 , weighting multiplier
	UINT16  dlt;		///< range:0~4095, threshold adjustment
} CTL_IFE_ISP_CLAMP;

/**
    IFE structure - IFE RB fill parameter set.
*/
typedef struct {
	UINT8   enable;									///< range: 0~1 , Enable/Disable
	UINT8   luma [CTL_IFE_ISP_RBFILL_LUMA_NUM];		///< range: 0~31,
	UINT8   ratio[CTL_IFE_ISP_RBFILL_RATIO_NUM];	///< range: 0~31,
	UINT8   ratio_mode;								///< range: 0~2 , n3.2, n2.3, n1.4
} CTL_IFE_ISP_RBFILL_PARAM;

typedef struct {
	UINT8							enable;			///< Enable/Disable
	CTL_IFE_ISP_SPATIAL				spatial;		///< filter spatial weight table
	CTL_IFE_ISP_RANGE_FILTER_R		rng_filt_r;		///< range filter ch0
	CTL_IFE_ISP_RANGE_FILTER_GR		rng_filt_gr;	///< range filter ch1
	CTL_IFE_ISP_RANGE_FILTER_GB		rng_filt_gb;	///< range filter ch2
	CTL_IFE_ISP_RANGE_FILTER_B		rng_filt_b;		///< range filter ch3
	CTL_IFE_ISP_RANGE_FILTER_IR		rng_filt_ir;	///< range filter ch2

	CTL_IFE_ISP_CENTER_MODIFY		center_mod;		///< center modify
	CTL_IFE_ISP_CLAMP				clamp;			///< clamp and weighting setting
	CTL_IFE_ISP_RBFILL_PARAM		rbfill;			///< RGBIr fill r/b pixel
	UINT8							blend_w;		///< range:0~255, range filter A and B weighting
	UINT8							rng_th_w;		///< range:0~15 ,range th weight
	UINT8							bin;			///< range:0~7  , ( Denominator: 2<<bin )
} CTL_IFE_ISP_FILTER;

/************************
    IFE digital gain
************************/
typedef struct {
	UINT8 enable;		// Enable/Disable. range:0~1.
	UINT16 dgain;		// digital gain. range 0~65535.
} CTL_IFE_ISP_DGAIN;

/************************
    IFE color gain
************************/
typedef struct {
	UINT8					enable;			///< range:0~1   , Enable/Disable
	UINT8					inv;			///< range:0~1   , color gain invert
	UINT8					hinv;			///< range:0~1   , color gain H-invert
	CTL_IFE_ISP_GAIN_FIELD	bit_field;		///< range:0~1   , color gain bit field
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
} CTL_IFE_ISP_CGAIN;

/************************
    ife vignette shading compensation
************************/
typedef struct {
	UINT8	enable;										///< range: 0~1   , Enable/Disable
	UINT16	dist_th;									///< range: 0~1023, disabled area distance threshold

	UINT16	ch_r_lut[CTL_IFE_ISP_VIG_CH0_LUT_SIZE];		///< range: 0~1023, VIG LUT of R  channel
	UINT16	ch_gr_lut[CTL_IFE_ISP_VIG_CH1_LUT_SIZE];	///< range: 0~1023, VIG LUT of Gr channel
	UINT16	ch_gb_lut[CTL_IFE_ISP_VIG_CH2_LUT_SIZE];	///< range: 0~1023, VIG LUT of Gb channel
	UINT16	ch_b_lut[CTL_IFE_ISP_VIG_CH3_LUT_SIZE];		///< range: 0~1023, VIG LUT of B  channel
	UINT16	ch_ir_lut[CTL_IFE_ISP_VIG_CH2_LUT_SIZE];	///< range: 0~1023, VIG LUT of Ir channel

	UINT8	dither_enable;								///< range: 0~1   , dithering enable/disable
	UINT8	dither_rst_enable;							///< range: 0~1   , dithering reset enable
} CTL_IFE_ISP_VIG;

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
	UINT8	gbal_ofs[CTL_IFE_ISP_GBAL_OFS_NUM];///< range: 0~63  , weight transision region
	UINT16	str_luma_low_bnd;
	UINT16	edge_luma_low_bnd;
} CTL_IFE_ISP_GBAL;

/************************
    IFE KDRV WDR/TONE CURVE Param and Enable/Disable
************************/
/**
    IFE structure - WDR input blending parameters
*/
typedef enum {
	CTL_IFE_ISP_INPUT_BLD_3x3_Y  = 0,   ///< use Y from bayer 3x3 as the input blending source selection
	CTL_IFE_ISP_INPUT_BLD_GMEAN  = 1,   ///< use Gmean from bayer 3x3 as the input blending source selection
	CTL_IFE_ISP_INPUT_BLD_BAYER  = 2,   ///< use Bayer as the input blending source selection
	ENUM_DUMMY4WORD(CTL_IFE_ISP_WDR_INPUT_BLDSEL)
} CTL_IFE_ISP_WDR_INPUT_BLDSEL;

typedef struct {
	CTL_IFE_ISP_WDR_INPUT_BLDSEL bld_sel;
	UINT8 blend_lut[CTL_IFE_ISP_WDR_INPUT_BLD_NUM];				///< blending of Y and raw [0~255], 0: Y, 255: raw
	UINT8 in_yv_blend_lut[CTL_IFE_ISP_WDR_INPUT_YV_BLD_NUM];	///< lut of YV blending [0~63]
} CTL_IFE_ISP_WDR_IN_BLD;

/**
    IFE structure - WDR non-equal table parameters
*/
typedef struct {
	UINT16 left_table[CTL_IFE_ISP_WDR_NEQ_TABLE_L_NUM];		///< non equatable table value [0~4095]
	UINT16 right_table[CTL_IFE_ISP_WDR_NEQ_TABLE_R_NUM];	///< non equatable table value [0~4095]
} CTL_IFE_ISP_WDR_NEQ_TABLE;

typedef enum {
	CTL_IFE_ISP_WDR_NORMAL_MODE = 0,	///< wdr normal mode
	CTL_IFE_ISP_WDR_ANTI_HALO_MODE = 1,	///< wdr anti halo mode
} CTL_IFE_ISP_WDR_MODE;

typedef enum {
	CTL_IFE_ISP_ANTI_HALO_BLENDING = 0, ///< blending anti halo method
	CTL_IFE_ISP_ANTI_HALO_ADAPTIVE = 1,	///< adaptive anti halo method
} CTL_IFE_ISP_ANTI_HALO_OPT;

/**
    IFE structure - WDR strength parameters
*/
typedef struct {
	INT16 wdr_coeff[CTL_IFE_ISP_WDR_COEF_NUM];	///< wdr coefficient [-4096~4095]
	UINT8 strength;								///< wdr strength [0~255]
	CTL_IFE_ISP_WDR_MODE wdr_mode;				///< wdr mode, 0:Normal mode, 1:Anti-halo mode
	CTL_IFE_ISP_ANTI_HALO_OPT wdr_anti_halo_opt;///< wdr anti halo option, 0:blending, 1:adaptive
	UINT8  wdr_halo_ratio;						///< wdr halo ratio [0~255]
	UINT8  wdr_halo_slope;						///< wdr halo slop [0~255]
	UINT16 wdr_b2p_var;							///< wdr b2p var [0~4095]
} CTL_IFE_ISP_WDR_STRENGTH;

/**
    IFE structure - WDR gain control parameters
*/
typedef struct {
	BOOL gainctrl_en;							///< enable gain control for wdr
	UINT8 max_gain;								///< maximum gain of wdr [1~255]
	UINT8 min_gain;								///< minimum gain of wdr [1~255], mapping to 1/256, 2/256, 255/256
} CTL_IFE_ISP_WDR_GAINCTRL;

/**
    IFE structure - WDR output blending parameters
*/
typedef struct {
	BOOL outbld_en;								///< enable output blending for wdr
	CTL_IFE_ISP_WDR_NEQ_TABLE outbld_lut;		///< wdr output blending table
} CTL_IFE_ISP_WDR_OUTBLD;

/**
    IFE structure - WDR saturation reduction parameters
*/
typedef struct {
	UINT16 sat_th;								///< threshold of wdr saturation reduction [0~4095]
	UINT8 sat_wt_low;							///< lower weight of wdr saturation reduction [0~255]
	UINT8 sat_delta;							///< delta of wdr saturation reduction [0~255]
} CTL_IFE_ISP_WDR_SAT_REDUCT;

typedef enum {
	CTL_IFE_ISP_WDR_ROUNDING        = 0,  ///< rounding
	CTL_IFE_ISP_WDR_HALFTONING      = 1,  ///< halftone rounding
	CTL_IFE_ISP_WDR_RANDOMLSB       = 2,  ///< random rounding
	ENUM_DUMMY4WORD(CTL_IFE_ISP_WDR_DITHER_MODE)
} CTL_IFE_ISP_WDR_DITHER_MODE;

/**
    IFE structure - WDR dither parameters.
*/
typedef struct {
	BOOL wdr_dithering_en;
	BOOL wdr_rand_rst;
	CTL_IFE_ISP_WDR_DITHER_MODE wdr_rand_sel;
} CTL_IFE_ISP_WDR_DITHER;

/**
    IFE structure - WDR foreground brightness compensation parameters
*/
typedef struct {
	BOOL    fbc_en;									///< wdr fbc enable, 0:disable, 1:enable
	UINT8   fbc_ratio;								///< wdr fbc ratio [0~255]
	UINT16  fbc_th[CTL_IFE_ISP_WDR_FBC_TH_NUM];		///< wdr th [0~8191]
} CTL_IFE_ISP_WDR_FBC;

typedef struct {
	BOOL wdr_enable;
	BOOL tonecurve_enable;
	UINT8 ftrcoef[CTL_IFE_ISP_WDR_SUBIMG_FILT_NUM];			///< wdr sub-image low pass filter coefficients, 3 entries
	CTL_IFE_ISP_WDR_IN_BLD		input_bld;					///< wdr input blending
	CTL_IFE_ISP_WDR_NEQ_TABLE	tonecurve;					///< wdr tonecurve configuration
	CTL_IFE_ISP_WDR_STRENGTH	wdr_str;					///< wdr strength
	CTL_IFE_ISP_WDR_GAINCTRL	gainctrl;					///< wdr gain control
	CTL_IFE_ISP_WDR_OUTBLD		outbld;						///< wdr output blending settings
	CTL_IFE_ISP_WDR_SAT_REDUCT	sat_reduct;					///< wdr saturation reduction
	CTL_IFE_ISP_WDR_DITHER		dither;						///< wdr sub-img dither
	CTL_IFE_ISP_WDR_FBC			fbc;						///< wdr foreground brightness compensation
	UINT16						wdr_gain_prot_str;			///< wdr gain prot str
} CTL_IFE_ISP_WDR;

/************************
    IFE WDR subimg size Information
************************/
typedef struct {
	UINT8	subimg_size_h;			///< wdr sub-image size [8~48]
	UINT8	subimg_size_v;			///< wdr sub-image size [8~48]
	//UINT16	subimg_lofs_in;			///< wdr subin lineoffset, lofs >= (subimg_size_h * 8)
	//UINT16	subimg_lofs_out;		///< wdr subout lineoffset, lofs >= (subimg_size_h * 8)
} CTL_IFE_ISP_WDR_SUBIMG;

/************************
    IFE Histogram Param and Enable/Disable
************************/
typedef enum {
	CTL_IFE_ISP_BEFORE_WDR	= 0,	///< histogram statistics before wdr algorithm
	CTL_IFE_ISP_AFTER_WDR	= 1,	///< histogram statistics after wdr algorithm
	ENUM_DUMMY4WORD(CTL_IFE_ISP_HIST_SEL)
} CTL_IFE_ISP_HIST_SEL;

typedef struct {
	BOOL hist_enable;				///< Enable histogram statistics
	CTL_IFE_ISP_HIST_SEL hist_sel;	///< selection of statistics input
	UINT8 step_h;					///< h step size of histogram [0~31]
	UINT8 step_v;					///< v step size of histogram [0~31]
} CTL_IFE_ISP_HIST;

/************************
    IFE structure - VA paramater settingss
************************/
typedef enum {
	CTL_IFE_ISP_VA_FILTER_SYM_MIRROR = 0,
	CTL_IFE_ISP_VA_FILTER_SYM_INVERSE,
} CTL_IFE_ISP_VA_FILTER_SYM_SEL;

typedef enum {
	CTL_IFE_ISP_VA_FILTER_SIZE_1 = 0,
	CTL_IFE_ISP_VA_FILTER_SIZE_3,
	CTL_IFE_ISP_VA_FILTER_SIZE_5,
	CTL_IFE_ISP_VA_FILTER_SIZE_7,
} CTL_IFE_ISP_VA_FILTER_SIZE_SEL;

typedef struct {
	CTL_IFE_ISP_VA_FILTER_SYM_SEL symmetry;		///< select filter kernel symmetry
	CTL_IFE_ISP_VA_FILTER_SIZE_SEL filter_size;	///< select filter kernel size
	UINT8 tap_a;								///< filter coefficent A, range: 0~31
	INT8  tap_b;								///< filter coefficent B, range: -16~15
	INT8  tap_c;								///< filter coefficent C, range: -8~7
	INT8  tap_d;								///< filter coefficent D, range: -8~7
	UINT8 div;									///< filter normalized term, range: 0~15
	UINT16 th_l;								///< lower threshold, range: 0~4095
	UINT16 th_u;								///< upper threshold, range: 0~4095
} CTL_IFE_ISP_VA_FILTER;

typedef struct {
	CTL_IFE_ISP_VA_FILTER_SYM_SEL symmetry_iir2;///< select filter kernel symmetry
	CTL_IFE_ISP_VA_FILTER_SYM_SEL symmetry_iir3;///< select filter kernel symmetry
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
} CTL_IFE_ISP_VA_IIR_FILTER;

typedef struct {
	CTL_IFE_ISP_VA_FILTER h_filt;				///< horizontal filter
	CTL_IFE_ISP_VA_FILTER v_filt;				///< vertical filter
	CTL_IFE_ISP_VA_IIR_FILTER iir_filt;			///< vertical filter
	BOOL count_enable;							///< count the nonzero pixels
} CTL_IFE_ISP_VA_GROUP_INFO;

typedef struct {
	BOOL enable;								///< va independent window enable
} CTL_IFE_ISP_VA_INDEP_OPT;

typedef struct {
	UINT8 ldg_low_th;          ///< Darkness threshold
	UINT8 ldg_high_th;         ///< Brightness threshold
	UINT8 ldg_low_gain;        ///< Dark region minima gain
	UINT8 ldg_high_gain;       ///< Bright region minima gain
	UINT8 ldg_low_slope;       ///< Dark region gain slope
	UINT8 ldg_high_slope;      ///< Bright region gain slope
} CTL_IFE_ISP_VA_LDG_PARAM;

typedef enum {
	CTL_IFE_ISP_NO_PRE_FILTER     = 0,   ///<
	CTL_IFE_ISP_PRE_FILTER_3x3_1  = 1,   ///<
	CTL_IFE_ISP_PRE_FILTER_3x3_2  = 2,   ///<
} CTL_IFE_ISP_VA_PRE_FILTER_MODE;

typedef enum {
	CTL_IFE_ISP_VA_ENERGY_COUNT     = 0,   ///<
	CTL_IFE_ISP_VA_HIGH_LUMA_COUNT  = 1,   ///<
} CTL_IFE_ISP_VA_CNT_OUTSEL;

typedef enum {
	CTL_IFE_ISP_VA_VDETGH_FIR     = 0,   ///< Horizontal Filter Selection
	CTL_IFE_ISP_VA_VDETGH_IIR  	 = 1,
} CTL_IFE_ISP_VA_VDETGH_FILTER_SEL;

typedef enum {
	CTL_IFE_ISP_VA_VDETGH_IIR_INPUT_ORIGINAL     = 0,   ///< G1/2H IIR input select
	CTL_IFE_ISP_VA_VDETGH_IIR_INPUT_JUMP_PIXEL   = 1,
} CTL_IFE_ISP_VA_VDETGH_IIR_INPUT_SEL;

typedef struct {
	CTL_IFE_ISP_VA_VDETGH_FILTER_SEL vdetgh1_filter_sel;
	CTL_IFE_ISP_VA_VDETGH_IIR_INPUT_SEL vdetgh1_iir_input_sel;  //538 add
	CTL_IFE_ISP_VA_VDETGH_FILTER_SEL vdetgh2_filter_sel;
	CTL_IFE_ISP_VA_VDETGH_IIR_INPUT_SEL vdetgh2_iir_input_sel;  //538 add
} CTL_IFE_ISP_VA_FILTER_SEL;

typedef struct {
	BOOL enable;								///< va function enable
	BOOL indep_va_enable;						///< independent va function enable
	BOOL ldg_enable;							///< va level dependent gain enable
	BOOL gamma_enable;							///< va gamma enable

	//filter
	CTL_IFE_ISP_VA_GROUP_INFO group_1;			///< va group 1 info
	CTL_IFE_ISP_VA_GROUP_INFO group_2;			///< va group 2 info
	CTL_IFE_ISP_VA_FILTER_SEL fltr_sel;

	//VA
	BOOL va_out_grp1_2;							///< 0 --> output only group 1, 1 --> output group 1 and 2
	//UINT32 va_lofs;								///< va output lineoffset
	USIZE win_num;								///< va window number, 1x1 ~ 8x8

	//independent VA
	CTL_IFE_ISP_VA_INDEP_OPT indep_win[CTL_IFE_ISP_VA_INDEP_NUM]; ///< va independent settings

	//ldg
	CTL_IFE_ISP_VA_LDG_PARAM ldg_para;

	//pre
	CTL_IFE_ISP_VA_PRE_FILTER_MODE pre_filter_mode;

	//gamma
	UINT16 va_gamma_lut[CTL_IFE_ISP_VA_GAMMA_LEN];

	CTL_IFE_ISP_VA_CNT_OUTSEL win_cnt_out_sel;
	UINT8 high_luma_th;
	UINT8 energy_w;

} CTL_IFE_ISP_VA;

/************************
    IFE Subisp
************************/
typedef struct {
	BOOL subisp_cst_en;
	INT16 coef[CTL_IFE_ISP_SUBISP_COEF_LEN];

	BOOL subisp_gamma_en;
	UINT8 subisp_gamma_lut[CTL_IFE_ISP_SUBISP_GAMMA_LEN];

	BOOL subisp_cfa_en;
} CTL_IFE_ISP_SUBISP_IQ;

/************************
    IFE FPN
************************/
typedef struct {
	BOOL   fpn_en;
	UINT8  fpn_cgain_range;
	UINT16  fpn_cgain_r[CTL_IFE_ISP_FPN_PATH_MAX];
	UINT16  fpn_cgain_gr[CTL_IFE_ISP_FPN_PATH_MAX];
	UINT16  fpn_cgain_gb[CTL_IFE_ISP_FPN_PATH_MAX];
	UINT16  fpn_cgain_b[CTL_IFE_ISP_FPN_PATH_MAX];
	UINT16  fpn_cgain_ir[CTL_IFE_ISP_FPN_PATH_MAX];
	UINT16  fpn_cofs_r[CTL_IFE_ISP_FPN_PATH_MAX];
	UINT16  fpn_cofs_gr[CTL_IFE_ISP_FPN_PATH_MAX];
	UINT16  fpn_cofs_gb[CTL_IFE_ISP_FPN_PATH_MAX];
	UINT16  fpn_cofs_b[CTL_IFE_ISP_FPN_PATH_MAX];
	UINT16  fpn_cofs_ir[CTL_IFE_ISP_FPN_PATH_MAX];
} CTL_IFE_ISP_FPN;

/************************
    misc structure
************************/

/************************
    main structure
************************/
typedef struct {
	CTL_IFE_ISP_NRS			*p_nrs0;
	CTL_IFE_ISP_FCURVE		*p_fcurve;
	CTL_IFE_ISP_FUSION		*p_fusion;
	CTL_IFE_ISP_OUTL		*p_outl;
	CTL_IFE_ISP_FILTER		*p_filt;
	CTL_IFE_ISP_DGAIN		*p_dgain;
	CTL_IFE_ISP_CGAIN		*p_cgain;
	CTL_IFE_ISP_VIG			*p_vig;
	CTL_IFE_ISP_GBAL		*p_gbal;
	CTL_IFE_ISP_WDR			*p_wdr;
	CTL_IFE_ISP_WDR_SUBIMG	*p_wdr_subimg;
	CTL_IFE_ISP_HIST		*p_hist;
	CTL_IFE_ISP_VA			*p_va;
	CTL_IFE_ISP_SUBISP_IQ	*p_subisp_iq;
	CTL_IFE_ISP_FPN			*p_fpn;
} CTL_IFE_ISP_IQ_ALL;

#endif
/*******************************************************************************
*	                          IPP IQ parameter End                             *
*******************************************************************************/

#endif

