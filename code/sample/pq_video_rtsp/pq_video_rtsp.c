/**
	@brief Sample code of video rtsp.\n

	@file pq_video_rtsp.c

	@author Photon Lin

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <pthread.h>

#include "hdal.h"
#include "hd_debug.h"
#include "vendor_isp.h"
#include "vendor_common.h"
#include "vendor_videocapture.h"
#include "vendor_videoprocess.h"
#include "pd_shm.h"

///////////////////////////////////////////////////////////////////////////////
//function control
#define VPRC_PATH5_ENABLE          1   //must set to 1 when using ime sharpen
#define DIRECT_ENABLE              0   //0: dram mode; 1: direct mode
#define SAVE_RAW_ENABLE            1
#define EXTEND_YUV_ENABLE          0

#define PLUG_NNSC_FUNC             0
#define PLUG_OSG_FUNC              0
#define NNSC_MODEL_SIZE            15319040 //NNSC_NET_MODEL_SIZE_V_3_01_000
#define VENDOR_AI_CFG              0x000F0000

#define VIDEOCAP_ALG_FUNC HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB
#define VIDEOPROC_ALG_FUNC HD_VIDEOPROC_FUNC_AF | HD_VIDEOPROC_FUNC_WDR | HD_VIDEOPROC_FUNC_DEFOG | HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA
///////////////////////////////////////////////////////////////////////////////
//sensor size list
#define VDO_SIZE_W_12M     4048
#define VDO_SIZE_H_12M     3040
#define VDO_SIZE_W_8M      3840
#define VDO_SIZE_H_8M      2160
#define VDO_SIZE_W_5M      2592
#define VDO_SIZE_H_5M      1944
#define VDO_SIZE_W_5M2     2880
#define VDO_SIZE_H_5M2     1620
#define VDO_SIZE_W_4M      2560
#define VDO_SIZE_H_4M      1440
#define VDO_SIZE_W_4M2     2688
#define VDO_SIZE_H_4M2     1520
#define VDO_SIZE_W_GC5603  2960
#define VDO_SIZE_H_GC5603  1664
#define VDO_SIZE_W_2M      1920
#define VDO_SIZE_H_2M      1080

///////////////////////////////////////////////////////////////////////////////
//sensor selection list
typedef enum _SEN_SEL {
	SEN_SEL_NONE                   =  0,
	SEN_SEL_IMX290                 =  1,
	SEN_SEL_OS02K10                =  2,
	SEN_SEL_OS04A10                =  3,
	SEN_SEL_OS04C10                =  4,
	SEN_SEL_OS05A10                =  5,
	SEN_SEL_SC450AI                =  6,
	SEN_SEL_SC500AI                =  7,
	SEN_SEL_SC835HAI               =  8,
	SEN_SEL_IMX678                 =  9,
	SEN_SEL_SC850SL                = 10,
	SEN_SEL_IMX415                 = 11,
	SEN_SEL_GC5603                 = 12,
	SEN_SEL_AR0544                 = 13,
	// NOTE: Do not modify SEN_SEL_MAX and SEN_SEL_PATGEN
	SEN_SEL_MAX,
	SEN_SEL_PATGEN                 = 99,
	ENUM_DUMMY4WORD(SEN_SEL)
} SEN_SEL;

///////////////////////////////////////////////////////////////////////////////
//sensor list
#define ISP_DEFAULT_CFG "isp_os02k10_0"

typedef struct _SENSOR_INFO_ {
	UINT32 sensor_num;
	USIZE size;
	CHAR sensor_name[32];
	CHAR isp_cfg_name[32];
} SENSOR_INFO;

SENSOR_INFO sensor_info[] = {
//        number                 size                              name                       cfg(/mnt/app/isp/xxx.cfg)
/*  0 */{SEN_SEL_NONE,          {0, 0},                            "NULL",                    "NULL"                 },
/*  1 */{SEN_SEL_IMX290,        {VDO_SIZE_W_2M, VDO_SIZE_H_2M},    "nvt_sen_imx290",          ISP_DEFAULT_CFG        },
/*  2 */{SEN_SEL_OS02K10,       {VDO_SIZE_W_2M, VDO_SIZE_H_2M},    "nvt_sen_os02k10",         "isp_os02k10_0"        },
/*  3 */{SEN_SEL_OS04A10,       {VDO_SIZE_W_4M2, VDO_SIZE_H_4M2},  "nvt_sen_os04a10",         "isp_os04a10_0"        },
/*  4 */{SEN_SEL_OS04C10,       {VDO_SIZE_W_4M, VDO_SIZE_H_4M},    "nvt_sen_os04c10",         "isp_os04c10_0"        },
/*  5 */{SEN_SEL_OS05A10,       {VDO_SIZE_W_5M, VDO_SIZE_H_5M},    "nvt_sen_os05a10",         ISP_DEFAULT_CFG        },
/*  6 */{SEN_SEL_SC450AI,       {VDO_SIZE_W_4M2, VDO_SIZE_H_4M2},  "nvt_sen_sc450ai",         "isp_sc450ai_0"        },
/*  7 */{SEN_SEL_SC500AI,       {VDO_SIZE_W_5M2, VDO_SIZE_H_5M2},  "nvt_sen_sc500ai",         "isp_sc500ai_0"        },
/*  8 */{SEN_SEL_SC835HAI,      {VDO_SIZE_W_8M, VDO_SIZE_H_8M},    "nvt_sen_sc835hai",        ISP_DEFAULT_CFG        },
/*  9 */{SEN_SEL_IMX678,        {VDO_SIZE_W_8M, VDO_SIZE_H_8M},    "nvt_sen_imx678",          "isp_imx678_0"         },
/* 10 */{SEN_SEL_SC850SL,       {VDO_SIZE_W_8M, VDO_SIZE_H_8M},    "nvt_sen_sc850sl",         "isp_sc850sl_0"        },
/* 11 */{SEN_SEL_IMX415,        {VDO_SIZE_W_8M, VDO_SIZE_H_8M},    "nvt_sen_imx415",          "isp_imx415_0"         },
/* 12 */{SEN_SEL_GC5603,        {VDO_SIZE_W_GC5603, VDO_SIZE_H_GC5603}, "nvt_sen_gc5603",     "isp_gc5603_0"         },
/* 13 */{SEN_SEL_AR0544,        {VDO_SIZE_W_4M, VDO_SIZE_H_4M},    "nvt_sen_ar0544",          "isp_ar0544_0"         },

// NOTE: Do not modify SEN_SEL_PATGEN
/* 99 */{SEN_SEL_PATGEN,        {VDO_SIZE_W_2M, VDO_SIZE_H_2M},    "PATTERN_GEN",             ISP_DEFAULT_CFG        },
};

///////////////////////////////////////////////////////////////////////////////
//pixel start for patgen
#define PATGEN_START_PIX_R  HD_VIDEO_PIX_RGGB_R
#define PATGEN_START_PIX_GR HD_VIDEO_PIX_RGGB_GR
#define PATGEN_START_PIX_GB HD_VIDEO_PIX_RGGB_GB
#define PATGEN_START_PIX_B  HD_VIDEO_PIX_RGGB_B

#define PATGEN_START_PIX HD_VIDEO_PIX_RGGB_R

///////////////////////////////////////////////////////////////////////////////
//header
#define DBGINFO_BUFSIZE() (0x200)
//RAW
#define VDO_RAW_BUFSIZE(w, h, pxlfmt) (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
//CA for AWB
#define VDO_CA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 3) << 1)
//LA for AE
#define VDO_LA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 1) << 1)
//VA for AF
#define VDO_VA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 4) << 1)
//YUV
#define VDO_YUV_BUFSIZE(w, h, pxlfmt) (ALIGN_CEIL_16((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
//MD
#define MD_HEAD_BUFSIZE() (0x40)
//The md info w, h is vprc input w, h not out w, h
#define MD_INFO_BUFSIZE(w, h)           (ALIGN_CEIL_64(((((w >> 7) + 3) >> 2) << 2) * ((h + 15) >> 4)))

///////////////////////////////////////////////////////////////////////////////
// GC5603 sensor uses RAW8 format configuration (sensor outputs RAW10)
#define SEN_OUT_FMT            HD_VIDEO_PXLFMT_RAW8
#define CAP_OUT_FMT            HD_VIDEO_PXLFMT_RAW8
#define CAP_NRX_OUT_FMT        HD_VIDEO_PXLFMT_NRX12
#define SHDR2_CAP_OUT_FMT      HD_VIDEO_PXLFMT_RAW12_SHDR2
#define SHDR2_NRX_CAP_OUT_FMT  HD_VIDEO_PXLFMT_NRX12_SHDR2

#define SEN_VCAP_ID_0      0
#define SEN_VCAP_ID_1      1
#define SEN_VCAP_ID_2      2
#define SEN_VCAP_ID_3      3
#define SEN_VCAP_ID_4      4
#define SEN_VCAP_ID_5      5

#define HD_COMMON_CFG_MULTIPROC 0x80000000

static UINT32 sensor_sel_1 = 0;
static UINT32 sensor_sel_2 = 0;
static UINT32 sensor_shdr_1 = 0;
static UINT32 sensor_shdr_2 = 0;
static BOOL is_direct_mode = DIRECT_ENABLE;
static UINT32 vcap_fps = 30;
static UINT32 vcap_preset_expt = 0;
static UINT32 vcap_preset_gain = 0;
static UINT32 sen1_vcap_id = SEN_VCAP_ID_0;
static UINT32 sen2_vcap_id = SEN_VCAP_ID_2;
static UINT32 mipi_lane_manual = 0;
static BOOL hdr_8ev_enable = FALSE;

static UINT32 patgen_size_w = VDO_SIZE_W_2M, patgen_size_h = VDO_SIZE_H_2M;
static UINT32 vdo_size_w_1 = VDO_SIZE_W_2M, vdo_size_h_1 = VDO_SIZE_H_2M;
static UINT32 vdo_size_w_2 = VDO_SIZE_W_2M, vdo_size_h_2 = VDO_SIZE_H_2M;
static UINT32 vdo_size_w_m = 0, vdo_size_h_m = 0;
static BOOL vpe_enable_1 = TRUE, vpe_enable_2 = TRUE;
static BOOL is_nt98539a = FALSE;

///////////////////////////////////////////////////////////////////////////////
typedef struct _VIDEO_RECORD {
	// (1)
	HD_VIDEOCAP_SYSCAPS cap_syscaps;
	HD_PATH_ID cap_ctrl;
	HD_PATH_ID cap_path;

	HD_DIM  cap_dim;
	HD_DIM  proc_max_dim;

	// (2)
	HD_PATH_ID proc_ctrl;
	HD_PATH_ID proc_path;
	#if (VPRC_PATH5_ENABLE)
	HD_PATH_ID proc_path_3dnr;
	#endif

	// (3)
	HD_PATH_ID enc_path;
	HD_DIM  enc_max_dim;
	HD_DIM  enc_dim;

	// (4) user pull
	UINT32     flow_start;
	#if (SAVE_RAW_ENABLE)
	pthread_t  cap_thread_id;
	UINT32     cap_num;
	UINT32     cap_stop;
	UINT32     cap_exit;
	UINT32     cap_frame_num;
	#endif
} VIDEO_RECORD;

