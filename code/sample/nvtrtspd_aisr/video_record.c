/**
	@brief Sample code of video record.\n

	@file video_record.c

	@author Boyan Huang

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "hdal.h"
#include "hd_debug.h"
#include "nvtrtspd.h"

#include "hd_debug.h"
#include "vendor_isp.h"
#include "vendor_ai.h"
#include "vendor_ai_cpu/vendor_ai_cpu.h"
#include "ai_cpu_post_accuracy.h"

#include "vendor_videocapture.h"
#include "vendor_videoprocess.h"
#include "vendor_gfx.h"
#include "thermal_lib.h"
#include "auto_contrast_lib.h"

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
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(test_nvtrtspd, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define MIN(a, b)       ((a) <= (b) ? (a) : (b))
#define MAX(a, b)       ((a) >= (b) ? (a) : (b))
#define CLAMP(x, a, b)  MAX(MIN(x, b), a)

#define DEBUG_MENU 1

#define CHKPNT          printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
#define DBGH(x)         printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x)         printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)


typedef enum _THERMAL_DATA_ID {
	THERMAL_DATA_B = 0,
	THERMAL_DATA_K,
	THERMAL_DATA_NUM,
	ENUM_DUMMY4WORD(THERMAL_DATA_ID)
} THERMAL_DATA_ID;

#define THERMAL_DATA_FILE_B "/mnt/sd/thermal_op/B.bin"
#define THERMAL_DATA_FILE_K "/mnt/sd/thermal_op/K.bin"

#define OOC_MAX_BLK_NUM 2

#define GET_OOC_ID(x) ((x)&0xFF)
#define GET_CFG_ID(x) (((x)&0xFF00)>>8)
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

#define VPRC_PATH5_ENABLE          1   //must set to 1 when using ime sharpen

#define SEN_OUT_FMT     HD_VIDEO_PXLFMT_RAW12
#define CAP_OUT_FMT     (HD_VIDEO_PXLFMT_RAW16 | HD_VIDEO_PIX_PACK_LSB)
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


#define DROP_FRAME_COUNT   96//48//recommended by sensor FAE

#define CALIBRATION_PERIOD   (g_fps*180)// assume 180 sec

#define OOC_SIZE_W       654
#define OOC_SIZE_H       522
#define OOC_PIXEL_DEPTH  8//bit
#define OOC_PACK_BUS_SEL VENDOR_VCAP_OOC_PACKBUS_8
#define OOC_LOFS(w, pxl_bit)   (ALIGN_CEIL_4((w) * (pxl_bit) / 8))
#define OOC_BUF_SIZE     (OOC_LOFS(OOC_SIZE_W, OOC_PIXEL_DEPTH)*OOC_SIZE_H)

#define RAW_SIZE_W     676
#define RAW_SIZE_H     522

#if 0
#define VDO_SIZE_W      1920
#define VDO_SIZE_H      1080
#else
#define VDO_SIZE_W      RAW_SIZE_W
#define VDO_SIZE_H      RAW_SIZE_H
#endif

#define RAW_ACTIVE_X   (20+10)//(22+10)
#define RAW_ACTIVE_Y   8
#define RAW_ACTIVE_W   640
#define RAW_ACTIVE_H   512
///////////////////////////////////////////////////////////////////////////////
#define PHY2VIRT_MAIN(pa) (vir_addr_main + (pa - phy_buf_main.buf_info.phy_addr))
static uintptr_t vir_addr_main;
static HD_VIDEOENC_BUFINFO phy_buf_main;

#define CROP_SIZE_W      640
#define CROP_SIZE_H      512

#define OUT_SIZE_W      2560
#define OUT_SIZE_H      2048

#define VIDEOCAP_ALG_FUNC 0//HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB
#define VIDEOPROC_ALG_FUNC HD_VIDEOPROC_FUNC_AF | HD_VIDEOPROC_FUNC_WDR | HD_VIDEOPROC_FUNC_DEFOG | HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA

#define SEN_SEL_IRAY_6122C   0

static UINT32 resolution_set = SEN_SEL_IRAY_6122C;

static UINT32 g_fps = 25;

static UINT32 serial_id = 0;//temp solution for different config data

static UINT32 drop_cnt = DROP_FRAME_COUNT;

static UINT32 cnt_for_calibration = 0;



#define SEN1_VCAP_ID 4

#define DEBUG_SAVE_RAW 0
///////////////////////////////////////////////////////////////////////////////

#define SHUTTER_3V3_P  P_GPIO(10)
#define SHUTTER_3V3_N  P_GPIO(11)

#define P_GPIO(pin)                  (pin + 0x20)

int gpio_export(unsigned pin)
{
	char buffer[1024];
	snprintf(buffer, sizeof(buffer), "echo %d > /sys/class/gpio/export", pin);
	system(buffer);
	return 0;
}

int gpio_set_dir(unsigned pin,int dir)
{
	char buffer[1024];
	int fd;
	gpio_export(pin);
	snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(buffer, O_WRONLY);
	if (fd == -1) {
	    printf("GPIO path open error");
	    return -1;
	}
	if (dir == 0) {
	    write(fd, "in", 2);
	} else if (dir == 1) {
	    write(fd, "out", 3);
	}
	close(fd);
	return 0;
}

int gpio_set_value(unsigned pin, int value)
{
    char buffer[1024];
    int fd;
    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/value", pin);
    fd = open(buffer, O_WRONLY);
    if (fd == -1) {
        printf("Error : Unable to open gpio%d/value!!", pin);
        return -1;
    }
    if (value == 0 || value == 1) {
        char val = (value == 0) ? '0' : '1';
        if (write(fd, &val, 1) != 1) {
            printf("Error : Unable to write gpio %d to %d", pin, value);
            close(fd);
            return -1;
        }
    } else {
        printf("Error : Invalid value %d", value);
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}
static HD_RESULT mem_init(void)
{
	HD_RESULT              ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

	// config common pool (cap)
	mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[0].blk_size = DBGINFO_BUFSIZE()+VDO_RAW_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, CAP_OUT_FMT)
        													+VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
													+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
	mem_cfg.pool_info[0].blk_cnt = 2 + OOC_MAX_BLK_NUM + 4;
	mem_cfg.pool_info[0].ddr_id = DDR_ID0;
	// config common pool (main)
	mem_cfg.pool_info[1].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[1].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(CROP_SIZE_W, CROP_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[1].blk_cnt = 8;
	mem_cfg.pool_info[1].ddr_id = DDR_ID0;
	// config common pool (main)
	mem_cfg.pool_info[2].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[2].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(OUT_SIZE_W, OUT_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[2].blk_cnt = 8;
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

/*
static HD_RESULT get_cap_sysinfo(HD_PATH_ID video_cap_ctrl)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOCAP_SYSINFO sys_info = {0};

	hd_videocap_get(video_cap_ctrl, HD_VIDEOCAP_PARAM_SYSINFO, &sys_info);
	printf("sys_info.devid =0x%X, cur_fps[0]=%d/%d, vd_count=%llu\r\n", sys_info.dev_id, GET_HI_UINT16(sys_info.cur_fps[0]), GET_LO_UINT16(sys_info.cur_fps[0]), sys_info.vd_count);
	return ret;
}
*/

static HD_RESULT set_cap_cfg(HD_PATH_ID *p_video_cap_ctrl)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
	HD_PATH_ID video_cap_ctrl = 0;
	HD_VIDEOCAP_CTRL iq_ctl = {0};

	snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_rtd6122c");
	printf("Using nvt_sen_rtd6122c\n");
	cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_TDIO;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux =  0;  //use @0 in peri-dev.dtsi

	ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(SEN1_VCAP_ID), &video_cap_ctrl); //open this for device control

	if (ret != HD_OK) {
		return ret;
	}
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
	iq_ctl.func = VIDEOCAP_ALG_FUNC;

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
		video_in_param.frc = HD_VIDEO_FRC_RATIO(g_fps,1);
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
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);
		//printf("set_cap_param CROP NONE=%d\r\n", ret);
	}
	#else //HD_CROP_ON
	{
		HD_VIDEOCAP_CROP video_crop_param = {0};

		video_crop_param.mode = HD_CROP_ON;
		video_crop_param.win.rect.x = 640;
		video_crop_param.win.rect.y = 284;
		video_crop_param.win.rect.w = CROP_SIZE_W;
		video_crop_param.win.rect.h= CROP_SIZE_H;
		video_crop_param.align.w = 4;
		video_crop_param.align.h = 4;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);
		//printf("set_cap_param CROP ON=%d\r\n", ret);
	}
	#endif
	{
		HD_VIDEOCAP_OUT video_out_param = {0};

		//without setting dim for no scaling, using original sensor out size
		video_out_param.pxlfmt = CAP_OUT_FMT;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		video_out_param.depth = 1;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT, &video_out_param);
		//printf("set_cap_param OUT=%d\r\n", ret);
	}
	{
		HD_VIDEOCAP_FUNC_CONFIG video_path_param = {0};

		video_path_param.out_func = 0;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_FUNC_CONFIG, &video_path_param);
		//printf("set_cap_param PATH_CONFIG=0x%X\r\n", ret);
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
		video_cfg_param.pipe = HD_VIDEOPROC_PIPE_RAWALL;
		video_cfg_param.isp_id = SEN1_VCAP_ID;
		video_cfg_param.ctrl_max.func = VIDEOPROC_ALG_FUNC;
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
	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		video_path_param.in_func = 0;
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
		//printf("set_proc_param PATH_CONFIG=0x%X\r\n", ret);
	}
	{
		VENDOR_VIDEOPROC_THERMAL_INFO thermal_info = {0};
		printf("thermal_info.enable\n");
		thermal_info.enable = TRUE;
		ret = vendor_videoproc_set(video_proc_ctrl, VENDOR_VIDEOPROC_PARAM_THERMAL_INFO, (VOID *)&thermal_info);
	}


	video_ctrl_param.func = VIDEOPROC_ALG_FUNC;
	video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_0_OUT_4;
	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);

	video_ll_param.delay_trig_lowlatency = 0;
	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_LL_CONFIG, &video_ll_param);

	*p_video_proc_ctrl = video_proc_ctrl;

	return ret;
}

static HD_RESULT set_proc_param(HD_PATH_ID video_proc_path, HD_DIM* p_dim)
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
		video_out_param.depth = 1;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	}
	if(p_dim->w == CROP_SIZE_W)
	{
		HD_VIDEOPROC_CROP  video_in_param = {0};

		video_in_param.mode  = HD_CROP_ON;
		video_in_param.win.rect.x = RAW_ACTIVE_X;
		video_in_param.win.rect.y = RAW_ACTIVE_Y;
		video_in_param.win.rect.w = RAW_ACTIVE_W;
		video_in_param.win.rect.h = RAW_ACTIVE_H;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_IN_CROP, &video_in_param);
	}

	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		video_path_param.out_func = 0;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////

static HD_RESULT set_enc_cfg(HD_PATH_ID video_enc_path, HD_DIM *p_max_dim, UINT32 max_bitrate, UINT32 isp_id)
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
		video_path_config.isp_id             = isp_id;

		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_PATH_CONFIG, &video_path_config);
		if (ret != HD_OK) {
			printf("set_enc_path_config = %d\r\n", ret);
			return HD_ERR_NG;
		}

		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_FUNC_CONFIG, &video_func_config);
		if (ret != HD_OK) {
			printf("set_enc_func_config = %d\r\n", ret);
			return HD_ERR_NG;
		}
	}

	return ret;
}

