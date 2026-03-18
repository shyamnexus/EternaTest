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
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <kwrap/file.h>
#include "hdal.h"
#include "hd_debug.h"
#include "vendor_isp.h"
#include "vendor_common.h"
#include "vendor_ai.h"
#include "vendor_ai_cpu/vendor_ai_cpu.h"
#include "vendor_videoprocess.h"
#include "vendor_videoenc.h"
#include "aiisp.h"

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
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(ai_video_liveview_with_net, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define DEBUG_MENU 		1

//#define CHKPNT			printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
//#define DBGH(x)			printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
//#define DBGD(x)			printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)

#define ENC_FRAME_RATE      24

#define VIDEO_DDRID    DDR_ID0

typedef struct _DDR_INFO {
	void *va;
	UINTPTR pa;
	UINT32 size;
} DDR_INFO;

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

#define SEN_OUT_FMT		HD_VIDEO_PXLFMT_NRX12 //HD_VIDEO_PXLFMT_RAW12
#define CAP_OUT_FMT		HD_VIDEO_PXLFMT_NRX12 //HD_VIDEO_PXLFMT_RAW12
#define SHDR_CAP_OUT_FMT HD_VIDEO_PXLFMT_NRX12_SHDR2 //HD_VIDEO_PXLFMT_RAW12_SHDR2
//#define SEN_OUT_FMT		HD_VIDEO_PXLFMT_RAW12
//#define CAP_OUT_FMT		HD_VIDEO_PXLFMT_RAW12
//#define SHDR_CAP_OUT_FMT HD_VIDEO_PXLFMT_RAW12_SHDR2
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

	// NOTE: Do not modify SEN_SEL_MAX and SEN_SEL_PATGEN
	SEN_SEL_MAX,
	SEN_SEL_PATGEN                 = 99,
	ENUM_DUMMY4WORD(SEN_SEL)
} SEN_SEL;

#define VDO_SIZE_W_8M      3840
#define VDO_SIZE_H_8M      2160
#define VDO_SIZE_W_5M      2592
#define VDO_SIZE_H_5M      1944
#define VDO_SIZE_W_4M      2688
#define VDO_SIZE_H_4M      1520
#define VDO_SIZE_W_2M      1920
#define VDO_SIZE_H_2M      1080
#define VDO_SIZE_W_VGA     640
#define VDO_SIZE_H_VGA     480

///////////////////////////////////////////////////////////////////////////////
//sensor list
#define ISP_DEFAULT_CFG "isp_os02k10_0"

typedef struct _SENSOR_INFO_ {
	UINT32 sensor_num;
	USIZE size;
	CHAR sensor_name[32];
	CHAR isp_cfg_name[32];
	CHAR fpn_name_f[64];
	CHAR fpn_name_e[64];
} SENSOR_INFO;

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
};

#define RESOLUTION_SET  2
//  0: 2M (IMX290),        1: 5M (OS05A),   2: 2M (OS02K10),   3: 2M (AR0237IR),   4: 2M (PATTERN_GEN)
//  5: VGA (PATTERN_GEN),  6: 8M (IMX485),  7: 4M (OS04A10),   8: 8M (IMX415),     9: 8M (IMX585)
// 10: 8M (IMX334)
#if ( RESOLUTION_SET == 0)
#define VDO_SIZE_W      1920
#define VDO_SIZE_H      1080
#elif (RESOLUTION_SET == 1)
#define VDO_SIZE_W      2592
#define VDO_SIZE_H      1944
#elif ( RESOLUTION_SET == 2)
#define VDO_SIZE_W      1920
#define VDO_SIZE_H      1080
#elif ( RESOLUTION_SET == 3)
#define VDO_SIZE_W      1920
#define VDO_SIZE_H      1080
#elif ( RESOLUTION_SET == 4)
#define VDO_SIZE_W      1920
#define VDO_SIZE_H      1080
#elif ( RESOLUTION_SET == 5)
#define VDO_SIZE_W       640
#define VDO_SIZE_H       480
#elif ( RESOLUTION_SET == 6)
#define VDO_SIZE_W      3840
#define VDO_SIZE_H      2160
#elif ( RESOLUTION_SET == 7)
#define VDO_SIZE_W      2688
#define VDO_SIZE_H      1520
#elif ( RESOLUTION_SET == 8)
#define VDO_SIZE_W      3840
#define VDO_SIZE_H      2160
#elif ( RESOLUTION_SET == 9)
#define VDO_SIZE_W      3840
#define VDO_SIZE_H      2160
#elif ( RESOLUTION_SET == 10)
#define VDO_SIZE_W      3840
#define VDO_SIZE_H      2160
#endif

#define VIDEOCAP_ALG_FUNC HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB
#define VIDEOPROC_ALG_FUNC HD_VIDEOPROC_FUNC_AF | HD_VIDEOPROC_FUNC_WDR | HD_VIDEOPROC_FUNC_DEFOG | HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA

static UINT32 g_shdr = 0; //fixed

#define	VDO_FRAME_FORMAT	HD_VIDEO_PXLFMT_YUV420

#define SUB_VDO_SIZE_W	640 //max for videoout
#define SUB_VDO_SIZE_H	480 //max for videoout

