#if defined(__FREERTOS)
#include <string.h>
#include "isp_dbg.h"
#include "isp_dev_int.h"
#else
#include "isp_ioctl.h"
#include "isp_dbg.h"
#include "isp_lib.h"
#include "isp_main.h"
#endif

#include <plat/top.h>

#include "isp_api_int.h"

#if defined(__KERNEL__)
extern UINT32 isp_id_list;
#endif

#define TIME_MEASURE 0
#if (TIME_MEASURE)
#include "comm/hwclock.h"
#endif

//=============================================================================
// global variable
//=============================================================================
#if defined(__FREERTOS)
static ISP_DEV_INFO pdev_info;
#endif
//=============================================================================
// function declaration
//=============================================================================
static UINT32 isp_frame_count[ISP_ID_MAX_NUM][SYNC_INFO_BUF];
extern UINT32 sie_crop_end_cnt[ISP_ID_MAX_NUM];
extern UINT32 ipp_proc_end_cnt[ISP_ID_MAX_NUM];

//=============================================================================
// external functions
//=============================================================================
void isp_dev_reset_item_valid(ISP_ID id, UINT32 frame_cnt)
{
	UINT32 i, j = frame_cnt % SYNC_INFO_BUF;
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();

	if (pdev_info == NULL) {
		DBG_ERR("pdev_info NULL \r\n");
		return;
	}
	if (id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return;
	}
	if (!isp_get_id_valid(id)) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "id_list not valid (%d) \r\n", id);
		return;
	}

	for (i = ISP_SYNC_AE_STATUS; i < ISP_SYNC_FRAME_CNT_MAX; i++) {
		*(BOOL *)(pdev_info->p_sync_valid[id] + (i * SYNC_INFO_BUF + j)) = FALSE;
	}
}