static HD_RESULT set_enc_param(HD_PATH_ID video_enc_path, HD_DIM *p_dim, UINT32 enc_type, UINT32 bitrate)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOENC_IN  video_in_param = {0};
	HD_VIDEOENC_OUT2 video_out_param = {0};
	HD_H26XENC_RATE_CONTROL2 rc_param = {0};

	if (p_dim != NULL) {

		//--- HD_VIDEOENC_PARAM_IN ---
		video_in_param.dir           = HD_VIDEO_DIR_NONE;
		video_in_param.pxl_fmt = HD_VIDEO_PXLFMT_YUV420_NVX2;
		video_in_param.dim.w   = p_dim->w;
		video_in_param.dim.h   = p_dim->h;
		video_in_param.frc     = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_IN, &video_in_param);
		if (ret != HD_OK) {
			printf("set_enc_param_in = %d\r\n", ret);
			return ret;
		}

		printf("enc_type=%d\r\n", (int)enc_type);

		if (enc_type == 0) {

			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_H265;
			video_out_param.h26x.profile       = HD_H265E_MAIN_PROFILE;
			video_out_param.h26x.level_idc     = HD_H265E_LEVEL_5;
			video_out_param.h26x.gop_num       = 60;
			video_out_param.h26x.ltr_interval  = 0;
			video_out_param.h26x.ltr_pre_ref   = 0;
			video_out_param.h26x.gray_en       = 0;
			video_out_param.h26x.source_output = 0;
			video_out_param.h26x.svc_layer     = HD_SVC_DISABLE;
			video_out_param.h26x.entropy_mode  = HD_H265E_CABAC_CODING;
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM2, &video_out_param);
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
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL2, &rc_param);
			if (ret != HD_OK) {
				printf("set_enc_rate_control = %d\r\n", ret);
				return ret;
			}
		} else if (enc_type == 1) {

			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_H264;
			video_out_param.h26x.profile       = HD_H264E_HIGH_PROFILE;
			video_out_param.h26x.level_idc     = HD_H264E_LEVEL_5_1;
			video_out_param.h26x.gop_num       = 60;
			video_out_param.h26x.ltr_interval  = 0;
			video_out_param.h26x.ltr_pre_ref   = 0;
			video_out_param.h26x.gray_en       = 0;
			video_out_param.h26x.source_output = 0;
			video_out_param.h26x.svc_layer     = HD_SVC_DISABLE;
			video_out_param.h26x.entropy_mode  = HD_H264E_CABAC_CODING;
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM2, &video_out_param);
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
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL2, &rc_param);
			if (ret != HD_OK) {
				printf("set_enc_rate_control = %d\r\n", ret);
				return ret;
			}

		} else if (enc_type == 2) {

			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_JPEG;
			video_out_param.jpeg.retstart_interval = 0;
			video_out_param.jpeg.image_quality = 50;
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM2, &video_out_param);
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
	UINT32	cap_snap;

	// (2)
	HD_DIM  proc_max_dim;
	HD_VIDEOPROC_SYSCAPS proc_syscaps;
	HD_PATH_ID proc_ctrl;
	HD_PATH_ID proc_path;
	HD_PATH_ID proc_path2;
	#if (VPRC_PATH5_ENABLE)
	HD_PATH_ID proc_path_3dnr;
	#endif

	pthread_t  prc_thread_id;
	UINT32	prc_enter;
	UINT32	prc_exit;
	UINT32 	prc_count;
	UINT32 	prc_loop;

	HD_DIM  enc_max_dim;
	HD_DIM  enc_dim;

	// (3)
	HD_VIDEOENC_SYSCAPS enc_syscaps;
	HD_PATH_ID enc_path;

	// (4) user pull
	pthread_t  enc_thread_id;
	UINT32     enc_exit;
	UINT32     flow_start;


	HD_COMMON_MEM_VB_BLK thermal_nuc_out_blk;
	UINTPTR thermal_nuc_out_pa;
	UINTPTR thermal_nuc_out_va;

	HD_COMMON_MEM_VB_BLK thermal_k_frame_blk;
	UINTPTR thermal_k_frame_pa;
	UINTPTR thermal_k_frame_va;

	HD_VIDEO_FRAME thermal_b_frame;

	HD_COMMON_MEM_VB_BLK thermal_ooc_blk[OOC_MAX_BLK_NUM];
	UINTPTR thermal_ooc_pa[OOC_MAX_BLK_NUM];
	UINTPTR thermal_ooc_va[OOC_MAX_BLK_NUM];
	UINTPTR thermal_ooc_index;

	UINTPTR thermal_workbuf_pa;
	UINTPTR thermal_workbuf_va;
} VIDEO_RECORD,VIDEO_LIVEVIEW;