#define SOURCE_PATH		HD_VIDEOPROC_0_OUT_0 //out 0~4 is physical path
#define EXTEND_PATH		HD_VIDEOPROC_0_OUT_5 //out 5~15 is extend path
#define EXTEND_PATH2	HD_VIDEOPROC_0_OUT_6 //out 5~15 is extend path

#define SOURCE_PATH2	HD_VIDEOPROC_1_OUT_0 //out 0~4 is physical path

#define DUMP_POSTPROC_INFO 0
#define DBG_OUT_DUMP                0  // debug mode, dump output iobuf

#define LABEL_LEN           256     ///< maximal length of class label
#define MAX_CLASS_NUMBER    1000
#define TOP_N                5

#define IPP_ID_0                0   //for ctrl_0 rawall pipe
#define IPP_ID_1                1   //for ctrl_1 scale pipe
#define ISP_ID_0                (1 << 31) | ((SEN1_VCAP_ID & 0x7F) << 24) | (1 << 15) | ((SEN1_VCAP_ID & 0x7F) << 8) | (IPP_ID_0 & 0xFF)
#define ISP_ID_1                (1 << 31) | ((SEN1_VCAP_ID & 0x7F) << 24) | (1 << 15) | ((SEN1_VCAP_ID & 0x7F) << 8) | (IPP_ID_1 & 0xFF)

#define SEN1_VCAP_ID 0
#define SEN2_VCAP_ID 2
///////////////////////////////////////////////////////////////////////////////

#define NET_PATH_ID		UINT32

#define VENDOR_AI_CFG  				0x000f0000  //vendor ai config

#define AI_RGB_BUFSIZE(w, h)		(ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(HD_VIDEO_PXLFMT_RGB888_PLANAR) / 8) * (h))

#define NET_VDO_SIZE_W	VDO_SIZE_W //max for net
#define NET_VDO_SIZE_H	VDO_SIZE_H //max for net
#define SUB2_VDO_SIZE_W	NET_VDO_SIZE_W
#define SUB2_VDO_SIZE_H	NET_VDO_SIZE_H

#define POOL_SIZE_USER_DEFINIED  0x1000000

static UINT32 sensor_sel = SEN_SEL_OS04A10;

///////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------*/
/* Type Definitions                                                            */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Global Functions                                                             */
/*-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------*/
/* Input Functions                                                             */
/*-----------------------------------------------------------------------------*/

///////////////////////////////////////////////////////////////////////////////

INT32 user_mem_config(HD_COMMON_MEM_INIT_CONFIG* p_mem_cfg, INT32 i)
{
	// config common pool (proc)
	p_mem_cfg->pool_info[i].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
	p_mem_cfg->pool_info[i].blk_size = POOL_SIZE_USER_DEFINIED;  // NOTE: dim need align to 16 for rotate buffer
	p_mem_cfg->pool_info[i].blk_cnt = 2;
	p_mem_cfg->pool_info[i].ddr_id = VIDEO_DDRID;
	i++;
	return i;
}

INT32 cap_mem_config(HD_PATH_ID video_cap_ctrl, HD_COMMON_MEM_INIT_CONFIG* p_mem_cfg, void* p_cfg, INT32 i)
{
	HD_DIM* p_dim = (HD_DIM*)p_cfg;
	// config common pool (cap)
	p_mem_cfg->pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	p_mem_cfg->pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_RAW_BUFSIZE(p_dim->w, p_dim->h, CAP_OUT_FMT)
													 +VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
													 +VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);

	p_mem_cfg->pool_info[i].blk_cnt = 12;

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

static HD_RESULT set_cap_cfg(HD_PATH_ID *p_video_cap_ctrl)
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
	} else if (sensor_sel == SEN_SEL_IMX290 || sensor_sel == SEN_SEL_OS05A10 || sensor_sel == SEN_SEL_OS02K10 || sensor_sel == SEN_SEL_IMX485 || sensor_sel == SEN_SEL_OS04A10 || sensor_sel == SEN_SEL_IMX415 || sensor_sel == SEN_SEL_IMX485 || sensor_sel == SEN_SEL_IMX334) {
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
		} else if (sensor_sel == SEN_SEL_IMX415) {
			printf("Using IMX415 or shdr\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
		}
	}

	if (sensor_sel == SEN_SEL_PATGEN_1080P || sensor_sel == SEN_SEL_PATGEN_VGA) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, HD_VIDEOCAP_SEN_PAT_GEN);
	}

	ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(SEN1_VCAP_ID), &video_cap_ctrl); //open this for device control
	if (ret != HD_OK) {
		return ret;
	}

	if (g_shdr == 1) {
		cap_cfg.sen_cfg.shdr_map = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR1, (HD_VIDEOCAP_0|HD_VIDEOCAP_1));
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

