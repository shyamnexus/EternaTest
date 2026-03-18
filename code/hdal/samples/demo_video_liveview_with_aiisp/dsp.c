/**
	@brief Source file of liveview with ai net sample code.

	@file video_liveview_with_aiisp.c

	@ingroup ai_net_sample

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2020.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Including Files                                                             */
/*-----------------------------------------------------------------------------*/
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <kwrap/file.h>
#include <pd_shm.h>
#include "hdal.h"
#include "hd_debug.h"
#include "vendor_isp.h"
#include "vendor_common.h"
#include "vendor_ai.h"
#include "vendor_ai_cpu/vendor_ai_cpu.h"
#include "vendor_videoprocess.h"
#include "vendor_videoenc.h"
#include "vendor_isp.h"
#include "vendor_videocapture.h"
#include "isp.h"
#include "aiisp.h"
// platform dependent
#if defined(__LINUX)
#include <signal.h>
#include <pthread.h>			//for pthread API
#define MAIN(argc, argv) 		int main(int argc, char** argv)
#define GETCHAR()				getchar()
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/signal.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <kwrap/util.h>		//for sleep API
#define sleep(x)    			vos_util_delay_ms(1000*(x))
#define msleep(x)    			vos_util_delay_ms(x)
#define usleep(x)   			vos_util_delay_us(x)
#include <kwrap/examsys.h> 	//for MAIN(), GETCHAR() API
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(ai_video_liveview_with_net, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

///////////////////////////////////////////////////////////////////////////////
#define VCAP_PATTERN_GEN                0 // set 1 for no sensor testing
#define FHD_HIGH_FRAME_RATE             0
#define IPP_NNISP_FRAME_MODE            0
#define ISP_CALLBACK                    1
#define FRAME_RATE                     30

///////////////////////////////////////////////////////////////////////////////
#define VPRC_PATH5_ENABLE               1
#define ISP_SMART_PARAM_ENABLE			1

///////////////////////////////////////////////////////////////////////////////
// ID related define.
#define SEN1_VCAP_ID            0
#define SEN2_VCAP_ID            3
#define SEN3_VCAP_ID            6
#define SEN4_VCAP_ID            9
#define ISP_ID_0                0   //for sensor1
#define ISP_ID_1                3   //for sensor2
#define ISP_ID_2                6   //for sensor3
#define ISP_ID_3                9   //for sensor4

///////////////////////////////////////////////////////////////////////////////
typedef struct _DDR_INFO {
	void *va;
	UINTPTR pa;
	UINT32 size;
} DDR_INFO;

///////////////////////////////////////////////////////////////////////////////
//header
#define DBGINFO_BUFSIZE() (0x200)
//RAW
#define VDO_RAW_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
//CA for AWB
#define VDO_CA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 3) << 1)
//LA for AE
#define VDO_LA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 1) << 1)
//YUV
#define VDO_YUV_BUFSIZE(w, h, pxlfmt)	(ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))

//AIISP
#define VDO_AIDED_MAP_BUFSIZE(w, h, pxlfmt) (ALIGN_CEIL_4((w) / 2) * ALIGN_CEIL_4((h) / 2))

#define VDO_AIDED_BUFSIZE(w, h, pxlfmt) (ALIGN_CEIL_4((w)/8) *ALIGN_CEIL_4((h) / 8) + ALIGN_CEIL_4((w)/2)*ALIGN_CEIL_4((h) / 4) + ALIGN_CEIL_4((w)/4)*ALIGN_CEIL_4((h) / 4))

///////////////////////////////////////////////////////////////////////////////
#define SEN_OUT_FMT      HD_VIDEO_PXLFMT_NRX12 //HD_VIDEO_PXLFMT_RAW12
#define CAP_OUT_FMT      HD_VIDEO_PXLFMT_RAW12 //HD_VIDEO_PXLFMT_NRX12
#define SHDR_CAP_OUT_FMT HD_VIDEO_PXLFMT_RAW12_SHDR2 //HD_VIDEO_PXLFMT_NRX12_SHDR2
#define BNR_OUT_FMT      HD_VIDEO_PXLFMT_RAW12
#define BNR_REF_FMT      HD_VIDEO_PXLFMT_NRX12
#define VPRC_OUT_FMT     HD_VIDEO_PXLFMT_YUV420_NVX2 //HD_VIDEO_PXLFMT_YUV420

#define CA_WIN_NUM_W        32
#define CA_WIN_NUM_H        32
#define LA_WIN_NUM_W        32
#define LA_WIN_NUM_H        32
#define VA_WIN_NUM_W        16
#define VA_WIN_NUM_H        16

typedef enum _SEN_SEL {
	SEN_SEL_IMX290       =  0,   //2M_SHDR
	SEN_SEL_OS05A10      =  1,   //5M_SHDR
	SEN_SEL_OS02K10      =  2,   //2M
	SEN_SEL_AR0237IR     =  3,
	SEN_SEL_PATGEN_1080P =  4,
	SEN_SEL_PATGEN_VGA   =  5,
	SEN_SEL_IMX485       =  6,   //8M
	SEN_SEL_OS04A10      =  7,   //4M
	SEN_SEL_IMX415       =  8,   //8M
	SEN_SEL_IMX585       =  9,   //8M
	SEN_SEL_IMX334       = 10,   //8M
	SEN_SEL_IMX678       = 11,   //8M
	SEN_SEL_SC450AI      = 12,   //4M
	SEN_SEL_SC850SL      = 13,   //8M
	SEN_SEL_OS08C10      = 14,   //8M
	SEN_SEL_OS04C10      = 15,   //4M
	SEN_SEL_PATGEN_3M    = 16,   //3M
	SEN_SEL_PATGEN_4M    = 17,   //4M
	SEN_SEL_PATGEN_5M_2  = 18,   //5M
	SEN_SEL_PATGEN_8M    = 19,   //8M
	SEN_SEL_OS12D40      = 20,   //12M

	// NOTE: Do not modify SEN_SEL_MAX and SEN_SEL_PATGEN
	SEN_SEL_MAX,
	SEN_SEL_PATGEN                 = 99,
	ENUM_DUMMY4WORD(SEN_SEL)
} SEN_SEL;

#define VDO_SIZE_W_12M     4512
#define VDO_SIZE_H_12M     2512
#define VDO_SIZE_W_8M      3840
#define VDO_SIZE_H_8M      2160
#define VDO_SIZE_W_5M      2592
#define VDO_SIZE_H_5M      1944
#define VDO_SIZE_W_5M_2    2688
#define VDO_SIZE_H_5M_2    2048
#define VDO_SIZE_W_4M      2688
#define VDO_SIZE_H_4M      1520
#define VDO_SIZE_W_4M_2    2560
#define VDO_SIZE_H_4M_2    1440
#define VDO_SIZE_W_3M      2048
#define VDO_SIZE_H_3M      1536
#define VDO_SIZE_W_2M      1920
#define VDO_SIZE_H_2M      1080
#define VDO_SIZE_W_VGA     640
#define VDO_SIZE_H_VGA     480

#define LOW_ISO_EFFECT      0
#define NORMAL_ISO_EFFECT   1
#define HIGH_ISO_EFFECT     2

///////////////////////////////////////////////////////////////////////////////
//sensor list
#define ISP_DEFAULT_CFG "isp_os02k10_0"

SENSOR_INFO sensor_info[] = {
	//        number                 size                               name                       cfg(/mnt/app/isp/xxx.cfg)
	/*  0 */{SEN_SEL_IMX290,        {VDO_SIZE_W_2M,  VDO_SIZE_H_2M},    "nvt_sen_imx290",          "isp_imx290_0"         },
	/*  1 */{SEN_SEL_OS05A10,       {VDO_SIZE_W_5M,  VDO_SIZE_H_5M},    "nvt_sen_os05a10",         ISP_DEFAULT_CFG        },
	/*  2 */{SEN_SEL_OS02K10,       {VDO_SIZE_W_2M,  VDO_SIZE_H_2M},    "nvt_sen_os02k10",         "isp_os02k10_0"        },
	/*  3 */{SEN_SEL_AR0237IR,      {VDO_SIZE_W_2M,  VDO_SIZE_W_2M},    "nvt_sen_ar0237ir",        ISP_DEFAULT_CFG        },
	/*  4 */{SEN_SEL_PATGEN,        {VDO_SIZE_W_2M,  VDO_SIZE_H_2M},    "PATTERN_GEN",             ISP_DEFAULT_CFG        },
	/*  5 */{SEN_SEL_PATGEN,        {VDO_SIZE_W_VGA, VDO_SIZE_H_VGA},   "PATTERN_GEN",             ISP_DEFAULT_CFG        },
	/*  6 */{SEN_SEL_IMX485,        {VDO_SIZE_W_8M,  VDO_SIZE_H_8M},    "nvt_sen_imx485",          "isp_imx485_0"         },
	/*  7 */{SEN_SEL_OS04A10,       {VDO_SIZE_W_4M,  VDO_SIZE_H_4M},    "nvt_sen_os04a10",         "isp_os04a10_0"        },
	/*  8 */{SEN_SEL_IMX415,        {VDO_SIZE_W_8M,  VDO_SIZE_H_8M},    "nvt_sen_imx415",          "isp_imx415_0"         },
	/*  9 */{SEN_SEL_IMX585,        {VDO_SIZE_W_8M,  VDO_SIZE_H_8M},    "nvt_sen_imx585",          "isp_imx585_0"         },
	/* 10 */{SEN_SEL_IMX334,        {VDO_SIZE_W_8M,  VDO_SIZE_H_8M},    "nvt_sen_imx334",          "isp_imx334_0"         },
	/* 11 */{SEN_SEL_IMX678,        {VDO_SIZE_W_8M,  VDO_SIZE_H_8M},    "nvt_sen_imx678",          "isp_imx678_0"         },
	/* 12 */{SEN_SEL_SC450AI,       {VDO_SIZE_W_4M,  VDO_SIZE_H_4M},    "nvt_sen_sc450ai",         "isp_sc450ai_0"         },
	/* 13 */{SEN_SEL_SC850SL,       {VDO_SIZE_W_8M,  VDO_SIZE_H_8M},    "nvt_sen_sc850sl",         "isp_sc850sl_0"         },
	/* 14 */{SEN_SEL_OS08C10,       {VDO_SIZE_W_8M,  VDO_SIZE_H_8M},    "nvt_sen_os08c10",         "isp_sc450ai_0"         },
	/* 15 */{SEN_SEL_OS04C10,       {VDO_SIZE_W_4M_2,  VDO_SIZE_H_4M_2},"nvt_sen_os04c10",         "isp_os04c10_0"         },
	/* 16 */{SEN_SEL_PATGEN,        {VDO_SIZE_W_3M,  VDO_SIZE_H_3M},    "PATTERN_GEN",             ISP_DEFAULT_CFG        },
	/* 17 */{SEN_SEL_PATGEN,        {VDO_SIZE_W_4M, VDO_SIZE_H_4M},     "PATTERN_GEN",             ISP_DEFAULT_CFG        },
	/* 18 */{SEN_SEL_PATGEN,        {VDO_SIZE_W_5M_2, VDO_SIZE_H_5M_2}, "PATTERN_GEN",             ISP_DEFAULT_CFG        },
	/* 19 */{SEN_SEL_PATGEN,        {VDO_SIZE_W_8M, VDO_SIZE_H_8M},     "PATTERN_GEN",             ISP_DEFAULT_CFG        },
	/* 20 */{SEN_SEL_PATGEN,        {VDO_SIZE_W_12M, VDO_SIZE_H_12M},   "nvt_sen_os12d40",         "isp_os12d40_0"        },

};

#define VIDEOCAP_ALG_FUNC HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB
#define VIDEOPROC_ALG_FUNC HD_VIDEOPROC_FUNC_AF | HD_VIDEOPROC_FUNC_WDR | HD_VIDEOPROC_FUNC_DEFOG | HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA

static UINT32 g_shdr = 0; //fixed

static UINT32 g_fps = FRAME_RATE;

static UINT32 g_bitrate = 8;

#define AI_PAR_MAX		32

#define SOURCE_PATH     HD_VIDEOPROC_3_OUT_0 //out 0~4 is physical path
#define EXTEND_PATH     HD_VIDEOPROC_3_OUT_5 //out 5~15 is extend path
#define EXTEND_PATH2    HD_VIDEOPROC_3_OUT_6 //out 5~15 is extend path
#define SOURCE_PATH2    HD_VIDEOPROC_1_OUT_0 //out 0~4 is physical path

#define DBG_OUT_DUMP                0  // debug mode, dump output iobuf

#define LABEL_LEN           256     ///< maximal length of class label
#define MAX_CLASS_NUMBER    1000
#define TOP_N                5

///////////////////////////////////////////////////////////////////////////////
#define AI_DDRID       DDR_ID0
#define VIDEO_DDRID    DDR_ID0

#define NET_PATH_ID UINT32
#define VENDOR_AI_CFG 0x000f0000  //vendor ai config
#define AI_RGB_BUFSIZE(w, h) (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(HD_VIDEO_PXLFMT_RGB888_PLANAR) / 8) * (h))
#define POOL_SIZE_USER_DEFINIED  0x1000000

static UINT32 sensor_sel = SEN_SEL_OS04A10;
static UINT32 sensor_en_map = 1; //0x1 for sensor 1, 0x2 for sensor 2, 0x4 for sensor 3, 0x8 sensor 4
static UINT32 sensor_cnt = 0;

char *chip_name;

static UINT32 ai_isp_flow = 0; //0:1_5_10_1_2, 1:1_5_16_1_1, 2:1_5_21_1_0
static UINT32 g_capbind = 1;  //0:D2D, 1:direct
static UINT32 fix_pattern = 0;
static UINT32 vprc_output = VPRC_OUTPUT_VENC_1OUT;

MODEL_PARAM_NAME model_param_name;
/*-----------------------------------------------------------------------------*/
/* Type Definitions                                                            */
/*-----------------------------------------------------------------------------*/

//==============================
// nr_param[0] = 0
// nr_param[1] = 0
// nr_param[2] = 0
// nr_param[3] = 0
// nr_param[4] = 128
// nr_param[5] = 0
// nr_param[6] = 255
//=======================

/*-----------------------------------------------------------------------------*/
/* Global Functions                                                            */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Input Functions                                                             */
/*-----------------------------------------------------------------------------*/

static BOOL sensor1_enabled(void)
{
	if (sensor_en_map & 1)
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
static BOOL sensor3_enabled(void)
{
	if (sensor_en_map & 4)
		return TRUE;
	else
		return FALSE;
}
static BOOL sensor4_enabled(void)
{
	if (sensor_en_map & 8)
		return TRUE;
	else
		return FALSE;
}


INT32 user_mem_config(HD_COMMON_MEM_INIT_CONFIG* p_mem_cfg, INT32 i)
{
	// config common pool (proc)
	p_mem_cfg->pool_info[i].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
	p_mem_cfg->pool_info[i].blk_size = POOL_SIZE_USER_DEFINIED;  // NOTE: dim need align to 16 for rotate buffer
	p_mem_cfg->pool_info[i].blk_cnt = 1;
	p_mem_cfg->pool_info[i].ddr_id = VIDEO_DDRID;
	i++;
	return i;
}

INT32 cap_mem_config(HD_COMMON_MEM_INIT_CONFIG* p_mem_cfg, void* p_cfg, INT32 i, UINT32 vprc_output, UINT32 sensor_cnt)
{
	HD_DIM* p_dim = (HD_DIM*)p_cfg;
	// config common pool (cap)
	p_mem_cfg->pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;

	if (g_capbind == 1) {
		//direct ... NOT require raw
		p_mem_cfg->pool_info[i].blk_size = vendor_common_mem_calc_max_buf_size(0, 0, CAP_OUT_FMT);
	} else {
		p_mem_cfg->pool_info[i].blk_size = vendor_common_mem_calc_max_buf_size(p_dim->w, p_dim->h, CAP_OUT_FMT);

	}

	p_mem_cfg->pool_info[i].blk_cnt = 3 * sensor_cnt;

	if (vprc_output == VPRC_OUTPUT_VENC) {
		p_mem_cfg->pool_info[i].blk_cnt *= 2;
	}

	p_mem_cfg->pool_info[i].ddr_id = VIDEO_DDRID;
	i++;

	return i;
}

static HD_RESULT get_cap_caps(HD_PATH_ID video_cap_ctrl, HD_VIDEOCAP_SYSCAPS *p_video_cap_syscaps)
{
	HD_RESULT ret = HD_OK;
	hd_videocap_get(video_cap_ctrl, HD_VIDEOCAP_PARAM_SYSCAPS, p_video_cap_syscaps);
	return ret;
}

static HD_RESULT set_cap_cfg(HD_PATH_ID *p_video_cap_ctrl, HD_PATH_ID cap_ctrl_id, UINT32 shdr_map)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
	HD_PATH_ID video_cap_ctrl = 0;
	HD_VIDEOCAP_CTRL iq_ctl = {0};

	snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, sensor_info[sensor_sel].sensor_name);
	printf("sensor Using %s \n", cap_cfg.sen_cfg.sen_dev.driver_name);

	if (sensor_sel == SEN_SEL_AR0237IR) {
		cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_P_RAW;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux =  0;  //use @0 in peri-dev.dtsi
		printf("Parallel interface\n");
	} else if (sensor_sel == SEN_SEL_IMX290 || sensor_sel == SEN_SEL_OS05A10 || sensor_sel == SEN_SEL_OS02K10 || sensor_sel == SEN_SEL_IMX485 || sensor_sel == SEN_SEL_OS04A10 || sensor_sel == SEN_SEL_IMX415 || sensor_sel == SEN_SEL_IMX485 || sensor_sel == SEN_SEL_IMX334 || sensor_sel == SEN_SEL_IMX678 || sensor_sel == SEN_SEL_SC450AI || sensor_sel == SEN_SEL_SC850SL || sensor_sel == SEN_SEL_OS08C10 || sensor_sel == SEN_SEL_OS04C10 || sensor_sel == SEN_SEL_OS12D40) {
		cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux =  0;  //use @0 in peri-dev.dtsi
		printf("MIPI interface\n");
	}
	if (g_shdr == 1) {
		printf("Using g_shdr_mode\n");
	}

	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(0, 0);

	for (UINT32 i = 0; i < HD_VIDEOCAP_SEN_SER_MAX_DATALANE; i++) {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[i] = HD_VIDEOCAP_SEN_IGNORE;
	}

	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
	if (g_shdr == 1) {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
	} else {
		if (sensor_sel == SEN_SEL_IMX290) {
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
		} else if (sensor_sel == SEN_SEL_OS05A10) {
			printf("Using OS052A or shdr\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
		} else if (sensor_sel == SEN_SEL_OS02K10) {
			printf("Using OS02K10 or shdr\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
		} else if (sensor_sel == SEN_SEL_OS04A10) {
			printf("Using OS04A10 or shdr\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
		} else if (sensor_sel == SEN_SEL_SC450AI) {
			printf("Using OS04A10 or shdr\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
		}
	}

	if (sensor_sel == SEN_SEL_PATGEN_1080P || sensor_sel == SEN_SEL_PATGEN_VGA) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, HD_VIDEOCAP_SEN_PAT_GEN);
	}

	ret = hd_videocap_open(0, cap_ctrl_id, &video_cap_ctrl); //open this for device control
	if (ret != HD_OK) {
		return ret;
	}

	if (g_shdr == 1) {
		cap_cfg.sen_cfg.shdr_map = shdr_map;
	}

	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);

	if (sensor_sel != SEN_SEL_PATGEN_1080P && sensor_sel != SEN_SEL_PATGEN_VGA) {
		iq_ctl.func = VIDEOCAP_ALG_FUNC;

		if (g_shdr == 1) {
			iq_ctl.func |= HD_VIDEOCAP_FUNC_SHDR;
		}

		ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);
	}

	*p_video_cap_ctrl = video_cap_ctrl;
	return ret;
}

