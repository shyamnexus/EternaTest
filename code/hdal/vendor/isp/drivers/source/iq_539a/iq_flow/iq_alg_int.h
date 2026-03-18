#ifndef _IQ_ALG_INT_H_
#define _IQ_ALG_INT_H_

#include "kflow_videocapture/ctl_sie_isp.h"
#include "kflow_videoprocess/ctl_ipp_isp_539a.h"
#include "kdrv_videoenc/kdrv_videoenc.h"

#include "isp_api.h"

#include "iq_alg.h"

#define IQ_MAX(a, b)         (((INT32)(a) > (INT32)(b)) ? (INT32)(a) : (INT32)(b))
#define IQ_MIN(a, b)         (((INT32)(a) < (INT32)(b)) ? (INT32)(a) : (INT32)(b))
#define IQ_CLAMP(in, lb, ub) (((INT32)(in) <= (INT32)(lb)) ? (INT32)(lb) : (((INT32)(in) >= (INT32)(ub)) ? (INT32)(ub) : (INT32)(in)))

//=============================================================================
// struct & definition
//=============================================================================
#define IQ_CGAIN_NUM 3
#define IQ_SHDR_FRAME_NUM 2
#define IQ_HISTO_MAX_SIZE 128
#define IQ_DFG_AIRLIGHT_NUM 3
#define IQ_SUBOUT_W_WINNUM 32
#define IQ_SUBOUT_H_WINNUM 32
#define IQ_SUBOUT_MAX_SIZE ISP_SUBOUT_W_WINNUM*ISP_SUBOUT_H_WINNUM

typedef enum {
	IQ_CAL_MULTIPLY,
	IQ_CAL_INTPL,
	IQ_CAL_HUE,
	IQ_ADJ_ITEM_MAX_CNT,
} IQ_CAL_WAY;

/**
	 IQ ALG Info.

	 IQ  ALG Info.
	 @note
*/
typedef struct _ISOMAP {
	unsigned int  iso;
	unsigned int  index;
} ISOMAP;

/**
	All Structs for IQInfo Page.
*/
// Struct of KDRV Parameter set
typedef struct _IQ_SYNC_INFO {
	ISP_AE_STATUS ae_status;
	UINT32 gain;                                // range : 100~3276800, sync from AE. 100 = 1x, for IQ parameter interpolation
	UINT32 dgain;                               // range : 0~32767, sync from AE. 128 = 1x
	UINT32 lv;                                  // range : 0~20000000, sync from AE. LV = lv / lv_base
	UINT32 lv_base;
	UINT32 shdr_ev_ratio[IQ_SHDR_FRAME_NUM];    // range : 64~16384, sync from AE. 64 = 0EV, 128 = 1EV, 256 = 2EV, 512 = 3EV, 1024 = 4EV, ..., 16384 = 8EV
	UINT32 shdr_tm_ratio;                       // range : 0~16384, sync from AE. 0 = linear fcurve, 64 = 0EV, 128 = 1EV, 256 = 2EV, 512 = 3EV, 1024 = 4EV, ..., 16384 = 8EV
	UINT32 cgain[IQ_CGAIN_NUM];                 // range : 0~2047, sync from AWB. 256 = 1x
	UINT32 ct;                                  // range : 1000~12000, sync from AWB.
	ISP_AE_HBS_PARAM shdr_hbs_param;
} IQ_SYNC_INFO;

typedef struct _IQ_REF_SHADING_PARAM {
	UINT32 manual_ecs_tbl[IQ_SHADING_ECS_LEN];   ///< manual ECS table
	IQ_SHADING_EXT_PARAM auto_tbl;
} IQ_REF_SHADING_PARAM;

typedef struct _IQ_REF_3DCC_PARAM {
	UINT32 manual_3dcc_lut[IQ_3DCC_LEN];         ///< manual 3DCC lut
	IQ_3DCC_EXT_PARAM auto_lut;
} IQ_REF_3DCC_PARAM;

