/**
    IPL Ctrl Layer, ISP Interface

    @file       ctl_ipp_isp.h
    @ingroup    mIIPL_Ctrl
    @note       None

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/


#ifndef _CTL_IPE_ISP_H
#define _CTL_IPE_ISP_H


typedef enum {
	CTL_IPE_ISP_CST_NOOP = 0,             ///< No operation
	CTL_IPE_ISP_CST_MINUS128 = 1,         ///< out = in - 128
} CTL_IPE_ISP_CST_OFFSEL;

/*******************************************************************************
*	Following #if 1 ... #endif block include all IPP IQ parameter.             *
*	These content must sync with kdrv IQ parameter. (see kdrv_ipe.h)           *
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
#define CTL_IPE_ISP_EDGE_KER_DIV_LEN		12
#define CTL_IPE_ISP_TONE_MAP_LUT_LEN		65
#define CTL_IPE_ISP_EDGE_MAP_LUT_LEN		16
#define CTL_IPE_ISP_ES_MAP_LUT_LEN			16
#define CTL_IPE_ISP_FTAB_LEN				16  // For fstab/fdtab
#define CTL_IPE_ISP_COEF_LEN				9  // For CCParam.coef/CSTCoef
#define CTL_IPE_ISP_CCTRL_TAB_LEN			24  // For Hue/Sat/Int/Edg
#define CTL_IPE_ISP_DDS_TAB_LEN				8
#define CTL_IPE_ISP_CCONTAB_LEN				17
#define CTL_IPE_ISP_PFR_COLOR_LEN			5
#define CTL_IPE_ISP_PFR_LUMA_LEN			13
#define CTL_IPE_ISP_PFR_SET_NUM				4
#define CTL_IPE_ISP_GAMMA_LEN				129
#define CTL_IPE_ISP_GAMMA_TRANS_LEN			208
#define CTL_IPE_ISP_YCURVE_LEN				129
#define CTL_IPE_ISP_YCURVE_TRANS_LEN		80
#define CTL_IPE_ISP_DFG_INTERP_DIFF_LEN		17
#define CTL_IPE_ISP_DFG_INPUT_BLD_LEN		9
#define CTL_IPE_ISP_DFG_AIRLIGHT_NUM		3
#define CTL_IPE_ISP_DFG_FOG_MOD_LEN			17
#define CTL_IPE_ISP_DFG_TARGET_LEN			9
#define CTL_IPE_ISP_DFG_OUTPUT_BLD_LEN		17
#define CTL_IPE_ISP_LCE_INPUT_BLD_LEN		9
#define CTL_IPE_ISP_LCE_INTERP_DIFF_LEN		17
#define CTL_IPE_ISP_LCE_LUMA_LEN			9
#define CTL_IPE_ISP_SUBIMG_FILT_LEN			3
#define CTL_IPE_ISP_VA_INDEP_NUM			5
#define CTL_IPE_ISP_CFA_FREQ_NUM			16
#define CTL_IPE_ISP_CFA_LUMA_NUM			17
#define CTL_IPE_ISP_CFA_FCS_NUM				16
#define CTL_IPE_ISP_3DCC_LEN				729
#define CTL_IPE_ISP_3DCC_TRANS_LEN			912

/************************
    IPE structure - IPE edge extraction.
************************/
typedef struct {
	UINT8 eext_enh;                ///< Legal range : 0~63, enhance term for kernel
	INT8 eext_div;                 ///< Legal range : -4~3, normalize term for kernel
} CTL_IPE_ISP_EEXT_KER_STR;

typedef struct {
	CTL_IPE_ISP_EEXT_KER_STR ker_freq0;    ///< strength of kernal A
	CTL_IPE_ISP_EEXT_KER_STR ker_freq1;    ///< strength of kernal C
	CTL_IPE_ISP_EEXT_KER_STR ker_freq2;    ///< strength of kernal D
} CTL_IPE_ISP_EEXT_KER_STRENGTH;

typedef struct {
	INT8 eext_div_con;             ///< Legal range : -8~7, normalize term for edge contrast
	INT8 eext_div_eng;             ///< Legal range : -8~7, normalize term for edge engergy
	UINT8 wt_con_eng;              ///< Legal range : 0~8, blending weight between edge engergy and edge contrast, 0: all contrast, 8: all edge energy
} CTL_IPE_ISP_EEXT_ENG_CON;

typedef struct {
	UINT8 wt_ker_thin;             ///< Legal range : 0~8, Thin kernel weighting between A & B, 8: all thinner kernel A
	UINT8 wt_ker_robust;           ///< Legal range : 0~8, robust kernel weighting between B & C, 8: all thinner kernel B
	UINT8 iso_ker_thin;            ///< Legal range : 0~8, Thin kernel weighting between A_B & C, 8: all thinner kernel A_B
	UINT8 iso_ker_robust;          ///< Legal range : 0~8, robust kernel weighting between B_C & D, 8: all thinner kernel B_C
} CTL_IPE_ISP_KER_THICKNESS;

typedef struct {
	UINT16 th_flat;                 ///< Legal range : 0~1023, the threshold for flat region
	UINT16 th_edge;                 ///< Legal range : 0~1023, the threshold for edge region
	UINT16 th_flat_hld;             ///< Legal range : 0~1023, the threshold for flat region in HLD mode
	UINT16 th_edge_hld;             ///< Legal range : 0~1023, the threshold for edge region in HLD mode
	UINT16 th_lum_hld;              ///< Legal range : 0~1023, the luma threshold in HLD mode
} CTL_IPE_ISP_REGION_THRESH;

typedef struct {
	UINT8 wt_low;                  ///< Legal range : 0~16, the kernel weighting of flat region
	UINT8 wt_high;                 ///< Legal range : 0~16, the kernel weighting of edge region
	UINT8 wt_low_hld;              ///< Legal range : 0~16, the kernel weighting of flat region in HLD mode
	UINT8 wt_high_hld;             ///< Legal range : 0~16, the kernel weighting of edge region in HLD mode
} CTL_IPE_ISP_REGION_WEIGHT;

typedef struct {
	CTL_IPE_ISP_REGION_THRESH reg_th;  ///< the region threshold for region classification
	CTL_IPE_ISP_REGION_WEIGHT reg_wt;  ///< the region weight for finding kernel weighting for blending
} CTL_IPE_ISP_REGION_PARAM;