static HD_RESULT set_cap_param(HD_PATH_ID video_cap_path, HD_DIM *p_dim, UINT32 depth)
{
	HD_RESULT ret = HD_OK;
	{//select sensor mode, manually or automatically
		HD_VIDEOCAP_IN video_in_param = {0};

		if (sensor_sel == SEN_SEL_PATGEN_1080P || sensor_sel == SEN_SEL_PATGEN_VGA) {// pattern gen
			video_in_param.sen_mode = HD_VIDEOCAP_PATGEN_MODE(HD_VIDEOCAP_SEN_PAT_COLORBAR, 200);
			#if FHD_HIGH_FRAME_RATE
			video_in_param.frc = HD_VIDEO_FRC_RATIO(120,1);
			#else
			video_in_param.frc = HD_VIDEO_FRC_RATIO(g_fps,1);
			#endif
			video_in_param.dim.w = p_dim->w;
			video_in_param.dim.h = p_dim->h;
		} else {
			video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO; //auto select sensor mode by the parameter of HD_VIDEOCAP_PARAM_OUT
			#if FHD_HIGH_FRAME_RATE
			video_in_param.frc = HD_VIDEO_FRC_RATIO(120,1);
			#else
			video_in_param.frc = HD_VIDEO_FRC_RATIO(g_fps,1);
			#endif
			video_in_param.dim.w = p_dim->w;
			video_in_param.dim.h = p_dim->h;
			video_in_param.pxlfmt = SEN_OUT_FMT;
			// NOTE: only SHDR with path 1
			if (g_shdr == 1) {
				video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_2;
			} else {
				video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
			}
		}

		if (sensor_sel == SEN_SEL_PATGEN_1080P || sensor_sel == SEN_SEL_PATGEN_VGA) {// pattern gen
			video_in_param.sen_mode = HD_VIDEOCAP_PATGEN_MODE(HD_VIDEOCAP_SEN_PAT_COLORBAR, 200);
		}

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
		video_crop_param.win.rect.w = p_dim->w/2;
		video_crop_param.win.rect.h= p_dim->h/2;
		video_crop_param.align.w = 4;
		video_crop_param.align.h = 4;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT_CROP, &video_crop_param);
		//printf("set_cap_param CROP ON=%d\r\n", ret);
	}
	#endif
	{
		HD_VIDEOCAP_OUT video_out_param = {0};

		//without setting dim for no scaling, using original sensor out size
		// NOTE: only SHDR with path 1
		if (g_shdr == 1) {
			video_out_param.pxlfmt = SHDR_CAP_OUT_FMT;
		} else {
			video_out_param.pxlfmt = CAP_OUT_FMT;
		}
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		video_out_param.depth = depth;
		#if (SAVE_RAW_ENABLE)
		video_out_param.depth += (g_shdr + 1);
		#endif
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT, &video_out_param);
		//printf("set_cap_param OUT=%d\r\n", ret);
	}


	if ((sensor_sel == SEN_SEL_SC850SL)) {
		UINT32 data_lane = 2;
		vendor_videocap_set(video_cap_path, VENDOR_VIDEOCAP_PARAM_DATA_LANE, &data_lane);
		printf("data_lane=%d \r\n", data_lane);
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

INT32 proc_mem_config(HD_COMMON_MEM_INIT_CONFIG* p_mem_cfg, void* p_cfg, INT32 i, UINT32 vprc_output, UINT32 sensor_cnt)
{
	HD_DIM* p_dim = (HD_DIM*)p_cfg;


	// config common pool (proc)
	p_mem_cfg->pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	p_mem_cfg->pool_info[i].blk_size = vendor_common_mem_calc_max_buf_size(p_dim->w, p_dim->h, VPRC_OUT_FMT);  // NOTE: dim need align to 16 for rotate buffer
	p_mem_cfg->pool_info[i].blk_cnt = 4 * sensor_cnt;
	if (vprc_output == VPRC_OUTPUT_VENC) {
		p_mem_cfg->pool_info[i].blk_cnt *= 2;
	}
	p_mem_cfg->pool_info[i].ddr_id = VIDEO_DDRID;
	i++;

	if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
		// config common pool (proc)
		p_mem_cfg->pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
		p_mem_cfg->pool_info[i].blk_size = vendor_common_mem_calc_max_buf_size(p_dim->w, p_dim->h, BNR_OUT_FMT);
		p_mem_cfg->pool_info[i].blk_cnt = 4 * sensor_cnt;
		if (vprc_output == VPRC_OUTPUT_VENC) {
			p_mem_cfg->pool_info[i].blk_cnt *= 2;
		}
		p_mem_cfg->pool_info[i].ddr_id = VIDEO_DDRID;
		i++;

		// config common pool (proc)
		p_mem_cfg->pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
		p_mem_cfg->pool_info[i].blk_size = vendor_common_mem_calc_max_buf_size(p_dim->w, p_dim->h, BNR_REF_FMT);
		p_mem_cfg->pool_info[i].blk_cnt = 3 * sensor_cnt;
		if (vprc_output == VPRC_OUTPUT_VENC) {
			p_mem_cfg->pool_info[i].blk_cnt *= 2;
		}
		p_mem_cfg->pool_info[i].ddr_id = VIDEO_DDRID;
		i++;

		p_mem_cfg->pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
		p_mem_cfg->pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_AIDED_MAP_BUFSIZE(p_dim->w, p_dim->h, HD_VIDEO_PXLFMT_RAW12);  // NOTE: dim need align to 16 for rotate buffer
		p_mem_cfg->pool_info[i].blk_cnt = 2 * sensor_cnt;
		if (vprc_output == VPRC_OUTPUT_VENC) {
			p_mem_cfg->pool_info[i].blk_cnt *= 2;
		}
		p_mem_cfg->pool_info[i].ddr_id = VIDEO_DDRID;
		i++;

		p_mem_cfg->pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
		p_mem_cfg->pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_AIDED_BUFSIZE(p_dim->w, p_dim->h, HD_VIDEO_PXLFMT_RAW12);  // NOTE: dim need align to 16 for rotate buffer
		p_mem_cfg->pool_info[i].blk_cnt = 3 * sensor_cnt;
		if (vprc_output == VPRC_OUTPUT_VENC) {
			p_mem_cfg->pool_info[i].blk_cnt *= 2;
		}
		p_mem_cfg->pool_info[i].ddr_id = VIDEO_DDRID;
		i++;
	}
	return i;
}

INT32 out_mem_config(HD_COMMON_MEM_INIT_CONFIG* p_mem_cfg, void* p_cfg, INT32 i)
{
	HD_DIM* p_dim = (HD_DIM*)p_cfg;
	// config common pool (proc)
	p_mem_cfg->pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	p_mem_cfg->pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(p_dim->w, p_dim->h, HD_VIDEO_PXLFMT_YUV420);  // NOTE: dim need align to 16 for rotate buffer
	p_mem_cfg->pool_info[i].blk_cnt = 5;
	p_mem_cfg->pool_info[i].ddr_id = VIDEO_DDRID;
	i++;
	return i;
}

static HD_RESULT set_proc_cfg(HD_PATH_ID *p_video_proc_ctrl, HD_DIM* p_max_dim, HD_CTRL_ID ctrl_id, HD_PATH_ID proc_path_out_id, HD_PATH_ID proc_path_3dnr_out_id, HD_VIDEOPROC_PIPE pipe, HD_VIDEOPROC_CTRLFUNC func, HD_VIDEO_PXLFMT input_pxlfmt, UINT32 isp_id)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
	HD_VIDEOPROC_CTRL video_ctrl_param = {0};
	HD_PATH_ID video_proc_ctrl = 0;

	ret = hd_videoproc_open(0, ctrl_id, &video_proc_ctrl); //open this for device control
	if (ret != HD_OK)
		return ret;

	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		video_path_param.in_func = 0;
		if (g_capbind == 1)
			video_path_param.in_func |= HD_VIDEOPROC_INFUNC_DIRECT; //direct NOTE: enable direct
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
		//printf("set_proc_param PATH_CONFIG=0x%X\r\n", ret);
	}

	if (p_max_dim != NULL ) {
		video_cfg_param.pipe = pipe;
		video_cfg_param.isp_id = isp_id;
		video_cfg_param.ctrl_max.func = func;
		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;
		if (g_shdr == 1) {
			video_cfg_param.in_max.pxlfmt = SHDR_CAP_OUT_FMT;
		} else {
			video_cfg_param.in_max.pxlfmt = CAP_OUT_FMT;
		}
		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
		if (ret != HD_OK) {
			return HD_ERR_NG;
		}
	}

	video_ctrl_param.func = func;
	if (func & HD_VIDEOPROC_FUNC_3DNR) {
		#if (VPRC_PATH5_ENABLE)
		video_ctrl_param.ref_path_3dnr = proc_path_3dnr_out_id;
		#else
		video_ctrl_param.ref_path_3dnr = proc_path_out_id;
		#endif
	}
	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);
	{
		UINT32 in_depth = 5;
		ret =vendor_videoproc_set(video_proc_ctrl, VENDOR_VIDEOPROC_PARAM_IN_DEPTH, &in_depth);
	}

	*p_video_proc_ctrl = video_proc_ctrl;

	return ret;
}

static HD_RESULT set_proc_cfg2(HD_PATH_ID *p_video_proc_ctrl, HD_PATH_ID proc_ctrl_id, HD_PATH_ID proc_path_out_id, HD_PATH_ID proc_path_3dnr_out_id, HD_DIM* p_max_dim, UINT32 isp_id)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
	HD_VIDEOPROC_CTRL video_ctrl_param = {0};
	HD_PATH_ID video_proc_ctrl = 0;

	ret = hd_videoproc_open(0, proc_ctrl_id, &video_proc_ctrl); //open this for device control
	if (ret != HD_OK)
		return ret;

	if (p_max_dim != NULL ) {
		video_cfg_param.pipe = HD_VIDEOPROC_PIPE_BNR_RAWALL;
		video_cfg_param.isp_id = isp_id;
		video_cfg_param.ctrl_max.func = VIDEOPROC_ALG_FUNC;
		if (g_shdr == 1) {
			video_cfg_param.ctrl_max.func |= HD_VIDEOPROC_FUNC_SHDR;
		}
		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;
		if (g_shdr == 1) {
			video_cfg_param.in_max.pxlfmt = SHDR_CAP_OUT_FMT;
		} else {
			video_cfg_param.in_max.pxlfmt = CAP_OUT_FMT;
		}
		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
		if (ret != HD_OK) {
			return HD_ERR_NG;
		}
	}

	video_ctrl_param.func = VIDEOPROC_ALG_FUNC;
	if (g_shdr == 1) {
		video_ctrl_param.func |= HD_VIDEOPROC_FUNC_SHDR;
	}
	#if (VPRC_PATH5_ENABLE)
	video_ctrl_param.ref_path_3dnr = proc_path_3dnr_out_id;
	#else
	video_ctrl_param.ref_path_3dnr = proc_path_out_id;
	#endif
	{
		UINT32 in_depth = 5;
		ret =vendor_videoproc_set(video_proc_ctrl, VENDOR_VIDEOPROC_PARAM_IN_DEPTH, &in_depth);
	}
	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);

	*p_video_proc_ctrl = video_proc_ctrl;

	return ret;
}

static HD_RESULT set_proc_param(HD_PATH_ID video_proc_path, HD_DIM* p_dim, UINT32 dir, UINT32 depth, HD_VIDEO_PXLFMT output_pxlfmt)
{
	HD_RESULT ret = HD_OK;

	if (p_dim != NULL) { //if videoproc is already binding to dest module, not require to setting this!
		HD_VIDEOPROC_OUT video_out_param = {0};
		video_out_param.func = 0;
		video_out_param.dim.w = p_dim->w;
		video_out_param.dim.h = p_dim->h;
		video_out_param.pxlfmt = output_pxlfmt;
		video_out_param.dir = dir;
		video_out_param.frc = HD_VIDEO_FRC_RATIO(1,1);
		video_out_param.depth = depth; //set 1 to allow pull
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	} else {
		HD_VIDEOPROC_OUT video_out_param = {0};
		video_out_param.func = 0;
		video_out_param.dim.w = 0;
		video_out_param.dim.h = 0;
		video_out_param.pxlfmt = 0;
		video_out_param.dir = dir;
		video_out_param.frc = HD_VIDEO_FRC_RATIO(1,1);
		video_out_param.depth = depth; //set 1 to allow pull
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	}

	return ret;
}

static HD_RESULT set_enc_cfg(HD_PATH_ID video_enc_path, HD_DIM *p_max_dim, UINT32 max_bitrate, UINT32 isp_id)
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
		video_path_config.isp_id             = isp_id;
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_PATH_CONFIG, &video_path_config);
		if (ret != HD_OK) {
			printf("set_enc_path_config = %d\r\n", ret);
			return HD_ERR_NG;
		}
	}

	return ret;
}