static char *g_shm = NULL;
static int g_shmid = 0;

#if (SAVE_RAW_ENABLE)
#define MAX_RAW_QUEUE  500
#define MAX_DDR_NUM    2
#define MAX_FRAME_NUM  2

#define SAVE_RAW_START 0x1
#define SAVE_RAW_WRITE 0x2
#define SAVE_RAW_END   0x4
#define SAVE_RAW_ALL   0x7

typedef struct _RAW_INFO {
	UINT64 vd_cnt;
	void *va;
	UINTPTR pa;
} RAW_INFO;

typedef struct _RAW_2A_INFO {
	UINT32 expt;
	UINT32 gain;
	UINT32 cg_r;
	UINT32 cg_g;
	UINT32 cg_b;
} RAW_2A_INFO;

RAW_2A_INFO raw_2a_info[2][MAX_FRAME_NUM][MAX_RAW_QUEUE] = {0};
RAW_INFO raw_info[2][MAX_FRAME_NUM][MAX_RAW_QUEUE] = {0};
UINT32 cap_idx[2][MAX_FRAME_NUM];
UINT32 cap_max_num = 0;
static UINT32 save_raw_cnt = 1;
static CHAR save_raw_folder[100] = "mnt/sd";
static BOOL save_raw_via_ddr = TRUE;

typedef struct _DDR_INFO {
	void *va;
	UINTPTR pa;
	UINT32 size;
} DDR_INFO;

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

#if (PLUG_OSG_FUNC)
static UINT32 osg_calc_buf_size(UINT32 osg_w, UINT32 osg_h)
{
	HD_VIDEO_FRAME frame = {0};
	int stamp_size;

	frame.sign   = MAKEFOURCC('O','S','G','P');
	frame.dim.w  = osg_w;
	frame.dim.h  = osg_h;
	frame.pxlfmt = HD_VIDEO_PXLFMT_ARGB4444;

	//get required buffer size for a single image
	stamp_size = hd_common_mem_calc_buf_size(&frame);
	if (!stamp_size) {
		printf("fail to query buffer size\n");
		return -1;
	}

	//ping pong buffer needs double size
	stamp_size *= 2;

	return stamp_size;
}
#endif

static HD_RESULT mem_init(void)
{
	HD_RESULT ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
	UINT32 pool_num = 0;

	// config common pool (cap)
	mem_cfg.pool_info[pool_num].type = HD_COMMON_MEM_COMMON_POOL;
	if (is_direct_mode) {
		mem_cfg.pool_info[pool_num].blk_size = DBGINFO_BUFSIZE() +
										VDO_CA_BUF_SIZE(ISP_CA_W_WINNUM, ISP_CA_H_WINNUM) +
										VDO_LA_BUF_SIZE(ISP_LA_W_WINNUM, ISP_LA_H_WINNUM);
	} else {
		mem_cfg.pool_info[pool_num].blk_size = DBGINFO_BUFSIZE() +
										VDO_RAW_BUFSIZE(vdo_size_w_1, vdo_size_h_1, CAP_OUT_FMT) +
										VDO_CA_BUF_SIZE(ISP_CA_W_WINNUM, ISP_CA_H_WINNUM) +
										VDO_LA_BUF_SIZE(ISP_LA_W_WINNUM, ISP_LA_H_WINNUM);
	}
	mem_cfg.pool_info[pool_num].blk_cnt = 9;
	mem_cfg.pool_info[pool_num].ddr_id = DDR_ID0;
	
	// config common pool
	pool_num++;  // 1
	mem_cfg.pool_info[pool_num].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[pool_num].blk_size = DBGINFO_BUFSIZE() + 
									VDO_YUV_BUFSIZE(vdo_size_w_1, vdo_size_h_1, HD_VIDEO_PXLFMT_YUV420) + 
									VDO_VA_BUF_SIZE(ISP_VA_W_WINNUM, ISP_VA_H_WINNUM) + 
									MD_HEAD_BUFSIZE() + MD_INFO_BUFSIZE(vdo_size_w_1, vdo_size_h_1);
	mem_cfg.pool_info[pool_num].blk_cnt = 6;
	mem_cfg.pool_info[pool_num].ddr_id = DDR_ID0;

	#if (PLUG_NNSC_FUNC)
	pool_num++;
	mem_cfg.pool_info[pool_num].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
	mem_cfg.pool_info[pool_num].blk_size = NNSC_MODEL_SIZE;
	mem_cfg.pool_info[pool_num].blk_cnt = 1;
	mem_cfg.pool_info[pool_num].ddr_id = DDR_ID0;
	#endif

	#if (PLUG_OSG_FUNC)
	pool_num++;
	mem_cfg.pool_info[pool_num].type = HD_COMMON_MEM_OSG_POOL;
	mem_cfg.pool_info[pool_num].blk_size = osg_calc_buf_size(vdo_size_w_1, vdo_size_h_1);
	mem_cfg.pool_info[pool_num].blk_cnt = 1;
	mem_cfg.pool_info[pool_num].ddr_id = DDR_ID0;
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
	UINT32 pinmux;

	snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN - 1, sensor_info[sensor_sel_1].sensor_name);
	printf("sensor 1: using %s \n", cap_cfg.sen_cfg.sen_dev.driver_name);

	pinmux = 0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux = pinmux;
	printf("sensor 1: pinmux = %d \n", pinmux);

	cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;

	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(0, 0);
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
	if (sensor_sel_2 != SEN_SEL_NONE) {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = HD_VIDEOCAP_SEN_IGNORE;
	} else {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
	}
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = HD_VIDEOCAP_SEN_IGNORE;

	if (sensor_shdr_1 == 1) {
		cap_cfg.sen_cfg.shdr_map = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR1, (HD_VIDEOCAP_0|HD_VIDEOCAP_1));
		printf("sensor 1: shdr map 0/1 \n");
	}

	ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(sen1_vcap_id), &video_cap_ctrl);
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
	iq_ctl.func = VIDEOCAP_ALG_FUNC;

	if (sensor_shdr_1 != 0) {
		iq_ctl.func |= HD_VIDEOCAP_FUNC_SHDR;
	}
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);

	*p_video_cap_ctrl = video_cap_ctrl;
	return ret;
}

static HD_RESULT set_cap2_cfg(HD_PATH_ID *p_video_cap_ctrl)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
	HD_PATH_ID video_cap_ctrl = 0;
	HD_VIDEOCAP_CTRL iq_ctl = {0};
	UINT32 pinmux;

	snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN - 1, sensor_info[sensor_sel_2].sensor_name);
	printf("sensor 2: using %s \n", cap_cfg.sen_cfg.sen_dev.driver_name);

	pinmux = 1;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux = pinmux;
	printf("sensor 2: pinmux = %d \n", pinmux);

	cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;

	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(2, 0);
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = HD_VIDEOCAP_SEN_IGNORE;


	if (sensor_shdr_2 != 0) {
		cap_cfg.sen_cfg.shdr_map = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR2, (HD_VIDEOCAP_2|HD_VIDEOCAP_3));
		printf("sensor 2: shdr map 2/3 \n");
	}

	ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(sen2_vcap_id), &video_cap_ctrl);
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
	iq_ctl.func = VIDEOCAP_ALG_FUNC;

	if (sensor_shdr_2 == 1) {
		iq_ctl.func |= HD_VIDEOCAP_FUNC_SHDR;
	}

	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);

	*p_video_cap_ctrl = video_cap_ctrl;
	return ret;
}

static HD_RESULT set_cap_param(HD_PATH_ID video_cap_path, HD_DIM *p_dim, UINT32 path)
{
	HD_RESULT ret = HD_OK;
	UINT32 color_bar_width = 200;
	{
		HD_VIDEOCAP_IN video_in_param = {0};
		if (sensor_info[sensor_sel_1].sensor_num != SEN_SEL_PATGEN) {
			video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO; //auto select sensor mode by the parameter of HD_VIDEOCAP_PARAM_OUT
			video_in_param.frc = HD_VIDEO_FRC_RATIO(vcap_fps,1);
			video_in_param.dim.w = p_dim->w;
			video_in_param.dim.h = p_dim->h;
			video_in_param.pxlfmt = SEN_OUT_FMT;
		} else {
			color_bar_width = (patgen_size_w >> 3) & 0xFFFFFFFE;
			video_in_param.sen_mode = HD_VIDEOCAP_PATGEN_MODE(HD_VIDEOCAP_SEN_PAT_COLORBAR, color_bar_width);
			video_in_param.frc = HD_VIDEO_FRC_RATIO(30,1);
			video_in_param.dim.w = p_dim->w;
			video_in_param.dim.h = p_dim->h;
		}

		if ((sensor_shdr_1 == 1) && (path == 0)) {
			video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_2;
		} else if ((sensor_shdr_2 == 1) && (path == 1)) {
			video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_2;
		} else {
			video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
		}

		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN, &video_in_param);
		if (ret != HD_OK) {
			return ret;
		}
	}
	//crop for GC5603: align width to 32 for BNR (2960->2944)
	if (sensor_sel_1 == SEN_SEL_GC5603) {
		HD_VIDEOCAP_CROP video_crop_param = {0};
		video_crop_param.mode = HD_CROP_ON;
		video_crop_param.win.rect.x = 8;
		video_crop_param.win.rect.y = 0;
		video_crop_param.win.rect.w = 2944;
		video_crop_param.win.rect.h = 1664;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);
	} else {
		HD_VIDEOCAP_CROP video_crop_param = {0};
		video_crop_param.mode = HD_CROP_OFF;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);
	}
	{
		HD_VIDEOCAP_OUT video_out_param = {0};

		if (is_direct_mode) {
			if (sensor_shdr_1 == 1) {
				video_out_param.pxlfmt = SHDR2_NRX_CAP_OUT_FMT;
			} else if (sensor_shdr_2 == 1) {
				video_out_param.pxlfmt = SHDR2_NRX_CAP_OUT_FMT;
			} else {
				video_out_param.pxlfmt = CAP_NRX_OUT_FMT;
			}
		} else {
			if (sensor_shdr_1 == 1) {
				video_out_param.pxlfmt = SHDR2_CAP_OUT_FMT;
			} else if (sensor_shdr_2 == 1) {
				video_out_param.pxlfmt = SHDR2_CAP_OUT_FMT;
			} else {
				video_out_param.pxlfmt = CAP_OUT_FMT;
			}
		}

		if (sensor_sel_1 == SEN_SEL_PATGEN) {
			video_out_param.pxlfmt |= PATGEN_START_PIX;
			printf("patgen pxlfmt: 0x%X \r\n", video_out_param.pxlfmt);
		}

		video_out_param.dir = HD_VIDEO_DIR_NONE;
		#if (SAVE_RAW_ENABLE | EXTEND_YUV_ENABLE)
		if (path == 0) {
			video_out_param.depth = sensor_shdr_1 + 1;
		} else if (path == 1) {
			video_out_param.depth = sensor_shdr_2 + 1;
		}
		#endif
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT, &video_out_param);
	}
	if (is_direct_mode) {
		HD_VIDEOCAP_FUNC_CONFIG video_path_param = {0};

		video_path_param.out_func = HD_VIDEOCAP_OUTFUNC_DIRECT;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_FUNC_CONFIG, &video_path_param);
	}

	if ((mipi_lane_manual == 2) || (sensor_sel_1 == SEN_SEL_OS02K10) || (sensor_sel_1 == SEN_SEL_OS05A10) || (sensor_sel_1 == SEN_SEL_AR0544) || (sensor_sel_2 != SEN_SEL_NONE)) {
		UINT32 data_lane = 2;
		vendor_videocap_set(video_cap_path, VENDOR_VIDEOCAP_PARAM_DATA_LANE, &data_lane);
	}

	return ret;
}

