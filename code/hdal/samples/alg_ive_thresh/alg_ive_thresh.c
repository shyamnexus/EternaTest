/**
	@brief Source file of vendor ai net sample code.

	@file alg_ive_erode.c

	@ingroup alg_ive_sample

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Including Files                                                             */
/*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "hdal.h"
#include "hd_debug.h"
#include "libive.h"
#include "kdrv_ive_lmt.h"

// platform dependent
#if defined(__LINUX)
#include <signal.h>
#include <pthread.h>			//for pthread API
#define MAIN(argc, argv) 		int main(int argc, char** argv)
#define GETCHAR()				getchar()
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/signal.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <kwrap/util.h>		    //for sleep API
#define sleep(x)    			vos_util_delay_ms(1000*(x))
#define msleep(x)    			vos_util_delay_ms(x)
#define usleep(x)   			vos_util_delay_us(x)
#include <kwrap/examsys.h> 	    //for MAIN(), GETCHAR() API
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(ai_net, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif


#define PROF                    0
#if PROF
	static struct timeval tstart, tend;
	#define PROF_START()    gettimeofday(&tstart, NULL);
	#define PROF_END(msg)   gettimeofday(&tend, NULL);  \
			printf("%s time (us): %lu\r\n", msg, (tend.tv_sec - tstart.tv_sec) * 1000000 + (tend.tv_usec - tstart.tv_usec));
#else
	#define PROF_START()
	#define PROF_END(msg)
#endif


/*-------------------------------------------------------------------------------------------------------------------*/
/* Type Definitions                                                                                                  */
/*-------------------------------------------------------------------------------------------------------------------*/
#define IVE_THRESH_USE_DDR          DDR_ID0
#define IVE_THRESH_VERBOSE          0
#define IVE_THRESH_MAX_STR_LEN      256

typedef struct _IVE_IMG_FILE_INFO {
    CHAR filename[256];
    LIB_IVE_DATA_TYPE datatype;
    UINT32 width;
    UINT32 height;
} IVE_IMG_FILE_INFO;

typedef struct _MEM_RANGE {
	UINTPTR va;             ///< Memory buffer starting address
	UINTPTR pa;             ///< Memory buffer starting address
	UINT32  size;           ///< Memory buffer size
	HD_COMMON_MEM_VB_BLK blk;
} MEM_RANGE, *PMEM_RANGE;

static UINT32 get_pxl_size(LIB_IVE_DATA_TYPE datatype);

/*-------------------------------------------------------------------------------------------------------------------*/
/* Memory Releated Functions                                                                                         */
/*-------------------------------------------------------------------------------------------------------------------*/
static HD_RESULT mem_init(IVE_SRC_IMAGE_S *img_src, IVE_SRC_IMAGE_S *img_dst)
{
    HD_RESULT ret = HD_OK;
    UINT32 pxl_size_src=0, pxl_size_dst=0;
    HD_COMMON_MEM_INIT_CONFIG mem_cfg = { 0 };

    // size checking
    if(img_src->u32Width*img_src->u32Height != img_dst->u32Width*img_dst->u32Height) {
        printf("%s: expect same dimensions for source and target images (%dx%d != %dx%d)\n\r", __func__, img_src->u32Stride, img_src->u32Height, img_dst->u32Stride, img_dst->u32Height);
        return HD_ERR_NOT_SUPPORT;
    }

    // memory configuration
    if((pxl_size_src=get_pxl_size(img_src->enDataType))==0) return HD_ERR_NOT_SUPPORT;
    if((pxl_size_dst=get_pxl_size(img_dst->enDataType))==0) return HD_ERR_NOT_SUPPORT;

    if(pxl_size_dst==pxl_size_src) {
    	mem_cfg.pool_info[0].type = HD_COMMON_MEM_USER_BLK;
    	mem_cfg.pool_info[0].blk_size = img_src->u32Stride * img_src->u32Height * pxl_size_src;
    	mem_cfg.pool_info[0].blk_cnt = 2;
    	mem_cfg.pool_info[0].ddr_id = IVE_THRESH_USE_DDR;
    } else {
        mem_cfg.pool_info[0].type = HD_COMMON_MEM_USER_BLK;
    	mem_cfg.pool_info[0].blk_size = img_src->u32Stride * img_src->u32Height * pxl_size_src;
    	mem_cfg.pool_info[0].blk_cnt = 1;
    	mem_cfg.pool_info[0].ddr_id = IVE_THRESH_USE_DDR;

        mem_cfg.pool_info[1].type = HD_COMMON_MEM_USER_BLK;
    	mem_cfg.pool_info[1].blk_size = img_dst->u32Stride * img_dst->u32Height * pxl_size_dst;
    	mem_cfg.pool_info[1].blk_cnt = 1;
    	mem_cfg.pool_info[1].ddr_id = IVE_THRESH_USE_DDR;
    }

    ret = hd_common_mem_init(&mem_cfg);
    if (HD_OK != ret) {
        printf("%s: hd_common_mem_init err: %d\r\n", __func__, ret);
    }

    return ret;
}

