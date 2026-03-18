/**
    Public header file for dal_ipe

    This file is the header file that define the API and data type for dal_ipe.

    @file       kdrv_ipe.h
    @ingroup    mILibIPLCom
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _KDRV_IPE_H_
#define _KDRV_IPE_H_

#include "kdrv_videoprocess/kdrv_ipp_utility.h"
#include "kdrv_builtin/kdrv_ipp_builtin.h"

#define IPE_SSDRV_SUPPORT (1)
#define IPE_538_KLOFW_SUPPORT (0)
#define IPE_538_KDRV_FEATURE (1)

typedef enum {
	KDRV_IPE_PARAM_CFG_PROCESS_CPU = 0,
	KDRV_IPE_PARAM_CFG_PROCESS_LL,
	KDRV_IPE_PARAM_TRIG_SINGLE,
	KDRV_IPE_PARAM_TRIG_LL,
	KDRV_IPE_PARAM_TRIG_DIR,
	KDRV_IPE_PARAM_SET_CB,
	KDRV_IPE_PARAM_STOP_SINGLE,
	KDRV_IPE_PARAM_LOAD_GAMMA,
	KDRV_IPE_PARAM_GAMMA_FT_TRANS,
	KDRV_IPE_PARAM_YCURVE_FT_TRANS,
	KDRV_IPE_PARAM_3DCC_FT_TRANS,		/*538 add*/
	KDRV_IPE_PARAM_HARD_RESET,
	KDRV_IPE_PARAM_DEFOG_STCS_RESULT,
	KDRV_IPE_PARAM_EDGE_STCS_RESULT,
	KDRV_IPE_PARAM_VA_RESULT,
	KDRV_IPE_PARAM_VA_RESULT_DUAL,		/*538 not support*/
	KDRV_IPE_PARAM_INDE_VA_RESULT,
	KDRV_IPE_PARAM_DMA_CH_ABORT,
	KDRV_IPE_PARAM_DMA_CH_STS,
	KDRV_IPE_PARAM_GET_REG_BASE_ADDR,
	KDRV_IPE_PARAM_GET_REG_BASE_ADDR_LL,
	KDRV_IPE_PARAM_GET_DBG_INFO,
	KDRV_IPE_PARAM_GET_HW_AIRLIGHT,
	KDRV_IPE_PARAM_GET_MERGED_AIRLIGHT,
	KDRV_IPE_PARAM_GET_MERGED_INDP_VA,
	KDRV_IPE_PARAM_GET_MERGED_EDGE_STSC,
	KDRV_IPE_PARAM_ID_MAX,
} KDRV_IPE_PARAM_ID;

typedef enum {
	KDRV_IPE_INTERRUPT_FMD              = 0x00000002,
	KDRV_IPE_INTERRUPT_STP              = 0x00000004,
	KDRV_IPE_INTERRUPT_FMS              = 0x00000008,
	KDRV_IPE_INTERRUPT_YCC_OUT_END      = 0x00000010,
	KDRV_IPE_INTERRUPT_GAMMA_IN_END     = 0x00000020,
	KDRV_IPE_INTERRUPT_DEFOG_SUBOUT_END = 0x00000040,
    //KDRV_IPE_INTERRUPT_LCE_SUBOUT_END   = 0x00000080,
	KDRV_IPE_INTERRUPTT_VA_OUT_END      = 0x00000100,
	KDRV_IPE_INTERRUPT_LL_DONE          = 0x00000200,
	KDRV_IPE_INTERRUPT_LL_JOBEND        = 0x00000400,
	KDRV_IPE_INTERRUPT_CFA_SUBOUT_END   = 0x00000800,
	KDRV_IPE_INTERRUPT_LL_ERR           = 0x00001000,
	KDRV_IPE_INTERRUPT_LL_ERR2          = 0x00002000,
	KDRV_IPE_INTERRUPT_FRAMEERR         = 0x00004000,
	KDRV_IPE_INTE_DMAIN1END             = 0x00008000,
	KDRV_IPE_INTE_ENC_OUTOVFL			= 0x00010000,
	KDRV_IPE_INTERRUPT_ALL              = (KDRV_IPE_INTERRUPT_FMD | KDRV_IPE_INTERRUPT_STP | KDRV_IPE_INTERRUPT_FMS | KDRV_IPE_INTERRUPT_YCC_OUT_END | KDRV_IPE_INTERRUPT_GAMMA_IN_END |
											KDRV_IPE_INTERRUPT_DEFOG_SUBOUT_END | KDRV_IPE_INTERRUPTT_VA_OUT_END | KDRV_IPE_INTERRUPT_LL_DONE | KDRV_IPE_INTERRUPT_LL_JOBEND | KDRV_IPE_INTERRUPT_LL_ERR |
											KDRV_IPE_INTERRUPT_LL_ERR2 | KDRV_IPE_INTERRUPT_FRAMEERR | KDRV_IPE_INTERRUPT_CFA_SUBOUT_END | KDRV_IPE_INTE_DMAIN1END | KDRV_IPE_INTE_ENC_OUTOVFL)
} KDRV_IPE_INTERRUPT;

typedef enum {
	KDRV_IPE_UPDATE_EEXT	 		= 0x00000001,
	KDRV_IPE_UPDATE_EEXT_TONEMAP 	= 0x00000002,
	KDRV_IPE_UPDATE_OVERSHOOT	 	= 0x00000004,
	KDRV_IPE_UPDATE_EPROC		 	= 0x00000008,
	KDRV_IPE_UPDATE_CC			 	= 0x00000040,
	KDRV_IPE_UPDATE_CCM			 	= 0x00000080,
	KDRV_IPE_UPDATE_CCTRL		 	= 0x00000100,
	//KDRV_IPE_UPDATE_CCTRL_CT	 	= 0x00000200,
	KDRV_IPE_UPDATE_CADJ_EE		 	= 0x00000400,
	KDRV_IPE_UPDATE_CADJ_YCCON	 	= 0x00000800,
	KDRV_IPE_UPDATE_CADJ_COFS		= 0x00001000,
	KDRV_IPE_UPDATE_CADJ_RAND		= 0x00002000,
	KDRV_IPE_UPDATE_CADJ_HUE		= 0x00004000,
	KDRV_IPE_UPDATE_CADJ_FIXTH		= 0x00008000,
	KDRV_IPE_UPDATE_CADJ_MASK		= 0x00010000,
	KDRV_IPE_UPDATE_GAMYRAND		= 0x00020000,
	KDRV_IPE_UPDATE_GAMMA			= 0x00040000,
	KDRV_IPE_UPDATE_YCURVE			= 0x00080000,
	KDRV_IPE_UPDATE_CST				= 0x00100000,
	KDRV_IPE_UPDATE_CSTP			= 0x00200000,
	KDRV_IPE_UPDATE_LCE				= 0x00400000,
	//KDRV_IPE_UPDATE_LCE_SUBIMG	= 0x01000000,
	KDRV_IPE_UPDATE_SUBIMG			= 0x01000000,
	KDRV_IPE_UPDATE_EDGEDBG			= 0x02000000,
	KDRV_IPE_UPDATE_VA				= 0x04000000,
	KDRV_IPE_UPDATE_VA_WIN			= 0x08000000,
	KDRV_IPE_UPDATE_EDGE_REGION_STR	= 0x10000000,
	KDRV_IPE_UPDATE_RGBLPF			= 0x20000000,
	KDRV_IPE_UPDATE_PFR				= 0x40000000,
	KDRV_IPE_UPDATE_DEFOG			= 0x80000000,
	KDRV_IPE_UPDATE_ALL				= (KDRV_IPE_UPDATE_EEXT | KDRV_IPE_UPDATE_EEXT_TONEMAP | KDRV_IPE_UPDATE_OVERSHOOT | KDRV_IPE_UPDATE_EPROC |
										KDRV_IPE_UPDATE_CC | KDRV_IPE_UPDATE_CCM | KDRV_IPE_UPDATE_CCTRL |
										KDRV_IPE_UPDATE_CADJ_EE | KDRV_IPE_UPDATE_CADJ_YCCON | KDRV_IPE_UPDATE_CADJ_COFS | KDRV_IPE_UPDATE_CADJ_RAND	 |
										KDRV_IPE_UPDATE_CADJ_HUE | KDRV_IPE_UPDATE_CADJ_FIXTH | KDRV_IPE_UPDATE_CADJ_MASK | KDRV_IPE_UPDATE_GAMYRAND | KDRV_IPE_UPDATE_GAMMA |
										KDRV_IPE_UPDATE_YCURVE | KDRV_IPE_UPDATE_CST | KDRV_IPE_UPDATE_CSTP | KDRV_IPE_UPDATE_LCE | KDRV_IPE_UPDATE_SUBIMG |
										KDRV_IPE_UPDATE_EDGEDBG | KDRV_IPE_UPDATE_VA | KDRV_IPE_UPDATE_VA_WIN | KDRV_IPE_UPDATE_EDGE_REGION_STR | KDRV_IPE_UPDATE_RGBLPF | KDRV_IPE_UPDATE_PFR |
										KDRV_IPE_UPDATE_DEFOG),
} KDRV_IPE_UPDATE;

typedef enum {
	//KDRV_IPE_UPDATE2_DEFOG_SUBIMG   = 0x00000001,
	KDRV_IPE_UPDATE2_CFA            = 0x00000001,
	KDRV_IPE_UPDATE2_IR_PARAM       = 0x00000002,
	KDRV_IPE_UPDATE2_COLOR_GAIN     = 0x00000004,
	KDRV_IPE_UPDATE2_PINK_REDUC     = 0x00000008,
	KDRV_IPE_UPDATE2_3DCC     		= 0x00000010,
	KDRV_IPE_UPDATE2_ALL			= (KDRV_IPE_UPDATE2_CFA | KDRV_IPE_UPDATE2_IR_PARAM | KDRV_IPE_UPDATE2_COLOR_GAIN | KDRV_IPE_UPDATE2_PINK_REDUC | KDRV_IPE_UPDATE2_3DCC),
} KDRV_IPE_UPDATE2;


