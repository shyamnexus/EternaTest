#include "kdrv_builtin.h"
#include "bridge.h"
#include "nvtmpp_init.h"
#include "kwrap/type.h"
#include "kwrap/file.h"
#include "kwrap/perf.h"
#include "kwrap/cpu.h"
#include "kwrap/util.h"

#include "ime_lib.h"
#include "kdrv_ipp_builtin.h"
#include "kdrv_ipp_builtin_int.h"
#include "sie_init.h"

//#include <mach/nvt-io.h>
#include <plat/nvt-sramctl.h>
#include <nvt_api_ver.h>

#if defined(__KERNEL__)
#include <plat/hardware.h>

extern NVT_API_CHK_DECLARE(kdrv_ipp_builtin_chk);

#define IPP_LL_START_TIME    0x54534C4C //MAKEFOURCC('L', 'L', 'S', 'T');
#define NVT_TIMER_TM0_CNT    0x108

#define KDRV_IPP_BUILTIN_NEW_BIND_FLOW (DISABLE)
#define LOAD_CTRL (0x20)  // 0x10:fram start load,  0x20:global load

#define IFE_FB_INT_STS_OFS 					0xC
#define IFE_FB_INT_R_DEC1_ERR_BIT 			0x2
#define IFE_FB_INT_R_DEC2_ERR_BIT 			0x4
#define IFE_FB_INT_RINGBUF_ERR_BIT 			0x100
#define IFE_FB_INT_R_FRAME_ERR_BIT 			0x200
#define IFE_FB_INT_ERR_CHK_BIT 				(IFE_FB_INT_R_DEC1_ERR_BIT | IFE_FB_INT_R_DEC2_ERR_BIT | IFE_FB_INT_RINGBUF_ERR_BIT | IFE_FB_INT_R_FRAME_ERR_BIT)

extern VOID ime_base_reg_init(VOID);
extern void nvt_bootts_add_ts(char *name);
extern UINT32 (*kdrv_ipp_builtin_get_reg_num)(KDRV_IPP_BUILTIN_ENG eng);

/* isp callback register before init */
static KDRV_IPP_BUILTIN_ISP_CB g_kdrv_ipp_isp_cb = NULL;
static KDRV_IPP_BUILTIN_CTL g_kdrv_ipp_builtin_ctl = {0};

void kdrv_ipp_builtin_dbg_info(void *p_data){

	KDRV_IPP_BUILTIN_DBG_INFO *dbg_info = (KDRV_IPP_BUILTIN_DBG_INFO *) p_data;

	UINT32 i=0;

	if(dbg_info == NULL){
		DBG_ERR("null input\n");
		return;
	}

	for(i=0; i < g_kdrv_ipp_builtin_ctl.hdl_num; i++){

		if(g_kdrv_ipp_builtin_ctl.p_hdl == NULL){
			DBG_ERR("null hdl\n");
			return;
		}

		dbg_info[i] = g_kdrv_ipp_builtin_ctl.p_hdl[i].dbg_info;
		dbg_info[i].frc_info = g_kdrv_ipp_builtin_ctl.p_hdl[i].frc;
	}

	return;
}

_INLINE BOOL kdrv_ipp_builtin_dtsi_ver(void)
{
	return g_kdrv_ipp_builtin_ctl.dtsi_ver;
}

_INLINE UINT32* kdrv_ipp_builtin_get_dtsi_buf(KDRV_IPP_BUILTIN_HDL* p_hdl, KDRV_IPP_BUILTIN_ENG eng)
{
	if(eng == KDRV_IPP_BUILTIN_IFE)
		return p_hdl->p_dtsi_ife_buf;
	else if(eng == KDRV_IPP_BUILTIN_IPE)
		return p_hdl->p_dtsi_ipe_buf;
	else if(eng == KDRV_IPP_BUILTIN_IME)
		return p_hdl->p_dtsi_ime_buf;
	else{
		DBG_ERR("unknown get dtsi buf\n");
		return NULL;
	}
}

_INLINE UINT32 kdrv_ipp_builtin_get_reg(UINT32 eng, UINT32 ofs)
{
	return *(UINT32 *)((ULONG)g_kdrv_ipp_builtin_ctl.reg_base[eng] + ofs);
}

_INLINE void kdrv_ipp_builtin_set_reg(UINT32 eng, UINT32 ofs, UINT32 val)
{
	*(UINT32 *)((ULONG)g_kdrv_ipp_builtin_ctl.reg_base[eng] + ofs) = val;
}

void kdrv_ipp_builtin_save_queue_blk_info(KDRV_IPP_BUILTIN_HDL *p_hdl)
{

	UINT32 i = 0;
	KDRV_IPP_BUILTIN_JOB *p_job = (KDRV_IPP_BUILTIN_JOB *)p_hdl->p_cur_job;

	for(i=0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++){

		p_hdl->vprc_blk[i][p_hdl->queue_out_idx[i]].frm_cnt = p_hdl->fed_cnt; // tmp check

		if(p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW)
			p_hdl->vprc_blk[i][p_hdl->queue_out_idx[i]].timestamp = p_hdl->fs_timestamp;
		else
			p_hdl->vprc_blk[i][p_hdl->queue_out_idx[i]].timestamp = p_job->timestamp;
		//yuv queue debug info
		//DBG_DUMP("-(%d,%d,%d,%d)-\n", i , p_hdl->queue_out_idx[i] , p_hdl->vprc_blk[i][p_hdl->queue_out_idx[i]].frm_cnt, p_hdl->vprc_blk[i][p_hdl->queue_out_idx[i]].timestamp);

		/*
		if(p_hdl->queue_out_idx[i] > 1 ){
			if((p_hdl->vprc_blk[i][p_hdl->queue_out_idx[i]].timestamp - p_hdl->vprc_blk[i][p_hdl->queue_out_idx[i]-1].timestamp) > 36000)
				DBG_DUMP("--------------------idx %d, drop : %d, %d----------------\n", p_hdl->queue_out_idx[i], p_hdl->vprc_blk[i][p_hdl->queue_out_idx[i]].timestamp , p_hdl->vprc_blk[i][p_hdl->queue_out_idx[i]-1].timestamp  );
		}*/
	}

	return;
}

INT32 kdrv_ipp_builtin_set(void *p_hdl, UINT32 param_id, void *p_data)
{
	INT32 ret= 0;

	switch(param_id){

	case KDRV_IPP_PARAM_BUILTIN_SET_ALGO_BYPASS_EN:
	{
		BOOL *en = (BOOL *) p_data;

		KDRV_IPP_BUILTIN_DUMP("algo bypass en:%d",(UINT32) *en);

		*en = *en & 0x01; //bit mask

		if(*en == TRUE)
			g_kdrv_ipp_builtin_ctl.p_hdl[0].dbg_func_en |= KDRV_IPP_BUILTIN_FUNC_ALGO_BY_PASS;
		else
			g_kdrv_ipp_builtin_ctl.p_hdl[0].dbg_func_en &= ~(KDRV_IPP_BUILTIN_FUNC_ALGO_BY_PASS);

	}
		break;
	case KDRV_IPP_PARAM_BUILTIN_SET_STAMP_MODE_EN:
	{
		BOOL *en = (BOOL *) p_data;

		KDRV_IPP_BUILTIN_DUMP("stamp mode en:%d",(UINT32) *en);

		*en = *en & 0x01; //bit mask

		if(*en == TRUE)
			g_kdrv_ipp_builtin_ctl.p_hdl[0].dbg_func_en |= KDRV_IPP_BUILTIN_FUNC_STAMP_MODE;
		else
			g_kdrv_ipp_builtin_ctl.p_hdl[0].dbg_func_en &= ~(KDRV_IPP_BUILTIN_FUNC_STAMP_MODE);

	}
		break;
	case KDRV_IPP_PARAM_BUILTIN_SET_PAUSE_IPP_OUTPUT:

		kdrv_ipp_builtin_dbg(&g_kdrv_ipp_builtin_ctl, DBG_STREAM_INPUT_SWITCH, p_data);

		break;
	case KDRV_IPP_PARAM_BUILTIN_SET_DBG_FUNC_EN:
	{
		KDRV_IPP_BUILTIN_DBG_FUNC_STRC *param = (KDRV_IPP_BUILTIN_DBG_FUNC_STRC *) p_data;

		KDRV_IPP_BUILTIN_DUMP("dbg_func_en id:%d, func:%d, en:%d",(UINT32) param->id,(UINT32) param->dbg_func_en, (UINT32) param->en);

		if(param->id >= g_kdrv_ipp_builtin_ctl.hdl_num){
			DBG_ERR("id > hdl_num support\n");
			return -1;
		}

		param->en = param->en & 0x01; //bit mask

		if(param->en == TRUE)
			g_kdrv_ipp_builtin_ctl.p_hdl[param->id].dbg_func_en |= param->dbg_func_en;
		else
			g_kdrv_ipp_builtin_ctl.p_hdl[param->id].dbg_func_en &= ~(param->dbg_func_en);

	}
		break;

	default:
		KDRV_IPP_BUILTIN_DUMP("unknown ipp builtin get case\r\n");
		break;
	}

	return ret;
}

INT32 kdrv_ipp_builtin_get(void *p_hdl, UINT32 param_id, void *p_data)
{
	INT32 ret = 0;

	switch(param_id){

	case KDRV_IPP_PARAM_BUILTIN_GET_DBGINFO:
		kdrv_ipp_builtin_dbg_info(p_data);
		break;
	case KDRV_IPP_PARAM_BUILTIN_GET_DTSI_CHKSUM:
	{
		UINT32 i=0;
		KDRV_IPP_BUILTIN_DBG_DTSI_CHKSUM* info = (KDRV_IPP_BUILTIN_DBG_DTSI_CHKSUM *)p_data;

		for(i=0; i < g_kdrv_ipp_builtin_ctl.hdl_num; i++){
			if(info->id == g_kdrv_ipp_builtin_ctl.p_hdl[i].isp_id)
				break;
		}

		if(i < g_kdrv_ipp_builtin_ctl.hdl_num )
			info->chksum = g_kdrv_ipp_builtin_ctl.p_hdl[i].dbg_info.dtsi_chksum;
		else{
			info->chksum = 0;
			DBG_ERR("get dtsi chksum id error:%d\n",info->id);
		}

		break;
	}
	case KDRV_IPP_PARAM_BUILTIN_GET_IPP_INFO:
	{
		UINT32 i=0;
		KDRV_IPP_BUILTIN_IPP_INFO* info = (KDRV_IPP_BUILTIN_IPP_INFO *)p_data;

		for(i=0; i < g_kdrv_ipp_builtin_ctl.hdl_num; i++){
			if(info->isp_id == g_kdrv_ipp_builtin_ctl.p_hdl[i].isp_id)
				break;
		}

		if(i < g_kdrv_ipp_builtin_ctl.hdl_num )
			info->func_en = g_kdrv_ipp_builtin_ctl.p_hdl[i].func_en;
		else{
			info->func_en = 0;
			DBG_ERR("get isp info error, isp_id:%d\n",info->isp_id);
		}

		break;
	}
	case KDRV_IPP_PARAM_BUILTIN_GET_QUEUE_INFO:
	{
		UINT32 i=0;
		KDRV_IPP_BUILTIN_QUEUE_GET_INFO* info = (KDRV_IPP_BUILTIN_QUEUE_GET_INFO *)p_data;

		for(i=0; i < g_kdrv_ipp_builtin_ctl.hdl_num; i++){
			if(info->isp_id == g_kdrv_ipp_builtin_ctl.p_hdl[i].isp_id)
				break;
		}

		if(i < g_kdrv_ipp_builtin_ctl.hdl_num ){
			UINT32 j = 0, k = 0;

			for(j=0; j < KDRV_IPP_BUILTIN_PATH_ID_MAX; j++){
				for(k=0; k < KDRV_IPP_BUILTIN_BUF_NUM_MAX; k++){

					info->queue_info.queued_frm[j][k].va = g_kdrv_ipp_builtin_ctl.p_hdl[i].vprc_blk[j][k].addr;
					info->queue_info.queued_frm[j][k].frm_cnt = g_kdrv_ipp_builtin_ctl.p_hdl[i].vprc_blk[j][k].frm_cnt;
					info->queue_info.queued_frm[j][k].timestamp = g_kdrv_ipp_builtin_ctl.p_hdl[i].vprc_blk[j][k].timestamp;

				}
			}

			for(j=0; j < KDRV_IPP_BUILTIN_PATH_ID_MAX; j++){
				info->queue_info.queued_num[j] = g_kdrv_ipp_builtin_ctl.p_hdl[i].buf_num_in_que[j];

				if (g_kdrv_ipp_builtin_ctl.p_hdl[i].queue_ring_mode_en) {
					if(g_kdrv_ipp_builtin_ctl.p_hdl[i].ring_flag[j] == 1)
						info->queue_info.queue_cur_idx[j] = g_kdrv_ipp_builtin_ctl.p_hdl[i].queue_out_idx[j];
					else
						info->queue_info.queue_cur_idx[j] = 0;
				} else {
					info->queue_info.queue_cur_idx[j] = 0; // non-ring mode always start from index 0
				}
				//yuv queue debug info
				//DBG_DUMP("------[%d]queued_num=%d, cur_idx=%d-------\n", i , info->queue_info.queued_num[j], info->queue_info.queue_cur_idx[j]);
				//DBG_DUMP("-xxxxxxxx[%d](%d,%d,0x%lx)xxxxxxxx-\n", info->queue_info.queue_cur_idx[0], info->queue_info.queued_frm[0][info->queue_info.queue_cur_idx[0]].frm_cnt, info->queue_info.queued_frm[0][info->queue_info.queue_cur_idx[0]].timestamp, info->queue_info.queued_frm[j][info->queue_info.queue_cur_idx[j]].va);
			}
		}
		else{
			DBG_ERR("get isp info error, isp_id:%d\n",info->isp_id);
		}

		break;
	}
	default:
		KDRV_IPP_BUILTIN_DUMP("unknown ipp builtin get case\r\n");
		break;
	}

	return ret;
}

static void kdrv_ipp_builtin_clr_ime_dram_chk_status(UINT32 clr_status)
{
	kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_DRAM_END_OUTPUT_STATUS_REG1, clr_status);

	return;
}

static UINT32 kdrv_ipp_builtin_get_ime_dram_chk_status(void)
{
	UINT32 ime_dram_status;

	ime_dram_status = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_DRAM_END_OUTPUT_STATUS_REG1);

	return ime_dram_status;
}
static BOOL kdrv_ipp_check_ime_dram_status(KDRV_IPP_BUILTIN_HDL *p_hdl)
{
	BOOL ret=0;
	UINT32 ime_dram_status;
	UINT8 idx = p_hdl->ime_dram_end_ck.r_idx;

	ime_dram_status = kdrv_ipp_builtin_get_ime_dram_chk_status();
	ime_dram_status &= ( 1 << p_hdl->ime_dram_end_ck.expected_bit[idx]);

	if(ime_dram_status){

		kdrv_ipp_builtin_clr_ime_dram_chk_status(ime_dram_status);
		ret = TRUE;

	}else{
		ret = FALSE;
	}

	if(p_hdl->ime_dram_end_ck.r_idx == 0) //update index
		p_hdl->ime_dram_end_ck.r_idx = 1;
	else
		p_hdl->ime_dram_end_ck.r_idx = 0;

	if(p_hdl->ime_dram_end_ck.r_idx == p_hdl->ime_dram_end_ck.w_idx) //error case when read and write index overlap
		p_hdl->dbg_info.intr_drop_cnt++;

	return ret;
}

static void kdrv_ipp_builtin_update_ime_dram_chk_bit(KDRV_IPP_BUILTIN_HDL *p_hdl)
{
	UINT32 ime_dram_chk_ctrl=0;
	UINT8 idx = p_hdl->ime_dram_end_ck.w_idx;

	ime_dram_chk_ctrl = p_hdl->ime_dram_end_ck.set_bit << 4; //chkbit
	ime_dram_chk_ctrl |= 0x1; //enable

	kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_DRAM_END_OUTPUT_STATUS_REG0, ime_dram_chk_ctrl);

	p_hdl->ime_dram_end_ck.expected_bit[idx] = p_hdl->ime_dram_end_ck.set_bit;

	p_hdl->last_out_info.ime_dram_end_status = p_hdl->ime_dram_end_ck.set_bit; //record last value for switch to hdal

	if(p_hdl->ime_dram_end_ck.w_idx == 0) //update idx
		p_hdl->ime_dram_end_ck.w_idx = 1;
	else
		p_hdl->ime_dram_end_ck.w_idx = 0;

	p_hdl->ime_dram_end_ck.set_bit ++;

	if(p_hdl->ime_dram_end_ck.set_bit >=32)
		p_hdl->ime_dram_end_ck.set_bit = 0;

	return;
}


_INLINE UINT32 kdrv_ipp_builtin_yuv_size(KDRV_IPP_BUILTIN_IMG_INFO *p_path_info)
{
	UINT32 size = 0;

	switch (p_path_info->fmt) {
	case KDRV_IPP_BUILTIN_FMT_Y8:
		size = p_path_info->loff[0] * p_path_info->size.h;
		break;

	case KDRV_IPP_BUILTIN_FMT_YUV420:
	case KDRV_IPP_BUILTIN_FMT_YUV420_PLANAR:
	case KDRV_IPP_BUILTIN_FMT_NVX2:
		size = p_path_info->loff[0] * p_path_info->size.h;
		size += p_path_info->loff[1] * (p_path_info->size.h >> 1);
		break;
	case KDRV_IPP_BUILTIN_FMT_YUV420_MB4:
	case KDRV_IPP_BUILTIN_FMT_YUV420_NVX5:
		DBG_ERR("not support\n");
		break;
	}

	return size;
}

_INLINE UINT32 kdrv_ipp_builtin_y_size(KDRV_IPP_BUILTIN_IMG_INFO *p_path_info)
{
	return p_path_info->loff[0] * p_path_info->size.h;
}

_INLINE UINT32 kdrv_ipp_builtin_uv_size(KDRV_IPP_BUILTIN_IMG_INFO *p_path_info)
{
	UINT32 size = 0;

	switch (p_path_info->fmt) {
	case KDRV_IPP_BUILTIN_FMT_YUV420_PLANAR:
		size = p_path_info->loff[1] * (p_path_info->size.h >> 1);
		break;

	case KDRV_IPP_BUILTIN_FMT_YUV420:
	case KDRV_IPP_BUILTIN_FMT_NVX2:
		size = p_path_info->loff[1] * (p_path_info->size.h >> 1);
		break;

	case KDRV_IPP_BUILTIN_FMT_Y8:
		break;
	case KDRV_IPP_BUILTIN_FMT_YUV420_MB4:
	case KDRV_IPP_BUILTIN_FMT_YUV420_NVX5:
		DBG_ERR("not support\n");
		break;
	}

	return size;
}

_INLINE KDRV_IPP_BUILTIN_FMT kdrv_ipp_builtin_typecast_fmt(KDRV_IPP_BUILTIN_PATH_ID pid, IME_GET_OUTPATH_INFO *path_info)
{
	if (pid < 4) {
		if (pid == 0) {
			/* IME_PE_ENC_EN, bit26 */
			if (kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, 0x04) & (0x4000000)) {
				return KDRV_IPP_BUILTIN_FMT_NVX2;
			}
		}
		if (path_info->out_path_image_format.out_format_sel == IME_OUTPUT_YCC_420) {
			if (path_info->out_path_image_format.out_format_type_sel == IME_OUTPUT_YCC_PLANAR) {
				return KDRV_IPP_BUILTIN_FMT_YUV420_PLANAR;
			} else {
				return KDRV_IPP_BUILTIN_FMT_YUV420;
			}
		} else {
			return KDRV_IPP_BUILTIN_FMT_Y8;
		}
	} else if (pid == 4) {
		/* IME_3DNR_REG_OUT_ENC_EN, bit31 */
		if (kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, 0x04) & (0x80000000)) {
			return KDRV_IPP_BUILTIN_FMT_NVX2;
		} else {
			return KDRV_IPP_BUILTIN_FMT_YUV420;
		}
	} else if (pid == 5) {
		return KDRV_IPP_BUILTIN_FMT_Y8;
	}

	return 0;
}
/*static UINT32 kdrv_ipp_builtin_get_ime_out_flip(UINT32 pid)
{
	UINT32 reg_val;

	reg_val = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, 0x08);

	return (reg_val & (0x800 << pid));
}*/

static void kdrv_ipp_builtin_set_ime_out_addr(KDRV_IPP_BUILTIN_HDL *p_hdl, UINT32 pid, KDRV_IPP_BUILTIN_IMG_INFO *p_info)
{
	UINT32 eng = KDRV_IPP_BUILTIN_IME;
	UINT32 addr[3] = {0};

	addr[0] = p_info->phyaddr[0];
	addr[1] = p_info->phyaddr[1];
	addr[2] = p_info->phyaddr[2];

	if (p_hdl->flip[pid] == 1) {
		addr[0] = addr[0] + (kdrv_ipp_builtin_y_size(p_info) - p_info->loff[0]);
		addr[1] = addr[1] + (kdrv_ipp_builtin_uv_size(p_info) - p_info->loff[1]);
		addr[2] = addr[2] + (kdrv_ipp_builtin_uv_size(p_info) - p_info->loff[1]);
	}

	switch (pid) {
	case 0:
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_P0_Y_DRAM_SAO, addr[0]);
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_P0_U_DRAM_SAO, addr[1]);
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_P0_V_DRAM_SAO, addr[2]);
		break;

	case 1:
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_P1_Y_DRAM_SAO, addr[0]);
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_P1_UV_DRAM_SAO, addr[1]);
		break;

	case 2:
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_P2_Y_DRAM_SAO, addr[0]);
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_P2_UV_DRAM_SAO, addr[1]);
		break;

	case 3:

		break;
	case 4:
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_REF_DRAM_Y_SAO, addr[0]);
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_REF_DRAM_UV_SAO, addr[1]);
		break;

	default:
		break;
	}
}

void kdrv_ipp_builtin_update_timestamp(void)
{
	/* for direct mode hdal first frame start timestamp */
	KDRV_IPP_BUILTIN_CTL *p_ctl = &g_kdrv_ipp_builtin_ctl;

	if (p_ctl->p_trig_hdl) {
		vos_perf_mark(&p_ctl->p_trig_hdl->fs_timestamp);
	}
}

void kdrv_ipp_builtin_frc(KDRV_IPP_BUILTIN_HDL *p_hdl, KDRV_IPP_BUILTIN_FMD_CB_INFO *p_info, UINT32 id)
{

	KDRV_IPP_BUILTIN_IMG_INFO *p_path;
	KDRV_IPP_BUILTIN_RATE_CTL *p_frc;
	static UINT32 dbg_drop_cnt[KDRV_IPP_BUILTIN_PATH_ID_MAX]= {0};

	p_path = &p_info->out_img[id];
	p_frc = &p_hdl->frc[id];

	if (!p_path->enable) {
		return;
	}

	if (p_hdl->fs_cnt <= p_frc->skip) {
		/* skip frame */
		p_path->enable = DISABLE;
		dbg_drop_cnt[id]++;
		p_hdl->dbg_info.frc_drop_cnt[id]++;
	} else {
		/* frame rate ctrl */
		if (p_frc->src != p_frc->dst) {
			if ((p_hdl->fs_cnt - p_frc->skip) > p_frc->src &&
				((p_hdl->fs_cnt - p_frc->skip) % p_frc->src) == 0) {
				p_frc->rate_cnt = 0;
			}

			p_frc->rate_cnt += KDRV_IPP_BUILTIN_FRC_BASE;
			if (p_frc->rate_cnt >= p_frc->drop_rate) {
				p_path->enable = DISABLE;
				p_frc->rate_cnt -= p_frc->drop_rate;
				dbg_drop_cnt[id]++;
			}
		} else if (p_frc->dst == 0) {
			p_path->enable = DISABLE;
		}
	}


	if( ( (p_hdl->fs_cnt % 60) == 0) && (dbg_drop_cnt[id] != 0) ){  //calculate number of drop frm every 60 frm
		p_hdl->dbg_info.frc_drop_cnt[id]=dbg_drop_cnt[id];
		dbg_drop_cnt[id]=0;
	}
}

BOOL kdrv_ipp_builtin_frc_yuv_que(KDRV_IPP_BUILTIN_HDL *p_hdl, KDRV_IPP_BUILTIN_FMD_CB_INFO *p_info)
{
	UINT32 i;
	KDRV_IPP_BUILTIN_IMG_INFO *p_path;
	KDRV_IPP_BUILTIN_RATE_CTL *p_frc;
	UINT32 rt_drop = 0;

	for (i = 0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++) {
		p_path = &p_info->out_img[i];
		p_frc = &p_hdl->frc[i];

		if (!p_path->enable) {
			continue;
		}

		if (p_hdl->fs_cnt_yuv_que <= p_frc->skip) {
			/* skip frame */
			p_path->enable = DISABLE;
			rt_drop = 1;
		} else {
			/* frame rate ctrl */
			if (p_frc->src != p_frc->dst) {
				if ((p_hdl->fs_cnt_yuv_que - p_frc->skip) > p_frc->src &&
					((p_hdl->fs_cnt_yuv_que - p_frc->skip) % p_frc->src) == 0) {
					p_frc->rate_cnt_yuv_que = 0;
				}

				p_frc->rate_cnt_yuv_que += KDRV_IPP_BUILTIN_FRC_BASE;
				if (p_frc->rate_cnt_yuv_que >= p_frc->drop_rate) {
					p_path->enable = DISABLE;
					rt_drop = 1;
					p_frc->rate_cnt_yuv_que -= p_frc->drop_rate;
				}
			} else if (p_frc->dst == 0) {
				p_path->enable = DISABLE;
				rt_drop = 1;
			}
		}

	}

	return rt_drop;
}

VOID kdrv_ipp_builtin_set_privacy_mask_line0_reg(UINT32 set, INT32 coef_a, INT32 coef_b, INT32 coef_c, INT32 coef_d)
{
	UINT32 pm_reg = 0;
	UINT32 pm_reg2 = 0;

	pm_reg |= (coef_d) & 0x03; //comp
	pm_reg |= ( (((coef_a < 0) ? (-1 * coef_a) : coef_a)) << 4) & 0x1FFF0; // coef_a
	pm_reg |= ((((coef_a < 0) ? 1 : 0)) << 17) & 0x20000; //sign_a
	pm_reg |= ((((coef_b < 0) ? (-1 * coef_b) : coef_b)) << 18) & 0x7FFC0000; //coef_b
	pm_reg |= ((((coef_b < 0) ? 1 : 0)) << 31) & 0x80000000; //sign_b

	pm_reg2 |= (((coef_c < 0) ? (-1 * coef_c) : coef_c)) & 0x3FFFFFF; //coef_c
	pm_reg2 |= ((((coef_c < 0) ? 1 : 0)) << 26) & 0x4000000; //sign_c


	if(set == 0){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE0_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE0_REG2, pm_reg2);
	}else if(set == 1){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET1_LINE0_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET1_LINE0_REG2, pm_reg2);
	}else if(set == 2){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE0_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE0_REG2, pm_reg2);
	}else if(set == 3){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET3_LINE0_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET3_LINE0_REG2, pm_reg2);
	}else if(set == 4){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE0_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE0_REG2, pm_reg2);
	}else if(set == 5){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET5_LINE0_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET5_LINE0_REG2, pm_reg2);
	}else if(set == 6){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE0_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE0_REG2, pm_reg2);
	}else if(set == 7){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET7_LINE0_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET7_LINE0_REG2, pm_reg2);
	}else{
		DBG_ERR("-unknown pm set : %d-\n", set);
	}

	return;
}

VOID kdrv_ipp_builtin_set_privacy_mask_line1_reg(UINT32 set, INT32 coef_a, INT32 coef_b, INT32 coef_c, INT32 coef_d)
{
	UINT32 pm_reg = 0;
	UINT32 pm_reg2 = 0;

	pm_reg |= (coef_d) & 0x03; //comp
	pm_reg |= ( (((coef_a < 0) ? (-1 * coef_a) : coef_a)) << 4) & 0x1FFF0; // coef_a
	pm_reg |= ((((coef_a < 0) ? 1 : 0)) << 17) & 0x20000; //sign_a
	pm_reg |= ((((coef_b < 0) ? (-1 * coef_b) : coef_b)) << 18) & 0x7FFC0000; //coef_b
	pm_reg |= ((((coef_b < 0) ? 1 : 0)) << 31) & 0x80000000; //sign_b

	pm_reg2 |= (((coef_c < 0) ? (-1 * coef_c) : coef_c)) & 0x3FFFFFF; //coef_c
	pm_reg2 |= ((((coef_c < 0) ? 1 : 0)) << 26) & 0x4000000; //sign_c


	if(set == 0){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE1_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE1_REG2, pm_reg2);
	}else if(set == 1){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET1_LINE1_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET1_LINE1_REG2, pm_reg2);
	}else if(set == 2){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE1_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE1_REG2, pm_reg2);
	}else if(set == 3){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET3_LINE1_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET3_LINE1_REG2, pm_reg2);
	}else if(set == 4){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE1_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE1_REG2, pm_reg2);
	}else if(set == 5){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET5_LINE1_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET5_LINE1_REG2, pm_reg2);
	}else if(set == 6){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE1_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE1_REG2, pm_reg2);
	}else if(set == 7){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET7_LINE1_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET7_LINE1_REG2, pm_reg2);
	}else{
		DBG_ERR("-unknown pm set : %d-\n", set);
	}

	return;
}

VOID kdrv_ipp_builtin_set_privacy_mask_line2_reg(UINT32 set, INT32 coef_a, INT32 coef_b, INT32 coef_c, INT32 coef_d)
{
	UINT32 pm_reg = 0;
	UINT32 pm_reg2 = 0;

	pm_reg |= (coef_d) & 0x03; //comp
	pm_reg |= ( (((coef_a < 0) ? (-1 * coef_a) : coef_a)) << 4) & 0x1FFF0; // coef_a
	pm_reg |= ((((coef_a < 0) ? 1 : 0)) << 17) & 0x20000; //sign_a
	pm_reg |= ((((coef_b < 0) ? (-1 * coef_b) : coef_b)) << 18) & 0x7FFC0000; //coef_b
	pm_reg |= ((((coef_b < 0) ? 1 : 0)) << 31) & 0x80000000; //sign_b

	pm_reg2 |= (((coef_c < 0) ? (-1 * coef_c) : coef_c)) & 0x3FFFFFF; //coef_c
	pm_reg2 |= ((((coef_c < 0) ? 1 : 0)) << 26) & 0x4000000; //sign_c


	if(set == 0){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE2_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE2_REG2, pm_reg2);
	}else if(set == 1){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET1_LINE2_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET1_LINE2_REG2, pm_reg2);
	}else if(set == 2){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE2_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE2_REG2, pm_reg2);
	}else if(set == 3){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET3_LINE2_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET3_LINE2_REG2, pm_reg2);
	}else if(set == 4){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE2_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE2_REG2, pm_reg2);
	}else if(set == 5){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET5_LINE2_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET5_LINE2_REG2, pm_reg2);
	}else if(set == 6){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE2_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE2_REG2, pm_reg2);
	}else if(set == 7){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET7_LINE2_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET7_LINE2_REG2, pm_reg2);
	}else{
		DBG_ERR("-unknown pm set : %d-\n", set);
	}

	return;
}