ER isp_dev_get_sync_item(ISP_ID id, ISP_SYNC_SEL sel, ISP_SYNC_ITEM item, void *data)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	BOOL emu_en = isp_api_get_emu_enable();
	UINT32 sync_id = 0, get_id = 0, frame_count = 0, i;
	UINT32 isp_id = id;
	UINT32 dbg_mode = isp_dbg_get_dbg_mode(isp_id);
	ISP_SYNC_INFO *p_info = NULL;

	if (pdev_info == NULL) {
		return E_SYS;
	}
	if (isp_id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return E_PAR;
	}
	if (!isp_get_id_valid(isp_id)) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "id_list not valid (%d) \r\n", id);
		return E_PAR;
	}

	switch (sel) {
	case ISP_SYNC_SEL_SIE:
		sync_id = (isp_sync_id_sie[isp_id]) % SYNC_INFO_BUF;
		frame_count = isp_sync_id_sie[isp_id];
		break;

	case ISP_SYNC_SEL_IPP:
		if (!(isp_get_ipp_indep(id)) && (pdev_info->ipp_to_sie_id_map_en[id])) {
			if (pdev_info->ipp_to_sie_id_table[id] != ISP_ID_IGNORE) {
				isp_id = pdev_info->ipp_to_sie_id_table[id];
			} else {
				isp_id = id;
			}
		} else {
			isp_id = id;
		}

		sync_id = (isp_sync_id_ipp[isp_id]) % SYNC_INFO_BUF;
		frame_count = isp_sync_id_ipp[isp_id];
		break;

	case ISP_SYNC_SEL_ENC:
		if (!(isp_get_ipp_indep(id)) && (pdev_info->enc_to_sie_id_map_en[id])) {
			if (pdev_info->enc_to_sie_id_table[id] != ISP_ID_IGNORE) {
				isp_id = pdev_info->enc_to_sie_id_table[id];
			} else {
				isp_id = id;
			}
		} else {
			isp_id = id;
		}

		sync_id = (isp_sync_id_enc[isp_id]) % SYNC_INFO_BUF;
		frame_count = isp_sync_id_enc[isp_id];
		break;

	default:
		sync_id = 1;
		frame_count = 1;
		break;
	}

	if (sync_id == 0) {
		get_id = SYNC_INFO_BUF - 1;
	} else {
		get_id = sync_id - 1;
	}

	if (item > ISP_SYNC_FRAME_CNT_MAX) {
		get_id = 0;        // out of frame cnt SyncInfo
	}
	if (emu_en || isp_get_ipp_indep(id)) {
		get_id = 0;        // fixed to setting idx = 0
	}
	if (*(BOOL *)(pdev_info->p_sync_valid[isp_id] + (item * SYNC_INFO_BUF + get_id)) == FALSE) {
		return E_SYS;
	}

	// check frame count
	if (((INT32)frame_count - (INT32)isp_frame_count[isp_id][get_id] >= SYNC_INFO_BUF) && (item < ISP_SYNC_FRAME_CNT_MAX)) {
		PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "sync info not match (%d, %d, %d, %d, %d, %d) \r\n", id, isp_id, frame_count, isp_frame_count[isp_id][get_id], get_id, sel);
		return E_SYS;
	}

	p_info = pdev_info->p_sync_info[isp_id] + get_id;

	switch (item) {
	// NOTE: Start of frame cnt SyncInfo
	case ISP_SYNC_AE_STATUS:
		memcpy((UINT32 *)data, &p_info->ae_status, sizeof(UINT32));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_AE_STATUS, "ISP (%2d) FrameCnt (%2d) Get (%2d) AE_STATUS (%d)\r\n", isp_id, sync_id, get_id, p_info->ae_status);
		break;

	case ISP_SYNC_AE_TOTAL_GAIN:
		memcpy((UINT32 *)data, &p_info->total_gain, sizeof(UINT32));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_TOTAL_GAIN, "ISP (%2d) FrameCnt (%2d) Get (%2d) TOTAL_GAIN (%d)\r\n", isp_id, sync_id, get_id, p_info->total_gain);
		break;

	case ISP_SYNC_AE_DGAIN:
		memcpy((UINT32 *)data, &p_info->dgain, sizeof(UINT32));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_D_GAIN, "ISP (%2d) FrameCnt (%2d) Get (%2d) DGAIN (%d)\r\n", isp_id, sync_id, get_id, p_info->dgain);
		break;

	case ISP_SYNC_AE_LV:
		memcpy((UINT32 *)data, &p_info->lv, sizeof(UINT32));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_LV, "ISP (%2d) FrameCnt (%2d) Get (%2d) LV (%d)\r\n", isp_id, sync_id, get_id, p_info->lv);
		break;

	case ISP_SYNC_AE_LV_BASE:
		memcpy((UINT32 *)data, &p_info->lv_base, sizeof(UINT32));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_LV, "ISP (%2d) FrameCnt (%2d) Get (%2d) LV_BASE (%d)\r\n", isp_id, sync_id, get_id, p_info->lv_base);
		break;

	case ISP_SYNC_AE_LA_ENABLE:
		memcpy((BOOL *)data, &p_info->la_enable, sizeof(BOOL));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_CA_LA_ENABLE, "ISP (%2d) FrameCnt (%2d) Get (%2d) la_enable (%d)\r\n", isp_id, sync_id, get_id, p_info->la_enable);
		break;

	case ISP_SYNC_AE_SHDR_EV_RATIO:
		memcpy((UINT32 *)data, &p_info->shdr_ev_ratio[0], sizeof(UINT32) * ISP_SEN_MFRAME_MAX_NUM);
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_EV_RATIO, "ISP (%2d) FrameCnt (%2d) Get (%2d) EV_RATIO (%d, %d)\r\n", isp_id, sync_id, get_id, p_info->shdr_ev_ratio[0], p_info->shdr_ev_ratio[1]);
		break;

	case ISP_SYNC_AE_SHDR_TM_RATIO:
		memcpy((UINT32 *)data, &p_info->shdr_tm_ratio, sizeof(UINT32));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_TM_RATIO, "ISP (%2d) FrameCnt (%2d) Get (%2d) TM_RATIO (%d)\r\n", isp_id, sync_id, get_id, p_info->shdr_tm_ratio);
		break;

	case ISP_SYNC_AWB_CGAIN:
		memcpy((UINT32 *)data, &p_info->cgain[0], sizeof(UINT32) * ISP_CGAIN_NUM);
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_C_GAIN, "ISP (%2d) FrameCnt (%2d) Get (%2d) CGAIN (%d, %d, %d)\r\n", isp_id, sync_id, get_id, p_info->cgain[0], p_info->cgain[1], p_info->cgain[2]);
		break;

	case ISP_SYNC_AWB_CT:
		memcpy((UINT32 *)data, &p_info->ct, sizeof(UINT32));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_CT, "ISP (%2d) FrameCnt (%2d) Get (%2d) CT (%d)\r\n", isp_id, sync_id, get_id, p_info->ct);
		break;

	case ISP_SYNC_AWB_CA_ENABLE:
		memcpy((BOOL *)data, &p_info->ca_enable, sizeof(BOOL));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_CA_LA_ENABLE, "ISP (%2d) FrameCnt (%2d) Get (%2d) ca_enable (%d)\r\n", isp_id, sync_id, get_id, p_info->ca_enable);
		break;

	case ISP_SYNC_IQ_FINAL_CGAIN:
		memcpy((UINT32 *)data, &p_info->final_cgain[0], sizeof(UINT32) * ISP_CGAIN_NUM);
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_FINAL_C_GAIN, "ISP (%2d) FrameCnt (%2d) Get (%2d) FINAL_CGAIN (%d, %d, %d)\r\n", isp_id, sync_id, get_id , p_info->final_cgain[0], p_info->final_cgain[1], p_info->final_cgain[2]);
		break;

	case ISP_SYNC_AE_SHDR_HBS:
		memcpy((UINT32 *)data, &p_info->shdr_hbs, sizeof(ISP_AE_HBS_PARAM));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_HBS_PARAM, "ISP (%2d) FrameCnt (%2d) Get (%2d) HBS_PARAM (%d, %d, %d)\r\n", isp_id, sync_id, get_id , p_info->shdr_hbs.lum_th, p_info->shdr_hbs.w_start, p_info->shdr_hbs.w_slope);
		break;
	// NOTE: end of frame cnt SyncInfo

	case ISP_SYNC_CAPTURE_TOTAL_GAIN:
		memcpy((UINT32 *)data, &pdev_info->capture_total_gain[isp_id], sizeof(UINT32));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_CAPTURE, "ISP (%2d) Get CAPTURE_TOTAL_GAIN (%d)\r\n", isp_id, pdev_info->capture_total_gain[isp_id]);
		break;

	case ISP_SYNC_CAPTURE_DGAIN:
		memcpy((UINT32 *)data, &pdev_info->capture_dgain[isp_id], sizeof(UINT32));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_CAPTURE, "ISP (%2d) Get CAPTURE_DGAIN (%d)\r\n", isp_id, pdev_info->capture_dgain[isp_id]);
		break;

	case ISP_SYNC_CAPTURE_CGAIN:
		memcpy((UINT32 *)data, &pdev_info->capture_cgain[isp_id][0], sizeof(UINT32) * ISP_CGAIN_NUM);
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_CAPTURE, "ISP (%2d) Get CAPTURE_CGAIN (%d, %d, %d)\r\n", isp_id, pdev_info->capture_cgain[isp_id][0], pdev_info->capture_cgain[isp_id][1], pdev_info->capture_cgain[isp_id][2]);
		break;

	case ISP_SYNC_AE_INIT:
		memcpy((UINT32 *)data, &pdev_info->ae_init_info[isp_id], sizeof(ISP_AE_INIT_INFO));
		break;

	case ISP_SYNC_AWB_INIT:
		memcpy((UINT32 *)data, &pdev_info->awb_init_info[isp_id], sizeof(ISP_AWB_INIT_INFO));
		break;

	case ISP_SYNC_IQ_INIT:
		memcpy((UINT32 *)data, &pdev_info->iq_init_info[isp_id], sizeof(ISP_IQ_INIT_INFO));
		break;

	case ISP_SYNC_CA_TH:
		memcpy((ISP_CA_TH_PARAM *)data, &pdev_info->ca_th[isp_id], sizeof(ISP_CA_TH_PARAM));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_CA_TH, "ISP (%2d), th_enable (%d), g(%d, %d), r(%d, %d), b(%d, %d), p(%d, %d)\r\n", isp_id, pdev_info->ca_th[isp_id].th_enable,
			pdev_info->ca_th[isp_id].g_th_l, pdev_info->ca_th[isp_id].g_th_u, pdev_info->ca_th[isp_id].r_th_l, pdev_info->ca_th[isp_id].r_th_u,
			pdev_info->ca_th[isp_id].b_th_l, pdev_info->ca_th[isp_id].b_th_u, pdev_info->ca_th[isp_id].p_th_l, pdev_info->ca_th[isp_id].p_th_u);
		break;

	case ISP_SYNC_CA_ROI:
		memcpy((UINT32 *)data, &pdev_info->ca_roi[isp_id], sizeof(URECT));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_CA_ROI, "ISP (%2d), ca_roi (%d, %d, %d, %d)\r\n", isp_id,
			pdev_info->ca_roi[isp_id].x, pdev_info->ca_roi[isp_id].y, pdev_info->ca_roi[isp_id].w, pdev_info->ca_roi[isp_id].h);
		break;

	case ISP_SYNC_LA_ROI:
		memcpy((UINT32 *)data, &pdev_info->la_roi[isp_id], sizeof(URECT));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_LA_ROI, "ISP (%2d), la_roi (%d, %d, %d, %d)\r\n", isp_id,
			pdev_info->la_roi[isp_id].x, pdev_info->la_roi[isp_id].y, pdev_info->la_roi[isp_id].w, pdev_info->la_roi[isp_id].h);
		break;

	case ISP_SYNC_VA_INDEP_ROI:
		memcpy((UINT32 *)data, &pdev_info->va_indep_roi[isp_id][0], sizeof(URECT) * ISP_INDEP_VA_WIN_NUM);
		for (i = 0; i < ISP_INDEP_VA_WIN_NUM; i++) {
			PRINT_ISP(dbg_mode & ISP_DBG_SYNC_VA_ROI, "ISP (%2d), va_indep_roi (%d, %d, %d, %d)\r\n", isp_id,
				pdev_info->va_indep_roi[isp_id][i].x, pdev_info->va_indep_roi[isp_id][i].y, pdev_info->va_indep_roi[isp_id][i].w, pdev_info->va_indep_roi[isp_id][i].h);
		}
		break;

	case ISP_SYNC_IFE_VA_INDEP_ROI:
		memcpy((UINT32 *)data, &pdev_info->ife_va_indep_roi[isp_id][0], sizeof(URECT) * ISP_INDEP_VA_WIN_NUM);
		for (i = 0; i < ISP_INDEP_VA_WIN_NUM; i++) {
			PRINT_ISP(dbg_mode & ISP_DBG_SYNC_VA_ROI, "ISP (%2d), ife_va_indep_roi (%d, %d, %d, %d)\r\n", isp_id,
				pdev_info->ife_va_indep_roi[isp_id][i].x, pdev_info->ife_va_indep_roi[isp_id][i].y, pdev_info->ife_va_indep_roi[isp_id][i].w, pdev_info->ife_va_indep_roi[isp_id][i].h);
		}
		break;

	default:
		break;
	}

	return E_OK;
}

