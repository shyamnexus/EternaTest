/**
	@brief Sample code of video liveview with vsp.\n

	@file video_record_with_vsp.c

	@author Ben Wang

	@ingroup mhdal

	@note This file is modified from video_record.c.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
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
#include "vendor_videocapture.h"
#include "vendor_videoprocess.h"
#include "vendor_videoenc.h"
#include "vendor_vpe.h"
#include "vendor_isp.h"
#include "vendor_gfx.h"
#include <kwrap/examsys.h> 	//for MAIN(), GETCHAR() API
#include <pd_shm.h>
#include <video_record.h>


// platform dependent
#if defined(__LINUX)
#define MAIN(argc, argv) 		int main(int argc, char** argv)
#define GETCHAR()				getchar()
#include <sys/prctl.h>
#include "math.h"
#include <sys/shm.h>
#else
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



#define YUV_BLK_SIZE      (DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, HD_VIDEO_PXLFMT_YUV420))
#define DEBUG_MENU 		1

#define CHKPNT			printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
#define DBGH(x)			printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x)			printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)
#define DBG_ERR(fmtstr, args...)  printf("\033[31mERR:%s(): \033[0m" fmtstr,__func__, ##args)
#define DBG_WRN(fmtstr, args...)  printf("\033[33mWRN:%s(): \033[0m" fmtstr,__func__, ##args)
#define DBG_DUMP(fmtstr, args...) printf(fmtstr, ##args)
#define DBG_IND(fmtstr, args...)
#define SEN1_VCAP_ID 0
#define SEN2_VCAP_ID 2
#define SEN3_VCAP_ID 4
#define SEN4_VCAP_ID 5

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

#define FUNC_WRITE_FILE		1
#define FUNC_PTZ		    0
#define FUNC_FUSION		    1
#define MAX_VAL(a,b)        ((a) > (b) ? (a) : (b))


///////////////////////////////////////////////////////////////////////////////
#define SEN_OUT_FMT		HD_VIDEO_PXLFMT_RAW12
#define CAP_OUT_FMT		HD_VIDEO_PXLFMT_RAW12
#define IPP_OUT_FMT     HD_VIDEO_PXLFMT_YUV420_NVX5
#define VSP_OUT_FMT     HD_VIDEO_PXLFMT_YUV420_NVX2
#define GFX_OUT_FMT     HD_VIDEO_PXLFMT_YUV420

#define CA_WIN_NUM_W		32
#define CA_WIN_NUM_H		32
#define LA_WIN_NUM_W		32
#define LA_WIN_NUM_H		32
#define VA_WIN_NUM_W		16
#define VA_WIN_NUM_H		16
#define YOUT_WIN_NUM_W		128
#define YOUT_WIN_NUM_H		128
#define ETH_8BIT_SEL		0 //0: 2bit out, 1:8 bit out
#define ETH_OUT_SEL		1 //0: full, 1: subsample 1/2

#define VDOOUT0_TYPE   1 //1 for LCD, 2 for HDMI, 0 for TV
#define VDOOUT1_TYPE   2 //1 for LCD, 2 for HDMI, 0 for TV

#define VDOOUT_HDMI  HD_VIDEOOUT_HDMI_1920X1080I60

#define MCLK_SRC_SYNC_FUNC    0

#define USE_REAL_SENSOR       1    // set 0 for patgen

#define ENABLE_2A             1



#define VSP_OUT_STREAM_CNT       3
#define VSP_MAX_BLEND_FRAME_NUM  4

UINT32 sen_vcap_id[VSP_MAX_BLEND_FRAME_NUM] = { 0, 2, -1, -1};
UINT32 sen_pinmux[VSP_MAX_BLEND_FRAME_NUM]  = { 0, 1, -1, -1};
UINT32 sen_csi_id[VSP_MAX_BLEND_FRAME_NUM]  = { 0, 2, -1, -1};
UINT32 vcap_out_fmt[VSP_MAX_BLEND_FRAME_NUM] = { HD_VIDEO_PXLFMT_RAW12,
												 HD_VIDEO_PXLFMT_RAW12,
												 HD_VIDEO_PXLFMT_RAW12,
												 HD_VIDEO_PXLFMT_RAW12,
												};

#define SENSOR_NAME        "nvt_sen_os04a10_slave"
#define VDO_SIZE_W         2688
#define VDO_SIZE_H         1520

#define VSP_IN_SIZE_W      VDO_SIZE_W
#define VSP_IN_SIZE_H      VDO_SIZE_H

#define VSP_DATA_PATH "/mnt/sd/vsp/"
UINT32 dre_work_buf_size = (ALIGN_CEIL_16(240)*2850*4);

UINT32 vcap_num = 0;
UINT32 vsp_sensor_mask = 0xffff;
UINT32 vsp_vpe_2d_scl_out_w[VSP_MAX_BLEND_FRAME_NUM] = {0};
UINT32 vsp_vpe_2d_scl_out_h = 2220;
UINT32 vsp_lut_tbl_sz = 257, vsp_blend_frame_num = 2;
UINT32 vsp_ovlp_w[VSP_MAX_BLEND_FRAME_NUM] = {0};
UINT32 vsp_vpe_out2_w = 0; // total overlap_width * 2
UINT32 vsp_ovlp_h = 2220, vsp_vpe_out2_h = 2220;
UINT32 vsp_out_size_w = 4688, vsp_out_size_h = 2220;
UINT32 ptz_out_size_w = 1440, ptz_out_size_h = 1440;
char lut_bin[VSP_MAX_BLEND_FRAME_NUM][50] = {0};


//dynamic offset
UINT32 dewarp_out_x_ofs_max[VSP_MAX_BLEND_FRAME_NUM] = {0};
INT32 overlap_shift_value[VSP_MAX_BLEND_FRAME_NUM] = {0};

#define STREAM2_SIZE_W        1920
#define STREAM2_SIZE_H        ALIGN_CEIL_4(STREAM2_SIZE_W * vsp_out_size_h/vsp_out_size_w)

#define STREAM3_SIZE_W        1280
#define STREAM3_SIZE_H        ALIGN_CEIL_4(STREAM3_SIZE_W * vsp_out_size_h/vsp_out_size_w)

#define VSP_STREAM1_ID        (vcap_num)
#define VSP_STREAM2_ID        (vcap_num + 1)
#define VSP_STREAM3_ID        (vcap_num + 2)

static UINT32 vdo_br      = (4 * 1024 * 1024);
static UINT32 stream1_br  = (8 * 1024 * 1024);
static UINT32 stream2_br  = (2 * 1024 * 1024);
static UINT32 stream3_br  = (1 * 1024 * 1024);


#define SENSOR_FPS         15
///////////////////////////////////////////////////////////////////////////////

#define VDO_DDR_ID		DDR_ID0
#define VCAP_DDR_ID		DDR_ID0
#define VPRC_DDR_ID		DDR_ID0
#define VPE_DDR_ID		DDR_ID0
#define DRE_DDR_ID		DDR_ID0
#define VENC_DDR_ID		DDR_ID0
#define GFX_DDR_ID		DDR_ID0

#define VSP_DEV_CNT        1
#define OUT_VSP_COUNT      1


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

#define VIDEOPROC_ALG_FUNC HD_VIDEOPROC_FUNC_WDR | HD_VIDEOPROC_FUNC_DEFOG | HD_VIDEOPROC_FUNC_COLORNR | HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA

#define VPE_EFFECT_NONE                   0
#define VPE_EFFECT_SHARPEN       0x00000001
#define VPE_EFFECT_GDC_CTL       0x00000002
#define VPE_EFFECT_GDC           0x00000004
#define VPE_EFFECT_2DLUT         0x00000008
#define VPE_EFFECT_DCTG          0x00000010
#define VSP_ISP_ID                        0
#define PTZ_ISP_ID                        1
#if defined(__LINUX)
static UINT32 g_effect = VPE_EFFECT_GDC_CTL|VPE_EFFECT_2DLUT|VPE_EFFECT_DCTG;
#else
// rtos not support iq
//static UINT32 g_effect = VPE_EFFECT_NONE;
static UINT32 g_effect = VPE_EFFECT_GDC_CTL|VPE_EFFECT_2DLUT|VPE_EFFECT_DCTG;
#endif

static VPET_2DLUT_PARAM lut2d = {0};

static int    g_quit = 0;
static int    g_vsp_mode = 0;
static UINT32 g_write_file = 1;
static UINT32 g_enc_poll = 1;
static UINT32 g_fps = SENSOR_FPS;
static UINT32 g_ipp_out_fmt = IPP_OUT_FMT;
static UINT32 g_en_sub = 1;
static UINT32 g_en_sub2 = 1;
static char   *g_shm = NULL;
static int    g_shmid = 0;


///////////////////////////////////////////////////////////////////////////////
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
static HD_RESULT mem_init(void)
{
	HD_RESULT              ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
	UINT32                   i = 0, j = 0;

	// config common pool (cap)
	mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_RAW_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, CAP_OUT_FMT)
        													+VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
        													+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
	mem_cfg.pool_info[i].blk_cnt = vcap_num * 3;
	mem_cfg.pool_info[i].ddr_id = VCAP_DDR_ID;

	//vsp input buffer
	i++;
	mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(VSP_IN_SIZE_W, VSP_IN_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[i].blk_cnt = VSP_DEV_CNT * vcap_num * 3;
	mem_cfg.pool_info[i].ddr_id = VPRC_DDR_ID;

	//vpe out temp buffer1
	i++;
	mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(ALIGN_CEIL_16((vsp_vpe_2d_scl_out_w[0]+vsp_vpe_2d_scl_out_w[1]+vsp_vpe_2d_scl_out_w[2]+vsp_vpe_2d_scl_out_w[3])-vsp_vpe_out2_w/2), vsp_vpe_2d_scl_out_h, HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[i].blk_cnt = VSP_DEV_CNT * 3;
	mem_cfg.pool_info[i].ddr_id = VDO_DDR_ID;

	//vpe out temp buffer2
	if (vsp_vpe_out2_w) {
		i++;
		mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
		mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(ALIGN_CEIL_16(vsp_vpe_out2_w), vsp_vpe_out2_h, HD_VIDEO_PXLFMT_YUV420);
		mem_cfg.pool_info[i].blk_cnt = VSP_DEV_CNT * 3;
		mem_cfg.pool_info[i].ddr_id = VDO_DDR_ID;
	}

	//vsp out buffer
	i++;
	mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(ALIGN_CEIL_16(vsp_out_size_w), vsp_out_size_h, HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[i].blk_cnt = VSP_DEV_CNT*6;
	mem_cfg.pool_info[i].ddr_id = VDO_DDR_ID;

	//stream2
	if (g_en_sub) {
		i++;
		mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
		mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()+2*VDO_YUV_BUFSIZE(ALIGN_CEIL_16(STREAM2_SIZE_W), STREAM2_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
		mem_cfg.pool_info[i].blk_cnt = 6;
		mem_cfg.pool_info[i].ddr_id = VDO_DDR_ID;
	}
	if (g_en_sub2) {
		//stream3
		i++;
		mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
		mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(STREAM3_SIZE_W, STREAM3_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
		mem_cfg.pool_info[i].blk_cnt = 6;
		mem_cfg.pool_info[i].ddr_id = VDO_DDR_ID;
	}

	// dre fusion buffer
	for (j = 0; j < vsp_blend_frame_num; j ++) {
		if (vsp_ovlp_w[j]) {
			i++;
			mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
			mem_cfg.pool_info[i].blk_size = 2*VDO_YUV_BUFSIZE(vsp_ovlp_w[j], vsp_ovlp_h, HD_VIDEO_PXLFMT_Y8);
			mem_cfg.pool_info[i].blk_cnt = VSP_DEV_CNT;
			mem_cfg.pool_info[i].ddr_id = DRE_DDR_ID;
		}
	}
	#if FUNC_FUSION
	// DRE working buffer
	i++;
	mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[i].blk_size = dre_work_buf_size;
	mem_cfg.pool_info[i].blk_cnt = 1;
	mem_cfg.pool_info[i].ddr_id = DRE_DDR_ID;
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

static HD_RESULT set_vsp_effect_cfg(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret = HD_OK;
	FILE     *fp;
	UINT32    frame_idx;

	if (g_effect & VPE_EFFECT_GDC_CTL) {
		VPET_DCE_CTL_PARAM dce_ctl = {0};

		for (frame_idx = 0; frame_idx < vsp_blend_frame_num; frame_idx++) {
			dce_ctl.id = ((frame_idx << 8) | p_stream->isp_id);
			dce_ctl.dce_ctl.enable = 1;
			dce_ctl.dce_ctl.dce_mode = VPE_ISP_DCE_MODE_2DLUT_ONLY;
			// set vpe
			vendor_vpe_set_cmd(VPET_ITEM_DCE_CTL_PARAM, &dce_ctl);
		}

		// set vpe ptz param
		dce_ctl.id = p_stream->ptz_isp_id;
		#if FUNC_PTZ
		dce_ctl.dce_ctl.enable = 1;
		dce_ctl.dce_ctl.dce_mode = VPE_ISP_DCE_MODE_2DLUT_DCTG;
		#else
		dce_ctl.dce_ctl.enable = 0;
		dce_ctl.dce_ctl.dce_mode = VPE_ISP_DCE_MODE_2DLUT_ONLY;
		#endif
		printf("dce isp_id = %d\r\n", dce_ctl.id);
		vendor_vpe_set_cmd(VPET_ITEM_DCE_CTL_PARAM, &dce_ctl);
	}
	if (g_effect & VPE_EFFECT_2DLUT) {
		UINT32 lut_size = 0;
		UINT32 vpe_2dlut_size;
		UINT32 read_len;
		char   *lut_path;

		switch (vsp_lut_tbl_sz) {
		case 9:
			lut_size = 12*9*4;
			vpe_2dlut_size = VPE_ISP_2DLUT_SZ_9X9;
			break;

		case 2:
			lut_size = 4*2*4;
			vpe_2dlut_size = VPE_ISP_2DLUT_SZ_2X2;
			break;

		case 65:
			lut_size = 68*65*4;
			vpe_2dlut_size = VPE_ISP_2DLUT_SZ_65X65;
			break;

		case 129:
			lut_size = 132*129*4;
			vpe_2dlut_size = VPE_ISP_2DLUT_SZ_129X129;
			break;

		default:
		case 257:
			lut_size = 260*257*4;
			vpe_2dlut_size = VPE_ISP_2DLUT_SZ_257X257;
			break;
		}

		for (frame_idx = 0; frame_idx < vsp_blend_frame_num; frame_idx++) {

			lut_path = lut_bin[frame_idx];
			lut2d.id = ((frame_idx << 8) | p_stream->isp_id);
			lut2d.lut2d.lut_sz = vpe_2dlut_size;
			if ((fp = fopen(lut_path, "rb")) == NULL) {
				printf("open file (%s) fail !!....\r\nPlease copy test pattern to SD Card !!\r\n\r\n", lut_path);
				return -1;
			}
			setvbuf(fp, NULL, _IONBF, 0);
			read_len = fread((void *)(lut2d.lut2d.lut), 1, lut_size, fp);
			if (read_len != lut_size) {
				printf("reading 2dlut error\n");
			}
			// close input image file
			fclose(fp);
			// set vpe
			vendor_vpe_set_cmd(VPET_ITEM_2DLUT_PARAM, &lut2d);
		}
	}

	//{1, 0, 3, 2592,2, 0, 0, 0, 0, 0, 0, 0, 0},
	if (g_effect & VPE_EFFECT_DCTG ) {
		VPET_DCTG_CTRL dctg = {0};

		dctg.id = p_stream->ptz_isp_id;
		// TODO: change to vpe
		vendor_vpe_get_cmd(VPET_ITEM_DCTG_CTRL, &dctg);
		#if FUNC_PTZ
		dctg.dctg.mode_sel = VPE_ISP_DCTG_MODE_PTZ;
		dctg.dctg.ptz_param.proj_type = VPE_ISP_PTZ_PROJ_EQUIRECTANGULAR;
		dctg.dctg.ptz_param.long_aov = 1200;
		dctg.dctg.ptz_param.lati_aov = 1200;
		dctg.dctg.ptz_param.pan_angle = -1800;
		dctg.dctg.ptz_param.tilt_angle = 0;
		dctg.dctg.ptz_param.rot_angle = 0;
		dctg.dctg.ptz_param.zoom_step = 0;
		dctg.dctg.ptz_param.cam_long_aov = 1800;
		dctg.dctg.ptz_param.cam_lati_aov = 1800;
		dctg.dctg.ptz_param.cam_w = VSP_OUT_SIZE_W;
		dctg.dctg.ptz_param.cam_h = VSP_OUT_SIZE_H;
		dctg.dctg.ptz_param.stitch_overlap_angle = 100;
		// TODO: change to vpe
		vendor_vpe_set_cmd(VPET_ITEM_DCTG_CTRL, &dctg);
		#else
		dctg.dctg.mode_sel = VPE_ISP_DCTG_MODE_DISABLE;
		#endif
	}
	return ret;
}

static void init_fusion_tbl(UINTPTR va, UINT32 img_w, UINT32 img_h)
{
	UINT32 i, smooth_length = 128;
	UINTPTR temp_va = va;
	UINT8  *p_smooth_curve;

	p_smooth_curve = (UINT8  *)temp_va;
	#if 0
	for (i = 0; i < img_h; i++) {
		memset((void *)temp_va, 255, img_w/2);
		//memset((void *)temp_va, 0, img_w/2);
		temp_va += img_w/2;
		memset((void *)temp_va, 0, img_w/2);
		temp_va += img_w/2;
	}
	#else
	for (i = 0; i < img_w; i++) {
        p_smooth_curve[i] = (UINT8)round((1.0 - (atan(-(double)smooth_length/2 + i*(double)smooth_length/(double)(img_w-1))/acos(-1) + 0.5)) * 255);
        //printf("%u ", p_smooth_curve[i]);
    }
	temp_va += img_w;
	for (i = 1; i < img_h; i++) {
		memcpy((void *)temp_va, (void *)va, img_w);
		temp_va += img_w;
	}
	#endif
	printf("fusion w = %d, h = %d, va = 0x%lx, temp_va = 0x%lx\r\n", img_w, img_h, va, temp_va);
}

static void release_all_fusion_blk(VIDEO_RECORD *p_stream)
{
	UINT32 i;

	for (i = 0; i < vsp_blend_frame_num;i ++) {
		if (p_stream->fusion_blk[i]) {
			hd_common_mem_release_block(p_stream->fusion_blk[i]);
			p_stream->fusion_blk[i] = HD_COMMON_MEM_VB_INVALID_BLK;
		}
	}
	if (p_stream->fusion_work_buf_blk) {
		hd_common_mem_release_block(p_stream->fusion_work_buf_blk);
		p_stream->fusion_work_buf_blk = HD_COMMON_MEM_VB_INVALID_BLK;
	}
}

///////////////////////////////////////////////////////////////////////////////

static HD_RESULT get_cap_caps(HD_PATH_ID video_cap_ctrl, HD_VIDEOCAP_SYSCAPS *p_video_cap_syscaps)
{
	HD_RESULT ret = HD_OK;
	hd_videocap_get(video_cap_ctrl, HD_VIDEOCAP_PARAM_SYSCAPS, p_video_cap_syscaps);
	return ret;
}

static HD_RESULT set_cap_cfg(HD_PATH_ID *p_video_cap_ctrl, UINT32 cap_idx)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
	HD_PATH_ID video_cap_ctrl = 0;
	#if USE_REAL_SENSOR
	HD_VIDEOCAP_CTRL iq_ctl = {0};

	snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, SENSOR_NAME);
	cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
	cap_cfg.sen_cfg.sen_dev.if_cfg.tge.tge_en = TRUE;
	cap_cfg.sen_cfg.sen_dev.if_cfg.tge.swap = FALSE;
	cap_cfg.sen_cfg.sen_dev.if_cfg.tge.vcap_vd_src = HD_VIDEOCAP_SEN_TGE_CH1_VD_TO_VCAP0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux =  sen_pinmux[cap_idx]; //use nvt_sen_os04a10_slave@10 in peri-dev.dtsi
	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(sen_csi_id[cap_idx], 0);
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
	ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(sen_vcap_id[cap_idx]), &video_cap_ctrl); //open this for device control
	if (ret != HD_OK) {
		return ret;
	}

	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
	#if USE_REAL_SENSOR
	#if ENABLE_2A
	iq_ctl.func = HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB;
	#endif
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);
	#endif

	*p_video_cap_ctrl = video_cap_ctrl;
	return ret;
}

static HD_RESULT set_cap_param(HD_PATH_ID video_cap_path, HD_DIM *p_dim, UINT32 out_fmt)
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
		video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
		#else
		video_in_param.sen_mode = HD_VIDEOCAP_PATGEN_MODE(HD_VIDEOCAP_SEN_PAT_COLORBAR, 200);
		video_in_param.frc = HD_VIDEO_FRC_RATIO(g_fps,1);
		video_in_param.dim.w = p_dim->w;
		video_in_param.dim.h = p_dim->h;
		#endif
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
		video_out_param.pxlfmt = out_fmt;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		video_out_param.dim.w = p_dim->w;
		video_out_param.dim.h = p_dim->h;
		video_out_param.depth = 1;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT, &video_out_param);
		//printf("set_cap_param OUT=%d\r\n", ret);
	}
	UINT32 data_lane = 2;
	ret = vendor_videocap_set(video_cap_path, VENDOR_VIDEOCAP_PARAM_DATA_LANE, &data_lane);
	if (ret != HD_OK) {
		printf("VENDOR_VIDEOCAP_PARAM_DATA_LANE failed!(%d)\r\n", ret);
	}

	{
		HD_VIDEOCAP_FUNC_CONFIG video_path_param = {0};
		video_path_param.ddr_id = VCAP_DDR_ID;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_FUNC_CONFIG, &video_path_param);
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////

static HD_RESULT set_proc_cfg(HD_PATH_ID *p_video_proc_ctrl, HD_DIM* p_max_dim, HD_OUT_ID _out_id, UINT32 isp_id, UINT32 ref_3dnr_id, UINT32 in_plxfmt)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
	HD_VIDEOPROC_CTRL video_ctrl_param = {0};
	HD_PATH_ID video_proc_ctrl = 0;

	ret = hd_videoproc_open(0, _out_id, &video_proc_ctrl); //open this for device control
	if (ret != HD_OK)
		return ret;

	if (p_max_dim != NULL ) {
		video_cfg_param.pipe = HD_VIDEOPROC_PIPE_RAWALL;
		video_cfg_param.isp_id = isp_id;
		video_cfg_param.ctrl_max.func = VIDEOPROC_ALG_FUNC;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;
		video_cfg_param.in_max.pxlfmt = in_plxfmt;
		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
		if (ret != HD_OK) {
			return HD_ERR_NG;
		}
	}
	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};
		video_path_param.in_func = 0;
		video_path_param.ddr_id = VDO_DDR_ID;
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}
	video_ctrl_param.ref_path_3dnr = ref_3dnr_id;
	video_ctrl_param.func = VIDEOPROC_ALG_FUNC;
	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);

	*p_video_proc_ctrl = video_proc_ctrl;

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
static HD_RESULT set_vsp_proc_cfg(VIDEO_RECORD *p_stream, HD_PATH_ID video_proc_ctrl, HD_DIM* p_max_dim)
{
	UINT32    i;
	HD_RESULT ret = HD_OK;
	HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
	HD_VIDEOPROC_CTRL video_ctrl_param = {0};

	if (p_max_dim != NULL ) {
		video_cfg_param.pipe = HD_VIDEOPROC_PIPE_VSP;
		video_cfg_param.isp_id = p_stream->vpe_isp_id;
		video_cfg_param.ctrl_max.func = 0;
		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;
		video_cfg_param.in_max.pxlfmt = g_ipp_out_fmt;
		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
		if (ret != HD_OK) {
			return HD_ERR_NG;
		}
	}
	{
		VENDOR_VIDEOPROC_VSP_CFG vsp_cfg = {0};
		UINT32                   blk_size;
 		UINT32                   fusion_w[VENDOR_VIDEOPROC_VSP_MAX_FRM_UM] = {0};

		// get fusion block
		for (i = 0; i < vsp_blend_frame_num;i ++) {
			fusion_w[i] = vsp_ovlp_w[i];
			if (fusion_w[i]) {
				blk_size = VDO_YUV_BUFSIZE(fusion_w[i], vsp_ovlp_h, HD_VIDEO_PXLFMT_Y8);
				p_stream->fusion_blk[i] = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, blk_size, DRE_DDR_ID); // Get block from mem pool
				if (p_stream->fusion_blk[i] == HD_COMMON_MEM_VB_INVALID_BLK) {
					printf("get fusion_blk fail\r\n");
					return HD_ERR_NG;
				}
				p_stream->fusion_pa[i] = hd_common_mem_blk2pa(p_stream->fusion_blk[i]); // Get physical addr
				if (p_stream->fusion_pa[i] == 0) {
					printf("blk2pa fail, fusion_blk = 0x%lx\r\n", p_stream->fusion_blk[i]);
					goto rel_in_blk;
				}
				p_stream->fusion_va[i] = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, p_stream->fusion_pa[i], blk_size); // Get virtual addr
				if (p_stream->fusion_va[i] == 0) {
					printf("Error: mmap fail !! fusion_pa 0x%lx, fusion_blk = 0x%lx\r\n", p_stream->fusion_pa[i], p_stream->fusion_blk[i]);
					goto rel_in_blk;
				}
				// init fusion table
				init_fusion_tbl(p_stream->fusion_va[i], fusion_w[i], vsp_ovlp_h);
				// flush cache
				hd_common_mem_flush_cache((void *)p_stream->fusion_va[i], blk_size);
			}
			printf("fusion_pa[%d]= 0x%lx, va[%d] = 0x%lx\r\n", i, p_stream->fusion_pa[i], i, p_stream->fusion_va[i]);
		}
		// get dre work buffer
		p_stream->fusion_work_buf_blk = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, dre_work_buf_size, DRE_DDR_ID); // Get block from mem pool
		if (p_stream->fusion_work_buf_blk == HD_COMMON_MEM_VB_INVALID_BLK) {
			printf("get fusion work bufer fail\r\n");
			return HD_ERR_NG;
		}
		p_stream->fusion_work_buf_pa = hd_common_mem_blk2pa(p_stream->fusion_work_buf_blk); // Get physical addr
		if (p_stream->fusion_work_buf_pa == 0) {
			printf("blk2pa fail, fusion_blk = 0x%lx\r\n", p_stream->fusion_work_buf_blk);
			goto rel_in_blk;
		}
		vsp_cfg.blend_frm_num = vsp_blend_frame_num;
		vsp_cfg.dre_proc_mode = VENDOR_VIDEOPROC_DRE_PROC_FUSION_SIMP;
		vsp_cfg.dre_quality_level = 3;
		if (FUNC_FUSION) {
			for (i = 0; i < vsp_blend_frame_num;i ++) {
				vsp_cfg.ovlp_width[i] = fusion_w[i];
				vsp_cfg.dre_fusion_tbl_pa[i] = p_stream->fusion_pa[i];
				vsp_cfg.dewarp_out_x_ofs_max[i] = dewarp_out_x_ofs_max[i];

				vsp_cfg.scale_out_size[i].w= vsp_vpe_2d_scl_out_w[i];
				vsp_cfg.scale_out_size[i].h = vsp_vpe_2d_scl_out_h;
			}
			vsp_cfg.dre_work_buf_pa = p_stream->fusion_work_buf_pa;
			vsp_cfg.dre_work_buf_size = dre_work_buf_size;
		}
		//ptz
		vsp_cfg.en_ptz = FUNC_PTZ;
		vsp_cfg.vpe_out_size.w = ptz_out_size_w;
		vsp_cfg.vpe_out_size.h = ptz_out_size_h;
		vsp_cfg.dis_fusion = !FUNC_FUSION;
		ret = vendor_videoproc_set(video_proc_ctrl, VENDOR_VIDEOPROC_PARAM_VSP_CFG, &vsp_cfg);
		if (ret != HD_OK) {
			printf("VENDOR_VIDEOPROC_PARAM_VSP_CFG failed!(%d)\r\n", ret);
		}
	}

	video_ctrl_param.func = 0;
	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);

	return ret;
rel_in_blk:
	for (i = 0; i < vsp_blend_frame_num;i ++) {
		if (p_stream->fusion_blk[i]) {
			hd_common_mem_release_block(p_stream->fusion_blk[i]);
		}
	}
	if (p_stream->fusion_work_buf_blk) {
		hd_common_mem_release_block(p_stream->fusion_work_buf_blk);
	}
	return HD_ERR_NG;

}


///////////////////////////////////////////////////////////////////////////////
static HD_RESULT set_vpe_proc_cfg(VIDEO_RECORD *p_stream, HD_PATH_ID video_proc_ctrl, HD_DIM* p_max_dim)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
	HD_VIDEOPROC_CTRL video_ctrl_param = {0};

	if (p_max_dim != NULL ) {
		video_cfg_param.pipe = HD_VIDEOPROC_PIPE_VPE;
		//video_cfg_param.isp_id = HD_ISP_DONT_CARE; // set isp_id don't care, because we just use scale down function
		video_cfg_param.isp_id = p_stream->vpe_isp_id;
		video_cfg_param.ctrl_max.func = 0;
		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;
		video_cfg_param.in_max.pxlfmt = g_ipp_out_fmt;
		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
		if (ret != HD_OK) {
			return HD_ERR_NG;
		}
	}
	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};
		video_path_param.in_func = 0;
		video_path_param.ddr_id = VDO_DDR_ID;
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}
	{
		video_ctrl_param.func = 0;
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);
	}

	return ret;
}

static HD_RESULT set_proc_param(HD_PATH_ID video_proc_path, HD_DIM* p_dim, HD_IRECT *rect, HD_VIDEO_PXLFMT pxlfmt)
{
	HD_RESULT ret = HD_OK;


	if (p_dim != NULL) {
		// set mirror
		HD_VIDEOPROC_IN video_in_param = {0};

		hd_videoproc_get(video_proc_path, HD_VIDEOPROC_PARAM_IN, &video_in_param);
		//video_in_param.dim.w = p_dim->w;
		//video_in_param.dim.h = p_dim->h;
		//video_in_param.dir = HD_VIDEO_DIR_MIRRORX;
		hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_IN, &video_in_param);
	}

	if (p_dim != NULL) { //if videoproc is already binding to dest module, not require to setting this!

		HD_VIDEOPROC_OUT video_out_param = {0};
		video_out_param.func = 0;
		video_out_param.dim.w = p_dim->w;
		video_out_param.dim.h = p_dim->h;
		video_out_param.pxlfmt = pxlfmt;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		video_out_param.frc = HD_VIDEO_FRC_RATIO(1,1);
		video_out_param.depth = 1;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	}
	if (rect != NULL) {
		HD_VIDEOPROC_CROP  video_out_param = {0};

		video_out_param.mode  = HD_CROP_ON;
		video_out_param.win.rect = *rect;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT_CROP, &video_out_param);
	}
	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};
		video_path_param.in_func = 0;
		video_path_param.ddr_id = VPRC_DDR_ID;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}
	//printf("video_proc_path = 0x%x, dim = %d, %d\r\n", video_proc_path, p_dim->w, p_dim->h);
	return ret;
}

static HD_RESULT set_vsp_proc_param(HD_PATH_ID video_proc_path, HD_DIM* p_dim)
{
	HD_RESULT ret = HD_OK;

	if (p_dim != NULL) { //if videoproc is already binding to dest module, not require to setting this!

		HD_VIDEOPROC_OUT video_out_param = {0};
		video_out_param.func = 0;
		video_out_param.dim.w = p_dim->w;
		video_out_param.dim.h = p_dim->h;
		video_out_param.pxlfmt = VSP_OUT_FMT;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		video_out_param.frc = HD_VIDEO_FRC_RATIO(1,1);
		video_out_param.depth = 1;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	}

	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};
		video_path_param.in_func = 0;
		video_path_param.ddr_id = VDO_DDR_ID;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}
	//printf("video_proc_path = 0x%x, dim = %d, %d\r\n", video_proc_path, p_dim->w, p_dim->h);
	return ret;
}

static HD_RESULT set_vpe_proc_param(HD_PATH_ID video_proc_path, HD_DIM* p_dim, HD_VIDEO_PXLFMT pxlfmt)
{
	HD_RESULT ret = HD_OK;

	if (p_dim != NULL) { //if videoproc is already binding to dest module, not require to setting this!

		HD_VIDEOPROC_OUT video_out_param = {0};
		video_out_param.func = 0;
		#if 0
		video_out_param.dim.w = p_dim->w;
		video_out_param.dim.h = p_dim->h;
		#else
		video_out_param.bg.w  = ALIGN_CEIL_16(p_dim->w);
		video_out_param.bg.h  = p_dim->h;
		video_out_param.rect.w = p_dim->w;
		video_out_param.rect.h = p_dim->h;
		#endif
		video_out_param.pxlfmt = pxlfmt; //VSP_OUT_FMT;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		video_out_param.frc = HD_VIDEO_FRC_RATIO(1,1);
		video_out_param.depth = 1;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	}

	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};
		video_path_param.in_func = 0;
		video_path_param.ddr_id = VPE_DDR_ID;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}
	printf("video_proc_path = 0x%x, dim = %d, %d\r\n", video_proc_path, p_dim->w, p_dim->h);
	return ret;
}

///////////////////////////////////////////////////////////////////////////////

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
	{
		HD_VIDEOENC_FUNC_CONFIG video_func_config = {0};

		video_func_config.in_func = 0;
		video_func_config.ddr_id = VENC_DDR_ID;
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
		video_in_param.dir           = HD_VIDEO_DIR_NONE;
		video_in_param.pxl_fmt = VSP_OUT_FMT;
		video_in_param.dim.w   = p_dim->w;
		video_in_param.dim.h   = p_dim->h;
		video_in_param.frc     = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_IN, &video_in_param);
		if (ret != HD_OK) {
			printf("set_enc_param_in = %d\r\n", ret);
			return ret;
		}

		printf("enc_type=%d\r\n", enc_type);

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

			#if 0
			{
				VENDOR_VIDEOENC_QUALITY_BASE_MODE  quality = {0};
				quality.quality_base_en = 1;
				vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_QUALITY_BASE , &quality);
			}
			#endif

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

static HD_RESULT set_isp_cfg(void)
{
	AET_CFG_INFO cfg_info = {0};
	AET_METER_WINDOW meter_window = {0};
	UINT32 iy, iyw, ix;
	UINT32 i;

	strncpy(cfg_info.path, "/mnt/app/isp/isp_os04a10_0_stitch.cfg", CFG_NAME_LENGTH);
		for (i = 0; i < vcap_num; i++) {
	cfg_info.id = sen_vcap_id[i];
		vendor_isp_set_ae(AET_ITEM_RLD_CONFIG, &cfg_info);
		vendor_isp_set_awb(AWBT_ITEM_RLD_CONFIG, &cfg_info);
		vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);
	}

		printf("load %s \n", cfg_info.path);

	meter_window.id = sen_vcap_id[0];;
	for(iy=0; iy<8; iy++) {
		iyw = iy*8;
		for(ix=0; ix<8; ix++) {
			meter_window.meter_window.matrix[iyw+ix] = 1;
		}
	}
	 vendor_isp_set_ae(AET_ITEM_METER_WIN, &meter_window);

	return HD_OK;
}

static HD_RESULT set_isp_cfg_ecs(void)
{
	AET_CFG_INFO cfg_info = {0};

	cfg_info.id = sen_vcap_id[0];
	strncpy(cfg_info.path, "/mnt/app/isp/isp_os04a10_0_ecs0.cfg", CFG_NAME_LENGTH);
	vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);
	printf("load %s \n", cfg_info.path);

	cfg_info.id = sen_vcap_id[1];
	strncpy(cfg_info.path, "/mnt/app/isp/isp_os04a10_0_ecs1.cfg", CFG_NAME_LENGTH);
	vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);
	printf("load %s \n", cfg_info.path);

	return HD_OK;
}

static HD_RESULT set_2a_stitch(BOOL enable)
{
	#if 1
	AWBT_STITCH_ID awb_stitch = {0};
	AET_STITCH_ID ae_stitch = {0};
	HD_RESULT result;
	UINT32 stitch_id_1, stitch_id_2;
	UINT32 i;

	stitch_id_1 = 0;

	if (enable) {
		#if 1
		for (i = 0; i < vcap_num; i++){
			stitch_id_1 = stitch_id_1 + (sen_vcap_id[i] << (i * 4));
		}
		for (i = vsp_blend_frame_num; i < 8; i++){
			stitch_id_1 = stitch_id_1 + (15 << (i * 4));
		}
		#endif
		//stitch_id_1 = 0xffffff20;
		stitch_id_2 = 0xFFFFFFFF;
	} else {
		stitch_id_1 = 0xFFFFFFFF;
		stitch_id_2 = 0xFFFFFFFF;
	}

	for (i = 0; i < vcap_num; i++) {
		awb_stitch.id = sen_vcap_id[i];
		awb_stitch.stitch_id.stitch_id_1 = stitch_id_1;
		awb_stitch.stitch_id.stitch_id_2 = stitch_id_2;
		result = vendor_isp_set_awb(AWBT_ITEM_STITCH_ID, &awb_stitch);
		if (result == HD_OK) {
			printf("id = %d, awb_stitch id1 = 0x%x \n", awb_stitch.id, awb_stitch.stitch_id.stitch_id_1);
		} else {
			printf("Set AWBT_ITEM_STITCH_ID fail \n");
		}

		ae_stitch.id = sen_vcap_id[i];
		ae_stitch.stitch_id.stitch_id_1 = stitch_id_1;
		ae_stitch.stitch_id.stitch_id_2 = stitch_id_2;
		result = vendor_isp_set_ae(AET_ITEM_STITCH_ID, &ae_stitch);
		if (result == HD_OK) {
			printf("id = %d, ae_stitch id1 = 0x%x \n", ae_stitch.id, ae_stitch.stitch_id.stitch_id_1);
		} else {
			printf("Set AET_ITEM_STITCH_ID fail \n");
		}
	}

	set_isp_cfg_ecs();
	#endif
	return HD_OK;
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
	if ((ret = hd_gfx_init()) != HD_OK)
		return ret;

	#if AI_ENABLE
	if ((ret = input_init()) != HD_OK)
		return ret;
	if ((ret = network_init()) != HD_OK)
		return ret;
	#endif

	return HD_OK;
}

static HD_RESULT open_module_sen(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim, UINT32 cap_idx)
{
	HD_RESULT ret;

	// set videocap config
	ret = set_cap_cfg(&p_stream->cap_ctrl, cap_idx);
	if (ret != HD_OK) {
		printf("set cap-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, HD_VIDEOPROC_CTRL(cap_idx), sen_vcap_id[cap_idx], HD_VIDEOPROC_OUT(cap_idx, 0), vcap_out_fmt[cap_idx]);
	if (ret != HD_OK) {
		printf("set proc-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	if ((ret = hd_videocap_open(HD_VIDEOCAP_IN(sen_vcap_id[cap_idx], 0), HD_VIDEOCAP_OUT(sen_vcap_id[cap_idx], 0), &p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_IN(cap_idx, 0), HD_VIDEOPROC_OUT(cap_idx, 0), &p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_IN(0, cap_idx), HD_VIDEOENC_OUT(0, cap_idx), &p_stream->enc_path)) != HD_OK)
		return ret;

	#if AI_ENABLE

	if (cap_idx == 0) {
		if ((ret = network_bind_isp_cb(p_stream)) != HD_OK)
			return ret;
	}

	if ((ret = network_open(p_stream->net_path)) != HD_OK)
		return ret;
	#endif

	return HD_OK;
}

static HD_RESULT open_module_vsp_stream1(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim, UINT32 stream_id)
{
	HD_RESULT ret;

	// venc isp_id
	p_stream->isp_id = sen_vcap_id[0];

	// vsp isp_id
	p_stream->vpe_isp_id = VSP_ISP_ID;
	p_stream->ptz_isp_id = PTZ_ISP_ID;	// ptz_isp_id must = vsp_isp_id+1
	ret = hd_videoproc_open(0, HD_VIDEOPROC_CTRL(stream_id), &p_stream->proc_ctrl); //open this for device control
	if (ret != HD_OK)
		return ret;

	// set videoproc config
	ret = set_vsp_proc_cfg(p_stream, p_stream->proc_ctrl, p_proc_max_dim);
	if (ret != HD_OK) {
		printf("set set_vsp_proc_cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	if (g_effect) {
		if ((ret = set_vsp_effect_cfg(p_stream)) != HD_OK)
			return ret;
	}
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_IN(stream_id, 0), HD_VIDEOPROC_OUT(stream_id, 0), &p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_IN(0, stream_id), HD_VIDEOENC_OUT(0, stream_id), &p_stream->enc_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT open_module_vsp_stream2(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim, UINT32 stream_id)
{
	HD_RESULT ret;

	// venc isp_id
	p_stream->isp_id = sen_vcap_id[0];
	p_stream->vpe_isp_id = HD_ISP_DONT_CARE; // set don't care, just use scale function
	ret = hd_videoproc_open(0, HD_VIDEOPROC_CTRL(stream_id), &p_stream->proc_ctrl); //open this for device control
	if (ret != HD_OK)
		return ret;

	// set videoproc config
	ret = set_vpe_proc_cfg(p_stream, p_stream->proc_ctrl, p_proc_max_dim);
	if (ret != HD_OK) {
		printf("set proc-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_IN(stream_id, 0), HD_VIDEOPROC_OUT(stream_id, 0), &p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_IN(0, stream_id), HD_VIDEOENC_OUT(0, stream_id), &p_stream->enc_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT open_module_vsp_stream3(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim, UINT32 stream_id)
{
	HD_RESULT ret;

	// venc isp_id
	p_stream->isp_id = sen_vcap_id[0];
	if ((ret = hd_videoenc_open(HD_VIDEOENC_IN(0, stream_id), HD_VIDEOENC_OUT(0, stream_id), &p_stream->enc_path)) != HD_OK)
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
	if ((ret = hd_videocap_close(p_stream->cap_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT close_module2(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;

	#if AI_ENABLE
	if ((ret = network_close(p_stream->net_path)) != HD_OK)
		return ret;
	#endif
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
	if ((ret = hd_gfx_uninit()) != HD_OK)
		return ret;

	#if AI_ENABLE
	if ((ret = input_uninit()) != HD_OK)
		return ret;
	if ((ret = network_uninit()) != HD_OK)
		return ret;
	#endif

	return HD_OK;
}

static void *vsp_thread(void *arg)
{
	UINT32               i, push_idx, frame_idx, pull_frame_bit, frame_mask = 0;

	HD_RESULT ret = HD_OK;
	VIDEO_RECORD  *p_stream[VSP_MAX_BLEND_FRAME_NUM + VSP_OUT_STREAM_CNT];
	VIDEO_RECORD  *p_stream0, *p_vsp_stream;
	HD_VIDEO_FRAME video_frame[VSP_MAX_BLEND_FRAME_NUM] = {0};
	HD_VIDEO_FRAME tmp_video_frame = {0};
	HD_VIDEOPROC_POLL_LIST poll_list[VSP_MAX_BLEND_FRAME_NUM];
	UINT64         base_time = 0;
	#if USE_REAL_SENSOR
	UINT64         time_threshold =  20000; // 20 ms
	#else
	UINT64         time_threshold = 100000; // 100 ms
	#endif
	BOOL           is_drop_old = FALSE;
	UINT32         keep_idx = 0, sensor_idx = 0;

	p_stream0 = (VIDEO_RECORD *)arg;

	for (i = 0; i < vsp_blend_frame_num ;i++) {
		p_stream[i] = p_stream0 + i;
	}
	// adjust p_stream for by sensor_mask
	for (i = 0; i < VSP_MAX_BLEND_FRAME_NUM + VSP_OUT_STREAM_CNT;i++) {
		if (vsp_sensor_mask & (1 << i)) {
			p_stream[sensor_idx] = p_stream0 + i;
			//printf("sensor_idx = %d, i = %d\r\n", sensor_idx, i);
			sensor_idx ++;
		}
	}
	p_vsp_stream = p_stream0 + VSP_STREAM1_ID;
	for (i = 0; i < vsp_blend_frame_num ;i++) {
		frame_mask |= (1 << i);
	}
	printf("frame_mask = 0x%x\r\n", frame_mask);
	#if defined(__LINUX)
	{
		pthread_attr_t thread_attr;
	    struct sched_param sch_param;

		prctl(PR_SET_NAME, "vsp_thread");
	    pthread_attr_init(&thread_attr);
	    sch_param.sched_priority = 99;
	    pthread_attr_setschedpolicy(&thread_attr,SCHED_RR);
	    pthread_attr_setschedparam(&thread_attr,&sch_param);
	    pthread_attr_setinheritsched(&thread_attr,PTHREAD_EXPLICIT_SCHED);
	}
	#endif
	for (i = 0; i < vsp_blend_frame_num ;i++) {
		poll_list[i].path_id = p_stream[i]->proc_path;
	}
	//--------- pull data test ---------
	pull_frame_bit = 0;
	while (!g_quit) {
		ret = hd_videoproc_poll_list(poll_list, vsp_blend_frame_num, 1000);
		if (ret != HD_OK)
			continue;
		for (i = 0; i < vsp_blend_frame_num ;i++) {
			if (FALSE == poll_list[i].revent.event)
				continue;
			ret = hd_videoproc_pull_out_buf(p_stream[i]->proc_path, &tmp_video_frame, -1); // -1 = blocking mode
			if (ret != HD_OK)
				continue;
			// set base time when get first frame
			if (pull_frame_bit == 0) {
				base_time = tmp_video_frame.timestamp;
			} else {
				if (tmp_video_frame.timestamp > base_time + time_threshold) {
					printf("New come %d, drop olds => base_time %lld video_frame_time %lld\r\n", i, base_time, tmp_video_frame.timestamp);
					base_time = tmp_video_frame.timestamp;
					is_drop_old = TRUE;
					keep_idx = i;
					goto release_buf;
				}
				if (tmp_video_frame.timestamp < base_time - time_threshold) {
					printf("drop old => base_time %lld video_frame_time %lld\r\n", i, base_time, tmp_video_frame.timestamp);
					hd_videoproc_release_out_buf(p_stream[i]->proc_path, &tmp_video_frame);
					continue;
				}
			}
			if (pull_frame_bit & (1 << i)) {
				hd_videoproc_release_out_buf(p_stream[i]->proc_path, &video_frame[i]);
			}
			video_frame[i] = tmp_video_frame;
			pull_frame_bit |= (1 << i);
			// printf("base_time %lld pull_frame_bit = 0x%x\r\n", base_time, pull_frame_bit);
			// get all frame
			if (pull_frame_bit == frame_mask) {
				break;
			}
		}

		if (pull_frame_bit != frame_mask) {
			continue;
		}
		if (g_vsp_mode) {
			for (i = 0; i < vsp_blend_frame_num ; i++) {
				push_idx = i;
				frame_idx = i;
				video_frame[frame_idx].count = 0;
				video_frame[frame_idx].reserved[0] = MAKEFOURCC('V','S','P','E');
				/* set 2dlut out crop offset here */
				/* out_shift[0..7] */
				video_frame[frame_idx].reserved[3] = (UINT32)(overlap_shift_value[frame_idx]);
				video_frame[frame_idx].ddr_id |= (push_idx << HD_VIDEO_MULTI_FRAME_SHIFT);
				ret	= hd_videoproc_push_in_buf(p_vsp_stream->proc_path, &video_frame[frame_idx], NULL, 0);
			}
		} else {
			for (i = 0; i < vsp_blend_frame_num ;i++) {
				if (pull_frame_bit & (1 << i)) {
					frame_idx = i;
					hd_videoenc_push_in_buf(p_stream[i]->enc_path, &video_frame[frame_idx], NULL, 0);
				}
			}
		}
