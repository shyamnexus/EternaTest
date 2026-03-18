/**
	@brief Sample code of video record with mode and pxlfmt.\n

	@file video_record_with_fastboot.c

	@author Boyan Huang

	@ingroup mhdal

	@note This file is modified from video_record_with_mode.c.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "hdal.h"
#include "hd_debug.h"
#include "mem_hotplug.h"
#if defined(__LINUX)
#include "vendor_isp.h"
#endif
#include "vendor_videocapture.h"
#include "vendor_videoprocess.h"

// platform dependent
#if defined(__LINUX)
#include <signal.h>
#include <pthread.h>            //for pthread API
#define MAIN(argc, argv)        int main(int argc, char** argv)
#define GETCHAR()               getchar()
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <kwrap/util.h>  //for sleep API
#define sleep(x)               vos_util_delay_ms(1000*(x))
#define msleep(x)               vos_util_delay_ms(x)
#define usleep(x)               vos_util_delay_us(x)
#include <kwrap/examsys.h>  //for MAIN(), GETCHAR() API
#define MAIN(argc, argv)		EXAMFUNC_ENTRY(hd_video_record_with_fastboot, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif
#define VOUT_DISP       DISABLE
#define DEBUG_MENU      1

#define AOUT_PLAYBACK   DISABLE

#define AI_DETECT       DISABLE

#define CHKPNT          printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
#define DBGH(x)         printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x)         printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)

///////////////////////////////////////////////////////////////////////////////
#define ISP_RELOAD_ENABLE 0

#define ISP_RELOAD_CFG 0
#define ISP_RELOAD_DTSI 1
#define ISP_RELOAD ISP_RELOAD_DTSI

//header
#define DBGINFO_BUFSIZE() (0x200)

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
#define VDO_YUV_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
//NVX: YUV compress
#define YUV_COMPRESS_RATIO 75
#define VDO_NVX_BUFSIZE(w, h, pxlfmt)    (VDO_YUV_BUFSIZE(w, h, pxlfmt) * YUV_COMPRESS_RATIO / 100)

//AI, refer to rtos's fast_ai.c
#define AI_PVD_MODEL_SIZE        793840
#define AI_PVD_BUF_SIZE          1893632
#define AI_LIMIT_FDET_MODEL_SIZE 249784
#define AI_LIMIT_FDET_BUF_SIZE   340608
#define AI_MEM_SIZE   (AI_PVD_MODEL_SIZE + AI_PVD_BUF_SIZE + AI_LIMIT_FDET_MODEL_SIZE + AI_LIMIT_FDET_BUF_SIZE)

//YUV QUEUE
//#define YUVQUE_NUM          3 //-- related with mem config
#define DEPTH_NUM           2
#define FAST_BOOT_DEPTH(buffing, depth)     ((buffing&0xFF)<<24 | depth)

///////////////////////////////////////////////////////////////////////////////

#define SEN_OUT_FMT     HD_VIDEO_PXLFMT_RAW12
#define CAP_OUT_FMT     HD_VIDEO_PXLFMT_RAW12
#define SHDR_CAP_OUT_FMT HD_VIDEO_PXLFMT_RAW12_SHDR2
#define CA_WIN_NUM_W    32
#define CA_WIN_NUM_H    32
#define LA_WIN_NUM_W    32
#define LA_WIN_NUM_H    32
#define VA_WIN_NUM_W    16
#define VA_WIN_NUM_H    16
#define YOUT_WIN_NUM_W  128
#define YOUT_WIN_NUM_H  128
#define ETH_8BIT_SEL    0 //0: 2bit out, 1:8 bit out
#define ETH_OUT_SEL     1 //0: full, 1: subsample 1/2

#define AUTO_RESOLUTION_SET 1 //1: use linux environment to decide RESOLUTION_SET

/**
 * 00: 2M(IMX290)
 * 01: 5M(OS05A)
 * 02: 2M (OS02K10)
 * 03: 2M (AR0237IR)
 * 04: (PATTERN_GEN)
 * 05: 2M (F37)
 * 06: 2M (F35)
 * 07: 4M (GC4653)
 * 08: 3M (Q03P)
 * 09: 2M(TC358840)
 * 10: 2M(TP2855)
 * 11: 4M(OS04C10)
 * 12: 4M(SC450AI)
 * 13: 4M(NT99436)
 * 14: 4M(OS04E10)
 */
static UINT32 RESOLUTION_SET = 0;
static char * SENSOR_NAME_TBL[] = {
	"sen_imx290",
	"sen_os05a10",
	"sen_os02k10",
	"sen_ar0237ir",
	"sen_off",
	"sen_f37",
	"sen_f35",
	"sen_gc4653",
	"sen_q03p",
	"sen_ad_tc358840",
	"sen_ad_tp2855",
	"sen_os04c10",
	"sen_sc450ai",
	"sen_nt99436",
	"sen_os04e10",
};
static UINT32 VDO_SIZE_W_TBL[] = {
	1920,
	2592,
	1920,
	1920,
	1920,
	1920,
	1920,
	2560,
	2304,
	1920,
	1920,
	2688,
	2688,
	2688,
	2048,
};
static UINT32 VDO_SIZE_H_TBL[] = {
	1080,
	1944,
	1080,
	1080,
	1080,
	1080,
	1080,
	1440,
	1296,
	1080,
	1080,
	1520,
	1520,
	1520,
	2048,
};

#define VIDEOCAP_ALG_FUNC HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB
#define VIDEOPROC_ALG_FUNC HD_VIDEOPROC_FUNC_WDR | HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA | HD_VIDEOPROC_FUNC_COLORNR | HD_VIDEOPROC_FUNC_DEFOG

#define SEN1_VCAP_ID sen1_vcap_id
#define SEN2_VCAP_ID sen2_vcap_id

static UINT32 sen1_vcap_id = 0;
static UINT32 sen2_vcap_id = 1;


//for single HDR sensor
#define HDR_SEN1_VCAP_ID     0
#define HDR_SEN1_SUB_VCAP_ID 1
#define SEN1_SHDR_VCAP_MAP ((1<<HDR_SEN1_VCAP_ID)|(1<<HDR_SEN1_SUB_VCAP_ID))

//for dual HDR sensor
#define HDR_SEN2_VCAP_ID     3
#define HDR_SEN2_SUB_VCAP_ID 4
#define SEN2_SHDR_VCAP_MAP ((1<<HDR_SEN2_VCAP_ID)|(1<<HDR_SEN2_SUB_VCAP_ID))

//for dual linear sensor
#define LINEAR_SEN2_VCAP_ID 1


static UINT32 g_shdr = 0; //fixed
static UINT32 g_capbind = 0;  //0:D2D, 1:direct, 2: one-buf, 0xff: no-bind
static UINT32 g_capfmt = 0; //0:RAW, 1:RAW-compress
static UINT32 g_prcbind = 0;  //0:D2D, 1:lowlatency, 2: one-buf, 0xff: no-bind
static UINT32 g_prcfmt = 0; //0:YUV, 1:YUV-compress
static UINT32 g_acapen = 0; //audio cap enable
static UINT32 g_mainen = 1;
static UINT32 g_snapshoten = 0;
static UINT32 g_suben = 0;
static UINT32 g_mainen2 = 0;
static UINT32 g_snapshoten2 = 0;
static UINT32 g_suben2 = 0;
static UINT32 sensor_en_map = 0; //0: single sensor mode,  1: dual mode sensor 1 on, 2: dual mode sensor 2 on, and 3: all
static UINT32 g_fps = 30;
static UINT32 g_prc_mask = 0; // ipp mask, draw a polygon on vproc
static UINT32 g_osg = 0; // render a novatek logo on codec and jpeg
static UINT32 g_rtos_ai = 0; // reserve a hdal memory for rtos-ai detect
static UINT32 g_prcout = 0; // videoprocess out path id of main stream
static UINT32 g_yuv_que = 0; // ipp yuv queue (related to mem init config)

//for dual-sensor crop size
static UINT32 g_crop_w = 1920;
static UINT32 g_crop_h = 1080;
///////////////////////////////////////////////////////////////////////////////
#define STAMP_WIDTH   416
#define STAMP_HEIGHT  128
///////////////////////////////////////////////////////////////////////////////
static UINT32 VDO_SIZE_W;
static UINT32 VDO_SIZE_H;

#define SUB_VDO_SIZE_W 640
#define SUB_VDO_SIZE_H 480

#define MAIN_BR			(2 * 1024 * 1024)
#define SUB_BR			(1 * 1024 * 1024)

VENDOR_VIDEOPROC_POLYGON_MASK prc_mask[] = {
	{1, 10, 0xFFFF0000, 0xFF, {{1376,450}, {1424,360}, {1568, 270}, {1712, 360}, {1760,  450}, {1760,  540}, {1712,630}, {1568,720}, {1424,630}, {1376,540}}, 0, 32, 32},
//	{1,  8, 0xFFFFFFFF, 0xFF, {{1280,  0}, {1600,  0}, {1920, 360}, {1920, 720}, {1600, 1080}, {1280, 1080}, {960 ,720}, {960 ,360}}                        , 0, 32, 32},
//	{1,  6, 0xFF0000FF, 0xFF, {{290 ,380}, {574 ,380}, {748 , 662}, {574 , 944}, {290 ,  944}, {116 ,  662}}                                                , 0, 32, 32},
//	{1,  4, 0xFF00FF00, 0xFF, {{100 ,  6}, {600 ,  6}, {600 ,  62}, {100 ,  62}}                                                                            , 0, 32, 32},
};

HD_OSG_MASK_ATTR prc_vp_mask[] = {
	{HD_OSG_MASK_TYPE_HOLLOW, 0x0000FF00, 255, {500, 0, 600, 0, 600, 120, 500, 120}, 5},
};

static UINT32 get_sen2_vcap_id(void)
{
	if(g_shdr) {
		return HDR_SEN2_VCAP_ID;
	} else {
		return LINEAR_SEN2_VCAP_ID;
	}
}
/*
static BOOL sensor1_enabled(void)
{
	if (sensor_en_map & 1)
		return TRUE;
	else
		return FALSE;
}
*/
static BOOL sensor1_enabled(void)
{
	if (sensor_en_map & 1 || sensor_en_map == 0)
		return TRUE;
	else
		return FALSE;
}

static BOOL sensor2_enabled(void)
{
	if (sensor_en_map & 2)
		return TRUE;
	else
		return FALSE;
}

static HD_RESULT mem_init(void)
{
	HD_RESULT              ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

	// config common pool (cap)
	mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
	if ((g_capbind == 0) || (g_capbind == 0xff)) {
		//normal
		mem_cfg.pool_info[0].blk_size = DBGINFO_BUFSIZE()
			+VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
			+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
		if (g_capfmt == HD_VIDEO_PXLFMT_NRX12)
			mem_cfg.pool_info[0].blk_size += VDO_NRX_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H);
		else
			mem_cfg.pool_info[0].blk_size += VDO_RAW_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, g_capfmt);
		mem_cfg.pool_info[0].blk_cnt = 2;
		mem_cfg.pool_info[0].ddr_id = DDR_ID0;
	} else if (g_capbind == 1) {
		//direct ... NOT require raw
		mem_cfg.pool_info[0].blk_size = DBGINFO_BUFSIZE()
										+VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
										+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
		mem_cfg.pool_info[0].blk_cnt = 2;
		mem_cfg.pool_info[0].ddr_id = DDR_ID0;
	} else if (g_capbind == 2) {
		//one-buf
		mem_cfg.pool_info[0].blk_size = DBGINFO_BUFSIZE()
										+VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
										+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
		mem_cfg.pool_info[0].blk_cnt = 1;
		mem_cfg.pool_info[0].ddr_id = DDR_ID0;
	} else {
		//not support
		mem_cfg.pool_info[0].blk_size = 0;
		mem_cfg.pool_info[0].blk_cnt = 0;
		mem_cfg.pool_info[0].ddr_id = DDR_ID0;
	}
	if (g_shdr == 1) {
		mem_cfg.pool_info[0].blk_cnt *= 2;
	}
	// config common pool (main)
	mem_cfg.pool_info[1].type = HD_COMMON_MEM_COMMON_POOL;

	if ((g_prcbind == 0) || (g_prcbind == 0xff)) {
		//normal
		if (sensor_en_map == 0) {
			mem_cfg.pool_info[1].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
			mem_cfg.pool_info[1].blk_cnt = 4; //venc release blk is slow, depend on its waiting list
			mem_cfg.pool_info[1].ddr_id = DDR_ID0;
		} else {
			mem_cfg.pool_info[1].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(g_crop_w, g_crop_h, HD_VIDEO_PXLFMT_YUV420);
			mem_cfg.pool_info[1].blk_cnt = 4; //venc release blk is slow, depend on its waiting list
			mem_cfg.pool_info[1].ddr_id = DDR_ID0;
		}
	} else if (g_prcbind == 1) {
		//low-latency
		if (sensor_en_map == 0) {
			mem_cfg.pool_info[1].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
			mem_cfg.pool_info[1].blk_cnt = 2;
			mem_cfg.pool_info[1].ddr_id = DDR_ID0;
		} else {
			mem_cfg.pool_info[1].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(g_crop_w, g_crop_h, HD_VIDEO_PXLFMT_YUV420);
			mem_cfg.pool_info[1].blk_cnt = 4;
			mem_cfg.pool_info[1].ddr_id = DDR_ID0;
		}
	} else if (g_prcbind == 2) {
		//one-buf
		if (sensor_en_map == 0) {
			mem_cfg.pool_info[1].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
			mem_cfg.pool_info[1].blk_cnt = 1;
			mem_cfg.pool_info[1].ddr_id = DDR_ID0;
		} else {
			mem_cfg.pool_info[1].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(g_crop_w, g_crop_h, HD_VIDEO_PXLFMT_YUV420);
			mem_cfg.pool_info[1].blk_cnt = 2;
			mem_cfg.pool_info[1].ddr_id = DDR_ID0;
		}
	} else {
		//not support
		mem_cfg.pool_info[1].blk_size = 0;
		mem_cfg.pool_info[1].blk_cnt = 0;
		mem_cfg.pool_info[1].ddr_id = DDR_ID0;
	}
	// config common pool (sub)
	if (g_suben) {
		mem_cfg.pool_info[2].type = HD_COMMON_MEM_COMMON_POOL;
		mem_cfg.pool_info[2].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(SUB_VDO_SIZE_W, SUB_VDO_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
		mem_cfg.pool_info[2].blk_cnt = 3;
		mem_cfg.pool_info[2].ddr_id = DDR_ID0;
	}

	// dual sensor
	if (sensor_en_map > 0) {
		mem_cfg.pool_info[0].blk_cnt *= 2;
		mem_cfg.pool_info[1].blk_cnt *= 2;
		mem_cfg.pool_info[2].blk_cnt *= 2;
	}

	int blk_idx = 3;

#if 0  //changed to internal malloc, no need
	// for kdrv h26x enc_var
	mem_cfg.pool_info[blk_idx].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[blk_idx].blk_size = 0x1000;
	mem_cfg.pool_info[blk_idx].blk_cnt = 1;
	mem_cfg.pool_info[blk_idx].ddr_id = DDR_ID0;
	blk_idx++;
#endif //changed to internal malloc, no need

#if (VOUT_DISP==ENABLE) /*always alloc,avoid change fastboot-hdal-mem.dtsi*/
	// config vout pool
	mem_cfg.pool_info[blk_idx].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[blk_idx].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[blk_idx].blk_cnt = 3;
	mem_cfg.pool_info[blk_idx].ddr_id = DDR_ID0;
	blk_idx++;
#endif

	if (g_yuv_que) {
		// config yuv queue
		mem_cfg.pool_info[blk_idx].type = HD_COMMON_MEM_COMMON_POOL;
		mem_cfg.pool_info[blk_idx].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
		mem_cfg.pool_info[blk_idx].blk_cnt = (g_yuv_que + DEPTH_NUM) * ((sensor_en_map == 3) ? 2 : 1);
		mem_cfg.pool_info[blk_idx].ddr_id = DDR_ID0;
		blk_idx++;
	}

#if (AOUT_PLAYBACK==ENABLE)
	/* user buffer for bs pushing in */
	mem_cfg.pool_info[blk_idx].type = HD_COMMON_MEM_USER_POOL_BEGIN;
	mem_cfg.pool_info[blk_idx].blk_size = 0x100000;
	mem_cfg.pool_info[blk_idx].blk_cnt = 1;
	mem_cfg.pool_info[blk_idx].ddr_id = DDR_ID0;
	blk_idx++;
#endif

	if (g_rtos_ai) {
		// for nn
		mem_cfg.pool_info[blk_idx].type = HD_COMMON_MEM_CNN_POOL;
		mem_cfg.pool_info[blk_idx].blk_size = AI_MEM_SIZE;
		mem_cfg.pool_info[blk_idx].blk_cnt = 1;
		mem_cfg.pool_info[blk_idx].ddr_id = DDR_ID0;
		blk_idx++;
	}

	if(g_osg){
		// for osg
		mem_cfg.pool_info[blk_idx].type = HD_COMMON_MEM_OSG_POOL;
		mem_cfg.pool_info[blk_idx].blk_size = ALIGN_CEIL(STAMP_WIDTH*STAMP_HEIGHT*2*2, 128);
		mem_cfg.pool_info[blk_idx].blk_cnt = 2;
		mem_cfg.pool_info[blk_idx].ddr_id = DDR_ID0;
		blk_idx++;
	}

	ret = hd_common_mem_init(&mem_cfg);
	return ret;
}

static unsigned int mem_alloc(UINT32 stamp_size, HD_COMMON_MEM_VB_BLK *stamp_blk, UINTPTR *stamp_pa)
{
	UINTPTR                 pa;
	HD_COMMON_MEM_VB_BLK    blk;

	if(!stamp_size){
		printf("stamp_size is unknown\n");
		return -1;
	}

	//get osd stamps' block
	blk = hd_common_mem_get_block(HD_COMMON_MEM_OSG_POOL, stamp_size, DDR_ID0);
	if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("get block fail\r\n");
		return -1;
	}

	if(stamp_blk)
		*stamp_blk = blk;

	//translate stamp block to physical address
	pa = hd_common_mem_blk2pa(blk);
	if (pa == 0) {
		printf("blk2pa fail, blk = 0x%x\r\n", blk);
		return -1;
	}

	if(stamp_pa)
		*stamp_pa = pa;

	return 0;
}

static HD_RESULT mem_exit(void)
{
	HD_RESULT ret = HD_OK;
	hd_common_mem_uninit();
	return ret;
}

///////////////////////////////////////////////////////////////////////////////

static HD_RESULT get_cap_caps(HD_PATH_ID video_cap_ctrl, HD_VIDEOCAP_SYSCAPS *p_video_cap_syscaps)
{
	HD_RESULT ret = HD_OK;
	hd_videocap_get(video_cap_ctrl, HD_VIDEOCAP_PARAM_SYSCAPS, p_video_cap_syscaps);
	return ret;
}

