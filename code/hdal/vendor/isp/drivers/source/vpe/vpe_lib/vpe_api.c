#if defined(__FREERTOS)
#include "string.h"
#endif
#include "kwrap/error_no.h"
#include "kwrap/cpu.h"
#include "nvtmpp.h"

#include "ctl_vpe_isp.h"
#include "vpe_alg_int.h"
#include "vpe_api_int.h"
#include "vpe_common_param_int.h"
#include "vpet_api_int.h"
#include "vpe_param_default.h"
#include "vpe_dbg.h"
#include "vpe_dev_int.h"
#include "vpe_uti.h"
#if defined(__KERNEL__)
#include "vpe_main.h"
#endif

//=============================================================================
// define
//=============================================================================
#define VPE_IF_REG_NAME   "NVT_VPE_IF"
#define VPE_IF_REG_EVENT  ISP_EVENT_VPE_CFG_IMM

extern UINT32 real_2dlut_size;

//=============================================================================
// global
//=============================================================================
#if defined(__KERNEL__)
extern UINT32 vpe_id_list;
extern UINT32 vpe_idx_list;
extern UINT32 vpe_idx_num;
extern UINT32 vpe_2dlut_size;
#else
UINT32 vpe_id_list = 0x3;
UINT32 vpe_idx_list = 0x1;
UINT32 vpe_idx_num = 0x2;
UINT32 vpe_2dlut_size = 0x5;
#endif

VPE_MODULE vpe_module;
VPE_PARAM_PTR *vpe_param[VPE_ID_MAX_NUM][VPE_IDX_MAX_NUM] = {NULL};
VPE_PARAM_PTR vpe_param_memalloc_addr[VPE_ID_MAX_NUM][VPE_IDX_MAX_NUM] = {0};
static BOOL vpe_param_memalloc_valid[VPE_ID_MAX_NUM][VPE_IDX_MAX_NUM] = {0};
BOOL vpe_id_valid[VPE_ID_MAX_NUM][VPE_IDX_MAX_NUM] = {0};
USIZE vpe_2dlut_output_size[VPE_ID_MAX_NUM][VPE_IDX_MAX_NUM] = {0};
USIZE vpe_dctg_output_size[VPE_ID_MAX_NUM][VPE_IDX_MAX_NUM] = {0};
USIZE vpe_out_size[VPE_ID_MAX_NUM];
UINT32 vpe_dcout_mode[VPE_ID_MAX_NUM] = {0};
static UINT32 predict_2dlut_size;
static ULONG param_phy_2dlut_addr;
static USIZE vpe_in_size;

//=============================================================================
// internal functions
//=============================================================================
static void vpe_set_dce_ctl_param(VPE_ID id, UINT32 idx)
{
	ctl_vpe_dce_ctl_init.enable = vpe_param[id][idx]->dce_ctl->enable;
	ctl_vpe_dce_ctl_init.lsb_rand = vpe_param[id][idx]->dce_ctl->lsb_rand;
	if (vpe_param[id][idx]->dce_ctl->dce_mode == VPE_ISP_DCE_MODE_2DLUT_ONLY) {
		ctl_vpe_dce_ctl_init.dce_mode = CTL_VPE_ISP_DCE_MODE_2DLUT_ONLY;
	} else {
		ctl_vpe_dce_ctl_init.dce_mode = CTL_VPE_ISP_DCE_MODE_2DLUT_DCTG;
	}
	ctl_vpe_dce_ctl_init.fovbound = vpe_param[id][idx]->dce_ctl->fovbound;
	ctl_vpe_dce_ctl_init.boundy = vpe_param[id][idx]->dce_ctl->boundy;
	ctl_vpe_dce_ctl_init.boundu = vpe_param[id][idx]->dce_ctl->boundu;
	ctl_vpe_dce_ctl_init.boundv = vpe_param[id][idx]->dce_ctl->boundv;
	memcpy(ctl_vpe_dce_ctl_init.geo_lut, vpe_param[id][idx]->dce_ctl->geo_lut, sizeof(UINT16) * VPE_GEO_LUT_NUMS);

	return;
}

static void vpe_set_sharpen_param(VPE_ID  id, UINT32 idx)
{
	ctl_vpe_sharpen_init.enable = vpe_param[id][idx]->sharpen->enable;
	ctl_vpe_sharpen_init.edge_sharp_str1 = vpe_param[id][idx]->sharpen->edge_sharp_str1;
	ctl_vpe_sharpen_init.edge_sharp_str2 = vpe_param[id][idx]->sharpen->edge_sharp_str2;
	ctl_vpe_sharpen_init.flat_sharp_str = vpe_param[id][idx]->sharpen->flat_sharp_str;
	ctl_vpe_sharpen_init.edge_weight_gain = vpe_param[id][idx]->sharpen->edge_weight_gain;
	ctl_vpe_sharpen_init.dbs_gain_en = vpe_param[id][idx]->sharpen->dbs_gain_en;
	ctl_vpe_sharpen_init.quad_area_clamping = vpe_param[id][idx]->sharpen->quad_area_clamping;
	memcpy(ctl_vpe_sharpen_init.dbs_gain, vpe_param[id][idx]->sharpen->dbs_gain, sizeof(UINT8) * VPE_SHARPEN_DBS_GAIN_NUM);

	return;
}

