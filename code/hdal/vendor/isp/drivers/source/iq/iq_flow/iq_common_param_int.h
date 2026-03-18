#ifndef _IQ_COMMON_PARAM_INT_H_
#define _IQ_COMMON_PARAM_INT_H_

#include "kwrap/type.h"

typedef struct _IQ_RAW_VA_IIR_PARAM {
	INT16 iir1_tap_a;                        ///< range : -511~511
	INT16 iir1_tap_b;                        ///< range : -511~511
	INT16 iir1_tap_e;                        ///< range : -511~511
	INT16 iir1_tap_f;                        ///< range : -511~511
	UINT8 iir1_div;                          ///< range : 0~15
	UINT8 iir1_shift;                        ///< range : 0~13
	BOOL iir2_en;
	INT16 iir2_tap_a;                        ///< range : -511~511
	INT16 iir2_tap_b;                        ///< range : -511~511
	INT16 iir2_tap_e;                        ///< range : -511~511
	INT16 iir2_tap_f;                        ///< range : -511~511
	UINT8 iir2_shift;                        ///< range : 0~13
	BOOL iir3_en;
	INT16 iir3_tap_a;                        ///< range : -511~511
	INT16 iir3_tap_b;                        ///< range : -511~511
	INT16 iir3_tap_e;                        ///< range : -511~511
	INT16 iir3_tap_f;                        ///< range : -511~511
	UINT8 iir3_shift;                        ///< range : 0~13
} IQ_RAW_VA_IIR_PARAM;

/////////////////////////////////////////
// extern variable
/////////////////////////////////////////
extern CTL_SIE_OB_PARAM ctl_sie_ob_init;
extern CTL_SIE_CA_PARAM ctl_sie_ca_init;
extern CTL_SIE_LA_PARAM ctl_sie_la_init;
extern CTL_SIE_CGAIN ctl_sie_cgain_init;
extern CTL_SIE_COMPANDING ctl_sie_companding_init;
extern CTL_SIE_DGAIN ctl_sie_dgain_init;
extern CTL_SIE_MD_PARAM ctl_sie_md_init;

extern CTL_SIE_ISP_ROI_RATIO ctl_sie_roi_ratio_init;

extern CTL_IFE_ISP_NRS ctl_ife_nrs_0_init;
extern CTL_IFE_ISP_NRS ctl_ife_nrs_1_init;
extern CTL_IFE_ISP_FCURVE ctl_ife_fcurve_init;
extern CTL_IFE_ISP_FUSION ctl_ife_fusion_init;
extern CTL_IFE_ISP_OUTL ctl_ife_outl_init;
extern CTL_IFE_ISP_FILTER ctl_ife_filter_init;
extern CTL_IFE_ISP_DGAIN ctl_ife_dgain_init;
extern CTL_IFE_ISP_CGAIN ctl_ife_cgain_init;
extern CTL_IFE_ISP_VIG ctl_ife_vig_init;
extern CTL_IFE_ISP_GBAL ctl_ife_gbal_init;
extern CTL_IFE_ISP_WDR ctl_ife_wdr_init;
extern CTL_IFE_ISP_WDR_SUBIMG ctl_ife_wdr_subimg_init;
extern CTL_IFE_ISP_HIST ctl_ife_hist_init;
extern CTL_IFE_ISP_VA ctl_ife_va_init;
extern CTL_IFE_ISP_SUBISP_IQ ctl_ife_subisp_init;
extern CTL_IFE_ISP_FPN ctl_ife_fpn_init;

