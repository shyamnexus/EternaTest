/**
    Public header file for dal_ime

    This file is the header file that define the API and data type for dal_ime.

    @file       kdrv_ime.h
    @ingroup    mILibIPLCom
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _KDRV_IME_H_
#define _KDRV_IME_H_

#include "kdrv_videoprocess/kdrv_ipp_utility.h"
#include "kdrv_builtin/kdrv_ipp_builtin.h"

#define IME_SSDRV_SUPPORT (1)
#define IME_538_KFLOW_SUPPORT
#define IME_538_KDRV_FEATURE (1)
#define IME_538_KDRV_TMP_BLOCK (1)

typedef enum {
	KDRV_IME_PARAM_CFG_PROCESS_CPU = 0, /* write config to register.            SET-Only,   data_type: KDRV_IME_JOB_CFG */
	KDRV_IME_PARAM_CFG_PROCESS_LL,      /* write config to ll.                  SET-Only,   data_type: KDRV_IME_JOB_CFG */
	KDRV_IME_PARAM_TRIG_SINGLE,         /* trig ime start.                      SET-Only,   data_type: NULL */
	KDRV_IME_PARAM_TRIG_LL,             /* trig ime start by linklist.          SET-Only,   data_type: UINT32 */
	KDRV_IME_PARAM_TRIG_DIR,            /* trig ime start by direct.          SET-Only,   data_type: UINT32 */
	KDRV_IME_PARAM_SET_CB,              /* set isr callback.                    SET-Only,   data_type: KDRV_IPP_ISR_CB */
	KDRV_IME_PARAM_STOP_SINGLE,         /* stop ime.                            SET-Only,   data_type: NULL */
	KDRV_IME_PARAM_NN_ISP_SLICE_DONE ,  /* set slice done                       SET-Only,   data_type: UINT32 */
	KDRV_IME_PARAM_GET_SINGLE_OUT,      /* get single out bit.                  GET-Only,   data_type: NULL */
	KDRV_IME_PARAM_HARD_RESET,          /* hard reset.                          SET-Only,   data_type: NULL */
	KDRV_IME_PARAM_GRAY_AVG,            /* get lca gray avg result from engine. GET-Only,   data_type: KDRV_IME_LCA_GRAY_AVG */
	KDRV_IME_PARAM_GET_DRAM_END_STS,    /* get dram end status.                 GET-Only,   data_type: NULL */
	KDRV_IME_PARAM_CLR_DRAM_END_STS,    /* clr dram end status.                 GET-Only,   data_type: NULL */
	KDRV_IME_PARAM_DMA_CH_ABORT,        /* dma abort config.                    SET-Only,   data_type: UINT32 */
	KDRV_IME_PARAM_DMA_CH_STS,          /* dma idle status.                     GET-Only,   data_type: UINT32 */
	KDRV_IME_PARAM_GET_REG_BASE_ADDR,   /* get reg base addr                    GET-Only,   data_type: UINT32 */
	KDRV_IME_PARAM_GET_REG_BASE_ADDR_LL,/* get reg base addr ll mode            GET-Only,   data_type: UINT32 */
#if(IME_538_KDRV_FEATURE == 0)
	KDRV_IME_PARAM_GET_VA_RESULT,       /* get Va information                   GET-Only,   data_type: KDRV_IME_VA_RESULT */
	KDRV_IME_PARAM_VA_RESULT_DUAL,       /* get Va dual information             GET-Only,   data_type: KDRV_IME_VA_RST_DUAL */
	KDRV_IME_PARAM_GET_INDE_VA_RESULT,  /* get Va information                   GET-Only,   data_type: KDRV_IME_INDE_VA_RESULT */
	KDRV_IME_PARAM_GET_MERGED_INDP_VA,  /* get dbg info                         GET-Only,   data_type: KDRV_IPP_CB_STCS_OUTPUT */
#endif
	KDRV_IME_PARAM_GET_DBG_INFO,		/* get dbg info                         GET-Only,   data_type: KDRV_IME_DBG_INFO */
	KDRV_IME_PARAM_GET_IN_SPEC,			/* get ime eng input spec               GET-Only,   data_type: KDRV_IME_IN_LMT */
	KDRV_IME_PARAM_GET_OUT_SPEC,		/* get ime eng output spec              GET-Only,   data_type: KDRV_IME_LMT */
	KDRV_IME_PARAM_GET_3DNR_INFO,		/* get 3dnr info                        GET-Only,   data_type: KDRV_IME_3DNR_INFO */
	KDRV_IME_PARAM_ID_MAX,
} KDRV_IME_PARAM_ID;

typedef enum {
	KDRV_IME_INTERRUPT_LL_END                  = 0x00000001,
	KDRV_IME_INTERRUPT_LL_ERR                  = 0x00000002,
	KDRV_IME_INTERRUPT_LL_LATE                 = 0x00000004,
	KDRV_IME_INTERRUPT_LL_JEND                 = 0x00000008,
	KDRV_IME_INTERRUPT_BP1                     = 0x00000010,
	KDRV_IME_INTERRUPT_BP2                     = 0x00000020,
	KDRV_IME_INTERRUPT_BP3                     = 0x00000040,
	KDRV_IME_INTERRUPT_3DNR_SLICE_END          = 0x00000080,
	KDRV_IME_INTERRUPT_3DNR_MOT_END            = 0x00000100,
	KDRV_IME_INTERRUPT_3DNR_MV_END             = 0x00000200,
	KDRV_IME_INTERRUPT_3DNR_STA_END            = 0x00000400,
	//KDRV_IME_INTERRUPT_VA_OUT_END              = 0x00000800,
	KDRV_IME_INTERRUPT_3DNR_ENC_OVR            = 0x00001000,
	KDRV_IME_INTERRUPT_3DNR_DEC_ERR            = 0x00002000,
	KDRV_IME_INTERRUPT_FRM_ERR                 = 0x00004000,
	KDRV_IME_INTERRUPT_SLINE                   = 0x00008000,
	KDRV_IME_INTERRUPT_P1_OUT_ENC_OVFL         = 0x00010000,
	KDRV_IME_INTERRUPT_P2_OUT_ENC_OVFL         = 0x00020000,
	KDRV_IME_INTERRUPT_P3_OUT_ENC_OVFL         = 0x00040000,
	KDRV_IME_INTERRUPT_P4_OUT_ENC_OVFL         = 0x00080000,
	KDRV_IME_INTERRUPT_IN_DEC_ERR              = 0x00100000,
	KDRV_IME_INTERRUPT_NN_ISP2CPU_P2_OUT_READY = 0x00200000,
	KDRV_IME_INTERRUPT_NN_ISP2CPU_P2_IN_CLEAR  = 0x00400000,
	KDRV_IME_INTERRUPT_NN_ISP2CPU_P3_OUT_READY = 0x00800000,
	KDRV_IME_INTERRUPT_NN_ISP2CPU_P3_IN_CLEAR  = 0x01000000,
	KDRV_IME_INTERRUPT_FRM_START               = 0x20000000,  ///< frame-start
	KDRV_IME_INTERRUPT_STRP_END                = 0x40000000,  ///< stripe-end
	KDRV_IME_INTERRUPT_FRM_END                 = 0x80000000,  ///< frame-end
	KDRV_IME_INTERRUPT_ALL                     = (KDRV_IME_INTERRUPT_LL_END | KDRV_IME_INTERRUPT_LL_ERR | KDRV_IME_INTERRUPT_LL_LATE | KDRV_IME_INTERRUPT_LL_JEND | KDRV_IME_INTERRUPT_BP1 |
			KDRV_IME_INTERRUPT_BP2 | KDRV_IME_INTERRUPT_BP3 | KDRV_IME_INTERRUPT_3DNR_SLICE_END | KDRV_IME_INTERRUPT_3DNR_MOT_END | KDRV_IME_INTERRUPT_3DNR_MV_END |
			KDRV_IME_INTERRUPT_3DNR_STA_END | KDRV_IME_INTERRUPT_3DNR_ENC_OVR | KDRV_IME_INTERRUPT_3DNR_DEC_ERR | KDRV_IME_INTERRUPT_FRM_ERR |
			KDRV_IME_INTERRUPT_SLINE | KDRV_IME_INTERRUPT_P1_OUT_ENC_OVFL | KDRV_IME_INTERRUPT_P2_OUT_ENC_OVFL | KDRV_IME_INTERRUPT_P3_OUT_ENC_OVFL | KDRV_IME_INTERRUPT_P4_OUT_ENC_OVFL |
			KDRV_IME_INTERRUPT_FRM_START | KDRV_IME_INTERRUPT_STRP_END | KDRV_IME_INTERRUPT_FRM_END | KDRV_IME_INTERRUPT_IN_DEC_ERR | KDRV_IME_INTERRUPT_NN_ISP2CPU_P2_OUT_READY |
			KDRV_IME_INTERRUPT_NN_ISP2CPU_P2_IN_CLEAR | KDRV_IME_INTERRUPT_NN_ISP2CPU_P3_OUT_READY | KDRV_IME_INTERRUPT_NN_ISP2CPU_P3_IN_CLEAR)
} KDRV_IME_INTERRUPT;

typedef enum {
	KDRV_IME_UPDATE_LCA             = 0x00000001,
	KDRV_IME_UPDATE_3DNR            = 0x00000002,
	KDRV_IME_UPDATE_DBCS            = 0x00000004,
	KDRV_IME_UPDATE_YUVCVT          = 0x00000008,
	KDRV_IME_UPDATE_COMPRESS        = 0x00000010,   /* special case, all parameter is define in kdrv_ime, only set this bit to set once */
	KDRV_IME_UPDATE_LCA_GRAY_STA    = 0x00000020,
	KDRV_IME_UPDATE_LCA_REFCENT     = 0x00000040,
	KDRV_IME_UPDATE_LCA_FILTER      = 0x00000080,
	KDRV_IME_UPDATE_SHARPEN         = 0x00000100,
	KDRV_IME_UPDATE_ALL             = (KDRV_IME_UPDATE_LCA | KDRV_IME_UPDATE_3DNR | KDRV_IME_UPDATE_DBCS | KDRV_IME_UPDATE_YUVCVT | KDRV_IME_UPDATE_COMPRESS |
									   KDRV_IME_UPDATE_LCA_GRAY_STA | KDRV_IME_UPDATE_LCA_REFCENT | KDRV_IME_UPDATE_LCA_FILTER | KDRV_IME_UPDATE_SHARPEN)

} KDRV_IME_UPDATE;


