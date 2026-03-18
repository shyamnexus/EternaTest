/**
	@brief Source file of vendor ai net sample code.

	@file ai_pvd_rtos_sample.c

	@ingroup ai_pvd_rtos_sample

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2020.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Including Files                                                             */
/*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <kwrap/task.h>
#include "hdal.h"
#include "hd_debug.h"
#include "vendor_ai.h"
#include "kdrv_gfx2d/kdrv_ise_ctl.h"

#include "ext_module_api.h"
#include "pvdcnn_lib.h"
#include "limit_fdet_lib.h"

// platform dependent
#if defined(__LINUX)
#include <pthread.h>			//for pthread API
#define MAIN(argc, argv) 		int main(int argc, char** argv)
#define GETCHAR()				getchar()
#else
#include <FreeRTOS_POSIX.h>	
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <kwrap/util.h>		//for sleep API
#define sleep(x)    			vos_util_delay_ms(1000*(x))
#define msleep(x)    			vos_util_delay_ms(x)
#define usleep(x)   			vos_util_delay_us(x)
#include <kwrap/examsys.h> 	//for MAIN(), GETCHAR() API
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(ai3_pvd_rtos_sample, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define CHKPNT			printf("\033[37mCHK: %s, %s: %d\033[0m\n",__FILE__,__func__,__LINE__)
#define DBGH(x)			printf("\033[0;35m%s=0x%08X\033[0m\n", #x, x)
#define DBGD(x)			printf("\033[0;35m%s=%d\033[0m\n", #x, x)

///////////////////////////////////////////////////////////////////////////////
#define VENDOR_AI_CFG  				0x000f0000  //vendor ai config
#define PRE_LOAD_PA 				(0x7bff100)  // Preload model address, may need modification
#define PRE_LOAD_MSIZE 				(0xFF000)  // Preload model size, may need modification
#define USE_PRE_LOAD 				DISABLE
#define AUTO_ADJUST_PVD_MEM_SIZE	ENABLE
///////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------*/
/* Type Definitions                                                            */
/*-----------------------------------------------------------------------------*/
static BOOL is_net_proc = TRUE;
static BOOL is_net_run  = FALSE;
static BOOL is_net_done = FALSE;

// PVD setting
UINT32 pvd_input_size = 320;
UINT32 fps_delay = 1000000 / 10; // ai 10 fps
UINT32 AI_PVD_MODEL_SIZE = 793840;
UINT32 AI_PVD_BUF_SIZE = 1893632;
UINT32 AI_LIMIT_FDET_MODEL_SIZE = 249784;
UINT32 AI_LIMIT_FDET_BUF_SIZE = 340608;

// 320x320 pvd combin model		
CHAR combin_model_name[256]	= "/mnt/sd/combin_model.bin"; // Combined pvd + limit_fdet
CHAR detect_image_path[256]	= "/mnt/sd/YUV420_SP_face_960x540.bin";
UINT PVD_MAX_DISTANCE_MODE = 0;

/*-----------------------------------------------------------------------------*/
/* Global Functions                                                             */
/*-----------------------------------------------------------------------------*/
static HD_RESULT mem_alloc(NN_CFG_BUF_M *mem_parm, CHAR* name, UINT32 size)
{
	HD_RESULT ret = HD_OK;
	UINTPTR pa   = 0;
	void  *va   = NULL;

	//alloc private pool
	ret = hd_common_mem_alloc(name, &pa, (void**)&va, size, DDR_ID0);
	if (ret!= HD_OK) {
		return ret;
	}

	mem_parm->pa   = pa;
	mem_parm->va   = (uintptr_t)va;
	mem_parm->size = size;
	return HD_OK;
}

static HD_RESULT mem_free(NN_CFG_BUF_M *mem_parm)
{
	HD_RESULT ret = HD_OK;
	
	//free private pool
	ret =  hd_common_mem_free(mem_parm->pa, (void *)mem_parm->va);
	if (ret!= HD_OK) {
		return ret;
	}
	
	mem_parm->pa = 0;
	mem_parm->va = 0;
	mem_parm->size = 0;
	return HD_OK;
}

