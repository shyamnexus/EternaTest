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
#include "vendor_videoout.h"
#include "vendor_common.h"
#include "vendor_vpe.h"
#include "vendor_isp.h"
#include "smart_stitch2_lib.h"
#include "vendor_gfx.h"
#include <kwrap/examsys.h> 	//for MAIN(), GETCHAR() API
#include <pd_shm.h>
#include "nvtinfo.h"
#include "vsp_dec_func.h"

// platform dependent
#if defined(__LINUX)
#define MAIN(argc, argv) 		int main(int argc, char** argv)
#define GETCHAR()				getchar()
#include <sys/prctl.h>
#include "math.h"
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
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
#define SEN2_VCAP_ID 3
#define SEN3_VCAP_ID 6
#define SEN4_VCAP_ID 9

#define SEN1_PINMUX  10
#define SEN2_PINMUX  11
#define SEN3_PINMUX  12
#define SEN4_PINMUX  13

#define SEN1_CSI_ID  0
#define SEN2_CSI_ID  2
#define SEN3_CSI_ID  4
#define SEN4_CSI_ID  6
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

// MB Info
#define VDO_H264_MBINFO_BUFSIZE(w, h)        (w * h * 5 / 16)
#define VDO_H265_MBINFO_BUFSIZE(w, h)        (w * h / 16)

// Side Info
/* SideInfo line-offset */
#define VDO_SIDEINFO_STRIDE_TMP(x)        ((ALIGN_CEIL_64((x))/64) * 16)
#define VDO_SIDEINFO_LINE_OFFSET(x)        ALIGN_CEIL(VDO_SIDEINFO_STRIDE_TMP(x), 128)/* Stride128 */
/* SideInfo size */
#define VDO_SIDEINFO_BUF_H(y)             (ALIGN_CEIL_64((y))/8)
#define VDO_SIDEINFO_SIZE(x,y)            (VDO_SIDEINFO_LINE_OFFSET((x)) * VDO_SIDEINFO_BUF_H((y)))

///////////////////////////////////////////////////////////////////////////////

#define FUNC_WRITE_FILE		1
#define FUNC_PTZ		    0
#define FUNC_FUSION		    1
#define SMART_STITCH        1
#define SMART_STITCH_LITE_EN   0
#define SMART_STITCH_LITE_Y_ALIGNMENT_EN   0
#define MEASURE_LATENCY     0
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
#define ETH_OUT_SEL		    1 //0: full, 1: subsample 1/2

#define VDOOUT0_TYPE   1 //1 for LCD, 2 for HDMI, 0 for TV
#define VDOOUT1_TYPE   2 //1 for LCD, 2 for HDMI, 0 for TV

#define VDOOUT_HDMI  HD_VIDEOOUT_HDMI_1920X1080I60

#define MCLK_SRC_SYNC_FUNC    0

#define USE_REAL_SENSOR       1    // set 0 for patgen

#define USE_PLAYBACK_FLOW     1

#define ENABLE_2A             1



#define VSP_OUT_STREAM_CNT       3
#define VSP_MAX_BLEND_FRAME_NUM  4

