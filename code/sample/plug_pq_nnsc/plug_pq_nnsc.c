/**
	@brief Sample code of video rtsp.\n

	@file plug_nn_sc.c

	@author Photon Lin

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#include <string.h>

#include "hdal.h"
#include "hd_debug.h"
#include "vendor_isp.h"

// NOTE: NNSC lib include
#include "nnsc_lib.h"
// NOTE: NNSC net include
#include "vendor_ai.h"
#include "vendor_ai_util.h"
#include "vendor_ai_cpu/vendor_ai_cpu.h"
#include "vendor_ai_cpu_postproc.h"

// platform dependent
#if defined(__LINUX)
#include <pthread.h> //for pthread API
#define MAIN(argc, argv) int main(int argc, char** argv)
#define GETCHAR() getchar()
#endif

///////////////////////////////////////////////////////////////////////////////
// NOTE: Function control of NNSC NET
#define CALC_NNSC_DURATION              14    // skip n frames and process once
#define ACCESS_NNSC_NET_EN               1
#define ACCESS_NNSC_LIB_EN               1
#define OSG_FUNC_EN                      1
#define SAVE_YUV_EN                      0
#define EXTEND_YUV_EN                    1

#define VCAP_ID_PATH_1                   2
#define VPRC_ID_PATH_1                   0
#define VPRC_ID_PATH_2                   1
#define VENC_ID_PATH_2                   VPRC_ID_PATH_2

#if (EXTEND_YUV_EN)
#define ISP_ID_PATH_2                    (1 << 31) | ((VCAP_ID_PATH_1 & 0x7F) << 24) | (1 << 15) | ((VCAP_ID_PATH_1 & 0x7F) << 8) | ((VCAP_ID_PATH_1 + 1) & 0xFF)
#define ISP_CFG_PATH                     "isp_os02k10_0"
#define ISP_CFG_PATH_HDR                 "isp_os02k10_0_hdr"

#else
#define ISP_ID_PATH_2                    (VCAP_ID_PATH_1 + 1)
#endif

#define VIDEOPROC_ALG_FUNC HD_VIDEOPROC_FUNC_AF | HD_VIDEOPROC_FUNC_WDR | HD_VIDEOPROC_FUNC_DEFOG | HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA
#define CAP_OUT_FMT        HD_VIDEO_PXLFMT_RAW12
#define SHDR2_CAP_OUT_FMT  HD_VIDEO_PXLFMT_RAW12_SHDR2

// NOTE: customer setting of NNSC NET
#define NNSC_NET_MODEL_NAME_V_3_01_000   "/mnt/sd/para/nvt_model_sc_3_01_230302.bin"
#define NNSC_NET_MODEL_SIZE_V_3_01_000   15319040

#define NNSC_NET_VERSION_V_3_01_000      1
#define NNSC_NET_VERSION                 NNSC_NET_VERSION_V_3_01_000

#if (NNSC_NET_VERSION == NNSC_NET_VERSION_V_3_01_000)
#define NNSC_NET_MODEL_NAME              NNSC_NET_MODEL_NAME_V_3_01_000
#define NNSC_NET_MODEL_SIZE              NNSC_NET_MODEL_SIZE_V_3_01_000
#endif
#define NNSC_NET_JOB_METHOD              1
#define NNSC_NET_WAIT_TIME               0
#define NNSC_NET_BUF_METHOD              0
//#define NNSC_NET_LABEL_NAME              "/mnt/sd/accuracy/labels.txt"
#define NNSC_NET_LABEL_NAME              "/mnt/sd/accuracy/labels_20211122.txt"

#define NNSC_NET_MODEL_IN_PATH           0
#define NNSC_NET_MODEL_PROC_PATH         0

///////////////////////////////////////////////////////////////////////////////
#define HD_VIDEOCAP_PATH(dev_id, in_id, out_id) (((dev_id) << 16) | (((in_id) & 0x00ff) << 8)| ((out_id) & 0x00ff))
#define HD_VIDEOPROC_PATH(dev_id, in_id, out_id) (((dev_id) << 16) | (((in_id) & 0x00ff) << 8)| ((out_id) & 0x00ff))
#define HD_VIDEOENC_PATH(dev_id, in_id, out_id) (((dev_id) << 16) | (((in_id) & 0x00ff) << 8)| ((out_id) & 0x00ff))

//header
#define DBGINFO_BUFSIZE() (0x200)
//YUV
#define VDO_YUV_BUFSIZE(w, h, pxlfmt) (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))

#define HD_COMMON_CFG_MULTIPROC 0x80000000

typedef struct _VIDEO_PLUG_STREAM {
	// (1)
	HD_PATH_ID cap_path;

	// (2)
	HD_PATH_ID proc_path;

	// (3)
	HD_PATH_ID enc_path;
} VIDEO_PLUG_STREAM;

static BOOL is_nnsc_init;
static BOOL is_nnsc_run;
pthread_t  nnsc_thread_id;
static BOOL nnsc_thread_run;
static BOOL nnsc_net_msg_en;
static BOOL nnsc_lib_func_en;
static UINT32 nnsc_lib_msg_type;

#if (EXTEND_YUV_EN)
static UINT32 nnsc_yuv_src = 1;  // 0: original yuv; 1: extend yuv
pthread_t extend_yuv_thread_id;
static BOOL extend_yuv_thread_run;
static UINT32 frame_num = 1;
#endif

///////////////////////////////////////////////////////////////////////////////
// NOTE: NNSC net usage
#define VENDOR_AI_CFG 0x000F0000  //vendor ai config
#define AI_RGB_BUFSIZE(w, h) (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(HD_VIDEO_PXLFMT_RGB888_PLANAR) / 8) * (h))
#define NET_PATH_ID UINT32

typedef struct _MEM_PARM {
	ULONG pa;
	ULONG va;
	UINT32 size;
	UINT32 blk;
} MEM_PARM;

typedef struct _NET_IN_CONFIG {
	UINT32 w;
	UINT32 h;
	UINT32 c;
	UINT32 loff;
	UINT32 fmt;
} NET_IN_CONFIG;

typedef struct _NET_IN {
	NET_IN_CONFIG in_cfg;
	MEM_PARM input_mem;
	UINT32 in_id;
	VENDOR_AI_BUF src_img;
} NET_IN;

typedef struct _NET_PROC_CONFIG {
	CHAR model_filename[256];
	INT32 binsize;
	int job_method;
	int job_wait_ms;
	int buf_method;
	void *p_share_model;
	CHAR label_filename[256];
} NET_PROC_CONFIG;

typedef struct _NET_PROC {
	NET_PROC_CONFIG net_cfg;
	MEM_PARM proc_mem;
	UINT32 proc_id;
	CHAR out_class_labels[MAX_CLASS_NUM * VENDOR_AIS_LBL_LEN];
	MEM_PARM rslt_mem;
	MEM_PARM io_mem;
} NET_PROC;

typedef struct _VIDEO_NN {
	// (1) input 
	HD_PATH_ID in_path;

	// (2) network 
	HD_PATH_ID net_path;
} VIDEO_NN;

///////////////////////////////////////////////////////////////////////////////
// NOTE: NNSC lib usage
#if (ACCESS_NNSC_NET_EN)
#define TONE_LV_SMOOTH_FACTOR     7

static NNSC_PARAM nnsc_param = {
	{
		{5, 200}, // NNSC_TYPE_BACKLIGHT
		{5, 200}, // NNSC_TYPE_FOGGY
		{5, 200}, // NNSC_TYPE_GRASS
		{5, 200}, // NNSC_TYPE_SKIN_COLOR
		{5, 200}, // NNSC_TYPE_SNOWFIELD
	}
};

static NNSC_TEST test_param ={
	.enable = FALSE,
	.mode_type = NNSC_TYPE_BACKLIGHT,
};

static SCD_CONTROL scd_control = {
	.enable = TRUE,
	.auto_enable = TRUE,
	.manual_stable = FALSE,
};

static SCD_PARAM scd_param = {
	.stable_sensitive = 7,
	.change_sensitive = 5,
	.g_diff_num_th = 200,
};

NNSC_ADJ nnsc_adj = {0};
AET_EXP_RATIO exp_ratio = {0};
AWBT_GREEN_REMOVE green_remove = {0};
AWBT_SKIN_REMOVE skin_remove = {0};
IQT_DARK_ENH_RATIO dark_enh_ratio = {0};
IQT_CONTRAST_ENH_RATIO contrast_enh_ratio = {0};
IQT_GREEN_ENH_RATIO green_enh_ratio = {0};
IQT_SKIN_ENH_RATIO skin_enh_ratio = {0};
IQT_SHDR_TONE_LV shdr_tone_lv = {0};
#endif

///////////////////////////////////////////////////////////////////////////////
// NOTE: NNSC net usage
#if (ACCESS_NNSC_NET_EN)
VIDEO_NN stream_nn = {NNSC_NET_MODEL_IN_PATH, NNSC_NET_MODEL_PROC_PATH}; // NN stream
// net in
NET_IN_CONFIG in_cfg = {0};
// net proc
NET_PROC_CONFIG net_cfg = {
	.model_filename = NNSC_NET_MODEL_NAME,
	.binsize =        NNSC_NET_MODEL_SIZE,
	.job_method =     NNSC_NET_JOB_METHOD,   // default 0: sequential
	.job_wait_ms =    NNSC_NET_WAIT_TIME,    // async mode
	.buf_method =     NNSC_NET_BUF_METHOD,   // 0: allocate each buffer, 1: shrink buffer space (default)
	.label_filename = NNSC_NET_LABEL_NAME,
};

static UINT32 nnsc_score_num = 0;
static NNSC_SCORE nnsc_score = {{0}};
static SCD_STATISTICS scd_statistic = {{0}};
static NET_IN g_in[16] = {0};
static NET_PROC g_net[16] = {0};
ULONG outlayer_buf_pa = 0;
VENDOR_AI_BUF outlayer_buf_va = {0};
ULONG softmax_loc_layer_float_pa = 0;
FLOAT *softmax_loc_layer_float_va = NULL;
UINT32 outlayer_path_list[256] = {0};
UINT32 outlayer_num = 0;
UINT32 outlayer_length;

#if (OSG_FUNC_EN)
#define OSG_ICON_FILE_NAME      "/mnt/sd/osg/pq_osg_icon_w20_h32.dat"
#define OSG_ICON_FILE_SIZE      79360
#define OSG_ICON_WIDTH          20
#define OSG_ICON_HEIGHT         32
#define OSG_ICON_NUM_NUMBER     10
#define OSG_ICON_CHAR_NUMBER    26

#define OSG_TRANSP              0x0000
#define OSG_BG_COLOR            0x4444
#define OSG_BULE                0xF00F
#define OSG_GREEN2              0xF0C0
#define OSG_GREEN               0xF0F0
#define OSG_GRAY                0xF888
#define OSG_PINK                0xFC68
#define OSG_WHITE2              0xFEEE
#define OSG_RED                 0xFF00
#define OSG_PURPLE              0xFF0F
#define OSG_YELLOW              0xFFF0
#define OSG_WHITE               0xFFFF
#define OSG_BAR_MAX_COLOR       OSG_RED
#define OSG_BAR_NORM_COLOR      OSG_WHITE

#define OSG_ITEM_NUM            8
#define OSG_BAR_MAX             100
#define OSG_TITLE_MAX           20
#define OSG_STRING_MAX          (OSG_TITLE_MAX<<1)

typedef struct _VIDEO_OSG {
	HD_PATH_ID stamp_path;
	HD_COMMON_MEM_VB_BLK stamp_blk;
	ULONG stamp_pa;
	UINT32 stamp_size;
	UINT16 *osg_buf;
	pthread_t thread_id;
	BOOL thread_run;
} VIDEO_OSG;

VIDEO_OSG stream_osg = {0};

typedef struct _OSG_UPDATE_INFO {
	UINT16 value[OSG_ITEM_NUM];
	UINT8 title_text[OSG_ITEM_NUM][OSG_TITLE_MAX];
	UINT16 title_color[OSG_ITEM_NUM];
} OSG_UPDATE_INFO;

OSG_UPDATE_INFO osg_update_info = {
	.title_text = {
		{" BL Human"},
		{" BL Scene"},
		{"    Foggy"},
		{"    Grass"},
		{"GrassLand"},
		{"SkinColor"},
		{"   Normal"},
		{"SnowField"}
	},
	.title_color = {
		OSG_YELLOW,
		OSG_YELLOW,
		OSG_BULE,
		OSG_GREEN,
		OSG_GREEN2,
		OSG_PINK,
		OSG_WHITE2,
		OSG_WHITE
	}
};

typedef struct _OSG_DISPLAY_ {
	UINT16 icon_w, icon_h, icon_size;
	UINT16 bar_step;
	UINT16 value[OSG_ITEM_NUM];
	UINT8 title_text[OSG_ITEM_NUM][OSG_TITLE_MAX];
	UINT16 title_color[OSG_ITEM_NUM];
	UINT8 string[OSG_ITEM_NUM][OSG_STRING_MAX];
	UINT16 line_icon_num[OSG_ITEM_NUM];
	UINT16 line_w[OSG_ITEM_NUM];
	UINT16 width, height;
} OSG_DISPLAY;

OSG_DISPLAY osg_display = {
	.icon_w = OSG_ICON_WIDTH,
	.icon_h = OSG_ICON_HEIGHT,
	.icon_size = (OSG_ICON_WIDTH * OSG_ICON_HEIGHT),
	.bar_step = 1,
	.value = {0},
	.title_text = {
		{" BL Human"},
		{" BL Scene"},
		{"    Foggy"},
		{"    Grass"},
		{"GrassLand"},
		{"SkinColor"},
		{"   Normal"},
		{"SnowField"}
	},
	.title_color = {
		OSG_YELLOW,
		OSG_YELLOW,
		OSG_BULE,
		OSG_GREEN,
		OSG_GREEN2,
		OSG_PINK,
		OSG_WHITE2,
		OSG_WHITE
	},
	.line_icon_num = {0},
	.line_w = {0},
	.width = OSG_ICON_WIDTH,
	.height = OSG_ICON_HEIGHT
};

static UINT16 *osg_number;
static UINT16 *osg_uppercase;
static UINT16 *osg_lowercase;
static UINT32 vdo_size_w = 1920, vdo_size_h = 1080;

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

static HD_RESULT osg_mem_alloc(void)
{
	ULONG pa;
	HD_COMMON_MEM_VB_BLK blk;

	if(!stream_osg.stamp_size){
		printf("stamp_size is unknown\n");
		return HD_ERR_NG;
	}

	//get osd stamp's block
	blk = hd_common_mem_get_block(HD_COMMON_MEM_OSG_POOL, stream_osg.stamp_size, DDR_ID0);
	if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("get block fail\r\n");
		return HD_ERR_NG;
	}

	stream_osg.stamp_blk = blk;

	//translate stamp block to physical address
	pa = hd_common_mem_blk2pa(blk);
	if (pa == 0) {
		printf("blk2pa fail, blk = 0x%x\r\n", blk);
		return HD_ERR_NG;
	}

	stream_osg.stamp_pa = pa;

	return HD_OK;
}

static HD_RESULT osg_icon_init(void)
{
	UINT32 i, ix, ix_img, iy, iy_img, iyw, iyw_img;
	int fd, ret;
	UINT16 *osg_icon_buf = NULL;

	osg_display.icon_w = OSG_ICON_WIDTH;
	osg_display.icon_h = OSG_ICON_HEIGHT;
	osg_display.icon_size = OSG_ICON_WIDTH * OSG_ICON_HEIGHT;

	osg_icon_buf = malloc(OSG_ICON_FILE_SIZE);
	if(!osg_icon_buf){
		printf("fail to allocate pq osg icon buffer\n");
		return HD_ERR_NG;
	}

	fd = open(OSG_ICON_FILE_NAME, O_RDONLY);
	if(fd == -1){
		printf("fail to open %s \n", OSG_ICON_FILE_NAME);
		return HD_ERR_NG;
	}

	ret = read(fd, osg_icon_buf, OSG_ICON_FILE_SIZE);
	close(fd);
	if (ret != OSG_ICON_FILE_SIZE) {
		printf("fail to read %s (%d, %d) \n", OSG_ICON_FILE_NAME, ret, OSG_ICON_FILE_SIZE);
		return HD_ERR_NG;
	}

	osg_number = malloc(OSG_ICON_NUM_NUMBER * osg_display.icon_size * sizeof(UINT16));
	osg_uppercase = malloc(OSG_ICON_CHAR_NUMBER * osg_display.icon_size * sizeof(UINT16));
	osg_lowercase = malloc(OSG_ICON_CHAR_NUMBER * osg_display.icon_size * sizeof(UINT16));

	for (i = 0; i < OSG_ICON_NUM_NUMBER; i++) {
		for (iy = 0, iy_img = 0; iy < osg_display.icon_h; iy++, iy_img++) {
			iyw = iy * osg_display.icon_w;
			iyw_img = iy_img * ((OSG_ICON_NUM_NUMBER + OSG_ICON_CHAR_NUMBER + OSG_ICON_CHAR_NUMBER) * osg_display.icon_w);
			for (ix = 0, ix_img = (i * osg_display.icon_w); ix < osg_display.icon_w; ix++, ix_img++) {
				osg_number[(i * osg_display.icon_size) + iyw + ix] = osg_icon_buf[iyw_img + ix_img];
			}
		}
	}

	for (i = 0; i < OSG_ICON_CHAR_NUMBER; i++) {
		for (iy = 0, iy_img = 0; iy < osg_display.icon_h; iy++, iy_img++) {
			iyw = iy * osg_display.icon_w;
			iyw_img = iy_img * ((OSG_ICON_NUM_NUMBER + OSG_ICON_CHAR_NUMBER + OSG_ICON_CHAR_NUMBER) * osg_display.icon_w);
			for (ix = 0, ix_img = ((i + OSG_ICON_NUM_NUMBER) * osg_display.icon_w); ix < osg_display.icon_w; ix++, ix_img++) {
				osg_uppercase[(i * osg_display.icon_size) + iyw + ix] = osg_icon_buf[iyw_img + ix_img];
			}
		}
	}

	for (i = 0; i < OSG_ICON_CHAR_NUMBER; i++) {
		for (iy = 0, iy_img = 0; iy < osg_display.icon_h; iy++, iy_img++) {
			iyw = iy * osg_display.icon_w;
			iyw_img = iy_img * ((OSG_ICON_NUM_NUMBER + OSG_ICON_CHAR_NUMBER + OSG_ICON_CHAR_NUMBER)*osg_display.icon_w);
			for(ix = 0, ix_img = ((i + OSG_ICON_NUM_NUMBER + OSG_ICON_CHAR_NUMBER) * osg_display.icon_w); ix < osg_display.icon_w; ix++, ix_img++) {
				osg_lowercase[(i * osg_display.icon_size) + iyw + ix] = osg_icon_buf[iyw_img+ix_img];
			}
		}
	}

	free(osg_icon_buf);

	return 0;
}

static HD_RESULT osg_icon_uninit(void)
{
	if (osg_number != NULL) {
		free(osg_number);
	}
	if (osg_uppercase != NULL) {
		free(osg_uppercase);
	}
	if (osg_lowercase != NULL) {
		free(osg_lowercase);
	}

	return HD_OK;
}

static void osg_clear_buf(UINT16 *osg_buf)
{
	UINT32 i;

	for (i = 0; i < (osg_display.width * osg_display.height); i++) {
		osg_buf[i] = OSG_BG_COLOR;
	}
}

static void osg_param_init(void)
{
	UINT32 i, idx, cnt;
	UINT32 osg_buf_w = 0, osg_buf_h = 0;
	CHAR string[OSG_STRING_MAX] = {0};
	UINT32 osg_buf_w_max = 0;

	for (idx = 0; idx < OSG_ITEM_NUM; idx++) {

		for (i = 0; i < OSG_STRING_MAX; i++) {
			string[i] = 0;
		}

		sprintf((CHAR *)string, "%s%3d", osg_display.title_text[idx], osg_display.value[idx]);

		cnt = 0;
		for (i = 0; i < OSG_STRING_MAX; i++) {
			if (string[i] != 0) {
				osg_display.string[idx][i] = string[i];
				cnt++;
			} else {
				osg_display.line_icon_num[idx] = cnt;
				osg_display.line_w[idx] = (osg_display.line_icon_num[idx] * osg_display.icon_w);
				if((cnt * osg_display.icon_w) >= osg_buf_w_max) {
					osg_buf_w_max = (cnt * osg_display.icon_w);
				}
				break;
			}
		}
	}

	osg_buf_w = osg_buf_w_max;
	osg_buf_w += (osg_display.bar_step * OSG_BAR_MAX);
	osg_buf_h = (OSG_ITEM_NUM * osg_display.icon_h);

	if ((osg_buf_h % 16) != 0) {
		osg_buf_h = (((osg_buf_h / 16) + 1) * 16);
	} else {
		osg_buf_h = ((osg_buf_h / 16) * 16);
	}

	osg_display.width = osg_buf_w;
	osg_display.height = osg_buf_h;
}

static UINT32 osg_get_max_ch(void)
{
	UINT32 i, max_value = 0, max_ch = OSG_ITEM_NUM;

	for (i = 0; i < OSG_ITEM_NUM; i++) {
		if (osg_display.value[i] > max_value) {
			max_value = osg_display.value[i];
			max_ch = i;
		}
	}

	return max_ch;
}

static void osg_draw(UINT16 *osg_buf)
{
	UINT32 osg_ix, osg_iy, osg_iyw;
	UINT32 idx;
	UINT8 char_idx;
	UINT32 line_ix, line_iy, line_idx;
	UINT32 line_iyw;
	UINT32 line_osg_ix, line_osg_iy;
	UINT32 ix, iy;

	for (idx = 0; idx < OSG_ITEM_NUM; idx++) {
		for (line_idx = 0; line_idx < osg_display.line_icon_num[idx]; line_idx++) {
			char_idx = osg_display.string[idx][line_idx];
			for (line_iy = 0, line_osg_iy = (idx * osg_display.icon_h); line_iy < osg_display.icon_h; line_iy++, line_osg_iy++) {
				line_iyw = line_iy * osg_display.icon_w;
				for (line_ix = 0, line_osg_ix = (line_idx * osg_display.icon_w); line_ix < osg_display.icon_w; line_ix++, line_osg_ix++) {
					osg_ix = line_osg_ix;
					osg_iy = line_osg_iy;
					osg_iyw = (osg_iy * osg_display.width);

					if ((char_idx >= 65) && (char_idx <= 90)) {
						osg_buf[osg_iyw+osg_ix] = (osg_uppercase[((char_idx - 65) * osg_display.icon_size) + line_iyw + line_ix] == 0x0000) ? OSG_BG_COLOR : osg_uppercase[((char_idx - 65) * osg_display.icon_size) + line_iyw + line_ix];
					} else if ((char_idx >= 97) && (char_idx <= 122)) {
						osg_buf[osg_iyw + osg_ix] = (osg_lowercase[((char_idx - 97) * osg_display.icon_size) + line_iyw + line_ix] == 0x0000) ? OSG_BG_COLOR : osg_lowercase[((char_idx-97) * osg_display.icon_size) + line_iyw + line_ix];
					} else if ((char_idx >= 48) && (char_idx <= 57)) {
						osg_buf[osg_iyw + osg_ix] = (osg_number[((char_idx  - 48) * osg_display.icon_size) + line_iyw + line_ix] == 0x0000) ? OSG_BG_COLOR : osg_number[((char_idx-48) * osg_display.icon_size) + line_iyw + line_ix];
					} else {
						osg_buf[osg_iyw+osg_ix] = OSG_BG_COLOR;
					}

					if (osg_buf[osg_iyw+osg_ix] != OSG_BG_COLOR) {
						osg_buf[osg_iyw+osg_ix] = osg_display.title_color[idx];
					}
				}
			}

			for (iy = 0; iy < osg_display.icon_h; iy++) {
				for (ix = 0; ix < (OSG_BAR_MAX * osg_display.bar_step); ix++) {
					osg_ix = ix + osg_display.line_w[idx];
					osg_iy = iy + (idx * osg_display.icon_h);
					osg_iyw = (osg_iy * osg_display.width);

					if (ix < (osg_display.value[idx] * osg_display.bar_step)) {
						if ((osg_get_max_ch() == idx) && (osg_display.value[idx] > 50)) {
							osg_buf[osg_iyw+osg_ix] = OSG_BAR_MAX_COLOR;
						} else {
							osg_buf[osg_iyw+osg_ix] = OSG_BAR_NORM_COLOR;
						}
					} else {
						osg_buf[osg_iyw+osg_ix] = OSG_BG_COLOR;
					}
				}
			}

			for (iy = 0; iy < osg_display.icon_h; iy++) {
				for (ix = 0; ix < (OSG_BAR_MAX * osg_display.bar_step); ix++) {
					osg_ix = ix + osg_display.line_w[idx];
					osg_iy = iy + (idx * osg_display.icon_h);
					osg_iyw = (osg_iy * osg_display.width);

					if (ix < (osg_display.value[idx] * osg_display.bar_step)) {
						if ((osg_get_max_ch() == idx) && (osg_display.value[idx] > 50)) {
							osg_buf[osg_iyw+osg_ix] = OSG_BAR_MAX_COLOR;
						} else {
							osg_buf[osg_iyw+osg_ix] = OSG_BAR_NORM_COLOR;
						}
					} else {
						osg_buf[osg_iyw+osg_ix] = OSG_BG_COLOR;
					}
				}
			}
		}
	}
}

static void osg_update(void)
{
	UINT32 i, idx;
	CHAR string[OSG_STRING_MAX] = {0};

	memcpy(&osg_display.value, &osg_update_info, sizeof(OSG_UPDATE_INFO));

	for (idx = 0; idx < OSG_ITEM_NUM; idx++) {

		for (i = 0; i < OSG_STRING_MAX; i++) {
			string[i] = 0;
		}

		if (osg_display.value[idx] > OSG_BAR_MAX) {
			osg_display.value[idx] = OSG_BAR_MAX;
		}

		sprintf((CHAR *)string, "%s%3d", osg_display.title_text[idx], osg_display.value[idx]);

		for (i = 0; i < OSG_STRING_MAX; i++) {
			if (string[i] != 0) {
				osg_display.string[idx][i] = string[i];
			} else {
				break;
			}
		}
	}
}

static HD_RESULT osg_set_enc_stamp(void)
{
	HD_OSG_STAMP_BUF  buf;
	HD_OSG_STAMP_IMG  img;
	HD_OSG_STAMP_ATTR attr;

	if(!stream_osg.stamp_pa){
		printf("stamp buffer is not allocated\n");
		return -1;
	}

	memset(&buf, 0, sizeof(HD_OSG_STAMP_BUF));

	buf.type      = HD_OSG_BUF_TYPE_PING_PONG;
	buf.p_addr    = stream_osg.stamp_pa;
	buf.size      = stream_osg.stamp_size;

	if(hd_videoenc_set(stream_osg.stamp_path, HD_VIDEOENC_PARAM_IN_STAMP_BUF, &buf) != HD_OK){
		printf("fail to set stamp buffer\n");
		return -1;
	}

	memset(&img, 0, sizeof(HD_OSG_STAMP_IMG));

	img.fmt        = HD_VIDEO_PXLFMT_ARGB4444;
	img.dim.w      = osg_display.width;
	img.dim.h      = osg_display.height;
	img.p_addr     = (ULONG)stream_osg.osg_buf;

	if(hd_videoenc_set(stream_osg.stamp_path, HD_VIDEOENC_PARAM_IN_STAMP_IMG, &img) != HD_OK){
		printf("fail to set stamp image\n");
		return -1;
	}

	memset(&attr, 0, sizeof(HD_OSG_STAMP_ATTR));

	attr.position.x = 10;
	attr.position.y = (vdo_size_h - osg_display.height - 10);
	attr.alpha      = 0;
	attr.layer      = 0;
	attr.region     = 0;

	return hd_videoenc_set(stream_osg.stamp_path, HD_VIDEOENC_PARAM_IN_STAMP_ATTR, &attr);
}

static void *osg_thread(void *arg)
{
	while(1) {
		usleep(30000);  // 30ms

		if (!stream_osg.thread_run) {
			break;
		}

		osg_update();
		osg_draw(stream_osg.osg_buf);
		osg_set_enc_stamp();
	}

	return 0;
}

static HD_RESULT osg_init(void)
{
	HD_RESULT ret = HD_OK;
	osg_param_init();

	//allocate logo buffer
	stream_osg.osg_buf = malloc(vdo_size_w * vdo_size_h * sizeof(unsigned short));
	if(!stream_osg.osg_buf){
		printf("fail to allocate pq osg buffer\n");
		return HD_ERR_NG;
	}

	osg_clear_buf(stream_osg.osg_buf);

	//load icon from sd card
	if(osg_icon_init()){
		printf("fail to load icon image\n");
		free(stream_osg.osg_buf);
		return HD_ERR_NG;
	}

	osg_update();
	osg_draw(stream_osg.osg_buf);

	// init stamp data
	stream_osg.stamp_blk  = 0;
	stream_osg.stamp_pa   = 0;
	stream_osg.stamp_size = osg_calc_buf_size(vdo_size_w, vdo_size_h);
	if(stream_osg.stamp_size <= 0){
		printf("osg_calc_buf_size() fail\n");
		return HD_ERR_NG;
	}

	ret = osg_mem_alloc();
	if (ret) {
		printf("allocate stamp buffer fail \n");
		return HD_ERR_NG;
	}

	//setup enc stamp parameter
	if (osg_set_enc_stamp() != HD_OK) {
		printf("osg_set_enc_stamp fial, path = 0x%X \r\n", stream_osg.stamp_path);
		return HD_ERR_NG;
	}

	if (hd_videoenc_start(stream_osg.stamp_path) != HD_OK) {
		printf("hd_videoenc_start fail, path = 0x%X \r\n", stream_osg.stamp_path);
	}

	stream_osg.thread_run = TRUE;
	ret = pthread_create(&stream_osg.thread_id, NULL, osg_thread, NULL);
	if (ret < 0) {
		printf("create osg thread fail \n");
		return HD_ERR_NG;
	}

	return ret;
}

static void osg_uninit(void)
{
	stream_osg.thread_run = FALSE;
	pthread_join(stream_osg.thread_id, NULL);

	osg_icon_uninit();

	if(stream_osg.stamp_blk) {
		if(hd_common_mem_release_block(stream_osg.stamp_blk) != HD_OK) {
			printf("hd_common_mem_release_block() fail \n");
		}
		stream_osg.stamp_blk = 0;
	}
}

#if (EXTEND_YUV_EN)
static HD_RESULT plug_pq_nnsc_set_proc_cfg(HD_PATH_ID *p_video_proc_ctrl, HD_DIM* p_max_dim, HD_OUT_ID _out_id)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
	HD_PATH_ID video_proc_ctrl = 0;

	ret = hd_videoproc_open(0, _out_id, &video_proc_ctrl); //open this for device control
	if (ret != HD_OK)
		return ret;

	if (p_max_dim != NULL ) {
		video_cfg_param.pipe = HD_VIDEOPROC_PIPE_RAWALL;
		video_cfg_param.isp_id = ISP_ID_PATH_2;
		video_cfg_param.ctrl_max.func = VIDEOPROC_ALG_FUNC;

		if (frame_num >= 2) {
			video_cfg_param.ctrl_max.func |= HD_VIDEOPROC_FUNC_SHDR;
		} else {
			video_cfg_param.ctrl_max.func &= ~HD_VIDEOPROC_FUNC_SHDR;
		}
		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;
		if (frame_num >= 2) {
			video_cfg_param.in_max.pxlfmt = SHDR2_CAP_OUT_FMT;
		} else {
			video_cfg_param.in_max.pxlfmt = CAP_OUT_FMT;
		}
		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
		if (ret != HD_OK) {
			return ret;
		}
	}

	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		video_path_param.in_func = 0;
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}

	{
		HD_VIDEOPROC_CTRL video_ctrl_param = {0};

		video_ctrl_param.func = VIDEOPROC_ALG_FUNC;
		video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_1_OUT_0;
		if (frame_num >= 2) {
			video_ctrl_param.func |= HD_VIDEOPROC_FUNC_SHDR;
		} else {
			video_ctrl_param.func &= ~HD_VIDEOPROC_FUNC_SHDR;
		}

		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);
	}

	*p_video_proc_ctrl = video_proc_ctrl;

	return ret;
}

static HD_RESULT plug_pq_nnsc_set_proc_param(HD_PATH_ID video_proc_path, HD_DIM* p_dim)
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

		video_path_param.out_func |= HD_VIDEOPROC_OUTFUNC_MD;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}

	return ret;
}

static HD_RESULT plug_pq_nnsc_set_enc_cfg(HD_PATH_ID video_enc_path, HD_DIM *p_max_dim, UINT32 max_bitrate, UINT32 isp_id)
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

static HD_RESULT plug_pq_nnsc_set_enc_param(HD_PATH_ID video_enc_path, HD_DIM *p_dim, UINT32 enc_type, UINT32 bitrate)
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
#endif

static HD_RESULT plug_pq_nnsc_init_module(void)
{
	HD_RESULT ret = HD_OK;

	if ((ret = hd_videocap_init()) != HD_OK) {
		printf("hd_videocap_init fail (%d) \n", ret);
		return ret;
	}
	if ((ret = hd_videoproc_init()) != HD_OK) {
		printf("hd_videoproc_init fail (%d) \n", ret);
		return ret;
	}
	if ((ret = hd_videoenc_init()) != HD_OK) {
		printf("hd_videoenc_init fail (%d) \n", ret);
		return ret;
	}

	return ret;
}

static HD_RESULT plug_pq_nnsc_exit_module(void)
{
	HD_RESULT ret = HD_OK;

	ret = hd_videocap_uninit();
	if (ret != HD_OK) {
		printf("hd_videocap_uninit fail (%d) \n", ret);
		return ret;
	}
	ret = hd_videoproc_uninit();
	if (ret != HD_OK) {
		printf("hd_videoproc_uninit fail (%d) \n", ret);
		return ret;
	}
	ret = hd_videoenc_uninit();
	if (ret != HD_OK) {
		printf("hd_videoenc_uninit fail (%d) \n", ret);
		return ret;
	}

	return ret;
}

static HD_RESULT plug_pq_nnsc_open_module(VIDEO_PLUG_STREAM *p_stream, HD_DIM *proc_max_dim)
{
	HD_RESULT ret = HD_OK;

	#if (OSG_FUNC_EN)
	ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_STAMP_0, &stream_osg.stamp_path);
	if (ret != HD_OK) {
		printf("hd_videoenc_open fail (%d) \n", ret);
		return ret;
	}
	#endif

	#if (EXTEND_YUV_EN)
	ret = plug_pq_nnsc_set_proc_cfg(&p_stream->proc_path, proc_max_dim, HD_VIDEOPROC_1_CTRL);
	if (ret != HD_OK) {
		printf("set proc-cfg fail = %d \n", ret);
		return ret;
	}

	ret = hd_videoproc_open(HD_VIDEOPROC_1_IN_0, HD_VIDEOPROC_1_OUT_0, &p_stream->proc_path);
	if (ret != HD_OK) {
		printf("hd_videoproc_open fail (%d) \n", ret);
		return ret;
	}
	ret = hd_videoenc_open(HD_VIDEOENC_0_IN_1, HD_VIDEOENC_0_OUT_1, &p_stream->enc_path);
	if (ret != HD_OK) {
		printf("hd_videoenc_open fail (%d) \n", ret);
		return ret;
	}
	#endif

	return ret;
}

static HD_RESULT plug_pq_nnsc_close_module(VIDEO_PLUG_STREAM *p_stream)
{
	HD_RESULT ret = HD_OK;

	#if (OSG_FUNC_EN | EXTEND_YUV_EN)
	ret = hd_videoenc_close(stream_osg.stamp_path);
	if (ret != HD_OK) {
		printf("hd_videoenc_close fail (%d) \n", ret);
		return ret;
	}
	#endif

	#if (EXTEND_YUV_EN)
	ret = hd_videoproc_close(p_stream->proc_path);
	if (ret != HD_OK) {
		printf("hd_videoproc_close fail (%d) \n", ret);
		return ret;
	}
	ret = hd_videoenc_close(p_stream->enc_path);
	if (ret != HD_OK) {
		printf("hd_videoenc_close fail (%d) \n", ret);
		return ret;
	}
	#endif

	return ret;
}
#endif

static HD_RESULT NNSCnet_open_model_label(NET_PATH_ID net_path);
static HD_RESULT NNSCnet_close(NET_PATH_ID net_path);
static HD_RESULT NNSCnet_get_mem(MEM_PARM *mem_parm, UINT32 size);
static HD_RESULT NNSCnet_rel_mem(MEM_PARM *mem_parm);
static HD_RESULT NNSCnet_alloc_mem(MEM_PARM *mem_parm, CHAR* name, UINT32 size);
static HD_RESULT NNSCnet_free_mem(MEM_PARM *mem_parm);

static HD_RESULT NNSCnet_init_input(void)
{
	HD_RESULT ret = HD_OK;
	int  i;
	
	for (i = 0; i < 16; i++) {
		NET_IN* p_net = g_in + i;
		p_net->in_id = i;
	}
	return ret;
}

static HD_RESULT NNSCnet_uninit_input(void)
{
	HD_RESULT ret = HD_OK;
	return ret;
}

static HD_RESULT NNSCnet_set_input_config(NET_PATH_ID net_path, NET_IN_CONFIG* p_in_cfg)
{
	HD_RESULT ret = HD_OK;
	NET_IN* p_net = g_in + net_path;
	UINT32 proc_id = p_net->in_id;
	
	memcpy((void*)&p_net->in_cfg, (void*)p_in_cfg, sizeof(NET_IN_CONFIG));
	printf("proc_id(%u) set in_cfg: buf=(%u,%u,%u,%u,%08x)\r\n", 
		proc_id,
		p_net->in_cfg.w,
		p_net->in_cfg.h,
		p_net->in_cfg.c,
		p_net->in_cfg.loff,
		p_net->in_cfg.fmt);
	
	return ret;
}

static HD_RESULT NNSCnet_open_input(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	return ret;
}

static HD_RESULT NNSCnet_close_input(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	return ret;
}

static HD_RESULT NNSCnet_start_input(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	return ret;
}

static HD_RESULT NNSCnet_stop_input(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	return ret;
}

static HD_RESULT NNSCnet_init_engine(void)
{
	HD_RESULT ret = HD_OK;
	int i;
	
	// config extend engine plugin, process scheduler
	{
		UINT32 schd = VENDOR_AI_PROC_SCHD_FAIR;
		vendor_ai_cfg_set(VENDOR_AI_CFG_PLUGIN_ENGINE, vendor_ai_cpu1_get_engine());
		printf("vendor_ai_cfg_set(VENDOR_AI_CFG_PLUGIN_ENGINE..\r\n");

		vendor_ai_cfg_set(VENDOR_AI_CFG_PROC_SCHD, &schd);
		printf("vendor_ai_cfg_set(VENDOR_AI_CFG_PROC_SCHD..\r\n");

	}

	ret = vendor_ai_init();
	if (ret != HD_OK) {
		printf("vendor_ai_init fail (%d) \r\n", ret);
		return ret;
	}

	for (i = 0; i < 16; i++) {
		NET_PROC* p_net = g_net + i;
		p_net->proc_id = i;
	}
	return ret;
}

static HD_RESULT NNSCnet_uninit_engine(void)
{
	HD_RESULT ret;
	if ((ret =vendor_ai_uninit()) != HD_OK) {
		printf("vendor_ai_uninit fail (%d) \r\n", ret);
		return ret;
	}
	return HD_OK;
}

static HD_RESULT NNSCnet_init_module(void)
{
	HD_RESULT ret;
	if ((ret = NNSCnet_init_input()) != HD_OK)
		return ret;
	if ((ret = NNSCnet_init_engine()) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT NNSCnet_uninit_module(void)
{
	HD_RESULT ret;
	if ((ret = NNSCnet_uninit_input()) != HD_OK)
		return ret;
	if ((ret = NNSCnet_uninit_engine()) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT NNSCnet_open_module(VIDEO_NN *p_stream)
{
	HD_RESULT ret;
	if ((ret = NNSCnet_open_input(p_stream->in_path)) != HD_OK)
		return ret;
	// load model
	if ((ret = NNSCnet_open_model_label(p_stream->net_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT NNSCnet_close_module(VIDEO_NN *p_stream)
{
	HD_RESULT ret;
	if ((ret = NNSCnet_close_input(p_stream->in_path)) != HD_OK)
		return ret;
	if ((ret = NNSCnet_close(p_stream->net_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static UINT32 NNSCnet_load_model(CHAR *filename, ULONG va)
{
	FILE  *fd;
	UINT32 file_size = 0, read_size = 0;
	const ULONG model_addr = va;

	fd = fopen(filename, "rb");
	if (!fd) {
		printf("load model(%s) fail \r\n", filename);
		return 0;
	}

	fseek(fd, 0, SEEK_END);
	file_size = ALIGN_CEIL_4(ftell(fd));
	fseek (fd, 0, SEEK_SET);

	read_size = fread((void *)model_addr, 1, file_size, fd);
	if (read_size != file_size) {
		printf("size mismatch, real = %d, idea = %d \r\n", (int)read_size, (int)file_size);
	}
	fclose(fd);

	printf("load model (%s) ok\r\n", filename);

	return read_size;
}

static HD_RESULT NNSCnet_load_label(ULONG addr, UINT32 line_len, const CHAR *filename)
{
	FILE *fd;
	CHAR *p_line = (CHAR *)addr;

	fd = fopen(filename, "r");
	if (!fd) {
		printf("load label(%s) fail \r\n", filename);
		return HD_ERR_NG;
	}

	while (fgets(p_line, line_len, fd) != NULL) {
		p_line[strlen(p_line) - 1] = '\0'; // remove newline character
		p_line += line_len;
	}

	if (fd) {
		fclose(fd);
	}

	printf("load label(%s) ok\r\n", filename);

	return HD_OK;
}

static HD_RESULT NNSCnet_set_config(NET_PATH_ID net_path, NET_PROC_CONFIG* p_proc_cfg)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id;
	p_net->proc_id = net_path;
	proc_id = p_net->proc_id;

	memcpy((void*)&p_net->net_cfg, (void*)p_proc_cfg, sizeof(NET_PROC_CONFIG));

	printf("proc_id(%u) set net_cfg: job-opt=(%u,%d), buf-opt(%u), binsize = %d \r\n", 
		proc_id,
		p_net->net_cfg.job_method,
		(int)p_net->net_cfg.job_wait_ms,
		p_net->net_cfg.buf_method,
		p_net->net_cfg.binsize);
	
	// set buf opt
	{
		VENDOR_AI_NET_CFG_BUF_OPT cfg_buf_opt = {0};
		cfg_buf_opt.method = p_net->net_cfg.buf_method;
		cfg_buf_opt.ddr_id = DDR_ID0;
		vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_BUF_OPT, &cfg_buf_opt);
	}

	// set job opt
	{
		VENDOR_AI_NET_CFG_JOB_OPT cfg_job_opt = {0};
		cfg_job_opt.method = p_net->net_cfg.job_method;
		cfg_job_opt.wait_ms = p_net->net_cfg.job_wait_ms;
		cfg_job_opt.schd_parm = VENDOR_AI_FAIR_CORE_ALL; //FAIR dispatch to ALL core
		vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_JOB_OPT, &cfg_job_opt);
	}

	return ret;
}

static HD_RESULT NNSCnet_alloc_io_buf(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id = p_net->proc_id;
	VENDOR_AI_NET_CFG_WORKBUF wbuf = {0};

	ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_CFG_WORKBUF, &wbuf);
	if (ret != HD_OK) {
		printf("proc_id(%lu) get VENDOR_AI_NET_PARAM_CFG_WORKBUF fail \r\n", proc_id);
		return HD_ERR_FAIL;
	}
	ret = NNSCnet_alloc_mem(&p_net->io_mem, "ai_io_buf", wbuf.size);
	if (ret != HD_OK) {
		printf("proc_id(%lu) alloc ai_io_buf fail \r\n", proc_id);
		return HD_ERR_FAIL;
	}

	wbuf.pa = p_net->io_mem.pa;
	wbuf.va = p_net->io_mem.va;
	wbuf.size = p_net->io_mem.size;
	ret = vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_WORKBUF, &wbuf);
	if (ret != HD_OK) {
		printf("proc_id(%lu) set VENDOR_AI_NET_PARAM_CFG_WORKBUF fail \r\n", proc_id);
		return HD_ERR_FAIL;
	}

	printf("alloc_io_buf: work buf, pa = %#lx, va = %#lx, size = %lu \r\n", wbuf.pa, wbuf.va, wbuf.size);

	return ret;
}

static HD_RESULT NNSCnet_free_io_buf(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	
	NET_PROC* p_net = g_net + net_path;
	
	if (p_net->io_mem.pa && p_net->io_mem.va) {
		NNSCnet_free_mem(&p_net->io_mem);
	}
	
	return ret;
}

static HD_RESULT NNSCnet_open_model_label(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id;
	p_net->proc_id = net_path;
	proc_id = p_net->proc_id;

	UINT32 loadsize = 0;

	if (strlen(p_net->net_cfg.model_filename) == 0) {
		printf("proc_id(%u) input model is null\r\n", proc_id);
		return 0;
	}

	NNSCnet_get_mem(&p_net->proc_mem, p_net->net_cfg.binsize);
	//load model BIN
	loadsize = NNSCnet_load_model(p_net->net_cfg.model_filename, p_net->proc_mem.va);
	if (loadsize <= 0) {
		printf("proc_id (%u) input model load fail: %s\r\n", proc_id, p_net->net_cfg.model_filename);
		return 0;
	}

	// load label
	ret = NNSCnet_load_label((ULONG)p_net->out_class_labels, VENDOR_AIS_LBL_LEN, p_net->net_cfg.label_filename);
	if (ret != HD_OK) {
		printf("proc_id (%u), load_label (%d) \r\n", proc_id, ret);
		return HD_ERR_FAIL;
	}

	// set model
	vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_MODEL, (VENDOR_AI_NET_CFG_MODEL*)&p_net->proc_mem);

	// open
	vendor_ai_net_open(proc_id);

	return ret;
}

static HD_RESULT NNSCnet_close(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id = p_net->proc_id;

	if ((ret = NNSCnet_free_io_buf(net_path)) != HD_OK) {
		return ret;
	}
	
	// close
	ret = vendor_ai_net_close(proc_id);
	
	NNSCnet_rel_mem(&p_net->proc_mem);

	return ret;
}

static HD_RESULT NNSCnet_get_mem(MEM_PARM *mem_parm, UINT32 size)
{
	HD_RESULT ret = HD_OK;
	ULONG pa = 0;
	void  *va = NULL;
	HD_COMMON_MEM_VB_BLK blk;

	blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_DEFINIED_POOL, size, DDR_ID0);
	if (HD_COMMON_MEM_VB_INVALID_BLK == blk) {
		printf("hd_common_mem_get_block fail \r\n");
		return HD_ERR_NG;
	}
	pa = hd_common_mem_blk2pa(blk);
	if (pa == 0) {
		printf("not get buffer, pa=%08x\r\n", (int)pa);
		return HD_ERR_NOMEM;
	}
	va = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, size);

	/* Release buffer if fail*/
	if (va == 0) {
		ret = hd_common_mem_munmap(va, size);
		if (ret != HD_OK) {
			printf("mem unmap fail \r\n");
			return ret;
		}
	}

	mem_parm->pa = pa;
	mem_parm->va = (ULONG)va;
	mem_parm->size = size;
	mem_parm->blk = blk;

	return HD_OK;
}

