#if defined(__FREERTOS)
#include <stdio.h>
#endif
#include "libfdt.h"
#include "kwrap/file.h"
#include "kwrap/error_no.h"
#include "kwrap/type.h"
#include "isp_dev.h"
#include "isp_uti.h"

#include "iq_dbg.h"
#include "iqt_dtsi.h"
#include "iq_version.h"

static IQT_DTSI iqt_dtsi[IQT_DTSI_ITEM_MAX_NUM] = {
	{"iq_ob",                NULL,   sizeof(IQ_OB_PARAM)              },
	{"iq_nr",                NULL,   sizeof(IQ_NR_PARAM)              },
	{"iq_cfa",               NULL,   sizeof(IQ_CFA_PARAM)             },
	{"iq_va",                NULL,   sizeof(IQ_VA_PARAM)              },
	{"iq_gamma",             NULL,   sizeof(IQ_GAMMA_PARAM)           },
	{"iq_ccm",               NULL,   sizeof(IQ_CCM_PARAM)             },      // 5
	{"iq_color",             NULL,   sizeof(IQ_COLOR_PARAM)           },
	{"iq_contrast",          NULL,   sizeof(IQ_CONTRAST_PARAM)        },
	{"iq_edge",              NULL,   sizeof(IQ_EDGE_PARAM)            },
	{"iq_3dnr",              NULL,   sizeof(IQ_3DNR_PARAM)            },
	{"iq_pfr",               NULL,   sizeof(IQ_PFR_PARAM)             },      // 10
	{"iq_wdr",               NULL,   sizeof(IQ_WDR_PARAM)             },
	{"iq_defog",             NULL,   sizeof(IQ_DEFOG_PARAM)           },
	{"iq_shdr",              NULL,   sizeof(IQ_SHDR_PARAM)            },
	{"iq_companding",        NULL,   sizeof(IQ_COMPANDING_PARAM)      },
	{"iq_rgbir",             NULL,   sizeof(IQ_RGBIR_PARAM)           },      // 15
	{"iq_rgbir_enh",         NULL,   sizeof(IQ_RGBIR_ENH_PARAM)       },
	{"iq_post_sharpen_2",    NULL,   sizeof(IQ_POST_SHARPEN_2_PARAM)  },
	// depend on module parameter v
	{"iq_dpc",               NULL,   sizeof(IQ_DPC_PARAM)             },
	{"iq_expand_dpc",        NULL,   sizeof(IQ_EXPAND_DPC_PARAM)      },
	{"iq_shading",           NULL,   sizeof(IQ_SHADING_PARAM)         },      // 20
	{"iq_shading_ext",       NULL,   sizeof(IQ_SHADING_EXT_PARAM)     },
	// depend on module parameter ^
	{"iq_ycurve",            NULL,   sizeof(IQ_YCURVE_PARAM)          },
	{"iq_cst",               NULL,   sizeof(IQ_CST_PARAM)             },
	{"iq_post_sharpen_1",    NULL,   sizeof(IQ_POST_SHARPEN_1_PARAM)  },
	{"iq_tone",              NULL,   sizeof(IQ_TONE_PARAM)            },      // 25
	{"iq_wdr_enh",           NULL,   sizeof(IQ_WDR_ENH_PARAM)         },
	{"iq_raw_va",            NULL,   sizeof(IQ_RAW_VA_PARAM)          },
	{"iq_fpn",               NULL,   sizeof(IQ_FPN_PARAM)             },
	{"iq_3dcc",              NULL,   sizeof(IQ_3DCC_PARAM)            },
	{"iq_3dcc_ext",          NULL,   sizeof(IQ_3DCC_EXT_PARAM)        },      // 30
	{"iq_bnr",               NULL,   sizeof(IQ_BNR_PARAM)             },
};

