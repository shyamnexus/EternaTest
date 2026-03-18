#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <hdal.h>
#include <kwrap/perf.h>
#include <kwrap/debug.h>
#include <kwrap/util.h>
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h>
#include "prjcfg.h"
#include "comm/timer.h"
#include "hd_common.h"
#include "sys_fastboot.h"
#include "flow_uvc.h"
#include "uvc_slice.h"
#include "flow_hid.h"
#include "hid_def.h"
#include "flow_ct_pu.h"
#include "flow_eu.h"
#include "flow_uvc_info.h"
#include "bin_info.h"
#include "dpc.h"
#include "vendor_videocapture.h"
#include "vendor_videoenc.h"
#include "vendor_audiocapture.h"
#include "vendor_videoprocess.h"
#include "sys_fdt.h"
#include "sys_mempool.h"
#include <rtosfdt.h>
#include <libfdt.h>
#include <gpio.h>
#include <plat/clock.h>


// platform dependent
#if defined(__LINUX)
#include <pthread.h>			//for pthread API
#define MAIN(argc, argv) 		int main(int argc, char** argv)
#define GETCHAR()				getchar()
#else
#include <pthread.h>			//for pthread API
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <kwrap/util.h>		//for sleep API
#define sleep(x)    			vos_util_delay_ms(1000*(x))
#define msleep(x)    			vos_util_delay_ms(x)
#define usleep(x)   			vos_util_delay_us(x)
#include <kwrap/examsys.h> 	//for MAIN(), GETCHAR() API
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(hd_video_record, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define DEBUG_MENU 		1
#define VCAP_PATTERN_GEN  1 // set 1 for no sensor testing

#define UVC_ON 1
#define AUDIO_ON 0
#define WRITE_BS 0
#define UVC_SUPPORT_YUV_FMT 1
#define UVAC_WAIT_RELEASE 1
#define MSDC_FUNC 0 // 0: DISABLE, 1: MSDC_DISK, 2: MSDC_IQ
#define PERF_TEST 0
#define FUNC_SLICE_MODE   0
#define AUDIO_FILTER 0
#define UVAC_SUSPEND_FUNC   0
#define AUDIO_ANR 0
#define PST_FUNC 0
#define U3UVAC 1

#define POOL_SIZE_USER_DEFINIED  0x1000000

#define COMMON_RECFRM_BUFFER 0
#define COMMON_RECFRM_FUNC_BASE   0
#define COMMON_RECFRM_FUNC_SVC    1
#define COMMON_RECFRM_FUNC_LTR    0

#if UVC_ON
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <compiler.h>
#include <usb2dev.h>
#include "UVAC.h"
#include "comm/timer.h"
#include "kwrap/stdio.h"
#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kwrap/task.h"
#include "kwrap/examsys.h"
#include "kwrap/sxcmd.h"
#include <kwrap/cmdsys.h>
#include "kwrap/error_no.h"
#include <kwrap/util.h>
#include "kwrap/perf.h"
#include "hd_common.h"
#include <sys/stat.h>
#include "ImageApp/ImageApp_UsbMovie.h"
#include "umsd.h"
#include "sdio.h"

#define BULK_PACKET_SIZE    512

#define USB_VID                         0x0603
#define USB_PID_PCCAM                   0x8612 // not support pc cam
#define USB_PID_WRITE                   0x8614
#define USB_PID_PRINT                   0x8613
#define USB_PID_MSDC                    0x8611

#define USB_PRODUCT_REVISION            '1', '.', '0', '0'
#define USB_VENDER_DESC_STRING          'N', 0x00,'O', 0x00,'V', 0x00,'A', 0x00,'T', 0x00,'E', 0x00,'K', 0x00, 0x20, 0x00,0x00, 0x00 // NULL
#define USB_VENDER_DESC_STRING_LEN      0x09
#define USB_PRODUCT_DESC_STRING         'D', 0x00,'E', 0x00,'M', 0x00,'O', 0x00,'1', 0x00, 0x20, 0x00,0x00, 0x00 // NULL
#define USB_PRODUCT_DESC_STRING_LEN     0x07
#define USB_PRODUCT_STRING              'N','v','t','-','D','S','C'
#define USB_SERIAL_NUM_STRING           '9', '8', '5', '2', '0','0', '0', '0', '0', '0','0', '0', '1', '0', '0'
#define USB_VENDER_STRING               'N','O','V','A','T','E','K'
#define USB_VENDER_SIDC_DESC_STRING     'N', 0x00,'O', 0x00,'V', 0x00,'A', 0x00,'T', 0x00,'E', 0x00,'K', 0x00, 0x20, 0x00,0x00, 0x00 // NULL
#define USB_VENDER_SIDC_DESC_STRING_LEN 0x09

#define VDO_SIZE_W_8M      3840
#define VDO_SIZE_H_8M      2160
#define VDO_SIZE_W_5M      2592
#define VDO_SIZE_H_5M      1944
#define VDO_SIZE_W_4M      2560
#define VDO_SIZE_H_4M      1440
#define VDO_SIZE_W_2M      1920
#define VDO_SIZE_H_2M      1080
#define VDO_SIZE_W_720P    1280
#define VDO_SIZE_H_720P    720
#define VDO_SIZE_W_360P    640
#define VDO_SIZE_H_360P    360
#define NVT_UI_UVAC_RESO_CNT  2
#if UVC_SUPPORT_YUV_FMT
#define MAX_YUV_W 640
#define MAX_YUV_H 480
#define MAX_YUV_FPS 30
#endif
#define SEN_SEL_IMX290   0   //2M_SHDR
#define SEN_SEL_OS05A10  1   //5M_SHDR
#define SEN_SEL_GC4653   2   //4M
static UINT32 sensor_sel = SEN_SEL_IMX290;

UINT32 cap_size_w = 0;
UINT32 cap_size_h = 0;

#define UVC_MJPEG_STREAM_BUF_MARGIN 110 //%
#define UVC_MJPEG_ENC_BLK_CNT   3
#define UVC_MJPEG_ENC_BUF_MS(fps) (1000*UVC_MJPEG_ENC_BLK_CNT*UVC_MJPEG_STREAM_BUF_MARGIN/100/(fps))

#define UVC_H264_ENC_BUF_MS 2000

#define MJPEG_TBR_MAX   (18*8*1024*1024)
#define MJPEG_TBR_2M    (18*8*1024*1024)
#define MJPEG_TBR_720P  (7*8*1024*1024)

#define H264_TBR_MAX    (2*8*1024*1024)
#define H264_TBR_2M     (8*1024*1024)
#define H264_TBR_720P   (8*1024*1024)

static UVAC_VID_RESO gUIUvacVidReso[NVT_UI_UVAC_RESO_CNT] = {
	{ VDO_SIZE_W_2M,   VDO_SIZE_H_2M,   1,      MAX_BS_FPS,      0,      0},
	{ VDO_SIZE_W_720P,   VDO_SIZE_H_720P,   1,   MAX_BS_FPS,      0,      0},
};

#define NVT_UI_UVAC_AUD_SAMPLERATE_CNT                  1
#define NVT_UI_FREQUENCY_16K                    0x003E80   //16k
#define NVT_UI_FREQUENCY_32K                    0x007D00   //32k
#define NVT_UI_FREQUENCY_48K                    0x00BB80   //48k
static UINT32 gUIUvacAudSampleRate[NVT_UI_UVAC_AUD_SAMPLERATE_CNT] = {
	NVT_UI_FREQUENCY_16K
};
#if UVC_SUPPORT_YUV_FMT
static UVAC_VID_RESO gUIUvacVidYuvReso[] = {
	{MAX_YUV_W,	MAX_YUV_H,	1,		MAX_YUV_FPS,		 0,		 0},
	{320,	240,	1,		30,		 0,		 0},
};
#endif
static UVAC_VIDEO_FORMAT m_VideoFmt[UVAC_VID_DEV_CNT_MAX] = {0}; //UVAC_VID_DEV_CNT_MAX=2
static BOOL m_bIsStaticPattern = FALSE;

static UINT32 uvac_pa = 0;
static UINT32 uvac_va = 0;
static UINT32 uvac_size = 0;
#if UVC_DPC_FUNC
UINT32 g_capbind = 0;  //0:D2D, 1:direct, 2: one-buf, 0xff: no-bind
#else
UINT32 g_capbind = 0;  //0:D2D, 1:direct, 2: one-buf, 0xff: no-bind
#endif
static UINT32 g_one_buf = 1;    //0x0: vprc noraml, 0x1: one buffer
static UINT32 g_low_latency = 0;
static BOOL UVAC_enable(void);
static BOOL UVAC_disable(void);
#endif

#if AUDIO_FILTER
#include "audfilt/audlib_filt.h"
#endif

#if UVC_DPC_FUNC
#include "dpc_lib.h"
#include "kwrap/file.h"
#include "kwrap/util.h"
#endif

#if UVC_SHUTTER
BOOL shutterFlag = FALSE;
#endif
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
#define SEN_OUT_FMT		HD_VIDEO_PXLFMT_RAW12
#if UVC_DPC_FUNC
#define CAP_OUT_FMT		HD_VIDEO_PXLFMT_RAW12
#else
#define CAP_OUT_FMT		HD_VIDEO_PXLFMT_NRX12
#endif

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

#define VIDEOCAP_ALG_FUNC (0)
#define VIDEOPROC_ALG_FUNC (0)

///////////////////////////////////////////////////////////////////////////////
HD_VIDEOENC_BUFINFO phy_buf_main;
HD_VIDEOENC_BUFINFO phy_buf_sub1;
HD_VIDEOENC_BUFINFO phy_buf_sub2;
HD_AUDIOCAP_BUFINFO phy_buf_main2;
UINT32 vir_addr_main;
UINT32 vir_addr_sub1;
UINT32 vir_addr_sub2;
UINT32 vir_addr_main2;
VK_TASK_HANDLE cap_thread_id;
UINT32 flow_audio_start = 0, encode_start = 0, acquire_start = 0;
#define JPG_YUV_TRANS 1
pthread_mutex_t flow_start_lock;

#include "vendor_isp.h"

VIDEO_STREAM stream[1] = {0}; //0: main stream, 1: sub stream

//static UINT8 yuv_buf[160000] = {0};

#if  AUDIO_ON
typedef struct _AUDIO_CAPONLY {
	HD_AUDIO_SR sample_rate_max;
	HD_AUDIO_SR sample_rate;
	HD_AUDIO_SOUND_MODE sound_mode;
	UINT32 volume;

	HD_PATH_ID cap_ctrl;
	HD_PATH_ID cap_path;

	void *anr_work_buf_addr;
	UINT32 anr_work_buf_size;

	UINT32 cap_exit;
} AUDIO_CAPONLY;

AUDIO_CAPONLY au_cap = {0};
#endif

#define CS_INTERFACE                        0x24
#define VC_EXTENSION_UNIT                   0x06

UINT8 get_eu_control_num(UINT8 *p_control, UINT8 size)
{
	UINT32 i, j;
	UINT8 num = 0, temp;

	for (j = 0; j < size; j++) {
		for (i = 0; i < 8; i++) {
			temp  = p_control[j] >> i;
			if (temp & 0x1) {
				num++;
			}
		}
	}
	return num;
}

#if UVAC_SUSPEND_FUNC
/*
static void clk_resume_cb(void)
{
	#if defined(_EMBMEM_SPI_NAND_)
	clk_copy_nand_resume();
	#elif defined(_EMBMEM_SPI_NOR_)
	clk_copy_nor_resume();
	#else
	printf("ERR:[clk_resume_cb] Not supported storage!\r\n");
	#endif
}*/
static HD_RESULT close_module(VIDEO_STREAM *p_stream);
static void UVACSuspendCB(void)
{
	UINT32 ret1,ret2,ret3=0;
    UINT32 value =1;

	printf("UVACSuspendCB++\r\n");
#if AUDIO_ON
	vendor_audiocap_set(au_cap.cap_ctrl, VENDOR_AUDIOCAP_ITEM_DMA_ABORT, &value);
#endif


    ret1=vendor_videoproc_set(stream[0].proc_ctrl,VENDOR_VIDEOPROC_PARAM_DMA_ABORT,&value);
    ret2=vendor_videocap_set(stream[0].cap_ctrl,VENDOR_VIDEOCAP_PARAM_ABORT,&value);
    ret3=vendor_videoenc_set(stream[0].enc_main_path,VENDOR_VIDEOENC_PARAM_OUT_DMA_ABORT,&value);
    printf("abort_module %d %d %d\r\n",ret1,ret2,ret3);


	printf("UVACSuspendCB--\r\n");
}
#endif

static HD_RESULT mem_init(void)
{
	HD_RESULT              ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
	HD_DIM buf_size;
	UINT32 id;

	// config common pool (cap)
	id = 0;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	if (g_capbind == 1) {
		//direct ... NOT require raw
		mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
														+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
	} else {
		buf_size.w = MAX_CAP_SIZE_W;
		buf_size.h = MAX_CAP_SIZE_H;
		mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_RAW_BUFSIZE(buf_size.w, buf_size.h, CAP_OUT_FMT)
														+VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
														+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
	}
	mem_cfg.pool_info[id].blk_cnt = 2;
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;

	// config common pool (main)
	id++;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(MAX_CAP_SIZE_W, ALIGN_CEIL_16(MAX_CAP_SIZE_H), HD_VIDEO_PXLFMT_YUV420);	//padding to 16x
	mem_cfg.pool_info[id].blk_cnt = 1;
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;

#if UVC_SUPPORT_YUV_FMT
	id++;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(MAX_YUV_W, ALIGN_CEIL_16(MAX_YUV_H), HD_VIDEO_PXLFMT_YUV422);	//padding to 16x
	mem_cfg.pool_info[id].blk_cnt = 6;
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;
#endif

#if AUDIO_ON
	// config common pool audio (main)
	id++;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[id].blk_size = 0x10000;
	mem_cfg.pool_info[id].blk_cnt = 2;
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;
#endif

#if (MSDC_FUNC == 1)
	mem_cfg.pool_info[0].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
	mem_cfg.pool_info[0].blk_size = POOL_SIZE_USER_DEFINIED;
	mem_cfg.pool_info[0].blk_cnt = 1;
	mem_cfg.pool_info[0].ddr_id = DDR_ID0;
#endif

#if COMMON_RECFRM_BUFFER
	id++;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(ALIGN_CEIL_64(MAX_BS_W), ALIGN_CEIL_64(MAX_BS_H), HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[id].blk_cnt = COMMON_RECFRM_FUNC_BASE + COMMON_RECFRM_FUNC_SVC + COMMON_RECFRM_FUNC_LTR;
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;
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
/*AUDIO */
#if AUDIO_ON

#if AUDIO_FILTER
static void audiocap_filt_design(UINT32 sample_rate, UINT32 mode)
{
	AUDFILT_INIT    filt_init = {0};
	AUDFILT_EQPARAM eq_param  = {0};

	filt_init.filt_ch        = mode; //channel count
	filt_init.smooth_enable  = 1;

	// open audio filter
	if (!audlib_filt_open(&filt_init)) {
		printf("Audio filter open err!\r\n");
	}

	// init audio filter
	if (!audlib_filt_init()) {
		printf("Audio filter init err!\r\n");
	}

	eq_param.sample_rate  = sample_rate;

	//Sample EQ

	audlib_filt_enable_eq(ENABLE, 3);  //Enable EQ, 3 is total number of filter

	//200 Hz high pass filter
	eq_param.filt_type   = AUDFILT_DESIGNTYPE_HIGHPASS;
	eq_param.frequency   = 200;
	eq_param.gain_db     = -15;
	eq_param.Q           = 1;
	audlib_filt_design_eq(0, &eq_param); //filter index 0

	//4000 Hz low shelf filter
	eq_param.filt_type   = AUDFILT_DESIGNTYPE_LOWSHELF;
	eq_param.frequency   = 4000;
	eq_param.gain_db     = 5;
	eq_param.Q           = 0.45;
	audlib_filt_design_eq(1, &eq_param); //filter index 1

	//peaking 2000Hz peaking filter
	eq_param.filt_type   = AUDFILT_DESIGNTYPE_PEAKING;
	eq_param.frequency   = 2000;
	eq_param.gain_db     = 6;
	eq_param.Q           = 1;
	audlib_filt_design_eq(2, &eq_param); //filter index 2

	return;
}
#endif

#if AUDIO_ANR
#include "audaec/audlib_aec.h"

static UINT32 audiocap_anr_open(UINT32 samplerate, UINT32 channelnum)
{
	audlib_ns_open();

	audlib_ns_set_config(AUDNS_CONFIG_ID_SAMPLERATE, samplerate);
	audlib_ns_set_config(AUDNS_CONFIG_ID_CHANNEL_NO, channelnum);

	au_cap.anr_work_buf_size = audlib_ns_get_required_buffer_size();

	au_cap.anr_work_buf_addr = malloc(au_cap.anr_work_buf_size);

	audlib_ns_set_config(AUDNS_CONFIG_ID_BUF_ADDR, (UINT32)au_cap.anr_work_buf_addr);
	audlib_ns_set_config(AUDNS_CONFIG_ID_BUF_SIZE, au_cap.anr_work_buf_size);

	audlib_ns_set_config(AUDNS_CONFIG_ID_NOISE_CANCEL_LVL,   -6);

	audlib_ns_init();

	return 0;
}

static UINT32 audiocap_anr_close(void)
{
	audlib_ns_close();

	if (au_cap.anr_work_buf_addr) {

		free(au_cap.anr_work_buf_addr);
		au_cap.anr_work_buf_addr = 0;
	}

	return 0;
}

static UINT32 audiocap_anr_apply(UINT32 inaddr ,UINT32 insize)
{
	AUDNS_BITSTREAM AudNSIO;

	AudNSIO.bitstram_buffer_in     = inaddr;
	AudNSIO.bitstram_buffer_out    = inaddr;
	AudNSIO.bitstram_buffer_length = insize >> 1;

	if (au_cap.sound_mode == HD_AUDIO_SOUND_MODE_STEREO) {
		AudNSIO.bitstram_buffer_length = AudNSIO.bitstram_buffer_length >> 1;
	}

	audlib_ns_run(&AudNSIO);

	return 0;
}
#endif

static HD_RESULT set_cap_cfg2(HD_PATH_ID *p_audio_cap_ctrl, HD_AUDIO_SR sample_rate, HD_AUDIO_SOUND_MODE mode, UINT32 vol)
{
	HD_RESULT ret;
	HD_PATH_ID audio_cap_ctrl = 0;
	HD_AUDIOCAP_DEV_CONFIG audio_dev_cfg = {0};
	HD_AUDIOCAP_DRV_CONFIG audio_drv_cfg = {0};
	INT32 prepwr_enable = TRUE;
	UINT32 gain_lvl = VENDOR_AUDIOCAP_GAIN_LEVEL32;
	UINT32 alc_en = FALSE;
	HD_AUDIOCAP_VOLUME audio_cap_volume = {0};

	ret = hd_audiocap_open(0, HD_AUDIOCAP_0_CTRL, &audio_cap_ctrl); //open this for device control
	if (ret != HD_OK) {
		return ret;
	}

	// set audiocap dev parameter
	audio_dev_cfg.in_max.sample_rate = sample_rate;
	audio_dev_cfg.in_max.sample_bit = HD_AUDIO_BIT_WIDTH_16;
	audio_dev_cfg.in_max.mode = mode;
	audio_dev_cfg.in_max.frame_sample = 256;
	audio_dev_cfg.frame_num_max = 10;
	ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_DEV_CONFIG, &audio_dev_cfg);
	if (ret != HD_OK) {
		return ret;
	}

	// set audiocap drv parameter
	audio_drv_cfg.mono = HD_AUDIO_MONO_RIGHT;
	ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_DRV_CONFIG, &audio_drv_cfg);

	// set PREPWR
	vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_PREPWR_ENABLE, &prepwr_enable);

	// set gain level to 32
	vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_GAIN_LEVEL, &gain_lvl);

	// set ALC disable
	vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_ALC_ENABLE, (VOID *)&alc_en);

	audio_cap_volume.volume = vol;
	hd_audiocap_set(au_cap.cap_ctrl, HD_AUDIOCAP_PARAM_VOLUME, &audio_cap_volume);

	*p_audio_cap_ctrl = audio_cap_ctrl;

	return ret;
}

