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
#include "hdal.h"
#include "hd_debug.h"
#include "vendor_videoenc.h"
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
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(hd_video_record, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define DEBUG_MENU 		1

#define CHKPNT			printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
#define DBGH(x)			printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x)			printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)

#define SEN1_VCAP_ID 0

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
// MD
#define MD_HEAD_BUFSIZE()	            (0x40)

// Note , the md info w, h is vprc input w, h not out w, h
#define MD_INFO_BUFSIZE(w, h)           (ALIGN_CEIL_64((((w + 511) >> 9) << 2) * ((h + 15) >> 4)))




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

#define ENABLE          1
#define DISABLE         0

#define FUNC_RELAY		    DISABLE // DISABLE: bind / ENABLE: NO bind, using relay_yuv_thread to pull_out then push_in


#define VIDEOCAP_ALG_FUNC HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB
#define VIDEOPROC_ALG_FUNC HD_VIDEOPROC_FUNC_SHDR | HD_VIDEOPROC_FUNC_WDR | HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_COLORNR | HD_VIDEOPROC_FUNC_DEFOG

static UINT32 g_3dnr = 1; //0:disable, 1:enable 3DNR
static UINT32 g_frameCnt = 0;
/////////////////////////////  SMART ENCODE CFG  /////////////////////////////
#define SHOW_BBOX                  1 // input airoi bbox
#define SMART_ROI_EN               0
#define AQ_EN                      0
#define MAQ_DIFF_EN                1
#define BGRDO_EN                   0
#define RRC_EN                     0
#define RC_MODE                    2 //0: CBR, 1: VBR, 2: EVBR
#define DYGOP_EN                   1
#define DYFR_EN                    1
#define FPS                        30
#define GOP                        60
#define MD_MODE                    0 //0: INTERNAL MD, 1: EXTERNAL MD
#define PRIVACY_MASK_EN            0 //you should turn off smart roi/bgrdo/dygop/dyfr when enable privacy mask
#define WRAP_SEI                   0 //wrap private frame with SEI
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

	// (5) user pull
	pthread_t  relay_thread_id;
	UINT32     proc_exit;

#if SHOW_BBOX
	pthread_t  bbox_thread_id;
	UINT32     bbox_exit;
	UINT32     flow_bbox_start;
#endif 
} VIDEO_RECORD;

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
	mem_cfg.pool_info[0].blk_cnt = 2;
	mem_cfg.pool_info[0].ddr_id = DDR_ID0;
	// config common pool (main)
	mem_cfg.pool_info[1].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[1].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, HD_VIDEO_PXLFMT_YUV420)
		                                             +MD_HEAD_BUFSIZE()
                                                     +MD_INFO_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H);
	mem_cfg.pool_info[1].blk_cnt = 3;
	mem_cfg.pool_info[1].ddr_id = DDR_ID0;

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

static HD_RESULT get_cap_sysinfo(HD_PATH_ID video_cap_ctrl)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOCAP_SYSINFO sys_info = {0};

	hd_videocap_get(video_cap_ctrl, HD_VIDEOCAP_PARAM_SYSINFO, &sys_info);
	printf("sys_info.devid =0x%X, cur_fps[0]=%d/%d, vd_count=%llu\r\n", sys_info.dev_id, GET_HI_UINT16(sys_info.cur_fps[0]), GET_LO_UINT16(sys_info.cur_fps[0]), sys_info.vd_count);
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
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);
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
		if (g_3dnr == 1) {
			video_cfg_param.ctrl_max.func |= HD_VIDEOPROC_FUNC_3DNR;
		}
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
	if (g_3dnr == 1) {
		video_ctrl_param.func |= HD_VIDEOPROC_FUNC_3DNR;
		video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_0_OUT_0;
	}
	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);
	*p_video_proc_ctrl = video_proc_ctrl;

	return ret;
}


