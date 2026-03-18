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
#include "flow_encode.h"
#include "vendor_videocapture.h"

#define ENC_TYPE 2 // 0:H265 1:H264 2:JPEG
#define AE_SKIP_FRAME 17 // Novatek AE shall process 17 frame for stable.
//#define FILE_WRITE

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

#if (RESOLUTION_SET == 0)
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
#elif (RESOLUTION_SET == 1)
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
#elif (RESOLUTION_SET == 4)
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
#elif (RESOLUTION_SET == 5)
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
		if (ret != HD_OK) {
			return ret;
		}
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

static HD_RESULT set_enc_cfg(HD_PATH_ID video_enc_path, HD_DIM *p_max_dim, UINT32 max_bitrate)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOENC_PATH_CONFIG video_path_config = {0};

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
		video_path_config.isp_id             = 0;
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_PATH_CONFIG, &video_path_config);
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
	HD_VIDEOENC_OUT2 video_out_param = {0};
	HD_H26XENC_RATE_CONTROL2 rc_param = {0};

	if (p_dim != NULL) {

		//--- HD_VIDEOENC_PARAM_IN ---
		video_in_param.dir           = HD_VIDEO_DIR_NONE;
		video_in_param.pxl_fmt = HD_VIDEO_PXLFMT_YUV420;
		video_in_param.dim.w   = p_dim->w;
		video_in_param.dim.h   = p_dim->h;
		video_in_param.frc     = HD_VIDEO_FRC_RATIO(1, 1);
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
			video_out_param.h26x.gop_num       = 15;
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
			video_out_param.h26x.gop_num       = 15;
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
	HD_DIM  proc_max_dim;

	// (2)
	HD_VIDEOPROC_SYSCAPS proc_syscaps;
	HD_PATH_ID proc_ctrl;
	HD_PATH_ID proc_path;

	HD_DIM  enc_max_dim;
	HD_DIM  enc_dim;

	// (3)
	HD_VIDEOENC_SYSCAPS enc_syscaps;
	HD_PATH_ID enc_path;

	// (4) user pull
	pthread_t  enc_thread_id;
	UINT32     enc_exit;
	UINT32     flow_start;
	UINT32     enc_start;

} VIDEO_RECORD;

//static VIDEO_LIVEVIEW stream[1] = {0}; //0: main stream
//static UINT32 out_type = 1;
static VIDEO_RECORD stream[1] = {0}; //0: main stream

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

static HD_RESULT init_module_enc(void)
{
	HD_RESULT ret;
	if ((ret = hd_videoenc_init()) != HD_OK) {
		return ret;
	}
	return HD_OK;
}

static HD_RESULT open_module_cap_proc(VIDEO_RECORD *p_stream, HD_DIM *p_proc_max_dim)
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

static HD_RESULT open_module_enc(VIDEO_RECORD *p_stream, HD_DIM *p_proc_max_dim)
{
	HD_RESULT ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_VIDEOENC_0_OUT_0, &p_stream->enc_path)) != HD_OK) {
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
	ret = open_module_cap_proc(&stream[0], &stream[0].proc_max_dim);
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
	ret = set_proc_param(stream[0].proc_path, &stream[0].cap_dim);
	if (ret != HD_OK) {
		printf("set proc fail=%d\n", ret);
		pthread_exit((void *) -1);
		return NULL;
	}

	hd_videocap_bind(HD_VIDEOCAP_0_OUT_0, HD_VIDEOPROC_0_IN_0);

	vos_perf_list_mark("capproc", __LINE__, 1);
	hd_videocap_start(stream[0].cap_path);
	vos_perf_list_mark("capproc", __LINE__, 2);
	hd_videoproc_start(stream[0].proc_path);
	vos_perf_list_mark("capproc", __LINE__, 3);

	HD_VIDEO_FRAME video_frame = {0};
	UINT32 skip = 0;

	// AE shall process AE_SKIP_FRAME frame for stable.
	while (skip < (AE_SKIP_FRAME - 1) /*16*/) {
		hd_videoproc_pull_out_buf(stream[0].proc_path, &video_frame, -1);

		if (skip == 0) {
			vos_perf_list_mark("capproc", __LINE__, 4);
		}

		hd_videoproc_release_out_buf(stream[0].proc_path, &video_frame);
		skip++;
	}
	// skip to AE_SKIP_FRAME-1 and start encode
	stream[0].enc_start = 1;

	vos_perf_list_mark("capproc", __LINE__, 5);
	printf("skip: %d\r\n", (int)skip);

	pthread_exit((void *)0);
	return NULL;
}