extern CTL_IPE_ISP_EEXT ctl_ipe_eext_init;
extern CTL_IPE_ISP_EEXT_TONEMAP ctl_ipe_eext_tone_init;
extern CTL_IPE_ISP_EDGE_OVERSHOOT ctl_ipe_edge_overshoot_init;
extern CTL_IPE_ISP_EPROC ctl_ipe_eproc_init;
extern CTL_IPE_ISP_RGBLPF ctl_ipe_rgblpf_init;
extern CTL_IPE_ISP_PFR ctl_ipe_pfr_init;
extern CTL_IPE_ISP_CC ctl_ipe_cc_init;
extern CTL_IPE_ISP_CCM ctl_ipe_ccm_init;
extern CTL_IPE_ISP_CCTRL ctl_ipe_cctrl_init;
extern CTL_IPE_ISP_CADJ_EE ctl_ipe_cadj_ee_init;
extern CTL_IPE_ISP_CADJ_YCCON ctl_ipe_cadj_yccon_init;
extern CTL_IPE_ISP_CADJ_COFS ctl_ipe_cadj_cofs_init;
extern CTL_IPE_ISP_CADJ_RAND ctl_ipe_cadj_rand_init;
extern CTL_IPE_ISP_CADJ_HUE ctl_ipe_cadj_hue_init;
extern CTL_IPE_ISP_CADJ_FIXTH ctl_ipe_cadj_fixth_init;
extern CTL_IPE_ISP_CADJ_MASK ctl_ipe_cadj_mask_init;
extern CTL_IPE_ISP_CST ctl_ipe_cst_init;
extern CTL_IPE_ISP_CSTP ctl_ipe_cstp_init;
extern CTL_IPE_ISP_GAMYRAND ctl_ipe_gamyrand_init;
extern CTL_IPE_ISP_GAMMA ctl_ipe_gamma_init;
extern CTL_IPE_ISP_YCURVE ctl_ipe_ycurve_init;
extern CTL_IPE_ISP_DEFOG ctl_ipe_defog_init;
extern CTL_IPE_ISP_LCE ctl_ipe_lce_init;
extern CTL_IPE_ISP_SUBIMG ctl_ipe_subimg_init;
extern CTL_IPE_ISP_EDGEDBG ctl_ipe_edgedbg_init;
extern CTL_IPE_ISP_VA ctl_ipe_va_init;
extern CTL_IPE_ISP_EDGE_REGION_STR ctl_ipe_edge_region_str_init;
extern CTL_IPE_ISP_CFA ctl_ipe_cfa_init;
extern CTL_IPE_ISP_3DCC ctl_ipe_3dcc_init;

extern CTL_IME_ISP_LCA ctl_ime_lca_init;
extern CTL_IME_ISP_DBCS ctl_ime_dbcs_init;
extern CTL_IME_ISP_TMNR ctl_ime_tmnr_init;
extern CTL_IME_ISP_YCC_CVT ctl_ime_ycc_cvt_init;
extern CTL_IME_ISP_SHARPEN ctl_ime_sharpen_init;

extern CTL_IPP_ISP_IFE_VIG_CENT_RATIO ctrl_ipp_ife_vig_cent_ratio_init;
extern CTL_IPP_ISP_IFE_VA_WIN_SIZE_RATIO ctrl_ipp_ife_va_win_size_init;
extern CTL_IPP_ISP_IPE_VA_WIN_SIZE_RATIO ctrl_ipp_ipe_va_win_size_init;
extern CTL_IPP_ISP_IME_LCA_DBG_X_POS_RATIO ctrl_ipp_ime_lca_dbg_x_pos_init;
extern CTL_IPP_ISP_FPN_INFO ctrl_ipp_ife_fpn_info_init;

extern KDRV_H26XENC_SPN kdrv_enc_post_sharpen_init;

extern CTL_IPE_ISP_CADJ_FIXTH iq_effect_param_color_pencil;
extern CTL_IPE_ISP_CADJ_FIXTH iq_effect_param_sketch;
extern CTL_IPE_ISP_CADJ_FIXTH iq_effect_param_rock;
extern INT8 iq_effect_param_select_cc[IQ_UI_CCID_MAX_CNT][IQ_CCM_SATTAB_LEN];
extern UINT32 iq_effect_param_negative[IQ_GAMMA_LEN];
extern UINT32 iq_effect_param_thermal_red[3][IQ_GAMMA_LEN];
extern UINT32 iq_effect_param_thermal_color[3][IQ_GAMMA_LEN];

extern CTL_IPE_ISP_CCTRL iq_nnsc_param_enh_green_cctrl_ct;
extern CTL_IPE_ISP_CCTRL iq_nnsc_param_enh_skin_cctrl;
extern CTL_IPE_ISP_CCTRL iq_nnsc_param_enh_skin_white_cctrl_ct;
extern CTL_IPE_ISP_CCTRL iq_nnsc_param_enh_skin_tan_cctrl_ct;

extern CTL_IME_ISP_TMNR iq_tmnr_still;

#endif