typedef struct {
	UINT8	eext_blending_th;			///< Legal range : 0~255, for edge kernel blending, luma threshold
	UINT8	eext_blending_w1;			///< Legal range : 0~255, for edge kernel blending, low region weight
	UINT8	eext_blending_w2;			///< Legal range : 0~255, for edge kernel blending, high region weight
	INT8	eext_blending_slope;		///< Legal range : -16~15,  for edge kernel blending, reansition slope
} CTL_IPE_ISP_EDGE_KER_BLENDING;

typedef struct {
	CTL_IPE_ISP_EEXT_KER_STR	ker_h;		///< Legal range : 0~63, strength of kernal H
	CTL_IPE_ISP_EEXT_KER_STR	ker_v;		///< Legal range : 0~63, strength of kernal V
	CTL_IPE_ISP_EEXT_KER_STR	ker_d1;		///< Legal range : 0~63, strength of kernal D1
	CTL_IPE_ISP_EEXT_KER_STR	ker_d2;		///< Legal range : 0~63, strength of kernal D2
} CTL_IPE_ISP_EEXT_DIR_KER_STRENGTH;

typedef struct {
    UINT16	eext_dir_min_s_low_bound;	//Legal range : 0~4095 Directional kernel min similarity low bound
    UINT8	eext_dir_s_th_mul;			//Legal range : 0~7 Dir kernel similarity threshold adjust multiple
    UINT8	eext_dir_s_th_shift;		//Legal range : 0~3 Dir kernel similarity threshold adjust shift
    UINT8	dir_eng_blend_w1;			//Legal range : 0~16 Dir energy and non-Dir energy blending ratio
    UINT8	dir_eng_blend_w2;			//Legal range : 0~16
    UINT8	dir_eng_blend_w3;			//Legal range : 0~16
    UINT8	dir_eng_blend_w4;			//Legal range : 0~16
    UINT8	s_count_th;					//Legal range : 0~4
} CTL_IPE_ISP_EEXT_DIR_KER_PARAM;

typedef struct {
	INT16 edge_ker[CTL_IPE_ISP_EDGE_KER_DIV_LEN];			///< Edge extraction user-defined kernel setting, table size: 12, range ([0]:0~1023, [1]~[9]:-512~511, [10]:0~15, [11]:0~31)
	CTL_IPE_ISP_EEXT_KER_STRENGTH eext_kerstrength;			///< Edge extraction fixed kernel strength setting
	CTL_IPE_ISP_EEXT_ENG_CON eext_engcon;					///< Edge extraction edge energy setting
	CTL_IPE_ISP_KER_THICKNESS ker_thickness;				///< Edge extraction kernel thickness setting
	CTL_IPE_ISP_KER_THICKNESS ker_thickness_hld;			///< Edge extraction kernel thickness in high light detail region setting
	CTL_IPE_ISP_REGION_PARAM eext_region;					///< Edge extraction region settings
	CTL_IPE_ISP_EDGE_KER_BLENDING eext_blending;			///< Edge extraction blending
	CTL_IPE_ISP_EEXT_DIR_KER_STRENGTH dir_ker_strength;		///< Edge extraction strength of dir kernal
	CTL_IPE_ISP_EEXT_DIR_KER_PARAM dir_ker_para;			///< Edge extraction parameter of dir kernal
} CTL_IPE_ISP_EEXT;

/************************
    IPE structure - IPE edge tone mapping curve.
************************/
typedef enum {
	CTL_IPE_ISP_EEXT_PRE_GAM = 0,		///< Select pre-gamma edge extraction
	CTL_IPE_ISP_EEXT_POST_GAM = 1,		///< Select post-gamma edge extraction
} CTL_IPE_ISP_EEXT_GAM_ENUM;

typedef struct {
	CTL_IPE_ISP_EEXT_GAM_ENUM gamma_sel;					///< Select pre- or post-gamma edge extraction
	UINT16 tone_map_lut[CTL_IPE_ISP_TONE_MAP_LUT_LEN];		///< Tone remap LUT, table size: 65
} CTL_IPE_ISP_EEXT_TONEMAP;

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
} CTL_IPE_ISP_EDGE_OVERSHOOT;

/************************
    IPE structure - IPE edge process.
************************/
typedef enum {
	CTL_IPE_ISP_EIN_ENG = 0,		///< Select edge energy for edge mapping input
	CTL_IPE_ISP_EIN_EEXT = 1,		///< Select EEXT for edge mapping input
	CTL_IPE_ISP_EIN_AVG = 2,		///< Select average of EEXT & edge energy for edge mapping input
} CTL_IPE_ISP_EMAP_IN_ENUM;

typedef struct {
	UINT16 ethr_low;				///< Legal range : 0~1023, for edge mapping, if(Ein < EthrA){Eout=EDTabA[0]}, else if(EthrA <= Ein <EthrB){Eout apply EtabA and EDtabA}, else if(Ein >= EthrB){Eout apply EtabB and EDtabB}
	UINT16 ethr_high;				///< Legal range : 0~1023, for edge mapping, if(Ein < EthrA){Eout=EDTabA[0]}, else if(EthrA <= Ein <EthrB){Eout apply EtabA and EDtabA}, else if(Ein >= EthrB){Eout apply EtabB and EDtabB}
	UINT8 etab_low;					///< Legal range : 0~6, for edge mapping, EDtabA table has 8 sections, each section width is (1 << etaA)
	UINT8 etab_high;				///< Legal range : 0~6, for edge mapping, EDtabB table has 8 sections, each section width is (1 << etaB)
	CTL_IPE_ISP_EMAP_IN_ENUM map_sel;	///< Edge mapping input selection
} CTL_IPE_ISP_EDGEMAP_PARAM;

typedef struct {
	UINT16 ethr_low;				///< Legal range : 0~1023, for edge mapping, if(Ein < EthrA){Eout=EDTabA[0]}, else if(EthrA <= Ein <EthrB){Eout apply EtabA and EDtabA}, else if(Ein >= EthrB){Eout apply EtabB and EDtabB}
	UINT16 ethr_high;				///< Legal range : 0~1023, for edge mapping, if(Ein < EthrA){Eout=EDTabA[0]}, else if(EthrA <= Ein <EthrB){Eout apply EtabA and EDtabA}, else if(Ein >= EthrB){Eout apply EtabB and EDtabB}
	UINT8 etab_low;					///< Legal range : 0~6, for edge mapping, EDtabA table has 8 sections, each section width is (1 << etaA)
	UINT8 etab_high;				///< Legal range : 0~6, for edge mapping, EDtabB table has 8 sections, each section width is (1 << etaB)
} CTL_IPE_ISP_ESMAP_PARAM;