INT32 load_bin_file_to_mem(CHAR *filename, UINTPTR va)
{
	FILE *fd;
	INT32 size = 0;
	fd = fopen(filename, "rb");
	if (!fd) {
		printf("cannot read %s\n", filename);
		return HD_ERR_NOT_OPEN;
	}

	fseek(fd, 0, SEEK_END);
	size = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	if (size < 0) {
		printf("getting %s size failed\n", filename);
	} else if ((INT32)fread((VOID *)va, 1, size, fd) != size) {
		printf("read size < %ld\n", size);
		size = -1;
	};
	//mem_parm->size = size;

	if (fd) {
		fclose(fd);
	}
	return size;
}

static HD_RESULT input_open(CHAR *input_filename, VENDOR_AI3_BUF *src_img)
{
	HD_RESULT ret = HD_OK;
	NN_CFG_BUF_M input_mem;
	
	ret = mem_alloc(&input_mem, "ai_in_buf", src_img->size);
	if (ret != HD_OK) {
		printf("alloc ai_in_buf fail\n");
		return HD_ERR_FAIL;
	}

	UINT32 file_len = load_bin_file_to_mem(input_filename, input_mem.va);
	if (file_len < 0) {
		printf("load buf(%s) fail\n", input_filename);
		return HD_ERR_NG;
	}
	printf("load buf(%s) %ld, %ld ok\n", input_filename, src_img->size, file_len);
	hd_common_mem_flush_cache((VOID *)input_mem.va, file_len);
	
	src_img->pa = input_mem.pa;
	src_img->va = input_mem.va;
	
	return ret;
}

static HD_RESULT input_close(VENDOR_AI3_BUF *src_img)
{
	HD_RESULT ret = HD_OK;
	if (src_img->pa && src_img->va) {
		NN_CFG_BUF_M input_mem;
		input_mem.pa = src_img->pa;
		input_mem.va = src_img->va;
		mem_free(&input_mem);
	}
	return ret;
}

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
typedef struct _AI_THREAD_PARM {
	NN_CFG_BUF_M pvd_mem;
	NN_CFG_BUF_M limit_fdet_mem;
	NN_CFG_BUF_M pvd_model_mem;
	NN_CFG_BUF_M limit_model_mem;
} AI_THREAD_PARM;

HD_RESULT assign_ai_buf(NN_CFG_BUF_M *buf, AI_THREAD_PARM *parm)
{
	HD_RESULT ret = HD_OK;

#if (USE_PRE_LOAD == ENABLE)
	if (PRE_LOAD_PA != buf->pa)
	{
		printf("ERR: Preload model address (0x%x) != hdal allocate address (0x%x)!!\n", PRE_LOAD_PA, buf->pa);
		return HD_ERR_FAIL;
	}
#endif

	parm->pvd_model_mem.pa = buf->pa;
	parm->pvd_model_mem.va = buf->va;
	parm->pvd_model_mem.size = ALIGN_CEIL(AI_PVD_MODEL_SIZE, 2048); // PVD model bin
	
	parm->limit_model_mem.pa = parm->pvd_model_mem.pa + parm->pvd_model_mem.size;
	parm->limit_model_mem.va = parm->pvd_model_mem.va + parm->pvd_model_mem.size;
	parm->limit_model_mem.size = ALIGN_CEIL(AI_LIMIT_FDET_MODEL_SIZE, 2048); // Limit fdet model bin
	
	parm->pvd_mem.pa = parm->limit_model_mem.pa + parm->limit_model_mem.size;
	parm->pvd_mem.va = parm->limit_model_mem.va + parm->limit_model_mem.size;
	parm->pvd_mem.size = AI_PVD_BUF_SIZE - ALIGN_CEIL(AI_PVD_MODEL_SIZE, 2048); // PVD working buffer

	parm->limit_fdet_mem.pa = parm->pvd_mem.pa + parm->pvd_mem.size;
	parm->limit_fdet_mem.va = parm->pvd_mem.va + parm->pvd_mem.size;
	parm->limit_fdet_mem.size = AI_LIMIT_FDET_BUF_SIZE - ALIGN_CEIL(AI_LIMIT_FDET_MODEL_SIZE, 2048); // Limit fdet model bin

	return ret;
}

