#ifndef _IQ_PARAM_H_
#define _IQ_PARAM_H_

#include "kwrap/type.h"

#define IQ_STAGGER2_PDAF_READY 0

//=============================================================================
// struct & definition
//=============================================================================
// OB
#define IQ_OB_LEN 5
// NR
#define IQ_NR_NRS_LEN 5
#define IQ_NR_OUTL_LEN 5
#define IQ_NR_TH_NUM 6
#define IQ_NR_TH_LUT 17
#define IQ_NR_LCA_NUM 3
// CFA
#define IQ_CFA_FCS_NUM 16
// TONE
#define IQ_TONE_INPUT_YV_BLD_NUM 9
#define IQ_TONE_LEFT_NUM 65
#define IQ_TONE_RIGHT_NUM 17
// GAMMA
#define IQ_GAMMA_LEN 129
// CCM
#define IQ_CCM_LEN 9
#define IQ_CCM_HUETAB_LEN 24
#define IQ_CCM_SATTAB_LEN 24
#define IQ_CCM_INTTAB_LEN 24
// COLOR
#define IQ_COLOR_FSTAB_LEN 16
#define IQ_COLOR_FDTAB_LEN 16
#define IQ_COLOR_CCON_LEN 17
// Contrast
#define IQ_CONTRAST_LCE_LEN 9
// Edge
#define IQ_EDGE_LUT_LEN 16
#define IQ_EDGE_HUE_NUM 24
#define IQ_EDGE_OBJECT_NUM 3

// 3DNR
#define IQ_3DNR_OBJECT_NUM 3
#define IQ_3DNR_TF0_OBJECT_NUM 2
#define IQ_3DNR_PENALTY_LEN 8
#define IQ_3DNR_SWITCH_LEN 8
#define IQ_3DNR_BASE_LEN 8
#define IQ_3DNR_COEFA_LEN 8
#define IQ_3DNR_COEFB_LEN 8
#define IQ_3DNR_STD_LEN 8
#define IQ_3DNR_TH_NUM 2
#define IQ_3DNR_NODE_NUM 2
#define IQ_3DNR_RATIO_NUM 2
#define IQ_3DNR_FREQ_NUM 4
#define IQ_3DNR_F3_LUT_LEN 8
#define IQ_3DNR_F4_LUT_LEN 8
#define IQ_3DNR_LUMA_LEN 8
#define IQ_3DNR_ROI_FINAL_THRESHOLD_TAB 2
// DPC
#define IQ_DPC_MAX_NUM 4096
// Shading
#define IQ_SHADING_ECS_WIN 65
#define IQ_SHADING_ECS_LEN (65*65)
#define IQ_SHADING_VIG_LEN 17
// PFR
#define IQ_PFR_LUMA_LEN 13
// WDR
#define IQ_WDR_FBC_TH_NUM 6
#define IQ_WDR_LEFT_NUM 65
#define IQ_WDR_RIGHT_NUM 17
// WDR_ENH
#define IQ_WDR_STR_LEN 17
// DEFOG
#define IQ_DEFOG_INTERP_DIFF_LEN 17
#define IQ_DEFOG_OUTPUT_BLD_LEN 17
// SHDR
#define IQ_SHDR_NRS_STR_NUM 6
#define IQ_SHDR_FUSION_DIFF_W_NUM 16
#define IQ_SHDR_FCURVE_Y_W_NUM 17
#define IQ_SHDR_FCURVE_LEFT_NUM 65
#define IQ_SHDR_FCURVE_RIGHT_NUM 17
#define IQ_SHDR_FCURVE_END_NUM 16
// COMPANDING
#define IQ_DECOMPANDING_KNEE_NUM 32
#define IQ_COMPANDING_LEFT_NUM 65
#define IQ_COMPANDING_MIDDLE_NUM 17
#define IQ_COMPANDING_RIGHT_NUM 16
// POST_3DNR
#define IQ_POST_3DNR_TH_NUM 3
#define IQ_POST_3DNR_FRAME_W_NUM 3
#define IQ_POST_3DNR_RANGE_W_NUM 3
// POST_SHARPEN
#define IQ_POST_SHARPEN_CURVE_NUM 17
// CST
#define IQ_CST_LEN 9
// YCURVE
#define IQ_YCURVE_LEN 129
// 3DCC
#define IQ_3DCC_LEN (9*9*9)
// BNR, 539A only
#define IQ_BNR_ERR_COMP_L_MAX 65
#define IQ_BNR_ERR_COMP_R_MAX 17
#define IQ_BNR_MD_TH_L 16
#define IQ_BNR_MD_TH_R 8
// AIISP
#define AIISP_PARAM_MAX 32

/**
	IQ process id
*/
typedef enum _IQ_ID {
	IQ_ID_1 = 0,                      ///< iq id 1
	IQ_ID_2,                          ///< iq id 2
	IQ_ID_3,                          ///< iq id 3
	IQ_ID_4,                          ///< iq id 4
	IQ_ID_5,                          ///< iq id 5
	IQ_ID_6,                          ///< iq id 6, IPP only
	IQ_ID_7,                          ///< iq id 7, IPP only
	IQ_ID_8,                          ///< iq id 8, IPP only
	IQ_ID_9,                          ///< iq id 9, IPP only
	IQ_ID_MAX_NUM,
	ENUM_DUMMY4WORD(IQ_ID)
} IQ_ID;

typedef enum _IQ_OP_TYPE {
	IQ_OP_TYPE_AUTO = 0,              ///< iq setting from manual_param
	IQ_OP_TYPE_MANUAL,                ///< iq setting from auto_param, interpolation by gain, LV or CT
	IQ_OP_TYPE_MAX,
	ENUM_DUMMY4WORD(IQ_OP_TYPE)
} IQ_OP_TYPE;

typedef enum _IQ_GAIN_ID {
	IQ_GAIN_1X = 0,                   ///< iq gain 1X
	IQ_GAIN_2X,                       ///< iq gain 2X
	IQ_GAIN_4X,                       ///< iq gain 4X
	IQ_GAIN_8X,                       ///< iq gain 8X
	IQ_GAIN_16X,                      ///< iq gain 16X
	IQ_GAIN_32X,                      ///< iq gain 32X
	IQ_GAIN_64X,                      ///< iq gain 64X
	IQ_GAIN_128X,                     ///< iq gain 128X
	IQ_GAIN_256X,                     ///< iq gain 256X
	IQ_GAIN_512X,                     ///< iq gain 512X
	IQ_GAIN_1024X,                    ///< iq gain 1024X
	IQ_GAIN_2048X,                    ///< iq gain 2048X
	IQ_GAIN_4096X,                    ///< iq gain 4096X
	IQ_GAIN_8192X,                    ///< iq gain 8192X
	IQ_GAIN_16384X,                   ///< iq gain 16384X
	IQ_GAIN_32768X,                   ///< iq gain 32768X
	IQ_GAIN_ID_MAX_NUM,
	ENUM_DUMMY4WORD(IQ_GAIN_ID)
} IQ_GAIN_ID;

typedef enum _IQ_COLOR_ID {
	IQ_COLOR_TEMPERATURE_0 = 0,       ///< iq color temperature high
	IQ_COLOR_TEMPERATURE_1,
	IQ_COLOR_TEMPERATURE_2,
	IQ_COLOR_TEMPERATURE_3,
	IQ_COLOR_TEMPERATURE_4,           ///< iq color temperature low
	IQ_COLOR_ID_MAX_NUM,
	ENUM_DUMMY4WORD(IQ_COLOR_ID)
} IQ_COLOR_ID;

typedef enum _IQ_TONE_AUTO_SEL {
	IQ_TONE_AUTO_BY_TONE_LEVEL = 1,
	IQ_TONE_AUTO_BY_AE_LV,
	ENUM_DUMMY4WORD(IQ_TONE_AUTO_SEL)
} IQ_TONE_AUTO_SEL;

typedef enum _IQ_TONE_ID {
	IQ_TONE_SET0 = 0,
	IQ_TONE_SET1,
	IQ_TONE_SET2,
	IQ_TONE_SET3,
	IQ_TONE_SET4,
	IQ_TONE_ID_MAX_NUM,
	ENUM_DUMMY4WORD(IQ_TONE_ID)
} IQ_TONE_ID;

typedef enum _IQ_GAMMA_AUTO_SEL {
	IQ_GAMMA_AUTO_BY_GAMMA_LEVEL = 1,
	IQ_GAMMA_AUTO_BY_AE_LV,
	ENUM_DUMMY4WORD(IQ_GAMMA_AUTO_SEL)
} IQ_GAMMA_AUTO_SEL;

typedef enum _IQ_GAMMA_ID {
	IQ_GAMMA_SET0 = 0,
	IQ_GAMMA_SET1,
	IQ_GAMMA_SET2,
	IQ_GAMMA_SET3,
	IQ_GAMMA_SET4,
	IQ_GAMMA_ID_MAX_NUM,
	ENUM_DUMMY4WORD(IQ_GAMMA_ID)
} IQ_GAMMA_ID;

typedef enum _IQ_RGBIR_LIGHT_ID {
	IQ_RGBIR_LIGHT_D = 0,                   ///< iq light D65
	IQ_RGBIR_LIGHT_A,                       ///< iq light A
	IQ_RGBIR_LIGHT_IR,                      ///< iq light IR
	IQ_RGBIR_LIGHT_ID_MAX_NUM,
	ENUM_DUMMY4WORD(IQ_RGBIR_LIGHT_ID)
} IQ_RGBIR_LIGHT_ID;

typedef enum _IQ_ECS_ID {
	IQ_ECS_TEMPERATURE_H = 0,       ///< iq ECS temperature high
	IQ_ECS_TEMPERATURE_M,           ///< iq ECS temperature middle
	IQ_ECS_TEMPERATURE_L,           ///< iq ECS temperature low
	IQ_ECS_ID_MAX_NUM,
	ENUM_DUMMY4WORD(IQ_ECS_ID)
} IQ_ECS_ID;

typedef enum IQ_FLOW_MODE {
	IQ_FLOW_LINEAR,
	IQ_FLOW_SHDR,
	ENUM_DUMMY4WORD(IQ_FLOW_MODE)
} IQ_FLOW_MODE;

typedef enum IQ_PROC_MODE {
	IQ_PROC_MOVIE,
	IQ_PROC_PHOTO,
	IQ_PROC_CAPTURE,
	ENUM_DUMMY4WORD(IQ_PROC_MODE)
} IQ_PROC_MODE;