static HD_RESULT NNSCnet_rel_mem(MEM_PARM *mem_parm)
{
	HD_RESULT ret = HD_OK;

	/* Release in buffer */
	if (mem_parm->va) {
		ret = hd_common_mem_munmap((void *)mem_parm->va, mem_parm->size);
		if (ret != HD_OK) {
			printf("mem_uninit : (mem_parm->va)hd_common_mem_munmap fail.\r\n");
			return ret;
		}
	}
	ret = hd_common_mem_release_block(mem_parm->blk);
	if (ret != HD_OK) {
		printf("mem_uninit : (mem_parm->pa)hd_common_mem_release_block fail.\r\n");
		return ret;
	}

	mem_parm->pa = 0;
	mem_parm->va = 0;
	mem_parm->size = 0;
	mem_parm->blk = (UINT32)-1;

	return HD_OK;
}

static HD_RESULT NNSCnet_alloc_mem(MEM_PARM *mem_parm, CHAR* name, UINT32 size)
{
	HD_RESULT ret = HD_OK;
	ULONG pa   = 0;
	void  *va   = NULL;

	//alloc private pool
	ret = hd_common_mem_alloc(name, &pa, (void**)&va, size, DDR_ID0);
	if (ret!= HD_OK) {
		return ret;
	}

	mem_parm->pa   = pa;
	mem_parm->va   = (ULONG)va;
	mem_parm->size = size;
	mem_parm->blk  = -1;

	return HD_OK;
}