static HD_RESULT set_proc_param(HD_PATH_ID video_proc_path, HD_DIM* p_dim, HD_VIDEO_PXLFMT pxlfmt, BOOL is_pull)
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
		video_out_param.depth = is_pull ? 1 : 0;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	} else {
		HD_VIDEOPROC_OUT video_out_param = {0};
		video_out_param.func = 0;
		video_out_param.dim.w = 0;
		video_out_param.dim.h = 0;
		video_out_param.pxlfmt = pxlfmt;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		video_out_param.frc = HD_VIDEO_FRC_RATIO(1,1);
		video_out_param.depth = is_pull ? 1 : 0;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	}
	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		video_path_param.out_func = HD_VIDEOPROC_OUTFUNC_MD;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
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
		video_path_config.isp_id             = SEN1_VCAP_ID;
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
	HD_VIDEOENC_OUT video_out_param = {0};
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

		//--- VENDOR_VIDEOENC_PARAM_OUT_H26X_PRIVACY ---
		{
			VENDOR_VIDEOENC_H26X_PRIVACY_CFG priv_cfg = {0};
			priv_cfg.enable = PRIVACY_MASK_EN;
			vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_H26X_PRIVACY, &priv_cfg);
		}

		//--- VENDOR_VIDEOENC_PARAM_OUT_SMART_ROI ---
		{
			VENDOR_VIDEOENC_SMART_ROI smart_roi = {0};
			smart_roi.enable = SMART_ROI_EN;
			smart_roi.mode = 0;
			smart_roi.fg_str[0] = 26;
			smart_roi.fg_str[1] = 55;
			smart_roi.fg_str[2] = 50;
			smart_roi.fg_str[3] = 0;
			smart_roi.fg_str[4] = 0;
			vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_SMART_ROI, &smart_roi);
		}

		//--- VENDOR_VIDEOENC_PARAM_OUT_DYNAMIC_GOP ---
		{
			VENDOR_VIDEOENC_DYNAMIC_GOP_CFG dygop = {0};
			dygop.enable = DYGOP_EN;
			dygop.max_gop = 300;
			dygop.motion_sensitivity = 8;
			ret = vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_DYNAMIC_GOP, &dygop);
			if (ret != HD_OK) {
				printf("set vendor_enc_param_in = %d\r\n", ret);
				return ret;
			}
		}

		//--- VENDOR_VIDEOENC_PARAM_OUT_DYNAMIC_FR ---
		{
			VENDOR_VIDEOENC_DYNAMIC_FR_CFG dyfr = {0};
			dyfr.enable = DYFR_EN;
			dyfr.min_fps = 10;
			dyfr.motion_sensitivity = 12;
			ret = vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_DYNAMIC_FR, &dyfr);
			if (ret != HD_OK) {
				printf("set vendor_enc_param_in = %d\r\n", ret);
				return ret;
			}
		}
		//--- HD_VIDEOENC_PARAM_OUT_AQ ---
		HD_H26XENC_AQ aq_param = {
			.enable = AQ_EN,
			.i_str = 2,
			.p_str = 2,
			.min_delta_qp = -4,
			.max_delta_qp = 4,
			.depth = 2,
			.thd_table	= {-120,-112,-104, -96, -88, -80, -72, -64, -56, -48, -40, -32, -24, -16, -8, 7, 15, 23, 31, 39,47, 55, 63, 71, 79, 87, 95, 103, 111, 119}
		};
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_AQ, &aq_param);
		if (ret != HD_OK) {
			printf("set_enc_aq = %d \n", ret);
			return ret;
		}

		//--- VENDOR_VIDEOENC_PARAM_OUT_H26X_MAQ_DIFF ---
		VENDOR_VIDEOENC_H26X_MAQ_DIFF maq_diff;
		maq_diff.b_enable = MAQ_DIFF_EN;
		maq_diff.str = -4;
		maq_diff.start_idx = 1;
		maq_diff.end_idx = 8;
		printf("set HD_VIDEOENC_PARAM_OUT MAQ_DIFF ! \n");
		ret = vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_H26X_MAQ_DIFF , &maq_diff);
		if (ret != HD_OK) {
			printf("set_enc_maq_diff = %d \n", ret);
			return ret;
		}

		//--- VENDOR_VIDEOENC_PARAM_OUT_BG_RDO ---
		VENDOR_VIDEOENC_BG_RDO_CFG bg_rdo = {0};
		bg_rdo.b_enable = BGRDO_EN;
		bg_rdo.avc_bg_skip_bias = 0;
		bg_rdo.hevc_bg_skip_bias = -16;
		bg_rdo.hevc_bg_merge_bias = 0;
		bg_rdo.bg_bias_shift = 0;
		bg_rdo.mode = 0;
		printf("set HD_VIDEOENC_PARAM_OUT BG_RDO ! \n");
		ret = vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_BG_RDO, &bg_rdo);
		if (ret != HD_OK) {
			printf("set_enc_bg_rdo = %d \n", ret);
			return ret;
		}
		//--- VENDOR_VIDEOENC_PARAM_OUT_RRC ---
		HD_H26XENC_ROW_RC rowrc = {0};	
		rowrc.enable = RRC_EN;
		rowrc.i_qp_range = 2;
		rowrc.i_qp_step = 1; 
		rowrc.p_qp_range = 4;
		rowrc.p_qp_step = 1; 
		rowrc.min_i_qp = 1;  
		rowrc.max_i_qp = 51;  
		rowrc.min_p_qp = 1;  
		rowrc.max_p_qp = 51;  
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ROW_RC, &rowrc);
		if (ret != HD_OK) {
			printf("set_rrc_param = %d\r\n", ret);
			return ret;
		}

		printf("enc_type=%d\r\n", enc_type);

		if (enc_type == 0) {

			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_H265;
			video_out_param.h26x.profile       = HD_H265E_MAIN_PROFILE;
			video_out_param.h26x.level_idc     = HD_H265E_LEVEL_5;
			video_out_param.h26x.gop_num       = GOP;
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
#if RC_MODE == 0
			rc_param.rc_mode             = HD_RC_MODE_CBR;
			rc_param.cbr.bitrate         = bitrate;
			rc_param.cbr.frame_rate_base = 30;
			rc_param.cbr.frame_rate_incr = 1;
			rc_param.cbr.init_i_qp       = 26;
			rc_param.cbr.min_i_qp        = 20;
			rc_param.cbr.max_i_qp        = 45;
			rc_param.cbr.init_p_qp       = 30;
			rc_param.cbr.min_p_qp        = 20;
			rc_param.cbr.max_p_qp        = 45;
			rc_param.cbr.static_time     = 4;
#elif RC_MODE == 1
			rc_param.rc_mode              = HD_RC_MODE_VBR;
			rc_param.vbr.bitrate          = bitrate;
			rc_param.vbr.frame_rate_base  = 30;
			rc_param.vbr.frame_rate_incr  = 1;
			rc_param.vbr.init_i_qp        = 28;
			rc_param.vbr.max_i_qp         = 51;
			rc_param.vbr.min_i_qp         = 1;
			rc_param.vbr.init_p_qp        = 36;
			rc_param.vbr.max_p_qp         = 51;
			rc_param.vbr.min_p_qp         = 1;
			rc_param.vbr.static_time      = 0;
			rc_param.vbr.ip_weight        = 0;
#elif RC_MODE == 2
			rc_param.rc_mode              = HD_RC_MODE_EVBR;
			rc_param.evbr.bitrate         = bitrate;
			rc_param.evbr.frame_rate_base = FPS;
			rc_param.evbr.frame_rate_incr = 1;
			rc_param.evbr.init_i_qp       = 28;
			rc_param.evbr.min_i_qp        = 1; 
			rc_param.evbr.max_i_qp        = 51;
			rc_param.evbr.init_p_qp       = 34;
			rc_param.evbr.min_p_qp        = 1; 
			rc_param.evbr.max_p_qp        = 51;
			rc_param.evbr.static_time     = 0;  // compensation for the bitrate (uints: GOP)
			rc_param.evbr.ip_weight       = 0;
			rc_param.evbr.motion_aq_str   = -6;
			rc_param.evbr.still_frame_cnd = 100;
			rc_param.evbr.motion_ratio_thd = 50;
			rc_param.evbr.still_i_qp      = 28;
			rc_param.evbr.still_p_qp      = 34;
			rc_param.evbr.key_p_period    = 0;
#endif
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL, &rc_param);
			if (ret != HD_OK) {
				printf("set_enc_rate_control = %d\r\n", ret);
				return ret;
			}
		} else if (enc_type == 1) {

			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_H264;
			video_out_param.h26x.profile       = HD_H264E_HIGH_PROFILE;
			video_out_param.h26x.level_idc     = HD_H264E_LEVEL_5_1;
			video_out_param.h26x.gop_num       = GOP;
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
#if RC_MODE == 0
			rc_param.rc_mode             = HD_RC_MODE_CBR;
			rc_param.cbr.bitrate         = bitrate;
			rc_param.cbr.frame_rate_base = 30;
			rc_param.cbr.frame_rate_incr = 1;
			rc_param.cbr.init_i_qp       = 26;
			rc_param.cbr.min_i_qp        = 20;
			rc_param.cbr.max_i_qp        = 45;
			rc_param.cbr.init_p_qp       = 30;
			rc_param.cbr.min_p_qp        = 20;
			rc_param.cbr.max_p_qp        = 45;
			rc_param.cbr.static_time     = 4;
#elif RC_MODE == 1
			rc_param.rc_mode              = HD_RC_MODE_VBR;
			rc_param.vbr.bitrate          = bitrate;
			rc_param.vbr.frame_rate_base  = 30;
			rc_param.vbr.frame_rate_incr  = 1;
			rc_param.vbr.init_i_qp        = 28;
			rc_param.vbr.max_i_qp         = 51;
			rc_param.vbr.min_i_qp         = 1;
			rc_param.vbr.init_p_qp        = 36;
			rc_param.vbr.max_p_qp         = 51;
			rc_param.vbr.min_p_qp         = 1;
			rc_param.vbr.static_time      = 0;
			rc_param.vbr.ip_weight        = 0;
#elif RC_MODE == 2
			rc_param.rc_mode              = HD_RC_MODE_EVBR;
			rc_param.evbr.bitrate         = bitrate;
			rc_param.evbr.frame_rate_base = FPS;
			rc_param.evbr.frame_rate_incr = 1;
			rc_param.evbr.init_i_qp       = 28;
			rc_param.evbr.min_i_qp        = 1; 
			rc_param.evbr.max_i_qp        = 51;
			rc_param.evbr.init_p_qp       = 34;
			rc_param.evbr.min_p_qp        = 1; 
			rc_param.evbr.max_p_qp        = 51;
			rc_param.evbr.static_time     = 0;  // compensation for the bitrate (uints: GOP)
			rc_param.evbr.ip_weight       = 0;
			rc_param.evbr.motion_aq_str   = -6;
			rc_param.evbr.still_frame_cnd = 100;
			rc_param.evbr.motion_ratio_thd = 50;
			rc_param.evbr.still_i_qp      = 28;
			rc_param.evbr.still_p_qp      = 34;
			rc_param.evbr.key_p_period    = 0;
#endif
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
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim);
	if (ret != HD_OK) {
		printf("set proc-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}

	if ((ret = hd_videocap_open(HD_VIDEOCAP_IN(SEN1_VCAP_ID, 0), HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), &p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &p_stream->proc_path)) != HD_OK)
		return ret;
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
	return HD_OK;
}