static HD_RESULT set_cap_param2(HD_PATH_ID audio_cap_path, HD_AUDIO_SR sample_rate, HD_AUDIO_SOUND_MODE mode)
{
	HD_RESULT ret = HD_OK;
	HD_AUDIOCAP_IN audio_cap_param = {0};


	// set audiocap input parameter
	audio_cap_param.sample_rate = sample_rate;
	audio_cap_param.sample_bit = HD_AUDIO_BIT_WIDTH_16;
	audio_cap_param.mode = mode;
	audio_cap_param.frame_sample = 256;
	ret = hd_audiocap_set(audio_cap_path, HD_AUDIOCAP_PARAM_IN, &audio_cap_param);

#if AUDIO_FILTER
	audiocap_filt_design(sample_rate, mode);
#endif

#if AUDIO_ANR
	audiocap_anr_open(sample_rate, mode);
#endif

	return ret;
}

static HD_RESULT open_module2(AUDIO_CAPONLY *p_caponly)
{
	HD_RESULT ret;
	ret = set_cap_cfg2(&p_caponly->cap_ctrl, p_caponly->sample_rate_max, p_caponly->sound_mode, p_caponly->volume);
	if (ret != HD_OK) {
		printf("set cap-cfg fail\n");
		return HD_ERR_NG;
	}
	if((ret = hd_audiocap_open(HD_AUDIOCAP_0_IN_0, HD_AUDIOCAP_0_OUT_0, &p_caponly->cap_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT close_module2(AUDIO_CAPONLY *p_caponly)
{
	HD_RESULT ret;
	if((ret = hd_audiocap_close(p_caponly->cap_path)) != HD_OK)
		return ret;

#if AUDIO_ANR
	audiocap_anr_close();
#endif
	return HD_OK;
}

static HD_RESULT exit_module2(void)
{
	HD_RESULT ret;
	if((ret = hd_audiocap_uninit()) != HD_OK)
		return ret;
	return HD_OK;
}


THREAD_DECLARE(capture_thread, arg)
{
	HD_RESULT ret = HD_OK;
	HD_AUDIO_FRAME  data_pull;
	int get_phy_flag=0;
#if WRITE_BS
	char file_path_main[64], file_path_len[64];
	FILE *f_out_main, *f_out_len;
#endif
	AUDIO_CAPONLY *p_cap_only = (AUDIO_CAPONLY *)arg;
#if UVC_ON
	UVAC_STRM_FRM strmFrm = {0};
#endif
	#define PHY2VIRT_MAIN2(pa) (vir_addr_main2 + (pa - phy_buf_main2.buf_info.phy_addr))

	THREAD_ENTRY();
#if WRITE_BS
	/* config pattern name */
	snprintf(file_path_main, sizeof(file_path_main), "/mnt/sd/audio_bs_%d_%d_%d_pcm.dat", HD_AUDIO_BIT_WIDTH_16, HD_AUDIO_SOUND_MODE_MONO, p_cap_only->sample_rate);
	snprintf(file_path_len, sizeof(file_path_len), "/mnt/sd/audio_bs_%d_%d_%d_pcm.len", HD_AUDIO_BIT_WIDTH_16, HD_AUDIO_SOUND_MODE_MONO, p_cap_only->sample_rate);
#endif

#if WRITE_BS
	/* open output files */
	if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
		printf("open file (%s) fail....\r\n", file_path_main);
	} else {
		printf("\r\ndump main bitstream to file (%s) ....\r\n", file_path_main);
	}

	if ((f_out_len = fopen(file_path_len, "wb")) == NULL) {
		printf("open len file (%s) fail....\r\n", file_path_len);
	}
#endif

	/* pull data test */
	while (p_cap_only->cap_exit == 0) {

		if (!flow_audio_start){
			if (get_phy_flag){
				/* mummap for bs buffer */
				//printf("release common buffer2\r\n");
				if (vir_addr_main2)hd_common_mem_munmap((void *)vir_addr_main2, phy_buf_main2.buf_info.buf_size);

				get_phy_flag = 0;
			}
			/* wait flow_audio_start */
			msleep(10);
			continue;
		}else{
			if (!get_phy_flag){

				/* query physical address of bs buffer	(this can ONLY query after hd_audiocap_start() is called !!) */
				hd_audiocap_get(au_cap.cap_ctrl, HD_AUDIOCAP_PARAM_BUFINFO, &phy_buf_main2);

				/* mmap for bs buffer  (just mmap one time only, calculate offset to virtual address later) */
				vir_addr_main2 = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_main2.buf_info.phy_addr, phy_buf_main2.buf_info.buf_size);
				if (vir_addr_main2 == 0) {
					printf("mmap error\r\n");
					THREAD_RETURN(0);
				}

				get_phy_flag = 1;
			}
		}

		// pull data
		ret = hd_audiocap_pull_out_buf(p_cap_only->cap_path, &data_pull, 200); // >1 = timeout mode

		if (ret == HD_OK) {
			UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN2(data_pull.phy_addr[0]);
			UINT32 size = data_pull.size;
#if WRITE_BS
			UINT32 timestamp = hd_gettime_ms();
#endif
			//printf("audio fram size =%u\r\n",size);

#if AUDIO_ANR
			audiocap_anr_apply(PHY2VIRT_MAIN2(data_pull.phy_addr[0]) ,size);
#endif

#if AUDIO_FILTER
			{
				AUDFILT_BITSTREAM   AudBitStream;
				BOOL    bStatus;

				AudBitStream.bitstram_buffer_in     = PHY2VIRT_MAIN2(data_pull.phy_addr[0]);
				AudBitStream.bitstram_buffer_out    = PHY2VIRT_MAIN2(data_pull.phy_addr[0]);
				AudBitStream.bitstram_buffer_length     = size;

				// apply audio filter
				if (!(bStatus = audlib_filt_run(&AudBitStream))) {
					printf("Audio filtering err!\r\n");
				}
			}
#endif

#if UVC_ON
			hd_common_mem_flush_cache((void *)ptr, size);

			strmFrm.path = UVAC_STRM_AUD;
			strmFrm.addr = data_pull.phy_addr[0];
			strmFrm.size = data_pull.size;
			strmFrm.timestamp = data_pull.timestamp;
			UVAC_SetEachStrmInfo(&strmFrm);

			#if UVAC_WAIT_RELEASE
			UVAC_WaitStrmDone(UVAC_STRM_AUD);
			#endif
#endif

#if WRITE_BS
			// write bs
			if (f_out_main) fwrite(ptr, 1, size, f_out_main);
			if (f_out_main) fflush(f_out_main);

			// write bs len
			if (f_out_len) fprintf(f_out_len, "%d %d\n", size, timestamp);
			if (f_out_len) fflush(f_out_len);
#endif
			// release data
			ret = hd_audiocap_release_out_buf(p_cap_only->cap_path, &data_pull);
			if (ret != HD_OK) {
				printf("release buffer failed. ret=%x\r\n", ret);
			}
		}
	}

	if (get_phy_flag){
		/* mummap for bs buffer */
		//printf("release common buffer2\r\n");
		if (vir_addr_main2)hd_common_mem_munmap((void *)vir_addr_main2, phy_buf_main2.buf_info.buf_size);
	}

#if WRITE_BS
	/* close output file */
	if (f_out_main) fclose(f_out_main);
	if (f_out_len) fclose(f_out_len);
#endif
	THREAD_RETURN(0);
}
#endif // AUDIO_ON
/*AUDIO end*/

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
	#if (VCAP_PATTERN_GEN == 0)
	HD_VIDEOCAP_CTRL iq_ctl = {0};
	#endif
	
	if (sensor_sel == SEN_SEL_IMX290) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_imx290");
		printf("Using nvt_sen_imx290\n");
	} else if (sensor_sel == SEN_SEL_OS05A10) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os05a10");
		printf("Using nvt_sen_os05a10\n");
	} else if (sensor_sel == SEN_SEL_GC4653) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_gc4653");
		printf("Using nvt_sen_gc4653\n");
	}
	cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux =  0x220; //PIN_SENSOR_CFG_MIPI | PIN_SENSOR_CFG_MCLK
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.serial_if_pinmux = 0xF01;//PIN_MIPI_LVDS_CFG_CLK2 | PIN_MIPI_LVDS_CFG_DAT0|PIN_MIPI_LVDS_CFG_DAT1 | PIN_MIPI_LVDS_CFG_DAT2 | PIN_MIPI_LVDS_CFG_DAT3
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.cmd_if_pinmux = 0x01;//PIN_I2C_CFG_CH1
	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_SEL_CSI0_USE_C0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;//HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;//HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = HD_VIDEOCAP_SEN_IGNORE;
#if VCAP_PATTERN_GEN
	snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, HD_VIDEOCAP_SEN_PAT_GEN);
#endif
	ret = hd_videocap_open(0, HD_VIDEOCAP_0_CTRL, &video_cap_ctrl); //open this for device control
	if (ret != HD_OK) {
		return ret;
	}
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);

	#if (VCAP_PATTERN_GEN == 0)
	iq_ctl.func = VIDEOCAP_ALG_FUNC;
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);
	#endif

	*p_video_cap_ctrl = video_cap_ctrl;
	return ret;
}

