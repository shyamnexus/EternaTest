/**
	@brief Sample code of video rtsp.\n

	@file pq_emu_rtsp.c

	@author Photon Lin

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include "hdal.h"
#include "hd_debug.h"
#include "vendor_isp.h"
#include "vendor_common.h"
#include "vendor_videocapture.h"
#include "vendor_videoprocess.h"
#include "vendor_videoenc.h"
#include "pd_shm.h"

// platform dependent
#include <pthread.h> //for pthread API
#define MAIN(argc, argv) int main(int argc, char** argv)
#define GETCHAR() getchar()

///////////////////////////////////////////////////////////////////////////////
//function control
#define BYPASS_1ST_IQ_ENABLE       1
#if defined(_CHIP_NT98690_) || defined(_CHIP_NT98538_)
#define VPRC_PATH5_ENABLE     1
#else
#define VPRC_PATH5_ENABLE     0
#endif
#define SAVE_YUV_ENABLE            1
#define SAVE_YUV_PACK              1
#define MEASURE_ENABLE             1
#define GARY_FOR_1ST_FRM_ENABLE    0
#if defined(_CHIP_NT9852x_) || defined(_CHIP_NT98530_) || defined(_CHIP_NT98690_) || defined(_CHIP_NT98538_) || defined(_CHIP_NT98567_)
#define VPE_FUN_ENABLE             1
#else
#define VPE_FUN_ENABLE             0
#endif
#define SUB_VDO_ENABLE             0
#define EMU_AI_ENABLE              1
#define LOAD_2A_INFO               1

#define FRAME_NUM_MAX   2
#define RUN_NUM_MAX     200
#define RAW_FILE_LINEAR "/%s/%s/RAW0_w%d_h%d_12b_pack_%d.raw"
#define RAW_FILE_HDR_0  "/%s/%s/RAW0_w%d_h%d_12b_pack_%d.raw"
#define RAW_FILE_HDR_1  "/%s/%s/RAW1_w%d_h%d_12b_pack_%d.raw"
#define RAW_FILE_HDR_2  "/%s/%s/RAW2_w%d_h%d_12b_pack_%d.raw"
#define YUV_FILE        "/%s/frame%04d_yuv420_w%d_h%d.yuv"
#define _2A_INFO_FILE   "/%s/%s/raw_2a_info.bin"

#if (MEASURE_ENABLE)
#include <sys/time.h>
#endif

#if (EMU_AI_ENABLE)
#include "aiisp.h"
#endif

#define VIDEOCAP_ALG_FUNC HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB
#if defined(_CHIP_NT9852x_)
#define VIDEOPROC_ALG_FUNC HD_VIDEOPROC_FUNC_AF | HD_VIDEOPROC_FUNC_WDR | HD_VIDEOPROC_FUNC_COLORNR | HD_VIDEOPROC_FUNC_DEFOG | HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA
#elif defined(_CHIP_NT9856x_)
#define VIDEOPROC_ALG_FUNC HD_VIDEOPROC_FUNC_AF | HD_VIDEOPROC_FUNC_WDR | HD_VIDEOPROC_FUNC_COLORNR | HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA
#elif defined(_CHIP_NT98567_)
#define VIDEOPROC_ALG_FUNC HD_VIDEOPROC_FUNC_AF | HD_VIDEOPROC_FUNC_WDR | HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA
#else // (_CHIP_NT98530_), (_CHIP_NT98690_), (_CHIP_NT98538_)
#define VIDEOPROC_ALG_FUNC HD_VIDEOPROC_FUNC_AF | HD_VIDEOPROC_FUNC_WDR | HD_VIDEOPROC_FUNC_DEFOG | HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA
#endif

///////////////////////////////////////////////////////////////////////////////
//sensor size
#define VDO_SIZE_W_DEFAULT      2688
#define VDO_SIZE_H_DEFAULT      1520

///////////////////////////////////////////////////////////////////////////////
//stripe level
#define HD_VIDEOPROC_CFG                0x000F0000  //vprc
#define HD_VIDEOPROC_CFG_STRIP_LV1      0x00000000  //vprc "0: cut w>1280, GDC =  on, 2D_LUT off after cut (LL slow)
#define HD_VIDEOPROC_CFG_STRIP_LV2      0x00010000  //vprc "1: cut w>2048, GDC = off, 2D_LUT off after cut (LL fast)
#define HD_VIDEOPROC_CFG_STRIP_LV3      0x00020000  //vprc "2: cut w>2688, GDC = off, 2D_LUT off after cut (LL middle)(2D_LUT best)
#define HD_VIDEOPROC_CFG_STRIP_LV4      0x00030000  //vprc "3: cut w> 720, GDC =  on, 2D_LUT off after cut (LL not allow)(GDC best)

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

#define ISP_ID_REMAP(sen_id, sie_id, ipp_id)  ((0x80008000) | ((sen_id & 0x7F) << 24) | (sie_id & 0x7F) << 8 | (ipp_id & 0xFF))

///////////////////////////////////////////////////////////////////////////////
#define SEN_OUT_FMT        HD_VIDEO_PXLFMT_RAW12
#define CAP_OUT_FMT        HD_VIDEO_PXLFMT_RAW12
#define SHDR2_CAP_OUT_FMT  HD_VIDEO_PXLFMT_RAW12_SHDR2
#define SHDR3_CAP_OUT_FMT  HD_VIDEO_PXLFMT_RAW12_SHDR3

#define SEN_VCAP_ID_0      0
#define SEN_VCAP_ID_2      2
#define SEN_VCAP_ID_3      3
#define SEN_VCAP_ID_4      4
#define SEN_VCAP_ID_5      5

#define MAX_DDR_NUM        2

///////////////////////////////////////////////////////////////////////////////
// control parameters 1
static UINT32 run_num_max = 20;
static BOOL conti_run = TRUE;
static BOOL save_yuv_en;
static BOOL measure_vcap_en, maesure_read_en;
static BOOL load_raw_via_ddr = TRUE;
static UINT32 frame_delay = 60;
static BOOL bypass_1st_3dnr_en = FALSE, bypass_1st_lce_en = FALSE, bypass_1st_wdr_en = FALSE;
#if (LOAD_2A_INFO)
static BOOL load_2a_info_en = FALSE;
#endif

///////////////////////////////////////////////////////////////////////////////
// control parameters 2
static UINT32 sen_vcap_id = SEN_VCAP_ID_0;
static UINT32 vdo_size_w = VDO_SIZE_W_DEFAULT, vdo_size_h = VDO_SIZE_H_DEFAULT;
static UINT32 frame_num = 1;
static UINT32 init_c_gain[3] = {512, 256, 512};
static UINT32 init_total_gain = 100;
#if (VPE_FUN_ENABLE)
static BOOL vpe_enable = FALSE;
#endif
static UINT32 strpix = 0;
static UINT32 enc_type = 1;
static UINT32 enc_bitrate = 8;
static BOOL is_nt98539a = FALSE;

///////////////////////////////////////////////////////////////////////////////
// control parameters 3
static CHAR linear_path[100] =  "linear";
static CHAR shdr_path[100] =    "shdr";
static CHAR isp_cfg_name[100] = "isp_os04a10_0_AI_539a";
static CHAR load_raw_folder[100] = "mnt/sd";
#if SUB_VDO_ENABLE
static UINT32 sub_vdo_size_w = 640, sub_vdo_size_h = 480;
#endif

///////////////////////////////////////////////////////////////////////////////
// control parameters 4
#if (EMU_AI_ENABLE)
static BOOL aiisp_flow_enable = FALSE;
#endif

///////////////////////////////////////////////////////////////////////////////
typedef struct _EMU_BUFFER {
	UINT32               buf_size;
	#if defined(_CHIP_NT98530_) || defined(_CHIP_NT98690_) || defined(_CHIP_NT98538_) || defined(_CHIP_NT98567_)
	UINTPTR              buf_pa;
	#else
	UINT32               buf_pa;
	#endif
	void                 *buf_va;
} EMU_BUFFER;

static EMU_BUFFER emu_buffer[RUN_NUM_MAX][FRAME_NUM_MAX];

///////////////////////////////////////////////////////////////////////////////
typedef struct _VIDEO_EMU {
	// (1)
	HD_VIDEOCAP_SYSCAPS cap_syscaps;
	HD_PATH_ID cap_ctrl;
	HD_PATH_ID cap_path;

	HD_DIM  cap_dim;
	HD_DIM  proc_max_dim;

	// (2)
	HD_PATH_ID proc_ctrl;
	HD_PATH_ID proc_ctrl2;
	HD_PATH_ID proc_path;
	HD_PATH_ID proc_path2;
	#if (SUB_VDO_ENABLE)
	HD_PATH_ID proc_path_sub;
	#endif
	#if (VPRC_PATH5_ENABLE)
	HD_PATH_ID proc_path_3dnr;
	#endif

	// (3)
	HD_PATH_ID enc_path;
	#if (SUB_VDO_ENABLE)
	HD_PATH_ID enc_path_sub;
	#endif
	HD_DIM  enc_max_dim;
	HD_DIM  enc_dim;

	// (4) user pull
} VIDEO_EMU;

VIDEO_EMU emu_stream = {0};
///////////////////////////////////////////////////////////////////////////////

typedef struct _DDR_INFO {
	void *va;
	#if defined(_CHIP_NT98530_) || defined(_CHIP_NT98690_) || defined(_CHIP_NT98538_) || defined(_CHIP_NT98567_)
	UINTPTR pa;
	#else
	UINT32 pa;
	#endif
	UINT32 size;
} DDR_INFO;

DDR_INFO ddr_max_free[MAX_DDR_NUM] = {0};
DDR_INFO ddr_remain[MAX_DDR_NUM] = {0};
///////////////////////////////////////////////////////////////////////////////
PD_SHM_INFO  *p_pd_shm;
static char *g_shm = NULL;
static int g_shmid = 0;

static BOOL emu_thread_start, emu_flow_start, emu_flow_stop, save_yuv_start;
FILE *fp[RUN_NUM_MAX][FRAME_NUM_MAX];
pthread_t emu_thread_id;
static BOOL is_init_emu;
static UINT32 run_num_cnt = 10, file_start_num, save_yuv_cnt;
static BOOL conti_run_1st_loop, yuv_bypass_1st_frm, emu_bypass_1st_frm;
static BOOL is_file_open[RUN_NUM_MAX];
static BOOL is_load_raw_via_ddr;
static UINT32 pre_load_max_num;
static UINT32 raw_size;
#if (LOAD_2A_INFO)
typedef struct _RAW_2A_INFO {
	UINT32 expt;
	UINT32 gain;
	UINT32 cg_r;
	UINT32 cg_g;
	UINT32 cg_b;
} RAW_2A_INFO;

static RAW_2A_INFO *p_raw_2a_info = NULL;
static UINT32 raw_2a_info_size;
#endif

static HD_RESULT mem_init(void)
{
	HD_RESULT ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
	UINT32 pool_num = 0;

	// config common pool (cap)
	mem_cfg.pool_info[pool_num].type = HD_COMMON_MEM_COMMON_POOL;

	mem_cfg.pool_info[pool_num].blk_size = DBGINFO_BUFSIZE() +
									VDO_RAW_BUFSIZE(vdo_size_w, vdo_size_h, CAP_OUT_FMT) + 
									VDO_CA_BUF_SIZE(ISP_CA_W_WINNUM, ISP_CA_H_WINNUM) + 
									VDO_LA_BUF_SIZE(ISP_LA_W_WINNUM, ISP_LA_H_WINNUM);
	mem_cfg.pool_info[pool_num].blk_cnt = 4 * frame_num;
	mem_cfg.pool_info[pool_num].ddr_id = DDR_ID0;
	
	// config common pool
	pool_num++;  // 1
	mem_cfg.pool_info[pool_num].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[pool_num].blk_size = DBGINFO_BUFSIZE() + 
									VDO_YUV_BUFSIZE(vdo_size_w, vdo_size_h, HD_VIDEO_PXLFMT_YUV420) + 
									VDO_VA_BUF_SIZE(ISP_VA_W_WINNUM, ISP_VA_H_WINNUM) + 
									MD_HEAD_BUFSIZE() + MD_INFO_BUFSIZE(vdo_size_w, vdo_size_h);
	#if defined(_CHIP_NT9856x_)
	mem_cfg.pool_info[pool_num].blk_cnt = 4;
	#else
	mem_cfg.pool_info[pool_num].blk_cnt = 4 * (vpe_enable ? 2 : 1);
	#endif
	#if (SUB_VDO_ENABLE)
	mem_cfg.pool_info[pool_num].blk_cnt *= 2;
	#endif
	if (is_nt98539a) {
		mem_cfg.pool_info[pool_num].blk_cnt += 5;
	}
	mem_cfg.pool_info[pool_num].ddr_id = DDR_ID0;

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

	snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN - 1, "PATTERN_GEN");
	printf("sensor 1: using %s \n", cap_cfg.sen_cfg.sen_dev.driver_name);

	if (sen_vcap_id == SEN_VCAP_ID_0) {
		if (frame_num == 2) {
			cap_cfg.sen_cfg.shdr_map = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR1, (HD_VIDEOCAP_0|HD_VIDEOCAP_1));
			printf("sensor 1: shdr map 0/1 \n");
		} else if (frame_num == 3) {
			cap_cfg.sen_cfg.shdr_map = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR1, (HD_VIDEOCAP_0|HD_VIDEOCAP_1|HD_VIDEOCAP_2));
			printf("sensor 1: shdr map 0/1/2 \n");
		}
	} else {
		if (frame_num == 2) {
			cap_cfg.sen_cfg.shdr_map = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR1, (HD_VIDEOCAP_2|HD_VIDEOCAP_4));
			printf("sensor 1: shdr map 2/4 \n");
		} else if (frame_num == 3) {
			cap_cfg.sen_cfg.shdr_map = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR1, (HD_VIDEOCAP_2|HD_VIDEOCAP_3|HD_VIDEOCAP_4));
			printf("sensor 1: shdr map 2/3/4 \n");
		}
	}

	ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(sen_vcap_id), &video_cap_ctrl);
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
	iq_ctl.func = VIDEOCAP_ALG_FUNC;

	if (frame_num > 1) {
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

		color_bar_width = (vdo_size_w >> 3) & 0xFFFFFFFE;
		video_in_param.sen_mode = HD_VIDEOCAP_PATGEN_MODE(HD_VIDEOCAP_SEN_PAT_COLORBAR, color_bar_width);
		video_in_param.frc = HD_VIDEO_FRC_RATIO(30,1);
		video_in_param.dim.w = p_dim->w;
		video_in_param.dim.h = p_dim->h;

		if ((frame_num == 2) && (path == 0)) {
			video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_2;
		} else if ((frame_num == 3) && (path == 0)) {
			video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_3;
		} else {
			video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
		}

		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN, &video_in_param);
		if (ret != HD_OK) {
			return ret;
		}
	}

	{
		HD_VIDEOCAP_CROP video_crop_param = {0};

		video_crop_param.mode = HD_CROP_OFF;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);
	}

	{
		HD_VIDEOCAP_OUT video_out_param = {0};

		//without setting dim for no scaling, using original sensor out size
		if (frame_num == 2) {
			video_out_param.pxlfmt = SHDR2_CAP_OUT_FMT;
		} else if (frame_num == 3) {
			video_out_param.pxlfmt = SHDR3_CAP_OUT_FMT;
		} else {
			video_out_param.pxlfmt = CAP_OUT_FMT;
		}

		video_out_param.pxlfmt |= (HD_VIDEO_PIX_RGGB_R + strpix);

		video_out_param.depth = frame_num;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT, &video_out_param);
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
		if ((HD_CTRL_ID)_out_id == HD_VIDEOPROC_0_CTRL) {
			#if (EMU_AI_ENABLE)
			if (aiisp_flow_enable) {
				video_cfg_param.pipe = HD_VIDEOPROC_PIPE_BNR_RAWALL;
				video_cfg_param.isp_id = sen_vcap_id;
				video_cfg_param.ctrl_max.func = (VIDEOPROC_ALG_FUNC | HD_VIDEOPROC_FUNC_AIDED | HD_VIDEOPROC_FUNC_BNR | HD_VIDEOPROC_FUNC_BNR_STA);
			} else 
			#endif
			{
				video_cfg_param.pipe = HD_VIDEOPROC_PIPE_RAWALL;
				video_cfg_param.isp_id = sen_vcap_id;
				video_cfg_param.ctrl_max.func = VIDEOPROC_ALG_FUNC;
				if (is_nt98539a) {
					video_cfg_param.ctrl_max.func |= (HD_VIDEOPROC_FUNC_BNR | HD_VIDEOPROC_FUNC_BNR_STA);
				}
			}
		#if (VPE_FUN_ENABLE)
		} else if (((HD_CTRL_ID)_out_id == HD_VIDEOPROC_2_CTRL) && vpe_enable) {
			video_cfg_param.pipe = HD_VIDEOPROC_PIPE_VPE;
			video_cfg_param.isp_id = 0;
			video_cfg_param.ctrl_max.func = 0;
		#endif
		} else {
			printf("set_proc_cfg _out_id incorrect %d \n", _out_id);
		}

		if (((HD_CTRL_ID)_out_id == HD_VIDEOPROC_0_CTRL) && (frame_num > 1)) {
			video_cfg_param.ctrl_max.func |= HD_VIDEOPROC_FUNC_SHDR;
			#if (EMU_AI_ENABLE)
			if (aiisp_flow_enable) {
				video_cfg_param.ctrl_max.func |= HD_VIDEOPROC_FUNC_FUSION;
			}
			#endif
		}

		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;

		if ((HD_CTRL_ID)_out_id == HD_VIDEOPROC_0_CTRL)
		{
			if (frame_num == 2) {
				video_cfg_param.in_max.pxlfmt = SHDR2_CAP_OUT_FMT;
			} else if (frame_num == 3) {
				video_cfg_param.in_max.pxlfmt = SHDR3_CAP_OUT_FMT;
			} else {
				video_cfg_param.in_max.pxlfmt = CAP_OUT_FMT;
			}
		#if (VPE_FUN_ENABLE)
		} else if (((HD_CTRL_ID)_out_id == HD_VIDEOPROC_2_CTRL) && vpe_enable) {
			video_cfg_param.in_max.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
		#endif
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
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}

	if ((HD_CTRL_ID)_out_id == HD_VIDEOPROC_0_CTRL) {
	#if (EMU_AI_ENABLE)
	if (aiisp_flow_enable) {
		video_ctrl_param.func = (VIDEOPROC_ALG_FUNC | HD_VIDEOPROC_FUNC_AIDED | HD_VIDEOPROC_FUNC_BNR | HD_VIDEOPROC_FUNC_BNR_STA);
	} else 
	#endif
	{
		video_ctrl_param.func = VIDEOPROC_ALG_FUNC;
		if (is_nt98539a) {
			video_ctrl_param.func |= (HD_VIDEOPROC_FUNC_BNR | HD_VIDEOPROC_FUNC_BNR_STA);
		}
	}
	#if (VPE_FUN_ENABLE)
	} else if (((HD_CTRL_ID)_out_id == HD_VIDEOPROC_2_CTRL) && vpe_enable) {
		video_ctrl_param.func = 0;
	#endif
	}

	if ((HD_CTRL_ID)_out_id == HD_VIDEOPROC_0_CTRL) {
		#if (VPRC_PATH5_ENABLE)
		video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_0_OUT_4;
		#else
		video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_0_OUT_0;
		#endif

		if (frame_num > 1) {
			video_ctrl_param.func |= HD_VIDEOPROC_FUNC_SHDR;
			#if (EMU_AI_ENABLE)
			if (aiisp_flow_enable) {
				video_ctrl_param.func |= HD_VIDEOPROC_FUNC_FUSION;
			}
			#endif
		} else {
			video_ctrl_param.func &= ~HD_VIDEOPROC_FUNC_SHDR;
		}
	}

	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);

	*p_video_proc_ctrl = video_proc_ctrl;

	return ret;
}

static HD_RESULT set_proc_param(HD_PATH_ID video_proc_path, HD_DIM* p_dim, HD_VIDEO_PXLFMT output_pxlfmt)
{
	HD_RESULT ret = HD_OK;

	if (p_dim != NULL) { //if videoproc is already binding to dest module, not require to setting this!
		HD_VIDEOPROC_OUT video_out_param = {0};
		video_out_param.func = 0;
		video_out_param.dim.w = p_dim->w;
		video_out_param.dim.h = p_dim->h;
		video_out_param.pxlfmt = output_pxlfmt; //HD_VIDEO_PXLFMT_YUV420;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		video_out_param.frc = HD_VIDEO_FRC_RATIO(1,1);
		video_out_param.depth = 1;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	}
	#if 0
	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		video_path_param.out_func |= HD_VIDEOPROC_OUTFUNC_MD;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}
	#endif

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
	HD_H26XENC_ROW_RC rowrc_param = {0};
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

		//--- HD_VIDEOENC_PARAM_OUT_RATE_CONTROL ---
		#if 1
		rc_param.rc_mode                     = HD_RC_MODE_CBR;
		rc_param.cbr.bitrate                 = bitrate;
		rc_param.cbr.frame_rate_base         = 30;
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
		rc_param.fixqp.frame_rate_base       = FRAME_RATE;
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

static HD_RESULT open_module(VIDEO_EMU *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;

	ret = set_cap_cfg(&p_stream->cap_ctrl);
	if (ret != HD_OK) {
		printf("set cap-cfg fail = %d \n", ret);
		return HD_ERR_NG;
	}

	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, HD_VIDEOPROC_0_CTRL);
	if (ret != HD_OK) {
		printf("set proc-cfg 0 fail = %d \n", ret);
		return HD_ERR_NG;
	}

	#if (VPE_FUN_ENABLE)
	if (vpe_enable) {
		ret = set_proc_cfg(&p_stream->proc_ctrl2, p_proc_max_dim, HD_VIDEOPROC_2_CTRL);
		if (ret != HD_OK) {
			printf("set proc-cfg 2 fail = %d\n", ret);
			return HD_ERR_NG;
		}
	}
	#endif

	if ((ret = hd_videocap_open(HD_VIDEOCAP_IN(sen_vcap_id, 0), HD_VIDEOCAP_OUT(sen_vcap_id, 0), &p_stream->cap_path)) != HD_OK) {
		printf("open vcap module fail = %d \n", ret);
		return ret;
	}

	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &p_stream->proc_path)) != HD_OK) {
		printf("open vprc 0-0 module fail = %d \n", ret);
		return ret;
	}

	#if (VPE_FUN_ENABLE)
	if (vpe_enable) {
		if ((ret = hd_videoproc_open(HD_VIDEOPROC_2_IN_0, HD_VIDEOPROC_2_OUT_0, &p_stream->proc_path2)) != HD_OK) {
			printf("open vprc 2-0 module fail = %d \n", ret);
			return ret;
		}
	}
	#endif

	#if (SUB_VDO_ENABLE)
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_1, &p_stream->proc_path_sub)) != HD_OK) {
		printf("open vprc module 0-1 fail = %d \n", ret);
		return ret;
	}
	#endif

	#if (VPRC_PATH5_ENABLE)
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_4, &p_stream->proc_path_3dnr)) != HD_OK) {
		printf("open vprc module 0-4 fail = %d \n", ret);
		return ret;
	}
	#endif

	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_VIDEOENC_0_OUT_0, &p_stream->enc_path)) != HD_OK) {
		printf("open venc module 0 fail = %d \n", ret);
		return ret;
	}

	#if (SUB_VDO_ENABLE)
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_1, HD_VIDEOENC_0_OUT_1, &p_stream->enc_path_sub)) != HD_OK) {
		printf("open venc module 1 fail = %d \n", ret);
		return ret;
	}
	#endif

	return HD_OK;
}

static HD_RESULT close_module(VIDEO_EMU *p_stream)
{
	HD_RESULT ret;

	if ((ret = hd_videocap_close(p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_close(p_stream->proc_path)) != HD_OK)
		return ret;
	#if (VPE_FUN_ENABLE)
	if (vpe_enable) {
		if ((ret = hd_videoproc_close(p_stream->proc_path2)) != HD_OK)
			return ret;
	}
	#endif
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

static void init_share_memory(void)
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

static void exit_share_memory(void)
{
	if (g_shm) {
		shmdt(g_shm);
		shmctl(g_shmid, IPC_RMID, NULL);
	}
}

#if defined(_CHIP_NT98530_) || defined(_CHIP_NT98690_) || defined(_CHIP_NT98538_) || defined(_CHIP_NT98567_)
static HD_RESULT plug_pq_emu_mem_remain(UINTPTR * phy_addr, void * * virt_addr, UINT32 size, HD_COMMON_MEM_DDR_ID ddr)
#else
static HD_RESULT plug_pq_emu_mem_remain(UINT32 * phy_addr, void * * virt_addr, UINT32 size, HD_COMMON_MEM_DDR_ID ddr)
#endif
{
	if (ALIGN_CEIL_4(size) > ddr_remain[ddr].size) {
		return HD_ERR_NOMEM;
	} else {
		*phy_addr = ddr_remain[ddr].pa;
		*virt_addr = ddr_remain[ddr].va;
		ddr_remain[ddr].size -= ALIGN_CEIL_4(size);
		ddr_remain[ddr].pa += ALIGN_CEIL_4(size);
		#if defined(_CHIP_NT98530_) || defined(_CHIP_NT98690_) || defined(_CHIP_NT98538_) || defined(_CHIP_NT98567_)
		ddr_remain[ddr].va = (void *)((UINTPTR)ddr_remain[ddr].va + ALIGN_CEIL_4(size));
		#else
		ddr_remain[ddr].va = (void *)((UINT32)ddr_remain[ddr].va + ALIGN_CEIL_4(size));
		#endif
		return HD_OK;
	}
}

static HD_RESULT plug_pq_emu_mem_alloc(void)
{
	HD_RESULT ret = HD_OK;
	VENDOR_COMM_MAX_FREE_BLOCK max_free_block[MAX_DDR_NUM];
	HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;
	UINT32 i, j;

	raw_size = vdo_size_w * vdo_size_h * 3 / 2;  // pack12

	// check whether the memory space is sufficient
	pre_load_max_num = 0;
	if (load_raw_via_ddr) {
		for (i = 0; i < MAX_DDR_NUM; i++) {
			max_free_block[i].ddr = i;
			if (vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_MAX_FREE_BLOCK_SIZE, &max_free_block[i]) == HD_OK) {
				pre_load_max_num += max_free_block[i].size / raw_size;
			}
		}
	}

	if (run_num_max <= pre_load_max_num) {
		printf("load_raw_via_ddr enable, max num = %d, total remain size = %ld \n", pre_load_max_num, (UINT64)(max_free_block[0].size + max_free_block[1].size));
		is_load_raw_via_ddr = TRUE;
		pre_load_max_num = run_num_max;
	} else {
		printf("load_raw_via_ddr disable, max num = 1 \n");
		is_load_raw_via_ddr = FALSE;
		pre_load_max_num = 1;
	}

	if (is_load_raw_via_ddr) {
		for (i = 0; i < MAX_DDR_NUM; i++) {
			if (max_free_block[i].size) {
				if (hd_common_mem_alloc("RAW", &ddr_max_free[i].pa, (void **)&ddr_max_free[i].va, max_free_block[i].size, max_free_block[i].ddr) == HD_OK) {
					ddr_remain[i].pa = ddr_max_free[i].pa;
					ddr_remain[i].va = ddr_max_free[i].va;
					ddr_remain[i].size = ddr_max_free[i].size = max_free_block[i].size;
				}
			}
		}
	} else {
		if (hd_common_mem_alloc("RAW", &ddr_max_free[0].pa, (void **)&ddr_max_free[0].va, raw_size * frame_num, DDR_ID0) == HD_OK) {
			ddr_remain[0].pa = ddr_max_free[0].pa;
			ddr_remain[0].va = ddr_max_free[0].va;
			ddr_remain[0].size = ddr_max_free[0].size = raw_size * frame_num;
		}
	}

	for (i = file_start_num; i < file_start_num + pre_load_max_num; i++) {
		for (j = 0; j < frame_num; j++) {
			emu_buffer[i][j].buf_size = raw_size;
			ret = plug_pq_emu_mem_remain(&emu_buffer[i][j].buf_pa, (void **)&emu_buffer[i][j].buf_va, raw_size, ddr_id);
			if (ret != HD_OK) {
				printf("ddr1 full, idx = %d \n", i);
				ddr_id++;
				if (ddr_id < MAX_DDR_NUM) {
					ret = plug_pq_emu_mem_remain(&emu_buffer[i][j].buf_pa, (void **)&emu_buffer[i][j].buf_va, raw_size, ddr_id);
					if (ret != HD_OK) {
						printf("ddr2 full, idx = %d \n", i);
						break;
					}
				}
			}
		}
	}

	return ret;
}

static HD_RESULT plug_pq_emu_mem_free(void)
{
	HD_RESULT ret = HD_OK;
	UINT32 i, j;

	for (i = 0; i < MAX_DDR_NUM; i++) {
		if (ddr_max_free[i].pa != 0) {
			ret = hd_common_mem_free(ddr_max_free[i].pa, ddr_max_free[i].va);
			if (ret != HD_OK) {
				printf("DDR[%d] free fail \n", i);
			}
			ddr_max_free[i].pa = 0;
			ddr_max_free[i].va = 0;
			ddr_max_free[i].size = 0;
		}
	}

	for (i = 0; i < pre_load_max_num; i++) {
		for (j = 0; j < frame_num; j++) {
			if (emu_buffer[i][j].buf_va) {
				emu_buffer[i][j].buf_va = 0;
			}
		}
	}

	return ret;
}

HD_RESULT plug_pq_emu_open_file(UINT32 file_num)
{
	CHAR src_raw_path[FRAME_NUM_MAX][128];
	UINT32 i;
	HD_RESULT ret = HD_OK;

	if (frame_num == 1) {
		snprintf(src_raw_path[0], sizeof(src_raw_path[0]) - 1, RAW_FILE_LINEAR, load_raw_folder, linear_path, (int)vdo_size_w, (int)vdo_size_h, file_num);
	} else if (frame_num == 2) {
		snprintf(src_raw_path[0], sizeof(src_raw_path[0]) - 1, RAW_FILE_HDR_0, load_raw_folder, shdr_path, (int)vdo_size_w, (int)vdo_size_h, file_num);
		snprintf(src_raw_path[1], sizeof(src_raw_path[1]) - 1, RAW_FILE_HDR_1, load_raw_folder, shdr_path, (int)vdo_size_w, (int)vdo_size_h, file_num);
	} else {
		printf("frame number (%d) illegal \n", frame_num);
		return HD_ERR_NG;
	}

	for (i = 0; i < frame_num; i++) {
		fp[file_num][i] = fopen(src_raw_path[i], "rb");
		printf("Open %s \n", src_raw_path[i]);
		if (fp[file_num][i] == NULL) {
			printf("fail to open %s \n", src_raw_path[i]);
			return HD_ERR_NG;
		}
	}

	is_file_open[file_num] = TRUE;

	return ret;
}

HD_RESULT plug_pq_emu_read_file(UINT32 file_num)
{
	UINT32 i, num;
	HD_RESULT ret = HD_OK;

	if (is_load_raw_via_ddr) {
		num = file_num;
	} else {
		num = 0;
	}

	for (i = 0; i < frame_num; i++) {
		if (conti_run) {
			fseek(fp[file_num][i], 0L, SEEK_SET);
		}
		fread((void *)emu_buffer[num][i].buf_va, sizeof(CHAR), raw_size, fp[file_num][i]);
	}

	return ret;
}

HD_RESULT plug_pq_emu_close_file(UINT32 file_num)
{
	UINT32 i;
	HD_RESULT ret = HD_OK;

	for (i = 0; i < frame_num; i++) {
		fclose(fp[file_num][i]);
	}

	is_file_open[file_num] = FALSE;

	return ret;
}

#if (LOAD_2A_INFO)
static void plug_pq_emu_load_2a_info(void)
{
	FILE *fp;
	struct stat statbuf = {0};
	CHAR _2a_info_str[64];

	if (frame_num == 1) {
		snprintf(_2a_info_str, sizeof(_2a_info_str) - 1, _2A_INFO_FILE, load_raw_folder, linear_path);
	} else if (frame_num == 2) {
		snprintf(_2a_info_str, sizeof(_2a_info_str) - 1, _2A_INFO_FILE, load_raw_folder, shdr_path);
	} else {
		printf("frame number (%d) illegal \n", frame_num);
		return;
	}

	if ((fp = fopen(_2a_info_str, "rb")) == NULL) {
		printf("Create file %s fail \n", _2a_info_str);
		return;
	}

	if (stat(_2a_info_str, &statbuf) == -1) {
		printf("%s stat fail \n", _2a_info_str);
		return;
	}

	raw_2a_info_size = statbuf.st_size / sizeof(RAW_2A_INFO);
	p_raw_2a_info = malloc(statbuf.st_size);

	if (p_raw_2a_info == NULL) {
		printf("malloc p_raw_2a_info fail \n");
		return;
	}

	printf("%s size = %d, raw_2a_info size = %d \n", _2a_info_str, statbuf.st_size, raw_2a_info_size);
	fread((void *)p_raw_2a_info, sizeof(CHAR), statbuf.st_size, fp);

	#if 0
	{
		UINT32 i;
		for(i = 0; i < raw_2a_info_size; i++) {
			printf("i = %d, %d, %d, %d, %d, %d \n", i, p_raw_2a_info[i].expt, p_raw_2a_info[i].gain, p_raw_2a_info[i].cg_r, p_raw_2a_info[i].cg_g, p_raw_2a_info[i].cg_b);
		}
	}
	#endif

	fclose(fp);
}
#endif

static void *emu_thread(void *arg)
{
	VIDEO_EMU *p_stream = (VIDEO_EMU *)arg;
	HD_RESULT ret = HD_OK;
	HD_VIDEO_FRAME video_frame[FRAME_NUM_MAX] = {0};
	UINT32 i, num;
	#if (MEASURE_ENABLE)
	UINT64 time_diff;
	struct timeval start, end;
	#endif

	//--------- pull data test ---------
	while (emu_thread_start) {
		// NOTE: vcap pull
		for (i = 0; i < frame_num; i++) {
			#if (MEASURE_ENABLE)
			gettimeofday(&start, NULL);
			#endif

			ret = hd_videocap_pull_out_buf(p_stream->cap_path, &video_frame[i], -1);// -1 = blocking mode, 0 = non-blocking mode, >0 = blocking-timeout mode
			if (ret != HD_OK) {
				printf("vcap_pull fail (%d) \n", ret);
				continue;
			}

			//printf("vcap, num %d, frame %d \n", run_num_cnt, video_frame[i].count);

			#if (MEASURE_ENABLE)
			gettimeofday(&end, NULL);
			time_diff = 1000000 * (UINT64)(end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
			if (measure_vcap_en) {
				printf("VCAP time = %llu \n", time_diff);
			}
			#endif
		}

		#if (LOAD_2A_INFO)
		if (emu_flow_start && load_2a_info_en) {
			if (run_num_cnt < raw_2a_info_size) {
				ISPT_TOTAL_GAIN total_gain = {0};
				ISPT_C_GAIN c_gain = {0};

				total_gain.id = sen_vcap_id;
				vendor_isp_get_common(ISPT_ITEM_TOTAL_GAIN, &total_gain);
				total_gain.gain = p_raw_2a_info[run_num_cnt].gain;
				vendor_isp_set_common(ISPT_ITEM_TOTAL_GAIN, &total_gain);

				c_gain.id = sen_vcap_id;
				vendor_isp_set_common(ISPT_ITEM_C_GAIN, &c_gain);
				c_gain.gain[0] = p_raw_2a_info[run_num_cnt].cg_r;
				c_gain.gain[1] = p_raw_2a_info[run_num_cnt].cg_g;
				c_gain.gain[2] = p_raw_2a_info[run_num_cnt].cg_b;
				vendor_isp_set_common(ISPT_ITEM_C_GAIN, &c_gain);
			} else {
				printf("run_num_cnt (%d) > raw_2a_info_size (%d) \n", run_num_cnt, raw_2a_info_size);
			}
		}
		#endif

		#if (BYPASS_1ST_IQ_ENABLE)
		if (emu_flow_start) {
			IQT_3DNR_PARAM nr_3d = {0};
			IQT_NR_PARAM nr_2d = {0};
			IQT_WDR_PARAM wdr = {0};
			#if (GARY_FOR_1ST_FRM_ENABLE)
			IQT_COLOR_PARAM color = {0};
			#endif

			// 1: for 1st loop; 2: for 2nd loop.
			// emu_bypass_1st_frm only for 1st loop.
			if (/*1*/((run_num_cnt == 0) && !conti_run_1st_loop && emu_bypass_1st_frm) || /*2*/((run_num_cnt == run_num_max - 1) && conti_run && bypass_1st_3dnr_en)) {
				//printf("disable 3dnr \n");
				nr_3d.id = sen_vcap_id;
				vendor_isp_get_iq(IQT_ITEM_3DNR_PARAM, &nr_3d);
				nr_3d._3dnr.enable = FALSE;
				vendor_isp_set_iq(IQT_ITEM_3DNR_PARAM, &nr_3d);
			} else if (run_num_cnt == 0) {
				//printf("enable 3dnr \n");
				nr_3d.id = sen_vcap_id;
				vendor_isp_get_iq(IQT_ITEM_3DNR_PARAM, &nr_3d);
				nr_3d._3dnr.enable = TRUE;
				vendor_isp_set_iq(IQT_ITEM_3DNR_PARAM, &nr_3d);
			}
			if (/*1*/((run_num_cnt == 0) && !conti_run_1st_loop && emu_bypass_1st_frm) || /*2*/((run_num_cnt == run_num_max - 1) && conti_run && bypass_1st_lce_en)) {
				//printf("disable lca \n");
				nr_2d.id = sen_vcap_id;
				vendor_isp_get_iq(IQT_ITEM_NR_PARAM, &nr_2d);
				nr_2d.nr.lca_enable = FALSE;
				vendor_isp_set_iq(IQT_ITEM_NR_PARAM, &nr_2d);
			} else if (run_num_cnt == 0) {
				//printf("enable lca \n");
				nr_2d.id = sen_vcap_id;
				vendor_isp_get_iq(IQT_ITEM_NR_PARAM, &nr_2d);
				nr_2d.nr.lca_enable = TRUE;
				vendor_isp_set_iq(IQT_ITEM_NR_PARAM, &nr_2d);
			}
			if (/*1*/((run_num_cnt == 0) && !conti_run_1st_loop && emu_bypass_1st_frm) || /*2*/((run_num_cnt == run_num_max - 1) && conti_run && bypass_1st_wdr_en)) {
				//printf("disable wdr \n");
				wdr.id = sen_vcap_id;
				vendor_isp_get_iq(IQT_ITEM_WDR_PARAM, &wdr);
				wdr.wdr.enable = FALSE;
				vendor_isp_set_iq(IQT_ITEM_WDR_PARAM, &wdr);
			} else if (run_num_cnt == 0) {
				//printf("enable wdr \n");
				wdr.id = sen_vcap_id;
				vendor_isp_get_iq(IQT_ITEM_WDR_PARAM, &wdr);
				wdr.wdr.enable = TRUE;
				vendor_isp_set_iq(IQT_ITEM_WDR_PARAM, &wdr);
			}
			#if (GARY_FOR_1ST_FRM_ENABLE)
			if (/*1*/((run_num_cnt == 0) && !conti_run_1st_loop && emu_bypass_1st_frm) || /*2*/((run_num_cnt == run_num_max - 1) && conti_run)) {
				color.id = sen_vcap_id;
				vendor_isp_get_iq(IQT_ITEM_COLOR_PARAM, &color);
				color.color.mode = IQ_OP_TYPE_MANUAL;
				color.color.manual_param.c_con = 0;
				vendor_isp_set_iq(IQT_ITEM_COLOR_PARAM, &color);
			} else if (run_num_cnt == 0) {
				color.id = sen_vcap_id;
				vendor_isp_get_iq(IQT_ITEM_COLOR_PARAM, &color);
				color.color.mode = IQ_OP_TYPE_AUTO;
				vendor_isp_set_iq(IQT_ITEM_COLOR_PARAM, &color);
			}
			#endif
		}
		#endif

		if (emu_flow_start && (!(run_num_cnt == 0) || conti_run_1st_loop || !emu_bypass_1st_frm)) {
			if ((!conti_run_1st_loop || !conti_run) && !is_file_open[file_start_num + run_num_cnt]) {
				if (plug_pq_emu_open_file(file_start_num + run_num_cnt) != HD_OK) {
					emu_flow_start = FALSE;
					plug_pq_emu_mem_free();
					#if (LOAD_2A_INFO)
					if (p_raw_2a_info) {
						free(p_raw_2a_info);
						p_raw_2a_info = NULL;
					}
					#endif
					goto read_raw_exit;
				}
			}

			#if (MEASURE_ENABLE)
			gettimeofday(&start, NULL);
			#endif

			if (!is_load_raw_via_ddr || (!conti_run_1st_loop || !conti_run)) {
				plug_pq_emu_read_file(file_start_num + run_num_cnt);
				if (!is_load_raw_via_ddr) {
					usleep(frame_delay * 1000);
				}
			} else {
				usleep(frame_delay * 1000);
			}

			#if (MEASURE_ENABLE)
			gettimeofday(&end, NULL);
			time_diff = 1000000 * (UINT64)(end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
			if (maesure_read_en) {
				printf("read time = %llu \n", time_diff);
			}
			#endif

			if (is_load_raw_via_ddr) {
				num = file_start_num + run_num_cnt;
			} else {
				num = 0;
			}

			for (i = 0; i < frame_num; i++) {
				hd_common_mem_flush_cache(emu_buffer[num][i].buf_va, emu_buffer[num][i].buf_size);
				#if defined(_CHIP_NT98530_) || defined(_CHIP_NT98690_) || defined(_CHIP_NT98538_) || defined(_CHIP_NT98567_)
				video_frame[i].phy_addr[0] = (UINTPTR)emu_buffer[num][i].buf_pa;
				#else
				video_frame[i].phy_addr[0] = (UINT32)emu_buffer[num][i].buf_pa;
				#endif
			}
		}

read_raw_exit:
		// NOTE: vprc push
		for (i = 0; i < frame_num; i++) {
			ret = hd_videoproc_push_in_buf(p_stream->proc_path, &video_frame[i], NULL, 0);
		}

		// NOTE: vcap release
		for (i = 0; i < frame_num; i++) {
			ret = hd_videocap_release_out_buf(p_stream->cap_path, &video_frame[i]);
			if (ret != HD_OK) {
				printf("cap_release fail (%d) \n\n", i);
			}
		}

		// NOTE: save yuv here
		#if (SAVE_YUV_ENABLE)
		if (save_yuv_en && save_yuv_start && (!(run_num_cnt == 0) || conti_run_1st_loop || !emu_bypass_1st_frm)) {
			uintptr_t phy_addr_yuv, vir_addr_yuv;
			CHAR save_yuv_path[128];
			static FILE *yuv_fp = NULL;
			UINT8 *ptr = NULL;
			UINT32 save_len, yuv_size;
			HD_VIDEO_FRAME vprc_out_frame = {0};

			// NOTE: vprc pull
			#if (VPE_FUN_ENABLE)
			if (vpe_enable) {
				ret = hd_videoproc_pull_out_buf(p_stream->proc_path2, &vprc_out_frame, -1);
			} else
			#endif
			{
				ret = hd_videoproc_pull_out_buf(p_stream->proc_path, &vprc_out_frame, -1);
			}
			if (ret != HD_OK) {
				if (ret != HD_ERR_UNDERRUN) {
					printf("vprc_pull fail (%d) \n", ret);
				}
				goto save_yuv_exit;
			}

			if (yuv_bypass_1st_frm) {
				yuv_bypass_1st_frm = FALSE;
				goto save_yuv_exit;
			}

			phy_addr_yuv = hd_common_mem_blk2pa(vprc_out_frame.blk);
			if (phy_addr_yuv == 0) {
				printf("blk2pa fail, blk = 0x%x \n", vprc_out_frame.blk);
				goto save_yuv_exit;
			}
			yuv_size = DBGINFO_BUFSIZE() + VDO_YUV_BUFSIZE(vprc_out_frame.pw[0],vprc_out_frame.ph[0], HD_VIDEO_PXLFMT_YUV420);
			vir_addr_yuv = (ULONG)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_yuv, yuv_size);
			if (vir_addr_yuv == 0) {
				printf("save yuv, mmap fail \n");
				goto save_yuv_exit;
			}

			#if (SAVE_YUV_PACK)
			if (save_yuv_cnt == 0) {
			#endif
				snprintf(save_yuv_path, sizeof(save_yuv_path) - 1, YUV_FILE, load_raw_folder, (int)(file_start_num + save_yuv_cnt), (int)vprc_out_frame.loff[0], (int)vprc_out_frame.ph[0]);
				printf("save %s \n", &save_yuv_path[8]);
				yuv_fp = fopen(save_yuv_path, "wb");
				if (yuv_fp == NULL) {
					printf("fail to open %s \n", save_yuv_path);
					goto save_yuv_exit;
				}
			#if (SAVE_YUV_PACK)
			}
			#endif

			//save Y plane
			ptr = (UINT8 *)(vir_addr_yuv + (vprc_out_frame.phy_addr[0] - phy_addr_yuv));
			save_len = vprc_out_frame.loff[0] * vprc_out_frame.ph[0];
			if (yuv_fp) fwrite(ptr, 1, save_len, yuv_fp);
			if (yuv_fp) fflush(yuv_fp);

			//save UV plane
			ptr = (UINT8 *)(vir_addr_yuv + (vprc_out_frame.phy_addr[1] - phy_addr_yuv));
			save_len = vprc_out_frame.loff[1] * vprc_out_frame.ph[1];
			if (yuv_fp) fwrite(ptr, 1, save_len, yuv_fp);
			if (yuv_fp) fflush(yuv_fp);

			#if (SAVE_YUV_PACK)
			if (save_yuv_cnt == (run_num_max - 1)) {
			#endif
				fclose(yuv_fp);
			#if (SAVE_YUV_PACK)
			}
			#endif

			// mummap for frame buffer
			ret = hd_common_mem_munmap((void *)vir_addr_yuv, yuv_size);
			if (ret != HD_OK) {
				printf("save yuv, mnumap fail \n");
				goto save_yuv_exit;
			}

			save_yuv_cnt++;
			if (save_yuv_start && (save_yuv_cnt == run_num_max)) {
				save_yuv_start = FALSE;
				system("sync");
			}

save_yuv_exit:
			// NOTE: vprc release
			#if (VPE_FUN_ENABLE)
			if (vpe_enable) {
				ret = hd_videoproc_release_out_buf(p_stream->proc_path2, &vprc_out_frame);
			} else 
			#endif
			{
				ret = hd_videoproc_release_out_buf(p_stream->proc_path, &vprc_out_frame);
			}
			if (ret != HD_OK) {
				printf("vprc_release fail (%d) \n", ret);
			}
		}
		#endif

		if (emu_flow_start) {
			if (emu_bypass_1st_frm) {
				emu_bypass_1st_frm = FALSE;
				continue;
			}
		}

		if (emu_flow_start) {
			run_num_cnt++;
			if (run_num_cnt == run_num_max) {
				if (!conti_run || emu_flow_stop) {
					for (i = file_start_num; i < file_start_num + run_num_max; i++) {
						plug_pq_emu_close_file(i);
					}
					emu_flow_start = FALSE;
					plug_pq_emu_mem_free();
					#if (LOAD_2A_INFO)
					if (p_raw_2a_info) {
						free(p_raw_2a_info);
						p_raw_2a_info = NULL;
					}
					#endif
					printf("\n emu_flow_stop \n");
				} else if (conti_run) {
					run_num_cnt = 0;
					conti_run_1st_loop = TRUE;
				}
			}
		}
	}
	return 0;
}