#if (FUNC_RELAY == ENABLE)
static void *relay_yuv_thread(void *arg)
{
	VIDEO_RECORD* p_stream0 = (VIDEO_RECORD *)arg;
	//DEMO* p_stream1 = p_stream0 + 1;
	HD_RESULT ret = HD_OK;
	HD_VIDEO_FRAME video_frame = {0};
	HD_VIDEOPROC_POLL_LIST poll_list[2];

	//------ wait flow_start ------
	while (p_stream0->flow_start == 0) sleep(1);

	printf("\r\relay main yuv  ....\r\n");
	//printf("\r\relay sub yuv  ....\r\n");

	//--------- pull data test ---------
	poll_list[0].path_id = p_stream0->proc_path;
	//poll_list[1].path_id = p_stream1->proc_path;

	//--------- pull data test ---------
	while (p_stream0->proc_exit == 0) {
		if (HD_OK == hd_videoproc_poll_list(poll_list, 1, -1)) {    // multi path poll_list , -1 = blocking mode
			if (TRUE == poll_list[0].revent.event) {
				ret = hd_videoproc_pull_out_buf(p_stream0->proc_path, &video_frame, -1); // -1 = blocking mode
				if (ret != HD_OK) {
					printf("pull_out [0] error !!\r\n\r\n");
					continue;
				}
				//printf("video_frame.p_next = 0x%x!!\r\n", (int)video_frame.p_next);

				ret = hd_videoenc_push_in_buf(p_stream0->enc_path, &video_frame, NULL, -1); // -1 = blocking mode
				if (ret != HD_OK) {
					printf("push_in [0] error !!\r\n");
				}

				ret = hd_videoproc_release_out_buf(p_stream0->proc_path, &video_frame);
				if (ret != HD_OK) {
					printf("release_out [0] error !!\r\n\r\n");
				}
			}
			#if 0
			if (TRUE == poll_list[1].revent.event) {
				ret = hd_videoproc_pull_out_buf(p_stream1->proc_path, &video_frame, -1); // -1 = blocking mode
				if (ret != HD_OK) {
					printf("pull_out [1] error !!\r\n\r\n");
					continue;
				}

				ret = hd_videoenc_push_in_buf(p_stream1->enc_path, &video_frame, NULL, -1); // -1 = blocking mode
				if (ret != HD_OK) {
					printf("push_in [1] error !!\r\n");
				}

				ret = hd_videoproc_release_out_buf(p_stream1->proc_path, &video_frame);
				if (ret != HD_OK) {
					printf("release_out [1] error !!\r\n\r\n");
				}
			}
			#endif
		}
	}

	return 0;
}
#endif