static HD_RESULT set_cap_cfg(HD_PATH_ID *p_video_cap_ctrl)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
	HD_PATH_ID video_cap_ctrl = 0;
	HD_VIDEOCAP_CTRL iq_ctl = {0};
	UINT32 ad_map;

	switch(RESOLUTION_SET) {
	case 0:
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_imx290");
		printf("Using nvt_sen_imx290\n");
		break;
	case 1:
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os05a10");
		printf("Using nvt_sen_os05a10\n");
		break;
	case 2:
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os02k10");
		printf("Using nvt_sen_os02k10\n");
		break;
	case 3:
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_ar0237ir");
		printf("Using nvt_sen_ar0237ir\n");
		break;
	case 4:
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, HD_VIDEOCAP_SEN_PAT_GEN);
		printf("Using sie pattern gen\n");
		break;
	case 5:
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_f37");
		printf("Using nvt_sen_f37\n");
		break;
	case 6:
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_f35");
		printf("Using nvt_sen_f35\n");
		break;
	case 7:
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_gc4653");
		printf("Using nvt_sen_gc4653\n");
		break;
	case 8:
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_q03p");
		printf("Using nvt_sen_q03p\n");
		break;
	case 9:
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_ad_tc358840");
		printf("Using nvt_sen_ad_tc358840\n");
		break;
	case 10:
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_ad_tp2855");
		printf("Using nvt_sen_ad_tp2855\n");
		break;
	case 11:
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os04c10");
		printf("Using nvt_sen_os04c10\n");
		break;
	case 12:
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_sc450ai");
		printf("Using nvt_sen_sc450ai_1\n");
		break;
	case 13:
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_nt99436");
		printf("Using nvt_sen_nt99436\n");
		break;
	case 14:
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os04e10");
		printf("Using nvt_sen_os04e10\n");
		break;

	default:
		printf("unhandled RESOLUTION_SET:%d\n", RESOLUTION_SET);
		return HD_ERR_NOT_SUPPORT;
	}

	if(RESOLUTION_SET == 3) {
		cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_P_RAW;
		printf("Parallel interface\n");
	}
	if(RESOLUTION_SET == 0 ||
		RESOLUTION_SET == 1 ||
		RESOLUTION_SET == 2 ||
		RESOLUTION_SET == 5 ||
		RESOLUTION_SET == 6 ||
		RESOLUTION_SET == 7 ||
		RESOLUTION_SET == 8 ||
		RESOLUTION_SET == 9 ||
		RESOLUTION_SET == 10 ||
		RESOLUTION_SET == 11 ||
		RESOLUTION_SET == 12 ||
		RESOLUTION_SET == 13 ||
		RESOLUTION_SET == 14)
	{
		cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
		printf("MIPI interface\n");
	}
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux =  0;  //use @0 in peri-dev.dtsi
	if (g_shdr == 1) {
		if (RESOLUTION_SET == 2 && sensor1_enabled() && sensor2_enabled()) {
			//OS02K10
		} else {
		}
		printf("Using g_shdr_mode\n");
	} else {
		switch(RESOLUTION_SET) {
		case 0:
			printf("Using imx290\n");
			break;
		case 1:
			printf("Using OS052A\n");
			break;
		case 2:
			printf("Using OS02K10\n");
			break;
		case 3:
			printf("unhandled RESOLUTION_SET:%d\n", RESOLUTION_SET);
			return HD_ERR_NOT_SUPPORT;
		case 4:
			printf("unhandled RESOLUTION_SET:%d\n", RESOLUTION_SET);
			return HD_ERR_NOT_SUPPORT;
		case 5:
			printf("Using F37\n");
			break;
		case 6:
			printf("Using F35\n");
			break;
		case 7:
			printf("Using GC4653\n");
			break;
		case 8:
			printf("Using Q03P\n");
			break;
		case 9:
			printf("Using TC358840\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.ccir_vd_hd_pin = TRUE;
			break;
		case 10:
			printf("Using TP2855\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.ccir_vd_hd_pin = TRUE;
			break;
		case 11:
			printf("Using OS04C10\n");
			break;
		case 12:
			printf("Using SC450AI_2\n");
			break;
		case 13:
			printf("Using NT99436\n");
			break;
		case 14:
			printf("Using OS04E10\n");
			break;

		default:
			printf("unhandled RESOLUTION_SET:%d\n", RESOLUTION_SET);
			return HD_ERR_NOT_SUPPORT;
		}
	}
	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(0, 0);
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = HD_VIDEOCAP_SEN_IGNORE;
	if (g_shdr == 1 && !sensor2_enabled()) {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
	} else {
		switch(RESOLUTION_SET) {
		case 0:
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
			break;
		case 1:
			printf("Using OS052A or shdr\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
			break;
		case 2:
			printf("Using OS02K10 or shdr\n");
			if (!sensor2_enabled()) {
				cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
				cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
			}
			break;
		case 3:
			printf("unhandled RESOLUTION_SET:%d\n", RESOLUTION_SET);
			return HD_ERR_NOT_SUPPORT;
		case 4:
			printf("unhandled RESOLUTION_SET:%d\n", RESOLUTION_SET);
			return HD_ERR_NOT_SUPPORT;
		case 5:
			printf("Using F37 or shdr\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = HD_VIDEOCAP_SEN_IGNORE;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = HD_VIDEOCAP_SEN_IGNORE;
			break;
		case 6:
			printf("Using F35 or shdr\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = HD_VIDEOCAP_SEN_IGNORE;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = HD_VIDEOCAP_SEN_IGNORE;
			break;
		case 7:
			printf("Using GC4653\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = HD_VIDEOCAP_SEN_IGNORE;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = HD_VIDEOCAP_SEN_IGNORE;
			break;
		case 8:
			printf("Using Q03P\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = HD_VIDEOCAP_SEN_IGNORE;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = HD_VIDEOCAP_SEN_IGNORE;
			break;
		case 9:
			printf("Using TC358840\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
			break;
		case 10:
			printf("Using TP2855\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
			break;
		case 11:
			printf("Using OS04C10\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = HD_VIDEOCAP_SEN_IGNORE;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = HD_VIDEOCAP_SEN_IGNORE;
			break;
		case 12:
			printf("Using SC450AI\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
			break;
		case 13:
			printf("Using NT99436\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = HD_VIDEOCAP_SEN_IGNORE;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = HD_VIDEOCAP_SEN_IGNORE;
			break;
		case 14:
			printf("Using OS04E10\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = HD_VIDEOCAP_SEN_IGNORE;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = HD_VIDEOCAP_SEN_IGNORE;
			break;
		default:
			printf("unhandled RESOLUTION_SET:%d\n", RESOLUTION_SET);
			return HD_ERR_NOT_SUPPORT;
		}
	}
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = HD_VIDEOCAP_SEN_IGNORE;
	ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(SEN1_VCAP_ID), &video_cap_ctrl); //open this for device control

	if (ret != HD_OK) {
		return ret;
	}

	if (g_shdr == 1) {
		cap_cfg.sen_cfg.shdr_map = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR1, SEN1_SHDR_VCAP_MAP);
	}
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);

	if (RESOLUTION_SET == 9 || RESOLUTION_SET == 10) {
		ad_map = VENDOR_VIDEOCAP_AD_MAP(0, 0, 1);
		vendor_videocap_set(video_cap_ctrl, VENDOR_VIDEOCAP_PARAM_AD_MAP, &ad_map);
		iq_ctl.func = 0;
	} else {
		iq_ctl.func = VIDEOCAP_ALG_FUNC;
		if (g_shdr == 1) {
			iq_ctl.func |= HD_VIDEOCAP_FUNC_SHDR;
		}
	}
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);
	*p_video_cap_ctrl = video_cap_ctrl;
	return ret;
}

static HD_RESULT set_cap2_cfg(HD_PATH_ID *p_video_cap_ctrl)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
	HD_PATH_ID video_cap_ctrl = 0;
	HD_VIDEOCAP_CTRL iq_ctl = {0};

	switch(RESOLUTION_SET) {
	case 1:
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os05a10");
		printf("Using nvt_sen_os05a10\n");
		break;
	case 2:
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os02k10");
		printf("Using nvt_sen_os02k10\n");
		break;
	default:
		printf("unhandled RESOLUTION_SET:%d\n", RESOLUTION_SET);
		return HD_ERR_NOT_SUPPORT;
	}
	cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux = 1;  //use @1 in peri-dev.dtsi
	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel =  HD_VIDEOCAP_SEN_CLANE_CSI(1, 0);
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = HD_VIDEOCAP_SEN_IGNORE;

	ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(SEN2_VCAP_ID), &video_cap_ctrl); //open this for device control
	if (ret != HD_OK) {
		return ret;
	}
	if (g_shdr) {
		cap_cfg.sen_cfg.shdr_map = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR2, SEN2_SHDR_VCAP_MAP);
	}
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
	iq_ctl.func = HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB;
	if (g_shdr) {
		iq_ctl.func |= HD_VIDEOCAP_FUNC_SHDR;
	}
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);

	*p_video_cap_ctrl = video_cap_ctrl;
	return ret;
}

static HD_RESULT set_cap_param(HD_PATH_ID video_cap_path, HD_DIM *p_dim, UINT32 path)
{
	HD_RESULT ret = HD_OK;
	{//select sensor mode, manually or automatically
		HD_VIDEOCAP_IN video_in_param = {0};

		switch(RESOLUTION_SET) {
		case 4:
			// pattern gen
			video_in_param.sen_mode = HD_VIDEOCAP_PATGEN_MODE(HD_VIDEOCAP_SEN_PAT_COLORBAR, 200);
			video_in_param.frc = HD_VIDEO_FRC_RATIO(g_fps,1);
			video_in_param.dim.w = p_dim->w;
			video_in_param.dim.h = p_dim->h;
			break;
		case 9:
			video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO; //auto select sensor mode by the parameter of HD_VIDEOCAP_PARAM_OUT
			video_in_param.frc = HD_VIDEO_FRC_RATIO(60,1);
			video_in_param.dim.w = p_dim->w;
			video_in_param.dim.h = p_dim->h;
			video_in_param.pxlfmt = HD_VIDEO_PXLFMT_YUV422;
			video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
			break;
		case 10:
			video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO; //auto select sensor mode by the parameter of HD_VIDEOCAP_PARAM_OUT
			video_in_param.frc = HD_VIDEO_FRC_RATIO(30,1);
			video_in_param.dim.w = p_dim->w;
			video_in_param.dim.h = p_dim->h;
			video_in_param.pxlfmt = HD_VIDEO_PXLFMT_YUV422;
			video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
			break;
		case 12:
			video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO; //auto select sensor mode by the parameter of HD_VIDEOCAP_PARAM_OUT
			video_in_param.frc = HD_VIDEO_FRC_RATIO(15,1);
			video_in_param.dim.w = p_dim->w;
			video_in_param.dim.h = p_dim->h;
			video_in_param.pxlfmt = HD_VIDEO_PXLFMT_RAW10;
			video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
			break;
		default:
			video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO; //auto select sensor mode by the parameter of HD_VIDEOCAP_PARAM_OUT
			video_in_param.frc = HD_VIDEO_FRC_RATIO(g_fps,1);
			video_in_param.dim.w = p_dim->w;
			video_in_param.dim.h = p_dim->h;
			video_in_param.pxlfmt = SEN_OUT_FMT;
			if ((path == 0) && (g_shdr == 1)) {
				video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_2;
			} else {
				video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
			}
			break;
		}
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN, &video_in_param);
		//printf("set_cap_param MODE=%d\r\n", ret);
		if (ret != HD_OK) {
			return ret;
		}
		if (RESOLUTION_SET == 9 || RESOLUTION_SET == 10) {
			VENDOR_VIDEOCAP_CCIR_INFO ccir_info;

			ccir_info.field_sel = VENDOR_VIDEOCAP_FIELD_DISABLE;
			ccir_info.fmt = VENDOR_VIDEOCAP_CCIR_FMT_SEL_CCIR601;
			ccir_info.interlace = 0;
			ccir_info.mux_data_index = 0;

			ret = vendor_videocap_set(video_cap_path, VENDOR_VIDEOCAP_PARAM_CCIR_INFO, &ccir_info);
			//printf("set VENDOR_VIDEOCAP_PARAM_CCIR_INFO\r\n");
			if (ret != HD_OK) {
				return ret;
			}
		}
	}
	if (sensor_en_map ==0) {
		#if 1 //no crop, full frame
		{
			HD_VIDEOCAP_CROP video_crop_param = {0};

			video_crop_param.mode = HD_CROP_OFF;
			ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);
			//printf("set_cap_param CROP NONE=%d\r\n", ret);
		}
		#else //HD_CROP_ON
		{
			HD_VIDEOCAP_CROP video_crop_param = {0};

			video_crop_param.mode = HD_CROP_ON;
			video_crop_param.win.rect.x = 0;
			video_crop_param.win.rect.y = 0;
			video_crop_param.win.rect.w = 1920;
			video_crop_param.win.rect.h= 1080;
			video_crop_param.align.w = 4;
			video_crop_param.align.h = 4;
			ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);
			//printf("set_cap_param CROP ON=%d\r\n", ret);
		}
		#endif
	} else {
		if (g_crop_w == VDO_SIZE_W && g_crop_h == VDO_SIZE_H) {
			HD_VIDEOCAP_CROP video_crop_param = {0};
			video_crop_param.mode = HD_CROP_OFF;
			ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);
		} else {
			HD_VIDEOCAP_CROP video_crop_param = {0};
			video_crop_param.mode = HD_CROP_ON;
			video_crop_param.win.rect.x = 0;
			video_crop_param.win.rect.y = 0;
			video_crop_param.win.rect.w = g_crop_w;
			video_crop_param.win.rect.h= g_crop_h;
			video_crop_param.align.w = 4;
			video_crop_param.align.h = 4;
			ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);

		}
	}
	{
		HD_VIDEOCAP_OUT video_out_param = {0};

		//without setting dim for no scaling, using original sensor out size
		// NOTE: only SHDR with path 1
		if ((path == 0) && (g_shdr == 1)) {
			if (sensor1_enabled() && sensor2_enabled()) {
				video_out_param.pxlfmt = HD_VIDEO_PXLFMT_NRX12_SHDR2;
			} else {
				video_out_param.pxlfmt = SHDR_CAP_OUT_FMT;
			}
		} else {
			if (RESOLUTION_SET == 9 || RESOLUTION_SET == 10)
				video_out_param.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
			else
		    	video_out_param.pxlfmt = g_capfmt;
		}
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		video_out_param.depth = 0;
		if (g_capbind == 0xff) //no-bind mode
			video_out_param.depth = 1;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT, &video_out_param);
		//printf("set_cap_param OUT=%d\r\n", ret);
	}
	{
		HD_VIDEOCAP_FUNC_CONFIG video_path_param = {0};

		video_path_param.out_func = 0;
		if (g_capbind == 1) //direct mode
			video_path_param.out_func = HD_VIDEOCAP_OUTFUNC_DIRECT;
		if (g_capbind == 2) //one-buf mode
			video_path_param.out_func = HD_VIDEOCAP_OUTFUNC_ONEBUF;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_FUNC_CONFIG, &video_path_param);
		//printf("set_cap_param PATH_CONFIG=0x%X\r\n", ret);
	}

	//nvt_sen_os02k10 set only
	UINT32 data_lane = 0;
	switch(RESOLUTION_SET) {
	case 2: //nvt_sen_os02k10
		data_lane = 4;
		break;
	case 12: //nvt_sen_sc450ai
		data_lane = 4;
		break;
	default:
		data_lane = 2;
		break;
	}
	ret = vendor_videocap_set(video_cap_path, VENDOR_VIDEOCAP_PARAM_DATA_LANE, &data_lane);
	if (ret != HD_OK) {
		printf("VENDOR_VIDEOCAP_PARAM_DATA_LANE failed!(%d)\r\n", ret);
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////

static HD_RESULT set_proc_cfg(HD_PATH_ID *p_video_proc_ctrl, HD_DIM* p_max_dim, HD_OUT_ID _out_id)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
	HD_VIDEOPROC_CTRL video_ctrl_param = {0};
	HD_VIDEOPROC_LL_CONFIG video_ll_param = {0};
	HD_PATH_ID video_proc_ctrl = 0;

	ret = hd_videoproc_open(0, _out_id, &video_proc_ctrl); //open this for device control
	if (ret != HD_OK)
		return ret;

	if (p_max_dim != NULL ) {
		if (RESOLUTION_SET == 9 || RESOLUTION_SET == 10) {
			video_cfg_param.pipe = HD_VIDEOPROC_PIPE_YUVALL;
			video_cfg_param.ctrl_max.func = 0;
		} else {
			video_cfg_param.pipe = HD_VIDEOPROC_PIPE_RAWALL;
			video_cfg_param.ctrl_max.func = VIDEOPROC_ALG_FUNC;
			if (g_shdr == 1) {
				video_cfg_param.ctrl_max.func |= HD_VIDEOPROC_FUNC_SHDR;
			}
		}
		if ((HD_CTRL_ID)_out_id == HD_VIDEOPROC_0_CTRL) {
			video_cfg_param.isp_id = 0;
		} else {
			video_cfg_param.isp_id = get_sen2_vcap_id();
		}

		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;
        // NOTE: only SHDR with path 1
		if (g_shdr == 1) {
			if (sensor1_enabled() && sensor2_enabled()) {
				video_cfg_param.in_max.pxlfmt = HD_VIDEO_PXLFMT_NRX12_SHDR2;
			} else {
				video_cfg_param.in_max.pxlfmt = SHDR_CAP_OUT_FMT;
			}
		} else {
		    video_cfg_param.in_max.pxlfmt = g_capfmt;
        }
		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
		if (ret != HD_OK) {
			return HD_ERR_NG;
		}
	}
	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		video_path_param.in_func = 0;
		if (g_capbind == 1)
			video_path_param.in_func |= HD_VIDEOPROC_INFUNC_DIRECT; //direct NOTE: enable direct
		if (g_capbind == 2)
			video_path_param.in_func |= HD_VIDEOPROC_INFUNC_ONEBUF;
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
		//printf("set_proc_param PATH_CONFIG=0x%X\r\n", ret);
	}

	video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_0_OUT_0;
	if (RESOLUTION_SET == 9 || RESOLUTION_SET == 10) {
		video_ctrl_param.func = 0;
	} else {
		video_ctrl_param.func = VIDEOPROC_ALG_FUNC;
		if (g_shdr == 1) {
			video_ctrl_param.func |= HD_VIDEOPROC_FUNC_SHDR;
		}
	}

	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);

	video_ll_param.delay_trig_lowlatency = 0;
	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_LL_CONFIG, &video_ll_param);

	*p_video_proc_ctrl = video_proc_ctrl;

	return ret;
}

static HD_RESULT set_proc_param(HD_PATH_ID video_proc_path, HD_DIM* p_dim, UINT32 depth)
{
	HD_RESULT ret = HD_OK;

	if (p_dim != NULL) { //if videoproc is already binding to dest module, not require to setting this!
		HD_VIDEOPROC_OUT video_out_param = {0};
		video_out_param.func = 0;
		video_out_param.dim.w = p_dim->w;
		video_out_param.dim.h = p_dim->h;
		video_out_param.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		video_out_param.frc = HD_VIDEO_FRC_RATIO(1,1);
		video_out_param.depth = depth; //set 1 to allow pull
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	} else {
		HD_VIDEOPROC_OUT video_out_param = {0};
		video_out_param.func = 0;
		video_out_param.dim.w = 0;
		video_out_param.dim.h = 0;
		video_out_param.pxlfmt = 0;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		video_out_param.frc = HD_VIDEO_FRC_RATIO(1,1);
		video_out_param.depth = depth; //set 1 to allow pull
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	}
	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		video_path_param.out_func = 0;
		if (g_prcbind == 1)
			video_path_param.out_func |= HD_VIDEOPROC_OUTFUNC_LOWLATENCY; //enable low-latency
		if (g_prcbind == 2)
			video_path_param.out_func |= HD_VIDEOPROC_OUTFUNC_ONEBUF;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}

	return ret;
}