/*******************************************************************************
*	Following #if 1 ... #endif block include all IPP IQ parameter.             *
*	These content must sync with kflow IQ parameter. (see ctl_ipe_isp.h)       *
*                                                                              *
*   How to modify the following IQ parameter:                                  *
*   1. main structure                                                          *
*      (i)   add item in KDRV_IPE_IQ_CFG                                       *
*               KDRV_IPE_IQ_XXX xxx;                                           *
*      (ii)  add item in CTL_IPE_ISP_IQ_ALL                                    *
*               CTL_IPE_ISP_XXX *p_xxx;                                        *
*      (iii) add memcpy in ctl_ipp_isp_set_ipe_iq()                            *
*               if (p_iq_ipe->p_xxx != NULL) {                                 *
*					memcpy((void *)&p_kdrv_iq_ipe->xxx, (void *)p_iq_ipe->p_xxx, sizeof(p_kdrv_iq_ipe->xxx)); *
*					p_kdrv_iq_xxx->update |= KDRV_IPE_UPDATE_XXX;              *
*				}                                                              *
*      (iv)  add static assert before (iii) to check structure size            *
*               STATIC_ASSERT(sizeof(CTL_IPE_ISP_XXX) == sizeof(KDRV_IPE_IQ_XXX)); *
*               if (p_iq_ipe->p_xxx != NULL) { ...                             *
*                                                                              *
*   2. sub structure                                                           *
*      (i)   modify content in #if 1 ... #endif of kdrv_ipe.h                  *
*      (ii)  modify content in #if 1 ... #endif of ctl_ipe_isp.h               *
*      (iii) compare #if 1 ... #endif are the same                             *
*               copy #if 1 ... #endif of ctl_ipe_isp.h to compare tool left    *
*               copy #if 1 ... #endif of kdrv_ipe.h to compare tool right      *
*               replace 'KDRV_IPE_IQ' to 'CTL_IPE_ISP'                         *
*               do compare! (the sub structure part should be exactly the same)*
*******************************************************************************/
#if 1
#define KDRV_IPE_IQ_EDGE_KER_DIV_LEN		12
#define KDRV_IPE_IQ_TONE_MAP_LUT_LEN		65
#define KDRV_IPE_IQ_EDGE_MAP_LUT_LEN		16
#define KDRV_IPE_IQ_ES_MAP_LUT_LEN			16
#define KDRV_IPE_IQ_FTAB_LEN				16  // For fstab/fdtab
#define KDRV_IPE_IQ_COEF_LEN				9  // For CCParam.coef/CSTCoef
#define KDRV_IPE_IQ_CCTRL_TAB_LEN			24  // For Hue/Sat/Int/Edg
#define KDRV_IPE_IQ_DDS_TAB_LEN				8
#define KDRV_IPE_IQ_CCONTAB_LEN				17
#define KDRV_IPE_IQ_PFR_COLOR_LEN			5
#define KDRV_IPE_IQ_PFR_LUMA_LEN			13
#define KDRV_IPE_IQ_PFR_SET_NUM				4
#define KDRV_IPE_IQ_GAMMA_LEN				129
#define KDRV_IPE_IQ_GAMMA_TRANS_LEN			208
#define KDRV_IPE_IQ_YCURVE_LEN				129
#define KDRV_IPE_IQ_YCURVE_TRANS_LEN		80
#define KDRV_IPE_IQ_DFG_INTERP_DIFF_LEN		17
#define KDRV_IPE_IQ_DFG_INPUT_BLD_LEN		9
#define KDRV_IPE_IQ_DFG_AIRLIGHT_NUM		3
#define KDRV_IPE_IQ_DFG_FOG_MOD_LEN			17
#define KDRV_IPE_IQ_DFG_TARGET_LEN			9
#define KDRV_IPE_IQ_DFG_OUTPUT_BLD_LEN		17
#define KDRV_IPE_IQ_LCE_INPUT_BLD_LEN		9
#define KDRV_IPE_IQ_LCE_INTERP_DIFF_LEN		17
#define KDRV_IPE_IQ_LCE_LUMA_LEN			9
#define KDRV_IPE_IQ_SUBIMG_FILT_LEN			3
#define KDRV_IPE_IQ_VA_INDEP_NUM			5
#define KDRV_IPE_IQ_CFA_FREQ_NUM			16
#define KDRV_IPE_IQ_CFA_LUMA_NUM			17
#define KDRV_IPE_IQ_CFA_FCS_NUM				16
#define KDRV_IPE_IQ_3DCC_LEN				729
#define KDRV_IPE_IQ_3DCC_TRANS_LEN			912

/************************
    IPE structure - IPE edge extraction.
************************/
typedef struct {
	UINT8 eext_enh;                ///< Legal range : 0~63, enhance term for kernel
	INT8 eext_div;                 ///< Legal range : -4~3, normalize term for kernel
} KDRV_IPE_IQ_EEXT_KER_STR;

typedef struct {
	KDRV_IPE_IQ_EEXT_KER_STR ker_freq0;    ///< strength of kernal A
	KDRV_IPE_IQ_EEXT_KER_STR ker_freq1;    ///< strength of kernal C
	KDRV_IPE_IQ_EEXT_KER_STR ker_freq2;    ///< strength of kernal D
} KDRV_IPE_IQ_EEXT_KER_STRENGTH;

typedef struct {
	INT8 eext_div_con;             ///< Legal range : -8~7, normalize term for edge contrast
	INT8 eext_div_eng;             ///< Legal range : -8~7, normalize term for edge engergy
	UINT8 wt_con_eng;              ///< Legal range : 0~8, blending weight between edge engergy and edge contrast, 0: all contrast, 8: all edge energy
} KDRV_IPE_IQ_EEXT_ENG_CON;

typedef struct {
	UINT8 wt_ker_thin;             ///< Legal range : 0~8, Thin kernel weighting between A & B, 8: all thinner kernel A
	UINT8 wt_ker_robust;           ///< Legal range : 0~8, robust kernel weighting between B & C, 8: all thinner kernel B
	UINT8 iso_ker_thin;            ///< Legal range : 0~8, Thin kernel weighting between A_B & C, 8: all thinner kernel A_B
	UINT8 iso_ker_robust;          ///< Legal range : 0~8, robust kernel weighting between B_C & D, 8: all thinner kernel B_C
} KDRV_IPE_IQ_KER_THICKNESS;

typedef struct {
	UINT16 th_flat;                 ///< Legal range : 0~1023, the threshold for flat region
	UINT16 th_edge;                 ///< Legal range : 0~1023, the threshold for edge region
	UINT16 th_flat_hld;             ///< Legal range : 0~1023, the threshold for flat region in HLD mode
	UINT16 th_edge_hld;             ///< Legal range : 0~1023, the threshold for edge region in HLD mode
	UINT16 th_lum_hld;              ///< Legal range : 0~1023, the luma threshold in HLD mode
} KDRV_IPE_IQ_REGION_THRESH;

typedef struct {
	UINT8 wt_low;                  ///< Legal range : 0~16, the kernel weighting of flat region
	UINT8 wt_high;                 ///< Legal range : 0~16, the kernel weighting of edge region
	UINT8 wt_low_hld;              ///< Legal range : 0~16, the kernel weighting of flat region in HLD mode
	UINT8 wt_high_hld;             ///< Legal range : 0~16, the kernel weighting of edge region in HLD mode
} KDRV_IPE_IQ_REGION_WEIGHT;

typedef struct {
	KDRV_IPE_IQ_REGION_THRESH reg_th;  ///< the region threshold for region classification
	KDRV_IPE_IQ_REGION_WEIGHT reg_wt;  ///< the region weight for finding kernel weighting for blending
} KDRV_IPE_IQ_REGION_PARAM;

typedef struct {
	UINT8	eext_blending_th;			///< Legal range : 0~255, for edge kernel blending, luma threshold
	UINT8	eext_blending_w1;			///< Legal range : 0~255, for edge kernel blending, low region weight
	UINT8	eext_blending_w2;			///< Legal range : 0~255, for edge kernel blending, high region weight
	INT8	eext_blending_slope;		///< Legal range : -16~15,  for edge kernel blending, reansition slope
} KDRV_IPE_IQ_EDGE_KER_BLENDING;

typedef struct {
	KDRV_IPE_IQ_EEXT_KER_STR	ker_h;		///< Legal range : 0~63, strength of kernal H
	KDRV_IPE_IQ_EEXT_KER_STR	ker_v;		///< Legal range : 0~63, strength of kernal V
	KDRV_IPE_IQ_EEXT_KER_STR	ker_d1;		///< Legal range : 0~63, strength of kernal D1
	KDRV_IPE_IQ_EEXT_KER_STR	ker_d2;		///< Legal range : 0~63, strength of kernal D2
} KDRV_IPE_IQ_EEXT_DIR_KER_STRENGTH;

typedef struct {
    UINT16	eext_dir_min_s_low_bound;	//Legal range : 0~4095 Directional kernel min similarity low bound
    UINT8	eext_dir_s_th_mul;			//Legal range : 0~7 Dir kernel similarity threshold adjust multiple
    UINT8	eext_dir_s_th_shift;		//Legal range : 0~3 Dir kernel similarity threshold adjust shift
    UINT8	dir_eng_blend_w1;			//Legal range : 0~16 Dir energy and non-Dir energy blending ratio
    UINT8	dir_eng_blend_w2;			//Legal range : 0~16
    UINT8	dir_eng_blend_w3;			//Legal range : 0~16
    UINT8	dir_eng_blend_w4;			//Legal range : 0~16
    UINT8	s_count_th;					//Legal range : 0~4
} KDRV_IPE_IQ_EEXT_DIR_KER_PARAM;

typedef struct {
	INT16 edge_ker[KDRV_IPE_IQ_EDGE_KER_DIV_LEN];			///< Edge extraction user-defined kernel setting, table size: 12, range ([0]:0~1023, [1]~[9]:-512~511, [10]:0~15, [11]:0~31)
	KDRV_IPE_IQ_EEXT_KER_STRENGTH eext_kerstrength;			///< Edge extraction fixed kernel strength setting
	KDRV_IPE_IQ_EEXT_ENG_CON eext_engcon;					///< Edge extraction edge energy setting
	KDRV_IPE_IQ_KER_THICKNESS ker_thickness;				///< Edge extraction kernel thickness setting
	KDRV_IPE_IQ_KER_THICKNESS ker_thickness_hld;			///< Edge extraction kernel thickness in high light detail region setting
	KDRV_IPE_IQ_REGION_PARAM eext_region;					///< Edge extraction region settings
	KDRV_IPE_IQ_EDGE_KER_BLENDING eext_blending;			///< Edge extraction blending
	KDRV_IPE_IQ_EEXT_DIR_KER_STRENGTH dir_ker_strength;		///< Edge extraction strength of dir kernal
	KDRV_IPE_IQ_EEXT_DIR_KER_PARAM dir_ker_para;			///< Edge extraction parameter of dir kernal
} KDRV_IPE_IQ_EEXT;

/************************
    IPE structure - IPE edge tone mapping curve.
************************/
typedef enum {
	KDRV_IPE_IQ_EEXT_PRE_GAM = 0,		///< Select pre-gamma edge extraction
	KDRV_IPE_IQ_EEXT_POST_GAM = 1,		///< Select post-gamma edge extraction
} KDRV_IPE_IQ_EEXT_GAM_ENUM;

typedef struct {
	KDRV_IPE_IQ_EEXT_GAM_ENUM gamma_sel;					///< Select pre- or post-gamma edge extraction
	UINT16 tone_map_lut[KDRV_IPE_IQ_TONE_MAP_LUT_LEN];		///< Tone remap LUT, table size: 65
} KDRV_IPE_IQ_EEXT_TONEMAP;