static HD_RESULT NNSCnet_free_mem(MEM_PARM *mem_parm)
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
	mem_parm->blk = -1;

	return HD_OK;
}

static HD_RESULT NNSCnet_start(VIDEO_NN *p_stream)
{
	HD_RESULT ret = HD_OK;

	ret = vendor_ai_net_start(p_stream->net_path);
	if (HD_OK != ret) {
		printf("proc_id(%u) start fail !!\r\n", p_stream->net_path);
	}
	
	return ret;
}

static HD_RESULT NNSCnet_stop(VIDEO_NN *p_stream)
{
	HD_RESULT ret = HD_OK;
	
	//stop: should be call after last time proc
	ret = vendor_ai_net_stop(p_stream->net_path);
	if (HD_OK != ret) {
		printf("proc_id (%u) stop fail \n", p_stream->net_path);
	}
	
	return ret;
}

static HD_RESULT NNSCnet_get_ai_outlayer_list(UINT32 proc_id, UINT32 *outlayer_num, UINT32 *outlayer_path_list)
{
	HD_RESULT ret = HD_OK;
	VENDOR_AI_NET_INFO net_info = {0};

	// get output layer number
	ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_INFO, &net_info);
	if (HD_OK != ret) {
		printf("proc_id (%lu) get info fail \n", proc_id);
		return ret;
	}

	*outlayer_num = net_info.out_buf_cnt;

	// get path_list
	ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_OUT_PATH_LIST, outlayer_path_list);
	if (HD_OK != ret) {
		printf("proc_id(%u) get outlayer_path_list fail \n", proc_id);
	}

	return ret;
}