typedef enum _IQ_OB_MODE {
	IQ_OB_SIE,
	IQ_OB_PRE_F,                      ///< 539A only
	IQ_OB_PRE,                        ///< 539A only
	IQ_OB_IFE_F,
	IQ_OB_IFE,
	ENUM_DUMMY4WORD(IQ_OB_MODE)
} IQ_OB_MODE;

typedef enum _IQ_DG_MODE {
	IQ_DG_OFF,
	IQ_DG_SIE,
	IQ_DG_PRE,                        ///< 539A only
	IQ_DG_IFE_F,
	IQ_DG_IFE,
	IQ_DG_IPE,
	ENUM_DUMMY4WORD(IQ_DG_MODE)
} IQ_DG_MODE;

typedef enum _IQ_CG_MODE {
	IQ_CG_SIE,
	IQ_CG_PRE_F,                      ///< 539A only
	IQ_CG_PRE,                        ///< 539A only
	IQ_CG_IFE_F,
	IQ_CG_IFE,
	IQ_CG_IPE,
	ENUM_DUMMY4WORD(IQ_CG_MODE)
} IQ_CG_MODE;

/**
	All Structs for IQTOOL
*/
// Struct of IQ Parameter
typedef struct _IQ_OB_TUNE_PARAM {
	UINT32 cofs[IQ_OB_LEN];                     ///< range : 0~4095, ob offset
} IQ_OB_TUNE_PARAM;

typedef struct _IQ_OB_PARAM {
	BOOL enable;                                ///< enable function
	IQ_OP_TYPE mode;
	IQ_OB_TUNE_PARAM manual_param;
	IQ_OB_TUNE_PARAM auto_param[IQ_GAIN_ID_MAX_NUM];
} IQ_OB_PARAM;

typedef enum _IQ_NR_OUTL_SEL {
	IQ_NR_OUTL_8_NODE,
	IQ_NR_OUTL_7_NODE,
	ENUM_DUMMY4WORD(IQ_NR_OUTL_SEL)
} IQ_NR_OUTL_SEL;

typedef struct _IQ_NR_TUNE_PARAM {
	UINT16 outl_ord_protect_th;                 ///< range : 0~1023, order method protect artifact th
	UINT8 outl_ord_blend_w;                     ///< range : 0~255, outlier and order result blending weight, 0: order, 255: outlier
	BOOL outl_avg_mode;                         ///< range : 0~1, outlier use direction average or not
	IQ_NR_OUTL_SEL outl_sel;
	UINT16 outl_bright_th[IQ_NR_OUTL_LEN];      ///< range : 0~4095
	UINT16 outl_dark_th[IQ_NR_OUTL_LEN];        ///< range : 0~4095
	UINT16 gbal_diff_th_str;                    ///< range : 0~4095, gbal strength
	UINT16 gbal_edge_protect_th;                ///< range : 0~4095, edge judgement th
	UINT16 gbal_str_luma_low_bnd;               ///< range : 0~4095, gbal strength luma low boundary
	UINT16 gbal_edge_luma_low_bnd;              ///< range : 0~4095, edge judgement th luma low boundary
	UINT16 filter_th[IQ_NR_TH_NUM];             ///< range : 0~1023, range filter threshold
	UINT16 filter_lut[IQ_NR_TH_LUT];            ///< range : 0~1023, range filter threshold adjustment
	UINT16 filter_th_b[IQ_NR_TH_NUM];           ///< range : 0~1023, range filter threshold
	UINT16 filter_lut_b[IQ_NR_TH_LUT];          ///< range : 0~1023, range filter threshold adjustment
	UINT8 filter_blend_w;                       ///< range : 0~15, range A and B weighting
	UINT16 filter_clamp_th;                     ///< range : 0~4095, threshold
	UINT8 filter_clamp_mul;                     ///< range : 0~255, 1X = 128, weighting multiplier
	UINT32 lca_edge_th;                         ///< range : 0~131071, edge thresholds to decides smooth, transition and edge regions
	UINT8 lca_y_filter_level[IQ_NR_LCA_NUM];    ///< range : 0~255, Range filter strength for luma
	UINT8 lca_y_coring_gain[IQ_NR_LCA_NUM];     ///< range : 0~32, LCA Y channel coring gain for still-edge, still-smooth and motion regions
	UINT8 lca_y_coring_cutoff[IQ_NR_LCA_NUM];   ///< range : 0~63, LCA Y channel coring cutoff threshold for still-edge, still-smooth and motion regions
	UINT8 lca_c_filter_level[IQ_NR_LCA_NUM];    ///< range : 0~255, Range filter strength for chroma
	UINT8 lca_c_coring_gain[IQ_NR_LCA_NUM];     ///< range : 0~32, LCA C channel coring gain for still-edge, still-smooth and motion regions
	UINT8 lca_c_coring_cutoff[IQ_NR_LCA_NUM];   ///< range : 0~63, LCA C channel coring cutoff threshold for still-edge, still-smooth and motion regions
	UINT32 dbcs_step_y;                         ///< range : 0~3, Step for luma (0:0~16 / 1:0~32 / 2:0~64 / 3:0~128)
	UINT32 dbcs_step_c;                         ///< range : 0~3, Step for chroma (0:-16~16 / 1:-32~32 / 2:-64~64 / 3:-128~128)
} IQ_NR_TUNE_PARAM;

// 539A only
typedef struct _IQ_NR_EXT_TUNE_PARAM {
	UINT16 filter_th_1[IQ_NR_TH_NUM];           ///< range : 0~1023, range filter threshold
	UINT16 filter_lut_1[IQ_NR_TH_LUT];          ///< range : 0~1023, range filter threshold adjustment
	UINT16 filter_th_b_1[IQ_NR_TH_NUM];         ///< range : 0~1023, range filter threshold
	UINT16 filter_lut_b_1[IQ_NR_TH_LUT];        ///< range : 0~1023, range filter threshold adjustment
	UINT16 filter_th_2[IQ_NR_TH_NUM];           ///< range : 0~1023, range filter threshold
	UINT16 filter_lut_2[IQ_NR_TH_LUT];          ///< range : 0~1023, range filter threshold adjustment
	UINT16 filter_th_b_2[IQ_NR_TH_NUM];         ///< range : 0~1023, range filter threshold
	UINT16 filter_lut_b_2[IQ_NR_TH_LUT];        ///< range : 0~1023, range filter threshold adjustment
} IQ_NR_EXT_TUNE_PARAM;

typedef struct _IQ_NR_PARAM {
	BOOL outl_enable;                           ///< enable function
	BOOL gbal_enable;                           ///< enable function
	BOOL filter_enable;                         ///< enable function
	BOOL lca_enable;                            ///< enable function
	BOOL dbcs_enable;                           ///< enable function
	IQ_OP_TYPE mode;
	IQ_NR_TUNE_PARAM manual_param;
	IQ_NR_TUNE_PARAM auto_param[IQ_GAIN_ID_MAX_NUM];
	UINT8 dbcs_proc_location;                   ///< processing location, 539A only
	IQ_NR_EXT_TUNE_PARAM ext_manual_param;      ///< 539A only
	IQ_NR_EXT_TUNE_PARAM ext_auto_param[IQ_GAIN_ID_MAX_NUM]; ///< 539A only
} IQ_NR_PARAM;

typedef struct _IQ_CFA_TUNE_PARAM {
	UINT16 edge_dth;                            ///< range : 0~4095, Edge threshold 1 for Bayer CFA interpolation
	UINT16 edge_dth2;                           ///< range : 0~4095, Edge threshold 2 for Bayer CFA interpolation
	UINT16 freq_th;                             ///< range : 0~4095, CFA frequency threshold
	UINT8 fcs_weight;                           ///< range : 0~255, Global strength for false color suppression
	UINT8 fcs_strength[IQ_CFA_FCS_NUM];         ///< range : 0~15, CFA false color suppression strength
} IQ_CFA_TUNE_PARAM;

typedef struct _IQ_CFA_PARAM {
	IQ_OP_TYPE mode;
	IQ_CFA_TUNE_PARAM manual_param;
	IQ_CFA_TUNE_PARAM auto_param[IQ_GAIN_ID_MAX_NUM];
} IQ_CFA_PARAM;

typedef enum _IQ_VA_H_FILTER_SEL {
	IQ_VA_H_FILTER_FIR = 0,
	IQ_VA_H_FILTER_IIR = 1,
} IQ_VA_H_FILTER_SEL;

typedef enum _IQ_VA_FILTER_SYM_SEL {
	IQ_VA_FILTER_SYM_MIRROR = 0,
	IQ_VA_FILTER_SYM_INVERSE = 1,
} IQ_VA_FILTER_SYM_SEL;

typedef enum _IQ_VA_IIR_FILTER_SET {
	IQ_VA_IIR_SET_FREQ_H,  // 0.12~0.24
	IQ_VA_IIR_SET_FREQ_M,  // 0.04~0.12
	IQ_VA_IIR_SET_FREQ_L,  // 0.02~0.08
	IQ_VA_IIR_SET_FREQ_L2, // 0.02~0.04
	IQ_VA_IIR_SET_MAX_NUM,
} IQ_VA_IIR_FILTER_SET;

typedef enum _IQ_VA_CNT_OUTSEL {
	IQ_VA_ENERGY_COUNT,
	IQ_VA_HIGH_LUMA_COUNT,
} IQ_VA_CNT_OUTSEL;

