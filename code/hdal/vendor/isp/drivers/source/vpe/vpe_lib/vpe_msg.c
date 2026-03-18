#if defined(__FREERTOS)
#include <string.h>
#include <stdio.h>
#else
#include <linux/seq_file.h>
#endif
#include "nvtmpp.h"

#include "kwrap/type.h"

#include "vpe_msg.h"
#include "vpe_dbg.h"
#include "vpe_uti.h"
#include "vpe_version.h"

//=============================================================================
// global
//=============================================================================
extern VPE_PARAM_PTR *vpe_param[VPE_ID_MAX_NUM][VPE_IDX_MAX_NUM];
extern USIZE vpe_2dlut_output_size[VPE_ID_MAX_NUM][VPE_IDX_MAX_NUM];
extern USIZE vpe_dctg_output_size[VPE_ID_MAX_NUM][VPE_IDX_MAX_NUM];
extern USIZE vpe_out_size[VPE_ID_MAX_NUM];
extern UINT32 vpe_dcout_mode[VPE_ID_MAX_NUM];
extern UINT32 vpe_id_list;
extern UINT32 vpe_idx_list;
extern UINT32 vpe_idx_num;
extern UINT32 vpe_2dlut_size;

//=============================================================================
// external functions
//=============================================================================
#if defined(__FREERTOS)
void vpe_msg_show_info(void)
#else
void vpe_msg_show_info(struct seq_file *sfile)
#endif
{
	UINT32 version = vpe_get_version();

	PRINT_VPE_INFO(sfile, "-----------------------------------------------------------------------------------------\r\n");
	PRINT_VPE_INFO(sfile, "NVT_VPE v%d.%d.%d.%d \r\n", (version >> 24) & 0xFF, (version >> 16) & 0xFF, (version >> 8) & 0xFF, version & 0xFF);
	PRINT_VPE_INFO(sfile, "-----------------------------------------------------------------------------------------\r\n");
	PRINT_VPE_INFO(sfile, "    Module Info:       \n");
	PRINT_VPE_INFO(sfile, "    vpe_id_list:  0x%X \n", vpe_id_list);
	PRINT_VPE_INFO(sfile, "   vpe_idx_list:  0x%X \n", vpe_idx_list);
	PRINT_VPE_INFO(sfile, "    vpe_idx_num:  0x%X \n", vpe_idx_num);
	PRINT_VPE_INFO(sfile, " vpe_2dlut_size:  %d   \n", vpe_2dlut_size);
	PRINT_VPE_INFO(sfile, "-----------------------------------------------------------------------------------------\r\n");
	PRINT_VPE_INFO(sfile, "    Warning Cnt:    %8d  \n", vpe_dbg_get_wrn_msg());
	PRINT_VPE_INFO(sfile, "-----------------------------------------------------------------------------------------\r\n");
}

