#if defined(__FREERTOS)
#include <string.h>
#include <stdio.h>
#endif
#include "kwrap/type.h"

#include "iqt_api.h"
#include "iq_alg_int.h"
#include "iq_flow.h"
#include "iq_msg.h"
#include "iq_nnsc_int.h"
#include "iq_ui_int.h"
#include "iq_dbg.h"
#include "iq_version.h"

#if defined(__KERNEL__)
extern UINT32 iq_id_list;
extern UINT32 iq_dpc_en;
extern UINT32 iq_ecs_en;
extern UINT32 iq_3dcc_en;
#endif

void iq_msg_sie(IQALG_INFO *iq_info)
{
	UINT32 dbg_mode = iq_dbg_get_dbg_mode(iq_info->id);
	CTL_SIE_OB_PARAM *ob = iq_info->final_setting.sie.ob_param;
	CTL_SIE_CA_PARAM *ca = iq_info->final_setting.sie.ca_param;
	CTL_SIE_ISP_ROI_RATIO *roi = iq_info->final_setting.sie_roi;
	CTL_SIE_LA_PARAM *la = iq_info->final_setting.sie.la_param;
	CTL_SIE_CGAIN *cgain = iq_info->final_setting.sie.cgain_param;
	CTL_SIE_DPC *dpc = iq_info->final_setting.sie.dpc_param;
	CTL_SIE_ECS *ecs = iq_info->final_setting.sie.ecs_param;
	CTL_SIE_COMPANDING *companding = iq_info->final_setting.sie.companding_param;
	CTL_SIE_DGAIN *dgain = iq_info->final_setting.sie.dgain_param;
	CTL_SIE_MD_PARAM *md = iq_info->final_setting.sie.md_param;

	IQ_SIE_DPC *dpc_param = iq_info->final_sie.sie_dpc;
	IQ_SIE_ECS *ecs_param = iq_info->final_sie.sie_ecs;

	UINT32 i;

	if (ob != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ob->bypass_enable);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ob->ob_ofs);
		PRINT_IQ(dbg_mode & IQ_DBG_P_SIE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_SIE, "ob_param does not need to update!! \r\n");
	}

	if ((ca != NULL) && (roi != NULL)) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ca->enable);
		if (ca->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ca->win_num.w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ca->win_num.h);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ca->th_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ca->g_th_l);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ca->g_th_u);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ca->r_th_l);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ca->r_th_u);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ca->b_th_l);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ca->b_th_u);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ca->p_th_l);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ca->p_th_u);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ca->irsub_r_weight);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ca->irsub_g_weight);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ca->irsub_b_weight);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ca->ca_ob_ofs);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ca->ca_src);
			if (roi != NULL) {
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, roi->ratio_base);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, roi->ca_crop_win_roi.x);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, roi->ca_crop_win_roi.y);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, roi->ca_crop_win_roi.w);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, roi->ca_crop_win_roi.h);
			} else {
				PRINT_IQ(dbg_mode & IQ_DBG_P_SIE, "ca_crop_win_roi does not need to update!! \r\n");
			}
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_SIE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_SIE, "ca_param does not need to update!! \r\n");
	}

	if ((la != NULL) && (roi != NULL)) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->enable);
		if (la->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->win_num.w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->win_num.h);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->la_src);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->la_rgb2y1mod);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->la_rgb2y2mod);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->cg_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->r_gain);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->g_gain);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->b_gain);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->gamma_enable);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_SIE, la->gamma_tbl, CTL_SIE_LA_GMA_TBL_NUM);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->hist_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->histo_src);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->irsub_r_weight);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->irsub_g_weight);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->irsub_b_weight);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->la_ob_ofs);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->lath_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->lathy1lower);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->lathy1upper);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->lathy2lower);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, la->lathy2upper);
			if (roi != NULL) {
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, roi->ratio_base);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, roi->la_crop_win_roi.x);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, roi->la_crop_win_roi.y);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, roi->la_crop_win_roi.w);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, roi->la_crop_win_roi.h);
			} else {
				PRINT_IQ(dbg_mode & IQ_DBG_P_SIE, "la_crop_win_roi does not need to update!! \r\n");
			}
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_SIE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_SIE, "la_param does not need to update!! \r\n");
	}

	if (cgain != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, cgain->enable);
		if (cgain->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, cgain->sel_37_fmt);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, cgain->r_gain);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, cgain->gr_gain);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, cgain->gb_gain);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, cgain->b_gain);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, cgain->ir_gain);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_SIE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_SIE, "cgain_param does not need to update!! \r\n");
	}

	if ((iq_info->sie_tab_update & IQ_TBL_SIE_DPC) && (dpc != NULL)) {
		PRINT_IQ_VAR((dbg_mode & IQ_DBG_P_SIE) || (dbg_mode & IQ_DBG_TABLE), dpc->enable);
		if (dpc->enable) {
			PRINT_IQ_VAR_HEX((dbg_mode & IQ_DBG_P_SIE) || (dbg_mode & IQ_DBG_TABLE), dpc->table_viraddr);
			PRINT_IQ_VAR_HEX((dbg_mode & IQ_DBG_P_SIE) || (dbg_mode & IQ_DBG_TABLE), dpc->table_phyaddr);
			for (i = 0; i < 30; i++) {
				PRINT_IQ((dbg_mode & IQ_DBG_P_SIE) || (dbg_mode & IQ_DBG_TABLE), "dpc->table[%d] = %x \r\n", i, *((UINT32 *)dpc->table_viraddr + i));
			}
			PRINT_IQ_VAR((dbg_mode & IQ_DBG_P_SIE) || (dbg_mode & IQ_DBG_TABLE), dpc->weight);
			PRINT_IQ_VAR((dbg_mode & IQ_DBG_P_SIE) || (dbg_mode & IQ_DBG_TABLE), dpc->dp_total_size);
		}
		PRINT_IQ((dbg_mode & IQ_DBG_P_SIE) || (dbg_mode & IQ_DBG_TABLE), "\r\n");
	} else if (dpc_param != NULL) {
		PRINT_IQ(dbg_mode & IQ_DBG_P_SIE, "not update, print previous setting!! \r\n");
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, dpc_param->enable);
		if (dpc_param->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, dpc_param->expand_en);
			PRINT_IQ_ARR_HEX(dbg_mode & IQ_DBG_P_SIE, dpc_param->table, 30);
			PRINT_IQ_VAR_HEX(dbg_mode & IQ_DBG_P_SIE, dpc_param->table_phyaddr);
			PRINT_IQ_VAR_HEX(dbg_mode & IQ_DBG_P_SIE, dpc_param->expand_table_viraddr);
			PRINT_IQ_VAR_HEX(dbg_mode & IQ_DBG_P_SIE, dpc_param->expand_table_phyaddr);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, dpc_param->weight);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, dpc_param->dp_buffer_size);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, dpc_param->dp_total_size);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_SIE, "\r\n");
	}

	if ((iq_info->sie_tab_update & IQ_TBL_SIE_ECS) && (ecs != NULL)) {
		PRINT_IQ_VAR((dbg_mode & IQ_DBG_P_SIE) || (dbg_mode & IQ_DBG_TABLE), ecs->enable);
		if (ecs->enable) {
			PRINT_IQ_VAR((dbg_mode & IQ_DBG_P_SIE) || (dbg_mode & IQ_DBG_TABLE), ecs->sel_37_fmt);
			PRINT_IQ_VAR_HEX((dbg_mode & IQ_DBG_P_SIE) || (dbg_mode & IQ_DBG_TABLE), ecs->map_tbl_viraddr);
			PRINT_IQ_VAR_HEX((dbg_mode & IQ_DBG_P_SIE) || (dbg_mode & IQ_DBG_TABLE), ecs->map_tbl_phyaddr);
			for (i = 0; i < 30; i++) {
				PRINT_IQ((dbg_mode & IQ_DBG_P_SIE) || (dbg_mode & IQ_DBG_TABLE), "ecs->map_tbl[%d] = %x \r\n", i, *((UINT32 *)ecs->map_tbl_viraddr + i));
			}
			PRINT_IQ_VAR((dbg_mode & IQ_DBG_P_SIE) || (dbg_mode & IQ_DBG_TABLE), ecs->map_sel);
			PRINT_IQ_VAR((dbg_mode & IQ_DBG_P_SIE) || (dbg_mode & IQ_DBG_TABLE), ecs->dthr_enable);
			PRINT_IQ_VAR((dbg_mode & IQ_DBG_P_SIE) || (dbg_mode & IQ_DBG_TABLE), ecs->dthr_reset);
			PRINT_IQ_VAR((dbg_mode & IQ_DBG_P_SIE) || (dbg_mode & IQ_DBG_TABLE), ecs->dthr_level);
			PRINT_IQ_VAR((dbg_mode & IQ_DBG_P_SIE) || (dbg_mode & IQ_DBG_TABLE), ecs->bayer_mode);
		}
		PRINT_IQ((dbg_mode & IQ_DBG_P_SIE) || (dbg_mode & IQ_DBG_TABLE), "\r\n");
	} else if (ecs_param != NULL) {
		PRINT_IQ(dbg_mode & IQ_DBG_P_SIE, "not update, print previous setting!! \r\n");
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ecs_param->enable);
		if (ecs_param->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ecs_param->sel_37_fmt);
			PRINT_IQ_ARR_HEX(dbg_mode & IQ_DBG_P_SIE, ecs_param->map_tbl, 30);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ecs_param->map_sel);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ecs_param->dthr_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ecs_param->dthr_reset);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ecs_param->dthr_level);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, ecs_param->bayer_mode);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_SIE, "\r\n");
	}

	if (companding != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, companding->enable);
		if (companding->enable) {
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_SIE, companding->decomp_info.decomp_kpx, CTL_SIE_DECOMPANDING_KPX_MAX_LEN);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_SIE, companding->decomp_info.decomp_kpy, CTL_SIE_DECOMPANDING_KPY_MAX_LEN);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_SIE, companding->decomp_info.decomp_gain, CTL_SIE_DECOMPANDING_GAIN_MAX_LEN);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_SIE, companding->decomp_info.decomp_sb, CTL_SIE_DECOMPANDING_SB_MAX_LEN);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_SIE, companding->comp_info.comp_fcurve_l, CTL_SIE_COMPANDING_FL_MAX_LEN);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_SIE, companding->comp_info.comp_fcurve_m, CTL_SIE_COMPANDING_FM_MAX_LEN);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_SIE, companding->comp_info.comp_fcurve_r, CTL_SIE_COMPANDING_FR_MAX_LEN);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, companding->comp_info.comp_fcurve_ev_fmt);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, companding->comp_shift);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_SIE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_SIE, "companding_param does not need to update!! \r\n");
	}

	if (dgain != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, dgain->enable);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, dgain->enable_stcs);
		if (dgain->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, dgain->gain);
		}

		PRINT_IQ(dbg_mode & IQ_DBG_P_SIE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_SIE, "dgain_param does not need to update!! \r\n");
	}

	if (md != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, md->enable);
		if (md->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, md->md_src);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, md->sum_frms);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, md->mask_mode);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, md->mask0);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, md->mask1);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, md->blkdiff_thr);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, md->total_blkdiff_thr);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_SIE, md->blkdiff_cnt_thr);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_SIE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_SIE, "md_param does not need to update!! \r\n");
	}

	iq_dbg_clr_dbg_mode(iq_info->id, IQ_DBG_P_SIE);
}

void iq_msg_ife(IQALG_INFO *iq_info)
{
	UINT32 dbg_mode = iq_dbg_get_dbg_mode(iq_info->id);
	CTL_IFE_ISP_NRS *nrs_0 = iq_info->final_setting.ife.p_nrs0;
	CTL_IFE_ISP_FCURVE *fcurve = iq_info->final_setting.ife.p_fcurve;
	CTL_IFE_ISP_FUSION *fusion = iq_info->final_setting.ife.p_fusion;
	CTL_IFE_ISP_OUTL *outl = iq_info->final_setting.ife.p_outl;
	CTL_IFE_ISP_FILTER *filt = iq_info->final_setting.ife.p_filt;
	CTL_IFE_ISP_DGAIN *dgain = iq_info->final_setting.ife.p_dgain;
	CTL_IFE_ISP_CGAIN *cgain = iq_info->final_setting.ife.p_cgain;
	CTL_IFE_ISP_VIG *vig = iq_info->final_setting.ife.p_vig;
	CTL_IFE_ISP_GBAL *gbal = iq_info->final_setting.ife.p_gbal;
	CTL_IFE_ISP_WDR *wdr = iq_info->final_setting.ife.p_wdr;
	CTL_IFE_ISP_WDR_SUBIMG *wdr_subimg = iq_info->final_setting.ife.p_wdr_subimg;
	CTL_IFE_ISP_HIST *hist = iq_info->final_setting.ife.p_hist;
	CTL_IFE_ISP_VA *va = iq_info->final_setting.ife.p_va;
	CTL_IFE_ISP_SUBISP_IQ *subisp = iq_info->final_setting.ife.p_subisp_iq;
	CTL_IFE_ISP_FPN *fpn = iq_info->final_setting.ife.p_fpn;
	CTL_IPP_ISP_IFE_VA_WIN_SIZE_RATIO *va_win_size = iq_info->final_setting.ipp.ife_va_win_size;

	if (nrs_0 != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, nrs_0->enable);
		if (nrs_0->enable) {
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, nrs_0->str, CTL_IFE_ISP_NRS_STR_NUM);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "nrs_0 does not need to update!! \r\n");
	}

	if (fcurve != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fcurve->enable);
		if (fcurve->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fcurve->fcur_ctrl.y_mean_sel);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fcurve->fcur_ctrl.yv_w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fcurve->fcur_ctrl.ev_fmt);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fcurve->y_weight.y_w_lut, CTL_IFE_ISP_FCURVE_Y_W_NUM);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fcurve->fcurve_l.fcur_l_lut, CTL_IFE_ISP_FCURVE_L_NUM);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fcurve->fcurve_r.fcur_r_lut, CTL_IFE_ISP_FCURVE_R_NUM);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "fcurve does not need to update!! \r\n");
	}

	if (fusion != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_ctrl.y_mean_sel);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_ctrl.mode);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_ctrl.ev_ratio);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->bld_cur.nor_sel);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->bld_cur.dif_sel);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fusion->bld_cur.l_nor_knee, CTL_IFE_ISP_FUSION_BLD_CUR_KNEE_NUM);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->bld_cur.l_nor_range);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->bld_cur.l_nor_slope);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fusion->bld_cur.s_nor_knee, CTL_IFE_ISP_FUSION_BLD_CUR_KNEE_NUM);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->bld_cur.s_nor_range);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->bld_cur.s_nor_slope);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fusion->bld_cur.l_dif_knee, CTL_IFE_ISP_FUSION_BLD_CUR_KNEE_NUM);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->bld_cur.l_dif_range);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->bld_cur.l_dif_slope);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fusion->bld_cur.s_dif_knee, CTL_IFE_ISP_FUSION_BLD_CUR_KNEE_NUM);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->bld_cur.s_dif_range);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->bld_cur.s_dif_slope);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->mc_para.lum_th);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->mc_para.diff_ratio);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fusion->mc_para.diff_w, CTL_IFE_ISP_FUSION_MC_DIFF_W_NUM);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->mc_para.dwd);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fusion->dk_sat.th, CTL_IFE_ISP_FUSION_DARK_SAT_NUM);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fusion->dk_sat.step, CTL_IFE_ISP_FUSION_DARK_SAT_NUM);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fusion->dk_sat.low_bound, CTL_IFE_ISP_FUSION_DARK_SAT_NUM);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.enable);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.bit_field);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.fcgain_s[CTL_IFE_ISP_CGAIN_CH_R]);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.fcgain_s[CTL_IFE_ISP_CGAIN_CH_GR]);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.fcgain_s[CTL_IFE_ISP_CGAIN_CH_GB]);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.fcgain_s[CTL_IFE_ISP_CGAIN_CH_B]);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.fcgain_s[CTL_IFE_ISP_CGAIN_CH_IR]);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_R]);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_GR]);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_GR]);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_B]);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.fcofs_s[CTL_IFE_ISP_CGAIN_CH_IR]);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.fcgain_l[CTL_IFE_ISP_CGAIN_CH_R]);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.fcgain_l[CTL_IFE_ISP_CGAIN_CH_GR]);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.fcgain_l[CTL_IFE_ISP_CGAIN_CH_GB]);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.fcgain_l[CTL_IFE_ISP_CGAIN_CH_B]);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.fcgain_l[CTL_IFE_ISP_CGAIN_CH_IR]);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_R]);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_GR]);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_GB]);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_B]);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->fu_cgain.fcofs_l[CTL_IFE_ISP_CGAIN_CH_IR]);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->dbg.enable);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fusion->dbg.mode);
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "fusion does not need to update!! \r\n");
	}

	if (outl != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, outl->enable);
		if (outl->enable) {
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, outl->bright_th, CTL_IFE_ISP_OUTL_BRI_TH_NUM);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, outl->dark_th, CTL_IFE_ISP_OUTL_DARK_TH_NUM);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, outl->outl_cnt, CTL_IFE_ISP_OUTL_CNT_NUM);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, outl->outl_weight);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, outl->dark_ofs);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, outl->bright_ofs);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, outl->outl_comp_mode);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, outl->ord_rng_bri);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, outl->ord_rng_dark);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, outl->ord_protect_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, outl->ord_blend_w);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, outl->ord_bri_w, CTL_IFE_ISP_OUTL_ORD_W_NUM);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, outl->ord_dark_w, CTL_IFE_ISP_OUTL_ORD_W_NUM);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, outl->outl_rgbir_rb_w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, outl->ord_rgbir_rb_w);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "outl does not need to update!! \r\n");
	}

	if (filt != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, filt->enable);
		if (filt->enable) {
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->spatial.weight, CTL_IFE_ISP_SPATIAL_W_LEN);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rng_filt_r.a_th, CTL_IFE_ISP_RANGE_A_TH_NUM);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rng_filt_r.a_lut, CTL_IFE_ISP_RANGE_A_LUT_SIZE);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rng_filt_r.b_th, CTL_IFE_ISP_RANGE_B_TH_NUM);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rng_filt_r.b_lut, CTL_IFE_ISP_RANGE_B_LUT_SIZE);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rng_filt_gr.a_th, CTL_IFE_ISP_RANGE_A_TH_NUM);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rng_filt_gr.a_lut, CTL_IFE_ISP_RANGE_A_LUT_SIZE);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rng_filt_gr.b_th, CTL_IFE_ISP_RANGE_B_TH_NUM);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rng_filt_gr.b_lut, CTL_IFE_ISP_RANGE_B_LUT_SIZE);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rng_filt_gb.a_th, CTL_IFE_ISP_RANGE_A_TH_NUM);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rng_filt_gb.a_lut, CTL_IFE_ISP_RANGE_A_LUT_SIZE);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rng_filt_gb.b_th, CTL_IFE_ISP_RANGE_B_TH_NUM);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rng_filt_gb.b_lut, CTL_IFE_ISP_RANGE_B_LUT_SIZE);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rng_filt_b.a_th, CTL_IFE_ISP_RANGE_A_TH_NUM);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rng_filt_b.a_lut, CTL_IFE_ISP_RANGE_A_LUT_SIZE);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rng_filt_b.b_th, CTL_IFE_ISP_RANGE_B_TH_NUM);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rng_filt_b.b_lut, CTL_IFE_ISP_RANGE_B_LUT_SIZE);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rng_filt_ir.a_th, CTL_IFE_ISP_RANGE_A_TH_NUM);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rng_filt_ir.a_lut, CTL_IFE_ISP_RANGE_A_LUT_SIZE);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rng_filt_ir.b_th, CTL_IFE_ISP_RANGE_B_TH_NUM);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rng_filt_ir.b_lut, CTL_IFE_ISP_RANGE_B_LUT_SIZE);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, filt->center_mod.enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, filt->center_mod.th1);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, filt->center_mod.th2);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, filt->center_mod.cen_sel);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, filt->clamp.th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, filt->clamp.mul);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, filt->clamp.dlt);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, filt->rbfill.enable);
			if (filt->rbfill.enable) {
				PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rbfill.luma, CTL_IFE_ISP_RBFILL_LUMA_NUM);
				PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, filt->rbfill.ratio, CTL_IFE_ISP_RBFILL_RATIO_NUM);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, filt->rbfill.ratio_mode);
			}
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, filt->blend_w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, filt->rng_th_w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, filt->bin);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "filt does not need to update!! \r\n");
	}

	if (dgain != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, dgain->enable);
		if (dgain->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, dgain->dgain);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "dgain does not need to update!! \r\n");
	}

	if (cgain != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, cgain->enable);
		if (cgain->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, cgain->inv);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, cgain->hinv);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, cgain->bit_field);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, cgain->mask);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, cgain->cgain_r);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, cgain->cgain_gr);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, cgain->cgain_gb);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, cgain->cgain_b);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, cgain->cgain_ir);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, cgain->cofs_r);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, cgain->cofs_gr);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, cgain->cofs_gb);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, cgain->cofs_b);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, cgain->cofs_ir);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "cgain does not need to update!! \r\n");
	}

	if (vig != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, vig->enable);
		if (vig->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, vig->dist_th);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, vig->ch_r_lut, CTL_IFE_ISP_VIG_CH0_LUT_SIZE);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, vig->ch_gr_lut, CTL_IFE_ISP_VIG_CH1_LUT_SIZE);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, vig->ch_gb_lut, CTL_IFE_ISP_VIG_CH1_LUT_SIZE);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, vig->ch_b_lut, CTL_IFE_ISP_VIG_CH3_LUT_SIZE);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, vig->ch_ir_lut, CTL_IFE_ISP_VIG_CH2_LUT_SIZE);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, vig->dither_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, vig->dither_rst_enable);
			if (iq_info->final_setting.ipp.ife_cent != NULL) {
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, iq_info->final_setting.ipp.ife_cent->ch0.x);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, iq_info->final_setting.ipp.ife_cent->ch0.y);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, iq_info->final_setting.ipp.ife_cent->ch1.x);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, iq_info->final_setting.ipp.ife_cent->ch1.y);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, iq_info->final_setting.ipp.ife_cent->ch2.x);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, iq_info->final_setting.ipp.ife_cent->ch2.y);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, iq_info->final_setting.ipp.ife_cent->ch3.x);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, iq_info->final_setting.ipp.ife_cent->ch3.y);
				PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "\r\n");
			} else {
				PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "ife_cent does not need to update!! \r\n");
			}
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "vig does not need to update!! \r\n");
	}

	if (gbal != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, gbal->enable);
		if (gbal->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, gbal->protect_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, gbal->diff_th_str);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, gbal->diff_w_max);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, gbal->edge_protect_th1);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, gbal->edge_protect_th0);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, gbal->edge_w_max);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, gbal->edge_w_min);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, gbal->gbal_ofs, CTL_IFE_ISP_GBAL_OFS_NUM);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, gbal->str_luma_low_bnd);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, gbal->edge_luma_low_bnd);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "gbal does not need to update!! \r\n");
	}

	if (wdr != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->wdr_enable);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, wdr->ftrcoef, CTL_IFE_ISP_WDR_SUBIMG_FILT_NUM);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->input_bld.bld_sel);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, wdr->input_bld.blend_lut, CTL_IFE_ISP_WDR_INPUT_BLD_NUM);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, wdr->input_bld.in_yv_blend_lut, CTL_IFE_ISP_WDR_INPUT_YV_BLD_NUM);
		if (wdr->wdr_enable) {
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, wdr->wdr_str.wdr_coeff, CTL_IFE_ISP_WDR_COEF_NUM);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->wdr_str.strength);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->wdr_str.wdr_mode);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->wdr_str.wdr_anti_halo_opt);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->wdr_str.wdr_halo_ratio);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->wdr_str.wdr_halo_slope);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->wdr_str.wdr_b2p_var);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->gainctrl.gainctrl_en);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->gainctrl.max_gain);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->gainctrl.min_gain);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->outbld.outbld_en);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, wdr->outbld.outbld_lut.left_table, CTL_IFE_ISP_WDR_NEQ_TABLE_L_NUM);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, wdr->outbld.outbld_lut.right_table, CTL_IFE_ISP_WDR_NEQ_TABLE_R_NUM);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->sat_reduct.sat_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->sat_reduct.sat_wt_low);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->sat_reduct.sat_delta);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->dither.wdr_rand_rst);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->dither.wdr_rand_sel);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->fbc.fbc_en);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->fbc.fbc_ratio);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->fbc.fbc_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->wdr_gain_prot_str);
		}
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr->tonecurve_enable);
		if (wdr->tonecurve_enable) {
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, wdr->tonecurve.left_table, CTL_IFE_ISP_WDR_NEQ_TABLE_L_NUM);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, wdr->tonecurve.right_table, CTL_IFE_ISP_WDR_NEQ_TABLE_R_NUM);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "wdr_param does not need to update!! \r\n");
	}

	if (wdr_subimg != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr_subimg->subimg_size_h);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, wdr_subimg->subimg_size_v);
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "wdr_subimg_param does not need to update!! \r\n");
	}

	if (hist != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, hist->hist_enable);
		if (hist->hist_enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, hist->hist_sel);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, hist->step_h);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, hist->step_v);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "hist_param does not need to update!! \r\n");
	}

	if (va != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->enable)
		if (va->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->indep_va_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->ldg_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->gamma_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.h_filt.symmetry);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.h_filt.filter_size);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.h_filt.tap_a);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.h_filt.tap_b);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.h_filt.tap_c);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.h_filt.tap_d);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.h_filt.div);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.h_filt.th_l);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.h_filt.th_u);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.v_filt.symmetry);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.v_filt.filter_size);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.v_filt.tap_a);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.v_filt.tap_b);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.v_filt.tap_c);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.v_filt.tap_d);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.v_filt.div);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.v_filt.th_l);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.v_filt.th_u);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.iir_filt.symmetry_iir2);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.iir_filt.symmetry_iir3);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.iir_filt.tap_iir1_e);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.iir_filt.tap_iir1_f);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.iir_filt.iir1_shift_bit);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.iir_filt.iir2_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.iir_filt.tap_iir2_a);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.iir_filt.tap_iir2_b);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.iir_filt.tap_iir2_e);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.iir_filt.tap_iir2_f);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.iir_filt.iir2_shift_bit);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.iir_filt.iir3_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.iir_filt.tap_iir3_a);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.iir_filt.tap_iir3_b);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.iir_filt.tap_iir3_e);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.iir_filt.tap_iir3_f);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.iir_filt.iir3_shift_bit);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_1.count_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.h_filt.symmetry);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.h_filt.filter_size);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.h_filt.tap_a);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.h_filt.tap_b);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.h_filt.tap_c);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.h_filt.tap_d);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.h_filt.div);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.h_filt.th_l);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.h_filt.th_u);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.v_filt.symmetry);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.v_filt.filter_size);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.v_filt.tap_a);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.v_filt.tap_b);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.v_filt.tap_c);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.v_filt.tap_d);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.v_filt.div);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.v_filt.th_l);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.v_filt.th_u);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.iir_filt.symmetry_iir2);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.iir_filt.symmetry_iir3);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.iir_filt.tap_iir1_e);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.iir_filt.tap_iir1_f);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.iir_filt.iir1_shift_bit);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.iir_filt.iir2_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.iir_filt.tap_iir2_a);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.iir_filt.tap_iir2_b);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.iir_filt.tap_iir2_e);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.iir_filt.tap_iir2_f);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.iir_filt.iir2_shift_bit);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.iir_filt.iir3_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.iir_filt.tap_iir3_a);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.iir_filt.tap_iir3_b);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.iir_filt.tap_iir3_e);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.iir_filt.tap_iir3_f);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.iir_filt.iir3_shift_bit);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->group_2.count_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->fltr_sel.vdetgh1_filter_sel);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->fltr_sel.vdetgh2_filter_sel);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->va_out_grp1_2);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->win_num.w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->win_num.h);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->indep_win[0].enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->indep_win[1].enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->indep_win[2].enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->indep_win[3].enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->indep_win[4].enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->ldg_para.ldg_low_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->ldg_para.ldg_high_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->ldg_para.ldg_low_gain);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->ldg_para.ldg_high_gain);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->ldg_para.ldg_low_slope);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->ldg_para.ldg_high_slope);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->pre_filter_mode);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, va->va_gamma_lut,CTL_IFE_ISP_VA_GAMMA_LEN);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->win_cnt_out_sel);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->high_luma_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va->energy_w);
			if (va_win_size != NULL) {
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->ratio_base);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->winsz_ratio.w);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->winsz_ratio.h);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->indep_roi_ratio[0].x);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->indep_roi_ratio[0].y);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->indep_roi_ratio[0].w);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->indep_roi_ratio[0].h);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->indep_roi_ratio[1].x);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->indep_roi_ratio[1].y);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->indep_roi_ratio[1].w);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->indep_roi_ratio[1].h);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->indep_roi_ratio[2].x);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->indep_roi_ratio[2].y);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->indep_roi_ratio[2].w);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->indep_roi_ratio[2].h);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->indep_roi_ratio[3].x);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->indep_roi_ratio[3].y);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->indep_roi_ratio[3].w);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->indep_roi_ratio[3].h);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->indep_roi_ratio[4].x);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->indep_roi_ratio[4].y);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->indep_roi_ratio[4].w);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, va_win_size->indep_roi_ratio[4].h);
			}
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "va does not need to update!! \r\n");
	}

	if (subisp != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, subisp->subisp_cst_en);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, subisp->coef, CTL_IFE_ISP_SUBISP_COEF_LEN);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, subisp->subisp_gamma_en);
		if (subisp->subisp_cst_en) {
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, subisp->subisp_gamma_lut, CTL_IFE_ISP_SUBISP_GAMMA_LEN);
		}
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, subisp->subisp_cfa_en);
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "subisp_param does not need to update!! \r\n");
	}

	if (fpn != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fpn->fpn_en);
		if (fpn->fpn_en) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, fpn->fpn_cgain_range);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fpn->fpn_cgain_r, CTL_IFE_ISP_FPN_PATH_MAX);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fpn->fpn_cgain_gr, CTL_IFE_ISP_FPN_PATH_MAX);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fpn->fpn_cgain_gb, CTL_IFE_ISP_FPN_PATH_MAX);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fpn->fpn_cgain_b, CTL_IFE_ISP_FPN_PATH_MAX);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fpn->fpn_cgain_ir, CTL_IFE_ISP_FPN_PATH_MAX);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fpn->fpn_cofs_r, CTL_IFE_ISP_FPN_PATH_MAX);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fpn->fpn_cofs_gr, CTL_IFE_ISP_FPN_PATH_MAX);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fpn->fpn_cofs_gb, CTL_IFE_ISP_FPN_PATH_MAX);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fpn->fpn_cofs_b, CTL_IFE_ISP_FPN_PATH_MAX);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IFE, fpn->fpn_cofs_ir, CTL_IFE_ISP_FPN_PATH_MAX);
			if (iq_info->final_setting.ipp.ife_fpn_info != NULL) {
				PRINT_IQ_VAR_HEX(dbg_mode & IQ_DBG_P_IFE, iq_info->final_setting.ipp.ife_fpn_info->buf_phyaddr);
				PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IFE, iq_info->final_setting.ipp.ife_fpn_info->buf_size);
				PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "\r\n");
			} else {
				PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "ife_cent does not need to update!! \r\n");
			}
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IFE, "fpn_param does not need to update!! \r\n");
	}

	iq_dbg_clr_dbg_mode(iq_info->id, IQ_DBG_P_IFE);
}

