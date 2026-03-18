#if defined(__FREERTOS)
#include <string.h>
#include <stdio.h>
#endif
#include "kwrap/error_no.h"
#include "kwrap/type.h"
#include "kwrap/cpu.h"

#include "comm/hwclock.h"

#include "iq_alg.h"
#include "iq_ui_int.h"
#include "iqt_api.h"
#include "isp_dev.h"
#include "isp_uti.h"

#include "iq_alg_int.h"
#include "iq_common_param_int.h"
#include "iq_flow.h"
#include "iq_msg.h"
#include "iqt_api_int.h"
#include "iq_param_default.h"
#include "iq_task_int.h"
#include "iq_dbg.h"
#include "iq_version.h"

//=============================================================================
// global
//=============================================================================
UINT32 iq_info_buffer_size;
UINT32 iq_info_buffer_dpc_ofs;
UINT32 iq_info_buffer_ecs_ofs;
UINT32 iq_info_buffer_3dcc_ofs;
IQ_PARAM_PTR *iq_param[IQ_ID_MAX_NUM] = {NULL};
IQ_PARAM_PTR iq_param_memalloc_addr[IQ_ID_MAX_NUM] = {0};
static BOOL iq_param_memalloc_valid[IQ_ID_MAX_NUM] = {0};
static BOOL iq_param_id_valid[IQ_ID_MAX_NUM] = {0};
static UINT32 iq_info_id_map[IQ_ID_MAX_NUM] = {0};
static UINT32 iq_info_id_map_index;
static BOOL iq_dpc_en;
static BOOL iq_ecs_en;
static BOOL iq_3dcc_en;
// temp for proc test
BOOL test_mode_en[IQ_ID_MAX_NUM] = {0};
IQ_OB_MODE test_mode_ob[IQ_ID_MAX_NUM] = {0};
IQ_DG_MODE test_mode_dg[IQ_ID_MAX_NUM] = {0};
IQ_CG_MODE test_mode_cg[IQ_ID_MAX_NUM] = {0};
BOOL companding_en[IQ_ID_MAX_NUM] = {0};
BOOL fcurve_en[IQ_ID_MAX_NUM] = {0};
BOOL cfa_en[IQ_ID_MAX_NUM] = {0};
BOOL test_shdr_en[IQ_ID_MAX_NUM] = {0};
UINT32 test_ev_ratio[IQ_ID_MAX_NUM][ISP_SEN_MFRAME_MAX_NUM] = {0};
UINT32 test_tm_ratio[IQ_ID_MAX_NUM] = {0};
BOOL test_shdr_hbs_en[IQ_ID_MAX_NUM] = {0};
ISP_AE_HBS_PARAM test_hbs[IQ_ID_MAX_NUM] = {0};
UINT32 nr_r_ratio[IQ_ID_MAX_NUM] = {0};
UINT32 nr_b_ratio[IQ_ID_MAX_NUM] = {0};
UINT32 nr_ir_ratio[IQ_ID_MAX_NUM] = {0};
UINT32 residue_reset_num[IQ_ID_MAX_NUM] = {0};
BOOL tmnr_ae_still_en[IQ_ID_MAX_NUM] = {0};
BOOL is_update_shdr_param[IQ_ID_MAX_NUM] = {0};

static UINT8 iq_defog_diff_wt[8][CTL_IPE_ISP_DFG_OUTPUT_BLD_LEN] = {
	{0, 0, 0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
	{3, 3, 3, 6, 9, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12},
	{6, 6, 6, 9, 12, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15},
	{12, 12, 12, 16, 20, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24},
	{18, 18, 18, 22, 26, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30},
	{24, 24, 24, 28, 32, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36},
	{30, 30, 30, 34, 38, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42},
	{36, 36, 36, 40, 44, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48}
};

ISP_MODULE iq_module;
#if defined(__KERNEL__)
extern INT32 iq_ioctl(INT32 cmd, ULONG arg, ULONG *buf_addr);
#endif

//=============================================================================
// function declaration
//=============================================================================

//=============================================================================
// internal functions
//=============================================================================
static INT32 iq_flow_get_param_addr(IQ_ID id)
{
	UINT32 total_param_size;
	IQ_PARAM_PTR *iq_param_temp = NULL;
	void *param_mem_addr = NULL;
	static BOOL use_param_phy_addr = TRUE;
	int align_byte = 4;

	if (use_param_phy_addr == TRUE) {
		use_param_phy_addr = FALSE;
		iq_param[id] = (IQ_PARAM_PTR *)iq_get_param_default();
	} else {
		total_param_size = ALIGN_CEIL(sizeof(IQ_OB_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_NR_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_CFA_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_VA_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_GAMMA_PARAM), align_byte) +
							ALIGN_CEIL(sizeof(IQ_CCM_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_COLOR_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_CONTRAST_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_EDGE_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_3DNR_PARAM), align_byte) +
							ALIGN_CEIL(sizeof(IQ_PFR_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_WDR_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_DEFOG_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_SHDR_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_COMPANDING_PARAM), align_byte) +
							ALIGN_CEIL(sizeof(IQ_RGBIR_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_RGBIR_ENH_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_POST_SHARPEN_2_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_YCURVE_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_CST_PARAM), align_byte) +
							ALIGN_CEIL(sizeof(IQ_POST_SHARPEN_1_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_TONE_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_WDR_ENH_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_RAW_VA_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_FPN_PARAM), align_byte) +
							ALIGN_CEIL(sizeof(IQ_BNR_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_AIISP_PARAM), align_byte) + ALIGN_CEIL(sizeof(IQ_AIISP_CUSTOM_PARAM), align_byte);

		param_mem_addr = isp_uti_vmem_alloc(total_param_size);
		if (param_mem_addr == NULL) {
			DBG_ERR("fail to allocate iq parameter fail!\n");
			return -E_SYS;
		}

		iq_param_memalloc_addr[id].ob = (IQ_OB_PARAM *)param_mem_addr;
		iq_param_memalloc_addr[id].nr = (IQ_NR_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].ob + ALIGN_CEIL(sizeof(IQ_OB_PARAM), align_byte));
		iq_param_memalloc_addr[id].cfa = (IQ_CFA_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].nr + ALIGN_CEIL(sizeof(IQ_NR_PARAM), align_byte));
		iq_param_memalloc_addr[id].va = (IQ_VA_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].cfa + ALIGN_CEIL(sizeof(IQ_CFA_PARAM), align_byte));
		iq_param_memalloc_addr[id].gamma = (IQ_GAMMA_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].va + ALIGN_CEIL(sizeof(IQ_VA_PARAM), align_byte));
		iq_param_memalloc_addr[id].ccm = (IQ_CCM_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].gamma + ALIGN_CEIL(sizeof(IQ_GAMMA_PARAM), align_byte));
		iq_param_memalloc_addr[id].color = (IQ_COLOR_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].ccm + ALIGN_CEIL(sizeof(IQ_CCM_PARAM), align_byte));
		iq_param_memalloc_addr[id].contrast = (IQ_CONTRAST_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].color + ALIGN_CEIL(sizeof(IQ_COLOR_PARAM), align_byte));
		iq_param_memalloc_addr[id].edge = (IQ_EDGE_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].contrast + ALIGN_CEIL(sizeof(IQ_CONTRAST_PARAM), align_byte));
		iq_param_memalloc_addr[id]._3dnr = (IQ_3DNR_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].edge + ALIGN_CEIL(sizeof(IQ_EDGE_PARAM), align_byte));
		iq_param_memalloc_addr[id].pfr = (IQ_PFR_PARAM *)((UINT8 *)iq_param_memalloc_addr[id]._3dnr + ALIGN_CEIL(sizeof(IQ_3DNR_PARAM), align_byte));
		iq_param_memalloc_addr[id].wdr = (IQ_WDR_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].pfr + ALIGN_CEIL(sizeof(IQ_PFR_PARAM), align_byte));
		iq_param_memalloc_addr[id].defog = (IQ_DEFOG_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].wdr + ALIGN_CEIL(sizeof(IQ_WDR_PARAM), align_byte));
		iq_param_memalloc_addr[id].shdr = (IQ_SHDR_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].defog + ALIGN_CEIL(sizeof(IQ_DEFOG_PARAM), align_byte));
		iq_param_memalloc_addr[id].companding = (IQ_COMPANDING_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].shdr + ALIGN_CEIL(sizeof(IQ_SHDR_PARAM), align_byte));
		iq_param_memalloc_addr[id].rgbir = (IQ_RGBIR_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].companding + ALIGN_CEIL(sizeof(IQ_COMPANDING_PARAM), align_byte));
		iq_param_memalloc_addr[id].rgbir_enh = (IQ_RGBIR_ENH_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].rgbir + ALIGN_CEIL(sizeof(IQ_RGBIR_PARAM), align_byte));
		iq_param_memalloc_addr[id].post_sharpen_2 = (IQ_POST_SHARPEN_2_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].rgbir_enh + ALIGN_CEIL(sizeof(IQ_RGBIR_ENH_PARAM), align_byte));
		iq_param_memalloc_addr[id].ycurve = (IQ_YCURVE_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].post_sharpen_2 + ALIGN_CEIL(sizeof(IQ_POST_SHARPEN_2_PARAM), align_byte));
		iq_param_memalloc_addr[id].cst = (IQ_CST_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].ycurve + ALIGN_CEIL(sizeof(IQ_YCURVE_PARAM), align_byte));
		iq_param_memalloc_addr[id].post_sharpen_1 = (IQ_POST_SHARPEN_1_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].cst + ALIGN_CEIL(sizeof(IQ_CST_PARAM), align_byte));
		iq_param_memalloc_addr[id].tone = (IQ_TONE_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].post_sharpen_1 + ALIGN_CEIL(sizeof(IQ_POST_SHARPEN_1_PARAM), align_byte));
		iq_param_memalloc_addr[id].wdr_enh = (IQ_WDR_ENH_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].tone + ALIGN_CEIL(sizeof(IQ_TONE_PARAM), align_byte));
		iq_param_memalloc_addr[id].raw_va = (IQ_RAW_VA_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].wdr_enh + ALIGN_CEIL(sizeof(IQ_WDR_ENH_PARAM), align_byte));
		iq_param_memalloc_addr[id].fpn = (IQ_FPN_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].raw_va + ALIGN_CEIL(sizeof(IQ_RAW_VA_PARAM), align_byte));
		iq_param_memalloc_addr[id].bnr = (IQ_BNR_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].fpn + ALIGN_CEIL(sizeof(IQ_FPN_PARAM), align_byte));
		iq_param_memalloc_addr[id].aiisp = (IQ_AIISP_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].bnr + ALIGN_CEIL(sizeof(IQ_BNR_PARAM), align_byte));
		iq_param_memalloc_addr[id].aiisp_custom = (IQ_AIISP_CUSTOM_PARAM *)((UINT8 *)iq_param_memalloc_addr[id].aiisp + ALIGN_CEIL(sizeof(IQ_AIISP_PARAM), align_byte));
		// DPC, SHADING, 3DCC...
		iq_param_temp = (IQ_PARAM_PTR *)iq_get_param_default();
		memcpy(iq_param_memalloc_addr[id].ob, iq_param_temp->ob, sizeof(IQ_OB_PARAM));
		memcpy(iq_param_memalloc_addr[id].nr, iq_param_temp->nr, sizeof(IQ_NR_PARAM));
		memcpy(iq_param_memalloc_addr[id].cfa, iq_param_temp->cfa, sizeof(IQ_CFA_PARAM));
		memcpy(iq_param_memalloc_addr[id].va, iq_param_temp->va, sizeof(IQ_VA_PARAM));
		memcpy(iq_param_memalloc_addr[id].gamma, iq_param_temp->gamma, sizeof(IQ_GAMMA_PARAM));
		memcpy(iq_param_memalloc_addr[id].ccm, iq_param_temp->ccm, sizeof(IQ_CCM_PARAM));
		memcpy(iq_param_memalloc_addr[id].color, iq_param_temp->color, sizeof(IQ_COLOR_PARAM));
		memcpy(iq_param_memalloc_addr[id].contrast, iq_param_temp->contrast, sizeof(IQ_CONTRAST_PARAM));
		memcpy(iq_param_memalloc_addr[id].edge, iq_param_temp->edge, sizeof(IQ_EDGE_PARAM));
		memcpy(iq_param_memalloc_addr[id]._3dnr, iq_param_temp->_3dnr, sizeof(IQ_3DNR_PARAM));
		memcpy(iq_param_memalloc_addr[id].pfr, iq_param_temp->pfr, sizeof(IQ_PFR_PARAM));
		memcpy(iq_param_memalloc_addr[id].wdr, iq_param_temp->wdr, sizeof(IQ_WDR_PARAM));
		memcpy(iq_param_memalloc_addr[id].defog, iq_param_temp->defog, sizeof(IQ_DEFOG_PARAM));
		memcpy(iq_param_memalloc_addr[id].shdr, iq_param_temp->shdr, sizeof(IQ_SHDR_PARAM));
		memcpy(iq_param_memalloc_addr[id].companding, iq_param_temp->companding, sizeof(IQ_COMPANDING_PARAM));
		memcpy(iq_param_memalloc_addr[id].rgbir, iq_param_temp->rgbir, sizeof(IQ_RGBIR_PARAM));
		memcpy(iq_param_memalloc_addr[id].rgbir_enh, iq_param_temp->rgbir_enh, sizeof(IQ_RGBIR_ENH_PARAM));
		memcpy(iq_param_memalloc_addr[id].post_sharpen_2, iq_param_temp->post_sharpen_2, sizeof(IQ_POST_SHARPEN_2_PARAM));
		memcpy(iq_param_memalloc_addr[id].ycurve, iq_param_temp->ycurve, sizeof(IQ_YCURVE_PARAM));
		memcpy(iq_param_memalloc_addr[id].cst, iq_param_temp->cst, sizeof(IQ_CST_PARAM));
		memcpy(iq_param_memalloc_addr[id].post_sharpen_1, iq_param_temp->post_sharpen_1, sizeof(IQ_POST_SHARPEN_1_PARAM));
		memcpy(iq_param_memalloc_addr[id].tone, iq_param_temp->tone, sizeof(IQ_TONE_PARAM));
		memcpy(iq_param_memalloc_addr[id].wdr_enh, iq_param_temp->wdr_enh, sizeof(IQ_WDR_ENH_PARAM));
		memcpy(iq_param_memalloc_addr[id].raw_va, iq_param_temp->raw_va, sizeof(IQ_RAW_VA_PARAM));
		memcpy(iq_param_memalloc_addr[id].fpn, iq_param_temp->fpn, sizeof(IQ_FPN_PARAM));
		memcpy(iq_param_memalloc_addr[id].bnr, iq_param_temp->bnr, sizeof(IQ_BNR_PARAM));
		memcpy(iq_param_memalloc_addr[id].aiisp, iq_param_temp->aiisp, sizeof(IQ_AIISP_PARAM));
		memcpy(iq_param_memalloc_addr[id].aiisp_custom, iq_param_temp->aiisp_custom, sizeof(IQ_AIISP_CUSTOM_PARAM));
		// DPC, SHADING, 3DCC...
		iq_param[id] = &iq_param_memalloc_addr[id];
		iq_param_memalloc_valid[id] = TRUE;
	}
	return E_OK;
}

//=============================================================================
// extern functions
//=============================================================================
ISP_MODULE *iq_get_module(void)
{
	return &iq_module;
}

ER iq_init_module(UINT32 id_list, UINT32 dpc_en, UINT32 ecs_en, UINT32 _3dcc_en)
{
	IQ_EVENT_OBJ iq_event_tab = {0};
	void *pinfo = NULL;
	UINT32 buffer_num = 0, i;

	if (id_list == 0) {
		DBG_WRN("iq_id_list = 0, modify to 1 \r\n");
		id_list = 1;
	}

	// clean IQ module
	memset(&iq_module, 0x0, sizeof(ISP_MODULE));

	// calculate buffer needed
	for (i = 0; i < IQ_ID_MAX_NUM; i++) {
		if ((id_list >> i) & 0x1) {
			buffer_num++;
		}
	}

	// allocate private data
	// ** private buffer **
	// ================================
	// |          IQALG_INFO          |
	// --------------------------------
	// |          IQ_SIE_DPC          |  << if (dpc_en)
	// --------------------------------
	// |          IQ_SIE_ECS          |  << if (ecs_en)
	// --------------------------------
	// |     IQ_REF_SHADING_PARAM     |  << if (ecs_en)
	// --------------------------------
	// |       CTL_IPE_ISP_3DCC       |  << if (3dcc_en)
	// --------------------------------
	// |       IQ_REF_3DCC_PARAM      |  << if (3dcc_en)
	// ================================
	iq_info_buffer_size = sizeof(IQALG_INFO);
	iq_info_buffer_dpc_ofs = sizeof(IQALG_INFO);
	iq_info_buffer_ecs_ofs = sizeof(IQALG_INFO);
	iq_info_buffer_3dcc_ofs = sizeof(IQALG_INFO);
	if (dpc_en != 0) {
		iq_dpc_en = TRUE;
		iq_info_buffer_size += sizeof(IQ_SIE_DPC);
		iq_info_buffer_ecs_ofs = iq_info_buffer_dpc_ofs + sizeof(IQ_SIE_DPC);
		iq_info_buffer_3dcc_ofs = iq_info_buffer_dpc_ofs + sizeof(IQ_SIE_DPC);
	}
	if (ecs_en != 0) {
		iq_ecs_en = TRUE;
		iq_info_buffer_size += sizeof(IQ_SIE_ECS);
		iq_info_buffer_size += sizeof(IQ_REF_SHADING_PARAM);
		iq_info_buffer_3dcc_ofs = iq_info_buffer_ecs_ofs + sizeof(IQ_SIE_ECS) + sizeof(IQ_REF_SHADING_PARAM);
	}
	if (_3dcc_en != 0) {
		iq_3dcc_en = TRUE;
		iq_info_buffer_size += sizeof(CTL_IPE_ISP_3DCC);
		iq_info_buffer_size += sizeof(IQ_REF_3DCC_PARAM);
	}
	pinfo = isp_uti_kmem_alloc(iq_info_buffer_size * buffer_num);
	if (pinfo == NULL) {
		DBG_ERR("fail to allocate iq_info_t fail!! \r\n");
		return -E_SYS;
	}
	memset(pinfo, 0, iq_info_buffer_size * buffer_num);

	sprintf(iq_module.name, "NVT_IQ");
	iq_module.private =         pinfo;
	#if defined(__FREERTOS)
	iq_module.fn_init =         NULL;
	iq_module.fn_uninit =       NULL;
	iq_module.fn_trig =         iq_task_trig;
	iq_module.fn_pause =        NULL;
	iq_module.fn_resume =       NULL;
	iq_module.fn_ioctl =        NULL;
	#else
	iq_module.fn_init =         iq_task_init;
	iq_module.fn_uninit =       iq_task_uninit;
	iq_module.fn_trig =         iq_task_trig;
	iq_module.fn_pause =        iq_task_pause;
	iq_module.fn_resume =       iq_task_resume;
	iq_module.fn_ioctl =        iq_ioctl;
	#endif

	// register to isp
	isp_dev_reg_iq_module(&iq_module);

	// install thread/task
	iq_task_id_install();
	iq_ui_init();
	iq_event_tab.init =    iq_flow_init;
	iq_event_tab.uninit =  iq_flow_uninit;
	iq_event_tab.process = iq_flow_process;
	iq_event_tab.pause =   NULL;
	iq_event_tab.resume =  NULL;
	for (i = 0; i < IQ_ID_MAX_NUM; i++) {
		if ((id_list >> i) & 0x1) {
			iq_task_open(i, &iq_event_tab);
			iq_task_init(i);
		}
	}
	return 0;
}

