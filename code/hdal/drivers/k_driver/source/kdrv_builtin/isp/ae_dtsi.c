#if defined(__KERNEL__)
#else
#include <stdio.h>
#endif
#include <libfdt.h>
#include <fdtfast.h>

#include "kwrap/debug.h"
#include "kwrap/type.h"
#include "unlzma.h"
#include "ae_dtsi.h"
#include "ae_param.h"
#include "isp_builtin_int.h"

//=============================================================================
// global
//=============================================================================
#define AE_DTSI_ITEM_MAX_NUM 12

static AE_DTSI ae_dtsi[AE_DTSI_ITEM_MAX_NUM] = {
	{"ae_expect_lum",           sizeof(AE_EXPECT_LUM)        },
	{"ae_la_clamp",             sizeof(AE_LA_CLAMP)          },
	{"ae_over_exposure",        sizeof(AE_OVER_EXPOSURE)     },
	{"ae_convergence",          sizeof(AE_CONVERGENCE)       },
	{"ae_curve_gen_movie",      sizeof(AE_CURVE_GEN_MOVIE)   },
	{"ae_meter_window",         sizeof(AE_METER_WINDOW)      },      // 5
	{"ae_lum_gamma",            sizeof(AE_LUM_GAMMA)         },
	{"ae_shdr",                 sizeof(AE_SHDR)              },
	{"ae_shdr_meter",           sizeof(AE_SHDR_METER)        },
	{"ae_shdr_hbs",             sizeof(AE_SHDR_HBS)          },
	{"ae_iris",                 sizeof(AE_IRIS_CFG)          },
	{"ae_curve_gen_photo",      sizeof(AE_CURVE_GEN_PHOTO)   }
};

//=============================================================================
// internal functions
//=============================================================================
static void ae_dtsi_sub_load(CHAR *node_path, UINT8 *param_ptr, UINT32 index)
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
	sprintf(sub_node_name, "%s/%s", node_path, ae_dtsi[index].sub_node_name);
	node_ofst = fdt_path_offset(pfdt_addr, (CHAR *)&sub_node_name);
	if (node_ofst >= 0) {
		pfdt_node = fdt_getprop(pfdt_addr, node_ofst, "size", (int *)&data_size);
		if ((pfdt_node != NULL) && (data_size != 0)) {
			size = *(UINT32 *)pfdt_node;
			if (size != ae_dtsi[index].size) {
				DBG_DUMP("%s/size mismatch (dtsi:%d, sdk:%d) \r\n", sub_node_name, size, ae_dtsi[index].size);
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
				cmp_prt[5] = ae_dtsi[index].size & 0xFF;
				cmp_prt[6] = (ae_dtsi[index].size >> 8) & 0xFF;
				cmp_prt[7] = (ae_dtsi[index].size >> 16) & 0xFF;
				cmp_prt[8] = (ae_dtsi[index].size >> 24) & 0xFF;

				pfdt_node = fdt_getprop(pfdt_addr, node_ofst, "data", (int *)&data_size);
				if ((pfdt_node != NULL) && (data_size != 0)) {
					memcpy(&cmp_prt[13], pfdt_node, cmp_size);
				} else {
					DBG_DUMP("cannot find %s/data \r\n", sub_node_name);
				}

				lzma_inflate(cmp_prt, cmp_size + 13, param_ptr, (UINT32)ae_dtsi[index].size);
				isp_builtin_vmem_free(cmp_prt);
				cmp_prt = NULL;
			} else {
				DBG_DUMP("allocate cmp_prt fail \n");
			}
		} else {
			pfdt_node = fdt_getprop(pfdt_addr, node_ofst, "data", (int *)&data_size);
			if ((pfdt_node != NULL) && (data_size != 0)) {
				memcpy(param_ptr, pfdt_node, ae_dtsi[index].size);
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
void ae_dtsi_load(UINT32 id, void *param)
{
	AE_PARAM *ae_param = (AE_PARAM *)param;
	CHAR node_path[SUB_NODE_LENGTH];
	ISP_BUILTIN_DTSI *isp_builtin_dtsi = NULL;

	#if (!ISP_BUILTIN_RELOAD_DTSI)
	return;
	#endif

	if (id < ISP_BUILTIN_DTSI_MAX) {
		isp_builtin_dtsi = isp_builtin_get_dtsi(id);
		if (isp_builtin_dtsi != NULL) {
			sprintf(node_path, isp_builtin_dtsi->ae_node_path);
		} else {
			DBG_DUMP("ae_dtsi_load, id = %d, isp_builtin_dtsi = NULL \r\n", id);
		}
	} else {
		DBG_DUMP("ae_dtsi_load, id = %d >= %d \r\n", id, ISP_BUILTIN_DTSI_MAX);
		return;
	}

	#if (ISP_BUILTIN_MEASURE_DTSI)
	nvt_bootts_add_ts("dtsi_ae");
	#endif

	// NOTE: expect_lum
	ae_dtsi_sub_load(node_path, (UINT8 *)ae_param->expect_lum, 0);

	// NOTE: la_clamp
	ae_dtsi_sub_load(node_path, (UINT8 *)ae_param->la_clamp, 1);

	// NOTE: over_exposure
	ae_dtsi_sub_load(node_path, (UINT8 *)ae_param->over_exposure, 2);

	// NOTE: convergence
	ae_dtsi_sub_load(node_path, (UINT8 *)ae_param->convergence, 3);

	// NOTE: curve_gen_movie
	ae_dtsi_sub_load(node_path, (UINT8 *)ae_param->curve_gen_movie, 4);

	// NOTE: meter_win
	ae_dtsi_sub_load(node_path, (UINT8 *)ae_param->meter_win, 5);

	// NOTE: lum_gamma
	ae_dtsi_sub_load(node_path, (UINT8 *)ae_param->lum_gamma, 6);

	// NOTE: shdr
	ae_dtsi_sub_load(node_path, (UINT8 *)ae_param->shdr, 7);

	// NOTE: shdr_meter
	ae_dtsi_sub_load(node_path, (UINT8 *)ae_param->shdr_meter, 8);

	// NOTE: shdr_hbs
	ae_dtsi_sub_load(node_path, (UINT8 *)ae_param->shdr_hbs, 9);

	// NOTE: iris_cfg
	ae_dtsi_sub_load(node_path, (UINT8 *)ae_param->iris_cfg, 10);

	// NOTE: curve_gen_photo
	ae_dtsi_sub_load(node_path, (UINT8 *)ae_param->curve_gen_photo, 11);

	#if (ISP_BUILTIN_MEASURE_DTSI)
	nvt_bootts_add_ts("dtsi_ae");
	#endif
}

