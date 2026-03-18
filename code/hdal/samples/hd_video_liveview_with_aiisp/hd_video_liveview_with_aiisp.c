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
#include "vendor_videocapture.h"
#include "vendor_gfx.h"
#include "vendor_common.h"
#include "vendor_vpe.h"
#include <pd_shm.h>

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

#define DEBUG_MENU 		1

//#define CHKPNT			printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
//#define DBGH(x)			printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
//#define DBGD(x)			printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)

#define SEN1_VCAP_ID 0

#define IME_POSTSHARPEN_ENABLE     1

#define AI_ENABLE        1
#define ISP_CALLBACK 1

static UINT32 ai_isp_flow = 0;

#define ENC_FRAME_RATE      24

#define AI_DDRID       DDR_ID0
#define VIDEO_DDRID    DDR_ID0

#define FPGA_TEST 0

typedef struct _DDR_INFO {
	void *va;
	UINTPTR pa;
	UINT32 size;
} DDR_INFO;


typedef struct _AIISP_PQ_FINAL_PARAM {
	unsigned int nr_param[32];
} AIISP_PQ_FINAL_PARAM;

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

//AIISP
#define VDO_AIDED_BUFSIZE(w, h, pxlfmt) (ALIGN_CEIL_4((w) / 4) * ALIGN_CEIL_4((h) / 4))

///////////////////////////////////////////////////////////////////////////////

//#define SEN_OUT_FMT		HD_VIDEO_PXLFMT_NRX12 //HD_VIDEO_PXLFMT_RAW12
//#define CAP_OUT_FMT		HD_VIDEO_PXLFMT_NRX12 //HD_VIDEO_PXLFMT_RAW12
//#define SHDR_CAP_OUT_FMT HD_VIDEO_PXLFMT_NRX12_SHDR2 //HD_VIDEO_PXLFMT_RAW12_SHDR2
//#define VPRC_OUT_FMT    HD_VIDEO_PXLFMT_YUV420_NVX2
#define SEN_OUT_FMT		HD_VIDEO_PXLFMT_RAW12
#define CAP_OUT_FMT		HD_VIDEO_PXLFMT_RAW12
#define SHDR_CAP_OUT_FMT HD_VIDEO_PXLFMT_RAW12_SHDR2
#define VPRC_OUT_FMT    HD_VIDEO_PXLFMT_YUV420 //HD_VIDEO_PXLFMT_YUV420_NVX2

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

#define VCAP_PATTERN_GEN 0

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
	SEN_SEL_SC850SL      = 13,   //8M
	SEN_SEL_OS08C10      = 14,   //8M

	// NOTE: Do not modify SEN_SEL_MAX and SEN_SEL_PATGEN
	SEN_SEL_MAX,
	SEN_SEL_PATGEN                 = 99,
	ENUM_DUMMY4WORD(SEN_SEL)
} SEN_SEL;

#define VDO_SIZE_W_8M      3840
#define VDO_SIZE_H_8M      2160
#define VDO_SIZE_W_5M      2592
#define VDO_SIZE_H_5M      1944
#define VDO_SIZE_W_5M_2    2688
#define VDO_SIZE_H_5M_2    2048
#define VDO_SIZE_W_4M      2688
#define VDO_SIZE_H_4M      1520
#define VDO_SIZE_W_3M      2048
#define VDO_SIZE_H_3M      1536
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
	/*  8 */{SEN_SEL_IMX415,        {VDO_SIZE_W_8M,  VDO_SIZE_H_8M},    "nvt_sen_imx415",          ISP_DEFAULT_CFG        },
	/*  9 */{SEN_SEL_IMX585,        {VDO_SIZE_W_8M,  VDO_SIZE_H_8M},    "nvt_sen_imx585",          "isp_imx585_0"         },
	/* 10 */{SEN_SEL_IMX334,        {VDO_SIZE_W_8M,  VDO_SIZE_H_8M},    "nvt_sen_imx334",          "isp_imx334_0"         },
	/* 11 */{SEN_SEL_PATGEN,        {VDO_SIZE_W_3M,  VDO_SIZE_H_3M},    "PATTERN_GEN",             ISP_DEFAULT_CFG        },
	/* 12 */{SEN_SEL_PATGEN,        {VDO_SIZE_W_5M_2,  VDO_SIZE_H_5M_2},"PATTERN_GEN",             ISP_DEFAULT_CFG        },
	/* 13 */{SEN_SEL_SC850SL,       {VDO_SIZE_W_8M,  VDO_SIZE_H_8M},    "nvt_sen_sc850sl",         "isp_sc850sl_0"         },
	/* 14 */{SEN_SEL_OS08C10,       {VDO_SIZE_W_8M,  VDO_SIZE_H_8M},    "nvt_sen_os08c10",         "isp_sc450ai_0"         },
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

static UINT32 SOURCE_PATH = HD_VIDEOPROC_0_OUT_0; //out 0~4 is physical path
static UINT32 EXTEND_PATH = HD_VIDEOPROC_0_OUT_5; //out 5~15 is extend path
static UINT32 EXTEND_PATH2 = HD_VIDEOPROC_0_OUT_6; //out 5~15 is extend path

#if (IME_POSTSHARPEN_ENABLE)
static UINT32 REF_PATH = HD_VIDEOPROC_0_OUT_4; //out 0~4 is physical path
#else
static UINT32 REF_PATH = HD_VIDEOPROC_0_OUT_0; //out 0~4 is physical path
#endif

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
static int    g_quit = 0;
static int    g_fps = 30;

#if AI_ENABLE
static AIISP_PQ_FINAL_PARAM aiisp_pq_final_param = {0,0,0,0,128,0,255};
static AIISP_PQ_FINAL_PARAM aiisp_pq_final_param_539A = {127, 127};
#endif

char *chip_name;

static UINT32 g_capbind = 0;  //0:D2D, 1:direct, 2: one-buf, 0xff: no-bind
///////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------*/
/* Type Definitions                                                            */
/*-----------------------------------------------------------------------------*/

typedef struct _MEM_PARM {
	UINTPTR pa;
	UINTPTR va;
	UINT32 size;
	UINT32 blk;
} MEM_PARM;

typedef enum {
	VPRC_OUTPUT_VOUT,
	VPRC_OUTPUT_VENC,
	VPRC_OUTPUT_VENC_1OUT,
	VPRC_OUTPUT_MAX,
	ENUM_DUMMY4WORD(VPRC_OUTPUT)
} VPRC_OUTPUT;

typedef struct _COPY_INFO {
	UINT32 copy_size;
	UINT32 ddr_id;
	UINT32 sleep_us;
	UINT32 fps;
} COPY_INFO;

typedef struct _ENGINE_CFG {
	UINT32 memcpy1_fps;
	UINT32 memcpy2_fps;
	UINT32 dmacpy1_fps;
	UINT32 grph_fps;
	UINT32 ise_fps;
	UINT32 vpe_fps;
} ENGINE_CFG;

/*-----------------------------------------------------------------------------*/
/* Global Functions                                                             */
/*-----------------------------------------------------------------------------*/
#if AI_ENABLE
static HD_RESULT mem_alloc(MEM_PARM *mem_parm, CHAR* name, UINT32 size)
{
	HD_RESULT ret = HD_OK;
	UINTPTR pa   = 0;
	void  *va   = NULL;

	//alloc private pool
	ret = hd_common_mem_alloc(name, &pa, (void**)&va, size, AI_DDRID);
	if (ret!= HD_OK) {
		return ret;
	}

	mem_parm->pa   = pa;
	mem_parm->va   = (UINTPTR)va;
	mem_parm->size = size;
	mem_parm->blk  = (UINT32)-1;

	return HD_OK;
}

static HD_RESULT mem_free(MEM_PARM *mem_parm)
{
	HD_RESULT ret = HD_OK;

	//free private pool
	ret =  hd_common_mem_free(mem_parm->pa, (void *)mem_parm->va);
	if (ret!= HD_OK) {
		return ret;
	}

	mem_parm->pa = 0;
	mem_parm->va = 0;
	mem_parm->size = 0;
	mem_parm->blk = (UINT32)-1;

	return HD_OK;
}
#endif

/*-----------------------------------------------------------------------------*/
/* Input Functions                                                             */
/*-----------------------------------------------------------------------------*/

///////////////////////////////////////////////////////////////////////////////

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

INT32 cap_mem_config(HD_PATH_ID video_cap_ctrl, HD_COMMON_MEM_INIT_CONFIG* p_mem_cfg, void* p_cfg, INT32 i)
{
	HD_DIM* p_dim = (HD_DIM*)p_cfg;
	// config common pool (cap)
	p_mem_cfg->pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	p_mem_cfg->pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_RAW_BUFSIZE(p_dim->w, p_dim->h, CAP_OUT_FMT)
													 +VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
													 +VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);

	p_mem_cfg->pool_info[i].blk_cnt = 10;

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
	} else if (sensor_sel == SEN_SEL_IMX290 || sensor_sel == SEN_SEL_OS05A10 || sensor_sel == SEN_SEL_OS02K10 || sensor_sel == SEN_SEL_IMX485 || sensor_sel == SEN_SEL_OS04A10 || sensor_sel == SEN_SEL_IMX415 || sensor_sel == SEN_SEL_IMX485 || sensor_sel == SEN_SEL_IMX334 || sensor_sel == SEN_SEL_SC850SL || sensor_sel == SEN_SEL_OS08C10) {
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
		}
	}

	if (sensor_info[sensor_sel].sensor_num == SEN_SEL_PATGEN) {
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

	if (sensor_info[sensor_sel].sensor_num != SEN_SEL_PATGEN) {
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

		if (sensor_info[sensor_sel].sensor_num == SEN_SEL_PATGEN) {// pattern gen
			video_in_param.sen_mode = HD_VIDEOCAP_PATGEN_MODE(HD_VIDEOCAP_SEN_PAT_COLORBAR, 200);
			#if FPGA_TEST
			video_in_param.frc = HD_VIDEO_FRC_RATIO(5,1);
			#else
			video_in_param.frc = HD_VIDEO_FRC_RATIO(g_fps,1);
			#endif
			video_in_param.dim.w = p_dim->w;
			video_in_param.dim.h = p_dim->h;
		} else {
			video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO; //auto select sensor mode by the parameter of HD_VIDEOCAP_PARAM_OUT
			video_in_param.frc = HD_VIDEO_FRC_RATIO(g_fps,1);
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

		if (sensor_info[sensor_sel].sensor_num == SEN_SEL_PATGEN) {// pattern gen
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

	{
		HD_VIDEOCAP_FUNC_CONFIG video_path_param = {0};

		video_path_param.out_func = 0;
		if (g_capbind == 1) //direct mode
			video_path_param.out_func = HD_VIDEOCAP_OUTFUNC_DIRECT;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_FUNC_CONFIG, &video_path_param);
		//printf("set_cap_param PATH_CONFIG=0x%X\r\n", ret);
	}

	if ((sensor_sel == SEN_SEL_SC850SL)) {
		UINT32 data_lane = 2;
		vendor_videocap_set(video_cap_path, VENDOR_VIDEOCAP_PARAM_DATA_LANE, &data_lane);
		printf("data_lane=%d \r\n", data_lane);
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
	p_mem_cfg->pool_info[i].blk_cnt = 10;
	p_mem_cfg->pool_info[i].ddr_id = VIDEO_DDRID;

	if (g_capbind == 1) {
		//direct
		p_mem_cfg->pool_info[i].blk_cnt += 2; //direct will pre-new 1
	}

	i++;

	p_mem_cfg->pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	p_mem_cfg->pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_NVX_BUFSIZE(p_dim->w, p_dim->h, HD_VIDEO_PXLFMT_YUV420_NVX2);  // NOTE: dim need align to 16 for rotate buffer
	p_mem_cfg->pool_info[i].blk_cnt = 10;
	p_mem_cfg->pool_info[i].ddr_id = VIDEO_DDRID;
	i++;

	if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
		p_mem_cfg->pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
		p_mem_cfg->pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_AIDED_BUFSIZE(p_dim->w, p_dim->h, HD_VIDEO_PXLFMT_RAW12);  // NOTE: dim need align to 16 for rotate buffer
		p_mem_cfg->pool_info[i].blk_cnt = 5;
		p_mem_cfg->pool_info[i].ddr_id = VIDEO_DDRID;
		i++;
	}

	return i;
}

INT32 out_mem_config(HD_PATH_ID video_proc_path, HD_COMMON_MEM_INIT_CONFIG* p_mem_cfg, void* p_cfg, INT32 i)
{
	HD_DIM* p_dim = (HD_DIM*)p_cfg;
	// config common pool (proc)
	p_mem_cfg->pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	p_mem_cfg->pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(p_dim->w, p_dim->h, HD_VIDEO_PXLFMT_YUV420);  // NOTE: dim need align to 16 for rotate buffer
	p_mem_cfg->pool_info[i].blk_cnt = 10;
	p_mem_cfg->pool_info[i].ddr_id = VIDEO_DDRID;
	i++;
	return i;
}

static HD_RESULT set_proc_cfg(HD_PATH_ID *p_video_proc_ctrl, HD_DIM* p_max_dim, HD_CTRL_ID ctrl_id, HD_VIDEOPROC_PIPE pipe, HD_VIDEOPROC_CTRLFUNC func, HD_VIDEO_PXLFMT input_pxlfmt)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
	HD_VIDEOPROC_CTRL video_ctrl_param = {0};
	HD_PATH_ID video_proc_ctrl = 0;

	ret = hd_videoproc_open(0, ctrl_id, &video_proc_ctrl); //open this for device control
	if (ret != HD_OK)
		return ret;

	if (p_max_dim != NULL ) {
		video_cfg_param.pipe = pipe;
		if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
			video_cfg_param.isp_id = (pipe == HD_VIDEOPROC_PIPE_PRE) ? ISP_ID_0 : ISP_ID_1;
		} else {
			video_cfg_param.isp_id = ISP_ID_0;
		}
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
		video_ctrl_param.ref_path_3dnr = REF_PATH;
	}
	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);

	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		video_path_param.in_func = 0;
		if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
			if ((g_capbind == 1) && (pipe == HD_VIDEOPROC_PIPE_PRE))
				video_path_param.in_func |= HD_VIDEOPROC_INFUNC_DIRECT; //direct NOTE: enable direct
		} else {
			if (g_capbind == 1)
				video_path_param.in_func |= HD_VIDEOPROC_INFUNC_DIRECT; //direct NOTE: enable direct
		}
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
		//printf("set_proc_param PATH_CONFIG=0x%X\r\n", ret);
	}

	{
		UINT32 in_depth = 3;
		ret =vendor_videoproc_set(video_proc_ctrl, VENDOR_VIDEOPROC_PARAM_IN_DEPTH, &in_depth);
	}

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