HD_RESULT plug_pq_emu_init(void)
{
	HD_RESULT ret = HD_OK;
	HD_DIM main_dim;
	#if (SUB_VDO_ENABLE)
	HD_DIM sub_dim;
	#endif
	UINT32 i, j;

	#if (EMU_AI_ENABLE)
	if (aiisp_flow_enable) {
		system("insmod /lib/modules/5.10.168/hdal/kflow_ai_isp/kflow_ai_isp.ko");
	}
	#endif

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

	for (i = 0; i < RUN_NUM_MAX; i++) {
		for (j = 0; j < FRAME_NUM_MAX; j++) {
			fp[i][j] = NULL;
		}
	}

	init_share_memory();

	if (vendor_isp_init() == HD_ERR_NG) {
		printf("vendor_isp_init failed \n");
	}

	{
		AET_CFG_INFO cfg_info = {0};
		ISPT_C_GAIN c_gain = {0};
		ISPT_TOTAL_GAIN total_gain = {0};

		cfg_info.id = sen_vcap_id;

		snprintf(cfg_info.path, sizeof(cfg_info.path) - 1, "/mnt/app/isp/%s.cfg", isp_cfg_name);
		printf("sensor 1 load %s \n", cfg_info.path);

		vendor_isp_set_ae(AET_ITEM_RLD_CONFIG, &cfg_info);
		vendor_isp_set_awb(AWBT_ITEM_RLD_CONFIG, &cfg_info);
		vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);

		c_gain.id = sen_vcap_id;
		c_gain.gain[0] = init_c_gain[0];
		c_gain.gain[1] = init_c_gain[1];
		c_gain.gain[2] = init_c_gain[2];
		vendor_isp_set_common(ISPT_ITEM_C_GAIN, &c_gain);

		total_gain.id = sen_vcap_id;
		total_gain.gain = init_total_gain;
		vendor_isp_set_common(ISPT_ITEM_TOTAL_GAIN, &total_gain);
	}

	// init hdal
	ret = hd_common_init(0);
	if (ret != HD_OK) {
		printf("common fail = %d \n", ret);
		return ret;
	}

	system("echo w emu_en 1 > /proc/hdal/vendor/isp/cmd");
	printf("echo w emu_en 1 > /proc/hdal/vendor/isp/cmd \n");

	#if (EMU_AI_ENABLE)
	if (aiisp_flow_enable) {
		hd_common_sysconfig(0, (1<<16), 0, VENDOR_AI_CFG); //enable AI engine
	}
	#endif

	// init memory
	ret = mem_init();
	if (ret != HD_OK) {
		printf("mem fail = %d \n", ret);
		return ret;
	}

	// init all modules
	ret = init_module();
	if (ret != HD_OK) {
		printf("init module fail = %d \n", ret);
		return ret;
	}

	printf("sen_vcap_id = %d  \n", sen_vcap_id);

	emu_stream.proc_max_dim.w = vdo_size_w;
	emu_stream.proc_max_dim.h = vdo_size_h;
	ret = open_module(&emu_stream, &emu_stream.proc_max_dim);
	if (ret != HD_OK) {
		return ret;
	}

	#if (EMU_AI_ENABLE)
	if (aiisp_flow_enable) {
		static uintptr_t ai_cb;
		VENDOR_VIDEOPROC_ISP_AI_EFFECT isp_ai_effect = {0};
		UINT32 proc_id  = aiisp_get_proc_id(0);
		UINT32 proc_id2 = aiisp_get_proc_id(1);

		aiisp_init();

		aiisp_open(VIDEO_AIISP_0);

		aiisp_start(VIDEO_AIISP_0);

		aiisp_get_ai_cb(&ai_cb);

		ret = vendor_videoproc_set(emu_stream.proc_ctrl, VENDOR_VIDEOPROC_PARAM_AI_CB, &ai_cb);
		if (ret != HD_OK) {
			printf("set ai cb fail = %d \n", ret);
			return ret;
		}

		isp_ai_effect.enable = 1;
		isp_ai_effect.path_id = 0;
		isp_ai_effect.proc_id[NORMAL_ISO_EFFECT] = proc_id;
		isp_ai_effect.proc_id[LOW_ISO_EFFECT] = proc_id2;
		ret = vendor_videoproc_set(emu_stream.proc_ctrl, VENDOR_VIDEOPROC_PARAM_ISP_AI_EFFECT, &isp_ai_effect);
		if (ret) {
			printf("set ISP_AI_EFFECT fail = %d \n", ret);
		}
	}
	#endif

	// get videocap capability
	ret = get_cap_caps(emu_stream.cap_ctrl, &emu_stream.cap_syscaps);
	if (ret != HD_OK) {
		printf("get cap-caps fail = %d \n", ret);
		return ret;
	}

	// set videocap parameter
	emu_stream.cap_dim.w = vdo_size_w;
	emu_stream.cap_dim.h = vdo_size_h;

	ret = set_cap_param(emu_stream.cap_path, &emu_stream.cap_dim, 0);
	if (ret != HD_OK) {
		printf("set cap fail = %d \n", ret);
		return ret;
	}

	// assign parameter by program options
	main_dim.w = vdo_size_w;
	main_dim.h = vdo_size_h;

	// set videoproc parameter
	ret = set_proc_param(emu_stream.proc_path, &main_dim, HD_VIDEO_PXLFMT_YUV420);
	if (ret != HD_OK) {
		printf("set main proc fail = %d \n", ret);
		return ret;
	}

	#if (SUB_VDO_ENABLE)
	sub_dim.w = sub_vdo_size_w;
	sub_dim.h = sub_vdo_size_h;

	ret = set_proc_param(emu_stream.proc_path_sub, &sub_dim, HD_VIDEO_PXLFMT_YUV420);
	if (ret != HD_OK) {
		printf("set sub proc fail = %d \n", ret);
		return ret;
	}
	#endif

	#if (VPRC_PATH5_ENABLE)
	ret = set_proc_param(emu_stream.proc_path_3dnr, &main_dim, HD_VIDEO_PXLFMT_YUV420);
	if (ret != HD_OK) {
		printf("set 3dnr proc fail = %d \n", ret);
		return ret;
	}
	#endif

	#if (VPE_FUN_ENABLE)
	if (vpe_enable) {
		ret = set_proc_param(emu_stream.proc_path2, &main_dim, HD_VIDEO_PXLFMT_YUV420);
		if (ret != HD_OK) {
			printf("set vpe proc fail = %d \n", ret);
			return ret;
		}
	}
	#endif

	emu_stream.enc_max_dim.w = vdo_size_w;
	emu_stream.enc_max_dim.h = vdo_size_h;
	ret = set_enc_cfg(emu_stream.enc_path, &emu_stream.enc_max_dim, enc_bitrate * 1024 * 1024, sen_vcap_id);
	if (ret != HD_OK) {
		printf("set main enc-cfg fail = %d \n", ret);
		return ret;
	}

	emu_stream.enc_dim.w = vdo_size_w;
	emu_stream.enc_dim.h = vdo_size_h;
	ret = set_enc_param(emu_stream.enc_path, &emu_stream.enc_dim, enc_type, enc_bitrate * 1024 * 1024);
	if (ret != HD_OK) {
		printf("set main enc fail = %d \n", ret);
		return ret;
	}

	#if (SUB_VDO_ENABLE)
	emu_stream.enc_max_dim.w = sub_vdo_size_w;
	emu_stream.enc_max_dim.h = sub_vdo_size_h;
	ret = set_enc_cfg(emu_stream.enc_path_sub, &emu_stream.enc_max_dim, enc_bitrate * 1024 * 1024, sen_vcap_id);
	if (ret != HD_OK) {
		printf("set sub enc-cfg fail = %d \n", ret);
		return ret;
	}

	emu_stream.enc_dim.w = sub_vdo_size_w;
	emu_stream.enc_dim.h = sub_vdo_size_h;
	ret = set_enc_param(emu_stream.enc_path_sub, &emu_stream.enc_dim, enc_type, enc_bitrate * 1024 * 1024);
	if (ret != HD_OK) {
		printf("set sub enc fail = %d \n", ret);
		return ret;
	}
	#endif

	// bind video_record modules
	#if (VPE_FUN_ENABLE)
	if (vpe_enable) {
		hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOPROC_2_IN_0);
		hd_videoproc_bind(HD_VIDEOPROC_2_OUT_0, HD_VIDEOENC_0_IN_0);
	} else 
	#endif
	{
		hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOENC_0_IN_0);
	}

	#if (SUB_VDO_ENABLE)
	hd_videoproc_bind(HD_VIDEOPROC_0_OUT_1, HD_VIDEOENC_0_IN_1);
	#endif

	hd_videocap_start(emu_stream.cap_path);

	hd_videoproc_start(emu_stream.proc_path);

	#if (VPE_FUN_ENABLE)
	if (vpe_enable) {
		hd_videoproc_start(emu_stream.proc_path2);
	}
	#endif

	#if (SUB_VDO_ENABLE)
	hd_videoproc_start(emu_stream.proc_path_sub);
	#endif

	#if (VPRC_PATH5_ENABLE)
	hd_videoproc_start(emu_stream.proc_path_3dnr);
	#endif

	hd_videoenc_start(emu_stream.enc_path);

	#if (SUB_VDO_ENABLE)
	hd_videoenc_start(emu_stream.enc_path_sub);
	#endif

	emu_thread_start = TRUE;
	sleep(1);
	ret = pthread_create(&emu_thread_id, NULL, emu_thread, (void *)&emu_stream);
	if (ret < 0) {
		printf("create emu_thread fail \n");
	} else {
		printf("create emu_thread success \n");
	}

	system("nvtrtspd_ipc &");
	printf("nvtrtspd_ipc \n");

	return ret;
}

