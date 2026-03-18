#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <hdal.h>
#include <kwrap/perf.h>
#include <kwrap/debug.h>
#include <kwrap/util.h>
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/signal.h>
#include <FreeRTOS_POSIX/pthread.h>
#include "prjcfg.h"
#include "sys_fastboot.h"
#include "flow_preview.h"
#include "vendor_videocapture.h"

//  0: imx290,   1: f37,  2: os02k10,  3: os05a10,  4: f35
//  5: gc4653,   6: f51,  7: sc450ai,  8: os04c10,  9: os04e10
// 10: nt99436
#if defined(_sen_imx290_)
#define RESOLUTION_SET  0
#elif defined(_sen_f37_)
#define RESOLUTION_SET  1
#elif defined(_sen_os02k10_)
#define RESOLUTION_SET  2
#elif defined(_sen_os05a10_)
#define RESOLUTION_SET  3
#elif defined(_sen_f35_)
#define RESOLUTION_SET  4
#elif defined(_sen_gc4653_)
#define RESOLUTION_SET  5
#elif defined(_sen_f51_)
#define RESOLUTION_SET  6
#elif defined(_sen_sc450ai_)
#define RESOLUTION_SET  7
#elif defined(_sen_os04c10_)
#define RESOLUTION_SET  8
#elif defined(_sen_os04e10_)
#define RESOLUTION_SET  9
#elif defined(_sen_nt99436_)
#define RESOLUTION_SET  10
#else
#error "un-implemented sensor for flow_preview.c"
#endif