static HD_RESULT set_enc_param(HD_PATH_ID video_enc_path, HD_DIM *p_dim, HD_VIDEO_PXLFMT pxlfmt, UINT32 enc_type, UINT32 bitrate)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOENC_IN  video_in_param = {0};
	HD_VIDEOENC_OUT video_out_param = {0};
	HD_H26XENC_RATE_CONTROL rc_param = {0};
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
		rc_param.cbr.frame_rate_base         = ENC_FRAME_RATE;
		rc_param.cbr.frame_rate_incr         = 1;
		rc_param.cbr.init_i_qp               = 40;
		rc_param.cbr.min_i_qp                = 20;
		rc_param.cbr.max_i_qp                = 45;
		rc_param.cbr.init_p_qp               = 26;
		rc_param.cbr.min_p_qp                = 20;
		rc_param.cbr.max_p_qp                = 45;
		rc_param.cbr.static_time             = 4;
		#else
		rc_param.rc_mode                     = HD_RC_MODE_FIX_QP;
		rc_param.fixqp.frame_rate_base       = ENC_FRAME_RATE;
		rc_param.fixqp.frame_rate_incr       = 1;
		rc_param.fixqp.fix_i_qp              = 28;
		rc_param.fixqp.fix_p_qp              = 30;
		#endif
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL, &rc_param);
		if (ret != HD_OK) {
			printf("set_enc_rate_control = %d \n", ret);
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
			video_out_param.h26x.gop_num       = (ENC_FRAME_RATE > 20) ? ENC_FRAME_RATE * 2 : ENC_FRAME_RATE * 3;
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
			video_out_param.h26x.gop_num       = (ENC_FRAME_RATE > 20) ? ENC_FRAME_RATE * 2 : ENC_FRAME_RATE * 3;
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
		video_path_config.max_mem.svc_layer  = HD_SVC_DISABLE;
		video_path_config.max_mem.ltr        = FALSE;
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

typedef struct _NOISE_PROFILE  { //input blob
	UINT16 noise_base;
	UINT16 noise_slope;
} NOISE_PROFILE;

typedef struct _SNR_STRENGTH  { //input blob
	UINT16 min_motion;
	UINT16 max_motion;
	UINT16 min_detail;
	UINT16 max_detail;
} SNR_STRENGTH;

typedef struct _USE_REFERENCE  { //input blob
	UINT16 blend;
} USE_REFERENCE;

typedef enum {
	NET_IN_INPUT_COEFFA,
	NET_IN_INPUT_COEFFB,
	NET_IN_INPUT_BLEND,
	NET_IN_INPUT_MAX,
	ENUM_DUMMY4WORD(NET_IN_INPUT)
} NET_IN_INPUT;

typedef struct _NET_IN_CONFIG {
	CHAR input_filename[256];
	UINT32 w;
	UINT32 h;
	UINT32 c;
	UINT32 loff;
	UINT32 fmt;
	UINT32 type;
	UINT32 batch;
	UINT32 time;
} NET_IN_CONFIG;

typedef struct _NET_IN {

	NET_IN_CONFIG in_cfg[NET_IN_INPUT_MAX];
	MEM_PARM input_mem[NET_IN_INPUT_MAX];
	UINT32 in_id;
	VENDOR_AI3_BUF src_img[NET_IN_INPUT_MAX];
} NET_IN;


typedef struct _NET_PROC_CONFIG {

	CHAR model_filename[256];
	INT32 binsize;
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
	VENDOR_AI3_NET_INFO net_info;

	MEM_PARM input_mem;

	NOISE_PROFILE noise_profile[3];
	SNR_STRENGTH snr_strength[3];
	USE_REFERENCE use_reference[3];
} NET_PROC;

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
	#if (IME_POSTSHARPEN_ENABLE)
	HD_PATH_ID proc_path_3dnr;
	#endif
	HD_PATH_ID proc_ctrl_pre; //-- (pre-process)
	HD_PATH_ID proc_path_pre; //-- (pre-process)

	HD_DIM  out_max_dim;
	HD_DIM  out_dim;

	// (3)
	HD_VIDEOOUT_SYSCAPS out_syscaps;
	HD_PATH_ID out_ctrl;
	HD_PATH_ID out_path;

	HD_VIDEOOUT_HDMI_ID hdmi_id;
	UINT32 out_type;

	// (4) network
	NET_PROC_CONFIG net_proc_cfg;
	NET_IN_CONFIG net_in_cfg;
	NET_PATH_ID net_path;
	NET_PATH_ID in_path;

	// (5) encode
	HD_DIM  enc_max_dim;
	HD_DIM  enc_dim;
	HD_VIDEOENC_SYSCAPS enc_syscaps;
	HD_PATH_ID enc_path;

	//ai sync - begin
	pthread_t  net_proc_thread_id;
	pthread_t  net_proc_sub_thread_id;
	UINT32 net_proc_start;
	UINT32 net_proc_exit;
	UINT32 net_proc_oneshot;
	pthread_t  net_cap_thread_id;
	//ai sync - end

	//ai async - begin
	pthread_t  net_push_thread_id;
	UINT32 net_push_start;
	UINT32 net_push_exit;
	UINT32 net_push_oneshot;
	pthread_t  net_pull_thread_id;
	UINT32 net_pull_start;
	UINT32 net_pull_exit;
	UINT32 net_pull_oneshot;
	#define NET_QUEUE_DEPTH 2
	UINT32 net_queue_in_id; //ready to alloc before push in
	UINT32 net_queue_out_id; //ready to free after pull out
	UINT32 net_queue_cnt; //count
	HD_VIDEO_FRAME net_queue[NET_QUEUE_DEPTH+1];
	//ai async - end

	uintptr_t ai_cb;  //isp call to ai
	uintptr_t isp_cb;  //ai call to isp
	UINT32 shot_count;
	BOOL fix_pattern;
	BOOL dump_yuv;
	VPRC_OUTPUT vprc_output;
	pthread_t    socket_thread_id;

	HD_COMMON_MEM_VB_BLK blk;
	UINTPTR pa;
	UINTPTR va;
} VIDEO_LIVEVIEW;

static VIDEO_LIVEVIEW stream[3] = {0}; //0: main stream //1: sub stream (vout)

#if AI_ENABLE
static NET_PROC g_net[16] = {0};
static NET_IN g_in[16] = {0};

static HD_RESULT input_init(void)
{
	HD_RESULT ret = HD_OK;
	int  i;

	for (i = 0; i < 16; i++) {
		NET_IN* p_net = g_in + i;
		p_net->in_id = i;
	}
	return ret;
}

static HD_RESULT input_uninit(void)
{
	HD_RESULT ret = HD_OK;
	return ret;
}

static INT32 _getsize_model(char* filename)
{
	FILE *bin_fd;
	UINT32 bin_size = 0;

	bin_fd = fopen(filename, "rb");
	if (!bin_fd) {
		printf("get bin(%s) size fail\n", filename);
		return (-1);
	}

	fseek(bin_fd, 0, SEEK_END);
	bin_size = ftell(bin_fd);
	fseek(bin_fd, 0, SEEK_SET);
	fclose(bin_fd);

	return bin_size;
}

static UINT32 _load_model(CHAR *filename, UINTPTR va)
{
	FILE  *fd;
	UINT32 file_size = 0, read_size = 0;
	const UINTPTR model_addr = va;
	//DBG_DUMP("model addr = %08x\r\n", (int)model_addr);

	fd = fopen(filename, "rb");
	if (!fd) {
		printf("load model(%s) fail\r\n", filename);
		return 0;
	}

	fseek ( fd, 0, SEEK_END );
	file_size = ALIGN_CEIL_4( ftell(fd) );
	fseek ( fd, 0, SEEK_SET );

	read_size = fread ((void *)model_addr, 1, file_size, fd);
	if (read_size != file_size) {
		printf("size mismatch, real = %d, idea = %d\r\n", (int)read_size, (int)file_size);
	}
	fclose(fd);

	printf("load model(%s) ok\r\n", filename);
	return read_size;
}

static HD_RESULT network_init(void)
{
	HD_RESULT ret = HD_OK;

	// call init
	{
		VENDOR_AI3_DEV_CFG dev_cfg = {0};

		ret = vendor_ai3_dev_init(&dev_cfg);
		if (ret != HD_OK) {
			printf("vendor_ai3_dev_init fail=%d\n", ret);
			return ret;
		}
	}
	// dump AI3 version
	{
		VENDOR_AI3_VER ai3_ver = {0};
		ret = vendor_ai3_dev_get(VENDOR_AI3_CFG_VER, &ai3_ver);
		if (ret != HD_OK) {
			printf("vendor_ai3_dev_get(CFG_VER) fail=%d\n", ret);
			return ret;
		}
		printf("vendor_ai version = %s\r\n", ai3_ver.vendor_ai_impl_version);
		printf("kflow_ai  version = %s\r\n", ai3_ver.kflow_ai_impl_version);
		printf("kdrv_ai   version = %s\r\n", ai3_ver.kdrv_ai_impl_version);
	}
	return ret;
}

static HD_RESULT network_uninit(void)
{
	HD_RESULT ret = HD_OK;

	ret = vendor_ai3_dev_uninit();
	if (ret != HD_OK) {
		printf("vendor_ai3_dev_uninit fail=%d\n", ret);
	}

	return ret;
}

