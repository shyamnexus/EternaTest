/**
    IPL Ctrl Layer, ISP Interface

    @file       ctl_ipp_isp.h
    @ingroup    mIIPL_Ctrl
    @note       None

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/


#ifndef _CTL_IME_ISP_H
#define _CTL_IME_ISP_H

/*******************************************************************************
*	Following #if 1 ... #endif block include all IPP IQ parameter.             *
*	These content must sync with kdrv IQ parameter. (see kdrv_ime.h)           *
*                                                                              *
*   How to modify the following IQ parameter:                                  *
*   1. main structure                                                          *
*      (i)   add item in KDRV_IME_IQ_CFG                                       *
*               KDRV_IME_IQ_XXX xxx;                                           *
*      (ii)  add item in CTL_IME_ISP_IQ_ALL                                    *
*               CTL_IME_ISP_XXX *p_xxx;                                        *
*      (iii) add memcpy in ctl_ipp_isp_set_ime_iq()                            *
*               if (p_iq_ime->p_xxx != NULL) {                                 *
*					memcpy((void *)&p_kdrv_iq_ime->xxx, (void *)p_iq_ime->p_xxx, sizeof(p_kdrv_iq_ime->xxx)); *
*					p_kdrv_iq_xxx->update |= KDRV_IME_UPDATE_XXX;              *
*				}                                                              *
*      (iv)  add static assert before (iii) to check structure size            *
*               STATIC_ASSERT(sizeof(CTL_IME_ISP_XXX) == sizeof(KDRV_IME_IQ_XXX)); *
*               if (p_iq_ime->p_xxx != NULL) { ...                             *
*                                                                              *
*   2. sub structure                                                           *
*      (i)   modify content in #if 1 ... #endif of kdrv_ime.h                  *
*      (ii)  modify content in #if 1 ... #endif of ctl_ime_isp.h               *
*      (iii) compare #if 1 ... #endif are the same                             *
*               copy #if 1 ... #endif of ctl_ime_isp.h to compare tool left    *
*               copy #if 1 ... #endif of kdrv_ime.h to compare tool right      *
*               replace 'KDRV_IME_IQ' to 'CTL_IME_ISP'                         *
*               do compare! (the sub structure part should be exactly the same)*
*******************************************************************************/
#if 1
#define CTL_IME_ISP_RF_CTR_TH_LEN						3
#define CTL_IME_ISP_RF_SS_RGN_TH_LEN					5
#define CTL_IME_ISP_RF_EDG_RGN_TH_LEN					5
#define CTL_IME_ISP_RF_MON_RGN_TH_LEN					5
#define CTL_IME_ISP_CORING_GAIN_LEN						3
#define CTL_IME_ISP_CUTOUT_LEN							3
#define CTL_IME_ISP_DBCS_WT_LUT_TAB						16
#define CTL_IME_ISP_TMNR_ME_SAD_PENALTY_TAB				8
#define CTL_IME_ISP_TMNR_ME_SWITCH_THRESHOLD_TAB		8
#define CTL_IME_ISP_TMNR_ME_DETAIL_PENALTY_TAB			8
#define CTL_IME_ISP_TMNR_ME_PROBABILITY_TAB				8
#define CTL_IME_ISP_TMNR_MD_SAD_COEFA_TAB				8
#define CTL_IME_ISP_TMNR_MD_SAD_COEFB_TAB				8
#define CTL_IME_ISP_TMNR_MD_SAD_STD_TAB					8
#define CTL_IME_ISP_TMNR_MD_FINAL_THRESHOLD_TAB			2
#define CTL_IME_ISP_TMNR_MD_ROI_FINAL_THRESHOLD_TAB		2
#define CTL_IME_ISP_TMNR_MC_SAD_BASE_TAB				8
#define CTL_IME_ISP_TMNR_MC_SAD_COEFA_TAB				8
#define CTL_IME_ISP_TMNR_MC_SAD_COEFB_TAB				8
#define CTL_IME_ISP_TMNR_MC_SAD_STD_TAB					8
#define CTL_IME_ISP_TMNR_MC_FINAL_THRESHOLD_TAB			2
#define CTL_IME_ISP_TMNR_MC_ROI_FINAL_THRESHOLD_TAB		2
#define CTL_IME_ISP_TMNR_PS_MIX_RATIO_TAB				2
#define CTL_IME_ISP_TMNR_NR_LUMA_RESIDUE_TH_TAB			3
#define CTL_IME_ISP_TMNR_NR_FREQ_WEIGHT_TAB				4
#define CTL_IME_ISP_TMNR_NR_LUMA_WEIGHT_TAB				8
#define CTL_IME_ISP_TMNR_NR_PRE_FILTER_STRENGTH_TAB		4
#define CTL_IME_ISP_TMNR_NR_PRE_FILTER_RATION_TAB		2
#define CTL_IME_ISP_TMNR_NR_SFILTER_STRENGTH_TAB		3
#define CTL_IME_ISP_TMNR_NR_TFILTER_STRENGTH_TAB		3
#define CTL_IME_ISP_TMNR_NR_LUMA_LUT_TAB				8
#define CTL_IME_ISP_TMNR_NR_LUMA_RATIO_TAB				2
#define CTL_IME_ISP_TMNR_NR_CHROMA_LUT_TAB				8
#define CTL_IME_ISP_TMNR_NR_CHROMA_RATIO_TAB			2
#define CTL_IME_ISP_TMNR_NR_TF0_FILTER_TAB				3
#define CTL_IME_ISP_TMNR_NR_C_TF0_RATIO_TAB				2
#define CTL_IME_ISP_TMNR_NR_CSHK_TH_TAB					8
#define CTL_IME_ISP_TMNR_NR_CSHK_VAL					8
#define CTL_IME_ISP_EWG_CURVE_LEN						9
#define CTL_IME_ISP_NOISE_CURVE_LEN						17