static HD_RESULT mem_uninit(void)
{
	HD_RESULT ret = HD_OK;
	hd_common_mem_uninit();
	return ret;
}

static HD_RESULT mem_alloc(MEM_RANGE *inout_buf, IVE_SRC_IMAGE_S *img_src, IVE_SRC_IMAGE_S *img_dst)
{
    UINT32 i;
    UINTPTR pa, va;
    HD_COMMON_MEM_VB_BLK blk;
    HD_RESULT ret = HD_OK;

    inout_buf[0].size = img_src->u32Stride * img_src->u32Height * get_pxl_size(img_src->enDataType);
    inout_buf[1].size = img_dst->u32Stride * img_dst->u32Height * get_pxl_size(img_dst->enDataType);
    for(i=0; i < 2; i++) {
        if(inout_buf[i].size==0) {
            printf("%s: zero in/out buffer size (%d)!?\n\r", i);
            return HD_ERR_NOT_SUPPORT;
        }
    }

    for(i=0; i < 2; i++) {
        inout_buf[i].blk = HD_COMMON_MEM_VB_INVALID_BLK;
    }

    for(i=0; i < 2; i++) {
        blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_BLK, inout_buf[i].size, IVE_THRESH_USE_DDR);
        if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
            printf("%s: hd_common_mem_get_block fail = %d, request %d\r\n", __func__, blk, inout_buf[i].size);
            ret =  HD_ERR_NG;
            return ret;
        }

        pa = hd_common_mem_blk2pa(blk);
    	if (pa == 0) {
    		printf("%s: hd_common_mem_blk2pa fail, blk = 0x%x\r\n", blk);
            ret = HD_ERR_NG;
    		goto mem_alloc_err;
    	}
        if (pa > 0) {
            va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, inout_buf[i].size);
            if (va == 0) {
                printf("%s: hd_common_mem_mmap fail\n\r");
                ret = HD_ERR_NG;
                goto mem_alloc_err;
            }
        }

		inout_buf[i].pa  = pa;
		inout_buf[i].va  = va;
		inout_buf[i].blk = blk;
#if IVE_THRESH_VERBOSE
		printf("mem_alloc: pa = 0x%x, va=0x%x, size =0x%x\r\n", inout_buf[i].pa, inout_buf[i].va, inout_buf[i].size);
#endif
    }
    return ret;

mem_alloc_err:
    for(; i > 0 ;) {
        i--;
        ret = hd_common_mem_release_block(inout_buf[i].blk);
		if (ret != HD_OK) {
			printf("%s: hd_common_mem_release_block fail %d\r\n", __func__, ret);
			ret =  HD_ERR_NG;
			return ret;
		}
    }
    return ret;
}

