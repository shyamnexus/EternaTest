/**
    @brief Source file of vendor ai net sample code.

    @file ai_net_with_buf.c

    @ingroup ai_net_sample

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
#include <signal.h>
#include "hdal.h"
#include "hd_type.h"
#include "hd_debug.h"
#include <arm_neon.h>
#include <sys/time.h>
#include "limit_fdet_lib.h"
#include "pvdcnn_lib.h"
#include "ext_module_api.h"

#define THIS_DBGLVL         7 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
///////////////////////////////////////////////////////////////////////////////
#define __MODULE__          __HDAL_PVDCNN_SAMPLE__
#define __DBGLVL__          THIS_DBGLVL
#define __DBGFLT__          "*" //*=All, [mark]=CustomClass
#include "kwrap/debug.h"

// platform dependent
#if defined(__LINUX)
#include <pthread.h>            //for pthread API
#define MAIN(argc, argv)        int main(int argc, char** argv)
#define GETCHAR()               getchar()
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <kwrap/util.h>     //for sleep API
#define sleep(x)                vos_util_delay_ms(1000*(x))
#define msleep(x)               vos_util_delay_ms(x)
#define usleep(x)               vos_util_delay_us(x)
#include <kwrap/examsys.h>  //for MAIN(), GETCHAR() API
#define MAIN(argc, argv)        EXAMFUNC_ENTRY(ai_net_with_buf, argc, argv)
#endif

// TODO: Test on 32/64 bit compilation
#define LOCAL_MEM_ALIGN_SIZE (8 * sizeof(uintptr_t))

///////////////////////////////////////////////////////////////////////////////
#define VENDOR_AI_CFG               0x000f0000  //vendor ai config
#define AI_RGB_BUFSIZE(w, h)        (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(HD_VIDEO_PXLFMT_RGB888_PLANAR) / 8) * (h))

#define SAVE_SCALE          0
#define LIMIT_PVD           0
#define max_distance_mode   0
#define MAX_FRAME_WIDTH     2880  //1920
#define MAX_FRAME_HEIGHT    1620  //1080
#define SCALE_BUF_SIZE      (MAX_FRAME_WIDTH * MAX_FRAME_HEIGHT * 3)

///////////////////////////////////////////////////////////////////////////////
#define AI_NN_PROF         ENABLE

#define PVD_BUFSIZE        (6137280)   // 512_prune
#define LIMIT_DET_BUFSIZE  (675520)

static UINT32 debug_enable = 0;

/*-----------------------------------------------------------------------------*/
/* Type Definitions                                                            */
/*-----------------------------------------------------------------------------*/

typedef struct _NET_IN {
	CHAR input_filename[256];
	CHAR filename[64];
	UINT32 w;
	UINT32 h;
	UINT32 c;
	UINT32 loff;
	UINT32 fmt;
	NN_CFG_BUF_M input_mem;
} NET_IN;

typedef struct _PVD_THREAD_MEM{
	NN_CFG_BUF_M pvd_mem;
	NN_CFG_BUF_M scale_mem;
	NN_CFG_BUF_M limit_fdet_mem;
} PVD_THREAD_MEM;

static HD_COMMON_MEM_VB_BLK g_blk[3];

static HD_RESULT system_memory_init(void)
{
	HD_RESULT ret;

	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
	mem_cfg.pool_info[0].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
	mem_cfg.pool_info[0].blk_size = PVD_BUFSIZE;
	mem_cfg.pool_info[0].blk_cnt = 1;
	mem_cfg.pool_info[0].ddr_id = DDR_ID0;
	
	mem_cfg.pool_info[1].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
	mem_cfg.pool_info[1].blk_size = SCALE_BUF_SIZE;
	mem_cfg.pool_info[1].blk_cnt = 1;
	mem_cfg.pool_info[1].ddr_id = DDR_ID0;
#if LIMIT_PVD
	mem_cfg.pool_info[2].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
	mem_cfg.pool_info[2].blk_size = (UINT32)LIMIT_DET_BUFSIZE;
	mem_cfg.pool_info[2].blk_cnt = 1;
	mem_cfg.pool_info[2].ddr_id = DDR_ID0;
#endif
	
	ret = hd_common_mem_init(&mem_cfg);
	if (HD_OK != ret) {
		DBG_ERR("hd_common_mem_init err: %d\r\n", ret);
		return ret;
	}
	
	return HD_OK;
}