typedef struct {
	CTL_IPE_ISP_EDGEMAP_PARAM edge_map_th;					///< ED map thresholds
	UINT8 edge_map_lut[CTL_IPE_ISP_EDGE_MAP_LUT_LEN];		///< ED map LUT(16 entries). Legal range: 0~255. 1X = 255, table size: 16
	CTL_IPE_ISP_ESMAP_PARAM es_map_th;						///< ES map thresholds
	UINT8 es_map_lut[CTL_IPE_ISP_ES_MAP_LUT_LEN];			///< ES map LUT(16 entries). Legal range: 0~255. 1X = 64, table size: 16
} CTL_IPE_ISP_EPROC;

/************************
    IPE structure - IPE RGB LPF
************************/
typedef enum {
	CTL_IPE_ISP_LPFSIZE_3X3 = 0,	///< 3x3 filter size
	CTL_IPE_ISP_LPFSIZE_5X5 = 1,	///< 5x5 filter size
	CTL_IPE_ISP_LPFSIZE_7X7 = 2,	///< 7x7 filter size
} CTL_IPE_ISP_LPF_SIZE_ENUM;

typedef struct {
	UINT8 lpf_w;								///< LPF weighting, range: 0~15
	UINT8 s_only_w;								///< S-Only weighting, range: 0~15
	UINT16 range_th0;							///< Range threshold 0, range: 0~1023
	UINT16 range_th1;							///< Range threshold 1, range: 0~1023
	CTL_IPE_ISP_LPF_SIZE_ENUM filt_size;		///< Filter size
} CTL_IPE_ISP_STR_RGBLPF_PARAM;

typedef struct {
	BOOL enable;									///< enable function
	CTL_IPE_ISP_STR_RGBLPF_PARAM	lpf_param_r;	///< LPF setting of R channel
	CTL_IPE_ISP_STR_RGBLPF_PARAM	lpf_param_g;	///< LPF setting of G channel
	CTL_IPE_ISP_STR_RGBLPF_PARAM	lpf_param_b;	///< LPF setting of B channel
} CTL_IPE_ISP_RGBLPF;

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
	UINT32 cdiff_lut[CTL_IPE_ISP_PFR_COLOR_LEN];	///< Legal Range: 0~255, table for color difference weighting
} CTL_IPE_ISP_COLORWET_PARAM;

typedef struct {
	BOOL enable;								///< enable function
	BOOL uv_filt_en;							///< UV filter enable function
	BOOL luma_level_en;							///< Luma level threshold enable function
	UINT8  out_wet;								///< PFR out wet,  range: 0~255. 0 = all LPF output, 255: all PRF output
	UINT8  edge_th;								///< PFR edge weighting threshold, range: 0~255
	UINT32 pfr_strength;						///< PFR strength, range: 0~255. 0 = no PFR, 255: strong PFR
	UINT8  edge_str;							///< PFR edge weighting strength, range: 0~255, 128 = 1x
	UINT16 luma_th;								///< PFR luminance level threshold, range: 0~1023
	UINT8  luma_lut[CTL_IPE_ISP_PFR_LUMA_LEN];	///< PFR luminance level control table, range: 0~255
	UINT8  color_wet_g;							///< Legal Range: 0~255, PFR weight for G channel
	CTL_IPE_ISP_COLORWET_PARAM color_wet_set[CTL_IPE_ISP_PFR_SET_NUM];
} CTL_IPE_ISP_PFR;

/************************
    IPE structure - IPE CC strength.
************************/
typedef enum {
	CTL_IPE_ISP_CC2_IDENTITY = 0,	///< Identity matrix
	CTL_IPE_ISP_CC2_GRAY = 1,		///< Gray matrix
} CTL_IPE_ISP_CC2_SEL_ENUM;

typedef enum {
	CTL_IPE_ISP_CC_MAX = 0,				///< Select max
	CTL_IPE_ISP_CC_MIN = 1,				///< Select min
	CTL_IPE_ISP_CC_MAX_MINUS_MIN = 2,	///< Select max-min
	ENUM_DUMMY4WORD(CTL_IPE_ISP_CC_STABSEL)
} CTL_IPE_ISP_CC_STABSEL;

typedef struct {
	UINT32 enable;
	CTL_IPE_ISP_CC2_SEL_ENUM cc2_sel;			///< Select the other data of CC weighting
	CTL_IPE_ISP_CC_STABSEL cc_stab_sel;			///< Select CC stab input
	UINT8 fstab[CTL_IPE_ISP_FTAB_LEN];			///< Color correction stab, table size: 16
	UINT8 fdtab[CTL_IPE_ISP_FTAB_LEN];			///< Color correction dtab, table size: 16
} CTL_IPE_ISP_CC;

/************************
    IPE structure - IPE CCM.
************************/
typedef enum {
	CTL_IPE_ISP_CCRANGE_2_9 = 0,    ///< Precision 2.9
	CTL_IPE_ISP_CCRANGE_3_8 = 1,    ///< Precision 3.8
	CTL_IPE_ISP_CCRANGE_4_7 = 2,    ///< Precision 4.7
} CTL_IPE_ISP_CC_RANGE_ENUM;

typedef enum {
	CTL_IPE_ISP_CC_PRE_GAM = 0,       ///< Select pre-gamma color correction
	CTL_IPE_ISP_CC_POST_GAM = 1,      ///< Select post-gamma color correction
} CTL_IPE_ISP_CC_GAMSEL;

typedef struct {
	CTL_IPE_ISP_CC_RANGE_ENUM cc_range;        ///< Range of matrix coefficients,0: 3.8 format,1: 4.7format
	CTL_IPE_ISP_CC_GAMSEL cc_gamma_sel;        ///< Select pre- or post-gamma CC
	INT16 coef[CTL_IPE_ISP_COEF_LEN];          ///< Color correction matrix [rr rg rb gr gg gb br bg bb], table size: 9
} CTL_IPE_ISP_CCM;

