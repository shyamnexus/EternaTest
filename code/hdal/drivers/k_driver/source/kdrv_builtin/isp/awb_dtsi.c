#if defined(__KERNEL__)
#else
#include <stdio.h>
#endif
#include <libfdt.h>
#include <fdtfast.h>

#include "kwrap/debug.h"
#include "kwrap/type.h"
#include "unlzma.h"
#include "awb_dtsi.h"
#include "awb_param.h"
#include "isp_builtin_int.h"
#include "awb_builtin_alg.h"

//=============================================================================
// global
//=============================================================================
#define AWB_DTSI_ITEM_MAX_NUM sizeof(AWB_PARAM)/4

static AWB_DTSI awb_dtsi[AWB_DTSI_ITEM_MAX_NUM] = {
	{"awb_th",                  sizeof(AWB_TH)            },
	{"awb_lv",                  sizeof(AWB_LV)            },
	{"awb_ct_weight",           sizeof(AWB_CT_WEIGHT)     },
	{"awb_target",              sizeof(AWB_TARGET)        },
	{"awb_ct_info",             sizeof(AWB_CT_INFO)       },
	{"awb_mwb",                 sizeof(AWB_MWB_GAIN)      },      // 5
	{"awb_converge",            sizeof(AWB_CONVERGE)      },
	{"awb_expand_block",        sizeof(AWB_EXPAND_BLOCK)  },
	{"awb_luma_weight",         sizeof(AWB_LUMA_WEIGHT)   },
};

//=============================================================================
// internal functions
//=============================================================================
static void awb_dtsi_sub_load(CHAR *node_path, UINT8 *param_ptr, UINT32 index)
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

	sprintf(sub_node_name, "%s/%s", node_path, awb_dtsi[index].sub_node_name);
	node_ofst = fdt_path_offset(pfdt_addr, (CHAR *)&sub_node_name);
	if (node_ofst >= 0) {
		pfdt_node = fdt_getprop(pfdt_addr, node_ofst, "size", (int *)&data_size);
		if ((pfdt_node != NULL) && (data_size != 0)) {
			size = *(UINT32 *)pfdt_node;
			if (size != awb_dtsi[index].size) {
				DBG_DUMP("%s/size mismatch (dtsi:%d, sdk:%d) \r\n", sub_node_name, size, awb_dtsi[index].size);
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
				cmp_prt[5] = awb_dtsi[index].size & 0xFF;
				cmp_prt[6] = (awb_dtsi[index].size >> 8) & 0xFF;
				cmp_prt[7] = (awb_dtsi[index].size >> 16) & 0xFF;
				cmp_prt[8] = (awb_dtsi[index].size >> 24) & 0xFF;

				pfdt_node = fdt_getprop(pfdt_addr, node_ofst, "data", (int *)&data_size);
				if ((pfdt_node != NULL) && (data_size != 0)) {
					memcpy(&cmp_prt[13], pfdt_node, cmp_size);
				} else {
					DBG_DUMP("cannot find %s/data \r\n", sub_node_name);
				}

				lzma_inflate(cmp_prt, cmp_size + 13, param_ptr, (UINT32)awb_dtsi[index].size);
				isp_builtin_vmem_free(cmp_prt);
				cmp_prt = NULL;
			} else {
				DBG_DUMP("allocate cmp_prt fail \n");
			}
		} else {
			pfdt_node = fdt_getprop(pfdt_addr, node_ofst, "data", (int *)&data_size);
			if ((pfdt_node != NULL) && (data_size != 0)) {
				memcpy(param_ptr, pfdt_node, awb_dtsi[index].size);
			} else {
				DBG_DUMP("cannot find %s/data \r\n", sub_node_name);
			}
		}
	} else {
		//DBG_DUMP("cannot find %s \r\n", sub_node_name);
	}
}

//=============================================================================
// external functions
//=============================================================================
void awb_dtsi_load(UINT32 id, void *param)
{
	AWB_PARAM *awb_param = (AWB_PARAM *)param;
	CHAR node_path[SUB_NODE_LENGTH];
	ISP_BUILTIN_DTSI *isp_builtin_dtsi = NULL;

	#if (!ISP_BUILTIN_RELOAD_DTSI)
	return;
	#endif

	if (id < ISP_BUILTIN_DTSI_MAX) {
		isp_builtin_dtsi = isp_builtin_get_dtsi(id);
		if (isp_builtin_dtsi != NULL) {
			sprintf(node_path, isp_builtin_dtsi->awb_node_path);
		} else {
			DBG_DUMP("awb_dtsi_load, id = %d, isp_builtin_dtsi = NULL \r\n", id);
		}
	} else {
		DBG_DUMP("awb_dtsi_load, id = %d >= %d \r\n", id, ISP_BUILTIN_DTSI_MAX);
		return;
	}

	#if (ISP_BUILTIN_MEASURE_DTSI)
	nvt_bootts_add_ts("dtsi_awb");
	#endif

	// NOTE: th
	awb_dtsi_sub_load(node_path, (UINT8 *)awb_param->th, 0);

	// NOTE: lv
	awb_dtsi_sub_load(node_path, (UINT8 *)awb_param->lv, 1);

	// NOTE: ct_weight
	awb_dtsi_sub_load(node_path, (UINT8 *)awb_param->ct_weight, 2);

	// NOTE: target
	awb_dtsi_sub_load(node_path, (UINT8 *)awb_param->target, 3);

	// NOTE: ct_info
	awb_dtsi_sub_load(node_path, (UINT8 *)awb_param->ct_info, 4);

	// NOTE: mwb_gain
	awb_dtsi_sub_load(node_path, (UINT8 *)awb_param->mwb_gain, 5);

	// NOTE: converge
	awb_dtsi_sub_load(node_path, (UINT8 *)awb_param->converge, 6);

	// NOTE: expand_block
	awb_dtsi_sub_load(node_path, (UINT8 *)awb_param->expand_block, 7);

	// NOTE: luma_weight
	awb_dtsi_sub_load(node_path, (UINT8 *)awb_param->luma_weight, 8);

	#if (ISP_BUILTIN_MEASURE_DTSI)
	nvt_bootts_add_ts("dtsi_awb");
	#endif
}