void iq_msg_ipe(IQALG_INFO *iq_info)
{
	UINT32 dbg_mode = iq_dbg_get_dbg_mode(iq_info->id);
	CTL_IPE_ISP_EEXT *eext = iq_info->final_setting.ipe.p_eext;
	CTL_IPE_ISP_EEXT_TONEMAP *eext_tonemap = iq_info->final_setting.ipe.p_eext_tonemap;
	CTL_IPE_ISP_EDGE_OVERSHOOT *edge_overshoot = iq_info->final_setting.ipe.p_edge_overshoot;
	CTL_IPE_ISP_EPROC *eproc = iq_info->final_setting.ipe.p_eproc;
	CTL_IPE_ISP_RGBLPF *rgb_lpf = iq_info->final_setting.ipe.p_rgb_lpf;
	CTL_IPE_ISP_PFR *pfr = iq_info->final_setting.ipe.p_pfr;
	CTL_IPE_ISP_CC *cc = iq_info->final_setting.ipe.p_cc;
	CTL_IPE_ISP_CCM *ccm = iq_info->final_setting.ipe.p_ccm;
	CTL_IPE_ISP_CCTRL *cctrl = iq_info->final_setting.ipe.p_cctrl;
	CTL_IPE_ISP_CADJ_EE *cadj_ee = iq_info->final_setting.ipe.p_cadj_ee;
	CTL_IPE_ISP_CADJ_YCCON *cadj_yccon = iq_info->final_setting.ipe.p_cadj_yccon;
	CTL_IPE_ISP_CADJ_COFS *cadj_cofs = iq_info->final_setting.ipe.p_cadj_cofs;
	CTL_IPE_ISP_CADJ_RAND *cadj_rand = iq_info->final_setting.ipe.p_cadj_rand;
	CTL_IPE_ISP_CADJ_HUE *cadj_hue = iq_info->final_setting.ipe.p_cadj_hue;
	CTL_IPE_ISP_CADJ_FIXTH *cadj_fixth = iq_info->final_setting.ipe.p_cadj_fixth;
	CTL_IPE_ISP_CADJ_MASK *cadj_mask = iq_info->final_setting.ipe.p_cadj_mask;
	CTL_IPE_ISP_CST *cst = iq_info->final_setting.ipe.p_cst;
	CTL_IPE_ISP_CSTP *cstp = iq_info->final_setting.ipe.p_cstp;
	CTL_IPE_ISP_GAMYRAND *gamy_rand = iq_info->final_setting.ipe.p_gamy_rand;
	CTL_IPE_ISP_GAMMA *gamma = iq_info->final_setting.ipe.p_gamma;
	CTL_IPE_ISP_YCURVE *y_curve = iq_info->final_setting.ipe.p_y_curve;
	CTL_IPE_ISP_DEFOG *defog = iq_info->final_setting.ipe.p_defog;
	CTL_IPE_ISP_LCE *lce = iq_info->final_setting.ipe.p_lce;
	CTL_IPE_ISP_SUBIMG *subimg = iq_info->final_setting.ipe.p_subimg;
	CTL_IPE_ISP_EDGEDBG *edgedbg = iq_info->final_setting.ipe.p_edgedbg;
	CTL_IPE_ISP_VA *va = iq_info->final_setting.ipe.p_va;
	CTL_IPE_ISP_EDGE_REGION_STR *edge_region = iq_info->final_setting.ipe.p_edge_region_str;
	CTL_IPP_ISP_IPE_VA_WIN_SIZE_RATIO *va_win_size = iq_info->final_setting.ipp.ipe_va_win_size;
	CTL_IPE_ISP_CFA *cfa = iq_info->final_setting.ipe.p_cfa;
	CTL_IPE_ISP_3DCC *_3dcc = iq_info->final_setting.ipe.p_3dcc;

	CTL_IPE_ISP_GAMMA *gamma_param = &iq_info->final_ipp.ipe_gamma;
	CTL_IPE_ISP_YCURVE *y_curve_param = &iq_info->final_ipp.ipe_ycurve;

	UINT32 i;

	if (eext != NULL) {
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, eext->edge_ker, CTL_IPE_ISP_EDGE_KER_DIV_LEN);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_kerstrength.ker_freq0.eext_enh);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_kerstrength.ker_freq0.eext_div);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_kerstrength.ker_freq1.eext_enh);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_kerstrength.ker_freq1.eext_div);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_kerstrength.ker_freq2.eext_enh);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_kerstrength.ker_freq2.eext_div);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_engcon.eext_div_con);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_engcon.eext_div_eng);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_engcon.wt_con_eng);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->ker_thickness.wt_ker_thin);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->ker_thickness.wt_ker_robust);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->ker_thickness.iso_ker_thin);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->ker_thickness.iso_ker_robust);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->ker_thickness_hld.wt_ker_thin);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->ker_thickness_hld.wt_ker_robust);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->ker_thickness_hld.iso_ker_thin);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->ker_thickness_hld.iso_ker_robust);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_region.reg_th.th_flat);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_region.reg_th.th_edge);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_region.reg_th.th_flat_hld);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_region.reg_th.th_edge_hld);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_region.reg_th.th_lum_hld);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_region.reg_wt.wt_low);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_region.reg_wt.wt_high);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_region.reg_wt.wt_low_hld);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_region.reg_wt.wt_high_hld);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_blending.eext_blending_th);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_blending.eext_blending_w1);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_blending.eext_blending_w2);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->eext_blending.eext_blending_slope);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->dir_ker_strength.ker_h.eext_enh);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->dir_ker_strength.ker_h.eext_div);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->dir_ker_strength.ker_v.eext_enh);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->dir_ker_strength.ker_v.eext_div);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->dir_ker_strength.ker_d1.eext_enh);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->dir_ker_strength.ker_d1.eext_div);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->dir_ker_strength.ker_d2.eext_enh);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->dir_ker_strength.ker_d2.eext_div);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->dir_ker_para.eext_dir_min_s_low_bound);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->dir_ker_para.eext_dir_s_th_mul);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->dir_ker_para.eext_dir_s_th_shift);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->dir_ker_para.dir_eng_blend_w1);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->dir_ker_para.dir_eng_blend_w2);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->dir_ker_para.dir_eng_blend_w3);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->dir_ker_para.dir_eng_blend_w4);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext->dir_ker_para.s_count_th);
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "eext does not need to update!! \r\n");
	}

	if (eext_tonemap != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eext_tonemap->gamma_sel);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, eext_tonemap->tone_map_lut, CTL_IPE_ISP_TONE_MAP_LUT_LEN);
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "eext_tonemap does not need to update!! \r\n");
	}

	if (edge_overshoot != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_overshoot->overshoot_en);
		if (edge_overshoot->overshoot_en) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_overshoot->wt_overshoot);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_overshoot->wt_undershoot);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_overshoot->th_overshoot);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_overshoot->th_undershoot);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_overshoot->th_undershoot_lum);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_overshoot->th_undershoot_eng);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_overshoot->slope_overshoot);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_overshoot->slope_undershoot);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_overshoot->slope_undershoot_lum);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_overshoot->slope_undershoot_eng);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_overshoot->clamp_wt_mod_lum);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_overshoot->clamp_wt_mod_eng);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_overshoot->strength_lum_eng);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_overshoot->norm_lum_eng);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "edge_overshoot does not need to update!! \r\n");
	}

	if (eproc != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eproc->edge_map_th.ethr_low);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eproc->edge_map_th.ethr_high);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eproc->edge_map_th.etab_low);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eproc->edge_map_th.etab_high);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eproc->edge_map_th.map_sel);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, eproc->edge_map_lut, CTL_IPE_ISP_EDGE_MAP_LUT_LEN);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eproc->es_map_th.ethr_low);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eproc->es_map_th.ethr_high);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eproc->es_map_th.etab_low);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, eproc->es_map_th.etab_high);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, eproc->es_map_lut, CTL_IPE_ISP_ES_MAP_LUT_LEN);
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "eproc does not need to update!! \r\n");
	}

	if (rgb_lpf != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, rgb_lpf->enable);
		if (rgb_lpf->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, rgb_lpf->lpf_param_r.lpf_w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, rgb_lpf->lpf_param_r.s_only_w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, rgb_lpf->lpf_param_r.range_th0);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, rgb_lpf->lpf_param_r.range_th1);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, rgb_lpf->lpf_param_r.filt_size);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, rgb_lpf->lpf_param_g.lpf_w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, rgb_lpf->lpf_param_g.s_only_w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, rgb_lpf->lpf_param_g.range_th0);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, rgb_lpf->lpf_param_g.range_th1);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, rgb_lpf->lpf_param_g.filt_size);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, rgb_lpf->lpf_param_b.lpf_w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, rgb_lpf->lpf_param_b.s_only_w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, rgb_lpf->lpf_param_b.range_th0);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, rgb_lpf->lpf_param_b.range_th1);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, rgb_lpf->lpf_param_b.filt_size);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "rgb_lpf does not need to update!! \r\n");
	}

	if (pfr != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->enable);
		if (pfr->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->uv_filt_en);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->luma_level_en);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->out_wet);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->pfr_strength);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->edge_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->edge_str);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->luma_th);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, pfr->luma_lut, CTL_IPE_ISP_PFR_LUMA_LEN);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_g);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[0].enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[0].color_u);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[0].color_v);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[0].color_wet_r);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[0].color_wet_b);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[0].cdiff_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[0].cdiff_step);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[0].cdiff_lut, CTL_IPE_ISP_PFR_COLOR_LEN);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[1].enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[1].color_u);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[1].color_v);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[1].color_wet_r);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[1].color_wet_b);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[1].cdiff_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[1].cdiff_step);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[1].cdiff_lut, CTL_IPE_ISP_PFR_COLOR_LEN);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[2].enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[2].color_u);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[2].color_v);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[2].color_wet_r);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[2].color_wet_b);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[2].cdiff_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[2].cdiff_step);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[2].cdiff_lut, CTL_IPE_ISP_PFR_COLOR_LEN);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[3].enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[3].color_u);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[3].color_v);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[3].color_wet_r);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[3].color_wet_b);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[3].cdiff_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[3].cdiff_step);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, pfr->color_wet_set[3].cdiff_lut, CTL_IPE_ISP_PFR_COLOR_LEN);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "pfr does not need to update!! \r\n");
	}

	if ((cc != NULL) && (ccm != NULL)) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cc->enable);
		if (cc->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cc->cc2_sel);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cc->cc_stab_sel);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, cc->fstab, CTL_IPE_ISP_FTAB_LEN);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, cc->fdtab, CTL_IPE_ISP_FTAB_LEN);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, ccm->cc_range);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, ccm->cc_gamma_sel);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, ccm->coef, CTL_IPE_ISP_COEF_LEN);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "cc does not need to update!! \r\n");
	}

	if (cctrl != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cctrl->enable);
		if (cctrl->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cctrl->int_ofs);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cctrl->sat_ofs);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cctrl->hue_c2g);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cctrl->cctrl_sel);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cctrl->vdet_div);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, cctrl->edge_tab, CTL_IPE_ISP_CCTRL_TAB_LEN);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, cctrl->dds_tab, CTL_IPE_ISP_DDS_TAB_LEN);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cctrl->hue_rotate_en);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, cctrl->hue_tab, CTL_IPE_ISP_CCTRL_TAB_LEN);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, cctrl->sat_tab, CTL_IPE_ISP_CCTRL_TAB_LEN);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, cctrl->int_tab, CTL_IPE_ISP_CCTRL_TAB_LEN);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "cctrl does not need to update!! \r\n");
	}

	if (cadj_ee != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_ee->enable);
		if (cadj_ee->enable) {
			PRINT_IQ_VAR(dbg_mode & (IQ_DBG_P_IPE | IQ_DBG_O_EDGE), cadj_ee->edge_enh_p);
			PRINT_IQ_VAR(dbg_mode & (IQ_DBG_P_IPE | IQ_DBG_O_EDGE), cadj_ee->edge_enh_n);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_ee->edge_inv_p_en);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_ee->edge_inv_n_en);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "cadj_ee does not need to update!! \r\n");
	}

	if (cadj_yccon != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_yccon->enable);
		if (cadj_yccon->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_yccon->y_con);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_yccon->c_con);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, cadj_yccon->cconlut, CTL_IPE_ISP_CCONTAB_LEN);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_yccon->ccontab_sel);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "cadj_yccon does not need to update!! \r\n");
	}

	if (cadj_cofs != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_cofs->enable);
		if (cadj_cofs->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_cofs->cb_ofs);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_cofs->cr_ofs);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "cadj_cofs does not need to update!! \r\n");
	}

	if (cadj_rand != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_rand->enable);
		if (cadj_rand->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_rand->rand_en_y);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_rand->rand_en_c);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_rand->rand_level_y);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_rand->rand_level_c);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_rand->rand_reset);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "cadj_rand does not need to update!! \r\n");
	}

	if (cadj_hue != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_hue->enable);
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "cadj_hue does not need to update!! \r\n");
	}

	if (cadj_fixth != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->enable);
		if (cadj_fixth->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->yth1.y_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->yth1.edge_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->yth1.ycth_sel_hit);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->yth1.ycth_sel_nonhit);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->yth1.value_hit);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->yth1.nonvalue_hit);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->yth2.y_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->yth2.ycth_sel_hit);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->yth2.ycth_sel_nonhit);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->yth2.value_hit);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->yth2.nonvalue_hit);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->cth.edge_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->cth.y_th_low);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->cth.y_th_high);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->cth.cb_th_low);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->cth.cb_th_high);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->cth.cr_th_low);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->cth.cr_th_high);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->cth.ycth_sel_hit);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->cth.ycth_sel_nonhit);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->cth.cb_value_hit);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->cth.cb_value_nonhit);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->cth.cr_value_hit);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_fixth->cth.cr_value_nonhit);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "cadj_fixth does not need to update!! \r\n");
	}

	if (cadj_mask != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_mask->enable);
		if (cadj_mask->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_mask->y_mask);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_mask->cb_mask);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cadj_mask->cr_mask);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "cadj_mask does not need to update!! \r\n");
	}

	if (cst != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cst->enable);
		if (cst->enable) {
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, cst->cst_coef, CTL_IPE_ISP_COEF_LEN);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cst->cst_off_sel);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "cst does not need to update!! \r\n");
	}

	if (cstp != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cstp->enable);
		if (cstp->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cstp->cstp_ratio);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "cstp does not need to update!! \r\n");
	}

	if (gamy_rand != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, gamy_rand->enable);
		if (gamy_rand->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, gamy_rand->rand_en);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, gamy_rand->rst_en);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, gamy_rand->rand_shift);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "gamy_rand does not need to update!! \r\n");
	}

	if ((iq_info->ipp_tab_update & IQ_TBL_IPE_GAMMA) && (gamma != NULL)) {
		PRINT_IQ_VAR((dbg_mode & IQ_DBG_P_IPE) || (dbg_mode & IQ_DBG_GAMMA), gamma->enable);
		if (gamma->enable) {
			PRINT_IQ_VAR((dbg_mode & IQ_DBG_P_IPE), gamma->option);
			PRINT_IQ_ARR((dbg_mode & IQ_DBG_P_IPE) || (dbg_mode & IQ_DBG_GAMMA), gamma->lut.gamma_lut[CTL_IPE_ISP_RGB_R], CTL_IPE_ISP_GAMMA_LEN);
			PRINT_IQ_ARR((dbg_mode & IQ_DBG_P_IPE), gamma->lut.gamma_lut[CTL_IPE_ISP_RGB_G], CTL_IPE_ISP_GAMMA_LEN);
			PRINT_IQ_ARR((dbg_mode & IQ_DBG_P_IPE), gamma->lut.gamma_lut[CTL_IPE_ISP_RGB_B], CTL_IPE_ISP_GAMMA_LEN);
		}
		PRINT_IQ((dbg_mode & IQ_DBG_P_IPE) || (dbg_mode & IQ_DBG_GAMMA), "\r\n");
		//iq_dbg_clr_dbg_mode(iq_info->id, IQ_DBG_GAMMA);
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "not update, print previous setting!! \r\n");
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, gamma_param->enable);
		if (gamma_param->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, gamma_param->option);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, gamma_param->lut.gamma_lut[CTL_IPE_ISP_RGB_R], CTL_IPE_ISP_GAMMA_LEN);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, gamma_param->lut.gamma_lut[CTL_IPE_ISP_RGB_G], CTL_IPE_ISP_GAMMA_LEN);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, gamma_param->lut.gamma_lut[CTL_IPE_ISP_RGB_B], CTL_IPE_ISP_GAMMA_LEN);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	}

	if ((iq_info->ipp_tab_update & IQ_TBL_IPE_YCURVE) && (y_curve != NULL)) {
		PRINT_IQ_VAR((dbg_mode & IQ_DBG_P_IPE) || (dbg_mode & IQ_DBG_TABLE), y_curve->enable);
		if (y_curve->enable) {
			PRINT_IQ_VAR((dbg_mode & IQ_DBG_P_IPE) || (dbg_mode & IQ_DBG_TABLE), y_curve->ycurve_sel);
			PRINT_IQ_ARR((dbg_mode & IQ_DBG_P_IPE) || (dbg_mode & IQ_DBG_TABLE), y_curve->lut.y_curve_lut, CTL_IPE_ISP_YCURVE_LEN);
		}
		PRINT_IQ((dbg_mode & IQ_DBG_P_IPE) || (dbg_mode & IQ_DBG_TABLE), "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "not update, print previous setting!! \r\n");
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, y_curve_param->enable);
		if (y_curve_param->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, y_curve_param->ycurve_sel);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, y_curve_param->lut.y_curve_lut, CTL_IPE_ISP_YCURVE_LEN);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	}

	if (defog != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, defog->enable);
		if (defog->enable) {
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, defog->scalup_param.interp_diff_lut, CTL_IPE_ISP_DFG_INTERP_DIFF_LEN);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, defog->scalup_param.interp_wdist);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, defog->scalup_param.interp_wout);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, defog->scalup_param.interp_wcenter);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, defog->scalup_param.interp_wsrc);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, defog->input_bld.in_blend_wt, CTL_IPE_ISP_DFG_INPUT_BLD_LEN);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, defog->env_estimation.dfg_self_comp_en);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, defog->env_estimation.dfg_min_diff);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, defog->env_estimation.dfg_airlight, CTL_IPE_ISP_DFG_AIRLIGHT_NUM);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, defog->env_estimation.fog_mod_lut, CTL_IPE_ISP_DFG_FOG_MOD_LEN);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, defog->dfg_strength.str_mode_sel);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, defog->dfg_strength.target_lut, CTL_IPE_ISP_DFG_TARGET_LEN);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, defog->dfg_strength.fog_ratio);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, defog->dfg_strength.dgain_ratio);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, defog->dfg_strength.gain_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, defog->dfg_outbld.outbld_ref_sel);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, defog->dfg_outbld.outbld_local_en);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, defog->dfg_outbld.outbld_lum_wt, CTL_IPE_ISP_DFG_OUTPUT_BLD_LEN);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, defog->dfg_outbld.outbld_diff_wt, CTL_IPE_ISP_DFG_OUTPUT_BLD_LEN);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, defog->dfg_stcs.airlight_stcs_ratio);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, defog->dfg_round.rand_opt);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, defog->dfg_round.rand_rst);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "defog does not need to update!! \r\n");
	}

	if (lce != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, lce->enable);
		if (lce->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, lce->diff_wt_pos);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, lce->diff_wt_neg);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, lce->diff_wt_avg);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, lce->lum_wt_lut, CTL_IPE_ISP_LCE_LUMA_LEN);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "lce does not need to update!! \r\n");
	}

	if (subimg != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, subimg->subimg_size.h_size);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, subimg->subimg_size.v_size);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, subimg->subimg_ftrcoef, CTL_IPE_ISP_SUBIMG_FILT_LEN);
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "subimg does not need to update!! \r\n");
	}

	if (edgedbg != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edgedbg->enable);
		if (edgedbg->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edgedbg->mode_sel);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "edgedbg does not need to update!! \r\n");
	}

	if (va != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->enable);
		if (va->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->indep_va_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->ldg_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_1.h_filt.symmetry);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_1.h_filt.filter_size);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_1.h_filt.tap_a);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_1.h_filt.tap_b);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_1.h_filt.tap_c);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_1.h_filt.tap_d);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_1.h_filt.div);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_1.h_filt.th_l);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_1.h_filt.th_u);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_1.v_filt.symmetry);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_1.v_filt.filter_size);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_1.v_filt.tap_a);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_1.v_filt.tap_b);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_1.v_filt.tap_c);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_1.v_filt.tap_d);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_1.v_filt.div);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_1.v_filt.th_l);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_1.v_filt.th_u);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_1.count_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_2.h_filt.symmetry);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_2.h_filt.filter_size);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_2.h_filt.tap_a);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_2.h_filt.tap_b);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_2.h_filt.tap_c);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_2.h_filt.tap_d);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_2.h_filt.div);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_2.h_filt.th_l);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_2.h_filt.th_u);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_2.v_filt.symmetry);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_2.v_filt.filter_size);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_2.v_filt.tap_a);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_2.v_filt.tap_b);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_2.v_filt.tap_c);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_2.v_filt.tap_d);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_2.v_filt.div);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_2.v_filt.th_l);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_2.v_filt.th_u);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->group_2.count_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->va_out_grp1_2);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->win_num.w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->win_num.h);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->indep_win[0].enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->indep_win[1].enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->indep_win[2].enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->indep_win[3].enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->indep_win[4].enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->ldg_para.ldg_low_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->ldg_para.ldg_high_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->ldg_para.ldg_low_gain);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->ldg_para.ldg_high_gain);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->ldg_para.ldg_low_slope);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->ldg_para.ldg_high_slope);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->pre_filter_mode);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->win_cnt_out_sel);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->high_luma_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va->energy_w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->ratio_base);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->winsz_ratio.w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->winsz_ratio.h);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->indep_roi_ratio[0].x);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->indep_roi_ratio[0].y);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->indep_roi_ratio[0].w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->indep_roi_ratio[0].h);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->indep_roi_ratio[1].x);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->indep_roi_ratio[1].y);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->indep_roi_ratio[1].w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->indep_roi_ratio[1].h);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->indep_roi_ratio[2].x);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->indep_roi_ratio[2].y);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->indep_roi_ratio[2].w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->indep_roi_ratio[2].h);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->indep_roi_ratio[3].x);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->indep_roi_ratio[3].y);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->indep_roi_ratio[3].w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->indep_roi_ratio[3].h);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->indep_roi_ratio[4].x);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->indep_roi_ratio[4].y);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->indep_roi_ratio[4].w);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, va_win_size->indep_roi_ratio[4].h);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "va does not need to update!! \r\n");
	}

	if (edge_region != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_region->enable);
		if (edge_region->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_region->enh_thin);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_region->enh_robust);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_region->slope_flat);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_region->slope_edge);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_region->str_flat);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, edge_region->str_edge);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "edge_region does not need to update!! \r\n");
	}

	if (cfa != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_enable);
		if (cfa->cfa_enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_interp.edge_dth);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_interp.edge_dth2);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_interp.freq_th);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_interp.freq_lut, CTL_IPE_ISP_CFA_FREQ_NUM);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_interp.luma_wt, CTL_IPE_ISP_CFA_FREQ_NUM);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_correction.rb_corr_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_correction.rb_corr_th1);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_correction.rb_corr_th2);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_fcs.fcs_dirsel);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_fcs.fcs_coring);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_fcs.fcs_weight);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_fcs.fcs_strength, CTL_IPE_ISP_CFA_FCS_NUM);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_ir_hfc.cl_check_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_ir_hfc.hf_check_enable);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_ir_hfc.average_mode);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_ir_hfc.cl_sel);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_ir_hfc.cl_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_ir_hfc.hf_gth);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_ir_hfc.hf_diff);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_ir_hfc.hf_eth);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_ir_hfc.ir_g_edge_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_ir_hfc.ir_rb_cstrength);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_ir_sub.ir_sub_r);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_ir_sub.ir_sub_g);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_ir_sub.ir_sub_b);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_ir_sub.ir_sub_wt_lb);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_ir_sub.ir_sub_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_ir_sub.ir_sub_range);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_ir_sub.ir_sat_gain);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_pink_reduc.pink_rd_en);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_pink_reduc.pink_rd_mode);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_pink_reduc.pink_rd_th1);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_pink_reduc.pink_rd_th2);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_pink_reduc.pink_rd_th3);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_pink_reduc.pink_rd_th4);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_cgain.r_gain);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_cgain.g_gain);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_cgain.b_gain);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IPE, cfa->cfa_cgain.gain_range);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IPE, "cfa_param does not need to update!! \r\n");
	}

	PRINT_IQ_VAR((dbg_mode & IQ_DBG_P_IPE) || (dbg_mode & IQ_DBG_TABLE), _3dcc->enable);
	if (_3dcc->enable) {
		for (i = 0; i < 30; i++) {
			PRINT_IQ((dbg_mode & IQ_DBG_P_IPE) || (dbg_mode & IQ_DBG_TABLE), "_3dcc->lut.rgb_3d_lut[%d] = 0x%08x \r\n", i, *((UINT32 *)_3dcc->lut.rgb_3d_lut + i));
		}
	}
	PRINT_IQ((dbg_mode & IQ_DBG_P_IPE) || (dbg_mode & IQ_DBG_TABLE), "\r\n");

	iq_dbg_clr_dbg_mode(iq_info->id, IQ_DBG_P_IPE);
}