/************************
    IPE structure - IPE CCTRL PARAM.
************************/
typedef enum {
	CTL_IPE_ISP_CCTRL_E5b = 0,		///< 5x5 kernel B output
	CTL_IPE_ISP_CCTRL_E5a = 1,		///< 5x5 kernel A output
	CTL_IPE_ISP_CCTRL_E7 = 2,		///< 7x7 kernel output
} CTL_IPE_ISP_CCTRL_SEL_ENUM;

typedef struct {
	BOOL enable;
	INT16 int_ofs;							///< Legal range : -128~127, Y offset, Y' = Y + Intofs
	INT16 sat_ofs;							///< Legal range : -128~127, color saturation adjust, 0 : NO color saturation adjust
	BOOL hue_c2g;							///< Select G ch for hue calculation, 0: input is G channel, 1: input is C channel
	CTL_IPE_ISP_CCTRL_SEL_ENUM cctrl_sel;	///< Color control: vdet source selection
	UINT8 vdet_div;							///< Vdet weighting. out =  in*vdetdiv>>8
	UINT8 edge_tab[CTL_IPE_ISP_CCTRL_TAB_LEN];	///< Legal range : 0~255, color control edge adjust table, 128 : NO edge adjust, table size: 24
	UINT8 dds_tab[CTL_IPE_ISP_DDS_TAB_LEN];		///< Legal range : 0~255, color control weighting table for SatTab, IntTab, and EdgTab, 0 : NO adjust, 32 : 1X adjust weighting, table size: 8
	BOOL hue_rotate_en;							///< Hue adjust rotation option, 0 : NO rotation, 1 : CHUEM[n] LSB 2 bit = 0 : 0 degree rotation, 1 : 90 degree rotation, 2 bit = 2 : 180 degree rotation, 3 : 270 degree rotation
	UINT8 hue_tab[CTL_IPE_ISP_CCTRL_TAB_LEN];	///< Legal range : 0~255, color control hue adjust table, 128 : NO hue adjust, table size: 24
	INT8 sat_tab[CTL_IPE_ISP_CCTRL_TAB_LEN];	///< Legal range : -128~127, color control sat adjust table, 0 : NO sat adjust, -128 : Min sat suppress, 127 : Max sat enhance, table size: 24
	INT8 int_tab[CTL_IPE_ISP_CCTRL_TAB_LEN];	///< Legal range : -128~127, color control int adjust table, 0 : NO int adjust, -128 : Min int suppress, 127 : Max int enhance, table size: 24
} CTL_IPE_ISP_CCTRL;

/************************
    IPE structure - IPE CADJ enhance & inverse.
************************/
typedef struct {
	BOOL enable;                ///< enable function
	UINT16 edge_enh_p;          ///< Positive edge enhance weighting. Legal range: 0~1023. 1X = 64.
	UINT16 edge_enh_n;          ///< Negative edge enhance weighting. Legal range: 0~1023. 1X = 64.
	BOOL edge_inv_p_en;         ///< Legal range : 0~1, if EInvP is 1, positive edge(overshoot) is inverted to negative edge(undershoot)
	BOOL edge_inv_n_en;         ///< Legal range : 0~1, if EInvN is 1, negative edge(undershoot) is inverted to positive edge(overshoot)
} CTL_IPE_ISP_CADJ_EE;

/************************
    IPE structure - IPE CADJ YC contrast.
************************/
typedef enum {
	CTL_IPE_ISP_SATURATION = 0,            ///<
	CTL_IPE_ISP_ABS_MEAN = 1,              ///<
} CTL_IPE_ISP_CCONTAB_SEL;

typedef struct {
	BOOL enable;                            ///< enable function
	UINT8 y_con;                            ///< Legal range : 0~255, Y contrast adjust, 128 : NO contrst adjust, Y' = (Y * Ycon) >> 7
	UINT8 c_con;                            ///< Legal range : 0~255, CbCr contrast adjust, 128 : NO contrst adjust, C' = (C * CCon) >> 7
	UINT16 cconlut[CTL_IPE_ISP_CCONTAB_LEN];   ///< Legal range : 0~1023, CbCr contrast adjust, 128 : NO contrst adjust, C' = (C * CCon) >> 7
	CTL_IPE_ISP_CCONTAB_SEL ccontab_sel;       ///< Legal range : 0~1, saturation mapping selection, 0 : use saturation as x-axis, 1: abs mean of cb, cr
} CTL_IPE_ISP_CADJ_YCCON;

/************************
    IPE structure - IPE CADJ CbCr offset.
************************/
typedef struct {
	BOOL enable;                ///< enable function
	UINT8 cb_ofs;               ///< Legal range : 0~255, Cb color offset, 128 : NO color offset
	UINT8 cr_ofs;               ///< Legal range : 0~255, Cr color offset, 128 : NO color offset
} CTL_IPE_ISP_CADJ_COFS;

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
} CTL_IPE_ISP_CADJ_RAND;

/************************
    IPE structure - IPE CADJ Hue adjustment function enable.
************************/
typedef struct {
	BOOL enable;                                ///< enable function
} CTL_IPE_ISP_CADJ_HUE;

/************************
    IPE structure - IPE YC fix effect.
************************/
typedef enum {
	CTL_IPE_ISP_YCTH_ORIGINAL_VALUE = 0,
	CTL_IPE_ISP_YCTH_REGISTER_VALUE = 1,
} CTL_IPE_ISP_YCTH_SEL_ENUM;

typedef struct {
	UINT8 y_th;									///< Y threshold, range: 0~255
	UINT16 edge_th;								///< Edge threshold, range: 0~1023
	CTL_IPE_ISP_YCTH_SEL_ENUM ycth_sel_hit;		///< Value selection when hit
	CTL_IPE_ISP_YCTH_SEL_ENUM ycth_sel_nonhit;	///< Value selection when not-hit
	UINT8 value_hit;							///< The register value when hit, range: 0~255
	UINT8 nonvalue_hit;							///< The register value when not-hit, range: 0~255
} CTL_IPE_ISP_STR_YTH1_PARAM;

typedef struct {
	UINT8 y_th;									///< Y threshold, range: 0~255
	CTL_IPE_ISP_YCTH_SEL_ENUM ycth_sel_hit;		///< Value selection when hit
	CTL_IPE_ISP_YCTH_SEL_ENUM ycth_sel_nonhit;	///< Value selection when not-hit
	UINT8 value_hit;							///< The register value when hit, range: 0~255
	UINT8 nonvalue_hit;							///< The register value when not-hit, range: 0~255
} CTL_IPE_ISP_STR_YTH2_PARAM;