static HD_RESULT NNSCnet_get_ai_outlayer_by_path_id(INT32 proc_id, UINT32 path_id, VENDOR_AI_BUF *p_outbuf)
{
	HD_RESULT ret = HD_OK;
	// get out buf by path_id
	ret = vendor_ai_net_get(proc_id, path_id, p_outbuf);
	if (HD_OK != ret) {
		printf("proc_id(%u) get AI_OUTBUF fail \n", proc_id);
		return ret;
	}

	return ret;
}

static HD_RESULT NNSCnet_assign_frame(NET_PATH_ID net_path, UINT32 pa, UINT32 va, UINT32 size)
{
	HD_RESULT ret = HD_OK;
	NET_IN* p_net = g_in + net_path;
	p_net->input_mem.pa = pa;
	p_net->input_mem.va = va;
	p_net->input_mem.size = size;
	p_net->src_img.width = p_net->in_cfg.w;
	p_net->src_img.height = p_net->in_cfg.h;
	p_net->src_img.channel = p_net->in_cfg.c;
	p_net->src_img.line_ofs= p_net->in_cfg.loff;
	p_net->src_img.fmt= p_net->in_cfg.fmt;
	
	p_net->src_img.pa = pa;
	p_net->src_img.va = va;
	p_net->src_img.sign = MAKEFOURCC('A','B','U','F');
	p_net->src_img.size = p_net->in_cfg.loff * p_net->in_cfg.h * 3 / 2;

	// set net input image
	ret = vendor_ai_net_set(net_path, VENDOR_AI_NET_PARAM_IN(0, 0), (void *)&(p_net->src_img));
	if (HD_OK != ret) {
		printf("proc_id (%u) push input fail \n", net_path);
		return ret;
	}

	return HD_OK;
}
#endif