void save_videoframe(HD_VIDEO_FRAME *p_video_frame, char *file_path_main)
{
	FILE *f_out_main;
	UINTPTR phy_addr_main, vir_addr_main;
	UINT32 blk_size;

	blk_size = DBGINFO_BUFSIZE() + VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
									+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H)
									+ VDO_RAW_BUFSIZE(RAW_SIZE_W, RAW_SIZE_H, CAP_OUT_FMT);


	phy_addr_main = hd_common_mem_blk2pa(p_video_frame->blk); // Get physical addr
	if (phy_addr_main == 0) {
		printf("hd_common_mem_blk2pa error !!\r\n\r\n");
	}
	vir_addr_main = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_main, blk_size);
	if (vir_addr_main == 0) {
		printf("memory map error !!\r\n\r\n");
	}
	hd_common_mem_flush_cache((void *)vir_addr_main, blk_size);
	printf("fmt           resolution     lineoffset     PA             blk_size\r\n");
	printf("0x08%X    %4dx%4d      %4d           0x%08X     %d\r\n",p_video_frame->pxlfmt,
																		p_video_frame->dim.w,
																		p_video_frame->dim.h,
																		p_video_frame->loff[0],
																		p_video_frame->phy_addr[0],
																		blk_size);


	printf("dump raw data file (%s) ....\r\n", file_path_main);
	//----- open output files -----
	if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
		printf("open file (%s) fail....\r\n\r\n", file_path_main);
	}
	if (f_out_main) {
		UINT8 *ptr = (UINT8 *)(vir_addr_main + (p_video_frame->phy_addr[0] - phy_addr_main));
		UINT32 len = p_video_frame->loff[0]*p_video_frame->ph[0]*2;

		fwrite(ptr, 1, len, f_out_main);
		fflush(f_out_main);
	}
	// close output file
	fclose(f_out_main);
	hd_common_mem_munmap((void *)vir_addr_main, blk_size);
	printf("dump raw ok\r\n\r\n");
}
void refine_b_frame(HD_VIDEO_FRAME *p_video_frame)
{
	UINTPTR phy_addr_main, vir_addr_main;
	UINT32 blk_size, mean = 0;
	UINT64 sum = 0;
	UINT32 i, j;
	UINTPTR raw_va, tmp;

	blk_size = DBGINFO_BUFSIZE() + VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
									+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H)
									+ VDO_RAW_BUFSIZE(RAW_SIZE_W, RAW_SIZE_H, CAP_OUT_FMT);


	phy_addr_main = hd_common_mem_blk2pa(p_video_frame->blk); // Get physical addr
	if (phy_addr_main == 0) {
		printf("hd_common_mem_blk2pa error !!\r\n\r\n");
	}
	vir_addr_main = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_main, blk_size);
	if (vir_addr_main == 0) {
		printf("memory map error !!\r\n\r\n");
	}
	hd_common_mem_flush_cache((void *)vir_addr_main, blk_size);
	printf("fmt           resolution     lineoffset     PA             blk_size\r\n");
	printf("0x08%X    %4dx%4d      %4d           0x%08X     %d\r\n",p_video_frame->pxlfmt,
																		p_video_frame->dim.w,
																		p_video_frame->dim.h,
																		p_video_frame->loff[0],
																		p_video_frame->phy_addr[0],
																		blk_size);

	raw_va = vir_addr_main + (p_video_frame->phy_addr[0] - phy_addr_main);

	for (i = 0; i < RAW_ACTIVE_H; i++) {
		for (j = 0; j < RAW_ACTIVE_W; j++) {
			tmp = raw_va + p_video_frame->loff[0]*(i+RAW_ACTIVE_Y) + RAW_ACTIVE_X*2 + j*2;
			sum += (UINT64)(*(UINT16 *)tmp);
			#if 0
			if (i < 2) {
				printf("0x%X:%04X sum=0x%X\r\n", tmp-raw_va, *(UINT16 *)tmp, sum);
			}
			#endif
		}
	}
	mean = (UINT32)(sum/RAW_ACTIVE_H/RAW_ACTIVE_W);
	printf("sum = 0x%X, mean=%d\r\n", sum, mean);

	for (i = 0; i < RAW_ACTIVE_H; i++) {
		for (j = 0; j < RAW_ACTIVE_W; j++) {
			tmp = raw_va + p_video_frame->loff[0]*(i+RAW_ACTIVE_Y) + RAW_ACTIVE_X*2 + j*2;
			#if 1
			*(UINT16 *)tmp = *(UINT16 *)tmp - mean;
			#else
			if (*(UINT16 *)tmp > mean) {
				*(UINT16 *)tmp = *(UINT16 *)tmp - mean;
			} else {
				*(UINT16 *)tmp = 0;
			}
			#endif
		}
	}

	hd_common_mem_flush_cache((void *)vir_addr_main, blk_size);
	hd_common_mem_munmap((void *)vir_addr_main, blk_size);
}
HD_RESULT thermal_nu_correction(VIDEO_LIVEVIEW *p_stream, HD_VIDEO_FRAME* p_in_video_frame,  HD_VIDEO_FRAME* p_out_video_frame)
{
	VENDOR_GFX_GRPH_TRIGGER_PARAM  param = {0};
	HD_RESULT ret;
	UINTPTR src_pa[3], dst_pa;

	memcpy((void *)p_out_video_frame, (void *)p_in_video_frame, sizeof(HD_VIDEO_FRAME));

	src_pa[0] = p_in_video_frame->phy_addr[0];
	src_pa[1] = p_stream->thermal_k_frame_pa;
	src_pa[2] = p_stream->thermal_b_frame.phy_addr[0];
	dst_pa = p_stream->thermal_nuc_out_pa;

	//printf("src_pa 0x%lX 0x%lX 0x%lX  dst_pa 0x%lX\n", src_pa[0], src_pa[1], src_pa[2], dst_pa);

	param.command = VENDOR_GRPH_CMD_Non_uni_correction;
	param.format = VENDOR_GFX_GRPH_FORMAT_16BITS;
	param.property[0].en = 1;
	param.property[0].id = VENDOR_GFX_GRPH_PROPERTY_ID_NORMAL;
	param.property[0].data.property = 0;// divided by 2^n
	param.images[0].img_id = VENDOR_GFX_GRPH_IMG_ID_A;
	param.images[0].dram_addr = src_pa[0];
	param.images[0].lineoffset = RAW_SIZE_W*2;
	param.images[0].width = RAW_SIZE_W*2;
	param.images[0].height = RAW_SIZE_H;
	param.images[1].img_id = VENDOR_GFX_GRPH_IMG_ID_B;
	param.images[1].dram_addr = src_pa[1];
	param.images[1].lineoffset = RAW_SIZE_W*2;
	param.images[2].img_id = VENDOR_GFX_GRPH_IMG_ID_C;
	param.images[2].dram_addr = dst_pa;
	param.images[2].lineoffset = RAW_SIZE_W*2;
	param.images[3].img_id = VENDOR_GFX_GRPH_IMG_ID_D;
	param.images[3].dram_addr = src_pa[2];
	param.images[3].lineoffset = RAW_SIZE_W*2;

	//ret = vendor_gfx_grph_trigger_no_flush(1, &param);
	ret = vendor_gfx_grph_trigger(1, &param);
	if(ret != HD_OK){
		printf("vendor_gfx_grph_trigger fail=%d\n", ret);
	}
	p_out_video_frame->phy_addr[0] = dst_pa;
	p_out_video_frame->blk = p_stream->thermal_nuc_out_blk;

	//debug only
	if(p_stream->cap_snap) {
		char file_path_main[128];

		p_stream->cap_snap = 0;

		memset((void *)file_path_main, 0, sizeof(file_path_main));
		snprintf(file_path_main, sizeof(file_path_main), "/mnt/sd/tsen[%d]_nuc_in.raw", p_stream->cap_count);
		save_videoframe(p_in_video_frame, file_path_main);

		memset((void *)file_path_main, 0, sizeof(file_path_main));
		snprintf(file_path_main, sizeof(file_path_main), "/mnt/sd/tsen[%d]_nuc_out.raw", p_stream->cap_count);
		save_videoframe(p_out_video_frame, file_path_main);

		#if 0
		memset((void *)file_path_main, 0, sizeof(file_path_main));
		snprintf(file_path_main, sizeof(file_path_main), "/mnt/sd/tsen_nuc_b_%d.raw", p_stream->cap_count);
		save_videoframe(&p_stream->thermal_b_frame, file_path_main);

		{
			FILE *f_out_main;
			UINT32 blk_size = VDO_RAW_BUFSIZE(RAW_SIZE_W, RAW_SIZE_H, CAP_OUT_FMT);

			//hd_common_mem_flush_cache((void *)p_stream->thermal_k_frame_va, blk_size);
			memset((void *)file_path_main, 0, sizeof(file_path_main));
			snprintf(file_path_main, sizeof(file_path_main), "/mnt/sd/tsen_nuc_k_%d.raw", p_stream->cap_count);
			printf("dump nuc file (%s) ....\r\n", file_path_main);
			if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
				printf("open file (%s) fail....\r\n\r\n", file_path_main);
			}
			if (f_out_main) {
				UINT8 *ptr = (UINT8 *)p_stream->thermal_k_frame_va;
				UINT32 len = blk_size;

				fwrite(ptr, 1, len, f_out_main);
				fflush(f_out_main);
			}
			// close output file
			fclose(f_out_main);
		}
		#endif
	}

	return ret;
}
static HD_RESULT init_thermal_ooc_mem(VIDEO_LIVEVIEW *p_stream)
{
	UINT32 i;
	UINT32 blk_size = OOC_BUF_SIZE;

	for (i = 0; i < OOC_MAX_BLK_NUM; i++) {
		p_stream->thermal_ooc_blk[i] = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, blk_size, DDR_ID0); // Get block from mem pool
		if (p_stream->thermal_ooc_blk[i] == 0) {
			printf("get block[%d] fail (0x%x)\r\n", i, p_stream->thermal_ooc_blk[i]);
			return HD_ERR_NG;
		}

		p_stream->thermal_ooc_pa[i] = hd_common_mem_blk2pa(p_stream->thermal_ooc_blk[i]); // Get physical addr
		if (p_stream->thermal_ooc_pa[i] == 0) {
			printf("blk2pa fail, thermal_ooc_blk[%d] = 0x%x\r\n", i, p_stream->thermal_ooc_blk[i]);
			return HD_ERR_NG;
		}

		p_stream->thermal_ooc_va[i] = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, p_stream->thermal_ooc_pa[i], blk_size); // Get virtual addr
		if (p_stream->thermal_ooc_va[i] == 0) {
			printf("Error: mmap pa[%d] fail !\r\n", i);
			return HD_ERR_NG;
		}
	}

	return HD_OK;
}
static HD_RESULT exit_thermal_ooc_mem(VIDEO_LIVEVIEW *p_stream)
{
	UINT32 i;
	HD_RESULT ret = HD_OK;
	UINT32 blk_size = OOC_BUF_SIZE;

	for (i = 0; i < OOC_MAX_BLK_NUM; i++) {
		ret = hd_common_mem_munmap((void *)p_stream->thermal_ooc_va[i], blk_size);
		if (ret != HD_OK) {
			printf("mnumap ooc error !!\r\n\r\n");
   			return ret;
		}
		ret = hd_common_mem_release_block(p_stream->thermal_ooc_blk[i]);
		if (ret != HD_OK) {
			printf("_mem_release ooc error !!\r\n\r\n");
			return ret;
		}
	}
	return HD_OK;
}
static HD_RESULT init_thermal_lib(VIDEO_LIVEVIEW *p_stream)
{
	TSEN_RAW_INFO raw_info = {0};
	UINT32 work_buf_size;

	raw_info.sz_x = RAW_SIZE_W;
	raw_info.sz_y = RAW_SIZE_H;
	raw_info.depth = 2;
	raw_info.lofs = ALIGN_CEIL_4(raw_info.sz_x*2);
	work_buf_size = tsen_get_working_buf_size(IRAY_6122C_25HZ, &raw_info);
	if (hd_common_mem_alloc("tsen_work", (UINTPTR *)&p_stream->thermal_workbuf_pa, (void **)&p_stream->thermal_workbuf_va, work_buf_size, DDR_ID0) != HD_OK) {
		printf("%s mem alloc fail\r\n", __func__);
		return HD_ERR_NG;
	}
	printf("thermal_workbuf_pa=0x%lX, thermal_workbuf_va=0x%lX, size=%d\r\n", p_stream->thermal_workbuf_pa, p_stream->thermal_workbuf_va, work_buf_size);
	tsen_init(p_stream->thermal_workbuf_va, work_buf_size);

	return HD_OK;
}
static HD_RESULT exit_thermal_lib(VIDEO_LIVEVIEW *p_stream)
{
	if (p_stream->thermal_workbuf_pa && p_stream->thermal_workbuf_va) {
		if (hd_common_mem_free((UINTPTR)p_stream->thermal_workbuf_pa, (void *)p_stream->thermal_workbuf_va) != HD_OK) {
			printf("%s mem free fail\r\n", __func__);
			return HD_ERR_NG;
		}
	}
	p_stream->thermal_workbuf_pa = 0;
	p_stream->thermal_workbuf_va = 0;
	return HD_OK;
}
static HD_RESULT init_thermal_frame(VIDEO_LIVEVIEW *p_stream)
{
	UINT32 i;
	UINT32 blk_size = VDO_RAW_BUFSIZE(RAW_SIZE_W, RAW_SIZE_H, CAP_OUT_FMT);


	//init k frame
	p_stream->thermal_k_frame_blk = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, blk_size, DDR_ID0); // Get block from mem pool
	if (p_stream->thermal_k_frame_blk == 0) {
		printf("get block fail (0x%x)\r\n", p_stream->thermal_k_frame_blk);
		return HD_ERR_NG;
	}

	p_stream->thermal_k_frame_pa = hd_common_mem_blk2pa(p_stream->thermal_k_frame_blk); // Get physical addr
	if (p_stream->thermal_k_frame_pa == 0) {
		printf("blk2pa fail, thermal_k_frame_pa = 0x%x\r\n", p_stream->thermal_k_frame_blk);
		return HD_ERR_NG;
	}

	p_stream->thermal_k_frame_va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, p_stream->thermal_k_frame_pa, blk_size); // Get virtual addr
	if (p_stream->thermal_k_frame_va == 0) {
		printf("Error: mmap pa fail !\r\n");
		return HD_ERR_NG;
	}

	//temporarily set to 0x1
	for (i = 0; i < blk_size; i += 4) {
		*(UINT32 *)(p_stream->thermal_k_frame_va + i) = 0x00010001;
	}
	hd_common_mem_flush_cache((void *)p_stream->thermal_k_frame_va, blk_size);

	//init nuc out frame
	p_stream->thermal_nuc_out_blk = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, blk_size, DDR_ID0); // Get block from mem pool
	if (p_stream->thermal_nuc_out_blk == 0) {
		printf("get out block fail (0x%x)\r\n", p_stream->thermal_nuc_out_blk);
		return HD_ERR_NG;
	}

	p_stream->thermal_nuc_out_pa = hd_common_mem_blk2pa(p_stream->thermal_nuc_out_blk); // Get physical addr
	if (p_stream->thermal_nuc_out_pa == 0) {
		printf("blk2pa fail, thermal_nuc_out_blk = 0x%x\r\n", i, p_stream->thermal_nuc_out_blk);
		return HD_ERR_NG;
	}

	p_stream->thermal_nuc_out_va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, p_stream->thermal_nuc_out_pa, blk_size); // Get virtual addr
	if (p_stream->thermal_nuc_out_va == 0) {
		printf("Error: mmap out pa fail !\r\n");
		return HD_ERR_NG;
	}
	hd_common_mem_flush_cache((void *)p_stream->thermal_nuc_out_va, blk_size);


	return HD_OK;
}
static HD_RESULT exit_thermal_frame(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret = HD_OK;
	UINT32 blk_size = VDO_RAW_BUFSIZE(RAW_SIZE_W, RAW_SIZE_H, CAP_OUT_FMT);

	ret = hd_common_mem_munmap((void *)p_stream->thermal_k_frame_va, blk_size);
	if (ret != HD_OK) {
		printf("mnumap data error !!\r\n\r\n");
			return ret;
	}

	ret = hd_common_mem_munmap((void *)p_stream->thermal_nuc_out_va, blk_size);
	if (ret != HD_OK) {
		printf("mnumap out error !!\r\n\r\n");
		return ret;
	}

	ret = hd_common_mem_release_block(p_stream->thermal_k_frame_blk);
	if (ret != HD_OK) {
		printf("_mem_release error !!\r\n\r\n");
		return ret;
	}

	ret = hd_common_mem_release_block(p_stream->thermal_nuc_out_blk);
	if (ret != HD_OK) {
		printf("_mem_release out error !!\r\n\r\n");
		return ret;
	}

	//relesae b frame
	if (p_stream->thermal_b_frame.blk) {
		ret = hd_videocap_release_out_buf(p_stream->cap_path, &p_stream->thermal_b_frame);
		if (ret != HD_OK) {
			printf("exit_thermal_frame cap_release error=%d !!\r\n\r\n", ret);
		}
	}
	return HD_OK;
}


#if 0
static HD_RESULT init_thermal_data(VIDEO_LIVEVIEW *p_stream)
{
	char filepath_thermal_data[128];
	UINT32 i;
	FILE *f_thermal;
	UINT32 read_len;
	UINT32 blk_size = VDO_RAW_BUFSIZE(RAW_SIZE_W, RAW_SIZE_H, CAP_OUT_FMT);

	for (i = 0; i < THERMAL_DATA_NUM; i++) {
		p_stream->thermal_data_blk[i] = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, blk_size, DDR_ID0); // Get block from mem pool
		if (p_stream->thermal_data_blk[i] == 0) {
			printf("get block[%d] fail (0x%x)\r\n", i, p_stream->thermal_data_blk[i]);
			return HD_ERR_NG;
		}

		p_stream->thermal_data_pa[i] = hd_common_mem_blk2pa(p_stream->thermal_data_blk[i]); // Get physical addr
		if (p_stream->thermal_data_pa[i] == 0) {
			printf("blk2pa fail, thermal_data_blk[%d] = 0x%x\r\n", i, p_stream->thermal_data_blk[i]);
			return HD_ERR_NG;
		}

		p_stream->thermal_data_va[i] = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, p_stream->thermal_data_pa[i], blk_size); // Get virtual addr
		if (p_stream->thermal_data_va[i] == 0) {
			printf("Error: mmap pa[%d] fail !\r\n", i);
			return HD_ERR_NG;
		}
	}

	p_stream->thermal_nuc_out_blk = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, blk_size, DDR_ID0); // Get block from mem pool
	if (p_stream->thermal_nuc_out_blk == 0) {
		printf("get out block fail (0x%x)\r\n", p_stream->thermal_nuc_out_blk);
		return HD_ERR_NG;
	}

	p_stream->thermal_nuc_out_pa = hd_common_mem_blk2pa(p_stream->thermal_nuc_out_blk); // Get physical addr
	if (p_stream->thermal_nuc_out_pa == 0) {
		printf("blk2pa fail, thermal_nuc_out_blk = 0x%x\r\n", i, p_stream->thermal_nuc_out_blk);
		return HD_ERR_NG;
	}

	p_stream->thermal_nuc_out_va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, p_stream->thermal_nuc_out_pa, blk_size); // Get virtual addr
	if (p_stream->thermal_nuc_out_va == 0) {
		printf("Error: mmap out pa fail !\r\n");
		return HD_ERR_NG;
	}

	//load thermal data B
	sprintf(filepath_thermal_data, THERMAL_DATA_FILE_B);
	if ((f_thermal = fopen(filepath_thermal_data, "rb")) == NULL) {
		printf("open file (%s) fail !\r\n\r\n", filepath_thermal_data);
		return HD_ERR_NG;
	}
#if 0
	read_len = fread((void *)p_stream->thermal_data_va[THERMAL_DATA_B], 1, blk_size, f_thermal);
	#else//debug only
	read_len = blk_size;
	memset((void *)p_stream->thermal_data_va[THERMAL_DATA_B], 0, read_len);
	#endif
	if (read_len != blk_size) {
		printf("reading len error\n");
		fclose(f_thermal);
		return HD_ERR_NG;
	}

	//--- data is written by CPU, flush CPU cache to PHY memory ---
	hd_common_mem_flush_cache((void *)p_stream->thermal_data_va[THERMAL_DATA_B], blk_size);
	fclose(f_thermal);

	//load thermal data K
	sprintf(filepath_thermal_data, THERMAL_DATA_FILE_K);
	if ((f_thermal = fopen(filepath_thermal_data, "rb")) == NULL) {
		printf("open file (%s) fail !\r\n\r\n", filepath_thermal_data);
		return HD_ERR_NG;
	}
