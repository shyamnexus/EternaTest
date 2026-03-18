/**
    @brief Demo code of 1 videocap path + 2 videoproc path + 1 videoenc path.\n

    @file hd_demo_turnkey.c

    @author iVOT/CVAI

    @ingroup mhdal

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2024.  All rights reserved.
*/

#if defined(__LINUX)
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <signal.h>
#include <pthread.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "hdal.h"
#include "hd_debug.h"
#include "hd_type.h"
#include "vendor_videoenc.h"
#include <kwrap/examsys.h> 	//for MAIN(), GETCHAR() API
#include <pd_shm.h>

#include "sample_ipc_ext.h"
#include "pvdcnn_lib.h"

#define DISABLE         0
#define ENABLE          1

// platform dependent
#if defined(__LINUX)
#define MAIN(argc, argv) 		int main(int argc, char** argv)
#define GETCHAR()				getchar()
#include <sys/prctl.h>
#include "math.h"
#include <sys/shm.h>
#else
// RTOS
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/signal.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <kwrap/util.h>		//for sleep API
#define sleep(x)    			vos_util_delay_ms(1000*(x))
#define msleep(x)    			vos_util_delay_ms(x)
#define usleep(x)   			vos_util_delay_us(x)
#include <kwrap/examsys.h> 	//for MAIN(), GETCHAR() API
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(hd_video_record_with_vsp, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define DEBUG_MENU 		1

#define CHKPNT			printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
#define DBGH(x)			printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x)			printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)
#define MAX_VAL(a,b)        ((a) > (b) ? (a) : (b))

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
// Sensor Config
#define SEN_OUT_FMT		 HD_VIDEO_PXLFMT_RAW12
#define CAP_OUT_FMT		 HD_VIDEO_PXLFMT_NRX12
#define SHDR_CAP_OUT_FMT HD_VIDEO_PXLFMT_RAW12_SHDR2

#define SEN1_VCAP_ID 0
#define SEN1_CSI_ID  0

#define CA_WIN_NUM_W		32
#define CA_WIN_NUM_H		32
#define LA_WIN_NUM_W		32
#define LA_WIN_NUM_H		32
#define VA_WIN_NUM_W		16
#define VA_WIN_NUM_H		16
#define YOUT_WIN_NUM_W		128
#define YOUT_WIN_NUM_H		128

#define SEN_SEL_IMX290    0  // 2M (IMX290)
#define SEN_SEL_OS05A10   1  // 5M (OS05A)
#define SEN_SEL_OS02K10   2  // 2M (OS02K10)
#define SEN_SEL_AR0237IR  3  // 2M (AR0237IR)
#define SEN_SEL_OS04C10   4  // 4M (OS04C10)
#define SEN_SEL_IMX415    5  // 8M (IMX415)
#define SEN_SEL_SC450AI   6  // 4M_2 (SC450AI)
#define SEN_SEL_OS04A10   7  // 4M_2 (OS04A10)

#define VDO_SIZE_W_8M      3840
#define VDO_SIZE_H_8M      2160
#define VDO_SIZE_W_5M      2592
#define VDO_SIZE_H_5M      1944
#define VDO_SIZE_W_5M_2    2880
#define VDO_SIZE_H_5M_2    1620
#define VDO_SIZE_W_4M      2560
#define VDO_SIZE_H_4M      1440
#define VDO_SIZE_W_4M_2    2688
#define VDO_SIZE_H_4M_2    1520
#define VDO_SIZE_W_2M      1920
#define VDO_SIZE_H_2M      1080

// AI size
#if (PVD_YUV_HEIGHT > 576)
#define VDO_SIZE_W_AI       1280
#define VDO_SIZE_H_AI        720
#else
#define VDO_SIZE_W_AI       1024
#define VDO_SIZE_H_AI        576
#endif

static UINT32 sensor_sel = SEN_SEL_IMX415;
static UINT32 g_shdr = 0;	//0:disable, 1:enable SHDR
static UINT32 g_capbind = 0;//0:D2D, 1:direct, 2: one-buf
static UINT32 g_capfmt = 0;	//0:RAW, 1:RAW-compress
static UINT32 g_prcbind = 0;//0:D2D, 1:lowlatency, 2: one-buf
static UINT32 g_prcfmt = 0;	//0:YUV, 1:YUV-compress
static UINT32 g_prccfg = 0;	//0(STRP_LV1), 1(STRP_LV2), 2(STRP_LV3), 3(GDC_OFF)
static UINT32 g_main_ltr_en = 0; //0: disable, 1: enable
static UINT32 g_fps = 30;
static UINT32 vdo_size_w = VDO_SIZE_W_8M;
static UINT32 vdo_size_h = VDO_SIZE_H_8M;
static UINT32 vdo_br = (8 * 1024 * 1024);
static UINT32 subvdo_size_w = VDO_SIZE_W_AI;
static UINT32 subvdo_size_h = VDO_SIZE_H_AI;

///////////////////////////////////////////////////////////////////////////////

#define VCAP_DDR_ID		DDR_ID0
#define VPRC_DDR_ID		DDR_ID0
#define VCNN_DDR_ID		DDR_ID0

#define HD_VIDEOPROC_CFG				0x000f0000	//vprc
#define HD_VIDEOPROC_CFG_STRIP_MASK		0x00000007  //vprc stripe rule mask: (default 0)
#define HD_VIDEOPROC_CFG_STRIP_LV1		0x00000000  //vprc "0: GDC,low latency balanced
#define HD_VIDEOPROC_CFG_STRIP_LV2		0x00010000  //vprc "1: low latency Best
#define HD_VIDEOPROC_CFG_STRIP_LV3		0x00020000  //vprc "2: 2D_LUT Best if 2D_LUT supported
#define HD_VIDEOPROC_CFG_STRIP_LV4		0x00030000  //vprc "3: GDC Best,low lantency don't case #define HD_VIDEOPROC_CFG_DISABLE_GDC	HD_VIDEOPROC_CFG_STRIP_LV2
#define HD_VIDEOPROC_CFG_DISABLE_GDC    HD_VIDEOPROC_CFG_STRIP_LV2
#define HD_VIDEOPROC_CFG_LL_FAST		HD_VIDEOPROC_CFG_STRIP_LV2
#define HD_VIDEOPROC_CFG_2DLUT_BEST		HD_VIDEOPROC_CFG_STRIP_LV3
#define HD_VIDEOPROC_CFG_GDC_BEST		HD_VIDEOPROC_CFG_STRIP_LV4

// iq parameter
#define USE_REAL_SENSOR     1    // set 0 for patgen
#define ENABLE_2A           1
#define VIDEOCAP_ALG_FUNC   HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB
#define FUNC_3DNR           DISABLE
#define FUNC_DEFOG          DISABLE
#define FUNC_COLORNR        DISABLE
#define FUNC_WDR            DISABLE

///////////////////////////////////////////////////////////////////////////////

// AI parameter
#define FUNC_AI             ENABLE
#define SHOW_BBOX_IN_VLC    ENABLE

///////////////////////////////////////////////////////////////////////////////

typedef struct _VIDEO_RECORD {

	// (1)
	HD_VIDEOCAP_SYSCAPS cap_syscaps;
	HD_PATH_ID cap_ctrl;
	HD_PATH_ID cap_path;

	HD_DIM  cap_dim;

	// (2)
	HD_DIM  proc_max_dim;
	HD_VIDEOPROC_SYSCAPS proc_syscaps;
	HD_PATH_ID proc_ctrl;
	HD_PATH_ID proc_path;
	HD_PATH_ID proc_path2;

	// (3)
	HD_DIM  enc_max_dim;
	HD_DIM  enc_dim;

	HD_VIDEOENC_SYSCAPS enc_syscaps;
	HD_PATH_ID enc_path;

	// (4) user pull
	INT32      wait_ms;
} VIDEO_RECORD;


static int    g_quit = 0;
static UINT32 g_enc_poll = 1;

static char *g_shm = NULL;
static int g_shmid = 0;

#if (FUNC_AI == ENABLE)

#define cfg_file    "/mnt/sd/configs/turnkey_para_config.txt"

typedef enum _turnkey_subprocess_name {
	turnkey_sample_stream_ipc = 1,
} turnkey_subprocess_name;

static turnkey_subprocess_name turnkey_name = turnkey_sample_stream_ipc;
static SAMPLE_CFG_BUFSIZE_S cfg_bufsize = {0};
static SAMPLE_CFG_PARAM_S cfg_param     = {0};

static HD_RESULT demo_read_system_cfg(void)
{
	HD_RESULT ret;

	SAMPLE_SYS_CFG_S system_cfg = {0};

	ret = sample_read_system_param(cfg_file, &system_cfg);
	if(ret != HD_OK){
		printf("[hd_demo_turnkey] get config params failed \r\n");
		printf("[hd_demo_turnkey] use sample code default params \r\n");
		return HD_OK;
	}
	turnkey_name = system_cfg.sample_id;
	sensor_sel   = system_cfg.sensor_id;

	ret = sample_get_cfg(cfg_file, &cfg_bufsize, &cfg_param, (int)PVD_YUV_WIDTH, (int)PVD_YUV_HEIGHT, (int)PRUNE37PVD, (int)0);
	if(ret != HD_OK){
		printf("[hd_demo_turnkey] get config params failed \r\n");
		printf("[hd_demo_turnkey] use cnn buffer size = %d\r\n", cfg_bufsize.total_bufsize);
		return HD_OK;
	}
    
    if ((LIMIT_FDET_PVD == DISABLE) || (cfg_param.ai_mode[3] <= 0))   // limit_fdet or pvdcnn disable
    {
		cfg_bufsize.total_bufsize -= cfg_bufsize.limit_fdet_bufsize;
	}

	if (cfg_param.frames_per_second != g_fps) {
		g_fps = cfg_param.frames_per_second;
		printf("[hd_demo_turnkey] changed sensor fps to %d\r\n", g_fps);
	}

	return HD_OK;
}

static HD_RESULT sub_process_arg_encode(turnkey_subprocess_name turnkey, CHAR* str_buffer, UINT str_buffer_size, UINT pull_out_id)
{
	HD_RESULT ret = HD_OK;
	int PDCNN_MODE = 0;
	int FDCNN_MODE = 0;
	int CDDCNN_MODE = 0;
	int PVDCNN_MODE = 0;
	int FADCNN_MODE = 0;

	for(UINT i=0; i < AI_MODEL_NUM; i++) {
		switch (i) {
			case 0: PDCNN_MODE = cfg_param.ai_mode[i]; break;
			case 1: FDCNN_MODE = cfg_param.ai_mode[i]; break;
			case 2: CDDCNN_MODE = cfg_param.ai_mode[i]; break;
			case 3: PVDCNN_MODE = cfg_param.ai_mode[i]; break;
			case 4: FADCNN_MODE = cfg_param.ai_mode[i]; break;
		default: break;
		}
	}

	// When modifying buffer definition, please consider these turnkey cases as well.
	switch(turnkey) {
	case turnkey_sample_stream_ipc:
		snprintf(str_buffer, str_buffer_size, "ai3_turnkey_sample_stream_ipc %u", pull_out_id);
		if ((PDCNN_MODE != 1) && (FDCNN_MODE != 1) && (CDDCNN_MODE != 1) && (PVDCNN_MODE != 1) && (FADCNN_MODE != 1)) {
			printf("   -> !!! AI MODE Incorrect, if you want to run ai3_turnkey_sample_stream_ipc, please set one of ai_mode[0~4] = 1, Ex. 00010 !!!\r\n");
			ret = HD_ERR_NOT_SUPPORT;
		}
		break;
	default:
		printf("[hd_demo_turnkey] Not supported turnkey sub-process: %d\r\n", turnkey);
		ret = HD_ERR_NOT_SUPPORT;
		break;
	}
	return ret;
}

#endif

///////////////////////////////////////////////////////////////////////////////
void init_share_memory(void)
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
    // Initialization.
    memset(g_shm, 0, PD_SHMSZ );
	p_pd_shm = (PD_SHM_INFO  *)g_shm;
	p_pd_shm->enc_path[0].w = vdo_size_w;
	p_pd_shm->enc_path[0].h = vdo_size_h;
	p_pd_shm->enc_path[1].w = vdo_size_w;
	p_pd_shm->enc_path[1].h = vdo_size_h;
	p_pd_shm->pd_path.w = subvdo_size_w;
	p_pd_shm->pd_path.h = subvdo_size_h;
}