typedef struct {
	UINT16 edge_th;								///< Edge threshold, range: 0~1023
	UINT8 y_th_low;								///< Low threshold of Y
	UINT8 y_th_high;							///< High threshold of Y
	UINT8 cb_th_low;							///< Low threshold of CB
	UINT8 cb_th_high;							///< High threshold of CB
	UINT8 cr_th_low;							///< Low threshold of CR
	UINT8 cr_th_high;							///< High threshold of CR
	CTL_IPE_ISP_YCTH_SEL_ENUM ycth_sel_hit;		///< Value selection when hit
	CTL_IPE_ISP_YCTH_SEL_ENUM ycth_sel_nonhit;	///< Value selection when not-hit
	UINT8 cb_value_hit;							///< The register value of CB when hit, range: 0~255
	UINT8 cb_value_nonhit;						///< The register value of CB when not-hit, range: 0~255
	UINT8 cr_value_hit;							///< The register value of CR when hit, range: 0~255
	UINT8 cr_value_nonhit;						///< The register value of CR when not-hit, range: 0~255
} CTL_IPE_ISP_STR_CTH_PARAM;

typedef struct {
	BOOL enable;							///< enable function
	CTL_IPE_ISP_STR_YTH1_PARAM yth1;		///< Y threshold setting 1
	CTL_IPE_ISP_STR_YTH2_PARAM yth2;		///< Y threshold setting 2
	CTL_IPE_ISP_STR_CTH_PARAM cth;			///< C threshold setting
} CTL_IPE_ISP_CADJ_FIXTH;

/************************
    IPE structure - IPE YC mask effect.
************************/
typedef struct {
	BOOL enable;            ///< enable function
	UINT8 y_mask;           ///< Mask of Y channel, range: 0~255
	UINT8 cb_mask;          ///< Mask of CB channel, range: 0~255
	UINT8 cr_mask;          ///< Mask of CR channel, range: 0~255
} CTL_IPE_ISP_CADJ_MASK;

/************************
    IPE structure - IPE Color space transform.
************************/
typedef struct {
	BOOL enable;							///< enable function
	INT16 cst_coef[CTL_IPE_ISP_COEF_LEN];	///< Color space transform matrix [yr yg yb ur ug ub vr vg vb], table size: 9
	UINT32 cst_off_sel;						///< Operation when CST is off
} CTL_IPE_ISP_CST;

/************************
    IPE structure - IPE Color space transform protection.
************************/
typedef struct {
	BOOL enable;            ///< enable function
	UINT8 cstp_ratio;       ///< Ratio of color space transform protection
} CTL_IPE_ISP_CSTP;

/************************
    IPE structure - IPE random input of gamma and y curve
************************/
typedef struct {
	BOOL enable;                                ///< enable function
	BOOL rand_en;                               ///< Enable of random input
	BOOL rst_en;                                ///< Reset of random input
	UINT8 rand_shift;                           ///< Shift of random input, range: 0~1, 0:2bits, 1:3bits
} CTL_IPE_ISP_GAMYRAND;

/************************
    IPE structure - IPE RGB gamma.
************************/
typedef enum {
	CTL_IPE_ISP_GAMMA_RGB_COMBINE = 0,  ///< RGB curve combin, reference uiGammaAddr[KDRV_IPP_RGB_R]
	CTL_IPE_ISP_GAMMA_RGB_SEPERATE = 1, ///< RGB curve seperate, only support when PARAM_YCURVE disable
} CTL_IPE_ISP_GAMMA_OPTION;

typedef enum {
	CTL_IPE_ISP_RGB_R = 0,                  ///< process channel R
	CTL_IPE_ISP_RGB_G,                      ///< process channel G
	CTL_IPE_ISP_RGB_B,                      ///< process channel B
	CTL_IPE_ISP_RGB_MAX_CH,
} CTL_IPE_ISP_RGB_CH;

typedef struct {
	BOOL enable;															///< enable function
	CTL_IPE_ISP_GAMMA_OPTION option;										///< select rgb gamma combin or seperate
	union {
		UINT32 gamma_lut[CTL_IPE_ISP_RGB_MAX_CH][CTL_IPE_ISP_GAMMA_LEN];	///< gamma DRAM input address, each channel table size: 129, UINT32
		UINT32 gamma_lut_trans[CTL_IPE_ISP_GAMMA_TRANS_LEN];				///< gamma DRAM input address (only for kdrv), table size: 208, include r&g&b
	} lut;
} CTL_IPE_ISP_GAMMA;

/************************
    IPE structure - IPE y curve.
************************/
typedef enum {
	CTL_IPE_ISP_YCURVE_RGB_Y     = 0,       ///<
	CTL_IPE_ISP_YCURVE_RGB_V     = 1,       ///<
	CTL_IPE_ISP_YCURVE_YUV       = 2,       ///<
} CTL_IPE_ISP_YCURVE_SEL;

typedef struct {
	BOOL enable;												///< enable function
	CTL_IPE_ISP_YCURVE_SEL ycurve_sel;							///< y curve selection
	union {
		UINT32 y_curve_lut[CTL_IPE_ISP_YCURVE_LEN];				///< y curve DRAM input address, table size: 129, UINT32
		UINT32 y_curve_lut_trans[CTL_IPE_ISP_YCURVE_TRANS_LEN];	///< y curve DRAM input address (only for kdrv), table size: 80, UINT32
	} lut;
} CTL_IPE_ISP_YCURVE;

/************************
    IPE structure - IPE Defog.
************************/
typedef struct {
	UINT8 interp_diff_lut[CTL_IPE_ISP_DFG_INTERP_DIFF_LEN];    ///< Legal Range: 0~ 63, difference table for edge preserved interpolation
	UINT8 interp_wdist;                                 ///< Legal Range: 0~ 8, weight of distance beween min and average in edge preserved interpolation
	UINT8 interp_wout;                                  ///< Legal Range: 0~ 8, weight of edge preserve result, 0: no edge preserved,  8: edge preserved
	UINT8 interp_wcenter;                               ///< Legal Range: 1~ 63, weight of center in edge preserve interpolation
	UINT8 interp_wsrc;                                  ///< Legal Range: 0~ 255, weight of edge preserve result and src image
} CTL_IPE_ISP_DEFOG_SCAL_PARAM;