static void vpe_set_dcout_mode_param(VPE_ID id, UINT32 idx)
{
	if (vpe_dcout_mode[id] == VPE_DCOUT_OUTPUT) {
		ctl_vpe_2dlut_init.out_size.w = vpe_out_size[id].w;
		ctl_vpe_2dlut_init.out_size.h = vpe_out_size[id].h;
	} else if (vpe_dcout_mode[id] == VPE_DCOUT_INPUT) {
		ctl_vpe_2dlut_init.out_size.w = 0;
		ctl_vpe_2dlut_init.out_size.h = 0;
	} else {
		ctl_vpe_2dlut_init.out_size.w = vpe_2dlut_output_size[id][idx].w;
		ctl_vpe_2dlut_init.out_size.h = vpe_2dlut_output_size[id][idx].h;
	}

	return;
}

static void vpe_set_2dlut_param(VPE_ID id, UINT32 idx)
{
	ULONG lut_addr_pa;
	USIZE lut_sz;

	lut_addr_pa = vos_cpu_get_phy_addr(vpe_param[id][idx]->lut2d_int->lut_addr);
	lut_sz.w = vpe_param[id][idx]->lut2d_int->lut_sz;
	lut_sz.h = vpe_param[id][idx]->lut2d_int->lut_sz;
	vpe_uti_calc_2dlut_output_size(&vpe_2dlut_output_size[id][idx], &lut_sz, &vpe_in_size, vpe_param[id][idx]->lut2d_int->lut2d_precision, (UINT32 *)vpe_param[id][idx]->lut2d_int->lut_addr);
	vos_cpu_dcache_sync((VOS_ADDR)vpe_param[id][idx]->lut2d_int->lut_addr, vpe_uti_calc_2dlut_tbl_size(vpe_param[id][idx]->lut2d_int->lut_sz), VOS_DMA_TO_DEVICE);

	ctl_vpe_2dlut_init.xofs_i = 0;
	ctl_vpe_2dlut_init.xofs_f = 0;
	ctl_vpe_2dlut_init.yofs_i = 0;
	ctl_vpe_2dlut_init.yofs_f = 0;
	ctl_vpe_2dlut_init.lut2d_width = vpe_param[id][idx]->lut2d_int->lut_sz;
	ctl_vpe_2dlut_init.lut2d_height = vpe_param[id][idx]->lut2d_int->lut_sz;
	ctl_vpe_2dlut_init.lut2d_lofs = ALIGN_CEIL(vpe_param[id][idx]->lut2d_int->lut_sz, 4);
	ctl_vpe_2dlut_init.lut_addr_pa = lut_addr_pa;
	vpe_set_dcout_mode_param(id, idx);
	ctl_vpe_2dlut_init.lut2d_precision = vpe_param[id][idx]->lut2d_int->lut2d_precision;

	return;
}

static void vpe_set_2dlut_expand_param(VPE_ID id, UINT32 idx)
{
	uintptr_t lut_addr_va;
	USIZE lut_sz;

	lut_addr_va = nvtmpp_sys_pa2va(vpe_param[id][idx]->lut2d_expand->lut_addr_pa);
	lut_sz.w = vpe_param[id][idx]->lut2d_expand->lut2d_width;
	lut_sz.h = vpe_param[id][idx]->lut2d_expand->lut2d_height;
	vpe_uti_calc_2dlut_output_size(&vpe_2dlut_output_size[id][idx], &lut_sz, &vpe_in_size, vpe_param[id][idx]->lut2d_expand->lut2d_precision, (UINT32 *)lut_addr_va);

	ctl_vpe_2dlut_init.xofs_i = vpe_param[id][idx]->lut2d_expand->xofs_i;
	ctl_vpe_2dlut_init.xofs_f = vpe_param[id][idx]->lut2d_expand->xofs_f;
	ctl_vpe_2dlut_init.yofs_i = vpe_param[id][idx]->lut2d_expand->yofs_i;
	ctl_vpe_2dlut_init.yofs_f = vpe_param[id][idx]->lut2d_expand->yofs_f;
	ctl_vpe_2dlut_init.lut2d_width = vpe_param[id][idx]->lut2d_expand->lut2d_width;
	ctl_vpe_2dlut_init.lut2d_height = vpe_param[id][idx]->lut2d_expand->lut2d_height;
	ctl_vpe_2dlut_init.lut2d_lofs = ALIGN_CEIL(vpe_param[id][idx]->lut2d_expand->lut2d_lofs, 4);
	ctl_vpe_2dlut_init.lut_addr_pa = vpe_param[id][idx]->lut2d_expand->lut_addr_pa;
	vpe_set_dcout_mode_param(id, idx);
	ctl_vpe_2dlut_init.lut2d_precision = vpe_param[id][idx]->lut2d_expand->lut2d_precision;

	return;
}

