#if defined(__FREERTOS)
#include "string.h"
#endif
#include "kwrap/error_no.h"
#include "kwrap/type.h"
#include "kwrap/cpu.h"

#include "vpet_api.h"
#include "vpe_common_param_int.h"
#include "vpe_dbg.h"
#include "vpe_dev_int.h"
#include "vpe_main.h"
#include "vpe_uti.h"
#include "vpe_version.h"

//=============================================================================
// extern include
//=============================================================================
extern VPE_PARAM_PTR *vpe_param[VPE_ID_MAX_NUM][VPE_IDX_MAX_NUM];
extern BOOL vpe_id_valid[VPE_ID_MAX_NUM][VPE_IDX_MAX_NUM];
extern UINT32 vpe_2dlut_size;
extern UINT32 vpe_dcout_mode[VPE_ID_MAX_NUM];

//=============================================================================
// global
//=============================================================================
typedef void (*vpet_fp)(ULONG addr);
static UINT32 vpe_idx;
static BOOL vpe_param_update[VPE_ID_MAX_NUM][VPE_IDX_MAX_NUM][VPET_ITEM_MAX];
UINT32 real_2dlut_size = VPE_2DLUT_NUM * sizeof(UINT32);

//=============================================================================
// function declaration
//=============================================================================
static void vpet_api_get_version(ULONG addr);
static void vpet_api_get_size_tab(ULONG addr);
static void vpet_api_get_sharpen(ULONG addr);
static void vpet_api_get_dce_ctl(ULONG addr);
static void vpet_api_get_2dlut(ULONG addr);
static void vpet_api_get_drt(ULONG addr);
static void vpet_api_get_dctg(ULONG addr);
static void vpet_api_get_flip_rot(ULONG addr);
static void vpet_api_get_2dlut_expand(ULONG addr);
static void vpet_api_get_dcout_mode(ULONG addr);
static void vpet_api_set_sharpen(ULONG addr);
static void vpet_api_set_dce_ctl(ULONG addr);
static void vpet_api_set_2dlut(ULONG addr);
static void vpet_api_set_drt(ULONG addr);
static void vpet_api_set_dctg(ULONG addr);
static void vpet_api_set_flip_rot(ULONG addr);
static void vpet_api_set_2dlut_expand(ULONG addr);
static void vpet_api_set_dcout_mode(ULONG addr);
static void vpet_api_reserve(ULONG addr);

#define RESERVE_SIZE 0

static VPET_INFO vpet_info = { {
	//id                             size
	{VPET_ITEM_VERSION,              sizeof(UINT32)                   },
	{VPET_ITEM_SIZE_TAB,             sizeof(VPET_INFO)                },
	{VPET_ITEM_RESERVE_02,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_03,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_04,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_05,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_06,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_07,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_08,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_09,           RESERVE_SIZE                     },
	{VPET_ITEM_DCE_CTL_PARAM,        sizeof(VPET_DCE_CTL_PARAM)       },
	{VPET_ITEM_SHARPEN_PARAM,        sizeof(VPET_SHARPEN_PARAM)       },
	{VPET_ITEM_RESERVE_12,           RESERVE_SIZE                      },
	{VPET_ITEM_2DLUT_PARAM,          sizeof(VPET_2DLUT_PARAM)         },
	{VPET_ITEM_DRT_PARAM,            sizeof(VPET_DRT_PARAM)           },
	{VPET_ITEM_DCTG_CTRL ,           sizeof(VPET_DCTG_CTRL)           },
	{VPET_ITEM_FLIP_ROT_PARAM,       sizeof(VPET_FLIP_ROT_PARAM)      },
	{VPET_ITEM_2DLUT_EXPAND_PARAM,   sizeof(VPET_2DLUT_EXPAND_PARAM)  },
	{VPET_ITEM_DCOUT_MODE_PARAM,     sizeof(VPET_DCOUT_MODE_PARAM)    },
	{VPET_ITEM_RESERVE_19,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_20,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_21,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_22,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_23,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_24,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_25,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_26,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_27,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_28,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_29,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_30,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_31,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_32,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_33,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_34,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_35,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_36,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_37,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_38,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_39,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_40,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_41,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_42,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_43,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_44,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_45,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_46,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_47,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_48,           RESERVE_SIZE                     },
	{VPET_ITEM_RESERVE_49,           RESERVE_SIZE                     }
} };