HD_RESULT set_cap_param(HD_PATH_ID video_cap_path, HD_DIM *p_dim, int fps, HD_DIM *p_out_dim)
{
	HD_RESULT ret = HD_OK;
	{//select sensor mode, manually or automatically
		HD_VIDEOCAP_IN video_in_param = {0};

		video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO; //auto select sensor mode by the parameter of HD_VIDEOCAP_PARAM_OUT
		video_in_param.frc = HD_VIDEO_FRC_RATIO(fps,1);
		video_in_param.dim.w = p_dim->w;
		video_in_param.dim.h = p_dim->h;
		video_in_param.pxlfmt = SEN_OUT_FMT;
		video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
		#if VCAP_PATTERN_GEN
		video_in_param.sen_mode = HD_VIDEOCAP_PATGEN_MODE(HD_VIDEOCAP_SEN_PAT_COLORBAR, 200);
		#endif
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN, &video_in_param);
		printf("set_cap_param fps=%d\r\n", fps);
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
		video_crop_param.win.rect.w = MAX_CAP_SIZE_W/2;
		video_crop_param.win.rect.h= MAX_CAP_SIZE_H/2;
		video_crop_param.align.w = 4;
		video_crop_param.align.h = 4;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);
		//printf("set_cap_param CROP ON=%d\r\n", ret);
	}
	#endif
	{
		HD_VIDEOCAP_OUT video_out_param = {0};

		//without setting dim for no scaling, using original sensor out size
		video_out_param.dim.w = p_out_dim->w;
		video_out_param.dim.h = p_out_dim->h;
		video_out_param.pxlfmt = CAP_OUT_FMT;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
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

	return ret;
}

HD_RESULT set_proc_cfg(HD_PATH_ID *p_video_proc_ctrl, HD_DIM* p_max_dim, int en_slice)
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
		video_cfg_param.isp_id = 0;
		video_cfg_param.ctrl_max.func = VIDEOPROC_ALG_FUNC;
		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;
		video_cfg_param.in_max.pxlfmt = CAP_OUT_FMT;
		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1,1);
		printf("vin w= %d, h = %d\r\n", p_max_dim->w, p_max_dim->h);
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
		if (ret != HD_OK) {
			return HD_ERR_NG;
		}
	}
	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		video_path_param.in_func = 0;
		if (g_capbind == 1)
			video_path_param.in_func |= HD_VIDEOPROC_INFUNC_DIRECT; //direct NOTE: enable direct

		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
		//printf("set_proc_param PATH_CONFIG=0x%X\r\n", ret);
	}
	video_ctrl_param.func = VIDEOPROC_ALG_FUNC;
	video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_0_OUT_0;
	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);
	if (ret != HD_OK) {
		return HD_ERR_NG;
	}
	// set slice mode
	#if FUNC_SLICE_MODE
	if (1){
		VENDOR_VIDEOPROC_SLICE_MODE slice_mode = {0};

		if (en_slice) {
			slice_mode.enable[0] = 1;
			slice_mode.enable[1] = 1;
			slice_mode.cnt = 16;
		}
		ret = vendor_videoproc_set(video_proc_ctrl, VENDOR_VIDEOPROC_PARAM_SLICE_MODE, &slice_mode);
		if (ret != HD_OK) {
			printf("VENDOR_VIDEOPROC_PARAM_SLICE_MODE failed!(%d)\r\n", ret);
		}
	}
	#endif
	*p_video_proc_ctrl = video_proc_ctrl;

	return ret;
}

HD_RESULT set_proc_param(HD_PATH_ID video_proc_path, HD_DIM* p_dim, HD_VIDEO_PXLFMT pxlfmt, UINT32 pull_allow, UINT32 b_one_buf)
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
		video_out_param.depth = pull_allow; //set 1 to allow pull
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	}

	HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};
	video_path_param.out_func = 0;
	if (g_low_latency == 1 && video_proc_path == stream[0].proc_main_path){
		//only the proc_path should be set
		printf("g_low_latency = %d\r\n", g_low_latency);
		video_path_param.out_func |= HD_VIDEOPROC_OUTFUNC_LOWLATENCY;
	}
	if (b_one_buf == 1){
		printf("g_one_buf = %d\r\n", g_one_buf);
		video_path_param.out_func |= HD_VIDEOPROC_OUTFUNC_ONEBUF;
	}
	ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	if (ret != HD_OK) {
		return HD_ERR_NG;
	}

	if (stream[0].codec_type == HD_CODEC_TYPE_JPEG){
		UINT32 h_align = 16;
		//to avoid MJPEG might encode read YUV out of range
		ret = vendor_videoproc_set(video_proc_path, VENDOR_VIDEOPROC_PARAM_HEIGHT_ALIGN, &h_align);
		if (ret != HD_OK) {
			printf("VENDOR_VIDEOPROC_PARAM_HEIGHT_ALIGN failed!(%d)\r\n", ret);
		}
	} else {
		UINT32 h_align = 2;
		//to avoid MJPEG might encode read YUV out of range
		ret = vendor_videoproc_set(video_proc_path, VENDOR_VIDEOPROC_PARAM_HEIGHT_ALIGN, &h_align);
		if (ret != HD_OK) {
			printf("VENDOR_VIDEOPROC_PARAM_HEIGHT_ALIGN failed!(%d)\r\n", ret);
		}
	}
	return ret;
}

static HD_RESULT set_enc_cfg(HD_PATH_ID video_enc_path, HD_DIM *p_max_dim, HD_VIDEO_CODEC codec_type, UINT32 max_bitrate)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOENC_PATH_CONFIG video_path_config = {0};
	VENDOR_VIDEOENC_H26X_ENC_COLMV h26x_enc_colmv = {0};
	VENDOR_VIDEOENC_FIT_WORK_MEMORY fit_work_mem_cfg = {0};

	//fit_work_mem_cfg.b_enable = TRUE;
	fit_work_mem_cfg.b_enable = FALSE;
    vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_FIT_WORK_MEMORY, &fit_work_mem_cfg);

	h26x_enc_colmv.h26x_colmv_en = 0;
	vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_COLMV, &h26x_enc_colmv);

#if COMMON_RECFRM_BUFFER
	VENDOR_VIDEOENC_H26X_COMM_RECFRM h26x_comm_recfrm = {0};

	h26x_comm_recfrm.enable = 1;
	h26x_comm_recfrm.h26x_comm_base_recfrm_en = COMMON_RECFRM_FUNC_BASE;
	h26x_comm_recfrm.h26x_comm_svc_recfrm_en = COMMON_RECFRM_FUNC_SVC;
	h26x_comm_recfrm.h26x_comm_ltr_recfrm_en = COMMON_RECFRM_FUNC_LTR;
	vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_COMM_RECFRM, &h26x_comm_recfrm);
#endif

#if FUNC_SLICE_MODE
	VENDOR_VIDEOENC_BS_QUEUE_CNT_CFG  bs_queue_cfg = {0};

	bs_queue_cfg.queue_cnt = 180;
	vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_BS_QUEUE_CNT, &bs_queue_cfg);
#endif

	if (p_max_dim != NULL) {

		//--- HD_VIDEOENC_PARAM_PATH_CONFIG ---
		video_path_config.max_mem.codec_type = codec_type;
		video_path_config.max_mem.max_dim.w  = p_max_dim->w;
		video_path_config.max_mem.max_dim.h  = p_max_dim->h;
		video_path_config.max_mem.bitrate    = max_bitrate;
		video_path_config.max_mem.enc_buf_ms = UVC_H264_ENC_BUF_MS;
		video_path_config.max_mem.svc_layer  = HD_SVC_4X;
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
	{
		HD_VIDEOENC_FUNC_CONFIG video_func_config = {0};
			video_func_config.in_func = 0;
		if (g_low_latency == 1)
			video_func_config.in_func |= HD_VIDEOENC_INFUNC_LOWLATENCY;
		else if (g_one_buf == 1)
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
	HD_VIDEOENC_OUT2 video_out2_param = {0};
	HD_H26XENC_RATE_CONTROL rc_param = {0};

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

		if (enc_type == HD_CODEC_TYPE_H264) {
			VENDOR_VIDEOENC_MIN_RATIO_CFG min_ratio = {0};

			min_ratio.min_i_ratio = UVC_H264_ENC_BUF_MS/2;
			min_ratio.min_p_ratio = UVC_H264_ENC_BUF_MS/3;
			vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_MIN_RATIO, &min_ratio);

			//printf("enc_type = HD_CODEC_TYPE_H264\r\n");
			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_H264;
			video_out_param.h26x.profile       = HD_H264E_HIGH_PROFILE;
			video_out_param.h26x.level_idc     = HD_H264E_LEVEL_5_1;
			video_out_param.h26x.gop_num       = 15;
			video_out_param.h26x.ltr_interval  = 0;
			video_out_param.h26x.ltr_pre_ref   = 0;
			video_out_param.h26x.gray_en       = 0;
			video_out_param.h26x.source_output = 0;
			video_out_param.h26x.svc_layer     = HD_SVC_4X;
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

		} else if (enc_type == HD_CODEC_TYPE_JPEG) {
			//printf("enc_type = HD_CODEC_TYPE_JPEG\r\n");
			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			VENDOR_VIDEOENC_MIN_RATIO_CFG min_ratio = {0};

			//ms*(PATH CONFIG target bitrate) to check if remaining buffer size is enough
			//only min_i_ratio is valid in MJPEG encode. bitstream buffer is designed to be less than (ms/1000)*target bitrate
			min_ratio.min_i_ratio = 1000*UVC_MJPEG_STREAM_BUF_MARGIN/100;//ms
			min_ratio.min_p_ratio = min_ratio.min_i_ratio;
			vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_MIN_RATIO, &min_ratio);

#if 0	//fixed quality
			video_out_param.codec_type         = HD_CODEC_TYPE_JPEG;
			video_out_param.jpeg.retstart_interval = 0;
			if (p_dim->w >= VDO_SIZE_W_4M ) {
				video_out_param.jpeg.image_quality = 90;
			} else {
				video_out_param.jpeg.image_quality = 95;
			}
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &video_out_param);
			if (ret != HD_OK) {
				printf("set_enc_param_out = %d\r\n", ret);
				return ret;
			}
#else	//MJPEG CBR
			video_out2_param.codec_type         = HD_CODEC_TYPE_JPEG;
			video_out2_param.jpeg.retstart_interval = 0;
			video_out2_param.jpeg.image_quality = 80;
			video_out2_param.jpeg.bitrate = bitrate;
			video_out2_param.jpeg.frame_rate_base = MAX_BS_FPS;
			video_out2_param.jpeg.frame_rate_incr = 1;
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM2, &video_out2_param);

			VENDOR_VIDEOENC_JPG_RC_CFG jpg_rc_param = {0};
			jpg_rc_param.vbr_mode_en = 0;  // 0: CBR, 1: VBR
			jpg_rc_param.min_quality = 1; // min quality
			jpg_rc_param.max_quality = 100; // max quality
			vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_JPG_RC, &jpg_rc_param);
			if (ret != HD_OK) {
				printf("set_enc_param_out = %d\r\n", ret);
				return ret;
			}
#endif
			//MJPG YUV trans(YUV420 to YUV422)
      #if JPG_YUV_TRANS
			VENDOR_VIDEOENC_JPG_YUV_TRANS_CFG yuv_trans_cfg = {0};
			yuv_trans_cfg.jpg_yuv_trans_en = JPG_YUV_TRANS;
			//printf("MJPG YUV TRANS(%d)\r\n", JPG_YUV_TRANS);
			vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_JPG_YUV_TRANS, &yuv_trans_cfg);
      #endif

		} else {

			printf("not support enc_type\r\n");
			return HD_ERR_NG;
		}
	}

	return ret;
}

static HD_RESULT init_module(void)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_init()) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_init()) != HD_OK)
		return ret;
    if ((ret = hd_videoenc_init()) != HD_OK)
		return ret;
#if AUDIO_ON
	if((ret = hd_audiocap_init()) != HD_OK)
        return ret;
#endif
	return HD_OK;
}

