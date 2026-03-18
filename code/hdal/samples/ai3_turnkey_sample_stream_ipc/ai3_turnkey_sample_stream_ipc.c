/**
    @brief Sample code of ai network with sensor input.\n

    @file ai3_turnkey_sample_stream_ipc.c

    @author iVOT/CVAI

    @ingroup mhdal

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2024.  All rights reserved.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <arm_neon.h>

#include "pvdcnn_lib.h"
#include "fdcnn_lib.h"
#include "sw_tracker.h"
#include "limit_fdet_lib.h"
#include "ext_module_api.h"
#include "sample_ipc_ext.h"

#include "pd_shm.h"
#include "vendor_videoenc.h"

#define HD_VIDEOENC_PATH(dev_id, in_id, out_id)	(((dev_id) << 16) | (((in_id) & 0x00ff) << 8)| ((out_id) & 0x00ff))

#define USE_MD      (0)
#define TK_DDR_ID   DDR_ID0

#define TURNKEY_VERSION      "017.0001" //implementation version
static char turnkey_ver[64] = "version="TURNKEY_VERSION;

#define TURNKEY_DBGLVL         7 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
///////////////////////////////////////////////////////////////////////////////
#define __MODULE__          __AI3_TURNKEY_LIB__
#define __DBGLVL__          TURNKEY_DBGLVL
#define __DBGFLT__          "*" //*=All, [mark]=CustomClass
#include "kwrap/debug.h"

// platform dependent
#if defined(__LINUX)
#include <pthread.h>            //for pthread API
#include <dirent.h>
#define MAIN(argc, argv)        int main(int argc, char** argv)
#define GETCHAR()               getchar()
#include <sys/ipc.h>
#include <sys/shm.h>
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <kwrap/util.h>     //for sleep API
#define sleep(x)                vos_util_delay_ms(1000*(x))
#define msleep(x)               vos_util_delay_ms(x)
#define usleep(x)               vos_util_delay_us(x)
#include <kwrap/examsys.h>  //for MAIN(), GETCHAR() API
#define MAIN(argc, argv)        EXAMFUNC_ENTRY(hd_video_liveview, argc, argv)
#endif

#define AI_IPC            ENABLE
#define SHOW_BBOX_IN_VLC  ENABLE
#define AI_NN_PROF        ENABLE
#define SET_TURNKEY_FRAME ENABLE

#define MAX_AIOB_NUM      (10)

#if (PVD_YUV_HEIGHT > 576)
#define AI_VDO_W            (1280)
#define AI_VDO_H            (720)
#else
#define AI_VDO_W            (1024)
#define AI_VDO_H            (576)
#endif

static UINT PVD_MAX_DISTANCE_MODE = 0;

static UINT32 AI_TOTAL_BUF = 0;
static BOOL PDCNN_MODE   = 0;
static BOOL FDCNN_MODE   = 0;
static BOOL CDDCNN_MODE  = 0;
static BOOL PVDCNN_MODE  = 0;
static BOOL FADCNN_MODE  = 0;

static BOOL sort_enable  = 0;
static UINT32 debug_mode = 0;
static BOOL save_results = 0;
static UINT fps_delay    = 0;

///////////////////////////////////////////////////////
static BOOL is_net_proc = TRUE;
static BOOL is_net_run  = FALSE;

static HD_COMMON_MEM_VB_BLK g_blk = 0;

static char parms_file[PATH_LENGTH_M] = {"/mnt/sd/configs/turnkey_para_config.txt"};

typedef struct _SUB_PROCESS_LIVEVIEW {
	HD_PATH_ID enc_path;
	HD_PATH_ID proc_alg_path;
} SUB_PROCESS_LIVEVIEW_S;

typedef struct _AI_SUB_PROCESS_THREAD_PARM {
	NN_CFG_BUF_M rslt_mem;
	NN_CFG_BUF_M pd_mem;
	NN_CFG_BUF_M cdd_mem;
	NN_CFG_BUF_M fd_mem;
	NN_CFG_BUF_M pvd_mem;
	NN_CFG_BUF_M limit_fdet_mem;
	NN_CFG_BUF_M md_mem;
	NN_CFG_BUF_M sw_tracker_mem;
	SUB_PROCESS_LIVEVIEW_S stream;
} AI_SUB_PROCESS_THREAD_PARM_S;

#if(AI_IPC)
///////////////////////////// (memory for final result) ///////////////////////////////
static char *g_shm = NULL;

void init_share_memory(void)
{
	int shmid = 0;
	key_t key;

	// Segment key.
	key = PD_SHM_KEY;
	// Create the segment.
	if ((shmid = shmget(key, PD_SHMSZ, 0666)) < 0) {
		perror("shmget");
		exit(1);
	}
	// Attach the segment to the data space.
	if ((g_shm = shmat(shmid, NULL, 0)) == (char *) - 1) {
		perror("shmat");
		exit(1);
	}

}

void exit_share_memory(void)
{
	shmdt(g_shm);
}
#endif

////////////////////////////// (AI Detection Info Integrate) ///////////////////////////////////
VOID ai_rslt_integrate_pvd(AI_RSLT_PARM *rslt, NN_RESULT_M *p_det_result, UINT32 pvd_out_num, UINT32 *box_border)
{
	NN_RESULT_M *final_results = (NN_RESULT_M *)rslt->rslt_mem.va;
	UINT32 i = 0;
	for (i = 0; i < pvd_out_num; i++) {
		switch(p_det_result[i].category){
			case 1:
				final_results[rslt->obj_num + i].category = 1;  //person
				break;
			case 2:
				final_results[rslt->obj_num + i].category = 4;  //vehicle
				break;
			case 3:
				final_results[rslt->obj_num + i].category = 5;  //non-motor
				break;
			default:
				continue;
		}
		final_results[rslt->obj_num + i].score = p_det_result[i].score;
		final_results[rslt->obj_num + i].x1 = p_det_result[i].x1;
		final_results[rslt->obj_num + i].y1 = p_det_result[i].y1;
		final_results[rslt->obj_num + i].x2 = p_det_result[i].x2;
		final_results[rslt->obj_num + i].y2 = p_det_result[i].y2;
		if (final_results[rslt->obj_num + i].x2 > (float)(box_border[0] - 1)) {
			final_results[rslt->obj_num + i].x2 = (float)(box_border[0] - 1);
		}
		if (final_results[rslt->obj_num + i].y2 > (float)(box_border[1] - 1)) {
			final_results[rslt->obj_num + i].y2 = (float)(box_border[1] - 1);
		}
	}
	rslt->obj_num += (unsigned long)pvd_out_num;
}

VOID ai_rslt_integrate_fd(AI_RSLT_PARM *rslt, FDCNN_RESULT *p_det_result, UINT32 fd_out_num, UINT32 *box_border)
{
	NN_RESULT_M *final_results = (NN_RESULT_M *)rslt->rslt_mem.va;
	UINT32 i = 0;
	for (i = 0; i < fd_out_num; i++) {
		final_results[rslt->obj_num + i].category = 2;
		final_results[rslt->obj_num + i].score = (float)p_det_result[i].score;
		final_results[rslt->obj_num + i].x1 = (float)p_det_result[i].x;
		final_results[rslt->obj_num + i].y1 = (float)p_det_result[i].y;
		final_results[rslt->obj_num + i].x2 = (float)(p_det_result[i].x + p_det_result[i].w);
		final_results[rslt->obj_num + i].y2 = (float)(p_det_result[i].y + p_det_result[i].h);
		if (final_results[rslt->obj_num + i].x2 > (float)(box_border[0] - 1)) {
			final_results[rslt->obj_num + i].x2 = (float)(box_border[0] - 1);
		}
		if (final_results[rslt->obj_num + i].y2 > (float)(box_border[1] - 1)) {
			final_results[rslt->obj_num + i].y2 = (float)(box_border[1] - 1);
		}
	}
	rslt->obj_num += (unsigned long)fd_out_num;
}

//////////////////////////////// (AI Detection Info To Draw BOX) /////////////////////////////////////////
HD_RESULT ai_rslt_trans_info(AI_RSLT_PARM *rslts, HD_PATH_ID enc_path, HD_DIM alg_vdo_size)
{
	HD_RESULT ret = HD_OK;

#if SHOW_BBOX_IN_VLC
	// update pdcnn result
	NN_RESULT_M* det_rslts = (NN_RESULT_M*)rslts->rslt_mem.va;
	UINT8 det_num = (UINT8)((rslts->obj_num > VENDOR_VIDEOENC_SMART_BBOX_MAX_NUM) ? VENDOR_VIDEOENC_SMART_BBOX_MAX_NUM : rslts->obj_num);

	VENDOR_VIDEOENC_SMART_BBOX smart_bbox = {0};
	smart_bbox.base_resolution.w = alg_vdo_size.w;
	smart_bbox.base_resolution.h = alg_vdo_size.h;
	for (UINT8 num = 0; num < det_num; num++) {
		smart_bbox.bbox[num].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_TOP_LEFT].x = (UINT32)det_rslts[num].x1;
		smart_bbox.bbox[num].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_TOP_LEFT].y = (UINT32)det_rslts[num].y1;
		smart_bbox.bbox[num].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_BOTTOM_RIGHT].x = (UINT32)det_rslts[num].x2;
		smart_bbox.bbox[num].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_BOTTOM_RIGHT].y = (UINT32)det_rslts[num].y2;
		// smart_bbox.bbox[num].class_id = det_rslts[num].category - 1;
		if (det_rslts[num].category == 1) {
			smart_bbox.bbox[num].class_id = 0;
		} else {
			smart_bbox.bbox[num].class_id = 1;  // Assign all other category num to 1
		}
	}
	smart_bbox.timestamp = 33333;
	smart_bbox.bbox_num = det_num;
	ret = vendor_videoenc_set(enc_path, VENDOR_VIDEOENC_PARAM_SMART_BBOX, &smart_bbox);
	if (ret != HD_OK) {
		printf("set smart bbox error=%d !!\r\n", ret);
	}
#else
	/* Deprecated RTSP draw method
	NN_RESULT_M* det_rslts = (NN_RESULT_M*)rslts->rslt_mem.va;
	PD_SHM_INFO *p_pd_shm = (PD_SHM_INFO *)g_shm;
	PD_SHM_RESULT *p_obj;

	if (p_pd_shm->exit) {
		return HD_ERR_NOT_SUPPORT;
	}
	printf("ai_rslt_trans_info\r\n");

	// update pdcnn result to share memory
	p_pd_shm->pd_num = rslts->obj_num;
	if (rslts->obj_num > 10) {
		p_pd_shm->pd_num = 10;
	}
	for (UINT32 i = 0; i < p_pd_shm->pd_num; i++) {
		p_obj = &p_pd_shm->pd_results[i];
		p_obj->score = det_rslts[i].score;
		p_obj->x1 = (int)(det_rslts[i].x1);
		p_obj->x2 = (int)(det_rslts[i].x2);
		p_obj->y1 = (int)(det_rslts[i].y1);
		p_obj->y2 = (int)(det_rslts[i].y2);
		if (det_rslts[i].category == 1) {
			p_obj->category = 1;
		} else {
			p_obj->category = 2;  // Assign all other category num to 2
		}
	}
	*/
