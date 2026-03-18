/**
	@brief Sample code of video record.\n

	@file VIDEO_STREAM.c

	@author Boyan Huang

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <linux/usb/ch9.h>
#include <linux/usb/video.h>
#include <linux/videodev2.h>

#include "vendor_videoenc.h"
#include "vendor_audiocapture.h"
#include "vendor_common.h"
#include "vendor_videoprocess.h"
#include "uvc_cam_gadget.h"


// platform dependent
#if 1//defined(__LINUX)
#include <pthread.h>			//for pthread API
#define MAIN(argc, argv) 		int main(int argc, char** argv)
#define GETCHAR()				getchar()
#define msleep(x)				usleep(1000*x)
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <kwrap/util.h>		//for sleep API
#define sleep(x)    			vos_util_delay_ms(1000*(x))
#define msleep(x)    			vos_util_delay_ms(x)
#define usleep(x)   			vos_util_delay_us(x)
#include <kwrap/examsys.h> 	//for MAIN(), GETCHAR() API
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(hd_video_record, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define WRITE_BS 0
#define UVC_SUPPORT_YUV_FMT 1
#define UVAC_WAIT_RELEASE 1

#define PERF_TEST 0

#define POOL_SIZE_USER_DEFINIED  0x1000000

#define COMMON_RECFRM_BUFFER 0
#define COMMON_RECFRM_FUNC_BASE   0
#define COMMON_RECFRM_FUNC_SVC    1
#define COMMON_RECFRM_FUNC_LTR    0
#define VDO_SIZE_W_8M      3840
#define VDO_SIZE_H_8M      2160
#define VDO_SIZE_W_5M      2592
#define VDO_SIZE_H_5M      1944
#define VDO_SIZE_W_4M      2560
#define VDO_SIZE_H_4M      1440
#define VDO_SIZE_W_2M      1920
#define VDO_SIZE_H_2M      1080
#define VDO_SIZE_W_720P    1280
#define VDO_SIZE_H_720P    720
//Max sensor size
#define MAX_CAP_SIZE_W cap_size_w
#define MAX_CAP_SIZE_H cap_size_h
//Max bitstream size
#define MAX_BS_W cap_size_w
#define MAX_BS_H cap_size_h
#define MAX_BS_FPS 30
#define NVT_UI_UVAC_RESO_CNT  2
#if UVC_SUPPORT_YUV_FMT
#define MAX_YUV_W 1280//640
#define MAX_YUV_H 720//480
#define MAX_YUV_FPS 30
#endif

#define SEN_SEL_IMX290   0   //2M_SHDR
#define SEN_SEL_OS05A10  1   //5M_SHDR
#define SEN_SEL_OS02K10  2   //4M
#define SEN_SEL_IMX415   3   //8M
#define SEN_SEL_PATGEN   99


static UINT32 sensor_sel = SEN_SEL_OS02K10;

static UINT32 cap_size_w = 0;
static UINT32 cap_size_h = 0;
static UINT32 sen1_vcap_id = 2;

#define UVC_MJPEG_STREAM_BUF_MARGIN 110 //%
#define UVC_MJPEG_ENC_BLK_CNT   3
#define UVC_MJPEG_ENC_BUF_MS(fps) (1000*UVC_MJPEG_ENC_BLK_CNT*UVC_MJPEG_STREAM_BUF_MARGIN/100/(fps))

#define UVC_H264_ENC_BUF_MS 2000

#define MJPEG_TBR_MAX   (20*8*1024*1024)
#define MJPEG_TBR_2M    (20*8*1024*1024)
#define MJPEG_TBR_720P  (7*8*1024*1024)

#define H264_TBR_MAX    (2*8*1024*1024)
#define H264_TBR_2M     (8*1024*1024)
#define H264_TBR_720P   (8*1024*1024)
static UINT32 g_capbind = 0;  //0:D2D, 1:direct, 2: one-buf, 0xff: no-bind
static UINT32 g_one_buf = 1;    //0x0: vprc noraml, 0x1: one buffer
static UINT32 g_low_latency = 0;

///////////////////////////////////////////////////////////////////////////////

//header
#define DBGINFO_BUFSIZE()	(0x200)

//RAW
#define VDO_RAW_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
//NRX: RAW compress: Only support 12bit mode
#define RAW_COMPRESS_RATIO 59
#define VDO_NRX_BUFSIZE(w, h)           (ALIGN_CEIL_4(ALIGN_CEIL_64(w) * 12 / 8 * RAW_COMPRESS_RATIO / 100 * (h)))
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

#define CAP_OUT_FMT		HD_VIDEO_PXLFMT_NRX12

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

#define VIDEOCAP_ALG_FUNC (HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB)
#define VIDEOPROC_ALG_FUNC (HD_VIDEOPROC_FUNC_WDR | HD_VIDEOPROC_FUNC_COLORNR | HD_VIDEOPROC_FUNC_3DNR)

///////////////////////////////////////////////////////////////////////////////
static HD_VIDEOENC_BUFINFO phy_buf_main;
//static HD_VIDEOENC_BUFINFO phy_buf_sub1;
//static HD_VIDEOENC_BUFINFO phy_buf_sub2;
//static HD_AUDIOCAP_BUFINFO phy_buf_main2;
static ULONG vir_addr_main;
//static ULONG vir_addr_sub1;
//static ULONG vir_addr_sub2;
//static ULONG vir_addr_main2;
//static VK_TASK_HANDLE cap_thread_id;
static UINT32 flow_audio_start = 0, encode_start = 0, acquire_start = 0;
#define JPG_YUV_TRANS 1
static pthread_mutex_t flow_start_lock;

static VIDEO_STREAM stream[1] = {0}; //0: main stream, 1: sub stream


#define CS_INTERFACE                        0x24
#define VC_EXTENSION_UNIT                   0x06


//YUV definition for UVC
#define VDO_PXLFMT_YUV422_YUYV	(0x51102422) ///< 1 plane, pixel=YUYV(w,h), packed format with Y2U1V1
static HD_RESULT mem_init(void)
{
	HD_RESULT              ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
	HD_DIM buf_size;
	UINT32 id;

	// config common pool (cap)
	id = 0;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	if (g_capbind == 1) {
		//direct ... NOT require raw
		mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
														+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
	} else {
		buf_size.w = MAX_CAP_SIZE_W;
		buf_size.h = MAX_CAP_SIZE_H;
		mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_RAW_BUFSIZE(buf_size.w, buf_size.h, CAP_OUT_FMT)
														+VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
														+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
	}
	mem_cfg.pool_info[id].blk_cnt = 2;
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;

	// config common pool (main)
	id++;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(MAX_CAP_SIZE_W, ALIGN_CEIL_16(MAX_CAP_SIZE_H), HD_VIDEO_PXLFMT_YUV420);	//padding to 16x
	if (g_one_buf)
	mem_cfg.pool_info[id].blk_cnt = 1;
	else
	mem_cfg.pool_info[id].blk_cnt = 2;
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;

#if UVC_SUPPORT_YUV_FMT
	id++;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(MAX_YUV_W, ALIGN_CEIL_16(MAX_YUV_H), HD_VIDEO_PXLFMT_YUV422);	//padding to 16x
	mem_cfg.pool_info[id].blk_cnt = 3;
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;
#endif


#if COMMON_RECFRM_BUFFER
	id++;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(ALIGN_CEIL_64(MAX_BS_W), ALIGN_CEIL_64(MAX_BS_H), HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[id].blk_cnt = COMMON_RECFRM_FUNC_BASE + COMMON_RECFRM_FUNC_SVC + COMMON_RECFRM_FUNC_LTR;
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;
#endif

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

	if (sensor_sel == SEN_SEL_IMX290) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_imx290");
		printf("Using nvt_sen_imx290\n");
	} else if (sensor_sel == SEN_SEL_OS05A10) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os05a10");
		printf("Using nvt_sen_os05a10\n");
	} else if (sensor_sel == SEN_SEL_OS02K10) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os02k10");
		printf("Using nvt_sen_os02k10\n");
	} else if (sensor_sel == SEN_SEL_IMX415) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_imx415");
		printf("Using nvt_sen_imx415\n");
	} else if (sensor_sel == SEN_SEL_PATGEN) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, HD_VIDEOCAP_SEN_PAT_GEN);
		printf("Using pattern gen\n");
	} else {
		printf("Unsupported sensor!\n");
		return HD_ERR_NOT_SUPPORT;
	}

	ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(sen1_vcap_id), &video_cap_ctrl); //open this for device control

	if (ret != HD_OK) {
		return ret;
	}
	if (sensor_sel != SEN_SEL_PATGEN) {
		cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
	    cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux =  0;  //use @0 in peri-dev.dtsi

		if (sen1_vcap_id == 0) {
			cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(0, 0);
		} else {//(sen1_vcap_id == 2)
			cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(1, 0);
		}

		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;

		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = HD_VIDEOCAP_SEN_IGNORE;
	}

	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);

	if (sensor_sel != SEN_SEL_PATGEN) {
		iq_ctl.func = VIDEOCAP_ALG_FUNC;
	}

	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);
	*p_video_cap_ctrl = video_cap_ctrl;
	return ret;
}

static HD_RESULT set_cap_param(HD_PATH_ID video_cap_path, HD_DIM *p_dim, int fps, HD_DIM *p_out_dim)
{
	HD_RESULT ret = HD_OK;
	{//select sensor mode, manually or automatically
		HD_VIDEOCAP_IN video_in_param = {0};

		if (sensor_sel != SEN_SEL_PATGEN) {
			video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO; //auto select sensor mode by the parameter of HD_VIDEOCAP_PARAM_OUT
			video_in_param.frc = HD_VIDEO_FRC_RATIO(30,1);
			video_in_param.dim.w = p_dim->w;
			video_in_param.dim.h = p_dim->h;
			video_in_param.pxlfmt = SEN_OUT_FMT;
		} else {
			video_in_param.sen_mode = HD_VIDEOCAP_PATGEN_MODE(HD_VIDEOCAP_SEN_PAT_COLORBAR, 200);
			video_in_param.frc = HD_VIDEO_FRC_RATIO(30,1);
			video_in_param.dim.w = p_dim->w;
			video_in_param.dim.h = p_dim->h;
		}
		video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN, &video_in_param);
		printf("set_cap_param fps=%d\r\n", fps);
		if (ret != HD_OK) {
			return ret;
		}
	}
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
		video_crop_param.win.rect.w = MAX_CAP_SIZE_W/2;
		video_crop_param.win.rect.h= MAX_CAP_SIZE_H/2;
		video_crop_param.align.w = 4;
		video_crop_param.align.h = 4;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);
		//printf("set_cap_param CROP ON=%d\r\n", ret);
	}
	#endif
	{
		HD_VIDEOCAP_OUT video_out_param = {0};

		//without setting dim for no scaling, using original sensor out size
		video_out_param.dim.w = p_out_dim->w;
		video_out_param.dim.h = p_out_dim->h;
		video_out_param.pxlfmt = CAP_OUT_FMT;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		if (g_capbind == 0) //D2D mode, for pull raw frame
			video_out_param.depth = 1;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT, &video_out_param);
		//printf("set_cap_param OUT=%d\r\n", ret);
	}

	{
		HD_VIDEOCAP_FUNC_CONFIG video_path_param = {0};

		video_path_param.out_func = 0;
		if (g_capbind == 1) //direct mode
			video_path_param.out_func = HD_VIDEOCAP_OUTFUNC_DIRECT;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_FUNC_CONFIG, &video_path_param);
		//printf("set_cap_param PATH_CONFIG=0x%X\r\n", ret);
	}

	return ret;
}

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
		video_cfg_param.isp_id = sen1_vcap_id;
		video_cfg_param.ctrl_max.func = VIDEOPROC_ALG_FUNC;
		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;
		video_cfg_param.in_max.pxlfmt = CAP_OUT_FMT;
		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1,1);
		printf("vin w= %d, h = %d\r\n", p_max_dim->w, p_max_dim->h);
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

		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
		//printf("set_proc_param PATH_CONFIG=0x%X\r\n", ret);
	}
	video_ctrl_param.func = VIDEOPROC_ALG_FUNC;
	video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_0_OUT_0;
	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);
	if (ret != HD_OK) {
		return HD_ERR_NG;
	}
	*p_video_proc_ctrl = video_proc_ctrl;

	return ret;
}

static HD_RESULT set_proc_param(HD_PATH_ID video_proc_path, HD_DIM* p_dim, HD_VIDEO_PXLFMT pxlfmt, UINT32 pull_allow, UINT32 b_one_buf)
{
	HD_RESULT ret = HD_OK;

	if (p_dim != NULL) { //if videoproc is already binding to dest module, not require to setting this!
		HD_VIDEOPROC_OUT video_out_param = {0};
		video_out_param.func = 0;
		video_out_param.dim.w = p_dim->w;
		video_out_param.dim.h = p_dim->h;
		video_out_param.pxlfmt = pxlfmt;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		video_out_param.frc = HD_VIDEO_FRC_RATIO(1,1);
		video_out_param.depth = pull_allow; //set 1 to allow pull
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	}

	HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};
	video_path_param.out_func = 0;
	if (g_low_latency == 1 && video_proc_path == stream[0].proc_main_path){
		//only the proc_path should be set
		printf("g_low_latency = %d\r\n", g_low_latency);
		video_path_param.out_func |= HD_VIDEOPROC_OUTFUNC_LOWLATENCY;
	}
	if (b_one_buf == 1){
		printf("g_one_buf = %d\r\n", g_one_buf);
		video_path_param.out_func |= HD_VIDEOPROC_OUTFUNC_ONEBUF;
	}
	ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	if (ret != HD_OK) {
		return HD_ERR_NG;
	}

	if (stream[0].codec_type == HD_CODEC_TYPE_JPEG){
		UINT32 h_align = 16;
		//to avoid MJPEG might encode read YUV out of range
		ret = vendor_videoproc_set(video_proc_path, VENDOR_VIDEOPROC_PARAM_HEIGHT_ALIGN, &h_align);
		if (ret != HD_OK) {
			printf("VENDOR_VIDEOPROC_PARAM_HEIGHT_ALIGN failed!(%d)\r\n", ret);
		}
	} else {
		UINT32 h_align = 2;
		//to avoid MJPEG might encode read YUV out of range
		ret = vendor_videoproc_set(video_proc_path, VENDOR_VIDEOPROC_PARAM_HEIGHT_ALIGN, &h_align);
		if (ret != HD_OK) {
			printf("VENDOR_VIDEOPROC_PARAM_HEIGHT_ALIGN failed!(%d)\r\n", ret);
		}
	}
	return ret;
}

static HD_RESULT set_enc_cfg(HD_PATH_ID video_enc_path, HD_DIM *p_max_dim, HD_VIDEO_CODEC codec_type, UINT32 max_bitrate)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOENC_PATH_CONFIG video_path_config = {0};
	VENDOR_VIDEOENC_H26X_ENC_COLMV h26x_enc_colmv = {0};
	VENDOR_VIDEOENC_FIT_WORK_MEMORY fit_work_mem_cfg = {0};

	//fit_work_mem_cfg.b_enable = TRUE;
	fit_work_mem_cfg.b_enable = FALSE;
    vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_FIT_WORK_MEMORY, &fit_work_mem_cfg);

	h26x_enc_colmv.h26x_colmv_en = 0;
	vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_COLMV, &h26x_enc_colmv);

#if COMMON_RECFRM_BUFFER
	VENDOR_VIDEOENC_H26X_COMM_RECFRM h26x_comm_recfrm = {0};

	h26x_comm_recfrm.enable = 1;
	h26x_comm_recfrm.h26x_comm_base_recfrm_en = COMMON_RECFRM_FUNC_BASE;
	h26x_comm_recfrm.h26x_comm_svc_recfrm_en = COMMON_RECFRM_FUNC_SVC;
	h26x_comm_recfrm.h26x_comm_ltr_recfrm_en = COMMON_RECFRM_FUNC_LTR;
	vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_COMM_RECFRM, &h26x_comm_recfrm);
#endif

	if (p_max_dim != NULL) {

		//--- HD_VIDEOENC_PARAM_PATH_CONFIG ---
		video_path_config.max_mem.codec_type = codec_type;
		video_path_config.max_mem.max_dim.w  = p_max_dim->w;
		video_path_config.max_mem.max_dim.h  = p_max_dim->h;
		video_path_config.max_mem.bitrate    = max_bitrate;
		video_path_config.max_mem.enc_buf_ms = UVC_H264_ENC_BUF_MS;
		video_path_config.max_mem.svc_layer  = HD_SVC_4X;
		video_path_config.max_mem.ltr        = FALSE;
		video_path_config.max_mem.rotate     = FALSE;
		video_path_config.max_mem.source_output   = FALSE;
		video_path_config.isp_id             = 0;
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_PATH_CONFIG, &video_path_config);
		if (ret != HD_OK) {
			printf("set_enc_path_config = %d\r\n", ret);
			return HD_ERR_NG;
		}
	}
	{
		HD_VIDEOENC_FUNC_CONFIG video_func_config = {0};
			video_func_config.in_func = 0;
		if (g_low_latency == 1)
			video_func_config.in_func |= HD_VIDEOENC_INFUNC_LOWLATENCY;
		else if (g_one_buf == 1)
			video_func_config.in_func |= HD_VIDEOENC_INFUNC_ONEBUF;

		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_FUNC_CONFIG, &video_func_config);
		if (ret != HD_OK) {
			printf("set_enc_path_config = %d\r\n", ret);
			return HD_ERR_NG;
		}
	}
	return ret;
}

static HD_RESULT set_enc_param(HD_PATH_ID video_enc_path, HD_DIM *p_dim, UINT32 enc_type, UINT32 bitrate)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOENC_IN  video_in_param = {0};
	HD_VIDEOENC_OUT video_out_param = {0};
	HD_VIDEOENC_OUT2 video_out2_param = {0};
	HD_H26XENC_RATE_CONTROL rc_param = {0};

	if (p_dim != NULL) {

		//--- HD_VIDEOENC_PARAM_IN ---
		video_in_param.dir           = HD_VIDEO_DIR_NONE;
		video_in_param.pxl_fmt = HD_VIDEO_PXLFMT_YUV420;
		video_in_param.dim.w   = p_dim->w;
		video_in_param.dim.h   = p_dim->h;
		video_in_param.frc     = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_IN, &video_in_param);
		if (ret != HD_OK) {
			printf("set_enc_param_in = %d\r\n", ret);
			return ret;
		}

		if (enc_type == HD_CODEC_TYPE_H264) {
			VENDOR_VIDEOENC_MIN_RATIO_CFG min_ratio = {0};

			min_ratio.min_i_ratio = UVC_H264_ENC_BUF_MS/2;
			min_ratio.min_p_ratio = UVC_H264_ENC_BUF_MS/3;
			vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_MIN_RATIO, &min_ratio);

			//printf("enc_type = HD_CODEC_TYPE_H264\r\n");
			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_H264;
			video_out_param.h26x.profile       = HD_H264E_HIGH_PROFILE;
			video_out_param.h26x.level_idc     = HD_H264E_LEVEL_5_1;
			video_out_param.h26x.gop_num       = 15;
			video_out_param.h26x.ltr_interval  = 0;
			video_out_param.h26x.ltr_pre_ref   = 0;
			video_out_param.h26x.gray_en       = 0;
			video_out_param.h26x.source_output = 0;
			video_out_param.h26x.svc_layer     = HD_SVC_4X;
			video_out_param.h26x.entropy_mode  = HD_H264E_CABAC_CODING;
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &video_out_param);
			if (ret != HD_OK) {
				printf("set_enc_param_out = %d\r\n", ret);
				return ret;
			}

			//--- HD_VIDEOENC_PARAM_OUT_RATE_CONTROL ---
			rc_param.rc_mode             = HD_RC_MODE_CBR;
			rc_param.cbr.bitrate         = bitrate;
			rc_param.cbr.frame_rate_base = 30;
			rc_param.cbr.frame_rate_incr = 1;
			rc_param.cbr.init_i_qp       = 26;
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

		} else if (enc_type == HD_CODEC_TYPE_JPEG) {
			//printf("enc_type = HD_CODEC_TYPE_JPEG\r\n");
			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			VENDOR_VIDEOENC_MIN_RATIO_CFG min_ratio = {0};

			//ms*(PATH CONFIG target bitrate) to check if remaining buffer size is enough
			//only min_i_ratio is valid in MJPEG encode. bitstream buffer is designed to be less than (ms/1000)*target bitrate
			min_ratio.min_i_ratio = 1000*UVC_MJPEG_STREAM_BUF_MARGIN/100;//ms
			min_ratio.min_p_ratio = min_ratio.min_i_ratio;
			vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_MIN_RATIO, &min_ratio);

#if 0	//fixed quality
			video_out_param.codec_type         = HD_CODEC_TYPE_JPEG;
			video_out_param.jpeg.retstart_interval = 0;
			if (p_dim->w >= VDO_SIZE_W_4M ) {
				video_out_param.jpeg.image_quality = 90;
			} else {
				video_out_param.jpeg.image_quality = 95;
			}
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &video_out_param);
			if (ret != HD_OK) {
				printf("set_enc_param_out = %d\r\n", ret);
				return ret;
			}
#else	//MJPEG CBR
			video_out2_param.codec_type         = HD_CODEC_TYPE_JPEG;
			video_out2_param.jpeg.retstart_interval = 0;
			video_out2_param.jpeg.image_quality = 80;
			video_out2_param.jpeg.bitrate = bitrate;
			video_out2_param.jpeg.frame_rate_base = MAX_BS_FPS;
			video_out2_param.jpeg.frame_rate_incr = 1;
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM2, &video_out2_param);

			VENDOR_VIDEOENC_JPG_RC_CFG jpg_rc_param = {0};
			jpg_rc_param.vbr_mode_en = 0;  // 0: CBR, 1: VBR
			jpg_rc_param.min_quality = 1; // min quality
			jpg_rc_param.max_quality = 100; // max quality
			vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_JPG_RC, &jpg_rc_param);
			if (ret != HD_OK) {
				printf("set_enc_param_out = %d\r\n", ret);
				return ret;
			}
#endif
			//MJPG YUV trans(YUV420 to YUV422)
      #if JPG_YUV_TRANS
			VENDOR_VIDEOENC_JPG_YUV_TRANS_CFG yuv_trans_cfg = {0};
			yuv_trans_cfg.jpg_yuv_trans_en = JPG_YUV_TRANS;
			//printf("MJPG YUV TRANS(%d)\r\n", JPG_YUV_TRANS);
			vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_JPG_YUV_TRANS, &yuv_trans_cfg);
      #endif

		} else {

			printf("not support enc_type\r\n");
			return HD_ERR_NG;
		}
	}

	return ret;
}

static HD_RESULT init_module(void)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_init()) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_init()) != HD_OK)
		return ret;
    if ((ret = hd_videoenc_init()) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT open_module(VIDEO_STREAM *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;
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
	if ((ret = hd_videocap_open(HD_VIDEOCAP_IN(sen1_vcap_id, 0), HD_VIDEOCAP_OUT(sen1_vcap_id, 0), &p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &p_stream->proc_main_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_1,  &p_stream->proc_sub_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_VIDEOENC_0_OUT_0, &p_stream->enc_main_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT close_module(VIDEO_STREAM *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_close(p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_close(p_stream->proc_main_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_close(p_stream->proc_sub_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_close(p_stream->enc_main_path)) != HD_OK)
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

	return HD_OK;
}

#define MAX_BITSTREAM_LEN 0x100000

#define UVC_EVENT_FIRST			(V4L2_EVENT_PRIVATE_START + 0)
#define UVC_EVENT_CONNECT		(V4L2_EVENT_PRIVATE_START + 0)
#define UVC_EVENT_DISCONNECT		(V4L2_EVENT_PRIVATE_START + 1)
#define UVC_EVENT_STREAMON		(V4L2_EVENT_PRIVATE_START + 2)
#define UVC_EVENT_STREAMOFF		(V4L2_EVENT_PRIVATE_START + 3)
#define UVC_EVENT_SETUP			(V4L2_EVENT_PRIVATE_START + 4)
#define UVC_EVENT_DATA			(V4L2_EVENT_PRIVATE_START + 5)
#define UVC_EVENT_LAST			(V4L2_EVENT_PRIVATE_START + 5)

#define SET_CUR		0x01
#define GET_CUR		0x81
#define GET_MIN		0x82
#define GET_MAX		0x83
#define GET_RES		0x84
#define GET_LEN		0x85
#define GET_INFO	0x86
#define GET_DEF		0x87

struct uvc_request_data
{
	__s32 length;
	__u8 data[60];
};

struct uvc_event
{
	union {
		enum usb_device_speed speed;
		struct usb_ctrlrequest req;
		struct uvc_request_data data;
	};
};

#define UVCIOC_SEND_RESPONSE		_IOW('U', 1, struct uvc_request_data)

#define UVC_INTF_CONTROL		0
#define UVC_INTF_STREAMING		1

//#include "uvc.h"

#define clamp(val, min, max) ({                 \
        typeof(val) __val = (val);              \
        typeof(min) __min = (min);              \
        typeof(max) __max = (max);              \
        (void) (&__val == &__min);              \
        (void) (&__val == &__max);              \
        __val = __val < __min ? __min: __val;   \
        __val > __max ? __max: __val; })

#define ARRAY_SIZE(a)	((sizeof(a) / sizeof(a[0])))
static unsigned int uvc_enable;
typedef enum _data_type{
	DATA_TYPE_CONTROL = 0,
	DATA_TYPE_STREAMING,
	DATA_TYPE_NUM,
} data_type;

struct uvc_device
{
	int fd;

	data_type type;

	struct uvc_streaming_control probe;
	struct uvc_streaming_control commit;

	int control;

	unsigned int fcc;
	unsigned int width;
	unsigned int height;

	void **mem;
	unsigned int nbufs;
	unsigned int bufsize;

	unsigned int bulk;
	uint8_t color;
	unsigned int imgsize;
	void *imgdata;
};

static void hd_video_stream_update(struct uvc_device *dev)
{
    HD_RESULT ret;
	//int j;
	int fps;
	int bitrate;

	printf("%s++: %d x %d\r\n", __func__, dev->width, dev->height);
	// assign parameter by program options
    if (g_capbind == 1) {
		hd_videoproc_stop(stream[0].proc_main_path);
		hd_videoproc_stop(stream[0].proc_sub_path);
		hd_videocap_stop(stream[0].cap_path);
	} else {
		hd_videocap_stop(stream[0].cap_path);
		hd_videoproc_stop(stream[0].proc_main_path);
		hd_videoproc_stop(stream[0].proc_sub_path);
	}
	if (stream[0].codec_type != HD_CODEC_TYPE_RAW){
		hd_videoenc_stop(stream[0].enc_main_path);
	}

	if (dev->fcc == V4L2_PIX_FMT_MJPEG) {
		printf("dev->fcc == V4L2_PIX_FMT_MJPEG\n");
		stream[0].codec_type = HD_CODEC_TYPE_JPEG;
	} else if (dev->fcc == V4L2_PIX_FMT_H264) {
		printf("dev->fcc == V4L2_PIX_FMT_H264\n");
		stream[0].codec_type = HD_CODEC_TYPE_H264;
	} else if (dev->fcc == V4L2_PIX_FMT_YUYV) {
		printf("dev->fcc == V4L2_PIX_FMT_YUYV\n");
		stream[0].codec_type = HD_CODEC_TYPE_RAW;
    } else {
		printf("dev->fcc == unavailable!!!\n");
    }
    //set encode resolution to device
	stream[0].enc_main_dim.w = dev->width;
	stream[0].enc_main_dim.h = dev->height;

    //one buffer limitation
	hd_videocap_unbind(HD_VIDEOCAP_0_OUT_0);
	hd_videoproc_close(stream[0].proc_main_path);
	hd_videoproc_close(stream[0].proc_sub_path);
	set_proc_cfg(&stream[0].proc_ctrl, &stream[0].cap_dim);
	#if 0
	if (pStrmInfo->strmCodec != UVAC_VIDEO_FORMAT_YUV) {
		//set_proc_cfg(&stream[0].proc_ctrl, &stream[0].enc_main_dim);
		hd_videoproc_get(stream[0].proc_main_path, HD_VIDEOPROC_PARAM_IN, &video_in_param);
		video_in_param.dim.w = stream[0].enc_main_dim.w;
		video_in_param.dim.h = stream[0].enc_main_dim.h;
		hd_videoproc_set(stream[0].proc_main_path, HD_VIDEOPROC_PARAM_IN, &video_in_param);
	} else {
		//set_proc_cfg(&stream[0].proc_ctrl, &stream[0].cap_dim);
		hd_videoproc_get(stream[0].proc_main_path, HD_VIDEOPROC_PARAM_IN, &video_in_param);
		video_in_param.dim.w = stream[0].cap_dim.w;
		video_in_param.dim.h = stream[0].cap_dim.h;
		hd_videoproc_set(stream[0].proc_main_path, HD_VIDEOPROC_PARAM_IN, &video_in_param);
	}
	#endif
	hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &stream[0].proc_main_path);
	hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_1, &stream[0].proc_sub_path);
	hd_videocap_bind(HD_VIDEOCAP_0_OUT_0, HD_VIDEOPROC_0_IN_0);

	if (stream[0].codec_type == HD_CODEC_TYPE_RAW) {
		//YUV format: stop vdoenc and allow pull from vprc sub path
		ret = set_proc_param(stream[0].proc_main_path, &stream[0].cap_dim, HD_VIDEO_PXLFMT_YUV420, 0, 1);
		ret = set_proc_param(stream[0].proc_sub_path, &stream[0].enc_main_dim, VDO_PXLFMT_YUV422_YUYV, 1, 0);
	} else {
		ret = set_proc_param(stream[0].proc_main_path, &stream[0].enc_main_dim, HD_VIDEO_PXLFMT_YUV420, 0, 1);
	}
	if (ret != HD_OK) {
		printf("set proc fail=%d\n", ret);
	}
	if (stream[0].codec_type == HD_CODEC_TYPE_JPEG) {
		//MJPG bitrate
		if (stream[0].enc_main_dim.w > VDO_SIZE_W_2M ) {
			bitrate = MJPEG_TBR_MAX;
		} else if(stream[0].enc_main_dim.w > VDO_SIZE_W_720P) {
			bitrate = MJPEG_TBR_2M;
		} else {
			bitrate = MJPEG_TBR_720P;
		}
	} else if (stream[0].codec_type == HD_CODEC_TYPE_RAW) {
		//YUV bitrate
		bitrate = 8*12*1024*1024;
	} else {
		//H264 bitrate
		if (stream[0].enc_main_dim.w > VDO_SIZE_W_2M) {
			bitrate = H264_TBR_MAX;
		} else if(stream[0].enc_main_dim.w > VDO_SIZE_W_720P) {
			bitrate = H264_TBR_2M;
		} else {
			bitrate = H264_TBR_720P;
		}
	}
	// set videoenc parameter (main)
	if (stream[0].codec_type != HD_CODEC_TYPE_RAW){
		ret = set_enc_param(stream[0].enc_main_path, &stream[0].enc_main_dim, stream[0].codec_type, bitrate);
	}
	if (ret != HD_OK) {
		printf("set enc fail=%d\n", ret);
	}
	#if 1
	fps = 30;//
	if (stream[0].codec_type == HD_CODEC_TYPE_RAW){
		ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim, fps, &stream[0].cap_dim);
	} else {
		ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim, fps, &stream[0].enc_main_dim);
	}
	if (ret != HD_OK) {
		printf("set cap fail=%d\n", ret);
	}
	#endif
	if (g_capbind == 0) {
		//start engine(modules)
		hd_videocap_start(stream[0].cap_path);
	}
	hd_videoproc_start(stream[0].proc_main_path);
	if (stream[0].codec_type == HD_CODEC_TYPE_RAW){
		hd_videoproc_start(stream[0].proc_sub_path);
	}
	if (g_capbind == 1) {
		//start engine(modules)
		hd_videocap_start(stream[0].cap_path);
	}
	// just wait ae/awb stable for auto-test, if don't care, user can remove it
	//sleep(1);
	if (stream[0].codec_type != HD_CODEC_TYPE_RAW){
		hd_videoenc_start(stream[0].enc_main_path);
	}
	printf("%s--\r\n", __func__);
	return;
}


static struct uvc_device *uvc_open(const char *devname)
{
	struct uvc_device *dev;
	struct v4l2_capability cap;
	int ret;
	int fd;

	fd = open(devname, O_RDWR | O_NONBLOCK);
	if (fd == -1) {
		printf("v4l2 open failed: %s (%d)\n", strerror(errno), errno);
		return NULL;
	}

	printf("open succeeded, file descriptor = %d\n", fd);

	ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
	if (ret < 0) {
		printf("unable to query device: %s (%d)\n", strerror(errno),
			errno);
		close(fd);
		return NULL;
        }

	dev = malloc(sizeof *dev);
	if (dev == NULL) {
		close(fd);
		return NULL;
	}

	memset(dev, 0, sizeof *dev);
	dev->fd = fd;

	return dev;
}

static void
uvc_close(struct uvc_device *dev)
{
	close(dev->fd);
	free(dev->imgdata);
	free(dev->mem);
	free(dev);
}

/* ---------------------------------------------------------------------------
 * Video streaming
 */