static void *thread_enc(void *ptr)
{
	VIDEO_RECORD *p_stream0 = (VIDEO_RECORD *)ptr;
	HD_RESULT ret;
	UINT32 enc_type = 0;

	// wait driver init ready

	fastboot_wait_done(BOOT_INIT_MEIDA_ENCODER); // Vanness can seperate to other thread.
	printf("##Wait init encode: done!!!!!\r\n");

	// init capture, proc
	ret = init_module_enc();
	if (ret != HD_OK) {
		printf("init fail=%d\n", ret);
		pthread_exit((void *) -1);
		return NULL;
	}

	// open capture, proc
	stream[0].proc_max_dim.w = VDO_SIZE_W; //assign by user
	stream[0].proc_max_dim.h = VDO_SIZE_H; //assign by user
	ret = open_module_enc(&stream[0], &stream[0].proc_max_dim);
	if (ret != HD_OK) {
		printf("open fail=%d\n", ret);
		pthread_exit((void *) -1);
		return NULL;
	}

	// set videoenc config (main)
	stream[0].enc_max_dim.w = VDO_SIZE_W;
	stream[0].enc_max_dim.h = VDO_SIZE_H;
	ret = set_enc_cfg(stream[0].enc_path, &stream[0].enc_max_dim, 2 * 1024 * 1024);
	if (ret != HD_OK) {
		printf("set enc-cfg fail=%d\n", ret);
		pthread_exit((void *) -1);
		return NULL;
	}

	// set videoenc parameter (main)
	stream[0].enc_dim.w = VDO_SIZE_W;
	stream[0].enc_dim.h = VDO_SIZE_H;
	enc_type = ENC_TYPE;
	ret = set_enc_param(stream[0].enc_path, &stream[0].enc_dim, enc_type, 2 * 1024 * 1024);
	if (ret != HD_OK) {
		printf("set enc fail=%d\n", ret);
		pthread_exit((void *) -1);
		return NULL;
	}

	// bind video_record modules (main)
	hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOENC_0_IN_0);

	//------ wait enc_start ------
	while (p_stream0->enc_start == 0) {
		usleep(3 * 1000);
	}

	hd_videoenc_start(stream[0].enc_path);

	pthread_exit((void *)0);
	return NULL;
}

static void *encode_thread(void *arg)
{
	VIDEO_RECORD *p_stream0 = (VIDEO_RECORD *)arg;
	HD_RESULT ret = HD_OK;
	HD_VIDEOENC_BS  data_pull;
#ifdef FILE_WRITE
	UINT32 j;
#endif
	ULONG vir_addr_main;
	HD_VIDEOENC_BUFINFO phy_buf_main;
#ifdef FILE_WRITE
	char file_path_main[32] = "/mnt/sd/dump_bs_main.dat";
	FILE *f_out_main;
#endif
#define PHY2VIRT_MAIN(pa) (vir_addr_main + (pa - phy_buf_main.buf_info.phy_addr))

	vos_perf_list_mark("enc", __LINE__, 0);

	//------ wait flow_start ------
	while (p_stream0->flow_start == 0) {
		usleep(3 * 1000);
	}

	vos_perf_list_mark("enc", __LINE__, 1);

	// query physical address of bs buffer ( this can ONLY query after hd_videoenc_start() is called !! )
	hd_videoenc_get(p_stream0->enc_path, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf_main);

	// mmap for bs buffer (just mmap one time only, calculate offset to virtual address later)
	vir_addr_main = (ULONG)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_main.buf_info.phy_addr, phy_buf_main.buf_info.buf_size);

	//----- open output files -----