#if WRAP_SEI
UINT32 wrap_sei(UINT8 *out_stream, const UINT8 *data, UINT32 eCodecType, int length)
{
	int byte_pos = 0;
	int tmp = length;
	int i;
	int counter = 0;

	if (eCodecType == HD_CODEC_TYPE_H265) {
		// 1. encode start code & SEI header
		out_stream[byte_pos++] = 0x00;
		out_stream[byte_pos++] = 0x00;
		out_stream[byte_pos++] = 0x00;
		out_stream[byte_pos++] = 0x01;
		out_stream[byte_pos++] = 0x50;	// type SEI
		out_stream[byte_pos++] = 0x01;	// type SEI
		// 2. encode sei type
		out_stream[byte_pos++] = 0x99;	// sei payload type
	}
	else if (eCodecType == HD_CODEC_TYPE_H264) {
		// 1. encode start code & SEI header
		out_stream[byte_pos++] = 0x00;
		out_stream[byte_pos++] = 0x00;
		out_stream[byte_pos++] = 0x00;
		out_stream[byte_pos++] = 0x01;
		out_stream[byte_pos++] = 0x06;	// type SEI
		// 2. encode sei type
		out_stream[byte_pos++] = 0x99;	// sei payload type
	}
	// 3. encode sei length
	while (tmp > 255) {
		out_stream[byte_pos++] = 0xFF;
		tmp -= 255;
	}
	out_stream[byte_pos++] = tmp;
	// 4. encode sei data
	for (i = 0; i < length; i++) {
		//if (0 == out_stream[byte_pos-2] && 0 == out_stream[byte_pos-2]
		if (2 == counter && !(data[i] & 0xFC)) {
			out_stream[byte_pos++] = 0x03;
			counter = 0;
		}
		out_stream[byte_pos++] = data[i];
		if (0x00 == data[i])
			counter++;
		else
			counter = 0;
	}
	out_stream[byte_pos++] = 0x80;
	return byte_pos;
}
#endif