typedef struct _IQ_RAW_VA_MANUAL_PARAM {
	INT16 g1_iir1_tap_a;                        ///< range : -511~511, grope1 IIR1 filter coefficent A
	INT16 g1_iir1_tap_b;                        ///< range : -511~511, grope1 IIR1 filter coefficent B
	INT16 g1_iir1_tap_e;                        ///< range : -511~511, grope1 IIR1 filter coefficent E
	INT16 g1_iir1_tap_f;                        ///< range : -511~511, grope1 IIR1 filter coefficent F
	UINT8 g1_iir1_shift_bit;                    ///< range : 0~13, grope1 IIR1 filter normalized term
	INT16 g1_iir2_tap_a;                        ///< range : -511~511, grope1 IIR2 filter coefficent A
	INT16 g1_iir2_tap_b;                        ///< range : -511~511, grope1 IIR2 filter coefficent B
	INT16 g1_iir2_tap_e;                        ///< range : -511~511, grope1 IIR2 filter coefficent E
	INT16 g1_iir2_tap_f;                        ///< range : -511~511, grope1 IIR2 filter coefficent F
	UINT8 g1_iir2_shift_bit;                    ///< range : 0~15, grope1 IIR2 filter normalized term
	INT16 g1_iir3_tap_a;                        ///< range : -511~511, grope1 IIR3 filter coefficent A
	INT16 g1_iir3_tap_b;                        ///< range : -511~511, grope1 IIR3 filter coefficent B
	INT16 g1_iir3_tap_e;                        ///< range : -511~511, grope1 IIR3 filter coefficent E
	INT16 g1_iir3_tap_f;                        ///< range : -511~511, grope1 IIR3 filter coefficent F
	UINT8 g1_iir3_shift_bit;                    ///< range : 0~15, grope1 IIR3 filter normalized term
	UINT16 g1_th_l;                             ///< range : 0~255, grope1 lower threshold
	UINT16 g1_th_u;                             ///< range : 0~255, grope1 upper threshold
	INT16 g2_iir1_tap_a;                        ///< range : -511~511, grope2 IIR1 filter coefficent A
	INT16 g2_iir1_tap_b;                        ///< range : -511~511, grope2 IIR1 filter coefficent B
	INT16 g2_iir1_tap_e;                        ///< range : -511~511, grope2 IIR1 filter coefficent E
	INT16 g2_iir1_tap_f;                        ///< range : -511~511, grope2 IIR1 filter coefficent F
	UINT8 g2_iir1_shift_bit;                    ///< range : 0~13, grope2 IIR1 filter normalized term
	INT16 g2_iir2_tap_a;                        ///< range : -511~511, grope2 IIR2 filter coefficent A
	INT16 g2_iir2_tap_b;                        ///< range : -511~511, grope2 IIR2 filter coefficent B
	INT16 g2_iir2_tap_e;                        ///< range : -511~511, grope2 IIR2 filter coefficent E
	INT16 g2_iir2_tap_f;                        ///< range : -511~511, grope2 IIR2 filter coefficent F
	UINT8 g2_iir2_shift_bit;                    ///< range : 0~15, grope2 IIR2 filter normalized term
	INT16 g2_iir3_tap_a;                        ///< range : -511~511, grope2 IIR3 filter coefficent A
	INT16 g2_iir3_tap_b;                        ///< range : -511~511, grope2 IIR3 filter coefficent B
	INT16 g2_iir3_tap_e;                        ///< range : -511~511, grope2 IIR3 filter coefficent E
	INT16 g2_iir3_tap_f;                        ///< range : -511~511, grope2 IIR3 filter coefficent F
	UINT8 g2_iir3_shift_bit;                    ///< range : 0~15, grope2 IIR3 filter normalized term
	UINT16 g2_th_l;                             ///< range : 0~255, grope2 lower threshold
	UINT16 g2_th_u;                             ///< range : 0~255, grope2 upper threshold
} IQ_RAW_VA_MANUAL_PARAM;

typedef struct _IQ_RAW_VA_AUTO_PARAM {
	IQ_VA_IIR_FILTER_SET g1_iir;
	UINT16 g1_th_l;                             ///< range : 0~255, grope1 lower threshold
	UINT16 g1_th_u;                             ///< range : 0~255, grope1 upper threshold
	IQ_VA_IIR_FILTER_SET g2_iir;
	UINT16 g2_th_l;                             ///< range : 0~255, grope2 lower threshold
	UINT16 g2_th_u;                             ///< range : 0~255, grope2 upper threshold
} IQ_RAW_VA_AUTO_PARAM;

typedef struct _IQ_RAW_VA_PARAM {
	BOOL pre_filter_enable;                     ///< enable function
	IQ_OP_TYPE mode;

	IQ_VA_H_FILTER_SEL g1_h_filter_sel;
	IQ_VA_FILTER_SYM_SEL g1_fir_sym_sel;
	INT16 g1_fir_tap_a;                         ///< range : 0~31, grope1 filter coefficent A
	INT16 g1_fir_tap_b;                         ///< range : -16~15, grope1 filter coefficent B
	INT16 g1_fir_tap_c;                         ///< range : -8~7, grope1 filter coefficent C
	INT16 g1_fir_tap_d;                         ///< range : -8~7, grope1 filter coefficent D
	UINT8 g1_fir_div;                           ///< range : 0~15, grope1 filter normalized term

	IQ_VA_H_FILTER_SEL g2_h_filter_sel;
	IQ_VA_FILTER_SYM_SEL g2_fir_sym_sel;
	INT16 g2_fir_tap_a;                         ///< range : -511~511, grope2 filter coefficent A
	INT16 g2_fir_tap_b;                         ///< range : -511~511, grope2 filter coefficent B
	INT16 g2_fir_tap_c;                         ///< range : -511~511, grope2 filter coefficent C
	INT16 g2_fir_tap_d;                         ///< range : -511~511, grope2 filter coefficent D
	UINT8 g2_fir_div;                           ///< range : 0~15, grope2 filter normalized term

	UINT8 ldg_low_th;                           ///< range : 0~255, Darkness threshold
	UINT8 ldg_high_th;                          ///< range : 0~255, Brightness threshold
	UINT8 ldg_low_gain;                         ///< range : 0~255, Dark region minima gain
	UINT8 ldg_high_gain;                        ///< range : 0~255, Bright region minima gain
	UINT8 ldg_low_slope;                        ///< range : 0~15, Dark region gain slope
	UINT8 ldg_high_slope;                       ///< range : 0~15, Bright region gain slope
	UINT8 energy_w;                             ///< range : 0~16, 0:Square output; 16: Linear output
	IQ_VA_CNT_OUTSEL win_cnt_out_sel;
	UINT8 high_luma_th;                         ///< range : 0~255, va count output brightness extends high_luma_th
	IQ_RAW_VA_MANUAL_PARAM manual_param;
	IQ_RAW_VA_AUTO_PARAM auto_param[IQ_GAIN_ID_MAX_NUM];
} IQ_RAW_VA_PARAM;

typedef struct _IQ_VA_TUNE_PARAM {
	UINT16 g1_th_l;                             ///< range : 0~255, grope1 lower threshold
	UINT16 g1_th_u;                             ///< range : 0~255, grope1 upper threshold
	UINT16 g2_th_l;                             ///< range : 0~255, grope2 lower threshold
	UINT16 g2_th_u;                             ///< range : 0~255, grope2 upper threshold
} IQ_VA_TUNE_PARAM;

typedef struct _IQ_VA_PARAM {
	BOOL pre_filter_enable;                     ///< enable function
	IQ_OP_TYPE mode;
	IQ_VA_FILTER_SYM_SEL g1_sym_sel;
	UINT8 g1_tap_a;                             ///< range : 0~31, grope1 filter coefficent A
	INT8 g1_tap_b;                              ///< range : -16~15, grope1 filter coefficent B
	INT8 g1_tap_c;                              ///< range : -8~7, grope1 filter coefficent C
	INT8 g1_tap_d;                              ///< range : -8~7, grope1 filter coefficent D
	UINT8 g1_div;                               ///< range : 0~15, grope1 filter normalized term
	IQ_VA_FILTER_SYM_SEL g2_sym_sel;
	UINT8 g2_tap_a;                             ///< range : 0~31, grope2 filter coefficent A
	INT8 g2_tap_b;                              ///< range : -16~15, grope2 filter coefficent B
	INT8 g2_tap_c;                              ///< range : -8~7, grope2 filter coefficent C
	INT8 g2_tap_d;                              ///< range : -8~7, grope2 filter coefficent D
	UINT8 g2_div;                               ///< range : 0~15, grope2 filter normalized term
	UINT8 ldg_low_th;                           ///< range : 0~255, Darkness threshold
	UINT8 ldg_high_th;                          ///< range : 0~255, Brightness threshold
	UINT8 ldg_low_gain;                         ///< range : 0~255, Dark region minima gain
	UINT8 ldg_high_gain;                        ///< range : 0~255, Bright region minima gain
	UINT8 ldg_low_slope;                        ///< range : 0~15, Dark region gain slope
	UINT8 ldg_high_slope;                       ///< range : 0~15, Bright region gain slope
	UINT8 energy_w;                             ///< range : 0~16, 0:Square output; 16: Linear output
	IQ_VA_CNT_OUTSEL win_cnt_out_sel;
	UINT8 high_luma_th;                         ///< range : 0~255, va count output brightness extends high_luma_th
	IQ_VA_TUNE_PARAM manual_param;
	IQ_VA_TUNE_PARAM auto_param[IQ_GAIN_ID_MAX_NUM];
} IQ_VA_PARAM;

typedef struct _IQ_TONE_AUTO_PARAM {
	UINT32 lv;                                                        ///< range : 0~20
	UINT32 tone_level;                                                ///< range : 0~100
} IQ_TONE_AUTO_PARAM;

typedef struct _IQ_TONE_PARAM {
	BOOL enable;                                                      ///< enable function
	IQ_OP_TYPE mode;
	IQ_TONE_AUTO_SEL auto_sel;
	UINT8 tone_in_yv_blend_lut[IQ_TONE_INPUT_YV_BLD_NUM];             ///< range : 0~63, blending of Y and V, 0: Y, 63: V
	UINT16 manual_lut_left[IQ_TONE_LEFT_NUM];                         ///< range : 0~4095, tone curve table value
	UINT16 manual_lut_right[IQ_TONE_RIGHT_NUM];                       ///< range : 0~4095, tone curve table value
	UINT32 auto_set0_level;                                           ///< range : 0~100
	UINT16 auto_set0_lut_left[IQ_TONE_LEFT_NUM];                      ///< range : 0~4095, tone curve table value
	UINT16 auto_set0_lut_right[IQ_TONE_RIGHT_NUM];                    ///< range : 0~4095, tone curve table value
	UINT32 auto_set1_level;                                           ///< range : 0~100
	UINT16 auto_set1_lut_left[IQ_TONE_LEFT_NUM];                      ///< range : 0~4095, tone curve table value
	UINT16 auto_set1_lut_right[IQ_TONE_RIGHT_NUM];                    ///< range : 0~4095, tone curve table value
	UINT32 auto_set2_level;                                           ///< range : 0~100
	UINT16 auto_set2_lut_left[IQ_TONE_LEFT_NUM];                      ///< range : 0~4095, tone curve table value
	UINT16 auto_set2_lut_right[IQ_TONE_RIGHT_NUM];                    ///< range : 0~4095, tone curve table value
	UINT32 auto_set3_level;                                           ///< range : 0~100
	UINT16 auto_set3_lut_left[IQ_TONE_LEFT_NUM];                      ///< range : 0~4095, tone curve table value
	UINT16 auto_set3_lut_right[IQ_TONE_RIGHT_NUM];                    ///< range : 0~4095, tone curve table value
	UINT32 auto_set4_level;                                           ///< range : 0~100
	UINT16 auto_set4_lut_left[IQ_TONE_LEFT_NUM];                      ///< range : 0~4095, tone curve table value
	UINT16 auto_set4_lut_right[IQ_TONE_RIGHT_NUM];                    ///< range : 0~4095, tone curve table value
	IQ_TONE_AUTO_PARAM auto_param[IQ_TONE_ID_MAX_NUM];
} IQ_TONE_PARAM;