VOID kdrv_ipp_builtin_set_privacy_mask_line3_reg(UINT32 set, INT32 coef_a, INT32 coef_b, INT32 coef_c, INT32 coef_d)
{
	UINT32 pm_reg = 0;
	UINT32 pm_reg2 = 0;

	pm_reg |= (coef_d) & 0x03; //comp
	pm_reg |= ( (((coef_a < 0) ? (-1 * coef_a) : coef_a)) << 4) & 0x1FFF0; // coef_a
	pm_reg |= ((((coef_a < 0) ? 1 : 0)) << 17) & 0x20000; //sign_a
	pm_reg |= ((((coef_b < 0) ? (-1 * coef_b) : coef_b)) << 18) & 0x7FFC0000; //coef_b
	pm_reg |= ((((coef_b < 0) ? 1 : 0)) << 31) & 0x80000000; //sign_b

	pm_reg2 |= (((coef_c < 0) ? (-1 * coef_c) : coef_c)) & 0x3FFFFFF; //coef_c
	pm_reg2 |= ((((coef_c < 0) ? 1 : 0)) << 26) & 0x4000000; //sign_c


	if(set == 0){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE3_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE3_REG2, pm_reg2);
	}else if(set == 1){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET1_LINE3_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET1_LINE3_REG2, pm_reg2);
	}else if(set == 2){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE3_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE3_REG2, pm_reg2);
	}else if(set == 3){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET3_LINE3_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET3_LINE3_REG2, pm_reg2);
	}else if(set == 4){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE3_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE3_REG2, pm_reg2);
	}else if(set == 5){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET5_LINE3_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET5_LINE3_REG2, pm_reg2);
	}else if(set == 6){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE3_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE3_REG2, pm_reg2);
	}else if(set == 7){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET7_LINE3_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET7_LINE3_REG2, pm_reg2);
	}else{
		DBG_ERR("-unknown pm set : %d-\n", set);
	}

	return;
}

VOID kdrv_ipp_builtin_set_privacy_mask_line4_reg(UINT32 set, INT32 coef_a, INT32 coef_b, INT32 coef_c, INT32 coef_d)
{
	UINT32 pm_reg = 0;
	UINT32 pm_reg2 = 0;

	pm_reg |= (coef_d) & 0x03; //comp
	pm_reg |= ( (((coef_a < 0) ? (-1 * coef_a) : coef_a)) << 4) & 0x1FFF0; // coef_a
	pm_reg |= ((((coef_a < 0) ? 1 : 0)) << 17) & 0x20000; //sign_a
	pm_reg |= ((((coef_b < 0) ? (-1 * coef_b) : coef_b)) << 18) & 0x7FFC0000; //coef_b
	pm_reg |= ((((coef_b < 0) ? 1 : 0)) << 31) & 0x80000000; //sign_b

	pm_reg2 |= (((coef_c < 0) ? (-1 * coef_c) : coef_c)) & 0x3FFFFFF; //coef_c
	pm_reg2 |= ((((coef_c < 0) ? 1 : 0)) << 26) & 0x4000000; //sign_c


	if(set == 0){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE4_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE4_REG2, pm_reg2);
	}else if(set == 1){

	}else if(set == 2){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE4_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE4_REG2, pm_reg2);
	}else if(set == 3){

	}else if(set == 4){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE4_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE4_REG2, pm_reg2);
	}else if(set == 5){

	}else if(set == 6){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE4_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE4_REG2, pm_reg2);
	}else if(set == 7){

	}else{
		DBG_ERR("-unknown pm set : %d-\n", set);
	}

	return;
}

VOID kdrv_ipp_builtin_set_privacy_mask_line5_reg(UINT32 set, INT32 coef_a, INT32 coef_b, INT32 coef_c, INT32 coef_d)
{
	UINT32 pm_reg = 0;
	UINT32 pm_reg2 = 0;

	pm_reg |= (coef_d) & 0x03; //comp
	pm_reg |= ( (((coef_a < 0) ? (-1 * coef_a) : coef_a)) << 4) & 0x1FFF0; // coef_a
	pm_reg |= ((((coef_a < 0) ? 1 : 0)) << 17) & 0x20000; //sign_a
	pm_reg |= ((((coef_b < 0) ? (-1 * coef_b) : coef_b)) << 18) & 0x7FFC0000; //coef_b
	pm_reg |= ((((coef_b < 0) ? 1 : 0)) << 31) & 0x80000000; //sign_b

	pm_reg2 |= (((coef_c < 0) ? (-1 * coef_c) : coef_c)) & 0x3FFFFFF; //coef_c
	pm_reg2 |= ((((coef_c < 0) ? 1 : 0)) << 26) & 0x4000000; //sign_c


	if(set == 0){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE5_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE5_REG2, pm_reg2);
	}else if(set == 1){

	}else if(set == 2){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE5_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE5_REG2, pm_reg2);
	}else if(set == 3){

	}else if(set == 4){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE5_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE5_REG2, pm_reg2);
	}else if(set == 5){

	}else if(set == 6){
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE5_REG1, pm_reg);
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE5_REG2, pm_reg2);
	}else if(set == 7){

	}else{
		DBG_ERR("-unknown pm set : %d-\n", set);
	}

	return;
}

VOID kdrv_ipp_builtin_set_privacy_mask_line0_buf_reg(UINT32 set_sel, INT32 *p_coefs)
{
	kdrv_ipp_builtin_set_privacy_mask_line0_reg(set_sel, p_coefs[0], p_coefs[1], p_coefs[2], p_coefs[3]);
}

VOID kdrv_ipp_builtin_set_privacy_mask_line1_buf_reg(UINT32  set_sel, INT32 *p_coefs)
{
	kdrv_ipp_builtin_set_privacy_mask_line1_reg(set_sel, p_coefs[0], p_coefs[1], p_coefs[2], p_coefs[3]);
}

VOID kdrv_ipp_builtin_set_privacy_mask_line2_buf_reg(UINT32 set_sel, INT32 *p_coefs)
{
	kdrv_ipp_builtin_set_privacy_mask_line2_reg(set_sel, p_coefs[0], p_coefs[1], p_coefs[2], p_coefs[3]);
}

VOID kdrv_ipp_builtin_set_privacy_mask_line3_buf_reg(UINT32  set_sel, INT32 *p_coefs)
{
	kdrv_ipp_builtin_set_privacy_mask_line3_reg(set_sel, p_coefs[0], p_coefs[1], p_coefs[2], p_coefs[3]);
}

VOID kdrv_ipp_builtin_set_privacy_mask_line4_buf_reg(UINT32 set_sel, INT32 *p_coefs)
{
	kdrv_ipp_builtin_set_privacy_mask_line4_reg(set_sel, p_coefs[0], p_coefs[1], p_coefs[2], p_coefs[3]);
}

VOID kdrv_ipp_builtin_set_privacy_mask_line5_buf_reg(UINT32 set_sel, INT32 *p_coefs)
{
	kdrv_ipp_builtin_set_privacy_mask_line5_reg(set_sel, p_coefs[0], p_coefs[1], p_coefs[2], p_coefs[3]);
}

static VOID kdrv_ipp_builtin_cal_linear_coef(IPOINT pnt_a, IPOINT pnt_b, IPOINT pnt_cent, KDRV_IPP_BUILTIN_PM_LINE_COEFS *p_coefs)
{
	// reserved
	INT32 x1 = 0, y1 = 0;
	INT32 x2 = 0, y2 = 0;
	INT32 check = 0;

	x1 = (INT32)pnt_a.x;
	y1 = (INT32)pnt_a.y;
	x2 = (INT32)pnt_b.x;
	y2 = (INT32)pnt_b.y;

	// 0: >=,  1: <=,  2: >,  3: <

	p_coefs->coefs[0] = (y2 - y1);
	p_coefs->coefs[1] = (x1 - x2);
	p_coefs->coefs[2] = (x1 * y2 - x2 * y1);

	check = (p_coefs->coefs[0] * pnt_cent.x) + (p_coefs->coefs[1] * pnt_cent.y);
	if (check >= p_coefs->coefs[2]) {
		p_coefs->coefs[3] = 0;
	} else if (check <= p_coefs->coefs[2]) {
		p_coefs->coefs[3] = 1;
	}
}



VOID kdrv_ipp_builtin_cal_convex_hull_coefs(IPOINT *p_pm_cvx_point, INT32 pnt_num, KDRV_IPP_BUILTIN_PM_LINE_COEFS *p_line_coefs)
{
	// reserved
	INT32 i = 0;
	INT32 sum = 0, cnt = 0;
	IPOINT Center = {0};

	sum = 0, cnt = 0;
	for (i = 0; i < pnt_num; i++) {
		sum += (p_pm_cvx_point[i].x);
		cnt += 1;
	}

	if (cnt)
		Center.x = (sum + (cnt >> 1)) / cnt;
	else
		DBG_ERR("zero cnt, causing DIVIDE_BY_ZERO\n");

	sum = 0, cnt = 0;
	for (i = 0; i < pnt_num; i++) {
		sum += (p_pm_cvx_point[i].y);
		cnt += 1;
	}
	if (cnt)
		Center.y = (sum + (cnt >> 1)) / cnt;
	else
		DBG_ERR("zero cnt, causing DIVIDE_BY_ZERO\n");


	for (i = 0; i < pnt_num; i++) {
		if (i == (pnt_num - 1)) {
			kdrv_ipp_builtin_cal_linear_coef(p_pm_cvx_point[i], p_pm_cvx_point[0], Center, &p_line_coefs[i]);
		} else {
			kdrv_ipp_builtin_cal_linear_coef(p_pm_cvx_point[i], p_pm_cvx_point[i + 1], Center, &p_line_coefs[i]);
		}

	}
}

static void kdrv_ipp_builtin_set_privacy_mask_set0_polygon_buf_reg(KDRV_IPP_BUILTIN_POLYGON_SEL set_sel)
{
	UINT32 pm_reg = 0;

	if (set_sel == KDRV_IPP_BUILTIN_POLYGON_HOLLOW) {

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE0_REG2);
		pm_reg &= ~(REG_VAL_IME_PM0_COMB_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_COMB_MODE_SEL_HOLLOW;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE0_REG2, pm_reg);

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0);
		pm_reg &= ~(REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_SHAPE_MODE_QUAD;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0, pm_reg);

	} else if (set_sel == KDRV_IPP_BUILTIN_POLYGON_4) {

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE0_REG2);
		pm_reg &= ~(REG_VAL_IME_PM0_COMB_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_COMB_MODE_SEL_DISABLE;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE0_REG2, pm_reg);

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0);
		pm_reg &= ~(REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_SHAPE_MODE_QUAD;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0, pm_reg);

	} else if (set_sel == KDRV_IPP_BUILTIN_POLYGON_6) {

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE0_REG2);
		pm_reg &= ~(REG_VAL_IME_PM0_COMB_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_COMB_MODE_SEL_DISABLE;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE0_REG2, pm_reg);

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0);
		pm_reg &= ~(REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_SHAPE_MODE_HEX;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0, pm_reg);


	} else if (set_sel == KDRV_IPP_BUILTIN_POLYGON_8) {

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE0_REG2);
		pm_reg &= ~(REG_VAL_IME_PM0_COMB_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_COMB_MODE_SEL_OCT;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE0_REG2, pm_reg);

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0);
		pm_reg &= ~(REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_SHAPE_MODE_QUAD;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0, pm_reg);

	} else if (set_sel == KDRV_IPP_BUILTIN_POLYGON_10) {

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE0_REG2);
		pm_reg &= ~(REG_VAL_IME_PM0_COMB_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_COMB_MODE_SEL_DEC;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET0_LINE0_REG2, pm_reg);

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0);
		pm_reg &= ~(REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_SHAPE_MODE_QUAD;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0, pm_reg);
	} else {
		//  do nothing...
	}
}

static void kdrv_ipp_builtin_set_privacy_mask_set2_polygon_buf_reg(KDRV_IPP_BUILTIN_POLYGON_SEL set_sel)
{
	UINT32 pm_reg = 0;

	if (set_sel == KDRV_IPP_BUILTIN_POLYGON_HOLLOW) {

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE0_REG2);
		pm_reg &= ~(REG_VAL_IME_PM0_COMB_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_COMB_MODE_SEL_HOLLOW;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE0_REG2, pm_reg);

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET2);
		pm_reg &= ~(REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_SHAPE_MODE_QUAD;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET2, pm_reg);

	} else if (set_sel == KDRV_IPP_BUILTIN_POLYGON_4) {

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE0_REG2);
		pm_reg &= ~(REG_VAL_IME_PM0_COMB_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_COMB_MODE_SEL_DISABLE;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE0_REG2, pm_reg);

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET2);
		pm_reg &= ~(REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_SHAPE_MODE_QUAD;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET2, pm_reg);

	} else if (set_sel == KDRV_IPP_BUILTIN_POLYGON_6) {

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE0_REG2);
		pm_reg &= ~(REG_VAL_IME_PM0_COMB_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_COMB_MODE_SEL_DISABLE;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE0_REG2, pm_reg);

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET2);
		pm_reg &= ~(REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_SHAPE_MODE_HEX;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET2, pm_reg);


	} else if (set_sel == KDRV_IPP_BUILTIN_POLYGON_8) {

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE0_REG2);
		pm_reg &= ~(REG_VAL_IME_PM0_COMB_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_COMB_MODE_SEL_OCT;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE0_REG2, pm_reg);

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET2);
		pm_reg &= ~(REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_SHAPE_MODE_QUAD;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET2, pm_reg);

	} else if (set_sel == KDRV_IPP_BUILTIN_POLYGON_10) {

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE0_REG2);
		pm_reg &= ~(REG_VAL_IME_PM0_COMB_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_COMB_MODE_SEL_DEC;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET2_LINE0_REG2, pm_reg);

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET2);
		pm_reg &= ~(REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_SHAPE_MODE_QUAD;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET2, pm_reg);
	} else {
		//  do nothing...
	}
}

static void kdrv_ipp_builtin_set_privacy_mask_set4_polygon_buf_reg(KDRV_IPP_BUILTIN_POLYGON_SEL set_sel)
{
	UINT32 pm_reg = 0;

	if (set_sel == KDRV_IPP_BUILTIN_POLYGON_HOLLOW) {

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE0_REG2);
		pm_reg &= ~(REG_VAL_IME_PM0_COMB_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_COMB_MODE_SEL_HOLLOW;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE0_REG2, pm_reg);

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET4);
		pm_reg &= ~(REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_SHAPE_MODE_QUAD;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET4, pm_reg);

	} else if (set_sel == KDRV_IPP_BUILTIN_POLYGON_4) {

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE0_REG2);
		pm_reg &= ~(REG_VAL_IME_PM0_COMB_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_COMB_MODE_SEL_DISABLE;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE0_REG2, pm_reg);

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET4);
		pm_reg &= ~(REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_SHAPE_MODE_QUAD;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET4, pm_reg);

	} else if (set_sel == KDRV_IPP_BUILTIN_POLYGON_6) {

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE0_REG2);
		pm_reg &= ~(REG_VAL_IME_PM0_COMB_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_COMB_MODE_SEL_DISABLE;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE0_REG2, pm_reg);

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET4);
		pm_reg &= ~(REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_SHAPE_MODE_HEX;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET4, pm_reg);


	} else if (set_sel == KDRV_IPP_BUILTIN_POLYGON_8) {

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE0_REG2);
		pm_reg &= ~(REG_VAL_IME_PM0_COMB_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_COMB_MODE_SEL_OCT;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE0_REG2, pm_reg);

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET4);
		pm_reg &= ~(REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_SHAPE_MODE_QUAD;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET4, pm_reg);

	} else if (set_sel == KDRV_IPP_BUILTIN_POLYGON_10) {

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE0_REG2);
		pm_reg &= ~(REG_VAL_IME_PM0_COMB_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_COMB_MODE_SEL_DEC;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET4_LINE0_REG2, pm_reg);

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET4);
		pm_reg &= ~(REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_SHAPE_MODE_QUAD;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET4, pm_reg);
	} else {
		//  do nothing...
	}
}

static void kdrv_ipp_builtin_set_privacy_mask_set6_polygon_buf_reg(KDRV_IPP_BUILTIN_POLYGON_SEL set_sel)
{
	UINT32 pm_reg = 0;

	if (set_sel == KDRV_IPP_BUILTIN_POLYGON_HOLLOW) {

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE0_REG2);
		pm_reg &= ~(REG_VAL_IME_PM0_COMB_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_COMB_MODE_SEL_HOLLOW;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE0_REG2, pm_reg);

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET6);
		pm_reg &= ~(REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_SHAPE_MODE_QUAD;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET6, pm_reg);

	} else if (set_sel == KDRV_IPP_BUILTIN_POLYGON_4) {

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE0_REG2);
		pm_reg &= ~(REG_VAL_IME_PM0_COMB_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_COMB_MODE_SEL_DISABLE;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE0_REG2, pm_reg);

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET6);
		pm_reg &= ~(REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_SHAPE_MODE_QUAD;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET6, pm_reg);

	} else if (set_sel == KDRV_IPP_BUILTIN_POLYGON_6) {

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE0_REG2);
		pm_reg &= ~(REG_VAL_IME_PM0_COMB_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_COMB_MODE_SEL_DISABLE;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE0_REG2, pm_reg);

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET6);
		pm_reg &= ~(REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_SHAPE_MODE_HEX;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET6, pm_reg);


	} else if (set_sel == KDRV_IPP_BUILTIN_POLYGON_8) {

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE0_REG2);
		pm_reg &= ~(REG_VAL_IME_PM0_COMB_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_COMB_MODE_SEL_OCT;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE0_REG2, pm_reg);

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET6);
		pm_reg &= ~(REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_SHAPE_MODE_QUAD;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET6, pm_reg);

	} else if (set_sel == KDRV_IPP_BUILTIN_POLYGON_10) {

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE0_REG2);
		pm_reg &= ~(REG_VAL_IME_PM0_COMB_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_COMB_MODE_SEL_DEC;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_PM_SET6_LINE0_REG2, pm_reg);

		pm_reg = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET6);
		pm_reg &= ~(REG_VAL_IME_PM0_SHAPE_MODE_SEL_MASK);
		pm_reg |= REG_VAL_IME_PM0_SHAPE_MODE_QUAD;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, KDRV_IPP_BUILTIN_PRI_MASK_REG_0_SET6, pm_reg);
	} else {
		//  do nothing...
	}
}

static INT32 kdrv_ipp_builtin_set_privacy_mask_polygon_buf_reg(UINT32 idx,  KDRV_IPP_BUILTIN_POLYGON_SEL pol_sel)
{

	switch (idx) {
	case 0:
	case 1:
		kdrv_ipp_builtin_set_privacy_mask_set0_polygon_buf_reg(pol_sel);
		break;

	case 2:
	case 3:
		kdrv_ipp_builtin_set_privacy_mask_set2_polygon_buf_reg(pol_sel);
		break;

	case 4:
	case 5:
		kdrv_ipp_builtin_set_privacy_mask_set4_polygon_buf_reg(pol_sel);
		break;

	case 6:
	case 7:
		kdrv_ipp_builtin_set_privacy_mask_set6_polygon_buf_reg(pol_sel);
		break;

	default:
		break;
	}

	return 0;
}