void isp_dev_set_sync_item(ISP_ID id, ISP_SYNC_SEL sel, ISP_SYNC_ITEM item, void *data)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	BOOL emu_en = isp_api_get_emu_enable();
	UINT32 sync_id = 0, set_id = 0, i;
	UINT32 dbg_mode = isp_dbg_get_dbg_mode(id);
	ISP_SYNC_INFO *p_info = NULL;

	if (pdev_info == NULL) {
		return;
	}
	if (id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return;
	}
	if (!isp_get_id_valid(id)) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "id_list not valid (%d) \r\n", id);
		return;
	}

	if ((emu_en || isp_get_ipp_indep(id)) && (item == ISP_SYNC_IQ_FINAL_CGAIN)) {
		return;
	}

	sync_id = ((sel == ISP_SYNC_SEL_CURRENT) || (emu_en || isp_get_ipp_indep(id))) ? 0 : (isp_sync_id_sie[id]) % SYNC_INFO_BUF;

	item = ((item == ISP_SYNC_AWB_CGAIN) && (emu_en || isp_get_ipp_indep(id))) ? ISP_SYNC_IQ_FINAL_CGAIN : item;

	set_id = sync_id;
	*(BOOL *)(pdev_info->p_sync_valid[id] + (item * SYNC_INFO_BUF + set_id)) = TRUE;

	p_info = pdev_info->p_sync_info[id] + set_id;

	switch (item) {
	// NOTE: Start of frame cnt SyncInfo
	case ISP_SYNC_AE_STATUS:
		memcpy(&p_info->ae_status, (UINT32 *)data, sizeof(UINT32));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_AE_STATUS, "ISP (%2d) FrameCnt (%2d) Set (%2d) AE_STATUS (%d)\r\n", id, sync_id, set_id, p_info->ae_status);
		break;

	case ISP_SYNC_AE_TOTAL_GAIN:
		memcpy(&p_info->total_gain, (UINT32 *)data, sizeof(UINT32));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_TOTAL_GAIN, "ISP (%2d) FrameCnt (%2d) Set (%2d) TOTAL_GAIN (%d)\r\n", id, sync_id, set_id, p_info->total_gain);
		break;

	case ISP_SYNC_AE_DGAIN:
		memcpy(&p_info->dgain, (UINT32 *)data, sizeof(UINT32));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_D_GAIN, "ISP (%2d) FrameCnt (%2d) Set (%2d) DGAIN (%d)\r\n", id, sync_id, set_id, p_info->dgain);
		break;

	case ISP_SYNC_AE_LV:
		memcpy(&p_info->lv, (UINT32 *)data, sizeof(UINT32));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_LV, "ISP (%2d) FrameCnt (%2d) Set (%2d) LV (%d)\r\n", id, sync_id, set_id, p_info->lv);
		break;

	case ISP_SYNC_AE_LV_BASE:
		memcpy(&p_info->lv_base, (UINT32 *)data, sizeof(UINT32));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_LV, "ISP (%2d) FrameCnt (%2d) Set (%2d) LV_BASE (%d)\r\n", id, sync_id, set_id, p_info->lv_base);
		break;

	case ISP_SYNC_AE_LA_ENABLE:
		memcpy(&p_info->la_enable, (BOOL *)data, sizeof(BOOL));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_CA_LA_ENABLE, "ISP (%2d) FrameCnt (%2d) Set (%2d) la_enable (%d)\r\n", id, sync_id, set_id, p_info->la_enable);
		break;

	case ISP_SYNC_AE_SHDR_EV_RATIO:
		memcpy(&p_info->shdr_ev_ratio[0], (UINT32 *)data, sizeof(UINT32) * ISP_SEN_MFRAME_MAX_NUM);
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_EV_RATIO, "ISP (%2d) FrameCnt (%2d) Set (%2d) EV_RATIO (%d, %d)\r\n", id, sync_id, set_id, p_info->shdr_ev_ratio[0], p_info->shdr_ev_ratio[1]);
		break;

	case ISP_SYNC_AE_SHDR_TM_RATIO:
		memcpy(&p_info->shdr_tm_ratio, (UINT32 *)data, sizeof(UINT32));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_TM_RATIO, "ISP (%2d) FrameCnt (%2d) Set (%2d) TM_RATIO (%d)\r\n", id, sync_id, set_id, p_info->shdr_tm_ratio);
		break;

	case ISP_SYNC_AWB_CGAIN:
		memcpy(&p_info->cgain[0], (UINT32 *)data, sizeof(UINT32) * ISP_CGAIN_NUM);
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_C_GAIN, "ISP (%2d) FrameCnt (%2d) Set (%2d) CGAIN (%d, %d, %d)\r\n", id, sync_id, set_id, p_info->cgain[0], p_info->cgain[1], p_info->cgain[2]);
		break;

	case ISP_SYNC_AWB_CT:
		memcpy(&p_info->ct, (UINT32 *)data, sizeof(UINT32));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_CT, "ISP (%2d) FrameCnt (%2d) Set (%2d) CT (%d)\r\n", id, sync_id, set_id, p_info->ct);
		break;

	case ISP_SYNC_AWB_CA_ENABLE:
		memcpy(&p_info->ca_enable, (BOOL *)data, sizeof(BOOL));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_CA_LA_ENABLE, "ISP (%2d) FrameCnt (%2d) Set (%2d) ca_enable (%d)\r\n", id, sync_id, set_id, p_info->ca_enable);
		break;

	case ISP_SYNC_IQ_FINAL_CGAIN:
		memcpy(&p_info->final_cgain[0], (UINT32 *)data, sizeof(UINT32) * ISP_CGAIN_NUM);
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_FINAL_C_GAIN, "ISP (%2d) FrameCnt (%2d) Set (%2d) FINAL_CGAIN (%d, %d, %d)\r\n", id, sync_id, set_id, p_info->final_cgain[0], p_info->final_cgain[1], p_info->final_cgain[2]);
		break;

	case ISP_SYNC_AE_SHDR_HBS:
		memcpy(&p_info->shdr_hbs, (UINT32 *)data, sizeof(ISP_AE_HBS_PARAM));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_HBS_PARAM, "ISP (%2d) FrameCnt (%2d) Set (%2d) HBS_PARAM (%d, %d, %d)\r\n", id, sync_id, set_id, p_info->shdr_hbs.lum_th, p_info->shdr_hbs.w_start, p_info->shdr_hbs.w_slope);
		break;
	// NOTE: end of frame cnt SyncInfo

	case ISP_SYNC_CAPTURE_TOTAL_GAIN:
		memcpy(&pdev_info->capture_total_gain[id], (UINT32 *)data, sizeof(UINT32));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_CAPTURE, "ISP (%2d) Set CAPTURE_TOTAL_GAIN (%d)\r\n", id, pdev_info->capture_total_gain[id]);
		break;

	case ISP_SYNC_CAPTURE_DGAIN:
		memcpy(&pdev_info->capture_dgain[id], (UINT32 *)data, sizeof(UINT32));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_CAPTURE, "ISP (%2d) Set CAPTURE_DGAIN (%d)\r\n", id, pdev_info->capture_dgain[id]);
		break;

	case ISP_SYNC_CAPTURE_CGAIN:
		memcpy(&pdev_info->capture_cgain[id][0], (UINT32 *)data, sizeof(UINT32) * ISP_CGAIN_NUM);
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_CAPTURE, "ISP (%2d) Set CAPTURE_CGAIN (%d, %d, %d)\r\n", id, pdev_info->capture_cgain[id][0], pdev_info->capture_cgain[id][1], pdev_info->capture_cgain[id][2]);
		break;

	case ISP_SYNC_AE_INIT:
		memcpy(&pdev_info->ae_init_info[id], (UINT32 *)data, sizeof(ISP_AE_INIT_INFO));
		break;

	case ISP_SYNC_AWB_INIT:
		memcpy(&pdev_info->awb_init_info[id], (UINT32 *)data, sizeof(ISP_AWB_INIT_INFO));
		break;

	case ISP_SYNC_IQ_INIT:
		memcpy(&pdev_info->iq_init_info[id], (UINT32 *)data, sizeof(ISP_IQ_INIT_INFO));
		break;

	case ISP_SYNC_CA_TH:
		memcpy(&pdev_info->ca_th[id], (ISP_CA_TH_PARAM *)data, sizeof(ISP_CA_TH_PARAM));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_CA_TH, "ISP (%2d), th_enable (%d), g(%d, %d), r(%d, %d), b(%d, %d), p(%d, %d)\r\n", id, pdev_info->ca_th[id].th_enable,
			pdev_info->ca_th[id].g_th_l, pdev_info->ca_th[id].g_th_u, pdev_info->ca_th[id].r_th_l, pdev_info->ca_th[id].r_th_u,
			pdev_info->ca_th[id].b_th_l, pdev_info->ca_th[id].b_th_u, pdev_info->ca_th[id].p_th_l, pdev_info->ca_th[id].p_th_u);
		break;

	case ISP_SYNC_CA_ROI:
		memcpy(&pdev_info->ca_roi[id], (UINT32 *)data, sizeof(URECT));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_CA_ROI, "ISP (%2d), ca_roi (%d, %d, %d, %d)\r\n", id,
			pdev_info->ca_roi[id].x, pdev_info->ca_roi[id].y, pdev_info->ca_roi[id].w, pdev_info->ca_roi[id].h);
		break;

	case ISP_SYNC_LA_ROI:
		memcpy(&pdev_info->la_roi[id], (UINT32 *)data, sizeof(URECT));
		PRINT_ISP(dbg_mode & ISP_DBG_SYNC_LA_ROI, "ISP (%2d), la_roi (%d, %d, %d, %d)\r\n", id,
			pdev_info->la_roi[id].x, pdev_info->la_roi[id].y, pdev_info->la_roi[id].w, pdev_info->la_roi[id].h);
		break;

	case ISP_SYNC_VA_INDEP_ROI:
		memcpy(&pdev_info->va_indep_roi[id][0], (UINT32 *)data, sizeof(URECT) * ISP_INDEP_VA_WIN_NUM);
		for (i = 0; i < ISP_INDEP_VA_WIN_NUM; i++) {
			PRINT_ISP(dbg_mode & ISP_DBG_SYNC_VA_ROI, "ISP (%2d), va_indep_roi (%d, %d, %d, %d)\r\n", id,
				pdev_info->va_indep_roi[id][i].x, pdev_info->va_indep_roi[id][i].y, pdev_info->va_indep_roi[id][i].w, pdev_info->va_indep_roi[id][i].h);
		}
		break;

		case ISP_SYNC_IFE_VA_INDEP_ROI:
			memcpy(&pdev_info->ife_va_indep_roi[id][0], (UINT32 *)data, sizeof(URECT) * ISP_INDEP_VA_WIN_NUM);
			for (i = 0; i < ISP_INDEP_VA_WIN_NUM; i++) {
				PRINT_ISP(dbg_mode & ISP_DBG_SYNC_VA_ROI, "ISP (%2d), ife_va_indep_roi (%d, %d, %d, %d)\r\n", id,
					pdev_info->ife_va_indep_roi[id][i].x, pdev_info->ife_va_indep_roi[id][i].y, pdev_info->ife_va_indep_roi[id][i].w, pdev_info->ife_va_indep_roi[id][i].h);
			}
			break;


	default:
		break;
	}
}