static HD_RESULT system_get_mem(UINT32 ddr, NN_CFG_BUF_M *buf, UINT32 size, INT32 index)
{
	HD_RESULT ret;
	uintptr_t pa   = 0;
	void  *va   = NULL;
	HD_COMMON_MEM_DDR_ID ddr_id = ddr;
	// HD_COMMON_MEM_VB_BLK blk;

	g_blk[index] = hd_common_mem_get_block(HD_COMMON_MEM_USER_DEFINIED_POOL, size, ddr_id);     //HD_COMMON_MEM_USER_DEFINIED_POOL + id HD_COMMON_MEM_CNN_POOL
	if (HD_COMMON_MEM_VB_INVALID_BLK == g_blk[index]) {
		DBG_ERR("hd_common_mem_get_block fail\r\n");
		return HD_ERR_NG;
	}
	pa = hd_common_mem_blk2pa(g_blk[index]);
	if (pa == 0) {
		DBG_ERR("not get buffer, pa=%08x\r\n", (int)pa);
		return HD_ERR_NOMEM;
	}
	va = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, size);

	if (va == 0) {
		ret = hd_common_mem_munmap(va, size);
		if (ret != HD_OK) {
			DBG_ERR("mem unmap fail\r\n");
			return ret;
		}
	}

	buf->pa = pa;
	buf->va = (uintptr_t)va;
	buf->size = size;

	return HD_OK;
}

static HD_RESULT system_free_mem(NN_CFG_BUF_M *buf, INT32 index)
{
	HD_RESULT ret = HD_OK;
	/* Release in buffer */
	if (buf->va) {
		ret = hd_common_mem_munmap((void *)buf->va, buf->size);
		if (ret != HD_OK) {
			DBG_ERR("mem_uninit : (g_mem.va)hd_common_mem_munmap fail.\r\n");
			return ret;
		}
	}
	ret = hd_common_mem_release_block(g_blk[index]);
	if (ret != HD_OK) {
		DBG_ERR("hd_common_mem_release_block fail.\r\n");
		return ret;
	}
	buf->pa = 0;
	buf->va = 0;
	buf->size = 0;
	
	return HD_OK;
}


static HD_RESULT pvdcnn_get_name(CHAR *modelname)
{
	char model_name1[PATH_LENGTH_M];
	char model_name2[PATH_LENGTH_M];
	if ((PVD_YUV_WIDTH == 320) && (PVD_YUV_HEIGHT == 320)) {
		snprintf(model_name1, PATH_LENGTH_M, "/mnt/sd/CNNLib/para/pvdcnn/nvt_model_320_origin.bin");
		snprintf(model_name2, PATH_LENGTH_M, "/mnt/sd/CNNLib/para/pvdcnn/nvt_model_320_prune37.bin");
	} else if ((PVD_YUV_WIDTH == 416) && (PVD_YUV_HEIGHT == 416)) {
		snprintf(model_name1, PATH_LENGTH_M, "/mnt/sd/CNNLib/para/pvdcnn/nvt_model_416_origin.bin");
		snprintf(model_name2, PATH_LENGTH_M, "/mnt/sd/CNNLib/para/pvdcnn/nvt_model_416_prune37.bin");
	} else if ((PVD_YUV_WIDTH == 512) && (PVD_YUV_HEIGHT == 512)) {
		snprintf(model_name1, PATH_LENGTH_M, "/mnt/sd/CNNLib/para/pvdcnn/nvt_model_512_origin.bin");
		snprintf(model_name2, PATH_LENGTH_M, "/mnt/sd/CNNLib/para/pvdcnn/nvt_model_512_prune37.bin");
	} else if ((PVD_YUV_WIDTH == 608) && (PVD_YUV_HEIGHT == 608)) {
		snprintf(model_name1, PATH_LENGTH_M, "/mnt/sd/CNNLib/para/pvdcnn/nvt_model_608_origin.bin");
		snprintf(model_name2, PATH_LENGTH_M, "/mnt/sd/CNNLib/para/pvdcnn/nvt_model_608_prune37.bin");
	} else {
		DBG_ERR("Our model can't support pvd model input width = %d height = %d \n", PVD_YUV_WIDTH, PVD_YUV_HEIGHT);
		return HD_ERR_FAIL;
	}
#if PRUNE37PVD
	sprintf(modelname, model_name2);
#else
	sprintf(modelname, model_name1);
#endif

	return HD_OK;
}

