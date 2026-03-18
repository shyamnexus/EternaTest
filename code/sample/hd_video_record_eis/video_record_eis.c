/**
	@brief Sample code of video record with eis.\n

	@file video_record_eis.c

	@author Janice Huang

	@ingroup mhdal


	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "hdal.h"
#include "hd_debug.h"
#include "vendor_videoprocess.h"
#include "vendor_videocapture.h"
#include "vendor_videoenc.h"
#include "vendor_eis.h"
#include "vendor_vpe.h"
#include <pd_shm.h>
#include "vendor_isp.h"
#include <math.h>
// platform dependent
#if defined(__LINUX)
#include <pthread.h>			//for pthread API
#define MAIN(argc, argv) 		int main(int argc, char** argv)
#define GETCHAR()				getchar()
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/prctl.h>
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <kwrap/util.h>		//for sleep API
#define sleep(x)    			vos_util_delay_ms(1000*(x))
#define msleep(x)    			vos_util_delay_ms(x)
#define usleep(x)   			vos_util_delay_us(x)
#include <kwrap/examsys.h> 	//for MAIN(), GETCHAR() API
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(hd_video_record_with_vprc_eis, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define DEBUG_MENU 		1

#define CHKPNT			printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
#define DBGH(x)			printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x)			printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)
#define BIND_BSMUXER    0
#define GYRO_FILE_MAX  (64*1024*1024)
///////////////////////////////////////////////////////////////////////////////

//header
#define DBGINFO_BUFSIZE()	(0x200)

//RAW
#define VDO_RAW_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
//NRX: RAW compress: Only support 12bit mode
#define RAW_COMPRESS_RATIO 41
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
#define VIDEOCAP_ALG_FUNC HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB
#define VIDEOPROC_ALG_FUNC HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_COLORNR

#define SEN_OUT_FMT		HD_VIDEO_PXLFMT_NRX12
#define CAP_OUT_FMT		HD_VIDEO_PXLFMT_NRX12
#define IPP_OUT_FMT     HD_VIDEO_PXLFMT_YUV420
#define VPE_OUT_FMT     HD_VIDEO_PXLFMT_YUV420 //HD_VIDEO_PXLFMT_YUV420_NVX4
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

#define RESOLUTION_SET  5 //0: 2M(IMX290), 1:5M(OS05A), 2: 2M (OS02K10), 3: 2M (AR0237IR), 4: 8M (IMX415), 5: 4M (OS04A10)

#define SEN_SEL_IMX290   0   //2M_SHDR
#define SEN_SEL_OS05A10  1   //5M_SHDR
#define SEN_SEL_OS02K10  2
#define SEN_SEL_AR0237IR 3
#define SEN_SEL_IMX415   4   //8M_SHDR
#define SEN_SEL_OS04A10  5   //4M_SHDR
#define SEN_SEL_PATGEN   99

#define VDO_SIZE_W_2M	   1920
#define VDO_SIZE_H_2M	   1080
#define VDO_BITRATE_2M          (4 * 1024 * 1024)
#define VDO_BITRATE_RTSP_2M     (2 * 1024 * 1024)

#define VDO_SIZE_W_4M      2560
#define VDO_SIZE_H_4M      1440
#define VDO_BITRATE_4M          (8 * 1024 * 1024)
#define VDO_BITRATE_RTSP_4M     (4 * 1024 * 1024)

#define VDO_SIZE_W_8M      3840
#define VDO_SIZE_H_8M      2160
#define VDO_BITRATE_8M          (16 * 1024 * 1024)
#define VDO_BITRATE_RTSP_8M     (8 * 1024 * 1024)

static UINT32 VDO_SIZE_W=0;
static UINT32 VDO_SIZE_H=0;

#define SUB_VDO_SIZE_W	1280
#define SUB_VDO_SIZE_H	720

#define GYRO_DATA_NUM   32
#define GYRO_DATA_SIZE (4+4*GYRO_DATA_NUM*6)

//#define EIS_PATH0_2DLUT_SIZE VENDOR_EIS_LUT_9x9
#define EIS_PATH0_2DLUT_SIZE VENDOR_EIS_LUT_65x65
#define ISP_EFFECT_ID 0
static UINT32 g_shdr = 0; //fixed
static UINT32 g_enc_poll = 1;
static UINT32 g_write_file = 0;
static UINT32 sensor_sel = SEN_SEL_IMX290;
static UINT32 vdo_br     = (VDO_BITRATE_2M);
static UINT32 sub_vdo_br     = (VDO_BITRATE_2M);
static UINT32 vdo_br_rtsp     = (VDO_BITRATE_RTSP_2M);

static UINT32 g_capbind = 0;  //0:D2D, 1:direct, 2: one-buf, 0xff: no-bind
static UINT32 g_vcap_blk_size = 0;
static UINT32 g_show_gyro = 0;
static UINT32 g_show_acc = 0;

static UINT32 g_remap_vcap_sie = 0;
#if ( RESOLUTION_SET == 4)
#define SEN1_VCAP_ID 0
#else
#define SEN1_VCAP_ID 0
#endif

#define DISPLAY             0
#define DISPLAY_SHARE_BUF   0
#define VCAP_SIE_MAP 1  // 0 for SIE1, 1 for SIE2
#define ISP_ID_REMAP_SIE(vcap_id, sie_map_id)  ((1 << 31) | (vcap_id << 24) | sie_map_id)
#define VPRC_PI 3.1415926

#define WRITE_GYRO_DATA 0

static char   *g_shm = NULL;
static int    g_shmid = 0;
static UINT32 g_fps = 30;

static UINT32 g_gyro_num = GYRO_DATA_NUM;

static double normalized_default_2dlut[2][9][9] =
{
	{//X
		{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9},
		{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9},
		{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9},
		{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9},
		{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9},
		{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9},
		{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9},
		{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9},
		{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9}
	},
	{//Y
		{0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1},
		{0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2},
		{0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3},
		{0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4},
		{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5},
		{0.6, 0.6, 0.6, 0.6, 0.6, 0.6, 0.6, 0.6, 0.6},
		{0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7},
		{0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8},
		{0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9}
	}
};

static void gen_default_2dlut(UINT32 *p_lut2d_buf, INT32 width, INT32 height, INT32 lut_size)
{
	INT32 i, j;
	UINT32 line_data_num = ((lut_size+3)>>2)<<2;

	for (j = 0; j < lut_size; j++) {
		for (i = 0; i < lut_size; i++) {
			p_lut2d_buf[j*line_data_num + i] =
				((UINT32)round(normalized_default_2dlut[1][j][i]*(double)height*4) << 16) +
				(UINT32)round(normalized_default_2dlut[0][j][i]*(double)width*4);
		}
	}
}
static VPET_2DLUT_PARAM lut2d_path0 = {
	0,
	{
		EIS_PATH0_2DLUT_SIZE,
		{0}
	}
};
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
	p_pd_shm->enc_path[0].w = VDO_SIZE_W;
	p_pd_shm->enc_path[0].h = VDO_SIZE_H;
	p_pd_shm->enc_path[1].w = SUB_VDO_SIZE_W;
	p_pd_shm->enc_path[1].h = SUB_VDO_SIZE_H;
}

void exit_share_memory(void)
{
	if (g_shm) {
		shmdt(g_shm);
		shmctl(g_shmid, IPC_RMID, NULL);
	}
}


static HD_RESULT mem_init(void)
{
	HD_RESULT              ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
    UINT32 id;

	// config common pool (cap)
	id = 0;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_RAW_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, CAP_OUT_FMT)
														+VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
														+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
	mem_cfg.pool_info[id].blk_size += GYRO_DATA_SIZE;
	mem_cfg.pool_info[id].blk_cnt = 3;
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;
	// config common pool (pre-process)
	id++;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[id].blk_size += (vendor_eis_buf_query(VENDOR_EIS_LUT_65x65) + GYRO_DATA_SIZE + sizeof(VENDOR_EIS_DBG_CTX));
	mem_cfg.pool_info[id].blk_cnt = 5;
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;
	// config common pool (main)
	id++;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[id].blk_cnt = 3;
	if (g_capbind == 1) {
		//direct
		mem_cfg.pool_info[id].blk_cnt += 2; //direct will pre-new 1
	}
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;
	// config common pool (sub)
	id++;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(SUB_VDO_SIZE_W, SUB_VDO_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[id].blk_cnt = 5;
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;
#if DISPLAY
	// config common pool (display)
	id ++;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[id].blk_cnt = 5;
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;
#endif
    // gyro data
	id ++;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
	mem_cfg.pool_info[id].blk_size = GYRO_FILE_MAX;
	mem_cfg.pool_info[id].blk_cnt = 1;
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;

	ret = hd_common_mem_init(&mem_cfg);

	g_vcap_blk_size = mem_cfg.pool_info[0].blk_size;

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

    if (sensor_sel == SEN_SEL_IMX290) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_imx290");
		printf("Using nvt_sen_imx290\n");
	} else if (sensor_sel == SEN_SEL_OS05A10) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os05a10");
		printf("Using nvt_sen_os05a10\n");
	} else if (sensor_sel == SEN_SEL_OS02K10) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os02k10");
		printf("Using nvt_sen_os02k10\n");
	} else if (sensor_sel == SEN_SEL_AR0237IR) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_ar0237ir");
		printf("Using nvt_sen_ar0237ir\n");
	} else if (sensor_sel == SEN_SEL_IMX415) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_imx415");
		printf("Using nvt_sen_imx415\n");
	} else {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os04a10");
		printf("Using nvt_sen_os04a10\n");
	}

	if (sensor_sel == SEN_SEL_PATGEN) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, HD_VIDEOCAP_SEN_PAT_GEN);
	}


    if(sensor_sel == 3) {
        cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_P_RAW;
	    cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux =  0;  //use @0 in peri-dev.dtsi
	    printf("Parallel interface\n");
    }
    if(sensor_sel == 0 || sensor_sel == 1 || sensor_sel == 2 || sensor_sel == 4 || sensor_sel == 5) {
	    cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
	    cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux =  0;  //use @0 in peri-dev.dtsi
	    printf("MIPI interface\n");
    }
	if (g_shdr == 1) {
		printf("Using g_shdr_mode\n");
	} else {
		if (sensor_sel == SEN_SEL_IMX290) {
			printf("Using imx290\n");
		} else if (sensor_sel == SEN_SEL_OS05A10) {
			printf("Using OS05A\n");
		} else if (sensor_sel == SEN_SEL_OS02K10) {
			printf("Using OS02K10\n");
		} else if (sensor_sel == SEN_SEL_IMX415) {
			printf("Using IMX415\n");
		} else if (sensor_sel == SEN_SEL_OS04A10) {
			printf("Using OS04A10\n");
		}
	}
	#if (SEN1_VCAP_ID == 0)
	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(0, 0);
	#else //(SEN1_VCAP_ID == 2)
	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(1, 0);
	#endif
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
	if (g_shdr == 1) {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
	} else {
		if (sensor_sel == 0 || sensor_sel == 4 || sensor_sel == 5) {
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
		} else if (sensor_sel == 1) {
			printf("Using OS052A or shdr\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
		} else if (sensor_sel == 2) {
			printf("Using OS02K10 or shdr\n");
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
			cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
		}
	}
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = HD_VIDEOCAP_SEN_IGNORE;
	ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(SEN1_VCAP_ID), &video_cap_ctrl); //open this for device control

	if (ret != HD_OK) {
		return ret;
	}

	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);

	if (sensor_sel != SEN_SEL_PATGEN) {
		iq_ctl.func = VIDEOCAP_ALG_FUNC;

		ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);
	}

	if (g_remap_vcap_sie) {
		UINT32 sie_map = VCAP_SIE_MAP;
		//56x only support compression in SIE2
		vendor_videocap_set(video_cap_ctrl, VENDOR_VIDEOCAP_PARAM_SIE_MAP, &sie_map);
	}

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

		if (sensor_sel == SEN_SEL_PATGEN) {// pattern gen
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
		video_crop_param.win.rect.w = 1920/2;
		video_crop_param.win.rect.h= 1080/2;
		video_crop_param.align.w = 4;
		video_crop_param.align.h = 4;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT_CROP, &video_crop_param);
		//printf("set_cap_param CROP ON=%d\r\n", ret);
	}
	#endif
	{
		HD_VIDEOCAP_OUT video_out_param = {0};

		//without setting dim for no scaling, using original sensor out size
		video_out_param.pxlfmt = CAP_OUT_FMT;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		if (g_capbind == 0xff) //no-bind mode
			video_out_param.depth = 1;
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
	{
		VENDOR_VIDEOCAP_GYRO_INFO vcap_gyro_info = {0};

		vcap_gyro_info.en = TRUE;
		vcap_gyro_info.data_num = g_gyro_num;
		ret = vendor_videocap_set(video_cap_path, VENDOR_VIDEOCAP_PARAM_GYRO_INFO, &vcap_gyro_info);
		//printf("set_cap_param OUT=%d\r\n", ret);
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////

static HD_RESULT set_proc_cfg(HD_OUT_ID ctrl, HD_PATH_ID *p_video_proc_ctrl, HD_VIDEOPROC_PIPE pipe, HD_DIM* p_max_dim)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
	HD_VIDEOPROC_CTRL video_ctrl_param = {0};
	HD_PATH_ID video_proc_ctrl = 0;

	ret = hd_videoproc_open(0, ctrl, &video_proc_ctrl); //open this for device control
	if (ret != HD_OK)
		return ret;

	if (p_max_dim != NULL ) {
		video_cfg_param.pipe = pipe;
		if (pipe == HD_VIDEOPROC_PIPE_RAWALL) {
    		if ((HD_CTRL_ID)ctrl == HD_VIDEOPROC_0_CTRL) {
    			video_cfg_param.isp_id = SEN1_VCAP_ID;
    		} else {
    			video_cfg_param.isp_id = 0;
    		}

		} else { //HD_VIDEOPROC_PIPE_VPE
			video_cfg_param.isp_id = ISP_EFFECT_ID;
		}
		if (pipe == HD_VIDEOPROC_PIPE_RAWALL) {
			video_cfg_param.ctrl_max.func = VIDEOPROC_ALG_FUNC;
		} else {
			video_cfg_param.ctrl_max.func = 0;
		}
		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;
		video_cfg_param.in_max.pxlfmt = (pipe == HD_VIDEOPROC_PIPE_VPE) ? HD_VIDEO_PXLFMT_YUV420 : CAP_OUT_FMT;
		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
		if (ret != HD_OK) {
			return HD_ERR_NG;
		}
	}
	if (pipe == HD_VIDEOPROC_PIPE_RAWALL) {
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};
		BOOL eis_func = TRUE;

		video_path_param.in_func = 0;
		if (g_capbind == 1)
			video_path_param.in_func |= HD_VIDEOPROC_INFUNC_DIRECT; //direct NOTE: enable direct
		if (g_capbind == 2)
			video_path_param.in_func |= HD_VIDEOPROC_INFUNC_ONEBUF;
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
		//printf("set_proc_param PATH_CONFIG=0x%X\r\n", ret);

		vendor_videoproc_set(video_proc_ctrl, VENDOR_VIDEOPROC_PARAM_EIS_FUNC, &eis_func);

		video_ctrl_param.func = VIDEOPROC_ALG_FUNC;
		video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_0_OUT_0;
	} else {
		video_ctrl_param.func = 0;
	}
	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);
	*p_video_proc_ctrl = video_proc_ctrl;

	return ret;
}

static HD_RESULT set_proc_param(HD_PATH_ID video_proc_path, HD_DIM* p_dim, HD_VIDEO_PXLFMT pxlfmt)
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
		video_path_config.max_mem.svc_layer  = HD_SVC_DISABLE;
		video_path_config.max_mem.ltr        = FALSE;
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
	HD_VIDEOENC_OUT video_out_param = {0};
	HD_H26XENC_RATE_CONTROL rc_param = {0};
	VENDOR_VIDEOENC_LONG_START_CODE lsc = {0};

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

		printf("enc_type=%d bitrate=%d\r\n", enc_type,bitrate);

		if (enc_type == 0) {

			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_H265;
			video_out_param.h26x.profile       = HD_H265E_MAIN_PROFILE;
			video_out_param.h26x.level_idc     = HD_H265E_LEVEL_5;
			video_out_param.h26x.gop_num       = 30;
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
			rc_param.cbr.frame_rate_base = g_fps;
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

			//--- VENDOR_VIDEOENC_PARAM_OUT_LONG_START_CODE ---
			lsc.long_start_code_en       = 1;
			ret = vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_LONG_START_CODE, &lsc);
			if (ret != HD_OK) {
				printf("set_enc_long_start_code = %d\r\n", ret);
			}
		} else if (enc_type == 1) {

			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_H264;
			video_out_param.h26x.profile       = HD_H264E_HIGH_PROFILE;
			video_out_param.h26x.level_idc     = HD_H264E_LEVEL_5_1;
			video_out_param.h26x.gop_num       = 30;
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
			rc_param.cbr.frame_rate_base = g_fps;
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
///////////////////////////////////////////////////////////////////////////////
#if (BIND_BSMUXER == 1)
#include "hd_bsmux_lib.h"
#include "hd_fileout_lib.h"
#include "sdio.h"
#include "FileSysTsk.h"

typedef struct _BSMUXER_STREAM {

	// (1)
	HD_PATH_ID bsmux_path;
	HD_VIDEOENC_BUFINFO bsmux_vencbufinfo;
	HD_AUDIOENC_BUFINFO bsmux_aencbufinfo;
	HD_BSMUX_VIDCODEC vidcodec;

	// (2)
	HD_PATH_ID fileout_path;

	// (3)
	CHAR drive;
	unsigned long filesys_pa;
	unsigned long filesys_va;
	unsigned long filesys_size;

} BSMUXER_STREAM;

INT32 bsmux_callback_func(CHAR *p_name, HD_BSMUX_CBINFO *cbinfo, UINT32 *param)
{
	HD_BSMUX_CB_EVENT event = cbinfo->cb_event;
	switch (event) {
	case HD_BSMUX_CB_EVENT_FOUTREADY: //ready ops buf
		{
			HD_PATH_ID fileout_path = cbinfo->id;
			HD_FILEOUT_BUF *fout_buf = (HD_FILEOUT_BUF *)cbinfo->out_data;
			if (hd_fileout_push_in_buf(fileout_path, fout_buf, -1) != HD_OK)
				printf("hd_fileout_push_in_buf fail\r\n");
		}
		break;
	default:
		break;
	}
	return 0;
}

INT32 fileout_callback_func(CHAR *p_name, HD_FILEOUT_CBINFO *cbinfo, UINT32 *param)
{
	HD_FILEOUT_CB_EVENT event = cbinfo->cb_event;
	switch (event) {
	case HD_FILEOUT_CB_EVENT_NAMING:
		{
			time_t time_sec = time(0);
			struct tm cur_time;
			localtime_r(&time_sec, &cur_time);
			cbinfo->fpath_size = 128;
			snprintf(cbinfo->p_fpath, cbinfo->fpath_size, "A:\\%04d%02d%02d-%02d%02d%02d_%02d.mp4",
				cur_time.tm_year+1900, cur_time.tm_mon+1, cur_time.tm_mday,
				cur_time.tm_hour, cur_time.tm_min, cur_time.tm_sec,(int)cbinfo->iport);
		}
		break;
	case HD_FILEOUT_CB_EVENT_CLOSED:
		break;
	case HD_FILEOUT_CB_EVENT_FS_ERR:
		printf("fs error\r\n");
		break;
	default:
		break;
	}
	return 0;
}

static HD_RESULT set_bsmuxer_config(BSMUXER_STREAM *p_stream)
{
	HD_BSMUX_VIDEOINFO  video_info = {0};
	HD_BSMUX_AUDIOINFO  audio_info = {0};
	HD_BSMUX_FILEINFO   file_info = {0};
	HD_BSMUX_BUFINFO    buf_info = {0};

	// (1) VIDEOINFO
	video_info.vidcodec      = p_stream->vidcodec; //user config
	video_info.vfr           = g_fps;
    if(p_stream->bsmux_path==0) {
	video_info.width         = VDO_SIZE_W;
	video_info.height        = VDO_SIZE_H;

    }else {
	video_info.width         = SUB_VDO_SIZE_W;
	video_info.height        = SUB_VDO_SIZE_H;
    }
	video_info.tbr           = vdo_br; // 2 Mb/s
	video_info.gop           = 30;
	hd_bsmux_set(p_stream->bsmux_path, HD_BSMUX_PARAM_VIDEOINFO, (VOID *)&video_info);
	// (2) AUDIOINFO
	audio_info.codectype     = HD_BSMUX_AUDCODEC_PCM;
	audio_info.chs           = 2;
	audio_info.asr           = 16000;
	audio_info.adts_bytes    = 7;
	audio_info.aud_en        = 0; //disable
	hd_bsmux_set(p_stream->bsmux_path, HD_BSMUX_PARAM_AUDIOINFO, (VOID *)&audio_info);
	// (3) FILEINFO
	file_info.seamlessSec    = 300;
	file_info.rollbacksec    = 3;
	file_info.keepsec        = 5;
	file_info.filetype       = HD_BSMUX_FTYPE_MP4;
	file_info.recformat      = HD_BSMUX_RECFORMAT_VID_ONLY;
	file_info.revsec         = 10;
	file_info.seamlessSec_ms = HD_BSMUX_SET_MS(300, 0);
	file_info.rollbacksec_ms = HD_BSMUX_SET_MS(3, 0);
	file_info.keepsec_ms     = HD_BSMUX_SET_MS(5, 0);
	file_info.revsec_ms      = HD_BSMUX_SET_MS(10, 0);
	hd_bsmux_set(p_stream->bsmux_path, HD_BSMUX_PARAM_FILEINFO, (VOID *)&file_info);
	HD_BSMUX_EN_UTIL dur_info = {0};
	dur_info.enable = 1;
	dur_info.type = HD_BSMUX_EN_UTIL_DUR_LIMIT;
	dur_info.resv[0] = 13 * 1000000;
	hd_bsmux_set(p_stream->bsmux_path,HD_BSMUX_PARAM_EN_UTIL, (VOID *)&dur_info);
	// (4) BUFINFO
	buf_info.videnc.phy_addr = p_stream->bsmux_vencbufinfo.buf_info.phy_addr;
	buf_info.videnc.buf_size = p_stream->bsmux_vencbufinfo.buf_info.buf_size;
	hd_bsmux_set(p_stream->bsmux_path, HD_BSMUX_PARAM_BUFINFO, (VOID *)&buf_info);
	// (5) CALLBACK
	hd_bsmux_set(p_stream->bsmux_path, HD_BSMUX_PARAM_REG_CALLBACK, (VOID *)bsmux_callback_func);
	hd_fileout_set(p_stream->fileout_path, HD_FILEOUT_PARAM_REG_CALLBACK, (VOID *)fileout_callback_func);

	return HD_OK;
}

static HD_RESULT init_filesys_module(BSMUXER_STREAM *p_stream)
{
	int    ret;
	FILE_TSK_INIT_PARAM     Param = {0};
	FS_HANDLE               StrgDXH;

	// (1) filesys
	p_stream->drive = 'A';
	p_stream->filesys_size = (ALIGN_CEIL_64(0x4000));  //for linux fs cmd
	if (hd_common_mem_alloc("FsLib", &p_stream->filesys_pa, (void **)&p_stream->filesys_va, p_stream->filesys_size, DDR_ID0) != HD_OK) {
		printf("%s mem alloc fail\r\n", __func__);
		return HD_ERR_NG;
	}
	FileSys_InstallID(FileSys_GetOPS_Linux());
	if (FST_STA_OK != FileSys_Init(FileSys_GetOPS_Linux())) {
		printf("FileSys_Init failed\r\n");
	}
	memset(&Param, 0, sizeof(FILE_TSK_INIT_PARAM));
	StrgDXH = (FS_HANDLE)sdio_getStorageObject(STRG_OBJ_FAT1);
	Param.FSParam.WorkBuf = p_stream->filesys_va;
	Param.FSParam.WorkBufSize = p_stream->filesys_size;
	strncpy(Param.FSParam.szMountPath, "/mnt/sd", sizeof(Param.FSParam.szMountPath) - 1); //only used by FsLinux
	Param.FSParam.szMountPath[sizeof(Param.FSParam.szMountPath) - 1] = '\0';
	Param.FSParam.MaxOpenedFileNum = 10;
	ret = FileSys_OpenEx(p_stream->drive, StrgDXH, &Param);
	if (FST_STA_OK != ret) {
		printf("FileSys_Open err %d\r\n", ret);
	}
	// call the function to wait init finish
	FileSys_WaitFinishEx(p_stream->drive);
	return HD_OK;
}

static HD_RESULT exit_filesys_module(BSMUXER_STREAM *p_stream)
{
	if (FileSys_CloseEx(p_stream->drive, 1000) != FST_STA_OK) {
		printf("%s fail\r\n", __func__);
	}
	if (p_stream->filesys_pa && p_stream->filesys_va) {
		if (hd_common_mem_free((UINT32)p_stream->filesys_pa, (void *)p_stream->filesys_va) != HD_OK) {
			printf("%s mem free fail\r\n", __func__);
		}
		p_stream->filesys_pa = 0;
		p_stream->filesys_va = 0;
	}
	return HD_OK;
}

static HD_RESULT init_bsmuxer_module(void)
{
	HD_RESULT ret;
	if ((ret = hd_gfx_init()) != HD_OK)
		return ret;
	if ((ret = hd_bsmux_init()) != HD_OK)
		return ret;
	if ((ret = hd_fileout_init()) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT open_bsmuxer_module(BSMUXER_STREAM *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_bsmux_open(HD_BSMUX_IN(0, 0), HD_BSMUX_OUT(0, 0), &p_stream->bsmux_path)) != HD_OK)
		return ret;
	if ((ret = hd_fileout_open(HD_FILEOUT_IN(0, 0), HD_FILEOUT_OUT(0, 0), &p_stream->fileout_path)) != HD_OK)
		return ret;
	return HD_OK;
}
static HD_RESULT open_bsmuxer_module1(BSMUXER_STREAM *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_bsmux_open(HD_BSMUX_IN(0, 1), HD_BSMUX_OUT(0, 1), &p_stream->bsmux_path)) != HD_OK)
		return ret;
	if ((ret = hd_fileout_open(HD_FILEOUT_IN(0, 1), HD_FILEOUT_OUT(0, 1), &p_stream->fileout_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT start_bsmuxer_module(BSMUXER_STREAM *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_fileout_start(p_stream->fileout_path)) != HD_OK)
		return ret;
	if ((ret = hd_bsmux_start(p_stream->bsmux_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT stop_bsmuxer_module(BSMUXER_STREAM *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_bsmux_stop(p_stream->bsmux_path)) != HD_OK)
		return ret;
	if ((ret = hd_fileout_stop(p_stream->fileout_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT close_bsmuxer_module(BSMUXER_STREAM *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_bsmux_close(p_stream->bsmux_path)) != HD_OK)
		return ret;
	if ((ret = hd_fileout_close(p_stream->fileout_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT exit_bsmuxer_module(void)
{
	HD_RESULT ret;
	if ((ret = hd_gfx_uninit()) != HD_OK)
		return ret;
	if ((ret = hd_bsmux_uninit()) != HD_OK)
		return ret;
	if ((ret = hd_fileout_uninit()) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT push_video_to_bsmuxer_module(BSMUXER_STREAM *p_stream, HD_VIDEOENC_BS *p_user_bs)
{
	HD_RESULT ret;
	if ((ret = hd_bsmux_push_in_buf_video(p_stream->bsmux_path, p_user_bs, -1)) != HD_OK)
		return ret;
	return HD_OK;
}
#endif

static HD_RESULT set_out_cfg(HD_PATH_ID *p_video_out_ctrl, UINT32 out_type, HD_VIDEOOUT_HDMI_ID hdmi_id)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOOUT_MODE videoout_mode = {0};
	HD_PATH_ID video_out_ctrl = 0;

	ret = hd_videoout_open(0, HD_VIDEOOUT_0_CTRL, &video_out_ctrl); //open this for device control
	if (ret != HD_OK) {
		return ret;
	}
    hdmi_id = 16;
	printf("out_type=%d hdmi_id=%d\r\n", out_type,hdmi_id);

	#if 0
	videoout_mode.output_type = HD_COMMON_VIDEO_OUT_LCD;
	videoout_mode.input_dim = HD_VIDEOOUT_IN_AUTO;
	videoout_mode.output_mode.lcd = HD_VIDEOOUT_LCD_0;
	if (out_type != 1) {
		printf("520 only support LCD\r\n");
	}
	#else
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
		videoout_mode.output_mode.hdmi= hdmi_id;
	break;
	default:
		printf("not support out_type\r\n");
	break;
	}
	#endif
	ret = hd_videoout_set(video_out_ctrl, HD_VIDEOOUT_PARAM_MODE, &videoout_mode);

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

static HD_RESULT set_out_param(HD_PATH_ID video_out_path, HD_DIM *p_dim)
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
	printf("##video_out_param w:%d,h:%d %x %x\r\n", video_out_param.dim.w, video_out_param.dim.h, video_out_param.pxlfmt, video_out_param.dir);

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
	UINT32 	cap_loop;

	HD_DIM  proc0_max_dim;

	// (2)
	HD_VIDEOPROC_SYSCAPS proc0_syscaps;
	HD_PATH_ID proc0_ctrl;
	HD_PATH_ID proc0_path;

	HD_DIM  proc1_max_dim;
	HD_DIM  proc1_dim;

	// (3)
	HD_VIDEOPROC_SYSCAPS proc1_syscaps;
	HD_PATH_ID proc1_ctrl;
	HD_PATH_ID proc1_path;

	HD_DIM  enc_max_dim;
	HD_DIM  enc_dim;

	// (4)
	HD_VIDEOENC_SYSCAPS enc_syscaps;
	HD_PATH_ID enc_path;

	// (5) user pull
	pthread_t  enc_thread_id;
	pthread_t  relay_thread_id;
	UINT32     enc_exit;
	UINT32     flow_start;

	// (6) vout
	HD_VIDEOOUT_SYSCAPS out_syscaps;
	HD_PATH_ID out_ctrl;
	HD_PATH_ID out_path;
	HD_DIM  out_max_dim;
	HD_DIM  out_dim;

    HD_VIDEOOUT_HDMI_ID hdmi_id;
#if (BIND_BSMUXER == 1)
	BSMUXER_STREAM muxer_stream;
#endif

} VIDEO_RECORD;

static HD_RESULT init_module(void)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_init()) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_init()) != HD_OK)
		return ret;
    if ((ret = hd_videoenc_init()) != HD_OK)
		return ret;
   if ((ret = hd_videoout_init()) != HD_OK)
		return ret;
	return HD_OK;
}

#if 0 //for IMX291
#if 0
//IMX291 with lens TRC-2191B6(IMX317)
double distor_pt_lut_4R[DISTOR_CURVE_TABLE_NUM] = {
0, 18.0243122612896, 36.0510039231625, 54.0824440742380, 72.1209811792078, 90.1689327668716, 108.228575118174, 126.302132954239, 144.391769124409, 162.499574294278, 180.627556633728, 198.777631504967, 216.951611150564, 235.151194381486, 253.377956265131, 271.633337813367, 289.918635670569, 308.234991801651, 326.583383180107, 344.964611476044, 363.379292744217, 381.827847112070, 400.310488467766, 418.827214148228, 437.377794627174, 455.961763203149, 474.578405687568, 493.226750092746, 511.905556319938, 530.613305847373, 549.348191418292, 568.108106728981, 586.890636116810, 605.693044248269, 624.512265807003, 643.344895181846, 662.187176154862, 681.034991589378, 699.883853118019, 718.728890830749, 737.564842962899, 756.386045583214, 775.186422281878, 793.959473858556, 812.698268010433, 831.395429020241, 850.043127444303, 868.633069800568, 887.156488256642, 905.604130317831, 923.966248515174, 942.232590093474, 960.392386699345, 978.434344069240, 996.346631717488, 1014.11687262433, 1031.73213292397, 1049.17891159257, 1066.44313013633, 1083.51012227953, 1100.36462365249, 1116.99076147972, 1133.37204426785, 1149.49135149374, 1165.33092329247, 1180.87235014540, 1196.09656256821, 1210.98382079891, 1225.51370448592, 1239.66510237604, 1253.41620200255, 1266.74447937323, 1279.62668865836, 1292.03885187880, 1303.95624859400, 1315.35340559006, 1326.20408656774, 1336.48128183051, 1346.15719797258, 1355.20324756694, 1363.59003885341, 1371.28736542665, 1378.26419592421, 1384.48866371455, 1389.92805658512, 1394.54880643034, 1398.31647893967, 1401.19576328565, 1403.15046181192, 1404.14347972124, 1404.13681476357, 1403.09154692408, 1400.96782811119, 1397.72487184459, 1393.32094294330, 1387.71334721371, 1380.85842113759, 1372.71152156014, 1363.22701537802, 1352.35826922741, 1340.05763917202, 1326.27646039112, 1310.96503686760, 1294.07263107601, 1275.54745367057, 1255.33665317321, 1233.38630566162, 1209.64140445729, 1184.04584981353, 1156.54243860351, 1127.07285400830, 1095.57765520491, 1061.99626705432, 1026.26696978950, 988.326888703502, 948.111983837415, 905.557039668468, 860.595654798034, 813.160231639676, 763.181966107179, 710.590837302589, 655.315597204248, 597.283760354830, 536.421593549378, 472.654105523336, 405.905036640590, 336.096848581505, 263.150714030957, 186.986506366370, 107.522789345752, 24.6768067957333, -61.6355277003971, -151.499641114664, -245.002311286354, -342.231677233991, -443.277249467305, -548.229920299177, -657.181974157624, -770.227097897749, -887.460391113704, -1008.97837645067, -1134.87900991681, -1265.26169119522, -1400.22727395591, -1539.87807616777, -1684.31789041053, -1833.65199418671, -1987.98716023361, -2147.43166683525, -2312.09530813433, -2482.08940444423, -2657.52681256095, -2838.52193607506, -3025.19073568368, -3217.65073950246, -3416.02105337752, -3620.42237119739, -3830.97698520506, -4047.80879630984, -4271.04332439939, -4500.80771865168, -4737.23076784691, -4980.44291067954, -5230.57624607019, -5487.76454347764,
-5752.14325321078, -6023.84951674060, -6303.02217701210, -6589.80178875632, -6884.33062880224, -7186.75270638882, -7497.21377347687, -7815.86133506109, -8142.84465948200, -8478.31478873792, -8822.42454879690, -9175.32855990874, -9537.18324691690, -9908.14684957050, -10288.3794328362, -10678.0428972105, -11077.3009890310, -11486.3193107891, -11905.2653314417, -12334.3083967229, -12773.6197394564, -13223.3724898672, -13683.7416858936, -14154.9042834992, -14637.0391669847, -15130.3271593004, -15634.9510323573, -16151.0955173399, -16678.9473150177, -17218.6951060571, -17770.5295613338, -18334.6433522443, -18911.2311610182, -19500.4896910300, -20102.6176771109, -20717.8158958612, -21346.2871759619, -21988.2364084868, -22643.8705572143, -23313.3986689399, -23997.0318837874, -24694.9834455212, -25407.4687118587, -26134.7051647815, -26876.9124208478, -27634.3122415044, -28407.1285433985, -29195.5874086897, -29999.9170953619, -30820.3480475355, -31657.1129057792, -32510.4465174219, -33380.5859468646, -34267.7704858929, -35172.2416639883, -36094.2432586402, -37034.0213056587, -37991.8241094854, -38967.9022535061, -39962.5086103628, -40975.8983522651, -42008.3289613028, -43060.0602397574, -44131.3543204143, -45222.4756768747, -46333.6911338676, -47465.2698775617, -48617.4834658774, -49790.6058387987, -50984.9133286854, -52200.6846705847, -53438.2010125435, -54697.7459259201, -55979.6054156963, -57284.0679307894, -58611.4243743641, -59961.9681141444, -61335.9949927258, -62733.8033378868, -64155.6939729015, -65601.9702268510, -67072.9379449358, -68568.9054987873, -70090.1837967801, -71637.0862943440, -73209.9290042758, -74809.0305070513, -76434.7119611372, -78087.2971133032, -79767.1123089339, -81474.4865023408, -83209.7512670741
};

double undistor_pt_lut[UNDISTOR_CURVE_TABLE_NUM] = {
0, 18.0235184490379, 36.0446579687234, 54.0610524949744, 72.0703621509529, 90.0702864733339, 108.058577409238, 126.033052006235, 143.991604722740, 161.932219292138, 179.852980081122, 197.752082890669, 215.627845156945, 233.478715518831, 251.303282728719, 269.100283893509, 286.868612043274, 304.607323035602, 322.315641814227, 339.992968050928, 357.638881209903, 375.253145083726, 392.835711859566, 410.386725783639, 427.906526500802, 445.395652154856, 462.854842343639, 480.285041031363, 497.687399529069, 515.063279662786, 532.414257257965, 549.742126078586, 567.048902369978, 584.336830166406, 601.608387538148, 618.866293968621, 636.113519070625, 653.353292872621, 670.589117931878, 687.824783562240, 705.064382501235, 722.312330385560, 739.573388457299, 756.852910553027, 774.157363297735, 791.492079919997, 808.863517677882, 826.278647022756, 843.745003935869, 861.270750596296, 878.864745769246, 896.536626578538, 914.296903665936, 932.157072159837, 950.129741398399, 968.228787006013, 986.469529744493, 1004.86894660114, 1023.44592090088, 1042.22153992785, 1061.21945073354, 1080.46628765810, 1100.00016097508, 1119.86959509757, 1140.10292498988
};

double distor_center[2] = {988.9514, 593.8084};
INT32 focal_length = 1170;
#else
//IMX291 with wide lens
double distor_pt_lut_4R[DISTOR_CURVE_TABLE_NUM] = {
0, 18.3432698655579, 36.6737941476184, 54.9790566381974, 73.2468303277731, 91.4652165965859, 109.622680254618, 127.708080362007, 145.710696829883, 163.620252862362, 181.426933352949, 199.121399392661, 216.694799082802, 234.138774872766, 251.445467663139, 268.607517927257, 285.618064111210, 302.470738573694, 319.159661324140, 335.679431810880, 352.025119001543, 368.192249986200, 384.176797320462, 399.975165311496, 415.584175435071, 431.001051056796, 446.223401615823, 461.249206414838, 476.076798146228, 490.704846271031, 505.132340354810, 519.358573452839, 533.383125626130, 547.205847659732, 560.826845045446, 574.246462282573, 587.465267542523, 600.484037735969, 613.303744014764, 625.925537734959, 638.350736901895, 650.580813113549, 662.617379013934, 674.462176264447, 686.117064037544, 697.584008033952, 708.865070021874, 719.962397894102, 730.878216236815, 741.614817401886, 752.174553072880, 762.559826313478, 772.773084085860, 782.816810225540, 792.693518858341, 802.405748244495, 811.956055034367, 821.347008919909, 830.581187665709, 839.661172503362, 848.589543872862, 857.368877494772, 866.001740757070, 874.490689400792, 882.838264488836, 891.046989642679, 899.119368532059, 907.057882603162, 914.864989031223, 922.543118883995, 930.094675482951, 937.522032949657, 944.827534925212, 952.013493451188, 959.082188001034, 966.035864651394, 972.876735383324, 979.606977503895, 986.228733179140, 992.744109069827, 999.155176061973, 1005.46396908448, 1011.67248700677, 1017.78269260957, 1023.79651262265, 1029.71583782348, 1035.54252319128, 1041.27838811119, 1046.92521662387, 1052.48475771582, 1057.95872564627, 1063.34880030675, 1068.65662760965, 1073.88381990236, 1079.03195640394, 1084.10258366136, 1089.09721602260, 1094.01733612424, 1098.86439539116, 1103.63981454631, 1108.34498412866, 1112.98126501743, 1117.54998896116, 1122.05245911006, 1126.48995055022, 1130.86371083865, 1135.17496053788, 1139.42489374911, 1143.61467864316, 1147.74545798822, 1151.81834967378, 1155.83444723002, 1159.79482034216, 1163.70051535912, 1167.55255579618, 1171.35194283109, 1175.09965579345, 1178.79665264688, 1182.44387046384, 1186.04222589286, 1189.59261561798, 1193.09591681024, 1196.55298757116, 1199.96466736803, 1203.33177746103, 1206.65512132217, 1209.93548504580, 1213.17363775107, 1216.37033197596, 1219.52630406322, 1222.64227453808, 1225.71894847790, 1228.75701587369, 1231.75715198380, 1234.72001767962, 1237.64625978354, 1240.53651139928,
1243.39139223461, 1246.21150891660, 1248.99745529954, 1251.74981276571, 1254.46915051897, 1257.15602587141, 1259.81098452321, 1262.43456083569, 1265.02727809790, 1267.58964878669, 1270.12217482048, 1272.62534780682, 1275.09964928398, 1277.54555095652, 1279.96351492512, 1282.35399391078, 1284.71743147335, 1287.05426222486, 1289.36491203736, 1291.64979824572, 1293.90932984538, 1296.14390768513, 1298.35392465514, 1300.53976587029, 1302.70180884889, 1304.84042368703, 1306.95597322849, 1309.04881323043, 1311.11929252501, 1313.16775317691, 1315.19453063693, 1317.19995389176, 1319.18434561007, 1321.14802228487, 1323.09129437240, 1325.01446642754, 1326.91783723583, 1328.80169994227, 1330.66634217686, 1332.51204617706, 1334.33908890725, 1336.14774217517, 1337.93827274553, 1339.71094245084, 1341.46600829941, 1343.20372258087, 1344.92433296888, 1346.62808262155, 1348.31521027922, 1349.98595035995, 1351.64053305266, 1353.27918440801, 1354.90212642706, 1356.50957714776, 1358.10175072940, 1359.67885753501, 1361.24110421171, 1362.78869376922, 1364.32182565641, 1365.84069583606, 1367.34549685785, 1368.83641792952, 1370.31364498647, 1371.77736075957, 1373.22774484147, 1374.66497375131, 1376.08922099787, 1377.50065714128, 1378.89944985325, 1380.28576397593, 1381.65976157935, 1383.02160201755, 1384.37144198339, 1385.70943556210, 1387.03573428362, 1388.35048717365, 1389.65384080360, 1390.94593933938, 1392.22692458906, 1393.49693604945, 1394.75611095159, 1396.00458430525, 1397.24248894243, 1398.46995555979, 1399.68711276025, 1400.89408709352, 1402.09100309586, 1403.27798332883, 1404.45514841727, 1405.62261708633, 1406.78050619781, 1407.92893078560, 1409.06800409034, 1410.19783759337, 1411.31854104991, 1412.43022252151, 1413.53298840778, 1414.62694347743, 1415.71219089870, 1416.78883226903, 1417.85696764416, 1418.91669556658, 1419.96811309338, 1421.01131582347, 1422.04639792428, 1423.07345215785, 1424.09256990632, 1425.10384119702, 1426.10735472686, 1427.10319788633, 1428.09145678298, 1429.07221626431, 1430.04555994031, 1431.01157020548, 1431.97032826034, 1432.92191413259, 1433.86640669777, 1434.80388369948, 1435.73442176928, 1436.65809644605
};

double undistor_pt_lut[UNDISTOR_CURVE_TABLE_NUM] = {
0, 18.3475454684106, 36.7078703748377, 55.0936270893503, 73.5173638863560, 91.9915614016050, 110.528667369977, 129.141129607893, 147.841427227013, 166.642100084852, 185.555776493103, 204.595199216181, 223.773249800938, 243.102971284298, 262.597589328860, 282.270531838103, 302.135447102787, 322.206220529183, 342.496989998010, 363.022159900866, 383.796413898728, 404.849289729118, 426.203421902367, 447.858099974934, 469.829577975147, 492.134436625029, 514.804842141041, 537.892844920089, 561.373716689882, 585.265734057416, 609.649555130352, 634.517301902601, 659.862513409642, 685.812694939792, 712.299818726856, 739.424113006898, 767.180293888268, 795.649055394862, 824.809694657323, 854.808999247040, 885.599721033242, 917.247973491668, 949.890888694323, 983.517485847009, 1018.20272833979, 1054.02490908075, 1091.06546176547, 1129.40877676307, 1169.14202237395, 1210.35497212022, 1253.23478339708, 1297.81539855336, 1344.26946524695, 1392.72185812093, 1443.39627302211, 1496.41872758681, 1552.00490567749, 1610.37694048904, 1671.79886774233, 1736.57776617959, 1804.99535165800, 1877.42405491934, 1954.28350799949, 2036.00295251565, 2123.20332249487
};

double distor_center[2] = {904.9962, 487.1206};
INT32 focal_length = 946;
#endif

#else //for IMX415

#if 0
//IMX415 with lens TRC-2191B6
double distor_pt_lut_4R[DISTOR_CURVE_TABLE_NUM] = {
0, 36.2630152174099, 72.5308647175537, 108.808347470982, 145.100191992390, 181.411021533959, 217.745319784240, 254.107397241061, 290.501358426992, 326.931070115872, 363.400130738894, 399.911841138776, 436.469176840521, 473.074762007268, 509.730845249757, 546.439277457908, 583.201491823029, 620.018486219162, 656.890808112069, 693.818542164387, 730.801300705436, 767.838217234212, 804.927943124063, 842.068647697562, 879.258021840080, 916.493285320586, 953.771197988159, 991.088075012741, 1028.43980633864, 1065.82188051927, 1103.22941310169, 1140.65717972937, 1178.09965413176, 1215.55105116920, 1253.00537510154, 1290.45647324916, 1327.89809521475, 1365.32395783448, 1402.72781602695, 1440.10353970854, 1477.44519694364, 1514.74714349813, 1552.00411896493, 1589.21134962979, 1626.36465824601, 1663.46058088667, 1700.49649104262, 1737.47073113505, 1774.38275161091, 1811.23325778978, 1848.02436463078, 1884.75975958786, 1921.44487372211, 1958.08706123956, 1994.69578762298, 2031.28282652616, 2067.86246559925, 2104.45172141362, 2141.07056365471, 2177.74214875150, 2214.49306311100, 2251.35357612630, 2288.35790312670, 2325.54447843848, 2362.95623872467, 2400.64091677250, 2438.65134589697, 2477.04577512897, 2515.88819535666, 2555.24867658836, 2595.20371650569, 2635.83660047534, 2677.23777318803, 2719.50522209313, 2762.74487279752, 2807.07099659707, 2852.60663030947, 2899.48400857663, 2947.84500880543, 2997.84160891515, 3049.63635806021, 3103.40286049660, 3159.32627276063, 3217.60381432848, 3278.44529192500, 3342.07363765031, 3408.72546109278, 3478.65161559673, 3552.11777885358, 3629.40504798472, 3710.81054928481, 3796.64806279394, 3887.24866186710, 3982.96136790962, 4084.15382044692, 4191.21296269721, 4304.54574281557, 4424.57983097798, 4551.76435247374, 4686.57063697489, 4829.49298415099, 4981.04944579797, 5141.78262464934, 5312.26049003845, 5493.07721058023, 5684.85400404089, 5888.24000456417, 6103.91314742258, 6332.58107146219, 6574.98203940944, 6831.88587620849, 7104.09492555768, 7392.44502481348, 7697.80649843055, 8021.08517010642, 8363.22339379922, 8725.20110378702, 9108.03688393732, 9512.78905635513, 9940.55678957819, 10392.4812264879, 10869.7466321042, 11373.5815614334, 11905.2600475371, 12466.1028099906, 13057.4784838996, 13680.8048696439, 14337.5502035158, 15029.2344494226, 15757.4306118213, 16523.7660700538, 17329.9239342515, 18177.6444229779, 19068.7262627771, 20005.0281097972, 20988.4699936571, 22021.0347837250, 23104.7696779770, 24241.7877146054, 25434.2693065429, 26684.4637990741, 27994.6910507001, 29367.3430374270, 30804.8854806450, 32309.8594987679, 33884.8832828008, 35532.6537960043, 37255.9484978251, 39057.6270922592, 40940.6333008190, 42907.9966602698, 44962.8343453072, 47108.3530163413, 49347.8506925581, 51684.7186504256, 54122.4433478133, 56664.6083738940, 59314.8964249955, 62077.0913065726, 64955.0799614645, 67952.8545246108, 71074.5144043899, 74324.2683907515, 77706.4367903106, 81225.4535885713, 84885.8686394494, 88692.3498822621, 92649.6855863531, 96762.7866235217, 101036.688768426, 105476.555027123, 110087.677993926, 114875.482236731, 119845.526710997, 125003.507202537, 130355.258799297, 135906.758392282, 141664.127205806, 147633.633357230, 153821.694446360, 160234.880174670, 166879.914994516, 173763.680788517, 180893.219579270, 188275.736269562, 195918.601413249, 203829.354016976, 212015.704372900, 220485.536922592, 229246.913152275, 238308.074519580, 247677.445411974, 257363.636137047, 267375.445944805, 277721.866082156, 288412.082879746, 299455.480871318, 310861.645945766, 322640.368532038, 334801.646817082, 347355.689996972, 360312.921561415, 373683.982611776, 387479.735212820, 401711.265778311, 416389.888490657, 431527.148754766, 447134.826686267, 463224.940634287, 479809.750738933, 496901.762523661, 514513.730522688, 532658.661943634, 551349.820365537, 570600.729472437, 590425.176822672, 610837.217654076, 631851.178725236, 653481.662192971, 675743.549526220, 698652.005456492, 722222.481965048, 746470.722306993, 771412.765072431, 797064.948284875, 823443.913537050, 850566.610164284,
    878450.299455644, 907112.558902983, 936571.286488071, 966844.705007984, 997951.366438905, 1029910.15633851, 1062740.29828715, 1096461.35836786, 1131093.24968561, 1166656.23692570, 1203170.94095158, 1240658.34344242, 1279139.79157024, 1318637.00271713, 1359172.06923253, 1400767.46323073, 1443446.04142893, 1487231.05002579, 1532146.12962084, 1578215.32017477, 1625463.06601092, 1673914.22085799, 1723594.05293424, 1774528.25007324, 1826742.92489156, 1880264.61999823, 1935120.31324648, 1991337.42302770, 2048943.81360789, 2107967.80050674
};

double undistor_pt_lut[UNDISTOR_CURVE_TABLE_NUM] = {
0, 36.2614014512333, 72.5179695867695, 108.764911627218, 144.997516550263, 181.211195595377, 217.401521519119, 253.564266199253, 289.695436217475, 325.791306088733, 361.848448848763, 397.863763759062, 433.834500938844, 469.758282784938, 505.633122091706, 541.457436832423, 577.230061609933, 612.950255826514, 648.617708659805, 684.232540962487, 719.795304227567, 755.306976777987, 790.768957348754, 826.183056231492, 861.551484145403, 896.876838985073, 932.162090574581, 967.410563529217, 1002.62591829098, 1037.81213036234, 1072.97346771455, 1108.11446629297, 1143.23990348176, 1178.35476932547, 1213.46423523487, 1248.57361983022, 1283.68835149694, 1318.81392714820, 1353.95586660689, 1389.11966193805, 1424.31072098472, 1459.53430428810, 1494.79545451266, 1530.09891745253, 1565.44905367648, 1600.84973988275, 1636.30425909448, 1671.81517894484, 1707.38421749405, 1743.01209630722, 1778.69838092153, 1814.44130936551, 1850.23761008173, 1886.08231146547, 1921.96854627730, 1957.88751649382, 1993.82761710990, 2029.77502915742, 2065.71335207466, 2101.62339559567, 2137.48307632918, 2173.26736686564, 2208.94831160209, 2244.49512392488, 2279.87437898897
};

double distor_center[2] = {1964.6, 923.6};
INT32 focal_length = 2244;
#else
double distor_pt_lut_4R[DISTOR_CURVE_TABLE_NUM] = {
	0.000000, 35.764734, 71.533684, 107.311037, 143.100922, 178.907382, 214.734347, 250.585601,
	286.464763, 322.375255, 358.320275, 394.302780, 430.325454, 466.390693, 502.500576, 538.656856,
	574.860931, 611.113837, 647.416227, 683.768363, 720.170103, 756.620894, 793.119767, 829.665331,
	866.255772, 902.888861, 939.561948, 976.271978, 1013.015500, 1049.788676, 1086.587303, 1123.406832,
	1160.242389, 1197.088806, 1233.940650, 1270.792261, 1307.637787, 1344.471228, 1381.286486, 1418.077416,
	1454.837880, 1491.561811, 1528.243274, 1564.876543, 1601.456171, 1637.977072, 1674.434609, 1710.824680,
	1747.143819, 1783.389299, 1819.559233, 1855.652696, 1891.669838, 1927.612012, 1963.481907, 1999.283685,
	2035.023123, 2070.707770, 2106.347098, 2141.952670, 2177.538312, 2213.120288, 2248.717492, 2284.351631,
	2320.047435, 2355.832860, 2391.739301, 2427.801822, 2464.059380, 2500.555067, 2537.336360, 2574.455369,
	2611.969110, 2649.939767, 2688.434983, 2727.528140, 2767.298666, 2807.832335, 2849.221589, 2891.565859,
	2934.971903, 2979.554152, 3025.435057, 3072.745461, 3121.624966, 3172.222321, 3224.695813, 3279.213670,
	3335.954477, 3395.107601, 3456.873622, 3521.464781, 3589.105437, 3660.032536, 3734.496081, 3812.759634,
	3895.100804, 3981.811769, 4073.199794, 4169.587768, 4271.314751, 4378.736533, 4492.226204, 4612.174737,
	4738.991586, 4873.105287, 5014.964088, 5165.036570, 5323.812304, 5491.802498, 5669.540679, 5857.583368,
	6056.510783, 6266.927549, 6489.463419, 6724.774017, 6973.541585, 7236.475752, 7514.314308, 7807.824005,
	8117.801357, 8445.073465, 8790.498856, 9154.968327, 9539.405818, 9944.769285, 10372.051602, 10822.281464,
	11296.524319, 11795.883304, 12321.500202, 12874.556416, 13456.273951, 14067.916421, 14710.790066, 15386.244788,
	16095.675199, 16840.521691, 17622.271518, 18442.459896, 19302.671119, 20204.539693, 21149.751486, 22140.044894,
	23177.212026, 24263.099905, 25399.611686, 26588.707894, 27832.407674, 29132.790068, 30491.995297, 31912.226073,
	33395.748924, 34944.895535, 36562.064110, 38249.720755, 40010.400871, 41846.710576, 43761.328139, 45757.005437,
	47836.569424, 50002.923629, 52259.049666, 54608.008765, 57052.943327, 59597.078490, 62243.723721, 64996.274429,
	67858.213594, 70833.113413, 73924.636981, 77136.539971, 80472.672353, 83936.980122, 87533.507057, 91266.396485,
	95139.893086, 99158.344700, 103326.204173, 107648.031205, 112128.494240, 116772.372361, 121584.557212, 126570.054950,
	131733.988205, 137081.598072, 142618.246119, 148349.416426, 154280.717634, 160417.885028, 166766.782638, 173333.405358,
	180123.881097, 187144.472949, 194401.581381, 201901.746454, 209651.650060, 217658.118187, 225928.123201, 234468.786162,
	243287.379154, 252391.327645, 261788.212868, 271485.774230, 281491.911740, 291814.688466, 302462.333014, 313443.242034,
	324765.982747, 336439.295503, 348472.096356, 360873.479674, 373652.720764, 386819.278532, 400382.798160, 414353.113814,
	428740.251380, 443554.431217, 458806.070946, 474505.788257, 490664.403748, 507292.943791, 524402.643415, 542004.949231,
	560111.522368, 578734.241447, 597885.205579, 617576.737387, 637821.386060, 658631.930430, 680021.382080, 702002.988477,
	724590.236136, 747796.853808, 771636.815696, 796124.344708, 821273.915723, 847100.258898, 873618.362998, 900843.478754,
	928791.122255, 957477.078362, 986917.404156, 1017128.432409, 1048126.775096, 1079929.326920, 1112553.268884, 1146016.071874,
	1180335.500290, 1215529.615695, 1251616.780494, 1288615.661650, 1326545.234427, 1365424.786159, 1405273.920056, 1446112.559036,
	1487960.949589
};
double undistor_pt_lut[UNDISTOR_CURVE_TABLE_NUM] = {
	0.000000, 35.763327, 71.522439, 107.273156, 143.011363, 178.733048, 214.434330, 250.111490,
	285.761001, 321.379556, 356.964094, 392.511819, 428.020228, 463.487123, 498.910632, 534.289220,
	569.621704, 604.907258, 640.145421, 675.336104, 710.479588, 745.576527, 780.627948, 815.635240,
	850.600157, 885.524806, 920.411640, 955.263447, 990.083339, 1024.874740, 1059.641367, 1094.387223,
	1129.116570, 1163.833916, 1198.543995, 1233.251739, 1267.962262, 1302.680823, 1337.412807, 1372.163684,
	1406.938978, 1441.744225, 1476.584928, 1511.466510, 1546.394254, 1581.373244, 1616.408295, 1651.503877,
	1686.664021, 1721.892232, 1757.191374, 1792.563556, 1828.009999, 1863.530896, 1899.125253, 1934.790728,
	1970.523454, 2006.317854, 2042.166454, 2078.059694, 2113.985743, 2149.930326, 2185.876573, 2221.804897,
	2257.692909
};
double distor_center[2] = {1990.237223, 1130.490869};
INT32 focal_length = 2243;
double max_out_radius = 2289;
#endif

#endif

#define GYRO_FXPT_PREC (1 << 16)
static HD_RESULT init_eis(void)
{
	VENDOR_EIS_OPEN_CFG test_open_obj = {0};
	UINT32 i;
	HD_RESULT ret;

	if (sensor_sel == SEN_SEL_IMX290 || sensor_sel == SEN_SEL_OS02K10) {
		test_open_obj.cam_intrins.distor_center[0] = distor_center[0]; //pixel
		test_open_obj.cam_intrins.distor_center[1] = distor_center[1]; //pixel
		for (i = 0; i < DISTOR_CURVE_TABLE_NUM; i++) {
			test_open_obj.cam_intrins.distor_curve[i] = distor_pt_lut_4R[i];
		}
		for (i = 0; i < UNDISTOR_CURVE_TABLE_NUM; i++) {
			test_open_obj.cam_intrins.undistor_curve[i] = undistor_pt_lut[i];
		}
		test_open_obj.cam_intrins.calib_img_size.w = VDO_SIZE_W; //pixel
		test_open_obj.cam_intrins.calib_img_size.h = VDO_SIZE_H; //pixel
		test_open_obj.cam_intrins.max_out_radius = max_out_radius; // pixel
		test_open_obj.cam_intrins.focal_length = focal_length; //pixel
	} else if (sensor_sel == SEN_SEL_IMX415) {
		test_open_obj.cam_intrins.distor_center[0] = distor_center[0]; //pixel
		test_open_obj.cam_intrins.distor_center[1] = distor_center[1]; //pixel
		for (i = 0; i < DISTOR_CURVE_TABLE_NUM; i++) {
			test_open_obj.cam_intrins.distor_curve[i] = distor_pt_lut_4R[i];
		}
		for (i = 0; i < UNDISTOR_CURVE_TABLE_NUM; i++) {
			test_open_obj.cam_intrins.undistor_curve[i] = undistor_pt_lut[i];
		}
		test_open_obj.cam_intrins.calib_img_size.w = VDO_SIZE_W; //pixel
		test_open_obj.cam_intrins.calib_img_size.h = VDO_SIZE_H; //pixel
		test_open_obj.cam_intrins.max_out_radius = max_out_radius; // pixel
		test_open_obj.cam_intrins.focal_length = focal_length; //pixel
	} else if (sensor_sel == SEN_SEL_OS05A10) {
		test_open_obj.cam_intrins.distor_center[0] = distor_center[0]; //pixel
		test_open_obj.cam_intrins.distor_center[1] = distor_center[1]; //pixel
		for (i = 0; i < DISTOR_CURVE_TABLE_NUM; i++) {
			test_open_obj.cam_intrins.distor_curve[i] = distor_pt_lut_4R[i];
		}
		for (i = 0; i < UNDISTOR_CURVE_TABLE_NUM; i++) {
			test_open_obj.cam_intrins.undistor_curve[i] = undistor_pt_lut[i];
		}
		test_open_obj.cam_intrins.calib_img_size.w = VDO_SIZE_W; //pixel
		test_open_obj.cam_intrins.calib_img_size.h = VDO_SIZE_H; //pixel
		test_open_obj.cam_intrins.max_out_radius = max_out_radius; // pixel
		test_open_obj.cam_intrins.focal_length = focal_length; //pixel
	} else {
		test_open_obj.cam_intrins.distor_center[0] = distor_center[0]; //pixel
		test_open_obj.cam_intrins.distor_center[1] = distor_center[1]; //pixel
		for (i = 0; i < DISTOR_CURVE_TABLE_NUM; i++) {
			test_open_obj.cam_intrins.distor_curve[i] = distor_pt_lut_4R[i];
		}
		for (i = 0; i < UNDISTOR_CURVE_TABLE_NUM; i++) {
			test_open_obj.cam_intrins.undistor_curve[i] = undistor_pt_lut[i];
		}
		test_open_obj.cam_intrins.calib_img_size.w = VDO_SIZE_W; //pixel
		test_open_obj.cam_intrins.calib_img_size.h = VDO_SIZE_H; //pixel
		test_open_obj.cam_intrins.max_out_radius = max_out_radius; // pixel
		test_open_obj.cam_intrins.focal_length = focal_length; //pixel
	}

	test_open_obj.stable_profile = VENDOR_EIS_SPORT_CAM;

	test_open_obj.imu_sync_shift_exposure_time_threshold = 99999; //us
	test_open_obj.imu_sync_shift_exposure_time_precent = 0;

	test_open_obj.imu_type = VENDOR_EIS_GYROSCOPE_ACCELEROMETER; //

#if 1
	//gyro board pin up, chip facing forward
	test_open_obj.gyro.axes_mapping[0].axis = VENDOR_EIS_CAM_X; //Ex. axis = CAM_Z, sign = -1;
	test_open_obj.gyro.axes_mapping[0].sign = 1;
	test_open_obj.gyro.axes_mapping[1].axis = VENDOR_EIS_CAM_Y;
	test_open_obj.gyro.axes_mapping[1].sign = 1;
	test_open_obj.gyro.axes_mapping[2].axis = VENDOR_EIS_CAM_Z;
	test_open_obj.gyro.axes_mapping[2].sign = 1;
#else
	//thermal
	test_open_obj.gyro.axes_mapping[0].axis = VENDOR_EIS_CAM_Y; // Ex. axis = CAM_Z, sign = -1;
	test_open_obj.gyro.axes_mapping[0].sign = -1;
	test_open_obj.gyro.axes_mapping[1].axis = VENDOR_EIS_CAM_Z;
	test_open_obj.gyro.axes_mapping[1].sign = -1;
	test_open_obj.gyro.axes_mapping[2].axis = VENDOR_EIS_CAM_X;
	test_open_obj.gyro.axes_mapping[2].sign = 1;
#endif
	test_open_obj.gyro.sampling_rate = g_fps*GYRO_DATA_NUM; //Ex. 1000 Hz
	test_open_obj.gyro.unit_conv = 250.0/32768.0/180.0*VPRC_PI;  //Ex. gyro_value*unit_conv = rad/s
	//thermal
	//test_open_obj.gyro.sampling_rate = 833;          // Ex. 1000 Hz
	//test_open_obj.gyro.unit_conv = 8.75 / 1000.0 / 180.0 * VPRC_PI; // Ex. gyro_value*unit_conv = rad/s

	//accelerometer
	if (test_open_obj.imu_type == VENDOR_EIS_GYROSCOPE_ACCELEROMETER) {
		test_open_obj.accel.axes_mapping[0].axis = VENDOR_EIS_CAM_X; //Ex. axis = CAM_Z, sign = -1;
		test_open_obj.accel.axes_mapping[0].sign = 1;
		test_open_obj.accel.axes_mapping[1].axis = VENDOR_EIS_CAM_Y;
		test_open_obj.accel.axes_mapping[1].sign = 1;
		test_open_obj.accel.axes_mapping[2].axis = VENDOR_EIS_CAM_Z;
		test_open_obj.accel.axes_mapping[2].sign = 1;

		test_open_obj.accel.sampling_rate = g_fps*GYRO_DATA_NUM; //Ex. 1000 Hz
		test_open_obj.accel.unit_conv = 2.0/32768.0;//*9.8;  //Ex. accel_value*unit_conv = m/s^2, 2 is adjustable
		test_open_obj.accel.accel_gain = 0.0005;
	}

	if (vendor_isp_init() == HD_ERR_NG) {
		return -1;
	}

	ret = vendor_eis_open(&test_open_obj);

	{
		VENDOR_EIS_PATH_INFO test_path_obj = {0};
		BOOL lib_set_status = FALSE;
		INT32 frame_rate;
		INT32 gyro_sample_per_frame;
		BOOL switch_val;
		VENDOR_EIS_IMG_ROTATE_SEL rot_sel;

		switch_val = FALSE;
		lib_set_status = vendor_eis_set(VENDOR_EIS_PARAM_FRAME_CNT_RESET, &switch_val);
		printf("Set EIS_RSC_PARAM_FRAME_CNT_RESET: %s\r\n", lib_set_status ? "Success" : "Failed");

		frame_rate = 30;
		lib_set_status = vendor_eis_set(VENDOR_EIS_PARAM_FRAME_RATE, &frame_rate);
		printf("Set EIS_RSC_PARAM_FRAME_RATE: %s\r\n", lib_set_status ? "Success" : "Failed");

		gyro_sample_per_frame = GYRO_DATA_NUM;
		lib_set_status = vendor_eis_set(VENDOR_EIS_PARAM_IMU_SAMPLE_NUM_PER_FRAME, &gyro_sample_per_frame);
		printf("Set EIS_RSC_PARAM_IMU_SAMPLE_NUM_PER_FRAME: %s\r\n", lib_set_status ? "Success" : "Failed");

		rot_sel = VENDOR_EIS_ROT_0;
		lib_set_status = vendor_eis_set(VENDOR_EIS_PARAM_ROTATE_TYPE, &rot_sel);
		printf("Set EIS_RSC_PARAM_ROTATE_TYPE: %s\r\n", lib_set_status ? "Success" : "Failed");

		switch_val = FALSE;
		lib_set_status = vendor_eis_set(VENDOR_EIS_PARAM_UNDISTORT_OUTPUT, &switch_val);
		printf("Set VENDOR_EIS_PARAM_UNDISTORT_OUTPUT: %s\r\n", lib_set_status ? "Success" : "Failed");

		test_path_obj.path_id = 0;
		test_path_obj.frame_latency = 1;
		test_path_obj.frame_size.w = VDO_SIZE_W;
		test_path_obj.frame_size.h = VDO_SIZE_H;
		test_path_obj.lut2d_size_sel = lut2d_path0.lut2d.lut_sz;
        test_path_obj.eis_crop_size.w = (INT32)((double)VDO_SIZE_W*0.80);
        test_path_obj.eis_crop_size.h = (INT32)((double)test_path_obj.eis_crop_size.w*9.0/16.0);
		lib_set_status = vendor_eis_set(VENDOR_EIS_PARAM_PATH_INFO, &test_path_obj);
		printf("Set EIS_RSC_PARAM_PATH_INFO: %s\r\n", lib_set_status ? "Success" : "Failed");

#if 0
		test_path_obj.path_id = 1;
		test_path_obj.frame_latency = 1;
		test_path_obj.frame_size.w = VDO_SIZE_W;
		test_path_obj.frame_size.h = VDO_SIZE_H;
		test_path_obj.lut2d_size_sel = VENDOR_EIS_LUT_65x65;
		lib_set_status = vendor_eis_set(VENDOR_EIS_PARAM_PATH_INFO, &test_path_obj);
		printf("Set EIS_RSC_PARAM_PATH_INFO: %s\r\n", lib_set_status ? "Success" : "Failed");
#endif
	}

	return ret;
}
static HD_RESULT init_vpe(void)
{
	HD_RESULT ret;
	VPET_DCE_CTL_PARAM dce_ctl = {0};

	if ((ret = vendor_vpe_init()) != HD_OK) {
		printf("vendor_vpe_init failed!(%d)\r\n", ret);
		return ret;
	}

	dce_ctl.id = ISP_EFFECT_ID;
	dce_ctl.dce_ctl.enable = 1;
	dce_ctl.dce_ctl.dce_mode = 1; // 0:GDC, 1:2DLUT
	//dce_ctl.dce_ctl.lens_radius = 0;
	vendor_vpe_set_cmd(VPET_ITEM_DCE_CTL_PARAM, &dce_ctl);

	lut2d_path0.id = ISP_EFFECT_ID;
	gen_default_2dlut(lut2d_path0.lut2d.lut, VDO_SIZE_W, VDO_SIZE_H, 9);
	vendor_vpe_set_cmd(VPET_ITEM_2DLUT_PARAM, &lut2d_path0);
	return ret;
}
static HD_RESULT open_module(VIDEO_RECORD *p_stream, HD_DIM* p_proc0_max_dim, HD_DIM* p_proc1_max_dim)
{
	HD_RESULT ret;
	// set videocap config
	ret = set_cap_cfg(&p_stream->cap_ctrl);
	if (ret != HD_OK) {
		printf("set cap-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	// set 1st videoproc config
	ret = set_proc_cfg(HD_VIDEOPROC_0_CTRL, &p_stream->proc0_ctrl, HD_VIDEOPROC_PIPE_RAWALL, p_proc0_max_dim);
	if (ret != HD_OK) {
		printf("set proc-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	// set 2nd videoproc config
	ret = set_proc_cfg(HD_VIDEOPROC_1_CTRL, &p_stream->proc1_ctrl, HD_VIDEOPROC_PIPE_VPE, p_proc1_max_dim);
	if (ret != HD_OK) {
		printf("set proc-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}

	if ((ret = hd_videocap_open(HD_VIDEOCAP_IN(SEN1_VCAP_ID, 0), HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), &p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &p_stream->proc0_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_1_IN_0, HD_VIDEOPROC_1_OUT_0, &p_stream->proc1_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_VIDEOENC_0_OUT_0, &p_stream->enc_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT open_module_2(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_1,  &p_stream->proc1_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_1, HD_VIDEOENC_0_OUT_1,  &p_stream->enc_path)) != HD_OK)
		return ret;

	return HD_OK;
}
static HD_RESULT open_module_3(VIDEO_RECORD *p_stream,UINT32 out_type)
{
	HD_RESULT ret;
	// set videoout config
	ret = set_out_cfg(&p_stream->out_ctrl, out_type, p_stream->hdmi_id);
	if (ret != HD_OK) {
		printf("set out-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
#if DISPLAY
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_1_IN_0, HD_VIDEOPROC_1_OUT_2,  &p_stream->proc1_path)) != HD_OK)
		return ret;
#endif

	if ((ret = hd_videoout_open(HD_VIDEOOUT_0_IN_0, HD_VIDEOOUT_0_OUT_0,  &p_stream->out_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT close_module(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_close(p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_close(p_stream->proc0_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_close(p_stream->proc1_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_close(p_stream->enc_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT close_module_2(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_videoproc_close(p_stream->proc1_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_close(p_stream->enc_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT close_module_3(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;

#if DISPLAY
	if ((ret = hd_videoproc_close(p_stream->proc1_path)) != HD_OK)
		return ret;
#endif
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
	if ((ret = hd_videoenc_uninit()) != HD_OK)
		return ret;
	if ((ret = hd_videoout_uninit()) != HD_OK)
		return ret;
	return HD_OK;
}
////for save gyro data
//#define TMP_SIZE 2048
//static char write_buf[TMP_SIZE] = {0};
// only used when (g_capbind = 0xff)  //no-bind mode

typedef struct {
	UINT32 *data_num;				//output gyro data number
	UINT64 *t_diff_crop;			//time for crop_start to crop_end
	UINT64 *t_diff_crp_end_to_vd;	//time for crop_end to next vd
	INT32 *agyro_x;
	INT32 *agyro_y;
	INT32 *agyro_z;
	INT32 *ags_x;
	INT32 *ags_y;
	INT32 *ags_z;
} USR_SIE_EXT_GYRO_DATA;


#if 0
INT32 vcap_get_gyro_data(UINT32 rsv1)
{
    INT32 rt = E_OK;
    USR_SIE_EXT_GYRO_DATA gyro_data;
    UINT32 buf_ofs = 0;

	gyro_data.data_num = (UINT32 *)(rsv1);

	buf_ofs += sizeof(*gyro_data.data_num);
	gyro_data.t_diff_crop = (UINT64 *)(rsv1 + buf_ofs);

	buf_ofs += sizeof(*gyro_data.t_diff_crop);
	gyro_data.t_diff_crp_end_to_vd = (UINT64 *)(rsv1 + buf_ofs);

	buf_ofs += sizeof(*gyro_data.t_diff_crp_end_to_vd);
	gyro_data.agyro_x =  (INT32 *)(rsv1 + buf_ofs);
	buf_ofs += (*gyro_data.data_num) * sizeof(INT32);
	gyro_data.agyro_y =  (INT32 *)(rsv1 + buf_ofs);
	buf_ofs += (*gyro_data.data_num) * sizeof(INT32);
	gyro_data.agyro_z =  (INT32 *)(rsv1 + buf_ofs);
	buf_ofs += (*gyro_data.data_num) * sizeof(INT32);
	gyro_data.ags_x = (INT32 *)(rsv1 + buf_ofs);
	buf_ofs += (*gyro_data.data_num) * sizeof(INT32);
	gyro_data.ags_y = (INT32 *)(rsv1 + buf_ofs);
	buf_ofs += (*gyro_data.data_num) * sizeof(INT32);
	gyro_data.ags_z = (INT32 *)(rsv1 + buf_ofs);

    return rt;
}
#endif
UINT64 vcap_ts = 0;
static void *capture_thread(void *arg)
{
	VIDEO_RECORD* p_stream0 = (VIDEO_RECORD *)arg;
	HD_RESULT ret = HD_OK;
	HD_VIDEO_FRAME video_frame = {0};
	char file_path_main[32] = {0};
	FILE *f_out_main;
	UINTPTR phy_addr_main, vir_addr_main;
    UINT64 last_frame =0;
    char *ptr = 0;
	UINTPTR pa   = 0;
	void  *va   = NULL;
    UINT32 keep_pos =0;
    static UINT32 checked =0;
	HD_COMMON_MEM_VB_BLK      blk;
	ISPT_SENSOR_EXPT sensor_expt = {0};

	 #define PHY2VIRT_RSV1(pa) (vir_addr_main + ((pa) - phy_addr_main))

    snprintf(file_path_main, 32, "/mnt/sd/gyro_dump.txt");
	if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
		HD_VIDEOENC_ERR("open file (%s) fail....\r\n", file_path_main);
	} else {
		printf("\r\ndump gyro to file (%s) ....\r\n", file_path_main);
	}

	p_stream0->cap_exit = 0;
	p_stream0->cap_loop = 0;
	p_stream0->cap_count = 0;

	blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_DEFINIED_POOL, GYRO_FILE_MAX, DDR_ID0);
	if (HD_COMMON_MEM_VB_INVALID_BLK == blk) {
		printf("hd_common_mem_get_block fail\r\n");
	}
	pa = hd_common_mem_blk2pa(blk);
	if (pa == 0) {
		printf("not get buffer, pa=%08x\r\n", (int)pa);
	}
	va = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, GYRO_FILE_MAX);

	//------ wait flow_start ------
	while (p_stream0->cap_enter == 0) usleep(100);

	//--------- pull data test ---------
	while (p_stream0->cap_exit == 0) {

		//printf("cap_pull ....\r\n");
		ret = hd_videocap_pull_out_buf(p_stream0->cap_path, &video_frame, -1); // -1 = blocking mode
		if (ret != HD_OK) {
			if (ret != HD_ERR_UNDERRUN)
			printf("cap_pull error=%d !!\r\n\r\n", ret);
    			goto skip;
		}

        if(checked<1) {
            printf("1cap ts=%llu count = %llu \r\n",video_frame.timestamp,video_frame.count);
            vcap_ts = video_frame.timestamp;
            checked++;
        }
		//printf("cap frame.count = %llu last_frame %llu\r\n", video_frame.count,last_frame);
		if((last_frame)&&(video_frame.count-last_frame!=1)){
            printf("###################ERR: drop frame %llu %llu ret %d ts:%llu ############\r\n",video_frame.count,last_frame,ret,video_frame.timestamp);

		}
		last_frame = video_frame.count;
///////////////////////

			phy_addr_main = hd_common_mem_blk2pa(video_frame.blk); // Get physical addr
			if (phy_addr_main == 0) {
				printf("hd_common_mem_blk2pa error !!\r\n\r\n");
				goto skip;
			}
			vir_addr_main = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_main, g_vcap_blk_size);
			if (vir_addr_main == 0) {
				printf("memory map error !!\r\n\r\n");
				goto skip;
			}
            vendor_isp_get_common(ISPT_ITEM_SENSOR_EXPT, &sensor_expt);
            //printf("exposure time (us) = %d, %d \n", sensor_expt.time[0], sensor_expt.time[1]);
            if (!sensor_expt.time[0]) {
                printf("no exposure time (us) = %d, %d \n");
            }

			{
				UINTPTR rsv1 = PHY2VIRT_RSV1(video_frame.phy_addr[HD_VIDEO_MAX_PLANE-1]);

				if (rsv1) {
                    USR_SIE_EXT_GYRO_DATA gyro_data;
                    UINT32 buf_ofs = 0;
					UINT32 data_num;
					INT32 *agyro_x;
					INT32 *agyro_y;
					INT32 *agyro_z;
					INT32 *ags_x;
					INT32 *ags_y;
					INT32 *ags_z;
                	UINT64 *t_diff_crop;
                	UINT64 *t_diff_crp_end_to_vd;

                	gyro_data.data_num = (UINT32 *)(rsv1);
                    data_num = *gyro_data.data_num;
                	buf_ofs += sizeof(*gyro_data.data_num);
                	t_diff_crop = (UINT64 *)(rsv1 + buf_ofs);
                	buf_ofs += sizeof(*gyro_data.t_diff_crop);
                	t_diff_crp_end_to_vd = (UINT64 *)(rsv1 + buf_ofs);

                	buf_ofs += sizeof(*gyro_data.t_diff_crp_end_to_vd);
                	agyro_x =  (INT32 *)(rsv1 + buf_ofs);
                	buf_ofs += (*gyro_data.data_num) * sizeof(INT32);
                	agyro_y =  (INT32 *)(rsv1 + buf_ofs);
                	buf_ofs += (*gyro_data.data_num) * sizeof(INT32);
                	agyro_z =  (INT32 *)(rsv1 + buf_ofs);

                	buf_ofs += (*gyro_data.data_num) * sizeof(INT32);
                	ags_x = (INT32 *)(rsv1 + buf_ofs);
                	buf_ofs += (*gyro_data.data_num) * sizeof(INT32);
                	ags_y = (INT32 *)(rsv1 + buf_ofs);
                	buf_ofs += (*gyro_data.data_num) * sizeof(INT32);
                	ags_z = (INT32 *)(rsv1 + buf_ofs);
                    {
                    UINT32 i = 0;
                    UINT32 pos=0;

                    ptr = va+keep_pos;
                    for(i=0;i<data_num;i++) {
                        sprintf(ptr+pos,"%lu %ld %ld %d %d %d %d %d %d %d\r\n",(unsigned long)video_frame.timestamp,(unsigned long)(*t_diff_crop),(unsigned long)(*t_diff_crp_end_to_vd),(int)sensor_expt.time[0],(int)*(agyro_x+i),(int)*(agyro_y+i),(int)*(agyro_z+i), \
                            (int)*(ags_x+i),(int)*(ags_y+i),(int)*(ags_z+i));
                        pos=strlen(ptr);
                    }
                    keep_pos += pos;
					if(g_show_gyro) {
                        printf("%s",ptr);
					}
                    //printf("##keep_pos %d %d ptr %lx %lx\r\n",keep_pos,pos,ptr,va);

                    }
				}
			}

///////////////////////
		//printf("proc_push ....\r\n");
		ret = hd_videoproc_push_in_buf(p_stream0->proc0_path, &video_frame, NULL, 0); // only support non-blocking mode now
		if (ret != HD_OK) {
			printf("proc_push error=%d !!\r\n\r\n", ret);
    			goto skip;
		}

		//printf("cap_release ....\r\n");
		ret = hd_videocap_release_out_buf(p_stream0->cap_path, &video_frame);
		if (ret != HD_OK) {
			printf("cap_release error=%d !!\r\n\r\n", ret);
    			goto skip;
		}
		hd_common_mem_munmap((void *)vir_addr_main, g_vcap_blk_size);

		p_stream0->cap_count ++;
		//printf("capture count = %d\r\n", p_stream0->cap_count);
skip:

		p_stream0->cap_loop++;
		usleep(100); //sleep for getchar()
	}


	if (f_out_main) fwrite(va, 1, keep_pos, f_out_main);
	if (f_out_main) fflush(f_out_main);
	// close output file
	if (f_out_main) fclose(f_out_main);
    hd_common_mem_release_block(blk);
	return 0;
}

#if DISPLAY_SHARE_BUF
static void *relay_yuv_thread(void *arg)
{
	VIDEO_RECORD* p_stream0 = (VIDEO_RECORD *)arg;
	VIDEO_RECORD* p_stream2 = p_stream0 + 2;
	HD_RESULT ret = HD_OK;
	HD_VIDEO_FRAME video_frame = {0};

	//------ wait flow_start ------
	while (p_stream0->flow_start == 0) sleep(1);

	printf("\r\nrelay main yuv  ....\r\n");

	//--------- pull data test ---------
	while (p_stream0->cap_exit == 0) {
		ret = hd_videoproc_pull_out_buf(p_stream0->proc1_path, &video_frame, 1000);
		if (ret != HD_OK) {
			continue;
		}
		printf("stream 0 reserved[0] = %d, w = %d, h =%d\r\n", video_frame.reserved[0], video_frame.dim.w, video_frame.dim.h);

		ret = hd_videoenc_push_in_buf(p_stream0->enc_path, &video_frame, NULL, -1); // -1 = blocking mode
		if (ret != HD_OK) {
			printf("push_in [0] error !!\r\n");
		}
		ret = hd_videoout_push_in_buf(p_stream2->out_path, &video_frame, NULL, -1); // -1 = blocking mode
		if (ret != HD_OK) {
			printf("push_in [0] error !!\r\n");
		}

		ret = hd_videoproc_release_out_buf(p_stream0->proc1_path, &video_frame);
		if (ret != HD_OK) {
			printf("release_out [0] error !!\r\n\r\n");
		}
	}
	return 0;
}
#endif

static void *encode_thread(void *arg)
{
	VIDEO_RECORD* p_stream0 = (VIDEO_RECORD *)arg;
	VIDEO_RECORD* p_stream1 = p_stream0 + 1;
	HD_RESULT ret = HD_OK;
	HD_VIDEOENC_BS  data_pull;
	UINT32 j;

	UINTPTR vir_addr_main;
	HD_VIDEOENC_BUFINFO phy_buf_main;
	char file_path_main[32] = "/mnt/sd/dump_bs_main.dat";
	FILE *f_out_main=NULL;
	#define PHY2VIRT_MAIN(pa) (vir_addr_main + (pa - phy_buf_main.buf_info.phy_addr))
	UINTPTR vir_addr_sub;
	HD_VIDEOENC_BUFINFO phy_buf_sub;
	char file_path_sub[32]  = "/mnt/sd/dump_bs_sub.dat";
	FILE *f_out_sub=NULL;
	#define PHY2VIRT_SUB(pa) (vir_addr_sub + (pa - phy_buf_sub.buf_info.phy_addr))
	HD_VIDEOENC_POLL_LIST poll_list[2];
	UINT32 poll_num = 2;

	//------ wait flow_start ------
	while (p_stream0->flow_start == 0) sleep(1);

	// query physical address of bs buffer ( this can ONLY query after hd_videoenc_start() is called !! )
	hd_videoenc_get(p_stream0->enc_path, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf_main);
	hd_videoenc_get(p_stream1->enc_path, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf_sub);

	// mmap for bs buffer (just mmap one time only, calculate offset to virtual address later)
	vir_addr_main = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_main.buf_info.phy_addr, phy_buf_main.buf_info.buf_size);
	vir_addr_sub  = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_sub.buf_info.phy_addr, phy_buf_sub.buf_info.buf_size);
    if (g_write_file) {
    	//----- open output files -----
    	if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
    		HD_VIDEOENC_ERR("open file (%s) fail....\r\n", file_path_main);
    	} else {
    		printf("\r\ndump main bitstream to file (%s) ....\r\n", file_path_main);
    	}
    	if ((f_out_sub = fopen(file_path_sub, "wb")) == NULL) {
    		HD_VIDEOENC_ERR("open file (%s) fail....\r\n", file_path_sub);
    	} else {
    		printf("\r\ndump sub  bitstream to file (%s) ....\r\n", file_path_sub);
    	}
    }
	printf("\r\nif you want to stop, enter \"q\" to exit !!\r\n\r\n");

	//--------- pull data test ---------
	poll_list[0].path_id = p_stream0->enc_path;
	poll_list[1].path_id = p_stream1->enc_path;


	//--------- pull data test ---------
	while (p_stream0->enc_exit == 0) {
		if (g_enc_poll && HD_OK == hd_videoenc_poll_list(poll_list, poll_num, -1)) {    // multi path poll_list , -1 = blocking mode
			if (TRUE == poll_list[0].revent.event) {
				//pull data
				ret = hd_videoenc_pull_out_buf(p_stream0->enc_path, &data_pull, -1); // -1 = blocking mode

				if (ret == HD_OK) {
					for (j=0; j< data_pull.pack_num; j++) {
						UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(data_pull.video_pack[j].phy_addr);
						UINT32 len = data_pull.video_pack[j].size;
						if (g_write_file && (!BIND_BSMUXER)) {
							if (f_out_main) fwrite(ptr, 1, len, f_out_main);
							if (f_out_main) fflush(f_out_main);
						}
					}
#if (BIND_BSMUXER == 1)
			push_video_to_bsmuxer_module(&(p_stream0->muxer_stream), &data_pull);
#endif

					// release data
					ret = hd_videoenc_release_out_buf(p_stream0->enc_path, &data_pull);
				}
			}

			if (TRUE == poll_list[1].revent.event) {
				//pull data
				ret = hd_videoenc_pull_out_buf(p_stream1->enc_path, &data_pull, -1); // -1 = blocking mode
				if (ret == HD_OK) {
					for (j=0; j< data_pull.pack_num; j++) {
						UINT8 *ptr = (UINT8 *)PHY2VIRT_SUB(data_pull.video_pack[j].phy_addr);
						UINT32 len = data_pull.video_pack[j].size;
						if (g_write_file && (!BIND_BSMUXER)) {
							if (f_out_sub) fwrite(ptr, 1, len, f_out_sub);
							if (f_out_sub) fflush(f_out_sub);
						}
					}
#if (BIND_BSMUXER == 1)
			push_video_to_bsmuxer_module(&(p_stream1->muxer_stream), &data_pull);
#endif
					// release data
					ret = hd_videoenc_release_out_buf(p_stream1->enc_path, &data_pull);
				}
			}
		} else {
			usleep(100000);
		}
	}

	// mummap for bs buffer
	hd_common_mem_munmap((void *)vir_addr_main, phy_buf_main.buf_info.buf_size);
	hd_common_mem_munmap((void *)vir_addr_sub, phy_buf_sub.buf_info.buf_size);

	// close output file
	if (f_out_main) fclose(f_out_main);
	if (f_out_sub) fclose(f_out_sub);

	return 0;
}

MAIN(argc, argv)
{
	HD_RESULT ret;
	INT key;
	VIDEO_RECORD stream[3] = {0}; //0: main stream, 1: sub stream,2: liveview
	UINT32 enc_type = 0;
	HD_DIM main_dim;
	HD_DIM sub_dim;
	UINT32 out_type = 2;
	PD_SHM_INFO  *p_pd_shm;
	AET_CFG_INFO cfg_info = {0};
	AET_EXPT_BOUND expt_bound = {0};
	UINT32       calibration = 0;
	HD_VIDEO_PXLFMT ipp_out_fmt;

	// query program options

	if (argc == 1 || argc > 7) {
		printf("Usage: <enc-type> <cap_out_bind> <cap_out_fmt> <prc_out_bind> <prc_out_fmt>.\r\n");
		printf("Help:\r\n");
		printf("  <enc_type> : 0(H265), 1(H264), 2(MJPG)\r\n");
		printf("  <cap_out_bind>: 0(D2D Mode), 1(Direct Mode), 2(OneBuf Mode)\r\n");
		printf("  <sensor> :  0(IMX290) , 4(IMX415)\r\n");
		printf("  <fps>  : 30,60\r\n");
		printf("  <calibration mode>  :  0, 1\r\n");
		printf("  <gyro num>\r\n");
		return 0;
	}


	if (argc > 1) {
		enc_type = atoi(argv[1]);
		printf("enc_type %d\r\n", enc_type);
		if(enc_type > 2) {
			printf("error: not support enc_type!\r\n");
			return 0;
		}
	}

	if (argc > 2) {
		if (argv[2][0] == 'x') {
			g_capbind = 0xff;
			printf("CAP-BIND x\r\n");
		} else {
			g_capbind = atoi(argv[2]);
			printf("CAP-BIND %d\r\n", g_capbind);
			if(g_capbind > 3) {
				printf("error: not support CAP-BIND! (%d) \r\n", g_capbind);
				return 0;
			}
		}
	}
	if (argc > 3) {
		sensor_sel = atoi(argv[3]);
		printf("sensor_sel %d\r\n", sensor_sel);
		if(sensor_sel > 5 && sensor_sel != SEN_SEL_PATGEN) {
			printf("error: not support sensor_sel!\r\n");
			return 0;
		}
	}
	if (argc > 4) {
		g_fps = atoi(argv[4]);
		printf("g_fps %d\r\n", g_fps);
	}
	if (argc > 5) {
		calibration = atoi(argv[5]);
		printf("calibration %d\r\n", calibration);
	}

	if (argc > 6) {
		g_gyro_num = atoi(argv[6]);
		printf("gyro_num %d\r\n", g_gyro_num);
	}

    if (sensor_sel == SEN_SEL_IMX415) {
		VDO_SIZE_W = VDO_SIZE_W_8M;
		VDO_SIZE_H = VDO_SIZE_H_8M;
		vdo_br = VDO_BITRATE_8M;
        vdo_br_rtsp = VDO_BITRATE_RTSP_8M;
	} else if (sensor_sel == SEN_SEL_OS04A10) {
		VDO_SIZE_W = VDO_SIZE_W_4M;
		VDO_SIZE_H = VDO_SIZE_H_4M;
		vdo_br = VDO_BITRATE_4M;
        vdo_br_rtsp = VDO_BITRATE_RTSP_4M;
	} else {
		VDO_SIZE_W = VDO_SIZE_W_2M;
		VDO_SIZE_H = VDO_SIZE_H_2M;
		vdo_br =VDO_BITRATE_2M;
        vdo_br_rtsp = VDO_BITRATE_RTSP_2M;

	}

    printf("###main %d*%d vdo_br=%d vdo_br_rtsp=%d\r\n",VDO_SIZE_W,VDO_SIZE_H,vdo_br,vdo_br_rtsp);
	if (vendor_isp_init() != HD_ERR_NG) {
		cfg_info.id = 0;
    if (sensor_sel == SEN_SEL_IMX415) {
		//strncpy(cfg_info.path, "/mnt/app/isp/isp_imx415_0.cfg", CFG_NAME_LENGTH);
		strncpy(cfg_info.path, "/mnt/app/isp/isp_imx415_eis.cfg", CFG_NAME_LENGTH);
    }else if (sensor_sel == SEN_SEL_OS04A10) {
		strncpy(cfg_info.path, "/mnt/app/isp/isp_os04a10_0.cfg", CFG_NAME_LENGTH);
    }else{
		strncpy(cfg_info.path, "/mnt/app/isp/isp_imx290_0.cfg", CFG_NAME_LENGTH);
    }
		printf("sensor 1 load %s \n", cfg_info.path);
		vendor_isp_set_ae(AET_ITEM_RLD_CONFIG, &cfg_info);
		expt_bound.id = 0;
		expt_bound.bound.l = (UINT32)500;
		expt_bound.bound.h = (UINT32)8000;
		vendor_isp_set_ae(AET_ITEM_EXPT_BOUND, &expt_bound);
		vendor_isp_set_awb(AWBT_ITEM_RLD_CONFIG, &cfg_info);
		vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);
		vendor_isp_uninit();
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

	init_share_memory();

	// init all modules
	ret = init_module();
	if (ret != HD_OK) {
		printf("init fail=%d\n", ret);
		goto exit;
	}
#if (BIND_BSMUXER == 1)
	init_filesys_module(&(stream[0].muxer_stream));
	ret = init_bsmuxer_module();
	if (ret != HD_OK) {
		printf("init bsmuxer fail=%d\n", ret);
		goto exit;
	}
#endif

	// init EIS lib
	ret = init_eis();
	if (ret != HD_OK) {
		printf("eis init fail=%d\n", ret);
		goto exit;
	}
	ret = init_vpe();
	if (ret != HD_OK) {
		printf("vpe init fail=%d\n", ret);
		goto exit;
	}

	// open video_record modules (pre-process)
	stream[0].proc0_max_dim.w = VDO_SIZE_W; //assign by user
	stream[0].proc0_max_dim.h = VDO_SIZE_H; //assign by user
	// open video_record modules (main)
	stream[0].proc1_max_dim.w = VDO_SIZE_W; //assign by user
	stream[0].proc1_max_dim.h = VDO_SIZE_H; //assign by user
	ret = open_module(&stream[0], &stream[0].proc0_max_dim, &stream[0].proc1_max_dim);
	if (ret != HD_OK) {
		printf("open fail=%d\n", ret);
		goto exit;
	}
#if (BIND_BSMUXER == 1)
	ret = open_bsmuxer_module(&(stream[0].muxer_stream));
	if (ret != HD_OK) {
		printf("open bsmuxer fail=%d\n", ret);
		goto exit;
	}
#endif

	// open video_record modules (sub)
	ret = open_module_2(&stream[1]);
	if (ret != HD_OK) {
		printf("open fail=%d\n", ret);
		goto exit;
	}
#if (BIND_BSMUXER == 1)
	ret = open_bsmuxer_module1(&(stream[1].muxer_stream));
	if (ret != HD_OK) {
		printf("open bsmuxer1 fail=%d\n", ret);
		goto exit;
	}
#endif

	// open vout modules (display)
	if (0) {
		ret = open_module_3(&stream[2], out_type);
		if (ret != HD_OK) {
			printf("open fail=%d\n", ret);
			goto exit;
		}
	}

	// get videocap capability
	ret = get_cap_caps(stream[0].cap_ctrl, &stream[0].cap_syscaps);
	if (ret != HD_OK) {
		printf("get cap-caps fail=%d\n", ret);
		goto exit;
	}

	// get videoout capability
	if (0) {
		ret = get_out_caps(stream[2].out_ctrl, &stream[2].out_syscaps);
		if (ret != HD_OK) {
			printf("get out-caps fail=%d\n", ret);
			goto exit;
		}
	}

    if (sensor_sel == SEN_SEL_IMX415) {
    	stream[2].out_dim.w = VDO_SIZE_W/6; //using vpe/8 and ide scale
    	stream[2].out_dim.h = VDO_SIZE_H/6; //using vpe/8 and ide scale
    } else {
	    stream[2].out_max_dim = stream[2].out_syscaps.output_dim;
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
	sub_dim.w = SUB_VDO_SIZE_W;
	sub_dim.h = SUB_VDO_SIZE_H;

	// set videoproc parameter (pre-process)
	stream[0].proc1_dim.w = main_dim.w;
	stream[0].proc1_dim.h = main_dim.h;
	if (calibration) {
		ipp_out_fmt = HD_VIDEO_PXLFMT_YUV420_NVX2;
	} else {
		ipp_out_fmt = IPP_OUT_FMT;
	}
	ret = set_proc_param(stream[0].proc0_path, &stream[0].proc1_dim, ipp_out_fmt);
	if (ret != HD_OK) {
		printf("set proc fail=%d\n", ret);
		goto exit;
	}

	if (HD_VIDEO_PXLFMT_YUV420_NVX5 == ipp_out_fmt)	{
		UINT32 h_align;

		h_align = 16;
        ret = vendor_videoproc_set(stream[0].proc0_path, VENDOR_VIDEOPROC_PARAM_HEIGHT_ALIGN, &h_align);
        if (ret != HD_OK) {
            printf("VENDOR_VIDEOPROC_PARAM_HEIGHT_ALIGN failed!(%d)\r\n", ret);
        }
    }

	// set videoproc parameter (main)
	stream[0].proc1_dim.w = main_dim.w;
	stream[0].proc1_dim.h = main_dim.h;
	ret = set_proc_param(stream[0].proc1_path, &stream[0].proc1_dim, VPE_OUT_FMT);
	if (ret != HD_OK) {
		printf("set proc fail=%d\n", ret);
		goto exit;
	}

	stream[1].proc1_dim.w = sub_dim.w;
	stream[1].proc1_dim.h = sub_dim.h;
	// set videoproc parameter (sub)
	ret = set_proc_param(stream[1].proc1_path, NULL, VPE_OUT_FMT);
	if (ret != HD_OK) {
		printf("set proc fail=%d\n", ret);
		goto exit;
	}

    //vpe scale ratio is 1/8,ide scale ratio is 1/2
    if (sensor_sel == SEN_SEL_IMX415) {
	// set videoout parameter (main)
		#if DISPLAY_SHARE_BUF
		stream[2].out_dim.w = VDO_SIZE_W;
		stream[2].out_dim.h = VDO_SIZE_H;
		#else
		stream[2].out_dim.w = VDO_SIZE_W/6; //using vpe/8 and ide scale
		stream[2].out_dim.h = VDO_SIZE_H/6; //using vpe/8 and ide scale
		#endif

    } else {
	// set videoout parameter (main)
	stream[2].out_dim.w = stream[2].out_max_dim.w; //using device max dim.w
	stream[2].out_dim.h = stream[2].out_max_dim.h; //using device max dim.h
    }

	if (0) {
		ret = set_out_param(stream[2].out_path, &stream[2].out_dim);
		if (ret != HD_OK) {
			printf("set out fail=%d\n", ret);
			goto exit;
		}
	}

	// set videoenc config (main)
	stream[0].enc_max_dim.w = main_dim.w;
	stream[0].enc_max_dim.h = main_dim.h;
	ret = set_enc_cfg(stream[0].enc_path, &stream[0].enc_max_dim, vdo_br);
	if (ret != HD_OK) {
		printf("set enc-cfg fail=%d\n", ret);
		goto exit;
	}

	// set videoenc parameter (main)
	stream[0].enc_dim.w = main_dim.w;
	stream[0].enc_dim.h = main_dim.h;
	ret = set_enc_param(stream[0].enc_path, &stream[0].enc_dim, enc_type, vdo_br);
	if (ret != HD_OK) {
		printf("set enc fail=%d\n", ret);
		goto exit;
	}

	// set videoenc config (sub)
	stream[1].enc_max_dim.w = sub_dim.w;
	stream[1].enc_max_dim.h = sub_dim.h;
	ret = set_enc_cfg(stream[1].enc_path, &stream[1].enc_max_dim, sub_vdo_br);
	if (ret != HD_OK) {
		printf("set enc-cfg fail=%d\n", ret);
		goto exit;
	}

	// set videoenc parameter (sub)
	stream[1].enc_dim.w = sub_dim.w;
	stream[1].enc_dim.h = sub_dim.h;
	ret = set_enc_param(stream[1].enc_path, &stream[1].enc_dim, enc_type, sub_vdo_br);
	if (ret != HD_OK) {
		printf("set enc fail=%d\n", ret);
		goto exit;
	}

	if (g_capbind == 0xff) { //no-bind mode
		// create capture_thread (pull_out frame, push_in frame then pull_out frame)
		ret = pthread_create(&stream[0].cap_thread_id, NULL, capture_thread, (void *)stream);
		if (ret < 0) {
			printf("create capture_thread failed");
			goto exit;
		}
	} else {
    	// bind video_record modules (main)
    	hd_videocap_bind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);
	}
	if (calibration) {
		hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOENC_0_IN_0);
	} else {
		hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOPROC_1_IN_0);
#if !DISPLAY_SHARE_BUF
		hd_videoproc_bind(HD_VIDEOPROC_1_OUT_0, HD_VIDEOENC_0_IN_0);
#endif
	}
	// bind video_record modules (sub)-no-eis
	hd_videoproc_bind(HD_VIDEOPROC_0_OUT_1, HD_VIDEOENC_0_IN_1);
#if DISPLAY
	// bind vout modules (display)
	hd_videoproc_bind(HD_VIDEOPROC_1_OUT_2, HD_VIDEOOUT_0_IN_0);
#endif
	// create encode_thread (pull_out bitstream)
	ret = pthread_create(&stream[0].enc_thread_id, NULL, encode_thread, (void *)stream);
	if (ret < 0) {
		printf("create encode thread failed");
		goto exit;
	}
	#if DISPLAY_SHARE_BUF
	ret = pthread_create(&stream[0].relay_thread_id, NULL, relay_yuv_thread, (void *)stream);
	if (ret < 0) {
		printf("create encode thread failed");
		goto exit;
	}
	#endif

	if (g_capbind == 1) {
		//direct NOTE: ensure videocap start after 1st videoproc phy path start
		hd_videoproc_start(stream[0].proc0_path);
		hd_videocap_start(stream[0].cap_path);
	} else {
		hd_videocap_start(stream[0].cap_path);
		hd_videoproc_start(stream[0].proc0_path);
	}
	// start video_record modules (main)
	hd_videoproc_start(stream[0].proc1_path);
	// start video_record modules (sub)
	hd_videoproc_start(stream[1].proc1_path);
#if DISPLAY
	// start vout modules (display)
	hd_videoproc_start(stream[2].proc1_path);
#endif
	// just wait ae/awb stable for auto-test, if don't care, user can remove it
	sleep(1);
	hd_videoenc_start(stream[0].enc_path);
	hd_videoenc_start(stream[1].enc_path);
#if (BIND_BSMUXER == 1)
	if (enc_type == 0) {
		stream[0].muxer_stream.vidcodec = HD_BSMUX_VIDCODEC_H265;
		stream[1].muxer_stream.vidcodec = HD_BSMUX_VIDCODEC_H265;
	}else if (enc_type == 1) {
		stream[0].muxer_stream.vidcodec = HD_BSMUX_VIDCODEC_H264;
		stream[1].muxer_stream.vidcodec = HD_BSMUX_VIDCODEC_H264;
    }else {
		printf("muxer vidcodec not supported\r\n");
    }
	hd_videoenc_get(stream[0].enc_path, HD_VIDEOENC_PARAM_BUFINFO, &(stream[0].muxer_stream.bsmux_vencbufinfo));
	set_bsmuxer_config(&(stream[0].muxer_stream));
	start_bsmuxer_module(&(stream[0].muxer_stream));
	hd_videoenc_get(stream[1].enc_path, HD_VIDEOENC_PARAM_BUFINFO, &(stream[1].muxer_stream.bsmux_vencbufinfo));
	set_bsmuxer_config(&(stream[1].muxer_stream));
	start_bsmuxer_module(&(stream[1].muxer_stream));
#endif

#if DISPLAY || DISPLAY_SHARE_BUF
	hd_videoout_start(stream[2].out_path);
#endif

	if (g_capbind == 0xff) { //no-bind mode
		// start capture_thread
		stream[0].cap_enter = 1;
	}

	// let encode_thread start to work
	stream[0].flow_start= 1;

	// query user key
	printf("Enter q to exit\n");
	while (1) {
		key = GETCHAR();
		if (key == 'q' || key == 0x3) {
        	if (g_capbind == 0xff) { //no-bind mode
        		// stop capture_thread
        		stream[0].cap_exit = 1;
        	}

			// let encode_thread stop loop and exit
			stream[0].enc_exit = 1;
			// quit program
			break;
		}

		if (key == 'z') {
			// enter debug menu
			system("echo dbg 1 1 > /proc/hdal/vendor/eis/cmd");

		}
        if (key == 'c') {
			printf("start rtsp\r\n");
            g_enc_poll = 0;
        	hd_videoenc_stop(stream[0].enc_path);
        	hd_videoenc_stop(stream[1].enc_path);
            usleep(500000);
        	ret = set_enc_param(stream[0].enc_path, &stream[0].enc_dim, enc_type, vdo_br_rtsp);
        	ret = set_enc_param(stream[1].enc_path, &stream[1].enc_dim, enc_type, sub_vdo_br);
            usleep(500000);
        	hd_videoenc_start(stream[0].enc_path);
        	hd_videoenc_start(stream[1].enc_path);
            usleep(500000);

			system("nvtrtspd_ipc &");
		}

        if (key == '4') {
            hd_videoenc_stop(stream[0].enc_path);
            hd_videoenc_stop(stream[1].enc_path);
            printf("stop encode\r\n");
        }
        if (key == '5') {
            hd_videoenc_start(stream[0].enc_path);
            hd_videoenc_start(stream[1].enc_path);
            printf("start encode\r\n");
        }
        if (key == '6') {
        	ret = set_enc_param(stream[0].enc_path, &stream[0].enc_dim, enc_type, vdo_br_rtsp);
        	ret = set_enc_param(stream[1].enc_path, &stream[1].enc_dim, enc_type, sub_vdo_br);
            printf("encode bitrate\r\n");
        }
		if (key == 'm') {
			g_show_gyro =1;
		}
		if (key == 'n') {
			g_show_acc =1;
		}
		#if (DEBUG_MENU == 1)
		if (key == 'd') {
			// enter debug menu
			hd_debug_run_menu();
			printf("\r\nEnter q to exit, Enter d to debug\r\n");
		}
		#endif
	}


	// notify child process
	p_pd_shm = (PD_SHM_INFO  *)g_shm;
	p_pd_shm->exit = 1;
	sleep(1);
	// destroy encode thread
	pthread_join(stream[0].enc_thread_id, NULL);
	#if DISPLAY_SHARE_BUF
	pthread_join(stream[0].relay_thread_id, NULL);
	#endif

	if (g_capbind == 1) {
		//direct NOTE: ensure videocap stop after all videoproc path stop
		hd_videoproc_stop(stream[0].proc0_path);
		hd_videocap_stop(stream[0].cap_path);
	} else {
		hd_videocap_stop(stream[0].cap_path);
		hd_videoproc_stop(stream[0].proc0_path);
	}
	 // stop video_record modules (main)
	hd_videoproc_stop(stream[0].proc1_path);
    // stop video_record modules (sub)
	hd_videoproc_stop(stream[1].proc1_path);
    // stop vout modules (dispaly)
	hd_videoproc_stop(stream[2].proc1_path);
	hd_videoenc_stop(stream[0].enc_path);
	hd_videoenc_stop(stream[1].enc_path);
#if (BIND_BSMUXER == 1)
	stop_bsmuxer_module(&(stream[0].muxer_stream));
	stop_bsmuxer_module(&(stream[1].muxer_stream));
#endif

#if DISPLAY || DISPLAY_SHARE_BUF
	hd_videoout_stop(stream[2].out_path);
#endif

	if (g_capbind == 0xff) { //no-bind mode
		// destroy capture_thread
		pthread_join(stream[0].cap_thread_id, NULL);  //NOTE: before destory, call stop to breaking pull(-1)
	} else {
    	// unbind video_record modules (main)
    	hd_videocap_unbind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0));
    }

	hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_0);
	hd_videoproc_unbind(HD_VIDEOPROC_1_OUT_0);

	// unbind video_record modules (sub)
	hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_1);
#if DISPLAY
	// unbind vout modules (display)
	hd_videoproc_unbind(HD_VIDEOPROC_1_OUT_2);
#endif
exit:
	// close video_record modules (main)
	ret = close_module(&stream[0]);
	if (ret != HD_OK) {
		printf("close fail=%d\n", ret);
	}

	// close video_record modules (sub)
	ret = close_module_2(&stream[1]);
	if (ret != HD_OK) {
		printf("close2 fail=%d\n", ret);
	}
	if (0) {
		// close vout modules (display)
		ret = close_module_3(&stream[2]);
		if (ret != HD_OK) {
			printf("close3 fail=%d\n", ret);
		}
	}
#if (BIND_BSMUXER == 1)
	ret = close_bsmuxer_module(&(stream[0].muxer_stream));
	if (ret != HD_OK) {
		printf("close bsmuxer fail=%d\n", ret);
		goto exit;
	}
	ret = close_bsmuxer_module(&(stream[1].muxer_stream));
	if (ret != HD_OK) {
		printf("close bsmuxer1 fail=%d\n", ret);
		goto exit;
	}
#endif

	vendor_vpe_uninit();
	ret = vendor_eis_close();
	if (ret != HD_OK) {
		printf("eis close fail=%d\n", ret);
	}
    if (vendor_isp_uninit() == HD_ERR_NG) {
		return -1;
	}

	// uninit all modules
	ret = exit_module();
	if (ret != HD_OK) {
		printf("exit fail=%d\n", ret);
	}
#if (BIND_BSMUXER == 1)
	ret = exit_bsmuxer_module();
	if (ret != HD_OK) {
		printf("exit bsmuxer fail=%d\n", ret);
		goto exit;
	}
	exit_filesys_module(&(stream[0].muxer_stream));
#endif

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
	exit_share_memory();

	return 0;
}