#endif

	return ret;
}

HD_RESULT ai_rslt_trans_trk_info(PDCNN_TRACKID_RESULT *objs, UINT32 obj_num, HD_PATH_ID enc_path, HD_DIM alg_vdo_size)
{
	HD_RESULT ret = HD_OK;

#if SHOW_BBOX_IN_VLC
	// update pdcnn result
	INT32 det_num = (INT32)(obj_num > VENDOR_VIDEOENC_SMART_BBOX_MAX_NUM) ? VENDOR_VIDEOENC_SMART_BBOX_MAX_NUM : obj_num;

	VENDOR_VIDEOENC_SMART_BBOX smart_bbox = {0};
	smart_bbox.base_resolution.w = alg_vdo_size.w;
	smart_bbox.base_resolution.h = alg_vdo_size.h;
	for (INT32 num = 0; num < det_num; num++) {
		smart_bbox.bbox[num].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_TOP_LEFT].x = (UINT32)objs[num].x1;
		smart_bbox.bbox[num].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_TOP_LEFT].y = (UINT32)objs[num].y1;
		smart_bbox.bbox[num].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_BOTTOM_RIGHT].x = (UINT32)objs[num].x2;
		smart_bbox.bbox[num].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_BOTTOM_RIGHT].y = (UINT32)objs[num].y2;
		// smart_bbox.bbox[num].class_id = objs[num].category - 1;
		if (objs[num].category == 1) {
			smart_bbox.bbox[num].class_id = 0;
		} else {
			smart_bbox.bbox[num].class_id = 1;  // Assign all other category num to 1
		}
	}
	smart_bbox.timestamp = 33333;
	smart_bbox.bbox_num = det_num;
	ret = vendor_videoenc_set(enc_path, VENDOR_VIDEOENC_PARAM_SMART_BBOX, &smart_bbox);
	if (ret != HD_OK) {
		printf("set smart bbox error=%d !!\r\n", ret);
	}