/************************
    KDRV IME structure - local chroma adaptation parameters
************************/
typedef enum {
	CTL_IME_ISP_EDG_KER_SIZE_3X3 = 0,
	CTL_IME_ISP_EDG_KER_SIZE_5X5 = 1
} CTL_IME_ISP_LCA_DRF_EDG_KER_SIZE;

typedef enum {
	CTL_IME_ISP_KER_SIZE_3X3 = 0,
	CTL_IME_ISP_KER_SIZE_5X5 = 1,
	CTL_IME_ISP_KER_SIZE_7X7 = 2,
	CTL_IME_ISP_KER_SIZE_9X9 = 3,
	CTL_IME_ISP_KER_SIZE_11X9 = 4,
	CTL_IME_ISP_KER_SIZE_13X9 = 5,
	CTL_IME_ISP_KER_SIZE_15X9 = 6,
} CTL_IME_ISP_LCA_DRF_KER_SIZE;

typedef struct {
	CTL_IME_ISP_LCA_DRF_EDG_KER_SIZE edge_ker_size_sel;
	UINT32 edge_th[2];
	CTL_IME_ISP_LCA_DRF_KER_SIZE sr_ker_size_sel;
	CTL_IME_ISP_LCA_DRF_KER_SIZE mr_ker_size_sel;
	UINT8 ctr_3x3_y_th[CTL_IME_ISP_RF_CTR_TH_LEN];
	UINT8 ctr_3x3_u_th[CTL_IME_ISP_RF_CTR_TH_LEN];
	UINT8 ctr_3x3_v_th[CTL_IME_ISP_RF_CTR_TH_LEN];
	UINT8 sr_y_wet; 											///<Center Reference weight of Y channel for smooth region
	UINT8 sr_uv_wet;
	UINT8 er_y_wet; 											///<Center Reference weight of Y channel for edge region
	UINT8 er_uv_wet;
	UINT8 ss_region_y_th[CTL_IME_ISP_RF_SS_RGN_TH_LEN]; 		///<Range filter threshold of Y channel for still-smoth region
	UINT16 ss_region_uv_th[CTL_IME_ISP_RF_SS_RGN_TH_LEN];
	UINT8 edge_region_y_th[CTL_IME_ISP_RF_EDG_RGN_TH_LEN];		///<Range filter threshold of Y channel for still-edge region
	UINT16 edge_region_uv_th[CTL_IME_ISP_RF_EDG_RGN_TH_LEN];
	UINT8 motion_region_y_th[CTL_IME_ISP_RF_MON_RGN_TH_LEN]; 	///<Range filter threshold of Y channel for motion region
	UINT16 motion_region_uv_th[CTL_IME_ISP_RF_MON_RGN_TH_LEN];

	//UINT8 uv_ref_y_wt;
	//UINT8 y_out_wt;
	//UINT8 uv_out_wt;
} CTL_IME_ISP_LCA_RF_PARAM;