static vpet_fp vpet_get_tab[VPET_ITEM_MAX] = {
	vpet_api_get_version,
	vpet_api_get_size_tab,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,               // 5
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_get_dce_ctl,           // 10
	vpet_api_get_sharpen,
	vpet_api_reserve,
	vpet_api_get_2dlut,
	vpet_api_get_drt,
	vpet_api_get_dctg,              // 15
	vpet_api_get_flip_rot,
	vpet_api_get_2dlut_expand,
	vpet_api_get_dcout_mode,
	vpet_api_reserve,
	vpet_api_reserve,               // 20
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,               // 25
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,               // 30
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,               // 35
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,               // 40
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,               // 45
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve
};

static vpet_fp vpet_set_tab[VPET_ITEM_MAX] = {
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,               // 5
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_set_dce_ctl,           // 10
	vpet_api_set_sharpen,
	vpet_api_reserve,
	vpet_api_set_2dlut,
	vpet_api_set_drt,
	vpet_api_set_dctg,              // 15
	vpet_api_set_flip_rot,
	vpet_api_set_2dlut_expand,
	vpet_api_set_dcout_mode,
	vpet_api_reserve,
	vpet_api_reserve,               // 20
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,               // 25
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,               // 30
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,               // 35
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,               // 40
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,               // 45
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve,
	vpet_api_reserve
};

UINT32 vpet_api_get_item_size(VPET_ITEM item)
{
	return vpet_info.size_tab[item][1];
}

static void vpet_api_get_version(ULONG addr)
{
	UINT32 *data = (UINT32 *)addr;

	*data = vpe_get_version();
}

static void vpet_api_get_size_tab(ULONG addr)
{
	memcpy((VPET_INFO *)addr, &vpet_info, sizeof(VPET_INFO));
}

static void vpet_api_get_dce_ctl(ULONG addr)
{
	VPET_DCE_CTL_PARAM *data = (VPET_DCE_CTL_PARAM *)addr;

	if (!vpe_id_valid[data->id][vpe_idx]) {
		PRINT_VPE_WRN(vpe_dbg_get_dbg_mode(data->id) & VPE_DBG_WRN_MSG, "id list not valid (%d) (%d) \r\n", data->id, vpe_idx);
		return;
	} else {
		memcpy(&(data->dce_ctl), vpe_param[data->id][vpe_idx]->dce_ctl, sizeof(VPE_DCE_CTL_PARAM));
	}
}

static void vpet_api_get_sharpen(ULONG addr)
{
	VPET_SHARPEN_PARAM *data = (VPET_SHARPEN_PARAM *)addr;

	if (!vpe_id_valid[data->id][vpe_idx]) {
		PRINT_VPE_WRN(vpe_dbg_get_dbg_mode(data->id) & VPE_DBG_WRN_MSG, "id list not valid (%d) (%d) \r\n", data->id, vpe_idx);
		return;
	} else {
		memcpy(&(data->sharpen), vpe_param[data->id][vpe_idx]->sharpen, sizeof(VPE_SHARPEN_PARAM));
	}
}

static void vpet_api_get_2dlut(ULONG addr)
{
	VPET_2DLUT_PARAM *data = (VPET_2DLUT_PARAM *)addr;

	if (!vpe_id_valid[data->id][vpe_idx]) {
		PRINT_VPE_WRN(vpe_dbg_get_dbg_mode(data->id) & VPE_DBG_WRN_MSG, "id list not valid (%d) (%d) \r\n", data->id, vpe_idx);
		return;
	} else {
		if (data->lut2d.lut_sz > vpe_2dlut_size) {
			PRINT_VPE_WRN(vpe_dbg_get_dbg_mode(data->id) & VPE_DBG_WRN_MSG, "lut size (%d) large than default (%d) \r\n", data->lut2d.lut_sz, vpe_2dlut_size);
			return;
		}

		real_2dlut_size = vpe_uti_calc_2dlut_tbl_size(data->lut2d.lut_sz);

		data->lut2d.lut_sz = vpe_param[data->id][vpe_idx]->lut2d_int->lut_sz;
		data->lut2d.lut2d_precision = vpe_param[data->id][vpe_idx]->lut2d_int->lut2d_precision;

		memcpy(&(data->lut2d.lut), (ULONG *)vpe_param[data->id][vpe_idx]->lut2d_int->lut_addr, real_2dlut_size);
	}
}

