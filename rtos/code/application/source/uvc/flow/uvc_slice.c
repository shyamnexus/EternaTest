#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <hdal.h>
#include <kwrap/perf.h>
//#include <kwrap/debug.h>
#include <kwrap/util.h>
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h>
#include "prjcfg.h"
#include "UVAC.h"
#include "vendor_videoprocess.h"

#define ISF_LATENCY_DEBUG 0
#define UVAC_WAIT_RELEASE 1
#define PERF_TEST         0

#define DBG_WRN(fmtstr, args...) printf("\033[33mWRN:%s(): \033[0m" fmtstr,__func__, ##args)
#define DBG_ERR(fmtstr, args...) printf("\033[31mERR:%s(): \033[0m" fmtstr,__func__, ##args)
#define DBG_DUMP(fmtstr, args...) printf(fmtstr, ##args)

#if 0
#define DBG_IND(fmtstr, args...)  printf("%s(): " fmtstr, __func__, ##args)
#else
#define DBG_IND(fmtstr, args...)
#endif

typedef struct {
	UINT64          frame_count;
	UINT64          timestamp;
	BOOL            slice_start;
	UINT32          slice_processed_h;      // for slice colortrans
	UINT8           slice_cnt;
	UINT8           slice_idx;
	UINT8           prev_slice_idx;
} UVC_SLICE_INFO;


UVC_SLICE_INFO g_slice_info = {0};