#if 1
	read_len = fread((void *)p_stream->thermal_data_va[THERMAL_DATA_K], 1, blk_size, f_thermal);
	#else//debug only
	read_len = blk_size;
	memset((void *)p_stream->thermal_data_va[THERMAL_DATA_K], 1, read_len);
	#endif
	if (read_len != blk_size) {
		printf("reading len error\n");
		fclose(f_thermal);
		return HD_ERR_NG;
	}
	//--- data is written by CPU, flush CPU cache to PHY memory ---
	hd_common_mem_flush_cache((void *)p_stream->thermal_data_va[THERMAL_DATA_K], blk_size);
	fclose(f_thermal);

	return HD_OK;
}
static HD_RESULT exit_thermal_data(VIDEO_LIVEVIEW *p_stream)
{
	UINT32 i;
	HD_RESULT ret = HD_OK;
	UINT32 blk_size = VDO_RAW_BUFSIZE(RAW_SIZE_W, RAW_SIZE_H, CAP_OUT_FMT);

	for (i = 0; i < THERMAL_DATA_NUM; i++) {
		ret = hd_common_mem_munmap((void *)p_stream->thermal_data_va[i], blk_size);
		if (ret != HD_OK) {
			printf("mnumap data error !!\r\n\r\n");
   			return ret;
		}
	}
	ret = hd_common_mem_munmap((void *)p_stream->thermal_nuc_out_va, blk_size);
	if (ret != HD_OK) {
		printf("mnumap out error !!\r\n\r\n");
		return ret;
	}

	for (i = 0; i < THERMAL_DATA_NUM; i++) {
		ret = hd_common_mem_release_block(p_stream->thermal_data_blk[i]);
		if (ret != HD_OK) {
			printf("_mem_release error !!\r\n\r\n");
			return ret;
		}
	}
	ret = hd_common_mem_release_block(p_stream->thermal_nuc_out_blk);
	if (ret != HD_OK) {
		printf("_mem_release out error !!\r\n\r\n");
		return ret;
	}

	return HD_OK;
}
#endif
static void get_ooc_buffer(VIDEO_LIVEVIEW *p_stream, UINTPTR *p_va, UINTPTR *p_pa)
{
	*p_va = p_stream->thermal_ooc_va[p_stream->thermal_ooc_index];
	*p_pa = p_stream->thermal_ooc_pa[p_stream->thermal_ooc_index];
	p_stream->thermal_ooc_index++;
	if (p_stream->thermal_ooc_index >= OOC_MAX_BLK_NUM) {
		p_stream->thermal_ooc_index = 0;
	}
}
static HD_RESULT init_thermal_cfg_ooc(VIDEO_LIVEVIEW *p_stream)
{
	TSEN_CONFIG config = {0};
	TSEN_OOC_INFO ooc_info = {0};
	VENDOR_VCAP_TSEN_CFG tsen_cfg = {0};
	VENDOR_VCAP_TSEN_OOC tsen_ooc = {0};
	UINTPTR ooc_va, ooc_pa;
	HD_RESULT ret = HD_OK;

	tsen_get_init_config(&config, serial_id);

	tsen_cfg.len = config.len;
	tsen_cfg.id = config.id;
	memcpy(tsen_cfg.tx_data, config.tx_data, sizeof(tsen_cfg.tx_data));
	ret = vendor_videocap_set(p_stream->cap_path, VENDOR_VIDEOCAP_PARAM_TSEN_CFG, &tsen_cfg);
	if (ret) {
		return ret;
	}

	get_ooc_buffer(p_stream, &ooc_va, &ooc_pa);

	ooc_info.addr = ooc_va;
	ooc_info.pack_bus_sel = OOC_PACK_BUS_SEL;//TSEN_OOC_PACKBUS_8;
	ooc_info.id = 0;
	ooc_info.width = OOC_SIZE_W;
	ooc_info.height = OOC_SIZE_H;
	ooc_info.lofs = OOC_LOFS(ooc_info.width, OOC_PIXEL_DEPTH);
	tsen_get_init_ooc(&ooc_info);
	#if DEBUG_SAVE_RAW
	//save init ooc
	{
		char file_path_main[128] = {0};
		FILE *f_out_main;

		snprintf(file_path_main, sizeof(file_path_main), "/mnt/sd/tsen_init_ooc.raw");
		printf("dump ooc file (%s) ....\r\n", file_path_main);

		if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
			printf("open file (%s) fail....\r\n\r\n", file_path_main);
		}
		if (f_out_main) {
			UINT8 *ptr = (UINT8 *)ooc_va;
			UINT32 len = OOC_BUF_SIZE;

			fwrite(ptr, 1, len, f_out_main);
			fflush(f_out_main);
		}
		// close output file
		fclose(f_out_main);
	}
	#endif

	hd_common_mem_flush_cache((void *)ooc_va, OOC_BUF_SIZE);
	tsen_ooc.buf_pa = ooc_pa;
	tsen_ooc.pack_bus_sel = OOC_PACK_BUS_SEL;
	tsen_ooc.lofs = OOC_LOFS(ooc_info.width, OOC_PIXEL_DEPTH);
	tsen_ooc.id = ooc_info.id;
	ret = vendor_videocap_set(p_stream->cap_path, VENDOR_VIDEOCAP_PARAM_TSEN_OOC, &tsen_ooc);

	return ret;
}
#define PHY2VIRT_EXE(pa) (vir_addr_main + ((pa) - phy_addr_main))
#define CALIBRATION_MAX_RETRY 1000

#define TEMP_SOLUTION_FOR_OOC_ID 0
#if 0
void DumpMem(ULONG Addr, UINT32 Size, UINT32 Alignment)
{
	UINT32 i;
	UINT16 *pBuf = (UINT16 *)Addr;
	for (i = 0; i < Size/2; i++) {
		printf("%d ", *(pBuf + i));
	}
	printf("\r\n");
}
#else
void DumpMem(ULONG Addr, UINT32 Size, UINT32 Alignment)
{
	UINT32 i;
	UINT32 *pBuf = (UINT32 *)Addr;
	for (i = 0; i < Size/4; i++) {
		printf("0x%08X ", *(pBuf + i));
	}
	printf("\r\n\r\n");
}
#endif

#if DEBUG_SAVE_RAW
void save_vcap_raw(VIDEO_LIVEVIEW *p_stream0, UINT32 cnt)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEO_FRAME video_frame = {0};
	char file_path_main[128] = {0};
	FILE *f_out_main;
	UINTPTR phy_addr_main, vir_addr_main;
	static UINT32 shot_count = 0;
	UINT32 i;
	UINT32 blk_size;
	#define PA2VA(pa) (vir_addr_main + ((pa) - phy_addr_main))

	blk_size = DBGINFO_BUFSIZE() + VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
									+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H)
									+ VDO_RAW_BUFSIZE(RAW_SIZE_W, RAW_SIZE_H, CAP_OUT_FMT);

	for (i = 0; i < cnt; i++) {
		ret = hd_videocap_pull_out_buf(p_stream0->cap_path, &video_frame, -1);// -1 = blocking mode, 0 = non-blocking mode, >0 = blocking-timeout mode

		phy_addr_main = hd_common_mem_blk2pa(video_frame.blk); // Get physical addr
		if (phy_addr_main == 0) {
			printf("hd_common_mem_blk2pa error !!\r\n\r\n");
			goto release_out;
		}
		vir_addr_main = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_main, blk_size);
		if (vir_addr_main == 0) {
			printf("memory map error !!\r\n\r\n");
			goto release_out;
		}
		hd_common_mem_flush_cache((void *)vir_addr_main, blk_size);
		printf("fmt           resolution     lineoffset     PA             blk_size\r\n");
		printf("0x08%X    %4dx%4d      %4d           0x%08X     %d\r\n",video_frame.pxlfmt,
																			video_frame.dim.w,
																			video_frame.dim.h,
																			video_frame.loff[0],
																			video_frame.phy_addr[0],
																			blk_size);


		snprintf(file_path_main, sizeof(file_path_main), "/mnt/sd/tsen_vdocap0_%lu.raw", (ULONG)shot_count);
		printf("dump raw data file (%s) ....\r\n", file_path_main);
		//----- open output files -----
		if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
			printf("open file (%s) fail....\r\n\r\n", file_path_main);
		}
		if (f_out_main) {
			UINT8 *ptr = (UINT8 *)PA2VA(video_frame.phy_addr[0]);
			UINT32 len = video_frame.loff[0]*video_frame.ph[0]*2;

			DumpMem((ULONG)ptr, 20*2, 16);

			fwrite(ptr, 1, len, f_out_main);
			fflush(f_out_main);
		}
		// close output file
		fclose(f_out_main);
		hd_common_mem_munmap((void *)vir_addr_main, blk_size);

		printf("dump raw ok\r\n\r\n");
		shot_count ++;
		release_out:
		ret = hd_videocap_release_out_buf(p_stream0->cap_path, &video_frame);
		printf("cap_released ....\r\n");
		if (ret != HD_OK) {
			printf("cap_release error !!\r\n\r\n");
		}
	}
}
#endif


static void shutter_control(BOOL open)
{
	if (open) {
		printf("open shutter\r\n");
		gpio_set_value(SHUTTER_3V3_P, 1);
		usleep(20000);
		gpio_set_value(SHUTTER_3V3_P, 0);
	} else {
		printf("close shutter\r\n");
		gpio_set_value(SHUTTER_3V3_N, 1);
		usleep(20000);
		gpio_set_value(SHUTTER_3V3_N, 0);
	}
}

static void skip_vcap_frame(VIDEO_LIVEVIEW *p_stream0, UINT32 skip_cnt)
{
	UINT32 i;
	HD_VIDEO_FRAME vcap_video_frame = {0};
	HD_RESULT ret = HD_OK;

	for (i = 0; i < skip_cnt; i++) {
		ret = hd_videocap_pull_out_buf(p_stream0->cap_path, &vcap_video_frame, -1); // -1 = blocking mode
		if (ret != HD_OK) {
			printf("skip_vcap_frame vcap pull error=%d !!\r\n\r\n", ret);
		}
		ret = hd_videocap_release_out_buf(p_stream0->cap_path, &vcap_video_frame);
		if (ret != HD_OK) {
			printf("skip_vcap_frame cap_release error=%d !!\r\n\r\n", ret);
		}
	}
}
HD_RESULT thermal_calibration(VIDEO_LIVEVIEW *p_stream)
{
	TSEN_CONFIG config = {0};
	TSEN_OOC_INFO ooc_info = {0};
	VENDOR_VCAP_TSEN_CFG tsen_cfg = {0};
	VENDOR_VCAP_TSEN_OOC tsen_ooc = {0};
	UINTPTR ooc_va, ooc_pa;
	HD_RESULT ret = HD_OK;
	HD_VIDEO_FRAME vcap_video_frame = {0};
	UINTPTR phy_addr_main, vir_addr_main;
	UINT32 blk_size;
	UINT32 update_result = 0;
	UINT32 b_frame_ooc_id = 0, b_frame_cfg_id = 0;
	UINT32 i;
	#if TEMP_SOLUTION_FOR_OOC_ID
	static UINT32 targe_ooc_id = TSEN_RESTART_CALIBRATION, target_cfg_id = TSEN_RESTART_CALIBRATION;
	#endif

	blk_size = DBGINFO_BUFSIZE()+ VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
								+ VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H)
								+ VDO_RAW_BUFSIZE(RAW_SIZE_W, RAW_SIZE_H, CAP_OUT_FMT);


	//close shutter
	shutter_control(FALSE);

	for (i = 0; i < CALIBRATION_MAX_RETRY; i++) {
		ret = hd_videocap_pull_out_buf(p_stream->cap_path, &vcap_video_frame, -1); // -1 = blocking mode
		if (ret != HD_OK) {
			printf("cap_pull error=%d !!\r\n\r\n", ret);
			return ret;
		}
		printf("frame[%llu] OOC ID=%d CFG ID=%d\r\n", vcap_video_frame.count, GET_OOC_ID(vcap_video_frame.reserved[2]), GET_CFG_ID(vcap_video_frame.reserved[2]));
		#if DEBUG_SAVE_RAW
		//save raw
		if (i != 0) {
			char file_path_main[128] = {0};

			snprintf(file_path_main, sizeof(file_path_main), "/mnt/sd/tsen[%d]_frame_%d.raw", p_stream->cap_count, targe_ooc_id);
			save_videoframe(&vcap_video_frame, file_path_main);
		}
		#endif


		phy_addr_main = hd_common_mem_blk2pa(vcap_video_frame.blk); // Get physical addr
		if (phy_addr_main == 0) {
			printf("hd_common_mem_blk2pa error !!\r\n\r\n");
			hd_videocap_release_out_buf(p_stream->cap_path, &vcap_video_frame);

		}
		vir_addr_main = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_main, blk_size);
		if (vir_addr_main == 0) {
			printf("memory map error !!\r\n\r\n");
			hd_videocap_release_out_buf(p_stream->cap_path, &vcap_video_frame);
		}

		hd_common_mem_flush_cache((void *)vir_addr_main, blk_size);

		if (i == 0) {
			ooc_info.id = TSEN_RESTART_CALIBRATION;
			config.id  = TSEN_RESTART_CALIBRATION;
		} else {
			#if TEMP_SOLUTION_FOR_OOC_ID
			ooc_info.id = targe_ooc_id;
			config.id = target_cfg_id;
			#else
			ooc_info.id =  GET_OOC_ID(vcap_video_frame.reserved[2]);
			config.id = GET_CFG_ID(vcap_video_frame.reserved[2]);
			#endif
		}
		get_ooc_buffer(p_stream, &ooc_va, &ooc_pa);
		ooc_info.addr = ooc_va;
		ooc_info.pack_bus_sel = OOC_PACK_BUS_SEL;//TSEN_OOC_PACKBUS_8;
		ooc_info.width = OOC_SIZE_W;
		ooc_info.height = OOC_SIZE_H;
		ooc_info.lofs = OOC_LOFS(ooc_info.width, OOC_PIXEL_DEPTH);