typedef struct _IQ_GAMMA_AUTO_PARAM {
	UINT32 lv;                                                        ///< range : 0~20
	UINT32 gamma_level;                                               ///< range : 0~100
} IQ_GAMMA_AUTO_PARAM;

typedef struct _IQ_GAMMA_PARAM {
	BOOL enable;                                                      ///< enable function
	IQ_OP_TYPE mode;
	IQ_GAMMA_AUTO_SEL auto_sel;
	UINT32 manual_lut[IQ_GAMMA_LEN];                                  ///< range : 0~1023, gamma input, table size: 129
	UINT32 auto_set0_level;                                           ///< range : 0~100
	UINT32 auto_set0_lut[IQ_GAMMA_LEN];                               ///< range : 0~1023, gamma input, table size: 129
	UINT32 auto_set1_level;                                           ///< range : 0~100
	UINT32 auto_set1_lut[IQ_GAMMA_LEN];                               ///< range : 0~1023, gamma input, table size: 129
	UINT32 auto_set2_level;                                           ///< range : 0~100
	UINT32 auto_set2_lut[IQ_GAMMA_LEN];                               ///< range : 0~1023, gamma input, table size: 129
	UINT32 auto_set3_level;                                           ///< range : 0~100
	UINT32 auto_set3_lut[IQ_GAMMA_LEN];                               ///< range : 0~1023, gamma input, table size: 129
	UINT32 auto_set4_level;                                           ///< range : 0~100
	UINT32 auto_set4_lut[IQ_GAMMA_LEN];                               ///< range : 0~1023, gamma input, table size: 129
	IQ_GAMMA_AUTO_PARAM auto_param[IQ_GAMMA_ID_MAX_NUM];
} IQ_GAMMA_PARAM;

typedef struct _IQ_CCM_MANUAL_PARAM {
	INT16 coef[IQ_CCM_LEN];                     ///< range : -1024~1023, 1X = 256, Color correction matrix [rr rg rb gr gg gb br bg bb], table size: 9
	UINT8 hue_tab[IQ_CCM_HUETAB_LEN];           ///< range : 0~255, color control hue adjust table, 128 : NO hue adjust, table size: 24
	INT8 sat_tab[IQ_CCM_SATTAB_LEN];            ///< range : -128~127, color control sat adjust table, 0 : NO sat adjust, -128 : Min sat suppress, 127 : Max sat enhance, table size: 24
	INT8 int_tab[IQ_CCM_INTTAB_LEN];            ///< range : -128~127, color control int adjust table, 0 : NO int adjust, -128 : Min int suppress, 127 : Max int enhance, table size: 24
} IQ_CCM_MANUAL_PARAM;

typedef struct _IQ_CCM_AUTO_PARAM {
	UINT32 ct;                                  ///< range : 1000~12000
	INT16 coef[IQ_CCM_LEN];                     ///< range : -1024~1023, 1X = 256, Color correction matrix [rr rg rb gr gg gb br bg bb], table size: 9
	UINT8 hue_tab[IQ_CCM_HUETAB_LEN];           ///< range : 0~255, color control hue adjust table, 128 : NO hue adjust, table size: 24
	INT8 sat_tab[IQ_CCM_SATTAB_LEN];            ///< range : -128~127, color control sat adjust table, 0 : NO sat adjust, -128 : Min sat suppress, 127 : Max sat enhance, table size: 24
	INT8 int_tab[IQ_CCM_INTTAB_LEN];            ///< range : -128~127, color control int adjust table, 0 : NO int adjust, -128 : Min int suppress, 127 : Max int enhance, table size: 24
} IQ_CCM_AUTO_PARAM;

typedef struct _IQ_CCM_PARAM {
	BOOL enable;                                ///< enable function
	IQ_OP_TYPE mode;
	IQ_CCM_MANUAL_PARAM manual_param;
	IQ_CCM_AUTO_PARAM auto_param[IQ_COLOR_ID_MAX_NUM];
} IQ_CCM_PARAM;

typedef struct _IQ_COLOR_TUNE_PARAM {
	UINT8 c_con;                                ///< range : 0~255, CbCr contrast adjust, 128 : NO contrst adjust, C' = (C * CCon) >> 7
	UINT8 fstab[IQ_COLOR_FSTAB_LEN];            ///< range : 0~255, Color correction stab, table size: 16
	UINT8 fdtab[IQ_COLOR_FDTAB_LEN];            ///< range : 0~255, Color correction dtab, table size: 16
	UINT16 cconlut[IQ_COLOR_CCON_LEN];          ///< range : 0~1023, CbCr contrast adjust, 128 : NO contrst adjust, C' = (C * CCon) >> 7
} IQ_COLOR_TUNE_PARAM;

typedef struct _IQ_COLOR_PARAM {
	BOOL enable;                                ///< enable function
	IQ_OP_TYPE mode;
	IQ_COLOR_TUNE_PARAM manual_param;
	IQ_COLOR_TUNE_PARAM auto_param[IQ_GAIN_ID_MAX_NUM];
} IQ_COLOR_PARAM;

typedef struct _IQ_CONTRAST_TUNE_PARAM {
	UINT8 y_con;                                ///< range : 0~255, Y contrast adjust, 128 : NO contrst adjust, Y' = (Y * Ycon) >> 7
	UINT8 lce_lum_wt_lut[IQ_CONTRAST_LCE_LEN];  ///< range : 0~191, 0 = OFF(1X), 64 = 2x, Lce luma weighting table
} IQ_CONTRAST_TUNE_PARAM;

typedef struct _IQ_CONTRAST_PARAM {
	BOOL enable;                                ///< enable function
	BOOL lce_enable;                            ///< enable function
	IQ_OP_TYPE mode;
	IQ_CONTRAST_TUNE_PARAM manual_param;
	IQ_CONTRAST_TUNE_PARAM auto_param[IQ_GAIN_ID_MAX_NUM];
} IQ_CONTRAST_PARAM;

typedef struct _IQ_EDGE_TUNE_PARAM {
	UINT32 edge_enh_p;                          ///< range : 0~1023, 1X = 64, Positive edge enhance weighting
	UINT32 edge_enh_n;                          ///< range : 0~1023, 1X = 64, Negative edge enhance weighting
	UINT32 thin_freq;                           ///< range : 0~16
	UINT32 robust_freq;                         ///< range : 0~16
	UINT32 wt_low;                              ///< range : 0~16, the kernel weighting of flat region
	UINT32 wt_high;                             ///< range : 0~16, the kernel weighting of edge region
	UINT32 th_flat_low;                         ///< range : 0~1023, the threshold for flat region
	UINT32 th_flat_high;                        ///< range : 0~1023, the threshold for flat region
	UINT32 th_edge_low;                         ///< range : 0~1023, the threshold for edge region
	UINT32 th_edge_high;                        ///< range : 0~1023, the threshold for edge region
	UINT8 str_flat;                             ///< range : 0~64, 1X = 64, edge enhance of flat region
	UINT8 str_edge;                             ///< range : 64~255, 1X = 64, edge enhance of edge region
	UINT32 overshoot_str;                       ///< range : 0~32767 (256, slope = 1), the slope of the overshooting weight curve,
	UINT32 undershoot_str;                      ///< range : 0~32767 (256, slope = 1), the slope of the undershooting weight curve
	UINT32 edge_ethr_low;                       ///< range : 0~1023, for edge mapping, if(Ein < EthrA){Eout=EDTabA[0]}, else if(EthrA <= Ein <EthrB){Eout apply EtabA and EDtabA}, else if(Ein >= EthrB){Eout apply EtabB and EDtabB}
	UINT32 edge_ethr_high;                      ///< range : 0~1023, for edge mapping, if(Ein < EthrA){Eout=EDTabA[0]}, else if(EthrA <= Ein <EthrB){Eout apply EtabA and EDtabA}, else if(Ein >= EthrB){Eout apply EtabB and EDtabB}
	UINT32 edge_etab_low;                       ///< range : 0~6, for edge mapping, EDtabA table has 8 sections, each section width is (1 << etaA)
	UINT32 edge_etab_high;                      ///< range : 0~6, for edge mapping, EDtabB table has 8 sections, each section width is (1 << etaB)
	UINT32 es_ethr_low;                         ///< range : 0~1023, for edge mapping, if(Ein < EthrA){Eout=EDTabA[0]}, else if(EthrA <= Ein <EthrB){Eout apply EtabA and EDtabA}, else if(Ein >= EthrB){Eout apply EtabB and EDtabB}
	UINT32 es_ethr_high;                        ///< range : 0~1023, for edge mapping, if(Ein < EthrA){Eout=EDTabA[0]}, else if(EthrA <= Ein <EthrB){Eout apply EtabA and EDtabA}, else if(Ein >= EthrB){Eout apply EtabB and EDtabB}
	UINT32 es_etab_low;                         ///< range : 0~6, for edge mapping, EDtabA table has 8 sections, each section width is (1 << etaA)
	UINT32 es_etab_high;                        ///< range : 0~6, for edge mapping, EDtabB table has 8 sections, each section width is (1 << etaB)
	UINT8 dir_eng_blend_w;                      ///< range : 0~16 Dir energy and non-Dir energy blending ratio
} IQ_EDGE_TUNE_PARAM;

// 539A only
typedef struct _IQ_EDGE_EXT_TUNE_PARAM {
	UINT8 motion_str[IQ_EDGE_OBJECT_NUM];
} IQ_EDGE_EXT_TUNE_PARAM;