/************************
    IPE structure - IPE overshoot/undershoot parameters
************************/
typedef struct {
	BOOL overshoot_en;				///< enable function
	UINT8 wt_overshoot;				///< Legal range : 0~255, 128 = 1x, the weight for overshooting
	UINT8 wt_undershoot;			///< Legal range : 0~255, 128 = 1x, the weight for undershooting

	UINT8 th_overshoot;				///< Legal range : 0~255, the threshold for overshooting
	UINT8 th_undershoot;			///< Legal range : 0~255, the threshold for undershooting
	UINT8 th_undershoot_lum;		///< Legal range : 0~255, the luma threshold for undershooting enhance
	UINT8 th_undershoot_eng;		///< Legal range : 0~255, the edge energy threshold for undershootingenhance

	UINT16 slope_overshoot;			///< Legal range : 0~32767 (256, slope = 1), the slope of the overshooting weight curve,
	UINT16 slope_undershoot;		///< Legal range : 0~32767 (256, slope = 1), the slope of the undershooting weight curve
	UINT16 slope_undershoot_lum;	///< Legal range : 0~32767 (256, slope = 1), the slope of the enhanced undershooting luminance weight curve
	UINT16 slope_undershoot_eng;	///< Legal range : 0~32767 (256, slope = 1), the slope of the enhanced undershooting energe weight curve

	UINT8 clamp_wt_mod_lum;			///< Legal range : 0~255, clamping weight of luminace undershooting enhance control
	UINT8 clamp_wt_mod_eng;			///< Legal range : 0~255, clamping weight of edge energy undershooting enhance control
	UINT8 strength_lum_eng;			///< Legal range : 0~15, Strength of undershooting enhance control
	UINT8 norm_lum_eng;				///< Legal range : 0~15, Normalized term of undershooting enhance control
} KDRV_IPE_IQ_EDGE_OVERSHOOT;

/************************
    IPE structure - IPE edge process.
************************/
typedef enum {
	KDRV_IPE_IQ_EIN_ENG = 0,		///< Select edge energy for edge mapping input
	KDRV_IPE_IQ_EIN_EEXT = 1,		///< Select EEXT for edge mapping input
	KDRV_IPE_IQ_EIN_AVG = 2,		///< Select average of EEXT & edge energy for edge mapping input
} KDRV_IPE_IQ_EMAP_IN_ENUM;

typedef struct {
	UINT16 ethr_low;				///< Legal range : 0~1023, for edge mapping, if(Ein < EthrA){Eout=EDTabA[0]}, else if(EthrA <= Ein <EthrB){Eout apply EtabA and EDtabA}, else if(Ein >= EthrB){Eout apply EtabB and EDtabB}
	UINT16 ethr_high;				///< Legal range : 0~1023, for edge mapping, if(Ein < EthrA){Eout=EDTabA[0]}, else if(EthrA <= Ein <EthrB){Eout apply EtabA and EDtabA}, else if(Ein >= EthrB){Eout apply EtabB and EDtabB}
	UINT8 etab_low;					///< Legal range : 0~6, for edge mapping, EDtabA table has 8 sections, each section width is (1 << etaA)
	UINT8 etab_high;				///< Legal range : 0~6, for edge mapping, EDtabB table has 8 sections, each section width is (1 << etaB)
	KDRV_IPE_IQ_EMAP_IN_ENUM map_sel;	///< Edge mapping input selection
} KDRV_IPE_IQ_EDGEMAP_PARAM;

typedef struct {
	UINT16 ethr_low;				///< Legal range : 0~1023, for edge mapping, if(Ein < EthrA){Eout=EDTabA[0]}, else if(EthrA <= Ein <EthrB){Eout apply EtabA and EDtabA}, else if(Ein >= EthrB){Eout apply EtabB and EDtabB}
	UINT16 ethr_high;				///< Legal range : 0~1023, for edge mapping, if(Ein < EthrA){Eout=EDTabA[0]}, else if(EthrA <= Ein <EthrB){Eout apply EtabA and EDtabA}, else if(Ein >= EthrB){Eout apply EtabB and EDtabB}
	UINT8 etab_low;					///< Legal range : 0~6, for edge mapping, EDtabA table has 8 sections, each section width is (1 << etaA)
	UINT8 etab_high;				///< Legal range : 0~6, for edge mapping, EDtabB table has 8 sections, each section width is (1 << etaB)
} KDRV_IPE_IQ_ESMAP_PARAM;

typedef struct {
	KDRV_IPE_IQ_EDGEMAP_PARAM edge_map_th;					///< ED map thresholds
	UINT8 edge_map_lut[KDRV_IPE_IQ_EDGE_MAP_LUT_LEN];		///< ED map LUT(16 entries). Legal range: 0~255. 1X = 255, table size: 16
	KDRV_IPE_IQ_ESMAP_PARAM es_map_th;						///< ES map thresholds
	UINT8 es_map_lut[KDRV_IPE_IQ_ES_MAP_LUT_LEN];			///< ES map LUT(16 entries). Legal range: 0~255. 1X = 64, table size: 16
} KDRV_IPE_IQ_EPROC;

/************************
    IPE structure - IPE RGB LPF
************************/
typedef enum {
	KDRV_IPE_IQ_LPFSIZE_3X3 = 0,	///< 3x3 filter size
	KDRV_IPE_IQ_LPFSIZE_5X5 = 1,	///< 5x5 filter size
	KDRV_IPE_IQ_LPFSIZE_7X7 = 2,	///< 7x7 filter size
} KDRV_IPE_IQ_LPF_SIZE_ENUM;

typedef struct {
	UINT8 lpf_w;								///< LPF weighting, range: 0~15
	UINT8 s_only_w;								///< S-Only weighting, range: 0~15
	UINT16 range_th0;							///< Range threshold 0, range: 0~1023
	UINT16 range_th1;							///< Range threshold 1, range: 0~1023
	KDRV_IPE_IQ_LPF_SIZE_ENUM filt_size;		///< Filter size
} KDRV_IPE_IQ_STR_RGBLPF_PARAM;

typedef struct {
	BOOL enable;									///< enable function
	KDRV_IPE_IQ_STR_RGBLPF_PARAM	lpf_param_r;	///< LPF setting of R channel
	KDRV_IPE_IQ_STR_RGBLPF_PARAM	lpf_param_g;	///< LPF setting of G channel
	KDRV_IPE_IQ_STR_RGBLPF_PARAM	lpf_param_b;	///< LPF setting of B channel
} KDRV_IPE_IQ_RGBLPF;

/************************
    IPE structure - IPE PFR
************************/
typedef struct {
	BOOL enable;
	UINT32 color_u;								///< Legal Range: 0~255, reference color u for PFR
	UINT32 color_v;								///< Legal Range: 0~255, reference color v for PFR
	UINT32 color_wet_r;							///< Legal Range: 0~255, PFR weight for R channel
	UINT32 color_wet_b;							///< Legal Range: 0~255, PFR weight for B channel
	UINT32 cdiff_th;							///< Legal Range: 0~255, threshold for color difference weighting table
	UINT32 cdiff_step;							///< Legal Range: 0~6, step size for color difference weighting table
	UINT32 cdiff_lut[KDRV_IPE_IQ_PFR_COLOR_LEN];	///< Legal Range: 0~255, table for color difference weighting
} KDRV_IPE_IQ_COLORWET_PARAM;

typedef struct {
	BOOL enable;								///< enable function
	BOOL uv_filt_en;							///< UV filter enable function
	BOOL luma_level_en;							///< Luma level threshold enable function
	UINT8  out_wet;								///< PFR out wet,  range: 0~255. 0 = all LPF output, 255: all PRF output
	UINT8  edge_th;								///< PFR edge weighting threshold, range: 0~255
	UINT32 pfr_strength;						///< PFR strength, range: 0~255. 0 = no PFR, 255: strong PFR
	UINT8  edge_str;							///< PFR edge weighting strength, range: 0~255, 128 = 1x
	UINT16 luma_th;								///< PFR luminance level threshold, range: 0~1023
	UINT8  luma_lut[KDRV_IPE_IQ_PFR_LUMA_LEN];	///< PFR luminance level control table, range: 0~255
	UINT8  color_wet_g;							///< Legal Range: 0~255, PFR weight for G channel
	KDRV_IPE_IQ_COLORWET_PARAM color_wet_set[KDRV_IPE_IQ_PFR_SET_NUM];
} KDRV_IPE_IQ_PFR;

/************************
    IPE structure - IPE CC strength.
************************/
typedef enum {
	KDRV_IPE_IQ_CC2_IDENTITY = 0,	///< Identity matrix
	KDRV_IPE_IQ_CC2_GRAY = 1,		///< Gray matrix
} KDRV_IPE_IQ_CC2_SEL_ENUM;

typedef enum {
	KDRV_IPE_IQ_CC_MAX = 0,				///< Select max
	KDRV_IPE_IQ_CC_MIN = 1,				///< Select min
	KDRV_IPE_IQ_CC_MAX_MINUS_MIN = 2,	///< Select max-min
	ENUM_DUMMY4WORD(KDRV_IPE_IQ_CC_STABSEL)
} KDRV_IPE_IQ_CC_STABSEL;

typedef struct {
	UINT32 enable;
	KDRV_IPE_IQ_CC2_SEL_ENUM cc2_sel;			///< Select the other data of CC weighting
	KDRV_IPE_IQ_CC_STABSEL cc_stab_sel;			///< Select CC stab input
	UINT8 fstab[KDRV_IPE_IQ_FTAB_LEN];			///< Color correction stab, table size: 16
	UINT8 fdtab[KDRV_IPE_IQ_FTAB_LEN];			///< Color correction dtab, table size: 16
} KDRV_IPE_IQ_CC;

/************************
    IPE structure - IPE CCM.
************************/
typedef enum {
	KDRV_IPE_IQ_CCRANGE_2_9 = 0,    ///< Precision 2.9
	KDRV_IPE_IQ_CCRANGE_3_8 = 1,    ///< Precision 3.8
	KDRV_IPE_IQ_CCRANGE_4_7 = 2,    ///< Precision 4.7
} KDRV_IPE_IQ_CC_RANGE_ENUM;

typedef enum {
	KDRV_IPE_IQ_CC_PRE_GAM = 0,       ///< Select pre-gamma color correction
	KDRV_IPE_IQ_CC_POST_GAM = 1,      ///< Select post-gamma color correction
} KDRV_IPE_IQ_CC_GAMSEL;

typedef struct {
	KDRV_IPE_IQ_CC_RANGE_ENUM cc_range;        ///< Range of matrix coefficients,0: 3.8 format,1: 4.7format
	KDRV_IPE_IQ_CC_GAMSEL cc_gamma_sel;        ///< Select pre- or post-gamma CC
	INT16 coef[KDRV_IPE_IQ_COEF_LEN];          ///< Color correction matrix [rr rg rb gr gg gb br bg bb], table size: 9
} KDRV_IPE_IQ_CCM;

/************************
    IPE structure - IPE CCTRL PARAM.
************************/
typedef enum {
	KDRV_IPE_IQ_CCTRL_E5b = 0,		///< 5x5 kernel B output
	KDRV_IPE_IQ_CCTRL_E5a = 1,		///< 5x5 kernel A output
	KDRV_IPE_IQ_CCTRL_E7 = 2,		///< 7x7 kernel output
} KDRV_IPE_IQ_CCTRL_SEL_ENUM;