/*******************************************************************************
*	Following #if 1 ... #endif block include all IPP IQ parameter.             *
*	These content must sync with kflow IQ parameter. (see ctl_ime_isp.h)       *
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
#define KDRV_IME_IQ_RF_CTR_TH_LEN						3
#define KDRV_IME_IQ_RF_SS_RGN_TH_LEN					5
#define KDRV_IME_IQ_RF_EDG_RGN_TH_LEN					5
#define KDRV_IME_IQ_RF_MON_RGN_TH_LEN					5
#define KDRV_IME_IQ_CORING_GAIN_LEN						3
#define KDRV_IME_IQ_CUTOUT_LEN							3
#define KDRV_IME_IQ_DBCS_WT_LUT_TAB						16
#define KDRV_IME_IQ_TMNR_ME_SAD_PENALTY_TAB				8
#define KDRV_IME_IQ_TMNR_ME_SWITCH_THRESHOLD_TAB		8
#define KDRV_IME_IQ_TMNR_ME_DETAIL_PENALTY_TAB			8
#define KDRV_IME_IQ_TMNR_ME_PROBABILITY_TAB				8
#define KDRV_IME_IQ_TMNR_MD_SAD_COEFA_TAB				8
#define KDRV_IME_IQ_TMNR_MD_SAD_COEFB_TAB				8
#define KDRV_IME_IQ_TMNR_MD_SAD_STD_TAB					8
#define KDRV_IME_IQ_TMNR_MD_FINAL_THRESHOLD_TAB			2
#define KDRV_IME_IQ_TMNR_MD_ROI_FINAL_THRESHOLD_TAB		2
#define KDRV_IME_IQ_TMNR_MC_SAD_BASE_TAB				8
#define KDRV_IME_IQ_TMNR_MC_SAD_COEFA_TAB				8
#define KDRV_IME_IQ_TMNR_MC_SAD_COEFB_TAB				8
#define KDRV_IME_IQ_TMNR_MC_SAD_STD_TAB					8
#define KDRV_IME_IQ_TMNR_MC_FINAL_THRESHOLD_TAB			2
#define KDRV_IME_IQ_TMNR_MC_ROI_FINAL_THRESHOLD_TAB		2
#define KDRV_IME_IQ_TMNR_PS_MIX_RATIO_TAB				2
#define KDRV_IME_IQ_TMNR_NR_LUMA_RESIDUE_TH_TAB			3
#define KDRV_IME_IQ_TMNR_NR_FREQ_WEIGHT_TAB				4
#define KDRV_IME_IQ_TMNR_NR_LUMA_WEIGHT_TAB				8
#define KDRV_IME_IQ_TMNR_NR_PRE_FILTER_STRENGTH_TAB		4
#define KDRV_IME_IQ_TMNR_NR_PRE_FILTER_RATION_TAB		2
#define KDRV_IME_IQ_TMNR_NR_SFILTER_STRENGTH_TAB		3
#define KDRV_IME_IQ_TMNR_NR_TFILTER_STRENGTH_TAB		3
#define KDRV_IME_IQ_TMNR_NR_LUMA_LUT_TAB				8
#define KDRV_IME_IQ_TMNR_NR_LUMA_RATIO_TAB				2
#define KDRV_IME_IQ_TMNR_NR_CHROMA_LUT_TAB				8
#define KDRV_IME_IQ_TMNR_NR_CHROMA_RATIO_TAB			2
#define KDRV_IME_IQ_TMNR_NR_TF0_FILTER_TAB				3
#define KDRV_IME_IQ_TMNR_NR_C_TF0_RATIO_TAB				2
#define KDRV_IME_IQ_TMNR_NR_CSHK_TH_TAB					8
#define KDRV_IME_IQ_TMNR_NR_CSHK_VAL					8
#define KDRV_IME_IQ_EWG_CURVE_LEN						9
#define KDRV_IME_IQ_NOISE_CURVE_LEN						17

/************************
    KDRV IME structure - local chroma adaptation parameters
************************/
typedef enum {
	KDRV_IME_IQ_EDG_KER_SIZE_3X3 = 0,
	KDRV_IME_IQ_EDG_KER_SIZE_5X5 = 1
} KDRV_IME_IQ_LCA_DRF_EDG_KER_SIZE;

typedef enum {
	KDRV_IME_IQ_KER_SIZE_3X3 = 0,
	KDRV_IME_IQ_KER_SIZE_5X5 = 1,
	KDRV_IME_IQ_KER_SIZE_7X7 = 2,
	KDRV_IME_IQ_KER_SIZE_9X9 = 3,
	KDRV_IME_IQ_KER_SIZE_11X9 = 4,
	KDRV_IME_IQ_KER_SIZE_13X9 = 5,
	KDRV_IME_IQ_KER_SIZE_15X9 = 6,
} KDRV_IME_IQ_LCA_DRF_KER_SIZE;

typedef struct {
	KDRV_IME_IQ_LCA_DRF_EDG_KER_SIZE edge_ker_size_sel;
	UINT32 edge_th[2];
	KDRV_IME_IQ_LCA_DRF_KER_SIZE sr_ker_size_sel;
	KDRV_IME_IQ_LCA_DRF_KER_SIZE mr_ker_size_sel;
	UINT8 ctr_3x3_y_th[KDRV_IME_IQ_RF_CTR_TH_LEN];
	UINT8 ctr_3x3_u_th[KDRV_IME_IQ_RF_CTR_TH_LEN];
	UINT8 ctr_3x3_v_th[KDRV_IME_IQ_RF_CTR_TH_LEN];
	UINT8 sr_y_wet; 											///<Center Reference weight of Y channel for smooth region
	UINT8 sr_uv_wet;
	UINT8 er_y_wet; 											///<Center Reference weight of Y channel for edge region
	UINT8 er_uv_wet;
	UINT8 ss_region_y_th[KDRV_IME_IQ_RF_SS_RGN_TH_LEN]; 		///<Range filter threshold of Y channel for still-smoth region
	UINT16 ss_region_uv_th[KDRV_IME_IQ_RF_SS_RGN_TH_LEN];
	UINT8 edge_region_y_th[KDRV_IME_IQ_RF_EDG_RGN_TH_LEN];		///<Range filter threshold of Y channel for still-edge region
	UINT16 edge_region_uv_th[KDRV_IME_IQ_RF_EDG_RGN_TH_LEN];
	UINT8 motion_region_y_th[KDRV_IME_IQ_RF_MON_RGN_TH_LEN]; 	///<Range filter threshold of Y channel for motion region
	UINT16 motion_region_uv_th[KDRV_IME_IQ_RF_MON_RGN_TH_LEN];

	//UINT8 uv_ref_y_wt;
	//UINT8 y_out_wt;
	//UINT8 uv_out_wt;
} KDRV_IME_IQ_LCA_RF_PARAM;

typedef struct {
	UINT8 still_y_gain[KDRV_IME_IQ_CORING_GAIN_LEN];
	UINT8 motion_y_gain[KDRV_IME_IQ_CORING_GAIN_LEN];
	UINT8 still_u_gain[KDRV_IME_IQ_CORING_GAIN_LEN];
	UINT8 motion_u_gain[KDRV_IME_IQ_CORING_GAIN_LEN];
	UINT8 still_v_gain[KDRV_IME_IQ_CORING_GAIN_LEN];
	UINT8 motion_v_gain[KDRV_IME_IQ_CORING_GAIN_LEN];
} KDRV_IME_IQ_LCA_CORING_GAIN_PARAM;

typedef struct {
	UINT8 still_y_coff[KDRV_IME_IQ_CUTOUT_LEN];
	UINT8 motion_y_coff[KDRV_IME_IQ_CUTOUT_LEN];
	UINT8 still_u_coff[KDRV_IME_IQ_CUTOUT_LEN];
	UINT8 motion_u_coff[KDRV_IME_IQ_CUTOUT_LEN];
	UINT8 still_v_coff[KDRV_IME_IQ_CUTOUT_LEN];
	UINT8 motion_v_coff[KDRV_IME_IQ_CUTOUT_LEN];
} KDRV_IME_IQ_LCA_CUTOUT_PARAM;

typedef enum {
	KDRV_IME_IQ_DBG_COLOR_MODE 		= 0,
	KDRV_IME_IQ_EDGE_INFORMATION	= 8,
	KDRV_IME_IQ_DBG_UNKNOWN,
} KDRV_IME_IQ_LCA_DBG_SEL;

typedef struct {
	BOOL  enable;
	KDRV_IME_IQ_LCA_DBG_SEL ch_sel;
	UINT8 ch_ofs;
	UINT16 x_pos;
} KDRV_IME_IQ_LCA_DBG_PARAM;

typedef enum {
	KDRV_IME_IQ_LCA_PROC_LOCATION_PRE_3DNR  = 0,
	KDRV_IME_IQ_LCA_PROC_LOCATION_POST_3DNR = 1,
	ENUM_DUMMY4WORD(KDRV_IME_IQ_LCA_PROC_LOCATION)
} KDRV_IME_IQ_LCA_PROC_LOCATION;

typedef struct {
	KDRV_IME_IQ_LCA_RF_PARAM rf;					///< Range Filter
	KDRV_IME_IQ_LCA_CORING_GAIN_PARAM coring_gain;	///< coring gain
	KDRV_IME_IQ_LCA_CUTOUT_PARAM cutout_coff;		///< cutout_coff
	UINT8 final_y_out_wt;
	UINT8 final_uv_out_wt;
	KDRV_IME_IQ_LCA_DBG_PARAM dbg;					///< lca debug methods

	KDRV_IME_IQ_LCA_PROC_LOCATION set_proc_location; 	///< processing location
} KDRV_IME_IQ_LCA;

/************************
    KDRV IME structure - dark and bright region chroma suppression
************************/
typedef enum {
	KDRV_IME_IQ_DBCS_DK_MODE   = 0,   ///< dark mode
	KDRV_IME_IQ_DBCS_BT_MODE   = 1,   ///< bright mode
	KDRV_IME_IQ_DBCS_BOTH_MODE  = 2,  ///< dark and bright mode
	ENUM_DUMMY4WORD(KDRV_IME_IQ_DBCS_MODE_SEL)
} KDRV_IME_IQ_DBCS_MODE_SEL;

typedef struct {
	UINT8 enable;								///< dark and bright region chroma suppression function enable
	KDRV_IME_IQ_DBCS_MODE_SEL op_mode;			///< Process mode
	UINT8 cent_u;								///< Center value for U channel
	UINT8 cent_v;								///< Center value for V channel
	UINT16 step_y;								///< Step for luma
	UINT16 step_c;								///< Step for chroma
	UINT8 wt_y[KDRV_IME_IQ_DBCS_WT_LUT_TAB];	///< Weighting LUT for luma channel, 16 elements, range: [0, 16]
	UINT8 wt_c[KDRV_IME_IQ_DBCS_WT_LUT_TAB];	///< Weighting LUT for luma channel, 16 elements, range: [0, 16]
} KDRV_IME_IQ_DBCS;

/************************
    KDRV IME structure - TMNR info.
************************/
/**
    IME enum - An option of update mode in motion estimation
*/
typedef enum {
	KDRV_IME_IQ_ME_UPDATE_RAND  = 0,
	KDRV_IME_IQ_ME_UPDATE_FIXED = 1,
	ENUM_DUMMY4WORD(KDRV_IME_IQ_TMNR_ME_UPDATE_MODE)
} KDRV_IME_IQ_TMNR_ME_UPDATE_MODE;

