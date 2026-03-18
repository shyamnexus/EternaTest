#if defined(__FREERTOS)
#include <string.h>
#include <stdio.h>
#endif

#include "iq_alg_int.h"
#include "iq_flow.h"
#include "iq_dbg.h"

//=============================================================================
// global
//=============================================================================

//=============================================================================
// function declaration
//=============================================================================

//=============================================================================
// internal functions
//=============================================================================

//=============================================================================
// extern functions
//=============================================================================
void iq_flow_ext_set_sie_ob_manual(IQ_ID id, UINT32 enable, UINT32 value)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->ob_mode_manual.sie_enable = enable;
		iq_info->ob_mode_manual.sie_value = value;

		PRINT_IQ(TRUE, "set iq_info(%d) enable = %d value = %d \r\n", id, enable, value);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid \r\n", id);
	}
}

void iq_flow_ext_set_pre_f_ob_manual(IQ_ID id, UINT32 enable, UINT32 *value)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->ob_mode_manual.pre_f_enable = enable;
		iq_info->ob_mode_manual.pre_f_value[0] = *(value);
		iq_info->ob_mode_manual.pre_f_value[1] = *(value + 1);
		iq_info->ob_mode_manual.pre_f_value[2] = *(value + 2);
		iq_info->ob_mode_manual.pre_f_value[3] = *(value + 3);
		iq_info->ob_mode_manual.pre_f_value[4] = *(value + 4);

		PRINT_IQ(TRUE, "set iq_info(%d) enable = %d value = %d %d %d %d %d \r\n", id, enable, *value, *(value + 1), *(value + 2), *(value + 3), *(value + 4));
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid \r\n", id);
	}
}

void iq_flow_ext_set_pre_ob_manual(IQ_ID id, UINT32 enable, UINT32 *value)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->ob_mode_manual.pre_enable = enable;
		iq_info->ob_mode_manual.pre_value[0] = *(value);
		iq_info->ob_mode_manual.pre_value[1] = *(value + 1);
		iq_info->ob_mode_manual.pre_value[2] = *(value + 2);
		iq_info->ob_mode_manual.pre_value[3] = *(value + 3);
		iq_info->ob_mode_manual.pre_value[4] = *(value + 4);

		PRINT_IQ(TRUE, "set iq_info(%d) enable = %d value = %d %d %d %d %d \r\n", id, enable, *value, *(value + 1), *(value + 2), *(value + 3), *(value + 4));
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid \r\n", id);
	}
}

void iq_flow_ext_set_ife_f_ob_manual(IQ_ID id, UINT32 enable, UINT32 *value)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->ob_mode_manual.ife_f_enable = enable;
		iq_info->ob_mode_manual.ife_f_value[0] = *(value);
		iq_info->ob_mode_manual.ife_f_value[1] = *(value + 1);
		iq_info->ob_mode_manual.ife_f_value[2] = *(value + 2);
		iq_info->ob_mode_manual.ife_f_value[3] = *(value + 3);
		iq_info->ob_mode_manual.ife_f_value[4] = *(value + 4);

		PRINT_IQ(TRUE, "set iq_info(%d) enable = %d value = %d %d %d %d %d \r\n", id, enable, *value, *(value + 1), *(value + 2), *(value + 3), *(value + 4));
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid \r\n", id);
	}
}

void iq_flow_ext_set_ife_ob_manual(IQ_ID id, UINT32 enable, UINT32 *value)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->ob_mode_manual.ife_enable = enable;
		iq_info->ob_mode_manual.ife_value[0] = *(value);
		iq_info->ob_mode_manual.ife_value[1] = *(value + 1);
		iq_info->ob_mode_manual.ife_value[2] = *(value + 2);
		iq_info->ob_mode_manual.ife_value[3] = *(value + 3);
		iq_info->ob_mode_manual.ife_value[4] = *(value + 4);

		PRINT_IQ(TRUE, "set iq_info(%d) enable = %d value = %d %d %d %d %d \r\n", id, enable, *value, *(value + 1), *(value + 2), *(value + 3), *(value + 4));
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid \r\n", id);
	}
}