ER iq_uninit_module(void)
{
	UINT32 i;

	if (iq_module.private) {
		isp_uti_kmem_free(iq_module.private);
		iq_module.private = NULL;
	}

	memset(&iq_module, 0x0, sizeof(iq_module));

	// uninstall thread/task
	for (i = 0; i < IQ_ID_MAX_NUM; i++) {
		if (iq_param_id_valid[i]) {
			iq_task_uninit(i);
			iq_task_close(i);
		}
	}
	iq_task_id_uninstall();

	// register NULL to isp
	isp_dev_reg_iq_module(NULL);

	return E_OK;
}

static void iq_flow_reset_sie_setting(IQALG_INFO *iq_info)
{
	iq_info->final_setting.sie_roi = 0;
}

static void iq_flow_reset_sie_param(IQALG_INFO *iq_info)
{
	memset(&iq_info->final_setting.sie, 0, sizeof(CTL_SIE_IQ_PARAM));
}

static void iq_flow_reset_ipp_setting(IQALG_INFO *iq_info)
{
	memset(&iq_info->final_setting.ipp, 0, sizeof(IQ_KFLOW_IPP_SETTING));
}

static void iq_flow_reset_ipp_param(IQALG_INFO *iq_info)
{
	memset(&iq_info->final_setting.ife, 0, sizeof(CTL_IFE_ISP_IQ_ALL));
	memset(&iq_info->final_setting.ipe, 0, sizeof(CTL_IPE_ISP_IQ_ALL));
	memset(&iq_info->final_setting.ime, 0, sizeof(CTL_IME_ISP_IQ_ALL));
}

static void iq_flow_reset_enc_param(IQALG_INFO *iq_info)
{
	iq_info->final_setting.enc_sharpen = 0;
}

static ER iq_flow_set_sie_setting(IQALG_INFO *iq_info)
{
	UINT32 isp_id = iq_info->id;

	if (iq_info->final_sie.ready_flag != TRUE) {
		PRINT_IQ_WRN(iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_WRN_MSG, "SIE param (%d) not ready!! \r\n", iq_info->id);
		return E_PAR;
	}

	iq_info->final_setting.sie_roi = &(iq_info->final_sie.sie_roi_ratio);

	isp_api_set_iq_param(isp_id, ISP_IQ_ITEM_SIE_ROI, iq_info->final_setting.sie_roi);
	return E_OK;
}

static ER iq_flow_set_sie_param(IQALG_INFO *iq_info)
{
	UINT32 isp_id = iq_info->id;

	if (iq_info->final_sie.ready_flag != TRUE) {
		PRINT_IQ_WRN(iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_WRN_MSG, "SIE param (%d) not ready!! \r\n", iq_info->id);
		return E_PAR;
	}

	// NOTE: Hardware limitation
	iq_info->final_setting.sie.ob_param = &(iq_info->final_sie.sie_ob);
	iq_info->final_setting.sie.ca_param = &(iq_info->final_sie.sie_ca);
	iq_info->final_setting.sie.la_param = &(iq_info->final_sie.sie_la);
	iq_info->final_setting.sie.cgain_param = &(iq_info->final_sie.sie_cgain);
	if ((iq_info->dpc_en == TRUE) && (iq_info->sie_tab_update & IQ_TBL_SIE_DPC)) {
		iq_info->final_setting.sie.dpc_param = &(iq_info->final_setting.sie_dpc_param);
		iq_info->final_setting.sie.dpc_param->enable = iq_info->final_sie.sie_dpc->enable;
		iq_info->final_setting.sie.dpc_param->mode = iq_info->final_sie.sie_dpc->mode;
		iq_info->final_setting.sie.dpc_param->table_viraddr = (iq_info->final_sie.sie_dpc->expand_en == TRUE) ? iq_info->final_sie.sie_dpc->expand_table_viraddr : (ULONG)iq_info->final_sie.sie_dpc->table;
		if (*((UINT16 *)(iq_info->final_setting.sie.dpc_param->table_viraddr)) == 0xFFFF) {
			iq_info->final_setting.sie.dpc_param->enable = FALSE;
		}
		iq_info->final_setting.sie.dpc_param->table_phyaddr = (iq_info->final_sie.sie_dpc->expand_en == TRUE) ? iq_info->final_sie.sie_dpc->expand_table_phyaddr : iq_info->final_sie.sie_dpc->table_phyaddr;
		iq_info->final_setting.sie.dpc_param->weight = iq_info->final_sie.sie_dpc->weight;
		iq_info->final_setting.sie.dpc_param->dp_total_size = iq_info->final_sie.sie_dpc->dp_total_size;
		iq_info->final_setting.sie.dpc_param->dbg_value = iq_info->final_sie.sie_dpc->dbg_value;
	}
	if ((iq_info->ecs_en == TRUE) && (iq_info->sie_tab_update & IQ_TBL_SIE_ECS)) {
		iq_info->final_setting.sie.ecs_param = &(iq_info->final_setting.sie_ecs_param);
		iq_info->final_setting.sie.ecs_param->enable = iq_info->final_sie.sie_ecs->enable;
		iq_info->final_setting.sie.ecs_param->sel_37_fmt = iq_info->final_sie.sie_ecs->sel_37_fmt;
		iq_info->final_setting.sie.ecs_param->map_tbl_viraddr = (ULONG)iq_info->final_sie.sie_ecs->map_tbl;
		iq_info->final_setting.sie.ecs_param->map_tbl_phyaddr = iq_info->final_sie.sie_ecs->map_tbl_phyaddr;
		iq_info->final_setting.sie.ecs_param->map_sel = iq_info->final_sie.sie_ecs->map_sel;
		iq_info->final_setting.sie.ecs_param->dthr_enable = iq_info->final_sie.sie_ecs->dthr_enable;
		iq_info->final_setting.sie.ecs_param->dthr_reset = iq_info->final_sie.sie_ecs->dthr_reset;
		iq_info->final_setting.sie.ecs_param->dthr_level = iq_info->final_sie.sie_ecs->dthr_level;
		iq_info->final_setting.sie.ecs_param->bayer_mode = iq_info->final_sie.sie_ecs->bayer_mode;
	}
	iq_info->final_setting.sie.companding_param = &(iq_info->final_sie.sie_companding);
	iq_info->final_setting.sie.dgain_param = &(iq_info->final_sie.sie_dgain);

	isp_api_set_iq_param(isp_id, ISP_IQ_ITEM_SIE_PARAM, &iq_info->final_setting.sie);
	return E_OK;
}

static ER iq_flow_set_ipp_setting(IQALG_INFO *iq_info)
{
	UINT32 isp_id = (iq_info->ipp_trig_obj.ipp_info << 16) + iq_info->id;

	if (iq_info->final_ipp.ready_flag != TRUE) {
		PRINT_IQ_WRN(iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_WRN_MSG, "IPP param (%d) not ready!! \r\n", iq_info->id);
		return E_PAR;
	}

	iq_info->final_setting.ipp.ife_cent = &(iq_info->final_ipp.ife_cent_ratio);
	isp_api_set_iq_param(isp_id, ISP_IQ_ITEM_IFE_VIG_CENT, iq_info->final_setting.ipp.ife_cent);

	iq_info->final_setting.ipp.ife_va_win_size = &(iq_info->final_ipp.ife_va_win_size);
	isp_api_set_iq_param(isp_id, ISP_IQ_ITEM_IFE_VA_WIN_SIZE, iq_info->final_setting.ipp.ife_va_win_size);

	iq_info->final_setting.ipp.ipe_va_win_size = &(iq_info->final_ipp.ipe_va_win_size);
	isp_api_set_iq_param(isp_id, ISP_IQ_ITEM_IPE_VA_WIN_SIZE, iq_info->final_setting.ipp.ipe_va_win_size);

	iq_info->final_setting.ipp.ime_lca_dbg_x_pos = &(iq_info->final_ipp.ime_lca_dbg_x_pos);
	isp_api_set_iq_param(isp_id, ISP_IQ_ITEM_IME_LCA_DBG_X_POS, iq_info->final_setting.ipp.ime_lca_dbg_x_pos);

	iq_info->final_setting.ipp.ife_fpn_info = &(iq_info->final_ipp.ife_fpn_info);
	isp_api_set_iq_param(isp_id, ISP_IQ_ITEM_IFE_FPN_INFO, iq_info->final_setting.ipp.ife_fpn_info);
	return E_OK;
}

static ER iq_flow_set_ife_param(IQALG_INFO *iq_info)
{
	UINT32 isp_id = (iq_info->ipp_trig_obj.ipp_info << 16) + iq_info->id;

	if (iq_info->final_ipp.ready_flag != TRUE) {
		PRINT_IQ_WRN(iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_WRN_MSG, "IFE param (%d) not ready!! \r\n", iq_info->id);
		return E_PAR;
	}

	iq_info->final_setting.ife.p_nrs0 = &(iq_info->final_ipp.ife_nrs_0);
	iq_info->final_setting.ife.p_fcurve = &(iq_info->final_ipp.ife_fcurve);
	iq_info->final_setting.ife.p_fusion = &(iq_info->final_ipp.ife_fusion);
	iq_info->final_setting.ife.p_outl = &(iq_info->final_ipp.ife_outl);
	iq_info->final_setting.ife.p_filt = &(iq_info->final_ipp.ife_filter);
	iq_info->final_setting.ife.p_dgain = &(iq_info->final_ipp.ife_dgain);
	iq_info->final_setting.ife.p_cgain = &(iq_info->final_ipp.ife_cgain);
	iq_info->final_setting.ife.p_vig = &(iq_info->final_ipp.ife_vig);
	iq_info->final_setting.ife.p_gbal = &(iq_info->final_ipp.ife_gbal);
	iq_info->final_setting.ife.p_wdr = &(iq_info->final_ipp.ife_wdr);
	iq_info->final_setting.ife.p_wdr_subimg = &(iq_info->final_ipp.ife_wdr_subimg);
	iq_info->final_setting.ife.p_hist = &(iq_info->final_ipp.ife_hist);
	iq_info->final_setting.ife.p_va = &(iq_info->final_ipp.ife_va);
	iq_info->final_setting.ife.p_subisp_iq = &(iq_info->final_ipp.ife_subisp);
	iq_info->final_setting.ife.p_fpn = &(iq_info->final_ipp.ife_fpn);

	isp_api_set_iq_param(isp_id, ISP_IQ_ITEM_IFE_PARAM, &iq_info->final_setting.ife);
	return E_OK;
}

static ER iq_flow_set_ipe_param(IQALG_INFO *iq_info)
{
	UINT32 isp_id = (iq_info->ipp_trig_obj.ipp_info << 16) + iq_info->id;

	if (iq_info->final_ipp.ready_flag != TRUE) {
		PRINT_IQ_WRN(iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_WRN_MSG, "IPE param (%d) not ready!! \r\n", iq_info->id);
		return E_PAR;
	}

	iq_info->final_setting.ipe.p_eext = &(iq_info->final_ipp.ipe_eext);
	iq_info->final_setting.ipe.p_eext_tonemap = &(iq_info->final_ipp.ipe_eext_tonemap);
	iq_info->final_setting.ipe.p_edge_overshoot = &(iq_info->final_ipp.ipe_edge_overshoot);
	iq_info->final_setting.ipe.p_eproc = &(iq_info->final_ipp.ipe_eproc);
	iq_info->final_setting.ipe.p_rgb_lpf = &(iq_info->final_ipp.ipe_rgblpf);
	iq_info->final_setting.ipe.p_pfr = &(iq_info->final_ipp.ipe_pfr);
	iq_info->final_setting.ipe.p_cc = &(iq_info->final_ipp.ipe_cc);
	iq_info->final_setting.ipe.p_ccm = &(iq_info->final_ipp.ipe_ccm);
	iq_info->final_setting.ipe.p_cctrl = &(iq_info->final_ipp.ipe_cctrl);
	iq_info->final_setting.ipe.p_cadj_ee = &(iq_info->final_ipp.ipe_cadj_ee);
	iq_info->final_setting.ipe.p_cadj_yccon = &(iq_info->final_ipp.ipe_cadj_yccon);
	iq_info->final_setting.ipe.p_cadj_cofs = &(iq_info->final_ipp.ipe_cadj_cofs);
	iq_info->final_setting.ipe.p_cadj_rand = &(iq_info->final_ipp.ipe_cadj_rand);
	iq_info->final_setting.ipe.p_cadj_hue = &(iq_info->final_ipp.ipe_cadj_hue);
	iq_info->final_setting.ipe.p_cadj_fixth = &(iq_info->final_ipp.ipe_cadj_fixth);
	iq_info->final_setting.ipe.p_cadj_mask = &(iq_info->final_ipp.ipe_cadj_mask);
	iq_info->final_setting.ipe.p_cst = &(iq_info->final_ipp.ipe_cst);
	iq_info->final_setting.ipe.p_cstp = &(iq_info->final_ipp.ipe_cstp);
	iq_info->final_setting.ipe.p_gamy_rand = &(iq_info->final_ipp.ipe_gamyrand);
	if ((iq_info->ipp_trig_obj.reset == TRUE) || (iq_info->ipp_tab_update & IQ_TBL_IPE_GAMMA)) {
		iq_info->final_setting.ipe.p_gamma = &(iq_info->final_ipp.ipe_gamma);
	}
	if ((iq_info->ipp_trig_obj.reset == TRUE) || (iq_info->ipp_tab_update & IQ_TBL_IPE_YCURVE)) {
		iq_info->final_setting.ipe.p_y_curve = &(iq_info->final_ipp.ipe_ycurve);
	}
	iq_info->final_setting.ipe.p_defog = &(iq_info->final_ipp.ipe_defog);
	iq_info->final_setting.ipe.p_lce = &(iq_info->final_ipp.ipe_lce);
	iq_info->final_setting.ipe.p_subimg = &(iq_info->final_ipp.ipe_subimg);
	iq_info->final_setting.ipe.p_edgedbg = &(iq_info->final_ipp.ipe_edgedbg);
	iq_info->final_setting.ipe.p_va = &(iq_info->final_ipp.ipe_va);
	iq_info->final_setting.ipe.p_edge_region_str = &(iq_info->final_ipp.ipe_edge_region_str);
	iq_info->final_setting.ipe.p_cfa = &(iq_info->final_ipp.ipe_cfa);
	if (iq_info->_3dcc_en == TRUE) {
		iq_info->final_setting.ipe.p_3dcc = iq_info->final_ipp.ipe_3dcc;
	}

	isp_api_set_iq_param(isp_id, ISP_IQ_ITEM_IPE_PARAM, &iq_info->final_setting.ipe);
	return E_OK;
}

static ER iq_flow_set_ime_param(IQALG_INFO *iq_info)
{
	UINT32 isp_id = (iq_info->ipp_trig_obj.ipp_info << 16) + iq_info->id;

	if (iq_info->final_ipp.ready_flag != TRUE) {
		PRINT_IQ_WRN(iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_WRN_MSG, "IME param (%d) not ready!! \r\n", iq_info->id);
		return E_PAR;
	}

	iq_info->final_setting.ime.p_lca = &(iq_info->final_ipp.ime_lca);
	iq_info->final_setting.ime.p_dbcs = &(iq_info->final_ipp.ime_dbcs);
	iq_info->final_setting.ime.p_tmnr = &(iq_info->final_ipp.ime_tmnr);
	iq_info->final_setting.ime.p_ycccvt = &(iq_info->final_ipp.ime_ycccvt);
	iq_info->final_setting.ime.p_sharpen = &(iq_info->final_ipp.ime_sharpen);

	isp_api_set_iq_param(isp_id, ISP_IQ_ITEM_IME_PARAM, &iq_info->final_setting.ime);
	return E_OK;
}

static ER iq_flow_set_enc_param(IQALG_INFO *iq_info)
{
	UINT32 isp_id = iq_info->id;

	if (iq_info->final_enc.ready_flag != TRUE) {
		PRINT_IQ_WRN(iq_dbg_get_dbg_mode(iq_info->id) & IQ_DBG_WRN_MSG, "ENC param (%d) not ready !! \r\n", iq_info->id);
		return E_PAR;
	}

	iq_info->final_setting.enc_sharpen = &iq_info->final_enc.post_sharpen;

	isp_api_set_iq_param(isp_id, ISP_IQ_ITEM_ENC_SHARPEN_PARAM, iq_info->final_setting.enc_sharpen);
	return E_OK;
}

BOOL iq_flow_get_id_valid(UINT32 id)
{
	return iq_param_id_valid[id];
}

UINT32 iq_flow_get_info_map(UINT32 id)
{
	return iq_info_id_map[id];
}