#if 0
HD_RESULT yuv_transform(UINT32 buf_va, UINT32 buf_size, HD_DIM *main_dim)
{
	int                       fd;
	HD_GFX_COLOR_TRANSFORM    param;
	void                      *va;
	HD_RESULT                 ret;
	HD_VIDEO_FRAME            frame;
	UINT32                    src_size, dst_size, temp_size;
	int                       len;
	int                       w = 0, h = 0;
	UINT32                    src_pa, src_va, dst_pa, dst_va, temp_pa;
	static UINT32             gfx_qcnt = 0;

	UVAC_STRM_FRM      strmFrm = {0};

	w = main_dim->w;
	h = main_dim->h;
	//calculate yuv420 image's buffer size
	frame.sign = MAKEFOURCC('V','F','R','M');
	frame.pxlfmt  = HD_VIDEO_PXLFMT_YUV420;
	frame.dim.h   = h;
	frame.loff[0] = w;
	frame.loff[1] = w;
	src_size = hd_common_mem_calc_buf_size(&frame);
	if(!src_size){
		printf("hd_common_mem_calc_buf_size() fail to calculate src size\n");
		return -1;
	}

	//calculate yuyv image's buffer size
	frame.sign = MAKEFOURCC('V','F','R','M');
	frame.pxlfmt  = HD_VIDEO_PXLFMT_YUV422_ONE;
	frame.dim.h   = h;
	frame.loff[0] = w;
	dst_size = hd_common_mem_calc_buf_size(&frame);
	if(!dst_size){
		printf("hd_common_mem_calc_buf_size() fail to calculate yuv size\n");
		return -1;
	}
	temp_size = dst_size;

	if((src_size + dst_size + temp_size) > gfx[gfx_qcnt].buf_size){
		printf("required size(%d) > allocated size(%d)\n", (src_size + dst_size + temp_size), gfx[gfx_qcnt].buf_size);
		return -1;
	}

	src_pa  = gfx[gfx_qcnt].buf_pa;
	dst_pa  = src_pa + src_size;
	temp_pa = dst_pa + dst_size;

	// cpoy video stream to gfx buffer
	if (gfx[gfx_qcnt].buf_va){
		memcpy((void*)gfx[gfx_qcnt].buf_va,(const void *)buf_va, src_size);//gfx[gfx_qcnt].buf_size);
	} else {
		printf("gfx[%d].buf_va NULL!\n, gfx_qcnt");
		return -1;
	}

	src_va  = gfx[gfx_qcnt].buf_va;
	dst_va  = src_va + src_size;

	//use gfx engine to transform yuv420 image to yuyv
	memset(&param, 0, sizeof(HD_GFX_COLOR_TRANSFORM));
	param.src_img.dim.w            = w;
	param.src_img.dim.h            = h;
	param.src_img.format           = HD_VIDEO_PXLFMT_YUV420;
	param.src_img.p_phy_addr[0]    = src_pa;
	param.src_img.p_phy_addr[1]    = src_pa + w*h;
	param.src_img.lineoffset[0]    = w;
	param.src_img.lineoffset[1]    = w;
	param.dst_img.dim.w            = w;
	param.dst_img.dim.h            = h;
	param.dst_img.format           = HD_VIDEO_PXLFMT_YUV422_ONE;
	param.dst_img.p_phy_addr[0]    = dst_pa;
	param.dst_img.lineoffset[0]    = w * 2;
	param.p_tmp_buf                = temp_pa;
	param.tmp_buf_size             = temp_size;

	ret = hd_gfx_color_transform(&param);
	if(ret != HD_OK){
		printf("hd_gfx_color_transform fail=%d\n", ret);
		goto exit;
	}

#if UVC_ON
	strmFrm.path = UVAC_STRM_VID ;
	strmFrm.addr = dst_pa;
	strmFrm.size = dst_size;
	strmFrm.pStrmHdr = 0;
	strmFrm.strmHdrSize = 0;
	strmFrm.va = dst_va;
	if (stream[0].codec_type == HD_CODEC_TYPE_RAW){
		UVAC_SetEachStrmInfo(&strmFrm);

		#if UVAC_WAIT_RELEASE
		UVAC_WaitStrmDone(UVAC_STRM_VID);
		#endif
	}
	gfx_qcnt++;
	if (gfx_qcnt >= GFX_QUEUE_MAX_NUM)
		gfx_qcnt = 0; // This state machine does not consider queue overwrite issue.
#endif
	ret = HD_OK;

exit:
	return ret;
}
#endif
static void yuv_slice_push_finalpart_to_usb(UINT32 buf_va, HD_VIDEO_FRAME *pVideoFrame)
{
	UVAC_STRM_FRM         strmFrm = {0};
	UVC_SLICE_INFO       *pObj = &g_slice_info;
	UINT32                proc_h, image_h;

	image_h = pVideoFrame->dim.h;
	proc_h = image_h - pObj->slice_processed_h;
	if (proc_h == 0) {
		return;
	}
	strmFrm.frame_type = UVAC_VIDEO_FRM_FINAL;
	strmFrm.path = UVAC_STRM_VID;
	strmFrm.addr = pVideoFrame->phy_addr[0] + (pVideoFrame->loff[0] * pObj->slice_processed_h);
	strmFrm.size = pVideoFrame->loff[0] * proc_h;
	strmFrm.pStrmHdr = 0;
	strmFrm.strmHdrSize = 0;
	strmFrm.va = buf_va + (pVideoFrame->loff[0] * pObj->slice_processed_h);
	if (pVideoFrame->timestamp) {
		strmFrm.timestamp = pVideoFrame->timestamp;
	} else {
		strmFrm.timestamp = pObj->timestamp;
	}

	DBG_IND("[slice]Final. proc_h=%03d, prced_h=%03d, size = %d, prv_idx=%d, ms=%ld\r\n",
			proc_h, pObj->slice_processed_h, strmFrm.size, pObj->prev_slice_idx, hd_gettime_ms());
	pObj->prev_slice_idx = 0;
	pObj->slice_processed_h += proc_h;
	pObj->slice_start = FALSE;
	UVAC_SetEachStrmInfo(&strmFrm);
	UVAC_WaitStrmDone(UVAC_STRM_VID);
}