typedef struct {
	BOOL enable;
	INT16 int_ofs;							///< Legal range : -128~127, Y offset, Y' = Y + Intofs
	INT16 sat_ofs;							///< Legal range : -128~127, color saturation adjust, 0 : NO color saturation adjust
	BOOL hue_c2g;							///< Select G ch for hue calculation, 0: input is G channel, 1: input is C channel
	KDRV_IPE_IQ_CCTRL_SEL_ENUM cctrl_sel;	///< Color control: vdet source selection
	UINT8 vdet_div;							///< Vdet weighting. out =  in*vdetdiv>>8
	UINT8 edge_tab[KDRV_IPE_IQ_CCTRL_TAB_LEN];	///< Legal range : 0~255, color control edge adjust table, 128 : NO edge adjust, table size: 24
	UINT8 dds_tab[KDRV_IPE_IQ_DDS_TAB_LEN];		///< Legal range : 0~255, color control weighting table for SatTab, IntTab, and EdgTab, 0 : NO adjust, 32 : 1X adjust weighting, table size: 8
	BOOL hue_rotate_en;							///< Hue adjust rotation option, 0 : NO rotation, 1 : CHUEM[n] LSB 2 bit = 0 : 0 degree rotation, 1 : 90 degree rotation, 2 bit = 2 : 180 degree rotation, 3 : 270 degree rotation
	UINT8 hue_tab[KDRV_IPE_IQ_CCTRL_TAB_LEN];	///< Legal range : 0~255, color control hue adjust table, 128 : NO hue adjust, table size: 24
	INT8 sat_tab[KDRV_IPE_IQ_CCTRL_TAB_LEN];	///< Legal range : -128~127, color control sat adjust table, 0 : NO sat adjust, -128 : Min sat suppress, 127 : Max sat enhance, table size: 24
	INT8 int_tab[KDRV_IPE_IQ_CCTRL_TAB_LEN];	///< Legal range : -128~127, color control int adjust table, 0 : NO int adjust, -128 : Min int suppress, 127 : Max int enhance, table size: 24
} KDRV_IPE_IQ_CCTRL;

/************************
    IPE structure - IPE CADJ enhance & inverse.
************************/
typedef struct {
	BOOL enable;                ///< enable function
	UINT16 edge_enh_p;          ///< Positive edge enhance weighting. Legal range: 0~1023. 1X = 64.
	UINT16 edge_enh_n;          ///< Negative edge enhance weighting. Legal range: 0~1023. 1X = 64.
	BOOL edge_inv_p_en;         ///< Legal range : 0~1, if EInvP is 1, positive edge(overshoot) is inverted to negative edge(undershoot)
	BOOL edge_inv_n_en;         ///< Legal range : 0~1, if EInvN is 1, negative edge(undershoot) is inverted to positive edge(overshoot)
} KDRV_IPE_IQ_CADJ_EE;

/************************
    IPE structure - IPE CADJ YC contrast.
************************/
typedef enum {
	KDRV_IPE_IQ_SATURATION = 0,            ///<
	KDRV_IPE_IQ_ABS_MEAN = 1,              ///<
} KDRV_IPE_IQ_CCONTAB_SEL;

typedef struct {
	BOOL enable;                            ///< enable function
	UINT8 y_con;                            ///< Legal range : 0~255, Y contrast adjust, 128 : NO contrst adjust, Y' = (Y * Ycon) >> 7
	UINT8 c_con;                            ///< Legal range : 0~255, CbCr contrast adjust, 128 : NO contrst adjust, C' = (C * CCon) >> 7
	UINT16 cconlut[KDRV_IPE_IQ_CCONTAB_LEN];   ///< Legal range : 0~1023, CbCr contrast adjust, 128 : NO contrst adjust, C' = (C * CCon) >> 7
	KDRV_IPE_IQ_CCONTAB_SEL ccontab_sel;       ///< Legal range : 0~1, saturation mapping selection, 0 : use saturation as x-axis, 1: abs mean of cb, cr
} KDRV_IPE_IQ_CADJ_YCCON;

/************************
    IPE structure - IPE CADJ CbCr offset.
************************/
typedef struct {
	BOOL enable;                ///< enable function
	UINT8 cb_ofs;               ///< Legal range : 0~255, Cb color offset, 128 : NO color offset
	UINT8 cr_ofs;               ///< Legal range : 0~255, Cr color offset, 128 : NO color offset
} KDRV_IPE_IQ_CADJ_COFS;

/************************
    IPE structure - IPE noise effect.
************************/
typedef struct {
	BOOL enable;				///< enable function
	BOOL rand_en_y;				///< Enable of random noise on Y
	BOOL rand_en_c;				///< Enable of random noise on C
	UINT8 rand_level_y;			///< Legal range : 0~7, 0 : NO Y random noise, 7 : MAX Y random noise level
	UINT8 rand_level_c;			///< Legal range : 0~7, 0 : NO CbCr random noise, 7 : MAX CbCr random noise level
	BOOL rand_reset;			///< Legal range : 0~1, 0 : No reset for random pattern sequence, 1 : Frame start reset for random pattern seqeunce.
} KDRV_IPE_IQ_CADJ_RAND;

/************************
    IPE structure - IPE CADJ Hue adjustment function enable.
************************/
typedef struct {
	BOOL enable;                                ///< enable function
} KDRV_IPE_IQ_CADJ_HUE;

/************************
    IPE structure - IPE YC fix effect.
************************/
typedef enum {
	KDRV_IPE_IQ_YCTH_ORIGINAL_VALUE = 0,
	KDRV_IPE_IQ_YCTH_REGISTER_VALUE = 1,
} KDRV_IPE_IQ_YCTH_SEL_ENUM;

typedef struct {
	UINT8 y_th;									///< Y threshold, range: 0~255
	UINT16 edge_th;								///< Edge threshold, range: 0~1023
	KDRV_IPE_IQ_YCTH_SEL_ENUM ycth_sel_hit;		///< Value selection when hit
	KDRV_IPE_IQ_YCTH_SEL_ENUM ycth_sel_nonhit;	///< Value selection when not-hit
	UINT8 value_hit;							///< The register value when hit, range: 0~255
	UINT8 nonvalue_hit;							///< The register value when not-hit, range: 0~255
} KDRV_IPE_IQ_STR_YTH1_PARAM;

typedef struct {
	UINT8 y_th;									///< Y threshold, range: 0~255
	KDRV_IPE_IQ_YCTH_SEL_ENUM ycth_sel_hit;		///< Value selection when hit
	KDRV_IPE_IQ_YCTH_SEL_ENUM ycth_sel_nonhit;	///< Value selection when not-hit
	UINT8 value_hit;							///< The register value when hit, range: 0~255
	UINT8 nonvalue_hit;							///< The register value when not-hit, range: 0~255
} KDRV_IPE_IQ_STR_YTH2_PARAM;

typedef struct {
	UINT16 edge_th;								///< Edge threshold, range: 0~1023
	UINT8 y_th_low;								///< Low threshold of Y
	UINT8 y_th_high;							///< High threshold of Y
	UINT8 cb_th_low;							///< Low threshold of CB
	UINT8 cb_th_high;							///< High threshold of CB
	UINT8 cr_th_low;							///< Low threshold of CR
	UINT8 cr_th_high;							///< High threshold of CR
	KDRV_IPE_IQ_YCTH_SEL_ENUM ycth_sel_hit;		///< Value selection when hit
	KDRV_IPE_IQ_YCTH_SEL_ENUM ycth_sel_nonhit;	///< Value selection when not-hit
	UINT8 cb_value_hit;							///< The register value of CB when hit, range: 0~255
	UINT8 cb_value_nonhit;						///< The register value of CB when not-hit, range: 0~255
	UINT8 cr_value_hit;							///< The register value of CR when hit, range: 0~255
	UINT8 cr_value_nonhit;						///< The register value of CR when not-hit, range: 0~255
} KDRV_IPE_IQ_STR_CTH_PARAM;

typedef struct {
	BOOL enable;							///< enable function
	KDRV_IPE_IQ_STR_YTH1_PARAM yth1;		///< Y threshold setting 1
	KDRV_IPE_IQ_STR_YTH2_PARAM yth2;		///< Y threshold setting 2
	KDRV_IPE_IQ_STR_CTH_PARAM cth;			///< C threshold setting
} KDRV_IPE_IQ_CADJ_FIXTH;

/************************
    IPE structure - IPE YC mask effect.
************************/
typedef struct {
	BOOL enable;            ///< enable function
	UINT8 y_mask;           ///< Mask of Y channel, range: 0~255
	UINT8 cb_mask;          ///< Mask of CB channel, range: 0~255
	UINT8 cr_mask;          ///< Mask of CR channel, range: 0~255
} KDRV_IPE_IQ_CADJ_MASK;

/************************
    IPE structure - IPE Color space transform.
************************/
typedef struct {
	BOOL enable;							///< enable function
	INT16 cst_coef[KDRV_IPE_IQ_COEF_LEN];	///< Color space transform matrix [yr yg yb ur ug ub vr vg vb], table size: 9
	UINT32 cst_off_sel;						///< Operation when CST is off
} KDRV_IPE_IQ_CST;

/************************
    IPE structure - IPE Color space transform protection.
************************/
typedef struct {
	BOOL enable;            ///< enable function
	UINT8 cstp_ratio;       ///< Ratio of color space transform protection
} KDRV_IPE_IQ_CSTP;

/************************
    IPE structure - IPE random input of gamma and y curve
************************/
typedef struct {
	BOOL enable;                                ///< enable function
	BOOL rand_en;                               ///< Enable of random input
	BOOL rst_en;                                ///< Reset of random input
	UINT8 rand_shift;                           ///< Shift of random input, range: 0~1, 0:2bits, 1:3bits
} KDRV_IPE_IQ_GAMYRAND;

/************************
    IPE structure - IPE RGB gamma.
************************/
typedef enum {
	KDRV_IPE_IQ_GAMMA_RGB_COMBINE = 0,  ///< RGB curve combin, reference uiGammaAddr[KDRV_IPP_RGB_R]
	KDRV_IPE_IQ_GAMMA_RGB_SEPERATE = 1, ///< RGB curve seperate, only support when PARAM_YCURVE disable
} KDRV_IPE_IQ_GAMMA_OPTION;

typedef enum {
	KDRV_IPE_IQ_RGB_R = 0,                  ///< process channel R
	KDRV_IPE_IQ_RGB_G,                      ///< process channel G
	KDRV_IPE_IQ_RGB_B,                      ///< process channel B
	KDRV_IPE_IQ_RGB_MAX_CH,
} KDRV_IPE_IQ_RGB_CH;

typedef struct {
	BOOL enable;															///< enable function
	KDRV_IPE_IQ_GAMMA_OPTION option;										///< select rgb gamma combin or seperate
	union {
		UINT32 gamma_lut[KDRV_IPE_IQ_RGB_MAX_CH][KDRV_IPE_IQ_GAMMA_LEN];	///< gamma DRAM input address, each channel table size: 129, UINT32
		UINT32 gamma_lut_trans[KDRV_IPE_IQ_GAMMA_TRANS_LEN];				///< gamma DRAM input address (only for kdrv), table size: 208, include r&g&b
	} lut;
} KDRV_IPE_IQ_GAMMA;

/************************
    IPE structure - IPE y curve.
************************/
typedef enum {
	KDRV_IPE_IQ_YCURVE_RGB_Y     = 0,       ///<
	KDRV_IPE_IQ_YCURVE_RGB_V     = 1,       ///<
	KDRV_IPE_IQ_YCURVE_YUV       = 2,       ///<
} KDRV_IPE_IQ_YCURVE_SEL;