typedef struct _IQ_REF_SET {
	// ECS settings
	IQ_OP_TYPE ecs_mode;
	UINT32 ecs_smooth_l_m_ct_lower;             ///< range : 1000~12000, interpolation range : ecs_smooth_l_m_ct_lower <= CT <= ecs_smooth_l_m_ct_upper
	UINT32 ecs_smooth_l_m_ct_upper;             ///< range : 1000~12000, interpolation range : ecs_smooth_l_m_ct_lower <= CT <= ecs_smooth_l_m_ct_upper
	UINT32 ecs_smooth_m_h_ct_lower;             ///< range : 1000~12000, interpolation range : ecs_smooth_m_h_ct_lower <= CT <= ecs_smooth_m_h_ct_upper
	UINT32 ecs_smooth_m_h_ct_upper;             ///< range : 1000~12000, interpolation range : ecs_smooth_m_h_ct_lower <= CT <= ecs_smooth_m_h_ct_upper
	UINT32 vig_reduce_th;                       ///< range : 100~3276800
	UINT32 vig_zero_th;                         ///< range : 100~3276800
	UINT16 vig_lut[IQ_SHADING_VIG_LEN];         ///< range : 0~8191, VIG LUT

	IQ_REF_SHADING_PARAM *ecs_ext;
	IQ_REF_3DCC_PARAM *_3dcc_ext;

	IQ_FIXTH_PARAM fixth;
} IQ_REF_SET;

typedef struct _IQ_SIE_DPC {
	BOOL enable;                                ///< dpc function enable
	CTL_SIE_DP_MODE_SEL mode;
	BOOL expand_en;                             ///< expand dpc function enable
	UINT32 table[IQ_DPC_MAX_NUM];               ///< dpc table
	ULONG table_phyaddr;                        ///< dpc table physical address, for hw
	ULONG expand_table_viraddr;                 ///< dpc expand table virtual address, for sw
	ULONG expand_table_phyaddr;                 ///< dpc expand table physical address, for hw
	CTL_SIE_DPC_WEIGTH weight;                  ///< dpc weigth select
	UINT32 dp_buffer_size;                      ///< defect pixel buffer size
	UINT32 dp_total_size;                       ///< defect pixel total size
	UINT16 dbg_value;                           ///< DPC replace value when mode = DEF_MODE_DEBUG
	BOOL def_same_ch_only_en;                   ///< DPC en/disable cross channel weighting, 539A supported only
} IQ_SIE_DPC;

typedef struct _IQ_SIE_ECS {
	BOOL enable;
	BOOL sel_37_fmt;
	UINT32 map_tbl[IQ_SHADING_ECS_LEN];         ///< ecs map table
	ULONG map_tbl_phyaddr;                      ///< ecs map table physical address, for hw
	CTL_SIE_ECS_MAP_SEL map_sel;

	BOOL dthr_enable;
	BOOL dthr_reset;
	UINT32 dthr_level;                          ///< dithering level; 0x0=bit[1:0], 0x7=bit[8:7]
	CTL_SIE_ECS_BAYER_MODE_SEL bayer_mode;
} IQ_SIE_ECS;

typedef struct _IQ_AIISP_RESERVED_PARAM {
	UINT8 dgain;
} IQ_AIISP_RESERVED_PARAM;

typedef struct _IQ_FINAL_SIE {
	BOOL                                    ready_flag;
	IQ_SYNC_INFO                            sync_info;
	UINT32                                  final_cgain[IQ_CGAIN_NUM]; // 1X = 256

	CTL_SIE_OB_PARAM                        sie_ob;
	CTL_SIE_CA_PARAM                        sie_ca;
	CTL_SIE_LA_PARAM                        sie_la;
	CTL_SIE_CGAIN                           sie_cgain;
	IQ_SIE_DPC                              *sie_dpc;
	IQ_SIE_ECS                              *sie_ecs;
	CTL_SIE_COMPANDING                      sie_companding;
	CTL_SIE_DGAIN                           sie_dgain;

	CTL_SIE_ISP_ROI_RATIO                   sie_roi_ratio;
} IQ_FINAL_SIE;