static int set_mask_param(HD_PATH_ID mask_path, int idx)
{
	return vendor_videoproc_set(mask_path, VENDOR_VIDEOPROC_PARAM_VPROC_POLYGON_MASK, &(prc_mask[idx]));
}

static int set_vp_mask_param(HD_PATH_ID mask_path, int idx)
{
	return hd_videoproc_set(mask_path, HD_VIDEOPROC_PARAM_IN_MASK_ATTR, &(prc_vp_mask[idx]));
}
///////////////////////////////////////////////////////////////////////////////
#if (VOUT_DISP==ENABLE)
static HD_RESULT set_out_cfg(HD_PATH_ID *p_video_out_ctrl, UINT32 out_type,HD_VIDEOOUT_HDMI_ID hdmi_id)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOOUT_MODE videoout_mode = {0};
	HD_PATH_ID video_out_ctrl = 0;

	ret = hd_videoout_open(0, HD_VIDEOOUT_0_CTRL, &video_out_ctrl); //open this for device control
	if (ret != HD_OK) {
		return ret;
	}

	//printf("out_type=%d\r\n", out_type);

	videoout_mode.output_type = HD_COMMON_VIDEO_OUT_LCD;
	videoout_mode.input_dim = HD_VIDEOOUT_IN_AUTO;
	videoout_mode.output_mode.lcd = HD_VIDEOOUT_LCD_0;
	if (out_type != 1) {
		printf("520 only support LCD\r\n");
	}
	ret = hd_videoout_set(video_out_ctrl, HD_VIDEOOUT_PARAM_MODE, &videoout_mode);

	*p_video_out_ctrl=video_out_ctrl ;
	return ret;
}

static HD_RESULT get_out_caps(HD_PATH_ID video_out_ctrl,HD_VIDEOOUT_SYSCAPS *p_video_out_syscaps)
{
	HD_RESULT ret = HD_OK;
    HD_DEVCOUNT video_out_dev = {0};

	ret = hd_videoout_get(video_out_ctrl, HD_VIDEOOUT_PARAM_DEVCOUNT, &video_out_dev);
	if (ret != HD_OK) {
		return ret;
	}
	//printf("##devcount %d\r\n", video_out_dev.max_dev_count);

	ret = hd_videoout_get(video_out_ctrl, HD_VIDEOOUT_PARAM_SYSCAPS, p_video_out_syscaps);
	if (ret != HD_OK) {
		return ret;
	}
	return ret;
}

static HD_RESULT set_out_param(HD_PATH_ID video_out_path, HD_DIM *p_dim)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOOUT_IN video_out_param={0};

	video_out_param.dim.w = p_dim->w;
	video_out_param.dim.h = p_dim->h;
	video_out_param.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
	video_out_param.dir = HD_VIDEO_DIR_NONE;
	ret = hd_videoout_set(video_out_path, HD_VIDEOOUT_PARAM_IN, &video_out_param);
	if (ret != HD_OK) {
		return ret;
	}
	memset((void *)&video_out_param,0,sizeof(HD_VIDEOOUT_IN));
	ret = hd_videoout_get(video_out_path, HD_VIDEOOUT_PARAM_IN, &video_out_param);
	if (ret != HD_OK) {
		return ret;
	}
	printf("##video_out_param w:%d,h:%d %x %x\r\n", video_out_param.dim.w, video_out_param.dim.h, video_out_param.pxlfmt, video_out_param.dir);
	return ret;
}

///////////////////////////////////////////////////////////////////////////////
#endif

static HD_RESULT set_enc_cfg(HD_PATH_ID video_enc_path, HD_DIM *p_max_dim, UINT32 max_bitrate, HD_OUT_ID _out_id)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOENC_PATH_CONFIG video_path_config = {0};
	HD_VIDEOENC_FUNC_CONFIG video_func_config = {0};

	if (p_max_dim != NULL) {

		//--- HD_VIDEOENC_PARAM_PATH_CONFIG ---
		video_path_config.max_mem.codec_type = HD_CODEC_TYPE_H264;
		video_path_config.max_mem.max_dim.w  = p_max_dim->w;
		video_path_config.max_mem.max_dim.h  = p_max_dim->h;
		video_path_config.max_mem.bitrate    = max_bitrate;
		video_path_config.max_mem.enc_buf_ms = 3000;
		video_path_config.max_mem.svc_layer  = HD_SVC_4X;
		video_path_config.max_mem.ltr        = TRUE;
		video_path_config.max_mem.rotate     = FALSE;
		video_path_config.max_mem.source_output   = FALSE;
		if ((HD_CTRL_ID)_out_id == HD_VIDEOPROC_0_CTRL) {
			video_path_config.isp_id = 0;
		} else {
			video_path_config.isp_id = get_sen2_vcap_id();
		}

		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_PATH_CONFIG, &video_path_config);
		if (ret != HD_OK) {
			printf("set_enc_path_config = %d\r\n", ret);
			return HD_ERR_NG;
		}

		video_func_config.in_func = 0;
		if (g_prcbind == 1)
			video_func_config.in_func |= HD_VIDEOENC_INFUNC_LOWLATENCY; //enable low-latency
		if (g_prcbind == 2)
			video_func_config.in_func |= HD_VIDEOENC_INFUNC_ONEBUF;

		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_FUNC_CONFIG, &video_func_config);
		if (ret != HD_OK) {
			printf("set_enc_path_config = %d\r\n", ret);
			return HD_ERR_NG;
		}
	}

	return ret;
}

static HD_RESULT set_enc_param(HD_PATH_ID video_enc_path, HD_DIM *p_dim, UINT32 enc_type, UINT32 bitrate, UINT32 is_snapshot)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOENC_IN  video_in_param = {0};
	HD_VIDEOENC_OUT video_out_param = {0};
	HD_H26XENC_RATE_CONTROL rc_param = {0};

	if (p_dim != NULL) {

		//--- HD_VIDEOENC_PARAM_IN ---
		video_in_param.dir           = HD_VIDEO_DIR_NONE;
		video_in_param.pxl_fmt = HD_VIDEO_PXLFMT_YUV420;
		video_in_param.dim.w   = p_dim->w;
		video_in_param.dim.h   = p_dim->h;
		if (is_snapshot) {
			video_in_param.frc     = HD_VIDEO_FRC_RATIO(5,g_fps);
		} else {
			video_in_param.frc     = HD_VIDEO_FRC_RATIO(1,1);
		}
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_IN, &video_in_param);
		if (ret != HD_OK) {
			printf("set_enc_param_in = %d\r\n", ret);
			return ret;
		}

		printf("enc_type=%d\r\n", enc_type);

		if (enc_type == 0) {

			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_H265;
			video_out_param.h26x.profile       = HD_H265E_MAIN_PROFILE;
			if (video_in_param.dim.w > 1920 && video_in_param.dim.h > 1080) {
				video_out_param.h26x.level_idc	   = HD_H265E_LEVEL_5;
			} else {
				video_out_param.h26x.level_idc	   = HD_H265E_LEVEL_4_1;
			}
			video_out_param.h26x.gop_num       = 15;
			video_out_param.h26x.ltr_interval  = 0;
			video_out_param.h26x.ltr_pre_ref   = 0;
			video_out_param.h26x.gray_en       = 0;
			video_out_param.h26x.source_output = 0;
			video_out_param.h26x.svc_layer     = HD_SVC_DISABLE;
			video_out_param.h26x.entropy_mode  = HD_H265E_CABAC_CODING;
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &video_out_param);
			if (ret != HD_OK) {
				printf("set_enc_param_out = %d\r\n", ret);
				return ret;
			}

			//--- HD_VIDEOENC_PARAM_OUT_RATE_CONTROL ---
			rc_param.rc_mode             = HD_RC_MODE_CBR;
			rc_param.cbr.bitrate         = bitrate;
			rc_param.cbr.frame_rate_base = g_fps;
			rc_param.cbr.frame_rate_incr = 1;
			rc_param.cbr.init_i_qp       = 37;
			rc_param.cbr.min_i_qp        = 10;
			rc_param.cbr.max_i_qp        = 45;
			rc_param.cbr.init_p_qp       = 26;
			rc_param.cbr.min_p_qp        = 10;
			rc_param.cbr.max_p_qp        = 45;
			rc_param.cbr.static_time     = 4;
			rc_param.cbr.ip_weight       = 0;

			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL, &rc_param);
			if (ret != HD_OK) {
				printf("set_enc_rate_control = %d\r\n", ret);
				return ret;
			}
		} else if (enc_type == 1) {

			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_H264;
			video_out_param.h26x.profile       = HD_H264E_HIGH_PROFILE;
			if (video_in_param.dim.w > 1920 && video_in_param.dim.h > 1080) {
				video_out_param.h26x.level_idc	   = HD_H264E_LEVEL_5_1;
			} else {
				video_out_param.h26x.level_idc	   = HD_H264E_LEVEL_4_1;
			}
			video_out_param.h26x.gop_num       = 15;
			video_out_param.h26x.ltr_interval  = 0;
			video_out_param.h26x.ltr_pre_ref   = 0;
			video_out_param.h26x.gray_en       = 0;
			video_out_param.h26x.source_output = 0;
			video_out_param.h26x.svc_layer     = HD_SVC_DISABLE;
			video_out_param.h26x.entropy_mode  = HD_H264E_CABAC_CODING;
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &video_out_param);
			if (ret != HD_OK) {
				printf("set_enc_param_out = %d\r\n", ret);
				return ret;
			}

			//--- HD_VIDEOENC_PARAM_OUT_RATE_CONTROL ---
			rc_param.rc_mode             = HD_RC_MODE_CBR;
			rc_param.cbr.bitrate         = bitrate;
			rc_param.cbr.frame_rate_base = g_fps;
			rc_param.cbr.frame_rate_incr = 1;
			rc_param.cbr.init_i_qp       = 37;
			rc_param.cbr.min_i_qp        = 10;
			rc_param.cbr.max_i_qp        = 45;
			rc_param.cbr.init_p_qp       = 26;
			rc_param.cbr.min_p_qp        = 10;
			rc_param.cbr.max_p_qp        = 45;
			rc_param.cbr.static_time     = 4;
			rc_param.cbr.ip_weight       = 0;

			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL, &rc_param);
			if (ret != HD_OK) {
				printf("set_enc_rate_control = %d\r\n", ret);
				return ret;
			}

		} else if (enc_type == 2) {

			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_JPEG;
			video_out_param.jpeg.retstart_interval = 0;
			video_out_param.jpeg.image_quality = 50;
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &video_out_param);
			if (ret != HD_OK) {
				printf("set_enc_param_out = %d\r\n", ret);
				return ret;
			}

		} else {

			printf("not support enc_type\r\n");
			return HD_ERR_NG;
		}
	}

	return ret;
}

static int set_enc_stamp_param(HD_PATH_ID stamp_path, UINTPTR stamp_pa, UINT32 stamp_size)
{
	HD_OSG_STAMP_BUF  buf;
	HD_OSG_STAMP_IMG  img;
	HD_OSG_STAMP_ATTR attr;

	if(!stamp_pa){
		printf("stamp buffer is not allocated\n");
		return -1;
	}
	//ping pong buffer address should at least be 64 aligned
	//best to be 128 aligned
	if(stamp_pa & 0x3f){
		printf("stamp ping pong buffer(0x%x) is not 64 aligned\n", (unsigned long)stamp_pa);
		return -1;
	}


	memset(&buf, 0, sizeof(HD_OSG_STAMP_BUF));

	buf.type      = HD_OSG_BUF_TYPE_PING_PONG;
	buf.p_addr    = stamp_pa;
	buf.size      = stamp_size;

	if(hd_videoenc_set(stamp_path, HD_VIDEOENC_PARAM_IN_STAMP_BUF, &buf) != HD_OK){
		printf("fail to set stamp buffer\n");
		return -1;
	}

	memset(&img, 0, sizeof(HD_OSG_STAMP_IMG));

	img.fmt        = HD_VIDEO_PXLFMT_ARGB4444;
	img.dim.w      = STAMP_WIDTH;
	img.dim.h      = STAMP_HEIGHT;
	img.p_addr     = stamp_pa; //novatek logo is already in ping pong buffer

	if(hd_videoenc_set(stamp_path, HD_VIDEOENC_PARAM_IN_STAMP_IMG, &img) != HD_OK){
		printf("fail to set stamp image\n");
		return -1;
	}

	memset(&attr, 0, sizeof(HD_OSG_STAMP_ATTR));

	attr.position.x = 0;
	attr.position.y = 240;
	attr.alpha      = 255;
	attr.layer      = 0;
	attr.region     = 0;

	return hd_videoenc_set(stamp_path, HD_VIDEOENC_PARAM_IN_STAMP_ATTR, &attr);
}

static HD_RESULT set_acap_cfg(HD_PATH_ID *p_audio_cap_ctrl, HD_AUDIO_SR sample_rate)
{
	HD_RESULT ret = HD_OK;
	HD_AUDIOCAP_DEV_CONFIG audio_cfg_param = {0};
	HD_AUDIOCAP_DRV_CONFIG audio_driver_cfg_param = {0};
	HD_AUDIOCAP_VOLUME audio_vol_param = {0};
	HD_PATH_ID audio_cap_ctrl = 0;

	ret = hd_audiocap_open(0, HD_AUDIOCAP_0_CTRL, &audio_cap_ctrl); //open this for device control
	if (ret != HD_OK) {
		return ret;
	}

	/*set audio capture maximum parameters*/
	audio_cfg_param.in_max.sample_rate = sample_rate;
	audio_cfg_param.in_max.sample_bit = HD_AUDIO_BIT_WIDTH_16;
	audio_cfg_param.in_max.mode = HD_AUDIO_SOUND_MODE_STEREO;
	audio_cfg_param.in_max.frame_sample = 1024;
	audio_cfg_param.frame_num_max = 50;
	audio_cfg_param.out_max.sample_rate = 0;
	ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_DEV_CONFIG, &audio_cfg_param);
	if (ret != HD_OK) {
		return ret;
	}

	/*set audio capture driver parameters*/
	audio_driver_cfg_param.mono = HD_AUDIO_MONO_RIGHT;
	ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_DRV_CONFIG, &audio_driver_cfg_param);
	if (ret != HD_OK) {
		return ret;
	}

	audio_vol_param.volume = 100;
	ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_VOLUME, &audio_vol_param);
	if (ret != HD_OK) {
		return ret;
	}

	*p_audio_cap_ctrl = audio_cap_ctrl;

	return ret;
}

static HD_RESULT set_acap_param(HD_PATH_ID audio_cap_path, HD_AUDIO_SR sample_rate)
{
	HD_RESULT ret = HD_OK;
	HD_AUDIOCAP_IN audio_cap_in_param = {0};
	HD_AUDIOCAP_OUT audio_cap_out_param = {0};

	// set hd_audiocapture input parameters
	audio_cap_in_param.sample_rate = sample_rate;
	audio_cap_in_param.sample_bit = HD_AUDIO_BIT_WIDTH_16;
	audio_cap_in_param.mode = HD_AUDIO_SOUND_MODE_MONO;
	audio_cap_in_param.frame_sample = 1024;
	ret = hd_audiocap_set(audio_cap_path, HD_AUDIOCAP_PARAM_IN, &audio_cap_in_param);
	if (ret != HD_OK) {
		return ret;
	}

	// set hd_audiocapture output parameters
	audio_cap_out_param.sample_rate = 0;
	ret = hd_audiocap_set(audio_cap_path, HD_AUDIOCAP_PARAM_OUT, &audio_cap_out_param);

	return ret;
}

#if (AOUT_PLAYBACK==ENABLE)
static HD_RESULT set_aout_cfg(HD_PATH_ID *p_audio_out_ctrl, HD_AUDIO_SR sample_rate)
{
	HD_RESULT ret = HD_OK;
	HD_AUDIOOUT_DEV_CONFIG audio_cfg_param = {0};
	HD_AUDIOOUT_DRV_CONFIG audio_driver_cfg_param = {0};
	HD_PATH_ID audio_out_ctrl = 0;

	ret = hd_audioout_open(0, HD_AUDIOOUT_0_CTRL, &audio_out_ctrl); //open this for device control
	if (ret != HD_OK) {
		return ret;
	}

	/* set audio out maximum parameters */
	audio_cfg_param.out_max.sample_rate = sample_rate;
	audio_cfg_param.out_max.sample_bit = HD_AUDIO_BIT_WIDTH_16;
	audio_cfg_param.out_max.mode = HD_AUDIO_SOUND_MODE_STEREO;
	audio_cfg_param.frame_sample_max = 1024;
	audio_cfg_param.frame_num_max = 10;
	audio_cfg_param.in_max.sample_rate = 0;
	ret = hd_audioout_set(audio_out_ctrl, HD_AUDIOOUT_PARAM_DEV_CONFIG, &audio_cfg_param);
	if (ret != HD_OK) {
		return ret;
	}

	/* set audio out driver parameters */
	audio_driver_cfg_param.mono = HD_AUDIO_MONO_LEFT;
	audio_driver_cfg_param.output = HD_AUDIOOUT_OUTPUT_LINE;
	ret = hd_audioout_set(audio_out_ctrl, HD_AUDIOOUT_PARAM_DRV_CONFIG, &audio_driver_cfg_param);

	*p_audio_out_ctrl = audio_out_ctrl;

	return ret;
}
#endif

#if (AOUT_PLAYBACK==ENABLE)
static HD_RESULT set_aout_param(HD_PATH_ID audio_out_ctrl, HD_PATH_ID audio_out_path, HD_AUDIO_SR sample_rate)
{
	HD_RESULT ret;
	HD_AUDIOOUT_OUT audio_out_out_param = {0};
	HD_AUDIOOUT_VOLUME audio_out_vol = {0};
	HD_AUDIOOUT_IN audio_out_in_param = {0};

	// set hd_audioout output parameters
	audio_out_out_param.sample_rate = sample_rate;
	audio_out_out_param.sample_bit = HD_AUDIO_BIT_WIDTH_16;
	audio_out_out_param.mode = HD_AUDIO_SOUND_MODE_STEREO;
	ret = hd_audioout_set(audio_out_path, HD_AUDIOOUT_PARAM_OUT, &audio_out_out_param);
	if (ret != HD_OK) {
		return ret;
	}

	// set hd_audioout volume
	audio_out_vol.volume = 50;
	ret = hd_audioout_set(audio_out_ctrl, HD_AUDIOOUT_PARAM_VOLUME, &audio_out_vol);
	if (ret != HD_OK) {
		return ret;
	}


	// set hd_audioout input parameters
	audio_out_in_param.sample_rate = 0;
	ret = hd_audioout_set(audio_out_path, HD_AUDIOOUT_PARAM_IN, &audio_out_in_param);

	return ret;
}
#endif