UINT32 sen_vcap_id[VSP_MAX_BLEND_FRAME_NUM] = { 0, 2, -1, -1};
UINT32 sen_pinmux[VSP_MAX_BLEND_FRAME_NUM]  = { 0, 1, -1, -1};
UINT32 sen_csi_id[VSP_MAX_BLEND_FRAME_NUM]  = { 0, 2, -1, -1};
UINT32 vcap_out_fmt[VSP_MAX_BLEND_FRAME_NUM] = { HD_VIDEO_PXLFMT_NRX12,
												 HD_VIDEO_PXLFMT_NRX12,
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
UINT32 vsp_lut_tbl_sz = 257;


//--- vsp_blend_frame_num
#define MAX_VSP_BLEND_FRAME_NUM 4
UINT32 vsp_blend_frame_num = MAX_VSP_BLEND_FRAME_NUM;
UINT32 vsp_overlap_num = MAX_VSP_BLEND_FRAME_NUM;


UINT32 vsp_ovlp_w[VSP_MAX_BLEND_FRAME_NUM] = {0};
UINT32 vsp_vpe_out2_w = 0; // total overlap_width * 2
UINT32 vsp_ovlp_h = 2220, vsp_vpe_out2_h = 2220;
UINT32 vsp_out_size_w = 4688, vsp_out_size_h = 2220;
UINT32 ptz_out_size_w = 1440, ptz_out_size_h = 1440;
char lut_bin[VSP_MAX_BLEND_FRAME_NUM][50] = {0};


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

#if SMART_STITCH
	#define LUT2D_BUFNUM  3
	char* My_stitchers[12] = {0x0};
	UINT32 g_smart_stitch_LUT_mem_size = 0;
	BOOL g_smartstitch_yalignment_en[VSP_MAX_BLEND_FRAME_NUM] = {1, 1, 1, 1};
	BOOL g_smartstitch_show_ystatus = FALSE;
	const int g_smartstitch_ystatus_cycle = 60;
	int g_smartstitch_ystatus_cycle_cnt = 0;
#endif


#define SENSOR_FPS         15
///////////////////////////////////////////////////////////////////////////////

#define VDO_DDR_ID		DDR_ID0
#define VCAP_DDR_ID		DDR_ID0
#define VPRC_DDR_ID		DDR_ID0
#define VPE_DDR_ID		DDR_ID0
#define DRE_DDR_ID		DDR_ID0
#define VENC_DDR_ID		DDR_ID0
#define GFX_DDR_ID		DDR_ID0
#define VDEC_DDR_ID     DDR_ID0

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


#if USE_PLAYBACK_FLOW
#define BS_BLK_SIZE         0x500000     // bitstream buffer size
#define BS_BLK_DESC_SIZE    0x200     // bitstream buffer size
#endif




static UINT32 lut2d_65[68*65]= {0};



UINT32 g_fps = SENSOR_FPS;
UINT8 g_folder_num = 0;
UINT32 g_vdec_num = 0;
static int    g_quit = 0;
static int    g_vsp_mode = 0;
static UINT32 g_write_file = 1;
static UINT32 g_enc_poll = 1;
static UINT32 g_ipp_out_fmt = IPP_OUT_FMT;
static UINT32 g_en_sub = 1;
static UINT32 g_en_sub2 = 1;
static char   *g_shm = NULL;
static int    g_shmid = 0;
static int    g_frame_synced = 1;
static ULONG  g_lut2d_pa = 0;
static ULONG  g_lut2d_va = 0;
static UINT32 g_lut2d_buf_idx = 0;
static ULONG  g_lut2d_lbuf_pool[VSP_MAX_BLEND_FRAME_NUM][LUT2D_BUFNUM] = {0};
static ULONG  g_lut2d_rbuf_pool[VSP_MAX_BLEND_FRAME_NUM][LUT2D_BUFNUM] = {0};
static ULONG  g_lut2d_l_addr[VSP_MAX_BLEND_FRAME_NUM] = {0};
static ULONG  g_lut2d_r_addr[VSP_MAX_BLEND_FRAME_NUM] = {0};
#if SMART_STITCH_LITE_EN != 0
static VPE_2DLUT_PARAM stitch_finetune_lut[VSP_MAX_BLEND_FRAME_NUM] = {0};
static VPE_2DLUT_PARAM stitch_original_lut[VSP_MAX_BLEND_FRAME_NUM] = {0};
static BOOL stitch_finetune_done[VSP_MAX_BLEND_FRAME_NUM] = {0};
static BOOL stitch_finetune_finish = 0;
#endif

static BOOL  g_dec_mode = 0;
static UINT32 g_dec_type = HD_CODEC_TYPE_H265;






#if SMART_STITCH

// 690 2d_lut_width[0...11], 2d_lut_height[12...23], 2d_lut_lofs[24...35], (opt.), user input 2dlut size, lofs
#define USER_2DLUT_SZ(w,h,lofs)   ( ((ULONG)w&0xFFF)|(((ULONG)h&0xFFF)<<12)|(((ULONG)lofs&0xFFF)<<24) )

static ULONG _conv_lut_sel(ULONG lut2d_sel)
{
    switch(lut2d_sel)
    {
		case VPE_ISP_2DLUT_SZ_257X257:
			return USER_2DLUT_SZ(257,257,260);
		case VPE_ISP_2DLUT_SZ_129X129:
			return USER_2DLUT_SZ(129,129,132);
		case VPE_ISP_2DLUT_SZ_65X65:
			return USER_2DLUT_SZ(65,65,68);
		case VPE_ISP_2DLUT_SZ_9X9:
			return USER_2DLUT_SZ(9,9,12);
		default:
            printf("not support 2dlut size %ld\r\n",lut2d_sel);
    }
    return -1;
}

ULONG smart_stitch_lut2d_va2pa(ULONG lut2d_va)
{
	ULONG lut2d_pa;

	if (lut2d_va == 0) {
		return 0;
	}
	lut2d_pa = lut2d_va - g_lut2d_va + g_lut2d_pa;
	//printf("lut2d_pa = 0x%lx\r\n", lut2d_pa);
	return lut2d_pa;
}

#endif


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
	mem_cfg.pool_info[i].blk_cnt = VSP_DEV_CNT * vcap_num * 4;
	mem_cfg.pool_info[i].ddr_id = VPRC_DDR_ID;

	//vpe out temp buffer1
	i++;
	mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(ALIGN_CEIL_16((vsp_vpe_2d_scl_out_w[0]+vsp_vpe_2d_scl_out_w[1]+vsp_vpe_2d_scl_out_w[2]+vsp_vpe_2d_scl_out_w[3])-vsp_vpe_out2_w/2), vsp_vpe_2d_scl_out_h, HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[i].blk_cnt = VSP_DEV_CNT * 4;
	#if SMART_STITCH
	mem_cfg.pool_info[i].blk_cnt *= 2;
	#endif
	mem_cfg.pool_info[i].ddr_id = VDO_DDR_ID;

	//vpe out temp buffer2
	if (vsp_vpe_out2_w) {
		i++;
		mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
		mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(ALIGN_CEIL_16(vsp_vpe_out2_w), vsp_vpe_out2_h, HD_VIDEO_PXLFMT_YUV420);
		mem_cfg.pool_info[i].blk_cnt = VSP_DEV_CNT * 4;
		#if SMART_STITCH
		mem_cfg.pool_info[i].blk_cnt *= 2;
		#endif
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
	#if USE_PLAYBACK_FLOW
	UINT32 mbinfo_bufsize = 0;
	i++;
	if (g_dec_type == HD_CODEC_TYPE_H265) {
		mbinfo_bufsize = VDO_H265_MBINFO_BUFSIZE(ALIGN_CEIL_64(VDO_SIZE_W), ALIGN_CEIL_64(VDO_SIZE_H));
	} else if (g_dec_type == HD_CODEC_TYPE_H264) {
	mbinfo_bufsize = VDO_H264_MBINFO_BUFSIZE(ALIGN_CEIL_64(VDO_SIZE_W), ALIGN_CEIL_64(VDO_SIZE_H));
	} else {
		mbinfo_bufsize = 0;
	}

	// config common pool (main)
	mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(ALIGN_CEIL_64(VDO_SIZE_W), ALIGN_CEIL_64(VDO_SIZE_H), HD_VIDEO_PXLFMT_YUV420)
													+ mbinfo_bufsize
													+ ALIGN_CEIL(VDO_SIDEINFO_SIZE(ALIGN_CEIL_64(VDO_SIZE_W), ALIGN_CEIL_64(VDO_SIZE_H))*3/2, 4096);
	mem_cfg.pool_info[i].blk_cnt = g_vdec_num * 4;
	mem_cfg.pool_info[i].ddr_id = VDEC_DDR_ID;
	i++;
	// config common pool (scale & display)
	mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[i].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
	// align to 16 for rotation panel
	mem_cfg.pool_info[i].blk_cnt = g_vdec_num * 4;
	mem_cfg.pool_info[i].ddr_id = VDEC_DDR_ID;
	// config common pool for bs pushing in
	i++;
	mem_cfg.pool_info[i].type = HD_COMMON_MEM_USER_POOL_BEGIN;
	mem_cfg.pool_info[i].blk_size = BS_BLK_SIZE;
	mem_cfg.pool_info[i].blk_cnt = g_vdec_num;
	mem_cfg.pool_info[i].ddr_id = VDEC_DDR_ID;
	i++;
	// config common pool for bs description pushing in
	mem_cfg.pool_info[i].type = HD_COMMON_MEM_USER_POOL_BEGIN;
	mem_cfg.pool_info[i].blk_size = BS_BLK_DESC_SIZE;
	mem_cfg.pool_info[i].blk_cnt = g_vdec_num;
	mem_cfg.pool_info[i].ddr_id = VDO_DDR_ID;
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

			#if SMART_STITCH_LITE_EN != 0
				stitch_finetune_lut[frame_idx].lut_sz = vpe_2dlut_size;
				stitch_original_lut[frame_idx].lut_sz = vpe_2dlut_size;
				//--- Initialize stitch_finetune_lut with data of camera calibration 2dlut
				memcpy((void *)(stitch_finetune_lut[frame_idx].lut), (void *)(lut2d[frame_idx].lut2d.lut), lut_size);
				memcpy((void *)(stitch_original_lut[frame_idx].lut), (void *)(lut2d[frame_idx].lut2d.lut), lut_size);
				stitch_finetune_done[frame_idx] = 0;
				stitch_finetune_finish = 0;
			#endif

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

HD_RESULT set_proc_cfg(HD_PATH_ID *p_video_proc_ctrl, HD_DIM* p_max_dim, HD_OUT_ID _out_id, UINT32 isp_id, UINT32 ref_3dnr_id, UINT32 in_plxfmt)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
	HD_VIDEOPROC_CTRL video_ctrl_param = {0};
	HD_PATH_ID video_proc_ctrl = 0;

	ret = hd_videoproc_open(0, _out_id, &video_proc_ctrl); //open this for device control
	if (ret != HD_OK)
		return ret;

	if (p_max_dim != NULL ) {
		if (!g_dec_mode) video_cfg_param.pipe = HD_VIDEOPROC_PIPE_RAWALL;
		else video_cfg_param.pipe = HD_VIDEOPROC_PIPE_SCALE;	//Vdec output format is YUV420
		video_cfg_param.isp_id = isp_id;
		if (!g_dec_mode) video_cfg_param.ctrl_max.func = VIDEOPROC_ALG_FUNC;
		else video_cfg_param.ctrl_max.func = 0;
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
	if (!g_dec_mode) video_ctrl_param.func = VIDEOPROC_ALG_FUNC;
	else video_cfg_param.ctrl_max.func = 0;
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
		#if SMART_STITCH
		vsp_cfg.smart_ver = 2;
		#endif
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
	if (!g_dec_mode) {
		if ((ret = hd_videocap_init()) != HD_OK)
			return ret;
	} else {
		if ((ret = hd_videodec_init()) != HD_OK)
			return ret;
	}
	if ((ret = hd_videoproc_init()) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_init()) != HD_OK)
		return ret;
	if ((ret = hd_gfx_init()) != HD_OK)
		return ret;
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
	#if SMART_STITCH
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_IN(stream_id, 0), HD_VIDEOPROC_OUT(stream_id, 1), &p_stream->proc_path2)) != HD_OK)
		return ret;
	#endif
	if ((ret = hd_videoenc_open(HD_VIDEOENC_IN(0, stream_id), HD_VIDEOENC_OUT(0, stream_id), &p_stream->enc_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT open_module_vsp_stream2(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim, UINT32 stream_id)
{
	HD_RESULT ret;

	// venc isp_id
	p_stream->isp_id = sen_vcap_id[0];
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

static HD_RESULT exit_module(void)
{
	HD_RESULT ret;
	if (!g_dec_mode) {
		if ((ret = hd_videocap_uninit()) != HD_OK)
			return ret;
	} else {
		if ((ret = hd_videodec_uninit()) != HD_OK)
			return ret;
	}
	if ((ret = hd_videoproc_uninit()) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_uninit()) != HD_OK)
		return ret;
	if ((ret = hd_gfx_uninit()) != HD_OK)
		return ret;

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
	if(g_dec_mode) time_threshold = 100000;
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
				/* set 2dlut here */
				#if (SMART_STITCH != 0 && SMART_STITCH_LITE_EN == 0)
				//--- In the lite mode, only the first 2dlut correction in the pipeline is need. Do not set the data for the 2nd 2dlut correction.
				video_frame[frame_idx].reserved[4] = _conv_lut_sel(VPE_ISP_2DLUT_SZ_65X65);
				video_frame[frame_idx].reserved[5] = smart_stitch_lut2d_va2pa(g_lut2d_l_addr[i]);
				video_frame[frame_idx].reserved[6] = smart_stitch_lut2d_va2pa(g_lut2d_r_addr[i]);
				#endif
				video_frame[frame_idx].ddr_id |= (push_idx << HD_VIDEO_MULTI_FRAME_SHIFT);
				ret	= hd_videoproc_push_in_buf(p_vsp_stream->proc_path, &video_frame[frame_idx], NULL, 0);
			}
			#if MEASURE_LATENCY
			{
				static UINT32 frame_count = 0;
				UINT64 time_c;

				time_c = hd_gettime_us();
				frame_count ++;
				if (frame_count % 30 == 0) {
				printf("four frame time diff %d %d %d %d\r\n",
					    time_c-video_frame[0].timestamp,
					    time_c-video_frame[1].timestamp,
					    time_c-video_frame[2].timestamp,
					    time_c-video_frame[3].timestamp
					    );
				}
			}
			#endif

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

#if SMART_STITCH
static int get_2dlut_size(VPE_ISP_2DLUT_SZ type)
{
	int lut_size=0;
	switch (type)
	{
		case VPE_ISP_2DLUT_SZ_9X9:		lut_size = 12*9*4; 		break;
		case VPE_ISP_2DLUT_SZ_2X2:		lut_size = 4*2*4; 		break;
		case VPE_ISP_2DLUT_SZ_65X65:	lut_size = 68*65*4;		break;
		case VPE_ISP_2DLUT_SZ_129X129:	lut_size = 132*129*4;	break;
		default:
		case VPE_ISP_2DLUT_SZ_257X257:	lut_size = 260*257*4;	break;
	}
	return lut_size;
}
static int get_2dlut_width(VPE_ISP_2DLUT_SZ type)
{
	int lut_size=0;
	switch (type)
	{
		case VPE_ISP_2DLUT_SZ_9X9:		lut_size = 12; 	break;
		case VPE_ISP_2DLUT_SZ_2X2:		lut_size = 4; 	break;
		case VPE_ISP_2DLUT_SZ_65X65:	lut_size = 68;	break;
		case VPE_ISP_2DLUT_SZ_129X129:	lut_size = 132;	break;
		default:
		case VPE_ISP_2DLUT_SZ_257X257:	lut_size = 260;	break;
	}
	return lut_size;
}
static int get_2dlut_height(VPE_ISP_2DLUT_SZ type)
{
	int lut_size=0;
	switch (type)
	{
		case VPE_ISP_2DLUT_SZ_9X9:		lut_size = 9; 	break;
		case VPE_ISP_2DLUT_SZ_2X2:		lut_size = 2; 	break;
		case VPE_ISP_2DLUT_SZ_65X65:	lut_size = 65;	break;
		case VPE_ISP_2DLUT_SZ_129X129:	lut_size = 129;	break;
		default:
		case VPE_ISP_2DLUT_SZ_257X257:	lut_size = 257;	break;
	}
	return lut_size;
}
static HD_RESULT smart_stitch_trig(HD_VIDEO_FRAME* p_video_frame, VIDEO_RECORD *p_stream)
{
	SMART_STITCH_INFO smart_stitch_info = {0};
	AET_STATUS_INFO status = {0};
	ISPT_LA_DATA la_data = {0};
	HD_RESULT result;
	UINT32 overlap_idx;

	#define PHY2VIRT_YUV(pa) (vir_addr_frame + ((pa) - phy_addr_frame))

#if 0
	printf("VPE[%d]-OUT[%d] blk=0x%X\r\n", 0, 1, p_video_frame->blk);
	printf("resv 0 ~ 7 = 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, cnt %llx, ts %llx\r\n",
		   p_video_frame->poc_info,
		   p_video_frame->reserved[0],
		   p_video_frame->reserved[1],
		   p_video_frame->reserved[2],
		   p_video_frame->reserved[3],
		   p_video_frame->reserved[4],
		   p_video_frame->reserved[5],
		   p_video_frame->reserved[6],
		   p_video_frame->count,
		   p_video_frame->timestamp
		);
#endif
		//--- Parsing stitch parameters from vsp flow reserved data
		UINT32 current_frame_num;
		UINT32 blend_width[MAX_VSP_BLEND_FRAME_NUM] = {0};
		UINT32 blend_height[MAX_VSP_BLEND_FRAME_NUM] = {0};
		current_frame_num = ((p_video_frame->reserved[6]>>16)&0xf); 		//bit 16..19
		blend_width[0] = (p_video_frame->reserved[2] & 0xffff); 			//bit 0..15
		blend_width[1] = ((p_video_frame->reserved[2] >>16) & 0xffff); 		//bit 16..31
		blend_width[2] = ((p_video_frame->reserved[2] >>32) & 0xffff); 		//bit 32..47
		blend_width[3] = ((p_video_frame->reserved[2] >>48) & 0xffff); 		//bit 48..63
		blend_height[0] = p_video_frame->ph[0];
		blend_height[1] = p_video_frame->ph[0];
		blend_height[2] = p_video_frame->ph[0];
		blend_height[3] = p_video_frame->ph[0];

		//--- Calculate line offset of y/uv images.
		//--- In the case of 4 cameras and 3 overlapping regions,
		//--- the 3 sets of overlapping images are horizontally aligned side by side in memory as shown below.
		//--- +-----------+-----------+-----------+-----------+-----------+-----------+
		//--- |           |           |           |           |           |           |
		//--- | overlap 0 | overlap 0 | overlap 1 | overlap 1 | overlap 2 | overlap 2 |
		//--- | Left      | Right     | Left      | Right     | Left      | Right     |
		//--- | Y data    | Y data    | Y data    | Y data    | Y data    | Y data    |
		//--- |           |           |           |           |           |           |
		//--- +-----------+-----------+-----------+-----------+-----------+-----------+
		//--- blend_width[0] indicate the width of left/right overlap 0 Y data.
		//--- the line offset of the image buffer is 2*blend_width[0]+2*blend_width[1]+2*blend_width[2]+2*blend_width[3]
		UINT32 y_lofs = 0;
		UINT32 uv_lofs = 0;
		for (UINT32 i=0; i<vsp_overlap_num; i++)
		{
			y_lofs += blend_width[i] << 1;
			uv_lofs += blend_width[i] << 1;
		}

		//--- calaulate virtual memory address of y/uv buffer.
 		UINTPTR phy_addr_frame = hd_common_mem_blk2pa(p_video_frame->blk);
 		if (phy_addr_frame == 0)
		{
        	printf("blk2pa fail, blk = 0x%x\r\n", p_video_frame->blk);
            return 0;
        }
        UINT32 frame_data_size = DBGINFO_BUFSIZE() + VDO_YUV_BUFSIZE(y_lofs, blend_height[0], HD_VIDEO_PXLFMT_YUV420);
		ULONG vir_addr_frame = (ULONG)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_frame, frame_data_size);
        if (vir_addr_frame==0)
		{
        	printf("hd_common_mem_mmap fail. \n");
            return 0;
        }
		ULONG y_va = PHY2VIRT_YUV(p_video_frame->phy_addr[0]);
		ULONG uv_va = PHY2VIRT_YUV(p_video_frame->phy_addr[1]);

		//--- calculate virtual memory address of each overlapping image.
		UINT32 anchorX = 0;
		//--- 360 deg view angle
		if(vsp_overlap_num == vsp_blend_frame_num)
			anchorX += blend_width[vsp_overlap_num-1];
		ULONG  y_addr_l[VSP_MAX_BLEND_FRAME_NUM] = {0};
		ULONG  y_addr_r[VSP_MAX_BLEND_FRAME_NUM] = {0};
		ULONG  uv_addr_l[VSP_MAX_BLEND_FRAME_NUM] = {0};
		ULONG  uv_addr_r[VSP_MAX_BLEND_FRAME_NUM] = {0};
		for (UINT32 i=0; i<vsp_overlap_num; i++)
		{
			y_addr_l[i] = y_va + anchorX;
			y_addr_r[i] = y_va + anchorX + blend_width[i];
			uv_addr_l[i] = uv_va + anchorX;
			uv_addr_r[i] = uv_va + anchorX + blend_width[i];
			anchorX += blend_width[i] << 1;
		}
		if(vsp_overlap_num == vsp_blend_frame_num)
		{
			y_addr_r[vsp_blend_frame_num-1] = y_va;
			uv_addr_r[vsp_blend_frame_num-1] = uv_va;
		}
		//--- Pass overlapping information to each instance of smart stich class.
		// int startPreprocess = nvt_timer_tm0_get();
		for (overlap_idx = 0; overlap_idx < vsp_overlap_num; overlap_idx++)
		{
			smart_stitch_info.frame_num = current_frame_num;
			smart_stitch_info.y_lofs_l = y_lofs;
			smart_stitch_info.y_lofs_r = y_lofs;
			smart_stitch_info.uv_lofs_l = uv_lofs;
			smart_stitch_info.uv_lofs_r = uv_lofs;
			smart_stitch_info.blend_height = blend_height[overlap_idx];
			smart_stitch_info.blend_width = blend_width[overlap_idx];
			smart_stitch_info.y_addr_l = y_addr_l[overlap_idx];
			smart_stitch_info.y_addr_r = y_addr_r[overlap_idx];
			smart_stitch_info.uv_addr_l = uv_addr_l[overlap_idx];
			smart_stitch_info.uv_addr_r = uv_addr_r[overlap_idx];

			//print lib trigger parameters
			#if 0
				printf("isp_lib param: \r\n	overlapid %d \r\n	"
				"overlap_idx %d \r\n	(y_l, y_r, uv_l, uv_r)addr (%lx, %lx, %lx, %lx) \r\n	"
				"(y_l, y_r, uv_l, uv_r)lofs (%d, %d, %d, %d) \r\n	"
				"blend_width %d, blend_height %d, image_width %d, image_height %d \r\n",
					overlap_idx,
					smart_stitch_info.frame_num,
					smart_stitch_info.y_addr_l, smart_stitch_info.y_addr_r,
					smart_stitch_info.uv_addr_l, smart_stitch_info.uv_addr_r,
					smart_stitch_info.y_lofs_l, smart_stitch_info.y_lofs_r,
					smart_stitch_info.uv_lofs_l, smart_stitch_info.uv_lofs_r,
					smart_stitch_info.blend_width, smart_stitch_info.blend_height,
					smart_stitch_info.image_width, smart_stitch_info.image_height);
			#endif

			if (smart_stitch2_process_input((void *)My_stitchers[overlap_idx], &smart_stitch_info) != HD_OK) {
				printf("smart_stitch2_process_input fail !!\r\n\r\n");
				return HD_ERR_SYS;
			}
			//printf("g_lut2d_l_addr[%d] = 0x%lx,  g_lut2d_r_addr[%d] = 0x%lx!!\r\n", overlap_idx, g_lut2d_l_addr[overlap_idx], overlap_idx, g_lut2d_r_addr[overlap_idx]);
		}

		g_smartstitch_ystatus_cycle_cnt--;
		//--- obtain the 2dlut result of each overlapping region.
		g_lut2d_buf_idx = (g_lut2d_buf_idx + 1) % LUT2D_BUFNUM;
		for (overlap_idx = 0; overlap_idx < vsp_overlap_num; overlap_idx++)
		{
			#if SMART_STITCH_LITE_EN == 0
				if (smart_stitch2_process_output((void *)My_stitchers[overlap_idx], g_lut2d_lbuf_pool[overlap_idx][g_lut2d_buf_idx], g_lut2d_rbuf_pool[overlap_idx][g_lut2d_buf_idx]) != HD_OK) {
					printf("smart_stitch2_process_output fail !!\r\n\r\n");
					return HD_ERR_SYS;
				}
				g_lut2d_l_addr[overlap_idx] = g_lut2d_lbuf_pool[overlap_idx][g_lut2d_buf_idx];
				g_lut2d_r_addr[overlap_idx] = g_lut2d_rbuf_pool[overlap_idx][g_lut2d_buf_idx];
				//printf("g_lut2d_l_addr[%d] = 0x%lx,  g_lut2d_r_addr[%d] = 0x%lx!!\r\n", overlap_idx, g_lut2d_l_addr[overlap_idx], overlap_idx, g_lut2d_r_addr[overlap_idx]);

			#else
				if (smart_stitch2_process_output_lite((void *)My_stitchers[overlap_idx],
												(ULONG)(stitch_finetune_lut[overlap_idx].lut),
												(ULONG)(stitch_finetune_lut[(overlap_idx+1)%vsp_blend_frame_num].lut)) == HD_OK)
				{
					stitch_finetune_done[overlap_idx] = 1;
				}

				//---Use identity lut for 2nd 2dlut correction pipeline
			#endif
			if(g_smartstitch_show_ystatus && g_smartstitch_yalignment_en[overlap_idx])
			{
				if(g_smartstitch_ystatus_cycle_cnt <= 0 && overlap_idx == 0)
				{
					printf("==========================%d===========================\n", nvt_timer_tm0_get());
				}
				const UINT8* ystatus = smart_stitch2_y_alignment_status((void *)My_stitchers[overlap_idx]);
				if(ystatus[0] == Y_Alignment_Disabled)
				{
					g_smartstitch_yalignment_en[overlap_idx] = FALSE;
					printf("overlap %u : Y alignment disabled.");
				}
				else
				{
					if(g_smartstitch_ystatus_cycle_cnt <= 0)
					{
						printf("overlap %u Y alignment status:", overlap_idx);
						for (size_t i = 0; i < 6; i++)
						{
							if(ystatus[i] == Y_Alignment_Not_Stable)
								printf("NotStable ");
							else if(ystatus[i] == Y_Alignment_Half_Anchor_Stable)
								printf("HalfStable ");
							else if(ystatus[i] == Y_Alignment_ALL_Anchors_Stable)
								printf("AllStable ");
						}
						printf("\r\n");
					}
				}
			}
		}
		if(g_smartstitch_ystatus_cycle_cnt <= 0)
		{
			g_smartstitch_ystatus_cycle_cnt = g_smartstitch_ystatus_cycle;
		}
		hd_common_mem_flush_cache((void*)g_lut2d_va, g_smart_stitch_LUT_mem_size);
		// int endPreprocess = nvt_timer_tm0_get();
		// printf("Process Time: %d (us)\nTurn it off after finishing debug.\r\n", endPreprocess - startPreprocess);

		#if 0
		// update to DRE fusion map
		for (overlap_idx = 0; overlap_idx < vsp_overlap_num; overlap_idx++)
		{
			UINT32 blk_size = VDO_YUV_BUFSIZE(smart_stitch_info.blend_width[overlap_idx], smart_stitch_info.blend_height[overlap_idx], HD_VIDEO_PXLFMT_Y8);
			// flush cache
			hd_common_mem_flush_cache((void*)(p_stream->fusion_va[overlap_idx]), blk_size);
		}

		// set vpe
		for (overlap_idx = 0; overlap_idx < vsp_blend_frame_num; overlap_idx++) {
			if (vendor_vpe_set_cmd(VPET_ITEM_2DLUT_PARAM, &lut2d[overlap_idx]) != HD_OK) {
				printf("update idx %d 2dlut fail !!\r\n\r\n", overlap_idx);
				return HD_ERR_SYS;
			}
		}
		#endif
		HD_RESULT ret = hd_common_mem_munmap((void *)vir_addr_frame, frame_data_size);
		if (ret != HD_OK) {
			printf("mnumap error !!\r\n\r\n");
		}

		#if SMART_STITCH_LITE_EN != 0
		FILE     *fp;
		if(stitch_finetune_finish == 0)
		{
			bool all_done = true;
			for (overlap_idx = 0; overlap_idx < vsp_overlap_num; overlap_idx++)
			{
				all_done = all_done & (stitch_finetune_done[overlap_idx]!=0);
			}
			if(all_done)
			{
				stitch_finetune_finish = 1;

				char lut_path[512]={0};
				UINT32 frame_idx;
				for (frame_idx = 0; frame_idx < vsp_blend_frame_num; frame_idx++)
				{
					//---save 2dlut bin
					sprintf(lut_path, "%sfinetune%d.bin", VSP_DATA_PATH, frame_idx);
					if ((fp = fopen(lut_path, "wb")) == NULL) {
						printf("open file (%s) fail !!!", lut_path);
						return -1;
					}
					setvbuf(fp, NULL, _IONBF, 0);

					int lut_size = get_2dlut_size(stitch_finetune_lut[frame_idx].lut_sz);
					int write_len = fwrite((void *)(stitch_finetune_lut[frame_idx].lut), 1, lut_size, fp);
					if (write_len != lut_size) {
						printf("writing 2dlut error\n");
					}
					// close input image file
					fclose(fp);

					//---repalce static lut
					memcpy((void *)(lut2d[frame_idx].lut2d.lut), (void *)(stitch_finetune_lut[frame_idx].lut), lut_size);
				}
				printf("Replace Static Stitching table.\n");
			}
		}
		#endif

		return HD_OK;
}
#endif




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
	prctl(PR_SET_NAME, "vsp_pull_thd");
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
	printf("\r\n vsp pull_thread....\r\n");
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

		#if SMART_STITCH
		ret = hd_videoproc_pull_out_buf(p_stream[VSP_STREAM1_ID]->proc_path2, &out_video_frame[0], 0);
		if (ret != HD_OK) {
			if (ret != HD_ERR_UNDERRUN) {
				printf("VSP[%d]-OUT[%d] pull_out error = %d!!\r\n", 0, 1, ret);
			}
		} else {
			//trigger lib
			if (smart_stitch_trig(&out_video_frame[0], p_stream[VSP_STREAM1_ID]) != HD_OK) {
				printf("release vsp out[%d] error !!\r\n\r\n", 1);
			}
			ret = hd_videoproc_release_out_buf(p_stream[VSP_STREAM1_ID]->proc_path2, &out_video_frame[0]);
			if (ret != HD_OK) {
				printf("release vsp out[%d] error !!\r\n\r\n", 1);
			}
		}
		#endif

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
	char file_path[VSP_MAX_BLEND_FRAME_NUM + VSP_OUT_STREAM_CNT][64] = {0};
	char file_len_path[VSP_MAX_BLEND_FRAME_NUM][64] = {0};
	FILE *f_out[VSP_MAX_BLEND_FRAME_NUM + VSP_OUT_STREAM_CNT];
	FILE *f_len_out[VSP_MAX_BLEND_FRAME_NUM];
	#define STREAMI_PHY2VIRT(i, pa) (vir_addr[i] + (pa - phy_buf[i].buf_info.phy_addr))
	char file_folder[64];

	mkdir("/mnt/sd/vsp/dump_bs", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	snprintf(file_folder, 64, "/mnt/sd/vsp/dump_bs/vsp_scenes_%d", g_folder_num);
	mkdir(file_folder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	UINT32 nul_sum = 0;

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
			if (g_dec_mode) {
				snprintf(file_path[i], 64, "%s/vsp_decode%d.dat", file_folder, i);
				snprintf(file_len_path[i], 64, "%s/vsp_decode%d.len", file_folder, i);
			} else {
				snprintf(file_path[i], 64, "%s/vsp_sensor%d.dat", file_folder, i);
				snprintf(file_len_path[i], 64, "%s/vsp_sensor%d.len", file_folder, i);
			}
		} else {
			snprintf(file_path[i], 64, "%s/vsp_out_stream%d.dat", file_folder, i - vsp_blend_frame_num);
		}
		if ((f_out[i] = fopen(file_path[i], "wb")) == NULL) {
			HD_VIDEOENC_ERR("open file (%s) fail....\r\n", file_path[i]);
		} else {
			printf("\r\ndump bitstream %d to file (%s) ....\r\n", i, file_path[i]);
		}
		if (i < vcap_num) {
			if ((f_len_out[i] = fopen(file_len_path[i], "wb")) == NULL) {
				HD_VIDEOENC_ERR("open file (%s) fail....\r\n", file_len_path[i]);
			} else {
				printf("\r\ndump bitstream len %d to file (%s) ....\r\n", i, file_len_path[i]);
			}
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
							// nul_sum is sum of bs header length(VPS SPS PPS), last pack is I frame data.
							if (j < data_pull.pack_num - 1) {
								nul_sum += len;
							}
							if (g_write_file) {
								if (f_out[i]) fwrite(ptr, 1, len, f_out[i]);
								if (f_out[i]) fflush(f_out[i]);
								// write bs len
								if (i < vcap_num) {
									if (j == data_pull.pack_num - 1 && data_pull.pack_num > 1) {
										if (f_len_out[i]) fprintf(f_len_out[i], "%d\n", nul_sum);
										if (f_len_out[i]) fflush(f_len_out[i]);
									}
									if (j == data_pull.pack_num - 1) {
										if (f_len_out[i]) fprintf(f_len_out[i], "%d\n", len);
										if (f_len_out[i]) fflush(f_len_out[i]);
									}
								}
							}
							#endif
						}
						nul_sum = 0;
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

		if (i < vcap_num) {
			if (f_len_out[i]) fclose(f_len_out[i]);
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
		g_vdec_num = vcap_num;
		printf("vcap_num(vdec_num) = %d, vsp_blend_frame_num = %d, vsp_sensor_mask = 0x%x\r\n",
			vcap_num, vsp_blend_frame_num, vsp_sensor_mask);

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


#if SMART_STITCH
static HD_RESULT smart_stitch_initialize(void)
{
	HD_RESULT ret;
	UINT32               	lut2d_size = ALIGN_CEIL_64(68*65*4) , alloc_size;
	ULONG                	lut2d_cur_va;
	SMART_STITCH_INIT_PARAM init_param;
	UINT32               	frm_idx, i;

	// set 2dlut
	g_smart_stitch_LUT_mem_size = ALIGN_CEIL_64(68*65*4) * LUT2D_BUFNUM * 2 * vsp_overlap_num;
	alloc_size = g_smart_stitch_LUT_mem_size;
	ret = hd_common_mem_alloc("2dlut1", (UINTPTR *)&g_lut2d_pa, (void **)&g_lut2d_va, alloc_size, 0);
	if (ret != HD_OK) {
		printf("err:alloc size 0x%x\r\n", (int)alloc_size);
		return ret;
	}
	lut2d_cur_va = g_lut2d_va;
	g_lut2d_buf_idx = 0;
	g_lut2d_buf_idx = 0;
	for (UINT32 frm_idx=0; frm_idx < vsp_overlap_num; frm_idx++)
	{
		for (i = 0;i < LUT2D_BUFNUM; i++) {
			g_lut2d_lbuf_pool[frm_idx][i] = lut2d_cur_va;
			lut2d_cur_va += lut2d_size;
			g_lut2d_rbuf_pool[frm_idx][i] = lut2d_cur_va;
			lut2d_cur_va += lut2d_size;
			printf("lut2d_l[%d] = 0x%lx, lut2d_r[%d] = 0x%lx\r\n", i, g_lut2d_lbuf_pool[frm_idx][i], i, g_lut2d_rbuf_pool[frm_idx][i]);
		}

		init_param.blend_width  = vsp_ovlp_w[frm_idx];
		init_param.blend_height = vsp_vpe_2d_scl_out_h;
		init_param.output_lut_width  = 65;
		init_param.output_lut_height = 65;
		init_param.overlapID = frm_idx;
		init_param.vsp_data_path = VSP_DATA_PATH;

		#if SMART_STITCH_LITE_EN == 0
			My_stitchers[frm_idx] = create_smart_stitch2(&init_param);
		#else
			uint16_t x_search_range = 100;
			#if SMART_STITCH_LITE_Y_ALIGNMENT_EN != 0
				uint16_t y_search_range = 60;
			#else
				uint16_t y_search_range = 0;
			#endif
			uint16_t y_ds_num = 4;
			float overlap_transition_proportion = 1.0;
			My_stitchers[frm_idx] = create_smart_stitch2_lite(&init_param,
			x_search_range, y_search_range, y_ds_num,
			overlap_transition_proportion, vsp_lut_tbl_sz, vsp_vpe_2d_scl_out_w[frm_idx], vsp_vpe_2d_scl_out_w[(frm_idx+1)%vsp_blend_frame_num]);
		#endif

		if (My_stitchers[frm_idx] == NULL)
		{
			printf("create stitcher obj[%d] fail.\n", frm_idx);
			return HD_ERR_NG;
		}
		printf("create stitcher obj[%d] OK.\n", frm_idx);
	}
	printf("g_lut2d_pa = 0x%lx, g_lut2d_va = 0x%lx, alloc_size = 0x%x, lut2d_cur_va = 0x%lx\r\n",
		g_lut2d_pa, g_lut2d_va, alloc_size, lut2d_cur_va);
	return HD_OK;
}

#if SMART_STITCH_LITE_EN != 0
static void smart_stitch2_lite_recalculate(void)
{
	//--- Restore applied 2dlut to the original camera calibration 2dlut.
	for (UINT32 frame_idx=0; frame_idx < vsp_blend_frame_num; frame_idx++)
	{
		int lut_size = get_2dlut_size(stitch_original_lut[frame_idx].lut_sz);
		//--- Restore finetune 2dlut to initial values.
		memcpy((void *)(stitch_finetune_lut[frame_idx].lut), (void *)(stitch_original_lut[frame_idx].lut), lut_size);
		//--- Restore the 2dlut applied to the hardware to initial values.
		memcpy((void *)(lut2d[frame_idx].lut2d.lut), (void *)(stitch_original_lut[frame_idx].lut), lut_size);
	}

	stitch_finetune_finish = 0;
	for (UINT32 overlap_idx=0; overlap_idx < vsp_overlap_num; overlap_idx++)
	{
		if (My_stitchers[overlap_idx] != NULL)
		{
			smart_stitch2_lite_redo(My_stitchers[overlap_idx]);
			stitch_finetune_done[overlap_idx] = 0;
		}
	}
}

static void smart_stitch2_lite_recalculate_single(int overlap_idx)
{
	int left_num = overlap_idx;
	int right_num = (left_num + 1) % vsp_blend_frame_num;

	//--- Restore applied 2dlut to the original camera calibration 2dlut.
	//--- right half part 2dlut of left image.
	int lut_h = get_2dlut_height(stitch_original_lut[left_num].lut_sz);
	int lut_w = get_2dlut_width(stitch_original_lut[left_num].lut_sz);
	for (int y = 0; y < lut_h; y++)
	{
		for (int x = lut_w/2; x < lut_w; x++)
		{
			stitch_finetune_lut[left_num].lut[y*lut_w+x] = stitch_original_lut[left_num].lut[y*lut_w+x];
			lut2d[left_num].lut2d.lut[y*lut_w+x] = stitch_original_lut[left_num].lut[y*lut_w+x];
		}
	}
	//--- left half part 2dlut of right image.
	lut_h = get_2dlut_height(stitch_original_lut[right_num].lut_sz);
	lut_w = get_2dlut_width(stitch_original_lut[right_num].lut_sz);
	for (int y = 0; y < lut_h; y++)
	{
		for (int x = 0; x < lut_w/2; x++)
		{
			stitch_finetune_lut[right_num].lut[y*lut_w+x] = stitch_original_lut[right_num].lut[y*lut_w+x];
			lut2d[right_num].lut2d.lut[y*lut_w+x] = stitch_original_lut[right_num].lut[y*lut_w+x];
		}
	}

	stitch_finetune_finish = 0;
	if (My_stitchers[overlap_idx] != NULL)
	{
		smart_stitch2_lite_redo(My_stitchers[overlap_idx]);
		stitch_finetune_done[overlap_idx] = 0;
	}
}

static void smart_stitch2_lite_restore_staticlut(void)
{
	stitch_finetune_finish = 1;
	for (UINT32 frame_idx=0; frame_idx < vsp_blend_frame_num; frame_idx++)
	{
		int lut_size = get_2dlut_size(stitch_original_lut[frame_idx].lut_sz);
		memcpy((void *)(stitch_finetune_lut[frame_idx].lut), (void *)(stitch_original_lut[frame_idx].lut), lut_size);
		memcpy((void *)(lut2d[frame_idx].lut2d.lut), (void *)(stitch_original_lut[frame_idx].lut), lut_size);
	}
}
#endif

static void smart_stitch_exit(void)
{
	UINT32 frm_idx;

	for (frm_idx=0; frm_idx < vsp_overlap_num; frm_idx++)
	{
		if (My_stitchers[frm_idx] != NULL)
		{
			destroy_smart_stitch2((void **)(&(My_stitchers[frm_idx])));
			printf("destroyed stitcher obj[%d].\n", frm_idx);
		}
	}
	hd_common_mem_free(g_lut2d_pa, (void *)g_lut2d_va);
}
#endif




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

	// query program options
	if (argc == 1) {
		printf("Usage: <fps> <ipp out fmt> <enable sub> <enable sub2> <enc_type> <enable decode> <folder num> <bit rate(Mbps)>.\r\n");
		printf("Help:\r\n");
		printf("  <fps>  : 20,25,30\r\n");
		printf("  <ipp out fmt>  : 0(YUV420), others(YCC)\r\n");
		printf("  <enable sub stream>  : 0(disable), 1(enable)\r\n");
		printf("  <enable sub stream2>  : 0(disable), 1(enable)\r\n");
		printf("  <enc_type>  : 0(H265), 1(H264), 2(MJPG)\r\n");
		printf("  <dec_mode>  : 0(liveview), 1(playback from folder_num)\r\n");
		printf("  <folder_num>  : 0(/mnt/sd/vsp/dump_bs/vsp_scenes_0), 1(/mnt/sd/vsp/dump_bs/vsp_scenes_1), 2(/mnt/sd/vsp/dump_bs/vsp_scenes_2)\r\n");
		printf("  <bitrate(Mbps)>  : bitrate for each stream. Unit: Mbps\r\n");
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

	if (argc >= 7) {
		g_dec_mode = atoi(argv[6]);
		if (USE_PLAYBACK_FLOW == 0) {
			printf("Use playback mode need to set macro \"USE_PLAYBACK_FLOW\" to 1");
			goto exit;
		}
		g_dec_type = enc_type;
	}
	if (argc >= 8) {
		g_folder_num = atoi(argv[7]);
	}
	if (argc >= 9) {
		vdo_br = atof(argv[8]) * 1024 * 1024;
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

	#if SMART_STITCH
	if(vsp_ovlp_w[vsp_blend_frame_num-1] == 0)
		vsp_overlap_num = vsp_blend_frame_num - 1;
	else
		vsp_overlap_num = vsp_blend_frame_num;
	smart_stitch2_set_dbg_out(0);
	#endif

	// check TEST pattern exist
	if (g_dec_mode) {
		for (i = 0;i < g_vdec_num; i++) {
			if(g_dec_type == 0) stream[i].dec_type= HD_CODEC_TYPE_H265;
			else if(g_dec_type == 1) stream[i].dec_type= HD_CODEC_TYPE_H264;
			else if(g_dec_type == 2) stream[i].dec_type= HD_CODEC_TYPE_JPEG;
			else {
				printf("not support decode type [%d]\r\n", g_dec_type);
				goto exit;
			}
		}
		if (check_test_pattern(stream, g_vdec_num) == FALSE) {
			printf("test_pattern isn't exist\r\n");
			goto exit;
		}
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

	if (!g_dec_mode) {
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
	} else {
		// open all vdec
		ret = open_module_all_dec(stream, VDO_SIZE_W, VDO_SIZE_H, g_vdec_num);
		if (ret != HD_OK) {
			printf("open_module_all_dec fail=%d\n", ret);
			goto exit;
		}
	}

	stream[VSP_STREAM1_ID].proc_max_dim.w = (vsp_out_size_w > VDO_SIZE_W) ? vsp_out_size_w: VDO_SIZE_W;//assign by user
	stream[VSP_STREAM1_ID].proc_max_dim.h = (vsp_out_size_h > VDO_SIZE_H) ? vsp_out_size_h: VDO_SIZE_H; //assign by user
	ret = open_module_vsp_stream1(&stream[VSP_STREAM1_ID], &stream[VSP_STREAM1_ID].proc_max_dim, VSP_STREAM1_ID);
	if (ret != HD_OK) {
		printf("open%d fail=%d\n", VSP_STREAM1_ID, ret);
		goto exit;
	}

	ret = open_module_vsp_stream2(&stream[VSP_STREAM2_ID], NULL, VSP_STREAM2_ID);
	if (ret != HD_OK) {
		printf("open%d fail=%d\n", VSP_STREAM2_ID, ret);
		goto exit;
	}

	ret = open_module_vsp_stream3(&stream[VSP_STREAM3_ID], NULL, VSP_STREAM3_ID);
	if (ret != HD_OK) {
		printf("open%d fail=%d\n", VSP_STREAM3_ID, ret);
		goto exit;
	}

	if (!g_dec_mode) {
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
	} else {
		// set all videodecode parameter
		ret = set_all_dec_cfg(stream, VDO_SIZE_W, VDO_SIZE_H, g_vdec_num);
		if (ret != HD_OK) {
			printf("set_all_dec_cfg fail=%d\n", ret);
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
		printf("set vsp_proc 1 fail=%d\n", ret);
		goto exit;
	}

#if SMART_STITCH
	vsp_out_dim.w = vsp_out_size_w;
	vsp_out_dim.h = vsp_out_size_h;
	ret = set_vsp_proc_param(stream[VSP_STREAM1_ID].proc_path2, &vsp_out_dim);
	if (ret != HD_OK) {
		printf("set vsp_proc 2 fail=%d\n", ret);
		goto exit;
	}
	smart_stitch_initialize();
#endif

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

	for (i = 0;i < vcap_num; i++) {
		// bind modules
		if (!g_dec_mode)
			hd_videocap_bind(HD_VIDEOCAP_OUT(sen_vcap_id[i], 0), HD_VIDEOPROC_IN(i, 0));
		else
			hd_videodec_bind(HD_VIDEODEC_OUT(0, i), HD_VIDEOPROC_IN(i, 0));
		// start modules
		if (!g_dec_mode)
			hd_videocap_start(stream[i].cap_path);
		else
			hd_videodec_start(stream[i].dec_path);
		hd_videoproc_start(stream[i].proc_path);
		hd_videoenc_start(stream[i].enc_path);
	}

	// start vsp
	hd_videoproc_start(stream[VSP_STREAM1_ID].proc_path);
	#if SMART_STITCH
	hd_videoproc_start(stream[VSP_STREAM1_ID].proc_path2);
	#endif
	hd_videoenc_start(stream[VSP_STREAM1_ID].enc_path);

	// start stream2
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
	if (g_dec_mode) {
		// create decode_thread (push_in bitstream)
		ret = create_all_dec_thread(stream, g_vdec_num);
		if (ret != HD_OK) {
			printf("create_all_dec_thread fail=%d\n", ret);
			goto exit;
		}
	}

	// query user key
	printf("Enter q to exit\n");
	printf("Enter d to debug\n");
	printf("Enter 0 to disable vsp mode & start encode each sensor screen\n");
	printf("Enter 1 to enable vsp mode & stop encode each sensor screen\n");
	printf("Enter 3 to restart vsp\n");
	printf("Enter 8 to stop write file\n");
	printf("Enter 9 to start write file\n");
	printf("Enter a to stop encode poll\n");
	printf("Enter b to start encode poll\n");
	printf("Enter s to record smart stitch source image\n");
	printf("Enter t to stop recording images.\n");
	printf("Enter m to save y alignment map.\n");
	printf("Enter k to toggle y alignment status printing\n");
	printf("Enter z to start rtsp\n");
	#if SMART_STITCH_LITE_EN != 0
		printf("Enter r to recalculate finetune 2dlut\n");
		printf("Enter o to restore original 2dlut\n");
	#endif
	while (1) {
		key = GETCHAR();
		if (key == 'q' || key == 0x3) {
			// let vsp_thread, pull_thread stop loop and exit
			g_quit = 1; // stop thread
			// quit program
			stream[0].enc_exit = 1;
			if (g_dec_mode) {
				stream[0].dec_exit = 1;
				stream[1].dec_exit = 1;
				stream[2].dec_exit = 1;
				stream[3].dec_exit = 1;
			}
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
			hd_videoproc_stop(stream[VSP_STREAM1_ID].proc_path);
			#if SMART_STITCH
			hd_videoproc_stop(stream[VSP_STREAM1_ID].proc_path2);
			#endif
			hd_videoproc_close(stream[VSP_STREAM1_ID].proc_path);
			#if SMART_STITCH
			hd_videoproc_close(stream[VSP_STREAM1_ID].proc_path2);
			#endif
			hd_videoproc_open(HD_VIDEOPROC_IN(VSP_STREAM1_ID, 0), HD_VIDEOPROC_OUT(VSP_STREAM1_ID, 0), &stream[VSP_STREAM1_ID].proc_path);
			#if SMART_STITCH
			hd_videoproc_open(HD_VIDEOPROC_IN(VSP_STREAM1_ID, 0), HD_VIDEOPROC_OUT(VSP_STREAM1_ID, 1), &stream[VSP_STREAM1_ID].proc_path2);
			#endif
			hd_videoproc_start(stream[VSP_STREAM1_ID].proc_path);
			#if SMART_STITCH
			hd_videoproc_start(stream[VSP_STREAM1_ID].proc_path2);
			#endif
		}
		if (key == '4') {
			printf("\r\n restart vsp\r\n");
			hd_videoproc_stop(stream[VSP_STREAM1_ID].proc_path);
			#if SMART_STITCH
			hd_videoproc_stop(stream[VSP_STREAM1_ID].proc_path2);
			#endif
			usleep(10000);
			hd_videoproc_start(stream[VSP_STREAM1_ID].proc_path);
			#if SMART_STITCH
			hd_videoproc_start(stream[VSP_STREAM1_ID].proc_path2);
			#endif
		}
		if (key == '6')
		{
			smart_stitch2_set_dbg_out(0);
			printf("debug disable\r\n");
		}
		if (key == '7') {
			printf("\r\n stop vsp\r\n");
			hd_videoproc_stop(stream[VSP_STREAM1_ID].proc_path);
			#if SMART_STITCH
			hd_videoproc_stop(stream[VSP_STREAM1_ID].proc_path2);
			#endif
		}

		if (key == '8') {
			g_write_file = 0;
			printf("stop write file\r\n");
		}
		if (key == '9') {
			g_frame_synced = 0;
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
		if (key == 's') {
				printf("Enter [0 ~ %d] to record source images of the specified overlapping region\r\n", vsp_overlap_num-1);

				INT key2 = GETCHAR();
				while (key2 == '\n' || key2 == EOF)
					key2 = GETCHAR();

				key2 = key2 - '0';
				if(key2 < 0 || key2 > 9)
				{
					printf("Wrong input\r\n");
				}
				else if(key2 < (INT)vsp_overlap_num)
				{
					HD_RESULT retv = smart_stitch2_record_src_img_enable(My_stitchers[key2], (char*)VSP_DATA_PATH"/smart_stich_dbimage");
					if(retv == HD_OK)
						printf("Start recording source images of overlap %d\r\n", key2);
					else
						printf("Can't record source images of overlap %d\r\n", key2);
				}
				else
				{
					printf("Index out of range\r\n");
				}
		}
		if (key == 't') {
			printf("Stop recording source images\n");
			for (size_t i = 0; i < vsp_overlap_num; i++)
			{
				smart_stitch2_record_src_img_disable(My_stitchers[i]);
			}
		}
		if (key == 'm') {
			#if SMART_STITCH_LITE_EN == 0
			printf("Start to save y alignment map.\n");
			for (size_t i = 0; i < vsp_overlap_num; i++)
			{
				smart_stitch2_save_yalignment_map(My_stitchers[i]);
			}
			#else
				printf("Not support.\n");
			#endif
		}
		if (key == 'k') {
			g_smartstitch_show_ystatus = g_smartstitch_show_ystatus ^ 1;
			printf("show y alignement status: %d\r\n", g_smartstitch_show_ystatus);
		}
		#if SMART_STITCH_LITE_EN != 0
			if (key == 'r') {
				printf("Enter [0 ~ %d] to finetune the specified overlapping region\r\n", vsp_overlap_num-1);
				printf("Enter 9 to finetune all the overlapping regions\r\n");

				INT key2 = GETCHAR();
				while (key2 == '\n' || key2 == EOF)
					key2 = GETCHAR();

				key2 = key2 - '0';
				if(key2 < 0 || key2 > 9)
				{
					printf("Wrong input\r\n");
				}
				else if(key2 < (INT)vsp_overlap_num)
				{
					printf("Recalculate finetune 2dlut for overlap %d\r\n", key2);
					smart_stitch2_lite_recalculate_single(key2);
				}
				else if(key2 == 9)
				{
					printf("Recalculate finetune 2dlut for all overlap regions\r\n");
					smart_stitch2_lite_recalculate();
				}
				else
				{
					printf("Index out of range\r\n");
				}
			}
			if (key == 'o') {
				smart_stitch2_lite_restore_staticlut();
				printf("Restore original 2dlut\r\n");
			}

		#endif
	}
	for (i = 0;i < vcap_num; i++) {
		if (!g_dec_mode)
			hd_videocap_stop(stream[i].cap_path);
		else{
			// destroy feed decode threads
			pthread_join(stream[i].feed_thread_id, NULL);
			// stop video_playback modules (decode 1st)
			hd_videodec_stop(stream[i].dec_path);
		}
		hd_videoproc_stop(stream[i].proc_path);
		hd_videoenc_stop(stream[i].enc_path);
	}

	// stop vsp
	hd_videoproc_stop(stream[VSP_STREAM1_ID].proc_path);


	#if SMART_STITCH
	hd_videoproc_stop(stream[VSP_STREAM1_ID].proc_path2);
	#endif
	hd_videoenc_stop(stream[VSP_STREAM1_ID].enc_path);

	// stop vsp stream2
	hd_videoenc_stop(stream[VSP_STREAM2_ID].enc_path);

	// stop vsp stream3
	hd_videoenc_stop(stream[VSP_STREAM3_ID].enc_path);

	// destroy threads
	pthread_join(stream[0].enc_thread_id, NULL);
	pthread_join(stream[VSP_STREAM1_ID].vsp_thread_id, (void* )NULL);
	// unbind  modules (sensor or vdec)
	for (i = 0;i < vcap_num; i++) {
		// unbind  modules
		if (!g_dec_mode) hd_videocap_unbind(HD_VIDEOCAP_OUT(sen_vcap_id[i], 0));
		else hd_videodec_unbind(HD_VIDEODEC_OUT(0, i));
	}

exit:
	if (!g_dec_mode) {
		for (i = 0;i < vcap_num; i++) {
			ret = close_module(&stream[i]);
			if (ret != HD_OK) {
				printf("close stream %d fail=%d\n", i, ret);
			}
		}
	} else ret = close_module_all_dec(stream, g_vdec_num);

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

	#if SMART_STITCH
	smart_stitch_exit();
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