typedef struct _IQ_FINAL_IPP {
	BOOL                                    ready_flag;
	IQ_SYNC_INFO                            sync_info;
	UINT32                                  enh_gain;

	CTL_PRE_ISP_NRS                         pre_nrs_0;
	CTL_PRE_ISP_FCURVE                      pre_fcurve;
	CTL_PRE_ISP_FUSION                      pre_fusion;
	CTL_PRE_ISP_OUTL                        pre_outl;
	CTL_PRE_ISP_VA                          pre_va;
	CTL_PRE_ISP_FPN                         pre_fpn;
	CTL_PRE_ISP_BNR                         pre_bnr;
	CTL_PRE_ISP_CGAIN                       pre_cgain;

	CTL_IPP_ISP_AIISP_PARAM                 aiisp;

	CTL_IFE_ISP_FILTER                      ife_filter;
	CTL_IFE_ISP_DGAIN                       ife_dgain;
	CTL_IFE_ISP_CGAIN                       ife_cgain;
	CTL_IFE_ISP_VIG                         ife_vig;
	CTL_IFE_ISP_GBAL                        ife_gbal;
	CTL_IFE_ISP_WDR                         ife_wdr;
	CTL_IFE_ISP_WDR_SUBIMG                  ife_wdr_subimg;
	CTL_IFE_ISP_HIST                        ife_hist;
	CTL_IFE_ISP_SUBISP_IQ                   ife_subisp;

	CTL_IPE_ISP_EEXT                        ipe_eext;
	CTL_IPE_ISP_EEXT_TONEMAP                ipe_eext_tonemap;
	CTL_IPE_ISP_EDGE_OVERSHOOT              ipe_edge_overshoot;
	CTL_IPE_ISP_EPROC                       ipe_eproc;
	CTL_IPE_ISP_RGBLPF                      ipe_rgblpf;
	CTL_IPE_ISP_PFR                         ipe_pfr;
	CTL_IPE_ISP_CC                          ipe_cc;
	CTL_IPE_ISP_CCM                         ipe_ccm;
	CTL_IPE_ISP_CCTRL                       ipe_cctrl;
	CTL_IPE_ISP_CADJ_EE                     ipe_cadj_ee;
	CTL_IPE_ISP_CADJ_YCCON                  ipe_cadj_yccon;
	CTL_IPE_ISP_CADJ_COFS                   ipe_cadj_cofs;
	CTL_IPE_ISP_CADJ_RAND                   ipe_cadj_rand;
	CTL_IPE_ISP_CADJ_HUE                    ipe_cadj_hue;
	CTL_IPE_ISP_CADJ_FIXTH                  ipe_cadj_fixth;
	CTL_IPE_ISP_CADJ_MASK                   ipe_cadj_mask;
	CTL_IPE_ISP_CST                         ipe_cst;
	CTL_IPE_ISP_CSTP                        ipe_cstp;
	CTL_IPE_ISP_GAMYRAND                    ipe_gamyrand;
	CTL_IPE_ISP_GAMMA                       ipe_gamma;
	CTL_IPE_ISP_YCURVE                      ipe_ycurve;
	CTL_IPE_ISP_DEFOG                       ipe_defog;
	CTL_IPE_ISP_LCE                         ipe_lce;
	CTL_IPE_ISP_SUBIMG                      ipe_subimg;
	CTL_IPE_ISP_EDGEDBG                     ipe_edgedbg;
	CTL_IPE_ISP_EDGE_REGION_STR             ipe_edge_region_str;
	CTL_IPE_ISP_CFA                         ipe_cfa;
	CTL_IPE_ISP_3DCC                        *ipe_3dcc;

	CTL_IME_ISP_LCA                         ime_lca;
	CTL_IME_ISP_DBCS                        ime_dbcs;
	CTL_IME_ISP_TMNR                        ime_tmnr;
	CTL_IME_ISP_YCC_CVT                     ime_ycccvt;
	CTL_IME_ISP_SHARPEN                     ime_sharpen;

	CTL_IPP_ISP_IFE_VIG_CENT_RATIO          ife_cent_ratio;
	CTL_IPP_ISP_PRE_VA_WIN_SIZE_RATIO       pre_va_win_size;
	CTL_IPP_ISP_IME_LCA_DBG_X_POS_RATIO     ime_lca_dbg_x_pos;
	CTL_IPP_ISP_FPN_INFO                    pre_fpn_info;
} IQ_FINAL_IPP;