typedef struct {
	UINT8  in_blend_wt[CTL_IPE_ISP_DFG_INPUT_BLD_LEN];         ///< Legal Range: 0~ 255, input blending weight table
} CTL_IPE_ISP_DEFOG_IN_BLD_PARAM;

typedef struct {
	BOOL dfg_self_comp_en;								///< defog self compare enable
	UINT16 dfg_min_diff;								///< defog minimum difference
	UINT16 dfg_airlight[CTL_IPE_ISP_DFG_AIRLIGHT_NUM];	///< defog airlight settings, range: 0~ 1023
	UINT16 fog_mod_lut[CTL_IPE_ISP_DFG_FOG_MOD_LEN];	///< defog fog modify table, range: 0~ 1023
} CTL_IPE_ISP_DEFOG_ENV_ESTIMATION;

typedef enum {
	CTL_IPE_ISP_DEFOG_METHOD_A    = 0,       ///<
	CTL_IPE_ISP_DEFOG_METHOD_B    = 1,       ///<
} CTL_IPE_ISP_DEFOG_METHOD_SEL;

typedef struct {
	CTL_IPE_ISP_DEFOG_METHOD_SEL  str_mode_sel;			///< defog method selection
	UINT16 target_lut[CTL_IPE_ISP_DFG_TARGET_LEN];		///< defog target min for method A, range: 0~ 1023
	UINT8 fog_ratio;									///< defog fog ratio for method B, range : 0~255
	UINT8 dgain_ratio;									///< defog d gain ratio, range: 0~ 255
	UINT8 gain_th;										///< defog gain threshold, range: 0~255
} CTL_IPE_ISP_DEFOG_STRENGTH_PARAM;

typedef enum {
	CTL_IPE_ISP_DEFOG_OUTBLD_REF_BEFOR    = 0,       ///<
	CTL_IPE_ISP_DEFOG_OUTBLD_REF_AFTER    = 1,       ///<
} CTL_IPE_ISP_DEFOG_OUTBLD_REF_SEL;

typedef struct {
	CTL_IPE_ISP_DEFOG_OUTBLD_REF_SEL outbld_ref_sel;		///< defog output blending reference selection
	BOOL outbld_local_en;									///< defog local output blending enable
	UINT8 outbld_lum_wt[CTL_IPE_ISP_DFG_OUTPUT_BLD_LEN];	///< defog luminance output blending table, range: 0~255
	UINT8 outbld_diff_wt[CTL_IPE_ISP_DFG_OUTPUT_BLD_LEN];	///< defog difference output blending table, range: 0~63
} CTL_IPE_ISP_DEFOG_OUTBLD_PARAM;

typedef struct {
	UINT32 airlight_stcs_ratio;                         ///< the ratio of the total pixel count, 4096 = 1x
} CTL_IPE_ISP_DEFOG_STCS_RATIO_PARAM;

typedef enum {
	CTL_IPE_ISP_ROUNDING = 0,             ///< rounding
	CTL_IPE_ISP_HALFTONE = 1,             ///< halftone rounding
	CTL_IPE_ISP_RANDOM = 2,               ///< random rounding
} CTL_IPE_ISP_DEFOG_ROUNDOPT;

typedef enum {
	CTL_IPE_ISP_B_DISABLE = 0,         ///< disable
	CTL_IPE_ISP_B_ENABLE = 1,          ///< enable
} CTL_IPE_ISP_ENABLE;

typedef struct {
	CTL_IPE_ISP_DEFOG_ROUNDOPT rand_opt;   ///< Rounding option
	CTL_IPE_ISP_ENABLE rand_rst;           ///< Reset of random input
} CTL_IPE_ISP_DEFOGROUND;

typedef struct {
	BOOL enable;											///< enable function
	CTL_IPE_ISP_DEFOG_SCAL_PARAM		scalup_param;		///< defog subin scaling parameters
	CTL_IPE_ISP_DEFOG_IN_BLD_PARAM		input_bld;			///< defog input blending parameters
	CTL_IPE_ISP_DEFOG_ENV_ESTIMATION    env_estimation;		///< defog environment estimation settings
	CTL_IPE_ISP_DEFOG_STRENGTH_PARAM	dfg_strength;		///< defog strength settings
	CTL_IPE_ISP_DEFOG_OUTBLD_PARAM		dfg_outbld;			///< defog output blending settings
	CTL_IPE_ISP_DEFOG_STCS_RATIO_PARAM	dfg_stcs;			///< defog statistics
	CTL_IPE_ISP_DEFOGROUND				dfg_round;			///< defog round
} CTL_IPE_ISP_DEFOG;

/************************
    IPE structure - IPE Local contrast enhancement.
************************/
typedef struct {
	UINT8  in_blend_wt[CTL_IPE_ISP_LCE_INPUT_BLD_LEN];	///< Legal Range: 0~ 255, input blending weight table
} CTL_IPE_ISP_LCE_IN_BLD_PARAM;

typedef struct {
	UINT8 interp_diff_lut[CTL_IPE_ISP_LCE_INTERP_DIFF_LEN];
	UINT8 interp_wout;
	UINT8 interp_wcenter;
} CTL_IPE_ISP_LCE_SCAL_PARAM;

typedef struct {
	BOOL enable;								///< enable function
	UINT8 diff_wt_pos;							///< Lce positive weight for difference gain, range: 0~ 255, 128 = 1x.
	UINT8 diff_wt_neg;							///< Lce negative weight for difference gain, range: 0~ 255, 128 = 1x.
	UINT8 diff_wt_avg;							///< Lce center weight for difference gain, range: 0~ 255, 128 = 1x.
	UINT8 lum_wt_lut[CTL_IPE_ISP_LCE_LUMA_LEN];	///< Lce luma weighting table, range: 0~ 255, 64 = 1x
	CTL_IPE_ISP_LCE_IN_BLD_PARAM input_bld;		///< lce input blending parameters
	CTL_IPE_ISP_LCE_SCAL_PARAM   scalup_param;	///< lce subin scaling parameters
} CTL_IPE_ISP_LCE;

/************************
    IPE structure - IPE defog/lce subimg image info.
************************/
typedef struct {
	UINT32 h_size;		///< image width(pixel)
	UINT32 v_size;		///< image height(pixel)
} CTL_IPE_ISP_SIZE;