INT32 iq_flow_init(UINT32 id)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;
	ISP_IQ_INIT_INFO iq_init_info = {0};
	IQ_PARAM_PTR *iq_builtin_param = NULL;
	ER iq_sync_rt = E_OK;
	UINT32 dbg_mode = iq_dbg_get_dbg_mode(id);
	UINT32 i;
	ER rt = E_OK;

	if (id >= IQ_ID_MAX_NUM) {
		PRINT_IQ_WRN(dbg_mode & IQ_DBG_WRN_MSG, "IQ ID (%d) out of range!! \r\n", id);
		return E_SYS;
	}

	rt = iq_flow_get_param_addr(id);

	if (rt != E_OK) {
		return rt;
	}

	iq_param_id_valid[id] = TRUE;
	iq_info_id_map[id] = iq_info_id_map_index;
	iq_info_id_map_index++;

	// NOTE: Parameter initialize start
	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
	iq_info->id = id;
	iq_info->shdr_long_id = id;
	iq_info->mul_last_id = id;
	iq_info->sie_param_id = id;

	iq_info->sie_proc_mode = IQ_PROC_MOVIE;
	iq_info->sie_flow_mode = IQ_FLOW_LINEAR;
	iq_info->ipp_flow_mode = IQ_FLOW_LINEAR;

	iq_info->ir_info.ir_level = 128;
	iq_info->ir_info.saturation = 256;
	iq_info->enc_isp_ratio.enc_edge_ratio = 256;
	iq_info->enc_isp_ratio.enc_2dnr_ratio = 256;
	iq_info->enc_isp_ratio.enc_3dnr_ratio = 256;

	iq_info->ob_mode = IQ_OB_IFE;
	iq_info->dg_mode = IQ_DG_SIE;
	iq_info->cg_mode = IQ_CG_IFE;
	iq_info->ob_mode_manual.sie_enable = FALSE;
	iq_info->ob_mode_manual.ife_f_enable = FALSE;
	iq_info->ob_mode_manual.ife_enable = FALSE;
	iq_info->ob_mode_manual.sie_value = 0;
	for (i = 0; i < IQ_OB_LEN; i++) {
		iq_info->ob_mode_manual.ife_f_value[i] = 0;
		iq_info->ob_mode_manual.ife_value[i] = 0;
	}
	iq_info->dpc_en = iq_dpc_en;
	iq_info->ecs_en = iq_ecs_en;
	iq_info->_3dcc_en = iq_3dcc_en;

	memset(iq_info->hist_stcs_pre_wdr, 0, sizeof(UINT16) * IQ_HISTO_MAX_SIZE);
	memset(iq_info->hist_stcs_post_wdr, 0, sizeof(UINT16) * IQ_HISTO_MAX_SIZE);
	iq_info->dfg_airlight[0] = 1023,
	iq_info->dfg_airlight[1] = 1023,
	iq_info->dfg_airlight[2] = 1023,
	memset(iq_info->dfg_subout_min, 0, sizeof(UINT16) * IQ_SUBOUT_MAX_SIZE);
	memset(iq_info->dfg_subout_avg, 0, sizeof(UINT16) * IQ_SUBOUT_MAX_SIZE);
	memset(iq_info->dfg_dynamic_range, 0, sizeof(UINT16) * IQ_SUBOUT_MAX_SIZE);
	iq_info->dr_level = 0,
	iq_info->cfg_valid = FALSE;
	iq_info->cfg_path[0] = '\0';

	iq_info->iq_ref_set.ecs_mode = IQ_OP_TYPE_MANUAL;
	iq_info->iq_ref_set.ecs_smooth_l_m_ct_lower = 3200;
	iq_info->iq_ref_set.ecs_smooth_l_m_ct_upper = 3400;
	iq_info->iq_ref_set.ecs_smooth_m_h_ct_lower = 4800;
	iq_info->iq_ref_set.ecs_smooth_m_h_ct_upper = 5000;
	iq_info->iq_ref_set.vig_reduce_th = 400;
	iq_info->iq_ref_set.vig_zero_th = 1600;
	memset(iq_info->iq_ref_set.vig_lut, 0, sizeof(UINT16) * IQ_SHADING_VIG_LEN);

	iq_info->iq_ref_set.fixth.enable = FALSE;
	iq_info->iq_ref_set.fixth.fixy_sel = IQ_FIXTH_Y,
	iq_info->iq_ref_set.fixth.fixy_y_sel = IQ_FIXTH_Y_HIGHER,
	iq_info->iq_ref_set.fixth.fixy_edge_th = 0,
	iq_info->iq_ref_set.fixth.fixy_y_th = 0,
	iq_info->iq_ref_set.fixth.fixy_y_value = 128,
	iq_info->iq_ref_set.fixth.fixc_y_th_low = 0,
	iq_info->iq_ref_set.fixth.fixc_y_th_high = 255,
	iq_info->iq_ref_set.fixth.fixc_cb_th_low = 0,
	iq_info->iq_ref_set.fixth.fixc_cb_th_high = 255,
	iq_info->iq_ref_set.fixth.fixc_cr_th_low = 0,
	iq_info->iq_ref_set.fixth.fixc_cr_th_high = 255,
	iq_info->iq_ref_set.fixth.fixc_cb_value = 128,
	iq_info->iq_ref_set.fixth.fixc_cr_value = 128,

	iq_info->final_sie.ready_flag = FALSE;
	iq_info->final_ipp.ready_flag = FALSE;
	iq_info->final_enc.ready_flag = FALSE;

	iq_info->final_sie.sync_info.ae_status = ISP_AE_STATUS_STABLE;
	iq_info->final_sie.sync_info.gain = 100;
	iq_info->final_sie.sync_info.dgain = 128;
	iq_info->final_sie.sync_info.lv = 8000000; // LV8
	iq_info->final_sie.sync_info.lv_base = 1000000;
	iq_info->final_sie.sync_info.cgain[0] = 256;
	iq_info->final_sie.sync_info.cgain[1] = 256;
	iq_info->final_sie.sync_info.cgain[2] = 256;
	iq_info->final_sie.sync_info.ct = 5500;
	iq_info->final_sie.sync_info.shdr_ev_ratio[0] = 64;
	iq_info->final_sie.sync_info.shdr_ev_ratio[1] = 1024;
	iq_info->final_sie.sync_info.shdr_tm_ratio = 16384;
	iq_info->final_sie.sync_info.shdr_hbs_param.lum_th = 0;
	iq_info->final_sie.sync_info.shdr_hbs_param.w_start = 0;
	iq_info->final_sie.sync_info.shdr_hbs_param.w_slope = 100;

	iq_info->final_sie.sie_ob = ctl_sie_ob_init;
	iq_info->final_sie.sie_ca = ctl_sie_ca_init;
	iq_info->final_sie.sie_la = ctl_sie_la_init;
	iq_info->final_sie.sie_cgain = ctl_sie_cgain_init;
	if (iq_info->dpc_en == TRUE) {
		iq_info->final_sie.sie_dpc = (IQ_SIE_DPC *)((ULONG)iq_info + iq_info_buffer_dpc_ofs);
		iq_info->final_sie.sie_dpc->enable = TRUE;
		iq_info->final_sie.sie_dpc->mode = CTL_SIE_DP_MODE_NORMAL;
		iq_info->final_sie.sie_dpc->expand_en = FALSE;
		iq_info->final_sie.sie_dpc->table[0] = 0x0000ffff;
		iq_info->final_sie.sie_dpc->table_phyaddr = vos_cpu_get_phy_addr((ULONG)&iq_info->final_sie.sie_dpc->table[0]);
		iq_info->final_sie.sie_dpc->expand_table_viraddr = 0;
		iq_info->final_sie.sie_dpc->expand_table_phyaddr = 0;
		iq_info->final_sie.sie_dpc->weight = CTL_SIE_6F25_PERCENT;
		iq_info->final_sie.sie_dpc->dp_buffer_size = IQ_DPC_MAX_NUM * sizeof(UINT32);
		iq_info->final_sie.sie_dpc->dp_total_size = IQ_DPC_MAX_NUM * sizeof(UINT32);
		iq_info->final_sie.sie_dpc->dbg_value = 0;
	} else {
		iq_info->final_sie.sie_dpc = NULL;
	}
	if (iq_info->ecs_en == TRUE) {
		iq_info->final_sie.sie_ecs = (IQ_SIE_ECS *)((ULONG)iq_info + iq_info_buffer_ecs_ofs);
		iq_info->final_sie.sie_ecs->enable = TRUE;
		iq_info->final_sie.sie_ecs->sel_37_fmt = FALSE;
		for (i = 0; i < IQ_SHADING_ECS_LEN; i++) {
			iq_info->final_sie.sie_ecs->map_tbl[i] = 0x10040100;
		}
		iq_info->final_sie.sie_ecs->map_tbl_phyaddr = vos_cpu_get_phy_addr((ULONG)&iq_info->final_sie.sie_ecs->map_tbl[0]);
		iq_info->final_sie.sie_ecs->map_sel = CTL_SIE_ECS_MAP_65x65;
		iq_info->final_sie.sie_ecs->dthr_enable = TRUE;
		iq_info->final_sie.sie_ecs->dthr_reset = FALSE;
		iq_info->final_sie.sie_ecs->dthr_level = 0;
		iq_info->final_sie.sie_ecs->bayer_mode = CTL_SIE_ECS_3CH_10B;

		iq_info->iq_ref_set.ecs_ext = (IQ_REF_SHADING_PARAM *)((ULONG)iq_info->final_sie.sie_ecs + sizeof(IQ_SIE_ECS));
		for (i = 0; i < IQ_SHADING_ECS_LEN; i++) {
			iq_info->iq_ref_set.ecs_ext->manual_ecs_tbl[i] = 0x10040100;
		}
		for (i = 0; i < IQ_SHADING_ECS_LEN; i++) {
			iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_H][i] = 0x10040100;
		}
		for (i = 0; i < IQ_SHADING_ECS_LEN; i++) {
			iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_M][i] = 0x10040100;
		}
		for (i = 0; i < IQ_SHADING_ECS_LEN; i++) {
			iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[IQ_ECS_TEMPERATURE_L][i] = 0x10040100;
		}
	} else {
		iq_info->final_sie.sie_ecs = NULL;
		iq_info->iq_ref_set.ecs_ext = NULL;
	}
	iq_info->final_sie.sie_companding = ctl_sie_companding_init;
	iq_info->final_sie.sie_dgain = ctl_sie_dgain_init;

	iq_info->final_sie.sie_roi_ratio = ctl_sie_roi_ratio_init;

	iq_info->final_ipp.sync_info.ae_status = ISP_AE_STATUS_STABLE;
	iq_info->final_ipp.sync_info.gain = 100;
	iq_info->final_ipp.sync_info.dgain = 128;
	iq_info->final_ipp.sync_info.lv = 8000000; // LV8
	iq_info->final_ipp.sync_info.lv_base = 1000000;
	iq_info->final_ipp.sync_info.cgain[0] = 256;
	iq_info->final_ipp.sync_info.cgain[1] = 256;
	iq_info->final_ipp.sync_info.cgain[2] = 256;
	iq_info->final_ipp.sync_info.ct = 5500;
	iq_info->final_ipp.sync_info.shdr_ev_ratio[0] = 64;
	iq_info->final_ipp.sync_info.shdr_ev_ratio[1] = 1024;
	iq_info->final_ipp.sync_info.shdr_tm_ratio = 16384;
	iq_info->final_ipp.sync_info.shdr_hbs_param.lum_th = 0;
	iq_info->final_ipp.sync_info.shdr_hbs_param.w_start = 0;
	iq_info->final_ipp.sync_info.shdr_hbs_param.w_slope = 100;

	iq_info->final_ipp.ife_nrs_0 = ctl_ife_nrs_0_init;
	iq_info->final_ipp.ife_fcurve = ctl_ife_fcurve_init;
	iq_info->final_ipp.ife_fusion = ctl_ife_fusion_init;
	iq_info->final_ipp.ife_outl = ctl_ife_outl_init;
	iq_info->final_ipp.ife_filter = ctl_ife_filter_init;
	iq_info->final_ipp.ife_dgain = ctl_ife_dgain_init;
	iq_info->final_ipp.ife_cgain = ctl_ife_cgain_init;
	iq_info->final_ipp.ife_vig = ctl_ife_vig_init;
	iq_info->final_ipp.ife_gbal = ctl_ife_gbal_init;
	iq_info->final_ipp.ife_wdr = ctl_ife_wdr_init;
	iq_info->final_ipp.ife_wdr_subimg = ctl_ife_wdr_subimg_init;
	iq_info->final_ipp.ife_hist = ctl_ife_hist_init;
	iq_info->final_ipp.ife_va = ctl_ife_va_init;
	iq_info->final_ipp.ife_subisp = ctl_ife_subisp_init;
	iq_info->final_ipp.ife_fpn = ctl_ife_fpn_init;

	iq_info->final_ipp.ipe_eext = ctl_ipe_eext_init;
	iq_info->final_ipp.ipe_eext_tonemap = ctl_ipe_eext_tone_init;
	iq_info->final_ipp.ipe_edge_overshoot = ctl_ipe_edge_overshoot_init;
	iq_info->final_ipp.ipe_eproc = ctl_ipe_eproc_init;
	iq_info->final_ipp.ipe_rgblpf = ctl_ipe_rgblpf_init;
	iq_info->final_ipp.ipe_pfr = ctl_ipe_pfr_init;
	iq_info->final_ipp.ipe_cc = ctl_ipe_cc_init;
	iq_info->final_ipp.ipe_ccm = ctl_ipe_ccm_init;
	iq_info->final_ipp.ipe_cctrl = ctl_ipe_cctrl_init;
	iq_info->final_ipp.ipe_cadj_ee = ctl_ipe_cadj_ee_init;
	iq_info->final_ipp.ipe_cadj_yccon = ctl_ipe_cadj_yccon_init;
	iq_info->final_ipp.ipe_cadj_cofs = ctl_ipe_cadj_cofs_init;
	iq_info->final_ipp.ipe_cadj_rand = ctl_ipe_cadj_rand_init;
	iq_info->final_ipp.ipe_cadj_hue = ctl_ipe_cadj_hue_init;
	iq_info->final_ipp.ipe_cadj_fixth = ctl_ipe_cadj_fixth_init;
	iq_info->final_ipp.ipe_cadj_mask = ctl_ipe_cadj_mask_init;
	iq_info->final_ipp.ipe_cst = ctl_ipe_cst_init;
	iq_info->final_ipp.ipe_cstp = ctl_ipe_cstp_init;
	iq_info->final_ipp.ipe_gamyrand = ctl_ipe_gamyrand_init;
	iq_info->final_ipp.ipe_gamma = ctl_ipe_gamma_init;
	iq_info->final_ipp.ipe_ycurve = ctl_ipe_ycurve_init;
	iq_info->final_ipp.ipe_defog = ctl_ipe_defog_init;
	iq_info->final_ipp.ipe_lce = ctl_ipe_lce_init;
	iq_info->final_ipp.ipe_subimg = ctl_ipe_subimg_init;
	iq_info->final_ipp.ipe_edgedbg = ctl_ipe_edgedbg_init;
	iq_info->final_ipp.ipe_va = ctl_ipe_va_init;
	iq_info->final_ipp.ipe_edge_region_str = ctl_ipe_edge_region_str_init;
	iq_info->final_ipp.ipe_cfa = ctl_ipe_cfa_init;
	if (iq_info->_3dcc_en == TRUE) {
		iq_info->final_ipp.ipe_3dcc = (CTL_IPE_ISP_3DCC *)((ULONG)iq_info + iq_info_buffer_3dcc_ofs);
		iq_info->final_ipp.ipe_3dcc->enable = FALSE;
		memcpy(iq_info->final_ipp.ipe_3dcc->lut.rgb_3d_lut, ctl_ipe_3dcc_init.lut.rgb_3d_lut, sizeof(UINT32) * IQ_3DCC_LEN);

		iq_info->iq_ref_set._3dcc_ext = (IQ_REF_3DCC_PARAM *)((ULONG)iq_info->final_ipp.ipe_3dcc + sizeof(CTL_IPE_ISP_3DCC));
		memcpy(iq_info->iq_ref_set._3dcc_ext->manual_3dcc_lut, ctl_ipe_3dcc_init.lut.rgb_3d_lut, sizeof(UINT32) * IQ_3DCC_LEN);
		for (i = 0; i < IQ_COLOR_ID_MAX_NUM; i++) {
			memcpy(iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[i], ctl_ipe_3dcc_init.lut.rgb_3d_lut, sizeof(UINT32) * IQ_3DCC_LEN);
		}
	} else {
		iq_info->final_ipp.ipe_3dcc = NULL;
	}

	iq_info->final_ipp.ime_lca = ctl_ime_lca_init;
	iq_info->final_ipp.ime_dbcs = ctl_ime_dbcs_init;
	iq_info->final_ipp.ime_tmnr = ctl_ime_tmnr_init;
	iq_info->final_ipp.ime_ycccvt = ctl_ime_ycc_cvt_init;
	iq_info->final_ipp.ime_sharpen = ctl_ime_sharpen_init;

	iq_info->final_ipp.ife_cent_ratio = ctrl_ipp_ife_vig_cent_ratio_init;
	iq_info->final_ipp.ife_va_win_size = ctrl_ipp_ife_va_win_size_init;
	iq_info->final_ipp.ipe_va_win_size = ctrl_ipp_ipe_va_win_size_init;
	iq_info->final_ipp.ime_lca_dbg_x_pos = ctrl_ipp_ime_lca_dbg_x_pos_init;

	iq_info->final_enc.sync_info.gain = 100;
	iq_info->final_enc.sync_info.dgain = 128;
	iq_info->final_enc.sync_info.lv = 8000000; // LV8
	iq_info->final_enc.sync_info.lv_base = 1000000; // LV8
	iq_info->final_enc.sync_info.cgain[0] = 256;
	iq_info->final_enc.sync_info.cgain[1] = 256;
	iq_info->final_enc.sync_info.cgain[2] = 256;
	iq_info->final_enc.sync_info.ct = 5500;
	iq_info->final_enc.sync_info.shdr_ev_ratio[0] = 64;
	iq_info->final_enc.sync_info.shdr_ev_ratio[1] = 1024;
	iq_info->final_enc.sync_info.shdr_tm_ratio = 16384;
	iq_info->final_enc.sync_info.shdr_hbs_param.lum_th = 0;
	iq_info->final_enc.sync_info.shdr_hbs_param.w_start = 0;
	iq_info->final_enc.sync_info.shdr_hbs_param.w_slope = 100;

	iq_info->final_enc.post_sharpen = kdrv_enc_post_sharpen_init;

	iq_info->final_sie.ready_flag = TRUE;
	iq_info->final_ipp.ready_flag = TRUE;
	iq_info->final_enc.ready_flag = TRUE;
	// NOTE: Parameter initialize end

	for (i = 0; i < IQ_ID_MAX_NUM; i++) {
		residue_reset_num[i] = 3; //residue = 0, 0, 0, 1, 0, 0, 0, 1....
		companding_en[i] = FALSE;
		fcurve_en[i] = FALSE;
		cfa_en[i] = TRUE;
		tmnr_ae_still_en[i] = FALSE;
	}

	iq_sync_rt |= isp_dev_get_sync_item(id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_IQ_INIT, &iq_init_info);
	if (iq_sync_rt == E_OK) {
		iq_ui_set_info(id, IQ_UI_ITEM_NR_LV, iq_init_info.nr_lv);
		iq_ui_set_info(id, IQ_UI_ITEM_3DNR_LV, iq_init_info._3dnr_lv);
		iq_ui_set_info(id, IQ_UI_ITEM_SHARPNESS_LV, iq_init_info.sharpness_lv);
		iq_ui_set_info(id, IQ_UI_ITEM_SATURATION_LV, iq_init_info.saturation_lv);
		iq_ui_set_info(id, IQ_UI_ITEM_CONTRAST_LV, iq_init_info.contrast_lv);
		iq_ui_set_info(id, IQ_UI_ITEM_BRIGHTNESS_LV, iq_init_info.brightness_lv);
		iq_ui_set_info(id, IQ_UI_ITEM_NIGHT_MODE, iq_init_info.night_mode);
		PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "IQ(%d) get UI init (%d, %d, %d, %d, %d, %d, %d) \r\n"
			, iq_info->id, iq_init_info.nr_lv, iq_init_info._3dnr_lv, iq_init_info.sharpness_lv
			, iq_init_info.saturation_lv, iq_init_info.contrast_lv, iq_init_info.brightness_lv, iq_init_info.night_mode);

		if (iq_init_info.param_addr != 0) {
			iq_builtin_param = (IQ_PARAM_PTR *)(iq_init_info.param_addr);
			PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "Bridge iq param from builtin id = %d \r\n", id);
			memcpy(iq_param[id]->ob, iq_builtin_param->ob, sizeof(IQ_OB_PARAM));
			memcpy(iq_param[id]->nr, iq_builtin_param->nr, sizeof(IQ_NR_PARAM));
			memcpy(iq_param[id]->cfa, iq_builtin_param->cfa, sizeof(IQ_CFA_PARAM));
			memcpy(iq_param[id]->va, iq_builtin_param->va, sizeof(IQ_VA_PARAM));
			memcpy(iq_param[id]->gamma, iq_builtin_param->gamma, sizeof(IQ_GAMMA_PARAM));
			memcpy(iq_param[id]->ccm, iq_builtin_param->ccm, sizeof(IQ_CCM_PARAM));
			memcpy(iq_param[id]->color, iq_builtin_param->color, sizeof(IQ_COLOR_PARAM));
			memcpy(iq_param[id]->contrast, iq_builtin_param->contrast, sizeof(IQ_CONTRAST_PARAM));
			memcpy(iq_param[id]->edge, iq_builtin_param->edge, sizeof(IQ_EDGE_PARAM));
			memcpy(iq_param[id]->_3dnr, iq_builtin_param->_3dnr, sizeof(IQ_3DNR_PARAM));
			memcpy(iq_param[id]->pfr, iq_builtin_param->pfr, sizeof(IQ_PFR_PARAM));
			memcpy(iq_param[id]->wdr, iq_builtin_param->wdr, sizeof(IQ_WDR_PARAM));
			memcpy(iq_param[id]->defog, iq_builtin_param->defog, sizeof(IQ_DEFOG_PARAM));
			memcpy(iq_param[id]->shdr, iq_builtin_param->shdr, sizeof(IQ_SHDR_PARAM));
			memcpy(iq_param[id]->companding, iq_builtin_param->companding, sizeof(IQ_COMPANDING_PARAM));
			memcpy(iq_param[id]->rgbir, iq_builtin_param->rgbir, sizeof(IQ_RGBIR_PARAM));
			memcpy(iq_param[id]->rgbir_enh, iq_builtin_param->rgbir_enh, sizeof(IQ_RGBIR_ENH_PARAM));
			memcpy(iq_param[id]->post_sharpen_2, iq_builtin_param->post_sharpen_2, sizeof(IQ_POST_SHARPEN_2_PARAM));
			memcpy(iq_param[id]->ycurve, iq_builtin_param->ycurve, sizeof(IQ_YCURVE_PARAM));
			memcpy(iq_param[id]->cst, iq_builtin_param->cst, sizeof(IQ_CST_PARAM));
			memcpy(iq_param[id]->post_sharpen_1, iq_builtin_param->post_sharpen_1, sizeof(IQ_POST_SHARPEN_1_PARAM));
			memcpy(iq_param[id]->tone, iq_builtin_param->tone, sizeof(IQ_TONE_PARAM));
			memcpy(iq_param[id]->wdr_enh, iq_builtin_param->wdr_enh, sizeof(IQ_WDR_ENH_PARAM));
			memcpy(iq_param[id]->raw_va, iq_builtin_param->raw_va, sizeof(IQ_RAW_VA_PARAM));
			memcpy(iq_param[id]->fpn, iq_builtin_param->fpn, sizeof(IQ_FPN_PARAM));

			// Update DPC
			if (iq_info->final_sie.sie_dpc != NULL) {
				iq_info->final_sie.sie_dpc->enable = iq_builtin_param->dpc->enable;
				memcpy(iq_info->final_sie.sie_dpc->table, iq_builtin_param->dpc->table, sizeof(UINT32) * IQ_DPC_MAX_NUM);
			}

			// Update SHADING
			iq_info->final_ipp.ife_vig.enable = iq_builtin_param->shading->vig_enable;
			iq_info->final_ipp.ife_cent_ratio.ch0.x = iq_builtin_param->shading->vig_center_x;
			iq_info->final_ipp.ife_cent_ratio.ch0.y = iq_builtin_param->shading->vig_center_y;
			iq_info->final_ipp.ife_cent_ratio.ch1.x = iq_builtin_param->shading->vig_center_x;
			iq_info->final_ipp.ife_cent_ratio.ch1.y = iq_builtin_param->shading->vig_center_y;
			iq_info->final_ipp.ife_cent_ratio.ch2.x = iq_builtin_param->shading->vig_center_x;
			iq_info->final_ipp.ife_cent_ratio.ch2.y = iq_builtin_param->shading->vig_center_y;
			iq_info->final_ipp.ife_cent_ratio.ch3.x = iq_builtin_param->shading->vig_center_x;
			iq_info->final_ipp.ife_cent_ratio.ch3.y = iq_builtin_param->shading->vig_center_y;
			iq_info->iq_ref_set.ecs_mode = iq_builtin_param->shading->mode;
			iq_info->iq_ref_set.ecs_smooth_l_m_ct_lower = iq_builtin_param->shading->ecs_smooth_l_m_ct_lower;
			iq_info->iq_ref_set.ecs_smooth_l_m_ct_upper = iq_builtin_param->shading->ecs_smooth_l_m_ct_upper;
			iq_info->iq_ref_set.ecs_smooth_m_h_ct_lower = iq_builtin_param->shading->ecs_smooth_m_h_ct_lower;
			iq_info->iq_ref_set.ecs_smooth_m_h_ct_upper = iq_builtin_param->shading->ecs_smooth_m_h_ct_upper;
			iq_info->iq_ref_set.vig_reduce_th = iq_builtin_param->shading->vig_reduce_th;
			iq_info->iq_ref_set.vig_zero_th = iq_builtin_param->shading->vig_zero_th;
			memcpy(iq_info->iq_ref_set.vig_lut, iq_builtin_param->shading->vig_lut, sizeof(UINT16) * IQ_SHADING_VIG_LEN);
			if (iq_info->final_sie.sie_ecs != NULL) {
				iq_info->final_sie.sie_ecs->enable = iq_builtin_param->shading->ecs_enable;
				memcpy(iq_info->final_sie.sie_ecs->map_tbl, iq_builtin_param->shading->ecs_map_tbl, sizeof(UINT32) * IQ_SHADING_ECS_LEN);
			}

			if (iq_info->iq_ref_set.ecs_ext != NULL) {
				memcpy(iq_info->iq_ref_set.ecs_ext->manual_ecs_tbl, iq_builtin_param->shading->ecs_map_tbl, sizeof(UINT32) * IQ_SHADING_ECS_LEN);
				memcpy(iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl, iq_builtin_param->shading_ext->ecs_map_tbl, sizeof(UINT32) * IQ_ECS_ID_MAX_NUM * IQ_SHADING_ECS_LEN);
			}

			// Update 3DCC
			if (iq_info->final_ipp.ipe_3dcc != NULL) {
				iq_info->final_ipp.ipe_3dcc->enable = iq_builtin_param->_3dcc->enable;
				memcpy(iq_info->final_ipp.ipe_3dcc->lut.rgb_3d_lut, iq_builtin_param->_3dcc->manual_3dcc_lut, sizeof(UINT32) * IQ_3DCC_LEN);
			}

			if (iq_info->iq_ref_set._3dcc_ext != NULL) {
				memcpy(iq_info->iq_ref_set._3dcc_ext->manual_3dcc_lut, iq_builtin_param->_3dcc->manual_3dcc_lut, sizeof(UINT32) * IQ_3DCC_LEN);
				memcpy(iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut, iq_builtin_param->_3dcc_ext->auto_3dcc_lut, sizeof(UINT32) * IQ_COLOR_ID_MAX_NUM * IQ_3DCC_LEN);
			}
		} else {
			DBG_DUMP("No bridge iq param from builtin \r\n");
		}
	} else {
		PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "IPP(%d) trig - IQ init not update!! \r\n", iq_info->id);
	}

	return rt;
}

