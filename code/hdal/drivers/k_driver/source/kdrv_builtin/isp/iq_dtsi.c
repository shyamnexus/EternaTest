#if defined(__KERNEL__)
#else
#include <compiler.h>
#include <stdio.h>
#endif
#include <libfdt.h>
#include <fdtfast.h>

#include "kwrap/debug.h"
#include "kwrap/type.h"
#include "unlzma.h"
#include "iq_dtsi.h"
#include "iq_param.h"

#include "iq_builtin.h"

#include "isp_builtin_int.h"

#if (NVT_FAST_ISP_FLOW)
#define FDT_NEW_METHOD 1

//=============================================================================
// global
//=============================================================================
static IQ_DTSI iq_dtsi[IQ_DTSI_ITEM_MAX_NUM] = {
	{"iq_ob",                   sizeof(IQ_OB_PARAM),              NULL},  // 00
	{"iq_nr",                   sizeof(IQ_NR_PARAM),              NULL},
	{"iq_cfa",                  sizeof(IQ_CFA_PARAM),             NULL},
	{"iq_raw_va",               sizeof(IQ_RAW_VA_PARAM),          NULL},
	{"iq_va",                   sizeof(IQ_VA_PARAM),              NULL},
	{"iq_tone",                 sizeof(IQ_TONE_PARAM),            NULL},  // 05
	{"iq_gamma",                sizeof(IQ_GAMMA_PARAM),           NULL},
	{"iq_ccm",                  sizeof(IQ_CCM_PARAM),             NULL},
	{"iq_color",                sizeof(IQ_COLOR_PARAM),           NULL},
	{"iq_contrast",             sizeof(IQ_CONTRAST_PARAM),        NULL},
	{"iq_edge",                 sizeof(IQ_EDGE_PARAM),            NULL},  // 10
	{"iq_3dnr",                 sizeof(IQ_3DNR_PARAM),            NULL},
	{"iq_pfr",                  sizeof(IQ_PFR_PARAM),             NULL},
	{"iq_wdr",                  sizeof(IQ_WDR_PARAM),             NULL},
	{"iq_wdr_enh",              sizeof(IQ_WDR_ENH_PARAM),         NULL},
	{"iq_defog",                sizeof(IQ_DEFOG_PARAM),           NULL},  // 15
	{"iq_shdr",                 sizeof(IQ_SHDR_PARAM),            NULL},
	{"iq_companding",           sizeof(IQ_COMPANDING_PARAM),      NULL},
	{"iq_rgbir",                sizeof(IQ_RGBIR_PARAM),           NULL},
	{"iq_rgbir_enh",            sizeof(IQ_RGBIR_ENH_PARAM),       NULL},
	{"iq_post_sharpen_1",       sizeof(IQ_POST_SHARPEN_1_PARAM),  NULL},  // 20
	{"iq_post_sharpen_2",       sizeof(IQ_POST_SHARPEN_2_PARAM),  NULL},
	{"iq_ycurve",               sizeof(IQ_YCURVE_PARAM),          NULL},
	{"iq_cst",                  sizeof(IQ_CST_PARAM),             NULL},
	{"iq_fpn",                  sizeof(IQ_FPN_PARAM),             NULL},
	// depend on module parameter v
	{"iq_dpc",                  sizeof(IQ_DPC_PARAM),             NULL},  // 25
	{"iq_expand_dpc",           sizeof(IQ_EXPAND_DPC_PARAM),      NULL},
	{"iq_shading",              sizeof(IQ_SHADING_PARAM),         NULL},
	{"iq_shading_ext",          sizeof(IQ_SHADING_EXT_PARAM),     NULL},
	// depend on module parameter ^
};