#ifdef FILE_WRITE
	if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
		HD_VIDEOENC_ERR("open file (%s) fail....\r\n", file_path_main);
	} else {
		printf("\r\ndump main bitstream to file (%s) ....\r\n", file_path_main);
	}
#endif

	printf("\r\nif you want to stop, enter \"q\" to exit !!\r\n\r\n");

#ifdef FILE_WRITE
	static int count = 0;
#endif
	//--------- pull data test ---------
	while (p_stream0->enc_exit == 0) {
		//pull data
		ret = hd_videoenc_pull_out_buf(p_stream0->enc_path, &data_pull, -1); // -1 = blocking mode
		if (ret == HD_OK) {
#ifdef FILE_WRITE
			for (j = 0; j < data_pull.pack_num; j++) {
				UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(data_pull.video_pack[j].phy_addr);
				UINT32 len = data_pull.video_pack[j].size;
				if (f_out_main) {
					fwrite(ptr, 1, len, f_out_main);
				}
				if (f_out_main) {
					fflush(f_out_main);
				}
			}
#endif
			vos_perf_list_mark("enc", __LINE__, 2);

#ifdef FILE_WRITE
			printf("hd_videoenc_pull_out_buf: success!(%d)\r\n", count);
#endif
			// release data
			ret = hd_videoenc_release_out_buf(p_stream0->enc_path, &data_pull);
			if (ret != HD_OK) {
				printf("enc_release error=%d !!\r\n", ret);
			}
		}


#ifdef FILE_WRITE
		if (count > 150) {
			p_stream0->enc_exit = 1;
			printf("encode_thread: exit!\r\n");
		}
		count++;
#else
		p_stream0->enc_exit = 1;
		printf("encode_thread: exit!\r\n");
#endif
	}

	// mummap for bs buffer
	if (vir_addr_main) {
		if (hd_common_mem_munmap((void *)vir_addr_main, phy_buf_main.buf_info.buf_size) != 0) {
			printf("failed to hd_common_mem_munmap.\r\n");
		}
	}

	// close output file
#ifdef FILE_WRITE
	if (f_out_main) {
		fclose(f_out_main);
	}
#endif

	pthread_exit((void *)0);
	return NULL;
}

int flow_encode(void)
{
	int ret;
	int pthread_ret;
	int join_ret;
	pthread_t handle_sensor;
	pthread_t handle_cap_proc;
	pthread_t handle_enc;
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

	// Create cap_proc
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

	// Start vidoe encode
	// create encode_thread (pull_out bitstream)
	ret = pthread_create(&stream[0].enc_thread_id, NULL, encode_thread, (void *)stream);
	if (ret < 0) {
		printf("create encode thread failed");
		return -1;
	}

	// Create enc
	pthread_ret = pthread_create(&handle_enc, NULL, thread_enc, (void *)stream);
	if (0 != pthread_ret) {
		printf("create handle_enc failed, ret %d\r\n", pthread_ret);
		return -1;
	}
	if (0 != pthread_getschedparam(handle_enc, &policy, &schedparam)) {
		printf("pthread_getschedparam failed\r\n");
	} else {
		schedparam.sched_priority = 19;
		pthread_setschedparam(handle_enc, policy, &schedparam);
	}

	// Join cap_proc
	pthread_ret = pthread_join(handle_cap_proc, (void *)&join_ret);
	if (0 != pthread_ret) {
		printf("thread_cap_proc pthread_join failed, ret %d\r\n", pthread_ret);
		return -1;
	}

	// Join enc
	pthread_ret = pthread_join(handle_enc, (void *)&join_ret);
	if (0 != pthread_ret) {
		printf("handle_enc pthread_join failed, ret %d\r\n", pthread_ret);
		return -1;
	}

	// let encode_thread start to work
	stream[0].flow_start = 1;

	// destroy encode thread
	pthread_ret = pthread_join(stream[0].enc_thread_id, NULL);
	if (0 != pthread_ret) {
		printf("thread_enc pthread_join failed, ret %d\r\n", pthread_ret);
		return -1;
	}

	return 0;
}