#if (AUTO_ADJUST_PVD_MEM_SIZE == ENABLE)
static HD_RESULT prepare_combin_model(void)
{
	HD_RESULT ret = HD_OK;
	NN_CFG_BUF_M combin_model_buf = {0};
	UINT32 mem_size_updated = FALSE;
#if (USE_PRE_LOAD == ENABLE)
	UINTPTR va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, PRE_LOAD_PA, PRE_LOAD_MSIZE);
	if (va == 0) {
		printf("hd_common_mem_mmap: PRE_LOAD model mmap failed!\r\n");
		ret = hd_common_mem_munmap((void *)va, PRE_LOAD_MSIZE);
		if (ret != HD_OK) {
			printf("mem unmap fail\r\n");
		}
		return ret;
	}
	combin_model_buf.pa = PRE_LOAD_PA;
	combin_model_buf.va = va;
	combin_model_buf.size = PRE_LOAD_MSIZE;
	ret = pvdcnn_parse_buffer_info_from_combin_model_buf(&combin_model_buf, &AI_PVD_MODEL_SIZE, &AI_PVD_BUF_SIZE, &AI_LIMIT_FDET_MODEL_SIZE, &AI_LIMIT_FDET_BUF_SIZE, pvd_input_size);
	if (ret == HD_OK) {
		mem_size_updated = TRUE;
	} else {
		return HD_ERR_NOT_SUPPORT;
	}
#else
	INT32 combin_model_size = network_get_modelsize_m_(combin_model_name);
	if (combin_model_size <= 0) {
		printf("Model name (%s) not exist!!\n", combin_model_name);
		return HD_ERR_ABORT;
	}
	// Temporary read model bin to parse info.
	ret = mem_alloc(&combin_model_buf, "combin_model_buf", combin_model_size);
	if (ret != HD_OK) {
		printf("combin_model_buf get fail (%d)!!\n", ret);
		return HD_ERR_ABORT;
	}
	// Load combin_model into temporary memory buffer
	INT32 combine_model_file_len = load_bin_file_to_mem(combin_model_name, combin_model_buf.va);
	if (combine_model_file_len <= 0) {
		printf("combin model load (%s) fail\r\n", combin_model_name);
	}
	hd_common_mem_flush_cache((VOID *)combin_model_buf.va, combin_model_size);
	// Parse and update required model/work buffer size.
	ret = pvdcnn_parse_buffer_info_from_combin_model_buf(&combin_model_buf, &AI_PVD_MODEL_SIZE, &AI_PVD_BUF_SIZE, &AI_LIMIT_FDET_MODEL_SIZE, &AI_LIMIT_FDET_BUF_SIZE, pvd_input_size);
	if (ret == HD_OK) {
		mem_size_updated = TRUE;
	}
	// free temporary memory
	ret = mem_free(&combin_model_buf);
	if (ret != HD_OK) {
		printf("combin_model_buf free fail!!\n");
		return HD_ERR_ABORT;
	}
#endif
	if (mem_size_updated == TRUE) {
		printf("Update AI_PVD_MODEL_SIZE = %d\n", AI_PVD_MODEL_SIZE);
		printf("Update AI_PVD_BUF_SIZE = %d\n", AI_PVD_BUF_SIZE);
		printf("Update AI_LIMIT_FDET_MODEL_SIZE = %d\n", AI_LIMIT_FDET_MODEL_SIZE);
		printf("Update AI_LIMIT_FDET_BUF_SIZE = %d\n", AI_LIMIT_FDET_BUF_SIZE);
	}
	return ret;
}
#endif

