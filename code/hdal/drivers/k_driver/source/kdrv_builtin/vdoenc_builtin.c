#if defined(__KERNEL__)
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/of.h>
#include <linux/dma-mapping.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/math64.h>
#elif defined(__FREERTOS)
#include <string.h>
#include "nvt_vdocdc_drv.h"
#endif
#include "kwrap/type.h"
#include "kwrap/task.h"
#include "kwrap/flag.h"
#include "kwrap/semaphore.h"
#include "kwrap/platform.h"
#include "kwrap/stdio.h"
#include "kwrap/debug.h"
#include "kwrap/util.h"
#include "kwrap/spinlock.h"

#include "kdrv_type.h"
#include "kdrv_videoenc/kdrv_videoenc.h"
#include <kdrv_videoenc/kdrv_videoenc_lmt.h>
#include "kdrv_ipp_builtin.h"
#include "vdoenc_builtin.h"
#include "vdoenc_builtin_platform.h"
#include "nvtmpp_init.h"
#include "h26x_def.h"
#include "h26x.h"
#include "h264_def.h"
#include "h264enc_api.h"
#include "h265_def.h"
#include "h265enc_api.h"
#include "vtrc.h"
#include "jpeg.h"
#include "jpg_enc.h"
#include <plat/top.h>
#include "bridge.h"
#include <nvt_api_ver.h>
#include "kdrv_builtin_debug.h" //must behind include kwarp/debug.h
#include "osg/osg_internal.h"

#define VENC_BUILTIN_NODE_HEAD "/fastboot/venc"