void exit_share_memory(void)
{
	if (g_shm) {
		shmdt(g_shm);
		shmctl(g_shmid, IPC_RMID, NULL);
	}
}

static HD_RESULT mem_init(UINT32 CNN_MEM_SIZE)
{
	HD_RESULT              ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
	UINT32                   i = 0;

	// config common pool (cap)
	mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	if (g_capbind == 0) {
		//normal
		mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()
										+VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
										+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
		if (g_capfmt == HD_VIDEO_PXLFMT_NRX12)
			mem_cfg.pool_info[i].blk_size += VDO_NRX_BUFSIZE(vdo_size_w, vdo_size_h);
		else
			mem_cfg.pool_info[i].blk_size += VDO_RAW_BUFSIZE(vdo_size_w, vdo_size_h, g_capfmt);
		mem_cfg.pool_info[i].blk_cnt = 2;
		mem_cfg.pool_info[i].ddr_id = VCAP_DDR_ID;
	} else if (g_capbind == 1) {
		//direct ... NOT require raw
		mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()
										+VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
										+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
		mem_cfg.pool_info[i].blk_cnt = 2;
		mem_cfg.pool_info[i].ddr_id = VCAP_DDR_ID;
	} else if (g_capbind == 2) {
		//one-buf
		mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()
										+VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
										+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
		if (g_capfmt == HD_VIDEO_PXLFMT_NRX12)
			mem_cfg.pool_info[i].blk_size += VDO_NRX_BUFSIZE(vdo_size_w, vdo_size_h);
		else
			mem_cfg.pool_info[i].blk_size += VDO_RAW_BUFSIZE(vdo_size_w, vdo_size_h, g_capfmt);
		mem_cfg.pool_info[i].blk_cnt = 1;
		mem_cfg.pool_info[i].ddr_id = VCAP_DDR_ID;
	} else {
		//not support
		mem_cfg.pool_info[i].blk_size = 0;
		mem_cfg.pool_info[i].blk_cnt = 0;
		mem_cfg.pool_info[i].ddr_id = VCAP_DDR_ID;
	}
	if (g_shdr == 1) {
		mem_cfg.pool_info[i].blk_cnt *= 2;
	}

	// config common pool (main)
	i++;
	mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	if (g_prcbind == 0) {
		//normal
		mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()
										+VDO_YUV_BUFSIZE(vdo_size_w, vdo_size_h, HD_VIDEO_PXLFMT_YUV420);
		mem_cfg.pool_info[i].blk_cnt = 3; //venc release blk is slow, depend on its waiting list
		mem_cfg.pool_info[i].ddr_id = VPRC_DDR_ID;
	} else if (g_prcbind == 1) {
		//low-latency
		mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()
										+VDO_YUV_BUFSIZE(vdo_size_w, vdo_size_h, HD_VIDEO_PXLFMT_YUV420);
		mem_cfg.pool_info[i].blk_cnt = 2;
		mem_cfg.pool_info[i].ddr_id = VPRC_DDR_ID;
	} else if (g_prcbind == 2) {
		//one-buf
		mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()
										+VDO_YUV_BUFSIZE(vdo_size_w, vdo_size_h, HD_VIDEO_PXLFMT_YUV420);
		mem_cfg.pool_info[i].blk_cnt = 1;
		mem_cfg.pool_info[i].ddr_id = VPRC_DDR_ID;
	} else {
		//not support
		mem_cfg.pool_info[i].blk_size = 0;
		mem_cfg.pool_info[i].blk_cnt = 0;
		mem_cfg.pool_info[i].ddr_id = VPRC_DDR_ID;
	}

	// config common pool (sub)
	i++;
	mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	if (g_prcbind == 0) {
		//normal
		mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()
										+VDO_YUV_BUFSIZE(subvdo_size_w, subvdo_size_h, HD_VIDEO_PXLFMT_YUV420);
		mem_cfg.pool_info[i].blk_cnt = 3; //venc release blk is slow, depend on its waiting list
		mem_cfg.pool_info[i].ddr_id = VPRC_DDR_ID;
	} else if (g_prcbind == 1) {
		//low-latency
		mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()
										+VDO_YUV_BUFSIZE(subvdo_size_w, subvdo_size_h, HD_VIDEO_PXLFMT_YUV420);
		mem_cfg.pool_info[i].blk_cnt = 2;
		mem_cfg.pool_info[i].ddr_id = VPRC_DDR_ID;
	} else if (g_prcbind == 2) {
		//one-buf
		mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()
										+VDO_YUV_BUFSIZE(subvdo_size_w, subvdo_size_h, HD_VIDEO_PXLFMT_YUV420);
		mem_cfg.pool_info[i].blk_cnt = 1;
		mem_cfg.pool_info[i].ddr_id = VPRC_DDR_ID;
	} else {
		//not support
		mem_cfg.pool_info[i].blk_size = 0;
		mem_cfg.pool_info[i].blk_cnt = 0;
		mem_cfg.pool_info[i].ddr_id = VPRC_DDR_ID;
	}

#if (FUNC_AI == ENABLE)
	i++;
	mem_cfg.pool_info[i].type = HD_COMMON_MEM_CNN_POOL;
	mem_cfg.pool_info[i].blk_size = CNN_MEM_SIZE;
	mem_cfg.pool_info[i].blk_cnt = 1;
	mem_cfg.pool_info[i].ddr_id = VCNN_DDR_ID;
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
#if USE_REAL_SENSOR
	HD_VIDEOCAP_CTRL iq_ctl = {0};

	if (sensor_sel == SEN_SEL_IMX290) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN - 1, "nvt_sen_imx290");
		printf("Using nvt_sen_imx290\n");
	} else if (sensor_sel == SEN_SEL_OS05A10) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN - 1, "nvt_sen_os05a10");
		printf("Using nvt_sen_os05a10\n");
	} else if (sensor_sel == SEN_SEL_OS02K10) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN - 1, "nvt_sen_os02k10");
		printf("Using nvt_sen_os02k10\n");
	} else if (sensor_sel == SEN_SEL_AR0237IR) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN - 1, "nvt_sen_ar0237ir");
		printf("Using nvt_sen_ar0237ir\n");
	} else if (sensor_sel == SEN_SEL_OS04C10) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN - 1, "nvt_sen_os04c10");
		printf("Using nvt_sen_os04c10\n");
	} else if (sensor_sel == SEN_SEL_IMX415) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN - 1, "nvt_sen_imx415");
		printf("Using nvt_sen_imx415\n");
	} else if (sensor_sel == SEN_SEL_SC450AI) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN - 1, "nvt_sen_sc450ai");
		printf("Using nvt_sen_sc450ai\n");
	} else if (sensor_sel == SEN_SEL_OS04A10) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN - 1, "nvt_sen_os04a10");
		printf("Using nvt_sen_os04a10\n");
	}

	cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
	if (sensor_sel == SEN_SEL_AR0237IR) {
		cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_P_RAW;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux = 0;  //use @0 in peri-dev.dtsi
		// printf("Parallel interface\n");
	} else {
		cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux = 0;  //use @0 in peri-dev.dtsi
		// printf("MIPI interface\n");
	}

	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(SEN1_CSI_ID, 0);
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = HD_VIDEOCAP_SEN_IGNORE;
#else
	snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, HD_VIDEOCAP_SEN_PAT_GEN);