#if 0
		printf("raw_va=0x%lX ooc_info va=0x%lX pack_bus_sel(%d) w,h,lof(%d,%d,%d) id(%d), config_id(%d)\r\n", PHY2VIRT_EXE(vcap_video_frame.phy_addr[0]),
																				ooc_info.addr, ooc_info.pack_bus_sel,
																				ooc_info.width, ooc_info.height, ooc_info.lofs, ooc_info.id, config.id);
#endif
		update_result = tsen_update_params(PHY2VIRT_EXE(vcap_video_frame.phy_addr[0]), &ooc_info, &config);

#if 0
		printf("update_result(0x%X) ooc_id(%d) config_id(%d)\r\n", update_result, ooc_info.id, config.id);
#endif

		if (update_result) {
			//just a debug log to check SYNC code
			DumpMem(PHY2VIRT_EXE(vcap_video_frame.phy_addr[0]), 22*2, 16);
		}

		hd_common_mem_munmap((void *)vir_addr_main, blk_size);
		ret = hd_videocap_release_out_buf(p_stream->cap_path, &vcap_video_frame);
		//printf("cap_released ....\r\n");
		if (ret != HD_OK) {
			printf("cap_release error !!\r\n\r\n");
		}

		if (update_result & UPDATE_CONFIG) {
		    tsen_cfg.len = config.len;
			tsen_cfg.id = config.id;
			memcpy(tsen_cfg.tx_data, config.tx_data, sizeof(tsen_cfg.tx_data));
			ret = vendor_videocap_set(p_stream->cap_path, VENDOR_VIDEOCAP_PARAM_TSEN_CFG, &tsen_cfg);
			if (ret) {
				printf("VENDOR_VIDEOCAP_PARAM_TSEN_CFG failed!\r\n\r\n");
			}
			#if TEMP_SOLUTION_FOR_OOC_ID
			target_cfg_id = config.id;
			#endif
		}
		if (update_result & UPDATE_OOC) {
			hd_common_mem_flush_cache((void *)ooc_va, OOC_BUF_SIZE);
		    tsen_ooc.buf_pa = ooc_pa;
			tsen_ooc.pack_bus_sel = OOC_PACK_BUS_SEL;
			tsen_ooc.lofs = OOC_LOFS(ooc_info.width, OOC_PIXEL_DEPTH);
			tsen_ooc.id = ooc_info.id;
			ret = vendor_videocap_set(p_stream->cap_path, VENDOR_VIDEOCAP_PARAM_TSEN_OOC, &tsen_ooc);
			if (ret) {
				printf("VENDOR_VIDEOCAP_PARAM_TSEN_OOC failed!\r\n\r\n");
			}
			#if TEMP_SOLUTION_FOR_OOC_ID
			targe_ooc_id = ooc_info.id;
			#endif

			#if DEBUG_SAVE_RAW
			//save ooc
			{
				char file_path_main[128] = {0};
				FILE *f_out_main;

				snprintf(file_path_main, sizeof(file_path_main), "/mnt/sd/tsen[%d]_ooc_%d.raw", p_stream->cap_count, ooc_info.id);
				printf("dump ooc file (%s) ....\r\n", file_path_main);

				if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
					printf("open file (%s) fail....\r\n\r\n", file_path_main);
				}
				if (f_out_main) {
					UINT8 *ptr = (UINT8 *)ooc_va;
					UINT32 len = OOC_BUF_SIZE;

					fwrite(ptr, 1, len, f_out_main);
					fflush(f_out_main);
				}
				// close output file
				fclose(f_out_main);
			}
			#endif


		}
		#if TEMP_SOLUTION_FOR_OOC_ID
		//skip frame for ooc or config to take effect
		skip_vcap_frame(p_stream, 3);
		#endif
		if (update_result & UPDATE_DONE) {
			b_frame_ooc_id = ooc_info.id & 0xFF;
			b_frame_cfg_id = config.id & 0xFF;
	        break;
		}
	}

	if (i >= CALIBRATION_MAX_RETRY) {
		printf("Calibration update param failed!\r\n");
		return HD_ERR_NG;
	}

	#if TEMP_SOLUTION_FOR_OOC_ID
	//skip frame for ooc or config to take effect
	skip_vcap_frame(p_stream, 3);
	#endif
	printf("Wait RAW with ooc_id(%d) and cfg_id(%d)\r\n", b_frame_ooc_id, b_frame_cfg_id);
	for (i = 0; i < CALIBRATION_MAX_RETRY; i++) {
		ret = hd_videocap_pull_out_buf(p_stream->cap_path, &vcap_video_frame, -1); // -1 = blocking mode
		if (ret != HD_OK) {
			printf("cap_pull error=%d !!\r\n\r\n", ret);
			return ret;
		}
		#if TEMP_SOLUTION_FOR_OOC_ID
		break;
		#else
		printf("frame[%llu] OOC ID=%d CFG ID=%d\r\n", vcap_video_frame.count, GET_OOC_ID(vcap_video_frame.reserved[2]), GET_CFG_ID(vcap_video_frame.reserved[2]));
		if (b_frame_ooc_id != GET_OOC_ID(vcap_video_frame.reserved[2]) || b_frame_cfg_id != GET_CFG_ID(vcap_video_frame.reserved[2])) {
			ret = hd_videocap_release_out_buf(p_stream->cap_path, &vcap_video_frame);
			if (ret != HD_OK) {
				printf("Wait RAW release error=%d !!\r\n\r\n", ret);
			}
			continue;
		} else {
			break;
		}
		#endif

	}
	if (i >= CALIBRATION_MAX_RETRY) {
		printf("Calibration wait b frame failed!\r\n");
		return HD_ERR_NG;
	}

	if (p_stream->thermal_b_frame.blk) {
		//release previous b frame
		ret = hd_videocap_release_out_buf(p_stream->cap_path, &p_stream->thermal_b_frame);
		if (ret != HD_OK) {
			printf("release previous b frame error=%d !!\r\n\r\n", ret);
		}
	}
	//update b frame
	memcpy(&p_stream->thermal_b_frame, &vcap_video_frame, sizeof(HD_VIDEO_FRAME));

	#if DEBUG_SAVE_RAW
	//save original b frame
	{
		char file_path_main[128] = {0};

		snprintf(file_path_main, sizeof(file_path_main), "/mnt/sd/tsen[%d]_frame_b_ori_%d.raw", p_stream->cap_count, b_frame_ooc_id);
		save_videoframe(&p_stream->thermal_b_frame, file_path_main);
	}
	#endif

	refine_b_frame(&p_stream->thermal_b_frame);

	#if DEBUG_SAVE_RAW
	//save refined b frame
	{
		char file_path_main[128] = {0};

		snprintf(file_path_main, sizeof(file_path_main), "/mnt/sd/tsen[%d]_frame_b_%d.raw", p_stream->cap_count, b_frame_ooc_id);
		save_videoframe(&p_stream->thermal_b_frame, file_path_main);
	}
	#endif


	//open shutter
	shutter_control(TRUE);


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
	if ((ret = hd_gfx_init()) != HD_OK)
		return ret;
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

	if ((ret = hd_videocap_open(HD_VIDEOCAP_IN(SEN1_VCAP_ID, 0), HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), &p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_1, &p_stream->proc_path2)) != HD_OK)
		return ret;
	#if (VPRC_PATH5_ENABLE)
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_4, &p_stream->proc_path_3dnr)) != HD_OK)
		return ret;
	#endif
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_VIDEOENC_0_OUT_0, &p_stream->enc_path)) != HD_OK)
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
	if ((ret = hd_videoproc_close(p_stream->proc_path2)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_close(p_stream->enc_path)) != HD_OK)
		return ret;
        #if (VPRC_PATH5_ENABLE)
        if ((ret = hd_videoproc_close(p_stream->proc_path_3dnr)) != HD_OK)
                return ret;
        #endif
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

//used when no-bind mode
static void *capture_thread(void *arg)
{
	VIDEO_LIVEVIEW* p_stream0 = (VIDEO_LIVEVIEW *)arg;
	HD_RESULT ret = HD_OK;

	HD_VIDEO_FRAME vcap_video_frame = {0};
	HD_VIDEO_FRAME nuc_video_frame = {0};

	p_stream0->cap_exit = 0;
	p_stream0->cap_count = 0;
	//------ wait flow_start ------
	while (p_stream0->cap_enter == 0) usleep(100);

	//---------skip dirty frame ---------
	printf("drop dirty %d frame ++\r\n", drop_cnt);
	skip_vcap_frame(p_stream0, drop_cnt);
	printf("drop dirty %d frame --\r\n", drop_cnt);
	p_stream0->flow_start = 1;
	#if 0//DEBUG_SAVE_RAW
	save_vcap_raw(p_stream0, 6);
	#endif
	cnt_for_calibration = 0;
	//--------- pull data test ---------
	while (p_stream0->cap_exit == 0) {

		if (cnt_for_calibration%CALIBRATION_PERIOD == 0) {
			thermal_calibration(p_stream0);
		}
		cnt_for_calibration++;
		//printf("cap_pull ....\r\n");
		ret = hd_videocap_pull_out_buf(p_stream0->cap_path, &vcap_video_frame, -1); // -1 = blocking mode
		if (ret != HD_OK) {
			if (ret != HD_ERR_UNDERRUN)
			printf("cap_pull error=%d !!\r\n\r\n", ret);
    			goto skip;
		}

		#if 0
		printf("[%llu]pxlfmt=0x08%X dim=%dx%d loff=%d pa=0x%08X\r\n",vcap_video_frame.count,vcap_video_frame.pxlfmt,
																					vcap_video_frame.dim.w,
																					vcap_video_frame.dim.h,
																					vcap_video_frame.loff[0],
																					vcap_video_frame.phy_addr[0]);
		#endif

#if 0 //bypass nuc
		#if 0
		ret = hd_videoproc_push_in_buf(p_stream0->proc_path, &vcap_video_frame, NULL, 0); // only support non-blocking mode now
		if (ret != HD_OK) {
			printf("proc_push error=%d !!\r\n\r\n", ret);
    			goto skip;
		}
		#endif
		ret = hd_videocap_release_out_buf(p_stream0->cap_path, &vcap_video_frame);
		if (ret != HD_OK) {
			printf("cap_release error=%d !!\r\n\r\n", ret);
    			goto skip;
		}
#else
		ret = thermal_nu_correction(p_stream0, &vcap_video_frame, &nuc_video_frame);
		if (ret != HD_OK) {
			printf("cap_release error=%d !!\r\n\r\n", ret);
    			goto skip;
		}

		//printf("cap_release ....\r\n");
		ret = hd_videocap_release_out_buf(p_stream0->cap_path, &vcap_video_frame);
		if (ret != HD_OK) {
			printf("cap_release error=%d !!\r\n\r\n", ret);
    			goto skip;
		}
	#if 1
		//printf("proc_push ....\r\n");
		ret = hd_videoproc_push_in_buf(p_stream0->proc_path, &nuc_video_frame, NULL, 0); // only support non-blocking mode now
		if (ret != HD_OK) {
			printf("proc_push error=%d !!\r\n\r\n", ret);
    			goto skip;
		}
	#endif
#endif
		p_stream0->cap_count ++;
		//printf("capture count = %d\r\n", p_stream0->cap_count);
skip:
		usleep(100); //sleep for getchar()
	}

	return 0;
}
typedef struct _VIDEO_INFO {
	VIDEO_RECORD *p_stream;
	HD_VIDEOENC_BS  data_pull;
	NVTLIVE555_CODEC codec_type;
	unsigned char vps[64];
	int vps_size;
	unsigned char sps[64];
	int sps_size;
	unsigned char pps[64];
	int pps_size;
	int ref_cnt;
} VIDEO_INFO;

