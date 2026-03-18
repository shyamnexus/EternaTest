/**
	@brief Sample code of color cast of vqa lib.\n
	@file alg_clrcst_sample.c
	@author Ethan Wang
	@ingroup mhdal
	@note Nothing.
	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "kwrap/examsys.h"
#include "hdal.h"
#include "hd_debug.h"
#include "vqa_clrcst_lib.h"

#if defined(__LINUX)
#include <pthread.h>			//for pthread API
#include <signal.h>
#define MAIN(argc, argv) 		int main(int argc, char** argv)
#define GETCHAR()				getchar()
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <FreeRTOS_POSIX/signal.h>
#include <kwrap/util.h>		//for sleep API
#define sleep(x)    			vos_util_delay_ms(1000*(x))
#define msleep(x)    			vos_util_delay_ms(x)
#define usleep(x)   			vos_util_delay_us(x)
#include <kwrap/examsys.h> 	//for MAIN(), GETCHAR() API
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(alg_clrcst_sample_stream, argc, argv)
#define GETCHAR()			NVT_EXAMSYS_GETCHAR()
#endif

#define DST_IMG_WIDTH  			128
#define DST_IMG_HEIGHT			72
#define DST_IMG_BUF_SIZE		((DST_IMG_WIDTH * DST_IMG_HEIGHT) * 3 / 2)

#define DEBUG_MENU              0
#define DEBUG_FILE              0

typedef struct _CLRCST_MEM_RANGE {
        UINTPTR               va;        ///< Memory buffer starting address
        UINTPTR               addr;      ///< Memory buffer starting address
        UINT32               size;      ///< Memory buffer size
        HD_COMMON_MEM_VB_BLK blk;
} CLRCST_MEM_RANGE, *PCLRCST_MEM_RANGE;

static BOOL is_clrcst_proc					= TRUE;
static BOOL is_clrcst_run					= TRUE;

static HD_RESULT mem_init(void)
{
	HD_RESULT              		ret;
	HD_COMMON_MEM_INIT_CONFIG	mem_cfg = {0};

	// config common pool (vqa)
	mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[0].blk_size = NVT_VQA_CLRCST_Calc_buf_size(DST_IMG_WIDTH, DST_IMG_HEIGHT);
	mem_cfg.pool_info[0].blk_cnt = 1;
	mem_cfg.pool_info[0].ddr_id = DDR_ID0;

	// config common pool (input)
	mem_cfg.pool_info[1].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[1].blk_size = DST_IMG_BUF_SIZE;
	mem_cfg.pool_info[1].blk_cnt = 1;
	mem_cfg.pool_info[1].ddr_id = DDR_ID0;
	
	ret = hd_common_mem_init(&mem_cfg);
	if (HD_OK != ret) {
		printf("hd_common_mem_init err: %d\r\n", ret);
		return ret;
	}
	return ret;
}

static HD_RESULT mem_exit(void)
{
	HD_RESULT ret = HD_OK;
	ret = hd_common_mem_uninit();
	return ret;
}

MAIN(argc, argv)
{
	HD_RESULT ret;
	HD_COMMON_MEM_VB_BLK blk, blk_yuv;

	UINTPTR vqa_clrcst_buf_addr_va, vqa_clrcst_buf_addr_pa;
	UINTPTR vqa_clrcst_yuv_buf_va, vqa_clrcst_yuv_buf_pa;
	UINT32 blk_clrcst_buf_size = NVT_VQA_CLRCST_Calc_buf_size(DST_IMG_WIDTH, DST_IMG_HEIGHT);

	CLRCST_PARM_S cl_parm;
	is_clrcst_proc = TRUE;
	is_clrcst_run = TRUE;

	// hdal initialization
	ret = hd_common_init(0);
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
		goto exit;
	}

	// memory initialization
	ret = mem_init();
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
		goto exit;
	}

	//// allocate vqa clrcst memory
	blk = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, NVT_VQA_CLRCST_Calc_buf_size(DST_IMG_WIDTH, DST_IMG_HEIGHT), DDR_ID0); // Get block from mem pool
	if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("get block fail (0x%x).. try again later.....\r\n", blk);
		goto exit;
	}

	vqa_clrcst_buf_addr_pa = hd_common_mem_blk2pa(blk); // Get physical addr
	if (vqa_clrcst_buf_addr_pa == 0) {
		printf("blk2pa fail, blk = 0x%x\r\n", blk);
		goto exit;
	}

	vqa_clrcst_buf_addr_va	= (uintptr_t) hd_common_mem_mmap(
		HD_COMMON_MEM_MEM_TYPE_CACHE,
		vqa_clrcst_buf_addr_pa,
		blk_clrcst_buf_size
	);
	//// end of allocate vqa clrcst memory
	
	// allocate pattern memory
	char in_file[64];
	UINT32 file_size = 0;
	FILE  *fd;
	UINT32 test_rlt[10] = {0, 0, 0, 0, 0, 1, 1, 1, 1, 0}; //This vector are golden results of the test pattern corresponding to cl_status.

	CLRCST_STATUS_S cl_status; //color cast status
	UINT32 frame_id = 0;
	UINT32 pat_size = (DST_IMG_WIDTH * DST_IMG_HEIGHT) * 3 / 2; // test pattern cfg: yuv format with width/height=128/72

	//// get block from memory pool
	blk_yuv = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, pat_size, DDR_ID0);
	if (blk_yuv == HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("get block fail (0x%x).. try again later.....\r\n", blk);
		goto exit;
	}

	// get physical address
	vqa_clrcst_yuv_buf_pa = hd_common_mem_blk2pa(blk_yuv);
	if (vqa_clrcst_buf_addr_pa == 0) {
		printf("blk2pa fail, blk = 0x%x\r\n", blk_yuv);
		goto exit;
	}

	// get virtual address
	vqa_clrcst_yuv_buf_va = (uintptr_t) hd_common_mem_mmap(
		HD_COMMON_MEM_MEM_TYPE_CACHE,
		vqa_clrcst_yuv_buf_pa,
		pat_size
	);
	//// end of allocate pattern memory

    printf("CLRCST: Autotest start. \n");
	// configuration
	cl_parm.cl_max_yuv_width 	= 1920;
	cl_parm.cl_max_yuv_height 	= 1080;
	cl_parm.cl_collect_time 	= 1;
	cl_parm.cl_warning_max 		= 1;
	cl_parm.cl_diff_th 		= 5;
	cl_parm.cl_diff_area 		= 50;
	cl_parm.cl_mode 		= 0;
	cl_parm.cl_src_type 		= 1;
	cl_parm.cl_background_update 	= 65535; //update background frame number 
	cl_parm.cl_is_debug 		= 0;
	cl_parm.cl_buf_va_addr 		= vqa_clrcst_buf_addr_va;
	cl_parm.sensor_ch 		= 0;

	// Init
	ret = NVT_VQA_CLRCST_Init(&cl_parm);
	if (ret < 0) {
		printf("CLRCST: Error to init clrcst_init()\r\n");
		return 0;
	}

	// run 10 patterns
	for(frame_id=0; frame_id < 10; frame_id+=1){
		snprintf(in_file, 64, "//mnt//sd//input//vqa_cl_input_%d.bin", frame_id);	
		fd = fopen(in_file, "r");
		if (!fd) {
			printf("cannot open %s\r\n", in_file);
			return 0;
		}
		file_size = fread((void *) vqa_clrcst_yuv_buf_va, 1, pat_size, fd);
		if (file_size == 0) {
			printf("filesize is zero %s\r\n", in_file);
			return 0;
		}
		cl_status = (CLRCST_STATUS_S) NVT_VQA_CLRCST_Detect(vqa_clrcst_yuv_buf_va, 128, 72);
		/* print detect message*/
		if (cl_status == CL_NORMAL_S) {
			printf("frame %d is normal status\r\n", frame_id);
		} else if (cl_status == CL_ALARM_S) {
			printf("frame %d is alarm status \r\n", frame_id);
		} else {
			printf("CLRCST: Error status.(%d)\r\n", cl_status);
		}
        if(cl_status != test_rlt[frame_id]){
			printf("CLRCST: Autotest fail(frame id: %d). \n", frame_id);
			//return 0;
		}
		fclose(fd);		
	}
    printf("CLRCST: Autotest OK\n");
	// UnInit
	NVT_VQA_CLRCST_Uninit(&cl_parm);

	ret = hd_common_mem_release_block(blk);
	if (HD_OK != ret) {
		printf("err:release blk fail %d\r\n", ret);
		goto exit;
	}
	ret = hd_common_mem_release_block(blk_yuv);
	if (HD_OK != ret) {
		printf("err:release blk_yuv fail %d\r\n", ret);
		goto exit;
	}

exit:
	ret = mem_exit();
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
	}

	ret = hd_common_uninit();
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
	}

	return 0;
}