typedef struct {
	BOOL enable;												///< enable function
	KDRV_IPE_IQ_YCURVE_SEL ycurve_sel;							///< y curve selection
	union {
		UINT32 y_curve_lut[KDRV_IPE_IQ_YCURVE_LEN];				///< y curve DRAM input address, table size: 129, UINT32
		UINT32 y_curve_lut_trans[KDRV_IPE_IQ_YCURVE_TRANS_LEN];	///< y curve DRAM input address (only for kdrv), table size: 80, UINT32
	} lut;
} KDRV_IPE_IQ_YCURVE;

/************************
    IPE structure - IPE Defog.
************************/
typedef struct {
	UINT8 interp_diff_lut[KDRV_IPE_IQ_DFG_INTERP_DIFF_LEN];    ///< Legal Range: 0~ 63, difference table for edge preserved interpolation
	UINT8 interp_wdist;                                 ///< Legal Range: 0~ 8, weight of distance beween min and average in edge preserved interpolation
	UINT8 interp_wout;                                  ///< Legal Range: 0~ 8, weight of edge preserve result, 0: no edge preserved,  8: edge preserved
	UINT8 interp_wcenter;                               ///< Legal Range: 1~ 63, weight of center in edge preserve interpolation
	UINT8 interp_wsrc;                                  ///< Legal Range: 0~ 255, weight of edge preserve result and src image
} KDRV_IPE_IQ_DEFOG_SCAL_PARAM;

typedef struct {
	UINT8  in_blend_wt[KDRV_IPE_IQ_DFG_INPUT_BLD_LEN];         ///< Legal Range: 0~ 255, input blending weight table
} KDRV_IPE_IQ_DEFOG_IN_BLD_PARAM;

typedef struct {
	BOOL dfg_self_comp_en;								///< defog self compare enable
	UINT16 dfg_min_diff;								///< defog minimum difference
	UINT16 dfg_airlight[KDRV_IPE_IQ_DFG_AIRLIGHT_NUM];	///< defog airlight settings, range: 0~ 1023
	UINT16 fog_mod_lut[KDRV_IPE_IQ_DFG_FOG_MOD_LEN];	///< defog fog modify table, range: 0~ 1023
} KDRV_IPE_IQ_DEFOG_ENV_ESTIMATION;

typedef enum {
	KDRV_IPE_IQ_DEFOG_METHOD_A    = 0,       ///<
	KDRV_IPE_IQ_DEFOG_METHOD_B    = 1,       ///<
} KDRV_IPE_IQ_DEFOG_METHOD_SEL;

typedef struct {
	KDRV_IPE_IQ_DEFOG_METHOD_SEL  str_mode_sel;			///< defog method selection
	UINT16 target_lut[KDRV_IPE_IQ_DFG_TARGET_LEN];		///< defog target min for method A, range: 0~ 1023
	UINT8 fog_ratio;									///< defog fog ratio for method B, range : 0~255
	UINT8 dgain_ratio;									///< defog d gain ratio, range: 0~ 255
	UINT8 gain_th;										///< defog gain threshold, range: 0~255
} KDRV_IPE_IQ_DEFOG_STRENGTH_PARAM;

typedef enum {
	KDRV_IPE_IQ_DEFOG_OUTBLD_REF_BEFOR    = 0,       ///<
	KDRV_IPE_IQ_DEFOG_OUTBLD_REF_AFTER    = 1,       ///<
} KDRV_IPE_IQ_DEFOG_OUTBLD_REF_SEL;

typedef struct {
	KDRV_IPE_IQ_DEFOG_OUTBLD_REF_SEL outbld_ref_sel;		///< defog output blending reference selection
	BOOL outbld_local_en;									///< defog local output blending enable
	UINT8 outbld_lum_wt[KDRV_IPE_IQ_DFG_OUTPUT_BLD_LEN];	///< defog luminance output blending table, range: 0~255
	UINT8 outbld_diff_wt[KDRV_IPE_IQ_DFG_OUTPUT_BLD_LEN];	///< defog difference output blending table, range: 0~63
} KDRV_IPE_IQ_DEFOG_OUTBLD_PARAM;

typedef struct {
	UINT32 airlight_stcs_ratio;                         ///< the ratio of the total pixel count, 4096 = 1x
} KDRV_IPE_IQ_DEFOG_STCS_RATIO_PARAM;

typedef enum {
	KDRV_IPE_IQ_ROUNDING = 0,             ///< rounding
	KDRV_IPE_IQ_HALFTONE = 1,             ///< halftone rounding
	KDRV_IPE_IQ_RANDOM = 2,               ///< random rounding
} KDRV_IPE_IQ_DEFOG_ROUNDOPT;

typedef enum {
	KDRV_IPE_IQ_B_DISABLE = 0,         ///< disable
	KDRV_IPE_IQ_B_ENABLE = 1,          ///< enable
} KDRV_IPE_IQ_ENABLE;

typedef struct {
	KDRV_IPE_IQ_DEFOG_ROUNDOPT rand_opt;   ///< Rounding option
	KDRV_IPE_IQ_ENABLE rand_rst;           ///< Reset of random input
} KDRV_IPE_IQ_DEFOGROUND;

typedef struct {
	BOOL enable;											///< enable function
	KDRV_IPE_IQ_DEFOG_SCAL_PARAM		scalup_param;		///< defog subin scaling parameters
	KDRV_IPE_IQ_DEFOG_IN_BLD_PARAM		input_bld;			///< defog input blending parameters
	KDRV_IPE_IQ_DEFOG_ENV_ESTIMATION    env_estimation;		///< defog environment estimation settings
	KDRV_IPE_IQ_DEFOG_STRENGTH_PARAM	dfg_strength;		///< defog strength settings
	KDRV_IPE_IQ_DEFOG_OUTBLD_PARAM		dfg_outbld;			///< defog output blending settings
	KDRV_IPE_IQ_DEFOG_STCS_RATIO_PARAM	dfg_stcs;			///< defog statistics
	KDRV_IPE_IQ_DEFOGROUND				dfg_round;			///< defog round
} KDRV_IPE_IQ_DEFOG;

/************************
    IPE structure - IPE Local contrast enhancement.
************************/
typedef struct {
	UINT8  in_blend_wt[KDRV_IPE_IQ_LCE_INPUT_BLD_LEN];	///< Legal Range: 0~ 255, input blending weight table
} KDRV_IPE_IQ_LCE_IN_BLD_PARAM;

typedef struct {
	UINT8 interp_diff_lut[KDRV_IPE_IQ_LCE_INTERP_DIFF_LEN];
	UINT8 interp_wout;
	UINT8 interp_wcenter;
} KDRV_IPE_IQ_LCE_SCAL_PARAM;

typedef struct {
	BOOL enable;								///< enable function
	UINT8 diff_wt_pos;							///< Lce positive weight for difference gain, range: 0~ 255, 128 = 1x.
	UINT8 diff_wt_neg;							///< Lce negative weight for difference gain, range: 0~ 255, 128 = 1x.
	UINT8 diff_wt_avg;							///< Lce center weight for difference gain, range: 0~ 255, 128 = 1x.
	UINT8 lum_wt_lut[KDRV_IPE_IQ_LCE_LUMA_LEN];	///< Lce luma weighting table, range: 0~ 255, 64 = 1x
	KDRV_IPE_IQ_LCE_IN_BLD_PARAM input_bld;		///< lce input blending parameters
	KDRV_IPE_IQ_LCE_SCAL_PARAM   scalup_param;	///< lce subin scaling parameters
} KDRV_IPE_IQ_LCE;

/************************
    IPE structure - IPE defog/lce subimg image info.
************************/
typedef struct {
	UINT32 h_size;		///< image width(pixel)
	UINT32 v_size;		///< image height(pixel)
} KDRV_IPE_IQ_SIZE;

typedef struct {
	KDRV_IPE_IQ_SIZE subimg_size;							///< defog sub-image size, max (32 x 32)
	//UINT32	subimg_lofs_in;									///< defog subin lineoffset
	//UINT32	subimg_lofs_out;								///< defog subout lineoffset
	UINT8	subimg_ftrcoef[KDRV_IPE_IQ_SUBIMG_FILT_LEN];	///< sub-image low pass filter coefficients, 3 entries
} KDRV_IPE_IQ_SUBIMG;

/************************
    IPE structure - IPE Edge Debug mode parameter.
************************/
typedef enum {
	KDRV_IPE_IQ_DBG_EDGE_REGION    = 0,       ///<
	KDRV_IPE_IQ_DBG_EDGE_WEIGHT    = 1,       ///<
	KDRV_IPE_IQ_DBG_EDGE_STRENGTH  = 2,       ///<
} KDRV_IPE_IQ_EDGEDBG_SEL;

typedef struct {
	BOOL enable;
	KDRV_IPE_IQ_EDGEDBG_SEL mode_sel;			///< Edge debug mode selection, 0: region, 1: kerenl weighting, 2: edge map
} KDRV_IPE_IQ_EDGEDBG;

/************************
    IPE structure - IPE VA paramater settingss
************************/
typedef enum {
	KDRV_IPE_IQ_VA_FILTER_SYM_MIRROR = 0,
	KDRV_IPE_IQ_VA_FILTER_SYM_INVERSE,
} KDRV_IPE_IQ_VA_FILTER_SYM_SEL;

typedef enum {
	KDRV_IPE_IQ_VA_FILTER_SIZE_1 = 0,
	KDRV_IPE_IQ_VA_FILTER_SIZE_3,
	KDRV_IPE_IQ_VA_FILTER_SIZE_5,
	KDRV_IPE_IQ_VA_FILTER_SIZE_7,
} KDRV_IPE_IQ_VA_FILTER_SIZE_SEL;

typedef struct {
	KDRV_IPE_IQ_VA_FILTER_SYM_SEL symmetry;		///< select filter kernel symmetry
	KDRV_IPE_IQ_VA_FILTER_SIZE_SEL filter_size;	///< select filter kernel size
	INT16 tap_a;								///< filter coefficent A, range: -511 ~ 511
	INT16 tap_b;								///< filter coefficent B, range: -511 ~ 511
	INT16 tap_c;								///< filter coefficent C, range: -511 ~ 511
	INT16 tap_d;								///< filter coefficent D, range: -511 ~ 511
	UINT8 div;									///< filter normalized term, range: 0~15
	UINT8 th_l;									///< lower threshold, range: 0~255
	UINT8 th_u;									///< upper threshold, range: 0~255
} KDRV_IPE_IQ_VA_FILTER;

typedef struct {
	KDRV_IPE_IQ_VA_FILTER h_filt;				///< horizontal filter
	KDRV_IPE_IQ_VA_FILTER v_filt;				///< vertical filter
	BOOL count_enable;							///< count the nonzero pixels
} KDRV_IPE_IQ_VA_GROUP_INFO;

typedef struct {
	BOOL enable;								///< va independent window enable
} KDRV_IPE_IQ_VA_INDEP_OPT;

typedef struct {
	UINT8 ldg_low_th;          ///< Darkness threshold
	UINT8 ldg_high_th;         ///< Brightness threshold
	UINT8 ldg_low_gain;        ///< Dark region minima gain
	UINT8 ldg_high_gain;       ///< Bright region minima gain
	UINT8 ldg_low_slope;       ///< Dark region gain slope
	UINT8 ldg_high_slope;      ///< Bright region gain slope
} KDRV_IPE_IQ_VA_LDG_PARAM;