#endif
	ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(SEN1_VCAP_ID), &video_cap_ctrl); //open this for device control
	if (ret != HD_OK) {
		return ret;
	}

#if USE_REAL_SENSOR
	if (g_shdr == 1) {
		cap_cfg.sen_cfg.shdr_map = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR1, (HD_VIDEOCAP_0|HD_VIDEOCAP_1));
	}
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
#if ENABLE_2A
	iq_ctl.func = VIDEOCAP_ALG_FUNC;
#endif
	if (g_shdr == 1) {
		iq_ctl.func |= HD_VIDEOCAP_FUNC_SHDR;
	}
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);
#endif

	*p_video_cap_ctrl = video_cap_ctrl;
	return ret;
}

static HD_RESULT set_cap_param(HD_PATH_ID video_cap_path, HD_DIM *p_dim)
{
	HD_RESULT ret = HD_OK;
	{//select sensor mode, manually or automatically
		HD_VIDEOCAP_IN video_in_param = {0};
#if USE_REAL_SENSOR
		video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO; //auto select sensor mode by the parameter of HD_VIDEOCAP_PARAM_OUT
		video_in_param.frc = HD_VIDEO_FRC_RATIO(g_fps,1);
		video_in_param.dim.w = p_dim->w;
		video_in_param.dim.h = p_dim->h;
		video_in_param.pxlfmt = SEN_OUT_FMT;
		// NOTE: only SHDR with path 1
		if ((video_cap_path == 0) && (g_shdr == 1)) {
			video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_2;
		} else {
			video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
		}
#else
		video_in_param.sen_mode = HD_VIDEOCAP_PATGEN_MODE(HD_VIDEOCAP_SEN_PAT_COLORBAR, 200);
		video_in_param.frc = HD_VIDEO_FRC_RATIO(30,1);
		video_in_param.dim.w = p_dim->w;
		video_in_param.dim.h = p_dim->h;
#endif
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN, &video_in_param);
		//printf("set_cap_param MODE=%d\r\n", ret);
		if (ret != HD_OK) {
			return ret;
		}
	}

	{
		HD_VIDEOCAP_CROP video_crop_param = {0};

		video_crop_param.mode = HD_CROP_OFF;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);
		//printf("set_cap_param CROP NONE=%d\r\n", ret);
	}

	{
		HD_VIDEOCAP_OUT video_out_param = {0};

		//without setting dim for no scaling, using original sensor out size
		// NOTE: only SHDR with path 1
		if ((video_cap_path == 0) && (g_shdr == 1)) {
			video_out_param.pxlfmt = SHDR_CAP_OUT_FMT;
		} else {
			video_out_param.pxlfmt = g_capfmt;
		}
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		video_out_param.depth = 0;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT, &video_out_param);
		//printf("set_cap_param OUT=%d\r\n", ret);
	}

	{
		HD_VIDEOCAP_FUNC_CONFIG video_path_param = {0};
		video_path_param.out_func = 0;
		if (g_capbind == 1) //direct mode
			video_path_param.out_func = HD_VIDEOCAP_OUTFUNC_DIRECT;
		if (g_capbind == 2) //one-buf mode
			video_path_param.out_func = HD_VIDEOCAP_OUTFUNC_ONEBUF;
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
		if ((HD_CTRL_ID)_out_id == HD_VIDEOPROC_0_CTRL) {
			video_cfg_param.isp_id = SEN1_VCAP_ID;
		} else {
			printf("unknown _out_id 0x%x\r\n", _out_id);
			return HD_ERR_NG;
		}
		video_cfg_param.ctrl_max.func = 0;
		if (g_shdr == 1) {
			video_cfg_param.ctrl_max.func |= HD_VIDEOPROC_FUNC_SHDR;
			video_cfg_param.in_max.pxlfmt = SHDR_CAP_OUT_FMT;
		} else {
			video_cfg_param.ctrl_max.func &= ~HD_VIDEOPROC_FUNC_SHDR;
			video_cfg_param.in_max.pxlfmt = g_capfmt;
		}
#if (FUNC_3DNR == ENABLE)
		video_cfg_param.ctrl_max.func |= HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA;
#endif
#if (FUNC_DEFOG == ENABLE)
		video_cfg_param.ctrl_max.func |= HD_VIDEOPROC_FUNC_DEFOG;
#endif
#if (FUNC_COLORNR == ENABLE)
		video_cfg_param.ctrl_max.func |= HD_VIDEOPROC_FUNC_COLORNR;
#endif
#if (FUNC_WDR == ENABLE)
		video_cfg_param.ctrl_max.func |= HD_VIDEOPROC_FUNC_WDR;
#endif
		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;
		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
		if (ret != HD_OK) {
			return HD_ERR_NG;
		}
	}
	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		video_path_param.in_func = 0;
		// video_path_param.ddr_id = VDO_DDR_ID;
		if (g_capbind == 1)
			video_path_param.in_func |= HD_VIDEOPROC_INFUNC_DIRECT; //direct NOTE: enable direct
		if (g_capbind == 2)
			video_path_param.in_func |= HD_VIDEOPROC_INFUNC_ONEBUF;
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}

	if ((HD_CTRL_ID)_out_id == HD_VIDEOPROC_0_CTRL) {
		video_ctrl_param.func = 0;
#if (FUNC_3DNR == ENABLE)
		video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_0_OUT_0;
#endif
	}
	if (g_shdr == 1) {
		video_ctrl_param.func |= HD_VIDEOPROC_FUNC_SHDR;
	} else {
		video_ctrl_param.func &= ~HD_VIDEOPROC_FUNC_SHDR;
	}