///////////////////////////////////////////////////////////////////////////////

typedef struct _VIDEO_RECORD {

	// (1)
	HD_VIDEOCAP_SYSCAPS cap_syscaps;
	HD_PATH_ID cap_ctrl;
	HD_PATH_ID cap_path;

	HD_DIM  cap_dim;

	// only used when (g_capbind = 0xff)  //no-bind mode
	pthread_t  cap_thread_id;
	UINT32	cap_enter;
	UINT32	cap_exit;
	UINT32	cap_count;
	UINT32 	cap_loop;

	// (2)
	HD_DIM  proc_max_dim;
	HD_VIDEOPROC_SYSCAPS proc_syscaps;
	HD_PATH_ID proc_ctrl;
	HD_PATH_ID proc_path;

	HD_PATH_ID mask_path[8]; // ipp mask
	HD_PATH_ID vp_mask_path[8]; // ipp vp mask

	// only used when (g_prcbind = 0xff)  //no-bind mode
	pthread_t  prc_thread_id;
	UINT32	prc_enter;
	UINT32	prc_exit;
	UINT32 	prc_count;
	UINT32 	prc_loop;

	// (3)
	HD_DIM  enc_max_dim;
	HD_DIM  enc_dim;
	HD_VIDEOENC_SYSCAPS enc_syscaps;
	HD_PATH_ID enc_path;

	// user pull
	pthread_t  enc_thread_id;
	UINT32     enc_enter;
	UINT32     enc_exit;

	// (4)
	pthread_t  acap_thread_id;
	HD_PATH_ID  acap_ctrl;
	HD_PATH_ID  acap_path;
	HD_AUDIO_SR acap_sr_max;
	HD_AUDIO_SR acap_sr;
	UINT32      acap_enter;
	UINT32      acap_exit;
#if (VOUT_DISP==ENABLE)
    // (5) display
	HD_DIM  out_max_dim;
	HD_DIM  out_dim;
	HD_VIDEOOUT_SYSCAPS out_syscaps;
	HD_PATH_ID out_ctrl;
	HD_PATH_ID out_path;
	HD_PATH_ID proc_path2;
#endif

	// (6)
	pthread_t   aout_thread_id;
	HD_PATH_ID  aout_ctrl;
	HD_PATH_ID  aout_path;
	UINT32      aout_enter;
	UINT32      aout_exit;

	// (7) osg
	HD_PATH_ID enc_stamp_path;
	HD_COMMON_MEM_VB_BLK stamp_blk;
	UINTPTR stamp_pa;
	UINT32 stamp_size;

} VIDEO_RECORD;

static HD_RESULT init_module(void)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_init()) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_init()) != HD_OK)
		return ret;
    if ((ret = hd_videoenc_init()) != HD_OK)
		return ret;
	if (g_acapen) {
		if ((ret = hd_audiocap_init()) != HD_OK)
			return ret;
#if (AOUT_PLAYBACK==ENABLE)
		if ((ret = hd_audioout_init()) != HD_OK)
			return ret;
#endif
	}
#if (VOUT_DISP==ENABLE)
	if ((ret = hd_videoout_init()) != HD_OK)
		return ret;
#endif
	return HD_OK;
}

static HD_RESULT open_module(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;

	// set videocap config
	ret = set_cap_cfg(&p_stream->cap_ctrl);
	if (ret != HD_OK) {
		printf("set cap-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, HD_VIDEOPROC_0_CTRL);
	if (ret != HD_OK) {
		printf("set proc-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}

#if (VOUT_DISP==ENABLE)
	// set videoout config 1:only support lcd
	ret = set_out_cfg(&p_stream->out_ctrl, 1,0);
	if (ret != HD_OK) {
		printf("set out-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
#endif
	if ((ret = hd_videocap_open(HD_VIDEOCAP_IN(SEN1_VCAP_ID, 0), HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), &p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_OUT(0, g_prcout), &p_stream->proc_path)) != HD_OK)
		return ret;
	if (g_prc_mask) {
		int i;
		int n_prc_mask = sizeof(prc_mask)/sizeof(prc_mask[0]);
		int n_mask_path = sizeof(p_stream->mask_path)/sizeof(p_stream->mask_path[0]);
		if (n_mask_path < n_prc_mask) {
			printf("mask_path array size is too small, require: %d\n", n_prc_mask);
			return HD_ERR_NG;
		}
		for(i = 0 ; i < n_prc_mask ; i++) {
			if((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_MASK(i*2), &(p_stream->mask_path[i]))) != HD_OK) {
				return ret;
			}
		}

		int n_vp_mask = sizeof(prc_vp_mask)/sizeof(prc_vp_mask[0]);
		int n_vp_mask_path = sizeof(p_stream->vp_mask_path)/sizeof(p_stream->vp_mask_path[0]);
		if (n_vp_mask_path < n_vp_mask) {
			printf("vp_mask_path array size is too small, require: %d\n", n_vp_mask_path);
			return HD_ERR_NG;
		}

		for(i = 0 ; i < n_vp_mask ; i++) {
			if((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_MASK(n_prc_mask*2 + i*2), &(p_stream->vp_mask_path[i]))) != HD_OK) {
				return ret;
			}
		}
	}
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_VIDEOENC_0_OUT_0, &p_stream->enc_path)) != HD_OK)
		return ret;
#if (VOUT_DISP==ENABLE)
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_2, &p_stream->proc_path2)) != HD_OK)
		return ret;
	if ((ret = hd_videoout_open(HD_VIDEOOUT_0_IN_0, HD_VIDEOOUT_0_OUT_0, &p_stream->out_path)) != HD_OK)
		return ret;
#endif

	if(g_osg){
		if((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_STAMP_0, &p_stream->enc_stamp_path)) != HD_OK)
			return ret;
	}

	return HD_OK;
}

static HD_RESULT open_audio_module(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;

	// set audiocap config
	ret = set_acap_cfg(&p_stream->acap_ctrl, p_stream->acap_sr_max);
	if (ret != HD_OK) {
		printf("set acap-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	if ((ret = hd_audiocap_open(HD_AUDIOCAP_0_IN_0, HD_AUDIOCAP_0_OUT_0, &p_stream->acap_path)) != HD_OK)
		return ret;

#if (AOUT_PLAYBACK==ENABLE)
	// set audioout config
	ret = set_aout_cfg(&p_stream->aout_ctrl, p_stream->acap_sr_max);
	if (ret != HD_OK) {
		printf("set aout-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	if ((ret = hd_audioout_open(HD_AUDIOOUT_0_IN_0, HD_AUDIOOUT_0_OUT_0, &p_stream->aout_path)) != HD_OK)
		return ret;
#endif

	return HD_OK;
}

static HD_RESULT open_module_2(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_1, HD_VIDEOENC_0_OUT_1,  &p_stream->enc_path)) != HD_OK)
		return ret;

	if(g_osg){
		if((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_1, HD_STAMP_0, &p_stream->enc_stamp_path)) != HD_OK)
			return ret;
	}

	return HD_OK;
}

static HD_RESULT open_module_3(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_1, &p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_2, HD_VIDEOENC_0_OUT_2,  &p_stream->enc_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT open_module_4(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;

	// set videocap config
	ret = set_cap2_cfg(&p_stream->cap_ctrl);
	if (ret != HD_OK) {
		printf("set cap-cfg2 fail=%d\n", ret);
		return HD_ERR_NG;
	}

	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, HD_VIDEOPROC_1_CTRL);
	if (ret != HD_OK) {
		printf("set proc-cfg2 fail=%d\n", ret);
		return HD_ERR_NG;
	}

	if((ret = hd_videocap_open(HD_VIDEOCAP_IN(SEN2_VCAP_ID, 0), HD_VIDEOCAP_OUT(SEN2_VCAP_ID, 0), &p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_1_IN_0, HD_VIDEOPROC_1_OUT_0, &p_stream->proc_path)) != HD_OK)
		return ret;

	return HD_OK;
}