#if ( RESOLUTION_SET == 0)
#define SEN_NAME_ID      0
#define VDO_SIZE_W       1920
#define VDO_SIZE_H       1080
#define SEN_DRVIVER_NAME "nvt_sen_imx290"
#define SEN_IF_TYPE      HD_COMMON_VIDEO_IN_MIPI_CSI
#define SEN_PINMUX       0x220
#define SERIAL_IF_PINMUX 0xF01
#define PIN_MAP_0        0
#define PIN_MAP_1        1
#define PIN_MAP_2        HD_VIDEOCAP_SEN_IGNORE
#define PIN_MAP_3        HD_VIDEOCAP_SEN_IGNORE
#elif ( RESOLUTION_SET == 1)
#define SEN_NAME_ID      1
#define VDO_SIZE_W       1920
#define VDO_SIZE_H       1080
#define SEN_DRVIVER_NAME "nvt_sen_f37"
#define SEN_IF_TYPE      HD_COMMON_VIDEO_IN_MIPI_CSI
#define SEN_PINMUX       0x220
#define SERIAL_IF_PINMUX 0xF01
#define PIN_MAP_0        0
#define PIN_MAP_1        1
#elif ( RESOLUTION_SET == 2)
#define SEN_NAME_ID      2
#define VDO_SIZE_W       1920
#define VDO_SIZE_H       1080
#define SEN_DRVIVER_NAME "nvt_sen_os02k10"
#define SEN_IF_TYPE      HD_COMMON_VIDEO_IN_MIPI_CSI
#define SEN_PINMUX       0x220
#define SERIAL_IF_PINMUX 0xF01
#define PIN_MAP_0        0
#define PIN_MAP_1        1
#define PIN_MAP_2        2
#define PIN_MAP_3        3
#elif (RESOLUTION_SET == 3)
#define SEN_NAME_ID      3
#define VDO_SIZE_W       2592
#define VDO_SIZE_H       1944
#define SEN_DRVIVER_NAME "nvt_sen_os05a10"
#define SEN_IF_TYPE      HD_COMMON_VIDEO_IN_MIPI_CSI
#define SEN_PINMUX       0x220
#define SERIAL_IF_PINMUX 0xF01
#define PIN_MAP_0        0
#define PIN_MAP_1        1
#define PIN_MAP_2        2
#define PIN_MAP_3        3
#elif ( RESOLUTION_SET == 4)
#define SEN_NAME_ID      4
#define VDO_SIZE_W       1920
#define VDO_SIZE_H       1080
#define SEN_DRVIVER_NAME "nvt_sen_f35"
#define SEN_IF_TYPE      HD_COMMON_VIDEO_IN_MIPI_CSI
#define SEN_PINMUX       0x220
#define SERIAL_IF_PINMUX 0x301
#define PIN_MAP_0        0
#define PIN_MAP_1        1
#define PIN_MAP_2        HD_VIDEOCAP_SEN_IGNORE
#define PIN_MAP_3        HD_VIDEOCAP_SEN_IGNORE
#elif ( RESOLUTION_SET == 5)
#define SEN_NAME_ID      5
#define VDO_SIZE_W       2560
#define VDO_SIZE_H       1440
#define SEN_DRVIVER_NAME "nvt_sen_gc4653"
#define SEN_IF_TYPE      HD_COMMON_VIDEO_IN_MIPI_CSI
#define SEN_PINMUX       0x220
#define SERIAL_IF_PINMUX 0x301
#define PIN_MAP_0        0
#define PIN_MAP_1        1
#define PIN_MAP_2        HD_VIDEOCAP_SEN_IGNORE
#define PIN_MAP_3        HD_VIDEOCAP_SEN_IGNORE
#elif (RESOLUTION_SET == 7)
#define SEN_NAME_ID      7
#define VDO_SIZE_W       2688
#define VDO_SIZE_H       1520
#define SEN_DRVIVER_NAME "nvt_sen_sc450ai"
#define SEN_IF_TYPE      HD_COMMON_VIDEO_IN_MIPI_CSI
#define SEN_PINMUX       0x220
#define SERIAL_IF_PINMUX 0x301
#define PIN_MAP_0        0
#define PIN_MAP_1        1
#define PIN_MAP_2        HD_VIDEOCAP_SEN_IGNORE
#define PIN_MAP_3        HD_VIDEOCAP_SEN_IGNORE
#elif (RESOLUTION_SET == 8)
#define SEN_NAME_ID      8
#define VDO_SIZE_W       2560
#define VDO_SIZE_H       1440
#define SEN_DRVIVER_NAME "nvt_sen_os04c10"
#define SEN_IF_TYPE      HD_COMMON_VIDEO_IN_MIPI_CSI
#define SEN_PINMUX       0x220
#define SERIAL_IF_PINMUX 0x301
#define PIN_MAP_0        0
#define PIN_MAP_1        1
#define PIN_MAP_2        HD_VIDEOCAP_SEN_IGNORE
#define PIN_MAP_3        HD_VIDEOCAP_SEN_IGNORE
#elif (RESOLUTION_SET == 9)
#define SEN_NAME_ID      9
#define VDO_SIZE_W       2048
#define VDO_SIZE_H       2048
#define SEN_DRVIVER_NAME "nvt_sen_os04e10"
#define SEN_IF_TYPE      HD_COMMON_VIDEO_IN_MIPI_CSI
#define SEN_PINMUX       0x220
#define SERIAL_IF_PINMUX 0x301
#define PIN_MAP_0        0
#define PIN_MAP_1        1
#define PIN_MAP_2        HD_VIDEOCAP_SEN_IGNORE
#define PIN_MAP_3        HD_VIDEOCAP_SEN_IGNORE
#elif (RESOLUTION_SET == 10)
#define SEN_NAME_ID      10
#define VDO_SIZE_W       2560
#define VDO_SIZE_H       1440
#define SEN_DRVIVER_NAME "nvt_sen_nt99436"
#define SEN_IF_TYPE      HD_COMMON_VIDEO_IN_MIPI_CSI
#define SEN_PINMUX       0x220
#define SERIAL_IF_PINMUX 0x301
#define PIN_MAP_0        0
#define PIN_MAP_1        1
#define PIN_MAP_2        HD_VIDEOCAP_SEN_IGNORE
#define PIN_MAP_3        HD_VIDEOCAP_SEN_IGNORE
#endif

#define CMD_IF_PINMUX     0x01;//PIN_I2C_CFG_CH1
#define CLK_LANE_SEL      HD_VIDEOCAP_SEN_CLANE_SEL_CSI0_USE_C0

#define PIN_MAP_4       HD_VIDEOCAP_SEN_IGNORE
#define PIN_MAP_5       HD_VIDEOCAP_SEN_IGNORE
#define PIN_MAP_6       HD_VIDEOCAP_SEN_IGNORE
#define PIN_MAP_7       HD_VIDEOCAP_SEN_IGNORE

///////////////////////////////////////////////////////////////////////////////

