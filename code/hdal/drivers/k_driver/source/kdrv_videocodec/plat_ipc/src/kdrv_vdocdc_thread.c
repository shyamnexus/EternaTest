#include <plat/top.h>
#include "comm/timer.h"
#include "comm/hwclock.h"

#include "kwrap/type.h"
#include "kwrap/task.h"
#include "kwrap/flag.h"
#include "kwrap/semaphore.h"
#include "kwrap/platform.h"
#include "kwrap/util.h"
#include "kwrap/spinlock.h"
#include "kwrap/stdio.h"
#include "kwrap/debug.h"

#include "kdrv_vdocdc_comn.h"
#include "kdrv_vdocdc_thread.h"

#include "h26x.h"
#include "h26x_common.h"
#include "h26xenc_api.h"
#include "h264enc_api.h"
#include "h264dec_api.h"
#include "h265enc_api.h"
#include "h265dec_api.h"

#define KDRV_VDOCDC_TSK_STACK_SIZE	(4096)
#define KDRV_VDOCDC_TSK_PRIORITY	(3)
#define KDRV_THREAD_DBG_EN			(1)

#define KDRV_VDOCDC_JOB_MAX			(8)

#if defined(__LINUX)
#define KDRV_VDOCDC_JOB_ALLOC		(1)
#else
#define KDRV_VDOCDC_JOB_ALLOC		(0)
#endif

#define FLG_VDOCDC_IDLE 		FLGPTN_BIT(0)
#define FLG_VDOCDC_STOP 		FLGPTN_BIT(1)
#define FLG_VDOCDC_STOP_DONE	FLGPTN_BIT(2)
#define FLG_VDOCDC_RUN_TRIG		FLGPTN_BIT(3)

THREAD_HANDLE _SECTION(".kercfg_data") KDRV_VDOCDC_TSK_HDL_RUN;

ID FLG_ID_VDOCDC_RUN = 0;

static vk_spinlock_t vdocdc_job_lock;

static BOOL gbVdoCdcThreadOpend = FALSE;

static unsigned int g_vdocdc_job_cnt = 0;
static unsigned int g_enc_job_cnt = 0;
static unsigned int g_dec_job_cnt = 0;

KDRV_VDOCDC_JOB g_vdocdc_job[KDRV_VDOCDC_JOB_MAX] = {0};

KDRV_VDOCDC_JOB *g_vdocdc_start_job = NULL;
KDRV_VDOCDC_JOB *g_vdocdc_end_job = NULL;

static void (*g_vdoenc_osdmask_cb)(UINT32 path_id, UINT32 isDo) = NULL;

extern KDRV_CALLBACK_FUNC *h26xd_cb_func;

static UINT64 get_timer(void)
{
	return hwclock_get_counter();
}

