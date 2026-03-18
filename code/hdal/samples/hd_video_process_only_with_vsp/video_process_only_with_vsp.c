/**
	@brief Sample code of videoprocess with vsp.\n

	@file video_process_only_with_vsp.c

	@author Ben Wang

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "hdal.h"
#include "hd_debug.h"
#include "vendor_videoprocess.h"
#include "vendor_vpe.h"
#include <kwrap/examsys.h> 	//for MAIN(), GETCHAR() API

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
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(hd_video_process_only_with_vsp, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define DEBUG_MENU 		1

#define CHKPNT			printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
#define DBGH(x)			printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x)			printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)

///////////////////////////////////////////////////////////////////////////////

//header
#define DBGINFO_BUFSIZE()	(0x200)

//RAW
#define VDO_RAW_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
//NRX: RAW compress: Only support 12bit mode
#define RAW_COMPRESS_RATIO 50
#define VDO_NRX_BUFSIZE(w, h)           (ALIGN_CEIL_4(ALIGN_CEIL_64(w) / 64 * ((24*RAW_COMPRESS_RATIO+99)/100) * 4 * (h)))
//CA for AWB
#define VDO_CA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 3) << 1)
//LA for AE
#define VDO_LA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 1) << 1)

//YUV
#define VDO_YUV_BUFSIZE(w, h, pxlfmt)	(ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
//NVX: YUV compress
#define YUV_COMPRESS_RATIO 75
#define VDO_NVX_BUFSIZE(w, h, pxlfmt)	(VDO_YUV_BUFSIZE(w, h, pxlfmt) * YUV_COMPRESS_RATIO / 100)

///////////////////////////////////////////////////////////////////////////////

#define VSP_BLEND_FRAME_NUM      2
#define VSP_MAX_BLEND_FRAME_NUM  4

//vsp flow vpe input image size
#define VSP_IN_SIZE_W      1440
#define VSP_IN_SIZE_H      1440

#define VSP_SCALE_OUT_SIZE_W1 1440
#define VSP_SCALE_OUT_SIZE_W2 1440
#define VSP_SCALE_OUT_SIZE_W3 0
#define VSP_SCALE_OUT_SIZE_W4 0
#define VSP_SCALE_OUT_SIZE_H  1440

//vsp flow overlap ratio, base is 1000
#define VSP_OVERLAP_RATIO  100
#define VSP_BLEND_WIDTH    VSP_IN_SIZE_W*VSP_OVERLAP_RATIO/1000

//total main out image
#define VSP_VPE_OUT1_W     VSP_SCALE_OUT_SIZE_W1+VSP_SCALE_OUT_SIZE_W2+VSP_SCALE_OUT_SIZE_W3+VSP_SCALE_OUT_SIZE_W4
#define VSP_VPE_OUT1_H     VSP_SCALE_OUT_SIZE_H

//total blend output image
#define VSP_VPE_OUT2_W     (VSP_BLEND_FRAME_NUM*VSP_BLEND_WIDTH*2)
#define VSP_VPE_OUT2_H     VSP_SCALE_OUT_SIZE_H

#define VSP_FUSION_SIZE_W  VSP_BLEND_WIDTH
#define VSP_FUSION_SIZE_H  VSP_SCALE_OUT_SIZE_H

#define VSP_OUT_SIZE_W     1440
#define VSP_OUT_SIZE_H     1440

#define DRE_WORK_BUF_SIZE   (ALIGN_CEIL_16(VSP_BLEND_WIDTH)*VSP_SCALE_OUT_SIZE_H*4)


#define YUV_IN_BLK_SIZE    (DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(VSP_IN_SIZE_W, VSP_IN_SIZE_H, HD_VIDEO_PXLFMT_YUV420))
#define YUV_OUT_BLK_SIZE   (DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(VSP_OUT_SIZE_W, VSP_OUT_SIZE_H, HD_VIDEO_PXLFMT_YUV420))


#define VDO_DDR_ID		DDR_ID0

#define VSP_DEV_CNT        2
#define OUT_VSP_COUNT      1


#define OUT0_BG_W ALIGN_FLOOR(VSP_OUT_SIZE_W, 16)
#define OUT0_BG_H ALIGN_FLOOR(VSP_OUT_SIZE_H, 2)

#define TEST_PATTERN_PATH "/mnt/sd/vsp/img1_y.bin"

// iq parameter
#define VPE_EFFECT_NONE                   0
#define VPE_EFFECT_SHARPEN       0x00000001
#define VPE_EFFECT_GDC_CTL       0x00000002
#define VPE_EFFECT_GDC           0x00000004
#define VPE_EFFECT_2DLUT         0x00000008
#define VPE_EFFECT_DCTG          0x00000010

static UINT32 g_effect = VPE_EFFECT_GDC_CTL|VPE_EFFECT_2DLUT|VPE_EFFECT_DCTG;
static VPET_2DLUT_PARAM lut2d = {0};



static int    g_quit = 0;
static BOOL   g_save_file = TRUE;

///////////////////////////////////////////////////////////////////////////////

typedef struct _VIDEO_PROCESS {
	UINT32 id;
	HD_DIM  proc_max_dim;
	HD_VIDEOPROC_SYSCAPS proc_syscaps;
	HD_PATH_ID proc_ctrl;
	HD_PATH_ID proc_path[OUT_VSP_COUNT];
	HD_VIDEOPROC_OUT proc_out[OUT_VSP_COUNT];
	UINT32 	shot_count;
	UINTPTR fusion_blk;
	UINTPTR fusion_pa;
	UINTPTR fusion_va;
	UINTPTR fusion_work_buf_blk;
	UINTPTR fusion_work_buf_pa;
	UINT32  isp_id;
	UINT32  ptz_isp_id;	//vsp need second isp_id for ptz parameters, ptz_isp_id must = isp_id+1
	UINT32  sleep_sec;
	pthread_t  vsp_thread_id;
	pthread_t  pull_thread_id;
} VIDEO_PROCESS;

///////////////////////////////////////////////////////////////////////////////


static HD_RESULT mem_init(void)
{
	HD_RESULT              ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
	UINT32                   i = 0;

	//vsp input buffer
	mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(VSP_IN_SIZE_W, VSP_IN_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[i].blk_cnt = VSP_DEV_CNT * VSP_BLEND_FRAME_NUM;
	mem_cfg.pool_info[i].ddr_id = VDO_DDR_ID;

	//vpe out temp buffer1
	i++;
	mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(ALIGN_CEIL_16(VSP_VPE_OUT1_W-VSP_VPE_OUT2_W/2), VSP_VPE_OUT1_H, HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[i].blk_cnt = VSP_DEV_CNT*2;
	mem_cfg.pool_info[i].ddr_id = VDO_DDR_ID;

	//vpe out temp buffer2
	i++;
	mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(ALIGN_CEIL_16(VSP_VPE_OUT2_W), VSP_VPE_OUT2_H, HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[i].blk_cnt = VSP_DEV_CNT*2;
	mem_cfg.pool_info[i].ddr_id = VDO_DDR_ID;

	//vsp out buffer
	i++;
	mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(VSP_OUT_SIZE_W, VSP_OUT_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[i].blk_cnt = VSP_DEV_CNT*3;
	mem_cfg.pool_info[i].ddr_id = VDO_DDR_ID;

	// dre fusion buffer
	i++;
	mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[i].blk_size = 2*VDO_YUV_BUFSIZE(VSP_FUSION_SIZE_W, VSP_FUSION_SIZE_H, HD_VIDEO_PXLFMT_Y8);
	mem_cfg.pool_info[i].blk_cnt = VSP_DEV_CNT;
	mem_cfg.pool_info[i].ddr_id = VDO_DDR_ID;

	// DRE working buffer
	i++;
	mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[i].blk_size = DRE_WORK_BUF_SIZE;
	mem_cfg.pool_info[i].blk_cnt = 1;
	mem_cfg.pool_info[i].ddr_id = VDO_DDR_ID;
	ret = hd_common_mem_init(&mem_cfg);
	return ret;
}

static HD_RESULT mem_exit(VIDEO_PROCESS *p_stream)
{
	UINT32 i;
	HD_RESULT ret = HD_OK;
	VIDEO_PROCESS *p_s = p_stream;

	for (i = 0; i < VSP_DEV_CNT; i++) {
		if (p_s->fusion_blk) {
			hd_common_mem_release_block(p_s->fusion_blk);
		}
		if (p_s->fusion_work_buf_blk) {
			hd_common_mem_release_block(p_s->fusion_work_buf_blk);
		}
		p_s++;
	}

	hd_common_mem_uninit();
	return ret;
}

static HD_RESULT set_effect_cfg(VIDEO_PROCESS *p_stream)
{
	static BOOL  is_vpe_init = FALSE;
	HD_RESULT ret = HD_OK;
	FILE     *fp;
	UINT32    frame_idx;

	if (!is_vpe_init) {
		if ((ret = vendor_vpe_init()) != HD_OK) {
			printf("vendor_vpe_init failed!(%d)\r\n", ret);
			return ret;
		}
	}
	is_vpe_init = TRUE;
	if (g_effect & VPE_EFFECT_GDC_CTL) {
		VPET_DCE_CTL_PARAM dce_ctl = {0};

		for (frame_idx = 0; frame_idx < VSP_BLEND_FRAME_NUM; frame_idx++) {
			dce_ctl.id = ((frame_idx << 8) | p_stream->isp_id);
			dce_ctl.dce_ctl.enable = 1;
			dce_ctl.dce_ctl.dce_mode = VPE_ISP_DCE_MODE_2DLUT_ONLY;
			// set vpe
			vendor_vpe_set_cmd(VPET_ITEM_DCE_CTL_PARAM, &dce_ctl);
		}
		// set vpe ptz parameters
		dce_ctl.id = p_stream->ptz_isp_id;
		dce_ctl.dce_ctl.enable = 1;
		dce_ctl.dce_ctl.dce_mode = VPE_ISP_DCE_MODE_2DLUT_DCTG;
		printf("dce dctg isp_id = %d\r\n", dce_ctl.id);
		vendor_vpe_set_cmd(VPET_ITEM_DCE_CTL_PARAM, &dce_ctl);
	}
	if (g_effect & VPE_EFFECT_2DLUT) {

		char lut_path[VSP_MAX_BLEND_FRAME_NUM][64] =
			{"/mnt/sd/vsp/Equirectangular_img1_2dlut.bin",
		     "/mnt/sd/vsp/Equirectangular_img2_2dlut.bin",
		     "/mnt/sd/vsp/Equirectangular_img3_2dlut.bin",
		     "/mnt/sd/vsp/Equirectangular_img4_2dlut.bin"};

		UINT32 lut_size = 17680; /* 68*65 table */
		UINT32 read_len;

		for (frame_idx = 0; frame_idx < VSP_BLEND_FRAME_NUM; frame_idx++) {

			lut2d.id = ((frame_idx << 8) | p_stream->isp_id);
			lut2d.lut2d.lut_sz = VPE_ISP_2DLUT_SZ_65X65;
			if ((fp = fopen(lut_path[frame_idx], "rb")) == NULL) {
				printf("open file (%s) fail !!....\r\nPlease copy test pattern to SD Card !!\r\n\r\n", lut_path);
				return -1;
			}
			setvbuf(fp, NULL, _IONBF, 0);
			read_len = fread((void *)(lut2d.lut2d.lut), 1, lut_size, fp);
			if (read_len != lut_size) {
				printf("reading 2dlut error\n");
			}
			// close input image file
			fclose(fp);
			// set vpe
			vendor_vpe_set_cmd(VPET_ITEM_2DLUT_PARAM, &lut2d);
		}
		#if 0
		lut2d.id = p_stream->isp_id;
		lut2d.lut2d.lut_sz = VPE_ISP_2DLUT_SZ_65X65;
		vendor_vpe_set_cmd(VPELT_ITEM_2DLUT_PARAM, &lut2d);
		#endif

	}

	//{1, 0, 3, 2592,2, 0, 0, 0, 0, 0, 0, 0, 0},
	if (g_effect & VPE_EFFECT_DCTG ) {
		VPET_DCTG_CTRL dctg = {0};
		// set vpe ptz parameters
		dctg.id = p_stream->ptz_isp_id;
		vendor_vpe_get_cmd(VPET_ITEM_DCTG_CTRL, &dctg);
		dctg.dctg.mode_sel = VPE_ISP_DCTG_MODE_PTZ;
		dctg.dctg.ptz_param.long_aov = 1200;
		dctg.dctg.ptz_param.lati_aov = 1200;
		if (p_stream->id == 0) {
			dctg.dctg.ptz_param.pan_angle = -1800;
		} else {
			dctg.dctg.ptz_param.pan_angle = 0;
		}
		dctg.dctg.ptz_param.tilt_angle = 0;
		dctg.dctg.ptz_param.rot_angle = 0;
		dctg.dctg.ptz_param.zoom_step = 0;
		dctg.dctg.ptz_param.cam_long_aov = 1800;
		dctg.dctg.ptz_param.cam_lati_aov = 1800;
		dctg.dctg.ptz_param.cam_w = VSP_OUT_SIZE_W;
		dctg.dctg.ptz_param.cam_h = VSP_OUT_SIZE_H;
		dctg.dctg.ptz_param.stitch_overlap_angle = 100;
		vendor_vpe_set_cmd(VPET_ITEM_DCTG_CTRL, &dctg);
	}
	return ret;
}