static void vpet_api_get_drt(ULONG addr)
{
	VPET_DRT_PARAM *data = (VPET_DRT_PARAM *)addr;

	if (!vpe_id_valid[data->id][vpe_idx]) {
		PRINT_VPE_WRN(vpe_dbg_get_dbg_mode(data->id) & VPE_DBG_WRN_MSG, "id list not valid (%d) (%d) \r\n", data->id, vpe_idx);
		return;
	} else {
		memcpy(&(data->drt), vpe_param[data->id][vpe_idx]->drt, sizeof(VPE_DRT_PARAM));
	}
}

static void vpet_api_get_dctg(ULONG addr)
{
	VPET_DCTG_CTRL *data = (VPET_DCTG_CTRL *)addr;

	if (!vpe_id_valid[data->id][vpe_idx]) {
		PRINT_VPE_WRN(vpe_dbg_get_dbg_mode(data->id) & VPE_DBG_WRN_MSG, "id list not valid (%d) (%d) \r\n", data->id, vpe_idx);
		return;
	} else {
		memcpy(&(data->dctg), vpe_param[data->id][vpe_idx]->dctg, sizeof(VPE_DCTG_CTRL));
	}
}

static void vpet_api_get_flip_rot(ULONG addr)
{
	VPET_FLIP_ROT_PARAM *data = (VPET_FLIP_ROT_PARAM *)addr;

	if (!vpe_id_valid[data->id][vpe_idx]) {
		PRINT_VPE_WRN(vpe_dbg_get_dbg_mode(data->id) & VPE_DBG_WRN_MSG, "id list not valid (%d) (%d) \r\n", data->id, vpe_idx);
		return;
	} else {
		memcpy(&(data->flip_rot), vpe_param[data->id][vpe_idx]->flip_rot, sizeof(VPE_FLIP_ROT_PARAM));
	}
}

static void vpet_api_get_2dlut_expand(ULONG addr)
{
	VPET_2DLUT_EXPAND_PARAM *data = (VPET_2DLUT_EXPAND_PARAM *)addr;

	if (!vpe_id_valid[data->id][vpe_idx]) {
		PRINT_VPE_WRN(vpe_dbg_get_dbg_mode(data->id) & VPE_DBG_WRN_MSG, "id list not valid (%d) (%d) \r\n", data->id, vpe_idx);
		return;
	} else {
		memcpy(&(data->lut2d_expand), vpe_param[data->id][vpe_idx]->lut2d_expand, sizeof(VPE_2DLUT_EXPAND_PARAM));
	}
}


static void vpet_api_get_dcout_mode(ULONG addr)
{
	VPET_DCOUT_MODE_PARAM *data = (VPET_DCOUT_MODE_PARAM *)addr;

	if (!vpe_id_valid[data->id][vpe_idx]) {
		PRINT_VPE_WRN(vpe_dbg_get_dbg_mode(data->id) & VPE_DBG_WRN_MSG, "id list not valid (%d) (%d) \r\n", data->id, vpe_idx);
		return;
	} else {
		data->dcout_mode = vpe_dcout_mode[data->id];
	}
}

BOOL vpet_get_param_update(UINT32 id, UINT32 idx, VPET_ITEM item)
{
	return vpe_param_update[id][idx][item];
}

ER vpet_api_get_cmd(VPET_ITEM item, ULONG addr)
{
	if (vpet_get_tab[item] == NULL) {
		DBG_WRN("vpet_get_tab(%d) NULL \r\n", item);
		return E_SYS;
	}

	if ((item >= VPET_ITEM_DCE_CTL_PARAM) && (item <= VPET_ITEM_DCOUT_MODE_PARAM)) {
		vpe_idx = (*((UINT32 *)addr) >> 8) & 0xFF;
		*((UINT32 *)addr) = *((UINT32 *)addr) & 0xFF;
		if (vpe_idx >= VPE_IDX_MAX_NUM) {
			DBG_WRN("idx(0x%X) out of range \r\n", vpe_idx);
			return E_SYS;
		}
	}

	if ((item != VPET_ITEM_VERSION) && (item != VPET_ITEM_SIZE_TAB) && (*((UINT32 *)addr) >= VPE_ID_MAX_NUM)) {
		DBG_WRN("item(0x%X) id(%d) out of range \r\n", item, *((UINT32 *)addr));
		return E_SYS;
	}

	if (item >= VPET_ITEM_MAX) {
		DBG_WRN("item(%d) out of range \r\n", item);
		return E_SYS;
	}

	vpet_get_tab[item](addr);
	return E_OK;
}