static void *encode_thread(void *arg)
{
	VIDEO_RECORD* p_stream0 = (VIDEO_RECORD *)arg;
	HD_RESULT ret = HD_OK;
	HD_VIDEOENC_BS  data_pull;
	UINT32 j;

	UINTPTR vir_addr_main;
	HD_VIDEOENC_BUFINFO phy_buf_main;
	char file_path_main[32] = "/mnt/sd/dump_bs_main.dat";
	FILE *f_out_main;
	#define PHY2VIRT_MAIN(pa) (vir_addr_main + (pa - phy_buf_main.buf_info.phy_addr))

	//------ wait flow_start ------
	while (p_stream0->flow_start == 0) sleep(1);

	// query physical address of bs buffer ( this can ONLY query after hd_videoenc_start() is called !! )
	hd_videoenc_get(p_stream0->enc_path, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf_main);

	// mmap for bs buffer (just mmap one time only, calculate offset to virtual address later)
	vir_addr_main = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_main.buf_info.phy_addr, phy_buf_main.buf_info.buf_size);

	//----- open output files -----
	if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
		HD_VIDEOENC_ERR("open file (%s) fail....\r\n", file_path_main);
	} else {
		printf("\r\ndump main bitstream to file (%s) ....\r\n", file_path_main);
	}

	printf("\r\nif you want to stop, enter \"q\" to exit !!\r\n\r\n");

	//--------- pull data test ---------
	while (p_stream0->enc_exit == 0) {
		//pull data
		ret = hd_videoenc_pull_out_buf(p_stream0->enc_path, &data_pull, -1); // -1 = blocking mode

		if (ret == HD_OK) {
			for (j=0; j< data_pull.pack_num; j++) {
				UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(data_pull.video_pack[j].phy_addr);
				UINT32 len = data_pull.video_pack[j].size;
				#if WRAP_SEI
				if (g_frameCnt % 2 == 1) //wrap private frame to SEI
				{
					UINT8 *wrap_sei_ptr = malloc(len*2);
					if (wrap_sei_ptr != NULL) {
						len = wrap_sei(wrap_sei_ptr, ptr, data_pull.vcodec_format, len);
						if (f_out_main) fwrite(wrap_sei_ptr, 1, len, f_out_main);
						free(wrap_sei_ptr);
					}
					break;
				}
				else
				#endif
				{
					if (f_out_main) fwrite(ptr, 1, len, f_out_main);
					if (f_out_main) fflush(f_out_main);
				}
			}
			++g_frameCnt;

			// release data
			ret = hd_videoenc_release_out_buf(p_stream0->enc_path, &data_pull);
			if (ret != HD_OK) {
				printf("enc_release error=%d !!\r\n", ret);
			}
		}
	}

	// mummap for bs buffer
	if (vir_addr_main) hd_common_mem_munmap((void *)vir_addr_main, phy_buf_main.buf_info.buf_size);

	// close output file
	if (f_out_main) fclose(f_out_main);

	return 0;
}