// return true means last slice
static int yuv_slice_push_to_usb(UINT32 buf_va, HD_VIDEO_FRAME *pVideoFrame)
{
	UVC_SLICE_INFO        *pObj = &g_slice_info;
	BOOL                  isFinalpart = FALSE;
	//UINT32                first_slice_h = pVideoFrame->dim.h/pObj->slice_cnt;
	UINT32                first_slice_h = (pVideoFrame->dim.h/pObj->slice_cnt)/2;
	UINT32                slice_h = ((pVideoFrame->dim.h - first_slice_h) << 4 )/ (pObj->slice_cnt-1);
	UINT32                proc_h, image_h;
	UVAC_STRM_FRM         strmFrm = {0};

	image_h = pVideoFrame->dim.h;
	//DBG_IND("[colortrans] p_src_img addr=0x%x, p_dst_img addr=0x%x\r\n", p_src_img->PxlAddr[0], p_dst_img->PxlAddr[0]);
	if (pObj->slice_idx >= pObj->slice_cnt) {
		DBG_ERR("[slice] slice_idx=%d, slice_cnt=%d\r\n", pObj->slice_idx, pObj->slice_cnt);
		return -1;
	}
	if (pObj->slice_start == FALSE) {
		if (pObj->slice_idx + 1 == pObj->slice_cnt) {
			proc_h = image_h;
		} else {
			proc_h = ALIGN_FLOOR_4(first_slice_h + ((slice_h* pObj->slice_idx) >> 4));
		}
		#if 0
		if (proc_h > image_h) {
			DBG_ERR("[colortrans] First slice proc_h=%d, image_h = %d, slice_idx = %d, prev_slice=%d\r\n",
			proc_h, image_h, pObj->slice_idx, pObj->prev_slice_idx);
			while (1);
		}
		#endif
		if (pObj->slice_idx + 1 == pObj->slice_cnt) {
			strmFrm.frame_type = UVAC_VIDEO_FRM_NORMAL;
		} else {
			strmFrm.frame_type = UVAC_VIDEO_FRM_FIRST;
			pObj->slice_start = TRUE;
			pObj->slice_processed_h = 0;
		}
		pObj->frame_count = pVideoFrame->count;
		pObj->timestamp = pVideoFrame->timestamp;
		strmFrm.path = UVAC_STRM_VID;
		strmFrm.addr = pVideoFrame->phy_addr[0];
		strmFrm.size = pVideoFrame->loff[0] * proc_h;
		strmFrm.pStrmHdr = 0;
		strmFrm.strmHdrSize = 0;
		strmFrm.va = buf_va;
		strmFrm.timestamp = pVideoFrame->timestamp;
		//printf("framecount = %lld, va = 0x%x, size = 0x%x, timestamp = %lld\r\n", pVideoFrame->count, strmFrm.va, strmFrm.size, strmFrm.timestamp);
		//printf("loff[0]= %d, loff[1]= %d, height = %d  \r\n", pVideoFrame->loff[0], pVideoFrame->loff[1], pVideoFrame->ph[0]);
		//printf("pa = 0x%x, size = 0x%x\r\n", strmFrm.addr, strmFrm.size);
		//DBG_IND("[slice]First proc_h=%d, idx = %d, prv_idx=%d, us=%lld\r\n",
			//proc_h, pObj->slice_idx, pObj->prev_slice_idx, hd_gettime_us());
		DBG_IND("[slice]First proc_h=%03d, prced_h=%03d, isFinal=%d, idx=%d, prv_idx=%d, ms=%d, size=%d\r\n",
			proc_h, pObj->slice_processed_h, isFinalpart, pObj->slice_idx, pObj->prev_slice_idx, hd_gettime_ms(), strmFrm.size);

	} else {
		if ((pObj->slice_idx <= pObj->prev_slice_idx) ||
			((UINT32)pVideoFrame->count != pObj->frame_count)) {
			DBG_ERR("[slice] slice_idx=%d, prev_slice_idx=%d, Serial =%d, frame_count=%d, us=%d\r\n",
				pObj->slice_idx, pObj->prev_slice_idx, pVideoFrame->count, pObj->frame_count, hd_gettime_ms());
			return -1;
		}
		if (pObj->slice_idx == pObj->slice_cnt - 1) {
			isFinalpart = TRUE;
		}
		if (isFinalpart) {
			proc_h = image_h - (pObj->slice_processed_h);
		} else {
			//proc_h = slice_h * (pObj->slice_idx - pObj->prev_slice_idx);
			proc_h = ALIGN_FLOOR_4(first_slice_h + ((slice_h* pObj->slice_idx) >> 4)) - (pObj->slice_processed_h);
		}
		if (proc_h == 0) {
			return 0;
		}
		#if 0
		if ((proc_h + first_slice_h + slice_h * pObj->prev_slice_idx) > image_h) {
			DBG_ERR("[colortrans] Cont. slice proc_h=%d, first_slice_h=%d, image_h = %d, slice_idx = %d, prev_slice=%d\r\n",
			proc_h, first_slice_h, image_h, pObj->slice_idx, pObj->prev_slice_idx);
			while (1);
		}
		#endif
		if (isFinalpart) {
			strmFrm.frame_type = UVAC_VIDEO_FRM_FINAL;
		} else {
			strmFrm.frame_type = UVAC_VIDEO_FRM_PARTIAL;
		}
		strmFrm.path = UVAC_STRM_VID;
		strmFrm.addr = pVideoFrame->phy_addr[0] + (pVideoFrame->loff[0] * pObj->slice_processed_h);
		strmFrm.size = pVideoFrame->loff[0] * proc_h;
		strmFrm.pStrmHdr = 0;
		strmFrm.strmHdrSize = 0;
		strmFrm.va = buf_va + (pVideoFrame->loff[0] * pObj->slice_processed_h);
		if (pVideoFrame->timestamp) {
			strmFrm.timestamp = pVideoFrame->timestamp;
		} else {
			strmFrm.timestamp = pObj->timestamp;
		}
		//printf("framecount = %lld, va = 0x%x, size = 0x%x, timestamp = %lld\r\n", pVideoFrame->count, strmFrm.va, strmFrm.size, strmFrm.timestamp);
		//printf("loff[0]= %d, loff[1]= %d, height = %d  \r\n", pVideoFrame->loff[0], pVideoFrame->loff[1], pVideoFrame->ph[0]);
		//printf("pa = 0x%x, size = 0x%x\r\n", strmFrm.addr, strmFrm.size);
		DBG_IND("[slice]Cont. proc_h=%03d, prced_h=%03d, isFinal=%d, idx=%d, prv_idx=%d, ms=%ld, size=%d\r\n",
			proc_h, pObj->slice_processed_h, isFinalpart, pObj->slice_idx, pObj->prev_slice_idx, hd_gettime_ms(), strmFrm.size);
	}
	UVAC_SetEachStrmInfo(&strmFrm);
	if (isFinalpart || strmFrm.frame_type == UVAC_VIDEO_FRM_NORMAL) {
		pObj->slice_start = FALSE;
		pObj->prev_slice_idx = 0;
	} else {
		pObj->prev_slice_idx = pObj->slice_idx;
	}
	#if 0
	{
		UINT32 t1, t2;

		t1 = hd_gettime_ms();
		UVAC_WaitStrmDone(UVAC_STRM_VID);
		t2 = hd_gettime_ms();
		DBG_IND("uvac wait %d ms\r\n", t2-t1);
	}
	#else
	UVAC_WaitStrmDone(UVAC_STRM_VID);
	#endif
	pObj->slice_processed_h += proc_h;
	return isFinalpart;
}

