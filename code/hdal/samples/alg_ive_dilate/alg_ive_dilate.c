/**
	@brief Source file of vendor ai net sample code.

	@file alg_ive_dilate.c

	@ingroup alg_ive_sample

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Including Files                                                                                                   */
/*-------------------------------------------------------------------------------------------------------------------*/
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
#define IVE_DILATE_USE_DDR          DDR_ID0
#define IVE_DILATE_VERBOSE          0
#define IVE_DILATE_MAX_STR_LEN      256

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


/*-------------------------------------------------------------------------------------------------------------------*/
/* Memory Releated Functions                                                                                         */
/*-------------------------------------------------------------------------------------------------------------------*/
static HD_RESULT mem_init(IVE_SRC_IMAGE_S *img_src, IVE_SRC_IMAGE_S *img_dst)
{
    HD_RESULT ret = HD_OK;

    HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

    if(img_src->u32Stride*img_src->u32Height != img_dst->u32Stride*img_dst->u32Height) {
        printf("%s: expect same dimensions of source and target images (%dx%d != %dx%d)\n\r", __func__, img_src->u32Stride, img_src->u32Height, img_dst->u32Stride, img_dst->u32Height);
        return HD_ERR_NOT_SUPPORT;
    }

	mem_cfg.pool_info[0].type = HD_COMMON_MEM_USER_BLK;
	mem_cfg.pool_info[0].blk_size = img_src->u32Stride * img_src->u32Height * sizeof(UINT8);
	mem_cfg.pool_info[0].blk_cnt = 2;
	mem_cfg.pool_info[0].ddr_id = IVE_DILATE_USE_DDR;

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

    inout_buf[0].size = img_src->u32Stride * img_src->u32Height * sizeof(UINT8);
    inout_buf[1].size = img_dst->u32Stride * img_dst->u32Height * sizeof(UINT8);

    for(i=0; i < 2; i++) {
        inout_buf[i].blk = HD_COMMON_MEM_VB_INVALID_BLK;
    }
    for(i=0; i < 2; i++) {
        blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_BLK, inout_buf[i].size, IVE_DILATE_USE_DDR);
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
#if IVE_DILATE_VERBOSE
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
#define ALIGN_FLOOR(value, base)  ((value) & ~((base)-1))                   ///< Align Floor
#define ALIGN_ROUND(value, base)  ALIGN_FLOOR((value) + ((base)/2), base)   ///< Align Round
#define ALIGN_CEIL(value, base)   ALIGN_FLOOR((value) + ((base)-1), base)   ///< Align Ceil

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

    if(img_finfo->width*img_finfo->height*sizeof(UINT8) != file_size) {
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
    printf("Usage: alg_ive_dilate out_file in_file datatype width height\n\r");
    printf("  @datatype: input/output file data type\n\r");
    printf("     option: 0 (LIB_IVE_DATA_TYPE_U8C1)\n\r");
}

/*-------------------------------------------------------------------------------------------------------------------*/
/* Main Function                                                                                                     */
/*  Excution example:                                                                                                */
/*   alg_ive_dilate ./output/lena_316x316_U8_t_dilate.bin ./input/lena_316x316_U8_t.bin 0 316 316                    */
/*-------------------------------------------------------------------------------------------------------------------*/
MAIN(argc, argv)
{
    UINT32 fsize;
    HD_RESULT ret;
    IVE_SRC_IMAGE_S img_src;
    IVE_DST_IMAGE_S img_dst;
    VOS_TICK    hw_tick_begin, hw_tick_end;
    MEM_RANGE inout_buf[2] = { 0 }; // [0]: input image; [1]: output image

    CHAR output_fname[IVE_DILATE_MAX_STR_LEN] = "//mnt//sd//output/lena_316x316_U8_t_dilate.bin";

    IVE_IMG_FILE_INFO input_info = {
        .filename = "//mnt//sd//input/lena_316x316_U8_t.bin",
        .width = 316,
        .height = 316,
        .datatype = LIB_IVE_DATA_TYPE_U8C1
    };
    IVE_MORPH_CTRL_S kernel = {
        .u8Mask = {
            0, 0, 1, 0, 0,
            0, 1, 1, 1, 0,
            1, 1,    1, 1,
            0, 1, 1, 1, 0,
            0, 0, 1, 0, 0
        }
    };
	UINT32 handle = 0;
	BOOL instant = 1; // 0: non-blocking, 1: blocking
	IVE_QUERY_INFO_S query_info = {0};

    // argument parsing
    if(argc>1 && argc!=6) {
        show_usage();
        return 0;
    } else if(argc==1) {
        show_usage();
    } else {
        if(strlen(argv[1]) <= IVE_DILATE_MAX_STR_LEN) {
            strncpy(output_fname, argv[1], IVE_DILATE_MAX_STR_LEN);
        } else {
            printf("filename (%s) exceeds allowed length (%d)\r\n", argv[1], IVE_DILATE_MAX_STR_LEN);
            return 0;
        }
        output_fname[IVE_DILATE_MAX_STR_LEN-1] = '\0';

        if(strlen(argv[2]) <= IVE_DILATE_MAX_STR_LEN) {
            strncpy(input_info.filename, argv[2], IVE_DILATE_MAX_STR_LEN);
        } else {
            printf("filename (%s) exceeds allowed length (%d)\r\n", argv[2], IVE_DILATE_MAX_STR_LEN);
            return 0;
        }
        input_info.filename[IVE_DILATE_MAX_STR_LEN-1] = '\0';

        input_info.datatype = (LIB_IVE_DATA_TYPE)(atoi(argv[3]));
        input_info.width    = atoi(argv[4]);
        input_info.height   = atoi(argv[5]);
    }
    printf("Run settings: %s %s %s %d %d %d\n\r", argv[0], output_fname, input_info.filename, input_info.datatype, input_info.width, input_info.height);

    // usage checking
    if(input_info.datatype!=LIB_IVE_DATA_TYPE_U8C1) {
        printf("image data type must be %d (LIB_IVE_DATA_TYPE_U8C1)\n\r", (int)LIB_IVE_DATA_TYPE_U8C1);
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
    img_src.u32Stride  = ALIGN_CEIL(input_info.width, IVE_IN_ADDR_ALIGN);

    img_dst.enDataType = input_info.datatype;
    img_dst.u32Width   = input_info.width;
    img_dst.u32Height  = input_info.height;
    img_dst.u32Stride  = img_src.u32Stride;

#if IVE_DILATE_VERBOSE
    printf("src img: type %d, w %d, h %d, stride %d\n\r", img_src.enDataType, img_src.u32Width, img_src.u32Height, img_src.u32Stride);
    printf("dst img: type %d, w %d, h %d, stride %d\n\r", img_dst.enDataType, img_dst.u32Width, img_dst.u32Height, img_dst.u32Stride);
#endif

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

#if IVE_DILATE_VERBOSE
    UINT8 *p_u8 = (UINT8*)inout_buf[0].va;
    printf("%s: src img ", __func__);
    for(i=0; i<16; i++) printf("%x ", p_u8[i]);
    printf("... ");
    for(i=inout_buf[0].size-17; i<inout_buf[0].size; i++) printf("%x ", p_u8[i]);
    printf("\n\r");
#endif

    // ive erosion
    img_src.u64Pa = inout_buf[0].pa;
    img_src.u64Va = inout_buf[0].va;
    img_dst.u64Pa = inout_buf[1].pa;
    img_dst.u64Va = inout_buf[1].va;

    hd_common_mem_flush_cache((VOID *)img_src.u64Va, img_src.u32Stride*img_src.u32Height*sizeof(UINT8));

    vos_perf_mark(&hw_tick_begin);
    ret = NVT_IVE_Dilate(&handle, &img_src, &img_dst, &kernel, instant);
    vos_perf_mark(&hw_tick_end);
	printf("NVT_IVE_Dilate time = %d\r\n", vos_perf_duration(hw_tick_begin, hw_tick_end)); 

    // output result
    if(ret==HD_OK) {
        hd_common_mem_flush_cache((VOID *)img_dst.u64Va, img_dst.u32Height*img_dst.u32Stride*sizeof(UINT8));
        fsize = write_image(output_fname, &img_dst);
        if(fsize!=0) printf("write out results %s, %d bytes\n\r", output_fname, fsize);
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