typedef struct _ADUIO_INFO {
	int ref_cnt;
	NVTLIVE555_AUDIO_INFO info;
} AUDIO_INFO;

static VIDEO_INFO video_info[1] = { 0 };
//static AUDIO_INFO audio_info[1] = { 0 };

static int flush_video(VIDEO_INFO *p_video_info)
{
	printf("flush_video\r\n");
	while (hd_videoenc_pull_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull, 0) == 0) {
		if(hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull) != HD_OK) {
			printf("failed to hd_videoenc_release_out_buf.\n");
		}
	}

	HD_H26XENC_REQUEST_IFRAME req_i = {0};
	req_i.enable   = 1;
	int ret = hd_videoenc_set(p_video_info->p_stream->enc_path, HD_VIDEOENC_PARAM_OUT_REQUEST_IFRAME, &req_i);
	if (ret != HD_OK) {
		printf("set_enc_param_out_request_ifrm0 = %d\r\n", ret);
		return ret;
	}
	hd_videoenc_start(p_video_info->p_stream->enc_path);
	printf("flush_video end\r\n");
	return ret;
}

static uintptr_t on_open_video(int channel)
{
	printf("on_open_video\r\n");
	if ((size_t)channel >= sizeof(video_info) / sizeof(video_info[0])) {
		printf("nvtrtspd video channel exceed\n");
		return 0;
	}
	if (video_info[channel].ref_cnt != 0) {
		printf("nvtrtspd video in use.\n");
		return 0;
	}

	//no video
	if (video_info[channel].codec_type == NVTLIVE555_CODEC_UNKNOWN) {
		return 0;
	}

	video_info[channel].ref_cnt++;


	flush_video(&video_info[channel]);
	printf("on_open_video end\r\n");
	return (uintptr_t)&video_info[channel];
}

static int on_close_video(uintptr_t handle)
{
	printf("on_close_video\r\n");
	if (handle) {
		VIDEO_INFO *p_info = (VIDEO_INFO *)handle;
		p_info->ref_cnt--;
	}
	printf("on_close_video end\r\n");
	return 0;
}

static int refresh_video_info(VIDEO_INFO *p_video_info)
{
	//while to get vsp, sps, pps
	int ret;
	printf("refresh_video_info\r\n");
	p_video_info->codec_type = NVTLIVE555_CODEC_UNKNOWN;
	p_video_info->vps_size = p_video_info->sps_size = p_video_info->pps_size = 0;
	memset(p_video_info->vps, 0, sizeof(p_video_info->vps));
	memset(p_video_info->sps, 0, sizeof(p_video_info->sps));
	memset(p_video_info->pps, 0, sizeof(p_video_info->pps));
	while ((ret = hd_videoenc_pull_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull, -1)) == 0) {
		if (p_video_info->data_pull.vcodec_format == HD_CODEC_TYPE_JPEG) {
			p_video_info->codec_type = NVTLIVE555_CODEC_MJPG;
			if (hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull) != HD_OK) {
				printf("failed to hd_videoenc_release_out_buf.\n");
			}
			break;
		} else if (p_video_info->data_pull.vcodec_format == HD_CODEC_TYPE_H264) {
			p_video_info->codec_type = NVTLIVE555_CODEC_H264;
			if (p_video_info->data_pull.pack_num != 3) {
				if(hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull) != HD_OK) {
					printf("failed to hd_videoenc_release_out_buf.\n");
				}
				continue;
			}
			printf("venc pull out\r\n");
			memcpy(p_video_info->sps, (void *)PHY2VIRT_MAIN(p_video_info->data_pull.video_pack[0].phy_addr), p_video_info->data_pull.video_pack[0].size);
			memcpy(p_video_info->pps, (void *)PHY2VIRT_MAIN(p_video_info->data_pull.video_pack[1].phy_addr), p_video_info->data_pull.video_pack[1].size);
			p_video_info->sps_size = p_video_info->data_pull.video_pack[0].size;
			p_video_info->pps_size = p_video_info->data_pull.video_pack[1].size;
			if(hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull) != HD_OK) {
				printf("failed to hd_videoenc_release_out_buf.\n");
			}
			break;
		} else if (p_video_info->data_pull.vcodec_format == HD_CODEC_TYPE_H265) {
			p_video_info->codec_type = NVTLIVE555_CODEC_H265;
			if (p_video_info->data_pull.pack_num != 4) {
				if(hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull) != HD_OK) {
					printf("failed to hd_videoenc_release_out_buf.\n");
				}
				continue;
			}
			memcpy(p_video_info->vps, (void *)PHY2VIRT_MAIN(p_video_info->data_pull.video_pack[0].phy_addr), p_video_info->data_pull.video_pack[0].size);
			memcpy(p_video_info->sps, (void *)PHY2VIRT_MAIN(p_video_info->data_pull.video_pack[1].phy_addr), p_video_info->data_pull.video_pack[1].size);
			memcpy(p_video_info->pps, (void *)PHY2VIRT_MAIN(p_video_info->data_pull.video_pack[2].phy_addr), p_video_info->data_pull.video_pack[2].size);
			p_video_info->vps_size = p_video_info->data_pull.video_pack[0].size;
			p_video_info->sps_size = p_video_info->data_pull.video_pack[1].size;
			p_video_info->pps_size = p_video_info->data_pull.video_pack[2].size;
			if(hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull) != HD_OK) {
				printf("failed to hd_videoenc_release_out_buf.\n");
			}
			break;
		} else {
			if(hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull) != HD_OK) {
				printf("failed to hd_videoenc_release_out_buf.\n");
			}
		}
	}
	printf("refresh_video_info end\r\n");
	return 0;
}

static int on_get_video_info(uintptr_t handle, int timeout_ms, NVTLIVE555_VIDEO_INFO *p_info)
{
	VIDEO_INFO *p_video_info = (VIDEO_INFO *)handle;

	p_info->codec_type = p_video_info->codec_type;
	if (p_video_info->vps_size) {
		p_info->vps_size = p_video_info->vps_size;
		memcpy(p_info->vps, p_video_info->vps, p_info->vps_size);
	}

	if (p_video_info->sps_size) {
		p_info->sps_size = p_video_info->sps_size;
		memcpy(p_info->sps, p_video_info->sps, p_info->sps_size);
	}

	if (p_video_info->pps_size) {
		p_info->pps_size = p_video_info->pps_size;
		memcpy(p_info->pps, p_video_info->pps, p_info->pps_size);
	}

	return  0;
}

static int on_lock_video(uintptr_t handle, int timeout_ms, NVTLIVE555_STRM_INFO *p_strm)
{
	VIDEO_INFO *p_video_info = (VIDEO_INFO *)handle;
	//printf("on_lock_video\r\n");
	//while to get vsp, sps, pps
	int ret = hd_videoenc_pull_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull, -1);
	if (ret != 0) {
		return ret;
	}

	p_strm->addr = PHY2VIRT_MAIN(p_video_info->data_pull.video_pack[p_video_info->data_pull.pack_num-1].phy_addr);
	p_strm->size = p_video_info->data_pull.video_pack[p_video_info->data_pull.pack_num-1].size;
	p_strm->timestamp = p_video_info->data_pull.timestamp;
	//printf("on_lock_video end\r\n");
	return 0;
}

static int on_unlock_video(uintptr_t handle)
{
	//printf("on_unlock_video\r\n");
	VIDEO_INFO *p_video_info = (VIDEO_INFO *)handle;
	int ret = hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull);
	//printf("on_unlock_video end\r\n");
	return ret;
}


static uintptr_t on_open_audio(int channel)
{
	return 0;
#if 0
	if (channel >= sizeof(audio_info) / sizeof(audio_info[0])) {
		printf("nvtrtspd audio channel exceed\n");
		return 0;
	}
	audio_info[channel].ref_cnt++;
	return (uintptr_t)&audio_info[channel];
#endif
}

static int on_close_audio(uintptr_t handle)
{
	if (handle) {
		AUDIO_INFO *p_info = (AUDIO_INFO *)handle;
		p_info->ref_cnt--;
	}
	return 0;
}

static int on_get_audio_info(uintptr_t handle, int timeout_ms, NVTLIVE555_AUDIO_INFO *p_info)
{
	return 0;
}

static int on_lock_audio(uintptr_t handle, int timeout_ms, NVTLIVE555_STRM_INFO *p_strm)
{
	return 0;
}

static int on_unlock_audio(uintptr_t handle)
{
	return 0;
}

static void *encode_thread(void *arg)
{
	VIDEO_INFO *p_video_info = (VIDEO_INFO *)arg;
	VIDEO_RECORD* p_stream0 = p_video_info->p_stream;

	//------ wait flow_start ------
	while (p_stream0->flow_start == 0) sleep(1);

	// query physical address of bs buffer ( this can ONLY query after hd_videoenc_start() is called !! )
	hd_videoenc_get(p_stream0->enc_path, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf_main);

	// mmap for bs buffer (just mmap one time only, calculate offset to virtual address later)
	vir_addr_main = (uintptr_t)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_main.buf_info.phy_addr, phy_buf_main.buf_info.buf_size);
	printf("encode_thread 1\r\n");
	// update video info for rtsp
	refresh_video_info(p_video_info);
	printf("encode_thread 2\r\n");
	//live555 setup
	NVTLIVE555_HDAL_CB hdal_cb;
	hdal_cb.open_video = on_open_video;
	hdal_cb.close_video = on_close_video;
	hdal_cb.get_video_info = on_get_video_info;
	hdal_cb.lock_video = on_lock_video;
	hdal_cb.unlock_video = on_unlock_video;
	hdal_cb.open_audio = on_open_audio;
	hdal_cb.close_audio = on_close_audio;
	hdal_cb.get_audio_info = on_get_audio_info;
	hdal_cb.lock_audio = on_lock_audio;
	hdal_cb.unlock_audio = on_unlock_audio;
	printf("encode_thread 3\r\n");
	nvtrtspd_init(&hdal_cb);
	printf("encode_thread 4\r\n");
	nvtrtspd_open();
	printf("encode_thread 5\r\n");
	printf("\r\nif you want to stop, enter \"q\" to exit !!\r\n\r\n");

	return 0;
}