///////////////////////////////////////////////////////////////////////////////
static INT32 get_choose_int(void)
{
	char buf[256];
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

static void *access_nn_thread(void *arg)
{
	VIDEO_PLUG_STREAM* p_stream = (VIDEO_PLUG_STREAM *)arg;
	HD_VIDEO_FRAME video_frame = {0};
	HD_RESULT ret = HD_OK;
	ISPT_WAIT_VD wait_vd = {0};
	UINT32 new_shdr_tone_lv;
	static UINT32 pre_shdr_tone_lv = 50;
	UINT32 i;
	#if (SAVE_YUV_EN || ACCESS_NNSC_NET_EN)
	ULONG phy_addr_yuv, vir_addr_yuv;
	UINT32 yuv_size;
	#define PHY2VIRT_YUV(pa) (vir_addr_yuv + ((pa) - phy_addr_yuv))
	#endif
	static UINT32 pre_exp_ratio = 65535;

	#if (ACCESS_NNSC_NET_EN)
	ISPT_CA_DATA isp_ca = {0};
	INT rt = 0;
	#endif

	#if (SAVE_YUV_EN)
	CHAR save_yuv_path[100];
	FILE *fp = NULL;
	static UINT32 save_yuv_cnt = 0;
	#endif

	wait_vd.id = VCAP_ID_PATH_1;
	wait_vd.timeout = 100;

	while (nnsc_thread_run) {
		for (i = 0; i < CALC_NNSC_DURATION; i++) {
			vendor_isp_get_common(ISPT_ITEM_WAIT_VD, &wait_vd);
		}

		#if (EXTEND_YUV_EN)
		ret = hd_videoproc_pull_out_buf(p_stream[nnsc_yuv_src].proc_path, &video_frame, -1);
		#else
		ret = hd_videoproc_pull_out_buf(p_stream[0].proc_path, &video_frame, -1);
		#endif
		if (ret != HD_OK) {
			if (ret != HD_ERR_UNDERRUN)
				printf("proc_pull fial (%d) \r\n", ret);
			return 0;
		}

		// NOTE: access nnsc net here
		#if (ACCESS_NNSC_NET_EN)
		phy_addr_yuv = hd_common_mem_blk2pa(video_frame.blk); // Get physical addr
		if (phy_addr_yuv == 0) {
			printf("blk2pa fail, blk = 0x%x \n", video_frame.blk);
			goto skip;
		}
		yuv_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(video_frame.pw[0], video_frame.ph[0], HD_VIDEO_PXLFMT_YUV420);
		vir_addr_yuv = (ULONG)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_yuv, yuv_size);
		if (vir_addr_yuv == 0) {
			printf("nnsc net, mmap fail. \n");
			goto skip;
		}

		ret = NNSCnet_assign_frame(stream_nn.net_path, phy_addr_yuv + DBGINFO_BUFSIZE(), PHY2VIRT_YUV(video_frame.phy_addr[0]), video_frame.loff[0]*video_frame.ph[0] + video_frame.loff[1]*video_frame.ph[1]);
		if (ret != HD_OK) {
			printf("NNSCnet_assign_frame fail, path = %u \n", stream_nn.net_path);
			goto skip;
		}

		// do net proc
		ret = vendor_ai_net_proc(stream_nn.net_path);
		if (ret != HD_OK) {
			printf("vendor_ai_net_proc, path = %u \n", stream_nn.net_path);
			goto skip;
		}

		// get net result
		NNSCnet_get_ai_outlayer_by_path_id(stream_nn.net_path, outlayer_path_list[0], &outlayer_buf_va);
		hd_common_mem_flush_cache((VOID *)(outlayer_buf_va.va), outlayer_buf_va.size);
		ret = vendor_ai_cpu_util_fixed2float((VOID *)(outlayer_buf_va.va), outlayer_buf_va.fmt, softmax_loc_layer_float_va, outlayer_buf_va.scale_ratio, outlayer_length);
		nnsc_score_num = outlayer_buf_va.channel;
		if (nnsc_score_num == NNSC_SCORE_MAX_NUM) {
			for(UINT32 f_cnt = 0; f_cnt < nnsc_score_num; f_cnt++) {
				nnsc_score.score[f_cnt] = (UINT32)((softmax_loc_layer_float_va[f_cnt]* 100 + 0.5));
				if (nnsc_net_msg_en) {
					if (f_cnt == 0) printf("score[%2u] = %3d - backlight_human \n", f_cnt, nnsc_score.score[f_cnt]);
					else if (f_cnt == 1) printf("score[%2u] = %3d - backlight_scene \n", f_cnt, nnsc_score.score[f_cnt]);
					else if (f_cnt == 2) printf("score[%2u] = %3d - foggy \n", f_cnt, nnsc_score.score[f_cnt]);
					else if (f_cnt == 3) printf("score[%2u] = %3d - grass \n", f_cnt, nnsc_score.score[f_cnt]);
					else if (f_cnt == 4) printf("score[%2u] = %3d - grass_landscape \n", f_cnt, nnsc_score.score[f_cnt]);
					else if (f_cnt == 5) printf("score[%2u] = %3d - hand_color \n", f_cnt, nnsc_score.score[f_cnt]);
					else if (f_cnt == 6) printf("score[%2u] = %3d - normal \n", f_cnt, nnsc_score.score[f_cnt]);
					else if (f_cnt == 7) printf("score[%2u] = %3d - snowfield \n", f_cnt, nnsc_score.score[f_cnt]);
					else printf("score[%u] = %d - unknown\n", f_cnt, nnsc_score.score[f_cnt]);
				}
			}
			nnsc_set_score(&nnsc_score);
		} else {
			printf("score number mismatch. NN (%d) and NNSC lib (%d)\r\n", nnsc_score_num, NNSC_SCORE_MAX_NUM);
		}

		// mummap for frame buffer
		ret = hd_common_mem_munmap((void *)vir_addr_yuv, yuv_size);
		if (ret != HD_OK) {
			printf("nnsc net, mnumap fail. \n");
			return 0;
		}
		#endif

		// NOTE: access nnsc lib here
		#if (ACCESS_NNSC_NET_EN)
		if (nnsc_lib_func_en == TRUE) {
			isp_ca.id = VCAP_ID_PATH_1;
			vendor_isp_get_common(ISPT_ITEM_CA_DATA, &isp_ca);
			memcpy(scd_statistic.ca_r, isp_ca.ca_rslt.r, sizeof(UINT16) * NNSC_CA_MAX_WINNUM);
			memcpy(scd_statistic.ca_g, isp_ca.ca_rslt.g, sizeof(UINT16) * NNSC_CA_MAX_WINNUM);
			memcpy(scd_statistic.ca_b, isp_ca.ca_rslt.b, sizeof(UINT16) * NNSC_CA_MAX_WINNUM);
			nnsc_set_scd_statistic(&scd_statistic);

			rt = nnsc_adj_cal(&nnsc_adj);
			if ((rt != HD_OK) && (rt != HD_ERR_ABORT)) {
				if (nnsc_lib_msg_type == NNSC_DBG_TYPE_MAIN) {
					printf("nnsc_adj_cal fail=%d !! \r\n", rt);
					printf("NNSC score :\r\n");
					#if (ACCESS_NNSC_NET_EN)
					printf("  NNSC_SCORE_BACKLIGHT_HUMAN =      %3d \r\n", nnsc_score.score[NNSC_SCORE_BACKLIGHT_HUMAN]);
					printf("  NNSC_SCORE_BACKLIGHT =            %3d \r\n", nnsc_score.score[NNSC_SCORE_BACKLIGHT]);
					printf("  NNSC_SCORE_FOGGY =                %3d \r\n", nnsc_score.score[NNSC_SCORE_FOGGY]);
					printf("  NNSC_SCORE_GRASS_A =              %3d \r\n", nnsc_score.score[NNSC_SCORE_GRASS_A]);
					printf("  NNSC_SCORE_GRASS_B =              %3d \r\n", nnsc_score.score[NNSC_SCORE_GRASS_B]);
					printf("  NNSC_SCORE_HAND_COLOR =           %3d \r\n", nnsc_score.score[NNSC_SCORE_HAND_COLOR]);
					printf("  NNSC_SCORE_SNOWFIELD =            %3d \r\n", nnsc_score.score[NNSC_SCORE_SNOWFIELD]);
					#else
					printf("  ACCESS_NNSC_NET_EN 0, no nnsc_score \r\n");
					#endif
			
					printf("NNSC param :\r\n");
					printf("  NNSC_TYPE_BACKLIGHT =       {%3d, %3d} \r\n", nnsc_param.param[NNSC_TYPE_BACKLIGHT].sensitive, nnsc_param.param[NNSC_TYPE_BACKLIGHT].adj_strength);
					printf("  NNSC_TYPE_FOGGY =           {%3d, %3d} \r\n", nnsc_param.param[NNSC_TYPE_FOGGY].sensitive, nnsc_param.param[NNSC_TYPE_FOGGY].adj_strength);
					printf("  NNSC_TYPE_GRASS =           {%3d, %3d} \r\n", nnsc_param.param[NNSC_TYPE_GRASS].sensitive, nnsc_param.param[NNSC_TYPE_GRASS].adj_strength);
					printf("  NNSC_TYPE_SKIN_COLOR        {%3d, %3d} \r\n", nnsc_param.param[NNSC_TYPE_SKIN_COLOR].sensitive, nnsc_param.param[NNSC_TYPE_SKIN_COLOR].adj_strength);
					printf("  NNSC_TYPE_SNOWFIELD =       {%3d, %3d} \r\n", nnsc_param.param[NNSC_TYPE_SNOWFIELD].sensitive, nnsc_param.param[NNSC_TYPE_SNOWFIELD].adj_strength);
				}
			} else {
				if ((nnsc_lib_msg_type == NNSC_DBG_TYPE_MAIN) && (rt == HD_OK)) {
					printf("NNSC output :\r\n");
					printf("NNSC_ADJ_EXP_ENH_RATIO =      %3d \r\n", nnsc_adj.exp_ratio);
					printf("NNSC_ADJ_GREEN_REMOVE_RATIO = %3d \r\n", nnsc_adj.green_remove);
					printf("NNSC_ADJ_SKIN_REMOVE_RATIO =  %3d \r\n", nnsc_adj.skin_remove);
					printf("NNSC_ADJ_DARK_ENH_RATIO =     %3d \r\n", nnsc_adj.dark_enh_ratio);
					printf("NNSC_ADJ_CONTRAST_ENH_RATIO = %3d \r\n", nnsc_adj.contrast_enh_ratio);
					printf("NNSC_ADJ_GREEN_ENH_RATIO =    %3d \r\n", nnsc_adj.green_enh_ratio);
					printf("NNSC_ADJ_SKIN_ENH_RATIO =     %3d \r\n", nnsc_adj.skin_enh_ratio);
				}

				exp_ratio.id = VCAP_ID_PATH_1;
				exp_ratio.ratio = nnsc_adj.exp_ratio;
				
				if (pre_exp_ratio != exp_ratio.ratio) {
					vendor_isp_set_ae(AET_ITEM_NNSC_EXP_RATIO, &exp_ratio);
					pre_exp_ratio = exp_ratio.ratio;
				}
				
				green_remove.id = VCAP_ID_PATH_1;
				green_remove.enable = nnsc_adj.green_remove;
				vendor_isp_set_awb(AWBT_ITEM_NNSC_GREEN_REMOVE, &green_remove);
				skin_remove.id = VCAP_ID_PATH_1;
				skin_remove.enable = nnsc_adj.skin_remove;
				vendor_isp_set_awb(AWBT_ITEM_NNSC_SKIN_REMOVE, &skin_remove);
				dark_enh_ratio.id = VCAP_ID_PATH_1;
				dark_enh_ratio.ratio = nnsc_adj.dark_enh_ratio;
				vendor_isp_set_iq(IQT_ITEM_NNSC_DARK_ENH_RATIO, &dark_enh_ratio);
				contrast_enh_ratio.id = VCAP_ID_PATH_1;
				contrast_enh_ratio.ratio = nnsc_adj.contrast_enh_ratio;
				vendor_isp_set_iq(IQT_ITEM_NNSC_CONTRAST_ENH_RATIO, &contrast_enh_ratio);
				green_enh_ratio.id = VCAP_ID_PATH_1;
				green_enh_ratio.ratio = nnsc_adj.green_enh_ratio;
				vendor_isp_set_iq(IQT_ITEM_NNSC_GREEN_ENH_RATIO, &green_enh_ratio);
				skin_enh_ratio.id = VCAP_ID_PATH_1;
				skin_enh_ratio.ratio = nnsc_adj.skin_enh_ratio;
				vendor_isp_set_iq(IQT_ITEM_NNSC_SKIN_ENH_RATIO, &skin_enh_ratio);

				new_shdr_tone_lv = 50 + dark_enh_ratio.ratio / 2;
				if (new_shdr_tone_lv > 100) {
					new_shdr_tone_lv = 100;
				}
				if (pre_shdr_tone_lv <= new_shdr_tone_lv) {
					pre_shdr_tone_lv = (pre_shdr_tone_lv * TONE_LV_SMOOTH_FACTOR + new_shdr_tone_lv * 1 + TONE_LV_SMOOTH_FACTOR) / (TONE_LV_SMOOTH_FACTOR + 1); // Unconditional carry
				} else {
					pre_shdr_tone_lv = (pre_shdr_tone_lv * TONE_LV_SMOOTH_FACTOR + new_shdr_tone_lv * 1) / (TONE_LV_SMOOTH_FACTOR + 1); // Unconditional chop
				}
				shdr_tone_lv.id = VCAP_ID_PATH_1;
				shdr_tone_lv.lv = pre_shdr_tone_lv;
				vendor_isp_set_iq(IQT_ITEM_SHDR_TONE_LV, &shdr_tone_lv);
				if (nnsc_lib_msg_type == NNSC_DBG_TYPE_MAIN) {
					printf("shdr_tone_lv =                %3d \r\n", shdr_tone_lv.lv);
				}
			}
		} else {
			exp_ratio.id = VCAP_ID_PATH_1;
			exp_ratio.ratio = 0;
			if (pre_exp_ratio != exp_ratio.ratio) {
				vendor_isp_set_ae(AET_ITEM_NNSC_EXP_RATIO, &exp_ratio);
				pre_exp_ratio = exp_ratio.ratio;
			}
			green_remove.id = VCAP_ID_PATH_1;
			green_remove.enable = 0;
			vendor_isp_set_awb(AWBT_ITEM_NNSC_GREEN_REMOVE, &green_remove);
			skin_remove.id = VCAP_ID_PATH_1;
			skin_remove.enable = 0;
			vendor_isp_set_awb(AWBT_ITEM_NNSC_SKIN_REMOVE, &skin_remove);
			dark_enh_ratio.id = VCAP_ID_PATH_1;
			dark_enh_ratio.ratio = 0;
			vendor_isp_set_iq(IQT_ITEM_NNSC_DARK_ENH_RATIO, &dark_enh_ratio);
			contrast_enh_ratio.id = VCAP_ID_PATH_1;
			contrast_enh_ratio.ratio = 0;
			vendor_isp_set_iq(IQT_ITEM_NNSC_CONTRAST_ENH_RATIO, &contrast_enh_ratio);
			green_enh_ratio.id = VCAP_ID_PATH_1;
			green_enh_ratio.ratio = 0;
			vendor_isp_set_iq(IQT_ITEM_NNSC_GREEN_ENH_RATIO, &green_enh_ratio);
			skin_enh_ratio.id = VCAP_ID_PATH_1;
			skin_enh_ratio.ratio = 50;
			vendor_isp_set_iq(IQT_ITEM_NNSC_SKIN_ENH_RATIO, &skin_enh_ratio);

			shdr_tone_lv.id = VCAP_ID_PATH_1;
			shdr_tone_lv.lv = 50;
			vendor_isp_set_iq(IQT_ITEM_SHDR_TONE_LV, &shdr_tone_lv);
			pre_shdr_tone_lv = 50;
		}
		#endif

		#if (OSG_FUNC_EN)
		for(i = 0; i < NNSC_SCORE_MAX_NUM; i++) {
			#if (ACCESS_NNSC_NET_EN)
			osg_update_info.value[i] = nnsc_score.score[i];
			#else
			osg_update_info.value[i] = 0;
			#endif
		}
		#endif

		// NOTE: save yuv here
		#if (SAVE_YUV_EN)
		save_yuv_cnt++;
		printf("cnt = %d, w = %d, h = %d, loff = %d \n", save_yuv_cnt, video_frame.pw[0], video_frame.ph[0], video_frame.loff[0]);
		phy_addr_yuv = hd_common_mem_blk2pa(video_frame.blk); // Get physical addr
		if (phy_addr_yuv == 0) {
			printf("blk2pa fail, blk = 0x%x\r\n", video_frame.blk);
			return 0;
		}
		yuv_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(video_frame.pw[0], video_frame.ph[0], HD_VIDEO_PXLFMT_YUV420);
		vir_addr_yuv = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_yuv, yuv_size);
		if (vir_addr_yuv == 0) {
			printf("save yuv, mmap fail. \n");
			return 0;
		}

		sprintf(save_yuv_path, "/mnt/sd/save_%d_yuv420_w%d_h%d.yuv", (int)save_yuv_cnt, (int)video_frame.loff[0], (int)video_frame.ph[0]);
		fp = fopen(save_yuv_path, "wb");
		if (fp == NULL) {
			printf("fail to open %s \n", save_yuv_path);
			return 0;
		}

		//save Y plane
		{
			UINT8 *ptr = (UINT8 *)PHY2VIRT_YUV(video_frame.phy_addr[0]);
			UINT32 len = video_frame.loff[0]*video_frame.ph[0];
			if (fp) fwrite(ptr, 1, len, fp);
			if (fp) fflush(fp);
		}
		//save UV plane
		{
			UINT8 *ptr = (UINT8 *)PHY2VIRT_YUV(video_frame.phy_addr[1]);
			UINT32 len = video_frame.loff[1]*video_frame.ph[1];
			if (fp) fwrite(ptr, 1, len, fp);
			if (fp) fflush(fp);
		}

		fclose(fp);

		// mummap for frame buffer
		ret = hd_common_mem_munmap((void *)vir_addr_yuv, yuv_size);
		if (ret != HD_OK) {
			printf("save yuv, mnumap fail. \n");
			return 0;
		}
		#endif

		#if (EXTEND_YUV_EN)
		ret = hd_videoproc_release_out_buf(p_stream[nnsc_yuv_src].proc_path, &video_frame);
		#else
		ret = hd_videoproc_release_out_buf(p_stream[0].proc_path, &video_frame);
		#endif
		if (ret != HD_OK) {
				printf("proc_release fail (%d) \r\n\r\n", ret);
			return 0;
		}
	}

	#if (ACCESS_NNSC_NET_EN)