static void vpet_api_set_dce_ctl(ULONG addr)
{
	VPET_DCE_CTL_PARAM *data = (VPET_DCE_CTL_PARAM *)addr;

	if (!vpe_id_valid[data->id][vpe_idx]) {
		PRINT_VPE_WRN(vpe_dbg_get_dbg_mode(data->id) & VPE_DBG_WRN_MSG, "id list not valid (%d) (%d) \r\n", data->id, vpe_idx);
		return;
	} else {
		memcpy(vpe_param[data->id][vpe_idx]->dce_ctl, &(data->dce_ctl), sizeof(VPE_DCE_CTL_PARAM));

		vpe_param_update[data->id][vpe_idx][VPET_ITEM_DCE_CTL_PARAM] = TRUE;
	}
}

static void vpet_api_set_sharpen(ULONG addr)
{
	VPET_SHARPEN_PARAM *data = (VPET_SHARPEN_PARAM *)addr;

	if (!vpe_id_valid[data->id][vpe_idx]) {
		PRINT_VPE_WRN(vpe_dbg_get_dbg_mode(data->id) & VPE_DBG_WRN_MSG, "id list not valid (%d) (%d) \r\n", data->id, vpe_idx);
		return;
	} else {
		memcpy(vpe_param[data->id][vpe_idx]->sharpen, &(data->sharpen), sizeof(VPE_SHARPEN_PARAM));

		vpe_param_update[data->id][vpe_idx][VPET_ITEM_SHARPEN_PARAM] = TRUE;
	}
}

static void vpet_api_set_2dlut(ULONG addr)
{
	VPET_2DLUT_PARAM *data = (VPET_2DLUT_PARAM *)addr;

	if (!vpe_id_valid[data->id][vpe_idx]) {
		PRINT_VPE_WRN(vpe_dbg_get_dbg_mode(data->id) & VPE_DBG_WRN_MSG, "id list not valid (%d) (%d) \r\n", data->id, vpe_idx);
		return;
	} else {
		if (data->lut2d.lut_sz > vpe_2dlut_size) {
			PRINT_VPE_WRN(vpe_dbg_get_dbg_mode(data->id) & VPE_DBG_WRN_MSG, "lut size (%d) large than default (%d) \r\n", data->lut2d.lut_sz, vpe_2dlut_size);
			return;
		}

		real_2dlut_size = vpe_uti_calc_2dlut_tbl_size(data->lut2d.lut_sz);
		vpe_param[data->id][vpe_idx]->lut2d_int->lut_sz = data->lut2d.lut_sz;
		memcpy((ULONG *)vpe_param[data->id][vpe_idx]->lut2d_int->lut_addr, data->lut2d.lut, real_2dlut_size);
		vpe_param[data->id][vpe_idx]->lut2d_int->lut2d_precision = data->lut2d.lut2d_precision;

		vpe_param_update[data->id][vpe_idx][VPET_ITEM_2DLUT_PARAM] = TRUE;
	}
}

static void vpet_api_set_drt(ULONG addr)
{
	VPET_DRT_PARAM *data = (VPET_DRT_PARAM *)addr;

	if (!vpe_id_valid[data->id][vpe_idx]) {
		PRINT_VPE_WRN(vpe_dbg_get_dbg_mode(data->id) & VPE_DBG_WRN_MSG, "id list not valid (%d) (%d) \r\n", data->id, vpe_idx);
		return;
	} else {
		memcpy(vpe_param[data->id][vpe_idx]->drt, &(data->drt), sizeof(VPE_DRT_PARAM));

		vpe_param_update[data->id][vpe_idx][VPET_ITEM_DRT_PARAM] = TRUE;
	}
}