#if defined(__FREERTOS)
void vpe_msg_show_dce_ctrl(VPE_ID vpe_proc_id, UINT32 vpe_proc_idx)
#else
void vpe_msg_show_dce_ctrl(struct seq_file *sfile, VPE_ID vpe_proc_id, UINT32 vpe_proc_idx)
#endif
{
	PRINT_VPE_INFO(sfile, "==================== id(%d) idx(%d) vpe ctl_param ==================== \r\n", vpe_proc_id, vpe_proc_idx);
	PRINT_VPE_INFO(sfile, "dce enable = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dce_ctl->enable);
	PRINT_VPE_INFO(sfile, "dce mode = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dce_ctl->dce_mode);
	PRINT_VPE_INFO(sfile, "dce fovbound = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dce_ctl->fovbound);
	PRINT_VPE_INFO(sfile, "dce boundy = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dce_ctl->boundy);
	PRINT_VPE_INFO(sfile, "dce boundu = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dce_ctl->boundu);
	PRINT_VPE_INFO(sfile, "dce boundv = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dce_ctl->boundv);
}

#if defined(__FREERTOS)
void vpe_msg_show_sharpen(VPE_ID vpe_proc_id, UINT32 vpe_proc_idx)
#else
void vpe_msg_show_sharpen(struct seq_file *sfile, VPE_ID vpe_proc_id, UINT32 vpe_proc_idx)
#endif
{
	INT32 i;

	PRINT_VPE_INFO(sfile, "==================== id(%d) idx(%d) vpe sharpen_param ==================== \r\n", vpe_proc_id, vpe_proc_idx);
	PRINT_VPE_INFO(sfile, "sharpen enable = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->sharpen->enable);
	PRINT_VPE_INFO(sfile, "sharpen edge sharp str1 = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->sharpen->edge_sharp_str1);
	PRINT_VPE_INFO(sfile, "sharpen edge sharp str2 = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->sharpen->edge_sharp_str2);
	PRINT_VPE_INFO(sfile, "sharpen flat sharp str = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->sharpen->flat_sharp_str);
	PRINT_VPE_INFO(sfile, "sharpen edge weight gain = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->sharpen->edge_weight_gain);
	PRINT_VPE_INFO(sfile, "sharpen dbs_gain_en = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->sharpen->dbs_gain_en);
	PRINT_VPE_INFO(sfile, "sharpen quad_area_clamping = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->sharpen->quad_area_clamping);
	for (i = 0; i < VPE_SHARPEN_DBS_GAIN_NUM; i++) {
		PRINT_VPE_INFO(sfile, "sharpen dbs_gain[%d] = %d, \r\n", i, vpe_param[vpe_proc_id][vpe_proc_idx]->sharpen->dbs_gain[i]);
	}

}

#if defined(__FREERTOS)
void vpe_msg_show_2dlut(VPE_ID vpe_proc_id, UINT32 vpe_proc_idx)
#else
void vpe_msg_show_2dlut(struct seq_file *sfile, VPE_ID vpe_proc_id, UINT32 vpe_proc_idx)
#endif
{
	INT32 i;
	UINT32 _2dlut_tab_temp;

	PRINT_VPE_INFO(sfile, "==================== id(%d) idx(%d) vpe 2dlut_param ==================== \r\n", vpe_proc_id, vpe_proc_idx);
	PRINT_VPE_INFO(sfile, "2dlut size = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->lut2d_int->lut_sz);
	for (i = 0; i < 7; i++) {
		_2dlut_tab_temp = *(UINT32 *)(vpe_param[vpe_proc_id][vpe_proc_idx]->lut2d_int->lut_addr + i * sizeof(UINT32));
		PRINT_VPE_INFO(sfile, "2dlut[%d] = 0x%8X, x = %d, y = %d \r\n", i, _2dlut_tab_temp, (_2dlut_tab_temp & 0xFFFF) >> 2, ((_2dlut_tab_temp >> 16) & 0xFFFF) >> 2);
	}
	for (i = 7; i >= 0; i--) {
		_2dlut_tab_temp = *(UINT32 *)(vpe_param[vpe_proc_id][vpe_proc_idx]->lut2d_int->lut_addr + vpe_uti_calc_2dlut_tbl_size(vpe_param[vpe_proc_id][vpe_proc_idx]->lut2d_int->lut_sz) - (i + 1) * sizeof(UINT32));
		PRINT_VPE_INFO(sfile, "2dlut[%d] = 0x%X, x = %d, y = %d \r\n", (int)(vpe_uti_calc_2dlut_tbl_size(vpe_param[vpe_proc_id][vpe_proc_idx]->lut2d_int->lut_sz) / sizeof(UINT32) - (i + 1)), _2dlut_tab_temp, (_2dlut_tab_temp & 0xFFFF) >> 2, ((_2dlut_tab_temp >> 16) & 0xFFFF) >> 2);
	}
	PRINT_VPE_INFO(sfile, "2dlut auto dcout size = %d, %d \r\n", vpe_2dlut_output_size[vpe_proc_id][vpe_proc_idx].w, vpe_2dlut_output_size[vpe_proc_id][vpe_proc_idx].h);
	PRINT_VPE_INFO(sfile, "2dlut output dcout size = %d, %d \r\n", vpe_out_size[vpe_proc_id].w, vpe_out_size[vpe_proc_id].h);
	PRINT_VPE_INFO(sfile, "2dlut dcout mode = %s \r\n", (vpe_dcout_mode[vpe_proc_id] == VPE_DCOUT_AUTO) ? "auto" : (vpe_dcout_mode[vpe_proc_id] == VPE_DCOUT_OUTPUT) ? "output" : "input");
	PRINT_VPE_INFO(sfile, "2dlut precision = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->lut2d_int->lut2d_precision);
}

#if defined(__FREERTOS)
void vpe_msg_show_2dlut_expand(VPE_ID vpe_proc_id, UINT32 vpe_proc_idx)
#else
void vpe_msg_show_2dlut_expand(struct seq_file *sfile, VPE_ID vpe_proc_id, UINT32 vpe_proc_idx)
#endif
{
	INT32 i;
	UINT32 _2dlut_tab_temp;

	PRINT_VPE_INFO(sfile, "==================== id(%d) idx(%d) vpe 2dlut_expand_param ==================== \r\n", vpe_proc_id, vpe_proc_idx);
	PRINT_VPE_INFO(sfile, "2dlut_expand xofs_i = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->lut2d_expand->xofs_i);
	PRINT_VPE_INFO(sfile, "2dlut_expand xofs_f = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->lut2d_expand->xofs_f);
	PRINT_VPE_INFO(sfile, "2dlut_expand yofs_i = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->lut2d_expand->yofs_i);
	PRINT_VPE_INFO(sfile, "2dlut_expand yofs_f = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->lut2d_expand->yofs_f);
	PRINT_VPE_INFO(sfile, "2dlut_expand lut2d_width = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->lut2d_expand->lut2d_width);
	PRINT_VPE_INFO(sfile, "2dlut_expand lut2d_height = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->lut2d_expand->lut2d_height);
	PRINT_VPE_INFO(sfile, "2dlut_expand lut2d_lofs = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->lut2d_expand->lut2d_lofs);
	PRINT_VPE_INFO(sfile, "2dlut_expand lut_addr_pa = 0x%lx, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->lut2d_expand->lut_addr_pa);
	PRINT_VPE_INFO(sfile, "2dlut auto dcout size = %d, %d \r\n", vpe_2dlut_output_size[vpe_proc_id][vpe_proc_idx].w, vpe_2dlut_output_size[vpe_proc_id][vpe_proc_idx].h);
	PRINT_VPE_INFO(sfile, "2dlut output dcout size = %d, %d \r\n", vpe_out_size[vpe_proc_id].w, vpe_out_size[vpe_proc_id].h);
	PRINT_VPE_INFO(sfile, "2dlut dcout mode = %s \r\n", (vpe_dcout_mode[vpe_proc_id] == VPE_DCOUT_AUTO) ? "auto" : "output");

	if (vpe_param[vpe_proc_id][vpe_proc_idx]->lut2d_expand->lut_addr_pa != 0) {
		uintptr_t lut_addr_va;

		lut_addr_va = nvtmpp_sys_pa2va(vpe_param[vpe_proc_id][vpe_proc_idx]->lut2d_expand->lut_addr_pa);
		for (i = 0; i < 7; i++) {
			_2dlut_tab_temp = *(UINT32 *)(lut_addr_va + i * sizeof(UINT32));
			PRINT_VPE_INFO(sfile, "2dlut_expand lut[%d] = 0x%8X, x = %d, y = %d \r\n", i, _2dlut_tab_temp, (_2dlut_tab_temp & 0xFFFF) >> 2, ((_2dlut_tab_temp >> 16) & 0xFFFF) >> 2);
		}
		for (i = 7; i >= 0; i--) {
			UINT32 lut2d_size = vpe_param[vpe_proc_id][vpe_proc_idx]->lut2d_expand->lut2d_lofs * vpe_param[vpe_proc_id][vpe_proc_idx]->lut2d_expand->lut2d_height;
			_2dlut_tab_temp = *(UINT32 *)(lut_addr_va + lut2d_size * sizeof(UINT32) - (i + 1) * sizeof(UINT32));
			PRINT_VPE_INFO(sfile, "2dlut_expand lut[%d] = 0x%X, x = %d, y = %d \r\n", lut2d_size - (i + 1), _2dlut_tab_temp, (_2dlut_tab_temp & 0xFFFF) >> 2, ((_2dlut_tab_temp >> 16) & 0xFFFF) >> 2);
		}
	}
	PRINT_VPE_INFO(sfile, "2dlut_expand precision = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->lut2d_expand->lut2d_precision);
}

#if defined(__FREERTOS)
void vpe_msg_show_drt(VPE_ID vpe_proc_id, UINT32 vpe_proc_idx)
#else
void vpe_msg_show_drt(struct seq_file *sfile, VPE_ID vpe_proc_id, UINT32 vpe_proc_idx)
#endif
{
	INT32 i;

	PRINT_VPE_INFO(sfile, "==================== id(%d) idx(%d) vpe drt_param ==================== \r\n", vpe_proc_id, vpe_proc_idx);
	for (i = 0; i < VPE_DRT_PATH_NUM; i++) {
		PRINT_VPE_INFO(sfile, "drt cvt_sel[%d] = %d, \r\n", i, vpe_param[vpe_proc_id][vpe_proc_idx]->drt->cvt_sel[i]);
	}
	for (i = 0; i < VPE_DRT_PATH_NUM; i++) {
		PRINT_VPE_INFO(sfile, "drt pc2tv_weight[%d] = %d, \r\n", i, vpe_param[vpe_proc_id][vpe_proc_idx]->drt->pc2tv_weight[i]);
	}
	PRINT_VPE_INFO(sfile, "drt src_drt_sel = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->drt->src_drt_sel);
	PRINT_VPE_INFO(sfile, "drt src_pc2tv_weight = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->drt->src_pc2tv_weight);
}

#if defined(__FREERTOS)
void vpe_msg_show_dctg(VPE_ID vpe_proc_id, UINT32 vpe_proc_idx)
#else
void vpe_msg_show_dctg(struct seq_file *sfile, VPE_ID vpe_proc_id, UINT32 vpe_proc_idx)
#endif
{
	PRINT_VPE_INFO(sfile, "==================== id(%d) idx(%d) vpe dctg_ctrl ==================== \r\n", vpe_proc_id, vpe_proc_idx);
	PRINT_VPE_INFO(sfile, "mode_sel = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->mode_sel);
	PRINT_VPE_INFO(sfile, "dctg mount_type = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->dctg_param.mount_type);
	PRINT_VPE_INFO(sfile, "dctg lut2d_width = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->dctg_param.lut2d_width);
	PRINT_VPE_INFO(sfile, "dctg lut2d_height = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->dctg_param.lut2d_height);
	PRINT_VPE_INFO(sfile, "dctg lens_r = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->dctg_param.lens_r);
	PRINT_VPE_INFO(sfile, "dctg lens_cent_x = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->dctg_param.lens_cent_x);
	PRINT_VPE_INFO(sfile, "dctg lens_cent_y = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->dctg_param.lens_cent_y);
	PRINT_VPE_INFO(sfile, "dctg long_aov = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->dctg_param.long_aov);
	PRINT_VPE_INFO(sfile, "dctg lati_aov = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->dctg_param.lati_aov);
	PRINT_VPE_INFO(sfile, "dctg pan_angle = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->dctg_param.pan_angle);
	PRINT_VPE_INFO(sfile, "dctg tilt_angle = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->dctg_param.tilt_angle);
	PRINT_VPE_INFO(sfile, "dctg rot_angle = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->dctg_param.rot_angle);
	PRINT_VPE_INFO(sfile, "dctg adj_v_angle = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->dctg_param.adj_v_angle);
	PRINT_VPE_INFO(sfile, "dctg distor_lut_en = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->dctg_param.distor_lut_en);
	PRINT_VPE_INFO(sfile, "dctg max_diag_dist = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->dctg_param.max_diag_dist);
	PRINT_VPE_INFO(sfile, "dctg adj_fov_aspect_ratio_en = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->dctg_param.adj_fov_aspect_ratio_en);
	PRINT_VPE_INFO(sfile, "ptz proj_type = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->ptz_param.proj_type);
	PRINT_VPE_INFO(sfile, "ptz long_aov = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->ptz_param.long_aov);
	PRINT_VPE_INFO(sfile, "ptz lati_aov = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->ptz_param.lati_aov);
	PRINT_VPE_INFO(sfile, "ptz pan_angle = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->ptz_param.pan_angle);
	PRINT_VPE_INFO(sfile, "ptz tilt_angle = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->ptz_param.tilt_angle);
	PRINT_VPE_INFO(sfile, "ptz rot_angle = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->ptz_param.rot_angle);
	PRINT_VPE_INFO(sfile, "ptz zoom_step = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->ptz_param.zoom_step);
	PRINT_VPE_INFO(sfile, "ptz cam_long_aov = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->ptz_param.cam_long_aov);
	PRINT_VPE_INFO(sfile, "ptz cam_lati_aov = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->ptz_param.cam_lati_aov);
	PRINT_VPE_INFO(sfile, "ptz cam_w = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->ptz_param.cam_w);
	PRINT_VPE_INFO(sfile, "ptz cam_h = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->ptz_param.cam_h);
	PRINT_VPE_INFO(sfile, "ptz stitch_overlap_angle = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->dctg->ptz_param.stitch_overlap_angle);
	PRINT_VPE_INFO(sfile, "output size = %d, %d \r\n", vpe_dctg_output_size[vpe_proc_id][vpe_proc_idx].w, vpe_dctg_output_size[vpe_proc_id][vpe_proc_idx].h);
}

#if defined(__FREERTOS)
void vpe_msg_show_flip_rot(VPE_ID vpe_proc_id, UINT32 vpe_proc_idx)
#else
void vpe_msg_show_flip_rot(struct seq_file *sfile, VPE_ID vpe_proc_id, UINT32 vpe_proc_idx)
#endif
{
	PRINT_VPE_INFO(sfile, "==================== id(%d) idx(%d) vpe flip_rot_param ==================== \r\n", vpe_proc_id, vpe_proc_idx);
	PRINT_VPE_INFO(sfile, "flip_rot_mode = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->flip_rot->flip_rot_mode);
	PRINT_VPE_INFO(sfile, "rot_degree = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->flip_rot->rot_manual_param.rot_degree);
	PRINT_VPE_INFO(sfile, "flip = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->flip_rot->rot_manual_param.flip);
	PRINT_VPE_INFO(sfile, "ratio_mode = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->flip_rot->rot_manual_param.ratio_mode);
	PRINT_VPE_INFO(sfile, "fovbound = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->flip_rot->rot_manual_param.fovbound);
	PRINT_VPE_INFO(sfile, "boundy = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->flip_rot->rot_manual_param.boundy);
	PRINT_VPE_INFO(sfile, "boundu = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->flip_rot->rot_manual_param.boundu);
	PRINT_VPE_INFO(sfile, "boundv = %d, \r\n", vpe_param[vpe_proc_id][vpe_proc_idx]->flip_rot->rot_manual_param.boundv);
}