static INT32 kdrv_ipp_builtin_cfg_privacy_mask_coefs(UINT32 idx, IPOINT *p_coord, IPOINT *p_coord2, KDRV_IPP_BUILTIN_POLYGON_SEL pol_sel)
{
	KDRV_IPP_BUILTIN_PM_LINE_COEFS line[10];

	if (pol_sel == KDRV_IPP_BUILTIN_POLYGON_HOLLOW || pol_sel == KDRV_IPP_BUILTIN_POLYGON_4) {

		kdrv_ipp_builtin_cal_convex_hull_coefs((IPOINT *)&p_coord[0], 4, line);
		kdrv_ipp_builtin_set_privacy_mask_line0_buf_reg(idx, &line[0].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line1_buf_reg(idx, &line[1].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line2_buf_reg(idx, &line[2].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line3_buf_reg(idx, &line[3].coefs[0]);

	} else if(pol_sel == KDRV_IPP_BUILTIN_POLYGON_6) {
		kdrv_ipp_builtin_cal_convex_hull_coefs((IPOINT *)&p_coord[0], 6, line);
		kdrv_ipp_builtin_set_privacy_mask_line0_buf_reg(idx, &line[0].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line1_buf_reg(idx, &line[1].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line2_buf_reg(idx, &line[2].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line3_buf_reg(idx, &line[3].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line4_buf_reg(idx, &line[4].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line5_buf_reg(idx, &line[5].coefs[0]);

	} else if(pol_sel == KDRV_IPP_BUILTIN_POLYGON_8){
		kdrv_ipp_builtin_cal_convex_hull_coefs((IPOINT *)&p_coord[0], 8, line);
		kdrv_ipp_builtin_set_privacy_mask_line0_buf_reg(idx, &line[0].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line1_buf_reg(idx, &line[1].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line2_buf_reg(idx, &line[2].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line3_buf_reg(idx, &line[3].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line4_buf_reg(idx, &line[4].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line5_buf_reg(idx, &line[5].coefs[0]);

		kdrv_ipp_builtin_set_privacy_mask_line0_buf_reg(idx+1, &line[6].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line1_buf_reg(idx+1, &line[7].coefs[0]);

	} else if(pol_sel == KDRV_IPP_BUILTIN_POLYGON_10){
		kdrv_ipp_builtin_cal_convex_hull_coefs((IPOINT *)&p_coord[0], 10, line);
		kdrv_ipp_builtin_set_privacy_mask_line0_buf_reg(idx, &line[0].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line1_buf_reg(idx, &line[1].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line2_buf_reg(idx, &line[2].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line3_buf_reg(idx, &line[3].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line4_buf_reg(idx, &line[4].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line5_buf_reg(idx, &line[5].coefs[0]);

		kdrv_ipp_builtin_set_privacy_mask_line0_buf_reg(idx+1, &line[6].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line1_buf_reg(idx+1, &line[7].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line2_buf_reg(idx+1, &line[8].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line3_buf_reg(idx+1, &line[9].coefs[0]);
	}

	if (pol_sel == KDRV_IPP_BUILTIN_POLYGON_HOLLOW) {
		kdrv_ipp_builtin_cal_convex_hull_coefs((IPOINT *)&p_coord2[0], 4, line);
		kdrv_ipp_builtin_set_privacy_mask_line0_buf_reg(idx+1, &line[0].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line1_buf_reg(idx+1, &line[1].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line2_buf_reg(idx+1, &line[2].coefs[0]);
		kdrv_ipp_builtin_set_privacy_mask_line3_buf_reg(idx+1, &line[3].coefs[0]);
	}

	if((idx & 0x1) == 0) //even pm_set  //polygon must be set after set coefs
		kdrv_ipp_builtin_set_privacy_mask_polygon_buf_reg(idx, pol_sel);


	return 0;
}

static INT32 kdrv_ipp_builtin_cfg_privacy_mask(KDRV_IPP_BUILTIN_HDL *p_hdl)
{
	KDRV_IPP_BUILTIN_PRI_MASK_REG_CTRL pm_reg_ctl;
	UINT32 i = 0, tmp = 0;
	UINT32 tmp_ofs = 0;

	/* config register base on mask */
	for (i = 0; i < KDRV_IPP_BUILTIN_PRI_MASK_NUM; i++) {

		tmp = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_FUNCTION_REG1);

		if (p_hdl->mask[i].enable) {
			tmp |= (1 << i);

			kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_FUNCTION_REG1, tmp);

			/* ctrl */
			tmp_ofs = (i < 4) ? (KDRV_IPP_BUILTIN_PRI_MASK_REG_0 + ((i % 4) * KDRV_IPP_BUILTIN_PRI_MASK_REG_OFS)) :
							(KDRV_IPP_BUILTIN_PRI_MASK_REG_1 + ((i % 4) * KDRV_IPP_BUILTIN_PRI_MASK_REG_OFS));

			pm_reg_ctl.val = 0;
			pm_reg_ctl.bit.type = 0;
			pm_reg_ctl.bit.color_y = p_hdl->mask[i].color[0];
			pm_reg_ctl.bit.color_u = p_hdl->mask[i].color[1];
			pm_reg_ctl.bit.color_v = p_hdl->mask[i].color[2];
			pm_reg_ctl.bit.hlw_en = p_hdl->mask[i].hlw_enable;
			kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, tmp_ofs, pm_reg_ctl.val);

			/* alpha */
			tmp_ofs = (i < 4) ? KDRV_IPP_BUILTIN_PRI_MASK_ALPHA_REG_0 : KDRV_IPP_BUILTIN_PRI_MASK_ALPHA_REG_1;

			pm_reg_ctl.val = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, tmp_ofs);
			pm_reg_ctl.val &= ~(0xff << (8 * (i % 4)));
			pm_reg_ctl.val |= p_hdl->mask[i].weight << (8 * (i % 4));
			kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, tmp_ofs, pm_reg_ctl.val);

			/* line coefs */
			kdrv_ipp_builtin_cfg_privacy_mask_coefs(i, p_hdl->mask[i].coord, p_hdl->mask[i].coord2, p_hdl->mask[i].poly_sel);

		} else {
			tmp &= ~(1 << i);
			kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_FUNCTION_REG1, tmp);
		}
	}

	return 0;
}

static INT32 kdrv_ipp_builtin_cfg_flip(KDRV_IPP_BUILTIN_HDL *p_hdl){

	UINT32 eng = KDRV_IPP_BUILTIN_IME;
	UINT32 func_ctrl_2;
	UINT32 i = 0;
	KDRV_IPP_BUILTIN_IMG_INFO *p_path_info = NULL;

	func_ctrl_2 = kdrv_ipp_builtin_get_reg(eng, REG_OFS_IME_FUNCTION_REG1);

	if(p_hdl->_3dnr_ref_flip)
		func_ctrl_2 |= REG_VAL_IME_3DNR_INREF_FLIP_EN; //0x08 bit 16 IME_3DNR_INREF_FLIP_EN
	else
		func_ctrl_2 &= ~(REG_VAL_IME_3DNR_INREF_FLIP_EN);

	/* path 1~5 */
	for (i = 0; i <= KDRV_IPP_BUILTIN_PATH_ID_5; i++) {
		p_path_info = &p_hdl->path_info[i];
		if (p_path_info->enable) {

				if(p_hdl->flip[i] == 1){

					if( i <= KDRV_IPP_BUILTIN_PATH_ID_3)
						func_ctrl_2 |= REG_VAL_IME_P0_FLIP_EN << i;  //0x08 bit 12~14 IME_P1_FLIP_EN

					if(i == KDRV_IPP_BUILTIN_PATH_ID_5)
						func_ctrl_2 |= REG_VAL_IME_3DNR_OUTREF_FLIP_EN; //0x08 bit 17 IME_3DNR_OUTREF_FLIP_EN
				}else{

					if( i <= KDRV_IPP_BUILTIN_PATH_ID_3)
						func_ctrl_2 &= ~(REG_VAL_IME_P0_FLIP_EN << i);  //0x08 bit 11~13 IME_P1_FLIP_EN

					if(i == KDRV_IPP_BUILTIN_PATH_ID_5)
						func_ctrl_2 &= ~(REG_VAL_IME_3DNR_OUTREF_FLIP_EN); //0x08 bit 17 IME_3DNR_OUTREF_FLIP_EN
				}

			}
	}

	kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_FUNCTION_REG1, func_ctrl_2);

	return 0;
}

static void kdrv_ipp_builtin_cfg_mirror(KDRV_IPP_BUILTIN_HDL *p_hdl)
{
	UINT32 cfapat_reg = 0;
	UINT32 pxl_fmt = 0;
	UINT32 eng = KDRV_IPP_BUILTIN_IFE;

	if(p_hdl->hdal_mirror != p_hdl->mirror && // if hdal mirror == 1 mean already mirror at hdal dump phase
			( ( (p_hdl->fs_cnt == 0) && (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) ) ||  (p_hdl->flow != KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) )){

		UINT8 bayer_fmt=0;

		cfapat_reg = kdrv_ipp_builtin_get_reg(eng, REG_OFS_IFE_FUNCTION_REG);

		bayer_fmt = cfapat_reg >> REG_BIT_IFE_BAYER_FORMAT & REG_MASK_IFE_BAYER_FORMAT;
		pxl_fmt = (cfapat_reg >> REG_BIT_IFE_CFAPAT) & REG_MASK_IFE_CFAPAT;

		if(bayer_fmt == 0){

			if(pxl_fmt == 0)
				pxl_fmt = 1;
			else if(pxl_fmt == 1)
				pxl_fmt = 0;
			else if(pxl_fmt == 2)
				pxl_fmt = 3;
			else if(pxl_fmt ==3)
				pxl_fmt =2;
			else
				DBG_ERR("unknown pxl fmt\n");

		}else if(bayer_fmt == 1){

			if(pxl_fmt == 0)
				pxl_fmt = 1;
			else if(pxl_fmt == 1)
				pxl_fmt = 0;
			else if(pxl_fmt == 2)
				pxl_fmt = 3;
			else if(pxl_fmt == 3)
				pxl_fmt =2;
			else if(pxl_fmt == 4)
				pxl_fmt = 5;
			else if(pxl_fmt == 5)
				pxl_fmt = 4;
			else if(pxl_fmt == 6)
				pxl_fmt = 7;
			else if(pxl_fmt == 7)
				pxl_fmt = 6;
			else
				DBG_ERR("unknown pxl fmt\n");

		}else
			DBG_DUMP("unknown bayer fmt\n");

		cfapat_reg &= 0xFFFFF1FF;
		cfapat_reg |= pxl_fmt;
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IFE_FUNCTION_REG, cfapat_reg);

	}

	return;
}

INT32 kdrv_ipp_builtin_cfg_ife(KDRV_IPP_BUILTIN_HDL *p_hdl)
{
	UINT32 eng = KDRV_IPP_BUILTIN_IFE;
	UINT32 eng_ctrl = 0;
	UINT32 func_ctrl = 0;
	UINT32 wdr_func_ctrl = 0;
	UINT32 single_out_bit = 0;
	UINT32 buf_idx = 0;
	UINT32 switch_idx = 0;
	KDRV_IPP_BUILTIN_JOB *p_job = NULL;

	if(p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW)
		buf_idx = p_hdl->fs_cnt % 2;
	else
		buf_idx = (p_hdl->fs_cnt) % 2;

	if(p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW)
		switch_idx = p_hdl->ll_queue_num % 2;
	else
		switch_idx = (~(buf_idx)) & 0x01;

	single_out_bit = REG_VAL_IFE_OUT_MODE;

	if (p_hdl->fs_cnt == 0) {
		/* shdr enable, set rinbuf addr for direct mode at first frame */
		if (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW &&
			p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_SHDR) {

			if(p_hdl->hdr_frm_num == 2)
				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IFE_DRAM_SAI1, p_hdl->vprc_shdr_blk[0].phy_addr);
			else{
				DBG_ERR("unknown hdr_frm_num:%d\n",p_hdl->hdr_frm_num);
			}
		}

		/* clear all interrupt */
		kdrv_ipp_builtin_set_reg(eng, IFE_FB_INT_STS_OFS, 0xffffffff);
	}

	if (p_hdl->fs_cnt == 0){ // check if hdal dump phase is mirror or not

		func_ctrl = kdrv_ipp_builtin_get_reg(eng, REG_OFS_IFE_FUNCTION_REG);

		if(func_ctrl & REG_VAL_IFE_MIRROR_EN)
			p_hdl->hdal_mirror = 1;
		else
			p_hdl->hdal_mirror = 0;

		DBG_DUMP("----hdal_mirror:%d----\n", p_hdl->hdal_mirror);
	}

	if(p_hdl->mirror == 1 ){

		func_ctrl = kdrv_ipp_builtin_get_reg(eng, REG_OFS_IFE_FUNCTION_REG);
		func_ctrl |= 1 << 30; //mirror : 0x4 bit 30
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IFE_FUNCTION_REG, func_ctrl);

	}else{

		func_ctrl = kdrv_ipp_builtin_get_reg(eng, REG_OFS_IFE_FUNCTION_REG);
		func_ctrl &= ~(1 << 30); //mirror : 0x4 bit 30
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IFE_FUNCTION_REG, func_ctrl);

	}

	/* set mirror pixel order */
	kdrv_ipp_builtin_cfg_mirror(p_hdl);

	/* wdr */
	if (p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_WDR) {

		wdr_func_ctrl = kdrv_ipp_builtin_get_reg(eng, REG_OFS_IFE_WDR_CRTL_REG);

		if (p_hdl->pri_buf.wdr[0].size > 0) {
			if (p_hdl->fs_cnt == 0) {
				/* disable wdr for first frame */
				wdr_func_ctrl &= ~(REG_VAL_IFE_WDR_EN);

				/* prevent iq misenable, set subin addr to un-used private buffer */
				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IFE_WDR_SUBIMG_SAI, p_hdl->pri_buf.wdr[1].phy_addr); //tmp need to check hw dependance
			} else {


				if( (p_hdl->rtos_ipp_en == 1) && (p_hdl->fs_cnt == 4)  && (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) ){ //fb2.0 direct

					kdrv_ipp_builtin_set_reg(eng, REG_OFS_IFE_WDR_SUBIMG_SAI, p_hdl->pri_buf.wdr[switch_idx].phy_addr);

				}else if( (p_hdl->rtos_ipp_en == 1) && (p_hdl->fs_cnt == p_hdl->rtos_frm_cnt) && ((p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_RAW) ||  (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_CCIR))){ //fb2.0 d2d

					kdrv_ipp_builtin_set_reg(eng, REG_OFS_IFE_WDR_SUBIMG_SAI, p_hdl->pri_buf.wdr[switch_idx].phy_addr);

				}else{
					/* update reference buffer */
					kdrv_ipp_builtin_set_reg(eng, REG_OFS_IFE_WDR_SUBIMG_SAI, p_hdl->last_out_info.wdr_addr.pa); //tmp need to check hw dependance  //567 modified
				}

			}

			/* wdr subout */
			kdrv_ipp_builtin_set_reg(eng, REG_OFS_IFE_WDR_SUBIMG_SAO, p_hdl->pri_buf.wdr[buf_idx].phy_addr); //tmp need to check hw dependance
			wdr_func_ctrl |= REG_VAL_IFE_WDR_SUBIMG_OUT_EN;
			single_out_bit |= REG_VAL_IFE_OUT1_SINGLE_EN;
			p_hdl->last_out_info.wdr_addr.pa = p_hdl->pri_buf.wdr[buf_idx].phy_addr;  //567 modified
			p_hdl->last_out_info.wdr_addr.va = p_hdl->pri_buf.wdr[buf_idx].addr;  //567 modified
		}

		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IFE_WDR_CRTL_REG, wdr_func_ctrl);
	}

	kdrv_ipp_builtin_set_reg(eng, REG_OFS_IFE_DRAM_SINGLE_OUT_CTRL, single_out_bit);

	/* AXI ctrl*/
	kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IFE, 0x3f0, 0x3FFFFFF0);
	kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IFE, 0x3f8, 0xF0F);

	if (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) {
		/* load control */
		eng_ctrl = kdrv_ipp_builtin_get_reg(eng, 0x0);
		eng_ctrl |= LOAD_CTRL;
		kdrv_ipp_builtin_set_reg(eng, 0x0, eng_ctrl);
	} else {
		/* d2d mode update input address from p_hdl->p_cur_job */
		if (p_hdl->p_cur_job) {
			p_job = (KDRV_IPP_BUILTIN_JOB *)p_hdl->p_cur_job;

			kdrv_ipp_builtin_set_reg(eng, REG_OFS_IFE_DRAM_SAI0, p_job->pa[0]);
			if (p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_SHDR) {
				if(p_hdl->hdr_frm_num == 2){
					kdrv_ipp_builtin_set_reg(eng, REG_OFS_IFE_DRAM_SAI1, p_job->pa_2[0]);
				}else{
					DBG_ERR("unknown hdr_frm_num:%d\n",p_hdl->hdr_frm_num);
				}
			}
		}

		/* eng start load */
		eng_ctrl = kdrv_ipp_builtin_get_reg(eng, REG_OFS_IFE_CTRL);
		eng_ctrl |= REG_VAL_IFE_LOAD_START;
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IFE_CTRL, eng_ctrl);
	}

	return 0;
}

#if 0 // 567 removed
INT32 kdrv_ipp_builtin_cfg_dce(KDRV_IPP_BUILTIN_HDL *p_hdl)
{
	UINT32 eng = KDRV_IPP_BUILTIN_DCE;
	UINT32 eng_ctrl;
	UINT32 func_ctrl;
	UINT32 single_out_bit;
	UINT32 buf_idx;
	UINT32 cfapat_reg = 0;
	UINT32 pxl_fmt = 0;
	KDRV_IPP_BUILTIN_JOB *p_job = NULL;

	buf_idx = p_hdl->fs_cnt % 2;
	single_out_bit = 0x10;
	func_ctrl = kdrv_ipp_builtin_get_reg(eng, 0x04);

	/* disable 2dlut for first frame */
	if (p_hdl->fs_cnt == 0) {
		func_ctrl &= ~(0x80000);
	}

	/* set mirror pixel order */
	if(p_hdl->hdal_mirror != p_hdl->mirror && // if hdal mirror == 1 mean already mirror at hdal dump phase
			( ( (p_hdl->fs_cnt == 0) && (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) ) ||  (p_hdl->flow != KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) )){

		UINT8 bayer_fmt=0;

		cfapat_reg = kdrv_ipp_builtin_get_reg(eng, 0xA0);

		bayer_fmt = cfapat_reg >> 3;
		pxl_fmt = cfapat_reg & 0x7;

		if(bayer_fmt == 0){

			if(pxl_fmt == 0)
				pxl_fmt = 1;
			else if(pxl_fmt == 1)
				pxl_fmt = 0;
			else if(pxl_fmt == 2)
				pxl_fmt = 3;
			else if(pxl_fmt ==3)
				pxl_fmt =2;
			else
				DBG_ERR("unknown pxl fmt\n");

		}else if(bayer_fmt == 1){

			if(pxl_fmt == 0)
				pxl_fmt = 1;
			else if(pxl_fmt == 1)
				pxl_fmt = 0;
			else if(pxl_fmt == 2)
				pxl_fmt = 3;
			else if(pxl_fmt == 3)
				pxl_fmt =2;
			else if(pxl_fmt == 4)
				pxl_fmt = 5;
			else if(pxl_fmt == 5)
				pxl_fmt = 4;
			else if(pxl_fmt == 6)
				pxl_fmt = 7;
			else if(pxl_fmt == 7)
				pxl_fmt = 6;
			else
				DBG_ERR("unknown pxl fmt\n");

		}else
			DBG_DUMP("unknown bayer fmt\n");

		cfapat_reg &= 0x8;
		cfapat_reg |= pxl_fmt;
		kdrv_ipp_builtin_set_reg(eng, 0xA0, cfapat_reg);

	}

	/* wdr */
	if (p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_WDR) {
		if (p_hdl->pri_buf.wdr[0].size > 0) {
			if (p_hdl->fs_cnt == 0) {
				/* disable wdr for first frame */
				func_ctrl &= ~(0x200);

				/* prevent iq misenable, set subin addr to un-used private buffer */
				kdrv_ipp_builtin_set_reg(eng, 0x124, p_hdl->pri_buf.wdr[1].phy_addr); //tmp need to check hw dependance
			} else {
				/* update reference buffer */
				kdrv_ipp_builtin_set_reg(eng, 0x124, p_hdl->last_out_info.wdr_addr.pa); //tmp need to check hw dependance //567 modified
			}

			/* wdr subout */
			kdrv_ipp_builtin_set_reg(eng, 0x12C, p_hdl->pri_buf.wdr[buf_idx].phy_addr); //tmp need to check hw dependance
			func_ctrl |= 0x400;
			single_out_bit |= 0x1;
			p_hdl->last_out_info.wdr_addr.pa = p_hdl->pri_buf.wdr[buf_idx].phy_addr; //567 modified
		}
	}

	/* single out, func ctrl */
	kdrv_ipp_builtin_set_reg(eng, 0x04, func_ctrl); //tmp need to check hw dependance
	kdrv_ipp_builtin_set_reg(eng, 0x18, single_out_bit); //tmp need to check hw dependance

	if (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) {
		/* load control */
		eng_ctrl = kdrv_ipp_builtin_get_reg(eng, 0x0);
		eng_ctrl |= LOAD_CTRL;
		kdrv_ipp_builtin_set_reg(eng, 0x0, eng_ctrl);
	}else if(p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_CCIR){

		/* d2d mode update input address from p_hdl->p_cur_job */
		if (p_hdl->p_cur_job) {
			p_job = (KDRV_IPP_BUILTIN_JOB *)p_hdl->p_cur_job;

			kdrv_ipp_builtin_set_reg(eng, 0x20, nvtmpp_sys_va2pa(p_job->va[0]));
			kdrv_ipp_builtin_set_reg(eng, 0x28, nvtmpp_sys_va2pa(p_job->va[1]));
		}

		/* eng start load */
		eng_ctrl = kdrv_ipp_builtin_get_reg(eng, 0x0); //tmp need to check hw dependance
		eng_ctrl |= 0x4;
		kdrv_ipp_builtin_set_reg(eng, 0x0, eng_ctrl); //tmp need to check hw dependance

	} else {
		/* eng start load */
		eng_ctrl = kdrv_ipp_builtin_get_reg(eng, 0x0); //tmp need to check hw dependance
		eng_ctrl |= 0x4;
		kdrv_ipp_builtin_set_reg(eng, 0x0, eng_ctrl); //tmp need to check hw dependance
	}

	return 0;
}
#endif
INT32 kdrv_ipp_builtin_cfg_ipe(KDRV_IPP_BUILTIN_HDL *p_hdl)
{
	UINT32 eng = KDRV_IPP_BUILTIN_IPE;
	UINT32 eng_ctrl;
	UINT32 func_ctrl;
	UINT32 single_out_bit;
	UINT32 buf_idx;
	UINT32 switch_idx = 0;

	if(p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW)
		buf_idx = p_hdl->fs_cnt % 2;
	else
		buf_idx = (p_hdl->fs_cnt) % 2;

	if(p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW)
		switch_idx = p_hdl->ll_queue_num % 2;
	else
		switch_idx = (~(buf_idx)) & 0x01;

	single_out_bit = REG_VAL_IPE_DRAM_OUT_MODE;
	func_ctrl = kdrv_ipp_builtin_get_reg(eng, REG_OFS_IPE_MODE_REG1);

	/* disable gamma, yout, wait iq flow enable
		direct mode, disable for first frame
		d2d mode, always disable and wait iq config
	*/
	if ((p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW && p_hdl->fs_cnt == 0) ||
		(p_hdl->flow != KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW)) {
		func_ctrl &= ~(REG_VAL_IPE_RGBGAMMA_EN | REG_VAL_IPE_YCURVE_EN);
	}

	/* enable defog subout, disable defog, lce func */
	if (p_hdl->pri_buf.defog[0].size > 0) {
		if (p_hdl->fs_cnt == 0) {
			/* disable defog, lce for first frame */
			func_ctrl &= ~(REG_VAL_IPE_LCE_EN);

			/* prevent iq misenable, set subin addr to un-used private buffer */
			kdrv_ipp_builtin_set_reg(eng, REG_OFS_IPE_DMA_DEFOG_SUBIMG_IN_CH, p_hdl->pri_buf.defog[1].phy_addr);
		} else {


			if( (p_hdl->rtos_ipp_en == 1) && (p_hdl->fs_cnt == 4)  && (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) ){ //fb2.0 direct

				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IPE_DMA_DEFOG_SUBIMG_IN_CH, p_hdl->pri_buf.defog[switch_idx].phy_addr);

			}else if( (p_hdl->rtos_ipp_en == 1) && (p_hdl->fs_cnt == p_hdl->rtos_frm_cnt) && ((p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_RAW) ||  (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_CCIR))){ //fb2.0 d2d

				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IPE_DMA_DEFOG_SUBIMG_IN_CH, p_hdl->pri_buf.defog[switch_idx].phy_addr);

			}else{
				/* update reference buffer */
				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IPE_DMA_DEFOG_SUBIMG_IN_CH, p_hdl->last_out_info.defog_addr.pa); //567 modified
			}

		}

		/* defog subout */
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IPE_DMA_DEFOG_SUBIMG_OUT_CH, p_hdl->pri_buf.defog[buf_idx].phy_addr);
		single_out_bit |= REG_VAL_IPE_DRAM_OUT3_SINGLE_EN;
		func_ctrl |= REG_VAL_DEFOG_SUBIMG_OUT_EN;
		p_hdl->last_out_info.defog_addr.pa = p_hdl->pri_buf.defog[buf_idx].phy_addr; //567 modified
		p_hdl->last_out_info.defog_addr.va = p_hdl->pri_buf.defog[buf_idx].addr;

		if (p_hdl->fs_cnt < 2) {
			DBG_DUMP("isp_id:%d, subin 0x%.8x, subout 0x%.8x, cnt %d\r\n",
				p_hdl->isp_id,
				kdrv_ipp_builtin_get_reg(eng, REG_OFS_IPE_DMA_DEFOG_SUBIMG_IN_CH),
				kdrv_ipp_builtin_get_reg(eng, REG_OFS_IPE_DMA_DEFOG_SUBIMG_OUT_CH),
				p_hdl->fs_cnt);
		}
	}

	/* set ipe_va_subimg output address*/
	if (p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_IPE_VA_SUBOUT) {

		/* ipe_va_subimg output */
		if (p_hdl->pri_buf.ipe_va[0].size > 0) {

			kdrv_ipp_builtin_set_reg(eng, REG_OFS_IPE_DRAM_SAO_VA, p_hdl->pri_buf.ipe_va[buf_idx].phy_addr);
			single_out_bit |= REG_VAL_IPE_DRAM_OUT2_SINGLE_EN;
			//func_ctrl |= (REG_VAL_IME_PM_PXL_SUBOUT_EN);

			p_hdl->last_out_info.ipe_va_addr.pa = p_hdl->pri_buf.ipe_va[buf_idx].phy_addr;
			p_hdl->last_out_info.ipe_va_addr.va = p_hdl->pri_buf.ipe_va[buf_idx].addr;
		}
	}

	/* single out, func ctrl */
	kdrv_ipp_builtin_set_reg(eng, REG_OFS_IPE_MODE_REG1, func_ctrl);
	kdrv_ipp_builtin_set_reg(eng, REG_OFS_IPE_DMA_REG0, single_out_bit);

	/* AXI ctrl*/
	kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IPE, 0x498, 0xf0f00);
	kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IPE, 0x49C, 0xFFFFFF);

	if (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) {
		/* load control */
		eng_ctrl = kdrv_ipp_builtin_get_reg(eng, REG_OFS_IPE_CTRL);
		eng_ctrl |= LOAD_CTRL;
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IPE_CTRL, eng_ctrl);
	} else {
		/* eng start load */
		eng_ctrl = kdrv_ipp_builtin_get_reg(eng, REG_OFS_IPE_CTRL);
		eng_ctrl |= REG_VAL_IPE_LODAD_START;
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IPE_CTRL, eng_ctrl);
	}

	return 0;
}

INT32 kdrv_ipp_builtin_cfg_ime(KDRV_IPP_BUILTIN_HDL *p_hdl)
{
	UINT32 eng = KDRV_IPP_BUILTIN_IME;
	UINT32 eng_ctrl = 0;
	UINT32 func_ctrl = 0;
	UINT32 func_ctrl_2 = 0;
	UINT32 intr_ctrl = 0;
	UINT32 single_out_bit = 0;
	UINT32 buf_idx = 0;
	UINT32 pm_buf_idx = 0;
	UINT32 switch_idx = 0;
	UINT32 i = 0;
	UINT32 yuv_que_drop = 0;
	KDRV_IPP_BUILTIN_IMG_INFO *p_path_info;
	KDRV_IPP_BUILTIN_FMD_CB_INFO info = {0};

	if(p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW)
		buf_idx = p_hdl->fs_cnt % 2;
	else
		buf_idx = (p_hdl->fs_cnt) % 2;

	pm_buf_idx = p_hdl->fs_cnt %3;

	if(p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW)
		switch_idx = p_hdl->ll_queue_num % 2;
	else
		switch_idx = (~(buf_idx)) & 0x01;

	single_out_bit = REG_VAL_IME_DRAM_OUT_MODE;
	func_ctrl   = kdrv_ipp_builtin_get_reg(eng, REG_OFS_IME_FUNCTION_REG0);
	func_ctrl_2   = kdrv_ipp_builtin_get_reg(eng, REG_OFS_IME_FUNCTION_REG1);

	/*set ime dram end status enable bit and check status at next frame end to make sure engine setting is applied correctly*/
	kdrv_ipp_builtin_update_ime_dram_chk_bit(p_hdl);

#if 0 // 567 removed
	/* set lca output address, enable bypass */
	if (p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_YUV_SUBOUT) {
		UINT32 lca_ctrl;

		if (p_hdl->fs_cnt == 0) {
			/* enable bypass for first frame */
			lca_ctrl = kdrv_ipp_builtin_get_reg(eng, 0x230);
			lca_ctrl |= 0x8;
			kdrv_ipp_builtin_set_reg(eng, 0x230, lca_ctrl);
		} else {
			/* disable bypass */
			lca_ctrl = kdrv_ipp_builtin_get_reg(eng, 0x230);
			if (lca_ctrl & 0x8) {
				lca_ctrl &= ~(0x8);
				kdrv_ipp_builtin_set_reg(eng, 0x230, lca_ctrl);
			}

			if( (p_hdl->fs_cnt == 4) && (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) )// fb2.0
				kdrv_ipp_builtin_set_reg(eng, 0x22C, p_hdl->pri_buf.lca[switch_idx].phy_addr); //lca input addr
			else
				kdrv_ipp_builtin_set_reg(eng, 0x22C, p_hdl->last_out_info.lca_addr.pa); //lca input addr //567 modified
		}

		/* lca output */
		if (p_hdl->pri_buf.lca[0].size > 0) {

			kdrv_ipp_builtin_set_reg(eng, 0x26C, p_hdl->pri_buf.lca[buf_idx].phy_addr); //lca output addr
			single_out_bit |= 0x1;
			func_ctrl |= 0x02000000;

			p_hdl->last_out_info.lca_addr.pa = p_hdl->pri_buf.lca[buf_idx].phy_addr; //567 modified
		}


		/* lca center uv */
		kdrv_ipp_builtin_set_reg(eng, 0x234, (p_hdl->last_out_info.gray_avg_u | (p_hdl->last_out_info.gray_avg_v << 8)));
	}
#endif
	/* set pm pixelization output address, enable bypass */
	if (p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_PM_PIXELIZTION) {

		if (p_hdl->fs_cnt == 0) {
			/* disable lca for first frame (to disable pm pixelization, need disable lca) */
			func_ctrl &= ~(REG_VAL_IME_LCA_EN);
			/* prevent iq misenable, set subin addr to un-used private buffer */
			kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_PM_Y_SAI, p_hdl->last_out_info.pm_pixl_addr.pa); //pm pixelation input addr
		} else {

			if( (p_hdl->rtos_ipp_en == 1) && (p_hdl->fs_cnt == 4)  && (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) ){ //fb2.0 direct

				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_PM_Y_SAI, p_hdl->pri_buf.pm_pixl[switch_idx].phy_addr);

			}else if( (p_hdl->rtos_ipp_en == 1) && (p_hdl->fs_cnt == p_hdl->rtos_frm_cnt) && ((p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_RAW) ||  (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_CCIR))){ //fb2.0 d2d

				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_PM_Y_SAI, p_hdl->pri_buf.pm_pixl[switch_idx].phy_addr);

			}else{
				/* update reference buffer */
				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_PM_Y_SAI, p_hdl->last_out_info.pm_pixl_addr.pa); //pm pixelation input addr
			}
		}

		/* pm pixelation output */
		if (p_hdl->pri_buf.pm_pixl[0].size > 0) {

			kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_PM_PXL_DRAM_SAO, p_hdl->pri_buf.pm_pixl[pm_buf_idx].phy_addr); //pm pixelation output addr
			single_out_bit |= REG_VAL_IME_PXL_SUBOUT_DRAM_OUT_SINGLE_EN;
			func_ctrl_2 |= (REG_VAL_IME_PM_PXL_SUBOUT_EN);

			p_hdl->last_out_info.pm_pixl_addr.pa = p_hdl->pri_buf.pm_pixl[pm_buf_idx].phy_addr;
			p_hdl->last_out_info.pm_pixl_addr.va = p_hdl->pri_buf.pm_pixl[pm_buf_idx].addr;
		}
	}
	/* 3dnr ctrl, no reference image, disable 3dnr at first frame */
	if (p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_3DNR) {
		if (p_hdl->fs_cnt == 0) {
			/* disable 3dnr for first frame */
			func_ctrl &= ~(REG_VAL_IME_3DNR_EN);

			/* prevent iq misenable, set subin addr to un-used private buffer */
			/* ref yuv */
			kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_REF_DRAM_Y_SAI, p_hdl->vprc_blk[p_hdl->_3dnr_ref_path][1].phy_addr);
			kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_REF_DRAM_UV_SAI, p_hdl->vprc_blk[p_hdl->_3dnr_ref_path][1].phy_addr);

			/* ref mv, ms */
			kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_MOT_DRAM_SAI, p_hdl->pri_buf._3dnr_ms[1].phy_addr);
			kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_MV_DRAM_SAI, p_hdl->pri_buf._3dnr_mv[1].phy_addr);

			/* ref fcvg */
			kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_FAST_CONVERGE_DRAM_SAI, p_hdl->pri_buf._3dnr_fcvg[1].phy_addr);
		} else {
			/* update reference buffer */

			/* ref yuv */
			if( (p_hdl->rtos_ipp_en == 1) && (p_hdl->_3dnr_ref_flip) && (p_hdl->fs_cnt == 4)  && (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) ){ //fb2.0 direct

				p_path_info = &p_hdl->path_info[p_hdl->_3dnr_ref_path];

				p_path_info->phyaddr[0] = p_hdl->vprc_blk[p_hdl->_3dnr_ref_path][switch_idx].phy_addr;
				p_path_info->phyaddr[1] = p_path_info->phyaddr[0] + kdrv_ipp_builtin_y_size(p_path_info);
				p_path_info->phyaddr[2] = p_path_info->phyaddr[1] + kdrv_ipp_builtin_uv_size(p_path_info);

				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_REF_DRAM_Y_SAI,  p_path_info->phyaddr[0]);
				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_REF_DRAM_UV_SAI, p_path_info->phyaddr[1]);

			}else if( (p_hdl->rtos_ipp_en == 1) && (p_hdl->fs_cnt == p_hdl->rtos_frm_cnt) && ((p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_RAW) ||  (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_CCIR))){ //fb2.0 d2d

				p_path_info = &p_hdl->path_info[p_hdl->_3dnr_ref_path];

				p_path_info->phyaddr[0] = p_hdl->vprc_blk[p_hdl->_3dnr_ref_path][switch_idx].phy_addr;
				p_path_info->phyaddr[1] = p_path_info->phyaddr[0] + kdrv_ipp_builtin_y_size(p_path_info);
				p_path_info->phyaddr[2] = p_path_info->phyaddr[1] + kdrv_ipp_builtin_uv_size(p_path_info);

				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_REF_DRAM_Y_SAI,  p_path_info->phyaddr[0]);
				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_REF_DRAM_UV_SAI, p_path_info->phyaddr[1]);

			}
			else{
				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_REF_DRAM_Y_SAI, p_hdl->last_out_info.path_addr_y[p_hdl->_3dnr_ref_path].pa); //567 modified
				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_REF_DRAM_UV_SAI, p_hdl->last_out_info.path_addr_u[p_hdl->_3dnr_ref_path].pa); //567 modified
			}
			/* ref mv, ms */
			if( (p_hdl->rtos_ipp_en == 1) && (p_hdl->fs_cnt == 4)  && (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) ){ //fb2.0 direct

				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_MOT_DRAM_SAI, p_hdl->pri_buf._3dnr_ms[switch_idx].phy_addr);
				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_MV_DRAM_SAI, p_hdl->pri_buf._3dnr_mv[switch_idx].phy_addr);

			}else if( (p_hdl->rtos_ipp_en == 1) && (p_hdl->fs_cnt == p_hdl->rtos_frm_cnt) && ((p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_RAW) ||  (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_CCIR))){ //fb2.0 d2d

				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_MOT_DRAM_SAI, p_hdl->pri_buf._3dnr_ms[switch_idx].phy_addr);
				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_MV_DRAM_SAI, p_hdl->pri_buf._3dnr_mv[switch_idx].phy_addr);

			}else{

				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_MOT_DRAM_SAI, p_hdl->last_out_info._3dnr_ms_addr.pa); //567 modified
				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_MV_DRAM_SAI, p_hdl->last_out_info._3dnr_mv_addr.pa);  //567 modified

			}

			/* ref fcvg */
			if( (p_hdl->rtos_ipp_en == 1) && (p_hdl->fs_cnt == 4)  && (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) ) //fb2.0 direct
				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_FAST_CONVERGE_DRAM_SAI, p_hdl->pri_buf._3dnr_fcvg[switch_idx].phy_addr);
			else if( (p_hdl->rtos_ipp_en == 1) && (p_hdl->fs_cnt == p_hdl->rtos_frm_cnt) && ((p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_RAW) ||  (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_CCIR))) //fb2.0 d2d
				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_FAST_CONVERGE_DRAM_SAI, p_hdl->pri_buf._3dnr_fcvg[switch_idx].phy_addr);
			else
				kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_FAST_CONVERGE_DRAM_SAI, p_hdl->last_out_info._3dnr_fcvg_addr.pa); //567 modified
		}

		/* 3dnr motion subout */
		if ((p_hdl->pri_buf._3dnr_ms[0].size > 0) &&
			(p_hdl->pri_buf._3dnr_mv[0].size > 0)) {
			kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_MOT_DRAM_SAO, p_hdl->pri_buf._3dnr_ms[buf_idx].phy_addr);
			kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_MV_DRAM_SAO, p_hdl->pri_buf._3dnr_mv[buf_idx].phy_addr);
			single_out_bit |= (REG_VAL_IME_3DNR_MV_DRAM_OUT_SINGLE_EN | REG_VAL_IME_3DNR_MS_DRAM_OUT_SINGLE_EN);

			p_hdl->last_out_info._3dnr_ms_addr.pa = p_hdl->pri_buf._3dnr_ms[buf_idx].phy_addr; //567 modified
			p_hdl->last_out_info._3dnr_ms_addr.va = p_hdl->pri_buf._3dnr_ms[buf_idx].addr;
			p_hdl->last_out_info._3dnr_mv_addr.pa = p_hdl->pri_buf._3dnr_mv[buf_idx].phy_addr; //567 modified
			p_hdl->last_out_info._3dnr_mv_addr.va = p_hdl->pri_buf._3dnr_mv[buf_idx].addr;
		}

		/* 3dnr fast coverage */
		if (p_hdl->pri_buf._3dnr_fcvg[0].size > 0) {

			kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_3DNR_FAST_CONVERGE_DRAM_SAO, p_hdl->pri_buf._3dnr_fcvg[buf_idx].phy_addr);
			single_out_bit |= REG_VAL_IME_3DNR_FC_DRAM_OUT_SINGLE_EN;

			p_hdl->last_out_info._3dnr_fcvg_addr.pa = p_hdl->pri_buf._3dnr_fcvg[buf_idx].phy_addr; //567 modified
			p_hdl->last_out_info._3dnr_fcvg_addr.va = p_hdl->pri_buf._3dnr_fcvg[buf_idx].addr;
		}

	}

	/* path 1~5 */
	info.name = p_hdl->name;
	info.isp_id = p_hdl->isp_id;

	for (i = 0; i <= KDRV_IPP_BUILTIN_PATH_ID_5; i++) {
		p_path_info = &p_hdl->path_info[i];
		if (p_path_info->enable) {

			if(p_hdl->queue_mode_en[i]){

				if(p_hdl->queue_num[i] > 0){

					p_hdl->fs_cnt_yuv_que++;
					info.out_img[i] = kdrv_ipp_builtin_get_path_info(info.isp_id, i);
					yuv_que_drop = kdrv_ipp_builtin_frc_yuv_que(p_hdl, &info);

					if (p_hdl->vprc_blk[i][p_hdl->queue_in_idx[i]].phy_addr) {
						//YUV buffer not support stitch
						p_path_info->phyaddr[0] = p_hdl->vprc_blk[i][p_hdl->queue_in_idx[i]].phy_addr;
						p_path_info->phyaddr[1] = p_path_info->phyaddr[0] + kdrv_ipp_builtin_y_size(p_path_info);
						p_path_info->phyaddr[2] = p_path_info->phyaddr[1] + kdrv_ipp_builtin_uv_size(p_path_info);

						p_path_info->addr[0] = p_hdl->vprc_blk[i][p_hdl->queue_in_idx[i]].addr;
						p_path_info->addr[1] = p_path_info->addr[0] + kdrv_ipp_builtin_y_size(p_path_info);
						p_path_info->addr[2] = p_path_info->addr[1] + kdrv_ipp_builtin_uv_size(p_path_info);

						kdrv_ipp_builtin_set_ime_out_addr(p_hdl, i, p_path_info);
						if (i == KDRV_IPP_BUILTIN_PATH_ID_5) {
							/* path 5(ref out) */
							func_ctrl |= REG_VAL_IME_3DNR_REF_OUT_EN;
							single_out_bit |= REG_VAL_IME_3DNR_REFOUT_DRAM_OUT_SINGLE_EN;
						} else {
							/* path1~4 */
							func_ctrl |= (REG_VAL_IME_IME_P0_EN << i);
							single_out_bit |= (REG_VAL_IME_OUT_P0_DRAM_OUT_SINGLE_EN << i);
						}
						p_hdl->last_out_info.path_addr_y[i].pa = p_path_info->phyaddr[0]; //567 modified
						p_hdl->last_out_info.path_addr_u[i].pa = p_path_info->phyaddr[1]; //567 modified
						p_hdl->last_out_info.path_addr_v[i].pa = p_path_info->phyaddr[2]; //567 modified
						p_hdl->last_out_info.path_addr_y[i].va = p_path_info->addr[0];
						p_hdl->last_out_info.path_addr_u[i].va = p_path_info->addr[1];
						p_hdl->last_out_info.path_addr_v[i].va = p_path_info->addr[2];

						if(p_hdl->queue_in_idx[i] == p_hdl->queue_num[i]-1){
							static UINT8 cnt=0;

							if(cnt < 36){
								DBG_DUMP("--queue full queue_in_idx %d, max_queue_num %d--\n", p_hdl->queue_in_idx[i], p_hdl->queue_num[i]);
								cnt++;
							}
						}

						if(!yuv_que_drop){
						    if (p_hdl->queue_ring_mode_en) {
						        p_hdl->queue_in_idx[i] = (p_hdl->queue_in_idx[i] + 1) % p_hdl->queue_num[i];
						    } else {
						        p_hdl->queue_in_idx[i] = min(p_hdl->queue_in_idx[i] + 1, p_hdl->queue_num[i]); // clamp to extra buffer
						    }

							if(p_hdl->buf_num_in_que[i] < p_hdl->queue_num[i]){
								p_hdl->buf_num_in_que[i]++;
							}
						}


					}else{
						DBG_ERR("failed addr : 0x%lx, path[%d]\n", p_hdl->vprc_blk[i][p_hdl->queue_in_idx[i]].phy_addr, i);
						return -1;
					}

				}else{
					DBG_ERR("queue mode enable but queue_num=%d\n", p_hdl->queue_num[i]);
				}

			}else if (p_hdl->vprc_blk[i][buf_idx].phy_addr){

				if(p_hdl->stitch_en[i]){


					p_path_info->phyaddr[0] = p_hdl->vprc_blk[i][buf_idx].phy_addr + p_hdl->stitch_addr_y_ofs[i];
					p_path_info->phyaddr[1] = p_hdl->vprc_blk[i][buf_idx].phy_addr + p_hdl->stitch_addr_uv_ofs[i];

					p_path_info->addr[0] = p_hdl->vprc_blk[i][buf_idx].addr + p_hdl->stitch_addr_y_ofs[i];
					p_path_info->addr[1] = p_hdl->vprc_blk[i][buf_idx].addr + p_hdl->stitch_addr_uv_ofs[i];

					//DBG_DUMP("p_hdl=0x%lx, y=0x%lx, uv=0x%lx\n",(ULONG)p_hdl, (ULONG) p_path_info->phyaddr[0], (ULONG)p_path_info->phyaddr[1]);

				}else{
					p_path_info->phyaddr[0] = p_hdl->vprc_blk[i][buf_idx].phy_addr;
					p_path_info->phyaddr[1] = p_path_info->phyaddr[0] + kdrv_ipp_builtin_y_size(p_path_info);
					p_path_info->phyaddr[2] = p_path_info->phyaddr[1] + kdrv_ipp_builtin_uv_size(p_path_info);

					p_path_info->addr[0] = p_hdl->vprc_blk[i][buf_idx].addr;
					p_path_info->addr[1] = p_path_info->addr[0] + kdrv_ipp_builtin_y_size(p_path_info);
					p_path_info->addr[2] = p_path_info->addr[1] + kdrv_ipp_builtin_uv_size(p_path_info);
				}
				kdrv_ipp_builtin_set_ime_out_addr(p_hdl, i, p_path_info);
				if (i == KDRV_IPP_BUILTIN_PATH_ID_5) {
					/* path 5(ref out) */
					func_ctrl |= REG_VAL_IME_3DNR_REF_OUT_EN;
					single_out_bit |= REG_VAL_IME_3DNR_REFOUT_DRAM_OUT_SINGLE_EN;
				} else {
					/* path1~4 */
					func_ctrl |= (REG_VAL_IME_IME_P0_EN << i);
					single_out_bit |= (REG_VAL_IME_OUT_P0_DRAM_OUT_SINGLE_EN << i);
				}
				p_hdl->last_out_info.path_addr_y[i].pa = p_path_info->phyaddr[0]; //567 modified
				p_hdl->last_out_info.path_addr_u[i].pa = p_path_info->phyaddr[1]; //567 modified
				p_hdl->last_out_info.path_addr_v[i].pa = p_path_info->phyaddr[2]; //567 modified
				p_hdl->last_out_info.path_addr_y[i].va = p_path_info->addr[0];
				p_hdl->last_out_info.path_addr_u[i].va = p_path_info->addr[1];
				p_hdl->last_out_info.path_addr_v[i].va = p_path_info->addr[2];
			}
		}
	}

	kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_FUNCTION_REG0, func_ctrl);
	kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_FUNCTION_REG1, func_ctrl_2);

	/* v-flip */
	kdrv_ipp_builtin_cfg_flip(p_hdl);

	/* privacy mask */
	kdrv_ipp_builtin_cfg_privacy_mask(p_hdl);

	/* single out, func ctrl */

	kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_DRAM_SINGLE_OUT_CTRL, single_out_bit);

	/* AXI ctrl*/
	kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, 0x9E8, 0xFFFFFFFF);
	kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, 0x9EC, 0xF);
	kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, 0x9F0, 0xF0F);
	kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, 0x9F4, 0xFFFFFFFF);
	kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, 0x9F8, 0xF);

	if (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) {
		/* load control */
		eng_ctrl = kdrv_ipp_builtin_get_reg(eng, REG_OFS_IME_CTRL);
		eng_ctrl |= LOAD_CTRL;
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_CTRL, eng_ctrl);
	} else {

		/* Turn on ime frm end interrupt and turn off ime ll end interrupt to force to using cpu mode*/
		intr_ctrl = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_INTERRUPT_EN);
		intr_ctrl &= ~(REG_VAL_IME_INTE_LL_END);
		intr_ctrl |= REG_VAL_IME_INTE_FRM_END;
		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_INTERRUPT_EN,intr_ctrl);
		/* eng start load */
		eng_ctrl = kdrv_ipp_builtin_get_reg(eng, REG_OFS_IME_CTRL);
		eng_ctrl |= REG_VAL_IME_START_LOAD;
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_CTRL, eng_ctrl);
	}

	return 0;
}
UINT32 kdrv_ipp_builtin_get_cfg_eng_bit(KDRV_IPP_BUILTIN_HDL *p_hdl)
{
	UINT32 cfg_eng_bit;

	switch (p_hdl->flow) {
	case KDRV_IPP_BUILTIN_FLOW_RAW:
	case KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW:
		cfg_eng_bit = (KDRV_IPP_BUILTIN_ENG_BIT(KDRV_IPP_BUILTIN_IFE) |
						/*KDRV_IPP_BUILTIN_ENG_BIT(KDRV_IPP_BUILTIN_DCE) |*/
						KDRV_IPP_BUILTIN_ENG_BIT(KDRV_IPP_BUILTIN_IPE) |
						KDRV_IPP_BUILTIN_ENG_BIT(KDRV_IPP_BUILTIN_IME));
		break;

	case KDRV_IPP_BUILTIN_FLOW_CCIR:
		cfg_eng_bit = (	/*KDRV_IPP_BUILTIN_ENG_BIT(KDRV_IPP_BUILTIN_DCE) |*/
						KDRV_IPP_BUILTIN_ENG_BIT(KDRV_IPP_BUILTIN_IPE) |
						KDRV_IPP_BUILTIN_ENG_BIT(KDRV_IPP_BUILTIN_IME));
		break;

	case KDRV_IPP_BUILTIN_FLOW_IME_D2D:
		cfg_eng_bit = KDRV_IPP_BUILTIN_ENG_BIT(KDRV_IPP_BUILTIN_IME);
		break;

	default:
		DBG_ERR("unsupport flow %d\r\n", (int)p_hdl->flow);
		return 0;
	}

	return cfg_eng_bit;
}