void iq_flow_ext_set_ob_mode_manual(IQ_ID id, UINT32 enable, UINT32 mode)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		if (mode == IQ_OB_IFE_F) {
			iq_info->ob_mode_manual.manual_enable = FALSE;
			iq_info->ob_mode_manual.manual_mode = IQ_OB_IFE;

			PRINT_IQ(TRUE, "manual_mode illegal, set iq_info(%d) enable = %d manual_mode = %d \r\n", id, iq_info->ob_mode_manual.manual_enable, iq_info->ob_mode_manual.manual_mode);
		} else {
			iq_info->ob_mode_manual.manual_enable = enable;
			iq_info->ob_mode_manual.manual_mode = mode;

			PRINT_IQ(TRUE, "set iq_info(%d) enable = %d ob_manual_mode = %d \r\n", id, enable, mode);
		}

	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid \r\n", id);
	}
}

void iq_flow_ext_set_sie_dg_manual(IQ_ID id, UINT32 enable, UINT32 value)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->dg_mode_manual.sie_enable = enable;
		iq_info->dg_mode_manual.sie_value = value;

		PRINT_IQ(TRUE, "set iq_info(%d) enable = %d value = %d \r\n", id, enable, value);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid \r\n", id);
	}
}

void iq_flow_ext_set_pre_dg_manual(IQ_ID id, UINT32 enable, UINT32 value)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->dg_mode_manual.pre_enable = enable;
		iq_info->dg_mode_manual.pre_value = value;

		PRINT_IQ(TRUE, "set iq_info(%d) enable = %d value = %d \r\n", id, enable, value);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid \r\n", id);
	}
}

void iq_flow_ext_set_ife_dg_manual(IQ_ID id, UINT32 enable, UINT32 value)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->dg_mode_manual.ife_enable = enable;
		iq_info->dg_mode_manual.ife_value = value;

		PRINT_IQ(TRUE, "set iq_info(%d) enable = %d value = %d \r\n", id, enable, value);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid \r\n", id);
	}
}

void iq_flow_ext_set_dg_mode_manual(IQ_ID id, UINT32 enable, UINT32 mode)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		if ((mode != IQ_DG_SIE) && (mode != IQ_DG_PRE) && (mode != IQ_DG_IFE) && (mode != IQ_DG_AI)) {
			iq_info->dg_mode_manual.manual_enable = FALSE;
			iq_info->dg_mode_manual.manual_mode = IQ_DG_SIE;

			PRINT_IQ(TRUE, "manual_mode illegal, set iq_info(%d) enable = %d manual_mode = %d \r\n", id, iq_info->dg_mode_manual.manual_enable, iq_info->dg_mode_manual.manual_mode);
		} else {
			iq_info->dg_mode_manual.manual_enable = enable;
			iq_info->dg_mode_manual.manual_mode = mode;

			PRINT_IQ(TRUE, "set iq_info(%d) enable = %d dg_manual_mode = %d \r\n", id, enable, mode);
		}
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid \r\n", id);
	}
}

void iq_flow_ext_set_sie_dg_max(IQ_ID id, UINT32 sie_dg_max)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {

			iq_info->dg_mode_manual.sie_dg_max = sie_dg_max;

			PRINT_IQ(TRUE, "set iq_info(%d) dg_max = %d \r\n", id, sie_dg_max);

	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid \r\n", id);
	}
}

