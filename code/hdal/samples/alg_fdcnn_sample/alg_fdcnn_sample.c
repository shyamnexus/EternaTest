/**
	@brief Source file of vendor net application sample using user-space net flow.

	@file alg_fdcnn_sample.c

	@ingroup alg_fdcnn_sample

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
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

#include "ext_module_api.h"
#include "fdcnn_lib.h"

#define THIS_DBGLVL         7 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
///////////////////////////////////////////////////////////////////////////////
#define __MODULE__          __HD_FDCNN_ALG_SAMPLE__
#define __DBGLVL__          THIS_DBGLVL
#define __DBGFLT__          "*" //*=All, [mark]=CustomClass
#include "kwrap/debug.h"

/*-----------------------------------------------------------------------------*/
/* Constant Definitions                                                        */
/*-----------------------------------------------------------------------------*/
//#define DEFAULT_DEVICE          "/dev/" VENDOR_AIS_FLOW_DEV_NAME

#define MAX_INPUT_WIDTH      (3840)
#define MAX_INPUT_HEIGHT     (2160)
#define MAX_INPUT_SIZE       (MAX_INPUT_WIDTH*MAX_INPUT_HEIGHT)

#define AI_FDNN_PROF         ENABLE
#define NN_SAVE_OBJS         ENABLE
#define NN_SAVE_LMKS         DISABLE

//#define NN_FDCNN_TYPE        FDCNN_NETWORK_V21

#define NN_USE_DRAM2         DISABLE //ENABLE
#define VENDOR_AI_CFG        0x000f0000  //ai project config
#define AI_RGB_BUFSIZE(w, h)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(HD_VIDEO_PXLFMT_RGB888_PLANAR) / 8) * (h))

#define FDCNN_BUFSIZE     (0x5FFE80)

typedef struct _SAMPLE_BUF_CFG_INFO {
    uintptr_t pa;
    uintptr_t va;
    UINT32 size;
    HD_COMMON_MEM_VB_BLK blk;
} SMP_BUF_CFG_INFO;

typedef struct _NET_IN {
	CHAR input_filename[PATH_LENGTH_M];
	CHAR filename[128];
	UINT32 w;
	UINT32 h;
	UINT32 c;
	UINT32 loff;
	UINT32 fmt;
	NN_CFG_BUF_M input_mem;
} NET_IN;

typedef struct _FDCNN_THREAD_PARM {
    SMP_BUF_CFG_INFO fd_mem;
    SMP_BUF_CFG_INFO input_mem;
    UINT32  start;
    UINT32  end;
} FDCNN_THREAD_PARM;

/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/
static UINT32 debug_enable = 0;
/*-----------------------------------------------------------------------------*/
/* Local Functions                                                             */
/*-----------------------------------------------------------------------------*/
static HD_RESULT system_memory_init(void){
	HD_RESULT ret;

	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
	mem_cfg.pool_info[0].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
	mem_cfg.pool_info[0].blk_size = (UINT32)FDCNN_BUFSIZE;
	mem_cfg.pool_info[0].blk_cnt = 1;
#if NN_USE_DRAM2
    mem_cfg.pool_info[0].ddr_id = DDR_ID1;
#else
    mem_cfg.pool_info[0].ddr_id = DDR_ID0;
#endif
    mem_cfg.pool_info[1].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
	mem_cfg.pool_info[1].blk_size = (UINT32)AI_RGB_BUFSIZE(MAX_INPUT_WIDTH, MAX_INPUT_HEIGHT);
	mem_cfg.pool_info[1].blk_cnt = 1;
#if NN_USE_DRAM2
    mem_cfg.pool_info[1].ddr_id = DDR_ID1;
#else
    mem_cfg.pool_info[1].ddr_id = DDR_ID0;
#endif

	ret = hd_common_mem_init(&mem_cfg);
	if (HD_OK != ret) {
		DBG_ERR("hd_common_mem_init err: %d\r\n", ret);
		return ret;
	}
	
	return HD_OK;
}

static HD_RESULT system_get_mem(UINT32 ddr, SMP_BUF_CFG_INFO *buf, UINT32 size)
{
	HD_RESULT ret;

	HD_COMMON_MEM_DDR_ID ddr_id = ddr;
    
    buf->size = size;
	buf->blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_DEFINIED_POOL, buf->size, ddr_id);     //HD_COMMON_MEM_USER_DEFINIED_POOL + id HD_COMMON_MEM_CNN_POOL
	if (HD_COMMON_MEM_VB_INVALID_BLK == buf->blk) {
		DBG_ERR("hd_common_mem_get_block fail\r\n");
		return HD_ERR_NG;
	}
	buf->pa = hd_common_mem_blk2pa(buf->blk);
	if (buf->pa == 0) {
		DBG_ERR("not get buffer, pa=%08x\r\n", (unsigned int)buf->pa);
		return HD_ERR_NOMEM;
	}

	buf->va = (uintptr_t)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, buf->pa, buf->size);
	if (buf->va == 0) {
		ret = hd_common_mem_munmap((void *)buf->va, buf->size);
		if (ret != HD_OK) {
			DBG_ERR("mem unmap fail\r\n");
			return ret;
		}
	}
    
	return HD_OK;
}