static void init_fusion_tbl(UINTPTR va, UINT32 img_w, UINT32 img_h)
{
	UINT32 i;
	UINTPTR temp_va = va;

	for (i = 0; i < img_h; i++) {
		memset((void *)temp_va, 255, img_w/2);
		temp_va += img_w/2;
		memset((void *)temp_va, 0, img_w/2);
		temp_va += img_w/2;
	}
	printf("fusion w = %d, h = %d, va = 0x%lx, temp_va = 0x%lx\r\n", img_w, img_h, va, temp_va);
}

///////////////////////////////////////////////////////////////////////////////
static HD_RESULT set_proc_cfg(VIDEO_PROCESS *p_stream, HD_PATH_ID video_proc_ctrl, HD_DIM* p_max_dim)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
	HD_VIDEOPROC_CTRL video_ctrl_param = {0};
	UINT32  i;

	if (p_max_dim != NULL ) {
		video_cfg_param.pipe = HD_VIDEOPROC_PIPE_VSP;
		video_cfg_param.isp_id = p_stream->isp_id;
		video_cfg_param.ctrl_max.func = 0;
		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;
		video_cfg_param.in_max.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
		if (ret != HD_OK) {
			return HD_ERR_NG;
		}
	}
	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};
		video_path_param.in_func = 0;
		video_path_param.ddr_id = VDO_DDR_ID;
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}
	{
		VENDOR_VIDEOPROC_VSP_CFG vsp_cfg = {0};
		UINT32                   blk_size;

		// get fusion block
		blk_size = VDO_YUV_BUFSIZE(VSP_FUSION_SIZE_W, VSP_FUSION_SIZE_H, HD_VIDEO_PXLFMT_Y8);
		p_stream->fusion_blk = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, blk_size, VDO_DDR_ID); // Get block from mem pool
		if (p_stream->fusion_blk == HD_COMMON_MEM_VB_INVALID_BLK) {
			printf("get fusion_blk fail\r\n");
			return HD_ERR_NG;
		}
		p_stream->fusion_pa = hd_common_mem_blk2pa(p_stream->fusion_blk); // Get physical addr
		if (p_stream->fusion_pa == 0) {
			printf("blk2pa fail, fusion_blk = 0x%lx\r\n", p_stream->fusion_blk);
			goto rel_in_blk;
		}
		p_stream->fusion_va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, p_stream->fusion_pa, blk_size); // Get virtual addr
		if (p_stream->fusion_va == 0) {
			printf("Error: mmap fail !! fusion_pa 0x%lx, fusion_blk = 0x%lx\r\n", p_stream->fusion_pa, p_stream->fusion_blk);
			goto rel_in_blk;
		}
		// get dre work buffer
		p_stream->fusion_work_buf_blk = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, DRE_WORK_BUF_SIZE, VDO_DDR_ID); // Get block from mem pool
		if (p_stream->fusion_work_buf_blk == HD_COMMON_MEM_VB_INVALID_BLK) {
			printf("get fusion work bufer fail\r\n");
			return HD_ERR_NG;
		}
		p_stream->fusion_work_buf_pa = hd_common_mem_blk2pa(p_stream->fusion_work_buf_blk); // Get physical addr
		if (p_stream->fusion_work_buf_pa == 0) {
			printf("blk2pa fail, fusion_blk = 0x%lx\r\n", p_stream->fusion_work_buf_blk);
			goto rel_in_blk;
		}
		// init fusion table
		init_fusion_tbl(p_stream->fusion_va, VSP_FUSION_SIZE_W, VSP_FUSION_SIZE_H);
		// flush cache
		hd_common_mem_flush_cache((void *)p_stream->fusion_va, blk_size);

		vsp_cfg.blend_frm_num = VSP_BLEND_FRAME_NUM;
		vsp_cfg.dre_proc_mode = VENDOR_VIDEOPROC_DRE_PROC_FUSION_SIMP;
		vsp_cfg.dre_quality_level = 2;
		for (i = 0; i < VSP_BLEND_FRAME_NUM;i ++) {
			vsp_cfg.ovlp_width[i] = VSP_BLEND_WIDTH;
			vsp_cfg.dre_fusion_tbl_pa[i] = p_stream->fusion_pa;
			printf("fusion_pa 0x%lx\r\n", p_stream->fusion_pa);

			vsp_cfg.scale_out_size[i].h = VSP_SCALE_OUT_SIZE_H;
		}
		vsp_cfg.scale_out_size[0].w = VSP_SCALE_OUT_SIZE_W1;
		vsp_cfg.scale_out_size[1].w = VSP_SCALE_OUT_SIZE_W2;
		vsp_cfg.scale_out_size[2].w = VSP_SCALE_OUT_SIZE_W3;
		vsp_cfg.scale_out_size[3].w = VSP_SCALE_OUT_SIZE_W4;
		vsp_cfg.dre_work_buf_pa = p_stream->fusion_work_buf_pa;
		vsp_cfg.dre_work_buf_size = DRE_WORK_BUF_SIZE;
		vsp_cfg.vpe_out_size.w = VSP_VPE_OUT1_W;
		vsp_cfg.vpe_out_size.h = VSP_VPE_OUT1_H;
		vsp_cfg.en_ptz = TRUE;
		ret = vendor_videoproc_set(video_proc_ctrl, VENDOR_VIDEOPROC_PARAM_VSP_CFG, &vsp_cfg);
		if (ret != HD_OK) {
			printf("VENDOR_VIDEOPROC_PARAM_VSP_CFG failed!(%d)\r\n", ret);
		}
	}

	video_ctrl_param.func = 0;
	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);

	return ret;