typedef struct _IQ_EDGE_PARAM {
	BOOL enable;                                ///< enable function
	IQ_OP_TYPE mode;
	UINT32 th_overshoot;                        ///< range : 0~255, the threshold for overshooting
	UINT32 th_undershoot;                       ///< range : 0~255, the threshold for undershooting
	UINT8 blending_th;                          ///< range : 0~255, for edge kernel blending, low luma region threshold
	UINT8 blending_low_luma_w;                  ///< range : 0~255, for edge kernel blending, low luma region weight. 0: pre edge gamma; 255: post edge gamma
	UINT8 blending_high_luma_w;                 ///< range : 0~255, for edge kernel blending, high luma region weight. 0: pre edge gamma; 255: post edge gamma
	UINT8 edge_map_lut[IQ_EDGE_LUT_LEN];        ///< range : 0~255, 1X = 255, table size: 16, ED map LUT(16 entries).
	UINT8 es_map_lut[IQ_EDGE_LUT_LEN];          ///< range : 0~255, 1X = 64, table size: 16, ES map LUT(16 entries).
	UINT8 edge_tab[IQ_EDGE_HUE_NUM];            ///< range : 0~255, color control edge adjust table, 128 : NO edge adjust, table size: 24
	UINT32 reduce_ratio;                        ///< range : 0~128, the edge enhance reduce ratio while scene change, 128 : NO edge enhance reduce
	IQ_EDGE_TUNE_PARAM manual_param;
	IQ_EDGE_TUNE_PARAM auto_param[IQ_GAIN_ID_MAX_NUM];
	IQ_EDGE_EXT_TUNE_PARAM ext_manual_param;    ///< 539A only
	IQ_EDGE_EXT_TUNE_PARAM ext_auto_param[IQ_GAIN_ID_MAX_NUM];  ///< 539A only
} IQ_EDGE_PARAM;

typedef struct _IQ_3DNR_TUNE_PARAM {
	UINT8 pf_str;                               ///< range : 0~255, Strength of pre-filters
	UINT8 cost_blend;                           ///< range : 0~15, Cost blending ratio, 0 for automatic cost, 15 for user defined cost
	UINT16 sad_penalty[IQ_3DNR_PENALTY_LEN];    ///< range : 0~1023
	UINT8 detail_penalty[IQ_3DNR_PENALTY_LEN];  ///< range : 0~15
	UINT8 switch_th[IQ_3DNR_SWITCH_LEN];        ///< range : 0~255
	UINT8 switch_rto;                           ///< range : 0~255
	UINT8 probability;                          ///< range : 0~8, switch probability
	UINT16 sad_base[IQ_3DNR_BASE_LEN];          ///< range : 0~16383, base level of noise
	UINT8 sad_coefa[IQ_3DNR_COEFA_LEN];         ///< range : 0~63, Edge coefficient of motion detection
	UINT16 sad_coefb[IQ_3DNR_COEFB_LEN];        ///< range : 0~16383, Offset of motion detection
	UINT16 sad_std[IQ_3DNR_STD_LEN];            ///< range : 0~16383, Standard deviation of motion detection
	UINT8 fth[IQ_3DNR_TH_NUM];                  ///< range : 0~63, final threshold of motion detection
	UINT8 mv_th;                                ///< range : 0~63, MV threshold
	UINT8 mix_ratio[IQ_3DNR_NODE_NUM];          ///< range : 0~63, Mix ratio in patch selection
	UINT8 ds_th;                                ///< range : 0~31, Threshold of motion status down-sampling
	UINT16 blur_eth;                            ///< range : 0~65535, Threshold of TF0 Blur for smooth region
	UINT8 luma_residue_th[IQ_3DNR_OBJECT_NUM];  ///< range : 0~15, Protection threshold of luma channel
	UINT8 chroma_residue_th;                    ///< range : 0~15, Protection threshold of chroma channel
	UINT8 tf0_blur_str[IQ_3DNR_OBJECT_NUM];     ///< range : 0~255, Strength of TF0 blur filter for Y-Channel, static && flat / transition / motion
	UINT8 tf0_blur_estr;                        ///< range : 0~255, Strength of TF0 blur filter for Y-Channel, static && edge
	UINT8 tf0_y_str[IQ_3DNR_OBJECT_NUM];        ///< range : 0~128, Strength of TF0 filter for Y-Channel
	UINT8 tf0_c_str[IQ_3DNR_OBJECT_NUM];        ///< range : 0~128, Strength of TF0 filter for C-Channel
	UINT8 tf0_u_th;                             ///< range : 0~255, Threshold of motion detection for chroma U channel
	UINT8 tf0_v_th;                             ///< range : 0~255, Threshold of motion detection for chroma V channel
	UINT8 tf0_uv_ratio[IQ_3DNR_TF0_OBJECT_NUM]; ///< range : 0~255, Strength of temporal filter in TF0 for still/moving object
	UINT8 pre_filter_str[IQ_3DNR_FREQ_NUM];     ///< range : 0~255, Strength of pre-filtering for low frequency
	UINT8 pre_filter_rto[IQ_3DNR_RATIO_NUM];    ///< range : 0~255, 1X = 255, adjustment ratio of pre-filtering for transitional object
	UINT32 snr_base_th;                         ///< range : 0~65535, Base threshold of spatial noise reduction
	UINT32 tnr_base_th;                         ///< range : 0~65535, Base threshold of spatial noise reduction
	UINT8 freq_wet[IQ_3DNR_FREQ_NUM];           ///< range : 0~255, 1X = 16, Filter weighting for low frequency
	UINT8 luma_wet[IQ_3DNR_F3_LUT_LEN];         ///< range : 0~255, 1X = 16, Filter intensity weighting
	UINT8 snr_str[IQ_3DNR_OBJECT_NUM];          ///< range : 0~255, 1X = 64, Strength of spatial filter for still object
	UINT8 tnr_str[IQ_3DNR_OBJECT_NUM];          ///< range : 0~255, 1X = 64, Strength of temporal filter for still object
	UINT8 luma_3d_lut[IQ_3DNR_F4_LUT_LEN];      ///< range : 0~127, Noise reduction LUT for luma channel
	UINT8 luma_3d_rto[IQ_3DNR_RATIO_NUM];       ///< range : 0~255, 1X = 255, Adjustment ratio 0 of noise reduction LUT for luma channel
	UINT8 chroma_3d_lut[IQ_3DNR_F4_LUT_LEN];    ///< range : 0~127, Noise reduction LUT for chroma channel
	UINT8 chroma_3d_rto[IQ_3DNR_RATIO_NUM];     ///< range : 0~255, 1X = 255, Adjustment ratio 0 of noise reduction LUT for chroma channel
	UINT8 luma_comp_str;                        ///< range : 0~255, luma compensation strength
	UINT8 fcvg_start_point;                     ///< range : 0~4, Fast converge process start point
	UINT8 fcvg_step_size;                       ///< range : 0~4, Fast converge step size
	UINT8 motion_sat_ratio;                     ///< range : 0~255, Saturation adjustment ratio for moving object
	UINT16 cshk_th[IQ_3DNR_LUMA_LEN];           ///< range : 0~1023, Color diterhing thresthod
	UINT8 cshk_val[IQ_3DNR_LUMA_LEN];           ///< range : 0~7, Color diterhing bit number
} IQ_3DNR_TUNE_PARAM;

// 539A only
typedef struct _IQ_3DNR_EXT_TUNE_PARAM {
	UINT8 dc_ratio0;                            ///< range : 0~255, pre-filtering dc ratio, 539A only
	UINT8 dc_ratio1;                            ///< range : 0~255, pre-filtering dc ratio, 539A only
} IQ_3DNR_EXT_TUNE_PARAM;

typedef struct _IQ_3DNR_PARAM {
	BOOL enable;                                ///< enable function
	BOOL fcvg_enable;                           ///< enable function
	IQ_OP_TYPE mode;
	IQ_3DNR_TUNE_PARAM manual_param;
	IQ_3DNR_TUNE_PARAM auto_param[IQ_GAIN_ID_MAX_NUM];
	IQ_3DNR_EXT_TUNE_PARAM ext_manual_param;                           // 539A only
	IQ_3DNR_EXT_TUNE_PARAM ext_auto_param[IQ_GAIN_ID_MAX_NUM];         // 539A only
} IQ_3DNR_PARAM;

typedef struct _IQ_3DNR_MISC_PARAM {
	UINT32 md_roi[IQ_3DNR_ROI_FINAL_THRESHOLD_TAB];
	UINT32 mc_roi[IQ_3DNR_ROI_FINAL_THRESHOLD_TAB];
	UINT32 roi_mv_th;
	UINT32 ds_th_roi;
} IQ_3DNR_MISC_PARAM;

typedef struct _IQ_DPC_PARAM {
	BOOL enable;                                ///< enable function
	UINT32 table[IQ_DPC_MAX_NUM];               ///< dpc table
	BOOL def_same_ch_only_en;                   ///< en/disable cross channel weighting, 539A only
} IQ_DPC_PARAM;

typedef struct _IQ_EXPAND_DPC_PARAM {
	BOOL enable;                                ///< enable function
	UINT32 size;                                ///< dpc table size, Byte
	ULONG table_phyaddr;                        ///< dpc physical address
} IQ_EXPAND_DPC_PARAM;

typedef struct _IQ_SHADING_PARAM {
	BOOL ecs_enable;                            ///< enable function
	BOOL vig_enable;                            ///< enable function
	IQ_OP_TYPE mode;
	BOOL ecs_dthr_enable;
	UINT32 ecs_smooth_l_m_ct_lower;             ///< range : 1000~12000, interpolation range : ecs_smooth_l_m_ct_lower <= CT <= ecs_smooth_l_m_ct_upper
	UINT32 ecs_smooth_l_m_ct_upper;             ///< range : 1000~12000, interpolation range : ecs_smooth_l_m_ct_lower <= CT <= ecs_smooth_l_m_ct_upper
	UINT32 ecs_smooth_m_h_ct_lower;             ///< range : 1000~12000, interpolation range : ecs_smooth_m_h_ct_lower <= CT <= ecs_smooth_m_h_ct_upper
	UINT32 ecs_smooth_m_h_ct_upper;             ///< range : 1000~12000, interpolation range : ecs_smooth_m_h_ct_lower <= CT <= ecs_smooth_m_h_ct_upper
	UINT32 ecs_map_tbl[IQ_SHADING_ECS_LEN];     ///< manual ECS table
	UINT32 vig_center_x;                        ///< range : 0~1000
	UINT32 vig_center_y;                        ///< range : 0~1000
	UINT32 vig_reduce_th;                       ///< range : 100~3276800
	UINT32 vig_zero_th;                         ///< range : 100~3276800
	UINT16 vig_lut[IQ_SHADING_VIG_LEN];         ///< range : 0~8191, UINT32[17], VIG LUT
} IQ_SHADING_PARAM;