void isp_dev_save_frame_count(UINT32 id)
{
	UINT32 sync_id = isp_sync_id_sie[id] % SYNC_INFO_BUF;
	BOOL emu_en = isp_api_get_emu_enable();
	UINT32 i;

	if (!emu_en && !(isp_get_ipp_indep(id))) {
		isp_frame_count[id][sync_id] = isp_sync_id_sie[id];
	} else {
		for (i = 0; i < SYNC_INFO_BUF; i++) {
			isp_frame_count[id][i] = isp_sync_id_ipp[id];
		}
	}
}

ISP_CA_RSLT *isp_dev_get_ca(ISP_ID id)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	ISP_FUNC_EN fun_en = 0;
	INT32 sie_ret = CTL_SIE_E_OK;
	CTL_SIE_ISP_STATUS sie_status = CTL_SIE_ISP_STS_CLOSE;
	#if (TIME_MEASURE)
	static UINT32 t_ca_1, t_ca_2, t_ca_min = 0xFFFFFFFF, t_ca_max = 0;
	#endif

	if (!isp_get_id_valid(id)) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "ca id_list not valid (%d) \r\n", id);
	}
	if (pdev_info == NULL) {
		return NULL;
	}
	if (pdev_info->ca_rslt[id] == NULL) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "ca result NULL (%d) \r\n", id);
		return NULL;
	}
	if (id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", id);
		return NULL;
	}

	#if (TIME_MEASURE)
	t_ca_1 = hwclock_get_counter();
	#endif

	if (!ctl_sie_get_is_open(id)) {
		return pdev_info->ca_rslt[id];
	}

	sie_ret = ctl_sie_isp_get(isp_get_sie_id(id), CTL_SIE_ISP_ITEM_STATUS, &sie_status);
	if (sie_ret == CTL_SIE_E_OK) {
		if (sie_status != CTL_SIE_ISP_STS_RUN) {
			return pdev_info->ca_rslt[id];
		}
	} else {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "get CTL_SIE_ISP_ITEM_STATUS fail (%d) \r\n", sie_ret);
	}

	sie_ret = ctl_sie_isp_get(isp_get_sie_id(id), CTL_SIE_ISP_ITEM_FUNC_EN, &fun_en);
	if ((sie_ret == CTL_SIE_E_OK) && (fun_en & ISP_FUNC_EN_AWB)) {
		sie_ret = ctl_sie_isp_get(isp_get_sie_id(id), CTL_SIE_ISP_ITEM_CA_RSLT, (void *)pdev_info->ca_rslt[id]);
		if (sie_ret != CTL_SIE_E_OK) {
			PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "get CTL_SIE_ISP_ITEM_CA_RSLT fail (%d) \r\n", sie_ret);
		}
	} else {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "get CTL_SIE_ISP_ITEM_FUNC_EN fail (%d) (0x%X) \r\n", sie_ret, fun_en);
	}

	#if (TIME_MEASURE)
	t_ca_2 = hwclock_get_counter();
	if (t_ca_2 - t_ca_1 > t_ca_max) {
		t_ca_max = t_ca_2 - t_ca_1;
	}
	if (t_ca_2 - t_ca_1 < t_ca_min) {
		t_ca_min = t_ca_2 - t_ca_1;
	}
	DBG_DUMP("CA: %4d, range(%4d, %4d) \r\n", t_ca_2 - t_ca_1, t_ca_min, t_ca_max);
	#endif

	return pdev_info->ca_rslt[id];
}