static HD_RESULT open_module(VIDEO_STREAM *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;
	// set videocap config
	ret = set_cap_cfg(&p_stream->cap_ctrl);
	if (ret != HD_OK) {
		printf("set cap-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, 0);
	if (ret != HD_OK) {
		printf("set proc-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	if ((ret = hd_videocap_open(HD_VIDEOCAP_0_IN_0, HD_VIDEOCAP_0_OUT_0, &p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &p_stream->proc_main_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_1,  &p_stream->proc_sub_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_VIDEOENC_0_OUT_0, &p_stream->enc_main_path)) != HD_OK)
		return ret;
#if UVC_SHUTTER
	if((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_MASK_0, &p_stream->vp_shutter_path)) != HD_OK)
		return ret;
#endif
	return HD_OK;
}

static HD_RESULT close_module(VIDEO_STREAM *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_close(p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_close(p_stream->proc_main_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_close(p_stream->proc_sub_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_close(p_stream->enc_main_path)) != HD_OK)
		return ret;
#if UVC_SHUTTER
	if((ret = hd_videoproc_close(p_stream->vp_shutter_path)) != HD_OK)
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

	return HD_OK;
}

static BOOL write_yuv = 0;

#if UVC_SUPPORT_YUV_FMT
THREAD_DECLARE(acquire_yuv_thread, arg)
{
	VIDEO_STREAM* p_stream0 = (VIDEO_STREAM *)arg;
	HD_RESULT ret = HD_OK;
	HD_VIDEO_FRAME video_frame = {0};
	UINT32 phy_addr_main, vir_addr_main;
	UINT32 yuv_size;

	HD_DIM dim;

	char file_path_main[32] = "/mnt/sd/dump_bs_sub.dat";
	FILE *f_out_sub;

	THREAD_ENTRY();
	dim.w = MAX_YUV_W;
	dim.h = MAX_YUV_H;

	//----- open output files -----
	

	#define PHY2VIRT_MAIN_YUV(pa) (vir_addr_main + ((pa) - phy_addr_main))

	printf("acquire_yuv_thread enter!!\r\n");
	//--------- pull data test ---------
	while (!p_stream0->acquire_exit) {
		if(acquire_start) {
			dim.w = stream[0].enc_main_dim.w;
			dim.h = stream[0].enc_main_dim.h;
			ret = hd_videoproc_pull_out_buf(p_stream0->proc_sub_path, &video_frame, 1000); // -1 = blocking mode, 0 = non-blocking mode, >0 = blocking-timeout mode
			if (ret != HD_OK) {
				printf("pull_out(1000) error = %d!!\r\n", ret);
				msleep(10);
				goto skip;
			}

			phy_addr_main = hd_common_mem_blk2pa(video_frame.blk); // Get physical addr
			if (phy_addr_main == 0) {
				printf("blk2pa fail, blk = 0x%x\r\n", video_frame.blk);
				goto skip;
			}

			yuv_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(dim.w, dim.h, HD_VIDEO_PXLFMT_YUV422);

			// mmap for frame buffer (just mmap one time only, calculate offset to virtual address later)
			//printf("phy_addr_main: 0x%x, 0x%x\n", phy_addr_main, yuv_size);
			vir_addr_main = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_main, yuv_size);
			if (vir_addr_main == 0) {
				printf("mmap error !!\r\n\r\n");
				goto skip;
			}

			UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN_YUV(video_frame.phy_addr[0]);


			if (0) {
				//memcpy((void *)ptr, (void *)yuv_buf, yuv_size);


				if (write_yuv == 0){

					if ((f_out_sub = fopen(file_path_main, "wb")) == NULL) {
						HD_VIDEOENC_ERR("open file (%s) fail....\r\n", file_path_main);
					} else {
						printf("\r\ndump main bitstream to file (%s) ....\r\n", file_path_main);

						setvbuf(f_out_sub, NULL, _IONBF, 0);
					}
					
					printf("=========== dim.w = %d, dim.h = %d, yuv size = %d\r\n", dim.w, dim.h, yuv_size);

					printf("=========== write ++\r\n");
					if (f_out_sub) {
						fwrite(ptr, 1, yuv_size, f_out_sub);
						printf("=========== write\r\n");
					}
					if (f_out_sub) fflush(f_out_sub);

					printf("=========== write --\r\n");

					if (f_out_sub) fclose(f_out_sub);

					write_yuv = 1;
				}
			}

			

			//printf("========  push yuv\r\n");
			
			//UINT32 len = video_frame.loff[0]*video_frame.ph[0] + video_frame.loff[1]*video_frame.ph[1];
			yuv_push_to_usb((UINT32)ptr, &video_frame);

			// mummap for frame buffer
			ret = hd_common_mem_munmap((void *)vir_addr_main, yuv_size);
			if (ret != HD_OK) {
				printf("mnumap error !!\r\n\r\n");
				goto skip;
			}

			ret = hd_videoproc_release_out_buf(p_stream0->proc_sub_path, &video_frame);
			if (ret != HD_OK) {
				printf("release_out() error !!\r\n\r\n");
				goto skip;
			}
		} else {
			msleep(10);
		}
skip:
		;
		//msleep(10);
	}

	// close output file
	
	
	THREAD_RETURN(0);
}
#endif // UVC_SUPPORT_YUV_FMT

THREAD_DECLARE(encode_thread, arg)
{

	VIDEO_STREAM* p_stream0 = (VIDEO_STREAM *)arg;
	HD_RESULT ret_main = HD_OK;
	HD_VIDEOENC_BS  data_pull_main;
	int get_phy_flag=0;

#if WRITE_BS
	char file_path_main[32] = "/mnt/sd/dump_bs_main.dat";
	FILE *f_out_main;
	UINT32 j;
#endif
	#define PHY2VIRT_MAIN(pa) (vir_addr_main + (pa - phy_buf_main.buf_info.phy_addr))
#if UVC_ON
	UVAC_STRM_FRM      strmFrm = {0};
	UINT32             loop =0;
#endif
	//UINT32 temp=1;
	THREAD_ENTRY();
#if WRITE_BS
	//----- open output files -----
	if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
		HD_VIDEOENC_ERR("open file (%s) fail....\r\n", file_path_main);
	} else {
		printf("\r\ndump main bitstream to file (%s) ....\r\n", file_path_main);
	}
#endif

	//--------- pull data test ---------
	while (p_stream0->enc_exit == 0) {
		if (!get_phy_flag){
			get_phy_flag = 1;
			// mmap for bs buffer (just mmap one time only, calculate offset to virtual address later)
			ret_main = hd_videoenc_get(stream[0].enc_main_path, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf_main);
			// mmap for bs buffer (just mmap one time only, calculate offset to virtual address later)
			vir_addr_main = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_main.buf_info.phy_addr, phy_buf_main.buf_info.buf_size);
		}

		if (!encode_start){
			//------ wait encode_start ------
			msleep(10);
			continue;
		}

		//pull data
		ret_main = hd_videoenc_pull_out_buf(p_stream0->enc_main_path, &data_pull_main, -1); // -1 = blocking mode

		{
			//UINT64 cur_time = hd_gettime_us();

			//printf("vd = %lld, encout = %lld, diff = %lld\r\n", data_pull.timestamp, cur_time, cur_time - data_pull.timestamp);
		}

		if (ret_main == HD_OK) {
#if UVC_ON
			if (p_stream0->codec_type == HD_CODEC_TYPE_JPEG && data_pull_main.pack_num == 2) {
				mjpg_slice_push_to_usb(PHY2VIRT_MAIN(data_pull_main.video_pack[0].phy_addr), &data_pull_main);
			} else {
				strmFrm.path = UVAC_STRM_VID ;
				strmFrm.addr = data_pull_main.video_pack[data_pull_main.pack_num-1].phy_addr;
				strmFrm.size = data_pull_main.video_pack[data_pull_main.pack_num-1].size;
				strmFrm.pStrmHdr = 0;
				strmFrm.strmHdrSize = 0;
				strmFrm.timestamp = data_pull_main.timestamp;
				if (p_stream0->codec_type == HD_CODEC_TYPE_H264){
					if (data_pull_main.pack_num > 1) {
						UINT32 header_size = 0;
						//if strmFrm.addr contains SPS/PPS, there is no need of pStrmHdr
						for (loop = 0; loop < data_pull_main.pack_num - 1; loop ++) {
							header_size += data_pull_main.video_pack[loop].size;
						}
						strmFrm.size += header_size;
						strmFrm.addr -= header_size;
					}
				}
				strmFrm.va = PHY2VIRT_MAIN(strmFrm.addr);
				//printf("++++va_main=0x%08X\n", strmFrm.va);
				if (stream[0].codec_type != HD_CODEC_TYPE_RAW){
					printf("111\r\n");
					UVAC_SetEachStrmInfo(&strmFrm);

					#if UVAC_WAIT_RELEASE
					#if PERF_TEST
					VOS_TICK t1 = 0, t2 = 0;
					UINT32 diff_us;
					static UINT32 cnt = 0;
					vos_perf_mark(&t1);
					#endif
					UVAC_WaitStrmDone(UVAC_STRM_VID);
					#if PERF_TEST
					vos_perf_mark(&t2);
					if (cnt%15 == 0) {
						diff_us = vos_perf_duration(t1, t2);
						printf("%dKB %dus(%dKB/sec)\r\n", strmFrm.size/1024, diff_us, strmFrm.size*1000/1024*1000/diff_us);
					}
					cnt++;
					#endif
					#endif
				}
			}
			/*timstamp for test
			if (temp){
				system("echo \"uvc\" > /proc/nvt_info/bootts");
				temp=0;
			}
			*/
#endif

#if WRITE_BS
			for (j=0; j< data_pull_main.pack_num; j++) {
				UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(data_pull_main.video_pack[j].phy_addr);
				UINT32 len = data_pull_main.video_pack[j].size;
				if (f_out_main) fwrite(ptr, 1, len, f_out_main);
				if (f_out_main) fflush(f_out_main);
			}

#endif

			// release data
			ret_main = hd_videoenc_release_out_buf(p_stream0->enc_main_path, &data_pull_main);
			if (ret_main != HD_OK) {
				printf("enc_release main error=%d !!\r\n", ret_main);
			}
		}
	}
	if (get_phy_flag){
		// mummap for bs buffer
		//printf("release common buffer\r\n");
		if (vir_addr_main) hd_common_mem_munmap((void *)vir_addr_main, phy_buf_main.buf_info.buf_size);
	}

#if WRITE_BS
	// close output file
	if (f_out_main) fclose(f_out_main);
#endif

	THREAD_RETURN(0);
}

void release_all_old_bs(VIDEO_STREAM* p_stream0)
{
	HD_RESULT ret_main = HD_OK;
	HD_VIDEOENC_BS  data_pull_main;

	while (1) {
		ret_main = hd_videoenc_pull_out_buf(p_stream0->enc_main_path, &data_pull_main, 0);
		if (ret_main != HD_OK) {
			break;
		}
		ret_main = hd_videoenc_release_out_buf(p_stream0->enc_main_path, &data_pull_main);
	}
}

VIDEO_STREAM* get_video_stream(void)
{
	return &stream[0];
}

#if UVC_ON
_ALIGNED(4) static UINT16 m_UVACSerialStrDesc3[] = {
	0x0320,                             // 20: size of String Descriptor = 32 bytes
	// 03: String Descriptor type
	'5', '1', '0', '5', '5',            // 96611-00000-001 (default)
	'0', '0', '0', '0', '0',
	'0', '0', '1', '0', '0'
};
_ALIGNED(4) const static UINT8 m_UVACManuStrDesc[] = {
	USB_VENDER_DESC_STRING_LEN * 2 + 2, // size of String Descriptor = 6 bytes
	0x03,                       // 03: String Descriptor type
	USB_VENDER_DESC_STRING
};

_ALIGNED(4) const static UINT8 m_UVACProdStrDesc[] = {
	USB_PRODUCT_DESC_STRING_LEN * 2 + 2, // size of String Descriptor = 6 bytes
	0x03,                       // 03: String Descriptor type
	USB_PRODUCT_DESC_STRING
};


static void USBMakerInit_UVAC(UVAC_VEND_DEV_DESC *pUVACDevDesc)
{
	extern BININFO bin_info;
	UINT16 major = (bin_info.Resv[BININFO_RESV_IDX_VER] >> 16) & 0xFF00;
	UINT16 minor = (bin_info.Resv[BININFO_RESV_IDX_VER] >> 8) & 0x0000FF;

	pUVACDevDesc->pManuStringDesc = (UVAC_STRING_DESC *)m_UVACManuStrDesc;
	pUVACDevDesc->pProdStringDesc = (UVAC_STRING_DESC *)m_UVACProdStrDesc;
	pUVACDevDesc->pSerialStringDesc = (UVAC_STRING_DESC *)m_UVACSerialStrDesc3;
	pUVACDevDesc->VID = USB_VID;
	pUVACDevDesc->PID = USB_PID_PCCAM;;
	pUVACDevDesc->bcdDevice = major|minor;

}

static UINT32 xUvacStartVideoCB(UVAC_VID_DEV_CNT vidDevIdx, UVAC_STRM_INFO *pStrmInfo)
{
	int ret=0;
	int fps;
	int bitrate;
	int en_slice = 0;
	PTZ_PARAM* ptz_param = NULL;
	//HD_VIDEOPROC_IN video_in_param = {0};

	//cfu_disable();

	if (pStrmInfo) {
		printf("UVAC Start[%d] resoIdx=%d,W=%d,H=%d,codec=%d,fps=%d,path=%d,tbr=0x%x\r\n", vidDevIdx, pStrmInfo->strmResoIdx, pStrmInfo->strmWidth, pStrmInfo->strmHeight, pStrmInfo->strmCodec, pStrmInfo->strmFps, pStrmInfo->strmPath, pStrmInfo->strmTBR);

		//stop poll data
		pthread_mutex_lock(&flow_start_lock);
		encode_start = 0;
		acquire_start = 0;
		pthread_mutex_unlock(&flow_start_lock);

		//ptz_param = uvac_get_ptz_param();
		if (ptz_param) {
			ptz_param->pan_v = 0;
			ptz_param->tilt_v = 0;
			ptz_param->zoom_idx = 0;
		}

		printf("g_capbind = %d\r\n", g_capbind);
		// stop VIDEO_STREAM modules (main)
		if (g_capbind == 1) {
			hd_videoproc_stop(stream[0].proc_main_path);
			hd_videoproc_stop(stream[0].proc_sub_path);
			hd_videocap_stop(stream[0].cap_path);
		} else {
			hd_videocap_stop(stream[0].cap_path);
			hd_videoproc_stop(stream[0].proc_main_path);
			hd_videoproc_stop(stream[0].proc_sub_path);

		}
		if (stream[0].codec_type != HD_CODEC_TYPE_RAW){
			//hd_videoenc_stop(stream[0].enc_main_path);
		}
		m_VideoFmt[vidDevIdx] = pStrmInfo->strmCodec;

		//set codec,resolution
		switch (pStrmInfo->strmCodec){
			case UVAC_VIDEO_FORMAT_H264:
				stream[0].codec_type = HD_CODEC_TYPE_H264;
				//g_low_latency = 1;
				break;
			case UVAC_VIDEO_FORMAT_MJPG:
				stream[0].codec_type = HD_CODEC_TYPE_JPEG;
				//g_low_latency = 0;
				break;
			case UVAC_VIDEO_FORMAT_YUV:
				stream[0].codec_type = HD_CODEC_TYPE_RAW;
				//g_low_latency = 0;
				break;
			default:
				printf("unrecognized codec(%d)\r\n",pStrmInfo->strmCodec);
				break;
		}
		printf("strmCodec = %d\r\n", pStrmInfo->strmCodec);
		//set encode resolution to device
		stream[0].enc_main_dim.w = pStrmInfo->strmWidth;
		stream[0].enc_main_dim.h = pStrmInfo->strmHeight;

		//one buffer limitation
		hd_videocap_unbind(HD_VIDEOCAP_0_OUT_0);
		hd_videoproc_close(stream[0].proc_main_path);
		hd_videoproc_close(stream[0].proc_sub_path);
		if (pStrmInfo->strmCodec == UVAC_VIDEO_FORMAT_YUV){
			en_slice = 1;
		} else if (pStrmInfo->strmCodec == UVAC_VIDEO_FORMAT_MJPG) {
			en_slice = 1;
		} else {
			en_slice = 0;
		}
		set_proc_cfg(&stream[0].proc_ctrl, &stream[0].cap_dim, en_slice);
		#if 0
		if (pStrmInfo->strmCodec != UVAC_VIDEO_FORMAT_YUV) {
			//set_proc_cfg(&stream[0].proc_ctrl, &stream[0].enc_main_dim);
			hd_videoproc_get(stream[0].proc_main_path, HD_VIDEOPROC_PARAM_IN, &video_in_param);
			video_in_param.dim.w = stream[0].enc_main_dim.w;
			video_in_param.dim.h = stream[0].enc_main_dim.h;
			hd_videoproc_set(stream[0].proc_main_path, HD_VIDEOPROC_PARAM_IN, &video_in_param);
		} else {
			//set_proc_cfg(&stream[0].proc_ctrl, &stream[0].cap_dim);
			hd_videoproc_get(stream[0].proc_main_path, HD_VIDEOPROC_PARAM_IN, &video_in_param);
			video_in_param.dim.w = stream[0].cap_dim.w;
			video_in_param.dim.h = stream[0].cap_dim.h;
			hd_videoproc_set(stream[0].proc_main_path, HD_VIDEOPROC_PARAM_IN, &video_in_param);
		}
		#endif
		hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &stream[0].proc_main_path);
		hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_1, &stream[0].proc_sub_path);
		hd_videocap_bind(HD_VIDEOCAP_0_OUT_0, HD_VIDEOPROC_0_IN_0);

		if (pStrmInfo->strmCodec == UVAC_VIDEO_FORMAT_YUV){
			//YUV format: stop vdoenc and allow pull from vprc sub path
			ret = set_proc_param(stream[0].proc_main_path, &stream[0].cap_dim, HD_VIDEO_PXLFMT_YUV420, 0, 1);
			ret = set_proc_param(stream[0].proc_sub_path, &stream[0].enc_main_dim, VDO_PXLFMT_YUV422_YUYV, 4, 0);
		} else {
			ret = set_proc_param(stream[0].proc_main_path, &stream[0].enc_main_dim, HD_VIDEO_PXLFMT_YUV420, 0, 1);
		}
		if (ret != HD_OK) {
			printf("set proc fail=%d\n", ret);
		}
		if (stream[0].codec_type == HD_CODEC_TYPE_JPEG) {
			//MJPG bitrate
			if (stream[0].enc_main_dim.w > VDO_SIZE_W_2M ) {
				bitrate = MJPEG_TBR_MAX;
			} else if(stream[0].enc_main_dim.w > VDO_SIZE_W_720P) {
				bitrate = MJPEG_TBR_2M;
			} else if(stream[0].enc_main_dim.w > VDO_SIZE_W_360P) {
				bitrate = MJPEG_TBR_720P;
			} else {
				bitrate = 8*400*1024;
			}
		} else if (stream[0].codec_type == HD_CODEC_TYPE_RAW) {
			//YUV bitrate
			bitrate = 8*12*1024*1024;
		} else {
			//H264 bitrate
			if (stream[0].enc_main_dim.w > VDO_SIZE_W_2M) {
				bitrate = H264_TBR_MAX;
			} else if(stream[0].enc_main_dim.w > VDO_SIZE_W_720P) {
				bitrate = H264_TBR_2M;
			} else {
				bitrate = H264_TBR_720P;
			}
		}
		// set videoenc parameter (main)
		if (stream[0].codec_type != HD_CODEC_TYPE_RAW){
			ret = set_enc_param(stream[0].enc_main_path, &stream[0].enc_main_dim, stream[0].codec_type, bitrate);
		}
		if (ret != HD_OK) {
			printf("set enc fail=%d\n", ret);
		}
		fps = pStrmInfo->strmFps;
		if (pStrmInfo->strmCodec == UVAC_VIDEO_FORMAT_YUV){
			ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim, fps, &stream[0].cap_dim);
		} else {
			ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim, fps, &stream[0].enc_main_dim);
		}
		if (ret != HD_OK) {
			printf("set cap fail=%d\n", ret);
		}
		if (g_capbind == 0) {
			//start engine(modules)
			hd_videocap_start(stream[0].cap_path);
		}
		hd_videoproc_start(stream[0].proc_main_path);
		if (pStrmInfo->strmCodec == UVAC_VIDEO_FORMAT_YUV){
			hd_videoproc_start(stream[0].proc_sub_path);
		}
		if (g_capbind == 1) {
			//start engine(modules)
			hd_videocap_start(stream[0].cap_path);
		}
		// just wait ae/awb stable for auto-test, if don't care, user can remove it
		//sleep(1);
		if (pStrmInfo->strmCodec != UVAC_VIDEO_FORMAT_YUV){
			release_all_old_bs(&stream[0]);
			//hd_videoenc_start(stream[0].enc_main_path);
		}

		if (pStrmInfo->strmCodec != UVAC_VIDEO_FORMAT_YUV){
			pthread_mutex_lock(&flow_start_lock);
			encode_start = 1;
			acquire_start = 0;
			pthread_mutex_unlock(&flow_start_lock);
		} else {
			pthread_mutex_lock(&flow_start_lock);
			encode_start = 0;
			acquire_start = 1;
			pthread_mutex_unlock(&flow_start_lock);
		}
	}

	write_yuv = 0;

	return E_OK;
}