HD_RESULT plug_pq_emu_uninit(void)
{
	HD_RESULT ret = HD_OK;

	emu_thread_start = FALSE;
	sleep(1);
	pthread_join(emu_thread_id, NULL);

	// stop video_record modules
	hd_videocap_stop(emu_stream.cap_path);
	hd_videoproc_stop(emu_stream.proc_path);

	#if (VPE_FUN_ENABLE)
	if (vpe_enable) {
		hd_videoproc_stop(emu_stream.proc_path2);
	}
	#endif
	#if (SUB_VDO_ENABLE)
	hd_videoenc_stop(emu_stream.proc_path_sub);
	#endif
	#if (VPRC_PATH5_ENABLE)
	hd_videoenc_stop(emu_stream.proc_path_3dnr);
	#endif
	hd_videoenc_stop(emu_stream.enc_path);
	#if (SUB_VDO_ENABLE)
	hd_videoenc_stop(emu_stream.enc_path_sub);
	#endif

	// unbind video_record modules
	hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_0);
	#if (VPE_FUN_ENABLE)
	if (vpe_enable) {
		hd_videoproc_unbind(HD_VIDEOPROC_2_OUT_0);
	}
	#endif
	#if (SUB_VDO_ENABLE)
	hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_1);
	#endif

	p_pd_shm = (PD_SHM_INFO *)g_shm;
	p_pd_shm->exit = 1;
	usleep(300000);

	#if EMU_AI_ENABLE
	if (aiisp_flow_enable) {
		aiisp_stop(VIDEO_AIISP_0);

		aiisp_close(VIDEO_AIISP_0);

		aiisp_uninit();
	}
	#endif

	// close video_record modules
	ret = close_module(&emu_stream);
	if (ret != HD_OK) {
		printf("close fail = %d \n", ret);
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

	system("echo w emu_en 0 > /proc/hdal/vendor/isp/cmd");
	printf("echo w emu_en 0 > /proc/hdal/vendor/isp/cmd \n");

	vendor_isp_uninit();

	exit_share_memory();

	#if (EMU_AI_ENABLE)
	if (aiisp_flow_enable) {
		system("rmmod kflow_ai_isp.ko");
	}
	#endif

	return ret;
}