void iq_flow_ext_set_cg_mode_manual(IQ_ID id, UINT32 enable, UINT32 mode)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		if ((mode != IQ_CG_SIE) && (mode != IQ_CG_PRE_F) && (mode != IQ_CG_PRE) && (mode != IQ_CG_IFE)) {
			iq_info->cg_mode_manual.manual_enable = FALSE;
			iq_info->cg_mode_manual.manual_mode = IQ_CG_IFE;

			PRINT_IQ(TRUE, "manual_mode illegal, set iq_info(%d) enable = %d manual_mode = %d \r\n", id, iq_info->cg_mode_manual.manual_enable, iq_info->cg_mode_manual.manual_mode);
		} else {
			iq_info->cg_mode_manual.manual_enable = enable;
			iq_info->cg_mode_manual.manual_mode = mode;

			PRINT_IQ(TRUE, "set iq_info(%d) enable = %d cg_manual_mode = %d \r\n", id, enable, mode);
		}
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid \r\n", id);
	}
}

void iq_flow_ext_set_dg_ai(IQ_ID id, UINT32 enable)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->dg_ai_enable = enable;
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid \r\n", id);
	}
}

void iq_flow_ext_set_nr_size(IQ_ID id, UINT32 size)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		#if 0 // TODO:
		iq_info->final_ipp.ife_filter.spatial.ife_filt_mode = size;
		#endif
		PRINT_IQ(TRUE, "set iq_info(%d) ife_filter.spatial.ife_filt_mode = %d\r\n", id, size);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid!! \r\n", id);
	}
}

void iq_flow_ext_set_irsub(IQ_ID id, UINT32 lb, UINT32 th, UINT32 rng)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->final_ipp.ipe_cfa.cfa_ir_sub.ir_sub_wt_lb = lb;
		iq_info->final_ipp.ipe_cfa.cfa_ir_sub.ir_sub_th = th;
		iq_info->final_ipp.ipe_cfa.cfa_ir_sub.ir_sub_range = rng;
		PRINT_IQ(TRUE, "set iq_info(%d) ipe_cfa.cfa_ir_sub.ir_sub_wt_lb = %d\r\n", id, lb);
		PRINT_IQ(TRUE, "set iq_info(%d) ipe_cfa.cfa_ir_sub.ir_sub_wt_lb = %d\r\n", id, th);
		PRINT_IQ(TRUE, "set iq_info(%d) ipe_cfa.cfa_ir_sub.ir_sub_range = %d\r\n", id, rng);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid!! \r\n", id);
	}
}

void iq_flow_ext_set_pink_red(IQ_ID id, BOOL en, UINT32 mode, UINT32 th1, UINT32 th2)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->final_ipp.ipe_cfa.cfa_pink_reduc.pink_rd_en = en;
		iq_info->final_ipp.ipe_cfa.cfa_pink_reduc.pink_rd_mode = mode;
		iq_info->final_ipp.ipe_cfa.cfa_pink_reduc.pink_rd_th1 = th1;
		iq_info->final_ipp.ipe_cfa.cfa_pink_reduc.pink_rd_th2 = th2;
		PRINT_IQ(TRUE, "set iq_info(%d) ipe_cfa.cfa_pink_reduc.pink_rd_en = %d\r\n", id, en);
		PRINT_IQ(TRUE, "set iq_info(%d) ipe_cfa.cfa_pink_reduc.pink_rd_mode = %d\r\n", id, mode);
		PRINT_IQ(TRUE, "set iq_info(%d) ipe_cfa.cfa_pink_reduc.pink_rd_th1 = %d\r\n", id, th1);
		PRINT_IQ(TRUE, "set iq_info(%d) ipe_cfa.cfa_pink_reduc.pink_rd_th2 = %d\r\n", id, th2);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid!! \r\n", id);
	}
}

void iq_flow_ext_set_wdr_subimg_lpf(IQ_ID id, UINT32 coef_0, UINT32 coef_1, UINT32 coef_2)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->final_ipp.ife_wdr.ftrcoef[0] = coef_0;
		iq_info->final_ipp.ife_wdr.ftrcoef[1] = coef_1;
		iq_info->final_ipp.ife_wdr.ftrcoef[2] = coef_2;
		PRINT_IQ(TRUE, "set iq_info(%d) ife_wdr.ftrcoef = %d, %d, %d\r\n", id, coef_0, coef_1, coef_2);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid!! \r\n", id);
	}
}