typedef struct _IQ_SHADING_EXT_PARAM_IF {
	IQ_ECS_ID ecs_map_idx;                      ///< 0 : hight, 1 : middle, 2 : low
	UINT32 ecs_map_tbl[IQ_SHADING_ECS_LEN];     ///< hight, middle, low CT ECS table
} IQ_SHADING_EXT_PARAM_IF;

typedef struct _IQ_SHADING_EXT_PARAM {
	UINT32 ecs_map_tbl[IQ_ECS_ID_MAX_NUM][IQ_SHADING_ECS_LEN];
} IQ_SHADING_EXT_PARAM;

typedef struct _IQ_PFR_TUNE_PARAM {
	UINT32 pfr_strength;                        ///< range : 0~255, 0 = no PFR, 255: strong PFR, PFR strength
	UINT32 luma_th;                             ///< range : 0~1023, PFR luminance level threshold
} IQ_PFR_TUNE_PARAM;

typedef struct _IQ_PFR_PARAM {
	BOOL enable;                                ///< enable function
	IQ_OP_TYPE mode;
	UINT8 luma_lut[IQ_PFR_LUMA_LEN];            ///< range : 0~255, PFR luminance level control table
	BOOL set0_en;
	UINT32 set0_color_u;                        ///< range : 0~255, reference color u for PFR
	UINT32 set0_color_v;                        ///< range : 0~255, reference color v for PFR
	BOOL set1_en;
	UINT32 set1_color_u;                        ///< range : 0~255, reference color u for PFR
	UINT32 set1_color_v;                        ///< range : 0~255, reference color v for PFR
	BOOL set2_en;
	UINT32 set2_color_u;                        ///< range : 0~255, reference color u for PFR
	UINT32 set2_color_v;                        ///< range : 0~255, reference color v for PFR
	BOOL set3_en;
	UINT32 set3_color_u;                        ///< range : 0~255, reference color u for PFR
	UINT32 set3_color_v;                        ///< range : 0~255, reference color v for PFR
	IQ_PFR_TUNE_PARAM manual_param;
	IQ_PFR_TUNE_PARAM auto_param[IQ_GAIN_ID_MAX_NUM];
} IQ_PFR_PARAM;

typedef struct _IQ_WDR_MANUAL_PARAM {
	UINT32 strength;                            ///< range : 0~255, wdr strength
} IQ_WDR_MANUAL_PARAM;

typedef struct _IQ_WDR_AUTO_PARAM {
	UINT32 level;                               ///< range : 0~255, wdr auto level
	UINT32 strength_min;                        ///< range : 0~255, wdr strength min
	UINT32 strength_max;                        ///< range : 0~255, wdr strength max
} IQ_WDR_AUTO_PARAM;

typedef struct _IQ_WDR_PARAM {
	BOOL enable;                                ///< enable function
	IQ_OP_TYPE mode;
	UINT32 subimg_size_h;                       ///< range : 8~48, wdr sub-image size
	UINT32 subimg_size_v;                       ///< range : 8~48, wdr sub-image size
	UINT32 max_gain;                            ///< range : 1~255, maximum gain of wdr
	UINT32 min_gain;                            ///< range : 1~255, minimum gain of wdr, mapping to 1/1, 1/2 ~ 1/255
	UINT8 halo_ratio;                           ///< range : 0~255, wdr halo ratio
	UINT8 halo_slope;                           ///< range : 0~255, wdr halo slop
	UINT8 fbc_ratio;                            ///< range : 0~255, wdr fbc ratio
	UINT16 gain_protect_str;                    ///< range : 0~256, wdr color protection strength
	UINT16 lut_left[IQ_WDR_LEFT_NUM];           ///< range : 0~4095, non equatable table value
	UINT16 lut_right[IQ_WDR_RIGHT_NUM];         ///< range : 0~4095, non equatable table value
	IQ_WDR_MANUAL_PARAM manual_param;
	IQ_WDR_AUTO_PARAM auto_param[IQ_GAIN_ID_MAX_NUM];
} IQ_WDR_PARAM;

typedef struct _IQ_WDR_ENH_PARAM {
	BOOL enable;                                ///< enable function
	UINT32 enh_ratio[IQ_WDR_STR_LEN];           ///< range : 128~1023, ISO enhance ratio, 128 = 1X
} IQ_WDR_ENH_PARAM;

typedef struct _IQ_DEFOG_MANUAL_PARAM {
	UINT16 fog_level;                                   ///< range : 256~1023, defog fog modify level
	UINT8 fog_ratio;                                    ///< range : 0~255, defog fog ratio for method B
	UINT8 gain_th;                                      ///< range : 32~255, defog gain threshold
	UINT8 outbld_lum_wt[IQ_DEFOG_OUTPUT_BLD_LEN];       ///< range : 0~255, defog luminance output blending table
} IQ_DEFOG_MANUAL_PARAM;

typedef struct _IQ_DEFOG_AUTO_PARAM {
	UINT16 dr_th;                                       ///< range : 0~1023, local dynamic range threshold for fog detection
	UINT16 fog_level_max;                               ///< range : 256~1023, defog fog modify level lower bound
	UINT8 fog_ratio;                                    ///< range : 0~255, defog fog ratio for method B
	UINT8 outbld_wt;                                    ///< range : 0~255, defog luminance output blending table, 128 = 1X
} IQ_DEFOG_AUTO_PARAM;

typedef struct _IQ_DEFOG_PARAM {
	BOOL enable;                                        ///< enable function
	IQ_OP_TYPE mode;
	BOOL outbld_local_en;                               ///< defog local output blending enable
	UINT8 outbld_diff_wt;                               ///< range : 0~7, defog difference output blending weight
	UINT8 min_diff_ratio;                               ///< range : 0~16, defog minimum difference ratio, 16 = 1x minimum airlight
	IQ_DEFOG_MANUAL_PARAM manual_param;
	IQ_DEFOG_AUTO_PARAM auto_param[IQ_GAIN_ID_MAX_NUM];
} IQ_DEFOG_PARAM;

typedef enum _IQ_SHDR_F_CURVE_Y_SEL {
	IQ_SHDR_F_CURVE_Y_8G4R4B = 0,
	IQ_SHDR_F_CURVE_Y_4G = 1,
	IQ_SHDR_F_CURVE_Y_ORI = 2,
	ENUM_DUMMY4WORD(IQ_SHDR_F_CURVE_Y_SEL)
} IQ_SHDR_F_CURVE_Y_SEL;

typedef enum _IQ_SHDR_FUSION_SEL {
	IQ_SHDR_FUSION_BY_L_EXP = 0,
	IQ_SHDR_FUSION_BY_S_EXP = 1,
	IQ_SHDR_FUSION_BOTH_EXP = 2,
	ENUM_DUMMY4WORD(IQ_SHDR_FUSION_SEL)
} IQ_SHDR_FUSION_SEL;

typedef struct _IQ_SHDR_TUNE_PARAM {
	UINT16 nrs_s_str[IQ_SHDR_NRS_STR_NUM];                     ///< range 0~1023, NRS strength for short exposure frame
} IQ_SHDR_TUNE_PARAM;

typedef struct _IQ_SHDR_PARAM {
	BOOL nrs_enable;                                           ///< enable function
	BOOL auto_ev_enable;                                       ///< enable function
	IQ_OP_TYPE mode;
	IQ_SHDR_FUSION_SEL fusion_nor_sel;                         ///< Normal blending curve weighting option
	UINT16 fusion_l_nor_knee;                                  ///< range : 0~4095, knee point of normal blending curve for long exposure
	UINT16 fusion_l_nor_range;                                 ///< range : 16~4095, normal blending curve range for long exposure
	UINT16 fusion_s_nor_knee;                                  ///< range : 0~4095, knee point of normal blending curve for short exposure
	UINT16 fusion_s_nor_range;                                 ///< range : 16~4095, normal blending curve range for short exposure
	IQ_SHDR_FUSION_SEL fusion_dif_sel;                         ///< Difference blending curve weighting option.
	UINT16 fusion_l_dif_knee;                                  ///< range : 0~4095, knee point of difference blending curve for long exposure
	UINT16 fusion_l_dif_range;                                 ///< range : 16~4095, difference blending curve range for long exposure
	UINT16 fusion_s_dif_knee;                                  ///< range : 0~4095, knee point of difference blending curve for short exposure
	UINT16 fusion_s_dif_range;                                 ///< range : 16~4095, difference blending curve range for short exposure
	UINT8 fusion_lum_th;                                       ///< range : 0~255, Lower than this threshold using normal blending curve
	UINT8 fusion_diff_w[IQ_SHDR_FUSION_DIFF_W_NUM];            ///< range : 0~16, Difference weighting
	IQ_SHDR_F_CURVE_Y_SEL fcurve_y_mean_sel;                   ///< Fcurve Y Mean Method selection
	UINT8 fcurve_yv_w;                                         ///< range : 0~8, Fcurve Y mean and V weight, 0: Y, 8: V
	UINT8 fcurve_y_w_lut[IQ_SHDR_FCURVE_Y_W_NUM];              ///< range : 0~255, Fcurve Y/V and RAW weight, 0: Y/V, 255: RAW
	UINT32 fcurve_left_lut[IQ_SHDR_FCURVE_LEFT_NUM];           ///< range : 0~65535 @ 2 frames, Fcurve Value
	UINT32 fcurve_right_lut[IQ_SHDR_FCURVE_RIGHT_NUM];         ///< range : 0~65535 @ 2 frames, Fcurve Value
	IQ_SHDR_TUNE_PARAM manual_param;
	IQ_SHDR_TUNE_PARAM auto_param[IQ_GAIN_ID_MAX_NUM];
	UINT32 fcurve_end_lut[IQ_SHDR_FCURVE_END_NUM];             ///< range : 0~1048575 @ 2 frames, Fcurve Value
} IQ_SHDR_PARAM;

