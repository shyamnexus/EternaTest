#if defined(__FREERTOS)
#include "string.h"
#include <stdio.h>
#include <kwrap/nvt_type.h>
#else
#include "kwrap/type.h"
#include "isp_builtin.h"
#endif
#include "kwrap/cpu.h"
#include <plat/top.h>

#include "isp_api_int.h"
#include "isp_include_int.h"
#include "isp_uti.h"
#include "isp_dbg.h"
#if defined(__FREERTOS)
#include "isp_dev_int.h"
#else
#include "kdrv_builtin/kdrv_builtin.h"
#include "isp_main.h"
#endif

//=============================================================================
// define
//=============================================================================
#define ISP_IF_REG_NAME             "NVT_ISP_IF"
#define ISP_IF_SIE_REG_EVENT        ISP_EVENT_VCAP_VD | ISP_EVENT_VCAP_CRPEND | ISP_EVENT_PARAM_RST
#define ISP_IF_IPP_REG_EVENT        ISP_EVENT_IPP_PROCEND | ISP_EVENT_IPP_CFGSTART | ISP_EVENT_IPP_CFGSTART_IMM | ISP_EVENT_PARAM_RST
#define ISP_IF_ENC_REG_EVENT        ISP_EVENT_ENC_RATIO | ISP_EVENT_ENC_SHARPEN | ISP_EVENT_PARAM_RST
#define ISP_IF_VIE_REG_EVENT        ISP_EVENT_VCAP_VD | ISP_EVENT_PARAM_RST
#define ISP_START_3A_FRAME_CNT_NUM  2

//=============================================================================
// global
//=============================================================================
static BOOL isp_id_valid[ISP_ID_MAX_NUM] = {0};
UINT32 isp_sync_id_sie[ISP_ID_MAX_NUM];
UINT32 isp_sync_id_ipp[ISP_ID_MAX_NUM];
UINT32 isp_sync_id_enc[ISP_ID_MAX_NUM];
UINT32 sie_crop_end_cnt[ISP_ID_MAX_NUM];
UINT32 ipp_proc_end_cnt[ISP_ID_MAX_NUM];

static BOOL yout_lock[ISP_ID_MAX_NUM] = {FALSE};
static BOOL raw_lock[ISP_ID_MAX_NUM] = {FALSE};
static UINT32 isp_src_id_mask[ISP_ID_MAX_NUM] = {0};
CTL_IPP_ISP_YUV_OUT ctl_ipp_isp_yuv_out;
CTL_SIE_ISP_HEADER_INFO ctl_isp_sie_header_info;
CTL_IPP_ISP_3DNR_STA ctl_ipp_isp_3dnr_sta;
static ISP_ENC_ISP_RATIO isp_enc_ratio[ISP_ID_MAX_NUM];
static ISP_SENSOR_INIT_INFO sensor_init_info;
static BOOL emu_enable = FALSE;
static BOOL isp_sync_ipp_indep[ISP_ID_MAX_NUM];
static ISP_SENSOR_MODE_MANUAL sensor_mode_manual[ISP_ID_MAX_NUM];
#if defined(__KERNEL__)
static BOOL is_fastboot[ISP_ID_MAX_NUM];
#endif
static BOOL sie_to_sen_id_map_en[ISP_ID_MAX_NUM];
static UINT32 sie_id_table[ISP_ID_MAX_NUM];
static UINT32 sie_to_sen_id_table[ISP_ID_MAX_NUM];
static UINT32 ipp_id_table[ISP_ID_MAX_NUM];
static UINT32 enc_id_table[ISP_ID_MAX_NUM];
static UINT32 low_power_lv;

//=============================================================================
// function declaration
//=============================================================================
static INT32 isp_api_cb_sie(ISP_ID id, ISP_EVENT evt, UINT32 frame_cnt, void *param);
static INT32 isp_api_cb_ipp(ISP_ID id, ISP_EVENT evt, UINT32 frame_cnt, void *param);
static INT32 isp_api_cb_enc(ISP_ID id, ISP_EVENT evt, UINT32 frame_cnt, void *param);
static INT32 isp_api_cb_vie(ISP_ID id, ISP_EVENT evt, UINT32 frame_cnt, void *param);
BOOL isp_get_id_valid(UINT32 id);
static ER isp_api_get_sensor_data_fmt(ISP_ID id, BOOL is_from_sie, UINT32 *data_fmt);
static ER isp_api_get_sensor_mode_type(ISP_ID id, BOOL is_from_sie, UINT32 *mode_type);
#if defined(__KERNEL__)
static void isp_api_get_fastboot_sync_info(UINT32 id);
#endif

//=============================================================================
// extern functions
//=============================================================================
void isp_api_reg_if(UINT32 id_list)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	UINT32 i;
	INT32 reg_sie_rt = 0;
	ER reg_ipp_rt = E_OK;
	INT32 reg_vie_rt = 0;

	reg_sie_rt = ctl_sie_isp_evt_fp_reg(ISP_IF_REG_NAME, &isp_api_cb_sie, ISP_IF_SIE_REG_EVENT, CTL_SIE_ISP_CB_MSG_NONE);
	if (reg_sie_rt != 0) {
		DBG_ERR("Reg sie ERROR \r\n");
	}

	reg_ipp_rt = ctl_ipp_isp_evt_fp_reg(ISP_IF_REG_NAME, &isp_api_cb_ipp, ISP_IF_IPP_REG_EVENT, CTL_IPP_ISP_CB_MSG_NONE);
	if (reg_ipp_rt != E_OK) {
		DBG_ERR("Reg ipp ERROR \r\n");
	}

	reg_vie_rt = ctl_vie_isp_evt_fp_reg(ISP_IF_REG_NAME, &isp_api_cb_vie, ISP_IF_VIE_REG_EVENT, CTL_VIE_ISP_CB_MSG_NONE);
	if (reg_vie_rt != 0) {
		DBG_ERR("Reg vie ERROR \r\n");
	}

	// Check size
	if ((sizeof(ISP_CA_RSLT) != sizeof(CTL_SIE_ISP_CA_RSLT)) ||
		(sizeof(ISP_LA_RSLT) != sizeof(CTL_SIE_ISP_LA_RSLT)) ||
		(sizeof(ISP_IFE_VA_RSLT) != sizeof(CTL_IPP_ISP_IFE_VA_RST)) ||
		(sizeof(ISP_IFE_VA_INDEP_RSLT) != sizeof(CTL_IPP_ISP_IFE_VA_INDEP_RST)) ||
		(sizeof(ISP_IPE_VA_RSLT) != sizeof(CTL_IPP_ISP_IPE_VA_RST)) ||
		(sizeof(ISP_IPE_VA_INDEP_RSLT) != sizeof(CTL_IPP_ISP_IPE_VA_INDEP_RST)) ||
		(sizeof(ISP_HISTO_RSLT) != sizeof(CTL_IPP_ISP_IFE_HIST_RST)) ||
		(sizeof(ISP_DEFOG_STCS) != sizeof(CTL_IPP_ISP_DEFOG_STCS))) {
		DBG_ERR("Size of statistics structure not match (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) \r\n", 
			(int)sizeof(ISP_CA_RSLT), (int)sizeof(CTL_SIE_ISP_CA_RSLT), (int)sizeof(ISP_LA_RSLT), (int)sizeof(CTL_SIE_ISP_LA_RSLT), 
			(int)sizeof(ISP_IFE_VA_RSLT), (int)sizeof(CTL_IPP_ISP_IFE_VA_RST), (int)sizeof(ISP_IFE_VA_INDEP_RSLT), (int)sizeof(CTL_IPP_ISP_IFE_VA_INDEP_RST), 
			(int)sizeof(ISP_IPE_VA_RSLT), (int)sizeof(CTL_IPP_ISP_IPE_VA_RST), (int)sizeof(ISP_IPE_VA_INDEP_RSLT), (int)sizeof(CTL_IPP_ISP_IPE_VA_INDEP_RST), 
			(int)sizeof(ISP_HISTO_RSLT), (int)sizeof(CTL_IPP_ISP_IFE_HIST_RST), (int)sizeof(ISP_HISTO_RSLT), (int)sizeof(CTL_IPP_ISP_IFE_HIST_RST), 
			(int)sizeof(ISP_DEFOG_STCS), (int)sizeof(CTL_IPP_ISP_DEFOG_STCS));
	}

	for (i = 0; i < ISP_ID_MAX_NUM; i++) {
		sie_id_table[i] = i;
		sie_to_sen_id_table[i] = i;
		ipp_id_table[i] = ISP_ID_IGNORE;
		enc_id_table[i] = ISP_ID_IGNORE;
		if ((id_list >> i) & 0x1) {
			isp_id_valid[i] = TRUE;
		} else {
			continue;
		}

		if (pdev_info != NULL) {
			pdev_info->ca_rslt[i] = isp_uti_vmem_alloc(sizeof(ISP_CA_RSLT));
			pdev_info->la_rslt[i] = isp_uti_vmem_alloc(sizeof(ISP_LA_RSLT));
			pdev_info->ipe_va_rslt[i] = isp_uti_vmem_alloc(sizeof(ISP_IPE_VA_RSLT));
			pdev_info->ipe_va_indep_rslt[i] = isp_uti_vmem_alloc(sizeof(ISP_IPE_VA_INDEP_RSLT));
			pdev_info->ife_va_rslt[i] = isp_uti_vmem_alloc(sizeof(ISP_IFE_VA_RSLT));
			pdev_info->ife_va_indep_rslt[i] = isp_uti_vmem_alloc(sizeof(ISP_IFE_VA_INDEP_RSLT));
			pdev_info->histo_rslt[i] = isp_uti_vmem_alloc(sizeof(ISP_HISTO_RSLT));
			pdev_info->ipe_subout_buf[i] = isp_uti_vmem_alloc(sizeof(ISP_IPE_SUBOUT_BUF));
			pdev_info->p_sync_valid[i] = isp_uti_vmem_alloc(ISP_SYNC_ITEM_MAX * SYNC_INFO_BUF * sizeof(BOOL));
			pdev_info->p_sync_info[i] = isp_uti_vmem_alloc(SYNC_INFO_BUF * sizeof(ISP_SYNC_INFO));

			memset(pdev_info->ca_rslt[i], 0x0, sizeof(ISP_CA_RSLT));
			memset(pdev_info->la_rslt[i], 0x0, sizeof(ISP_LA_RSLT));
			memset(pdev_info->ipe_va_rslt[i], 0x0, sizeof(ISP_IPE_VA_RSLT));
			memset(pdev_info->ipe_va_indep_rslt[i], 0x0, sizeof(ISP_IPE_VA_INDEP_RSLT));
			memset(pdev_info->ife_va_rslt[i], 0x0, sizeof(ISP_IFE_VA_RSLT));
			memset(pdev_info->ife_va_indep_rslt[i], 0x0, sizeof(ISP_IFE_VA_INDEP_RSLT));
			memset(pdev_info->histo_rslt[i], 0x0, sizeof(ISP_HISTO_RSLT));
			memset(pdev_info->ipe_subout_buf[i], 0x0, sizeof(ISP_IPE_SUBOUT_BUF));
			memset(pdev_info->p_sync_valid[i], 0x0, ISP_SYNC_ITEM_MAX * SYNC_INFO_BUF * sizeof(BOOL));
			memset(pdev_info->p_sync_info[i], 0x0, SYNC_INFO_BUF * sizeof(ISP_SYNC_INFO));
		}

		#if defined(__KERNEL__)
		if (i < ISP_BUILTIN_ID_MAX_NUM) {
			is_fastboot[i] = isp_api_get_fastboot_valid();
			if (is_fastboot[i]) {
				isp_api_get_fastboot_sync_info(i);
			}
		}
		#endif
	}
}

void isp_api_unreg_if(void)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	UINT32 i;
	INT32 reg_sie_rt = 0;
	ER reg_ipp_rt = E_OK;
	INT32 reg_vie_rt = 0;

	reg_sie_rt = ctl_sie_isp_evt_fp_unreg(ISP_IF_REG_NAME);
	if (reg_sie_rt != 0) {
		DBG_ERR("Unreg sie ERROR \r\n");
	}

	reg_ipp_rt = ctl_ipp_isp_evt_fp_unreg(ISP_IF_REG_NAME);
	if (reg_ipp_rt != E_OK) {
		DBG_ERR("Unreg ipp ERROR \r\n");
	}

	reg_vie_rt = ctl_vie_isp_evt_fp_unreg(ISP_IF_REG_NAME);
	if (reg_vie_rt != E_OK) {
		DBG_ERR("Unreg vie ERROR \r\n");
	}

	if (pdev_info == NULL) {
		DBG_ERR("pdev_info NULL \r\n");
		return;
	}

	for (i = 0; i < ISP_ID_MAX_NUM; i++) {
		if (isp_id_valid[i] && (pdev_info != NULL)) {
			isp_uti_vmem_free(pdev_info->ca_rslt[i]);
			isp_uti_vmem_free(pdev_info->la_rslt[i]);
			isp_uti_vmem_free(pdev_info->ipe_va_rslt[i]);
			isp_uti_vmem_free(pdev_info->ipe_va_indep_rslt[i]);
			isp_uti_vmem_free(pdev_info->ife_va_rslt[i]);
			isp_uti_vmem_free(pdev_info->ife_va_indep_rslt[i]);
			isp_uti_vmem_free(pdev_info->histo_rslt[i]);
			isp_uti_vmem_free(pdev_info->ipe_subout_buf[i]);
			isp_uti_vmem_free(pdev_info->p_sync_valid[i]);
			isp_uti_vmem_free(pdev_info->p_sync_info[i]);
			pdev_info->ca_rslt[i] = NULL;
			pdev_info->la_rslt[i] = NULL;
			pdev_info->ipe_va_rslt[i] = NULL;
			pdev_info->ipe_va_indep_rslt[i] = NULL;
			pdev_info->ife_va_rslt[i] = NULL;
			pdev_info->ife_va_indep_rslt[i] = NULL;
			pdev_info->histo_rslt[i] = NULL;
			pdev_info->ipe_subout_buf[i] = NULL;
			pdev_info->p_sync_valid[i] = NULL;
			pdev_info->p_sync_info[i] = NULL;
		}

		if (pdev_info->histo_rslt_cap[i] != NULL) {
			isp_uti_vmem_free(pdev_info->histo_rslt_cap[i]);
			pdev_info->histo_rslt_cap[i] = NULL;
		}

		if (pdev_info->ipe_subout_buf_cap[i] != NULL) {
			isp_uti_vmem_free(pdev_info->ipe_subout_buf_cap[i]);
			pdev_info->ipe_subout_buf_cap[i] = NULL;
		}
	}
}