rel_in_blk:
	if (p_stream->fusion_blk) {
		hd_common_mem_release_block(p_stream->fusion_blk);
	}
	if (p_stream->fusion_work_buf_blk) {
		hd_common_mem_release_block(p_stream->fusion_work_buf_blk);
	}
	return HD_ERR_NG;

}

static HD_RESULT set_proc_param(VIDEO_PROCESS *p_stream, UINT32 out_id)
{
	HD_RESULT ret = HD_OK;
	//dim MUST be zero for specified output region
	p_stream->proc_out[out_id].dim.w = 0;
	p_stream->proc_out[out_id].dim.h = 0;
	p_stream->proc_out[out_id].func = 0;
	p_stream->proc_out[out_id].pxlfmt = HD_VIDEO_PXLFMT_YUV420;
	p_stream->proc_out[out_id].dir = HD_VIDEO_DIR_NONE;
	p_stream->proc_out[out_id].frc = HD_VIDEO_FRC_RATIO(1,1);
	p_stream->proc_out[out_id].depth = 4; //set 1 to allow pull

	ret = hd_videoproc_set(p_stream->proc_path[out_id], HD_VIDEOPROC_PARAM_OUT, &p_stream->proc_out[out_id]);

	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};
		video_path_param.in_func = 0;
		video_path_param.ddr_id = VDO_DDR_ID;
		ret = hd_videoproc_set(p_stream->proc_path[out_id], HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}

	return ret;
}