static VOID *ai_turnkey_thread_api(VOID *arg)
{
	is_net_done = FALSE;
	
	HD_RESULT ret = HD_OK;
	AI_THREAD_PARM *ai_parm = (AI_THREAD_PARM *)arg;
	struct timeval tstart0, tend0;
	UINT32 cur_time0 = 0;
	VENDOR_AI3_BUF src_img = {0};
	
	//open one image for ai input
	src_img.width 		= 960;
	src_img.height 		= 540;
	src_img.channel 	= 2;
	src_img.line_ofs 	= 960;
	src_img.fmt      	= HD_VIDEO_PXLFMT_YUV420;
	src_img.sign 		= MAKEFOURCC('A','B','U','F');
	src_img.size 		= src_img.line_ofs * src_img.height * 3 / 2;
	if ((ret = input_open(detect_image_path, &src_img)) != HD_OK) {
		printf("input image open fail !!\n");
		goto exit_thread;
	}

	//ai initial
	VENDOR_AI3_DEV_CFG dev_cfg = {0};
	ret = vendor_ai3_dev_init(&dev_cfg);
	if (ret != HD_OK) {
		printf("vendor_ai3_dev_init fail=%d\n", ret);
		goto exit_thread;
	}

	// Load model from sd card
	NN_CFG_BUF_M pvdcnn_buf = ai_parm->pvd_mem;
	NN_CFG_BUF_M pvd_model_mem = ai_parm->pvd_model_mem;

#if (USE_PRE_LOAD == DISABLE)
	int size = load_bin_file_to_mem(combin_model_name, pvd_model_mem.va);
	if (size <= 0) {
		printf("ERR: pvdcnn load model fail (%ld)!!\n", size);
		goto exit_thread;
	}
#endif
	
	UINT32 pvdcnn_debug = DISABLE;

	// PVD Init
	NN_FILE_PATH pvd_files = { 0 };
	snprintf(pvd_files.model_file, PATH_LENGTH_M, "%s", "");
	snprintf(pvd_files.para_file, PATH_LENGTH_M, "%s", "");
	ret = pvdcnn_init(pvdcnn_buf, (UINT32)0, PVD_MAX_DISTANCE_MODE, &pvd_model_mem, (VOID *)NULL, &pvd_files, pvdcnn_debug);
	if (ret != HD_OK){
		printf("[pvdcnn] Failed to init pvdcnn \r\n");
		goto exit_thread;
	}

	// Turn off bounding box stabilization
	PVD_ADJUSTABLE_PARAM pvd_params = {0};
	ret = pvdcnn_params_get(pvdcnn_buf, &pvd_params);
	pvd_params.TRACKER_ENBALE = 0;
	// pvd_params.confindence_threshold = 0.3;
	ret = pvdcnn_params_set(pvdcnn_buf, &pvd_params);

	// Adjust PVD category threshold
	ret = pvdcnn_set_category_conf_threshold(pvdcnn_buf, 1, 0.3f);
	ret = pvdcnn_set_category_conf_threshold(pvdcnn_buf, 2, 0.35f);
	ret = pvdcnn_set_category_conf_threshold(pvdcnn_buf, 3, 0.5f);

#if (LIMIT_FDET_PVD)
	// Limit Fdet Init
	NN_FILE_PATH limit_files = { 0 };
	snprintf(limit_files.model_file, PATH_LENGTH_M, "%s", "");
	snprintf(limit_files.para_file, PATH_LENGTH_M, "%s", "");
	NN_CFG_BUF_M limit_fdet_buf = ai_parm->limit_fdet_mem;
	NN_CFG_BUF_M limit_fdet_model_mem = ai_parm->limit_model_mem;
	ret = limit_fdet_init(limit_fdet_buf, (UINT32)1, &limit_fdet_model_mem, (VOID *)NULL, &limit_files, pvdcnn_debug);
	if (ret != HD_OK){
		printf("[limit_fdet] Failed to init limit_fdet \r\n");
		goto exit_thread;
	}
	LIMIT_FDET_ADJUSTABLE_PARAM limit_fdet_params = {0};
	ret = limit_fdet_params_get(limit_fdet_buf, &limit_fdet_params);
	limit_fdet_params.detection_threshold = 0.95f;
	limit_fdet_params.cls_threshold[0] = 0.5f; // Increase people detection rate
	ret = limit_fdet_params_set(limit_fdet_buf, &limit_fdet_params);
#endif

	INT32 pvd_out_num = 0;
	NN_RESULT_M *pvd_final_result = NULL;
	do {
		gettimeofday(&tstart0, NULL);
		
		if (is_net_run) {

#if (LIMIT_FDET_PVD)
			ret = pvdcnn_process_inplace(pvdcnn_buf, (void *)&src_img, &limit_fdet_buf, &pvd_final_result, &pvd_out_num, (PVDCNN_PD_DET_SW | PVDCNN_VD_DET_SW | PVDCNN_ND_DET_SW));
			// ret = pvdcnn_process_inplace_getone_pd(pvdcnn_buf, &src_img, &limit_fdet_buf, &pvd_final_result, &pvd_out_num, PVDCNN_PD_DET_SW, 10);
#else
			ret = pvdcnn_process_inplace(pvdcnn_buf, (void *)&src_img, NULL, &pvd_final_result, &pvd_out_num, (PVDCNN_PD_DET_SW | PVDCNN_VD_DET_SW | PVDCNN_ND_DET_SW));
			// ret = pvdcnn_process_inplace_getone_pd(pvdcnn_buf, &src_img, (VOID *)NULL, &pvd_final_result, &pvd_out_num, PVDCNN_PD_DET_SW, 10);
#endif
		}
		
		gettimeofday(&tend0, NULL);
		cur_time0 = (tend0.tv_sec - tstart0.tv_sec) * 1000000 + (tend0.tv_usec - tstart0.tv_usec);
		printf("pvd time: %d us\n", cur_time0);
		if ((fps_delay) && (cur_time0 < fps_delay)) {
			vos_util_delay_us(fps_delay - cur_time0);
		}
		
		// Print detection result
		if (pvd_out_num > 0) {
			for (INT32 num = 0; num < pvd_out_num; num++) {
				INT32 xmin = (INT32)(pvd_final_result[num].x1);
				INT32 ymin = (INT32)(pvd_final_result[num].y1);
				INT32 width = (INT32)(pvd_final_result[num].x2 - xmin);
				INT32 height = (INT32)(pvd_final_result[num].y2 - ymin);
				INT32 category = (INT32)(pvd_final_result[num].category);
				FLOAT score = pvd_final_result[num].score;
				printf("Objects: %d %f %d %d %d %d\n", category, score, xmin, ymin, width, height);
			}
		}
		
	} while (is_net_proc);

exit_thread:

	ret = pvdcnn_uninit(pvdcnn_buf);
	if (HD_OK != ret) {
		printf("[pvdcnn] uninit fail (%d)!!\r\n", ret);
	}

#if (LIMIT_FDET_PVD)
	ret = limit_fdet_uninit(limit_fdet_buf);
	if (HD_OK != ret) {
		printf("[limit_fdet] uninit fail (%d)!!\r\n", ret);
	}
#endif

	if ((ret = input_close(&src_img)) != HD_OK) {
		printf("input image close fail !!\n");
	}

	// uninit ai3
	ret = vendor_ai3_dev_uninit();
	if (ret != HD_OK) {
		printf("vendor_ai3_dev_uninit fail=%d\n", ret);
	}

	is_net_done = TRUE;
	
	while(1){
		vos_util_delay_ms(10000);
		//printf("wait destroy thread\n");
	}
	
	return 0;
}