THREAD_HANDLE                                   VDOENC_BUILTIN_TSK_ID_H26X = 0;
THREAD_HANDLE                                   VDOENC_BUILTIN_TSK_ID_JPEG = 0;
ID                                              FLG_ID_VDOENC_BUILTIN_H26X = 0;
ID                                              FLG_ID_VDOENC_BUILTIN_JPEG = 0;
ID                                              FLG_ID_VDOENC_BUILTIN_CHECK[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
SEM_HANDLE                                      VDOENC_BUILTIN_BS_SEM_ID = 0;

static int nodeoffset = 0;
static int nodeoffset_list[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
VDOENC_BUILTIN_OBJ *gVdoEncBuiltinObj = NULL;
VDOENC_BUILTIN_JOBQ gVdoEncBuiltinJobQ_H26X = {0};
VDOENC_BUILTIN_JOBQ gVdoEncBuiltinJobQ_JPEG = {0};
H26XENC_VAR  *enc_var = NULL;
H264ENC_INIT init_avc_obj[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
H264ENC_INFO info_avc_obj[BUILTIN_VDOENC_PATH_ID_MAX] = {0};

H265ENC_INIT init_hevc_obj[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
H265ENC_INFO info_hevc_obj[BUILTIN_VDOENC_PATH_ID_MAX] = {0};

VDOENC_BUILTIN_DTSI_PARAM gvdoenc_dtsi_param[BUILTIN_VDOENC_PATH_ID_MAX];

UINT32 gVdoEnc_en[BUILTIN_VDOENC_PATH_ID_MAX];
UINT32 gVdoEnc_CodecType[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
static UINT32 bsque_max[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
static UINT32 gFrameRate[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
static UINT32 gSec[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
static int yuv_unlock_cnt[BUILTIN_VDOENC_PATH_ID_MAX] = {0}; // yuv_unlock_cnt == 2, set builtin_stop_encode as TRUE
static UINT32 builtin_stop_encode[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
static UINT32 SkipModeFrameIdx[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
static ULONG  gBSStart[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
static ULONG  gBSEnd[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
static BOOL   bH26xStart = FALSE;
static BOOL   disable_h26x_int = FALSE;
static UINT32 en_lnx_h26x_int_fisrt = 0;

// jpeg builtin dtsi
static UINT32 jpeg_quality[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
static UINT32 jpeg_fps[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
static UINT32 jpeg_width[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
static UINT32 jpeg_height[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
static UINT32 jpeg_max_mem_size[BUILTIN_VDOENC_PATH_ID_MAX] = {0};

// dynamic settings
static UINT32 dynamic_codec[BUILTIN_VDOENC_PATH_ID_MAX] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static UINT32 dynamic_rc_byterate[BUILTIN_VDOENC_PATH_ID_MAX] = {0};

// 3DNR & Post Sharpen
SIE_FB_ISR_FP vdoenc_builtin_get_sie_fp = NULL;
#if !defined(_BSP_NA51102_) && !defined(_BSP_NS02302_)
H26XEncTnrCfg h26x_enc_tnr[ISP_ID_MAX] = {0};
#endif
VDOENC_BUILTIN_H26XENC_SPN h26x_enc_spn[ISP_ID_MAX] = {0};

VDOENC_BUILTIN_START_ENC_CB vdoenc_start_enc_cb = NULL;
VDOENC_BUILTIN_RESULT_CB vdoenc_result_cb = NULL;

// pre enc in rtos
static BOOL bPreInit = 0;
static UINT32 pre_enc_num[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
static UINT32 rtos_yuv_num[BUILTIN_VDOENC_PATH_ID_MAX] = {0};

// Rate control
VDOENC_BUILTIN_RC_PARAM gRcParam[BUILTIN_VDOENC_PATH_ID_MAX] = {0};


static VK_DEFINE_SPINLOCK(my_lock);

UINT32 VdoEnc_Builtin_spin_lock(void)
{
	unsigned long flags;
	vk_spin_lock_irqsave(&my_lock, flags);
	return flags;
}

void VdoEnc_Builtin_spin_unlock(UINT32 flags)
{
	vk_spin_unlock_irqrestore(&my_lock, flags);
}

INT32 vdoenc_builtin_reg_start_enc_cb(VDOENC_BUILTIN_START_ENC_CB fp)
{
	if (vdoenc_start_enc_cb == NULL) {
		vdoenc_start_enc_cb = fp;
		return 0;
	}

	DBG_ERR("dec_result_cb has been registered\r\n");

	return -1;
}

INT32 vdoenc_builtin_reg_result_cb(VDOENC_BUILTIN_RESULT_CB fp)
{
	if (vdoenc_result_cb == NULL) {
		vdoenc_result_cb = fp;
		return 0;
	}

	DBG_ERR("vdoenc_result_cb has been registered\r\n");

	return -1;
}

VOID vdoenc_builtin_set_pre_init_flag(BOOL isPreInit)
{
	bPreInit = isPreInit;
	return;
}

VOID vdoenc_builtin_set_pre_num(UINT32 pathID, ULONG pre_num)
{
	pre_enc_num[pathID] = (UINT32)pre_num;
	DBG_IND("pre_enc_num %d\r\n", pre_enc_num[pathID]);
	return;
}

VOID vdoenc_builtin_get_pre_num(UINT32 pathID, VOID *pre_num)
{
	*(UINT32 *)pre_num = pre_enc_num[pathID];
	return;
}

VOID vdoenc_builtin_set_en_lnx_h26x_int_first(ULONG value)
{
	en_lnx_h26x_int_fisrt = (UINT32)value;
	DBG_IND("set en_lnx_h26x_int_fisrt %d\r\n", en_lnx_h26x_int_fisrt);
	return;
}

VOID vdoenc_builtin_get_en_lnx_h26x_int_first(VOID *value)
{
	*(UINT32 *)value = en_lnx_h26x_int_fisrt;
	DBG_IND("get en_lnx_h26x_int_fisrt %d\r\n", *value);
	return;
}

VOID vdoenc_builtin_set_enc_obj_addr(ULONG enc_obj_addr)
{
#if defined(__KERNEL__)
	uintptr_t va = 0;
	UINT32 i = 0;

	va = (uintptr_t)vdoenc_builtin_ioremap(enc_obj_addr, sizeof(VDOENC_BUILTIN_OBJ) * BUILTIN_VDOENC_PATH_ID_MAX);
	DBG_IND("[vdoenc_builtin] linux gVdoEncBuiltinObj pa 0x%lx, va 0x%lx\r\n", (unsigned long)enc_obj_addr, (unsigned long)va);
	gVdoEncBuiltinObj = (VDOENC_BUILTIN_OBJ *)vdoenc_builtin_alloc(sizeof(VDOENC_BUILTIN_OBJ) * BUILTIN_VDOENC_PATH_ID_MAX);
	memcpy((void *)gVdoEncBuiltinObj, (void *)va, sizeof(VDOENC_BUILTIN_OBJ) * BUILTIN_VDOENC_PATH_ID_MAX);
	vdoenc_builtin_iounmap((void *)va);

	for (i = 0; i < BUILTIN_VDOENC_PATH_ID_MAX; i++) {
		DBG_IND("[vdoenc_builtin] linux gVdoEncBuiltinObj[%d] BeginTag=0x%x, EndTag=0x%x\r\n", i, gVdoEncBuiltinObj[i].BeginTag, gVdoEncBuiltinObj[i].EndTag);
	}
#endif
	return;
}

VOID vdoenc_builtin_get_enc_obj_addr(VOID *enc_obj_addr)
{
	DBG_IND("[vdoenc_builtin] rtos enc_obj_addr 0x%lx\r\n", (unsigned long)gVdoEncBuiltinObj);
	*(ULONG *)enc_obj_addr = (ULONG)gVdoEncBuiltinObj;
	return;
}

VOID vdoenc_builtin_set_enc_var_addr(ULONG enc_var_addr)
{
#if defined(__KERNEL__)
	uintptr_t va = 0;

	va = (uintptr_t)vdoenc_builtin_ioremap(enc_var_addr, sizeof(H26XENC_VAR) * BUILTIN_VDOENC_PATH_ID_MAX);
	DBG_IND("[vdoenc_builtin] linux enc_var_addr 0x%lx, va 0x%lx\r\n", (unsigned long)enc_var_addr, (unsigned long)va);
	enc_var = (H26XENC_VAR *)vdoenc_builtin_alloc(sizeof(H26XENC_VAR) * BUILTIN_VDOENC_PATH_ID_MAX);
	memcpy((void *)enc_var, (void *)va, sizeof(H26XENC_VAR) * BUILTIN_VDOENC_PATH_ID_MAX);
	vdoenc_builtin_iounmap((void *)va);
#endif

}

VOID vdoenc_builtin_get_enc_var_addr(VOID *enc_var_addr)
{
	DBG_IND("[vdoenc_builtin] rtos enc_var_addr 0x%lx\r\n", (unsigned long)enc_var);
	*(ULONG *)enc_var_addr = (ULONG)enc_var;
	return;
}

VOID vdoenc_builtIn_disable_h26x_int(VOID)
{
	unsigned long flags;
	FLGPTN uiFlag = 0;

	set_flg(FLG_ID_VDOENC_BUILTIN_H26X, FLG_VDOENC_BUILTIN_STOP);
	flags = VdoEnc_Builtin_spin_lock();
	if (bH26xStart == FALSE) {
		h26x_setIntEn(0, 0);
		disable_h26x_int = TRUE;
	}
	DBG_IND("vdoenc_builtIn_disable_h26x_int bH26xStart %d, disable_h26x_int %d\r\n", bH26xStart, disable_h26x_int);
	VdoEnc_Builtin_spin_unlock(flags);
	wai_flg(&uiFlag, FLG_ID_VDOENC_BUILTIN_H26X, FLG_VDOENC_BUILTIN_STOP_DONE, TWF_ORW);
}

int _vdoenc_builtin_get_gcd(int a, int b)
{
	while ((a != 0) && (b != 0)) {
		if (a > b)
			a = a % b;
		else
			b = b % a;
	}
	return (a + b);
}

BOOL _vdoenc_builtin_frc_update(UINT32 pathID, VDOENC_BUILTIN_FRC *p_frc, UINT32 framepersecond)
{
	UINT32               src_fr = 0;
	UINT32               dst_fr = 0;
	if((!p_frc))
		return FALSE;

	if (p_frc->sample_mode == VDOENC_BUILTIN_SAMPLE_OFF) {
		return TRUE;
	}
	if (p_frc->sample_mode == VDOENC_BUILTIN_SAMPLE_ALL) {
		return TRUE;
	}

	p_frc->frm_counter     = 0;
	p_frc->output_counter  = 0;
	p_frc->rate_counter    = 0;
	p_frc->framepersecond  = framepersecond;
	DBG_IND("[%d]: [frc] update, frc = 0x%x\r\n", pathID, framepersecond);
	if (GET_LO_UINT16(framepersecond) == 0) {
		p_frc->rate = VDOENC_BUILTIN_FRC_UNIT;
	} else {
		src_fr              = GET_LO_UINT16(framepersecond);
		dst_fr              = GET_HI_UINT16(framepersecond);

		if (src_fr > 7680) {  // if src=p30, it is equal to 1/256s
			DBG_ERR("[%d]: [frc] src=%d > 7680! stop output.\r\n", pathID, src_fr);
			p_frc->sample_mode = VDOENC_BUILTIN_SAMPLE_OFF;
		}
		if (dst_fr > (0xffffffff/VDOENC_BUILTIN_FRC_UNIT)) {
			DBG_ERR("[%d]: [frc] dst=%d > %u! stop output.\r\n", pathID, dst_fr, (0xffffffff/VDOENC_BUILTIN_FRC_UNIT));
			p_frc->sample_mode = VDOENC_BUILTIN_SAMPLE_OFF;
		}
		if ((src_fr <= 0) || (dst_fr <= 0)) {
			p_frc->sample_mode = VDOENC_BUILTIN_SAMPLE_OFF;
			return FALSE;
		}
		p_frc->rate            = ((dst_fr*VDOENC_BUILTIN_FRC_UNIT +(src_fr/2))/src_fr);   //rate=ROUND(dst*UNIT/src)
	}

	return TRUE;
}


UINT32 _vdoenc_builtin_frc_is_select(UINT32 pathID, VDOENC_BUILTIN_FRC *p_frc)
{
	UINT32 ret = 0, reset = FALSE;
	if((!p_frc))
		return 0;

	if (p_frc->sample_mode == VDOENC_BUILTIN_SAMPLE_OFF) {
		return 0;
	}
	if (p_frc->sample_mode == VDOENC_BUILTIN_SAMPLE_ALL) {
		//rate = 1.0
		p_frc->output_counter++;
		ret = 1;
		return ret;
	}

	if (p_frc->rate >= VDOENC_BUILTIN_FRC_UNIT) {
		//rate > 1.0
		p_frc->output_counter++;
		ret = 1;
	}
	else if (p_frc->frm_counter == 0) {
		p_frc->output_counter++;
		ret = 1;
	}
	else {
		p_frc->rate_counter += p_frc->rate;
		if (p_frc->rate_counter >= VDOENC_BUILTIN_FRC_UNIT) {
			p_frc->rate_counter -= VDOENC_BUILTIN_FRC_UNIT;
			p_frc->output_counter++;
			ret = 1;
		}
	}
	p_frc->frm_counter++;
	DBG_MSG("[%d]: [frc] frm_cnt %d , out_cnt %d\r\n", pathID, p_frc->frm_counter,p_frc->output_counter);

	// reset counter
	if (p_frc->framepersecond == 0 && p_frc->frm_counter >= 30) {
		reset = TRUE;
	}
	/*
	else if ((p_frc->framepersecond != 0) && (p_frc->frm_counter >= GET_LO_UINT16(p_frc->framepersecond))) {
		reset = TRUE;
	}
	*/
	else if ((p_frc->framepersecond != 0) && (p_frc->frm_counter >= GET_LO_UINT16(p_frc->framepersecond) / _vdoenc_builtin_get_gcd(GET_HI_UINT16(p_frc->framepersecond), GET_LO_UINT16(p_frc->framepersecond)))) {
		reset = TRUE;
	}
	if (reset) {
		if(p_frc->framepersecond_new != p_frc->framepersecond) {
			_vdoenc_builtin_frc_update(pathID, p_frc, p_frc->framepersecond_new);
		} else {
			p_frc->frm_counter     = 0;
			p_frc->output_counter  = 0;
			p_frc->rate_counter    = 0;
		}
	}
	return ret;
}

UINT32 _VdoEnc_builtin_GetBytesPerSecond(UINT32 width, UINT32 height)
{
	UINT32 bytePerSec;

	bytePerSec = width * height / 5;

	return bytePerSec;
}

BOOL _VdoEnc_Builtin_PutYuv(UINT32 pathID, VDOENC_BUILTIN_YUV_INFO *pYuvInfo)
{
	VDOENC_BUILTIN_YUVQ *pObj;
	unsigned long         flags;

	flags = VdoEnc_Builtin_spin_lock();

	pObj = &(gVdoEncBuiltinObj[pathID].yuvQueue);

	if (pObj->Front > VDOENC_BUILTIN_YUVQ_MAX - 1 ||  pObj->Rear > VDOENC_BUILTIN_YUVQ_MAX - 1) {
		DBG_ERR("%s:(line%d) ERROR INDEX !!! pathID=%d, BuiltinObj BeginTag=0x%x, EndTag=0x%x; pObj->Queue 0x%lx, f=%d/%d, r=%d/%d, full=%d, BeginTag=0x%x, EndTag=0x%x\r\n", __func__, __LINE__, pathID, gVdoEncBuiltinObj[pathID].BeginTag, gVdoEncBuiltinObj[pathID].EndTag, (unsigned long)pObj->Queue, pObj->Front, VDOENC_BUILTIN_YUVQ_MAX, pObj->Rear, VDOENC_BUILTIN_YUVQ_MAX, pObj->bFull, pObj->BeginTag, pObj->EndTag);
		VdoEnc_Builtin_spin_unlock(flags);
		return FALSE;
	}

	if ((pObj->Front == pObj->Rear) && (pObj->bFull == TRUE)) {
		VdoEnc_Builtin_spin_unlock(flags);
		return FALSE;
	} else {
		pObj->Queue[pObj->Rear].enable = pYuvInfo->enable;
		pObj->Queue[pObj->Rear].y_addr = pYuvInfo->y_addr;
		pObj->Queue[pObj->Rear].c_addr = pYuvInfo->c_addr;
		pObj->Queue[pObj->Rear].width = pYuvInfo->width;
		pObj->Queue[pObj->Rear].height = pYuvInfo->height;
		pObj->Queue[pObj->Rear].y_line_offset = pYuvInfo->y_line_offset;
		pObj->Queue[pObj->Rear].c_line_offset = pYuvInfo->c_line_offset;
		pObj->Queue[pObj->Rear].timestamp = pYuvInfo->timestamp;
		pObj->Queue[pObj->Rear].release_flag = pYuvInfo->release_flag;
#ifdef __KERNEL__
		pObj->Queue[pObj->Rear].isRtosYUV = FALSE;
#else
		pObj->Queue[pObj->Rear].isRtosYUV = TRUE;
#endif

		if (pObj->Queue[pObj->Rear].release_flag && pObj->Queue[pObj->Rear].enable) {
			DBG_IND("%s [%d] enable=%d,0x%08x\r\n",__func__,pathID,pYuvInfo->enable,pObj->Queue[pObj->Rear].y_addr);
			nvtmpp_lock_fastboot_blk(pObj->Queue[pObj->Rear].y_addr); //lock yuv blk
		}

		pObj->Rear = (pObj->Rear + 1) % VDOENC_BUILTIN_YUVQ_MAX;
		if (pObj->Front == pObj->Rear) { // Check Queue full
			pObj->bFull = TRUE;
		}
		VdoEnc_Builtin_spin_unlock(flags);
		return TRUE;
	}
}

BOOL _VdoEnc_Builtin_GetYuv(UINT32 pathID, VDOENC_BUILTIN_YUV_INFO *pYuvInfo)
{
	VDOENC_BUILTIN_YUVQ *pObj;
	unsigned long         flags;

	flags = VdoEnc_Builtin_spin_lock();

	pObj = &(gVdoEncBuiltinObj[pathID].yuvQueue);

	if (pObj->Front > VDOENC_BUILTIN_YUVQ_MAX - 1 ||  pObj->Rear > VDOENC_BUILTIN_YUVQ_MAX - 1) {
		DBG_ERR("%s:(line%d) ERROR INDEX !!! pathID=%d, BuiltinObj BeginTag=0x%x, EndTag=0x%x; pObj->Queue 0x%lx, f=%d/%d, r=%d/%d, full=%d, BeginTag=0x%x, EndTag=0x%x\r\n", __func__, __LINE__, pathID, gVdoEncBuiltinObj[pathID].BeginTag, gVdoEncBuiltinObj[pathID].EndTag, (unsigned long)pObj->Queue, pObj->Front, VDOENC_BUILTIN_YUVQ_MAX, pObj->Rear, VDOENC_BUILTIN_YUVQ_MAX, pObj->bFull, pObj->BeginTag, pObj->EndTag);
		VdoEnc_Builtin_spin_unlock(flags);
		return FALSE;
	}

	if ((pObj->Front == pObj->Rear) && (pObj->bFull == FALSE)) {
		VdoEnc_Builtin_spin_unlock(flags);
		DBG_ERR("get yuv queue empty!\r\n");
		return FALSE;
	} else {
		pYuvInfo->enable = pObj->Queue[pObj->Front].enable;

		if (pObj->Queue[pObj->Front].isRtosYUV == TRUE) {
			pYuvInfo->y_addr = nvtmpp_buitin_sys_pa2va(pObj->Queue[pObj->Front].y_addr);
			pYuvInfo->c_addr = nvtmpp_buitin_sys_pa2va(pObj->Queue[pObj->Front].c_addr);
		} else {
			pYuvInfo->y_addr = pObj->Queue[pObj->Front].y_addr;
			pYuvInfo->c_addr = pObj->Queue[pObj->Front].c_addr;
		}
		pYuvInfo->width = pObj->Queue[pObj->Front].width;
		pYuvInfo->height = pObj->Queue[pObj->Front].height;
		pYuvInfo->y_line_offset = pObj->Queue[pObj->Front].y_line_offset;
		pYuvInfo->c_line_offset = pObj->Queue[pObj->Front].c_line_offset;
		pYuvInfo->timestamp = pObj->Queue[pObj->Front].timestamp;
		pYuvInfo->release_flag = pObj->Queue[pObj->Front].release_flag;
		pObj->Front = (pObj->Front + 1) % VDOENC_BUILTIN_YUVQ_MAX;
		if (pObj->Front == pObj->Rear) { // Check Queue full
			pObj->bFull = FALSE;
		}
		VdoEnc_Builtin_spin_unlock(flags);
		return TRUE;
	}
}

UINT32 _VdoEnc_Builtin_HowManyInYUVQ(UINT32 pathID)
{
	UINT32 front, rear, full, sq = 0;
	VDOENC_BUILTIN_YUVQ *pObj;

	//SEM_WAIT(VDOENC_BUILTIN_YUV_SEM_ID); //wai_sem(NMP_VDODEC_SEM_ID[pathID]);
	pObj = &(gVdoEncBuiltinObj[pathID].yuvQueue);
	front = pObj->Front;
	rear = pObj->Rear;
	full = pObj->bFull;
	//SEM_SIGNAL(VDOENC_BUILTIN_YUV_SEM_ID); //sig_sem(NMP_VDODEC_SEM_ID[pathID]);
	if (front < rear) {
		sq = rear - front;
	} else if (front > rear) {
		sq = VDOENC_BUILTIN_YUVQ_MAX - (front - rear);
	} else if (front == rear && full == TRUE) {
		sq = VDOENC_BUILTIN_YUVQ_MAX;
	} else {
		sq = 0;
	}

	DBG_IND("%s:(line%d) sq=%d\r\n", __func__, __LINE__, sq);

	return sq;
}

BOOL _VdoEnc_builtin_PutJob_H26X(UINT32 pathID)
{
	unsigned long         flags;

	flags = VdoEnc_Builtin_spin_lock();
	if ((gVdoEncBuiltinJobQ_H26X.Front == gVdoEncBuiltinJobQ_H26X.Rear) && (gVdoEncBuiltinJobQ_H26X.bFull == TRUE)) {
		VdoEnc_Builtin_spin_unlock(flags);
		return FALSE;
	} else {
		gVdoEncBuiltinJobQ_H26X.Queue[gVdoEncBuiltinJobQ_H26X.Rear].pathID = pathID;
		gVdoEncBuiltinJobQ_H26X.Rear = (gVdoEncBuiltinJobQ_H26X.Rear + 1) % VDOENC_BUILTIN_JOBQ_MAX;
		if (gVdoEncBuiltinJobQ_H26X.Front == gVdoEncBuiltinJobQ_H26X.Rear) { // Check Queue full
			gVdoEncBuiltinJobQ_H26X.bFull = TRUE;
		}
		VdoEnc_Builtin_spin_unlock(flags);
		return TRUE;
	}
}

BOOL _VdoEnc_builtin_GetJob_H26X(UINT32 *pPathID)
{
	unsigned long         flags;

	flags = VdoEnc_Builtin_spin_lock();
	if ((gVdoEncBuiltinJobQ_H26X.Front == gVdoEncBuiltinJobQ_H26X.Rear) && (gVdoEncBuiltinJobQ_H26X.bFull == FALSE)) {
		VdoEnc_Builtin_spin_unlock(flags);
		DBG_ERR("[VDOTRIG] Job Queue(H26x) is Empty!\r\n");
		return FALSE;
	} else {
		*pPathID = gVdoEncBuiltinJobQ_H26X.Queue[gVdoEncBuiltinJobQ_H26X.Front].pathID;
		gVdoEncBuiltinJobQ_H26X.Front = (gVdoEncBuiltinJobQ_H26X.Front + 1) % VDOENC_BUILTIN_JOBQ_MAX;
		if (gVdoEncBuiltinJobQ_H26X.Front == gVdoEncBuiltinJobQ_H26X.Rear) { // Check Queue full
			gVdoEncBuiltinJobQ_H26X.bFull = FALSE;
		}
		VdoEnc_Builtin_spin_unlock(flags);
		return TRUE;
	}
}

UINT32 _VdoEnc_builtin_GetJobCount_H26X(void)
{
	UINT32 front, rear, full, sq = 0;
	unsigned long         flags;

	flags = VdoEnc_Builtin_spin_lock();
	front = gVdoEncBuiltinJobQ_H26X.Front;
	rear = gVdoEncBuiltinJobQ_H26X.Rear;
	full = gVdoEncBuiltinJobQ_H26X.bFull;
	VdoEnc_Builtin_spin_unlock(flags);

	if (front < rear) {
		sq = (rear - front);
	} else if (front > rear) {
		sq = (VDOENC_BUILTIN_JOBQ_MAX - (front - rear));
	} else if (front == rear && full == TRUE) {
		sq = VDOENC_BUILTIN_JOBQ_MAX;
	} else {
		sq = 0;
	}
	return sq;
}

BOOL _VdoEnc_builtin_PutJob_JPEG(UINT32 pathID)
{
	unsigned long		  flags;

	flags = VdoEnc_Builtin_spin_lock();
	if ((gVdoEncBuiltinJobQ_JPEG.Front == gVdoEncBuiltinJobQ_JPEG.Rear) && (gVdoEncBuiltinJobQ_JPEG.bFull == TRUE)) {
		VdoEnc_Builtin_spin_unlock(flags);
		return FALSE;
	} else {
		gVdoEncBuiltinJobQ_JPEG.Queue[gVdoEncBuiltinJobQ_JPEG.Rear].pathID = pathID;
		gVdoEncBuiltinJobQ_JPEG.Rear = (gVdoEncBuiltinJobQ_JPEG.Rear + 1) % VDOENC_BUILTIN_JOBQ_MAX;
		if (gVdoEncBuiltinJobQ_JPEG.Front == gVdoEncBuiltinJobQ_JPEG.Rear) { // Check Queue full
			gVdoEncBuiltinJobQ_JPEG.bFull = TRUE;
		}
		VdoEnc_Builtin_spin_unlock(flags);
		return TRUE;
	}
}

BOOL _VdoEnc_builtin_GetJob_JPEG(UINT32 *pPathID)
{
	unsigned long		  flags;

	flags = VdoEnc_Builtin_spin_lock();
	if ((gVdoEncBuiltinJobQ_JPEG.Front == gVdoEncBuiltinJobQ_JPEG.Rear) && (gVdoEncBuiltinJobQ_JPEG.bFull == FALSE)) {
		VdoEnc_Builtin_spin_unlock(flags);
		DBG_ERR("[VDOTRIG] Job Queue(H26x) is Empty!\r\n");
		return FALSE;
	} else {
		*pPathID = gVdoEncBuiltinJobQ_JPEG.Queue[gVdoEncBuiltinJobQ_JPEG.Front].pathID;
		gVdoEncBuiltinJobQ_JPEG.Front = (gVdoEncBuiltinJobQ_JPEG.Front + 1) % VDOENC_BUILTIN_JOBQ_MAX;
		if (gVdoEncBuiltinJobQ_JPEG.Front == gVdoEncBuiltinJobQ_JPEG.Rear) { // Check Queue full
			gVdoEncBuiltinJobQ_JPEG.bFull = FALSE;
		}
		VdoEnc_Builtin_spin_unlock(flags);
		return TRUE;
	}
}

UINT32 _VdoEnc_builtin_GetJobCount_JPEG(void)
{
	UINT32 front, rear, full, sq = 0;
	unsigned long		  flags;

	flags = VdoEnc_Builtin_spin_lock();
	front = gVdoEncBuiltinJobQ_JPEG.Front;
	rear = gVdoEncBuiltinJobQ_JPEG.Rear;
	full = gVdoEncBuiltinJobQ_JPEG.bFull;
	VdoEnc_Builtin_spin_unlock(flags);

	if (front < rear) {
		sq = (rear - front);
	} else if (front > rear) {
		sq = (VDOENC_BUILTIN_JOBQ_MAX - (front - rear));
	} else if (front == rear && full == TRUE) {
		sq = VDOENC_BUILTIN_JOBQ_MAX;
	} else {
		sq = 0;
	}
	return sq;
}

#if !defined(_BSP_NA51102_) && !defined(_BSP_NS02302_)
void _VdoEnc_builtin_3DNR_Internal(UINT32 pathID, UINT32 p3DNRConfig)
{
	SIE_FB_ISR_FP fp = vdoenc_builtin_get_sie_fp;
	UINT32 isp_id   = gvdoenc_dtsi_param[pathID].isp_id;

	if (fp != NULL)  {
		fp(isp_id, BUILTIN_ISP_EVENT_ENC_TNR);   // iq will then call vdoenc_builtin_set() to update 3DNR parameters
	}
#if TNR_DBG
	DBG_DUMP("[VDOENC_BUILTIN][%d] isp_id = %d, SIE tnr = (%d, %d, %d, %d), fp = 0x%08x\r\n",
		pathID,
		isp_id,
		h26x_enc_tnr[isp_id].nr_3d_mode,
		h26x_enc_tnr[isp_id].nr_3d_adp_th_p2p[0],
		h26x_enc_tnr[isp_id].nr_3d_adp_th_p2p[1],
		h26x_enc_tnr[isp_id].nr_3d_adp_th_p2p[2],
		(UINT32)fp);
#endif

	// copy iq 3DNR parameters to H26x driver
	memcpy((void *)p3DNRConfig, &h26x_enc_tnr[isp_id], sizeof(H26XEncTnrCfg));
}
#endif

void _VdoEnc_builtin_PostSharpen_Internal(UINT32 pathID, ULONG pPostSharpenConfig)
{
	SIE_FB_ISR_FP fp = vdoenc_builtin_get_sie_fp;
	UINT32 isp_id = gvdoenc_dtsi_param[pathID].isp_id;

//	if (isp_id == ISP_ID_IGNORE) {
//		isp_id = ISP_ID_1; // TODO: should use videoproc push ISF_DATA's isp_id later;
//	}

	//TODO: isp_id = 0~7 sensor id,  >7 virtual isp id,   0xffffffff  don't care
	if (fp != NULL)  {
		fp(isp_id, BUILTIN_ISP_EVENT_ENC_SHARPEN);   // iq will then call kflow_videoenc_set() to update postsharpen parameters
	}

#if SPN_DBG
	DBG_DUMP("[VDOENC_BUILTIN][%d] isp_id = %d, SIE spn = (%d, %d, %d, %d), fp = 0x%08x\r\n",
		pathID,
		isp_id,
		h26x_enc_spn[isp_id].bEnable,
		h26x_enc_spn[isp_id].ucEdgeSharpStr1,
		h26x_enc_spn[isp_id].ucEdgeStr,
		h26x_enc_spn[isp_id].ucTransitionStr,
		(UINT32)fp);
#endif

	// copy iq postsharpen parameters to H26x driver
	memcpy((void *)pPostSharpenConfig, &h26x_enc_spn[isp_id], sizeof(VDOENC_BUILTIN_H26XENC_SPN));
}

void _VdoEnc_builtin_Get_SrcYuv_Info(VDOENC_BUILTIN_FMD_INFO *pSrc, VDOENC_BUILTIN_YUV_INFO *pDst, UINT32 ipp_pathID)
{
	pDst->enable = pSrc->out_img[ipp_pathID].enable;
	pDst->y_addr = pSrc->out_img[ipp_pathID].addr[0];
	pDst->c_addr = pSrc->out_img[ipp_pathID].addr[1];
	pDst->width = pSrc->out_img[ipp_pathID].size.w;
	pDst->height = pSrc->out_img[ipp_pathID].size.h;
	pDst->y_line_offset = pSrc->out_img[ipp_pathID].loff[0];
	pDst->c_line_offset = pSrc->out_img[ipp_pathID].loff[1];
	pDst->timestamp = (UINT32)pSrc->out_img[ipp_pathID].timestamp;
	pDst->release_flag = pSrc->release_flg;
}

int vdoenc_builtin_get_of_node_venc(void)
{
	if (vdoenc_plat_get_nodeoffset(VENC_BUILTIN_NODE_HEAD, &nodeoffset) < 0) {
		// error handling
		DBG_ERR("%s get nodeofs fail\r\n", VENC_BUILTIN_NODE_HEAD);
		return -1;
	}

	vdoenc_plat_get_nodeoffset_list(nodeoffset, &nodeoffset_list[0]);

	return 0;
}

BOOL vdoenc_builtin_set(UINT32 id, BUILTIN_VDOENC_ISP_ITEM item, void *data)
{
#if !defined(_BSP_NA51102_) && !defined(_BSP_NS02302_)
	if (item == BUILTIN_VDOENC_ISP_ITEM_TNR) {
		memcpy(&h26x_enc_tnr[id], data, sizeof(H26XEncTnrCfg));
	}
#endif

	if (item == BUILTIN_VDOENC_ISP_ITEM_SPN) {
		memcpy(&h26x_enc_spn[id], data, sizeof(VDOENC_BUILTIN_H26XENC_SPN));
	}

	return TRUE;
}

BOOL vdoenc_builtin_evt_fp_reg(CHAR *name, SIE_FB_ISR_FP fp)
{
	vdoenc_builtin_get_sie_fp = fp;  // Let PQ team register callback function
	return TRUE;
}

BOOL vdoenc_builtin_evt_fp_unreg(CHAR *name)
{
	vdoenc_builtin_get_sie_fp = NULL;  // Let PQ team register callback function
	return TRUE;
}

BOOL VdoEnc_Builtin_AllocQueMem(UINT32 pathID, UINT32 bsq_max_num, UINT32 yuvq_max_num, BOOL is_pre_init)
{
	UINT32 uiBufSize = sizeof(VDOENC_BUILTIN_BS_INFO) * bsq_max_num;
	PVDOENC_BUILTIN_BSQ pObj_bs;
	PVDOENC_BUILTIN_YUVQ pObj_yuv;
#if defined(__KERNEL__)
	uintptr_t va = 0;
#endif

	if (uiBufSize == 0) {
		DBG_ERR("[VDOBUILTIN] AllocQueMem fail\r\n");
		return FALSE;
	}
	// bsq
	pObj_bs = &(gVdoEncBuiltinObj[pathID].bsQueue);
	if (is_pre_init == 0) {
		pObj_bs->BeginTag = MAKEFOURCC('B', 'S', 'Q', 'B');
		pObj_bs->bFull = 0;
		pObj_bs->Front = 0;
		pObj_bs->Rear = 0;
		pObj_bs->Queue = (VDOENC_BUILTIN_BS_INFO *)vdoenc_builtin_alloc(uiBufSize);
		pObj_bs->EndTag = MAKEFOURCC('B', 'S', 'Q', 'E');
		DBG_IND("[%d] VdoEnc_Builtin_AllocQueMem pObj_bs->Queue=0x%lx, f=%d, r=%d, Full=%d, BeginTag=0x%x, EndTag=0x%x\r\n", (int)pathID, (unsigned long)pObj_bs->Queue, pObj_bs->Front, pObj_bs->Rear, pObj_bs->bFull, pObj_bs->BeginTag, pObj_bs->EndTag);
		if (pObj_bs->Queue == 0) {
			DBG_ERR("[vdoenc_builtin][%d] invalid pObj_bs->Queue addr 0x%lx\r\n", (int)pathID, (unsigned long)pObj_bs->Queue);
			return FALSE;
		}
		memset(pObj_bs->Queue, 0, uiBufSize);
	} else {
#if defined(__KERNEL__)
		va = (uintptr_t)vdoenc_builtin_ioremap((ULONG)pObj_bs->Queue, uiBufSize);
		pObj_bs->Queue = (VDOENC_BUILTIN_BS_INFO *)vdoenc_builtin_alloc(uiBufSize);
		if (pObj_bs->Queue == 0) {
			DBG_ERR("[vdoenc_builtin][%d] invalid pObj_bs->Queue addr 0x%lx\r\n", (int)pathID, (unsigned long)pObj_bs->Queue);
			return FALSE;
		}
		memcpy((void *)pObj_bs->Queue, (void *)va, uiBufSize);
		vdoenc_builtin_iounmap((void *)va);
#endif
	}

	// yuvq
	pObj_yuv = &(gVdoEncBuiltinObj[pathID].yuvQueue);
	uiBufSize = sizeof(VDOENC_BUILTIN_YUV_INFO) * yuvq_max_num;
	if (is_pre_init == 0) {
		pObj_yuv->BeginTag = MAKEFOURCC('Y', 'C', 'Q', 'B');
		pObj_yuv->bFull = 0;
		pObj_yuv->Front = 0;
		pObj_yuv->Rear = 0;
		pObj_yuv->Queue = (VDOENC_BUILTIN_YUV_INFO *)vdoenc_builtin_alloc(uiBufSize);
		pObj_yuv->EndTag = MAKEFOURCC('Y', 'C', 'Q', 'E');
		DBG_IND("[%d] VdoEnc_Builtin_AllocQueMem pObj_yuv->Queue=0x%lx, f=%d, r=%d, Full=%d, BeginTag=0x%x, EndTag=0x%x\r\n", (int)pathID, (unsigned long)pObj_yuv->Queue, pObj_yuv->Front, pObj_yuv->Rear, pObj_yuv->bFull, pObj_yuv->BeginTag, pObj_yuv->EndTag);
		if (pObj_yuv->Queue == 0) {
			DBG_ERR("[vdoenc_builtin][%d] invalid pObj_yuv->Queue addr 0x%lx\r\n", (int)pathID, (unsigned long)pObj_yuv->Queue);
			return FALSE;
		}
		memset(pObj_yuv->Queue, 0, uiBufSize);
	} else {
#if defined(__KERNEL__)
		va = (uintptr_t)vdoenc_builtin_ioremap((ULONG)pObj_yuv->Queue, uiBufSize);
		pObj_yuv->Queue = (VDOENC_BUILTIN_YUV_INFO *)vdoenc_builtin_alloc(uiBufSize);
		if (pObj_yuv->Queue == 0) {
			DBG_ERR("[vdoenc_builtin][%d] invalid pObj_yuv->Queue addr 0x%lx\r\n", (int)pathID, (unsigned long)pObj_yuv->Queue);
			return FALSE;
		}
		memcpy((void *)pObj_yuv->Queue, (void *)va, uiBufSize);
		vdoenc_builtin_iounmap((void *)va);
		rtos_yuv_num[pathID] = _VdoEnc_Builtin_HowManyInYUVQ(pathID);
#endif
	}

	return TRUE;
}

VOID VdoEnc_Builtin_FreeQueMem(UINT32 pathID)
{
	PVDOENC_BUILTIN_BSQ pObj_bs;
	PVDOENC_BUILTIN_YUVQ pObj_yuv;
	PVDOENC_BUILTIN_JOBQ pJobQH26X;
	PVDOENC_BUILTIN_JOBQ pJobQJPEG;

	pObj_bs = &(gVdoEncBuiltinObj[pathID].bsQueue);
	if (pObj_bs->Queue != NULL) {
		vdoenc_builtin_free(pObj_bs->Queue);
		pObj_bs->Queue = NULL;
	}

	pObj_yuv = &(gVdoEncBuiltinObj[pathID].yuvQueue);
	if (pObj_yuv->Queue != NULL) {
		vdoenc_builtin_free(pObj_yuv->Queue);
		pObj_yuv->Queue= NULL;
	}

	pJobQH26X = &gVdoEncBuiltinJobQ_H26X;
	if (pJobQH26X->Queue != NULL) {
		vdoenc_builtin_free(pJobQH26X->Queue);
		pJobQH26X->Queue = NULL;
	}

	pJobQJPEG = &gVdoEncBuiltinJobQ_JPEG;
	if (pJobQJPEG->Queue != NULL) {
		vdoenc_builtin_free(pJobQJPEG->Queue);
		pJobQJPEG->Queue = NULL;
	}
}

VOID vdoenc_builtin_free_obj_mem(VOID)
{
	if (gVdoEncBuiltinObj != NULL) {
		vdoenc_builtin_free(gVdoEncBuiltinObj);
	}

	if (enc_var != NULL) {
		vdoenc_builtin_free(enc_var);
	}
}

BOOL VdoEnc_Builtin_PutBS(UINT32 pathID, BOOL isKeyFrm, VDOENC_BUILTIN_PARAM *enc_param, H26XEncResultCfg *enc_result)
{
	PVDOENC_BUILTIN_BSQ pObj;

	if (enc_param == NULL) {
		DBG_ERR("[VDOBUILTIN] BS Lock Error enc_param is NULL\r\n");
		return FALSE;
	}

	if (enc_param->bs_addr_1 == 0 || enc_param->bs_size_1 == 0) {
		DBG_ERR("[VDOBUILTIN] BS Lock Error Addr 0x%lx Size is %d\r\n", enc_param->bs_addr_1, enc_param->bs_size_1);
		return FALSE;
	}

	SEM_WAIT(VDOENC_BUILTIN_BS_SEM_ID);

	pObj = &(gVdoEncBuiltinObj[pathID].bsQueue);

	if (gvdoenc_dtsi_param[pathID].codectype == BUILTIN_VDOENC_H264 || gvdoenc_dtsi_param[pathID].codectype == BUILTIN_VDOENC_H265) {
		if ((pObj->Front == pObj->Rear) && (pObj->bFull == TRUE)) {
			//DBG_ERR("[VdoEncBuiltin] BS Lock Queue is Full!\r\n");
			SEM_SIGNAL(VDOENC_BUILTIN_BS_SEM_ID);
			return FALSE;
		} else {
			pObj->Queue[pObj->Rear].Addr = enc_param->bs_addr_1;
			pObj->Queue[pObj->Rear].Size = enc_param->bs_size_1;
			pObj->Queue[pObj->Rear].temproal_id = enc_result->uiSvcLable;
			pObj->Queue[pObj->Rear].re_encode_en = (enc_param->interrupt != H26X_FINISH_INT);
			pObj->Queue[pObj->Rear].timestamp = enc_param->timestamp;
			pObj->Queue[pObj->Rear].nxt_frm_type = enc_result->ucNxtPicType;
			pObj->Queue[pObj->Rear].base_qp = enc_result->ucQP;
			pObj->Queue[pObj->Rear].bs_size_1 = enc_result->uiBSLen;
			pObj->Queue[pObj->Rear].frm_type = enc_result->ucPicType;
			pObj->Queue[pObj->Rear].encode_time = enc_result->uiHwEncTime;
			pObj->Queue[pObj->Rear].isKeyFrame = isKeyFrm;
#ifdef __KERNEL__
			pObj->Queue[pObj->Rear].isRtosBS = FALSE;
#else
			pObj->Queue[pObj->Rear].isRtosBS = TRUE;
#endif
			pObj->Rear = (pObj->Rear + 1) % bsque_max[pathID];
			if (pObj->Front == pObj->Rear) { // Check Queue full
				pObj->bFull = TRUE;
			}
		}
	} else if (gvdoenc_dtsi_param[pathID].codectype == BUILTIN_VDOENC_MJPEG) {
		if ((pObj->Front == pObj->Rear) && (pObj->bFull == TRUE)) {
			//DBG_ERR("[VdoEncBuiltin] BS Lock Queue is Full!\r\n");
			SEM_SIGNAL(VDOENC_BUILTIN_BS_SEM_ID);
			return FALSE;
		} else {
			pObj->Queue[pObj->Rear].Addr = enc_param->bs_addr_1;
			pObj->Queue[pObj->Rear].Size = enc_param->bs_size_1;
			pObj->Queue[pObj->Rear].temproal_id = pathID;
			pObj->Queue[pObj->Rear].re_encode_en = 0;
			pObj->Queue[pObj->Rear].timestamp = enc_param->timestamp;
			pObj->Queue[pObj->Rear].nxt_frm_type = 0;//enc_result->ucNxtPicType;
			pObj->Queue[pObj->Rear].base_qp = enc_param->base_qp;
			pObj->Queue[pObj->Rear].bs_size_1 = enc_param->bs_size_1;//enc_result->uiBSLen;
			pObj->Queue[pObj->Rear].frm_type = enc_param->frm_type;
			pObj->Queue[pObj->Rear].encode_time = 0;//uiHwEncTime;
			pObj->Queue[pObj->Rear].isKeyFrame = 0;
#ifdef __KERNEL__
			pObj->Queue[pObj->Rear].isRtosBS = FALSE;
#else
			pObj->Queue[pObj->Rear].isRtosBS = TRUE;
#endif
			pObj->Rear = (pObj->Rear + 1) % bsque_max[pathID];
			if (pObj->Front == pObj->Rear) { // Check Queue full
				pObj->bFull = TRUE;
			}
		}
	}
	SEM_SIGNAL(VDOENC_BUILTIN_BS_SEM_ID);
	return TRUE;
}

BOOL VdoEnc_Builtin_GetBS(UINT32 pathID, VDOENC_BUILTIN_BS_INFO *builtin_bs_info)
{
	VDOENC_BUILTIN_BSQ *pObj;

	SEM_WAIT(VDOENC_BUILTIN_BS_SEM_ID);

	pObj = &(gVdoEncBuiltinObj[pathID].bsQueue);

	DBG_IND("%s:(line%d) pathID=%d, BuiltinObj BeginTag=0x%x, EndTag=0x%x; bsQueue f=%d, r=%d, full=%d, BeginTag=0x%x, EndTag=0x%x\r\n", __func__, __LINE__, pathID, gVdoEncBuiltinObj[pathID].BeginTag, gVdoEncBuiltinObj[pathID].EndTag, pObj->Front, pObj->Rear, pObj->bFull, pObj->BeginTag, pObj->EndTag);
	if (pObj->Front > bsque_max[pathID] - 1 ||  pObj->Rear > bsque_max[pathID] - 1) {
		DBG_ERR("%s:(line%d) ERROR INDEX !!! pathID=%d, BuiltinObj BeginTag=0x%x, EndTag=0x%x; pObj->Queue 0x%lx, f=%d/%d, r=%d/%d, full=%d\r\n", __func__, __LINE__, pathID, gVdoEncBuiltinObj[pathID].BeginTag, gVdoEncBuiltinObj[pathID].EndTag, (unsigned long)pObj->Queue, pObj->Front, bsque_max[pathID], pObj->Rear, bsque_max[pathID], pObj->bFull);
		SEM_SIGNAL(VDOENC_BUILTIN_BS_SEM_ID);
		return FALSE;
	}

	if ((pObj->Front == pObj->Rear) && (pObj->bFull == FALSE)) {
		SEM_SIGNAL(VDOENC_BUILTIN_BS_SEM_ID);
		DBG_ERR("get bs queue empty!\r\n");
		return FALSE;
	} else {
		if (pObj->Queue[pObj->Front].isRtosBS == TRUE) {
			builtin_bs_info->Addr = nvtmpp_buitin_sys_pa2va(pObj->Queue[pObj->Front].Addr);
		} else {
			builtin_bs_info->Addr = pObj->Queue[pObj->Front].Addr;
		}
		builtin_bs_info->Size = pObj->Queue[pObj->Front].Size;
		builtin_bs_info->temproal_id = pObj->Queue[pObj->Front].temproal_id;
		builtin_bs_info->re_encode_en = pObj->Queue[pObj->Front].re_encode_en;
		builtin_bs_info->timestamp = pObj->Queue[pObj->Front].timestamp;
		builtin_bs_info->nxt_frm_type = pObj->Queue[pObj->Front].nxt_frm_type;
		builtin_bs_info->base_qp = pObj->Queue[pObj->Front].base_qp;
		builtin_bs_info->bs_size_1 = pObj->Queue[pObj->Front].bs_size_1;
		builtin_bs_info->frm_type = pObj->Queue[pObj->Front].frm_type;
		builtin_bs_info->encode_time = pObj->Queue[pObj->Front].encode_time;
		builtin_bs_info->isKeyFrame = pObj->Queue[pObj->Front].isKeyFrame;
		pObj->Front = (pObj->Front + 1) % bsque_max[pathID];

		if (pObj->Front == pObj->Rear) { // Check Queue full
			pObj->bFull = FALSE;
		}

		SEM_SIGNAL(VDOENC_BUILTIN_BS_SEM_ID);
		return TRUE;
	}
}

UINT32 VdoEnc_Builtin_HowManyInBSQ(UINT32 pathID)
{
	UINT32 front, rear, full, sq = 0;
	VDOENC_BUILTIN_BSQ *pObj;

	SEM_WAIT(VDOENC_BUILTIN_BS_SEM_ID);
	pObj = &(gVdoEncBuiltinObj[pathID].bsQueue);
	front = pObj->Front;
	rear = pObj->Rear;
	full = pObj->bFull;
	SEM_SIGNAL(VDOENC_BUILTIN_BS_SEM_ID);
	if (front < rear) {
		sq = rear - front;
	} else if (front > rear) {
		sq = bsque_max[pathID] - (front - rear);
	} else if (front == rear && full == TRUE) {
		sq = bsque_max[pathID];
	} else {
		sq = 0;
	}

	DBG_IND("%s:(line%d) sq=%d\r\n", __func__, __LINE__, sq);

	return sq;
}

BOOL VdoEnc_Builtin_GetEncVar(UINT32 pathID, void *kdrv_vdoenc_var)
{
	memcpy((H26XENC_VAR *)kdrv_vdoenc_var, &enc_var[pathID], sizeof(H26XENC_VAR));
	return TRUE;
}

UINT32 VdoEnc_Builtin_SetParam(UINT32 pathID, UINT32 Param, ULONG Value)
{
	switch (Param) {
	case BUILTIN_VDOENC_INIT_PARAM_DYNAMIC_CODEC:
		dynamic_codec[pathID] = Value;
		break;

	case BUILTIN_VDOENC_INIT_PARAM_DYNAMIC_BYTERATE:
		dynamic_rc_byterate[pathID] = Value;
		break;

	default:
		DBG_ERR("[VDOENCBUILTIN] Get invalid param = %d\r\n", Param);
		return -1;
	}
	return 0;
}

UINT32 VdoEnc_Builtin_GetParam(UINT32 pathID, UINT32 Param, VOID *pValue)
{
	UINT32 codectype = 0;
	codectype = gvdoenc_dtsi_param[pathID].codectype;

	switch (Param) {
	case BUILTIN_VDOENC_INIT_PARAM_ENC_EN:
		*(UINT32 *)pValue = gVdoEnc_en[pathID];
		break;

	case BUILTIN_VDOENC_INIT_PARAM_CODEC:
		*(UINT32 *)pValue = gvdoenc_dtsi_param[pathID].codectype;
		break;

	case BUILTIN_VDOENC_INIT_PARAM_DIRECT:
		*(UINT32 *)pValue =  0;
		break;

	case BUILTIN_VDOENC_INIT_PARAM_WIDTH:
		if (codectype == BUILTIN_VDOENC_H265) {
			*(UINT32 *)pValue = init_hevc_obj[pathID].uiDisplayWidth;
		} else if (codectype == BUILTIN_VDOENC_H264) {
			*(UINT32 *)pValue = init_avc_obj[pathID].uiDisplayWidth;
		} else if (codectype == BUILTIN_VDOENC_MJPEG) {
			*(UINT32 *)pValue = jpeg_width[pathID];
		}
		break;

	case BUILTIN_VDOENC_INIT_PARAM_HEIGHT:
		if (codectype == BUILTIN_VDOENC_H265) {
			*(UINT32 *)pValue = init_hevc_obj[pathID].uiHeight;
		} else if (codectype == BUILTIN_VDOENC_H264) {
			*(UINT32 *)pValue = init_avc_obj[pathID].uiHeight;
		} else if (codectype == BUILTIN_VDOENC_MJPEG) {
			*(UINT32 *)pValue = jpeg_height[pathID];
		}
		break;

	case BUILTIN_VDOENC_INIT_PARAM_FRAME_RATE:
		*(UINT32 *)pValue = gFrameRate[pathID] * 1000;
		break;

	case BUILTIN_VDOENC_INIT_PARAM_PROFILE:
		if (init_avc_obj[pathID].eProfile == 100) {
			*(UINT32 *)pValue = 2;
		}
		break;

	case BUILTIN_VDOENC_INIT_PARAM_LEVEL_IDC:
		if (codectype == BUILTIN_VDOENC_H265) {
			*(UINT32 *)pValue = init_hevc_obj[pathID].ucLevelIdc;
		} else if (codectype == BUILTIN_VDOENC_H264) {
			*(UINT32 *)pValue = init_avc_obj[pathID].ucLevelIdc;
		}
		break;

	case BUILTIN_VDOENC_INIT_PARAM_GOP_NUM:
		if (codectype == BUILTIN_VDOENC_H265) {
			*(UINT32 *)pValue = init_hevc_obj[pathID].uiGopNum;
		} else if (codectype == BUILTIN_VDOENC_H264) {
			*(UINT32 *)pValue = init_avc_obj[pathID].uiGopNum;
		}
		break;

	case BUILTIN_VDOENC_INIT_PARAM_LTR_INTERVAL:
		if (codectype == BUILTIN_VDOENC_H265) {
			*(UINT32 *)pValue = init_hevc_obj[pathID].uiLTRInterval;
		} else if (codectype == BUILTIN_VDOENC_H264) {
			*(UINT32 *)pValue = init_avc_obj[pathID].uiLTRInterval;
		}
		break;

	case BUILTIN_VDOENC_INIT_PARAM_LTR_PRE_REF:
		if (codectype == BUILTIN_VDOENC_H265) {
			*(UINT32 *)pValue = init_hevc_obj[pathID].uiLTRPreRef;
		} else if (codectype == BUILTIN_VDOENC_H264) {
			*(UINT32 *)pValue = init_avc_obj[pathID].bLTRPreRef;
		}
		break;

	case BUILTIN_VDOENC_INIT_PARAM_GRAY_EN:
		if (codectype == BUILTIN_VDOENC_H265) {
			*(UINT32 *)pValue = init_hevc_obj[pathID].bGrayEn;
		} else if (codectype == BUILTIN_VDOENC_H264) {
			*(UINT32 *)pValue = init_avc_obj[pathID].bGrayEn;
		}
		break;

	case BUILTIN_VDOENC_INIT_PARAM_SRC_OUT:
		if (codectype == BUILTIN_VDOENC_H265) {
			*(UINT32 *)pValue = 0;
		} else if (codectype == BUILTIN_VDOENC_H264) {
			*(UINT32 *)pValue = info_avc_obj[pathID].bSrcOutEn;
		}
		break;

	case BUILTIN_VDOENC_INIT_PARAM_SVC:
		if (codectype == BUILTIN_VDOENC_H265) {
			*(UINT32 *)pValue = init_hevc_obj[pathID].ucSVCLayer;
		} else if (codectype == BUILTIN_VDOENC_H264) {
			*(UINT32 *)pValue = init_avc_obj[pathID].ucSVCLayer;
		}
		break;

	case BUILTIN_VDOENC_INIT_PARAM_ENTROPY:
		*(UINT32 *)pValue = init_avc_obj[pathID].eEntropyMode;
		break;

	case BUILTIN_VDOENC_INIT_PARAM_BSQ_MAX:
		*(UINT32 *)pValue = bsque_max[pathID];
		break;

	case BUILTIN_VDOENC_INIT_PARAM_SEC:
		*(UINT32 *)pValue = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_SEC);
		break;

	case BUILTIN_VDOENC_INIT_PARAM_JPEG_QUALITY:
		*(UINT32 *)pValue = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_JPEG_QUALITY);
		break;

	case BUILTIN_VDOENC_INIT_PARAM_JPEG_FPS:
		*(UINT32 *)pValue = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_JPEG_FPS);
		break;

	case BUILTIN_VDOENC_INIT_PARAM_JPEG_MAX_MEM_SIZE:
		*(UINT32 *)pValue = jpeg_max_mem_size[pathID];
		break;

	case BUILTIN_VDOENC_INIT_PARAM_DYNAMIC_CODEC:
		*(UINT32 *)pValue = dynamic_codec[pathID];
		break;

	case BUILTIN_VDOENC_INIT_PARAM_BS_START:
		*(ULONG *)pValue = gBSStart[pathID];
		break;

	case BUILTIN_VDOENC_INIT_PARAM_BS_END:
		*(ULONG *)pValue = gBSEnd[pathID];
		break;

	case BUILTIN_VDOENC_INIT_PARAM_DYNAMIC_BYTERATE:
		*(UINT32 *)pValue = dynamic_rc_byterate[pathID];
		break;

	case BUILTIN_VDOENC_INIT_PARAM_RATE_CONTROL:
		memcpy((VDOENC_BUILTIN_RC_PARAM *)pValue, &gRcParam[pathID], sizeof(VDOENC_BUILTIN_RC_PARAM));
		break;

	default:
		DBG_ERR("[VDOENCBUILTIN] Get invalid param = %d\r\n", Param);
		return -1;
	}
	return 0;
}

UINT32 VdoEnc_Builtin_CheckBuiltinStop(UINT32 pathID)
{
	FLGPTN uiFlag;
	ER ret = E_OK;
	UINT32 i = 0;

	for (i = 0; i < BUILTIN_VDOENC_PATH_ID_MAX; i++) {
		if (gVdoEnc_en[i] == 1) {
			ret = vos_flag_wait_timeout(&uiFlag, FLG_ID_VDOENC_BUILTIN_CHECK[i], FLG_VDOENC_BUILTIN_DONE, TWF_ORW, vos_util_msec_to_tick(1000));
			if (ret != E_OK) {
				DBG_ERR("[%d] VdoEnc_Builtin_CheckBuiltinStop time out, pathID %d\r\n", i, pathID);
			}
		}
	}

	return 0;
}

int VdoEnc_builtin_get_dtsi_param(UINT32 pathID, BUILTIN_VDOENC_DTSI_PARAM param)
{
	UINT32 value = 0;
	int r = 0;

//	struct device_node* of_node;
	const char *path[BUILTIN_VDOENC_PATH_ID_MAX] = {
		"venc0",
		"venc1",
		"venc2",
		"venc3",
		"venc4",
		"venc5",
	};

//	of_node = of_get_child_by_name((struct device_node*)nodeoffset, path[pathID]);
//	if (of_node == NULL) {
//		//DBG_ERR("[%d] venc of_get_child_by_name is NULL\r\n", pathID);
//		return -1;
//	}

	switch (param) {
		case BUILTIN_VDOENC_DTSI_PARAM_ENC_EN:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "enable", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_CODECTYPE:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "codectype", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_SRC_VPRC_DEV:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "vprc_src_dev", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_SRC_VPRC_PATH:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "vprc_src_path", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_WIDTH:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "width", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_HEIGHT:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "height", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_INIT_BYTE_RATE:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "init_byte_rate", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_FRAMERATE:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "framerate", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_SEC:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "sec", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_GOP:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "gop", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_QP:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "qp", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_JPEG_QUALITY:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "jpeg_quality", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_JPEG_FPS:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "jpeg_fps", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_RC_MODE:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "rc", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_INIT_IQP:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "init_i_qp", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_MIN_IQP:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "min_i_qp", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_MAX_IQP:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "max_i_qp", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_INIT_PQP:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "init_p_qp", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_MIN_PQP:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "min_p_qp", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_MAX_PQP:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "max_p_qp", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_RC_BYTE_RATE:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "rc_byte_rate", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_STATIC_TIME:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "static_time", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_IP_WEIGHT:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "ip_weight", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_KEY_P_PERIOD:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "key_p_period", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_KP_WEIGHT:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "kp_weight", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_P2_WEIGHT:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "p2_weight", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_P3_WEIGHT:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "p3_weight", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_LT_WEIGHT:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "lt_weight", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_MOTION_AQ_STR:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "motion_aq_str", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_STILL_FRM_CND:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "still_frm_cnd", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_MOTION_RATIO_THD:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "motion_ratio_thd", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_I_PSNR_CND:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "i_psnr_cnd", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_P_PSNR_CND:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "p_psnr_cnd", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_KP_PSNR_CND:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "kp_psnr_cnd", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_CHANGE_POSITION:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "change_pos", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_SVC_LAYER:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "svc_layer", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_LTR_INTERVAL:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "ltr_interval", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_D2D:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "d2d", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_GDC:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "gdc", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_COLMV:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "colmv", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_QUALITY_LV:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "qualitylv", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_ISP_ID:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "isp_id", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_SRCOUT_SIZE:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "srcout_size", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_SVC_WEIGHT_MODE:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "svc_weight_mode", &value) != 0) {
					r = -1;
				}
			}
		break;

		case BUILTIN_VDOENC_DTSI_PARAM_ROTATE:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "rotate", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_SKIP_FRM_EN:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "skip_frm_en", &value) != 0) {
					r = -1;
				}
			}
			break;
		case BUILTIN_VDOENC_DTSI_PARAM_SKIP_FRM_TARGET_FR:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "skip_frm_target_fr", &value) != 0) {
					r = -1;
				}
			}
			break;
		case BUILTIN_VDOENC_DTSI_PARAM_SKIP_FRM_INPUT_CNT:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "skip_frm_input_cnt", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_YCC_EN:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "ycc_en", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_YCC_WIDTH:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "ycc_width", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_YCC_HEIGHT:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "ycc_height", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_YCC_YLOFF:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "ycc_yloff", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_YCC_CLOFF:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "ycc_cloff", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_FRC_SAMPLE_MODE:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "frc_sample_mode", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_FRC_RATE:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "frc_rate", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_FRC_FRAMEPERSEC:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "frc_framepersecond", &value) != 0) {
					r = -1;
				}
			}
			break;

		case BUILTIN_VDOENC_DTSI_PARAM_FRC_FRAMEPERSEC_NEW:
			if (nodeoffset_list[pathID]) {
				if (vdoenc_plat_read_dtsi(nodeoffset_list[pathID], "frc_framepersecond_new", &value) != 0) {
					r = -1;
				}
			}
			break;

		default:
		DBG_ERR("[VDOENCBUILTIN] Get invalid param = %d\r\n", param);
	}

	if (r == -1) {
		DBG_ERR("cannot find %s param_id %d\r\n", path[pathID], param);
	}

	return value;
}

void VdoEnc_builtin_set_rate_control(UINT32 pathID)
{
	UINT32 rc_mode = 0;

	rc_mode = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_RC_MODE);

	if (rc_mode == BUILTIN_VDOENC_RC_CBR) {
		H26XEncRCParam sRcParam = {0};
		UINT32 src_frame_rate = 0, encode_frame_rate = 0;

		src_frame_rate = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_SKIP_FRM_INPUT_CNT);
		encode_frame_rate = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_SKIP_FRM_TARGET_FR);
		gRcParam[pathID].uiEncId = sRcParam.uiEncId = pathID;
		gRcParam[pathID].uiRCMode = sRcParam.uiRCMode = H26X_RC_CBR;
		gRcParam[pathID].uiInitIQp = sRcParam.uiInitIQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_INIT_IQP);
		gRcParam[pathID].uiMinIQp = sRcParam.uiMinIQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MIN_IQP);
		gRcParam[pathID].uiMaxIQp = sRcParam.uiMaxIQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MAX_IQP);
		gRcParam[pathID].uiInitPQp = sRcParam.uiInitPQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_INIT_PQP);
		gRcParam[pathID].uiMinPQp = sRcParam.uiMinPQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MIN_PQP);
		gRcParam[pathID].uiMaxPQp = sRcParam.uiMaxPQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MAX_PQP);
		gRcParam[pathID].uiBitRate = sRcParam.uiBitRate = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_RC_BYTE_RATE)*8;
		if (src_frame_rate > 0 && encode_frame_rate > 0) {
			gRcParam[pathID].uiFrameRateBase = (VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_FRAMERATE)*1000) * src_frame_rate / encode_frame_rate;
			gRcParam[pathID].uiFrameRateIncr = 1000;
			gRcParam[pathID].uiGOP = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_GOP) * src_frame_rate / encode_frame_rate;
		}
		else {
			gRcParam[pathID].uiFrameRateBase = sRcParam.uiFrameRateBase = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_FRAMERATE)*1000;
			gRcParam[pathID].uiFrameRateIncr = sRcParam.uiFrameRateIncr = 1000;
			gRcParam[pathID].uiGOP = sRcParam.uiGOP = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_GOP);
		}
		gRcParam[pathID].uiRowLevelRCEnable = sRcParam.uiRowLevelRCEnable = 1;
		gRcParam[pathID].uiStaticTime = sRcParam.uiStaticTime = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_STATIC_TIME);
		gRcParam[pathID].iIPWeight = sRcParam.iIPWeight = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_IP_WEIGHT);
		gRcParam[pathID].uiKeyPPeriod = sRcParam.uiKeyPPeriod = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_KEY_P_PERIOD);
		gRcParam[pathID].iKPWeight = sRcParam.iKPWeight = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_KP_WEIGHT);
		gRcParam[pathID].iP2Weight = sRcParam.iP2Weight = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_P2_WEIGHT);
		gRcParam[pathID].iP3Weight = sRcParam.iP3Weight = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_P3_WEIGHT);
		gRcParam[pathID].iLTWeight = sRcParam.iLTWeight = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_LT_WEIGHT);
		gRcParam[pathID].iMotionAQStrength = sRcParam.iMotionAQStrength = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MOTION_AQ_STR);
		gRcParam[pathID].uiSvcBAMode = sRcParam.uiSvcBAMode = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_SVC_WEIGHT_MODE);

		h26XEnc_setRcInit(&enc_var[pathID], &sRcParam);
	} else if (rc_mode == BUILTIN_VDOENC_RC_EVBR) {
		H26XEncRCParam sRcParam = {0};

		gRcParam[pathID].uiEncId = sRcParam.uiEncId = pathID;
		gRcParam[pathID].uiRCMode = sRcParam.uiRCMode = H26X_RC_EVBR;
		gRcParam[pathID].uiInitIQp = sRcParam.uiInitIQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_INIT_IQP);
		gRcParam[pathID].uiMinIQp = sRcParam.uiMinIQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MIN_IQP);
		gRcParam[pathID].uiMaxIQp = sRcParam.uiMaxIQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MAX_IQP);
		gRcParam[pathID].uiInitPQp = sRcParam.uiInitPQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_INIT_PQP);
		gRcParam[pathID].uiMinPQp = sRcParam.uiMinPQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MIN_PQP);
		gRcParam[pathID].uiMaxPQp = sRcParam.uiMaxPQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MAX_PQP);
		gRcParam[pathID].uiBitRate = sRcParam.uiBitRate = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_RC_BYTE_RATE)*8;
		gRcParam[pathID].uiFrameRateBase = sRcParam.uiFrameRateBase = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_FRAMERATE)*1000;
		gRcParam[pathID].uiFrameRateIncr = sRcParam.uiFrameRateIncr = 1000;
		gRcParam[pathID].uiGOP = sRcParam.uiGOP = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_GOP);
		gRcParam[pathID].uiRowLevelRCEnable = sRcParam.uiRowLevelRCEnable = 1;
		gRcParam[pathID].uiStaticTime = sRcParam.uiStaticTime = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_STATIC_TIME);
		gRcParam[pathID].iIPWeight = sRcParam.iIPWeight = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_IP_WEIGHT);
		gRcParam[pathID].iKPWeight = sRcParam.iKPWeight = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_KP_WEIGHT);
		gRcParam[pathID].uiKeyPPeriod = sRcParam.uiKeyPPeriod = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_KEY_P_PERIOD);
		gRcParam[pathID].iMotionAQStrength = sRcParam.iMotionAQStrength = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MOTION_AQ_STR);
		gRcParam[pathID].uiStillFrameCnd = sRcParam.uiStillFrameCnd = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_STILL_FRM_CND);
		gRcParam[pathID].uiMotionRatioThd = sRcParam.uiMotionRatioThd = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MOTION_RATIO_THD);
		gRcParam[pathID].uiIPsnrCnd = sRcParam.uiIPsnrCnd = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_I_PSNR_CND);
		gRcParam[pathID].uiPPsnrCnd = sRcParam.uiPPsnrCnd = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_P_PSNR_CND);
		gRcParam[pathID].uiKeyPPsnrCnd = sRcParam.uiKeyPPsnrCnd = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_KP_PSNR_CND);
		gRcParam[pathID].uiMinStillIQp = sRcParam.uiMinStillIQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MIN_PQP);
		gRcParam[pathID].iP2Weight = sRcParam.iP2Weight = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_P2_WEIGHT);
		gRcParam[pathID].iP3Weight = sRcParam.iP3Weight = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_P3_WEIGHT);
		gRcParam[pathID].iLTWeight = sRcParam.iLTWeight = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_LT_WEIGHT);
		gRcParam[pathID].uiSvcBAMode = sRcParam.uiSvcBAMode = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_SVC_WEIGHT_MODE);

		h26XEnc_setRcInit(&enc_var[pathID], &sRcParam);
	} else if (rc_mode == BUILTIN_VDOENC_RC_VBR || rc_mode == BUILTIN_VDOENC_RC_VBR2) {
		H26XEncRCParam sRcParam = {0};

		gRcParam[pathID].uiEncId = sRcParam.uiEncId = pathID;
		if (rc_mode == BUILTIN_VDOENC_RC_VBR2) {
			gRcParam[pathID].uiRCMode = sRcParam.uiRCMode = H26X_RC_VBR2;
		} else {
			gRcParam[pathID].uiRCMode = sRcParam.uiRCMode = H26X_RC_VBR;
		}
		gRcParam[pathID].uiInitIQp = sRcParam.uiInitIQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_INIT_IQP);
		gRcParam[pathID].uiMinIQp = sRcParam.uiMinIQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MIN_IQP);
		gRcParam[pathID].uiMaxIQp = sRcParam.uiMaxIQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MAX_IQP);
		gRcParam[pathID].uiInitPQp = sRcParam.uiInitPQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_INIT_PQP);
		gRcParam[pathID].uiMinPQp = sRcParam.uiMinPQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MIN_PQP);
		gRcParam[pathID].uiMaxPQp = sRcParam.uiMaxPQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MAX_PQP);
		gRcParam[pathID].uiBitRate = sRcParam.uiBitRate = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_RC_BYTE_RATE)*8;
		gRcParam[pathID].uiFrameRateBase = sRcParam.uiFrameRateBase = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_FRAMERATE)*1000;
		gRcParam[pathID].uiFrameRateIncr = sRcParam.uiFrameRateIncr = 1000;
		gRcParam[pathID].uiGOP = sRcParam.uiGOP = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_GOP);
		gRcParam[pathID].uiRowLevelRCEnable = sRcParam.uiRowLevelRCEnable = 1;
		gRcParam[pathID].uiStaticTime = sRcParam.uiStaticTime = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_STATIC_TIME);
		gRcParam[pathID].iIPWeight = sRcParam.iIPWeight = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_IP_WEIGHT);
		gRcParam[pathID].uiChangePos = sRcParam.uiChangePos = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_CHANGE_POSITION);
		gRcParam[pathID].uiKeyPPeriod = sRcParam.uiKeyPPeriod = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_KEY_P_PERIOD);
		gRcParam[pathID].iKPWeight = sRcParam.iKPWeight = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_KP_WEIGHT);
		gRcParam[pathID].iP2Weight = sRcParam.iP2Weight = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_P2_WEIGHT);
		gRcParam[pathID].iP3Weight = sRcParam.iP3Weight = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_P3_WEIGHT);
		gRcParam[pathID].iLTWeight = sRcParam.iLTWeight = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_LT_WEIGHT);
		gRcParam[pathID].iMotionAQStrength = sRcParam.iMotionAQStrength = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MOTION_AQ_STR);
		gRcParam[pathID].uiSvcBAMode = sRcParam.uiSvcBAMode = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_SVC_WEIGHT_MODE);

		h26XEnc_setRcInit(&enc_var[pathID], &sRcParam);
	} else if (rc_mode == BUILTIN_VDOENC_RC_FIXQP) {
		H26XEncRCParam sRcParam = {0};

		gRcParam[pathID].uiEncId = sRcParam.uiEncId = pathID;
		gRcParam[pathID].uiRCMode = sRcParam.uiRCMode = H26X_RC_FixQp;
		gRcParam[pathID].uiMinIQp = sRcParam.uiMinIQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_INIT_IQP);
		gRcParam[pathID].uiFixIQp = sRcParam.uiFixIQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_INIT_IQP);
		gRcParam[pathID].uiMaxIQp = sRcParam.uiMaxIQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_INIT_IQP);
		gRcParam[pathID].uiMinPQp = sRcParam.uiMinPQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_INIT_PQP);
		gRcParam[pathID].uiFixPQp = sRcParam.uiFixPQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_INIT_PQP);
		gRcParam[pathID].uiMaxPQp = sRcParam.uiMaxPQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_INIT_PQP);
		gRcParam[pathID].uiFrameRateBase = sRcParam.uiFrameRateBase = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_FRAMERATE)*1000;
		gRcParam[pathID].uiFrameRateIncr = sRcParam.uiFrameRateIncr = 1000;

		h26XEnc_setRcInit(&enc_var[pathID], &sRcParam);
	}


	return;
}