INT32 kdrv_ipp_builtin_parsing_dtsi(KDRV_IPP_BUILTIN_HDL *p_hdl)
{
	CHAR *dtsi_reg_num_tag[KDRV_IPP_BUILTIN_ENG_MAX] = {
		"ife_reg_num",
		/*"dce_reg_num",*/
		"ipe_reg_num",
		"ime_reg_num",
	};
	CHAR *dtsi_reg_cfg_tag[KDRV_IPP_BUILTIN_ENG_MAX] = {
		"ife_reg_cfg",
		/*"dce_reg_cfg",*/
		"ipe_reg_cfg",
		"ime_reg_cfg",
	};
	UINT32 eng;
	UINT32 reg_cfg_num;
	UINT32 *p_reg_buf;
	INT32 rt = 0;

	rt = kdrv_ipp_builtin_set_child_nodeofsset(&p_hdl->root_node, "register");
	if (rt != 0) {
		DBG_ERR("set register child_nodeofsset failed\r\n");
		return E_SYS;
	}

	for (eng = KDRV_IPP_BUILTIN_IFE; eng < KDRV_IPP_BUILTIN_ENG_MAX; eng++){

		if (kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, dtsi_reg_num_tag[eng], &reg_cfg_num, 1)) {
			DBG_ERR("read dtsi %s failed\r\n", dtsi_reg_num_tag[eng]);
			return -1;
		}

		p_reg_buf = kdrv_ipp_builtin_get_dtsi_buf(p_hdl, eng);

		if (p_reg_buf == NULL) {
			DBG_ERR("no buffer to read dtsi (eng=%d)\r\n", eng);
			return -1;
		}

		if (kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, dtsi_reg_cfg_tag[eng], p_reg_buf, (reg_cfg_num * 2))) {
			DBG_ERR("read dtsi %s failed\r\n", dtsi_reg_cfg_tag[eng]);
			return -1;
		}
	}

	return E_OK;
}

INT32 kdrv_ipp_builtin_cfg_eng_all(KDRV_IPP_BUILTIN_HDL *p_hdl)
{
#if 1//(KDRV_IPP_RTOS_IPP_ENABLE == 0) //fb2.0
	UINT32 eng;
	UINT32 reg_cfg_num;
	UINT32 *p_reg_buf;
	UINT32 ofs;
	UINT32 val;
	UINT32 i;
#endif

	UINT32 cfg_eng_bit;

	cfg_eng_bit = kdrv_ipp_builtin_get_cfg_eng_bit(p_hdl);
	if (cfg_eng_bit == 0) {
		return -1;
	}

#if 1//(KDRV_IPP_RTOS_IPP_ENABLE == 0) //fb2.0
	/* config from dtsi
		1. direct mode start
		2. dram mode
	*/

	if ((p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW && p_hdl->fs_cnt == 0) ||
		(p_hdl->flow != KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW)) {

		for (eng = KDRV_IPP_BUILTIN_IFE; eng < KDRV_IPP_BUILTIN_ENG_MAX; eng++) {
			if ((cfg_eng_bit & KDRV_IPP_BUILTIN_ENG_BIT(eng)) == 0) {
				continue;
			}

			reg_cfg_num = kdrv_ipp_builtin_get_reg_num(eng);
			p_reg_buf = kdrv_ipp_builtin_get_dtsi_buf(p_hdl, eng);

			for (i = 0; i < reg_cfg_num; i++) {
				ofs = p_reg_buf[i * 2];
				val = p_reg_buf[(i * 2) + 1];

				/* skip engine ctrl regiser, set it at last */
				if (ofs == 0) {
					continue;
				} else {
					/* write config to engine */
					kdrv_ipp_builtin_set_reg(eng, ofs, val);
				}
			}
		}
	}

#endif
	/* config engine */
	if (cfg_eng_bit & KDRV_IPP_BUILTIN_ENG_BIT(KDRV_IPP_BUILTIN_IFE)) {
		kdrv_ipp_builtin_cfg_ife(p_hdl);
	}
	#if 0 // 567 removed
	if (cfg_eng_bit & KDRV_IPP_BUILTIN_ENG_BIT(KDRV_IPP_BUILTIN_DCE)) {
		kdrv_ipp_builtin_cfg_dce(p_hdl);
	}
	#endif
	if (cfg_eng_bit & KDRV_IPP_BUILTIN_ENG_BIT(KDRV_IPP_BUILTIN_IPE)) {
		kdrv_ipp_builtin_cfg_ipe(p_hdl);
	}
	if (cfg_eng_bit & KDRV_IPP_BUILTIN_ENG_BIT(KDRV_IPP_BUILTIN_IME)) {
		kdrv_ipp_builtin_cfg_ime(p_hdl);
	}

	/* isp reset event for frm 0(both direct and drame mode)
		trig event for dram flow
	*/
	if (g_kdrv_ipp_isp_cb) {

		if(p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW){

			if (p_hdl->fs_cnt == /*0*/ p_hdl->ll_queue_num) { // rtos switch to builtin patch //fb2.0
				g_kdrv_ipp_isp_cb(p_hdl->isp_id, KDRV_IPP_BUILTIN_ISP_EVENT_RESET);
			}else{

				if( !(p_hdl->dbg_func_en & KDRV_IPP_BUILTIN_FUNC_ALGO_BY_PASS))
					g_kdrv_ipp_isp_cb(p_hdl->isp_id, KDRV_IPP_BUILTIN_ISP_EVENT_TRIG);
				else{

				}
			}

		}else{

			if (p_hdl->fs_cnt == p_hdl->rtos_frm_cnt) { // rtos switch to builtin patch //fb2.0
				g_kdrv_ipp_isp_cb(p_hdl->isp_id, KDRV_IPP_BUILTIN_ISP_EVENT_RESET);
			}else{

				if( !(p_hdl->dbg_func_en & KDRV_IPP_BUILTIN_FUNC_ALGO_BY_PASS))
					g_kdrv_ipp_isp_cb(p_hdl->isp_id, KDRV_IPP_BUILTIN_ISP_EVENT_TRIG);
				else{

				}
			}
		}
	}

	return 0;
}

INT32 kdrv_ipp_builtin_trig_eng_start(KDRV_IPP_BUILTIN_HDL *p_hdl)
{
	KDRV_IPP_BUILTIN_CTL *p_ctl;
	UINT32 cfg_eng_bit;
	UINT32 eng;
	UINT32 val;

	p_ctl = &g_kdrv_ipp_builtin_ctl;
	if (p_ctl->p_trig_hdl != NULL) {
		DBG_ERR("already triggered for hdl 0x%.8lx\r\n", (ULONG)p_ctl->p_trig_hdl);
		return -1;
	}
	p_ctl->p_trig_hdl = p_hdl;

	/* start engine */
	cfg_eng_bit = kdrv_ipp_builtin_get_cfg_eng_bit(p_hdl);

	eng = KDRV_IPP_BUILTIN_IFE;
	if (cfg_eng_bit & KDRV_IPP_BUILTIN_ENG_BIT(eng)) {
		val = kdrv_ipp_builtin_get_reg(eng, REG_OFS_IFE_CTRL) | REG_VAL_IFE_START;
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IFE_CTRL, val);
	}

	eng = KDRV_IPP_BUILTIN_IPE;
	if (cfg_eng_bit & KDRV_IPP_BUILTIN_ENG_BIT(eng)) {
		val = kdrv_ipp_builtin_get_reg(eng, REG_OFS_IPE_CTRL) | REG_VAL_IPE_START;
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IPE_CTRL, val);
	}

	eng = KDRV_IPP_BUILTIN_IME;
	if (cfg_eng_bit & KDRV_IPP_BUILTIN_ENG_BIT(eng)) {
		val = kdrv_ipp_builtin_get_reg(eng, REG_OFS_IME_CTRL) | REG_VAL_IME_START;
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_IME_CTRL, val);
	}
	#if 0 // 567 removed
	eng = KDRV_IPP_BUILTIN_DCE;
	if (cfg_eng_bit & KDRV_IPP_BUILTIN_ENG_BIT(eng)) {
		val = kdrv_ipp_builtin_get_reg(eng, REG_OFS_DCE_CTRL) | REG_VAL_DCE_START;
		kdrv_ipp_builtin_set_reg(eng, REG_OFS_DCE_CTRL, val);
	}
	#endif
	return 0;
}
INT32 kdrv_ipp_builtin_soft_reset_eng(void)
{
	UINT32 eng_ctrl;
	UINT32 eng;

	for (eng = KDRV_IPP_BUILTIN_IFE; eng < KDRV_IPP_BUILTIN_ENG_MAX; eng++) {
		eng_ctrl = kdrv_ipp_builtin_get_reg(eng, 0);
		eng_ctrl |= 0x1;
		kdrv_ipp_builtin_set_reg(eng, 0, eng_ctrl);
		eng_ctrl &= ~(0x1);
		kdrv_ipp_builtin_set_reg(eng, 0, eng_ctrl);
	}

	return 0;
}
static void kdrv_ipp_builtin_upd_isp_info(KDRV_IPP_BUILTIN_HDL *p_hdl)
{
	UINT32 i=0, tmp=0;

	return;

	/* dce histogram result */
	tmp = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IFE, REG_OFS_IFE_WDR_CRTL_REG);
	p_hdl->isp_info.hist_rst.enable = (tmp & 0xC) >> 2;
	if (p_hdl->isp_info.hist_rst.enable) {
		p_hdl->isp_info.hist_rst.sel = (tmp & 0xC) >> 3;
		for (i = 0; i < 64; i++) {
			tmp = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IFE, (REG_OFS_IFE_HISTOGRAM_REG + (i << 2)));
			p_hdl->isp_info.hist_rst.stcs[i * 2] = tmp & (0xffff);
			p_hdl->isp_info.hist_rst.stcs[(i * 2) + 1] = (tmp >> 16) & (0xffff);
		}
	}

	/* edge statistic */ //567 not support
	tmp = 0;
	p_hdl->isp_info.edge_stcs.localmax_max = tmp & 0x3ff;
	p_hdl->isp_info.edge_stcs.coneng_max = (tmp >> 10) & 0x3ff;
	p_hdl->isp_info.edge_stcs.coneng_avg = (tmp >> 20) & 0x3ff;

	/* defog airlight */  //560 not support
	tmp = 0;
	p_hdl->isp_info.defog_stcs.airlight[0] = 0;
	p_hdl->isp_info.defog_stcs.airlight[1] = 0;
	tmp = 0;
	p_hdl->isp_info.defog_stcs.airlight[2] = 0;

	/* defog subout addr */
	p_hdl->isp_info.defog_subout_addr.pa = p_hdl->last_out_info.defog_addr.pa;
	p_hdl->isp_info.defog_subout_addr.va = p_hdl->last_out_info.defog_addr.va;
}
#if 0 // 530 removed
static void kdrv_ipp_builtin_upd_lca_gray_avg(KDRV_IPP_BUILTIN_HDL *p_hdl)
{

	UINT32 sum = 0;
	UINT32 cnt = 0;

	if (p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_YUV_SUBOUT) {
		cnt = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, 0xABC) & 0xFFFFFFF;
		if (cnt) {
			sum = (kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, 0xAA8) & 0x7ffff) +
					((kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, 0xAAC) & 0x3ffff) << 19);

			p_hdl->last_out_info.gray_avg_u = (sum + (cnt >> 1)) / cnt;

			sum = (kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, 0xAB4) & 0x7ffff) +
					((kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, 0xAB8) & 0x3ffff) << 19);
			p_hdl->last_out_info.gray_avg_v = (sum + (cnt >> 1)) / cnt;
		}
	}

}
#endif

BOOL kdrv_ipp_builtin_chech_do_stitch(void){


	if( (g_kdrv_ipp_builtin_ctl.p_hdl[0].frm_done == 1) && (g_kdrv_ipp_builtin_ctl.p_hdl[1].frm_done == 1)){
		return TRUE;
	}
	else
		return FALSE;
}

BOOL kdrv_ipp_builtin_clear_stitch_frm_done_chk(void){

	g_kdrv_ipp_builtin_ctl.p_hdl[0].frm_done = 0;
	g_kdrv_ipp_builtin_ctl.p_hdl[1].frm_done = 0;

	return TRUE;
}

void kdrv_ipp_builtin_cal_ll_tag_byte(KDRV_IPP_BUILTIN_HDL *p_hdl)
{
	UINT8 carry = 0;
	UINT32 pre_cal = 0;


	pre_cal = ( p_hdl->ll_queue_num  - 1) % 16;

	if( (pre_cal % 4) > 0)
		carry = 1;

	p_hdl->target_reg  = REG_OFS_IME_LL_TAB_0 + ( ((pre_cal/4) - 1 + carry) * 4 );
	p_hdl->target_byte = (pre_cal % 4);

	if(p_hdl->target_byte == 0 )
		p_hdl->target_byte = 4;

	p_hdl->target_position = (p_hdl->target_byte - 1) << 3;

	return;
}

BOOL kdrv_ipp_builtin_check_rtos_to_builtin(KDRV_IPP_BUILTIN_HDL *p_hdl)
{
	UINT32 result = 0;
	BOOL rt = 0;

	//DBG_DUMP("job_ed:0x%x,0x%x,0x%x,0x%x\n", kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_LL_TAB_0),
	//		kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_LL_TAB_1),
	//		kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_LL_TAB_2),
	//		kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_LL_TAB_3));


	result = (kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, p_hdl->target_reg) >> p_hdl->target_position ) & 0xFF;

	//DBG_DUMP("---------reg:0x%x byte:%d,rslt:0x%x ----------\n", p_hdl->target_reg, p_hdl->target_byte, result);

	if(result == 0xfe)
		rt = TRUE;
	else
		rt = FALSE;

	return rt;
}

KDRV_IPP_BUILTIN_IMG_INFO kdrv_ipp_builtin_get_buf_info(UINT32 isp_id, KDRV_IPP_BUILTIN_PATH_ID pid, UINT32 idx)
{
	UINT32 buf_idx;
	KDRV_IPP_BUILTIN_IMG_INFO img = {0};
	KDRV_IPP_BUILTIN_IMG_INFO *p_img = &img;
	KDRV_IPP_BUILTIN_HDL *p_hdl;
	KDRV_IPP_BUILTIN_JOB *p_job;
	UINT32 i;
	//KDRV_IPP_BUILTIN_CTL* p_ctl = &g_kdrv_ipp_builtin_ctl;
	p_hdl = NULL;

	for (i = 0; i < g_kdrv_ipp_builtin_ctl.hdl_num; i++) {
		if (isp_id == g_kdrv_ipp_builtin_ctl.p_hdl[i].isp_id) {
			p_hdl = &g_kdrv_ipp_builtin_ctl.p_hdl[i];
			break;
		}
	}


	if (p_hdl == NULL) {
		DBG_ERR("find no hdl isp_id:%d!!!!!!\r\n", isp_id);
		return img;
	}

	*p_img = p_hdl->path_info[pid];
	buf_idx = idx;
	if (pid < 5) {

		if (p_img->enable) {
			/* path1/2/3/4 address */
			if (p_hdl->vprc_blk[pid][buf_idx].addr) {
				p_img->addr[0] = p_hdl->vprc_blk[pid][buf_idx].addr;
				p_img->addr[1] = p_hdl->vprc_blk[pid][buf_idx].addr + (p_img->loff[0] * p_img->size.h);
				p_img->phyaddr[0] = nvtmpp_sys_va2pa(p_img->addr[0]);
				p_img->phyaddr[1] = nvtmpp_sys_va2pa(p_img->addr[1]);
			} else {
				p_img->enable = DISABLE;
			}

		}

	} else if (pid == 4) {
		/* todo: ime referenct out */

	} else if (pid == 5) {
		/* todo: dce cfa out */
		p_img->enable = DISABLE;
	}

	if (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) {
		p_img->timestamp = p_hdl->fs_timestamp;
	} else if (p_hdl->p_cur_job) {
		p_job = (KDRV_IPP_BUILTIN_JOB *)p_hdl->p_cur_job;
		p_img->timestamp = p_job->timestamp;
	}

	return img;
}

BOOL kdrv_ipp_builtin_push_ll_frm(KDRV_IPP_BUILTIN_HDL *p_hdl){

	UINT32 i = 0;
	UINT32 j = 0;
	KDRV_IPP_BUILTIN_FMD_CB_INFO info = {0};
	KDRV_IPP_BUILTIN_CTL *p_ctl = NULL;
	BOOL cb_found = 0;

	p_ctl = &g_kdrv_ipp_builtin_ctl;

	for (i = 0; i < KDRV_IPP_BUILTIN_FMD_CB_NUM; i++) {
		if (p_ctl->fmd_cb[i]) {
			cb_found = 1;
		}
	}

	if(cb_found == 0){
		DBG_ERR("-fmd_cb not registered-\n");
		return FALSE;
	}

	for(j = 0; j < p_hdl->ll_queue_num; j++){

		info.name = p_hdl->name;
		info.isp_id = p_hdl->isp_id;
		for (i = 0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++) {
			info.out_img[i] = kdrv_ipp_builtin_get_buf_info(info.isp_id, i, j);
			info.out_img[i].timestamp = p_hdl->switch_timestamp - (p_hdl->frm_interval_us * (p_hdl->ll_queue_num - j - 1) );
		}

		//if(j == 14 || j == 15)
		//	info.release_flg = 1;

		for (i = 0; i < KDRV_IPP_BUILTIN_FMD_CB_NUM; i++) {
			if (p_ctl->fmd_cb[i]) {
				p_ctl->fmd_cb[i](&info, 0);
			}
		}

		/*release linked list yuv queue idx 2 ~ queue_num*/
		if(j >= 2){
			for (i = 0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++){
				if(info.out_img[i].addr[0])
					if(nvtmpp_unlock_fastboot_blk(info.out_img[i].addr[0]) != 0)
						DBG_ERR("nvtmpp_unlock_fastboot_blk fail addr:%lx\n", (ULONG)info.out_img[i].addr[0]);
			}
		}

	}

	return TRUE;
}
void kdrv_ipp_builtin_ime_isr(UINT32 sts)
{
	static BOOL ime_1st_frame_end_skip = FALSE;
	KDRV_IPP_BUILTIN_CTL *p_ctl;
	KDRV_IPP_BUILTIN_HDL *p_hdl;
	KDRV_IPP_BUILTIN_JOB *p_job;
	KDRV_IPP_BUILTIN_FMD_CB_INFO info = {0};
	KDRV_IPP_BUILTIN_DBG_DRAW_INFO draw_buf_info = {0};
	UINT32 i = 0;

	VOS_TICK fed_tick=0;
	static VOS_TICK fs_pre_tick=0;
	static VOS_TICK fed_pre_tick=0;

	p_ctl = &g_kdrv_ipp_builtin_ctl;
	p_hdl = p_ctl->p_trig_hdl;
	if (p_hdl == NULL) {
		DBG_ERR("null handle in ime isr\r\n");
		return ;
	}

	if (sts & IME_BUILTIN_INTS_FB_FRM_END) {
		p_hdl->out_buf_release_cnt++;
	}

	if (sts & IME_BUILTIN_INTS_FRM_END) {

		if((p_hdl->builtin_ipp_frm_ed_start == 1) || (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_RAW) || (p_hdl->rtos_ipp_en == 0)){

			vos_perf_mark(&fed_tick);

			p_hdl->dbg_info.output_frm_interval = fed_tick - fed_pre_tick;
			p_hdl->dbg_info.output_cnt++;
			fed_pre_tick = fed_tick;

			p_hdl->fed_cnt++;
			kdrv_ipp_builtin_save_queue_blk_info(p_hdl);

			/* log first frame end timestamp */
			if (p_hdl->fs_cnt == 1) {
				if((kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IFE, IFE_FB_INT_STS_OFS) & IFE_FB_INT_ERR_CHK_BIT)) {
					ime_1st_frame_end_skip = TRUE;
					kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IFE, IFE_FB_INT_STS_OFS, 0xffffffff);	//clear all inter
				} else {
					nvt_bootts_add_ts("IME"); //end
				}
			} else {
				if (ime_1st_frame_end_skip == TRUE) {
					ime_1st_frame_end_skip = FALSE;
					nvt_bootts_add_ts("IME"); //end
				}
			}

			if(g_kdrv_ipp_isp_cb)
				g_kdrv_ipp_isp_cb(p_hdl->isp_id, KDRV_IPP_BUILTIN_ISP_EVENT_FRM_ED);

			/* update isp info */
			kdrv_ipp_builtin_upd_isp_info(p_hdl);

			/* update lca gray avg */
			//kdrv_ipp_builtin_upd_lca_gray_avg(p_hdl); 530 removed

			/* Check register setting is applied on this frame successfully or not */
			if( !kdrv_ipp_check_ime_dram_status(p_hdl) ){
				p_hdl->dbg_info.apply_fail_cnt++;
			}

			if (ime_1st_frame_end_skip == FALSE) {
				/* frame end callback flow */
				info.name = p_hdl->name;
				info.isp_id = p_hdl->isp_id;

				if (p_hdl->flow != KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW && p_hdl->p_cur_job) {
					p_job = (KDRV_IPP_BUILTIN_JOB *)p_hdl->p_cur_job;
					if (p_job->buf_ctrl == SIE_BUILTIN_HEADER_CTL_PUSH) {
						p_hdl->out_buf_release_cnt++;
					}
				}

				info.release_flg = (p_hdl->out_buf_release_cnt > 0) ? 1 : 0;

				for (i = 0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++) {
					info.out_img[i] = kdrv_ipp_builtin_get_path_info(info.isp_id, i);

					/* lock 3dnr last buffer for dram mode */
					if (p_hdl->out_buf_release_cnt == 2) {
						if ((p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_3DNR) && (p_hdl->flow != KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) &&
							i == p_hdl->_3dnr_ref_path && info.out_img[i].enable && info.out_img[i].addr[0]) {

							if(p_hdl->stitch_en[i]){

								if(kdrv_ipp_builtin_chech_do_stitch()){

									KDRV_IPP_BUILTIN_DUMP("FASTBOOTasfadf LOCK 3DNR REF 0x%.8lx\r\n", info.out_img[i].addr[0]);
									nvtmpp_lock_fastboot_blk(info.out_img[i].addr[0]);
									p_ctl->_3dnr_lock_addr = info.out_img[i].addr[0];
									p_ctl->p_hdl[0].dbg_info._3dnr_lock_cnt++;
								}

							}else{
								KDRV_IPP_BUILTIN_DUMP("FASTBOOTasfadf LOCK 3DNR REF 0x%.8lx\r\n", info.out_img[i].addr[0]);
								nvtmpp_lock_fastboot_blk(info.out_img[i].addr[0]);
								p_ctl->_3dnr_lock_addr = info.out_img[i].addr[0];
								p_ctl->p_hdl[0].dbg_info._3dnr_lock_cnt++;
							}

							p_hdl->lock_cnt++;
							p_hdl->_3dnr_lock_addr = info.out_img[i].addr[0];
						}
					}

					/* Do frame rate control */
					kdrv_ipp_builtin_frc(p_hdl, &info, i);

					// Do yuv queue mode setting
					if(info.out_img[i].enable){

						if((p_hdl->queue_num[i]!= 0)){

							if (p_hdl->queue_ring_mode_en) {
								p_hdl->queue_out_idx[i] = (p_hdl->queue_out_idx[i] + 1) % p_hdl->queue_num[i];
							} else {
								p_hdl->queue_out_idx[i] = min(p_hdl->queue_out_idx[i] + 1, p_hdl->queue_num[i]); // clamp to extra buffer
							}
							if( (p_hdl->ring_flag[i]) == 0 && (p_hdl->queue_out_idx[i] == 0))
								p_hdl->ring_flag[i] = 1;
						}
					} else{
						if((p_hdl->queue_num[i]!= 0)){
							if(p_hdl->out_buf_release_cnt == 2){
								//yuv queue mode, if queue already ringed and builtin last frame need to drop, update queue index for indicate to correct queue head
								if (p_hdl->queue_ring_mode_en) {
									p_hdl->queue_out_idx[i] = (p_hdl->queue_out_idx[i] + 1) % p_hdl->queue_num[i];
								} else {
									p_hdl->queue_out_idx[i] = min(p_hdl->queue_out_idx[i] + 1, p_hdl->queue_num[i]); // clamp to extra buffer
								}
								if( (p_hdl->ring_flag[i]) == 0 && (p_hdl->queue_out_idx[i] == 0))
									p_hdl->ring_flag[i] = 1;
							}
						}
					}

					/* Paste stamp*/
					draw_buf_info.p_hdl = p_hdl;
					draw_buf_info.draw_buf_info.w = info.out_img[i].size.w;
					draw_buf_info.draw_buf_info.h=  info.out_img[i].size.h;
					draw_buf_info.draw_buf_info.lofs = info.out_img[i].loff[0];
					draw_buf_info.draw_buf_info.ptr = (void *)info.out_img[i].addr[0];
					draw_buf_info.frm_skip_flag = ( ~(info.out_img[i].enable) ) & 0x01;

					if(info.out_img[i].addr[0]){
						kdrv_ipp_builtin_dbg(&g_kdrv_ipp_builtin_ctl, DBG_STAMP_PASTE, &draw_buf_info);
						vos_cpu_dcache_sync((ULONG)draw_buf_info.draw_buf_info.ptr, (draw_buf_info.draw_buf_info.w = info.out_img[i].size.w << 8), VOS_DMA_TO_DEVICE);
					}

				}

				if(p_hdl->dbg_func_en & KDRV_IPP_PARAM_BUILTIN_SET_PTS_MSG_EN){
					DBG_DUMP("-[%d,%d,%d,%d]-\n", info.out_img[0].enable,
												info.out_img[1].enable,
												info.out_img[2].enable,
												info.out_img[3].enable);
				}


				p_hdl->frm_done = 1;
				p_hdl->stitch_frm_sync_flg = 1;

				if(kdrv_builtin_is_fastboot_yuvque_en() == 0 || ((kdrv_builtin_is_fastboot_yuvque_en() == 1) && p_hdl->queue_h264_mode_en)){
					for (i = 0; i < KDRV_IPP_BUILTIN_FMD_CB_NUM; i++) {
						if (p_ctl->fmd_cb[i]  || p_hdl->fmd_cb[i]) {
							if(p_hdl->stitch_en[0]){

								if(kdrv_ipp_builtin_chech_do_stitch()){

									if(p_ctl->fmd_cb[i]){
										info.name = p_ctl->p_hdl[0].name;
										info.isp_id = p_ctl->p_hdl[0].isp_id & 0xff;
										p_ctl->fmd_cb[i](&info, 0);
									}

									if(p_hdl->fmd_cb[i]){
										info.name = p_hdl->name;
										info.isp_id = p_hdl->isp_id & 0xff;
										p_hdl->fmd_cb[i](&info, 0);
									}
								}

							}else{

								if(p_ctl->fmd_cb[i]){
									info.isp_id = p_hdl->isp_id & 0xff;
									p_ctl->fmd_cb[i](&info, 0);
								}

								if(p_hdl->fmd_cb[i]){
									info.isp_id = p_hdl->isp_id & 0xff;
									p_hdl->fmd_cb[i](&info, 0);
								}
							}
						}
					}
				}

				/* release blk when release_flg == 1 */
				if (info.release_flg) {

					DBG_DUMP("-hdl:0x%lx, release_flg=%d-\n", (ULONG)p_hdl, info.release_flg);

					for (i = 0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++) {
						if (info.out_img[i].addr[0]) {

							if(p_hdl->stitch_en[i] && p_hdl->queue_mode_en[i] == 0){

								if(kdrv_ipp_builtin_chech_do_stitch()){
									if(nvtmpp_unlock_fastboot_blk(info.out_img[i].addr[0])!= 0)
										DBG_ERR("nvtmpp_unlock_fastboot_blk fail addr:%lx\n", (ULONG)info.out_img[i].addr[0]);
									p_hdl->dbg_info.unlock_cnt++;
									p_hdl->lock_cnt--;

									DBG_DUMP("------------ unlock yuv out -------------\n");
								}else{
									DBG_DUMP(">>>>>>>>>>> not yet to <<<<<<<<<<\n");
								}

							}else if(p_hdl->queue_mode_en[i] == 0) {
								if(nvtmpp_unlock_fastboot_blk(info.out_img[i].addr[0])!= 0)
									DBG_ERR("nvtmpp_unlock_fastboot_blk fail addr:%lx\n", (ULONG)info.out_img[i].addr[0]);
								p_hdl->dbg_info.unlock_cnt++;
								p_hdl->lock_cnt--;
							}
						}
					}
				}


				if(kdrv_ipp_builtin_chech_do_stitch()){
					kdrv_ipp_builtin_clear_stitch_frm_done_chk();
				}

			}

			/* set jobdone flg for d2d mode */
			if (p_hdl->flow != KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) {
				p_ctl->p_trig_hdl = NULL;
				vos_flag_iset(p_ctl->proc_tsk_flg_id, KDRV_IPP_BUILTIN_TSK_JOBDONE);
			}
		}

		//DBG_DUMP("-linux_frm_ed:%d-\n", p_hdl->fs_cnt);

	}

	if (sts & IME_BUILTIN_INTS_FRM_START) {

		vos_perf_mark(&p_hdl->switch_timestamp);

		if((p_hdl->builtin_ipp_frm_st_start == 1) || (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_RAW) || (p_hdl->rtos_ipp_en == 0)){

			p_hdl->fs_cnt++;
			p_hdl->dbg_info.input_cnt++;

			//DBG_DUMP("-lin_fs:%d-\n", p_hdl->fs_cnt);

			if (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) {
				kdrv_ipp_builtin_cfg_eng_all(p_hdl);
				vos_perf_mark(&p_hdl->fs_timestamp);
				p_hdl->dbg_info.input_frm_interval = p_hdl->fs_timestamp - fs_pre_tick;
				fs_pre_tick = p_hdl->fs_timestamp;
			}
		}

		//DBG_DUMP("-linux_frm_st-\n");

	}
	if(sts & IME_BUILTIN_INTS_JOB_END){

		if (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) {


			UINT32 ts=0;

			vos_perf_mark(&ts);

			DBG_DUMP("---lin_job_ed:%d---\n", ts);

			p_hdl->job_end_cnt++;

			if(p_hdl->switch_start_flg == 0){

				if(kdrv_ipp_builtin_check_rtos_to_builtin(p_hdl)){

					p_hdl->fs_cnt = 3/*p_hdl->ll_queue_num-1*/; //let rtos_ipp switch to builtin_ipp first fs_cnt is 4
					p_hdl->switch_start_flg = 1;
				}
			}else{

				p_hdl->builtin_ipp_frm_st_start = 1;

			}
		}
	}

	if(sts & IME_BUILTIN_INTS_LL_END){

		if (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) {

			UINT32 result = 0;

			if(p_hdl->job_end_cnt >=2){

				kdrv_ipp_builtin_push_ll_frm(p_hdl);
				p_hdl->builtin_ipp_frm_ed_start = 1;

				DBG_DUMP("ll_status:0x%x,0x%x,0x%x,0x%x\n", kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_LL_TAB_0),
						kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_LL_TAB_1),
						kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_LL_TAB_2),
						kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_LL_TAB_3));

				result = (kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, p_hdl->target_reg) >> p_hdl->target_position ) & 0xFF;
				DBG_DUMP("---------reg:0x%x byte:%d,rslt:0x%x ----------\n", p_hdl->target_reg, p_hdl->target_byte, result);
				DBG_DUMP("====== switch success ======\n");
			}
		}
	}

}