static void run_execute(void)
{
	UINT32 channel_id = KDRV_DEV_ID_CHANNEL(g_vdocdc_start_job->id);

	//g_vdocdc_start_job->status = VDOCDC_JOB_RUN_START;

	if (H26X_ENC_MODE) {
		h26x_setEncDirectRegSet(0, g_vdocdc_start_job->apb_addr);
	}
	#if 0
	else {
		h26x_setEncLLRegSet(0, 1, h26x_getPhyAddr(h26xEnc_getVaLLCAddr(g_vdocdc_start_job->p_var)));
	}
	#endif

	if (g_vdocdc_start_job->codec_mode == VDOCDC_ENC_MODE) {
		vdoenc_info *info = g_vdocdc_start_job->info;

		H26XCOMN_CTX *pComnCtx = info->enc_var.pComnCtx;

		if (pComnCtx->bSliceLowLatencyEn) {
			h26x_setSliceSetCtrlEn(info->enc_var.uiChipIdx);
		}
	}
	else {
		vdodec_info *info = g_vdocdc_start_job->info;
		KDRV_VDODEC_PARAM *p_dec_param = g_vdocdc_start_job->p_param;
		H26XDEC_VAR *p_var = &info->dec_var;

		if (p_dec_param->cur_bs_size != 0) {
			h26x_setIntEn(p_var->uiChipIdx, H26X_FINISH_INT | H26X_BSDMA_INT | H26X_ERR_INT | H26X_BSOUT_INT | H26X_FBC_ERR_INT | H26X_SWRST_FINISH_INT);
		}
		else {
			h26x_setIntEn(p_var->uiChipIdx, H26X_FINISH_INT | H26X_BSDMA_INT | H26X_ERR_INT | H26X_TIME_OUT_INT | H26X_FRAME_TIME_OUT_INT | H26X_BSOUT_INT | H26X_FBC_ERR_INT | H26X_SWRST_FINISH_INT);
		}
	}

	g_vdocdc_start_job->start_time = get_timer();

	h26x_start(0);

	//DBG_FUNC("h26x_start : %d\r\n", g_vdocdc_start_job->id);
	g_vdocdc_start_job->status = VDOCDC_JOB_RUN_ONGOING;
	g_vdocdc_start_job->interrupt = (h26x_waitINT(0) & ~(H26X_SRC_DECMP_ERR_INT));

	if (g_vdocdc_start_job->codec_mode == VDOCDC_ENC_MODE) {
		vdoenc_info *info = g_vdocdc_start_job->info;

		H26XCOMN_CTX *pComnCtx = info->enc_var.pComnCtx;

		if (pComnCtx->bSliceLowLatencyEn && info->vdoenc_slice_low_latency_cb != NULL) {
			while(g_vdocdc_start_job->interrupt & H26X_SLICE_DONE_INT) {
				KDRV_H26XENC_SLICE_LOW_LATENCY_RESULT sSliceResult = {0};

				sSliceResult.uiChannelId = channel_id;
				sSliceResult.uiSliceIdx = pComnCtx->uiSliceCurIdx;
				sSliceResult.bIsIFrame = info->b_is_i_frm;
				sSliceResult.uiBsOnlySize = h26x_getSliceBsLenwoDummy(info->enc_var.uiChipIdx);
				sSliceResult.uiTotalSize  = h26x_getSliceBsLenwiDummy(info->enc_var.uiChipIdx);

				info->vdoenc_slice_low_latency_cb(&sSliceResult);

				pComnCtx->uiSliceCurIdx++;

				if (pComnCtx->uiSliceCurIdx != h26xEnc_getLastSliceForSliceLowLatency(&info->enc_var)) {
					h26x_setSliceSetEn(info->enc_var.uiChipIdx, pComnCtx->uiSliceCurIdx);
				}
				else {
					h26x_setSliceSetEn(info->enc_var.uiChipIdx, 0x3ff);
				}

				g_vdocdc_start_job->interrupt = (h26x_waitINT(info->enc_var.uiChipIdx) & ~(H26X_SRC_DECMP_ERR_INT));
			}
		}
	}

	g_vdocdc_start_job->end_time = get_timer();
	g_vdocdc_start_job->status = VDOCDC_JOB_RUN_DONE;
	DBG_FUNC("interrupt = 0x%x\r\n", g_vdocdc_start_job->interrupt);

	if (g_vdocdc_start_job->codec_mode == VDOCDC_ENC_MODE) {
		vdoenc_info *info = g_vdocdc_start_job->info;

		KDRV_VDOENC_PARAM *p_enc_param = g_vdocdc_start_job->p_param;

		H26XENC_VAR *p_var = &info->enc_var;
		H26XCOMN_CTX *pComnCtx = p_var->pComnCtx;

		H26XEncResultCfg sResult = {0};
		H26XEncNaluLenResult nalu_len_rslt = {0};

		UINT32 re_trigger = 0;
		UINT64 sse;

		if (g_vdocdc_start_job->interrupt ==  H26X_BSOUT_INT && p_enc_param->bs_addr_2 && (p_enc_param->bs_size_2 >> 7)) {
			h26x_setNextBsBuf(p_var->uiChipIdx, h26x_getPhyAddr(p_enc_param->bs_addr_2), (p_enc_param->bs_size_2 >> 7) << 7, 0, 0);
			re_trigger = 1;
			g_vdocdc_start_job->status = VDOCDC_JOB_RUN_RETRIGGER;
			g_vdocdc_start_job->interrupt = (h26x_waitINT(p_var->uiChipIdx) & ~(H26X_SRC_DECMP_ERR_INT));
			g_vdocdc_start_job->status = VDOCDC_JOB_RUN_DONE;
		}

		if (p_var->eCodecType == VCODEC_H265) {
			h265Enc_getResult(p_var, H26X_ENC_MODE, &sResult, g_vdocdc_start_job->interrupt);
		}
		else {
			h264Enc_getResult(p_var, H26X_ENC_MODE, &sResult, g_vdocdc_start_job->interrupt);
		}
		h26xEnc_getNaluLenResult(p_var, &nalu_len_rslt);

		if (g_vdocdc_start_job->interrupt == H26X_FINISH_INT) {
			pComnCtx->uiEncRatio = 100;
		}
		else {
			if (g_vdocdc_start_job->interrupt == H26X_BSOUT_INT) {
				pComnCtx->uiEncRatio = (p_var->eCodecType == VCODEC_H265) ? h265Enc_getEncodeRatio(p_var) : h264Enc_getEncodeRatio(p_var);
			}

			DBG_ERR("[0x%x]encode error interrupt(0x%08x)\r\n", g_vdocdc_start_job->id, g_vdocdc_start_job->interrupt);
			h26x_module_reset(p_var->uiChipIdx);
			//h26x_prtReg(0);

			p_enc_param->encode_err = TRUE;
			p_enc_param->re_encode_en = TRUE;
		}

		h26xEnc_getNaluLenResult(p_var, &nalu_len_rslt);

		p_enc_param->temproal_id = sResult.uiSvcLable;
		p_enc_param->encode_err = (g_vdocdc_start_job->interrupt != H26X_FINISH_INT);;
		p_enc_param->re_encode_en = (g_vdocdc_start_job->interrupt != H26X_FINISH_INT);
		p_enc_param->nxt_frm_type = sResult.ucNxtPicType;
		p_enc_param->base_qp = sResult.ucQP;
		if (sResult.uiBSLen > p_enc_param->bs_size_1) {
			re_trigger = 1;
		}
		p_enc_param->bs_size_1 = sResult.uiBSLen;
		p_enc_param->frm_type = sResult.ucPicType;
		p_enc_param->encode_time = sResult.uiHwEncTime;
		p_enc_param->motion_ratio = sResult.uiMotionRatio;
		p_enc_param->re_trigger = re_trigger;
		#if defined(__LINUX)
		sse = (((UINT64)sResult.uiYPSNR[1])<<32) | sResult.uiYPSNR[0];
		p_enc_param->y_mse = (UINT32)div_u64(sse, pComnCtx->uiWidth * pComnCtx->uiHeight);
		sse = (((UINT64)sResult.uiUPSNR[1])<<32) | sResult.uiUPSNR[0];
		p_enc_param->u_mse = (UINT32)div_u64(sse, (pComnCtx->uiWidth / 2) * (pComnCtx->uiHeight / 2));
		sse = (((UINT64)sResult.uiVPSNR[1])<<32) | sResult.uiVPSNR[0];
		p_enc_param->v_mse = (UINT32)div_u64(sse, (pComnCtx->uiWidth / 2) * (pComnCtx->uiHeight / 2));
		#else
		sse = (((UINT64)sResult.uiYPSNR[1])<<32) | sResult.uiYPSNR[0];
		p_enc_param->y_mse = (UINT32)(sse / pComnCtx->uiWidth / pComnCtx->uiHeight);
		sse = (((UINT64)sResult.uiUPSNR[1])<<32) | sResult.uiUPSNR[0];
		p_enc_param->u_mse = (UINT32)(sse / (pComnCtx->uiWidth / 2) / (pComnCtx->uiHeight / 2));
		sse = (((UINT64)sResult.uiVPSNR[1])<<32) | sResult.uiVPSNR[0];
		p_enc_param->v_mse = (UINT32)(sse / (pComnCtx->uiWidth / 2) / (pComnCtx->uiHeight / 2));
		#endif
		p_enc_param->evbr_still_flag = sResult.bEVBRStillFlag;

		p_enc_param->nalu_num       = nalu_len_rslt.uiSliceNum;
		p_enc_param->nalu_size_addr = nalu_len_rslt.uiVaAddr;
		//printk("int = 0x%x, bslen = %d, time(%lld, %lld, %lld)\r\n", g_vdocdc_start_job->interrupt, p_enc_param->bs_size_1, g_vdocdc_start_job->setup_time, g_vdocdc_start_job->start_time, g_vdocdc_start_job->end_time);
		g_vdocdc_start_job->callback->callback(p_enc_param, g_vdocdc_start_job->user_data);
	}
	else {
		vdodec_info *info = g_vdocdc_start_job->info;
		KDRV_VDODEC_PARAM *p_dec_param = g_vdocdc_start_job->p_param;
		H26XDEC_VAR *p_var = &info->dec_var;

		p_dec_param->interrupt = g_vdocdc_start_job->interrupt;

		if (g_vdocdc_start_job->interrupt & ~(H26X_FINISH_INT | H26X_BSDMA_INT)) {
			DBG_ERR("[0x%x]decode error interrupt:(0x%08x)\r\n", g_vdocdc_start_job->id, (unsigned int)g_vdocdc_start_job->interrupt);

            if (nvt_get_chip_id() == CHIP_NS02302) {
                // 538 only using module reset //
                DBG_IND("using module reset\r\n");
                h26x_module_reset(p_var->uiChipIdx);
            }
            else {
                // if DBG_PORT_1[31] != 0, only hw reset in timeout case //
                if ((!(h26x_getDbg1(0, 1) & (0x1<<31))) && (p_dec_param->interrupt & (H26X_FRAME_TIME_OUT_INT | H26X_ERR_INT))) {
                    DBG_IND("using module reset\r\n");
                    h26x_module_reset(p_var->uiChipIdx);
                }
                else {
                    DBG_IND("using sw reset\r\n");
                    h26x_reset(p_var->uiChipIdx);
                }
            }
			p_dec_param->errorcode = -1;
		}
		else {
			p_dec_param->errorcode = 0;
		}

		g_vdocdc_start_job->callback->callback(p_dec_param, g_vdocdc_start_job->user_data);

		if (info->dec_var.eCodecType == VCODEC_H264) {
			if ((h264Dec_getResYAddr(&info->dec_var) != 0) && g_vdocdc_start_job->interrupt == H26X_FINISH_INT)
				h26xd_cb_func->free_buf(h264Dec_getResYAddr(&info->dec_var), &channel_id);
			if ((h264Dec_getResYAddr2(&info->dec_var) != 0) && g_vdocdc_start_job->interrupt == H26X_FINISH_INT)
				h26xd_cb_func->free_buf(h264Dec_getResYAddr2(&info->dec_var), &channel_id);
		}
		else {
			if ((h265Dec_getResYAddr(&info->dec_var) != 0) && g_vdocdc_start_job->interrupt == H26X_FINISH_INT)
				h26xd_cb_func->free_buf(h265Dec_getResYAddr(&info->dec_var), &channel_id);
		}

	}

	g_vdocdc_start_job->status = VDOCDC_JOB_FINISH;

}