void iq_msg_ime(IQALG_INFO *iq_info)
{
	UINT32 dbg_mode = iq_dbg_get_dbg_mode(iq_info->id);
	CTL_IME_ISP_LCA *lca = iq_info->final_setting.ime.p_lca;
	CTL_IME_ISP_DBCS *dbcs = iq_info->final_setting.ime.p_dbcs;
	CTL_IME_ISP_TMNR *tmnr = iq_info->final_setting.ime.p_tmnr;
	CTL_IME_ISP_YCC_CVT *ycccvt = iq_info->final_setting.ime.p_ycccvt;
	CTL_IME_ISP_SHARPEN *sharpen = iq_info->final_setting.ime.p_sharpen;

	if (lca != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, lca->rf.edge_ker_size_sel);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->rf.edge_th, 2);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, lca->rf.sr_ker_size_sel);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, lca->rf.mr_ker_size_sel);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->rf.ctr_3x3_y_th, CTL_IME_ISP_RF_CTR_TH_LEN);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->rf.ctr_3x3_u_th, CTL_IME_ISP_RF_CTR_TH_LEN);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->rf.ctr_3x3_v_th, CTL_IME_ISP_RF_CTR_TH_LEN);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, lca->rf.sr_y_wet);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, lca->rf.sr_uv_wet);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, lca->rf.er_y_wet);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, lca->rf.er_uv_wet);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->rf.ss_region_y_th, CTL_IME_ISP_RF_SS_RGN_TH_LEN);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->rf.ss_region_uv_th, CTL_IME_ISP_RF_SS_RGN_TH_LEN);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->rf.edge_region_y_th, CTL_IME_ISP_RF_EDG_RGN_TH_LEN);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->rf.edge_region_uv_th, CTL_IME_ISP_RF_EDG_RGN_TH_LEN);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->rf.motion_region_y_th, CTL_IME_ISP_RF_MON_RGN_TH_LEN);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->rf.motion_region_uv_th, CTL_IME_ISP_RF_MON_RGN_TH_LEN);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->coring_gain.still_y_gain, CTL_IME_ISP_CORING_GAIN_LEN);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->coring_gain.motion_y_gain, CTL_IME_ISP_CORING_GAIN_LEN);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->coring_gain.still_u_gain, CTL_IME_ISP_CORING_GAIN_LEN);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->coring_gain.motion_u_gain, CTL_IME_ISP_CORING_GAIN_LEN);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->coring_gain.still_v_gain, CTL_IME_ISP_CORING_GAIN_LEN);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->coring_gain.motion_v_gain, CTL_IME_ISP_CORING_GAIN_LEN);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->cutout_coff.still_y_coff, CTL_IME_ISP_CUTOUT_LEN);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->cutout_coff.motion_y_coff, CTL_IME_ISP_CUTOUT_LEN);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->cutout_coff.still_u_coff, CTL_IME_ISP_CUTOUT_LEN);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->cutout_coff.motion_u_coff, CTL_IME_ISP_CUTOUT_LEN);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->cutout_coff.still_v_coff, CTL_IME_ISP_CUTOUT_LEN);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, lca->cutout_coff.motion_v_coff, CTL_IME_ISP_CUTOUT_LEN);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, lca->final_y_out_wt);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, lca->final_uv_out_wt);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, lca->dbg.enable);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, lca->dbg.ch_sel);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, lca->dbg.ch_ofs);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, lca->dbg.x_pos);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, lca->set_proc_location);
		PRINT_IQ(dbg_mode & IQ_DBG_P_IME, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IME, "lca does not need to update!! \r\n");
	}

	if (dbcs != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, dbcs->enable);
		if (dbcs->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, dbcs->op_mode);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, dbcs->cent_u);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, dbcs->cent_v);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, dbcs->step_y);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, dbcs->step_c);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, dbcs->wt_y, CTL_IME_ISP_DBCS_WT_LUT_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, dbcs->wt_c, CTL_IME_ISP_DBCS_WT_LUT_TAB);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IME, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IME, "dbcs does not need to update!! \r\n");
	}

	if (tmnr != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->enable);
		if (tmnr->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->me_param.update_mode);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->me_param.boundary_set);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->me_param.sad_shift);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->me_param.cost_blend);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->me_param.rand_bit_x);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->me_param.rand_bit_y);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->me_param.min_detail);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->me_param.sad_penalty, CTL_IME_ISP_TMNR_ME_SAD_PENALTY_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->me_param.switch_th, CTL_IME_ISP_TMNR_ME_SWITCH_THRESHOLD_TAB);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->me_param.switch_rto);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->me_param.detail_penalty, CTL_IME_ISP_TMNR_ME_DETAIL_PENALTY_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->me_param.probability, CTL_IME_ISP_TMNR_ME_PROBABILITY_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->md_param.sad_coefa, CTL_IME_ISP_TMNR_MD_SAD_COEFA_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->md_param.sad_coefb, CTL_IME_ISP_TMNR_MD_SAD_COEFB_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->md_param.sad_std, CTL_IME_ISP_TMNR_MD_SAD_STD_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->md_param.fth, CTL_IME_ISP_TMNR_MD_FINAL_THRESHOLD_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->md_roi_param.fth, CTL_IME_ISP_TMNR_MD_ROI_FINAL_THRESHOLD_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->mc_param.sad_base, CTL_IME_ISP_TMNR_MC_SAD_BASE_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->mc_param.sad_coefa, CTL_IME_ISP_TMNR_MC_SAD_COEFA_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->mc_param.sad_coefb, CTL_IME_ISP_TMNR_MC_SAD_COEFB_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->mc_param.sad_std, CTL_IME_ISP_TMNR_MC_SAD_STD_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->mc_param.fth, CTL_IME_ISP_TMNR_MC_FINAL_THRESHOLD_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->mc_roi_param.fth, CTL_IME_ISP_TMNR_MC_ROI_FINAL_THRESHOLD_TAB);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->ps_param.smart_roi_ctrl_en);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->ps_param.mv_check_en);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->ps_param.roi_mv_check_en);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->ps_param.mv_info_mode);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->ps_param.mv_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->ps_param.roi_mv_th);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->ps_param.mix_ratio, CTL_IME_ISP_TMNR_PS_MIX_RATIO_TAB);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->ps_param.ds_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->ps_param.ds_th_roi);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->ps_param.fs_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->ps_param.blur_eth);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.luma_ch_en);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.chroma_ch_en);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.center_wzeros_y);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.luma_residue_th, CTL_IME_ISP_TMNR_NR_LUMA_RESIDUE_TH_TAB);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.chroma_residue_th);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.freq_wet, CTL_IME_ISP_TMNR_NR_FREQ_WEIGHT_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.luma_wet, CTL_IME_ISP_TMNR_NR_LUMA_WEIGHT_TAB);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.pre_y_blur_str);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.pf_str);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.pre_filter_str, CTL_IME_ISP_TMNR_NR_PRE_FILTER_STRENGTH_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.pre_filter_rto, CTL_IME_ISP_TMNR_NR_PRE_FILTER_RATION_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.snr_str, CTL_IME_ISP_TMNR_NR_SFILTER_STRENGTH_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.tnr_str, CTL_IME_ISP_TMNR_NR_TFILTER_STRENGTH_TAB);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.snr_base_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.tnr_base_th);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.luma_3d_lut, CTL_IME_ISP_TMNR_NR_LUMA_LUT_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.luma_3d_rto, CTL_IME_ISP_TMNR_NR_LUMA_RATIO_TAB);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.luma_comp_str);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.chroma_3d_lut, CTL_IME_ISP_TMNR_NR_CHROMA_LUT_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.chroma_3d_rto, CTL_IME_ISP_TMNR_NR_CHROMA_RATIO_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.tf0_blur_str, CTL_IME_ISP_TMNR_NR_TF0_FILTER_TAB);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.tf0_blur_estr);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.tf0_y_str, CTL_IME_ISP_TMNR_NR_TF0_FILTER_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.tf0_c_str, CTL_IME_ISP_TMNR_NR_TF0_FILTER_TAB);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.u_tf0_md_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.v_tf0_md_th);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.c_tf0_ratio, CTL_IME_ISP_TMNR_NR_C_TF0_RATIO_TAB);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.motion_sat_ratio);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.c_tf0_tprot_th);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.cshk_th, CTL_IME_ISP_TMNR_NR_CSHK_TH_TAB);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, tmnr->nr_param.cshk_val, CTL_IME_ISP_TMNR_NR_CSHK_VAL);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->dbg_param.dbg_mv0);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->dbg_param.dbg_mode);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->fcvg_param.en);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->fcvg_param.start_point);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, tmnr->fcvg_param.step_size);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IME, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IME, "tmnr does not need to update!! \r\n");
	}

	if (ycccvt != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, ycccvt->enable);
		if (ycccvt->enable) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, ycccvt->cvt_sel);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IME, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IME, "ycccvt does not need to update!! \r\n");
	}

	if (sharpen != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->shp_en);
		if (sharpen->shp_en) {
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->motion_bit_en);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->dbg_en);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->shp_src_sel);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->jnd_filt_sel);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->weight_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->weight_gain);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->noise_lv);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->flat_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->edge_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->wet);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, sharpen->noise_curve, CTL_IME_ISP_NOISE_CURVE_LEN);
			PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_IME, sharpen->ewg_curve, CTL_IME_ISP_EWG_CURVE_LEN);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->slope);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->flat_region_str);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->edge_region_str);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->motion_str);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->static_str);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->trans_str);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->blend_inv_gamma);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->coring_th);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->edge_str);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->bright_halo_clip);
			PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_IME, sharpen->dark_halo_clip);
		}
		PRINT_IQ(dbg_mode & IQ_DBG_P_IME, "\r\n");
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_IME, "sharpen does not need to update!! \r\n");
	}

	PRINT_IQ(dbg_mode & IQ_DBG_P_IME, "\r\n");

	iq_dbg_clr_dbg_mode(iq_info->id, IQ_DBG_P_IME);
}

void iq_msg_enc(IQALG_INFO *iq_info)
{
	UINT32 dbg_mode = iq_dbg_get_dbg_mode(iq_info->id);
	KDRV_H26XENC_SPN *enc_sharpen = iq_info->final_setting.enc_sharpen;

	if (enc_sharpen != NULL) {
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->bEnable);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->ucConEng);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->usSlopConEng);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->ucBHC);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->ucDHC);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->ucEWT);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->ucEWG);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->ucEdgeSharpStr1);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->ucCT);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->ucNL);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->ucBIG);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->usFlatTh);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->usEdgeTh);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->ucEdgeStr);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->ucTransitionStr);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->ucMotionStr);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->ucStaticStr);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->ucFlatStr);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->ucNC, 17);
		PRINT_IQ_ARR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->usEWG, 9);
		PRINT_IQ_VAR(dbg_mode & IQ_DBG_P_ENC, enc_sharpen->bShowSharpInfo);
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_P_ENC, "enc_sharpen_param does not need to update!! \r\n");
	}

	PRINT_IQ(dbg_mode & IQ_DBG_P_ENC, "\r\n");

	iq_dbg_clr_dbg_mode(iq_info->id, IQ_DBG_P_ENC);
}

#if defined(__FREERTOS)
void iq_msg_show_info(void)
#else
void iq_msg_show_info(struct seq_file *sfile)
#endif
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;
	UINT32 version = iq_get_version();
	UINT32 id;

	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");
	PRINT_IQ_INFO(sfile, "NVT_IQ NT98538 v%d.%d.%d.%d \r\n", (version >> 24) & 0xFF, (version >> 16) & 0xFF, (version >> 8) & 0xFF, version & 0xFF);
	#if defined(__KERNEL__)
	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");
	PRINT_IQ_INFO(sfile, "    Module Info:       \n");
	PRINT_IQ_INFO(sfile, "     iq_id_list:  0x%X \n", iq_id_list);
	PRINT_IQ_INFO(sfile, "      iq_dpc_en:  0x%X \n", iq_dpc_en);
	PRINT_IQ_INFO(sfile, "      iq_ecs_en:  0x%X \n", iq_ecs_en);
	PRINT_IQ_INFO(sfile, "     iq_3dcc_en:  0x%X \n", iq_3dcc_en);
	#endif
	PRINT_IQ_INFO(sfile, "-----------------------------------------------------------------------------------------\r\n");
	PRINT_IQ_INFO(sfile, "          id :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->id);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");
	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");

	PRINT_IQ_INFO(sfile, " SIE    mode :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			if (iq_info->sie_proc_mode == IQ_PROC_MOVIE) {
				PRINT_IQ_INFO(sfile, "     MOVIE");
			} else if (iq_info->sie_proc_mode == IQ_PROC_PHOTO) {
				PRINT_IQ_INFO(sfile, "     PHOTO");
			} else if (iq_info->sie_proc_mode == IQ_PROC_CAPTURE) {
				PRINT_IQ_INFO(sfile, "       CAP");
			} else {
				PRINT_IQ_INFO(sfile, "    UNKNOW");
			}
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, " SIE    flow :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			if (iq_info->sie_flow_mode == IQ_FLOW_LINEAR) {
				PRINT_IQ_INFO(sfile, "    LINEAR");
			} else if (iq_info->sie_flow_mode == IQ_FLOW_SHDR) {
				PRINT_IQ_INFO(sfile, "      SHDR");
			} else {
				PRINT_IQ_INFO(sfile, "    UNKNOW");
			}
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "    param_id :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->sie_param_id);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "      src_id :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->sie_trig_obj.dupl_src_id);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, " mul_last_id :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->mul_last_id);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "     id_mask :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->sie_trig_obj.src_id_mask);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "   frame_num :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->sie_trig_obj.frame_num);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "         iso :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_sie.sync_info.gain);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "       dgain :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_sie.sync_info.dgain);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "          lv :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_sie.sync_info.lv);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "     lv_base :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_sie.sync_info.lv_base);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "       Rgain :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_sie.sync_info.cgain[0]);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "       Ggain :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_sie.sync_info.cgain[1]);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "       Bgain :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_sie.sync_info.cgain[2]);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "          ct :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_sie.sync_info.ct);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "     EVratio :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_sie.sync_info.shdr_ev_ratio[0]);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");
	PRINT_IQ_INFO(sfile, "             :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_sie.sync_info.shdr_ev_ratio[1]);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "     TMratio :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_sie.sync_info.shdr_tm_ratio);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");
	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");

	PRINT_IQ_INFO(sfile, " IPP    flow :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			if (iq_info->ipp_flow_mode == IQ_FLOW_LINEAR) {
				PRINT_IQ_INFO(sfile, "    LINEAR");
			} else if (iq_info->ipp_flow_mode == IQ_FLOW_SHDR) {
				PRINT_IQ_INFO(sfile, "      SHDR");
			} else {
				PRINT_IQ_INFO(sfile, "    UNKNOW");
			}
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, " IPP  direct :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->ipp_trig_obj.ipp_direct);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "   frame num :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->ipp_trig_obj.frame_num);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "     long_id :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->shdr_long_id);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "   ae_status :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			if (iq_info->final_ipp.sync_info.ae_status == ISP_AE_STATUS_STABLE) {
				PRINT_IQ_INFO(sfile, "    STABLE");
			} else if (iq_info->final_ipp.sync_info.ae_status == ISP_AE_STATUS_FINE) {
				PRINT_IQ_INFO(sfile, "      FINE");
			} else if (iq_info->final_ipp.sync_info.ae_status == ISP_AE_STATUS_COARSE) {
				PRINT_IQ_INFO(sfile, "    COARSE");
			} else {
				PRINT_IQ_INFO(sfile, "    UNKNOW");
			}
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "         iso :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_ipp.sync_info.gain);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "    enh_gain :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_ipp.enh_gain);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "       dgain :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_ipp.sync_info.dgain);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "          lv :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_ipp.sync_info.lv);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "     lv_base :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_ipp.sync_info.lv_base);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "       Rgain :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_ipp.sync_info.cgain[0]);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "       Ggain :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_ipp.sync_info.cgain[1]);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "       Bgain :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_ipp.sync_info.cgain[2]);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "          ct :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_ipp.sync_info.ct);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "     EVratio :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_ipp.sync_info.shdr_ev_ratio[0]);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");
	PRINT_IQ_INFO(sfile, "             :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_ipp.sync_info.shdr_ev_ratio[1]);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "     TMratio :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_ipp.sync_info.shdr_tm_ratio);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "       HBSth :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_ipp.sync_info.shdr_hbs_param.lum_th);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "    HBSstart :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_ipp.sync_info.shdr_hbs_param.w_start);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "    HBSslope :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_ipp.sync_info.shdr_hbs_param.w_slope);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");
	PRINT_IQ_INFO(sfile, "     IRlevel :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->ir_info.ir_level);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "       IRSat :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->ir_info.saturation);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "  ENC_Eratio :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->enc_isp_ratio.enc_edge_ratio);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, " ENC_2Dratio :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->enc_isp_ratio.enc_2dnr_ratio);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, " ENC_3Dratio :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->enc_isp_ratio.enc_3dnr_ratio);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "     DRlevel :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->dr_level);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");
	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");

	PRINT_IQ_INFO(sfile, " ENC     iso :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->final_enc.sync_info.gain);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");
	PRINT_IQ_INFO(sfile, "    SENSOR INFORMATION \r\n");
	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");

	PRINT_IQ_INFO(sfile, " SIE  FORMAT :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			if (iq_info->sie_trig_obj.data_fmt == ISP_SEN_DATA_FMT_RGB) {
				PRINT_IQ_INFO(sfile, "       RGB");
			} else if (iq_info->sie_trig_obj.data_fmt == ISP_SEN_DATA_FMT_RGBIR) {
				PRINT_IQ_INFO(sfile, "     RGBIR");
			} else if (iq_info->sie_trig_obj.data_fmt == ISP_SEN_DATA_FMT_RCCB) {
				PRINT_IQ_INFO(sfile, "      RCCB");
			} else if (iq_info->sie_trig_obj.data_fmt == ISP_SEN_DATA_FMT_YUV) {
				PRINT_IQ_INFO(sfile, "       YUV");
			} else if (iq_info->sie_trig_obj.data_fmt == ISP_SEN_DATA_FMT_Y_ONLY) {
				PRINT_IQ_INFO(sfile, "    Y_ONLY");
			} else if (iq_info->sie_trig_obj.data_fmt == ISP_SEN_DATA_FMT_DVS) {
				PRINT_IQ_INFO(sfile, "       DVS");
			} else {
				PRINT_IQ_INFO(sfile, "    UNKNOW");
			}
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, " IPP  FORMAT :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			if (iq_info->ipp_trig_obj.data_fmt == ISP_SEN_DATA_FMT_RGB) {
				PRINT_IQ_INFO(sfile, "       RGB");
			} else if (iq_info->ipp_trig_obj.data_fmt == ISP_SEN_DATA_FMT_RGBIR) {
				PRINT_IQ_INFO(sfile, "     RGBIR");
			} else if (iq_info->ipp_trig_obj.data_fmt == ISP_SEN_DATA_FMT_RCCB) {
				PRINT_IQ_INFO(sfile, "      RCCB");
			} else if (iq_info->ipp_trig_obj.data_fmt == ISP_SEN_DATA_FMT_YUV) {
				PRINT_IQ_INFO(sfile, "       YUV");
			} else if (iq_info->ipp_trig_obj.data_fmt == ISP_SEN_DATA_FMT_Y_ONLY) {
				PRINT_IQ_INFO(sfile, "    Y_ONLY");
			} else if (iq_info->ipp_trig_obj.data_fmt == ISP_SEN_DATA_FMT_DVS) {
				PRINT_IQ_INFO(sfile, "       DVS");
			} else {
				PRINT_IQ_INFO(sfile, "    UNKNOW");
			}
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");
	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");

	PRINT_IQ_INFO(sfile, " SIE    TYPE :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			if (iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_LINEAR) {
				PRINT_IQ_INFO(sfile, "    LINEAR");
			} else if (iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_BUILTIN_HDR) {
				PRINT_IQ_INFO(sfile, "  BU_HDR_1");
			} else if (iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_CCIR) {
				PRINT_IQ_INFO(sfile, "      CCIR");
			} else if (iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_CCIR_INTERLACE) {
				PRINT_IQ_INFO(sfile, "  CCIR_INT");
			} else if (iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_RAW_PDAF) {
				PRINT_IQ_INFO(sfile, "  R_PDAF_1");
			} else if (iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_BUILTIN_DCG_HDR) {
				PRINT_IQ_INFO(sfile, "  BU_DCG_1");
			} else if (iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_STAGGER_HDR) {
				PRINT_IQ_INFO(sfile, "    SHDR_2");
			} else if (iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_PDAF) {
				PRINT_IQ_INFO(sfile, "    PDAF_2");
			} else if (iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_BUILTIN_DCG_SHDR) {
				PRINT_IQ_INFO(sfile, "  BU_DCG_2");
			} else if (iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_STAGGER_PDAF) {
				PRINT_IQ_INFO(sfile, "  R_PDAF_2");
			} else if (iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_DCG_HDR) {
				PRINT_IQ_INFO(sfile, " DCG_HDR_2");
			} else if (iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_DCG_SHDR) {
				PRINT_IQ_INFO(sfile, " DCG_HDR_3");
			} else if (iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_STAGGER3_HDR) {
				PRINT_IQ_INFO(sfile, "    SHDR_3");
			} else if (iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_THERMAL) {
				PRINT_IQ_INFO(sfile, "   THERMAL");
			} else {
				PRINT_IQ_INFO(sfile, "    UNKNOW");
			}
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, " IPP    TYPE :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			if (iq_info->ipp_trig_obj.mode_type == ISP_SEN_MODE_LINEAR) {
				PRINT_IQ_INFO(sfile, "    LINEAR");
			} else if (iq_info->ipp_trig_obj.mode_type == ISP_SEN_MODE_BUILTIN_HDR) {
				PRINT_IQ_INFO(sfile, "  BU_HDR_1");
			} else if (iq_info->ipp_trig_obj.mode_type == ISP_SEN_MODE_CCIR) {
				PRINT_IQ_INFO(sfile, "      CCIR");
			} else if (iq_info->ipp_trig_obj.mode_type == ISP_SEN_MODE_CCIR_INTERLACE) {
				PRINT_IQ_INFO(sfile, "  CCIR_INT");
			} else if (iq_info->ipp_trig_obj.mode_type == ISP_SEN_MODE_RAW_PDAF) {
				PRINT_IQ_INFO(sfile, "  R_PDAF_1");
			} else if (iq_info->ipp_trig_obj.mode_type == ISP_SEN_MODE_BUILTIN_DCG_HDR) {
				PRINT_IQ_INFO(sfile, "  BU_DCG_1");
			} else if (iq_info->ipp_trig_obj.mode_type == ISP_SEN_MODE_STAGGER_HDR) {
				PRINT_IQ_INFO(sfile, "    SHDR_2");
			} else if (iq_info->ipp_trig_obj.mode_type == ISP_SEN_MODE_PDAF) {
				PRINT_IQ_INFO(sfile, "    PDAF_2");
			} else if (iq_info->ipp_trig_obj.mode_type == ISP_SEN_MODE_BUILTIN_DCG_SHDR) {
				PRINT_IQ_INFO(sfile, "  BU_DCG_2");
			} else if (iq_info->ipp_trig_obj.mode_type == ISP_SEN_MODE_STAGGER_PDAF) {
				PRINT_IQ_INFO(sfile, "  R_PDAF_2");
			} else if (iq_info->ipp_trig_obj.mode_type == ISP_SEN_MODE_DCG_HDR) {
				PRINT_IQ_INFO(sfile, " DCG_HDR_2");
			} else if (iq_info->ipp_trig_obj.mode_type == ISP_SEN_MODE_DCG_SHDR) {
				PRINT_IQ_INFO(sfile, " DCG_HDR_3");
			} else if (iq_info->ipp_trig_obj.mode_type == ISP_SEN_MODE_STAGGER3_HDR) {
				PRINT_IQ_INFO(sfile, "    SHDR_3");
			} else if (iq_info->ipp_trig_obj.mode_type == ISP_SEN_MODE_THERMAL) {
				PRINT_IQ_INFO(sfile, "   THERMAL");
			} else {
				PRINT_IQ_INFO(sfile, "    UNKNOW");
			}
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");
	PRINT_IQ_INFO(sfile, "    FUNCTION ENABLE \r\n");
	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");

	PRINT_IQ_INFO(sfile, " SIE      AE :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->sie_trig_obj.func_en & ISP_FUNC_EN_AE) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "         AWB :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->sie_trig_obj.func_en & ISP_FUNC_EN_AWB) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "        SHDR :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->sie_trig_obj.func_en & ISP_FUNC_EN_SHDR) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");

	PRINT_IQ_INFO(sfile, " IPP      AF :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_AF) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "         WDR :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_WDR) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "        SHDR :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_SHDR) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "       DEFOG :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_DEFOG) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "         GDC :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_GDC) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "        3DNR :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_3DNR) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "     IFE NN0 :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_NN_IFE_0) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "     IFE NN1 :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_NN_IFE_1) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "     IME NN0 :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_NN_IME_0) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "     IME NN1 :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_NN_IME_1) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");

	PRINT_IQ_INFO(sfile, "     ob_mode :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			if (iq_info->ob_mode == IQ_OB_SIE) {
				PRINT_IQ_INFO(sfile, "       SIE");
			} else if (iq_info->ob_mode == IQ_OB_PRE_F) {
				PRINT_IQ_INFO(sfile, "     PRE_F");
			} else if (iq_info->ob_mode == IQ_OB_PRE) {
				PRINT_IQ_INFO(sfile, "       PRE");
			} else if (iq_info->ob_mode == IQ_OB_IFE_F) {
				PRINT_IQ_INFO(sfile, "     IFE_F");
			} else if (iq_info->ob_mode == IQ_OB_IFE) {
				PRINT_IQ_INFO(sfile, "       IFE");
			} else {
				PRINT_IQ_INFO(sfile, "    UNKNOW");
			}
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "  dgain_mode :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			if (iq_info->dg_mode == IQ_DG_OFF) {
				PRINT_IQ_INFO(sfile, "       OFF");
			} else if (iq_info->dg_mode == IQ_DG_SIE) {
				PRINT_IQ_INFO(sfile, "       SIE");
			} else if (iq_info->dg_mode == IQ_DG_PRE) {
				PRINT_IQ_INFO(sfile, "       PRE");
			} else if (iq_info->dg_mode == IQ_DG_IFE_F) {
				PRINT_IQ_INFO(sfile, "     IFE_F");
			} else if (iq_info->dg_mode == IQ_DG_IFE) {
				PRINT_IQ_INFO(sfile, "       IFE");
			} else if (iq_info->dg_mode == IQ_DG_IPE) {
				PRINT_IQ_INFO(sfile, "       IPE");
			} else {
				PRINT_IQ_INFO(sfile, "    UNKNOW");
			}
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "  cgain_mode :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			if (iq_info->cg_mode == IQ_CG_SIE) {
				PRINT_IQ_INFO(sfile, "       SIE");
			} else if (iq_info->cg_mode == IQ_CG_PRE_F) {
				PRINT_IQ_INFO(sfile, "    PREE_F");
			} else if (iq_info->cg_mode == IQ_CG_PRE) {
				PRINT_IQ_INFO(sfile, "       PRE");
			} else if (iq_info->cg_mode == IQ_CG_IFE_F) {
				PRINT_IQ_INFO(sfile, "     IFE_F");
			} else if (iq_info->cg_mode == IQ_CG_IFE) {
				PRINT_IQ_INFO(sfile, "       IFE");
			} else if (iq_info->cg_mode == IQ_CG_IPE) {
				PRINT_IQ_INFO(sfile, "       IPE");
			} else {
				PRINT_IQ_INFO(sfile, "    UNKNOW");
			}
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");

	PRINT_IQ_INFO(sfile, "      dpc_en :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->dpc_en);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "      ecs_en :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", iq_info->ecs_en);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");

	PRINT_IQ_INFO(sfile, " Update  SIE :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9x", iq_info->sie_tab_update);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "         IPP :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9x", iq_info->ipp_tab_update);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");

	PRINT_IQ_INFO(sfile, "  Warning Cnt: %9d \r\n", iq_dbg_get_wrn_msg());

	PRINT_IQ_INFO(sfile, "----------------------------------------------------------------------------------------- \r\n");
}

