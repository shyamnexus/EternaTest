#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "hdal.h"
#include "hd_debug.h"
#include <kwrap/examsys.h>
#include <kwrap/perf.h>
#include "math.h"
#include "vqa_lib.h"


#if defined(__FREERTOS)
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h>
#include <FreeRTOS_POSIX/signal.h>
#include <kwrap/task.h>
#define sleep(x)    vos_task_delay_ms(1000*x)
#define usleep(x)   vos_task_delay_us(x)
#include <kwrap/examsys.h> 	//for MAIN(), GETCHAR() API
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(alg_vqa_sample_stream, argc, argv)
#else
#include <pthread.h>
#include <signal.h>
#define MAIN(argc, argv) 		int main(int argc, char** argv)
#endif

#define IMG_WIDTH           320
#define IMG_HEIGHT          180
#define IMG_BUF_SIZE        (IMG_WIDTH * IMG_HEIGHT)

/* Calculate mean brightness from Y plane */
static float calc_mean_brightness(UINT8 *y_buf, UINT32 size)
{
    UINT64 sum = 0;
    for (UINT32 i = 0; i < size; i++) {
        sum += y_buf[i];
    }
    return (float)sum / size;
}

#define DEBUG_MENU 			1
#define DEBUG_FILE 			0
#define OUTPUT_BMP 			0

typedef struct _VQA_MEM_RANGE {
	UINTPTR               va;        ///< Memory buffer starting address
	UINTPTR               addr;      ///< Memory buffer starting address
	UINT32               size;      ///< Memory buffer size
	HD_COMMON_MEM_VB_BLK blk;
} VQA_MEM_RANGE, *PVQA_MEM_RANGE;


static HD_RESULT vqa_set_para(UINT8 en_blur, UINT32 blur_strength, UINT32 blur_cover_th, UINT32 blur_alarm_times)
{
	vqa_param_t vqa_param = {0};
	HD_RESULT ret = HD_OK;
	// init VQA default param
	// enable parameters
	vqa_param.enable_param.en_too_light 		= 1;
	vqa_param.enable_param.en_too_dark 		= 1;
	vqa_param.enable_param.en_blur 			= en_blur;
	vqa_param.enable_param.en_blur_block_info 	= 1;
	vqa_param.enable_param.en_auto_adj_param 	= 0;
	vqa_param.enable_param.en_ref_md 		= 0;
	// global parameters
	vqa_param.global_param.width 			= 320;
	vqa_param.global_param.height 			= 180;
	//vqa_param.global_param.mb_y_size		= 40;
	//vqa_param.global_param.mb_x_size 		= 32;
	vqa_param.global_param.auto_adj_period 		= 5;
	vqa_param.global_param.g_alarm_frame_num 	= 20;
	// overexposure detection parameters
	vqa_param.light_param.too_light_strength_th 	= 215;
	vqa_param.light_param.too_light_cover_th 	= 15;
	vqa_param.light_param.too_light_alarm_times 	= 5;
	// underexposure detection parameters
	vqa_param.light_param.too_dark_strength_th 	= 50;
	vqa_param.light_param.too_dark_cover_th 	= 75;
	vqa_param.light_param.too_dark_alarm_times 	= 5;
	// blur detection parameters
	vqa_param.contrast_param.blur_strength 		= blur_strength;
	vqa_param.contrast_param.blur_cover_th 		= blur_cover_th;
	vqa_param.contrast_param.blur_alarm_times 	= blur_alarm_times;
	vqa_param.contrast_param.blur_w_num  		= 160;
    vqa_param.contrast_param.blur_h_num  		= 94;

	NVT_VQA_Set_param(0, &vqa_param);
	return ret;
}