static HD_RESULT open_module_5(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;

	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_3, HD_VIDEOENC_0_OUT_3,  &p_stream->enc_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT open_module_6(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_4, HD_VIDEOENC_0_OUT_4,  &p_stream->enc_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT open_module_7(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;

	if ((ret = hd_videoproc_open(HD_VIDEOPROC_1_IN_0, HD_VIDEOPROC_1_OUT_1, &p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_5, HD_VIDEOENC_0_OUT_5,  &p_stream->enc_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT close_module(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_close(p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_close(p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_close(p_stream->enc_path)) != HD_OK)
		return ret;
#if (VOUT_DISP==ENABLE)
	if ((ret = hd_videoproc_close(p_stream->proc_path2)) != HD_OK)
		return ret;
	if ((ret = hd_videoout_close(p_stream->out_path)) != HD_OK)
		return ret;
#endif

	if(g_osg){
		if((ret = hd_videoenc_close(p_stream->enc_stamp_path)) != HD_OK)
			return ret;
	}

	return HD_OK;
}

static HD_RESULT close_audio_module(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;

	if ((ret = hd_audiocap_close(p_stream->acap_path)) != HD_OK)
		return ret;

#if (AOUT_PLAYBACK==ENABLE)
	if ((ret = hd_audioout_close(p_stream->aout_path)) != HD_OK)
		return ret;
#endif

	return HD_OK;
}

static HD_RESULT close_module_2(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_videoenc_close(p_stream->enc_path)) != HD_OK)
		return ret;
	if(g_osg){
		if((ret = hd_videoenc_close(p_stream->enc_stamp_path)) != HD_OK)
			return ret;
	}
	return HD_OK;
}

static HD_RESULT close_module_3(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_videoproc_close(p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_close(p_stream->enc_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT close_module_4(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;

	if ((ret = hd_videocap_close(p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_close(p_stream->proc_path)) != HD_OK)
		return ret;

	return HD_OK;
}


static HD_RESULT close_module_5(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_videoenc_close(p_stream->enc_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT close_module_6(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_videoenc_close(p_stream->enc_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT close_module_7(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;

	if ((ret = hd_videoproc_close(p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_close(p_stream->enc_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT exit_module(void)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_uninit()) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_uninit()) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_uninit()) != HD_OK)
		return ret;
	if (g_acapen) {
		if ((ret = hd_audiocap_uninit()) != HD_OK)
			return ret;
#if (AOUT_PLAYBACK==ENABLE)
		if ((ret = hd_audioout_uninit()) != HD_OK)
			return ret;
#endif
	}
#if (VOUT_DISP==ENABLE)
	if ((ret = hd_videoout_uninit()) != HD_OK)
		return ret;
#endif

	return HD_OK;
}

// only used when (g_capbind = 0xff)  //no-bind mode
static void *capture_thread(void *arg)
{
	VIDEO_RECORD* p_stream0 = (VIDEO_RECORD *)arg;
	HD_RESULT ret = HD_OK;

	HD_VIDEO_FRAME video_frame = {0};

	p_stream0->cap_exit = 0;
	p_stream0->cap_loop = 0;
	p_stream0->cap_count = 0;
	//------ wait flow_start ------
	while (p_stream0->cap_enter == 0) usleep(100);

	//--------- pull data test ---------
	while (p_stream0->cap_exit == 0) {

		//printf("cap_pull ....\r\n");
		ret = hd_videocap_pull_out_buf(p_stream0->cap_path, &video_frame, -1); // -1 = blocking mode
		if (ret != HD_OK) {
			if (ret != HD_ERR_UNDERRUN)
			printf("cap_pull error=%d !!\r\n\r\n", ret);
    			goto skip;
		}
		//printf("cap frame.count = %llu\r\n", video_frame.count);

		//printf("proc_push ....\r\n");
		ret = hd_videoproc_push_in_buf(p_stream0->proc_path, &video_frame, NULL, 0); // only support non-blocking mode now
		if (ret != HD_OK) {
			printf("proc_push error=%d !!\r\n\r\n", ret);
    			goto skip;
		}

		//printf("cap_release ....\r\n");
		ret = hd_videocap_release_out_buf(p_stream0->cap_path, &video_frame);
		if (ret != HD_OK) {
			printf("cap_release error=%d !!\r\n\r\n", ret);
    			goto skip;
		}

		p_stream0->cap_count ++;
		//printf("capture count = %d\r\n", p_stream0->cap_count);
skip:
		p_stream0->cap_loop++;
		usleep(100); //sleep for getchar()
	}

	return 0;
}

// only used when (g_prcbind = 0xff)  //no-bind mode
static void *process_thread(void *arg)
{
	VIDEO_RECORD* p_stream0 = (VIDEO_RECORD *)arg;
	HD_RESULT ret = HD_OK;

	HD_VIDEO_FRAME video_frame = {0};

	UINT64 start_time = 0, end_time = 0;

	p_stream0->prc_exit = 0;
	p_stream0->prc_loop = 0;
	p_stream0->prc_count = 0;
	//------ wait flow_start ------
	while (p_stream0->prc_enter == 0) usleep(100);

	//--------- pull data test ---------
	while (p_stream0->prc_exit == 0) {

		//printf("proc_pull ....\r\n");
		ret = hd_videoproc_pull_out_buf(p_stream0->proc_path, &video_frame, -1); // -1 = blocking mode
		if (ret != HD_OK) {
			if (ret != HD_ERR_UNDERRUN)
			printf("proc_pull error=%d !!\r\n\r\n", ret);
    			goto skip2;
		}
		//printf("proc frame.count = %llu\r\n", video_frame.count);

		//printf("enc_push ....\r\n");
		if (!p_stream0->enc_exit) {
			start_time = hd_gettime_us();
			ret = hd_videoenc_push_in_buf(p_stream0->enc_path, &video_frame, NULL, -1); // blocking mode
			if (ret != HD_OK) {
				printf("enc_push error=%d , release buffer!!\r\n\r\n", ret);
				hd_videoproc_release_out_buf(p_stream0->proc_path, &video_frame);
				goto skip2;
			}
			end_time = hd_gettime_us();
		}

		if (g_yuv_que) {
			if (p_stream0->prc_count < (g_yuv_que + DEPTH_NUM)) {
				printf("prc_pull frame.count = %llu %llu addr=%x %llu\r\n", video_frame.count, video_frame.timestamp, video_frame.phy_addr[0], (end_time - start_time));
			}
		}
		//else {
		//	if ((end_time - start_time) > 1000000) {  //for refer a,b check enc > 1 sec
		//		printf("enc %llu\r\n", (end_time - start_time));
		//	}
		//}

		//printf("proc_release ....\r\n");
		ret = hd_videoproc_release_out_buf(p_stream0->proc_path, &video_frame);
		if (ret != HD_OK) {
			printf("proc_release error=%d !!\r\n\r\n", ret);
    			goto skip2;
		}

		p_stream0->prc_count ++;
		//printf("process count = %d\r\n", p_stream0->prc_count);
skip2:
		p_stream0->prc_loop++;
		usleep(100); //sleep for getchar()
	}

	return 0;
}

static void *process_thread2(void *arg)
{
	VIDEO_RECORD* p_stream = (VIDEO_RECORD *)arg;
	VIDEO_RECORD* p_stream3 = p_stream + 3;
	VIDEO_RECORD* p_stream4 = p_stream + 4;

	HD_RESULT ret = HD_OK;

	HD_VIDEO_FRAME video_frame = {0};

	UINT64 start_time = 0, end_time = 0;

	p_stream3->prc_exit = 0;
	p_stream3->prc_loop = 0;
	p_stream3->prc_count = 0;
	//------ wait flow_start ------
	while (p_stream3->prc_enter == 0) usleep(100);

	//--------- pull data test ---------
	while (p_stream3->prc_exit == 0) {

		//printf("proc_pull ....\r\n");
		ret = hd_videoproc_pull_out_buf(p_stream3->proc_path, &video_frame, -1); // -1 = blocking mode
		if (ret != HD_OK) {
			if (ret != HD_ERR_UNDERRUN)
				printf("proc_pull error=%d !!\r\n\r\n", ret);
			goto skip2;
		}
		//printf("proc frame.count = %llu\r\n", video_frame.count);

		//printf("enc_push ....\r\n");
		if(!p_stream4->enc_exit) {
			start_time = hd_gettime_us();
			ret = hd_videoenc_push_in_buf(p_stream4->enc_path, &video_frame, NULL, -1); // blocking mode
			if (ret != HD_OK) {
				printf("enc_push4 error=%d , release buffer!!\r\n\r\n", ret);
				hd_videoproc_release_out_buf(p_stream3->proc_path, &video_frame);
				goto skip2;
			}
			end_time = hd_gettime_us();
		}

		if (g_yuv_que) {
			if (p_stream3->prc_count < (g_yuv_que + DEPTH_NUM)) {
				printf("prc2_pull frame.count = %llu %llu addr=%x %llu\r\n", video_frame.count, video_frame.timestamp, video_frame.phy_addr[0], (end_time - start_time));
			}
		}
		//else {
		//	if ((end_time - start_time) > 1000000) {  //for refer a,b check enc > 1 sec
		//		printf("enc %llu\r\n", (end_time - start_time));
		//	}
		//}

		//printf("proc_release ....\r\n");
		ret = hd_videoproc_release_out_buf(p_stream3->proc_path, &video_frame);
		if (ret != HD_OK) {
			printf("proc_release error=%d !!\r\n\r\n", ret);
			goto skip2;
		}

		p_stream3->prc_count ++;
		//printf("process count = %d\r\n", p_stream3->prc_count);
skip2:
		p_stream3->prc_loop++;
		usleep(100); //sleep for getchar()
	}

	return 0;
}

static void *encode_thread(void *arg)
{
	VIDEO_RECORD* p_stream0 = (VIDEO_RECORD *)arg;
	VIDEO_RECORD* p_stream1 = p_stream0 + 1;
	VIDEO_RECORD* p_stream2 = p_stream0 + 2;
	VIDEO_RECORD* p_stream4 = p_stream0 + 4;
	VIDEO_RECORD* p_stream5 = p_stream0 + 5;
	VIDEO_RECORD* p_stream6 = p_stream0 + 6;

	HD_RESULT ret = HD_OK;
	UINT32 poll_num = 0;
	UINT32 main_poll_id = 0, snap_poll_id = 0, sub_poll_id = 0, main2_poll_id = 0, snap2_poll_id = 0, sub2_poll_id = 0;
	HD_VIDEOENC_BS  data_pull;
	UINT32 i = 0;
	UINT32 j;
	HD_VIDEOENC_POLL_LIST poll_list[6];

	UINTPTR vir_addr_main = 0;
	HD_VIDEOENC_BUFINFO phy_buf_main;
	char file_path_main[32] = "/mnt/sd/dump_bs_main.dat";
	FILE *f_out_main = NULL;
	#define PHY2VIRT_MAIN(pa) (vir_addr_main + (pa - phy_buf_main.buf_info.phy_addr))

	UINTPTR vir_addr_snapshot = 0;
	HD_VIDEOENC_BUFINFO phy_buf_snapshot;
	char file_path_snapshot[32]  = "/mnt/sd/dump_bs_snapshot.dat";
	FILE *f_out_snapshot = NULL;
	#define PHY2VIRT_SNAPSHOT(pa) (vir_addr_snapshot + (pa - phy_buf_snapshot.buf_info.phy_addr))

	UINTPTR vir_addr_sub = 0;
	HD_VIDEOENC_BUFINFO phy_buf_sub;
	char file_path_sub[32] = "/mnt/sd/dump_bs_sub.dat";
	FILE *f_out_sub = NULL;
	#define PHY2VIRT_SUB(pa) (vir_addr_sub + (pa - phy_buf_sub.buf_info.phy_addr))

	UINTPTR vir_addr_main2 = 0;
	HD_VIDEOENC_BUFINFO phy_buf_main2;
	char file_path_main2[32] = "/mnt/sd/dump_bs_main2.dat";
	FILE *f_out_main2 = NULL;
	#define PHY2VIRT_MAIN2(pa) (vir_addr_main2 + (pa - phy_buf_main2.buf_info.phy_addr))

	UINTPTR vir_addr_snapshot2 = 0;
	HD_VIDEOENC_BUFINFO phy_buf_snapshot2;
	char file_path_snapshot2[32]  = "/mnt/sd/dump_bs_snapshot2.dat";
	FILE *f_out_snapshot2 = NULL;
	#define PHY2VIRT_SNAPSHOT2(pa) (vir_addr_snapshot2 + (pa - phy_buf_snapshot2.buf_info.phy_addr))

	UINTPTR vir_addr_sub2 = 0;
	HD_VIDEOENC_BUFINFO phy_buf_sub2;
	char file_path_sub2[32] = "/mnt/sd/dump_bs_sub2.dat";
	FILE *f_out_sub2 = NULL;
	#define PHY2VIRT_SUB2(pa) (vir_addr_sub2 + (pa - phy_buf_sub2.buf_info.phy_addr))

	if (g_mainen) {
		poll_num++;
	}
	if (g_snapshoten) {
		poll_num++;
	}
	if (g_suben) {
		poll_num++;
	}
	if (g_mainen2) {
		poll_num++;
	}
	if (g_snapshoten2) {
		poll_num++;
	}
	if (g_suben2) {
		poll_num++;
	}

	//------ wait flow_start ------
	while (p_stream0->enc_enter == 0) sleep(1);

	// query physical address of bs buffer ( this can ONLY query after hd_videoenc_start() is called !! )
	if (g_mainen) {
		hd_videoenc_get(p_stream0->enc_path, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf_main);
	}
	if (g_snapshoten) {
		hd_videoenc_get(p_stream1->enc_path, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf_snapshot);
	}
	if (g_suben) {
		hd_videoenc_get(p_stream2->enc_path, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf_sub);
	}
	if (g_mainen2) {
		hd_videoenc_get(p_stream4->enc_path, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf_main2);
	}
	if (g_snapshoten2) {
		hd_videoenc_get(p_stream5->enc_path, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf_snapshot2);
	}
	if (g_suben2) {
		hd_videoenc_get(p_stream6->enc_path, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf_sub2);
	}

	// mmap for bs buffer (just mmap one time only, calculate offset to virtual address later)
	if (g_mainen) {
		vir_addr_main = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_main.buf_info.phy_addr, phy_buf_main.buf_info.buf_size);
	}
	if (g_snapshoten) {
		vir_addr_snapshot  = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_snapshot.buf_info.phy_addr, phy_buf_snapshot.buf_info.buf_size);
	}
	if (g_suben) {
		vir_addr_sub = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_sub.buf_info.phy_addr, phy_buf_sub.buf_info.buf_size);
	}
	if (g_mainen2) {
		vir_addr_main2 = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_main2.buf_info.phy_addr, phy_buf_main2.buf_info.buf_size);
	}
	if (g_snapshoten2) {
		vir_addr_snapshot2  = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_snapshot2.buf_info.phy_addr, phy_buf_snapshot2.buf_info.buf_size);
	}
	if (g_suben2) {
		vir_addr_sub2 = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_sub2.buf_info.phy_addr, phy_buf_sub2.buf_info.buf_size);
	}

	//----- open output files -----
	if (g_mainen) {
		if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
			HD_VIDEOENC_ERR("open file (%s) fail....\r\n", file_path_main);
		} else {
			printf("\r\ndump main bitstream to file (%s) ....\r\n", file_path_main);
		}
	}
	if (g_snapshoten) {
		if ((f_out_snapshot = fopen(file_path_snapshot, "wb")) == NULL) {
			HD_VIDEOENC_ERR("open file (%s) fail....\r\n", file_path_snapshot);
		} else {
			printf("\r\ndump sub  bitstream to file (%s) ....\r\n", file_path_snapshot);
		}
	}
	if (g_suben) {
		if ((f_out_sub = fopen(file_path_sub, "wb")) == NULL) {
			HD_VIDEOENC_ERR("open file (%s) fail....\r\n", file_path_sub);
		} else {
			printf("\r\ndump main bitstream to file (%s) ....\r\n", file_path_sub);
		}
	}
	if (g_mainen2) {
		if ((f_out_main2 = fopen(file_path_main2, "wb")) == NULL) {
			HD_VIDEOENC_ERR("open file (%s) fail....\r\n", file_path_main2);
		} else {
			printf("\r\ndump main2 bitstream to file (%s) ....\r\n", file_path_main2);
		}
	}
	if (g_snapshoten2) {
		if ((f_out_snapshot2 = fopen(file_path_snapshot2, "wb")) == NULL) {
			HD_VIDEOENC_ERR("open file (%s) fail....\r\n", file_path_snapshot2);
		} else {
			printf("\r\ndump snapshot2 bitstream to file (%s) ....\r\n", file_path_snapshot2);
		}
	}
	if (g_suben2) {
		if ((f_out_sub2 = fopen(file_path_sub2, "wb")) == NULL) {
			HD_VIDEOENC_ERR("open file (%s) fail....\r\n", file_path_sub2);
		} else {
			printf("\r\ndump sub2 bitstream to file (%s) ....\r\n", file_path_sub2);
		}
	}

	printf("\r\nif you want to stop, enter \"q\" to exit !!\r\n\r\n");

	//--------- pull data test ---------
	i = 0;
	if (g_mainen) {
		poll_list[i].path_id = p_stream0->enc_path;
		main_poll_id = i;
		i++;
	}
	if (g_snapshoten) {
		poll_list[i].path_id = p_stream1->enc_path;
		snap_poll_id = i;
		i++;
	}
	if (g_suben) {
		poll_list[i].path_id = p_stream2->enc_path;
		sub_poll_id = i;
		i++;
	}
	if (g_mainen2) {
		poll_list[i].path_id = p_stream4->enc_path;
		main2_poll_id = i;
		i++;
	}
	if (g_snapshoten2) {
		poll_list[i].path_id = p_stream5->enc_path;
		snap2_poll_id = i;
		i++;
	}
	if (g_suben2) {
		poll_list[i].path_id = p_stream6->enc_path;
		sub2_poll_id = i;
		i++;
	}

	/**
	 * SQA autotest timestamp mechanism (IVOT_N12009_CO-1066)
	 */
	int timestamp_debug = 0;
	char *timestamp_debug_env = getenv("TIMESTAMP_DEBUG");
	if (timestamp_debug_env && strncmp(timestamp_debug_env, "1", 1) == 0) {
		timestamp_debug = 1;
	}

	while (p_stream0->enc_exit == 0) {
		if (HD_OK == hd_videoenc_poll_list(poll_list, poll_num, -1)) {    // multi path poll_list , -1 = blocking mode
			if (g_mainen) {
				if (TRUE == poll_list[main_poll_id].revent.event) {
					//pull data
					ret = hd_videoenc_pull_out_buf(p_stream0->enc_path, &data_pull, 0); // 0 = non-blocking mode
					if (timestamp_debug) {
						printf("ts = %llu\r\n",data_pull.timestamp/1000);
					}
					if (ret == HD_OK) {
						for (j=0; j< data_pull.pack_num; j++) {
							UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(data_pull.video_pack[j].phy_addr);
							UINT32 len = data_pull.video_pack[j].size;
							if (f_out_main) fwrite(ptr, 1, len, f_out_main);
							if (f_out_main) fflush(f_out_main);
						}

						// release data
						ret = hd_videoenc_release_out_buf(p_stream0->enc_path, &data_pull);
						if (ret != HD_OK) {
							printf("enc_release error=%d !!\r\n", ret);
						}
					}
				}
			}

			if (g_snapshoten) {
				if (TRUE == poll_list[snap_poll_id].revent.event) {
					//pull data
					ret = hd_videoenc_pull_out_buf(p_stream1->enc_path, &data_pull, 0); // 0 = non-blocking mode

					if (ret == HD_OK) {
						for (j=0; j< data_pull.pack_num; j++) {
							UINT8 *ptr = (UINT8 *)PHY2VIRT_SNAPSHOT(data_pull.video_pack[j].phy_addr);
							UINT32 len = data_pull.video_pack[j].size;
							if (f_out_snapshot) fwrite(ptr, 1, len, f_out_snapshot);
							if (f_out_snapshot) fflush(f_out_snapshot);
						}

						// release data
						ret = hd_videoenc_release_out_buf(p_stream1->enc_path, &data_pull);
						if (ret != HD_OK) {
							printf("enc_release error=%d !!\r\n", ret);
						}
					}
				}
			}

			if (g_suben) {
				if (TRUE == poll_list[sub_poll_id].revent.event) {
					//pull data
					ret = hd_videoenc_pull_out_buf(p_stream2->enc_path, &data_pull, 0); // 0 = non-blocking mode

					if (ret == HD_OK) {
						for (j=0; j< data_pull.pack_num; j++) {
							UINT8 *ptr = (UINT8 *)PHY2VIRT_SUB(data_pull.video_pack[j].phy_addr);
							UINT32 len = data_pull.video_pack[j].size;
							if (f_out_sub) fwrite(ptr, 1, len, f_out_sub);
							if (f_out_sub) fflush(f_out_sub);
						}

						// release data
						ret = hd_videoenc_release_out_buf(p_stream2->enc_path, &data_pull);
						if (ret != HD_OK) {
							printf("enc_release error=%d !!\r\n", ret);
						}
					}
				}
			}

			if (g_mainen2) {
				if (TRUE == poll_list[main2_poll_id].revent.event) {
				//pull data
					ret = hd_videoenc_pull_out_buf(p_stream4->enc_path, &data_pull, 0); // 0 = non-blocking mode

					if (ret == HD_OK) {
						for (j=0; j< data_pull.pack_num; j++) {
							UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN2(data_pull.video_pack[j].phy_addr);
							UINT32 len = data_pull.video_pack[j].size;
							if (f_out_main2) fwrite(ptr, 1, len, f_out_main2);
							if (f_out_main2) fflush(f_out_main2);
						}

						// release data
						ret = hd_videoenc_release_out_buf(p_stream4->enc_path, &data_pull);
						if (ret != HD_OK) {
							printf("enc_release error=%d !!\r\n", ret);
						}
					}
				}
			}

			if (g_snapshoten2) {
				if (TRUE == poll_list[snap2_poll_id].revent.event) {
					//pull data
					ret = hd_videoenc_pull_out_buf(p_stream5->enc_path, &data_pull, 0); // 0 = non-blocking mode

					if (ret == HD_OK) {
						for (j=0; j< data_pull.pack_num; j++) {
							UINT8 *ptr = (UINT8 *)PHY2VIRT_SNAPSHOT2(data_pull.video_pack[j].phy_addr);
							UINT32 len = data_pull.video_pack[j].size;
							if (f_out_snapshot2) fwrite(ptr, 1, len, f_out_snapshot2);
							if (f_out_snapshot2) fflush(f_out_snapshot2);
						}

						// release data
						ret = hd_videoenc_release_out_buf(p_stream5->enc_path, &data_pull);
						if (ret != HD_OK) {
							printf("enc_release error=%d !!\r\n", ret);
						}
					}
				}
			}

			if (g_suben2) {
				if (TRUE == poll_list[sub2_poll_id].revent.event) {
					//pull data
					ret = hd_videoenc_pull_out_buf(p_stream6->enc_path, &data_pull, 0); // 0 = non-blocking mode

					if (ret == HD_OK) {
						for (j=0; j< data_pull.pack_num; j++) {
							UINT8 *ptr = (UINT8 *)PHY2VIRT_SUB2(data_pull.video_pack[j].phy_addr);
							UINT32 len = data_pull.video_pack[j].size;
							if (f_out_sub2) fwrite(ptr, 1, len, f_out_sub2);
							if (f_out_sub2) fflush(f_out_sub2);
						}

						// release data
						ret = hd_videoenc_release_out_buf(p_stream6->enc_path, &data_pull);
						if (ret != HD_OK) {
							printf("enc_release error=%d !!\r\n", ret);
						}
					}
				}
			}
		}
	}

	// mummap for bs buffer
	if (g_mainen) {
		if (vir_addr_main) hd_common_mem_munmap((void *)vir_addr_main, phy_buf_main.buf_info.buf_size);
	}
	if (g_snapshoten) {
		if (vir_addr_snapshot)  hd_common_mem_munmap((void *)vir_addr_snapshot, phy_buf_snapshot.buf_info.buf_size);
	}
	if (g_suben) {
		if (vir_addr_sub) hd_common_mem_munmap((void *)vir_addr_sub, phy_buf_sub.buf_info.buf_size);
	}
	if (g_mainen2) {
		if (vir_addr_main2) hd_common_mem_munmap((void *)vir_addr_main2, phy_buf_main2.buf_info.buf_size);
	}
	if (g_snapshoten2) {
		if (vir_addr_snapshot2)  hd_common_mem_munmap((void *)vir_addr_snapshot2, phy_buf_snapshot2.buf_info.buf_size);
	}
	if (g_suben2) {
		if (vir_addr_sub2) hd_common_mem_munmap((void *)vir_addr_sub2, phy_buf_sub2.buf_info.buf_size);
	}

	// close output file
	if (g_mainen) {
		if (f_out_main) fclose(f_out_main);
	}
	if (g_snapshoten) {
		if (f_out_snapshot) fclose(f_out_snapshot);
	}
	if (g_suben) {
		if (f_out_sub) fclose(f_out_sub);
	}
	if (g_mainen2) {
		if (f_out_main2) fclose(f_out_main2);
	}
	if (g_snapshoten2) {
		if (f_out_snapshot2) fclose(f_out_snapshot2);
	}
	if (g_suben2) {
		if (f_out_sub2) fclose(f_out_sub2);
	}

	return 0;
}

static int setup_vdo_size(void)
{
#if (AUTO_RESOLUTION_SET && defined(__LINUX))
	char *sensor1_name = getenv("SENSOR1");
	if (sensor1_name) {
		int i;
		int resolution = -1;
		int n = sizeof(SENSOR_NAME_TBL)/sizeof(SENSOR_NAME_TBL[0]);
		for (i = 0; i < n; i++) {
			if (strncmp(sensor1_name, SENSOR_NAME_TBL[i], strlen(SENSOR_NAME_TBL[i])+1) == 0) {
				resolution = i;
				break;
			}
		}
		if (resolution == -1) {
			printf("unable to handle sensor: %s, does its support add to SENSOR_NAME_TBL, VDO_SIZE_W_TBL and VDO_SIZE_W_TBL", RESOLUTION_SET);
			return -1;
		}
		RESOLUTION_SET = (UINT32)resolution;
	} else {
		printf("failed to getenv:'SENSOR1', use default RESOLUTION_SET: %d \n", RESOLUTION_SET);
	}
#endif
	if (RESOLUTION_SET >= sizeof(VDO_SIZE_W_TBL)/sizeof(VDO_SIZE_W_TBL[0])) {
		printf("invalid RESOLUTION_SET: %d\n", RESOLUTION_SET);
		return -1;
	}
	printf("setup RESOLUTION_SET to %s\n", SENSOR_NAME_TBL[RESOLUTION_SET]);
	VDO_SIZE_W = VDO_SIZE_W_TBL[RESOLUTION_SET];
	VDO_SIZE_H = VDO_SIZE_H_TBL[RESOLUTION_SET];
	return 0;
}

static void *acapture_thread(void *arg)
{
	HD_RESULT ret = HD_OK;
	HD_AUDIO_FRAME  data_pull;
	UINTPTR vir_addr_main;
	HD_AUDIOCAP_BUFINFO phy_buf_main;
	char file_path_main[64], file_path_len[64];
	FILE *f_out_main, *f_out_len;
	VIDEO_RECORD* p_cap_only = (VIDEO_RECORD *)arg;

	#define PHY2VIRT_MAIN(pa) (vir_addr_main + (pa - phy_buf_main.buf_info.phy_addr))

	/* config pattern name */
	snprintf(file_path_main, sizeof(file_path_main), "/mnt/sd/audio_bs_%d_%d_%d_pcm.dat", HD_AUDIO_BIT_WIDTH_16, HD_AUDIO_SOUND_MODE_MONO, p_cap_only->acap_sr);
	snprintf(file_path_len, sizeof(file_path_len), "/mnt/sd/audio_bs_%d_%d_%d_pcm.len", HD_AUDIO_BIT_WIDTH_16, HD_AUDIO_SOUND_MODE_MONO, p_cap_only->acap_sr);

	/* wait flow_start */
	while (p_cap_only->acap_enter == 0) {
		usleep(100);
	}

	/* query physical address of bs buffer
	  (this can ONLY query after hd_audiocap_start() is called !!) */
	hd_audiocap_get(p_cap_only->acap_ctrl, HD_AUDIOCAP_PARAM_BUFINFO, &phy_buf_main);

	/* mmap for bs buffer
	  (just mmap one time only, calculate offset to virtual address later) */
	vir_addr_main = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_main.buf_info.phy_addr, phy_buf_main.buf_info.buf_size);

	if (vir_addr_main == 0) {
		printf("acap mmap error\r\n");
		return 0;
	}

	/* open output files */
	if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
		printf("open file (%s) fail....\r\n", file_path_main);
	} else {
		//printf("\r\ndump main bitstream to file (%s) ....\r\n", file_path_main);
	}

	if ((f_out_len = fopen(file_path_len, "wb")) == NULL) {
		printf("open len file (%s) fail....\r\n", file_path_len);
	}

	/* pull data test */
	while (p_cap_only->acap_exit == 0) {
		// pull data
		ret = hd_audiocap_pull_out_buf(p_cap_only->acap_path, &data_pull, 1000); // >1 = timeout mode

		if (ret == HD_OK) {
			UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(data_pull.phy_addr[0]);
			UINT32 size = data_pull.size;
			UINT32 timestamp = hd_gettime_ms();
			// write bs
			if (f_out_main) fwrite(ptr, 1, size, f_out_main);
			if (f_out_main) fflush(f_out_main);

			// write bs len
			if (f_out_len) fprintf(f_out_len, "%d %d\n", size, timestamp);
			if (f_out_len) fflush(f_out_len);

			// release data
			ret = hd_audiocap_release_out_buf(p_cap_only->acap_path, &data_pull);
			if (ret != HD_OK) {
				printf("release buffer failed. ret=%x\r\n", ret);
			}
		}
	}

	/* mummap for bs buffer */
	hd_common_mem_munmap((void *)vir_addr_main, phy_buf_main.buf_info.buf_size);

	/* close output file */
	if (f_out_main) fclose(f_out_main);
	if (f_out_len) fclose(f_out_len);

	return 0;
}

#if (AOUT_PLAYBACK==ENABLE)
static void *aplayback_thread(void *arg)
{
	INT ret, bs_size, result;
	CHAR filename[50];
	FILE *bs_fd, *len_fd;
	HD_AUDIO_FRAME  bs_in_buf = {0};
	HD_COMMON_MEM_VB_BLK blk;
	UINTPTR pa, va;
	UINT32 blk_size = 0x100000;
	HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;
	UINTPTR bs_buf_start, bs_buf_curr, bs_buf_end;
	INT timestamp;
	VIDEO_RECORD *p_out_only = (VIDEO_RECORD *)arg;

	/* read test pattern */
	snprintf(filename, sizeof(filename), "/mnt/sd/audio_bs_%d_%d_%d_pcm.dat", HD_AUDIO_BIT_WIDTH_16, HD_AUDIO_SOUND_MODE_STEREO, p_out_only->acap_sr);
	bs_fd = fopen(filename, "rb");
	if (bs_fd == NULL) {
		printf("[ERROR] Open %s failed!!\n", filename);
		return 0;
	}
	printf("play file: [%s]\n", filename);

	snprintf(filename, sizeof(filename), "/mnt/sd/audio_bs_%d_%d_%d_pcm.len", HD_AUDIO_BIT_WIDTH_16, HD_AUDIO_SOUND_MODE_STEREO, p_out_only->acap_sr);
	len_fd = fopen(filename, "rb");
	if (len_fd == NULL) {
		printf("[ERROR] Open %s failed!!\n", filename);
		goto play_fclose;
	}
	printf("len file: [%s]\n", filename);

	/* get memory */
	blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_POOL_BEGIN, blk_size, ddr_id); // Get block from mem pool
	if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("get block fail, blk = 0x%lx\n", (unsigned long)blk);
		goto play_fclose;
	}
	pa = hd_common_mem_blk2pa(blk); // get physical addr
	if (pa == 0) {
		printf("blk2pa fail, blk(0x%lx)\n", (unsigned long)blk);
		goto rel_blk;
	}
	if (pa > 0) {
		va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, blk_size); // Get virtual addr
		if (va == 0) {
			printf("get va fail, va(0x%lx)\n", (unsigned long)blk);
			goto rel_blk;
		}
		/* allocate bs buf */
		bs_buf_start = va;
		bs_buf_curr = bs_buf_start;
		bs_buf_end = bs_buf_start + (ULONG)blk_size;
		printf("alloc bs_buf: start(0x%lx) curr(0x%lx) end(0x%lx) size(0x%lx)\n", (unsigned long)bs_buf_start, (unsigned long)bs_buf_curr, (unsigned long)bs_buf_end, (unsigned long)blk_size);
	}

	while (1) {
		if (p_out_only->aout_exit == 1) {
			p_out_only->aout_enter = 0;
			break;
		}

		/* get bs size */
		if (fscanf(len_fd, "%d %d\n", &bs_size, &timestamp) == EOF) {
			// reach EOF, read from the beginning
			fseek(bs_fd, 0, SEEK_SET);
			fseek(len_fd, 0, SEEK_SET);
			if (fscanf(len_fd, "%d %d\n", &bs_size, &timestamp) == EOF) {
				printf("[ERROR] fscanf error\n");
				continue;
			}
		}

		/* check bs buf rollback */
		if ((bs_buf_curr + (ULONG)bs_size) > bs_buf_end) {
			bs_buf_curr = bs_buf_start;
		}

		/* read bs from file */
		result = fread((void *)bs_buf_curr, 1, bs_size, bs_fd);
		if (result != bs_size) {
			printf("reading error\n");
			continue;
		}
		bs_in_buf.sign = MAKEFOURCC('A','F','R','M');
		bs_in_buf.phy_addr[0] = pa + (bs_buf_curr - bs_buf_start); // needs to add offset
		bs_in_buf.size = bs_size;
		bs_in_buf.ddr_id = ddr_id;
		bs_in_buf.timestamp = hd_gettime_us();
		bs_in_buf.bit_width = HD_AUDIO_BIT_WIDTH_16;
		bs_in_buf.sound_mode = HD_AUDIO_SOUND_MODE_STEREO;
		bs_in_buf.sample_rate = p_out_only->acap_sr;

resend:
		ret = hd_audioout_push_in_buf(p_out_only->aout_path, &bs_in_buf, -1);
		if (ret != HD_OK) {
			//printf("hd_audioout_push_in_buf fail, ret(%d)\n", ret);
			usleep(10000);
			goto resend;
		}

		bs_buf_curr += ALIGN_CEIL_4(bs_size); // shift to next
	}

	/* release memory */
	hd_common_mem_munmap((void*)va, blk_size);
rel_blk:
	ret = hd_common_mem_release_block(blk);
	if (HD_OK != ret) {
		printf("release blk fail, ret(%d)\n", ret);
		goto play_fclose;
	}

play_fclose:
	if (bs_fd != NULL) { fclose(bs_fd);}
	if (len_fd != NULL) { fclose(len_fd);}

	return 0;
}
#endif