#else
	/* Deprecated RTSP draw method
	PD_SHM_INFO *p_pd_shm = (PD_SHM_INFO *)g_shm;
	PD_SHM_RESULT *p_obj;

	if (p_pd_shm->exit) {
		return HD_ERR_NOT_SUPPORT;
	}

	// update pdcnn result to share memory
	p_pd_shm->pd_num = (int)obj_num;
	if (obj_num > 10) {
		p_pd_shm->pd_num = 10;
	}
	for (UINT32 i = 0; i < p_pd_shm->pd_num; i++) {
		p_obj = &p_pd_shm->pd_results[i];
		p_obj->score = objs[i].score;
		p_obj->x1 = (int)(objs[i].x1);
		p_obj->x2 = (int)(objs[i].x2);
		p_obj->y1 = (int)(objs[i].y1);
		p_obj->y2 = (int)(objs[i].y2);
		if (objs[i].category == 1) {
			p_obj->category = 1;
		} else {
			p_obj->category = 2;  // Assign all other category num to 2
		}
	}
	*/
#endif

	return ret;
}

//////////////////////////////// (Alloc Mem Block For Sample_stream_ipc) //////////////////////////////////////////
static HD_RESULT mem_init(NN_CFG_BUF_M *buf)
{
	HD_RESULT             ret = HD_OK;
	HD_COMMON_MEM_DDR_ID  ddr_id = TK_DDR_ID;
	HD_COMMON_MEM_VB_BLK  blk;
	UINTPTR               pa, va;
	
	ret = hd_common_mem_init(NULL);
	if (HD_OK != ret) {
		DBG_ERR("hd_common_mem_init err: %d\r\n", ret);
		return ret;
	}
	
	// Allocate parameter buffer
	if (buf->va != 0) {
		DBG_ERR("mem has already been inited\r\n");
		return HD_OK;
	}
	// nn get block ---
	blk = hd_common_mem_get_block(HD_COMMON_MEM_CNN_POOL, AI_TOTAL_BUF, ddr_id);
	if (HD_COMMON_MEM_VB_INVALID_BLK == blk) {
		DBG_ERR("hd_common_mem_get_block fail\r\n");
		return HD_ERR_NG;
	}
	//DBG_IND("block to pa \r\n");
	pa = hd_common_mem_blk2pa(blk);
	if (pa == 0) {
		DBG_ERR("get None buf, pa=%08x\r\n", (int)pa);
		return HD_ERR_FAIL;
	}
	//DBG_IND("pa mapping to va \r\n");
	va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, AI_TOTAL_BUF);
	// Release buffer
	if (va == 0) {
		ret = hd_common_mem_munmap((void *)va, AI_TOTAL_BUF);
		if (ret != HD_OK) {
			DBG_ERR("mem unmap fail\r\n");
			return ret;
		}
		return HD_ERR_FAIL;
	}
	
	memset((void *)va, 0, AI_TOTAL_BUF);
	
	buf->pa = pa;
	buf->va = va;
	buf->size = AI_TOTAL_BUF;
	g_blk = blk;
	// - end nn get block
	return ret;
}

static HD_RESULT mem_exit(NN_CFG_BUF_M *buf)
{
	HD_RESULT ret = HD_OK;
	/* Release in buffer */
	if (buf->va > 0) {
		ret = hd_common_mem_munmap((void *)buf->va, AI_TOTAL_BUF);
		if (ret != HD_OK) {
			DBG_ERR("hd_common_mem_munmap fail.\r\n");
			return ret;
		}
	}
	ret = hd_common_mem_release_block(g_blk);
	if (ret != HD_OK) {
		DBG_ERR("hd_common_mem_release_block fail.\r\n");
		return ret;
	}

	hd_common_mem_uninit();
	return ret;
}