static UINT32 xUvacStopVideoCB(UINT32 isClosed)
{
	//printf(":isClosed=%d\r\n", isClosed);
	printf("stop encode flow\r\n");
	//cfu_enable();
	pthread_mutex_lock(&flow_start_lock);
	encode_start = 0;
	acquire_start = 0;
	pthread_mutex_unlock(&flow_start_lock);
	return E_OK;
}

static UINT32 xUvacStartAudioCB(UVAC_AUD_DEV_CNT audDevIdx, UVAC_STRM_INFO *pStrmInfo)
{
	//UINT32 stop_acap = FALSE;

	if (pStrmInfo->isAudStrmOn) {
		//cfu_disable();
		//stop poll data
		pthread_mutex_lock(&flow_start_lock);
		flow_audio_start = 0;
		pthread_mutex_unlock(&flow_start_lock);

#if AUDIO_ON
		//start audio capture module
		hd_audiocap_start(au_cap.cap_path);
#endif
		pthread_mutex_lock(&flow_start_lock);
		flow_audio_start = 1;
		pthread_mutex_unlock(&flow_start_lock);
	} else {
		pthread_mutex_lock(&flow_start_lock);
		if (flow_audio_start) {
			//stop_acap = TRUE;
		}
		flow_audio_start = 0;
		pthread_mutex_unlock(&flow_start_lock);

#if AUDIO_ON
		//stop audio capture module
		if (stop_acap) {
			hd_audiocap_stop(au_cap.cap_path);
		}
#endif
		//cfu_enable();
		return E_OK;
	}

	return E_OK;
}

static UINT32 xUvacSetVolCB(UINT32 volume)
{
	#if AUDIO_ON

	HD_AUDIOCAP_VOLUME audio_cap_volume = {0};
	UINT32 step = volume/0x180;

	au_cap.volume = volume;

	//printf("vol=0x%x step=%d\r\n", (unsigned int)volume, (int)step);

	switch (step) {
		case 0:
			//mute
			audio_cap_volume.volume = 0;
			hd_audiocap_set(au_cap.cap_ctrl, HD_AUDIOCAP_PARAM_VOLUME, &audio_cap_volume);
			//hd_audiocap_start(au_cap.cap_path);
			break;
		case 1:
			//1.5 dB
			audio_cap_volume.volume = 45;
			hd_audiocap_set(au_cap.cap_ctrl, HD_AUDIOCAP_PARAM_VOLUME, &audio_cap_volume);
			//hd_audiocap_start(au_cap.cap_path);
			break;
		case 2:
			//3 dB
			audio_cap_volume.volume = 48;
			hd_audiocap_set(au_cap.cap_ctrl, HD_AUDIOCAP_PARAM_VOLUME, &audio_cap_volume);
			//hd_audiocap_start(au_cap.cap_path);
			break;
		case 3:
			//4.5 dB
			audio_cap_volume.volume = 51;
			hd_audiocap_set(au_cap.cap_ctrl, HD_AUDIOCAP_PARAM_VOLUME, &audio_cap_volume);
			//hd_audiocap_start(au_cap.cap_path);
			break;
		case 4:
			//6 dB
			audio_cap_volume.volume = 54;
			hd_audiocap_set(au_cap.cap_ctrl, HD_AUDIOCAP_PARAM_VOLUME, &audio_cap_volume);
			//hd_audiocap_start(au_cap.cap_path);
			break;
		case 5:
			//7.5 dB
			audio_cap_volume.volume = 57;
			hd_audiocap_set(au_cap.cap_ctrl, HD_AUDIOCAP_PARAM_VOLUME, &audio_cap_volume);
			//hd_audiocap_start(au_cap.cap_path);
			break;
		case 6:
			//9 dB
			audio_cap_volume.volume = 60;
			hd_audiocap_set(au_cap.cap_ctrl, HD_AUDIOCAP_PARAM_VOLUME, &audio_cap_volume);
			//hd_audiocap_start(au_cap.cap_path);
			break;
		case 7:
			//10.5 dB
			audio_cap_volume.volume = 63;
			hd_audiocap_set(au_cap.cap_ctrl, HD_AUDIOCAP_PARAM_VOLUME, &audio_cap_volume);
			//hd_audiocap_start(au_cap.cap_path);
			break;
		case 8:
			//12 dB
			audio_cap_volume.volume = 66;
			hd_audiocap_set(au_cap.cap_ctrl, HD_AUDIOCAP_PARAM_VOLUME, &audio_cap_volume);
			//hd_audiocap_start(au_cap.cap_path);
			break;
		case 9:
			//13.5 dB
			audio_cap_volume.volume = 69;
			hd_audiocap_set(au_cap.cap_ctrl, HD_AUDIOCAP_PARAM_VOLUME, &audio_cap_volume);
			//hd_audiocap_start(au_cap.cap_path);
			break;
#if 0
		case 10:
			//15 dB
			audio_cap_volume.volume = 72;
			hd_audiocap_set(au_cap.cap_ctrl, HD_AUDIOCAP_PARAM_VOLUME, &audio_cap_volume);
			//hd_audiocap_start(au_cap.cap_path);
			break;
		case 11:
			//16.5 dB
			audio_cap_volume.volume = 75;
			hd_audiocap_set(au_cap.cap_ctrl, HD_AUDIOCAP_PARAM_VOLUME, &audio_cap_volume);
			//hd_audiocap_start(au_cap.cap_path);
			break;
		case 12:
			//18 dB
			audio_cap_volume.volume = 78;
			hd_audiocap_set(au_cap.cap_ctrl, HD_AUDIOCAP_PARAM_VOLUME, &audio_cap_volume);
			//hd_audiocap_start(au_cap.cap_path);
			break;
		case 13:
			//19.5 dB
			audio_cap_volume.volume = 81;
			hd_audiocap_set(au_cap.cap_ctrl, HD_AUDIOCAP_PARAM_VOLUME, &audio_cap_volume);
			//hd_audiocap_start(au_cap.cap_path);
			break;
		case 14:
			//21 dB
			audio_cap_volume.volume = 84;
			hd_audiocap_set(au_cap.cap_ctrl, HD_AUDIOCAP_PARAM_VOLUME, &audio_cap_volume);
			//hd_audiocap_start(au_cap.cap_path);
			break;
		case 15:
			//22.5 dB
			audio_cap_volume.volume = 87;
			hd_audiocap_set(au_cap.cap_ctrl, HD_AUDIOCAP_PARAM_VOLUME, &audio_cap_volume);
			//hd_audiocap_start(au_cap.cap_path);
			break;
#endif
		default:
			printf("Not supported vol = 0x%x\r\n", volume);
			break;
	}

	pthread_mutex_lock(&flow_start_lock);
	if (flow_audio_start) {
		hd_audiocap_start(au_cap.cap_path);
	}
	pthread_mutex_unlock(&flow_start_lock);

	#endif

	return E_OK;
}

#if (MSDC_FUNC == 1)
static UINT32 user_va = 0;
static UINT32 user_pa = 0;
static HD_COMMON_MEM_VB_BLK msdc_blk;
static PMSDC_OBJ p_msdc_object;
static _ALIGNED(64) UINT8 InquiryData[36] = {
	0x00, 0x80, 0x05, 0x02, 0x20, 0x00, 0x00, 0x00,
//    //Vendor identification, PREMIER
	'N', 'O', 'V', 'A', 'T', 'E', 'K', '-',
//    //product identification, DC8365
	'N', 'T', '9', '8', '5', '2', '8', '-',
	'D', 'S', 'P', ' ', 'I', 'N', 'S', 'I',
//    //product revision level, 1.00
	'D', 'E', ' ', ' '
};

static BOOL emuusb_msdc_strg_detect(void)
{
	return 1;
}
#endif

#if (UVC_MSOS20_FUNC == ENABLE)
static UINT8 USBVideoMSOS20DescriptorSet[] =
{
	/* Microsoft OS 2.0 Descriptor Set Header */
    0x0A, 0x00,             // wLength of MSOS20_SET_HEADER_DESCRIPTOR
    0x00, 0x00,             // wDescriptorType == MSOS20_SET_HEADER_DESCRIPTOR
    0x00, 0x00, 0x00, 0x0A, // dwWindowsVersion - 0x10000000 for Windows 10
    0x88, 0x00,             // wTotalLength - Total length 0x2C8 (712)

    /* Microsoft OS 2.0 Configuration Subset Header */
    0x08, 0x00,             // wLength of MSOS20_SUBSET_HEADER_CONFIGURATION
    0x01, 0x00,             // wDescriptorType == MSOS20_SUBSET_HEADER_CONFIGURATION
    0x00,                   // bConfigurationValue set to the first configuration
    0x00,                   // bReserved set to 0.
    0x7E, 0x00,             // wTotalLength - Total length 0x2BE (702)

    /****************Color Camera Function******************/

    /* Microsoft OS 2.0 Function Subset Header */
    0x08, 0x00,             // wLength of MSOS20_SUBSET_HEADER_FUNCTION
    0x02, 0x00,             // wDescriptorType == MSOS20_SUBSET_HEADER_FUNCTION
    0x00,                   // bFirstInterface field of the first IAD
    0x00,                   // bReserved set to 0.
    0x76, 0x00,             // wSubsetLength - Length 0x16E (366)

    /****************Register the Color Camera in a sensor group******************/


    /****************Enable Platform DMFT for ROI-capable USB Camera************/

    /* Microsoft OS 2.0 Registry Value Feature Descriptor */
    0x3C, 0x00,             // wLength 0x3C (60) in bytes of this descriptor
    0x04, 0x00,             // wDescriptorType - MSOS20_FEATURE_REG_PROPERTY
    0x04, 0x00,             // wPropertyDataType - REG_DWORD
    0x2E, 0x00,             // wPropertyNameLength - 0x2E (46) bytes
    'U', 0x00, 'V', 0x00,   // Property Name - "UVC-EnablePlatformDmft"
    'C', 0x00, '-', 0x00,
    'E', 0x00, 'n', 0x00,
    'a', 0x00, 'b', 0x00,
    'l', 0x00, 'e', 0x00,
    'P', 0x00, 'l', 0x00,
    'a', 0x00, 't', 0x00,
    'f', 0x00, 'o', 0x00,
    'r', 0x00, 'm', 0x00,
    'D', 0x00, 'm', 0x00,
    'f', 0x00, 't', 0x00,
    0x00, 0x00,
    0x04, 0x00,              // wPropertyDataLength - 4 bytes
    0x01, 0x00, 0x00, 0x00,  // Enable Platform DMFT

	//
    // Microsoft OS 2.0 Registry Value Feature Descriptor
    //
	0x32, 0x00,               // wLength 0x32 (50) in bytes of this descriptor
	0x04, 0x00,               // wDescriptorType - MSOS20_FEATURE_REG_PROPERTY
	0x04, 0x00,               // wPropertyDataType - REG_DWORD
	0x24, 0x00,               // wPropertyNameLength - 0x24 (36) bytes
	'C',  0x00, 'a',  0x00,   // Property Name - "CacheUVCControl"
	'c',  0x00, 'h',  0x00,
    'e',  0x00, 'U',  0x00,
    'V',  0x00, 'C',  0x00,
    'C',  0x00, 'o',  0x00,
    'n',  0x00, 't',  0x00,
    'r',  0x00, 'o',  0x00,
    'l',  0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
	0x04, 0x00,               // wPropertyDataLength - 4 bytes
	0x01, 0x00, 0x00, 0x00,   // Enable to cache UVC controls
};
#endif