static void free_job(KDRV_VDOCDC_JOB *job)
{
	g_vdocdc_job_cnt--;
	g_enc_job_cnt -= (job->codec_mode == VDOCDC_ENC_MODE);
	g_dec_job_cnt -= (job->codec_mode == VDOCDC_DEC_MODE);

	memset(job , 0, sizeof(KDRV_VDOCDC_JOB));
}

static int _run_thread(void)
{
	FLGPTN flag = 0;

	THREAD_ENTRY();

	clr_flg(FLG_ID_VDOCDC_RUN, FLG_VDOCDC_IDLE);

	while(!THREAD_SHOULD_STOP){
		set_flg(FLG_ID_VDOCDC_RUN, FLG_VDOCDC_IDLE);
		PROFILE_TASK_IDLE();
		wai_flg(&flag, FLG_ID_VDOCDC_RUN, FLG_VDOCDC_STOP | FLG_VDOCDC_RUN_TRIG, TWF_ORW | TWF_CLR);
		PROFILE_TASK_BUSY();
		clr_flg(FLG_ID_VDOCDC_RUN, FLG_VDOCDC_IDLE);

		if (flag & FLG_VDOCDC_STOP) {
			break;
		}

		if (flag & FLG_VDOCDC_RUN_TRIG) {
			if (g_vdocdc_start_job == NULL) {
				DBG_ERR("vdocdc_run_thread trigger , but start_job is NULL\r\n");
			}
			else {
				while(g_vdocdc_start_job) {
					KDRV_VDOCDC_JOB *vdocdc_nxt_job = NULL;
					unsigned long lock_flag;

					run_execute();

					vk_spin_lock_irqsave(&vdocdc_job_lock, lock_flag);
					vdocdc_nxt_job = g_vdocdc_start_job->next_job;

					free_job(g_vdocdc_start_job);

					DBG_FUNC("run_sub : %d, %d\r\n", g_vdocdc_job_cnt, g_vdocdc_start_job->id);

					if (g_vdocdc_start_job == g_vdocdc_end_job) {
						g_vdocdc_start_job = NULL;
						g_vdocdc_end_job = NULL;
						clr_flg(FLG_ID_VDOCDC_RUN, FLG_VDOCDC_RUN_TRIG);	// Tuba 20231218: move venc run flag inside
						//printk("run thread idle\r\n");
					}
					else {
						g_vdocdc_start_job = vdocdc_nxt_job;
						g_vdocdc_start_job->status = VDOCDC_JOB_RUN_START;
					}
					vk_spin_unlock_irqrestore(&vdocdc_job_lock, lock_flag);
				}
			}
		}
	}

	return 0;
}