static HD_RESULT system_free_mem(SMP_BUF_CFG_INFO *buf)
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
    ret = hd_common_mem_release_block(buf->blk);
	if (ret != HD_OK) {
		printf("mem_uninit : (g_mem.pa)hd_common_mem_release_block fail.\r\n");
		return ret;
	}
	buf->pa = 0;
	buf->va = 0;
	buf->size = 0;
	
	return HD_OK;
}

HD_RESULT hdal_mem_alloc(NN_CFG_BUF_M *buf, CHAR *name, UINT32 size)
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

HD_RESULT hdal_mem_free(NN_CFG_BUF_M *buf)
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
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/

static HD_RESULT input_open(NET_IN *p_nn_in, HD_GFX_IMG_BUF *p_input_info)
{
	HD_RESULT ret = HD_OK;
	
	// ret = hdal_mem_alloc(&(p_nn_in->input_mem), "ai_in_buf", AI_RGB_BUFSIZE(p_nn_in->w, p_nn_in->h));
	// if (ret != HD_OK) {
	// 	DBG_ERR("[fdcnn] alloc ai_in_buf fail\r\n");
	// 	return HD_ERR_FAIL;
	// }

	INT32 data_size = load_file_m_(p_nn_in->input_filename, p_nn_in->input_mem.va);
	if (data_size <= 0) {
		DBG_ERR("[fdcnn] load image(%s) fail\r\n", p_nn_in->input_filename);
		return HD_ERR_FAIL;
	}

	hd_common_mem_flush_cache((VOID *)p_nn_in->input_mem.va, (UINT32)data_size);
	
	p_input_info->dim.w = p_nn_in->w;
	p_input_info->dim.h = p_nn_in->h;
	p_input_info->format = HD_VIDEO_PXLFMT_YUV420;
	p_input_info->lineoffset[0] = p_nn_in->loff;
	p_input_info->lineoffset[1] = p_nn_in->loff;
	p_input_info->p_phy_addr[0] = p_nn_in->input_mem.pa;
	p_input_info->p_phy_addr[1] = p_nn_in->input_mem.pa + p_nn_in->w * p_nn_in->h;
	
	return ret;
}

HD_RESULT input_close(NET_IN *p_nn_in)
{
	HD_RESULT ret = HD_OK;
    
    memset((void *)p_nn_in->input_mem.va, 0, p_nn_in->input_mem.size);
	// if (p_nn_in->input_mem.pa && p_nn_in->input_mem.va) {
	// 	NN_CFG_BUF_M input_mem;
	// 	input_mem.pa = p_nn_in->input_mem.pa;
	// 	input_mem.va = p_nn_in->input_mem.va;
	// 	hdal_mem_free(&input_mem);
	// }
	return ret;
}