INT32 iq_flow_uninit(UINT32 id)
{
	if (iq_param_memalloc_valid[id] == TRUE) {
		isp_uti_vmem_free(iq_param_memalloc_addr[id].ob);
		iq_param_memalloc_addr[id].ob = NULL;
		iq_param_memalloc_valid[id] = FALSE;
	}

	iq_info_id_map_index = 0;

	return E_OK;
}

static void iq_flow_set_update(ISP_TRIG_MSG msg, IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param)
{
	BOOL update_all = FALSE;
	BOOL update_fixth = FALSE;
	UINT32 ui_image_effect = iq_ui_get_info(iq_info->id, IQ_UI_ITEM_IMAGEEFFECT);
	UINT32 ui_tone_lv = iq_ui_get_info(iq_info->id, IQ_UI_ITEM_TONE_LV);
	UINT32 ui_gamma_lv = iq_ui_get_info(iq_info->id, IQ_UI_ITEM_GAMMA_LV);
	UINT32 dbg_mode = iq_dbg_get_dbg_mode(iq_info->id);
	static UINT32 dpc_update_mask[IQ_ID_MAX_NUM] = {0}, ecs_update_mask[IQ_ID_MAX_NUM] = {0};
	static UINT32 pre_lv[IQ_ID_MAX_NUM] = {0};
	static UINT32 pre_ui_image_effect[IQ_ID_MAX_NUM] = {0};
	static UINT32 pre_ui_tone_lv[IQ_ID_MAX_NUM] = {0};
	static UINT32 pre_ui_gamma_lv[IQ_ID_MAX_NUM] = {0};

	switch ((UINT32)msg) {
	case ISP_TRIG_IQ_SIE:
		if (iq_info->sie_trig_obj.reset) {
			update_all = TRUE;
			iq_info->sie_tab_update |= (IQ_TBL_SIE_DPC | IQ_TBL_SIE_ECS);
			PRINT_IQ(dbg_mode & IQ_DBG_TABLE, "IQ(%d) DPC, ECS updated from reset!! \r\n", iq_info->id);
		}

		//Update from IQ setting
		if ((iq_info->ecs_en == TRUE) && (iq_info->iq_ref_set.ecs_mode == IQ_OP_TYPE_AUTO)) {
			iq_info->sie_tab_update |= IQ_TBL_SIE_ECS;
			PRINT_IQ(dbg_mode & IQ_DBG_TABLE, "IQ(%d) ECS updated from AWB!! \r\n", iq_info->id);
		}

		// Update from Tool
		// IQ_DPC_PARAM setting
		if ((iq_info->dpc_en == TRUE) && (update_all || (iqt_get_param_update(iq_info->sie_param_id, IQT_ITEM_DPC_PARAM) == TRUE))) {
			iq_info->sie_tab_update |= IQ_TBL_SIE_DPC;
			if (iqt_get_param_update(iq_info->sie_param_id, IQT_ITEM_DPC_PARAM) == TRUE) {
				PRINT_IQ(dbg_mode & IQ_DBG_TABLE, "IQ(%d) DPC updated from IQT!! \r\n", iq_info->id);
			}
			// SHDR multi frame parameter update
			dpc_update_mask[iq_info->sie_param_id] |= (1 << iq_info->id);
			if (dpc_update_mask[iq_info->sie_param_id] == iq_info->sie_trig_obj.src_id_mask) {
				iqt_reset_param_update(iq_info->sie_param_id, IQT_ITEM_DPC_PARAM);
				dpc_update_mask[iq_info->sie_param_id] = 0;
			}
		}

		// IQ_SHADING_PARAM setting
		if ((iq_info->ecs_en == TRUE) && (update_all || (iqt_get_param_update(iq_info->sie_param_id, IQT_ITEM_SHADING_PARAM) == TRUE))) {
			iq_info->sie_tab_update |= IQ_TBL_SIE_ECS;
			if (iqt_get_param_update(iq_info->sie_param_id, IQT_ITEM_SHADING_PARAM) == TRUE) {
				PRINT_IQ(dbg_mode & IQ_DBG_TABLE, "IQ(%d) ECS updated from IQT!! \r\n", iq_info->id);
			}
			// SHDR multi frame parameter update
			ecs_update_mask[iq_info->sie_param_id] |= (1 << iq_info->id);
			if (ecs_update_mask[iq_info->sie_param_id] == iq_info->sie_trig_obj.src_id_mask) {
				iqt_reset_param_update(iq_info->sie_param_id, IQT_ITEM_SHADING_PARAM);
				ecs_update_mask[iq_info->sie_param_id] = 0;
			}
		}

		// IQ_COMPANDING_PARAM setting
		if (update_all || (iqt_get_param_update(iq_info->sie_param_id, IQT_ITEM_COMPANDING_PARAM) == TRUE)) {
			memcpy(iq_info->final_sie.sie_companding.decomp_info.decomp_kpx, iq_param->companding->decomp_kpx, sizeof(UINT32) * IQ_DECOMPANDING_KNEE_NUM);
			memcpy(iq_info->final_sie.sie_companding.decomp_info.decomp_kpy, iq_param->companding->decomp_kpy, sizeof(UINT32) * IQ_DECOMPANDING_KNEE_NUM);
			memcpy(iq_info->final_sie.sie_companding.decomp_info.decomp_gain, iq_param->companding->decomp_gain, sizeof(UINT32) * IQ_DECOMPANDING_KNEE_NUM);
			memcpy(iq_info->final_sie.sie_companding.decomp_info.decomp_sb, iq_param->companding->decomp_sb, sizeof(UINT32) * IQ_DECOMPANDING_KNEE_NUM);
			memcpy(iq_info->final_sie.sie_companding.comp_info.comp_fcurve_l, iq_param->companding->comp_fcurve_l, sizeof(UINT32) * IQ_COMPANDING_LEFT_NUM);
			memcpy(iq_info->final_sie.sie_companding.comp_info.comp_fcurve_m, iq_param->companding->comp_fcurve_m, sizeof(UINT32) * IQ_COMPANDING_MIDDLE_NUM);
			memcpy(iq_info->final_sie.sie_companding.comp_info.comp_fcurve_r, iq_param->companding->comp_fcurve_r, sizeof(UINT32) * IQ_COMPANDING_RIGHT_NUM);
			iq_info->final_sie.sie_companding.comp_info.comp_fcurve_ev_fmt = iq_param->companding->comp_fcurve_ev_fmt;
			iqt_reset_param_update(iq_info->sie_param_id, IQT_ITEM_COMPANDING_PARAM);
		}
		break;

	case ISP_TRIG_IQ_IPP:
		if ((iq_info->ipp_trig_obj.reset) || (iq_info->ipp_trig_obj.ipp_capture == TRUE)) {
			update_all = TRUE;
			iq_info->ipp_tab_update |= (IQ_TBL_IPE_GAMMA | IQ_TBL_IPE_YCURVE);
			PRINT_IQ(dbg_mode & IQ_DBG_GAMMA, "IQ(%d) GAMMA updated from reset!! \r\n", iq_info->id);
			PRINT_IQ(dbg_mode & IQ_DBG_TABLE, "IQ(%d) YCURVE updated from reset!! \r\n", iq_info->id);
		}

		//Update from IQ setting
		if (iq_info->final_ipp.sync_info.lv != pre_lv[iq_info->id]) {
			iq_info->ipp_tab_update |= IQ_TBL_IPE_GAMMA;
			PRINT_IQ(dbg_mode & IQ_DBG_GAMMA, "IQ(%d) GAMMA updated from AE at lv(%d)!! \r\n", iq_info->id, iq_info->final_ipp.sync_info.lv);
		}
		if (pre_ui_image_effect[iq_info->id] != ui_image_effect) {
			iq_info->ipp_tab_update |= IQ_TBL_IPE_GAMMA;
			PRINT_IQ(dbg_mode & IQ_DBG_GAMMA, "IQ(%d) GAMMA updated from Effect!! \r\n", iq_info->id);
			iq_info->ipp_tab_update |= IQ_TBL_IPE_YCURVE;
			PRINT_IQ(dbg_mode & IQ_DBG_TABLE, "IQ(%d) YCURVE updated from Effect!! \r\n", iq_info->id);
			update_fixth = TRUE;
		}
		if ((pre_ui_tone_lv[iq_info->id] != ui_tone_lv) || (pre_ui_gamma_lv[iq_info->id] != ui_gamma_lv)) {
			iq_info->ipp_tab_update |= IQ_TBL_IPE_GAMMA;
			PRINT_IQ(dbg_mode & IQ_DBG_GAMMA, "IQ(%d) GAMMA updated from UI level!! \r\n", iq_info->id);
		}
		pre_lv[iq_info->id] = iq_info->final_ipp.sync_info.lv;
		pre_ui_image_effect[iq_info->id] = ui_image_effect;
		pre_ui_tone_lv[iq_info->id] = ui_tone_lv;
		pre_ui_gamma_lv[iq_info->id] = ui_gamma_lv;

		// IQ_NR_PARAM setting
		if (update_all || (iqt_get_param_update(iq_info->id, IQT_ITEM_NR_PARAM) == TRUE)) {
			iq_info->final_ipp.ife_outl.enable = iq_param->nr->outl_enable;
			iq_info->final_ipp.ife_gbal.enable = iq_param->nr->gbal_enable;
			iq_info->final_ipp.ife_filter.enable = iq_param->nr->filter_enable;
			iq_info->final_ipp.ime_dbcs.enable = iq_param->nr->dbcs_enable;
			iqt_reset_param_update(iq_info->id, IQT_ITEM_NR_PARAM);
		}

		// IQ_RAW_VA_PARAM setting
		if (update_all || (iqt_get_param_update(iq_info->id, IQT_ITEM_RAW_VA_PARAM) == TRUE)) {
			iq_info->final_ipp.ife_va.fltr_sel.vdetgh1_filter_sel = iq_param->raw_va->g1_h_filter_sel;
			if (iq_param->raw_va->g1_h_filter_sel == IQ_VA_H_FILTER_FIR) {
				iq_info->final_ipp.ife_va.group_1.h_filt.symmetry = iq_param->raw_va->g1_fir_sym_sel;
				iq_info->final_ipp.ife_va.group_1.h_filt.tap_a = iq_param->raw_va->g1_fir_tap_a;
				iq_info->final_ipp.ife_va.group_1.h_filt.tap_b = iq_param->raw_va->g1_fir_tap_b;
				iq_info->final_ipp.ife_va.group_1.h_filt.tap_c = iq_param->raw_va->g1_fir_tap_c;
				iq_info->final_ipp.ife_va.group_1.h_filt.tap_d = iq_param->raw_va->g1_fir_tap_d;
				iq_info->final_ipp.ife_va.group_1.h_filt.div = iq_param->raw_va->g1_fir_div;
			} else {
				iq_info->final_ipp.ife_va.group_1.h_filt.symmetry = CTL_IFE_ISP_VA_FILTER_SYM_INVERSE;
				// @ iq_operation_iso
			}
			iq_info->final_ipp.ife_va.group_1.v_filt.symmetry = iq_param->raw_va->g1_fir_sym_sel;
			iq_info->final_ipp.ife_va.group_1.v_filt.tap_a = iq_param->raw_va->g1_fir_tap_a;
			iq_info->final_ipp.ife_va.group_1.v_filt.tap_b = iq_param->raw_va->g1_fir_tap_b;
			iq_info->final_ipp.ife_va.group_1.v_filt.tap_c = iq_param->raw_va->g1_fir_tap_c;
			iq_info->final_ipp.ife_va.group_1.v_filt.tap_d = iq_param->raw_va->g1_fir_tap_d;
			iq_info->final_ipp.ife_va.group_1.v_filt.div = iq_param->raw_va->g1_fir_div;
			iq_info->final_ipp.ife_va.fltr_sel.vdetgh2_filter_sel = iq_param->raw_va->g2_h_filter_sel;
			if (iq_param->raw_va->g2_h_filter_sel == IQ_VA_H_FILTER_FIR) {
				iq_info->final_ipp.ife_va.group_2.h_filt.symmetry = iq_param->raw_va->g2_fir_sym_sel;
				iq_info->final_ipp.ife_va.group_2.h_filt.tap_a = iq_param->raw_va->g2_fir_tap_a;
				iq_info->final_ipp.ife_va.group_2.h_filt.tap_b = iq_param->raw_va->g2_fir_tap_b;
				iq_info->final_ipp.ife_va.group_2.h_filt.tap_c = iq_param->raw_va->g2_fir_tap_c;
				iq_info->final_ipp.ife_va.group_2.h_filt.tap_d = iq_param->raw_va->g2_fir_tap_d;
				iq_info->final_ipp.ife_va.group_2.h_filt.div = iq_param->raw_va->g2_fir_div;
			} else {
				iq_info->final_ipp.ife_va.group_2.h_filt.symmetry = CTL_IFE_ISP_VA_FILTER_SYM_INVERSE;
				// @ iq_operation_iso
			}
			iq_info->final_ipp.ife_va.group_2.v_filt.symmetry = iq_param->raw_va->g2_fir_sym_sel;
			iq_info->final_ipp.ife_va.group_2.v_filt.tap_a = iq_param->raw_va->g2_fir_tap_a;
			iq_info->final_ipp.ife_va.group_2.v_filt.tap_b = iq_param->raw_va->g2_fir_tap_b;
			iq_info->final_ipp.ife_va.group_2.v_filt.tap_c = iq_param->raw_va->g2_fir_tap_c;
			iq_info->final_ipp.ife_va.group_2.v_filt.tap_d = iq_param->raw_va->g2_fir_tap_d;
			iq_info->final_ipp.ife_va.group_2.v_filt.div = iq_param->raw_va->g2_fir_div;
			iq_info->final_ipp.ife_va.ldg_para.ldg_low_th = iq_param->raw_va->ldg_low_th;
			iq_info->final_ipp.ife_va.ldg_para.ldg_high_th = iq_param->raw_va->ldg_high_th;
			iq_info->final_ipp.ife_va.ldg_para.ldg_low_gain = iq_param->raw_va->ldg_low_gain;
			iq_info->final_ipp.ife_va.ldg_para.ldg_high_gain = iq_param->raw_va->ldg_high_gain;
			iq_info->final_ipp.ife_va.ldg_para.ldg_low_slope = iq_param->raw_va->ldg_low_slope;
			iq_info->final_ipp.ife_va.ldg_para.ldg_high_slope = iq_param->raw_va->ldg_high_slope;
			iq_info->final_ipp.ife_va.energy_w = iq_param->raw_va->energy_w;
			iq_info->final_ipp.ife_va.win_cnt_out_sel = iq_param->raw_va->win_cnt_out_sel;
			iq_info->final_ipp.ife_va.high_luma_th = iq_param->raw_va->high_luma_th;
			iqt_reset_param_update(iq_info->id, IQT_ITEM_RAW_VA_PARAM);
		}

		// IQ_VA_PARAM setting
		if (update_all || (iqt_get_param_update(iq_info->id, IQT_ITEM_VA_PARAM) == TRUE)) {
			if (iq_param->va->pre_filter_enable == TRUE) {
				iq_info->final_ipp.ipe_va.pre_filter_mode = CTL_IPE_ISP_PRE_FILTER_3x3_2;
			} else {
				iq_info->final_ipp.ipe_va.pre_filter_mode = CTL_IPE_ISP_NO_PRE_FILTER;
			}
			iq_info->final_ipp.ipe_va.group_1.h_filt.tap_a = iq_param->va->g1_tap_a;
			iq_info->final_ipp.ipe_va.group_1.h_filt.tap_b = iq_param->va->g1_tap_b;
			iq_info->final_ipp.ipe_va.group_1.h_filt.tap_c = iq_param->va->g1_tap_c;
			iq_info->final_ipp.ipe_va.group_1.h_filt.tap_d = iq_param->va->g1_tap_d;
			iq_info->final_ipp.ipe_va.group_1.h_filt.div = iq_param->va->g1_div;
			iq_info->final_ipp.ipe_va.group_1.v_filt.tap_a = iq_param->va->g1_tap_a;
			iq_info->final_ipp.ipe_va.group_1.v_filt.tap_b = iq_param->va->g1_tap_b;
			iq_info->final_ipp.ipe_va.group_1.v_filt.tap_c = iq_param->va->g1_tap_c;
			iq_info->final_ipp.ipe_va.group_1.v_filt.tap_d = iq_param->va->g1_tap_d;
			iq_info->final_ipp.ipe_va.group_1.v_filt.div = iq_param->va->g1_div;
			iq_info->final_ipp.ipe_va.group_2.h_filt.tap_a = iq_param->va->g2_tap_a;
			iq_info->final_ipp.ipe_va.group_2.h_filt.tap_b = iq_param->va->g2_tap_b;
			iq_info->final_ipp.ipe_va.group_2.h_filt.tap_c = iq_param->va->g2_tap_c;
			iq_info->final_ipp.ipe_va.group_2.h_filt.tap_d = iq_param->va->g2_tap_d;
			iq_info->final_ipp.ipe_va.group_2.h_filt.div = iq_param->va->g2_div;
			iq_info->final_ipp.ipe_va.group_2.v_filt.tap_a = iq_param->va->g2_tap_a;
			iq_info->final_ipp.ipe_va.group_2.v_filt.tap_b = iq_param->va->g2_tap_b;
			iq_info->final_ipp.ipe_va.group_2.v_filt.tap_c = iq_param->va->g2_tap_c;
			iq_info->final_ipp.ipe_va.group_2.v_filt.tap_d = iq_param->va->g2_tap_d;
			iq_info->final_ipp.ipe_va.group_2.v_filt.div = iq_param->va->g2_div;
			iq_info->final_ipp.ipe_va.ldg_para.ldg_low_th = iq_param->va->ldg_low_th;
			iq_info->final_ipp.ipe_va.ldg_para.ldg_high_th = iq_param->va->ldg_high_th;
			iq_info->final_ipp.ipe_va.ldg_para.ldg_low_gain = iq_param->va->ldg_low_gain;
			iq_info->final_ipp.ipe_va.ldg_para.ldg_high_gain = iq_param->va->ldg_high_gain;
			iq_info->final_ipp.ipe_va.ldg_para.ldg_low_slope = iq_param->va->ldg_low_slope;
			iq_info->final_ipp.ipe_va.ldg_para.ldg_high_slope = iq_param->va->ldg_high_slope;
			iq_info->final_ipp.ipe_va.energy_w = iq_param->va->energy_w;
			iq_info->final_ipp.ipe_va.win_cnt_out_sel = iq_param->va->win_cnt_out_sel;
			iq_info->final_ipp.ipe_va.high_luma_th = iq_param->va->high_luma_th;
			iqt_reset_param_update(iq_info->id, IQT_ITEM_VA_PARAM);
		}

		// IQ_TONE_PARAM setting
		if (update_all || (iqt_get_param_update(iq_info->id, IQT_ITEM_TONE_PARAM) == TRUE)) {
			iq_info->final_ipp.ife_wdr.tonecurve_enable = iq_param->tone->enable;
			memcpy(iq_info->final_ipp.ife_wdr.input_bld.in_yv_blend_lut, iq_param->tone->tone_in_yv_blend_lut, sizeof(UINT8) * CTL_IFE_ISP_WDR_INPUT_YV_BLD_NUM);
			iq_info->ipp_tab_update |= IQ_TBL_IPE_GAMMA;
			iqt_reset_param_update(iq_info->id, IQT_ITEM_TONE_PARAM);
		}

		// IQ_GAMMA_PARAM setting
		if (update_all || (iqt_get_param_update(iq_info->id, IQT_ITEM_GAMMA_PARAM) == TRUE)) {
			iq_info->final_ipp.ipe_gamma.enable = iq_param->gamma->enable;
			iq_info->ipp_tab_update |= IQ_TBL_IPE_GAMMA;
			if (iqt_get_param_update(iq_info->id, IQT_ITEM_GAMMA_PARAM) == TRUE) {
				PRINT_IQ(dbg_mode & IQ_DBG_GAMMA, "IQ(%d) GAMMA updated from IQT\r\n", iq_info->id);
			}
			iqt_reset_param_update(iq_info->id, IQT_ITEM_GAMMA_PARAM);
		}

		// IQ_EDGE_PARAM setting
		if (update_all || (iqt_get_param_update(iq_info->id, IQT_ITEM_EDGE_PARAM) == TRUE)) {
			iq_info->final_ipp.ipe_cadj_ee.enable = iq_param->edge->enable;
			if (iq_info->final_ipp.ipe_cadj_ee.enable) {
				iq_info->final_ipp.ipe_edge_overshoot.th_overshoot = iq_param->edge->th_overshoot;
				iq_info->final_ipp.ipe_edge_overshoot.th_undershoot = iq_param->edge->th_undershoot;
				// eext blending settings is calculated at iq_operation_gamma()
				#if 1
				// NOTE: Check code independent
				iq_operation_gamma(iq_info, iq_param, TRUE);
				#else
				iq_info->final_ipp.ipe_eext.eext_blending.eext_blending_th = iq_param->edge->blending_th;
				iq_info->final_ipp.ipe_eext.eext_blending.eext_blending_w1 = iq_param->edge->blending_low_luma_w;
				iq_info->final_ipp.ipe_eext.eext_blending.eext_blending_w2 = iq_param->edge->blending_high_luma_w;
				#endif
				memcpy(iq_info->final_ipp.ipe_eproc.edge_map_lut, iq_param->edge->edge_map_lut, sizeof(UINT8) * CTL_IPE_ISP_EDGE_MAP_LUT_LEN);
				memcpy(iq_info->final_ipp.ipe_eproc.es_map_lut, iq_param->edge->es_map_lut, sizeof(UINT8) * CTL_IPE_ISP_ES_MAP_LUT_LEN);
				memcpy(iq_info->final_ipp.ipe_cctrl.edge_tab, iq_param->edge->edge_tab, sizeof(UINT8) * CTL_IPE_ISP_CCTRL_TAB_LEN);
			}
			iqt_reset_param_update(iq_info->id, IQT_ITEM_EDGE_PARAM);
		}

		// IQ_CONTRAST_PARAM setting
		if (update_all || (iqt_get_param_update(iq_info->id, IQT_ITEM_CONTRAST_PARAM) == TRUE)) {
			iq_info->final_ipp.ipe_lce.enable = iq_param->contrast->lce_enable;
			iqt_reset_param_update(iq_info->id, IQT_ITEM_CONTRAST_PARAM);
		}

		// IQ_PFR_PARAM setting
		if (update_all || iqt_get_param_update(iq_info->id, IQT_ITEM_PFR_PARAM) == TRUE) {
			iq_info->final_ipp.ipe_pfr.enable = iq_param->pfr->enable;
			if (iq_info->final_ipp.ipe_pfr.enable) {
				memcpy(iq_info->final_ipp.ipe_pfr.luma_lut, iq_param->pfr->luma_lut, sizeof(UINT8) * CTL_IPE_ISP_PFR_LUMA_LEN);
				iq_info->final_ipp.ipe_pfr.color_wet_set[0].enable = iq_param->pfr->set0_en;
				iq_info->final_ipp.ipe_pfr.color_wet_set[0].color_u = iq_param->pfr->set0_color_u;
				iq_info->final_ipp.ipe_pfr.color_wet_set[0].color_v = iq_param->pfr->set0_color_v;
				iq_info->final_ipp.ipe_pfr.color_wet_set[1].enable = iq_param->pfr->set1_en;
				iq_info->final_ipp.ipe_pfr.color_wet_set[1].color_u = iq_param->pfr->set1_color_u;
				iq_info->final_ipp.ipe_pfr.color_wet_set[1].color_v = iq_param->pfr->set1_color_v;
				iq_info->final_ipp.ipe_pfr.color_wet_set[2].enable = iq_param->pfr->set2_en;
				iq_info->final_ipp.ipe_pfr.color_wet_set[2].color_u = iq_param->pfr->set2_color_u;
				iq_info->final_ipp.ipe_pfr.color_wet_set[2].color_v = iq_param->pfr->set2_color_v;
				iq_info->final_ipp.ipe_pfr.color_wet_set[3].enable = iq_param->pfr->set3_en;
				iq_info->final_ipp.ipe_pfr.color_wet_set[3].color_u = iq_param->pfr->set3_color_u;
				iq_info->final_ipp.ipe_pfr.color_wet_set[3].color_v = iq_param->pfr->set3_color_v;
				if ((iq_param->pfr->set0_en == FALSE) && (iq_param->pfr->set1_en == FALSE) && (iq_param->pfr->set2_en == FALSE) && (iq_param->pfr->set3_en == FALSE)) {
					iq_info->final_ipp.ipe_pfr.enable = FALSE;
				}
			}
			iqt_reset_param_update(iq_info->id, IQT_ITEM_PFR_PARAM);
		}

		// IQ_WDR_PARAM setting
		if (update_all || (iqt_get_param_update(iq_info->id, IQT_ITEM_WDR_PARAM) == TRUE)) {
			iq_info->final_ipp.ife_wdr.wdr_enable = iq_param->wdr->enable;
			if (iq_info->final_ipp.ife_wdr.wdr_enable) {
				iq_info->final_ipp.ife_wdr_subimg.subimg_size_h = IQ_CLAMP(iq_param->wdr->subimg_size_h, 8, 48);
				iq_info->final_ipp.ife_wdr_subimg.subimg_size_v = IQ_CLAMP(iq_param->wdr->subimg_size_v, 8, 48);
				iq_info->final_ipp.ife_wdr.gainctrl.max_gain = IQ_CLAMP(iq_param->wdr->max_gain, 1, 255);
				iq_param->wdr->min_gain = IQ_CLAMP(iq_param->wdr->min_gain, 1, 255);
				iq_info->final_ipp.ife_wdr.gainctrl.min_gain = 255 / iq_param->wdr->min_gain;
				iq_info->final_ipp.ife_wdr.wdr_str.wdr_halo_ratio = iq_param->wdr->halo_ratio;
				iq_info->final_ipp.ife_wdr.wdr_str.wdr_halo_slope = iq_param->wdr->halo_slope;
				iq_info->final_ipp.ife_wdr.fbc.fbc_ratio = iq_param->wdr->fbc_ratio;
				iq_info->final_ipp.ife_wdr.wdr_gain_prot_str = iq_param->wdr->gain_protect_str;
				memcpy(iq_info->final_ipp.ife_wdr.outbld.outbld_lut.left_table, iq_param->wdr->lut_left, sizeof(UINT16) * CTL_IFE_ISP_WDR_NEQ_TABLE_L_NUM);
				memcpy(iq_info->final_ipp.ife_wdr.outbld.outbld_lut.right_table, iq_param->wdr->lut_right, sizeof(UINT16) * CTL_IFE_ISP_WDR_NEQ_TABLE_R_NUM);
			}
			iqt_reset_param_update(iq_info->id, IQT_ITEM_WDR_PARAM);
		}

		// IQ_DEFOG_PARAM setting
		if (update_all || (iqt_get_param_update(iq_info->id, IQT_ITEM_DEFOG_PARAM) == TRUE)) {
			iq_info->final_ipp.ipe_defog.enable = iq_param->defog->enable;
			if (1) { // for NNSC defog auto switch
				iq_info->final_ipp.ipe_defog.dfg_outbld.outbld_local_en = iq_param->defog->outbld_local_en;
				iq_param->defog->outbld_diff_wt = IQ_CLAMP(iq_param->defog->outbld_diff_wt, 0, 7);
				memcpy(iq_info->final_ipp.ipe_defog.dfg_outbld.outbld_diff_wt, iq_defog_diff_wt[iq_param->defog->outbld_diff_wt], sizeof(UINT8) * CTL_IPE_ISP_DFG_OUTPUT_BLD_LEN);
			}
			iqt_reset_param_update(iq_info->id, IQT_ITEM_DEFOG_PARAM);
		}

		// IQ_SHDR_PARAM setting
		if (update_all || (iqt_get_param_update(iq_info->id, IQT_ITEM_SHDR_PARAM) == TRUE)) {
			iq_info->final_ipp.ife_nrs_0.enable = iq_param->shdr->nrs_enable;
			iq_info->final_ipp.ife_fusion.bld_cur.nor_sel = iq_param->shdr->fusion_nor_sel;
			iq_info->final_ipp.ife_fusion.bld_cur.l_nor_knee[0] = iq_param->shdr->fusion_l_nor_knee;
			iq_info->final_ipp.ife_fusion.bld_cur.l_nor_range = iq_param->shdr->fusion_l_nor_range;
			iq_info->final_ipp.ife_fusion.bld_cur.l_nor_slope = IQ_CLAMP(256 * 4096 / iq_param->shdr->fusion_l_nor_range, 0, 65535);
			// blend curve settings for S is calculated at iq_operation_shdr_fusion()
			iq_info->final_ipp.ife_fusion.bld_cur.dif_sel = iq_param->shdr->fusion_dif_sel;
			iq_info->final_ipp.ife_fusion.bld_cur.l_dif_knee[0] = iq_param->shdr->fusion_l_dif_knee;
			iq_info->final_ipp.ife_fusion.bld_cur.l_dif_range = iq_param->shdr->fusion_l_dif_range;
			iq_info->final_ipp.ife_fusion.bld_cur.l_dif_slope = IQ_CLAMP(256 * 4096 / iq_param->shdr->fusion_l_dif_range, 0, 65535);
			// blend curve settings for S is calculated at iq_operation_shdr_fusion()
			iq_info->final_ipp.ife_fusion.mc_para.lum_th = iq_param->shdr->fusion_lum_th;
			memcpy(iq_info->final_ipp.ife_fusion.mc_para.diff_w, iq_param->shdr->fusion_diff_w, sizeof(UINT8) * IQ_SHDR_FUSION_DIFF_W_NUM);
			iq_info->final_ipp.ife_fcurve.fcur_ctrl.y_mean_sel = iq_param->shdr->fcurve_y_mean_sel;
			iq_info->final_ipp.ife_fcurve.fcur_ctrl.yv_w = iq_param->shdr->fcurve_yv_w;
			memcpy(iq_info->final_ipp.ife_fcurve.y_weight.y_w_lut, iq_param->shdr->fcurve_y_w_lut, sizeof(UINT8) * IQ_SHDR_FCURVE_Y_W_NUM);
			if (iqt_get_param_update(iq_info->id, IQT_ITEM_SHDR_PARAM) == TRUE) {
				is_update_shdr_param[iq_info->id] = TRUE;
			}
			iqt_reset_param_update(iq_info->id, IQT_ITEM_SHDR_PARAM);
		}

		// IQ_YCURVE_PARAM setting
		if (update_all || (iqt_get_param_update(iq_info->id, IQT_ITEM_YCURVE_PARAM) == TRUE)) {
			iq_info->final_ipp.ipe_ycurve.enable = iq_param->ycurve->enable;
			memcpy(iq_info->final_ipp.ipe_ycurve.lut.y_curve_lut, iq_param->ycurve->ycurve_lut, sizeof(UINT32) * CTL_IPE_ISP_YCURVE_LEN);
			iq_info->ipp_tab_update |= IQ_TBL_IPE_YCURVE;
			if (iqt_get_param_update(iq_info->id, IQT_ITEM_YCURVE_PARAM) == TRUE) {
				PRINT_IQ(dbg_mode & IQ_DBG_TABLE, "IQ(%d) YCURVE updated from IQT\r\n", iq_info->id);
			}
			iqt_reset_param_update(iq_info->id, IQT_ITEM_YCURVE_PARAM);
		}

		// IQ_FIXTH_PARAM setting
		if (update_all || update_fixth || (iqt_get_param_update(iq_info->id, IQT_ITEM_FIXTH_PARAM) == TRUE)) {
			if (iq_info->iq_ref_set.fixth.enable == FALSE) {
				iq_info->final_ipp.ipe_cadj_fixth = ctl_ipe_cadj_fixth_init;
			} else {
				iq_info->final_ipp.ipe_cadj_fixth.enable = TRUE;
				// Y1 judge : ((e > edge_th) && (y > y_th))
				// Y2 judge : ((y > y_th))
				if (iq_info->iq_ref_set.fixth.fixy_sel == IQ_FIXTH_EDGE) {
					iq_info->final_ipp.ipe_cadj_fixth.yth1.y_th = 0;
					iq_info->final_ipp.ipe_cadj_fixth.yth1.edge_th = iq_info->iq_ref_set.fixth.fixy_edge_th;
					iq_info->final_ipp.ipe_cadj_fixth.yth1.ycth_sel_hit = CTL_IPE_ISP_YCTH_REGISTER_VALUE;
					iq_info->final_ipp.ipe_cadj_fixth.yth1.ycth_sel_nonhit = CTL_IPE_ISP_YCTH_ORIGINAL_VALUE;
					iq_info->final_ipp.ipe_cadj_fixth.yth1.value_hit = iq_info->iq_ref_set.fixth.fixy_y_value;
					iq_info->final_ipp.ipe_cadj_fixth.yth1.nonvalue_hit = 0;
					iq_info->final_ipp.ipe_cadj_fixth.yth2.y_th = 0;
					iq_info->final_ipp.ipe_cadj_fixth.yth2.ycth_sel_hit = CTL_IPE_ISP_YCTH_ORIGINAL_VALUE;
					iq_info->final_ipp.ipe_cadj_fixth.yth2.ycth_sel_nonhit = CTL_IPE_ISP_YCTH_ORIGINAL_VALUE;
					iq_info->final_ipp.ipe_cadj_fixth.yth2.value_hit = 0;
					iq_info->final_ipp.ipe_cadj_fixth.yth2.nonvalue_hit = 0;
				} else {
					iq_info->final_ipp.ipe_cadj_fixth.yth1.y_th = 0;
					iq_info->final_ipp.ipe_cadj_fixth.yth1.edge_th = 0;
					iq_info->final_ipp.ipe_cadj_fixth.yth1.ycth_sel_hit = CTL_IPE_ISP_YCTH_ORIGINAL_VALUE;
					iq_info->final_ipp.ipe_cadj_fixth.yth1.ycth_sel_nonhit = CTL_IPE_ISP_YCTH_ORIGINAL_VALUE;
					iq_info->final_ipp.ipe_cadj_fixth.yth1.value_hit = 0;
					iq_info->final_ipp.ipe_cadj_fixth.yth1.nonvalue_hit = 0;
					iq_info->final_ipp.ipe_cadj_fixth.yth2.y_th = iq_info->iq_ref_set.fixth.fixy_y_th;
					if (iq_info->iq_ref_set.fixth.fixy_y_sel == IQ_FIXTH_Y_HIGHER) {
						iq_info->final_ipp.ipe_cadj_fixth.yth2.ycth_sel_hit = CTL_IPE_ISP_YCTH_REGISTER_VALUE;
						iq_info->final_ipp.ipe_cadj_fixth.yth2.ycth_sel_nonhit = CTL_IPE_ISP_YCTH_ORIGINAL_VALUE;
						iq_info->final_ipp.ipe_cadj_fixth.yth2.value_hit = iq_info->iq_ref_set.fixth.fixy_y_value;
						iq_info->final_ipp.ipe_cadj_fixth.yth2.nonvalue_hit = 0;
					} else {
						iq_info->final_ipp.ipe_cadj_fixth.yth2.ycth_sel_hit = CTL_IPE_ISP_YCTH_ORIGINAL_VALUE;
						iq_info->final_ipp.ipe_cadj_fixth.yth2.ycth_sel_nonhit = CTL_IPE_ISP_YCTH_REGISTER_VALUE;
						iq_info->final_ipp.ipe_cadj_fixth.yth2.value_hit = 0;
						iq_info->final_ipp.ipe_cadj_fixth.yth2.nonvalue_hit = iq_info->iq_ref_set.fixth.fixy_y_value;
					}
				}
				// C judge : ((e < edge_th) || (y/cb/cr match))
				iq_info->final_ipp.ipe_cadj_fixth.cth.edge_th = 0;
				iq_info->final_ipp.ipe_cadj_fixth.cth.y_th_low = iq_info->iq_ref_set.fixth.fixc_y_th_low;
				iq_info->final_ipp.ipe_cadj_fixth.cth.y_th_high = iq_info->iq_ref_set.fixth.fixc_y_th_high;
				iq_info->final_ipp.ipe_cadj_fixth.cth.cb_th_low = iq_info->iq_ref_set.fixth.fixc_cb_th_low;
				iq_info->final_ipp.ipe_cadj_fixth.cth.cb_th_high = iq_info->iq_ref_set.fixth.fixc_cb_th_high;
				iq_info->final_ipp.ipe_cadj_fixth.cth.cr_th_low = iq_info->iq_ref_set.fixth.fixc_cr_th_low;
				iq_info->final_ipp.ipe_cadj_fixth.cth.cr_th_high = iq_info->iq_ref_set.fixth.fixc_cr_th_high;
				iq_info->final_ipp.ipe_cadj_fixth.cth.ycth_sel_hit = CTL_IPE_ISP_YCTH_REGISTER_VALUE;
				iq_info->final_ipp.ipe_cadj_fixth.cth.ycth_sel_nonhit = CTL_IPE_ISP_YCTH_ORIGINAL_VALUE;
				iq_info->final_ipp.ipe_cadj_fixth.cth.cb_value_hit = iq_info->iq_ref_set.fixth.fixc_cb_value;
				iq_info->final_ipp.ipe_cadj_fixth.cth.cb_value_nonhit = 0;
				iq_info->final_ipp.ipe_cadj_fixth.cth.cr_value_hit = iq_info->iq_ref_set.fixth.fixc_cr_value;
				iq_info->final_ipp.ipe_cadj_fixth.cth.cr_value_nonhit = 0;
			}
			iqt_reset_param_update(iq_info->id, IQT_ITEM_FIXTH_PARAM);
		}

		// IQ_CST_PARAM setting
		if (update_all || (iqt_get_param_update(iq_info->id, IQT_ITEM_CST_PARAM) == TRUE)) {
			memcpy(iq_info->final_ipp.ipe_cst.cst_coef, iq_param->cst->cst_coef, sizeof(INT16) * CTL_IPE_ISP_COEF_LEN);
			iq_info->final_ipp.ipe_cctrl.int_ofs = iq_param->cst->y_ofs;
			iq_info->final_ipp.ipe_cadj_cofs.cb_ofs = iq_param->cst->cb_ofs;
			iq_info->final_ipp.ipe_cadj_cofs.cr_ofs = iq_param->cst->cr_ofs;
			iqt_reset_param_update(iq_info->id, IQT_ITEM_CST_PARAM);
		}
		break;

	default:
	case ISP_TRIG_IQ_ENC:
		break;
	}
}