//header
#define DBGINFO_BUFSIZE()   (0x200)

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
#define VDO_NVX_BUFSIZE(w, h, pxlfmt)   (VDO_YUV_BUFSIZE(w, h, pxlfmt) * YUV_COMPRESS_RATIO / 100)

///////////////////////////////////////////////////////////////////////////////

#define SEN_OUT_FMT     HD_VIDEO_PXLFMT_RAW12
#define CAP_OUT_FMT     HD_VIDEO_PXLFMT_RAW12
#define CA_WIN_NUM_W        32
#define CA_WIN_NUM_H        32
#define LA_WIN_NUM_W        32
#define LA_WIN_NUM_H        32
#define VA_WIN_NUM_W        16
#define VA_WIN_NUM_H        16
#define YOUT_WIN_NUM_W  128
#define YOUT_WIN_NUM_H  128
#define ETH_8BIT_SEL        0 //0: 2bit out, 1:8 bit out
#define ETH_OUT_SEL     1 //0: full, 1: subsample 1/2

#define LCD_SIZE_W      960
#define LCD_SIZE_H      240


#define AE_PRESET_FUNC DISABLE
#if (AE_PRESET_FUNC == ENABLE)
#include "adc.h"
#define ADC_CH_PHOTORESISTOR         ADC_CHANNEL_0
#define ADC_READY_RETRY_CNT          (1000000/10) //1 sec
#endif


static UINT32 g_shdr_mode = 0;

static HD_RESULT mem_init(void)
{
	HD_RESULT              ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

	// config common pool (cap)
	mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[0].blk_size = DBGINFO_BUFSIZE() + VDO_RAW_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, CAP_OUT_FMT)
		+ VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
		+ VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
	mem_cfg.pool_info[0].blk_cnt = 2;
	mem_cfg.pool_info[0].ddr_id = DDR_ID0;
	// config common pool (main)
	mem_cfg.pool_info[1].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[1].blk_size = DBGINFO_BUFSIZE() + VDO_YUV_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[1].blk_cnt = 3;
	mem_cfg.pool_info[1].ddr_id = DDR_ID0;
	// config common pool (temp for sample of read flash)
	mem_cfg.pool_info[2].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[2].blk_size = 16 * 0x100000;
	mem_cfg.pool_info[2].blk_cnt = 1;
	mem_cfg.pool_info[2].ddr_id = DDR_ID0;

	ret = hd_common_mem_init(&mem_cfg);
	return ret;
}

///////////////////////////////////////////////////////////////////////////////

static HD_RESULT set_cap_cfg(HD_PATH_ID *p_video_cap_ctrl)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
	HD_PATH_ID video_cap_ctrl = 0;
	HD_VIDEOCAP_CTRL iq_ctl = {0};

	snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN - 1, SEN_DRVIVER_NAME);
	cap_cfg.sen_cfg.sen_dev.if_type = SEN_IF_TYPE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux =  0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = CLK_LANE_SEL;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = PIN_MAP_0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = PIN_MAP_1;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = PIN_MAP_2;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = PIN_MAP_3;
#if (RESOLUTION_SET == 0)
	if (g_shdr_mode == 1) {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
	}
#endif
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = PIN_MAP_4;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = PIN_MAP_5;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = PIN_MAP_6;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = PIN_MAP_7;

	printf("Using %s\n", cap_cfg.sen_cfg.sen_dev.driver_name);
	if (cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux == HD_COMMON_VIDEO_IN_MIPI_CSI) {
		printf("Parallel interface\n");
	} else if (cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux == HD_COMMON_VIDEO_IN_MIPI_CSI) {
		printf("MIPI interface\n");
	}
	if (g_shdr_mode == 1) {
		printf("Using g_shdr_mode\n");
	}
	ret = hd_videocap_open(0, HD_VIDEOCAP_0_CTRL, &video_cap_ctrl); //open this for device control

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
	{
		//select sensor mode, manually or automatically
		HD_VIDEOCAP_IN video_in_param = {0};

		video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO; //auto select sensor mode by the parameter of HD_VIDEOCAP_PARAM_OUT
		video_in_param.frc = HD_VIDEO_FRC_RATIO(30, 1);
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
		if (ret != HD_OK) {
			return ret;
		}
		//printf("set_cap_param CROP NONE=%d\r\n", ret);
	}