VOID *fdcnn_thread_api(VOID *arg)
{
    HD_RESULT ret;
#if AI_FDNN_PROF
    static struct timeval tstart, tend;
    static UINT64 cur_time = 0, sum_time = 0;
#endif
	HD_GFX_IMG_BUF input_image = {0};
    UINT32 fd_proc_id = 0;
	
	FDCNN_THREAD_PARM *p_fd_parm = (FDCNN_THREAD_PARM*)arg;
    
    NN_CFG_BUF_M fdcnn_mem = {0};
    fdcnn_mem.pa = p_fd_parm->fd_mem.pa;
    fdcnn_mem.va = p_fd_parm->fd_mem.va;
    fdcnn_mem.size = p_fd_parm->fd_mem.size;

	NN_FILE_PATH fd_file = {0};
	if (FDCNN_LIGHT){
		snprintf(fd_file.model_file, PATH_LENGTH_M, "/mnt/sd/CNNLib/para/fdcnn/nvt_model_light.bin");
    } else {
        snprintf(fd_file.model_file, PATH_LENGTH_M, "/mnt/sd/CNNLib/para/fdcnn/nvt_model.bin");
    }
    
	ret = fdcnn_init(fdcnn_mem, fd_proc_id, &fd_file, debug_enable);
	if(ret != HD_OK){
		DBG_ERR("[fdcnn] Init fail!!! \r\n");
		return 0;
	}
	static FDCNN_RESULT *fdcnn_info = NULL;
    UINT32 fdcnn_num = 0;
    HD_URECT fdcnn_size = {0, 0, MAX_INPUT_WIDTH, MAX_INPUT_HEIGHT};

	NET_IN nn_in = {0};
    nn_in.input_mem.pa = p_fd_parm->input_mem.pa;
    nn_in.input_mem.va = p_fd_parm->input_mem.va;
    nn_in.input_mem.size = p_fd_parm->input_mem.size;
    
    nn_in.c = 2;
    nn_in.fmt = HD_VIDEO_PXLFMT_YUV420;
    
	CHAR IMG_LIST[256] = "/mnt/sd/jpg/fdlist.txt";
    CHAR IMG_PATH[256] = "/mnt/sd/jpg/FD";
	FILE *fr = fopen(IMG_LIST, "r");
#if NN_SAVE_OBJS
    UINT32 i = 0;
	FILE *fw = NULL;
    UINT32 out_size = 0;
    CHAR out_file[256] = {0};
#endif
#if NN_SAVE_LMKS
    UINT32 j;
#endif
    INT32 skip_image_num = 0;
    INT32 line_seg_num = 0;
    UINT32 image_proc_num = 0;
    
	CHAR *token = NULL, *line_infor = NULL;
    CHAR list_infor[PATH_LENGTH_M] = "";
	CHAR result_file[PATH_LENGTH_M] = "";
    
	while(fgets(list_infor, PATH_LENGTH_M, fr) != NULL){
		list_infor[strlen(list_infor) - 1] = '\0';
		line_seg_num = 0;
		line_infor = list_infor;
		while ((token = strtok(line_infor, " ")) != NULL) {
			if (line_seg_num > 2) {
				break;
			}
			if (line_seg_num == 0) {
				strcpy(nn_in.filename, token);
				snprintf(nn_in.input_filename, PATH_LENGTH_M, "%s/%s", IMG_PATH, token);
                printf("--> image_file(%u): %s. \r\n", image_proc_num+1, nn_in.input_filename);
			}
			if (line_seg_num == 1) {
				nn_in.w = (UINT32)atoi(token);
				nn_in.loff = ALIGN_CEIL_4(nn_in.w);
			}
			if (line_seg_num == 2) {
				nn_in.h = (UINT32)atoi(token);
			}
			line_infor = NULL;
			line_seg_num++;
		}
        if ((nn_in.w * nn_in.h) > MAX_INPUT_SIZE){
            skip_image_num++;
            continue;
        }
		
		if ((ret = input_open(&nn_in, &input_image)) != HD_OK) {
			DBG_ERR("[fdcnn] input image open fail !!\n");
			goto exit_thread;
		}
		image_proc_num++;
		fdcnn_size.w = nn_in.w;
		fdcnn_size.h = nn_in.h;

#if AI_FDNN_PROF
    	gettimeofday(&tstart, NULL);
#endif
        ret = fdcnn_process(fdcnn_mem, &input_image, &fdcnn_info, &fdcnn_size, &fdcnn_num);
		if (ret != HD_OK) {
			DBG_ERR("[fdcnn] process image fail=%d\n", ret);
			goto exit_thread;
		}
#if AI_FDNN_PROF
    	gettimeofday(&tend, NULL);
    	cur_time = (UINT64)(tend.tv_sec - tstart.tv_sec) * 1000000 + (tend.tv_usec - tstart.tv_usec);
    	sum_time += cur_time;
#endif

#if (NN_SAVE_OBJS || NN_SAVE_LMKS)
		// save result (must creat dir in SD card before)
		snprintf(result_file, PATH_LENGTH_M, "/mnt/sd/det_results/fd/%s.txt", nn_in.filename);
		fw = fopen(result_file, "w");
		if (NULL == fw){
			DBG_ERR("[fdcnn] Open/Create file '%s' fail!\r\n", result_file);
            goto exit_thread;
		}
		out_size = snprintf(out_file, 256, "%u\r\n", fdcnn_num);
		fwrite((UINT8 *)(out_file), out_size, 1, fw);
#endif
		INT32 x1 = 0, y1 = 0, x2 = 0, y2 = 0;
		FLOAT score = 0.0;
		for(i = 0; i < fdcnn_num; i++ )
		{
			// score = (FLOAT)fdcnn_info[i].score / (FLOAT)(1<<14);
			score = fdcnn_info[i].score;
			x1 = fdcnn_info[i].x;
			y1 = fdcnn_info[i].y;
			x2 = fdcnn_info[i].x + fdcnn_info[i].w;
			y2 = fdcnn_info[i].y + fdcnn_info[i].h;
			printf("[fdcnn] img_num: %u, [%u -> %lf, %d, %d, %d, %d] \r\n", image_proc_num, i, score, x1, y1, x2, y2);
#if NN_SAVE_OBJS
            fprintf(fw, "%d %d %d %d %f\r\n", x1, y1, x2, y2, score);
#endif
#if NN_SAVE_LMKS
            for(j = 0; j < 5; j++)
            {
                fprintf(fw, "%d %d;", fdcnn_info[i].landmark_points[j].x, fdcnn_info[i].landmark_points[j].y);
            }
			fprintf(fw, "\r\n");
#endif
		}
#if (NN_SAVE_OBJS || NN_SAVE_LMKS)
		fclose(fw);
#endif
		if ((ret = input_close(&nn_in)) != HD_OK) {
			DBG_ERR("[fdcnn] input image close fail !!\n");
			goto exit_thread;
		}
	}
    printf("\r\n---> Finish: process %u images in total, %u images skipped, mean time: %lld \r\n", image_proc_num, skip_image_num, sum_time / image_proc_num);
exit_thread:
	fclose(fr);
    ret = fdcnn_uninit(fdcnn_mem);
	if (HD_OK != ret) {
		DBG_ERR("[fdcnn] uninit fail (%d)\n", ret);
	}

    // ret = vendor_ai_release_id(fd_proc_id);
    // if (HD_OK != ret){
    //     DBG_ERR("[fdcnn] release proc_id fail (%d)!!\r\n", ret);
    // }

    return 0;
}