typedef struct {
	CTL_IPE_ISP_SIZE subimg_size;							///< defog sub-image size, max (32 x 32)
	//UINT32	subimg_lofs_in;									///< defog subin lineoffset
	//UINT32	subimg_lofs_out;								///< defog subout lineoffset
	UINT8	subimg_ftrcoef[CTL_IPE_ISP_SUBIMG_FILT_LEN];	///< sub-image low pass filter coefficients, 3 entries
} CTL_IPE_ISP_SUBIMG;

/************************
    IPE structure - IPE Edge Debug mode parameter.
************************/
typedef enum {
	CTL_IPE_ISP_DBG_EDGE_REGION    = 0,       ///<
	CTL_IPE_ISP_DBG_EDGE_WEIGHT    = 1,       ///<
	CTL_IPE_ISP_DBG_EDGE_STRENGTH  = 2,       ///<
} CTL_IPE_ISP_EDGEDBG_SEL;

typedef struct {
	BOOL enable;
	CTL_IPE_ISP_EDGEDBG_SEL mode_sel;			///< Edge debug mode selection, 0: region, 1: kerenl weighting, 2: edge map
} CTL_IPE_ISP_EDGEDBG;

/************************
    IPE structure - IPE VA paramater settingss
************************/
typedef enum {
	CTL_IPE_ISP_VA_FILTER_SYM_MIRROR = 0,
	CTL_IPE_ISP_VA_FILTER_SYM_INVERSE,
} CTL_IPE_ISP_VA_FILTER_SYM_SEL;

typedef enum {
	CTL_IPE_ISP_VA_FILTER_SIZE_1 = 0,
	CTL_IPE_ISP_VA_FILTER_SIZE_3,
	CTL_IPE_ISP_VA_FILTER_SIZE_5,
	CTL_IPE_ISP_VA_FILTER_SIZE_7,
} CTL_IPE_ISP_VA_FILTER_SIZE_SEL;

typedef struct {
	CTL_IPE_ISP_VA_FILTER_SYM_SEL symmetry;		///< select filter kernel symmetry
	CTL_IPE_ISP_VA_FILTER_SIZE_SEL filter_size;	///< select filter kernel size
	INT16 tap_a;								///< filter coefficent A, range: -511 ~ 511
	INT16 tap_b;								///< filter coefficent B, range: -511 ~ 511
	INT16 tap_c;								///< filter coefficent C, range: -511 ~ 511
	INT16 tap_d;								///< filter coefficent D, range: -511 ~ 511
	UINT8 div;									///< filter normalized term, range: 0~15
	UINT8 th_l;									///< lower threshold, range: 0~255
	UINT8 th_u;									///< upper threshold, range: 0~255
} CTL_IPE_ISP_VA_FILTER;

typedef struct {
	CTL_IPE_ISP_VA_FILTER h_filt;				///< horizontal filter
	CTL_IPE_ISP_VA_FILTER v_filt;				///< vertical filter
	BOOL count_enable;							///< count the nonzero pixels
} CTL_IPE_ISP_VA_GROUP_INFO;

typedef struct {
	BOOL enable;								///< va independent window enable
} CTL_IPE_ISP_VA_INDEP_OPT;

typedef struct {
	UINT8 ldg_low_th;          ///< Darkness threshold
	UINT8 ldg_high_th;         ///< Brightness threshold
	UINT8 ldg_low_gain;        ///< Dark region minima gain
	UINT8 ldg_high_gain;       ///< Bright region minima gain
	UINT8 ldg_low_slope;       ///< Dark region gain slope
	UINT8 ldg_high_slope;      ///< Bright region gain slope
} CTL_IPE_ISP_VA_LDG_PARAM;

typedef enum {
	CTL_IPE_ISP_NO_PRE_FILTER     = 0,   ///<
	CTL_IPE_ISP_PRE_FILTER_3x3_1  = 1,   ///<
	CTL_IPE_ISP_PRE_FILTER_3x3_2  = 2,   ///<
} CTL_IPE_ISP_VA_PRE_FILTER_MODE;

typedef enum {
	CTL_IPE_ISP_VA_ENERGY_COUNT     = 0,   ///<
	CTL_IPE_ISP_VA_HIGH_LUMA_COUNT  = 1,   ///<
} CTL_IPE_ISP_VA_CNT_OUTSEL;

typedef struct {
	BOOL enable;								///< va function enable
	BOOL indep_va_enable;						///< independent va function enable
	BOOL ldg_enable;							///< va level dependent gain enable

	//filter
	CTL_IPE_ISP_VA_GROUP_INFO group_1;			///< va group 1 info
	CTL_IPE_ISP_VA_GROUP_INFO group_2;			///< va group 2 info

	//VA
	BOOL va_out_grp1_2;							///< 0 --> output only group 1, 1 --> output group 1 and 2
	//UINT32 va_lofs;								///< va output lineoffset
	USIZE win_num;								///< va window number, 1x1 ~ 8x8

	//independent VA
	CTL_IPE_ISP_VA_INDEP_OPT indep_win[CTL_IPE_ISP_VA_INDEP_NUM]; ///< va independent settings

	//ldg
	CTL_IPE_ISP_VA_LDG_PARAM ldg_para;

	//pre
	CTL_IPE_ISP_VA_PRE_FILTER_MODE pre_filter_mode;

	CTL_IPE_ISP_VA_CNT_OUTSEL win_cnt_out_sel;
	UINT8 high_luma_th;
	UINT8 energy_w;

} CTL_IPE_ISP_VA;

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
} CTL_IPE_ISP_EDGE_REGION_STR;

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
	UINT8	freq_lut[CTL_IPE_ISP_CFA_FREQ_NUM];	///< CFA frequency blending weight look-up table [0~15]
	UINT8	luma_wt[CTL_IPE_ISP_CFA_LUMA_NUM];	///< Luma gain for CFA [0~255]
} CTL_IPE_ISP_CFA_INTERP;

/**
    IPE CTL CFA correction term
*/
typedef struct {
	BOOL    rb_corr_enable;         //Enable for RB channel correction
	UINT16  rb_corr_th1;            //RB correction noise threshold1, range [0~1023]
	UINT16  rb_corr_th2;            //RB correction noise threshold2, range [0~1023]
} CTL_IPE_ISP_CFA_CORR;