int H264Enc_builtin_init(UINT32 pathID, VDOENC_BUILTIN_INIT_INFO *p_info)
{
	UINT32 width = 0, height = 0;
	UINT32 svc_layer = 0, ltr_interval = 0, quality_lv = 0, d2d = 0, gdc = 0, colmv = 0, ltr_saving_buf_bs_size = 0;
	UINT32 srcout_size = 0, rotate = 0;
	ULONG venc_ctrl_0[2] = {0};

	/* H264 codec buffer */
	H26XEncMeminfo memInfo = {0};
	uintptr_t codec_mem_addr = 0;
	UINT32 codec_mem_size = 0;
	UINT32 sizePerSec = 0, uiMinIRatio = 150, uiMinPRatio = 100;

	VDOENC_BUILTIN_OBJ *p_obj = &gVdoEncBuiltinObj[pathID];

	if (bPreInit == 0) {
		memset(&gVdoEncBuiltinObj[pathID], 0, sizeof(VDOENC_BUILTIN_OBJ));
		memset(&enc_var[pathID], 0, sizeof(H26XENC_VAR));
	}
	memset(&init_avc_obj[pathID], 0, sizeof(H264ENC_INIT));
	memset(&info_avc_obj[pathID], 0, sizeof(H264ENC_INFO));

	// frc
	gVdoEncBuiltinObj[pathID].frc.sample_mode = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_FRC_SAMPLE_MODE);
	gVdoEncBuiltinObj[pathID].frc.rate = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_FRC_RATE);
	gVdoEncBuiltinObj[pathID].frc.frm_counter = 0;
	gVdoEncBuiltinObj[pathID].frc.rate_counter = 0;
	gVdoEncBuiltinObj[pathID].frc.output_counter = 0;
	gVdoEncBuiltinObj[pathID].frc.framepersecond = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_FRC_FRAMEPERSEC);
	gVdoEncBuiltinObj[pathID].frc.framepersecond_new = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_FRC_FRAMEPERSEC_NEW);

	width = p_info[pathID].width;
	height = p_info[pathID].height;
	venc_ctrl_0[0] = p_info[pathID].max_blk_addr;
	venc_ctrl_0[1] = p_info[pathID].max_blk_size;
	if (width == 0 || height == 0) {
		DBG_ERR("H264Enc_builtin_init error size width %d, height %d\r\n", (int)width, (int)height);
		return -1;
	}

	svc_layer = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_SVC_LAYER);
	ltr_interval = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_LTR_INTERVAL);
	quality_lv = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_QUALITY_LV);
	d2d = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_D2D);
	gdc = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_GDC);
	colmv = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_COLMV);
	srcout_size = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_SRCOUT_SIZE);
	rotate = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_ROTATE);