/////////////////////////////// (Turnkey Assign Buffer For Models) //////////////////////////////////////
static HD_RESULT assign_ai_buf(NN_CFG_BUF_M temp_buf, AI_SUB_PROCESS_THREAD_PARM_S *parm, SAMPLE_CFG_BUFSIZE_S *bs)
{
	HD_RESULT ret = HD_OK;
	
	int limit_fdet_enable = (LIMIT_FDET_PVD && PVDCNN_MODE);
	
	UINT32 all_bufsize = 0;
	if (PDCNN_MODE == 1){
		parm->pd_mem.pa = temp_buf.pa;
		parm->pd_mem.va = temp_buf.va;
		parm->pd_mem.size = bs->pdcnn_bufsize;
		temp_buf.va += parm->pd_mem.size;
		temp_buf.pa += parm->pd_mem.size;
		temp_buf.size -= parm->pd_mem.size;
		
		all_bufsize += bs->pdcnn_bufsize;
	}
	if (FDCNN_MODE == 1) {
		parm->fd_mem.pa = temp_buf.pa;
		parm->fd_mem.va = temp_buf.va;
		parm->fd_mem.size = bs->fdcnn_bufsize;
		temp_buf.va += parm->fd_mem.size;
		temp_buf.pa += parm->fd_mem.size;
		temp_buf.size -= parm->fd_mem.size;
		
		all_bufsize += bs->fdcnn_bufsize;
	}
	if (CDDCNN_MODE == 1) {
		parm->cdd_mem.pa = temp_buf.pa;
		parm->cdd_mem.va = temp_buf.va;
		parm->cdd_mem.size = bs->cddcnn_bufsize;
		temp_buf.va += parm->cdd_mem.size;
		temp_buf.pa += parm->cdd_mem.size;
		temp_buf.size -= parm->cdd_mem.size;
		
		all_bufsize += bs->cddcnn_bufsize;
	}
	if (PVDCNN_MODE == 1) {
		parm->pvd_mem.pa = temp_buf.pa;
		parm->pvd_mem.va = temp_buf.va;
		parm->pvd_mem.size = bs->pvdcnn_bufsize;
		temp_buf.va += parm->pvd_mem.size;
		temp_buf.pa += parm->pvd_mem.size;
		temp_buf.size -= parm->pvd_mem.size;
		
		all_bufsize += bs->pvdcnn_bufsize;
	}
	if (limit_fdet_enable){
		parm->limit_fdet_mem.pa = temp_buf.pa;
		parm->limit_fdet_mem.va = temp_buf.va;
		parm->limit_fdet_mem.size = bs->limit_fdet_bufsize;
		temp_buf.va += parm->limit_fdet_mem.size;
		temp_buf.pa += parm->limit_fdet_mem.size;
		temp_buf.size -= parm->limit_fdet_mem.size;
		
		all_bufsize += bs->limit_fdet_bufsize;
	}
	if(sort_enable){
		parm->sw_tracker_mem.pa = temp_buf.pa;
		parm->sw_tracker_mem.va = temp_buf.va;
		parm->sw_tracker_mem.size = bs->sw_tracker_bufsize;
		temp_buf.va += parm->sw_tracker_mem.size;
		temp_buf.pa += parm->sw_tracker_mem.size;
		temp_buf.size -= parm->sw_tracker_mem.size;
	
		all_bufsize += bs->sw_tracker_bufsize;
	}
	parm->rslt_mem.pa = temp_buf.pa;
	parm->rslt_mem.va = temp_buf.va;
	parm->rslt_mem.size = bs->result_bufsize;
	temp_buf.va += parm->rslt_mem.size;
	temp_buf.pa += parm->rslt_mem.size;
	temp_buf.size -= parm->rslt_mem.size;
	
	all_bufsize += bs->result_bufsize;

	if(debug_mode){
		if(PDCNN_MODE) DBG_MSG("pd bufsize: %ld; \r\n", parm->pd_mem.size);
		if(FDCNN_MODE) DBG_MSG("fd bufsize: %ld; \r\n", parm->fd_mem.size);
		if(CDDCNN_MODE) DBG_MSG("cdd bufsize: %ld; \r\n", parm->cdd_mem.size);
		if(PVDCNN_MODE) DBG_MSG("pvd bufsize: %ld; \r\n", parm->pvd_mem.size);
		if(limit_fdet_enable) DBG_MSG("limit bufsize: %ld; \r\n", parm->limit_fdet_mem.size);
		DBG_IND("total bufsize: %ld, required bufsize: %ld, remain bufsize: %ld \r\n", AI_TOTAL_BUF, all_bufsize, temp_buf.size);
	}

	if (all_bufsize > AI_TOTAL_BUF) {
		DBG_ERR("ai flow require mem size: %ld, but only allocate mem size: %ld.\r\n", all_bufsize, AI_TOTAL_BUF);
		return HD_ERR_NOMEM;
	}

	return ret;
}