static void vpe_set_drt_param(VPE_ID id, UINT32 idx)
{
	memcpy(ctl_vpe_cvt_init.cvt_sel, vpe_param[id][idx]->drt->cvt_sel, sizeof(VPE_YUV_CVT) * VPE_DRT_PATH_NUM);
	memcpy(ctl_vpe_cvt_init.pc2tv_weight, vpe_param[id][idx]->drt->pc2tv_weight, sizeof(UINT16) * VPE_DRT_PATH_NUM);
	ctl_vpe_cvt_init.src_drt_sel = vpe_param[id][idx]->drt->src_drt_sel;
	ctl_vpe_cvt_init.src_pc2tv_weight = vpe_param[id][idx]->drt->src_pc2tv_weight;

	return;
}

static void vpe_set_dctg_param(VPE_ID id, UINT32 idx)
{
	ctl_vpe_dctg_init.mode_sel = vpe_param[id][idx]->dctg->mode_sel;
	ctl_vpe_dctg_init.dctg_param.mount_type = vpe_param[id][idx]->dctg->dctg_param.mount_type;
	ctl_vpe_dctg_init.dctg_param.lut2d_width = vpe_param[id][idx]->dctg->dctg_param.lut2d_width;
	ctl_vpe_dctg_init.dctg_param.lut2d_height = vpe_param[id][idx]->dctg->dctg_param.lut2d_height;
	ctl_vpe_dctg_init.dctg_param.lens_r = vpe_param[id][idx]->dctg->dctg_param.lens_r;
	ctl_vpe_dctg_init.dctg_param.lens_cent_x = vpe_param[id][idx]->dctg->dctg_param.lens_cent_x;
	ctl_vpe_dctg_init.dctg_param.lens_cent_y = vpe_param[id][idx]->dctg->dctg_param.lens_cent_y;
	ctl_vpe_dctg_init.dctg_param.long_aov = vpe_param[id][idx]->dctg->dctg_param.long_aov;
	ctl_vpe_dctg_init.dctg_param.lati_aov = vpe_param[id][idx]->dctg->dctg_param.lati_aov;
	ctl_vpe_dctg_init.dctg_param.pan_angle = vpe_param[id][idx]->dctg->dctg_param.pan_angle;
	ctl_vpe_dctg_init.dctg_param.tilt_angle = vpe_param[id][idx]->dctg->dctg_param.tilt_angle;
	ctl_vpe_dctg_init.dctg_param.rot_angle = vpe_param[id][idx]->dctg->dctg_param.rot_angle;
	ctl_vpe_dctg_init.dctg_param.adj_v_angle = vpe_param[id][idx]->dctg->dctg_param.adj_v_angle;
	ctl_vpe_dctg_init.dctg_param.distor_lut_en = vpe_param[id][idx]->dctg->dctg_param.distor_lut_en;
	ctl_vpe_dctg_init.dctg_param.max_diag_dist = vpe_param[id][idx]->dctg->dctg_param.max_diag_dist;
	ctl_vpe_dctg_init.dctg_param.adj_fov_aspect_ratio_en = vpe_param[id][idx]->dctg->dctg_param.adj_fov_aspect_ratio_en;
	ctl_vpe_dctg_init.ptz_param.proj_type = vpe_param[id][idx]->dctg->ptz_param.proj_type;
	ctl_vpe_dctg_init.ptz_param.long_aov = vpe_param[id][idx]->dctg->ptz_param.long_aov;
	ctl_vpe_dctg_init.ptz_param.lati_aov = vpe_param[id][idx]->dctg->ptz_param.lati_aov;
	ctl_vpe_dctg_init.ptz_param.pan_angle = vpe_param[id][idx]->dctg->ptz_param.pan_angle;
	ctl_vpe_dctg_init.ptz_param.tilt_angle = vpe_param[id][idx]->dctg->ptz_param.tilt_angle;
	ctl_vpe_dctg_init.ptz_param.rot_angle = vpe_param[id][idx]->dctg->ptz_param.rot_angle;
	ctl_vpe_dctg_init.ptz_param.zoom_step = vpe_param[id][idx]->dctg->ptz_param.zoom_step;
	ctl_vpe_dctg_init.ptz_param.cam_long_aov = vpe_param[id][idx]->dctg->ptz_param.cam_long_aov;
	ctl_vpe_dctg_init.ptz_param.cam_lati_aov = vpe_param[id][idx]->dctg->ptz_param.cam_lati_aov;
	ctl_vpe_dctg_init.ptz_param.cam_w = vpe_param[id][idx]->dctg->ptz_param.cam_w;
	ctl_vpe_dctg_init.ptz_param.cam_h = vpe_param[id][idx]->dctg->ptz_param.cam_h;
	ctl_vpe_dctg_init.ptz_param.stitch_overlap_angle = vpe_param[id][idx]->dctg->ptz_param.stitch_overlap_angle;
	ctl_vpe_dctg_init.out_size.w = vpe_dctg_output_size[id][idx].w;
	ctl_vpe_dctg_init.out_size.h = vpe_dctg_output_size[id][idx].h;

	return;
}