KDRV_IPP_BUILTIN_JOB* kdrv_ipp_builtin_job_alloc(void)
{
	unsigned long loc_flg;
	KDRV_IPP_BUILTIN_MEM_POOL *p_mem_pool;
	KDRV_IPP_BUILTIN_JOB *p_job;

	p_mem_pool = &g_kdrv_ipp_builtin_ctl.job_pool;

	vk_spin_lock_irqsave(&p_mem_pool->lock, loc_flg);
	p_job = vos_list_first_entry_or_null(&p_mem_pool->free_list_root, KDRV_IPP_BUILTIN_JOB, pool_list);
	if (p_job) {
		vos_list_del_init(&p_job->pool_list);
		vos_list_add_tail(&p_job->pool_list, &p_mem_pool->used_list_root);
	}
	vk_spin_unlock_irqrestore(&p_mem_pool->lock, loc_flg);

	return p_job;
}

void kdrv_ipp_builtin_job_free(KDRV_IPP_BUILTIN_JOB *p_job)
{
	unsigned long loc_flg;
	KDRV_IPP_BUILTIN_MEM_POOL *p_mem_pool;

	p_mem_pool = &g_kdrv_ipp_builtin_ctl.job_pool;

	vk_spin_lock_irqsave(&p_mem_pool->lock, loc_flg);
	vos_list_del_init(&p_job->pool_list);
	vos_list_add_tail(&p_job->pool_list, &p_mem_pool->free_list_root);
	vk_spin_unlock_irqrestore(&p_mem_pool->lock, loc_flg);
}

KDRV_IPP_BUILTIN_JOB* kdrv_ipp_builtin_get_proc_job(void)
{
	unsigned long loc_flg;
	KDRV_IPP_BUILTIN_JOB *p_job;

	vk_spin_lock_irqsave(&g_kdrv_ipp_builtin_ctl.job_list_lock, loc_flg);
	p_job = vos_list_first_entry_or_null(&g_kdrv_ipp_builtin_ctl.job_list_root, KDRV_IPP_BUILTIN_JOB, proc_list);
	vk_spin_unlock_irqrestore(&g_kdrv_ipp_builtin_ctl.job_list_lock, loc_flg);

	return p_job;
}

KDRV_IPP_BUILTIN_JOB* kdrv_ipp_builtin_pop_proc_job(void)
{
	unsigned long loc_flg;
	KDRV_IPP_BUILTIN_JOB *p_job;

	vk_spin_lock_irqsave(&g_kdrv_ipp_builtin_ctl.job_list_lock, loc_flg);
	p_job = vos_list_first_entry_or_null(&g_kdrv_ipp_builtin_ctl.job_list_root, KDRV_IPP_BUILTIN_JOB, proc_list);
	if (p_job) {
		vos_list_del_init(&p_job->proc_list);
	}
	vk_spin_unlock_irqrestore(&g_kdrv_ipp_builtin_ctl.job_list_lock, loc_flg);

	return p_job;
}

void kdrv_ipp_builtin_del_proc_job(KDRV_IPP_BUILTIN_JOB *p_job)
{
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&g_kdrv_ipp_builtin_ctl.job_list_lock, loc_flg);
	vos_list_del_init(&p_job->proc_list);
	vk_spin_unlock_irqrestore(&g_kdrv_ipp_builtin_ctl.job_list_lock, loc_flg);
}

void kdrv_ipp_builtin_put_job(KDRV_IPP_BUILTIN_JOB *p_job)
{
	unsigned long loc_flg;

	vk_spin_lock_irqsave(&g_kdrv_ipp_builtin_ctl.job_list_lock, loc_flg);
	vos_list_add_tail(&p_job->proc_list, &g_kdrv_ipp_builtin_ctl.job_list_root);
	vk_spin_unlock_irqrestore(&g_kdrv_ipp_builtin_ctl.job_list_lock, loc_flg);
}

void kdrv_ipp_builtin_flush_job(void)
{
	KDRV_IPP_BUILTIN_JOB *p_job;

	while ((p_job = kdrv_ipp_builtin_pop_proc_job()) != NULL) {
		DBG_ERR("fastboot queue job flushed\r\n");
	}
}


BOOL kdrv_ipp_builtin_do_frm_sync(KDRV_IPP_BUILTIN_HDL* p_hdl)
{

	if(p_hdl->stitch_en[0]){ // stitch mode

		if(p_hdl == g_kdrv_ipp_builtin_ctl.p_frm_sync_master){

			if(p_hdl->stitch_frm_sync_flg == 0)
				return TRUE;
			else
				return FALSE;

		}else{

			if(g_kdrv_ipp_builtin_ctl.p_frm_sync_master->stitch_frm_sync_flg == 1){
				g_kdrv_ipp_builtin_ctl.p_frm_sync_master->stitch_frm_sync_flg = 0;
				return TRUE;
			}else{

				return FALSE;
			}

		}

	}else{ // no stitch mode

		return TRUE;
	}

}

void kdrv_ipp_builtin_sie_cb(UINT32 id, SIE_BUILTIN_HEADER_INFO *p_info)
{
	KDRV_IPP_BUILTIN_HDL *p_hdl;
	KDRV_IPP_BUILTIN_JOB *p_job;
	UINT32 i;
	unsigned long loc_flg;

	/* todo: hdr collect two frame */
	/* find handle with corresponding src_sie_id */

	vk_spin_lock_irqsave(&g_kdrv_ipp_builtin_ctl.sie_cb_lock, loc_flg);

	p_hdl = NULL;
	for (i = 0; i < g_kdrv_ipp_builtin_ctl.hdl_num; i++) {
		if (g_kdrv_ipp_builtin_ctl.p_hdl[i].src_sie_id_bit & (1 << id)) {
			p_hdl = &g_kdrv_ipp_builtin_ctl.p_hdl[i];
			break;
		}
	}

	if (p_hdl == NULL) {
		DBG_ERR("hdl null\r\n");
		vk_spin_unlock_irqrestore(&g_kdrv_ipp_builtin_ctl.sie_cb_lock, loc_flg);
		return ;
	}

	vos_perf_mark(&p_hdl->fs_timestamp);
	p_hdl->dbg_info.input_frm_interval = p_hdl->fs_timestamp - p_hdl->pre_fs_timestamp;
	p_hdl->pre_fs_timestamp = p_hdl->fs_timestamp;

	p_hdl->dbg_info.sensor_cnt++;

	if (p_info->buf_addr && p_info->addr_ch0) {
		p_job = kdrv_ipp_builtin_job_alloc();
		if (p_job == NULL) {
			DBG_ERR("job queue full\r\n");
			vk_spin_unlock_irqrestore(&g_kdrv_ipp_builtin_ctl.sie_cb_lock, loc_flg);
			return ;
		}

		p_job->p_owner   = p_hdl;
		p_job->blk[0]    = p_info->buf_addr;
		p_job->blk[1]    = 0;
		p_job->blk_2[0]  = p_info->frm2_buf_addr;
		p_job->blk_2[1]  = 0;
		p_job->va[0]     = p_info->addr_ch0;
		p_job->va[1]     = p_info->addr_ch1;
		p_job->va_2[0]   = p_info->frm2_addr_ch0;
		//p_job->va_2[1]  = p_info->frm2_addr_ch1;
		p_job->pa[0] = p_info->addr_ch0_pa;
		p_job->pa[1] = p_info->addr_ch1_pa;
		p_job->pa_2[0] = p_info->frm2_addr_ch0_pa;
		//p_job->pa_2[1] = p_info->frm2_addr_ch1_pa;
		p_job->buf_ctrl  = p_info->buf_ctrl;
		p_job->timestamp = p_info->timestamp;
		p_job->count = p_info->count;

		if (p_job->buf_ctrl == SIE_BUILTIN_HEADER_CTL_PUSH) {
			KDRV_IPP_BUILTIN_DUMP("fastboot isp_id %d receive sie blk 0x%.8x, 0x%.8x for release blk\r\n",
				p_hdl->isp_id, (unsigned int)p_job->blk[0], (unsigned int)p_job->blk[1]);
		}

		kdrv_ipp_builtin_put_job(p_job);
		vos_flag_iset(g_kdrv_ipp_builtin_ctl.proc_tsk_flg_id, KDRV_IPP_BUILTIN_TSK_TRIGGER);
	} else {
		DBG_ERR("sie(%d) buf_addr 0x%.8x, ch0_addr 0x%.8x\r\n", (int)id, (unsigned int)p_info->buf_addr, (unsigned int)p_info->addr_ch0);
	}

	vk_spin_unlock_irqrestore(&g_kdrv_ipp_builtin_ctl.sie_cb_lock, loc_flg);

	return ;
}

BOOL kdrv_ipp_builtin_check_src_valid(KDRV_IPP_BUILTIN_HDL *p_hdl)
{
	KDRV_IPP_BUILTIN_CTL *p_ctl = &g_kdrv_ipp_builtin_ctl;

	if (p_hdl->src_sie_id_bit == 0) {
		return FALSE;
	}

       /* all src id must be valid */
	if ((p_hdl->src_sie_id_bit & p_ctl->valid_src_id_bit) == p_hdl->src_sie_id_bit) {
		return TRUE;
	}

	return FALSE;
}

THREAD_DECLARE(kdrv_ipp_builtin_ll_switch_tsk, p1)
{
	KDRV_IPP_BUILTIN_CTL *p_ctl;
	KDRV_IPP_BUILTIN_HDL *p_hdl;

	p_ctl = &g_kdrv_ipp_builtin_ctl;
	p_hdl = &p_ctl->p_hdl[0];

	THREAD_ENTRY();

	if(1/*p_hdl->job_end_cnt == 0*/){

		UINT32 i = 0;
		UINT32 regs[4] = {0};
		UINT32 tm0_begin = 0;
		UINT32 tm0_end =  0/*(UINT32)nvt_readl(NVT_TIMER_BASE_VIRT + NVT_TIMER_TM0_CNT)*/;
		BOOL no_ll_flg = 1;

		kdrv_bridge_get_tag(IPP_LL_START_TIME, &tm0_begin);
		for (i = 0; i < 4; i++) {
			regs[i] =  kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_LL_TAB_0 + (i << 2));
		}


		for(i=0; i < 65; i++){  // polling 65 ms for 2 linked list frm end to switch success

			if(p_hdl->job_end_cnt >=2){
				no_ll_flg = 0;
				break;
			}

			vos_util_delay_ms(1);
		}

		if(no_ll_flg == 1){

			DBG_DUMP("========= ERR: ll not enough : ll_detected_cnt = %d =========\n", p_hdl->job_end_cnt);
			DBG_ERR("begin:%d, end:%d, diff:%d\r\n", tm0_begin, tm0_end, tm0_end-tm0_begin);
			DBG_ERR("ll_idx_regs: %08X %08X %08X %08X\r\n", regs[0], regs[1], regs[2], regs[3]);

			DBG_DUMP("ll_status:0x%x,0x%x,0x%x,0x%x\n", kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_LL_TAB_0),
					kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_LL_TAB_1),
					kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_LL_TAB_2),
					kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_LL_TAB_3));

			kdrv_ipp_builtin_push_ll_frm(p_hdl);

			//vos_util_delay_ms(30);

			p_hdl->fs_cnt = 0;
			p_hdl->builtin_ipp_frm_st_start = 1;
			p_hdl->builtin_ipp_frm_ed_start = 1;
		}else{

		}
	}

	THREAD_RETURN(0);
}

THREAD_DECLARE(kdrv_ipp_builtin_tsk, p1)
{
	FLGPTN wait_flg;
	KDRV_IPP_BUILTIN_CTL *p_ctl;
	KDRV_IPP_BUILTIN_HDL *p_hdl;
	KDRV_IPP_BUILTIN_JOB *p_job;
	INT32 rt;
	UINT8 i;
	UINT8 fastboot_done;

	p_ctl = &g_kdrv_ipp_builtin_ctl;

	THREAD_ENTRY();

	while (!THREAD_SHOULD_STOP) {
		vos_flag_set(p_ctl->proc_tsk_flg_id, KDRV_IPP_BUILTIN_TSK_RESUME_END);
		PROFILE_TASK_IDLE();

		/* receive job from list */
		while (1) {
			p_job = kdrv_ipp_builtin_pop_proc_job();
			if (p_job == NULL) {
				vos_flag_wait(&wait_flg, p_ctl->proc_tsk_flg_id, KDRV_IPP_BUILTIN_TSK_TRIGGER, TWF_CLR);
			} else {
				break;
			}
		}

		/* process job */
		PROFILE_TASK_BUSY();
		p_hdl = p_job->p_owner;
		p_hdl->p_cur_job = (void *)p_job;

		if(kdrv_ipp_builtin_do_frm_sync(p_hdl)){

			rt = kdrv_ipp_builtin_cfg_eng_all(p_hdl);
			if (rt != 0 || (p_hdl->input_disable_flag == 1) ) {

				if(p_hdl->input_disable_flag == 0){
					/* config failed drop frame */
					KDRV_IPP_BUILTIN_DUMP("config failed drop frame!!\n");
				}
			} else {
				/* trigger engine, wait job end */
				vos_flag_clr(p_ctl->proc_tsk_flg_id, KDRV_IPP_BUILTIN_TSK_JOBDONE);
				kdrv_ipp_builtin_trig_eng_start(p_hdl);
				vos_flag_wait(&wait_flg, p_ctl->proc_tsk_flg_id, KDRV_IPP_BUILTIN_TSK_JOBDONE, TWF_CLR);
			}

			/* release buffer, check if all buffer release and all handle release */
			if (p_job->buf_ctrl == SIE_BUILTIN_HEADER_CTL_PUSH) {
				if(nvtmpp_unlock_fastboot_blk(p_job->blk[0])!= 0)
					DBG_ERR("nvtmpp_unlock_fastboot_blk fail addr:%lx\n", (ULONG)p_job->blk[0]);

				if (p_job->blk[1]) {
					if(nvtmpp_unlock_fastboot_blk(p_job->blk[1])!= 0)
						DBG_ERR("nvtmpp_unlock_fastboot_blk fail addr:%lx\n", (ULONG)p_job->blk[1]);

				}
				p_hdl->in_buf_release_cnt++;
				KDRV_IPP_BUILTIN_DUMP("fastboot isp_id %d release sie blk 0x%.8x, 0x%.8x, release_cnt %d\r\n",
					p_hdl->isp_id, (unsigned int)p_job->blk[0], (unsigned int)p_job->blk[1], (int)p_hdl->in_buf_release_cnt);

				if ( (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_RAW) && (p_hdl->func_en  & KDRV_IPP_BUILTIN_FUNC_SHDR)){

					if(nvtmpp_unlock_fastboot_blk(p_job->blk_2[0])!= 0)
						DBG_ERR("nvtmpp_unlock_fastboot_blk fail addr:%lx\n", (ULONG)p_job->blk_2[0]);

					if (p_job->blk_2[1]) {
						if(nvtmpp_unlock_fastboot_blk(p_job->blk_2[1])!= 0)
							DBG_ERR("nvtmpp_unlock_fastboot_blk fail addr:%lx\n", (ULONG)p_job->blk_2[1]);
					}

					KDRV_IPP_BUILTIN_DUMP("fastboot hdl %s release hdr_sie blk 0x%.8x, 0x%.8x, release_cnt %d\r\n",
						p_hdl->name, (unsigned int)p_job->blk_2[0], (unsigned int)p_job->blk_2[1], (int)p_hdl->in_buf_release_cnt);
				}

				fastboot_done = TRUE;
				for (i = 0; i < p_ctl->hdl_num; i++) {
					/* only check hdl with valid src_id */
					if (kdrv_ipp_builtin_check_src_valid(&p_ctl->p_hdl[i])) {
						fastboot_done &= (p_ctl->p_hdl[i].in_buf_release_cnt >= KDRV_IPP_BUILTIN_BUF_SIE);
					}

				}
				if (fastboot_done) {
					/* release job and exit task */
					kdrv_ipp_builtin_job_free(p_job);
					goto IPP_TSK_DESTROY;
				}
			}

			/* release job */
			p_hdl->p_cur_job = NULL;
			kdrv_ipp_builtin_job_free(p_job);
		}else{
			/* drop job for frm sync */
			p_hdl->p_cur_job = NULL;
			kdrv_ipp_builtin_job_free(p_job);
		}
	}

IPP_TSK_DESTROY:
	vos_flag_set(p_ctl->proc_tsk_flg_id, (KDRV_IPP_BUILTIN_TSK_EXIT_END | KDRV_IPP_BUILTIN_TSK_FASTBOOT_DONE));
	THREAD_RETURN(0);
}

INT32 kdrv_ipp_builtin_init_out_path_info(KDRV_IPP_BUILTIN_HDL *p_hdl)
{
#if 1//(KDRV_IPP_RTOS_IPP_ENABLE == 0) //fb2.0
	CHAR regnode[75];
	UINT32 reg_cfg_num;
	UINT32 *p_reg_buf;
	UINT32 ofs;
	UINT32 val;
#endif
	UINT32 cfg_eng_bit;
	UINT32 eng;
	UINT32 i;

	KDRV_IPP_BUILTIN_IMG_INFO ime_path_info = {0};
	KDRV_IPP_BUILTIN_IMG_INFO *p_path;

	cfg_eng_bit = kdrv_ipp_builtin_get_cfg_eng_bit(p_hdl);
	eng = KDRV_IPP_BUILTIN_IME;
	if ((cfg_eng_bit & KDRV_IPP_BUILTIN_ENG_BIT(eng)) == 0) {
		return 0;
	}

	if(p_hdl->rtos_ipp_en == 0){  //fb2.0 bypass secondary read dtsi

		snprintf(regnode, 75, "%s/register", p_hdl->node);

		reg_cfg_num = kdrv_ipp_builtin_get_reg_num(KDRV_IPP_BUILTIN_IME);
		p_reg_buf = kdrv_ipp_builtin_get_dtsi_buf(p_hdl, KDRV_IPP_BUILTIN_IME);

		for (i = 0; i < reg_cfg_num; i++) {
			ofs = p_reg_buf[i * 2];
			val = p_reg_buf[(i * 2) + 1];

			/* skip engine ctrl regiser */
			if (ofs == 0) {
				continue;
			} else {
				/* write config to engine */
				//DBG_DUMP("set:%x,%x,%x\n", eng,ofs,val);
				kdrv_ipp_builtin_set_reg(eng, ofs, val);
			}
		}
	}else{

		KDRV_IPP_BUILTIN_DUMP("-by pass 2nd read dtsi when fb2.0-\n");
	}

	/* path 1~4 info */
	for (i = KDRV_IPP_BUILTIN_PATH_ID_1; i <= KDRV_IPP_BUILTIN_PATH_ID_4; i++) {
		ime_builtin_get_path_info(i, &ime_path_info);
    //tmp remove for build
		p_path = &p_hdl->path_info[i];
		p_path->enable = ime_path_info.enable;
		if (p_path->enable) {
			p_path->size.w  = ime_path_info.size.w;
			p_path->size.h  = ime_path_info.size.h;
			p_path->fmt     = ime_path_info.fmt ;
			p_path->loff[0] = ime_path_info.loff[0];
			p_path->loff[1] = ime_path_info.loff[1];
		}

		KDRV_IPP_BUILTIN_DUMP("path%d, en %d, size(%4d, %4d), lofs(%4d %4d), fmt 0x%.8x\r\n",
			(int)i, (int)p_path->enable, (int)p_path->size.w, (int)p_path->size.h,
			(int)p_path->loff[0], (int)p_path->loff[1], (unsigned int)p_path->fmt);
	}

	/* reference out info */
	ime_builtin_get_path_info(KDRV_IPP_BUILTIN_PATH_ID_5, &ime_path_info);
	p_path = &p_hdl->path_info[KDRV_IPP_BUILTIN_PATH_ID_5];
	p_path->enable = ime_path_info.enable;
	if (p_path->enable) {
		p_path->size.w = ime_path_info.size.w;
		p_path->size.h = ime_path_info.size.h;
		p_path->fmt = kdrv_ipp_builtin_typecast_fmt(KDRV_IPP_BUILTIN_PATH_ID_5, NULL);
		p_path->loff[0] = ime_path_info.loff[0];
		p_path->loff[1] = ime_path_info.loff[1];
	}

	KDRV_IPP_BUILTIN_DUMP("path%d for 3dnr reference out path, en %d, size(%4d, %4d), lofs(%4d %4d), fmt 0x%.8x\r\n\r\n", KDRV_IPP_BUILTIN_PATH_ID_5,
			(int)p_path->enable,  (int)p_path->size.w, (int)p_path->size.h,
			(int)p_path->loff[0], (int)p_path->loff[1], (unsigned int)p_path->fmt);


	return 0;
}
INT32 kdrv_ipp_builtin_init_frc(KDRV_IPP_BUILTIN_HDL* p_hdl)
{
	INT32 rt = 0;
	UINT32 i;
	UINT32 tmp[KDRV_IPP_BUILTIN_PATH_ID_MAX];

	rt = kdrv_ipp_builtin_set_child_nodeofsset(&p_hdl->root_node, "frm-rate-ctrl");
	if (rt != 0) {
		DBG_ERR("set frm-rate-ctrl child_nodeofsset failed\r\n");
		return E_SYS;
	}

	rt = kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "skip", (UINT32 *)tmp, KDRV_IPP_BUILTIN_PATH_ID_MAX);
	if (rt != 0) {
		DBG_WRN("read ipp %s/skip failed\r\n", "frm-rate-ctrl");
	}
	for (i = 0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++) {
		p_hdl->frc[i].skip = (rt == 0) ? tmp[i] : 0;
	}

	rt =  kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "src", (UINT32 *)tmp, KDRV_IPP_BUILTIN_PATH_ID_MAX);
	if (rt != 0) {
		DBG_WRN("read ipp %s/src failed\r\n", "frm-rate-ctrl");
	}
	for (i = 0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++) {
		p_hdl->frc[i].src = (rt == 0) ? tmp[i] : 30;
	}

	rt =  kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "dst", (UINT32 *)tmp, KDRV_IPP_BUILTIN_PATH_ID_MAX);
	if (rt != 0) {
		DBG_WRN("read ipp %s/dst failed\r\n", "frm-rate-ctrl");
	}
	for (i = 0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++) {
		p_hdl->frc[i].dst = (rt == 0) ? tmp[i] : 30;

		if (p_hdl->frc[i].src && (p_hdl->frc[i].src != p_hdl->frc[i].dst)) {
			p_hdl->frc[i].drop_rate = (KDRV_IPP_BUILTIN_FRC_BASE * p_hdl->frc[i].src) / (p_hdl->frc[i].src - p_hdl->frc[i].dst);
		} else {
			p_hdl->frc[i].drop_rate = 0;
		}
		p_hdl->frc[i].rate_cnt = 0;
		p_hdl->frc[i].rate_cnt_yuv_que = 0;

		KDRV_IPP_BUILTIN_DUMP("path %d: skip %d; frc dst/src = %d/%d\r\n", i, p_hdl->frc[i].skip, p_hdl->frc[i].dst, p_hdl->frc[i].src);
	}

	return E_OK;
}