void isp_api_reg_enc_if(void)
{
	UINT32 i;
	ER reg_enc_rt = E_OK;

	reg_enc_rt = kflow_videoenc_evt_fp_reg(ISP_IF_REG_NAME, &isp_api_cb_enc, ISP_IF_ENC_REG_EVENT, KFLOW_VIDEOENC_WAIT_FLG_NONE);
	if (reg_enc_rt != E_OK) {
		DBG_ERR("Reg enc ERROR \r\n");
	}

	// Initial global value
	for (i = 0; i < ISP_ID_MAX_NUM; i++) {
		isp_enc_ratio[i].enc_edge_ratio = 256;
		isp_enc_ratio[i].enc_2dnr_ratio = 256;
		isp_enc_ratio[i].enc_3dnr_ratio = 256;
	}
}

void isp_api_unreg_enc_if(void)
{
	ER reg_enc_rt = E_OK;

	reg_enc_rt = kflow_videoenc_evt_fp_unreg(ISP_IF_REG_NAME);
	if (reg_enc_rt != E_OK) {
		DBG_ERR("Unreg enc ERROR \r\n");
	}
}

static ER isp_api_check_id_mask(UINT32 mask, UINT32 frame_num)
{
	ER rt = E_OK;
	UINT32 i, cnt = 0;

	for (i = 0; i < ISP_ID_MAX_NUM; i++) {
		if (mask & (0x1 << i)) {
			cnt++;
		}
	}
	if (cnt != frame_num) {
		rt = E_SYS;
	}

	return rt;
}

static INT32 isp_api_cb_sie(ISP_ID id, ISP_EVENT evt, UINT32 frame_cnt, void *param)
{
	BOOL sie_to_sen_id_map_enable = (id >> 31) & 0x1; // 1 bit
	UINT32 sen_id = (id >> 24) & 0x7F;                // 7 bit
	UINT32 sie_id = id & 0xFFFFFF;                    // 24 bit
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	ISP_AE_TRIG_OBJ ae_trig_obj;
	ISP_AWB_TRIG_OBJ awb_trig_obj;
	ISP_IQ_TRIG_OBJ iq_trig_obj;
	static ISP_FUNC_EN fun_en[SIE_ID_MAX_NUM] = {0};
	ISP_FUNC_EN fun_en_tmp = 0;
	static UINT32 frm_grp[SIE_ID_MAX_NUM];
	UINT32 frm_grp_tmp = 0;
	static UINT32 src_id[SIE_ID_MAX_NUM] = {0};
	UINT32 src_id_tmp = 0;
	UINT32 frame_num = 1, combine_num = 1, trig_num, i;
	static UINT32 ae_src_id_cnt[SIE_ID_MAX_NUM] = {0};
	static UINT32 awb_src_id_cnt[SIE_ID_MAX_NUM] = {0};
	UINT32 start_3a_frame_cnt_num = ISP_START_3A_FRAME_CNT_NUM;
	static UINT32 start_3a_frame_cnt[SIE_ID_MAX_NUM] = {0};
	UINT32 data_fmt = ISP_SEN_DATA_FMT_RGB, mode_type = ISP_SEN_MODE_LINEAR;
	UINT32 dbg_mode = isp_dbg_get_dbg_mode(sie_id);
	INT32 sie_ret = CTL_SIE_E_OK;

	if (!isp_id_valid[sie_id]) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(sie_id) & ISP_DBG_WRN_MSG, "id list not valid (%d) \r\n", sie_id);
		return 0;
	}

	if (pdev_info == NULL) {
		return 0;
	}
	if (sie_id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", id);
		return 0;
	}

	sie_to_sen_id_map_en[sie_id] = sie_to_sen_id_map_enable;
	sie_to_sen_id_table[sie_id] = sen_id;
	sie_id_table[sie_id] = id;

	sie_ret = ctl_sie_isp_get(id, CTL_SIE_ISP_ITEM_FUNC_EN, &fun_en_tmp);
	if (sie_ret == CTL_SIE_E_OK) {
		fun_en[sie_id] = fun_en_tmp;
	} else {
		PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "get CTL_SIE_ISP_ITEM_FUNC_EN fail (%d) (%d) (0x%X) \r\n", sie_id, sie_ret, fun_en_tmp);
	}

	sie_ret = ctl_sie_isp_get(id, CTL_SIE_ISP_ITEM_MULTI_FRM_GRP, &frm_grp_tmp);
	if (sie_ret == CTL_SIE_E_OK) {
		frm_grp[sie_id] = frm_grp_tmp;
	} else {
		PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "get CTL_SIE_ISP_ITEM_MULTI_FRM_GRP fail (%d) (%d) (0x%X) \r\n", sie_id, sie_ret, frm_grp_tmp);
	}

	sie_ret = ctl_sie_isp_get(id, CTL_SIE_ISP_ITEM_DUPL_SRC_ID, &src_id_tmp);
	if ((sie_ret == CTL_SIE_E_OK) && (src_id_tmp < SIE_ID_MAX_NUM)) {
		src_id[sie_id] = src_id_tmp;
	} else {
		PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "get CTL_SIE_ISP_ITEM_DUPL_SRC_ID fail (%d) (%d) (%d) \r\n", sie_id, sie_ret, src_id_tmp);
	}

	isp_api_get_frame_num(src_id[sie_id], TRUE, &frame_num);
	if (fun_en[sie_id] & ISP_FUNC_EN_SHDR) {
		if (frame_num == 1) {
			PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "SHDR frame number must > 1 (%d) \r\n", frame_num);
		}

		start_3a_frame_cnt_num = frame_num * ISP_START_3A_FRAME_CNT_NUM - 1;
	}

	if (fun_en[sie_id] & ISP_FUNC_EN_COMBINE) {
		combine_num = 2;
	}

	trig_num = frame_num * combine_num;

	isp_sync_id_sie[sie_id] = frame_cnt;

	if ((evt & ISP_EVENT_PARAM_RST) || (evt & ISP_EVENT_VCAP_VD)) {
		isp_dev_reset_item_valid(sie_id, frame_cnt);
		isp_dev_save_frame_count(sie_id);

		#if defined(__KERNEL__)
		wake_up_interruptible_all(&pdev_info->wait_vd[sie_id]);
		#endif

		if (start_3a_frame_cnt[sie_id] < start_3a_frame_cnt_num) {
			start_3a_frame_cnt[sie_id]++;
		}

		if (evt & ISP_EVENT_PARAM_RST) {
			#if defined(__KERNEL__)
			if (is_fastboot[sie_id]) {
				if (sie_id < ISP_BUILTIN_ID_MAX_NUM) {
					isp_api_get_fastboot_sync_info(sie_id);
					is_fastboot[sie_id] = FALSE;
				} else {
					PRINT_ISP_WRN(isp_dbg_get_dbg_mode(sie_id) & ISP_DBG_WRN_MSG, "fastboot does not support id (%d) \r\n", sie_id);
				}
			}
			#endif

			start_3a_frame_cnt[sie_id] = 0;
			for (i = 0; i < SYNC_INFO_BUF; i++) {
				isp_dev_reset_item_valid(sie_id, i);
			}
			ae_trig_obj.reset = TRUE;
			awb_trig_obj.reset = TRUE;
			iq_trig_obj.msg = ISP_TRIG_IQ_SIE_IMM;
			iq_trig_obj.sie.reset = TRUE;
			PRINT_ISP(dbg_mode & ISP_DBG_SIE_CB, "sie rst,   id = %8X, frm = %d, 3A start %d/%d \r\n", id, frame_cnt, start_3a_frame_cnt[sie_id], start_3a_frame_cnt_num);
		} else {
			ae_trig_obj.reset = FALSE;
			awb_trig_obj.reset = FALSE;
			iq_trig_obj.msg = ISP_TRIG_IQ_SIE;
			iq_trig_obj.sie.reset = FALSE;
			PRINT_ISP(dbg_mode & ISP_DBG_SIE_CB, "sie cb,    id = %8X, frm = %d, 3A start %d/%d \r\n", id, frame_cnt, start_3a_frame_cnt[sie_id], start_3a_frame_cnt_num);
		}

		isp_src_id_mask[src_id[sie_id]] = frm_grp[sie_id];
		isp_api_get_sensor_data_fmt(src_id[sie_id], TRUE, &data_fmt);
		isp_api_get_sensor_mode_type(src_id[sie_id], TRUE, &mode_type);

		if ((pdev_info->isp_ae != NULL) && (fun_en[sie_id] & ISP_FUNC_EN_AE) && ((start_3a_frame_cnt[sie_id] >= start_3a_frame_cnt_num) || (evt & ISP_EVENT_PARAM_RST))) {
			if ((fun_en[sie_id] & ISP_FUNC_EN_SHDR) || (fun_en[sie_id] & ISP_FUNC_EN_COMBINE)) {
				ae_src_id_cnt[src_id[sie_id]]++;
				if (ae_src_id_cnt[src_id[sie_id]] == trig_num) {
					PRINT_ISP(dbg_mode & ISP_DBG_SIE_CB, "sie vd,    id = %8X, frm = %d, src id = %d, cnt = %d, mask = %d \r\n", id, frame_cnt, src_id[sie_id], ae_src_id_cnt[src_id[sie_id]], frm_grp[src_id[sie_id]]);
					ae_trig_obj.func_en = fun_en[sie_id];
					ae_trig_obj.src_id_mask = frm_grp[sie_id];
					ae_trig_obj.mode_type = mode_type;
					ae_trig_obj.frame_num = frame_num;
					ae_trig_obj.low_power_lv = low_power_lv;
					if (isp_api_check_id_mask(ae_trig_obj.src_id_mask, trig_num) == E_OK) {
						pdev_info->isp_ae->fn_trig(src_id[sie_id], &ae_trig_obj);
					}
					ae_src_id_cnt[src_id[sie_id]] = 0;
				}
			} else {
				PRINT_ISP(dbg_mode & ISP_DBG_SIE_CB, "sie vd,    id = %8X, frm = %d \r\n", id, frame_cnt);
				ae_trig_obj.func_en = fun_en[sie_id];
				ae_trig_obj.src_id_mask = 1 << sie_id;
				ae_trig_obj.mode_type = mode_type;
				ae_trig_obj.frame_num = frame_num;
				ae_trig_obj.low_power_lv = low_power_lv;
				pdev_info->isp_ae->fn_trig(sie_id, &ae_trig_obj);
			}
		}
		if ((pdev_info->isp_awb != NULL) && (fun_en[sie_id] & ISP_FUNC_EN_AWB) && ((start_3a_frame_cnt[sie_id] >= start_3a_frame_cnt_num) || (evt & ISP_EVENT_PARAM_RST))) {
			if ((fun_en[sie_id] & ISP_FUNC_EN_SHDR) || (fun_en[sie_id] & ISP_FUNC_EN_COMBINE)) {
				awb_src_id_cnt[src_id[sie_id]]++;
				if (awb_src_id_cnt[src_id[sie_id]] == trig_num) {
					awb_trig_obj.func_en = fun_en[sie_id];
					awb_trig_obj.src_id_mask = frm_grp[sie_id];
					awb_trig_obj.data_fmt = data_fmt;
					awb_trig_obj.mode_type = mode_type;
					awb_trig_obj.frame_num = frame_num;
					awb_trig_obj.low_power_lv = low_power_lv;
					if (isp_api_check_id_mask(awb_trig_obj.src_id_mask, trig_num) == E_OK) {
						pdev_info->isp_awb->fn_trig(src_id[sie_id], &awb_trig_obj);
					}
					awb_src_id_cnt[src_id[sie_id]] = 0;
				}
			} else {
				awb_trig_obj.func_en = fun_en[sie_id];
				awb_trig_obj.src_id_mask =  1 << sie_id;
				awb_trig_obj.data_fmt = data_fmt;
				awb_trig_obj.mode_type = mode_type;
				awb_trig_obj.frame_num = frame_num;
				awb_trig_obj.low_power_lv = low_power_lv;
				pdev_info->isp_awb->fn_trig(sie_id, &awb_trig_obj);
			}
		}

		if (pdev_info->isp_iq != NULL) {
			iq_trig_obj.sie.data_fmt = data_fmt;
			iq_trig_obj.sie.mode_type = mode_type;
			iq_trig_obj.sie.frame_num = frame_num;
			iq_trig_obj.sie.func_en = fun_en[sie_id];
			iq_trig_obj.sie.dupl_src_id = src_id[sie_id];
			iq_trig_obj.sie.src_id_mask = frm_grp[sie_id];
			iq_trig_obj.sie.low_power_lv = low_power_lv;
			pdev_info->isp_iq->fn_trig(sie_id, &iq_trig_obj);
		}
	}

	if (evt & ISP_EVENT_VCAP_CRPEND) {
		#if defined(__KERNEL__)
		wake_up_interruptible_all(&pdev_info->wait_frmend[sie_id]);
		#endif
		sie_crop_end_cnt[sie_id] = frame_cnt;
	}

	return 0;
}