#else //HD_CROP_ON
	{
		HD_VIDEOCAP_CROP video_crop_param = {0};

		video_crop_param.mode = HD_CROP_ON;
		video_crop_param.win.rect.x = 0;
		video_crop_param.win.rect.y = 0;
		video_crop_param.win.rect.w = 1920 / 2;
		video_crop_param.win.rect.h = 1080 / 2;
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
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT, &video_out_param);
		//printf("set_cap_param OUT=%d\r\n", ret);
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////

static HD_RESULT set_proc_cfg(HD_PATH_ID *p_video_proc_ctrl, HD_DIM *p_max_dim)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
	HD_VIDEOPROC_CTRL video_ctrl_param = {0};
	HD_PATH_ID video_proc_ctrl = 0;

	ret = hd_videoproc_open(0, HD_VIDEOPROC_0_CTRL, &video_proc_ctrl); //open this for device control
	if (ret != HD_OK) {
		return ret;
	}

	if (p_max_dim != NULL) {
		video_cfg_param.pipe = HD_VIDEOPROC_PIPE_RAWALL;
		video_cfg_param.isp_id = 0;
		video_cfg_param.ctrl_max.func = 0;
		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;
		video_cfg_param.in_max.pxlfmt = CAP_OUT_FMT;
		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1, 1);
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