//=============================================================================
// internal functions
//=============================================================================
#if (FDT_NEW_METHOD)
static void iq_dtsi_sub_load(UINT8 *pfdt_addr, INT32 node_ofst, UINT32 *index)
{
	const void *pfdt_node;
	UINT32 size = 0, cmp_size = 0;
	INT32 data_size, len, node;
	UINT8 *cmp_prt = NULL;

	fdt_for_each_subnode(node, pfdt_addr, node_ofst) {
		const char *name = fdt_get_name(pfdt_addr, node, &len);
		//DBG_DUMP("%20s, index %2d, size %4d \r\n", name, *index, iq_dtsi[*index].size);
		pfdt_node = fdt_getprop(pfdt_addr, node, "size", (int *)&data_size);
		if ((pfdt_node != NULL) && (data_size != 0)) {
			size = *(UINT32 *)pfdt_node;
			if (size != iq_dtsi[*index].size) {
				DBG_DUMP("%s/size mismatch (dtsi:%d, sdk:%d) \r\n", name, size, iq_dtsi[*index].size);
				goto subnode_loop;
			}
		} else {
			DBG_DUMP("cannot find %s/size \r\n", name);
			goto subnode_loop;;
		}

		pfdt_node = fdt_getprop(pfdt_addr, node, "cmp_size", (int *)&data_size);
		if ((pfdt_node != NULL) && (data_size != 0)) {
			cmp_size = *(UINT32 *)pfdt_node;
		} else {
			cmp_size = 0;
		}

		if (cmp_size != 0) {
			cmp_prt = isp_builtin_vmem_alloc(ALIGN_CEIL(cmp_size + 13, 4));
			if (cmp_prt != NULL) {
				cmp_prt[0] = 0x5D;
				cmp_prt[4] = 0x1;
				cmp_prt[5] = iq_dtsi[*index].size & 0xFF;
				cmp_prt[6] = (iq_dtsi[*index].size >> 8) & 0xFF;
				cmp_prt[7] = (iq_dtsi[*index].size >> 16) & 0xFF;
				cmp_prt[8] = (iq_dtsi[*index].size >> 24) & 0xFF;

				pfdt_node = fdt_getprop(pfdt_addr, node, "data", (int *)&data_size);
				if ((pfdt_node != NULL) && (data_size != 0)) {
					memcpy(&cmp_prt[13], pfdt_node, cmp_size);
				} else {
					DBG_DUMP("cannot find %s/data \r\n", name);
				}

				lzma_inflate(cmp_prt, cmp_size + 13, iq_dtsi[*index].param, (UINT32)iq_dtsi[*index].size);
				isp_builtin_vmem_free(cmp_prt);
				cmp_prt = NULL;
			} else {
				DBG_DUMP("allocate cmp_prt fail \n");
			}
		} else {
			pfdt_node = fdt_getprop(pfdt_addr, node, "data", (int *)&data_size);
			if ((pfdt_node != NULL) && (data_size != 0)) {
				memcpy(iq_dtsi[*index].param, pfdt_node, iq_dtsi[*index].size);
			} else {
				DBG_DUMP("cannot find %s/data \r\n", name);
			}
		}

		subnode_loop:
		*index += 1;
	}
}
#else
static void iq_dtsi_sub_load(CHAR *node_path, UINT8 *param_ptr, UINT32 index)
{
	UINT8 *pfdt_addr = (unsigned char *)fdtfast_get_base();
	CHAR sub_node_name[SUB_NODE_LENGTH];
	const void *pfdt_node;
	UINT32 size = 0, cmp_size = 0;
	INT32 node_ofst = 0, data_size;
	UINT8 *cmp_prt = NULL;

	if (pfdt_addr == NULL) {
		DBG_DUMP("fdtfast_get_base fail \r\n");
		return;
	}

	sprintf(sub_node_name, "%s/%s", node_path, iq_dtsi[index].sub_node_name);
	node_ofst = fdt_path_offset(pfdt_addr, (CHAR *)&sub_node_name);
	if (node_ofst >= 0) {
		pfdt_node = fdt_getprop(pfdt_addr, node_ofst, "size", (int *)&data_size);
		if ((pfdt_node != NULL) && (data_size != 0)) {
			size = *(UINT32 *)pfdt_node;
			if (size != iq_dtsi[index].size) {
				DBG_DUMP("%s/size mismatch (dtsi:%d, sdk:%d) \r\n", sub_node_name, size, iq_dtsi[index].size);
				return;
			}
		} else {
			DBG_DUMP("cannot find %s/size \r\n", sub_node_name);
			return;
		}

		pfdt_node = fdt_getprop(pfdt_addr, node_ofst, "cmp_size", (int *)&data_size);
		if ((pfdt_node != NULL) && (data_size != 0)) {
			cmp_size = *(UINT32 *)pfdt_node;
		} else {
			cmp_size = 0;
		}

		if (cmp_size != 0) {
			cmp_prt = isp_builtin_vmem_alloc(ALIGN_CEIL(cmp_size + 13, 4));
			if (cmp_prt != NULL) {
				cmp_prt[0] = 0x5D;
				cmp_prt[4] = 0x1;
				cmp_prt[5] = iq_dtsi[index].size & 0xFF;
				cmp_prt[6] = (iq_dtsi[index].size >> 8) & 0xFF;
				cmp_prt[7] = (iq_dtsi[index].size >> 16) & 0xFF;
				cmp_prt[8] = (iq_dtsi[index].size >> 24) & 0xFF;

				pfdt_node = fdt_getprop(pfdt_addr, node_ofst, "data", (int *)&data_size);
				if ((pfdt_node != NULL) && (data_size != 0)) {
					memcpy(&cmp_prt[13], pfdt_node, cmp_size);
				} else {
					DBG_DUMP("cannot find %s/data \r\n", sub_node_name);
				}

				lzma_inflate(cmp_prt, cmp_size + 13, param_ptr, (UINT32)iq_dtsi[index].size);
				isp_builtin_vmem_free(cmp_prt);
				cmp_prt = NULL;
			} else {
				DBG_DUMP("allocate cmp_prt fail \n");
			}
		} else {
			pfdt_node = fdt_getprop(pfdt_addr, node_ofst, "data", (int *)&data_size);
			if ((pfdt_node != NULL) && (data_size != 0)) {
				memcpy(param_ptr, pfdt_node, iq_dtsi[index].size);
			} else {
				DBG_DUMP("cannot find %s/data \r\n", sub_node_name);
			}
		}
	} else {
		DBG_DUMP("cannot find %s \r\n", sub_node_name);
	}
}
#endif