INT32 kdrv_ipp_builtin_init_privacy_mask(KDRV_IPP_BUILTIN_HDL* p_hdl)
{
	//USIZE ipp_in_size;
	UINT32 buf[KDRV_IPP_BUILTIN_PRI_MASK_NUM * 20];
	UINT32 i = 0;
	UINT32 j = 0;
	INT32 rt = 0;
	//UINT32 j = 0;
	//UINT32 tmp = 0;
	//UINT8 chk_limit = 0;

	rt = kdrv_ipp_builtin_set_child_nodeofsset(&p_hdl->root_node, "privacy-mask");
	if (rt != 0) {
		DBG_ERR("set privacy-mask child_nodeofsset failed\r\n");
		return E_SYS;
	}

	/* parse dtsi to struct first */
	/* enable */
	if (kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "enable", (UINT32 *)buf, KDRV_IPP_BUILTIN_PRI_MASK_NUM)) {
		DBG_ERR("read dtsi pm_en failed\r\n");
		goto MASK_INIT_ERR;
	}
	for (i = 0; i < KDRV_IPP_BUILTIN_PRI_MASK_NUM; i++) {
		p_hdl->mask[i].enable = buf[i];
	}

	/* poly_sel, each mask has 4 coordinate(x, y) */
	if (kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "poly_sel", (UINT32 *)buf, KDRV_IPP_BUILTIN_PRI_MASK_NUM)) {
		DBG_ERR("read dtsi poly_sel failed\r\n");
		goto MASK_INIT_ERR;
	}
	for (i = 0; i < KDRV_IPP_BUILTIN_PRI_MASK_NUM; i++) {
		p_hdl->mask[i].poly_sel = buf[i];
	}

	/* coordinate, each mask has 4 coordinate(x, y) */
	if (kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "coordinate", (UINT32 *)buf, KDRV_IPP_BUILTIN_PRI_MASK_NUM * 20)) {
		DBG_ERR("read dtsi pm coord failed\r\n");
		goto MASK_INIT_ERR;
	}

	for (i = 0; i < KDRV_IPP_BUILTIN_PRI_MASK_NUM; i++) {
		p_hdl->mask[i].coord[0].x = buf[i * 20];
		p_hdl->mask[i].coord[0].y = buf[(i * 20) + 1];
		p_hdl->mask[i].coord[1].x = buf[(i * 20) + 2];
		p_hdl->mask[i].coord[1].y = buf[(i * 20) + 3];
		p_hdl->mask[i].coord[2].x = buf[(i * 20) + 4];
		p_hdl->mask[i].coord[2].y = buf[(i * 20) + 5];
		p_hdl->mask[i].coord[3].x = buf[(i * 20) + 6];
		p_hdl->mask[i].coord[3].y = buf[(i * 20) + 7];
		p_hdl->mask[i].coord[4].x = buf[(i * 20) + 8];
		p_hdl->mask[i].coord[4].y = buf[(i * 20) + 9];
		p_hdl->mask[i].coord[5].x = buf[(i * 20) + 10];
		p_hdl->mask[i].coord[5].y = buf[(i * 20) + 11];
		p_hdl->mask[i].coord[6].x = buf[(i * 20) + 12];
		p_hdl->mask[i].coord[6].y = buf[(i * 20) + 13];
		p_hdl->mask[i].coord[7].x = buf[(i * 20) + 14];
		p_hdl->mask[i].coord[7].y = buf[(i * 20) + 15];
		p_hdl->mask[i].coord[8].x = buf[(i * 20) + 16];
		p_hdl->mask[i].coord[8].y = buf[(i * 20) + 17];
		p_hdl->mask[i].coord[9].x = buf[(i * 20) + 18];
		p_hdl->mask[i].coord[9].y = buf[(i * 20) + 19];

	}

	/* alpha */
	if (kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "alpha", (UINT32 *)buf, KDRV_IPP_BUILTIN_PRI_MASK_NUM)) {
		DBG_ERR("read dtsi pm_alpha failed\r\n");
		goto MASK_INIT_ERR;
	}
	for (i = 0; i < KDRV_IPP_BUILTIN_PRI_MASK_NUM; i++) {
		p_hdl->mask[i].weight = buf[i];
	}

	/* yuv color, each mask has 3 component(y, u, v) */
	if (kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "color", (UINT32 *)buf, KDRV_IPP_BUILTIN_PRI_MASK_NUM * 3)) {
		DBG_ERR("read dtsi pm_color failed\r\n");
		goto MASK_INIT_ERR;
	}

	for (i = 0; i < KDRV_IPP_BUILTIN_PRI_MASK_NUM; i++) {
		p_hdl->mask[i].color[0] = buf[i * 3];
		p_hdl->mask[i].color[1] = buf[(i * 3) + 1];
		p_hdl->mask[i].color[2] = buf[(i * 3) + 2];
	}

	/* hlw_enable */
	if (kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "hlw_enable", (UINT32 *)buf, KDRV_IPP_BUILTIN_PRI_MASK_NUM)) {
		DBG_ERR("read dtsi pm_hlw_en failed\r\n");
		goto MASK_INIT_ERR;
	}
	for (i = 0; i < KDRV_IPP_BUILTIN_PRI_MASK_NUM; i++) {
		p_hdl->mask[i].hlw_enable = buf[i];
	}

	/* hlw_coordinate, each mask has 4 coordinate(x, y) */
	if (kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "hlw_coordinate", (UINT32 *)buf, KDRV_IPP_BUILTIN_PRI_MASK_NUM * 8)) {
		DBG_ERR("read dtsi pm_hlw_coord failed\r\n");
		goto MASK_INIT_ERR;
	}
	for (i = 0; i < KDRV_IPP_BUILTIN_PRI_MASK_NUM; i++) {
		p_hdl->mask[i].coord2[0].x = buf[i * 8];
		p_hdl->mask[i].coord2[0].y = buf[(i * 8) + 1];
		p_hdl->mask[i].coord2[1].x = buf[(i * 8) + 2];
		p_hdl->mask[i].coord2[1].y = buf[(i * 8) + 3];
		p_hdl->mask[i].coord2[2].x = buf[(i * 8) + 4];
		p_hdl->mask[i].coord2[2].y = buf[(i * 8) + 5];
		p_hdl->mask[i].coord2[3].x = buf[(i * 8) + 6];
		p_hdl->mask[i].coord2[3].y = buf[(i * 8) + 7];
	}

#if 0
	/* limitation check */
	chk_limit = TRUE;
	tmp = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_INPUT_IMAGE_SIZE);
	ipp_in_size.w = tmp & 0xffff;
	ipp_in_size.h = (tmp >> 16) & 0xffff;
	for (i = 0; i < KDRV_IPP_BUILTIN_PRI_MASK_NUM; i++) {
		if (p_hdl->mask[i].hlw_enable && (i & 0x1)) {
			DBG_ERR("privacy mask %d, hollow mask only support PM_SET_0/2/4/6", (int)i);
			chk_limit = FALSE;
		}

		if (p_hdl->mask[i].enable && (i & 0x1)) {
			if (p_hdl->mask[i - 1].hlw_enable) {
				DBG_ERR("privacy mask %d, when PM_SET_0/2/4/6 enable hollow mask, PM_SET_1/3/5/7 can not be used\r\n", (int)i);
				chk_limit = FALSE;
			}
		}

		for (j = 0; j < 4; j++) {
			if (p_hdl->mask[i].enable) {
				if (p_hdl->mask[i].coord[j].x > ipp_in_size.w || p_hdl->mask[i].coord[j].y > ipp_in_size.h) {
					DBG_ERR("privacy mask %d, coord(%d %d) overflow, ipp_in_size(%d, %d)\r\n", (int)i,
								(int)p_hdl->mask[i].coord[j].x, (int)p_hdl->mask[i].coord[j].y, (int)ipp_in_size.w, (int)ipp_in_size.h);
					chk_limit = FALSE;
				}

				if (p_hdl->mask[i].hlw_enable) {
					if (p_hdl->mask[i].coord[j].x > ipp_in_size.w || p_hdl->mask[i].coord[j].y > ipp_in_size.h) {
						DBG_ERR("privacy mask %d, hlw_coord(%d %d) overflow, ipp_in_size(%d, %d)\r\n", (int)i,
									(int)p_hdl->mask[i].coord[j].x, (int)p_hdl->mask[i].coord[j].y, (int)ipp_in_size.w, (int)ipp_in_size.h);
						chk_limit = FALSE;
					}
				}
			}
		}
	}

	if (!chk_limit) {
		DBG_ERR("privacy mask config not illegal, skip mask config\r\n");
		goto MASK_INIT_ERR;
	}
#endif
	/* dump privacy mask information */
	KDRV_IPP_BUILTIN_DUMP("\r\n%8s %4s %16s %4s %16s %6s %10s %10s %10s %8s\r\n",
				"mask_id", "en", "mask_points", "hlw", "hlw_points", "type", "color[0]", "color[1]", "color[2]", "weight");
	for (i = 0; i < KDRV_IPP_BUILTIN_PRI_MASK_NUM; i++) {
		KDRV_IPP_BUILTIN_DUMP("%8d %4d (%6d, %6d) %4d (%6d, %6d) %6d %10d %10d %10d %8d\r\n",
					(int)i,
					(int)p_hdl->mask[i].enable,
					(int)p_hdl->mask[i].coord[0].x,
					(int)p_hdl->mask[i].coord[0].y,
					(int)p_hdl->mask[i].hlw_enable,
					(int)p_hdl->mask[i].coord2[0].x,
					(int)p_hdl->mask[i].coord2[0].y,
					(int)0,
					(int)p_hdl->mask[i].color[0],
					(int)p_hdl->mask[i].color[1],
					(int)p_hdl->mask[i].color[2],
					(int)p_hdl->mask[i].weight);
		if (p_hdl->mask[i].enable) {
			for (j = 1; j < 4; j++) {
				KDRV_IPP_BUILTIN_DUMP("%8s %4s (%6d, %6d) %4s (%6d, %6d)\r\n", " ", " ",
							(int)p_hdl->mask[i].coord[j].x,
							(int)p_hdl->mask[i].coord[j].y,
							" ",
							(int)p_hdl->mask[i].coord2[j].x,
							(int)p_hdl->mask[i].coord2[j].y);
			}

			for (j = 4; j < 10; j++) {
				KDRV_IPP_BUILTIN_DUMP("%8s %4s (%6d, %6d) %4s\r\n", " ", " ",
							(int)p_hdl->mask[i].coord[j].x,
							(int)p_hdl->mask[i].coord[j].y,
							" ");
			}
		}
	}

	return 0;

MASK_INIT_ERR:
	for (i = 0; i < KDRV_IPP_BUILTIN_PRI_MASK_NUM; i++) {
		p_hdl->mask[i].enable = DISABLE;
	}
	return 0;
}

INT32 kdrv_ipp_builtin_alloc_stitch_buf(KDRV_IPP_BUILTIN_HDL *p_hdl)
{
	UINT32 i = 0;
	UINT32 j = 0;
	KDRV_IPP_BUILTIN_IMG_INFO *p_path;

	kdrv_ipp_builtin_init_out_path_info(p_hdl);
	for (i = 0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++) {
			p_path = &p_hdl->path_info[i];
			for (j = 0; j < 2; j++) {
				if (p_path->enable) {

					if(p_hdl->stitch_en[i]){
						g_kdrv_ipp_builtin_ctl.stitch_buf[i][j].size = p_hdl->stitch_buf_y_size[i] + p_hdl->stitch_buf_uv_size[i];
						g_kdrv_ipp_builtin_ctl.stitch_buf[i][j].addr = nvtmpp_get_fastboot_blk(g_kdrv_ipp_builtin_ctl.stitch_buf[i][j].size);
					}
				}

				KDRV_IPP_BUILTIN_DUMP("path_en:%d stitch [%d][%d] addr= 0x%lx, size= %d\n", p_path->enable, i, j,(ULONG) g_kdrv_ipp_builtin_ctl.stitch_buf[i][j].addr, g_kdrv_ipp_builtin_ctl.stitch_buf[i][j].size);
			}
	}

	return 0;
}

void kdrv_ipp_builtin_find_frm_sync_master(void){

	UINT32 i = 0;
	UINT32 j = 0;
	UINT32 target_size = 0;

	for(i=0; i < g_kdrv_ipp_builtin_ctl.hdl_num; i++){  // find biggest size for p_hdl master
		for(j=0; j < KDRV_IPP_BUILTIN_PATH_ID_MAX; j++){

			UINT32 cur_size = g_kdrv_ipp_builtin_ctl.p_hdl[i].path_info[j].size.w * g_kdrv_ipp_builtin_ctl.p_hdl[i].path_info[j].size.h;

			KDRV_IPP_BUILTIN_DUMP("-=======(%d,%d, cur:%d,tar:%d)======-\n",g_kdrv_ipp_builtin_ctl.p_hdl[i].path_info[j].size.w ,g_kdrv_ipp_builtin_ctl.p_hdl[i].path_info[j].size.h, cur_size, target_size );

			if( cur_size > target_size){
				target_size = cur_size;
				g_kdrv_ipp_builtin_ctl.p_frm_sync_master = &g_kdrv_ipp_builtin_ctl.p_hdl[i];
			}

		}

	}

	if(g_kdrv_ipp_builtin_ctl.p_frm_sync_master  == &g_kdrv_ipp_builtin_ctl.p_hdl[0])
		g_kdrv_ipp_builtin_ctl.p_frm_sync_slave = &g_kdrv_ipp_builtin_ctl.p_hdl[1];
	else
		g_kdrv_ipp_builtin_ctl.p_frm_sync_slave = &g_kdrv_ipp_builtin_ctl.p_hdl[0];


	KDRV_IPP_BUILTIN_DUMP("-----frm_sync_master:0x%lx------\n", (ULONG) g_kdrv_ipp_builtin_ctl.p_frm_sync_master);
	KDRV_IPP_BUILTIN_DUMP("-----frm_sync_slave :0x%lx------\n", (ULONG) g_kdrv_ipp_builtin_ctl.p_frm_sync_slave);

}

INT32 kdrv_ipp_builtin_init_handle(KDRV_IPP_BUILTIN_HDL *p_hdl)
{
	ULONG cur_buf_addr;
	UINT32 tmp_buf_size;
	UINT32 i, j, buf_num;
	KDRV_IPP_BUILTIN_IMG_INFO *p_path;
	INT32 rt = 0;

	rt = kdrv_ipp_builtin_set_child_nodeofsset(&p_hdl->root_node, "buffer-size");
	if (rt != 0) {
		DBG_ERR("set buffer-size child_nodeofsset failed\r\n");
		return E_SYS;
	}

	cur_buf_addr = p_hdl->vprc_ctrl.addr;
	if(kdrv_ipp_builtin_plat_read_dtsi_array(p_hdl->node, "func_en", (UINT32 *)&p_hdl->func_en, 1)){
		DBG_ERR("read dtsi func_en error\n");
		return -1;
	}

	KDRV_IPP_BUILTIN_DUMP("isp_id: %d, flow %d, isp_id 0x%.8x, func_en 0x%.8x\r\n",
		p_hdl->isp_id, (int)p_hdl->flow, (unsigned int)p_hdl->isp_id, (unsigned int)p_hdl->func_en);

	if(kdrv_ipp_builtin_plat_read_dtsi_array(p_hdl->node, "3dnr_ref_path", (UINT32 *)&p_hdl->_3dnr_ref_path, 1)){
		DBG_ERR("read dtsi 3dnr_ref_path error\n");
		return -1;
	}
	//if (p_hdl->_3dnr_ref_path != KDRV_IPP_BUILTIN_PATH_ID_1) {  //limit fastboot only support path 1 for 3dnr reference path to simplify control flow
	//	DBG_ERR("fastboot only support path1 as 3dnr reference path, %d\r\n", p_hdl->_3dnr_ref_path);
	//	return -1;
	//}

	KDRV_IPP_BUILTIN_DUMP("-------- mirror/flip ---------\n");
	kdrv_ipp_builtin_plat_read_dtsi_array(p_hdl->node, "mirror", (UINT32 *)&p_hdl->mirror, 1);

	KDRV_IPP_BUILTIN_DUMP("mirror:%d\n", p_hdl->mirror);

	if(kdrv_ipp_builtin_plat_read_dtsi_array(p_hdl->node, "flip", (UINT32 *)&p_hdl->flip, KDRV_IPP_BUILTIN_PATH_ID_MAX)){
			DBG_ERR("read flip error\n");
			return -1;
		}

	for(i=0; i < KDRV_IPP_BUILTIN_PATH_ID_5; i++){

		if(p_hdl->flip[i] == 1 && p_hdl->_3dnr_ref_path == i)
			p_hdl->_3dnr_ref_flip = 1;

		KDRV_IPP_BUILTIN_DUMP("flip[%d]:%d\n", i,p_hdl->flip[i]);
	}

	KDRV_IPP_BUILTIN_DUMP("ref_flip:%d\n", p_hdl->_3dnr_ref_flip);

	if (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW && p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_SHDR) {

		for(i = 0; i < (p_hdl->hdr_frm_num - 1); i++){
			p_hdl->vprc_shdr_blk[i].phy_addr = nvtmpp_sys_va2pa(p_hdl->vprc_shdr_blk[i].addr);
			if (p_hdl->vprc_shdr_blk[i].phy_addr == 0) {
				DBG_ERR("shdr ringbuf addr[%d] 0\r\n",i);
				return -1;
			}
		}
	}

	memset((void *)&p_hdl->pri_buf, 0, sizeof(KDRV_IPP_BUILTIN_PRI_BUF));

	#if 0  //530 LCA removed
	if (p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_YUV_SUBOUT) {
		if(kdrv_ipp_builtin_plat_read_dtsi_array_ex(p_hdl->root_node, "lca", &tmp_buf_size, 1)){
			DBG_ERR("read dtsi lca error\n");
			return -1;
		}
		for (i = 0; i < KDRV_IPP_BUILTIN_BUF_LCA_NUM; i++) {
			p_hdl->pri_buf.lca[i].addr = cur_buf_addr;
			p_hdl->pri_buf.lca[i].size = tmp_buf_size;
			p_hdl->pri_buf.lca[i].phy_addr = nvtmpp_sys_va2pa(cur_buf_addr);
			if (p_hdl->pri_buf.lca[i].phy_addr == 0) {
				DBG_ERR("va2pa failed 0x%.8lx\r\n", cur_buf_addr);
				return -1;
			}

			/* support single buffer for direct mode, add addr at last buffer idx */
			if (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) {
				if (i == (KDRV_IPP_BUILTIN_BUF_LCA_NUM - 1)) {
					cur_buf_addr += tmp_buf_size;
				}
			} else {
				cur_buf_addr += tmp_buf_size;
			}
		}
		KDRV_IPP_BUILTIN_DUMP("LCA: 0x%.8lx, 0x%.8lx, size 0x%.8x\r\n",
			p_hdl->pri_buf.lca[0].addr, p_hdl->pri_buf.lca[1].addr, p_hdl->pri_buf.lca[0].size);
	}
	#endif

	if (p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_3DNR) {
		if(kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "3dnr_mv", &tmp_buf_size, 1)){
			DBG_ERR("read dtsi 3dnr_mv error\n");
			return -1;
		}
		for (i = 0; i < KDRV_IPP_BUILTIN_BUF_3DNR_NUM; i++) {
			p_hdl->pri_buf._3dnr_mv[i].addr = cur_buf_addr;
			p_hdl->pri_buf._3dnr_mv[i].size = tmp_buf_size;
			p_hdl->pri_buf._3dnr_mv[i].phy_addr = nvtmpp_sys_va2pa(cur_buf_addr);
			if (p_hdl->pri_buf._3dnr_mv[i].phy_addr == 0) {
				DBG_ERR("va2pa failed 0x%.8lx\r\n", cur_buf_addr);
				return -1;
			}

			/* support single buffer for direct mode, add addr at last buffer idx */
			if (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) {
				if (i == (KDRV_IPP_BUILTIN_BUF_3DNR_NUM - 1)) {
					cur_buf_addr += tmp_buf_size;
				}
			} else {
				cur_buf_addr += tmp_buf_size;
			}
		}
		KDRV_IPP_BUILTIN_DUMP("3DNR MV: 0x%.8lx(0x%.8lx), 0x%.8lx(0x%.8lx), size 0x%.8x\r\n",
			p_hdl->pri_buf._3dnr_mv[0].addr, p_hdl->pri_buf._3dnr_mv[0].phy_addr, p_hdl->pri_buf._3dnr_mv[1].addr, p_hdl->pri_buf._3dnr_mv[1].phy_addr, p_hdl->pri_buf._3dnr_mv[0].size);

		if(kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "3dnr_ms", &tmp_buf_size, 1)){
			DBG_ERR("read dtsi 3dnr_ms error\n");
			return -1;
		}
		for (i = 0; i < KDRV_IPP_BUILTIN_BUF_3DNR_NUM; i++) {
			p_hdl->pri_buf._3dnr_ms[i].addr = cur_buf_addr;
			p_hdl->pri_buf._3dnr_ms[i].size = tmp_buf_size;
			p_hdl->pri_buf._3dnr_ms[i].phy_addr = nvtmpp_sys_va2pa(cur_buf_addr);
			if (p_hdl->pri_buf._3dnr_ms[i].phy_addr == 0) {
				DBG_ERR("va2pa failed 0x%.8lx\r\n", cur_buf_addr);
				return -1;
			}

			/* support single buffer for direct mode, add addr at last buffer idx */
			if (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) {
				if (i == (KDRV_IPP_BUILTIN_BUF_3DNR_NUM - 1)) {
					cur_buf_addr += tmp_buf_size;
				}
			} else {
				cur_buf_addr += tmp_buf_size;
			}
		}
		KDRV_IPP_BUILTIN_DUMP("3DNR MS: 0x%.8lx(0x%.8lx), 0x%.8lx(0x%.8lx), size 0x%.8x\r\n",
			p_hdl->pri_buf._3dnr_ms[0].addr, p_hdl->pri_buf._3dnr_ms[0].phy_addr, p_hdl->pri_buf._3dnr_ms[1].addr, p_hdl->pri_buf._3dnr_ms[1].phy_addr, p_hdl->pri_buf._3dnr_ms[0].size);

		if(kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "3dnr_ms_roi", &tmp_buf_size, 1)){
			DBG_ERR("read dtsi 3dnr_ms_roi error\n");
			return -1;
		}
		for (i = 0; i < KDRV_IPP_BUILTIN_BUF_3DNR_NUM; i++) {
			p_hdl->pri_buf._3dnr_ms_roi[i].addr = cur_buf_addr;
			p_hdl->pri_buf._3dnr_ms_roi[i].size = tmp_buf_size;
			p_hdl->pri_buf._3dnr_ms_roi[i].phy_addr = nvtmpp_sys_va2pa(cur_buf_addr);
			if (p_hdl->pri_buf._3dnr_ms_roi[i].phy_addr == 0) {
				DBG_ERR("va2pa failed 0x%.8lx\r\n", cur_buf_addr);
				return -1;
			}
			cur_buf_addr += tmp_buf_size;
		}
		KDRV_IPP_BUILTIN_DUMP("3DNR MS ROI: 0x%.8lx(0x%.8lx), 0x%.8lx(0x%.8lx), size 0x%.8x\r\n",
			p_hdl->pri_buf._3dnr_ms_roi[0].addr, p_hdl->pri_buf._3dnr_ms_roi[0].phy_addr, p_hdl->pri_buf._3dnr_ms_roi[1].addr, p_hdl->pri_buf._3dnr_ms_roi[1].phy_addr, p_hdl->pri_buf._3dnr_ms_roi[0].size);

		if (p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_3DNR_STA) {
			if(kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "3dnr_sta", &tmp_buf_size, 1)){
				DBG_ERR("read dtsi 3dnr_sta error\n");
				return -1;
			}
			for (i = 0; i < KDRV_IPP_BUILTIN_BUF_3DNR_STA_NUM; i++) {
				p_hdl->pri_buf._3dnr_sta[i].addr = cur_buf_addr;
				p_hdl->pri_buf._3dnr_sta[i].size = tmp_buf_size;
				p_hdl->pri_buf._3dnr_sta[i].phy_addr = nvtmpp_sys_va2pa(cur_buf_addr);
				if (p_hdl->pri_buf._3dnr_sta[i].phy_addr == 0) {
					DBG_ERR("va2pa failed 0x%.8lx\r\n", cur_buf_addr);
					return -1;
				}
				cur_buf_addr += tmp_buf_size;
			}
			KDRV_IPP_BUILTIN_DUMP("3DNR STA: 0x%.8lx(0x%.8lx), size 0x%.8x\r\n",
				p_hdl->pri_buf._3dnr_sta[0].addr, p_hdl->pri_buf._3dnr_sta[0].phy_addr, p_hdl->pri_buf._3dnr_sta[0].size);
		}

		if(kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "3dnr_fcvg", &tmp_buf_size, 1)){
			DBG_ERR("read dtsi 3dnr_fcvg error\n");
			return -1;
		}
		for (i = 0; i < KDRV_IPP_BUILTIN_BUF_3DNR_FCVR_NUM; i++) {
			p_hdl->pri_buf._3dnr_fcvg[i].addr = cur_buf_addr;
			p_hdl->pri_buf._3dnr_fcvg[i].size = tmp_buf_size;
			p_hdl->pri_buf._3dnr_fcvg[i].phy_addr = nvtmpp_sys_va2pa(cur_buf_addr);
			if (p_hdl->pri_buf._3dnr_fcvg[i].phy_addr == 0) {
				DBG_ERR("va2pa failed 0x%.8lx\r\n", cur_buf_addr);
				return -1;
			}

			/* support single buffer for direct mode, add addr at last buffer idx */
			if (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) {
				if (i == (KDRV_IPP_BUILTIN_BUF_3DNR_FCVR_NUM - 1)) {
					cur_buf_addr += tmp_buf_size;
				}
			} else {
				cur_buf_addr += tmp_buf_size;
			}
		}
		KDRV_IPP_BUILTIN_DUMP("3DNR FCVG: 0x%.8lx, 0x%.8lx, size 0x%.8x\r\n",
			p_hdl->pri_buf._3dnr_fcvg[0].addr, p_hdl->pri_buf._3dnr_fcvg[1].addr, p_hdl->pri_buf._3dnr_fcvg[0].size);

	}

	if(kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "defog", &tmp_buf_size, 1)){
		DBG_ERR("read dtsi defog error\n");
		return -1;
	}
	for (i = 0; i < KDRV_IPP_BUILTIN_BUF_DFG_NUM; i++) {
		p_hdl->pri_buf.defog[i].addr = cur_buf_addr;
		p_hdl->pri_buf.defog[i].size = tmp_buf_size;
		p_hdl->pri_buf.defog[i].phy_addr = nvtmpp_sys_va2pa(cur_buf_addr);
		if (p_hdl->pri_buf.defog[i].phy_addr == 0) {
			DBG_ERR("va2pa failed 0x%.8lx\r\n", cur_buf_addr);
			return -1;
		}
		cur_buf_addr += tmp_buf_size;
	}
	KDRV_IPP_BUILTIN_DUMP("DEFOG: 0x%.8lx, 0x%.8lx, size 0x%.8x\r\n",
		p_hdl->pri_buf.defog[0].addr, p_hdl->pri_buf.defog[1].addr, p_hdl->pri_buf.defog[0].size);

	if (p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_PM_PIXELIZTION) {
		if(kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "pm", &tmp_buf_size, 1)){
			DBG_ERR("read dtsi pm error\n");
			return -1;
		}
		for (i = 0; i < KDRV_IPP_BUILTIN_BUF_PM_NUM; i++) {
			p_hdl->pri_buf.pm_pixl[i].addr = cur_buf_addr;
			p_hdl->pri_buf.pm_pixl[i].size = tmp_buf_size;
			p_hdl->pri_buf.pm_pixl[i].phy_addr = nvtmpp_sys_va2pa(cur_buf_addr);
			if (p_hdl->pri_buf.pm_pixl[i].phy_addr == 0) {
				DBG_ERR("va2pa failed 0x%.8lx\r\n", cur_buf_addr);
				return -1;
			}
			cur_buf_addr += tmp_buf_size;
		}
		KDRV_IPP_BUILTIN_DUMP("PM: 0x%.8lx, 0x%.8lx, 0x%.8lx, size 0x%.8x\r\n",
			p_hdl->pri_buf.pm_pixl[0].addr, p_hdl->pri_buf.pm_pixl[1].addr, p_hdl->pri_buf.pm_pixl[2].addr, p_hdl->pri_buf.pm_pixl[0].size);

	}

	if (p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_WDR) {
		if(kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "wdr", &tmp_buf_size, 1)){
			DBG_ERR("read dtsi wdr error\n");
			return -1;
		}
		for (i = 0; i < KDRV_IPP_BUILTIN_BUF_WDR_NUM; i++) {
			p_hdl->pri_buf.wdr[i].addr = cur_buf_addr;
			p_hdl->pri_buf.wdr[i].size = tmp_buf_size;
			p_hdl->pri_buf.wdr[i].phy_addr = nvtmpp_sys_va2pa(cur_buf_addr);
			if (p_hdl->pri_buf.wdr[i].phy_addr == 0) {
				DBG_ERR("va2pa failed 0x%.8lx\r\n", cur_buf_addr);
				return -1;
			}
			cur_buf_addr += tmp_buf_size;
		}
		KDRV_IPP_BUILTIN_DUMP("WDR: 0x%.8lx, 0x%.8lx, size 0x%.8x\r\n",
			p_hdl->pri_buf.wdr[0].addr, p_hdl->pri_buf.wdr[1].addr, p_hdl->pri_buf.wdr[0].size);

	}

		if (p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_IFE_VA_SUBOUT) {
		if(kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "ife_va", &tmp_buf_size, 1)){
			DBG_ERR("read dtsi ife va error\n");
			return -1;
		}
		for (i = 0; i < KDRV_IPP_BUILTIN_BUF_IFE_VA_NUM; i++) {
			p_hdl->pri_buf.ife_va[i].addr = cur_buf_addr;
			p_hdl->pri_buf.ife_va[i].size = tmp_buf_size;
			p_hdl->pri_buf.ife_va[i].phy_addr = nvtmpp_sys_va2pa(cur_buf_addr);
			if (p_hdl->pri_buf.ife_va[i].phy_addr == 0) {
				DBG_ERR("va2pa failed 0x%.8lx\r\n", cur_buf_addr);
				return -1;
			}
			cur_buf_addr += tmp_buf_size;
		}
		KDRV_IPP_BUILTIN_DUMP("IFE_VA: 0x%.8lx, 0x%.8lx, size 0x%.8x\r\n",
			p_hdl->pri_buf.ife_va[0].addr, p_hdl->pri_buf.ife_va[1].addr, p_hdl->pri_buf.ife_va[0].size);
	}

	if (p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_IPE_VA_SUBOUT) {
		if(kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "va", &tmp_buf_size, 1)){
			DBG_ERR("read dtsi va error\n");
			return -1;
		}
		for (i = 0; i < KDRV_IPP_BUILTIN_BUF_VA_NUM; i++) {
			p_hdl->pri_buf.ipe_va[i].addr = cur_buf_addr;
			p_hdl->pri_buf.ipe_va[i].size = tmp_buf_size;
			p_hdl->pri_buf.ipe_va[i].phy_addr = nvtmpp_sys_va2pa(cur_buf_addr);
			if (p_hdl->pri_buf.ipe_va[i].phy_addr == 0) {
				DBG_ERR("va2pa failed 0x%.8lx\r\n", cur_buf_addr);
				return -1;
			}
			cur_buf_addr += tmp_buf_size;
		}
		KDRV_IPP_BUILTIN_DUMP("IPE_VA: 0x%.8lx, 0x%.8lx, size 0x%.8x\r\n",
			p_hdl->pri_buf.ipe_va[0].addr, p_hdl->pri_buf.ipe_va[1].addr, p_hdl->pri_buf.ipe_va[0].size);
	}

	if (p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_IME_VA_SUBOUT) {
		if(kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "ime_va", &tmp_buf_size, 1)){
			DBG_ERR("read dtsi ime va error\n");
			return -1;
		}
		for (i = 0; i < KDRV_IPP_BUILTIN_BUF_IME_VA_NUM; i++) {
			p_hdl->pri_buf.ime_va[i].addr = cur_buf_addr;
			p_hdl->pri_buf.ime_va[i].size = tmp_buf_size;
			p_hdl->pri_buf.ime_va[i].phy_addr = nvtmpp_sys_va2pa(cur_buf_addr);
			if (p_hdl->pri_buf.ime_va[i].phy_addr == 0) {
				DBG_ERR("va2pa failed 0x%.8lx\r\n", cur_buf_addr);
				return -1;
			}
			cur_buf_addr += tmp_buf_size;
		}
		KDRV_IPP_BUILTIN_DUMP("IME_VA: 0x%.8lx, 0x%.8lx, size 0x%.8x\r\n",
			p_hdl->pri_buf.ime_va[0].addr, p_hdl->pri_buf.ime_va[1].addr, p_hdl->pri_buf.ime_va[0].size);
	}

	if ((cur_buf_addr - p_hdl->vprc_ctrl.addr) > p_hdl->vprc_ctrl.size) {
		DBG_ERR("buffer size overflow\r\n");
		return -1;
	}