//static int get_phy_flag=0;
static void uvc_video_fill_buffer(struct uvc_device *dev, struct v4l2_buffer *buf)
{
	//unsigned int i;
	HD_RESULT ret = HD_OK;
	UINT8 *ptr;
	#if WRITE_BS
	char file_path_main[32] = "/mnt/sd/dump_bs_main.bs";
	FILE *f_out_main;
	static UINT32 j = 0;
	#endif

	//printf("uvc_video_fill_buffer+++\r\n");
	buf->bytesused = 0;

	if (dev->fcc == V4L2_PIX_FMT_YUYV) {
		#if 1
		HD_VIDEO_FRAME video_frame = {0};
		ULONG phy_addr_main_yuv, vir_addr_main_yuv;
		UINT32 blk_size, yuv_size;
		HD_DIM dim;


		#define PHY2VIRT_MAIN_YUV(pa) (vir_addr_main_yuv + ((pa) - phy_addr_main_yuv))

		dim.w = stream[0].enc_main_dim.w;
		dim.h = stream[0].enc_main_dim.h;
		ret = hd_videoproc_pull_out_buf(stream[0].proc_sub_path, &video_frame, 1000); // -1 = blocking mode, 0 = non-blocking mode, >0 = blocking-timeout mode
		if (ret != HD_OK) {
			printf("pull_out(1000) ret = %d!!\r\n", ret);
			return;
		}

		phy_addr_main_yuv = hd_common_mem_blk2pa(video_frame.blk); // Get physical addr
		if (phy_addr_main_yuv == 0) {
			printf("blk2pa fail, blk = 0x%x\r\n", video_frame.blk);
			goto skip_yuv;
		}

		blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(dim.w, dim.h, HD_VIDEO_PXLFMT_YUV422);

		// mmap for frame buffer (just mmap one time only, calculate offset to virtual address later)
		//printf("phy_addr_main_yuv: 0x%x, 0x%x\n", phy_addr_main_yuv, blk_size);
		vir_addr_main_yuv = (ULONG)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_main_yuv, blk_size);
		if (vir_addr_main_yuv == 0) {
			printf("mmap error !!\r\n\r\n");
			goto skip_yuv;
		}

		ptr = (UINT8 *)PHY2VIRT_MAIN_YUV(video_frame.phy_addr[0]);

		yuv_size = video_frame.loff[0]*video_frame.ph[0] + video_frame.loff[1]*video_frame.ph[1];
		#if WRITE_BS
		j++;
		if (j == 300) {
			//----- open output files -----
			if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
				HD_VIDEOENC_ERR("open file (%s) fail....\r\n", file_path_main);
			} else {
				printf("save yuyv_%dx%d(%d) to %s\r\n", dim.w, dim.h, yuv_size, file_path_main);
			}
			if (f_out_main) fwrite(ptr, 1, yuv_size, f_out_main);
			if (f_out_main) fflush(f_out_main);
			if (f_out_main) fclose(f_out_main);
		}
		#endif

		memcpy(dev->mem[buf->index], ptr, yuv_size);
		buf->bytesused = yuv_size;

		// mummap for frame buffer
		ret = hd_common_mem_munmap((void *)vir_addr_main_yuv, blk_size);
		if (ret != HD_OK) {
			printf("mnumap error !!\r\n\r\n");
		}