typedef struct _IQ_COMPANDING_PARAM {
	UINT32 decomp_kpx[IQ_DECOMPANDING_KNEE_NUM];        ///< range: 0 ~ 4095, knee-point X for de-companding
	UINT32 decomp_kpy[IQ_DECOMPANDING_KNEE_NUM];        ///< range: 0 ~ 1048575, knee-point Y for de-companding
	UINT32 decomp_gain[IQ_DECOMPANDING_KNEE_NUM];       ///< range: 0 ~ 1023, gain for de-companding
	UINT32 decomp_sb[IQ_DECOMPANDING_KNEE_NUM];         ///< range: 0 ~ 31, shift-bit for de-companding
	UINT32 comp_fcurve_l[IQ_COMPANDING_LEFT_NUM];       ///< range: 0 ~ 4095, left part of f-curve LUT for companding
	UINT32 comp_fcurve_m[IQ_COMPANDING_MIDDLE_NUM];     ///< range: 0 ~ 4095, middle part of f-curve LUT for companding
	UINT32 comp_fcurve_r[IQ_COMPANDING_RIGHT_NUM];      ///< range: 0 ~ 4095, right part of f-curve LUT for companding (only comp_fcurve_ev_fmt=1 valid)
	UINT8  comp_fcurve_ev_fmt;                          ///< range: 0 ~ 1, format option of f-curve LUT (0 : 16bit mode, 1 : 20bit mode)
} IQ_COMPANDING_PARAM;

typedef struct _IQ_RGBIR_MANUAL_PARAM {
	UINT32 irsub_r_weight;	                            ///< range : 0~2047, R channel IR sub ratio, 256 = 1X
	UINT32 irsub_g_weight;                              ///< range : 0~2047, G channel IR sub ratio, 256 = 1X
	UINT32 irsub_b_weight;                              ///< range : 0~2047, B channel IR sub ratio, 256 = 1X
	UINT32 ir_sat_gain;                                 ///< range : 0~1023, Saturation gain multiplied after IR sub
} IQ_RGBIR_MANUAL_PARAM;

typedef struct _IQ_RGBIR_AUTO_PARAM {
	UINT32 irsub_r_weight;	                            ///< range : 0~2047, R channel IR sub ratio, 256 = 1X
	UINT32 irsub_g_weight;                              ///< range : 0~2047, G channel IR sub ratio, 256 = 1X
	UINT32 irsub_b_weight;                              ///< range : 0~2047, B channel IR sub ratio, 256 = 1X
	UINT32 irsub_reduce_th;                             ///< range : 128~255, IR level threshold start to reduce ir_sub
	UINT32 night_mode_th;                               ///< range : 128~255, IR level threshold start to NIGHT_Mode, fix ir_sub 0
} IQ_RGBIR_AUTO_PARAM;

typedef struct _IQ_RGBIR_PARAM {
	BOOL enable;                                        ///< enable function
	IQ_OP_TYPE mode;
	IQ_RGBIR_MANUAL_PARAM manual_param;
	IQ_RGBIR_AUTO_PARAM auto_param;
} IQ_RGBIR_PARAM;

typedef struct _IQ_RGBIR_ENH_MANUAL_PARAM {
	UINT32 enh_ratio[IQ_GAIN_ID_MAX_NUM];               ///< range: 16 ~ 2048, 16 = 1X
	UINT8 outl_rgbir_rb_w;                              ///< range: 0 ~ 255, 0 = disable R/B channel outlier function
	UINT8 outl_ord_rgbir_rb_w;                          ///< range: 0 ~ 255, 0 = disable R/B channel order function
} IQ_RGBIR_ENH_MANUAL_PARAM;

typedef struct _IQ_RGBIR_ENH_AUTO_PARAM {
	UINT32 ir_th;                                       ///< range: 0 ~ 255
	UINT32 enh_ratio[IQ_GAIN_ID_MAX_NUM];               ///< range: 16 ~ 2048, 16 = 1X
	UINT8 outl_rgbir_rb_w;                              ///< range: 0 ~ 255, 0 = disable R/B channel outlier function
	UINT8 outl_ord_rgbir_rb_w;                          ///< range: 0 ~ 255, 0 = disable R/B channel order function
} IQ_RGBIR_ENH_AUTO_PARAM;

typedef struct _IQ_RGBIR_ENH_PARAM {
	BOOL enable;                                        ///< enable function
	IQ_OP_TYPE mode;
	UINT32 min_ir_th;                                   ///< range: 0 ~ 255, if (ir_level < ir_th), enh_ratio = 16(1X)
	IQ_RGBIR_ENH_MANUAL_PARAM manual_param;
	IQ_RGBIR_ENH_AUTO_PARAM auto_param[IQ_RGBIR_LIGHT_ID_MAX_NUM];
} IQ_RGBIR_ENH_PARAM;

typedef struct _IQ_POST_SHARPEN_2_TUNE_PARAM {
	UINT8 noise_level;                                  ///< range: 0 ~ 255, Noise level
	UINT8 noise_curve[IQ_POST_SHARPEN_CURVE_NUM];       ///< range: 0 ~ 255, 17 control points of noise modulation curve
	UINT8 edge_weight_th;                               ///< range: 0 ~ 255, Edge weight coring threshold
	UINT8 edge_weight_gain;                             ///< range: 0 ~ 255, Edge weight gain
	UINT16 th_flat;                                     ///< range: 0 ~ 2047, flat region threshold
	UINT16 th_edge;                                     ///< range: 0 ~ 2047, Edge region threshold
	UINT8 flat_region_str;                              ///< range: 0 ~ 255, Flat region weight strength
	UINT8 edge_region_str;                              ///< range: 0 ~ 255, Edge region weight strength
	UINT8 motion_edge_w_str;                            ///< range: 0 ~ 255, Motion region edge weight
	UINT8 tarnsition_edge_w_str;                        ///< range: 0 ~ 255, Transition region weight strength
	UINT8 static_edge_w_str;                            ///< range: 0 ~ 255, Static region edge weight strength
	UINT8 coring_th;                                    ///< range: 0 ~ 255, Coring threshold
	UINT8 blend_inv_gamma;                              ///< range: 0 ~ 255, Blending ratio of HPF results
	UINT8 sharp_str;                                    ///< range: 0 ~ 128, Sharpen strength
	UINT8 bright_halo_clip;                             ///< range: 0 ~ 128, Bright halo clip ratio 0~128
	UINT8 dark_halo_clip;                               ///< range: 0 ~ 128, Dark halo clip ratio 0~128
} IQ_POST_SHARPEN_2_TUNE_PARAM;

typedef struct _IQ_POST_SHARPEN_2_PARAM {
	BOOL enable;                                        ///< enable function
	IQ_OP_TYPE mode;
	IQ_POST_SHARPEN_2_TUNE_PARAM manual_param;
	IQ_POST_SHARPEN_2_TUNE_PARAM auto_param[IQ_GAIN_ID_MAX_NUM];
} IQ_POST_SHARPEN_2_PARAM;

typedef enum _IQ_POST_SHARPEN_EDGE_FILTER_SEL {
	IQ_POST_SHARPEN_EDGE_FILT_3x3 = 0,
	IQ_POST_SHARPEN_EDGE_FILT_5x5 = 1,
	ENUM_DUMMY4WORD(IQ_POST_SHARPEN_EDGE_FILTER_SEL)
} IQ_POST_SHARPEN_EDGE_FILTER_SEL;

typedef struct _IQ_POST_SHARPEN_1_TUNE_PARAM {
	UINT8 noise_level;                                  ///< range: 0 ~ 255, Noise level
	UINT8 noise_curve[IQ_POST_SHARPEN_CURVE_NUM];       ///< range: 0 ~ 255, 17 control points of noise modulation curve
	UINT8 edge_weight_th;                               ///< range: 0 ~ 255, Edge weight coring threshold
	UINT8 edge_weight_gain;                             ///< range: 0 ~ 255, Edge weight gain
	UINT16 th_flat;                                     ///< range: 0 ~ 2047, flat region threshold
	UINT16 th_edge;                                     ///< range: 0 ~ 2047, Edge region threshold
	UINT8 flat_region_str;                              ///< range: 0 ~ 255, Flat region weight strength
	UINT8 edge_region_str;                              ///< range: 0 ~ 255, Edge region weight strength
	UINT8 motion_edge_w_str;                            ///< range: 0 ~ 255, Motion region edge weight
	UINT8 tarnsition_edge_w_str;                        ///< range: 0 ~ 255, Transition region weight strength
	UINT8 static_edge_w_str;                            ///< range: 0 ~ 255, Static region edge weight strength
	UINT8 coring_th;                                    ///< range: 0 ~ 255, Coring threshold
	UINT8 blend_inv_gamma;                              ///< range: 0 ~ 255, Blending ratio of HPF results
	IQ_POST_SHARPEN_EDGE_FILTER_SEL edge_filt_sel;      ///< edge kernel size selection
	UINT8 sharp_str;                                    ///< range: 0 ~ 255, Sharpen strength
	UINT8 bright_halo_clip;                             ///< range: 0 ~ 128, Bright halo clip ratio 0~128
	UINT8 dark_halo_clip;                               ///< range: 0 ~ 128, Dark halo clip ratio 0~128
} IQ_POST_SHARPEN_1_TUNE_PARAM;

typedef struct _IQ_POST_SHARPEN_1_PARAM {
	BOOL enable;                                        ///< enable function
	IQ_OP_TYPE mode;
	IQ_POST_SHARPEN_1_TUNE_PARAM manual_param;
	IQ_POST_SHARPEN_1_TUNE_PARAM auto_param[IQ_GAIN_ID_MAX_NUM];
} IQ_POST_SHARPEN_1_PARAM;

typedef struct _IQ_YCURVE_PARAM {
	BOOL enable;                                        ///< enable function
	UINT32 ycurve_lut[IQ_YCURVE_LEN];                   ///< range: 0 ~ 255, y curve, table size: 129
} IQ_YCURVE_PARAM;

typedef struct _IQ_CST_TUNE_PARAM {
	UINT8 cstp_ratio;                                   ///< range : 0~8, Ratio of color space transform protection
} IQ_CST_TUNE_PARAM;

typedef struct _IQ_CST_PARAM {
	IQ_OP_TYPE mode;
	INT16 cst_coef[IQ_CST_LEN];                         ///< Color space transform matrix [yr yg yb ur ug ub vr vg vb], table size: 9
	INT16 y_ofs;                                        ///< range : -128~127, Y offset, Y' = Y + Intofs
	UINT8 cb_ofs;                                       ///< range : 0~255, Cb color offset, 128 : NO color offset
	UINT8 cr_ofs;                                       ///< range : 0~255, Cr color offset, 128 : NO color offset
	IQ_CST_TUNE_PARAM manual_param;
	IQ_CST_TUNE_PARAM auto_param[IQ_GAIN_ID_MAX_NUM];
} IQ_CST_PARAM;