static HD_RESULT hdal_mem_alloc(NN_CFG_BUF_M *buf, CHAR *name, UINT32 size)
{
	HD_RESULT ret = HD_OK;
	uintptr_t pa   = 0;
	void  *va   = NULL;
	
	ret = hd_common_mem_alloc(name, &pa, (void **)&va, size, DDR_ID0);
	if (ret != HD_OK) {
		DBG_ERR("hdal_mem_alloc failed \r\n");
		return ret;
	}

	buf->pa   = pa;
	buf->va   = (uintptr_t)va;
	buf->size = size;
	
	return HD_OK;
}

static HD_RESULT hdal_mem_free(NN_CFG_BUF_M *buf)
{
	HD_RESULT ret = HD_OK;

	ret =  hd_common_mem_free(buf->pa, (void *)buf->va);
	if (ret != HD_OK) {
		DBG_ERR("hdal_mem_free failed \r\n");
		return ret;
	}

	buf->pa = 0;
	buf->va = 0;
	buf->size = 0;
	
	return HD_OK;
}

/*-----------------------------------------------------------------------------*/
/* Global Functions                                                             */
/*-----------------------------------------------------------------------------*/
BOOL need_ise_resize(NET_IN *nn_in, UINT32 width, UINT32 height)
{
	if (nn_in->w < width || nn_in->h < height) {
		return TRUE;
	} else if (nn_in->w > MAX_FRAME_WIDTH || nn_in->h > MAX_FRAME_HEIGHT) {
		return TRUE;
	} else {
		return FALSE;
	}
}