#if defined(__FREERTOS)
void iq_msg_show_buffer_size(void)
#else
void iq_msg_show_buffer_size(struct seq_file *sfile)
#endif
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;
	UINT32 id;
	int align_byte = 4;

	PRINT_IQ_INFO(sfile, " private buffer : \r\n");
	PRINT_IQ_INFO(sfile, " ================================ \r\n");
	PRINT_IQ_INFO(sfile, " |          IQALG_INFO          | size = 0x%x\r\n", (UINT32)sizeof(IQALG_INFO));
	PRINT_IQ_INFO(sfile, " -------------------------------- \r\n");
	PRINT_IQ_INFO(sfile, " |          IQ_SIE_DPC          | size = 0x%x\r\n", (UINT32)sizeof(IQ_SIE_DPC));
	PRINT_IQ_INFO(sfile, " -------------------------------- \r\n");
	PRINT_IQ_INFO(sfile, " |          IQ_SIE_ECS          | size = 0x%x\r\n", (UINT32)sizeof(IQ_SIE_ECS));
	PRINT_IQ_INFO(sfile, " -------------------------------- \r\n");
	PRINT_IQ_INFO(sfile, " |     IQ_SHADING_EXT_PARAM     | size = 0x%x\r\n", (UINT32)sizeof(IQ_SHADING_EXT_PARAM));
	PRINT_IQ_INFO(sfile, " -------------------------------- \r\n");
	PRINT_IQ_INFO(sfile, " |       CTL_IPE_ISP_3DCC       | size = 0x%x\r\n", (UINT32)sizeof(CTL_IPE_ISP_3DCC));
	PRINT_IQ_INFO(sfile, " -------------------------------- \r\n");
	PRINT_IQ_INFO(sfile, " |       IQ_3DCC_EXT_PARAM      | size = 0x%x\r\n", (UINT32)sizeof(IQ_3DCC_EXT_PARAM));
	PRINT_IQ_INFO(sfile, " ================================ \r\n");
	PRINT_IQ_INFO(sfile, " iq_info_buffer_size = 0x%x\r\n", iq_info_buffer_size);
	PRINT_IQ_INFO(sfile, " iq_info_buffer_dpc_ofs = 0x%x\r\n", iq_info_buffer_dpc_ofs);
	PRINT_IQ_INFO(sfile, " iq_info_buffer_ecs_ofs = 0x%x\r\n", iq_info_buffer_ecs_ofs);
	PRINT_IQ_INFO(sfile, " iq_info_buffer_3dcc_ofs = 0x%x\r\n", iq_info_buffer_3dcc_ofs);

	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");
	PRINT_IQ_INFO(sfile, "          id :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, "   %16d", iq_info->id);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");
	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");
	PRINT_IQ_INFO(sfile, "  IQALG_INFO :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " 0x%16lx", (ULONG)iq_info);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");
	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");
	PRINT_IQ_INFO(sfile, "  IQ_SIE_DPC :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " 0x%16lx", (ULONG)iq_info->final_sie.sie_dpc);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");
	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");
	PRINT_IQ_INFO(sfile, "  IQ_SIE_ECS :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " 0x%16lx", (ULONG)iq_info->final_sie.sie_ecs);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");
	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");
	PRINT_IQ_INFO(sfile, "     ECS_EXT :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " 0x%16lx", (ULONG)iq_info->iq_ref_set.ecs_ext);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");
	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");
	PRINT_IQ_INFO(sfile, "        3DCC :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " 0x%16lx", (ULONG)iq_info->final_ipp.ipe_3dcc);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");
	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");
	PRINT_IQ_INFO(sfile, "    3DCC_EXT :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " 0x%16lx", (ULONG)iq_info->iq_ref_set._3dcc_ext);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");
	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");

	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " ================================== \r\n");
			PRINT_IQ_INFO(sfile, " |          IQ_SIE_DPC %2d         | \r\n", id);
			PRINT_IQ_INFO(sfile, " ================================== \r\n");
			PRINT_IQ_INFO(sfile, " sizeof(IQ_SIE_DPC) = 0x%x \r\n", (UINT32)sizeof(IQ_SIE_DPC));
			PRINT_IQ_INFO(sfile, " iq_info->final_sie.sie_dpc = 0x%lx \r\n", (ULONG)iq_info->final_sie.sie_dpc);
			if ((iq_info->dpc_en == TRUE) && (iq_info->final_sie.sie_dpc != NULL)) {
				PRINT_IQ_INFO(sfile, "    enable = %d \r\n", iq_info->final_sie.sie_dpc->enable);
				PRINT_IQ_INFO(sfile, "    mode = %d \r\n", iq_info->final_sie.sie_dpc->mode);
				PRINT_IQ_INFO(sfile, "    expand_en = %d \r\n", iq_info->final_sie.sie_dpc->expand_en);
				PRINT_IQ_INFO(sfile, "    table = {0x%x, 0x%x, 0x%x, 0x%x, 0x%x, ... , 0x%x} \r\n"
					, iq_info->final_sie.sie_dpc->table[0], iq_info->final_sie.sie_dpc->table[1]
					, iq_info->final_sie.sie_dpc->table[2], iq_info->final_sie.sie_dpc->table[3]
					, iq_info->final_sie.sie_dpc->table[4], iq_info->final_sie.sie_dpc->table[IQ_DPC_MAX_NUM - 1]);
				PRINT_IQ_INFO(sfile, "    table_phyaddr = 0x%lx \r\n",  iq_info->final_sie.sie_dpc->table_phyaddr);
				PRINT_IQ_INFO(sfile, "    expand_table_viraddr = 0x%lx \r\n", iq_info->final_sie.sie_dpc->expand_table_viraddr);
				PRINT_IQ_INFO(sfile, "    expand_table_phyaddr = 0x%lx \r\n", iq_info->final_sie.sie_dpc->expand_table_phyaddr);
				PRINT_IQ_INFO(sfile, "    weight = %d \r\n", iq_info->final_sie.sie_dpc->weight);
				PRINT_IQ_INFO(sfile, "    dp_buffer_size = %d \r\n", iq_info->final_sie.sie_dpc->dp_buffer_size);
				PRINT_IQ_INFO(sfile, "    dp_total_size = %d \r\n", iq_info->final_sie.sie_dpc->dp_total_size);
			}
		}
	}

	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " ================================ \r\n");
			PRINT_IQ_INFO(sfile, " |          IQ_SIE_ECS %2d       | \r\n", id);
			PRINT_IQ_INFO(sfile, " ================================ \r\n");
			PRINT_IQ_INFO(sfile, " sizeof(IQ_SIE_ECS) = 0x%x \r\n", (UINT32)sizeof(IQ_SIE_ECS));
			PRINT_IQ_INFO(sfile, " iq_info->final_sie.sie_ecs = 0x%lx \r\n", (ULONG)iq_info->final_sie.sie_ecs);
			if ((iq_info->ecs_en == TRUE) && (iq_info->final_sie.sie_ecs != NULL)) {
				PRINT_IQ_INFO(sfile, "    enable = %d \r\n", iq_info->final_sie.sie_ecs->enable);
				PRINT_IQ_INFO(sfile, "    sel_37_fmt = %d \r\n", iq_info->final_sie.sie_ecs->sel_37_fmt);
				PRINT_IQ_INFO(sfile, "    map_tbl = {0x%x, 0x%x, 0x%x, 0x%x, 0x%x, ... , 0x%x} \r\n"
					, iq_info->final_sie.sie_ecs->map_tbl[0], iq_info->final_sie.sie_ecs->map_tbl[1]
					, iq_info->final_sie.sie_ecs->map_tbl[2], iq_info->final_sie.sie_ecs->map_tbl[3]
					, iq_info->final_sie.sie_ecs->map_tbl[4], iq_info->final_sie.sie_ecs->map_tbl[IQ_SHADING_ECS_LEN - 1]);
				PRINT_IQ_INFO(sfile, "    map_tbl_phyaddr = 0x%lx \r\n", iq_info->final_sie.sie_ecs->map_tbl_phyaddr);
				PRINT_IQ_INFO(sfile, "    map_sel = %d \r\n", iq_info->final_sie.sie_ecs->map_sel);
				PRINT_IQ_INFO(sfile, "    dthr_enable = %d \r\n", iq_info->final_sie.sie_ecs->dthr_enable);
				PRINT_IQ_INFO(sfile, "    dthr_reset = %d \r\n", iq_info->final_sie.sie_ecs->dthr_reset);
				PRINT_IQ_INFO(sfile, "    dthr_level = %d \r\n", iq_info->final_sie.sie_ecs->dthr_level);
				PRINT_IQ_INFO(sfile, "    bayer_mode = %d \r\n", iq_info->final_sie.sie_ecs->bayer_mode);

				PRINT_IQ_INFO(sfile, "    ecs_mode = %d \r\n", iq_info->iq_ref_set.ecs_mode);
				PRINT_IQ_INFO(sfile, "    ecs_smooth_l_m_ct_lower = %d \r\n", iq_info->iq_ref_set.ecs_smooth_l_m_ct_lower);
				PRINT_IQ_INFO(sfile, "    ecs_smooth_l_m_ct_upper = %d \r\n", iq_info->iq_ref_set.ecs_smooth_l_m_ct_upper);
				PRINT_IQ_INFO(sfile, "    ecs_smooth_m_h_ct_lower = %d \r\n", iq_info->iq_ref_set.ecs_smooth_m_h_ct_lower);
				PRINT_IQ_INFO(sfile, "    ecs_smooth_m_h_ct_upper = %d \r\n", iq_info->iq_ref_set.ecs_smooth_m_h_ct_upper);
			}
		}
	}

	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " ================================ \r\n");
			PRINT_IQ_INFO(sfile, " |          IQ_ECS_EXT %2d       | \r\n", id);
			PRINT_IQ_INFO(sfile, " ================================ \r\n");
			PRINT_IQ_INFO(sfile, " sizeof(IQ_SHADING_EXT_PARAM) = 0x%x \r\n", (UINT32)sizeof(IQ_SHADING_EXT_PARAM));
			PRINT_IQ_INFO(sfile, " iq_info->iq_ref_set.ecs_ext = 0x%lx \r\n", (ULONG)iq_info->iq_ref_set.ecs_ext);
			if (iq_info->ecs_en == TRUE) {
				PRINT_IQ_INFO(sfile, "     manual_ecs_tbl = {0x%x, 0x%x, 0x%x, 0x%x, 0x%x, ... , 0x%x} \r\n"
					, iq_info->iq_ref_set.ecs_ext->manual_ecs_tbl[0], iq_info->iq_ref_set.ecs_ext->manual_ecs_tbl[1]
					, iq_info->iq_ref_set.ecs_ext->manual_ecs_tbl[2], iq_info->iq_ref_set.ecs_ext->manual_ecs_tbl[3]
					, iq_info->iq_ref_set.ecs_ext->manual_ecs_tbl[4], iq_info->iq_ref_set.ecs_ext->manual_ecs_tbl[IQ_SHADING_ECS_LEN - 1]);
				PRINT_IQ_INFO(sfile, "     auto_ecs_tbl[IQ_ECS_TEMPERATURE_H] = {0x%x, 0x%x, 0x%x, 0x%x, 0x%x, ... , 0x%x} \r\n"
					, iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_H][0], iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_H][1]
					, iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_H][2], iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_H][3]
					, iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_H][4], iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_H][IQ_SHADING_ECS_LEN - 1]);
				PRINT_IQ_INFO(sfile, "     auto_ecs_tbl[IQ_ECS_TEMPERATURE_M] = {0x%x, 0x%x, 0x%x, 0x%x, 0x%x, ... , 0x%x} \r\n"
					, iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_M][0], iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_M][1]
					, iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_M][2], iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_M][3]
					, iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_M][4], iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_M][IQ_SHADING_ECS_LEN - 1]);
				PRINT_IQ_INFO(sfile, "     auto_ecs_tbl[IQ_ECS_TEMPERATURE_L] = {0x%x, 0x%x, 0x%x, 0x%x, 0x%x, ... , 0x%x} \r\n"
					, iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_L][0], iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_L][1]
					, iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_L][2], iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_L][3]
					, iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_L][4], iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_L][IQ_SHADING_ECS_LEN - 1]);

				PRINT_IQ_INFO(sfile, "     ecs_mode = %d \r\n", iq_info->iq_ref_set.ecs_mode);
				PRINT_IQ_INFO(sfile, "     ecs_smooth_l_m_ct_lower = %d \r\n", iq_info->iq_ref_set.ecs_smooth_l_m_ct_lower);
				PRINT_IQ_INFO(sfile, "     ecs_smooth_l_m_ct_upper = %d \r\n", iq_info->iq_ref_set.ecs_smooth_l_m_ct_upper);
				PRINT_IQ_INFO(sfile, "     ecs_smooth_m_h_ct_lower = %d \r\n", iq_info->iq_ref_set.ecs_smooth_m_h_ct_lower);
				PRINT_IQ_INFO(sfile, "     ecs_smooth_m_h_ct_upper = %d \r\n", iq_info->iq_ref_set.ecs_smooth_m_h_ct_upper);
				PRINT_IQ_INFO(sfile, "     vig_reduce_th = %d \r\n", iq_info->iq_ref_set.vig_reduce_th);
				PRINT_IQ_INFO(sfile, "     vig_zero_th = %d \r\n", iq_info->iq_ref_set.vig_zero_th);
				PRINT_IQ_INFO(sfile, "     vig_lut = {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d} \r\n"
					, iq_info->iq_ref_set.vig_lut[0], iq_info->iq_ref_set.vig_lut[1]
					, iq_info->iq_ref_set.vig_lut[2], iq_info->iq_ref_set.vig_lut[3]
					, iq_info->iq_ref_set.vig_lut[4], iq_info->iq_ref_set.vig_lut[5]
					, iq_info->iq_ref_set.vig_lut[6], iq_info->iq_ref_set.vig_lut[7]
					, iq_info->iq_ref_set.vig_lut[8], iq_info->iq_ref_set.vig_lut[9]
					, iq_info->iq_ref_set.vig_lut[10], iq_info->iq_ref_set.vig_lut[11]
					, iq_info->iq_ref_set.vig_lut[12], iq_info->iq_ref_set.vig_lut[13]
					, iq_info->iq_ref_set.vig_lut[14], iq_info->iq_ref_set.vig_lut[15]
					, iq_info->iq_ref_set.vig_lut[16]);
			}
		}
	}

	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " ================================ \r\n");
			PRINT_IQ_INFO(sfile, " |      CTL_IPE_ISP_3DCC %2d     | \r\n", id);
			PRINT_IQ_INFO(sfile, " ================================ \r\n");
			PRINT_IQ_INFO(sfile, " sizeof(CTL_IPE_ISP_3DCC) = 0x%x \r\n", (UINT32)sizeof(CTL_IPE_ISP_3DCC));
			PRINT_IQ_INFO(sfile, " iq_info->final_ipp.ipe_3dcc = 0x%lx \r\n", (ULONG)iq_info->final_ipp.ipe_3dcc);
			if ((iq_info->_3dcc_en == TRUE) && (iq_info->final_ipp.ipe_3dcc != NULL)) {
				PRINT_IQ_INFO(sfile, "    enable = %d \r\n", iq_info->final_ipp.ipe_3dcc->enable);
				PRINT_IQ_INFO(sfile, "    rgb_3d_lut = {0x%x, 0x%x, 0x%x, 0x%x, 0x%x, ... , 0x%x} \r\n"
					, iq_info->final_ipp.ipe_3dcc->lut.rgb_3d_lut[0], iq_info->final_ipp.ipe_3dcc->lut.rgb_3d_lut[1]
					, iq_info->final_ipp.ipe_3dcc->lut.rgb_3d_lut[2], iq_info->final_ipp.ipe_3dcc->lut.rgb_3d_lut[3]
					, iq_info->final_ipp.ipe_3dcc->lut.rgb_3d_lut[4], iq_info->final_ipp.ipe_3dcc->lut.rgb_3d_lut[IQ_3DCC_LEN - 1]);
			}
		}
	}

	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " ================================ \r\n");
			PRINT_IQ_INFO(sfile, " |         IQ_3DCC_EXT %2d       | \r\n", id);
			PRINT_IQ_INFO(sfile, " ================================ \r\n");
			PRINT_IQ_INFO(sfile, " sizeof(IQ_3DCC_EXT_PARAM) = 0x%x \r\n", (UINT32)sizeof(IQ_3DCC_EXT_PARAM));
			PRINT_IQ_INFO(sfile, " iq_info->iq_ref_set._3dcc_ext = 0x%lx \r\n", (ULONG)iq_info->iq_ref_set._3dcc_ext);
			if (iq_info->_3dcc_en == TRUE) {
				PRINT_IQ_INFO(sfile, "     manual_3dcc_lut = {0x%x, 0x%x, 0x%x, 0x%x, 0x%x, ... , 0x%x} \r\n"
					, iq_info->iq_ref_set._3dcc_ext->manual_3dcc_lut[0], iq_info->iq_ref_set._3dcc_ext->manual_3dcc_lut[1]
					, iq_info->iq_ref_set._3dcc_ext->manual_3dcc_lut[2], iq_info->iq_ref_set._3dcc_ext->manual_3dcc_lut[3]
					, iq_info->iq_ref_set._3dcc_ext->manual_3dcc_lut[4], iq_info->iq_ref_set._3dcc_ext->manual_3dcc_lut[IQ_3DCC_LEN - 1]);
				PRINT_IQ_INFO(sfile, "     auto_3dcc_lut[IQ_COLOR_TEMPERATURE_0] = {0x%x, 0x%x, 0x%x, 0x%x, 0x%x, ... , 0x%x} \r\n"
					, iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_0][0], iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_0][1]
					, iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_0][2], iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_0][3]
					, iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_0][4], iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_0][IQ_3DCC_LEN - 1]);
				PRINT_IQ_INFO(sfile, "     auto_3dcc_lut[IQ_COLOR_TEMPERATURE_1] = {0x%x, 0x%x, 0x%x, 0x%x, 0x%x, ... , 0x%x} \r\n"
					, iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_1][0], iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_1][1]
					, iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_1][2], iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_1][3]
					, iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_1][4], iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_1][IQ_3DCC_LEN - 1]);
				PRINT_IQ_INFO(sfile, "     auto_3dcc_lut[IQ_COLOR_TEMPERATURE_2] = {0x%x, 0x%x, 0x%x, 0x%x, 0x%x, ... , 0x%x} \r\n"
					, iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_2][0], iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_2][1]
					, iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_2][2], iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_2][3]
					, iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_2][4], iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_2][IQ_3DCC_LEN - 1]);
				PRINT_IQ_INFO(sfile, "     auto_3dcc_lut[IQ_COLOR_TEMPERATURE_3] = {0x%x, 0x%x, 0x%x, 0x%x, 0x%x, ... , 0x%x} \r\n"
					, iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_3][0], iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_3][1]
					, iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_3][2], iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_3][3]
					, iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_3][4], iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_3][IQ_3DCC_LEN - 1]);
				PRINT_IQ_INFO(sfile, "     auto_3dcc_lut[IQ_COLOR_TEMPERATURE_4] = {0x%x, 0x%x, 0x%x, 0x%x, 0x%x, ... , 0x%x} \r\n"
					, iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_4][0], iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_4][1]
					, iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_4][2], iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_4][3]
					, iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_4][4], iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[IQ_COLOR_TEMPERATURE_4][IQ_3DCC_LEN - 1]);
			}
		}
	}

	PRINT_IQ_INFO(sfile, " ================================== \r\n");
	PRINT_IQ_INFO(sfile, " |            IQ_PARAM            | \r\n");
	PRINT_IQ_INFO(sfile, " ================================== \r\n");
	PRINT_IQ_INFO(sfile, " total iq_param : 0x%9x \r\n"
		, (UINT32)(ALIGN_CEIL(sizeof(IQ_OB_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_NR_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_CFA_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_VA_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_GAMMA_PARAM), align_byte) +
		ALIGN_CEIL(sizeof(IQ_CCM_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_COLOR_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_CONTRAST_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_EDGE_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_3DNR_PARAM), align_byte) +
		ALIGN_CEIL(sizeof(IQ_PFR_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_WDR_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_DEFOG_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_SHDR_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_COMPANDING_PARAM), align_byte) +
		ALIGN_CEIL(sizeof(IQ_RGBIR_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_RGBIR_ENH_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_POST_SHARPEN_2_PARAM), align_byte) +
		ALIGN_CEIL(sizeof(IQ_YCURVE_PARAM), align_byte)+ ALIGN_CEIL(sizeof(IQ_CST_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_POST_SHARPEN_1_PARAM), align_byte) +
		ALIGN_CEIL(sizeof(IQ_TONE_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_WDR_ENH_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_RAW_VA_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_FPN_PARAM), align_byte)));

	PRINT_IQ_INFO(sfile, " ---------------------------- \r\n");
	PRINT_IQ_INFO(sfile, "             ob : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_OB_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "             nr : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_NR_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "            cfa : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_CFA_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "             va : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_VA_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "          gamma : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_GAMMA_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "            ccm : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_CCM_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "          color : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_COLOR_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "       contrast : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_CONTRAST_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "           edge : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_EDGE_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "           3dnr : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_3DNR_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "            pfr : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_PFR_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "            wdr : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_WDR_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "          defog : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_DEFOG_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "           shdr : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_SHDR_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "     companding : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_COMPANDING_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "          rgbir : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_RGBIR_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "      rgbir_enh : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_RGBIR_ENH_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, " post_sharpen_2 : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_POST_SHARPEN_2_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "         ycurve : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_YCURVE_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "            cst : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_CST_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, " post_sharpen_1 : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_POST_SHARPEN_1_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "           tone : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_TONE_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "        wdr_enh : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_WDR_ENH_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "         raw_va : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_RAW_VA_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, "            fpn : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_FPN_PARAM), align_byte));
	PRINT_IQ_INFO(sfile, " ---------------------------- \r\n");

	PRINT_IQ_INFO(sfile, " ================================== \r\n");
	PRINT_IQ_INFO(sfile, " |           CTL_PARAM            | \r\n");
	PRINT_IQ_INFO(sfile, " ================================== \r\n");
	PRINT_IQ_INFO(sfile, "total ctl_param : 0x%9x \r\n"
		, (UINT32)(sizeof(IQ_FINAL_SIE) + sizeof(IQ_FINAL_IPP) + sizeof(IQ_FINAL_ENC) + sizeof(IQ_FINAL_SETTING)));
	PRINT_IQ_INFO(sfile, " ---------------------------- \r\n");
	PRINT_IQ_INFO(sfile, "      final_sie : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_FINAL_SIE), align_byte));
	PRINT_IQ_INFO(sfile, "      final_ipp : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_FINAL_IPP), align_byte));
	PRINT_IQ_INFO(sfile, "      final_enc : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_FINAL_ENC), align_byte));
	PRINT_IQ_INFO(sfile, "  final_setting : 0x%9x \r\n", (UINT32)ALIGN_CEIL(sizeof(IQ_FINAL_SETTING), align_byte));
	PRINT_IQ_INFO(sfile, " ---------------------------- \r\n");

}


#if defined(__FREERTOS)
void iq_msg_show_param(IQ_ID iq_proc_id, UINT32 iq_proc_iso)
#else
void iq_msg_show_param(struct seq_file *sfile, IQ_ID iq_proc_id, UINT32 iq_proc_iso)
#endif
{
	ISP_MODULE                       *iq_module = iq_get_module();
	void                             *iq_private = iq_module->private;
	IQALG_INFO                       *iq_info;

	IQ_OB_TUNE_PARAM                 *ob_m, *ob_a;
	IQ_NR_TUNE_PARAM                 *nr_m, *nr_a;
	IQ_CFA_TUNE_PARAM                *cfa_m, *cfa_a;
	IQ_RAW_VA_MANUAL_PARAM           *raw_va_m;
	IQ_RAW_VA_AUTO_PARAM             *raw_va_a;
	IQ_VA_TUNE_PARAM                 *va_m, *va_a;
	IQ_TONE_AUTO_PARAM               *tone_a;
	IQ_GAMMA_AUTO_PARAM              *gamma_a;
	IQ_CCM_MANUAL_PARAM              *ccm_m;
	IQ_CCM_AUTO_PARAM                *ccm_a;
	IQ_COLOR_TUNE_PARAM              *color_m, *color_a;
	IQ_CONTRAST_TUNE_PARAM           *contrast_m, *contrast_a;
	IQ_EDGE_TUNE_PARAM               *edge_m, *edge_a;
	IQ_3DNR_TUNE_PARAM               *_3dnr_m, *_3dnr_a;
	IQ_PFR_TUNE_PARAM                *pfr_m, *pfr_a;
	IQ_WDR_MANUAL_PARAM              *wdr_m;
	IQ_WDR_AUTO_PARAM                *wdr_a;
	IQ_DEFOG_MANUAL_PARAM            *defog_m;
	IQ_DEFOG_AUTO_PARAM              *defog_a;
	IQ_SHDR_TUNE_PARAM               *shdr_m, *shdr_a;
	IQ_RGBIR_MANUAL_PARAM            *rgbir_m;
	IQ_RGBIR_AUTO_PARAM              *rgbir_a;
	IQ_RGBIR_ENH_MANUAL_PARAM        *rgbir_enh_m;
	IQ_RGBIR_ENH_AUTO_PARAM          *rgbir_enh_a;
	IQ_CST_TUNE_PARAM                *cst_m, *cst_a;
	IQ_POST_SHARPEN_1_TUNE_PARAM     *post_sharpen_1_m, *post_sharpen_1_a;
	IQ_POST_SHARPEN_2_TUNE_PARAM     *post_sharpen_2_m, *post_sharpen_2_a;

	PRINT_IQ_INFO(sfile, " get iq(%d) param_id(%d) param\n", iq_proc_id, iq_proc_iso);
	PRINT_IQ_INFO(sfile, " \r\n");

	// iq_ob_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_ob_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "enable = %d, \r\n", iq_param[iq_proc_id]->ob->enable);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->ob->mode);
	ob_m = &iq_param[iq_proc_id]->ob->manual_param;
	PRINT_IQ_INFO(sfile, "manual_param.cofs = %d, %d, %d, %d, %d, \r\n"
		, ob_m->cofs[0], ob_m->cofs[1], ob_m->cofs[2], ob_m->cofs[3], ob_m->cofs[4]);
	ob_a = &iq_param[iq_proc_id]->ob->auto_param[iq_proc_iso];
	PRINT_IQ_INFO(sfile, "auto_param[%d].cofs = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, ob_a->cofs[0], ob_a->cofs[1], ob_a->cofs[2], ob_a->cofs[3], ob_a->cofs[4]);

	// iq_nr_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_nr_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "outl_enable = %d, \r\n", iq_param[iq_proc_id]->nr->outl_enable);
	PRINT_IQ_INFO(sfile, "gbal_enable = %d, \r\n", iq_param[iq_proc_id]->nr->gbal_enable);
	PRINT_IQ_INFO(sfile, "filter_enable = %d, \r\n", iq_param[iq_proc_id]->nr->filter_enable);
	PRINT_IQ_INFO(sfile, "lca_enable = %d, \r\n", iq_param[iq_proc_id]->nr->lca_enable);
	PRINT_IQ_INFO(sfile, "dbcs_enable = %d, \r\n", iq_param[iq_proc_id]->nr->dbcs_enable);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->nr->mode);
	nr_m = &iq_param[iq_proc_id]->nr->manual_param;
	PRINT_IQ_INFO(sfile, "manual_param.outl_ord_protect_th = %d, \r\n", nr_m->outl_ord_protect_th);
	PRINT_IQ_INFO(sfile, "manual_param.outl_ord_blend_w = %d, \r\n", nr_m->outl_ord_blend_w);
	PRINT_IQ_INFO(sfile, "manual_param.outl_avg_mode = %d, \r\n", nr_m->outl_avg_mode);
	PRINT_IQ_INFO(sfile, "manual_param.outl_sel = %d, \r\n", nr_m->outl_sel);
	PRINT_IQ_INFO(sfile, "manual_param.outl_bright_th = %d, %d, %d, %d, %d, \r\n"
		, nr_m->outl_bright_th[0], nr_m->outl_bright_th[1], nr_m->outl_bright_th[2], nr_m->outl_bright_th[3], nr_m->outl_bright_th[4]);
	PRINT_IQ_INFO(sfile, "manual_param.outl_bright_th = %d, %d, %d, %d, %d, \r\n"
		, nr_m->outl_dark_th[0], nr_m->outl_dark_th[1], nr_m->outl_dark_th[2], nr_m->outl_dark_th[3], nr_m->outl_dark_th[4]);
	PRINT_IQ_INFO(sfile, "manual_param.gbal_diff_th_str = %d, \r\n", nr_m->gbal_diff_th_str);
	PRINT_IQ_INFO(sfile, "manual_param.gbal_edge_protect_th = %d, \r\n", nr_m->gbal_edge_protect_th);
	PRINT_IQ_INFO(sfile, "manual_param.gbal_str_luma_low_bnd = %d, \r\n", nr_m->gbal_str_luma_low_bnd);
	PRINT_IQ_INFO(sfile, "manual_param.gbal_edge_luma_low_bnd = %d, \r\n", nr_m->gbal_edge_luma_low_bnd);
	PRINT_IQ_INFO(sfile, "manual_param.filter_th = %d, %d, %d, %d, %d, \r\n"
		, nr_m->filter_th[0], nr_m->filter_th[1], nr_m->filter_th[2], nr_m->filter_th[3], nr_m->filter_th[4]);
	PRINT_IQ_INFO(sfile, "manual_param.filter_lut = %d, %d, %d, %d, %d, \r\n"
		, nr_m->filter_lut[0], nr_m->filter_lut[1], nr_m->filter_lut[2], nr_m->filter_lut[3], nr_m->filter_lut[4]);
	PRINT_IQ_INFO(sfile, "manual_param.filter_th_b = %d, %d, %d, %d, %d, \r\n"
		, nr_m->filter_th_b[0], nr_m->filter_th_b[1], nr_m->filter_th_b[2], nr_m->filter_th_b[3], nr_m->filter_th_b[4]);
	PRINT_IQ_INFO(sfile, "manual_param.filter_lut_b = %d, %d, %d, %d, %d, \r\n"
		, nr_m->filter_lut_b[0], nr_m->filter_lut_b[1], nr_m->filter_lut_b[2], nr_m->filter_lut_b[3], nr_m->filter_lut_b[4]);
	PRINT_IQ_INFO(sfile, "manual_param.filter_blend_w = %d, \r\n", nr_m->filter_blend_w);
	PRINT_IQ_INFO(sfile, "manual_param.filter_clamp_th = %d, \r\n", nr_m->filter_clamp_th);
	PRINT_IQ_INFO(sfile, "manual_param.filter_clamp_mul = %d, \r\n", nr_m->filter_clamp_mul);
	PRINT_IQ_INFO(sfile, "manual_param.lca_edge_th = %d, \r\n", nr_m->lca_edge_th);
	PRINT_IQ_INFO(sfile, "manual_param.lca_y_filter_level = %d, %d, %d, \r\n"
		, nr_m->lca_y_filter_level[0], nr_m->lca_y_filter_level[1], nr_m->lca_y_filter_level[2]);
	PRINT_IQ_INFO(sfile, "manual_param.lca_y_coring_gain = %d, %d, %d, \r\n"
		, nr_m->lca_y_coring_gain[0], nr_m->lca_y_coring_gain[1], nr_m->lca_y_coring_gain[2]);
	PRINT_IQ_INFO(sfile, "manual_param.lca_y_coring_cutoff = %d, %d, %d, \r\n"
		, nr_m->lca_y_coring_cutoff[0], nr_m->lca_y_coring_cutoff[1], nr_m->lca_y_coring_cutoff[2]);
	PRINT_IQ_INFO(sfile, "manual_param.lca_y_filter_level = %d, %d, %d, \r\n"
		, nr_m->lca_c_filter_level[0], nr_m->lca_c_filter_level[1], nr_m->lca_c_filter_level[2]);
	PRINT_IQ_INFO(sfile, "manual_param.lca_y_coring_gain = %d, %d, %d, \r\n"
		, nr_m->lca_c_coring_gain[0], nr_m->lca_c_coring_gain[1], nr_m->lca_c_coring_gain[2]);
	PRINT_IQ_INFO(sfile, "manual_param.lca_y_coring_cutoff = %d, %d, %d, \r\n"
		, nr_m->lca_c_coring_cutoff[0], nr_m->lca_c_coring_cutoff[1], nr_m->lca_c_coring_cutoff[2]);
	PRINT_IQ_INFO(sfile, "manual_param.dbcs_step_y = %d, \r\n", nr_m->dbcs_step_y);
	PRINT_IQ_INFO(sfile, "manual_param.dbcs_step_c = %d, \r\n", nr_m->dbcs_step_c);
	nr_a = &iq_param[iq_proc_id]->nr->auto_param[iq_proc_iso];
	PRINT_IQ_INFO(sfile, "auto_param[%d].outl_ord_protect_th = %d, \r\n", iq_proc_iso, nr_a->outl_ord_protect_th);
	PRINT_IQ_INFO(sfile, "auto_param[%d].outl_ord_blend_w = %d, \r\n", iq_proc_iso, nr_a->outl_ord_blend_w);
	PRINT_IQ_INFO(sfile, "auto_param[%d].outl_avg_mode = %d, \r\n", iq_proc_iso, nr_a->outl_avg_mode);
	PRINT_IQ_INFO(sfile, "auto_param[%d].outl_sel = %d, \r\n", iq_proc_iso, nr_a->outl_sel);
	PRINT_IQ_INFO(sfile, "auto_param[%d].outl_bright_th = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, nr_a->outl_bright_th[0], nr_a->outl_bright_th[1], nr_a->outl_bright_th[2], nr_a->outl_bright_th[3], nr_a->outl_bright_th[4]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].outl_bright_th = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, nr_a->outl_dark_th[0], nr_a->outl_dark_th[1], nr_a->outl_dark_th[2], nr_a->outl_dark_th[3], nr_a->outl_dark_th[4]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].gbal_diff_th_str = %d, \r\n", iq_proc_iso, nr_a->gbal_diff_th_str);
	PRINT_IQ_INFO(sfile, "auto_param[%d].gbal_edge_protect_th = %d, \r\n", iq_proc_iso, nr_a->gbal_edge_protect_th);
	PRINT_IQ_INFO(sfile, "auto_param[%d].gbal_str_luma_low_bnd = %d, \r\n", iq_proc_iso, nr_a->gbal_str_luma_low_bnd);
	PRINT_IQ_INFO(sfile, "auto_param[%d].gbal_edge_luma_low_bnd = %d, \r\n", iq_proc_iso, nr_a->gbal_edge_luma_low_bnd);
	PRINT_IQ_INFO(sfile, "auto_param[%d].filter_th = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, nr_a->filter_th[0], nr_a->filter_th[1], nr_a->filter_th[2], nr_a->filter_th[3], nr_a->filter_th[4]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].filter_lut = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, nr_a->filter_lut[0], nr_a->filter_lut[1], nr_a->filter_lut[2], nr_a->filter_lut[3], nr_a->filter_lut[4]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].filter_th_b = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, nr_a->filter_th_b[0], nr_a->filter_th_b[1], nr_a->filter_th_b[2], nr_a->filter_th_b[3], nr_a->filter_th_b[4]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].filter_lut_b = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, nr_a->filter_lut_b[0], nr_a->filter_lut_b[1], nr_a->filter_lut_b[2], nr_a->filter_lut_b[3], nr_a->filter_lut_b[4]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].filter_blend_w = %d, \r\n", iq_proc_iso, nr_a->filter_blend_w);
	PRINT_IQ_INFO(sfile, "auto_param[%d].filter_clamp_th = %d, \r\n", iq_proc_iso, nr_a->filter_clamp_th);
	PRINT_IQ_INFO(sfile, "auto_param[%d].filter_clamp_mul = %d, \r\n", iq_proc_iso, nr_a->filter_clamp_mul);
	PRINT_IQ_INFO(sfile, "auto_param[%d].lca_edge_th = %d, \r\n", iq_proc_iso, nr_a->lca_edge_th);
	PRINT_IQ_INFO(sfile, "auto_param[%d].lca_y_filter_level = %d, %d, %d, \r\n", iq_proc_iso
		, nr_a->lca_y_filter_level[0], nr_a->lca_y_filter_level[1], nr_a->lca_y_filter_level[2]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].lca_y_coring_gain = %d, %d, %d, \r\n", iq_proc_iso
		, nr_a->lca_y_coring_gain[0], nr_a->lca_y_coring_gain[1], nr_a->lca_y_coring_gain[2]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].lca_y_coring_cutoff = %d, %d, %d, \r\n", iq_proc_iso
		, nr_a->lca_y_coring_cutoff[0], nr_a->lca_y_coring_cutoff[1], nr_a->lca_y_coring_cutoff[2]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].lca_y_filter_level = %d, %d, %d, \r\n", iq_proc_iso
		, nr_a->lca_c_filter_level[0], nr_a->lca_c_filter_level[1], nr_a->lca_c_filter_level[2]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].lca_y_coring_gain = %d, %d, %d, \r\n", iq_proc_iso
		, nr_a->lca_c_coring_gain[0], nr_a->lca_c_coring_gain[1], nr_a->lca_c_coring_gain[2]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].lca_y_coring_cutoff = %d, %d, %d, \r\n", iq_proc_iso
		, nr_a->lca_c_coring_cutoff[0], nr_a->lca_c_coring_cutoff[1], nr_a->lca_c_coring_cutoff[2]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].dbcs_step_y = %d, \r\n", iq_proc_iso, nr_a->dbcs_step_y);
	PRINT_IQ_INFO(sfile, "auto_param[%d].dbcs_step_c = %d, \r\n", iq_proc_iso, nr_a->dbcs_step_c);

	// iq_cfa_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_cfa_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->cfa->mode);
	cfa_m = &iq_param[iq_proc_id]->cfa->manual_param;
	PRINT_IQ_INFO(sfile, "manual_param.edge_dth = %d, \r\n", cfa_m->edge_dth);
	PRINT_IQ_INFO(sfile, "manual_param.edge_dth2 = %d, \r\n", cfa_m->edge_dth2);
	PRINT_IQ_INFO(sfile, "manual_param.freq_th = %d, \r\n", cfa_m->freq_th);
	PRINT_IQ_INFO(sfile, "manual_param.fcs_weight = %d, \r\n", cfa_m->fcs_weight);
	cfa_a = &iq_param[iq_proc_id]->cfa->auto_param[iq_proc_iso];
	PRINT_IQ_INFO(sfile, "auto_param[%d].edge_dth = %d, \r\n", iq_proc_iso, cfa_a->edge_dth);
	PRINT_IQ_INFO(sfile, "auto_param[%d].edge_dth2 = %d, \r\n", iq_proc_iso, cfa_a->edge_dth2);
	PRINT_IQ_INFO(sfile, "auto_param[%d].freq_th = %d, \r\n", iq_proc_iso, cfa_a->freq_th);
	PRINT_IQ_INFO(sfile, "auto_param[%d].fcs_weight = %d, \r\n", iq_proc_iso, cfa_a->fcs_weight);

	// iq_raw_va_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_raw_va_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "pre_filter_enable = %d, \r\n", iq_param[iq_proc_id]->raw_va->pre_filter_enable);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->raw_va->mode);
	PRINT_IQ_INFO(sfile, "g1_h_filter_sel = %d, \r\n", iq_param[iq_proc_id]->raw_va->g1_h_filter_sel);
	PRINT_IQ_INFO(sfile, "g1_fir_sym_sel = %d, \r\n", iq_param[iq_proc_id]->raw_va->g1_fir_sym_sel);
	PRINT_IQ_INFO(sfile, "g1_fir_tap_a = %d, \r\n", iq_param[iq_proc_id]->raw_va->g1_fir_tap_a);
	PRINT_IQ_INFO(sfile, "g1_fir_tap_b = %d, \r\n", iq_param[iq_proc_id]->raw_va->g1_fir_tap_b);
	PRINT_IQ_INFO(sfile, "g1_fir_tap_c = %d, \r\n", iq_param[iq_proc_id]->raw_va->g1_fir_tap_c);
	PRINT_IQ_INFO(sfile, "g1_fir_tap_d = %d, \r\n", iq_param[iq_proc_id]->raw_va->g1_fir_tap_d);
	PRINT_IQ_INFO(sfile, "g1_fir_div = %d, \r\n", iq_param[iq_proc_id]->raw_va->g1_fir_div);
	PRINT_IQ_INFO(sfile, "g2_h_filter_sel = %d, \r\n", iq_param[iq_proc_id]->raw_va->g2_h_filter_sel);
	PRINT_IQ_INFO(sfile, "g2_fir_sym_sel = %d, \r\n", iq_param[iq_proc_id]->raw_va->g2_fir_sym_sel);
	PRINT_IQ_INFO(sfile, "g2_fir_tap_a = %d, \r\n", iq_param[iq_proc_id]->raw_va->g2_fir_tap_a);
	PRINT_IQ_INFO(sfile, "g2_fir_tap_b = %d, \r\n", iq_param[iq_proc_id]->raw_va->g2_fir_tap_b);
	PRINT_IQ_INFO(sfile, "g2_fir_tap_c = %d, \r\n", iq_param[iq_proc_id]->raw_va->g2_fir_tap_c);
	PRINT_IQ_INFO(sfile, "g2_fir_tap_d = %d, \r\n", iq_param[iq_proc_id]->raw_va->g2_fir_tap_d);
	PRINT_IQ_INFO(sfile, "g2_fir_div = %d, \r\n", iq_param[iq_proc_id]->raw_va->g2_fir_div);
	PRINT_IQ_INFO(sfile, "ldg_low_th = %d, \r\n", iq_param[iq_proc_id]->raw_va->ldg_low_th);
	PRINT_IQ_INFO(sfile, "ldg_high_th = %d, \r\n", iq_param[iq_proc_id]->raw_va->ldg_high_th);
	PRINT_IQ_INFO(sfile, "ldg_low_gain = %d, \r\n", iq_param[iq_proc_id]->raw_va->ldg_low_gain);
	PRINT_IQ_INFO(sfile, "ldg_high_gain = %d, \r\n", iq_param[iq_proc_id]->raw_va->ldg_high_gain);
	PRINT_IQ_INFO(sfile, "ldg_low_slope = %d, \r\n", iq_param[iq_proc_id]->raw_va->ldg_low_slope);
	PRINT_IQ_INFO(sfile, "ldg_high_slope = %d, \r\n", iq_param[iq_proc_id]->raw_va->ldg_high_slope);
	PRINT_IQ_INFO(sfile, "energy_w = %d, \r\n", iq_param[iq_proc_id]->raw_va->energy_w);
	PRINT_IQ_INFO(sfile, "win_cnt_out_sel = %d, \r\n", iq_param[iq_proc_id]->raw_va->win_cnt_out_sel);
	PRINT_IQ_INFO(sfile, "high_luma_th = %d, \r\n", iq_param[iq_proc_id]->raw_va->high_luma_th);
	raw_va_m = &iq_param[iq_proc_id]->raw_va->manual_param;
	PRINT_IQ_INFO(sfile, "manual_param.g1_iir1_tap_a = %d, \r\n", raw_va_m->g1_iir1_tap_a);
	PRINT_IQ_INFO(sfile, "manual_param.g1_iir1_tap_b = %d, \r\n", raw_va_m->g1_iir1_tap_b);
	PRINT_IQ_INFO(sfile, "manual_param.g1_iir1_tap_e = %d, \r\n", raw_va_m->g1_iir1_tap_e);
	PRINT_IQ_INFO(sfile, "manual_param.g1_iir1_tap_f = %d, \r\n", raw_va_m->g1_iir1_tap_f);
	PRINT_IQ_INFO(sfile, "manual_param.g1_iir1_shift_bit = %d, \r\n", raw_va_m->g1_iir1_shift_bit);
	PRINT_IQ_INFO(sfile, "manual_param.g1_iir2_tap_a = %d, \r\n", raw_va_m->g1_iir2_tap_a);
	PRINT_IQ_INFO(sfile, "manual_param.g1_iir2_tap_b = %d, \r\n", raw_va_m->g1_iir2_tap_b);
	PRINT_IQ_INFO(sfile, "manual_param.g1_iir2_tap_e = %d, \r\n", raw_va_m->g1_iir2_tap_e);
	PRINT_IQ_INFO(sfile, "manual_param.g1_iir2_tap_f = %d, \r\n", raw_va_m->g1_iir2_tap_f);
	PRINT_IQ_INFO(sfile, "manual_param.g1_iir2_shift_bit = %d, \r\n", raw_va_m->g1_iir2_shift_bit);
	PRINT_IQ_INFO(sfile, "manual_param.g1_iir3_tap_a = %d, \r\n", raw_va_m->g1_iir3_tap_a);
	PRINT_IQ_INFO(sfile, "manual_param.g1_iir3_tap_b = %d, \r\n", raw_va_m->g1_iir3_tap_b);
	PRINT_IQ_INFO(sfile, "manual_param.g1_iir3_tap_e = %d, \r\n", raw_va_m->g1_iir3_tap_e);
	PRINT_IQ_INFO(sfile, "manual_param.g1_iir3_tap_f = %d, \r\n", raw_va_m->g1_iir3_tap_f);
	PRINT_IQ_INFO(sfile, "manual_param.g1_iir3_shift_bit = %d, \r\n", raw_va_m->g1_iir3_shift_bit);
	PRINT_IQ_INFO(sfile, "manual_param.g1_th_l = %d, \r\n", raw_va_m->g1_th_l);
	PRINT_IQ_INFO(sfile, "manual_param.g1_th_u = %d, \r\n", raw_va_m->g1_th_u);
	PRINT_IQ_INFO(sfile, "manual_param.g2_iir1_tap_a = %d, \r\n", raw_va_m->g2_iir1_tap_a);
	PRINT_IQ_INFO(sfile, "manual_param.g2_iir1_tap_b = %d, \r\n", raw_va_m->g2_iir1_tap_b);
	PRINT_IQ_INFO(sfile, "manual_param.g2_iir1_tap_e = %d, \r\n", raw_va_m->g2_iir1_tap_e);
	PRINT_IQ_INFO(sfile, "manual_param.g2_iir1_tap_f = %d, \r\n", raw_va_m->g2_iir1_tap_f);
	PRINT_IQ_INFO(sfile, "manual_param.g2_iir1_shift_bit = %d, \r\n", raw_va_m->g2_iir1_shift_bit);
	PRINT_IQ_INFO(sfile, "manual_param.g2_iir2_tap_a = %d, \r\n", raw_va_m->g2_iir2_tap_a);
	PRINT_IQ_INFO(sfile, "manual_param.g2_iir2_tap_b = %d, \r\n", raw_va_m->g2_iir2_tap_b);
	PRINT_IQ_INFO(sfile, "manual_param.g2_iir2_tap_e = %d, \r\n", raw_va_m->g2_iir2_tap_e);
	PRINT_IQ_INFO(sfile, "manual_param.g2_iir2_tap_f = %d, \r\n", raw_va_m->g2_iir2_tap_f);
	PRINT_IQ_INFO(sfile, "manual_param.g2_iir2_shift_bit = %d, \r\n", raw_va_m->g2_iir2_shift_bit);
	PRINT_IQ_INFO(sfile, "manual_param.g2_iir3_tap_a = %d, \r\n", raw_va_m->g2_iir3_tap_a);
	PRINT_IQ_INFO(sfile, "manual_param.g2_iir3_tap_b = %d, \r\n", raw_va_m->g2_iir3_tap_b);
	PRINT_IQ_INFO(sfile, "manual_param.g2_iir3_tap_e = %d, \r\n", raw_va_m->g2_iir3_tap_e);
	PRINT_IQ_INFO(sfile, "manual_param.g2_iir3_tap_f = %d, \r\n", raw_va_m->g2_iir3_tap_f);
	PRINT_IQ_INFO(sfile, "manual_param.g2_iir3_shift_bit = %d, \r\n", raw_va_m->g2_iir3_shift_bit);
	PRINT_IQ_INFO(sfile, "manual_param.g2_th_l = %d, \r\n", raw_va_m->g2_th_l);
	PRINT_IQ_INFO(sfile, "manual_param.g2_th_u = %d, \r\n", raw_va_m->g2_th_u);
	raw_va_a = &iq_param[iq_proc_id]->raw_va->auto_param[iq_proc_iso];
	PRINT_IQ_INFO(sfile, "auto_param[%d].g1_iir = %d, \r\n", iq_proc_iso, raw_va_a->g1_iir);
	PRINT_IQ_INFO(sfile, "auto_param[%d].g1_th_l = %d, \r\n", iq_proc_iso, raw_va_a->g1_th_l);
	PRINT_IQ_INFO(sfile, "auto_param[%d].g1_th_u = %d, \r\n", iq_proc_iso, raw_va_a->g1_th_u);
	PRINT_IQ_INFO(sfile, "auto_param[%d].g2_iir = %d, \r\n", iq_proc_iso, raw_va_a->g2_iir);
	PRINT_IQ_INFO(sfile, "auto_param[%d].g2_th_l = %d, \r\n", iq_proc_iso, raw_va_a->g2_th_l);
	PRINT_IQ_INFO(sfile, "auto_param[%d].g2_th_u = %d, \r\n", iq_proc_iso, raw_va_a->g2_th_u);

	// iq_va_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_va_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "pre_filter_enable = %d, \r\n", iq_param[iq_proc_id]->va->pre_filter_enable);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->va->mode);
	PRINT_IQ_INFO(sfile, "g1_tap_a = %d, \r\n", iq_param[iq_proc_id]->va->g1_tap_a);
	PRINT_IQ_INFO(sfile, "g1_tap_b = %d, \r\n", iq_param[iq_proc_id]->va->g1_tap_b);
	PRINT_IQ_INFO(sfile, "g1_tap_c = %d, \r\n", iq_param[iq_proc_id]->va->g1_tap_c);
	PRINT_IQ_INFO(sfile, "g1_tap_d = %d, \r\n", iq_param[iq_proc_id]->va->g1_tap_d);
	PRINT_IQ_INFO(sfile, "g1_div = %d, \r\n", iq_param[iq_proc_id]->va->g1_div);
	PRINT_IQ_INFO(sfile, "g2_tap_a = %d, \r\n", iq_param[iq_proc_id]->va->g2_tap_a);
	PRINT_IQ_INFO(sfile, "g2_tap_b = %d, \r\n", iq_param[iq_proc_id]->va->g2_tap_b);
	PRINT_IQ_INFO(sfile, "g2_tap_c = %d, \r\n", iq_param[iq_proc_id]->va->g2_tap_c);
	PRINT_IQ_INFO(sfile, "g2_tap_d = %d, \r\n", iq_param[iq_proc_id]->va->g2_tap_d);
	PRINT_IQ_INFO(sfile, "g2_div = %d, \r\n", iq_param[iq_proc_id]->va->g2_div);
	PRINT_IQ_INFO(sfile, "ldg_low_th = %d, \r\n", iq_param[iq_proc_id]->va->ldg_low_th);
	PRINT_IQ_INFO(sfile, "ldg_high_th = %d, \r\n", iq_param[iq_proc_id]->va->ldg_high_th);
	PRINT_IQ_INFO(sfile, "ldg_low_gain = %d, \r\n", iq_param[iq_proc_id]->va->ldg_low_gain);
	PRINT_IQ_INFO(sfile, "ldg_high_gain = %d, \r\n", iq_param[iq_proc_id]->va->ldg_high_gain);
	PRINT_IQ_INFO(sfile, "ldg_low_slope = %d, \r\n", iq_param[iq_proc_id]->va->ldg_low_slope);
	PRINT_IQ_INFO(sfile, "ldg_high_slope = %d, \r\n", iq_param[iq_proc_id]->va->ldg_high_slope);
	PRINT_IQ_INFO(sfile, "energy_w = %d, \r\n", iq_param[iq_proc_id]->va->energy_w);
	PRINT_IQ_INFO(sfile, "win_cnt_out_sel = %d, \r\n", iq_param[iq_proc_id]->va->win_cnt_out_sel);
	PRINT_IQ_INFO(sfile, "high_luma_th = %d, \r\n", iq_param[iq_proc_id]->va->high_luma_th);
	va_m = &iq_param[iq_proc_id]->va->manual_param;
	PRINT_IQ_INFO(sfile, "manual_param.g1_th_l = %d, \r\n", va_m->g1_th_l);
	PRINT_IQ_INFO(sfile, "manual_param.g1_th_u = %d, \r\n", va_m->g1_th_u);
	PRINT_IQ_INFO(sfile, "manual_param.g2_th_l = %d, \r\n", va_m->g2_th_l);
	PRINT_IQ_INFO(sfile, "manual_param.g2_th_u = %d, \r\n", va_m->g2_th_u);
	va_a = &iq_param[iq_proc_id]->va->auto_param[iq_proc_iso];
	PRINT_IQ_INFO(sfile, "auto_param[%d].g1_th_l = %d, \r\n", iq_proc_iso, va_a->g1_th_l);
	PRINT_IQ_INFO(sfile, "auto_param[%d].g1_th_u = %d, \r\n", iq_proc_iso, va_a->g1_th_u);
	PRINT_IQ_INFO(sfile, "auto_param[%d].g2_th_l = %d, \r\n", iq_proc_iso, va_a->g2_th_l);
	PRINT_IQ_INFO(sfile, "auto_param[%d].g2_th_u = %d, \r\n", iq_proc_iso, va_a->g2_th_u);

	// iq_tone_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_tone_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "enable = %d, \r\n", iq_param[iq_proc_id]->tone->enable);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->tone->mode);
	PRINT_IQ_INFO(sfile, "auto_sel = %d, \r\n", iq_param[iq_proc_id]->tone->auto_sel);
	PRINT_IQ_INFO(sfile, "tone_in_yv_blend_lut = %d, %d, %d, %d, %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->tone->tone_in_yv_blend_lut[0], iq_param[iq_proc_id]->tone->tone_in_yv_blend_lut[1]
		, iq_param[iq_proc_id]->tone->tone_in_yv_blend_lut[2], iq_param[iq_proc_id]->tone->tone_in_yv_blend_lut[3]
		, iq_param[iq_proc_id]->tone->tone_in_yv_blend_lut[4], iq_param[iq_proc_id]->tone->tone_in_yv_blend_lut[5]
		, iq_param[iq_proc_id]->tone->tone_in_yv_blend_lut[6], iq_param[iq_proc_id]->tone->tone_in_yv_blend_lut[7]
		, iq_param[iq_proc_id]->tone->tone_in_yv_blend_lut[8]);
	PRINT_IQ_INFO(sfile, "manual_lut_left = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->tone->manual_lut_left[0], iq_param[iq_proc_id]->tone->manual_lut_left[1]
		, iq_param[iq_proc_id]->tone->manual_lut_left[2], iq_param[iq_proc_id]->tone->manual_lut_left[3]
		, iq_param[iq_proc_id]->tone->manual_lut_left[4]);
	PRINT_IQ_INFO(sfile, "manual_lut_right = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->tone->manual_lut_right[0], iq_param[iq_proc_id]->tone->manual_lut_right[1]
		, iq_param[iq_proc_id]->tone->manual_lut_right[2], iq_param[iq_proc_id]->tone->manual_lut_right[3]
		, iq_param[iq_proc_id]->tone->manual_lut_right[4]);
	PRINT_IQ_INFO(sfile, "auto_set0_level = %d, \r\n", iq_param[iq_proc_id]->tone->auto_set0_level);
	PRINT_IQ_INFO(sfile, "auto_set0_lut_left = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->tone->auto_set0_lut_left[0], iq_param[iq_proc_id]->tone->auto_set0_lut_left[1]
		, iq_param[iq_proc_id]->tone->auto_set0_lut_left[2], iq_param[iq_proc_id]->tone->auto_set0_lut_left[3]
		, iq_param[iq_proc_id]->tone->auto_set0_lut_left[4]);
	PRINT_IQ_INFO(sfile, "auto_set0_lut_right = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->tone->auto_set0_lut_right[0], iq_param[iq_proc_id]->tone->auto_set0_lut_right[1]
		, iq_param[iq_proc_id]->tone->auto_set0_lut_right[2], iq_param[iq_proc_id]->tone->auto_set0_lut_right[3]
		, iq_param[iq_proc_id]->tone->auto_set0_lut_right[4]);
	PRINT_IQ_INFO(sfile, "auto_set1_level = %d, \r\n", iq_param[iq_proc_id]->tone->auto_set1_level);
	PRINT_IQ_INFO(sfile, "auto_set1_lut_left = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->tone->auto_set1_lut_left[0], iq_param[iq_proc_id]->tone->auto_set1_lut_left[1]
		, iq_param[iq_proc_id]->tone->auto_set1_lut_left[2], iq_param[iq_proc_id]->tone->auto_set1_lut_left[3]
		, iq_param[iq_proc_id]->tone->auto_set1_lut_left[4]);
	PRINT_IQ_INFO(sfile, "auto_set1_lut_right = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->tone->auto_set1_lut_right[0], iq_param[iq_proc_id]->tone->auto_set1_lut_right[1]
		, iq_param[iq_proc_id]->tone->auto_set1_lut_right[2], iq_param[iq_proc_id]->tone->auto_set1_lut_right[3]
		, iq_param[iq_proc_id]->tone->auto_set1_lut_right[4]);
	PRINT_IQ_INFO(sfile, "auto_set2_level = %d, \r\n", iq_param[iq_proc_id]->tone->auto_set2_level);
	PRINT_IQ_INFO(sfile, "auto_set2_lut_left = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->tone->auto_set2_lut_left[0], iq_param[iq_proc_id]->tone->auto_set2_lut_left[1]
		, iq_param[iq_proc_id]->tone->auto_set2_lut_left[2], iq_param[iq_proc_id]->tone->auto_set2_lut_left[3]
		, iq_param[iq_proc_id]->tone->auto_set2_lut_left[4]);
	PRINT_IQ_INFO(sfile, "auto_set2_lut_right = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->tone->auto_set2_lut_right[0], iq_param[iq_proc_id]->tone->auto_set2_lut_right[1]
		, iq_param[iq_proc_id]->tone->auto_set2_lut_right[2], iq_param[iq_proc_id]->tone->auto_set2_lut_right[3]
		, iq_param[iq_proc_id]->tone->auto_set2_lut_right[4]);
	PRINT_IQ_INFO(sfile, "auto_set3_level = %d, \r\n", iq_param[iq_proc_id]->tone->auto_set3_level);
	PRINT_IQ_INFO(sfile, "auto_set3_lut_left = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->tone->auto_set3_lut_left[0], iq_param[iq_proc_id]->tone->auto_set3_lut_left[1]
		, iq_param[iq_proc_id]->tone->auto_set3_lut_left[2], iq_param[iq_proc_id]->tone->auto_set3_lut_left[3]
		, iq_param[iq_proc_id]->tone->auto_set3_lut_left[4]);
	PRINT_IQ_INFO(sfile, "auto_set3_lut_right = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->tone->auto_set3_lut_right[0], iq_param[iq_proc_id]->tone->auto_set3_lut_right[1]
		, iq_param[iq_proc_id]->tone->auto_set3_lut_right[2], iq_param[iq_proc_id]->tone->auto_set3_lut_right[3]
		, iq_param[iq_proc_id]->tone->auto_set3_lut_right[4]);
	PRINT_IQ_INFO(sfile, "auto_set4_level = %d, \r\n", iq_param[iq_proc_id]->tone->auto_set4_level);
	PRINT_IQ_INFO(sfile, "auto_set4_lut_left = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->tone->auto_set4_lut_left[0], iq_param[iq_proc_id]->tone->auto_set4_lut_left[1]
		, iq_param[iq_proc_id]->tone->auto_set4_lut_left[2], iq_param[iq_proc_id]->tone->auto_set4_lut_left[3]
		, iq_param[iq_proc_id]->tone->auto_set4_lut_left[4]);
	PRINT_IQ_INFO(sfile, "auto_set4_lut_right = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->tone->auto_set4_lut_right[0], iq_param[iq_proc_id]->tone->auto_set4_lut_right[1]
		, iq_param[iq_proc_id]->tone->auto_set4_lut_right[2], iq_param[iq_proc_id]->tone->auto_set4_lut_right[3]
		, iq_param[iq_proc_id]->tone->auto_set4_lut_right[4]);
	if (iq_proc_iso < IQ_TONE_ID_MAX_NUM) {
		tone_a = &iq_param[iq_proc_id]->tone->auto_param[iq_proc_iso];
		PRINT_IQ_INFO(sfile, "auto_param[%d].lv = %d, \r\n", iq_proc_iso, tone_a->lv);
		PRINT_IQ_INFO(sfile, "auto_param[%d].tone_level = %d, \r\n", iq_proc_iso, tone_a->tone_level);
	}

	// iq_gamma_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_gamma_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "enable = %d, \r\n", iq_param[iq_proc_id]->gamma->enable);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->gamma->mode);
	PRINT_IQ_INFO(sfile, "auto_sel = %d, \r\n", iq_param[iq_proc_id]->gamma->auto_sel);
	PRINT_IQ_INFO(sfile, "manual_lut = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->gamma->manual_lut[0], iq_param[iq_proc_id]->gamma->manual_lut[1]
		, iq_param[iq_proc_id]->gamma->manual_lut[2], iq_param[iq_proc_id]->gamma->manual_lut[3]
		, iq_param[iq_proc_id]->gamma->manual_lut[4]);
	PRINT_IQ_INFO(sfile, "auto_set0_level = %d, \r\n", iq_param[iq_proc_id]->gamma->auto_set0_level);
	PRINT_IQ_INFO(sfile, "auto_set0_lut = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->gamma->auto_set0_lut[0], iq_param[iq_proc_id]->gamma->auto_set0_lut[1]
		, iq_param[iq_proc_id]->gamma->auto_set0_lut[2], iq_param[iq_proc_id]->gamma->auto_set0_lut[3]
		, iq_param[iq_proc_id]->gamma->auto_set0_lut[4]);
	PRINT_IQ_INFO(sfile, "auto_set1_level = %d, \r\n", iq_param[iq_proc_id]->gamma->auto_set1_level);
	PRINT_IQ_INFO(sfile, "auto_set1_lut = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->gamma->auto_set1_lut[0], iq_param[iq_proc_id]->gamma->auto_set1_lut[1]
		, iq_param[iq_proc_id]->gamma->auto_set1_lut[2], iq_param[iq_proc_id]->gamma->auto_set1_lut[3]
		, iq_param[iq_proc_id]->gamma->auto_set1_lut[4]);
	PRINT_IQ_INFO(sfile, "auto_set2_level = %d, \r\n", iq_param[iq_proc_id]->gamma->auto_set2_level);
	PRINT_IQ_INFO(sfile, "auto_set2_lut = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->gamma->auto_set2_lut[0], iq_param[iq_proc_id]->gamma->auto_set2_lut[1]
		, iq_param[iq_proc_id]->gamma->auto_set2_lut[2], iq_param[iq_proc_id]->gamma->auto_set2_lut[3]
		, iq_param[iq_proc_id]->gamma->auto_set2_lut[4]);
	PRINT_IQ_INFO(sfile, "auto_set3_level = %d, \r\n", iq_param[iq_proc_id]->gamma->auto_set3_level);
	PRINT_IQ_INFO(sfile, "auto_set3_lut = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->gamma->auto_set3_lut[0], iq_param[iq_proc_id]->gamma->auto_set3_lut[1]
		, iq_param[iq_proc_id]->gamma->auto_set3_lut[2], iq_param[iq_proc_id]->gamma->auto_set3_lut[3]
		, iq_param[iq_proc_id]->gamma->auto_set3_lut[4]);
	PRINT_IQ_INFO(sfile, "auto_set4_level = %d, \r\n", iq_param[iq_proc_id]->gamma->auto_set4_level);
	PRINT_IQ_INFO(sfile, "auto_set4_lut = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->gamma->auto_set4_lut[0], iq_param[iq_proc_id]->gamma->auto_set4_lut[1]
		, iq_param[iq_proc_id]->gamma->auto_set4_lut[2], iq_param[iq_proc_id]->gamma->auto_set4_lut[3]
		, iq_param[iq_proc_id]->gamma->auto_set4_lut[4]);
	if (iq_proc_iso < IQ_GAMMA_ID_MAX_NUM) {
		gamma_a = &iq_param[iq_proc_id]->gamma->auto_param[iq_proc_iso];
		PRINT_IQ_INFO(sfile, "auto_param[%d].lv = %d, \r\n", iq_proc_iso, gamma_a->lv);
		PRINT_IQ_INFO(sfile, "auto_param[%d].gamma_level = %d, \r\n", iq_proc_iso, gamma_a->gamma_level);
	}

	// iq_ccm_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_ccm_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "enable = %d, \r\n", iq_param[iq_proc_id]->ccm->enable);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->ccm->mode);
	ccm_m = &iq_param[iq_proc_id]->ccm->manual_param;
	PRINT_IQ_INFO(sfile, "manual_param.coef = %d, %d, %d, %d, %d, %d, %d, %d, %d, \r\n"
		, ccm_m->coef[0], ccm_m->coef[1], ccm_m->coef[2], ccm_m->coef[3], ccm_m->coef[4]
		, ccm_m->coef[5], ccm_m->coef[6], ccm_m->coef[7], ccm_m->coef[8]);
	PRINT_IQ_INFO(sfile, "manual_param.hue_tab = %d, %d, %d, %d, %d, \r\n"
		, ccm_m->hue_tab[0], ccm_m->hue_tab[1], ccm_m->hue_tab[2], ccm_m->hue_tab[3], ccm_m->hue_tab[4]);
	PRINT_IQ_INFO(sfile, "manual_param.sat_tab = %d, %d, %d, %d, %d, \r\n"
		, ccm_m->sat_tab[0], ccm_m->sat_tab[1], ccm_m->sat_tab[2], ccm_m->sat_tab[3], ccm_m->sat_tab[4]);
	PRINT_IQ_INFO(sfile, "manual_param.int_tab = %d, %d, %d, %d, %d, \r\n"
		, ccm_m->int_tab[0], ccm_m->int_tab[1], ccm_m->int_tab[2], ccm_m->int_tab[3], ccm_m->int_tab[4]);
	if (iq_proc_iso < IQ_COLOR_ID_MAX_NUM) {
		ccm_a = &iq_param[iq_proc_id]->ccm->auto_param[iq_proc_iso];
		PRINT_IQ_INFO(sfile, "auto_param[%d].ct = %d, \r\n", iq_proc_iso, ccm_a->ct);
		PRINT_IQ_INFO(sfile, "auto_param[%d].coef = %d, %d, %d, %d, %d, %d, %d, %d, %d, \r\n", iq_proc_iso
			, ccm_a->coef[0], ccm_a->coef[1], ccm_a->coef[2], ccm_a->coef[3], ccm_a->coef[4]
			, ccm_a->coef[5], ccm_a->coef[6], ccm_a->coef[7], ccm_a->coef[8]);
		PRINT_IQ_INFO(sfile, "auto_param[%d].hue_tab = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
			, ccm_a->hue_tab[0], ccm_a->hue_tab[1], ccm_a->hue_tab[2], ccm_a->hue_tab[3], ccm_a->hue_tab[4]);
		PRINT_IQ_INFO(sfile, "auto_param[%d].sat_tab = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
			, ccm_a->sat_tab[0], ccm_a->sat_tab[1], ccm_a->sat_tab[2], ccm_a->sat_tab[3], ccm_a->sat_tab[4]);
		PRINT_IQ_INFO(sfile, "auto_param[%d].int_tab = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
			, ccm_a->int_tab[0], ccm_a->int_tab[1], ccm_a->int_tab[2], ccm_a->int_tab[3], ccm_a->int_tab[4]);
	}

	// iq_color_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_color_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "enable = %d, \r\n", iq_param[iq_proc_id]->color->enable);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->color->mode);
	color_m = &iq_param[iq_proc_id]->color->manual_param;
	PRINT_IQ_INFO(sfile, "manual_param.c_con = %d, \r\n", color_m->c_con);
	PRINT_IQ_INFO(sfile, "manual_param.fstab = %d, %d, %d, %d, %d, \r\n"
		, color_m->fstab[0], color_m->fstab[1], color_m->fstab[2], color_m->fstab[3], color_m->fstab[4]);
	PRINT_IQ_INFO(sfile, "manual_param.fdtab = %d, %d, %d, %d, %d, \r\n"
		, color_m->fdtab[0], color_m->fdtab[1], color_m->fdtab[2], color_m->fdtab[3], color_m->fdtab[4]);
	PRINT_IQ_INFO(sfile, "manual_param.cconlut = %d, %d, %d, %d, %d, \r\n"
		, color_m->cconlut[0], color_m->cconlut[1], color_m->cconlut[2], color_m->cconlut[3], color_m->cconlut[4]);
	color_a = &iq_param[iq_proc_id]->color->auto_param[iq_proc_iso];
	PRINT_IQ_INFO(sfile, "auto_param[%d].c_con = %d, \r\n", iq_proc_iso, color_a->c_con);
	PRINT_IQ_INFO(sfile, "auto_param[%d].fstab = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, color_a->fstab[0], color_a->fstab[1], color_a->fstab[2], color_a->fstab[3], color_a->fstab[4]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].fstab = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, color_a->fdtab[0], color_a->fdtab[1], color_a->fdtab[2], color_a->fdtab[3], color_a->fdtab[4]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].fstab = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, color_a->cconlut[0], color_a->cconlut[1], color_a->cconlut[2], color_a->cconlut[3], color_a->cconlut[4]);

	// iq_contrast_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_contrast_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->contrast->mode);
	contrast_m = &iq_param[iq_proc_id]->contrast->manual_param;
	PRINT_IQ_INFO(sfile, "manual_param.y_con = %d, \r\n", contrast_m->y_con);
	PRINT_IQ_INFO(sfile, "manual_param.lce_lum_wt_lut = %d, %d, %d, %d, %d, \r\n"
		, contrast_m->lce_lum_wt_lut[0], contrast_m->lce_lum_wt_lut[1], contrast_m->lce_lum_wt_lut[2], contrast_m->lce_lum_wt_lut[3], contrast_m->lce_lum_wt_lut[4]);
	contrast_a = &iq_param[iq_proc_id]->contrast->auto_param[iq_proc_iso];
	PRINT_IQ_INFO(sfile, "auto_param[%d].y_con = %d, \r\n", iq_proc_iso, contrast_a->y_con);
	PRINT_IQ_INFO(sfile, "auto_param[%d].lce_lum_wt_lut = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, contrast_a->lce_lum_wt_lut[0], contrast_a->lce_lum_wt_lut[1], contrast_a->lce_lum_wt_lut[2], contrast_a->lce_lum_wt_lut[3], contrast_a->lce_lum_wt_lut[4]);

	// iq_edge_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_edge_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "enable = %d, \r\n", iq_param[iq_proc_id]->edge->enable);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->edge->mode);
	PRINT_IQ_INFO(sfile, "th_overshoot = %d, \r\n", iq_param[iq_proc_id]->edge->th_overshoot);
	PRINT_IQ_INFO(sfile, "th_undershoot = %d, \r\n", iq_param[iq_proc_id]->edge->th_undershoot);
	PRINT_IQ_INFO(sfile, "blending_th = %d, \r\n", iq_param[iq_proc_id]->edge->blending_th);
	PRINT_IQ_INFO(sfile, "blending_low_luma_w = %d, \r\n", iq_param[iq_proc_id]->edge->blending_low_luma_w);
	PRINT_IQ_INFO(sfile, "blending_high_luma_w = %d, \r\n", iq_param[iq_proc_id]->edge->blending_high_luma_w);
	PRINT_IQ_INFO(sfile, "edge_map_lut = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->edge->edge_map_lut[0], iq_param[iq_proc_id]->edge->edge_map_lut[1]
		, iq_param[iq_proc_id]->edge->edge_map_lut[2], iq_param[iq_proc_id]->edge->edge_map_lut[3]
		, iq_param[iq_proc_id]->edge->edge_map_lut[4]);
	PRINT_IQ_INFO(sfile, "es_map_lut = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->edge->es_map_lut[0], iq_param[iq_proc_id]->edge->es_map_lut[1]
		, iq_param[iq_proc_id]->edge->es_map_lut[2], iq_param[iq_proc_id]->edge->es_map_lut[3]
		, iq_param[iq_proc_id]->edge->es_map_lut[4]);
	PRINT_IQ_INFO(sfile, "edge_tab = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->edge->edge_tab[0], iq_param[iq_proc_id]->edge->edge_tab[1]
		, iq_param[iq_proc_id]->edge->edge_tab[2], iq_param[iq_proc_id]->edge->edge_tab[3]
		, iq_param[iq_proc_id]->edge->edge_tab[4]);
	PRINT_IQ_INFO(sfile, "reduce_ratio = %d, \r\n", iq_param[iq_proc_id]->edge->reduce_ratio);
	edge_m = &iq_param[iq_proc_id]->edge->manual_param;
	PRINT_IQ_INFO(sfile, "manual_param.edge_enh_p = %d, \r\n", edge_m->edge_enh_p);
	PRINT_IQ_INFO(sfile, "manual_param.edge_enh_n = %d, \r\n", edge_m->edge_enh_n);
	PRINT_IQ_INFO(sfile, "manual_param.thin_freq = %d, \r\n", edge_m->thin_freq);
	PRINT_IQ_INFO(sfile, "manual_param.robust_freq = %d, \r\n", edge_m->robust_freq);
	PRINT_IQ_INFO(sfile, "manual_param.wt_low = %d, \r\n", edge_m->wt_low);
	PRINT_IQ_INFO(sfile, "manual_param.wt_high = %d, \r\n", edge_m->wt_high);
	PRINT_IQ_INFO(sfile, "manual_param.th_flat_low = %d, \r\n", edge_m->th_flat_low);
	PRINT_IQ_INFO(sfile, "manual_param.th_flat_high = %d, \r\n", edge_m->th_flat_high);
	PRINT_IQ_INFO(sfile, "manual_param.th_edge_low = %d, \r\n", edge_m->th_edge_low);
	PRINT_IQ_INFO(sfile, "manual_param.th_edge_high = %d, \r\n", edge_m->th_edge_high);
	PRINT_IQ_INFO(sfile, "manual_param.str_flat = %d, \r\n", edge_m->str_flat);
	PRINT_IQ_INFO(sfile, "manual_param.str_edge = %d, \r\n", edge_m->str_edge);
	PRINT_IQ_INFO(sfile, "manual_param.overshoot_str = %d, \r\n", edge_m->overshoot_str);
	PRINT_IQ_INFO(sfile, "manual_param.undershoot_str = %d, \r\n", edge_m->undershoot_str);
	PRINT_IQ_INFO(sfile, "manual_param.edge_ethr_low = %d, \r\n", edge_m->edge_ethr_low);
	PRINT_IQ_INFO(sfile, "manual_param.edge_ethr_high = %d, \r\n", edge_m->edge_ethr_high);
	PRINT_IQ_INFO(sfile, "manual_param.edge_etab_low = %d, \r\n", edge_m->edge_etab_low);
	PRINT_IQ_INFO(sfile, "manual_param.edge_etab_high = %d, \r\n", edge_m->edge_etab_high);
	PRINT_IQ_INFO(sfile, "manual_param.es_ethr_low = %d, \r\n", edge_m->es_ethr_low);
	PRINT_IQ_INFO(sfile, "manual_param.es_ethr_high = %d, \r\n", edge_m->es_ethr_high);
	PRINT_IQ_INFO(sfile, "manual_param.es_etab_low = %d, \r\n", edge_m->es_etab_low);
	PRINT_IQ_INFO(sfile, "manual_param.es_etab_high = %d, \r\n", edge_m->es_etab_high);
	PRINT_IQ_INFO(sfile, "manual_param.dir_eng_blend_w = %d, \r\n", edge_m->dir_eng_blend_w);
	edge_a = &iq_param[iq_proc_id]->edge->auto_param[iq_proc_iso];
	PRINT_IQ_INFO(sfile, "auto_param[%d].edge_enh_p = %d, \r\n", iq_proc_iso, edge_a->edge_enh_p);
	PRINT_IQ_INFO(sfile, "auto_param[%d].edge_enh_n = %d, \r\n", iq_proc_iso, edge_a->edge_enh_n);
	PRINT_IQ_INFO(sfile, "auto_param[%d].thin_freq = %d, \r\n", iq_proc_iso, edge_a->thin_freq);
	PRINT_IQ_INFO(sfile, "auto_param[%d].robust_freq = %d, \r\n", iq_proc_iso, edge_a->robust_freq);
	PRINT_IQ_INFO(sfile, "auto_param[%d].wt_low = %d, \r\n", iq_proc_iso, edge_a->wt_low);
	PRINT_IQ_INFO(sfile, "auto_param[%d].wt_high = %d, \r\n", iq_proc_iso, edge_a->wt_high);
	PRINT_IQ_INFO(sfile, "auto_param[%d].th_flat_low = %d, \r\n", iq_proc_iso, edge_a->th_flat_low);
	PRINT_IQ_INFO(sfile, "auto_param[%d].th_flat_high = %d, \r\n", iq_proc_iso, edge_a->th_flat_high);
	PRINT_IQ_INFO(sfile, "auto_param[%d].th_edge_low = %d, \r\n", iq_proc_iso, edge_a->th_edge_low);
	PRINT_IQ_INFO(sfile, "auto_param[%d].th_edge_high = %d, \r\n", iq_proc_iso, edge_a->th_edge_high);
	PRINT_IQ_INFO(sfile, "auto_param[%d].str_flat = %d, \r\n", iq_proc_iso, edge_a->str_flat);
	PRINT_IQ_INFO(sfile, "auto_param[%d].str_edge = %d, \r\n", iq_proc_iso, edge_a->str_edge);
	PRINT_IQ_INFO(sfile, "auto_param[%d].overshoot_str = %d, \r\n", iq_proc_iso, edge_a->overshoot_str);
	PRINT_IQ_INFO(sfile, "auto_param[%d].undershoot_str = %d, \r\n", iq_proc_iso, edge_a->undershoot_str);
	PRINT_IQ_INFO(sfile, "auto_param[%d].edge_ethr_low = %d, \r\n", iq_proc_iso, edge_a->edge_ethr_low);
	PRINT_IQ_INFO(sfile, "auto_param[%d].edge_ethr_high = %d, \r\n", iq_proc_iso, edge_a->edge_ethr_high);
	PRINT_IQ_INFO(sfile, "auto_param[%d].edge_etab_low = %d, \r\n", iq_proc_iso, edge_a->edge_etab_low);
	PRINT_IQ_INFO(sfile, "auto_param[%d].edge_etab_high = %d, \r\n", iq_proc_iso, edge_a->edge_etab_high);
	PRINT_IQ_INFO(sfile, "auto_param[%d].es_ethr_low = %d, \r\n", iq_proc_iso, edge_a->es_ethr_low);
	PRINT_IQ_INFO(sfile, "auto_param[%d].es_ethr_high = %d, \r\n", iq_proc_iso, edge_a->es_ethr_high);
	PRINT_IQ_INFO(sfile, "auto_param[%d].es_etab_low = %d, \r\n", iq_proc_iso, edge_a->es_etab_low);
	PRINT_IQ_INFO(sfile, "auto_param[%d].es_etab_high = %d, \r\n", iq_proc_iso, edge_a->es_etab_high);
	PRINT_IQ_INFO(sfile, "auto_param[%d].dir_eng_blend_w = %d, \r\n", iq_proc_iso, edge_a->dir_eng_blend_w);

	// iq_3dnr_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_3dnr_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "enable = %d, \r\n", iq_param[iq_proc_id]->_3dnr->enable);
	PRINT_IQ_INFO(sfile, "fcvg_enable = %d, \r\n", iq_param[iq_proc_id]->_3dnr->fcvg_enable);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->_3dnr->mode);
	_3dnr_m = &iq_param[iq_proc_id]->_3dnr->manual_param;
	PRINT_IQ_INFO(sfile, "manual_param.pf_str = %d, \r\n", _3dnr_m->pf_str);
	PRINT_IQ_INFO(sfile, "manual_param.cost_blend = %d, \r\n", _3dnr_m->cost_blend);
	PRINT_IQ_INFO(sfile, "manual_param.sad_penalty = %d, %d, %d, %d, %d, \r\n"
		, _3dnr_m->sad_penalty[0], _3dnr_m->sad_penalty[1], _3dnr_m->sad_penalty[2], _3dnr_m->sad_penalty[3], _3dnr_m->sad_penalty[4]);
	PRINT_IQ_INFO(sfile, "manual_param.detail_penalty = %d, %d, %d, %d, %d, \r\n"
		, _3dnr_m->detail_penalty[0], _3dnr_m->detail_penalty[1], _3dnr_m->detail_penalty[2], _3dnr_m->detail_penalty[3], _3dnr_m->detail_penalty[4]);
	PRINT_IQ_INFO(sfile, "manual_param.switch_th = %d, %d, %d, %d, %d, \r\n"
		, _3dnr_m->switch_th[0], _3dnr_m->switch_th[1], _3dnr_m->switch_th[2], _3dnr_m->switch_th[3], _3dnr_m->switch_th[4]);
	PRINT_IQ_INFO(sfile, "manual_param.switch_rto = %d, \r\n", _3dnr_m->switch_rto);
	PRINT_IQ_INFO(sfile, "manual_param.probability = %d, \r\n", _3dnr_m->probability);
	PRINT_IQ_INFO(sfile, "manual_param.sad_base = %d, %d, %d, %d, %d, \r\n"
		, _3dnr_m->sad_base[0], _3dnr_m->sad_base[1], _3dnr_m->sad_base[2], _3dnr_m->sad_base[3], _3dnr_m->sad_base[4]);
	PRINT_IQ_INFO(sfile, "manual_param.sad_coefa = %d, %d, %d, %d, %d, \r\n"
		, _3dnr_m->sad_coefa[0], _3dnr_m->sad_coefa[1], _3dnr_m->sad_coefa[2], _3dnr_m->sad_coefa[3], _3dnr_m->sad_coefa[4]);
	PRINT_IQ_INFO(sfile, "manual_param.sad_coefb = %d, %d, %d, %d, %d, \r\n"
		, _3dnr_m->sad_coefb[0], _3dnr_m->sad_coefb[1], _3dnr_m->sad_coefb[2], _3dnr_m->sad_coefb[3], _3dnr_m->sad_coefb[4]);
	PRINT_IQ_INFO(sfile, "manual_param.sad_std = %d, %d, %d, %d, %d, \r\n"
		, _3dnr_m->sad_std[0], _3dnr_m->sad_std[1], _3dnr_m->sad_std[2], _3dnr_m->sad_std[3], _3dnr_m->sad_std[4]);
	PRINT_IQ_INFO(sfile, "manual_param.fth = %d, %d, \r\n", _3dnr_m->fth[0], _3dnr_m->fth[1]);
	PRINT_IQ_INFO(sfile, "manual_param.mv_th = %d, \r\n", _3dnr_m->mv_th);
	PRINT_IQ_INFO(sfile, "manual_param.mix_ratio = %d, %d, \r\n", _3dnr_m->mix_ratio[0], _3dnr_m->mix_ratio[1]);
	PRINT_IQ_INFO(sfile, "manual_param.ds_th = %d, \r\n", _3dnr_m->ds_th);
	PRINT_IQ_INFO(sfile, "manual_param.blur_eth = %d, \r\n", _3dnr_m->blur_eth);
	PRINT_IQ_INFO(sfile, "manual_param.luma_residue_th = %d, %d, %d, \r\n", _3dnr_m->luma_residue_th[0], _3dnr_m->luma_residue_th[1], _3dnr_m->luma_residue_th[2]);
	PRINT_IQ_INFO(sfile, "manual_param.chroma_residue_th = %d, \r\n", _3dnr_m->chroma_residue_th);
	PRINT_IQ_INFO(sfile, "manual_param.tf0_blur_str = %d, %d, %d\r\n"
		, _3dnr_m->tf0_blur_str[0], _3dnr_m->tf0_blur_str[1], _3dnr_m->tf0_blur_str[2]);
	PRINT_IQ_INFO(sfile, "manual_param.tf0_blur_estr = %d, \r\n", _3dnr_m->tf0_blur_estr);
	PRINT_IQ_INFO(sfile, "manual_param.tf0_y_str = %d, %d, %d\r\n"
		, _3dnr_m->tf0_y_str[0], _3dnr_m->tf0_y_str[1], _3dnr_m->tf0_y_str[2]);
	PRINT_IQ_INFO(sfile, "manual_param.tf0_c_str = %d, %d, %d\r\n"
		, _3dnr_m->tf0_c_str[0], _3dnr_m->tf0_c_str[1], _3dnr_m->tf0_c_str[2]);
	PRINT_IQ_INFO(sfile, "manual_param.tf0_u_th = %d, \r\n", _3dnr_m->tf0_u_th);
	PRINT_IQ_INFO(sfile, "manual_param.tf0_v_th = %d, \r\n", _3dnr_m->tf0_v_th);
	PRINT_IQ_INFO(sfile, "manual_param.tf0_uv_ratio = %d, %d\r\n"
		, _3dnr_m->tf0_uv_ratio[0], _3dnr_m->tf0_uv_ratio[1]);
	PRINT_IQ_INFO(sfile, "manual_param.pre_filter_str = %d, %d, %d, %d\r\n"
		, _3dnr_m->pre_filter_str[0], _3dnr_m->pre_filter_str[1], _3dnr_m->pre_filter_str[2], _3dnr_m->pre_filter_str[3]);
	PRINT_IQ_INFO(sfile, "manual_param.pre_filter_rto = %d, %d, \r\n", _3dnr_m->pre_filter_rto[0], _3dnr_m->pre_filter_rto[1]);
	PRINT_IQ_INFO(sfile, "manual_param.snr_base_th = %d, \r\n", _3dnr_m->snr_base_th);
	PRINT_IQ_INFO(sfile, "manual_param.tnr_base_th = %d, \r\n", _3dnr_m->tnr_base_th);
	PRINT_IQ_INFO(sfile, "manual_param.freq_wet = %d, %d, %d, %d\r\n"
		, _3dnr_m->freq_wet[0], _3dnr_m->freq_wet[1], _3dnr_m->freq_wet[2], _3dnr_m->freq_wet[3]);
	PRINT_IQ_INFO(sfile, "manual_param.luma_wet = %d, %d, %d, %d, %d, \r\n"
		, _3dnr_m->luma_wet[0], _3dnr_m->luma_wet[1], _3dnr_m->luma_wet[2], _3dnr_m->luma_wet[3], _3dnr_m->luma_wet[4]);
	PRINT_IQ_INFO(sfile, "manual_param.snr_str = %d, %d, %d\r\n"
		, _3dnr_m->snr_str[0], _3dnr_m->snr_str[1], _3dnr_m->snr_str[2]);
	PRINT_IQ_INFO(sfile, "manual_param.tnr_str = %d, %d, %d\r\n"
		, _3dnr_m->tnr_str[0], _3dnr_m->tnr_str[1], _3dnr_m->tnr_str[2]);
	PRINT_IQ_INFO(sfile, "manual_param.luma_3d_lut = %d, %d, %d, %d, %d, \r\n"
		, _3dnr_m->luma_3d_lut[0], _3dnr_m->luma_3d_lut[1], _3dnr_m->luma_3d_lut[2], _3dnr_m->luma_3d_lut[3], _3dnr_m->luma_3d_lut[4]);
	PRINT_IQ_INFO(sfile, "manual_param.luma_3d_rto = %d, %d, \r\n", _3dnr_m->luma_3d_rto[0], _3dnr_m->luma_3d_rto[1]);
	PRINT_IQ_INFO(sfile, "manual_param.chroma_3d_lut = %d, %d, %d, %d, %d, \r\n"
		, _3dnr_m->chroma_3d_lut[0], _3dnr_m->chroma_3d_lut[1], _3dnr_m->chroma_3d_lut[2], _3dnr_m->chroma_3d_lut[3], _3dnr_m->chroma_3d_lut[4]);
	PRINT_IQ_INFO(sfile, "manual_param.chroma_3d_rto = %d, %d, \r\n", _3dnr_m->chroma_3d_rto[0], _3dnr_m->chroma_3d_rto[1]);
	PRINT_IQ_INFO(sfile, "manual_param.luma_comp_str = %d, \r\n", _3dnr_m->luma_comp_str);
	PRINT_IQ_INFO(sfile, "manual_param.fcvg_start_point = %d, \r\n", _3dnr_m->fcvg_start_point);
	PRINT_IQ_INFO(sfile, "manual_param.fcvg_step_size = %d, \r\n", _3dnr_m->fcvg_step_size);
	PRINT_IQ_INFO(sfile, "manual_param.motion_sat_ratio = %d, \r\n", _3dnr_m->motion_sat_ratio);
	PRINT_IQ_INFO(sfile, "manual_param.cshk_th = %d, %d, %d, %d, %d, \r\n"
		, _3dnr_m->cshk_th[0], _3dnr_m->cshk_th[1], _3dnr_m->cshk_th[2], _3dnr_m->cshk_th[3], _3dnr_m->cshk_th[4]);
	PRINT_IQ_INFO(sfile, "manual_param.cshk_val = %d, %d, %d, %d, %d, \r\n"
		, _3dnr_m->cshk_val[0], _3dnr_m->cshk_val[1], _3dnr_m->cshk_val[2], _3dnr_m->cshk_val[3], _3dnr_m->cshk_val[4]);
	_3dnr_a = &iq_param[iq_proc_id]->_3dnr->auto_param[iq_proc_iso];
	PRINT_IQ_INFO(sfile, "auto_param[%d].pf_str = %d, \r\n", iq_proc_iso, _3dnr_a->pf_str);
	PRINT_IQ_INFO(sfile, "auto_param[%d].cost_blend = %d, \r\n", iq_proc_iso, _3dnr_a->cost_blend);
	PRINT_IQ_INFO(sfile, "auto_param[%d].sad_penalty = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, _3dnr_a->sad_penalty[0], _3dnr_a->sad_penalty[1], _3dnr_a->sad_penalty[2], _3dnr_a->sad_penalty[3], _3dnr_a->sad_penalty[4]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].detail_penalty = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, _3dnr_a->detail_penalty[0], _3dnr_a->detail_penalty[1], _3dnr_a->detail_penalty[2], _3dnr_a->detail_penalty[3], _3dnr_a->detail_penalty[4]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].switch_th = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, _3dnr_a->switch_th[0], _3dnr_a->switch_th[1], _3dnr_a->switch_th[2], _3dnr_a->switch_th[3], _3dnr_a->switch_th[4]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].switch_rto = %d, \r\n", iq_proc_iso, _3dnr_a->switch_rto);
	PRINT_IQ_INFO(sfile, "auto_param[%d].probability = %d, \r\n", iq_proc_iso, _3dnr_a->probability);
	PRINT_IQ_INFO(sfile, "auto_param[%d].sad_base = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, _3dnr_a->sad_base[0], _3dnr_a->sad_base[1], _3dnr_a->sad_base[2], _3dnr_a->sad_base[3], _3dnr_a->sad_base[4]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].sad_coefa = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, _3dnr_a->sad_coefa[0], _3dnr_a->sad_coefa[1], _3dnr_a->sad_coefa[2], _3dnr_a->sad_coefa[3], _3dnr_a->sad_coefa[4]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].sad_coefb = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, _3dnr_a->sad_coefb[0], _3dnr_a->sad_coefb[1], _3dnr_a->sad_coefb[2], _3dnr_a->sad_coefb[3], _3dnr_a->sad_coefb[4]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].sad_std = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, _3dnr_a->sad_std[0], _3dnr_a->sad_std[1], _3dnr_a->sad_std[2], _3dnr_a->sad_std[3], _3dnr_a->sad_std[4]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].fth = %d, %d, \r\n", iq_proc_iso, _3dnr_a->fth[0], _3dnr_a->fth[1]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].mv_th = %d, \r\n", iq_proc_iso, _3dnr_a->mv_th);
	PRINT_IQ_INFO(sfile, "auto_param[%d].mix_ratio = %d, %d, \r\n", iq_proc_iso, _3dnr_a->mix_ratio[0], _3dnr_a->mix_ratio[1]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].ds_th = %d, \r\n", iq_proc_iso, _3dnr_a->ds_th);
	PRINT_IQ_INFO(sfile, "auto_param[%d].blur_eth = %d, \r\n", iq_proc_iso, _3dnr_a->blur_eth);
	PRINT_IQ_INFO(sfile, "auto_param[%d].luma_residue_th = %d, %d, %d, \r\n", iq_proc_iso, _3dnr_a->luma_residue_th[0], _3dnr_a->luma_residue_th[1], _3dnr_a->luma_residue_th[2]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].chroma_residue_th = %d, \r\n", iq_proc_iso, _3dnr_a->chroma_residue_th);
	PRINT_IQ_INFO(sfile, "auto_param[%d].tf0_blur_str = %d, %d, %d\r\n", iq_proc_iso
		, _3dnr_a->tf0_blur_str[0], _3dnr_a->tf0_blur_str[1], _3dnr_a->tf0_blur_str[2]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].tf0_blur_estr = %d, \r\n", iq_proc_iso, _3dnr_a->tf0_blur_estr);
	PRINT_IQ_INFO(sfile, "auto_param[%d].tf0_y_str = %d, %d, %d\r\n", iq_proc_iso
		, _3dnr_a->tf0_y_str[0], _3dnr_a->tf0_y_str[1], _3dnr_a->tf0_y_str[2]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].tf0_c_str = %d, %d, %d\r\n", iq_proc_iso
		, _3dnr_a->tf0_c_str[0], _3dnr_a->tf0_c_str[1], _3dnr_a->tf0_c_str[2]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].tf0_u_th = %d, \r\n", iq_proc_iso, _3dnr_a->tf0_u_th);
	PRINT_IQ_INFO(sfile, "auto_param[%d].tf0_v_th = %d, \r\n", iq_proc_iso, _3dnr_a->tf0_v_th);
	PRINT_IQ_INFO(sfile, "auto_param[%d].tf0_uv_ratio = %d, %d\r\n", iq_proc_iso
		, _3dnr_a->tf0_uv_ratio[0], _3dnr_a->tf0_uv_ratio[1]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].pre_filter_str = %d, %d, %d, %d\r\n", iq_proc_iso
		, _3dnr_a->pre_filter_str[0], _3dnr_a->pre_filter_str[1], _3dnr_a->pre_filter_str[2], _3dnr_a->pre_filter_str[3]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].pre_filter_rto = %d, %d, \r\n", iq_proc_iso, _3dnr_a->pre_filter_rto[0], _3dnr_a->pre_filter_rto[1]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].snr_base_th = %d, \r\n", iq_proc_iso, _3dnr_a->snr_base_th);
	PRINT_IQ_INFO(sfile, "auto_param[%d].tnr_base_th = %d, \r\n", iq_proc_iso, _3dnr_a->tnr_base_th);
	PRINT_IQ_INFO(sfile, "auto_param[%d].freq_wet = %d, %d, %d, %d\r\n", iq_proc_iso
		, _3dnr_a->freq_wet[0], _3dnr_a->freq_wet[1], _3dnr_a->freq_wet[2], _3dnr_a->freq_wet[3]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].luma_wet = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, _3dnr_a->luma_wet[0], _3dnr_a->luma_wet[1], _3dnr_a->luma_wet[2], _3dnr_a->luma_wet[3], _3dnr_a->luma_wet[4]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].snr_str = %d, %d, %d\r\n", iq_proc_iso
		, _3dnr_a->snr_str[0], _3dnr_a->snr_str[1], _3dnr_a->snr_str[2]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].tnr_str = %d, %d, %d\r\n", iq_proc_iso
		, _3dnr_a->tnr_str[0], _3dnr_a->tnr_str[1], _3dnr_a->tnr_str[2]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].luma_3d_lut = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, _3dnr_a->luma_3d_lut[0], _3dnr_a->luma_3d_lut[1], _3dnr_a->luma_3d_lut[2], _3dnr_a->luma_3d_lut[3], _3dnr_a->luma_3d_lut[4]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].luma_3d_rto = %d, %d, \r\n", iq_proc_iso, _3dnr_a->luma_3d_rto[0], _3dnr_a->luma_3d_rto[1]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].chroma_3d_lut = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, _3dnr_a->chroma_3d_lut[0], _3dnr_a->chroma_3d_lut[1], _3dnr_a->chroma_3d_lut[2], _3dnr_a->chroma_3d_lut[3], _3dnr_a->chroma_3d_lut[4]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].chroma_3d_rto = %d, %d, \r\n", iq_proc_iso, _3dnr_a->chroma_3d_rto[0], _3dnr_a->chroma_3d_rto[1]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].luma_comp_str = %d, \r\n", iq_proc_iso, _3dnr_a->luma_comp_str);
	PRINT_IQ_INFO(sfile, "auto_param[%d].fcvg_start_point = %d, \r\n", iq_proc_iso, _3dnr_a->fcvg_start_point);
	PRINT_IQ_INFO(sfile, "auto_param[%d].fcvg_step_size = %d, \r\n", iq_proc_iso, _3dnr_a->fcvg_step_size);
	PRINT_IQ_INFO(sfile, "auto_param[%d].motion_sat_ratio = %d, \r\n", iq_proc_iso, _3dnr_a->motion_sat_ratio);
	PRINT_IQ_INFO(sfile, "auto_param[%d].cshk_th = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, _3dnr_a->cshk_th[0], _3dnr_a->cshk_th[1], _3dnr_a->cshk_th[2], _3dnr_a->cshk_th[3], _3dnr_a->cshk_th[4]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].cshk_val = %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, _3dnr_a->cshk_val[0], _3dnr_a->cshk_val[1], _3dnr_a->cshk_val[2], _3dnr_a->cshk_val[3], _3dnr_a->cshk_val[4]);

	// iq_pfr_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_pfr_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "enable = %d, \r\n", iq_param[iq_proc_id]->pfr->enable);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->pfr->mode);
	PRINT_IQ_INFO(sfile, "luma_lut = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->pfr->luma_lut[0], iq_param[iq_proc_id]->pfr->luma_lut[1]
		, iq_param[iq_proc_id]->pfr->luma_lut[2], iq_param[iq_proc_id]->pfr->luma_lut[3]
		, iq_param[iq_proc_id]->pfr->luma_lut[4]);
	PRINT_IQ_INFO(sfile, "set0_en = %d, \r\n", iq_param[iq_proc_id]->pfr->set0_en);
	PRINT_IQ_INFO(sfile, "set0_color_u = %d, \r\n", iq_param[iq_proc_id]->pfr->set0_color_u);
	PRINT_IQ_INFO(sfile, "set0_color_v = %d, \r\n", iq_param[iq_proc_id]->pfr->set0_color_v);
	PRINT_IQ_INFO(sfile, "set1_en = %d, \r\n", iq_param[iq_proc_id]->pfr->set1_en);
	PRINT_IQ_INFO(sfile, "set1_color_u = %d, \r\n", iq_param[iq_proc_id]->pfr->set1_color_u);
	PRINT_IQ_INFO(sfile, "set1_color_v = %d, \r\n", iq_param[iq_proc_id]->pfr->set1_color_v);
	PRINT_IQ_INFO(sfile, "set2_en = %d, \r\n", iq_param[iq_proc_id]->pfr->set2_en);
	PRINT_IQ_INFO(sfile, "set2_color_u = %d, \r\n", iq_param[iq_proc_id]->pfr->set2_color_u);
	PRINT_IQ_INFO(sfile, "set2_color_v = %d, \r\n", iq_param[iq_proc_id]->pfr->set2_color_v);
	PRINT_IQ_INFO(sfile, "set3_en = %d, \r\n", iq_param[iq_proc_id]->pfr->set3_en);
	PRINT_IQ_INFO(sfile, "set3_color_u = %d, \r\n", iq_param[iq_proc_id]->pfr->set3_color_u);
	PRINT_IQ_INFO(sfile, "set3_color_v = %d, \r\n", iq_param[iq_proc_id]->pfr->set3_color_v);
	pfr_m = &iq_param[iq_proc_id]->pfr->manual_param;
	PRINT_IQ_INFO(sfile, "manual_param.pfr_strength = %d, \r\n", pfr_m->pfr_strength);
	PRINT_IQ_INFO(sfile, "manual_param.luma_th = %d, \r\n", pfr_m->luma_th);
	pfr_a = &iq_param[iq_proc_id]->pfr->auto_param[iq_proc_iso];
	PRINT_IQ_INFO(sfile, "auto_param[%d].pfr_strength = %d, \r\n", iq_proc_iso, pfr_a->pfr_strength);
	PRINT_IQ_INFO(sfile, "auto_param[%d].luma_th = %d, \r\n", iq_proc_iso, pfr_a->luma_th);

	// iq_wdr_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_wdr_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "enable = %d, \r\n", iq_param[iq_proc_id]->wdr->enable);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->wdr->mode);
	PRINT_IQ_INFO(sfile, "subimg_size_h = %d, \r\n", iq_param[iq_proc_id]->wdr->subimg_size_h);
	PRINT_IQ_INFO(sfile, "subimg_size_v = %d, \r\n", iq_param[iq_proc_id]->wdr->subimg_size_v);
	PRINT_IQ_INFO(sfile, "max_gain = %d, \r\n", iq_param[iq_proc_id]->wdr->max_gain);
	PRINT_IQ_INFO(sfile, "min_gain = %d, \r\n", iq_param[iq_proc_id]->wdr->min_gain);
	PRINT_IQ_INFO(sfile, "halo_ratio = %d, \r\n", iq_param[iq_proc_id]->wdr->halo_ratio);
	PRINT_IQ_INFO(sfile, "halo_slope = %d, \r\n", iq_param[iq_proc_id]->wdr->halo_slope);
	PRINT_IQ_INFO(sfile, "fbc_ratio = %d, \r\n", iq_param[iq_proc_id]->wdr->fbc_ratio);
	PRINT_IQ_INFO(sfile, "lut_left = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->wdr->lut_left[0], iq_param[iq_proc_id]->wdr->lut_left[1]
		, iq_param[iq_proc_id]->wdr->lut_left[2], iq_param[iq_proc_id]->wdr->lut_left[3]
		, iq_param[iq_proc_id]->wdr->lut_left[4]);
	PRINT_IQ_INFO(sfile, "lut_right = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->wdr->lut_right[0], iq_param[iq_proc_id]->wdr->lut_right[1]
		, iq_param[iq_proc_id]->wdr->lut_right[2], iq_param[iq_proc_id]->wdr->lut_right[3]
		, iq_param[iq_proc_id]->wdr->lut_right[4]);
	wdr_m = &iq_param[iq_proc_id]->wdr->manual_param;
	PRINT_IQ_INFO(sfile, "manual_param.strength = %d, \r\n", wdr_m->strength);
	wdr_a = &iq_param[iq_proc_id]->wdr->auto_param[iq_proc_iso];
	PRINT_IQ_INFO(sfile, "auto_param[%d].strength = %d, \r\n", iq_proc_iso, wdr_a->level);
	PRINT_IQ_INFO(sfile, "auto_param[%d].strength_min = %d, \r\n", iq_proc_iso, wdr_a->strength_min);
	PRINT_IQ_INFO(sfile, "auto_param[%d].strength_max = %d, \r\n", iq_proc_iso, wdr_a->strength_max);

	// iq_wdr_enh_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_wdr_enh_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "enable = %d, \r\n", iq_param[iq_proc_id]->wdr_enh->enable);
	PRINT_IQ_INFO(sfile, "enh_ratio = %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d \r\n"
		, iq_param[iq_proc_id]->wdr_enh->enh_ratio[0], iq_param[iq_proc_id]->wdr_enh->enh_ratio[1]
		, iq_param[iq_proc_id]->wdr_enh->enh_ratio[2], iq_param[iq_proc_id]->wdr_enh->enh_ratio[3]
		, iq_param[iq_proc_id]->wdr_enh->enh_ratio[4], iq_param[iq_proc_id]->wdr_enh->enh_ratio[5]
		, iq_param[iq_proc_id]->wdr_enh->enh_ratio[6], iq_param[iq_proc_id]->wdr_enh->enh_ratio[7]
		, iq_param[iq_proc_id]->wdr_enh->enh_ratio[8], iq_param[iq_proc_id]->wdr_enh->enh_ratio[9]
		, iq_param[iq_proc_id]->wdr_enh->enh_ratio[10], iq_param[iq_proc_id]->wdr_enh->enh_ratio[11]
		, iq_param[iq_proc_id]->wdr_enh->enh_ratio[12], iq_param[iq_proc_id]->wdr_enh->enh_ratio[13]
		, iq_param[iq_proc_id]->wdr_enh->enh_ratio[14], iq_param[iq_proc_id]->wdr_enh->enh_ratio[15]
		, iq_param[iq_proc_id]->wdr_enh->enh_ratio[16]);

	// iq_defog_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_defog_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "enable = %d, \r\n", iq_param[iq_proc_id]->defog->enable);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->defog->mode);
	PRINT_IQ_INFO(sfile, "outbld_local_en = %d, \r\n", iq_param[iq_proc_id]->defog->outbld_local_en);
	PRINT_IQ_INFO(sfile, "outbld_local_en = %d, \r\n", iq_param[iq_proc_id]->defog->outbld_local_en);
	PRINT_IQ_INFO(sfile, "min_diff_ratio = %d, \r\n", iq_param[iq_proc_id]->defog->min_diff_ratio);
	defog_m = &iq_param[iq_proc_id]->defog->manual_param;
	PRINT_IQ_INFO(sfile, "manual_param.fog_mod_level = %d, \r\n", defog_m->fog_level);
	PRINT_IQ_INFO(sfile, "manual_param.fog_ratio = %d, \r\n", defog_m->fog_ratio);
	PRINT_IQ_INFO(sfile, "manual_param.gain_th = %d, \r\n", defog_m->gain_th);
	PRINT_IQ_INFO(sfile, "manual_param.outbld_lum_wt = %d, %d, %d, %d, %d, \r\n"
		, defog_m->outbld_lum_wt[0], defog_m->outbld_lum_wt[1], defog_m->outbld_lum_wt[2], defog_m->outbld_lum_wt[3], defog_m->outbld_lum_wt[4]);
	defog_a = &iq_param[iq_proc_id]->defog->auto_param[iq_proc_iso];
	PRINT_IQ_INFO(sfile, "auto_param[%d].dr_th = %d, \r\n", iq_proc_iso, defog_a->dr_th);
	PRINT_IQ_INFO(sfile, "auto_param[%d].fog_mod_level_lb = %d, \r\n", iq_proc_iso, defog_a->fog_level_max);
	PRINT_IQ_INFO(sfile, "auto_param[%d].fog_ratio = %d, \r\n", iq_proc_iso, defog_a->fog_ratio);
	PRINT_IQ_INFO(sfile, "auto_param[%d].outbld_lum_wt_str = %d, \r\n", iq_proc_iso, defog_a->outbld_wt);

	// iq_shdr_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_shdr_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "nrs_enable = %d, \r\n", iq_param[iq_proc_id]->shdr->nrs_enable);
	PRINT_IQ_INFO(sfile, "auto_ev_enable = %d, \r\n", iq_param[iq_proc_id]->shdr->auto_ev_enable);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->shdr->mode);
	PRINT_IQ_INFO(sfile, "fusion_nor_sel = %d, \r\n", iq_param[iq_proc_id]->shdr->fusion_nor_sel);
	PRINT_IQ_INFO(sfile, "fusion_l_nor_knee = %d, \r\n", iq_param[iq_proc_id]->shdr->fusion_l_nor_knee);
	PRINT_IQ_INFO(sfile, "fusion_l_nor_range = %d, \r\n", iq_param[iq_proc_id]->shdr->fusion_l_nor_range);
	PRINT_IQ_INFO(sfile, "fusion_s_nor_knee = %d, \r\n", iq_param[iq_proc_id]->shdr->fusion_s_nor_knee);
	PRINT_IQ_INFO(sfile, "fusion_s_nor_range = %d, \r\n", iq_param[iq_proc_id]->shdr->fusion_s_nor_range);
	PRINT_IQ_INFO(sfile, "fusion_dif_sel = %d, \r\n", iq_param[iq_proc_id]->shdr->fusion_dif_sel);
	PRINT_IQ_INFO(sfile, "fusion_l_dif_knee = %d, \r\n", iq_param[iq_proc_id]->shdr->fusion_l_dif_knee);
	PRINT_IQ_INFO(sfile, "fusion_l_dif_range = %d, \r\n", iq_param[iq_proc_id]->shdr->fusion_l_dif_range);
	PRINT_IQ_INFO(sfile, "fusion_s_dif_knee = %d, \r\n", iq_param[iq_proc_id]->shdr->fusion_s_dif_knee);
	PRINT_IQ_INFO(sfile, "fusion_s_dif_range = %d, \r\n", iq_param[iq_proc_id]->shdr->fusion_s_dif_range);
	PRINT_IQ_INFO(sfile, "fusion_lum_th = %d, \r\n", iq_param[iq_proc_id]->shdr->fusion_lum_th);
	PRINT_IQ_INFO(sfile, "fusion_diff_w = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->shdr->fusion_diff_w[0], iq_param[iq_proc_id]->shdr->fusion_diff_w[1]
		, iq_param[iq_proc_id]->shdr->fusion_diff_w[2], iq_param[iq_proc_id]->shdr->fusion_diff_w[3]
		, iq_param[iq_proc_id]->shdr->fusion_diff_w[4]);
	PRINT_IQ_INFO(sfile, "fcurve_y_mean_sel = %d, \r\n", iq_param[iq_proc_id]->shdr->fcurve_y_mean_sel);
	PRINT_IQ_INFO(sfile, "fcurve_yv_w = %d, \r\n", iq_param[iq_proc_id]->shdr->fcurve_yv_w);
	PRINT_IQ_INFO(sfile, "fcurve_y_w_lut = %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->shdr->fcurve_y_w_lut[0], iq_param[iq_proc_id]->shdr->fcurve_y_w_lut[1]
		, iq_param[iq_proc_id]->shdr->fcurve_y_w_lut[2], iq_param[iq_proc_id]->shdr->fcurve_y_w_lut[3]
		, iq_param[iq_proc_id]->shdr->fcurve_y_w_lut[4], iq_param[iq_proc_id]->shdr->fcurve_y_w_lut[5]
		, iq_param[iq_proc_id]->shdr->fcurve_y_w_lut[6], iq_param[iq_proc_id]->shdr->fcurve_y_w_lut[7]
		, iq_param[iq_proc_id]->shdr->fcurve_y_w_lut[8], iq_param[iq_proc_id]->shdr->fcurve_y_w_lut[9]
		, iq_param[iq_proc_id]->shdr->fcurve_y_w_lut[10], iq_param[iq_proc_id]->shdr->fcurve_y_w_lut[11]
		, iq_param[iq_proc_id]->shdr->fcurve_y_w_lut[12], iq_param[iq_proc_id]->shdr->fcurve_y_w_lut[13]
		, iq_param[iq_proc_id]->shdr->fcurve_y_w_lut[14], iq_param[iq_proc_id]->shdr->fcurve_y_w_lut[15]
		, iq_param[iq_proc_id]->shdr->fcurve_y_w_lut[16]);
	PRINT_IQ_INFO(sfile, "fcurve_left_lut = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->shdr->fcurve_left_lut[0], iq_param[iq_proc_id]->shdr->fcurve_left_lut[1]
		, iq_param[iq_proc_id]->shdr->fcurve_left_lut[2], iq_param[iq_proc_id]->shdr->fcurve_left_lut[3]
		, iq_param[iq_proc_id]->shdr->fcurve_left_lut[4]);
	PRINT_IQ_INFO(sfile, "fcurve_right_lut = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->shdr->fcurve_right_lut[0], iq_param[iq_proc_id]->shdr->fcurve_right_lut[1]
		, iq_param[iq_proc_id]->shdr->fcurve_right_lut[2], iq_param[iq_proc_id]->shdr->fcurve_right_lut[3]
		, iq_param[iq_proc_id]->shdr->fcurve_right_lut[4]);
	shdr_m = &iq_param[iq_proc_id]->shdr->manual_param;
	PRINT_IQ_INFO(sfile, "manual_param.nrs_s_str = %d, %d, %d, %d, %d, %d, \r\n"
		, shdr_m->nrs_s_str[0], shdr_m->nrs_s_str[1], shdr_m->nrs_s_str[2]
		, shdr_m->nrs_s_str[3], shdr_m->nrs_s_str[4], shdr_m->nrs_s_str[5]);
	shdr_a = &iq_param[iq_proc_id]->shdr->auto_param[iq_proc_iso];
	PRINT_IQ_INFO(sfile, "auto_param[%d].nrs_s_str = %d, %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, shdr_a->nrs_s_str[0], shdr_a->nrs_s_str[1], shdr_a->nrs_s_str[2]
		, shdr_a->nrs_s_str[3], shdr_a->nrs_s_str[4], shdr_a->nrs_s_str[5]);

	// iq_companding_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_companding_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "decomp_kpx = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->companding->decomp_kpx[0], iq_param[iq_proc_id]->companding->decomp_kpx[1]
		, iq_param[iq_proc_id]->companding->decomp_kpx[2], iq_param[iq_proc_id]->companding->decomp_kpx[3]
		, iq_param[iq_proc_id]->companding->decomp_kpx[4]);
	PRINT_IQ_INFO(sfile, "decomp_kpy = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->companding->decomp_kpy[0], iq_param[iq_proc_id]->companding->decomp_kpy[1]
		, iq_param[iq_proc_id]->companding->decomp_kpy[2], iq_param[iq_proc_id]->companding->decomp_kpy[3]
		, iq_param[iq_proc_id]->companding->decomp_kpy[4]);
	PRINT_IQ_INFO(sfile, "decomp_gain = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->companding->decomp_gain[0], iq_param[iq_proc_id]->companding->decomp_gain[1]
		, iq_param[iq_proc_id]->companding->decomp_gain[2], iq_param[iq_proc_id]->companding->decomp_gain[3]
		, iq_param[iq_proc_id]->companding->decomp_gain[4]);
	PRINT_IQ_INFO(sfile, "decomp_sb = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->companding->decomp_sb[0], iq_param[iq_proc_id]->companding->decomp_sb[1]
		, iq_param[iq_proc_id]->companding->decomp_sb[2], iq_param[iq_proc_id]->companding->decomp_sb[3]
		, iq_param[iq_proc_id]->companding->decomp_sb[4]);
	PRINT_IQ_INFO(sfile, "comp_fcurve_l = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->companding->comp_fcurve_l[0], iq_param[iq_proc_id]->companding->comp_fcurve_l[1]
		, iq_param[iq_proc_id]->companding->comp_fcurve_l[2], iq_param[iq_proc_id]->companding->comp_fcurve_l[3]
		, iq_param[iq_proc_id]->companding->comp_fcurve_l[4]);
	PRINT_IQ_INFO(sfile, "comp_fcurve_m = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->companding->comp_fcurve_m[0], iq_param[iq_proc_id]->companding->comp_fcurve_m[1]
		, iq_param[iq_proc_id]->companding->comp_fcurve_m[2], iq_param[iq_proc_id]->companding->comp_fcurve_m[3]
		, iq_param[iq_proc_id]->companding->comp_fcurve_m[4]);
	PRINT_IQ_INFO(sfile, "comp_fcurve_r = %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->companding->comp_fcurve_r[0], iq_param[iq_proc_id]->companding->comp_fcurve_r[1]
		, iq_param[iq_proc_id]->companding->comp_fcurve_r[2], iq_param[iq_proc_id]->companding->comp_fcurve_r[3]
		, iq_param[iq_proc_id]->companding->comp_fcurve_r[4]);
	PRINT_IQ_INFO(sfile, "comp_fcurve_ev_fmt = %d, \r\n", iq_param[iq_proc_id]->companding->comp_fcurve_ev_fmt);

	// iq_rgbir_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_rgbir_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->rgbir->mode);
	rgbir_m = &iq_param[iq_proc_id]->rgbir->manual_param;
	PRINT_IQ_INFO(sfile, "manual_param.irsub_r_weight = %d, \r\n", rgbir_m->irsub_r_weight);
	PRINT_IQ_INFO(sfile, "manual_param.irsub_g_weight = %d, \r\n", rgbir_m->irsub_g_weight);
	PRINT_IQ_INFO(sfile, "manual_param.irsub_b_weight = %d, \r\n", rgbir_m->irsub_b_weight);
	PRINT_IQ_INFO(sfile, "manual_param.ir_sat_gain = %d, \r\n", rgbir_m->ir_sat_gain);
	rgbir_a = &iq_param[iq_proc_id]->rgbir->auto_param;
	PRINT_IQ_INFO(sfile, "auto_param.irsub_r_weight = %d, \r\n", rgbir_a->irsub_r_weight);
	PRINT_IQ_INFO(sfile, "auto_param.irsub_g_weight = %d, \r\n", rgbir_a->irsub_g_weight);
	PRINT_IQ_INFO(sfile, "auto_param.irsub_b_weight = %d, \r\n", rgbir_a->irsub_b_weight);
	PRINT_IQ_INFO(sfile, "auto_param.irsub_reduce_th = %d, \r\n", rgbir_a->irsub_reduce_th);
	PRINT_IQ_INFO(sfile, "auto_param.night_mode_th = %d, \r\n", rgbir_a->night_mode_th);
	
	// iq_rgbir_enh_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_rgbir_enh_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "enable = %d, \r\n", iq_param[iq_proc_id]->rgbir_enh->enable);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->rgbir_enh->mode);
	PRINT_IQ_INFO(sfile, "min_ir_th = %d, \r\n", iq_param[iq_proc_id]->rgbir_enh->min_ir_th);
	rgbir_enh_m = &iq_param[iq_proc_id]->rgbir_enh->manual_param;
	PRINT_IQ_INFO(sfile, "manual_param.enh_ratio = %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, \r\n"
		, rgbir_enh_m->enh_ratio[0], rgbir_enh_m->enh_ratio[1], rgbir_enh_m->enh_ratio[2], rgbir_enh_m->enh_ratio[3]
		, rgbir_enh_m->enh_ratio[4], rgbir_enh_m->enh_ratio[5], rgbir_enh_m->enh_ratio[6], rgbir_enh_m->enh_ratio[7]
		, rgbir_enh_m->enh_ratio[8], rgbir_enh_m->enh_ratio[9], rgbir_enh_m->enh_ratio[10], rgbir_enh_m->enh_ratio[11]
		, rgbir_enh_m->enh_ratio[12], rgbir_enh_m->enh_ratio[13], rgbir_enh_m->enh_ratio[14], rgbir_enh_m->enh_ratio[15]);
	PRINT_IQ_INFO(sfile, "manual_param.outl_rgbir_rb_w = %d, \r\n", rgbir_enh_m->outl_rgbir_rb_w);
	PRINT_IQ_INFO(sfile, "manual_param.outl_ord_rgbir_rb_w = %d, \r\n", rgbir_enh_m->outl_ord_rgbir_rb_w);
	if (iq_proc_iso < IQ_RGBIR_LIGHT_ID_MAX_NUM) {
		rgbir_enh_a = &iq_param[iq_proc_id]->rgbir_enh->auto_param[iq_proc_iso];
		PRINT_IQ_INFO(sfile, "auto_param[%d].ir_th = %d, \r\n", iq_proc_iso, rgbir_enh_a->ir_th);
		PRINT_IQ_INFO(sfile, "auto_param[%d].enh_ratio = %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, \r\n", iq_proc_iso
			, rgbir_enh_a->enh_ratio[0], rgbir_enh_a->enh_ratio[1], rgbir_enh_a->enh_ratio[2], rgbir_enh_a->enh_ratio[3]
			, rgbir_enh_a->enh_ratio[4], rgbir_enh_a->enh_ratio[5], rgbir_enh_a->enh_ratio[6], rgbir_enh_a->enh_ratio[7]
			, rgbir_enh_a->enh_ratio[8], rgbir_enh_a->enh_ratio[9], rgbir_enh_a->enh_ratio[10], rgbir_enh_a->enh_ratio[11]
			, rgbir_enh_a->enh_ratio[12], rgbir_enh_a->enh_ratio[13], rgbir_enh_a->enh_ratio[14], rgbir_enh_a->enh_ratio[15]);
		PRINT_IQ_INFO(sfile, "auto_param[%d].outl_rgbir_rb_w = %d, \r\n", iq_proc_iso, rgbir_enh_a->outl_rgbir_rb_w);
		PRINT_IQ_INFO(sfile, "auto_param[%d].outl_ord_rgbir_rb_w = %d, \r\n", iq_proc_iso, rgbir_enh_a->outl_ord_rgbir_rb_w);
	}

	// iq_post_sharpen_1_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_post_sharpen_1_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "enable = %d, \r\n", iq_param[iq_proc_id]->post_sharpen_1->enable);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->post_sharpen_1->mode);
	post_sharpen_1_m = &iq_param[iq_proc_id]->post_sharpen_1->manual_param;
	PRINT_IQ_INFO(sfile, "manual_param.noise_level = %d, \r\n", post_sharpen_1_m->noise_level);
	PRINT_IQ_INFO(sfile, "manual_param.noise_curve = %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, \r\n"
		, post_sharpen_1_m->noise_curve[0], post_sharpen_1_m->noise_curve[1], post_sharpen_1_m->noise_curve[2], post_sharpen_1_m->noise_curve[3]
		, post_sharpen_1_m->noise_curve[4], post_sharpen_1_m->noise_curve[5], post_sharpen_1_m->noise_curve[6], post_sharpen_1_m->noise_curve[7]
		, post_sharpen_1_m->noise_curve[8], post_sharpen_1_m->noise_curve[9], post_sharpen_1_m->noise_curve[10], post_sharpen_1_m->noise_curve[11]
		, post_sharpen_1_m->noise_curve[12], post_sharpen_1_m->noise_curve[13], post_sharpen_1_m->noise_curve[14], post_sharpen_1_m->noise_curve[15]
		, post_sharpen_1_m->noise_curve[16]);
	PRINT_IQ_INFO(sfile, "manual_param.edge_weight_th = %d, \r\n", post_sharpen_1_m->edge_weight_th);
	PRINT_IQ_INFO(sfile, "manual_param.edge_weight_gain = %d, \r\n", post_sharpen_1_m->edge_weight_gain);
	PRINT_IQ_INFO(sfile, "manual_param.th_flat = %d, \r\n", post_sharpen_1_m->th_flat);
	PRINT_IQ_INFO(sfile, "manual_param.th_edge = %d, \r\n", post_sharpen_1_m->th_edge);
	PRINT_IQ_INFO(sfile, "manual_param.flat_region_str = %d, \r\n", post_sharpen_1_m->flat_region_str);
	PRINT_IQ_INFO(sfile, "manual_param.edge_region_str = %d, \r\n", post_sharpen_1_m->edge_region_str);
	PRINT_IQ_INFO(sfile, "manual_param.motion_edge_w_str = %d, \r\n", post_sharpen_1_m->motion_edge_w_str);
	PRINT_IQ_INFO(sfile, "manual_param.tarnsition_edge_w_str = %d, \r\n", post_sharpen_1_m->tarnsition_edge_w_str);
	PRINT_IQ_INFO(sfile, "manual_param.static_edge_w_str = %d, \r\n", post_sharpen_1_m->static_edge_w_str);
	PRINT_IQ_INFO(sfile, "manual_param.coring_th = %d, \r\n", post_sharpen_1_m->coring_th);
	PRINT_IQ_INFO(sfile, "manual_param.blend_inv_gamma = %d, \r\n", post_sharpen_1_m->blend_inv_gamma);
	PRINT_IQ_INFO(sfile, "manual_param.edge_filt_sel = %d, \r\n", post_sharpen_1_m->edge_filt_sel);
	PRINT_IQ_INFO(sfile, "manual_param.sharp_str = %d, \r\n", post_sharpen_1_m->sharp_str);
	PRINT_IQ_INFO(sfile, "manual_param.bright_halo_clip = %d, \r\n", post_sharpen_1_m->bright_halo_clip);
	PRINT_IQ_INFO(sfile, "manual_param.dark_halo_clip = %d, \r\n", post_sharpen_1_m->dark_halo_clip);
	post_sharpen_1_a = &iq_param[iq_proc_id]->post_sharpen_1->auto_param[iq_proc_iso];
	PRINT_IQ_INFO(sfile, "auto_param[%d].noise_level = %d, \r\n", iq_proc_iso, post_sharpen_1_a->noise_level);
	PRINT_IQ_INFO(sfile, "auto_param[%d].noise_curve = %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, post_sharpen_1_a->noise_curve[0], post_sharpen_1_a->noise_curve[1], post_sharpen_1_a->noise_curve[2], post_sharpen_1_a->noise_curve[3]
		, post_sharpen_1_a->noise_curve[4], post_sharpen_1_a->noise_curve[5], post_sharpen_1_a->noise_curve[6], post_sharpen_1_a->noise_curve[7]
		, post_sharpen_1_a->noise_curve[8], post_sharpen_1_a->noise_curve[9], post_sharpen_1_a->noise_curve[10], post_sharpen_1_a->noise_curve[11]
		, post_sharpen_1_a->noise_curve[12], post_sharpen_1_a->noise_curve[13], post_sharpen_1_a->noise_curve[14], post_sharpen_1_a->noise_curve[15]
		, post_sharpen_1_a->noise_curve[16]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].edge_weight_th = %d, \r\n", iq_proc_iso, post_sharpen_1_a->edge_weight_th);
	PRINT_IQ_INFO(sfile, "auto_param[%d].edge_weight_gain = %d, \r\n", iq_proc_iso, post_sharpen_1_a->edge_weight_gain);
	PRINT_IQ_INFO(sfile, "auto_param[%d].th_flat = %d, \r\n", iq_proc_iso, post_sharpen_1_a->th_flat);
	PRINT_IQ_INFO(sfile, "auto_param[%d].th_edge = %d, \r\n", iq_proc_iso, post_sharpen_1_a->th_edge);
	PRINT_IQ_INFO(sfile, "auto_param[%d].flat_region_str = %d, \r\n", iq_proc_iso, post_sharpen_1_a->flat_region_str);
	PRINT_IQ_INFO(sfile, "auto_param[%d].edge_region_str = %d, \r\n", iq_proc_iso, post_sharpen_1_a->edge_region_str);
	PRINT_IQ_INFO(sfile, "auto_param[%d].motion_edge_w_str = %d, \r\n", iq_proc_iso, post_sharpen_1_a->motion_edge_w_str);
	PRINT_IQ_INFO(sfile, "auto_param[%d].tarnsition_edge_w_str = %d, \r\n", iq_proc_iso, post_sharpen_1_a->tarnsition_edge_w_str);
	PRINT_IQ_INFO(sfile, "auto_param[%d].static_edge_w_str = %d, \r\n", iq_proc_iso, post_sharpen_1_a->static_edge_w_str);
	PRINT_IQ_INFO(sfile, "auto_param[%d].coring_th = %d, \r\n", iq_proc_iso, post_sharpen_1_a->coring_th);
	PRINT_IQ_INFO(sfile, "auto_param[%d].blend_inv_gamma = %d, \r\n", iq_proc_iso, post_sharpen_1_a->blend_inv_gamma);
	PRINT_IQ_INFO(sfile, "auto_param[%d].edge_filt_sel = %d, \r\n", iq_proc_iso, post_sharpen_1_a->edge_filt_sel);
	PRINT_IQ_INFO(sfile, "auto_param[%d].sharp_str = %d, \r\n", iq_proc_iso, post_sharpen_1_a->sharp_str);
	PRINT_IQ_INFO(sfile, "auto_param[%d].bright_halo_clip = %d, \r\n", iq_proc_iso, post_sharpen_1_a->bright_halo_clip);
	PRINT_IQ_INFO(sfile, "auto_param[%d].dark_halo_clip = %d, \r\n", iq_proc_iso, post_sharpen_1_a->dark_halo_clip);

	// iq_post_sharpen_2_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_post_sharpen_2_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "enable = %d, \r\n", iq_param[iq_proc_id]->post_sharpen_2->enable);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->post_sharpen_2->mode);
	post_sharpen_2_m = &iq_param[iq_proc_id]->post_sharpen_2->manual_param;
	PRINT_IQ_INFO(sfile, "manual_param.noise_level = %d, \r\n", post_sharpen_2_m->noise_level);
	PRINT_IQ_INFO(sfile, "manual_param.noise_curve = %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, \r\n"
		, post_sharpen_2_m->noise_curve[0], post_sharpen_2_m->noise_curve[1], post_sharpen_2_m->noise_curve[2], post_sharpen_2_m->noise_curve[3]
		, post_sharpen_2_m->noise_curve[4], post_sharpen_2_m->noise_curve[5], post_sharpen_2_m->noise_curve[6], post_sharpen_2_m->noise_curve[7]
		, post_sharpen_2_m->noise_curve[8], post_sharpen_2_m->noise_curve[9], post_sharpen_2_m->noise_curve[10], post_sharpen_2_m->noise_curve[11]
		, post_sharpen_2_m->noise_curve[12], post_sharpen_2_m->noise_curve[13], post_sharpen_2_m->noise_curve[14], post_sharpen_2_m->noise_curve[15]
		, post_sharpen_2_m->noise_curve[16]);
	PRINT_IQ_INFO(sfile, "manual_param.edge_weight_th = %d, \r\n", post_sharpen_2_m->edge_weight_th);
	PRINT_IQ_INFO(sfile, "manual_param.edge_weight_gain = %d, \r\n", post_sharpen_2_m->edge_weight_gain);
	PRINT_IQ_INFO(sfile, "manual_param.th_flat = %d, \r\n", post_sharpen_2_m->th_flat);
	PRINT_IQ_INFO(sfile, "manual_param.th_edge = %d, \r\n", post_sharpen_2_m->th_edge);
	PRINT_IQ_INFO(sfile, "manual_param.flat_region_str = %d, \r\n", post_sharpen_2_m->flat_region_str);
	PRINT_IQ_INFO(sfile, "manual_param.edge_region_str = %d, \r\n", post_sharpen_2_m->edge_region_str);
	PRINT_IQ_INFO(sfile, "manual_param.motion_edge_w_str = %d, \r\n", post_sharpen_2_m->motion_edge_w_str);
	PRINT_IQ_INFO(sfile, "manual_param.tarnsition_edge_w_str = %d, \r\n", post_sharpen_2_m->tarnsition_edge_w_str);
	PRINT_IQ_INFO(sfile, "manual_param.static_edge_w_str = %d, \r\n", post_sharpen_2_m->static_edge_w_str);
	PRINT_IQ_INFO(sfile, "manual_param.coring_th = %d, \r\n", post_sharpen_2_m->coring_th);
	PRINT_IQ_INFO(sfile, "manual_param.blend_inv_gamma = %d, \r\n", post_sharpen_2_m->blend_inv_gamma);
	PRINT_IQ_INFO(sfile, "manual_param.sharp_str = %d, \r\n", post_sharpen_2_m->sharp_str);
	PRINT_IQ_INFO(sfile, "manual_param.bright_halo_clip = %d, \r\n", post_sharpen_2_m->bright_halo_clip);
	PRINT_IQ_INFO(sfile, "manual_param.dark_halo_clip = %d, \r\n", post_sharpen_2_m->dark_halo_clip);
	post_sharpen_2_a = &iq_param[iq_proc_id]->post_sharpen_2->auto_param[iq_proc_iso];
	PRINT_IQ_INFO(sfile, "auto_param[%d].noise_level = %d, \r\n", iq_proc_iso, post_sharpen_2_a->noise_level);
	PRINT_IQ_INFO(sfile, "auto_param[%d].noise_curve = %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, \r\n", iq_proc_iso
		, post_sharpen_2_a->noise_curve[0], post_sharpen_2_a->noise_curve[1], post_sharpen_2_a->noise_curve[2], post_sharpen_2_a->noise_curve[3]
		, post_sharpen_2_a->noise_curve[4], post_sharpen_2_a->noise_curve[5], post_sharpen_2_a->noise_curve[6], post_sharpen_2_a->noise_curve[7]
		, post_sharpen_2_a->noise_curve[8], post_sharpen_2_a->noise_curve[9], post_sharpen_2_a->noise_curve[10], post_sharpen_2_a->noise_curve[11]
		, post_sharpen_2_a->noise_curve[12], post_sharpen_2_a->noise_curve[13], post_sharpen_2_a->noise_curve[14], post_sharpen_2_a->noise_curve[15]
		, post_sharpen_2_a->noise_curve[16]);
	PRINT_IQ_INFO(sfile, "auto_param[%d].edge_weight_th = %d, \r\n", iq_proc_iso, post_sharpen_2_a->edge_weight_th);
	PRINT_IQ_INFO(sfile, "auto_param[%d].edge_weight_gain = %d, \r\n", iq_proc_iso, post_sharpen_2_a->edge_weight_gain);
	PRINT_IQ_INFO(sfile, "auto_param[%d].th_flat = %d, \r\n", iq_proc_iso, post_sharpen_2_a->th_flat);
	PRINT_IQ_INFO(sfile, "auto_param[%d].th_edge = %d, \r\n", iq_proc_iso, post_sharpen_2_a->th_edge);
	PRINT_IQ_INFO(sfile, "auto_param[%d].flat_region_str = %d, \r\n", iq_proc_iso, post_sharpen_2_a->flat_region_str);
	PRINT_IQ_INFO(sfile, "auto_param[%d].edge_region_str = %d, \r\n", iq_proc_iso, post_sharpen_2_a->edge_region_str);
	PRINT_IQ_INFO(sfile, "auto_param[%d].motion_edge_w_str = %d, \r\n", iq_proc_iso, post_sharpen_2_a->motion_edge_w_str);
	PRINT_IQ_INFO(sfile, "auto_param[%d].tarnsition_edge_w_str = %d, \r\n", iq_proc_iso, post_sharpen_2_a->tarnsition_edge_w_str);
	PRINT_IQ_INFO(sfile, "auto_param[%d].static_edge_w_str = %d, \r\n", iq_proc_iso, post_sharpen_2_a->static_edge_w_str);
	PRINT_IQ_INFO(sfile, "auto_param[%d].coring_th = %d, \r\n", iq_proc_iso, post_sharpen_2_a->coring_th);
	PRINT_IQ_INFO(sfile, "auto_param[%d].blend_inv_gamma = %d, \r\n", iq_proc_iso, post_sharpen_2_a->blend_inv_gamma);
	PRINT_IQ_INFO(sfile, "auto_param[%d].sharp_str = %d, \r\n", iq_proc_iso, post_sharpen_2_a->sharp_str);
	PRINT_IQ_INFO(sfile, "auto_param[%d].bright_halo_clip = %d, \r\n", iq_proc_iso, post_sharpen_2_a->bright_halo_clip);
	PRINT_IQ_INFO(sfile, "auto_param[%d].dark_halo_clip = %d, \r\n", iq_proc_iso, post_sharpen_2_a->dark_halo_clip);

	// iq_ycurve_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_ycurve_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "enable = %d, \r\n", iq_param[iq_proc_id]->ycurve->enable);
	PRINT_IQ_INFO(sfile, "ycurve_lut = %d, %d, %d, %d, %d, %d, %d, %d, %d, %d,,,,,,, \r\n"
		, iq_param[iq_proc_id]->ycurve->ycurve_lut[0], iq_param[iq_proc_id]->ycurve->ycurve_lut[1]
		, iq_param[iq_proc_id]->ycurve->ycurve_lut[2], iq_param[iq_proc_id]->ycurve->ycurve_lut[3]
		, iq_param[iq_proc_id]->ycurve->ycurve_lut[4], iq_param[iq_proc_id]->ycurve->ycurve_lut[5]
		, iq_param[iq_proc_id]->ycurve->ycurve_lut[6], iq_param[iq_proc_id]->ycurve->ycurve_lut[7]
		, iq_param[iq_proc_id]->ycurve->ycurve_lut[8], iq_param[iq_proc_id]->ycurve->ycurve_lut[9]);

	// iq_cst_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_cst_param==================== \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "mode = %d, \r\n", iq_param[iq_proc_id]->cst->mode);
	PRINT_IQ_INFO(sfile, "cst_coef = %d, %d, %d, %d, %d, %d, %d, %d, %d, \r\n"
		, iq_param[iq_proc_id]->cst->cst_coef[0], iq_param[iq_proc_id]->cst->cst_coef[1], iq_param[iq_proc_id]->cst->cst_coef[2]
		, iq_param[iq_proc_id]->cst->cst_coef[3], iq_param[iq_proc_id]->cst->cst_coef[4], iq_param[iq_proc_id]->cst->cst_coef[5]
		, iq_param[iq_proc_id]->cst->cst_coef[6], iq_param[iq_proc_id]->cst->cst_coef[7], iq_param[iq_proc_id]->cst->cst_coef[8]);
	PRINT_IQ_INFO(sfile, "y_ofs = %d, \r\n", iq_param[iq_proc_id]->cst->y_ofs);
	PRINT_IQ_INFO(sfile, "cb_ofs = %d, \r\n", iq_param[iq_proc_id]->cst->cb_ofs);
	PRINT_IQ_INFO(sfile, "cr_ofs = %d, \r\n", iq_param[iq_proc_id]->cst->cr_ofs);
	cst_m = &iq_param[iq_proc_id]->cst->manual_param;
	PRINT_IQ_INFO(sfile, "manual_param.cstp_ratio = %d, \r\n", cst_m->cstp_ratio);
	if (iq_proc_iso < IQ_GAIN_ID_MAX_NUM) {
		cst_a = &iq_param[iq_proc_id]->cst->auto_param[iq_proc_iso];
		PRINT_IQ_INFO(sfile, "auto_param[%d].cstp_ratio = %d, \r\n", iq_proc_iso, cst_a->cstp_ratio);
	}

	// iq_fixth_param
	PRINT_IQ_INFO(sfile, "====================id(%d) iq_fixth_param==================== \r\n", iq_proc_id);
	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(iq_proc_id));
	PRINT_IQ_INFO(sfile, "enable = %d, \r\n", iq_info->iq_ref_set.fixth.enable);
	PRINT_IQ_INFO(sfile, "fixy_sel = %d, \r\n", iq_info->iq_ref_set.fixth.fixy_sel);
	PRINT_IQ_INFO(sfile, "fixy_y_sel = %d, \r\n", iq_info->iq_ref_set.fixth.fixy_y_sel);
	PRINT_IQ_INFO(sfile, "fixy_edge_th = %d, \r\n", iq_info->iq_ref_set.fixth.fixy_edge_th);
	PRINT_IQ_INFO(sfile, "fixy_y_th = %d, \r\n", iq_info->iq_ref_set.fixth.fixy_y_th);
	PRINT_IQ_INFO(sfile, "fixy_y_value = %d, \r\n", iq_info->iq_ref_set.fixth.fixy_y_value);
	PRINT_IQ_INFO(sfile, "fixc_y_th_low = %d, \r\n", iq_info->iq_ref_set.fixth.fixc_y_th_low);
	PRINT_IQ_INFO(sfile, "fixc_y_th_high = %d, \r\n", iq_info->iq_ref_set.fixth.fixc_y_th_high);
	PRINT_IQ_INFO(sfile, "fixc_cb_th_low = %d, \r\n", iq_info->iq_ref_set.fixth.fixc_cb_th_low);
	PRINT_IQ_INFO(sfile, "fixc_cb_th_high = %d, \r\n", iq_info->iq_ref_set.fixth.fixc_cb_th_high);
	PRINT_IQ_INFO(sfile, "fixc_cr_th_low = %d, \r\n", iq_info->iq_ref_set.fixth.fixc_cr_th_low);
	PRINT_IQ_INFO(sfile, "fixc_cr_th_high = %d, \r\n", iq_info->iq_ref_set.fixth.fixc_cr_th_high);
	PRINT_IQ_INFO(sfile, "fixc_cb_value = %d, \r\n", iq_info->iq_ref_set.fixth.fixc_cb_value);
	PRINT_IQ_INFO(sfile, "fixc_cr_value = %d, \r\n", iq_info->iq_ref_set.fixth.fixc_cr_value);
}

