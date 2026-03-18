/**
	@brief Sample code of gfx.\n

	@file gfx_only.c

	@author YongChang Qui

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "hdal.h"
#include "vendor_gfx.h"
#include "hd_debug.h"

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
#include <kwrap/util.h>		//for sleep API
#define sleep(x)    			vos_util_delay_ms(1000*(x))
#define msleep(x)    			vos_util_delay_ms(x)
#define usleep(x)   			vos_util_delay_us(x)
#include <kwrap/examsys.h> 	//for MAIN(), GETCHAR() API
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(hd_gfx_only, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

///////////////////////////////////////////////////////////////////////////////

//setup all gfx's buffer to demo purpose
static HD_RESULT mem_init(HD_COMMON_MEM_VB_BLK *buf_blk, UINT32 buf_size, UINTPTR *buf_pa)
{
	HD_RESULT                    ret;
	HD_COMMON_MEM_INIT_CONFIG    mem_cfg = {0};

	mem_cfg.pool_info[0].type     = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[0].blk_size = buf_size;
	mem_cfg.pool_info[0].blk_cnt  = 1;
	mem_cfg.pool_info[0].ddr_id   = DDR_ID0;

	//register gfx's buffer to common memory pool
	ret = hd_common_mem_init(&mem_cfg);
	if(ret != HD_OK){
		printf("fail to allocate %d bytes from common pool\n", buf_size);
		return ret;
	}

	//get gfx's buffer block
	*buf_blk = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, buf_size, DDR_ID0);
	if (*buf_blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("get block fail\r\n");
		return HD_ERR_NOMEM;
	}

	//translate gfx's buffer block to physical address
	*buf_pa = hd_common_mem_blk2pa(*buf_blk);
	if (*buf_pa == 0) {
		printf("blk2pa fail, buf_blk = 0x%x\r\n", *buf_blk);
		return HD_ERR_NOMEM;
	}

	return HD_OK;
}

static HD_RESULT mem_exit(void)
{
	HD_RESULT ret = HD_OK;
	hd_common_mem_uninit();
	return ret;
}

///////////////////////////////////////////////////////////////////////////////

static HD_RESULT draw_mosaic(HD_VIDEO_FRAME *frame, UINT32 x, UINT32 y, UINT32 w, UINT32 h)
{
    int                              i;
    VENDOR_GFX_GRPH_TRIGGER_PARAM    trigger_param = {0};
    HD_RESULT                        ret;
    int                              order = 0;
    UINTPTR                          y_addr = 0, uv_addr = 0;
	UINT32                           blksz = 8;
	
	if(frame == NULL){
		printf("frame is NULL\n");
		return HD_ERR_PARAM;
	}

	if(frame->pxlfmt != HD_VIDEO_PXLFMT_YUV420){
		printf("only yuv420 is supported\n");
		return HD_ERR_PARAM;
	}

	if(frame->dim.w == 0 || frame->dim.h == 0 || frame->loff[0] == 0 || frame->loff[1] == 0){
		printf("invalid w(%d) or h(%d) or loff(%d,%d)\n", 
					frame->dim.w, frame->dim.h, frame->loff[0], frame->loff[1]);
		return HD_ERR_PARAM;
	}

	if(frame->phy_addr[0] == 0 || frame->phy_addr[1] == 0){
		printf("invalid addr y(%x) or uv(%x)\n", frame->phy_addr[0], frame->phy_addr[1]);
		return HD_ERR_PARAM;
	}
	
	if((x + w) > frame->dim.w){
		printf("mosaic x(%d) + w(%d) > image w(%d)\n", x, w, frame->dim.w);
		return HD_ERR_PARAM;
	}
	
	if((y + h) > frame->dim.h){
		printf("mosaic y(%d) + h(%d) > image h(%d)\n", y, h, frame->dim.h);
		return HD_ERR_PARAM;
	}
	
	if((x & 0x03) || (y & 0x01)){
		printf("invalid x(%d) or y(%d), x must be 4 aligned, y must be 2 aligned\n", x, y);
		return HD_ERR_PARAM;
	}
    
    if(blksz == 8){
        order = 3;
    }else if(blksz == 16){
        order = 4;
    }else if(blksz == 32){
        order = 5;
    }else if(blksz == 64){
        order = 6;
    }else{
        printf("only 8,16,32,64 mosiac are supported\n");
        return -1;
    }
    
    y_addr = frame->phy_addr[0];
	y_addr += (x + y*frame->loff[0]);
    uv_addr = frame->phy_addr[1];
	uv_addr += (x + (y/2)*frame->loff[1]);

    //draw mosaic to destination with sub image
    for(i = 0 ; i < 2 ; ++i){
        memset(&trigger_param, 0, sizeof(VENDOR_GFX_GRPH_TRIGGER_PARAM));
        trigger_param.command = VENDOR_GFX_GRPH_CMD_VCOV;
        if(i == 0)
            trigger_param.format = VENDOR_GFX_GRPH_FORMAT_8BITS;
        else
            trigger_param.format = VENDOR_GFX_GRPH_FORMAT_16BITS;
        trigger_param.property[0].en = 1;
        trigger_param.property[0].id = VENDOR_GFX_GRPH_PROPERTY_ID_YUVFMT;
        trigger_param.property[0].data.property = 1;
        trigger_param.property[1].en = 1;
        trigger_param.property[1].id = VENDOR_GFX_GRPH_PROPERTY_ID_QUAD_PTR;
        trigger_param.property[1].data.quad.top_left.x = 0;
        trigger_param.property[1].data.quad.top_left.y = 0;
        trigger_param.property[1].data.quad.top_right.x = w;
        trigger_param.property[1].data.quad.top_right.y = 0;
        trigger_param.property[1].data.quad.bottom_right.x = w;
        trigger_param.property[1].data.quad.bottom_right.y = h;
        trigger_param.property[1].data.quad.bottom_left.x = 0;
        trigger_param.property[1].data.quad.bottom_left.y = h;
        trigger_param.property[1].data.quad.blend_en = 0;
        trigger_param.property[1].data.quad.alpha = 0;
        if(i == 0){
            trigger_param.property[1].data.quad.mosaic_width = ((unsigned int)1 << order);
        }else{
            trigger_param.property[1].data.quad.mosaic_width = (((unsigned int)1 << order) << 1);
        }
        trigger_param.property[1].data.quad.mosaic_height = ((unsigned int)1 << order);

        trigger_param.images[0].img_id = VENDOR_GFX_GRPH_IMG_ID_A;
        if(i == 0)
            trigger_param.images[0].dram_addr = y_addr;
        else
            trigger_param.images[0].dram_addr = uv_addr;
        trigger_param.images[0].lineoffset = frame->loff[i];
        trigger_param.images[0].width = w;
        if(i == 0)
            trigger_param.images[0].height = h;
        else
            trigger_param.images[0].height = (h/2);
        trigger_param.images[1].img_id = VENDOR_GFX_GRPH_IMG_ID_C;
        if(i == 0)
            trigger_param.images[1].dram_addr = y_addr;
        else
            trigger_param.images[1].dram_addr = uv_addr;
        trigger_param.images[1].lineoffset = frame->loff[i];

        ret = vendor_gfx_grph_trigger_no_flush(0, &trigger_param);
        if(ret != HD_OK){
            printf("vendor_gfx_grph_trigger_no_flush fail=%d\n", ret);
            return -1;
        }
    }

    return ret;
}

static HD_RESULT test_draw_mosaic(UINTPTR buf_pa, UINT32 buf_size)
{
	int                 fd          = -1;
	void                *va         = NULL;
	HD_RESULT           ret;
	HD_VIDEO_FRAME      frame       = {0};
	UINT32              size        = 0;
	int                 len         = 0;
	HD_VIDEO_FRAME      video_frame = {0};

	//calculate image's buffer size
	frame.sign = MAKEFOURCC('V','F','R','M');
	frame.pxlfmt  = HD_VIDEO_PXLFMT_YUV420;
	frame.dim.h   = 1080;
	frame.loff[0] = 1920;
	frame.loff[1] = 1920;
	size = hd_common_mem_calc_buf_size(&frame);
	if(!size){
		printf("hd_common_mem_calc_buf_size() fail\n");
		return -1;
	}
	if(size > buf_size){
		printf("required size(%d) > allocated size(%d)\n", size, buf_size);
		return -1;
	}

	//map gfx's buffer from physical address to user space
	va = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, buf_pa, buf_size);
	if(!va){
		printf("hd_common_mem_mmap() fail\n");
		return -1;
	}

	//load image from sd card
	fd = open("/mnt/sd/video_frm_1920_1080_1_yuv420.dat", O_RDONLY);
	if(fd == -1){
		printf("fail to open /mnt/sd/video_frm_1920_1080_1_yuv420.dat\n");
		printf("video_frm_1920_1080_1_yuv420.dat is in SDK/code/hdal/samples/pattern/video_frm_1920_1080_1_yuv420.dat\n");
		ret = HD_ERR_NOT_FOUND;
		goto exit;
	}

	//coverity[string_null_argument]
	len = read(fd, va, size);
	close(fd);
	if(len != (int)size){
		printf("fail to read /mnt/sd/video_frm_1920_1080_1_yuv420.dat\n");
		ret = HD_ERR_SYS;
		goto exit;
	}

	video_frame.sign        = MAKEFOURCC('V','F','R','M');
	video_frame.ddr_id      = DDR_ID0;
	video_frame.pxlfmt      = HD_VIDEO_PXLFMT_YUV420;
	video_frame.dim.w       = 1920;
	video_frame.dim.h       = 1080;
	video_frame.count       = 0;
	video_frame.loff[0]     = 1920; // Y
	video_frame.loff[1]     = 1920; // UV
	video_frame.phy_addr[0] = buf_pa;              // Y
	video_frame.phy_addr[1] = buf_pa + 1920*1080;  // UV pack
	
	ret = draw_mosaic(&video_frame, 600, 200, 400, 800);
	if(ret != HD_OK){
		printf("fail to draw mosaic\n");
		goto exit;
	}

	//save the result image to sd card
	fd = open("/mnt/sd/hd_gfx_draw_mosaic_1920_1080_1.yuv420", O_WRONLY | O_CREAT, 0644);
	if(fd == -1){
		printf("fail to open /mnt/sd/hd_gfx_draw_mosaic_1920_1080_1.yuv420\n");
		ret = HD_ERR_SYS;
		goto exit;
	}

	len = write(fd, va, size);
	close(fd);
	if(len != (int)size){
		printf("fail to write /mnt/sd/hd_gfx_draw_mosaic_1920_1080_1.yuv420\n");
		ret = HD_ERR_SYS;
		goto exit;
	}
	printf("result is /mnt/sd/hd_gfx_draw_mosaic_1920_1080_1.yuv420\n");

exit:

	if(hd_common_mem_munmap(va, buf_size))
		printf("fail to unmap va(%lx)\n", (unsigned long)va);

	return ret;
}

///////////////////////////////////////////////////////////////////////////////

typedef struct _GFX {

	// (1)
	HD_COMMON_MEM_VB_BLK buf_blk;
	UINT32               buf_size;
	UINTPTR              buf_pa;
	
} GFX;

MAIN(argc, argv)
{
    HD_RESULT    ret;
	GFX          gfx = {0};

	gfx.buf_blk = 0;
	gfx.buf_size = 16 * 1024 * 1024;
	gfx.buf_pa = 0;

	ret = hd_common_init(0);
    if(ret != HD_OK) {
        printf("common fail=%d\n", ret);
        goto exit;
    }

	ret = mem_init(&gfx.buf_blk, gfx.buf_size, &gfx.buf_pa);
    if(ret != HD_OK) {
        printf("mem fail=%d\n", ret);
        goto exit;
    }

    ret = hd_gfx_init();
    if(ret != HD_OK) {
        printf("init fail=%d\n", ret);
        goto exit;
    }

	ret = test_draw_mosaic(gfx.buf_pa, gfx.buf_size);
	if(ret != HD_OK)
		printf("drawing mosaic fail=%d\n", ret);

exit:

	if(gfx.buf_blk)
		if(HD_OK != hd_common_mem_release_block(gfx.buf_blk))
			printf("hd_common_mem_release_block() fail\n");

	ret = hd_gfx_uninit();
	if(ret != HD_OK)
		printf("uninit fail=%d\n", ret);

	ret = mem_exit();
	if(ret != HD_OK)
		printf("mem fail=%d\n", ret);

	ret = hd_common_uninit();
	if(ret != HD_OK)
		printf("common fail=%d\n", ret);

	return 0;
}