INT32 network_mem_config(NET_PATH_ID net_path, HD_COMMON_MEM_INIT_CONFIG* p_mem_cfg, void* p_cfg)
{
	NET_PROC* p_net = g_net + net_path;
	NET_PROC_CONFIG* p_proc_cfg = (NET_PROC_CONFIG*)p_cfg;

	memcpy((void*)&p_net->net_cfg, (void*)p_proc_cfg, sizeof(NET_PROC_CONFIG));
	if (strlen(p_net->net_cfg.model_filename) == 0) {
		printf("net_path(%u) input model is null\r\n", net_path);
		return HD_ERR_NG;
	}

	p_net->net_cfg.binsize = _getsize_model(p_net->net_cfg.model_filename);
	if (p_net->net_cfg.binsize <= 0) {
		printf("net_path(%u) input model is not exist?\r\n", net_path);
		return HD_ERR_NG;
	}

	printf("net_path(%u) set net_mem_cfg: model-file(%s), binsize=%d\r\n",
		net_path,
		p_net->net_cfg.model_filename,
		p_net->net_cfg.binsize);

	return HD_OK;
}

static HD_RESULT network_alloc_io_buf(NET_PATH_ID net_path, UINT32 req_size)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	CHAR mem_name[23] ;
	snprintf(mem_name, 23, "ai_io_buf %u", net_path);

	ret = mem_alloc(&p_net->io_mem, mem_name, req_size);
	if (ret != HD_OK) {
		printf("net_path(%lu) alloc ai_io_buf fail\r\n", net_path);
		return HD_ERR_FAIL;
	}

	printf("alloc_io_buf: work buf, pa = %#lx, va = %#lx, size = %lu\r\n", p_net->io_mem.pa, p_net->io_mem.va, p_net->io_mem.size);

	return ret;
}

static HD_RESULT network_free_io_buf(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;

	if (p_net->io_mem.pa && p_net->io_mem.va) {
		mem_free(&p_net->io_mem);
	}
	return ret;
}

static HD_RESULT network_alloc_intl_buf(NET_PATH_ID net_path, UINT32 req_size)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;

	CHAR mem_name[23] ;
	snprintf(mem_name, 23, "ai_ronly_buf %u", net_path);

	ret = mem_alloc(&p_net->intl_mem, mem_name, req_size);
	if (ret != HD_OK) {
		printf("net_path(%lu) alloc ai_ronly_buf fail\r\n", net_path);
		return HD_ERR_FAIL;
	}

	printf("alloc_intl_buf: internal buf, pa = %#lx, va = %#lx, size = %lu\r\n", p_net->intl_mem.pa, p_net->intl_mem.va, p_net->intl_mem.size);

	return ret;
}

static HD_RESULT network_free_intl_buf(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;

	if (p_net->intl_mem.pa && p_net->intl_mem.va) {
		mem_free(&p_net->intl_mem);
	}
	return ret;
}

static HD_RESULT network_alloc_input_buf(NET_PATH_ID net_path, UINT32 req_size)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	CHAR mem_name[23] ;
	snprintf(mem_name, 23, "ai_input_buf %u", net_path);

	ret = mem_alloc(&p_net->input_mem, mem_name, req_size);
	if (ret != HD_OK) {
		printf("net_path(%lu) alloc ai_input_buf fail\r\n", net_path);
		return HD_ERR_FAIL;
	}

	printf("alloc_input_buf: work buf, pa = %#lx, va = %#lx, size = %lu\r\n", p_net->input_mem.pa, p_net->input_mem.va, p_net->input_mem.size);

	return ret;
}

static HD_RESULT network_free_input_buf(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;

	if (p_net->input_mem.pa && p_net->input_mem.va) {
		mem_free(&p_net->input_mem);
	}
	return ret;
}


static HD_RESULT network_open(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 loadsize = 0;
	CHAR mem_name[23] ;
	snprintf(mem_name, 23, "model.bin %u", net_path);

	if (strlen(p_net->net_cfg.model_filename) == 0) {
		printf("net_path(%u) input model is null\r\n", net_path);
		return 0;
	}
	ret  =  mem_alloc(&p_net->proc_mem, mem_name,  p_net->net_cfg.binsize);
	if (ret != HD_OK) {
		printf("net_path(%u) mem_alloc model.bin fail=%d\n", net_path, ret);
		return HD_ERR_FAIL;
	}
	//load file
	loadsize = _load_model(p_net->net_cfg.model_filename, p_net->proc_mem.va);

	if (loadsize <= 0) {
		printf("net_path(%u) input model load fail: %s\r\n", net_path, p_net->net_cfg.model_filename);
		return 0;
	}

	// query model info for WORKBUF/RONLYBUF size , then alloc WORKBUF/RONLYBUF
	{
		VENDOR_AI3_MODEL_INFO model_info = {0};

		model_info.model_buf.pa   = p_net->proc_mem.pa;
		model_info.model_buf.va   = p_net->proc_mem.va;
		model_info.model_buf.size = p_net->proc_mem.size;
#if DBG_OUT_DUMP
		model_info.ctrl           = CTRL_BUF_DEBUG | CTRL_JOB_DEBUG | CTRL_JOB_DUMPOUT;
#endif
		ret = vendor_ai3_dev_get(VENDOR_AI3_CFG_MODEL_INFO, &model_info);
		if (ret != HD_OK) {
			printf("net_path(%u) vendor_ai3_dev_get(MODEL_INFO) fail=%d\n", net_path, ret);
			return HD_ERR_FAIL;
		}

		printf("model_info get => workbuf size = %d, ronlybuf size = %d\r\n", model_info.proc_mem.buf[AI3_PROC_BUF_WORKBUF].size, model_info.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size);

		// alloc WORKBUF/RONLYBUF
		ret = network_alloc_intl_buf(net_path, model_info.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size);
		if (ret != HD_OK) {
			printf("net_path(%u) alloc ronlybuf fail=%d\n", net_path, ret);
			return HD_ERR_FAIL;
		}

		// aiisp mode could share workbuf in multi-iso => only first model needs to alloc work buf
		if(net_path == 0 ) {
			ret = network_alloc_io_buf(net_path, model_info.proc_mem.buf[AI3_PROC_BUF_WORKBUF].size);
			if (ret != HD_OK) {
				printf("net_path(%u) alloc workbuf fail=%d\n", net_path, ret);
				return HD_ERR_FAIL;
			}
		}

		ret = network_alloc_input_buf(net_path, 0x10000);
		if (ret != HD_OK) {
			printf("net_path(%u) alloc workbuf fail=%d\n", net_path, ret);
			return HD_ERR_FAIL;
		}

		{
			//default
			p_net->noise_profile[0].noise_base = 378;
			p_net->noise_profile[0].noise_slope = 9762;
			p_net->snr_strength[0].min_motion = 3276;
			p_net->snr_strength[0].max_motion = 4095;
			p_net->snr_strength[0].min_detail = 410;
			p_net->snr_strength[0].max_detail = 410;
			p_net->use_reference[0].blend = 1;

			//disable 3D
			p_net->noise_profile[1].noise_base = 378;
			p_net->noise_profile[1].noise_slope = 9762;
			p_net->snr_strength[1].min_motion = 3276;
			p_net->snr_strength[1].max_motion = 4095;
			p_net->snr_strength[1].min_detail = 410;
			p_net->snr_strength[1].max_detail = 410;
			p_net->use_reference[1].blend = 0;

			//small noise profile
			p_net->noise_profile[2].noise_base = 141;
			p_net->noise_profile[2].noise_slope = 4994;
			p_net->snr_strength[2].min_motion = 3276;
			p_net->snr_strength[2].max_motion = 4095;
			p_net->snr_strength[2].min_detail = 410;
			p_net->snr_strength[2].max_detail = 410;
			p_net->use_reference[2].blend = 1;
		}
	}

	// call open()
	{
		VENDOR_AI3_PROC_CFG proc_cfg = {0};

		proc_cfg.model_buf.pa   = p_net->proc_mem.pa;
		proc_cfg.model_buf.va   = p_net->proc_mem.va;
		proc_cfg.model_buf.size = p_net->proc_mem.size;

		proc_cfg.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].pa   = p_net->intl_mem.pa;
		proc_cfg.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].va   = p_net->intl_mem.va;
		proc_cfg.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size = p_net->intl_mem.size;

		proc_cfg.proc_mem.buf[AI3_PROC_BUF_WORKBUF].pa   = g_net->io_mem.pa;
		proc_cfg.proc_mem.buf[AI3_PROC_BUF_WORKBUF].va   = g_net->io_mem.va;
		proc_cfg.proc_mem.buf[AI3_PROC_BUF_WORKBUF].size = g_net->io_mem.size;

		proc_cfg.plugin[AI3_PLUGIN_CPU] = vendor_ai_cpu1_get_engine();
		proc_cfg.config[AI3_PROC_CFG_ISP_POOL_ID] = ISP_POOL_ID(0) ;
		proc_cfg.config[AI3_PROC_CFG_ISP_MODE] = AI3_ISP_MULTI_ISO_MODE;
#if DBG_OUT_DUMP
		proc_cfg.ctrl           = CTRL_BUF_DEBUG | CTRL_JOB_DEBUG | CTRL_JOB_DUMPOUT;
#endif
		ret = vendor_ai3_net_open(&p_net->proc_id, &proc_cfg, &p_net->net_info);
		if (ret != HD_OK) {
			printf("net_path(%u) vendor_ai3_net_open() fail=%d\n", net_path, ret);
			return HD_ERR_FAIL;
		} else {
			printf("net_path(%u) open success => get proc_id(%u), need to set (%u)input buf, (%u)output buf\r\n", net_path, p_net->proc_id, p_net->net_info.in_buf_cnt, p_net->net_info.out_buf_cnt);
		}


	}
	return ret;
}

static HD_RESULT network_close(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id = p_net->proc_id;

// close
	ret = vendor_ai3_net_close(proc_id);
	if (ret != HD_OK) {
		printf("net_path(%u), proc_id(%u) vendor_ai3_net_close fail=%d\n", net_path, proc_id, ret);
		return HD_ERR_FAIL;
	}

	if ((ret = network_free_intl_buf(net_path)) != HD_OK)
		return ret;

	if ((ret = network_free_io_buf(net_path)) != HD_OK)
		return ret;

	if ((ret = network_free_input_buf(net_path)) != HD_OK)
		return ret;

	mem_free(&p_net->proc_mem);
	memset(&p_net->net_info, 0, sizeof(VENDOR_AI3_NET_INFO));

	return ret;
}
static HD_RESULT network_bind_cb(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret = HD_OK;

	ret = vendor_ai3_dev_get(VENDOR_AI3_CFG_AI_CB, &p_stream->ai_cb);
	if (ret != HD_OK) {
		printf("get ai cb, ret = %d\r\n", ret);
		return ret;
	}

	ret = vendor_videoproc_set(p_stream->proc_ctrl, VENDOR_VIDEOPROC_PARAM_AI_CB, &p_stream->ai_cb);
	if (ret != HD_OK) {
		printf("set ai cb, ret = %d\r\n", ret);
		return ret;
	}

	return ret;
}

#if ISP_CALLBACK
static HD_RESULT network_bind_isp_cb(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret = HD_OK;

	ret = vendor_videoproc_get(p_stream->proc_ctrl, VENDOR_VIDEOPROC_PARAM_ISP_CB, &p_stream->isp_cb);
	if (ret != HD_OK) {
		printf("get isp cb, ret = %d\r\n", ret);
		return ret;
	}

	ret = vendor_ai3_dev_set(VENDOR_AI3_CFG_ISP_CB, &p_stream->isp_cb);
	if (ret != HD_OK) {
		printf("set isp cb, ret = %d\r\n", ret);
		return ret;
	}

	return ret;
}
#endif