typedef struct {
	UINT8 still_y_gain[CTL_IME_ISP_CORING_GAIN_LEN];
	UINT8 motion_y_gain[CTL_IME_ISP_CORING_GAIN_LEN];
	UINT8 still_u_gain[CTL_IME_ISP_CORING_GAIN_LEN];
	UINT8 motion_u_gain[CTL_IME_ISP_CORING_GAIN_LEN];
	UINT8 still_v_gain[CTL_IME_ISP_CORING_GAIN_LEN];
	UINT8 motion_v_gain[CTL_IME_ISP_CORING_GAIN_LEN];
} CTL_IME_ISP_LCA_CORING_GAIN_PARAM;

typedef struct {
	UINT8 still_y_coff[CTL_IME_ISP_CUTOUT_LEN];
	UINT8 motion_y_coff[CTL_IME_ISP_CUTOUT_LEN];
	UINT8 still_u_coff[CTL_IME_ISP_CUTOUT_LEN];
	UINT8 motion_u_coff[CTL_IME_ISP_CUTOUT_LEN];
	UINT8 still_v_coff[CTL_IME_ISP_CUTOUT_LEN];
	UINT8 motion_v_coff[CTL_IME_ISP_CUTOUT_LEN];
} CTL_IME_ISP_LCA_CUTOUT_PARAM;

typedef enum {
	CTL_IME_ISP_DBG_COLOR_MODE 		= 0,
	CTL_IME_ISP_EDGE_INFORMATION	= 8,
	CTL_IME_ISP_DBG_UNKNOWN,
} CTL_IME_ISP_LCA_DBG_SEL;

typedef struct {
	BOOL  enable;
	CTL_IME_ISP_LCA_DBG_SEL ch_sel;
	UINT8 ch_ofs;
	UINT16 x_pos;
} CTL_IME_ISP_LCA_DBG_PARAM;

typedef enum {
	CTL_IME_ISP_LCA_PROC_LOCATION_PRE_3DNR  = 0,
	CTL_IME_ISP_LCA_PROC_LOCATION_POST_3DNR = 1,
	ENUM_DUMMY4WORD(CTL_IME_ISP_LCA_PROC_LOCATION)
} CTL_IME_ISP_LCA_PROC_LOCATION;

typedef struct {
	CTL_IME_ISP_LCA_RF_PARAM rf;					///< Range Filter
	CTL_IME_ISP_LCA_CORING_GAIN_PARAM coring_gain;	///< coring gain
	CTL_IME_ISP_LCA_CUTOUT_PARAM cutout_coff;		///< cutout_coff
	UINT8 final_y_out_wt;
	UINT8 final_uv_out_wt;
	CTL_IME_ISP_LCA_DBG_PARAM dbg;					///< lca debug methods

	CTL_IME_ISP_LCA_PROC_LOCATION set_proc_location; 	///< processing location
} CTL_IME_ISP_LCA;

/************************
    KDRV IME structure - dark and bright region chroma suppression
************************/
typedef enum {
	CTL_IME_ISP_DBCS_DK_MODE   = 0,   ///< dark mode
	CTL_IME_ISP_DBCS_BT_MODE   = 1,   ///< bright mode
	CTL_IME_ISP_DBCS_BOTH_MODE  = 2,  ///< dark and bright mode
	ENUM_DUMMY4WORD(CTL_IME_ISP_DBCS_MODE_SEL)
} CTL_IME_ISP_DBCS_MODE_SEL;