#if 1
	/* output buffer */
	kdrv_ipp_builtin_init_out_path_info(p_hdl);
	for (i = 0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++) {
		p_path = &p_hdl->path_info[i];

		if(p_hdl->queue_mode_en[i]){

			if(p_path->enable){

				if( p_hdl->queue_num[i] > 0){

					if (p_hdl->queue_ring_mode_en) {
						buf_num = p_hdl->queue_num[i];
					} else {
						buf_num = p_hdl->queue_num[i] + 1; // non-ring mode, set one extra buffer for clamping on max index
					}
					for (j = 0; j < buf_num; j++) {
						p_hdl->vprc_blk[i][j].size = kdrv_ipp_builtin_yuv_size(p_path);
						p_hdl->vprc_blk[i][j].addr = nvtmpp_get_fastboot_blk(p_hdl->vprc_blk[i][j].size);

						if (p_hdl->vprc_blk[i][j].addr && p_hdl->vprc_blk[i][j].size) {
							p_hdl->vprc_blk[i][j].phy_addr = nvtmpp_sys_va2pa(p_hdl->vprc_blk[i][j].addr);
							if (p_hdl->vprc_blk[i][j].phy_addr == 0) {
								DBG_ERR("get phy addr failed, va = 0x%.8x\r\n", (unsigned int)p_hdl->vprc_blk[i][j].addr);
								return -1;
							}else {
								//nvtmpp_lock_fastboot_blk(p_hdl->vprc_blk[i][j].addr);
								p_hdl->dbg_info.lock_cnt++;
								p_hdl->lock_cnt++;
							}
						}else if(p_hdl->vprc_blk[i][j].phy_addr == 0) {
								DBG_ERR("--------------get blk fail--------------\n");
								//DBG_ERR("get phy addr failed, va = 0x%.8x\r\n", (unsigned int)p_hdl->vprc_blk[i][j].addr);
								return -1;
						}
						KDRV_IPP_BUILTIN_DUMP("que_mode pid %d, blk %d, addr 0x%.8x, size 0x%.8x, pa 0x%.8x\r\n",
							i, j, (unsigned int)p_hdl->vprc_blk[i][j].addr, (unsigned int)p_hdl->vprc_blk[i][j].size, (unsigned int)p_hdl->vprc_blk[i][j].phy_addr);
					}
				}else {
					DBG_ERR("path[%d] enable but queue num is %d\n",i,p_hdl->queue_num[i]);
					return -1;
				}
			}

		}else {
			for (j = 0; j < p_hdl->ll_queue_num; j++) {
				if (p_path->enable) {

					if(p_hdl->stitch_en[i]){
						p_hdl->vprc_blk[i][j].size = p_hdl->stitch_buf_y_size[i] + p_hdl->stitch_buf_uv_size[i];
						p_hdl->vprc_blk[i][j].addr = g_kdrv_ipp_builtin_ctl.stitch_buf[i][j].addr;
					}else{
						p_hdl->vprc_blk[i][j].size = kdrv_ipp_builtin_yuv_size(p_path);
						p_hdl->vprc_blk[i][j].addr = nvtmpp_get_fastboot_blk(p_hdl->vprc_blk[i][j].size);
					}
					if (p_hdl->vprc_blk[i][j].addr && p_hdl->vprc_blk[i][j].size) {
						p_hdl->vprc_blk[i][j].phy_addr = nvtmpp_sys_va2pa(p_hdl->vprc_blk[i][j].addr);

						if (p_hdl->vprc_blk[i][j].phy_addr == 0) {
							DBG_ERR("get phy addr failed, va = 0x%.8x\r\n", (unsigned int)p_hdl->vprc_blk[i][j].addr);
							return -1;
						}else{
							p_hdl->dbg_info.lock_cnt++;
							p_hdl->lock_cnt++;
						}
					}else if (p_hdl->vprc_blk[i][j].addr == 0)
						DBG_ERR("--------------get blk fail--------------\n");
				}

				KDRV_IPP_BUILTIN_DUMP("pid %d, blk %d, addr 0x%.8x, size 0x%.8x, pa 0x%.8x\r\n",
					i, j, (unsigned int)p_hdl->vprc_blk[i][j].addr, (unsigned int)p_hdl->vprc_blk[i][j].size, (unsigned int)p_hdl->vprc_blk[i][j].phy_addr);
			}
		}
	}
#endif

	/* frame rate ctrl init */
	kdrv_ipp_builtin_init_frc(p_hdl);

	/* privact mask ctrl init */
	kdrv_ipp_builtin_init_privacy_mask(p_hdl);

	return 0;
}

INT32 kdrv_ipp_builtin_alloc_dtsi_buf(KDRV_IPP_BUILTIN_HDL* p_hdl)
{
	UINT32 buf_size = 0;

	buf_size = kdrv_ipp_builtin_get_reg_num(KDRV_IPP_BUILTIN_IFE) * 2 * 4; // 2 means reg_num + reg_tag, 4 means size 4 byte per register
	p_hdl->p_dtsi_ife_buf = kdrv_ipp_builtin_plat_malloc(buf_size);

	if (p_hdl->p_dtsi_ife_buf == NULL) {
		DBG_ERR("kmalloc dtsi ife buf(0x%.8x) failed\r\n", (unsigned int)buf_size);
		return -1;
	}

	KDRV_IPP_BUILTIN_DUMP("dtsi ife buffer 0x%.8lx, size 0x%.8x\r\n", (ULONG)p_hdl->p_dtsi_ife_buf, (unsigned int)buf_size);

	buf_size = kdrv_ipp_builtin_get_reg_num(KDRV_IPP_BUILTIN_IPE) * 2 * 4; // 2 means reg_num + reg_tag, 4 means size 4 byte per register
	p_hdl->p_dtsi_ipe_buf = kdrv_ipp_builtin_plat_malloc(buf_size);

	if (p_hdl->p_dtsi_ipe_buf == NULL) {
		DBG_ERR("kmalloc dtsi ipe buf(0x%.8x) failed\r\n", (unsigned int)buf_size);
		return -1;
	}

	KDRV_IPP_BUILTIN_DUMP("dtsi ipe buffer 0x%.8lx, size 0x%.8x\r\n", (ULONG)p_hdl->p_dtsi_ipe_buf, (unsigned int)buf_size);

	buf_size = kdrv_ipp_builtin_get_reg_num(KDRV_IPP_BUILTIN_IME) * 2 * 4; // 2 means reg_num + reg_tag, 4 means size 4 byte per register
	p_hdl->p_dtsi_ime_buf = kdrv_ipp_builtin_plat_malloc(buf_size);

	if (p_hdl->p_dtsi_ime_buf == NULL) {
		DBG_ERR("kmalloc dtsi ime buf(0x%.8x) failed\r\n", (unsigned int)buf_size);
		return -1;
	}

	KDRV_IPP_BUILTIN_DUMP("dtsi ime buffer 0x%.8lx, size 0x%.8x\r\n", (ULONG)p_hdl->p_dtsi_ime_buf, (unsigned int)buf_size);

	return E_OK;
}
INT32 kdrv_ipp_builtin_init(KDRV_IPP_BUILTIN_INIT_INFO *info)
{
	KDRV_IPP_BUILTIN_BLK reg_base_info[KDRV_IPP_BUILTIN_ENG_MAX] = {
		{0x2F0340000, 0xD88, 0x2F0340000},	/* IFE */
		{0x2F0400000, 0x159C,0x2F0400000},	/* IPE */
		{0x2F0410000, 0xC8C, 0x2F0410000},	/* IME */
	};
	KDRV_IPP_BUILTIN_CTL* p_ctl = NULL;
	KDRV_IPP_BUILTIN_HDL* p_hdl = NULL;

	UINT32 dev_id = 0;
	UINT32 i=0, k=0, j=0;
	UINT32 intr_ctrl = 0;

	/* efuse check*/
	if(NVT_API_CHK_CALL(kdrv_ipp_builtin_chk) != TRUE){
		DBG_ERR("objver mismatch %s\r\n", "kdrv_builtin_ipp");
		return -1;
	}

	if (kdrv_builtin_is_fastboot() == FALSE) {
		return E_NOSPT;
	}

	if (info == NULL) {
		DBG_ERR("init info null\r\n");
		return -1;
	}

	nvt_bootts_add_ts("IME"); //begin

	if(info->rtos_ipp_bit == 0)
		DBG_DUMP("\r\n\r\n-------------------------- ipp builtin init 1.0------------------------------- \r\n");
	else
		DBG_DUMP("\r\n\r\n-------------------------- ipp builtin init 2.0------------------------------- \r\n");

	/* init builtin flow */
	p_ctl = &g_kdrv_ipp_builtin_ctl;
	memset((void *)p_ctl, 0, sizeof(KDRV_IPP_BUILTIN_CTL));
	p_ctl->valid_src_id_bit = info->valid_src_id_bit;
	KDRV_IPP_BUILTIN_DUMP("valid src id bit 0x%.8x\r\n", (unsigned int)p_ctl->valid_src_id_bit);


	/* init engine io addr */
	for (i = 0; i < KDRV_IPP_BUILTIN_ENG_MAX; i++) {
		p_ctl->reg_base[i] = kdrv_ipp_builtin_plat_ioremap_nocache(reg_base_info[i].addr, reg_base_info[i].size);
		if (p_ctl->reg_base[i] == NULL) {
			DBG_ERR("ioremap eng %d failed\r\n", i);
			return -1;
		}
	}

	/* handle init */
	if(kdrv_ipp_builtin_plat_read_dtsi_array("/fastboot/ipp/ctl", "num", (UINT32 *)&p_ctl->hdl_num, 1)){
		DBG_ERR("read dtsi num error\n");
		return -1;
	}


	/* backward compatible flow select */
	if (p_ctl->hdl_num == 0) {
		p_ctl->hdl_num = 1;
		p_ctl->dtsi_ver = 0;
	} else {
		p_ctl->dtsi_ver = 1;
	}
	p_ctl->p_hdl = kdrv_ipp_builtin_plat_malloc(sizeof(KDRV_IPP_BUILTIN_HDL) * p_ctl->hdl_num);

	if(p_ctl->p_hdl == NULL){
		DBG_ERR("alloc hdl failed\n");
		return -1;
	}

	memset((void *)p_ctl->p_hdl , 0, sizeof(KDRV_IPP_BUILTIN_HDL) * p_ctl->hdl_num);

	KDRV_IPP_BUILTIN_DUMP("dtsi version %d, hdl_buffer 0x%.8lx, num %d, size 0x%.8lx\r\n",
		(UINT32)p_ctl->dtsi_ver, (ULONG)p_ctl->p_hdl, (UINT32)p_ctl->hdl_num, (ULONG)(sizeof(KDRV_IPP_BUILTIN_HDL) * p_ctl->hdl_num));

	/* init default handle by dtsi */
	for (k = 0; k < p_ctl->hdl_num; k++) {

		//coverity[overwrite_var]
		p_hdl = &p_ctl->p_hdl[k];
		p_hdl->dbg_info.pri_buf_info = &p_hdl->pri_buf;
		p_hdl->dbg_info.last_buf_info = &p_hdl->last_out_info;
		p_hdl->dbg_info.last_isp_info = &p_hdl->isp_info;
		p_hdl->dbg_info.output_info = p_hdl->path_info;
		p_hdl->dbg_info.flow = &p_hdl->flow;
		p_hdl->dbg_info.ch_cnt = p_ctl->hdl_num;
		p_hdl->sensor_fps = info->sensor_fps;
		p_hdl->frm_interval_us = (1000000 / p_hdl->sensor_fps)*100;

		p_hdl->frm_done = 0;

		if(info->rtos_ipp_bit != 0)
			p_hdl->rtos_ipp_en = 1;
		else
			p_hdl->rtos_ipp_en = 0;

		if (kdrv_ipp_builtin_dtsi_ver() == 0) {
			snprintf(p_hdl->node, 63, "/fastboot/ipp-config");
			kdrv_ipp_builtin_get_root_node_info(p_hdl->node, &p_hdl->root_node);

			p_hdl->name ="builtin_vprc1";
			p_hdl->flow = KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW;
			p_hdl->isp_id = 0;


		} else if (kdrv_ipp_builtin_dtsi_ver() == 1) {

			snprintf(p_hdl->node, 63, "/fastboot/ipp/config-%d", k);

			kdrv_ipp_builtin_get_root_node_info(p_hdl->node, &p_hdl->root_node);

			if(kdrv_ipp_builtin_set_child_nodeofsset(&p_hdl->root_node, "stitch_mode")){
				DBG_ERR("set stitch mode child nodeoffset failed\n");
				return -1;
			}

			if(kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "stitch_en", (UINT32 *)&p_hdl->stitch_en[0] , KDRV_IPP_BUILTIN_PATH_ID_MAX)){
				DBG_ERR("read stitch_en error\n");
				return -1;
			}

			if(kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "stitch_buf_y_size", (UINT32 *)&p_hdl->stitch_buf_y_size[0] , KDRV_IPP_BUILTIN_PATH_ID_MAX)){
				DBG_ERR("read stitch_buf_y_size error\n");
				return -1;
			}

			if(kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "stitch_buf_uv_size", (UINT32 *)&p_hdl->stitch_buf_uv_size[0] , KDRV_IPP_BUILTIN_PATH_ID_MAX)){
				DBG_ERR("read stitch_buf_uv_size error\n");
				return -1;
			}

			if(kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "stitch_addr_y_ofs", (UINT32 *)&p_hdl->stitch_addr_y_ofs[0] , KDRV_IPP_BUILTIN_PATH_ID_MAX)){
				DBG_ERR("read stitch_addr_y_ofs error\n");
				return -1;
			}

			if(kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "stitch_addr_uv_ofs", (UINT32 *)&p_hdl->stitch_addr_uv_ofs[0], KDRV_IPP_BUILTIN_PATH_ID_MAX)){
				DBG_ERR("read stitch_addr_uv_ofs error\n");
				return -1;
			}

			if(kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "stitch_img_width", (UINT32 *)&p_hdl->stitch_img_width[0] , KDRV_IPP_BUILTIN_PATH_ID_MAX)){
				DBG_ERR("read stitch_img_width error\n");
				return -1;
			}

			if(kdrv_ipp_builtin_plat_read_dtsi_array_ex(&p_hdl->root_node, "stitch_img_height", (UINT32 *)&p_hdl->stitch_img_height[0], KDRV_IPP_BUILTIN_PATH_ID_MAX)){
				DBG_ERR("read stitch_img_height error\n");
				return -1;
			}


			for(i=0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++){

				if( p_hdl->stitch_en[i]){
					KDRV_IPP_BUILTIN_DUMP("-----------------stitch_path_%d hdl_%d----------------\n", i, k);
					KDRV_IPP_BUILTIN_DUMP("stitch_buf_y_size =%d, stitch_buf_uv_size=%d\n", p_hdl->stitch_buf_y_size[i], p_hdl->stitch_buf_uv_size[i]);
					KDRV_IPP_BUILTIN_DUMP("stitch_addr_y_ofs =%d, stitch_addr_uv_ofs=%d\n", p_hdl->stitch_addr_y_ofs[i], p_hdl->stitch_addr_uv_ofs[i]);
					KDRV_IPP_BUILTIN_DUMP("stitch_img_wdith  =%d, stitch_img_height =%d\n", p_hdl->stitch_img_width[i], p_hdl->stitch_img_height[i]);
				}
			}

			//if(kdrv_ipp_builtin_plat_read_dtsi_string(p_hdl->node, "hdl_name", &p_hdl->name[0])){
			//	DBG_ERR("read dtsi hdl_name error\n");
			//	return -1;
			//}
			if(kdrv_ipp_builtin_plat_read_dtsi_array(p_hdl->node, "flow", (UINT32 *)&p_hdl->flow, 1)){
				DBG_ERR("read dtsi flow error\n");
				return -1;
			}
			if(kdrv_ipp_builtin_plat_read_dtsi_array(p_hdl->node, "isp_id", (UINT32 *)&p_hdl->isp_id, 1)){
				DBG_ERR("read dtsi isp_id error\n");
				return -1;
			}

			if(kdrv_ipp_builtin_plat_read_dtsi_array(p_hdl->node, "shdr_frm_num", (UINT32 *)&p_hdl->hdr_frm_num, 1)){
				DBG_ERR("read dtsi shdr_frm_num error\n");
			}else
				p_hdl->dbg_info.shdr_frm_num = p_hdl->hdr_frm_num;

			if(kdrv_ipp_builtin_plat_read_dtsi_array(p_hdl->node, "dbg_func_en", (UINT32 *)&p_hdl->dbg_func_en, 1)){

			}

			KDRV_IPP_BUILTIN_DUMP("------------------- dbg_fun: 0x%x ---------------\n",p_hdl->dbg_func_en);
#if 1
			/* yuv queue func */
			if(kdrv_builtin_is_fastboot_yuvque_en()/*p_hdl->queue_mode_en*/){

				kdrv_ipp_builtin_plat_read_dtsi_array(p_hdl->node, "queue_num", (UINT32 *)p_hdl->queue_num, KDRV_IPP_BUILTIN_PATH_ID_MAX);

				for (i = 0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++){

					if(p_hdl->queue_num[i] > KDRV_IPP_BUILTIN_BUF_NUM_MAX){
						DBG_ERR("request queue[%d] num is over maximum:%d\n", i,KDRV_IPP_BUILTIN_BUF_NUM_MAX);
						return -1;
					}

					if(p_hdl->queue_num[i] != 0){
						p_hdl->queue_mode_en[i] = 1;
						if(p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW)
							DBG_ERR("----enable queue mode in direct mode-----\n");
					}
					else
						p_hdl->queue_mode_en[i] = 0;

					DBG_DUMP("ch[%d]queue_en[%d] = %d\n", k, i, p_hdl->queue_mode_en[i]);
				}

				kdrv_ipp_builtin_plat_read_dtsi_array(p_hdl->node, "queue_ring_mode_en", (UINT32 *)&p_hdl->queue_ring_mode_en, 1);
				kdrv_ipp_builtin_plat_read_dtsi_array(p_hdl->node, "queue_h264_mode_en", (UINT32 *)&p_hdl->queue_h264_mode_en, 1);
				KDRV_IPP_BUILTIN_DUMP("queue_ring_mode_en = %d, queue_h264_mode_en = %d\n", p_hdl->queue_ring_mode_en, p_hdl->queue_h264_mode_en);

			}else{
				DBG_DUMP("-yuv_queue_mode off-\n");
			}

#endif
			if(kdrv_ipp_builtin_plat_read_dtsi_array(p_hdl->node, "preroll_en", (UINT32 *)&p_hdl->pre_roll_flg, 1)){
				DBG_ERR("read dtsi preroll_en error\n");
				return -1;
			}

			KDRV_IPP_BUILTIN_DUMP("------------------- preroll_en: 0x%x ---------------\n", p_hdl->pre_roll_flg);

			if(p_hdl->pre_roll_flg == 1)
				break;

			if(kdrv_ipp_builtin_plat_read_dtsi_array(p_hdl->node, "chksum", (UINT32 *)&p_hdl->dbg_info.dtsi_chksum, 1)){
				DBG_ERR("read dtsi chksum error\n");
				return -1;
			}

			if(p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW){ //fb2.0
				if(kdrv_ipp_builtin_plat_read_dtsi_array(p_hdl->node, "ll_queue", (UINT32 *)&p_hdl->ll_queue_num, 1)){
					DBG_ERR("read ll_queue error\n");
					return -1;
				}
			}else{
				p_hdl->ll_queue_num = 2;
			}

			p_hdl->output_queue_num = p_hdl->rtos_queue_num; //fb2.0
			kdrv_ipp_builtin_cal_ll_tag_byte(p_hdl);

		} else {
			DBG_ERR("Unknown dtsi version %d\r\n", kdrv_ipp_builtin_dtsi_ver());
			return -1;
		}

		kdrv_ipp_builtin_alloc_dtsi_buf(p_hdl);
		kdrv_ipp_builtin_parsing_dtsi(p_hdl);
	}

	if(p_ctl->p_hdl[0].stitch_en[0]){
		KDRV_IPP_BUILTIN_DUMP("------------------- stitch_en ---------------\n");
		kdrv_ipp_builtin_alloc_stitch_buf(&p_ctl->p_hdl[0]);
	}

	/* init handle ctrl blk by init_info */
	for (k = 0; k < info->hdl_num; k++) {

		/* find matched handle from pool */
		for (i = 0; i < p_ctl->hdl_num; i++) {
			if (info->hdl_info[k].isp_id == p_ctl->p_hdl[i].isp_id) {
				DBG_DUMP("###(%d,%d) k=%d, i=%d\n",info->hdl_info[k].isp_id,p_ctl->p_hdl[i].isp_id, k, i);
				break;
			}
		}

		if (i < p_ctl->hdl_num) {

			p_hdl = &p_ctl->p_hdl[i];
			p_hdl->src_sie_id_bit = info->hdl_info[k].src_sie_id_bit;
			p_hdl->vprc_ctrl.addr = info->hdl_info[k].ctrl_blk_addr;
			p_hdl->vprc_ctrl.size = info->hdl_info[k].ctrl_blk_size;
			p_hdl->vprc_ctrl.phy_addr = nvtmpp_sys_va2pa(p_hdl->vprc_ctrl.addr);

			if(p_hdl->pre_roll_flg == 1)
				break;

			if (p_hdl->vprc_ctrl.phy_addr == 0) {
				DBG_ERR("isp_id: %d, blk_addr 0x%.8x, get phy_blk_addr 0\r\n", p_hdl->isp_id, (unsigned int)p_hdl->vprc_ctrl.addr);
				return -1;
			}

			for(j = 0; j < (p_hdl->hdr_frm_num - 1); j++){
				p_hdl->vprc_shdr_blk[j].addr = info->hdl_info[k].shdr_ring_buf_addr[j];
				p_hdl->vprc_shdr_blk[j].size = info->hdl_info[k].shdr_ring_buf_size[j];
			}

			KDRV_IPP_BUILTIN_DUMP("ctrl: 0x%.8lx, 0x%.8x;\r\n", p_hdl->vprc_ctrl.addr, p_hdl->vprc_ctrl.size);

			if(kdrv_ipp_builtin_init_handle(p_hdl)){
				DBG_ERR("--------------------- init handle failed!!!! --------------------\n");
				return -1;
			}

		} else {
			DBG_WRN("find no handle isp_id:%d\r\n", info->hdl_info[k].isp_id);
		}
	}
#if 1
	for (k = 0; k < p_ctl->hdl_num; k++) {
		if (kdrv_ipp_builtin_dtsi_ver() == 1) {
			snprintf(p_hdl->node, 63, "/fastboot/ipp/config-%d", (int)k);

			//fb2.0
			/* yuv queue func */
			if(kdrv_builtin_is_fastboot_yuvque_en() && info->rtos_ipp_bit){
				for (i = 0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++){
					if(p_hdl->queue_mode_en[i]){
						//yuv frc
						if(p_hdl->frc[i].skip >= info->ipp_rtos_frm_idx){
								p_hdl->yuv_str_idx[i] = 0;

						} else if(p_hdl->frc[i].src != p_hdl->frc[i].dst && p_hdl->frc[i].dst != 0){

							p_hdl->yuv_str_idx[i] = 1;
							p_hdl->yuv_frc_drop[i] = (KDRV_IPP_BUILTIN_FRC_BASE * p_hdl->frc[i].src) / (p_hdl->frc[i].src - p_hdl->frc[i].dst);

								for(j = 1; j < info->ipp_rtos_frm_idx - p_hdl->frc[i].skip; j++){
									p_hdl->yuv_str_cnt[i] += KDRV_IPP_BUILTIN_FRC_BASE;
									if(p_hdl->yuv_str_cnt[i] < p_hdl->yuv_frc_drop[i]){
										p_hdl->yuv_str_idx[i]++;
									} else {
										p_hdl->yuv_str_cnt[i] -= p_hdl->yuv_frc_drop[i];
									}
									KDRV_IPP_BUILTIN_DUMP("-yuv_str_idx[%d] %d, yuv_str_cnt[%d] %d-\n", i, p_hdl->yuv_str_idx[i], i, p_hdl->yuv_str_cnt[i]);
								}

							KDRV_IPP_BUILTIN_DUMP("-yuv_init_idx[%d] %d yuv_frc_drop[%d] %d-\n", i, p_hdl->yuv_str_idx[i], i, p_hdl->yuv_frc_drop[i]);

						} else if(p_hdl->frc[i].src == p_hdl->frc[i].dst){
							p_hdl->yuv_str_idx[i] = info->ipp_rtos_frm_idx;
						}

						if (p_hdl->queue_ring_mode_en) {
							p_hdl->queue_out_idx[i] = p_hdl->yuv_str_idx[i] % p_hdl->queue_num[i];
							p_hdl->queue_in_idx[i] = p_hdl->queue_out_idx[i];
						} else {
							p_hdl->queue_out_idx[i] = min(p_hdl->yuv_str_idx[i], p_hdl->queue_num[i]); // clamp to extra buffer
							p_hdl->queue_in_idx[i] = p_hdl->queue_out_idx[i];
						}

						if(p_hdl->queue_ring_mode_en && p_hdl->yuv_str_idx[i] >= p_hdl->queue_num[i])
							p_hdl->ring_flag[i] = 1;

						if(p_hdl->ring_flag[i] == 1) {
							p_hdl->buf_num_in_que[i] = p_hdl->queue_num[i];

						} else if(p_hdl->buf_num_in_que[i] < p_hdl->queue_num[i]){
								p_hdl->buf_num_in_que[i] = p_hdl->queue_in_idx[i] + 1;
						}

						p_hdl->fs_cnt_yuv_que = info->ipp_rtos_frm_idx;

						KDRV_IPP_BUILTIN_DUMP("-queue_in_idx[%d] %d, ring_flag %d-\n", i, p_hdl->queue_in_idx[i], p_hdl->ring_flag[i]);
					}
				}

			}
		}
	}
#endif

	if(p_ctl->p_hdl[0].stitch_en[0])
		kdrv_ipp_builtin_find_frm_sync_master();

	/* set ime builtin cb, config ipp register */
	dev_id = KDRV_DEV_ID(KDRV_CHIP0, KDRV_VIDEOPROCS_IME_ENGINE0, 0);
	ime_builtin_reg_fastboot_cb(dev_id, kdrv_ipp_builtin_ime_isr);
	p_hdl = &p_ctl->p_hdl[0];


	if(info->rtos_ipp_bit == 0){ // rtos switch to builtin patch  //fb2.0
		/* config engine clock */
		if (kdrv_ipp_builtin_plat_init_clk() != 0) {
			return -1;
		}

		/* sram ctrl */
		nvt_disable_sram_shutdown(IFE_SD);
		/*nvt_disable_sram_shutdown(DCE_SD);*/ //567 removed
		nvt_disable_sram_shutdown(IPE_SD);
		nvt_disable_sram_shutdown(IME_SD);
		//nvt_disable_sram_shutdown(IFE2_SD); //530 removed
		kdrv_ipp_builtin_ife_shared_sram_clksel(); //Set CG (clock gen,0xF0020000, 0x18) IFE_SHARED_SRAM_CLKSLE to ife from cpu
		//kdrv_ipp_builtin_ipe_shared_sram_clksel(); //538 removed
		//kdrv_ipp_builtin_ime_shared_sram_clksel(); //538 removed

		#if 0 // 567 removed
		if (p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_GDC){
			kdrv_ipp_builtin_cnn_dce_shared_sram_clksel(); // Set CG PLL_CLKSEL_CNN_SRAM_DCE to dce from cnn when GDC enabled
			nvt_disable_sram_shutdown(CNN_SD);

			if(kdrv_builtin_ai_is_fastboot())
				DBG_ERR("---GDC & CNN both enable---\n");
		}
		#endif
	}else{

		if (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW){

			p_ctl->p_trig_hdl = p_hdl; //fb2.0
			p_hdl->fs_cnt = p_hdl->ll_queue_num; //fb2.0

		}else if(p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_RAW){

			for (k = 0; k < info->hdl_num; k++) {

                p_hdl = &p_ctl->p_hdl[k];

                p_hdl->fs_cnt = info->ipp_rtos_frm_idx + 1;
                p_hdl->rtos_frm_cnt = info->ipp_rtos_frm_idx;

                KDRV_IPP_BUILTIN_DUMP("-d2d rtos switch to builtin with frm cnt:%d-\n", info->ipp_rtos_frm_idx);
			}

		}else{

			KDRV_IPP_BUILTIN_DUMP("-flow is not dir or d2d mode detected-\n");
		}

		//ime_builtin_request_irq(); //fb2.0

		intr_ctrl = kdrv_ipp_builtin_get_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_INTERRUPT_EN); //turn on linked list interrupt //fb2.0
		intr_ctrl |= REG_VAL_IME_INTE_LL_JOB_END | REG_VAL_IME_INTE_LL_END | IME_BUILTIN_INTS_LL_ERR;
		intr_ctrl |= REG_VAL_IME_INTE_FRM_END;

		kdrv_ipp_builtin_set_reg(KDRV_IPP_BUILTIN_IME, REG_OFS_IME_INTERRUPT_EN, intr_ctrl);

		/* soft reset engine */
		//kdrv_ipp_builtin_soft_reset_eng();  // rtos switch to builtin patch  //fb2.0
	}


	KDRV_IPP_BUILTIN_DUMP("\r\n\r\n-------------------------- flow=%d ------------------------------- \r\n",p_hdl->flow );

	p_hdl = &p_ctl->p_hdl[0];

	if (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) { //fb2.0

		if(info->rtos_ipp_bit == 0){

			/* trigger engine start if direct mode */
			if(kdrv_ipp_builtin_cfg_eng_all(p_hdl)){
				DBG_ERR("kdrv_ipp_builtin_cfg_eng_all fail\n");
				return E_SYS;
			}

			kdrv_ipp_builtin_trig_eng_start(p_hdl);
		}else{ 		// rtos switch to builtin patch //fb2.0

			DBG_DUMP("---rtos switch builtin---\n");

			THREAD_CREATE(p_ctl->ll_switch_tsk_id, kdrv_ipp_builtin_ll_switch_tsk, 0, "ll_switch_tsk");
			if (p_ctl->ll_switch_tsk_id == 0) {
				DBG_ERR("open ll_switch task failed\r\n");
				return E_SYS;
			}
			THREAD_SET_PRIORITY(p_ctl->ll_switch_tsk_id, KDRV_IPP_BUILTIN_TSK_PRIORITY);
			THREAD_RESUME(p_ctl->ll_switch_tsk_id);
		}

	} else {
		/* init resource for d2d flow */
		p_ctl->job_pool.blk_size = sizeof(KDRV_IPP_BUILTIN_JOB);
		p_ctl->job_pool.blk_num = KDRV_IPP_BUILTIN_QUE_DEPTH;
		p_ctl->job_pool.total_size = p_ctl->job_pool.blk_num * p_ctl->job_pool.blk_size;
		p_ctl->job_pool.start_addr = (ULONG)kdrv_ipp_builtin_plat_malloc(p_ctl->job_pool.total_size);
		if (p_ctl->job_pool.start_addr == 0) {
			DBG_ERR("alloc job pool failed, want_size 0x%.8x\r\n", (unsigned int)p_ctl->job_pool.total_size);
			return E_NOMEM;
		}
		KDRV_IPP_BUILTIN_DUMP("job buffer 0x%.8lx, num %d, size 0x%.8x\r\n",
			p_ctl->job_pool.start_addr, KDRV_IPP_BUILTIN_QUE_DEPTH, p_ctl->job_pool.total_size);
		vk_spin_lock_init(&p_ctl->job_pool.lock);
		VOS_INIT_LIST_HEAD(&p_ctl->job_pool.free_list_root);
		VOS_INIT_LIST_HEAD(&p_ctl->job_pool.used_list_root);
		for (i = 0; i < p_ctl->job_pool.blk_num; i++) {
			KDRV_IPP_BUILTIN_JOB *p_job;

			p_job = (KDRV_IPP_BUILTIN_JOB *)(p_ctl->job_pool.start_addr + (p_ctl->job_pool.blk_size * i));
			VOS_INIT_LIST_HEAD(&p_job->pool_list);
			VOS_INIT_LIST_HEAD(&p_job->proc_list);
			vos_list_add_tail(&p_job->pool_list, &p_ctl->job_pool.free_list_root);
		}

		VOS_INIT_LIST_HEAD(&p_ctl->job_list_root);
		vk_spin_lock_init(&p_ctl->job_list_lock);
		vk_spin_lock_init(&p_ctl->sie_cb_lock);
		vos_flag_create(&p_ctl->proc_tsk_flg_id, NULL, "builtin_ipp_flg");  //tmp remove for build

		THREAD_CREATE(p_ctl->proc_tsk_id, kdrv_ipp_builtin_tsk, 0, "builtin_ipp_tsk");
		if (p_ctl->proc_tsk_id == 0) {
			DBG_ERR("open builtin ipp task failed\r\n");
			return E_SYS;
		}
		THREAD_SET_PRIORITY(p_ctl->proc_tsk_id, KDRV_IPP_BUILTIN_TSK_PRIORITY);
		THREAD_RESUME(p_ctl->proc_tsk_id);

		/* register sie callback */
		sie_fb_reg_buf_out_cb(kdrv_ipp_builtin_sie_cb);
	}

