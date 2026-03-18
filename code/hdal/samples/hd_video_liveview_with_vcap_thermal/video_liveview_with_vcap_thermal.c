/**
	@brief Sample code of video liveview with thermal sensor.\n

	@file video_liveview_with_vcap_thermal.c

	@author Ben Wang

	@ingroup mhdal

	@note This file is modified from video_liveview_with_mode.c.

	Copyright Novatek Microelectronics Corp. 2024.  All rights reserved.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "hdal.h"
#include "hd_debug.h"
#include "vendor_videocapture.h"
#include "vendor_videoprocess.h"
#include "vendor_gfx.h"
#include "thermal_lib.h"
#include "vendor_isp.h"

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
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(hd_video_liveview_with_mode, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define DEBUG_MENU 		1

#define CHKPNT			printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
#define DBGH(x)			printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x)			printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)


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

#define RESOLUTION_SET  0 //0: RTD6122C,

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

#if ( RESOLUTION_SET == 0)
#define VDO_SIZE_W      RAW_SIZE_W
#define VDO_SIZE_H      RAW_SIZE_H
#endif

#define RAW_ACTIVE_X   (20+10)//(22+10)
#define RAW_ACTIVE_Y   8
#define RAW_ACTIVE_W   640
#define RAW_ACTIVE_H   512


#define VDO_OUT_SIZE_W      960
#define VDO_OUT_SIZE_H      240

#define VIDEOCAP_ALG_FUNC 0//HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB
#define VIDEOPROC_ALG_FUNC HD_VIDEOPROC_FUNC_AF | HD_VIDEOPROC_FUNC_WDR | HD_VIDEOPROC_FUNC_DEFOG | HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA

#define SEN_SEL_IRAY_6122C   0

static UINT32 resolution_set = SEN_SEL_IRAY_6122C;

static UINT32 g_prcbind = 0;  //0:D2D, 1:lowlatency, 2: one-buf, 0xff: no-bind
static UINT32 g_prcfmt = 0; //0:YUV, 1:YUV-compress
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
	UINT32 id;

	// config common pool (cap)
	id = 0;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	//normal
	mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()
    													+VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
    													+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);

	mem_cfg.pool_info[id].blk_size += VDO_RAW_BUFSIZE(RAW_SIZE_W, RAW_SIZE_H, CAP_OUT_FMT);
	mem_cfg.pool_info[id].blk_cnt = 2 + OOC_MAX_BLK_NUM + 3;// 3 => k + b frame + nuc out
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;

	// config common pool (main)
	id++;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	if ((g_prcbind == 0) || (g_prcbind == 0xff)) {
		//normal
		mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(VDO_OUT_SIZE_W, VDO_OUT_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
		mem_cfg.pool_info[id].blk_cnt = 4; //vout release blk is fast
		mem_cfg.pool_info[id].ddr_id = DDR_ID0;
	} else if (g_prcbind == 2) {
		//one-buf
		mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(VDO_OUT_SIZE_W, VDO_OUT_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
		mem_cfg.pool_info[id].blk_cnt = 1;
		mem_cfg.pool_info[id].ddr_id = DDR_ID0;
	} else {
		//not support
		mem_cfg.pool_info[id].blk_size = 0;
		mem_cfg.pool_info[id].blk_cnt = 0;
		mem_cfg.pool_info[id].ddr_id = DDR_ID0;
	}

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

    #if (RESOLUTION_SET == 0)
	snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_rtd6122c");
	printf("Using nvt_sen_imx290\n");
	#elif (RESOLUTION_SET == 1)
	snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os05a10");
	printf("Using nvt_sen_os05a10\n");
	#endif

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
	#else //HD_CROP_ON  only for test
	{
		HD_VIDEOCAP_CROP video_crop_param = {0};

		video_crop_param.mode = HD_CROP_ON;
		video_crop_param.win.rect.x = 0;
		video_crop_param.win.rect.y = 0;
		video_crop_param.win.rect.w = RAW_SIZE_W;
		video_crop_param.win.rect.h= RAW_SIZE_H;
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
		video_out_param.depth = 0;
		if (g_prcbind == 0xff) //no-bind mode
			video_out_param.depth = 1;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	}

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
		if (g_prcbind == 1)
			video_path_param.out_func |= HD_VIDEOPROC_OUTFUNC_LOWLATENCY; //enable low-latency
		if (g_prcbind == 2)
			video_path_param.out_func |= HD_VIDEOPROC_OUTFUNC_ONEBUF;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////

static HD_RESULT set_out_cfg(HD_PATH_ID *p_video_out_ctrl, UINT32 out_type, HD_VIDEOOUT_HDMI_ID hdmi_id)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOOUT_MODE videoout_mode = {0};
	HD_PATH_ID video_out_ctrl = 0;

	ret = hd_videoout_open(0, HD_VIDEOOUT_0_CTRL, &video_out_ctrl); //open this for device control
	if (ret != HD_OK) {
		return ret;
	}

	printf("out_type=%d\r\n", out_type);

	#if 0
	videoout_mode.output_type = HD_COMMON_VIDEO_OUT_LCD;
	videoout_mode.input_dim = HD_VIDEOOUT_IN_AUTO;
	videoout_mode.output_mode.lcd = HD_VIDEOOUT_LCD_0;
	if (out_type != 1) {
		printf("520 only support LCD\r\n");
	}
	#else
	switch(out_type) {
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
	#endif
	ret = hd_videoout_set(video_out_ctrl, HD_VIDEOOUT_PARAM_MODE, &videoout_mode);

	*p_video_out_ctrl=video_out_ctrl ;

	if (ret != HD_OK) {
		return ret;
	}

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

static HD_RESULT set_out_param(HD_PATH_ID video_out_path, HD_DIM *p_dim)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOOUT_IN video_out_param = {0};
	HD_VIDEOOUT_FUNC_CONFIG videoout_cfg = {0};

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

	if (g_prcbind == 2)
		videoout_cfg.in_func |= HD_VIDEOOUT_INFUNC_ONEBUF;
	ret = hd_videoout_set(video_out_path, HD_VIDEOOUT_PARAM_FUNC_CONFIG, &videoout_cfg);


	return ret;
}

///////////////////////////////////////////////////////////////////////////////

typedef struct _VIDEO_LIVEVIEW {

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
	HD_PATH_ID proc_path_3dnr;

	// only used when (g_prcbind = 0xff)  //no-bind mode
	pthread_t  prc_thread_id;
	UINT32	prc_enter;
	UINT32	prc_exit;
	UINT32 	prc_count;
	UINT32 	prc_loop;

	// (3)
	HD_DIM  out_max_dim;
	HD_DIM  out_dim;
	HD_VIDEOOUT_SYSCAPS out_syscaps;
	HD_PATH_ID out_ctrl;
	HD_PATH_ID out_path;

	HD_VIDEOOUT_HDMI_ID hdmi_id;


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
} VIDEO_LIVEVIEW;

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
#define PHY2VIRT_MAIN(pa) (vir_addr_main + ((pa) - phy_addr_main))
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
		//printf("skip_vcap_frame %d\r\n", i);
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
		printf("raw_va=0x%lX ooc_info va=0x%lX pack_bus_sel(%d) w,h,lof(%d,%d,%d) id(%d), config_id(%d)\r\n", PHY2VIRT_MAIN(vcap_video_frame.phy_addr[0]),
																				ooc_info.addr, ooc_info.pack_bus_sel,
																				ooc_info.width, ooc_info.height, ooc_info.lofs, ooc_info.id, config.id);
#endif
		update_result = tsen_update_params(PHY2VIRT_MAIN(vcap_video_frame.phy_addr[0]), &ooc_info, &config);

#if 0
		printf("update_result(0x%X) ooc_id(%d) config_id(%d)\r\n", update_result, ooc_info.id, config.id);
#endif

		if (update_result) {
			//just a debug log to check SYNC code
			DumpMem(PHY2VIRT_MAIN(vcap_video_frame.phy_addr[0]), 22*2, 16);
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
	if ((ret = hd_videoout_init()) != HD_OK)
		return ret;
	if ((ret = hd_gfx_init()) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT open_module(VIDEO_LIVEVIEW *p_stream, HD_DIM* p_proc_max_dim, UINT32 out_type)
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
	// set videoout config
	ret = set_out_cfg(&p_stream->out_ctrl, out_type,p_stream->hdmi_id);
	if (ret != HD_OK) {
		printf("set out-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	if ((ret = hd_videocap_open(HD_VIDEOCAP_IN(SEN1_VCAP_ID, 0), HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), &p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_4, &p_stream->proc_path_3dnr)) != HD_OK)
		return ret;
	if ((ret = hd_videoout_open(HD_VIDEOOUT_0_IN_0, HD_VIDEOOUT_0_OUT_0, &p_stream->out_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT close_module(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_close(p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_close(p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_close(p_stream->proc_path_3dnr)) != HD_OK)
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

// only used when (g_capbind = 0xff)  //no-bind mode
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

// only used when (g_prcbind = 0xff)  //no-bind mode
static void *process_thread(void *arg)
{
	VIDEO_LIVEVIEW* p_stream0 = (VIDEO_LIVEVIEW *)arg;
	HD_RESULT ret = HD_OK;

	HD_VIDEO_FRAME video_frame = {0};

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

		//printf("out_push ....\r\n");
		ret = hd_videoout_push_in_buf(p_stream0->out_path, &video_frame, NULL, -1); // blocking mode
		if (ret != HD_OK) {
			printf("out_push error=%d !!\r\n\r\n", ret);
    			goto skip2;
		}

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

MAIN(argc, argv)
{
	HD_RESULT ret;
	INT key;
	VIDEO_LIVEVIEW stream[1] = {0}; //0: main stream
	UINT32 out_type = 1;
	INT32 arg_idx = 0;
	HD_DIM main_dim;
	ISPT_C_GAIN cgain = {0};
	IQT_CFG_INFO cfg_info = {0};
	IQT_NIGHT_MODE night_mode = {0};
	IQT_IMAGEEFFECT imageeffect = {0};

	imageeffect.effect = IQ_UI_IMAGEEFFECT_OFF;

	if (argc == 2 && argv[1][0] == '?') {
		printf("Usage: <drop cnt> <sensor> <fps> <serial id>\r\n");
		printf("Help:\r\n");
		printf("  <sensor> :  0(IRAY_6122C)\r\n");
		printf("  <fps> :\r\n");
		printf("  <serial id> :\r\n");
		printf("  <drop cnt> :\r\n");
		return 0;
	}

	// assign g_prcfmt
	if (g_prcfmt == 0) {
		g_prcfmt = HD_VIDEO_PXLFMT_YUV420;
	} else {
		g_prcfmt = HD_VIDEO_PXLFMT_YUV420_NVX2;
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

	// open video_liveview modules (main)
	stream[0].proc_max_dim.w = VDO_SIZE_W; //assign by user
	stream[0].proc_max_dim.h = VDO_SIZE_H; //assign by user
	ret = open_module(&stream[0], &stream[0].proc_max_dim, out_type);
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
	ret = get_out_caps(stream[0].out_ctrl, &stream[0].out_syscaps);
	if (ret != HD_OK) {
		printf("get out-caps fail=%d\n", ret);
		goto exit;
	}
	stream[0].out_max_dim = stream[0].out_syscaps.output_dim;

	// set videocap parameter
	stream[0].cap_dim.w = RAW_SIZE_W; //assign by user
	stream[0].cap_dim.h = RAW_SIZE_H; //assign by user
	ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim);
	if (ret != HD_OK) {
		printf("set cap fail=%d\n", ret);
		goto exit;
	}

	// set videoproc parameter (main)
	if (g_prcbind == 0xff) //no-bind mode
		ret = set_proc_param(stream[0].proc_path, &stream[0].out_max_dim); //must set vprc's out dim
	else
		ret = set_proc_param(stream[0].proc_path, NULL); //keep vprc's out dim = {0,0}, it means auto sync from vout's in dim
	if (ret != HD_OK) {
		printf("set proc fail=%d\n", ret);
		goto exit;
	}

	main_dim.w = RAW_ACTIVE_W;
	main_dim.h = RAW_ACTIVE_H;
	ret = set_proc_param(stream[0].proc_path_3dnr, &main_dim);
	if (ret != HD_OK) {
		printf("set proc fail = %d \n", ret);
		goto exit;
	}

	// set videoout parameter (main)
	stream[0].out_dim.w = stream[0].out_max_dim.w; //using device max dim.w
	stream[0].out_dim.h = stream[0].out_max_dim.h; //using device max dim.h
	ret = set_out_param(stream[0].out_path, &stream[0].out_dim);
	if (ret != HD_OK) {
		printf("set out fail=%d\n", ret);
		goto exit;
	}

	// create capture_thread (pull_out frame, push_in frame then pull_out frame)
	ret = pthread_create(&stream[0].cap_thread_id, NULL, capture_thread, (void *)stream);
	if (ret < 0) {
		printf("create capture_thread failed");
		goto exit;
	}

	if (g_prcbind == 0xff) { //no-bind mode
		// create process_thread (pull_out frame, push_in frame then pull_out bitstream)
		ret = pthread_create(&stream[0].prc_thread_id, NULL, process_thread, (void *)stream);
		if (ret < 0) {
			printf("create process_thread failed");
			goto exit;
		}
	} else {
		// bind video_liveview modules (main)
		hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOOUT_0_IN_0);
	}

	//update first ooc and cfg before VCAP start
	init_thermal_cfg_ooc(&stream[0]);


	// start video_liveview modules (main)
	hd_videocap_start(stream[0].cap_path);
	hd_videoproc_start(stream[0].proc_path);
	hd_videoproc_start(stream[0].proc_path_3dnr);
	// wait thermal stable
	sleep(2);
	hd_videoout_start(stream[0].out_path);

	// start capture_thread
	stream[0].cap_enter = 1;

	// query user key
	printf("Enter q to exit\n");
	while (1) {
		key = GETCHAR();
		if (key == 'q' || key == 0x3) {
			// quit program
			break;
		}
		if (key == 's') {
			stream[0].cap_snap = 1;
		}

		if (key == 'a') {
			hd_videoproc_stop(stream[0].proc_path);
			hd_videoproc_start(stream[0].proc_path);
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
		if (key == 'k') {
			cnt_for_calibration = 0;
		}
	}

	// stop capture_thread
	stream[0].cap_exit = 1;

	// destroy capture_thread
	pthread_join(stream[0].cap_thread_id, NULL);  //NOTE: before destory, call stop to breaking pull(-1)

	// stop video_liveview modules (main)
	hd_videocap_stop(stream[0].cap_path);
	hd_videoproc_stop(stream[0].proc_path);
	hd_videoproc_stop(stream[0].proc_path_3dnr);
	hd_videoout_stop(stream[0].out_path);



	if (g_prcbind == 0xff) { //no-bind mode
		// destroy process_thread
		pthread_join(stream[0].prc_thread_id, NULL);  //NOTE: before destory, call stop to breaking pull(-1)
	} else {
		// unbind video_liveview modules (main)
		hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_0);
	}

exit:
	// close video_liveview modules (main)
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