static BOOL UVAC_enable(void)
{

	UVAC_INFO       UvacInfo = {0};
	UINT32          retV = 0;

	UVAC_VEND_DEV_DESC UIUvacDevDesc = {0};
	UVAC_AUD_SAMPLERATE_ARY uvacAudSampleRateAry = {0};
	UINT32 bVidFmtType = UVAC_VIDEO_FORMAT_H264_MJPEG;
	UAC_VOL_INFO vol_info = {0};
	m_bIsStaticPattern = FALSE;

	printf("###Real Open UVAC-lib\r\n");

	HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;
	UINT32 pa;
	void *va;
	HD_RESULT hd_ret;

	hd_ret = hd_gfx_init();
	if(hd_ret != HD_OK) {
        printf("init gfx fail=%d\n", hd_ret);
	}

	#if (U3UVAC == 1)
	UVAC_SetObject(UVAC_TYPE_U3);
	#endif

	#if (MSDC_FUNC == 1)
	{
		STORAGE_OBJ *pStrg = sdio_getStorageObject(STRG_OBJ_FAT1);
		USB_MSDC_INFO       MSDCInfo;
		UINT32 i;
		HD_RESULT                 ret;
		UVAC_MSDC_INFO       uvac_msdc_info;

		if (pStrg == NULL) {
			printf("failed to get STRG_OBJ_FAT1.n");
			return -1;
		}

		msdc_blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_DEFINIED_POOL, POOL_SIZE_USER_DEFINIED, DDR_ID0);
	    if (HD_COMMON_MEM_VB_INVALID_BLK == msdc_blk) {
	            printf("hd_common_mem_get_block fail\r\n");
	            return HD_ERR_NG;
	    }
	    user_pa = hd_common_mem_blk2pa(msdc_blk);
	    if (user_pa == 0) {
	            printf("not get buffer, pa=%08x\r\n", (int)user_pa);
	            return HD_ERR_NG;
	    }
	    user_va = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, user_pa, POOL_SIZE_USER_DEFINIED);
	    /* Release buffer */
	    if (user_va == 0) {
	            printf("mem map fail\r\n");
	            ret = hd_common_mem_munmap((void *)user_va, POOL_SIZE_USER_DEFINIED);
	            if (ret != HD_OK) {
	                    printf("mem unmap fail\r\n");
	                    return ret;
	            }
	            return HD_ERR_NG;
	    }

		p_msdc_object = Msdc_getObject(MSDC_ID_USB20);
		p_msdc_object->SetConfig(USBMSDC_CONFIG_ID_SELECT_POWER,  USBMSDC_POW_SELFPOWER);
		p_msdc_object->SetConfig(USBMSDC_CONFIG_ID_COMPOSITE,  TRUE);

		MSDCInfo.uiMsdcBufAddr_va= (UINT32)user_va;
		MSDCInfo.uiMsdcBufAddr_pa= (UINT32)user_pa;
		if (MSDCInfo.uiMsdcBufAddr_pa == 0) {
			printf("malloc buffer failed\n");
		}
		MSDCInfo.uiMsdcBufSize = MSDC_MIN_BUFFER_SIZE;
		MSDCInfo.pInquiryData = (UINT8 *)&InquiryData[0];

		MSDCInfo.msdc_check_cb = NULL;
		MSDCInfo.msdc_vendor_cb = NULL;

		MSDCInfo.msdc_RW_Led_CB = NULL;
		MSDCInfo.msdc_StopUnit_Led_CB = NULL;
		MSDCInfo.msdc_UsbSuspend_Led_CB = NULL;

		for (i = 0; i < MAX_LUN; i++) {
			MSDCInfo.msdc_type[i] = MSDC_STRG;
		}

		MSDCInfo.pStrgHandle[0] = pStrg;//pSDIOObj
		MSDCInfo.msdc_strgLock_detCB[0] = NULL;
		MSDCInfo.msdc_storage_detCB[0] = emuusb_msdc_strg_detect;
		MSDCInfo.LUNs = 1;

		if (p_msdc_object->Open(&MSDCInfo) != E_OK) {
			printf("msdc open failed\r\n ");
		}

		uvac_msdc_info.en = TRUE;
		UVAC_SetConfig(UVAC_CONFIG_MSDC_INFO, (UINT32) &uvac_msdc_info);
	}
	#elif (MSDC_FUNC == 2)
	{
		UVAC_MSDC_INFO uvac_msdc_info;
		uvac_msdc_info.en = TRUE;
		nvt_cmdsys_runcmd("msdcnvt open");
		UVAC_SetConfig(UVAC_CONFIG_MSDC_INFO, (UINT32) &uvac_msdc_info);
	}
	#endif

	//UVAC_SetConfig(UVAC_CONFIG_MAX_FRAME_SIZE, 4*1024*1024); //default 800KB, MAX:3MB

	uvac_size = UVAC_GetNeedMemSize();

	uvac_size += 0x800000;
	//printf("uvac_size = %u\r\n", uvac_size);
	if ((hd_ret = hd_common_mem_alloc("usbmovie", &pa, (void **)&va, uvac_size, ddr_id)) != HD_OK) {
		printf("hd_common_mem_alloc failed(%d)\r\n", hd_ret);
		uvac_va = 0;
		uvac_pa = 0;
		uvac_size = 0;
	} else {
		uvac_va = (UINT32)va;
		uvac_pa = (UINT32)pa;
	}

	UvacInfo.UvacMemAdr    = uvac_pa + 0x800000;
	UvacInfo.UvacMemSize   = uvac_size;

	UvacInfo.fpStartVideoCB  = (UVAC_STARTVIDEOCB)xUvacStartVideoCB;
	UvacInfo.fpStopVideoCB  = (UVAC_STOPVIDEOCB)xUvacStopVideoCB;
	UvacInfo.fpSetVolCB = (UVAC_SETVOLCB)xUvacSetVolCB;

	USBMakerInit_UVAC(&UIUvacDevDesc);

	UVAC_SetConfig(UVAC_CONFIG_VEND_DEV_DESC, (UINT32)&UIUvacDevDesc);
	//UVAC_SetConfig(UVAC_CONFIG_HW_COPY_CB, (UINT32)hd_gfx_memcpy);

	UVAC_SetConfig(UVAC_CONFIG_AUD_START_CB, (UINT32)xUvacStartAudioCB);
	//printf("%s:uvacAddr=0x%x,s=0x%x;IPLAddr=0x%x,s=0x%x\r\n", __func__, UvacInfo.UvacMemAdr, UvacInfo.UvacMemSize, m_VideoBuf.addr, m_VideoBuf.size);

	#if 0 //sample code for customized EU descriptor
	{
		static UINT8 baSourceID[] = {0}; //this should be a static array, set 0 for default
		static UINT8 bmControls[] = {0xFF, 0x00}; //this should be a static array
		UVAC_EU_DESC eu_desc = {0};
		//UINT8 guid[16] = {0x29, 0xa7, 0x87, 0xc9, 0xd3, 0x59, 0x69, 0x45, 0x84, 0x67, 0xff, 0x08, 0x49, 0xfc, 0x19, 0xe8};
		UINT8 guid[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};

		eu_desc.bUnitID = 5;
		eu_desc.bDescriptorType = CS_INTERFACE;
		eu_desc.bDescriptorSubtype = VC_EXTENSION_UNIT;
		memcpy(eu_desc.guidExtensionCode, guid, sizeof(eu_desc.guidExtensionCode));
		eu_desc.bNrInPins = sizeof(baSourceID);
		eu_desc.baSourceID = baSourceID;
		eu_desc.bControlSize = sizeof(bmControls);
		eu_desc.bmControls = bmControls;
		eu_desc.bNumControls = get_eu_control_num(eu_desc.bmControls, eu_desc.bControlSize);
		eu_desc.bLength = 24 + eu_desc.bNrInPins + eu_desc.bControlSize;
		UVAC_SetConfig(UVAC_CONFIG_EU_DESC, (UINT32)&eu_desc);
	}
	#endif

	#if 0 //sample code for customized EU descriptor
	{
		static UINT8 baSourceID[] = {0}; //this should be a static array, set 0 for default
		static UINT8 bmControls[] = {0xFF, 0x00}; //this should be a static array
		UVAC_EU_DESC eu_desc = {0};
		//UINT8 guid[16] = {0x29, 0xa7, 0x87, 0xc9, 0xd3, 0x59, 0x69, 0x45, 0x84, 0x67, 0xff, 0x08, 0x49, 0xfc, 0x19, 0xe8};
		UINT8 guid[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};

		eu_desc.bUnitID = 5;
		eu_desc.bDescriptorType = CS_INTERFACE;
		eu_desc.bDescriptorSubtype = VC_EXTENSION_UNIT;
		memcpy(eu_desc.guidExtensionCode, guid, sizeof(eu_desc.guidExtensionCode));
		eu_desc.bNrInPins = sizeof(baSourceID);
		eu_desc.baSourceID = baSourceID;
		eu_desc.bControlSize = sizeof(bmControls);
		eu_desc.bmControls = bmControls;
		eu_desc.bNumControls = get_eu_control_num(eu_desc.bmControls, eu_desc.bControlSize);
		eu_desc.bLength = 24 + eu_desc.bNrInPins + eu_desc.bControlSize;
		eu_desc.eu_cb = (UVAC_UNIT_CB)xUvacEU2_CB;
		UVAC_SetConfig(UVAC_CONFIG_EU_DESC_2ND, (UINT32)&eu_desc);
	}
	#endif

	#if 0 //sample code for setting UVC and UAC string
	{
		//this should be a static array
		static UINT16 uvc_string_descriptor[] = {
			0x0316,                             // 16: size of String Descriptor = 22 bytes
												// 03: String Descriptor type
			'U', 'V', 'C', '_', 'D',            // UVC_DEVICE
			'E', 'V', 'I', 'C', 'E'
		};
		//this should be a static array
		static UINT16 uac_string_descriptor[] = {
			0x0316,                             // 16: size of String Descriptor = 22 bytes
												// 03: String Descriptor type
			'U', 'A', 'C', '_', 'D',            // UAC_DEVICE
			'E', 'V', 'I', 'C', 'E'
		};
		UVAC_SetConfig(UVAC_CONFIG_UVC_STRING, (UINT32)uvc_string_descriptor);
		UVAC_SetConfig(UVAC_CONFIG_UAC_STRING, (UINT32)uac_string_descriptor);
	}
	#endif
	#if HID_FUNC//sample code for setting HID
	{
		static UINT8 report_desc[] = {
		//##### for CFU  ######
			HID_USAGE_PAGE_16(VENDOR_USAGE_PAGE_CFU_TLC&0xFF, VENDOR_USAGE_PAGE_CFU_TLC>>8),
			HID_USAGE_16(VENDOR_USAGE_CFU&0xFF, VENDOR_USAGE_CFU>>8),
			HID_COLLECTION(APPLICATION),
				HID_REPORT_ID(CFU_GET_REATURE_REPORT_ID),
				HID_LOGICAL_MIN_8(0x00),
				HID_LOGICAL_MAX_8(0x7F),
				HID_REPORT_SIZE(8),
				HID_REPORT_COUNT(sizeof(CFU_FW_VERSION)),
				HID_USAGE_8(VERSION_FEATURE_USAGE),
				HID_FEATURE_16(Data_Var_Abs, BuffBytes),

				HID_REPORT_ID(CFU_UPDATE_CONTENT_REPORT_ID),
				HID_REPORT_SIZE(8),
				HID_REPORT_COUNT(sizeof(CFU_UPDATE_CONTENT)),
				HID_USAGE_8(CONTENT_OUTPUT_USAGE),
				HID_OUTPUT_16(Data_Var_Abs, BuffBytes),
				HID_LOGICAL_MIN_32(0x00, 0x00, 0x00, 0x80),
				HID_LOGICAL_MAX_32(0xFF, 0xFF, 0xFF, 0x7F),
				HID_REPORT_SIZE(32),
				HID_REPORT_COUNT(4),
				HID_USAGE_MIN_8(0x66),
				HID_USAGE_MAX_8(0x69),
				HID_USAGE_8(CONTENT_RESPONSE_INPUT_USAGE),
				HID_INPUT_8(Data_Var_Abs),

				HID_REPORT_ID(CFU_UPDATE_OFFER_REPORT_ID),
				HID_USAGE_MIN_8(0x8E),
				HID_USAGE_MAX_8(0x91),
				HID_USAGE_8(OFFER_OUTPUT_USAGE),
				HID_OUTPUT_8(Data_Var_Abs),
				HID_USAGE_MIN_8(0x8A),
				HID_USAGE_MAX_8(0x8D),
				HID_USAGE_8(OFFER_RESPONSE_INPUT_USAGE),
				HID_INPUT_8(Data_Var_Abs),
			HID_END_COLLECTION(),
		//##### for camera control  ######
			HID_USAGE_PAGE_16(VENDOR_USAGE_PAGE_VC_TLC&0xFF, VENDOR_USAGE_PAGE_VC_TLC>>8),
			HID_USAGE_16(VENDOR_USAGE_VC&0xFF, VENDOR_USAGE_VC>>8),
			HID_COLLECTION(APPLICATION),
				HID_REPORT_SIZE(8),
				HID_REPORT_COUNT(VC_MAX_SIZE),//max 63 exclude report id
				HID_LOGICAL_MIN_8(0x00),
				HID_LOGICAL_MAX_8(0x7F),

				HID_USAGE_8(VC_REQUEST_FEATURE_USAGE),
				HID_REPORT_ID(VC_REQUEST_REPORT_ID),
				HID_FEATURE_16(Data_Var_Abs, BuffBytes),

				HID_USAGE_8(VC_RESPONSE_INPUT_USAGE),
				HID_REPORT_ID(VC_RESPONSE_REPORT_ID),
				HID_INPUT_8(Data_Var_Abs),
			HID_END_COLLECTION(),
		//##### for Telemetry  ######
			HID_USAGE_PAGE_16(TELEMETRY_USAGE_PAGE_TLC&0xFF, TELEMETRY_USAGE_PAGE_TLC>>8),
			HID_USAGE_8(TELEMETRY_USAGE),
			HID_COLLECTION(APPLICATION),
				HID_REPORT_ID(TELEMETRY_CLIENT_REPORT_ID),
				HID_REPORT_SIZE(8),
				HID_REPORT_COUNT(0x3F),
				HID_LOGICAL_MIN_8(0x00),
				HID_LOGICAL_MAX_8(0x7F),
				HID_USAGE_8(3),
				HID_FEATURE_8(Data_Var_Abs),

				HID_REPORT_ID(TELEMETRY_DEVICE_REPORT_ID),
				HID_REPORT_SIZE(1),
				HID_REPORT_COUNT(1),
				HID_LOGICAL_MIN_8(0x0),
				HID_LOGICAL_MAX_8(0x1),
				HID_USAGE_8(4),
				HID_INPUT_8(Data_Var_Rel),
				HID_REPORT_COUNT(7),
				HID_INPUT_8(Const_Arr_Abs),
			HID_END_COLLECTION()
		};
		UVAC_HID_INFO hid_info = {0};

		hid_info.en = TRUE;
		hid_info.cb = m_UvacHidCB;
		hid_info.hid_desc.bLength = 9;
		hid_info.hid_desc.bHidDescType = 0x21;
		hid_info.hid_desc.bcdHID = 0x0111;
		hid_info.hid_desc.bCountryCode = 0;
		hid_info.hid_desc.bNumDescriptors = 1;
		hid_info.hid_desc.bDescriptorType = 0x22;//Report Descriptor
		hid_info.hid_desc.wDescriptorLength = sizeof(report_desc);
		hid_info.p_report_desc = report_desc;
		UVAC_SetConfig(UVAC_CONFIG_HID_INFO, (UINT32)&hid_info);
	}
	#endif

	//must set to 1I14P ratio
	UvacInfo.strmInfo.strmWidth = MAX_BS_W;
	UvacInfo.strmInfo.strmHeight = MAX_BS_H;
	UvacInfo.strmInfo.strmFps = MAX_BS_FPS;
	//UvacInfo.strmInfo.strmCodec = MEDIAREC_ENC_H264;
	//w=1280,720,30,2
	UVAC_ConfigVidReso(gUIUvacVidReso, NVT_UI_UVAC_RESO_CNT);
	//set default frame index, starting from 1
	//UVAC_SetConfig(UVAC_CONFIG_MJPG_DEF_FRM_IDX, 1);
	//UVAC_SetConfig(UVAC_CONFIG_H264_DEF_FRM_IDX, 1);

	#if UVC_SUPPORT_YUV_FMT
	{
		UVAC_VID_RESO_ARY uvacVidYuvResoAry = {0};

		uvacVidYuvResoAry.aryCnt = sizeof(gUIUvacVidYuvReso)/sizeof(UVAC_VID_RESO);
		uvacVidYuvResoAry.pVidResAry = &gUIUvacVidYuvReso[0];
		//set default frame index, starting from 1
		//uvacVidYuvResoAry.bDefaultFrameIndex = 1;
		UVAC_SetConfig(UVAC_CONFIG_YUV_FRM_INFO, (UINT32)&uvacVidYuvResoAry);
	}
	#endif
	uvacAudSampleRateAry.aryCnt = NVT_UI_UVAC_AUD_SAMPLERATE_CNT;
	uvacAudSampleRateAry.pAudSampleRateAry = &gUIUvacAudSampleRate[0];
	UVAC_SetConfig(UVAC_CONFIG_AUD_SAMPLERATE, (UINT32)&uvacAudSampleRateAry);
	UvacInfo.channel = 1;
	//printf("w=%d,h=%d,fps=%d,codec=%d\r\n", UvacInfo.strmInfo.strmWidth, UvacInfo.strmInfo.strmHeight, UvacInfo.strmInfo.strmFps, UvacInfo.strmInfo.strmCodec);
	//UVAC_SetConfig(UVAC_CONFIG_AUD_CHANNEL_NUM, au_cap.sound_mode);
	UVAC_SetConfig(UVAC_CONFIG_VIDEO_FORMAT_TYPE, bVidFmtType);

	//Extension Unit
	#if 1
	UVAC_SetConfig(UVAC_CONFIG_XU_CTRL, (UINT32)xUvacEU_CB);
	#else
	//deprecated, backward compatible
	UVAC_SetConfig(UVAC_CONFIG_EU_VENDCMDCB_ID01, (UINT32)xUvacEUVendCmdCB_Idx1);
	UVAC_SetConfig(UVAC_CONFIG_EU_VENDCMDCB_ID02, (UINT32)xUvacEUVendCmdCB_Idx2);
	UVAC_SetConfig(UVAC_CONFIG_EU_VENDCMDCB_ID03, (UINT32)xUvacEUVendCmdCB_Idx3);
	UVAC_SetConfig(UVAC_CONFIG_EU_VENDCMDCB_ID04, (UINT32)xUvacEUVendCmdCB_Idx4);
	#endif

	#if CT_PU_FUNC
	//CT & PU
	UVAC_SetConfig(UVAC_CONFIG_CT_CONTROLS, uvc_ct_controls);
	UVAC_SetConfig(UVAC_CONFIG_PU_CONTROLS, uvc_pu_controls);
	UVAC_SetConfig(UVAC_CONFIG_CT_CB, (UINT32)xUvacCT_CB);
	UVAC_SetConfig(UVAC_CONFIG_PU_CB, (UINT32)xUvacPU_CB);
	#endif