ISP_LA_RSLT *isp_dev_get_la(ISP_ID id)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	ISP_FUNC_EN fun_en = 0;
	INT32 sie_ret = CTL_SIE_E_OK;
	CTL_SIE_ISP_STATUS sie_status = CTL_SIE_ISP_STS_CLOSE;
	#if (TIME_MEASURE)
	static UINT32 t_la_1, t_la_2, t_la_min = 0xFFFFFFFF, t_la_max = 0;
	#endif

	if (!isp_get_id_valid(id)) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "la id_list not valid (%d) \r\n", id);
	}
	if (pdev_info == NULL) {
		return NULL;
	}
	if (pdev_info->la_rslt[isp_get_sie_id(id)] == NULL) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "la result NULL (%d) \r\n", id);
		return NULL;
	}
	if (id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", id);
		return NULL;
	}

	#if (TIME_MEASURE)
	t_la_1 = hwclock_get_counter();
	#endif

	if (!ctl_sie_get_is_open(id)) {
		return pdev_info->la_rslt[id];
	}

	sie_ret = ctl_sie_isp_get(isp_get_sie_id(id), CTL_SIE_ISP_ITEM_STATUS, &sie_status);
	if (sie_ret == CTL_SIE_E_OK) {
		if (sie_status != CTL_SIE_ISP_STS_RUN) {
			return pdev_info->la_rslt[id];
		}
	} else {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "get CTL_SIE_ISP_ITEM_STATUS fail (%d) \r\n", sie_ret);
	}

	sie_ret = ctl_sie_isp_get(isp_get_sie_id(id), CTL_SIE_ISP_ITEM_FUNC_EN, &fun_en);
	if ((sie_ret == CTL_SIE_E_OK) && (fun_en & ISP_FUNC_EN_AE)) {
		sie_ret = ctl_sie_isp_get(isp_get_sie_id(id), CTL_SIE_ISP_ITEM_LA_RSLT, (void *)pdev_info->la_rslt[id]);
		if (sie_ret != CTL_SIE_E_OK) {
			PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "get CTL_SIE_ISP_ITEM_LA_RSLT fail (%d) \r\n", sie_ret);
		}
	} else {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "get CTL_SIE_ISP_ITEM_FUNC_EN fail (%d) (0x%X) \r\n", sie_ret, fun_en);
	}

	#if (TIME_MEASURE)
	t_la_2 = hwclock_get_counter();
	if (t_la_2 - t_la_1 > t_la_max) {
		t_la_max = t_la_2 - t_la_1;
	}
	if (t_la_2 - t_la_1 < t_la_min) {
		t_la_min = t_la_2 - t_la_1;
	}
	DBG_DUMP("LA: %4d, range(%4d, %4d) \r\n", t_la_2 - t_la_1, t_la_min, t_la_max);
	#endif

	return pdev_info->la_rslt[id];
}