/**
    IME structure - 3DNR motion estimation parameters
*/
typedef struct {
	KDRV_IME_IQ_TMNR_ME_UPDATE_MODE update_mode;	///< An option of update mode in motion estimation. Range 0~1
	UINT8 boundary_set;								///< Set image boundary as still type or not.  Range 0~1
	UINT8 sad_shift;								///< Right shift number for SAD statistic data, range = [0, 15]
	UINT8 cost_blend;								///< Cost blending ratio, 0 for automatic cost, 0xF for user define

	UINT8 rand_bit_x;	///<range 1~7
	UINT8 rand_bit_y;	///<range 1~7
	UINT16 min_detail;	///< range 0~16383

	UINT16 sad_penalty[KDRV_IME_IQ_TMNR_ME_SAD_PENALTY_TAB];		///< range 0~1023
	UINT8 switch_th[KDRV_IME_IQ_TMNR_ME_SWITCH_THRESHOLD_TAB];		///< range 0~255
	UINT8 switch_rto;												///< range 0~255
	UINT8 detail_penalty[KDRV_IME_IQ_TMNR_ME_DETAIL_PENALTY_TAB];	///< range 0~15
	UINT8 probability[KDRV_IME_IQ_TMNR_ME_PROBABILITY_TAB];			///< range 0~1
} KDRV_IME_IQ_TMNR_ME;

/**
    IME structure - 3DNR motion detection parameters
*/
typedef struct {
	UINT8 sad_coefa[KDRV_IME_IQ_TMNR_MD_SAD_COEFA_TAB];    ///< Edge coefficient of motion detection. Range 0~63
	UINT16 sad_coefb[KDRV_IME_IQ_TMNR_MD_SAD_COEFB_TAB];   ///< Offset of motion detection. Range 0~16383
	UINT16 sad_std[KDRV_IME_IQ_TMNR_MD_SAD_STD_TAB];       ///< Standard deviation of motion detection. Range 0~16383
	UINT8 fth[KDRV_IME_IQ_TMNR_MD_FINAL_THRESHOLD_TAB];    ///< final threshold of motion detection. Range 0~63
} KDRV_IME_IQ_TMNR_MD;

/**
    IME structure - 3DNR motion detection parameters for ROI
*/
typedef struct {
	UINT8 fth[KDRV_IME_IQ_TMNR_MD_ROI_FINAL_THRESHOLD_TAB];   ///< final threshold of motion detection.Range 0~63
} KDRV_IME_IQ_TMNR_MD_ROI;

/**
    IME structure - 3DNR motion compensation parameters
*/
typedef struct {
	UINT16 sad_base[KDRV_IME_IQ_TMNR_MC_SAD_BASE_TAB];     ///< base level of noise. Range 0~16383
	UINT8 sad_coefa[KDRV_IME_IQ_TMNR_MC_SAD_COEFA_TAB];    ///< edge coefficient of motion compensation.Range 0~63
	UINT16 sad_coefb[KDRV_IME_IQ_TMNR_MC_SAD_COEFB_TAB];   ///< offset of motion compensation.Range 0~16383
	UINT16 sad_std[KDRV_IME_IQ_TMNR_MC_SAD_STD_TAB];       ///< standard deviation of motion compensation.Range 0~16383
	UINT8 fth[KDRV_IME_IQ_TMNR_MC_FINAL_THRESHOLD_TAB];    ///< final threshold of motion compensation. Range 0~63
} KDRV_IME_IQ_TMNR_MC;

/**
    IME structure - 3DNR motion compensation parameters for ROI
*/
typedef struct {
	UINT8 fth[KDRV_IME_IQ_TMNR_MC_ROI_FINAL_THRESHOLD_TAB];   ///< final threshold of motion compensation.Range 0~63
} KDRV_IME_IQ_TMNR_MC_ROI;

/**
    IME structure - 3DNR patch selection parameters
*/
typedef enum {
	KDRV_IME_IQ_MV_INFO_MODE_AVERAGE  = 0,
	KDRV_IME_IQ_MV_INFO_MODE_LPF      = 1,
	KDRV_IME_IQ_MV_INFO_MODE_MINIMUM  = 2,
	KDRV_IME_IQ_MV_INFO_MODE_MAXIMUM  = 3,
	ENUM_DUMMY4WORD(KDRV_IME_IQ_TMNR_MV_INFO_MODE)
} KDRV_IME_IQ_TMNR_MV_INFO_MODE;

typedef struct {
	UINT8 smart_roi_ctrl_en;					///< ROI control. Range 0~1
	UINT8 mv_check_en;							///< MV checkiong process in PS module. Range 0~1
	UINT8 roi_mv_check_en;						///< MV checking process for ROI. Range 0~1

	KDRV_IME_IQ_TMNR_MV_INFO_MODE mv_info_mode;			///< Mode option for MV length calculation. Range 0~3
	UINT8 mv_th;										///< MV threshold. Range 0~63
	UINT8 roi_mv_th;									///< MV threshold for ROI. Range 0~63
	UINT8 mix_ratio[KDRV_IME_IQ_TMNR_PS_MIX_RATIO_TAB];	///< Mix ratio in patch selection. Range 0~255
	UINT8 ds_th;										///< Threshold of motion status down-sampling. Range 0~31
	UINT8 ds_th_roi;									///< Threshold for motion status down-sampling for ROI.Range 0~31
	UINT16 fs_th;										////< Threshold of patch error. Range 0~16383.
	UINT16 blur_eth;									////< Threshold of TF0 Blur for smooth region
} KDRV_IME_IQ_TMNR_PS;

/**
    IME enum - Strength option for Y-channel pre-filter
*/
typedef enum {
	KDRV_IME_IQ_PRE_FILTER_Y_STR_DISABLE = 0,	///< off
	KDRV_IME_IQ_PRE_FILTER_Y_STR_1 = 1,			///< type 1 filter
	KDRV_IME_IQ_PRE_FILTER_Y_STR_2 = 2,			///< type 2 filter
	ENUM_DUMMY4WORD(KDRV_IME_IQ_TMNR_PRE_FILTER_Y_STR)
} KDRV_IME_IQ_TMNR_PRE_FILTER_Y_STR;

typedef enum {
	KDRV_IME_IQ_PRE_FILTER_DISABLE = 0,    ///< disable
	KDRV_IME_IQ_PRE_FILTER_TYPE_1 = 1,     ///< 3x3
	KDRV_IME_IQ_PRE_FILTER_TYPE_2 = 2,     ///< 3x3
	KDRV_IME_IQ_PRE_FILTER_TYPE_3 = 3,     ///< 5x5
	ENUM_DUMMY4WORD(KDRV_IME_IQ_TMNR_PRE_FILTER_TYPE)
} KDRV_IME_IQ_TMNR_PRE_FILTER_TYPE;

/**
    IME structure - 3DNR noise filter parameters
*/
typedef struct {
	UINT8 luma_ch_en;	///< noise filter enable for luma channel. Range 0~1
	UINT8 chroma_ch_en;	///< noise filter enable for chroma channel.Range 0~1

	UINT8 center_wzeros_y;	///< Set weighting as 0 to the center pixel in internal NR-filter. Range 0~1

	UINT8 luma_residue_th[KDRV_IME_IQ_TMNR_NR_LUMA_RESIDUE_TH_TAB];	///< Protection threshold of luma channel. Range 0~255
	UINT8 chroma_residue_th;	///< Protection threshold of luma channel. Range 0~255

	UINT8 freq_wet[KDRV_IME_IQ_TMNR_NR_FREQ_WEIGHT_TAB];	///< Filter weighting for low frequency. Range 0~255
	UINT8 luma_wet[KDRV_IME_IQ_TMNR_NR_LUMA_WEIGHT_TAB];	///< Filter intensity weighting. Range 0~255

	KDRV_IME_IQ_TMNR_PRE_FILTER_Y_STR pre_y_blur_str;	///< Strength option for luma channel pre-filter.Range 0~2
	UINT8 pf_str;										///<  Strength of pre-filters
	UINT8 pre_filter_str[KDRV_IME_IQ_TMNR_NR_PRE_FILTER_STRENGTH_TAB];	///< Strength of pre-filtering for low frequency. Range 0~255
	UINT8 pre_filter_rto[KDRV_IME_IQ_TMNR_NR_PRE_FILTER_RATION_TAB];	///< adjustment ratio of pre-filtering for transitional object.Range 0~255

	UINT8 snr_str[KDRV_IME_IQ_TMNR_NR_SFILTER_STRENGTH_TAB];	///< Strength of spatial filter for still object.Range 0~255
	UINT8 tnr_str[KDRV_IME_IQ_TMNR_NR_TFILTER_STRENGTH_TAB];	///< Strength of temporal filter for still object.Range 0~255

	UINT32 snr_base_th;					///< Base threshold of spatial noise reduction.Range 0~65535
	UINT32 tnr_base_th;					///< Base threshold of spatial noise reduction.Range 0~65535

	UINT8 luma_3d_lut[KDRV_IME_IQ_TMNR_NR_LUMA_LUT_TAB];			///< Noise reduction LUT for luma channel.Range 0~127
	UINT8 luma_3d_rto[KDRV_IME_IQ_TMNR_NR_LUMA_RATIO_TAB];			///< Adjustment ratio 0 of noise reduction LUT for luma channel.Range 0~255
	UINT8 luma_comp_str;											///< luma compensation strength
	UINT8 chroma_3d_lut[KDRV_IME_IQ_TMNR_NR_CHROMA_LUT_TAB];		///< Noise reduction LUT for chroma channel.Range 0~127
	UINT8 chroma_3d_rto[KDRV_IME_IQ_TMNR_NR_CHROMA_RATIO_TAB];		///< Adjustment ratio 0 of noise reduction LUT for chroma channel.Range 0~255

	UINT8 luma_nr_type;	///< 3DNR filter option, Range 0~1

	UINT8 tf0_blur_str[KDRV_IME_IQ_TMNR_NR_TF0_FILTER_TAB];
	UINT8 tf0_blur_estr;
	UINT8 tf0_y_str[KDRV_IME_IQ_TMNR_NR_TF0_FILTER_TAB];
	UINT8 tf0_c_str[KDRV_IME_IQ_TMNR_NR_TF0_FILTER_TAB];
	//538 add
	UINT8 u_tf0_md_th;											///< Threshold of motion detection for chroma U channel.Range 0~255
	UINT8 v_tf0_md_th;
	UINT8 c_tf0_ratio[KDRV_IME_IQ_TMNR_NR_C_TF0_RATIO_TAB];	///< Strength of temporal filter in TF0 for still/moving object.Range 0~255
	UINT8 motion_sat_ratio;									///< Saturation adjustment ratio for moving object.Range 0~255
	UINT8 c_tf0_tprot_th; 										///< Residual threshold of temporal filter in TF0.Range 0~15
	UINT16 cshk_th[KDRV_IME_IQ_TMNR_NR_CSHK_TH_TAB];			///< Color diterhing thresthod.Range 0~1023
	UINT8 cshk_val[KDRV_IME_IQ_TMNR_NR_CSHK_VAL];				///< Color diterhing bit number.Range 0~7
} KDRV_IME_IQ_TMNR_NR;