static HD_RESULT set_proc_param(HD_PATH_ID video_proc_path, HD_DIM *p_dim)
{
	HD_RESULT ret = HD_OK;

	if (p_dim != NULL) { //if videoproc is already binding to dest module, not require to setting this!
		HD_VIDEOPROC_OUT video_out_param = {0};
		video_out_param.func = 0;
		video_out_param.dim.w = p_dim->w;
		video_out_param.dim.h = p_dim->h;
		video_out_param.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		video_out_param.frc = HD_VIDEO_FRC_RATIO(1, 1);
		video_out_param.depth = 1; //set 1 to allow pull
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
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

	printf("out_type=%d\r\n", (int)out_type);

#if 1
	videoout_mode.output_type = HD_COMMON_VIDEO_OUT_LCD;
	videoout_mode.input_dim = HD_VIDEOOUT_IN_AUTO;
	videoout_mode.output_mode.lcd = HD_VIDEOOUT_LCD_0;
	if (out_type != 1) {
		printf("520 only support LCD\r\n");
	}
#else
	switch (out_type) {
	case 0:
		videoout_mode.output_type = HD_COMMON_VIDEO_OUT_CVBS;
		videoout_mode.input_dim = HD_VIDEOOUT_IN_AUTO;
		videoout_mode.output_mode.cvbs = HD_VIDEOOUT_CVBS_NTSC;
		break;
	case 1:
		videoout_mode.output_type = HD_COMMON_VIDEO_OUT_LCD;
		videoout_mode.input_dim = HD_VIDEOOUT_IN_AUTO;
		videoout_mode.output_mode.lcd = HD_VIDEOOUT_LCD_0;
		break;
	case 2:
		videoout_mode.output_type = HD_COMMON_VIDEO_OUT_HDMI;
		videoout_mode.input_dim = HD_VIDEOOUT_IN_AUTO;
		videoout_mode.output_mode.hdmi = hdmi_id;
		break;
	default:
		printf("not support out_type\r\n");
		break;
	}
#endif
	ret = hd_videoout_set(video_out_ctrl, HD_VIDEOOUT_PARAM_MODE, &videoout_mode);

	*p_video_out_ctrl = video_out_ctrl ;
	return ret;
}

static HD_RESULT get_out_caps(HD_PATH_ID video_out_ctrl, HD_VIDEOOUT_SYSCAPS *p_video_out_syscaps)
{
	HD_RESULT ret = HD_OK;
	HD_DEVCOUNT video_out_dev = {0};

	ret = hd_videoout_get(video_out_ctrl, HD_VIDEOOUT_PARAM_DEVCOUNT, &video_out_dev);
	if (ret != HD_OK) {
		return ret;
	}
	printf("##devcount %d\r\n", (int)video_out_dev.max_dev_count);

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

	video_out_param.dim.w = p_dim->w;
	video_out_param.dim.h = p_dim->h;
	video_out_param.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
	video_out_param.dir = HD_VIDEO_DIR_NONE;
	ret = hd_videoout_set(video_out_path, HD_VIDEOOUT_PARAM_IN, &video_out_param);
	if (ret != HD_OK) {
		return ret;
	}
	memset((void *)&video_out_param, 0, sizeof(HD_VIDEOOUT_IN));
	ret = hd_videoout_get(video_out_path, HD_VIDEOOUT_PARAM_IN, &video_out_param);
	if (ret != HD_OK) {
		return ret;
	}
	printf("##video_out_param w:%d,h:%d %x %x\r\n", (int)video_out_param.dim.w, (int)video_out_param.dim.h, (int)video_out_param.pxlfmt, (int)video_out_param.dir);

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
	HD_DIM  proc_dim;

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
} VIDEO_LIVEVIEW;

static VIDEO_LIVEVIEW stream[1] = {0}; //0: main stream
static UINT32 out_type = 1;

#if (AE_PRESET_FUNC == ENABLE)

static UINT32 ae_adc_tbl[18][3] = {
	{4096,  100, 1000},
	{4096,  100, 1000},
	{2030,  112, 1000},
	{1990,  207, 1000},
	{1952,  403, 1000},
	{1894,  781, 1000},
	{1066, 1584, 1000},
	{ 543, 3072, 1000},
	{ 278, 6176, 1000},
	{ 136, 10000, 1250},
	{  68, 20000, 1340},
	{  29, 30000, 1520},
	{  14, 30000, 3520},
	{   7, 30000, 6480},
	{   3, 30000, 13590},
	{   0, 30000, 13590},
	{   0, 30000, 13590},
	{   0, 30000, 13590},
};

void ae_adc_get_exp(UINT32 adc_value, UINT32 *exptime, UINT32 *isogain)
{
	UINT32 idx0, idx1;
	UINT32 exptime0, exptime1, isogain0, isogain1;
	UINT32 adc_ev, ev0, ev1;

	for (idx1 = 1; idx1 < 18; idx1++) {
		if (adc_value > ae_adc_tbl[idx1][0]) {
			break;
		}
	}

	idx0 = idx1 - 1;

	exptime0 = ae_adc_tbl[idx0][1];
	isogain0 = (ae_adc_tbl[idx0][2]);
	exptime1 = ae_adc_tbl[idx1][1];
	isogain1 = (ae_adc_tbl[idx1][2]);
	ev0 = (exptime0 * isogain0);
	ev1 = (exptime1 * isogain1);

	adc_ev = ((ev1 - ev0) * (adc_value - ae_adc_tbl[idx1][0])) / (ae_adc_tbl[idx0][0] - ae_adc_tbl[idx1][0]) + ev0;

	*exptime = exptime0;
	*isogain = (adc_ev / exptime0);

	//DBG_DUMP("idx = %d, %d, adc = %d, fast open preset exp = %d, %d\r\n", idx0, idx1, adc_value, *exptime, *isogain);
}


static void get_ae_preset(UINT32 *p_exp_time, UINT32 *p_gain_ratio)
{
	UINT32 uiADCValue, retry = 0;
	UINT32 i;
	UINT32 adc_value, adc_expt, adc_gain;
	UINT32 adc_avg, adc_cnt;
	UINT32 adc_mV;

	if (adc_open(ADC_CH_PHOTORESISTOR)) {
		printf("Can't open ADC channel for photoresistor\r\n");
		return;
	}
	adc_setChConfig(ADC_CH_PHOTORESISTOR, ADC_CH_CONFIG_ID_SAMPLE_FREQ, 10000); //10K Hz, sample once about 100 us for CONTINUOUS mode
	//adc_setChConfig(ADC_CH_PHOTORESISTOR, ADC_CH_CONFIG_ID_SAMPLE_MODE, ADC_CH_SAMPLEMODE_ONESHOT);
	adc_setChConfig(ADC_CH_PHOTORESISTOR, ADC_CH_CONFIG_ID_SAMPLE_MODE, ADC_CH_SAMPLEMODE_CONTINUOUS);
	adc_setChConfig(ADC_CH_PHOTORESISTOR, ADC_CH_CONFIG_ID_INTEN, FALSE);
	adc_setEnable(TRUE);
	adc_triggerOneShot(ADC_CH_PHOTORESISTOR);
	while (FALSE == adc_isDataReady(ADC_CH_PHOTORESISTOR) && retry < ADC_READY_RETRY_CNT) {
		vos_util_delay_us_polling(10);
		retry++;
	}

	adc_avg = 0;
	adc_cnt = 0;

	for (i = 0; i < 10; i++) {
		uiADCValue = adc_readData(ADC_CH_PHOTORESISTOR);
		adc_mV = adc_readVoltage(ADC_CH_PHOTORESISTOR);
		adc_avg += adc_mV;
		printf("photoresistor ADC = %d, voltage = %d\r\n", (unsigned int)uiADCValue, (unsigned int)adc_mV);
		vos_util_delay_us_polling(10);
		adc_cnt++;
	}

	adc_avg /= adc_cnt;

	adc_close(ADC_CH_PHOTORESISTOR);

	//convert ADC value to exp time and gain ratio
	adc_value = adc_avg;

	ae_adc_get_exp(adc_value, &adc_expt, &adc_gain);

	*p_exp_time = adc_expt;
	*p_gain_ratio = adc_gain;

	printf("adc_mV = %d, adc_exp =========== %d, %d\r\n", (unsigned int)adc_avg, (unsigned int)adc_expt, (unsigned int)adc_gain);

}
#endif
static void fast_open_sensor(void)
{
	VENDOR_VIDEOCAP_FAST_OPEN_SENSOR cap_cfg = {0};

	snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN - 1, SEN_DRVIVER_NAME);
	cap_cfg.sen_cfg.sen_dev.if_type = SEN_IF_TYPE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux =  SEN_PINMUX;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = CLK_LANE_SEL;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = PIN_MAP_0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = PIN_MAP_1;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = PIN_MAP_2;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = PIN_MAP_3;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = PIN_MAP_4;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = PIN_MAP_5;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = PIN_MAP_6;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = PIN_MAP_7;
	cap_cfg.sen_mode = 0;
	cap_cfg.frc = HD_VIDEO_FRC_RATIO(30, 1);
#if (AE_PRESET_FUNC == ENABLE)
	cap_cfg.ae_preset.enable = TRUE;
	get_ae_preset(&cap_cfg.ae_preset.exp_time, &cap_cfg.ae_preset.gain_ratio);
#endif
	vendor_videocap_set_lite(0, VENDOR_VIDEOCAP_PARAM_FAST_OPEN_SENSOR, &cap_cfg);
}