typedef struct {
	UINT8 enable;								///< dark and bright region chroma suppression function enable
	CTL_IME_ISP_DBCS_MODE_SEL op_mode;			///< Process mode
	UINT8 cent_u;								///< Center value for U channel
	UINT8 cent_v;								///< Center value for V channel
	UINT16 step_y;								///< Step for luma
	UINT16 step_c;								///< Step for chroma
	UINT8 wt_y[CTL_IME_ISP_DBCS_WT_LUT_TAB];	///< Weighting LUT for luma channel, 16 elements, range: [0, 16]
	UINT8 wt_c[CTL_IME_ISP_DBCS_WT_LUT_TAB];	///< Weighting LUT for luma channel, 16 elements, range: [0, 16]
} CTL_IME_ISP_DBCS;

/************************
    KDRV IME structure - TMNR info.
************************/
/**
    IME enum - An option of update mode in motion estimation
*/
typedef enum {
	CTL_IME_ISP_ME_UPDATE_RAND  = 0,
	CTL_IME_ISP_ME_UPDATE_FIXED = 1,
	ENUM_DUMMY4WORD(CTL_IME_ISP_TMNR_ME_UPDATE_MODE)
} CTL_IME_ISP_TMNR_ME_UPDATE_MODE;

/**
    IME structure - 3DNR motion estimation parameters
*/
typedef struct {
	CTL_IME_ISP_TMNR_ME_UPDATE_MODE update_mode;	///< An option of update mode in motion estimation. Range 0~1
	UINT8 boundary_set;								///< Set image boundary as still type or not.  Range 0~1
	UINT8 sad_shift;								///< Right shift number for SAD statistic data, range = [0, 15]
	UINT8 cost_blend;								///< Cost blending ratio, 0 for automatic cost, 0xF for user define

	UINT8 rand_bit_x;	///<range 1~7
	UINT8 rand_bit_y;	///<range 1~7
	UINT16 min_detail;	///< range 0~16383

	UINT16 sad_penalty[CTL_IME_ISP_TMNR_ME_SAD_PENALTY_TAB];		///< range 0~1023
	UINT8 switch_th[CTL_IME_ISP_TMNR_ME_SWITCH_THRESHOLD_TAB];		///< range 0~255
	UINT8 switch_rto;												///< range 0~255
	UINT8 detail_penalty[CTL_IME_ISP_TMNR_ME_DETAIL_PENALTY_TAB];	///< range 0~15
	UINT8 probability[CTL_IME_ISP_TMNR_ME_PROBABILITY_TAB];			///< range 0~1
} CTL_IME_ISP_TMNR_ME;

/**
    IME structure - 3DNR motion detection parameters
*/
typedef struct {
	UINT8 sad_coefa[CTL_IME_ISP_TMNR_MD_SAD_COEFA_TAB];    ///< Edge coefficient of motion detection. Range 0~63
	UINT16 sad_coefb[CTL_IME_ISP_TMNR_MD_SAD_COEFB_TAB];   ///< Offset of motion detection. Range 0~16383
	UINT16 sad_std[CTL_IME_ISP_TMNR_MD_SAD_STD_TAB];       ///< Standard deviation of motion detection. Range 0~16383
	UINT8 fth[CTL_IME_ISP_TMNR_MD_FINAL_THRESHOLD_TAB];    ///< final threshold of motion detection. Range 0~63
} CTL_IME_ISP_TMNR_MD;

/**
    IME structure - 3DNR motion detection parameters for ROI
*/
typedef struct {
	UINT8 fth[CTL_IME_ISP_TMNR_MD_ROI_FINAL_THRESHOLD_TAB];   ///< final threshold of motion detection.Range 0~63
} CTL_IME_ISP_TMNR_MD_ROI;

/**
    IME structure - 3DNR motion compensation parameters
*/
typedef struct {
	UINT16 sad_base[CTL_IME_ISP_TMNR_MC_SAD_BASE_TAB];     ///< base level of noise. Range 0~16383
	UINT8 sad_coefa[CTL_IME_ISP_TMNR_MC_SAD_COEFA_TAB];    ///< edge coefficient of motion compensation.Range 0~63
	UINT16 sad_coefb[CTL_IME_ISP_TMNR_MC_SAD_COEFB_TAB];   ///< offset of motion compensation.Range 0~16383
	UINT16 sad_std[CTL_IME_ISP_TMNR_MC_SAD_STD_TAB];       ///< standard deviation of motion compensation.Range 0~16383
	UINT8 fth[CTL_IME_ISP_TMNR_MC_FINAL_THRESHOLD_TAB];    ///< final threshold of motion compensation. Range 0~63
} CTL_IME_ISP_TMNR_MC;