//scale a 1920x1080 yuv420 image up to 3840x2160
static HD_RESULT test_scale(HD_VIDEO_FRAME *in_frame, HD_VIDEO_FRAME *out_frame)
{
	HD_GFX_SCALE        param;
	HD_RESULT           ret;
	//printf("test_scale start\n");

	//use gfx engine to scale up 1920x1080 image
	memset(&param, 0, sizeof(HD_GFX_SCALE));
	param.src_img.dim.w            = 640;
	param.src_img.dim.h            = 512;
	param.src_img.format           = HD_VIDEO_PXLFMT_Y8;
	param.src_img.p_phy_addr[0]    = in_frame->phy_addr[0];
	param.src_img.lineoffset[0]    = 640;
	param.dst_img.dim.w            = 2560;
	param.dst_img.dim.h            = 2048;
	param.dst_img.format           = HD_VIDEO_PXLFMT_Y8;
	param.dst_img.p_phy_addr[0]    = out_frame->phy_addr[0];
	param.dst_img.lineoffset[0]    = 2560;
	param.src_region.x             = 0;
	param.src_region.y             = 0;
	param.src_region.w             = 640;
	param.src_region.h             = 512;
	param.dst_region.x             = 0;
	param.dst_region.y             = 0;
	param.dst_region.w             = 2560;
	param.dst_region.h             = 2048;

	ret = hd_gfx_scale(&param);
	if(ret != HD_OK){
		printf("hd_gfx_scale fail=%d\n", ret);
	}
	//printf("test_scale end\n");
	return ret;
}


#define NET_PATH_ID					UINT32
typedef struct _MEM_PARM {
	UINTPTR pa;
	UINTPTR va;
	UINT32 size;
	UINTPTR blk;
} MEM_PARM;
typedef struct _NET_PROC_CONFIG {

	CHAR model_filename[256];
	INT32 binsize;
	void *p_share_model;

	CHAR label_filename[256];

} NET_PROC_CONFIG;

typedef struct _NET_PROC {

	NET_PROC_CONFIG net_cfg;
	MEM_PARM proc_mem;
	UINT32 proc_id;

	CHAR out_class_labels[MAX_CLASS_NUMBER * LABEL_LEN];
	MEM_PARM rslt_mem;
	MEM_PARM io_mem;
	MEM_PARM intl_mem;
	MEM_PARM *out_mem;
	VENDOR_AI3_NET_INFO net_info;

} NET_PROC;


typedef struct _NET_IN_CONFIG {

	CHAR input_filename[256];
	UINT32 w;
	UINT32 h;
	UINT32 c;
	UINT32 loff;
	UINT32 fmt;

} NET_IN_CONFIG;

typedef struct _NET_IN {

	NET_IN_CONFIG in_cfg;
	MEM_PARM input_mem;
	UINT32 in_id;
	VENDOR_AI3_BUF src_img;

} NET_IN;


typedef struct _VIDEO_LIVEVIEW {

	// (1) input
	NET_IN_CONFIG net_in_cfg;
	NET_PATH_ID in_path;

	// (2) network
	NET_PROC_CONFIG net_proc_cfg;
	NET_PATH_ID net_path;
	pthread_t  proc_thread_id;
	UINT32 proc_start;
	UINT32 proc_exit;
	UINT32 proc_oneshot;
} VIDEO_AI;
extern VIDEO_AI ai_stream[]; //0: net proc
extern int do_main(void);
extern int close_main(void);
extern HD_RESULT network_user_oneshot(VIDEO_AI *p_stream);
extern HD_RESULT network_user_trigger(void);
extern HD_RESULT set_image(VIDEO_AI *p_stream,HD_VIDEO_FRAME *input_frame,HD_VIDEO_FRAME *output_frame);

UINT32 scale_mode=1;
UINT32 t1,t2,t3,t4,t5;
static void *process_thread(void *arg)
{
	VIDEO_RECORD* p_stream0 = (VIDEO_RECORD *)arg;
	HD_RESULT ret = HD_OK;

	HD_VIDEO_FRAME video_frame = {0};
	HD_VIDEO_FRAME video_frame2 = {0};

	p_stream0->prc_exit = 0;
	p_stream0->prc_loop = 0;
	p_stream0->prc_count = 0;
	//------ wait flow_start ------
	while (p_stream0->prc_enter == 0) usleep(100);

	//--------- pull data test ---------
	while (p_stream0->prc_exit == 0) {
		t1 = hd_gettime_ms();
		//printf("proc_pull ....\r\n");
		ret = hd_videoproc_pull_out_buf(p_stream0->proc_path, &video_frame, -1); // -1 = blocking mode
		if (ret != HD_OK) {
			if (ret != HD_ERR_UNDERRUN)
			printf("proc_pull error=%d !!\r\n\r\n", ret);
    			goto skip2;
		}
		t2 = hd_gettime_ms();
		ret = hd_videoproc_pull_out_buf(p_stream0->proc_path2, &video_frame2, 100); // -1 = blocking mode
		if (ret != HD_OK) {
			if (ret != HD_ERR_UNDERRUN)
			printf("proc_pull error=%d !!\r\n\r\n", ret);
    			goto skip2;
		}
		t3 = hd_gettime_ms();
		//printf("proc frame.count = %llu\r\n", video_frame.count);
#if 1
		//scale up path0 Y to path2 Y
		if(scale_mode == 0)
		{
			test_scale(&video_frame,&video_frame2);
		}
		else if(scale_mode == 1){
			set_image(&ai_stream[0],&video_frame,&video_frame2);
			network_user_trigger();
		}
#endif
		t4 = hd_gettime_ms();
		//printf("enc_push ....\r\n");
		ret = hd_videoenc_push_in_buf(p_stream0->enc_path, &video_frame2, NULL, -1); // blocking mode
		if (ret != HD_OK) {
			printf("enc_push error=%d !!\r\n\r\n", ret);
    			goto skip2;
		}
skip2:
		t5 = hd_gettime_ms();
		//printf("proc_release ....\r\n");
		ret = hd_videoproc_release_out_buf(p_stream0->proc_path, &video_frame);
		if (ret != HD_OK) {
			printf("proc_release error=%d !!\r\n\r\n", ret);
    			goto skip2;
		}
		ret = hd_videoproc_release_out_buf(p_stream0->proc_path2, &video_frame2);
		if (ret != HD_OK) {
			printf("proc_release 2 error=%d !!\r\n\r\n", ret);
    			goto skip2;
		}
		//printf("t2 %d,t3 %d,t4 %d,t5 %d\n",t2-t1,t3-t1,t4-t1,t5-t1);
		p_stream0->prc_count ++;
		//printf("process count = %d\r\n", p_stream0->prc_count);

		p_stream0->prc_loop++;
		//usleep(100); //sleep for getchar()
	}

	return 0;
}

pthread_t auto_contrast_thread_id;
static UINT32 auto_contrast_conti_run = 1;
BOOL auto_contrast_enable = FALSE;
BOOL auto_tone_enable = FALSE;
BOOL gain_clamp = FALSE;
UINT32 pre_gain = 256;
UINT32 pre_offset = 0;

static void *auto_contrast_thread(void *arg)
{
	ISPT_HISTO_DATA histo_data = {0};
	IQT_YCURVE_PARAM ycurve = {0};
	IQT_GAMMA_PARAM gamma = {0};
	AET_STATUS_INFO status = {0};
	UINT32 id = *(UINT32 *)arg;
	INT rt = 0;
	UINT32 i;
	UINT32 his_max = 0, his_min = 0;
	UINT32 gain = 256, offset = 0;
	UINT16 tone_curve[129] = {0};
	IQT_TONE_PARAM tone = {0}; //538

	histo_data.id = id;
	gamma.id = id;
	ycurve.id = id;
	status.id = id;

	while (auto_contrast_conti_run) {
		if (auto_contrast_enable == TRUE) {
			vendor_isp_get_common(ISPT_ITEM_HISTO_DATA, &histo_data);
			vendor_isp_get_iq(IQT_ITEM_GAMMA_PARAM, &gamma);
			vendor_isp_get_ae(AET_ITEM_STATUS, &status);

			rt = auto_contrast_cal(&histo_data, &ycurve, &gamma, &status);
			if (rt != HD_OK) {
				printf("auto_contrast_cal fail=%d\n", rt);
			}
			ycurve.ycurve.enable = TRUE;
			vendor_isp_set_iq(IQT_ITEM_YCURVE_PARAM, &ycurve);
		} else {
			vendor_isp_get_iq(IQT_ITEM_YCURVE_PARAM, &ycurve);
			ycurve.ycurve.enable = FALSE;
			vendor_isp_set_iq(IQT_ITEM_YCURVE_PARAM, &ycurve);
			//printf("disable Ycurve id = %d \n", id);
		}
			/*
			printf("hist pre wdr = \r\n");
			for(i = 0; i < ISP_HISTO_MAX_SIZE; i++) {
				printf("%d ",histo_data.histo_rslt.hist_stcs_pre_wdr[i]);
			}
			printf("\r\n");
			*/
			for(i = 0; i < ISP_HISTO_MAX_SIZE; i++) {
				if (histo_data.histo_rslt.hist_stcs_pre_wdr[i] != 0) {
					his_min = i;
					break;
				}
			}

			for(i = 0; i < ISP_HISTO_MAX_SIZE; i++) {
				if (histo_data.histo_rslt.hist_stcs_pre_wdr[i] != 0) {
					his_max = i;
				}
			}

			printf("his_min = %d, his_max = %d \r\n", his_min, his_max);

			//TBD his_min = 0
			his_min = his_min / 2;

			gain =  256 * ISP_HISTO_MAX_SIZE / (his_max - his_min);
			offset = his_min * 32;

			//TBD clamp 4x
			if (gain_clamp) {
				gain = CLAMP(gain, 0, 1024);
			}
			//printf("tone_curve 1 = \r\n");
			for (i = 0; i < 129; i++) {
				tone_curve[i] = i * 32;
				//printf("%d ",tone_curve[i]);
			}
			//printf("\r\n");
			//printf("tone_curve 2 = \r\n");
			pre_offset = ((pre_offset + offset ) >> 1);
			pre_gain = ((pre_gain + gain) >> 1);
			for (i = 0; i < 129; i++) {
				if (tone_curve[i] > pre_offset) {
					tone_curve[i] = CLAMP((((tone_curve[i] - pre_offset) * pre_gain) >> 8) ,0, 4095);
				} else {
					tone_curve[i] = 0;
				}
				//printf("%d ",tone_curve[i]);
			}
			//printf("\r\n");

			tone.id = id;
			vendor_isp_get_iq(IQT_ITEM_TONE_PARAM, &tone);
			tone.tone.enable = auto_tone_enable;
			tone.tone.mode = 1;

			for (i = 0; i < 65; i++) {
				tone.tone.manual_lut_left[i] = tone_curve[i];
			}

			for (i = 0; i < 17; i++) {
				tone.tone.manual_lut_right[i] = tone_curve[64 + i * 4];
			}

			vendor_isp_set_iq(IQT_ITEM_TONE_PARAM, &tone);

		sleep(1);
	}

	return 0;
}