static INT32 isp_api_cb_ipp(ISP_ID id, ISP_EVENT evt, UINT32 frame_cnt, void *param)
{
	BOOL ipp_to_sen_id_map_enable = (id >> 31) & 0x1; // 1 bit
	UINT32 sen_id = (id >> 24) & 0x7F;                // 7 bit
	UINT32 ipp_info = (id >> 16) & 0xFF;              // 8 bit
	BOOL ipp_to_sie_id_map_enable = (id >> 15) & 0x1; // 1 bit
	UINT32 sie_id = (id >> 8) & 0x7F;                 // 7 bit
	UINT32 ipp_id = id & 0xFF;                        // 8 bit
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	ISP_AF_TRIG_OBJ af_trig_obj;
	ISP_IQ_TRIG_OBJ iq_trig_obj;
	ISP_FUNC_EN fun_en = 0;
	CTL_IPP_ISP_STATUS_INFO ipp_isp_status_info = {0};
	CTL_IPP_ISP_DEFOG_SUBOUT ipp_isp_defog_subout = {0};
	UINT32 data_fmt = ISP_SEN_DATA_FMT_RGB, mode_type = ISP_SEN_MODE_LINEAR, frame_num = 1;
	UINT32 start_3a_frame_cnt_num = ISP_START_3A_FRAME_CNT_NUM;
	static UINT32 start_3a_frame_cnt[ISP_ID_MAX_NUM] = {0};
	UINT32 dbg_mode = isp_dbg_get_dbg_mode(ipp_id);
	BOOL emu_en = isp_api_get_emu_enable();
	ER rt = E_OK;
	UINT32 i;
	INT32 j;

	if (!isp_id_valid[ipp_id]) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(ipp_id) & ISP_DBG_WRN_MSG, "id list not valid (%d) \r\n", ipp_id);
		return 0;
	}

	if (pdev_info == NULL) {
		return 0;
	}

	if (ipp_id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "isp id out of range (0x%x) \r\n", id);
		return 0;
	}

	if (ipp_info >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "ipp info out of range (0x%x) \r\n", ipp_info);
		return 0;
	}

	if (evt == ISP_EVENT_PARAM_RST) {
		for (i = 0; i < ISP_ID_MAX_NUM; i++) {
			ipp_id_table[i] = ISP_ID_IGNORE;
			pdev_info->ipp_to_sie_id_table[i] = ISP_ID_IGNORE;
			pdev_info->enc_to_sie_id_table[i] = ISP_ID_IGNORE;
		}
	}

	// Allocate buffer for capture usages
	if ((ipp_info != ipp_id) && (evt == ISP_EVENT_IPP_CFGSTART_IMM)) {
		pdev_info->is_capture[ipp_id] = TRUE;
		pdev_info->cap_id_mapping_table[ipp_id] = ipp_info;

		if (pdev_info->histo_rslt_cap[ipp_info] == NULL) {
			pdev_info->histo_rslt_cap[ipp_info] = isp_uti_vmem_alloc(sizeof(ISP_HISTO_RSLT));
		}

		if (pdev_info->ipe_subout_buf_cap[ipp_info] == NULL) {
			pdev_info->ipe_subout_buf_cap[ipp_info] = isp_uti_vmem_alloc(sizeof(ISP_IPE_SUBOUT_BUF));
		}
	} else {
		pdev_info->is_capture[ipp_id] = FALSE;
	}

	pdev_info->ipp_to_sen_id_map_en[ipp_id] = ipp_to_sen_id_map_enable;
	pdev_info->ipp_to_sen_id_table[ipp_id] = sen_id;
	pdev_info->ipp_to_sie_id_map_en[ipp_id] = ipp_to_sie_id_map_enable;
	ipp_id_table[ipp_id] = id;
	pdev_info->ipp_to_sie_id_table[ipp_id] = sie_id;

	rt = ctl_ipp_isp_get(id, CTL_IPP_ISP_ITEM_STATUS_INFO, &ipp_isp_status_info);
	if (rt != E_OK) {
		PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "get CTL_IPP_ISP_ITEM_STATUS_INFO fail (%d) (%d) (%d) (%d) \r\n", id, pdev_info->is_capture[ipp_id], frame_cnt, rt);
	}
	if (((ipp_isp_status_info.flow == CTL_IPP_ISP_FLOW_CAPTURE_RAW) || (ipp_isp_status_info.flow == CTL_IPP_ISP_FLOW_CAPTURE_CCIR)) && (evt != ISP_EVENT_IPP_CFGSTART_IMM)) {
		// capture mode with ISP_EVENT_PARAM_RST/ISP_EVENT_IPP_CFGSTART
		return 0;
	}
	if (((ipp_isp_status_info.flow != CTL_IPP_ISP_FLOW_CAPTURE_RAW) && (ipp_isp_status_info.flow != CTL_IPP_ISP_FLOW_CAPTURE_CCIR)) && (evt == ISP_EVENT_IPP_CFGSTART_IMM)) {
		// preview mode with ISP_EVENT_IPP_CFGSTART_IMM
		return 0;
	}

	rt = ctl_ipp_isp_get(id, CTL_IPP_ISP_ITEM_FUNC_EN, &fun_en);
	if (rt != E_OK) {
		PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "get CTL_IPP_ISP_ITEM_FUNC_EN fail, do not trigger (%d) (%d) (%d) (%d) \r\n", id, pdev_info->is_capture[ipp_id], frame_cnt, rt);
		return 0;
	}

	isp_sync_id_ipp[ipp_id] = frame_cnt;
	if ((fun_en & ISP_FUNC_EN_SHDR) && (isp_src_id_mask[ipp_id] != 0)) {
		for (j = ISP_ID_MAX_NUM - 1; j >= 0; j--) {
			if ((0x1 << j) & isp_src_id_mask[ipp_id]) {
				break;
			}
		}

		if (j != (INT32)ipp_id) {
			isp_sync_id_ipp[j] = frame_cnt;
		}
	}
	if ((emu_en) || (isp_sync_ipp_indep[ipp_id] == TRUE)) {
		// isp_frame_count update for emulation
		isp_dev_save_frame_count(ipp_id);
	}

	isp_api_get_sensor_data_fmt(ipp_id, FALSE, &data_fmt);
	isp_api_get_sensor_mode_type(ipp_id, FALSE, &mode_type);
	isp_api_get_frame_num(ipp_id, FALSE, &frame_num);

	if (evt & ISP_EVENT_IPP_CFGSTART) {
		#if defined(__KERNEL__)
		wake_up_interruptible_all(&pdev_info->wait_cfgstr[ipp_id]);
		#endif
	}

	switch (evt) {
	case ISP_EVENT_PARAM_RST:
	case ISP_EVENT_IPP_CFGSTART:
	case ISP_EVENT_IPP_CFGSTART_IMM:
		if (evt == ISP_EVENT_PARAM_RST) {
			#if defined(__KERNEL__)
			if (is_fastboot[ipp_id]) {
				if (ipp_id < ISP_BUILTIN_ID_MAX_NUM) {
					isp_api_get_fastboot_sync_info(ipp_id);
					is_fastboot[ipp_id] = FALSE;
				} else {
					PRINT_ISP_WRN(isp_dbg_get_dbg_mode(sie_id) & ISP_DBG_WRN_MSG, "fastboot does not support id (%d) \r\n", sie_id);
				}
			}
			#endif

			start_3a_frame_cnt[ipp_id] = 0;
			for (i = 0; i < SYNC_INFO_BUF; i++) {
				isp_dev_reset_item_valid(ipp_id, i);
			}

			if ((fun_en & ISP_FUNC_EN_SHDR) && (isp_src_id_mask[ipp_id] != 0)) {
				for (j = ISP_ID_MAX_NUM - 1; j >= 0; j--) {
					if ((0x1 << j) & isp_src_id_mask[ipp_id]) {
						break;
					}
				}
				if (j != (INT32)ipp_id) {
					for (i = 0; i < SYNC_INFO_BUF; i++) {
						isp_dev_reset_item_valid(j, i);
					}
				}
			}

			iq_trig_obj.msg = ISP_TRIG_IQ_IPP_IMM;
			iq_trig_obj.ipp.reset = TRUE;
			iq_trig_obj.ipp.ipp_capture = FALSE;
			PRINT_ISP(dbg_mode & ISP_DBG_IPP_CB, "ipp rst,   id = %8X, frm = %d, func = 0x%x, 3A start %d/%d \r\n", id, frame_cnt, fun_en, start_3a_frame_cnt[ipp_id], start_3a_frame_cnt_num);
		} else if (evt == ISP_EVENT_IPP_CFGSTART) {

			if (start_3a_frame_cnt[ipp_id] < start_3a_frame_cnt_num) {
				start_3a_frame_cnt[ipp_id]++;
			}

			iq_trig_obj.msg = ISP_TRIG_IQ_IPP;
			if (emu_en) {
				iq_trig_obj.ipp.reset = TRUE;
			} else {
				iq_trig_obj.ipp.reset = FALSE;
			}
			iq_trig_obj.ipp.ipp_capture = FALSE;
			if (ipp_info == 0) {
				PRINT_ISP(dbg_mode & ISP_DBG_IPP_CB, "ipp cb_M,  id = %8X, frm = %d, func = 0x%x, 3A start %d/%d \r\n", id, frame_cnt, fun_en, start_3a_frame_cnt[ipp_id], start_3a_frame_cnt_num);
			} else {
				PRINT_ISP(dbg_mode & ISP_DBG_IPP_CB, "ipp cb_S,  id = %8X, frm = %d, func = 0x%x, 3A start %d/%d \r\n", id, frame_cnt, fun_en, start_3a_frame_cnt[ipp_id], start_3a_frame_cnt_num);
			}
		} else if (evt == ISP_EVENT_IPP_CFGSTART_IMM) {
			iq_trig_obj.msg = ISP_TRIG_IQ_IPP_IMM;
			iq_trig_obj.ipp.reset = FALSE;
			iq_trig_obj.ipp.ipp_capture = TRUE;
			PRINT_ISP(dbg_mode & ISP_DBG_IPP_CB, "ipp cb_C,  id = %8X, frm = %d, func = 0x%x, 3A start %d/%d \r\n", id, frame_cnt, fun_en, start_3a_frame_cnt[ipp_id], start_3a_frame_cnt_num);
		}

		if (fun_en & ISP_FUNC_EN_WDR) {
			if (pdev_info->is_capture[ipp_id]) {
				rt = ctl_ipp_isp_get(id, CTL_IPP_ISP_ITEM_IFE_HIST_RST, pdev_info->histo_rslt_cap[ipp_info]);
			} else if ((evt != ISP_EVENT_PARAM_RST) || isp_api_get_fastboot_valid()) {
				rt = ctl_ipp_isp_get(id, CTL_IPP_ISP_ITEM_IFE_HIST_RST, pdev_info->histo_rslt[ipp_id]);
			} else {
				rt = E_OK;
			}
			if (rt != E_OK) {
				PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "get CTL_IPP_ISP_ITEM_IFE_HIST_RST fail (%d) (%d) (%d) (%d) \r\n", id, pdev_info->is_capture[ipp_id], frame_cnt, rt);
			}
		}

		if (fun_en & ISP_FUNC_EN_DEFOG) {
			if (pdev_info->is_capture[ipp_id]) {
				rt = ctl_ipp_isp_get(id, CTL_IPP_ISP_ITEM_DEFOG_STCS, &pdev_info->defog_stcs_cap[ipp_info]);
			} else if ((evt != ISP_EVENT_PARAM_RST) || isp_api_get_fastboot_valid()) {
				rt = ctl_ipp_isp_get(id, CTL_IPP_ISP_ITEM_DEFOG_STCS, &pdev_info->defog_stcs[ipp_id]);
			} else {
				rt = E_OK;
			}
			if (rt != E_OK) {
				PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "get CTL_IPP_ISP_ITEM_DEFOG_STCS fail (%d) (%d) (%d) (%d) \r\n", id, pdev_info->is_capture[ipp_id], frame_cnt, rt);
			}

			if (evt != ISP_EVENT_PARAM_RST) {
				rt = ctl_ipp_isp_get(id, CTL_IPP_ISP_ITEM_DEFOG_SUBOUT, &ipp_isp_defog_subout);
				if (rt != E_OK) {
					PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "get CTL_IPP_ISP_ITEM_DEFOG_SUBOUT fail (%d) (%d) (%d) (%d) \r\n", id, pdev_info->is_capture[ipp_id], frame_cnt, rt);
				}

				if (ipp_isp_defog_subout.addr != 0) {
					for (i = 0; i < ISP_SUBOUT_MAX_SIZE; i++) {
						if (pdev_info->is_capture[ipp_id]) {
							pdev_info->ipe_subout_buf_cap[ipp_info]->min[i] = *((UINT32 *)ipp_isp_defog_subout.addr + i) & 0x3FF;
							pdev_info->ipe_subout_buf_cap[ipp_info]->avg[i] = *((UINT32 *)ipp_isp_defog_subout.addr + i) >> 16 & 0x3FF;
						} else {
							pdev_info->ipe_subout_buf[ipp_id]->min[i] = *((UINT32 *)ipp_isp_defog_subout.addr + i) & 0x3FF;
							pdev_info->ipe_subout_buf[ipp_id]->avg[i] = *((UINT32 *)ipp_isp_defog_subout.addr + i) >> 16 & 0x3FF;
						}
					}
				}
			}
		}

		if ((pdev_info->isp_af != NULL) && (fun_en & ISP_FUNC_EN_AF) && (evt == ISP_EVENT_PARAM_RST)) {
			af_trig_obj.reset = TRUE;
			af_trig_obj.func_en = fun_en;
			af_trig_obj.mode_type = mode_type;
			af_trig_obj.frame_num = frame_num;
			af_trig_obj.low_power_lv = low_power_lv;
			pdev_info->isp_af->fn_trig(ipp_id, &af_trig_obj);
		}

		if (pdev_info->isp_iq != NULL) {
			if (ipp_isp_status_info.flow == CTL_IPP_ISP_FLOW_DIRECT_RAW) {
				iq_trig_obj.ipp.ipp_direct = TRUE;
			} else {
				iq_trig_obj.ipp.ipp_direct = FALSE;
			}
			iq_trig_obj.ipp.data_fmt = data_fmt;
			iq_trig_obj.ipp.mode_type = mode_type;
			iq_trig_obj.ipp.frame_num = frame_num;
			if ((ipp_isp_status_info.flow == CTL_IPP_ISP_FLOW_CCIR) || (ipp_isp_status_info.flow == CTL_IPP_ISP_FLOW_CAPTURE_CCIR)) {
				iq_trig_obj.ipp.flow_type = ISP_FLOW_CCIR;
			} else {
				iq_trig_obj.ipp.flow_type = ISP_FLOW_RAW;
			}
			iq_trig_obj.ipp.func_en = fun_en;
			iq_trig_obj.ipp.ipp_info = ipp_info;
			iq_trig_obj.ipp.low_power_lv = low_power_lv;
			pdev_info->isp_iq->fn_trig(ipp_id, &iq_trig_obj);
		}
		break;

		case ISP_EVENT_IPP_PROCEND:
			#if defined(__KERNEL__)
			wake_up_interruptible_all(&pdev_info->wait_procend[ipp_id]);
			#endif
			ipp_proc_end_cnt[ipp_id] = frame_cnt;
			PRINT_ISP(dbg_mode & ISP_DBG_IPP_CB, "ipp  end,  id = %8X, frm = %d \r\n", id, frame_cnt);

			if (fun_en & ISP_FUNC_EN_AF) {
				rt = ctl_ipp_isp_get(id, CTL_IPP_ISP_ITEM_IFE_VA_RST, pdev_info->ife_va_rslt[ipp_id]);
				rt |= ctl_ipp_isp_get(id, CTL_IPP_ISP_ITEM_IFE_VA_INDEP_RST, pdev_info->ife_va_indep_rslt[ipp_id]);
				if (rt != E_OK) {
					PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "get CTL_IPP_ISP_ITEM_IFE_VA_RST fail (%d) (%d) (%d) (%d) \r\n", id, pdev_info->is_capture[ipp_id], frame_cnt, rt);
				}
				rt = ctl_ipp_isp_get(id, CTL_IPP_ISP_ITEM_IPE_VA_RST, pdev_info->ipe_va_rslt[ipp_id]);
				rt |= ctl_ipp_isp_get(id, CTL_IPP_ISP_ITEM_IPE_VA_INDEP_RST, pdev_info->ipe_va_indep_rslt[ipp_id]);
				if (rt != E_OK) {
					PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "get CTL_IPP_ISP_ITEM_IPE_VA_RST fail (%d) (%d) (%d) (%d) \r\n", id, pdev_info->is_capture[ipp_id], frame_cnt, rt);
				}
			}

			if ((pdev_info->isp_af != NULL) && (fun_en & ISP_FUNC_EN_AF)) {
				af_trig_obj.reset = FALSE;
				af_trig_obj.func_en = fun_en;
				af_trig_obj.mode_type = mode_type;
				af_trig_obj.frame_num = frame_num;
				pdev_info->isp_af->fn_trig(ipp_id, &af_trig_obj);
			}
			break;

	default:
		PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "ISP_EVENT out of range (%d) \r\n", evt);
		break;
	}
	return 0;
}