typedef struct _IQ_FINAL_ENC {
	BOOL                                    ready_flag;
	IQ_SYNC_INFO                            sync_info;

	KDRV_H26XENC_SPN                        post_sharpen;
} IQ_FINAL_ENC;

typedef struct _IQ_KFLOW_IPP_SETTING {
	CTL_IPP_ISP_IFE_VIG_CENT_RATIO          *ife_cent;
	CTL_IPP_ISP_PRE_VA_WIN_SIZE_RATIO       *pre_va_win_size;
	CTL_IPP_ISP_IPE_VA_WIN_SIZE_RATIO       *ipe_va_win_size;
	CTL_IPP_ISP_IME_LCA_DBG_X_POS_RATIO     *ime_lca_dbg_x_pos;
	CTL_IPP_ISP_FPN_INFO                    *pre_fpn_info;
} IQ_KFLOW_IPP_SETTING;

typedef struct _IQ_FINAL_SETTING {
	CTL_SIE_ISP_ROI_RATIO                   *sie_roi;
	CTL_SIE_DPC                             sie_dpc_param;
	CTL_SIE_ECS                             sie_ecs_param;
	CTL_SIE_IQ_PARAM                        sie;
	IQ_KFLOW_IPP_SETTING                    ipp;
	CTL_PRE_ISP_IQ_ALL                      pre;
	CTL_IFE_ISP_IQ_ALL                      ife;
	CTL_IPE_ISP_IQ_ALL                      ipe;
	CTL_IME_ISP_IQ_ALL                      ime;
	CTL_IPP_ISP_AIISP_PARAM                 *aiisp;

	KDRV_H26XENC_SPN                        *enc_sharpen;
} IQ_FINAL_SETTING;