#if SHOW_BBOX
static void *bbox_thread(void *arg)
{
	VIDEO_RECORD* p_stream0 = (VIDEO_RECORD *)arg;
	HD_RESULT ret = HD_OK;

	//------ wait flow_start ------
	while (p_stream0->flow_bbox_start == 0) sleep(1);


	printf("\r\nif you want to stop, enter \"q\" to exit !!\r\n\r\n");

	//--------- pull data test ---------
	while (p_stream0->bbox_exit == 0) {
		VENDOR_VIDEOENC_SMART_BBOX smart_bbox = {0};

		// AI detect resolution //
		smart_bbox.base_resolution.w = 960;
		smart_bbox.base_resolution.h = 544;

		// detect bbox number //
		smart_bbox.bbox_num = 3;

		// bbox info //
		smart_bbox.bbox[2].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_TOP_LEFT].x = 0;
		smart_bbox.bbox[2].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_TOP_LEFT].y = 0;
		smart_bbox.bbox[2].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_BOTTOM_RIGHT].x = 100;
		smart_bbox.bbox[2].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_BOTTOM_RIGHT].y = 100;
		smart_bbox.bbox[2].class_id = 0;

		smart_bbox.bbox[0].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_TOP_LEFT].x = 50;
		smart_bbox.bbox[0].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_TOP_LEFT].y = 50;
		smart_bbox.bbox[0].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_BOTTOM_RIGHT].x = 150;
		smart_bbox.bbox[0].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_BOTTOM_RIGHT].y = 150;
		smart_bbox.bbox[0].class_id = 1;

		smart_bbox.bbox[1].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_TOP_LEFT].x = 100;
		smart_bbox.bbox[1].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_TOP_LEFT].y = 100;
		smart_bbox.bbox[1].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_BOTTOM_RIGHT].x = 200;
		smart_bbox.bbox[1].positions[VENDOR_VIDEOENC_SMART_BBOX_POS_BOTTOM_RIGHT].y = 200;
		smart_bbox.bbox[1].class_id = 2;

		smart_bbox.timestamp = 100000;
		ret = vendor_videoenc_set(p_stream0->enc_path, VENDOR_VIDEOENC_PARAM_SMART_BBOX, &smart_bbox);
		if (ret != HD_OK) {
			printf("set smart bbox error=%d !!\r\n", ret);
		}
		usleep(100000);
	}

	return 0;
}
#endif