static INT32 isp_api_cb_enc(ISP_ID id, ISP_EVENT evt, UINT32 frame_cnt, void *param)
{
	BOOL enc_to_sen_id_map_enable = (id >> 31) & 0x1; // 1 bit
	UINT32 sen_id = (id >> 24) & 0x7F;                // 7 bit
	BOOL enc_to_sie_id_map_enable = (id >> 15) & 0x1; // 1 bit
	UINT32 sie_id = (id >> 8) & 0x7F;                 // 7 bit
	UINT32 enc_id = id & 0xFF;                        // 8 bit
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	ISP_IQ_TRIG_OBJ iq_trig_obj;
	KDRV_VDOENC_ISP_RATIO enc_isp_ratio = {0};
	UINT32 dbg_mode = isp_dbg_get_dbg_mode(enc_id);
	ER rt = E_OK;

	if (pdev_info == NULL) {
		return 0;
	}
	if (enc_id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return 0;
	}

	pdev_info->enc_to_sen_id_map_en[enc_id] = enc_to_sen_id_map_enable;
	pdev_info->enc_to_sen_id_table[enc_id] = sen_id;
	pdev_info->enc_to_sie_id_map_en[enc_id] = enc_to_sie_id_map_enable;
	enc_id_table[enc_id] = id;
	pdev_info->enc_to_sie_id_table[enc_id] = sie_id;

	isp_sync_id_enc[enc_id] = frame_cnt;

	switch (evt) {
	case ISP_EVENT_PARAM_RST:
	case ISP_EVENT_ENC_SHARPEN:
		if (evt == ISP_EVENT_PARAM_RST) {
			PRINT_ISP(dbg_mode & ISP_DBG_ENC_CB, "enc  reset,      id = %5X, frm = %d \r\n", id, frame_cnt);
			iq_trig_obj.msg = ISP_TRIG_IQ_ENC_IMM;
			iq_trig_obj.enc.reset = TRUE;
		} else {
			PRINT_ISP(dbg_mode & ISP_DBG_ENC_CB, "enc     cb,      id = %5X, frm = %d \r\n", id, frame_cnt);
			iq_trig_obj.msg = ISP_TRIG_IQ_ENC;
			iq_trig_obj.enc.reset = FALSE;
		}
		if (pdev_info->isp_iq != NULL) {
			pdev_info->isp_iq->fn_trig(enc_id, &iq_trig_obj);
		}
		break;

	case ISP_EVENT_ENC_RATIO:
		rt |= kflow_videoenc_get(id, KFLOW_VIDEOENC_ISP_ITEM_RATIO, &enc_isp_ratio);
		PRINT_ISP(dbg_mode & ISP_DBG_ENC_CB, "enc  cb,      id = %5X, isp_ratio(%d, (%d, %d, %d)) \r\n", id, enc_isp_ratio.ratio_base, enc_isp_ratio.edge_ratio, enc_isp_ratio.dn_2d_ratio, enc_isp_ratio.dn_3d_ratio);
		if ((rt != E_OK) || (enc_isp_ratio.ratio_base == 0)) {
			isp_enc_ratio[enc_id].enc_edge_ratio = 256;
			isp_enc_ratio[enc_id].enc_2dnr_ratio = 256;
			isp_enc_ratio[enc_id].enc_3dnr_ratio = 256;
		} else {
			isp_enc_ratio[enc_id].enc_edge_ratio = (enc_isp_ratio.edge_ratio << 8) / enc_isp_ratio.ratio_base;
			isp_enc_ratio[enc_id].enc_2dnr_ratio = (enc_isp_ratio.dn_2d_ratio << 8) / enc_isp_ratio.ratio_base;
			isp_enc_ratio[enc_id].enc_3dnr_ratio = (enc_isp_ratio.dn_3d_ratio << 8) / enc_isp_ratio.ratio_base;
		}
		break;

	default:
		PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "ENC CB out of ISP_EVENT (%d) \r\n", evt);
		break;
	}
	return 0;
}

static INT32 isp_api_cb_vie(ISP_ID id, ISP_EVENT evt, UINT32 frame_cnt, void *param)
{
	UINT32 i;
	UINT32 dbg_mode = isp_dbg_get_dbg_mode(id);

	if ((id < SIE_ID_MAX_NUM) || (id >= ISP_ID_MAX_NUM)) {
		PRINT_ISP_WRN(dbg_mode & ISP_DBG_WRN_MSG, "vie id out of range (%d) \r\n", id);
		return 0;
	}

	if (evt & ISP_EVENT_PARAM_RST) {
		PRINT_ISP(dbg_mode & ISP_DBG_VIE_CB, "vie rst,   id = %8X, frm = %d \r\n", id, frame_cnt);

		for (i = SIE_ID_MAX_NUM; i < ISP_ID_MAX_NUM; i++) {
			isp_src_id_mask[i] = 0;
		}
	}

	if (evt & ISP_EVENT_VCAP_VD) {
		PRINT_ISP(dbg_mode & ISP_DBG_VIE_CB, "vie vd,    id = %8X, frm = %d \r\n", id, frame_cnt);

		isp_src_id_mask[id] |= 1 << id;
	}

	return 0;
}

BOOL isp_get_id_valid(UINT32 id)
{
	return isp_id_valid[id];
}

UINT32 isp_get_sie_id(UINT32 id)
{
	return sie_id_table[id];
}

ER isp_api_get_expt(ISP_ID id, ISP_SENSOR_CTRL *sensor_ctrl)
{
	UINT32 sen_id;
	ER rt = E_OK;

	if (emu_enable) {
		return E_NOSPT;
	}

	if (sie_to_sen_id_map_en[id]) {
		sen_id = sie_to_sen_id_table[id];
	} else {
		sen_id = id;
	}

	if ((ctl_sen_status(sen_id) & CTL_SEN_STATUS_OPEN) == FALSE) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_WRN_MSG, "CTL_SEN_STATUS_OPEN FALSE, id: %d \r\n", id);
		return E_NOEXS;
	}

	rt = ctl_sen_get(sen_id, CTL_SEN_CFGID_GET_EXPT, sensor_ctrl);

	return rt;
}

ER isp_api_get_gain(ISP_ID id, ISP_SENSOR_CTRL *sensor_ctrl)
{
	UINT32 sen_id;
	ER rt = E_OK;

	if (emu_enable) {
		return E_NOSPT;
	}

	if (sie_to_sen_id_map_en[id]) {
		sen_id = sie_to_sen_id_table[id];
	} else {
		sen_id = id;
	}

	if ((ctl_sen_status(sen_id) & CTL_SEN_STATUS_PWR_ON) == FALSE) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_WRN_MSG, "CTL_SEN_STATUS_OPEN FALSE, id: %d \r\n", id);
		return E_NOEXS;
	}

	rt = ctl_sen_get(sen_id, CTL_SEN_CFGID_GET_GAIN, sensor_ctrl);

	return rt;
}

ER isp_api_get_direction(ISP_ID id, ISP_SENSOR_DIRECTION *sensor_direction)
{
	CTL_SEN_FLIP sen_flip = CTL_SEN_FLIP_NONE;
	UINT32 sen_id;
	ER rt = E_OK;

	if (emu_enable) {
		return E_NOSPT;
	}

	if (sie_to_sen_id_map_en[id]) {
		sen_id = sie_to_sen_id_table[id];
	} else {
		sen_id = id;
	}

	if ((ctl_sen_status(sen_id) & CTL_SEN_STATUS_PWR_ON) == FALSE) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_WRN_MSG, "CTL_SEN_STATUS_OPEN FALSE, id: %d \r\n", id);
		return E_NOEXS;
	}

	rt = ctl_sen_get(sen_id, CTL_SEN_CFGID_FLIP_TYPE, &sen_flip);
	if (rt != 0) {
		return rt;
	}

	if (sen_flip & CTL_SEN_FLIP_H) {
		sensor_direction->mirror = TRUE;
	}
	if (sen_flip & CTL_SEN_FLIP_V) {
		sensor_direction->flip = TRUE;
	}

	return rt;
}

ER isp_api_get_sync_time(ISP_ID id, UINT32 *sync_time)
{
	CTL_SEN_GET_ATTR_PARAM attr_param;
	UINT32 sen_id;
	ER rt = E_OK;

	if (emu_enable) {
		return E_NOSPT;
	}

	if (sie_to_sen_id_map_en[id]) {
		sen_id = sie_to_sen_id_table[id];
	} else {
		sen_id = id;
	}

	if ((ctl_sen_status(sen_id) & CTL_SEN_STATUS_PWR_ON) == FALSE) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_WRN_MSG, "CTL_SEN_STATUS_OPEN FALSE, id: %d \r\n", id);
		return E_NOEXS;
	}

	rt = ctl_sen_get(sen_id, CTL_SEN_CFGID_GET_ATTR, &attr_param);
	if (rt != 0) {
		return rt;
	}

	*sync_time = attr_param.sync_timing;

	return rt;
}

