/**
 * @file vendor_ive_sample.c
 * @brief start ive sample.
 * @author ALG1-CV
 * @date in the year 2018
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "hd_debug.h"
#include "hdal.h"
#include "libive/libive.h"
#include <kwrap/perf.h>
#if defined(__LINUX)
#include <signal.h>
#include <pthread.h>            //for pthread API
#define MAIN(argc, argv)        int main(int argc, char** argv)
#define GETCHAR()               getchar()
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/signal.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <kwrap/util.h>     //for sleep API
#define sleep(x)                vos_util_delay_ms(1000*(x))
#define msleep(x)               vos_util_delay_ms(x)
#define usleep(x)               vos_util_delay_us(x)
#include <kwrap/examsys.h>  //for MAIN(), GETCHAR() API
#define MAIN(argc, argv)        EXAMFUNC_ENTRY(vendor_ive, argc, argv)
#define GETCHAR()               NVT_EXAMSYS_GETCHAR()
#endif

#define MAX_IMG_WIDTH         1280
#define MAX_IMG_HEIGHT        720
#define IMG_BUF_SIZE          (MAX_IMG_WIDTH * MAX_IMG_HEIGHT)

typedef struct _MEM_RANGE {
	UINTPTR va;        ///< Memory buffer starting address
	UINTPTR addr;      ///< Memory buffer starting address
	UINT32 size;      ///< Memory buffer size
	HD_COMMON_MEM_VB_BLK blk;
} MEM_RANGE, *PMEM_RANGE;

static void share_memory_init(MEM_RANGE *p_share_mem, UINT8 u8BlkDist, UINT8 u8Pass2_en)
{

    HD_COMMON_MEM_VB_BLK blk;
	UINT8 i;
	UINTPTR pa, va;
	UINT32 blk_size = IMG_BUF_SIZE;
	HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;
	HD_RESULT ret;
    UINT8 memory_num = 3;

    if(!u8Pass2_en) memory_num = 2;
	//mem : pstSrc/pstCorner/tmp buffer
	for(i=0;i<memory_num;i++){
		p_share_mem[i].addr = 0x00;
		p_share_mem[i].va   = 0x00;
		p_share_mem[i].size = 0x00;
		p_share_mem[i].blk  = HD_COMMON_MEM_VB_INVALID_BLK;
	}
	for(i=0;i<memory_num;i++){
		if (i==0) blk_size = IMG_BUF_SIZE;
        else blk_size = IMG_BUF_SIZE/u8BlkDist/u8BlkDist* sizeof(UINT16)*3;
		blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_BLK, blk_size, ddr_id);
		if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
			printf("err:get block fail\r\n", blk);
			return;
		}
		pa = hd_common_mem_blk2pa(blk);
		if (pa == 0) {
			printf("err:blk2pa fail, blk = 0x%x\r\n", blk);
			goto blk2pa_err;
		}
		
		if (pa > 0) {
			va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, blk_size);
			if (va == 0) {
				goto map_err;
			}
		}
		p_share_mem[i].addr = pa;
		p_share_mem[i].va   = va;
		p_share_mem[i].size = blk_size;
		p_share_mem[i].blk  = blk;
		
	}
	return;
blk2pa_err:
map_err:
	for (; i > 0 ;) {
		i -= 1;
		ret = hd_common_mem_release_block(p_share_mem[i].blk);
		if (HD_OK != ret) {
			printf("err:release blk fail %d\r\n", ret);
			return;
		}
	}
}

static void share_memory_exit(MEM_RANGE *p_share_mem, UINT8 u8Pass2_en)
{
	UINT8 i;
	HD_RESULT ret;
    UINT8 memory_num = 3;

    if(!u8Pass2_en) memory_num = 2;
	for(i=0;i<memory_num;i++){
		if (p_share_mem[i].va != 0) {
			ret = hd_common_mem_munmap((void*)p_share_mem[i].va, p_share_mem[i].size);
			if (HD_OK != ret) {
                printf("err:hd_common_mem_munmap err: \r\n\r\n");
            }
		}
		if (p_share_mem[i].blk != HD_COMMON_MEM_VB_INVALID_BLK) {
			ret = hd_common_mem_release_block(p_share_mem[i].blk);
			if (HD_OK != ret) {
				printf("err:hd_common_mem_release_block err: %d\r\n", ret);
			}
		}
		p_share_mem[i].addr = 0x00;
		p_share_mem[i].va   = 0x00;
		p_share_mem[i].size = 0x00;
		p_share_mem[i].blk  = HD_COMMON_MEM_VB_INVALID_BLK;
	}
}

static int mem_init(void)
{
	HD_RESULT                 ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg  = {0};

	mem_cfg.pool_info[0].type = HD_COMMON_MEM_USER_BLK;
	mem_cfg.pool_info[0].blk_size = IMG_BUF_SIZE;
	mem_cfg.pool_info[0].blk_cnt = 3; 
	mem_cfg.pool_info[0].ddr_id = DDR_ID0;

	ret = hd_common_mem_init(&mem_cfg);
	if (HD_OK != ret) {
		printf("err:hd_common_mem_init err: %d\r\n", ret);
	}

	return ret;



}

static HD_RESULT mem_exit(void)
{
	HD_RESULT ret = HD_OK;
	//share_memory_exit();
	ret = hd_common_mem_uninit();
	return ret;
}

static UINT32 ive_load_file(CHAR *p_filename, UINTPTR va)
{
	FILE  *fd;
	UINT32 file_size = 0, read_size = 0;
	const UINTPTR addr = va;

	fd = fopen(p_filename, "rb");
	if (!fd) {
		printf("cannot read %s\r\n", p_filename);
		return 0;
	}

	fseek ( fd, 0, SEEK_END );
	file_size = ALIGN_CEIL_4( ftell(fd) );
	fseek ( fd, 0, SEEK_SET );

	read_size = fread ((void *)addr, 1, file_size, fd);
	if (read_size != file_size) {
		printf("size mismatch, real = %d, idea = %d\r\n", (int)read_size, (int)file_size);
	}
	fclose(fd);
	return read_size;
}




MAIN(argc, argv)
{
    HD_RESULT                          ret;
	LIB_IVE_ERROR_CODE                 lib_ive_ret;
	char out_file1[64],in_file1[64];
    FILE  *fd;
    UINT32 file_size = 0;    
    
	int width = 1280;
	int height = 720;	
	int output_size;
    UINT16 pu32OutCorNum = 0 ;
    UINT32 threshold_mode = 0;
    IVE_STCORNER_CTRL_S pstSTCornerCtrl = {0} ;
	IVE_MEM_INFO_S pstCorner = {0};
    IVE_SRC_IMAGE_S pstSrc = {0}; 
    MEM_RANGE share_mem[3] = {0};
	VOS_TICK    hw_tick_begin, hw_tick_end;
	UINT32 handle1 = 0;
    UINT32 handle2 = 0;
	BOOL instant = 1; // 0: non-blocking, 1: blocking
	IVE_QUERY_INFO_S query_info = {0};

	// init common module
	ret = hd_common_init(0);
    if(ret != HD_OK) {
        printf("init fail=%d\n", ret);
        goto comm_init_fail;
    }
	// init memory
	ret = mem_init();
    if(ret != HD_OK) {
        printf("init fail=%d\n", ret);
        goto mem_init_fail;
    }
    char *chip_name = getenv("NVT_CHIP_ID");
    if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) { //CHIP_NT98539A
        if (argc == 2) {
    		sscanf(argv[1], "%hu", &threshold_mode);
    		printf("threshold_mode:%d\n", threshold_mode);
    	}
        pstSTCornerCtrl.threshold_mode = threshold_mode;
        pstSTCornerCtrl.threshold_value = 100;
    }
    
    pstSTCornerCtrl.u8QualityLevel = 1;
    pstSTCornerCtrl.u8BlkDist = 4;
    pstSTCornerCtrl.u8Pass2_en = 1;
    
    // init share memory
	share_memory_init(share_mem, pstSTCornerCtrl.u8BlkDist, pstSTCornerCtrl.u8Pass2_en);

	//ive_load_input();
#if defined(__FREERTOS)
	snprintf(in_file1, 64, "A:\\input\\y_1280_720.bin");
#else
    snprintf(in_file1, 64, "//mnt//sd//input//y_1280_720.bin");
#endif
	
    file_size = ive_load_file(in_file1, share_mem[0].va);
    if (file_size == 0) {
			printf("load dram_in_y.bin : %s error \r\n", in_file1);
			goto exit;
			//return 0;
    }	

	ret = vendor_ive_init();
	if (ret != HD_OK) {
		printf("%s: vendor_ive_init error (%d)\r\n", __func__, ret);
		goto exit;
	}
	
	//set input & output info   	
	pstSrc.enDataType = LIB_IVE_DATA_TYPE_U8C1 ;
	pstSrc.u64Pa = share_mem[0].addr;
	pstSrc.u64Va = share_mem[0].va;
	pstSrc.u32Width = width;
	pstSrc.u32Height = height;
	pstSrc.u32Stride = width;

	pstCorner.u64Pa = share_mem[1].addr;
	pstCorner.u64Va = share_mem[1].va;
    if(pstSTCornerCtrl.u8Pass2_en) {
    	pstSTCornerCtrl.stTempMem.u64Pa = share_mem[2].addr;
    	pstSTCornerCtrl.stTempMem.u64Va = share_mem[2].va;
    }
	hd_common_mem_flush_cache((VOID *)pstSrc.u64Va, pstSrc.u32Width*pstSrc.u32Height);	
	vos_perf_mark(&hw_tick_begin);
	lib_ive_ret = NVT_IVE_STCorner(&handle1, &handle2, &pstSrc,&pstCorner,&pstSTCornerCtrl,&pu32OutCorNum, pstSTCornerCtrl.u8Pass2_en, instant);
	vos_perf_mark(&hw_tick_end);
    printf("NVT_IVE_STCorner time = %d\r\n", vos_perf_duration(hw_tick_begin, hw_tick_end));
	if (lib_ive_ret != LIB_IVE_OK) {
		printf("err:NVT_IVE_STCorner error %d\r\n",ret);
		goto exit;
	}
    if(!instant) {
    	query_info.handle = handle1;
    	query_info.instant = 0;
        query_info.job_finish = 0;
    	while (1) {
    		NVT_IVE_QUERY(&query_info);
    		if (query_info.job_finish == 1) {
    			break;
    		} else {
    			printf("not finish\n");
    		}
    	}
        if(pstSTCornerCtrl.u8Pass2_en) {
            query_info.handle = handle2;
        	query_info.instant = 0;
            query_info.job_finish = 0;
        	while (1) {
        		NVT_IVE_QUERY(&query_info);
        		if (query_info.job_finish == 1) {
        			break;
        		} else {
        			printf("not finish\n");
        		}
        	}
        }
    }
    if(pstSTCornerCtrl.u8Pass2_en)
	    output_size = sizeof(UINT32) + pu32OutCorNum * sizeof(UINT16) * 3;
    else
        output_size = sizeof(UINT16)*3 * pu32OutCorNum + sizeof(UINT16);
    printf("output_size = %d\r\n", output_size);
	hd_common_mem_flush_cache((VOID *)pstCorner.u64Va, output_size);

	// call vendor_ive_uninit once for uninitialize IVE 
	ret = vendor_ive_uninit();
	if (ret != HD_OK) {
		printf("%s: vendor_ive_uninit error (%d)\r\n", __func__, ret);
		goto exit;
	}

	// set func enable
	if (threshold_mode) snprintf(out_file1, 64, "//mnt//sd//output//stcorner_out_thresh_mode.bin");
    else snprintf(out_file1, 64, "//mnt//sd//output//stcorner_out.bin");

	fd = fopen(out_file1, "wb");
	if (!fd) {
		printf("cannot open %s\r\n", out_file1);
		goto exit;
	}

	//file_size = fwrite((const void *)(share_mem[1].va+4),1,output_size-4,fd);
	if(pu32OutCorNum % 2 == 1) output_size += 6;
    file_size = fwrite((const void *)(share_mem[1].va),1,output_size,fd);
	fclose(fd);
   
exit:	
	share_memory_exit(share_mem, pstSTCornerCtrl.u8Pass2_en);
	ret = mem_exit();
	if (ret != HD_OK) {
        printf("mem fail=%d\n", ret);
    }
mem_init_fail:
	ret = hd_common_uninit();
    if(ret != HD_OK) {
        printf("uninit fail=%d\n", ret);

    }
comm_init_fail:
	return 0;


}