release_buf:
		for (i = 0; i < vsp_blend_frame_num ;i++) {
			if (pull_frame_bit & (1 << i)) {
				ret = hd_videoproc_release_out_buf(p_stream[i]->proc_path, &video_frame[i]);
				if (ret != HD_OK) {
					printf("release vsp in[%d] error !!\r\n", i);
				}
			}
		}
		if (is_drop_old) {
			pull_frame_bit = (1 << keep_idx);
			video_frame[keep_idx] = tmp_video_frame;
		} else {
			pull_frame_bit = 0;
		}
		is_drop_old = 0;
		//usleep(1000); //delay 1 ms
	}
	// release buffer
	for (i = 0; i < vsp_blend_frame_num ;i++) {
		if (pull_frame_bit & (1 << i)) {
			hd_videoproc_release_out_buf(p_stream[i]->proc_path, &video_frame[i]);
			if (ret != HD_OK) {
				printf("release vsp in[%d] error !!\r\n", i);
			}
		}
	}
	printf("exit flow\r\n");
	return 0;
}

static int  gfxscale(HD_VIDEO_FRAME *p_srcvdo, UINT32 dst_w, UINT32 dst_h, HD_VIDEO_FRAME *p_dstvdo)
{
	HD_GFX_SCALE         param = {0};
	UINT32               blk_size;
	HD_COMMON_MEM_VB_BLK blk;
	ULONG             	 pa;

	p_dstvdo->sign = MAKEFOURCC('V','F','R','M');
	p_dstvdo->dim.w = dst_w;
	p_dstvdo->dim.h = dst_h;
	p_dstvdo->loff[0]= dst_w;
	p_dstvdo->loff[1]= dst_w;
	p_dstvdo->pxlfmt = GFX_OUT_FMT;
	blk_size = p_dstvdo->loff[0] * dst_h * 3 / 2;
	//printf("blk_size = 0x%x\r\n", (int)blk_size);
	blk = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, blk_size, GFX_DDR_ID);
	if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("gfxscale get block fail\r\n");
		return -1;
	}
	pa = hd_common_mem_blk2pa(blk);
	p_dstvdo->blk = blk;
	p_dstvdo->phy_addr[0] = pa;
	p_dstvdo->phy_addr[1] = pa + (p_dstvdo->dim.h * p_dstvdo->loff[0]);
	p_dstvdo->count = p_srcvdo->count;
	p_dstvdo->timestamp = p_srcvdo->timestamp;
	param.src_img.dim.w            = p_srcvdo->dim.w;
	param.src_img.dim.h            = p_srcvdo->dim.h;
	param.src_img.format           = p_srcvdo->pxlfmt;
	param.src_img.p_phy_addr[0]    = p_srcvdo->phy_addr[0];
	param.src_img.lineoffset[0]    = p_srcvdo->loff[0];
	param.src_img.p_phy_addr[1]    = p_srcvdo->phy_addr[1];
	param.src_img.lineoffset[1]    = p_srcvdo->loff[1];
	param.dst_img.dim.w            = p_dstvdo->dim.w;
	param.dst_img.dim.h            = p_dstvdo->dim.h;
	param.dst_img.format           = p_dstvdo->pxlfmt;
	param.dst_img.p_phy_addr[0]    = p_dstvdo->phy_addr[0];
	param.dst_img.lineoffset[0]    = p_dstvdo->loff[0];
	param.dst_img.p_phy_addr[1]    = p_dstvdo->phy_addr[1];
	param.dst_img.lineoffset[1]    = p_dstvdo->loff[1];
	param.src_region.x             = 0;
	param.src_region.y             = 0;
	param.src_region.w             = param.src_img.dim.w;
	param.src_region.h             = param.src_img.dim.h;
	param.dst_region.x             = 0;
	param.dst_region.y             = 0;
	param.dst_region.w             = param.dst_img.dim.w;
	param.dst_region.h             = param.dst_img.dim.h;

	#if 0
	printf("src w = %d ,h = %d, linoff = %d, y_pa = 0x%lx, uv_pa = 0x%lx \r\n",
		   param.src_img.dim.w, param.src_img.dim.h, param.src_img.lineoffset[0],
		   p_srcvdo->phy_addr[0], p_srcvdo->phy_addr[1]);
	printf("dst w = %d ,h = %d, linoff = %d, y_pa = 0x%lx, uv_pa = 0x%lx \r\n",
		   param.dst_img.dim.w, param.dst_img.dim.h, param.dst_img.lineoffset[0],
		   p_dstvdo->phy_addr[0], p_dstvdo->phy_addr[1]);
	#endif

	vendor_gfx_scale_dma_flush(&param, 0);
	return 0;
}