ER isp_api_get_frame_num(ISP_ID id, BOOL is_from_sie, UINT32 *frame_num)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	CTL_SEN_GET_MODE_BASIC_PARAM mode_param;
	UINT32 sen_id;
	ER rt = E_OK;

	if (pdev_info == NULL) {
		return E_SYS;
	}

	if (is_from_sie && sie_to_sen_id_map_en[id]) {
		sen_id = sie_to_sen_id_table[id];
	} else if (!is_from_sie && pdev_info->ipp_to_sen_id_map_en[id]) {
		sen_id = pdev_info->ipp_to_sen_id_table[id];
	} else {
		sen_id = id;
	}

	if (sensor_mode_manual[sen_id].enable) {
		*frame_num = sensor_mode_manual[sen_id].frame_num;
		return E_OK;
	}

	if (emu_enable) {
		return E_NOSPT;
	}

	if ((ctl_sen_status(sen_id) & CTL_SEN_STATUS_PWR_ON) == FALSE) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_WRN_MSG, "CTL_SEN_STATUS_OPEN FALSE, id: %d \r\n", id);
		return E_NOEXS;
	}

	mode_param.mode = CTL_SEN_MODE_CUR;
	rt = ctl_sen_get(sen_id, CTL_SEN_CFGID_GET_MODE_BASIC, &mode_param);
	if (rt != 0) {
		return rt;
	}

	*frame_num = mode_param.frame_num;

	return rt;
}

ER isp_api_get_chg_fps(ISP_ID id, UINT32 *chg_fps)
{
	CTL_SEN_GET_FPS_PARAM fps_param;
	UINT32 sen_id;
	ER rt = E_OK;

	if (emu_enable) {
		return E_NOSPT;
	}

	if (sie_to_sen_id_map_en[id]) {
		sen_id = sie_to_sen_id_table[id];
	} else {
		sen_id = id;
	}

	if ((ctl_sen_status(sen_id) & CTL_SEN_STATUS_PWR_ON) == FALSE) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_WRN_MSG, "CTL_SEN_STATUS_OPEN FALSE, id: %d \r\n", id);
		return E_NOEXS;
	}

	rt = ctl_sen_get(sen_id, CTL_SEN_CFGID_GET_FPS, &fps_param);
	if (rt != 0) {
		return rt;
	}

	*chg_fps = fps_param.chg_fps;
	return rt;
}

static ER isp_api_get_sensor_data_fmt(ISP_ID id, BOOL is_from_sie, UINT32 *data_fmt)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	CTL_SEN_GET_MODE_BASIC_PARAM mode_basic_param;
	UINT32 sen_id;
	ER rt = E_OK;

	if (pdev_info == NULL) {
		return E_SYS;
	}

	if (is_from_sie && sie_to_sen_id_map_en[id]) {
		sen_id = sie_to_sen_id_table[id];
	} else if (!is_from_sie && pdev_info->ipp_to_sen_id_map_en[id]) {
		sen_id = pdev_info->ipp_to_sen_id_table[id];
	} else {
		sen_id = id;
	}

	if (sensor_mode_manual[sen_id].enable) {
		*data_fmt = sensor_mode_manual[sen_id].data_fmt;
		return E_OK;
	}

	if (emu_enable) {
		return E_NOSPT;
	}

	if ((ctl_sen_status(sen_id) & CTL_SEN_STATUS_PWR_ON) == FALSE) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_WRN_MSG, "CTL_SEN_STATUS_OPEN FALSE, id: %d \r\n", id);
		return E_NOEXS;
	}

	mode_basic_param.mode = CTL_SEN_MODE_CUR;
	rt = ctl_sen_get(sen_id, CTL_SEN_CFGID_GET_MODE_BASIC, &mode_basic_param);
	if (rt != 0) {
		return rt;
	}

	if (mode_basic_param.data_fmt == CTL_SEN_DATA_FMT_RGBIR) {
		*data_fmt = ISP_SEN_DATA_FMT_RGBIR;
	} else if (mode_basic_param.data_fmt == CTL_SEN_DATA_FMT_RCCB) {
		*data_fmt = ISP_SEN_DATA_FMT_RCCB;
	}else if (mode_basic_param.data_fmt == CTL_SEN_DATA_FMT_YUV) {
		*data_fmt = ISP_SEN_DATA_FMT_YUV;
	} else if (mode_basic_param.data_fmt == CTL_SEN_DATA_FMT_Y_ONLY) {
		*data_fmt = ISP_SEN_DATA_FMT_Y_ONLY;
	} else if (mode_basic_param.data_fmt == CTL_SEN_DATA_FMT_DVS) {
		*data_fmt = ISP_SEN_DATA_FMT_DVS;
	} else {
		*data_fmt = ISP_SEN_DATA_FMT_RGB;
	}

	return rt;
}

static ER isp_api_get_sensor_mode_type(ISP_ID id, BOOL is_from_sie, UINT32 *mode_type)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	CTL_SEN_GET_MODE_BASIC_PARAM mode_basic_param;
	ISP_FUNC_EN fun_en;
	UINT32 sen_id;
	ER rt = E_OK;

	if (pdev_info == NULL) {
		return E_SYS;
	}

	if (is_from_sie && sie_to_sen_id_map_en[id]) {
		sen_id = sie_to_sen_id_table[id];
	} else if (!is_from_sie && pdev_info->ipp_to_sen_id_map_en[id]) {
		sen_id = pdev_info->ipp_to_sen_id_table[id];
	} else {
		sen_id = id;
	}

	if (sensor_mode_manual[sen_id].enable) {
		*mode_type = sensor_mode_manual[sen_id].mode_type;
		return E_OK;
	}

	if (emu_enable) {
		if (!ctl_sie_get_is_open(id)) {
			PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "sie is not open (%d) \r\n", id);
			return E_SYS;
		}
		rt = ctl_sie_isp_get(id, CTL_SIE_ISP_ITEM_FUNC_EN, &fun_en);
		if (rt == E_OK) {
			if (fun_en & ISP_FUNC_EN_SHDR) {
				*mode_type = ISP_SEN_MODE_STAGGER_HDR;
			} else {
				*mode_type = ISP_SEN_MODE_LINEAR;
			}
		} else {
			*mode_type = ISP_SEN_MODE_LINEAR;
		}

		return E_NOSPT;
	}

	if ((ctl_sen_status(sen_id) & CTL_SEN_STATUS_PWR_ON) == FALSE) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_WRN_MSG, "CTL_SEN_STATUS_OPEN FALSE, id: %d \r\n", id);

		return E_NOEXS;
	}

	mode_basic_param.mode = CTL_SEN_MODE_CUR;
	rt = ctl_sen_get(sen_id, CTL_SEN_CFGID_GET_MODE_BASIC, &mode_basic_param);
	if (rt != 0) {
		return rt;
	}

	switch ((UINT64)mode_basic_param.mode_type) {
		default:
		case CTL_SEN_MODE_LINEAR:
			*mode_type = ISP_SEN_MODE_LINEAR;
		break;

		case CTL_SEN_MODE_BUILTIN_HDR:
			*mode_type = ISP_SEN_MODE_BUILTIN_HDR;
		break;

		case CTL_SEN_MODE_CCIR:
			*mode_type = ISP_SEN_MODE_CCIR;
		break;

		case CTL_SEN_MODE_CCIR_INTERLACE:
			*mode_type = ISP_SEN_MODE_CCIR_INTERLACE;
		break;

		case CTL_SEN_MODE_RAW_PDAF:
			*mode_type = ISP_SEN_MODE_RAW_PDAF;
		break;

		case CTL_SEN_MODE_BUILTIN_DCG_HDR:
			*mode_type = ISP_SEN_MODE_BUILTIN_DCG_HDR;
		break;

		case CTL_SEN_MODE_STAGGER_HDR:
			*mode_type = ISP_SEN_MODE_STAGGER_HDR;
		break;

		case CTL_SEN_MODE_PDAF:
			*mode_type = ISP_SEN_MODE_PDAF;
		break;

		case CTL_SEN_MODE_BUILTIN_DCG_SHDR:
			*mode_type = ISP_SEN_MODE_BUILTIN_DCG_SHDR;
		break;

		case CTL_SEN_MODE_STAGGER_PDAF:
			*mode_type = ISP_SEN_MODE_STAGGER_PDAF;
		break;

		case CTL_SEN_MODE_DCG_HDR:
			*mode_type = ISP_SEN_MODE_DCG_HDR;
		break;

		case CTL_SEN_MODE_DCG_SHDR:
			*mode_type = ISP_SEN_MODE_DCG_SHDR;
		break;

		case CTL_SEN_MODE_STAGGER3_HDR:
			*mode_type = ISP_SEN_MODE_STAGGER3_HDR;
		break;

		case CTL_SEN_MODE_THERMAL:
			*mode_type = ISP_SEN_MODE_THERMAL;
		break;
	}

	return rt;
}

ER isp_api_get_sensor_mode_info(ISP_ID id, ISP_SENSOR_MODE_INFO *mode_param)
{
	CTL_SEN_GET_MODE_BASIC_PARAM mode_basic_param;
	ISP_FUNC_EN fun_en;
	UINT32 min_expt = 0;
	UINT32 sen_id;
	UINT32 i;
	ER rt = E_OK;

	if (sie_to_sen_id_map_en[id]) {
		sen_id = sie_to_sen_id_table[id];
	} else {
		sen_id = id;
	}

	if (emu_enable) {
		mode_param->data_fmt = ISP_SEN_DATA_FMT_RGB;

		if (!ctl_sie_get_is_open(id)) {
			PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "sie is not open (%d) \r\n", id);
			return E_SYS;
		}
		rt = ctl_sie_isp_get(id, CTL_SIE_ISP_ITEM_FUNC_EN, &fun_en);
		if (rt == E_OK) {
			if (fun_en & ISP_FUNC_EN_SHDR) {
				mode_param->mode_type = ISP_SEN_MODE_STAGGER_HDR;
			} else {
				mode_param->mode_type = ISP_SEN_MODE_LINEAR;
			}
		} else {
			mode_param->mode_type = ISP_SEN_MODE_LINEAR;
		}

		return E_NOSPT;
	}

	if ((ctl_sen_status(sen_id) & CTL_SEN_STATUS_PWR_ON) == FALSE) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_WRN_MSG, "CTL_SEN_STATUS_OPEN FALSE, id: %d \r\n", id);

		return E_NOEXS;
	}

	mode_basic_param.mode = CTL_SEN_MODE_CUR;
	rt = ctl_sen_get(sen_id, CTL_SEN_CFGID_GET_MODE_BASIC, &mode_basic_param);
	if (rt != 0) {
		return rt;
	}

	if (mode_basic_param.data_fmt == CTL_SEN_DATA_FMT_RGBIR) {
		mode_param->data_fmt = ISP_SEN_DATA_FMT_RGBIR;
	} else if (mode_basic_param.data_fmt == CTL_SEN_DATA_FMT_DVS) {
		mode_param->data_fmt = ISP_SEN_DATA_FMT_DVS;
	} else {
		mode_param->data_fmt = ISP_SEN_DATA_FMT_RGB;
	}

	if (mode_basic_param.mode_type == CTL_SEN_MODE_BUILTIN_HDR) {
		mode_param->mode_type = ISP_SEN_MODE_BUILTIN_HDR;
	} else if (mode_basic_param.mode_type == CTL_SEN_MODE_STAGGER_HDR) {
		mode_param->mode_type = ISP_SEN_MODE_STAGGER_HDR;
	} else if (mode_basic_param.mode_type == (UINT64)CTL_SEN_MODE_STAGGER3_HDR) {
		mode_param->mode_type = ISP_SEN_MODE_STAGGER3_HDR;
	} else if (mode_basic_param.mode_type == CTL_SEN_MODE_CCIR){
		mode_param->mode_type = ISP_SEN_MODE_CCIR;
	} else if (mode_basic_param.mode_type == CTL_SEN_MODE_CCIR_INTERLACE){
		mode_param->mode_type = ISP_SEN_MODE_CCIR_INTERLACE;
	} else {
		mode_param->mode_type = ISP_SEN_MODE_LINEAR;
	}

	mode_param->max_gain = mode_basic_param.gain.max;
	mode_param->min_gain = mode_basic_param.gain.min;
	rt |= ctl_sen_get(sen_id, CTL_SEN_CFGID_USER_DEFINE2, &min_expt);
	mode_param->min_expt = min_expt;

	for (i = 0; i < ISP_SEN_MFRAME_MAX_NUM; i++) {
		memcpy(&mode_param->act_size[i], &mode_basic_param.act_size[0], sizeof(URECT));
	}
	memcpy(&mode_param->crp_size, &mode_basic_param.crp_size, sizeof(USIZE));
	mode_param->row_time = mode_basic_param.row_time;
	mode_param->row_time_step = mode_basic_param.row_time_step;
	mode_param->line_length = mode_basic_param.signal_info.hd_period;
	mode_param->frame_length = mode_basic_param.signal_info.vd_period;
	mode_param->dft_fps = mode_basic_param.dft_fps;

	return rt;
}