skip_yuv:
		ret = hd_videoproc_release_out_buf(stream[0].proc_sub_path, &video_frame);
		if (ret != HD_OK) {
			printf("release_out() error !!\r\n\r\n");
		}
		#else// test pattern
		unsigned int bpl;
		unsigned int i;
		/* Fill the buffer with video data. */
		bpl = dev->width * 2;
		for (i = 0; i < dev->height; ++i)
			memset(dev->mem[buf->index] + i*bpl, dev->color++, bpl);

		buf->bytesused = bpl * dev->height;
		#endif

	} else if (dev->fcc == V4L2_PIX_FMT_MJPEG || dev->fcc == V4L2_PIX_FMT_H264) {
		HD_VIDEOENC_BS  data_pull_main;
		UINT32 loop =0;
		ULONG stream_pa;
		UINT32 stream_size;
		#define PHY2VIRT_MAIN(pa) (vir_addr_main + (pa - phy_buf_main.buf_info.phy_addr))
		//pull data
		//ret = hd_videoenc_pull_out_buf(stream[0].enc_main_path, &data_pull_main, -1); // -1 = blocking mode
		ret = hd_videoenc_pull_out_buf(stream[0].enc_main_path, &data_pull_main, 500); // -1 = blocking mode
		if (ret == HD_OK) {
			//UINT64 cur_time = hd_gettime_us();
			//printf("vd = %lld, encout = %lld, diff = %lld\r\n", data_pull_main.timestamp, cur_time, cur_time - data_pull_main.timestamp);

			stream_pa = data_pull_main.video_pack[data_pull_main.pack_num-1].phy_addr;
			stream_size = data_pull_main.video_pack[data_pull_main.pack_num-1].size;
			//if (stream[0].codec_type == HD_CODEC_TYPE_H264){
			if (dev->fcc == V4L2_PIX_FMT_H264) {
				if (data_pull_main.pack_num > 1) {
					UINT32 header_size = 0;
					//if stream_pa contains SPS/PPS, there is no need of pStrmHdr
					for (loop = 0; loop < data_pull_main.pack_num - 1; loop ++) {
						header_size += data_pull_main.video_pack[loop].size;
					}
					stream_size += header_size;
					stream_pa -= header_size;
				}
			}
			ptr = (UINT8 *)PHY2VIRT_MAIN(stream_pa);

			memcpy(dev->mem[buf->index], ptr, stream_size);
			buf->bytesused = stream_size;
			//printf("++++va_main=0x%08X\n", strmFrm.va);

			/*timstamp for test
			if (temp){
				system("echo \"uvc\" > /proc/nvt_info/bootts");
				temp=0;
			}
			*/

#if 0//WRITE_BS
			for (j=0; j< data_pull_main.pack_num; j++) {
				UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(data_pull_main.video_pack[j].phy_addr);
				UINT32 len = data_pull_main.video_pack[j].size;
				if (f_out_main) fwrite(ptr, 1, len, f_out_main);
				if (f_out_main) fflush(f_out_main);
			}
#endif
			// release data
			ret = hd_videoenc_release_out_buf(stream[0].enc_main_path, &data_pull_main);
			if (ret != HD_OK) {
				printf("enc_release main error=%d !!\r\n", ret);
			}
		}


	} else {
		printf("unsupport format!\r\n");
	}

	//printf("uvc_video_fill_buffer---\r\n");
}