typedef enum {
	KDRV_IPE_IQ_NO_PRE_FILTER     = 0,   ///<
	KDRV_IPE_IQ_PRE_FILTER_3x3_1  = 1,   ///<
	KDRV_IPE_IQ_PRE_FILTER_3x3_2  = 2,   ///<
} KDRV_IPE_IQ_VA_PRE_FILTER_MODE;

typedef enum {
	KDRV_IPE_IQ_VA_ENERGY_COUNT     = 0,   ///<
	KDRV_IPE_IQ_VA_HIGH_LUMA_COUNT  = 1,   ///<
} KDRV_IPE_IQ_VA_CNT_OUTSEL;

typedef struct {
	BOOL enable;								///< va function enable
	BOOL indep_va_enable;						///< independent va function enable
	BOOL ldg_enable;							///< va level dependent gain enable

	//filter
	KDRV_IPE_IQ_VA_GROUP_INFO group_1;			///< va group 1 info
	KDRV_IPE_IQ_VA_GROUP_INFO group_2;			///< va group 2 info

	//VA
	BOOL va_out_grp1_2;							///< 0 --> output only group 1, 1 --> output group 1 and 2
	//UINT32 va_lofs;								///< va output lineoffset
	USIZE win_num;								///< va window number, 1x1 ~ 8x8

	//independent VA
	KDRV_IPE_IQ_VA_INDEP_OPT indep_win[KDRV_IPE_IQ_VA_INDEP_NUM]; ///< va independent settings

	//ldg
	KDRV_IPE_IQ_VA_LDG_PARAM ldg_para;

	//pre
	KDRV_IPE_IQ_VA_PRE_FILTER_MODE pre_filter_mode;

	KDRV_IPE_IQ_VA_CNT_OUTSEL win_cnt_out_sel;
	UINT8 high_luma_th;
	UINT8 energy_w;

} KDRV_IPE_IQ_VA;

/************************
    IPE structure - IPE edge region strength
************************/
typedef struct {
	UINT8 enable;
	UINT8 enh_thin;
	UINT8 enh_robust;
	INT16 slope_flat;
	INT16 slope_edge;
	UINT8 str_flat;
	UINT8 str_edge;
} KDRV_IPE_IQ_EDGE_REGION_STR;

/************************
    IPE structure - IPE Color Filter Array Interpolation
************************/
/**
    IPE structure - CFA interpolation term
*/
typedef struct {
	UINT16	edge_dth;							///< Edge threshold 1 for Bayer CFA interpolation [0~4095]
	UINT16	edge_dth2;							///< Edge threshold 2 for Bayer CFA interpolation [0~4095]
	UINT16	freq_th;							///< CFA frequency threshold [0~4095]
	UINT8	freq_lut[KDRV_IPE_IQ_CFA_FREQ_NUM];	///< CFA frequency blending weight look-up table [0~15]
	UINT8	luma_wt[KDRV_IPE_IQ_CFA_LUMA_NUM];	///< Luma gain for CFA [0~255]
} KDRV_IPE_IQ_CFA_INTERP;

/**
    IPE CTL CFA correction term
*/
typedef struct {
	BOOL    rb_corr_enable;         //Enable for RB channel correction
	UINT16  rb_corr_th1;            //RB correction noise threshold1, range [0~1023]
	UINT16  rb_corr_th2;            //RB correction noise threshold2, range [0~1023]
} KDRV_IPE_IQ_CFA_CORR;

/**
    IPE structure - CFA false color suppression term
*/
typedef struct {
	BOOL     fcs_dirsel;                       ///< Direction selection for detecting high frequency, 0: detect 2 directions, 1: detect 4 directions
	UINT8    fcs_coring;                       ///< Coring threshold for false color suppression [0~255]
	UINT8    fcs_weight;                       ///< Global strength for false color suppression [0~255]
	UINT8    fcs_strength[KDRV_IPE_IQ_CFA_FCS_NUM];   ///< CFA false color suppression strength [0~15]
} KDRV_IPE_IQ_CFA_FCS;

/**
    IPE CTL CFA high frequency check term
*/
typedef struct {
	BOOL   cl_check_enable;        ///< Enable cross line check
	BOOL   hf_check_enable;        ///< Enable high frequency check
	BOOL   average_mode;           ///< Interpolation method selection when flat region detect
	BOOL   cl_sel;                 ///< operation type when cross line is not found
	UINT8  cl_th;                  ///< Cross line checking threshold, range [0~255]
	UINT8  hf_gth;                 ///< G channel smoothness checking threshold, range [0~7]
	UINT8  hf_diff;                ///< Difference threshold value between Gr and Gb to determing high frequency pattern detected or not, range [0~255]
	UINT8  hf_eth;                 ///< Threshold to determine H or V direction high frequency pattern detected, range [0~255]
	UINT16 ir_g_edge_th;           ///< Noise margin for edge in Green component interpolation [0~1023]
	UINT8  ir_rb_cstrength;        ///< RB channel smooth strength [0~7]
} KDRV_IPE_IQ_CFA_HFC;

/**
    IPE structure - CFA IR subtraction term
*/
typedef struct {
	UINT16   ir_sub_r;                         ///< R channel IR sub [0~4095]
	UINT16   ir_sub_g;                         ///< G channel IR sub [0~4095]
	UINT16   ir_sub_b;                         ///< B channel IR sub [0~4095]
	UINT8    ir_sub_wt_lb;                     ///< Lower bound of IR local weighting [0~255]
	UINT8    ir_sub_th;                        ///< Threshold to decay IR local weighting [0~255]
	UINT8    ir_sub_range;                     ///< Range to decay IR local weighting [0~3]
	UINT16   ir_sat_gain;                      ///< Saturation gain multiplied after IR sub [0~1023]
} KDRV_IPE_IQ_CFA_IR;

typedef enum {
	KDRV_IPE_IQ_PINKR_MOD_G  = 0,	///< modify G channel
	KDRV_IPE_IQ_PINKR_MOD_RB = 1,	///< modify RB channel
	ENUM_DUMMY4WORD(KDRV_IPE_IQ_PINKR_MODE)
} KDRV_IPE_IQ_PINKR_MODE;

/**
    IPE structure - CFA pink reduction term
*/
typedef struct {
	BOOL pink_rd_en;						///< pink reduction enable
	KDRV_IPE_IQ_PINKR_MODE pink_rd_mode;	///< pink reduction mode
	UINT8    pink_rd_th1;					///< pink reduction threshold 1 [0~255]
	UINT8    pink_rd_th2;					///< pink reduction threshold 2 [0~255]
	UINT8    pink_rd_th3;					///< pink reduction threshold 3 [0~255]
	UINT8    pink_rd_th4;					///< pink reduction threshold 4 [0~255]
} KDRV_IPE_IQ_CFA_PINKR;

typedef enum {
	KDRV_IPE_IQ_CGAIN_2_8 = 0,		///< gain range integer/fraction 2.8
	KDRV_IPE_IQ_CGAIN_3_7 = 1,		///< gain range integer/fraction 3.7
	ENUM_DUMMY4WORD(KDRV_IPE_IQ_CGAIN_RANGE)
} KDRV_IPE_IQ_CGAIN_RANGE;

/**
    IPE structure - CFA color gain term
*/
typedef struct {
	UINT16	r_gain;							///< R channel gain [0~1023]
	UINT16	g_gain;							///< G channel gain [0~1023]
	UINT16	b_gain;							///< B channel gain [0~1023]
	KDRV_IPE_IQ_CGAIN_RANGE gain_range;		///< select fraction type of gain: 2.8 or 3.7
} KDRV_IPE_IQ_CFA_CGAIN;

/**
    IPE structure - IPE CFA parameters.
*/
typedef struct {
	BOOL cfa_enable;						///< enable function
	KDRV_IPE_IQ_CFA_INTERP cfa_interp;		///< CFA interpolation term
	KDRV_IPE_IQ_CFA_CORR cfa_correction;	///< CFA correction term
	KDRV_IPE_IQ_CFA_FCS cfa_fcs;			///< CFA false color suppression term
	KDRV_IPE_IQ_CFA_HFC cfa_ir_hfc;			///< CFA high frequency check term
	KDRV_IPE_IQ_CFA_IR cfa_ir_sub;			///< CFA IR subtraction term
	KDRV_IPE_IQ_CFA_PINKR cfa_pink_reduc;	///< CFA pink reduction term
	KDRV_IPE_IQ_CFA_CGAIN cfa_cgain;		///< CFA color gain term
} KDRV_IPE_IQ_CFA;

/************************
    IPE structure - IPE 3D color. 538 add
************************/
typedef struct {
	BOOL enable;												///< enable function
	union {
		UINT32 rgb_3d_lut[KDRV_IPE_IQ_3DCC_LEN];				///< 3DCC DRAM input address, table size: 729, UINT32
		UINT32 rgb_3d_lut_trans[KDRV_IPE_IQ_3DCC_TRANS_LEN];	///< 3DCC DRAM input address (only for kdrv), table size: 912, UINT32
	} lut;
} KDRV_IPE_IQ_3DCC;

/************************
    misc structure
************************/
typedef struct {
	UINT32 ratio_base;
	USIZE winsz_ratio;									///< va window size of each window, max 511x511
	URECT indep_roi_ratio[KDRV_IPE_IQ_VA_INDEP_NUM];	///< va indep-window size, max 511x511
} KDRV_IPE_IQ_VA_WIN;

/************************
    main structure
************************/
typedef struct {
	KDRV_IPE_IQ_EEXT				eext;
	KDRV_IPE_IQ_EEXT_TONEMAP		eext_tonemap;
	KDRV_IPE_IQ_EDGE_OVERSHOOT		edge_overshoot;
	KDRV_IPE_IQ_EPROC				eproc;
	KDRV_IPE_IQ_RGBLPF				rgb_lpf;
	KDRV_IPE_IQ_PFR					pfr;
	KDRV_IPE_IQ_CC					cc;
	KDRV_IPE_IQ_CCM					ccm;
	KDRV_IPE_IQ_CCTRL				cctrl;
	KDRV_IPE_IQ_CADJ_EE				cadj_ee;
	KDRV_IPE_IQ_CADJ_YCCON			cadj_yccon;
	KDRV_IPE_IQ_CADJ_COFS			cadj_cofs;
	KDRV_IPE_IQ_CADJ_RAND			cadj_rand;
	KDRV_IPE_IQ_CADJ_HUE			cadj_hue;
	KDRV_IPE_IQ_CADJ_FIXTH			cadj_fixth;
	KDRV_IPE_IQ_CADJ_MASK			cadj_mask;
	KDRV_IPE_IQ_CST					cst;
	KDRV_IPE_IQ_CSTP				cstp;
	KDRV_IPE_IQ_GAMYRAND			gamy_rand;
	KDRV_IPE_IQ_GAMMA				gamma;
	KDRV_IPE_IQ_YCURVE				y_curve;
	KDRV_IPE_IQ_DEFOG				defog;
	KDRV_IPE_IQ_LCE					lce;
	KDRV_IPE_IQ_SUBIMG				subimg;
	KDRV_IPE_IQ_EDGEDBG				edgedbg;
	KDRV_IPE_IQ_VA					va;
	KDRV_IPE_IQ_EDGE_REGION_STR		edge_region_str;
	KDRV_IPE_IQ_CFA					cfa;
	KDRV_IPE_IQ_3DCC				_3dcc;

	KDRV_IPE_IQ_VA_WIN				va_window;
	KDRV_IPE_UPDATE					update;
	KDRV_IPE_UPDATE2				update2;
} KDRV_IPE_IQ_CFG;