skip:
	hd_common_mem_free(outlayer_buf_pa, &outlayer_buf_va);
	hd_common_mem_free(softmax_loc_layer_float_pa, softmax_loc_layer_float_va);
	#endif

	#if (SAVE_YUV_EN)
	save_yuv_cnt = 0;
	#endif

	return 0;
}

#if (EXTEND_YUV_EN)
static void *access_extend_yuv_thread(void *arg)
{
	VIDEO_PLUG_STREAM* p_stream = (VIDEO_PLUG_STREAM *)arg;
	HD_VIDEO_FRAME video_frame[2] = {0};
	UINT32 i;
	HD_RESULT ret = HD_OK;

	while (extend_yuv_thread_run) {
		for (i = 0; i <= frame_num; i++) {
			ret = hd_videocap_pull_out_buf(p_stream[0].cap_path, &video_frame[0], -1);
			if (ret != HD_OK) {
				if (ret != HD_ERR_UNDERRUN)
					printf("cap_pull fail (%d) \n", ret);
			}

			ret = hd_videoproc_push_in_buf(p_stream[1].proc_path, &video_frame[0], NULL, -1);
			if (ret != HD_OK) {
				printf("proc_push fail (%d) \n", ret);
			}

			ret = hd_videocap_release_out_buf(p_stream[0].cap_path, &video_frame[0]);
			if (ret != HD_OK) {
				printf("cap_release fial (%d) \n", ret);
			}
		}
	}

	return 0;
}
#endif