static void *vsp_pull_out_thread(void *arg)
{
	#define MEASURE_LATENCY 0
	VIDEO_RECORD *p_stream[VSP_MAX_BLEND_FRAME_NUM + VSP_OUT_STREAM_CNT];

	HD_RESULT ret = HD_OK;
	HD_VIDEO_FRAME out_video_frame[VSP_OUT_STREAM_CNT];
	UINT64 frame_count = 1;

	p_stream[0] = (VIDEO_RECORD *)arg;
	p_stream[VSP_STREAM1_ID] = p_stream[0] + VSP_STREAM1_ID;
	p_stream[VSP_STREAM2_ID] = p_stream[0] + VSP_STREAM2_ID;
	p_stream[VSP_STREAM3_ID] = p_stream[0] + VSP_STREAM3_ID;

	#if defined(__LINUX)
	prctl(PR_SET_NAME, "pull_thread");
	{
		pthread_attr_t thread_attr;
	    struct sched_param sch_param;

	    pthread_attr_init(&thread_attr);
	    sch_param.sched_priority = 95;
	    pthread_attr_setschedpolicy(&thread_attr,SCHED_RR);
	    pthread_attr_setschedparam(&thread_attr,&sch_param);
	    pthread_attr_setinheritsched(&thread_attr,PTHREAD_EXPLICIT_SCHED);
	}
	#endif
	printf("\r\npull_thread....\r\n");
	//--------- pull data test ---------
	while (!g_quit) {
		//wait output finish
		ret = hd_videoproc_pull_out_buf(p_stream[VSP_STREAM1_ID]->proc_path, &out_video_frame[0], -1); // -1 = blocking mode, 0 = non-blocking mode, >0 = blocking-timeout mode
		if (ret != HD_OK) {
			printf("pull_out error = %d!! blk=0x%X\r\n", ret, out_video_frame[0].blk);
			continue;
		}

		#if MEASURE_LATENCY
		{
				UINT64 time_c;

				time_c = hd_gettime_us();
				if (frame_count % 30 == 0) {
				printf("encode time diff %d \r\n",
					    time_c-out_video_frame[0].timestamp
					    );
				}
		}
		#endif
		out_video_frame[0].count = frame_count;
		hd_videoenc_push_in_buf(p_stream[VSP_STREAM1_ID]->enc_path, &out_video_frame[0], NULL, 0);
		if (g_en_sub) {
			ret = gfxscale(&out_video_frame[0], STREAM2_SIZE_W, STREAM2_SIZE_H, &out_video_frame[1]);
			if (ret >= 0) {
				hd_videoenc_push_in_buf(p_stream[VSP_STREAM2_ID]->enc_path, &out_video_frame[1], NULL, 0);
			} else {
				goto scale_err;
			}
		}
		if (g_en_sub2) {
			ret = gfxscale(&out_video_frame[1], STREAM3_SIZE_W, STREAM3_SIZE_H, &out_video_frame[2]);
			if (ret >= 0) {
				hd_videoenc_push_in_buf(p_stream[VSP_STREAM3_ID]->enc_path, &out_video_frame[2], NULL, 0);
				ret = hd_common_mem_release_block(out_video_frame[2].blk);
				if (HD_OK != ret) {
					printf("release blk fail %d\r\n", ret);
				}
			}
		}
		if (g_en_sub) {
			ret = hd_common_mem_release_block(out_video_frame[1].blk);
			if (HD_OK != ret) {
				printf("release blk fail %d\r\n", ret);
			}
		}
scale_err:
		ret = hd_videoproc_release_out_buf(p_stream[VSP_STREAM1_ID]->proc_path, &out_video_frame[0]);
		if (ret != HD_OK) {
			printf("release vsp out[%d] error !!\r\n\r\n", 0);
		}
	}
	return 0;
}