static HD_RESULT set_enc_param(HD_PATH_ID video_enc_path, HD_DIM *p_dim, HD_VIDEO_PXLFMT pxlfmt, UINT32 enc_type, UINT32 bitrate)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOENC_IN  video_in_param = {0};
	HD_VIDEOENC_OUT video_out_param = {0};
	HD_H26XENC_RATE_CONTROL rc_param = {0};
	HD_H26XENC_ROW_RC rowrc_param = {0};
	HD_H26XENC_VUI vui = {0};

	if (p_dim != NULL) {

		//--- HD_VIDEOENC_PARAM_IN ---
		video_in_param.dir           = HD_VIDEO_DIR_NONE;
		video_in_param.pxl_fmt = pxlfmt ? pxlfmt : HD_VIDEO_PXLFMT_YUV420;
		video_in_param.dim.w   = p_dim->w;
		video_in_param.dim.h   = p_dim->h;
		video_in_param.frc     = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_IN, &video_in_param);
		if (ret != HD_OK) {
			printf("set_enc_param_in = %d\r\n", ret);
			return ret;
		}

		//--- HD_VIDEOENC_PARAM_OUT_RATE_CONTROL ---
		#if 1
		rc_param.rc_mode                     = HD_RC_MODE_CBR;
		rc_param.cbr.bitrate                 = bitrate;
		rc_param.cbr.frame_rate_base         = g_fps;
		rc_param.cbr.frame_rate_incr         = 1;
		rc_param.cbr.init_i_qp               = 26;
		rc_param.cbr.min_i_qp                = 10;
		rc_param.cbr.max_i_qp                = 51;
		rc_param.cbr.init_p_qp               = 26;
		rc_param.cbr.min_p_qp                = 10;
		rc_param.cbr.max_p_qp                = 51;
		rc_param.cbr.static_time             = 4;
		#else
		rc_param.rc_mode                     = HD_RC_MODE_FIX_QP;
		rc_param.fixqp.frame_rate_base       = g_fps;
		rc_param.fixqp.frame_rate_incr       = 1;
		rc_param.fixqp.fix_i_qp              = 28;
		rc_param.fixqp.fix_p_qp              = 30;
		#endif
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL, &rc_param);
		if (ret != HD_OK) {
			printf("set_enc_rate_control = %d \n", ret);
			return ret;
		}

		rowrc_param.enable = TRUE;
		rowrc_param.i_qp_range = 1;
		rowrc_param.i_qp_step = 1;
		rowrc_param.p_qp_range = 1;
		rowrc_param.p_qp_step = 1;
		rowrc_param.min_i_qp = 10;
		rowrc_param.max_i_qp = 51;
		rowrc_param.min_p_qp = 10;
		rowrc_param.max_p_qp = 51;
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ROW_RC, &rowrc_param);
		if (ret != HD_OK) {
			printf("set_row_rc = %d \n", ret);
			return ret;
		}

		//--- HD_VIDEOENC_PARAM_OUT_VUI ---
		vui.vui_en = TRUE;
		vui.color_range = 1; // 0: Not full range, 1: Full range
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_VUI , &vui);
		if (ret != HD_OK) {
			printf("set_enc_out_ui = %d \n", ret);
			return ret;
		}

		//--- HD_VIDEOENC_PARAM_OUT_AQ ---
		HD_H26XENC_AQ aq_param = {
			.enable = TRUE,
			.i_str = 4,
			.p_str = 4,
			.min_delta_qp = -4,
			.max_delta_qp = 4,
			.depth = 2,
			.thd_table = {-120,-112,-104, -96, -88, -80, -72, -64, -56, -48, -40, -32, -24, -16, -8, 7, 15, 23, 31, 39,47, 55, 63, 71, 79, 87, 95, 103, 111, 119}
		};
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_AQ, &aq_param);
		if (ret != HD_OK) {
			printf("set_enc_aq = %d \n", ret);
			return ret;
		}

		//--- VENDOR_VIDEOENC_PARAM_OUT_H26X_MAQ_DIFF ---
		VENDOR_VIDEOENC_H26X_MAQ_DIFF maq_diff;
		maq_diff.b_enable = 1;
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
		bg_rdo.b_enable = TRUE;
		bg_rdo.avc_bg_skip_bias = 0;
		bg_rdo.hevc_bg_skip_bias = -16;
		bg_rdo.hevc_bg_merge_bias = 0;
		bg_rdo.bg_bias_shift = 0;
		bg_rdo.mode = 0;
		printf("set HD_VIDEOENC_PARAM_OUT BG ! \n");
		ret = vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_BG_RDO, &bg_rdo);
		if (ret != HD_OK) {
			printf("set_enc_bg_rdo = %d \n", ret);
			return ret;
		}

		printf("enc_type=%d\r\n", enc_type);

		if (enc_type == 0) {
			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_H265;
			video_out_param.h26x.profile       = HD_H265E_MAIN_PROFILE;
			video_out_param.h26x.level_idc     = HD_H265E_LEVEL_5;
			video_out_param.h26x.gop_num       = (g_fps > 20) ? FRAME_RATE * 2 : FRAME_RATE * 3;
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
		} else if (enc_type == 1) {
			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_H264;
			video_out_param.h26x.profile       = HD_H264E_HIGH_PROFILE;
			video_out_param.h26x.level_idc     = HD_H264E_LEVEL_5_1;
			video_out_param.h26x.gop_num       = (g_fps > 20) ? FRAME_RATE * 2 : FRAME_RATE * 3;
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

static HD_RESULT set_out_cfg(HD_PATH_ID *p_video_out_ctrl, UINT32 out_type,HD_VIDEOOUT_HDMI_ID hdmi_id)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOOUT_MODE3 videoout_mode = {0};
	HD_PATH_ID video_out_ctrl = 0;

	ret = hd_videoout_open(0, HD_VIDEOOUT_0_CTRL, &video_out_ctrl); //open this for device control
	if (ret != HD_OK) {
		return ret;
	}

	printf("out_type=%d\r\n", out_type);
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
		videoout_mode.output_mode.hdmi= HD_VIDEOOUT_HDMI_1920X1080P60;
		videoout_mode.hdmi_id = 1;
	break;
	default:
		printf("not support out_type\r\n");
	break;
	}
	ret = hd_videoout_set(video_out_ctrl, HD_VIDEOOUT_PARAM_MODE3, &videoout_mode);

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

/*-----------------------------------------------------------------------------*/
/* Network Functions                                                             */
/*-----------------------------------------------------------------------------*/

typedef enum _VIDEO_LIVE_CNT {
	VIDEO_LIVE_0,
	VIDEO_LIVE_0_SUB,
	VIDEO_LIVE_1,
	VIDEO_LIVE_1_SUB,
	VIDEO_LIVE_2,
	VIDEO_LIVE_2_SUB,
	VIDEO_LIVE_3,
	VIDEO_LIVE_3_SUB,
	VIDEO_LIVE_CNT_MAX,
	ENUM_DUMMY4WORD(VIDEO_LIVE_CNT)
} VIDEO_LIVE_CNT;

static VIDEO_LIVEVIEW stream[VIDEO_LIVE_CNT_MAX] = {0}; //0: main stream //1: sub stream (vout) //2: second AI

#if (SAVE_RAW_ENABLE)
#define MAX_RAW_QUEUE 500
#define MAX_DDR_NUM 2
#define MAX_FRAME_NUM 3

typedef struct _RAW_INFO {
	UINT64 vd_cnt[MAX_RAW_QUEUE];
	void *va[MAX_RAW_QUEUE];
	UINTPTR pa[MAX_RAW_QUEUE];
} RAW_INFO;

RAW_INFO raw_info[2][MAX_FRAME_NUM] = {0};
UINT32 cap_idx[2][MAX_FRAME_NUM];
UINT32 cap_max_num = 0;

DDR_INFO ddr_max_free[MAX_DDR_NUM] = {0};
DDR_INFO ddr_remain[MAX_DDR_NUM] = {0};

static HD_RESULT mem_alloc_remain(UINTPTR * phy_addr, void * * virt_addr, UINT32 size, HD_COMMON_MEM_DDR_ID ddr)
{
	if (ALIGN_CEIL_4(size) > ddr_remain[ddr].size) {
		return HD_ERR_NOMEM;
	} else {
		*phy_addr = ddr_remain[ddr].pa;
		*virt_addr = ddr_remain[ddr].va;
		ddr_remain[ddr].size -= ALIGN_CEIL_4(size);
		ddr_remain[ddr].pa += ALIGN_CEIL_4(size);
		ddr_remain[ddr].va = (void *)((UINTPTR)ddr_remain[ddr].va + ALIGN_CEIL_4(size));
		return HD_OK;
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
static char   *g_shm = NULL;
static int    g_shmid = 0;

void init_share_memory(UINT32 cap_size_w, UINT32 cap_size_h)
{
	int g_shmid = 0;
	key_t key;
	PD_SHM_INFO  *p_pd_shm;

	// Segment key.
	key = PD_SHM_KEY;
	// Create the segment.
	if( ( g_shmid = shmget( key, PD_SHMSZ, IPC_CREAT | 0666 ) ) < 0 ) {
		perror( "shmget" );
		exit(1);
	}
	// Attach the segment to the data space.
	if( ( g_shm = shmat( g_shmid, NULL, 0 ) ) == (char *)-1 ) {
		perror( "shmat" );
		exit(1);
	}

	p_pd_shm = (PD_SHM_INFO  *)g_shm;
	p_pd_shm->exit = 1;
	usleep(200000);

	// Initialization.
	memset(g_shm, 0, PD_SHMSZ );
	p_pd_shm = (PD_SHM_INFO  *)g_shm;
	p_pd_shm->enc_path[0].w = cap_size_w;
	p_pd_shm->enc_path[0].h = cap_size_h;
	p_pd_shm->pd_path.w = cap_size_w;
	p_pd_shm->pd_path.h = cap_size_h;
}

void exit_share_memory(void)
{
	if (g_shm) {
		shmdt(g_shm);
		shmctl(g_shmid, IPC_RMID, NULL);
	}
}

static HD_RESULT load_input_raw(VIDEO_LIVEVIEW *p_stream)
{

	UINT32 blk_size = POOL_SIZE_USER_DEFINIED;
	HD_COMMON_MEM_DDR_ID ddr_id = VIDEO_DDRID;
	char filepath_raw_main[128];
	FILE *f_in_main;
	UINT32 raw_size = VDO_RAW_BUFSIZE(p_stream->cap_dim.w, p_stream->cap_dim.h, CAP_OUT_FMT);
	int ret = 0;
	UINT32 read_len;


	sprintf(filepath_raw_main, "/mnt/sd/video_frm_%d_%d_raw.dat", p_stream->cap_dim.w, p_stream->cap_dim.h);
	if ((f_in_main = fopen(filepath_raw_main, "rb")) == NULL) {
		printf("open file (%s) fail !!....\r\nPlease copy test pattern to SD Card !!\r\n\r\n", filepath_raw_main);
		return 0;
	}

	/* get memory */
	p_stream->blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_DEFINIED_POOL, blk_size, ddr_id); // Get block from mem pool
	if (p_stream->blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("get block fail, blk = 0x%x\n", p_stream->blk);
		goto fclose;
	}
	p_stream->pa = hd_common_mem_blk2pa(p_stream->blk); // get physical addr
	if (p_stream->pa == 0) {
		printf("blk2pa fail, blk(0x%x)\n", p_stream->blk);
		goto rel_blk;
	}
	if (p_stream->pa > 0) {
		p_stream->va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, p_stream->pa, blk_size); // Get virtual addr
		if (p_stream->va == 0) {
			printf("get va fail, va(0x%x)\n", p_stream->blk);
			goto rel_blk;
		}
	}

	//--- Read YUV from file ---
	read_len = fread((void *)p_stream->va, 1, raw_size, f_in_main);
	if (read_len != raw_size) {
		if (feof(f_in_main)) {
			fseek(f_in_main, 0, SEEK_SET);  //rewind and try again
			read_len = fread((void *)p_stream->va, 1, raw_size, f_in_main);
			if (read_len != raw_size) {
				printf("reading len error\n");
				goto rel_blk;
			}
		}
	}

	//--- data is written by CPU, flush CPU cache to PHY memory ---
	hd_common_mem_flush_cache((void *)p_stream->va, raw_size);

	goto fclose;

rel_blk:
	ret = hd_common_mem_release_block(p_stream->blk);
	if (HD_OK != ret) {
		printf("release blk fail, ret(%d)\n", ret);
	}
fclose:
	/* close output file */
	if (f_in_main) fclose(f_in_main);

	return ret;
}

static HD_RESULT push_input_raw(VIDEO_LIVEVIEW *p_stream)
{
	HD_VIDEO_FRAME video_frame = {0};
	int ret = 0;

	video_frame.sign        = MAKEFOURCC('V','F','R','M');
	video_frame.p_next      = NULL;
	video_frame.ddr_id      = VIDEO_DDRID;
	video_frame.pxlfmt      = HD_VIDEO_PXLFMT_RAW12 | HD_VIDEO_PIX_RGGB_R;
	video_frame.dim.w       = p_stream->cap_dim.w;
	video_frame.dim.h       = p_stream->cap_dim.h;
	video_frame.count       = 0;
	video_frame.timestamp   = hd_gettime_us();
	video_frame.pw[0]       = p_stream->cap_dim.w; // RAW
	video_frame.ph[0]       = p_stream->cap_dim.h; // RAW
	video_frame.loff[0]     = ALIGN_CEIL_4(p_stream->cap_dim.w * HD_VIDEO_PXLFMT_BPP(video_frame.pxlfmt) / 8); // RAW
	video_frame.phy_addr[0] = p_stream->pa; // RAW
	video_frame.blk         = p_stream->blk;

	ret = hd_videoproc_push_in_buf(p_stream->proc_path, &video_frame, NULL, 0); // only support non-blocking mode now
	if (ret != HD_OK) {
		printf("push_in(%d) error = %d!!\r\n", 0, ret);
	}

	return ret;
}

static HD_RESULT release_input_raw(VIDEO_LIVEVIEW *p_stream)
{

	UINT32 blk_size = POOL_SIZE_USER_DEFINIED;
	int ret = 0;

	if (p_stream->va == 0) {
		return ret;
	}

	hd_common_mem_munmap((void*)p_stream->va, blk_size);

	ret = hd_common_mem_release_block(p_stream->blk);
	if (HD_OK != ret) {
		printf("release blk fail, ret(%d)\n", ret);
	}

	return ret;
}

static HD_RESULT init_module(UINT32 out_type)
{
	HD_RESULT ret;

	if ((ret = hd_videocap_init()) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_init()) != HD_OK)
		return ret;

	if (out_type <= 2) {
		if ((ret = hd_videoout_init()) != HD_OK)
			return ret;
	}

	if ((ret = hd_videoenc_init()) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT open_module(VIDEO_LIVEVIEW *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;
	// set videocap config
	if (fix_pattern == FALSE) {
		ret = set_cap_cfg(&p_stream->cap_ctrl, p_stream->cap_ctrl_id, p_stream->shdr_map);
		if (ret != HD_OK) {
			printf("set cap-cfg fail=%d\n", ret);
			return HD_ERR_NG;
		}
	}
	if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
		HD_VIDEOPROC_CTRLFUNC func;

		// set videoproc config
		func = VIDEOPROC_ALG_FUNC;
		#if AI_ENABLE
		func |= HD_VIDEOPROC_FUNC_BNR | HD_VIDEOPROC_FUNC_BNR_STA | HD_VIDEOPROC_FUNC_AIDED;
		#endif
		if (g_shdr == 1) {
			func |= HD_VIDEOPROC_FUNC_SHDR;
			#if AI_ENABLE
			func |= HD_VIDEOPROC_FUNC_FUSION;
			#endif
		}
		ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, p_stream->proc_ctrl_id, p_stream->proc_path_out_id, p_stream->proc_path_3dnr_out_id, HD_VIDEOPROC_PIPE_BNR_RAWALL, func, CAP_OUT_FMT, p_stream->isp_id);
		if (ret != HD_OK) {
			printf("set proc-cfg fail=%d\n", ret);
			return HD_ERR_NG;
		}
	} else {
		HD_VIDEOPROC_CTRLFUNC func;

		// set videoproc config
		func = VIDEOPROC_ALG_FUNC;
		if (g_shdr == 1) {
			func |= HD_VIDEOPROC_FUNC_SHDR;
		}
		ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, p_stream->proc_ctrl_id, p_stream->proc_path_out_id, p_stream->proc_path_3dnr_out_id, HD_VIDEOPROC_PIPE_RAWALL, func, CAP_OUT_FMT, p_stream->isp_id);
		if (ret != HD_OK) {
			printf("set proc-cfg fail=%d\n", ret);
			return HD_ERR_NG;
		}
	}

	if (fix_pattern == FALSE) {
		if ((ret = hd_videocap_open(p_stream->cap_path_in_id, p_stream->cap_path_out_id, &p_stream->cap_path)) != HD_OK)
			return ret;
	}
	if ((ret = hd_videoproc_open(p_stream->proc_path_in_id, p_stream->proc_path_out_id, &p_stream->proc_path)) != HD_OK)
		return ret;

	#if (VPRC_PATH5_ENABLE)
	if ((ret = hd_videoproc_open(p_stream->proc_path_in_id, p_stream->proc_path_3dnr_out_id, &p_stream->proc_path_3dnr)) != HD_OK)
		return ret;
	#endif

	return HD_OK;
}