static int
uvc_video_process(struct uvc_device *dev)
{
	struct v4l2_buffer buf;
	int ret;

	#if PERF_TEST
	static VOS_TICK t1 = 0, t2 = 0, t3 = 0, temp = 0;
	UINT32 diff_us;
	static UINT32 cnt = 0;
	vos_perf_mark(&t1);
	if (cnt%13 == 0) {
		diff_us = vos_perf_duration(temp, t1);
		printf("q event diff=%d\r\n", diff_us/1000);
	}
	temp = t1;
	#endif

	memset(&buf, 0, sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	buf.memory = V4L2_MEMORY_MMAP;

	if ((ret = ioctl(dev->fd, VIDIOC_DQBUF, &buf)) < 0) {
		printf("Unable to dequeue buffer: %s (%d).\n", strerror(errno),
			errno);
		return ret;
	}

	uvc_video_fill_buffer(dev, &buf);
	#if PERF_TEST
	vos_perf_mark(&t2);
	#endif

	if ((ret = ioctl(dev->fd, VIDIOC_QBUF, &buf)) < 0) {
		printf("Unable to requeue buffer: %s (%d).\n", strerror(errno),
			errno);
		return ret;
	}
	else{
		//printf("uvc_video_process 001 \n");
	}
	#if PERF_TEST
	vos_perf_mark(&t3);
	if (cnt%15 == 0) {
		diff_us = vos_perf_duration(t1, t2);
		printf("%dKB t1=%d t2=%d t3=%d\r\n", buf.bytesused/1024, t1/1000, t2/1000, t3/1000);
	}
	cnt++;
	#endif
	return 0;
}

static int
uvc_video_reqbufs(struct uvc_device *dev, int nbufs)
{
	struct v4l2_requestbuffers rb;
	struct v4l2_buffer buf;
	unsigned int i;
	int ret;

	for (i = 0; i < dev->nbufs; ++i)
		munmap(dev->mem[i], dev->bufsize);

	free(dev->mem);
	dev->mem = 0;
	dev->nbufs = 0;

	memset(&rb, 0, sizeof rb);
	rb.count = nbufs;
	rb.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	rb.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(dev->fd, VIDIOC_REQBUFS, &rb);
	if (ret < 0) {
		printf("Unable to allocate buffers: %s (%d).\n",
			strerror(errno), errno);
		return ret;
	}

	printf("%u buffers allocated.\n", rb.count);

	/* Map the buffers. */
	dev->mem = malloc(rb.count * sizeof dev->mem[0]);

	for (i = 0; i < rb.count; ++i) {
		memset(&buf, 0, sizeof buf);
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(dev->fd, VIDIOC_QUERYBUF, &buf);
		if (ret < 0) {
			printf("Unable to query buffer %u: %s (%d).\n", i,
				strerror(errno), errno);
			return -1;
		}
		printf("length: %u offset: %u\n", buf.length, buf.m.offset);

		dev->mem[i] = mmap(0, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, dev->fd, buf.m.offset);
		if (dev->mem[i] == MAP_FAILED) {
			printf("Unable to map buffer %u: %s (%d)\n", i,
				strerror(errno), errno);
			return -1;
		}
		printf("Buffer %u mapped at address %p.\n", i, dev->mem[i]);
	}

	dev->bufsize = buf.length;
	dev->nbufs = rb.count;

	return 0;
}

static int
uvc_video_stream(struct uvc_device *dev, int enable)
{
	struct v4l2_buffer buf;
	unsigned int i;
	int type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	int ret = 0;

	if (!enable) {
		printf("Stopping video stream.\n");
		ioctl(dev->fd, VIDIOC_STREAMOFF, &type);
		return 0;
	}

	printf("Starting video stream.\n");

	for (i = 0; i < dev->nbufs; ++i) {
		memset(&buf, 0, sizeof buf);

		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf.memory = V4L2_MEMORY_MMAP;

		uvc_video_fill_buffer(dev, &buf);

		printf("B Queueing buffer %u.\n", i);
		if ((ret = ioctl(dev->fd, VIDIOC_QBUF, &buf)) < 0) {
			printf("Unable to queue buffer: %s (%d).\n",
				strerror(errno), errno);
			break;
		}
	}

	ioctl(dev->fd, VIDIOC_STREAMON, &type);
	return ret;
}

static int
uvc_video_set_format(struct uvc_device *dev)
{
	struct v4l2_format fmt;
	int ret;

	printf("Setting format to 0x%08x %ux%u\n",
		dev->fcc, dev->width, dev->height);

	memset(&fmt, 0, sizeof fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	fmt.fmt.pix.width = dev->width;
	fmt.fmt.pix.height = dev->height;
	fmt.fmt.pix.pixelformat = dev->fcc;
	fmt.fmt.pix.field = V4L2_FIELD_NONE;
	if (dev->fcc == V4L2_PIX_FMT_MJPEG)
		fmt.fmt.pix.sizeimage = MAX_BITSTREAM_LEN;//dev->imgsize * 1.5;
	if (dev->fcc == V4L2_PIX_FMT_H264)
		fmt.fmt.pix.sizeimage = MAX_BITSTREAM_LEN;//dev->imgsize * 1.5;

	if ((ret = ioctl(dev->fd, VIDIOC_S_FMT, &fmt)) < 0)
		printf("Unable to set format: %s (%d).\n",
			strerror(errno), errno);

	return ret;
}

static int
uvc_video_init(struct uvc_device *dev __attribute__((__unused__)))
{
	return 0;
}

/* ---------------------------------------------------------------------------
 * Request processing
 */

struct uvc_frame_info
{
	unsigned int width;
	unsigned int height;
	unsigned int intervals[8];
};

struct uvc_format_info
{
	unsigned int fcc;
	const struct uvc_frame_info *frames;
};

#if 0
static const struct uvc_frame_info uvc_frames_yuyv[] = {
	{  640, 480, { 333333, 416666, 500000, 666666, 1000000, 1333333, 2000000, 0 }, },
	{ 2304, 1536, { 4999998, 0 }, },
	{ 0, 0, { 0, }, },
};

static const struct uvc_frame_info uvc_frames_mjpeg[] = {
	{  640, 480, { 333333, 416666, 500000, 666666, 1000000, 1333333, 2000000, 0 }, },
	{ 1920,1080, { 333333, 416666, 500000, 666666, 1000000, 1333333, 2000000, 0 }, },
	{ 0, 0, { 0, }, },
};

static const struct uvc_frame_info uvc_frames_h264[] = {
	{  640, 480, { 333333, 416666, 500000, 666666, 1000000, 1333333, 2000000, 0 }, },
	{ 1920,1080, { 333333, 416666, 500000, 666666, 1000000, 1333333, 2000000, 0 }, },
	{ 3840,2160, { 333333, 416666, 500000, 666666, 1000000, 1333333, 2000000, 0 }, },
	{ 0, 0, { 0, }, },
};
//RichardLin 20190501 (H264)
static const struct uvc_format_info uvc_formats[] = {
	{ V4L2_PIX_FMT_YUYV, uvc_frames_yuyv },
	{ V4L2_PIX_FMT_H264, uvc_frames_h264 },
	{ V4L2_PIX_FMT_MJPEG, uvc_frames_mjpeg },
};
#else
static const struct uvc_frame_info uvc_frames_yuyv[] = {
	{  640, 360, { 333333, 416666, 500000, 0 }, },
	{ 1280, 720, { 500000, 50000000, 0 }, },
	{ 0, 0, { 0, }, },
};

static const struct uvc_frame_info uvc_frames_mjpeg[] = {
	{  640, 360, { 333333, 416666, 500000, 0 }, },
	{ 1280, 720, { 333333, 416666, 500000, 0 }, },
	{ 1920, 1080, { 333333, 416666, 500000, 0 }, },
	{ 0, 0, { 0, }, },
};
static const struct uvc_frame_info uvc_frames_h264[] = {
	{  640, 360, { 333333, 416666, 500000, 0 }, },
	{ 1280, 720, { 333333, 416666, 500000, 0 }, },
	{ 1920, 1080, { 333333, 416666, 500000, 0 }, },
	{ 0, 0, { 0, }, },
};
static const struct uvc_format_info uvc_formats[] = {
	{ V4L2_PIX_FMT_YUYV, uvc_frames_yuyv },
	{ V4L2_PIX_FMT_MJPEG, uvc_frames_mjpeg },
	{ V4L2_PIX_FMT_H264, uvc_frames_h264 },
};
#endif


static void
uvc_fill_streaming_control(struct uvc_device *dev,
			   struct uvc_streaming_control *ctrl,
			   int iframe, int iformat)
{
	const struct uvc_format_info *format;
	const struct uvc_frame_info *frame;
	unsigned int nframes;

	if (iformat < 0)
		iformat = ARRAY_SIZE(uvc_formats) + iformat;
	if (iformat < 0 || iformat >= (int)ARRAY_SIZE(uvc_formats))
		return;
	format = &uvc_formats[iformat];

	nframes = 0;
	while (format->frames[nframes].width != 0)
		++nframes;

	if (iframe < 0)
		iframe = nframes + iframe;
	if (iframe < 0 || iframe >= (int)nframes)
		return;
	frame = &format->frames[iframe];

	memset(ctrl, 0, sizeof *ctrl);

	ctrl->bmHint = 1;
	ctrl->bFormatIndex = iformat + 1;
	ctrl->bFrameIndex = iframe + 1;
	ctrl->dwFrameInterval = frame->intervals[0];
	switch (format->fcc) {
	case V4L2_PIX_FMT_YUYV:
		ctrl->dwMaxVideoFrameSize = 4915200;
		//ctrl->dwMaxPayloadTransferSize = 0;
		break;
	case V4L2_PIX_FMT_MJPEG:
		ctrl->dwMaxVideoFrameSize = 4915200;
		//ctrl->dwMaxPayloadTransferSize = 2048;
		break;
	case V4L2_PIX_FMT_H264:
		//ctrl->dwMaxVideoFrameSize = dev->imgsize* 100;
		//ctrl->dwMaxVideoFrameSize = dev->imgsize;
		ctrl->dwMaxVideoFrameSize = 4915200;
		//ctrl->dwMaxPayloadTransferSize = 2048;
		//min_loop2++;
		//ctrl->dwMaxVideoFrameSize = (dev->imgsize)>>(min_loop2);
		break;
	}
	ctrl->dwMaxPayloadTransferSize = 2048;//512;	/* TODO this should be filled by the driver. */
	ctrl->bmFramingInfo = 3;
	ctrl->bPreferedVersion = 1;
	ctrl->bMaxVersion = 1;
}

static void
uvc_events_process_standard(struct uvc_device *dev, struct usb_ctrlrequest *
ctrl,
			    struct uvc_request_data *resp)
{
	printf("standard request\n");
	(void)dev;
	(void)ctrl;
	(void)resp;
}

#define CAMERA_TERMINAL_CONTROL_SELECTORS    0x01
#define PROCESSING_UNIT_CONTROL_SELECTORS    0x02

#define CT_AE_MODE_CONTROL                   0x02
#define CT_AE_PRIORITY_CONTROL               0x03
#define CT_EXPOSURE_TIME_ABSOLUTE_CONTROL    0x04
#define CT_EXPOSURE_TIME_RELATIVE_CONTROL    0x05
#define CT_FOCUS_ABSOLUTE_CONTROL            0x06
#define CT_FOCUS_AUTO_CONTROL                0x08
#define CT_IRIS_ABSOLUTE_CONTROL             0x09
#define CT_ZOOM_ABSOLUTE_CONTROL             0x0B
#define CT_PANTILT_ABSOLUTE_CONTROL          0x0D
#define CT_ROLL_ABSOLUTE_CONTROL             0x0F

#define PU_BACKLIGHT_COMPENSATION_CONTROL    0x01
#define PU_BRIGHTNESS_CONTROL                0x02
#define PU_CONTRAST_CONTROL                  0x03
#define PU_GAIN_CONTROL                      0x04
#define PU_POWER_LINE_FREQUENCY_CONTROL      0x05
#define PU_HUE_CONTROL                       0x06
#define PU_SATURATION_CONTROL                0x07
#define PU_SHARPNESS_CONTROL                 0x08
#define PU_GAMMA_CONTROL                     0x09
#define PU_WHITE_BALANCE_TEMPERATURE_CONTROL 0x0A

static void
uvc_events_process_control(struct uvc_device *dev, uint8_t req, uint8_t cs, uint8_t index,
							struct usb_ctrlrequest *ctrl, struct uvc_request_data *resp)
{
	//printf("z uvc_events_process_control (req %02x cs %02x)\n", req, cs);


	(void)dev;
	//(void)resp;
	//resp->length = 1;
	switch (index){

		default:
			printf("[0x%02X][0x%02X] index not recognized.\n", index, cs);
			(void)resp;
	}
}

static void
uvc_events_process_streaming(struct uvc_device *dev, uint8_t req, uint8_t cs,
			     struct uvc_request_data *resp)
{
	struct uvc_streaming_control *ctrl;

	//printf("zz uvc_events_process_streaming() (req %02x cs %02x)\n", req, cs);

	if (cs != UVC_VS_PROBE_CONTROL && cs != UVC_VS_COMMIT_CONTROL)
		return;

	ctrl = (struct uvc_streaming_control *)&resp->data;
	resp->length = sizeof *ctrl;

	switch (req) {
	case UVC_SET_CUR:
		dev->control = cs;
		resp->length = 34;
		break;

	case UVC_GET_CUR:
		if (cs == UVC_VS_PROBE_CONTROL)
			memcpy(ctrl, &dev->probe, sizeof *ctrl);
		else
			memcpy(ctrl, &dev->commit, sizeof *ctrl);
		break;

	case UVC_GET_MIN:
	case UVC_GET_MAX:
	case UVC_GET_DEF:
		uvc_fill_streaming_control(dev, ctrl, req == UVC_GET_MAX ? -1 : 0,
					   req == UVC_GET_MAX ? -1 : 0);
		break;

	case UVC_GET_RES:
		memset(ctrl, 0, sizeof *ctrl);
		break;

	case UVC_GET_LEN:
		resp->data[0] = 0x00;
		resp->data[1] = 0x22;
		resp->length = 2;
		break;

	case UVC_GET_INFO:
		resp->data[0] = 0x03;
		resp->length = 1;
		break;
	}
}

static void
uvc_events_process_class(struct uvc_device *dev, struct usb_ctrlrequest *ctrl, struct uvc_request_data *resp)
{
	if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE)
		return;

	switch (ctrl->wIndex & 0xff) {
	case UVC_INTF_CONTROL:
		dev->type = UVC_INTF_CONTROL;
		uvc_events_process_control(dev, ctrl->bRequest, ctrl->wValue >> 8, ctrl->wIndex >> 8, ctrl, resp);
		break;

	case UVC_INTF_STREAMING:
		dev->type = UVC_INTF_STREAMING;
		uvc_events_process_streaming(dev, ctrl->bRequest, ctrl->wValue >> 8, resp);
		break;

	default:
		break;
	}
}

static void uvc_events_process_setup(struct uvc_device *dev, struct usb_ctrlrequest *ctrl, struct uvc_request_data *resp)
{
	dev->control = 0;
#if 0
	printf("uvc_events_process_setup bRequestType %02x bRequest %02x wValue %04x wIndex %04x wLength %04x\n",
					ctrl->bRequestType,
					ctrl->bRequest,
					ctrl->wValue,
					ctrl->wIndex,
					ctrl->wLength);
#endif
	switch (ctrl->bRequestType & USB_TYPE_MASK) {
	case USB_TYPE_STANDARD:
		printf("standard setup.\n");
		uvc_events_process_standard(dev, ctrl, resp);
		break;

	case USB_TYPE_CLASS:
		uvc_events_process_class(dev, ctrl, resp);
		break;

	default:
		break;
	}
}
static void uvc_events_process_data(struct uvc_device *dev, struct uvc_request_data *data)
{
	struct uvc_streaming_control *target;
	struct uvc_streaming_control *ctrl;
	const struct uvc_format_info *format;
	const struct uvc_frame_info *frame;
	const unsigned int *interval;
	unsigned int iformat, iframe;
	unsigned int nframes;

	switch (dev->control) {
	case UVC_VS_PROBE_CONTROL:
		printf("setting probe control, length = %d\n", data->length);
		target = &dev->probe;
		//DumpMem((unsigned int) &data->data, data->length, 16);

		break;

	case UVC_VS_COMMIT_CONTROL:
		printf("setting commit control, length = %d\n", data->length);
		target = &dev->commit;
		//DumpMem((unsigned int) &data->data, data->length, 16);
		break;

	default:
		printf("setting unknown control, length = %d\n", data->length);
		return;
	}

	ctrl = (struct uvc_streaming_control *)&data->data;
	iformat = clamp((unsigned int)ctrl->bFormatIndex, 1U,
			(unsigned int)ARRAY_SIZE(uvc_formats));
	format = &uvc_formats[iformat-1];

	nframes = 0;
	while (format->frames[nframes].width != 0)
		++nframes;

	iframe = clamp((unsigned int)ctrl->bFrameIndex, 1U, nframes);
	frame = &format->frames[iframe-1];
	interval = frame->intervals;

	while (interval[0] < ctrl->dwFrameInterval && interval[1])
		++interval;

	target->bFormatIndex = iformat;
	target->bFrameIndex = iframe;
	switch (format->fcc) {
	case V4L2_PIX_FMT_YUYV:
		target->dwMaxVideoFrameSize = frame->width * frame->height * 2;
		break;
	case V4L2_PIX_FMT_MJPEG:
		target->dwMaxVideoFrameSize = MAX_BITSTREAM_LEN;
		break;
	case V4L2_PIX_FMT_H264:
		target->dwMaxVideoFrameSize = MAX_BITSTREAM_LEN;
		break;
	default:
		printf("WARNING: unknow fmt(0x%X)\n", format->fcc);
		break;
	}
	target->dwFrameInterval = *interval;

printf("bFormatIndex %d, bFrameIndex %d, dwMaxVideoFrameSize %d, dwFrameInterval %d\n", target->bFormatIndex, target->bFrameIndex, target->dwMaxVideoFrameSize, target->dwFrameInterval);

	if (dev->control == UVC_VS_COMMIT_CONTROL) {
		dev->fcc = format->fcc;
		dev->width = frame->width;
		dev->height = frame->height;
		hd_video_stream_update(dev);
		uvc_video_set_format(dev);
		if (dev->bulk)
			uvc_video_stream(dev, 1);
	}
}
static void uvc_events_process(struct uvc_device *dev)
{
	struct v4l2_event v4l2_event;
	struct uvc_event *uvc_event = (void *)&v4l2_event.u.data;
	struct uvc_request_data resp;
	int ret;

	ret = ioctl(dev->fd, VIDIOC_DQEVENT, &v4l2_event);
	if (ret < 0) {
		printf("VIDIOC_DQEVENT failed: %s (%d)\n", strerror(errno),
			errno);
		return;
	}

	memset(&resp, 0, sizeof resp);
	resp.length = -EL2HLT;

	switch (v4l2_event.type) {
	case UVC_EVENT_CONNECT:
	case UVC_EVENT_DISCONNECT:
		return;

	case UVC_EVENT_SETUP:
		printf("event setup\r\n");
		uvc_events_process_setup(dev, &uvc_event->req, &resp);
if (resp.length>0) {
	//DumpMem((unsigned int) &resp.data[0], resp.length, 16);
}

		break;

	case UVC_EVENT_DATA:
		printf("event data\r\n");
		uvc_events_process_data(dev, &uvc_event->data);
		return;

	case UVC_EVENT_STREAMON:
		printf("stream on\r\n");
		uvc_video_reqbufs(dev, 4);
		uvc_video_stream(dev, 1);
		uvc_enable = 1;
		break;

	case UVC_EVENT_STREAMOFF:
		printf("stream off\r\n");
		uvc_video_stream(dev, 0);
		uvc_video_reqbufs(dev, 0);
		uvc_enable = 0;
		break;
	}

	ioctl(dev->fd, UVCIOC_SEND_RESPONSE, &resp);
	if (ret < 0) {
		printf("UVCIOC_S_EVENT failed: %s (%d)\n", strerror(errno),
			errno);
		return;
	}
}
static void uvc_events_init(struct uvc_device *dev)
{
	struct v4l2_event_subscription sub;

	uvc_fill_streaming_control(dev, &dev->probe, 0, 0);
	uvc_fill_streaming_control(dev, &dev->commit, 0, 0);

	if (dev->bulk) {
		/* FIXME Crude hack, must be negotiated with the driver. */
		dev->probe.dwMaxPayloadTransferSize = 16 * 1024;
		dev->commit.dwMaxPayloadTransferSize = 16 * 1024;
	}


	memset(&sub, 0, sizeof sub);
	sub.type = UVC_EVENT_SETUP;
	ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
	sub.type = UVC_EVENT_DATA;
	ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
	sub.type = UVC_EVENT_STREAMON;
	ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
	sub.type = UVC_EVENT_STREAMOFF;
	ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
}
static void *uvc_thread(void *arg)
{
	//VIDEO_STREAM* p_stream0 = (VIDEO_STREAM *)arg;
	char *device = "/dev/video0";
	struct uvc_device *dev;
	int bulk_mode = 0;
	fd_set fds;
	struct timeval tv;
	//int ret= 0;

#if 0
	usleep(2000000);
#endif
	uvc_enable = 0;

	dev = uvc_open(device);
	if (dev == NULL)
		return 0;

	dev->bulk = bulk_mode;

	uvc_events_init(dev);
	uvc_video_init(dev);

	while (1) {
		FD_ZERO(&fds);
		FD_SET(dev->fd, &fds);

		fd_set efds = fds;
		fd_set wfds = fds;

		//FD_ZERO(&efds);
		//FD_SET(dev->fd, &efds);
		//FD_ZERO(&wfds);
		//FD_SET(dev->fd, &wfds);

		/* Timeout. */
        tv.tv_sec = 2;
        tv.tv_usec = 0;

		//RichardLin
		//printf("select ++\n");
		select(dev->fd + 1, NULL, &wfds, &efds, &tv);
		//printf("ret=%d\n",ret);
		if (FD_ISSET(dev->fd, &efds)) {
			//printf("uvc_events_process\n");
			uvc_events_process(dev);
		}
		if (FD_ISSET(dev->fd, &wfds) && uvc_enable) {
			//printf("uvc_video_process\n");
			uvc_video_process(dev);
		}
		//printf("select --\n");
	}

	uvc_close(dev);
	return 0;
}


static void app_main(int argc, char* argv[])
{
	HD_RESULT ret;
	//INT key;

	UINT32 enc_type = HD_CODEC_TYPE_H264;
	HD_DIM main_dim;

	stream[0].codec_type = enc_type;
	// query program options
	if (argc > 1) {
		sensor_sel = atoi(argv[1]);
		printf("sensor_sel %d\r\n", sensor_sel);
	}

	if (sensor_sel == SEN_SEL_IMX290) {
		cap_size_w = VDO_SIZE_W_2M;
		cap_size_h = VDO_SIZE_H_2M;
	} else if (sensor_sel == SEN_SEL_OS05A10) {
		cap_size_w = VDO_SIZE_W_5M;
		cap_size_h = VDO_SIZE_H_5M;
	} else if (sensor_sel == SEN_SEL_OS02K10) {
		cap_size_w = VDO_SIZE_W_2M;
		cap_size_h = VDO_SIZE_H_2M;
	} else if (sensor_sel == SEN_SEL_IMX415) {
		cap_size_w = VDO_SIZE_W_8M;
		cap_size_h = VDO_SIZE_H_8M;
		sen1_vcap_id = 0;
	} else if (sensor_sel == SEN_SEL_PATGEN) {
		cap_size_w = VDO_SIZE_W_2M;
		cap_size_h = VDO_SIZE_H_2M;
		g_capbind = 0;
	} else {
		printf("Unsupported sensor!\n");
		return;
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

	// open VIDEO_STREAM modules (main)
	stream[0].proc_max_dim.w = MAX_CAP_SIZE_W; //assign by user
	stream[0].proc_max_dim.h = MAX_CAP_SIZE_H; //assign by user

	ret = open_module(&stream[0], &stream[0].proc_max_dim);
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

	// set videocap parameter
	stream[0].cap_dim.w = MAX_CAP_SIZE_W; //assign by user
	stream[0].cap_dim.h = MAX_CAP_SIZE_H; //assign by user
	ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim, MAX_BS_FPS, &stream[0].cap_dim);
	if (ret != HD_OK) {
		printf("set cap fail=%d\n", ret);
		goto exit;
	}

	// assign parameter by program options
	main_dim.w = MAX_BS_W;
	main_dim.h = MAX_BS_H;
	// set videoproc parameter (main)
	ret = set_proc_param(stream[0].proc_main_path, &main_dim, HD_VIDEO_PXLFMT_YUV420, 0, 1);
	if (ret != HD_OK) {
		printf("1111set proc fail=%d\n", ret);
		goto exit;
	}

	// set videoenc config (main), enc_cfg uses H264 to define bitstream buffer
	stream[0].enc_main_max_dim.w = MAX_BS_W;
	stream[0].enc_main_max_dim.h = MAX_BS_H;
	if ((MJPEG_TBR_MAX/(1024*1024))*UVC_MJPEG_ENC_BUF_MS(30) < ((H264_TBR_MAX)/(1024*1024))*UVC_H264_ENC_BUF_MS){	//H264 max bs buffer > MJPEG max bs buffer
		ret = set_enc_cfg(stream[0].enc_main_path, &stream[0].enc_main_max_dim, enc_type, H264_TBR_MAX);
	} else {		//MJPEG max bs buffer > H264 max bs buffer
		ret = set_enc_cfg(stream[0].enc_main_path, &stream[0].enc_main_max_dim, enc_type, (((MJPEG_TBR_MAX/(1024*1024))*UVC_MJPEG_ENC_BUF_MS(MAX_BS_FPS))/UVC_H264_ENC_BUF_MS)*1024*1024);
	}

	if (ret != HD_OK) {
		printf("set enc-cfg fail=%d\n", ret);
		goto exit;
	}

	// set videoenc parameter (main)
	stream[0].enc_main_dim.w = main_dim.w;
	stream[0].enc_main_dim.h = main_dim.h;
	ret = set_enc_param(stream[0].enc_main_path, &stream[0].enc_main_dim, enc_type, H264_TBR_MAX);
	if (ret != HD_OK) {
		printf("set enc fail=%d\n", ret);
		goto exit;
	}
	// bind VIDEO_STREAM modules (main)
	hd_videocap_bind(HD_VIDEOCAP_OUT(sen1_vcap_id, 0), HD_VIDEOPROC_0_IN_0);
	hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOENC_0_IN_0);

	// start VIDEO_STREAM modules (main)
	if (g_capbind == 1) {
		//direct NOTE: ensure videocap start after 1st videoproc phy path start
		hd_videoproc_start(stream[0].proc_main_path);
		hd_videocap_start(stream[0].cap_path);
	} else {
		hd_videocap_start(stream[0].cap_path);
		hd_videoproc_start(stream[0].proc_main_path);
	}

	// just wait ae/awb stable for auto-test, if don't care, user can remove it
	//sleep(1);
	hd_videoenc_start(stream[0].enc_main_path);

	hd_videoenc_get(stream[0].enc_main_path, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf_main);
	// mmap for bs buffer (just mmap one time only, calculate offset to virtual address later)
	vir_addr_main = (ULONG)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_main.buf_info.phy_addr, phy_buf_main.buf_info.buf_size);