static HD_RESULT network_set_isp_ai_cfg(VIDEO_LIVEVIEW *p_stream, UINT32 path_id, BOOL enable)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + p_stream->net_path;

	//for dummy model
	VENDOR_VIDEOPROC_ISP_AI isp_ai = {0};
	IQT_OB_MODE_MANUAL ob_mode_manual = {0};
	IQT_DG_MODE_MANUAL dg_mode_manual = {0};

	isp_ai.path_id = path_id;
	isp_ai.proc_id = p_net->proc_id;

	ob_mode_manual.id = 0;
	dg_mode_manual.id = 0;
	vendor_isp_get_iq(IQT_ITEM_OB_MODE_MANUAL, &ob_mode_manual);
	vendor_isp_get_iq(IQT_ITEM_DG_MODE_MANUAL, &dg_mode_manual);

	if (enable) {
		if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
			if (g_shdr == 0) {
				ob_mode_manual.ob_mode_manual.manual_enable = 1;
				ob_mode_manual.ob_mode_manual.manual_mode = 2;
				dg_mode_manual.dg_mode_manual.manual_enable = 1;
				dg_mode_manual.dg_mode_manual.manual_mode = 2;

				ob_mode_manual.ob_mode_manual.sie_enable = 0;
				ob_mode_manual.ob_mode_manual.ife_enable = 0;
			} else {
				ob_mode_manual.ob_mode_manual.manual_enable = 0;
				dg_mode_manual.dg_mode_manual.manual_enable = 0;

				ob_mode_manual.ob_mode_manual.sie_enable = 1;
				ob_mode_manual.ob_mode_manual.sie_value = 0;
				ob_mode_manual.ob_mode_manual.ife_enable = 1;
				ob_mode_manual.ob_mode_manual.ife_value[0] = 0;
				ob_mode_manual.ob_mode_manual.ife_value[1] = ob_mode_manual.ob_mode_manual.ife_value[0];
				ob_mode_manual.ob_mode_manual.ife_value[2] = ob_mode_manual.ob_mode_manual.ife_value[0];
				ob_mode_manual.ob_mode_manual.ife_value[3] = ob_mode_manual.ob_mode_manual.ife_value[0];
				ob_mode_manual.ob_mode_manual.ife_value[4] = ob_mode_manual.ob_mode_manual.ife_value[0];
			}
			ret = vendor_videoproc_set(p_stream->proc_ctrl, VENDOR_VIDEOPROC_PARAM_ISP_AI_START, &isp_ai);
		} else {
			if (ai_isp_flow == 0) {
				ob_mode_manual.ob_mode_manual.ife_f_enable = 1;
				ob_mode_manual.ob_mode_manual.ife_f_value[0] = 256;
				ob_mode_manual.ob_mode_manual.ife_f_value[1] = ob_mode_manual.ob_mode_manual.ife_f_value[0];
				ob_mode_manual.ob_mode_manual.ife_f_value[2] = ob_mode_manual.ob_mode_manual.ife_f_value[0];
				ob_mode_manual.ob_mode_manual.ife_f_value[3] = ob_mode_manual.ob_mode_manual.ife_f_value[0];
				ob_mode_manual.ob_mode_manual.ife_f_value[4] = ob_mode_manual.ob_mode_manual.ife_f_value[0];
				ob_mode_manual.ob_mode_manual.ife_enable = 1;
				ob_mode_manual.ob_mode_manual.ife_value[0] = 0;
				ob_mode_manual.ob_mode_manual.ife_value[1] = ob_mode_manual.ob_mode_manual.ife_value[0];
				ob_mode_manual.ob_mode_manual.ife_value[2] = ob_mode_manual.ob_mode_manual.ife_value[0];
				ob_mode_manual.ob_mode_manual.ife_value[3] = ob_mode_manual.ob_mode_manual.ife_value[0];
				ob_mode_manual.ob_mode_manual.ife_value[4] = ob_mode_manual.ob_mode_manual.ife_value[0];
				ob_mode_manual.ob_mode_manual.sie_enable = 0;
			} else if(ai_isp_flow == 1) {
				ob_mode_manual.ob_mode_manual.ife_f_enable = 0;
				ob_mode_manual.ob_mode_manual.ife_enable = 1;
				ob_mode_manual.ob_mode_manual.ife_value[0] = 0;
				ob_mode_manual.ob_mode_manual.ife_value[1] = ob_mode_manual.ob_mode_manual.ife_value[0];
				ob_mode_manual.ob_mode_manual.ife_value[2] = ob_mode_manual.ob_mode_manual.ife_value[0];
				ob_mode_manual.ob_mode_manual.ife_value[3] = ob_mode_manual.ob_mode_manual.ife_value[0];
				ob_mode_manual.ob_mode_manual.ife_value[4] = ob_mode_manual.ob_mode_manual.ife_value[0];
				ob_mode_manual.ob_mode_manual.sie_enable = 1;
				ob_mode_manual.ob_mode_manual.sie_value = 256;
			} else if(ai_isp_flow == 2) {
				ob_mode_manual.ob_mode_manual.ife_f_enable = 0;
				ob_mode_manual.ob_mode_manual.ife_enable = 1;
				ob_mode_manual.ob_mode_manual.ife_value[0] = 0;
				ob_mode_manual.ob_mode_manual.ife_value[1] = ob_mode_manual.ob_mode_manual.ife_value[0];
				ob_mode_manual.ob_mode_manual.ife_value[2] = ob_mode_manual.ob_mode_manual.ife_value[0];
				ob_mode_manual.ob_mode_manual.ife_value[3] = ob_mode_manual.ob_mode_manual.ife_value[0];
				ob_mode_manual.ob_mode_manual.ife_value[4] = ob_mode_manual.ob_mode_manual.ife_value[0];
				ob_mode_manual.ob_mode_manual.sie_enable = 1;
				ob_mode_manual.ob_mode_manual.sie_value = 0;
			}
			ret = vendor_videoproc_set(p_stream->proc_ctrl, VENDOR_VIDEOPROC_PARAM_ISP_AI_START, &isp_ai);
		}
	} else {
		if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
			if (g_shdr == 0) {
				ob_mode_manual.ob_mode_manual.manual_enable = 1;
				ob_mode_manual.ob_mode_manual.manual_mode = 2;
				dg_mode_manual.dg_mode_manual.manual_enable = 1;
				dg_mode_manual.dg_mode_manual.manual_mode = 2;

				ob_mode_manual.ob_mode_manual.sie_enable = 0;
				ob_mode_manual.ob_mode_manual.ife_enable = 0;
			} else {
				ob_mode_manual.ob_mode_manual.manual_enable = 0;
				dg_mode_manual.dg_mode_manual.manual_enable = 0;

				ob_mode_manual.ob_mode_manual.sie_enable = 1;
				ob_mode_manual.ob_mode_manual.sie_value = 0;
				ob_mode_manual.ob_mode_manual.ife_enable = 1;
				ob_mode_manual.ob_mode_manual.ife_value[0] = 0;
				ob_mode_manual.ob_mode_manual.ife_value[1] = ob_mode_manual.ob_mode_manual.ife_value[0];
				ob_mode_manual.ob_mode_manual.ife_value[2] = ob_mode_manual.ob_mode_manual.ife_value[0];
				ob_mode_manual.ob_mode_manual.ife_value[3] = ob_mode_manual.ob_mode_manual.ife_value[0];
				ob_mode_manual.ob_mode_manual.ife_value[4] = ob_mode_manual.ob_mode_manual.ife_value[0];
			}
			ret = vendor_videoproc_set(p_stream->proc_ctrl, VENDOR_VIDEOPROC_PARAM_ISP_AI_STOP, &isp_ai);
		} else {
			ob_mode_manual.ob_mode_manual.ife_f_enable = 0;
			ob_mode_manual.ob_mode_manual.ife_f_enable = 0;
			ob_mode_manual.ob_mode_manual.ife_enable = 0;
			ret = vendor_videoproc_set(p_stream->proc_ctrl, VENDOR_VIDEOPROC_PARAM_ISP_AI_STOP, &isp_ai);
		}
	}
	vendor_isp_set_iq(IQT_ITEM_OB_MODE_MANUAL, &ob_mode_manual);
	vendor_isp_set_iq(IQT_ITEM_DG_MODE_MANUAL, &dg_mode_manual);
	if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
		ob_mode_manual.id = 1;
		dg_mode_manual.id = 1;
		vendor_isp_set_iq(IQT_ITEM_OB_MODE_MANUAL, &ob_mode_manual);
		vendor_isp_set_iq(IQT_ITEM_DG_MODE_MANUAL, &dg_mode_manual);
	}

	return ret;
}

static HD_RESULT network_set_buf_by_in_path_list(VIDEO_LIVEVIEW *p_stream, UINT32 para_id)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + p_stream->net_path;
	UINT32 proc_id = p_net->proc_id;
	UINT32 i;
	VENDOR_AI3_BUF ai_buf = {0};

	uintptr_t va, pa;

	ai_buf.sign = MAKEFOURCC('A','B','U','F');
	pa = p_net->input_mem.pa;
	va = p_net->input_mem.va;

	for (i = 0; i < p_net->net_info.in_buf_cnt; i++) {
		// get out buf (by out path list)
		ret = vendor_ai3_net_get(proc_id, p_net->net_info.in_path_list[i], &ai_buf);
		if (HD_OK != ret) {
			printf("net_path(%u), proc_id(%u) get in buf fail, i(%d), in_path(0x%lx)\n", p_stream->net_path, proc_id, i,  p_net->net_info.in_path_list[i]);
			goto exit;
		}

		ai_buf.va = va;
		ai_buf.pa = pa;

if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
		if (i == 0) {
			//motion_2d_str
			*(UINT8*)ai_buf.va = (UINT8)aiisp_pq_final_param_539A.nr_param[i];
			hd_common_mem_flush_cache((void *)ai_buf.va, sizeof(UINT8));
			ai_buf.size = sizeof(UINT8);
			ai_buf.fmt = HD_VIDEO_PXLFMT_AI_UINT8;

			printf("set _2dnr_motion_str %d\r\n", *(UINT8*)ai_buf.va);
		} else if (i == 1) {
			//still_2d_str
			*(UINT8*)ai_buf.va = (UINT8)aiisp_pq_final_param_539A.nr_param[i];
			hd_common_mem_flush_cache((void *)ai_buf.va, sizeof(UINT8));
			ai_buf.size = sizeof(UINT8);
			ai_buf.fmt = HD_VIDEO_PXLFMT_AI_UINT8;

			printf("set _2dnr_still_str %d\r\n", *(UINT8*)ai_buf.va);
		} else {
			printf("539A should has only 2 inputfeat ...!!\r\n");
		}
} else {
		if(i < 2) {
			// coeff_a & coeff_b
			*(UINT16*)ai_buf.va = (UINT16)aiisp_pq_final_param.nr_param[i];
			hd_common_mem_flush_cache((void *)ai_buf.va, sizeof(UINT16));
			ai_buf.size = sizeof(UINT16);
			ai_buf.fmt = HD_VIDEO_PXLFMT_AI_UINT16;
		} else {
			*(UINT8*)ai_buf.va = (UINT8)aiisp_pq_final_param.nr_param[i];
			hd_common_mem_flush_cache((void *)ai_buf.va, sizeof(UINT8));
			ai_buf.size = sizeof(UINT8);
			ai_buf.fmt = HD_VIDEO_PXLFMT_AI_UINT8;
		}
}
		ret = vendor_ai3_net_set(proc_id, p_net->net_info.in_path_list[i], &ai_buf);
		if (HD_OK != ret) {
			printf("proc_id(%u)push input fail !! i(%lu)\n", proc_id, i);
			goto exit;
		}

		pa += ALIGN_CEIL_64(ai_buf.size);
		va += ALIGN_CEIL_64(ai_buf.size);
	}

exit:
	return ret;
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

#if !FPGA_TEST
	if (out_type <= 2) {
		if ((ret = hd_videoout_init()) != HD_OK)
			return ret;
	}

	if ((ret = hd_videoenc_init()) != HD_OK)
		return ret;