static HD_RESULT set_proc_cfg(HD_PATH_ID *p_video_proc_ctrl, HD_DIM* p_max_dim, HD_OUT_ID _out_id)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
	HD_VIDEOPROC_CTRL video_ctrl_param = {0};
	HD_PATH_ID video_proc_ctrl = 0;

	ret = hd_videoproc_open(0, _out_id, &video_proc_ctrl); //open this for device control
	if (ret != HD_OK)
		return ret;

	if (p_max_dim != NULL ) {
		if (((HD_CTRL_ID)_out_id == HD_VIDEOPROC_0_CTRL) && (sensor_sel_1 != SEN_SEL_NONE)) {
			video_cfg_param.pipe = HD_VIDEOPROC_PIPE_RAWALL;
			#if (EXTEND_YUV_ENABLE)
			video_cfg_param.isp_id = (1 << 31) | ((sen1_vcap_id & 0x7F) << 24) | (1 << 15) | ((sen1_vcap_id & 0x7F) << 8) | (sen1_vcap_id & 0xFF);
			#else
			video_cfg_param.isp_id = sen1_vcap_id;
			#endif
			video_cfg_param.ctrl_max.func = VIDEOPROC_ALG_FUNC;
			if (is_nt98539a) {
				video_cfg_param.ctrl_max.func |= (HD_VIDEOPROC_FUNC_BNR | HD_VIDEOPROC_FUNC_BNR_STA);
			}
		} else if (((HD_CTRL_ID)_out_id == HD_VIDEOPROC_1_CTRL) && (sensor_sel_2 != SEN_SEL_NONE)) {
			video_cfg_param.pipe = HD_VIDEOPROC_PIPE_RAWALL;
			video_cfg_param.isp_id = sen2_vcap_id;
			video_cfg_param.ctrl_max.func = VIDEOPROC_ALG_FUNC;
			if (is_nt98539a) {
				video_cfg_param.ctrl_max.func |= (HD_VIDEOPROC_FUNC_BNR | HD_VIDEOPROC_FUNC_BNR_STA);
			}
		} else if (((HD_CTRL_ID)_out_id == HD_VIDEOPROC_2_CTRL) && vpe_enable_1) {
			video_cfg_param.pipe = HD_VIDEOPROC_PIPE_VPE;
			video_cfg_param.isp_id = 0;
			video_cfg_param.ctrl_max.func = 0;
		} else if (((HD_CTRL_ID)_out_id == HD_VIDEOPROC_3_CTRL) && vpe_enable_2) {
			video_cfg_param.pipe = HD_VIDEOPROC_PIPE_VPE;
			video_cfg_param.isp_id = 1;
			video_cfg_param.ctrl_max.func = 0;
		} else {
			printf("set_proc_cfg _out_id incorrect %d \n", _out_id);
		}

		if (((sensor_shdr_1 != 0) && ((HD_CTRL_ID)_out_id == HD_VIDEOPROC_0_CTRL)) || ((sensor_shdr_2 != 0) && ((HD_CTRL_ID)_out_id == HD_VIDEOPROC_1_CTRL))) {
			video_cfg_param.ctrl_max.func |= HD_VIDEOPROC_FUNC_SHDR;
		} else {
			video_cfg_param.ctrl_max.func &= ~HD_VIDEOPROC_FUNC_SHDR;
		}
		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;

		if (is_direct_mode && (((HD_CTRL_ID)_out_id == HD_VIDEOPROC_0_CTRL) || ((HD_CTRL_ID)_out_id == HD_VIDEOPROC_1_CTRL))) {
			if (sensor_shdr_1 == 1) {
				video_cfg_param.in_max.pxlfmt = SHDR2_NRX_CAP_OUT_FMT;
			} else if (sensor_shdr_2 == 1) {
				video_cfg_param.in_max.pxlfmt = SHDR2_NRX_CAP_OUT_FMT;
			} else {
				video_cfg_param.in_max.pxlfmt = CAP_NRX_OUT_FMT;
			}
		} else {
			if (sensor_shdr_1 == 1) {
				video_cfg_param.in_max.pxlfmt = SHDR2_CAP_OUT_FMT;
			} else if (sensor_shdr_2 == 1) {
				video_cfg_param.in_max.pxlfmt = SHDR2_CAP_OUT_FMT;
			} else {
				video_cfg_param.in_max.pxlfmt = CAP_OUT_FMT;
			}
		}

		if (((HD_CTRL_ID)_out_id == HD_VIDEOPROC_2_CTRL) && vpe_enable_1) {
			video_cfg_param.in_max.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
		}

		if (((HD_CTRL_ID)_out_id == HD_VIDEOPROC_3_CTRL) && vpe_enable_2) {
			video_cfg_param.in_max.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
		}

		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
		if (ret != HD_OK) {
			return HD_ERR_NG;
		}
	}
	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		video_path_param.in_func = 0;
		if (is_direct_mode && (((HD_CTRL_ID)_out_id == HD_VIDEOPROC_0_CTRL) || ((HD_CTRL_ID)_out_id == HD_VIDEOPROC_1_CTRL))) {
			video_path_param.in_func |= HD_VIDEOPROC_INFUNC_DIRECT;
		}
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}

	if ((HD_CTRL_ID)_out_id == HD_VIDEOPROC_0_CTRL) {
		video_ctrl_param.func = VIDEOPROC_ALG_FUNC;
		if (is_nt98539a) {
			video_ctrl_param.func |= (HD_VIDEOPROC_FUNC_BNR | HD_VIDEOPROC_FUNC_BNR_STA);
		}
	} else if ((HD_CTRL_ID)_out_id == HD_VIDEOPROC_1_CTRL) {
		video_ctrl_param.func = VIDEOPROC_ALG_FUNC;
		if (is_nt98539a) {
			video_ctrl_param.func |= (HD_VIDEOPROC_FUNC_BNR | HD_VIDEOPROC_FUNC_BNR_STA);
		}
	} else if (((HD_CTRL_ID)_out_id == HD_VIDEOPROC_2_CTRL) && vpe_enable_1) {
		video_ctrl_param.func = 0;
	} else if (((HD_CTRL_ID)_out_id == HD_VIDEOPROC_3_CTRL) && vpe_enable_2) {
		video_ctrl_param.func = 0;
	}

	if ((HD_CTRL_ID)_out_id == HD_VIDEOPROC_0_CTRL) {
		#if (VPRC_PATH5_ENABLE)
		video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_0_OUT_4;
		#else
		video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_0_OUT_0;
		#endif
	} else if ((HD_CTRL_ID)_out_id == HD_VIDEOPROC_1_CTRL){
		#if (VPRC_PATH5_ENABLE)
		video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_1_OUT_4;
		#else
		video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_1_OUT_0;
		#endif
	}
	if (((sensor_shdr_1 != 0) && ((HD_CTRL_ID)_out_id == HD_VIDEOPROC_0_CTRL)) || ((sensor_shdr_2 != 0) && ((HD_CTRL_ID)_out_id == HD_VIDEOPROC_1_CTRL))) {
		video_ctrl_param.func |= HD_VIDEOPROC_FUNC_SHDR;
	} else {
		video_ctrl_param.func &= ~HD_VIDEOPROC_FUNC_SHDR;
	}

	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);

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
		#if (PLUG_NNSC_FUNC | PLUG_OSG_FUNC)
		video_out_param.depth = 1;
		#endif
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	}
	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		video_path_param.out_func |= HD_VIDEOPROC_OUTFUNC_MD;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}

	return ret;
}