static void vpet_api_set_dctg(ULONG addr)
{
	VPET_DCTG_CTRL *data = (VPET_DCTG_CTRL *)addr;

	if (!vpe_id_valid[data->id][vpe_idx]) {
		PRINT_VPE_WRN(vpe_dbg_get_dbg_mode(data->id) & VPE_DBG_WRN_MSG, "id list not valid (%d) (%d) \r\n", data->id, vpe_idx);
		return;
	} else {
		memcpy(vpe_param[data->id][vpe_idx]->dctg, &(data->dctg), sizeof(VPE_DCTG_CTRL));

		vpe_param_update[data->id][vpe_idx][VPET_ITEM_DCTG_CTRL] = TRUE;
	}
}

static void vpet_api_set_flip_rot(ULONG addr)
{
	VPET_FLIP_ROT_PARAM *data = (VPET_FLIP_ROT_PARAM *)addr;

	if (!vpe_id_valid[data->id][vpe_idx]) {
		PRINT_VPE_WRN(vpe_dbg_get_dbg_mode(data->id) & VPE_DBG_WRN_MSG, "id list not valid (%d) (%d) \r\n", data->id, vpe_idx);
		return;
	} else {
		memcpy(vpe_param[data->id][vpe_idx]->flip_rot, &(data->flip_rot), sizeof(VPE_FLIP_ROT_PARAM));

		vpe_param_update[data->id][vpe_idx][VPET_ITEM_FLIP_ROT_PARAM] = TRUE;
	}
}

static void vpet_api_set_2dlut_expand(ULONG addr)
{
	VPET_2DLUT_EXPAND_PARAM *data = (VPET_2DLUT_EXPAND_PARAM *)addr;

	if (!vpe_id_valid[data->id][vpe_idx]) {
		PRINT_VPE_WRN(vpe_dbg_get_dbg_mode(data->id) & VPE_DBG_WRN_MSG, "id list not valid (%d) (%d) \r\n", data->id, vpe_idx);
		return;
	} else {
		memcpy(vpe_param[data->id][vpe_idx]->lut2d_expand, &(data->lut2d_expand), sizeof(VPE_2DLUT_EXPAND_PARAM));

		vpe_param_update[data->id][vpe_idx][VPET_ITEM_2DLUT_EXPAND_PARAM] = TRUE;
	}
}

static void vpet_api_set_dcout_mode(ULONG addr)
{
	VPET_DCOUT_MODE_PARAM *data = (VPET_DCOUT_MODE_PARAM *)addr;

	if (!vpe_id_valid[data->id][vpe_idx]) {
		PRINT_VPE_WRN(vpe_dbg_get_dbg_mode(data->id) & VPE_DBG_WRN_MSG, "id list not valid (%d) (%d) \r\n", data->id, vpe_idx);
		return;
	} else {
		vpe_dcout_mode[data->id] = data->dcout_mode;

		vpe_param_update[data->id][vpe_idx][VPET_ITEM_DCOUT_MODE_PARAM] = TRUE;
	}
}

static void vpet_api_reserve(ULONG addr)
{
	return;
}

void vpet_set_param_update(UINT32 id, UINT32 idx, VPET_ITEM item, BOOL en)
{
	vpe_param_update[id][idx][item] = en;
}

ER vpet_api_set_cmd(VPET_ITEM item, ULONG addr)
{
	if (vpet_set_tab[item] == NULL) {
		DBG_WRN("vpet_set_tab(%d) NULL \r\n", item);
		return E_SYS;
	}

	if ((item >= VPET_ITEM_DCE_CTL_PARAM) && (item <= VPET_ITEM_DCOUT_MODE_PARAM)) {
		vpe_idx = (*((UINT32 *)addr) >> 8) & 0xFF;
		*((UINT32 *)addr) = *((UINT32 *)addr) & 0xFF;
		if (vpe_idx >= VPE_IDX_MAX_NUM) {
			DBG_WRN("idx(0x%X) out of range \r\n", vpe_idx);
			return E_SYS;
		}
	}

	if (*((UINT32 *)addr) >= VPE_ID_MAX_NUM) {
		DBG_WRN("id(0x%X) out of range \r\n", *((UINT32 *)addr));
		return E_SYS;
	}

	if (item >= VPET_ITEM_MAX) {
		DBG_WRN("item(%d) out of range \r\n", item);
		return E_SYS;
	}

	vpet_set_tab[item](addr);
	return E_OK;
}