#if defined(__FREERTOS)
void iq_msg_show_ui(IQ_ID iq_proc_id)
#else
void iq_msg_show_ui(struct seq_file *sfile, IQ_ID iq_proc_id)
#endif
{
	PRINT_IQ_INFO(sfile, "iq_proc_get_ui_param \r\n");
	PRINT_IQ_INFO(sfile, "id : %d \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "get ui nr_lv : %d \r\n", iq_ui_get_info(iq_proc_id, IQ_UI_ITEM_NR_LV));
	PRINT_IQ_INFO(sfile, "get ui 3dnr_lv : %d \r\n", iq_ui_get_info(iq_proc_id, IQ_UI_ITEM_3DNR_LV));
	PRINT_IQ_INFO(sfile, "get ui sharpness_lv : %d \r\n", iq_ui_get_info(iq_proc_id, IQ_UI_ITEM_SHARPNESS_LV));
	PRINT_IQ_INFO(sfile, "get ui saturation_lv : %d \r\n", iq_ui_get_info(iq_proc_id, IQ_UI_ITEM_SATURATION_LV));
	PRINT_IQ_INFO(sfile, "get ui contrast_lv : %d \r\n", iq_ui_get_info(iq_proc_id, IQ_UI_ITEM_CONTRAST_LV));
	PRINT_IQ_INFO(sfile, "get ui brightness_lv : %d \r\n", iq_ui_get_info(iq_proc_id, IQ_UI_ITEM_BRIGHTNESS_LV));
	PRINT_IQ_INFO(sfile, "get ui night_mode : %d \r\n", iq_ui_get_info(iq_proc_id, IQ_UI_ITEM_NIGHT_MODE));
	PRINT_IQ_INFO(sfile, "get ui ycc_format : %d \r\n", iq_ui_get_info(iq_proc_id, IQ_UI_ITEM_YCC_FORMAT));
	PRINT_IQ_INFO(sfile, "get ui operation : %d \r\n", iq_ui_get_info(iq_proc_id, IQ_UI_ITEM_OPERATION));
	PRINT_IQ_INFO(sfile, "get ui imageeffect : %d \r\n", iq_ui_get_info(iq_proc_id, IQ_UI_ITEM_IMAGEEFFECT));
	PRINT_IQ_INFO(sfile, "get ui ccid : %d \r\n", iq_ui_get_info(iq_proc_id, IQ_UI_ITEM_CCID));
	PRINT_IQ_INFO(sfile, "get ui hue_shift : %d \r\n", iq_ui_get_info(iq_proc_id, IQ_UI_ITEM_HUE_SHIFT));
}

#if defined(__FREERTOS)
void iq_msg_show_nnsc(IQ_ID iq_proc_id)
#else
void iq_msg_show_nnsc(struct seq_file *sfile, IQ_ID iq_proc_id)
#endif
{
	PRINT_IQ_INFO(sfile, "iq_proc_get_nnsc_param \r\n");
	PRINT_IQ_INFO(sfile, "id : %d \r\n", iq_proc_id);
	PRINT_IQ_INFO(sfile, "get nnsc dark_enh_ratio : %d \r\n", iq_nnsc_get_info(iq_proc_id, IQ_NNSC_ITEM_DARK_ENH_RATIO));
	PRINT_IQ_INFO(sfile, "get nnsc contrast_enh_ratio : %d \r\n", iq_nnsc_get_info(iq_proc_id, IQ_NNSC_ITEM_CONTRAST_ENH_RATIO));
	PRINT_IQ_INFO(sfile, "get nnsc green_enh_ratio : %d \r\n", iq_nnsc_get_info(iq_proc_id, IQ_NNSC_ITEM_GREEN_ENH_RATIO));
	PRINT_IQ_INFO(sfile, "get nnsc skin_enh_ratio : %d \r\n", iq_nnsc_get_info(iq_proc_id, IQ_NNSC_ITEM_SKIN_ENH_RATIO));
}

#if defined(__FREERTOS)
void iq_msg_show_cfg_path(void)
#else
void iq_msg_show_cfg_path(struct seq_file *sfile)
#endif
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;
	UINT32 id;

	for (id = 0; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_info->cfg_valid) {
			PRINT_IQ_INFO(sfile, "id : %d, path = %s \r\n", id, iq_info->cfg_path);
		}
	}
}