static HD_RESULT mem_dealloc(MEM_RANGE *inout_buf)
{
    UINT8 i;
    for(i=0; i < 2; i++) {
        if(inout_buf[i].va != 0) {
            hd_common_mem_munmap((void *)inout_buf[i].va, inout_buf[i].size);
        }
		if (inout_buf[i].blk != HD_COMMON_MEM_VB_INVALID_BLK) {
			hd_common_mem_release_block(inout_buf[i].blk);
		}
		inout_buf[i].pa   = 0x00;
		inout_buf[i].va   = 0x00;
		inout_buf[i].size = 0x00;
		inout_buf[i].blk  = HD_COMMON_MEM_VB_INVALID_BLK;
    }
    return HD_OK;
}

/*-------------------------------------------------------------------------------------------------------------------*/
/* Supporting Functions                                                                                              */
/*-------------------------------------------------------------------------------------------------------------------*/
#define ALIGN_FLOOR(value, base)        ((value) & ~((base)-1))                   ///< Align Floor
#define ALIGN_ROUND(value, base)        ALIGN_FLOOR((value) + ((base)/2), base)   ///< Align Round
#define ALIGN_CEIL(value, base)         ALIGN_FLOOR((value) + ((base)-1), base)   ///< Align Ceil

static UINT32 get_pxl_size(LIB_IVE_DATA_TYPE datatype)
{
    UINT32 psize = 0;
    switch(datatype) {
        case LIB_IVE_DATA_TYPE_U8C1:
        case LIB_IVE_DATA_TYPE_S8C1:
            psize = sizeof(UINT8);
            break;
        case LIB_IVE_DATA_TYPE_U16C1:
        case LIB_IVE_DATA_TYPE_S16C1:
            psize = sizeof(UINT16);
            break;
        default:
            printf("%s: not support data type %d\n\r", datatype);
            break;
    }
    return psize;
}

static UINT32 read_image(MEM_RANGE *in_buf, IVE_IMG_FILE_INFO *img_finfo)
{
    FILE *fd;
    UINT32 file_size=0, read_size=0;

    fd = fopen(img_finfo->filename, "rb");
	if (!fd) {
		printf("%s: cannot read %s\r\n", __func__, img_finfo->filename);
		return 0;
	}

	fseek (fd, 0, SEEK_END);
	file_size = ALIGN_CEIL_4(ftell(fd));
	fseek (fd, 0, SEEK_SET);

    if(img_finfo->width*img_finfo->height*get_pxl_size(img_finfo->datatype) != file_size) {
        printf("%s: expect image size %d != file size %d\n\r", __func__, img_finfo->width*img_finfo->height*sizeof(UINT8), file_size);
        fclose(fd);
        return 0;
    }

	read_size = fread ((void *)in_buf->va, 1, file_size, fd);
	if (read_size != file_size) {
		printf("%s: size mismatch, read_sz = %d, ftel_sz = %d\r\n", __func__, (int)read_size, (int)file_size);
	}
    fclose(fd);

    return read_size;
}

static UINT32 write_image(char *fname, IVE_DST_IMAGE_S *img)
{
    UINT32 file_size = 0;

    FILE *fp = fopen(fname, "wb");
    if(!fp) {
        printf("%s: fail to open file %s\n\r", fname);
        return file_size;
    }
    file_size = fwrite((const void *)img->u64Va, 1, img->u32Stride*img->u32Height*sizeof(UINT8), fp);
    fclose(fp);

    return file_size;
}

static void show_usage(void)
{
    printf("Usage: alg_ive_thresh mode out_file out_datatype in_file in_datatype width height thresh_lo thresh_hi vmin vmid vmax\n\r");
    printf("  @mode: thresholding mode, options=[1, 11]\n\r");
    printf("  @in_datatye/@out_datatype: input/output file data type\n\r");
    printf("   options: 0 (LIB_IVE_DATA_TYPE_U8C1) | 1 (LIB_IVE_DATA_TYPE_S8C1) | 8 (LIB_IVE_DATA_TYPE_U16C1) | 9 (LIB_IVE_DATA_TYPE_S16C1)\n\r");
    printf("  @thresh_lo/@thresh_hi: low threshold value and high threshold value\n\r");
    printf("  @vmin/@vmid/@vmax: the low/median/high output value\n\r");
}