static void iq_flow_set_sync_param(ISP_TRIG_MSG msg, IQALG_INFO *iq_info, IQ_PARAM_PTR *iq_param)
{
	UINT32 dbg_mode = iq_dbg_get_dbg_mode(iq_info->id);
	URECT ca_roi = {0};
	ISP_CA_TH_PARAM ca_th_param = {0};
	BOOL ca_enable = 0, la_enable = 0;
	URECT la_roi = {0};
	URECT ife_va_indep_roi[ISP_INDEP_VA_WIN_NUM] = {0};
	URECT va_indep_roi[ISP_INDEP_VA_WIN_NUM] = {0};
	ER ca_sync_rt = E_OK, la_sync_rt = E_OK, va_sync_rt = E_OK;

	switch ((UINT32)msg) {
	case ISP_TRIG_IQ_SIE:
		if (iq_info->sie_trig_obj.func_en & ISP_FUNC_EN_AWB) {
			ca_sync_rt = isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_CA_ROI, &ca_roi);
			if ((ca_sync_rt == E_OK) && (ca_roi.w > 0) && (ca_roi.h > 0)
				&& ((ca_roi.x + ca_roi.w) <= iq_info->final_sie.sie_roi_ratio.ratio_base) && ((ca_roi.y + ca_roi.h) <= iq_info->final_sie.sie_roi_ratio.ratio_base)) {
				iq_info->final_sie.sie_roi_ratio.ca_crop_win_roi = ca_roi;
			} else {
				if (ca_sync_rt != E_OK) {
					PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "CA ROI not update!! \r\n");
				} else {
					PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "IQ(%d) sync ca_roi (%d, %d, %d, %d) is out of range!! \r\n", iq_info->id, ca_roi.x, ca_roi.y, ca_roi.w, ca_roi.h);
				}
			}

			ca_sync_rt = isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_CA_TH, &ca_th_param);
			if (ca_sync_rt == E_OK) {
				iq_info->final_sie.sie_ca.th_enable = ca_th_param.th_enable;
				iq_info->final_sie.sie_ca.g_th_l = ca_th_param.g_th_l;
				iq_info->final_sie.sie_ca.g_th_u = ca_th_param.g_th_u;
				iq_info->final_sie.sie_ca.r_th_l = ca_th_param.r_th_l;
				iq_info->final_sie.sie_ca.r_th_u = ca_th_param.r_th_u;
				iq_info->final_sie.sie_ca.b_th_l = ca_th_param.b_th_l;
				iq_info->final_sie.sie_ca.b_th_u = ca_th_param.b_th_u;
				iq_info->final_sie.sie_ca.p_th_l = ca_th_param.p_th_l;
				iq_info->final_sie.sie_ca.p_th_u = ca_th_param.p_th_u;
			} else {
				PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "CA th not update!! \r\n");
			}

			ca_sync_rt = isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AWB_CA_ENABLE, &ca_enable);
			if (ca_sync_rt == E_OK) {
				iq_info->final_sie.sie_ca.enable = ca_enable;
			} else {
				PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "CA enable not update!! \r\n");
			}
		}

		if (iq_info->sie_trig_obj.func_en & ISP_FUNC_EN_AE) {
			la_sync_rt = isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_LA_ROI, &la_roi);
			if ((la_sync_rt == E_OK) && (la_roi.w > 0) && (la_roi.h > 0)
				&& ((la_roi.x + la_roi.w) <= iq_info->final_sie.sie_roi_ratio.ratio_base) && ((la_roi.y + la_roi.h) <= iq_info->final_sie.sie_roi_ratio.ratio_base)) {
				iq_info->final_sie.sie_roi_ratio.la_crop_win_roi = la_roi;
			} else {
				if (la_sync_rt != E_OK) {
					PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "LA ROI not update!! \r\n");
				} else {
					PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "IQ(%d) sync la_roi (%d, %d, %d, %d) is out of range!! \r\n", iq_info->id, la_roi.x, la_roi.y, la_roi.w, la_roi.h);
				}
			}

			la_sync_rt = isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AE_LA_ENABLE, &la_enable);
			if (la_sync_rt == E_OK) {
				iq_info->final_sie.sie_la.enable = la_enable;
			} else {
				PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "LA enable not update!! \r\n");
			}
		}
		break;

	case ISP_TRIG_IQ_IPP:
		if (iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_AF) {
			va_sync_rt = isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_IFE_VA_INDEP_ROI, &ife_va_indep_roi[0]);
			if ((va_sync_rt == E_OK) && (ife_va_indep_roi[0].w > 0) && (ife_va_indep_roi[0].h > 0) && (ife_va_indep_roi[1].w > 0) && (ife_va_indep_roi[1].h > 0)
				&& (ife_va_indep_roi[2].w > 0) && (ife_va_indep_roi[2].h > 0) && (ife_va_indep_roi[3].w > 0) && (ife_va_indep_roi[3].h > 0) && (ife_va_indep_roi[4].w > 0) && (ife_va_indep_roi[4].h > 0)
				&& ((ife_va_indep_roi[0].x + ife_va_indep_roi[0].w) <= iq_info->final_ipp.ife_va_win_size.ratio_base) && ((ife_va_indep_roi[0].y + ife_va_indep_roi[0].h) <= iq_info->final_ipp.ife_va_win_size.ratio_base)
				&& ((ife_va_indep_roi[1].x + ife_va_indep_roi[1].w) <= iq_info->final_ipp.ife_va_win_size.ratio_base) && ((ife_va_indep_roi[1].y + ife_va_indep_roi[1].h) <= iq_info->final_ipp.ife_va_win_size.ratio_base)
				&& ((ife_va_indep_roi[2].x + ife_va_indep_roi[2].w) <= iq_info->final_ipp.ife_va_win_size.ratio_base) && ((ife_va_indep_roi[2].y + ife_va_indep_roi[2].h) <= iq_info->final_ipp.ife_va_win_size.ratio_base)
				&& ((ife_va_indep_roi[3].x + ife_va_indep_roi[3].w) <= iq_info->final_ipp.ife_va_win_size.ratio_base) && ((ife_va_indep_roi[3].y + ife_va_indep_roi[3].h) <= iq_info->final_ipp.ife_va_win_size.ratio_base)
				&& ((ife_va_indep_roi[4].x + ife_va_indep_roi[4].w) <= iq_info->final_ipp.ife_va_win_size.ratio_base) && ((ife_va_indep_roi[4].y + ife_va_indep_roi[4].h) <= iq_info->final_ipp.ife_va_win_size.ratio_base)) {
				iq_info->final_ipp.ife_va_win_size.indep_roi_ratio[0] = ife_va_indep_roi[0];
				iq_info->final_ipp.ife_va_win_size.indep_roi_ratio[1] = ife_va_indep_roi[1];
				iq_info->final_ipp.ife_va_win_size.indep_roi_ratio[2] = ife_va_indep_roi[2];
				iq_info->final_ipp.ife_va_win_size.indep_roi_ratio[3] = ife_va_indep_roi[3];
				iq_info->final_ipp.ife_va_win_size.indep_roi_ratio[4] = ife_va_indep_roi[4];
			} else {
				if (va_sync_rt != E_OK) {
					PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "IFE VA ROI not update!! \r\n");
				} else {
					PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "IQ(%d) sync ife_va_indep_roi is out of range!! \r\n", iq_info->id);
					PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "ife_va_indep_roi(0) (%d, %d, %d, %d)!! \r\n", ife_va_indep_roi[0].x, ife_va_indep_roi[0].y, ife_va_indep_roi[0].w, ife_va_indep_roi[0].h);
					PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "ife_va_indep_roi(1) (%d, %d, %d, %d)!! \r\n", ife_va_indep_roi[1].x, ife_va_indep_roi[1].y, ife_va_indep_roi[1].w, ife_va_indep_roi[1].h);
					PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "ife_va_indep_roi(2) (%d, %d, %d, %d)!! \r\n", ife_va_indep_roi[2].x, ife_va_indep_roi[2].y, ife_va_indep_roi[2].w, ife_va_indep_roi[2].h);
					PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "ife_va_indep_roi(3) (%d, %d, %d, %d)!! \r\n", ife_va_indep_roi[3].x, ife_va_indep_roi[3].y, ife_va_indep_roi[3].w, ife_va_indep_roi[3].h);
					PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "ife_va_indep_roi(4) (%d, %d, %d, %d)!! \r\n", ife_va_indep_roi[4].x, ife_va_indep_roi[4].y, ife_va_indep_roi[4].w, ife_va_indep_roi[4].h);
				}
			}

			va_sync_rt = isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_VA_INDEP_ROI, &va_indep_roi[0]);
			if ((va_sync_rt == E_OK) && (va_indep_roi[0].w > 0) && (va_indep_roi[0].h > 0) && (va_indep_roi[1].w > 0) && (va_indep_roi[1].h > 0)
				&& (va_indep_roi[2].w > 0) && (va_indep_roi[2].h > 0) && (va_indep_roi[3].w > 0) && (va_indep_roi[3].h > 0) && (va_indep_roi[4].w > 0) && (va_indep_roi[4].h > 0)
				&& ((va_indep_roi[0].x + va_indep_roi[0].w) <= iq_info->final_ipp.ipe_va_win_size.ratio_base) && ((va_indep_roi[0].y + va_indep_roi[0].h) <= iq_info->final_ipp.ipe_va_win_size.ratio_base)
				&& ((va_indep_roi[1].x + va_indep_roi[1].w) <= iq_info->final_ipp.ipe_va_win_size.ratio_base) && ((va_indep_roi[1].y + va_indep_roi[1].h) <= iq_info->final_ipp.ipe_va_win_size.ratio_base)
				&& ((va_indep_roi[2].x + va_indep_roi[2].w) <= iq_info->final_ipp.ipe_va_win_size.ratio_base) && ((va_indep_roi[2].y + va_indep_roi[2].h) <= iq_info->final_ipp.ipe_va_win_size.ratio_base)
				&& ((va_indep_roi[3].x + va_indep_roi[3].w) <= iq_info->final_ipp.ipe_va_win_size.ratio_base) && ((va_indep_roi[3].y + va_indep_roi[3].h) <= iq_info->final_ipp.ipe_va_win_size.ratio_base)
				&& ((va_indep_roi[4].x + va_indep_roi[4].w) <= iq_info->final_ipp.ipe_va_win_size.ratio_base) && ((va_indep_roi[4].y + va_indep_roi[4].h) <= iq_info->final_ipp.ipe_va_win_size.ratio_base)) {
				iq_info->final_ipp.ipe_va_win_size.indep_roi_ratio[0] = va_indep_roi[0];
				iq_info->final_ipp.ipe_va_win_size.indep_roi_ratio[1] = va_indep_roi[1];
				iq_info->final_ipp.ipe_va_win_size.indep_roi_ratio[2] = va_indep_roi[2];
				iq_info->final_ipp.ipe_va_win_size.indep_roi_ratio[3] = va_indep_roi[3];
				iq_info->final_ipp.ipe_va_win_size.indep_roi_ratio[4] = va_indep_roi[4];
			} else {
				if (va_sync_rt != E_OK) {
					PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "IPE VA ROI not update!! \r\n");
				} else {
					PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "IQ(%d) sync va_indep_roi is out of range!! \r\n", iq_info->id);
					PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "va_indep_roi(0) (%d, %d, %d, %d)!! \r\n", va_indep_roi[0].x, va_indep_roi[0].y, va_indep_roi[0].w, va_indep_roi[0].h);
					PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "va_indep_roi(1) (%d, %d, %d, %d)!! \r\n", va_indep_roi[1].x, va_indep_roi[1].y, va_indep_roi[1].w, va_indep_roi[1].h);
					PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "va_indep_roi(2) (%d, %d, %d, %d)!! \r\n", va_indep_roi[2].x, va_indep_roi[2].y, va_indep_roi[2].w, va_indep_roi[2].h);
					PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "va_indep_roi(3) (%d, %d, %d, %d)!! \r\n", va_indep_roi[3].x, va_indep_roi[3].y, va_indep_roi[3].w, va_indep_roi[3].h);
					PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "va_indep_roi(4) (%d, %d, %d, %d)!! \r\n", va_indep_roi[4].x, va_indep_roi[4].y, va_indep_roi[4].w, va_indep_roi[4].h);
				}
			}
		}
		break;

	default:
	case ISP_TRIG_IQ_ENC:
		break;
	}
}