static void vpe_set_flip_rot_param(VPE_ID id, UINT32 idx)
{
	ctl_vpe_flip_rot_init.flip_rot_mode = vpe_param[id][idx]->flip_rot->flip_rot_mode;
	ctl_vpe_flip_rot_init.rot_manual_param.rot_degree = vpe_param[id][idx]->flip_rot->rot_manual_param.rot_degree;
	ctl_vpe_flip_rot_init.rot_manual_param.flip = vpe_param[id][idx]->flip_rot->rot_manual_param.flip;
	ctl_vpe_flip_rot_init.rot_manual_param.ratio_mode = vpe_param[id][idx]->flip_rot->rot_manual_param.ratio_mode;
	ctl_vpe_flip_rot_init.rot_manual_param.fovbound = vpe_param[id][idx]->flip_rot->rot_manual_param.fovbound;
	ctl_vpe_flip_rot_init.rot_manual_param.boundy = vpe_param[id][idx]->flip_rot->rot_manual_param.boundy;
	ctl_vpe_flip_rot_init.rot_manual_param.boundu = vpe_param[id][idx]->flip_rot->rot_manual_param.boundu;
	ctl_vpe_flip_rot_init.rot_manual_param.boundv = vpe_param[id][idx]->flip_rot->rot_manual_param.boundv;

	return;
}