static HD_RESULT open_module_2(VIDEO_LIVEVIEW *p_stream, UINT32 out_type)
{
	HD_RESULT ret;
	// set videoout config
	ret = set_out_cfg(&p_stream->out_ctrl, out_type,p_stream->hdmi_id);
	if (ret != HD_OK) {
		printf("set out-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	if ((ret = hd_videoout_open(p_stream->out_path_in_id, p_stream->out_path_out_id,  &p_stream->out_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT open_module_3(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret;

	if ((ret = hd_videoenc_open(p_stream->enc_path_in_id, p_stream->enc_path_out_id, &p_stream->enc_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT open_module_4(VIDEO_LIVEVIEW *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;

	// set videoproc config
	ret = set_proc_cfg2(&p_stream->proc_ctrl, p_stream->proc_ctrl_id, p_stream->proc_path_out_id, p_stream->proc_path_3dnr_out_id, p_proc_max_dim, p_stream->isp_id);
	if (ret != HD_OK) {
		printf("set proc-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}

	if ((ret = hd_videoproc_open(p_stream->proc_path_in_id, p_stream->proc_path_out_id, &p_stream->proc_path)) != HD_OK)
		return ret;
	#if (VPRC_PATH5_ENABLE)
	if ((ret = hd_videoproc_open(p_stream->proc_path_in_id, p_stream->proc_path_3dnr_out_id, &p_stream->proc_path_3dnr)) != HD_OK)
		return ret;
	#endif

	if ((ret = hd_videoenc_open(p_stream->enc_path_in_id, p_stream->enc_path_out_id, &p_stream->enc_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT close_module(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret;
	if (fix_pattern == FALSE) {
		if ((ret = hd_videocap_close(p_stream->cap_path)) != HD_OK)
			return ret;
	}

	if ((ret = hd_videoproc_close(p_stream->proc_path)) != HD_OK)
		return ret;

	#if (VPRC_PATH5_ENABLE)
	if ((ret = hd_videoproc_close(p_stream->proc_path_3dnr)) != HD_OK)
		return ret;
	#endif

	return HD_OK;
}

static HD_RESULT close_module_2(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_videoout_close(p_stream->out_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT close_module_3(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_videoenc_close(p_stream->enc_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT close_module_4(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret;

	if ((ret = hd_videoproc_close(p_stream->proc_path)) != HD_OK)
		return ret;

	#if (VPRC_PATH5_ENABLE)
	if ((ret = hd_videoproc_close(p_stream->proc_path_3dnr)) != HD_OK)
		return ret;
	#endif

	if ((ret = hd_videoenc_close(p_stream->enc_path)) != HD_OK)
		return ret;

	return HD_OK;
}


static HD_RESULT exit_module(UINT32 out_type)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_uninit()) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_uninit()) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_uninit()) != HD_OK)
		return ret;
	if (out_type <= 2) {
		if ((ret = hd_videoout_uninit()) != HD_OK)
			return ret;
	}

	return HD_OK;
}

uintptr_t get_post_buf(uint32_t size)
{
	uintptr_t buf = (uintptr_t)malloc(size);

	return buf;

}

static VOID *network_proc_thread(VOID *arg);
static VOID *network_cap_thread(VOID *arg);

static VOID *network_cap_thread(VOID *arg)
{
	HD_RESULT ret = HD_OK;
	VIDEO_LIVEVIEW *p_stream = (VIDEO_LIVEVIEW*)arg;
	VIDEO_LIVEVIEW *p_stream_sub = ((VIDEO_LIVEVIEW*)arg)+1;
retry:

	printf("\r\n");
	while (p_stream->net_proc_start == 0) sleep(1);

	printf("\r\n");
	while (p_stream->net_proc_exit == 0) {

		if (1) {
			HD_VIDEO_FRAME video_frame = {0};

			ret = hd_videocap_pull_out_buf(p_stream->cap_path, &video_frame, -1); // -1 = blocking mode, 0 = non-blocking mode, >0 = blocking-timeout mode
			if(ret != HD_OK) {
				printf("hd_videocap_pull_out_buf fail (%d)\n\r", ret);
				goto retry;
			}

			ret = hd_videoproc_push_in_buf(p_stream->proc_path, &video_frame, NULL, 300); // blocking mode
			if (ret != HD_OK && ret != HD_ERR_OVERRUN) {
				printf("enc_push error=%d !!\r\n\r\n", ret);
			}

			ret = hd_videoproc_push_in_buf(p_stream_sub->proc_path, &video_frame, NULL, 300); // blocking mode
			if (ret != HD_OK && ret != HD_ERR_OVERRUN) {
				printf("22 enc_push error=%d !!\r\n\r\n", ret);
			}

			ret = hd_videocap_release_out_buf(p_stream->cap_path, &video_frame);
			if(ret != HD_OK) {
				printf("hd_videocap_release_out_buf fail (%d)\n\r", ret);
				goto skip;
			}
		}
	}

skip:

	return 0;
}

static VOID *network_proc_thread(VOID *arg)
{
	HD_RESULT ret = HD_OK;
	VIDEO_LIVEVIEW *p_stream = (VIDEO_LIVEVIEW*)arg;
	UINT64 tstart, tend ;
retry:

	printf("\r\n");
	while (p_stream->net_proc_start == 0) sleep(1);

	printf("\r\n");
	tstart = hd_gettime_us();
	while (p_stream->net_proc_exit == 0) {

		if (1) {
			HD_VIDEO_FRAME video_frame = {0};

			ret = hd_videoproc_pull_out_buf(p_stream->proc_path, &video_frame, -1); // -1 = blocking mode, 0 = non-blocking mode, >0 = blocking-timeout mode
			if(ret != HD_OK) {
				printf("hd_videoproc_pull_out_buf fail (%d)\n\r", ret);
				goto retry;
			}

			if (p_stream->out_type <= 2 && vprc_output == VPRC_OUTPUT_VOUT) {
				ret = hd_videoout_push_in_buf(p_stream->out_path, &video_frame, NULL, -1); // blocking mode
				if (ret != HD_OK && ret != HD_ERR_OVERRUN) {
					printf("out_push error=%d !!\r\n\r\n", ret);
					goto skip;
				}
			} else if (vprc_output == VPRC_OUTPUT_VENC || vprc_output == VPRC_OUTPUT_VENC_1OUT) {
				ret = hd_videoenc_push_in_buf(p_stream->enc_path, &video_frame, NULL, 300); // blocking mode
				if (ret != HD_OK && ret != HD_ERR_OVERRUN) {
					printf("33 enc_push error=%d !!\r\n\r\n", ret);
				}
			}

			{
				UINTPTR phy_addr_main, vir_addr_main;
				UINT32 yuv_size;
				#define PHY2VIRT_MAIN(pa) (vir_addr_main + ((pa) - phy_addr_main))

				phy_addr_main = hd_common_mem_blk2pa(video_frame.blk); // Get physical addr
				if (phy_addr_main == 0) {
					printf("blk2pa fail, blk = 0x%x\r\n", video_frame.blk);
					goto skip;
				}

				yuv_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(p_stream->cap_dim.w, p_stream->cap_dim.h, HD_VIDEO_PXLFMT_YUV420);

				// mmap for frame buffer (just mmap one time only, calculate offset to virtual address later)
				vir_addr_main = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_main, yuv_size);
				if (vir_addr_main == 0) {
					printf("mmap error !!\r\n\r\n");
					goto skip;
				}

				if (p_stream->dump_yuv) {
					char file_path_main[50] = {0};
					FILE *f_out_main;

					snprintf(file_path_main, 50, "/mnt/sd/dump_frm_yuv420_%u.dat", p_stream->shot_count);
					printf("dump snapshot frame to file (%s) ....\r\n", file_path_main);

					//----- open output files -----
					if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
						printf("open file (%s) fail....\r\n\r\n", file_path_main);
						goto skip;
					}

					//save Y plane
					{
						UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(video_frame.phy_addr[0]);
						UINT32 len = video_frame.loff[0]*video_frame.ph[0];
						if (f_out_main) fwrite(ptr, 1, len, f_out_main);
						if (f_out_main) fflush(f_out_main);
					}
					//save UV plane
					{
						UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(video_frame.phy_addr[1]);
						UINT32 len = video_frame.loff[1]*video_frame.ph[1];
						if (f_out_main) fwrite(ptr, 1, len, f_out_main);
						if (f_out_main) fflush(f_out_main);
					}
					if (f_out_main) fclose(f_out_main);
				}

				// mummap for frame buffer
				ret = hd_common_mem_munmap((void *)vir_addr_main, yuv_size);
				if (ret != HD_OK) {
					printf("mnumap error !!\r\n\r\n");
					goto skip;
				}

			}

			ret = hd_videoproc_release_out_buf(p_stream->proc_path, &video_frame);
			if(ret != HD_OK) {
				printf("hd_videoproc_release_out_buf fail (%d)\n\r", ret);
				goto skip;
			}

			p_stream->shot_count++;
		}
	}
	tend = hd_gettime_us();
	printf("============== AI ISP time (%d) cnt = (%u) Averg time (%d) ==================\n\r",(int)(tend - tstart), p_stream->shot_count, (int)(tend - tstart)/p_stream->shot_count);

skip:

	return 0;
}

#if (SAVE_RAW_ENABLE)
static void save_raw(UINT32 frame_num, UINT32 frame_size)
{
	AET_STATUS_INFO ae_status = {0};
	AWBT_STATUS awb_status = {0};
	CHAR dir_str[64], file_str[64], full_str[128];
	FILE *f_out_main;
	UINT32 i, j, k, path;
	static UINT32 save_cnt = 1;

	if (frame_size == 0)
		return;

	// NOTE: fix to 1 path
	path = 1;

	ae_status.id = SEN1_VCAP_ID;
	vendor_isp_get_ae(AET_ITEM_STATUS, &ae_status);
	awb_status.id = SEN1_VCAP_ID;
	vendor_isp_get_awb(AWBT_ITEM_STATUS, &awb_status);
	snprintf(dir_str, 63, "/mnt/sd/%d_%s_expt_%d_iso_%d_cg_%d_%d_%d", save_cnt, &sensor_info[sensor_sel].sensor_name[8],
														ae_status.status_info.expotime[0], ae_status.status_info.iso_gain[0],
														awb_status.status.cur_r_gain, awb_status.status.cur_g_gain, awb_status.status.cur_b_gain);

	snprintf(full_str, 127, "mkdir %s", dir_str);
	system(full_str);
	printf("\nCreate directory %s \n", dir_str);

	for (k = 0; k < path; k++) {
		for (i = 0; i < frame_num; i++) {
			for (j = 0; j < cap_idx[k][i]; j++) {
				snprintf(file_str, 63, "RAW%u_w%d_h%d_12b_pack_%u.raw", i + k * frame_num, (int)sensor_info[sensor_sel].size.w, (int)sensor_info[sensor_sel].size.h, j);

				printf("%s \n", file_str);
				snprintf(full_str, 127, "%s/%s", dir_str, file_str);
				//----- open output files -----
				if ((f_out_main = fopen(full_str, "wb")) == NULL) {
					printf("open file (%s) fail.... \n\n", file_str);
					continue;
				}
				if (f_out_main) {
					UINT8 *ptr = (UINT8 *)raw_info[k][i].va[j];

					fwrite(ptr, 1, frame_size, f_out_main);
					fflush(f_out_main);
				}
				fclose(f_out_main);
			}
		}
	}

	save_cnt++;

	system("sync");
	printf("Save raw finish \n");
}

static void *cap_raw_thread(void *arg)
{
	VIDEO_LIVEVIEW *p_stream = (VIDEO_LIVEVIEW *)arg;
	HD_RESULT ret = HD_OK;
	HD_VIDEO_FRAME video_frame[2][MAX_FRAME_NUM] = {0};
	UINTPTR phy_addr_main[2][MAX_FRAME_NUM];
	UINT32 index[2], size[2] = {0};
	UINT32 i, j, path;
	HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;

	// NOTE: fix to 1 path
	path = 1;

	//--------- pull data test ---------
	while (p_stream[0].cap_stop == 0) {
		for (j = 0; j < path; j++) {
			printf("path %d: ", j);
			for (i = 0; i < p_stream[j].cap_frame_num; i++) {
				ret = hd_videocap_pull_out_buf(p_stream[j].cap_path, &video_frame[j][i], -1);// -1 = blocking mode, 0 = non-blocking mode, >0 = blocking-timeout mode
				if (ret != HD_OK) {
					printf("pull_out (%d) error = %d \n", -1, ret);
					continue;
				}

				index[j] = video_frame[j][i].ddr_id >> 8;

				if (index[j] >= p_stream[j].cap_frame_num) {
					printf("something wrong (0x%X) \n", video_frame[j][i].pxlfmt);
					continue;
				}

				phy_addr_main[j][index[j]] = video_frame[j][i].phy_addr[0];
				if (phy_addr_main[j][index[j]] == 0) {
					printf("phy_addr_main[%d] error \n\n", index[j]);
					goto release_out;
				}
				raw_info[j][index[j]].vd_cnt[cap_idx[j][index[j]]] = video_frame[j][i].count;

				printf("cnt %d, vd %d, ", cap_idx[j][index[j]], (unsigned int)video_frame[j][i].count);
			}
		}
		printf("\n");

		size[0] = video_frame[0][0].loff[0] * video_frame[0][0].ph[0];
		size[1] = video_frame[1][0].loff[0] * video_frame[1][0].ph[0];

		//copy raw
		for (j = 0; j < path; j++) {
			for (i = 0; i < p_stream[j].cap_frame_num; i++) {
				ret = mem_alloc_remain(&raw_info[j][i].pa[cap_idx[j][i]], (void **)&raw_info[j][i].va[cap_idx[j][i]], size[j], ddr_id);
				if (ret != HD_OK) {
					printf("ddr[%d] full, idx = %d \n", ddr_id, cap_idx[j][i]);
					ddr_id++;
					if (ddr_id < MAX_DDR_NUM) {
						ret = mem_alloc_remain(&raw_info[j][i].pa[cap_idx[j][i]], (void **)&raw_info[j][i].va[cap_idx[j][i]], size[j], ddr_id);
						if (ret != HD_OK) {
							//no DDR2
							p_stream[0].cap_stop = 1;
							break;
						}
					} else {
						p_stream[0].cap_stop = 1;
						break;
					}
				}

				if (hd_gfx_memcpy(raw_info[j][i].pa[cap_idx[j][i]], phy_addr_main[j][i], size[j]) == NULL) {
					printf("hd_gfx_memcpy fail \n");
				}

				cap_idx[j][i]++;
				if (cap_idx[j][p_stream[j].cap_frame_num - 1] >= p_stream[j].cap_num) {
					p_stream[0].cap_stop = 1;
					break;
				}
			}
		}

		release_out:
		for (j = 0; j < path; j++) {
			for (i = 0; i < p_stream[j].cap_frame_num; i++) {
				ret = hd_videocap_release_out_buf(p_stream[j].cap_path, &video_frame[j][i]);
				if (ret != HD_OK) {
					printf("cap_release fial (%d) (%d) \n\n", j, i);
				}
			}
		}

		usleep(500);
	}

	save_raw(p_stream[0].cap_frame_num, size[0]);
	p_stream[0].cap_exit = 1;

	return 0;
}
#endif


static HD_RESULT hdal_open(VIDEO_LIVEVIEW *p_stream, VIDEO_LIVEVIEW *p_stream_sub, UINT32 out_type, UINT32 enc_type, UINT32 enc_bitrate)
{
	HD_RESULT ret = 0;

	// open video_liveview modules (main)
	ret = open_module(p_stream, &p_stream->proc_max_dim);
	if (ret != HD_OK) {
		printf("open fail=%d\n", ret);
		return HD_ERR_FAIL;
	}


	// open video_liveview modules (sub)
	if (out_type <= 2 && vprc_output == VPRC_OUTPUT_VOUT) {
		ret = open_module_2(p_stream, out_type);
		if (ret != HD_OK) {
			printf("open2 fail=%d\n", ret);
			return HD_ERR_FAIL;
		}
	}

	if (vprc_output == VPRC_OUTPUT_VENC) {
		ret = open_module_3(p_stream);
		if (ret != HD_OK) {
			printf("open3 fail=%d\n", ret);
			return HD_ERR_FAIL;
		}

		ret = open_module_4(p_stream_sub, &p_stream_sub->proc_max_dim);
		if (ret != HD_OK) {
			printf("open fail=%d\n", ret);
			return HD_ERR_FAIL;
		}
	} else if (vprc_output == VPRC_OUTPUT_VENC_1OUT) {
		ret = open_module_3(p_stream);
		if (ret != HD_OK) {
			printf("open3 fail=%d\n", ret);
			return HD_ERR_FAIL;
		}
	}


	#if AI_ENABLE
	{
		if ((ret = input_init()) != HD_OK) {
			printf("input_init fail=%d\n", ret);
			return HD_ERR_FAIL;
		}
		if ((ret = network_init()) != HD_OK) {
			printf("network_init fail=%d\n", ret);
			return HD_ERR_FAIL;
		}

		if ((ret = network_mem_config(p_stream->net_path, &p_stream->net_proc_cfg)) != HD_OK) {
			printf("network_mem_config stream1 fail=%d\n", ret);
			return HD_ERR_FAIL;
		}

		if ((ret = network_open(p_stream->net_path)) != HD_OK) {
			printf("network_open fail=%d\n", ret);
			return HD_ERR_FAIL;
		}

		if (p_stream->second_ai) {
			ret = network_mem_config(p_stream->second_net_path, &p_stream->second_net_proc_cfg);
			if (ret != HD_OK) {
				printf("network_mem_config stream2 fail (%d) \n", ret);
				return HD_ERR_FAIL;
			}

			if ((ret = network_open(p_stream->second_net_path)) != HD_OK) {
				printf("network_open fail=%d\n", ret);
				return HD_ERR_FAIL;
			}
		}

#if 0 //only for AIISP slice mode
		ret = vendor_videoproc_get(p_stream->proc_ctrl, VENDOR_VIDEOPROC_PARAM_ISP_CB, &p_stream->isp_cb);
		if (ret != HD_OK) {
			printf("get isp cb, ret = %d\r\n", ret);
			return ret;
		}

		ret = network_bind_isp_cb(&p_stream->isp_cb);
		if (ret != HD_OK) {
			printf("bind_isp_cb fail (%d) \n", ret);
			return HD_ERR_FAIL;
		}
#endif

		ret = network_start(p_stream->net_path);
		if (ret != HD_OK) {
			printf("network_start fail (%d) \n", ret);
			return HD_ERR_FAIL;
		}

		if (p_stream->second_ai) {
			ret = network_start(p_stream->second_net_path);
			if (ret != HD_OK) {
				printf("network_start stream2 fail (%d) \n", ret);
				return HD_ERR_FAIL;
			}
		}


		ret = network_get_param_name(p_stream->net_path, 0, &model_param_name);
		if (ret != HD_OK) {
			printf("network_get_param_name fail (%d) \n", ret);
			return HD_ERR_FAIL;
		}

		ret = isp_update_param_name(0, &model_param_name, p_stream->isp_id);
		if (ret != HD_OK) {
			printf("isp_update_param_name fail (%d) \n", ret);
			return HD_ERR_FAIL;
		}


		VENDOR_VIDEOPROC_ISP_AI_EFFECT isp_ai_effect = {0};
		UINT32 proc_id  = ((NET_PROC*)(g_net + p_stream->net_path))->proc_id;

		ret = network_get_ai_cb(&p_stream->ai_cb);
		if (ret != HD_OK) {
			printf("network_get_ai_cb fail (%d) \n", ret);
			return HD_ERR_FAIL;
		}

		ret = vendor_videoproc_set(p_stream->proc_ctrl, VENDOR_VIDEOPROC_PARAM_AI_CB, &p_stream->ai_cb);
		if (ret != HD_OK) {
			printf("set ai cb, ret = %d\r\n", ret);
			return ret;
		}

        isp_ai_effect.enable = 1;
        isp_ai_effect.path_id = 0;
        isp_ai_effect.proc_id[NORMAL_ISO_EFFECT] = proc_id;
		if (p_stream->second_ai) {
			UINT32 proc_id2 = ((NET_PROC*)(g_net + p_stream->second_net_path))->proc_id;

	        isp_ai_effect.proc_id[LOW_ISO_EFFECT] = proc_id2;
		}
        ret = vendor_videoproc_set(p_stream->proc_ctrl, VENDOR_VIDEOPROC_PARAM_ISP_AI_EFFECT, &isp_ai_effect);
		if (ret) {
			printf("set ISP_AI_EFFECT fail=%d\r\n", ret);
		}
    }
	#endif


	if (fix_pattern == FALSE) {
		// get videocap capability
		ret = get_cap_caps(p_stream->cap_ctrl, &p_stream->cap_syscaps);
		if (ret != HD_OK) {
			printf("get cap-caps fail=%d\n", ret);
			return HD_ERR_FAIL;
		}
	}

	// get videoout capability
	if (out_type <= 2 && vprc_output == VPRC_OUTPUT_VOUT) {
		ret = get_out_caps(p_stream->out_ctrl, &p_stream->out_syscaps);
		if (ret != HD_OK) {
			printf("get out-caps fail=%d\n", ret);
			return HD_ERR_FAIL;
		}
		p_stream->out_max_dim = p_stream->out_syscaps.output_dim;
	}

	if (fix_pattern == FALSE) {
		UINT32 depth;

		if (vprc_output == VPRC_OUTPUT_VENC) {
			//no-bind mode
			if (g_shdr == 1) {
				depth = 2;
			} else {
				depth = 1;
			}
		} else {
			depth = 0;
		}

		// set videocap parameter
		ret = set_cap_param(p_stream->cap_path, &p_stream->cap_dim, depth);
		if (ret != HD_OK) {
			printf("set cap fail=%d\n", ret);
			return HD_ERR_FAIL;
		}
	}

	// set videoproc parameter (main)
	ret = set_proc_param(p_stream->proc_path, &p_stream->proc_max_dim, HD_VIDEO_DIR_NONE, 1, VPRC_OUT_FMT);
	if (ret != HD_OK) {
		printf("set proc fail=%d\n", ret);
		return HD_ERR_FAIL;
	}

	#if (VPRC_PATH5_ENABLE)
	ret = set_proc_param(p_stream->proc_path_3dnr, &p_stream->proc_max_dim, HD_VIDEO_DIR_NONE, 1, VPRC_OUT_FMT);
	if (ret != HD_OK) {
		printf("set proc 3dnr fail = %d \n", ret);
		return HD_ERR_FAIL;
	}
	#endif

	// set videoout parameter (sub)
	if (out_type <= 2 && vprc_output == VPRC_OUTPUT_VOUT) {
		ret = set_out_param(p_stream->out_path, &p_stream->out_dim, NULL);
		if (ret != HD_OK) {
			printf("set out fail=%d\n", ret);
			return HD_ERR_FAIL;
		}
	}

	if (vprc_output == VPRC_OUTPUT_VENC) {
		// set videoenc config (main)
		ret = set_enc_cfg(p_stream->enc_path, &p_stream->enc_max_dim, enc_bitrate * 1024 * 1024, p_stream->isp_id);
		if (ret != HD_OK) {
			printf("set enc-cfg fail=%d\n", ret);
			return HD_ERR_FAIL;
		}

		// set videoenc parameter (main)
		ret = set_enc_param(p_stream->enc_path, &p_stream->enc_dim, VPRC_OUT_FMT, enc_type, enc_bitrate * 1024 * 1024);
		if (ret != HD_OK) {
			printf("set enc fail=%d\n", ret);
			return HD_ERR_FAIL;
		}

		// set videoproc parameter (sub)
		ret = set_proc_param(p_stream_sub->proc_path, &p_stream_sub->proc_max_dim, HD_VIDEO_DIR_NONE, 1, VPRC_OUT_FMT);
		if (ret != HD_OK) {
			printf("set proc fail=%d\n", ret);
			return HD_ERR_FAIL;
		}

		#if (VPRC_PATH5_ENABLE)
		ret = set_proc_param(p_stream_sub->proc_path_3dnr, &p_stream_sub->proc_max_dim, HD_VIDEO_DIR_NONE, 1, VPRC_OUT_FMT);
		if (ret != HD_OK) {
			printf("set proc 3dnr fail = %d \n", ret);
			return HD_ERR_FAIL;
		}
		#endif

		// set videoenc config (sub)
		ret = set_enc_cfg(p_stream_sub->enc_path, &p_stream_sub->enc_max_dim, enc_bitrate * 1024 * 1024, p_stream_sub->isp_id);
		if (ret != HD_OK) {
			printf("set enc-cfg fail=%d\n", ret);
			return HD_ERR_FAIL;
		}

		// set videoenc parameter (sub)
		ret = set_enc_param(p_stream_sub->enc_path, &p_stream_sub->enc_dim, VPRC_OUT_FMT, enc_type, enc_bitrate * 1024 * 1024);
		if (ret != HD_OK) {
			printf("set enc fail=%d\n", ret);
			return HD_ERR_FAIL;
		}
	} else if (vprc_output == VPRC_OUTPUT_VENC_1OUT) {
		// set videoenc config (main)
		ret = set_enc_cfg(p_stream->enc_path, &p_stream->enc_max_dim, enc_bitrate * 1024 * 1024, p_stream->isp_id);
		if (ret != HD_OK) {
			printf("set enc-cfg fail=%d\n", ret);
			return HD_ERR_FAIL;
		}

		// set videoenc parameter (main)
		ret = set_enc_param(p_stream->enc_path, &p_stream->enc_dim, VPRC_OUT_FMT, enc_type, enc_bitrate * 1024 * 1024);
		if (ret != HD_OK) {
			printf("set enc fail=%d\n", ret);
			return HD_ERR_FAIL;
		}
	}

	if (fix_pattern == FALSE && vprc_output == VPRC_OUTPUT_VENC_1OUT) {
		// bind video_liveview modules (main)
		hd_videocap_bind(p_stream->cap_path_out_id, p_stream->proc_path_in_id);
		//hd_videoproc_bind(SOURCE_PATH, ...); //src_path is not used
	}

	return ret;
}

static HD_RESULT hdal_start(VIDEO_LIVEVIEW *p_stream, VIDEO_LIVEVIEW *p_stream_sub, UINT32 out_type)
{
	HD_RESULT ret = 0;

	// bind video_liveview modules (sub2)
	//.... bind EXTEND_PATH2 to network

	{
		p_stream->net_proc_start = 0;
		p_stream->net_proc_exit = 0;
		p_stream->net_proc_oneshot = 0;

		p_stream_sub->net_proc_start = 0;
		p_stream_sub->net_proc_exit = 0;
		p_stream_sub->net_proc_oneshot = 0;

		if (vprc_output == VPRC_OUTPUT_VENC) {
			ret = pthread_create(&p_stream->net_cap_thread_id, NULL, network_cap_thread, (VOID*)p_stream);
			if (ret < 0) {
				return HD_ERR_FAIL;
			}

			ret = pthread_create(&p_stream->net_proc_sub_thread_id, NULL, network_proc_thread, (VOID*)p_stream_sub);
			if (ret < 0) {
				return HD_ERR_FAIL;
			}
		}

		ret = pthread_create(&p_stream->net_proc_thread_id, NULL, network_proc_thread, (VOID*)p_stream);
		if (ret < 0) {
			return HD_ERR_FAIL;
		}

		p_stream->net_proc_start = 1;
		p_stream->net_proc_exit = 0;
		p_stream->net_proc_oneshot = 0;

		p_stream_sub->net_proc_start = 1;
		p_stream_sub->net_proc_exit = 0;
		p_stream_sub->net_proc_oneshot = 0;
	}

	if (g_capbind == 1) {

		// start video_liveview modules (main)
		hd_videoproc_start(p_stream->proc_path);
		#if (VPRC_PATH5_ENABLE)
		hd_videoproc_start(p_stream->proc_path_3dnr);
		#endif

		if (fix_pattern == FALSE) {
			hd_videocap_start(p_stream->cap_path);
		}

	} else {
		// start video_liveview modules (main)
		if (fix_pattern == FALSE) {
			hd_videocap_start(p_stream->cap_path);
		}
		hd_videoproc_start(p_stream->proc_path);
		#if (VPRC_PATH5_ENABLE)
		hd_videoproc_start(p_stream->proc_path_3dnr);
		#endif
	}

	if (fix_pattern) {
		load_input_raw(p_stream);
	}

	// start video_liveview modules (sub)
	if (out_type <= 2 && vprc_output == VPRC_OUTPUT_VOUT) {
		hd_videoout_start(p_stream->out_path);
	}

	if (vprc_output == VPRC_OUTPUT_VENC) {
		// start video_liveview modules (main)
		hd_videoenc_start(p_stream->enc_path);

		// start video_liveview modules (sub)
		hd_videoproc_start(p_stream_sub->proc_path);
		#if (VPRC_PATH5_ENABLE)
		hd_videoproc_start(p_stream_sub->proc_path_3dnr);
		#endif

		hd_videoenc_start(p_stream_sub->enc_path);
	} else if (vprc_output == VPRC_OUTPUT_VENC_1OUT) {
		// start video_liveview modules (main)
		hd_videoenc_start(p_stream->enc_path);
	}

	return ret;
}

static HD_RESULT hdal_stop(VIDEO_LIVEVIEW *p_stream, VIDEO_LIVEVIEW *p_stream_sub, UINT32 out_type)
{
	HD_RESULT ret = 0;

	if (g_capbind == 1) {
		// stop video_liveview modules (main)
		hd_videoproc_stop(p_stream->proc_path);

		#if (VPRC_PATH5_ENABLE)
		hd_videoproc_stop(p_stream->proc_path_3dnr);
		#endif

		if (fix_pattern == FALSE) {
			hd_videocap_stop(p_stream->cap_path);
		}
	} else {
		// stop video_liveview modules (main)
		if (fix_pattern == FALSE) {
			hd_videocap_stop(p_stream->cap_path);
		}
		hd_videoproc_stop(p_stream->proc_path);

		#if (VPRC_PATH5_ENABLE)
		hd_videoproc_stop(p_stream->proc_path_3dnr);
		#endif
	}

	// stop video_liveview modules (sub)
	if (out_type <= 2 && vprc_output == VPRC_OUTPUT_VOUT) {
		hd_videoout_stop(p_stream->out_path);
	}

	if (vprc_output == VPRC_OUTPUT_VENC) {
		hd_videoenc_stop(p_stream->enc_path);


		hd_videoproc_stop(p_stream_sub->proc_path);
		#if (VPRC_PATH5_ENABLE)
		hd_videoproc_stop(p_stream_sub->proc_path_3dnr);
		#endif
		hd_videoenc_stop(p_stream_sub->enc_path);
	} else if (vprc_output == VPRC_OUTPUT_VENC_1OUT) {
		hd_videoenc_stop(p_stream->enc_path);
	}

	{
		if (p_stream->net_cap_thread_id) {
			pthread_join(p_stream->net_cap_thread_id, NULL);
		}

		if (p_stream->net_proc_thread_id) {
			pthread_join(p_stream->net_proc_thread_id, NULL);
		}

		if (p_stream->net_proc_sub_thread_id) {
			pthread_join(p_stream->net_proc_sub_thread_id, NULL);
		}
	}

	// destroy aquire_thread
	//pthread_join(stream[VIDEO_LIVE_0].aquire_thread_id, (void* )NULL);

	if (fix_pattern == FALSE && vprc_output != VPRC_OUTPUT_VENC) {
		// unbind video_liveview modules (main)
		hd_videocap_unbind(p_stream->cap_path_out_id);
		//hd_videoproc_unbind(SOURCE_PATH); //src_path is not used
	}

	#if AI_ENABLE
	{
		network_stop(p_stream->net_path);

		if (p_stream->second_ai) {
			network_stop(p_stream->second_net_path);
		}

		if ((ret = network_close(p_stream->net_path)) != HD_OK) {
			return 0;
		}

		if (p_stream->second_ai) {
			if ((ret = network_close(p_stream->second_net_path)) != HD_OK) {
				return 0;
			}
		}
	}
	#endif

	return ret;
}

static HD_RESULT hdal_close(VIDEO_LIVEVIEW *p_stream, VIDEO_LIVEVIEW *p_stream_sub, UINT32 out_type)
{
	HD_RESULT ret = 0;

	// close video_liveview modules (main)
	ret = close_module(p_stream);
	if (ret != HD_OK) {
		printf("close fail=%d\n", ret);
	}

	// close video_liveview modules (sub)
	if (out_type <= 2 && vprc_output == VPRC_OUTPUT_VOUT) {
		ret = close_module_2(p_stream);
		if (ret != HD_OK) {
			printf("close2 fail=%d\n", ret);
		}
	}

	if (vprc_output == VPRC_OUTPUT_VENC) {
		ret = close_module_3(p_stream);
		if (ret != HD_OK) {
			printf("close3 fail=%d\n", ret);
		}

		ret = close_module_4(p_stream_sub);
		if (ret != HD_OK) {
			printf("close4 fail=%d\n", ret);
		}
	} else if (vprc_output == VPRC_OUTPUT_VENC_1OUT) {
		ret = close_module_3(p_stream);
		if (ret != HD_OK) {
			printf("close3 fail=%d\n", ret);
		}
	}
	if (fix_pattern) {
		release_input_raw(p_stream);
	}

	return ret;
}

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
	HD_RESULT ret;
	INT key;

	//vprc main
	HD_DIM main_dim = {0};

	//vprc sub
	HD_DIM sub_dim = {0}; //extend scale

	//vout
	UINT32 out_type = 0xFF;
	HD_DIM out_dim = {0};

	UINT32 cap_size_w = 0;
	UINT32 cap_size_h = 0;

	UINT32 enc_type = 1; // 0:H265; 1:H264
	UINT32 enc_bitrate = 8; // MB

	chip_name = getenv("NVT_CHIP_ID");

	printf("========= TEST VERSION =========\r\n");

	system("insmod /lib/modules/5.10.168/hdal/kflow_ai_isp/kflow_ai_isp.ko");
	system("echo H264FroSync 1 > /proc/kdrv_vdocdc/param");

	if (argc == 1) {
		printf("Usage: <out_type> <fix_pattern> <sensor_sel> <vprc_output> <shdr> <model> <second_ai> <second model> <enc_type> <fps> <bit_rate> <ai_isp_flow> <capbind> <sensor_en_map>\r\n");
		printf("Help:\r\n");

		printf("  <out_type> : \r\n");
		printf("  <fix_pattern> : \r\n");
		printf("  <sensor_sel> : \r\n");
		printf("  <vprc_output> : \r\n");
		printf("  <shdr> : 0(disable), 1(2-frame)\r\n");
		printf("  <model> : \r\n");
		printf("  <second_ai> : \r\n");
		printf("  <second model> : \r\n");
		printf("  <enc_type> : \r\n");
		printf("  <fps> : \r\n");
		printf("  <bit_rate> : \r\n");
		printf("  <ai_isp_flow> : \r\n");
		printf("  <capbind> : \r\n");
		printf("  <sensor_en_map> : 1(sensor1), 2(sensor2), 3(sensor1/2), ...,15(sensor1/2/3/4)\r\n");
		return 0;
	}

	// parse out config
	if (argc >= 2) {
		out_type = atoi(argv[1]);
		printf("out_type %d\r\n", out_type);
		if(out_type > 2) {
			if (out_type != 0xFF) {
				printf("error: not support out_type!\r\n");
				return 0;
			}
		}
	}
	if (argc >= 3) {
		fix_pattern = atoi(argv[2]);
		printf("fix_pattern %d\r\n", fix_pattern);
	}


	if (argc >= 4) {
		sensor_sel = atoi(argv[3]);
		printf("sensor_sel %d\r\n", sensor_sel);
	}
	if (argc >= 5) {
		vprc_output = atoi(argv[4]);

		if (vprc_output == VPRC_OUTPUT_VOUT) {
			vprc_output = VPRC_OUTPUT_VENC_1OUT;
			printf("vprc_output not support %d\r\n", VPRC_OUTPUT_VOUT);
		}
		printf("vprc_output %d\r\n", vprc_output);
	}
	if (argc >= 6) {
		g_shdr = atoi(argv[5]);
		printf("shdr %d\r\n", g_shdr);
	}

	if (argc >= 7) {
		#if AI_ENABLE
		sprintf(stream[VIDEO_LIVE_0].net_proc_cfg.model_filename, "%s", argv[6]);
		printf("model %s\r\n", stream[VIDEO_LIVE_0].net_proc_cfg.model_filename);
		#endif
	}
	if (argc >= 9) {
		stream[VIDEO_LIVE_0].second_ai = atoi(argv[7]);
		if (stream[VIDEO_LIVE_0].second_ai) {
			sprintf(stream[VIDEO_LIVE_0].second_net_proc_cfg.model_filename, "%s", argv[8]);
		}
		printf("second_ai %d, model2 %s\r\n", stream[VIDEO_LIVE_0].second_ai, stream[VIDEO_LIVE_0].second_net_proc_cfg.model_filename);
	}

	if (argc >= 10) {
		enc_type = atoi(argv[9]);

		if(enc_type == 1) {
			printf("enc_type = %d (H264)\r\n", enc_type);
		} else {
			printf("enc_type = %d (H265)\r\n", enc_type);
		}
	}

	if (argc >= 11) {
		g_fps = atoi(argv[10]);
		printf("fps %d\r\n", g_fps);
	}

	if (argc >= 12) {
		g_bitrate = atoi(argv[11]);
		enc_bitrate = g_bitrate;
		printf("bit rate = %d\r\n", g_bitrate);
	}

	if (argc >= 13) {
		ai_isp_flow = atoi(argv[12]);
		printf("ai isp flow = %d\r\n", ai_isp_flow);
	}

	if (argc >= 14) {
		g_capbind = atoi(argv[13]);
		printf("capbind %d\r\n", g_capbind);
	}

	if (argc >= 15) {
		sensor_en_map = atoi(argv[14]);
		printf("sensor_en_map %d\r\n", sensor_en_map);
	}

	if (vprc_output == VPRC_OUTPUT_VENC && g_capbind == 1) {
		g_capbind = 0;
		printf("vprc_output == VPRC_OUTPUT_VENC not support direct, force D2D\r\n");
	}

	if (sensor1_enabled()) {
		sensor_cnt++;
	}

	if (sensor2_enabled()) {
		sensor_cnt++;

		sprintf(stream[VIDEO_LIVE_1].net_proc_cfg.model_filename, "%s", stream[VIDEO_LIVE_0].net_proc_cfg.model_filename);
		stream[VIDEO_LIVE_1].second_ai = stream[VIDEO_LIVE_0].second_ai;
		if (stream[VIDEO_LIVE_1].second_ai) {
			sprintf(stream[VIDEO_LIVE_1].second_net_proc_cfg.model_filename, "%s", stream[VIDEO_LIVE_0].second_net_proc_cfg.model_filename);
		}
	}

	if (sensor3_enabled()) {
		sensor_cnt++;

		sprintf(stream[VIDEO_LIVE_2].net_proc_cfg.model_filename, "%s", stream[VIDEO_LIVE_0].net_proc_cfg.model_filename);
		stream[VIDEO_LIVE_2].second_ai = stream[VIDEO_LIVE_0].second_ai;
		if (stream[VIDEO_LIVE_2].second_ai) {
			sprintf(stream[VIDEO_LIVE_2].second_net_proc_cfg.model_filename, "%s", stream[VIDEO_LIVE_0].second_net_proc_cfg.model_filename);
		}
	}

	if (sensor4_enabled()) {
		sensor_cnt++;

		sprintf(stream[VIDEO_LIVE_3].net_proc_cfg.model_filename, "%s", stream[VIDEO_LIVE_0].net_proc_cfg.model_filename);
		stream[VIDEO_LIVE_3].second_ai = stream[VIDEO_LIVE_0].second_ai;
		if (stream[VIDEO_LIVE_3].second_ai) {
			sprintf(stream[VIDEO_LIVE_3].second_net_proc_cfg.model_filename, "%s", stream[VIDEO_LIVE_0].second_net_proc_cfg.model_filename);
		}
	}

	if (sensor_cnt == 0) {
		printf("error: sensor count is 0\r\n");
		return 0;
	} else if (sensor_cnt > 1 && out_type != 0xFF) {
		printf("error: out_type %d not support multi sensor\r\n", sensor_cnt);
		return 0;
	}

	cap_size_w = sensor_info[sensor_sel].size.w;
	cap_size_h = sensor_info[sensor_sel].size.h;

	init_share_memory(cap_size_w, cap_size_h);

	for (UINT32 i = 0; i < VIDEO_LIVE_CNT_MAX; i++) {
		stream[i].id = i;
	}


	if (fix_pattern == 1) {
		stream[VIDEO_LIVE_0].dump_yuv = 1;
	}

	printf("\r\n\r\n");

	if (sensor1_enabled()) {
		stream[VIDEO_LIVE_0].net_path = 0;
		stream[VIDEO_LIVE_0].second_net_path = 1;
		stream[VIDEO_LIVE_0].in_path = 0;
		stream[VIDEO_LIVE_0].out_type = out_type;

		memcpy((void*)&stream[VIDEO_LIVE_0_SUB], (void*)&stream[VIDEO_LIVE_0], sizeof(VIDEO_LIVEVIEW));

		stream[VIDEO_LIVE_0].cap_ctrl_id               = HD_VIDEOCAP_CTRL(SEN1_VCAP_ID);
		stream[VIDEO_LIVE_0].cap_path_in_id            = HD_VIDEOCAP_IN(SEN1_VCAP_ID, 0);
		stream[VIDEO_LIVE_0].cap_path_out_id           = HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0);
		stream[VIDEO_LIVE_0].shdr_map                  = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR1, (HD_VIDEOCAP_0|HD_VIDEOCAP_1));

		stream[VIDEO_LIVE_0].proc_ctrl_id              = HD_VIDEOPROC_0_CTRL;
		stream[VIDEO_LIVE_0].proc_path_in_id           = HD_VIDEOPROC_0_IN_0;
		stream[VIDEO_LIVE_0].proc_path_out_id          = HD_VIDEOPROC_0_OUT_0;
		stream[VIDEO_LIVE_0].proc_path_out_id          = HD_VIDEOPROC_0_OUT_0;
		stream[VIDEO_LIVE_0].proc_path_3dnr_out_id     = HD_VIDEOPROC_0_OUT_4;

		stream[VIDEO_LIVE_0].out_ctrl_id               = HD_VIDEOOUT_0_CTRL;
		stream[VIDEO_LIVE_0].out_path_in_id            = HD_VIDEOOUT_0_IN_0;
		stream[VIDEO_LIVE_0].out_path_out_id           = HD_VIDEOOUT_0_OUT_0;

		stream[VIDEO_LIVE_0].enc_path_in_id            = HD_VIDEOENC_0_IN_0;
		stream[VIDEO_LIVE_0].enc_path_out_id           = HD_VIDEOENC_0_OUT_0;

		//sub
		stream[VIDEO_LIVE_0_SUB].proc_ctrl_id          = HD_VIDEOPROC_1_CTRL;
		stream[VIDEO_LIVE_0_SUB].proc_path_in_id       = HD_VIDEOPROC_1_IN_0;
		stream[VIDEO_LIVE_0_SUB].proc_path_out_id      = HD_VIDEOPROC_1_OUT_0;
		stream[VIDEO_LIVE_0_SUB].proc_path_3dnr_out_id = HD_VIDEOPROC_1_OUT_4;

		stream[VIDEO_LIVE_0_SUB].enc_path_in_id        = HD_VIDEOENC_0_IN_1;
		stream[VIDEO_LIVE_0_SUB].enc_path_out_id       = HD_VIDEOENC_0_OUT_1;

		stream[VIDEO_LIVE_0].isp_id     = ISP_ID_0;
		stream[VIDEO_LIVE_0_SUB].isp_id = ISP_ID_0;
	}

	if (sensor2_enabled()) {
		stream[VIDEO_LIVE_1].net_path = 2;
		stream[VIDEO_LIVE_1].second_net_path = 3;
		stream[VIDEO_LIVE_1].in_path = 0;
		stream[VIDEO_LIVE_1].out_type = out_type;
		memcpy((void*)&stream[VIDEO_LIVE_1_SUB], (void*)&stream[VIDEO_LIVE_1], sizeof(VIDEO_LIVEVIEW));

		stream[VIDEO_LIVE_1].cap_ctrl_id               = HD_VIDEOCAP_CTRL(SEN2_VCAP_ID);
		stream[VIDEO_LIVE_1].cap_path_in_id            = HD_VIDEOCAP_IN(SEN2_VCAP_ID, 0);
		stream[VIDEO_LIVE_1].cap_path_out_id           = HD_VIDEOCAP_OUT(SEN2_VCAP_ID, 0);
		stream[VIDEO_LIVE_1].shdr_map                  = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR2, (HD_VIDEOCAP_3|HD_VIDEOCAP_4));

		stream[VIDEO_LIVE_1].proc_ctrl_id              = HD_VIDEOPROC_2_CTRL;
		stream[VIDEO_LIVE_1].proc_path_in_id           = HD_VIDEOPROC_2_IN_0;
		stream[VIDEO_LIVE_1].proc_path_out_id          = HD_VIDEOPROC_2_OUT_0;
		stream[VIDEO_LIVE_1].proc_path_out_id          = HD_VIDEOPROC_2_OUT_0;
		stream[VIDEO_LIVE_1].proc_path_3dnr_out_id     = HD_VIDEOPROC_2_OUT_4;

		stream[VIDEO_LIVE_1].out_ctrl_id               = HD_VIDEOOUT_0_CTRL;
		stream[VIDEO_LIVE_1].out_path_in_id            = HD_VIDEOOUT_0_IN_0;
		stream[VIDEO_LIVE_1].out_path_out_id           = HD_VIDEOOUT_0_OUT_0;

		stream[VIDEO_LIVE_1].enc_path_in_id            = HD_VIDEOENC_0_IN_2;
		stream[VIDEO_LIVE_1].enc_path_out_id           = HD_VIDEOENC_0_OUT_2;

		//sub
		stream[VIDEO_LIVE_1_SUB].proc_ctrl_id          = HD_VIDEOPROC_3_CTRL;
		stream[VIDEO_LIVE_1_SUB].proc_path_in_id       = HD_VIDEOPROC_3_IN_0;
		stream[VIDEO_LIVE_1_SUB].proc_path_out_id      = HD_VIDEOPROC_3_OUT_0;
		stream[VIDEO_LIVE_1_SUB].proc_path_3dnr_out_id = HD_VIDEOPROC_3_OUT_4;

		stream[VIDEO_LIVE_1_SUB].enc_path_in_id        = HD_VIDEOENC_0_IN_3;
		stream[VIDEO_LIVE_1_SUB].enc_path_out_id       = HD_VIDEOENC_0_OUT_3;

		stream[VIDEO_LIVE_1].isp_id     = ISP_ID_1;
		stream[VIDEO_LIVE_1_SUB].isp_id = ISP_ID_1;

	}

	if (sensor3_enabled()) {
		stream[VIDEO_LIVE_2].net_path = 4;
		stream[VIDEO_LIVE_2].second_net_path = 5;
		stream[VIDEO_LIVE_2].in_path = 0;
		stream[VIDEO_LIVE_2].out_type = out_type;
		memcpy((void*)&stream[VIDEO_LIVE_2_SUB], (void*)&stream[VIDEO_LIVE_2], sizeof(VIDEO_LIVEVIEW));

		stream[VIDEO_LIVE_2].cap_ctrl_id     = HD_VIDEOCAP_CTRL(SEN3_VCAP_ID);
		stream[VIDEO_LIVE_2].cap_path_in_id  = HD_VIDEOCAP_IN(SEN3_VCAP_ID, 0);
		stream[VIDEO_LIVE_2].cap_path_out_id = HD_VIDEOCAP_OUT(SEN3_VCAP_ID, 0);
		stream[VIDEO_LIVE_2].shdr_map = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR3, (HD_VIDEOCAP_6|HD_VIDEOCAP_7));

		stream[VIDEO_LIVE_2].proc_ctrl_id              = HD_VIDEOPROC_4_CTRL;
		stream[VIDEO_LIVE_2].proc_path_in_id           = HD_VIDEOPROC_4_IN_0;
		stream[VIDEO_LIVE_2].proc_path_out_id          = HD_VIDEOPROC_4_OUT_0;
		stream[VIDEO_LIVE_2].proc_path_out_id          = HD_VIDEOPROC_4_OUT_0;
		stream[VIDEO_LIVE_2].proc_path_3dnr_out_id     = HD_VIDEOPROC_4_OUT_4;

		stream[VIDEO_LIVE_2].out_ctrl_id               = HD_VIDEOOUT_0_CTRL;
		stream[VIDEO_LIVE_2].out_path_in_id            = HD_VIDEOOUT_0_IN_0;
		stream[VIDEO_LIVE_2].out_path_out_id           = HD_VIDEOOUT_0_OUT_0;

		stream[VIDEO_LIVE_2].enc_path_in_id            = HD_VIDEOENC_0_IN_4;
		stream[VIDEO_LIVE_2].enc_path_out_id           = HD_VIDEOENC_0_OUT_4;

		//sub
		stream[VIDEO_LIVE_2_SUB].proc_ctrl_id          = HD_VIDEOPROC_5_CTRL;
		stream[VIDEO_LIVE_2_SUB].proc_path_in_id       = HD_VIDEOPROC_5_IN_0;
		stream[VIDEO_LIVE_2_SUB].proc_path_out_id      = HD_VIDEOPROC_5_OUT_0;
		stream[VIDEO_LIVE_2_SUB].proc_path_3dnr_out_id = HD_VIDEOPROC_5_OUT_4;

		stream[VIDEO_LIVE_2_SUB].enc_path_in_id        = HD_VIDEOENC_0_IN_5;
		stream[VIDEO_LIVE_2_SUB].enc_path_out_id       = HD_VIDEOENC_0_OUT_5;

		stream[VIDEO_LIVE_2].isp_id     = ISP_ID_2;
		stream[VIDEO_LIVE_2_SUB].isp_id = ISP_ID_2;

	}

	if (sensor4_enabled()) {
		stream[VIDEO_LIVE_3].net_path = 6;
		stream[VIDEO_LIVE_3].second_net_path = 7;
		stream[VIDEO_LIVE_3].in_path = 0;
		stream[VIDEO_LIVE_3].out_type = out_type;
		memcpy((void*)&stream[VIDEO_LIVE_3_SUB], (void*)&stream[VIDEO_LIVE_3], sizeof(VIDEO_LIVEVIEW));

		stream[VIDEO_LIVE_3].cap_ctrl_id     = HD_VIDEOCAP_CTRL(SEN4_VCAP_ID);
		stream[VIDEO_LIVE_3].cap_path_in_id  = HD_VIDEOCAP_IN(SEN4_VCAP_ID, 0);
		stream[VIDEO_LIVE_3].cap_path_out_id = HD_VIDEOCAP_OUT(SEN4_VCAP_ID, 0);
		stream[VIDEO_LIVE_3].shdr_map = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR4, (HD_VIDEOCAP_9|HD_VIDEOCAP_10));

		stream[VIDEO_LIVE_3].proc_ctrl_id              = HD_VIDEOPROC_6_CTRL;
		stream[VIDEO_LIVE_3].proc_path_in_id           = HD_VIDEOPROC_6_IN_0;
		stream[VIDEO_LIVE_3].proc_path_out_id          = HD_VIDEOPROC_6_OUT_0;
		stream[VIDEO_LIVE_3].proc_path_out_id          = HD_VIDEOPROC_6_OUT_0;
		stream[VIDEO_LIVE_3].proc_path_3dnr_out_id     = HD_VIDEOPROC_6_OUT_4;

		stream[VIDEO_LIVE_3].out_ctrl_id               = HD_VIDEOOUT_0_CTRL;
		stream[VIDEO_LIVE_3].out_path_in_id            = HD_VIDEOOUT_0_IN_0;
		stream[VIDEO_LIVE_3].out_path_out_id           = HD_VIDEOOUT_0_OUT_0;

		stream[VIDEO_LIVE_3].enc_path_in_id            = HD_VIDEOENC_0_IN_6;
		stream[VIDEO_LIVE_3].enc_path_out_id           = HD_VIDEOENC_0_OUT_6;

		//sub
		stream[VIDEO_LIVE_3_SUB].proc_ctrl_id          = HD_VIDEOPROC_7_CTRL;
		stream[VIDEO_LIVE_3_SUB].proc_path_in_id       = HD_VIDEOPROC_7_IN_0;
		stream[VIDEO_LIVE_3_SUB].proc_path_out_id      = HD_VIDEOPROC_7_OUT_0;
		stream[VIDEO_LIVE_3_SUB].proc_path_3dnr_out_id = HD_VIDEOPROC_7_OUT_4;

		stream[VIDEO_LIVE_3_SUB].enc_path_in_id        = HD_VIDEOENC_0_IN_7;
		stream[VIDEO_LIVE_3_SUB].enc_path_out_id       = HD_VIDEOENC_0_OUT_7;

		stream[VIDEO_LIVE_3].isp_id     = ISP_ID_3;
		stream[VIDEO_LIVE_3_SUB].isp_id = ISP_ID_3;
	}

	// init hdal
	ret = hd_common_init(0);
	if (ret != HD_OK) {
		printf("hd_common_init fail=%d\n", ret);
		goto exit;
	}
	// set project config for AI
	hd_common_sysconfig(0, (1<<16), 0, VENDOR_AI_CFG); //enable AI engine

	// init mem
	{
		INT32 i = 0; // mempool index

		VENDOR_COMM_MEM_CFG_MAX_BUF  cfg_max_buf = {0};

		cfg_max_buf.md_func = 1;
		cfg_max_buf.vprc_max_in_size.w = cap_size_w;
		cfg_max_buf.vprc_max_in_size.h = cap_size_h;
		cfg_max_buf.eis_func = 1;
		cfg_max_buf.gyro_max_num = 64;
		cfg_max_buf.raw_compress_ratio = 50;
		ret = vendor_common_mem_set(VENDOR_COMMON_MEM_ITEM_CFG_MAX_BUF, (VOID *)&cfg_max_buf);
		if(ret != HD_OK) {
			printf("test_cfg_max_buf fail=%d\r\n", ret);
			return ret;
	    }

		if (sensor1_enabled()) {
			stream[VIDEO_LIVE_0].cap_dim.w = cap_size_w; //assign by user
			stream[VIDEO_LIVE_0].cap_dim.h = cap_size_h; //assign by user

			stream[VIDEO_LIVE_0].proc_max_dim.w = cap_size_w; //assign by user
			stream[VIDEO_LIVE_0].proc_max_dim.h = cap_size_h; //assign by user
		}

		if (sensor2_enabled()) {
			stream[VIDEO_LIVE_1].cap_dim.w = cap_size_w; //assign by user
			stream[VIDEO_LIVE_1].cap_dim.h = cap_size_h; //assign by user

			stream[VIDEO_LIVE_1].proc_max_dim.w = cap_size_w; //assign by user
			stream[VIDEO_LIVE_1].proc_max_dim.h = cap_size_h; //assign by user
		}

		if (sensor3_enabled()) {
			stream[VIDEO_LIVE_2].cap_dim.w = cap_size_w; //assign by user
			stream[VIDEO_LIVE_2].cap_dim.h = cap_size_h; //assign by user

			stream[VIDEO_LIVE_2].proc_max_dim.w = cap_size_w; //assign by user
			stream[VIDEO_LIVE_2].proc_max_dim.h = cap_size_h; //assign by user
		}

		if (sensor4_enabled()) {
			stream[VIDEO_LIVE_3].cap_dim.w = cap_size_w; //assign by user
			stream[VIDEO_LIVE_3].cap_dim.h = cap_size_h; //assign by user

			stream[VIDEO_LIVE_3].proc_max_dim.w = cap_size_w; //assign by user
			stream[VIDEO_LIVE_3].proc_max_dim.h = cap_size_h; //assign by user
		}


		i = cap_mem_config(&mem_cfg, &stream[VIDEO_LIVE_0].cap_dim, i, vprc_output, sensor_cnt);
		i = proc_mem_config(&mem_cfg, &stream[VIDEO_LIVE_0].proc_max_dim, i, vprc_output, sensor_cnt);
		if (out_type <= 2) {
			i = out_mem_config(&mem_cfg, &stream[VIDEO_LIVE_0].proc_max_dim, i);
		}
		i = user_mem_config(&mem_cfg, i);

		ret = hd_common_mem_init(&mem_cfg);
		if (HD_OK != ret) {
			printf("hd_common_mem_init fail (%d) \n", ret);
			goto exit;
		}
	}

	// reload ISP cfg
	ret = isp_init();
	if (ret != HD_OK) {
		printf("isp_init fail=%d\n", ret);
	}

	if (sensor1_enabled()) {
		ret = isp_init_sensor(&sensor_info[sensor_sel], chip_name, g_shdr, stream[VIDEO_LIVE_0].isp_id);
		if (ret != HD_OK) {
			printf("isp_init fail=%d\n", ret);
		}
	}

	if (sensor2_enabled()) {
		ret = isp_init_sensor(&sensor_info[sensor_sel], chip_name, g_shdr, stream[VIDEO_LIVE_1].isp_id);
		if (ret != HD_OK) {
			printf("isp_init fail=%d\n", ret);
		}
	}

	if (sensor3_enabled()) {
		ret = isp_init_sensor(&sensor_info[sensor_sel], chip_name, g_shdr, stream[VIDEO_LIVE_2].isp_id);
		if (ret != HD_OK) {
			printf("isp_init fail=%d\n", ret);
		}
	}

	if (sensor4_enabled()) {
		ret = isp_init_sensor(&sensor_info[sensor_sel], chip_name, g_shdr, stream[VIDEO_LIVE_3].isp_id);
		if (ret != HD_OK) {
			printf("isp_init fail=%d\n", ret);
		}
	}

	// init all modules
	ret = init_module(out_type);
	if (ret != HD_OK) {
		printf("init fail=%d\n", ret);
		goto exit;
	}

	// assign parameter by program options
	main_dim.w = cap_size_w;
	main_dim.h = cap_size_h;
	out_dim.w = main_dim.w; //display device size
	out_dim.h = main_dim.h; //display device size
	sub_dim.w = out_dim.w;
	sub_dim.h = out_dim.h;

	// open and init hdal
	if (sensor1_enabled()) {

		stream[VIDEO_LIVE_0].enc_max_dim.w = main_dim.w;
		stream[VIDEO_LIVE_0].enc_max_dim.h = main_dim.h;
		// set videoenc parameter (main)
		stream[VIDEO_LIVE_0].enc_dim.w = main_dim.w;
		stream[VIDEO_LIVE_0].enc_dim.h = main_dim.h;
		stream[VIDEO_LIVE_0].out_dim.w = sub_dim.w;
		stream[VIDEO_LIVE_0].out_dim.h = sub_dim.h;


		stream[VIDEO_LIVE_0_SUB].cap_dim.w = cap_size_w; //assign by user
		stream[VIDEO_LIVE_0_SUB].cap_dim.h = cap_size_h; //assign by user
		stream[VIDEO_LIVE_0_SUB].proc_max_dim.w = cap_size_w; //assign by user
		stream[VIDEO_LIVE_0_SUB].proc_max_dim.h = cap_size_h; //assign by user
		stream[VIDEO_LIVE_0_SUB].enc_max_dim.w = main_dim.w;
		stream[VIDEO_LIVE_0_SUB].enc_max_dim.h = main_dim.h;
		// set videoenc parameter (sub)
		stream[VIDEO_LIVE_0_SUB].enc_dim.w = main_dim.w;
		stream[VIDEO_LIVE_0_SUB].enc_dim.h = main_dim.h;

		ret = hdal_open(&stream[VIDEO_LIVE_0], &stream[VIDEO_LIVE_0_SUB], out_type, enc_type, enc_bitrate);
		if (ret != 0) {
			goto exit;
		}
	}

	if (sensor2_enabled()) {

		stream[VIDEO_LIVE_1].enc_max_dim.w = main_dim.w;
		stream[VIDEO_LIVE_1].enc_max_dim.h = main_dim.h;
		// set videoenc parameter (main)
		stream[VIDEO_LIVE_1].enc_dim.w = main_dim.w;
		stream[VIDEO_LIVE_1].enc_dim.h = main_dim.h;
		stream[VIDEO_LIVE_1].out_dim.w = sub_dim.w;
		stream[VIDEO_LIVE_1].out_dim.h = sub_dim.h;


		stream[VIDEO_LIVE_1_SUB].cap_dim.w = cap_size_w; //assign by user
		stream[VIDEO_LIVE_1_SUB].cap_dim.h = cap_size_h; //assign by user
		stream[VIDEO_LIVE_1_SUB].proc_max_dim.w = cap_size_w; //assign by user
		stream[VIDEO_LIVE_1_SUB].proc_max_dim.h = cap_size_h; //assign by user
		stream[VIDEO_LIVE_1_SUB].enc_max_dim.w = main_dim.w;
		stream[VIDEO_LIVE_1_SUB].enc_max_dim.h = main_dim.h;
		// set videoenc parameter (sub)
		stream[VIDEO_LIVE_1_SUB].enc_dim.w = main_dim.w;
		stream[VIDEO_LIVE_1_SUB].enc_dim.h = main_dim.h;

		ret = hdal_open(&stream[VIDEO_LIVE_1], &stream[VIDEO_LIVE_1_SUB], out_type, enc_type, enc_bitrate);
		if (ret != 0) {
			goto exit;
		}
	}

	if (sensor3_enabled()) {

		stream[VIDEO_LIVE_2].enc_max_dim.w = main_dim.w;
		stream[VIDEO_LIVE_2].enc_max_dim.h = main_dim.h;
		// set videoenc parameter (main)
		stream[VIDEO_LIVE_2].enc_dim.w = main_dim.w;
		stream[VIDEO_LIVE_2].enc_dim.h = main_dim.h;
		stream[VIDEO_LIVE_2].out_dim.w = sub_dim.w;
		stream[VIDEO_LIVE_2].out_dim.h = sub_dim.h;


		stream[VIDEO_LIVE_2_SUB].cap_dim.w = cap_size_w; //assign by user
		stream[VIDEO_LIVE_2_SUB].cap_dim.h = cap_size_h; //assign by user
		stream[VIDEO_LIVE_2_SUB].proc_max_dim.w = cap_size_w; //assign by user
		stream[VIDEO_LIVE_2_SUB].proc_max_dim.h = cap_size_h; //assign by user
		stream[VIDEO_LIVE_2_SUB].enc_max_dim.w = main_dim.w;
		stream[VIDEO_LIVE_2_SUB].enc_max_dim.h = main_dim.h;
		// set videoenc parameter (sub)
		stream[VIDEO_LIVE_2_SUB].enc_dim.w = main_dim.w;
		stream[VIDEO_LIVE_2_SUB].enc_dim.h = main_dim.h;

		ret = hdal_open(&stream[VIDEO_LIVE_0], &stream[VIDEO_LIVE_2_SUB], out_type, enc_type, enc_bitrate);
		if (ret != 0) {
			goto exit;
		}
	}

	if (sensor4_enabled()) {

		stream[VIDEO_LIVE_3].enc_max_dim.w = main_dim.w;
		stream[VIDEO_LIVE_3].enc_max_dim.h = main_dim.h;
		// set videoenc parameter (main)
		stream[VIDEO_LIVE_3].enc_dim.w = main_dim.w;
		stream[VIDEO_LIVE_3].enc_dim.h = main_dim.h;
		stream[VIDEO_LIVE_3].out_dim.w = sub_dim.w;
		stream[VIDEO_LIVE_3].out_dim.h = sub_dim.h;


		stream[VIDEO_LIVE_3_SUB].cap_dim.w = cap_size_w; //assign by user
		stream[VIDEO_LIVE_3_SUB].cap_dim.h = cap_size_h; //assign by user
		stream[VIDEO_LIVE_3_SUB].proc_max_dim.w = cap_size_w; //assign by user
		stream[VIDEO_LIVE_3_SUB].proc_max_dim.h = cap_size_h; //assign by user
		stream[VIDEO_LIVE_3_SUB].enc_max_dim.w = main_dim.w;
		stream[VIDEO_LIVE_3_SUB].enc_max_dim.h = main_dim.h;
		// set videoenc parameter (sub)
		stream[VIDEO_LIVE_3_SUB].enc_dim.w = main_dim.w;
		stream[VIDEO_LIVE_3_SUB].enc_dim.h = main_dim.h;

		ret = hdal_open(&stream[VIDEO_LIVE_3], &stream[VIDEO_LIVE_3_SUB], out_type, enc_type, enc_bitrate);
		if (ret != 0) {
			goto exit;
		}
	}

	// start hdal
	if (sensor1_enabled()) {
		hdal_start(&stream[VIDEO_LIVE_0], &stream[VIDEO_LIVE_0_SUB], out_type);
	}

	if (sensor2_enabled()) {
		hdal_start(&stream[VIDEO_LIVE_1], &stream[VIDEO_LIVE_1_SUB], out_type);
	}

	if (sensor3_enabled()) {
		hdal_start(&stream[VIDEO_LIVE_2], &stream[VIDEO_LIVE_2_SUB], out_type);
	}

	if (sensor4_enabled()) {
		hdal_start(&stream[VIDEO_LIVE_3], &stream[VIDEO_LIVE_3_SUB], out_type);
	}

	#if (ISP_SMART_PARAM_ENABLE)
	pthread_t isp_smart_param_thread_id;
	//sleep(1);
	if (pthread_create(&isp_smart_param_thread_id, NULL, isp_smart_param_thread, (void *)NULL) < 0) {
		printf("create isp smart param thread failed \n");
		goto exit;
	}
	#endif

	//start rtsp
	system("nvtrtspd_ipc &");
	printf("nvtrtspd_ipc\r\n");

	printf("\r\nEnter q to exit, Enter d to debug\r\n");
	do {
		key = getchar();
		if (key == 'q' || key == 0x3) {
			PD_SHM_INFO  *p_pd_shm;

			stream[VIDEO_LIVE_0].net_proc_exit = 1;
			stream[VIDEO_LIVE_0_SUB].net_proc_exit = 1;

			stream[VIDEO_LIVE_1].net_proc_exit = 1;
			stream[VIDEO_LIVE_1_SUB].net_proc_exit = 1;

			stream[VIDEO_LIVE_2].net_proc_exit = 1;
			stream[VIDEO_LIVE_2_SUB].net_proc_exit = 1;

			stream[VIDEO_LIVE_3].net_proc_exit = 1;
			stream[VIDEO_LIVE_3_SUB].net_proc_exit = 1;
			// quit program
			// notify child process
			p_pd_shm = (PD_SHM_INFO  *)g_shm;
			p_pd_shm->exit = 1;
			usleep(300000);
			break;
		}

		if (key == 'm') {
			system("echo saveraw 0 > /proc/kflow_ipp/cmd");
			printf("dump pre\r\n");
		}

		if (key == 'n') {
			system("echo saveraw 3 > /proc/kflow_ipp/cmd");
			printf("dump raw all lite\r\n");
		}

		if (key == 'd') {
			hd_debug_run_menu(); // call debug menu
			printf("\r\nEnter q to exit, Enter d to debug\r\n");
		}

		if (key == 'h') {
			system("echo dump all >/proc/kdrv_ipp/cmd");
		}

		if (key == 'a') {
			system("cat /proc/kdrv_ipp/utilization");
		}

		if (key == 'z') {
			system("echo dumpt > /proc/kflow_ipp/cmd");
		}

		if (key == 's') {
			if (fix_pattern) {
				push_input_raw(&stream[VIDEO_LIVE_0]);
				printf("push fix pattern to vprc\r\n");
			}
		}

		if (key == 'k') {
			system("echo w cfg 0 1000 > /proc/nvt_drv_sys/dram_info");
			system("echo w start 0 > /proc/nvt_drv_sys/dram_info");
		}

		if (key == 'l') {
			system("echo w stop 0 > /proc/nvt_drv_sys/dram_info");
		}

		if (key == 'i') {
			system("cat /proc/kflow_ipp/info");
			system("echo dump all >/proc/kdrv_ipp/cmd");
		}

		if (key == 'o') {
			#if (AI_ENABLE)
			if (sensor1_enabled()) {
				isp_set_config(TRUE, 0xFF, TRUE, stream[VIDEO_LIVE_0].isp_id);
			}

			if (sensor2_enabled()) {
				isp_set_config(TRUE, 0xFF, TRUE, stream[VIDEO_LIVE_1].isp_id);
			}

			if (sensor3_enabled()) {
				isp_set_config(TRUE, 0xFF, TRUE, stream[VIDEO_LIVE_2].isp_id);
			}

			if (sensor4_enabled()) {
				isp_set_config(TRUE, 0xFF, TRUE, stream[VIDEO_LIVE_3].isp_id);
			}
			printf("enable aiisp\r\n");
			#else
			printf("AI is disabled\r\n");
			#endif
		}
		if (key == 'b') {
			system("echo r param 0 0 > /proc/hdal/vendor/iq/cmd");
			system("cat /proc/hdal/vendor/iq/cmd");
		}

		if (key == 'p') {
			#if (AI_ENABLE)
			if (sensor1_enabled()) {
				isp_set_config(TRUE, 0xFF, FALSE, stream[VIDEO_LIVE_0].isp_id);
			}

			if (sensor2_enabled()) {
				isp_set_config(TRUE, 0xFF, FALSE, stream[VIDEO_LIVE_1].isp_id);
			}

			if (sensor3_enabled()) {
				isp_set_config(TRUE, 0xFF, FALSE, stream[VIDEO_LIVE_2].isp_id);
			}

			if (sensor4_enabled()) {
				isp_set_config(TRUE, 0xFF, FALSE, stream[VIDEO_LIVE_3].isp_id);
			}
			printf("disable aiisp\r\n");
			#else
			printf("AI is disabled\r\n");
			#endif
		}

		if (key == '0') {
			#if (AI_ENABLE)
			if (sensor1_enabled()) {
				isp_set_config(TRUE, 0, TRUE, stream[VIDEO_LIVE_0].isp_id);
			}

			if (sensor2_enabled()) {
				isp_set_config(TRUE, 0, TRUE, stream[VIDEO_LIVE_1].isp_id);
			}

			if (sensor3_enabled()) {
				isp_set_config(TRUE, 0, TRUE, stream[VIDEO_LIVE_2].isp_id);
			}

			if (sensor4_enabled()) {
				isp_set_config(TRUE, 0, TRUE, stream[VIDEO_LIVE_3].isp_id);
			}
			printf("set aiisp config 0\r\n");
			#else
			printf("AI is disabled\r\n");
			#endif
		}

		if (key == '1') {
			#if (AI_ENABLE)
			if (sensor1_enabled()) {
				isp_set_config(TRUE, 1, TRUE, stream[VIDEO_LIVE_0].isp_id);
			}

			if (sensor2_enabled()) {
				isp_set_config(TRUE, 1, TRUE, stream[VIDEO_LIVE_1].isp_id);
			}

			if (sensor3_enabled()) {
				isp_set_config(TRUE, 1, TRUE, stream[VIDEO_LIVE_2].isp_id);
			}

			if (sensor4_enabled()) {
				isp_set_config(TRUE, 1, TRUE, stream[VIDEO_LIVE_3].isp_id);
			}
			printf("set aiisp config 1\r\n");
			#else
			printf("AI is disabled\r\n");
			#endif
		}

		if (key == '2') {
			#if (AI_ENABLE)
			if (sensor1_enabled()) {
				isp_set_config(TRUE, 2, TRUE, stream[VIDEO_LIVE_0].isp_id);
			}

			if (sensor2_enabled()) {
				isp_set_config(TRUE, 2, TRUE, stream[VIDEO_LIVE_1].isp_id);
			}

			if (sensor3_enabled()) {
				isp_set_config(TRUE, 2, TRUE, stream[VIDEO_LIVE_2].isp_id);
			}

			if (sensor4_enabled()) {
				isp_set_config(TRUE, 2, TRUE, stream[VIDEO_LIVE_3].isp_id);
			}

			printf("set aiisp config 2\r\n");
			#else
			printf("AI is disabled\r\n");
			#endif
		}

		if (key == '3') {
			#if (AI_ENABLE)
			if (sensor1_enabled()) {
				isp_set_config(TRUE, 3, TRUE, stream[VIDEO_LIVE_0].isp_id);
			}

			if (sensor2_enabled()) {
				isp_set_config(TRUE, 3, TRUE, stream[VIDEO_LIVE_1].isp_id);
			}

			if (sensor3_enabled()) {
				isp_set_config(TRUE, 3, TRUE, stream[VIDEO_LIVE_2].isp_id);
			}

			if (sensor4_enabled()) {
				isp_set_config(TRUE, 3, TRUE, stream[VIDEO_LIVE_3].isp_id);
			}
			printf("set aiisp config 3\r\n");
			#else
			printf("AI is disabled\r\n");
			#endif
		}

		if (key == '4') {
			#if (AI_ENABLE)
			if (sensor1_enabled()) {
				isp_set_config(TRUE, 4, TRUE, stream[VIDEO_LIVE_0].isp_id);
			}

			if (sensor2_enabled()) {
				isp_set_config(TRUE, 4, TRUE, stream[VIDEO_LIVE_1].isp_id);
			}

			if (sensor3_enabled()) {
				isp_set_config(TRUE, 4, TRUE, stream[VIDEO_LIVE_2].isp_id);
			}

			if (sensor4_enabled()) {
				isp_set_config(TRUE, 4, TRUE, stream[VIDEO_LIVE_3].isp_id);
			}
			printf("set aiisp config 4\r\n");
			#else
			printf("AI is disabled\r\n");
			#endif
		}

		if (key == '5') {
			#if (AI_ENABLE)
			if (sensor1_enabled()) {
				isp_set_config(TRUE, 5, TRUE, stream[VIDEO_LIVE_0].isp_id);
			}

			if (sensor2_enabled()) {
				isp_set_config(TRUE, 5, TRUE, stream[VIDEO_LIVE_1].isp_id);
			}

			if (sensor3_enabled()) {
				isp_set_config(TRUE, 5, TRUE, stream[VIDEO_LIVE_2].isp_id);
			}

			if (sensor4_enabled()) {
				isp_set_config(TRUE, 5, TRUE, stream[VIDEO_LIVE_3].isp_id);
			}
			printf("set aiisp config 5\r\n");
			#else
			printf("AI is disabled\r\n");
			#endif
		}

		#if (SAVE_RAW_ENABLE)
		if (key == 'v') {
			VENDOR_COMM_MAX_FREE_BLOCK max_free_block = {0};
			CHAR buf[32];
			INT rt;
			UINT32 val, raw_size;
			UINT32 i;

			#if 0
			if ((is_direct_mode == 1) && (sensor_sel_2 == SEN_SEL_NONE)) {
				printf("Does not support raw cap in direct mode \n", cap_max_num);
				continue;
			}
			#endif

			raw_size = ((sensor_info[sensor_sel].size.w * 3 / 2) * sensor_info[sensor_sel].size.h) * (g_shdr + 1);

			ret = hd_gfx_init();
			if(ret != HD_OK) {
				printf("init gfx fail (%d) \n", ret);
				goto exit;
			}

			cap_max_num = 0;
			for (i = 0; i < MAX_DDR_NUM; i++) {
				max_free_block.ddr = i;
				if (vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_MAX_FREE_BLOCK_SIZE, &max_free_block) == HD_OK) {
					if (max_free_block.size) {
						if (HD_OK == hd_common_mem_alloc("RAW", &ddr_max_free[i].pa, (void **)&ddr_max_free[i].va, max_free_block.size, max_free_block.ddr)) {
							ddr_remain[i].pa = ddr_max_free[i].pa;
							ddr_remain[i].va = ddr_max_free[i].va;
							ddr_remain[i].size = ddr_max_free[i].size = max_free_block.size;
							cap_max_num += ddr_max_free[i].size / raw_size;
						}
					}
				}
			}
			if (cap_max_num >= MAX_RAW_QUEUE) {
				cap_max_num = MAX_RAW_QUEUE;
			}

			printf("Please input capture number, maximum is about %d \n", cap_max_num);
			rt = scanf("%d", &val);
			if (rt != 1) {
				clearerr(stdin);
				fgets(buf, sizeof(buf), stdin);
				printf("Invalid cap_num. set to 1 \n");
				stream[VIDEO_LIVE_0].cap_num = 1;
				stream[VIDEO_LIVE_0_SUB].cap_num = 1;
			} else if (val < cap_max_num) {
				stream[VIDEO_LIVE_0].cap_num = val;
				stream[VIDEO_LIVE_0_SUB].cap_num = val;
			} else {
				stream[VIDEO_LIVE_0].cap_num = cap_max_num;
				stream[VIDEO_LIVE_0_SUB].cap_num = cap_max_num;
			}

			stream[VIDEO_LIVE_0].cap_frame_num = g_shdr + 1;
			stream[VIDEO_LIVE_0].cap_stop = 0;
			stream[VIDEO_LIVE_0].cap_exit = 0;
			stream[VIDEO_LIVE_0_SUB].cap_frame_num = g_shdr + 1;
			stream[VIDEO_LIVE_0_SUB].cap_stop = 0;
			stream[VIDEO_LIVE_0_SUB].cap_exit = 0;
			for (i = 0; i < stream[VIDEO_LIVE_0].cap_frame_num; i++) {
				cap_idx[0][i] = 0;
				cap_idx[1][i] = 0;
			}

			ret = pthread_create(&stream[VIDEO_LIVE_0].cap_thread_id, NULL, cap_raw_thread, (void *)&stream[VIDEO_LIVE_0]);
			if (ret < 0) {
				printf("create cap_raw_thread fail \n");
				break;
			}

			while (stream[VIDEO_LIVE_0].cap_exit == 0) sleep(1);

			pthread_join(stream[VIDEO_LIVE_0].cap_thread_id, NULL);

			//free hdal malloc
			for (i = 0; i < MAX_DDR_NUM; i++) {
				if (ddr_max_free[i].pa != 0) {
					ret = hd_common_mem_free(ddr_max_free[i].pa, ddr_max_free[i].va);
					if (ret != HD_OK) {
						printf("DDR[%d] free pa = 0x%lx, va = 0x%lx fail \n", i, (UINTPTR)(ddr_max_free[i].pa), (UINTPTR)(ddr_max_free[i].va));
					}
					ddr_max_free[i].pa = 0;
					ddr_max_free[i].va = 0;
					ddr_max_free[i].size = 0;
				}
			}

			ret = hd_gfx_uninit();
			if(ret != HD_OK) {
				printf("uninit gfx fail (%d) \n", ret);
			}
		}
		#endif

	} while(1);

	if (sensor1_enabled()) {
		hdal_stop(&stream[VIDEO_LIVE_0], &stream[VIDEO_LIVE_0_SUB], out_type);
	}

	if (sensor2_enabled()) {
		hdal_stop(&stream[VIDEO_LIVE_1], &stream[VIDEO_LIVE_1_SUB], out_type);
	}

	if (sensor3_enabled()) {
		hdal_stop(&stream[VIDEO_LIVE_2], &stream[VIDEO_LIVE_2_SUB], out_type);
	}

	if (sensor4_enabled()) {
		hdal_stop(&stream[VIDEO_LIVE_3], &stream[VIDEO_LIVE_3_SUB], out_type);
	}


	#if AI_ENABLE
	if ((ret = input_uninit()) != HD_OK)
		return 0;
	if ((ret = network_uninit()) != HD_OK)
		return 0;
	#endif

	// unbind video_liveview modules (sub)
	///unbind EXTEND_PATH2 from network

exit:

	if (sensor1_enabled()) {
		hdal_close(&stream[VIDEO_LIVE_0], &stream[VIDEO_LIVE_0_SUB], out_type);
	}

	if (sensor2_enabled()) {
		hdal_close(&stream[VIDEO_LIVE_1], &stream[VIDEO_LIVE_1_SUB], out_type);
	}

	if (sensor3_enabled()) {
		hdal_close(&stream[VIDEO_LIVE_2], &stream[VIDEO_LIVE_2_SUB], out_type);
	}

	if (sensor4_enabled()) {
		hdal_close(&stream[VIDEO_LIVE_3], &stream[VIDEO_LIVE_3_SUB], out_type);
	}


	// uninit all modules
	ret = exit_module(out_type);
	if (ret != HD_OK) {
		printf("exit fail=%d\n", ret);
	}

	// uninit memory
	ret = hd_common_mem_uninit();
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
	}

	// uninit hdal
	ret = hd_common_uninit();
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
	}

	exit_share_memory();

	return ret;
}