/**
    IME structure - 3DNR debug parameters
*/
typedef struct {
	UINT8 dbg_mv0;	///< debug for mv = 0
	UINT8 dbg_mode;	///< debug mode selection. Range 0~6
} KDRV_IME_IQ_TMNR_DBG;

typedef struct {
	UINT8 en;
	UINT8 start_point;
	UINT8 step_size;
} KDRV_IME_IQ_TMNR_FCVG;

typedef struct {
	UINT8 enable;
	KDRV_IME_IQ_TMNR_ME me_param;
	KDRV_IME_IQ_TMNR_MD md_param;
	KDRV_IME_IQ_TMNR_MD_ROI md_roi_param;
	KDRV_IME_IQ_TMNR_MC mc_param;
	KDRV_IME_IQ_TMNR_MC_ROI mc_roi_param;
	KDRV_IME_IQ_TMNR_PS ps_param;
	KDRV_IME_IQ_TMNR_NR nr_param;
	KDRV_IME_IQ_TMNR_DBG dbg_param;
	KDRV_IME_IQ_TMNR_FCVG fcvg_param;
} KDRV_IME_IQ_TMNR;

/************************
    KDRV IME structure - YCbCr converter parameters
************************/
/**
    KDRV IME enum - YCbCr converter selection
*/
typedef enum {
	KDRV_IME_IQ_YCC_CVT_BT601 = 0,	///< BT.601
	KDRV_IME_IQ_YCC_CVT_BT709 = 1,	///< BT.709
	ENUM_DUMMY4WORD(KDRV_IME_IQ_YCC_CVT_SEL)
} KDRV_IME_IQ_YCC_CVT_SEL;

typedef struct {
	UINT8 enable;						///< YCbCrconverter function enable/disable
	KDRV_IME_IQ_YCC_CVT_SEL cvt_sel;	///< YCbCr converter selection, BT.601 or BT.709
} KDRV_IME_IQ_YCC_CVT;

/************************
    KDRV IME structure - post sharpen
************************/
typedef enum {
	KDRV_IME_IQ_SHP_EDG_WT_SRC_Y			= 0,	///<
	KDRV_IME_IQ_SHP_EDG_WT_SRC_INV_GAMMA	= 1,	///<
} KDRV_IME_IQ_SHP_EDG_WT_SRC_SEL;

typedef enum {
	KDRV_IME_IQ_SHP_JND_FILT_3x3 = 0,
	KDRV_IME_IQ_SHP_JND_FILT_5x5 = 1,
} KDRV_IME_IQ_SHP_JND_FILTER_SEL;

typedef struct {
	BOOL   shp_en;
	BOOL   motion_bit_en;
	BOOL   dbg_en;
	KDRV_IME_IQ_SHP_EDG_WT_SRC_SEL shp_src_sel;
	KDRV_IME_IQ_SHP_JND_FILTER_SEL jnd_filt_sel;
	UINT8  weight_th;
	UINT8  weight_gain;
	UINT8  noise_lv;
	UINT16 flat_th;
	UINT16 edge_th;
	UINT8  wet;
	UINT8  noise_curve[KDRV_IME_IQ_NOISE_CURVE_LEN];
	UINT16 ewg_curve[KDRV_IME_IQ_EWG_CURVE_LEN];
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
} KDRV_IME_IQ_SHARPEN;

/************************
    misc structure
************************/

/************************
    main structure
************************/
typedef struct {
	KDRV_IME_IQ_LCA					lca;
	KDRV_IME_IQ_DBCS				dbcs;
	KDRV_IME_IQ_TMNR				tmnr;
	KDRV_IME_IQ_YCC_CVT				yuv_cvt;
	KDRV_IME_IQ_SHARPEN				sharpen;

	KDRV_IME_UPDATE					update;
} KDRV_IME_IQ_CFG;

#endif
/*******************************************************************************
*	                          IPP IQ parameter End                             *
*******************************************************************************/

//538 modified
#define KDRV_IME_PM_COORD_TAB       (10)
#define KDRV_IME_PM_COORD_TAB2      (4)

#define KDRV_IME_DS_COLOR_KEY_TAB   (4)
#define KDRV_IME_DS_PLT_TAB         (16)




//#if(IME_538_KDRV_FEATURE == 0)
#define KDRV_IME_MRNR_EDGE_DETECT_L1_LEN (8)
#define KDRV_IME_MRNR_EDGE_DETECT_L2_LEN (8)
#define KDRV_IME_MRNR_EDGE_SMOOTH_L1_LEN (8)
#define KDRV_IME_MRNR_EDGE_SMOOTH_L2_LEN (8)
//#endif

#define KDRV_IME_LCA_RANGE_TH_TAB      (3)
#define KDRV_IME_LCA_RANGE_WT_TAB      (4)
#define KDRV_IME_LCA_FILTER_TH_TAB     (5)
#define KDRV_IME_LCA_FILTER_WT_TAB     (6)


#define KDRV_IME_BIL_SPL_KNL_COEF_LEN  (6)
#define KDRV_IME_BIL_RNG_KNL_TH_LEN    (5)
//538 modified


#if(IME_538_KDRV_FEATURE == 0)
#define KDRV_IME_DRF_CTR_TH_LEN        (3)
#define KDRV_IME_DRF_FLAT_RGN_TH_LEN   (5)
#define KDRV_IME_DRF_EDG_RGN_TH_LEN    (5)
#define KDRV_IME_FREQ_CORING_LEN       (4)

#define KDRV_IME_INDEP_VA_WIN_NUM      (5)
#define KDRV_IME_VA_INDEP_WIN_MAX      (511)
#define KDRV_IME_VA_MAX_WINNUM         (16*16)
#define KDRV_IME_VA_BUF_SIZE           (8192) // 16(h-blks) x 16(v-blks) x 2(g1/g2)x 2(h/v) x 8(byte) = 8192
#define KDRV_IME_VA_INDEP_NUM          (5)
#endif

#define KDRV_IME_STRP_NUM_MAX (8)

typedef enum {
	KDRV_IME_PROC_MODE_LINKLIST = 0,	/* linlist mode */
	KDRV_IME_PROC_MODE_CPU,				/* cpu write register, only for debug */
	KDRV_IME_PROC_MODE_MAX,
} KDRV_IME_PROC_MODE;

typedef enum {
	KDRV_IME_QUERY_REG_NUM = 0,     /* get reg num for memory alloc.                data_tpye: UINT32 */
	KDRV_IME_QUERY_3DNR_BUFSIZE,    /* get 3dnr buffer size for memory alloc.       data_tpye: KDRV_IME_3DNR_QUERY_BUF_SIZE */
	KDRV_IME_QUERY_3DNR_STAINFO,    /* get 3dnr sta information.                    data_tpye: KDRV_IME_3DNR_QUERY_STA_INFO */
	KDRV_IME_QUERY_STRP_INFO,       /* get ime stripe info for dce cal.             data_type: KDRV_IME_STRP_INFO */
	KDRV_IME_QUERY_ID_MAX,
} KDRV_IME_QUERY_ID;

/*********************
IME DRAM ADDR
**********************/
typedef struct{
	ULONG va;
	ULONG pa;
}KDRV_IME_BUF_ADDR;
/*********************
IME DBG info
**********************/
typedef struct{
	UINT32  ime_frm_ed_cnt;
	UINT32  ime_frm_st_cnt;
	UINT32  ime_strp_done_cnt;
	UINT32  ime_ll_ed_cnt;
	UINT32  ime_ll_error_cnt;
	UINT32  ime_ll_late_cnt;
	UINT32  ime_frame_err_cnt;
	UINT32  ime_3dnr_slice_ed_cnt;
	UINT32  ime_3dnr_mot_ed_cnt;
	UINT32  ime_3dnr_mv_ed_cnt;
	UINT32  ime_3dnr_sta_ed_cnt;
	UINT32  ime_p1_enc_ovfl_cnt;
	UINT32  ime_3dnr_enc_ovfl_cnt;
	UINT32  ime_3dnr_dec_err_cnt;
	UINT32  ime_p2_rdy;
	UINT32  ime_p2_clr;
	UINT32  ime_p3_rdy;
	UINT32  ime_p3_clr;
}KDRV_IME_DBG_INFO;


/*********************
IME 3DNR info
**********************/
typedef struct{
	UINT32  ime_ro_3dnr_motion_sum;
	UINT32  ime_ro_3dnr_edge_sum;
}KDRV_IME_3DNR_INFO;




typedef struct {
	UINT32 in_w_min;
	UINT32 in_w_max;
	UINT32 in_w_align;
	UINT32 in_h_min;
	UINT32 in_h_max;
	UINT32 in_h_align;
	UINT32 in_addr_align;
	UINT32 in_uv_addr_align;
	UINT32 in_lofs_align;
	UINT32 in_uv_lofs_align;

	UINT32 _3dnr_ref_w_align;
	UINT32 _3dnr_ref_h_align;
	UINT32 _3dnr_buf_lofs_align;
	UINT32 _3dnr_buf_addr_align;

	UINT32 scl_w_min;
	UINT32 scl_w_max;
	UINT32 scl_w_align;
	//UINT32 scl_nvx2_w_align;
	UINT32 scl_h_min;
	UINT32 scl_h_max;
	UINT32 scl_h_align;
	//UINT32 scl_nvx2_h_align;

	UINT32 post_crop_x_align;
	//UINT32 crop_nvx2_x_align;
	UINT32 post_crop_y_align;
	UINT32 post_crop_w_align;
	UINT32 post_crop_h_align;

#if(IME_538_KDRV_FEATURE == 0)
	UINT32 pre_crop_x_align;
	UINT32 pre_crop_y_align;
	UINT32 pre_crop_w_align;
	UINT32 pre_crop_h_align;
#endif

	UINT32 out_w_min;
	UINT32 out_w_max;
	UINT32 out_w_align;
	UINT32 out_h_min;
	UINT32 out_h_max;
	UINT32 out_h_align;

	UINT32 out_y_addr_align;
	UINT32 out_uv_addr_align;
	UINT32 out_lofs_align;
	UINT32 out_uv_lofs_align;

	UINT32 lca_w_min;
	UINT32 lca_w_max;
	UINT32 lca_w_align;
	UINT32 lca_h_min;
	UINT32 lca_h_max;
	UINT32 lca_h_align;
	UINT32 lca_buf_lofs;
	UINT32 lca_buf_addr_align;

	UINT32 pm_pxl_w_min;
	UINT32 pm_pxl_w_max;
	UINT32 pm_pxl_w_align;
	UINT32 pm_pxl_h_min;
	UINT32 pm_pxl_h_max;
	UINT32 pm_pxl_h_align;
	UINT32 pm_pxl_buf_lofs;
	UINT32 pm_pxl_buf_addr_align;
} KDRV_IME_LMT;

typedef struct {
	UINT32 in_w_min;
	UINT32 in_w_max;
	UINT32 in_w_align;
	UINT32 in_h_min;
	UINT32 in_h_max;
	UINT32 in_h_align;
	UINT32 in_addr_align;
	UINT32 in_uv_addr_align;
	UINT32 in_lofs_align;
	UINT32 in_uv_lofs_align;

#if(IME_538_KDRV_FEATURE == 0)
	UINT32 scl_w_min;
	UINT32 scl_w_max;
	UINT32 scl_w_align;
	UINT32 scl_h_min;
	UINT32 scl_h_max;
	UINT32 scl_h_align;
#endif

} KDRV_IME_IN_LMT;