int init_osg(VIDEO_RECORD *p_stream)
{
	int ret;

	if(!p_stream){
		printf("p_stream is NULL\n");
		return -1;
	}

	// init stamp data
	p_stream->stamp_blk  = 0;
	p_stream->stamp_pa   = 0;
	p_stream->stamp_size = ALIGN_CEIL(STAMP_WIDTH*STAMP_HEIGHT*2*2, 128);

	ret = mem_alloc(p_stream->stamp_size, &(p_stream->stamp_blk), &(p_stream->stamp_pa));
	if(ret){
		printf("fail to allocate stamp buffer\n");
		return -1;
	}

	//setup enc stamp parameter
	if(set_enc_stamp_param(p_stream->enc_stamp_path, p_stream->stamp_pa, p_stream->stamp_size)){
		printf("fail to set enc stamp\r\n");
		return -1;
	}

	//render enc stamp
	ret = hd_videoenc_start(p_stream->enc_stamp_path);
	if (ret != HD_OK) {
		printf("start enc stamp fail=%d\n", ret);
		return -1;
	}

	return 0;
}

MAIN(argc, argv)
{
	HD_RESULT ret;
	INT key;
	VIDEO_RECORD stream[7] = {0}; //0: main stream
	UINT32 sensor_on = 0;
	UINT32 enc_type = 0, main2_enc_type = 0;
	UINT32 snapshot_enc_type = 0;
	UINT32 sub_enc_type = 0, sub2_enc_type = 0;
	HD_DIM main_dim;
	HD_DIM sub_dim;

	#if (ISP_RELOAD_ENABLE == 1)
	#if (ISP_RELOAD == ISP_RELOAD_DTSI)
	IQT_NIGHT_MODE iq_night_mode;
	#endif

	#if defined(__LINUX)
	#if (ISP_RELOAD == ISP_RELOAD_DTSI)
	IQT_DTSI_INFO iq_dtsi_info = {0};
	AET_DTSI_INFO ae_dtsi_info = {0};
	AWBT_DTSI_INFO awb_dtsi_info = {0};
	#else
	AET_CFG_INFO cfg_info = {0};
	#endif
	#endif
	#endif

	if (setup_vdo_size() != 0) {
		return 0;
	}

	if (argc == 1 || argc > 22) {
		printf("Usage: <enc-type> <cap_out_bind> <cap_out_fmt> <prc_out_bind> <prc_out_fmt> <acap_enable> <shdr_enable> <snapshot_enable> <sub_enable> <sub_enc_type> <main2_enable> <main2_enc_type> <snapshot2_enable> <sub2_enable> <sub2_enc_type> <sensor_on>.\r\n");
		printf("Help:\r\n");
		printf("  <enc_type> : 0(H265), 1(H264), 2(MJPG)\r\n");
		printf("  <cap_out_bind>: 0(D2D Mode), 1(Direct Mode), 2(OneBuf Mode), x(no-bind)\r\n");
		printf("  <cap_out_fmt> : 0(RAW12),  1(NRX12 - RAW compress)\r\n");
		printf("  <prc_out_bind>: 0(D2D Mode), 1(LowLatency Mode), 2(OneBuf Mode), x(no-bind)\r\n");
		printf("  <prc_out_fmt> : 0(YUV420), 1(NVX - YUV compress)\r\n");
		printf("  <acap_enable> : 0(Disable acap), 1(Enable acap)\r\n");
		printf("  <shdr_enable> : 0(Disable SHDR), 1(Enable SHDR)\r\n");
		printf("  <snapshot_enable> : 0(Disable snapshot), 1(Enable snapshot)\r\n");
		printf("  <sub_enable> : 0(Disable substream), 1(Enable substream)\r\n");
		printf("  <sub_enc_type> : 0(H265), 1(H264), 2(MJPG)\r\n");
		printf("  <main2_enable> : 0(Disable main2 stream), 1(Enable main2 stream)\r\n");
		printf("  <main2_enc_type> : 0(H265), 1(H264), 2(MJPG)\r\n");
		printf("  <snapshot2_enable> : 0(Disable snapshot2), 1(Enable snapshot2)\r\n");
		printf("  <sub2_enable> : 0(Disable sub2 stream), 1(Enable sub2 stream)\r\n");
		printf("  <sub2_enc_type> : 0(H265), 1(H264), 2(MJPG)\r\n");
		printf("  <sensor_on> : 0(single sensor) 1(dual-sensor 1 on), 2(dual-sensor 2 on), 3(dual-sensor all on)\r\n");
		printf("  <prc_mask> : 0(off), 1(draw a polygon on video)\r\n");
		printf("  <osg> : 0(off), 1(draw a novatek logo on main stream and snapshot stream)\r\n");
		printf("  <rtos_ai> : 0(off), 1(reserve a hdal memory for rtos ai)\r\n");
		printf("  <prc_out_id> : 0, 1, 2, 3, 4\r\n");
		printf("  <yuv_queue> : 0(Disable yuv queue), 1 ~ n(Enable yuv queue, related to mem config)\r\n");
		return 0;
	}

	// query program options
	if (argc > 1) {
		enc_type = atoi(argv[1]);
		printf("enc_type %d\r\n", enc_type);
		if(enc_type > 2) {
			printf("error: not support enc_type!\r\n");
			return 0;
		}
	}
	if (argc > 2) {
		if (argv[2][0] == 'x') {
			g_capbind = 0xff;
			printf("CAP-BIND x\r\n");
		} else {
			g_capbind = atoi(argv[2]);
			printf("CAP-BIND %d\r\n", g_capbind);
			if(g_capbind > 3) {
				printf("error: not support CAP-BIND! (%d) \r\n", g_capbind);
				return 0;
			}
		}
	}
	if (argc > 3) {
		g_capfmt = atoi(argv[3]);
		printf("CAP-FMT %d\r\n", g_capfmt);
		if(g_capfmt > 1) {
			printf("error: not support CAP-FMT! (%d) \r\n", g_capfmt);
			return 0;
		}
	}
	if (argc > 4) {
		if (argv[4][0] == 'x') {
			g_prcbind = 0xff;
			printf("PRC-BIND x\r\n");
		} else {
			g_prcbind = atoi(argv[4]);
			printf("PRC-BIND %d\r\n", g_prcbind);
			if(g_prcbind > 3) {
				printf("error: not support PRC-BIND! (%d) \r\n", g_prcbind);
				return 0;
			}
		}
	}
	if (argc > 5) {
		g_prcfmt = atoi(argv[5]);
		printf("PRC-FMT %d\r\n", g_prcfmt);
		if(g_prcfmt > 2) {
			printf("error: not support PRC-FMT ! (%d) \r\n", g_prcfmt);
			return 0;
		}
	}
	if (argc > 6) {
		g_acapen = atoi(argv[6]);
		printf("ACAP-EN %d\r\n", g_acapen);
	}
	if (argc > 7) {
		g_shdr = atoi(argv[7]);
		printf("SHDR-EN %d\r\n", g_shdr);
	}
	if (argc > 8) {
		g_snapshoten = atoi(argv[8]);
		printf("SNAPSHOT-EN %d\r\n", g_snapshoten);
	}
	if (argc > 9) {
		g_suben = atoi(argv[9]);
		printf("SUBSTREAM-EN %d\r\n", g_suben);
	}
	if (argc > 10) {
		sub_enc_type = atoi(argv[10]);
		printf("sub_enc_type %d\r\n", sub_enc_type);
		if(sub_enc_type > 2) {
			printf("error: not support sub_enc_type!\r\n");
			return 0;
		}
	}
	if (argc > 11) {
		g_mainen2 = atoi(argv[11]);
		printf("MAIN2 STREAM-EN %d\r\n", g_mainen2);
	}
	if (argc > 12) {
		main2_enc_type = atoi(argv[12]);
		printf("main2_enc_type %d\r\n", main2_enc_type);
		if(main2_enc_type > 2) {
			printf("error: not support main2_enc_type!\r\n");
			return 0;
		}
	}
	if (argc > 13) {
		g_snapshoten2 = atoi(argv[13]);
		printf("SNAPSHOT2-EN %d\r\n", g_snapshoten2);
	}
	if (argc > 14) {
		g_suben2 = atoi(argv[14]);
		printf("SUB2 STREAM-EN %d\r\n", g_suben2);
	}
	if (argc > 15) {
		sub2_enc_type = atoi(argv[15]);
		printf("sub2_enc_type %d\r\n", sub2_enc_type);
		if(sub2_enc_type > 2) {
			printf("error: not support sub2_enc_type!\r\n");
			return 0;
		}
	}
	if (argc > 16) {
		sensor_on = atoi(argv[16]);
		printf("sensor_on %d\r\n", sensor_on);
		if(sensor_on > 4) {
			printf("error: not support sensor_on!\r\n");
			return 0;
		}
	}
	if (argc > 17) {
		g_prc_mask = atoi(argv[17]);
		printf("prc_mask %d\r\n", g_prc_mask);
		if(g_prc_mask > 1) {
			printf("error: not support prc_mask!\r\n");
			return 0;
		}
	}
	if (argc > 18) {
		g_osg = atoi(argv[18]);
		printf("codec/jpeg osg %d\r\n", g_osg);
		if(g_osg > 1) {
			printf("error: not support codec/jpeg osg!\r\n");
			return 0;
		}
	}
	if (argc > 19) {
		g_rtos_ai = atoi(argv[19]);
		printf("rtos ai %d\r\n", g_rtos_ai);
		if(g_rtos_ai > 1) {
			printf("error: not support rtos ai!\r\n");
			return 0;
		}
	}
	if (argc > 20) {
		g_prcout = atoi(argv[20]);
		printf("prc path id %d\r\n", g_prcout);
		if(g_prcout > 4) {
			printf("error: not support prc path id!\r\n");
			return 0;
		}
	}
	if (argc > 21) {
		g_yuv_que = atoi(argv[21]);
		printf("yuv queue %d\r\n", g_yuv_que);
		//if(g_yuv_que > 1) {
		//	printf("error: not support yuv queue!\r\n");
		//	return 0;
		//}
		if (g_prcbind != 0xff) {
			printf("yuv queue only support PRC-BIND x! disable yuv queue!\r\n");
			g_yuv_que = 0;
		}
		if (g_yuv_que) {
			printf("enable yuv queue, need to stop venc ('b') before dump dtsi ('i')\r\n");
		}
	}

#if defined(__LINUX)
	char *sensor_env = NULL;
	sensor_env = getenv("SENSOR_ON");
	if (sensor_env) {
		if (strcmp(sensor_env, "sensor1") == 0) {
			printf("SENSOR1 ON !!!\r\n");
			sensor_on = 1;
		} else if (strcmp(sensor_env, "sensor2") == 0) {
			printf("SENSOR2 ON !!!\r\n");
			sensor_on = 2;
		} else if (strcmp(sensor_env, "dual_sensor") == 0) {
			printf("DUAL_SENSOR ON !!!\r\n");
			sensor_on = 3;
		}
	}
	sensor_en_map = sensor_on;
	if (!sensor1_enabled()) {
		g_mainen = 0;
	}
	if (sensor2_enabled()) {
		g_mainen2 = 1;
	}
	printf("sensor_en_map %d\r\n", sensor_en_map);
#endif
	if (sensor1_enabled() && sensor2_enabled() && g_capbind == 1) {
		printf("daul sensor should be D2D mode\r\n");
		g_capbind = 0;
	}

		// assign g_capfmt
		if (RESOLUTION_SET == 9 || RESOLUTION_SET == 10) {
			g_capfmt = HD_VIDEO_PXLFMT_YUV422;
		} else {
			if (g_capfmt == 0) {
				g_capfmt = HD_VIDEO_PXLFMT_RAW12;
			} else {
				g_capfmt =  HD_VIDEO_PXLFMT_NRX12;
			}
		}

		// assign g_prcfmt
		if (g_prcfmt == 0) {
			g_prcfmt = HD_VIDEO_PXLFMT_YUV420;
		} else {
			#if 0 //680
				if (enc_type == 0) {
					g_prcfmt = HD_VIDEO_PXLFMT_YUV420_NVX1_H265;
				} else if (enc_type == 1) {
					g_prcfmt = HD_VIDEO_PXLFMT_YUV420_NVX1_H264;
				} else {
					g_prcfmt = HD_VIDEO_PXLFMT_YUV420;
				}
			#else //520
				g_prcfmt = HD_VIDEO_PXLFMT_YUV420_NVX2;
			#endif
		}

	#if (ISP_RELOAD_ENABLE == 1)
	#if defined(__LINUX)
	// load ae/awb/iq paramters
	if (vendor_isp_init() == HD_OK) {
		#if (ISP_RELOAD == ISP_RELOAD_DTSI)
		iq_dtsi_info.id = 0;
		ae_dtsi_info.id = 0;
		awb_dtsi_info.id = 0;

		iq_night_mode.id = 0;
		vendor_isp_get_iq(IQT_ITEM_NIGHT_MODE, &iq_night_mode);

		switch(RESOLUTION_SET) {
		default:
		case 0:
			if (g_shdr == 1) {
				strncpy(ae_dtsi_info.node_path, "/isp/ae/imx290_ae_0", DTSI_NAME_LENGTH);
				strncpy(awb_dtsi_info.node_path, "/isp/awb/imx290_awb_0", DTSI_NAME_LENGTH);
				strncpy(iq_dtsi_info.node_path, "/isp/iq/imx290_iq_hdr_0", DTSI_NAME_LENGTH);
				strncpy(ae_dtsi_info.file_path, "/mnt/app/isp/isp.dtb", DTSI_NAME_LENGTH);
				strncpy(awb_dtsi_info.file_path, "/mnt/app/isp/isp.dtb", DTSI_NAME_LENGTH);
				strncpy(iq_dtsi_info.file_path, "/mnt/app/isp/isp.dtb", DTSI_NAME_LENGTH);
				printf("Load /mnt/app/isp/isp.dtb\n");
			} else {
				strncpy(ae_dtsi_info.node_path, "/isp/ae/imx290_ae_0", DTSI_NAME_LENGTH);
				strncpy(awb_dtsi_info.node_path, "/isp/awb/imx290_awb_0", DTSI_NAME_LENGTH);
				strncpy(iq_dtsi_info.node_path, "/isp/iq/imx290_iq_0", DTSI_NAME_LENGTH);
				strncpy(ae_dtsi_info.file_path, "/mnt/app/isp/isp.dtb", DTSI_NAME_LENGTH);
				strncpy(awb_dtsi_info.file_path, "/mnt/app/isp/isp.dtb", DTSI_NAME_LENGTH);
				strncpy(iq_dtsi_info.file_path, "/mnt/app/isp/isp.dtb", DTSI_NAME_LENGTH);
				printf("Load /mnt/app/isp/isp.dtb\n");
			}
			break;
		case 5:
				strncpy(ae_dtsi_info.node_path, "/isp/ae/f37_ae_0", DTSI_NAME_LENGTH);
				strncpy(awb_dtsi_info.node_path, "/isp/awb/f37_awb_0", DTSI_NAME_LENGTH);
				strncpy(iq_dtsi_info.node_path, "/isp/iq/f37_iq_0", DTSI_NAME_LENGTH);
				strncpy(ae_dtsi_info.file_path, "/mnt/app/isp/isp.dtb", DTSI_NAME_LENGTH);
				strncpy(awb_dtsi_info.file_path, "/mnt/app/isp/isp.dtb", DTSI_NAME_LENGTH);
				strncpy(iq_dtsi_info.file_path, "/mnt/app/isp/isp.dtb", DTSI_NAME_LENGTH);
				printf("Load /mnt/app/isp/isp.dtb\n");
			break;
		case 6:
				if (g_shdr == 1) {
					strncpy(iq_dtsi_info.node_path, "/isp/iq/f35_iq_hdr_0", DTSI_NAME_LENGTH);
					strncpy(ae_dtsi_info.node_path, "/isp/ae/f35_ae_0", DTSI_NAME_LENGTH);
				} else {
					if(iq_night_mode.mode == 1) {
						strncpy(iq_dtsi_info.node_path, "/isp/iq/f35_iq_ir_0", DTSI_NAME_LENGTH);
						strncpy(ae_dtsi_info.node_path, "/isp/ae/f35_ae_ir_0", DTSI_NAME_LENGTH);
					} else {
						strncpy(iq_dtsi_info.node_path, "/isp/iq/f35_iq_0", DTSI_NAME_LENGTH);
						strncpy(ae_dtsi_info.node_path, "/isp/ae/f35_ae_0", DTSI_NAME_LENGTH);
					}
				}
				strncpy(awb_dtsi_info.node_path, "/isp/awb/f35_awb_0", DTSI_NAME_LENGTH);
				strncpy(ae_dtsi_info.file_path, "/mnt/app/isp/isp.dtb", DTSI_NAME_LENGTH);
				strncpy(awb_dtsi_info.file_path, "/mnt/app/isp/isp.dtb", DTSI_NAME_LENGTH);
				strncpy(iq_dtsi_info.file_path, "/mnt/app/isp/isp.dtb", DTSI_NAME_LENGTH);
				printf("Load /mnt/app/isp/isp.dtb\n");
			break;
		case 9:
			strncpy(ae_dtsi_info.node_path, "/isp/ae/tc358840_ae_0", DTSI_NAME_LENGTH);
			strncpy(awb_dtsi_info.node_path, "/isp/awb/tc358840_awb_0", DTSI_NAME_LENGTH);
			strncpy(iq_dtsi_info.node_path, "/isp/iq/tc358840_iq_0", DTSI_NAME_LENGTH);
			strncpy(ae_dtsi_info.file_path, "/mnt/app/isp/isp.dtb", DTSI_NAME_LENGTH);
			strncpy(awb_dtsi_info.file_path, "/mnt/app/isp/isp.dtb", DTSI_NAME_LENGTH);
			strncpy(iq_dtsi_info.file_path, "/mnt/app/isp/isp.dtb", DTSI_NAME_LENGTH);
			printf("Load /mnt/app/isp/isp.dtb\n");
			break;
		case 10:
			strncpy(ae_dtsi_info.node_path, "/isp/ae/tp2855_ae_0", DTSI_NAME_LENGTH);
			strncpy(awb_dtsi_info.node_path, "/isp/awb/tp2855_awb_0", DTSI_NAME_LENGTH);
			strncpy(iq_dtsi_info.node_path, "/isp/iq/tp2855_iq_0", DTSI_NAME_LENGTH);
			strncpy(ae_dtsi_info.file_path, "/mnt/app/isp/isp.dtb", DTSI_NAME_LENGTH);
			strncpy(awb_dtsi_info.file_path, "/mnt/app/isp/isp.dtb", DTSI_NAME_LENGTH);
			strncpy(iq_dtsi_info.file_path, "/mnt/app/isp/isp.dtb", DTSI_NAME_LENGTH);
			printf("Load /mnt/app/isp/isp.dtb\n");
			break;
		}
		vendor_isp_set_ae(AET_ITEM_RLD_DTSI, &ae_dtsi_info);
		vendor_isp_set_awb(AWBT_ITEM_RLD_DTSI, &awb_dtsi_info);
		vendor_isp_set_iq(IQT_ITEM_RLD_DTSI, &iq_dtsi_info);

		#else
		cfg_info.id = 0;

		switch(RESOLUTION_SET) {
		default:
		case 0:
			if (g_shdr == 1) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_imx290_0_hdr.cfg", CFG_NAME_LENGTH);
				printf("Load /mnt/app/isp/isp_imx290_0_hdr.cfg\n");
			} else {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_imx290_0.cfg", CFG_NAME_LENGTH);
				printf("Load /mnt/app/isp/isp_imx290_0.cfg\n");
			}
			break;
		case 5:
			strncpy(cfg_info.path, "/mnt/app/isp/isp_f37.cfg", CFG_NAME_LENGTH);
			printf("Load /mnt/app/isp/isp_f37.cfg\n");
			break;
		case 6:
			strncpy(cfg_info.path, "/mnt/app/isp/isp_f35.cfg", CFG_NAME_LENGTH);
			printf("Load /mnt/app/isp/isp_f35.cfg\n");
			break;
		case 7:
			strncpy(cfg_info.path, "/mnt/app/isp/isp_gc4663_0_52x.cfg", CFG_NAME_LENGTH);
			printf("Load /mnt/app/isp/isp_gc4663_0_528.cfg\n");
			break;
		case 9:
			strncpy(cfg_info.path, "/mnt/app/isp/isp_tc358840_0.cfg", CFG_NAME_LENGTH);
			printf("Load /mnt/app/isp/isp_tc358840_0.cfg\n");
			break;
		case 10:
			strncpy(cfg_info.path, "/mnt/app/isp/isp_tp2855_0.cfg", CFG_NAME_LENGTH);
			printf("Load /mnt/app/isp/isp_tp2855_0.cfg\n");
			break;
		}
		vendor_isp_set_ae(AET_ITEM_RLD_CONFIG, &cfg_info);
		vendor_isp_set_awb(AWBT_ITEM_RLD_CONFIG, &cfg_info);
		vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);
		#endif
		vendor_isp_uninit();
	} else {
		printf("Init vendor isp fail. \n");
	}
	#endif
	#endif

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

	// memory hotplug
	ret = memory_hotplug();
	if (ret != HD_OK) {
		printf("memory hotplug fail=%d\n", ret);
		//goto exit;
	}

	// init all modules
	ret = init_module();
	if (ret != HD_OK) {
		printf("init fail=%d\n", ret);
		goto exit;
	}

	// open video_record modules (main)
	stream[0].proc_max_dim.w = VDO_SIZE_W; //assign by user
	stream[0].proc_max_dim.h = VDO_SIZE_H; //assign by user
	if (g_acapen) {
		stream[0].acap_sr_max = HD_AUDIO_SR_48000;
		open_audio_module(&stream[0]);
		if (ret != HD_OK) {
			printf("open audio fail=%d\n", ret);
			goto exit;
		}
	}
	if (sensor1_enabled()) {
		ret = open_module(&stream[0], &stream[0].proc_max_dim);
		if (ret != HD_OK) {
			printf("open fail=%d\n", ret);
			goto exit;
		}
		if(g_osg){
			ret = init_osg(&(stream[0]));
			if (ret != HD_OK) {
				printf("fail to init stream[0] osd=%d\n", ret);
				goto exit;
			}
		}
		if (g_snapshoten) {
			// open video_record modules (snapshot)
			stream[1].proc_max_dim.w = VDO_SIZE_W; //assign by user
			stream[1].proc_max_dim.h = VDO_SIZE_H; //assign by user
			ret = open_module_2(&stream[1]);
			if (ret != HD_OK) {
				printf("open2 fail=%d\n", ret);
				goto exit;
			}
			if(g_osg){
				ret = init_osg(&(stream[1]));
				if (ret != HD_OK) {
					printf("fail to init stream[1] osd=%d\n", ret);
					goto exit;
				}
			}
		}
		if (g_suben) {
			// open video_record modules (sub)
			stream[2].proc_max_dim.w = VDO_SIZE_W; //assign by user
			stream[2].proc_max_dim.h = VDO_SIZE_H; //assign by user
			ret = open_module_3(&stream[2]);
			if (ret != HD_OK) {
				printf("open3 fail=%d\n", ret);
				goto exit;
			}
		}
	}

	if (sensor2_enabled()) {
		// open video liview modules (sensor 2nd)
		stream[3].proc_max_dim.w = VDO_SIZE_W; //assign by user
		stream[3].proc_max_dim.h = VDO_SIZE_H; //assign by user
		ret = open_module_4(&stream[3], &stream[3].proc_max_dim);
		if (ret != HD_OK) {
			printf("open4 fail=%d\n", ret);
			goto exit;
		}

		if (g_mainen2) {
			// open video_record modules with sensor #2 (main)
			ret = open_module_5(&stream[4]);
			if (ret != HD_OK) {
				printf("open5 fail=%d\n", ret);
				goto exit;
			}
		}
		if (g_snapshoten2) {
			// open video_record modules with sensor #2 (snapshot)
			ret = open_module_6(&stream[5]);
			if (ret != HD_OK) {
				printf("open6 fail=%d\n", ret);
				goto exit;
			}
		}
		if (g_suben2) {
			// open video_record modules with sensor #2 (sub)
			stream[6].proc_max_dim.w = VDO_SIZE_W; //assign by user
			stream[6].proc_max_dim.h = VDO_SIZE_H; //assign by user
			ret = open_module_7(&stream[6]);
			if (ret != HD_OK) {
				printf("open7 fail=%d\n", ret);
				goto exit;
			}
		}
	}

	if (sensor1_enabled()) {
		// get videocap capability
		ret = get_cap_caps(stream[0].cap_ctrl, &stream[0].cap_syscaps);
		if (ret != HD_OK) {
			printf("get cap-caps fail=%d\n", ret);
			goto exit;
		}

		// set videocap parameter
		stream[0].cap_dim.w = VDO_SIZE_W; //assign by user
		stream[0].cap_dim.h = VDO_SIZE_H; //assign by user
		ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim, 0);
		if (ret != HD_OK) {
			printf("set cap fail=%d\n", ret);
			goto exit;
		}

		// assign parameter by program options
		main_dim.w = VDO_SIZE_W;
		main_dim.h = VDO_SIZE_H;
		sub_dim.w = SUB_VDO_SIZE_W;
		sub_dim.h = SUB_VDO_SIZE_H;
		// set videoproc parameter (main)
		if (g_prcbind == 0xff) { //no-bind mode
			if (g_yuv_que) {
				ret = set_proc_param(stream[0].proc_path, &main_dim, FAST_BOOT_DEPTH(g_yuv_que, DEPTH_NUM)); //must set vprc's out dim
			} else {
				ret = set_proc_param(stream[0].proc_path, &main_dim, 1); //must set vprc's out dim
			}
		} else {
			ret = set_proc_param(stream[0].proc_path, NULL, 0); //keep vprc's out dim = {0,0}, it means auto sync from venc's in dim
		}
		if (ret != HD_OK) {
			printf("set proc fail=%d\n", ret);
			goto exit;
		}

		// set ipp mask
		if(g_prc_mask) {
			int i;
			int n = sizeof(prc_mask)/sizeof(prc_mask[0]);
			for(i = 0 ; i < n ; i++) {
				set_mask_param(stream[0].mask_path[i], i);
			}
			n = sizeof(prc_vp_mask)/sizeof(prc_vp_mask[0]);
			for(i = 0 ; i < n ; i++) {
				set_vp_mask_param(stream[0].vp_mask_path[i], i);
			}
		}

		// set videoenc config (main)
		if (sensor_en_map == 0) {
			stream[0].enc_max_dim.w = main_dim.w;
			stream[0].enc_max_dim.h = main_dim.h;
		} else {
			stream[0].enc_max_dim.w = g_crop_w;
			stream[0].enc_max_dim.h = g_crop_h;
		}
		ret = set_enc_cfg(stream[0].enc_path, &stream[0].enc_max_dim, MAIN_BR, HD_VIDEOPROC_0_CTRL);
		if (ret != HD_OK) {
			printf("set enc-cfg fail=%d\n", ret);
			goto exit;
		}

		// set videoenc parameter (main)
		if (sensor_en_map == 0) {
			stream[0].enc_dim.w = main_dim.w;
			stream[0].enc_dim.h = main_dim.h;
		} else {
			stream[0].enc_dim.w = g_crop_w;
			stream[0].enc_dim.h = g_crop_h;
		}
		ret = set_enc_param(stream[0].enc_path, &stream[0].enc_dim, enc_type, MAIN_BR, 0);
		if (ret != HD_OK) {
			printf("set enc fail=%d\n", ret);
			goto exit;
		}

		if (g_snapshoten) {
			// set videoenc config (sub)
			if (sensor_en_map == 0) {
				stream[1].enc_max_dim.w = main_dim.w;
				stream[1].enc_max_dim.h = main_dim.h;
			} else {
				stream[1].enc_max_dim.w = g_crop_w;
				stream[1].enc_max_dim.h = g_crop_h;
			}
			ret = set_enc_cfg(stream[1].enc_path, &stream[1].enc_max_dim, SUB_BR, HD_VIDEOPROC_0_CTRL);
			if (ret != HD_OK) {
				printf("set enc-cfg fail=%d\n", ret);
				goto exit;
			}

			// set videoenc parameter (sub)
			snapshot_enc_type = 2;
			if (sensor_en_map == 0) {
				stream[1].enc_dim.w = main_dim.w;
				stream[1].enc_dim.h = main_dim.h;
			} else {
				stream[1].enc_dim.w = g_crop_w;
				stream[1].enc_dim.h = g_crop_h;
			}
			ret = set_enc_param(stream[1].enc_path, &stream[1].enc_dim, snapshot_enc_type, SUB_BR, 1);
			if (ret != HD_OK) {
				printf("set enc fail=%d\n", ret);
				goto exit;
			}
		}

		if (g_suben) {
			// set videoproc parameter (sub)
			ret = set_proc_param(stream[2].proc_path, NULL, 0); //keep vprc's out dim = {0,0}, it means auto sync from venc's in dim
			if (ret != HD_OK) {
				printf("set proc fail=%d\n", ret);
				goto exit;
			}

			// set videoenc config (sub)
			stream[2].enc_max_dim.w = sub_dim.w;
			stream[2].enc_max_dim.h = sub_dim.h;
			ret = set_enc_cfg(stream[2].enc_path, &stream[2].enc_max_dim, SUB_BR, HD_VIDEOPROC_0_CTRL);
			if (ret != HD_OK) {
				printf("set enc-cfg fail=%d\n", ret);
				goto exit;
			}

			// set videoenc parameter (sub)
			stream[2].enc_dim.w = sub_dim.w;
			stream[2].enc_dim.h = sub_dim.h;
			ret = set_enc_param(stream[2].enc_path, &stream[2].enc_dim, sub_enc_type, SUB_BR, 0);
			if (ret != HD_OK) {
				printf("set enc fail=%d\n", ret);
				goto exit;
			}
		}
	}

	if (sensor2_enabled()) {
		// get videocap capability (sensor 2nd)
		ret = get_cap_caps(stream[3].cap_ctrl, &stream[3].cap_syscaps);

		// set videocap parameter (sensor 2nd)
		stream[3].cap_dim.w = VDO_SIZE_W; //assign by user
		stream[3].cap_dim.h = VDO_SIZE_H; //assign by user
		ret = set_cap_param(stream[3].cap_path, &stream[3].cap_dim, 0);
		if (ret != HD_OK) {
			printf("set cap2 fail=%d\n", ret);
			goto exit;
		}

    		// assign parameter by program options
	    	main_dim.w = VDO_SIZE_W;
    		main_dim.h = VDO_SIZE_H;
		sub_dim.w = SUB_VDO_SIZE_W;
		sub_dim.h = SUB_VDO_SIZE_H;
    		// set videoproc parameter (main)
			if (g_prcbind == 0xff) { //no-bind mode
				if (g_yuv_que) {
					ret = set_proc_param(stream[3].proc_path, &main_dim, FAST_BOOT_DEPTH(g_yuv_que, DEPTH_NUM)); //must set vprc's out dim
				} else {
					ret = set_proc_param(stream[3].proc_path, &main_dim, 1); //must set vprc's out dim
				}
			} else {
    			ret = set_proc_param(stream[3].proc_path, NULL, 0); //keep vprc's out dim = {0,0}, it means auto sync from venc's in dim
			}
    		if (ret != HD_OK) {
    			printf("set proc fail=%d\n", ret);
    			goto exit;
    		}

		if (g_mainen2) {
			// set videoenc config with sensor #2 (main)
			if (sensor_en_map == 0) {
				stream[4].enc_max_dim.w = main_dim.w;
				stream[4].enc_max_dim.h = main_dim.h;
			} else {
				stream[4].enc_max_dim.w = g_crop_w;
				stream[4].enc_max_dim.h = g_crop_h;
			}
			ret = set_enc_cfg(stream[4].enc_path, &stream[4].enc_max_dim, SUB_BR, HD_VIDEOPROC_1_CTRL);
			if (ret != HD_OK) {
				printf("set enc-cfg fail=%d\n", ret);
				goto exit;
			}
			// set videoenc parameter with sensor #2 (main)
			if (sensor_en_map == 0) {
				stream[4].enc_dim.w = main_dim.w;
				stream[4].enc_dim.h = main_dim.h;
			} else {
				stream[4].enc_dim.w = g_crop_w;
				stream[4].enc_dim.h = g_crop_h;
			}
			ret = set_enc_param(stream[4].enc_path, &stream[4].enc_dim, main2_enc_type, SUB_BR, 0);
			if (ret != HD_OK) {
				printf("set enc fail=%d\n", ret);
				goto exit;
			}
		}
		if (g_snapshoten2) {
			// set videoenc config with sensor #2 (snapshot)
			if (sensor_en_map == 0) {
				stream[5].enc_max_dim.w = main_dim.w;
				stream[5].enc_max_dim.h = main_dim.h;
			} else {
				stream[5].enc_max_dim.w = g_crop_w;
				stream[5].enc_max_dim.h = g_crop_h;
			}
			ret = set_enc_cfg(stream[5].enc_path, &stream[5].enc_max_dim, SUB_BR, HD_VIDEOPROC_1_CTRL);
			if (ret != HD_OK) {
				printf("set enc-cfg fail=%d\n", ret);
				goto exit;
			}
			// set videoenc parameter with sensor #2 (snapshot)
			snapshot_enc_type = 2;
			if (sensor_en_map == 0) {
				stream[5].enc_dim.w = main_dim.w;
				stream[5].enc_dim.h = main_dim.h;
			} else {
				stream[5].enc_dim.w = g_crop_w;
				stream[5].enc_dim.h = g_crop_h;
			}
			ret = set_enc_param(stream[5].enc_path, &stream[5].enc_dim, snapshot_enc_type, SUB_BR, 1);
			if (ret != HD_OK) {
				printf("set enc fail=%d\n", ret);
				goto exit;
			}
		}
		if (g_suben2) {
			ret = set_proc_param(stream[6].proc_path, NULL, 0); //keep vprc's out dim = {0,0}, it means auto sync from venc's in dim
			if (ret != HD_OK) {
				printf("set proc fail=%d\n", ret);
				goto exit;
			}

			// set videoenc config with sensor #2 (sub)
			stream[6].enc_max_dim.w = sub_dim.w;
			stream[6].enc_max_dim.h = sub_dim.h;
			ret = set_enc_cfg(stream[6].enc_path, &stream[6].enc_max_dim, SUB_BR, HD_VIDEOPROC_1_CTRL);
			if (ret != HD_OK) {
				printf("set enc-cfg fail=%d\n", ret);
				goto exit;
			}
			// set videoenc parameter with sensor #2 (sub)
			stream[6].enc_dim.w = sub_dim.w;
			stream[6].enc_dim.h = sub_dim.h;
			ret = set_enc_param(stream[6].enc_path, &stream[6].enc_dim, sub2_enc_type, SUB_BR, 0);
			if (ret != HD_OK) {
				printf("set enc fail=%d\n", ret);
				goto exit;
			}
		}
	}

	if (g_acapen) {
		stream[0].acap_sr = HD_AUDIO_SR_16000;
		ret = set_acap_param(stream[0].acap_path, stream[0].acap_sr);
		if (ret != HD_OK) {
			printf("set acap fail=%d\n", ret);
			goto exit;
		}
#if (AOUT_PLAYBACK==ENABLE)
		ret = set_aout_param(stream[0].aout_ctrl, stream[0].aout_path, stream[0].acap_sr);
		if (ret != HD_OK) {
			printf("set aout fail=%d\n", ret);
			goto exit;
		}
#endif
	}

	if (sensor1_enabled()) {
		if (g_capbind == 0xff) { //no-bind mode
			// create capture_thread (pull_out frame, push_in frame then pull_out frame)
			ret = pthread_create(&stream[0].cap_thread_id, NULL, capture_thread, (void *)stream);
			if (ret < 0) {
				printf("create capture_thread failed");
				goto exit;
			}
		} else {
			// bind video_record modules (main)
			hd_videocap_bind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);
		}
		if (g_prcbind == 0xff) { //no-bind mode
			// create process_thread (pull_out frame, push_in frame then pull_out bitstream)
			ret = pthread_create(&stream[0].prc_thread_id, NULL, process_thread, (void *)stream);
			if (ret < 0) {
				printf("create process_thread failed");
				goto exit;
			}
		} else {
			// bind video_record modules (main)
			hd_videoproc_bind(HD_VIDEOPROC_OUT(0, g_prcout), HD_VIDEOENC_0_IN_0);
		}
		if (g_suben) {
			// bind video_record modules (sub)
			hd_videoproc_bind(HD_VIDEOPROC_0_OUT_1, HD_VIDEOENC_0_IN_2);
		}