MAIN(argc, argv)
{
	HD_RESULT ret;
	INT key;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
	printf("\n\n");
	//pthread_t ai_turnkey_thread_id;
	VK_TASK_HANDLE ai_turnkey_thread_id;
	AI_THREAD_PARM ai_thread_parm;
	memset(&ai_thread_parm, 0, sizeof(ai_thread_parm));
	
	is_net_proc = TRUE;
	is_net_run  = FALSE;
	is_net_done = FALSE;
	
	// init hdal
	ret = hd_common_init(0);
	if (ret != HD_OK) {
		printf("hd_common_init fail=%d\n", ret);
		goto exit;
	}

//this is no longer need in 538
/*
	// set project config for AI
	hd_common_sysconfig(0, (1<<16), 0, VENDOR_AI_CFG); //enable AI engine
*/

	// config common pool 
	mem_cfg.pool_info[0].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
	mem_cfg.pool_info[0].blk_size = 1024;
	mem_cfg.pool_info[0].blk_cnt = 1;
	mem_cfg.pool_info[0].ddr_id = DDR_ID0;
	ret = hd_common_mem_init(&mem_cfg);
	if (HD_OK != ret) {
		printf("hd_common_mem_init err: %d\n", ret);
		goto exit;
	}
	
	//init gfx
	ret = hd_gfx_init();
	if (ret != HD_OK) {
		printf("hd_gfx_init fail\n");
		goto exit;
	}

#if (AUTO_ADJUST_PVD_MEM_SIZE == ENABLE)
	ret = prepare_combin_model();
	if (ret != HD_OK) {
		printf("Using default model/buffer size\n");
	}
#endif

	//mem init
	NN_CFG_BUF_M ai_allbuf_mem = {0};
	UINT32 AI_TOTAL_BUF = AI_PVD_BUF_SIZE + AI_LIMIT_FDET_BUF_SIZE;
	ret = mem_alloc(&ai_allbuf_mem, "ai_allbuf", AI_TOTAL_BUF);
	if (ret != HD_OK) {
		printf("AI all mem get fail (%d)!!\n", ret);
		goto exit;
	}
	ret = assign_ai_buf(&ai_allbuf_mem, &ai_thread_parm);
	if (ret != HD_OK) {
		printf("ERR: ai buf assign fail (%ld)!!\n", ret);
		goto exit;
	}
	printf("pvdcnn_allbuf: pa=(%#x), va=(%#x), size=(%ld)\n", ai_thread_parm.pvd_mem.pa, ai_thread_parm.pvd_mem.va, ai_thread_parm.pvd_mem.size);
	printf("limit_fdet_buf: pa=(%#x), va=(%#x), size=(%ld)\n", ai_thread_parm.limit_fdet_mem.pa, ai_thread_parm.limit_fdet_mem.va, ai_thread_parm.limit_fdet_mem.size);
	
	//create ai turnkey thread
	//ret = pthread_create(&ai_turnkey_thread_id, NULL, ai_turnkey_thread_api, (VOID*)(&ai_thread_parm));
	ai_turnkey_thread_id = vos_task_create(ai_turnkey_thread_api, (VOID*)(&ai_thread_parm), "ai_turnkey_thread_api", 25, 32768);
	vos_task_resume(ai_turnkey_thread_id);
	if (ret < 0) {
		printf("create ai turnkey thread thread failed");
		goto exit;
	}

	ret = kdrv_ise_rtos_init();
	if (ret != HD_OK) {
		printf("kdrv_ise_rtos_init fail=%d\n", ret);
		goto exit;
	}

	printf("Enter q to quit\n");
	printf("Enter r to run turnkey\n");
	do {
		key = GETCHAR();
		if (key == 'r') {
			is_net_proc = TRUE;
			is_net_run = TRUE;
			printf("start ai turnkey!\n");
			continue;
		}
		if (key == 'q' || key == 0x3) {
			is_net_run = FALSE;
			is_net_proc = FALSE;
			break;
		}
	} while(1);

	while(1){
		if(is_net_done)
			break;
		vos_util_delay_us(10000);
	};
	
	//pthread_join(ai_turnkey_thread_id, NULL);
	vos_task_destroy(ai_turnkey_thread_id);
	
exit:
	// uninit memory
	ret = mem_free(&ai_allbuf_mem);
	if (ret != HD_OK) {
		printf("mem_free fail!!\n");
	}

	ret = kdrv_ise_rtos_uninit();
	if (ret != HD_OK) {
		printf("kdrv_ise_rtos_uninit fail=%d\n", ret);
	}

	// unint hd_gfx
	ret = hd_gfx_uninit();
	if (ret != HD_OK) {
		printf("hd_gfx_uninit fail!!\n");
	}
	
	ret = hd_common_mem_uninit();
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
	}
	
	// uninit hdal
	ret = hd_common_uninit();
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
	}
	return ret;
}