static void *encode_thread(void *arg)
{
	VIDEO_RECORD *p_stream[VSP_MAX_BLEND_FRAME_NUM + VSP_OUT_STREAM_CNT]= {0};

	HD_RESULT ret = HD_OK;
	HD_VIDEOENC_BS  data_pull;
	UINT32 i, j;
	uintptr_t           vir_addr[VSP_MAX_BLEND_FRAME_NUM + VSP_OUT_STREAM_CNT]= {0};
	HD_VIDEOENC_BUFINFO phy_buf[VSP_MAX_BLEND_FRAME_NUM + VSP_OUT_STREAM_CNT]= {0};
	char file_path[VSP_MAX_BLEND_FRAME_NUM + VSP_OUT_STREAM_CNT][32] = {0};
	FILE *f_out[VSP_MAX_BLEND_FRAME_NUM + VSP_OUT_STREAM_CNT];
	#define STREAMI_PHY2VIRT(i, pa) (vir_addr[i] + (pa - phy_buf[i].buf_info.phy_addr))


	#if defined(__LINUX)
	prctl(PR_SET_NAME, "encode_thd");
	#endif

	HD_VIDEOENC_POLL_LIST poll_list[VSP_MAX_BLEND_FRAME_NUM + VSP_OUT_STREAM_CNT];
	UINT32 poll_num = vcap_num + VSP_OUT_STREAM_CNT;


	p_stream[0] = (VIDEO_RECORD *)arg;
	for (i = 1; i < vcap_num + VSP_OUT_STREAM_CNT; i++) {
		p_stream[i] = p_stream[i-1] + 1;
	}


	for (i = 0; i < vcap_num + VSP_OUT_STREAM_CNT;i++) {
		// query physical address of bs buffer ( this can ONLY query after hd_videoenc_start() is called !! )
		hd_videoenc_get(p_stream[i]->enc_path, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf[i]);
		// mmap for bs buffer (just mmap one time only, calculate offset to virtual address later)
		vir_addr[i]  = (uintptr_t)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf[i].buf_info.phy_addr, phy_buf[i].buf_info.buf_size);
		//----- open output files -----
		if (i < vcap_num) {
			snprintf(file_path[i], 32, "/mnt/sd/vsp_sensor%d.dat", i);
		} else {
			snprintf(file_path[i], 32, "/mnt/sd/vsp_out_stream%d.dat", i - vsp_blend_frame_num);
		}
		if ((f_out[i] = fopen(file_path[i], "wb")) == NULL) {
			HD_VIDEOENC_ERR("open file (%s) fail....\r\n", file_path[i]);
		} else {
			printf("\r\ndump bitstream %d to file (%s) ....\r\n", i, file_path[i]);
		}
		//--------- pull data test ---------
		poll_list[i].path_id = p_stream[i]->enc_path;
	}
	printf("\r\nif you want to stop, enter \"q\" to exit !!\r\n\r\n");


	//--------- pull data test ---------
	while (p_stream[0]->enc_exit == 0) {
		if (g_enc_poll && HD_OK == hd_videoenc_poll_list(poll_list, poll_num, 1000)) {    // multi path poll_list , -1 = blocking mode
			for (i = 0; i < vcap_num + VSP_OUT_STREAM_CNT; i++) {
				if (TRUE == poll_list[i].revent.event) {
					//pull data
					ret = hd_videoenc_pull_out_buf(p_stream[i]->enc_path, &data_pull, -1); // -1 = blocking mode

					if (ret == HD_OK) {
						for (j=0; j< data_pull.pack_num; j++) {
							#if FUNC_WRITE_FILE
							UINT8 *ptr = (UINT8 *)STREAMI_PHY2VIRT(i, data_pull.video_pack[j].phy_addr);
							UINT32 len = data_pull.video_pack[j].size;
							if (g_write_file) {
								if (f_out[i]) fwrite(ptr, 1, len, f_out[i]);
								if (f_out[i]) fflush(f_out[i]);
							}
							#endif
						}

						// release data
						ret = hd_videoenc_release_out_buf(p_stream[i]->enc_path, &data_pull);
					}
				}

			}
		} else {
			usleep(100000);
		}
	}
	for (i = 0; i < vcap_num + VSP_OUT_STREAM_CNT;i++) {
		// mummap for bs buffer
		hd_common_mem_munmap((void *)vir_addr[i], phy_buf[i].buf_info.buf_size);
		// close output file
		if (f_out[i]) {
			fclose(f_out[i]);
		}
	}
	system("sync");
	return 0;
}