#endif

	#if AI_ENABLE
	if ((ret = input_init()) != HD_OK)
		return ret;
	if ((ret = network_init()) != HD_OK)
		return ret;
	#endif
	if ((ret = hd_gfx_init()) != HD_OK)
		return ret;
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

	if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
		HD_VIDEOPROC_CTRLFUNC func;
		HD_VIDEO_PXLFMT input_pxlfmt;

		// set videoproc config (pre-process)
		func = HD_VIDEOPROC_FUNC_AF;
		func |= HD_VIDEOPROC_FUNC_BNR | HD_VIDEOPROC_FUNC_BNR_STA | HD_VIDEOPROC_FUNC_AIDED;
		if (g_shdr == 1) {
			func |= HD_VIDEOPROC_FUNC_SHDR;
			func |= HD_VIDEOPROC_FUNC_FUSION;
		}
		if (g_shdr == 1) {
			input_pxlfmt = SHDR_CAP_OUT_FMT;
		} else {
			input_pxlfmt = CAP_OUT_FMT;
		}
		ret = set_proc_cfg(&p_stream->proc_ctrl_pre, p_proc_max_dim, HD_VIDEOPROC_0_CTRL, HD_VIDEOPROC_PIPE_PRE, func, input_pxlfmt);
		if (ret != HD_OK) {
			printf("set proc-cfg-pre fail=%d\n", ret);
			return HD_ERR_NG;
		}

		if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &p_stream->proc_path_pre)) != HD_OK)
			return ret;

		// set videoproc config
		func = HD_VIDEOPROC_FUNC_AF | HD_VIDEOPROC_FUNC_WDR | HD_VIDEOPROC_FUNC_DEFOG | HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA;
		func |= HD_VIDEOPROC_FUNC_AIDED;
		input_pxlfmt = CAP_OUT_FMT;
		ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, HD_VIDEOPROC_3_CTRL, HD_VIDEOPROC_PIPE_RAWALL_LITE, func, input_pxlfmt);
		if (ret != HD_OK) {
			printf("set proc-cfg fail=%d\n", ret);
			return HD_ERR_NG;
		}

		if (stream[0].fix_pattern == FALSE) {
			if ((ret = hd_videocap_open(HD_VIDEOCAP_IN(SEN1_VCAP_ID, 0), HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), &p_stream->cap_path)) != HD_OK)
				return ret;
		}
		if ((ret = hd_videoproc_open(HD_VIDEOPROC_3_IN_0, SOURCE_PATH, &p_stream->proc_path)) != HD_OK)
			return ret;

		#if (IME_POSTSHARPEN_ENABLE)
		if ((ret = hd_videoproc_open(HD_VIDEOPROC_3_IN_0, HD_VIDEOPROC_3_OUT_4, &p_stream->proc_path_3dnr)) != HD_OK)
			return ret;
		#endif
	} else {
		HD_VIDEOPROC_CTRLFUNC func;

		// set videoproc config
		func = VIDEOPROC_ALG_FUNC;
		if (g_shdr == 1) {
			func |= HD_VIDEOPROC_FUNC_SHDR;
		}
		ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, HD_VIDEOPROC_0_CTRL, HD_VIDEOPROC_PIPE_RAWALL, func, CAP_OUT_FMT);
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
	}

	#if AI_ENABLE

	#if ISP_CALLBACK
	if ((ret = network_bind_isp_cb(p_stream)) != HD_OK)
		return ret;
	#endif

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

static HD_RESULT open_module_3(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret;

	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_VIDEOENC_0_OUT_0, &p_stream->enc_path)) != HD_OK)
		return ret;

	return HD_OK;
}
static HD_RESULT open_module_5(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret = HD_OK;

	#if (AI_ENABLE)
	if ((ret = network_open(p_stream->net_path)) != HD_OK)
		return ret;
	#endif

	return ret;
}

static HD_RESULT close_module(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret;
	if (stream[0].fix_pattern == FALSE) {
		if ((ret = hd_videocap_close(p_stream->cap_path)) != HD_OK)
			return ret;
	}
	if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
		if ((ret = hd_videoproc_close(p_stream->proc_path_pre)) != HD_OK)
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

static HD_RESULT close_module_3(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_videoenc_close(p_stream->enc_path)) != HD_OK)
		return ret;

	return HD_OK;
}


static HD_RESULT close_module_5(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret = HD_OK;

	#if (AI_ENABLE)
	if ((ret = network_close(p_stream->net_path)) != HD_OK)
		return ret;
	#endif

	return ret;
}

static HD_RESULT exit_module(UINT32 out_type)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_uninit()) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_uninit()) != HD_OK)
		return ret;

#if !FPGA_TEST
	if ((ret = hd_videoenc_uninit()) != HD_OK)
		return ret;
	if (out_type <= 2) {
		if ((ret = hd_videoout_uninit()) != HD_OK)
			return ret;
	}
#endif

	#if AI_ENABLE
	if ((ret = input_uninit()) != HD_OK)
		return ret;
	if ((ret = network_uninit()) != HD_OK)
		return ret;
	#endif
	if ((ret = hd_gfx_uninit()) != HD_OK)
		return ret;
	return HD_OK;
}

uintptr_t get_post_buf(uint32_t size)
{
	uintptr_t buf = (uintptr_t)malloc(size);

	return buf;

}

///////////////////////////////////////////////////////////////////////////////

static VOID *network_proc_thread(VOID *arg);

static HD_RESULT network_start(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret = HD_OK;

	#if (AI_ENABLE)
	NET_PROC* p_net = g_net + p_stream->net_path;
	UINT32 proc_id = p_net->proc_id;
	#endif

	#if (AI_ENABLE)
	ret = vendor_ai3_net_start(proc_id);
	if (HD_OK != ret) {
		printf("net_path(%u), proc_id(%u) vendor_ai3_net_start fail !!\n", p_stream->net_path, proc_id);
	}

	ret = network_set_buf_by_in_path_list(p_stream, 0);
	if (HD_OK != ret) {
		printf("net_path(%u), proc_id(%u), para_id(%u) network_set_buf_by_in_path_list fail !!\n", p_stream->net_path, proc_id, 0);
	}
	#endif

	return ret;
}