#if defined(CONFIG_NVT_SMALL_HDAL)
	ltr_saving_buf_bs_size = ((width * height * 3 /2) * 20/100);
#else
	ltr_saving_buf_bs_size = 0;
#endif


	// get H264 codec buffer size
	{
		memInfo.uiWidth = ALIGN_CEIL_16(((rotate == 1) || (rotate == 2)) ? height : width);
		memInfo.uiHeight = ((rotate == 1) || (rotate == 2)) ? width : height;
		memInfo.ucSVCLayer = svc_layer;
		memInfo.uiLTRInterval = ltr_interval;
		memInfo.ucQualityLevel = quality_lv;
		memInfo.bTileEn= 0;
		memInfo.bD2dEn= d2d;
		memInfo.bGdcEn= gdc;
		memInfo.bColMvEn = colmv;
		memInfo.bCommReconFrmBuf = 0;
		if(memInfo.uiLTRInterval > 0){
			memInfo.uiLtrSavingBufBsSize = ltr_saving_buf_bs_size;
		}else{
			memInfo.uiLtrSavingBufBsSize = 0;
		}

		codec_mem_addr = venc_ctrl_0[0];
		codec_mem_size = h264Enc_queryMemSize(&memInfo);
		if (codec_mem_size == 0) {
			DBG_ERR("H264Enc query error MemSize 0\r\n");
			return -1;
		}
	}

	p_obj->venc_param.codec_mem_addr = codec_mem_addr;
	p_obj->venc_param.codec_mem_size = codec_mem_size;
	p_obj->venc_param.bs_start_addr = p_obj->venc_param.codec_mem_addr + ALIGN_CEIL_4(p_obj->venc_param.codec_mem_size);
	p_obj->venc_param.bs_end_addr = venc_ctrl_0[0] + venc_ctrl_0[1] - VDOENC_MD_MAP_MAX_SIZE - VDOENC_BUF_RESERVED_BYTES - srcout_size;
	gBSStart[pathID] = p_obj->venc_param.bs_start_addr;
	gBSEnd[pathID] = p_obj->venc_param.bs_end_addr;
	gFrameRate[pathID] = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_FRAMERATE);
	gSec[pathID] = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_SEC);
	init_avc_obj[pathID].uiFrmRate = gFrameRate[pathID] * 1000;
	bsque_max[pathID] = gFrameRate[pathID] * gSec[pathID];
	if (!VdoEnc_Builtin_AllocQueMem(pathID, bsque_max[pathID], VDOENC_BUILTIN_YUVQ_MAX, bPreInit)){
		DBG_ERR("VdoEnc Builtin AllocQueMem fail !!!\r\n");
		return -1;
	}

	if (pre_enc_num[pathID] > 0) {
		VDOENC_BUILTIN_BSQ *pBSQ;

		pBSQ = &(gVdoEncBuiltinObj[pathID].bsQueue);

		if (pBSQ->bFull) {
			p_obj->venc_param.bs_addr_1 = nvtmpp_buitin_sys_pa2va(pBSQ->Queue[bsque_max[pathID] - 1].Addr) + ALIGN_CEIL_4(pBSQ->Queue[bsque_max[pathID] - 1].bs_size_1);
			p_obj->venc_param.bs_size_1 = p_obj->venc_param.bs_end_addr - p_obj->venc_param.bs_addr_1;
			DBG_IND("pre init H264 bs_rear (0x%lx, 0x%x), bs_curr (0x%lx, 0x%x)\r\n",
				pBSQ->Queue[bsque_max[pathID] - 1].Addr,
				pBSQ->Queue[bsque_max[pathID] - 1].Size,
				p_obj->venc_param.bs_addr_1,
				p_obj->venc_param.bs_size_1);
		} else {
			if (pBSQ->Rear - 1 >= bsque_max[pathID]) {
				DBG_ERR("H264 init error bsq rear %d\r\n", pBSQ->Rear - 1);
				return -1;
			}
			p_obj->venc_param.bs_addr_1 = nvtmpp_buitin_sys_pa2va(pBSQ->Queue[pBSQ->Rear - 1].Addr) + ALIGN_CEIL_4(pBSQ->Queue[pBSQ->Rear - 1].bs_size_1);
			p_obj->venc_param.bs_size_1 = p_obj->venc_param.bs_end_addr - p_obj->venc_param.bs_addr_1;
			DBG_IND("pre init H264 bs_rear (0x%lx, 0x%x), bs_curr (0x%lx, 0x%x)\r\n",
				pBSQ->Queue[pBSQ->Rear - 1].Addr,
				pBSQ->Queue[pBSQ->Rear - 1].Size,
				p_obj->venc_param.bs_addr_1,
				p_obj->venc_param.bs_size_1);
		}
	} else {
		p_obj->venc_param.bs_addr_1 = p_obj->venc_param.bs_start_addr;
		p_obj->venc_param.bs_size_1 = p_obj->venc_param.bs_end_addr - p_obj->venc_param.bs_start_addr;
	}
	sizePerSec = _VdoEnc_builtin_GetBytesPerSecond(width, height);
#ifdef __KERNEL__
	p_obj->venc_param.bs_min_i_size = (UINT32)div_u64((UINT64)sizePerSec * (UINT64)uiMinIRatio, 100);
	p_obj->venc_param.bs_min_p_size = (UINT32)div_u64((UINT64)sizePerSec * (UINT64)uiMinPRatio, 100);
#else
	p_obj->venc_param.bs_min_i_size = ((UINT64)sizePerSec * (UINT64)uiMinIRatio / 100);
	p_obj->venc_param.bs_min_p_size = ((UINT64)sizePerSec * (UINT64)uiMinPRatio / 100);
#endif

	DBG_IND("codec_addr 0x%lx, codec_size 0x%x, bs_start_addr 0x%lx, bs_end_addr 0x%lx, bs_addr_1 0x%lx, bs_size_1 0x%x\r\n",
		(unsigned long)p_obj->venc_param.codec_mem_addr,
		(unsigned int)p_obj->venc_param.codec_mem_size,
		(unsigned long)p_obj->venc_param.bs_start_addr,
		(unsigned long)p_obj->venc_param.bs_end_addr,
		(unsigned long)p_obj->venc_param.bs_addr_1,
		(unsigned int)p_obj->venc_param.bs_size_1);
	// Init encode engine
	{
		init_avc_obj[pathID].uiDisplayWidth = (rotate == 1 || rotate == 2) ? height : width;
		init_avc_obj[pathID].uiWidth  = ALIGN_CEIL_16(init_avc_obj[pathID].uiDisplayWidth);
		init_avc_obj[pathID].uiHeight = (rotate == 1 || rotate == 2) ? width : height;
		init_avc_obj[pathID].uiEncBufAddr = codec_mem_addr;
		init_avc_obj[pathID].uiEncBufSize = codec_mem_size;
		init_avc_obj[pathID].uiGopNum = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_GOP);
		init_avc_obj[pathID].ucDisLFIdc = 0 & 0x3; // remove 3'b100 (TileMode), which is H265 only
		init_avc_obj[pathID].cDBAlpha = 0;
		init_avc_obj[pathID].cDBBeta = 0;
		init_avc_obj[pathID].cChrmQPIdx = 0;
		init_avc_obj[pathID].cSecChrmQPIdx = 0;
		init_avc_obj[pathID].ucSVCLayer = svc_layer;
		init_avc_obj[pathID].uiLTRInterval = ltr_interval;
		init_avc_obj[pathID].bLTRPreRef = 0;
		init_avc_obj[pathID].uiBitRate = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_INIT_BYTE_RATE)*8;
		init_avc_obj[pathID].ucIQP = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_INIT_IQP);
		init_avc_obj[pathID].ucPQP = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_INIT_PQP);
		init_avc_obj[pathID].ucMaxIQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MAX_IQP);
		init_avc_obj[pathID].ucMinIQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MIN_IQP);
		init_avc_obj[pathID].ucMaxPQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MAX_PQP);
		init_avc_obj[pathID].ucMinPQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MIN_PQP);
		if (VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_RC_MODE) == BUILTIN_VDOENC_RC_FIXQP) {
			init_avc_obj[pathID].iIPWeight = 0;
			init_avc_obj[pathID].uiStaticTime = 0;
		} else {
			init_avc_obj[pathID].iIPWeight = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_IP_WEIGHT);
			init_avc_obj[pathID].uiStaticTime = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_STATIC_TIME);
		}
		init_avc_obj[pathID].bFBCEn = 0;
		init_avc_obj[pathID].bGrayEn = 0;
		init_avc_obj[pathID].bFastSearchEn = 0;//pinfo->fast_search;	// nt98520 not support search range set to 1 : big range //
		init_avc_obj[pathID].bHwPaddingEn = 1;
		init_avc_obj[pathID].ucRotate = rotate;
		init_avc_obj[pathID].bD2dEn = d2d;
		init_avc_obj[pathID].bColMvEn = colmv;

		init_avc_obj[pathID].eProfile = PROFILE_HIGH;
		init_avc_obj[pathID].eEntropyMode = 1; //1: KDRV_VDOENC_CABAC, 0: KDRV_VDOENC_CAVLC
		if (init_avc_obj[pathID].uiWidth > 1920 && init_avc_obj[pathID].uiHeight > 1080) {
			init_avc_obj[pathID].ucLevelIdc = 51; //LEVEL_51
		} else {
			init_avc_obj[pathID].ucLevelIdc = 41; //LEVEL_51
		}
		init_avc_obj[pathID].bTrans8x8En = 1;
		init_avc_obj[pathID].bForwardRecChrmEn = 0;

		init_avc_obj[pathID].bVUIEn = 0;
		init_avc_obj[pathID].usSarWidth = width;
		init_avc_obj[pathID].usSarHeight = height;
		init_avc_obj[pathID].ucMatrixCoef = 0;
		init_avc_obj[pathID].ucTransferCharacteristics = 0;
		init_avc_obj[pathID].ucColourPrimaries = 0;
		init_avc_obj[pathID].ucVideoFormat = 0;
		init_avc_obj[pathID].ucColorRange = 0;
		init_avc_obj[pathID].bTimeingPresentFlag = 0;
		init_avc_obj[pathID].bRecBufComm = 0;
		init_avc_obj[pathID].uiRecBufAddr[0] = 0;
		init_avc_obj[pathID].uiRecBufAddr[1] = 0;
		init_avc_obj[pathID].uiRecBufAddr[2] = 0;
    	if(init_avc_obj[pathID].uiLTRInterval > 0) {
			init_avc_obj[pathID].uiLtrSavingBufBsSize = ltr_saving_buf_bs_size;
		} else {
			init_avc_obj[pathID].uiLtrSavingBufBsSize = 0;
		}

		h264Enc_initEncoder(&init_avc_obj[pathID], &enc_var[pathID], bPreInit);

		{
			uintptr_t desc_addr = 0;
			UINT32    desc_size = 0;
			UINT8     *ptr;

			h264Enc_getSeqHdr(&enc_var[pathID], &desc_addr, &desc_size);
			ptr = (UINT8 *)desc_addr;
			if (*(ptr+0) != 0x00 || *(ptr+1) != 0x00 || *(ptr+2) != 0x00 || *(ptr+3) != 0x01 || *(ptr+4) != 0x67) {
				DBG_ERR("H264 error descriptor\r\n");
				return -1;
			}
		}
	}

	return 0;
}