ER isp_api_get_func(ISP_ID id, ISP_FUNC_INFO *isp_func)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	ISP_FUNC_EN sie_func_en = 0, ipp_func_en = 0;
	CTL_SIE_ISP_STATUS sie_status = CTL_SIE_ISP_STS_CLOSE;
	CTL_IPP_ISP_STATUS_INFO ipp_status_info = {0};
	CTL_IPP_ISP_IOSIZE ipp_isp_iosize = {0};
	ER rt = E_OK;

	if (pdev_info == NULL) {
		return E_SYS;
	}
	if (id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return E_SYS;
	}

	if (id < SIE_ID_MAX_NUM) {
		if (ctl_sie_get_is_open(id)) {
			rt |= ctl_sie_isp_get(sie_id_table[id], CTL_SIE_ISP_ITEM_FUNC_EN, &sie_func_en);
			rt |= ctl_sie_isp_get(sie_id_table[id], CTL_SIE_ISP_ITEM_STATUS, &sie_status);
		} else {
			sie_func_en = 0;
			sie_status = CTL_SIE_ISP_STS_CLOSE;
		}
	} else {
		sie_func_en = 0;
		sie_status = CTL_SIE_ISP_STS_CLOSE;
	}

	rt |= ctl_ipp_isp_get(ipp_id_table[id], CTL_IPP_ISP_ITEM_FUNC_EN, &ipp_func_en);
	rt |= ctl_ipp_isp_get(ipp_id_table[id], CTL_IPP_ISP_ITEM_STATUS_INFO, &ipp_status_info);
	rt |= ctl_ipp_isp_get(ipp_id_table[id], CTL_IPP_ISP_ITEM_IOSIZE, &ipp_isp_iosize);

	isp_func->sie_valid = (sie_status == CTL_SIE_ISP_STS_RUN)  ? TRUE : FALSE;
	isp_func->ipp_valid = ((ipp_status_info.sts == CTL_IPP_ISP_STS_READY) || (ipp_status_info.sts == CTL_IPP_ISP_STS_RUN)) ? TRUE : FALSE;
	isp_func->ae_valid = FALSE;
	isp_func->af_valid = FALSE;
	isp_func->awb_valid = FALSE;
	if (pdev_info->isp_ae != NULL) {
		isp_func->ae_valid = ((sie_func_en & ISP_FUNC_EN_AE) && !(strcmp(pdev_info->isp_ae->name, "NVT_AE"))) ? TRUE : FALSE;
	}
	if (pdev_info->isp_af != NULL) {
		isp_func->af_valid = ((ipp_func_en & ISP_FUNC_EN_AF) && !(strcmp(pdev_info->isp_af->name, "NVT_AF"))) ? TRUE : FALSE;
	}
	if (pdev_info->isp_awb != NULL) {
		isp_func->awb_valid = ((sie_func_en & ISP_FUNC_EN_AWB) && !(strcmp(pdev_info->isp_awb->name, "NVT_AWB"))) ? TRUE : FALSE;
	}
	isp_func->defog_valid = (ipp_func_en & ISP_FUNC_EN_DEFOG) ? TRUE : FALSE;
	isp_func->shdr_valid = ((sie_func_en & ISP_FUNC_EN_SHDR) && (ipp_func_en & ISP_FUNC_EN_SHDR)) ? TRUE : FALSE;
	isp_func->wdr_valid = (ipp_func_en & ISP_FUNC_EN_WDR) ? TRUE : FALSE;
	memcpy(&isp_func->yuv_out_ch, &ipp_isp_iosize.out_ch, sizeof(ipp_isp_iosize.out_ch));

	return rt;
}

ER isp_api_get_sensor_reg(ISP_ID id, UINT32 *addr, UINT32 *data)
{
	CTL_SEN_CMD cmd = {0};
	UINT32 sen_id;
	ER rt = E_OK;

	if (sie_to_sen_id_map_en[id]) {
		sen_id = sie_to_sen_id_table[id];
	} else {
		sen_id = id;
	}

	if ((ctl_sen_status(sen_id) & CTL_SEN_STATUS_PWR_ON) == FALSE) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_WRN_MSG, "CTL_SEN_STATUS_OPEN FALSE, id: %d \r\n", id);
		return E_NOEXS;
	}

	cmd.addr = *addr;
	rt = ctl_sen_read_reg(sen_id, &cmd);
	if (rt != 0) {
		return rt;
	}

	*data = ((cmd.data[1] << 8) & 0xFF00) | (cmd.data[0] & 0xFF);
	PRINT_ISP(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_SENSOR_REG, "get sensor reg., id (%d), addr (0x%X), data (0x%X) \r\n", (int)sen_id, *(int *)addr, *(int *)data);

	return rt;
}

ER isp_api_get_sensor_info(ISP_SENSOR_INFO *sensor_info)
{
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	CTL_SEN_GET_ATTR_PARAM attr_param = {0};
	CTL_IPP_ISP_STATUS_INFO ipp_status_info = {0};
	UINT32 id = 0, sen_id = 0, frm_grp = 0;
	ER rt = E_OK;

	if (pdev_info == NULL) {
		return E_SYS;
	}

	for (id = 0; id < ISP_ID_MAX_NUM; id++) {
		sensor_info->ipp_id_table[id] = ipp_id_table[id];

		if (pdev_info->ipp_to_sen_id_map_en[id]) {
			sen_id = pdev_info->ipp_to_sen_id_table[id];
		} else {
			sen_id = id;
		}

		if ((ctl_sen_status(sen_id) & CTL_SEN_STATUS_PWR_ON) == FALSE) {
			memset(&ipp_status_info, 0x0, sizeof(CTL_IPP_ISP_STATUS_INFO));
			rt = ctl_ipp_isp_get(ipp_id_table[id], CTL_IPP_ISP_ITEM_STATUS_INFO, &ipp_status_info);
			if ((rt == E_OK) && ((ipp_status_info.sts == CTL_IPP_ISP_STS_READY) || (ipp_status_info.sts == CTL_IPP_ISP_STS_RUN))) {
				sprintf(sensor_info->name[id], "    PATGEN");
				if (!ctl_sie_get_is_open(id)) {
					PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "sie is not open (%d) \r\n", id);
					return E_SYS;
				}
				rt = ctl_sie_isp_get(sie_id_table[id], CTL_SIE_ISP_ITEM_MULTI_FRM_GRP, &frm_grp);
				if (rt == E_OK) {
					sensor_info->src_id_mask[id] = frm_grp;
				} else {
					PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "get CTL_SIE_ISP_ITEM_MULTI_FRM_GRP fail (%d) \r\n", rt);
				}
			} else {
				sprintf(sensor_info->name[id], "NULL");
				sensor_info->src_id_mask[id] = 0x0;
			}
		} else {
			if (isp_src_id_mask[id] == 0) {
				sprintf(sensor_info->name[id], "NULL");
			} else {
				rt = ctl_sen_get(sen_id, CTL_SEN_CFGID_GET_ATTR, &attr_param);
				strcpy(sensor_info->name[id], attr_param.name);
			}

			sensor_info->src_id_mask[id] = isp_src_id_mask[id];
		}
	}
	return rt;
}

ER isp_api_get_yuv(ISP_ID id, ISP_YUV_INFO *yuv_info)
{
	UINT32 i;
	ER rt = E_OK;

	if (id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return 0;
	}
	if (yout_lock[id] == FALSE) {
		yout_lock[id] = TRUE;

		ctl_ipp_isp_yuv_out.pid = yuv_info->pid;
		rt = ctl_ipp_isp_get(ipp_id_table[id], CTL_IPP_ISP_ITEM_YUV_OUT, &ctl_ipp_isp_yuv_out);

		if (rt == E_OK) {
			yuv_info->info_vaild = TRUE;
			yuv_info->pxlfmt = (VDO_PXLFMT)ctl_ipp_isp_yuv_out.vdo_frm.pxlfmt;
			yuv_info->size.w = ctl_ipp_isp_yuv_out.vdo_frm.size.w;
			yuv_info->size.h = ctl_ipp_isp_yuv_out.vdo_frm.size.h;
			for (i = 0; i < VDO_MAX_PLANE; i++) {
				yuv_info->pw[i] = ctl_ipp_isp_yuv_out.vdo_frm.pw[i];
				yuv_info->ph[i] = ctl_ipp_isp_yuv_out.vdo_frm.ph[i];
				yuv_info->loff[i] = ctl_ipp_isp_yuv_out.vdo_frm.loff[i];
				yuv_info->addr[i] = ctl_ipp_isp_yuv_out.vdo_frm.addr[i];
				#if defined(__KERNEL__)
				if (ctl_ipp_isp_yuv_out.vdo_frm.addr[i] != 0) {
					yuv_info->psy_addr[i] = vos_cpu_get_phy_addr(ctl_ipp_isp_yuv_out.vdo_frm.addr[i]);
				} else {
					 yuv_info->psy_addr[i] = 0x0;
				}
				#else
				yuv_info->psy_addr[i] = ctl_ipp_isp_yuv_out.vdo_frm.addr[i];
				#endif
			}
		} else {
			PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "get yuv timeout (%d, %d) (%d) \r\n", id, yuv_info->pid, rt);
			yuv_info->info_vaild = FALSE;
			rt = E_SYS;
		}
	} else {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "id = %d, yout_lock: TRUE \r\n", id);
		yuv_info->info_vaild = FALSE;
		rt = E_SYS;
	}

	return rt;
}

ER isp_api_get_raw(ISP_ID id, ISP_RAW_INFO *raw_info, UINT32 vdo_plane)
{
	ER rt = E_OK;

	if (id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return E_SYS;
	}

	if (vdo_plane >= VDO_MAX_PLANE) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "vdo_plane out of range (%d) \r\n", vdo_plane);
		return E_SYS;
	}

	if (raw_lock[id] == FALSE) {
		if (!ctl_sie_get_is_open(id)) {
			PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "sie is not open (%d) \r\n", id);
			return E_SYS;
		}
		raw_lock[id] = TRUE;
		rt = ctl_sie_isp_get(sie_id_table[id], CTL_SIE_ISP_ITEM_IMG_OUT, &ctl_isp_sie_header_info);
		if (rt == E_OK) {
			raw_info->info_vaild = TRUE;
			raw_info->pxlfmt = (VDO_PXLFMT)ctl_isp_sie_header_info.vdo_frm.pxlfmt;
			raw_info->size.w = ctl_isp_sie_header_info.vdo_frm.size.w;
			raw_info->size.h = ctl_isp_sie_header_info.vdo_frm.size.h;
			raw_info->pw = ctl_isp_sie_header_info.vdo_frm.pw[vdo_plane];
			raw_info->ph = ctl_isp_sie_header_info.vdo_frm.ph[vdo_plane];
			raw_info->loff = ctl_isp_sie_header_info.vdo_frm.loff[vdo_plane];
			raw_info->addr = ctl_isp_sie_header_info.vdo_frm.addr[vdo_plane];
			#if defined(__KERNEL__)
			raw_info->psy_addr = vos_cpu_get_phy_addr(ctl_isp_sie_header_info.vdo_frm.addr[vdo_plane]);
			#else
			raw_info->psy_addr = ctl_isp_sie_header_info.vdo_frm.addr[vdo_plane];
			#endif
		} else {
			PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "get raw fail (%d) (%d) \r\n", id, rt);
			raw_info->info_vaild = FALSE;
			rt = E_SYS;
		}

	} else {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "id = %d, raw_lock: TRUE \r\n", id);
		raw_info->info_vaild = FALSE;
		rt = E_SYS;
	}

	return rt;
}

ER isp_api_get_bnr_sta(ISP_ID id, ISP_BNR_STA_INFO *bnr_sta_info)
{
	if (id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return 0;
	}

	PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "get bnr_sta fail (%d)  \r\n", id);
	bnr_sta_info->info_vaild = FALSE;

	return E_SYS;
}

ER isp_api_get_3dnr_sta(ISP_ID id, ISP_3DNR_STA_INFO *_3dnr_sta_info)
{
	ER rt = E_OK;

	if (id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return 0;
	}
	rt = ctl_ipp_isp_get(ipp_id_table[id], CTL_IPP_ISP_ITEM_3DNR_STA, &ctl_ipp_isp_3dnr_sta);

	if (rt == E_OK) {
		_3dnr_sta_info->info_vaild = TRUE;
		_3dnr_sta_info->enable = ctl_ipp_isp_3dnr_sta.enable;
		_3dnr_sta_info->buf_addr = ctl_ipp_isp_3dnr_sta.buf_addr;
		_3dnr_sta_info->max_sample_num = ctl_ipp_isp_3dnr_sta.max_sample_num;
		_3dnr_sta_info->lofs = ctl_ipp_isp_3dnr_sta.lofs;
		_3dnr_sta_info->sample_step.w = ctl_ipp_isp_3dnr_sta.sample_step.w;
		_3dnr_sta_info->sample_step.h = ctl_ipp_isp_3dnr_sta.sample_step.h;
		_3dnr_sta_info->sample_num.w = ctl_ipp_isp_3dnr_sta.sample_num.w;
		_3dnr_sta_info->sample_num.h = ctl_ipp_isp_3dnr_sta.sample_num.h;
		_3dnr_sta_info->sample_st.x = ctl_ipp_isp_3dnr_sta.sample_st.x;
		_3dnr_sta_info->sample_st.y = ctl_ipp_isp_3dnr_sta.sample_st.y;
	} else {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "get _3dnr_sta fail (%d)  \r\n", id);
		_3dnr_sta_info->info_vaild = FALSE;
		rt = E_SYS;
	}

	return rt;
}