HD_RESULT yuv_frame_push_to_usb(UINT32 buf_va, HD_VIDEO_FRAME *pVideoFrame)
{
	UVAC_STRM_FRM      strmFrm = {0};

	#if 0
	if (pVideoFrame->pxlfmt != VDO_PXLFMT_YUV422_YVYU) {
		DBG_ERR("pxlfmt = 0x%x\r\n", pVideoFrame->pxlfmt);
		return HD_ERR_PARAM;
	}
	#endif
	strmFrm.path = UVAC_STRM_VID;
	strmFrm.addr = pVideoFrame->phy_addr[0];
	strmFrm.size = pVideoFrame->loff[0]*pVideoFrame->ph[0] + pVideoFrame->loff[1]*pVideoFrame->ph[1];
	strmFrm.pStrmHdr = 0;
	strmFrm.strmHdrSize = 0;
	strmFrm.va = buf_va;
	strmFrm.timestamp = pVideoFrame->timestamp;
	strmFrm.frame_type = UVAC_VIDEO_FRM_NORMAL;
	//printf("framecount = %lld, va = 0x%x, size = 0x%x, timestamp = %lld\r\n", pVideoFrame->count, strmFrm.va, strmFrm.size, strmFrm.timestamp);
	//printf("loff[0]= %d, loff[1]= %d, height = %d  \r\n", pVideoFrame->loff[0], pVideoFrame->loff[1], pVideoFrame->ph[0]);
	//printf("pa = 0x%x, size = 0x%x\r\n", strmFrm.addr, strmFrm.size);
	UVAC_SetEachStrmInfo(&strmFrm);
	UVAC_WaitStrmDone(UVAC_STRM_VID);
	return HD_OK;
}