/*-----------------------------------------------------------------------------*/
/* Input Functions                                                             */
/*-----------------------------------------------------------------------------*/
///////////////////////////////////////////////////////////////////////////////
static HD_RESULT input_open(NET_IN *p_nn_in, VENDOR_AI3_BUF *p_src_img, FLOAT *dump_ratios, NN_CFG_BUF_M *scale_mem)
{
	HD_RESULT ret = HD_OK;
	
	dump_ratios[0] = 1.0;
	dump_ratios[1] = 1.0;
	
	ret = hdal_mem_alloc(&(p_nn_in->input_mem), "ai_in_buf", AI_RGB_BUFSIZE(p_nn_in->w, p_nn_in->h));
	if (ret != HD_OK) {
		DBG_ERR("[pvdcnn] alloc ai_in_buf fail\r\n");
		return HD_ERR_FAIL;
	}

	UINT32 file_len = load_file_m_(p_nn_in->input_filename, p_nn_in->input_mem.va);
	if (file_len <= 0) {
		DBG_ERR("[pvdcnn] load image(%s) fail\r\n", p_nn_in->input_filename);
		return HD_ERR_FAIL;
	}

	hd_common_mem_flush_cache((VOID *)p_nn_in->input_mem.va, file_len);
	
	BOOL ise_resize = need_ise_resize(p_nn_in, (UINT32)PVD_YUV_WIDTH, (UINT32)PVD_YUV_HEIGHT);
	
	if(ise_resize){
		dump_ratios[0] = (FLOAT)p_nn_in->w / (FLOAT)PVD_YUV_WIDTH;
		dump_ratios[1] = (FLOAT)p_nn_in->h / (FLOAT)PVD_YUV_HEIGHT;
		HD_GFX_IMG_BUF gfx_img = {0};
		VENDOR_AI3_BUF src_img = {0};
		src_img.width    = p_nn_in->w;
		src_img.height   = p_nn_in->h;
		src_img.channel  = p_nn_in->c;
		src_img.line_ofs = p_nn_in->loff;
		src_img.fmt      = HD_VIDEO_PXLFMT_YUV420;
		src_img.pa       = p_nn_in->input_mem.pa;
		src_img.va       = p_nn_in->input_mem.va;
		src_img.sign     = MAKEFOURCC('A', 'B', 'U', 'F');
		src_img.size     = p_nn_in->loff * p_nn_in->h * 3 / 2;
		
		gfx_img.dim.w = PVD_YUV_WIDTH;
		gfx_img.dim.h = PVD_YUV_HEIGHT;
		gfx_img.format = src_img.fmt;
		gfx_img.lineoffset[0] = ALIGN_CEIL_4(PVD_YUV_WIDTH);
		gfx_img.lineoffset[1] = ALIGN_CEIL_4(PVD_YUV_WIDTH);
		gfx_img.p_phy_addr[0] = scale_mem->pa;
		gfx_img.p_phy_addr[1] = scale_mem->pa + PVD_YUV_WIDTH * PVD_YUV_HEIGHT;

		NN_F_RECT roi = { 0, 0, src_img.width, src_img.height };
		ret = network_crop_img_m_(&gfx_img, (void *)&src_img, HD_GFX_SCALE_QUALITY_BILINEAR, &roi);
		if (ret != HD_OK) {
			DBG_ERR("[pvdcnn] ai_crop_img fail=%d\n", ret);
			return ret;
		}
		network_cvt_gfx2vendor_m_((void *)p_src_img, &gfx_img, (UINT32)scale_mem->va);
#if SAVE_SCALE
		CHAR BMP_FILE[256];
		FILE *fb;
		sprintf(BMP_FILE, "/mnt/sd/save_bmp/%s_scale.bin", p_nn_in->filename);
		fb = fopen(BMP_FILE, "wb+");
		fwrite((UINT32 *)scale_mem->va, sizeof(UINT32), (gfx_img.dim.h * gfx_img.dim.w + gfx_img.dim.h * gfx_img.dim.w / 2), fb);
		fclose(fb);
#endif
	}
	else{
		p_src_img->fmt     = HD_VIDEO_PXLFMT_YUV420;
		p_src_img->channel = p_nn_in->c;
		p_src_img->pa      = p_nn_in->input_mem.pa;
		p_src_img->va      = p_nn_in->input_mem.va;
		p_src_img->line_ofs = p_nn_in->loff;
		p_src_img->width   = p_nn_in->w;
		p_src_img->height  = p_nn_in->h;
		p_src_img->sign    = MAKEFOURCC('A', 'B', 'U', 'F');
		p_src_img->size    = p_nn_in->loff * p_nn_in->h * 3 / 2;
	}
	
	return ret;
}