static HD_RESULT init_module(void)
{
	HD_RESULT ret;
	if ((ret = hd_videoproc_init()) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT open_module(VIDEO_PROCESS *p_stream, HD_DIM* p_proc_max_dim)
{
    HD_RESULT ret;
	UINT32 i;

	ret = hd_videoproc_open(0, HD_VIDEOPROC_CTRL(p_stream->id), &p_stream->proc_ctrl); //open this for device control
	if (ret != HD_OK)
		return ret;

	// set videoproc config
	ret = set_proc_cfg(p_stream, p_stream->proc_ctrl, p_proc_max_dim);
	if (ret != HD_OK) {
		printf("set proc-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	if (g_effect) {
		if ((ret = set_effect_cfg(p_stream)) != HD_OK)
			return ret;
	}
	for (i=0; i < OUT_VSP_COUNT; i++) {
		if ((ret = hd_videoproc_open(HD_VIDEOPROC_IN(p_stream->id, 0), HD_VIDEOPROC_OUT(p_stream->id, i), &p_stream->proc_path[i])) != HD_OK)
			return ret;
	}
	return HD_OK;
}

static HD_RESULT close_module(VIDEO_PROCESS *p_stream)
{
	HD_RESULT ret;
	UINT32 i;

	for (i = 0; i < OUT_VSP_COUNT; i++) {
		if ((ret = hd_videoproc_close(p_stream->proc_path[i])) != HD_OK)
			return ret;
	}
	return HD_OK;
}

static HD_RESULT exit_module(void)
{
	HD_RESULT ret;
	if ((ret = hd_videoproc_uninit()) != HD_OK)
		return ret;
	if (g_effect) {
		vendor_vpe_uninit();
	}
	return HD_OK;
}

static BOOL check_test_pattern(void)
{
	FILE *f_in;
	char filepath[128];

	sprintf(filepath, TEST_PATTERN_PATH);

	if ((f_in = fopen(filepath, "rb")) == NULL) {
		printf("fail to open %s\n", filepath);
		printf("Test pattern is in SDK/code/hdal/samples/pattern/\n");
		return FALSE;
	}

	fclose(f_in);
	return TRUE;
}

static void trigger(VIDEO_PROCESS *p_stream, UINT32 loop_count, UINT32 sleep_sec)
{
	HD_RESULT ret = HD_OK;
	UINT32 i;
	HD_COMMON_MEM_DDR_ID ddr_id = VDO_DDR_ID;
	UINT32 in_blk_size = YUV_IN_BLK_SIZE;
	UINT32 in_y_size = VSP_IN_SIZE_W * VSP_IN_SIZE_H;
	UINT32 in_uv_size = in_y_size/2;
	char filepath_y[VSP_MAX_BLEND_FRAME_NUM][64] ={"/mnt/sd/vsp/img1_y.bin",
		                                           "/mnt/sd/vsp/img2_y.bin",
		                                           "/mnt/sd/vsp/img3_y.bin",
		                                           "/mnt/sd/vsp/img4_y.bin"
		                                           };
	char filepath_uv[VSP_MAX_BLEND_FRAME_NUM][64] ={"/mnt/sd/vsp/img1_uv.bin",
		                                            "/mnt/sd/vsp/img2_uv.bin",
		                                            "/mnt/sd/vsp/img3_uv.bin",
		                                            "/mnt/sd/vsp/img4_uv.bin"
		                                            };
	FILE *f_in_y, *f_in_uv;
	UINTPTR pa_yuv_in[VSP_MAX_BLEND_FRAME_NUM] = {0};
	UINTPTR va_yuv_in[VSP_MAX_BLEND_FRAME_NUM] = {0};
	HD_COMMON_MEM_VB_BLK in_blk[VSP_MAX_BLEND_FRAME_NUM];
	#if 0
	HD_COMMON_MEM_VB_BLK out_blk[OUT_VSP_COUNT] = {0};
	UINTPTR out_va[OUT_VSP_COUNT] = {0};
	UINTPTR out_pa[OUT_VSP_COUNT] = {0};
	UINT32 out_blk_size[OUT_VSP_COUNT] = {0};
	HD_VIDEO_FRAME out_video_frame[OUT_VSP_COUNT];
	#define PHY2VIRT_MAIN(pa, i) (out_va[i] + ((pa) - out_pa[i]))
	#endif
	UINT32 read_len;
	UINT32 trigger_count = 0;


	//--- Get memory for input image buffer---
	for (i = 0; i < VSP_BLEND_FRAME_NUM; i++) {
		in_blk[i] = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, in_blk_size, ddr_id); // Get block from mem pool
		if (in_blk[i] == HD_COMMON_MEM_VB_INVALID_BLK) {
			printf("get block[%d] fail (0x%x).. try again later.....\r\n", i, in_blk[i]);
			goto rel_in_blk;
		}
		pa_yuv_in[i] = hd_common_mem_blk2pa(in_blk[i]); // Get physical addr
		if (pa_yuv_in[i] == 0) {
			printf("blk2pa fail, in_blk[%d] = 0x%x\r\n", i, in_blk[i]);
			goto rel_in_blk;
		}
		va_yuv_in[i] = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa_yuv_in[i], in_blk_size); // Get virtual addr
		if (va_yuv_in[i] == 0) {
			printf("Error: mmap fail !! pa_yuv_main[%d], in_blk[%d] = 0x%x\r\n", i, i, in_blk[i]);
			goto rel_in_blk;
		}
		printf("\r\n@@IN[%d] BLK=0x%lX, PA=0x%lX, VA=0x%lX\r\n", i, (ULONG)in_blk[i], (ULONG)pa_yuv_in[i], (ULONG)va_yuv_in[i]);
		//--- Read YUV from file ---
		if ((f_in_y = fopen(filepath_y[i], "rb")) == NULL) {
			printf("open file (%s) fail !!....\r\nPlease copy test pattern to SD Card !!\r\n\r\n", filepath_y);
			return;
		}
		setvbuf(f_in_y, NULL, _IONBF, 0);
		if ((f_in_uv = fopen(filepath_uv[i], "rb")) == NULL) {
			printf("open file (%s) fail !!....\r\nPlease copy test pattern to SD Card !!\r\n\r\n", filepath_uv);
			return;
		}
		setvbuf(f_in_uv, NULL, _IONBF, 0);
		printf("read file\r\n");
		read_len = fread((void *)(va_yuv_in[i]), 1, in_y_size, f_in_y);
		if (read_len != in_y_size) {
			printf("reading y error\n");
			goto quit;
		}
		read_len = fread((void *)(va_yuv_in[i] + in_y_size), 1, in_uv_size, f_in_uv);
		if (read_len != in_uv_size) {
			printf("reading uv error\n");
			goto quit;
		}
		// close input image file
		fclose(f_in_y);
		fclose(f_in_uv);
		printf("flush cache size 0x%x\r\n", in_y_size + in_uv_size);
		//--- data is written by CPU, flush CPU cache to PHY memory ---
		hd_common_mem_flush_cache((void *)va_yuv_in[i], in_y_size + in_uv_size);
	}

	while ((!g_quit) && trigger_count < loop_count) {
		trigger_count++;
		printf("vsp_dev%d trigger_count %d\r\n", p_stream->id, trigger_count);
		//--- push_in ---
		for (i = 0; i < VSP_BLEND_FRAME_NUM; i++) {

			HD_VIDEO_FRAME video_frame = {0};

			//set input image buffer
			video_frame.sign        = MAKEFOURCC('V','F','R','M');
			//video_frame.p_next      = NULL;
			video_frame.ddr_id      = ddr_id;
			video_frame.pxlfmt      = HD_VIDEO_PXLFMT_YUV420;
			video_frame.dim.w       = VSP_IN_SIZE_W;
			video_frame.dim.h       = VSP_IN_SIZE_H;
			video_frame.count       = 0;
			video_frame.timestamp   = hd_gettime_us();
			video_frame.pw[0]       = VSP_IN_SIZE_W; // Y
			video_frame.ph[0]       = VSP_IN_SIZE_H; // UV
			video_frame.pw[1]       = VSP_IN_SIZE_W >> 1; // Y
			video_frame.ph[1]       = VSP_IN_SIZE_H >> 1; // UV
			video_frame.loff[0]     = ALIGN_CEIL_4(video_frame.pw[0]); // Y
			video_frame.loff[1]     = video_frame.loff[0]; // UV
			video_frame.phy_addr[0] = pa_yuv_in[i];                          // Y
			video_frame.phy_addr[1] = pa_yuv_in[i] + video_frame.loff[0]*video_frame.ph[0];  // UV
			video_frame.blk         = in_blk[i];
			video_frame.reserved[0] = MAKEFOURCC('V','S','P','E');
			video_frame.ddr_id |= (i << HD_VIDEO_MULTI_FRAME_SHIFT);

			printf("vsp_dev%d push in %d\r\n", p_stream->id, i);
			ret = hd_videoproc_push_in_buf(p_stream->proc_path[0], &video_frame, NULL, 0); // only support non-blocking mode
			if (ret != HD_OK) {
				printf("push_in(%d) error = %d!!\r\n", 0, ret);
				break;
			}
		}
		sleep(sleep_sec);
	}

rel_in_blk:
	//--- Release memory ---
	for (i = 0; i < VSP_BLEND_FRAME_NUM; i++) {
		ret = hd_common_mem_release_block(in_blk[i]);
		if (ret != HD_OK) {
			printf("_mem_release error !!\r\n\r\n");
		}
	}
quit:
	// mummap for input yuv buffer
	for (i=0; i< VSP_BLEND_FRAME_NUM; i++) {
		if (va_yuv_in[i] != 0) {
			ret = hd_common_mem_munmap((void *)va_yuv_in[i], in_blk_size);
			if (ret != HD_OK) {
				printf("mnumap error !!\r\n\r\n");
			}
		}
	}
}


void *vsp_thread(void *arg)
{
	VIDEO_PROCESS *p_stream = (VIDEO_PROCESS *)arg;

	printf("vsp_thread %d start, vsp_thread_id = 0x%x\r\n", p_stream->id, p_stream->vsp_thread_id);
	while (!g_quit) {
		trigger(p_stream, 1000, p_stream->sleep_sec);
	}
	return 0;
}

static void *pull_thread(void *arg)
{
	VIDEO_PROCESS *p_stream[VSP_DEV_CNT];

	HD_RESULT ret = HD_OK;
	HD_VIDEOPROC_POLL_LIST poll_list[VSP_DEV_CNT];
	UINT32         stream_i, i;
	HD_COMMON_MEM_VB_BLK out_blk[OUT_VSP_COUNT] = {0};
	UINTPTR out_va[OUT_VSP_COUNT] = {0};
	UINTPTR out_pa[OUT_VSP_COUNT] = {0};
	UINT32 out_blk_size[OUT_VSP_COUNT] = {0};
	HD_VIDEO_FRAME out_video_frame[OUT_VSP_COUNT];
	#define PHY2VIRT_MAIN(pa, i) (out_va[i] + ((pa) - out_pa[i]))


	p_stream[0] = (VIDEO_PROCESS *)arg;

	//------ wait flow_start ------
	//while (p_stream0->flow_start == 0) sleep(1);
	printf("\r\npull_thread....\r\n");
	for (i = 0; i < VSP_DEV_CNT; i++) {
		p_stream[i] = p_stream[0] + i;
		poll_list[i].path_id = p_stream[i]->proc_path[0];

	}
	//--------- pull data test ---------
	while (!g_quit) {
		ret = hd_videoproc_poll_list(poll_list, VSP_DEV_CNT, -1);// multi path poll_list , -1 = blocking mode
		if (ret != HD_OK) {
			sleep(1);
			continue;
		}
		for (stream_i = 0; stream_i < VSP_DEV_CNT; stream_i++) {
			if (TRUE == poll_list[stream_i].revent.event) {
				//wait output finish
				ret = hd_videoproc_pull_out_buf(p_stream[stream_i]->proc_path[0], &out_video_frame[0], -1); // -1 = blocking mode, 0 = non-blocking mode, >0 = blocking-timeout mode
				if (ret != HD_OK) {
					printf("pull_out[%d] error = %d!! blk=0x%X\r\n", i, ret, out_video_frame[0].blk);
					if (ret == HD_ERR_BAD_DATA) {
						printf("VPE[%d]-OUT[%d] blk=0x%X\r\n", i, 0, out_video_frame[0].blk);
					}
				}
				printf("VPE[%d]-OUT[%d] phy_addr_Y=0x%X, phy_addr_UV=0x%X\r\n", p_stream[stream_i]->id, 0, out_video_frame[0].phy_addr[0], out_video_frame[0].phy_addr[1]);
				out_blk[0] = out_video_frame[0].blk;
				//save output buffer
				for (i = 0; i < OUT_VSP_COUNT; i++) {
					out_pa[i] = hd_common_mem_blk2pa(out_blk[i]);
					out_blk_size[i] = (DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(p_stream[stream_i]->proc_out[i].bg.w, p_stream[stream_i]->proc_out[i].bg.h, HD_VIDEO_PXLFMT_YUV420));
					out_va[i] = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, out_pa[i], out_blk_size[i]); // Get virtual addr
					if (out_pa[i] == 0) {
						printf("blk2pa fail, blk = 0x%lx\r\n", out_blk[i]);
					}
					if (g_save_file) {
						char file_path_main[OUT_VSP_COUNT][128] = {0};
						FILE *f_out_main[OUT_VSP_COUNT];

						snprintf(file_path_main[i], 128, "/mnt/sd/vsp/VSP[%u]_out[%u]_%u_%u_%08x_%u.dat",
												p_stream[stream_i]->id, i, p_stream[stream_i]->proc_out[i].bg.w, p_stream[stream_i]->proc_out[i].bg.h,
												(unsigned int)p_stream[stream_i]->proc_out[i].pxlfmt, p_stream[stream_i]->shot_count);
						//printf("phy_addr_Y=0x%X, phy_addr_UV=0x%X\r\n", out_video_frame[i].phy_addr[0], out_video_frame[i].phy_addr[1]);
						printf("Save to (%s) ....\r\n", file_path_main[i]);
						if ((f_out_main[i] = fopen(file_path_main[i], "wb+")) == NULL) {
							printf("open file (%s) fail....\r\n\r\n", file_path_main[i]);
							goto unmap_buffer;
						}
						setvbuf(f_out_main[i], NULL, _IONBF, 0);
						//save Y plane
						{
							UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(out_video_frame[i].phy_addr[0], i);
							UINT32 len = out_video_frame[i].loff[0]*out_video_frame[i].ph[0];
							if (f_out_main[i]) fwrite(ptr, 1, len, f_out_main[i]);
							if (f_out_main[i]) fflush(f_out_main[i]);
						}
						//save UV plane
						{
							UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(out_video_frame[i].phy_addr[1], i);
							UINT32 len = out_video_frame[i].loff[1]*out_video_frame[i].ph[1];
							if (f_out_main[i]) fwrite(ptr, 1, len, f_out_main[i]);
							if (f_out_main[i]) fflush(f_out_main[i]);
						}
						fclose(f_out_main[i]);

					}
unmap_buffer:
					ret = hd_common_mem_munmap((void *)out_va[i], out_blk_size[i]);
					if (ret != HD_OK) {
						printf("mnumap error !!\r\n\r\n");
					}
				}
				ret = hd_videoproc_release_out_buf(p_stream[stream_i]->proc_path[0], &out_video_frame[0]);
				if (ret != HD_OK) {
					printf("release_out[%d] error !!\r\n\r\n", 0);
				}
			}
		}
	}

	return 0;
}