#if (UVC_MSOS20_FUNC == ENABLE)
	{
		UVAC_MSOS20_INFO msos_info = {0};

		msos_info.en = TRUE;
		msos_info.p_msos20_desc = USBVideoMSOS20DescriptorSet;
		msos_info.msos20_desc_size = sizeof(USBVideoMSOS20DescriptorSet);
		UVAC_SetConfig(UVAC_CONFIG_MSOS20_INFO, (UINT32)&msos_info);
	}
#endif

	#if UVAC_SUSPEND_FUNC
	//register call back for suspend
	//clk_set_resume_callback(clk_resume_cb);
	UVAC_SetConfig(UVAC_CONFIG_POWERDOWN_SUSPEND_CB, (UINT32)UVACSuspendCB);
	#endif

	#if 0	//sample code for setting UAC wMaxPacketSize and bInterval
	UVAC_SetConfig(UVAC_CONFIG_UAC_PACKETSIZE, 68);
	UVAC_SetConfig(UVAC_CONFIG_UAC_INTERVAL, 4);
	#endif

	vol_info.vol_def = 0x600; //6 dB
	vol_info.vol_max = 0xD80; //13.5 dB
	vol_info.vol_min = 0x180; //1.5 dB
	vol_info.vol_res = 0x180; //1.5 dB
	UVAC_SetConfig(UVAC_CONFIG_UAC_VOL_INFO, (UINT32)&vol_info);

	retV = UVAC_Open(&UvacInfo);
	if (retV != E_OK) {
		printf("Error open UVAC task(%u)\r\n", retV);
	}

	return TRUE;
}

static BOOL UVAC_disable(void)
{
	HD_RESULT    ret = HD_OK;

	#if (MSDC_FUNC == 1)
	{
		p_msdc_object->Close();

		if(msdc_blk) {
			hd_common_mem_release_block(msdc_blk);
		}
		if(user_va) {
			if(hd_common_mem_munmap((void *)user_va, POOL_SIZE_USER_DEFINIED)) {
				printf("fail to unmap va(%x)\n", (int)user_va);
			}
		}
	}
	#elif (MSDC_FUNC == 2)
	{
		nvt_cmdsys_runcmd("msdcnvt close");
	}
	#endif

	ret = hd_common_mem_free(uvac_pa, &uvac_va);
	if (HD_OK == ret) {
		printf("Error handle test for free an invalid address: fail %d\r\n", (int)(ret));
	}
	UVAC_Close();

	return TRUE;
}
#endif

static ER apply_isp_config(UINT32 sensor_id){
	ER ret = E_OK;
	INT32 node_ofst, data_len;
	char isp_config_path[32];
	const void *pfdt_node, *p_fdt_app;
	SENSOR_PATH_INFO path_info = {0};

	// ae_path
	path_info.path[0] = 0;	//reset path
	snprintf(isp_config_path, sizeof(isp_config_path)-1, "/isp/sensor@%d", sensor_id);
	p_fdt_app = fdt_get_app();
	if (p_fdt_app && ((node_ofst = fdt_path_offset(p_fdt_app, isp_config_path)) > 0)) {
		pfdt_node = fdt_getprop(p_fdt_app, node_ofst, "ae_path", (int *)&data_len);
		if (pfdt_node != NULL) {
			path_info.id = 0;
			strncpy(path_info.path, (char *)pfdt_node, 31);
			path_info.addr = (UINT32)p_fdt_app;
			DBG_DUMP("ae_path %d=%s, app_addr =%x\r\n", path_info.id, path_info.path, path_info.addr);
		}
	}
	AET_DTSI_INFO ae_dtsi_info;
	ae_dtsi_info.id = 0;
	strncpy(ae_dtsi_info.node_path, path_info.path, 31);
	strncpy(ae_dtsi_info.file_path, "null", DTSI_NAME_LENGTH);
	ae_dtsi_info.buf_addr = (UINT8 *)path_info.addr;
	if ((vendor_isp_set_ae(AET_ITEM_RLD_DTSI, &ae_dtsi_info)) != HD_OK) {
		return E_SYS;
	}

	// awb_path
	path_info.path[0] = 0;	//reset path
	snprintf(isp_config_path, sizeof(isp_config_path)-1, "/isp/sensor@%d", sensor_id);
	p_fdt_app = fdt_get_app();
	if (p_fdt_app && ((node_ofst = fdt_path_offset(p_fdt_app, isp_config_path)) > 0)) {
		pfdt_node = fdt_getprop(p_fdt_app, node_ofst, "awb_path", (int *)&data_len);
		if (pfdt_node != NULL) {
			path_info.id = 0;
			strncpy(path_info.path, (char *)pfdt_node, 31);
			path_info.addr = (UINT32)p_fdt_app;
			DBG_DUMP("awb_path %d=%s, app_addr =%x\r\n", path_info.id, path_info.path, path_info.addr);
		}
	}
	AWBT_DTSI_INFO awb_dtsi_info;
	awb_dtsi_info.id = 0;
	strncpy(awb_dtsi_info.node_path, path_info.path, 31);
	strncpy(awb_dtsi_info.file_path, "null", DTSI_NAME_LENGTH);
	awb_dtsi_info.buf_addr = (UINT8 *)path_info.addr;
	if ((vendor_isp_set_awb(AWBT_ITEM_RLD_DTSI, &awb_dtsi_info)) != HD_OK) {
		return E_SYS;
	}

	// iq_path
	path_info.path[0] = 0;	//reset path
	snprintf(isp_config_path, sizeof(isp_config_path)-1, "/isp/sensor@%d", sensor_id);
	p_fdt_app = fdt_get_app();
	if (p_fdt_app && ((node_ofst = fdt_path_offset(p_fdt_app, isp_config_path)) > 0)) {
		pfdt_node = fdt_getprop(p_fdt_app, node_ofst, "iq_path", (int *)&data_len);
		if (pfdt_node != NULL) {
			path_info.id = 0;
			strncpy(path_info.path, (char *)pfdt_node, 31);
			path_info.addr = (UINT32)p_fdt_app;
			DBG_DUMP("iq_path %d=%s, app_addr =%x\r\n", path_info.id, path_info.path, path_info.addr);
		}
	}
	IQT_DTSI_INFO iq_dtsi_info;
	iq_dtsi_info.id = 0;
	strncpy(iq_dtsi_info.node_path, path_info.path, 31);
	strncpy(iq_dtsi_info.file_path, "null", DTSI_NAME_LENGTH);
	iq_dtsi_info.buf_addr = (UINT8 *)path_info.addr;
	if ((vendor_isp_set_iq(IQT_ITEM_RLD_DTSI, &iq_dtsi_info)) != HD_OK) {
		return E_SYS;
	}

	return ret;
}
#if UVC_SHUTTER
static int set_shutter_param(HD_PATH_ID shutter_path, int en)
{
	HD_OSG_MASK_ATTR attr;

	printf("set_shutter_param %d\r\n", en);

	memset(&attr, 0, sizeof(HD_OSG_MASK_ATTR));

	attr.position[0].x = 0;
	attr.position[0].y = 0;
	attr.position[1].x = cap_size_w;
	attr.position[1].y = 0;
	attr.position[2].x = cap_size_w;
	attr.position[2].y = cap_size_h;
	attr.position[3].x = 0;
	attr.position[3].y = cap_size_h;
	attr.type          = HD_OSG_MASK_TYPE_SOLID;
	attr.alpha         = en;
	attr.color         = 0x00808000;

	return hd_videoproc_set(shutter_path, HD_VIDEOPROC_PARAM_IN_MASK_ATTR, &attr);
}