ER iqt_dtsi_load(UINT8 *node_path, UINT8 *file_path, UINT8 *buf_addr, IQALG_INFO *iq_info, void *param)
{
	ER rt = E_OK;
	UINT8 *pfdt_addr = NULL;
	INT32 node_ofst = 0, data_size, param_size = 0;
	UINT32 *pdata32, module_version = 0, i;
	CHAR sub_node_name[SUB_NODE_LENGTH];
	const void *pfdt_node;
	IQ_PARAM_PTR *iq_param = (IQ_PARAM_PTR *)param;
	IQ_DPC_PARAM *dpc;
	IQ_EXPAND_DPC_PARAM *expand_dpc;
	IQ_SHADING_PARAM *shading;
	IQ_3DCC_PARAM *_3dcc;
	BOOL dbg_en = ((iq_dbg_get_dbg_mode(0) & IQ_DBG_DTS) ? TRUE : FALSE);
	#if defined(__KERNEL__)
	UINT32 read_size = 256*1024;
	VOS_FILE fp;

	pfdt_addr = isp_uti_vmem_alloc(read_size);
	if (pfdt_addr == NULL) {
		DBG_WRN("fail to allocate pfdt_addr. \r\n");
		isp_uti_vmem_free(pfdt_addr);
		pfdt_addr = NULL;
		return E_SYS;
	}

	fp = vos_file_open((CHAR *)file_path, O_RDONLY, 0x777);
	if (fp == (VOS_FILE)(-1)) {
		DBG_WRN("open %s fail. \r\n", file_path);
		isp_uti_vmem_free(pfdt_addr);
		pfdt_addr = NULL;
		return E_SYS;
	}

	if (vos_file_read(fp, pfdt_addr, read_size) < 1) {
		DBG_WRN("read %s fail. \r\n", file_path);
		vos_file_close(fp);
		isp_uti_vmem_free(pfdt_addr);
		pfdt_addr = NULL;
		return E_SYS;
	}

	vos_file_close(fp);
	#else
	pfdt_addr = buf_addr;
	if (pfdt_addr == NULL) {
		DBG_WRN("pfdt_addr is NULL. \r\n");
		return E_SYS;
	}
	#endif

	node_ofst = fdt_path_offset(pfdt_addr, (CHAR *)node_path);
	if (node_ofst < 0) {
		DBG_WRN("%s not available. \r\n", node_path);
		#if defined(__KERNEL__)
		isp_uti_vmem_free(pfdt_addr);
		pfdt_addr = NULL;
		#endif
		return E_SYS;
	}

	pfdt_node = fdt_getprop(pfdt_addr, node_ofst, "version-info", (int *)&data_size);
	if ((pfdt_node != NULL) && (data_size != 0)) {
		pdata32 = (UINT32 *)pfdt_node;
		module_version = iq_get_version();
		if (*pdata32 == module_version) {
			PRINT_IQ(dbg_en, "iq dtsi node_path: %s \r\n", node_path);
			PRINT_IQ(dbg_en, "iq dtsi version: 0x%x \r\n", *pdata32);
		} else {
			PRINT_IQ(dbg_en, "iq dtsi node_path: %s \r\n", node_path);
			DBG_WRN("iq dtsi version not match. (dtsi: 0x%X) (iq module: 0x%X) \r\n", *pdata32, module_version);
		}
	} else {
		PRINT_IQ(dbg_en, "%s version-info not available. \r\n", node_path);
	}

	for (i = 0; i < IQT_DTSI_ITEM_MAX_NUM; i++) {
		sprintf(sub_node_name, "%s/%s", node_path, iqt_dtsi[i].sub_node_name);
		node_ofst = fdt_path_offset(pfdt_addr, (CHAR *)&sub_node_name);

		if (node_ofst >= 0) {
			pfdt_node = fdt_getprop(pfdt_addr, node_ofst, "size", (int *)&data_size);
			if ((pfdt_node != NULL) && (data_size != 0)) {
				param_size = *(INT32 *)pfdt_node;
			} else {
				DBG_WRN("%s(%d) size not available. \r\n", sub_node_name, i);
				iqt_dtsi[i].pdata = NULL;
				continue;
			}
			pfdt_node = fdt_getprop(pfdt_addr, node_ofst, "data", (int *)&data_size);
			if ((pfdt_node != NULL) && (data_size == param_size) && ((i == IQT_DTSI_ITEM_EXPAND_DPC_PARAM) || (data_size == iqt_dtsi[i].size))) {
				iqt_dtsi[i].pdata = (UINT8 *)pfdt_node;
			} else {
				iqt_dtsi[i].pdata = NULL;
				DBG_WRN("%s(%d) size not match. (%d) (%d) (%d) \r\n", sub_node_name, i, data_size, param_size, iqt_dtsi[i].size);
			}
			PRINT_IQ(dbg_en, "%s checked ok!! \r\n", sub_node_name);
		} else {
			//DBG_WRN("%s not available. \r\n", sub_node_name);
			iqt_dtsi[i].pdata = NULL;
		}
	}

	if (iqt_dtsi[IQT_DTSI_ITEM_OB_PARAM].pdata != NULL) {
		memcpy(iq_param->ob, iqt_dtsi[IQT_DTSI_ITEM_OB_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_OB_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_NR_PARAM].pdata != NULL) {
		memcpy(iq_param->nr, iqt_dtsi[IQT_DTSI_ITEM_NR_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_NR_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_CFA_PARAM].pdata != NULL) {
		memcpy(iq_param->cfa, iqt_dtsi[IQT_DTSI_ITEM_CFA_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_CFA_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_VA_PARAM].pdata != NULL) {
		memcpy(iq_param->va, iqt_dtsi[IQT_DTSI_ITEM_VA_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_VA_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_GAMMA_PARAM].pdata != NULL) {
		memcpy(iq_param->gamma, iqt_dtsi[IQT_DTSI_ITEM_GAMMA_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_GAMMA_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_CCM_PARAM].pdata != NULL) {
		memcpy(iq_param->ccm, iqt_dtsi[IQT_DTSI_ITEM_CCM_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_CCM_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_COLOR_PARAM].pdata != NULL) {
		memcpy(iq_param->color, iqt_dtsi[IQT_DTSI_ITEM_COLOR_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_COLOR_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_CONTRAST_PARAM].pdata != NULL) {
		memcpy(iq_param->contrast, iqt_dtsi[IQT_DTSI_ITEM_CONTRAST_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_CONTRAST_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_EDGE_PARAM].pdata != NULL) {
		memcpy(iq_param->edge, iqt_dtsi[IQT_DTSI_ITEM_EDGE_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_EDGE_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_3DNR_PARAM].pdata != NULL) {
		memcpy(iq_param->_3dnr, iqt_dtsi[IQT_DTSI_ITEM_3DNR_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_3DNR_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_PFR_PARAM].pdata != NULL) {
		memcpy(iq_param->pfr, iqt_dtsi[IQT_DTSI_ITEM_PFR_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_PFR_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_WDR_PARAM].pdata != NULL) {
		memcpy(iq_param->wdr, iqt_dtsi[IQT_DTSI_ITEM_WDR_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_WDR_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_DEFOG_PARAM].pdata != NULL) {
		memcpy(iq_param->defog, iqt_dtsi[IQT_DTSI_ITEM_DEFOG_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_DEFOG_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_SHDR_PARAM].pdata != NULL) {
		memcpy(iq_param->shdr, iqt_dtsi[IQT_DTSI_ITEM_SHDR_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_SHDR_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_COMPANDING_PARAM].pdata != NULL) {
		memcpy(iq_param->companding, iqt_dtsi[IQT_DTSI_ITEM_COMPANDING_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_COMPANDING_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_RGBIR_PARAM].pdata != NULL) {
		memcpy(iq_param->rgbir, iqt_dtsi[IQT_DTSI_ITEM_RGBIR_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_RGBIR_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_RGBIR_ENH_PARAM].pdata != NULL) {
		memcpy(iq_param->rgbir_enh, iqt_dtsi[IQT_DTSI_ITEM_RGBIR_ENH_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_RGBIR_ENH_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_POST_SHARPEN_2_PARAM].pdata != NULL) {
		memcpy(iq_param->post_sharpen_2, iqt_dtsi[IQT_DTSI_ITEM_POST_SHARPEN_2_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_POST_SHARPEN_2_PARAM].size);
	}
	if ((iqt_dtsi[IQT_DTSI_ITEM_DPC_PARAM].pdata != NULL) && (iq_info->final_sie.sie_dpc != NULL)) {
		dpc = (IQ_DPC_PARAM *)iqt_dtsi[IQT_DTSI_ITEM_DPC_PARAM].pdata;
		iq_info->final_sie.sie_dpc->enable = dpc->enable;
		memcpy(iq_info->final_sie.sie_dpc->table, dpc->table, sizeof(UINT32) * IQ_DPC_MAX_NUM);
		iq_info->final_sie.sie_dpc->def_same_ch_only_en = dpc->def_same_ch_only_en;
	}
	if ((iqt_dtsi[IQT_DTSI_ITEM_EXPAND_DPC_PARAM].pdata != NULL) && (iq_info->final_sie.sie_dpc != NULL)) {
		expand_dpc = (IQ_EXPAND_DPC_PARAM *)iqt_dtsi[IQT_DTSI_ITEM_EXPAND_DPC_PARAM].pdata;
		iq_info->final_sie.sie_dpc->expand_en = expand_dpc->enable;
		if ((iq_info->final_sie.sie_dpc->expand_table_viraddr != 0) && (iq_info->final_sie.sie_dpc->dp_buffer_size >= expand_dpc->size)) {
			iq_info->final_sie.sie_dpc->dp_total_size = expand_dpc->size;
			memcpy((UINT32 *)iq_info->final_sie.sie_dpc->expand_table_viraddr, &expand_dpc->table_phyaddr, expand_dpc->size);
		} else {
			if (iq_info->final_sie.sie_dpc->dp_buffer_size < expand_dpc->size) {
				PRINT_IQ(dbg_en, "[DPC] dpc_expand size overflow!! buffer size %d, dpc_expand size %d \r\n", iq_info->final_sie.sie_dpc->dp_buffer_size, expand_dpc->size);
			}
		}
		if (iq_info->final_sie.sie_dpc->expand_table_viraddr == 0) {
			iq_info->final_sie.sie_dpc->expand_en = FALSE;
			iq_info->final_sie.sie_dpc->dp_total_size = IQ_DPC_MAX_NUM * sizeof(UINT32);
			PRINT_IQ(dbg_en, "[DPC] expand_table_viraddr is not available, expand_en force to 0 !! \r\n");
		}
		if (iq_info->final_sie.sie_dpc->expand_en == FALSE) {
			iq_info->final_sie.sie_dpc->dp_total_size = IQ_DPC_MAX_NUM * sizeof(UINT32);
		}
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_SHADING_PARAM].pdata != NULL) {
		shading = (IQ_SHADING_PARAM *)iqt_dtsi[IQT_DTSI_ITEM_SHADING_PARAM].pdata;
		iq_info->iq_ref_set.ecs_mode = shading->mode;
		iq_info->iq_ref_set.ecs_smooth_l_m_ct_lower = shading->ecs_smooth_l_m_ct_lower;
		iq_info->iq_ref_set.ecs_smooth_l_m_ct_upper = shading->ecs_smooth_l_m_ct_upper;
		iq_info->iq_ref_set.ecs_smooth_m_h_ct_lower = shading->ecs_smooth_m_h_ct_lower;
		iq_info->iq_ref_set.ecs_smooth_m_h_ct_upper = shading->ecs_smooth_m_h_ct_upper;
		if (iq_info->final_sie.sie_ecs != NULL) {
			iq_info->final_sie.sie_ecs->enable = shading->ecs_enable;
		}
		if (iq_info->iq_ref_set.ecs_ext != NULL) {
			memcpy(iq_info->iq_ref_set.ecs_ext->manual_ecs_tbl, shading->ecs_map_tbl, sizeof(UINT32) * IQ_SHADING_ECS_LEN);
		}
		iq_info->final_ipp.ife_vig.enable = shading->vig_enable;
		iq_info->final_ipp.ife_cent_ratio.ch0.x = shading->vig_center_x;
		iq_info->final_ipp.ife_cent_ratio.ch1.x = shading->vig_center_x;
		iq_info->final_ipp.ife_cent_ratio.ch2.x = shading->vig_center_x;
		iq_info->final_ipp.ife_cent_ratio.ch3.x = shading->vig_center_x;
		iq_info->final_ipp.ife_cent_ratio.ch0.y = shading->vig_center_y;
		iq_info->final_ipp.ife_cent_ratio.ch1.y = shading->vig_center_y;
		iq_info->final_ipp.ife_cent_ratio.ch2.y = shading->vig_center_y;
		iq_info->final_ipp.ife_cent_ratio.ch3.y = shading->vig_center_y;
		iq_info->iq_ref_set.vig_reduce_th = shading->vig_reduce_th;
		iq_info->iq_ref_set.vig_zero_th = shading->vig_zero_th;
		memcpy(iq_info->iq_ref_set.vig_lut, shading->vig_lut, sizeof(UINT16) * IQ_SHADING_VIG_LEN);
	}
	if ((iqt_dtsi[IQT_DTSI_ITEM_SHADING_EXT_PARAM].pdata != NULL) && (iq_info->iq_ref_set.ecs_ext != NULL)) {
		memcpy(&iq_info->iq_ref_set.ecs_ext->auto_tbl, iqt_dtsi[IQT_DTSI_ITEM_SHADING_EXT_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_SHADING_EXT_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_YCURVE_PARAM].pdata != NULL) {
		memcpy(iq_param->ycurve, iqt_dtsi[IQT_DTSI_ITEM_YCURVE_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_YCURVE_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_CST_PARAM].pdata != NULL) {
		memcpy(iq_param->cst, iqt_dtsi[IQT_DTSI_ITEM_CST_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_CST_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_POST_SHARPEN_1_PARAM].pdata != NULL) {
		memcpy(iq_param->post_sharpen_1, iqt_dtsi[IQT_DTSI_ITEM_POST_SHARPEN_1_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_POST_SHARPEN_1_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_TONE_PARAM].pdata != NULL) {
		memcpy(iq_param->tone, iqt_dtsi[IQT_DTSI_ITEM_TONE_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_TONE_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_WDR_ENH_PARAM].pdata != NULL) {
		memcpy(iq_param->wdr_enh, iqt_dtsi[IQT_DTSI_ITEM_WDR_ENH_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_WDR_ENH_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_RAW_VA_PARAM].pdata != NULL) {
		memcpy(iq_param->raw_va, iqt_dtsi[IQT_DTSI_ITEM_RAW_VA_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_RAW_VA_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_FPN_PARAM].pdata != NULL) {
		memcpy(iq_param->tone, iqt_dtsi[IQT_DTSI_ITEM_FPN_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_FPN_PARAM].size);
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_3DCC_PARAM].pdata != NULL) {
		_3dcc = (IQ_3DCC_PARAM *)iqt_dtsi[IQT_DTSI_ITEM_3DCC_PARAM].pdata;
		if (iq_info->final_ipp.ipe_3dcc != NULL) {
			iq_info->final_ipp.ipe_3dcc->enable = _3dcc->enable;
			iq_info->final_ipp.ipe_3dcc->mode = _3dcc->mode;
		}
		if (iq_info->iq_ref_set._3dcc_ext != NULL) {
			memcpy(iq_info->iq_ref_set._3dcc_ext->manual_3dcc_lut, _3dcc->manual_3dcc_lut, sizeof(UINT32) * IQ_3DCC_LEN);
		}
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_3DCC_EXT_PARAM].pdata != NULL) {
		if (iq_info->iq_ref_set._3dcc_ext != NULL) {
			memcpy(&iq_info->iq_ref_set._3dcc_ext->auto_lut, iqt_dtsi[IQT_DTSI_ITEM_3DCC_EXT_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_3DCC_EXT_PARAM].size);
		}
	}
	if (iqt_dtsi[IQT_DTSI_ITEM_BNR_PARAM].pdata != NULL) {
		memcpy(iq_param->bnr, iqt_dtsi[IQT_DTSI_ITEM_BNR_PARAM].pdata, iqt_dtsi[IQT_DTSI_ITEM_BNR_PARAM].size);
	}

	#if defined(__KERNEL__)
	isp_uti_vmem_free(pfdt_addr);
	//coverity[UNUSED_VALUE]
	pfdt_addr = NULL;
	#endif

	return rt;
}

