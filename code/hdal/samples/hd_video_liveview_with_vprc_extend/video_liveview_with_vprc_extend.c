/**
	@brief Sample code of video liveview with extend out.\n

	@file video_liveview_with_vprc_extend.c

	@author Jeah Yen

	@ingroup mhdal

	@note This file is modified from video_record_with_substream.c.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "hdal.h"
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
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(hd_video_liveview_with_vprc_extend, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define DEBUG_MENU 		1

#define CHKPNT			printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
#define DBGH(x)			printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x)			printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)
#define HD_VIDEOPROC_CFG				0x000f0000	//vprc
#define HD_VIDEOPROC_CFG_STRIP_MASK		0x00000007  //vprc stripe rule mask: (default 0)
#define HD_VIDEOPROC_CFG_STRIP_LV1		0x00000000  //vprc "0: GDC,low latency balanced
#define HD_VIDEOPROC_CFG_STRIP_LV2		0x00010000  //vprc "1: low latency Best
#define HD_VIDEOPROC_CFG_STRIP_LV3		0x00020000  //vprc "2: 2D_LUT Best if 2D_LUT supported
#define HD_VIDEOPROC_CFG_STRIP_LV4		0x00030000  //vprc "3: GDC Best,low lantency don't case #define HD_VIDEOPROC_CFG_DISABLE_GDC	HD_VIDEOPROC_CFG_STRIP_LV2
#define HD_VIDEOPROC_CFG_DISABLE_GDC    HD_VIDEOPROC_CFG_STRIP_LV2
#define HD_VIDEOPROC_CFG_LL_FAST		HD_VIDEOPROC_CFG_STRIP_LV2
#define HD_VIDEOPROC_CFG_2DLUT_BEST		HD_VIDEOPROC_CFG_STRIP_LV3
#define HD_VIDEOPROC_CFG_GDC_BEST		HD_VIDEOPROC_CFG_STRIP_LV4

#define SEN1_VCAP_ID 0

///////////////////////////////////////////////////////////////////////////////

#define VOUT_IDE1  0x1
#define VOUT_IDE2  0x2

static UINT32 g_ide = VOUT_IDE1;
static UINT32 vout_fmt = HD_VIDEO_PXLFMT_YUV420;
static UINT32 vprc_fmt = HD_VIDEO_PXLFMT_YUV420;
static UINT32 vprc_ext_fmt = HD_VIDEO_PXLFMT_YUV420;
static UINT32 g_crop_x = 100;
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

#define SEN_OUT_FMT		HD_VIDEO_PXLFMT_RAW12
#define CAP_OUT_FMT		HD_VIDEO_PXLFMT_RAW12
#define CA_WIN_NUM_W		32
#define CA_WIN_NUM_H		32
#define LA_WIN_NUM_W		32
#define LA_WIN_NUM_H		32
#define VA_WIN_NUM_W		16
#define VA_WIN_NUM_H		16
#define YOUT_WIN_NUM_W	128
#define YOUT_WIN_NUM_H	128
#define ETH_8BIT_SEL		0 //0: 2bit out, 1:8 bit out
#define ETH_OUT_SEL		1 //0: full, 1: subsample 1/2

#define VDO_SIZE_W		1920
#define VDO_SIZE_H		1080

#define SUB_VDO_SIZE_W	1920 //max for videoout
#define SUB_VDO_SIZE_H	1080 //max for videoout

#define SOURCE_PATH		HD_VIDEOPROC_0_OUT_0 //out 0~4 is physical path
#define SOURCE_PATH1	HD_VIDEOPROC_0_OUT_1 //out 0~4 is physical path
#define EXTEND_PATH		HD_VIDEOPROC_0_OUT_6 //out 5~15 is extend path

///////////////////////////////////////////////////////////////////////////////


static HD_RESULT mem_init(void)
{
	HD_RESULT              ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

	// config common pool (cap)
	mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[0].blk_size = DBGINFO_BUFSIZE()+VDO_RAW_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, CAP_OUT_FMT)
        													+VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
        													+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
	mem_cfg.pool_info[0].blk_cnt = 3;
	mem_cfg.pool_info[0].ddr_id = DDR_ID0;
	// config common pool (main)
	mem_cfg.pool_info[1].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[1].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(ALIGN_CEIL_16(VDO_SIZE_W), ALIGN_CEIL_16(VDO_SIZE_H), vprc_fmt);  // align to 16 for rotate buffer
	mem_cfg.pool_info[1].blk_cnt = 3;
	mem_cfg.pool_info[1].ddr_id = DDR_ID0;
	// config common pool (sub)
	mem_cfg.pool_info[2].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[2].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(ALIGN_CEIL_16(SUB_VDO_SIZE_W), ALIGN_CEIL_16(SUB_VDO_SIZE_H), vprc_ext_fmt);  // align to 16 for rotate buffer
	mem_cfg.pool_info[2].blk_cnt = 3;
	mem_cfg.pool_info[2].ddr_id = DDR_ID0;

	ret = hd_common_mem_init(&mem_cfg);
	return ret;
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

	snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os02k10");
	cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux =  0;  //use @0 in peri-dev.dtsi
	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(0, 0);
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = HD_VIDEOCAP_SEN_IGNORE;
	ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(SEN1_VCAP_ID), &video_cap_ctrl); //open this for device control
	if (ret != HD_OK) {
		return ret;
	}
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
	iq_ctl.func = HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB;
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);

	*p_video_cap_ctrl = video_cap_ctrl;
	return ret;
}

static HD_RESULT set_cap_param(HD_PATH_ID video_cap_path, HD_DIM *p_dim)
{
	HD_RESULT ret = HD_OK;
	{//select sensor mode, manually or automatically
		HD_VIDEOCAP_IN video_in_param = {0};

		video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO; //auto select sensor mode by the parameter of HD_VIDEOCAP_PARAM_OUT
		video_in_param.frc = HD_VIDEO_FRC_RATIO(30,1);
		video_in_param.dim.w = p_dim->w;
		video_in_param.dim.h = p_dim->h;
		video_in_param.pxlfmt = SEN_OUT_FMT;
		video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN, &video_in_param);
		//printf("set_cap_param MODE=%d\r\n", ret);
		if (ret != HD_OK) {
			return ret;
		}
	}
	#if 1 //no crop, full frame
	{
		HD_VIDEOCAP_CROP video_crop_param = {0};

		video_crop_param.mode = HD_CROP_OFF;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT_CROP, &video_crop_param);
		//printf("set_cap_param CROP NONE=%d\r\n", ret);
	}
	#else //HD_CROP_ON
	{
		HD_VIDEOCAP_CROP video_crop_param = {0};

		video_crop_param.mode = HD_CROP_ON;
		video_crop_param.win.rect.x = 0;
		video_crop_param.win.rect.y = 0;
		video_crop_param.win.rect.w = 1920/2;
		video_crop_param.win.rect.h= 1080/2;
		video_crop_param.align.w = 4;
		video_crop_param.align.h = 4;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT_CROP, &video_crop_param);
		//printf("set_cap_param CROP ON=%d\r\n", ret);
	}
	#endif
	{
		HD_VIDEOCAP_OUT video_out_param = {0};

		//without setting dim for no scaling, using original sensor out size
		video_out_param.pxlfmt = CAP_OUT_FMT;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT, &video_out_param);
		//printf("set_cap_param OUT=%d\r\n", ret);
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////

static HD_RESULT set_proc_cfg(HD_PATH_ID *p_video_proc_ctrl, HD_DIM* p_max_dim)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
	HD_VIDEOPROC_CTRL video_ctrl_param = {0};
	HD_PATH_ID video_proc_ctrl = 0;

	ret = hd_videoproc_open(0, HD_VIDEOPROC_0_CTRL, &video_proc_ctrl); //open this for device control
	if (ret != HD_OK)
		return ret;

	if (p_max_dim != NULL ) {
		video_cfg_param.pipe = HD_VIDEOPROC_PIPE_RAWALL;
		video_cfg_param.isp_id = SEN1_VCAP_ID;
		video_cfg_param.ctrl_max.func = 0;
		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;
		video_cfg_param.in_max.pxlfmt = CAP_OUT_FMT;
		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
		if (ret != HD_OK) {
			return HD_ERR_NG;
		}
	}

	video_ctrl_param.func = 0;
	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);

	*p_video_proc_ctrl = video_proc_ctrl;

	return ret;
}

static HD_RESULT set_proc_param(HD_PATH_ID video_proc_path, HD_DIM* p_dim, UINT32 dir)
{
	HD_RESULT ret = HD_OK;

	if (p_dim != NULL) { //if videoproc is already binding to dest module, not require to setting this!
		HD_VIDEOPROC_OUT video_out_param = {0};
		video_out_param.func = 0;
		video_out_param.dim.w = p_dim->w;
		video_out_param.dim.h = p_dim->h;
		video_out_param.pxlfmt = vprc_fmt;
		video_out_param.dir = dir;
		video_out_param.frc = HD_VIDEO_FRC_RATIO(1,1);
		video_out_param.depth = 1; //set 1 to allow pull
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	} else {
		HD_VIDEOPROC_OUT video_out_param = {0};
		video_out_param.func = 0;
		video_out_param.dim.w = 0;
		video_out_param.dim.h = 0;
		video_out_param.pxlfmt = 0;
		video_out_param.dir = dir;
		video_out_param.frc = HD_VIDEO_FRC_RATIO(1,1);
		video_out_param.depth = 1; //set 1 to allow pull
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	}

	return ret;
}

static HD_RESULT set_proc_param_extend(HD_PATH_ID video_proc_path, HD_PATH_ID src_path, HD_URECT* p_crop, HD_DIM* p_dim, UINT32 dir)
{
	HD_RESULT ret = HD_OK;

	if ((p_crop != NULL) && !((p_crop->x == 0)&&(p_crop->y == 0)&&(p_crop->w == 0)&&(p_crop->h == 0))) { //if videoproc is already binding to dest module, not require to setting this!
		HD_VIDEOPROC_CROP video_out_param = {0};
		video_out_param.mode = HD_CROP_ON;
		video_out_param.win.rect.x = p_crop->x;
		video_out_param.win.rect.y = p_crop->y;
		video_out_param.win.rect.w = p_crop->w;
		video_out_param.win.rect.h = p_crop->h;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT_EX_CROP, &video_out_param);
	} else {
		HD_VIDEOPROC_CROP video_out_param = {0};
		video_out_param.mode = HD_CROP_OFF;
		video_out_param.win.rect.x = 0;
		video_out_param.win.rect.y = 0;
		video_out_param.win.rect.w = 0;
		video_out_param.win.rect.h = 0;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT_EX_CROP, &video_out_param);
	}
	if (ret != HD_OK) {
		return ret;
	}

	if (p_dim != NULL) { //if videoproc is already binding to dest module, not require to setting this!
		HD_VIDEOPROC_OUT_EX video_out_param = {0};
		video_out_param.src_path = src_path;
		video_out_param.dim.w = p_dim->w;
		video_out_param.dim.h = p_dim->h;
		video_out_param.pxlfmt = vprc_ext_fmt;
		video_out_param.dir = dir;
		video_out_param.depth = 0; //set 1 to allow pull
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT_EX, &video_out_param);
	} else {
		HD_VIDEOPROC_OUT_EX video_out_param = {0};
		video_out_param.src_path = src_path;
		video_out_param.dim.w = 0; //auto reference to downstream's in dim.w
		video_out_param.dim.h = 0; //auto reference to downstream's in dim.h
		video_out_param.pxlfmt = 0; //auto reference to downstream's in pxlfmt
		video_out_param.dir = dir;
		video_out_param.depth = 0; //set 1 to allow pull
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT_EX, &video_out_param);
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////

static HD_RESULT set_out_cfg(HD_PATH_ID *p_video_out_ctrl, UINT32 out_type,HD_VIDEOOUT_HDMI_ID hdmi_id)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOOUT_MODE videoout_mode = {0};
	HD_PATH_ID video_out_ctrl = 0;

    if(g_ide&VOUT_IDE1) {
    	ret = hd_videoout_open(0, HD_VIDEOOUT_0_CTRL, &video_out_ctrl); //open this for device control

    }
    if(g_ide&VOUT_IDE2) {
    	ret = hd_videoout_open(0, HD_VIDEOOUT_1_CTRL, &video_out_ctrl); //open this for device control
    }

	if(ret!=HD_OK)
		return ret;

	memset((void *)&videoout_mode,0,sizeof(HD_VIDEOOUT_MODE));

	switch(out_type){
	case 0:
		videoout_mode.output_type = HD_COMMON_VIDEO_OUT_CVBS;
		videoout_mode.input_dim = HD_VIDEOOUT_IN_AUTO;
		videoout_mode.output_mode.cvbs= HD_VIDEOOUT_CVBS_NTSC;
	break;
	case 1:
		videoout_mode.output_type = HD_COMMON_VIDEO_OUT_LCD;
		videoout_mode.input_dim = HD_VIDEOOUT_IN_AUTO;
		videoout_mode.output_mode.lcd = HD_VIDEOOUT_LCD_0;
	break;
	case 2:
		videoout_mode.output_type = HD_COMMON_VIDEO_OUT_HDMI;
		videoout_mode.input_dim = HD_VIDEOOUT_IN_AUTO;
		videoout_mode.output_mode.hdmi= hdmi_id;
	break;
	default:
		printf("not support out_type\r\n");
	break;
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
	printf("##devcount %d\r\n", video_out_dev.max_dev_count);

	ret = hd_videoout_get(video_out_ctrl, HD_VIDEOOUT_PARAM_SYSCAPS, p_video_out_syscaps);
	if (ret != HD_OK) {
		return ret;
	}
	return ret;
}

static HD_RESULT set_out_param(HD_PATH_ID video_out_path, HD_DIM *p_dim, HD_URECT* p_rect)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOOUT_IN video_out_param={0};

	if (p_dim != NULL) {
    	video_out_param.dim.w = p_dim->w;
    	video_out_param.dim.h = p_dim->h;
	    video_out_param.dir = HD_VIDEO_DIR_NONE;
    }

	video_out_param.pxlfmt = vout_fmt;

	ret = hd_videoout_set(video_out_path, HD_VIDEOOUT_PARAM_IN, &video_out_param);
	if (ret != HD_OK) {
		return ret;
	}
	memset((void *)&video_out_param,0,sizeof(HD_VIDEOOUT_IN));
	ret = hd_videoout_get(video_out_path, HD_VIDEOOUT_PARAM_IN, &video_out_param);
	if (ret != HD_OK) {
		return ret;
	}

	if (p_rect != NULL) {
		HD_VIDEOOUT_WIN_ATTR video_out_param;
		video_out_param.visible = 1;
		video_out_param.rect.x = p_rect->x;
		video_out_param.rect.y = p_rect->y;
		video_out_param.rect.w = p_rect->w;
		video_out_param.rect.h = p_rect->h;
		video_out_param.layer = HD_LAYER1;

		ret =  hd_videoout_set(video_out_path, HD_VIDEOOUT_PARAM_IN_WIN_ATTR, &video_out_param);
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////

typedef struct _VIDEO_LIVEVIEW {

	// (1)
	HD_VIDEOCAP_SYSCAPS cap_syscaps;
	HD_PATH_ID cap_ctrl;
	HD_PATH_ID cap_path;

	HD_DIM  cap_dim;
	HD_DIM  proc_max_dim;

	// (2)
	HD_VIDEOPROC_SYSCAPS proc_syscaps;
	HD_PATH_ID proc_ctrl;
	HD_PATH_ID proc_path;

	HD_DIM  out_max_dim;
	HD_DIM  out_dim;

	// (3)
	HD_VIDEOOUT_SYSCAPS out_syscaps;
	HD_PATH_ID out_ctrl;
	HD_PATH_ID out_path;

    HD_VIDEOOUT_HDMI_ID hdmi_id;

	// (4) user pull
	pthread_t  aquire_thread_id;
	UINT32     proc_exit;
	UINT32   flow_start;

	UINT32  ext_mode;

} VIDEO_LIVEVIEW;

static HD_RESULT init_module(void)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_init()) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_init()) != HD_OK)
		return ret;
	if ((ret = hd_videoout_init()) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT open_module(VIDEO_LIVEVIEW *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;
    UINT32  source_path=0;
	// set videocap config
	ret = set_cap_cfg(&p_stream->cap_ctrl);
	if (ret != HD_OK) {
		printf("set cap-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim);
	if (ret != HD_OK) {
		printf("set proc-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	if ((ret = hd_videocap_open(HD_VIDEOCAP_IN(SEN1_VCAP_ID, 0), HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), &p_stream->cap_path)) != HD_OK)
		return ret;
    if(vprc_ext_fmt==HD_VIDEO_PXLFMT_YUV422_YUYV) {
        source_path = SOURCE_PATH1;
    }else {
        source_path = SOURCE_PATH;
    }

	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, source_path, &p_stream->proc_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT open_module_2(VIDEO_LIVEVIEW *p_stream, HD_DIM* p_proc_max_dim, UINT32 out_type)
{
	HD_RESULT ret;
	// set videoout config
	ret = set_out_cfg(&p_stream->out_ctrl, out_type,p_stream->hdmi_id);
	if (ret != HD_OK) {
		printf("set out-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, EXTEND_PATH, &p_stream->proc_path)) != HD_OK)
		return ret;
    if(g_ide & VOUT_IDE1) {
        if((ret = hd_videoout_open(HD_VIDEOOUT_0_IN_0, HD_VIDEOOUT_0_OUT_0, &p_stream->out_path)) != HD_OK)
            return ret;
    }
    if(g_ide & VOUT_IDE2) {
        if((ret = hd_videoout_open(HD_VIDEOOUT_1_IN_0, HD_VIDEOOUT_1_OUT_0, &p_stream->out_path)) != HD_OK)
            return ret;
    }
	return HD_OK;
}

static HD_RESULT close_module(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_close(p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_close(p_stream->proc_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT close_module_2(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_videoproc_close(p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoout_close(p_stream->out_path)) != HD_OK)
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
	if ((ret = hd_videoout_uninit()) != HD_OK)
		return ret;
	return HD_OK;
}

static void *aquire_yuv_thread(void *arg)
{
	VIDEO_LIVEVIEW* p_stream0 = (VIDEO_LIVEVIEW *)arg;
	HD_RESULT ret = HD_OK;
	HD_VIDEO_FRAME video_frame = {0};

	//--------- pull data test ---------
	while (p_stream0->proc_exit == 0) {

		ret = hd_videoproc_pull_out_buf(p_stream0->proc_path, &video_frame, 500); // -1 = blocking mode, 0 = non-blocking mode, >0 = blocking-timeout mode
		if (ret != HD_OK) {
			printf("pull_out(%d) error = %d!!\r\n", -1, ret);
    			goto skip;
		}
		//printf("pull_out(%d) ok!!\r\n");

		//printf("release_out() ....\r\n");
		ret = hd_videoproc_release_out_buf(p_stream0->proc_path, &video_frame);
		if (ret != HD_OK) {
			printf("release_out() error = %d!!\r\n", ret);
    			goto skip;
		}
skip:
		usleep(1000); //delay 1 ms
	}

	return 0;
}

static void show_help(void)
{
	printf("Usage: <out_type> <hdmi_mode> <ide> <vout_fmt>.\r\n");
	printf("Help:\r\n");
	printf("  <out_type> : 1(LCD), 2(HDMI)\r\n");
	printf("  <extend_out_mode> :0:share,1:scale,2:rotate,3:scale+rotate,4:crop+scale,5:crop+rotate,6:crop+scale+rotate\r\n");
	printf("  <hdmi_mode>: 0(640X480P60), 1(720X480P60), 5(1920X1080I60)\r\n");
	printf("  <ide>      : 1(ide1),2(ide2)\r\n");
	printf("  <ext_fmt>  : 0(yuv420),1(NVX2),2(MB4),3(NVX5)\r\n");
	printf("  <x_crop> : x position\r\n");

}
/*  format test case
extern scale nvx2  :hd_video_liveview_with_vprc_extend 2 1 16 1 1
extern rotate nvx5 :hd_video_liveview_with_vprc_extend 2 8 16 1 2
extern rotate mb4  :hd_video_liveview_with_vprc_extend 2 8 16 1 3
extern scale yvuv  :hd_video_liveview_with_vprc_extend 2 1 16 1 4
*/
MAIN(argc, argv)
{
	HD_RESULT ret;
	INT key;
	VIDEO_LIVEVIEW stream[2] = {0}; //0: main stream //1: sub stream
	UINT32 out_type = 0;
    UINT32 vout_test_fmt = 0;
	UINT32 extend_out_mode = 0; //0: test share, 1: test scale, 2: test rotate, 3: test scale+rotate
	HD_DIM out_dim;
	HD_DIM main_dim;

    if ((argc == 2)&&(*argv[1] == '?')) {
        show_help();
        return 0;
    }

	//vprc
	HD_URECT sub_crop; //extend crop
	HD_DIM sub_dim; //extend scale
	UINT32 sub_dir; //extend rotate

    //vout
	HD_URECT sub_win; //out window

	// query program options
	if (argc >= 2) {
		out_type = atoi(argv[1]);
		printf("out_type %d\r\n", out_type);
		if(out_type > 2) {
			printf("error: not support out_type!\r\n");
			return 0;
		}
	}
    stream[1].hdmi_id=HD_VIDEOOUT_HDMI_1920X1080I60;//default

	// query program options
	if (argc >= 3) {
		extend_out_mode = atoi(argv[2]);
		if(extend_out_mode > 8) {
			printf("error: not support extend_mode!\r\n");
			return 0;
		}
	}
	printf("run extend mode = %d\r\n", extend_out_mode);
	if (argc >= 4 && (atoi(argv[3]) !=0)) {
		stream[1].hdmi_id = atoi(argv[3]);
		printf("hdmi_mode %d\r\n", stream[1].hdmi_id);
	}
    if (argc >= 5 && (atoi(argv[4]) !=0)) {
        g_ide = atoi(argv[4]);
		printf("###g_ide %d\r\n",g_ide);
		if(g_ide > 2) {
			printf("error: not support g_ide!\r\n");
			return 0;
		}
	}

	if (argc >= 6) {
		vout_test_fmt = atoi(argv[5]);

        if(vout_test_fmt==1) {
            vprc_fmt = HD_VIDEO_PXLFMT_YUV420_NVX2;
    	    vout_fmt = HD_VIDEO_PXLFMT_YUV420_NVX2;
        } else if(vout_test_fmt==2) {
            vprc_fmt = HD_VIDEO_PXLFMT_YUV420_NVX5;
    	    vout_fmt = HD_VIDEO_PXLFMT_YUV420_NVX2;
        } else if(vout_test_fmt==3) {
            vprc_fmt = HD_VIDEO_PXLFMT_YUV420_MB4;
    	    vout_fmt = HD_VIDEO_PXLFMT_YUV420;
        } else if(vout_test_fmt==4) {
            vprc_fmt = HD_VIDEO_PXLFMT_YUV422_YUYV;
    	    vout_fmt = HD_VIDEO_PXLFMT_YUV420;
            vprc_ext_fmt = HD_VIDEO_PXLFMT_YUV422_YUYV;
        } else {
            vprc_fmt = HD_VIDEO_PXLFMT_YUV420;
    	    vout_fmt = HD_VIDEO_PXLFMT_YUV420;
        }
		printf("vout_test_fmt %d vout_fmt %x\r\n", vout_test_fmt,vout_fmt);
	}
	if (argc >= 7) {
		g_crop_x = atoi(argv[6]);
        printf("g_crop_x %d  \r\n",g_crop_x);
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
    if(vout_test_fmt !=0) {
	    hd_common_sysconfig(HD_VIDEOPROC_CFG, 0, HD_VIDEOPROC_CFG_STRIP_LV2, 0);
    }

	// init all modules
	ret = init_module();
	if (ret != HD_OK) {
		printf("init fail=%d\n", ret);
		goto exit;
	}

	// open video_liveview modules (main)
	stream[0].proc_max_dim.w = VDO_SIZE_W; //assign by user
	stream[0].proc_max_dim.h = VDO_SIZE_H; //assign by user
	ret = open_module(&stream[0], &stream[0].proc_max_dim);
	if (ret != HD_OK) {
		printf("open fail=%d\n", ret);
		goto exit;
	}

	// open video_liveview modules (sub)
	stream[1].proc_max_dim.w = VDO_SIZE_W; //assign by user
	stream[1].proc_max_dim.h = VDO_SIZE_H; //assign by user
	ret = open_module_2(&stream[1], &stream[1].proc_max_dim, out_type);
	if (ret != HD_OK) {
		printf("open fail=%d\n", ret);
		goto exit;
	}

	// get videocap capability
	ret = get_cap_caps(stream[0].cap_ctrl, &stream[0].cap_syscaps);
	if (ret != HD_OK) {
		printf("get cap-caps fail=%d\n", ret);
		goto exit;
	}

	// get videoout capability
	ret = get_out_caps(stream[1].out_ctrl, &stream[1].out_syscaps);
	if (ret != HD_OK) {
		printf("get out-caps fail=%d\n", ret);
		goto exit;
	}
	stream[1].out_max_dim = stream[1].out_syscaps.output_dim;

	// set videocap parameter
	stream[0].cap_dim.w = VDO_SIZE_W; //assign by user
	stream[0].cap_dim.h = VDO_SIZE_H; //assign by user
	ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim);
	if (ret != HD_OK) {
		printf("set cap fail=%d\n", ret);
		goto exit;
	}

	// assign parameter by program options
	main_dim.w = VDO_SIZE_W;
	main_dim.h = VDO_SIZE_H;
	out_dim.w = stream[1].out_max_dim.w; //display device size
	out_dim.h = stream[1].out_max_dim.h; //display device size
	DBGD(out_dim.w);
	DBGD(out_dim.h);
	sub_dim.w = out_dim.w;
	sub_dim.h = out_dim.h;
	sub_dir = HD_VIDEO_DIR_NONE;
	if (extend_out_mode == 0) {
		// test extend output with same data of src_path
		sub_crop.x = 0;
		sub_crop.y = 0;
		sub_crop.w = 0;
		sub_crop.h = 0;
		sub_dim.w = out_dim.w;
		sub_dim.h = out_dim.h;
		sub_dir = HD_VIDEO_DIR_NONE;
		sub_win.x = 0;
		sub_win.y = 0;
		sub_win.w = sub_dim.w;
		sub_win.h = sub_dim.h;
	} else if (extend_out_mode == 1) {
		// test extend output with scale
		sub_crop.x = 0;
		sub_crop.y = 0;
		sub_crop.w = 0;
		sub_crop.h = 0;
		sub_dim.w = out_dim.w;
		sub_dim.h = out_dim.h * 9 / 12;
		sub_dir = HD_VIDEO_DIR_NONE;
		sub_win.x = 0;
		sub_win.y = (out_dim.h - sub_dim.h)/2;
		sub_win.w = sub_dim.w;
		sub_win.h = sub_dim.h;
	} else if (extend_out_mode == 2) {
		// test extend output with rotate
		sub_crop.x = 0;
		sub_crop.y = 0;
		sub_crop.w = 0;
		sub_crop.h = 0;
		sub_dim.w = out_dim.w;
		sub_dim.h = out_dim.h;
		sub_dir = HD_VIDEO_DIR_ROTATE_90;
		sub_win.x = 0;
		sub_win.y = 0;
		sub_win.w = sub_dim.w;
		sub_win.h = sub_dim.h;
	} else if (extend_out_mode == 3) {
		// test extend output with scale, rotate
		sub_crop.x = 0;
		sub_crop.y = 0;
		sub_crop.w = 0;
		sub_crop.h = 0;
		sub_dim.w = ALIGN_CEIL_4(out_dim.w * 9 * 3 / 4 / 16);
		sub_dim.h = out_dim.h;
		sub_dir = HD_VIDEO_DIR_ROTATE_90;
		sub_win.x = (out_dim.w - sub_dim.w)/2;
		sub_win.y = 0;
		sub_win.w = sub_dim.w;
		sub_win.h = sub_dim.h;
	} else if (extend_out_mode == 4) {
		// test extend output with crop
		sub_crop.x = g_crop_x;
		sub_crop.y = 100;
		sub_crop.w = 640;
		sub_crop.h = 480;
		sub_dim.w = out_dim.w;
		sub_dim.h = out_dim.h;
		sub_dir = HD_VIDEO_DIR_NONE;
		sub_win.x = 0;
		sub_win.y = 0;
		sub_win.w = sub_dim.w;
		sub_win.h = sub_dim.h;
	} else if (extend_out_mode == 5) {
		// test extend output with crop, scale
		sub_crop.x = g_crop_x;
		sub_crop.y = 100;
		sub_crop.w = 640;
		sub_crop.h = 480;
		sub_dim.w = out_dim.w;
		sub_dim.h = out_dim.h * 9 / 12;
		sub_dir = HD_VIDEO_DIR_NONE;
		sub_win.x = 0;
		sub_win.y = (out_dim.h - sub_dim.h)/2;
		sub_win.w = sub_dim.w;
		sub_win.h = sub_dim.h;
	} else if (extend_out_mode == 6) {
		// test extend output with crop, rotate
		sub_crop.x = g_crop_x;
		sub_crop.y = 100;
		sub_crop.w = 640;
		sub_crop.h = 480;
		sub_dim.w = out_dim.w;
		sub_dim.h = out_dim.h;
		sub_dir = HD_VIDEO_DIR_ROTATE_90;
		sub_win.x = 0;
		sub_win.y = 0;
		sub_win.w = sub_dim.w;
		sub_win.h = sub_dim.h;
	} else if (extend_out_mode == 7) {
		// test extend output with crop, scale, rotate
		sub_crop.x = g_crop_x;
		sub_crop.y = 100;
		sub_crop.w = 640;
		sub_crop.h = 480;
		sub_dim.w = ALIGN_CEIL_32(out_dim.w * 9 * 3 / 4 / 16);
		sub_dim.h = out_dim.h;
		sub_dir = HD_VIDEO_DIR_ROTATE_90;
		sub_win.x = (out_dim.w - sub_dim.w)/2;
		sub_win.y = 0;
		sub_win.w = sub_dim.w;
		sub_win.h = sub_dim.h;
	} else {
		// test extend output with rotate (NVX5/MB4)
    	main_dim.w = 256;
    	main_dim.h = 256;
    	out_dim.w = 256;
    	out_dim.h = 256;

		sub_crop.x = 0;
		sub_crop.y = 0;
		sub_crop.w = 0;
		sub_crop.h = 0;
		sub_dim.w = ALIGN_CEIL_32(out_dim.h);
		sub_dim.h = ALIGN_CEIL_32(out_dim.w);
		sub_dir = HD_VIDEO_DIR_ROTATE_90;
		sub_win.x = 0;
		sub_win.y = 0;
		sub_win.w = out_dim.w;
		sub_win.h = out_dim.h;
	}
	stream[0].ext_mode = extend_out_mode;

	// set videoproc parameter (main)
	ret = set_proc_param(stream[0].proc_path, &main_dim, HD_VIDEO_DIR_NONE);
	if (ret != HD_OK) {
		printf("set proc fail=%d\n", ret);
		goto exit;
	}

	// set videoproc parameter (sub)
	if(vprc_ext_fmt==HD_VIDEO_PXLFMT_YUV422_YUYV) {
	    ret = set_proc_param_extend(stream[1].proc_path, SOURCE_PATH1, &sub_crop, &sub_dim, sub_dir);
	} else {
	    ret = set_proc_param_extend(stream[1].proc_path, SOURCE_PATH, &sub_crop, NULL, sub_dir);
	}
	if (ret != HD_OK) {
		printf("set proc fail=%d\n", ret);
		goto exit;
	}

	// set videoout parameter (sub)
	stream[1].out_dim.w = sub_dim.w;
	stream[1].out_dim.h = sub_dim.h;
	ret = set_out_param(stream[1].out_path, &stream[1].out_dim, &sub_win);
	if (ret != HD_OK) {
		printf("set out fail=%d\n", ret);
		goto exit;
	}

	// bind video_liveview modules (main)
	hd_videocap_bind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);
	//hd_videoproc_bind(SOURCE_PATH, ...); //src_path is not used

	// bind video_liveview modules (sub)
	if(g_ide & VOUT_IDE1) {
		hd_videoproc_bind(EXTEND_PATH, HD_VIDEOOUT_0_IN_0);
	}
	if(g_ide & VOUT_IDE2) {
		hd_videoproc_bind(EXTEND_PATH, HD_VIDEOOUT_1_IN_0);
	}

	// create aquire_thread (pull_out frame)
	ret = pthread_create(&stream[0].aquire_thread_id, NULL, aquire_yuv_thread, (void *)stream);
	if (ret < 0) {
		printf("create aquire thread failed");
		goto exit;
	}

	// start video_liveview modules (main)
	hd_videocap_start(stream[0].cap_path);
	hd_videoproc_start(stream[0].proc_path);

	// start video_liveview modules (sub)
	hd_videoproc_start(stream[1].proc_path);
	// just wait ae/awb stable for auto-test, if don't care, user can remove it
	sleep(1);

	if(vprc_ext_fmt != HD_VIDEO_PXLFMT_YUV422_YUYV){
 		hd_videoout_start(stream[1].out_path);
    } else {
        printf("vout not support YUYV,cat info and save yuvu\r\n");
    }

	// query user key
	printf("Enter q to exit\n");
	while (1) {
		key = GETCHAR();
		if (key == 'q' || key == 0x3) {
			// let aquire_thread stop loop and exit
			stream[0].proc_exit = 1;
            usleep(300000); //delay 300 ms
			// quit program
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

	// stop video_liveview modules (main)
	hd_videocap_stop(stream[0].cap_path);
	hd_videoproc_stop(stream[0].proc_path);

	// stop video_liveview modules (sub)
	hd_videoproc_stop(stream[1].proc_path);
	hd_videoout_stop(stream[1].out_path);

	// destroy aquire_thread
	pthread_join(stream[0].aquire_thread_id, (void* )NULL);

	// unbind video_liveview modules (main)
	hd_videocap_unbind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0));
	//hd_videoproc_unbind(SOURCE_PATH); //src_path is not used

	// unbind video_liveview modules (sub)
	hd_videoproc_unbind(EXTEND_PATH);

exit:
	// close video_liveview modules (main)
	ret = close_module(&stream[0]);
	if (ret != HD_OK) {
		printf("close fail=%d\n", ret);
	}

	// close video_liveview modules (sub)
	ret = close_module_2(&stream[1]);
	if (ret != HD_OK) {
		printf("close fail=%d\n", ret);
	}

	// uninit all modules
	ret = exit_module();
	if (ret != HD_OK) {
		printf("exit fail=%d\n", ret);
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