MAIN(argc, argv)
{
	HD_RESULT ret;
	INT key;
	VIDEO_RECORD stream[1] = {0}; //0: main stream
	UINT32 enc_type = 0;
	HD_DIM main_dim;

	// set MD Mode
	char cmd[50];
	sprintf(cmd, "echo %d MDMode > /proc/kdrv_vdocdc/param", MD_MODE);
	system(cmd);

	// query program options
	if (argc == 2) {
		enc_type = atoi(argv[1]);
		printf("enc_type %d\r\n", enc_type);
		if(enc_type > 2) {
			printf("error: not support enc_type!\r\n");
			return 0;
		}
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

	// open video_record modules (main)
	stream[0].proc_max_dim.w = VDO_SIZE_W; //assign by user
	stream[0].proc_max_dim.h = VDO_SIZE_H; //assign by user
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
	main_dim.w = VDO_SIZE_W;
	main_dim.h = VDO_SIZE_H;

	// set videoproc parameter (main)
#if (FUNC_RELAY == ENABLE)
	ret = set_proc_param(stream[0].proc_path, &main_dim, HD_VIDEO_PXLFMT_YUV420, TRUE); //no bind, must set out dim, must enable pull
#else
	ret = set_proc_param(stream[0].proc_path, NULL, 0, FALSE);
#endif
	if (ret != HD_OK) {
		printf("set proc fail=%d\n", ret);
		goto exit;
	}

	// set videoenc config (main)
	stream[0].enc_max_dim.w = main_dim.w;
	stream[0].enc_max_dim.h = main_dim.h;
	ret = set_enc_cfg(stream[0].enc_path, &stream[0].enc_max_dim, 2 * 1024 * 1024);
	if (ret != HD_OK) {
		printf("set enc-cfg fail=%d\n", ret);
		goto exit;
	}

	// set videoenc parameter (main)
	stream[0].enc_dim.w = main_dim.w;
	stream[0].enc_dim.h = main_dim.h;
	ret = set_enc_param(stream[0].enc_path, &stream[0].enc_dim, enc_type, 2 * 1024 * 1024);
	if (ret != HD_OK) {
		printf("set enc fail=%d\n", ret);
		goto exit;
	}

	// bind video_record modules (main)
	hd_videocap_bind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);
#if (FUNC_RELAY != ENABLE)
	hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOENC_0_IN_0);
#endif

#if SHOW_BBOX
		// create bbox_thread (pull_out bitstream)
		ret = pthread_create(&stream[0].bbox_thread_id, NULL, bbox_thread, (void *)stream);
		if (ret < 0) {
			printf("create bbox thread failed");
			goto exit;
		}
#endif

#if (FUNC_RELAY == ENABLE)
	// create relay_thread (pull_out yuv and push_in yuv)
	ret = pthread_create(&stream[0].relay_thread_id, NULL, relay_yuv_thread, (void *)stream);
	if (ret < 0) {
		printf("create relay thread failed");
		goto exit;
	}
#endif

	// create encode_thread (pull_out bitstream)
	ret = pthread_create(&stream[0].enc_thread_id, NULL, encode_thread, (void *)stream);
	if (ret < 0) {
		printf("create encode thread failed");
		goto exit;
	}

	// start video_record modules (main)
	hd_videocap_start(stream[0].cap_path);
	hd_videoproc_start(stream[0].proc_path);
	// just wait ae/awb stable for auto-test, if don't care, user can remove it
	sleep(1);
	hd_videoenc_start(stream[0].enc_path);

	// let encode_thread start to work
	stream[0].flow_start = 1;

	// query user key
	printf("Enter q to exit\n");
	while (1) {
		key = GETCHAR();
		if (key == 'q' || key == 0x3) {
			// let encode_thread stop loop and exit
			stream[0].enc_exit = 1;
			// quit program
			break;
		}
		if (key == 'k') {
    		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

    		video_path_param.out_func = HD_VIDEOPROC_OUTFUNC_MD;
    		ret = hd_videoproc_set(stream[0].proc_path, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
        	hd_videoproc_start(stream[0].proc_path);

	    }
		if (key == 'm') {
    		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

    		video_path_param.out_func = 0;
    		ret = hd_videoproc_set(stream[0].proc_path, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
        	hd_videoproc_start(stream[0].proc_path);

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
		#if SHOW_BBOX
		if (key == 'a') {
			stream[0].flow_bbox_start = 1;
		}
		#endif
	}

	// destroy encode thread
	pthread_join(stream[0].enc_thread_id, NULL);
#if (FUNC_RELAY == ENABLE)
	// destroy relay thread
	pthread_join(stream[0].relay_thread_id, NULL);
#endif

	// stop video_record modules (main)
	hd_videocap_stop(stream[0].cap_path);
	hd_videoproc_stop(stream[0].proc_path);
	hd_videoenc_stop(stream[0].enc_path);

	// unbind video_record modules (main)
	hd_videocap_unbind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0));
#if (FUNC_RELAY != ENABLE)
	hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_0);
#endif

exit:
	// close video_record modules (main)
	ret = close_module(&stream[0]);
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