static HD_RESULT input_close(NET_IN *p_nn_in)
{
	HD_RESULT ret = HD_OK;
	if (p_nn_in->input_mem.pa && p_nn_in->input_mem.va) {
		NN_CFG_BUF_M input_mem;
		input_mem.pa = p_nn_in->input_mem.pa;
		input_mem.va = p_nn_in->input_mem.va;
		hdal_mem_free(&input_mem);
	}
	return ret;
}

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
static VOID *nn_thread_api(VOID *arg)
{
	HD_RESULT ret;
#if AI_NN_PROF
	static struct timeval tstart, tend;
	UINT32 all_time = 0, cur_time = 0;
#endif
	NET_IN nn_in;
	VENDOR_AI3_BUF p_src_img;
	
	PVD_THREAD_MEM *pvd_parm = (PVD_THREAD_MEM *)arg;

	NN_FILE_PATH pvd_files = {0};
	snprintf(pvd_files.para_file, PATH_LENGTH_M, "/mnt/sd/configs/pvdcnn_para_config.txt");
	
	ret = pvdcnn_get_name(pvd_files.model_file);
	if (ret != HD_OK){
		DBG_ERR("[pvdcnn] get model name fail \n");
		return 0;
	}
	
	ret = pvdcnn_init(pvd_parm->pvd_mem, 0, max_distance_mode, (VOID *)NULL, (VOID *)NULL, &pvd_files, debug_enable);
	if(ret != HD_OK){
		DBG_ERR("[pvdcnn] Failed to init pvdcnn \r\n");
        return 0;
	}
	// pvdcnn_params.maxdistance = (UINT) max_distance_mode;
#if LIMIT_PVD
	NN_FILE_PATH limit_files = {0};
	snprintf(limit_files.model_file, PATH_LENGTH_M, "/mnt/sd/CNNLib/para/public/nvt_model.bin");
	snprintf(limit_files.para_file, PATH_LENGTH_M, "/mnt/sd/configs/limit_fdet_para_config.txt");
    
	UINT32 limit_fdet_proc_id = 0;
	ret = limit_fdet_init(pvd_parm->limit_fdet_mem, limit_fdet_proc_id, (VOID *)NULL, (VOID *)NULL, &limit_files, debug_enable);
	if (ret != HD_OK) {
		DBG_ERR("[limit_fdet] Failed to init limit det \r\n");
		goto exit_thread;
	}
#endif
	// network_get_ai_inputlayer_info_m_(pvdcnn_params.run_id);
	
	INT32 img_num = 0;
	CHAR IMG_PATH[256];
	CHAR IMG_LIST[256];
	CHAR list_infor[256];
	CHAR *line_infor;

	sprintf(IMG_LIST, "/mnt/sd/jpg/pvdlist.txt");
	sprintf(IMG_PATH, "/mnt/sd/jpg/PVD");

	FILE *fr;
	fr = fopen(IMG_LIST, "r");

	INT32 len = 0;
	CHAR *token;
	INT32 sl = 0;
	FLOAT dump_ratios[2] = {1.0};

	if (NULL == fr) {
		DBG_ERR("[pvdcnn] Failed to open img_list: %s!\r\n", IMG_LIST);
	}
	while (fgets(list_infor, 256, fr) != NULL) {
		len = strlen(list_infor);
		list_infor[len - 1] = '\0';
		sl = 0;
		line_infor = list_infor;

		while ((token = strtok(line_infor, " ")) != NULL) {
			if (sl > 2) {
				break;
			}
			if (sl == 0) {
				strcpy(nn_in.filename, token);
				sprintf(nn_in.input_filename, "%s/%s", IMG_PATH, token);
				printf("image name: %s \n", token);
			}
			if (sl == 1) {
				nn_in.w = atoi(token);
				nn_in.loff = ALIGN_CEIL_4(nn_in.w);
			}
			if (sl == 2) {
				nn_in.h = atoi(token);
			}
			line_infor = NULL;
			sl++;
		}

		img_num++;
		nn_in.c = 2;
		if ((ret = input_open(&nn_in, &p_src_img, dump_ratios, &(pvd_parm->scale_mem))) != HD_OK) {
			DBG_ERR("[pvdcnn] input image open fail !!\r\n");
			goto exit_thread;
		}
		if(debug_enable){
			DBG_MSG("[pvdcnn] input open finished \r\n");
		}
		
		NN_RESULT_M *pvd_final_result = NULL;
		INT32 pvd_out_num = 0;
		
#if AI_NN_PROF
		gettimeofday(&tstart, NULL);
#endif
		// do proc
#if LIMIT_PVD
		ret = pvdcnn_process_inplace(pvd_parm->pvd_mem, (void*)&p_src_img, &(pvd_parm->limit_fdet_mem), &pvd_final_result, &pvd_out_num, (PVDCNN_PD_DET_SW | PVDCNN_VD_DET_SW | PVDCNN_ND_DET_SW));
#else
		ret = pvdcnn_process_inplace(pvd_parm->pvd_mem, (void*)&p_src_img, NULL, &pvd_final_result, &pvd_out_num, (PVDCNN_PD_DET_SW | PVDCNN_VD_DET_SW | PVDCNN_ND_DET_SW));
#endif
		if (ret != HD_OK) {
			DBG_ERR("[pvdcnn] pvdcnn_process fail!\r\n");
			goto exit_thread;
		}
#if AI_NN_PROF
		gettimeofday(&tend, NULL);
		cur_time = (tend.tv_sec - tstart.tv_sec) * 1000000 + (tend.tv_usec - tstart.tv_usec);
		all_time += cur_time;
#endif
		if (pvd_out_num > 0) {
			CHAR TXTFILE[256] = "";
			FILE *fs;
			sprintf(TXTFILE, "/mnt/sd/det_results/pvd/pvd_%s.txt", nn_in.filename);
			fs = fopen(TXTFILE, "w");
			for (INT32 num = 0; num < pvd_out_num; num++) {
				INT32 xmin = (INT32)(pvd_final_result[num].x1 * dump_ratios[0]);
				INT32 ymin = (INT32)(pvd_final_result[num].y1 * dump_ratios[1]);
				INT32 width = (INT32)(pvd_final_result[num].x2 * dump_ratios[0] - xmin);
				INT32 height = (INT32)(pvd_final_result[num].y2 * dump_ratios[1] - ymin);
				INT32 category = (INT32)(pvd_final_result[num].category);
				FLOAT score = pvd_final_result[num].score;
				if(debug_enable){
					printf("Objects: %ld [%ld %f %ld %ld %ld %ld]\r\n", num, category, score, xmin, ymin, width, height);
				}
				fprintf(fs, "%ld %f %ld %ld %ld %ld\r\n", category, score, xmin, ymin, width, height);
			}
			fclose(fs);
		}
		
		if ((ret = input_close(&nn_in)) != HD_OK) {
			DBG_ERR("[pvdcnn] input image close fail !!\n");
			goto exit_thread;
		}
		
	}
	fclose(fr);
	printf("[pvdcnn] input stream closed \n");
	printf("[pvdcnn] img_num: %ld, cur_time: %d, mean_time: %d \n", img_num, cur_time, all_time / img_num);
	
exit_thread:
	ret = pvdcnn_uninit(pvd_parm->pvd_mem);
	if (HD_OK != ret) {
		DBG_ERR("[pvdcnn] uninit fail (%d)!!\r\n", ret);
	}
#if LIMIT_PVD
	ret = limit_fdet_uninit(pvd_parm->limit_fdet_mem);
	if (HD_OK != ret) {
		DBG_ERR("[limit_fdet] uninit fail (%d)!!\r\n", ret);
	}
#endif
	
	return 0;
}