static void iq_builtin_front_param_sub_load(UINT8 *pfdt_addr, INT32 node_ofst, UINT32 *param_ptr, CHAR *prop_name, INT32 prop_size)
{
	const void *pfdt_node;
	INT32 data_size;
	INT32 num, i;

	pfdt_node = fdt_getprop(pfdt_addr, node_ofst, prop_name, (int *)&data_size);
	if ((pfdt_node != NULL) && (data_size != 0)) {
		if (data_size != prop_size) {
			DBG_DUMP("%s size mismatch (dtsi:%d, sdk:%d) \r\n", prop_name, data_size, prop_size);
		}
		num = data_size/sizeof(UINT32);
		for (i = 0; i < num; i++) {
			*(param_ptr + i) = be32_to_cpu(*((unsigned int *)pfdt_node + i));
		}
	} else {
		DBG_DUMP("cannot find prop %s \r\n", prop_name);
	}
}

//=============================================================================
// external functions
//=============================================================================
void iq_dtsi_load(UINT32 id, void *param)
{
	IQ_PARAM_PTR *iq_param = (IQ_PARAM_PTR *)param;
	CHAR node_path[SUB_NODE_LENGTH];
	CHAR node_path2[SUB_NODE_LENGTH];
	CHAR node_path3[SUB_NODE_LENGTH];
	ISP_BUILTIN_DTSI *isp_builtin_dtsi = NULL;
	#if (FDT_NEW_METHOD)
	UINT32 index;
	UINT8 *pfdt_addr = NULL;
	INT32 node_ofst, node_ofst2, node_ofst3;
	#endif

	#if defined(__KERNEL__)
	#if (ISP_BUILTIN_MEASURE_DTSI)
	nvt_bootts_add_ts("dtsi_iq");
	#endif
	#endif

	if (id < ISP_BUILTIN_DTSI_MAX) {
		isp_builtin_dtsi = isp_builtin_get_dtsi(id);
		if (isp_builtin_dtsi != NULL) {
			sprintf(node_path, isp_builtin_dtsi->iq_node_path);
			sprintf(node_path2, isp_builtin_dtsi->iq_dpc_node_path);
			sprintf(node_path3, isp_builtin_dtsi->iq_shading_node_path);
		} else {
			DBG_DUMP("iq_dtsi_load, id = %d, isp_builtin_dtsi = NULL \r\n", id);
		}
	} else {
		DBG_DUMP("iq_dtsi_load, id = %d >= %d \r\n", id, ISP_BUILTIN_DTSI_MAX);
		return;
	}

	#if (FDT_NEW_METHOD)
	iq_dtsi[IQ_DTSI_ITEM_OB_PARAM].param =               (UINT8*)iq_param->ob;
	iq_dtsi[IQ_DTSI_ITEM_NR_PARAM].param =               (UINT8*)iq_param->nr;
	iq_dtsi[IQ_DTSI_ITEM_CFA_PARAM].param =              (UINT8*)iq_param->cfa;
	iq_dtsi[IQ_DTSI_ITEM_RAW_VA_PARAM].param =           (UINT8*)iq_param->raw_va;
	iq_dtsi[IQ_DTSI_ITEM_VA_PARAM].param =               (UINT8*)iq_param->va;
	iq_dtsi[IQ_DTSI_ITEM_TONE_PARAM].param =             (UINT8*)iq_param->tone;
	iq_dtsi[IQ_DTSI_ITEM_GAMMA_PARAM].param =            (UINT8*)iq_param->gamma;
	iq_dtsi[IQ_DTSI_ITEM_CCM_PARAM].param =              (UINT8*)iq_param->ccm;
	iq_dtsi[IQ_DTSI_ITEM_COLOR_PARAM].param =            (UINT8*)iq_param->color;
	iq_dtsi[IQ_DTSI_ITEM_CONTRAST_PARAM].param =         (UINT8*)iq_param->contrast;
	iq_dtsi[IQ_DTSI_ITEM_EDGE_PARAM].param =             (UINT8*)iq_param->edge;
	iq_dtsi[IQ_DTSI_ITEM_3DNR_PARAM].param =             (UINT8*)iq_param->_3dnr;
	iq_dtsi[IQ_DTSI_ITEM_PFR_PARAM].param =              (UINT8*)iq_param->pfr;
	iq_dtsi[IQ_DTSI_ITEM_WDR_PARAM].param =              (UINT8*)iq_param->wdr;
	iq_dtsi[IQ_DTSI_ITEM_WDR_ENH_PARAM].param =          (UINT8*)iq_param->wdr_enh;
	iq_dtsi[IQ_DTSI_ITEM_DEFOG_PARAM].param =            (UINT8*)iq_param->defog;
	iq_dtsi[IQ_DTSI_ITEM_SHDR_PARAM].param =             (UINT8*)iq_param->shdr;
	iq_dtsi[IQ_DTSI_ITEM_COMPANDING_PARAM].param =       (UINT8*)iq_param->companding;
	iq_dtsi[IQ_DTSI_ITEM_RGBIR_PARAM].param =            (UINT8*)iq_param->rgbir;
	iq_dtsi[IQ_DTSI_ITEM_RGBIR_ENH_PARAM].param =        (UINT8*)iq_param->rgbir_enh;
	iq_dtsi[IQ_DTSI_ITEM_POST_SHARPEN_1_PARAM].param =   (UINT8*)iq_param->post_sharpen_1;
	iq_dtsi[IQ_DTSI_ITEM_POST_SHARPEN_2_PARAM].param =   (UINT8*)iq_param->post_sharpen_2;
	iq_dtsi[IQ_DTSI_ITEM_YCURVE_PARAM].param =           (UINT8*)iq_param->ycurve;
	iq_dtsi[IQ_DTSI_ITEM_CST_PARAM].param =              (UINT8*)iq_param->cst;
	iq_dtsi[IQ_DTSI_ITEM_FPN_PARAM].param =              (UINT8*)iq_param->fpn;
	iq_dtsi[IQ_DTSI_ITEM_DPC_PARAM].param =              (UINT8*)iq_param->dpc;
	iq_dtsi[IQ_DTSI_ITEM_SHADING_PARAM].param =          (UINT8*)iq_param->shading;
	iq_dtsi[IQ_DTSI_ITEM_SHADING_EXT_PARAM].param =      (UINT8*)iq_param->shading_ext;

	pfdt_addr = (unsigned char *)fdtfast_get_base();
	if (pfdt_addr == NULL) {
		DBG_DUMP("fdtfast_get_base fail \r\n");
		return;
	}

	node_ofst = fdt_path_offset(pfdt_addr, (CHAR *)&node_path);
	if (node_ofst >= 0) {
		index = IQ_DTSI_ITEM_OB_PARAM;
		iq_dtsi_sub_load(pfdt_addr, node_ofst, &index);
	} else {
		DBG_DUMP("cannot find %s \r\n", node_path);
	}

	if (strncmp(node_path2, "NULL", 4) != 0) {
		node_ofst2 = fdt_path_offset(pfdt_addr, (CHAR *)&node_path2);
		if (node_ofst2 >= 0) {
			index = IQ_DTSI_ITEM_DPC_PARAM;
			iq_dtsi_sub_load(pfdt_addr, node_ofst2, &index);
		} else {
			DBG_DUMP("cannot find %s \r\n", node_path2);
		}
	}

	if (strncmp(node_path3, "NULL", 4) != 0) {
		node_ofst3 = fdt_path_offset(pfdt_addr, (CHAR *)&node_path3);
		if (node_ofst3 >= 0) {
			index = IQ_DTSI_ITEM_SHADING_PARAM;
			iq_dtsi_sub_load(pfdt_addr, node_ofst3, &index);
		} else {
			DBG_DUMP("cannot find %s \r\n", node_path3);
		}
	}

	#else

	// NOTE: OB
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->ob, IQ_DTSI_ITEM_OB_PARAM);

	// NOTE: NR
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->nr, IQ_DTSI_ITEM_NR_PARAM);

	// NOTE: CFA
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->cfa, IQ_DTSI_ITEM_CFA_PARAM);

	// NOTE: VA
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->va, IQ_DTSI_ITEM_VA_PARAM);

	// NOTE: GAMMA
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->gamma, IQ_DTSI_ITEM_GAMMA_PARAM);

	// NOTE: CCM
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->ccm, IQ_DTSI_ITEM_CCM_PARAM);

	// NOTE: COLOR
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->color, IQ_DTSI_ITEM_COLOR_PARAM);

	// NOTE: CONTRAST
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->contrast, IQ_DTSI_ITEM_CONTRAST_PARAM);

	// NOTE: EDGE
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->edge, IQ_DTSI_ITEM_EDGE_PARAM);

	// NOTE: 3DNR
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->_3dnr, IQ_DTSI_ITEM_3DNR_PARAM);

	// NOTE: PFR
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->pfr, IQ_DTSI_ITEM_PFR_PARAM);

	// NOTE: WDR
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->wdr, IQ_DTSI_ITEM_WDR_PARAM);

	// NOTE: DEFOG
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->defog, IQ_DTSI_ITEM_DEFOG_PARAM);

	// NOTE: SHDR
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->shdr, IQ_DTSI_ITEM_SHDR_PARAM);

	// NOTE: COMPANDING
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->companding, IQ_DTSI_ITEM_COMPANDING_PARAM);

	// NOTE: RGBIR
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->rgbir, IQ_DTSI_ITEM_RGBIR_PARAM);

	// NOTE: RGBIR ENH
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->rgbir_enh, IQ_DTSI_ITEM_RGBIR_ENH_PARAM);

	// NOTE: POST SHARPEN 2
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->post_sharpen_2, IQ_DTSI_ITEM_POST_SHARPEN_2_PARAM);

	// NOTE: DPC
	if (strncmp(node_path2, "NULL", 4) != 0) {
		iq_dtsi_sub_load(node_path2, (UINT8 *)iq_param->dpc, IQ_DTSI_ITEM_DPC_PARAM);
	}

	// NOTE: EXPAND DPC
	// not support at builtin

	// NOTE: SHADING
	if (strncmp(node_path3, "NULL", 4) != 0) {
		iq_dtsi_sub_load(node_path3, (UINT8 *)iq_param->shading, IQ_DTSI_ITEM_SHADING_PARAM);
	}

	// NOTE: SHADING EXT
	if (strncmp(node_path3, "NULL", 4) != 0) {
		iq_dtsi_sub_load(node_path3, (UINT8 *)iq_param->shading_ext, IQ_DTSI_ITEM_SHADING_EXT_PARAM);
	}

	// NOTE: YCURVE
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->ycurve, IQ_DTSI_ITEM_YCURVE_PARAM);

	// NOTE: CST
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->cst, IQ_DTSI_ITEM_CST_PARAM);

	// NOTE: POST SHARPEN 1
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->post_sharpen_1, IQ_DTSI_ITEM_POST_SHARPEN_1_PARAM);

	// NOTE: TONE
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->tone, IQ_DTSI_ITEM_TONE_PARAM);

	// NOTE: WDR ENH
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->wdr_enh, IQ_DTSI_ITEM_WDR_ENH_PARAM);

	// NOTE: RAW VA
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->raw_va, IQ_DTSI_ITEM_RAW_VA_PARAM);

	// NOTE: FPN
	iq_dtsi_sub_load(node_path, (UINT8 *)iq_param->fpn, IQ_DTSI_ITEM_FPN_PARAM);
	#endif

	#if defined(__KERNEL__)
	#if (ISP_BUILTIN_MEASURE_DTSI)
	nvt_bootts_add_ts("dtsi_iq");
	#endif
	#endif
}