#if (FUNC_3DNR == ENABLE)
	video_ctrl_param.func |= HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA;
#endif
#if (FUNC_DEFOG == ENABLE)
	video_ctrl_param.func |= HD_VIDEOPROC_FUNC_DEFOG;
#endif
#if (FUNC_COLORNR == ENABLE)
	video_ctrl_param.func |= HD_VIDEOPROC_FUNC_COLORNR;
#endif
#if (FUNC_WDR == ENABLE)
	video_ctrl_param.func |= HD_VIDEOPROC_FUNC_WDR;
#endif

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

	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};
		video_path_param.in_func = 0;
		if (g_prcbind == 1)
			video_path_param.out_func |= HD_VIDEOPROC_OUTFUNC_LOWLATENCY; //enable low-latency
		if (g_prcbind == 2)
			video_path_param.out_func |= HD_VIDEOPROC_OUTFUNC_ONEBUF;
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

	if (g_capbind == 1 && g_prcbind == 1 && vdo_size_w < 5120) {
		VENDOR_VIDEOENC_TILE_SPLIT_MODE_CFG tile_split = {0};
		tile_split.mode = VENDOR_VIDEOENC_ONE_TILE_SPLIT;
		ret = vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_TILE_SPLIT_MODE, &tile_split);
		if (ret != HD_OK) {
			printf("set VENDOR_VIDEOENC_PARAM_OUT_TILE_SPLIT_MODE fail = %d\r\n", ret);
			return HD_ERR_NG;
		}
	}

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

	{
		video_func_config.in_func = 0;
		if (g_prcbind == 1)
			video_func_config.in_func |= HD_VIDEOENC_INFUNC_LOWLATENCY; //enable low-latency
		if (g_prcbind == 2)
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
	HD_H26XENC_RATE_CONTROL rc_param = {0};

	if (p_dim != NULL) {

		//--- HD_VIDEOENC_PARAM_IN ---
		video_in_param.dir     = HD_VIDEO_DIR_NONE;
		video_in_param.pxl_fmt = g_prcfmt;
		video_in_param.dim.w   = p_dim->w;
		video_in_param.dim.h   = p_dim->h;
		video_in_param.frc     = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_IN, &video_in_param);
		if (ret != HD_OK) {
			printf("set_enc_param_in = %d\r\n", ret);
			return ret;
		}

		printf("enc_type=%d\r\n", enc_type);

#if SHOW_BBOX_IN_VLC
		{
			VENDOR_VIDEOENC_SMART_ROI smart_roi = {0};
			smart_roi.enable = 1;
			smart_roi.mode = 0xFF;
			vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_SMART_ROI, &smart_roi);
		}