static HD_RESULT set_enc_cfg(HD_PATH_ID video_enc_path, HD_DIM *p_max_dim, UINT32 max_bitrate, UINT32 isp_id)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOENC_PATH_CONFIG video_path_config = {0};
	HD_VIDEOENC_FUNC_CONFIG video_func_config = {0};

	if (p_max_dim != NULL) {
		//--- HD_VIDEOENC_PARAM_PATH_CONFIG ---
		video_path_config.max_mem.codec_type      = HD_CODEC_TYPE_H264;
		video_path_config.max_mem.max_dim.w       = p_max_dim->w;
		video_path_config.max_mem.max_dim.h       = p_max_dim->h;
		video_path_config.max_mem.bitrate         = max_bitrate;
		video_path_config.max_mem.enc_buf_ms      = 3000;
		video_path_config.max_mem.svc_layer       = HD_SVC_4X;
		video_path_config.max_mem.ltr             = TRUE;
		video_path_config.max_mem.rotate          = FALSE;
		video_path_config.max_mem.source_output   = FALSE;
		video_path_config.isp_id                  = isp_id;

		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_PATH_CONFIG, &video_path_config);
		if (ret != HD_OK) {
			printf("set_enc_path_config = %d \n", ret);
			return HD_ERR_NG;
		}

		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_FUNC_CONFIG, &video_func_config);
		if (ret != HD_OK) {
			printf("set_enc_func_config = %d \n", ret);
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
	HD_H26XENC_VUI vui = {0};

	if (p_dim != NULL) {
		//--- HD_VIDEOENC_PARAM_IN ---
		video_in_param.dir     = HD_VIDEO_DIR_NONE;
		video_in_param.pxl_fmt = HD_VIDEO_PXLFMT_YUV420;
		video_in_param.dim.w   = p_dim->w;
		video_in_param.dim.h   = p_dim->h;
		video_in_param.frc     = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_IN, &video_in_param);
		if (ret != HD_OK) {
			printf("set_enc_param_in = %d \n", ret);
			return ret;
		}

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
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &video_out_param);
			if (ret != HD_OK) {
				printf("set_enc_param_out = %d \n", ret);
				return ret;
			}

			//--- HD_VIDEOENC_PARAM_OUT_RATE_CONTROL ---
			rc_param.rc_mode             = HD_RC_MODE_CBR;
			rc_param.cbr.bitrate         = bitrate;
			rc_param.cbr.frame_rate_base = 30;
			rc_param.cbr.frame_rate_incr = 1;
			rc_param.cbr.init_i_qp       = 26;
			rc_param.cbr.min_i_qp        = 20;
			rc_param.cbr.max_i_qp        = 45;
			rc_param.cbr.init_p_qp       = 26;
			rc_param.cbr.min_p_qp        = 20;
			rc_param.cbr.max_p_qp        = 45;
			rc_param.cbr.static_time     = 4;
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL, &rc_param);
			if (ret != HD_OK) {
				printf("set_enc_rate_control = %d \n", ret);
				return ret;
			}

			vui.vui_en = TRUE;
			vui.color_range = 1; // 0: Not full range, 1: Full range
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_VUI , &vui);
			if (ret != HD_OK) {
				printf("set_enc_out_ui = %d \n", ret);
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
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &video_out_param);
			if (ret != HD_OK) {
				printf("set_enc_param_out = %d \n", ret);
				return ret;
			}

			//--- HD_VIDEOENC_PARAM_OUT_RATE_CONTROL ---
			rc_param.rc_mode             = HD_RC_MODE_CBR;
			rc_param.cbr.bitrate         = bitrate;
			rc_param.cbr.frame_rate_base = 30;
			rc_param.cbr.frame_rate_incr = 1;
			rc_param.cbr.init_i_qp       = 26;
			rc_param.cbr.min_i_qp        = 20;
			rc_param.cbr.max_i_qp        = 45;
			rc_param.cbr.init_p_qp       = 26;
			rc_param.cbr.min_p_qp        = 20;
			rc_param.cbr.max_p_qp        = 45;
			rc_param.cbr.static_time     = 4;
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL, &rc_param);
			if (ret != HD_OK) {
				printf("set_enc_rate_control = %d \n", ret);
				return ret;
			}

			vui.vui_en = TRUE;
			vui.color_range = 1; // 0: Not full range, 1: Full range
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_VUI , &vui);
			if (ret != HD_OK) {
				printf("set_enc_out_ui = %d \n", ret);
				return ret;
			}
		} else if (enc_type == 2) {
			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_JPEG;
			video_out_param.jpeg.retstart_interval = 0;
			video_out_param.jpeg.image_quality = bitrate / (1024 * 1024);
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &video_out_param);
			if (ret != HD_OK) {
				printf("set_enc_param_out = %d \n", ret);
				return ret;
			}
		} else {
			printf("not support enc_type \n");
			return HD_ERR_NG;
		}
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
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
		printf("set cap-cfg fail = %d \n", ret);
		return HD_ERR_NG;
	}
	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, HD_VIDEOPROC_0_CTRL);
	if (ret != HD_OK) {
		printf("set proc-cfg fail = %d \n", ret);
		return HD_ERR_NG;
	}

	if ((ret = hd_videocap_open(HD_VIDEOCAP_IN(sen1_vcap_id, 0), HD_VIDEOCAP_OUT(sen1_vcap_id, 0), &p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &p_stream->proc_path)) != HD_OK)
		return ret;
	#if (VPRC_PATH5_ENABLE)
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_4, &p_stream->proc_path_3dnr)) != HD_OK)
		return ret;
	#endif
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_VIDEOENC_0_OUT_0, &p_stream->enc_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT open_module_without_enc(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;

	// set videocap config
	ret = set_cap_cfg(&p_stream->cap_ctrl);
	if (ret != HD_OK) {
		printf("set cap-cfg fail = %d \n", ret);
		return HD_ERR_NG;
	}
	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, HD_VIDEOPROC_0_CTRL);
	if (ret != HD_OK) {
		printf("set proc-cfg fail = %d \n", ret);
		return HD_ERR_NG;
	}

	if ((ret = hd_videocap_open(HD_VIDEOCAP_IN(sen1_vcap_id, 0), HD_VIDEOCAP_OUT(sen1_vcap_id, 0), &p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &p_stream->proc_path)) != HD_OK)
		return ret;
	#if (VPRC_PATH5_ENABLE)
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_4, &p_stream->proc_path_3dnr)) != HD_OK)
		return ret;
	#endif

	return HD_OK;
}