void iq_flow_ext_set_wdr_blend_w(IQ_ID id, UINT32 blend_w)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->final_ipp.ife_wdr.input_bld.in_yv_blend_lut[0] = blend_w;
		iq_info->final_ipp.ife_wdr.input_bld.in_yv_blend_lut[1] = blend_w;
		iq_info->final_ipp.ife_wdr.input_bld.in_yv_blend_lut[2] = blend_w;
		iq_info->final_ipp.ife_wdr.input_bld.in_yv_blend_lut[3] = blend_w;
		iq_info->final_ipp.ife_wdr.input_bld.in_yv_blend_lut[4] = blend_w;
		iq_info->final_ipp.ife_wdr.input_bld.in_yv_blend_lut[5] = blend_w;
		iq_info->final_ipp.ife_wdr.input_bld.in_yv_blend_lut[6] = blend_w;
		iq_info->final_ipp.ife_wdr.input_bld.in_yv_blend_lut[7] = blend_w;
		iq_info->final_ipp.ife_wdr.input_bld.in_yv_blend_lut[8] = blend_w;
		PRINT_IQ(TRUE, "set iq_info(%d) ife_wdr.input_bld.in_yv_blend_lut[0] = %d\r\n", id, blend_w);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid!! \r\n", id);
	}
}

void iq_flow_ext_set_rgblpf(IQ_ID id, UINT32 en, UINT32 th0, UINT32 th1)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->final_ipp.ipe_rgblpf.enable = en;
		iq_info->final_ipp.ipe_rgblpf.lpf_param_r.range_th0 = th0;
		iq_info->final_ipp.ipe_rgblpf.lpf_param_r.range_th1 = th1;
		iq_info->final_ipp.ipe_rgblpf.lpf_param_g.range_th0 = th0;
		iq_info->final_ipp.ipe_rgblpf.lpf_param_g.range_th1 = th1;
		iq_info->final_ipp.ipe_rgblpf.lpf_param_b.range_th0 = th0;
		iq_info->final_ipp.ipe_rgblpf.lpf_param_b.range_th1 = th1;
		PRINT_IQ(TRUE, "set iq_info(%d) ipe_rgblpf = %d\r\n", id, en);
		PRINT_IQ(TRUE, "set iq_info(%d) ipe_rgblpf.lpf_param_r = %d, %d\r\n", id, th0, th1);
		PRINT_IQ(TRUE, "set iq_info(%d) ipe_rgblpf.lpf_param_g = %d, %d\r\n", id, th0, th1);
		PRINT_IQ(TRUE, "set iq_info(%d) ipe_rgblpf.lpf_param_b = %d, %d\r\n", id, th0, th1);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid!! \r\n", id);
	}
}

void iq_flow_ext_set_ipe_subimg_size(IQ_ID id, UINT32 width, UINT32 height)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->final_ipp.ipe_subimg.subimg_size.h_size = IQ_CLAMP(width, 5, 32);
		iq_info->final_ipp.ipe_subimg.subimg_size.v_size = IQ_CLAMP(height, 5, 32);
		PRINT_IQ(TRUE, "set iq_info(%d) ipe_subimg.subimg_size = h(%d) v(%d)\r\n", id, width, height);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid!! \r\n", id);
	}
}

void iq_flow_ext_set_edge_gamma_sel(IQ_ID id, UINT32 sel)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->final_ipp.ipe_eext_tonemap.gamma_sel = sel;
		PRINT_IQ(TRUE, "set iq_info(%d) ipe_eext_tonemap.gamma_sel = %d\r\n", id, sel);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid!! \r\n", id);
	}
}

void iq_flow_ext_set_defog_min_diff(IQ_ID id, UINT32 min_diff)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->final_ipp.ipe_defog.env_estimation.dfg_min_diff = min_diff;
		PRINT_IQ(TRUE, "set iq_info(%d) ipe_defog.env_estimation.dfg_min_diff =%d\r\n", id, min_diff);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid!! \r\n", id);
	}
}