void H264Enc_builtin_encodeOne(UINT32 pathID, BOOL enIntFirst)
{
	UINT32 interrupt = 0;
	BOOL is_keyfrm = 0;
	BOOL enc_err = 0;
	H26XEncResultCfg sResult = {0};
	VDOENC_BUILTIN_OBJ *p_obj = &gVdoEncBuiltinObj[pathID];
	VDOENC_BUILTIN_YUV_INFO yuv_info = {0};
	unsigned long flags = 0;
	UINT32 trig_enc = 0;
//#if defined(__FREERTOS)
//	static BOOL bStopRTOSEnc = FALSE;

//	if (bStopRTOSEnc == TRUE) {
//		return;
//	}
//#endif

#ifdef __KERNEL__
	if (enIntFirst) {
		goto EN_INT_FIRST_H264;
	}
#endif

	if (_VdoEnc_Builtin_HowManyInYUVQ(pathID) <= 0) {
		return;
	}

	if (_VdoEnc_Builtin_GetYuv(pathID, &yuv_info) == FALSE) {
		return;
	}

	if (p_obj->bsQueue.bFull) {
		//DBG_DUMP("Bs Queue Full, Not Encode Further\r\n");
		if (yuv_info.release_flag) {
			if (yuv_info.enable) {
				DBG_IND("%s_1 0x%08x\r\n",__func__,yuv_info.y_addr);
				nvtmpp_unlock_fastboot_blk(yuv_info.y_addr);
			}
			yuv_unlock_cnt[pathID]++;
			if (yuv_unlock_cnt[pathID] == 2) {
				builtin_stop_encode[pathID] = TRUE;
				set_flg(FLG_ID_VDOENC_BUILTIN_CHECK[pathID], FLG_VDOENC_BUILTIN_DONE);
			}
		}
		return;
	}

	info_avc_obj[pathID].ePicType = h264Enc_getNxtPicType(&enc_var[pathID]);
	info_avc_obj[pathID].bGetSeqHdrEn = IS_ISLICE(info_avc_obj[pathID].ePicType);
	is_keyfrm = IS_ISLICE(info_avc_obj[pathID].ePicType);
	if (p_obj->venc_param.bs_end_addr - p_obj->venc_param.bs_addr_1 < (IS_ISLICE(info_avc_obj[pathID].ePicType) ? p_obj->venc_param.bs_min_i_size : p_obj->venc_param.bs_min_p_size)) {
		//DBG_DUMP("Bs Buffer Full, Not Encode Further\r\n");
		if (yuv_info.release_flag) {
			if (yuv_info.enable) {
				DBG_IND("%s_2 0x%08x\r\n",__func__,yuv_info.y_addr);
				nvtmpp_unlock_fastboot_blk(yuv_info.y_addr);
			}
			yuv_unlock_cnt[pathID]++;
			if (yuv_unlock_cnt[pathID] == 2) {
				builtin_stop_encode[pathID] = TRUE;
				set_flg(FLG_ID_VDOENC_BUILTIN_CHECK[pathID], FLG_VDOENC_BUILTIN_DONE);
			}
		}
		return;
	}

#if TNR_ENABLE
	{
		H26XEncTnrCfg stCfg = {0};
		_VdoEnc_builtin_3DNR_Internal(pathID, (UINT32)&stCfg);
		h26XEnc_setTnrCfg(&enc_var[pathID], &stCfg);
	}
#endif
#if SPN_ENABLE
	{
		VDOENC_BUILTIN_H26XENC_SPN pinfo;
		H26XEncSpnCfg stCfg = {0};
		int i;
		_VdoEnc_builtin_PostSharpen_Internal(pathID, (ULONG)&pinfo);
		stCfg.bEnable = pinfo.bEnable;
		stCfg.ucConEng = pinfo.ucConEng;
		stCfg.usSlopConEng = pinfo.usSlopConEng;
		stCfg.ucBHC = pinfo.ucBHC;
		stCfg.ucDHC = pinfo.ucDHC;
		stCfg.ucEWT = pinfo.ucEWT;
		stCfg.ucEWG = pinfo.ucEWG;
		//stCfg.ucESS[0] = pinfo.ucEdgeSharpStr1;
		stCfg.ucESS = pinfo.ucEdgeSharpStr1;
		stCfg.ucCT = pinfo.ucCT;
		stCfg.ucNL = pinfo.ucNL;
		stCfg.ucBIG = pinfo.ucBIG;
		stCfg.usFlatTh = pinfo.usFlatTh;
		stCfg.usEdgeTh = pinfo.usEdgeTh;
		stCfg.ucEdgeStr = pinfo.ucEdgeStr;
		stCfg.ucTransitionStr = pinfo.ucTransitionStr;
		stCfg.ucMotionStr = pinfo.ucMotionStr;
		stCfg.ucStaticStr = pinfo.ucStaticStr;
		stCfg.ucFlatStr = pinfo.ucFlatStr;
		for(i=0;i<17;i++)
			stCfg.ucNC[i] = pinfo.ucNC[i];
		for(i=0;i<9;i++)
			stCfg.usEWG[i] = pinfo.usEWG[i];
		stCfg.bShowSharpInfo = pinfo.bShowSharpInfo;
		h26XEnc_setSpnCfg(&enc_var[pathID], &stCfg);
	}
#endif

	// set encode param to H264ENC_INFO
	if (yuv_info.enable) {
		// do frc
		trig_enc = _vdoenc_builtin_frc_is_select(pathID, &(p_obj->frc));

		if (builtin_stop_encode[pathID]) {
			return;
		}
		if (trig_enc) {
		if (gvdoenc_dtsi_param[pathID].skip_frm_en) {
			UINT32 uiSkipFrmTargetFr = gvdoenc_dtsi_param[pathID].skip_frm_target_fr;
			UINT32 uiSkipFrmInputCnt = gvdoenc_dtsi_param[pathID].skip_frm_input_cnt;
			UINT32 ratio = uiSkipFrmTargetFr / uiSkipFrmInputCnt;
			if (IS_ISLICE(info_avc_obj[pathID].ePicType)) {
				SkipModeFrameIdx[pathID] = 0;
			}
			if (SkipModeFrameIdx[pathID] % ratio == 0 || IS_ISLICE(info_avc_obj[pathID].ePicType)) {
				info_avc_obj[pathID].bSkipFrmEn = FALSE;
			} else {
				info_avc_obj[pathID].bSkipFrmEn = TRUE;
			}
		} else {
			info_avc_obj[pathID].bSkipFrmEn = FALSE;
		}

		info_avc_obj[pathID].uiSrcYAddr = yuv_info.y_addr; //0: y addr, 1: uv addr
		info_avc_obj[pathID].uiSrcCAddr = yuv_info.c_addr; //0: y addr, 1: uv addr
		info_avc_obj[pathID].uiSrcYLineOffset = yuv_info.y_line_offset; //0: y addr, 1: uv addr;
		info_avc_obj[pathID].uiSrcCLineOffset = yuv_info.c_line_offset;
		info_avc_obj[pathID].bSrcOutEn = 0;
#ifndef VDOCDC_EMU
		info_avc_obj[pathID].SdeCfg.bEnable = gvdoenc_dtsi_param[pathID].ycc_en;
		info_avc_obj[pathID].SdeCfg.uiWidth = gvdoenc_dtsi_param[pathID].ycc_width;
		info_avc_obj[pathID].SdeCfg.uiHeight = gvdoenc_dtsi_param[pathID].ycc_height;
		info_avc_obj[pathID].SdeCfg.uiYLofst = gvdoenc_dtsi_param[pathID].ycc_yloff;
		info_avc_obj[pathID].SdeCfg.uiCLofst = gvdoenc_dtsi_param[pathID].ycc_cloff;
#endif
		info_avc_obj[pathID].uiBsOutBufAddr = p_obj->venc_param.bs_addr_1;
		info_avc_obj[pathID].uiBsOutBufSize = p_obj->venc_param.bs_end_addr - p_obj->venc_param.bs_addr_1;
//		info_obj.uiNalLenOutAddr = nalu_len_addr; //maybe no need to set

		if (fastboot_osg_setup_h26x_stamp(&enc_var[pathID], pathID, &yuv_info)){
			DBG_ERR("fail to setup h26x stamp for encoding path(%u)\n", pathID);
			return;
		}

		// encode picture
		h264Enc_prepareOnePicture(&info_avc_obj[pathID], &enc_var[pathID]);

		{
			flags = VdoEnc_Builtin_spin_lock();

			if (H26X_ENC_MODE) {
				h26x_setEncDirectRegSet(0, h26xEnc_getVaAPBAddr(&enc_var[pathID]));
				if (disable_h26x_int == FALSE) {
					h26x_setIntEn(0, H26X_FINISH_INT | H26X_BSDMA_INT | H26X_ERR_INT | H26X_FRAME_TIME_OUT_INT | H26X_BSOUT_INT | H26X_FBC_ERR_INT | H26X_SWRST_FINISH_INT | H26X_DMACH_DIS_INT);
				}
			} else {
				h26x_setEncLLRegSet(0, 1, h26x_getPhyAddr(h26xEnc_getVaLLCAddr(&enc_var[pathID])));
			}
			#if 0 //TODO
			if (h26x_getRtosFastboot() == 0) {
				h26x_setLock(enc_var[pathID].uiChipIdx);
				h26x_enableClk(enc_var[pathID].uiChipIdx);
			}
			#endif

#if 0 //TBD
			if (h26x_getPatchCount(0) == 0) {
				//h26x_setUnitChecksum(0, 0xc0000000);
				h26x_start(0);
				h26x_waitINT(0);
				h26x_module_reset(0);
				h26x_setEncDirectRegSet(0, h26xEnc_getVaAPBAddr(&enc_var[pathID]));
				h26x_setPatchCount(0, 1);
			}
#endif

			h26x_start(0);
			bH26xStart = TRUE;

			// notify ipp venc start to encode
			if (vdoenc_start_enc_cb) {
				vdoenc_start_enc_cb(&yuv_info, 0);
			}

			if (disable_h26x_int == TRUE) {
				en_lnx_h26x_int_fisrt = 1 << 16 | pathID;
				VdoEnc_Builtin_spin_unlock(flags);
//#if defined(__FREERTOS)
//				bStopRTOSEnc = TRUE;
//#endif
				return;
			}

			VdoEnc_Builtin_spin_unlock(flags);

#ifdef __KERNEL__
EN_INT_FIRST_H264:
			if (enIntFirst) {
				H26XENC_VAR *pVar = &enc_var[pathID];
				H26XCOMN_CTX *pComnCtx = (H26XCOMN_CTX *)pVar->pComnCtx;
				DBG_IND("uiAPBAddr 0x%x, uiSPFrmBsOutAddr (0x%x, 0x%x), uiBsOutAddr (0x%x, 0x%x)\r\n",
					pComnCtx->stVaAddr.uiAPBAddr,
					pComnCtx->stVaAddr.uiSPFrmBsOutAddr, nvtmpp_buitin_sys_pa2va(pComnCtx->stVaAddr.uiSPFrmBsOutAddr),
					pComnCtx->stVaAddr.uiBsOutAddr, nvtmpp_buitin_sys_pa2va(pComnCtx->stVaAddr.uiBsOutAddr));
				pComnCtx->stVaAddr.uiBsOutAddr = nvtmpp_buitin_sys_pa2va(pComnCtx->stVaAddr.uiBsOutAddr);
				h26x_setIntEn(0, H26X_FINISH_INT | H26X_BSDMA_INT | H26X_ERR_INT | H26X_TIME_OUT_INT | H26X_FRAME_TIME_OUT_INT | H26X_BSOUT_INT | H26X_FBC_ERR_INT | H26X_SRC_DECMP_ERR_INT | H26X_SWRST_FINISH_INT | H26X_SRC_D2D_OV_INT | H26X_DMACH_DIS_INT);
			}
#endif
			interrupt = h26x_waitINT(0);

			if ((interrupt & h26x_getIntEn(0)) != 0x00000001)  {
				enc_err = 1;
				//h26x_resetHW(enc_var[pathID].uiChipIdx);
				h26x_module_reset(enc_var[pathID].uiChipIdx);

				DBG_ERR("encode error interrupt(0x%08x)\r\n", (int)interrupt);

				return;
				//h26x_prtReg(enc_var[pathID].uiChipIdx);
				//h26x_getDebug(enc_var[pathID].uiChipIdx);
				//vtrc_prtReg(enc_var[pathID].uiChipIdx);
			}

			h264Enc_getResult(&enc_var[pathID], H26X_ENC_MODE, &sResult, interrupt);
			#if 0 //TODO
			if (h26x_getRtosFastboot() == 0) {
				h26x_disableClk(enc_var[pathID].uiChipIdx);
				h26x_setUnLock(enc_var[pathID].uiChipIdx);
			}
			#endif
#if 0
			{
				UINT8 *ptr;
				ptr = (UINT8 *)p_obj->venc_param.bs_addr_1;
				DBG_DUMP("[%d] H264 BSAddr 0x%lx, BSSize 0x%x\r\n", pathID, p_obj->venc_param.bs_addr_1, sResult.uiBSLen);
				DBG_DUMP("%02X %02X %02X %02X %02X %02X %02X %02X\r\n",
					*(ptr+0), *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5), *(ptr+6), *(ptr+7));
				DBG_DUMP("========================\r\n");
			}
#endif
			p_obj->venc_param.interrupt = interrupt;
			p_obj->venc_param.timestamp = yuv_info.timestamp;
			p_obj->venc_param.bs_size_1 = sResult.uiBSLen;
		}

		if (enc_err == 0) {
#ifdef __KERNEL__
			VdoEnc_Builtin_PutBS(pathID, is_keyfrm, &(p_obj->venc_param), &sResult);
#else
			VdoEnc_Builtin_PutBS(pathID, is_keyfrm, &(p_obj->venc_param), &sResult);
			pre_enc_num[pathID]++;
#endif
			p_obj->venc_param.bs_addr_1 += ALIGN_CEIL_4(p_obj->venc_param.bs_size_1);

			if (gvdoenc_dtsi_param[pathID].skip_frm_en) {
				SkipModeFrameIdx[pathID] ++;
			}
		}
		}
	}

	if (yuv_info.release_flag) {
		if (yuv_info.enable) {
			DBG_IND("%s_3 0x%08x\r\n",__func__,yuv_info.y_addr);
			nvtmpp_unlock_fastboot_blk(yuv_info.y_addr);
		}
		yuv_unlock_cnt[pathID]++;
		if (yuv_unlock_cnt[pathID] == 2) {
			builtin_stop_encode[pathID] = TRUE;
			set_flg(FLG_ID_VDOENC_BUILTIN_CHECK[pathID], FLG_VDOENC_BUILTIN_DONE);
		}
	}

	flags = VdoEnc_Builtin_spin_lock();
	bH26xStart = FALSE;
	VdoEnc_Builtin_spin_unlock(flags);

	return;
}

int H265Enc_builtin_init(UINT32 pathID, VDOENC_BUILTIN_INIT_INFO *p_info)
{
	UINT32 width = 0, height = 0;
	UINT32 svc_layer = 0, ltr_interval = 0, quality_lv = 0, d2d = 0, gdc = 0, colmv = 0, ltr_saving_buf_bs_size = 0;
	UINT32 srcout_size = 0, rotate = 0;
	ULONG venc_ctrl_0[2] = {0};

	/* H265 codec buffer */
	H26XEncMeminfo memInfo = {0};
	uintptr_t codec_mem_addr = 0;
	UINT32 codec_mem_size = 0;
	UINT32 sizePerSec = 0, uiMinIRatio = 150, uiMinPRatio = 100;

	VDOENC_BUILTIN_OBJ *p_obj = &gVdoEncBuiltinObj[pathID];

	if (bPreInit == 0) {
		memset(&gVdoEncBuiltinObj[pathID], 0, sizeof(VDOENC_BUILTIN_OBJ));
		memset(&enc_var[pathID], 0, sizeof(H26XENC_VAR));
	}
	memset(&init_hevc_obj[pathID], 0, sizeof(H265ENC_INIT));
	memset(&info_hevc_obj[pathID], 0, sizeof(H265ENC_INFO));

	// frc
	gVdoEncBuiltinObj[pathID].frc.sample_mode = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_FRC_SAMPLE_MODE);
	gVdoEncBuiltinObj[pathID].frc.rate = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_FRC_RATE);
	gVdoEncBuiltinObj[pathID].frc.frm_counter = 0;
	gVdoEncBuiltinObj[pathID].frc.rate_counter = 0;
	gVdoEncBuiltinObj[pathID].frc.output_counter = 0;
	gVdoEncBuiltinObj[pathID].frc.framepersecond = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_FRC_FRAMEPERSEC);
	gVdoEncBuiltinObj[pathID].frc.framepersecond_new = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_FRC_FRAMEPERSEC_NEW);

	width = p_info[pathID].width;
	height = p_info[pathID].height;
	venc_ctrl_0[0] = p_info[pathID].max_blk_addr;
	venc_ctrl_0[1] = p_info[pathID].max_blk_size;
	if (width == 0 || height == 0) {
		DBG_ERR("H265Enc_builtin_init error size width %d, height %d\r\n", (int)width, (int)height);
		return -1;
	}

	svc_layer = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_SVC_LAYER);
	ltr_interval = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_LTR_INTERVAL);
	quality_lv = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_QUALITY_LV);
	d2d = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_D2D);
	gdc = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_GDC);
	colmv = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_COLMV);
	srcout_size = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_SRCOUT_SIZE);
	rotate = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_ROTATE);
#if defined(CONFIG_NVT_SMALL_HDAL)
	ltr_saving_buf_bs_size = ((width * height * 3 /2) * 20/100);
#else
	ltr_saving_buf_bs_size = 0;
#endif

	// get H265 codec buffer size
	{
		memInfo.uiWidth = ALIGN_CEIL_16(((rotate == 1) || (rotate == 2)) ? height : width);
		memInfo.uiHeight = ((rotate == 1) || (rotate == 2)) ? width : height;
		memInfo.ucSVCLayer = svc_layer;
		memInfo.uiLTRInterval = ltr_interval;
		memInfo.ucQualityLevel = quality_lv;
		memInfo.bTileEn= 0;
		memInfo.bD2dEn= d2d;
		memInfo.bGdcEn= gdc;
		memInfo.bColMvEn = colmv;
		memInfo.bCommReconFrmBuf = 0;
		if(memInfo.uiLTRInterval > 0){
			memInfo.uiLtrSavingBufBsSize = ltr_saving_buf_bs_size;
		}else{
			memInfo.uiLtrSavingBufBsSize = 0;
		}
		codec_mem_addr = venc_ctrl_0[0];
		codec_mem_size = h265Enc_queryMemSize(&memInfo);

		if (codec_mem_size == 0) {
			DBG_ERR("H265Enc query error MemSize 0\r\n");
			return -1;
		}
	}

	p_obj->venc_param.codec_mem_addr = codec_mem_addr;
	p_obj->venc_param.codec_mem_size = codec_mem_size;
	p_obj->venc_param.bs_start_addr = p_obj->venc_param.codec_mem_addr + ALIGN_CEIL_4(p_obj->venc_param.codec_mem_size);
	p_obj->venc_param.bs_end_addr = venc_ctrl_0[0] + venc_ctrl_0[1] - VDOENC_MD_MAP_MAX_SIZE - VDOENC_BUF_RESERVED_BYTES - srcout_size;
	gBSStart[pathID] = p_obj->venc_param.bs_start_addr;
	gBSEnd[pathID] = p_obj->venc_param.bs_end_addr;
	gFrameRate[pathID] = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_FRAMERATE);
	gSec[pathID] = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_SEC);
	init_hevc_obj[pathID].uiFrmRate = gFrameRate[pathID] * 1000;
	bsque_max[pathID] = gFrameRate[pathID] * gSec[pathID];
	if (!VdoEnc_Builtin_AllocQueMem(pathID, bsque_max[pathID], VDOENC_BUILTIN_YUVQ_MAX, bPreInit)){
		DBG_ERR("VdoEnc Builtin AllocQueMem fail !!!\r\n");
		return -1;
	}

	if (pre_enc_num[pathID] > 0) {
		VDOENC_BUILTIN_BSQ *pBSQ;

		pBSQ = &(gVdoEncBuiltinObj[pathID].bsQueue);

		if (pBSQ->bFull) {
			p_obj->venc_param.bs_addr_1 = nvtmpp_buitin_sys_pa2va(pBSQ->Queue[bsque_max[pathID] - 1].Addr) + ALIGN_CEIL_4(pBSQ->Queue[bsque_max[pathID] - 1].bs_size_1);
			p_obj->venc_param.bs_size_1 = p_obj->venc_param.bs_end_addr - p_obj->venc_param.bs_addr_1;
			DBG_IND("pre init H265 bs_rear (0x%lx, 0x%x), bs_curr (0x%lx, 0x%x)\r\n",
				pBSQ->Queue[bsque_max[pathID] - 1].Addr,
				pBSQ->Queue[bsque_max[pathID] - 1].Size,
				p_obj->venc_param.bs_addr_1,
				p_obj->venc_param.bs_size_1);
		} else {
			if (pBSQ->Rear - 1 >= bsque_max[pathID]) {
				DBG_ERR("H265 init error bsq rear %d\r\n", pBSQ->Rear - 1);
				return -1;
			}
			p_obj->venc_param.bs_addr_1 = nvtmpp_buitin_sys_pa2va(pBSQ->Queue[pBSQ->Rear - 1].Addr) + ALIGN_CEIL_4(pBSQ->Queue[pBSQ->Rear - 1].bs_size_1);
			p_obj->venc_param.bs_size_1 = p_obj->venc_param.bs_end_addr - p_obj->venc_param.bs_addr_1;
			DBG_IND("pre init H265 bs_rear (0x%lx, 0x%x), bs_curr (0x%lx, 0x%x)\r\n",
				pBSQ->Queue[pBSQ->Rear - 1].Addr,
				pBSQ->Queue[pBSQ->Rear - 1].Size,
				p_obj->venc_param.bs_addr_1,
				p_obj->venc_param.bs_size_1);
		}
	} else {
		p_obj->venc_param.bs_addr_1 = p_obj->venc_param.bs_start_addr;
		p_obj->venc_param.bs_size_1 = p_obj->venc_param.bs_end_addr - p_obj->venc_param.bs_start_addr;
	}
	sizePerSec = _VdoEnc_builtin_GetBytesPerSecond(width, height);
