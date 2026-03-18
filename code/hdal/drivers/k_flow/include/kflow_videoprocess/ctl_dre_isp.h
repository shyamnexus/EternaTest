/**
    DRE Ctrl Layer, ISP Interface

    @file       ctl_dre_isp.h
    @ingroup    mIDRE_Ctrl
    @note       None

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/


#ifndef _CTL_DRE_ISP_H
#define _CTL_DRE_ISP_H

#include "kflow_common/isp_if.h"
#include "kflow_common/type_vdo.h"

#define CTL_DRE_ISP_MSNR_LUN_NUM     (128)
#define CTL_DRE_ISP_YCMOD_LUT_NUM    (16)
#define CTL_DRE_ISP_JOINT_CH_NUM     (3)
#define CTL_DRE_ISP_S_WEIGHT_NUM     (6)
#define CTL_DRE_ISP_RTH_LUT_NUM      (8)
#define CTL_DRE_ISP_FU_SPA_FILT_NUM  (3)
#define CTL_DRE_ISP_FU_MANUAL_WT_TBL_MAX_NUM     (128)
#define CTL_DRE_ISP_FU_WT_TBL_NUM    (17)

typedef enum {
	CTL_DRE_ISP_ITEM_IQ_PARAM = 0,					///< [SET], 	data_type: CTL_DRE_ISP_IQ_ALL
	CTL_DRE_ISP_ITEM_MAX,
} CTL_DRE_ISP_ITEM;

typedef struct{
	UINT8 msnr_ylut[CTL_DRE_ISP_MSNR_LUN_NUM];
	UINT8 msnr_ulut[CTL_DRE_ISP_MSNR_LUN_NUM];
	UINT8 msnr_vlut[CTL_DRE_ISP_MSNR_LUN_NUM];
} CTL_DRE_ISP_MSNR_LUT;

typedef struct{
	CTL_DRE_ISP_MSNR_LUT lut;
	BOOL y_lut_en;
	BOOL u_lut_en;
	BOOL v_lut_en;
}CTL_DRE_ISP_MSNR_PARAM;

typedef struct{
	UINT8 y_lut[CTL_DRE_ISP_YCMOD_LUT_NUM];
	UINT8 c_lut[CTL_DRE_ISP_YCMOD_LUT_NUM];
} CTL_DRE_ISP_YCMOD_LUT;

typedef struct{
	CTL_DRE_ISP_YCMOD_LUT lut;
	UINT8 cbofs;
	UINT8 crofs;
	UINT8 stepy;
	UINT8 stepc;
	UINT8 sel;
	UINT8 lutsel;
	BOOL en;
}CTL_DRE_ISP_YCMOD_PARAM;

typedef struct{
	BOOL joint_y[CTL_DRE_ISP_JOINT_CH_NUM];
	BOOL joint_u[CTL_DRE_ISP_JOINT_CH_NUM];
	BOOL joint_v[CTL_DRE_ISP_JOINT_CH_NUM];
	BOOL outl_en[CTL_DRE_ISP_JOINT_CH_NUM];
} CTL_DRE_ISP_NR_JOINT_OUTL;

typedef struct{
	UINT8 s_weight[CTL_DRE_ISP_S_WEIGHT_NUM];
	UINT8 rth_y_lut[CTL_DRE_ISP_RTH_LUT_NUM];
	UINT8 rth_u_lut[CTL_DRE_ISP_RTH_LUT_NUM];
	UINT8 rth_v_lut[CTL_DRE_ISP_RTH_LUT_NUM];
} CTL_DRE_ISP_NR_FILT;

typedef struct{
	CTL_DRE_ISP_NR_JOINT_OUTL nr_joint_outl;
	CTL_DRE_ISP_NR_FILT       nr_filter;
} CTL_DRE_ISP_NR_PARAM;

typedef struct{
	UINT8 fusion_wt_table_val[CTL_DRE_ISP_FU_WT_TBL_NUM];
} CTL_DRE_ISP_FUSION_WT_TBL;

typedef struct {
	CTL_DRE_ISP_MSNR_PARAM      *p_msnr;
	CTL_DRE_ISP_YCMOD_PARAM		*p_ycmod;
	CTL_DRE_ISP_NR_PARAM      	*p_nr_param;
	CTL_DRE_ISP_FUSION_WT_TBL 	*p_weight_tbl;
} CTL_DRE_ISP_IQ_ALL;

typedef enum {
	CTL_DRE_ISP_CB_MSG_NONE = 0x00000000,
	CTL_DRE_ISP_CB_MAX_MAX,
} CTL_DRE_ISP_CB_MSG;

ER ctl_dre_isp_evt_fp_reg(CHAR *name, ISP_EVENT_FP fp, ISP_EVENT evt, CTL_DRE_ISP_CB_MSG cb_msg);
ER ctl_dre_isp_evt_fp_unreg(CHAR *name);
ER ctl_dre_isp_set(ISP_ID id, CTL_DRE_ISP_ITEM item, void *data);
ER ctl_dre_isp_get(ISP_ID id, CTL_DRE_ISP_ITEM item, void *data);

#endif