static HD_RESULT init_module_cap_proc(void)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_init()) != HD_OK) {
		return ret;
	}
	if ((ret = hd_videoproc_init()) != HD_OK) {
		return ret;
	}
	return HD_OK;
}

static HD_RESULT init_module_videoout(void)
{
	HD_RESULT ret;
	if ((ret = hd_videoout_init()) != HD_OK) {
		printf("failed to hd_videoout_init:%d\n", ret);
		return ret;
	}
	return HD_OK;
}

static HD_RESULT open_module_cap_proc(VIDEO_LIVEVIEW *p_stream, HD_DIM *p_proc_max_dim, UINT32 out_type)
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
	if ((ret = hd_videocap_open(HD_VIDEOCAP_0_IN_0, HD_VIDEOCAP_0_OUT_0, &p_stream->cap_path)) != HD_OK) {
		return ret;
	}

	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &p_stream->proc_path)) != HD_OK) {
		return ret;
	}

	return HD_OK;
}

static HD_RESULT open_module_videoout(VIDEO_LIVEVIEW *p_stream, HD_DIM *p_proc_max_dim, UINT32 out_type)
{
	HD_RESULT ret;
	// set videoout config
	ret = set_out_cfg(&p_stream->out_ctrl, out_type, p_stream->hdmi_id);
	if (ret != HD_OK) {
		printf("set out-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}

	if ((ret = hd_videoout_open(HD_VIDEOOUT_0_IN_0, HD_VIDEOOUT_0_OUT_0, &p_stream->out_path)) != HD_OK) {
		return ret;
	}

	return HD_OK;
}


static void *thread_sensor(void *ptr)
{
	fastboot_wait_done(BOOT_INIT_SENSOR);
	vos_perf_list_mark("ss", __LINE__, 0);
	//quick sensor setup flow
	fast_open_sensor();
	vos_perf_list_mark("ss", __LINE__, 1);
	pthread_exit((void *)0);
	return NULL;
}

static void *thread_cap_proc(void *ptr)
{
	HD_RESULT ret;

	// wait sensor setup ready
	int join_ret;
	pthread_t *p_handle_sensor = (pthread_t *)ptr;
	int pthread_ret = pthread_join(*p_handle_sensor, (void *)&join_ret);
	if (0 != pthread_ret) {
		printf("thread_sensor pthread_join failed, ret %d\r\n", pthread_ret);
		pthread_exit((void *) -1);
		return NULL;
	}
	// wait driver init ready
	fastboot_wait_done(BOOT_INIT_CAPTURE);
	vos_perf_list_mark("capproc", __LINE__, 0);

	stream[0].hdmi_id = HD_VIDEOOUT_HDMI_1920X1080I60; //default

	// init capture, proc
	ret = init_module_cap_proc();
	if (ret != HD_OK) {
		printf("init fail=%d\n", ret);
		pthread_exit((void *) -1);
		return NULL;
	}

	// open capture, proc
	stream[0].proc_max_dim.w = VDO_SIZE_W; //assign by user
	stream[0].proc_max_dim.h = VDO_SIZE_H; //assign by user
	ret = open_module_cap_proc(&stream[0], &stream[0].proc_max_dim, out_type);
	if (ret != HD_OK) {
		printf("open fail=%d\n", ret);
		pthread_exit((void *) -1);
		return NULL;
	}

	// set videocap parameter
	stream[0].cap_dim.w = VDO_SIZE_W; //assign by user
	stream[0].cap_dim.h = VDO_SIZE_H; //assign by user
	ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim);
	if (ret != HD_OK) {
		printf("set cap fail=%d\n", ret);
		pthread_exit((void *) -1);
		return NULL;
	}

	// set videoproc parameter (main)
	stream[0].proc_dim.w = LCD_SIZE_W;
	stream[0].proc_dim.h = LCD_SIZE_H;
	ret = set_proc_param(stream[0].proc_path, &stream[0].proc_dim);
	if (ret != HD_OK) {
		printf("set proc fail=%d\n", ret);
		pthread_exit((void *) -1);
		return NULL;
	}

	// bind video_liveview modules (main)
	hd_videocap_bind(HD_VIDEOCAP_0_OUT_0, HD_VIDEOPROC_0_IN_0);

	// start video_liveview modules (main)
	hd_videocap_start(stream[0].cap_path);
	vos_perf_list_mark("capproc", __LINE__, 1);

	hd_videoproc_start(stream[0].proc_path);

	// get first frame
	HD_VIDEO_FRAME video_frame = {0};
	hd_videoproc_pull_out_buf(stream[0].proc_path, &video_frame, -1);
	hd_videoproc_release_out_buf(stream[0].proc_path, &video_frame);

	vos_perf_list_mark("capproc", __LINE__, 2);
	pthread_exit((void *)0);
	return NULL;
}

static void *thread_videoout(void *ptr)
{
	HD_RESULT ret;

	fastboot_wait_done(BOOT_INIT_DISPLAY);
	vos_perf_list_mark("disp", __LINE__, 0);

	// init videoout
	ret = init_module_videoout();
	if (ret != HD_OK) {
		printf("init fail=%d\n", ret);
		pthread_exit((void *) -1);
		return NULL;
	}

	// open capture, proc modules (main)
	stream[0].proc_max_dim.w = VDO_SIZE_W; //assign by user
	stream[0].proc_max_dim.h = VDO_SIZE_H; //assign by user
	ret = open_module_videoout(&stream[0], &stream[0].proc_max_dim, out_type);
	if (ret != HD_OK) {
		printf("open fail=%d\n", ret);
		pthread_exit((void *) -1);
		return NULL;
	}

	// get videoout capability
	ret = get_out_caps(stream[0].out_ctrl, &stream[0].out_syscaps);
	if (ret != HD_OK) {
		printf("get out-caps fail=%d\n", ret);
		pthread_exit((void *) -1);
		return NULL;
	}
	stream[0].out_max_dim = stream[0].out_syscaps.output_dim;

	// set videoout parameter (main)
	stream[0].out_dim.w = stream[0].out_max_dim.w; //using device max dim.w
	stream[0].out_dim.h = stream[0].out_max_dim.h; //using device max dim.h
	ret = set_out_param(stream[0].out_path, &stream[0].out_dim);
	if (ret != HD_OK) {
		printf("set out fail=%d\n", ret);
		pthread_exit((void *) -1);
		return NULL;
	}

	if (hd_videoout_start(stream[0].out_path) != HD_OK) {
		printf("hd_videoout_start fail\n");
		pthread_exit((void *) -1);
		return NULL;
	}

	vos_perf_list_mark("disp", __LINE__, 1);
	pthread_exit((void *)0);
	return NULL;
}

static void *thread_preview(void *ptr)
{
	HD_RESULT ret;
	HD_VIDEO_FRAME video_frame = {0};
	while (1) {
		if ((ret = hd_videoproc_pull_out_buf(stream[0].proc_path, &video_frame, -1)) != HD_OK) {
			printf("failed to hd_videoproc_pull_out_buf, er=%d\n", (int)ret);
			pthread_exit((void *)0);
			return NULL;
		}
		if ((ret = hd_videoout_push_in_buf(stream[0].out_path, &video_frame, NULL, 0)) != HD_OK) {
			//printf("videoout drop one frame, er=%d\n", (int)ret);
		}
		if ((ret = hd_videoproc_release_out_buf(stream[0].proc_path, &video_frame)) != HD_OK) {
			printf("failed to hd_videoproc_release_out_buf, er=%d\n", (int)ret);
			pthread_exit((void *)0);
			return NULL;
		}
	}

	pthread_exit((void *)0);
	return NULL;
}

int flow_preview(void)
{
	int ret;
	int pthread_ret;
	int join_ret;
	pthread_t handle_sensor;
	pthread_t handle_cap_proc;
	pthread_t handle_videoout;
	pthread_t handle_preview;
	int policy;
	struct sched_param schedparam = {0};


	// quick sensor setup
	pthread_ret = pthread_create(&handle_sensor, NULL, thread_sensor, NULL);
	if (0 != pthread_ret) {
		printf("create thread_sensor failed, ret %d\r\n", pthread_ret);
		return -1;
	}
	if (0 != pthread_getschedparam(handle_sensor, &policy, &schedparam)) {
		printf("pthread_getschedparam failed\r\n");
	} else {
		schedparam.sched_priority = 20;
		pthread_setschedparam(handle_sensor, policy, &schedparam);
	}

	//hd_common_init(including vds), must wait until insmod_capture()
	fastboot_wait_done(BOOT_INIT_CAPTURE);

	// init hdal
	ret = hd_common_init(0);
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
		return -1;
	}

	// init memory
	ret = mem_init();
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
		return -1;
	}

	pthread_ret = pthread_create(&handle_cap_proc, NULL, thread_cap_proc, (void *)&handle_sensor);
	if (0 != pthread_ret) {
		printf("create thread_cap_proc failed, ret %d\r\n", pthread_ret);
		return -1;
	}
	if (0 != pthread_getschedparam(handle_cap_proc, &policy, &schedparam)) {
		printf("pthread_getschedparam failed\r\n");
	} else {
		schedparam.sched_priority = 20;
		pthread_setschedparam(handle_cap_proc, policy, &schedparam);
	}


	pthread_ret = pthread_create(&handle_videoout, NULL, thread_videoout, NULL);
	if (0 != pthread_ret) {
		printf("create thread_videoout failed, ret %d\r\n", pthread_ret);
		return -1;
	}
	if (0 != pthread_getschedparam(handle_videoout, &policy, &schedparam)) {
		printf("pthread_getschedparam failed\r\n");
	} else {
		schedparam.sched_priority = 19;
		pthread_setschedparam(handle_videoout, policy, &schedparam);
	}

	pthread_ret = pthread_join(handle_cap_proc, (void *)&join_ret);
	if (0 != pthread_ret) {
		printf("thread_cap_proc pthread_join failed, ret %d\r\n", pthread_ret);
		return -1;
	}

	pthread_ret = pthread_join(handle_videoout, (void *)&join_ret);
	if (0 != pthread_ret) {
		printf("thread_videoout pthread_join failed, ret %d\r\n", pthread_ret);
		return -1;
	}

	// start video_liveview on lcd
	pthread_ret = pthread_create(&handle_preview, NULL, thread_preview, NULL);
	if (0 != pthread_ret) {
		printf("create thread_preview failed, ret %d\r\n", pthread_ret);
		return -1;
	}
	return 0;
}