#endif
/*******************************************************************************
*	                          IPP IQ parameter End                             *
*******************************************************************************/

#define KDRV_IPE_VA_INDEP_WIN_MAX  1024//511
#define KDRV_IPE_VA_MAX_WINNUM    (8*8)
#define KDRV_IPE_PFR_GAMMA_LEN    (17)
#define KDRV_IPE_PFR_BLEND_WET_TBL_LEN (65)
#define KDRV_IPE_YCURVE_PRE_LEN   (129)   // y curve len before trans
#define KDRV_IPE_VA_BUF_SIZE      (2048)  // 8(h-blks) x 8(v-blks) x 2(g1/g2)x 2(h/v) x 8(byte) = 2048
#define KDRV_IPE_LCE_BUF_SIZE     (4096)  // share: 32 x 32 x 4, lce indep: 128 x 128 x 2
#define KDRV_IPE_DEFOG_BUF_SIZE   (4096)  // 32 x 32 x 4
#define KDRV_IPE_PFR_RNG_FILTER_G_TH_LEN (5)
#define KDRV_IPE_STRP_NUM_MAX (8)

typedef enum {
	KDRV_IPE_PROC_MODE_LINKLIST = 0,	/* linlist mode */
	KDRV_IPE_PROC_MODE_CPU,				/* cpu write register, only for debug */
	KDRV_IPE_PROC_MODE_MAX,
} KDRV_IPE_PROC_MODE;

typedef enum {
	KDRV_IPE_QUERY_REG_NUM = 0,	/* get reg num for memory alloc.				data_tpye: UINT32 */
	KDRV_IPE_QUERY_STRP_INFO,	/* get stripe info for dce calculate.			data_tpye: KDRV_IPE_STRP_INFO */
	KDRV_IPE_QUERY_ID_MAX,
} KDRV_IPE_QUERY_ID;


typedef enum {
	KDRV_IPE_OUT_ORIGINAL = 0,
	KDRV_IPE_OUT_DIRECTION = 2,
	KDRV_IPE_OUT_ETH = 3,
} KDRV_IPE_DRAM_OUT_SEL;

typedef enum {
	KDRV_IPE_OVLP_AUTO = 0,
	KDRV_IPE_OVLP_16,
} KDRV_IPE_MST_HOVLP;

#if 0
typedef enum {
	KDRV_IPE_RGBLPF_EN 			= 0x00000001,			///< RGB LPF function enable
	KDRV_IPE_RGBGAMMA_EN 		= 0x00000002,			///< RGB Gamma function enable
	KDRV_IPE_YCURVE_EN 			= 0x00000004,			///< Y curve function enable
	KDRV_IPE_CCR_EN 			= 0x00000008,			///< Color correction function enable
	KDRV_IPE_DEFOG_SUBOUT_EN 	= 0x00000010,			///< Defog subimg output function enable
	KDRV_IPE_DEFOG_EN 			= 0x00000020,			///< Defog function enable
	KDRV_IPE_LCE_EN 			= 0x00000040,			///< Local contrast enhancement function enable
	KDRV_IPE_CST_EN 			= 0x00000080,			///< Color space transform function enable, RGB->YCC
	KDRV_IPE_CTRL_EN 			= 0x00000200,			///< Color control function enable
	KDRV_IPE_HADJ_EN 			= 0x00000400,			///< Hue adjustment function enable
	KDRV_IPE_CADJ_EN 			= 0x00000800,			///< Color component adjust function enable
	KDRV_IPE_CADJ_YENH_EN 		= 0x00001000,			///< Color component edge enhance function enable, sub-function of IPE_CADJ_EN
	KDRV_IPE_CADJ_YCON_EN  		= 0x00002000,			///< Color component Y contrast adjust function enable, sub-function of IPE_CADJ_EN
	KDRV_IPE_CADJ_CCON_EN  		= 0x00004000,			///< Color component CbCr contrast adjust function enable, sub-function of IPE_CADJ_EN
	KDRV_IPE_YCTH_EN 			= 0x00008000, 			///< Color component YCbCr substitution function enable, sub-function of IPE_CADJ_EN
	KDRV_IPE_CSTP_EN  			= 0x00010000,			///< Color space transform protect function enable
	KDRV_IPE_EDGE_DBG_EN 		= 0x00020000, 			///< Edge debug mode function enable
	KDRV_IPE_VACC_EN 			= 0x00080000, 			///< Variation accumulation statistics function enable
	KDRV_IPE_VA_IDNEP_WIN0_EN 	= 0x00100000, 			///< Variation accumulation statistics function enable
	KDRV_IPE_VA_IDNEP_WIN1_EN 	= 0x00200000, 			///< Variation accumulation statistics function enable
	KDRV_IPE_VA_IDNEP_WIN2_EN 	= 0x00400000, 			///< Variation accumulation statistics function enable
	KDRV_IPE_VA_IDNEP_WIN3_EN 	= 0x00800000,			///< Variation accumulation statistics function enable
	KDRV_IPE_VA_IDNEP_WIN4_EN 	= 0x01000000,			///< Variation accumulation statistics function enable
	KDRV_IPE_PFR_EN 			= 0x02000000,			///< Purple fringe reductin function enable
} KDRV_IPE_FUNC_EN;
#endif

typedef struct {
	UINT16 airlight[KDRV_IPE_IQ_DFG_AIRLIGHT_NUM];		///< 3 entry array stcs result

#if(IPE_538_KLOFW_SUPPORT == 0)
	UINT16 *p_airlight;		///< 3 entry array stcs result
#endif

} KDRV_IPE_DEFOG_STCS_RESULT;

typedef struct {
	UINT16 alignment;
	UINT16 overlap;
} KDRV_IPE_STRP_INFO;

#if(IPE_538_KDRV_FEATURE == 0)
typedef enum {
	KDRV_ETH_OUT_2BITS = 0,             ///< 2 bits/pixel
	KDRV_ETH_OUT_8BITS = 1,             ///< 8 bits/pixel
} KDRV_IPE_ETH_OUTFMT;

typedef enum {
	KDRV_IPE_ETH_OUT_ORIGINAL = 0,	/* output all pixel */
	KDRV_IPE_ETH_OUT_DOWNSAMPLE,	/* output w/2, h/2, pixel select by b_h_out_sel/b_v_out_sel */
	//KDRV_IPE_ETH_OUT_BOTH,        /* output both type(original + downsample) */  690 removed
	KDRV_IPE_ETH_OUT_UNKNOWN,
	ENUM_DUMMY4WORD(KDRV_IPE_ETH_OUT_SEL)
} KDRV_IPE_ETH_OUT_SEL;
#endif

/**
	IPE DRAM ADDR
 */
typedef struct{
	ULONG va;
	ULONG pa;
}KDRV_IPE_BUF_ADDR;

/**
    IPE dbg info
*/
typedef struct{
	UINT32  ipe_frm_ed_cnt;
	UINT32  ipe_frm_st_cnt;
	UINT32  ipe_strp_done_cnt;
	UINT32  ipe_ll_ed_cnt;
	UINT32  ipe_ll_error_cnt;
	UINT32  ipe_ll_error2_cnt;
	UINT32  ipe_frame_err_cnt;
	UINT32  ipe_ycbcr_out_ed_cnt;
	UINT32  ipe_load_gamma_ed_cnt;
	UINT32  ipe_defog_out_ed_cnt;
	//UINT32  ipe_lce_out_ed_cnt; //538 remove
	UINT32  ipe_cfa_out_ed_cnt;
	UINT32  ipe_va_out_ed_cnt;
	//538 add
	UINT32  ipe_enc_out_ovfl_cnt;
}KDRV_IPE_DBG_INFO;

#if(IPE_538_KDRV_FEATURE == 0)
typedef struct {
	BOOL enable;		///< eth enable
	KDRV_IPE_ETH_OUTFMT out_bit_sel;	///< 0 --> output 2 bit per pixel, 1 --> output 8 bit per pixel
	KDRV_IPE_ETH_OUT_SEL out_sel;
	BOOL h_out_sel;		///< 0 --> select even pixel to output, 1 --> select odd pixel to output
	BOOL v_out_sel;		///< 0 --> select even pixel to output, 1 --> select odd pixel to output
	UINT16 th_low;		///< edge threshold
	UINT16 th_mid;
	UINT16 th_high;
} KDRV_IPE_ETH;
#endif

typedef enum {
	KDRV_IPE_EEXT_G_CHANNEL = 0,       ///< Select G edge extraction
	KDRV_IPE_EEXT_Y_CHANNEL = 1,     ///< Select Y edge extraction
} KDRV_IPE__EEXT_CH_ENUM;

typedef struct {
    UINT32 pfr_edge_eng_th;
	UINT32 pfr_edge_wet_gain;
} KDRV_IPE_PFR_EDGE_INFO;

typedef struct {
	BOOL      ir_cl_check_en;       // Enable cross line check
	BOOL      ir_hf_check_en;       // Enable high frequency check
	BOOL      ir_average_mode;      // Interpolation method selection when flat region detect
	BOOL      ir_cl_sel;            // operation type when cross line is not found
	UINT32    ir_cl_th;             // Cross line checking threshold
	UINT32    ir_hf_gth;            // G channel smoothness checking threshold
	UINT32    ir_hf_diff;           // Difference threshold value between Gr and Gb to determing high frequency pattern detected or not
	UINT32    ir_hf_eth;            // Threshold to determine H or V direction high frequency pattern detected
	UINT32    ir_g_edge_th;         // Noise margin for edge in Green component interpolation
	UINT32    ir_rb_cstrength;      // RB channel smooth strength
} KDRV_IPE_CFA_IR_HFC;

typedef struct {
	UINT32    ir_sub_r;         // R channel IR sub.
	UINT32    ir_sub_g;         // G channel IR sub.
	UINT32    ir_sub_b;         // B channel IR sub.
	UINT32    ir_sub_wt_lb;     // Lower bound of IR local weighting
	UINT32    ir_sub_th;        // Threshold to decay IR local weighting
	UINT32    ir_sub_range;     // Range to decay IR local weighting
	UINT32    ir_sat_gain;      // Saturation gain multiplied after IR sub
} KDRV_IPE_CFA_IR_SUB;

typedef struct {
	KDRV_IPE_CFA_IR_HFC high_freq;
	KDRV_IPE_CFA_IR_SUB ir_sub;
} KDRV_IPE_CFA_RGBIR;

typedef enum {
	KDRV_IPE_GAMMA_R = 0,
	KDRV_IPE_GAMMA_RGB = 1,
	KDRV_IPE_GAMMA_Y = 2,
	KDRV_IPE_GAMMA_R_Y = 3,
} KDRV_IPE_GAMMA_LOAD_TYPE;