static ER vpe_api_get_param_addr(VPE_ID id)
{
	UINT32 total_param_size;
	VPE_PARAM_PTR *vpe_param_temp = NULL;
	void *param_mem_addr = NULL;
	static BOOL use_param_phy_addr = TRUE;
	int align_byte = 4;
	UINT32 idx;

	for (idx = 0; idx < vpe_idx_num; idx++) {
		if (!vpe_id_valid[id][idx]) {
			continue;
		}

		if (use_param_phy_addr == TRUE) {
			vpe_param[id][idx] = (VPE_PARAM_PTR*)vpe_get_param_default();
		} else {
			total_param_size = ALIGN_CEIL(sizeof(VPE_DCE_CTL_PARAM), align_byte) + 
								ALIGN_CEIL(sizeof(VPE_SHARPEN_PARAM), align_byte) + 
								ALIGN_CEIL(sizeof(VPE_2DLUT_INT_PARAM), align_byte) + 
								ALIGN_CEIL(sizeof(VPE_2DLUT_EXPAND_PARAM), align_byte) + 
								ALIGN_CEIL(sizeof(VPE_DRT_PARAM), align_byte) + 
								ALIGN_CEIL(sizeof(VPE_DCTG_CTRL), align_byte) +
								ALIGN_CEIL(sizeof(VPE_FLIP_ROT_PARAM), align_byte);

			param_mem_addr = vpe_uti_vmem_alloc(total_param_size);
			if (param_mem_addr == NULL) {
				DBG_WRN("allocate vpe parameter fail \r\n");
				return E_SYS;
			}

			vpe_param_memalloc_addr[id][idx].dce_ctl = (VPE_DCE_CTL_PARAM *)param_mem_addr;
			vpe_param_memalloc_addr[id][idx].sharpen = (VPE_SHARPEN_PARAM *)((UINT8 *)vpe_param_memalloc_addr[id][idx].dce_ctl + ALIGN_CEIL(sizeof(VPE_DCE_CTL_PARAM), align_byte));
			vpe_param_memalloc_addr[id][idx].lut2d_int = (VPE_2DLUT_INT_PARAM *)((UINT8 *)vpe_param_memalloc_addr[id][idx].sharpen + ALIGN_CEIL(sizeof(VPE_SHARPEN_PARAM), align_byte));
			vpe_param_memalloc_addr[id][idx].lut2d_expand = (VPE_2DLUT_EXPAND_PARAM *)((UINT8 *)vpe_param_memalloc_addr[id][idx].lut2d_int + ALIGN_CEIL(sizeof(VPE_2DLUT_INT_PARAM), align_byte));
			vpe_param_memalloc_addr[id][idx].drt = (VPE_DRT_PARAM *)((UINT8 *)vpe_param_memalloc_addr[id][idx].lut2d_expand + ALIGN_CEIL(sizeof(VPE_2DLUT_EXPAND_PARAM), align_byte));
			vpe_param_memalloc_addr[id][idx].dctg = (VPE_DCTG_CTRL *)((UINT8 *)vpe_param_memalloc_addr[id][idx].drt + ALIGN_CEIL(sizeof(VPE_DRT_PARAM), align_byte));
			vpe_param_memalloc_addr[id][idx].flip_rot = (VPE_FLIP_ROT_PARAM *)((UINT8 *)vpe_param_memalloc_addr[id][idx].dctg + ALIGN_CEIL(sizeof(VPE_DCTG_CTRL), align_byte));
			vpe_param_temp = (VPE_PARAM_PTR*)vpe_get_param_default();
			memcpy(vpe_param_memalloc_addr[id][idx].dce_ctl, vpe_param_temp->dce_ctl, sizeof(VPE_DCE_CTL_PARAM));
			memcpy(vpe_param_memalloc_addr[id][idx].sharpen, vpe_param_temp->sharpen, sizeof(VPE_SHARPEN_PARAM));
			memcpy(vpe_param_memalloc_addr[id][idx].lut2d_int, vpe_param_temp->lut2d_int, sizeof(VPE_2DLUT_INT_PARAM));
			memcpy(vpe_param_memalloc_addr[id][idx].lut2d_expand, vpe_param_temp->lut2d_expand, sizeof(VPE_2DLUT_EXPAND_PARAM));
			memcpy(vpe_param_memalloc_addr[id][idx].drt, vpe_param_temp->drt, sizeof(VPE_DRT_PARAM));
			memcpy(vpe_param_memalloc_addr[id][idx].dctg, vpe_param_temp->dctg, sizeof(VPE_DCTG_CTRL));
			memcpy(vpe_param_memalloc_addr[id][idx].flip_rot, vpe_param_temp->flip_rot, sizeof(VPE_FLIP_ROT_PARAM));
			vpe_param[id][idx] = &vpe_param_memalloc_addr[id][idx];
			vpe_param_memalloc_valid[id][idx] = TRUE;
		}

		predict_2dlut_size = vpe_uti_calc_2dlut_tbl_size(vpe_2dlut_size);
		vpe_param[id][idx]->lut2d_int->lut_sz = vpe_2dlut_size;
		vpe_param[id][idx]->lut2d_int->lut_addr = (ULONG)vpe_uti_kmem_alloc(predict_2dlut_size);

		if (use_param_phy_addr) {
			use_param_phy_addr = FALSE;
			param_phy_2dlut_addr = vpe_param[id][idx]->lut2d_int->lut_addr;
		}

		if (vpe_param[id][idx]->lut2d_int->lut_addr == 0) {
			DBG_WRN("allocate vpe 2dlut fail \r\n");
			return E_SYS;
		}
	}

	return E_OK;
}

