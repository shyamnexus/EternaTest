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

#define MAX_IMG_WIDTH         4000
#define MAX_IMG_HEIGHT        4000
#define IMG_BUF_SIZE          (MAX_IMG_WIDTH * MAX_IMG_HEIGHT)

unsigned int IVE_16to8_TEST_CASE = 0; //0: s16 to s8, 1: s16 to u8 abs, 2: s16 to u8 bias, 3: u16 to u8

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

	for(i=0;i<2;i++){
		p_share_mem[i].addr = 0x00;
		p_share_mem[i].va   = 0x00;
		p_share_mem[i].size = 0x00;
		p_share_mem[i].blk  = HD_COMMON_MEM_VB_INVALID_BLK;
	}
	for(i=0;i<2;i++){
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
	for(i=0;i<2;i++){
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
	mem_cfg.pool_info[0].blk_cnt = 2;
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
    HD_RESULT ret;
    char out_file1[64],in_file1[64];
    FILE  *fd;
    UINT32 file_size = 0;
    //UINT32 out_selection=0;
    int output_size = 512*376*2;
    MEM_RANGE share_mem[2] = {0};
	IVE_SRC_IMAGE_S  src_img_info = {0};
	IVE_DST_IMAGE_S  dst_img_info = {0};
    VOS_TICK    hw_tick_begin, hw_tick_end;
	IVE_16BIT_TO_8BIT_CTRL_S bit_clamp_ctrl = {0};
	UINT32 handle = 0;
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
	
	if (argc == 2) {
		sscanf(argv[1], "%hu", &IVE_16to8_TEST_CASE);
		
		printf("16 to 8 bit test case is set to %d\n", IVE_16to8_TEST_CASE);
	}

	//ive_load_input();
#if defined(__FREERTOS)
	snprintf(in_file1, 64, "A:\\input\\bit_clamp_input.bin");
#else
    snprintf(in_file1, 64, "//mnt//sd//input//bit_clamp_input.bin");
#endif
    file_size = ive_load_file(in_file1, share_mem[0].va);
    if (file_size == 0) {
			printf("load dram_in_y.bin : %s\r\n", in_file1);
			return 0;
    }
	
	// call vendor_ive_init once for initialize IVE 
	ret = vendor_ive_init();
	if (ret != HD_OK) {
		printf("%s: vendor_ive_init error (%d)\r\n", __func__, ret);
		goto exit;
	}

	src_img_info.enDataType = (IVE_16to8_TEST_CASE == LIB_IVE_MODE_U16_TO_U8) ? LIB_IVE_DATA_TYPE_U16C1 : LIB_IVE_DATA_TYPE_S16C1;
	src_img_info.u32Stride  = 256 * 2;
	src_img_info.u64Pa      = share_mem[0].addr;
	src_img_info.u64Va      = share_mem[0].va;
	src_img_info.u32Width   = 256;
	src_img_info.u32Height  = 256;
	
	dst_img_info.enDataType = (IVE_16to8_TEST_CASE == LIB_IVE_MODE_S16_TO_S8) ? LIB_IVE_DATA_TYPE_S8C1 : LIB_IVE_DATA_TYPE_U8C1;
	dst_img_info.u32Stride  = 256;
	dst_img_info.u64Pa      = share_mem[1].addr;
	dst_img_info.u64Va      = share_mem[1].va;
	dst_img_info.u32Width   = 256;
	dst_img_info.u32Height  = 256;

	output_size = dst_img_info.u32Stride * dst_img_info.u32Height;
	
	bit_clamp_ctrl.en16To8Mode = IVE_16to8_TEST_CASE;
	bit_clamp_ctrl.u32Coef = 1;
	bit_clamp_ctrl.u8ShiftBit = 8; 
	if (IVE_16to8_TEST_CASE == LIB_IVE_MODE_S16_TO_U8_BIAS) {
		bit_clamp_ctrl.s8Bias = 127; 
	} else {
		//don't care
	}
	
	hd_common_mem_flush_cache((VOID *)src_img_info.u64Va, src_img_info.u32Stride*src_img_info.u32Height);
    vos_perf_mark(&hw_tick_begin);
    ret = NVT_IVE_16BitTo8Bit(&handle, &src_img_info, &dst_img_info, &bit_clamp_ctrl, instant);
    vos_perf_mark(&hw_tick_end);
	printf("NVT_IVE_16BitTo8Bit time = %d\r\n", vos_perf_duration(hw_tick_begin, hw_tick_end));
    if (ret != HD_OK) {
		printf("err:NVT_IVE_16BitTo8Bit error %d\r\n",ret);
		goto exit;
	}
    if(!instant) {
    	query_info.handle = handle;
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
	hd_common_mem_flush_cache((VOID *)dst_img_info.u64Va, output_size);
	
	// call vendor_ive_uninit once for uninitialize IVE 
	ret = vendor_ive_uninit();
	if (ret != HD_OK) {
		printf("%s: vendor_ive_uninit error (%d)\r\n", __func__, ret);
		goto exit;
	}

#if defined(__FREERTOS)
	snprintf(out_file1, 64, "A:\\output\\16bit_to_8bit_mode%d_output.bin", IVE_16to8_TEST_CASE);
#else
    snprintf(out_file1, 64, "//mnt//sd//output//16bit_to_8bit_mode%d_output.bin", IVE_16to8_TEST_CASE);
#endif
	fd = fopen(out_file1, "wb");
	if (!fd) {
		printf("cannot open %s\r\n", out_file1);
		return 0;
	}

	file_size = fwrite((const void *)share_mem[1].va,1,output_size,fd);

	fclose(fd);
    
	printf("output writeout size : %d \n",file_size);
    printf("[ive 16bit to 8bit sample] write result done!\n");
	
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