static HD_RESULT network_stop(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret = HD_OK;

	#if AI_ENABLE
	NET_PROC* p_net = g_net + p_stream->net_path;
	UINT32 proc_id = p_net->proc_id;

	//stop: should be call after last time proc
	ret = vendor_ai3_net_stop(proc_id);
	if (HD_OK != ret) {
		printf("net_path(%u), proc_id(%u) vendor_ai3_net_stop fail !!\n", p_stream->net_path, proc_id);
	}
	#endif

	return ret;
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

			if (p_stream->out_type <= 2 && p_stream->vprc_output == VPRC_OUTPUT_VOUT) {
				ret = hd_videoout_push_in_buf(p_stream->out_path, &video_frame, NULL, -1); // blocking mode
				if (ret != HD_OK && ret != HD_ERR_OVERRUN) {
					printf("out_push error=%d !!\r\n\r\n", ret);
					goto skip;
				}
			} else if (p_stream->vprc_output == VPRC_OUTPUT_VENC) {
				ret = hd_videoenc_push_in_buf(p_stream->enc_path, &video_frame, NULL, 300); // blocking mode
				if (ret != HD_OK && ret != HD_ERR_OVERRUN) {
					printf("enc_push error=%d !!\r\n\r\n", ret);
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

static void *memcpy_thread(void *arg)
{
	void                 *va1, *va2;
	UINT32               size = 0x500000;
	UINT64               total_size = 0;
	COPY_INFO            *info = (COPY_INFO *)arg;
	UINT64               frame_us = 0, t1, t2, spent_time, remain_time, total_t = 0;
	int                  cnt = 0;


	frame_us = 1000000/info->fps;
	if (NULL != info) {
		size = info->copy_size;
	}
	va1 = malloc(size);
	if (va1 == NULL) {
		printf("err:alloc size 0x%x\r\n", size);
		return 0;
	}
	va2 = malloc(size);
	if (va2 == NULL) {
		printf("err:alloc size 0x%x\r\n", size);
		return 0;
	}
	while (!g_quit) {
		t1 = hd_gettime_us();
		memcpy(va1, va2, size);
		t2 = hd_gettime_us();
		spent_time = t2 - t1;
		if (frame_us > spent_time) {
			remain_time = frame_us - spent_time;
		} else {
			remain_time = 0;
		}
		if (remain_time) {
			usleep(remain_time);
		}
		//printf("remain_time = %d us\r\n", remain_time);
		total_size += size;
		total_t = total_t + (t2 - t1);
		cnt++;
		if (cnt == 600){
			printf("memcpy size = %d MB, time = %d us, perf = %f MB/s\r\n", (unsigned int)total_size/1024/1024, total_t, (float)total_size/total_t*1000000/1024/1024);
			cnt = 0;
			total_t = 0;
			total_size = 0;
		}
	}
	free(va1);
	free(va2);
	return 0;
}

static void *dmacpy_thread(void *arg)
{
	void                 *va1, *va2;
	uintptr_t            pa1, pa2;
	UINT32               size = 0x400000;
	HD_RESULT            ret;
	UINT64               total_size = 0;
	COPY_INFO            *info = (COPY_INFO *)arg;
	UINT64               frame_us = 0, t1, t2, spent_time, remain_time, total_t = 0;
	int                  cnt = 0;


	frame_us = 1000000/info->fps;
	if (NULL != info) {
		size = info->copy_size;
	}
	ret = hd_common_mem_alloc("test1", &pa1, (void **)&va1, size, info->ddr_id);
	if (ret != HD_OK) {
		printf("err:alloc size 0x%x, ddr %d\r\n", size, info->ddr_id);
		return 0;
	}
	ret = hd_common_mem_alloc("test2", &pa2, (void **)&va2, size, info->ddr_id);
	if (ret != HD_OK) {
		printf("err:alloc size 0x%x, ddr %d\r\n", size, info->ddr_id);
		return 0;
	}
	while (!g_quit) {
		t1 = hd_gettime_us();
		hd_gfx_memcpy(pa1, pa2, size);
		t2 = hd_gettime_us();
		spent_time = t2 - t1;
		if (frame_us > spent_time) {
			remain_time = frame_us - spent_time;
		} else {
			remain_time = 0;
		}
		if (remain_time) {
			usleep(remain_time);
		}
		//printf("remain_time = %d us\r\n", remain_time);
		total_size += size;
		total_t = total_t + (t2 - t1);
		cnt++;
		if (cnt == 600){
			printf("dmacpy size = %d MB, time = %d ms, perf = %f MB/s\r\n", (unsigned int)total_size/1024/1024, total_t, (float)total_size/total_t*1000000/1024/1024);
			cnt = 0;
			total_t = 0;
			total_size = 0;
		}
	}
	ret = hd_common_mem_free(pa1, va1);
	if (ret != HD_OK) {
		printf("err:free pa = 0x%lx, va = 0x%lx\r\n", (uintptr_t)pa1, (uintptr_t)va1);
		return 0;
	}
	ret = hd_common_mem_free(pa2, va2);
	if (ret != HD_OK) {
		printf("err:free pa = 0x%lx, va = 0x%lx\r\n", (uintptr_t)pa2, (uintptr_t)va2);
		return 0;
	}
	return 0;
}

static void *gfxcpy_thread(void *arg)
{
	#define COMPARE_DATA  0
	void                 *va1, *va2;
	uintptr_t            pa1, pa2;
	UINT32               size = 0x400000;
	HD_RESULT            ret;
	UINT32               ts, te;
	UINT32               total_size = 0;
	HD_GFX_COPY          param = {0};
	BOOL                 is_print = FALSE;
	COPY_INFO            *info = (COPY_INFO *)arg;
	UINT32               frame_count;
	UINT32               i, *pdata;
	#if COMPARE_DATA
	UINT32               *pdata2;
	BOOL                 hasError = FALSE;
	#endif
	UINT64               frame_us = 0, t1, t2, spent_time, remain_time, total_time;

	frame_us = 1000000/info->fps;
	if (NULL != info) {
		size = info->copy_size;
	}
	ret = hd_common_mem_alloc("test1", &pa1, (void **)&va1, size, info->ddr_id);
	if (ret != HD_OK) {
		printf("err1:alloc size 0x%x, ddr %d\r\n", size, info->ddr_id);
		return 0;
	}
	ret = hd_common_mem_alloc("test2", &pa2, (void **)&va2, size, info->ddr_id);
	if (ret != HD_OK) {
		printf("err2:alloc size 0x%x, ddr %d\r\n", size, info->ddr_id);
		return 0;
	}
	param.src_img.dim.w            = 4096;
	param.src_img.dim.h            = size/4096;
	param.src_img.format           = HD_VIDEO_PXLFMT_Y8;
	param.src_img.p_phy_addr[0]    = pa1;
	param.src_img.lineoffset[0]    = param.src_img.dim.w;
	param.dst_img.dim.w            = 4096;
	param.dst_img.dim.h            = param.src_img.dim.h;
	param.dst_img.format           = HD_VIDEO_PXLFMT_Y8;
	param.dst_img.p_phy_addr[0]    = pa2;
	param.dst_img.lineoffset[0]    = param.src_img.dim.w;
	param.src_region.x             = 0;
	param.src_region.y             = 0;
	param.src_region.w             = param.src_img.dim.w;
	param.src_region.h             = param.src_img.dim.h;
	param.dst_pos.x                = 0;
	param.dst_pos.y                = 0;
	param.colorkey                 = 0;
	param.alpha                    = 255;
	ts = hd_gettime_ms();
	printf("src w = %d ,h = %d, linoff = %d\r\n", param.src_img.dim.w, param.src_img.dim.h, param.src_img.lineoffset[0]);
	printf("dst w = %d ,h = %d, linoff = %d\r\n", param.dst_img.dim.w, param.dst_img.dim.h, param.dst_img.lineoffset[0]);
	frame_count = 0;
	total_time = 0;
	while (!g_quit) {
		frame_count ++;
		t1 = hd_gettime_us();
		//vendor_gfx_copy_no_flush(&param);
		//cpu modify
		pdata = (UINT32 *)va1;
		for (i= 0;i< 1000;i++) {
			*pdata = frame_count;
			if (i % 20 == 0) {
				pdata += 500;
			} else {
				pdata ++;
			}
		}
		hd_gfx_copy(&param);
		t2 = hd_gettime_us();
		spent_time = t2 - t1;
		if (frame_us > spent_time) {
			remain_time = frame_us - spent_time;
		} else {
			remain_time = 0;
		}
		total_time += spent_time;
		if (remain_time) {
			usleep(remain_time);
		}
		total_size += size;
		te = hd_gettime_ms();
		if (!is_print && te - ts > 5000) {
			printf("total_size = 0x%x, time = %d ms, perf = %f MB/s\r\n", (unsigned int)total_size, te - ts, (float)total_size/(te - ts)/1024);
			is_print = TRUE;
		}
		if (frame_count % 100 == 0) {
			//printf("gfxcpy_thread framecount = %d\r\n", *pdata);
			printf("one frame time = %lld us, size =%d\r\n", total_time/frame_count, size);
		}
		// cpu compare
		#if COMPARE_DATA
		pdata = (UINT32 *)va1;
		pdata2 = (UINT32 *)va2;
		if (frame_count % 100 == 0) {
			//printf("gfxcpy_thread framecount = %d\r\n", *pdata);
			//printf("one frame time = %d us, size =%d\r\n", (hd_gettime_us() - tb)/frame_count);
		}
		for (i= 0;i< size/4;i++) {
			if (*pdata != *pdata2) {
				printf("ERR: i = %d, pdata = 0x%x, pdata2 = 0x%x\r\n", i, *pdata, *pdata2);
				hasError = TRUE;
			}
			pdata++;
			pdata2++;
		}
		if (hasError) {
			break;
		}
		#endif
	}
	ret = hd_common_mem_free(pa1, va1);
	if (ret != HD_OK) {
		printf("err:free pa = 0x%lx, va = 0x%lx\r\n", (uintptr_t)pa1, (uintptr_t)va1);
		return 0;
	}
	ret = hd_common_mem_free(pa2, va2);
	if (ret != HD_OK) {
		printf("err:free pa = 0x%lx, va = 0x%lx\r\n", (uintptr_t)pa2, (uintptr_t)va2);
		return 0;
	}
	return 0;
}

static void *gfxscale_thread(void *arg)
{
	void                 *va1, *va2;
	uintptr_t            pa1, pa2;
	UINT32               size = 0x400000;
	HD_RESULT            ret;
	UINT32               ts, te;
	UINT32               total_size = 0;
	HD_GFX_SCALE         param = {0};
	BOOL                 is_print = FALSE;
	COPY_INFO            *info = (COPY_INFO *)arg;
	UINT64               frame_us = 0, t1, t2, spent_time, remain_time;

	frame_us = 1000000/info->fps;
	if (NULL != info) {
		size = info->copy_size;
	}
	ret = hd_common_mem_alloc("test1", &pa1, (void **)&va1, size, info->ddr_id);
	if (ret != HD_OK) {
		printf("err1:alloc size 0x%x, ddr %d\r\n", size, info->ddr_id);
		return 0;
	}
	ret = hd_common_mem_alloc("test2", &pa2, (void **)&va2, size, info->ddr_id);
	if (ret != HD_OK) {
		printf("err2:alloc size 0x%x, ddr %d\r\n", size, info->ddr_id);
		return 0;
	}
	param.src_img.dim.w            = 4096;
	param.src_img.dim.h            = size/4096;
	param.src_img.format           = HD_VIDEO_PXLFMT_Y8;
	param.src_img.p_phy_addr[0]    = pa1;
	param.src_img.lineoffset[0]    = param.src_img.dim.w;
	param.dst_img.dim.w            = 4096;
	param.dst_img.dim.h            = param.src_img.dim.h;
	param.dst_img.format           = HD_VIDEO_PXLFMT_Y8;
	param.dst_img.p_phy_addr[0]    = pa2;
	param.dst_img.lineoffset[0]    = param.src_img.dim.w;
	param.src_region.x             = 0;
	param.src_region.y             = 0;
	param.src_region.w             = param.src_img.dim.w;
	param.src_region.h             = param.src_img.dim.h;
	param.dst_region.x             = 0;
	param.dst_region.y             = 0;
	param.dst_region.w             = param.src_img.dim.w;
	param.dst_region.h             = param.src_img.dim.h;
	ts = hd_gettime_ms();

	printf("src w = %d ,h = %d, linoff = %d\r\n", param.src_img.dim.w, param.src_img.dim.h, param.src_img.lineoffset[0]);
	printf("dst w = %d ,h = %d, linoff = %d\r\n", param.dst_img.dim.w, param.dst_img.dim.h, param.dst_img.lineoffset[0]);
	while (!g_quit) {
		t1 = hd_gettime_us();
		vendor_gfx_scale_dma_flush(&param, 1);
		t2 = hd_gettime_us();
		spent_time = t2 - t1;
		if (frame_us > spent_time) {
			remain_time = frame_us - spent_time;
		} else {
			remain_time = 0;
		}
		if (remain_time) {
			usleep(remain_time);
		}
		total_size += size;
		te = hd_gettime_ms();
		if (!is_print && te - ts > 5000) {
			printf("scale total_size = 0x%x, time = %d ms, perf = %f MB/s\r\n", (unsigned int)total_size, te - ts, (float)total_size/(te - ts)/1024);
			is_print = TRUE;
		}
	}
	ret = hd_common_mem_free(pa1, va1);
	if (ret != HD_OK) {
		printf("err:free pa = 0x%lx, va = 0x%lx\r\n", (uintptr_t)pa1, (uintptr_t)va1);
		return 0;
	}
	ret = hd_common_mem_free(pa2, va2);
	if (ret != HD_OK) {
		printf("err:free pa = 0x%lx, va = 0x%lx\r\n", (uintptr_t)pa2, (uintptr_t)va2);
		return 0;
	}
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

	UINT32 enc_type = 1; // 0:H265; 1:H264
	UINT32 enc_bitrate = 8; // MB
	BOOL second_ai = FALSE;

	pthread_t  memcpy_thread_id;
	pthread_t  dmacpy_thread_id;
	pthread_t  gfxcpy_thread_id;
	pthread_t  memcpy_thread_id2;
	pthread_t  gfxscale_thread_id;
	COPY_INFO  copy_info[4];
	BOOL       is_start_memcpy_test = FALSE;
	BOOL       is_start_dmacpy_test = FALSE;
	BOOL       is_start_gfxcpy_test = FALSE;
	BOOL       is_start_memcpy_test2 = FALSE;
	BOOL       is_start_gfxscale_test = FALSE;
	ENGINE_CFG  engine_cfg = {0};
	int    sleep_us = 0; //5000;

	chip_name = getenv("NVT_CHIP_ID");

	#if AI_ENABLE
	static UINT32 curr_ai_path = 0;

	//net proc
	NET_PROC_CONFIG net_cfg = {
		.model_filename = "/mnt/sd/para/nvt_model_aiisp.bin",
		.label_filename = "/mnt/sd/accuracy/labels.txt"
	};

	NET_PROC_CONFIG net_cfg2 = {
		.model_filename = "/mnt/sd/para/nvt_model_aiisp.bin",
		.label_filename = "/mnt/sd/accuracy/labels2.txt"
	};
	#endif

	system("insmod /lib/modules/5.10.168/hdal/kflow_ai_isp/kflow_ai_isp.ko");

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
		stream[0].fix_pattern = atoi(argv[2]);
		printf("fix_pattern %d\r\n", stream[0].fix_pattern);
	}
	if (argc >= 4) {
		sensor_sel = atoi(argv[3]);
		printf("sensor_sel %d\r\n", sensor_sel);
	}
	if (argc >= 5) {
		stream[0].vprc_output = atoi(argv[4]);
		printf("vprc_output %d\r\n", stream[0].vprc_output);
	}
	if (argc >= 6) {
		g_shdr = atoi(argv[5]);
		printf("shdr %d\r\n", g_shdr);
	}
	if (argc >= 7) {
		#if AI_ENABLE
		sprintf(net_cfg.model_filename, "%s", argv[6]);
		printf("model %s\r\n", net_cfg.model_filename);
		#endif
	}
	if (argc >= 9) {
		second_ai = atoi(argv[7]);
		if (second_ai) {
			sprintf(net_cfg2.model_filename, "%s", argv[8]);
		}
		printf("second_ai %d, model2 %s\r\n", second_ai, net_cfg2.model_filename);
	}
	if (argc >= 10) {
		g_fps = atoi(argv[9]);
		printf("fps %d\r\n", g_fps);
	}
	if (argc >= 11) {
		g_capbind = atoi(argv[10]);
		printf("capbind %d\r\n", g_capbind);
	}

	cap_size_w = sensor_info[sensor_sel].size.w;
	cap_size_h = sensor_info[sensor_sel].size.h;

	#if VCAP_PATTERN_GEN
	sensor_info[sensor_sel].sensor_num = SEN_SEL_PATGEN;
	snprintf(sensor_info[sensor_sel].sensor_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "PATTERN_GEN");

	printf("force PATTERN_GEN\r\n");
	#endif

	if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
		SOURCE_PATH = HD_VIDEOPROC_3_OUT_0; //out 0~4 is physical path
		EXTEND_PATH = HD_VIDEOPROC_3_OUT_5; //out 5~15 is extend path
		EXTEND_PATH2 = HD_VIDEOPROC_3_OUT_6; //out 5~15 is extend path
		#if (IME_POSTSHARPEN_ENABLE)
		REF_PATH = HD_VIDEOPROC_3_OUT_4; //out 0~4 is physical path
		#else
		REF_PATH = HD_VIDEOPROC_3_OUT_0; //out 0~4 is physical path
		#endif
	}

	init_share_memory(cap_size_w, cap_size_h);

	if (stream[0].fix_pattern == 1) {
		stream[0].dump_yuv = 1;
	}

	if (stream[0].vprc_output == VPRC_OUTPUT_VENC_1OUT) {
		stream[0].vprc_output = VPRC_OUTPUT_VENC;
	}

	printf("\r\n\r\n");

	stream[0].net_path = 0;
	stream[0].in_path = 0;

	stream[0].out_type = out_type;

	memcpy((void*)&stream[1], (void*)&stream[0], sizeof(VIDEO_LIVEVIEW));

	// set engine fps default value
	engine_cfg.memcpy1_fps = 100;
	engine_cfg.memcpy2_fps = 60;
	engine_cfg.dmacpy1_fps = 100;
	engine_cfg.grph_fps = 100;
	engine_cfg.ise_fps = 50;
	engine_cfg.vpe_fps = 25;

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

		i = out_mem_config(stream[0].out_path, &mem_cfg, &stream[0].proc_max_dim, i);

		i = user_mem_config(&mem_cfg, i);

		#if AI_ENABLE
		network_mem_config(stream[0].net_path, &mem_cfg, &net_cfg);
		if (second_ai) {
			stream[2].net_path = 1;
			stream[2].in_path = 1;

			ret = network_mem_config(stream[2].net_path, &mem_cfg, &net_cfg2);
			if (ret != HD_OK) {
				printf("network_mem_config stream2 fail (%d) \n", ret);
				goto exit;
			}

		}
		#endif
		ret = hd_common_mem_init(&mem_cfg);
	}
	if (HD_OK != ret) {
		printf("hd_common_mem_init err: %d\r\n", ret);
		goto exit;
	}

#if !FPGA_TEST
	if (vendor_isp_init() == HD_ERR_NG) {
		printf("vendor_isp_init fail\r\n");
		goto exit;
	}

	// reload ISP cfg
	{
	AET_CFG_INFO cfg_info = {0};

	cfg_info.id = IPP_ID_0;
	if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
		snprintf(cfg_info.path, 64, "/mnt/app/isp/%s_AI_539a_old.cfg", sensor_info[sensor_sel].isp_cfg_name);
	} else {
		snprintf(cfg_info.path, 64, "/mnt/app/isp/%s_AI.cfg", sensor_info[sensor_sel].isp_cfg_name);
	}
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
#endif

	// init all modules
	ret = init_module(out_type);
	if (ret != HD_OK) {
		printf("init fail=%d\n", ret);
		goto exit;
	}


	// open video_liveview modules (main)
	ret = open_module(&stream[0], &stream[0].proc_max_dim);
	if (ret != HD_OK) {
		printf("open fail=%d\n", ret);
		goto exit;
	}


	// open video_liveview modules (sub)
	if (out_type <= 2 && stream[0].vprc_output == VPRC_OUTPUT_VOUT) {
		ret = open_module_2(&stream[0], out_type);
		if (ret != HD_OK) {
			printf("open2 fail=%d\n", ret);
			goto exit;
		}
	}

	if (stream[0].vprc_output == VPRC_OUTPUT_VENC) {
		ret = open_module_3(&stream[0]);
		if (ret != HD_OK) {
			printf("open3 fail=%d\n", ret);
			goto exit;
		}
	}

	if (second_ai) {
		ret = open_module_5(&stream[2]);
		if (ret != HD_OK) {
			printf("open5 fail=%d\n", ret);
			goto exit;
		}

		stream[2].proc_ctrl = stream[0].proc_ctrl;
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

	if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
		// set videoproc parameter (pre-process)
		ret = set_proc_param(stream[0].proc_path_pre, &main_dim, HD_VIDEO_DIR_NONE, 1, HD_VIDEO_PXLFMT_RAW12 | HD_VIDEO_PIX_RGGB_R);
		if (ret != HD_OK) {
			printf("set proc-pre fail=%d\n", ret);
			goto exit;
		}
	}

	// set videoproc parameter (main)
	ret = set_proc_param(stream[0].proc_path, &main_dim, HD_VIDEO_DIR_NONE, 1, VPRC_OUT_FMT);
	if (ret != HD_OK) {
		printf("set proc fail=%d\n", ret);
		goto exit;
	}

	#if (IME_POSTSHARPEN_ENABLE)
	ret = set_proc_param(stream[0].proc_path_3dnr, &main_dim, HD_VIDEO_DIR_NONE, 1, VPRC_OUT_FMT);
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

	if (stream[0].vprc_output == VPRC_OUTPUT_VENC) {
		// set videoenc config (main)
		stream[0].enc_max_dim.w = main_dim.w;
		stream[0].enc_max_dim.h = main_dim.h;
		ret = set_enc_cfg(stream[0].enc_path, &stream[0].enc_max_dim, enc_bitrate * 1024 * 1024, ISP_ID_0);
		if (ret != HD_OK) {
			printf("set enc-cfg fail=%d\n", ret);
			goto exit;
		}

		// set videoenc parameter (main)
		stream[0].enc_dim.w = main_dim.w;
		stream[0].enc_dim.h = main_dim.h;
		ret = set_enc_param(stream[0].enc_path, &stream[0].enc_dim, VPRC_OUT_FMT, enc_type, enc_bitrate * 1024 * 1024);
		if (ret != HD_OK) {
			printf("set enc fail=%d\n", ret);
			goto exit;
		}
	}


	if (stream[0].fix_pattern == FALSE) {
		if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
			// bind video_liveview modules (pre-process)
			hd_videocap_bind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);
			// bind video_liveview modules (main)
			hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOPROC_3_IN_0);
		} else {
			// bind video_liveview modules (main)
			hd_videocap_bind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);
			//hd_videoproc_bind(SOURCE_PATH, ...); //src_path is not used
		}
	}

	// bind video_liveview modules (sub2)
	//.... bind EXTEND_PATH2 to network

	//start network
	network_start(&stream[0]);

	if (second_ai) {
		network_start(&stream[2]);
	}

	{
		stream[0].net_proc_start = 0;
		stream[0].net_proc_exit = 0;
		stream[0].net_proc_oneshot = 0;

		stream[1].net_proc_start = 0;
		stream[1].net_proc_exit = 0;
		stream[1].net_proc_oneshot = 0;

		ret = pthread_create(&stream[0].net_proc_thread_id, NULL, network_proc_thread, (VOID*)(&stream[0]));
		if (ret < 0) {
			return HD_ERR_FAIL;
		}

		stream[0].net_proc_start = 1;
		stream[0].net_proc_exit = 0;
		stream[0].net_proc_oneshot = 0;

		stream[1].net_proc_start = 1;
		stream[1].net_proc_exit = 0;
		stream[1].net_proc_oneshot = 0;
	}

	// start video_liveview modules (main)
	if (stream[0].fix_pattern == FALSE) {
		hd_videocap_start(stream[0].cap_path);
	}
	hd_videoproc_start(stream[0].proc_path);
	#if (IME_POSTSHARPEN_ENABLE)
	hd_videoproc_start(stream[0].proc_path_3dnr);
	#endif

	if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
		hd_videoproc_start(stream[0].proc_path_pre);
	}

	#if AI_ENABLE
	if ((ret = network_bind_cb(&stream[0])) != HD_OK) {

		stream[0].net_proc_exit = 1;
		stream[1].net_proc_exit = 1;
		// quit program
		goto exit;
	}
	ret = network_set_isp_ai_cfg(&stream[0], 0, TRUE);

	if (!stream[0].fix_pattern) {
		if (ret != HD_OK) {

			stream[0].net_proc_exit = 1;
			stream[1].net_proc_exit = 1;
			// quit program
			printf("set isp_ai fail=%d\n", ret);
			goto exit;
		}
	}
	#endif

	if (stream[0].fix_pattern) {
		load_input_raw(&stream[0]);
	}

	// start video_liveview modules (sub)
	if (out_type <= 2 && stream[0].vprc_output == VPRC_OUTPUT_VOUT) {
		hd_videoout_start(stream[0].out_path);
	}

	if (stream[0].vprc_output == VPRC_OUTPUT_VENC) {
		// start video_liveview modules (main)
		hd_videoenc_start(stream[0].enc_path);

		system("nvtrtspd_ipc &");
		printf("nvtrtspd_ipc\r\n");
	}

	printf("\r\nEnter q to exit, Enter d to debug\r\n");
	do {
		key = getchar();
		if (key == 'q' || key == 0x3) {
			PD_SHM_INFO  *p_pd_shm;

			stream[0].net_proc_exit = 1;
			stream[1].net_proc_exit = 1;
			// quit program
			// notify child process
			p_pd_shm = (PD_SHM_INFO  *)g_shm;
			p_pd_shm->exit = 1;
			g_quit = 1; // stop thread
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
			system("cat /proc/nvt_info/nvt_clk/clk_dump_info");
			system("cat /sys/kernel/debug/clk/clk_summary");
		}

		if (key == 's') {
			if (stream[0].fix_pattern) {
				push_input_raw(&stream[0]);
				printf("push fix pattern to vprc\r\n");
			}
		}

		if (key == 'n') {
			system("echo w cfg 0 1000 > /proc/nvt_drv_sys/dram_info");
			system("echo w start 0 > /proc/nvt_drv_sys/dram_info");

			printf("start DDR\r\n");
		}

		if (key == 'm') {
			system("echo w stop 0 > /proc/nvt_drv_sys/dram_info");
			printf("stop DDR\r\n");
		}

		#if AI_ENABLE
		if (key == '1') {
			NET_PROC* p_net ;
			UINT32 proc_id ;
			VIDEO_LIVEVIEW *p_stream;
			if (curr_ai_path == 0) {
				p_stream = &stream[0];
			} else {
				p_stream = &stream[2];
			}
			p_net = g_net + p_stream->net_path ;
			proc_id = p_net->proc_id;

			ret = network_set_buf_by_in_path_list(p_stream, 0);
			if (HD_OK != ret) {
				printf("net_path(%u), proc_id(%u), para_id(%u) network_set_buf_by_in_path_list fail !!\n", p_stream->net_path, proc_id, 0);
			}
			printf("set AI default parameter\r\n");
		}

		if (key == '2') {
			NET_PROC* p_net ;
			UINT32 proc_id ;
			VIDEO_LIVEVIEW *p_stream;
			if (curr_ai_path == 0) {
				p_stream = &stream[0];
			} else {
				p_stream = &stream[2];
			}
			p_net = g_net + p_stream->net_path ;
			proc_id = p_net->proc_id;

			ret = network_set_buf_by_in_path_list(p_stream, 1);
			if (HD_OK != ret) {
				printf("net_path(%u), proc_id(%u), para_id(%u) network_set_buf_by_in_path_list fail !!\n", p_stream->net_path, proc_id, 0);
			}
			printf("set AI disable 3D\r\n");

		}

		if (key == '3') {
			NET_PROC* p_net ;
			UINT32 proc_id ;
			VIDEO_LIVEVIEW *p_stream;
			if (curr_ai_path == 0) {
				p_stream = &stream[0];
			} else {
				p_stream = &stream[2];
			}
			p_net = g_net + p_stream->net_path ;
			proc_id = p_net->proc_id;
			ret = network_set_buf_by_in_path_list(p_stream, 2);
			if (HD_OK != ret) {
				printf("net_path(%u), proc_id(%u), para_id(%u) network_set_buf_by_in_path_list fail !!\n", stream[0].net_path, proc_id, 0);
			}
			printf("set AI small noise profile\r\n");

		}
		#endif

		if (key == 'o') {
			#if AI_ENABLE
			VIDEO_LIVEVIEW *p_stream;
			if (curr_ai_path == 0) {
				p_stream = &stream[0];
			} else {
				p_stream = &stream[2];
			}
			ret = network_set_isp_ai_cfg(p_stream, 0, TRUE);
			if (!stream[0].fix_pattern) {
				if (ret != HD_OK) {
					printf("set isp_ai fail=%d\n", ret);
					return HD_ERR_NG;
				}
			}
			printf("enable isp ai cfg\r\n");
			#else
			printf("AI is disabled\r\n");
			#endif
		}

		if (key == 'p') {
			#if AI_ENABLE
			VIDEO_LIVEVIEW *p_stream;
			if (curr_ai_path == 0) {
				p_stream = &stream[0];
			} else {
				p_stream = &stream[2];
			}
			ret = network_set_isp_ai_cfg(p_stream, 0, FALSE);
			if (!stream[0].fix_pattern) {
				if (ret != HD_OK) {
					printf("set isp_ai fail=%d\n", ret);
					return HD_ERR_NG;
				}
			}
			printf("disable isp ai cfg\r\n");
			#else
			printf("AI is disabled\r\n");
			#endif
		}
		if (key == 'c') {
			#if (AI_ENABLE)

			if (second_ai) {
				VIDEO_LIVEVIEW *p_stream;

				if (curr_ai_path == 0) {
					p_stream = &stream[2];
					curr_ai_path = 1;
				} else {
					p_stream = &stream[0];
					curr_ai_path = 0;
				}

				ret = network_set_isp_ai_cfg(p_stream, 0, TRUE);
				if (!stream[0].fix_pattern) {
					if (ret != HD_OK) {
						printf("set isp_ai fail=%d\n", ret);
						return HD_ERR_NG;
					}
				}

				printf("enable isp ai cfg %d\r\n", curr_ai_path);
			}
			#else
			printf("AI is disabled\r\n");
			#endif
		}
		// normal flow ¤£Â_ stop/start/set input
		if (key == 'b') {
			#if AI_ENABLE

			stream[0].net_proc_exit = 1;
			stream[1].net_proc_exit = 1;
			// quit program


			//printf("111\r\n");
			//usleep(100000);

			ret = network_set_isp_ai_cfg(&stream[0], 0, FALSE);
			if (ret != HD_OK) {
				printf("set isp_ai fail=%d\n", ret);
				return HD_ERR_NG;
			}

			//printf("222\r\n");
			//usleep(100000);

			network_stop(&stream[0]);


			//printf("disable isp ai cfg\r\n");
			//usleep(100000);



			network_start(&stream[0]);
			{
				stream[0].net_proc_start = 0;
				stream[0].net_proc_exit = 0;
				stream[0].net_proc_oneshot = 0;

				stream[1].net_proc_start = 0;
				stream[1].net_proc_exit = 0;
				stream[1].net_proc_oneshot = 0;

				ret = pthread_create(&stream[0].net_proc_thread_id, NULL, network_proc_thread, (VOID*)(&stream[0]));
				if (ret < 0) {
					return HD_ERR_FAIL;
				}

				stream[0].net_proc_start = 1;
				stream[0].net_proc_exit = 0;
				stream[0].net_proc_oneshot = 0;

				stream[1].net_proc_start = 1;
				stream[1].net_proc_exit = 0;
				stream[1].net_proc_oneshot = 0;
			}


			//printf("333\r\n");
			//usleep(100000);

			ret = network_set_isp_ai_cfg(&stream[0], 0, TRUE);
			if (ret != HD_OK) {
				printf("set isp_ai fail=%d\n", ret);
				return HD_ERR_NG;
			}
			printf("enable isp ai cfg\r\n");
			usleep(100000);
			#else
			printf("AI is disabled\r\n");
			#endif
		}

		//normal flow ¤£Â_ stop/close/open/start/set input
		if (key == 'v') {
			#if AI_ENABLE

			stream[0].net_proc_exit = 1;
			stream[1].net_proc_exit = 1;
			// quit program
			usleep(100000);

			ret = network_set_isp_ai_cfg(&stream[0], 0, FALSE);
			if (ret != HD_OK) {
				printf("set isp_ai fail=%d\n", ret);
				return HD_ERR_NG;
			}

			// stop video_liveview modules (main)
			if (stream[0].fix_pattern == FALSE) {
				hd_videocap_stop(stream[0].cap_path);
			}
			hd_videoproc_stop(stream[0].proc_path);
			if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
				hd_videoproc_stop(stream[0].proc_path_pre);
			}

			#if (IME_POSTSHARPEN_ENABLE)
			hd_videoproc_stop(stream[0].proc_path_3dnr);
			#endif

			// stop video_liveview modules (sub)
			if (out_type <= 2 && stream[0].vprc_output == VPRC_OUTPUT_VOUT) {
				hd_videoout_stop(stream[0].out_path);
			}

			network_stop(&stream[0]);

			// destroy aquire_thread
			//pthread_join(stream[0].aquire_thread_id, (void* )NULL);

			if (stream[0].fix_pattern == FALSE) {
				// unbind video_liveview modules (main)
				hd_videocap_unbind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0));
			}
			if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
				hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_0);
			}

			// close video_liveview modules (main)
			ret = close_module(&stream[0]);
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



			// open video_liveview modules (main)
			ret = open_module(&stream[0], &stream[0].proc_max_dim);
			if (ret != HD_OK) {
				printf("open fail=%d\n", ret);
				goto exit;
			}


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

			if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
				// set videoproc parameter (pre-process)
				ret = set_proc_param(stream[0].proc_path_pre, &main_dim, HD_VIDEO_DIR_NONE, 1, HD_VIDEO_PXLFMT_RAW12 | HD_VIDEO_PIX_RGGB_R);
				if (ret != HD_OK) {
					printf("set proc-pre fail=%d\n", ret);
					goto exit;
				}
			}

			// set videoproc parameter (main)
			ret = set_proc_param(stream[0].proc_path, &main_dim, HD_VIDEO_DIR_NONE, 1, VPRC_OUT_FMT);
			if (ret != HD_OK) {
				printf("set proc fail=%d\n", ret);
				goto exit;
			}

			#if (IME_POSTSHARPEN_ENABLE)
			ret = set_proc_param(stream[0].proc_path_3dnr, &main_dim, HD_VIDEO_DIR_NONE, 1, VPRC_OUT_FMT);
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
				if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
					// bind video_liveview modules (pre-process)
					hd_videocap_bind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);
					// bind video_liveview modules (main)
					hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOPROC_3_IN_0);
				} else {
					// bind video_liveview modules (main)
					hd_videocap_bind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);
					//hd_videoproc_bind(SOURCE_PATH, ...); //src_path is not used
				}
			}

			// bind video_liveview modules (sub2)
			//.... bind EXTEND_PATH2 to network

			//start network
			network_start(&stream[0]);
			{
				stream[0].net_proc_start = 0;
				stream[0].net_proc_exit = 0;
				stream[0].net_proc_oneshot = 0;

				stream[1].net_proc_start = 0;
				stream[1].net_proc_exit = 0;
				stream[1].net_proc_oneshot = 0;

				ret = pthread_create(&stream[0].net_proc_thread_id, NULL, network_proc_thread, (VOID*)(&stream[0]));
				if (ret < 0) {
					return HD_ERR_FAIL;
				}

				stream[0].net_proc_start = 1;
				stream[0].net_proc_exit = 0;
				stream[0].net_proc_oneshot = 0;

				stream[1].net_proc_start = 1;
				stream[1].net_proc_exit = 0;
				stream[1].net_proc_oneshot = 0;
			}

			// start video_liveview modules (main)
			if (stream[0].fix_pattern == FALSE) {
				hd_videocap_start(stream[0].cap_path);
			}
			if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
				hd_videoproc_start(stream[0].proc_path_pre);
			}
			hd_videoproc_start(stream[0].proc_path);
			#if (IME_POSTSHARPEN_ENABLE)
			hd_videoproc_start(stream[0].proc_path_3dnr);
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
				goto exit;
			}
			#endif

			// start video_liveview modules (sub)
			if (out_type <= 2 && stream[0].vprc_output == VPRC_OUTPUT_VOUT) {
				hd_videoout_start(stream[0].out_path);
			}
			printf("enable isp ai cfg\r\n");
			#else
			printf("AI is disabled\r\n");
			#endif
		}
		if (key == 'f') {
			copy_info[0].copy_size = 0x500000;
			copy_info[0].ddr_id = 0;
			copy_info[0].sleep_us = sleep_us;
			copy_info[0].fps = engine_cfg.dmacpy1_fps;

			if (TRUE == is_start_dmacpy_test) {
				continue;
			}
			printf("start dmacpy test1, fps %d\r\n", copy_info[0].fps);
			ret = pthread_create(&dmacpy_thread_id, NULL, dmacpy_thread, &copy_info[0]);
			if (ret < 0) {
				printf("create dmacpy thread failed");
				goto exit;
			}
			is_start_dmacpy_test = TRUE;
		}

		if (key == 'g') {
			copy_info[0].copy_size = 0x500000;
			copy_info[0].ddr_id = 0;
			copy_info[0].sleep_us = sleep_us;
			copy_info[0].fps = engine_cfg.memcpy1_fps;

			if (TRUE == is_start_memcpy_test) {
				continue;
			}
			printf("start memcpy test1, fps %d\r\n", copy_info[0].fps);
			ret = pthread_create(&memcpy_thread_id, NULL, memcpy_thread, &copy_info[0]);
			if (ret < 0) {
				printf("create memcpy thread failed");
				goto exit;
			}
			is_start_memcpy_test = TRUE;
		}
		if (key == 'h') {
			copy_info[1].copy_size = 0x800000;
			copy_info[1].ddr_id = 0;
			copy_info[1].sleep_us = sleep_us;
			copy_info[1].fps = engine_cfg.grph_fps;

			if (TRUE == is_start_gfxcpy_test) {
				continue;
			}
			printf("start gfxcpy test, fps = %d\r\n", copy_info[1].fps);
			ret = pthread_create(&gfxcpy_thread_id, NULL, gfxcpy_thread, &copy_info[1]);
			if (ret < 0) {
				printf("create gfxcpy thread failed");
				goto exit;
			}
			is_start_gfxcpy_test = TRUE;
		}
		if (key == 'i') {
			copy_info[2].copy_size = 0x500000;
			copy_info[2].ddr_id = 0;
			copy_info[2].sleep_us = sleep_us;
			copy_info[2].fps = engine_cfg.memcpy2_fps;

			if (TRUE == is_start_memcpy_test2) {
				continue;
			}
			printf("start memcpy test2, fps = %d\r\n", copy_info[2].fps);
			ret = pthread_create(&memcpy_thread_id2, NULL, memcpy_thread, &copy_info[2]);
			if (ret < 0) {
				printf("create memcpy thread2 failed");
				goto exit;
			}
			is_start_memcpy_test2 = TRUE;
		}
		if (key == 'j') {
			copy_info[3].copy_size = 0x800000;
			copy_info[3].ddr_id = 0;
			copy_info[3].sleep_us = sleep_us;
			copy_info[3].fps = engine_cfg.ise_fps;

			if (TRUE == is_start_gfxscale_test) {
				continue;
			}
			printf("start gfxscale test, fps = %d\r\n", copy_info[3].fps);
			ret = pthread_create(&gfxscale_thread_id, NULL, gfxscale_thread, &copy_info[3]);
			if (ret < 0) {
				printf("create gfxscale test failed");
				goto exit;
			}
			is_start_gfxscale_test = TRUE;
		}
	} while(1);

	// stop video_liveview modules (main)
	if (stream[0].fix_pattern == FALSE) {
		hd_videocap_stop(stream[0].cap_path);
	}
	hd_videoproc_stop(stream[0].proc_path);
	if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
		hd_videoproc_stop(stream[0].proc_path_pre);
	}

	#if (IME_POSTSHARPEN_ENABLE)
	hd_videoproc_stop(stream[0].proc_path_3dnr);
	#endif

	// stop video_liveview modules (sub)
	if (out_type <= 2 && stream[0].vprc_output == VPRC_OUTPUT_VOUT) {
		hd_videoout_stop(stream[0].out_path);
	}

	if (stream[0].vprc_output == VPRC_OUTPUT_VENC) {
		hd_videoenc_stop(stream[0].enc_path);
	}
	if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
		hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_0);
	}

	{
		if (stream[0].net_cap_thread_id) {
			pthread_join(stream[0].net_cap_thread_id, NULL);
		}

		if (stream[0].net_proc_thread_id) {
			pthread_join(stream[0].net_proc_thread_id, NULL);
		}

		if (stream[0].net_proc_sub_thread_id) {
			pthread_join(stream[0].net_proc_sub_thread_id, NULL);
		}
	}

	network_stop(&stream[0]);

	if (second_ai) {
		network_stop(&stream[2]);
	}
	// destroy aquire_thread
	//pthread_join(stream[0].aquire_thread_id, (void* )NULL);

	if (stream[0].fix_pattern == FALSE) {
		// unbind video_liveview modules (main)
		hd_videocap_unbind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0));
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

	// close video_liveview modules (sub)
	if (out_type <= 2 && stream[0].vprc_output == VPRC_OUTPUT_VOUT) {
		ret = close_module_2(&stream[0]);
		if (ret != HD_OK) {
			printf("close2 fail=%d\n", ret);
		}
	}

	if (stream[0].vprc_output == VPRC_OUTPUT_VENC) {
		ret = close_module_3(&stream[0]);
		if (ret != HD_OK) {
			printf("close3 fail=%d\n", ret);
		}
	}
	if (second_ai) {
		close_module_5(&stream[2]);
	}

	// uninit all modules
	ret = exit_module(out_type);
	if (ret != HD_OK) {
		printf("exit fail=%d\n", ret);
	}

	if (stream[0].fix_pattern) {
		release_input_raw(&stream[0]);
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