//=============================================================================
// external functions
//=============================================================================
INT32 vpe_api_cb_flow(ISP_ID id, ISP_EVENT evt, UINT32 frame_cnt, void *param)
{
	CTL_VPE_ISP_IQ_ALL data = {NULL};
	UINT32 dbg_mode = vpe_dbg_get_dbg_mode(id);
	CTL_VPE_ISP_CB_PARAM *cb_param = (CTL_VPE_ISP_CB_PARAM *)param;

	if (id >= (UINT32)VPE_ID_MAX_NUM) {
		PRINT_VPE_WRN(dbg_mode & VPE_DBG_WRN_MSG, "vpe id out of range (%d) \r\n", id);
		return 0;
	}
	if (cb_param->vsp_frm_idx >= VPE_IDX_MAX_NUM) {
		PRINT_VPE_WRN(dbg_mode & VPE_DBG_WRN_MSG, "vpe idx out of range (%d) \r\n", cb_param->vsp_frm_idx);
		return 0;
	}

	vpe_in_size.w = cb_param->dce_in_size.w;
	vpe_in_size.h = cb_param->dce_in_size.h;
	vpe_out_size[id].w = cb_param->out_size.w;
	vpe_out_size[id].h = cb_param->out_size.h;

	data.p_dce_ctl = NULL;
	data.p_sharpen_param = NULL;
	data.p_dce_2dlut_param = NULL;
	data.p_yuv_cvt_param = NULL;
	data.p_dctg_ctl = NULL;
	data.p_flip_rot_ctl = NULL;

	if (evt & ISP_EVENT_VPE_CFG_IMM) {
		if(!vpe_id_valid[id][cb_param->vsp_frm_idx]) {
			PRINT_VPE_WRN(dbg_mode & VPE_DBG_WRN_MSG, "id list not valid (%d) (%d) \r\n", id, cb_param->vsp_frm_idx);
			return 0;
		}

		PRINT_VPE(dbg_mode & VPE_DBG_VPE_CB, "vpe cb, id(%d), idx(%d/%d) \r\n", id, cb_param->vsp_frm_idx, cb_param->vsp_frm_num);

		if (vpet_get_param_update(id, cb_param->vsp_frm_idx, VPET_ITEM_DCE_CTL_PARAM)) {
			if (cb_param->vsp_frm_num == 1) {
				vpet_set_param_update(id, cb_param->vsp_frm_idx, VPET_ITEM_DCE_CTL_PARAM, FALSE);
			}

			data.p_dce_ctl = &ctl_vpe_dce_ctl_init;
			vpe_set_dce_ctl_param(id, cb_param->vsp_frm_idx);
			if (vpe_param[id][cb_param->vsp_frm_idx]->dce_ctl->enable && (vpe_param[id][cb_param->vsp_frm_idx]->flip_rot->flip_rot_mode != VPE_ISP_ROTATE_0)) {
				PRINT_VPE_WRN(dbg_mode & VPE_DBG_WRN_MSG, "vpe dce is turned on, force flip_rot disable (%d) (%d/%d) \r\n", id, cb_param->vsp_frm_idx, cb_param->vsp_frm_num);
				vpe_param[id][cb_param->vsp_frm_idx]->flip_rot->flip_rot_mode = VPE_ISP_ROTATE_0;
				data.p_flip_rot_ctl = &ctl_vpe_flip_rot_init;
				vpe_set_flip_rot_param(id, cb_param->vsp_frm_idx);
			}
			if (vpe_param[id][cb_param->vsp_frm_idx]->dce_ctl->dce_mode == VPE_ISP_DCE_MODE_2DLUT_ONLY) {
				vpe_param[id][cb_param->vsp_frm_idx]->dctg->mode_sel = VPE_ISP_DCTG_MODE_DISABLE;
				data.p_dctg_ctl = &ctl_vpe_dctg_init;
				vpe_set_dctg_param(id, cb_param->vsp_frm_idx);
			}
			PRINT_VPE(dbg_mode & VPE_DBG_VPE_UPDATE, "vpe update, id(%d), idx(%d/%d), ctl \r\n", id, cb_param->vsp_frm_idx, cb_param->vsp_frm_num);
		}

		if (vpet_get_param_update(id, cb_param->vsp_frm_idx, VPET_ITEM_SHARPEN_PARAM)) {
			if (cb_param->vsp_frm_num == 1) {
				vpet_set_param_update(id, cb_param->vsp_frm_idx, VPET_ITEM_SHARPEN_PARAM, FALSE);
			}
			data.p_sharpen_param = &ctl_vpe_sharpen_init;
			vpe_set_sharpen_param(id, cb_param->vsp_frm_idx);
			PRINT_VPE(dbg_mode & VPE_DBG_VPE_UPDATE, "vpe update, id(%d), idx(%d/%d), sharpen \r\n", id, cb_param->vsp_frm_idx, cb_param->vsp_frm_num);
		}

		if ((vpet_get_param_update(id, cb_param->vsp_frm_idx, VPET_ITEM_2DLUT_PARAM)) && (vpe_param[id][cb_param->vsp_frm_idx]->dce_ctl->dce_mode == VPE_ISP_DCE_MODE_2DLUT_ONLY)) {
			if (cb_param->vsp_frm_num == 1) {
				vpet_set_param_update(id, cb_param->vsp_frm_idx, VPET_ITEM_2DLUT_PARAM, FALSE);
			}
			data.p_dce_2dlut_param = &ctl_vpe_2dlut_init;

			vpe_set_2dlut_param(id, cb_param->vsp_frm_idx);
			PRINT_VPE(dbg_mode & VPE_DBG_VPE_UPDATE, "vpe update, id(%d), idx(%d/%d), 2d lut \r\n", id, cb_param->vsp_frm_idx, cb_param->vsp_frm_num);
		}

		if ((vpet_get_param_update(id, cb_param->vsp_frm_idx, VPET_ITEM_2DLUT_EXPAND_PARAM)) && (vpe_param[id][cb_param->vsp_frm_idx]->dce_ctl->dce_mode == VPE_ISP_DCE_MODE_2DLUT_ONLY)) {
			if (cb_param->vsp_frm_num == 1) {
				vpet_set_param_update(id, cb_param->vsp_frm_idx, VPET_ITEM_2DLUT_EXPAND_PARAM, FALSE);
			}
			data.p_dce_2dlut_param = &ctl_vpe_2dlut_init;

			vpe_set_2dlut_expand_param(id, cb_param->vsp_frm_idx);
			PRINT_VPE(dbg_mode & VPE_DBG_VPE_UPDATE, "vpe update, id(%d), idx(%d/%d), 2d lut expand \r\n", id, cb_param->vsp_frm_idx, cb_param->vsp_frm_num);
		}

		if ((vpet_get_param_update(id, cb_param->vsp_frm_idx, VPET_ITEM_DCOUT_MODE_PARAM)) && (vpe_param[id][cb_param->vsp_frm_idx]->dce_ctl->dce_mode == VPE_ISP_DCE_MODE_2DLUT_ONLY)) {
			if (cb_param->vsp_frm_num == 1) {
				vpet_set_param_update(id, cb_param->vsp_frm_idx, VPET_ITEM_DCOUT_MODE_PARAM, FALSE);
			}
			data.p_dce_2dlut_param = &ctl_vpe_2dlut_init;

			vpe_set_dcout_mode_param(id, cb_param->vsp_frm_idx);
			PRINT_VPE(dbg_mode & VPE_DBG_VPE_UPDATE, "vpe update, id(%d), idx(%d/%d), dcout mode \r\n", id, cb_param->vsp_frm_idx, cb_param->vsp_frm_num);
		}

		if (vpet_get_param_update(id, cb_param->vsp_frm_idx, VPET_ITEM_DRT_PARAM)) {
			if (cb_param->vsp_frm_num == 1) {
				vpet_set_param_update(id, cb_param->vsp_frm_idx, VPET_ITEM_DRT_PARAM, FALSE);
			}
			data.p_yuv_cvt_param = &ctl_vpe_cvt_init;
			vpe_set_drt_param(id, cb_param->vsp_frm_idx);
			PRINT_VPE(dbg_mode & VPE_DBG_VPE_UPDATE, "vpe update, id(%d), idx(%d/%d), drt \r\n", id, cb_param->vsp_frm_idx, cb_param->vsp_frm_num);
		}

		if ((vpet_get_param_update(id, cb_param->vsp_frm_idx, VPET_ITEM_DCTG_CTRL)) && (vpe_param[id][cb_param->vsp_frm_idx]->dce_ctl->dce_mode == VPE_ISP_DCE_MODE_2DLUT_DCTG)) {
			if (cb_param->vsp_frm_num == 1) {
				vpet_set_param_update(id, cb_param->vsp_frm_idx, VPET_ITEM_DCTG_CTRL, FALSE);
			}

			vpe_dctg_output_size[id][cb_param->vsp_frm_idx].w = cb_param->dce_in_size.w;
			vpe_dctg_output_size[id][cb_param->vsp_frm_idx].h = cb_param->dce_in_size.h;
			data.p_dctg_ctl = &ctl_vpe_dctg_init;
			vpe_set_dctg_param(id, cb_param->vsp_frm_idx);
			PRINT_VPE(dbg_mode & VPE_DBG_VPE_UPDATE, "vpe update, id(%d), idx(%d/%d), dctg \r\n", id, cb_param->vsp_frm_idx, cb_param->vsp_frm_num);
		}

		if (vpet_get_param_update(id, cb_param->vsp_frm_idx, VPET_ITEM_FLIP_ROT_PARAM)) {
			if (cb_param->vsp_frm_num == 1) {
				vpet_set_param_update(id, cb_param->vsp_frm_idx, VPET_ITEM_FLIP_ROT_PARAM, FALSE);
			}

			data.p_flip_rot_ctl = &ctl_vpe_flip_rot_init;
			vpe_set_flip_rot_param(id, cb_param->vsp_frm_idx);
			if (vpe_param[id][cb_param->vsp_frm_idx]->dce_ctl->enable && (vpe_param[id][cb_param->vsp_frm_idx]->flip_rot->flip_rot_mode != VPE_ISP_ROTATE_0)) {
				PRINT_VPE_WRN(dbg_mode & VPE_DBG_WRN_MSG, "vpe flip_rot is turned on, force dce enable to 0 (%d) (%d/%d) \r\n", id, cb_param->vsp_frm_idx, cb_param->vsp_frm_num);
				vpe_param[id][cb_param->vsp_frm_idx]->dce_ctl->enable = FALSE;
				data.p_dce_ctl = &ctl_vpe_dce_ctl_init;
				vpe_set_dce_ctl_param(id, cb_param->vsp_frm_idx);
			}
			PRINT_VPE(dbg_mode & VPE_DBG_VPE_UPDATE, "vpe update, id(%d), idx(%d/%d), flip_rot \r\n", id, cb_param->vsp_frm_idx, cb_param->vsp_frm_num);
		}

		ctl_vpe_isp_set(id, CTL_VPE_ISP_FLOW_VPE, CTL_VPE_ISP_ITEM_IQ_PARAM, (CTL_VPE_ISP_IQ_ALL *)&data);
	}

	return 0;
}