static VOID *ai_thread_api(VOID *arg)
{
	HD_RESULT ret;
#if AI_NN_PROF
	static struct timeval tstart0, tend0;
	static UINT64 cur_time = 0, mean_time = 0, sum_time = 0;
	static UINT32 icount = 0;
#endif
	AI_SUB_PROCESS_THREAD_PARM_S *ai_parm = (AI_SUB_PROCESS_THREAD_PARM_S *)arg;

	UINTPTR phy_addr_main, vir_addr_main;
	UINT32 yuv_size = 0;
	
	HD_VIDEO_FRAME video_frame   = {0};
	VENDOR_AI3_BUF pd_cdd_src_img = {0};
    HD_GFX_IMG_BUF fd_src_img = {0};
	
    // result params
	AI_RSLT_PARM det_rslts = {0};
	det_rslts.rslt_mem = ai_parm->rslt_mem;
	SUB_PROCESS_LIVEVIEW_S stream = ai_parm->stream;
	UINT32 ai_pd_frame = 0;
	UINT32 video_size[2] = {0};
	
	SAMPLE_FILE_DIR_S files = {0};
	ret = sample_get_param_files(parms_file, &files, (int)PVD_YUV_WIDTH, (int)PVD_YUV_HEIGHT, (int)PRUNE37PVD);
	if(ret != HD_OK){
		DBG_ERR("Read params fail!\r\n");
		return 0;
	}
	
	NN_CFG_BUF_M sw_tracker_allbuf = ai_parm->sw_tracker_mem;
	NN_CFG_BUF_M sw_tracker_buf = {0};
	PDCNN_TRACKID_RESULT *sw_tracker_result = NULL;
	sort_tracker_params sw_tracker_param = {0};
    UINT32 limit_fdet_proc_id = 0;
    UINT32 pvd_proc_id = 1;
    UINT32 fd_proc_id = 2;
	
	if (LIMIT_FDET_PVD && PVDCNN_MODE) {
		ret = limit_fdet_init(ai_parm->limit_fdet_mem, limit_fdet_proc_id, (VOID *)NULL, (VOID *)NULL, &(files.limit_files), debug_mode);
		if (ret != HD_OK) {
			DBG_ERR("[LIMIT_FDET] Init fail!!! \r\n");
			return 0;
		}
	}

	NN_RESULT_M *pvd_final_result = NULL;

	if (PVDCNN_MODE == 1) {
		ret = pvdcnn_init(ai_parm->pvd_mem, pvd_proc_id, PVD_MAX_DISTANCE_MODE, (VOID *)NULL, (VOID *)NULL, &(files.pvd_files), debug_mode);
		if (ret != HD_OK){
			DBG_ERR("[PVDCNN] Failed to init pvdcnn \r\n");
			goto exit_thread;
		}
	}
    if (FDCNN_MODE == 1) {
		ret = fdcnn_init(ai_parm->fd_mem, fd_proc_id, &(files.fd_files), debug_mode);
		if (ret != HD_OK){
			DBG_ERR("[FDCNN] Failed to init fdcnn \r\n");
			goto exit_thread;
		}
	}
	
	if (sort_enable == 1) {
		ret = sort_init(&sw_tracker_allbuf, &sw_tracker_buf, &sw_tracker_result, &sw_tracker_param, files.sw_para_file);
		if (ret != HD_OK) {
			DBG_ERR("[SORT] Software tracker init fail!\r\n");
			goto exit_thread;
		}
	}
	
	if (save_results) {
		if (chk_file_exist_m_("/mnt/sd/det_results/AI/txt") != 0) {
			printf("snapshot or save_results is on, but the path -/mnt/sd/det_results/AI/txt- to save files don't exist. \n");
			system("mkdir -p /mnt/sd/det_results/AI/txt");
			printf("mkdir /mnt/sd/det_results/AI/txt\n");
		} else {
			printf("snapshot directory /mnt/sd/det_results/AI/txt exist\n");
		}
		if (chk_file_exist_m_("/mnt/sd/det_results/AI/yuv") != 0) {
			printf("snapshot or save_results is on, but the path -/mnt/sd/det_results/AI/yuv- to save files don't exist. \n");
			system("mkdir -p /mnt/sd/det_results/AI/yuv");
			printf("mkdir /mnt/sd/det_results/AI/yuv\n");
		} else {
			printf("snapshot directory /mnt/sd/det_results/AI/yuv exist\n");
		}
	}

	do {
		if (is_net_run) {
			ret = hd_videoproc_pull_out_buf(stream.proc_alg_path, &video_frame, -1); // -1 = blocking mode, 0 = non-blocking mode, >0 = blocking-timeout mode
			if (ret != HD_OK) {
				DBG_ERR("hd_videoproc_pull_out_buf fail (%d)\n\r", ret);
				goto exit_thread;
			}
#if AI_NN_PROF
			++icount;
			gettimeofday(&tstart0, NULL);
#endif
			if (video_frame.dim.w < AI_VDO_W || video_frame.dim.h < AI_VDO_H) {
				DBG_ERR("Function requires that the stream of the input algorithm is not less than %dx%d, please adjust!\r\n", AI_VDO_W, AI_VDO_H);
				goto exit_thread;
			}

			phy_addr_main = hd_common_mem_blk2pa(video_frame.blk); // Get physical addr
			if (phy_addr_main == 0) {
				DBG_ERR("blk2pa fail, blk = 0x%x\r\n", video_frame.blk);
        		goto exit_thread;
			}
			yuv_size = DBGINFO_BUFSIZE() + VDO_YUV_BUFSIZE(video_frame.dim.w, video_frame.dim.h, HD_VIDEO_PXLFMT_YUV420);
			// mmap for frame buffer (just mmap one time only, calculate offset to virtual address later)
			vir_addr_main = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_main, yuv_size);
			if (vir_addr_main == 0) {
				DBG_ERR("mmap error !!\r\n\r\n");
        		goto exit_thread;
			}
			#define PHY2VIRT_MAIN(pa) (vir_addr_main + ((pa) - phy_addr_main))
			// UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(video_frame.phy_addr[0]);
			// yuv_va = (UINT32)ptr;
			video_size[0] = video_frame.dim.w;
			video_size[1] = video_frame.dim.h;
			
			if (PDCNN_MODE == 1 || CDDCNN_MODE == 1 || PVDCNN_MODE == 1) {
				pd_cdd_src_img.width    = video_frame.dim.w;
				pd_cdd_src_img.height   = video_frame.dim.h;
				pd_cdd_src_img.channel  = 2;
				pd_cdd_src_img.line_ofs = video_frame.loff[0];
				pd_cdd_src_img.fmt      = video_frame.pxlfmt;
				pd_cdd_src_img.pa       = video_frame.phy_addr[0];
				pd_cdd_src_img.va       = vir_addr_main;
				pd_cdd_src_img.sign     = MAKEFOURCC('A', 'B', 'U', 'F');
				pd_cdd_src_img.size     = video_frame.loff[0] * video_frame.dim.h * 3 / 2;
			}
            if(FDCNN_MODE == 1){
                fd_src_img.dim.w = video_frame.dim.w;
                fd_src_img.dim.h = video_frame.dim.h;
				fd_src_img.format = HD_VIDEO_PXLFMT_YUV420;
				fd_src_img.p_phy_addr[0] = video_frame.phy_addr[0];
				fd_src_img.p_phy_addr[1] = video_frame.phy_addr[1];
				fd_src_img.p_phy_addr[2] = video_frame.phy_addr[1]; // for avoid hd_gfx_scale message
				fd_src_img.lineoffset[0] = video_frame.loff[0];
				fd_src_img.lineoffset[1] = video_frame.loff[0]; // for avoid hd_gfx_scale message
				fd_src_img.lineoffset[2] = video_frame.loff[0]; // for avoid hd_gfx_scale message
            }

			if (PVDCNN_MODE == 1) {
				INT32 pvd_out_num = 0;
				//// do proc
				if(LIMIT_FDET_PVD)
					ret = pvdcnn_process_inplace(ai_parm->pvd_mem, (void *)&pd_cdd_src_img, &(ai_parm->limit_fdet_mem), &pvd_final_result, &pvd_out_num, (INT32)(PVDCNN_PD_DET_SW | PVDCNN_VD_DET_SW | PVDCNN_ND_DET_SW));
				else
					ret = pvdcnn_process_inplace(ai_parm->pvd_mem, (void *)&pd_cdd_src_img, NULL, &pvd_final_result, &pvd_out_num, (INT32)(PVDCNN_PD_DET_SW | PVDCNN_VD_DET_SW | PVDCNN_ND_DET_SW));
				if (ret != HD_OK) {
					DBG_ERR("[PVDCNN] pvdcnn process fail!\r\n");
					goto exit_thread;
				}
				ai_rslt_integrate_pvd(&det_rslts, pvd_final_result, pvd_out_num, video_size);
			}
            if (FDCNN_MODE == 1){
                UINT32 fd_out_num = 0;
                HD_URECT frame_size = {0, 0, video_frame.dim.w, video_frame.dim.h};
                FDCNN_RESULT *fd_final_result = NULL;
                ret = fdcnn_process(ai_parm->fd_mem, &fd_src_img, &fd_final_result, &frame_size, &fd_out_num);
                if (ret != HD_OK) {
					DBG_ERR("[FDCNN] fdcnn process fail!\r\n");
					goto exit_thread;
				}
                ai_rslt_integrate_fd(&det_rslts, fd_final_result, fd_out_num, video_size);
            }

#if AI_NN_PROF
			gettimeofday(&tend0, NULL);
			cur_time = (UINT64)(tend0.tv_sec - tstart0.tv_sec) * 1000000 + (tend0.tv_usec - tstart0.tv_usec);
			sum_time += cur_time;
			mean_time = sum_time / icount;
			printf("[AI] process cur time(us): %lld, mean time(us): %lld \r\n", cur_time, mean_time);
#endif
			if (PDCNN_MODE == 1 || FDCNN_MODE == 1 || CDDCNN_MODE == 1 || PVDCNN_MODE == 1) {
				NN_RESULT_M *ai_final_rslts = (NN_RESULT_M *)det_rslts.rslt_mem.va;
				det_rslts.obj_num = network_nonmax_suppress_m_(ai_final_rslts, det_rslts.obj_num, 0.8, 1, MAX_AIOB_NUM);
				if (ret != HD_OK) {
					DBG_WRN("network nonmax_suppress proceed failed. \r\n");
				}
				if (save_results) {
					if (det_rslts.obj_num > 0) {
						CHAR TXT_FILE[256], YUV_FILE[256];
						FILE *fs, *fb;
						sprintf(TXT_FILE, "/mnt/sd/det_results/AI/txt/%09d.txt", ai_pd_frame);
						sprintf(YUV_FILE, "/mnt/sd/det_results/AI/yuv/%09d.bin", ai_pd_frame);
						fs = fopen(TXT_FILE, "w+");
						fb = fopen(YUV_FILE, "wb+");
						fwrite((void*)vir_addr_main, sizeof(UINT8), (video_frame.dim.w * video_frame.dim.h * 3 / 2), fb);
						fclose(fb);

						for (UINT32 num = 0; num < det_rslts.obj_num; num++) {
							INT32 xmin = (INT32)(ai_final_rslts[num].x1);
							INT32 ymin = (INT32)(ai_final_rslts[num].y1);
							INT32 width = (INT32)(ai_final_rslts[num].x2 - xmin);
							INT32 height = (INT32)(ai_final_rslts[num].y2 - ymin);
							INT32 category = (INT32)(ai_final_rslts[num].category);
							FLOAT score = ai_final_rslts[num].score;
							fprintf(fs, "%d %f %d %d %d %d\r\n", category, score, xmin, ymin, width, height);
						}
						fclose(fs);
						ai_pd_frame++;
					}
				}
				
                if(debug_mode){
					if (det_rslts.obj_num > 0) {
						for (UINT32 num = 0; num < det_rslts.obj_num; num++){
							INT32 xmin = (INT32)(ai_final_rslts[num].x1);
							INT32 ymin = (INT32)(ai_final_rslts[num].y1);
							INT32 width = (INT32)(ai_final_rslts[num].x2 - xmin);
							INT32 height = (INT32)(ai_final_rslts[num].y2 - ymin);
							INT32 category = (INT32)(ai_final_rslts[num].category);
							FLOAT score = ai_final_rslts[num].score;
							DBG_MSG("Objects: %d %f %d %d %d %d\r\n", category, score, xmin, ymin, width, height);
						}
					}
				}
				if (sort_enable == 1) {
					UINT32 sw_tracker_num = MIN(det_rslts.obj_num, (UINT32)sw_tracker_param.max_tracker_num);
					for (UINT32 num = 0; num < sw_tracker_num; num++) {
						sw_tracker_result[num].x1 = ai_final_rslts[num].x1;
						sw_tracker_result[num].y1 = ai_final_rslts[num].y1;
						sw_tracker_result[num].x2 = ai_final_rslts[num].x2;
						sw_tracker_result[num].y2 = ai_final_rslts[num].y2;
						sw_tracker_result[num].category = ai_final_rslts[num].category;
						sw_tracker_result[num].score = ai_final_rslts[num].score;
						sw_tracker_result[num].trackID = 0;
					}
					
					sort_update_inplace(&sw_tracker_buf, &sw_tracker_param, sw_tracker_result, &sw_tracker_num);
					ret = ai_rslt_trans_trk_info(sw_tracker_result, sw_tracker_num, stream.enc_path, video_frame.dim);
				} else {
					ret = ai_rslt_trans_info(&det_rslts, stream.enc_path, video_frame.dim);
					if (ret != HD_OK) {
						DBG_WRN("shm info may be not exist or programm will exit. \r\n");
					}
				}
			}
			det_rslts.obj_num = 0;
#if AI_NN_PROF
			gettimeofday(&tend0, NULL);
			cur_time = (UINT64)(tend0.tv_sec - tstart0.tv_sec) * 1000000 + (tend0.tv_usec - tstart0.tv_usec);
			if ((fps_delay) && (cur_time < fps_delay)) {
				usleep(fps_delay - cur_time);
			}
#endif
			if ((ret = hd_common_mem_munmap((void *)vir_addr_main, yuv_size)) != HD_OK) {
				DBG_ERR("hd_common_mem_munmap fail\r\n");
				goto exit_thread;
			}
			if ((ret = hd_videoproc_release_out_buf(stream.proc_alg_path, &video_frame)) != HD_OK) {
				DBG_ERR("hd_videoproc_release_out_buf fail (%d)\n\r", ret);
				goto exit_thread;
			}
		}
	} while (is_net_proc);