static HD_RESULT parse_vsp_info(void)
{
	FILE     *fp = NULL;
    char tmp_str[50];
    UINT32 frm_idx, i;
    UINT32 max_ovlp_width = 0;

    if ((fp = fopen("/mnt/sd/vsp/camview_anchors.txt", "rb")) == NULL) {
		printf(" open %scamview_anchors.txt fail, use default value !!....\r\n !!",VSP_DATA_PATH);
    } else {
	    while (fscanf(fp, "%[^\n ] ", tmp_str) != EOF) {
	    	if (strncmp(tmp_str,"VSP_VPE_2D_SCALE_OUT_W_1", 24) == 0) {
				fscanf(fp, "%d\n", &vsp_vpe_2d_scl_out_w[0]);
	    	} else if (strncmp(tmp_str,"VSP_VPE_2D_SCALE_OUT_W_2", 24) == 0) {
				fscanf(fp, "%d\n", &vsp_vpe_2d_scl_out_w[1]);
	    	} else if (strncmp(tmp_str,"VSP_VPE_2D_SCALE_OUT_W_3", 24) == 0) {
				fscanf(fp, "%d\n", &vsp_vpe_2d_scl_out_w[2]);
	    	} else if (strncmp(tmp_str,"VSP_VPE_2D_SCALE_OUT_W_4", 24) == 0) {
				fscanf(fp, "%d\n", &vsp_vpe_2d_scl_out_w[3]);

	    	} else if (strncmp(tmp_str,"VSP_VPE_2D_SCALE_OUT_H", 24) == 0) {
				fscanf(fp, "%d\n", &vsp_vpe_2d_scl_out_h);
	    	} else if (strncmp(tmp_str,"VCAP_NUM", 8) == 0) {
				fscanf(fp, "%d\n", &vcap_num);
				if (vcap_num > VSP_MAX_BLEND_FRAME_NUM) {
					printf("vcap_num %d > limit %d, truncate to %d\r\n",
						vcap_num, VSP_MAX_BLEND_FRAME_NUM, VSP_MAX_BLEND_FRAME_NUM);
					vcap_num = VSP_MAX_BLEND_FRAME_NUM;
				}
	    	} else if (strncmp(tmp_str,"VSP_SENSOR_MASK", 15) == 0) {
				fscanf(fp, "%x\n", &vsp_sensor_mask);

	    	} else if (strncmp(tmp_str,"VSP_CHANNEL_NUM", 16) == 0) {
				fscanf(fp, "%d\n", &vsp_blend_frame_num);
				if (vsp_blend_frame_num > VSP_MAX_BLEND_FRAME_NUM) {
					printf("vsp_blend_frame_num %d > limit %d, truncate to %d\r\n",
						vsp_blend_frame_num, VSP_MAX_BLEND_FRAME_NUM, VSP_MAX_BLEND_FRAME_NUM);
					vsp_blend_frame_num = VSP_MAX_BLEND_FRAME_NUM;
				}
	    	} else if (strncmp(tmp_str,"VSP_LUT_TBL_SZ", 16) == 0) {
				fscanf(fp, "%d\n", &vsp_lut_tbl_sz);
	    	} else if (strncmp(tmp_str,"VSP_OVERLAP_W_1", 16) == 0) {
				fscanf(fp, "%d\n", &vsp_ovlp_w[0]);
	    	} else if (strncmp(tmp_str,"VSP_OVERLAP_W_2", 16) == 0) {
				fscanf(fp, "%d\n", &vsp_ovlp_w[1]);
	    	} else if (strncmp(tmp_str,"VSP_OVERLAP_W_3", 16) == 0) {
				fscanf(fp, "%d\n", &vsp_ovlp_w[2]);
	    	} else if (strncmp(tmp_str,"VSP_OVERLAP_W_4", 16) == 0) {
				fscanf(fp, "%d\n", &vsp_ovlp_w[3]);

	    	} else if (strncmp(tmp_str,"VSP_OUT_SIZE_W", 16) == 0) {
				fscanf(fp, "%d\n", &vsp_out_size_w);
	    	} else if (strncmp(tmp_str,"VSP_OUT_SIZE_H", 16) == 0) {
				fscanf(fp, "%d\n", &vsp_out_size_h);
	    	} else if (strncmp(tmp_str,"VSP_LUT_TBL_CAM_1", 20) == 0) {
				fscanf(fp, "%[^\n ] ", tmp_str);
				snprintf(lut_bin[0], 50, VSP_DATA_PATH"%s", tmp_str);
	    	} else if (strncmp(tmp_str,"VSP_LUT_TBL_CAM_2", 20) == 0) {
				fscanf(fp, "%[^\n ] ", tmp_str);
				snprintf(lut_bin[1], 50, VSP_DATA_PATH"%s", tmp_str);
	    	} else if (strncmp(tmp_str,"VSP_LUT_TBL_CAM_3", 20) == 0) {
				fscanf(fp, "%[^\n ] ", tmp_str);
				snprintf(lut_bin[2], 50, VSP_DATA_PATH"%s", tmp_str);
	    	} else if (strncmp(tmp_str,"VSP_LUT_TBL_CAM_4", 20) == 0) {
				fscanf(fp, "%[^\n ] ", tmp_str);
				snprintf(lut_bin[3], 50, VSP_DATA_PATH"%s", tmp_str);
	    	}
	    }
		// backward compatible
		if (vcap_num == 0) {
			vcap_num = vsp_blend_frame_num;
		}
		printf("vcap_num = %d, vsp_blend_frame_num = %d, vsp_sensor_mask = 0x%x\r\n",
			vcap_num, vsp_blend_frame_num, vsp_sensor_mask);

		for (i = 0; i < vsp_blend_frame_num; i ++) {
			if (vsp_ovlp_w[i] > (dewarp_out_x_ofs_max[i] + 96)) {
				vsp_ovlp_w[i] = vsp_ovlp_w[i] - dewarp_out_x_ofs_max[i];
				printf("ofs_max[%d] = %d, vsp_ovlp_w[%d] is change to %d\r\n", i, dewarp_out_x_ofs_max[i], i,vsp_ovlp_w[i]);
			}
		}

#if FUNC_FUSION
#else
		vsp_ovlp_w[0] = 0;
		vsp_ovlp_w[1] = 0;
		vsp_ovlp_w[2] = 0;
		vsp_ovlp_w[3] = 0;
		vsp_out_size_w = vsp_vpe_2d_scl_out_w[0] + vsp_vpe_2d_scl_out_w[1] + vsp_vpe_2d_scl_out_w[2] + vsp_vpe_2d_scl_out_w[3];
#endif
		vsp_ovlp_h = vsp_vpe_2d_scl_out_h;
		max_ovlp_width = MAX_VAL(MAX_VAL(MAX_VAL(vsp_ovlp_w[0], vsp_ovlp_w[1]),vsp_ovlp_w[2]),vsp_ovlp_w[3]);
		dre_work_buf_size = (ALIGN_CEIL_16(max_ovlp_width)*vsp_ovlp_h*4);
		vsp_vpe_out2_h = vsp_vpe_2d_scl_out_h;
		vsp_out_size_h = vsp_vpe_2d_scl_out_h;
		vsp_vpe_out2_w = ((vsp_ovlp_w[0]+vsp_ovlp_w[1]+vsp_ovlp_w[2]+vsp_ovlp_w[3])*2);

		for (frm_idx=0; frm_idx< vsp_blend_frame_num; frm_idx++) {
			if (strncmp(lut_bin[frm_idx],"", 16) == 0) {
				printf("total vsp frame num is %d, plz input %d 2dlut table\r\n", vsp_blend_frame_num, vsp_blend_frame_num);
				printf("idx: %d 2dlut table NULL\r\n", frm_idx);
			}
		}
		fclose(fp);
    }
#if 0
printf("\r\n\r\n");
printf(" vsp_vpe_out1 (%d, %d), blend_num %d, tbl_sz %d \r\n", vsp_vpe_2d_scl_out_w, vsp_vpe_2d_scl_out_h, vsp_blend_frame_num, vsp_lut_tbl_sz);
printf(" ovlp_sz w(%d, %d, %d, %d), h %d \r\n", vsp_ovlp_w[0], vsp_ovlp_w[1], vsp_ovlp_w[2], vsp_ovlp_w[3], vsp_ovlp_h);
printf(" vsp_vpe_out2_ (%d, %d) \r\n", vsp_vpe_out2_w, vsp_vpe_out2_h);
printf(" vsp_out (%d, %d) \r\n", vsp_out_size_w, vsp_out_size_h);
printf("\r\n\r\n");
#endif
	return HD_OK;
}