/**
    IME structure - 3DNR motion compensation parameters for ROI
*/
typedef struct {
	UINT8 fth[CTL_IME_ISP_TMNR_MC_ROI_FINAL_THRESHOLD_TAB];   ///< final threshold of motion compensation.Range 0~63
} CTL_IME_ISP_TMNR_MC_ROI;

/**
    IME structure - 3DNR patch selection parameters
*/
typedef enum {
	CTL_IME_ISP_MV_INFO_MODE_AVERAGE  = 0,
	CTL_IME_ISP_MV_INFO_MODE_LPF      = 1,
	CTL_IME_ISP_MV_INFO_MODE_MINIMUM  = 2,
	CTL_IME_ISP_MV_INFO_MODE_MAXIMUM  = 3,
	ENUM_DUMMY4WORD(CTL_IME_ISP_TMNR_MV_INFO_MODE)
} CTL_IME_ISP_TMNR_MV_INFO_MODE;

typedef struct {
	UINT8 smart_roi_ctrl_en;					///< ROI control. Range 0~1
	UINT8 mv_check_en;							///< MV checkiong process in PS module. Range 0~1
	UINT8 roi_mv_check_en;						///< MV checking process for ROI. Range 0~1

	CTL_IME_ISP_TMNR_MV_INFO_MODE mv_info_mode;			///< Mode option for MV length calculation. Range 0~3
	UINT8 mv_th;										///< MV threshold. Range 0~63
	UINT8 roi_mv_th;									///< MV threshold for ROI. Range 0~63
	UINT8 mix_ratio[CTL_IME_ISP_TMNR_PS_MIX_RATIO_TAB];	///< Mix ratio in patch selection. Range 0~255
	UINT8 ds_th;										///< Threshold of motion status down-sampling. Range 0~31
	UINT8 ds_th_roi;									///< Threshold for motion status down-sampling for ROI.Range 0~31
	UINT16 fs_th;										////< Threshold of patch error. Range 0~16383.
	UINT16 blur_eth;									////< Threshold of TF0 Blur for smooth region
} CTL_IME_ISP_TMNR_PS;

/**
    IME enum - Strength option for Y-channel pre-filter
*/
typedef enum {
	CTL_IME_ISP_PRE_FILTER_Y_STR_DISABLE = 0,	///< off
	CTL_IME_ISP_PRE_FILTER_Y_STR_1 = 1,			///< type 1 filter
	CTL_IME_ISP_PRE_FILTER_Y_STR_2 = 2,			///< type 2 filter
	ENUM_DUMMY4WORD(CTL_IME_ISP_TMNR_PRE_FILTER_Y_STR)
} CTL_IME_ISP_TMNR_PRE_FILTER_Y_STR;

typedef enum {
	CTL_IME_ISP_PRE_FILTER_DISABLE = 0,    ///< disable
	CTL_IME_ISP_PRE_FILTER_TYPE_1 = 1,     ///< 3x3
	CTL_IME_ISP_PRE_FILTER_TYPE_2 = 2,     ///< 3x3
	CTL_IME_ISP_PRE_FILTER_TYPE_3 = 3,     ///< 5x5
	ENUM_DUMMY4WORD(CTL_IME_ISP_TMNR_PRE_FILTER_TYPE)
} CTL_IME_ISP_TMNR_PRE_FILTER_TYPE;