exit_thread:
    if (FDCNN_MODE == 1) {
		ret = fdcnn_uninit(ai_parm->fd_mem);
		if (HD_OK != ret) {
			DBG_ERR("[FDCNN] fdcnn uninit fail (%d)!!\r\n", ret);
		}
	}
	if (PVDCNN_MODE == 1) {
		ret = pvdcnn_uninit(ai_parm->pvd_mem);
		if (HD_OK != ret) {
			DBG_ERR("[PVDCNN] pvdcnn uninit fail (%d)!!\r\n", ret);
		}
	}
	if (LIMIT_FDET_PVD && PVDCNN_MODE){
		ret = limit_fdet_uninit(ai_parm->limit_fdet_mem);
		if (HD_OK != ret) {
			DBG_ERR("[LIMIT_FDET] limit_fdet uninit fail (%d)!!\r\n", ret);
		}
	}

	return 0;
}

////////////////////////////// Read Parameters From TXT File And Initialize Sample////////////////////////////////
static HD_RESULT turnkey_params_init(SAMPLE_CFG_BUFSIZE_S *bufsize)
{
	HD_RESULT ret;
	
	SAMPLE_CFG_PARAM_S conf_parms = {0};
	
	ret = sample_get_cfg(parms_file, bufsize, &conf_parms, (int) PVD_YUV_WIDTH, (int)PVD_YUV_HEIGHT, (int)PRUNE37PVD, 0);
	if(ret != HD_OK){
		DBG_ERR("[demo2] get config params failed \r\n");
		return ret;
	}
    
    if ((LIMIT_FDET_PVD == DISABLE) || (conf_parms.ai_mode[3] <= 0))   // limit_fdet or pvdcnn disable
    {
		bufsize->total_bufsize -= bufsize->limit_fdet_bufsize;
	}
	
	AI_TOTAL_BUF = bufsize->total_bufsize;
#if SET_TURNKEY_FRAME
    fps_delay = (UINT32)(1000000 / (10));
#else
	fps_delay = (UINT32)(1000000 / (conf_parms.frames_per_second));
#endif
	debug_mode = conf_parms.debug_mode;
	save_results = conf_parms.save_result;
	sort_enable = conf_parms.sw_tracker_enable;
	
	PDCNN_MODE = conf_parms.ai_mode[0];
	FDCNN_MODE = conf_parms.ai_mode[1];
	CDDCNN_MODE = conf_parms.ai_mode[2];
	PVDCNN_MODE = conf_parms.ai_mode[3];
	FADCNN_MODE = conf_parms.ai_mode[4];
	
	// PD_MAX_DISTANCE_MODE  = conf_parms.max_distance_mode[0];
	// CDD_MAX_DISTANCE_MODE = conf_parms.max_distance_mode[1];
	PVD_MAX_DISTANCE_MODE = conf_parms.max_distance_mode[2];
	
	return HD_OK;
}