THREAD_DECLARE(vdocdc_run_thread, p1)
{
	_run_thread();
	set_flg(FLG_ID_VDOCDC_RUN, FLG_VDOCDC_STOP_DONE);
	THREAD_RETURN(0);
}

int kdrv_vdocdc_create_thread(void)
{
	if (gbVdoCdcThreadOpend == TRUE) {
		DBG_ERR("kdrv_vdocdc thread create already\r\n");
		return -1;
	}

	OS_CONFIG_FLAG(FLG_ID_VDOCDC_RUN);

	THREAD_CREATE(KDRV_VDOCDC_TSK_HDL_RUN, vdocdc_run_thread, NULL, "KDRV_VDOCDC_TSK_RUN");

	if (KDRV_VDOCDC_TSK_HDL_RUN == NULL) {
		DBG_ERR("kdrv_vdocdc create run thread fail\r\n");
		return -1;
	}

	vk_spin_lock_init(&vdocdc_job_lock);

	THREAD_SET_PRIORITY(KDRV_VDOCDC_TSK_HDL_RUN, KDRV_VDOCDC_TSK_PRIORITY);
	THREAD_RESUME(KDRV_VDOCDC_TSK_HDL_RUN);

	gbVdoCdcThreadOpend = TRUE;

	memset(&g_vdocdc_job, 0, sizeof(KDRV_VDOCDC_JOB)*KDRV_VDOCDC_JOB_MAX);
	g_vdocdc_job_cnt = 0;
	g_enc_job_cnt = 0;
	g_dec_job_cnt = 0;

	return 0;
}