/*-------------------------------------------------------------------------------------------------------------------*/
/* Main Function                                                                                                     */
/*  Excution examples for mode 1~11:                                                                                 */
/*  #1: alg_ive_thresh 1 /mnt//sd//output/lena_316x316_U8_mode1.bin 0 /mnt//sd//input/lena_316x316_U8.bin 0 316 316 80 160 0 128 255 */
/*  #2: alg_ive_thresh 2 /mnt//sd//output/lena_316x316_U8_mode2.bin 0 /mnt//sd//input/lena_316x316_U8.bin 0 316 316 80 160 0 128 255 */
/*  #3: alg_ive_thresh 3 /mnt//sd//output/lena_316x316_U8_mode3.bin 0 /mnt//sd//input/lena_316x316_U8.bin 0 316 316 80 160 0 128 255 */
/*  #4: alg_ive_thresh 4 /mnt//sd//output/lena_316x316_U8_mode4.bin 0 /mnt//sd//input/lena_316x316_U8.bin 0 316 316 80 160 0 128 255 */
/*  #5: alg_ive_thresh 5 /mnt//sd//output/lena_316x316_U8_mode5.bin 0 /mnt//sd//input/lena_316x316_U8.bin 0 316 316 80 160 0 128 255 */
/*  #6: alg_ive_thresh 6 /mnt//sd//output/stripe_480x320_S16_V-S16_mode6.bin 1 /mnt//sd//input/stripe_480x320_S16_V-S16.bin 9        */
/*                       480 320 -16000 16000 0 64 127                                                               */
/*  #7: alg_ive_thresh 7 /mnt//sd//output/stripe_480x320_S16_V-S8_mode7.bin 1 /mnt//sd//input/stripe_480x320_S16_V-S8.bin 9          */
/*                       480 320 -64 64 0 64 127                                                                     */
/*  #8: alg_ive_thresh 8 /mnt//sd//output/stripe_480x320_S16_V-S16_mode8.bin 0 /mnt//sd//input/stripe_480x320_S16_V-S16.bin 9        */
/*                       480 320 -16000 16000 0 128 255                                                              */
/*  #9: alg_ive_thresh 9 /mnt//sd//output/stripe_480x320_S16_V-U8_mode9.bin 0 /mnt//sd//input/stripe_480x320_S16_V-U8.bin 9          */
/*                       480 320 80 160 0 128 255                                                                    */
/*  #10: alg_ive_thresh 10 /mnt//sd//output/stripe_480x320_U16_V-U16_mode10.bin 0 /mnt//sd//input/stripe_480x320_U16_V-U16.bin 8     */
/*                       480 320 21845 43690 0 128 255                                                               */
/*  #11: alg_ive_thresh 11 /mnt//sd//output/stripe_480x320_U16_V-U8_mode11.bin 0 /mnt//sd//input/stripe_480x320_U16_V-U8.bin 8       */
/*                       480 320 80 160 0 128 255                                                                    */
/*-------------------------------------------------------------------------------------------------------------------*/
MAIN(argc, argv)
{
    UINT32 fsize;
    HD_RESULT ret;
    IVE_SRC_IMAGE_S img_src;
    IVE_DST_IMAGE_S img_dst;
    VOS_TICK    hw_tick_begin, hw_tick_end;
    MEM_RANGE inout_buf[2] = { 0 }; // [0]: input image; [1]: output image
	UINT32 handle = 0;
	BOOL instant = 1; // 0: non-blocking, 1: blocking
	IVE_QUERY_INFO_S query_info = {0};

    IVE_IMG_FILE_INFO input_info = {
        .filename = "//mnt//sd//input/lena_316x316_U8.bin",
        .width = 316,
        .height = 316,
        .datatype = LIB_IVE_DATA_TYPE_U8C1
    };

    IVE_IMG_FILE_INFO output_info = {
        .filename = "//mnt//sd//output/lena_316x316_U8_mode1.bin",
        .width = 316,
        .height = 316,
        .datatype = LIB_IVE_DATA_TYPE_U8C1
    };

    IVE_THRESH_CTRL_S ctrl_thresh = {
        .mode = 1,
        .s32LowThresh  = 80,
        .s32HighThresh = 160,
        .s16MinVal = 0,
        .s16MidVal = 128,
        .s16MaxVal = 255,
        .u8LutThresh = { 0 }
    };

    // argument parsing
    if(argc>1 && argc!=13) {
        printf("Wrong usage!\n\r");
        show_usage();
        return 0;
    } else if(argc==1) {
        show_usage();
    } else {
        ctrl_thresh.mode = (VENDOR_IVE_THRES_LUT_MODE)(atoi(argv[1]));

        if(strlen(argv[2]) <= IVE_THRESH_MAX_STR_LEN) {
            strncpy(output_info.filename, argv[2], IVE_THRESH_MAX_STR_LEN);
        } else {
            printf("filename (%s) exceeds allowed length (%d)\r\n", argv[2], IVE_THRESH_MAX_STR_LEN);
            return 0;
        }
        output_info.filename[IVE_THRESH_MAX_STR_LEN-1] = '\0';
        output_info.datatype = (LIB_IVE_DATA_TYPE)(atoi(argv[3]));

        if(strlen(argv[4]) <= IVE_THRESH_MAX_STR_LEN) {
            strncpy(input_info.filename, argv[4], IVE_THRESH_MAX_STR_LEN);
        } else {
            printf("filename (%s) exceeds allowed length (%d)\r\n", argv[4], IVE_THRESH_MAX_STR_LEN);
            return 0;
        }
        input_info.filename[IVE_THRESH_MAX_STR_LEN-1] = '\0';

        input_info.datatype = (LIB_IVE_DATA_TYPE)(atoi(argv[5]));
        input_info.width = atoi(argv[6]);
        input_info.height = atoi(argv[7]);
        ctrl_thresh.s32LowThresh = atoi(argv[8]);
        ctrl_thresh.s32HighThresh = atoi(argv[9]);
        ctrl_thresh.s16MinVal = (INT16)(atoi(argv[10]));
        ctrl_thresh.s16MidVal = (INT16)(atoi(argv[11]));
        ctrl_thresh.s16MaxVal = (INT16)(atoi(argv[12]));
        output_info.width = input_info.width;
        output_info.height = input_info.height;
    }
    printf("Run settings: %s %d %s %d %s %d %d %d %d %d %d %d %d\n\r", argv[0], ctrl_thresh.mode, output_info.filename, output_info.datatype,
        input_info.filename, input_info.datatype, input_info.width, input_info.height, ctrl_thresh.s32LowThresh, ctrl_thresh.s32HighThresh,
        ctrl_thresh.s16MinVal, ctrl_thresh.s16MidVal, ctrl_thresh.s16MaxVal);

    // usage checking
    if(input_info.width!=output_info.width || input_info.height!=output_info.height) {
        printf("Input and output image sizes must be the same (%d,%d)!=(%d,%d)\n\r", input_info.width, input_info.height, output_info.width, output_info.height);
        return 0;
    }
    if(input_info.datatype!=LIB_IVE_DATA_TYPE_U8C1 && input_info.datatype!=LIB_IVE_DATA_TYPE_U16C1 && input_info.datatype!=LIB_IVE_DATA_TYPE_S16C1) {
        printf("Input image not support datatype %d\n\r", input_info.datatype);
        return 0;
    }
    if(output_info.datatype!=LIB_IVE_DATA_TYPE_U8C1 && output_info.datatype!=LIB_IVE_DATA_TYPE_S8C1) {
        printf("Output image not support datatype %d\n\r", output_info.datatype);
        return 0;
    }

    // ive init
    ret = vendor_ive_init();
    if(ret != HD_OK) {
        printf("%s: vendor_ive_init error (%d)\r\n", __func__, ret);
        goto exit;
    }

    // set inout image info
    img_src.enDataType = input_info.datatype;
    img_src.u32Width   = input_info.width;
    img_src.u32Height  = input_info.height;
    img_src.u32Stride  = ALIGN_CEIL(input_info.width*get_pxl_size(img_src.enDataType), IVE_IN_ADDR_ALIGN);

    img_dst.enDataType = output_info.datatype;
    img_dst.u32Width   = output_info.width;
    img_dst.u32Height  = output_info.height;
    img_dst.u32Stride  = ALIGN_CEIL(output_info.width*get_pxl_size(img_dst.enDataType), IVE_IN_ADDR_ALIGN);

    // common initialization
    ret = hd_common_init(0);
    if(ret != HD_OK) {
        printf("hd_common_init fail = %d\n\r", ret);
        goto exit;
    }

    ret = mem_init(&img_src, &img_dst);
    if(ret != HD_OK) {
        printf("mem_init fail = %d\n\r", ret);
        goto exit_common_uninit;
    }

    ret = mem_alloc(inout_buf, &img_src, &img_dst);
    if(ret != HD_OK) {
        printf("mem_alloc fail = %d\n\r", ret);
        goto exit_mem_uninit;
    }

    // read image
    fsize = read_image(&inout_buf[0], &input_info);
    if(fsize == 0) {
        printf("read_image %s fail\n\r", input_info.filename);
        goto exit_mem_dealloc;
    }

    // ive erosion
    img_src.u64Pa = inout_buf[0].pa;
    img_src.u64Va = inout_buf[0].va;
    img_dst.u64Pa = inout_buf[1].pa;
    img_dst.u64Va = inout_buf[1].va;

#if IVE_THRESH_VERBOSE
    printf("src-img: %dWx%dH, stride %d, dtype %d, pa 0x%x, va 0x%x\n\r", img_src.u32Width, img_src.u32Height, img_src.u32Stride, img_src.enDataType, img_src.u64Pa, img_src.u64Va);
    printf("dst-img: %dWx%dH, stride %d, dtype %d, pa 0x%x, va 0x%x\n\r", img_dst.u32Width, img_dst.u32Height, img_dst.u32Stride, img_dst.enDataType, img_dst.u64Pa, img_dst.u64Va);
    printf("mode %d, thresh_lo %d, thresh_hi %d, vmin %d, vmid %d, vmax %d\n\r", ctrl_thresh.mode, ctrl_thresh.s32LowThresh, ctrl_thresh.s32HighThresh, ctrl_thresh.s16MinVal, ctrl_thresh.s16MidVal, ctrl_thresh.s16MaxVal);
#endif

    hd_common_mem_flush_cache((VOID *)img_src.u64Va, img_src.u32Stride*img_src.u32Height*get_pxl_size(img_src.enDataType));

    vos_perf_mark(&hw_tick_begin);
    ret = NVT_IVE_Thresh(&handle, &img_src, &img_dst, &ctrl_thresh, instant);
    vos_perf_mark(&hw_tick_end);
    printf("NVT_IVE_Thresh time = %d\r\n", vos_perf_duration(hw_tick_begin, hw_tick_end));

    // output result
    if(ret==HD_OK) {
        hd_common_mem_flush_cache((VOID *)img_dst.u64Va, img_dst.u32Height*img_dst.u32Stride*get_pxl_size(img_dst.enDataType));
        fsize = write_image(output_info.filename, &img_dst);
        if(fsize!=0) printf("write out results %s, %d bytes\n\r", output_info.filename, fsize);
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
exit_mem_dealloc:
    ret = mem_dealloc(inout_buf);
    if(ret != HD_OK) {
        printf("mem_dealloc fail = %d\n\r", ret);
    }

exit_mem_uninit:
    ret = mem_uninit();
    if(ret != HD_OK) {
        printf("mem_uninit fail = %d\n\r", ret);
    }

exit_common_uninit:
    ret = hd_common_uninit();
    if(ret != HD_OK) {
        printf("hd_common_uninit fail=%d\n", ret);
    }

    ret = vendor_ive_uninit();
    if (HD_OK != ret) {
        printf("%s: vendor_ive_uninit, uninit failed (%d)\n\r", __func__, ret);
    }

exit:
    return 0;
}