static HD_RESULT mem_init(void)
{
        HD_RESULT                       ret;
        HD_COMMON_MEM_INIT_CONFIG       mem_cfg = {0};

        // config common pool (vqa)
        mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
        mem_cfg.pool_info[0].blk_size = NVT_VQA_Calc_buf_size(IMG_WIDTH, IMG_HEIGHT);
        mem_cfg.pool_info[0].blk_cnt = 1;
        mem_cfg.pool_info[0].ddr_id = DDR_ID0;

        // config common pool (input)
        mem_cfg.pool_info[1].type = HD_COMMON_MEM_COMMON_POOL;
        mem_cfg.pool_info[1].blk_size = IMG_BUF_SIZE;
        mem_cfg.pool_info[1].blk_cnt = 1;
        mem_cfg.pool_info[1].ddr_id = DDR_ID0;

        mem_cfg.pool_info[2].type = HD_COMMON_MEM_COMMON_POOL;
        mem_cfg.pool_info[2].blk_size = IMG_BUF_SIZE;
        mem_cfg.pool_info[2].blk_cnt = 1;
        mem_cfg.pool_info[2].ddr_id = DDR_ID0;

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

	HD_COMMON_MEM_VB_BLK blk, blk_yuv, blk_blur;
        UINTPTR vqa_buf_addr_va, vqa_buf_addr_pa;
       	UINTPTR vqa_y_buf_va, vqa_y_buf_pa, vqa_blur_buf_va, vqa_blur_buf_pa;

	char in_file[64];
	UINT32 file_size = 0;
	FILE  *fd;
	vqa_res_t vqa_rst;

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

	//// allocate vqa memory
        blk = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, NVT_VQA_Calc_buf_size(IMG_WIDTH, IMG_HEIGHT), DDR_ID0); // Get block from mem pool
        if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
                printf("get block fail (0x%x).. try again later.....\r\n", blk);
                goto exit;
        }

        vqa_buf_addr_pa = hd_common_mem_blk2pa(blk); // Get physical addr
        if (vqa_buf_addr_pa == 0) {
                printf("blk2pa fail, blk = 0x%x\r\n", blk);
                goto exit;
        }

        vqa_buf_addr_va  = (uintptr_t) hd_common_mem_mmap(
                HD_COMMON_MEM_MEM_TYPE_CACHE,
                vqa_buf_addr_pa,
                NVT_VQA_Calc_buf_size(IMG_WIDTH, IMG_HEIGHT)
        );
	// y memory 
	blk_yuv = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, IMG_BUF_SIZE, DDR_ID0);
        if (blk_yuv == HD_COMMON_MEM_VB_INVALID_BLK) {
                printf("get block fail (0x%x).. try again later.....\r\n", blk);
                goto exit;
        }
        // get physical address
        vqa_y_buf_pa = hd_common_mem_blk2pa(blk_yuv);
        if (vqa_y_buf_pa == 0) {
                printf("blk2pa fail, blk = 0x%x\r\n", blk_yuv);
                goto exit;
        }
        // get virtual address
        vqa_y_buf_va = (uintptr_t) hd_common_mem_mmap(
                HD_COMMON_MEM_MEM_TYPE_CACHE,
                vqa_y_buf_pa,
                IMG_BUF_SIZE);
	// blur info memory 
	blk_blur = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, IMG_BUF_SIZE, DDR_ID0);
        if (blk_blur == HD_COMMON_MEM_VB_INVALID_BLK) {
                printf("get block fail (0x%x).. try again later.....\r\n", blk);
                goto exit;
        }
        // get physical address
        vqa_blur_buf_pa = hd_common_mem_blk2pa(blk_blur);
        if (vqa_blur_buf_pa == 0) {
                printf("blk2pa fail, blk = 0x%x\r\n", blk_blur);
                goto exit;
        }
        // get virtual address
        vqa_blur_buf_va = (uintptr_t) hd_common_mem_mmap(
                HD_COMMON_MEM_MEM_TYPE_CACHE,
                vqa_blur_buf_pa,
                IMG_BUF_SIZE
        );

	NVT_VQA_Init(0, vqa_buf_addr_va);

	vqa_set_para(1, 8, 60, 1);

	/* Test each input file */
	int num_files = 2;
	if (argc >= 2) {
		num_files = atoi(argv[1]);
	}

	printf("\n============================================\n");
	printf("  Novatek VQA Sample - Detailed Analysis\n");
	printf("  Resolution: %dx%d\n", IMG_WIDTH, IMG_HEIGHT);
	printf("============================================\n\n");

	for (int file_idx = 1; file_idx <= num_files; file_idx++) {
		snprintf(in_file, 64, "//mnt//sd//input//vqa_y_input_%d.bin", file_idx);
		fd = fopen(in_file, "r");
		if (!fd) {
			printf("Cannot open: %s\n", in_file);
			continue;
		}
		file_size = fread((void *) vqa_y_buf_va, 1, IMG_BUF_SIZE, fd);
		fclose(fd);

		if (file_size != IMG_BUF_SIZE) {
			printf("File size mismatch: %s (got %u, expected %u)\n", 
			       in_file, file_size, IMG_BUF_SIZE);
			continue;
		}

		printf("--- File: vqa_y_input_%d.bin ---\n", file_idx);

		/* Calculate mean brightness */
		float mean_brightness = calc_mean_brightness((UINT8*)vqa_y_buf_va, IMG_BUF_SIZE);
		printf("Mean Brightness: %.1f / 255\n", mean_brightness);

		vqa_rst.res_blur_block = (UINT8 *) vqa_blur_buf_va;

		/* Time the VQA run */
		VOS_TICK t_start, t_end;
		vos_perf_mark(&t_start);

		ret = NVT_VQA_Run(0, (UINT8*) vqa_y_buf_va, &vqa_rst);

		vos_perf_mark(&t_end);
		UINT32 vqa_time = vos_perf_duration(t_start, t_end);

		if (ret != HD_OK) {
			printf("NVT_VQA_Run error: %d\n", ret);
			continue;
		}

		/* Print results */
		printf("Overexposure:    %s (flag=%u)\n", 
		       vqa_rst.res_too_light ? "YES" : "NO", vqa_rst.res_too_light);
		printf("Underexposure:   %s (flag=%u)\n", 
		       vqa_rst.res_too_dark ? "YES" : "NO", vqa_rst.res_too_dark);
		printf("Blur Detected:   %s (flag=%u)\n", 
		       vqa_rst.res_blur ? "YES" : "NO", vqa_rst.res_blur);
		printf("Blur Coverage:   %u%%\n", vqa_rst.res_blur_cover);
		printf("VQA Time:        %u us\n", vqa_time);
		printf("\n");

		/* Run multiple times for average timing */
		UINT32 total_time = 0;
		int iterations = 10;
		for (int i = 0; i < iterations; i++) {
			vos_perf_mark(&t_start);
			NVT_VQA_Run(0, (UINT8*) vqa_y_buf_va, &vqa_rst);
			vos_perf_mark(&t_end);
			total_time += vos_perf_duration(t_start, t_end);
		}
		printf("Average VQA Time (%d runs): %u us\n", iterations, total_time / iterations);
		printf("\n");
	}

	printf("============================================\n");
	printf("  Test Complete\n");
	printf("============================================\n\n");
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

        ret = hd_common_mem_release_block(blk_blur);
        if (HD_OK != ret) {
                printf("err:release blk_blur fail %d\r\n", ret);
                goto exit;
	}

	ret = mem_exit();
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
	}

	ret = hd_common_uninit();
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
	}
exit:
	return 0;
}