EXAMFUNC_ENTRY(hd_video_record_with_vsp, argc, argv)
{
	HD_RESULT ret;
	INT key;
	VIDEO_RECORD stream[VSP_MAX_BLEND_FRAME_NUM + VSP_OUT_STREAM_CNT] = {0}; //0: sensor 1, 1: sensor2, 2: sensor3, 3: sensor4, 4: vsp stream1, vout, 5: vsp stream2, 6: vsp stream3,
    //HD_DIM  sub_dim;
    HD_DIM  main_dim;
	HD_DIM  sub_dim;
	HD_DIM  vsp_out_dim;
	UINT32  enc_type = 1; //H264
	UINT32  tmp_fmt = 0;
	PD_SHM_INFO  *p_pd_shm;
	UINT32 i;

	#if AI_ENABLE
	//net proc
	NET_PROC_CONFIG net_cfg = {
		.model_filename = "/mnt/sd/para/nvt_model.bin",
		.label_filename = "/mnt/sd/accuracy/labels.txt"
	};
	#endif


	// query program options
	if (argc == 1) {
		printf("Usage: <fps> <ipp out fmt> <enable sub> <enable sub2> <enc_type>.\r\n");
		printf("Help:\r\n");
		printf("  <fps>  : 20,25,30\r\n");
		printf("  <ipp out fmt>  : 0(YUV420), others(YCC)\r\n");
		printf("  <enable sub stream>  : 0(disable), 1(enable)\r\n");
		printf("  <enable sub stream2>  : 0(disable), 1(enable)\r\n");
		printf("  <enc_type>  : 0(H265), 1(H264), 2(MJPG)\r\n");
		return 0;
	}
	if (argc >= 2) {
		g_fps = atoi(argv[1]);
	}
	if (argc >= 3) {
		tmp_fmt = atoi(argv[2]);
		if (tmp_fmt == 0) {
			g_ipp_out_fmt = HD_VIDEO_PXLFMT_YUV420;
		} else {
			g_ipp_out_fmt = HD_VIDEO_PXLFMT_YUV420_NVX2;
		}
	}
	if (argc >= 4) {
		g_en_sub = atoi(argv[3]);
	}
	if (argc >= 5) {
		g_en_sub2 = atoi(argv[4]);
	}
	if (argc >= 6) {
		enc_type = atoi(argv[5]);
	}
	printf("g_fps %d\r\n", g_fps);
	if (g_en_sub2) {
		// USE ISE scale, only support 420
		g_en_sub = 1;
		g_ipp_out_fmt = HD_VIDEO_PXLFMT_YUV420;
	}
	if (g_ipp_out_fmt == HD_VIDEO_PXLFMT_YUV420) {
		printf("ipp_out_fmt 420\r\n");
	} else {
		printf("ipp_out_fmt YCC\r\n");
	}
	printf("g_en_sub %d\r\n", g_en_sub);
	printf("g_en_sub2 %d\r\n", g_en_sub2);
	init_share_memory();

	//parsing paramters from txt file
	if (parse_vsp_info() != HD_OK) {
		printf("parsing vsp info from file fail\n");
		return 0;
	}

	// init hdal
	ret = hd_common_init(0);
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
		goto exit;
	}
	hd_common_sysconfig(HD_VIDEOPROC_CFG, 0, HD_VIDEOPROC_CFG_DISABLE_GDC, 0);
	hd_common_sysconfig(0, (1<<16), 0, 0x000f0000); //enable AI engine
	// init memory
	ret = mem_init();
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
		goto exit;
	}

	#if AI_ENABLE
	stream[0].net_path = 0;
	stream[1].net_path = 1;

	network_mem_config(stream[0].net_path, &net_cfg);
	network_mem_config(stream[1].net_path, &net_cfg);
	#endif


	// init all modules
	ret = init_module();
	if (ret != HD_OK) {
		printf("init fail=%d\n", ret);
		goto exit;
	}

	//isp init
	ret = vendor_isp_init();
	if (ret != HD_OK) {
		printf("vendor_isp_init fail=%d\n", ret);
	}

	ret = vendor_vpe_init();
	if (ret != HD_OK) {
		printf("vendor_vpe_init fail=%d\n", ret);
	}

	// open all sensors
	for (i = 0;i < vcap_num; i++) {
		stream[i].proc_max_dim.w = VDO_SIZE_W; //assign by user
		stream[i].proc_max_dim.h = VDO_SIZE_H; //assign by user
	    stream[i].wait_ms = -1; //blocking mode
		ret = open_module_sen(&stream[i], &stream[i].proc_max_dim, i);
		if (ret != HD_OK) {
			printf("open_sen%d fail=%d\n", i, ret);
			goto exit;
		}
	}

	stream[VSP_STREAM1_ID].proc_max_dim.w = vsp_out_size_w; //assign by user
	stream[VSP_STREAM1_ID].proc_max_dim.h = vsp_out_size_h; //assign by user
	ret = open_module_vsp_stream1(&stream[VSP_STREAM1_ID], &stream[VSP_STREAM1_ID].proc_max_dim, VSP_STREAM1_ID);
	if (ret != HD_OK) {
		printf("open%d fail=%d\n", VSP_STREAM1_ID, ret);
		goto exit;
	}

	stream[VSP_STREAM2_ID].proc_max_dim.w = vsp_out_size_w; //assign by user
	stream[VSP_STREAM2_ID].proc_max_dim.h = vsp_out_size_h; //assign by user
	ret = open_module_vsp_stream2(&stream[VSP_STREAM2_ID], &stream[VSP_STREAM2_ID].proc_max_dim, VSP_STREAM2_ID);
	if (ret != HD_OK) {
		printf("open%d fail=%d\n", VSP_STREAM2_ID, ret);
		goto exit;
	}


	stream[VSP_STREAM3_ID].proc_max_dim.w = STREAM3_SIZE_W; //assign by user
	stream[VSP_STREAM3_ID].proc_max_dim.h = STREAM3_SIZE_H; //assign by user
	ret = open_module_vsp_stream3(&stream[VSP_STREAM3_ID], &stream[VSP_STREAM3_ID].proc_max_dim, VSP_STREAM3_ID);
	if (ret != HD_OK) {
		printf("open%d fail=%d\n", VSP_STREAM3_ID, ret);
		goto exit;
	}

	for (i = 0;i < vcap_num; i++) {
		// get videocap capability (sensor 1st)
		ret = get_cap_caps(stream[i].cap_ctrl, &stream[i].cap_syscaps);
		if (ret != HD_OK) {
			printf("get cap%d_caps fail=%d\n", i, ret);
			goto exit;
		}
	}

	for (i = 0;i < vcap_num; i++) {
		// set videocap parameter
		stream[i].cap_dim.w = VDO_SIZE_W; //assign by user
		stream[i].cap_dim.h = VDO_SIZE_H; //assign by user
		ret = set_cap_param(stream[i].cap_path, &stream[i].cap_dim, vcap_out_fmt[i]);
		if (ret != HD_OK) {
			printf("set cap%d fail=%d\n", i, ret);
			goto exit;
		}
	}

	for (i = 0;i < vcap_num; i++) {
		// set videoproc parameter
		main_dim.w = VDO_SIZE_W;
		main_dim.h = VDO_SIZE_H;
		ret = set_proc_param(stream[i].proc_path, &main_dim, NULL, g_ipp_out_fmt);
		if (ret != HD_OK) {
			printf("set proc%d fail=%d\n", i, ret);
			goto exit;
		}
	}

	// set vsp videoproc parameter
	vsp_out_dim.w = vsp_out_size_w;
	vsp_out_dim.h = vsp_out_size_h;
	ret = set_vsp_proc_param(stream[VSP_STREAM1_ID].proc_path, &vsp_out_dim);
	if (ret != HD_OK) {
		printf("set proc5 fail=%d\n", ret);
		goto exit;
	}
	// set vpe vproc parameter
	sub_dim.w = STREAM2_SIZE_W;
	sub_dim.h = STREAM2_SIZE_H;
	ret = set_vpe_proc_param(stream[VSP_STREAM2_ID].proc_path, &sub_dim, VSP_OUT_FMT);
	if (ret != HD_OK) {
		printf("set proc6 fail=%d\n", ret);
		goto exit;
	}

	for (i = 0;i < vcap_num; i++) {
		// set videoenc config
		stream[i].enc_max_dim.w = main_dim.w;
		stream[i].enc_max_dim.h = main_dim.h;
		ret = set_enc_cfg(stream[i].enc_path, &stream[i].enc_max_dim, vdo_br, sen_vcap_id[i]);
		if (ret != HD_OK) {
			printf("set enc-cfg fail=%d\n", ret);
			goto exit;
		}
		// set videoenc parameter
		stream[i].enc_dim.w = main_dim.w;
		stream[i].enc_dim.h = main_dim.h;
		ret = set_enc_param(stream[i].enc_path, &stream[i].enc_dim, enc_type, vdo_br);
		if (ret != HD_OK) {
			printf("set enc fail=%d\n", ret);
			goto exit;
		}
	}

	// set videoenc config (vsp out main, stream1)
	stream[VSP_STREAM1_ID].enc_max_dim.w = vsp_out_size_w;
	stream[VSP_STREAM1_ID].enc_max_dim.h = vsp_out_size_h;
	ret = set_enc_cfg(stream[VSP_STREAM1_ID].enc_path, &stream[VSP_STREAM1_ID].enc_max_dim, stream1_br, stream[VSP_STREAM1_ID].isp_id);
	if (ret != HD_OK) {
		printf("set enc-cfg fail=%d\n", ret);
		goto exit;
	}

	// set videoenc parameter (vsp out main, stream1)
	stream[VSP_STREAM1_ID].enc_dim.w = vsp_out_size_w;
	stream[VSP_STREAM1_ID].enc_dim.h = vsp_out_size_h;
	ret = set_enc_param(stream[VSP_STREAM1_ID].enc_path, &stream[VSP_STREAM1_ID].enc_dim, enc_type, stream1_br);
	if (ret != HD_OK) {
		printf("set enc fail=%d\n", ret);
		goto exit;
	}

	// set videoenc config (stream2)
	stream[VSP_STREAM2_ID].enc_max_dim.w = STREAM2_SIZE_W;
	stream[VSP_STREAM2_ID].enc_max_dim.h = STREAM2_SIZE_H;
	ret = set_enc_cfg(stream[VSP_STREAM2_ID].enc_path, &stream[VSP_STREAM2_ID].enc_max_dim, stream2_br, stream[VSP_STREAM2_ID].isp_id);
	if (ret != HD_OK) {
		printf("set enc-cfg fail=%d\n", ret);
		goto exit;
	}

	// set videoenc parameter (stream2)
	stream[VSP_STREAM2_ID].enc_dim.w = STREAM2_SIZE_W;
	stream[VSP_STREAM2_ID].enc_dim.h = STREAM2_SIZE_H;
	ret = set_enc_param(stream[VSP_STREAM2_ID].enc_path, &stream[VSP_STREAM2_ID].enc_dim, enc_type, stream2_br);
	if (ret != HD_OK) {
		printf("set enc fail=%d\n", ret);
		goto exit;
	}

	// set videoenc config (stream3)
	stream[VSP_STREAM3_ID].enc_max_dim.w = STREAM3_SIZE_W;
	stream[VSP_STREAM3_ID].enc_max_dim.h = STREAM3_SIZE_H;
	ret = set_enc_cfg(stream[VSP_STREAM3_ID].enc_path, &stream[VSP_STREAM3_ID].enc_max_dim, stream3_br, stream[VSP_STREAM3_ID].isp_id);
	if (ret != HD_OK) {
		printf("set enc-cfg fail=%d\n", ret);
		goto exit;
	}

	// set videoenc parameter (stream3)
	stream[VSP_STREAM3_ID].enc_dim.w = STREAM3_SIZE_W;
	stream[VSP_STREAM3_ID].enc_dim.h = STREAM3_SIZE_H;
	ret = set_enc_param(stream[VSP_STREAM3_ID].enc_path, &stream[VSP_STREAM3_ID].enc_dim, enc_type, stream3_br);
	if (ret != HD_OK) {
		printf("set enc fail=%d\n", ret);
		goto exit;
	}

	// NOTE: set isp cfg
	set_isp_cfg();


	#if AI_ENABLE
	network_start(&stream[0]);
	network_start(&stream[1]);
	#endif

	for (i = 0;i < vcap_num; i++) {
		// bind modules
		hd_videocap_bind(HD_VIDEOCAP_OUT(sen_vcap_id[i], 0), HD_VIDEOPROC_IN(i, 0));
		// start modules
		hd_videocap_start(stream[i].cap_path);
		hd_videoproc_start(stream[i].proc_path);
		hd_videoenc_start(stream[i].enc_path);
	}

	// start vsp
	hd_videoproc_start(stream[VSP_STREAM1_ID].proc_path);
	// start stream1
	hd_videoenc_start(stream[VSP_STREAM1_ID].enc_path);

	// start stream2
	hd_videoproc_start(stream[VSP_STREAM2_ID].proc_path);
	hd_videoenc_start(stream[VSP_STREAM2_ID].enc_path);

	// start stream3
	hd_videoenc_start(stream[VSP_STREAM3_ID].enc_path);
	if (g_ipp_out_fmt == HD_VIDEO_PXLFMT_YUV420_MB4 || g_ipp_out_fmt == HD_VIDEO_PXLFMT_YUV420_NVX5) {
		set_2a_stitch(1);
		g_vsp_mode = 1;
		printf("\r\n MB format, Force enable vsp mode\r\n");
	}
	ret = pthread_create(&stream[VSP_STREAM1_ID].vsp_thread_id, NULL, vsp_thread, &stream[0]);
	if (ret < 0) {
		printf("create vsp_thread failed\r\n");
		goto exit;
	}
	ret = pthread_create(&stream[VSP_STREAM1_ID].pull_thread_id, NULL, vsp_pull_out_thread, &stream[0]);
	if (ret < 0) {
		printf("create vsp_pull_out_thread failed\r\n");
		goto exit;
	}
	// create encode_thread (pull_out bitstream)
	ret = pthread_create(&stream[0].enc_thread_id, NULL, encode_thread, (void *)stream);
	if (ret < 0) {
		printf("create encode thread failed\r\n");
		goto exit;
	}

	#if AI_ENABLE
	if ((ret = network_bind_cb(&stream[0])) != HD_OK) {
		// quit program
		goto exit;
	}
	ret = network_set_isp_ai_cfg(&stream[0], 0, TRUE);
	if (ret != HD_OK) {
		// quit program
		printf("set isp_ai fail=%d\n", ret);
		//goto exit;
	}

	if ((ret = network_bind_cb(&stream[1])) != HD_OK) {
		// quit program
		//goto exit;
	}
	ret = network_set_isp_ai_cfg(&stream[1], 0, TRUE);
	if (ret != HD_OK) {
		// quit program
		printf("set isp_ai 2 fail=%d\n", ret);
		//goto exit;
	}
	#endif

	// query user key
	printf("Enter q to exit\n");
	printf("Enter d to debug\n");
	printf("Enter 0 to disable vsp mode\n");
	printf("Enter 1 to enable vsp mode\n");
	printf("Enter 3 to restart vsp\n");
	printf("Enter 8 to stop write file\n");
	printf("Enter 9 to start write file\n");
	printf("Enter a to stop encode poll\n");
	printf("Enter b to start encode poll\n");
	printf("Enter z to start rtsp\n");
	while (1) {
		key = GETCHAR();
		if (key == 'q' || key == 0x3) {
			// let vsp_thread, pull_thread stop loop and exit
			g_quit = 1; // stop thread
			// quit program
			stream[0].enc_exit = 1;
			// notify child process
			p_pd_shm = (PD_SHM_INFO  *)g_shm;
			p_pd_shm->exit = 1;
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
		if (key == '0') {
			set_2a_stitch(0);
			g_vsp_mode = 0;
			printf("\r\nDisable vsp mode\r\n");
		}
		if (key == '1') {
			set_2a_stitch(1);
			g_vsp_mode = 1;
			printf("\r\nEnable vsp mode\r\n");
		}
		if (key == '2') {
			printf("\r\nStart vcap\r\n");
			for (i = 0;i < vcap_num; i++) {
				hd_videocap_start(stream[i].cap_path);
			}
		}
		if (key == '3') {
			printf("\r\n restart vsp\r\n");
			hd_videoproc_stop(stream[4].proc_path);
			hd_videoproc_close(stream[4].proc_path);
			//open_module_5(&stream[4], &stream[4].proc_max_dim, VDOOUT0_TYPE);
			hd_videoproc_open(HD_VIDEOPROC_4_IN_0, HD_VIDEOPROC_4_OUT_0, &stream[4].proc_path);
			hd_videoproc_start(stream[4].proc_path);
		}
		if (key == '8') {
			g_write_file = 0;
			printf("stop write file\r\n");
		}
		if (key == '9') {
			g_write_file = 1;
			printf("start write file\r\n");
		}
		if (key == 'a') {
			g_enc_poll = 0;
			printf("stop encode poll\r\n");
		}
		if (key == 'b') {
			g_enc_poll = 1;
			printf("start encode poll\r\n");
		}
		if (key == 'z') {
			g_enc_poll = 0;
			system("nvtrtspd_ipc &");
			printf("nvtrtspd_ipc\r\n");
		}


		if (key == 'm') {
			network_set_isp_ai_cfg(&stream[0], 0, FALSE);
			network_set_isp_ai_cfg(&stream[1], 0, FALSE);
			printf("disable\r\n");
		}

		if (key == 'n') {
			network_set_isp_ai_cfg(&stream[0], 0, TRUE);
			network_set_isp_ai_cfg(&stream[1], 0, TRUE);
			printf("enable\r\n");
		}

	}

	#if AI_ENABLE
	network_stop(&stream[0]);
	network_stop(&stream[1]);
	#endif

	for (i = 0;i < vcap_num; i++) {
		hd_videocap_stop(stream[i].cap_path);
		hd_videoproc_stop(stream[i].proc_path);
		hd_videoenc_stop(stream[i].enc_path);
	}

	// stop vsp
	hd_videoproc_stop(stream[VSP_STREAM1_ID].proc_path);
	// stop vsp stream1
	hd_videoenc_stop(stream[VSP_STREAM1_ID].enc_path);

	// stop vsp stream2
	hd_videoproc_stop(stream[VSP_STREAM2_ID].proc_path);
	hd_videoenc_stop(stream[VSP_STREAM2_ID].enc_path);

	// stop vsp stream3
	hd_videoenc_stop(stream[VSP_STREAM3_ID].enc_path);

	// destroy threads
	pthread_join(stream[0].enc_thread_id, NULL);
	pthread_join(stream[VSP_STREAM1_ID].vsp_thread_id, (void* )NULL);
	// unbind  modules (sensor 1st)
	for (i = 0;i < vcap_num; i++) {
		// unbind  modules
		hd_videocap_unbind(HD_VIDEOCAP_OUT(sen_vcap_id[i], 0));
	}

exit:
	for (i = 0;i < vcap_num; i++) {
		ret = close_module(&stream[i]);
		if (ret != HD_OK) {
			printf("close stream %d fail=%d\n", i, ret);
		}

		ret = close_module2(&stream[i]);
		if (ret != HD_OK) {
			printf("close stream %d fail=%d\n", i, ret);
		}
	}

	// close modules (vsp)
	ret = close_module(&stream[VSP_STREAM1_ID]);
	if (ret != HD_OK) {
		//printf("close fail=%d\n", ret);
	}
	release_all_fusion_blk(&stream[VSP_STREAM1_ID]);
	// close modules
	ret = close_module(&stream[VSP_STREAM2_ID]);
	if (ret != HD_OK) {
		//printf("close fail=%d\n", ret);
	}

	// close modules
	ret = close_module(&stream[VSP_STREAM3_ID]);
	if (ret != HD_OK) {
		//printf("close fail=%d\n", ret);
	}

	// uninit all modules
	ret = exit_module();
	if (ret != HD_OK) {
		printf("exit fail=%d\n", ret);
	}


	// uninit isp
	ret = vendor_isp_uninit();
	if (ret != HD_OK) {
		printf("vendor_isp_uninit fail=%d\n", ret);
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
	exit_share_memory();
	return 0;
}