void iq_flow_ext_set_edge_region_str(IQ_ID id, UINT32 enable, UINT32 enh_thin, UINT32 enh_robust)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->final_ipp.ipe_edge_region_str.enable = enable;
		iq_info->final_ipp.ipe_edge_region_str.enh_thin = enh_thin;
		iq_info->final_ipp.ipe_edge_region_str.enh_robust = enh_robust;
		PRINT_IQ(TRUE, "set iq_info(%d) ipe_edge_region_str = (%d) (%d, %d)\r\n", id, enable, enh_thin, enh_robust);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid!! \r\n", id);
	}
}

void iq_flow_ext_set_edge_overshoot_w(IQ_ID id, UINT32 overshoot_w, UINT32 undershoot_w)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->final_ipp.ipe_edge_overshoot.wt_overshoot = overshoot_w;
		iq_info->final_ipp.ipe_edge_overshoot.wt_undershoot = undershoot_w;
		PRINT_IQ(TRUE, "set iq_info(%d) ipe_edge_overshoot = (%d, %d)\r\n", id, overshoot_w, undershoot_w);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid!! \r\n", id);
	}
}

void iq_flow_ext_set_edge_dir_th_shift(IQ_ID id, UINT32 th_shift)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->final_ipp.ipe_eext.dir_ker_para.eext_dir_s_th_shift = th_shift;
		PRINT_IQ(TRUE, "set iq_info(%d) ipe_eext.dir_ker_para.eext_dir_s_th_shift = %d \r\n", id, th_shift);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid!! \r\n", id);
	}
}

void iq_flow_ext_set_dbcs_mode(IQ_ID id, UINT32 mode)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->final_ipp.ime_dbcs.op_mode = mode;
		PRINT_IQ(TRUE, "set iq_info(%d) ime_dbcs.op_mode = %d\r\n", id, mode);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid!! \r\n", id);
	}
}

void iq_flow_ext_set_dbcs_y_wt(IQ_ID id, UINT32 wt0, UINT32 wt1, UINT32 wt2, UINT32 wt3, UINT32 wt4, UINT32 wt5  \
										, UINT32 wt6, UINT32 wt7, UINT32 wt8, UINT32 wt9, UINT32 wt10           \
										, UINT32 wt11, UINT32 wt12, UINT32 wt13, UINT32 wt14, UINT32 wt15)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->final_ipp.ime_dbcs.wt_y[0] = wt0;
		iq_info->final_ipp.ime_dbcs.wt_y[1] = wt1;
		iq_info->final_ipp.ime_dbcs.wt_y[2] = wt2;
		iq_info->final_ipp.ime_dbcs.wt_y[3] = wt3;
		iq_info->final_ipp.ime_dbcs.wt_y[4] = wt4;
		iq_info->final_ipp.ime_dbcs.wt_y[5] = wt5;
		iq_info->final_ipp.ime_dbcs.wt_y[6] = wt6;
		iq_info->final_ipp.ime_dbcs.wt_y[7] = wt7;
		iq_info->final_ipp.ime_dbcs.wt_y[8] = wt8;
		iq_info->final_ipp.ime_dbcs.wt_y[9] = wt9;
		iq_info->final_ipp.ime_dbcs.wt_y[10] = wt10;
		iq_info->final_ipp.ime_dbcs.wt_y[11] = wt11;
		iq_info->final_ipp.ime_dbcs.wt_y[12] = wt12;
		iq_info->final_ipp.ime_dbcs.wt_y[13] = wt13;
		iq_info->final_ipp.ime_dbcs.wt_y[14] = wt14;
		iq_info->final_ipp.ime_dbcs.wt_y[15] = wt15;
		PRINT_IQ(TRUE, "set iq_info(%d) ime_dbcs.wt_y = (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)\r\n"
			, id, wt0, wt1, wt2, wt3, wt4, wt5, wt6, wt7, wt8, wt9, wt10, wt11, wt12, wt13, wt14, wt15);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid!! \r\n", id);
	}
}