static HD_RESULT sample_system_init(FDCNN_THREAD_PARM *p_thread_parms)
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

	VENDOR_AI3_DEV_CFG dev_cfg = {0};
	ret = vendor_ai3_dev_init(&dev_cfg);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai3_dev_init fail=%d\n", ret);
		return ret;
	}
    VENDOR_AI3_VER ai3_ver = {0};
	ret = vendor_ai3_dev_get(VENDOR_AI3_CFG_VER, &ai3_ver);
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai3_dev_get(CFG_VER) fail=%d\n", ret);
		return ret;
	}
	DBG_IND("vendor_ai version = %s\r\n", ai3_ver.vendor_ai_impl_version);
	DBG_IND("kflow_ai  version = %s\r\n", ai3_ver.kflow_ai_impl_version);
	DBG_IND("kdrv_ai   version = %s\r\n", ai3_ver.kdrv_ai_impl_version);
    
	ret = system_memory_init();
	if (ret != HD_OK) {
		DBG_ERR("sample_memory_init fail=%d\n", ret);
		return ret;
	}
#if NN_USE_DRAM2
    UINT32 ddr_id = DDR_ID1;
#else
    UINT32 ddr_id = DDR_ID0;
#endif

	ret = system_get_mem(ddr_id, &(p_thread_parms->fd_mem), (UINT32)(FDCNN_BUFSIZE));
    if (ret != HD_OK) {
		DBG_ERR("alloc fd_mem fail=%d\n", ret);
		return ret;
	}
	ret = system_get_mem(ddr_id, &(p_thread_parms->input_mem), (UINT32)(AI_RGB_BUFSIZE(MAX_INPUT_WIDTH, MAX_INPUT_HEIGHT)));
	if (ret != HD_OK) {
		DBG_ERR("alloc input_mem fail=%d\n", ret);
		return ret;
	}
	
	return HD_OK;
}

int main(int argc, char *argv[])
{
	HD_RESULT ret;
	
	pthread_t nn_thread_id;
	FDCNN_THREAD_PARM fd_thread_parm;
	memset(&fd_thread_parm, 0, sizeof(fd_thread_parm));
	
    if(argc != 4)
    {
        DBG_ERR("cmd: alg_fdcnn_sample [debug_mode] [start idx] [end idx]\n");
        return HD_ERR_NOT_SUPPORT;
    }
	debug_enable = (UINT32)atoi(argv[1]);
    fd_thread_parm.start = (UINT32)atoi(argv[2]);
    fd_thread_parm.end   = (UINT32)atoi(argv[3]);
	
	ret = sample_system_init(&fd_thread_parm);
	if (ret != HD_OK) {
		DBG_ERR("system_init fail=%d\n", ret);
		goto exit;
	}
	
	ret = pthread_create(&nn_thread_id, NULL, fdcnn_thread_api, (VOID*)(&fd_thread_parm));
    if (ret < 0) {
        DBG_ERR("create fdcnn fd thread failed");
        goto exit;
    }

    pthread_join(nn_thread_id, NULL);

exit:
	ret = system_free_mem(&(fd_thread_parm.fd_mem));
	if (ret != HD_OK) {
		DBG_ERR("free fd_mem fail\r\n");
	}
    ret = system_free_mem(&(fd_thread_parm.input_mem));
	if (ret != HD_OK) {
		DBG_ERR("free input_mem fail\r\n");
	}
	ret = hd_gfx_uninit();
	if (ret != HD_OK) {
		DBG_ERR("hd_gfx_uninit fail\r\n");
	}
    ret = vendor_ai3_dev_uninit();
	if (ret != HD_OK) {
		DBG_ERR("vendor_ai3_dev_uninit fail=%d\n", ret);
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