/**
    IPE structure - CFA false color suppression term
*/
typedef struct {
	BOOL     fcs_dirsel;                       ///< Direction selection for detecting high frequency, 0: detect 2 directions, 1: detect 4 directions
	UINT8    fcs_coring;                       ///< Coring threshold for false color suppression [0~255]
	UINT8    fcs_weight;                       ///< Global strength for false color suppression [0~255]
	UINT8    fcs_strength[CTL_IPE_ISP_CFA_FCS_NUM];   ///< CFA false color suppression strength [0~15]
} CTL_IPE_ISP_CFA_FCS;

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
} CTL_IPE_ISP_CFA_HFC;

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
} CTL_IPE_ISP_CFA_IR;

typedef enum {
	CTL_IPE_ISP_PINKR_MOD_G  = 0,	///< modify G channel
	CTL_IPE_ISP_PINKR_MOD_RB = 1,	///< modify RB channel
	ENUM_DUMMY4WORD(CTL_IPE_ISP_PINKR_MODE)
} CTL_IPE_ISP_PINKR_MODE;

/**
    IPE structure - CFA pink reduction term
*/
typedef struct {
	BOOL pink_rd_en;						///< pink reduction enable
	CTL_IPE_ISP_PINKR_MODE pink_rd_mode;	///< pink reduction mode
	UINT8    pink_rd_th1;					///< pink reduction threshold 1 [0~255]
	UINT8    pink_rd_th2;					///< pink reduction threshold 2 [0~255]
	UINT8    pink_rd_th3;					///< pink reduction threshold 3 [0~255]
	UINT8    pink_rd_th4;					///< pink reduction threshold 4 [0~255]
} CTL_IPE_ISP_CFA_PINKR;

typedef enum {
	CTL_IPE_ISP_CGAIN_2_8 = 0,		///< gain range integer/fraction 2.8
	CTL_IPE_ISP_CGAIN_3_7 = 1,		///< gain range integer/fraction 3.7
	ENUM_DUMMY4WORD(CTL_IPE_ISP_CGAIN_RANGE)
} CTL_IPE_ISP_CGAIN_RANGE;

/**
    IPE structure - CFA color gain term
*/
typedef struct {
	UINT16	r_gain;							///< R channel gain [0~1023]
	UINT16	g_gain;							///< G channel gain [0~1023]
	UINT16	b_gain;							///< B channel gain [0~1023]
	CTL_IPE_ISP_CGAIN_RANGE gain_range;		///< select fraction type of gain: 2.8 or 3.7
} CTL_IPE_ISP_CFA_CGAIN;

/**
    IPE structure - IPE CFA parameters.
*/
typedef struct {
	BOOL cfa_enable;						///< enable function
	CTL_IPE_ISP_CFA_INTERP cfa_interp;		///< CFA interpolation term
	CTL_IPE_ISP_CFA_CORR cfa_correction;	///< CFA correction term
	CTL_IPE_ISP_CFA_FCS cfa_fcs;			///< CFA false color suppression term
	CTL_IPE_ISP_CFA_HFC cfa_ir_hfc;			///< CFA high frequency check term
	CTL_IPE_ISP_CFA_IR cfa_ir_sub;			///< CFA IR subtraction term
	CTL_IPE_ISP_CFA_PINKR cfa_pink_reduc;	///< CFA pink reduction term
	CTL_IPE_ISP_CFA_CGAIN cfa_cgain;		///< CFA color gain term
} CTL_IPE_ISP_CFA;

/************************
    IPE structure - IPE 3D color. 538 add
************************/
typedef struct {
	BOOL enable;												///< enable function
	union {
		UINT32 rgb_3d_lut[CTL_IPE_ISP_3DCC_LEN];				///< 3DCC DRAM input address, table size: 729, UINT32
		UINT32 rgb_3d_lut_trans[CTL_IPE_ISP_3DCC_TRANS_LEN];	///< 3DCC DRAM input address (only for kdrv), table size: 912, UINT32
	} lut;
} CTL_IPE_ISP_3DCC;

/************************
    misc structure
************************/
/**
	gamma transformation
*/
typedef struct {
	UINT32 *p_gamma;
	UINT32 *p_r_lut;
	UINT32 *p_g_lut;
	UINT32 *p_b_lut;
	CTL_IPE_ISP_GAMMA_OPTION option;
} CTL_IPP_ISP_GAMMA_TRANS;

/************************
    main structure
************************/
typedef struct {
	CTL_IPE_ISP_EEXT				*p_eext;
	CTL_IPE_ISP_EEXT_TONEMAP		*p_eext_tonemap;
	CTL_IPE_ISP_EDGE_OVERSHOOT		*p_edge_overshoot;
	CTL_IPE_ISP_EPROC				*p_eproc;
	CTL_IPE_ISP_RGBLPF				*p_rgb_lpf;
	CTL_IPE_ISP_PFR					*p_pfr;
	CTL_IPE_ISP_CC					*p_cc;
	CTL_IPE_ISP_CCM					*p_ccm;
	CTL_IPE_ISP_CCTRL				*p_cctrl;
	CTL_IPE_ISP_CADJ_EE				*p_cadj_ee;
	CTL_IPE_ISP_CADJ_YCCON			*p_cadj_yccon;
	CTL_IPE_ISP_CADJ_COFS			*p_cadj_cofs;
	CTL_IPE_ISP_CADJ_RAND			*p_cadj_rand;
	CTL_IPE_ISP_CADJ_HUE			*p_cadj_hue;
	CTL_IPE_ISP_CADJ_FIXTH			*p_cadj_fixth;
	CTL_IPE_ISP_CADJ_MASK			*p_cadj_mask;
	CTL_IPE_ISP_CST					*p_cst;
	CTL_IPE_ISP_CSTP				*p_cstp;
	CTL_IPE_ISP_GAMYRAND			*p_gamy_rand;
	CTL_IPE_ISP_GAMMA				*p_gamma;
	CTL_IPE_ISP_YCURVE				*p_y_curve;
	CTL_IPE_ISP_DEFOG				*p_defog;
	CTL_IPE_ISP_LCE					*p_lce;
	CTL_IPE_ISP_SUBIMG				*p_subimg;
	CTL_IPE_ISP_EDGEDBG				*p_edgedbg;
	CTL_IPE_ISP_VA					*p_va;
	CTL_IPE_ISP_EDGE_REGION_STR		*p_edge_region_str;
	CTL_IPE_ISP_CFA					*p_cfa;
	CTL_IPE_ISP_3DCC				*p_3dcc;
} CTL_IPE_ISP_IQ_ALL;

#endif
/*******************************************************************************
*	                          IPP IQ parameter End                             *
*******************************************************************************/

#endif