static HD_RESULT sample_system_init(PVD_THREAD_MEM *pvd_thread_parms)
{
	HD_RESULT ret;
	
	ret = hd_common_init(0);
	if (ret != HD_OK) {
		DBG_ERR("hd_common_init fail=%d\n", ret);
		return ret;
	}
	// set project config for AI
	hd_common_sysconfig(0, (1 << 16), 0, VENDOR_AI_CFG); //enable AI engine

	ret = hd_gfx_init();
	if (ret != HD_OK) {
		DBG_ERR("hd_gfx_init fail\r\n");
		return ret;
	}

	// call init
	{
		VENDOR_AI3_DEV_CFG dev_cfg = {0};

		ret = vendor_ai3_dev_init(&dev_cfg);
		if (ret != HD_OK) {
			printf("vendor_ai3_dev_init fail=%d\n", ret);
			return ret;
		}
	}

	// dump AI3 version
	{
		VENDOR_AI3_VER ai3_ver = {0};
		ret = vendor_ai3_dev_get(VENDOR_AI3_CFG_VER, &ai3_ver);
		if (ret != HD_OK) {
			printf("vendor_ai3_dev_get(CFG_VER) fail=%d\n", ret);
			return ret;
		}
		printf("vendor_ai version = %s\r\n", ai3_ver.vendor_ai_impl_version);
		printf("kflow_ai  version = %s\r\n", ai3_ver.kflow_ai_impl_version);
		printf("kdrv_ai   version = %s\r\n", ai3_ver.kdrv_ai_impl_version);
	}
	
	ret = system_memory_init();
	if (ret != HD_OK) {
		DBG_ERR("sample_memory_init fail=%d\n", ret);
		return ret;
	}
	ret = system_get_mem(DDR_ID0, &(pvd_thread_parms->pvd_mem), (UINT32)(PVD_BUFSIZE), (INT32)0);
	if (ret != HD_OK) {
		DBG_ERR("get pvd_buf fail=%d\n", ret);
		return ret;
	}
	ret = system_get_mem(DDR_ID0, &(pvd_thread_parms->scale_mem), (UINT32)(SCALE_BUF_SIZE), (INT32)1);
	if (ret != HD_OK) {
		DBG_ERR("get scale_mem fail=%d\n", ret);
		return ret;
	}
	
#if LIMIT_PVD
	ret = system_get_mem(DDR_ID0, &(pvd_thread_parms->limit_fdet_mem), (UINT32)LIMIT_DET_BUFSIZE, (INT32)2);
	if (ret != HD_OK) {
		DBG_ERR("get limit_det_mem fail=%d\n", ret);
		return ret;
	}
#endif

	return HD_OK;
}