ISP_IPE_VA_RSLT *isp_dev_get_ipe_va(ISP_ID id)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();

	if (nvt_get_chip_id() == CHIP_NS02402) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "NT98539A not support ipe va. \r\n");
		return NULL;
	}

	if (!isp_get_id_valid(id)) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "ipe_va id_list not valid (%d) \r\n", id);
	}
	if (pdev_info == NULL) {
		return NULL;
	}
	if (pdev_info->ipe_va_rslt[id] == NULL) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "ipe_va result NULL (%d) \r\n", id);
		return NULL;
	}
	if (id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return NULL;
	}
	return pdev_info->ipe_va_rslt[id];
}

ISP_IPE_VA_INDEP_RSLT *isp_dev_get_ipe_va_indep(ISP_ID id)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();

	if (nvt_get_chip_id() == CHIP_NS02402) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "NT98539A not support ipe va indep. \r\n");
		return NULL;
	}

	if (!isp_get_id_valid(id)) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "ipe_va id_list not valid (%d) \r\n", id);
	}
	if (pdev_info == NULL) {
		return NULL;
	}
	if (pdev_info->ipe_va_indep_rslt[id] == NULL) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "ipe_va indep result NULL (%d) \r\n", id);
		return NULL;
	}
	if (id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return NULL;
	}
	return pdev_info->ipe_va_indep_rslt[id];
}