void iq_flow_ext_set_dbcs_c_wt(IQ_ID id, UINT32 wt0, UINT32 wt1, UINT32 wt2, UINT32 wt3, UINT32 wt4, UINT32 wt5  \
										, UINT32 wt6, UINT32 wt7, UINT32 wt8, UINT32 wt9, UINT32 wt10           \
										, UINT32 wt11, UINT32 wt12, UINT32 wt13, UINT32 wt14, UINT32 wt15)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->final_ipp.ime_dbcs.wt_c[0] = wt0;
		iq_info->final_ipp.ime_dbcs.wt_c[1] = wt1;
		iq_info->final_ipp.ime_dbcs.wt_c[2] = wt2;
		iq_info->final_ipp.ime_dbcs.wt_c[3] = wt3;
		iq_info->final_ipp.ime_dbcs.wt_c[4] = wt4;
		iq_info->final_ipp.ime_dbcs.wt_c[5] = wt5;
		iq_info->final_ipp.ime_dbcs.wt_c[6] = wt6;
		iq_info->final_ipp.ime_dbcs.wt_c[7] = wt7;
		iq_info->final_ipp.ime_dbcs.wt_c[8] = wt8;
		iq_info->final_ipp.ime_dbcs.wt_c[9] = wt9;
		iq_info->final_ipp.ime_dbcs.wt_c[10] = wt10;
		iq_info->final_ipp.ime_dbcs.wt_c[11] = wt11;
		iq_info->final_ipp.ime_dbcs.wt_c[12] = wt12;
		iq_info->final_ipp.ime_dbcs.wt_c[13] = wt13;
		iq_info->final_ipp.ime_dbcs.wt_c[14] = wt14;
		iq_info->final_ipp.ime_dbcs.wt_c[15] = wt15;
		PRINT_IQ(TRUE, "set iq_info(%d) ime_dbcs.wt_c = (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)\r\n"
			, id, wt0, wt1, wt2, wt3, wt4, wt5, wt6, wt7, wt8, wt9, wt10, wt11, wt12, wt13, wt14, wt15);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid!! \r\n", id);
	}
}

void iq_flow_ext_set_lca_location(IQ_ID id, UINT32 location)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->final_ipp.ime_lca.set_proc_location = location;
		PRINT_IQ(TRUE, "set iq_info(%d) ime_lca.set_proc_location = %d \r\n", id, location);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid!! \r\n", id);
	}
}

void iq_flow_ext_set_lca_edge_ker_size(IQ_ID id, UINT32 ker_size)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->final_ipp.ime_lca.rf.edge_ker_size_sel = ker_size;
		PRINT_IQ(TRUE, "set iq_info(%d) ime_lca.final_edge_ker_size_sel = %d \r\n", id, ker_size);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid!! \r\n", id);
	}
}

void iq_flow_ext_set_lca_y_out(IQ_ID id, UINT32 y_out)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->final_ipp.ime_lca.final_y_out_wt = y_out;
		PRINT_IQ(TRUE, "set iq_info(%d) ime_lca.final_y_out_wt = %d \r\n", id, y_out);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid!! \r\n", id);
	}
}

void iq_flow_ext_set_lca_c_out(IQ_ID id, UINT32 c_out)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->final_ipp.ime_lca.final_uv_out_wt = c_out;
		PRINT_IQ(TRUE, "set iq_info(%d) ime_lca.final_uv_out_wt = %d \r\n", id, c_out);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid!! \r\n", id);
	}
}

void iq_flow_ext_set_post_sharpen_con_eng(IQ_ID id, UINT32 con_eng)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));

	if (iq_flow_get_id_valid(id)) {
		iq_info->final_enc.post_sharpen.ucConEng = con_eng;
		PRINT_IQ(TRUE, "set iq_info(%d) post_sharpen.ucConEng = %d \r\n", id, con_eng);
	} else {
		PRINT_IQ(TRUE, "set iq_info(%d) is not valid!! \r\n", id);
	}
}