#ifdef __KERNEL__
	p_obj->venc_param.bs_min_i_size = (UINT32)div_u64((UINT64)sizePerSec * (UINT64)uiMinIRatio, 100);
	p_obj->venc_param.bs_min_p_size = (UINT32)div_u64((UINT64)sizePerSec * (UINT64)uiMinPRatio, 100);
#else
	p_obj->venc_param.bs_min_i_size = ((UINT64)sizePerSec * (UINT64)uiMinIRatio / 100);
	p_obj->venc_param.bs_min_p_size = ((UINT64)sizePerSec * (UINT64)uiMinPRatio / 100);
#endif

	DBG_IND("codec_addr 0x%lx, codec_size 0x%x, bs_start_addr 0x%lx, bs_end_addr 0x%lx, bs_addr_1 0x%lx, bs_size_1 0x%x\r\n",
		(unsigned long)p_obj->venc_param.codec_mem_addr,
		(unsigned int)p_obj->venc_param.codec_mem_size,
		(unsigned long)p_obj->venc_param.bs_start_addr,
		(unsigned long)p_obj->venc_param.bs_end_addr,
		(unsigned long)p_obj->venc_param.bs_addr_1,
		(unsigned int)p_obj->venc_param.bs_size_1);
	// Init encode engine
	{
		init_hevc_obj[pathID].uiDisplayWidth = (rotate == 1 || rotate == 2) ? height : width;
		init_hevc_obj[pathID].uiWidth  = ALIGN_CEIL_16(init_hevc_obj[pathID].uiDisplayWidth);
		init_hevc_obj[pathID].uiHeight = (rotate == 1 || rotate == 2) ? width : height;
		init_hevc_obj[pathID].bTileEn = 0;
		init_hevc_obj[pathID].eQualityLvl = quality_lv;
		init_hevc_obj[pathID].uiEncBufAddr = codec_mem_addr;
		init_hevc_obj[pathID].uiEncBufSize = codec_mem_size;
		init_hevc_obj[pathID].uiGopNum = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_GOP);
		init_hevc_obj[pathID].uiUsrQpSize = 0;
		init_hevc_obj[pathID].ucDisableDB = 0;
		init_hevc_obj[pathID].cDBAlpha = 0;
		init_hevc_obj[pathID].cDBBeta = 0;
		init_hevc_obj[pathID].iQpCbOffset = 0;
		init_hevc_obj[pathID].iQpCrOffset= 0;
		init_hevc_obj[pathID].ucSVCLayer = svc_layer;
		init_hevc_obj[pathID].uiLTRInterval = ltr_interval;
		init_hevc_obj[pathID].uiLTRPreRef = 0;
		init_hevc_obj[pathID].uiSAO = 1;
		init_hevc_obj[pathID].uiSaoLumaFlag = 1;
		init_hevc_obj[pathID].uiSaoChromaFlag = 1;
		init_hevc_obj[pathID].uiBitRate = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_INIT_BYTE_RATE)*8;
		init_hevc_obj[pathID].ucIQP = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_INIT_IQP);
		init_hevc_obj[pathID].ucPQP = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_INIT_PQP);
		init_hevc_obj[pathID].ucMaxIQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MAX_IQP);
		init_hevc_obj[pathID].ucMinIQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MIN_IQP);
		init_hevc_obj[pathID].ucMaxPQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MAX_PQP);
		init_hevc_obj[pathID].ucMinPQp = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_MIN_PQP);
		if (VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_RC_MODE) == BUILTIN_VDOENC_RC_FIXQP) {
			init_hevc_obj[pathID].iIPQPoffset = 0;
			init_hevc_obj[pathID].uiStaticTime = 0;
		} else {
			init_hevc_obj[pathID].iIPQPoffset = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_IP_WEIGHT);
			init_hevc_obj[pathID].uiStaticTime = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_STATIC_TIME);
		}
		init_hevc_obj[pathID].bFBCEn = 0;
		init_hevc_obj[pathID].bGrayEn = 0;
		init_hevc_obj[pathID].bFastSearchEn = 0;//pinfo->fast_search;	// nt98520 not support search range set to 1 : big range //
		init_hevc_obj[pathID].bHwPaddingEn = 1;
		init_hevc_obj[pathID].ucRotate = rotate;
		init_hevc_obj[pathID].bD2dEn = d2d;
		init_hevc_obj[pathID].bGdcEn = gdc;
		init_hevc_obj[pathID].bColMvEn = colmv;
		init_hevc_obj[pathID].uiMultiTLayer = 0;
		if (init_hevc_obj[pathID].uiWidth > 1920 && init_hevc_obj[pathID].uiHeight > 1080) {
			init_hevc_obj[pathID].ucLevelIdc = 150;
		} else {
			init_hevc_obj[pathID].ucLevelIdc = 123;
		}
		init_hevc_obj[pathID].bVUIEn = 0;
		init_hevc_obj[pathID].usSarWidth = width;
		init_hevc_obj[pathID].usSarHeight = height;
		init_hevc_obj[pathID].ucMatrixCoef = 2;
		init_hevc_obj[pathID].ucTransferCharacteristics = 2;
		init_hevc_obj[pathID].ucColourPrimaries = 2;
		init_hevc_obj[pathID].ucVideoFormat = 5;
		init_hevc_obj[pathID].ucColorRange = 0;
		init_hevc_obj[pathID].bTimeingPresentFlag = 0;
		init_hevc_obj[pathID].bRecBufComm = 0;
		init_hevc_obj[pathID].uiRecBufAddr[0] = 0;
		init_hevc_obj[pathID].uiRecBufAddr[1] = 0;
		init_hevc_obj[pathID].uiRecBufAddr[2] = 0;
		if(init_hevc_obj[pathID].uiLTRInterval > 0) {
			init_hevc_obj[pathID].uiLtrSavingBufBsSize = ltr_saving_buf_bs_size;
		} else {
			init_hevc_obj[pathID].uiLtrSavingBufBsSize = 0;
        }

		h265Enc_initEncoder(&init_hevc_obj[pathID], &enc_var[pathID], bPreInit);
		{
			uintptr_t desc_addr = 0;
			UINT32    desc_size = 0;
			UINT8     *ptr;

			h265Enc_getSeqHdr(&enc_var[pathID], &desc_addr, &desc_size);

			ptr = (UINT8 *)desc_addr;
			if (*(ptr+0) != 0x00 || *(ptr+1) != 0x00 || *(ptr+2) != 0x00 || *(ptr+3) != 0x01 || *(ptr+4) != 0x40) {
				DBG_ERR("H265 error descriptor\r\n");
				return -1;
			}
		}
	}

	return 0;
}

void H265Enc_builtin_encodeOne(UINT32 pathID, BOOL enIntFirst)
{
	UINT32 interrupt = 0;
	BOOL is_keyfrm = 0;
	BOOL enc_err = 0;
	H26XEncResultCfg sResult = {0};
	VDOENC_BUILTIN_OBJ *p_obj = &gVdoEncBuiltinObj[pathID];
	VDOENC_BUILTIN_YUV_INFO yuv_info = {0};
	unsigned long flags = 0;
	UINT32 trig_enc = 0;
//#if defined(__FREERTOS)
//	static BOOL bStopRTOSEnc = FALSE;

//	if (bStopRTOSEnc == TRUE) {
//		return;
//	}
//#endif

#ifdef __KERNEL__
	if (enIntFirst) {
		goto EN_INT_FIRST_H265;
	}
#endif

	if (_VdoEnc_Builtin_HowManyInYUVQ(pathID) <= 0) {
		return;
	}

	if (_VdoEnc_Builtin_GetYuv(pathID, &yuv_info) == FALSE) {
		return;
	}

	if (p_obj->bsQueue.bFull) {
		//DBG_DUMP("Bs Queue Full, Not Encode Further\r\n");
		if (yuv_info.release_flag) {
			if (yuv_info.enable) {
				DBG_IND("%s_1 0x%08x\r\n",__func__,yuv_info.y_addr);
				nvtmpp_unlock_fastboot_blk(yuv_info.y_addr);
			}
			yuv_unlock_cnt[pathID]++;
			if (yuv_unlock_cnt[pathID] == 2) {
				builtin_stop_encode[pathID] = TRUE;
				set_flg(FLG_ID_VDOENC_BUILTIN_CHECK[pathID], FLG_VDOENC_BUILTIN_DONE);
			}
		}
		return;
	}

	info_hevc_obj[pathID].ePicType = h265Enc_getNxtPicType(&enc_var[pathID]);
	info_hevc_obj[pathID].bGetSeqHdrEn = IS_ISLICE(info_hevc_obj[pathID].ePicType);
	is_keyfrm = IS_ISLICE(info_hevc_obj[pathID].ePicType);
	if (p_obj->venc_param.bs_end_addr - p_obj->venc_param.bs_addr_1 < (IS_ISLICE(info_hevc_obj[pathID].ePicType) ? p_obj->venc_param.bs_min_i_size : p_obj->venc_param.bs_min_p_size)) {
		//DBG_DUMP("Bs Buffer Full, Not Encode Further\r\n");
		if (yuv_info.release_flag) {
			if (yuv_info.enable) {
				DBG_IND("%s_2 0x%08x\r\n",__func__,yuv_info.y_addr);
				nvtmpp_unlock_fastboot_blk(yuv_info.y_addr);
			}
			yuv_unlock_cnt[pathID]++;
			if (yuv_unlock_cnt[pathID] == 2) {
				builtin_stop_encode[pathID] = TRUE;
				set_flg(FLG_ID_VDOENC_BUILTIN_CHECK[pathID], FLG_VDOENC_BUILTIN_DONE);
			}
		}
		return;
	}
#if TNR_ENABLE
	{
		H26XEncTnrCfg stCfg = {0};
		_VdoEnc_builtin_3DNR_Internal(pathID, (UINT32)&stCfg);
		h26XEnc_setTnrCfg(&enc_var[pathID], &stCfg);
	}
#endif
#if SPN_ENABLE
	{
		VDOENC_BUILTIN_H26XENC_SPN pinfo;
		H26XEncSpnCfg stCfg = {0};
		int i;
		_VdoEnc_builtin_PostSharpen_Internal(pathID, (ULONG)&pinfo);
		stCfg.bEnable = pinfo.bEnable;
		stCfg.ucConEng = pinfo.ucConEng;
		stCfg.usSlopConEng = pinfo.usSlopConEng;
		stCfg.ucBHC = pinfo.ucBHC;
		stCfg.ucDHC = pinfo.ucDHC;
		stCfg.ucEWT = pinfo.ucEWT;
		stCfg.ucEWG = pinfo.ucEWG;
		//stCfg.ucESS[0] = pinfo.ucEdgeSharpStr1;
		stCfg.ucESS = pinfo.ucEdgeSharpStr1;
		stCfg.ucCT = pinfo.ucCT;
		stCfg.ucNL = pinfo.ucNL;
		stCfg.ucBIG = pinfo.ucBIG;
		stCfg.usFlatTh = pinfo.usFlatTh;
		stCfg.usEdgeTh = pinfo.usEdgeTh;
		stCfg.ucEdgeStr = pinfo.ucEdgeStr;
		stCfg.ucTransitionStr = pinfo.ucTransitionStr;
		stCfg.ucMotionStr = pinfo.ucMotionStr;
		stCfg.ucStaticStr = pinfo.ucStaticStr;
		stCfg.ucFlatStr = pinfo.ucFlatStr;
		for(i=0;i<17;i++)
			stCfg.ucNC[i] = pinfo.ucNC[i];
		for(i=0;i<9;i++)
			stCfg.usEWG[i] = pinfo.usEWG[i];
		stCfg.bShowSharpInfo = pinfo.bShowSharpInfo;
		h26XEnc_setSpnCfg(&enc_var[pathID], &stCfg);
	}
#endif

	// set encode param to H265ENC_INFO
	if (yuv_info.enable) {
		// do frc
		trig_enc = _vdoenc_builtin_frc_is_select(pathID, &(p_obj->frc));

		if (builtin_stop_encode[pathID]) {
			return;
		}
		if (trig_enc) {
		if (gvdoenc_dtsi_param[pathID].skip_frm_en) {
			UINT32 uiSkipFrmTargetFr = gvdoenc_dtsi_param[pathID].skip_frm_target_fr;
			UINT32 uiSkipFrmInputCnt = gvdoenc_dtsi_param[pathID].skip_frm_input_cnt;
			UINT32 ratio = uiSkipFrmTargetFr / uiSkipFrmInputCnt;
			if (IS_ISLICE(info_hevc_obj[pathID].ePicType)) {
				SkipModeFrameIdx[pathID] = 0;
			}
			if (SkipModeFrameIdx[pathID] % ratio == 0 || IS_ISLICE(info_hevc_obj[pathID].ePicType)) {
				info_hevc_obj[pathID].bSkipFrmEn = FALSE;
			} else {
				info_hevc_obj[pathID].bSkipFrmEn = TRUE;
			}
		} else {
			info_hevc_obj[pathID].bSkipFrmEn = FALSE;
		}

		info_hevc_obj[pathID].uiSrcYAddr = yuv_info.y_addr; //0: y addr, 1: uv addr
		info_hevc_obj[pathID].uiSrcCAddr = yuv_info.c_addr; //0: y addr, 1: uv addr
		info_hevc_obj[pathID].uiSrcYLineOffset = yuv_info.y_line_offset; //0: y addr, 1: uv addr;
		info_hevc_obj[pathID].uiSrcCLineOffset = yuv_info.c_line_offset;
		info_hevc_obj[pathID].bSrcOutEn = 0;
#ifndef VDOCDC_EMU
		info_hevc_obj[pathID].SdeCfg.bEnable = gvdoenc_dtsi_param[pathID].ycc_en;
		info_hevc_obj[pathID].SdeCfg.uiWidth = gvdoenc_dtsi_param[pathID].ycc_width;
		info_hevc_obj[pathID].SdeCfg.uiHeight = gvdoenc_dtsi_param[pathID].ycc_height;
		info_hevc_obj[pathID].SdeCfg.uiYLofst = gvdoenc_dtsi_param[pathID].ycc_yloff;
		info_hevc_obj[pathID].SdeCfg.uiCLofst = gvdoenc_dtsi_param[pathID].ycc_cloff;
#endif
		info_hevc_obj[pathID].uiBsOutBufAddr = p_obj->venc_param.bs_addr_1;
		info_hevc_obj[pathID].uiBsOutBufSize = p_obj->venc_param.bs_end_addr - p_obj->venc_param.bs_addr_1;
//		info_obj.uiNalLenOutAddr = nalu_len_addr; //maybe no need to set

		if (fastboot_osg_setup_h26x_stamp(&enc_var[pathID], pathID, &yuv_info)){
			DBG_ERR("fail to setup h26x stamp for encoding path(%u)\n", pathID);
			return;
		}

		// encode picture
		h265Enc_prepareOnePicture(&info_hevc_obj[pathID], &enc_var[pathID]);

		{
			flags = VdoEnc_Builtin_spin_lock();

			if (H26X_ENC_MODE) {
				h26x_setEncDirectRegSet(0, h26xEnc_getVaAPBAddr(&enc_var[pathID]));
				if (disable_h26x_int == FALSE) {
					h26x_setIntEn(0, H26X_FINISH_INT | H26X_BSDMA_INT | H26X_ERR_INT | H26X_FRAME_TIME_OUT_INT | H26X_BSOUT_INT | H26X_FBC_ERR_INT | H26X_SWRST_FINISH_INT | H26X_DMACH_DIS_INT);
				}
			} else {
				h26x_setEncLLRegSet(0, 1, h26x_getPhyAddr(h26xEnc_getVaLLCAddr(&enc_var[pathID])));
			}
			#if 0 //TODO
			if (h26x_getRtosFastboot() == 0) {
				h26x_setLock(enc_var[pathID].uiChipIdx);
				h26x_enableClk(enc_var[pathID].uiChipIdx);
			}
			#endif

#if 0 //TBD
			if (h26x_getPatchCount(0) == 0) {
				//h26x_setUnitChecksum(0, 0xc0000000);
				h26x_start(0);
				h26x_waitINT(0);
				h26x_module_reset(0);
				h26x_setEncDirectRegSet(0, h26xEnc_getVaAPBAddr(&enc_var[pathID]));
				h26x_setPatchCount(0, 1);
			}
#endif

			h26x_start(0);
			bH26xStart = TRUE;

			// notify ipp venc start to encode
			if (vdoenc_start_enc_cb) {
				vdoenc_start_enc_cb(&yuv_info, 0);
			}
			if (disable_h26x_int == TRUE) {
				en_lnx_h26x_int_fisrt = 1 << 16 | pathID;
				VdoEnc_Builtin_spin_unlock(flags);
//#if defined(__FREERTOS)
//				bStopRTOSEnc = TRUE;
//#endif
				return;
			}

			VdoEnc_Builtin_spin_unlock(flags);

#ifdef __KERNEL__
EN_INT_FIRST_H265:
			if (enIntFirst) {
				H26XENC_VAR *pVar = &enc_var[pathID];
				H26XCOMN_CTX *pComnCtx = (H26XCOMN_CTX *)pVar->pComnCtx;
				DBG_IND("uiAPBAddr 0x%x, uiSPFrmBsOutAddr (0x%x, 0x%x), uiBsOutAddr (0x%x, 0x%x)\r\n",
					pComnCtx->stVaAddr.uiAPBAddr,
					pComnCtx->stVaAddr.uiSPFrmBsOutAddr, nvtmpp_buitin_sys_pa2va(pComnCtx->stVaAddr.uiSPFrmBsOutAddr),
					pComnCtx->stVaAddr.uiBsOutAddr, nvtmpp_buitin_sys_pa2va(pComnCtx->stVaAddr.uiBsOutAddr));
				pComnCtx->stVaAddr.uiBsOutAddr = nvtmpp_buitin_sys_pa2va(pComnCtx->stVaAddr.uiBsOutAddr);
				h26x_setIntEn(0, H26X_FINISH_INT | H26X_BSDMA_INT | H26X_ERR_INT | H26X_TIME_OUT_INT | H26X_FRAME_TIME_OUT_INT | H26X_BSOUT_INT | H26X_FBC_ERR_INT | H26X_SRC_DECMP_ERR_INT | H26X_SWRST_FINISH_INT | H26X_SRC_D2D_OV_INT | H26X_DMACH_DIS_INT);
			}
#endif

			interrupt = h26x_waitINT(0);

			if ((interrupt & h26x_getIntEn(0)) != 0x00000001)  {
				enc_err = 1;
				//h26x_resetHW(enc_var[pathID].uiChipIdx);
				h26x_module_reset(enc_var[pathID].uiChipIdx);

				DBG_ERR("encode error interrupt(0x%08x)\r\n", (int)interrupt);

				return;
				//h26x_prtReg(enc_var[pathID].uiChipIdx);
				//h26x_getDebug(enc_var[pathID].uiChipIdx);
				//vtrc_prtReg(enc_var[pathID].uiChipIdx);
			}

			h265Enc_getResult(&enc_var[pathID], H26X_ENC_MODE, &sResult, interrupt);
			#if 0 //TODO
			if (h26x_getRtosFastboot() == 0) {
				h26x_disableClk(enc_var[pathID].uiChipIdx);
				h26x_setUnLock(enc_var[pathID].uiChipIdx);
			}
			#endif
#if 0
			{
				UINT8 *ptr;
				ptr = (UINT8 *)p_obj->venc_param.bs_addr_1;
				DBG_DUMP("[%d] H265 BSAddr 0x%lx, BSSize 0x%x\r\n", pathID, p_obj->venc_param.bs_addr_1, sResult.uiBSLen);
				DBG_DUMP("%02X %02X %02X %02X %02X %02X %02X %02X\r\n",
					*(ptr+0), *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5), *(ptr+6), *(ptr+7));
				DBG_DUMP("========================\r\n");
			}
#endif
			p_obj->venc_param.interrupt = interrupt;
			p_obj->venc_param.timestamp = yuv_info.timestamp;
			p_obj->venc_param.bs_size_1 = sResult.uiBSLen;
		}

		if (enc_err == 0) {
#ifdef __KERNEL__
			VdoEnc_Builtin_PutBS(pathID, is_keyfrm, &(p_obj->venc_param), &sResult);
#else
			VdoEnc_Builtin_PutBS(pathID, is_keyfrm, &(p_obj->venc_param), &sResult);
			pre_enc_num[pathID]++;
#endif
			p_obj->venc_param.bs_addr_1 += ALIGN_CEIL_4(p_obj->venc_param.bs_size_1);

			if (gvdoenc_dtsi_param[pathID].skip_frm_en) {
				SkipModeFrameIdx[pathID] ++;
			}
		}
		}
	}

	if (yuv_info.release_flag) {
		if (yuv_info.enable) {
			DBG_IND("%s_3 0x%08x\r\n",__func__,yuv_info.y_addr);
			nvtmpp_unlock_fastboot_blk(yuv_info.y_addr);
		}
		yuv_unlock_cnt[pathID]++;
		if (yuv_unlock_cnt[pathID] == 2) {
			builtin_stop_encode[pathID] = TRUE;
			set_flg(FLG_ID_VDOENC_BUILTIN_CHECK[pathID], FLG_VDOENC_BUILTIN_DONE);
		}
	}

	flags = VdoEnc_Builtin_spin_lock();
	bH26xStart = FALSE;
	VdoEnc_Builtin_spin_unlock(flags);

	return;
}