ISP_IFE_VA_RSLT *isp_dev_get_ife_va(ISP_ID id)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();

	if (!isp_get_id_valid(id)) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "ife_va id_list not valid (%d) \r\n", id);
	}
	if (pdev_info == NULL) {
		return NULL;
	}
	if (pdev_info->ife_va_rslt[id] == NULL) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "ife_va result NULL (%d) \r\n", id);
		return NULL;
	}
	if (id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return NULL;
	}
	return pdev_info->ife_va_rslt[id];
}

ISP_IFE_VA_INDEP_RSLT *isp_dev_get_ife_va_indep(ISP_ID id)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();

	if (!isp_get_id_valid(id)) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "ife_va id_list not valid (%d) \r\n", id);
	}
	if (pdev_info == NULL) {
		return NULL;
	}
	if (pdev_info->ife_va_indep_rslt[id] == NULL) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "ife_va indep result NULL (%d) \r\n", id);
		return NULL;
	}
	if (id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return NULL;
	}
	return pdev_info->ife_va_indep_rslt[id];
}

ISP_HISTO_RSLT *isp_dev_get_histo(ISP_ID id)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();

	if (!isp_get_id_valid(id)) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "ca id_list not valid (%d) \r\n", id);
	}
	if (pdev_info == NULL) {
		return NULL;
	}
	if (id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return NULL;
	}

	if (pdev_info->is_capture[id] == TRUE) {
		if (pdev_info->histo_rslt_cap[pdev_info->cap_id_mapping_table[id]] == NULL) {
			PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "histo cap result NULL (%d) (%d) \r\n", id, pdev_info->cap_id_mapping_table[id]);
			return NULL;
		}
		return pdev_info->histo_rslt_cap[pdev_info->cap_id_mapping_table[id]];
	} else {
		if (pdev_info->histo_rslt[id] == NULL) {
			PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "histo result NULL (%d) \r\n", id);
			return NULL;
		}
		return pdev_info->histo_rslt[id];
	}
}

ISP_DEFOG_STCS *isp_dev_get_defog_stcs(ISP_ID id)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();

	if (pdev_info == NULL) {
		return NULL;
	}
	if (id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return NULL;
	}

	if (pdev_info->is_capture[id] == TRUE) {
		return &pdev_info->defog_stcs_cap[pdev_info->cap_id_mapping_table[id]];
	} else {
		return &pdev_info->defog_stcs[id];
	}
}

ISP_IPE_SUBOUT_BUF *isp_dev_get_ipe_subout_buf(ISP_ID id)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();

	if (!isp_get_id_valid(id)) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "ca id_list not valid (%d) \r\n", id);
	}
	if (pdev_info == NULL) {
		return NULL;
	}
	if (id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return NULL;
	}

	if (pdev_info->is_capture[id] == TRUE) {
		if (pdev_info->ipe_subout_buf_cap[pdev_info->cap_id_mapping_table[id]] == NULL) {
			PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "subout cap result NULL (%d) (%d) \r\n", id, pdev_info->cap_id_mapping_table[id]);
			return NULL;
		}
		return pdev_info->ipe_subout_buf_cap[pdev_info->cap_id_mapping_table[id]];
	} else {
		if (pdev_info->ipe_subout_buf[id] == NULL) {
			PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "subout result NULL (%d) \r\n", id);
			return NULL;
		}
		return pdev_info->ipe_subout_buf[id];
	}
}

CTL_MTR_DRV_TAB *isp_dev_get_mtr_drv_tab(void)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();

	if (pdev_info == NULL) {
		return NULL;
	}
	return pdev_info->mrt_drv_tab;
}

void isp_dev_reg_ae_module(ISP_MODULE *ae_module)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();

	if (pdev_info == NULL) {
		return;
	}
	pdev_info->isp_ae = ae_module;
}

void isp_dev_reg_af_module(ISP_MODULE *af_module)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();

	if (pdev_info == NULL) {
		return;
	}
	pdev_info->isp_af = af_module;
}

void isp_dev_reg_awb_module(ISP_MODULE *awb_module)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();

	if (pdev_info == NULL) {
		return;
	}
	pdev_info->isp_awb = awb_module;
}

void isp_dev_reg_iq_module(ISP_MODULE *iq_module)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();

	if (pdev_info == NULL) {
		return;
	}
	pdev_info->isp_iq = iq_module;
}

void isp_dev_reg_motor_driver(CTL_MTR_DRV_TAB *mrt_drv_tab)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();

	if (pdev_info == NULL) {
		return;
	}
	pdev_info->mrt_drv_tab = mrt_drv_tab;
}

void isp_dev_wait_vd(ISP_ID id, UINT32 timeout, UINT32 *frame_count)
{
	#if defined(__KERNEL__)
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	DECLARE_WAITQUEUE(wait, current);
	INT32 time;

	if (pdev_info == NULL) {
		return;
	}
	if (id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return;
	}

	add_wait_queue(&pdev_info->wait_vd[id], &wait);
	__set_current_state(TASK_INTERRUPTIBLE);
	if (timeout == 0) {
		timeout = WAIT_VD_TIMOUT;
	}
	time = schedule_timeout(msecs_to_jiffies(timeout));
	remove_wait_queue(&pdev_info->wait_vd[id], &wait);
	*frame_count = isp_sync_id_sie[id];

	if (time == 0) {
		*frame_count = 0;
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "wait vd timeout (%d) (%d) \r\n", id, timeout);
		return;
	}
	#endif
}