/**
    IME structure - 3DNR noise filter parameters
*/
typedef struct {
	UINT8 luma_ch_en;	///< noise filter enable for luma channel. Range 0~1
	UINT8 chroma_ch_en;	///< noise filter enable for chroma channel.Range 0~1

	UINT8 center_wzeros_y;	///< Set weighting as 0 to the center pixel in internal NR-filter. Range 0~1

	UINT8 luma_residue_th[CTL_IME_ISP_TMNR_NR_LUMA_RESIDUE_TH_TAB];	///< Protection threshold of luma channel. Range 0~255
	UINT8 chroma_residue_th;	///< Protection threshold of luma channel. Range 0~255

	UINT8 freq_wet[CTL_IME_ISP_TMNR_NR_FREQ_WEIGHT_TAB];	///< Filter weighting for low frequency. Range 0~255
	UINT8 luma_wet[CTL_IME_ISP_TMNR_NR_LUMA_WEIGHT_TAB];	///< Filter intensity weighting. Range 0~255

	CTL_IME_ISP_TMNR_PRE_FILTER_Y_STR pre_y_blur_str;	///< Strength option for luma channel pre-filter.Range 0~2
	UINT8 pf_str;										///<  Strength of pre-filters
	UINT8 pre_filter_str[CTL_IME_ISP_TMNR_NR_PRE_FILTER_STRENGTH_TAB];	///< Strength of pre-filtering for low frequency. Range 0~255
	UINT8 pre_filter_rto[CTL_IME_ISP_TMNR_NR_PRE_FILTER_RATION_TAB];	///< adjustment ratio of pre-filtering for transitional object.Range 0~255

	UINT8 snr_str[CTL_IME_ISP_TMNR_NR_SFILTER_STRENGTH_TAB];	///< Strength of spatial filter for still object.Range 0~255
	UINT8 tnr_str[CTL_IME_ISP_TMNR_NR_TFILTER_STRENGTH_TAB];	///< Strength of temporal filter for still object.Range 0~255

	UINT32 snr_base_th;					///< Base threshold of spatial noise reduction.Range 0~65535
	UINT32 tnr_base_th;					///< Base threshold of spatial noise reduction.Range 0~65535

	UINT8 luma_3d_lut[CTL_IME_ISP_TMNR_NR_LUMA_LUT_TAB];			///< Noise reduction LUT for luma channel.Range 0~127
	UINT8 luma_3d_rto[CTL_IME_ISP_TMNR_NR_LUMA_RATIO_TAB];			///< Adjustment ratio 0 of noise reduction LUT for luma channel.Range 0~255
	UINT8 luma_comp_str;											///< luma compensation strength
	UINT8 chroma_3d_lut[CTL_IME_ISP_TMNR_NR_CHROMA_LUT_TAB];		///< Noise reduction LUT for chroma channel.Range 0~127
	UINT8 chroma_3d_rto[CTL_IME_ISP_TMNR_NR_CHROMA_RATIO_TAB];		///< Adjustment ratio 0 of noise reduction LUT for chroma channel.Range 0~255

	UINT8 luma_nr_type;	///< 3DNR filter option, Range 0~1

	UINT8 tf0_blur_str[CTL_IME_ISP_TMNR_NR_TF0_FILTER_TAB];
	UINT8 tf0_blur_estr;
	UINT8 tf0_y_str[CTL_IME_ISP_TMNR_NR_TF0_FILTER_TAB];
	UINT8 tf0_c_str[CTL_IME_ISP_TMNR_NR_TF0_FILTER_TAB];
	//538 add
	UINT8 u_tf0_md_th;											///< Threshold of motion detection for chroma U channel.Range 0~255
	UINT8 v_tf0_md_th;
	UINT8 c_tf0_ratio[CTL_IME_ISP_TMNR_NR_C_TF0_RATIO_TAB];	///< Strength of temporal filter in TF0 for still/moving object.Range 0~255
	UINT8 motion_sat_ratio;									///< Saturation adjustment ratio for moving object.Range 0~255
	UINT8 c_tf0_tprot_th; 										///< Residual threshold of temporal filter in TF0.Range 0~15
	UINT16 cshk_th[CTL_IME_ISP_TMNR_NR_CSHK_TH_TAB];			///< Color diterhing thresthod.Range 0~1023
	UINT8 cshk_val[CTL_IME_ISP_TMNR_NR_CSHK_VAL];				///< Color diterhing bit number.Range 0~7
} CTL_IME_ISP_TMNR_NR;