MAIN(argc, argv)
{
	HD_RESULT ret;
	INT key;
	VIDEO_RECORD stream[1] = {0}; //0: main stream
	UINT32 enc_type = 0;
	UINT32 enc_bitrate = 4;
	HD_DIM main_dim;

	ISPT_C_GAIN cgain = {0};
	IQT_CFG_INFO cfg_info = {0};
	IQT_NIGHT_MODE night_mode = {0};
	IQT_IMAGEEFFECT imageeffect = {0};

	imageeffect.effect = IQ_UI_IMAGEEFFECT_OFF;
	UINT32 id;

	INT32 arg_idx = 2;

	if (argc == 1 || argc > 7) {
		printf("Usage: <enc_type> <enc_bitrate>\r\n");
		printf("Help:\r\n");
		printf("  <enc_type>    : 0(H265), 1(H264) 2(MJPG)\r\n");
		printf("  <enc_bitrate> : Mbps\r\n");
		printf("  <sensor> :  0(IRAY_6122C)\r\n");
		printf("  <fps> :\r\n");
		printf("  <serial id> :\r\n");
		printf("  <drop cnt> :\r\n");
		return 0;
	}

	// query program options
	if (argc > 1) {
		enc_type = atoi(argv[1]);
		if (enc_type == 0) {
			printf("enc_type: H.265\r\n");
		} else if (enc_type == 1) {
			printf("enc_type: H.264\r\n");
		} else if (enc_type == 2) {
			printf("enc_type: MJPG\r\n");
		} else {
			printf("error: not support enc_type!\r\n");
			return 0;
		}
	}
	if (argc > 2) {
		enc_bitrate = atoi(argv[2]);
		printf("enc_bitrate: %d Mbps\r\n", (int)enc_bitrate);
	}

	arg_idx++;
	if (argc > arg_idx) {
		resolution_set = atoi(argv[arg_idx]);
		printf("sensor %d\r\n", resolution_set);
	}

	arg_idx++;
	if (argc > arg_idx) {
		g_fps = atoi(argv[arg_idx]);
		printf("g_fps %d\r\n", g_fps);
	}

	arg_idx++;
	if (argc > arg_idx) {
		serial_id = atoi(argv[arg_idx]);
		printf("serial_id %d\r\n", serial_id);
	}

	arg_idx++;
	if (argc > arg_idx) {
		drop_cnt = atoi(argv[arg_idx]);
		printf("drop_cnt %d\r\n", drop_cnt);
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

	if (vendor_isp_init() == HD_ERR_NG) {
		printf("vendor_isp_init failed \n");
	}

	cgain.id = SEN1_VCAP_ID;
	cgain.gain[0] = 256;
	cgain.gain[1] = 256;
	cgain.gain[2] = 256;
	vendor_isp_set_common(ISPT_ITEM_C_GAIN, &cgain);

	cfg_info.id = SEN1_VCAP_ID;
	strncpy(cfg_info.path, "/mnt/app/isp/isp_rtd6122c_0.cfg", CFG_NAME_LENGTH);
	vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);

	gpio_set_dir(SHUTTER_3V3_P, 1);
	gpio_set_dir(SHUTTER_3V3_N, 1);
	gpio_set_value(SHUTTER_3V3_P, 0);
	gpio_set_value(SHUTTER_3V3_N, 0);

	/* ##### Just a hard code to modify PAD driving, please use nvt-gpio.dtsi for your own project! #####
	For example
	&top {
		sgpio6 {pad_config = <PAD_DS_SGPIO(6) PAD_DS_L1 PAD_SGPIO(6) PAD_PULLUP>;};
	};
	*/
	system("mem w 0x2f0030160 0x01100011");


	ret = init_thermal_lib(&stream[0]);
	if (ret != HD_OK) {
		printf("init thermal lib failed\n");
		goto exit;
	}
#if 0
	ret = init_thermal_data(&stream[0]);
	if (ret != HD_OK) {
		printf("init thermal data failed\n");
		goto exit;
	}
#else
	ret = init_thermal_frame(&stream[0]);
	if (ret != HD_OK) {
		printf("init thermal frame failed\n");
		goto exit;
	}

#endif
	ret = init_thermal_ooc_mem(&stream[0]);
	if (ret != HD_OK) {
		printf("init thermal mem failed\n");
		goto exit;
	}

	// open video_record modules (main)
	stream[0].proc_max_dim.w = 1280;//VDO_SIZE_W; //assign by user
	stream[0].proc_max_dim.h = 720;//VDO_SIZE_H; //assign by user
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
	stream[0].cap_dim.w = VDO_SIZE_W; //assign by user
	stream[0].cap_dim.h = VDO_SIZE_H; //assign by user
	ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim);
	if (ret != HD_OK) {
		printf("set cap fail=%d\n", ret);
		goto exit;
	}

	// assign parameter by program options
	main_dim.w = CROP_SIZE_W;
	main_dim.h = CROP_SIZE_H;

	// set videoproc parameter (main)
	ret = set_proc_param(stream[0].proc_path, &main_dim);
	if (ret != HD_OK) {
		printf("set proc fail=%d\n", ret);
		goto exit;
	}
	#if (VPRC_PATH5_ENABLE)
	ret = set_proc_param(stream[0].proc_path_3dnr, &main_dim);
	if (ret != HD_OK) {
		printf("set proc fail = %d \n", ret);
		goto exit;
	}
	#endif

	// set videoproc parameter (sub)
	main_dim.w = OUT_SIZE_W;
	main_dim.h = OUT_SIZE_H;

	ret = set_proc_param(stream[0].proc_path2, &main_dim);
	if (ret != HD_OK) {
		printf("set proc fail=%d\n", ret);
		goto exit;
	}

	// set videoenc config (main)
	stream[0].enc_max_dim.w = main_dim.w;
	stream[0].enc_max_dim.h = main_dim.h;
	ret = set_enc_cfg(stream[0].enc_path, &stream[0].enc_max_dim, enc_bitrate * 1024 * 1024, 0);
	if (ret != HD_OK) {
		printf("set enc-cfg fail=%d\n", ret);
		goto exit;
	}

	// create capture_thread (pull_out frame, push_in frame then pull_out frame)
	ret = pthread_create(&stream[0].cap_thread_id, NULL, capture_thread, (void *)stream);
	if (ret < 0) {
		printf("create capture_thread failed");
		goto exit;
	}
	// set videoenc parameter (main)
	stream[0].enc_dim.w = main_dim.w;
	stream[0].enc_dim.h = main_dim.h;
	ret = set_enc_param(stream[0].enc_path, &stream[0].enc_dim, enc_type, enc_bitrate * 1024 * 1024);
	if (ret != HD_OK) {
		printf("set enc fail=%d\n", ret);
		goto exit;
	}

	ret = pthread_create(&stream[0].prc_thread_id, NULL, process_thread, (void *)stream);
	if (ret < 0) {
		printf("create process_thread failed");
		goto exit;
	}

	//update first ooc and cfg before VCAP start
	init_thermal_cfg_ooc(&stream[0]);

	// bind video_record modules (main)
	//hd_videocap_bind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);
	//hd_videoproc_bind(HD_VIDEOPROC_0_OUT_1, HD_VIDEOENC_0_IN_0);

	// create encode_thread (pull_out bitstream)
	video_info[0].p_stream = &stream[0];
	ret = pthread_create(&stream[0].enc_thread_id, NULL, encode_thread, (void *)&video_info[0]);
	if (ret < 0) {
		printf("create encode thread failed");
		goto exit;
	}
	do_main();
	// start video_record modules (main)
	hd_videocap_start(stream[0].cap_path);
	hd_videoproc_start(stream[0].proc_path);
	hd_videoproc_start(stream[0].proc_path2);
	#if (VPRC_PATH5_ENABLE)
	hd_videoproc_start(stream[0].proc_path_3dnr);
	#endif
	// just wait ae/awb stable for auto-test, if don't care, user can remove it
	sleep(1);
	hd_videoenc_start(stream[0].enc_path);

	// let encode_thread start to work
	//stream[0].flow_start = 1;
	stream[0].prc_enter = 1;
	//rtspd_start(554);
	// start capture_thread
	stream[0].cap_enter = 1;

	auto_contrast_conti_run = 1;
	auto_contrast_enable = 1;
	auto_tone_enable = 1;
	gain_clamp = 1;
	id = 4;
	auto_contrast_set_str(16);
	if (pthread_create(&auto_contrast_thread_id, NULL, auto_contrast_thread, &id) < 0) {
		printf("create auto_contrast thread failed");
	}

	// query user key
	printf("Enter q to exit \n");
	while (1) {
		key = GETCHAR();
		if (key == 'q' || key == 0x3) {
			// let encode_thread stop loop and exit
			stream[0].enc_exit = 1;
			stream[0].prc_exit = 1;
			stream[0].flow_start = 0;
			auto_contrast_conti_run = 0;
			// quit program
			break;
		}
		if (key == 's') {
			stream[0].cap_snap = 1;
		}
		if (key == 'r') {

			// run once
			network_user_oneshot(&ai_stream[0]);
			continue;
		}
		if (key == '2') {

			// run once
			scale_mode = 2;
			continue;
		}
		if (key == '1') {

			// run once
			scale_mode = 1;
			continue;
		}
		if (key == '0') {

			// run once
			scale_mode = 0;
			continue;
		}

		if (key == '5') {
			printf("open shutter\r\n");
			gpio_set_value(SHUTTER_3V3_P, 1);
			usleep(20000);
			gpio_set_value(SHUTTER_3V3_P, 0);
		}
		if (key == '6') {
			printf("close shutter\r\n");
			gpio_set_value(SHUTTER_3V3_N, 1);
			usleep(20000);
			gpio_set_value(SHUTTER_3V3_N, 0);
		}

		#if (DEBUG_MENU == 1)
		if (key == 'd') {
			// enter debug menu
			hd_debug_run_menu();
			printf("\r\nEnter q to exit, Enter d to debug\r\n");
		}
		#endif

		if (key == 'h') {
			imageeffect.id = SEN1_VCAP_ID;
			night_mode.id = SEN1_VCAP_ID;
			if (imageeffect.effect == IQ_UI_IMAGEEFFECT_OFF) {
				imageeffect.effect = IQ_UI_IMAGEEFFECT_THERMAL_RED;
				night_mode.mode = IQ_UI_NIGHT_MODE_OFF;
			} else if (imageeffect.effect == IQ_UI_IMAGEEFFECT_THERMAL_RED) {
				imageeffect.effect = IQ_UI_IMAGEEFFECT_THERMAL_COLOR;
				night_mode.mode = IQ_UI_NIGHT_MODE_OFF;
			} else {
				imageeffect.effect = IQ_UI_IMAGEEFFECT_OFF;
				night_mode.mode = IQ_UI_NIGHT_MODE_ON;
			}
			vendor_isp_set_iq(IQT_ITEM_IMAGEEFFECT, &imageeffect);
			vendor_isp_set_iq(IQT_ITEM_NIGHT_MODE, &night_mode);
		}

		if (key == 'e') {
			if (auto_contrast_enable == 0) {
				auto_contrast_enable = 1;
				auto_contrast_set_str(16);
			} else {
				auto_contrast_enable = 0;
				auto_contrast_set_str(0);
			}
		}

		if (key == 't') {
			if (auto_tone_enable == 0) {
				auto_tone_enable = 1;
			} else {
				auto_tone_enable = 0;
			}
		}

		if (key == 'c') {
			if (gain_clamp == 0) {
				gain_clamp = 1;
			} else {
				gain_clamp = 0;
			}
		}

		if (key == 'k') {
			cnt_for_calibration = 0;
		}
	}
	nvtrtspd_close();

	// stop capture_thread
	stream[0].cap_exit = 1;

	// destroy capture_thread
	pthread_join(stream[0].cap_thread_id, NULL);  //NOTE: before destory, call stop to breaking pull(-1)

	stream[0].prc_exit = 1;
	// stop video_record modules (main)
	hd_videocap_stop(stream[0].cap_path);
	hd_videoproc_stop(stream[0].proc_path);
	hd_videoproc_stop(stream[0].proc_path2);
        #if (VPRC_PATH5_ENABLE)
        hd_videoproc_stop(stream[0].proc_path_3dnr);
        #endif
	hd_videoenc_stop(stream[0].enc_path);

	// refresh to indicate no stream
	//refresh_video_info(&video_info[0]);

	// destroy encode thread
	pthread_join(stream[0].enc_thread_id, NULL);

	// unbind video_record modules (main)
	//hd_videocap_unbind(HD_VIDEOCAP_0_OUT_0);
	//hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_1);

	// mummap for bs buffer
	if (vir_addr_main) {
		ret = hd_common_mem_munmap((void *)vir_addr_main, phy_buf_main.buf_info.buf_size);
		if (ret != HD_OK) {
			printf("mem_munmap fail=%d\n", ret);
		}
	}

	close_main();
exit:
	// close video_record modules (main)
	ret = close_module(&stream[0]);
	if (ret != HD_OK) {
		printf("close fail=%d\n", ret);
	}

	ret = exit_thermal_lib(&stream[0]);
	if (ret != HD_OK) {
		printf("exit thermal lib fail=%d\n", ret);
	}
#if 0
	ret = exit_thermal_data(&stream[0]);
	if (ret != HD_OK) {
		printf("exit thermal fail=%d\n", ret);
	}
#else
	ret = exit_thermal_frame(&stream[0]);
	if (ret != HD_OK) {
		printf("exit thermal frame fail=%d\n", ret);
	}
#endif
	ret = exit_thermal_ooc_mem(&stream[0]);
	if (ret != HD_OK) {
		printf("exit thermal mem fail=%d\n", ret);
	}

	// uninit all modules
	ret = exit_module();
	if (ret != HD_OK) {
		printf("exit fail=%d\n", ret);
	}

	vendor_isp_uninit();

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