#if 0
	// create encode_thread (pull_out bitstream)
	THREAD_CREATE(stream[0].enc_thread_id, encode_thread, (void *)stream, "encode_thread");
	if (0 == stream[0].enc_thread_id) {
		printf("create encode thread failed");
		goto exit;
	}
	THREAD_RESUME(stream[0].enc_thread_id);
#endif
#if 0//UVC_SUPPORT_YUV_FMT
	THREAD_CREATE(stream[0].acquire_thread_id, acquire_yuv_thread, (void *)stream, "acquire_yuv_thread");
	if (0 == stream[0].acquire_thread_id) {
		printf("create acquire thread failed");
		goto exit;
	}
	THREAD_RESUME(stream[0].acquire_thread_id);
#endif

	THREAD_CREATE(stream[0].uvc_thread_id, uvc_thread, (void *)stream, "uvc_thread");
	if (0 == stream[0].uvc_thread_id) {
		printf("create uvc thread failed");
		goto exit;
	}
	THREAD_RESUME(stream[0].uvc_thread_id);

	pthread_mutex_lock(&flow_start_lock);
	flow_audio_start = 0;
	encode_start = 0;
	acquire_start = 0;
	pthread_mutex_unlock(&flow_start_lock);

	// query user key
	printf("Enter q to exit\n");