typedef struct _IQ_FPN_TUNE_PARAM {
	ULONG buf_phyaddr;                                  ///< input image buffer address
	UINT32 buf_size;                                    ///< input image buffer size (Byte)
	UINT32 gain;                                        ///< range : 0~2047, 256 = 1x
} IQ_FPN_TUNE_PARAM;

typedef struct _IQ_FPN_PARAM {
	BOOL enable;                                        ///< enable function
	IQ_OP_TYPE mode;
	IQ_FPN_TUNE_PARAM manual_param;
	IQ_FPN_TUNE_PARAM auto_param[IQ_GAIN_ID_MAX_NUM];
} IQ_FPN_PARAM;

typedef struct _IQ_3DCC_PARAM {
	BOOL enable;                                        ///< enable function
	UINT32 manual_3dcc_lut[IQ_3DCC_LEN];
	UINT32 mode;                                        ///< Interpolation method, 539A only
} IQ_3DCC_PARAM;

typedef struct _IQ_3DCC_EXT_PARAM_IF {
	IQ_COLOR_ID _3dcc_idx;                              ///< 0 : hight, ..., 4 : low
	UINT32 _3dcc_lut[IQ_3DCC_LEN];
} IQ_3DCC_EXT_PARAM_IF;

typedef struct _IQ_3DCC_EXT_PARAM {
	UINT32 auto_3dcc_lut[IQ_COLOR_ID_MAX_NUM][IQ_3DCC_LEN];
} IQ_3DCC_EXT_PARAM;

typedef enum _IQ_FIXTH_SEL {
	IQ_FIXTH_Y = 0,
	IQ_FIXTH_EDGE = 1,
	ENUM_DUMMY4WORD(IQ_FIXTH_SEL)
} IQ_FIXTH_SEL;

typedef enum _IQ_FIXTH_Y_SEL {
	IQ_FIXTH_Y_HIGHER = 0,
	IQ_FIXTH_Y_LOWER = 1,
	ENUM_DUMMY4WORD(IQ_FIXTH_Y_SEL)
} IQ_FIXTH_Y_SEL;

typedef struct _IQ_FIXTH_PARAM {
	BOOL enable;                                        ///< enable function
	IQ_FIXTH_SEL fixy_sel;                              ///< hit by edge or Y. If by edge, fixc nonwork
	IQ_FIXTH_Y_SEL fixy_y_sel;                          ///< Y for highTH or lowTH
	UINT32 fixy_edge_th;                                ///< range : 0~1023, Edge threshold
	UINT32 fixy_y_th;                                   ///< range : 0~255, Y threshold
	UINT32 fixy_y_value;                                ///< range : 0~255, The register value when hit
	UINT32 fixc_y_th_low;                               ///< range : 0~255, Low threshold of Y
	UINT32 fixc_y_th_high;                              ///< range : 0~255, High threshold of Y
	UINT32 fixc_cb_th_low;                              ///< range : 0~255, Low threshold of CB
	UINT32 fixc_cb_th_high;                             ///< range : 0~255, High threshold of CB
	UINT32 fixc_cr_th_low;                              ///< range : 0~255, Low threshold of CR
	UINT32 fixc_cr_th_high;                             ///< range : 0~255, High threshold of CR
	UINT32 fixc_cb_value;                               ///< range : 0~255, The register value of CB when hit
	UINT32 fixc_cr_value;                               ///< range : 0~255, The register value of CR when hit
} IQ_FIXTH_PARAM;

typedef struct _IQ_OB_MODE_MANUAL {
	BOOL sie_enable;
	BOOL pre_f_enable;                                  ///< 539A only
	BOOL pre_enable;                                    ///< 539A only
	BOOL ife_f_enable;
	BOOL ife_enable;
	UINT32 sie_value;
	UINT32 pre_f_value[IQ_OB_LEN];                      ///< 539A only
	UINT32 pre_value[IQ_OB_LEN];                        ///< 539A only
	UINT32 ife_f_value[IQ_OB_LEN];
	UINT32 ife_value[IQ_OB_LEN];
	BOOL manual_enable;
	IQ_OB_MODE manual_mode;
} IQ_OB_MODE_MANUAL;

typedef struct _IQ_DG_MODE_MANUAL {
	BOOL sie_enable;
	BOOL pre_enable;
	BOOL ife_enable;
	UINT32 sie_value;
	UINT32 pre_value;                                   ///< 539A only
	UINT32 ife_value;
	BOOL manual_enable;
	IQ_DG_MODE manual_mode;
} IQ_DG_MODE_MANUAL;

// 539A only
typedef struct _IQ_BNR_TUNE_PARAM {
	UINT8 prefilter_str;
	UINT16 err_comp_l[IQ_BNR_ERR_COMP_L_MAX];
	UINT16 err_comp_r[IQ_BNR_ERR_COMP_R_MAX];
	UINT8 err_sft;
	UINT16 coef_a;
	UINT16 coef_b;
	UINT8 downsample_th1;
	UINT8 downsample_th2;
	UINT16 static_region;
	UINT16 transision_region;
	UINT16 motion_region;
	UINT32 residue_th;
	UINT16 md_th_l[IQ_BNR_MD_TH_L];
	UINT16 md_th_r[IQ_BNR_MD_TH_R];
	UINT16 md_base;
	UINT16 md_k1;
	UINT16 md_k2;
} IQ_BNR_TUNE_PARAM;

// 539A only
typedef struct _IQ_BNR_PARAM {
	BOOL enable;
	BOOL err_comp_en;
	IQ_OP_TYPE mode;
	IQ_BNR_TUNE_PARAM manual_param;
	IQ_BNR_TUNE_PARAM auto_param[IQ_GAIN_ID_MAX_NUM];
} IQ_BNR_PARAM;

// 539A only
typedef struct _IQ_LOW_POWER_PARAM {
	UINT32 dpc;                      ///< SIE
	UINT32 ecs;                      ///< SIE
	UINT32 va;                       ///< PRE
	UINT32 outlier;                  ///< PRE
	UINT32 bnr;                      ///< PRE
	UINT32 gbal;                     ///< IFE
	UINT32 nr;                       ///< IFE
	UINT32 wdr;                      ///< IFE
	UINT32 rgblpf;                   ///< IPE
	UINT32 pfr;                      ///< IPE
	UINT32 edge;                     ///< IPE
	UINT32 cfa;                      ///< IPE
	UINT32 cc;                       ///< IPE
	UINT32 y_curve;                  ///< IPE
	UINT32 gamma;                    ///< IPE
	UINT32 defog_lce;                ///< IPE
	UINT32 cst;                      ///< IPE
	UINT32 cstp;                     ///< IPE
	UINT32 cctrl;                    ///< IPE
	UINT32 cadjust;                  ///< IPE
	UINT32 lca;                      ///< IME
	UINT32 _3dnr;                    ///< IME
	UINT32 post_sharpen;             ///< IME
} IQ_LOW_POWER_PARAM;

typedef struct _IQ_AIISP_TUNE_PARAM {
	BOOL enable;
	UINT32 effect;                                           ///< Select AI model 0...3
	UINT32 param[AIISP_PARAM_MAX];
} IQ_AIISP_TUNE_PARAM;

typedef struct _IQ_AIISP_PARAM {
	UINT32 path_id;                                          ///< Path 0...3
	UINT32 version;
	UINT32 param_num;                                        ///< Number of parameters
	UINT32 param_size[AIISP_PARAM_MAX];                      ///< The size of each parameter
	CHAR param_name[AIISP_PARAM_MAX][32];                    ///< The name of each parameter
	IQ_OP_TYPE mode;
	IQ_AIISP_TUNE_PARAM manual_param;
	IQ_AIISP_TUNE_PARAM auto_param[IQ_GAIN_ID_MAX_NUM];
} IQ_AIISP_PARAM;

typedef struct _IQ_AIISP_CUSTOM_PARAM {
	BOOL enable;          ///< 0:disable or 1:enable
	UINT32 path_id ;      ///< aiisp path 0...3
	UINT32 effect;        ///< select AI model 0...3
	UINT32 param_num;
	UINT32 param_size[AIISP_PARAM_MAX]; 
	ULONG param_phyaddr[AIISP_PARAM_MAX];
} IQ_AIISP_CUSTOM_PARAM;

typedef struct _IQ_PARAM_PTR {
	IQ_OB_PARAM                      *ob;
	IQ_NR_PARAM                      *nr;
	IQ_CFA_PARAM                     *cfa;
	IQ_VA_PARAM                      *va;
	IQ_GAMMA_PARAM                   *gamma;
	IQ_CCM_PARAM                     *ccm;
	IQ_COLOR_PARAM                   *color;
	IQ_CONTRAST_PARAM                *contrast;
	IQ_EDGE_PARAM                    *edge;
	IQ_3DNR_PARAM                    *_3dnr;
	IQ_PFR_PARAM                     *pfr;
	IQ_WDR_PARAM                     *wdr;
	IQ_DEFOG_PARAM                   *defog;
	IQ_SHDR_PARAM                    *shdr;
	IQ_COMPANDING_PARAM              *companding;
	IQ_RGBIR_PARAM                   *rgbir;
	IQ_RGBIR_ENH_PARAM               *rgbir_enh;
	IQ_POST_SHARPEN_2_PARAM          *post_sharpen_2;
	IQ_YCURVE_PARAM                  *ycurve;
	IQ_CST_PARAM                     *cst;
	IQ_POST_SHARPEN_1_PARAM          *post_sharpen_1;
	IQ_TONE_PARAM                    *tone;
	IQ_WDR_ENH_PARAM                 *wdr_enh;
	IQ_RAW_VA_PARAM                  *raw_va;
	IQ_FPN_PARAM                     *fpn;
	IQ_BNR_PARAM                     *bnr;
	IQ_AIISP_PARAM                   *aiisp;
	IQ_AIISP_CUSTOM_PARAM            *aiisp_custom;
	// Dynamic memory allocation
	IQ_DPC_PARAM                     *dpc;
	IQ_SHADING_PARAM                 *shading;
	IQ_SHADING_EXT_PARAM             *shading_ext;
	IQ_3DCC_PARAM                    *_3dcc;
	IQ_3DCC_EXT_PARAM                *_3dcc_ext;
} IQ_PARAM_PTR;

//=============================================================================
// extern functions
//=============================================================================
extern void iq_param_get_param(ULONG *param);

#endif