typedef enum {
	KDRV_IPE_RGB_R = 0,                  ///< process channel R
	KDRV_IPE_RGB_G,                      ///< process channel G
	KDRV_IPE_RGB_B,                      ///< process channel B
	KDRV_IPE_RGB_MAX_CH,
} KDRV_IPE_RGB_CH;

typedef struct {
	KDRV_IPE_GAMMA_LOAD_TYPE type;
	ULONG gamma_addr;				/* 0: do nothing; else: ll mode must write addr to reg before load */
	ULONG gamma_phyaddr;			/* 0: do nothing; else: ll mode must write addr to reg before load */
	ULONG ycurve_addr;				/* 0: do nothing; else: ll mode must write addr to reg before load */
	ULONG ycurve_phyaddr;			/* 0: do nothing; else: ll mode must write addr to reg before load */
} KDRV_IPE_GAMMA_LOAD_INFO;

typedef struct {
	UINT32 *p_r_lut;
	UINT32 *p_g_lut;
	UINT32 *p_b_lut;
	UINT32 *p_gamma;
} KDRV_IPE_GAMMA_TRANS_INFO;

typedef struct {
	UINT32 *p_y_lut;
	UINT32 *p_ycurve;
	UINT32 ycurv_sel;
} KDRV_IPE_YCURVE_TRANS_INFO;

typedef struct {
	UINT32 *p_3d_lut;
	UINT32 *p_3dcc;
} KDRV_IPE_3DCC_TRANS_INFO;

typedef struct {
	UINT32 localmax_max;
	UINT32 coneng_max;
	UINT32 coneng_avg;
} KDRV_IPE_EDGE_STCS_RESULT;

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
} KDRV_IPE_VA_RST_SETTING_DUAL;

typedef struct {
	UINT32* g1_h;
	UINT32* g1_v;
	UINT32* g2_h;
	UINT32* g2_v;
	UINT32* g1_h_cnt;
	UINT32* g1_v_cnt;
	UINT32* g2_h_cnt;
	UINT32* g2_v_cnt;
	KDRV_IPE_VA_RST_SETTING_DUAL va_setting;
} KDRV_IPE_VA_RST_DUAL;

typedef struct {
	BOOL va_en;
	UINT32 win_num_x;
	UINT32 win_num_y;
	BOOL outsel;
	KDRV_IPE_BUF_ADDR address;
	UINT32 lineoffset;
} KDRV_IPE_VA_RST_SETTING;

typedef struct {
	UINT32 g1_h[KDRV_IPE_VA_MAX_WINNUM];
	UINT32 g1_v[KDRV_IPE_VA_MAX_WINNUM];
	UINT32 g2_h[KDRV_IPE_VA_MAX_WINNUM];
	UINT32 g2_v[KDRV_IPE_VA_MAX_WINNUM];
	UINT32 g1_h_cnt[KDRV_IPE_VA_MAX_WINNUM];
	UINT32 g1_v_cnt[KDRV_IPE_VA_MAX_WINNUM];
	UINT32 g2_h_cnt[KDRV_IPE_VA_MAX_WINNUM];
	UINT32 g2_v_cnt[KDRV_IPE_VA_MAX_WINNUM];

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
	KDRV_IPE_VA_RST_SETTING va_setting;
} KDRV_IPE_VA_RST;

typedef struct {
	UINT32 g1_h[KDRV_IPE_IQ_VA_INDEP_NUM];
	UINT32 g1_v[KDRV_IPE_IQ_VA_INDEP_NUM];
	UINT32 g2_h[KDRV_IPE_IQ_VA_INDEP_NUM];
	UINT32 g2_v[KDRV_IPE_IQ_VA_INDEP_NUM];
	UINT32 g1_h_cnt[KDRV_IPE_IQ_VA_INDEP_NUM];
	UINT32 g1_v_cnt[KDRV_IPE_IQ_VA_INDEP_NUM];
	UINT32 g2_h_cnt[KDRV_IPE_IQ_VA_INDEP_NUM];
	UINT32 g2_v_cnt[KDRV_IPE_IQ_VA_INDEP_NUM];

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
} KDRV_IPE_VA_INDEP_RSLT;

/**
    IPE KDRV CFA subout Param and Enable/Disable
*/

typedef enum {
	KDRV_IPE_CFA_SUBOUT_CH0 = 0,         ///< Select channel 0 as output channel
	KDRV_IPE_CFA_SUBOUT_CH1 = 1,         ///< Select channel 1 as output channel
	KDRV_IPE_CFA_SUBOUT_CH2 = 2,         ///< Select channel 2 as output channel
	KDRV_IPE_CFA_SUBOUT_CH3 = 3,         ///< Select channel 3 as output channel
	KDRV_IPE_CFA_SUBOUT_CH_UNKNOWN,
	ENUM_DUMMY4WORD(KDRV_IPE_CFA_SUBOUT_CH_SEL)
} KDRV_IPE_CFA_SUBOUT_CH_SEL;

typedef enum {
	KDRV_IPE_CFA_SUBOUT_1BYTE = 0,       ///< Output 1 byte per pixel for CFA subout
	KDRV_IPE_CFA_SUBOUT_2BYTE = 1,       ///< Output 2 byte per pixel for CFA subout
	ENUM_DUMMY4WORD(KDRV_IPE_CFA_SUBOUT_BYTE)
} KDRV_IPE_CFA_SUBOUT_BYTE;

typedef struct _KDRV_IPE_CFA_SUBOUT_PARAM {
	BOOL cfa_subout_enable;
	BOOL cfa_subout_flip_enable;
	KDRV_IPE_BUF_ADDR  cfa_addr;                     ///< cfa subout addr
	ULONG cfa_addr_ofs;
	UINT16 cfa_lofs;                                 ///< cfa subout lineoffsett
	KDRV_IPE_CFA_SUBOUT_CH_SEL subout_ch_sel;        ///< CFA sub-img output channel selection
	KDRV_IPE_CFA_SUBOUT_BYTE subout_byte;            ///< CFA sub-img output format
	UINT16 subout_shiftbit;                          ///< CFA sub-img output right shift bit
} KDRV_IPE_CFA_SUBOUT_INFO;

typedef enum {
	KDRV_IPE_LEFT   = 0,
	KDRV_IPE_RIGHT  = 1,
	KDRV_IPE_UNKNOWN = 2,
}KDRV_IPE_DUAL_IDX;

typedef struct{
	UINT16 hw_airlight[3];
}KDRV_IPE_DEFOG;

typedef struct {
	KDRV_IPP_OPMODE mode;
	KDRV_IPP_OP_STATUS ipp_status; // [0]: ipp is start [1]: ipp is stop
	KDRV_IPP_FRAME in_frm;
	KDRV_IPP_FRAME out_frm;

	UINT32 inte_en;
	UINT32 dma_out_en;

	BOOL ycc_en;
	BOOL ycc_shift_mode_en;

#if(IPE_538_KDRV_FEATURE == 0)
	KDRV_IPE_ETH eth_info;
	/* lce para */
	UINT32 lce_sub_out_en;              //lce sub img out enable
	KDRV_IPE_BUF_ADDR lce_sub_in_addr;  //lce sub input
	KDRV_IPE_BUF_ADDR lce_sub_out_addr; //lce sub output
	UINT32 lce_subin_addr_ofs; //defog subin addr offset
	UINT32 lce_subout_addr_ofs;//defog subout addr offset

	/* defog para */
	UINT32 defog_sub_out_en;                //defog sub img out enable
	KDRV_IPE_BUF_ADDR defog_sub_in_addr;    //defog sub input
	KDRV_IPE_BUF_ADDR defog_sub_out_addr;   //defog sub output
#else
	UINT32 defog_subin_addr_ofs; //defog subin addr offset
	UINT32 defog_subout_addr_ofs;//defog subout addr offset

//#if(IPE_538_KLOFW_SUPPORT == 0)
	UINT32 sub_out_en;              //defog sub img out enable
	KDRV_IPE_BUF_ADDR sub_in_addr;  //defog sub input
	KDRV_IPE_BUF_ADDR sub_out_addr; //defog sub output
	// 538 added
	UINT32	subimg_lofs_in;			///< defog subin lineoffset
	UINT32	subimg_lofs_out;		///< defog subout lineoffset
//#endif

#endif

	/* for kdrv_ipp config */
	KDRV_IPE_MST_HOVLP mst_h_ovlp;
	/* va */
	KDRV_IPE_BUF_ADDR va_out_addr;
	UINT32 va_lofs;								///< va output lineoffset
	UINT32 va_out_addr_ofs;
	/* 3dcc */
	KDRV_IPE_BUF_ADDR _3dcc_addr;

	KDRV_IPE_BUF_ADDR ycurve_addr;
	KDRV_IPE_BUF_ADDR gamma_addr;

	KDRV_IPE_DEFOG defog_info;

    KDRV_IPE_CFA_SUBOUT_INFO cfa_sub;

    UINT32 ime_ovlp;

    UINT32 ipe_strp[KDRV_IPE_STRP_NUM_MAX];
    UINT8  ipe_strp_num;
    UINT32 ipe_ovlp;

    UINT32 va_win_stx;    // internal reference
    UINT32 va_win_size_x; // internal reference
    UINT32 va_win_skip_x; // internal reference

    BOOL dual_en;
    KDRV_IPE_DUAL_IDX dual_idx;
    UINT16 dual_start_ofs;
    UINT32 dual_left_width;
    UINT32 dual_right_width;
    UINT16 dual_left_ovlp;
    UINT16 dual_right_ovlp;

} KDRV_IPE_IO_CFG;

typedef struct {
	KDRV_IPE_IO_CFG *p_iocfg;
	KDRV_IPE_IQ_CFG *p_iqcfg;
	void *p_ll_blk;
} KDRV_IPE_JOB_CFG;

INT32 kdrv_ipe_module_init(void);
INT32 kdrv_ipe_module_uninit(void);

INT32 kdrv_ipe_open(UINT32 chip, UINT32 engine);
INT32 kdrv_ipe_close(UINT32 chip, UINT32 engine);

#ifdef CONFIG_PM
INT32 kdrv_ipe_suspend(UINT32 chip, UINT32 engine);
INT32 kdrv_ipe_resume(UINT32 chip, UINT32 engine);
#endif

INT32 kdrv_ipe_query(UINT32 id, KDRV_IPE_QUERY_ID qid, void *p_param);
INT32 kdrv_ipe_get(UINT32 id, KDRV_IPE_PARAM_ID param_id, void *p_param);
INT32 kdrv_ipe_set(UINT32 id, KDRV_IPE_PARAM_ID param_id, void *p_param);

INT32 kdrv_ipe_rtos_init(void);
INT32 kdrv_ipe_rtos_uninit(void);

void kdrv_ipe_dump(void);
void kdrv_ipe_dump_cfg(KDRV_IPE_JOB_CFG *p_cfg, int (*KDRV_IPE_DUMP)(const char *fmt, ...));
void kdrv_ipe_dump_register(void);
void kdrv_ipe_dump_register_log(void);
INT32 kdrv_ipe_dbg_mode(void *p_hdl, UINT32 param_id, void *data);

#endif
