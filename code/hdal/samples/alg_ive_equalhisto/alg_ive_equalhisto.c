/**
 * @file alg_ive_equal_histo.c
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

#define MAX_IMG_WIDTH         4000
#define MAX_IMG_HEIGHT        4000
#define IMG_BUF_SIZE          (MAX_IMG_WIDTH * MAX_IMG_HEIGHT)
#define IMG_BUF_NUM           3

typedef struct _MEM_RANGE {
	UINTPTR va;        ///< Memory buffer starting address
	UINTPTR addr;      ///< Memory buffer starting address
	UINT32 size;      ///< Memory buffer size
	HD_COMMON_MEM_VB_BLK blk;
} MEM_RANGE, *PMEM_RANGE;

static void share_memory_init(MEM_RANGE *p_share_mem)
{
    HD_COMMON_MEM_VB_BLK blk;
	UINT8 i;
	UINTPTR pa, va;
	UINT32 blk_size = IMG_BUF_SIZE;
	HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;
	HD_RESULT ret;

	for(i=0;i<IMG_BUF_NUM;i++){
		p_share_mem[i].addr = 0x00;
		p_share_mem[i].va   = 0x00;
		p_share_mem[i].size = 0x00;
		p_share_mem[i].blk  = HD_COMMON_MEM_VB_INVALID_BLK;
	}
	for(i=0;i<IMG_BUF_NUM;i++){
		blk_size = IMG_BUF_SIZE;
		//printf("blk_size : %d ",blk_size);
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
		//printf("pa = 0x%x\r\n", pa);
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

static void share_memory_exit(MEM_RANGE *p_share_mem)
{
	UINT8 i;
	HD_RESULT ret;
	for(i=0;i<IMG_BUF_NUM;i++){
		if (p_share_mem[i].va != 0) {
			ret = hd_common_mem_munmap((void *)p_share_mem[i].va, p_share_mem[i].size);
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
	mem_cfg.pool_info[0].blk_cnt = IMG_BUF_NUM;
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
    HD_RESULT	ret;
	LIB_IVE_ERROR_CODE lib_ive_ret;
    char out_file1[64],in_file1[64];
    FILE  *fd;
    UINT32 file_size = 0;
    int output_size = 512*376*2;
    MEM_RANGE share_mem[IMG_BUF_NUM] = {0};
	IVE_SRC_IMAGE_S  src_img_info = {0};
	IVE_DST_IMAGE_S  dst_img_info1 = {0};
    IVE_DST_IMAGE_S  dst_img_info2 = {0};
	IVE_EQUAL_HISTO_CTRL_S equal_histo_ctrl = {0};
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

    // init share memory
	share_memory_init(share_mem);

	//ive_load_input();
#if defined(__FREERTOS)
	snprintf(in_file1, 64, "A:\\IVEP\\input\\y_only_512x376.bin");
#else
    snprintf(in_file1, 64, "//mnt//sd//IVEP//input//y_only_512x376.bin");
#endif
    file_size = ive_load_file(in_file1, share_mem[0].va);
    if (file_size == 0) {
		printf("load dram_in_y.bin : %s\r\n", in_file1);
		return 0;
    }
	
	ret = vendor_ive_init();
	if (ret != HD_OK) {
		printf("err:vendor_ive_init error %d\r\n",ret);
		goto exit;
	}

	src_img_info.enDataType = LIB_IVE_DATA_TYPE_U8C1;
	src_img_info.u64Pa      = share_mem[0].addr;
	src_img_info.u64Va      = share_mem[0].va;
	src_img_info.u32Width   = 512;
	src_img_info.u32Height  = 376;
    src_img_info.u32Stride  = 512;

	dst_img_info1.enDataType = LIB_IVE_DATA_TYPE_U8C1;
	dst_img_info1.u64Pa      = share_mem[1].addr;
	dst_img_info1.u64Va      = share_mem[1].va;
	dst_img_info1.u32Width   = 512;
	dst_img_info1.u32Height  = 376;
    dst_img_info1.u32Stride  = 512;
	output_size = dst_img_info1.u32Stride * dst_img_info1.u32Height;

    dst_img_info2.enDataType = LIB_IVE_DATA_TYPE_U32C1;
	dst_img_info2.u64Pa      = share_mem[2].addr;
	dst_img_info2.u64Va      = share_mem[2].va;

	equal_histo_ctrl.u8ShiftBit = 10;
	hd_common_mem_flush_cache((VOID *)src_img_info.u64Va, src_img_info.u32Stride*src_img_info.u32Height);
	vos_perf_mark(&hw_tick_begin);
	lib_ive_ret = NVT_IVE_EqualHisto(&handle1, &handle2, &src_img_info, &dst_img_info1, &dst_img_info2, &equal_histo_ctrl, instant);
	vos_perf_mark(&hw_tick_end);
	printf("NVT_IVE_EqualHisto time = %d\r\n", vos_perf_duration(hw_tick_begin, hw_tick_end));
	if (lib_ive_ret != LIB_IVE_OK) {
		printf("err:NVT_IVE_EqualHisto error %d\r\n",lib_ive_ret);
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
	hd_common_mem_flush_cache((VOID *)dst_img_info1.u64Va, output_size);

	ret = vendor_ive_uninit();
	if (HD_OK != ret) {
		printf("%s: vendor_ive_uninit, uninit failed (%d)\n\r", __func__, ret);
		goto exit;
	}

#if defined(__FREERTOS)
	snprintf(out_file1, 64, "A:\\IVEP\\output\\equal_histo_output.bin");
#else
    snprintf(out_file1, 64, "//mnt//sd//IVEP//output//equal_histo_output.bin");
#endif
	fd = fopen(out_file1, "wb");
	if (!fd) {
		printf("cannot open %s\r\n", out_file1);
		return 0;
	}
	file_size = fwrite((const void *)share_mem[1].va,1,output_size,fd);
	fclose(fd);
    
	printf("output writeout size : %d \n",file_size);
    printf("[ive sample] write result done!\n");

    char out_file2[64];
#if defined(__FREERTOS)
    snprintf(out_file2, 64, "A:\\IVEP\\output\\equal_histo_output.bin");
#else
    snprintf(out_file2, 64, "//mnt//sd//IVEP//output//equal_histo_cdf.bin");
#endif
    fd = fopen(out_file2, "wb");
    if (!fd) {
        printf("cannot open %s\r\n", out_file2);
        return 0;
    }
    output_size  = (256+1) * 4;
    file_size = fwrite((const void *)share_mem[2].va,1,output_size,fd);
    fclose(fd);
    
    printf("output writeout size : %d \n",file_size);
    printf("[ive sample] write result done!\n");

exit:
	share_memory_exit(share_mem);
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