void vpe_api_reg_if(UINT32 id_list, UINT32 idx_list, UINT32 idx_num, UINT32 _2dlut_size)
{
	ER ret = E_OK;
	UINT32 i, idx;
	#if defined(__KERNEL__)
	UINT32 predict_ioctl_buf;
	VPE_DEV_INFO *pdev_info = vpe_get_dev_info();

	if (pdev_info == NULL) {
		return;
	}

	// Assign buffer for ioctl usage
	if (vpe_2dlut_size >= VPE_ISP_2DLUT_SZ_MAX_NUM) {
		vpe_2dlut_size = VPE_ISP_2DLUT_SZ_257X257;
		DBG_WRN("vpe_2dlut_size > %d, force to %d \r\n", VPE_ISP_2DLUT_SZ_257X257, VPE_ISP_2DLUT_SZ_257X257);
	}

	real_2dlut_size = vpe_uti_calc_2dlut_tbl_size(vpe_2dlut_size);

	predict_ioctl_buf = vpe_uti_calc_2dlut_ioctl_size(vpe_2dlut_size);
	pdev_info->ioctl_buf = vpe_uti_vmem_alloc(predict_ioctl_buf);
	if (pdev_info->ioctl_buf == NULL) {
		DBG_WRN("allocate vpe ioctl_buf fail \r\n");
		return;
	}
	#else
	vpe_id_list = id_list;
	vpe_idx_list = idx_list;
	vpe_idx_num = idx_num;
	vpe_2dlut_size = _2dlut_size;
	#endif

	if (vpe_id_list == 0) {
		DBG_WRN("vpe_id_list = 0, force to 1 \r\n");
		vpe_id_list = 1;
	}

	if (vpe_idx_num == 0) {
		DBG_WRN("vpe_idx_num = 0, force to 1 \r\n");
		vpe_idx_num = 1;
	}

	// clean VPE module
	memset(&vpe_module, 0x0, sizeof(VPE_MODULE));

	// calculate buffer needed
	for (i = 0; i < VPE_ID_MAX_NUM; i++) {
		if ((vpe_id_list >> i) & 0x1) {
			vpe_id_valid[i][0] = TRUE;

			if ((vpe_idx_list >> i) & 0x1) {
				for (idx = 1; idx < vpe_idx_num; idx++) {
					vpe_id_valid[i][idx] = TRUE;
				}
			}
		}
	}

	for (i = 0; i < VPE_ID_MAX_NUM; i++) {
		if ((vpe_id_list >> i) & 0x1) {
			ret |= vpe_api_get_param_addr(i);
		}
	}

	// registed callback function
	if (ret == E_OK) {
		ctl_vpe_isp_evt_fp_reg(VPE_IF_REG_NAME, &vpe_api_cb_flow, VPE_IF_REG_EVENT, CTL_VPE_ISP_CB_MSG_NONE);
	} else {
		DBG_WRN("construct fail, no callback registration \r\n");
	}
}