#if 0
	while (1) {
		key = GETCHAR();
		if (key == 'q' || key == 0x3) {
			pthread_mutex_lock(&flow_start_lock);
			flow_audio_start = 0;
			encode_start = 0;
			acquire_start = 0;
			pthread_mutex_unlock(&flow_start_lock);
			// let encode_thread stop loop and exit
			stream[0].enc_exit = 1; //stop video


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
		if (key == '0') {
			get_cap_sysinfo(stream[0].cap_ctrl);
		}
		if (key == 'u') {
			uvc_debug_menu_entry_p(uvc_debug_menu, "UVAC DEBUG MAIN");
		}

	}
#else // Reduce CPU loading. 8x% ---> 1x%.
	while(1) {sleep(1);}
#endif

	if (vir_addr_main) {
		hd_common_mem_munmap((void *)vir_addr_main, phy_buf_main.buf_info.buf_size);
	}
	// destroy encode thread
	THREAD_DESTROY(stream[0].enc_thread_id);
#if UVC_SUPPORT_YUV_FMT
	THREAD_DESTROY(stream[0].acquire_thread_id);
#endif

	// stop VIDEO_STREAM modules (main)
	if (g_capbind == 1){
		hd_videoproc_stop(stream[0].proc_main_path);
		hd_videocap_stop(stream[0].cap_path);
	} else {
		hd_videocap_stop(stream[0].cap_path);
		hd_videoproc_stop(stream[0].proc_main_path);
	}
	hd_videoenc_stop(stream[0].enc_main_path);
	// unbind VIDEO_STREAM modules (main)
	hd_videocap_unbind(HD_VIDEOCAP_OUT(sen1_vcap_id, 0));
	hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_0);