int kdrv_vdocdc_remove_thread(void)
{
	FLGPTN flag = 0;

	if (gbVdoCdcThreadOpend == FALSE) {
		DBG_ERR("kdrv_vdocdc thread not create already\r\n");
		return -1;
	}

	// close thread //
	wai_flg(&flag, FLG_ID_VDOCDC_RUN, FLG_VDOCDC_IDLE, TWF_ORW);
	set_flg(FLG_ID_VDOCDC_RUN, FLG_VDOCDC_STOP);
	wai_flg(&flag, FLG_ID_VDOCDC_RUN, FLG_VDOCDC_STOP_DONE, TWF_ORW);

	gbVdoCdcThreadOpend = FALSE;

	memset(&g_vdocdc_job, 0, sizeof(KDRV_VDOCDC_JOB)*KDRV_VDOCDC_JOB_MAX);
	g_vdocdc_job_cnt = 0;

	return 0;
}

static int get_empty_job(void)
{
	int i;

	for (i = 0; i < KDRV_VDOCDC_JOB_MAX; i++) {
		if (g_vdocdc_job[i].status == VDOCDC_JOB_IDLE) {
			return i;
		}
	}

	return -1;
}

int kdrv_vdocdc_add_job(KDRV_VDOCDC_MODE codec_mode, UINT32 id, uintptr_t uiVaApbAddr, KDRV_CALLBACK_FUNC *p_cb_func, void *user_data, void *p_param, void *job_info)
{
	unsigned long flags;
	int job_idx = -1;

	if (p_cb_func == NULL || p_param == NULL || job_info == NULL) {
		DBG_ERR("job add error : input parameter(cb:%p, param:%p, info:%p) error\r\n", p_cb_func, p_param, job_info);
		return -1;
	}
	else {
		vk_spin_lock_irqsave(&vdocdc_job_lock, flags);
		if ((job_idx = get_empty_job()) == -1) {
			DBG_ERR("job queue no empty\r\n");
			vk_spin_unlock_irqrestore(&vdocdc_job_lock, flags);
			return -1;
		}

		g_vdocdc_job_cnt++;
		g_enc_job_cnt += (codec_mode == VDOCDC_ENC_MODE);
		g_dec_job_cnt += (codec_mode == VDOCDC_DEC_MODE);

		g_vdocdc_job[job_idx].setup_time = get_timer();
		g_vdocdc_job[job_idx].start_time = 0;
		g_vdocdc_job[job_idx].end_time = 0;

		g_vdocdc_job[job_idx].apb_addr = uiVaApbAddr;
		g_vdocdc_job[job_idx].interrupt = 0;
		g_vdocdc_job[job_idx].status = VDOCDC_JOB_STANDBY;

		g_vdocdc_job[job_idx].id = id;
		g_vdocdc_job[job_idx].codec_mode = codec_mode;
		g_vdocdc_job[job_idx].callback = p_cb_func;
		g_vdocdc_job[job_idx].user_data = user_data;
		g_vdocdc_job[job_idx].p_param = p_param;
		g_vdocdc_job[job_idx].info = job_info;

		if (g_vdocdc_start_job == NULL) {
			g_vdocdc_start_job = &g_vdocdc_job[job_idx];
			g_vdocdc_start_job->status = VDOCDC_JOB_RUN_START;
		}
		else {
			g_vdocdc_end_job->next_job = &g_vdocdc_job[job_idx];
		}
		g_vdocdc_end_job = &g_vdocdc_job[job_idx];
		g_vdocdc_end_job->next_job = NULL;

		//printk("add_job : %d, %d\r\n", g_vdocdc_job_cnt, g_vdocdc_job[job_idx].id);

		if (kchk_flg(FLG_ID_VDOCDC_RUN, FLG_VDOCDC_RUN_TRIG) == 0) {
			g_vdocdc_start_job->status = VDOCDC_JOB_RUN_START;
			set_flg(FLG_ID_VDOCDC_RUN, FLG_VDOCDC_RUN_TRIG);
		}
		vk_spin_unlock_irqrestore(&vdocdc_job_lock, flags);
	}

	return 0;
}