int main(int argc, char *argv[])
{
	HD_RESULT ret;
	
	pthread_t nn_thread_id;
	
	PVD_THREAD_MEM pvd_thread_parms;
	memset(&pvd_thread_parms, 0, sizeof(pvd_thread_parms));
	
	ret = sample_system_init(&pvd_thread_parms);
	if (ret != HD_OK) {
		DBG_ERR("system_init fail=%d\n", ret);
		goto exit;
	}
	ret = pthread_create(&nn_thread_id, NULL, nn_thread_api, (VOID *)(&pvd_thread_parms));
	if (ret < 0) {
		DBG_ERR("create encode thread failed");
		goto exit;
	}
	// wait encode thread destroyed
	pthread_join(nn_thread_id, NULL);
exit:
#if LIMIT_PVD
	ret = system_free_mem(&(pvd_thread_parms.limit_fdet_mem), (INT32)2);
	if (ret != HD_OK) {
		DBG_ERR("free limit_det_mem fail\r\n");
	}
#endif
	ret = system_free_mem(&(pvd_thread_parms.pvd_mem), (INT32)0);
	if (ret != HD_OK) {
		DBG_ERR("free pvd_mem fail\r\n");
	}
	ret = system_free_mem(&(pvd_thread_parms.scale_mem), (INT32)1);
	if (ret != HD_OK) {
		DBG_ERR("free scale_mem fail\r\n");
	}

	// uninit ai3
	ret = vendor_ai3_dev_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai3_dev_uninit fail=%d\n", ret);
	}
	
	ret = hd_gfx_uninit();
	if (ret != HD_OK) {
		DBG_ERR("hd_gfx_uninit fail\r\n");
	}
	// uninit memory
	ret = hd_common_mem_uninit();
	if (ret != HD_OK) {
		DBG_ERR("mem fail=%d\n", ret);
	}

	// uninit hdal
	ret = hd_common_uninit();
	if (ret != HD_OK) {
		DBG_ERR("common fail=%d\n", ret);
	}
	return ret;
}