EXAMFUNC_ENTRY(hd_videoprocess_only_with_vsp, argc, argv)
{
	HD_RESULT ret;
	INT key;
	VIDEO_PROCESS stream[VSP_DEV_CNT] = {0};
	UINT32 i, j;
	UINT32 sleep_sec = 10;


	/*
	if (argc == 1) {
		printf("Usage: <sleep sec> .\r\n");
		printf("Help:\r\n");
		printf("  <main stream enc_type> : 0(H265), 1(H264), 2(MJPG)\r\n");
		return 0;
	}*/
	// query program options
	if (argc >= 2) {
		sleep_sec = atoi(argv[1]);
		printf("sleep_sec %d\r\n", sleep_sec);
	}
	if (argc >= 3) {
		g_save_file = atoi(argv[2]);
		printf("save_file %d\r\n", g_save_file);
	}
	// check TEST pattern exist
	if (check_test_pattern() == FALSE) {
		printf("test_pattern isn't exist\r\n");
		exit(0);
	}
	// init hdal
	ret = hd_common_init(0);
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
		goto exit;
	}

	// init memory
	ret = mem_init();
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
		goto exit;
	}

	// init all modules
	ret = init_module();
	if (ret != HD_OK) {
		printf("init fail=%d\n", ret);
		goto exit;
	}


	for (i = 0; i < VSP_DEV_CNT; i++) {
		VIDEO_PROCESS *p_stream = &stream[i];

		p_stream->id = i;
		if (g_effect == VPE_EFFECT_NONE) {
			p_stream->isp_id = HD_ISP_DONT_CARE;
		} else {
			p_stream->isp_id = i * 2;
			p_stream->ptz_isp_id = p_stream->isp_id + 1;	// ptz_isp_id must = isp_id+1
		}
		p_stream->sleep_sec = sleep_sec;
		// open video_process modules (1st)
		p_stream->proc_max_dim.w = VSP_IN_SIZE_W; //assign by user
		p_stream->proc_max_dim.h = VSP_IN_SIZE_H; //assign by user
		ret = open_module(p_stream, &p_stream->proc_max_dim);
		if (ret != HD_OK) {
			printf("open fail=%d\n", ret);
			goto exit;
		}

		// set videoproc parameter (1st)
		p_stream->proc_out[0].bg.w = OUT0_BG_W;
		p_stream->proc_out[0].bg.h = OUT0_BG_H;
		p_stream->proc_out[0].rect.x = 0;
		p_stream->proc_out[0].rect.y = 0;
		p_stream->proc_out[0].rect.w = OUT0_BG_W;
		p_stream->proc_out[0].rect.h = OUT0_BG_H;
		ret = set_proc_param(p_stream, 0);
		if (ret != HD_OK) {
			printf("set proc0-0 fail=%d\n", ret);
			goto exit;
		}
		for (j = 0; j < OUT_VSP_COUNT; j++) {
			// start video_process modules
			hd_videoproc_start(p_stream->proc_path[j]);
		}
	}
	ret = pthread_create(&stream[0].pull_thread_id, NULL, pull_thread, &stream[0]);
	if (ret < 0) {
		printf("create pull_thread failed\r\n");
		goto exit;
	}

	// query user key
	printf("Enter q to exit, s to trigger\n");
	while (1) {
		key = GETCHAR();
		if (key == 's') {
			trigger(&stream[0], 1, 1);
		}
		if (key == 'm') {
			for (i = 0; i < VSP_DEV_CNT; i++) {
				g_save_file = FALSE;
				ret = pthread_create(&stream[i].vsp_thread_id, NULL, vsp_thread, &stream[i]);
				if (ret < 0) {
					printf("create vsp_thread failed\r\n");
					goto exit;
				}
			}
		}
		if (key == 'q' || key == 0x3) {
			g_quit = 1; // stop thread
			break;
		}
		#if (DEBUG_MENU == 1)
		if (key == 'd') {
			// enter debug menu
			hd_debug_run_menu();
			printf("\r\nEnter q to exit, Enter d to debug\r\n");
		}
		#endif
	}
	for (i = 0; i < VSP_DEV_CNT; i++) {
		for (j = 0; j < OUT_VSP_COUNT; j++) {
			hd_videoproc_stop(stream[i].proc_path[j]); // it could force abort pull_out with blocking mode
		}
	}

	for (i = 0; i < VSP_DEV_CNT; i++) {
		if (stream[i].vsp_thread_id) {
			pthread_join(stream[i].vsp_thread_id, NULL);
		}
	}
exit:

	for (i = 0; i < VSP_DEV_CNT; i++) {
		// close video_process modules
		ret = close_module(&stream[i]);
		if (ret != HD_OK) {
			printf("close fail=%d\n", ret);
		}
	}
	// uninit all modules
	ret = exit_module();
	if (ret != HD_OK) {
		printf("exit fail=%d\n", ret);
	}

	// uninit memory
	ret = mem_exit(&stream[0]);
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
	}

	// uninit hdal
	ret = hd_common_uninit();
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
	}

	return 0;
}