void iq_builtin_front_dtsi_load(UINT32 id, void *param)
{
	ISP_BUILTIN_DTSI *isp_builtin_dtsi = NULL;
	CHAR node_path[SUB_NODE_LENGTH];
	IQ_BUILTIN_FRONT_PARAM *iq_front_param = (IQ_BUILTIN_FRONT_PARAM *)param;
	UINT8 *pfdt_addr = NULL;
	INT32 node_ofst;

	if (id < ISP_BUILTIN_DTSI_MAX) {
		isp_builtin_dtsi = isp_builtin_get_dtsi(id);
		if (isp_builtin_dtsi != NULL) {
			sprintf(node_path, isp_builtin_dtsi->iq_front_node_path);
		} else {
			DBG_DUMP("iq_builtin_front_dtsi_load, id = %d, isp_builtin_dtsi = NULL \r\n", id);
		}
	} else {
		DBG_DUMP("iq_builtin_front_dtsi_load, id = %d >= %d \r\n", id, ISP_BUILTIN_DTSI_MAX);
		return;
	}

	pfdt_addr = (unsigned char *)fdtfast_get_base();
	if (pfdt_addr == NULL) {
		DBG_DUMP("fdtfast_get_base fail \r\n");
		return;
	}

	node_ofst = fdt_path_offset(pfdt_addr, (CHAR *)&node_path);
	if (node_ofst < 0) {
		DBG_DUMP("cannot find node %s \r\n", node_path);
		return;
	}

	iq_builtin_front_param_sub_load(pfdt_addr, node_ofst, (UINT32 *)&iq_front_param->smooth_thr, "smooth_frm", sizeof(iq_front_param->smooth_thr));

	iq_builtin_front_param_sub_load(pfdt_addr, node_ofst, (UINT32 *)&iq_front_param->iso_w, "iso_w", sizeof(iq_front_param->iso_w));

	iq_builtin_front_param_sub_load(pfdt_addr, node_ofst, (UINT32 *)&iq_front_param->rth_nlm, "rth_nlm", sizeof(iq_front_param->rth_nlm));

	iq_builtin_front_param_sub_load(pfdt_addr, node_ofst, (UINT32 *)&iq_front_param->rth_nlm_lut, "rth_nlm_lut", sizeof(iq_front_param->rth_nlm_lut));

	iq_builtin_front_param_sub_load(pfdt_addr, node_ofst, (UINT32 *)&iq_front_param->clamp_th, "clamp_th", sizeof(iq_front_param->clamp_th));

	iq_builtin_front_param_sub_load(pfdt_addr, node_ofst, (UINT32 *)&iq_front_param->c_con, "saturation", sizeof(iq_front_param->c_con));

	iq_builtin_front_param_sub_load(pfdt_addr, node_ofst, (UINT32 *)&iq_front_param->overshoot, "edge_enh", sizeof(iq_front_param->overshoot));

	#if 0//IQ_BUILTIN_DBG_MSG
	DBG_DUMP("================== id %d ================== \r\n", id);
	DBG_DUMP("smooth_thr ========== %d\r\n", iq_front_param->smooth_thr);
	DBG_DUMP("iso_w =============== %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\r\n",
											iq_front_param->iso_w[IQ_GAIN_1X],
											iq_front_param->iso_w[IQ_GAIN_2X],
											iq_front_param->iso_w[IQ_GAIN_4X],
											iq_front_param->iso_w[IQ_GAIN_8X],
											iq_front_param->iso_w[IQ_GAIN_16X],
											iq_front_param->iso_w[IQ_GAIN_32X],
											iq_front_param->iso_w[IQ_GAIN_64X],
											iq_front_param->iso_w[IQ_GAIN_128X],
											iq_front_param->iso_w[IQ_GAIN_256X],
											iq_front_param->iso_w[IQ_GAIN_512X],
											iq_front_param->iso_w[IQ_GAIN_1024X],
											iq_front_param->iso_w[IQ_GAIN_2048X],
											iq_front_param->iso_w[IQ_GAIN_4096X],
											iq_front_param->iso_w[IQ_GAIN_8192X],
											iq_front_param->iso_w[IQ_GAIN_16384X],
											iq_front_param->iso_w[IQ_GAIN_32768X]
											);
	DBG_DUMP("rth_nlm ============= %d\r\n", iq_front_param->rth_nlm);
	DBG_DUMP("rth_nlm_lut ========= %d\r\n", iq_front_param->rth_nlm_lut);
	DBG_DUMP("clamp_th ============ %d\r\n", iq_front_param->clamp_th);
	DBG_DUMP("c_con =============== %d\r\n", iq_front_param->c_con);
	DBG_DUMP("overshoot =========== %d\r\n", iq_front_param->overshoot);
	#endif
}

#endif