void shutter_GpioIsr(UINT32 uiEvent){
	if (uvc_mte_info.shutter_dir)
		shutterFlag = !gpio_getPin(UVC_SHUTTER_GPIO);
	else
		shutterFlag = gpio_getPin(UVC_SHUTTER_GPIO);

	if (uvc_custom_info.check_shutter)
		shutterFlag = 0;

	set_shutter_param(stream[0].vp_shutter_path, 255);
	//_TO_DO_
	//update status to interrupt endpoint
	//PrivCtrlUpdate_setEvt(PRIVCTRL_EVT_PRIVACY_ENABLE);
}
#endif
static void app_main(int argc, char* argv[])
{
	HD_RESULT ret;
	//INT key;

	UINT32 enc_type = HD_CODEC_TYPE_H264;
	HD_DIM main_dim;

	stream[0].codec_type = enc_type;
	// query program options
	if (argc > 1) {
		sensor_sel = atoi(argv[1]);
		printf("sensor_sel %d\r\n", sensor_sel);
	}
	if (sensor_sel == SEN_SEL_IMX290) {
		cap_size_w = VDO_SIZE_W_2M;
		cap_size_h = VDO_SIZE_H_2M;
	} else if (sensor_sel == SEN_SEL_OS05A10) {
		cap_size_w = VDO_SIZE_W_5M;
		cap_size_h = VDO_SIZE_H_5M;
	} else if (sensor_sel == SEN_SEL_GC4653) {
		cap_size_w = VDO_SIZE_W_4M;
		cap_size_h = VDO_SIZE_H_4M;
	} else {
		printf("Not support sensor_sel %d\r\n", sensor_sel);
		sensor_sel = SEN_SEL_IMX290;
		cap_size_w = VDO_SIZE_W_2M;
		cap_size_h = VDO_SIZE_H_2M;
	}
	gUIUvacVidReso[0].width = cap_size_w;
	gUIUvacVidReso[0].height = cap_size_h;

	//AET_CFG_INFO cfg_info = {0};
	if (vendor_isp_init() != HD_ERR_NG) {
		//apply isp config
		if (apply_isp_config(0) != E_OK){
			printf("apply isp config failed!\n");
		}

		

#if 0
		{
			DBG_DUMP("sleep for FileSys init\r\n");
			sleep(1);
			char file_path_main[80] = "/mnt/sd/hd_gfx_color_transform_320x240.yuyv";
			FILE *f_out_sub;

			//----- open output files -----
			if ((f_out_sub = fopen(file_path_main, "rb")) == NULL) {
				HD_VIDEOENC_ERR("open file (%s) fail....\r\n", file_path_main);
			} else {
				printf("\r\ndump main bitstream to file (%s) ....\r\n", file_path_main);

				setvbuf(f_out_sub, NULL, _IONBF, 0);
			}
	
			printf("=========== read ++\r\n");
			if (f_out_sub) {
				fread((void *)yuv_buf, 1, 153600, f_out_sub);
				printf("=========== read\r\n");			
			}

			printf("=========== read --\r\n");

			if (f_out_sub) fclose(f_out_sub);
		}


#endif
		
#if UVC_DPC_FUNC
		const unsigned int sensor_id = 0;
		IQT_DPC_PARAM *p_dpc_param = (IQT_DPC_PARAM *)malloc(sizeof(IQT_DPC_PARAM));
		UINT8 *dpc_table = (UINT8 *)malloc(IQ_DPC_MAX_NUM * sizeof(UINT32));
		//apply dp.bin
		DBG_DUMP("sleep for FileSys init\r\n");
		sleep(1);
		DBG_DUMP("apply dp.bin for dpc function\r\n");
		if (read_dpc_config(dpc_table) == E_OK){
			p_dpc_param->id = sensor_id;
			vendor_isp_get_iq(IQT_ITEM_DPC_PARAM, p_dpc_param);
			p_dpc_param->dpc.enable = 1;
			memcpy(p_dpc_param->dpc.table, dpc_table, IQ_DPC_MAX_NUM * sizeof(UINT32));
			if (vendor_isp_set_iq(IQT_ITEM_DPC_PARAM, p_dpc_param) < 0){
				printf("enable dpc failed!\n");
			}
		}
		free(p_dpc_param);
		free(dpc_table);
#endif
		vendor_isp_uninit();
	}

	// init hdal
	ret = hd_common_init(0);
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
		goto exit;
	}

	// init memory
#if (MSDC_FUNC == 2)
	mempool_init();
#endif

#if (PST_FUNC)
	check_and_restore_header();
	load_uvc_mte_info();
	dump_uvc_mte_info();
	load_uvc_custom_info();
	dump_uvc_custom_info();
	set_update_onehead();
#endif

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

	// open VIDEO_STREAM modules (main)
	stream[0].proc_max_dim.w = MAX_CAP_SIZE_W; //assign by user
	stream[0].proc_max_dim.h = MAX_CAP_SIZE_H; //assign by user

	ret = open_module(&stream[0], &stream[0].proc_max_dim);
	if (ret != HD_OK) {
		printf("open fail=%d\n", ret);
		goto exit;
	}

#if AUDIO_ON
	//open capture module
	au_cap.sample_rate_max = gUIUvacAudSampleRate[0];
	au_cap.sound_mode = HD_AUDIO_SOUND_MODE_MONO;		//UAC sound mode: HD_AUDIO_SOUND_MODE_STEREO or HD_AUDIO_SOUND_MODE_MONO
	au_cap.volume = 54; // set vol = 6 dB
	ret = open_module2(&au_cap);
	if(ret != HD_OK) {
		printf("open fail=%d\n", ret);
		goto exit;
	}

	//set audiocap parameter
	au_cap.sample_rate = gUIUvacAudSampleRate[0];
	ret = set_cap_param2(au_cap.cap_path, au_cap.sample_rate, au_cap.sound_mode);
	if (ret != HD_OK) {
		printf("set cap fail=%d\n", ret);
		goto exit;
	}
#endif

	// get videocap capability
	ret = get_cap_caps(stream[0].cap_ctrl, &stream[0].cap_syscaps);
	if (ret != HD_OK) {
		printf("get cap-caps fail=%d\n", ret);
		goto exit;
	}

	// set videocap parameter
	stream[0].cap_dim.w = MAX_CAP_SIZE_W; //assign by user
	stream[0].cap_dim.h = MAX_CAP_SIZE_H; //assign by user
	ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim, MAX_BS_FPS, &stream[0].cap_dim);
	if (ret != HD_OK) {
		printf("set cap fail=%d\n", ret);
		goto exit;
	}

	// assign parameter by program options
	main_dim.w = MAX_BS_W;
	main_dim.h = MAX_BS_H;
	// set videoproc parameter (main)
	ret = set_proc_param(stream[0].proc_main_path, &main_dim, HD_VIDEO_PXLFMT_YUV420, 0, 1);
	if (ret != HD_OK) {
		printf("1111set proc fail=%d\n", ret);
		goto exit;
	}
#if UVC_SHUTTER
	if (uvc_mte_info.shutter_dir)
		shutterFlag = !gpio_getPin(UVC_SHUTTER_GPIO);
	else
		shutterFlag = gpio_getPin(UVC_SHUTTER_GPIO);

	if (uvc_custom_info.check_shutter)
		shutterFlag = 0;

	gpio_setIntTypePol(UVC_SHUTTER_GPIO_INT, GPIO_INTTYPE_EDGE, GPIO_INTPOL_BOTHEDGE);
	gpio_setIntIsr(UVC_SHUTTER_GPIO_INT, shutter_GpioIsr);
	gpio_enableInt(UVC_SHUTTER_GPIO_INT);

	//setup shutter mask parameter
	if(set_shutter_param(stream[0].vp_shutter_path, 0)){
		printf("fail to set shutter attr\r\n");
		goto exit;
	}

	//render shutter mask
	ret = hd_videoproc_start(stream[0].vp_shutter_path);
	if (ret != HD_OK) {
		printf("fail to start shutter\n");
		goto exit;
	}
#endif
	// set videoenc config (main), enc_cfg uses H264 to define bitstream buffer
	stream[0].enc_main_max_dim.w = MAX_BS_W;
	stream[0].enc_main_max_dim.h = MAX_BS_H;
	if ((MJPEG_TBR_MAX/(1024*1024))*UVC_MJPEG_ENC_BUF_MS(30) < ((H264_TBR_MAX)/(1024*1024))*UVC_H264_ENC_BUF_MS){	//H264 max bs buffer > MJPEG max bs buffer
		ret = set_enc_cfg(stream[0].enc_main_path, &stream[0].enc_main_max_dim, enc_type, H264_TBR_MAX);
	} else {		//MJPEG max bs buffer > H264 max bs buffer
		ret = set_enc_cfg(stream[0].enc_main_path, &stream[0].enc_main_max_dim, enc_type, (((MJPEG_TBR_MAX/(1024*1024))*UVC_MJPEG_ENC_BUF_MS(MAX_BS_FPS))/UVC_H264_ENC_BUF_MS)*1024*1024);
	}

	if (ret != HD_OK) {
		printf("set enc-cfg fail=%d\n", ret);
		goto exit;
	}

	// set videoenc parameter (main)
	stream[0].enc_main_dim.w = main_dim.w;
	stream[0].enc_main_dim.h = main_dim.h;
	ret = set_enc_param(stream[0].enc_main_path, &stream[0].enc_main_dim, enc_type, H264_TBR_MAX);
	if (ret != HD_OK) {
		printf("set enc fail=%d\n", ret);
		goto exit;
	}
	// bind VIDEO_STREAM modules (main)
	hd_videocap_bind(HD_VIDEOCAP_0_OUT_0, HD_VIDEOPROC_0_IN_0);
	//hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOENC_0_IN_0);

#if AUDIO_ON
	//audio create capture thread
	THREAD_CREATE(cap_thread_id, capture_thread, (void *)&au_cap, "capture_thread");
	if (0 == cap_thread_id) {
		printf("create encode thread failed");
		goto exit;
	}
	THREAD_SET_PRIORITY(cap_thread_id, 4);
	THREAD_RESUME(cap_thread_id);
	//start audio capture module
	//hd_audiocap_start(au_cap.cap_path);
#endif

	// start VIDEO_STREAM modules (main)
	if (g_capbind == 1) {
		//direct NOTE: ensure videocap start after 1st videoproc phy path start
		hd_videoproc_start(stream[0].proc_main_path);
		hd_videocap_start(stream[0].cap_path);
	} else {
		hd_videocap_start(stream[0].cap_path);
		hd_videoproc_start(stream[0].proc_main_path);
	}

	hd_videoproc_start(stream[0].proc_sub_path);

	// just wait ae/awb stable for auto-test, if don't care, user can remove it
	//sleep(1);
	//hd_videoenc_start(stream[0].enc_main_path);
	// create encode_thread (pull_out bitstream)
	#if 1
	THREAD_CREATE(stream[0].enc_thread_id, encode_thread, (void *)stream, "encode_thread");
	if (0 == stream[0].enc_thread_id) {
		printf("create encode thread failed");
		goto exit;
	}
	THREAD_SET_PRIORITY(stream[0].enc_thread_id, 4);
	//THREAD_RESUME(stream[0].enc_thread_id);
	#else
	pthread_attr_t enc_thd_attr;
    struct sched_param enc_thd_param;

    pthread_attr_init(&enc_thd_attr);
    enc_thd_param.sched_priority = 96;
    pthread_attr_setschedpolicy(&enc_thd_attr,SCHED_RR);
    pthread_attr_setschedparam(&enc_thd_attr,&enc_thd_param);
    pthread_attr_setinheritsched(&enc_thd_attr,PTHREAD_EXPLICIT_SCHED);

    ret = pthread_create(&fd_thread_id, &enc_thd_attr, encode_thread, (VOID*)(&enc_thd_param));
    pthread_setname_np(pthread_self(), "encode_thread");
	#endif

#if UVC_SUPPORT_YUV_FMT
	THREAD_CREATE(stream[0].acquire_thread_id, acquire_yuv_thread, (void *)stream, "acquire_yuv_thread");
	if (0 == stream[0].acquire_thread_id) {
		printf("create acquire thread failed");
		goto exit;
	}
	THREAD_RESUME(stream[0].acquire_thread_id);
#endif

	pthread_mutex_lock(&flow_start_lock);
	flow_audio_start = 0;
	encode_start = 0;
	acquire_start = 0;
	pthread_mutex_unlock(&flow_start_lock);

	/*if(Cfu_InstallID()){
		DBG_ERR("fail to install cfu id\r\n");
		goto exit;
	}
	if(cfu_init()){
		DBG_ERR("fail to init cfu\r\n");
		goto exit;
	}
	if(cfu_enable()){
		DBG_ERR("fail to enable cfu\r\n");
		goto exit;
	}*/

#if UVC_ON
	sleep(1);
	UVAC_enable(); //init usb device, start to prepare video/audio buffer to send
#endif

	// query user key
	printf("Enter q to exit\n");
#if 0
	while (1) {
		key = GETCHAR();
		if (key == 'q' || key == 0x3) {
			pthread_mutex_lock(&flow_start_lock);
			flow_audio_start = 0;
			encode_start = 0;
			acquire_start = 0;
			pthread_mutex_unlock(&flow_start_lock);
			// let encode_thread stop loop and exit
			stream[0].enc_exit = 1; //stop video

#if AUDIO_ON
			au_cap.cap_exit = 1; //stop audio
#endif

			// quit program
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
			get_cap_sysinfo(stream[0].cap_ctrl);
		}
		if (key == 'u') {
			uvc_debug_menu_entry_p(uvc_debug_menu, "UVAC DEBUG MAIN");
		}
	}
#else // Reduce CPU loading. 8x% ---> 1x%.
	while(1) {sleep(1);}
#endif
	// destroy encode thread
	THREAD_DESTROY(stream[0].enc_thread_id);
#if UVC_SUPPORT_YUV_FMT
	THREAD_DESTROY(stream[0].acquire_thread_id);
#endif
#if AUDIO_ON
	THREAD_DESTROY(cap_thread_id); // stop audio
#endif

#if UVC_ON
	UVAC_disable();
#endif

#if AUDIO_ON
	//stop audio capture module
	hd_audiocap_stop(au_cap.cap_path);
#endif
	// stop VIDEO_STREAM modules (main)
	if (g_capbind == 1){
		hd_videoproc_stop(stream[0].proc_main_path);
		hd_videocap_stop(stream[0].cap_path);
	} else {
		hd_videocap_stop(stream[0].cap_path);
		hd_videoproc_stop(stream[0].proc_main_path);
	}
	//hd_videoenc_stop(stream[0].enc_main_path);
	// unbind VIDEO_STREAM modules (main)
	hd_videocap_unbind(HD_VIDEOCAP_0_OUT_0);
	//hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_0);

exit:
	// close VIDEO_STREAM modules (main)
	ret = close_module(&stream[0]);
	if (ret != HD_OK) {
		printf("close fail=%d\n", ret);
	}

#if AUDIO_ON
	//close audio module
	ret = close_module2(&au_cap);
	if(ret != HD_OK) {
		printf("close fail=%d\n", ret);
	}
#endif

	// uninit all modules
	ret = exit_module();
	if (ret != HD_OK) {
		printf("exit fail=%d\n", ret);
	}

#if AUDIO_ON
	// uninit all modules
	ret = exit_module2();
	if (ret != HD_OK) {
		printf("exit fail=%d\n", ret);
	}
#endif

    ret = hd_gfx_uninit();
	if (ret != HD_OK) {
		printf("hd_gfx_uninit fail=%d\n", ret);
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

}

int flow_uvc(void)
{
	int argc = 0;
	char * argv[2] = {0};

	fastboot_wait_done(BOOT_INIT_SENSOR);
	fastboot_wait_done(BOOT_INIT_CAPTURE);
	fastboot_wait_done(BOOT_INIT_MEIDA_ENCODER);
	fastboot_wait_done(BOOT_INIT_CAPTURE);

	pthread_mutex_init(&flow_start_lock, NULL);

	app_main(argc, argv);

	pthread_mutex_destroy(&flow_start_lock);

	return 0;
}

MAINFUNC_ENTRY(uvc_boot_time, argc, argv)
{
	vos_perf_list_dump();
	return 0;
}