/**
    IME structure - 3DNR debug parameters
*/
typedef struct {
	UINT8 dbg_mv0;	///< debug for mv = 0
	UINT8 dbg_mode;	///< debug mode selection. Range 0~6
} CTL_IME_ISP_TMNR_DBG;

typedef struct {
	UINT8 en;
	UINT8 start_point;
	UINT8 step_size;
} CTL_IME_ISP_TMNR_FCVG;

typedef struct {
	UINT8 enable;
	CTL_IME_ISP_TMNR_ME me_param;
	CTL_IME_ISP_TMNR_MD md_param;
	CTL_IME_ISP_TMNR_MD_ROI md_roi_param;
	CTL_IME_ISP_TMNR_MC mc_param;
	CTL_IME_ISP_TMNR_MC_ROI mc_roi_param;
	CTL_IME_ISP_TMNR_PS ps_param;
	CTL_IME_ISP_TMNR_NR nr_param;
	CTL_IME_ISP_TMNR_DBG dbg_param;
	CTL_IME_ISP_TMNR_FCVG fcvg_param;
} CTL_IME_ISP_TMNR;

/************************
    KDRV IME structure - YCbCr converter parameters
************************/
/**
    KDRV IME enum - YCbCr converter selection
*/
typedef enum {
	CTL_IME_ISP_YCC_CVT_BT601 = 0,	///< BT.601
	CTL_IME_ISP_YCC_CVT_BT709 = 1,	///< BT.709
	ENUM_DUMMY4WORD(CTL_IME_ISP_YCC_CVT_SEL)
} CTL_IME_ISP_YCC_CVT_SEL;

typedef struct {
	UINT8 enable;						///< YCbCrconverter function enable/disable
	CTL_IME_ISP_YCC_CVT_SEL cvt_sel;	///< YCbCr converter selection, BT.601 or BT.709
} CTL_IME_ISP_YCC_CVT;

/************************
    KDRV IME structure - post sharpen
************************/
typedef enum {
	CTL_IME_ISP_SHP_EDG_WT_SRC_Y			= 0,	///<
	CTL_IME_ISP_SHP_EDG_WT_SRC_INV_GAMMA	= 1,	///<
} CTL_IME_ISP_SHP_EDG_WT_SRC_SEL;

typedef enum {
	CTL_IME_ISP_SHP_JND_FILT_3x3 = 0,
	CTL_IME_ISP_SHP_JND_FILT_5x5 = 1,
} CTL_IME_ISP_SHP_JND_FILTER_SEL;

typedef struct {
	BOOL   shp_en;
	BOOL   motion_bit_en;
	BOOL   dbg_en;
	CTL_IME_ISP_SHP_EDG_WT_SRC_SEL shp_src_sel;
	CTL_IME_ISP_SHP_JND_FILTER_SEL jnd_filt_sel;
	UINT8  weight_th;
	UINT8  weight_gain;
	UINT8  noise_lv;
	UINT16 flat_th;
	UINT16 edge_th;
	UINT8  wet;
	UINT8  noise_curve[CTL_IME_ISP_NOISE_CURVE_LEN];
	UINT16 ewg_curve[CTL_IME_ISP_EWG_CURVE_LEN];
	UINT16 slope;
	UINT8  flat_region_str;
	UINT8  edge_region_str;
	UINT8  motion_str;
	UINT8  static_str;
	UINT8  trans_str;
	UINT8  blend_inv_gamma;
	UINT8  coring_th;
	UINT8  edge_str;
	UINT8  bright_halo_clip;
	UINT8  dark_halo_clip;
} CTL_IME_ISP_SHARPEN;

/************************
    misc structure
************************/

/************************
    main structure
************************/
typedef struct {
	CTL_IME_ISP_LCA					*p_lca;
	CTL_IME_ISP_DBCS				*p_dbcs;
	CTL_IME_ISP_TMNR				*p_tmnr;
	CTL_IME_ISP_YCC_CVT				*p_ycccvt;
	CTL_IME_ISP_SHARPEN				*p_sharpen;
} CTL_IME_ISP_IQ_ALL;

#endif
/*******************************************************************************
*	                          IPP IQ parameter End                             *
*******************************************************************************/

#endif