ER isp_api_get_md_sta(ISP_ID id, ISP_MD_STA_INFO *md_sta)
{
	CTL_IPP_ISP_MD_SUBOUT ipp_isp_md_subout = {0};
	UINT32 w_div = 0, h_div = 0, read_value, max_cnt;
	UINT32 i, j, i_start, i_range, j_start, j_end, j_range, line_cnt = 0, bit_cnt = 0, bit_cnt_start;
	ULONG base_addr;
	ER rt = E_OK;

	rt = ctl_ipp_isp_get(ipp_id_table[id], CTL_IPP_ISP_ITEM_MD_SUBOUT, &ipp_isp_md_subout);
	if (rt != E_OK) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "get md_sta fail (%d)  \r\n", id);
		md_sta->vaild = FALSE;
		return rt;
	}

	if ((ipp_isp_md_subout.md_size.w < ISP_VA_W_WINNUM) || (ipp_isp_md_subout.md_size.h < ISP_VA_H_WINNUM)) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "md_size too small %d x %d (%d)  \r\n", ipp_isp_md_subout.md_size.w, ipp_isp_md_subout.md_size.h, id);
		md_sta->vaild = FALSE;
		return rt;
	}

	memset(md_sta, 0, sizeof(ISP_MD_STA_INFO));
	md_sta->vaild = TRUE;

	w_div = ipp_isp_md_subout.md_size.w / ISP_VA_W_WINNUM;
	h_div = ipp_isp_md_subout.md_size.h / ISP_VA_H_WINNUM;
	j_range = h_div * ISP_VA_H_WINNUM;
	j_start = (ipp_isp_md_subout.md_size.h - j_range) >> 1;
	j_end = j_start + j_range;
	i_range = w_div * ISP_VA_W_WINNUM;
	i_start = (ipp_isp_md_subout.md_size.w - i_range) >> 1;
	max_cnt = w_div * h_div;
	base_addr = ipp_isp_md_subout.addr;
	//printk("size %d %d, %d %d, lofs %d, addr 0x%X, div %d %d, i: %d %d, j: %d %d \r\n ", ipp_isp_md_subout.src_img_size.w, ipp_isp_md_subout.src_img_size.h, ipp_isp_md_subout.md_size.w, ipp_isp_md_subout.md_size.h, ipp_isp_md_subout.md_lofs, ipp_isp_md_subout.addr, w_div, h_div, i_start, i_range, j_start, j_end);

	for (j = j_start; j < j_end; j++) {
		for (i = 0; i < ipp_isp_md_subout.md_lofs; i+=4) {
			#if defined(__FREERTOS)
			read_value = *(UINT32 volatile *)(base_addr + (j * ipp_isp_md_subout.md_lofs + i));
			#else
			read_value = ioread32((void *)(base_addr + (j * ipp_isp_md_subout.md_lofs + i)));
			#endif

			if (i == 0) {
				bit_cnt_start = i_start;
			} else {
				bit_cnt_start = 0;
			}

			for (bit_cnt = bit_cnt_start; bit_cnt < 32; bit_cnt++) {
				if (line_cnt < i_range) {
					md_sta->data[((j - j_start) / h_div * ISP_VA_W_WINNUM) + (line_cnt / w_div)] += (read_value >> bit_cnt) & 0x1;
					line_cnt++;
				}
			}

			//if (j == j_start) {
			//	printk("i: %2d, j: %2d, line_cnt: %3d, idx: %2d, addr: %8X, data: %8X \r\n", i, j, line_cnt, ((j - j_start) / h_div * ISP_VA_W_WINNUM) + ((line_cnt - 1) / w_div), base_addr + (j * ipp_isp_md_subout.md_lofs + i), read_value);
			//}
		}

		//printk("i: %2d, j: %2d, line_cnt: %3d, idx: %2d, addr: %8X, data: %8X \r\n", i, j, line_cnt, ((j - j_start) / h_div * ISP_VA_W_WINNUM) + ((line_cnt - 1) / w_div), base_addr + (j * ipp_isp_md_subout.md_lofs + i - 4), read_value);
		line_cnt = 0;
	}

	for (i = 0; i < ISP_VA_MAX_WINNUM; i++) {
		md_sta->data[i] = md_sta->data[i] * 100 / max_cnt;
	}

	return E_OK;
}

ER isp_api_get_ir_info(ISP_ID id, ISP_RGBIR_INFO *ir_info)
{
	CTL_SIE_ISP_RGBIR_INFO ctl_ir_info;
	ER rt = E_OK;

	if (id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", id);
		return E_NOSPT;
	}

	if (!ctl_sie_get_is_open(id)) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "sie is not open (%d) \r\n", id);
		return E_SYS;
	}

	rt |= ctl_sie_isp_get(sie_id_table[id], CTL_SIE_ISP_ITEM_IR_INFO, &ctl_ir_info);
	if (rt == E_OK) {
		ir_info->ir_level = ctl_ir_info.ir_level;
		ir_info->saturation = ctl_ir_info.ir_sat;
	}
	return rt;
}

ER isp_api_get_enc_isp_ratio(ISP_ID id, ISP_ENC_ISP_RATIO *enc_isp_ratio)
{
	ER rt = E_OK;

	if (id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return 0;
	}

	memcpy(enc_isp_ratio, &(isp_enc_ratio[id]), sizeof(ISP_ENC_ISP_RATIO));
	return rt;
}

ISP_SENSOR_INIT_INFO *isp_api_get_fastboot_sensor_info(void)
{
	return &sensor_init_info;
}

BOOL isp_api_get_fastboot_valid(void)
{
	#if defined(__KERNEL__)
	return kdrv_builtin_is_fastboot();
	#else
	return FALSE;
	#endif
}

BOOL isp_api_get_fastboot_sensor_valid(UINT32 id)
{
	#if defined(__KERNEL__)
	return isp_builtin_get_sensor_valid(id);
	#else
	return FALSE;
	#endif
}

#if defined(__KERNEL__)
static void isp_api_get_fastboot_sync_info(UINT32 id)
{
	ISP_AE_INIT_INFO ae_init_info;
	ISP_AWB_INIT_INFO awb_init_info;
	ISP_IQ_INIT_INFO iq_init_info;
	ISP_BUILTIN_SHDR_EV_RATIO *builtin_shdr_ev_ratio;
	ISP_BUILTIN_CGAIN *builtin_cgain;
	ISP_BUILTIN_SENSOR_CTRL *p_sensor_ctrl;
	INT32 i;
	UINT32 id_mask;

	if (isp_builtin_get_sensor_valid(id)) {
		ae_init_info.lv = isp_builtin_get_lv(id);
		ae_init_info.total_gain = isp_builtin_get_total_gain(id);
		ae_init_info.d_gain = isp_builtin_get_dgain(id);
		ae_init_info.expt_max = isp_builtin_get_sensor_expt_max(id);
		ae_init_info.overexposure_offset = isp_builtin_get_overexposure_offset(id);
		ae_init_info.stitch_mode = isp_builtin_get_ae_stitch_mode(id);
		ae_init_info.shdr_tm_ratio = isp_builtin_get_shdr_tm_ratio(id);
		builtin_shdr_ev_ratio = isp_builtin_get_shdr_ev_ratio(id);
		ae_init_info.compensation_ratio = isp_builtin_get_compensation_ratio(id);
		ae_init_info.shdr_ev_ratio[0] = builtin_shdr_ev_ratio->ratio[0];
		ae_init_info.shdr_ev_ratio[1] = builtin_shdr_ev_ratio->ratio[1];
		ae_init_info.shdr_ev_ratio[2] = builtin_shdr_ev_ratio->ratio[2];
		isp_builtin_get_ae_param(id, &ae_init_info.param_addr);
		isp_dev_set_sync_item(id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_AE_INIT, &ae_init_info);
		if (isp_builtin_get_shdr_enable(id)) {
			id_mask = isp_builtin_get_shdr_id_mask(id);
			for (i = ISP_ID_MAX_NUM - 1; i >= 0; i--) {
				if ((0x1 << i) & id_mask) {
					isp_dev_set_sync_item(i, ISP_SYNC_SEL_CURRENT, ISP_SYNC_AE_INIT, &ae_init_info);
					break;
				}
			}
		}
		awb_init_info.ct = isp_builtin_get_ct(id);
		builtin_cgain = isp_builtin_get_cgain(id);
		awb_init_info.r_gain = builtin_cgain->r;
		awb_init_info.g_gain = builtin_cgain->g;
		awb_init_info.b_gain = builtin_cgain->b;
		awb_init_info.stitch_mode = isp_builtin_get_awb_stitch_mode(id);
		isp_builtin_get_awb_param(id, &awb_init_info.param_addr);
		isp_dev_set_sync_item(id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_AWB_INIT, &awb_init_info);
		if (isp_builtin_get_shdr_enable(id)) {
			isp_dev_set_sync_item(i, ISP_SYNC_SEL_CURRENT, ISP_SYNC_AWB_INIT, &awb_init_info);
		}
		iq_init_info.nr_lv = isp_builtin_get_nr_lv(id);
		iq_init_info._3dnr_lv = isp_builtin_get_3dnr_lv(id);
		iq_init_info.sharpness_lv = isp_builtin_get_sharpness_lv(id);
		iq_init_info.saturation_lv = isp_builtin_get_saturation_lv(id);
		iq_init_info.contrast_lv = isp_builtin_get_contrast_lv(id);
		iq_init_info.brightness_lv = isp_builtin_get_brightness_lv(id);
		iq_init_info.night_mode = isp_builtin_get_night_mode(id);
		isp_builtin_get_iq_param(id, &iq_init_info.param_addr);
		isp_dev_set_sync_item(id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_IQ_INIT, &iq_init_info);

		p_sensor_ctrl = isp_builtin_get_sensor_expt(id);
		sensor_init_info.expt = p_sensor_ctrl->exp_time[id];
		p_sensor_ctrl = isp_builtin_get_sensor_gain(id);
		sensor_init_info.gain = p_sensor_ctrl->gain_ratio[id];
	}
}
#endif

BOOL isp_api_get_emu_enable(void)
{
	return emu_enable;
}

ER isp_get_sie_func(ISP_ID id, ISP_FUNC_EN *fun_en)
{
	ER rt = E_OK;

	if (!ctl_sie_get_is_open(id)) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "sie is not open (%d) \r\n", id);
		return E_SYS;
	}
	rt = ctl_sie_isp_get(sie_id_table[id], CTL_SIE_ISP_ITEM_FUNC_EN, fun_en);

	return rt;
}

ER isp_get_ipp_func(ISP_ID id, ISP_FUNC_EN *fun_en)
{
	ER rt = E_OK;

	rt = ctl_ipp_isp_get(ipp_id_table[id], CTL_IPP_ISP_ITEM_FUNC_EN, fun_en);

	return rt;
}

UINT32 isp_get_ipp_table(ISP_ID id)
{
	return ipp_id_table[id];
}

UINT32 isp_get_ipp_indep(ISP_ID id)
{
	return isp_sync_ipp_indep[id];
}

void isp_get_sensor_mode_manual(ISP_ID id, ISP_SENSOR_MODE_MANUAL *manual)
{
	memcpy(manual, &sensor_mode_manual, sizeof(ISP_SENSOR_MODE_MANUAL));
}

UINT32 isp_api_get_low_power_lv(void)
{
	return low_power_lv;
}

ER isp_api_set_expt(ISP_ID id, ISP_SENSOR_CTRL *sensor_ctrl)
{
	UINT32 sen_id;
	ER rt = E_OK;

	if (sie_to_sen_id_map_en[id]) {
		sen_id = sie_to_sen_id_table[id];
	} else {
		sen_id = id;
	}

	if (isp_dbg_get_bypass_eng(sen_id) & ISP_BYPASS_SEN_EXPT) {
		return E_OK;
	}

	if ((ctl_sen_status(sen_id) & CTL_SEN_STATUS_PWR_ON) == FALSE) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_WRN_MSG, "CTL_SEN_STATUS_OPEN FALSE, id: %d \r\n", id);
		return E_NOEXS;
	}

	PRINT_ISP(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_SENSOR_EXPT, "id (%d), Expt (%7d, %7d) \r\n", (int)sen_id, (int)sensor_ctrl->exp_time[0], (int)sensor_ctrl->exp_time[1]);
	rt = ctl_sen_set(sen_id, CTL_SEN_CFGID_SET_EXPT, sensor_ctrl);

	return rt;
}

ER isp_api_set_gain(ISP_ID id, ISP_SENSOR_CTRL *sensor_ctrl)
{
	UINT32 sen_id;
	ER rt = E_OK;

	if (sie_to_sen_id_map_en[id]) {
		sen_id = sie_to_sen_id_table[id];
	} else {
		sen_id = id;
	}

	if (isp_dbg_get_bypass_eng(sen_id) & ISP_BYPASS_SEN_GAIN) {
		return E_OK;
	}

	if ((ctl_sen_status(sen_id) & CTL_SEN_STATUS_PWR_ON) == FALSE) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_WRN_MSG, "CTL_SEN_STATUS_OPEN FALSE, id: %d \r\n", id);
		return E_NOEXS;
	}

	PRINT_ISP(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_SENSOR_GAIN, "id (%d), Gain (%7d, %7d) \r\n", (int)sen_id, (int)sensor_ctrl->gain_ratio[0], (int)sensor_ctrl->gain_ratio[1]);
	rt = ctl_sen_set(sen_id, CTL_SEN_CFGID_SET_GAIN, sensor_ctrl);

	return rt;
}

ER isp_api_set_preset(ISP_ID id, ISP_SENSOR_PRESET_CTRL *preset_ctrl)
{
	UINT32 sen_id;
	ER rt = E_OK;

	if (sie_to_sen_id_map_en[id]) {
		sen_id = sie_to_sen_id_table[id];
	} else {
		sen_id = id;
	}

	if ((ctl_sen_status(sen_id) & CTL_SEN_STATUS_PWR_ON) == FALSE) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_WRN_MSG, "CTL_SEN_STATUS_OPEN FALSE, id: %d \r\n", id);
		return E_NOEXS;
	}

	rt = ctl_sen_set(sen_id, CTL_SEN_CFGID_USER_DEFINE1, preset_ctrl);

	return rt;
}

ER isp_api_set_direction(ISP_ID id, ISP_SENSOR_DIRECTION *sensor_direction)
{
	UINT32 sen_flip = CTL_SEN_FLIP_NONE;
	UINT32 sen_id;
	ER rt = E_OK;

	if (sie_to_sen_id_map_en[id]) {
		sen_id = sie_to_sen_id_table[id];
	} else {
		sen_id = id;
	}

	if (isp_dbg_get_bypass_eng(sen_id) & ISP_BYPASS_SEN_DIR) {
		return E_OK;
	}

	if ((ctl_sen_status(sen_id) & CTL_SEN_STATUS_PWR_ON) == FALSE) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_WRN_MSG, "CTL_SEN_STATUS_OPEN FALSE, id: %d \r\n", id);
		return E_NOEXS;
	}

	if (sensor_direction->mirror) {
		sen_flip |= CTL_SEN_FLIP_H;
	}
	if (sensor_direction->flip) {
		sen_flip |= CTL_SEN_FLIP_V;
	}

	rt = ctl_sen_set(sen_id, CTL_SEN_CFGID_FLIP_TYPE, &sen_flip);

	return rt;
}