#if defined(__FREERTOS)
int H265Enc_builtin_init_lite(VDOENC_BUILTIN_INIT_INFO *p_info, H26XENC_VAR *enc_var_lite, VDOENC_BUILTIN_OBJ *venc_obj_lite)
{
	H265ENC_INIT init_hevc_obj_lite = {0};
	UINT32 width = 0, height = 0;
	UINT32 svc_layer = 0, ltr_interval = 0, quality_lv = 0, d2d = 0, gdc = 1, colmv = 1, ltr_saving_buf_bs_size = 0;
	UINT32 srcout_size = 0, rotate = 0;
	ULONG venc_ctrl_0[2] = {0};

	/* H265 codec buffer */
	H26XEncMeminfo memInfo = {0};
	uintptr_t codec_mem_addr = 0;
	UINT32 codec_mem_size = 0;

	VDOENC_BUILTIN_OBJ *p_obj = venc_obj_lite;

	width = p_info->width;
	height = p_info->height;
	venc_ctrl_0[0] = p_info->max_blk_addr;
	venc_ctrl_0[1] = p_info->max_blk_size;
	if (width == 0 || height == 0) {
		DBG_ERR("H265Enc_builtin_init error size width %d, height %d\r\n", (int)width, (int)height);
		return -1;
	}

	// get H265 codec buffer size
	{
		memInfo.uiWidth = ALIGN_CEIL_16(width);
		memInfo.uiHeight = height;
		memInfo.ucSVCLayer = svc_layer;
		memInfo.uiLTRInterval = ltr_interval;
		memInfo.ucQualityLevel = quality_lv;
		memInfo.bTileEn= 0;
		memInfo.bD2dEn= d2d;
		memInfo.bGdcEn= gdc;
		memInfo.bColMvEn = colmv;
		memInfo.bCommReconFrmBuf = 0;
		if(memInfo.uiLTRInterval > 0){
			memInfo.uiLtrSavingBufBsSize = ltr_saving_buf_bs_size;
		}else{
			memInfo.uiLtrSavingBufBsSize = 0;
		}
		codec_mem_addr = venc_ctrl_0[0];
		codec_mem_size = h265Enc_queryMemSize(&memInfo);

		if (codec_mem_size == 0) {
			DBG_ERR("H265Enc query error MemSize 0\r\n");
			return -1;
		}
	}

	p_obj->venc_param.codec_mem_addr = codec_mem_addr;
	p_obj->venc_param.codec_mem_size = codec_mem_size;
	p_obj->venc_param.bs_start_addr = p_obj->venc_param.codec_mem_addr + ALIGN_CEIL_4(p_obj->venc_param.codec_mem_size);
	p_obj->venc_param.bs_end_addr = venc_ctrl_0[0] + venc_ctrl_0[1] - VDOENC_MD_MAP_MAX_SIZE - VDOENC_BUF_RESERVED_BYTES - srcout_size;
	init_hevc_obj_lite.uiFrmRate = 30000;

	p_obj->venc_param.bs_addr_1 = p_obj->venc_param.bs_start_addr;
	p_obj->venc_param.bs_size_1 = p_obj->venc_param.bs_end_addr - p_obj->venc_param.bs_start_addr;

	// Init encode engine
	{
		init_hevc_obj_lite.uiDisplayWidth = width;
		init_hevc_obj_lite.uiWidth  = ALIGN_CEIL_16(init_hevc_obj_lite.uiDisplayWidth);
		init_hevc_obj_lite.uiHeight = height;
		init_hevc_obj_lite.bTileEn = 0;
		init_hevc_obj_lite.eQualityLvl = quality_lv;
		init_hevc_obj_lite.uiEncBufAddr = codec_mem_addr;
		init_hevc_obj_lite.uiEncBufSize = codec_mem_size;
		init_hevc_obj_lite.uiGopNum = 15;
		init_hevc_obj_lite.uiUsrQpSize = 0;
		init_hevc_obj_lite.ucDisableDB = 0;
		init_hevc_obj_lite.cDBAlpha = 0;
		init_hevc_obj_lite.cDBBeta = 0;
		init_hevc_obj_lite.iQpCbOffset = 0;
		init_hevc_obj_lite.iQpCrOffset= 0;
		init_hevc_obj_lite.ucSVCLayer = svc_layer;
		init_hevc_obj_lite.uiLTRInterval = ltr_interval;
		init_hevc_obj_lite.uiLTRPreRef = 0;
		init_hevc_obj_lite.uiSAO = 1;
		init_hevc_obj_lite.uiSaoLumaFlag = 1;
		init_hevc_obj_lite.uiSaoChromaFlag = 1;
		init_hevc_obj_lite.uiBitRate = 512*1024;
		init_hevc_obj_lite.ucIQP = 37;
		init_hevc_obj_lite.ucPQP = 37;
		init_hevc_obj_lite.ucMaxIQp = 37;
		init_hevc_obj_lite.ucMinIQp = 37;
		init_hevc_obj_lite.ucMaxPQp = 37;
		init_hevc_obj_lite.ucMinPQp = 37;
		init_hevc_obj_lite.iIPQPoffset = 0;
		init_hevc_obj_lite.uiStaticTime = 0;
		init_hevc_obj_lite.bFBCEn = 0;
		init_hevc_obj_lite.bGrayEn = 0;
		init_hevc_obj_lite.bFastSearchEn = 0;//pinfo->fast_search;	// nt98520 not support search range set to 1 : big range //
		init_hevc_obj_lite.bHwPaddingEn = 1;
		init_hevc_obj_lite.ucRotate = rotate;
		init_hevc_obj_lite.bD2dEn = d2d;
		init_hevc_obj_lite.bGdcEn = gdc;
		init_hevc_obj_lite.bColMvEn = colmv;
		init_hevc_obj_lite.uiMultiTLayer = 0;
		if (init_hevc_obj_lite.uiWidth > 1920 && init_hevc_obj_lite.uiHeight > 1080) {
			init_hevc_obj_lite.ucLevelIdc = 150;
		} else {
			init_hevc_obj_lite.ucLevelIdc = 123;
		}
		init_hevc_obj_lite.bVUIEn = 0;
		init_hevc_obj_lite.usSarWidth = width;
		init_hevc_obj_lite.usSarHeight = height;
		init_hevc_obj_lite.ucMatrixCoef = 2;
		init_hevc_obj_lite.ucTransferCharacteristics = 2;
		init_hevc_obj_lite.ucColourPrimaries = 2;
		init_hevc_obj_lite.ucVideoFormat = 5;
		init_hevc_obj_lite.ucColorRange = 0;
		init_hevc_obj_lite.bTimeingPresentFlag = 0;
		init_hevc_obj_lite.bRecBufComm = 0;
		init_hevc_obj_lite.uiRecBufAddr[0] = 0;
		init_hevc_obj_lite.uiRecBufAddr[1] = 0;
		init_hevc_obj_lite.uiRecBufAddr[2] = 0;
		if(init_hevc_obj_lite.uiLTRInterval > 0) {
			init_hevc_obj_lite.uiLtrSavingBufBsSize = ltr_saving_buf_bs_size;
		} else {
			init_hevc_obj_lite.uiLtrSavingBufBsSize = 0;
		}

		h265Enc_initEncoder(&init_hevc_obj_lite, enc_var_lite, 0);
		{
			uintptr_t desc_addr = 0;
			UINT32	  desc_size = 0;
			UINT8	  *ptr;

			h265Enc_getSeqHdr(enc_var_lite, &desc_addr, &desc_size);

			ptr = (UINT8 *)desc_addr;
			if (*(ptr+0) != 0x00 || *(ptr+1) != 0x00 || *(ptr+2) != 0x00 || *(ptr+3) != 0x01 || *(ptr+4) != 0x40) {
				DBG_ERR("H265 error descriptor\r\n");
				return -1;
			}
		}
	}

	return 0;
}

void H265Enc_builtin_encodeOne_lite(H26XENC_VAR *enc_var_lite, VDOENC_BUILTIN_OBJ *venc_obj_lite)
{
	H265ENC_INFO info_hevc_obj_lite = {0};
	UINT32 interrupt = 0;
	VDOENC_BUILTIN_OBJ *p_obj = venc_obj_lite;
	VDOENC_BUILTIN_YUV_INFO yuv_info = {0};

	yuv_info.enable = 1;
	yuv_info.y_addr = (uintptr_t)vdoenc_builtin_alloc(H265E_WIDTH_MIN_538 * H265E_HEIGHT_MIN_538 * 3 /2);
	yuv_info.c_addr = yuv_info.y_addr + (H265E_WIDTH_MIN_538 * H265E_HEIGHT_MIN_538);
	yuv_info.width = H265E_WIDTH_MIN_538;
	yuv_info.height = H265E_HEIGHT_MIN_538;
	yuv_info.y_line_offset = H265E_WIDTH_MIN_538;
	yuv_info.c_line_offset = H265E_WIDTH_MIN_538;

	info_hevc_obj_lite.ePicType = h265Enc_getNxtPicType(enc_var_lite);
	info_hevc_obj_lite.bGetSeqHdrEn = IS_ISLICE(info_hevc_obj_lite.ePicType);

	// set encode param to H265ENC_INFO
	info_hevc_obj_lite.bSkipFrmEn = FALSE;
	info_hevc_obj_lite.uiSrcYAddr = yuv_info.y_addr; //0: y addr, 1: uv addr
	info_hevc_obj_lite.uiSrcCAddr = yuv_info.c_addr; //0: y addr, 1: uv addr
	info_hevc_obj_lite.uiSrcYLineOffset = yuv_info.y_line_offset; //0: y addr, 1: uv addr;
	info_hevc_obj_lite.uiSrcCLineOffset = yuv_info.c_line_offset;
	info_hevc_obj_lite.bSrcOutEn = 0;
#ifndef VDOCDC_EMU
	info_hevc_obj_lite.SdeCfg.bEnable = 0;
	info_hevc_obj_lite.SdeCfg.uiWidth = 0;
	info_hevc_obj_lite.SdeCfg.uiHeight = 0;
	info_hevc_obj_lite.SdeCfg.uiYLofst = 0;
	info_hevc_obj_lite.SdeCfg.uiCLofst = 0;
#endif
	info_hevc_obj_lite.uiBsOutBufAddr = p_obj->venc_param.bs_addr_1;
	info_hevc_obj_lite.uiBsOutBufSize = p_obj->venc_param.bs_end_addr - p_obj->venc_param.bs_addr_1;

	// encode picture
	h265Enc_prepareOnePicture(&info_hevc_obj_lite, enc_var_lite);

	if (H26X_ENC_MODE) {
		h26x_setEncDirectRegSet(0, h26xEnc_getVaAPBAddr(enc_var_lite));
	} else {
		h26x_setEncLLRegSet(0, 1, h26x_getPhyAddr(h26xEnc_getVaLLCAddr(enc_var_lite)));
	}

	h26x_start(0);
	interrupt = h26x_waitINT(0);
	if ((interrupt & h26x_getIntEn(0)) != 0x00000001)  {
		DBG_ERR("encode lite error interrupt(0x%08x)\r\n", (int)interrupt);
	}
	h26x_module_reset(0);
	h26x_setEncDirectRegSet(0, h26xEnc_getVaAPBAddr(enc_var_lite));
	h26x_setPatchCount(0, 1);

	vdoenc_builtin_free((void *)yuv_info.y_addr);

	return;
}
#endif

int MJPGEnc_builtin_init(UINT32 pathID, VDOENC_BUILTIN_INIT_INFO *p_info)
{
	UINT32 width = 0, height = 0;
	ULONG venc_ctrl_0[2] = {0};
	UINT32 sizePerSec = 0, uiMinIRatio = 150;
	VDOENC_BUILTIN_OBJ *p_obj = &gVdoEncBuiltinObj[pathID];

	memset(&gVdoEncBuiltinObj[pathID], 0, sizeof(VDOENC_BUILTIN_OBJ));

	// frc
	gVdoEncBuiltinObj[pathID].frc.sample_mode = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_FRC_SAMPLE_MODE);
	gVdoEncBuiltinObj[pathID].frc.rate = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_FRC_RATE);
	gVdoEncBuiltinObj[pathID].frc.frm_counter = 0;
	gVdoEncBuiltinObj[pathID].frc.rate_counter = 0;
	gVdoEncBuiltinObj[pathID].frc.output_counter = 0;
	gVdoEncBuiltinObj[pathID].frc.framepersecond = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_FRC_FRAMEPERSEC);
	gVdoEncBuiltinObj[pathID].frc.framepersecond_new = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_FRC_FRAMEPERSEC_NEW);

	jpeg_width[pathID] = p_info[pathID].width;
	jpeg_height[pathID] = p_info[pathID].height;
	venc_ctrl_0[0] = p_info[pathID].max_blk_addr;
	venc_ctrl_0[1] = p_info[pathID].max_blk_size;
	jpeg_max_mem_size[pathID] =p_info[pathID].max_blk_size;
	jpeg_quality[pathID] = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_JPEG_QUALITY);
	jpeg_fps[pathID] = VdoEnc_builtin_get_dtsi_param(pathID, BUILTIN_VDOENC_DTSI_PARAM_JPEG_FPS);
	jpg_enc_setdata(pathID, JPG_TARGET_RATE, 0);
	jpg_enc_setdata(pathID, JPG_VBR_QUALITY, jpeg_quality[pathID]);
	bsque_max[pathID] = 90; // 30fps * 3s

	// enable jpeg encode in builtin linux
	if (!VdoEnc_Builtin_AllocQueMem(pathID, bsque_max[pathID], VDOENC_BUILTIN_YUVQ_MAX, 0)){
		DBG_ERR("VdoEnc Builtin AllocQueMem fail !!!\r\n");
		return -1;
	}

	p_obj->venc_param.bs_start_addr = venc_ctrl_0[0];//(UINT32)vmalloc(3200);
	p_obj->venc_param.bs_end_addr = p_obj->venc_param.bs_start_addr + venc_ctrl_0[1];//3200;
	p_obj->venc_param.bs_addr_1 = p_obj->venc_param.bs_start_addr;
	p_obj->venc_param.bs_size_1 = p_obj->venc_param.bs_end_addr - p_obj->venc_param.bs_start_addr;
	sizePerSec = _VdoEnc_builtin_GetBytesPerSecond(width, height);
#ifdef __KERNEL__
	p_obj->venc_param.bs_min_i_size = (UINT32)div_u64((UINT64)sizePerSec * (UINT64)uiMinIRatio, 100);
#else
	p_obj->venc_param.bs_min_i_size = ((UINT64)sizePerSec * (UINT64)uiMinIRatio / 100);
#endif

	return 0;
}

void MJPGEnc_builtin_encodeOne(UINT32 pathID)
{
	UINT32 width = 0, height = 0;
	BOOL is_keyfrm = 0;
	H26XEncResultCfg sResult = {0};
	ER ret = E_OK;
	KDRV_VDOJPGE_PARAM jpgenc_param = {0};
	VDOENC_BUILTIN_YUV_INFO yuv_info = {0};
	VDOENC_BUILTIN_OBJ *p_obj = &gVdoEncBuiltinObj[pathID];
	UINT32 trig_enc = 0;

	if (_VdoEnc_Builtin_HowManyInYUVQ(pathID) <= 0) {
		return;
	}

	if (_VdoEnc_Builtin_GetYuv(pathID, &yuv_info) == FALSE) {
		return;
	}

	if (p_obj->bsQueue.bFull) {
		//DBG_DUMP("Bs Queue Full, Not Encode Further\r\n");
		if (yuv_info.release_flag) {
			if (yuv_info.enable) {
				DBG_IND("%s_1 0x%08x\r\n",__func__,yuv_info.y_addr);
				nvtmpp_unlock_fastboot_blk(yuv_info.y_addr);
			}
			yuv_unlock_cnt[pathID]++;
			if (yuv_unlock_cnt[pathID] == 2) {
				builtin_stop_encode[pathID] = TRUE;
				set_flg(FLG_ID_VDOENC_BUILTIN_CHECK[pathID], FLG_VDOENC_BUILTIN_DONE);
			}
		}
		return;
	}

	if (p_obj->venc_param.bs_end_addr - p_obj->venc_param.bs_addr_1 < 0x64000/*p_obj->venc_param.bs_min_i_size*/) {
		//DBG_DUMP("Bs Buffer Full, Not Encode Further\r\n");
		if (yuv_info.release_flag) {
			if (yuv_info.enable) {
				DBG_IND("%s_2 0x%08x\r\n",__func__,yuv_info.y_addr);
				nvtmpp_unlock_fastboot_blk(yuv_info.y_addr);
			}
			yuv_unlock_cnt[pathID]++;
			if (yuv_unlock_cnt[pathID] == 2) {
				builtin_stop_encode[pathID] = TRUE;
				set_flg(FLG_ID_VDOENC_BUILTIN_CHECK[pathID], FLG_VDOENC_BUILTIN_DONE);
			}
		}
		return;
	}

	if (builtin_stop_encode[pathID]) {
		return;
	}

	width = yuv_info.width;
	height = yuv_info.height;

	if(yuv_info.enable)
	{
		// do frc
		trig_enc = _vdoenc_builtin_frc_is_select(pathID, &(p_obj->frc));

		if (trig_enc) {
			jpgenc_param.job_id = 0;
			jpgenc_param.chn_id = pathID;
			jpgenc_param.y_addr = yuv_info.y_addr;
			jpgenc_param.c_addr = yuv_info.c_addr;
			jpgenc_param.y_addr_pa = nvtmpp_sys_va2pa(yuv_info.y_addr);
			jpgenc_param.uv_addr_pa = nvtmpp_sys_va2pa(yuv_info.c_addr);
			jpgenc_param.y_line_offset = yuv_info.y_line_offset;
			jpgenc_param.c_line_offset = yuv_info.c_line_offset;
			jpgenc_param.src_ddr_id = 0;
			jpgenc_param.bs_addr_va[0] = p_obj->venc_param.bs_addr_1;
			jpgenc_param.bs_addr_pa[0] = nvtmpp_sys_va2pa(p_obj->venc_param.bs_addr_1);
			jpgenc_param.bs_buf_size[0] = p_obj->venc_param.bs_end_addr - p_obj->venc_param.bs_addr_1;
			jpgenc_param.bs_addr_va[1] = 0;
			jpgenc_param.bs_addr_pa[1] = 0;
			jpgenc_param.bs_buf_size[1] = 0;
			jpgenc_param.bs_ddr_id = 0;
			jpgenc_param.quality = jpeg_quality[pathID];
			jpgenc_param.retstart_interval = 0;
			jpgenc_param.encode_width = width;
			jpgenc_param.encode_height = height;
			jpgenc_param.in_fmt = 1;
			jpgenc_param.st_src_decompression.enable = gvdoenc_dtsi_param[pathID].ycc_en;
			jpgenc_param.st_src_decompression.width = gvdoenc_dtsi_param[pathID].ycc_width;
			jpgenc_param.st_src_decompression.height = gvdoenc_dtsi_param[pathID].ycc_height;
			jpgenc_param.st_src_decompression.y_lofst = gvdoenc_dtsi_param[pathID].ycc_yloff;
			jpgenc_param.st_src_decompression.c_lofst = gvdoenc_dtsi_param[pathID].ycc_cloff;

			if (fastboot_osg_setup_jpeg_stamp(&jpgenc_param, pathID)){
				DBG_ERR("fail to render jpeg stamp for jpeg path(%u)\n", pathID);
				return;
			}

			ret = jpeg_add_queue(pathID ,JPEG_CODEC_MODE_ENC,(void *)(&jpgenc_param), 0, 0);

			if (ret == E_OK) {
				p_obj->venc_param.bs_size_1 = jpgenc_param.bs_buf_size[0];
				p_obj->venc_param.base_qp = jpgenc_param.quality;
				p_obj->venc_param.frm_type = 3;//jpgenc_param.frm_type;
				p_obj->venc_param.timestamp = yuv_info.timestamp;
				VdoEnc_Builtin_PutBS(pathID, is_keyfrm, &(p_obj->venc_param), &sResult);
				p_obj->venc_param.bs_addr_1 += ALIGN_CEIL_64(p_obj->venc_param.bs_size_1);
			}
		}
	}

	if (yuv_info.release_flag) {
		if (yuv_info.enable) {
			DBG_IND("%s_3 0x%08x\r\n",__func__,yuv_info.y_addr);
			nvtmpp_unlock_fastboot_blk(yuv_info.y_addr);
		}
		yuv_unlock_cnt[pathID]++;
		if (yuv_unlock_cnt[pathID] == 2) {
			builtin_stop_encode[pathID] = TRUE;
			set_flg(FLG_ID_VDOENC_BUILTIN_CHECK[pathID], FLG_VDOENC_BUILTIN_DONE);
		}
	}

	return;
}