static HD_RESULT open_module2(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;

	// set videocap config
	ret = set_cap2_cfg(&p_stream->cap_ctrl);
	if (ret != HD_OK) {
		printf("set cap-cfg fail = %d \n", ret);
		return HD_ERR_NG;
	}
	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, HD_VIDEOPROC_1_CTRL);
	if (ret != HD_OK) {
		printf("set proc-cfg fail = %d \n", ret);
		return HD_ERR_NG;
	}

	if ((ret = hd_videocap_open(HD_VIDEOCAP_IN(sen2_vcap_id, 0), HD_VIDEOCAP_OUT(sen2_vcap_id, 0), &p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_1_IN_0, HD_VIDEOPROC_1_OUT_0, &p_stream->proc_path)) != HD_OK)
		return ret;
	#if (VPRC_PATH5_ENABLE)
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_1_IN_0, HD_VIDEOPROC_1_OUT_4, &p_stream->proc_path_3dnr)) != HD_OK)
		return ret;
	#endif
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_1, HD_VIDEOENC_0_OUT_1, &p_stream->enc_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT open_module3(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;

	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, HD_VIDEOPROC_2_CTRL);
	if (ret != HD_OK) {
		printf("set proc-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}

	if ((ret = hd_videoproc_open(HD_VIDEOPROC_2_IN_0, HD_VIDEOPROC_2_OUT_0, &p_stream->proc_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT open_module4(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;

	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, HD_VIDEOPROC_3_CTRL);
	if (ret != HD_OK) {
		printf("set proc-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}

	if ((ret = hd_videoproc_open(HD_VIDEOPROC_3_IN_0, HD_VIDEOPROC_3_OUT_0, &p_stream->proc_path)) != HD_OK)
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

static HD_RESULT close_module2(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;

	if ((ret = hd_videoproc_close(p_stream->proc_path)) != HD_OK)
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

void init_share_memory(void)
{
	int g_shmid = 0;
	key_t key;

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
	// Initialization.
	memset(g_shm, 0, PD_SHMSZ );
}

void exit_share_memory(void)
{
	if (g_shm) {
		shmdt(g_shm);
		shmctl(g_shmid, IPC_RMID, NULL);
	}
}

#if (SAVE_RAW_ENABLE)
static void cap_raw_save(UINT32 frame_num, UINT32 frame_size, UINT32 ctl_flow)
{
	static CHAR dir_str[64];
	CHAR file_str[64], full_str[128], _2a_info_str[64];
	static FILE *fp = NULL, *fp2 = NULL, *fp3 = NULL;
	UINT32 i, j, k, path;
	struct stat stat_buf;
	BOOL is_dir_exists = TRUE;
	UINT32 cap_index = 0, file_index = 0;

	if (frame_size == 0) {
		printf("frame_size = 0 \n");
		return;
	}

	if (sensor_sel_2 == SEN_SEL_NONE) {
		path = 1;
	} else {
		path = 2;
	}

	// NOTE: step 1, prepare folder, txt, and bin.
	if (ctl_flow & SAVE_RAW_START) {
		while (is_dir_exists) {
			snprintf(dir_str, sizeof(dir_str) - 1, "/%s/%d_%s", save_raw_folder, save_raw_cnt, &sensor_info[sensor_sel_1].sensor_name[8]);
			if (stat((CHAR *)dir_str, &stat_buf) != 0) {
				is_dir_exists = FALSE;
			} else {
				printf("Directory %s exists \n", dir_str);
				save_raw_cnt++;
			}
		}
		snprintf(full_str, sizeof(full_str) - 1, "mkdir %s", dir_str);
		system(full_str);
		printf("Create directory %s \n", dir_str);

		// Open raw_2a_info.txt
		snprintf(full_str, sizeof(full_str) - 1, "%s/raw_2a_info.txt", dir_str);
		if ((fp2 = fopen(full_str, "wb")) != NULL) {
			printf("Create file %s \n", full_str);
		}

		// Open raw_2a_info.bin
		snprintf(full_str, sizeof(full_str) - 1, "%s/raw_2a_info.bin", dir_str);
		if ((fp3 = fopen(full_str, "wb")) != NULL) {
			printf("Create file %s \n", full_str);
		}
	}

	// NOTE: step 2, write raw , 2a info.
	if (ctl_flow & SAVE_RAW_WRITE) {
		for (k = 0; k < path; k++) {
			for (i = 0; i < frame_num; i++) {
				if (save_raw_via_ddr) {
					cap_index = cap_idx[k][i];
				} else {
					cap_index = 1;
				}
				for (j = 0; j < cap_index; j++) {
					if (save_raw_via_ddr) {
						file_index = j;
					} else {
						file_index = cap_idx[k][i];
					}
					snprintf(file_str, sizeof(file_str) - 1, "RAW%u_w%d_h%d_12b_pack_%u.raw", i + k * frame_num, (int)vdo_size_w_1, (int)vdo_size_h_1, file_index);
					snprintf(full_str, sizeof(full_str) - 1, "%s/%s", dir_str, file_str);
					printf("%s \n", file_str);
					//----- open output files -----
					if ((fp = fopen(full_str, "wb")) != NULL) {
						fwrite((UINT8 *)raw_info[k][i][j].va, 1, frame_size, fp);
						fflush(fp);
						fclose(fp);
						fp = NULL;
					} else {
						printf("open file (%s) fail \n\n", file_str);
						continue;
					}

					// Write raw_2a_info.txt
					snprintf(_2a_info_str, sizeof(_2a_info_str) - 1, "RAW%u: cnt %3u, expt %6d, gain %6d, cg %4d %4d %4d", i + k * frame_num, file_index, raw_2a_info[k][i][j].expt, raw_2a_info[k][i][j].gain, 
																													raw_2a_info[k][i][j].cg_r, raw_2a_info[k][i][j].cg_g, raw_2a_info[k][i][j].cg_b);
					if (fp2) {
						_2a_info_str[sizeof(_2a_info_str) - 1] = '\n';
						fwrite((UINT8 *)_2a_info_str, 1, sizeof(_2a_info_str), fp2);
					}

					// Write raw_2a_info.bin
					if (fp3) {
						fwrite((UINT8 *)&raw_2a_info[k][i][j].expt, 1, sizeof(RAW_2A_INFO), fp3);
					}
				}
			}
		}
	}

	// NOTE: step 3, close text, bin
	if (ctl_flow & SAVE_RAW_END) {
		if (fp2) {
			fflush(fp2);
			fclose(fp2);
			fp2 = NULL;
		}
		if (fp3) {
			fflush(fp3);
			fclose(fp3);
			fp3 = NULL;
		}

		save_raw_cnt++;

		system("sync");
		printf("Save raw finish \n");
	}
}

static void *cap_raw_thread(void *arg)
{
	VIDEO_RECORD *p_stream = (VIDEO_RECORD *)arg;
	HD_RESULT ret = HD_OK;
	HD_VIDEO_FRAME video_frame[2][MAX_FRAME_NUM] = {0};
	UINTPTR phy_addr_main[2][MAX_FRAME_NUM];
	UINT32 index[2], size[2] = {0};
	UINT32 i, j, path;
	HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;
	ISPT_SENSOR_EXPT sensor_expt = {0};
	ISPT_TOTAL_GAIN total_gain = {0};
	ISPT_C_GAIN c_gain = {0};
	UINT32 cap_index = 0, raw_size;
	uintptr_t phy_addr_raw = 0, vir_addr_raw[2][MAX_FRAME_NUM] = {0};

	if (sensor_sel_2 == SEN_SEL_NONE) {
		path = 1;
	} else {
		path = 2;
	}

	//------ wait flow_start ------
	while (p_stream[0].flow_start == 0) sleep(1);

	if (!save_raw_via_ddr) {
		cap_raw_save(p_stream[0].cap_frame_num, 1, SAVE_RAW_START);
	}

	//--------- pull data test ---------
	while (p_stream[0].cap_stop == 0) {
		for (j = 0; j < path; j++) {
			printf("path %d: ", j);
			for (i = 0; i < p_stream[j].cap_frame_num; i++) {
				ret = hd_videocap_pull_out_buf(p_stream[j].cap_path, &video_frame[j][i], -1);// -1 = blocking mode, 0 = non-blocking mode, >0 = blocking-timeout mode
				if (ret != HD_OK) {
					printf("pull_out (%d) error = %d \n", -1, ret);
					goto release_out;;
				}

				index[j] = video_frame[j][i].ddr_id >> 8;

				if (index[j] >= p_stream[j].cap_frame_num) {
					printf("something wrong (0x%X) \n", video_frame[j][i].pxlfmt);
					goto release_out;;
				}

				phy_addr_main[j][index[j]] = video_frame[j][i].phy_addr[0];
				if (phy_addr_main[j][index[j]] == 0) {
					printf("phy_addr_main[%d] error \n\n", index[j]);
					goto release_out;
				}

				sensor_expt.id = sen1_vcap_id;
				vendor_isp_get_common(ISPT_ITEM_SENSOR_EXPT, &sensor_expt);
				total_gain.id = sen1_vcap_id;
				vendor_isp_get_common(ISPT_ITEM_TOTAL_GAIN, &total_gain);
				c_gain.id = sen1_vcap_id;
				vendor_isp_get_common(ISPT_ITEM_C_GAIN, &c_gain);

				if (save_raw_via_ddr) {
					cap_index = cap_idx[j][index[j]];
				} else {
					cap_index = 0;
				}

				raw_info[j][index[j]][cap_index].vd_cnt = video_frame[j][i].count;

				raw_2a_info[j][index[j]][cap_index].expt = sensor_expt.time[0];
				raw_2a_info[j][index[j]][cap_index].gain = total_gain.gain;
				raw_2a_info[j][index[j]][cap_index].cg_r = c_gain.gain[0];
				raw_2a_info[j][index[j]][cap_index].cg_g = c_gain.gain[1];
				raw_2a_info[j][index[j]][cap_index].cg_b = c_gain.gain[2];
				printf("cnt %d, vd %lld, expt %d, iso %d, cg %d %d %d ", cap_idx[j][index[j]], raw_info[j][index[j]][cap_index].vd_cnt, 
												raw_2a_info[j][index[j]][cap_index].expt, raw_2a_info[j][index[j]][cap_index].gain, 
												raw_2a_info[j][index[j]][cap_index].cg_r, raw_2a_info[j][index[j]][cap_index].cg_g, raw_2a_info[j][index[j]][cap_index].cg_b);

				if (!save_raw_via_ddr) {
					phy_addr_raw = hd_common_mem_blk2pa(video_frame[j][i].blk);
					if (phy_addr_raw == 0) {
						printf("blk2pa fail, blk = 0x%x \n", video_frame[j][i].blk);
						goto release_out;
					}

					raw_size = DBGINFO_BUFSIZE() + video_frame[0][0].loff[0] * video_frame[0][0].ph[0];
					vir_addr_raw[j][i] = (ULONG)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_raw, raw_size);
					if (vir_addr_raw[j][i] == 0) {
						printf("vir_addr_raw mmap fail \n");
						goto release_out;
					}

					raw_info[j][i][cap_index].pa = video_frame[j][i].phy_addr[0];
					raw_info[j][i][cap_index].va = (void *)(vir_addr_raw[j][i] + (video_frame[j][i].phy_addr[0] - phy_addr_raw));
				}
			}
		}
		printf("\n");

		size[0] = video_frame[0][0].loff[0] * video_frame[0][0].ph[0];
		size[1] = video_frame[1][0].loff[0] * video_frame[1][0].ph[0];

		//copy raw
		if (save_raw_via_ddr) {
			for (j = 0; j < path; j++) {
				for (i = 0; i < p_stream[j].cap_frame_num; i++) {
					ret = mem_alloc_remain(&raw_info[j][i][cap_idx[j][i]].pa, (void **)&raw_info[j][i][cap_idx[j][i]].va, size[j], ddr_id);
					if (ret != HD_OK) {
						printf("ddr[%d] full, idx = %d \n", ddr_id, cap_idx[j][i]);
						ddr_id++;
						if (ddr_id < MAX_DDR_NUM) {
							ret = mem_alloc_remain(&raw_info[j][i][cap_idx[j][i]].pa, (void **)&raw_info[j][i][cap_idx[j][i]].va, size[j], ddr_id);
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

					if (hd_gfx_memcpy(raw_info[j][i][cap_idx[j][i]].pa, phy_addr_main[j][i], size[j]) == NULL) {
						printf("hd_gfx_memcpy fail \n");
					}

					cap_idx[j][i]++;
					if (cap_idx[j][p_stream[j].cap_frame_num - 1] >= p_stream[j].cap_num) {
						p_stream[0].cap_stop = 1;
						break;
					}
				}
			}
		} else {
			cap_raw_save(p_stream[0].cap_frame_num, size[0], SAVE_RAW_WRITE);

			for (j = 0; j < path; j++) {
				for (i = 0; i < p_stream[j].cap_frame_num; i++) {
					if (vir_addr_raw[j][i] != 0) {
						raw_size = DBGINFO_BUFSIZE() + video_frame[0][0].loff[0] * video_frame[0][0].ph[0];
						if (hd_common_mem_munmap((void *)vir_addr_raw[j][i], raw_size) != HD_OK) {
							printf("vir_addr_raw, mnumap fail \n");
						}
					}
				}
			}

			for (j = 0; j < path; j++) {
				for (i = 0; i < p_stream[j].cap_frame_num; i++) {
					cap_idx[j][i]++;
					if (cap_idx[j][p_stream[j].cap_frame_num - 1] >= p_stream[j].cap_num) {
						p_stream[0].cap_stop = 1;
						break;
					}
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

	if (save_raw_via_ddr) {
		cap_raw_save(p_stream[0].cap_frame_num, size[0], SAVE_RAW_ALL);
	} else {
		cap_raw_save(p_stream[0].cap_frame_num, size[0], SAVE_RAW_END);
	}
	p_stream[0].cap_exit = 1;

	return 0;
}
#endif

static INT32 pq_get_int(void)
{
	CHAR buf[256];
	INT val, rt;

	rt = scanf("%d", &val);

	if (rt != 1) {
		printf("Invalid option. Try again.\n");
		clearerr(stdin);
		fgets(buf, sizeof(buf), stdin);
		val = -1;
	}

	return val;
}

static void pq_menu(void)
{
	INT32 option;
	BOOL menu = TRUE;

	while (menu) {
		printf("----------------------------------------\n");
		printf("  10.  Set direct (%s) \n", is_direct_mode ? "true" : "false");
		printf("  11.  Set vpe enable (%s) \n", vpe_enable_1 ? "true" : "false");
		printf("  12.  Set vcap fps (%d) \n", vcap_fps);
		printf("  13.  Set vcap manual resolution (%dx%d) \n", vdo_size_w_m, vdo_size_h_m);
		printf("  14.  Set vcap preset (expt: %d, gain: %d) \n", vcap_preset_expt, vcap_preset_gain);
		printf("  15.  Set mipi lane number (%d) \n", mipi_lane_manual);
		printf("  16.  Set hdr 8ev enable (%s), 539A only \n", hdr_8ev_enable ? "true" : "false");
		printf("----------------------------------------\n");
		printf("  20.  Set save_raw_folder (%s) \n", save_raw_folder);
		printf("  21.  Set save_raw_cnt (%d) \n", save_raw_cnt);
		printf("  22.  Set save_raw_via_ddr (%s) \n", save_raw_via_ddr ? "true" : "false");
		printf("----------------------------------------\n");
		printf("   0.  Quit\n");
		printf("----------------------------------------\n");
		printf(">> ");

		option = pq_get_int();
		switch (option) {
			case 10:
				printf("Current %s \n", is_direct_mode ? "true" : "false");
				printf("Set direct_mode (0: disable, 1: enable)>> \n");
				is_direct_mode = (BOOL)pq_get_int();
				printf("Set to %s \n", is_direct_mode ? "true" : "false");
				break;

			case 11:
				printf("Current %s \n", vpe_enable_1 ? "true" : "false");
				printf("Set vpe_enable (0: disable, 1: enable)>> \n");
				vpe_enable_1 = (BOOL)pq_get_int();
				vpe_enable_2 = vpe_enable_1;
				printf("Set to %s \n", vpe_enable_1 ? "true" : "false");
				break;

			case 12:
				printf("Current %d \n", vcap_fps);
				printf("Set vcap_fps>> \n");
				vcap_fps = (UINT32)pq_get_int();
				printf("Set to %d \n", vcap_fps);
				break;

			case 13:
				printf("Current %dx%d \n", vdo_size_w_m, vdo_size_h_m);
				printf("Set vdo_size_w_m>> \n");
				vdo_size_w_m = (UINT32)pq_get_int();
				printf("Set vdo_size_h_m>> \n");
				vdo_size_h_m = (UINT32)pq_get_int();
				printf("Set to %dx%d \n", vdo_size_w_m, vdo_size_h_m);
				break;

			case 14:
				printf("Current expt %d, gain %d \n", vcap_preset_expt, vcap_preset_gain);
				printf("Set vcap_preset_expt (us)>> \n");
				vcap_preset_expt = (UINT32)pq_get_int();
				printf("Set vcap_preset_gain (1000 = 1X)>> \n");
				vcap_preset_gain = (UINT32)pq_get_int();
				printf("Set to expt %d, gain %d \n", vcap_preset_expt, vcap_preset_gain);
				break;

			case 15:
				printf("Current %d \n", mipi_lane_manual);
				printf("Set mipi_lane_manual (2, 4)>> \n");
				mipi_lane_manual = (UINT32)pq_get_int();
				if ((mipi_lane_manual == 2) || (mipi_lane_manual == 4)) {
					printf("Set to %d \n", mipi_lane_manual);
				} else {
					printf("Illegal (%d), Force to 2 \n", mipi_lane_manual);
					mipi_lane_manual = 2;
				}
				break;

			case 16:
				printf("Current %s \n", hdr_8ev_enable ? "true" : "false");
				printf("Set hdr_8ev_enable (0: disable, 1: enable)>> \n");
				hdr_8ev_enable = (BOOL)pq_get_int();
				printf("Set to %s \n", hdr_8ev_enable ? "true" : "false");
				break;

			case 20:
				printf("Current %s \n", save_raw_folder);
				printf("Set save_raw_folder >> \n");
				scanf("%s", save_raw_folder);
				printf("Set to %s \n", save_raw_folder);
			break;

			case 21:
				printf("Current %d \n", save_raw_cnt);
				printf("Set save_raw_cnt>> \n");
				save_raw_cnt = (UINT32)pq_get_int();
				printf("Set to %d \n", save_raw_cnt);
				break;

			case 22:
				printf("Current %s \n", save_raw_via_ddr ? "true" : "false");
				printf("Set pre-save raw enable (0: disable, 1: enable)>> \n");
				save_raw_via_ddr = (BOOL)pq_get_int();
				printf("Set to %s \n", save_raw_via_ddr ? "true" : "false");
				break;

			default:
				printf("wrong input (%d) \n", option);
				break;

			case 0:
				menu = FALSE;
				break;
		}
	}

	return;
}

int main(int argc, char** argv)
{
	HD_RESULT ret;
	INT key;
	VIDEO_RECORD stream[4] = {0};
	UINT32 enc_type = 0;
	UINT32 enc_bitrate = 4;
	HD_DIM main_dim;
	AET_CFG_INFO cfg_info = {0};
	AWBT_CT_TO_CGAIN ct_to_cgain = {0};
	ISPT_C_GAIN c_gain = {0};
	ISPT_CT ct = {0};
	UINT32 i;
	PD_SHM_INFO  *p_pd_shm;
	VENDOR_VIDEOCAP_AE_PRESET ae_preset = {0};

	if (argc == 1 || argc > 8) {
		printf("\033[1;37;43mUsage:\033[0m \033[1;32;40m<sen_sel_1 > \033[1;31;40m<sen_shdr_1> \033[1;32;40m<sen_sel_2 > \033[1;31;40m<sen_shdr_2> \033[1;37;40m<enc_type> <enc_bitrate>\033[0m. \n");
		printf("\n");
		printf("\033[1;32;40m  <sen_sel_1    > : refer below sensor list                                                \033[0m \n");
		printf("\033[1;31;40m  <sen_shdr_1   > : 0(disable), 1(2-frame), 2(3-frame)                                     \033[0m \n");
		printf("\033[1;32;40m  <sen_sel_2    > : refer below sensor list                                                \033[0m \n");
		printf("\033[1;31;40m  <sen_shdr_2   > : 0(disable), 1(2-frame), 2(3-frame)                                     \033[0m \n");
		printf("\033[1;37;40m  <enc_type     > : 0(H265), 1(H264), 2(MJPG)                                              \033[0m \n");
		printf("\033[1;37;40m  <enc_bitrate  > : Mbps for H265/H264, or Quality for MJPG                                \033[0m \n");

		printf("\033[1;32;40m  sensor list: \033[0m \n");
		printf(" ");
		for (i = 0; i < (sizeof(sensor_info) / sizeof(SENSOR_INFO)); i++) {
			if ((sensor_info[i].sensor_num == SEN_SEL_NONE) || (sensor_info[i].sensor_num == SEN_SEL_PATGEN)) {
				printf("\033[1;32;40m %2d(%12s),\033[0m ", sensor_info[i].sensor_num, sensor_info[i].sensor_name);
			} else {
				printf("\033[1;32;40m %2d(%12s),\033[0m ", sensor_info[i].sensor_num, &sensor_info[i].sensor_name[8]);
			}
			if ((i != 0) && ((i + 1) % 4 == 0)) {
				printf("\n");
				printf(" ");
			}
		}
		printf("\n");
		return 0;
	}

	// query program options
	if (argc > 1) {
		if (atoi(argv[1]) < SEN_SEL_MAX) {
			sensor_sel_1 = atoi(argv[1]);
		} else {
			if (atoi(argv[1]) == SEN_SEL_PATGEN) {
				sensor_sel_1 = SEN_SEL_MAX;
			} else {
				printf("sensor 1: sen_sel_1 not available \n");
				return 0;
			}
		}
		printf("sensor 1: %s \n", sensor_info[sensor_sel_1].sensor_name);
	}

	if (argc > 2) {
		sensor_shdr_1 = atoi(argv[2]);
		if (sensor_shdr_1 == 0) {
			printf("sensor 1: shdr off \n");
		} else if (sensor_shdr_1 == 1) {
			printf("sensor 1: shdr on, frame number = %d \n", sensor_shdr_1);
		} else {
			printf("error: not support shdr_mode (%d) \n", sensor_shdr_1);
			return 0;
		}
	}

	if (argc > 3) {
		if (sensor_info[sensor_sel_1].sensor_num == SEN_SEL_PATGEN) {
			if (atoi(argv[3]) > 0) {
				patgen_size_w = atoi(argv[3]);
			}
			sensor_sel_2 = SEN_SEL_NONE;
			printf("sensor 2: none, patgen w = %d \n", patgen_size_w);
		} else {
			if (atoi(argv[3]) < SEN_SEL_MAX) {
				sensor_sel_2 = atoi(argv[3]);
			} else {
				printf("sensor 2: sen_sel_2 not available \n");
				return 0;
			}
			printf("sensor 2: %s \n", sensor_info[sensor_sel_2].sensor_name);
		}
	}

	if (argc > 4) {
		if (sensor_info[sensor_sel_1].sensor_num == SEN_SEL_PATGEN) {
			if (atoi(argv[4]) > 0) {
				patgen_size_h = atoi(argv[4]);
			}
			sensor_shdr_2 = 0;
			printf("sensor 2: shdr off, patgen h = %d \n", patgen_size_h);
		} else {
			sensor_shdr_2 = atoi(argv[4]);
			if (sensor_shdr_2 == 0) {
				printf("sensor 2: shdr off \n");
			} else if (sensor_shdr_2 == 1) {
				printf("sensor 2: shdr on, frame number = %d \n", sensor_shdr_2);
			} else {
				printf("error: not support shdr_mode (%d) \n", sensor_shdr_2);
				return 0;
			}
		}
	}

	if (argc > 5) {
		enc_type = atoi(argv[5]);
		if (enc_type == 0) {
			printf("Enc type: H.265 \n", enc_type);
		} else if (enc_type == 1) {
			printf("Enc type: H.264 \n", enc_type);
		} else if (enc_type == 2) {
			printf("Enc type: MJPG \n", enc_type);
		} else {
			printf("error: not support enc_type (%d) \n", enc_type);
			return 0;
		}
	}

	if (argc > 6) {
		enc_bitrate = atoi(argv[6]);
		printf("Enc bitrate: %d Mbps \n", enc_bitrate);
	}

	if (argc > 7) {
		if (atoi(argv[7]) > 0) {
			pq_menu();
		}
	}

	{
		char *chip_name = getenv("NVT_CHIP_ID");
		if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
			printf("NT98539A \n");
			is_nt98539a = TRUE;
		} else {
			printf("NT98538 \n");
			is_nt98539a = FALSE;
		}
	}

	printf("Dram size = 0x%X  \n", _BOARD_DRAM_SIZE_);

	if ((sensor_sel_1 == SEN_SEL_NONE) && (sensor_sel_2 == SEN_SEL_NONE)) {
		printf("sensor_sel_1 & sensor_sel_1 = SEN_SEL_NONE \n");
		return 0;
	}

	if ((is_direct_mode) & (sensor_sel_2 != SEN_SEL_NONE)) {
		printf("sensor_sel_2 != 0, disable direct mode \n");
		is_direct_mode = FALSE;
	}

	init_share_memory();

	if (vendor_isp_init() == HD_ERR_NG) {
		printf("vendor_isp_init failed \n");
	}

	if (sensor_sel_1 != SEN_SEL_NONE) {
		cfg_info.id = sen1_vcap_id;
		if (is_nt98539a) {
			if (sensor_shdr_1 == 1) {
				if (hdr_8ev_enable) {
					snprintf(cfg_info.path, AE_CFG_NAME_LENGTH - 1, "/mnt/app/isp/%s_8ev_hdr_539a.cfg", sensor_info[sensor_sel_1].isp_cfg_name);
				} else {
					snprintf(cfg_info.path, AE_CFG_NAME_LENGTH - 1, "/mnt/app/isp/%s_hdr_539a.cfg", sensor_info[sensor_sel_1].isp_cfg_name);
				}
			} else {
				snprintf(cfg_info.path, AE_CFG_NAME_LENGTH - 1, "/mnt/app/isp/%s_539a.cfg", sensor_info[sensor_sel_1].isp_cfg_name);
			}
		} else {
			if (sensor_shdr_1 == 1) {
				snprintf(cfg_info.path, AE_CFG_NAME_LENGTH - 1, "/mnt/app/isp/%s_hdr.cfg", sensor_info[sensor_sel_1].isp_cfg_name);
			} else {
				snprintf(cfg_info.path, AE_CFG_NAME_LENGTH - 1, "/mnt/app/isp/%s.cfg", sensor_info[sensor_sel_1].isp_cfg_name);
			}
		}
		printf("sensor 1 load %s \n", cfg_info.path);

		vendor_isp_set_ae(AET_ITEM_RLD_CONFIG, &cfg_info);
		vendor_isp_set_awb(AWBT_ITEM_RLD_CONFIG, &cfg_info);
		vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);

		ct_to_cgain.id = sen1_vcap_id;
		ct_to_cgain.ct_to_cgain.ct = 4700;
		vendor_isp_get_awb(AWBT_ITEM_CT_TO_CGAIN, &ct_to_cgain);

		c_gain.id = sen1_vcap_id;
		c_gain.gain[0] = ct_to_cgain.ct_to_cgain.r_gain;
		c_gain.gain[1] = ct_to_cgain.ct_to_cgain.g_gain;
		c_gain.gain[2] = ct_to_cgain.ct_to_cgain.b_gain;
		vendor_isp_set_common(ISPT_ITEM_C_GAIN, &c_gain);

		ct.id = sen1_vcap_id;
		ct.ct = 4700;
		vendor_isp_set_common(ISPT_ITEM_CT, &ct);
	}

	if (sensor_sel_2 != SEN_SEL_NONE) {
		cfg_info.id = sen2_vcap_id;
		if (is_nt98539a) {
			if (sensor_shdr_2 == 1) {
				snprintf(cfg_info.path, AE_CFG_NAME_LENGTH - 1, "/mnt/app/isp/%s_hdr_539a.cfg", sensor_info[sensor_sel_2].isp_cfg_name);
			} else {
				snprintf(cfg_info.path, AE_CFG_NAME_LENGTH - 1, "/mnt/app/isp/%s_539a.cfg", sensor_info[sensor_sel_2].isp_cfg_name);
			}
		} else {
			if (sensor_shdr_2 == 1) {
				snprintf(cfg_info.path, AE_CFG_NAME_LENGTH - 1, "/mnt/app/isp/%s_hdr.cfg", sensor_info[sensor_sel_2].isp_cfg_name);
			} else {
				snprintf(cfg_info.path, AE_CFG_NAME_LENGTH - 1, "/mnt/app/isp/%s.cfg", sensor_info[sensor_sel_2].isp_cfg_name);
			}
		}
		printf("sensor 2 load %s \n", cfg_info.path);

		vendor_isp_set_ae(AET_ITEM_RLD_CONFIG, &cfg_info);
		vendor_isp_set_awb(AWBT_ITEM_RLD_CONFIG, &cfg_info);
		vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);

		ct_to_cgain.id = 1;
		ct_to_cgain.ct_to_cgain.ct = 4700;
		vendor_isp_get_awb(AWBT_ITEM_CT_TO_CGAIN, &ct_to_cgain);

		c_gain.id = 1;
		c_gain.gain[0] = ct_to_cgain.ct_to_cgain.r_gain;
		c_gain.gain[1] = ct_to_cgain.ct_to_cgain.g_gain;
		c_gain.gain[2] = ct_to_cgain.ct_to_cgain.b_gain;
		vendor_isp_set_common(ISPT_ITEM_C_GAIN, &c_gain);

		ct.id = 1;
		ct.ct = 4700;
		vendor_isp_set_common(ISPT_ITEM_CT, &ct);
	}

	if (sensor_info[sensor_sel_1].sensor_num == SEN_SEL_PATGEN) {
		vdo_size_w_1 = patgen_size_w;
		vdo_size_h_1 = patgen_size_h;
	} else {
		if (vdo_size_w_m && vdo_size_h_m) {
			vdo_size_w_1 = vdo_size_w_m;
			vdo_size_h_1 = vdo_size_h_m;
		} else {
			vdo_size_w_1 = sensor_info[sensor_sel_1].size.w;
			vdo_size_h_1 = sensor_info[sensor_sel_1].size.h;
			// GC5603: adjust for cropped output (2960->2944 for BNR 32-alignment)
			if (sensor_sel_1 == SEN_SEL_GC5603) {
				vdo_size_w_1 = 2944;
			}
		}
	}

	vdo_size_w_2 = sensor_info[sensor_sel_2].size.w;
	vdo_size_h_2 = sensor_info[sensor_sel_2].size.h;

	// init hdal
	#if (PLUG_NNSC_FUNC | PLUG_OSG_FUNC)
	ret = hd_common_init(HD_COMMON_CFG_MULTIPROC | 0);
	#else
	ret = hd_common_init(0);
	#endif
	if (ret != HD_OK) {
		printf("common fail = %d \n", ret);
		goto exit;
	}

	#if (PLUG_NNSC_FUNC)
	hd_common_sysconfig(0, (1<<16), 0, VENDOR_AI_CFG); //enable NN AI engine
	#endif

	// init memory
	ret = mem_init();
	if (ret != HD_OK) {
		printf("mem fail = %d \n", ret);
		goto exit;
	}

	// init all modules
	ret = init_module();
	if (ret != HD_OK) {
		printf("init module fail = %d \n", ret);
		goto exit;
	}

	if (sensor_sel_1 != SEN_SEL_NONE) {
		printf("sen1_vcap_id = %d  \n", sen1_vcap_id);

		stream[0].proc_max_dim.w = vdo_size_w_1;
		stream[0].proc_max_dim.h = vdo_size_h_1;
		ret = open_module(&stream[0], &stream[0].proc_max_dim);
		if (ret != HD_OK) {
			printf("open module fail = %d \n", ret);
			goto exit;
		}

		if (vpe_enable_1) {
			stream[2].proc_max_dim.w = vdo_size_w_1;
			stream[2].proc_max_dim.h = vdo_size_h_1;
			ret = open_module3(&stream[2], &stream[2].proc_max_dim);
			if (ret != HD_OK) {
				printf("open module4 fail = %d \n", ret);
				goto exit;
			}
		}

		// get videocap capability
		ret = get_cap_caps(stream[0].cap_ctrl, &stream[0].cap_syscaps);
		if (ret != HD_OK) {
			printf("get cap-caps fail = %d \n", ret);
			goto exit;
		}

		// set videocap parameter
		stream[0].cap_dim.w = vdo_size_w_1;
		stream[0].cap_dim.h = vdo_size_h_1;

		ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim, 0);
		if (ret != HD_OK) {
			printf("set cap fail = %d \n", ret);
			goto exit;
		}

		if (vcap_preset_expt && vcap_preset_gain) {
			ae_preset.enable = ENABLE;
			ae_preset.exp_time = vcap_preset_expt;
			ae_preset.gain_ratio = vcap_preset_gain;
			vendor_videocap_set(stream[0].cap_path, VENDOR_VIDEOCAP_PARAM_AE_PRESET, &ae_preset);
			printf("ae preset expt %d, gain %d \n", vcap_preset_expt, vcap_preset_gain);
		}

		// assign parameter by program options
		main_dim.w = vdo_size_w_1;
		main_dim.h = vdo_size_h_1;

		// set videoproc parameter
		ret = set_proc_param(stream[0].proc_path, &main_dim);
		if (ret != HD_OK) {
			printf("set proc fail = %d \n", ret);
			goto exit;
		}

		#if (VPRC_PATH5_ENABLE)
		ret = set_proc_param(stream[0].proc_path_3dnr, &main_dim);
		if (ret != HD_OK) {
			printf("set proc fail = %d \n", ret);
			goto exit;
		}
		#endif

		if (vpe_enable_1) {
			ret = set_proc_param(stream[2].proc_path, &main_dim);
			if (ret != HD_OK) {
				printf("set proc fail = %d \n", ret);
				goto exit;
			}
		}

		// set videoenc config
		stream[0].enc_max_dim.w = vdo_size_w_1;
		stream[0].enc_max_dim.h = vdo_size_h_1;
		ret = set_enc_cfg(stream[0].enc_path, &stream[0].enc_max_dim, enc_bitrate * 1024 * 1024, sen1_vcap_id);
		if (ret != HD_OK) {
			printf("set enc-cfg fail = %d \n", ret);
			goto exit;
		}

		// set videoenc parameter
		stream[0].enc_dim.w = vdo_size_w_1;
		stream[0].enc_dim.h = vdo_size_h_1;
		ret = set_enc_param(stream[0].enc_path, &stream[0].enc_dim, enc_type, enc_bitrate * 1024 * 1024);
		if (ret != HD_OK) {
			printf("set enc fail = %d \n", ret);
			goto exit;
		}

		// bind video_record modules
		hd_videocap_bind(HD_VIDEOCAP_OUT(sen1_vcap_id, 0), HD_VIDEOPROC_0_IN_0);
		if (vpe_enable_1) {
			hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOPROC_2_IN_0);
			hd_videoproc_bind(HD_VIDEOPROC_2_OUT_0, HD_VIDEOENC_0_IN_0);
		} else {
			hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOENC_0_IN_0);
		}
	}

	if (sensor_sel_2 != SEN_SEL_NONE) {
		printf("sen2_vcap_id = %d  \n", sen2_vcap_id);

		stream[1].proc_max_dim.w = vdo_size_w_2;
		stream[1].proc_max_dim.h = vdo_size_h_2;
		ret = open_module2(&stream[1], &stream[1].proc_max_dim);
		if (ret != HD_OK) {
			printf("open module2 fail = %d \n", ret);
			goto exit;
		}

		if (vpe_enable_2) {
			stream[3].proc_max_dim.w = vdo_size_w_2;
			stream[3].proc_max_dim.h = vdo_size_h_2;
			ret = open_module4(&stream[3], &stream[3].proc_max_dim);
			if (ret != HD_OK) {
				printf("open module5 fail = %d \n", ret);
				goto exit;
			}
		}

		// get videocap capability
		ret = get_cap_caps(stream[1].cap_ctrl, &stream[1].cap_syscaps);
		if (ret != HD_OK) {
			printf("get cap-caps fail = %d \n", ret);
			goto exit;
		}

		// set videocap parameter
		stream[1].cap_dim.w = vdo_size_w_2;
		stream[1].cap_dim.h = vdo_size_h_2;
		ret = set_cap_param(stream[1].cap_path, &stream[1].cap_dim, 1);
		if (ret != HD_OK) {
			printf("set cap2 fail = %d \n", ret);
			goto exit;
		}

		main_dim.w = vdo_size_w_2;
		main_dim.h = vdo_size_h_2;

		ret = set_proc_param(stream[1].proc_path, &main_dim);
		if (ret != HD_OK) {
			printf("set proc2 fail = %d \n", ret);
			goto exit;
		}

		#if (VPRC_PATH5_ENABLE)
		ret = set_proc_param(stream[1].proc_path_3dnr, &main_dim);
		if (ret != HD_OK) {
			printf("set proc fail = %d \n", ret);
			goto exit;
		}
		#endif

		if (vpe_enable_2) {
			ret = set_proc_param(stream[3].proc_path, &main_dim);
			if (ret != HD_OK) {
				printf("set proc fail = %d \n", ret);
				goto exit;
			}
		}

		stream[1].enc_max_dim.w = vdo_size_w_2;
		stream[1].enc_max_dim.h = vdo_size_h_2;
		ret = set_enc_cfg(stream[1].enc_path, &stream[1].enc_max_dim, enc_bitrate * 1024 * 1024, sen2_vcap_id);
		if (ret != HD_OK) {
			printf("set enc-cfg fail = %d \n", ret);
			goto exit;
		}

		stream[1].enc_dim.w = vdo_size_w_2;
		stream[1].enc_dim.h = vdo_size_h_2;
		ret = set_enc_param(stream[1].enc_path, &stream[1].enc_dim, enc_type, enc_bitrate * 1024 * 1024);
		if (ret != HD_OK) {
			printf("set enc2 fail = %d \n", ret);
			goto exit;
		}

		hd_videocap_bind(HD_VIDEOCAP_OUT(sen2_vcap_id, 0), HD_VIDEOPROC_1_IN_0);
		if (vpe_enable_2) {
			hd_videoproc_bind(HD_VIDEOPROC_1_OUT_0, HD_VIDEOPROC_3_IN_0);
			hd_videoproc_bind(HD_VIDEOPROC_3_OUT_0, HD_VIDEOENC_0_IN_1);
		} else {
			hd_videoproc_bind(HD_VIDEOPROC_1_OUT_0, HD_VIDEOENC_0_IN_1);
		}
	}

	if (sensor_sel_1 != SEN_SEL_NONE) {
		if (is_direct_mode) {
			hd_videoproc_start(stream[0].proc_path);
			hd_videocap_start(stream[0].cap_path);
		} else {
			hd_videocap_start(stream[0].cap_path);
			hd_videoproc_start(stream[0].proc_path);
		}

		if (vpe_enable_1) {
			hd_videoproc_start(stream[2].proc_path);
		}

		#if (VPRC_PATH5_ENABLE)
		hd_videoproc_start(stream[0].proc_path_3dnr);
		#endif

		hd_videoenc_start(stream[0].enc_path);
	}

	if (sensor_sel_2 != SEN_SEL_NONE) {
		hd_videocap_start(stream[1].cap_path);
		hd_videoproc_start(stream[1].proc_path);

		if (vpe_enable_2) {
			hd_videoproc_start(stream[3].proc_path);
		}

		#if (VPRC_PATH5_ENABLE)
		hd_videoproc_start(stream[1].proc_path_3dnr);
		#endif

		hd_videoenc_start(stream[1].enc_path);
	}

	// let encode_thread start to work
	if (sensor_sel_1 != SEN_SEL_NONE) {
		stream[0].flow_start = 1;
	} else {
		stream[1].flow_start = 1;
	}

	// Wait for video pipeline to stabilize and encoder to produce frames
	printf("Waiting for video pipeline to stabilize...\n");
	sleep(2);

	#if (PLUG_NNSC_FUNC | PLUG_OSG_FUNC)
	system("nvtrtspd_ipc 0 1 &");
	printf("nvtrtspd_ipc 0 1 \n");
	#else
	system("nvtrtspd_ipc &");
	printf("nvtrtspd_ipc \n");
	#endif

	// query user key
	printf("Enter q to exit \n");

	while (1) {
		key = getchar();
		if (key == 'q' || key == 0x3) {
			p_pd_shm = (PD_SHM_INFO *)g_shm;
			p_pd_shm->exit = 1;
			usleep(300000);
			break;
		}

		// NOTE: Start of test path 1 linear/shdr switch
		if (key == 'x') {
			ISPT_D_GAIN d_gain = {0};
			ISPT_C_GAIN c_gain = {0};
			ISPT_TOTAL_GAIN total_gain = {0};
			ISPT_LV lv = {0};
			ISPT_CT ct = {0};
			ISPT_SENSOR_EXPT sensor_expt = {0};
			ISPT_SENSOR_GAIN sensor_gain = {0};

			d_gain.id = sen1_vcap_id;
			c_gain.id = sen1_vcap_id;
			total_gain.id = sen1_vcap_id;
			lv.id = sen1_vcap_id;
			ct.id = sen1_vcap_id;
			vendor_isp_get_common(ISPT_ITEM_D_GAIN, &d_gain);
			vendor_isp_get_common(ISPT_ITEM_C_GAIN, &c_gain);
			vendor_isp_get_common(ISPT_ITEM_TOTAL_GAIN, &total_gain);
			vendor_isp_get_common(ISPT_ITEM_LV, &lv);
			vendor_isp_get_common(ISPT_ITEM_CT, &ct);
			vendor_isp_get_common(ISPT_ITEM_SENSOR_EXPT, &sensor_expt);
			vendor_isp_get_common(ISPT_ITEM_SENSOR_GAIN, &sensor_gain);
			printf("d gain = %d \n", d_gain.gain);
			printf("c gain = %d %d %d \n", c_gain.gain[0], c_gain.gain[1], c_gain.gain[2]);
			printf("total gain = %d \n", total_gain.gain);
			printf("lv = %d \n", lv.lv);
			printf("ct = %d \n", ct.ct);
			printf("expt = %d, ratio = %d \n", sensor_expt.time[0], sensor_gain.ratio[0]);

			sensor_shdr_1 = !sensor_shdr_1;
			printf("SHDR %d \n", sensor_shdr_1);

			hd_videocap_stop(stream[0].cap_path);
			hd_videocap_close(stream[0].cap_path);
			hd_videoproc_stop(stream[0].proc_path);
			hd_videoproc_close(stream[0].proc_path);
			//hd_videoenc_stop(stream[0].enc_path);
			//hd_videoenc_close(stream[0].enc_path);
			stream[0].flow_start = 0;

			open_module_without_enc(&stream[0], &stream[0].proc_max_dim);
			get_cap_caps(stream[0].cap_ctrl, &stream[0].cap_syscaps);
			set_cap_param(stream[0].cap_path, &stream[0].cap_dim, 0);
			set_proc_param(stream[0].proc_path, &main_dim);
			//set_enc_cfg(stream[0].enc_path, &stream[0].enc_max_dim, enc_bitrate * 1024 * 1024, 0);
			//set_enc_param(stream[0].enc_path, &stream[0].enc_dim, enc_type, enc_bitrate * 1024 * 1024);

			vendor_isp_set_common(ISPT_ITEM_D_GAIN, &d_gain);
			vendor_isp_set_common(ISPT_ITEM_C_GAIN, &c_gain);
			vendor_isp_set_common(ISPT_ITEM_TOTAL_GAIN, &total_gain);
			vendor_isp_set_common(ISPT_ITEM_LV, &lv);
			vendor_isp_set_common(ISPT_ITEM_CT, &ct);

			ae_preset.enable = ENABLE;
			ae_preset.exp_time = sensor_expt.time[0];
			ae_preset.gain_ratio = sensor_gain.ratio[0];
			vendor_videocap_set(stream[0].cap_path, VENDOR_VIDEOCAP_PARAM_AE_PRESET, &ae_preset);

			hd_videocap_start(stream[0].cap_path);
			hd_videoproc_start(stream[0].proc_path);
			//hd_videoenc_start(stream[0].enc_path);
			stream[0].flow_start = 1;
		}
		// NOTE: End of test path 1 linear/shdr switch

		// NOTE: Start of test path 1 shdr debug mode switch
		if (key == 'a') {
			IQT_SHDR_MODE shdr_mode = {0};
			static UINT32 sensor_shdr_1_mode = 1;

			if (sensor_shdr_1 != 0) {
				sensor_shdr_1_mode = !sensor_shdr_1_mode;

				cfg_info.id = sen1_vcap_id;
				if (sensor_shdr_1_mode == 0) {
					snprintf(cfg_info.path, AE_CFG_NAME_LENGTH - 1, "/mnt/app/isp/%s.cfg", sensor_info[sensor_sel_1].isp_cfg_name);
				} else if ((sensor_shdr_1_mode == 1) && (sensor_shdr_1 == 1)) {
					snprintf(cfg_info.path, AE_CFG_NAME_LENGTH - 1, "/mnt/app/isp/%s_hdr.cfg", sensor_info[sensor_sel_1].isp_cfg_name);
				} else {
					printf("No isp cfg was chosen \n", cfg_info.path);
				}

				vendor_isp_set_awb(AWBT_ITEM_RLD_CONFIG, &cfg_info);
				vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);
				printf("sensor 1 load %s \n", cfg_info.path);

				shdr_mode.id = sen1_vcap_id;
				if(sensor_shdr_1_mode == 1) {
					shdr_mode.shdr_mode = IQT_SHDR_MODE_FUSION;
					printf("Switch SHDR mode to Fusion\n");
				} else {
					shdr_mode.shdr_mode = IQT_SHDR_MODE_LONG;
					printf("Switch SHDR mode to Long\n");
				}
				vendor_isp_set_iq(IQT_ITEM_SHDR_MODE, &shdr_mode);
			} else {
				printf("sensor_shdr_1 = %d, can not switch shdr mode \n", sensor_shdr_1);
			}
		}
		// NOTE: End of test path 1 shdr debug mode switch

		#if (SAVE_RAW_ENABLE)
		if (key == 'c') {
			VENDOR_COMM_MAX_FREE_BLOCK max_free_block = {0};
			CHAR buf[32];
			INT rt;
			UINT32 val, raw_size;
			
			if ((is_direct_mode) && (sensor_sel_2 == SEN_SEL_NONE)) {
				printf("Does not support raw cap in direct mode \n", cap_max_num);
				continue;
			}

			raw_size = ((vdo_size_w_1 * 3 / 2) * vdo_size_h_1) * (sensor_shdr_1 + 1);
			if (sensor_sel_2 != SEN_SEL_NONE) {
				raw_size += ((vdo_size_w_2 * 3 / 2) * vdo_size_h_2) * (sensor_shdr_2 + 1);
			}

			if (save_raw_via_ddr) {
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
			} else {
				// TODO: need check disk size
				cap_max_num = 99999;
			}

			printf("Please input capture number, maximum is about %d \n", cap_max_num);
			rt = scanf("%d", &val);
			if (rt != 1) {
				clearerr(stdin);
				fgets(buf, sizeof(buf), stdin);
				printf("Invalid cap_num. set to 1 \n");
				stream[0].cap_num = 1;
				stream[1].cap_num = 1;
			} else if (val < cap_max_num) {
				stream[0].cap_num = val;
				stream[1].cap_num = val;
			} else {
				stream[0].cap_num = cap_max_num;
				stream[1].cap_num = cap_max_num;
			}

			stream[0].cap_frame_num = sensor_shdr_1 + 1;
			stream[0].cap_stop = 0;
			stream[0].cap_exit = 0;
			stream[1].cap_frame_num = sensor_shdr_2 + 1;
			stream[1].cap_stop = 0;
			stream[1].cap_exit = 0;
			for (i = 0; i < stream[0].cap_frame_num; i++) {
				cap_idx[0][i] = 0;
				cap_idx[1][i] = 0;
			}

			ret = pthread_create(&stream[0].cap_thread_id, NULL, cap_raw_thread, (void *)&stream[0]);
			if (ret < 0) {
				printf("create cap_raw_thread fail \n");
				break;
			}

			while (stream[0].cap_exit == 0) sleep(1);

			pthread_join(stream[0].cap_thread_id, NULL);

			if (save_raw_via_ddr) {
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
		}
		#endif
	}

	// stop video_record modules
	if (sensor_sel_1 != SEN_SEL_NONE) {
		if (is_direct_mode) {
			hd_videoproc_stop(stream[0].proc_path);
			hd_videocap_stop(stream[0].cap_path);
		} else {
			hd_videocap_stop(stream[0].cap_path);
			hd_videoproc_stop(stream[0].proc_path);
		}
		if (vpe_enable_1) {
			hd_videoproc_stop(stream[2].proc_path);
		}
		hd_videoenc_stop(stream[0].enc_path);
	}

	if (sensor_sel_2 != SEN_SEL_NONE) {
		hd_videocap_stop(stream[1].cap_path);
		hd_videoproc_stop(stream[1].proc_path);
		if (vpe_enable_2) {
			hd_videoproc_stop(stream[3].proc_path);
		}
		hd_videoenc_stop(stream[1].enc_path);
	}

	if (sensor_sel_1 != SEN_SEL_NONE) {
		// unbind video_record modules
		hd_videocap_unbind(HD_VIDEOCAP_OUT(sen1_vcap_id, 0));
		hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_0);
		if (vpe_enable_1) {
			hd_videoproc_unbind(HD_VIDEOPROC_2_OUT_0);
		}
	}

	if (sensor_sel_2 != SEN_SEL_NONE) {
		// unbind video_record modules
		hd_videocap_unbind(HD_VIDEOCAP_OUT(sen2_vcap_id, 0));
		hd_videoproc_unbind(HD_VIDEOPROC_1_OUT_0);
		if (vpe_enable_2) {
			hd_videoproc_unbind(HD_VIDEOPROC_3_OUT_0);
		}
	}

exit:
	if (sensor_sel_1 != SEN_SEL_NONE) {
		// close video_record modules
		ret = close_module(&stream[0]);
		if (ret != HD_OK) {
			printf("close fail = %d \n", ret);
		}

		if (vpe_enable_1) {
			ret = close_module2(&stream[2]);
			if (ret != HD_OK) {
				printf("close2 fail=%d\n", ret);
			}
		}
	}

	if (sensor_sel_2 != SEN_SEL_NONE) {
		ret = close_module(&stream[1]);
		if (ret != HD_OK) {
			printf("close fail= %d \n", ret);
		}

		if (vpe_enable_2) {
			ret = close_module2(&stream[3]);
			if (ret != HD_OK) {
				printf("close2 fail=%d\n", ret);
			}
		}
	}

	// uninit all modules
	ret = exit_module();
	if (ret != HD_OK) {
		printf("exit fail = %d \n", ret);
	}

	// uninit memory
	ret = mem_exit();
	if (ret != HD_OK) {
		printf("mem fail = %d \n", ret);
	}

	// uninit hdal
	ret = hd_common_uninit();
	if (ret != HD_OK) {
		printf("common fail = %d \n", ret);
	}

	vendor_isp_uninit();

	exit_share_memory();

	return 0;
}