static HD_RESULT system_init(NN_CFG_BUF_M *allbuf, AI_SUB_PROCESS_THREAD_PARM_S *ai_thread_parm, UINT pull_out_id)
{
	HD_RESULT ret;
	
	///// read turnkey params and init /////
	SAMPLE_CFG_BUFSIZE_S bufsize = {0};  //pd, fd, cdd, pvd, pe, dis, odt, buffer_pool, ba, limit_fdet, result, md, sw_tracker, ...
	ret = turnkey_params_init(&bufsize);
	if(ret != HD_OK){
		DBG_ERR("[turnkey] params init failed!\r\n");
		return ret;
	}
	
#if(!AI_IPC)
	ret = hd_common_init(0);
#else
	ret = hd_common_init(2);
#endif
	if (ret != HD_OK) {
		DBG_ERR("hd common init fail=%d\n", ret);
		return ret;
	}
	//set project config for AI
	hd_common_sysconfig(0, (1 << 16), 0, VENDOR_AI_CFG); //enable AI engine
#if(AI_IPC)
	init_share_memory();
#endif
	if((ret = hd_gfx_init()) != HD_OK){
		DBG_ERR("hd_gfx_init fail\n");
		return ret;
	}

	// Init ai3
	VENDOR_AI3_DEV_CFG dev_cfg = {0};
	ret = vendor_ai3_dev_init(&dev_cfg);
	if (ret != HD_OK) {
		printf("vendor_ai3_dev_init fail=%d\n", ret);
		return ret;
	}

	ret = mem_init(allbuf);
	if(ret != HD_OK){
		DBG_ERR("AI all mem get fail (%d)!!\n", ret);
		return ret;
	}
	
	if ((ret = hd_videoproc_init()) != HD_OK) {
		DBG_ERR("hd_videoproc_init fail (%d)!!\n", ret);
		return ret;
	}
	
	SUB_PROCESS_LIVEVIEW_S *stream = NULL;
	stream = &(ai_thread_parm->stream);

	// Setup alg stream path
	stream->proc_alg_path = HD_VIDEOPROC_PATH(HD_DAL_VIDEOPROC(0), HD_IN(0), HD_OUT(pull_out_id));
	stream->enc_path = HD_VIDEOENC_PATH(HD_DAL_VIDEOENC(0), HD_IN(0), HD_OUT(0));

	if((ret = assign_ai_buf(*allbuf, ai_thread_parm, &bufsize)) != HD_OK){
		DBG_ERR("ai buf assign fail (%ld)!!\n", ret);
		return ret;
	}
	if(debug_mode){
		DBG_MSG("%-16s: pa=(0x%lx), va=(0x%lx), size=(0x%lx)\n", "pd_buf", ai_thread_parm->pd_mem.pa, ai_thread_parm->pd_mem.va, ai_thread_parm->pd_mem.size);
		DBG_MSG("%-16s: pa=(0x%lx), va=(0x%lx), size=(0x%lx)\n", "fd_buf", ai_thread_parm->fd_mem.pa, ai_thread_parm->fd_mem.va, ai_thread_parm->fd_mem.size);
		DBG_MSG("%-16s: pa=(0x%lx), va=(0x%lx), size=(0x%lx)\n", "cdd_buf", ai_thread_parm->cdd_mem.pa, ai_thread_parm->cdd_mem.va, ai_thread_parm->cdd_mem.size);
		DBG_MSG("%-16s: pa=(0x%lx), va=(0x%lx), size=(0x%lx)\n", "pvd_buf", ai_thread_parm->pvd_mem.pa, ai_thread_parm->pvd_mem.va, ai_thread_parm->pvd_mem.size);
		DBG_MSG("%-16s: pa=(0x%lx), va=(0x%lx), size=(0x%lx)\n", "limit_fdet_buf", ai_thread_parm->limit_fdet_mem.pa, ai_thread_parm->limit_fdet_mem.va, ai_thread_parm->limit_fdet_mem.size);
		DBG_MSG("%-16s: pa=(0x%lx), va=(0x%lx), size=(0x%lx)\n", "rslt_buf", ai_thread_parm->rslt_mem.pa, ai_thread_parm->rslt_mem.va, ai_thread_parm->rslt_mem.size);
		// DBG_MSG("USE_MD: %d \n", USE_MD);
	}

	return HD_OK;
}