exit:
	// close VIDEO_STREAM modules (main)
	ret = close_module(&stream[0]);
	if (ret != HD_OK) {
		printf("close fail=%d\n", ret);
	}

	// uninit all modules
	ret = exit_module();
	if (ret != HD_OK) {
		printf("exit fail=%d\n", ret);
	}

    ret = hd_gfx_uninit();
	if (ret != HD_OK) {
		printf("hd_gfx_uninit fail=%d\n", ret);
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

}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include "hdal.h"
#include <kwrap/examsys.h>
#include <kwrap/cmdsys.h>

static int check_if_multiple_instance(char *p_name)
{
	char buf[128];
	FILE *pp;

	snprintf(buf, sizeof(buf), "ps | grep %s | wc -l", p_name);
	if ((pp = popen(buf, "r")) == NULL) {
		printf("popen() error!\n");
		exit(1);
	}

	while (fgets(buf, sizeof buf, pp)) {
	}

	pclose(pp);

	if(atoi(buf) > 3) {
		return 1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	if (check_if_multiple_instance(argv[0]) == 1) {
		if (argc > 1) {
			nvt_cmdsys_ipc_cmd(argc, argv);
			return 0;
		}
		printf("%s has already in running. quit.\n", argv[0]);
		return -1;
	}

	nvt_cmdsys_init();      // command system
	nvt_examsys_init();     // exam system

	//start do your program
	app_main(argc, argv);
	return 0;
}