typedef struct _KDRV_IME_IMG_SIZE_ {
	UINT32    h_size;    ///< Horizontal image size
	UINT32    v_size;    ///< Vertical image size
} KDRV_IME_IMG_SIZE;

typedef enum {
	KDRV_IME_SCL_BICUBIC     = 0,    ///< bicubic interpolation
	KDRV_IME_SCL_BILINEAR    = 1,    ///< bilinear interpolation
	KDRV_IME_SCL_NEAREST     = 2,    ///< nearest interpolation
	KDRV_IME_SCL_INTEGRATION = 3,    ///< integration interpolation,support only scale down
	KDRV_IME_SCL_AUTO        = 4,    ///< automatical calculation
	KDRV_IME_SCL_METHOD_MAX,
} KDRV_IME_SCL_METHOD;

/*********************
Output path Structure
**********************/
typedef enum {
	KDRV_IME_PATH_1 = 0,
	KDRV_IME_PATH_2,
	KDRV_IME_PATH_3,
	KDRV_IME_PATH_4,
	KDRV_IME_PATH_REF,
	KDRV_IME_PATH_NUM_MAX,
} KDRV_IME_PATH_ID;

typedef struct {
	UINT8 enable;
	UINT8 dma_en;
	UINT8 flip_en;
	KDRV_IPP_FRAME frm;
	USIZE scl_size;
	URECT pre_crp_window; //538 not support
	URECT post_crp_window;

	UINT32 sca_init_ofs_h;

	#if 0 // 690 removed
	UINT8  sprt_en;
	UINT32 sprt_pos;
	KDRV_IME_BUF_ADDR sprt_addr2[3];
	UINT32 sprt_lofs2[3];
	#endif
} KDRV_IME_OUT_PATH;

typedef struct {
	UINT32 scl_th;                  /* scale done ratio threshold, [31..16]output, [15..0]input */
	KDRV_IME_SCL_METHOD method_l;   /* scale method when scale down ratio <= scl_th(output/input) */
	KDRV_IME_SCL_METHOD method_h;   /* scale method when scale down ratio >  scl_th(output/input) */
} KDRV_IME_SCL_METHOD_SEL;

/*********************
Get stripe info
**********************/
typedef struct {
	KDRV_IPP_OPMODE mode;
	UINT8 _3dnr_en;         /* 3dnr enable */
	UINT8 yuv422_one_en;    /* yuv422 one plane format enable */
	UINT8 in_yuv_compress_en;  /* nvx2 input format enable */
	UINT8 out_yuv_compress_en; /* nvx2 output format enable */
	UINT8 lca_en;           /* lca enable*/
	UINT8 pxl_subout_en;    /* private mask pixelation enable*/
	UINT8 mb_en;            /* path1 mb format enable */
	UINT8 nn_isp_en;        /* nn isp enable */
	KDRV_IME_SCL_METHOD_SEL scl_method_sel; /* scale method threshold */

	UINT32 in_width;        /* use to calculate max scale down rate */
	UINT32 in_height;
	BOOL path_en[KDRV_IME_PATH_NUM_MAX];
	USIZE path_size[KDRV_IME_PATH_NUM_MAX];
	UINT32 lca_width;       /* 0 for disable */
} KDRV_IME_STRP_CAL_INFO;

typedef struct {
	UINT16 alignment;
	UINT16 overlap;
	UINT32 ovlp_sel;
	UINT16 partition;
	UINT32 prt_sel;
	UINT32 strp_num;
	UINT32 h_l;
	UINT32 h_n;
	UINT32 h_m;
	UINT32 v_l;
	UINT32 v_n;
	UINT32 v_m;
	KDRV_IME_SCL_METHOD scl_method[KDRV_IME_PATH_NUM_MAX];
	UINT32 h_varied_en;
	UINT32 ovlp_h_size;
	UINT32 prt_h_size;
	UINT16 strp_max_width;
} KDRV_IME_STRP_RST;

typedef struct {
	KDRV_IME_STRP_CAL_INFO cal_info;
	KDRV_IME_STRP_RST rst;
} KDRV_IME_STRP_INFO;

/*********************
LCA Structure
**********************/
#if 0
typedef enum {				///<LCA processing location
	KDRV_IME_LCA_PROC_LOCATION_PRE_3DNR  = 0,
	KDRV_IME_LCA_PROC_LOCATION_POST_3DNR = 1,
	ENUM_DUMMY4WORD(KDRV_IME_LCA_PROC_LOCATION)
} KDRV_IME_LCA_PROC_LOCATION;
#endif

typedef struct {
	UINT8 in_enable;       /* lca function enable */
	USIZE in_size;         /* input width, height */
	USIZE out_size;        /* output width, height */
	//KDRV_IME_LCA_PROC_LOCATION set_proc_location; /* processing location */
} KDRV_IME_LCA_IOCFG;

typedef enum {
	KDRV_IME_RANGE_8   = 0,  ///< range 8
	KDRV_IME_RANGE_16  = 1,  ///< range 16
	KDRV_IME_RANGE_32  = 2,  ///< range 32
	KDRV_IME_RANGE_64  = 3,  ///< range 64
	KDRV_IME_RANGE_128 = 4,  ///< range 128
	ENUM_DUMMY4WORD(KDRV_IME_RANGE_SEL)
} KDRV_IME_RANGE_SEL;

typedef enum {
	KDRV_IME_LCA_SUBOUT_SRC_A = 0, ///< source: after LCA
	KDRV_IME_LCA_SUBOUT_SRC_B = 1, ///< source: before LCA
} KDRV_IME_LCA_SUBOUT_SRC;

typedef struct {
	UINT8                  ref_y_wt;   ///< Chroma reference weighting for Y channels
	UINT8                  ref_uv_wt;  ///< Chroma reference weighting for UV channels
	UINT8                  out_uv_wt;  ///< Chroma adaptation output weighting

	KDRV_IME_RANGE_SEL     y_rng;      ///< Chroma adaptation range selection for UV channels, Max = KDRV_IME_RANGE_128
	KDRV_IME_RANGE_SEL     y_wt_prc;   ///< Chroma adaptation weighting precision for UV channel, Max = KDRV_IME_RANGE_64
	UINT8                  y_th;       ///< Chroma adaptation range threshold for UV channels
	UINT8                  y_wt_s;     ///< Chroma adaptation weighting start value for UV channels
	UINT8                  y_wt_e;     ///< Chroma adaptation weighting end value for UV channels


	KDRV_IME_RANGE_SEL     uv_rng;     ///< Chroma adaptation range selection for UV channels, Max = KDRV_IME_RANGE_128
	KDRV_IME_RANGE_SEL     uv_wt_prc;  ///< Chroma adaptation weighting precision for UV channel, Max = KDRV_IME_RANGE_64
	UINT8                  uv_th;      ///< Chroma adaptation range threshold for UV channels
	UINT8                  uv_wt_s;    ///< Chroma adaptation weighting start value for UV channels
	UINT8                  uv_wt_e;    ///< Chroma adaptation weighting end value for UV channels
} KDRV_IME_LCA_IQC_PARAM;

typedef struct {
	UINT8              enable;      ///< Function enable
	KDRV_IME_RANGE_SEL uv_rng;      ///< Chroma adjustment range selection for UV channels, Max = KDRV_IME_RANGE_128
	KDRV_IME_RANGE_SEL uv_wt_prc;   ///< Chroma adjustment weighting precision for UV channel, Max = KDRV_IME_RANGE_64
	UINT8              uv_th;       ///< Chroma adjustment range threshold for UV channels
	UINT8              uv_wt_s;     ///< Chroma adjustment weighting start value for UV channels
	UINT8              uv_wt_e;     ///< Chroma adjustment weighting end value for UV channels
} KDRV_IME_LCA_CA_PARAM;

typedef struct {
	UINT8                  enable;     ///< Function enable
	UINT8                  ref_wt;     ///< Luma adaptation reference weighting for UV channels
	UINT8                  out_wt;     ///< Luma adaptation output weighting
	KDRV_IME_RANGE_SEL     rng;        ///< Luma adaptation range selection for UV channels, Max = KDRV_IME_RANGE_128
	KDRV_IME_RANGE_SEL     wt_prc;     ///< Luma adaptation weighting precision for UV channel, Max = KDRV_IME_RANGE_64
	UINT8                  th;         ///< Luma adaptation range threshold for UV channels
	UINT8                  wt_s;       ///< Luma adaptation weighting start value for UV channels
	UINT8                  wt_e;       ///< Luma adaptation weighting end value for UV channels
} KDRV_IME_LCA_IQL_PARAM;

typedef struct {
	UINT8 u_th0;            ///< Gray Statistical threshold for U channel
	UINT8 u_th1;            ///< Gray Statistical threshold for U channel
	UINT8 v_th0;            ///< Gray Statistical threshold for V channel
	UINT8 v_th1;            ///< Gray Statistical threshold for V channel
} KDRV_IME_LCA_GRAY_STA_TH;

typedef struct {
	UINT8 rng_th[KDRV_IME_LCA_RANGE_TH_TAB];    ///< UINT8[3], Range filter threshold for reference center
	UINT8 rng_wt[KDRV_IME_LCA_RANGE_WT_TAB];    ///< UINT8[4], Range filter weighting for reference center
	UINT8 cent_wt;                              ///< Reference center weighting
	UINT8 outl_dth;                             ///< Outlier difference threshold
	UINT8 outl_th;                              ///< Reference center outlier threshold
} KDRV_IME_LCA_REFCENT;

typedef struct {
	KDRV_IME_LCA_REFCENT y;     ///< for Y channel
	KDRV_IME_LCA_REFCENT uv;    ///< for UV channel
} KDRV_IME_LCA_REFCENT_PARAM;

typedef enum {
	KDRV_IME_LCA_FLTR_SIZE_3x3 = 0,         ///< 3x3 window
	KDRV_IME_LCA_FLTR_SIZE_5x5 = 1,         ///< 5x5 window
	KDRV_IME_LCA_FLTR_SIZE_7x7 = 2,         ///< 7x7 window
	KDRV_IME_LCA_FLTR_SIZE_9x9 = 3,         ///< 9x9 window
} KDRV_IME_LCA_FILT_SIZE;

typedef struct {
	UINT8  hv_th;                   ///< Edge direction threshold for two diagonal directions
	UINT8  pn_th;                   ///< Edge direction threshold for horizontal and vertical directions
} KDRV_IME_LCA_EDG_DIR_TH_PARAM;

typedef enum {
	KDRV_IME_LCA_EKNL_SIZE_3x3 = 0,         ///< 3x3 window
	KDRV_IME_LCA_EKNL_SIZE_5x5 = 1,         ///< 5x5 window
} KDRV_IME_LCA_EDGE_KER_SIZE;