ER isp_api_set_sensor_reg(ISP_ID id, UINT32 *addr, UINT32 *data)
{
	CTL_SEN_CMD cmd = {0};
	UINT32 sen_id;
	ER rt = E_OK;

	if (sie_to_sen_id_map_en[id]) {
		sen_id = sie_to_sen_id_table[id];
	} else {
		sen_id = id;
	}

	if (isp_dbg_get_bypass_eng(sen_id) & ISP_BYPASS_SEN_REG) {
		return E_OK;
	}

	if ((ctl_sen_status(sen_id) & CTL_SEN_STATUS_PWR_ON) == FALSE) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_WRN_MSG, "CTL_SEN_STATUS_OPEN FALSE, id: %d \r\n", id);
		return E_NOEXS;
	}

	cmd.addr = *addr;
	cmd.data[1] = (*data >> 8) & 0xFF;  // MSB
	cmd.data[0] = *data & 0xFF;         // LSB

	PRINT_ISP(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_SENSOR_REG, "set sensor reg., id (%d), addr (0x%X), data (0x%X) \r\n", (int)sen_id, *(int *)addr, *(int *)data);
	rt = ctl_sen_write_reg(sen_id, &cmd);

	return rt;
}

ER isp_api_set_sensor_sleep(ISP_ID id)
{
	UINT32 sen_id;
	ER rt = E_OK;

	if (sie_to_sen_id_map_en[id]) {
		sen_id = sie_to_sen_id_table[id];
	} else {
		sen_id = id;
	}

	if (isp_dbg_get_bypass_eng(sen_id) & ISP_BYPASS_SEN_SLEEP) {
		return E_OK;
	}

	if ((ctl_sen_status(sen_id) & CTL_SEN_STATUS_PWR_ON) == FALSE) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_WRN_MSG, "CTL_SEN_STATUS_OPEN FALSE, id: %d \r\n", id);
		return E_NOEXS;
	}

	rt = ctl_sen_sleep(sen_id);

	return rt;
}

ER isp_api_set_sensor_wakeup(ISP_ID id)
{
	UINT32 sen_id;
	ER rt = E_OK;

	if (sie_to_sen_id_map_en[id]) {
		sen_id = sie_to_sen_id_table[id];
	} else {
		sen_id = id;
	}

	if (isp_dbg_get_bypass_eng(sen_id) & ISP_BYPASS_SEN_WAKEUP) {
		return E_OK;
	}

	if ((ctl_sen_status(sen_id) & CTL_SEN_STATUS_PWR_ON) == FALSE) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(sen_id) & ISP_DBG_WRN_MSG, "CTL_SEN_STATUS_OPEN FALSE, id: %d \r\n", id);
		return E_NOEXS;
	}

	rt = ctl_sen_wakeup(sen_id);

	return rt;
}

ER isp_api_set_yuv(ISP_ID id)
{
	ER rt = E_OK;

	if (id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return 0;
	}
	if (yout_lock[id] == TRUE) {
		yout_lock[id] = FALSE;

		rt = ctl_ipp_isp_set(ipp_id_table[id], CTL_IPP_ISP_ITEM_YUV_OUT, &ctl_ipp_isp_yuv_out);
	} else {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "id = %d, yout_lock: FALSE \r\n", id);
		rt = E_SYS;
	}

	return rt;
}

ER isp_api_set_raw(ISP_ID id)
{
	ER rt = E_OK;

	if (id >= SIE_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "sie id out of range (%d) \r\n", id);
		return E_SYS;
	}
	if (raw_lock[id] == TRUE) {
		if (!ctl_sie_get_is_open(id)) {
			PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "sie is not open (%d) \r\n", id);
			return E_SYS;
		}
		raw_lock[id] = FALSE;
		rt = ctl_sie_isp_set(sie_id_table[id], CTL_SIE_ISP_ITEM_IMG_OUT, &ctl_isp_sie_header_info);
	} else {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "id = %d, raw_lock: FALSE \r\n", id);
		rt = E_SYS;
	}
	return rt;
}

void isp_api_set_emu_enable(BOOL enable)
{
	emu_enable = enable;
}

ER isp_api_set_emu_new_buf(ISP_ID id, CTL_SIE_ISP_SIM_BUF_NEW *param)
{
	ER rt = E_OK;

	if (!ctl_sie_get_is_open(id)) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "sie is not open (%d) \r\n", id);
		return E_SYS;
	}
	rt = ctl_sie_isp_set(sie_id_table[id], CTL_SIE_ISP_ITEM_SIM_BUF_NEW, param);

	return rt;
}

ER isp_api_set_emu_run(ISP_ID id, CTL_SIE_ISP_SIM_BUF_PUSH *param)
{
	ER rt = E_OK;

	if (!param->sim_end) {
		emu_enable = TRUE;
	} else {
		emu_enable = FALSE;
	}

	if (!ctl_sie_get_is_open(id)) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "sie is not open (%d) \r\n", id);
		return E_SYS;
	}
	rt = ctl_sie_isp_set(sie_id_table[id], CTL_SIE_ISP_ITEM_SIM_BUF_PUSH, param);

	return rt;
}

void isp_set_ipp_indep(ISP_ID id, BOOL enable)
{
	isp_sync_ipp_indep[id] = enable;
}

void isp_set_sensor_mode_manual(ISP_ID id, ISP_SENSOR_MODE_MANUAL *manual)
{
	memcpy(&sensor_mode_manual, manual, sizeof(ISP_SENSOR_MODE_MANUAL));
}

void isp_api_set_low_power_lv(UINT32 lv)
{
	if (lv > 5) {
		PRINT_ISP_WRN(TRUE, "low power level out of range (%d) \r\n", lv);
		low_power_lv = 5;
	} else {
		low_power_lv = lv;
	}
}

void isp_api_set_iq_param(ISP_ID id, ISP_IQ_ITEM isp_iq_item, void *param)
{
	UINT32 isp_id = id & 0xFF;
	ER rt = E_OK;

	if (isp_id >= ISP_ID_MAX_NUM) {
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(isp_id) & ISP_DBG_WRN_MSG, "isp id out of range (%d) \r\n", id);
		return;
	}

	switch (isp_iq_item) {
	case ISP_IQ_ITEM_SIE_ROI:
		if (!(isp_dbg_get_bypass_eng(isp_id) & ISP_BYPASS_SIE_ROI)) {
			rt = ctl_sie_isp_set(sie_id_table[isp_id], CTL_SIE_ISP_ITEM_STA_ROI_RATIO, (CTL_SIE_ISP_ROI_RATIO *)param);
			if (rt != E_OK) {
				PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "ctl_sie_isp_set CTL_SIE_ISP_ITEM_STA_ROI_RATIO fail (%d) \r\n", id);
			}
		}
	break;

	case ISP_IQ_ITEM_SIE_PARAM:
		if (!(isp_dbg_get_bypass_eng(isp_id) & ISP_BYPASS_SIE_PARAM)) {
			rt = ctl_sie_isp_set(sie_id_table[isp_id], CTL_SIE_ISP_ITEM_IQ_PARAM, (CTL_SIE_IQ_PARAM *)param);
			if (rt != E_OK) {
				PRINT_ISP_WRN(isp_dbg_get_dbg_mode(id) & ISP_DBG_WRN_MSG, "ctl_sie_isp_set CTL_SIE_ISP_ITEM_IQ_PARAM fail (%d) \r\n", id);
			}
			PRINT_ISP(isp_dbg_get_dbg_mode(isp_id) & ISP_DBG_SIE_SET, "ctl_sie_isp_set (%X) SIE_IQ_PARAM \r\n", sie_id_table[isp_id]);
		}
	break;

	case ISP_IQ_ITEM_IFE_PARAM:
		if (!(isp_dbg_get_bypass_eng(isp_id) & ISP_BYPASS_IFE_PARAM)) {
			ctl_ipp_isp_set(ipp_id_table[isp_id], CTL_IPP_ISP_ITEM_IFE_IQ_PARAM, (CTL_IFE_ISP_IQ_ALL *)param);
			PRINT_ISP(isp_dbg_get_dbg_mode(isp_id) & ISP_DBG_IPP_SET, "ctl_ipp_isp_set (%X) IFE_IQ_PARAM \r\n", ipp_id_table[isp_id]);
		}
	break;

	case ISP_IQ_ITEM_IPE_PARAM:
		if (!(isp_dbg_get_bypass_eng(isp_id) & ISP_BYPASS_IPE_PARAM)) {
			ctl_ipp_isp_set(ipp_id_table[isp_id], CTL_IPP_ISP_ITEM_IPE_IQ_PARAM, (CTL_IPE_ISP_IQ_ALL *)param);
		}
	break;

	case ISP_IQ_ITEM_IME_PARAM:
		if (!(isp_dbg_get_bypass_eng(isp_id) & ISP_BYPASS_IME_PARAM)) {
			ctl_ipp_isp_set(ipp_id_table[isp_id], CTL_IPP_ISP_ITEM_IME_IQ_PARAM, (CTL_IME_ISP_IQ_ALL *)param);
		}
	break;

	case ISP_IQ_ITEM_IFE_VIG_CENT:
		if (!(isp_dbg_get_bypass_eng(isp_id) & ISP_BYPASS_IFE_VIG_CENT)) {
			ctl_ipp_isp_set(ipp_id_table[isp_id], CTL_IPP_ISP_ITEM_IFE_VIG_CENT, (CTL_IPP_ISP_IFE_VIG_CENT_RATIO *)param);
		}
	break;

	case ISP_IQ_ITEM_IFE_VA_WIN_SIZE:
		if (!(isp_dbg_get_bypass_eng(isp_id) & ISP_BYPASS_IFE_VA_WIN_SIZE)) {
			ctl_ipp_isp_set(ipp_id_table[isp_id], CTL_IPP_ISP_ITEM_IFE_VA_WIN_SIZE, (CTL_IPP_ISP_IFE_VA_WIN_SIZE_RATIO *)param);
		}
	break;

	case ISP_IQ_ITEM_IPE_VA_WIN_SIZE:
		if (!(isp_dbg_get_bypass_eng(isp_id) & ISP_BYPASS_IPE_VA_WIN_SIZE)) {
			ctl_ipp_isp_set(ipp_id_table[isp_id], CTL_IPP_ISP_ITEM_IPE_VA_WIN_SIZE, (CTL_IPP_ISP_IPE_VA_WIN_SIZE_RATIO *)param);
		}
	break;

	case ISP_IQ_ITEM_IME_LCA_DBG_X_POS:
		if (!(isp_dbg_get_bypass_eng(isp_id) & ISP_BYPASS_IME_LCA_DBG_X_POS)) {
			ctl_ipp_isp_set(ipp_id_table[isp_id], CTL_IPP_ISP_ITEM_IME_LCA_DBG_X_POS, (CTL_IPP_ISP_IME_LCA_DBG_X_POS_RATIO *)param);
		}
	break;

	case ISP_IQ_ITEM_IFE_FPN_INFO:
		if (!(isp_dbg_get_bypass_eng(isp_id) & ISP_BYPASS_IFE_FPN_INFO)) {
			ctl_ipp_isp_set(ipp_id_table[isp_id], CTL_IPP_ISP_ITEM_FPN_INFO, (CTL_IPP_ISP_FPN_INFO *)param);
		}
	break;

	case ISP_IQ_ITEM_ENC_SHARPEN_PARAM:
		if (!(isp_dbg_get_bypass_eng(isp_id) & ISP_BYPASS_ENC_PARAM)) {
			kflow_videoenc_set(enc_id_table[isp_id], KFLOW_VIDEOENC_ISP_ITEM_SPN, (KDRV_H26XENC_SPN *)param);
			PRINT_ISP(isp_dbg_get_dbg_mode(isp_id) & ISP_DBG_ENC_SET, "kflow_videoenc_set (%d) ENC_SHARPEN_IQ_PARAM \r\n", id);
		}
	break;

	default:
		PRINT_ISP_WRN(isp_dbg_get_dbg_mode(isp_id) & ISP_DBG_WRN_MSG, "ISP_IQ_ITEM out of renage (%d) \r\n", isp_iq_item);
	break;
	}

	return;
}

#if defined(__KERNEL__)
EXPORT_SYMBOL(isp_api_get_expt);
EXPORT_SYMBOL(isp_api_get_gain);
EXPORT_SYMBOL(isp_api_get_sync_time);
EXPORT_SYMBOL(isp_api_get_frame_num);
EXPORT_SYMBOL(isp_api_get_chg_fps);
EXPORT_SYMBOL(isp_api_get_sensor_mode_info);
EXPORT_SYMBOL(isp_api_get_md_sta);
EXPORT_SYMBOL(isp_api_get_ir_info);
EXPORT_SYMBOL(isp_api_get_enc_isp_ratio);
EXPORT_SYMBOL(isp_api_get_fastboot_sensor_info);
EXPORT_SYMBOL(isp_api_get_fastboot_valid);
EXPORT_SYMBOL(isp_api_get_fastboot_sensor_valid);
EXPORT_SYMBOL(isp_api_set_expt);
EXPORT_SYMBOL(isp_api_set_gain);
EXPORT_SYMBOL(isp_api_set_preset);
EXPORT_SYMBOL(isp_api_set_iq_param);
#endif