#if (VOUT_DISP==ENABLE)
		// get videoout capability
		ret = get_out_caps(stream[0].out_ctrl, &stream[0].out_syscaps);
		if (ret != HD_OK) {
			printf("get out-caps fail=%d\n", ret);
			goto exit;
		}
		stream[0].out_max_dim = stream[0].out_syscaps.output_dim;

		// set videoout parameter (main)
		stream[0].out_dim.w = stream[0].out_max_dim.w; //using device max dim.w
		stream[0].out_dim.h = stream[0].out_max_dim.h; //using device max dim.h
		ret = set_out_param(stream[0].out_path, &stream[0].out_dim);
		if (ret != HD_OK) {
			printf("set out fail=%d\n", ret);
			goto exit;
		}
		hd_videoproc_bind(HD_VIDEOPROC_0_OUT_2, HD_VIDEOOUT_0_IN_0);
#endif
	}

	if (sensor2_enabled()) {
		hd_videocap_bind(HD_VIDEOCAP_OUT(SEN2_VCAP_ID, 0), HD_VIDEOPROC_1_IN_0);
		if (g_mainen2) {
			if (g_prcbind == 0xff) { //no-bind mode
				// create process_thread (pull_out frame, push_in frame then pull_out bitstream)
				ret = pthread_create(&stream[3].prc_thread_id, NULL, process_thread2, (void *)stream);
				if (ret < 0) {
					printf("create process_thread2 failed");
					goto exit;
				}
			} else {
				// bind video_record modules (main2)
				hd_videoproc_bind(HD_VIDEOPROC_1_OUT_0, HD_VIDEOENC_0_IN_3);
			}
		}
		if (g_suben2) {
			hd_videoproc_bind(HD_VIDEOPROC_1_OUT_1, HD_VIDEOENC_0_IN_5);
		}
	}

	// create encode_thread (pull_out bitstream)
	ret = pthread_create(&stream[0].enc_thread_id, NULL, encode_thread, (void *)stream);
	if (ret < 0) {
		printf("create encode_thread failed");
		goto exit;
	}

	if (sensor1_enabled()) {

		// ad detection
		if (RESOLUTION_SET == 9 || RESOLUTION_SET == 10) {
			BOOL plugged = FALSE;
			UINT32 i = 0;

			while (i < 10000) {
				if (vendor_videocap_get(stream[0].cap_ctrl, VENDOR_VIDEOCAP_PARAM_GET_PLUG, &plugged) == HD_OK && plugged != FALSE) {
					printf("plug in (%ums)\r\n", i);
					break;
				}
				usleep(100*1000);
				i += 100;
			}
			if (i >= 10000) {
				printf("detect timeout (10000ms)\r\n");
			}
		}

		hd_videoenc_start(stream[0].enc_path);
		if (g_snapshoten) {
			hd_videoenc_start(stream[1].enc_path);
		}
		if (g_suben) {
			hd_videoenc_start(stream[2].enc_path);
		}

		// start video_record modules (main)
		if (g_capbind == 1) {
			//direct NOTE: ensure videocap start after 1st videoproc phy path start
			hd_videoproc_start(stream[0].proc_path);
			hd_videocap_start(stream[0].cap_path);
		} else {
			hd_videocap_start(stream[0].cap_path);
			hd_videoproc_start(stream[0].proc_path);
		}
#if (VOUT_DISP==ENABLE)
		hd_videoproc_start(stream[0].proc_path2);
		hd_videoout_start(stream[0].out_path);
#endif
		// just wait ae/awb stable for auto-test, if don't care, user can remove it
		//sleep(1);
		if (g_suben) {
			hd_videoproc_start(stream[2].proc_path);
		}
	}

	if (sensor2_enabled()) {
		if (g_mainen2) {
			hd_videoenc_start(stream[4].enc_path);
		}
		if (g_snapshoten2) {
			hd_videoenc_start(stream[5].enc_path);
		}
		if (g_suben2) {
			hd_videoenc_start(stream[6].enc_path);
		}

		hd_videocap_start(stream[3].cap_path);
		hd_videoproc_start(stream[3].proc_path);

		if (g_suben2) {
			hd_videoproc_start(stream[6].proc_path);
		}
	}

	if (g_acapen) {
		// create encode_thread (pull_out bitstream)
		ret = pthread_create(&stream[0].acap_thread_id, NULL, acapture_thread, (void *)stream);
		if (ret < 0) {
			printf("create encode_thread failed");
			goto exit;
		}
		hd_audiocap_start(stream[0].acap_path);

		// start acapture_thread
		stream[0].acap_enter = 1;

#if (AOUT_PLAYBACK==ENABLE)
		ret = pthread_create(&stream[0].aout_thread_id, NULL, aplayback_thread, (void *)stream);
		if (ret < 0) {
			printf("create aplayback_thread failed");
			goto exit;
		}
		hd_audioout_start(stream[0].aout_path);

		// start aplayback_thread
		stream[0].aout_enter = 1;
#endif
	}

	if(g_prc_mask) {
		int i;
		int n = sizeof(prc_vp_mask)/sizeof(prc_vp_mask[0]);
		for(i = 0 ; i < n ; i++) {
			//render vp mask
			ret = hd_videoproc_start(stream[0].vp_mask_path[i]);
			if (ret != HD_OK) {
				printf("fail to start vp mask\n");
				goto exit;
			}
		}
	}

	if (sensor1_enabled()) {
		if (g_capbind == 0xff) { //no-bind mode
			// start capture_thread
			stream[0].cap_enter = 1;
		}
		if (g_prcbind == 0xff) { //no-bind mode
			// start process_thread
			stream[0].prc_enter = 1;
			stream[3].prc_enter = 1;
		}
	}
	// start encode_thread
	stream[0].enc_enter = 1;

	// query user key
	printf("Enter q to exit\n");
	while (1) {
		key = GETCHAR();
		if (key == 'q' || key == 0x3) {
			// quit program
			break;
		}

		if (key == 'a') {
			hd_videoproc_stop(stream[0].proc_path);
			hd_videoproc_start(stream[0].proc_path);
		}

		if (key == 'b') {
			//for vprc unbind,stop venc for dump dtsi
			stream[0].enc_exit = 1;
			stream[4].enc_exit = 1;
			usleep(33000);
			if (sensor1_enabled()) {
				hd_videoenc_stop(stream[0].enc_path);
			}
			if (sensor2_enabled()) {
				if (g_mainen2) {
					hd_videoenc_stop(stream[4].enc_path);
				}
			}
		}

		if (key == 'i') {
			printf("dumping fastboot dtsi to /tmp \r\n");
			system("mkdir -p /tmp/fastboot");
			system("echo dump_fastboot  /tmp/fastboot/nvt-fastboot-sie.dtsi > /proc/kflow_sie/cmd");
			system("echo dump_fastboot /tmp/fastboot/nvt-fastboot-ipp.dtsi > /proc/kflow_ipp/cmd");
			system("echo vdoenc dump_fastboot /tmp/fastboot/nvt-fastboot-venc.dtsi > /proc/hdal/venc/cmd");
			system("echo dump_fastboot /tmp/fastboot/nvt-fastboot-acap.dtsi > /proc/hdal/acap/cmd");
			// to solve 'Command length is too long!'
			system("echo nvtmpp fastboot_mem /tmp/hdal-mem.dtsi > /proc/hdal/comm/cmd");
			system("mv /tmp/hdal-mem.dtsi /tmp/fastboot/nvt-fastboot-hdal-mem.dtsi");
			printf("finish. \r\n");
		}

		if (key == 's') {
			char *embmem = getenv("EMBMEM");
			printf("dumping & saving fastboot dtsi to flash\r\n");
			system("cp /etc/fastboot /tmp -rf");
			system("rm /tmp/fastboot/nvt-fastboot-sie.dtsi");
			system("rm /tmp/fastboot/nvt-fastboot-ipp.dtsi");
			system("rm /tmp/fastboot/nvt-fastboot-venc.dtsi");
			system("rm /tmp/fastboot/nvt-fastboot-acap.dtsi");
			system("rm /tmp/fastboot/nvt-fastboot-hdal-mem.dtsi");
			system("echo dump_fastboot  /tmp/fastboot/nvt-fastboot-sie.dtsi > /proc/kflow_sie/cmd");
			system("echo dump_fastboot /tmp/fastboot/nvt-fastboot-ipp.dtsi > /proc/kflow_ipp/cmd");
			system("echo vdoenc dump_fastboot /tmp/fastboot/nvt-fastboot-venc.dtsi > /proc/hdal/venc/cmd");
			system("echo dump_fastboot /tmp/fastboot/nvt-fastboot-acap.dtsi > /proc/hdal/acap/cmd");
			// to solve 'Command length is too long!'
			system("echo nvtmpp fastboot_mem /tmp/hdal-mem.dtsi > /proc/hdal/comm/cmd");
			system("mv /tmp/hdal-mem.dtsi /tmp/fastboot/nvt-fastboot-hdal-mem.dtsi");
			system("cd /tmp/fastboot && sh build.sh");
			if (embmem) {
				if (strncmp(embmem, "EMBMEM_SPI_NAND", 15) == 0 ||
					strncmp(embmem, "EMBMEM_SPI_NOR", 14) == 0) {
					char cmd[] = \
						"mtd=`cat /proc/mtd | grep \"fdt.fast\" | awk -F':' '{printf $1}'`;" \
						"flashcp -v /tmp/fastboot/nvt-fastboot.dtb /dev/$mtd";
					system(cmd);
				} else if(strncmp(embmem, "EMBMEM_EMMC", 11) == 0) {
					printf("not implemented, write to emmc\n");
				}
			}
			printf("finish. \r\n");
		}

		#if (DEBUG_MENU == 1)
		if (key == 'd') {
			// enter debug menu
			hd_debug_run_menu();
			printf("\r\nEnter q to exit, Enter d to debug\r\n");
		}
		#endif
	}

	if (sensor1_enabled()) {
		if (g_capbind == 0xff) { //no-bind mode
			// stop capture_thread
			stream[0].cap_exit = 1;
		}
		if (g_prcbind == 0xff) { //no-bind mode
			// stop process_thread
			stream[0].prc_exit = 1;
		}
	}
	// stop encode_thread
	stream[0].enc_exit = 1;
	stream[4].enc_exit = 1;

	if (g_acapen) {
		// start acapture_thread
		stream[0].acap_exit = 1;
#if (AOUT_PLAYBACK==ENABLE)
		stream[0].aout_exit = 1;
#endif
	}

	if (sensor1_enabled()) {
#if (VOUT_DISP==ENABLE)
		hd_videoproc_stop(stream[0].proc_path2);
		hd_videoout_stop(stream[0].out_path);
#endif

		if (g_suben) {
			hd_videoproc_stop(stream[2].proc_path);
			hd_videoenc_stop(stream[2].enc_path);
		}

		// stop video_record modules (main)
		if (g_capbind == 1) {
			//direct NOTE: ensure videocap stop after all videoproc path stop
			hd_videoproc_stop(stream[0].proc_path);
			hd_videocap_stop(stream[0].cap_path);
		} else {
			hd_videocap_stop(stream[0].cap_path);
			hd_videoproc_stop(stream[0].proc_path);
		}

		hd_videoenc_stop(stream[0].enc_path);
		if (g_snapshoten) {
			hd_videoenc_stop(stream[1].enc_path);
		}
	}

	if (sensor2_enabled()) {
		hd_videocap_stop(stream[3].cap_path);
		hd_videoproc_stop(stream[3].proc_path);
#if 1
		if (g_mainen2) {
			hd_videoenc_stop(stream[4].enc_path);
		}
		if (g_snapshoten2) {
			hd_videoenc_stop(stream[5].enc_path);
		}
		if (g_suben2) {
			hd_videoproc_stop(stream[6].proc_path);
			hd_videoenc_stop(stream[6].enc_path);
		}
#endif
	}

	if (g_acapen) {
		hd_audiocap_stop(stream[0].acap_path);
#if (AOUT_PLAYBACK==ENABLE)
		// wait aplayback
		while (stream[0].aout_enter == 1) usleep(100);
		hd_audioout_stop(stream[0].aout_path);
#endif
	}

	// destroy encode_thread
	pthread_join(stream[0].enc_thread_id, NULL); //NOTE: before destory, call stop to breaking pull(-1)

	if (sensor1_enabled()) {
		if (g_capbind == 0xff) { //no-bind mode
			// destroy capture_thread
			pthread_join(stream[0].cap_thread_id, NULL);  //NOTE: before destory, call stop to breaking pull(-1)
		} else {
			// unbind video_record modules (main)
			hd_videocap_unbind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0));
		}
		if (g_prcbind == 0xff) { //no-bind mode
			// destroy process_thread
			pthread_join(stream[0].prc_thread_id, NULL);  //NOTE: before destory, call stop to breaking pull(-1)
		} else {
			// unbind video_record modules (main)
			hd_videoproc_unbind(HD_VIDEOPROC_OUT(0, g_prcout));
		}
		if (g_suben) {
			// unbind video_record modules (sub)
			hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_1);
		}