typedef struct {
	UINT8 th[KDRV_IME_LCA_FILTER_TH_TAB];   ///< UINT32[5], Filter threshold LUT
	UINT8 wt[KDRV_IME_LCA_FILTER_WT_TAB];   ///< UINT32[6], Filter weighting LUT
} KDRV_IME_LCA_FILTER;

typedef struct {
	UINT8 enable;                               ///< filter Y channel enable
	KDRV_IME_LCA_FILT_SIZE size;                ///< filter size
	KDRV_IME_LCA_EDG_DIR_TH_PARAM edg_dir;      ///< filter edge direction threshold
	KDRV_IME_LCA_EDGE_KER_SIZE edg_ker_size;    ///< edge kernel size
	KDRV_IME_LCA_FILTER set_y;                  ///< Filter parameters for Y Channel
	KDRV_IME_LCA_FILTER set_u;                  ///< Filter parameters for U Channel
	KDRV_IME_LCA_FILTER set_v;                  ///< Filter parameters for V Channel
} KDRV_IME_LCA_FILTER_PARAM;

typedef struct {
	UINT32 u;           ///< average for U channel
	UINT32 v;           ///< average for V channel
} KDRV_IME_LCA_GRAY_AVG;

/*********************
3DNR Structure
**********************/

typedef enum {
	KDRV_IME_TMNR_REF_OUT_LOCATION_POST_3DNR  = 0,    ///< POST_3DNR
	KDRV_IME_TMNR_REF_OUT_LOCATION_POST_LCA   = 1,    ///< POST_LCA
	ENUM_DUMMY4WORD(KDRV_IME_TMNR_REF_OUT_LOCATION_SEL)
} KDRV_IME_TMNR_REF_OUT_LOCATION_SEL;

typedef struct {
	/* input parameter */
	USIZE size;
	UINT32 sta_max_spl_num;

	/* buffer size result */
	UINT32 mv_size;
	UINT32 ms_size;
	UINT32 ms_roi_size;
	UINT32 sta_size;
	UINT32 fcvg_size;
} KDRV_IME_3DNR_QUERY_BUF_SIZE;

typedef struct {
	/* input parameter */
	USIZE size;
	UINT32 sta_max_spl_num;

	/* sta info result */
	UINT32 lofs;
	UINT32 buf_size;
	UINT32 spl_step_h;
	UINT32 spl_step_v;
	UINT32 spl_num_x;
	UINT32 spl_num_y;
	UINT32 spl_st_x;
	UINT32 spl_st_y;
} KDRV_IME_3DNR_QUERY_STA_INFO;

typedef struct {
	UINT8 out_ms_roi_en;
	UINT8 out_ms_roi_flip_en;
	UINT8 out_sta_en;

	KDRV_IME_TMNR_REF_OUT_LOCATION_SEL ref_out_sel; //internal reference
	UINT8 ref_out_min_y; //unuse for kflow
	UINT8 ref_out_max_y; //unuse for kflow
	UINT8 ref_out_min_uv;//unuse for kflow
	UINT8 ref_out_max_uv;//unuse for kflow

	KDRV_IPP_FRAME in_ref_frm;
	UINT8 in_ref_flip_enable;
	UINT32 out_sta_max_spl_num;

	//single ipp not support **_addr_ofs
	KDRV_IME_BUF_ADDR in_mv_addr;
	UINT32            in_mv_addr_ofs;

	KDRV_IME_BUF_ADDR in_ms_addr;
	UINT32            in_ms_addr_ofs;

	KDRV_IME_BUF_ADDR in_fcvg_addr;
	UINT32            in_fcvg_addr_ofs;

	KDRV_IME_BUF_ADDR out_mv_addr;
	UINT32            out_mv_addr_ofs;

	KDRV_IME_BUF_ADDR out_ms_addr;
	UINT32            out_ms_addr_ofs;

	KDRV_IME_BUF_ADDR out_ms_roi_addr;
	UINT32            out_ms_roi_addr_ofs;

	KDRV_IME_BUF_ADDR out_sta_addr;
	UINT32            out_sta_addr_ofs;

	KDRV_IME_BUF_ADDR out_fcvg_addr;
	UINT32            out_fcvg_addr_ofs;
} KDRV_IME_3DNR_IOCFG;

typedef enum KDRV_IME_TMNR_ENC_FMT_SEL {
	KDRV_IME_TMNR_ENC_FMT_PGSV       = 0,    ///< progressive mode
	//KDRV_IME_TMNR_ENC_COMP_FMT_MB    = 1,    ///< YCC compression maco-block mode
	//KDRV_IME_TMNR_ENC_FMT_MB         = 2,    ///< non-YCC compression maco-block mode

} KDRV_IME_TMNR_ENC_FMT_SEL;

typedef struct {
	UINT8 en;
} KDRV_IME_NN_3DNR_IQCFG;

typedef struct {
	UINT8 en;
} KDRV_IME_NN_2DNR_IQCFG;

/*********************
Privacy Mask Structure
**********************/
typedef enum {
	KDRV_IME_PM_SET_IDX_0   = 0,
	KDRV_IME_PM_SET_IDX_1   = 1,
	KDRV_IME_PM_SET_IDX_2   = 2,
	KDRV_IME_PM_SET_IDX_3   = 3,
	KDRV_IME_PM_SET_IDX_4   = 4,
	KDRV_IME_PM_SET_IDX_5   = 5,
	KDRV_IME_PM_SET_IDX_6   = 6,
	KDRV_IME_PM_SET_IDX_7   = 7,
	KDRV_IME_PM_SET_IDX_MAX = 8,
} KDRV_IME_PM_SET_IDX;

typedef enum {
	KDRV_IME_PM_MASK_TYPE_YUV = 0,  ///< Using YUV color
	KDRV_IME_PM_MASK_TYPE_PXL = 1,  ///< Using pixilation
	KDRV_IME_PM_MASK_TYPE_UNKNOWN,
	ENUM_DUMMY4WORD(KDRV_IME_PM_MASK_TYPE)
} KDRV_IME_PM_MASK_TYPE;

typedef enum {
	KDRV_IME_PM_PIXELATION_08 = 0,  ///< 8x8
	KDRV_IME_PM_PIXELATION_16 = 1,  ///< 16x16
	KDRV_IME_PM_PIXELATION_32 = 2,  ///< 32x32
	KDRV_IME_PM_PIXELATION_64 = 3,  ///< 64x64
	KDRV_IME_PM_PIXELATION_UNKNOWN,
	ENUM_DUMMY4WORD(KDRV_IME_PM_PXL_BLK_SIZE)
} KDRV_IME_PM_PXL_BLK_SIZE;


typedef enum  {
	KDRV_IME_PM_PROC_LOCATION_PRE_3DNR = 0,      ///< pre-3DNR
	KDRV_IME_PM_PROC_LOCATION_PRE_SCROP = 1,     ///< scale pre-crop
	ENUM_DUMMY4WORD(KDRV_IME_PM_PROC_LOCATION)
} KDRV_IME_PM_PROC_LOCATION;

typedef enum {
	KDRV_IME_PM_CONVEX_POLYGON_HOLLOW    = 0,
	KDRV_IME_PM_CONVEX_POLYGON_4         = 1,
	KDRV_IME_PM_CONVEX_POLYGON_6         = 2,
	KDRV_IME_PM_CONVEX_POLYGON_8         = 3,
	KDRV_IME_PM_CONVEX_POLYGON_10        = 4,
	ENUM_DUMMY4WORD(KDRV_IME_PM_CONVEX_POLYGON_SEL)
} KDRV_IME_PM_CONVEX_POLYGON_SEL;

typedef struct {
	KDRV_IME_PM_SET_IDX         set_idx;                        ///< total 8 set, idx = 0~7
	UINT8                       enable;                         ///< privacy mask set enable
	KDRV_IME_PM_CONVEX_POLYGON_SEL	set_poly_sel;				///< privacy mask shape selection
	IPOINT                      coord[KDRV_IME_PM_COORD_TAB];   ///< privacy mask 4/6/8/10 coordinates(horizontal and vertical direction), must be Convex Hull and enter in a clockwise direction
	IPOINT                      coord2[KDRV_IME_PM_COORD_TAB2]; ///< privacy mask 4 coordinates(horizontal and vertical direction) for hollow, must be Convex Hull and enter in a clockwise direction

	KDRV_IME_PM_MASK_TYPE       msk_type;                       ///< privacy mask type selection
	UINT8                       color[3];                       ///< privacy mask using YUV color, range: [0, 255]
	UINT8                       weight;                         ///< privacy mask alpha weight, range: [0, 255]
} KDRV_IME_PM_INFO;

typedef enum  {
	KDRV_IME_PM_PIXELATION_DRAM = 0,     ///< from DRAM
	KDRV_IME_PM_PIXELATION_LCA  = 1,     ///< from LCA
	KDRV_IME_PM_PIXELATION_CURF = 2,     ///< from current frame
	ENUM_DUMMY4WORD(KDRV_IME_PM_PXL_SRC_SEL)
} KDRV_IME_PM_PXL_SRC_SEL;

typedef struct {
	KDRV_IME_PM_PXL_SRC_SEL     src_sel;         ///< pixelation source selection
	USIZE                       img_size;        ///< pixelation input image size of privacy mask
	KDRV_IME_PM_PXL_BLK_SIZE    blk_size;        ///< block size of pixelation
	UINT32                      lofs;            ///< pixelation input image lineoffset
	KDRV_IME_BUF_ADDR           dma_addr;        ///< pixelation input imaeg buffer address
	USIZE                       out_img_size;    ///< pixelation output image size of privacy mask
	UINT32                      out_lofs;        ///< pixelation output image lineoffset
	KDRV_IME_BUF_ADDR           out_dma_addr;    ///< pixelation output imaeg buffer address
	UINT8                       pxl_subout_en;   ///< pixelation output enable
	KDRV_IME_PM_PROC_LOCATION   pm_proc_location;///< privacy mask location select
} KDRV_IME_PM_PXL_IMG_INFO;

typedef struct {
	UINT8  enable;
	UINT32 cent_x;   ///< fish mask center x
	UINT32 cent_y;   ///< fish mask center y
	UINT32 valid_r;  ///< fish mask radius
	UINT8  decs_r;   ///< fish mask sharp range: [0, 255]
} KDRV_IME_PM_FISHEYE_INFO;

/*********************
OSD Structure
**********************/
#if(IME_538_KDRV_FEATURE == 0)
typedef enum {
	KDRV_IME_OSD_FMT_RGB565   = 0,  ///< OSD format, RGB565
	KDRV_IME_OSD_FMT_RGB1555  = 1,  ///< OSD format, RGB1555
	KDRV_IME_OSD_FMT_RGB4444  = 2,  ///< OSD format, RGB4444
	KDRV_IME_OSD_FMT_RGB8888  = 3,  ///< OSD format, RGB8888
	KDRV_IME_OSD_FMT_UNKNOWN,
	ENUM_DUMMY4WORD(KDRV_IME_OSD_FMT_SEL)
} KDRV_IME_OSD_FMT_SEL;