#if 0
	/* isp reset event */
	if (g_kdrv_ipp_isp_cb) {
		g_kdrv_ipp_isp_cb(0, KDRV_IPP_BUILTIN_ISP_EVENT_RESET);
	}
#endif
	p_ctl->is_init = TRUE;
	p_ctl->is_fastboot = TRUE;

	return E_OK;
}

INT32 kdrv_ipp_builtin_exit(void)
{
	FLGPTN wait_flg;
	KDRV_IPP_BUILTIN_CTL* p_ctl;
	KDRV_IPP_BUILTIN_HDL* p_hdl;
	KDRV_IPP_BUILTIN_FMD_CB_INFO info = {0};
	KDRV_IPP_BUILTIN_DBG_DRAW_INFO draw_buf_info = {0};
	UINT32 i=0;
	UINT32 j=0,k=0;
	UINT32 dev_id=0;

	if (kdrv_builtin_is_fastboot() == FALSE) {
		return E_NOSPT;
	}

	p_ctl = &g_kdrv_ipp_builtin_ctl;
	if (!p_ctl->is_init) {
		return E_OK;
	}
	p_hdl = p_ctl->p_trig_hdl;

	if (p_hdl && p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) {
		/* direct mode */
		/* frame end callback flow, push last output buffer */
		/* add frame start cnt, last fst is switch to normal isr cb */


		if(p_hdl->rtos_ipp_en){

			if(p_hdl->job_end_cnt < 2){
				DBG_ERR("rtos switch to builtin fail\r\n");
				return E_SYS;
			}
		}


		if (!p_hdl->out_buf_release_cnt) {
			DBG_ERR("direct mode exit but switch_to_normal = FALSE\r\n");
			return E_SYS;
		}

		p_hdl->fs_cnt++;
		p_hdl->fed_cnt++;
		p_hdl->dbg_info.output_cnt++;

		info.name = p_hdl->name;
		info.isp_id = p_hdl->isp_id;
		info.release_flg = (p_hdl->out_buf_release_cnt > 0) ? 1 : 0;
		for (i = 0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++) {
			info.out_img[i] = kdrv_ipp_builtin_get_path_info(info.isp_id, i);
			/* lock 3dnr ref path buffer */
			if ((p_hdl->func_en & KDRV_IPP_BUILTIN_FUNC_3DNR) &&
				(p_hdl->_3dnr_ref_path == i) &&
				(info.out_img[i].enable) &&
				(info.out_img[i].addr[0])) {
				KDRV_IPP_BUILTIN_DUMP("FASTBOOT LOCK 3DNR REF 0x%.8lx\r\n", info.out_img[i].addr[0]);
				nvtmpp_lock_fastboot_blk(info.out_img[i].addr[0]);
				p_ctl->_3dnr_lock_addr = info.out_img[i].addr[0];
				p_hdl->_3dnr_lock_addr = info.out_img[i].addr[0];
				p_ctl->p_hdl[0].dbg_info._3dnr_lock_cnt++;
				p_hdl->lock_cnt++;
			}


			/* Do frame rate control */
			kdrv_ipp_builtin_frc(p_hdl, &info, i);

			// Do yuv queue mode setting
			if(info.out_img[i].enable){

				if( (p_hdl->queue_num[i]!= 0)){

					if (p_hdl->queue_ring_mode_en) {
						p_hdl->queue_out_idx[i] = (p_hdl->queue_out_idx[i] + 1) % p_hdl->queue_num[i];
					} else {
						p_hdl->queue_out_idx[i] = min(p_hdl->queue_out_idx[i] + 1, p_hdl->queue_num[i]); // clamp to extra buffer
					}
					if( (p_hdl->ring_flag[i]) == 0 && (p_hdl->queue_out_idx[i] == 0))
						p_hdl->ring_flag[i] = 1;
				}
			}else{
				if( (p_hdl->queue_num[i]!= 0)){
					if(p_hdl->out_buf_release_cnt > 0){
						//yuv queue mode, if queue already ringed and builtin last frame need to drop, update queue index for indicate to correct queue head
						if (p_hdl->queue_ring_mode_en) {
							p_hdl->queue_out_idx[i] = (p_hdl->queue_out_idx[i] + 1) % p_hdl->queue_num[i];
						} else {
							p_hdl->queue_out_idx[i] = min(p_hdl->queue_out_idx[i] + 1, p_hdl->queue_num[i]); // clamp to extra buffer
						}
						if( (p_hdl->ring_flag[i]) == 0 && (p_hdl->queue_out_idx[i] == 0))
							p_hdl->ring_flag[i] = 1;
					}
				}
			}

			/* Paste debug stamp */
			draw_buf_info.p_hdl = p_hdl;
			draw_buf_info.draw_buf_info.w = info.out_img[i].size.w;
			draw_buf_info.draw_buf_info.h=  info.out_img[i].size.h;
			draw_buf_info.draw_buf_info.lofs = info.out_img[i].loff[0];
			draw_buf_info.draw_buf_info.ptr = (void *)info.out_img[i].addr[0];
			draw_buf_info.frm_skip_flag = ( ~(info.out_img[i].enable) ) & 0x01;

			if(info.out_img[i].addr[0]){
				kdrv_ipp_builtin_dbg(&g_kdrv_ipp_builtin_ctl, DBG_STAMP_PASTE, &draw_buf_info);
				vos_cpu_dcache_sync((ULONG)draw_buf_info.draw_buf_info.ptr, (draw_buf_info.draw_buf_info.w = info.out_img[i].size.w << 8), VOS_DMA_TO_DEVICE);
			}
		}


		/* Check register setting is applied on this frame successfully or not */
		if( !kdrv_ipp_check_ime_dram_status(p_hdl) ){
			p_hdl->dbg_info.apply_fail_cnt++;
		}

		if(p_hdl->dbg_func_en & KDRV_IPP_PARAM_BUILTIN_SET_PTS_MSG_EN){
			DBG_DUMP("-[%d,%d,%d,%d]-\n", info.out_img[0].enable,
										info.out_img[1].enable,
										info.out_img[2].enable,
										info.out_img[3].enable);
		}

		if(kdrv_builtin_is_fastboot_yuvque_en() == 0 ||  (  (kdrv_builtin_is_fastboot_yuvque_en() == 1) && p_hdl->queue_h264_mode_en)) {
			for (i = 0; i < KDRV_IPP_BUILTIN_FMD_CB_NUM; i++) {
				if (p_ctl->fmd_cb[i] || p_hdl->fmd_cb[i]) {
					if (p_ctl->fmd_cb[i])
						p_ctl->fmd_cb[i](&info, 0);

					if (p_hdl->fmd_cb[i])
						p_hdl->fmd_cb[i](&info, 0);
				}
			}
		}

		/* release blk when release_flg == 1 */
		if (info.release_flg) {
			for (i = 0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++) {
				if (info.out_img[i].addr[0]) {
					if(p_hdl->queue_mode_en[i] == 0) {
						if(nvtmpp_unlock_fastboot_blk(info.out_img[i].addr[0]) != 0)
							DBG_ERR("nvtmpp_unlock_fastboot_blk fail addr:%lx\n", (ULONG)info.out_img[i].addr[0]);
						p_hdl->dbg_info.unlock_cnt++;
						p_hdl->lock_cnt--;
					}
				}
			}
		}
	} else {

		DBG_DUMP("--fastboot exit--\n");

		/* d2d mode, wait all hdl receive last frame and finish job */
		vos_flag_wait(&wait_flg, p_ctl->proc_tsk_flg_id, (KDRV_IPP_BUILTIN_TSK_EXIT_END | KDRV_IPP_BUILTIN_TSK_FASTBOOT_DONE), TWF_ANDW);

		/* destroy flg */
		vos_flag_destroy(p_ctl->proc_tsk_flg_id);

		/* flush job, should have no job remain */
		kdrv_ipp_builtin_flush_job();

		/* release job pool */
		kdrv_ipp_builtin_plat_free((void *)p_ctl->job_pool.start_addr);

		/* unregister ime builtin cb */
		dev_id = KDRV_DEV_ID(KDRV_CHIP0, KDRV_VIDEOPROCS_IME_ENGINE0, 0);
		ime_builtin_unreg_fastboot_cb(dev_id);
	}

	/*unlock not used queued buffer*/
	for (k = 0; k < p_ctl->hdl_num; k++) {
		p_hdl = &p_ctl->p_hdl[k];

		for (i = 0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++) {

			if(p_hdl->queue_mode_en[i]){
				if(p_hdl->queue_num[i] - p_hdl->buf_num_in_que[i] > 0){

					for(j = p_hdl->queue_in_idx[i]; j < p_hdl->queue_num[i]; j++){

						if(p_hdl->vprc_blk[i][j].phy_addr){
							if(nvtmpp_unlock_fastboot_blk(p_hdl->vprc_blk[i][j].addr)!= 0)
								DBG_ERR("nvtmpp_unlock_fastboot_blk fail addr:%lx\n", (ULONG)p_hdl->vprc_blk[i][j].addr);
							p_hdl->dbg_info.unlock_cnt++;
							p_hdl->lock_cnt--;

							p_hdl->dbg_info.non_ring_release_idx[i] = p_hdl->queue_in_idx[i];
							p_hdl->dbg_info.non_ring_release_cnt[i]++;
						}
					}
				}
			}
		}
	}

	for (i = 0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++){
		if(p_hdl->queue_mode_en[i])
			DBG_DUMP("-[%d]ring=%d, r_idx=%d, r_cnt=%d-\n",i ,p_hdl->ring_flag[i], p_hdl->dbg_info.non_ring_release_idx[i], p_hdl->dbg_info.non_ring_release_cnt[i]);

	}

	/* unmap register */
	for (i = 0; i < KDRV_IPP_BUILTIN_ENG_MAX; i++) {
		if (p_ctl->reg_base[i] != NULL) {
			kdrv_ipp_builtin_plat_iounmap(p_ctl->reg_base[i]);
			p_ctl->reg_base[i] = NULL;
		}
	}
	/* release buffer */
	p_ctl->p_trig_hdl = NULL;
	//kdrv_ipp_builtin_plat_free(p_ctl->p_hdl);

	p_ctl->is_init = FALSE;
	KDRV_IPP_BUILTIN_DUMP("builtin ipp exit\r\n");

	/* Debug function for restore fastboot last frame to file*/
	for(i = 0; i < p_ctl->hdl_num; i++ ){

		kdrv_ipp_builtin_plat_free(p_ctl->p_hdl[i].p_dtsi_ife_buf);
		p_ctl->p_hdl[i].p_dtsi_ife_buf = NULL;
		kdrv_ipp_builtin_plat_free(p_ctl->p_hdl[i].p_dtsi_ipe_buf);
		p_ctl->p_hdl[i].p_dtsi_ipe_buf = NULL;
		kdrv_ipp_builtin_plat_free(p_ctl->p_hdl[i].p_dtsi_ime_buf);
		p_ctl->p_hdl[i].p_dtsi_ime_buf = NULL;

		if(p_ctl->p_hdl[i].dbg_func_en & KDRV_IPP_BUILTIN_FUNC_RESTORE_LAST_FRAME)
			kdrv_ipp_builtin_dbg(&p_ctl->p_hdl[i], DBG_RESTORE_LAST_FRAME_SAVE, &i);
	}

	return E_OK;
}

INT32 kdrv_ipp_builtin_reg_fmd_cb(KDRV_IPP_BUILTIN_FMD_CB fp)
{
	UINT32 i;

	for (i = 0; i < KDRV_IPP_BUILTIN_FMD_CB_NUM; i++) {
		if (g_kdrv_ipp_builtin_ctl.fmd_cb[i] == NULL) {
			g_kdrv_ipp_builtin_ctl.fmd_cb[i] = fp;
			KDRV_IPP_BUILTIN_DUMP("fmd_cb[%d] registered for 0x%.8lx\r\n", (int)i, (ULONG)fp);
			return E_OK;
		}
	}
	DBG_ERR("all fmd_cb have been registered\r\n");

	return E_SYS;
}

INT32 kdrv_ipp_builtin_reg_fmd_cb_by_isp_id(UINT32 isp_id, KDRV_IPP_BUILTIN_FMD_CB fp)
{
	UINT32 i;
	KDRV_IPP_BUILTIN_HDL *p_hdl;

	p_hdl = NULL;
	for (i = 0; i < g_kdrv_ipp_builtin_ctl.hdl_num; i++) {
		if (isp_id == g_kdrv_ipp_builtin_ctl.p_hdl[i].isp_id) {
			p_hdl = &g_kdrv_ipp_builtin_ctl.p_hdl[i];
			break;
		}
	}

	if (p_hdl == NULL) {
		DBG_ERR("find no hdl isp_id:%d!!!!!!\r\n", isp_id);
		return E_SYS;
	}

	for (i = 0; i < KDRV_IPP_BUILTIN_FMD_CB_NUM; i++) {
		if (p_hdl->fmd_cb[i] == NULL) {
			p_hdl->fmd_cb[i] = fp;
			KDRV_IPP_BUILTIN_DUMP("fmd_cb[%d] registered by isp_id:%d for 0x%.8lx\r\n", (int)i, isp_id, (ULONG)fp);
			return E_OK;
		}
	}
	DBG_ERR("all fmd_cb have been registered\r\n");

	return E_SYS;
}

INT32 kdrv_ipp_builtin_reg_isp_cb(KDRV_IPP_BUILTIN_ISP_CB fp)
{
	g_kdrv_ipp_isp_cb = fp;
	return E_OK;
}

KDRV_IPP_BUILTIN_IMG_INFO kdrv_ipp_builtin_get_path_info(UINT32 isp_id, KDRV_IPP_BUILTIN_PATH_ID pid)
{
	UINT32 buf_idx;
	KDRV_IPP_BUILTIN_IMG_INFO img = {0};
	KDRV_IPP_BUILTIN_IMG_INFO *p_img = &img;
	KDRV_IPP_BUILTIN_HDL *p_hdl;
	KDRV_IPP_BUILTIN_JOB *p_job;
	UINT32 i;
	static UINT32 pre_pts = 0;
	//KDRV_IPP_BUILTIN_CTL* p_ctl = &g_kdrv_ipp_builtin_ctl;
	p_hdl = NULL;
	for (i = 0; i < g_kdrv_ipp_builtin_ctl.hdl_num; i++) {
		if (isp_id == g_kdrv_ipp_builtin_ctl.p_hdl[i].isp_id) {
			p_hdl = &g_kdrv_ipp_builtin_ctl.p_hdl[i];
			break;
		}
	}


	if (p_hdl == NULL) {
		DBG_ERR("find no hdl isp_id:%d!!!!!!\r\n", isp_id);
		return img;
	}

	if(p_hdl->stitch_en[pid]){

		p_img->enable = p_hdl->path_info[pid].enable;
		p_img->fmt = p_hdl->path_info[pid].fmt;
		p_img->size.w = p_hdl->stitch_img_width[pid];
		p_img->size.h =  p_hdl->stitch_img_height[pid];//p_hdl->path_info[pid].size.h << 1;

		p_img->loff[0] = p_hdl->path_info[pid].loff[0];
		p_img->loff[1] = p_hdl->path_info[pid].loff[1];

		p_img->timestamp = p_hdl->path_info[pid].timestamp;
	}else{
		*p_img = p_hdl->path_info[pid];
	}

	if(p_hdl->queue_mode_en[pid]){

	}else {
		buf_idx = (p_hdl->fs_cnt + 1) % 2; //ping pong index
	}
	if (pid < 5) {

		if (p_img->enable) {
			if(p_hdl->queue_mode_en[pid]){
				/* path1/2/3/4/5 address */
				if(/*(p_hdl->queue_out_idx[pid] < p_hdl->queue_num) &&*/ (p_hdl->vprc_blk[pid][p_hdl->queue_out_idx[pid]].addr)) {

					p_img->addr[0] = p_hdl->vprc_blk[pid][p_hdl->queue_out_idx[pid]].addr;
					p_img->addr[1] = p_hdl->vprc_blk[pid][p_hdl->queue_out_idx[pid]].addr + (p_img->loff[0] * p_img->size.h);

					p_img->phyaddr[0] = nvtmpp_sys_va2pa(p_img->addr[0]);
					p_img->phyaddr[1] = nvtmpp_sys_va2pa(p_img->addr[1]);
				} else {
					DBG_ERR("output addr err, pid:%d, que_id:%d\n", pid, p_hdl->queue_out_idx[pid]);
					p_img->enable = DISABLE;
				}

			} else{
				/* path1/2/3/4 address */
				if (p_hdl->vprc_blk[pid][buf_idx].addr) {

					if(p_hdl->stitch_en[pid]){

						p_img->addr[0]  = p_hdl->vprc_blk[pid][buf_idx].addr;
						p_img->addr[1]  = p_hdl->vprc_blk[pid][buf_idx].addr + p_hdl->stitch_buf_y_size[pid];
					}else{
						p_img->addr[0] = p_hdl->vprc_blk[pid][buf_idx].addr;
						p_img->addr[1] = p_hdl->vprc_blk[pid][buf_idx].addr + (p_img->loff[0] * p_img->size.h);
					}
					p_img->phyaddr[0] = nvtmpp_sys_va2pa(p_img->addr[0]);
					p_img->phyaddr[1] = nvtmpp_sys_va2pa(p_img->addr[1]);

				} else {
					DBG_ERR("output addr err\n");
					p_img->enable = DISABLE;
				}
			}

		}

	} else if (pid == 4) {
		/* todo: ime referenct out */

	} else if (pid == 5) {
		/* todo: dce cfa out */
		p_img->enable = DISABLE;
	}

	if (p_hdl->flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW) {
		p_img->timestamp = p_hdl->fs_timestamp;
	} else if (p_hdl->p_cur_job) {
		p_job = (KDRV_IPP_BUILTIN_JOB *)p_hdl->p_cur_job;
		p_img->timestamp = p_job->timestamp;
	}

	if(p_hdl->dbg_func_en & KDRV_IPP_PARAM_BUILTIN_SET_PTS_MSG_EN){
		if(pid == KDRV_IPP_BUILTIN_PATH_ID_1){
			DBG_DUMP("-c:%d,t:%d,d:%d-\n", p_hdl->fs_cnt, p_img->timestamp, p_img->timestamp - pre_pts);
			pre_pts = p_img->timestamp;
		}
	}

	return img;
}

KDRV_IPP_BUILTIN_PHY_OUT_INFO* kdrv_ipp_builtin_get_phy_out_info(UINT32 isp_id)
{
	UINT32 i;

	isp_id = isp_id & 0xff;

	if (g_kdrv_ipp_builtin_ctl.p_hdl) {
		/* old version only support one direct mode handle */
		if (kdrv_ipp_builtin_dtsi_ver() == 0) {
			return &g_kdrv_ipp_builtin_ctl.p_hdl[0].last_out_info;
		}

		/* new version, find compatible hdl_name then return
			only support get phy out once
		*/
		for (i = 0; i < g_kdrv_ipp_builtin_ctl.hdl_num; i++) {
			if (isp_id == g_kdrv_ipp_builtin_ctl.p_hdl[i].isp_id) {
				if (g_kdrv_ipp_builtin_ctl.p_hdl[i].get_phy_out_cnt == 0) {
					g_kdrv_ipp_builtin_ctl.p_hdl[i].get_phy_out_cnt = 1;
					return &g_kdrv_ipp_builtin_ctl.p_hdl[i].last_out_info;
				} else {
					return NULL;
				}
			}
		}
		DBG_ERR("find no builtin_hdl with isp_id %d\r\n", isp_id);
	} else {
		DBG_ERR("builtin handle already released\r\n");
	}

	return NULL;
}

BOOL kdrv_ipp_builtin_get_status(void)
{
	return g_kdrv_ipp_builtin_ctl.is_init;
}

BOOL kdrv_ipp_builtin_is_direct(void)
{
	if(g_kdrv_ipp_builtin_ctl.p_hdl[0].flow == KDRV_IPP_BUILTIN_FLOW_DIRECT_RAW)
		return 1;
	else
		return 0;
}

BOOL kdrv_ipp_builtin_is_fastboot(void)
{
	return g_kdrv_ipp_builtin_ctl.is_fastboot;
}

INT32 kdrv_ipp_builtin_unlock_queued_buf(UINT32 isp_id, UINT32 path_id, UINT32 queue_buf_idx)
{

	UINT32 i=0;
	INT32 rt = 0;

	for(i=0; i < g_kdrv_ipp_builtin_ctl.hdl_num; i++){
		if(isp_id == g_kdrv_ipp_builtin_ctl.p_hdl[i].isp_id)
			break;
	}

	if(i < g_kdrv_ipp_builtin_ctl.hdl_num ){

		if(g_kdrv_ipp_builtin_ctl.p_hdl[i].vprc_blk[path_id][queue_buf_idx].addr){

			//in queue mode, 3dnr ref buff only can be unlocked when both hdal vproc & other user all can unlock
			rt = nvtmpp_unlock_fastboot_blk(g_kdrv_ipp_builtin_ctl.p_hdl[i].vprc_blk[path_id][queue_buf_idx].addr);
			g_kdrv_ipp_builtin_ctl.p_hdl[i].lock_cnt--;
			g_kdrv_ipp_builtin_ctl.p_hdl[i].dbg_info.unlock_cnt++;

		}else{
			DBG_ERR("unlock addr fail:0x%lx,isp_id:%d, path_id:%d\n", g_kdrv_ipp_builtin_ctl.p_hdl[i].vprc_blk[path_id][queue_buf_idx].phy_addr, isp_id, path_id);
		}
	}
	else{

		DBG_ERR("get isp info error, isp_id:%d\n",isp_id);
	}

	return rt;
}

INT32 kdrv_ipp_builtin_unlock_3dnr_ref(UINT32 isp_id) //tmp need check
{

	UINT32 i=0;

	for(i=0; i < g_kdrv_ipp_builtin_ctl.hdl_num; i++){
		if(isp_id == g_kdrv_ipp_builtin_ctl.p_hdl[i].isp_id)
			break;
	}

	if(i < g_kdrv_ipp_builtin_ctl.hdl_num ){

		if (g_kdrv_ipp_builtin_ctl.p_hdl[i]._3dnr_lock_addr) {
			KDRV_IPP_BUILTIN_DUMP("FASTBOOT UNLOCK 3DNR REF 0x%.8x\r\n", g_kdrv_ipp_builtin_ctl.p_hdl[i]._3dnr_lock_addr);
			if(nvtmpp_unlock_fastboot_blk(g_kdrv_ipp_builtin_ctl.p_hdl[i]._3dnr_lock_addr) != 0)
				DBG_ERR("nvtmpp_unlock_fastboot_blk fail addr:%lx\n", (ULONG)g_kdrv_ipp_builtin_ctl.p_hdl[i]._3dnr_lock_addr);
			g_kdrv_ipp_builtin_ctl.p_hdl[i].lock_cnt--;
			g_kdrv_ipp_builtin_ctl.p_hdl[i].dbg_info._3dnr_unlock_cnt++;
			g_kdrv_ipp_builtin_ctl.p_hdl[i]._3dnr_lock_addr = 0;
		}

	}
	else{

		DBG_ERR("get isp info error, isp_id:%d\n",isp_id);
	}



	return 0;
}

#if 0
#endif

static KDRV_IPP_BUILTIN_DTSI_CB g_kdrv_ipp_builtin_dtsi_cb = {NULL, NULL};

void kdrv_ipp_builtin_reg_dtsi_cb(KDRV_IPP_BUILTIN_DTSI_CB cb)
{
	g_kdrv_ipp_builtin_dtsi_cb = cb;
}

void kdrv_ipp_builtin_get_hdal_hdl_list(ULONG *hdl_list, UINT32 list_size)
{
	if (g_kdrv_ipp_builtin_dtsi_cb.get_hdl_list) {
		g_kdrv_ipp_builtin_dtsi_cb.get_hdl_list(hdl_list, list_size);
	}
}

void kdrv_ipp_builtin_get_hdal_reg_dtsi(ULONG hdl, UINT32 cnt, UINT32 **p_reg)
{
	if (g_kdrv_ipp_builtin_dtsi_cb.get_reg_dtsi) {
		g_kdrv_ipp_builtin_dtsi_cb.get_reg_dtsi(hdl, cnt, p_reg);
	}
}

int kdrv_ipp_builtin_frc_dump(char *frc_node, void* fd, char *pre_fix)
{
	char tmp_buf[128] = "";
	int len;
	UINT32 i = 0;

	len = snprintf(tmp_buf, sizeof(tmp_buf), "%sfrm-rate-ctrl {\r\n", pre_fix);

	vos_file_write((VOS_FILE)fd, (void *)tmp_buf, len);

	/* skip */
	len = snprintf(tmp_buf, 127, "%s\tskip = <", pre_fix);
	vos_file_write((VOS_FILE)fd, (void *)tmp_buf, len);

	for (i = 0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++){
		len = snprintf(tmp_buf, 127, "%d ", 0);
		vos_file_write((VOS_FILE)fd, (void *)tmp_buf, len);
	}

	len = snprintf(tmp_buf, 127, ">;\r\n");
	vos_file_write((VOS_FILE)fd, (void *)tmp_buf, len);

	/* src */
	len = snprintf(tmp_buf, 127, "%s\tsrc = <", pre_fix);
	vos_file_write((VOS_FILE)fd, (void *)tmp_buf, len);

	for (i = 0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++){
		len = snprintf(tmp_buf, 127, "%d ", 30);
		vos_file_write((VOS_FILE)fd, (void *)tmp_buf, len);
	}
	len = snprintf(tmp_buf, 127, ">;\r\n");
	vos_file_write((VOS_FILE)fd, (void *)tmp_buf, len);

	/* dst */
	len = snprintf(tmp_buf, 127, "%s\tdst = <", pre_fix);
	vos_file_write((VOS_FILE)fd, (void *)tmp_buf, len);
	for (i = 0; i < KDRV_IPP_BUILTIN_PATH_ID_MAX; i++){
		len = snprintf(tmp_buf, 127, "%d ", 30);
		vos_file_write((VOS_FILE)fd, (void *)tmp_buf, len);
	}
	len = snprintf(tmp_buf, 127, ">;\r\n");
	vos_file_write((VOS_FILE)fd, (void *)tmp_buf, len);


	len = snprintf(tmp_buf, sizeof(tmp_buf), "%s};\r\n", pre_fix);
	vos_file_write((VOS_FILE)fd, (void *)tmp_buf, len);

	return E_OK;
}

KDRV_IPP_BUILTIN_ISP_INFO* kdrv_ipp_builtin_get_isp_info(UINT32 isp_id)
{
	UINT32 i;

	isp_id = isp_id & 0xff;

	if (g_kdrv_ipp_builtin_ctl.p_hdl) {
		/* old version only support one direct mode handle */
		if (kdrv_ipp_builtin_dtsi_ver() == 0) {
			return &g_kdrv_ipp_builtin_ctl.p_hdl[0].isp_info;
		}

		/* new version, find compatible hdl_name then return
			only support get isp info once
		*/
		for (i = 0; i < g_kdrv_ipp_builtin_ctl.hdl_num; i++) {
			if (isp_id == g_kdrv_ipp_builtin_ctl.p_hdl[i].isp_id) {
				if (g_kdrv_ipp_builtin_ctl.p_hdl[i].get_isp_info_cnt== 0) {
					g_kdrv_ipp_builtin_ctl.p_hdl[i].get_isp_info_cnt = 1;
					return &g_kdrv_ipp_builtin_ctl.p_hdl[i].isp_info;
				} else {
					return NULL;
				}
			}
		}
		DBG_ERR("find no builtin_hdl with isp_id %d\r\n", isp_id);
	} else {
		DBG_ERR("builtin handle already released\r\n");
	}

	return NULL;
}

#else
INT32 kdrv_ipp_builtin_exit(void)
{
	return E_NOSPT;
}
void kdrv_ipp_builtin_update_timestamp(void)
{
	DBG_ERR("N/A\n");
}
KDRV_IPP_BUILTIN_PHY_OUT_INFO* kdrv_ipp_builtin_get_phy_out_info(UINT32 isp_id)
{
	DBG_ERR("N/A\n");
	return NULL;
}

BOOL kdrv_ipp_builtin_get_status(void)
{
	return FALSE;
}

BOOL kdrv_ipp_builtin_is_direct(void)
{
	return 0;
}

BOOL kdrv_ipp_builtin_is_fastboot(void)
{
	return FALSE;
}

int kdrv_ipp_builtin_frc_dump(char *frc_node, void* fd, char *pre_fix)
{
	return E_OK;
}

INT32 kdrv_ipp_builtin_unlock_queued_buf(UINT32 isp_id, UINT32 path_id, UINT32 queue_buf_idx)
{
	return 0;
}

INT32 kdrv_ipp_builtin_unlock_3dnr_ref(UINT32 isp_id)
{
	return 0;
}
INT32 kdrv_ipp_builtin_set(void *p_hdl, UINT32 param_id, void *p_data)
{
	DBG_ERR("N/A\n");
	return E_SYS;
}

INT32 kdrv_ipp_builtin_get(void *p_hdl, UINT32 param_id, void *p_data)
{
	DBG_ERR("N/A\n");
	return E_SYS;
}
void kdrv_ipp_builtin_reg_dtsi_cb(KDRV_IPP_BUILTIN_DTSI_CB cb)
{

}

void kdrv_ipp_builtin_get_hdal_hdl_list(ULONG *hdl_list, UINT32 list_size)
{

}

void kdrv_ipp_builtin_get_hdal_reg_dtsi(ULONG hdl, UINT32 cnt, UINT32 **p_reg)
{

}

KDRV_IPP_BUILTIN_ISP_INFO* kdrv_ipp_builtin_get_isp_info(UINT32 isp_id)
{
	return NULL;
}
#endif