#if (VOUT_DISP==ENABLE)
		hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_2);
#endif
	}

	if (sensor2_enabled()) {
		// unbind video_liveview modules (sensor 2nd)
		hd_videocap_unbind(HD_VIDEOCAP_OUT(SEN2_VCAP_ID, 0));
		if (g_mainen2) {
			hd_videoproc_unbind(HD_VIDEOPROC_1_OUT_0);
		}
		if (g_suben2) {
			hd_videoproc_unbind(HD_VIDEOPROC_1_OUT_1);
		}
	}

	if (g_acapen) {
		pthread_join(stream[0].acap_thread_id, NULL);
#if (AOUT_PLAYBACK==ENABLE)
		pthread_join(stream[0].aout_thread_id, NULL);
#endif
	}

exit:

	// close video_record modules (main)
	if (sensor1_enabled()) {
		ret = close_module(&stream[0]);
		if (ret != HD_OK) {
			printf("close fail=%d\n", ret);
		}

		// close video_record modules (snapshot)
		if (g_snapshoten) {
			ret = close_module_2(&stream[1]);
			if (ret != HD_OK) {
				printf("close2 fail=%d\n", ret);
			}
		}

		// close video_record modules (sub)
		if (g_suben) {
			ret = close_module_3(&stream[2]);
			if (ret != HD_OK) {
				printf("close3 fail=%d\n", ret);
			}
		}
	}

	if (g_acapen) {
		ret = close_audio_module(&stream[0]);
		if (ret != HD_OK) {
			printf("close audio fail=%d\n", ret);
		}
	}


	if (sensor2_enabled()) {
		ret = close_module_4(&stream[3]);
		if (ret != HD_OK) {
			printf("close2 fail=%d\n", ret);
		}

		if (g_mainen2) {
			close_module_5(&stream[4]);
		}
		if (g_snapshoten2) {
			close_module_6(&stream[5]);
		}
		if (g_suben2) {
			close_module_7(&stream[6]);
		}
	}

	// uninit all modules
	ret = exit_module();
	if (ret != HD_OK) {
		printf("exit fail=%d\n", ret);
	}

	if(g_osg){
		if (sensor1_enabled()) {
			if(stream[0].stamp_blk){
				if(HD_OK != hd_common_mem_release_block(stream[0].stamp_blk)){
					printf("hd_common_mem_release_block() fail\n");
				}
			}

			if (g_snapshoten) {
				if(stream[1].stamp_blk){
					if(HD_OK != hd_common_mem_release_block(stream[1].stamp_blk)){
						printf("hd_common_mem_release_block() fail\n");
					}
				}
			}
		}
	}

	// uninit memory
	ret = mem_exit();
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