typedef enum {
	KDRV_IME_OSD_SET_IDX_0   = 0,
	KDRV_IME_OSD_SET_IDX_1   = 1,
	KDRV_IME_OSD_SET_IDX_2   = 2,
	KDRV_IME_OSD_SET_IDX_3   = 3,
	KDRV_IME_OSD_SET_IDX_MAX = 4,
} KDRV_IME_OSD_SET_IDX;

typedef enum {
	KDRV_IME_OSD_CKEY_RGB_MODE = 0, ///< using RGB mode
	KDRV_IME_OSD_CKEY_ARGB_MODE = 1, ///< using ARGB mode
	KDRV_IME_OSD_CKEY_MODE_UNKNOWN,
	ENUM_DUMMY4WORD(KDRV_IME_OSD_COLOR_KEY_MODE_SEL)
} KDRV_IME_OSD_COLOR_KEY_MODE_SEL;

typedef struct {
	USIZE                   size;   ///< Image size, maximum 8192x8192
	KDRV_IME_OSD_FMT_SEL    fmt;    ///< Image format selection
	UPOINT                  pos;    ///< Blending position
	UINT32                  lofs;   ///< Data lineoffset
	ULONG                   addr;   ///< Data address
} KDRV_IME_OSD_IMAGE_INFO;

typedef struct {
	UINT8                           color_key_en;   ///< color key enable
	KDRV_IME_OSD_COLOR_KEY_MODE_SEL color_key_mode;
	UINT8                           color_key_val[KDRV_IME_DS_COLOR_KEY_TAB];   ///< Color key, ,valid when color_key_en = TRUE
	///< color_key_val[0]: for A channel,
	///< color_key_val[1]: for R channel,
	///< color_key_val[2]: for G channel,
	///< color_key_val[3]: for B channel,
	UINT8 plt_en;         ///< palette mode enable
	UINT8 bld_wt_0;       ///< Blending weighting, if RGB565, range: [0, 16]; others, range: [0, 15]
	UINT8 bld_wt_1;       ///< Blending weighting, range: [0, 15]
} KDRV_IME_OSD_IQ_INFO;

typedef struct {
	UINT8                       enable;         ///< Function enable
	KDRV_IME_OSD_IMAGE_INFO     image;          ///< Image information
	KDRV_IME_OSD_IQ_INFO        ds_iq;      ///< OSD quality information
} KDRV_IME_OSD_INFO;

typedef struct {
	UINT8  enable;             ///< color space trans enable
	UINT8  cst_auto_param_en;  ///< parameter auto mode enable
	UINT8  cst_coef0;          ///< Coefficient of color space transform
	UINT8  cst_coef1;          ///< Coefficient of color space transform
	UINT8  cst_coef2;          ///< Coefficient of color space transform
	UINT8  cst_coef3;          ///< Coefficient of color space transform
} KDRV_IME_OSD_CST_INFO;

typedef enum {
	KDRV_IME_OSD_PLT_1BIT_MODE = 0, ///< 1bit mode
	KDRV_IME_OSD_PLT_2BIT_MODE = 1, ///< 2bit mode
	KDRV_IME_OSD_PLT_4BIT_MODE = 2, ///< 4bit mode
	KDRV_IME_OSD_PLT_MODE_UNKNOWN,
	ENUM_DUMMY4WORD(KDRV_IME_OSD_PLT_MODE_SEL)
} KDRV_IME_OSD_PLT_MODE_SEL;

typedef struct _KDRV_IME_OSD_PLT_INFO {
	KDRV_IME_OSD_PLT_MODE_SEL   plt_mode;   ///< palette mode selection
	UINT8   plt_a[KDRV_IME_DS_PLT_TAB];     ///< color palette of A channel, range: [0, 255]
	UINT8   plt_r[KDRV_IME_DS_PLT_TAB];     ///< color palette of R channel, range: [0, 255]
	UINT8   plt_g[KDRV_IME_DS_PLT_TAB];     ///< color palette of G channel, range: [0, 255]
	UINT8   plt_b[KDRV_IME_DS_PLT_TAB];     ///< color palette of B channel, range: [0, 255]
} KDRV_IME_OSD_PLT_INFO;
#endif

/*********************
single out info, get-only
**********************/
typedef struct {
	UINT32 single_out_ch;
	UINT32 single_out_en;
} KDRV_IME_SINGLE_OUT_INFO;

/*********************
low delay path select
**********************/
typedef struct {
	UINT8 en;
	KDRV_IME_PATH_ID pid;
} KDRV_IME_LOW_DELAY_INFO;

/*********************
bp config
**********************/
typedef struct {
	UINT32 bp1;     ///< 0 for disable
	UINT32 bp2;
	UINT32 bp3;
} KDRV_IME_BP;

/*********************
VA result
**********************/
//538 removed
#if(IME_538_KDRV_FEATURE == 0)
typedef struct KDRV_IME_INDEP_VA_PARAM_ {
	BOOL     indep_va_en;
	//BOOL     linemax_g1_en;
	//BOOL     linemax_g2_en;
} KDRV_IME_INDEP_VA_PARAM;

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
} KDRV_IME_VA_RST_SETTING_DUAL;

typedef struct {
	UINT32* g1_h;
	UINT32* g1_v;
	UINT32* g2_h;
	UINT32* g2_v;
	UINT32* g1_h_cnt;
	UINT32* g1_v_cnt;
	UINT32* g2_h_cnt;
	UINT32* g2_v_cnt;
	KDRV_IME_VA_RST_SETTING_DUAL va_setting;
} KDRV_IME_VA_RST_DUAL;

typedef struct {
	BOOL va_en;
	UINT32 win_num_x;
	UINT32 win_num_y;
	BOOL outsel;
	KDRV_IME_BUF_ADDR address;
	UINT32 lineoffset;
} KDRV_IME_VA_RST_SETTING;

typedef struct {
	UINT32 g1_h[KDRV_IME_VA_MAX_WINNUM];
	UINT32 g1_v[KDRV_IME_VA_MAX_WINNUM];
	UINT32 g2_h[KDRV_IME_VA_MAX_WINNUM];
	UINT32 g2_v[KDRV_IME_VA_MAX_WINNUM];
	UINT32 g1_h_cnt[KDRV_IME_VA_MAX_WINNUM];
	UINT32 g1_v_cnt[KDRV_IME_VA_MAX_WINNUM];
	UINT32 g2_h_cnt[KDRV_IME_VA_MAX_WINNUM];
	UINT32 g2_v_cnt[KDRV_IME_VA_MAX_WINNUM];
	KDRV_IME_VA_RST_SETTING va_setting;

#if(IPE_538_KLOFW_SUPPORT == 0)
	UINT32 *p_g1_h;
	UINT32 *p_g1_v;
	UINT32 *p_g2_h;
	UINT32 *p_g2_v;
	UINT32 *p_g1_h_cnt;
	UINT32 *p_g1_v_cnt;
	UINT32 *p_g2_h_cnt;
	UINT32 *p_g2_v_cnt;
#endif
} KDRV_IME_VA_RESULT;

typedef struct {
	UINT32 g1_h[KDRV_IME_VA_INDEP_NUM];
	UINT32 g1_v[KDRV_IME_VA_INDEP_NUM];
	UINT32 g2_h[KDRV_IME_VA_INDEP_NUM];
	UINT32 g2_v[KDRV_IME_VA_INDEP_NUM];
	UINT32 g1_h_cnt[KDRV_IME_VA_INDEP_NUM];
	UINT32 g1_v_cnt[KDRV_IME_VA_INDEP_NUM];
	UINT32 g2_h_cnt[KDRV_IME_VA_INDEP_NUM];
	UINT32 g2_v_cnt[KDRV_IME_VA_INDEP_NUM];

#if(IPE_538_KLOFW_SUPPORT == 0)
	UINT32 *p_g1_h;
	UINT32 *p_g1_v;
	UINT32 *p_g2_h;
	UINT32 *p_g2_v;
	UINT32 *p_g1_h_cnt;
	UINT32 *p_g1_v_cnt;
	UINT32 *p_g2_h_cnt;
	UINT32 *p_g2_v_cnt;
#endif
} KDRV_IME_VA_INDEP_RESULT;

typedef struct{
	KDRV_IME_BUF_ADDR va_out_addr;
	UINT32 va_out_addr_ofs;
}KDRV_IME_VA_IOCFG;
#endif

typedef enum {
	KDRV_IME_LEFT   = 0,
	KDRV_IME_RIGHT  = 1,
	KDRV_IME_UNKNOWN = 2,
}KDRV_IME_DUAL_IDX;

//538 not support
typedef struct{
	BOOL input_scl_en;
	USIZE input_scl_size;
}KDRV_IME_INPUT_SCL;

typedef enum _KDRV_IME_NN_ISP_PATH_ {
	KDRV_IME_NN_ISP_PATH2 = 0,
	KDRV_IME_NN_ISP_PATH3 = 1,
	ENUM_DUMMY4WORD(KDRV_IME_NN_ISP_PATH_SEL)
} KDRV_IME_NN_ISP_PATH_SEL;

typedef struct{
	KDRV_IME_BUF_ADDR in_mot_addr;
	KDRV_IME_BUF_ADDR out_mot_addr;
	UINT32 mot_lofs;

	KDRV_IME_BUF_ADDR txt_addr;
	UINT32 txt_lofs;
}KDRV_IME_NN_2DNR;


typedef struct{
	BOOL update;

	BOOL path_en;
	BOOL fw_handshake_en;
	UINT32 proc_id;

	KDRV_IME_BUF_ADDR ringbuf_y_addr;
	KDRV_IME_BUF_ADDR ringbuf_uv_addr;
	KDRV_IME_BUF_ADDR outbuf_y_addr;
	KDRV_IME_BUF_ADDR outbuf_uv_addr;
	UINT32 yuv_lofs;

	UINT32 slice_height;
	UINT32 slice_ovlp;

	UINT32 outbuf_height;
	UINT32 ringbuf_height;

	UINT32 nn_max_strp;

}KDRV_IME_NN_ISP_PATH;

typedef struct{
	BOOL glb_nn_isp_en;
	KDRV_IME_NN_ISP_PATH nn_isp_p2;
	KDRV_IME_NN_ISP_PATH nn_isp_p3;
	KDRV_IME_NN_2DNR nn_2dnr;
}KDRV_IME_NN_ISP_PARAM;

typedef struct {
	USIZE in_size;
	USIZE out_path_scl_size;
	KDRV_IME_SCL_METHOD_SEL out_scl_method_sel;
	KDRV_IME_PATH_ID id;
	KDRV_IME_SCL_METHOD result;
} KDRV_IME_GET_SCL_METHOD_INFO;

#if(IME_538_KDRV_FEATURE == 0)
typedef struct KDRV_IME_VA_LDG_PARAM {
	UINT8 low_slope;       ///< Dark region gain slope
	UINT8 high_slope;      ///< Bright region gain slope
	UINT8 low_th;          ///< Darkness threshold
	UINT8 high_th;         ///< Brightness threshold
	UINT8 low_gain;        ///< Dark region minima gain
	UINT8 high_gain;       ///< Bright region minima gain
} KDRV_IME_VA_LDG_PARAM;