void vpe_api_unreg_if(void)
{
	UINT32 i, idx;
	#if defined(__KERNEL__)
	VPE_DEV_INFO *pdev_info = vpe_get_dev_info();

	if (pdev_info == NULL) {
		return;
	}
	
	vpe_uti_vmem_free(pdev_info->ioctl_buf);
	pdev_info->ioctl_buf = NULL;
	#endif

	vpe_uti_kmem_free((ULONG *)param_phy_2dlut_addr);
	param_phy_2dlut_addr = 0;

	for (i = 0; i < VPE_ID_MAX_NUM; i++) {
		if ((vpe_id_list >> i) & 0x1) {
			for (idx = 0; idx < vpe_idx_num; idx++) {
				if (vpe_param_memalloc_valid[i][idx]) {
					vpe_uti_kmem_free((ULONG *)vpe_param[i][idx]->lut2d_int->lut_addr);
					vpe_param[i][idx]->lut2d_int->lut_addr = 0;

					vpe_uti_vmem_free(vpe_param_memalloc_addr[i][idx].dce_ctl);
					vpe_param_memalloc_addr[i][idx].dce_ctl = NULL;
					vpe_param_memalloc_addr[i][idx].sharpen = NULL;
					vpe_param_memalloc_addr[i][idx].lut2d_int = NULL;
					vpe_param_memalloc_addr[i][idx].lut2d_expand = NULL;
					vpe_param_memalloc_addr[i][idx].drt = NULL;
					vpe_param_memalloc_addr[i][idx].dctg = NULL;
					vpe_param_memalloc_valid[i][idx] = FALSE;
				}
			}
		}
	}

	// un-registed callback function
	ctl_vpe_isp_evt_fp_unreg(VPE_IF_REG_NAME);
}