void isp_dev_wait_frmend(ISP_ID id, UINT32 timeout, UINT32 *frame_count)
{
	#if defined(__KERNEL__)
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	DECLARE_WAITQUEUE(wait_frmend, current);
	INT32 time;

	if (pdev_info == NULL) {
		return;
	}
	if (id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return;
	}

	add_wait_queue(&pdev_info->wait_frmend[id], &wait_frmend);
	__set_current_state(TASK_INTERRUPTIBLE);
	if (timeout == 0) {
		timeout = WAIT_VD_TIMOUT;
	}
	time = schedule_timeout(msecs_to_jiffies(timeout));
	remove_wait_queue(&pdev_info->wait_frmend[id], &wait_frmend);
	*frame_count = sie_crop_end_cnt[id];

	if (time == 0) {
		*frame_count = 0;
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "wait frmend timeout (%d) (%d) \r\n", id, timeout);
		return;
	}
	#endif
}

void isp_dev_wait_cfgstr(ISP_ID id, UINT32 timeout, UINT32 *frame_count)
{
	#if defined(__KERNEL__)
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	DECLARE_WAITQUEUE(wait_cfgstr, current);
	INT32 time;

	if (pdev_info == NULL) {
		return;
	}
	if (id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return;
	}

	add_wait_queue(&pdev_info->wait_cfgstr[id], &wait_cfgstr);
	__set_current_state(TASK_INTERRUPTIBLE);
	if (timeout == 0) {
		timeout = WAIT_VD_TIMOUT;
	}
	time = schedule_timeout(msecs_to_jiffies(timeout));
	remove_wait_queue(&pdev_info->wait_cfgstr[id], &wait_cfgstr);
	*frame_count = isp_sync_id_ipp[id];

	if (time == 0) {
		*frame_count = 0;
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "wait cfgstr timeout (%d) (%d) \r\n", id, timeout);
		return;
	}
	#endif
}

void isp_dev_wait_procend(ISP_ID id, UINT32 timeout, UINT32 *frame_count)
{
	#if defined(__KERNEL__)
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	DECLARE_WAITQUEUE(wait_procend, current);
	INT32 time;

	if (pdev_info == NULL) {
		return;
	}
	if (id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return;
	}

	add_wait_queue(&pdev_info->wait_procend[id], &wait_procend);
	__set_current_state(TASK_INTERRUPTIBLE);
	if (timeout == 0) {
		timeout = WAIT_VD_TIMOUT;
	}
	time = schedule_timeout(msecs_to_jiffies(timeout));
	remove_wait_queue(&pdev_info->wait_procend[id], &wait_procend);
	*frame_count = ipp_proc_end_cnt[id];

	if (time == 0) {
		*frame_count = 0;
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "wait procend timeout (%d) (%d) \r\n", id, timeout);
		return;
	}
	#endif
}

#if defined(__FREERTOS)
ISP_DEV_INFO *isp_get_dev_info(void)
{
	return &pdev_info;
}
#else
//=============================================================================
// internal functions
//=============================================================================
INT32 isp_dev_construct(ISP_DEV_INFO *pdev_info)
{
	INT32 ret = 0;
	UINT32 i;

	// initialize synchronization mechanism
	sema_init(&pdev_info->api_mutex, 1);
	sema_init(&pdev_info->proc_mutex, 1);
	sema_init(&pdev_info->ioc_mutex, 1);

	// initialize event waitqueue
	for (i = 0; i < ISP_ID_MAX_NUM; i++) {
		init_waitqueue_head(&pdev_info->wait_vd[i]);
		init_waitqueue_head(&pdev_info->wait_frmend[i]);
		init_waitqueue_head(&pdev_info->wait_cfgstr[i]);
		init_waitqueue_head(&pdev_info->wait_procend[i]);
	}

	// registed callback function
	isp_api_reg_if(isp_id_list);
	isp_api_reg_enc_if();

	return ret;
}

void isp_dev_deconstruct(ISP_DEV_INFO *pdev_info)
{
	isp_api_unreg_if();
	isp_api_unreg_enc_if();
}

EXPORT_SYMBOL(isp_dev_get_sync_item);
EXPORT_SYMBOL(isp_dev_set_sync_item);
EXPORT_SYMBOL(isp_dev_get_ca);
EXPORT_SYMBOL(isp_dev_get_la);
EXPORT_SYMBOL(isp_dev_get_ipe_va);
EXPORT_SYMBOL(isp_dev_get_ipe_va_indep);
EXPORT_SYMBOL(isp_dev_get_ife_va);
EXPORT_SYMBOL(isp_dev_get_ife_va_indep);
EXPORT_SYMBOL(isp_dev_get_histo);
EXPORT_SYMBOL(isp_dev_get_defog_stcs);
EXPORT_SYMBOL(isp_dev_get_ipe_subout_buf);
EXPORT_SYMBOL(isp_dev_get_mtr_drv_tab);
EXPORT_SYMBOL(isp_dev_reg_ae_module);
EXPORT_SYMBOL(isp_dev_reg_af_module);
EXPORT_SYMBOL(isp_dev_reg_awb_module);
EXPORT_SYMBOL(isp_dev_reg_iq_module);
EXPORT_SYMBOL(isp_dev_reg_motor_driver);

#endif