typedef enum KDRV_IME_VA_FLTR_SYMM_SEL {
	KDRV_IME_VA_FLTR_MIRROR  = 0,   ///<
	KDRV_IME_VA_FLTR_INVERSE = 1,   ///<
} KDRV_IME_VA_FLTR_SYMM_SEL;

typedef enum KDRV_IME_VA_FLTR_SIZE_SEL {
	KDRV_IME_VA_FLTR_SIZE_1  = 0,   ///<
	KDRV_IME_VA_FLTR_SIZE_3  = 1,   ///<
	KDRV_IME_VA_FLTR_SIZE_5  = 2,   ///<
	KDRV_IME_VA_FLTR_SIZE_7  = 3,   ///<
} KDRV_IME_VA_FLTR_SIZE_SEL;

typedef struct KDRV_IME_VA_FLTR_PARAM {
	UINT8                   tap_a;     ///<
	INT8                    tap_b;      ///<
	INT8                    tap_c;      ///<
	INT8                    tap_d;      ///<
	KDRV_IME_VA_FLTR_SYMM_SEL    filt_symm;   ///<
	KDRV_IME_VA_FLTR_SIZE_SEL    fltr_size;   ///<
	UINT8                   div;      ///<
	UINT16                  th_low;      ///<
	UINT16                  th_high;      ///<
} KDRV_IME_VA_FLTR_PARAM;

typedef struct KDRV_IME_VA_FLTR_GROUP_PARAM {
	KDRV_IME_VA_FLTR_PARAM       filt_h;
	KDRV_IME_VA_FLTR_PARAM       filt_v;
	//BOOL                  linemax_en;   ///<
	BOOL                    cnt_en;       ///<
	//UINT8                   energy_wet;     ///< Edge strength weight
} KDRV_IME_VA_FLTR_GROUP_PARAM;

typedef struct {
	UINT32 ratio_base;
	USIZE  winsz_ratio;                                  ///< va window size of each window, max 511x511
	URECT  indep_roi_ratio[KDRV_IME_INDEP_VA_WIN_NUM];   ///< va indep-window size, max 511x511
} KDRV_IME_VA_WIN_INFO;

typedef enum {
	KDRV_IME_VA_ENERGY_COUNT     = 0,   ///<
	KDRV_IME_VA_HIGH_LUMA_COUNT  = 1,   ///<
}KDRV_IME_VA_CNT_OUTSEL;

typedef struct{

	BOOL va_en;
	BOOL va_indep_va_en;
	BOOL ldg_en;

	KDRV_IME_VA_FLTR_GROUP_PARAM fltr_g1;
	KDRV_IME_VA_FLTR_GROUP_PARAM fltr_g2;

	UINT8 va_out_grp1_2;
	UINT32 va_lofs;                         ///< va output lineoffset
	USIZE win_num;                          ///< va window number, 1x1 ~ 16x16

	KDRV_IME_INDEP_VA_PARAM  va_indep_param[KDRV_IME_INDEP_VA_WIN_NUM];

	KDRV_IME_VA_LDG_PARAM ldg_param;
	KDRV_IME_VA_CNT_OUTSEL win_cnt_out_sel;
	UINT8 high_luma_th;
	UINT8 energy_w;

}KDRV_IME_VA_IQCFG;
#endif

#if(IME_538_KDRV_FEATURE == 0)
typedef enum{
	KDRV_IME_MRNR_DBG_DISABLE      = 0,
	KDRV_IME_MRNR_DBG_L1_Y_EDG_MAP = 1,
	KDRV_IME_MRNR_DBG_L2_Y_EDG_MAP = 2,
	KDRV_IME_MRNR_DBG_L2_C_EDG_MAP = 3,
	KDRV_IME_MRNR_DBG_UNKNOWN      = 4
}KDRV_IME_MRNR_DBG_SEL;

typedef enum{
	KDRV_IME_MRNR_DBG_TH_E1      = 0,
	KDRV_IME_MRNR_DBG_TH_E2      = 1,
	KDRV_IME_MRNR_DBG_TH_UNKNOWN = 2
}KDRV_IME_MRNR_DBG_TH;

typedef struct{
	UINT16 y_edge_detct_l1[KDRV_IME_MRNR_EDGE_DETECT_L1_LEN];
	UINT16 y_edge_detct_l2[KDRV_IME_MRNR_EDGE_DETECT_L2_LEN];
	UINT16 cb_edge_detct_l2;
	UINT16 cr_edge_detct_l2;
}KDRV_IME_MRNR_EDGE_DETECT_PARAM;

typedef struct{
	UINT8 y_edge_smooth_l1[KDRV_IME_MRNR_EDGE_SMOOTH_L1_LEN];
	UINT8 y_edge_smooth_l2[KDRV_IME_MRNR_EDGE_SMOOTH_L2_LEN];
    UINT8 cb_edge_smooth;
    UINT8 cr_edge_smooth;
}KDRV_IME_MRNR_EDGE_SMOOTH_PARAM;

typedef struct{
    UINT8 nr_str_y_0;
    UINT8 nr_str_y_1;
    UINT8 nr_str_c_1;
    UINT8 nr_str_mot;
    UINT8 nr_str_trans;
    UINT8 nr_str_still;
}KDRV_IME_MRNR_NR_STRENGTH_PARAM;

typedef struct{
    UINT8 txtr_str_th1;
    UINT8 txtr_str_th2;
    UINT8 txtr_str_min;
    UINT8 txtr_str_max;
    UINT16 txtr_str_slop;
}KDRV_IME_MRNR_TEXTURE_STRENGTH_PARAM;

typedef struct{
    KDRV_IME_MRNR_DBG_SEL mrnr_dbg_sel;
    KDRV_IME_MRNR_DBG_TH mrnr_dbg_th_y_th1;
    KDRV_IME_MRNR_DBG_TH mrnr_dbg_th_y_th2;
    KDRV_IME_MRNR_DBG_TH mrnr_dbg_th_uv_th;
}KDRV_IME_MRNR_DBG_PARAM;

typedef struct{
	BOOL mrnr_en;
	BOOL mot_map_en;
	BOOL txtr_map_en;

	KDRV_IME_MRNR_EDGE_DETECT_PARAM edge_detct;
    KDRV_IME_MRNR_EDGE_SMOOTH_PARAM edge_smooth;
    KDRV_IME_MRNR_NR_STRENGTH_PARAM nr_str;
    KDRV_IME_MRNR_TEXTURE_STRENGTH_PARAM txtr_str;
    KDRV_IME_MRNR_DBG_PARAM dbg;

}KDRV_IME_MRNR_IQCFG;
#endif

/*********************
IO/IQ Config
**********************/
typedef struct {
	KDRV_IPP_OPMODE mode;
	KDRV_IPP_OP_STATUS ipp_status; // [0]: ipp is start [1]: ipp is stop
	KDRV_IPP_FRAME in_frm;
	UINT32 inte_en;

	KDRV_IME_OUT_PATH out_path[KDRV_IME_PATH_NUM_MAX];
	KDRV_IME_SCL_METHOD_SEL out_scl_method_sel;
	USIZE post_in_scl_size; //internal used

	KDRV_IME_INPUT_SCL input_scl_param; //538 not support

	KDRV_IME_PATH_ID ref_path;

	KDRV_IME_LOW_DELAY_INFO low_delay;
	KDRV_IME_BP bp;

	KDRV_IME_LCA_IOCFG lca;
	KDRV_IME_3DNR_IOCFG _3dnr;

	KDRV_IME_PM_PXL_IMG_INFO pm_pxlimg;
	KDRV_IME_PM_INFO pm_mask[KDRV_IME_PM_SET_IDX_MAX];
	KDRV_IME_PM_FISHEYE_INFO pm_fisheye;

	//KDRV_IME_OSD_INFO ds[KDRV_IME_OSD_SET_IDX_MAX];  //530 removed
	//KDRV_IME_OSD_CST_INFO ds_cst;  //530 removed
	//KDRV_IME_OSD_PLT_INFO ds_plt;  //530 removed

#if(IME_538_KDRV_FEATURE == 0)
	KDRV_IME_VA_IOCFG va;
#endif

	/* for kdrv_ipp config */
	UINT16 strp_max_width;  ///< ime d2d mode will calculate in ime_eng, other mode decide by dce
	UINT16 strp_ovlp;
	UINT16 strp_prt;

	/* drame chk status */
	BOOL dram_sts_set_en;
	UINT8 dram_sts_chk_bit;

    UINT32 ime_strp[KDRV_IME_STRP_NUM_MAX];
    UINT8  ime_strp_num;
    UINT32 ime_ovlp;
	KDRV_IME_STRP_RST ime_qry_rst; // internal reference

    UINT32 va_win_stx;    // internal reference
    UINT32 va_win_size_x; // internal reference
    UINT32 va_win_skip_x; // internal reference

    BOOL dual_en;
    KDRV_IME_DUAL_IDX dual_idx;
    UINT16 dual_start_ofs;
    UINT32 dual_in_left_width;
    UINT32 dual_in_right_width;
    UINT16 dual_in_left_ovlp;
    UINT16 dual_in_right_ovlp;

    UINT32 dual_out_left_width[KDRV_IME_PATH_NUM_MAX];
    UINT32 dual_out_right_width[KDRV_IME_PATH_NUM_MAX];
    UINT16 dual_out_left_ovlp[KDRV_IME_PATH_NUM_MAX];
    UINT16 dual_out_right_ovlp[KDRV_IME_PATH_NUM_MAX];

    UINT32 dual_in_width_total;
    UINT32 dual_scl_width_total[KDRV_IME_PATH_NUM_MAX];

    KDRV_IME_NN_ISP_PARAM nn_isp_param;
    BOOL nn_isp_drop_frm_flg;

} KDRV_IME_IO_CFG;

typedef struct {
	KDRV_IME_IO_CFG *p_iocfg;
	KDRV_IME_IQ_CFG *p_iqcfg;
	void *p_ll_blk;
} KDRV_IME_JOB_CFG;


/*********************
kdrvier api
**********************/
INT32 kdrv_ime_module_init(void);
INT32 kdrv_ime_module_uninit(void);

INT32 kdrv_ime_open(UINT32 chip, UINT32 engine);
INT32 kdrv_ime_close(UINT32 chip, UINT32 engine);
INT32 kdrv_ime_query(UINT32 id, KDRV_IME_QUERY_ID qid, void *p_param);
INT32 kdrv_ime_set(UINT32 id, KDRV_IME_PARAM_ID param_id, void *p_param);
INT32 kdrv_ime_get(UINT32 id, KDRV_IME_PARAM_ID param_id, void *p_param);

INT32 ime_builtin_rtos_init(void);
void ime_builtin_rtos_uninit(void);

void kdrv_ime_dump(void);
void kdrv_ime_dump_cfg(KDRV_IME_JOB_CFG *p_cfg, int (*KDRV_IME_DUMP)(const char *fmt, ...));
void kdrv_ime_dump_register(void);
void kdrv_ime_dump_register_log(void);
INT32 kdrv_ime_dbg_mode(void *p_hdl, UINT32 param_id, void *data);

#endif