static HD_RESULT set_cap_cfg2(HD_PATH_ID *p_video_cap_ctrl)
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
	} else if (sensor_sel == SEN_SEL_IMX290 || sensor_sel == SEN_SEL_OS05A10 || sensor_sel == SEN_SEL_OS02K10 || sensor_sel == SEN_SEL_IMX485 || sensor_sel == SEN_SEL_OS04A10 || sensor_sel == SEN_SEL_IMX415 || sensor_sel == SEN_SEL_IMX485 || sensor_sel == SEN_SEL_IMX334) {
		cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux =  1;  //use @0 in peri-dev.dtsi
		printf("MIPI interface\n");
	}
	if (g_shdr == 1) {
		printf("Using g_shdr_mode\n");
	}

	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(2, 0);

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
		} else if (sensor_sel == SEN_SEL_IMX415) {
			printf("Using IMX415 or shdr\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
		}
	}

	if (sensor_sel == SEN_SEL_PATGEN_1080P || sensor_sel == SEN_SEL_PATGEN_VGA) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, HD_VIDEOCAP_SEN_PAT_GEN);
	}

	ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(SEN2_VCAP_ID), &video_cap_ctrl); //open this for device control
	if (ret != HD_OK) {
		return ret;
	}

	if (g_shdr == 1) {
		cap_cfg.sen_cfg.shdr_map = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR1, (HD_VIDEOCAP_0|HD_VIDEOCAP_1));
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
			video_in_param.frc = HD_VIDEO_FRC_RATIO(30,1);
			video_in_param.dim.w = p_dim->w;
			video_in_param.dim.h = p_dim->h;
		} else {
			video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO; //auto select sensor mode by the parameter of HD_VIDEOCAP_PARAM_OUT
			video_in_param.frc = HD_VIDEO_FRC_RATIO(20,1);
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
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT, &video_out_param);
		//printf("set_cap_param OUT=%d\r\n", ret);
	}

	return ret;
}
///////////////////////////////////////////////////////////////////////////////

INT32 proc_mem_config(HD_PATH_ID video_proc_path, HD_COMMON_MEM_INIT_CONFIG* p_mem_cfg, void* p_cfg, INT32 i)
{
	HD_DIM* p_dim = (HD_DIM*)p_cfg;
	// config common pool (proc)
	p_mem_cfg->pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	p_mem_cfg->pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(p_dim->w, p_dim->h, HD_VIDEO_PXLFMT_YUV420);  // NOTE: dim need align to 16 for rotate buffer
	p_mem_cfg->pool_info[i].blk_cnt = 20;
	p_mem_cfg->pool_info[i].ddr_id = VIDEO_DDRID;
	i++;
	return i;
}

INT32 out_mem_config(HD_PATH_ID video_proc_path, HD_COMMON_MEM_INIT_CONFIG* p_mem_cfg, void* p_cfg, INT32 i)
{
	HD_DIM* p_dim = (HD_DIM*)p_cfg;
	// config common pool (proc)
	p_mem_cfg->pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	p_mem_cfg->pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(p_dim->w, p_dim->h, HD_VIDEO_PXLFMT_YUV420);  // NOTE: dim need align to 16 for rotate buffer
	p_mem_cfg->pool_info[i].blk_cnt = 15;
	p_mem_cfg->pool_info[i].ddr_id = VIDEO_DDRID;
	i++;
	return i;
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
		video_cfg_param.isp_id = SEN1_VCAP_ID;
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

	#if (IME_POSTSHARPEN_ENABLE)
	video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_0_OUT_4;
	#else
	video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_0_OUT_0;
	#endif

	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);

	*p_video_proc_ctrl = video_proc_ctrl;

	return ret;
}

static HD_RESULT set_proc_cfg2(HD_PATH_ID *p_video_proc_ctrl, HD_DIM* p_max_dim)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
	HD_VIDEOPROC_CTRL video_ctrl_param = {0};
	HD_PATH_ID video_proc_ctrl = 0;

	ret = hd_videoproc_open(0, HD_VIDEOPROC_1_CTRL, &video_proc_ctrl); //open this for device control
	if (ret != HD_OK)
		return ret;

	if (p_max_dim != NULL ) {
		video_cfg_param.pipe = HD_VIDEOPROC_PIPE_RAWALL;
		video_cfg_param.isp_id = SEN2_VCAP_ID;
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

	#if (IME_POSTSHARPEN_ENABLE)
	video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_1_OUT_4;
	#else
	video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_1_OUT_0;
	#endif

	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);

	*p_video_proc_ctrl = video_proc_ctrl;

	return ret;
}


static HD_RESULT set_proc_param(HD_PATH_ID video_proc_path, HD_DIM* p_dim, UINT32 dir, UINT32 depth)
{
	HD_RESULT ret = HD_OK;

	if (p_dim != NULL) { //if videoproc is already binding to dest module, not require to setting this!
		HD_VIDEOPROC_OUT video_out_param = {0};
		video_out_param.func = 0;
		video_out_param.dim.w = p_dim->w;
		video_out_param.dim.h = p_dim->h;
		video_out_param.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
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

#if 0
static HD_RESULT set_proc_param_extend(HD_PATH_ID video_proc_path, HD_PATH_ID src_path, HD_URECT* p_crop, HD_DIM* p_dim, UINT32 dir, UINT32 depth)
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
		video_out_param.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
		video_out_param.dir = dir;
		video_out_param.depth = depth; //set 1 to allow pull
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT_EX, &video_out_param);
	} else {
		HD_VIDEOPROC_OUT_EX video_out_param = {0};
		video_out_param.src_path = src_path;
		video_out_param.dim.w = 0; //auto reference to downstream's in dim.w
		video_out_param.dim.h = 0; //auto reference to downstream's in dim.h
		video_out_param.pxlfmt = 0; //auto reference to downstream's in pxlfmt
		video_out_param.dir = dir;
		video_out_param.depth = depth; //set 1 to allow pull
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT_EX, &video_out_param);
	}

	return ret;
}
#endif
///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------*/
/* Network Functions                                                             */
/*-----------------------------------------------------------------------------*/