MAIN(argc, argv)
{
	HD_RESULT ret;
	INT key;

	pthread_t ai_thread_id;
	int start_run = 1;
    UINT pull_out_path_id = 1;  // default video frame pull out path id: 1
    if (argc >= 2){
        pull_out_path_id = atoi(argv[1]);
    }
    
	NN_CFG_BUF_M allbuf = {0};
	AI_SUB_PROCESS_THREAD_PARM_S ai_thread_parm;
	memset(&ai_thread_parm, 0, sizeof(ai_thread_parm));
	
	printf("[Turnkey] turnkey version: %s \r\n", turnkey_ver);
	
	///// init system params /////
	ret = system_init(&allbuf, &ai_thread_parm, pull_out_path_id);
	if(ret != HD_OK){
		DBG_ERR("[turnkey] system init failed!\r\n");
		goto exit;
	}

	///// create thread /////
	if ((ret = pthread_create(&ai_thread_id, NULL, ai_thread_api, (VOID*)(&ai_thread_parm))) != HD_OK) {
		DBG_ERR("create ai_thread encode thread failed");
		goto exit;
	}
	
	/////  run network directly /////
#if(AI_IPC)
	is_net_proc = TRUE;
	is_net_run = TRUE;
#endif
	if (start_run) {
		// query user key
		printf("usage:\n");
		printf("  enter q: exit\n");
		printf("  enter r: run engine\n");
		do {
			key = getchar();
			if ((key == 'q') || (key == 0x3)) {
				is_net_proc = FALSE;
				is_net_run = FALSE;
				break;
			} else if (key == 'r') {
				//  run network
				is_net_proc = TRUE;
				is_net_run = TRUE;
				printf("[ai sample] write result done!\n");
				continue;
			}

		} while (1);
	}
	
	pthread_join(ai_thread_id, NULL);
	///// Exit Turnkey /////
exit:
	if((ret = hd_gfx_uninit()) != HD_OK){
		DBG_ERR("hd_gfx_uninit fail!!\n");
	}
	
	if((ret = mem_exit(&allbuf)) != HD_OK){
		DBG_ERR("mem_exit fail!!\n");
	}
	if ((ret = hd_videoproc_uninit()) != HD_OK) {
		DBG_ERR("hd_videoproc_uninit fail!!\n");
	}

	// uninit ai3
	ret = vendor_ai3_dev_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai3_dev_uninit fail=%d\n", ret);
	}

#if(AI_IPC)
	exit_share_memory();
#endif

	return 0;
}