static INT32 get_choose_int(void)
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

MAIN(argc, argv)
{
	INT32 option;
	UINT32 trig = 1;
	#if (EMU_AI_ENABLE)
	CHAR file_name[64];
	IQT_AIISP_PARAM aiisp = {0};
	#endif

	while (trig) {
		printf("----------------------------------------\n");
		#if defined(_CHIP_NT9852x_)
		printf("   NT9852x Emulator  \n");
		#elif defined(_CHIP_NT9856x_)
		printf("   NT9856x Emulator  \n");
		#elif defined(_CHIP_NT98530_)
		printf("   NT98530 Emulator  \n");
		#elif defined(_CHIP_NT98690_)
		printf("   NT98690 Emulator  \n");
		#elif  defined(_CHIP_NT98538_)
		printf("   NT98538 Emulator  \n");
		#elif  defined(_CHIP_NT98567_)
		printf("   NT98567 Emulator  \n");
		#endif
		printf("----------------------------------------\n");
		printf("   1.  Init (%s) \n", is_init_emu ? "true" : "false");
		printf("   2.  Uninit \n");
		printf("   3.  Start (%s) \n", emu_flow_start ? "true" : "false");
		printf("   4.  Stop \n");
		printf("----------------------------------------\n");
		printf("  10.  Set run_num_max (%d) \n", run_num_max);
		printf("  11.  Set file start num (%d) \n", file_start_num);
		printf("  12.  Set conti_run (%s) \n", conti_run ? "enable" : "disable");
		printf("  13.  Set save_yuv_en (%s) \n", save_yuv_en ? "enable" : "disable");
		printf("  14.  Set measure_vcap_en (%s) \n", measure_vcap_en ? "enable" : "disable");
		printf("  15.  Set maesure_read_en (%s) \n", maesure_read_en ? "enable" : "disable");
		printf("  16.  Set load_raw_via_ddr (%s) \n", load_raw_via_ddr ? "auto" : "disable");
		printf("  17.  Set frame_delay (%d ms) \n", frame_delay);
		#if (LOAD_2A_INFO)
		printf("  18.  Set load_2a_info_en (%s) \n", load_2a_info_en ? "enable" : "disable");
		#endif
		printf("----------------------------------------\n");
		printf("  20.  Set sen_vcap_id (%d) \n", sen_vcap_id);
		printf("  21.  Set resolution (%d x %d) \n", vdo_size_w, vdo_size_h);
		#if (SUB_VDO_ENABLE)
		printf("  22.  Set vprc sub_vdo resolution (%d x %d) \n", sub_vdo_size_w, sub_vdo_size_h);
		#endif
		printf("  23.  Set frame_num (%d) \n", frame_num);
		printf("  24.  Set c_gain (%d %d %d) \n", init_c_gain[0], init_c_gain[1], init_c_gain[2]);
		printf("  25.  Set total_gain (%d) \n", init_total_gain);
		#if (VPE_FUN_ENABLE)
		printf("  26.  Set vpe_en (%s) \n", vpe_enable ? "enable" : "disable");
		#endif
		printf("  27.  Set strpix (%d) \n", strpix);
		printf("  30.  Set enc_type (%s) \n", (enc_type == 0) ? "H.265" : "H.264");
		printf("  31.  Set enc_bitrate (%d Mbps) \n", enc_bitrate);
		printf("----------------------------------------\n");
		printf("  40.  Set linear dir. path (%s) \n", linear_path);
		printf("  41.  Set shdr dir. path (%s) \n", shdr_path);
		printf("  42.  Set isp cfg name (%s) \n", isp_cfg_name);
		printf("  43.  Set load_raw_folder (%s) \n", load_raw_folder);
		printf("----------------------------------------\n");
		printf("  50.  Set bypass_1st_3dnr_en (%s) \n", bypass_1st_3dnr_en ? "enable" : "disable");
		printf("  51.  Set bypass_1st_lce_en (%s) \n", bypass_1st_lce_en ? "enable" : "disable");
		printf("  52.  Set bypass_1st_wdr_en (%s) \n", bypass_1st_wdr_en ? "enable" : "disable");
		printf("----------------------------------------\n");
		#if (EMU_AI_ENABLE)
		printf("  71.  Set ai flow eable (%s) \n", aiisp_flow_enable ? "enable" : "disable");
		aiisp_get_model_name(0, file_name);
		printf("  73.  Set ai model name 1 (%s) \n", file_name);
		aiisp_get_model_name(1, file_name);
		printf("  74.  Set ai model name 2 (%s) \n", file_name);
		printf("  75.  Set aiisp manual param (enable %d, effect %d, param %d %d) \n", aiisp.aiisp.manual_param.enable, aiisp.aiisp.manual_param.effect, aiisp.aiisp.manual_param.param[0], aiisp.aiisp.manual_param.param[1]);
		printf("----------------------------------------\n");
		#endif
		printf("   0.  Quit\n");
		printf("----------------------------------------\n");
		printf(">> ");
		option = get_choose_int();

		switch (option) {
		case 1:
			if (is_init_emu) {
				printf("emu has been initialized \n");
				break;
			}

			plug_pq_emu_init();

			is_init_emu = TRUE;
			break;

		case 2:
			if (!is_init_emu) {
				printf("emu not yet initialized \n");
				break;
			}

			plug_pq_emu_uninit();

			is_init_emu = FALSE;
		break;

		case 3:
			if (!is_init_emu) {
				printf("emu not yet initialized \n");
				break;
			}
			if (emu_flow_start) {
				printf("emu has already started \n");
				break;
			}

			#if (LOAD_2A_INFO)
			if (load_2a_info_en) {
				plug_pq_emu_load_2a_info();
			}
			#endif

			plug_pq_emu_mem_alloc();

			conti_run_1st_loop = FALSE;
			yuv_bypass_1st_frm = TRUE;
			emu_bypass_1st_frm = TRUE;
			save_yuv_start = TRUE;
			run_num_cnt = 0;
			save_yuv_cnt = 0;
			emu_flow_start = TRUE;
			emu_flow_stop = FALSE;
		break;

		case 4:
			emu_flow_stop = TRUE;
		break;

		case 10:
			if (is_init_emu) {
				printf("emu has been started \n");
				break;
			}

			printf("Current %d \n", run_num_max);
			printf("Set run_num_max>> \n");
			run_num_max = (UINT32)get_choose_int();
			if (run_num_max < 2) {
				run_num_max = 2;
				printf("Force set to %d \n", run_num_max);
			} else {
				printf("Set to %d \n", run_num_max);
			}
		break;

		case 11:
			if (is_init_emu) {
				printf("emu has been started \n");
				break;
			}

			printf("Current %d \n", file_start_num);
			printf("Set file_start_num>> \n");
			file_start_num = (UINT32)get_choose_int();
			printf("Set to %d \n", file_start_num);
		break;

		case 12:
			printf("Current %s \n", conti_run ? "enable" : "disable");
			printf("Set conti_run(0: disable; 1: enable)>> \n");
			conti_run = (UINT32)get_choose_int();
			printf("Set to %s \n", conti_run ? "enable" : "disable");
		break;

		case 13:
			printf("Current %s \n", save_yuv_en ? "enable" : "disable");
			printf("Set save_yuv_en(0: disable; 1: enable)>> \n");
			save_yuv_en = (UINT32)get_choose_int();
			printf("Set to %s \n", save_yuv_en ? "enable" : "disable");
		break;

		case 14:
			printf("Current %s \n", measure_vcap_en ? "enable" : "disable");
			printf("Set measure_vcap_en(0: disable; 1: enable)>> \n");
			measure_vcap_en = (UINT32)get_choose_int();
			printf("Set to %s \n", measure_vcap_en ? "enable" : "disable");
		break;

		case 15:
			printf("Current %s \n", maesure_read_en ? "enable" : "disable");
			printf("Set maesure_read_en(0: disable; 1: enable)>> \n");
			maesure_read_en = (UINT32)get_choose_int();
			printf("Set to %s \n", maesure_read_en ? "enable" : "disable");
		break;

		case 16:
			if (is_init_emu) {
				printf("emu has been initialized \n");
				break;
			}

			printf("Current %s \n", load_raw_via_ddr ? "auto" : "disable");
			printf("Set load_raw_via_ddr(0: disable; 1: auto)>> \n");
			load_raw_via_ddr = (UINT32)get_choose_int();
			printf("Set to %s \n", load_raw_via_ddr ? "auto" : "disable");
		break;

		case 17:
			printf("Current %d ms \n", frame_delay);
			printf("Set frame_delay>> \n");
			frame_delay = (UINT32)get_choose_int();
			printf("Set to %d ms \n", frame_delay);
		break;

		#if (LOAD_2A_INFO)
		case 18:
			printf("Current %s \n", load_2a_info_en ? "enable" : "disable");
			printf("Set load_2a_info_en(0: disable; 1: enable)>> \n");
			load_2a_info_en = (UINT32)get_choose_int();
			printf("Set to %s \n", load_2a_info_en ? "enable" : "disable");
		break;
		#endif

		case 20:
			if (is_init_emu) {
				printf("emu has been initialized \n");
				break;
			}

			printf("Current %d \n", sen_vcap_id);
			printf("Set sen_vcap_id>> \n");
			sen_vcap_id = (UINT32)get_choose_int();
			printf("Set to %d \n", sen_vcap_id);
		break;

		case 21:
			if (is_init_emu) {
				printf("emu has been initialized \n");
				break;
			}
			
			printf("Current %d x %d \n", vdo_size_w, vdo_size_h);
			printf("Set vdo_size_w>> \n");
			vdo_size_w = (UINT32)get_choose_int();
			printf("Set vdo_size_h>> \n");
			vdo_size_h = (UINT32)get_choose_int();
			printf("Set to %d x %d \n", vdo_size_w, vdo_size_h);
		break;

		#if (SUB_VDO_ENABLE)
		case 22:
			if (is_init_emu) {
				printf("emu has been initialized \n");
				break;
			}
			
			printf("Current %d x %d \n", sub_vdo_size_w, sub_vdo_size_h);
			printf("Set vprc_size_w>> \n");
			sub_vdo_size_w = (UINT32)get_choose_int();
			printf("Set sub_vdo resolution>> \n");
			sub_vdo_size_h = (UINT32)get_choose_int();
			printf("Set to %d x %d \n", sub_vdo_size_w, sub_vdo_size_h);
		break;
		#endif

		case 23:
			if (is_init_emu) {
				printf("emu has been initialized \n");
				break;
			}
			
			printf("Current %d \n", frame_num);
			printf("Set frame_num>> \n");
			frame_num = (UINT32)get_choose_int();
			printf("Set to %d \n", frame_num);
		break;

		case 24:
			printf("Current %d %d %d \n", init_c_gain[0], init_c_gain[1], init_c_gain[2]);
			printf("Set r gain>> \n");
			init_c_gain[0] = (UINT32)get_choose_int();
			printf("Set g gain>> \n");
			init_c_gain[1] = (UINT32)get_choose_int();
			printf("Set b gain>> \n");
			init_c_gain[2] = (UINT32)get_choose_int();
			printf("Set to %d %d %d \n", init_c_gain[0], init_c_gain[1], init_c_gain[2]);

			{
				ISPT_C_GAIN c_gain = {0};

				c_gain.id = sen_vcap_id;
				c_gain.gain[0] = init_c_gain[0];
				c_gain.gain[1] = init_c_gain[1];
				c_gain.gain[2] = init_c_gain[2];
				vendor_isp_set_common(ISPT_ITEM_C_GAIN, &c_gain);
			}
		break;

		case 25:
			printf("Current %d \n", init_total_gain);
			printf("Set init_total_gain>> \n");
			init_total_gain = (UINT32)get_choose_int();
			printf("Set to %d \n", init_total_gain);

			{
				ISPT_TOTAL_GAIN total_gain = {0};

				total_gain.id = sen_vcap_id;
				total_gain.gain = init_total_gain;
				vendor_isp_set_common(ISPT_ITEM_TOTAL_GAIN, &total_gain);
			}
		break;

		#if (VPE_FUN_ENABLE)
		case 26:
			if (is_init_emu) {
				printf("emu has been initialized \n");
				break;
			}

			printf("Current %s \n", (vpe_enable == 0) ? "enable" : "disable");
			printf("Set vpe_enable(0: disable; 1: enable)>> \n");
			vpe_enable = (UINT32)get_choose_int();
			printf("Set to %s \n", (vpe_enable == 0) ? "enable" : "disable");
		break;
		#endif

		case 27:
			if (is_init_emu) {
				printf("emu has been initialized \n");
				break;
			}

			printf("Current %d \n", strpix);
			printf("Set strpix(0: R; 1: GR; 2: GB; 3: B)>> \n");
			strpix = (UINT32)get_choose_int();
			if (strpix > 3) {
				strpix = 0;
				printf("Force set to %d \n", strpix);
			} else {
				printf("Set to %d \n", strpix);
			}
		break;

		case 30:
			if (is_init_emu) {
				printf("emu has been initialized \n");
				break;
			}

			printf("Current %s \n", (enc_type == 0) ? "H.265" : "H.264");
			printf("Set enc_type>> \n");
			enc_type = (UINT32)get_choose_int();
			printf("Set to %s \n", (enc_type == 0) ? "H.265" : "H.264");
		break;

		case 31:
			if (is_init_emu) {
				printf("emu has been initialized \n");
				break;
			}

			printf("Current %d \n", enc_bitrate);
			printf("Set enc_bitrate>> \n");
			enc_bitrate = (UINT32)get_choose_int();
			printf("Set to %d \n", enc_bitrate);
		break;

		case 40:
			if (emu_flow_start) {
				printf("emu has already started \n");
				break;
			}

			printf("Current %s \n", linear_path);
			printf("Set linear_path >> \n");
			scanf("%s", linear_path);
			printf("Set to %s \n", linear_path);
		break;

		case 41:
			if (emu_flow_start) {
				printf("emu has already started \n");
				break;
			}

			printf("Current %s \n", shdr_path);
			printf("Set shdr_path >> \n");
			scanf("%s", shdr_path);
			printf("Set to %s \n", shdr_path);
		break;

		case 42:
			if (is_init_emu) {
				printf("emu has been started \n");
				break;
			}

			printf("Current %s \n", isp_cfg_name);
			printf("Set isp_cfg_name >> \n");
			scanf("%s", isp_cfg_name);
			printf("Set to %s \n", isp_cfg_name);
		break;

		case 43:
			printf("Current %s \n", load_raw_folder);
			printf("Set load_raw_folder >> \n");
			scanf("%s", load_raw_folder);
			printf("Set to %s \n", load_raw_folder);
		break;

		case 50:
			printf("Current %s \n", bypass_1st_3dnr_en ? "enable" : "disable");
			printf("Set bypass_1st_3dnr_en(0: disable; 1: enable)>> \n");
			bypass_1st_3dnr_en = (UINT32)get_choose_int();
			printf("Set to %s \n", bypass_1st_3dnr_en ? "enable" : "disable");
		break;

		case 51:
			printf("Current %s \n", bypass_1st_lce_en ? "enable" : "disable");
			printf("Set bypass_1st_lce_en(0: disable; 1: enable)>> \n");
			bypass_1st_lce_en = (UINT32)get_choose_int();
			printf("Set to %s \n", bypass_1st_lce_en ? "enable" : "disable");
		break;

		case 52:
			printf("Current %s \n", bypass_1st_wdr_en ? "enable" : "disable");
			printf("Set bypass_1st_wdr_en(0: disable; 1: enable)>> \n");
			bypass_1st_wdr_en = (UINT32)get_choose_int();
			printf("Set to %s \n", bypass_1st_wdr_en ? "enable" : "disable");
		break;

		#if (EMU_AI_ENABLE)
		case 71:
			if (is_init_emu) {
				printf("emu has been started \n");
				break;
			}

			printf("Current %s \n", aiisp_flow_enable ? "enable" : "disable");
			printf("Set ai flow enable(0: disable; 1: enable)>> \n");
			aiisp_flow_enable = (UINT32)get_choose_int();
			printf("Set to %s \n", aiisp_flow_enable ? "enable" : "disable");
		break;

		case 73:
			aiisp_get_model_name(0, file_name);

			if (is_init_emu) {
				printf("emu has been started \n");
				break;
			}

			printf("Current %s \n", file_name);
			printf("Set ai model name1 >> \n");
			scanf("%s", file_name);
			printf("Set to %s \n", file_name);
			aiisp_set_model_name(0, file_name);
		break;

		case 74:
			aiisp_get_model_name(1, file_name);

			if (is_init_emu) {
				printf("emu has been started \n");
				break;
			}

			printf("Current %s \n", file_name);
			printf("Set ai model name2 >> \n");
			scanf("%s", file_name);
			printf("Set to %s \n", file_name);
			aiisp_set_model_name(1, file_name);
		break;

		case 75:
			aiisp.id = sen_vcap_id + 1;
			vendor_isp_get_iq(IQT_ITEM_AIISP_PARAM, &aiisp);
			printf("Current enable %d, effect %d, param %d %d \n", aiisp.aiisp.manual_param.enable, aiisp.aiisp.manual_param.effect, aiisp.aiisp.manual_param.param[0], aiisp.aiisp.manual_param.param[1]);
			printf("Set enable>> \n");
			aiisp.aiisp.manual_param.enable = (UINT32)get_choose_int();
			printf("Set effect>> \n");
			aiisp.aiisp.manual_param.effect = (UINT32)get_choose_int();
			printf("Set param[0]>> \n");
			aiisp.aiisp.manual_param.param[0] = (UINT32)get_choose_int();
			printf("Set param[1]>> \n");
			aiisp.aiisp.manual_param.param[1] = (UINT32)get_choose_int();
			aiisp.aiisp.mode = IQ_OP_TYPE_MANUAL;
			vendor_isp_set_iq(IQT_ITEM_AIISP_PARAM, &aiisp);
			aiisp.id = sen_vcap_id;
			vendor_isp_set_iq(IQT_ITEM_AIISP_PARAM, &aiisp);
			printf("Set to enable %d, effect %d, param %d %d \n", aiisp.aiisp.manual_param.enable, aiisp.aiisp.manual_param.effect, aiisp.aiisp.manual_param.param[0], aiisp.aiisp.manual_param.param[1]);
		break;
		#endif

		default:
			printf("wrong input (%d) \n", option);
			break;

		case 0:
			trig = 0;
			break;
		}
	}

	return 0;
}