static VIDEO_LIVEVIEW stream[2] = {0}; //0: main stream //1: sub stream (vout)

///////////////////////////////////////////////////////////////////////////////

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

	#if AI_ENABLE
	if ((ret = input_init()) != HD_OK)
		return ret;
	if ((ret = network_init()) != HD_OK)
		return ret;
	#endif
	return HD_OK;
}

static HD_RESULT open_module(VIDEO_LIVEVIEW *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;

	// set videocap config
	if (stream[0].fix_pattern == FALSE) {
		ret = set_cap_cfg(&p_stream->cap_ctrl);
		if (ret != HD_OK) {
			printf("set cap-cfg fail=%d\n", ret);
			return HD_ERR_NG;
		}
	}
	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim);
	if (ret != HD_OK) {
		printf("set proc-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}

	if (stream[0].fix_pattern == FALSE) {
		if ((ret = hd_videocap_open(HD_VIDEOCAP_IN(SEN1_VCAP_ID, 0), HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), &p_stream->cap_path)) != HD_OK)
			return ret;
	}
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, SOURCE_PATH, &p_stream->proc_path)) != HD_OK)
		return ret;

	#if (IME_POSTSHARPEN_ENABLE)
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_4, &p_stream->proc_path_3dnr)) != HD_OK)
		return ret;
	#endif

	#if AI_ENABLE

	if ((ret = network_bind_isp_cb(p_stream)) != HD_OK)
		return ret;

	if ((ret = network_open(p_stream->net_path)) != HD_OK)
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
	if ((ret = hd_videoout_open(HD_VIDEOOUT_0_IN_0, HD_VIDEOOUT_0_OUT_0,  &p_stream->out_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT open_module3(VIDEO_LIVEVIEW *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;
	// set videocap config
	if (stream[0].fix_pattern == FALSE) {
		ret = set_cap_cfg2(&p_stream->cap_ctrl);
		if (ret != HD_OK) {
			printf("set cap-cfg fail=%d\n", ret);
			return HD_ERR_NG;
		}
	}

	// set videoproc config
	ret = set_proc_cfg2(&p_stream->proc_ctrl, p_proc_max_dim);
	if (ret != HD_OK) {
		printf("set proc-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}

	if (stream[0].fix_pattern == FALSE) {
		if ((ret = hd_videocap_open(HD_VIDEOCAP_IN(SEN2_VCAP_ID, 0), HD_VIDEOCAP_OUT(SEN2_VCAP_ID, 0), &p_stream->cap_path)) != HD_OK)
			return ret;
	}
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_1_IN_0, HD_VIDEOPROC_1_OUT_0, &p_stream->proc_path)) != HD_OK)
		return ret;

	#if (IME_POSTSHARPEN_ENABLE)
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_1_IN_0, HD_VIDEOPROC_1_OUT_4, &p_stream->proc_path_3dnr)) != HD_OK)
		return ret;
	#endif

	#if AI_ENABLE

	/*if ((ret = network_bind_isp_cb(p_stream)) != HD_OK)
		return ret;*/

	if ((ret = network_open(p_stream->net_path)) != HD_OK)
		return ret;
	#endif
	return HD_OK;
}


static HD_RESULT close_module(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret;
	if (stream[0].fix_pattern == FALSE) {
		if ((ret = hd_videocap_close(p_stream->cap_path)) != HD_OK)
			return ret;
	}
	if ((ret = hd_videoproc_close(p_stream->proc_path)) != HD_OK)
		return ret;

	#if (IME_POSTSHARPEN_ENABLE)
	if ((ret = hd_videoproc_close(p_stream->proc_path_3dnr)) != HD_OK)
		return ret;
	#endif

	#if AI_ENABLE
	if ((ret = network_close(p_stream->net_path)) != HD_OK)
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

	#if AI_ENABLE
	if ((ret = input_uninit()) != HD_OK)
		return ret;
	if ((ret = network_uninit()) != HD_OK)
		return ret;
	#endif
	return HD_OK;
}

uintptr_t get_post_buf(uint32_t size)
{
	uintptr_t buf = (uintptr_t)malloc(size);

	return buf;

}

///////////////////////////////////////////////////////////////////////////////