THREAD_DECLARE(VdoEnc_BuiltIn_Tsk_H26X, arglist) //static void VdoEnc_BuiltIn_Tsk_H26X(void)
{
	FLGPTN			uiFlag = 0;
	UINT32			pathID = 0;
	UINT32			codectype = 0;

	THREAD_ENTRY(); //kent_tsk();

	clr_flg(FLG_ID_VDOENC_BUILTIN_H26X, FLG_VDOENC_BUILTIN_IDLE);
	wai_flg(&uiFlag, FLG_ID_VDOENC_BUILTIN_H26X, FLG_VDOENC_BUILTIN_INIT_DONE, TWF_ORW | TWF_CLR);

	// coverity[no_escape]
	while (!THREAD_SHOULD_STOP) {
		set_flg(FLG_ID_VDOENC_BUILTIN_H26X, FLG_VDOENC_BUILTIN_IDLE);

		PROFILE_TASK_IDLE();
		wai_flg(&uiFlag, FLG_ID_VDOENC_BUILTIN_H26X, FLG_VDOENC_BUILTIN_ENCODE | FLG_VDOENC_BUILTIN_STOP, TWF_ORW | TWF_CLR);
		PROFILE_TASK_BUSY();

		clr_flg(FLG_ID_VDOENC_BUILTIN_H26X, FLG_VDOENC_BUILTIN_IDLE);

		if (uiFlag & FLG_VDOENC_BUILTIN_STOP) {
			break;
		}

		if (uiFlag & FLG_VDOENC_BUILTIN_ENCODE) {
			if (_VdoEnc_builtin_GetJobCount_H26X() > 0) {
				_VdoEnc_builtin_GetJob_H26X(&pathID);
				codectype = gvdoenc_dtsi_param[pathID].codectype;
				clr_flg(FLG_ID_VDOENC_BUILTIN_CHECK[pathID], FLG_VDOENC_BUILTIN_DONE);
				if (codectype == BUILTIN_VDOENC_H265) {
					H265Enc_builtin_encodeOne(pathID, 0);
				} else if (codectype == BUILTIN_VDOENC_H264) {
					H264Enc_builtin_encodeOne(pathID, 0);
				}
			}

			if (_VdoEnc_builtin_GetJobCount_H26X() > 0) { // continue to decode
				set_flg(FLG_ID_VDOENC_BUILTIN_H26X, FLG_VDOENC_BUILTIN_ENCODE);
			}
		}
	} // end of while loop

	set_flg(FLG_ID_VDOENC_BUILTIN_H26X, FLG_VDOENC_BUILTIN_STOP_DONE);

	THREAD_RETURN(0);
}

int VdoEnc_BuiltIn_TskStart_H26X(void)
{
	// start tsk
	THREAD_CREATE(VDOENC_BUILTIN_TSK_ID_H26X, VdoEnc_BuiltIn_Tsk_H26X, NULL, "VdoEnc_BuiltIn_Tsk_H26X"); //sta_tsk(VDOENC_BUILTIN_TSK_ID_H26X, 0);
	if (VDOENC_BUILTIN_TSK_ID_H26X == 0) {
		DBG_ERR("Invalid VDOENC_BUILTIN_TSK_ID_H26X\r\n");
		return -1;
	}
	THREAD_SET_PRIORITY(VDOENC_BUILTIN_TSK_ID_H26X, VDOENC_BUILTIN_TSK_PRI);
	THREAD_RESUME(VDOENC_BUILTIN_TSK_ID_H26X);
	DBG_IND("[VDOENC_BUILTIN] video encoding task start...\r\n");

	return 0;
}

int VdoEnc_BuiltIn_TskStop_H26X(void)
{
	FLGPTN uiFlag;

	if (FLG_ID_VDOENC_BUILTIN_H26X == 0) {
		return E_OK;
	}

	DBG_DUMP("[VDOENC_BUILTIN]tskstop wait idle ..\r\n");
	wai_flg(&uiFlag, FLG_ID_VDOENC_BUILTIN_H26X, FLG_VDOENC_BUILTIN_IDLE, TWF_ORW);
	DBG_DUMP("[VDOENC_BUILTIN]tskstop wait idle OK\r\n");

	// close decoder
	/*if (gNMPVdoDecObj[pathID].pMpVideoDecoder->SetInfo) {
		(gNMPVdoDecObj[pathID].pMpVideoDecoder->SetInfo)(MP_VDODEC_SETINFO_CLOSEBUS, 0, 0, 0);
	} else {
		DBG_ERR("MP decoder is NULL\r\n");
	}*/

//#if __KERNEL__
#if 1
	set_flg(FLG_ID_VDOENC_BUILTIN_H26X, FLG_VDOENC_BUILTIN_STOP);
	wai_flg(&uiFlag, FLG_ID_VDOENC_BUILTIN_H26X, FLG_VDOENC_BUILTIN_STOP_DONE, TWF_ORW | TWF_CLR);
#endif
	// Terminate Task
	//THREAD_DESTROY(NMP_VDODEC_TSK_ID); //ter_tsk(NMP_VDODEC_TSK_ID);

	return E_OK;
}

THREAD_DECLARE(VdoEnc_BuiltIn_Tsk_JPEG, arglist) //static void VdoEnc_BuiltIn_Tsk_JPEG(void)
{
	FLGPTN			uiFlag = 0;
	UINT32			pathID = 0;

	THREAD_ENTRY(); //kent_tsk();

	clr_flg(FLG_ID_VDOENC_BUILTIN_JPEG, FLG_VDOENC_BUILTIN_IDLE);
	wai_flg(&uiFlag, FLG_ID_VDOENC_BUILTIN_JPEG, FLG_VDOENC_BUILTIN_INIT_DONE, TWF_ORW | TWF_CLR);

	// coverity[no_escape]
	while (!THREAD_SHOULD_STOP) {
		set_flg(FLG_ID_VDOENC_BUILTIN_JPEG, FLG_VDOENC_BUILTIN_IDLE);

		PROFILE_TASK_IDLE();
		wai_flg(&uiFlag, FLG_ID_VDOENC_BUILTIN_JPEG, FLG_VDOENC_BUILTIN_ENCODE | FLG_VDOENC_BUILTIN_STOP, TWF_ORW | TWF_CLR);
		PROFILE_TASK_BUSY();

		clr_flg(FLG_ID_VDOENC_BUILTIN_JPEG, FLG_VDOENC_BUILTIN_IDLE);

		if (uiFlag & FLG_VDOENC_BUILTIN_STOP) {
			break;
		}

		if (uiFlag & FLG_VDOENC_BUILTIN_ENCODE) {
			if (_VdoEnc_builtin_GetJobCount_JPEG() > 0) {
				_VdoEnc_builtin_GetJob_JPEG(&pathID);
				clr_flg(FLG_ID_VDOENC_BUILTIN_CHECK[pathID], FLG_VDOENC_BUILTIN_DONE);
				MJPGEnc_builtin_encodeOne(pathID);
			}

			if (_VdoEnc_builtin_GetJobCount_JPEG() > 0) { // continue to decode
				set_flg(FLG_ID_VDOENC_BUILTIN_JPEG, FLG_VDOENC_BUILTIN_ENCODE);
			}
		}
	} // end of while loop

	set_flg(FLG_ID_VDOENC_BUILTIN_JPEG, FLG_VDOENC_BUILTIN_STOP_DONE);

	THREAD_RETURN(0);
}

int VdoEnc_BuiltIn_TskStart_JPEG(void)
{
	// start tsk
	THREAD_CREATE(VDOENC_BUILTIN_TSK_ID_JPEG, VdoEnc_BuiltIn_Tsk_JPEG, NULL, "VdoEnc_BuiltIn_Tsk_JPEG"); //sta_tsk(VDOENC_BUILTIN_TSK_ID_JPEG, 0);
	if (VDOENC_BUILTIN_TSK_ID_JPEG == 0) {
		DBG_ERR("Invalid VDOENC_BUILTIN_TSK_ID_JPEG\r\n");
		return -1;
	}
	THREAD_SET_PRIORITY(VDOENC_BUILTIN_TSK_ID_JPEG, VDOENC_BUILTIN_TSK_PRI);
	THREAD_RESUME(VDOENC_BUILTIN_TSK_ID_JPEG);
	DBG_IND("[VDOENC_BUILTIN] video encoding task start...\r\n");

	return 0;
}

int VdoEnc_BuiltIn_TskStop_JPEG(void)
{
	FLGPTN uiFlag;

	if (FLG_ID_VDOENC_BUILTIN_JPEG == 0) {
		return E_OK;
	}

	DBG_DUMP("[VDOENC_BUILTIN]tskstop wait idle ..\r\n");
	wai_flg(&uiFlag, FLG_ID_VDOENC_BUILTIN_JPEG, FLG_VDOENC_BUILTIN_IDLE, TWF_ORW);
	DBG_DUMP("[VDOENC_BUILTIN]tskstop wait idle OK\r\n");

	// close decoder
	/*if (gNMPVdoDecObj[pathID].pMpVideoDecoder->SetInfo) {
		(gNMPVdoDecObj[pathID].pMpVideoDecoder->SetInfo)(MP_VDODEC_SETINFO_CLOSEBUS, 0, 0, 0);
	} else {
		DBG_ERR("MP decoder is NULL\r\n");
	}*/

//#if __KERNEL__
#if 1
	set_flg(FLG_ID_VDOENC_BUILTIN_JPEG, FLG_VDOENC_BUILTIN_STOP);
	wai_flg(&uiFlag, FLG_ID_VDOENC_BUILTIN_JPEG, FLG_VDOENC_BUILTIN_STOP_DONE, TWF_ORW | TWF_CLR);
#endif
	// Terminate Task
	//THREAD_DESTROY(NMP_VDODEC_TSK_ID); //ter_tsk(NMP_VDODEC_TSK_ID);

	return E_OK;
}


// install task
void VdoEnc_BuiltIn_Install_ID(void)
{
	UINT32 i = 0;

	OS_CONFIG_FLAG(FLG_ID_VDOENC_BUILTIN_H26X);
	OS_CONFIG_FLAG(FLG_ID_VDOENC_BUILTIN_JPEG);
	for (i = 0; i < BUILTIN_VDOENC_PATH_ID_MAX; i++) {
		OS_CONFIG_FLAG(FLG_ID_VDOENC_BUILTIN_CHECK[i]);
	}
	OS_CONFIG_SEMPHORE(VDOENC_BUILTIN_BS_SEM_ID, 0, 1, 1);
}

void VdoEnc_BuiltIn_Uninstall_ID(void)
{
	UINT32 i = 0;

	if (FLG_ID_VDOENC_BUILTIN_H26X) {
		rel_flg(FLG_ID_VDOENC_BUILTIN_H26X);
	}
	if (FLG_ID_VDOENC_BUILTIN_JPEG) {
		rel_flg(FLG_ID_VDOENC_BUILTIN_JPEG);
	}
	for (i = 0; i < BUILTIN_VDOENC_PATH_ID_MAX; i++) {
		if (FLG_ID_VDOENC_BUILTIN_CHECK[i]) {
			rel_flg(FLG_ID_VDOENC_BUILTIN_CHECK[i]);
		}
	}
	if (VDOENC_BUILTIN_BS_SEM_ID) {
		SEM_DESTROY(VDOENC_BUILTIN_BS_SEM_ID);
	}
}

#ifdef __KERNEL__
extern NVT_API_CHK_DECLARE(vdoenc_builtin);
#endif

int  VdoEnc_builtin_init(VDOENC_BUILTIN_INIT_INFO *p_info)
{
	UINT32 uiBufSize = 0;
	PVDOENC_BUILTIN_JOBQ pJobQ26X = &gVdoEncBuiltinJobQ_H26X;
#ifdef __KERNEL__
	// enable jpeg encode in builtin linux
	PVDOENC_BUILTIN_JOBQ pJobQJPEG = &gVdoEncBuiltinJobQ_JPEG;
#endif
	UINT32 i = 0, j = 0;
	UINT32 en_h26x_int_fisrt = en_lnx_h26x_int_fisrt >> 16 & 0xFFFF;
	UINT32 en_h26x_int_pathid = en_lnx_h26x_int_fisrt & 0xFFFF;

#ifdef __KERNEL__
	if(NVT_API_CHK_CALL(vdoenc_builtin) != TRUE){
		DBG_ERR("objver mismatch %s\r\n", "vdoenc_builtin");
		return -1;
	}
#endif

	VdoEnc_BuiltIn_Install_ID();
	VdoEnc_BuiltIn_TskStart_H26X();

	VdoEnc_BuiltIn_TskStart_JPEG();

#ifdef __KERNEL__
	if (bPreInit == 0) {
		enc_var = (H26XENC_VAR *)vdoenc_builtin_alloc(sizeof(H26XENC_VAR) * BUILTIN_VDOENC_PATH_ID_MAX);
		gVdoEncBuiltinObj = (VDOENC_BUILTIN_OBJ *)vdoenc_builtin_alloc(sizeof(VDOENC_BUILTIN_OBJ) * BUILTIN_VDOENC_PATH_ID_MAX);
		memset(gVdoEncBuiltinObj, 0, sizeof(VDOENC_BUILTIN_OBJ) * BUILTIN_VDOENC_PATH_ID_MAX);

		for (i = 0; i < BUILTIN_VDOENC_PATH_ID_MAX; i++) {
			gVdoEncBuiltinObj[i].BeginTag = MAKEFOURCC('O', 'B', 'J', 'B');
			gVdoEncBuiltinObj[i].EndTag = MAKEFOURCC('O', 'B', 'J', 'E');
			DBG_IND("VdoEnc_builtin_init gVdoEncBuiltinObj[%d] BeginTag=0x%x, EndTag=0x%x\r\n", i, gVdoEncBuiltinObj[i].BeginTag, gVdoEncBuiltinObj[i].EndTag);
		}
	}
#else
//   do nvt_vdocdc_drv_init() in VdoEnc_builtin_init_lite
//	#if 0
//	nvt_vdocdc_drv_init(1);
//	#else
//	nvt_vdocdc_drv_init();
//	#endif
	enc_var = (H26XENC_VAR *)vdoenc_builtin_alloc(sizeof(H26XENC_VAR) * BUILTIN_VDOENC_PATH_ID_MAX);
	memset(enc_var, 0, sizeof(H26XENC_VAR) * BUILTIN_VDOENC_PATH_ID_MAX);
	gVdoEncBuiltinObj = (VDOENC_BUILTIN_OBJ *)vdoenc_builtin_alloc(sizeof(VDOENC_BUILTIN_OBJ) * BUILTIN_VDOENC_PATH_ID_MAX);
	memset(gVdoEncBuiltinObj, 0, sizeof(VDOENC_BUILTIN_OBJ) * BUILTIN_VDOENC_PATH_ID_MAX);
	for (i = 0; i < BUILTIN_VDOENC_PATH_ID_MAX; i++) {
		gVdoEncBuiltinObj[i].BeginTag = MAKEFOURCC('O', 'B', 'J', 'B');
		gVdoEncBuiltinObj[i].EndTag = MAKEFOURCC('O', 'B', 'J', 'E');
		DBG_IND("VdoEnc_builtin_init gVdoEncBuiltinObj[%d] BeginTag=0x%x, EndTag=0x%x\r\n", i, gVdoEncBuiltinObj[i].BeginTag, gVdoEncBuiltinObj[i].EndTag);
	}
#endif

	uiBufSize = sizeof(VDOENC_BUILTIN_JOB_INFO) * VDOENC_BUILTIN_JOBQ_MAX;
	if (uiBufSize == 0) {
		DBG_ERR("[VDOBUILTIN] JobQue size fail\r\n");
		return -1;
	}
	pJobQ26X->Queue = (VDOENC_BUILTIN_JOB_INFO *)vdoenc_builtin_alloc(uiBufSize);
	if (pJobQ26X->Queue == NULL) {
		DBG_ERR("[VDOBUILTIN] AllocJobQueMem fail\r\n");
		return -1;
	}

#ifdef __KERNEL__
	// enable jpeg encode in builtin linux
	pJobQJPEG->Queue = (VDOENC_BUILTIN_JOB_INFO *)vdoenc_builtin_alloc(uiBufSize);
	if (pJobQJPEG->Queue == NULL) {
		DBG_ERR("[VDOBUILTIN] AllocJobQueMem fail\r\n");
		return -1;
	}
#endif

	memset((void *)gvdoenc_dtsi_param, 0, sizeof(VDOENC_BUILTIN_DTSI_PARAM) * BUILTIN_VDOENC_PATH_ID_MAX);
	memset((void *)gVdoEnc_en, 0, sizeof(UINT32) * BUILTIN_VDOENC_PATH_ID_MAX);

	for (i = 0; i < BUILTIN_VDOENC_PATH_ID_MAX; i++) {
		gVdoEnc_en[i] = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_ENC_EN);
		if (gVdoEnc_en[i] == 1) {
			gvdoenc_dtsi_param[i].codectype = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_CODECTYPE);
			gvdoenc_dtsi_param[i].vprc_src_dev = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_SRC_VPRC_DEV);
			gvdoenc_dtsi_param[i].vprc_src_path = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_SRC_VPRC_PATH);
			gvdoenc_dtsi_param[i].width = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_WIDTH);
			gvdoenc_dtsi_param[i].height = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_HEIGHT);
			gvdoenc_dtsi_param[i].ycc_en = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_YCC_EN);
			gvdoenc_dtsi_param[i].ycc_width = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_YCC_WIDTH);
			gvdoenc_dtsi_param[i].ycc_height = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_YCC_HEIGHT);
			gvdoenc_dtsi_param[i].ycc_yloff = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_YCC_YLOFF);
			gvdoenc_dtsi_param[i].ycc_cloff = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_YCC_CLOFF);
			if (gvdoenc_dtsi_param[i].codectype == BUILTIN_VDOENC_H265) {
				gvdoenc_dtsi_param[i].isp_id = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_ISP_ID);
				gvdoenc_dtsi_param[i].skip_frm_en = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_SKIP_FRM_EN);
				gvdoenc_dtsi_param[i].skip_frm_target_fr = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_SKIP_FRM_TARGET_FR);
				gvdoenc_dtsi_param[i].skip_frm_input_cnt = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_SKIP_FRM_INPUT_CNT);
				H265Enc_builtin_init(i, p_info);
				if (en_h26x_int_fisrt && (en_h26x_int_pathid == i)) {
					H265Enc_builtin_encodeOne(i, en_h26x_int_fisrt);
				}
				if (bPreInit == 0) {
					VdoEnc_builtin_set_rate_control(i);
				}
				for (j = 0; j < rtos_yuv_num[i]; j++) {
					_VdoEnc_builtin_PutJob_H26X(i);
					iset_flg(FLG_ID_VDOENC_BUILTIN_H26X, FLG_VDOENC_BUILTIN_ENCODE);
				}
			} else if (gvdoenc_dtsi_param[i].codectype == BUILTIN_VDOENC_H264) {
				gvdoenc_dtsi_param[i].isp_id = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_ISP_ID);
				gvdoenc_dtsi_param[i].skip_frm_en = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_SKIP_FRM_EN);
				gvdoenc_dtsi_param[i].skip_frm_target_fr = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_SKIP_FRM_TARGET_FR);
				gvdoenc_dtsi_param[i].skip_frm_input_cnt = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_SKIP_FRM_INPUT_CNT);
				H264Enc_builtin_init(i, p_info);
				if (en_h26x_int_fisrt && (en_h26x_int_pathid == i)) {
					H264Enc_builtin_encodeOne(i, en_h26x_int_fisrt);
				}
				if (bPreInit == 0) {
					VdoEnc_builtin_set_rate_control(i);
				}
				for (j = 0; j < rtos_yuv_num[i]; j++) {
					_VdoEnc_builtin_PutJob_H26X(i);
					iset_flg(FLG_ID_VDOENC_BUILTIN_H26X, FLG_VDOENC_BUILTIN_ENCODE);
				}
			} else if (gvdoenc_dtsi_param[i].codectype == BUILTIN_VDOENC_MJPEG) {
#ifdef __KERNEL__
				// enable jpeg encode in builtin linux
				// jpeg_open();
				MJPGEnc_builtin_init(i, p_info);
#endif
			}
		}
	}
	set_flg(FLG_ID_VDOENC_BUILTIN_H26X, FLG_VDOENC_BUILTIN_INIT_DONE);
	set_flg(FLG_ID_VDOENC_BUILTIN_JPEG, FLG_VDOENC_BUILTIN_INIT_DONE);
#ifdef __KERNEL__
	kdrv_ipp_builtin_reg_fmd_cb(VdoEnc_BuiltIn_trig);
#endif

	return 0;
}

#if defined(__FREERTOS)
int  VdoEnc_builtin_init_lite(VDOENC_BUILTIN_INIT_INFO *p_info)
{
	H26XENC_VAR enc_var_lite = {0};
	VDOENC_BUILTIN_OBJ venc_obj_lite = {0};
	nvt_vdocdc_drv_init(); //TODO
	H265Enc_builtin_init_lite(p_info, &enc_var_lite, &venc_obj_lite);
	H265Enc_builtin_encodeOne_lite(&enc_var_lite, &venc_obj_lite);

	return 0;
}
#endif