#if defined(__FREERTOS)
void iq_msg_show_low_power_info(void)
#else
void iq_msg_show_low_power_info(struct seq_file *sfile)
#endif
{
	PRINT_IQ_INFO(sfile, "NT98538 not support iq_msg_show_low_power_info() \r\n");
}

#if defined(__FREERTOS)
void iq_msg_show_low_power_param(void)
#else
void iq_msg_show_low_power_param(struct seq_file *sfile)
#endif
{
	PRINT_IQ_INFO(sfile, "NT98538 not support iq_msg_show_low_power_param() \r\n");
}

#if defined(__FREERTOS)
void iq_msg_show_manual_info(void)
#else
void iq_msg_show_manual_info(struct seq_file *sfile)
#endif
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;
	UINT32 id, i;

	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");
	PRINT_IQ_INFO(sfile, "    OB MANUAL INFORMATION \r\n");
	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");

	PRINT_IQ_INFO(sfile, "      sie_en :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->ob_mode_manual.sie_enable) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "   sie_value :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, "      %4d", iq_info->ob_mode_manual.sie_value);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "    ife_f_en :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->ob_mode_manual.ife_f_enable) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	for (i = 0; i < IQ_OB_LEN; i++) {
		PRINT_IQ_INFO(sfile, "ife_f_value%d :", i);
		for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
			iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
			if (iq_flow_get_id_valid(id)) {
				PRINT_IQ_INFO(sfile, "      %4d", iq_info->ob_mode_manual.ife_f_value[i]);
			}
		}
		PRINT_IQ_INFO(sfile, "\r\n");
	}

	PRINT_IQ_INFO(sfile, "      ife_en :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->ob_mode_manual.ife_enable) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	for (i = 0; i < IQ_OB_LEN; i++) {
		PRINT_IQ_INFO(sfile, "  ife_value%d :", i);
		for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
			iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
			if (iq_flow_get_id_valid(id)) {
				PRINT_IQ_INFO(sfile, "      %4d", iq_info->ob_mode_manual.ife_value[i]);
			}
		}
		PRINT_IQ_INFO(sfile, "\r\n");
	}

	PRINT_IQ_INFO(sfile, "   manual_en :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->ob_mode_manual.manual_enable) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, " manual_mode :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			if (iq_info->ob_mode_manual.manual_mode == IQ_OB_SIE) {
				PRINT_IQ_INFO(sfile, "       SIE");
			} else if (iq_info->ob_mode_manual.manual_mode == IQ_OB_PRE_F) {
				PRINT_IQ_INFO(sfile, "     PRE_F");
			} else if (iq_info->ob_mode_manual.manual_mode == IQ_OB_PRE) {
				PRINT_IQ_INFO(sfile, "       PRE");
			} else if (iq_info->ob_mode_manual.manual_mode == IQ_OB_IFE_F) {
				PRINT_IQ_INFO(sfile, "     IFE_F");
			} else if (iq_info->ob_mode_manual.manual_mode == IQ_OB_IFE) {
				PRINT_IQ_INFO(sfile, "       IFE");
			} else {
				PRINT_IQ_INFO(sfile, "    UNKNOW");
			}
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");
	PRINT_IQ_INFO(sfile, "    D GAIN MANUAL INFORMATION \r\n");
	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");

	PRINT_IQ_INFO(sfile, "      sie_en :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->dg_mode_manual.sie_enable) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "   sie_value :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, "      %4d", iq_info->dg_mode_manual.sie_value);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "      ife_en :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->dg_mode_manual.ife_enable) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "   ife_value :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, "      %4d", iq_info->dg_mode_manual.ife_value);
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "   manual_en :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->dg_mode_manual.manual_enable) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, " manual_mode :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			if (iq_info->dg_mode_manual.manual_mode == IQ_DG_OFF) {
				PRINT_IQ_INFO(sfile, "       OFF");
			} else if (iq_info->dg_mode_manual.manual_mode == IQ_DG_SIE) {
				PRINT_IQ_INFO(sfile, "       SIE");
			} else if (iq_info->dg_mode_manual.manual_mode == IQ_DG_PRE) {
				PRINT_IQ_INFO(sfile, "       PRE");
			} else if (iq_info->dg_mode_manual.manual_mode == IQ_DG_IFE) {
				PRINT_IQ_INFO(sfile, "       IFE");
			} else {
				PRINT_IQ_INFO(sfile, "    UNKNOW");
			}
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");
	PRINT_IQ_INFO(sfile, "    C GAIN MANUAL INFORMATION \r\n");
	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");

	PRINT_IQ_INFO(sfile, "   manual_en :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			PRINT_IQ_INFO(sfile, " %9d", ((iq_info->cg_mode_manual.manual_enable) ? 1 : 0));
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, " manual_mode :");
	for (id = IQ_ID_1; id < IQ_ID_MAX_NUM; id++) {
		iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
		if (iq_flow_get_id_valid(id)) {
			if (iq_info->cg_mode_manual.manual_mode == IQ_CG_SIE) {
				PRINT_IQ_INFO(sfile, "       SIE");
			} else if (iq_info->cg_mode_manual.manual_mode == IQ_CG_IFE_F) {
				PRINT_IQ_INFO(sfile, "     IFE_F");
			} else if (iq_info->cg_mode_manual.manual_mode == IQ_CG_IFE) {
				PRINT_IQ_INFO(sfile, "       IFE");
			} else {
				PRINT_IQ_INFO(sfile, "    UNKNOW");
			}
		}
	}
	PRINT_IQ_INFO(sfile, "\r\n");

	PRINT_IQ_INFO(sfile, "---------------------------------------------------------------------------------------------- \r\n");
}

