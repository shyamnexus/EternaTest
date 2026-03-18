/**
 * @file alg_ive_dma.c
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

#define MAX_IMG_WIDTH         1024
#define MAX_IMG_HEIGHT        1024
#define IMG_BUF_SIZE          (MAX_IMG_WIDTH * MAX_IMG_HEIGHT)
#define IMG_BUF_NUM           2

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
	IVE_DST_IMAGE_S  dst_img_info = {0};
	IVE_DMA_CTRL_S dma_ctrl = {0};
	VOS_TICK    hw_tick_begin, hw_tick_end;
	UINT32 handle = 0;
	BOOL instant = 1; // 0: non-blocking, 1: blocking
	IVE_QUERY_INFO_S query_info = {0};
    UINT32 i = 0;
    UINT32 dma_copy_in_format = 0;
    UINT32 dma_copy_out_format = 0;

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

    char *chip_name = getenv("NVT_CHIP_ID");
    if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) { //CHIP_NT98539A
        if (argc == 3) {
    		sscanf(argv[1], "%hu", &dma_copy_in_format);		
    		printf("dma copy input format is set to %d (0:y format, 1: 24bit (8 + 8 + 8), 2: 40bit (16 + 16 + 8))\n", dma_copy_in_format);
            sscanf(argv[2], "%hu", &dma_copy_out_format);		
    		printf("dma copy out format is set to %d (0: 24bit (8 + 8 + 8), 1: 40bit (16 + 16 + 8))\n", dma_copy_out_format);
    	}
    }
    else { //CHIP_NT98538
        if (argc == 2) {
            sscanf(argv[1], "%hu", &dma_copy_out_format);		
    		printf("dma copy out format is set to %d (0: 24bit (8 + 8 + 8), 1: 40bit (16 + 16 + 8))\n", dma_copy_out_format);
    	}
    }

	//ive_load_input();
	if (dma_copy_in_format == 0) {
        snprintf(in_file1, 64, "//mnt//sd//IVEP//input//y_only_512x376.bin");
        file_size = ive_load_file(in_file1, share_mem[0].va);
        if (file_size == 0) {
    		printf("load dram_in_y.bin : %s\r\n", in_file1);
    		return 0;
        }
    }
    else if (dma_copy_in_format == 1) {
        snprintf(in_file1, 64, "//mnt//sd//IVEP//input//dy_dx_Y_24bit_512x376.bin");
        file_size = ive_load_file(in_file1, share_mem[0].va);
        if (file_size == 0) {
    		printf("load dram_in_y.bin : %s\r\n", in_file1);
    		return 0;
        }
    }
    else if (dma_copy_in_format == 2) {
        snprintf(in_file1, 64, "//mnt//sd//IVEP//input//dy_dx_Y_40bit_512x376.bin");
        file_size = ive_load_file(in_file1, share_mem[0].va);
        if (file_size == 0) {
    		printf("load dram_in_y.bin : %s\r\n", in_file1);
    		return 0;
        }
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
    if (dma_copy_in_format == 0) // 0: y format, 1: 24bit (8 + 8 + 8), 2: 40bit (16 + 16 + 8)
        src_img_info.u32Stride  = src_img_info.u32Width;
    else if (dma_copy_in_format == 1)
        src_img_info.u32Stride  = src_img_info.u32Width*3;
    else if (dma_copy_in_format == 2)
            src_img_info.u32Stride  = src_img_info.u32Width*5;

	dst_img_info.enDataType = LIB_IVE_DATA_TYPE_U8C1;
	dst_img_info.u64Pa      = share_mem[1].addr;
	dst_img_info.u64Va      = share_mem[1].va;

    dma_ctrl.u8HorByte    = 16; //2,3,4,8,16
	dma_ctrl.u16VerByte   = 4; //1~8191
	dma_ctrl.u8EleByte    = 7; //1~u8HorByte
	
	dst_img_info.u32Width   = (512/dma_ctrl.u8HorByte)*dma_ctrl.u8EleByte;
	dst_img_info.u32Height  = 376/dma_ctrl.u16VerByte;
    if (dma_copy_out_format == 0) {//24bit pack(8+8+8) or 40bit pack(16+16+8)
        dst_img_info.u32Stride = dst_img_info.u32Width*3;
    }
    else if (dma_copy_out_format == 1) {
        dst_img_info.u32Stride = dst_img_info.u32Width*5;
    }
	output_size = dst_img_info.u32Stride * dst_img_info.u32Height;
	
	dma_ctrl.gen_enable = 0;
    for (i=0; i<IVE_DRV_GEN_FILT_NUM; i++) {
		dma_ctrl.gen_u8Mask[i] = 0;
	}
    dma_ctrl.gen_in_fmt = 0;

	dma_ctrl.sobel_enable = 1;
    dma_ctrl.s8Mask[0] = 0;
	dma_ctrl.s8Mask[1] = 0;
	dma_ctrl.s8Mask[2] = 0;
	dma_ctrl.s8Mask[3] = 0;
	dma_ctrl.s8Mask[4] = 0;
	
	dma_ctrl.s8Mask[5] = 0;
	dma_ctrl.s8Mask[6] = -1;
	dma_ctrl.s8Mask[7] = 0;
	dma_ctrl.s8Mask[8] = 1;
	dma_ctrl.s8Mask[9] = 0;
	
	dma_ctrl.s8Mask[10] = 0;
	dma_ctrl.s8Mask[11] = -2;
	dma_ctrl.s8Mask[12] = 0;
	dma_ctrl.s8Mask[13] = 2;
	dma_ctrl.s8Mask[14] = 0;
	
	dma_ctrl.s8Mask[15] = 0;
	dma_ctrl.s8Mask[16] = -1;
	dma_ctrl.s8Mask[17] = 0;
	dma_ctrl.s8Mask[18] = 1;
	dma_ctrl.s8Mask[19] = 0;
	
	dma_ctrl.s8Mask[20] = 0;
	dma_ctrl.s8Mask[21] = 0;
	dma_ctrl.s8Mask[22] = 0;
	dma_ctrl.s8Mask[23] = 0;
	dma_ctrl.s8Mask[24] = 0;
    dma_ctrl.input_fmt = dma_copy_in_format; // 0: y format, 1: 24bit (8 + 8 + 8), 2: 40bit (16 + 16 + 8)
	dma_ctrl.dma_sobel_out_sel = dma_copy_out_format; // 0: 24bit pack(8+8+8), 1: 40bit pack(16+16+8)

	memset((VOID *)dst_img_info.u64Va, 0,output_size);
	hd_common_mem_flush_cache((VOID *)dst_img_info.u64Va, output_size);
	hd_common_mem_flush_cache((VOID *)src_img_info.u64Va, src_img_info.u32Stride*src_img_info.u32Height);
	vos_perf_mark(&hw_tick_begin);
	lib_ive_ret = NVT_IVE_DMA(&handle, &src_img_info, &dst_img_info, &dma_ctrl, instant);
	vos_perf_mark(&hw_tick_end);
	printf("NVT_IVE_DMA time = %d\r\n", vos_perf_duration(hw_tick_begin, hw_tick_end));
	if (lib_ive_ret != LIB_IVE_OK) {
		printf("err:NVT_IVE_DMA error %d\r\n",lib_ive_ret);
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
	
	ret = vendor_ive_uninit();
	if (HD_OK != ret) {
		printf("%s: vendor_ive_uninit, uninit failed (%d)\n\r", __func__, ret);
		goto exit;
	}

    snprintf(out_file1, 64, "//mnt//sd//IVEP//output//dma_copy_sobel_output_%d_%d.bin", dma_copy_in_format, dma_copy_out_format);
	fd = fopen(out_file1, "wb");
	if (!fd) {
		printf("cannot open %s\r\n", out_file1);
		return 0;
	}
	file_size = fwrite((const void *)share_mem[1].va,1,output_size,fd);
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