#endif

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

static HD_RESULT open_module(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim, UINT pull_out_id)
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
	// if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_1, &p_stream->proc_path2)) != HD_OK)
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_PATH(HD_DAL_VIDEOPROC(0), HD_IN(0), HD_OUT(pull_out_id)), &p_stream->proc_path2)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_VIDEOENC_0_OUT_0, &p_stream->enc_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT close_module(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;

	if ((ret = hd_videoenc_close(p_stream->enc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_close(p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_close(p_stream->proc_path2)) != HD_OK)
		return ret;
	if ((ret = hd_videocap_close(p_stream->cap_path)) != HD_OK)
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


EXAMFUNC_ENTRY(hd_video_record_with_vsp, argc, argv)
{
	HD_RESULT ret;
	INT key = -1;
	VIDEO_RECORD stream[1] = {0};
	UINT32  enc_type = 0; //H264
	PD_SHM_INFO  *p_pd_shm;
	FILE* p_child_process = NULL;
    UINT pull_out_path_id = 1;  // default video frame pull out path id: 1

	// query program options
	if (argc == 1) {
		printf("Usage: <fps> <enc_type>.\r\n");
		printf("Help:\r\n");
		printf("  <fps>  : 20,25,30\r\n");
		printf("  <enc_type>  : 0(H265), 1(H264), 2(MJPG)\r\n");
		// return 0;
	}
	if (argc >= 2) {
		g_fps = atoi(argv[1]);
	}
	if (argc >= 3) {
		enc_type = atoi(argv[2]);
	}
    if (argc >= 4){
        pull_out_path_id = atoi(argv[3]);
    }

#if (FUNC_AI == ENABLE)
	if ((ret = demo_read_system_cfg()) != HD_OK){
		ret = HD_OK; // Still continue run even if cfg config file parse failed.
	}
#endif

	// Adjust bitrate base on fps
	if (g_fps != 30) {
		vdo_br = vdo_br * g_fps / 30;
	}

	// assign g_capfmt
	if (g_capfmt == 0) {
		g_capfmt = HD_VIDEO_PXLFMT_RAW12;
	} else {
		g_capfmt = HD_VIDEO_PXLFMT_NRX12;
	}

	// assign g_prcfmt
	if (g_prcfmt == 0) {
		g_prcfmt = HD_VIDEO_PXLFMT_YUV420;
	} else {
		g_prcfmt = HD_VIDEO_PXLFMT_YUV420_NVX2;
	}

	// Sensor setting
	if (sensor_sel == SEN_SEL_IMX290) {
		vdo_size_w = VDO_SIZE_W_2M;
		vdo_size_h = VDO_SIZE_H_2M;
		vdo_br = 2 * 1024 *1024;
	} else if (sensor_sel == SEN_SEL_OS02K10) {
		vdo_size_w = VDO_SIZE_W_2M;
		vdo_size_h = VDO_SIZE_H_2M;
		vdo_br = 2 * 1024 *1024;
		g_capbind = 0;
	} else if (sensor_sel == SEN_SEL_OS04A10) {
		vdo_size_w = VDO_SIZE_W_4M_2;
		vdo_size_h = VDO_SIZE_H_4M_2;
		vdo_br = 4 * 1024 *1024;
	} else if (sensor_sel == SEN_SEL_OS04C10) {
		vdo_size_w = VDO_SIZE_W_4M;
		vdo_size_h = VDO_SIZE_H_4M;
		vdo_br = 4 * 1024 *1024;
	} else if (sensor_sel == SEN_SEL_OS05A10) {
		vdo_size_w = VDO_SIZE_W_5M;
		vdo_size_h = VDO_SIZE_H_5M;
		vdo_br = 5 * 1024 *1024;
		g_main_ltr_en = 0;
	} else if (sensor_sel == SEN_SEL_SC450AI) {
		vdo_size_w = VDO_SIZE_W_4M_2;
		vdo_size_h = VDO_SIZE_H_4M_2;
		vdo_br = 4 * 1024 *1024;
	} else if (sensor_sel == SEN_SEL_IMX415) {
		vdo_size_w = VDO_SIZE_W_8M;
		vdo_size_h = VDO_SIZE_H_8M;
		vdo_br = 8 * 1024 *1024;
		g_capbind = 0;
		g_main_ltr_en = 0;
		if (g_fps >= 20) {
			g_prcbind = 2; // one buf
		}
	}

	// init hdal
	ret = hd_common_init(0);
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
		goto exit;
	}

	// set videoproc sysconfig
	if (g_prccfg == 0) {
		hd_common_sysconfig(HD_VIDEOPROC_CFG, 0, HD_VIDEOPROC_CFG_STRIP_LV1, 0); //default
	} else if (g_prccfg == 1) {
		hd_common_sysconfig(HD_VIDEOPROC_CFG, 0, HD_VIDEOPROC_CFG_STRIP_LV2, 0);
	} else if (g_prccfg == 2) {
		hd_common_sysconfig(HD_VIDEOPROC_CFG, 0, HD_VIDEOPROC_CFG_STRIP_LV3, 0);
	} else if (g_prccfg == 3) {
		hd_common_sysconfig(HD_VIDEOPROC_CFG, 0, HD_VIDEOPROC_CFG_STRIP_LV4, 0);
	} else if (g_prccfg == 4) {
		hd_common_sysconfig(HD_VIDEOPROC_CFG, 0, HD_VIDEOPROC_CFG_DISABLE_GDC, 0);
	}

#if (FUNC_AI == ENABLE)
	// init memory
	printf("[hd_demo_turnkey] all bufsize: %ld \r\n", cfg_bufsize.total_bufsize);

	ret = mem_init(cfg_bufsize.total_bufsize);
	if (ret != HD_OK) {
		printf("[hd_demo_turnkey] mem init fail=%d\n", ret);
		goto exit;
	}
#else
	ret = mem_init(0);
	if (ret != HD_OK) {
		printf("[hd_demo_turnkey] mem init fail=%d\n", ret);
		goto exit;
	}
#endif

	// init all modules
	ret = init_module();
	if (ret != HD_OK) {
		printf("init fail=%d\n", ret);
		goto exit;
	}

	init_share_memory();

	// open video liview modules (sensor 1st)
	stream[0].proc_max_dim.w = vdo_size_w; //assign by user
	stream[0].proc_max_dim.h = vdo_size_h; //assign by user
	stream[0].wait_ms = -1; //blocking mode
	ret = open_module(&stream[0], &stream[0].proc_max_dim, pull_out_path_id);
	if (ret != HD_OK) {
		printf("open fail=%d\n", ret);
		goto exit;
	}

	// get videocap capability (sensor 1st)
	ret = get_cap_caps(stream[0].cap_ctrl, &stream[0].cap_syscaps);
	if (ret != HD_OK) {
		printf("get cap-caps fail=%d\n", ret);
		goto exit;
	}

	// set videocap parameter (sensor 1st)
	stream[0].cap_dim.w = vdo_size_w; //assign by user
	stream[0].cap_dim.h = vdo_size_h; //assign by user
	ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim);
	if (ret != HD_OK) {
		printf("set cap fail=%d\n", ret);
		goto exit;
	}

	// set videoproc parameter (sensor 1st)
	ret = set_proc_param(stream[0].proc_path, &stream[0].proc_max_dim);
	if (ret != HD_OK) {
		printf("set proc fail=%d\n", ret);
		goto exit;
	}

	// set videoproc substream parameter
	HD_DIM sub_dim;
	sub_dim.w = subvdo_size_w;
	sub_dim.h = subvdo_size_h;
	ret = set_proc_param(stream[0].proc_path2, &sub_dim);
	if (ret != HD_OK) {
		printf("set proc fail=%d\n", ret);
		goto exit;
	}

	// set videoenc config (sensor 1)
	stream[0].enc_max_dim.w = vdo_size_w;
	stream[0].enc_max_dim.h = vdo_size_h;
	ret = set_enc_cfg(stream[0].enc_path, &stream[0].enc_max_dim, vdo_br, SEN1_VCAP_ID);
	if (ret != HD_OK) {
		printf("set enc-cfg fail=%d\n", ret);
		goto exit;
	}

	// set videoenc parameter (sensor 1)
	stream[0].enc_dim.w = vdo_size_w;
	stream[0].enc_dim.h = vdo_size_h;
	ret = set_enc_param(stream[0].enc_path, &stream[0].enc_dim, enc_type, vdo_br);
	if (ret != HD_OK) {
		printf("set enc fail=%d\n", ret);
		goto exit;
	}

	// bind modules (sensor 1st)
	hd_videocap_bind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);
	hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOENC_0_IN_0);

	// start  modules (sensor 1st)
	if (g_capbind == 1) {
		// direct NOTE: ensure videocap start after 1st videoproc phy path start
		hd_videoproc_start(stream[0].proc_path);
		hd_videoproc_start(stream[0].proc_path2);
		hd_videocap_start(stream[0].cap_path);
	} else {
		hd_videocap_start(stream[0].cap_path);
		hd_videoproc_start(stream[0].proc_path);
		hd_videoproc_start(stream[0].proc_path2);
	}
	hd_videoenc_start(stream[0].enc_path);

	// query user key
	printf("Enter q to exit\n");
	printf("Enter d to debug\n");
	printf("Enter c to start start subprocess\n");
	printf("Enter z to start rtsp\n");
	while (1) {

		key = GETCHAR();
		if (key == 'q' || key == 0x3) {
			g_quit = 1; // stop thread
			// notify child process
			p_pd_shm = (PD_SHM_INFO  *)g_shm;
			p_pd_shm->exit = 1;
			if (p_child_process != NULL) {
				fprintf(p_child_process, "q\n");
				if (ferror(p_child_process)){
					printf("Cannot terminate sub process!\r\n");
				}
				if (pclose(p_child_process) != 0){
					printf("Cannot close sub process!.\n");
				}else{
					p_child_process = NULL;
				}
			}
			usleep(300000);
			break;
		}
#if (DEBUG_MENU == 1)
		if (key == 'd') {
			// enter debug menu
			hd_debug_run_menu();
			printf("\r\nEnter q to exit, Enter d to debug\r\n");
		}
#endif
		if (key == 'c') {

#if (FUNC_AI == ENABLE)
			// If no buffer for ai is initiated, it won't start subprocess
			if(cfg_bufsize.total_bufsize == 0) {
				printf("buf_size %d !!!\r\n", cfg_bufsize.total_bufsize);
				printf("No given buffer for subprocess, demo won't start!!\r\n");
				continue;
			}

			static int started = 0;
			if (started) {
				printf("sub process already started.\r\n");
				continue;
			}
			started = 1;
			printf("start sub process......\r\n");

			CHAR command[256];
			HD_RESULT arg_encode;
			arg_encode = sub_process_arg_encode(turnkey_name, command, sizeof(command)/sizeof(CHAR), pull_out_path_id);
			printf("command = %s \n",command);
			if (arg_encode == HD_OK) {
				p_child_process = popen(command, "w");
				if (p_child_process == NULL)
					printf("Turnkey process is not initiated!\r\n");
			} else {
				started = 0;
				printf("Turnkey process is not initiated!\r\n");
				continue;
			}
#else
			p_child_process = NULL;
			printf("No subprocess logic in the sample code!!\r\n");
#endif
		}
		
		if (key == 'z') {
			g_enc_poll = 0;
			system("nvtrtspd_ipc &");
			printf("nvtrtspd_ipc\r\n");
		}
	}

	sleep(1);

	// stop video_liveview modules (sensor 1st)
	if (g_capbind == 1) {
		// direct NOTE: ensure videocap stop after all videoproc path stop
		hd_videoproc_stop(stream[0].proc_path);
		hd_videoproc_stop(stream[0].proc_path2);
		hd_videocap_stop(stream[0].cap_path);
	} else {
		hd_videocap_stop(stream[0].cap_path);
		hd_videoproc_stop(stream[0].proc_path);
		hd_videoproc_stop(stream[0].proc_path2);
	}
	hd_videoenc_stop(stream[0].enc_path);

	// unbind  modules (sensor 1st)
	hd_videocap_unbind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0));
	hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_0);

exit:

	// close modules (sensor 1st)
	ret = close_module(&stream[0]);
	if (ret != HD_OK) {
		printf("close fail=%d\n", ret);
	}

	// uninit all modules
	ret = exit_module();
	if (ret != HD_OK) {
		printf("exit fail=%d\n", ret);
	}

	exit_share_memory();

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