INT32 iq_flow_process(UINT32 id, ISP_TRIG_MSG msg, void *arg)
{
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;
	IQALG_INFO *iq_info_sie_src;
	ISP_IQ_TRIG_OBJ *trig_obj = (ISP_IQ_TRIG_OBJ *)arg;
	ISP_AE_INIT_INFO ae_init_info = {0};
	ISP_AWB_INIT_INFO awb_init_info = {0};
	ER rt = E_OK, ae_sync_rt = E_OK, awb_sync_rt = E_OK, ir_sync_rt = E_OK, enc_sync_rt = E_OK;
	ISP_HISTO_RSLT *histo_rslt;
	ISP_DEFOG_STCS *defog_stcs;
	ISP_IPE_SUBOUT_BUF *subout_buf;
	UINT32 dbg_mode = iq_dbg_get_dbg_mode(id);
	UINT32 t0, t1;
	INT32 i;

	t0 = hwclock_get_counter();

	if (id >= IQ_ID_MAX_NUM) {
		PRINT_IQ_WRN(dbg_mode & IQ_DBG_WRN_MSG, "IQ ID (%d) out of range!! \r\n", id);
		return E_SYS;
	}

	if (!iq_flow_get_id_valid(id)) {
		PRINT_IQ_WRN(dbg_mode & IQ_DBG_WRN_MSG, "invalid IQ ID (%d)!! \r\n", id);
		return E_SYS;
	}

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(id));
	if (iq_info == NULL) {
		PRINT_IQ_WRN(dbg_mode & IQ_DBG_WRN_MSG, "wrong iq_module->private!! \r\n");
		return E_SYS;
	}

	if (iq_info->id != id) {
		PRINT_IQ_WRN(dbg_mode & IQ_DBG_WRN_MSG, "wrong IQ (%d) init ID (%d)!! \r\n", id, iq_info->id);
		iq_info->id = id;
	}

	if (msg == ISP_TRIG_IQ_SIE) {
		iq_info->sie_trig_obj = trig_obj->sie;
	} else if (msg == ISP_TRIG_IQ_IPP) {
		iq_info->ipp_trig_obj = trig_obj->ipp;
	}

	switch ((UINT32)msg) {
	case ISP_TRIG_IQ_SIE:
		if (iq_info->final_sie.ready_flag == TRUE) {
			iq_info->final_sie.ready_flag = FALSE;
		} else {
			PRINT_IQ_WRN(dbg_mode & IQ_DBG_WRN_MSG, "IQ(%d) SIE process retrigger before finish!! \r\n", iq_info->id);
		}

		// NOTE: FLOW mode setting
		switch (iq_info->sie_trig_obj.mode_type) {
		case ISP_SEN_MODE_STAGGER_HDR:
		case ISP_SEN_MODE_BUILTIN_DCG_SHDR:
		case ISP_SEN_MODE_STAGGER_PDAF:
		case ISP_SEN_MODE_DCG_HDR:
		case ISP_SEN_MODE_DCG_SHDR:
		case ISP_SEN_MODE_STAGGER3_HDR:
		#if IQ_STAGGER2_PDAF_READY
		case ISP_SEN_MODE_STAGGER2_PDAF:
		#endif
			if (iq_info->sie_trig_obj.func_en & ISP_FUNC_EN_SHDR) {
				iq_info->sie_flow_mode = IQ_FLOW_SHDR;
			} else {
				iq_info->sie_flow_mode = IQ_FLOW_LINEAR;
			}
		break;

		default:
			iq_info->sie_flow_mode = IQ_FLOW_LINEAR;
			break;
		}

		// NOTE: DPAF/Companding
		if (iq_info->sie_trig_obj.frame_num != 1) {
			for (i = ISP_ID_MAX_NUM - 1; i >= 0; i--) {
				if (((iq_info->sie_trig_obj.src_id_mask >> i) & 0x01) != 0) {
					iq_info->mul_last_id = (UINT32)i;
					break;
				}
			}
		} else {
			iq_info->mul_last_id = iq_info->id;
		}

		// NOTE: SHDR
		if ((iq_info->sie_flow_mode == IQ_FLOW_SHDR) && (iq_info->sie_trig_obj.func_en & ISP_FUNC_EN_SHDR)) {
			iq_info->shdr_long_id = iq_info->mul_last_id;
		} else {
			iq_info->shdr_long_id = iq_info->id;
		}

		// NOTE: Reset process
		if (iq_info->sie_trig_obj.reset) {
			if (iq_ui_get_info(iq_info->id, IQ_UI_ITEM_OPERATION) == IQ_UI_OPERATION_MOVIE) {
				iq_info->sie_proc_mode = IQ_PROC_MOVIE;
			} else if (iq_ui_get_info(iq_info->id, IQ_UI_ITEM_OPERATION) == IQ_UI_OPERATION_PHOTO) {
				iq_info->sie_proc_mode = IQ_PROC_PHOTO;
			} else if (iq_ui_get_info(iq_info->id, IQ_UI_ITEM_OPERATION) == IQ_UI_OPERATION_CAPTURE) {
				iq_info->sie_proc_mode = IQ_PROC_CAPTURE;
			} else {
				iq_info->sie_proc_mode = IQ_PROC_MOVIE;
				PRINT_IQ_WRN(dbg_mode & IQ_DBG_WRN_MSG, "IQ(%d) IQ_UI_OPERATION(%d) unknow, sie_proc_mode fixed to IQ_PROC_MOVIE!! \r\n"
					, iq_info->id, iq_ui_get_info(iq_info->id, IQ_UI_ITEM_OPERATION));
			}

			// NOTE: get AE/AWB initial information here.
			ae_sync_rt |= isp_dev_get_sync_item(id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_AE_INIT, &ae_init_info);
			if (ae_sync_rt == E_OK) {
				if (ae_init_info.lv != 0) {
					iq_info->final_sie.sync_info.lv = ae_init_info.lv;
				}
				if (ae_init_info.total_gain != 0) {
					iq_info->final_sie.sync_info.gain = ae_init_info.total_gain;
				}
				if (ae_init_info.d_gain != 0) {
					iq_info->final_sie.sync_info.dgain = ae_init_info.d_gain;
				}
				if (ae_init_info.shdr_tm_ratio != 0) {
					iq_info->final_sie.sync_info.shdr_tm_ratio = ae_init_info.shdr_tm_ratio;
				}
				if (ae_init_info.shdr_ev_ratio[0] != 0) {
					iq_info->final_sie.sync_info.shdr_ev_ratio[0] = ae_init_info.shdr_ev_ratio[0];
					iq_info->final_sie.sync_info.shdr_ev_ratio[1] = ae_init_info.shdr_ev_ratio[1];
				}
			} else {
				PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "SIE(%d) trig - AE init not update!! \r\n", iq_info->id);
				ae_sync_rt = E_OK; //reset for later sync
			}
			awb_sync_rt |= isp_dev_get_sync_item(id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_AWB_INIT, &awb_init_info);
			if (awb_sync_rt == E_OK) {
				if (awb_init_info.ct != 0) {
					iq_info->final_sie.sync_info.ct = awb_init_info.ct;
				}
				if (awb_init_info.r_gain != 0) {
					iq_info->final_sie.sync_info.cgain[0] = awb_init_info.r_gain;
				}
				if (awb_init_info.g_gain != 0) {
					iq_info->final_sie.sync_info.cgain[1] = awb_init_info.g_gain;
				}
				if (awb_init_info.b_gain != 0) {
					iq_info->final_sie.sync_info.cgain[2] = awb_init_info.b_gain;
				}
			} else {
				PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "SIE(%d) trig - AWB init not update!! \r\n", iq_info->id);
				awb_sync_rt = E_OK; //reset for later sync
			}

			if ((iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_BUILTIN_HDR) || (iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_BUILTIN_DCG_HDR) || (iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_BUILTIN_DCG_SHDR)) {
				companding_en[iq_info->id] = TRUE;
				if ((iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_BUILTIN_DCG_SHDR) && (iq_info->id != iq_info->mul_last_id)) {
					companding_en[iq_info->id] = FALSE;
				}
			}

			// NOTE: initial CA/LA enable
			iq_info->final_sie.sie_ca.enable = ctl_sie_ca_init.enable;
			iq_info->final_sie.sie_la.enable = ctl_sie_la_init.enable;
		}

		// NOTE: OB/CG/DG mode setting
		if ((iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_BUILTIN_HDR) || (iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_BUILTIN_DCG_HDR) || (iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_BUILTIN_DCG_SHDR)) {
			// Builtin
			iq_info->ob_mode = IQ_OB_SIE;
			iq_info->dg_mode = IQ_DG_SIE;
			iq_info->cg_mode = IQ_CG_SIE;
			iq_info->final_sie.sie_companding.enable = companding_en[iq_info->id];
		} else if ((iq_info->sie_flow_mode == IQ_FLOW_SHDR) && (iq_info->sie_trig_obj.func_en & ISP_FUNC_EN_SHDR)) {
			// SHDR
			iq_info->ob_mode = IQ_OB_IFE_F;
			iq_info->dg_mode = IQ_DG_SIE;
			iq_info->cg_mode = IQ_CG_IFE_F;
			iq_info->final_sie.sie_companding.enable = companding_en[iq_info->id];
		} else if (iq_info->sie_trig_obj.data_fmt == ISP_SEN_DATA_FMT_RGBIR) {
			// RGBIR
			iq_info->ob_mode = IQ_OB_IFE;
			iq_info->dg_mode = IQ_DG_SIE;
			iq_info->cg_mode = IQ_CG_IPE;
			iq_info->final_sie.sie_companding.enable = companding_en[iq_info->id];
		} else {
			iq_info->ob_mode = IQ_OB_IFE;
			iq_info->dg_mode = IQ_DG_SIE;
			iq_info->cg_mode = IQ_CG_IFE;
			iq_info->final_sie.sie_companding.enable = companding_en[iq_info->id];
		}
		if (iq_info->ob_mode_manual.manual_enable) {
			iq_info->ob_mode = iq_info->ob_mode_manual.manual_mode;
		}
		if (iq_info->dg_mode_manual.manual_enable) {
			iq_info->dg_mode = iq_info->dg_mode_manual.manual_mode;
		}
		if (test_mode_en[iq_info->id] == TRUE) {
			iq_info->ob_mode = test_mode_ob[iq_info->id];
			iq_info->dg_mode = test_mode_dg[iq_info->id];
			iq_info->cg_mode = test_mode_cg[iq_info->id];
		}

		// NOTE: get AE/AWB information here.
		if (iq_info->sie_proc_mode == IQ_PROC_CAPTURE) {
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AE_STATUS, &(iq_info->final_sie.sync_info.ae_status));
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_CAPTURE_TOTAL_GAIN, &(iq_info->final_sie.sync_info.gain));
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_CAPTURE_DGAIN, &(iq_info->final_sie.sync_info.dgain));
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AE_LV, &(iq_info->final_sie.sync_info.lv));
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AE_LV_BASE, &(iq_info->final_sie.sync_info.lv_base));

			awb_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AWB_CGAIN, &(iq_info->final_sie.sync_info.cgain[0]));
			awb_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AWB_CT, &(iq_info->final_sie.sync_info.ct));

			PRINT_IQ(dbg_mode & IQ_DBG_CAPTURE, "SIE(%d) get_sync_item for capture total_gain(%d) dgain(%d) cgain(%d, %d, %d) ct(%d)!! \r\n"
				, iq_info->id, iq_info->final_sie.sync_info.gain, iq_info->final_sie.sync_info.dgain
				, iq_info->final_sie.sync_info.cgain[0], iq_info->final_sie.sync_info.cgain[1]
				, iq_info->final_sie.sync_info.cgain[2], iq_info->final_sie.sync_info.ct);
		} else {
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AE_STATUS, &(iq_info->final_sie.sync_info.ae_status));
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AE_TOTAL_GAIN, &(iq_info->final_sie.sync_info.gain));
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AE_DGAIN, &(iq_info->final_sie.sync_info.dgain));
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AE_LV, &(iq_info->final_sie.sync_info.lv));
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AE_LV_BASE, &(iq_info->final_sie.sync_info.lv_base));
			// NOTE: SHDR
			if ((iq_info->sie_flow_mode == IQ_FLOW_SHDR) && (iq_info->sie_trig_obj.func_en & ISP_FUNC_EN_SHDR)) {
				// SHDR sync info
				ae_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AE_SHDR_TM_RATIO, &(iq_info->final_sie.sync_info.shdr_tm_ratio));
				ae_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AE_SHDR_EV_RATIO, &iq_info->final_sie.sync_info.shdr_ev_ratio[0]);
				ae_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AE_SHDR_HBS, &(iq_info->final_sie.sync_info.shdr_hbs_param));
			}

			awb_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AWB_CGAIN, &(iq_info->final_sie.sync_info.cgain[0]));
			awb_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_SIE, ISP_SYNC_AWB_CT, &(iq_info->final_sie.sync_info.ct));
		}
		if (ae_sync_rt != E_OK) {
			PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "SIE(%d) trig - AE status not update!! \r\n", iq_info->id);
		}
		if (awb_sync_rt != E_OK) {
			PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "SIE(%d) trig - AWB status not update!! \r\n", iq_info->id);
		}
		if (test_shdr_en[iq_info->id] == TRUE) {
			iq_info->final_sie.sync_info.shdr_ev_ratio[1] = test_ev_ratio[iq_info->id][1];
			iq_info->final_sie.sync_info.shdr_tm_ratio = test_tm_ratio[iq_info->id];
		}
		if (test_shdr_hbs_en[iq_info->id] == TRUE) {
			iq_info->final_sie.sync_info.shdr_hbs_param.lum_th = test_hbs[iq_info->id].lum_th;
			iq_info->final_sie.sync_info.shdr_hbs_param.w_start = test_hbs[iq_info->id].w_start;
			iq_info->final_sie.sync_info.shdr_hbs_param.w_slope = test_hbs[iq_info->id].w_slope;
		}

		// NOTE: get IR information
		ir_sync_rt |= isp_api_get_ir_info(iq_info->id, &(iq_info->ir_info));
		if (ir_sync_rt != E_OK) {
			PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "SIE(%d) trig - IR status not update!! \r\n", iq_info->id);
		}

		// NOTE: SHDR sie_param select and sie_setting synchronize
		if (iq_info->sie_flow_mode == IQ_FLOW_SHDR) {
			iq_info->sie_param_id = iq_info->sie_trig_obj.dupl_src_id;
		} else {
			iq_info->sie_param_id = iq_info->id;
		}
		if (iq_info->sie_param_id != iq_info->id) {
			iq_info_sie_src = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(iq_info->sie_param_id));
			if ((iq_info->dpc_en == TRUE) && (iq_info->sie_trig_obj.reset || (iqt_get_param_update(iq_info->sie_param_id, IQT_ITEM_DPC_PARAM) == TRUE))) {
				memcpy(iq_info->final_sie.sie_dpc, iq_info_sie_src->final_sie.sie_dpc, sizeof(IQ_SIE_DPC));
				PRINT_IQ(dbg_mode & IQ_DBG_TABLE, "IQ(%d) DPC updated from src_id!! \r\n", iq_info->id);
			}
			if ((iq_info->ecs_en == TRUE) && (iq_info->sie_trig_obj.reset || (iqt_get_param_update(iq_info->sie_param_id, IQT_ITEM_SHADING_PARAM) == TRUE))) {
				memcpy(iq_info->final_sie.sie_ecs, iq_info_sie_src->final_sie.sie_ecs, sizeof(IQ_SIE_ECS));
				iq_info->iq_ref_set.ecs_mode = iq_info_sie_src->iq_ref_set.ecs_mode;
				iq_info->iq_ref_set.ecs_smooth_l_m_ct_lower = iq_info_sie_src->iq_ref_set.ecs_smooth_l_m_ct_lower;
				iq_info->iq_ref_set.ecs_smooth_l_m_ct_upper = iq_info_sie_src->iq_ref_set.ecs_smooth_l_m_ct_upper;
				iq_info->iq_ref_set.ecs_smooth_m_h_ct_lower = iq_info_sie_src->iq_ref_set.ecs_smooth_m_h_ct_lower;
				iq_info->iq_ref_set.ecs_smooth_m_h_ct_upper = iq_info_sie_src->iq_ref_set.ecs_smooth_m_h_ct_upper;
				memcpy(iq_info->iq_ref_set.ecs_ext, iq_info_sie_src->iq_ref_set.ecs_ext, sizeof(IQ_REF_SHADING_PARAM));
				PRINT_IQ(dbg_mode & IQ_DBG_TABLE, "IQ(%d) ECS updated from src_id!! \r\n", iq_info->id);
			}
		}

		// NOTE: Set LA/CA enable
		if (!(iq_info->sie_trig_obj.func_en & ISP_FUNC_EN_AE)) {
			iq_info->final_sie.sie_la.enable = FALSE;
		}
		if (!(iq_info->sie_trig_obj.func_en & ISP_FUNC_EN_AWB)) {
			iq_info->final_sie.sie_ca.enable = FALSE;
		}

		iq_flow_set_update(msg, iq_info, iq_param[iq_info->sie_param_id]);
		iq_flow_set_sync_param(msg, iq_info, iq_param[iq_info->sie_param_id]);
		iq_operation_iso(msg, iq_info, iq_param[iq_info->sie_param_id]);
		if (iq_info->sie_trig_obj.data_fmt == ISP_SEN_DATA_FMT_RGBIR) {
			iq_operation_rgbir(msg, iq_info, iq_param[iq_info->sie_param_id]);
		}

		isp_dev_set_sync_item(iq_info->id, ISP_SYNC_SEL_SIE, ISP_SYNC_IQ_FINAL_CGAIN, &iq_info->final_sie.final_cgain[0]);

		iq_operation_ecs(iq_info, iq_param[iq_info->id], (iq_info->sie_tab_update & IQ_TBL_SIE_ECS));
		iq_operation_effect(msg, iq_info, iq_param[iq_info->sie_param_id], TRUE);

		// NOTE: Special sensor setting
		if ((iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_CCIR) || (iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_CCIR_INTERLACE)) {
			iq_operation_sie_bypass(msg, iq_info, iq_param[iq_info->sie_param_id]);
		}
		#if IQ_STAGGER2_PDAF_READY
		if (((iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_PDAF) || (iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_STAGGER2_PDAF)) && (!(iq_info->sie_trig_obj.func_en & ISP_FUNC_EN_AE)) && (!(iq_info->sie_trig_obj.func_en & ISP_FUNC_EN_AWB))) // last path is PD data
		#else
		if ((iq_info->sie_trig_obj.mode_type == ISP_SEN_MODE_PDAF) && (!(iq_info->sie_trig_obj.func_en & ISP_FUNC_EN_AE)) && (!(iq_info->sie_trig_obj.func_en & ISP_FUNC_EN_AWB))) // last path is PD data
		#endif
		{
			iq_operation_sie_bypass(msg, iq_info, iq_param[iq_info->sie_param_id]);
		}
		if (iq_info->sie_trig_obj.data_fmt == ISP_SEN_DATA_FMT_DVS) {
			iq_operation_sie_bypass(msg, iq_info, iq_param[iq_info->sie_param_id]);
		}

		iq_info->final_sie.ready_flag = TRUE;

		iq_flow_reset_sie_setting(iq_info);
		iq_flow_reset_sie_param(iq_info);
		rt |= iq_flow_set_sie_setting(iq_info);
		rt |= iq_flow_set_sie_param(iq_info);
		iq_msg_sie(iq_info);

		iq_info->sie_tab_update &= ~(IQ_TBL_SIE_DPC | IQ_TBL_SIE_ECS);

		t1 = hwclock_get_counter();
		PRINT_IQ(dbg_mode & IQ_DBG_PERFORMANCE, "SIE time (%d)!! \r\n", t1 - t0);
		break;

	case ISP_TRIG_IQ_IPP:
		if (iq_info->final_ipp.ready_flag == TRUE) {
			iq_info->final_ipp.ready_flag = FALSE;
		} else {
			PRINT_IQ_WRN(dbg_mode & IQ_DBG_WRN_MSG, "IQ(%d) IPP process retrigger before finish!! \r\n", iq_info->id);
		}

		// NOTE: FLOW mode setting
		switch (iq_info->ipp_trig_obj.mode_type) {
		case ISP_SEN_MODE_STAGGER_HDR:
		case ISP_SEN_MODE_BUILTIN_DCG_SHDR:
		case ISP_SEN_MODE_STAGGER_PDAF:
		case ISP_SEN_MODE_DCG_HDR:
		case ISP_SEN_MODE_DCG_SHDR:
		case ISP_SEN_MODE_STAGGER3_HDR:
		#if IQ_STAGGER2_PDAF_READY
		case ISP_SEN_MODE_STAGGER2_PDAF:
		#endif
			if (iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_SHDR) {
				iq_info->ipp_flow_mode = IQ_FLOW_SHDR;
			} else {
				iq_info->ipp_flow_mode = IQ_FLOW_LINEAR;
			}
		break;

		default:
			iq_info->ipp_flow_mode = IQ_FLOW_LINEAR;
			break;
		}

		if (iq_info->ipp_trig_obj.reset) {
			// NOTE: get IQ/AE/AWB initial information here.
			ae_sync_rt |= isp_dev_get_sync_item(id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_AE_INIT, &ae_init_info);
			if (ae_sync_rt == E_OK) {
				if (ae_init_info.lv != 0) {
					iq_info->final_ipp.sync_info.lv = ae_init_info.lv;
				}
				if (ae_init_info.total_gain != 0) {
					iq_info->final_ipp.sync_info.gain = ae_init_info.total_gain;
				}
				if (ae_init_info.d_gain != 0) {
					iq_info->final_ipp.sync_info.dgain = ae_init_info.d_gain;
				}
				if (ae_init_info.shdr_tm_ratio != 0) {
					iq_info->final_ipp.sync_info.shdr_tm_ratio = ae_init_info.shdr_tm_ratio;
				}
				if (ae_init_info.shdr_ev_ratio[0] != 0) {
					iq_info->final_ipp.sync_info.shdr_ev_ratio[0] = ae_init_info.shdr_ev_ratio[0];
					iq_info->final_ipp.sync_info.shdr_ev_ratio[1] = ae_init_info.shdr_ev_ratio[1];
				}
				PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "IPP(%d) trig - AE init (%d, %d, %d) \r\n"
					, iq_info->id, ae_init_info.lv, ae_init_info.total_gain, ae_init_info.d_gain);
			} else {
				PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "IPP(%d) trig - AE init not update!! \r\n", iq_info->id);
				ae_sync_rt = E_OK; //reset for later sync
			}
			awb_sync_rt |= isp_dev_get_sync_item(id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_AWB_INIT, &awb_init_info);
			if (awb_sync_rt == E_OK) {
				if (awb_init_info.ct != 0) {
					iq_info->final_ipp.sync_info.ct = awb_init_info.ct;
				}

				if ((awb_init_info.r_gain != 0) && (awb_init_info.g_gain != 0) && (awb_init_info.b_gain != 0)) {
					iq_info->final_ipp.sync_info.cgain[0] = awb_init_info.r_gain;
					iq_info->final_ipp.sync_info.cgain[1] = awb_init_info.g_gain;
					iq_info->final_ipp.sync_info.cgain[2] = awb_init_info.b_gain;
				}
				PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "IPP(%d) trig - AWB init (%d, %d, %d, %d) \r\n"
					, iq_info->id, awb_init_info.ct, awb_init_info.r_gain, awb_init_info.g_gain, awb_init_info.b_gain);
			} else {
				PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "IPP(%d) trig - AWB init not update!! \r\n", iq_info->id);
				awb_sync_rt = E_OK; //reset for later sync
			}

			// NOTE: SHDR, assign default parameter
			if (!is_update_shdr_param[id]) {
				if (((iq_info->ipp_flow_mode == IQ_FLOW_SHDR) && (iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_SHDR)) && (iqt_get_param_update(id, IQT_ITEM_SHDR_PARAM) == FALSE)) {
					#if IQ_STAGGER2_PDAF_READY
					if (((iq_info->ipp_flow_mode == IQ_FLOW_SHDR) && (iq_info->ipp_trig_obj.frame_num == 3)) && (iq_info->ipp_trig_obj.mode_type != ISP_SEN_MODE_STAGGER2_PDAF))
					#else
					if ((iq_info->ipp_flow_mode == IQ_FLOW_SHDR) && (iq_info->ipp_trig_obj.frame_num == 3))
					#endif
					{
						DBG_DUMP("SHDR 3 frame, reset fcurve \r\n");
						memcpy(iq_param[iq_info->id]->shdr->fcurve_left_lut, ctl_ife_fcurve_init.fcurve_l.fcur_l_lut, sizeof(UINT32) * IQ_SHDR_FCURVE_LEFT_NUM);
						memcpy(iq_param[iq_info->id]->shdr->fcurve_right_lut, ctl_ife_fcurve_init.fcurve_r.fcur_r_lut, sizeof(UINT32) * IQ_SHDR_FCURVE_RIGHT_NUM);
					}
				}
			}

			iq_info->final_ipp.ife_hist.hist_enable = ctl_ife_hist_init.hist_enable;
			iq_info->final_ipp.ife_va.enable = ctl_ife_va_init.enable;
			iq_info->final_ipp.ife_va.indep_va_enable = ctl_ife_va_init.indep_va_enable;
			iq_info->final_ipp.ipe_va.enable = ctl_ipe_va_init.enable;
			iq_info->final_ipp.ipe_va.indep_va_enable = ctl_ipe_va_init.indep_va_enable;
		}

		// NOTE: SHDR
		if ((iq_info->ipp_trig_obj.mode_type == ISP_SEN_MODE_BUILTIN_HDR) || (iq_info->ipp_trig_obj.mode_type == ISP_SEN_MODE_BUILTIN_DCG_HDR) || (iq_info->ipp_trig_obj.mode_type == ISP_SEN_MODE_BUILTIN_DCG_SHDR)) {
			// Builtin
			iq_info->ob_mode = IQ_OB_SIE;
			iq_info->dg_mode = IQ_DG_SIE;
			iq_info->cg_mode = IQ_CG_SIE;
			iq_info->final_ipp.ife_nrs_0.enable = FALSE;
			iq_info->final_ipp.ife_fcurve.enable = fcurve_en[iq_info->id];
		} else if ((iq_info->ipp_flow_mode == IQ_FLOW_SHDR) && (iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_SHDR)) {
			// SHDR
			iq_info->ob_mode = IQ_OB_IFE_F;
			iq_info->dg_mode = IQ_DG_SIE;
			iq_info->cg_mode = IQ_CG_IFE_F;
			// NOTE: SHDR, need check here
			iq_info->final_ipp.ife_nrs_0.enable = iq_param[iq_info->id]->shdr->nrs_enable;
			if ((iq_info->final_ipp.ife_fusion.fu_ctrl.mode == CTL_IFE_ISP_MODE_FUSION) && !iq_info->final_ipp.ife_fusion.dbg.enable) {
				iq_info->final_ipp.ife_fcurve.enable = TRUE;
			} else {
				iq_info->final_ipp.ife_fcurve.enable = fcurve_en[iq_info->id];
			}
		} else if (iq_info->ipp_trig_obj.data_fmt == ISP_SEN_DATA_FMT_RGBIR) {
			// RGBIR
			iq_info->ob_mode = IQ_OB_IFE;
			iq_info->dg_mode = IQ_DG_SIE;
			iq_info->cg_mode = IQ_CG_IPE;
			iq_info->final_ipp.ife_nrs_0.enable = FALSE;
			iq_info->final_ipp.ife_fcurve.enable = fcurve_en[iq_info->id];
		} else {
			iq_info->ob_mode = IQ_OB_IFE;
			iq_info->dg_mode = IQ_DG_SIE;
			iq_info->cg_mode = IQ_CG_IFE;
			iq_info->final_ipp.ife_nrs_0.enable = FALSE;
			if (iq_info->ipp_trig_obj.mode_type == ISP_SEN_MODE_THERMAL) {
				iq_info->final_ipp.ife_fcurve.enable = TRUE;
			} else {
				iq_info->final_ipp.ife_fcurve.enable = fcurve_en[iq_info->id];
			}
		}
		if ((iq_info->ipp_trig_obj.reset) || (iq_info->ipp_trig_obj.ipp_capture == TRUE)) {
			iq_info->final_ipp.ife_hist.hist_sel = CTL_IFE_ISP_BEFORE_WDR;
		} else {
			if (iq_info->final_ipp.ife_hist.hist_sel == CTL_IFE_ISP_BEFORE_WDR) {
				iq_info->final_ipp.ife_hist.hist_sel = CTL_IFE_ISP_AFTER_WDR;
			} else {
				iq_info->final_ipp.ife_hist.hist_sel = CTL_IFE_ISP_BEFORE_WDR;
			}
		}
		if (iq_info->ob_mode_manual.manual_enable) {
			iq_info->ob_mode = iq_info->ob_mode_manual.manual_mode;
		}
		if (iq_info->dg_mode_manual.manual_enable) {
			iq_info->dg_mode = iq_info->dg_mode_manual.manual_mode;
		}
		if (test_mode_en[iq_info->id] == TRUE) {
			iq_info->ob_mode = test_mode_ob[iq_info->id];
			iq_info->dg_mode = test_mode_dg[iq_info->id];
			iq_info->cg_mode = test_mode_cg[iq_info->id];
		}

		if (iq_info->ipp_trig_obj.ipp_capture == TRUE) {
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_IPP, ISP_SYNC_AE_STATUS, &(iq_info->final_ipp.sync_info.ae_status));
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_CAPTURE_TOTAL_GAIN, &(iq_info->final_ipp.sync_info.gain));
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_CAPTURE_DGAIN, &(iq_info->final_ipp.sync_info.dgain));
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_IPP, ISP_SYNC_AE_LV, &(iq_info->final_ipp.sync_info.lv));
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_IPP, ISP_SYNC_AE_LV_BASE, &(iq_info->final_ipp.sync_info.lv_base));

			awb_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_CURRENT, ISP_SYNC_CAPTURE_CGAIN, &(iq_info->final_ipp.sync_info.cgain[0]));
			awb_sync_rt |= isp_dev_get_sync_item(iq_info->id, ISP_SYNC_SEL_IPP, ISP_SYNC_AWB_CT, &(iq_info->final_ipp.sync_info.ct));

			PRINT_IQ(dbg_mode & IQ_DBG_CAPTURE, "IPP(%d) get_sync_item for capture total_gain(%d) dgain(%d) cgain(%d, %d, %d) ct(%d)!! \r\n"
				, iq_info->id, iq_info->final_ipp.sync_info.gain, iq_info->final_ipp.sync_info.dgain
				, iq_info->final_ipp.sync_info.cgain[0], iq_info->final_ipp.sync_info.cgain[1]
				, iq_info->final_ipp.sync_info.cgain[2], iq_info->final_ipp.sync_info.ct);
		} else {
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->shdr_long_id, ISP_SYNC_SEL_IPP, ISP_SYNC_AE_STATUS, &(iq_info->final_ipp.sync_info.ae_status));
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->shdr_long_id, ISP_SYNC_SEL_IPP, ISP_SYNC_AE_TOTAL_GAIN, &(iq_info->final_ipp.sync_info.gain));
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->shdr_long_id, ISP_SYNC_SEL_IPP, ISP_SYNC_AE_DGAIN, &(iq_info->final_ipp.sync_info.dgain));
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->shdr_long_id, ISP_SYNC_SEL_IPP, ISP_SYNC_AE_LV, &(iq_info->final_ipp.sync_info.lv));
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->shdr_long_id, ISP_SYNC_SEL_IPP, ISP_SYNC_AE_LV_BASE, &(iq_info->final_ipp.sync_info.lv_base));
			// NOTE: SHDR
			if ((iq_info->ipp_flow_mode == IQ_FLOW_SHDR) && (iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_SHDR)) {
				// SHDR sync info
				ae_sync_rt |= isp_dev_get_sync_item(iq_info->shdr_long_id, ISP_SYNC_SEL_IPP, ISP_SYNC_AE_SHDR_TM_RATIO, &(iq_info->final_ipp.sync_info.shdr_tm_ratio));
				ae_sync_rt |= isp_dev_get_sync_item(iq_info->shdr_long_id, ISP_SYNC_SEL_IPP, ISP_SYNC_AE_SHDR_EV_RATIO, &iq_info->final_ipp.sync_info.shdr_ev_ratio[0]);
				ae_sync_rt |= isp_dev_get_sync_item(iq_info->shdr_long_id, ISP_SYNC_SEL_IPP, ISP_SYNC_AE_SHDR_HBS, &(iq_info->final_ipp.sync_info.shdr_hbs_param));
			}

			awb_sync_rt |= isp_dev_get_sync_item(iq_info->shdr_long_id, ISP_SYNC_SEL_IPP, ISP_SYNC_IQ_FINAL_CGAIN, &(iq_info->final_ipp.sync_info.cgain[0]));
			awb_sync_rt |= isp_dev_get_sync_item(iq_info->shdr_long_id, ISP_SYNC_SEL_IPP, ISP_SYNC_AWB_CT, &(iq_info->final_ipp.sync_info.ct));
		}
		if (ae_sync_rt != E_OK) {
			PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "IPP(%d) trig - AE status not update!! \r\n", iq_info->id);
		}
		if (awb_sync_rt != E_OK) {
			PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "IPP(%d) trig - AWB status not update!! \r\n", iq_info->id);
		}
		if (test_shdr_en[iq_info->id] == TRUE) {
			iq_info->final_ipp.sync_info.shdr_ev_ratio[1] = test_ev_ratio[iq_info->id][1];
			iq_info->final_ipp.sync_info.shdr_tm_ratio = test_tm_ratio[iq_info->id];
		}
		if (test_shdr_hbs_en[iq_info->id] == TRUE) {
			iq_info->final_ipp.sync_info.shdr_hbs_param.lum_th = test_hbs[iq_info->id].lum_th;
			iq_info->final_ipp.sync_info.shdr_hbs_param.w_start = test_hbs[iq_info->id].w_start;
			iq_info->final_ipp.sync_info.shdr_hbs_param.w_slope = test_hbs[iq_info->id].w_slope;
		}

		enc_sync_rt |= isp_api_get_enc_isp_ratio(iq_info->id, &(iq_info->enc_isp_ratio));
		if (enc_sync_rt != E_OK) {
			PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "IPP(%d) trig - ENC ISP RATIO not update!! \r\n", iq_info->id);
		}

		if (iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_WDR) {
			histo_rslt = isp_dev_get_histo(iq_info->id);
			if (histo_rslt != NULL) {
				memcpy(iq_info->hist_stcs_pre_wdr, histo_rslt->hist_stcs_pre_wdr, sizeof(UINT16) * IQ_HISTO_MAX_SIZE);
				memcpy(iq_info->hist_stcs_post_wdr, histo_rslt->hist_stcs_post_wdr, sizeof(UINT16) * IQ_HISTO_MAX_SIZE);
			} else {
				PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "IPP(%d) trig - histo_stcs not update!! \r\n", iq_info->id);
			}
		}

		if (iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_DEFOG) {
			defog_stcs = isp_dev_get_defog_stcs(iq_info->id);
			if ((defog_stcs != NULL) && (defog_stcs->dfg_airlight[0] != 0) && (defog_stcs->dfg_airlight[1] != 0) && (defog_stcs->dfg_airlight[2] != 0)) {
				iq_info->dfg_airlight[0] = defog_stcs->dfg_airlight[0];
				iq_info->dfg_airlight[1] = defog_stcs->dfg_airlight[1];
				iq_info->dfg_airlight[2] = defog_stcs->dfg_airlight[2];
			} else {
				PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "IPP(%d) trig - defog_stcs not update!! \r\n", iq_info->id);
			}
			subout_buf = isp_dev_get_ipe_subout_buf(iq_info->id);
			if (subout_buf != NULL) {
				memcpy(iq_info->dfg_subout_min, subout_buf->min, sizeof(UINT16) * IQ_SUBOUT_MAX_SIZE);
				memcpy(iq_info->dfg_subout_avg, subout_buf->avg, sizeof(UINT16) * IQ_SUBOUT_MAX_SIZE);
			} else {
				PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "IPP(%d) trig - subout not update!! \r\n", iq_info->id);
			}
		}

		// NOTE: Set VA enable
		if (!(iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_AF)) {
			iq_info->final_ipp.ife_va.enable = FALSE;
			iq_info->final_ipp.ife_va.indep_va_enable = FALSE;
			iq_info->final_ipp.ipe_va.enable = FALSE;
			iq_info->final_ipp.ipe_va.indep_va_enable = FALSE;
		}
		if (!(iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_WDR)) {
			iq_info->final_ipp.ife_hist.hist_enable = FALSE;
		}

		iq_flow_set_update(msg, iq_info, iq_param[iq_info->id]);
		iq_flow_set_sync_param(msg, iq_info, iq_param[iq_info->id]);
		iq_operation_cgain(msg, iq_info, iq_param[iq_info->id]);
		iq_operation_iso(msg, iq_info, iq_param[iq_info->id]);
		if (iq_info->ipp_trig_obj.data_fmt == ISP_SEN_DATA_FMT_RGBIR) {
			iq_operation_rgbir(msg, iq_info, iq_param[iq_info->id]);
		}

		iq_operation_color(iq_info, iq_param[iq_info->id], TRUE);
		iq_operation_tonecurve(iq_info, iq_param[iq_info->id], (iq_info->ipp_tab_update & IQ_TBL_IPE_GAMMA));
		iq_operation_gamma(iq_info, iq_param[iq_info->id], (iq_info->ipp_tab_update & IQ_TBL_IPE_GAMMA));
		iq_operation_effect(msg, iq_info, iq_param[iq_info->id], TRUE);
		iq_operation_nnsc(msg, iq_info, iq_param[iq_info->id], TRUE);
		iq_operation_smooth(msg, iq_info, iq_param[iq_info->id], TRUE);

		if ((iq_info->ipp_trig_obj.flow_type & ISP_FLOW_CCIR) || (iq_info->ipp_trig_obj.mode_type == ISP_SEN_MODE_THERMAL)) {
			iq_info->final_ipp.ife_subisp.subisp_cfa_en = FALSE;
			iq_info->final_ipp.ipe_cfa.cfa_enable = FALSE;
		} else {
			iq_info->final_ipp.ife_subisp.subisp_cfa_en = cfa_en[iq_info->id];
			iq_info->final_ipp.ipe_cfa.cfa_enable = cfa_en[iq_info->id];
		}

		iq_info->final_ipp.ready_flag = TRUE;

		iq_flow_reset_ipp_setting(iq_info);
		iq_flow_reset_ipp_param(iq_info);
		rt |= iq_flow_set_ipp_setting(iq_info);
		rt |= iq_flow_set_ife_param(iq_info);
		rt |= iq_flow_set_ipe_param(iq_info);
		rt |= iq_flow_set_ime_param(iq_info);
		iq_msg_ife(iq_info);
		iq_msg_ipe(iq_info);
		iq_msg_ime(iq_info);
		iq_dbg_clr_dbg_mode(iq_info->id, IQ_DBG_A_WDR | IQ_DBG_A_DEFOG | IQ_DBG_A_SHDR);

		iq_info->ipp_tab_update &= ~(IQ_TBL_IPE_GAMMA | IQ_TBL_IPE_YCURVE);

		t1 = hwclock_get_counter();
		PRINT_IQ(dbg_mode & IQ_DBG_PERFORMANCE, "IPP time (%d)!! \r\n", t1 - t0);
		break;

	case ISP_TRIG_IQ_ENC:
		if (iq_info->final_enc.ready_flag == TRUE) {
			iq_info->final_enc.ready_flag = FALSE;
		} else {
			DBG_MSG("ENC process missmatch!! \r\n");
		}

		ae_sync_rt |= isp_dev_get_sync_item(iq_info->shdr_long_id, ISP_SYNC_SEL_ENC, ISP_SYNC_AE_TOTAL_GAIN, &(iq_info->final_enc.sync_info.gain));
		ae_sync_rt |= isp_dev_get_sync_item(iq_info->shdr_long_id, ISP_SYNC_SEL_ENC, ISP_SYNC_AE_DGAIN, &(iq_info->final_enc.sync_info.dgain));
		ae_sync_rt |= isp_dev_get_sync_item(iq_info->shdr_long_id, ISP_SYNC_SEL_ENC, ISP_SYNC_AE_LV, &(iq_info->final_enc.sync_info.lv));
		ae_sync_rt |= isp_dev_get_sync_item(iq_info->shdr_long_id, ISP_SYNC_SEL_ENC, ISP_SYNC_AE_LV_BASE, &(iq_info->final_enc.sync_info.lv_base));
		// NOTE: SHDR
		if ((iq_info->ipp_flow_mode == IQ_FLOW_SHDR) && (iq_info->ipp_trig_obj.func_en & ISP_FUNC_EN_SHDR)) {
			// SHDR sync info
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->shdr_long_id, ISP_SYNC_SEL_ENC, ISP_SYNC_AE_SHDR_TM_RATIO, &(iq_info->final_enc.sync_info.shdr_tm_ratio));
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->shdr_long_id, ISP_SYNC_SEL_ENC, ISP_SYNC_AE_SHDR_EV_RATIO, &iq_info->final_enc.sync_info.shdr_ev_ratio[0]);
			ae_sync_rt |= isp_dev_get_sync_item(iq_info->shdr_long_id, ISP_SYNC_SEL_ENC, ISP_SYNC_AE_SHDR_HBS, &(iq_info->final_enc.sync_info.shdr_hbs_param));
		}

		awb_sync_rt |= isp_dev_get_sync_item(iq_info->shdr_long_id, ISP_SYNC_SEL_ENC, ISP_SYNC_IQ_FINAL_CGAIN, &(iq_info->final_enc.sync_info.cgain[0]));
		awb_sync_rt |= isp_dev_get_sync_item(iq_info->shdr_long_id, ISP_SYNC_SEL_ENC, ISP_SYNC_AWB_CT, &(iq_info->final_enc.sync_info.ct));

		if (ae_sync_rt != E_OK) {
			PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "ENC(%d) trig - AE status not update!! \r\n", iq_info->id);
		}
		if (awb_sync_rt != E_OK) {
			PRINT_IQ(dbg_mode & IQ_DBG_SYNC, "ENC(%d) trig - AWB status not update!! \r\n", iq_info->id);
		}

		iq_flow_set_update(msg, iq_info, iq_param[iq_info->id]);
		iq_operation_iso(msg, iq_info, iq_param[iq_info->id]);

		iq_info->final_enc.ready_flag = TRUE;

		iq_flow_reset_enc_param(iq_info);
		rt |= iq_flow_set_enc_param(iq_info);
		iq_msg_enc(iq_info);
		break;

	default:
		PRINT_IQ_WRN(dbg_mode & IQ_DBG_WRN_MSG, "iq_flow_process get unknow msg(%d) \r\n", msg);
		break;
	}
	return rt;
}