VOID *network_proc_thread(VOID *arg)
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

			if (p_stream->out_path && p_stream->out_type <= 2 && p_stream->vprc_output == VPRC_OUTPUT_VOUT) {
				ret = hd_videoout_push_in_buf(p_stream->out_path, &video_frame, NULL, -1); // blocking mode
				if (ret != HD_OK && ret != HD_ERR_OVERRUN) {
					printf("out_push error=%d !!\r\n\r\n", ret);
					goto skip;
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

	#if AI_ENABLE
	//net proc
	NET_PROC_CONFIG net_cfg = {
		.model_filename = "/mnt/sd/para/nvt_model.bin",
		.label_filename = "/mnt/sd/accuracy/labels.txt"
	};
	#endif

	// parse out config
	if (argc == 2) {
		out_type = atoi(argv[1]);
		printf("out_type %d\r\n", out_type);
		if(out_type > 2) {
			if (out_type != 0xFF) {
				printf("error: not support out_type!\r\n");
				return 0;
			}
		}
	} else if (argc == 3) {
		out_type = atoi(argv[1]);
		stream[0].fix_pattern = atoi(argv[2]);

		printf("out_type %d, fix_pattern %d\r\n", out_type, stream[0].fix_pattern);
		if(out_type > 2) {
			if (out_type != 0xFF) {
				printf("error: not support out_type!\r\n");
				return 0;
			}
		}
	} else if (argc == 4) {
		out_type = atoi(argv[1]);
		stream[0].fix_pattern = atoi(argv[2]);
		sensor_sel = atoi(argv[3]);

		printf("out_type %d, fix_pattern %d, sensor_sel %d\r\n", out_type, stream[0].fix_pattern, sensor_sel);
		if(out_type > 2) {
			if (out_type != 0xFF) {
				printf("error: not support out_type!\r\n");
				return 0;
			}
		}
	} else if (argc == 5) {
		out_type = atoi(argv[1]);
		stream[0].fix_pattern = atoi(argv[2]);
		sensor_sel = atoi(argv[3]);
		stream[0].vprc_output = atoi(argv[4]);

		printf("out_type %d, fix_pattern %d, sensor_sel %d, vprc_output %d\r\n", out_type, stream[0].fix_pattern, sensor_sel, stream[0].vprc_output);
		if(out_type > 2) {
			if (out_type != 0xFF) {
				printf("error: not support out_type!\r\n");
				return 0;
			}
		}
	} else if (argc == 6) {

		out_type = atoi(argv[1]);
		stream[0].fix_pattern = atoi(argv[2]);
		sensor_sel = atoi(argv[3]);
		stream[0].vprc_output = atoi(argv[4]);
		g_shdr = atoi(argv[5]);

		printf("out_type %d, fix_pattern %d, sensor_sel %d, vprc_output %d, shdr %d\r\n", out_type, stream[0].fix_pattern, sensor_sel, stream[0].vprc_output, g_shdr);
		if(out_type > 2) {
			if (out_type != 0xFF) {
				printf("error: not support out_type!\r\n");
				return 0;
			}
		}
	} else if (argc == 7) {
		out_type = atoi(argv[1]);
		stream[0].fix_pattern = atoi(argv[2]);
		sensor_sel = atoi(argv[3]);
		stream[0].vprc_output = atoi(argv[4]);
		g_shdr = atoi(argv[5]);

		#if AI_ENABLE
		sprintf(net_cfg.model_filename, "%s", argv[6]);

		printf("out_type %d, fix_pattern %d, sensor_sel %d, vprc_output %d, model %s\r\n", out_type, stream[0].fix_pattern, sensor_sel, stream[0].vprc_output, net_cfg.model_filename);
		#else
		printf("out_type %d, fix_pattern %d, sensor_sel %d, vprc_output %d\r\n", out_type, stream[0].fix_pattern, sensor_sel, stream[0].vprc_output);
		#endif
		if(out_type > 2) {
			if (out_type != 0xFF) {
				printf("error: not support out_type!\r\n");
				return 0;
			}
		}
	}

	cap_size_w = sensor_info[sensor_sel].size.w;
	cap_size_h = sensor_info[sensor_sel].size.h;

	if (stream[0].fix_pattern == 1) {
		stream[0].dump_yuv = 1;
	}

	printf("\r\n\r\n");

	stream[0].net_path = 0;
	stream[0].in_path = 0;

	stream[0].out_type = out_type;

	memcpy((void*)&stream[1], (void*)&stream[0], sizeof(VIDEO_LIVEVIEW));

	stream[1].net_path = 1;

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

		// config common pool
		stream[0].cap_dim.w = cap_size_w; //assign by user
		stream[0].cap_dim.h = cap_size_h; //assign by user
		i = cap_mem_config(stream[0].proc_path, &mem_cfg, &stream[0].cap_dim, i);

		stream[0].proc_max_dim.w = cap_size_w; //assign by user
		stream[0].proc_max_dim.h = cap_size_h; //assign by user
		i = proc_mem_config(stream[0].proc_path, &mem_cfg, &stream[0].proc_max_dim, i);

		stream[1].cap_dim.w = cap_size_w; //assign by user
		stream[1].cap_dim.h = cap_size_h; //assign by user
		i = cap_mem_config(stream[1].proc_path, &mem_cfg, &stream[1].cap_dim, i);

		stream[1].proc_max_dim.w = cap_size_w; //assign by user
		stream[1].proc_max_dim.h = cap_size_h; //assign by user
		i = proc_mem_config(stream[1].proc_path, &mem_cfg, &stream[1].proc_max_dim, i);


		i = out_mem_config(stream[0].out_path, &mem_cfg, &stream[0].proc_max_dim, i);

		i = user_mem_config(&mem_cfg, i);

		#if AI_ENABLE
		network_mem_config(stream[0].net_path, &mem_cfg, &net_cfg);

		network_mem_config(stream[1].net_path, &mem_cfg, &net_cfg);
		#endif
		ret = hd_common_mem_init(&mem_cfg);
	}
	if (HD_OK != ret) {
		printf("hd_common_mem_init err: %d\r\n", ret);
		goto exit;
	}

	if (vendor_isp_init() == HD_ERR_NG) {
		printf("vendor_isp_init fail\r\n");
		goto exit;
	}

	// reload ISP cfg
	{
	AET_CFG_INFO cfg_info = {0};

	cfg_info.id = IPP_ID_0;
	snprintf(cfg_info.path, 64, "/mnt/app/isp/%s_AI.cfg", sensor_info[sensor_sel].isp_cfg_name);
	printf("proc 0 load %s \n", cfg_info.path);

	vendor_isp_set_ae(AET_ITEM_RLD_CONFIG, &cfg_info);
	vendor_isp_set_awb(AWBT_ITEM_RLD_CONFIG, &cfg_info);
	vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);

	cfg_info.id = IPP_ID_1;
	snprintf(cfg_info.path, 64, "/mnt/app/isp/%s.cfg", sensor_info[sensor_sel].isp_cfg_name);
	printf("proc 1 load %s \n", cfg_info.path);
	vendor_isp_set_ae(AET_ITEM_RLD_CONFIG, &cfg_info);
	vendor_isp_set_awb(AWBT_ITEM_RLD_CONFIG, &cfg_info);
	vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);
	}

	// init all modules
	ret = init_module(out_type);
	if (ret != HD_OK) {
		printf("init fail=%d\n", ret);
		goto exit;
	}

	printf("1\r\n");

	// open video_liveview modules (main)
	ret = open_module(&stream[0], &stream[0].proc_max_dim);
	if (ret != HD_OK) {
		printf("open fail=%d\n", ret);
		goto exit;
	}


	printf("2\r\n");

	ret = open_module3(&stream[1], &stream[1].proc_max_dim);
	if (ret != HD_OK) {
		printf("open3 fail=%d\n", ret);
		goto exit;
	}

	printf("3\r\n");

	// open video_liveview modules (sub)
	if (out_type <= 2 && stream[0].vprc_output == VPRC_OUTPUT_VOUT) {
		ret = open_module_2(&stream[0], out_type);
		if (ret != HD_OK) {
			printf("open2 fail=%d\n", ret);
			goto exit;
		}
	}

	if (stream[0].fix_pattern == FALSE) {
		// get videocap capability
		ret = get_cap_caps(stream[0].cap_ctrl, &stream[0].cap_syscaps);
		if (ret != HD_OK) {
			printf("get cap-caps fail=%d\n", ret);
			goto exit;
		}
	}

	// get videoout capability
	if (out_type <= 2 && stream[0].vprc_output == VPRC_OUTPUT_VOUT) {
		ret = get_out_caps(stream[0].out_ctrl, &stream[0].out_syscaps);
		if (ret != HD_OK) {
			printf("get out-caps fail=%d\n", ret);
			goto exit;
		}
		stream[0].out_max_dim = stream[0].out_syscaps.output_dim;
	}

	if (stream[0].fix_pattern == FALSE) {
		UINT32 depth;

		depth = 0;

		// set videocap parameter
		ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim, depth);
		if (ret != HD_OK) {
			printf("set cap fail=%d\n", ret);
			goto exit;
		}

		// set videocap parameter
		ret = set_cap_param(stream[1].cap_path, &stream[1].cap_dim, depth);
		if (ret != HD_OK) {
			printf("set cap fail=%d\n", ret);
			goto exit;
		}
	}

	// assign parameter by program options
	#if 0
	main_dim.w = cap_size_w/2;
	main_dim.h = cap_size_h/2;
	#else
	main_dim.w = cap_size_w;
	main_dim.h = cap_size_h;
	#endif

	if (out_type <= 2 && stream[0].vprc_output == VPRC_OUTPUT_VOUT) {
		out_dim.w = main_dim.w; //display device size
		out_dim.h = main_dim.h; //display device size
		sub_dim.w = out_dim.w;
		sub_dim.h = out_dim.h;
	}

	// set videoproc parameter (main)
	ret = set_proc_param(stream[0].proc_path, &main_dim, HD_VIDEO_DIR_NONE, 1);
	if (ret != HD_OK) {
		printf("set proc fail=%d\n", ret);
		goto exit;
	}

	#if (IME_POSTSHARPEN_ENABLE)
	ret = set_proc_param(stream[0].proc_path_3dnr, &main_dim, HD_VIDEO_DIR_NONE, 1);
	if (ret != HD_OK) {
		printf("set proc 3dnr fail = %d \n", ret);
		goto exit;
	}
	#endif

	// set videoproc parameter (main second)
	ret = set_proc_param(stream[1].proc_path, &main_dim, HD_VIDEO_DIR_NONE, 1);
	if (ret != HD_OK) {
		printf("set proc fail=%d\n", ret);
		goto exit;
	}

	#if (IME_POSTSHARPEN_ENABLE)
	ret = set_proc_param(stream[1].proc_path_3dnr, &main_dim, HD_VIDEO_DIR_NONE, 1);
	if (ret != HD_OK) {
		printf("set proc 3dnr fail = %d \n", ret);
		goto exit;
	}
	#endif

	// set videoout parameter (sub)
	if (out_type <= 2 && stream[0].vprc_output == VPRC_OUTPUT_VOUT) {
		stream[0].out_dim.w = sub_dim.w;
		stream[0].out_dim.h = sub_dim.h;
		ret = set_out_param(stream[0].out_path, &stream[0].out_dim, NULL);
		if (ret != HD_OK) {
			printf("set out fail=%d\n", ret);
			goto exit;
		}
	}


	if (stream[0].fix_pattern == FALSE) {
		// bind video_liveview modules (main)
		hd_videocap_bind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);
		//hd_videoproc_bind(SOURCE_PATH, ...); //src_path is not used

		// bind video_liveview modules (main second)
		hd_videocap_bind(HD_VIDEOCAP_OUT(SEN2_VCAP_ID, 0), HD_VIDEOPROC_1_IN_0);
	}

	// bind video_liveview modules (sub2)
	//.... bind EXTEND_PATH2 to network

	stream[0].net_proc_start = 0;
	stream[0].net_proc_exit = 0;
	stream[0].net_proc_oneshot = 0;

	stream[1].net_proc_start = 0;
	stream[1].net_proc_exit = 0;
	stream[1].net_proc_oneshot = 0;

	//start network
	network_start(&stream[0]);
	network_start(&stream[1]);

	ret = pthread_create(&stream[0].net_proc_thread_id, NULL, network_proc_thread, (VOID*)(&stream[0]));
	if (ret < 0) {
		return HD_ERR_FAIL;
	}

	ret = pthread_create(&stream[1].net_proc_thread_id, NULL, network_proc_thread, (VOID*)(&stream[1]));
	if (ret < 0) {
		return HD_ERR_FAIL;
	}

	// start video_liveview modules (main)
	if (stream[0].fix_pattern == FALSE) {
		hd_videocap_start(stream[0].cap_path);
	}

	// start video_liveview modules (main second)
	if (stream[1].fix_pattern == FALSE) {
		hd_videocap_start(stream[1].cap_path);
	}
	hd_videoproc_start(stream[0].proc_path);
	#if (IME_POSTSHARPEN_ENABLE)
	hd_videoproc_start(stream[0].proc_path_3dnr);
	#endif

	hd_videoproc_start(stream[1].proc_path);
	#if (IME_POSTSHARPEN_ENABLE)
	hd_videoproc_start(stream[1].proc_path_3dnr);
	#endif

	#if AI_ENABLE
	if ((ret = network_bind_cb(&stream[0])) != HD_OK) {

		stream[0].net_proc_exit = 1;
		stream[1].net_proc_exit = 1;
		// quit program
		goto exit;
	}
	ret = network_set_isp_ai_cfg(&stream[0], 0, TRUE);
	if (ret != HD_OK) {

		stream[0].net_proc_exit = 1;
		stream[1].net_proc_exit = 1;
		// quit program
		printf("set isp_ai fail=%d\n", ret);
		//goto exit;
	}

	if ((ret = network_bind_cb(&stream[1])) != HD_OK) {

		stream[0].net_proc_exit = 1;
		stream[1].net_proc_exit = 1;
		// quit program
		//goto exit;
	}
	ret = network_set_isp_ai_cfg(&stream[1], 0, TRUE);
	if (ret != HD_OK) {

		stream[0].net_proc_exit = 1;
		stream[1].net_proc_exit = 1;
		// quit program
		printf("set isp_ai 2 fail=%d\n", ret);
		//goto exit;
	}
	#endif

	if (stream[0].fix_pattern) {
		load_input_raw(&stream[0]);
	}

	if (stream[1].fix_pattern) {
		load_input_raw(&stream[1]);
	}

	// start video_liveview modules (sub)
	if (out_type <= 2 && stream[0].vprc_output == VPRC_OUTPUT_VOUT) {
		hd_videoout_start(stream[0].out_path);
	}

	stream[0].net_proc_start = 1;
	stream[0].net_proc_exit = 0;
	stream[0].net_proc_oneshot = 0;

	stream[1].net_proc_start = 1;
	stream[1].net_proc_exit = 0;
	stream[1].net_proc_oneshot = 0;

	do {
		key = getchar();
		if (key == 'q' || key == 0x3) {
			stream[0].net_proc_exit = 1;
			stream[1].net_proc_exit = 1;
			// quit program
			usleep(300000);
			break;
		}

		if (key == 'd') {
			hd_debug_run_menu(); // call debug menu
			printf("\r\nEnter q to exit, Enter d to debug\r\n");
		}

		if (key == 'a') {
			system("cat /proc/kdrv_ipp/utilization");
		}

		if (key == 'z') {
			system("echo dumpt > /proc/kflow_ipp/cmd");
		}

		if (key == 's') {
			if (stream[0].fix_pattern) {
				push_input_raw(&stream[0]);
				printf("push fix pattern to vprc\r\n");
			}
		}

		#if AI_ENABLE
		if (key == '1') {
			ret = network_set_buf_by_in_path_list(&stream[0], 0);
			if (HD_OK != ret) {
				printf("net_path(%u), para_id(%u) network_set_buf_by_in_path_list fail !!\n", stream[0].net_path, 0);
			}
			printf("set AI default parameter\r\n");
		}

		if (key == '2') {
			ret = network_set_buf_by_in_path_list(&stream[0], 1);
			if (HD_OK != ret) {
				printf("net_path(%u), para_id(%u) network_set_buf_by_in_path_list fail !!\n", stream[0].net_path, 0);
			}
			printf("set AI disable 3D\r\n");

		}

		if (key == '3') {
			ret = network_set_buf_by_in_path_list(&stream[0], 2);
			if (HD_OK != ret) {
				printf("net_path(%u), para_id(%u) network_set_buf_by_in_path_list fail !!\n", stream[0].net_path, 0);
			}
			printf("set AI small noise profile\r\n");

		}
		#endif

		if (key == 'o') {
			#if AI_ENABLE
			ret = network_set_isp_ai_cfg(&stream[0], 0, TRUE);
			if (ret != HD_OK) {
				printf("set isp_ai fail=%d\n", ret);
				return HD_ERR_NG;
			}
			printf("enable isp ai cfg\r\n");
			#else
			printf("AI is disabled\r\n");
			#endif
		}

		if (key == 'p') {
			#if AI_ENABLE
			ret = network_set_isp_ai_cfg(&stream[0], 0, FALSE);
			if (ret != HD_OK) {
				printf("set isp_ai fail=%d\n", ret);
				return HD_ERR_NG;
			}
			printf("disable isp ai cfg\r\n");
			#else
			printf("AI is disabled\r\n");
			#endif
		}
	} while(1);

	// stop video_liveview modules (main)
	if (stream[0].fix_pattern == FALSE) {
		hd_videocap_stop(stream[0].cap_path);
	}

	if (stream[1].fix_pattern == FALSE) {
		hd_videocap_stop(stream[1].cap_path);
	}
	hd_videoproc_stop(stream[0].proc_path);

	#if (IME_POSTSHARPEN_ENABLE)
	hd_videoproc_stop(stream[0].proc_path_3dnr);
	#endif

	hd_videoproc_stop(stream[1].proc_path);

	#if (IME_POSTSHARPEN_ENABLE)
	hd_videoproc_stop(stream[1].proc_path_3dnr);
	#endif

	// stop video_liveview modules (sub)
	if (out_type <= 2 && stream[0].vprc_output == VPRC_OUTPUT_VOUT) {
		hd_videoout_stop(stream[0].out_path);
	}


	if (stream[0].net_cap_thread_id) {
		pthread_join(stream[0].net_cap_thread_id, NULL);
	}

	if (stream[0].net_proc_thread_id) {
		pthread_join(stream[0].net_proc_thread_id, NULL);
	}

	if (stream[0].net_proc_sub_thread_id) {
		pthread_join(stream[0].net_proc_sub_thread_id, NULL);
	}

	if (stream[1].net_cap_thread_id) {
		pthread_join(stream[1].net_cap_thread_id, NULL);
	}

	if (stream[1].net_proc_thread_id) {
		pthread_join(stream[1].net_proc_thread_id, NULL);
	}

	if (stream[1].net_proc_sub_thread_id) {
		pthread_join(stream[1].net_proc_sub_thread_id, NULL);
	}

	network_stop(&stream[0]);

	network_stop(&stream[1]);

	// destroy aquire_thread
	//pthread_join(stream[0].aquire_thread_id, (void* )NULL);

	if (stream[0].fix_pattern == FALSE) {
		// unbind video_liveview modules (main)
		hd_videocap_unbind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0));
		//hd_videoproc_unbind(SOURCE_PATH); //src_path is not used
	}

	if (stream[1].fix_pattern == FALSE) {
		// unbind video_liveview modules (main)
		hd_videocap_unbind(HD_VIDEOCAP_OUT(SEN2_VCAP_ID, 0));
		//hd_videoproc_unbind(SOURCE_PATH); //src_path is not used
	}

	// unbind video_liveview modules (sub)
	///unbind EXTEND_PATH2 from network

exit:
	// close video_liveview modules (main)
	ret = close_module(&stream[0]);
	if (ret != HD_OK) {
		printf("close fail=%d\n", ret);
	}

	// close video_liveview modules (main second)
	ret = close_module(&stream[1]);
	if (ret != HD_OK) {
		printf("close fail=%d\n", ret);
	}

	// close video_liveview modules (sub)
	if (out_type <= 2 && stream[0].vprc_output == VPRC_OUTPUT_VOUT) {
		ret = close_module_2(&stream[0]);
		if (ret != HD_OK) {
			printf("close2 fail=%d\n", ret);
		}
	}

	// uninit all modules
	ret = exit_module(out_type);
	if (ret != HD_OK) {
		printf("exit fail=%d\n", ret);
	}

	if (stream[0].fix_pattern) {
		release_input_raw(&stream[0]);
	}

	if (stream[1].fix_pattern) {
		release_input_raw(&stream[1]);
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

	return ret;
}