MAIN(argc, argv)
{
	HD_RESULT ret;
	INT32 option, option_1;
	UINT32 trig = 1;
	#if (ACCESS_NNSC_NET_EN)
	ISPT_YUV_INFO yuv_info = {0};
	#endif
	UINT32 nnsc_lib_version;
	VIDEO_PLUG_STREAM stream[2] = {0};
	#if (OSG_FUNC_EN)
	HD_VIDEOPROC_OUT video_out_param = {0};
	#endif
	#if (OSG_FUNC_EN | EXTEND_YUV_EN)
	HD_DIM proc_max_dim;
	#endif

	printf("%s \n", (EXTEND_YUV_EN == 1) ? "Using extend yuv" : "Using original yuv");

	if (vendor_isp_init() == HD_ERR_NG) {
		printf("init vendor isp fail \n ");
		return 0;
	}

	hd_common_init(HD_COMMON_CFG_MULTIPROC | 2);
	hd_common_mem_init(NULL);

	stream[0].cap_path =  HD_VIDEOCAP_PATH(HD_DAL_VIDEOCAP(VCAP_ID_PATH_1), HD_IN(0), HD_OUT(0));
	stream[0].proc_path = HD_VIDEOPROC_PATH(HD_DAL_VIDEOPROC(VPRC_ID_PATH_1), HD_IN(0), HD_OUT(0));

	nnsc_net_msg_en =    FALSE;
	nnsc_lib_func_en =   TRUE;
	nnsc_lib_msg_type =  0;

	#if (OSG_FUNC_EN | EXTEND_YUV_EN)
	if (plug_pq_nnsc_init_module() != HD_OK) {
		return 0;
	}

	#if (EXTEND_YUV_EN)
	{
		HD_VIDEOCAP_IN video_in_param = {0};

		hd_videocap_get(stream[0].cap_path, HD_VIDEOCAP_PARAM_IN, &video_in_param);
		frame_num = video_in_param.out_frame_num;
		printf("frame number = 0x%X \n", frame_num);
	}
	#endif

	hd_videoproc_get(stream[0].proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	proc_max_dim.w = video_out_param.dim.w;
	proc_max_dim.h = video_out_param.dim.h;
	vdo_size_w = video_out_param.dim.w;
	vdo_size_h = video_out_param.dim.h;
	printf("frame width = %d, height = %d \n", vdo_size_w, vdo_size_h);

	if (plug_pq_nnsc_open_module(&stream[1], &proc_max_dim) != HD_OK) {
		return 0;
	}
	#endif

	#if (EXTEND_YUV_EN)
	{
		AET_CFG_INFO cfg_info = {0};

		cfg_info.id = VCAP_ID_PATH_1 + 1;

		if (frame_num == 1) {
			snprintf(cfg_info.path, 64, "/mnt/app/isp/%s.cfg", ISP_CFG_PATH);
		} else {
			snprintf(cfg_info.path, 64, "/mnt/app/isp/%s.cfg", ISP_CFG_PATH_HDR);
		}
		printf("load %s \n", cfg_info.path);

		vendor_isp_set_ae(AET_ITEM_RLD_CONFIG, &cfg_info);
		vendor_isp_set_awb(AWBT_ITEM_RLD_CONFIG, &cfg_info);
		vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);
	}

	ret = plug_pq_nnsc_set_proc_param(stream[1].proc_path, &proc_max_dim);
	if (ret != HD_OK) {
		printf("set proc fail = %d \n", ret);
		return 0;
	}

	ret = plug_pq_nnsc_set_enc_cfg(stream[1].enc_path, &proc_max_dim, 4 /*enc_bitrate*/ * 1024 * 1024, ISP_ID_PATH_2);
	if (ret != HD_OK) {
		printf("set enc-cfg fail = %d \n", ret);
		goto exit;
	}

	ret = plug_pq_nnsc_set_enc_param(stream[1].enc_path, &proc_max_dim, 1 /*enc_type*/, 4 /*enc_bitrate*/ * 1024 * 1024);
	if (ret != HD_OK) {
		printf("set enc2 fail = %d \n", ret);
		goto exit;
	}

	hd_videoproc_bind(HD_VIDEOPROC_1_OUT_0, HD_VIDEOENC_0_IN_1);
	hd_videoproc_start(stream[1].proc_path);
	hd_videoenc_start(stream[1].enc_path);

	extend_yuv_thread_run = TRUE;
	ret = pthread_create(&extend_yuv_thread_id, NULL, access_extend_yuv_thread, (void *)&stream[0]);
	if (ret < 0) {
		printf("create access_nn_thread thread failed");
		goto exit;
	}



	#endif

	#if (OSG_FUNC_EN)
	if (osg_init() != HD_OK) {
		printf("osg_init fail \n");
		return 0;
	}
	#endif

	while (trig) {
		printf("----------------------------------------\n");
		printf("   1.  Init & config NNSC net \n");
		printf("   3.  Run NNSC \n");
		printf("   5.  Stop & close NNSC net \n");
		printf("----------------------------------------\n");
		printf("   10. Config NNSC NET message \n");
		printf("----------------------------------------\n");
		printf("   20. Get NNSC LIB version \n");
		printf("   21. Config NNSC LIB function \n");
		printf("   22. Config NNSC LIB message \n");
		printf("   23. Config NNSC LIB param \n");
		printf("   24. Config NNSC LIB test mode \n");
		printf("   25. Config NNSC LIB SCD control \n");
		printf("   26. Config NNSC LIB SCD param \n");
		printf("----------------------------------------\n");
		#if (EXTEND_YUV_EN)
		printf("   50. Set nnsc_yuv_src (%s) \n", (nnsc_yuv_src == 0) ? "original yuv" : "extend");
		printf("----------------------------------------\n");
		#endif
		printf("   0.  Quit\n");
		printf("----------------------------------------\n");
		do {
			printf(">> ");
			option = get_choose_int();
		} while (0);

		switch (option) {
		case 1:
			if (is_nnsc_init) {
				is_nnsc_run = FALSE;
				printf("NNSC NET init yet \n");
				break;
			}

			printf("Start of init NNSC NET \n");

			#if (ACCESS_NNSC_NET_EN)
			// print info
			printf("nnsc net, model name: %s, size: %d \n", NNSC_NET_MODEL_NAME, NNSC_NET_MODEL_SIZE);

			// init network modules
			ret = NNSCnet_init_module();
			if (ret != HD_OK) {
				printf("init module for NN fail (%d) \n", ret);
				goto exit;
			}

			// get yuv info
			yuv_info.id = VCAP_ID_PATH_1;
			yuv_info.yuv_info.pid = 0;
			vendor_isp_get_common(ISPT_ITEM_YUV, &yuv_info);
			vendor_isp_set_common(ISPT_ITEM_YUV, &yuv_info.id);

			// setup with queried yuv info
			in_cfg.w = yuv_info.yuv_info.pw[0];
			in_cfg.h = yuv_info.yuv_info.ph[0];
			in_cfg.c = 2; // 2: YUV420
			in_cfg.loff = yuv_info.yuv_info.loff[0];
			in_cfg.fmt = HD_VIDEO_PXLFMT_YUV420;

			ret = NNSCnet_set_input_config(stream_nn.in_path, &in_cfg);
			if (HD_OK != ret) {
				printf("proc_id(%u) NNSCnet input set config (%d) \n", stream_nn.in_path, ret);
				goto exit;
			}

			// do NN proc config
			ret = NNSCnet_set_config(stream_nn.net_path, &net_cfg);
			if (HD_OK != ret) {
				printf("proc_id(%u) NNSCnet set config (%d) \n", stream_nn.net_path, ret);
				goto exit;
			}

			// in NNSCnet_open_module(), we do:
			// load input image from SD card for test (optional)
			// load network model bin
			// load network label txt
			ret = NNSCnet_open_module(&stream_nn);
			if (ret != HD_OK) {
				printf("nnsc net open module (%d) \n", ret);
				goto exit;
			}
			
			ret = NNSCnet_alloc_io_buf(stream_nn.net_path);
			if (ret != HD_OK) {
				printf("nnsc net alloc io buf (%d) \n", ret);
				goto exit;
			}

			NNSCnet_start_input(stream_nn.in_path);

			// let scene classification network start to work
			ret = NNSCnet_start(&stream_nn);
			if (ret != HD_OK)
			{
				printf("network_user_start() fail (%d) \r\n", ret);
				goto exit;
			}


			NNSCnet_get_ai_outlayer_list(stream_nn.net_path, &outlayer_num, outlayer_path_list);
			printf("output layer number: %d\r\n", outlayer_num);
			if (outlayer_num!=1) {
				printf("output layer number should be 1.\r\n");
				return 0;
			}

			ret = hd_common_mem_alloc("ai_layer_structure", &outlayer_buf_pa, (void*)&outlayer_buf_va, outlayer_num * sizeof(VENDOR_AI_BUF), DDR_ID0);
			if (ret != HD_OK)
			{
				printf("hd_common_mem_alloc() fail (%d) \r\n", ret);
				goto exit;
			}

			NNSCnet_get_ai_outlayer_by_path_id(stream_nn.net_path, outlayer_path_list[0], &outlayer_buf_va);
			outlayer_length = outlayer_buf_va.width * outlayer_buf_va.height * outlayer_buf_va.channel * outlayer_buf_va.batch_num;

			ret = hd_common_mem_alloc("ai_outlayer_softmax", &softmax_loc_layer_float_pa, (void**)&softmax_loc_layer_float_va, outlayer_length * sizeof(FLOAT), DDR_ID0);
			if (ret != HD_OK)
			{
				printf("hd_common_mem_alloc() fail (%d) \r\n", ret);
				goto exit;
			}
			#endif

			printf("End of init NNSC NET \n");
			is_nnsc_init = TRUE;
			is_nnsc_run = FALSE;

			break;

		case 3:
			if (!is_nnsc_init) {
				printf("NNSC not init yet\n");
				break;
			}
			if (is_nnsc_run) {
				printf("NNSC NET run yet \n");
				break;
			}

			printf("Start of NNSC \n");

			nnsc_set_param(&nnsc_param);
			nnsc_thread_run = 1;
			ret = pthread_create(&nnsc_thread_id, NULL, access_nn_thread, (void *)&stream[0]);
			if (ret < 0) {
				printf("create access_nn_thread thread failed");
				goto exit;
			}

			is_nnsc_run = TRUE;
			break;

		case 5:
			if (!is_nnsc_init) {
				printf("nnsc net not init yet\n");
				is_nnsc_run = FALSE;
				break;
			}
			printf("Start of uninit NNSC NET \n");

			nnsc_thread_run = 0;
			sleep(1);
			pthread_join(nnsc_thread_id, NULL);

			#if (ACCESS_NNSC_NET_EN)
			NNSCnet_stop_input(stream_nn.in_path); 
			NNSCnet_stop(&stream_nn);
			ret = NNSCnet_close_module(&stream_nn);
			if (ret != HD_OK) {
				printf("net close module fail (%d) \n", ret);
			}
			ret = NNSCnet_uninit_module();
			if (ret != HD_OK) {
				printf("net unint module fail (%d) \n", ret);
			}
			#endif

			is_nnsc_init = FALSE;
			is_nnsc_run = FALSE;
			printf("End of uninit NNSC NET \n");

			break;

		case 10:
			do {
				printf("Config nnsc net msg(0: disable; 1: enable)>> \n");
				nnsc_net_msg_en = (UINT32)get_choose_int();
			} while (0);
			break;

		case 20:
			nnsc_lib_version = nnsc_get_version();
			printf(" nnsc lib version : v%d.%d \n", (nnsc_lib_version >> 8) & 0xff, (nnsc_lib_version >> 0) & 0xff);
			break;

		case 21:
			do {
				printf("Config nnsc lib func(0: disable; 1: enable)>> \n");
				nnsc_lib_func_en = (UINT32)get_choose_int();
			} while (0);
			break;

		case 22:
			do {
				printf("Config nnsc lib msg type>> \n");
				printf("---------------------------------------- \r\n");
				printf("%4d >> NNSC_DBG_TYPE_NULL \n", NNSC_DBG_TYPE_NULL);
				printf("%4d >> NNSC_DBG_TYPE_MAIN \n", NNSC_DBG_TYPE_MAIN);
				printf("%4d >> NNSC_DBG_TYPE_SCD \n", NNSC_DBG_TYPE_SCD);
				printf("%4d >> NNSC_DBG_TYPE_AE \n", NNSC_DBG_TYPE_AE);
				printf("%4d >> NNSC_DBG_TYPE_AWB \n", NNSC_DBG_TYPE_AWB);
				printf("%4d >> NNSC_DBG_TYPE_IQ \n", NNSC_DBG_TYPE_IQ);
				printf("%4d >> NNSC_DBG_TYPE_PARAM \n", NNSC_DBG_TYPE_PARAM);
				printf("---------------------------------------- \r\n");
				nnsc_lib_msg_type = (UINT32)get_choose_int();
				nnsc_set_dbg_out(nnsc_lib_msg_type);
			} while (0);
			break;

		case 23:
			do {
				printf("Config nnsc lib nnsc param item (0 ~ %d)>> \r\n", NNSC_TYPE_MAX_NUM - 1);
				printf("---------------------------------------- \r\n");
				printf("%2d >> NNSC_TYPE_BACKLIGHT \n", NNSC_TYPE_BACKLIGHT);
				printf("%2d >> NNSC_TYPE_FOGGY \n", NNSC_TYPE_FOGGY);
				printf("%2d >> NNSC_TYPE_GRASS \n", NNSC_TYPE_GRASS);
				printf("%2d >> NNSC_TYPE_SKIN_COLOR \n", NNSC_TYPE_SKIN_COLOR);
				printf("%2d >> NNSC_TYPE_SNOWFIELD \n", NNSC_TYPE_SNOWFIELD);
				printf("---------------------------------------- \r\n");
				option_1 = (UINT32)get_choose_int();
			} while (0);
			printf("Config nnsc lib param sensitive (0 ~ %d)>> \r\n", NNSC_PARAM_SENSITIVE_MAX);
			nnsc_param.param[option_1].sensitive = get_choose_int();
			if (nnsc_param.param[option_1].sensitive > NNSC_PARAM_SENSITIVE_MAX) {
				nnsc_param.param[option_1].sensitive = NNSC_PARAM_SENSITIVE_MAX;
			}
			printf("Config nnsc lib param strength (0 ~ %d)>> \r\n", NNSC_PARAM_STRENGTH_MAX);
			nnsc_param.param[option_1].adj_strength = get_choose_int();
			if (nnsc_param.param[option_1].adj_strength > NNSC_PARAM_STRENGTH_MAX) {
				nnsc_param.param[option_1].adj_strength = NNSC_PARAM_STRENGTH_MAX;
			}
			nnsc_set_param(&nnsc_param);
			break;

		case 24:
			do {
				printf("Config nnsc lib test enable (0: disable; 1: enable)>> \n");
				test_param.enable = (UINT32)get_choose_int();
			} while (0);
			do {
				printf("Config nnsc lib test mode (0 ~ %d)>> \n", NNSC_TYPE_MAX_NUM - 1);
				printf("---------------------------------------- \r\n");
				printf("%2d >> NNSC_TYPE_BACKLIGHT \n", NNSC_TYPE_BACKLIGHT);
				printf("%2d >> NNSC_TYPE_FOGGY \n", NNSC_TYPE_FOGGY);
				printf("%2d >> NNSC_TYPE_GRASS \n", NNSC_TYPE_GRASS);
				printf("%2d >> NNSC_TYPE_SKIN_COLOR \n", NNSC_TYPE_SKIN_COLOR);
				printf("%2d >> NNSC_TYPE_SNOWFIELD \n", NNSC_TYPE_SNOWFIELD);
				printf("---------------------------------------- \r\n");
				test_param.mode_type = (UINT32)get_choose_int();
			} while (0);
			nnsc_set_test_mode(&test_param);
			break;

		case 25:
			do {
				printf("Config nnsc lib scd control : \r\n");
			} while (0);
			printf("Config scd enable (0: disable; 1: enable)>> \n");
			scd_control.enable = (UINT32)get_choose_int();
			printf("Config scd auto_enable (0: disable; 1: enable)>> \n");
			scd_control.auto_enable = (UINT32)get_choose_int();
			printf("Config scd manual_stable (0: change; 1: stable)>> \n");
			scd_control.manual_stable = (UINT32)get_choose_int();
			nnsc_set_scd_control(&scd_control);
			break;

		case 26:
			do {
				printf("Config nnsc lib scd param : \r\n");
			} while (0);
			printf("Config stable_sensitive (0 ~ 10)>> \r\n");
			scd_param.stable_sensitive = get_choose_int();
			printf("Config change_sensitive (0 ~ 10)>> \r\n");
			scd_param.change_sensitive = get_choose_int();
			printf("Config g_diff_num_th (0 ~ 1024)>> \r\n");
			scd_param.g_diff_num_th = get_choose_int();
			nnsc_set_scd_param(&scd_param);
			break;

		#if (EXTEND_YUV_EN)
		case 50:
			printf("Current %d \n", nnsc_yuv_src);
			printf("Set nnsc_yuv_src(0: original; 1: yuv extend >> \n");
			nnsc_yuv_src = (UINT32)get_choose_int();
			if (nnsc_yuv_src <= 1) {
				printf("Set to %s \n", (nnsc_yuv_src == 0) ? "original yuv" : "extend");
			}
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

exit:
	#if (OSG_FUNC_EN)
	osg_uninit();
	#endif

	#if (EXTEND_YUV_EN)
	extend_yuv_thread_run = TRUE;
	sleep(1);
	pthread_join(extend_yuv_thread_id, NULL);

	hd_videoproc_stop(stream[1].proc_path);
	hd_videoenc_stop(stream[1].enc_path);
	hd_videoproc_unbind(HD_VIDEOPROC_1_OUT_0);
	#endif

	#if (OSG_FUNC_EN | EXTEND_YUV_EN)
	plug_pq_nnsc_close_module(&stream[1]);
	plug_pq_nnsc_exit_module();
	#endif

	vendor_isp_uninit();
	hd_common_mem_uninit();
	hd_common_uninit();

	return 0;
}