int kdrv_vdocdc_stop_job(UINT32 stop_job_id)
{
	KDRV_VDOCDC_JOB *job = NULL;
	KDRV_VDOCDC_JOB *pre_job = NULL;

	unsigned long lock_flag;

	BOOL b_find = FALSE;

	vk_spin_lock_irqsave(&vdocdc_job_lock, lock_flag);
	job = g_vdocdc_start_job;

	while(job != NULL){
		if (job->id == stop_job_id && job->status != VDOCDC_JOB_IDLE) {
			if (job->status == VDOCDC_JOB_STANDBY) {

				job->callback->callback(job->p_param, job->user_data);

				if (job == g_vdocdc_start_job) {
					if (g_vdocdc_start_job == g_vdocdc_end_job) {
						g_vdocdc_start_job = NULL;
						g_vdocdc_end_job = NULL;
					}
					else {
						g_vdocdc_start_job = job->next_job;
					}
				}
				else {
					if (job == g_vdocdc_end_job) {
						g_vdocdc_end_job = pre_job;
					}
					pre_job->next_job = job->next_job;
				}
				free_job(job);
				vk_spin_unlock_irqrestore(&vdocdc_job_lock, lock_flag);	// stop job after job done //
			}
			#if 1 // stop job after job done //
			else {
				vk_spin_unlock_irqrestore(&vdocdc_job_lock, lock_flag);
				while(job->status >= VDOCDC_JOB_RUN_START && job->status < VDOCDC_JOB_FINISH) {
					vos_task_delay_ms(1);
				}
			}
			#endif
			b_find = TRUE;
			break;
		}
		pre_job = job;
		job = job->next_job;
	}
	//vk_spin_unlock_irqrestore(&vdocdc_job_lock, lock_flag);	// stop job but not job done //

	if (b_find == FALSE) {
		vk_spin_unlock_irqrestore(&vdocdc_job_lock, lock_flag);	// stop job after job done //
		//DBG_WRN("stop_job_id(%d) not found\r\n", stop_job_id);
	}

	return 0;
}

int kdrv_vdocdc_set_llc_mem(UINT32 addr, UINT32 size)
{
	return 0;
}

UINT32 kdrv_vdocdc_get_llc_mem(void)
{
	return 0x100;
}

BOOL kdrv_vdocdc_get_job_done(void)
{
	return (g_vdocdc_job_cnt == 0);
}

BOOL kdrv_vdocdc_get_enc_job_done(void)
{
	return (g_enc_job_cnt == 0);
}

BOOL kdrv_vdocdc_get_dec_job_done(void)
{
	return (g_dec_job_cnt == 0);
}

void kdrv_vdocdc_set_osdmask_cb(KDRV_VDOENC_OSDMASKCB *pOsdMaskCB)
{
	g_vdoenc_osdmask_cb = pOsdMaskCB->vdoenc_osdmask_cb;
}