HD_RESULT yuv_push_to_usb(UINT32 buf_va, HD_VIDEO_FRAME *pVideoFrame)
{
	UVC_SLICE_INFO        *pObj = &g_slice_info;

	if (pVideoFrame->reserved[1] == MAKEFOURCC('S', 'L', 'I', 'C')) {

		printf("slice???????????\r\n");
		#if 0
		pObj->slice_cnt = pVideoFrame->reserved[2] >> 16;
		pObj->slice_idx = pVideoFrame->reserved[2] & 0xFF;
		printf("slice_cnt = %d, slice_idx = %d, time =%lld us\r\n", pObj->slice_cnt, pObj->slice_idx, hd_gettime_us());
		return HD_OK;
		#endif
		if (pObj->slice_start && pVideoFrame->count != pObj->frame_count) {
			yuv_slice_push_finalpart_to_usb(buf_va, pVideoFrame);
		}
		pObj->slice_cnt = pVideoFrame->reserved[2] >> 16;
		pObj->slice_idx = pVideoFrame->reserved[2] & 0xFF;
		yuv_slice_push_to_usb(buf_va, pVideoFrame);
		return HD_OK;
	} else {
		return yuv_frame_push_to_usb(buf_va, pVideoFrame);
	}
}

HD_RESULT mjpg_slice_push_to_usb(UINT32 buf_va, HD_VIDEOENC_BS *p_bs)
{
	//UVC_SLICE_INFO     *pObj = &g_slice_info;
	UVAC_STRM_FRM      strmFrm = {0};
	UINT32             slice_cnt;

	if (!(p_bs->pack_num == 2 && p_bs->video_pack[1].phy_addr == MAKEFOURCC('S', 'L', 'I', 'C'))) {
		DBG_ERR("This is not slice mode\r\n");
	}
	slice_cnt = p_bs->video_pack[1].size;
	strmFrm.path = UVAC_STRM_VID ;
	strmFrm.addr = p_bs->video_pack[0].phy_addr;
	strmFrm.size = p_bs->video_pack[0].size;
	strmFrm.pStrmHdr = 0;
	strmFrm.strmHdrSize = 0;
	strmFrm.timestamp = p_bs->timestamp;
	strmFrm.va = buf_va;

	if (slice_cnt == 0) {
		strmFrm.frame_type = UVAC_VIDEO_FRM_FIRST;
	} else if (slice_cnt & 0x80000000) {
		strmFrm.frame_type = UVAC_VIDEO_FRM_FINAL;
	} else {
		strmFrm.frame_type = UVAC_VIDEO_FRM_PARTIAL;
	}
	//hd_common_mem_flush_cache((void *)strmFrm.va, strmFrm.size);
	//printf("slice_cnt =0x%08X, frame_type = %d, bs_addr = 0x%x bs_size = 0x%x\r\n", slice_cnt, strmFrm.frame_type, strmFrm.va, strmFrm.size);
	UVAC_SetEachStrmInfo(&strmFrm);
	#if UVAC_WAIT_RELEASE
	#if PERF_TEST
	VOS_TICK t1 = 0, t2 = 0;
	UINT32 diff_us;
	static UINT32 cnt = 0;
	vos_perf_mark(&t1);
	#endif
	UVAC_WaitStrmDone(UVAC_STRM_VID);
	#if PERF_TEST
	vos_perf_mark(&t2);
	if (cnt%15 == 0) {
		diff_us = vos_perf_duration(t1, t2);
		printf("%dKB %dus(%dKB/sec)\r\n", strmFrm.size/1024, diff_us, strmFrm.size*1000/1024*1000/diff_us);
	}
	cnt++;
	#endif
	#endif
	return HD_OK;
}