/**
	 IQ Information.

	 IQ Information.
	 @note
*/
typedef struct _IQALG_INFO {
	UINT32 id;
	UINT32 shdr_long_id;
	UINT32 mul_last_id;
	UINT32 sie_param_id;
	IQ_PROC_MODE sie_proc_mode;
	IQ_FLOW_MODE sie_flow_mode;
	IQ_FLOW_MODE ipp_flow_mode;
	ISP_IQ_SIE_TRIG_OBJ sie_trig_obj;
	ISP_IQ_IPP_TRIG_OBJ ipp_trig_obj;
	ISP_RGBIR_INFO ir_info;
	ISP_ENC_ISP_RATIO enc_isp_ratio;
	IQ_OB_MODE ob_mode;
	IQ_DG_MODE dg_mode;
	IQ_CG_MODE cg_mode;
	IQ_OB_MODE_MANUAL ob_mode_manual;
	IQ_DG_MODE_MANUAL dg_mode_manual;
	IQ_CG_MODE_MANUAL cg_mode_manual;
	BOOL dg_ai_enable;
	UINT32 dg_ai;
	UINT32 dg_ai2;
	UINT32 ob_ai;
	BOOL dpc_en;
	BOOL ecs_en;
	BOOL _3dcc_en;
	UINT16 hist_stcs_pre_wdr[IQ_HISTO_MAX_SIZE];
	UINT16 hist_stcs_post_wdr[IQ_HISTO_MAX_SIZE];
	UINT16 dfg_airlight[IQ_DFG_AIRLIGHT_NUM];          ///< Legal range : 0~1023, defog statistics
	UINT16 dfg_subout_min[IQ_SUBOUT_MAX_SIZE];         ///< Legal range : 0~1023, the min value of block in subout image
	UINT16 dfg_subout_avg[IQ_SUBOUT_MAX_SIZE];         ///< Legal range : 0~1023, the avg value of block in subout image
	UINT16 dfg_dynamic_range[IQ_SUBOUT_MAX_SIZE];      ///< Legal range : 0~1023, the avg - min value of block in subout image
	UINT32 dr_level;
	BOOL cfg_valid;
	CHAR cfg_path[256];
	BOOL ai_aided_en;
	UINT32 low_power_lv;
	IQ_LOW_POWER_PARAM low_power_param;
	UINT32 aiisp_final_param[AIISP_PARAM_MAX];
	IQ_AIISP_RESERVED_PARAM aiisp_reserved_param;

	UINT32 sie_tab_update;
	UINT32 ipp_tab_update;

	IQ_REF_SET iq_ref_set;

	UINT16 target_shading_vig[IQ_SHADING_VIG_LEN];
	UINT32 target_wdr_str;
	UINT32 target_edge_enh_p;
	UINT32 target_edge_enh_n;

	IQ_FINAL_SIE final_sie;
	IQ_FINAL_IPP final_ipp;
	IQ_FINAL_ENC final_enc;
	IQ_FINAL_SETTING final_setting;
} IQALG_INFO;

typedef struct _IQALG_WDR_AUTO {
	UINT32 level;                               ///< range : 0~255, wdr auto level
	UINT32 strength_min;                        ///< range : 0~255, wdr strength min
	UINT32 strength_max;                        ///< range : 0~255, wdr strength max
} IQALG_WDR_AUTO;

typedef struct _IQALG_WDR {
	IQALG_WDR_AUTO auto_set;
} IQALG_WDR;

typedef struct _IQALG_DEFOG_AUTO {
	UINT16 dr_th;                               ///< local dynamic range threshold for fog detection
	UINT16 fog_level_max;                       ///< defog fog modify level lower bound
	UINT8 outbld_wt;                            ///< range : 0~255, defog luminance output blending table, 128 = 1X
} IQALG_DEFOG_AUTO;

typedef struct _IQALG_DEFOG_MANUAL {
	UINT16 fog_level;                           ///< defog fog modify level
} IQALG_DEFOG_MANUAL;

typedef struct _IQALG_DEFOG {
	IQALG_DEFOG_AUTO auto_set;
	IQALG_DEFOG_MANUAL manual_set;
} IQALG_DEFOG;

/////////////////////////////////////////
// extern variable
/////////////////////////////////////////
extern ISOMAP iso_map_tab[IQ_GAIN_ID_MAX_NUM];
extern INT32 iq_cal(INT32 ratio, INT32 int_value, INT32 min_value, INT32 max_value, IQ_CAL_WAY way);
extern void iq_operation_iso(ISP_TRIG_MSG msg, IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param);
extern void iq_operation_rgbir(ISP_TRIG_MSG msg, IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param);
extern void iq_operation_cgain(ISP_TRIG_MSG msg, IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param);
extern void iq_operation_sie_bypass(ISP_TRIG_MSG msg, IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param);
extern void iq_operation_ecs(IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param, BOOL do_flag);
extern void iq_operation_color(IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param, BOOL do_flag);
extern void iq_operation_tonecurve(IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param, BOOL do_flag);
extern void iq_operation_gamma(IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param, BOOL do_flag);
extern void iq_operation_effect(ISP_TRIG_MSG msg, IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param, BOOL do_flag);
extern void iq_operation_nnsc(ISP_TRIG_MSG msg, IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param, BOOL do_flag);
extern void iq_operation_smooth(ISP_TRIG_MSG msg, IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param, BOOL do_flag);

#endif
