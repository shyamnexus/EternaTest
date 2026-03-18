#include "vendor_videocapture.h"
#include "vendor_isp.h"
#include "vendor_common.h"
#include "vendor_videoenc.h"
#include "aov1_common.h"
#include "aov1_ctrl.h"
#include "aov1_filesave.h"

#define SEN_VCAP_ID_0      0
#define SEN_VCAP_ID_1	   2
#define MD_SUPPORT		0
#define NOT_ENCODE		0 //test
#define NO_VIDEO        0 //test
#define ENC_TYPE 1 // 0:H265 1:H264 2:JPEG
#define END_COUNT 150
#define OSG_SUPPORT 0

#define PVD_IMG_WIDTH			1024
#define PVD_IMG_HEIGHT          576
#define PVD_IMG_BUF_SIZE        (PVD_IMG_WIDTH * PVD_IMG_HEIGHT)
UINT32 check_t1,check_t2,check_t3,check_t4,check_t5;
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
#define VDO_SIZE_W_2M      1920
#define VDO_SIZE_H_2M      1080
//sensor selection list
typedef enum _SEN_SEL {
	SEN_SEL_SC450AI                =  0,
	SEN_SEL_SC200AI                =  1,
	SEN_SEL_SC835HAI               =  2,
	SEN_SEL_OS04C10                =  3,
	SEN_SEL_OS04E10                =  4,
	SEN_SEL_IMX678				   =  5,
	SEN_SEL_IMX415_DUAL			   =  6,
	SEN_SEL_MAX,
	ENUM_DUMMY4WORD(SEN_SEL)
} SEN_SEL;

typedef enum _SEN_STATUS {
	SEN_STATUS_ASLEEP               =  0,
	SEN_STATUS_AWAKE                =  1,
	SEN_STATUS_MAX,
	ENUM_DUMMY4WORD(SEN_STATUS)
} SEN_STATUS;

typedef struct _SENSOR_INFO_ {
	UINT32 id;
	CHAR name[32];
	UINT32 default_fps;
	USIZE size;
	CHAR isp_cfg_name[32];
	UINT32 len;
} SENSOR_INFO;

SENSOR_INFO sensor_info[] = {
	{
		SEN_SEL_SC450AI,    					// ID 0
		"sc450ai",								// Name
		60,										// default_fps
		{ VDO_SIZE_W_4M2, VDO_SIZE_H_4M2 },		// Size
		"isp_sc450ai_0",						// ISP cfg
		4										// Lens
	},
	{
		SEN_SEL_SC200AI,    					// ID 1
		"sc200ai",								// Name
		60,										// default_fps
		{ VDO_SIZE_W_2M, VDO_SIZE_H_2M },		// Size
		"isp_sc450ai_0",						// ISP cfg
		2										// Lens
	},
	{
		SEN_SEL_SC835HAI,    					// ID 2
		"sc835hai",								// Name
		30,										// default_fps
		{ VDO_SIZE_W_8M, VDO_SIZE_H_8M },		// Size
		"isp_sc450ai_0",						// ISP cfg
		4										// Lens
	},
	{
		SEN_SEL_OS04C10,    					// ID 3
		"os04c10",								// Name
		60,										// default_fps
		{ VDO_SIZE_W_4M2, VDO_SIZE_H_4M2 },		// Size

		"isp_os04c10_0",						// ISP cfg
		2										// Lens
	},
	{
		SEN_SEL_OS04E10,    					// ID 4
		"os04e10",								// Name
		30,										// default_fps
		{ 2048, 2048 },							// Size
		"isp_os04c10_0",						// ISP cfg
		2										// Lens
	},
	{
		SEN_SEL_IMX678,    						// ID 5
		"imx678",								// Name
		30,										// default_fps
		{ VDO_SIZE_W_8M, VDO_SIZE_H_8M },		// Size
		"isp_sc450ai_0",						// ISP cfg
		4										// Lens
	},
	{
		SEN_SEL_IMX415_DUAL,					// ID 6
		"imx415",								// Name
		24,										// default_fps
		{VDO_SIZE_W_8M, VDO_SIZE_H_8M},			// Size
		"isp_imx415_0",							// ISP cfg
		4										// Lens
	},
};
UINT32 SEL_SEN = 0;
UINT32 AE_SKIP_FRAME_NUM = 24;
BOOL SEN_SUPPORT_STREAMOUT_SLEEP = FALSE;
///////////////////////////////////////////////////////////////////////////////

#define SEN_OUT_FMT		HD_VIDEO_PXLFMT_RAW12
#define CAP_OUT_FMT		HD_VIDEO_PXLFMT_RAW12
#define CA_WIN_NUM_W		32
#define CA_WIN_NUM_H		32
#define LA_WIN_NUM_W		32
#define LA_WIN_NUM_H		32
#define VA_WIN_NUM_W		16
#define VA_WIN_NUM_H		16

/////////////////////////////////////////////
#define AWB_SKIP_FRAME 5  // set awb skip frame
#define VENDOR_AI_CFG 					0x000f0000  //ai

#define HD_VIDEOPROC_CFG				0x000f0000	//vprc
#define HD_VIDEOPROC_CFG_STRIP_MASK		0x00000007  //vprc stripe rule mask: (default 0)
#define HD_VIDEOPROC_CFG_STRIP_LV1		0x00000000  //vprc "0: cut w>1280, GDC =  on, 2D_LUT off after cut (LL slow)
#define HD_VIDEOPROC_CFG_STRIP_LV2		0x00010000  //vprc "1: cut w>2048, GDC = off, 2D_LUT off after cut (LL fast)
#define HD_VIDEOPROC_CFG_STRIP_LV3		0x00020000  //vprc "2: cut w>2688, GDC = off, 2D_LUT off after cut (LL middle)(2D_LUT best)
#define HD_VIDEOPROC_CFG_STRIP_LV4		0x00030000  //vprc "3: cut w> 720, GDC =  on, 2D_LUT off after cut (LL not allow)(GDC best)
#define HD_VIDEOPROC_CFG_DISABLE_GDC	HD_VIDEOPROC_CFG_STRIP_LV2
#define HD_VIDEOPROC_CFG_LL_FAST		HD_VIDEOPROC_CFG_STRIP_LV2
#define HD_VIDEOPROC_CFG_2DLUT_BEST		HD_VIDEOPROC_CFG_STRIP_LV3
#define HD_VIDEOPROC_CFG_GDC_BEST		HD_VIDEOPROC_CFG_STRIP_LV4

///////////////////////////////////////////////////////////////////////////////
//-------------------------------------
//#define MASK_NUMBER 2
typedef struct _GFX_MASK_INFO {
        int x, y, w, h;
		UINT32 color;
		UINT32 alpha;
		HD_OSG_MASK_TYPE type;
		int thick;
} GFX_MASK_INFO;
GFX_MASK_INFO mask[MASK_NUMBER] = {
	{0,    0,   50, 50, 0xFFFF0000, 0x7F, HD_OSG_MASK_TYPE_SOLID, 0},	// Red,   PVD
	{100,  0,   50, 50,   0xFFFFFF00, 0x7F, HD_OSG_MASK_TYPE_SOLID, 0}	// Yellow, MD
};
//-----------------------------------------
//header
#define DBGINFO_BUFSIZE()	(0x200)

//RAW
#define VDO_RAW_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
//NRX: RAW compress: Only support 12bit mode
#define RAW_COMPRESS_RATIO 59
#define VDO_NRX_BUFSIZE(w, h)           (ALIGN_CEIL_4(ALIGN_CEIL_64(w) * 12 / 8 * RAW_COMPRESS_RATIO / 100 * (h)))
//CA for AWB
#define VDO_CA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 3) << 1)
//LA for AE
#define VDO_LA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 1) << 1)

//YUV
#define VDO_YUV_BUFSIZE(w, h, pxlfmt)	(ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
//NVX: YUV compress
#define YUV_COMPRESS_RATIO 75
#define VDO_NVX_BUFSIZE(w, h, pxlfmt)	(VDO_YUV_BUFSIZE(w, h, pxlfmt) * YUV_COMPRESS_RATIO / 100)

int APP_MODE= 0;
int LD_DDR_RATE= 0;
int EN_BTN_SAVE= 0;
int EN_ENC_SKIP_FRAME= 0;
int skip_ratio = 1;
UINT32 LOWPOWER_FPS = 1;
UINT32 NORMAL_FPS = 12;
static VIDEO_RECORD stream[SENSOR_MAX_NUM] = {0}; //0: main stream
HD_VIDEO_FRAME video_frame_Main[SENSOR_MAX_NUM] = {0};
UINT32 g_sensor_num;
static UINT32 SEN_VCAP_ID[SENSOR_MAX_NUM] = {0};
static BOOL SENSOR_STATUS = SEN_STATUS_ASLEEP;
BOOL SYS_STATUS = NORMAL_MODE;
UINT32 BITRATE = (2*1024*1024);
UINT32 ENC_BUFF_SIZE = (4*1024*1024);
UINT32 EN_AUTO_SAVE = 1;
// parameter for BUTTON_SAVE
UINT32 venc_save_ram=1;
unsigned int first_skip = 1;
UINT32	change_fps = 0;
UINT32	CURRENT_FPS = 0;
int do_venc_flag = 0;
unsigned int wait_ai_thd_done = 0;
unsigned int wait_enc_thd_done = 0;
unsigned int wait_filesave_done = 0;
extern unsigned int wait_md_thd_done;


/////////////////////////////////////////////
#if MD_SUPPORT
extern UINT32 MD_result;
extern void *md_thread_api(VOID *arg);
pthread_t md_thread_id;
#endif
// MD
#define MD_HEAD_BUFSIZE()	            (0x40)
#define IMG_WIDTH           320
#define IMG_HEIGHT          180
#define IMG_BUF_SIZE        (IMG_WIDTH * IMG_HEIGHT)
// Note , the md info w, h is vprc input w, h not out w, h
#define MD_INFO_BUFSIZE(w, h)           (ALIGN_CEIL_64((((w + 511) >> 9) << 2) * ((h + 15) >> 4)))

UINT32 md_threshold = 5;
/* Low Power setting */
static UINT32 g_svc = HD_SVC_DISABLE;//HD_SVC_2X; // HD_SVC_DISABLE/HD_SVC_2X/HD_SVC_4X
static UINT32 g_md_lpm = 1; // Set Motion Bitmap for enable H26x Low Power Mode
////////////////////////////// For AI PVD ///////////////////////////////////////////
#include "ai3_pvd.h"
#define PVD_THRESHOLD					60		// 12 fps keep times, per second 




extern HD_VIDEO_FRAME video_frame_PVD[SENSOR_MAX_NUM];
extern CHAR combin_model_name[256];
extern pthread_t nn_thread_id;
extern PVD_THREAD_MEM pvd_thread_parms;
extern UINT32 ai_thread_flow_start;
extern BOOL exit_ai_thread;
extern BOOL start_ai_thread;
extern void *nn_thread_api(VOID *arg);
HD_PATH_ID ai_thread_pvd_path[SENSOR_MAX_NUM];
///////////////////////////////////////////////////////////////////////////////
static BOOL exit_encode_thread;
static BOOL exit_filesave_thread;
static UINT32 g_shdr = 0; //fixed
static UINT32 g_capbind = 0;  //0:D2D, 1:direct, 2: one-buf, 0xff: no-bind
static UINT32 g_capfmt = 0; //0:RAW, 1:RAW-compress
static UINT32 g_prcbind = 0xff;  //0:D2D, 1:lowlatency, 2: one-buf, 0xff: no-bind
static UINT32 g_prcfmt = 0; //0:YUV, 1:YUV-compress
static UINT32 g_prccfg = HD_VIDEOPROC_CFG_STRIP_LV1; //0(STRP_LV1), 1(STRP_LV2), 2(STRP_LV3), 3(GDC_OFF), 4(AI_CNN_ON)
static UINT32 g_3dnr = 1;		// 1:Enable 3dnr
static UINT32 g_advPushRatio = 0;		// Enable adv push and set ratio , 0:Disable adv push
///////////////////////////////////////////////////////////////////////////////

static HD_RESULT stop_module(void);
static HD_RESULT start_module(void);
static int osg_blink(HD_PATH_ID mask_path, int en);
///////////////////////////////////////////////////////////////////////////////

int check_exist(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (file) {
        fclose(file);
        return 1; // exist
    } else {
        return 0; // not exist
    }
}

static inline void sensor_sleep(UINT32 isp_id)
{
	if(SENSOR_STATUS == SEN_STATUS_AWAKE)
	{
		//printf("[%s]\r\n", __func__);
		vendor_isp_set_common(ISPT_ITEM_SENSOR_SLEEP, &isp_id);
	}
	//else
	//	printf("[%s] SKIP\r\n", __func__);
}

static inline void sensor_wakeup(UINT32 isp_id)
{
	if(SENSOR_STATUS == SEN_STATUS_ASLEEP)
	{
		//printf("[%s]\r\n", __func__);
		vendor_isp_set_common(ISPT_ITEM_SENSOR_WAKEUP, &isp_id);
	}
	//else
	//	printf("[%s] SKIP\r\n", __func__);
}

static inline void wait_frmend(UINT32 id)
{
	ISPT_WAIT_FRMEND wait_fd = {0};
	wait_fd.id = id;
	wait_fd.timeout = 100;
	vendor_isp_get_common(ISPT_ITEM_WAIT_FRMEND, &wait_fd);
}

static void camera_flash(int gpio_pin, int light_time)
{
	pwm_en(1,1);
	pwm_en(1,0);
}

static HD_RESULT set_frc(UINT32 frc)//change to vcap frc
{
	HD_RESULT ret = HD_OK;
	if( CURRENT_FPS == frc)
		return ret;
	else
	{
		UINT32 fps;
		
		printf("\r\n\n\n\n\033[1;37;41m  Set Frc to %ld fps   \033[0m\r\n\n\n", frc);
			fps = HD_VIDEO_FRC_RATIO(frc,1);
		ret = vendor_videocap_set(stream[0].cap_ctrl, VENDOR_VIDEOCAP_PARAM_SET_FPS, &fps);
		if (ret != HD_OK) {
			printf("[WRN] VENDOR_VIDEOCAP_PARAM_SET_FPS failed!(%d)\r\n", ret);
		}

	}
	change_fps = frc;
	CURRENT_FPS = frc;
	return ret;
}

static UINT32 check_ae_state(AET_STATUS_INFO *stat) {
	AET_STATUS_INFO *status = (AET_STATUS_INFO *)stat;
	UINT32 ret = 0;
	vendor_isp_get_ae(AET_ITEM_STATUS, status);

	if(status->status_info.lum == 0 && status->status_info.state_adj == 0)
		ret = 1;	// Sensor first frame
	else if (status->status_info.state_adj == AE_STATUS_COARSE)
		ret = 1;	// AE unstable
	else if (status->status_info.state_adj == AE_STATUS_LIMIT)
		ret = -1;	// AE over limit
	else
		ret = 0;

	return ret;
}

static int skip_frame(int skip_num)
{
	AET_STATUS_INFO status = {0};
	HD_VIDEO_FRAME video_frame = {0};
	int skip = 0;
	UINT32 ret = 0;
	UINT32 flag_clean_queue = 0;
	status.id = SEN_VCAP_ID_0;
	if( skip_num == -1)
	{
		flag_clean_queue = 1;
		skip_num = 5;
	}
	while(skip < skip_num) 
	{
		skip++;
		ret = hd_videoproc_pull_out_buf(stream[0].proc_path, &video_frame, 100);
		if (ret != HD_OK) 
		{
			if(flag_clean_queue == 1)
			{
				printf("%s: Pull None, queue cleaned\r\n", __func__);
				break;
			}
			else
			{
				printf("[WRN] %s: Pull fail (%d)\r\n", __func__, ret);
				continue;
			}
			
		}
		#if MD_SUPPORT
		if(APP_MODE>=1)
			wait_md_thd_done=1;		// Trigger MD, for MD stable quickly
		#endif
		check_ae_state(&status);
		printf("lum=%d , ae_state=%d \r\n", status.status_info.lum, status.status_info.state_adj);
		hd_videoproc_release_out_buf(stream[0].proc_path, &video_frame);
		
	}

	printf("\r\nSkip: %d\r\n", skip);
	return 0;
}


static void quick_ae(void) {
	AET_STATUS_INFO status = {0};
	HD_RESULT ret;
	UINT32 i;

	for (i = 0; i < g_sensor_num; i++) {
		status.id = SEN_VCAP_ID[i];
		ret = check_ae_state(&status);
		if(ret == 1)
		{
			if(SEN_SUPPORT_STREAMOUT_SLEEP)
			{
				sensor_wakeup(SEN_VCAP_ID[i]);
				SENSOR_STATUS = SEN_STATUS_AWAKE;
			}
			printf("AE unstable, skip_frame\r\n");
			skip_frame(AE_SKIP_FRAME_NUM);
			printf("AE stable\r\n");
			if(SEN_SUPPORT_STREAMOUT_SLEEP)
			{
				sensor_sleep(SEN_VCAP_ID[i]);
				SENSOR_STATUS = SEN_STATUS_ASLEEP;
			}
		}
		else if(ret == -1)
			printf("AE_STATUS_LIMIT, do nothing\r\n");
	}
}



static HD_RESULT mem_init(void)
{
	HD_RESULT              ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
	UINT32 ai_b_size;

	// config common pool (cap)
	mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[0].blk_size = DBGINFO_BUFSIZE()+VDO_RAW_BUFSIZE(sensor_info[SEL_SEN].size.w, sensor_info[SEL_SEN].size.h, CAP_OUT_FMT)
														+VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
														+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
	mem_cfg.pool_info[0].blk_cnt = 6;
	mem_cfg.pool_info[0].ddr_id = DDR_ID;
	// config common pool (main)
	mem_cfg.pool_info[1].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[1].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(sensor_info[SEL_SEN].size.w, sensor_info[SEL_SEN].size.h, HD_VIDEO_PXLFMT_YUV420)+MD_HEAD_BUFSIZE()
                                                     +MD_INFO_BUFSIZE(sensor_info[SEL_SEN].size.w, sensor_info[SEL_SEN].size.h);
	mem_cfg.pool_info[1].blk_cnt = 8;
	
	mem_cfg.pool_info[1].ddr_id = DDR_ID;
#if MD_SUPPORT
	if(APP_MODE>=1)
	{
		// config common pool (sub)
		mem_cfg.pool_info[2].type = HD_COMMON_MEM_COMMON_POOL;
		mem_cfg.pool_info[2].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(IMG_WIDTH, IMG_HEIGHT, HD_VIDEO_PXLFMT_YUV420);
		mem_cfg.pool_info[2].blk_cnt = 4;
		mem_cfg.pool_info[2].ddr_id = DDR_ID;

		// for md
		mem_cfg.pool_info[3].type = HD_COMMON_MEM_USER_BLK;
		mem_cfg.pool_info[3].blk_size = IMG_BUF_SIZE;
		mem_cfg.pool_info[3].blk_cnt = 2;
		mem_cfg.pool_info[3].ddr_id = DDR_ID;
		
		mem_cfg.pool_info[4].type = HD_COMMON_MEM_USER_BLK;
		mem_cfg.pool_info[4].blk_size = IMG_BUF_SIZE*70;
		mem_cfg.pool_info[4].blk_cnt = 1;
		mem_cfg.pool_info[4].ddr_id = DDR_ID;
	}
#endif
	// config common pool (pvd)
	mem_cfg.pool_info[5].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[5].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(PVD_IMG_WIDTH, PVD_IMG_HEIGHT, HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[5].blk_cnt = 4;
	mem_cfg.pool_info[5].ddr_id = DDR_ID;


	if (HD_OK != ai_buffer_total_size_get(&ai_b_size)) {
		(void)fprintf(stderr, "get ai buffer size failed\n");
		return HD_ERR_NG;
	}
	mem_cfg.pool_info[6].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
	mem_cfg.pool_info[6].blk_size = ai_b_size;
	mem_cfg.pool_info[6].blk_cnt = 1;
	mem_cfg.pool_info[6].ddr_id = DDR_ID;

	//mem_cfg.pool_info[7].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
	//mem_cfg.pool_info[7].blk_size = SCALE_BUF_SIZE;
	//mem_cfg.pool_info[7].blk_cnt = 1;
	//mem_cfg.pool_info[7].ddr_id = DDR_ID;

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
static HD_RESULT set_cap_cfg(HD_PATH_ID *p_video_cap_ctrl)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
	HD_PATH_ID video_cap_ctrl = 0;
	HD_VIDEOCAP_CTRL iq_ctl = {0};

	snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_%s", sensor_info[SEL_SEN].name);
	cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux =  0;  //use @0 in peri-dev.dtsi
	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(0, 0);
	if(sensor_info[SEL_SEN].len == 2)
	{
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = HD_VIDEOCAP_SEN_IGNORE;
	}
	if(sensor_info[SEL_SEN].len == 4)
	{
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = HD_VIDEOCAP_SEN_IGNORE;
	}
	
	printf("Using %s\n", cap_cfg.sen_cfg.sen_dev.driver_name);
	ret = hd_videocap_open(0, HD_VIDEOCAP_0_CTRL, &video_cap_ctrl); //open this for device control

	if (ret != HD_OK) {
		printf("[ERR] %s hd_videocap_open fail=%d\n", __func__, ret);
		return ret;
	}
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
	iq_ctl.func = HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB;
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);

	*p_video_cap_ctrl = video_cap_ctrl;
	return ret;
}

static HD_RESULT set_cap_cfg2(HD_PATH_ID *p_video_cap_ctrl)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
	HD_PATH_ID video_cap_ctrl = 0;
	HD_VIDEOCAP_CTRL iq_ctl = {0};

	snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_imx415");
	cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux =  1;  //use @0 in peri-dev.dtsi
	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(2, 0);
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = HD_VIDEOCAP_SEN_IGNORE;
	
	printf("Using %s\n", cap_cfg.sen_cfg.sen_dev.driver_name);
	ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(SEN_VCAP_ID_1), &video_cap_ctrl); //open this for device control

	if (ret != HD_OK) {
		printf("[ERR] %s hd_videocap_open fail=%d\n", __func__, ret);
		return ret;
	}
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
	iq_ctl.func = HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB;
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);

	*p_video_cap_ctrl = video_cap_ctrl;
	return ret;
}

static HD_RESULT set_cap_param(HD_PATH_ID video_cap_path, HD_DIM *p_dim)
{
	HD_RESULT ret = HD_OK;
	{//select sensor mode, manually or automatically
		HD_VIDEOCAP_IN video_in_param = {0};

		video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO; //auto select sensor mode by the parameter of HD_VIDEOCAP_PARAM_OUT
		video_in_param.frc = HD_VIDEO_FRC_RATIO(sensor_info[SEL_SEN].default_fps,1);
		video_in_param.dim.w = p_dim->w;
		video_in_param.dim.h = p_dim->h;
		video_in_param.pxlfmt = SEN_OUT_FMT;
		video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN, &video_in_param);
		//printf("set_cap_param MODE=%d\r\n", ret);
		if (ret != HD_OK) {
			printf("[ERR] %s hd_videocap_set fail=%d\n", __func__, ret);
			return ret;
		}
	}

	{
		HD_VIDEOCAP_CROP video_crop_param = {0};

		video_crop_param.mode = HD_CROP_OFF;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);
		if (ret != HD_OK) {
			printf("[ERR] %s hd_videocap_set fail=%d\n", __func__, ret);
			return ret;
		}
		//printf("set_cap_param CROP NONE=%d\r\n", ret);
	}

	{
		HD_VIDEOCAP_OUT video_out_param = {0};

		//without setting dim for no scaling, using original sensor out size
		video_out_param.pxlfmt = CAP_OUT_FMT;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT, &video_out_param);
		if (ret != HD_OK) {
			printf("[ERR] %s hd_videocap_set fail=%d\n", __func__, ret);
			return ret;
		}
		//printf("set_cap_param OUT=%d\r\n", ret);
	}
	{
		UINT32 data_lane = sensor_info[SEL_SEN].len;
		vendor_videocap_set(video_cap_path, VENDOR_VIDEOCAP_PARAM_DATA_LANE, &data_lane);
	}

	if( g_capbind == HD_VIDEOCAP_OUTFUNC_DIRECT)
	{
		HD_VIDEOCAP_FUNC_CONFIG video_path_param = {0};

		video_path_param.out_func = 0;
		video_path_param.out_func = HD_VIDEOCAP_OUTFUNC_DIRECT;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_FUNC_CONFIG, &video_path_param);
		//printf("set_cap_param PATH_CONFIG=0x%X\r\n", ret);
	}

	if( g_advPushRatio > 0)
	{
		BOOL advance_push = TRUE; 
		UINT32 ratio = g_advPushRatio; 
		     
		vendor_videocap_set(video_cap_path, VENDOR_VIDEOCAP_PARAM_ADVANCE_PUSH, &advance_push);
		vendor_videocap_set(video_cap_path, VENDOR_VIDEOCAP_PARAM_BP3_RATIO, &ratio);
	}

	return ret;
}

static HD_RESULT set_cap_param2(HD_PATH_ID video_cap_path, HD_DIM *p_dim)
{
	HD_RESULT ret = HD_OK;
	UINT32 color_bar_width = 200;
	{//select sensor mode, manually or automatically
		HD_VIDEOCAP_IN video_in_param = {0};

		// color_bar_width = (1920 >> 3) & 0xFFFFFFFE;
		// video_in_param.sen_mode = HD_VIDEOCAP_PATGEN_MODE(HD_VIDEOCAP_SEN_PAT_COLORBAR, color_bar_width); //auto select sensor mode by the parameter of HD_VIDEOCAP_PARAM_OUT
		video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO;
		video_in_param.frc = HD_VIDEO_FRC_RATIO(sensor_info[SEL_SEN].default_fps,1);
		video_in_param.dim.w = p_dim->w;
		video_in_param.dim.h = p_dim->h;
		video_in_param.pxlfmt = SEN_OUT_FMT;
		video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN, &video_in_param);
		//printf("set_cap_param MODE=%d\r\n", ret);
		if (ret != HD_OK) {
			printf("[ERR] %s hd_videocap_set fail=%d\n", __func__, ret);
			return ret;
		}
	}

	{
		HD_VIDEOCAP_CROP video_crop_param = {0};

		video_crop_param.mode = HD_CROP_OFF;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);
		if (ret != HD_OK) {
			printf("[ERR] %s hd_videocap_set fail=%d\n", __func__, ret);
			return ret;
		}
		//printf("set_cap_param CROP NONE=%d\r\n", ret);
	}

	{
		HD_VIDEOCAP_OUT video_out_param = {0};

		//without setting dim for no scaling, using original sensor out size
		video_out_param.pxlfmt = CAP_OUT_FMT | HD_VIDEO_PIX_RGGB_R;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT, &video_out_param);
		if (ret != HD_OK) {
			printf("[ERR] %s hd_videocap_set fail=%d\n", __func__, ret);
			return ret;
		}
		//printf("set_cap_param OUT=%d\r\n", ret);
	}
	{
		UINT32 data_lane = sensor_info[SEL_SEN].len;
		vendor_videocap_set(video_cap_path, VENDOR_VIDEOCAP_PARAM_DATA_LANE, &data_lane);
	}

	// if( g_capbind == HD_VIDEOCAP_OUTFUNC_DIRECT)
	// {
	// 	HD_VIDEOCAP_FUNC_CONFIG video_path_param = {0};

	// 	video_path_param.out_func = 0;
	// 	video_path_param.out_func = HD_VIDEOCAP_OUTFUNC_DIRECT;
	// 	ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_FUNC_CONFIG, &video_path_param);
	// 	//printf("set_cap_param PATH_CONFIG=0x%X\r\n", ret);
	// }

	if( g_advPushRatio > 0)
	{
		BOOL advance_push = TRUE; 
		UINT32 ratio = g_advPushRatio; 
		     
		vendor_videocap_set(video_cap_path, VENDOR_VIDEOCAP_PARAM_ADVANCE_PUSH, &advance_push);
		vendor_videocap_set(video_cap_path, VENDOR_VIDEOCAP_PARAM_BP3_RATIO, &ratio);
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////

static HD_RESULT set_proc_cfg(HD_PATH_ID *p_video_proc_ctrl, HD_DIM* p_max_dim)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
	HD_VIDEOPROC_CTRL video_ctrl_param = {0};
	HD_PATH_ID video_proc_ctrl = 0;

	ret = hd_videoproc_open(0, HD_VIDEOPROC_0_CTRL, &video_proc_ctrl); //open this for device control
	if (ret != HD_OK)
	{
		printf("[ERR] %s hd_videoproc_open fail=%d\n", __func__, ret);
		return ret;
	}

	if (p_max_dim != NULL ) {
		video_cfg_param.pipe = HD_VIDEOPROC_PIPE_RAWALL;
		video_cfg_param.isp_id = SEN_VCAP_ID_0;
		if(g_3dnr)
			video_cfg_param.ctrl_max.func = HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA;
		else
			video_cfg_param.ctrl_max.func = 0;
		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;
		video_cfg_param.in_max.pxlfmt = CAP_OUT_FMT;
		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
		if (ret != HD_OK) {
			return HD_ERR_NG;
		}
	}
	if(g_capbind == HD_VIDEOCAP_OUTFUNC_DIRECT)
	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		video_path_param.in_func = 0;
		video_path_param.in_func |= HD_VIDEOPROC_INFUNC_DIRECT; //direct NOTE: enable direct
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
		//printf("set_proc_param PATH_CONFIG=0x%X\r\n", ret);
	}

	if(g_3dnr){
		video_ctrl_param.func = HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA;
		video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_0_OUT_0;
	}
	else{
		video_ctrl_param.func = 0;
	}
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
	{
		printf("[ERR] %s hd_videoproc_open1 fail=%d\n", __func__, ret);
		return ret;
	}

	if (p_max_dim != NULL ) {
		video_cfg_param.pipe = HD_VIDEOPROC_PIPE_RAWALL;
		video_cfg_param.isp_id = SEN_VCAP_ID_1;
		if(g_3dnr)
			video_cfg_param.ctrl_max.func = HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA;
		else
			video_cfg_param.ctrl_max.func = 0;
		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;
		video_cfg_param.in_max.pxlfmt = CAP_OUT_FMT;
		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
		if (ret != HD_OK) {
			return HD_ERR_NG;
		}
	}
	// if(g_capbind == HD_VIDEOCAP_OUTFUNC_DIRECT)
	// {
	// 	HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

	// 	video_path_param.in_func = 0;
	// 	video_path_param.in_func |= HD_VIDEOPROC_INFUNC_DIRECT; //direct NOTE: enable direct
	// 	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	// 	//printf("set_proc_param PATH_CONFIG=0x%X\r\n", ret);
	// }

	if(g_3dnr){
		video_ctrl_param.func = HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA;
		video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_0_OUT_0;
	}
	else{
		video_ctrl_param.func = 0;
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
		video_out_param.depth = 1; //set 1 to allow pull
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	}

	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		video_path_param.out_func = 0;
		if (g_md_lpm == 1){
			if(stream[0].proc_path == video_proc_path)//only encode need
				video_path_param.out_func |= HD_VIDEOPROC_OUTFUNC_MD;
		}
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
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
		video_path_config.max_mem.enc_buf_ms = 2000;
		video_path_config.max_mem.svc_layer  = HD_SVC_DISABLE;//HD_SVC_4X;
		video_path_config.max_mem.ltr        = FALSE;
		video_path_config.max_mem.rotate     = FALSE;
		video_path_config.max_mem.source_output   = FALSE;
		video_path_config.isp_id             = 0;
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_PATH_CONFIG, &video_path_config);
		if (ret != HD_OK) {
			printf("[ERR] %s hd_videoenc_set fail=%d\n", __func__, ret);
			return HD_ERR_NG;
		}
	}

	return ret;
}

static HD_RESULT set_enc_param(HD_PATH_ID video_enc_path, HD_DIM *p_dim, UINT32 enc_type, UINT32 bitrate)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOENC_IN  video_in_param = {0};
	HD_VIDEOENC_OUT2 video_out_param = {0};
	HD_H26XENC_RATE_CONTROL2 rc_param = {0};

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

		printf("enc_type=%lu\r\n", enc_type);

		if (enc_type == 0) {

			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_H265;
			video_out_param.h26x.profile       = HD_H265E_MAIN_PROFILE;
			video_out_param.h26x.level_idc     = HD_H265E_LEVEL_5;
			video_out_param.h26x.gop_num       = NORMAL_FPS/LOWPOWER_FPS; //12/1fps = 12 or 12/3fps = 4
			video_out_param.h26x.ltr_interval  = 0;
			video_out_param.h26x.ltr_pre_ref   = 0;
			video_out_param.h26x.gray_en       = 0;
			video_out_param.h26x.source_output = 0;
			video_out_param.h26x.svc_layer     = g_svc;//HD_SVC_DISABLE;
			video_out_param.h26x.entropy_mode  = HD_H265E_CABAC_CODING;
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM2, &video_out_param);
			if (ret != HD_OK) {
				printf("set_enc_param_out = %d\r\n", ret);
				return ret;
			}

			//--- HD_VIDEOENC_PARAM_OUT_RATE_CONTROL ---
			rc_param.rc_mode             = HD_RC_MODE_CBR;
			rc_param.cbr.bitrate         = bitrate/30*NORMAL_FPS;
			rc_param.cbr.frame_rate_base = NORMAL_FPS;//1fps
			rc_param.cbr.frame_rate_incr = 1;
			rc_param.cbr.init_i_qp       = 26;
			rc_param.cbr.min_i_qp        = 10;
			rc_param.cbr.max_i_qp        = 51;
			rc_param.cbr.init_p_qp       = 26;
			rc_param.cbr.min_p_qp        = 10;
			rc_param.cbr.max_p_qp        = 51;
			rc_param.cbr.static_time     = 4;
			rc_param.cbr.ip_weight       = 0;
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL2, &rc_param);
			if (ret != HD_OK) {
				printf("set_enc_rate_control = %d\r\n", ret);
				return ret;
			}
			{
				HD_H26XENC_VUI vui = {0};
				vui.vui_en = 1;
				vui.timing_present_flag =1;
				hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_VUI, &vui);
			}
		} else if (enc_type == 1) {

			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_H264;
			video_out_param.h26x.profile       = HD_H264E_HIGH_PROFILE;
			video_out_param.h26x.level_idc     = HD_H264E_LEVEL_5_1;
			video_out_param.h26x.gop_num       = NORMAL_FPS/LOWPOWER_FPS; //12/1fps = 12 or 12/3fps = 4
			video_out_param.h26x.ltr_interval  = 0;
			video_out_param.h26x.ltr_pre_ref   = 0;
			video_out_param.h26x.gray_en       = 0;
			video_out_param.h26x.source_output = 0;
			video_out_param.h26x.svc_layer     = g_svc;//HD_SVC_DISABLE;
			video_out_param.h26x.entropy_mode  = HD_H264E_CABAC_CODING;
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM2, &video_out_param);
			if (ret != HD_OK) {
				printf("set_enc_param_out = %d\r\n", ret);
				return ret;
			}

			//--- HD_VIDEOENC_PARAM_OUT_RATE_CONTROL ---
			rc_param.rc_mode             = HD_RC_MODE_CBR;
			rc_param.cbr.bitrate         = bitrate;
			rc_param.cbr.frame_rate_base = NORMAL_FPS;//1fps
			rc_param.cbr.frame_rate_incr = 1;
			rc_param.cbr.init_i_qp       = 26;
			rc_param.cbr.min_i_qp        = 10;
			rc_param.cbr.max_i_qp        = 51;
			rc_param.cbr.init_p_qp       = 26;
			rc_param.cbr.min_p_qp        = 10;
			rc_param.cbr.max_p_qp        = 51;
			rc_param.cbr.static_time     = 4;
			rc_param.cbr.ip_weight       = 0;
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL2, &rc_param);
			if (ret != HD_OK) {
				printf("set_enc_rate_control = %d\r\n", ret);
				return ret;
			}
			{
				HD_H26XENC_VUI vui = {0};
				vui.vui_en = 1;
				vui.timing_present_flag =1;
				hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_VUI, &vui);
			}

		} else if (enc_type == 2) {

			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_JPEG;
			video_out_param.jpeg.retstart_interval = 0;
			video_out_param.jpeg.image_quality = 50;
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM2, &video_out_param);
			if (ret != HD_OK) {
				printf("set_enc_param_out = %d\r\n", ret);
				return ret;
			}

		} else {

			printf("not support enc_type\r\n");
			return HD_ERR_NG;
		}
	}
	if(EN_ENC_SKIP_FRAME == 1)
	{
#if 0
		{
		    VENDOR_VIDEOENC_H26X_SKIP_FRM_CFG skip = {0};
		    skip.b_enable = 1;
		    skip.target_fr = NORMAL_FPS; 
		    skip.input_frm_cnt = LOWPOWER_FPS;
		    vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_H26X_SKIP_FRM, &skip);
		}
#endif
	}
	{
		VENDOR_VIDEOENC_H26X_LOW_POWER_CFG lpm = {0};
		lpm.b_enable = 1;
		//lpm.mode = 0;
		vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_H26X_LOW_POWER, &lpm);
	}
	return ret;
}

static HD_RESULT init_module_cap_proc(void)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_init()) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_init()) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT init_module_enc(void)
{
	HD_RESULT ret;
    if ((ret = hd_videoenc_init()) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT set_cap_cfg_new(HD_PATH_ID *p_video_cap_ctrl, UINT32 sen_id)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
	HD_PATH_ID video_cap_ctrl = 0;
	HD_VIDEOCAP_CTRL iq_ctl = {0};

	snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_%s", sensor_info[SEL_SEN].name);
	cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux = (sen_id == 0) ? 0 : 1;  //use @0 in peri-dev.dtsi
	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = (sen_id == 0) ? HD_VIDEOCAP_SEN_CLANE_CSI(0, 0) : HD_VIDEOCAP_SEN_CLANE_CSI(2, 0);
	if(sensor_info[SEL_SEN].len == 2)
	{
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = HD_VIDEOCAP_SEN_IGNORE;
	}
	if(sensor_info[SEL_SEN].len == 4)
	{
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = HD_VIDEOCAP_SEN_IGNORE;
	}
	
	printf("Using %s\n", cap_cfg.sen_cfg.sen_dev.driver_name);
	ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(sen_id), &video_cap_ctrl); //open this for device control

	if (ret != HD_OK) {
		printf("[ERR] %s hd_videocap_open fail=%d\n", __func__, ret);
		return ret;
	}
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
	iq_ctl.func = HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB;
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);

	*p_video_cap_ctrl = video_cap_ctrl;
	return ret;
}

static HD_RESULT set_proc_cfg_new(HD_PATH_ID *p_video_proc_ctrl, HD_DIM* p_max_dim, UINT32 sen_id)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
	HD_VIDEOPROC_CTRL video_ctrl_param = {0};
	HD_PATH_ID video_proc_ctrl = 0;

	ret = hd_videoproc_open(0, HD_VIDEOPROC_CTRL(sen_id), &video_proc_ctrl); //open this for device control
	if (ret != HD_OK)
	{
		printf("[ERR] %s hd_videoproc_open fail=%d\n", __func__, ret);
		return ret;
	}

	if (p_max_dim != NULL ) {
		video_cfg_param.pipe = HD_VIDEOPROC_PIPE_RAWALL;
		video_cfg_param.isp_id = sen_id;
		if(g_3dnr)
			video_cfg_param.ctrl_max.func = HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA;
		else
			video_cfg_param.ctrl_max.func = 0;
		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;
		video_cfg_param.in_max.pxlfmt = CAP_OUT_FMT;
		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
		if (ret != HD_OK) {
			return HD_ERR_NG;
		}
	}
	if(g_capbind == HD_VIDEOCAP_OUTFUNC_DIRECT)
	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		video_path_param.in_func = 0;
		video_path_param.in_func |= HD_VIDEOPROC_INFUNC_DIRECT; //direct NOTE: enable direct
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
		//printf("set_proc_param PATH_CONFIG=0x%X\r\n", ret);
	}

	if(g_3dnr){
		video_ctrl_param.func = HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA;
		video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_0_OUT_0;
	}
	else{
		video_ctrl_param.func = 0;
	}
	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);

	*p_video_proc_ctrl = video_proc_ctrl;

	return ret;
}

static HD_RESULT open_module_cap_proc_new(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim, UINT32 sen_id)
{
	HD_RESULT ret;

	// set videocap config
	ret = set_cap_cfg_new(&p_stream->cap_ctrl, sen_id);
	if (ret != HD_OK) {
		printf("[ERR] %s set_cap_cfg fail=%d\n", __func__, ret);
		return HD_ERR_NG;
	}
	// set videoproc config
	ret = set_proc_cfg_new(&p_stream->proc_ctrl, p_proc_max_dim, sen_id);
	if (ret != HD_OK) {
		printf("[ERR] %s set_proc_cfg fail=%d\n", __func__, ret);
		return HD_ERR_NG;
	}

	if ((ret = hd_videocap_open(HD_VIDEOCAP_IN(sen_id, 0), HD_VIDEOCAP_OUT(sen_id, 0), &p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_IN(sen_id, 0), HD_VIDEOPROC_OUT(sen_id, 0), &p_stream->proc_path)) != HD_OK)
		return ret;

#if MD_SUPPORT
	if (APP_MODE >= 1) {
		if ((ret = hd_videoproc_open(HD_VIDEOPROC_IN(sen_id, 0), HD_VIDEOPROC_OUT(sen_id, 1), &p_stream->proc_alg_path)) != HD_OK)
			return ret;
	}
#endif

	if ((ret = hd_videoproc_open(HD_VIDEOPROC_IN(sen_id, 0), HD_VIDEOPROC_OUT(sen_id, 2), &p_stream->proc_pvd_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT open_module_cap_proc(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;
	// set videocap config
	ret = set_cap_cfg(&p_stream->cap_ctrl);
	if (ret != HD_OK) {
		printf("[ERR] %s set_cap_cfg fail=%d\n", __func__, ret);
		return HD_ERR_NG;
	}
	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim);
	if (ret != HD_OK) {
		printf("[ERR] %s set_proc_cfg fail=%d\n", __func__, ret);
		return HD_ERR_NG;
	}

	if ((ret = hd_videocap_open(HD_VIDEOCAP_0_IN_0, HD_VIDEOCAP_0_OUT_0, &p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &p_stream->proc_path)) != HD_OK)
		return ret;
#if MD_SUPPORT
	if(APP_MODE>=1)
	{
		if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_1, &p_stream->proc_alg_path)) != HD_OK)
			return ret;
	}
#endif
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_2, &p_stream->proc_pvd_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT open_module_cap_proc2(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;
	// set videocap config
	ret = set_cap_cfg2(&p_stream->cap_ctrl);
	if (ret != HD_OK) {
		printf("[ERR] %s set_cap_cfg fail=%d\n", __func__, ret);
		return HD_ERR_NG;
	}
	// set videoproc config
	ret = set_proc_cfg2(&p_stream->proc_ctrl, p_proc_max_dim);
	if (ret != HD_OK) {
		printf("[ERR] %s set_proc_cfg fail=%d\n", __func__, ret);
		return HD_ERR_NG;
	}

	if ((ret = hd_videocap_open(HD_VIDEOCAP_IN(SEN_VCAP_ID_1, 0), HD_VIDEOCAP_OUT(SEN_VCAP_ID_1, 0), &p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_1_IN_0, HD_VIDEOPROC_1_OUT_0, &p_stream->proc_path)) != HD_OK)
		return ret;
#if MD_SUPPORT
	if(APP_MODE>=1)
	{
		if ((ret = hd_videoproc_open(HD_VIDEOPROC_1_IN_0, HD_VIDEOPROC_1_OUT_1, &p_stream->proc_alg_path)) != HD_OK)
			return ret;
	}
#endif
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_1_IN_0, HD_VIDEOPROC_1_OUT_2, &p_stream->proc_pvd_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static int set_enc_mask_param(HD_PATH_ID mask_path, int idx)
{
	VENDOR_VIDEOENC_OSG_MASK attr;

	memset(&attr, 0, sizeof(VENDOR_VIDEOENC_OSG_MASK));

	attr.position.x = mask[idx].x;
	attr.position.y = mask[idx].y;
	attr.dim.w      = mask[idx].w;
	attr.dim.h      = mask[idx].h;
	attr.type       = mask[idx].type;
	attr.thickness  = mask[idx].thick;
	attr.alpha      = mask[idx].alpha;
	attr.color      = mask[idx].color;
	attr.layer      = 0;
	attr.region     = idx;

	return vendor_videoenc_set(mask_path, VENDOR_VIDEOENC_PARAM_IN_MASK_ATTR, &attr);
}

static HD_RESULT open_module_enc(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_VIDEOENC_0_OUT_0, &p_stream->enc_path)) != HD_OK)
		return ret;
#if OSG_SUPPORT
	int i;
	for(i = 0 ; i < MASK_NUMBER ; ++i)
	{
		if((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_STAMP(i), &(p_stream->enc_mask_path[i]))) != HD_OK)
		{
			return ret;
			printf("fail to open enc mask\r\n");
		}
		if(set_enc_mask_param(stream[0].enc_mask_path[i], i))
		{
			return ret;
			printf("fail to set enc mask\r\n");
		}
	}
#endif

	return HD_OK;
}

static HD_RESULT open_module_enc2(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_1, HD_VIDEOENC_0_OUT_1, &p_stream->enc_path)) != HD_OK)
		return ret;
#if OSG_SUPPORT
	int i;
	for(i = 0 ; i < MASK_NUMBER ; ++i)
	{
		if((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_STAMP(i), &(p_stream->enc_mask_path[i]))) != HD_OK)
		{
			return ret;
			printf("fail to open enc mask\r\n");
		}
		if(set_enc_mask_param(stream[0].enc_mask_path[i], i))
		{
			return ret;
			printf("fail to set enc mask\r\n");
		}
	}
#endif

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

static int osg_blink(HD_PATH_ID mask_path, int en)
{
#if OSG_SUPPORT
	if(en)
		return hd_videoenc_start(mask_path);
	else
		return hd_videoenc_stop(mask_path);
#endif
	return 0;
}

#if MD_SUPPORT
extern void *md_thread_api(VOID *arg);
pthread_t md_thread_id;
#endif
static void *thread_cap_proc(void *ptr)
{
	UINT32 i;
	HD_RESULT ret;

	// init capture, proc
	ret = init_module_cap_proc();
	if (ret != HD_OK) {
		printf("[ERR] %s init_module_cap_proc fail=%d\n", __func__, ret);
		pthread_exit((void *)-1);
		return NULL;
	}

	for (i = 0; i < g_sensor_num; i++) {
		stream[i].proc_max_dim.w = sensor_info[SEL_SEN].size.w;
		stream[i].proc_max_dim.h = sensor_info[SEL_SEN].size.h;
		ret = open_module_cap_proc_new(&stream[i], &stream[i].proc_max_dim, SEN_VCAP_ID[i]);
		if (ret != HD_OK) {
			printf("[ERR] %s open_module_cap_proc fail=%d\n", __func__, ret);
			pthread_exit((void *)-1);
			return NULL;
		}

		stream[i].cap_dim.w = sensor_info[SEL_SEN].size.w; //assign by user
		stream[i].cap_dim.h = sensor_info[SEL_SEN].size.h; //assign by user
		ret = set_cap_param(stream[i].cap_path, &stream[i].cap_dim);
		if (ret != HD_OK) {
			printf("[ERR] %s set_cap_param fail=%d\n", __func__, ret);
			pthread_exit((void *)-1);
			return NULL;
		}

		ret = set_proc_param(stream[i].proc_path, &stream[i].cap_dim);
		if (ret != HD_OK) {
			printf("[ERR] %s set_proc_param fail=%d\n", __func__, ret);
			pthread_exit((void *)-1);
			return NULL;
		}

		hd_videocap_bind(HD_VIDEOCAP_OUT(SEN_VCAP_ID[i], 0), HD_VIDEOPROC_IN(SEN_VCAP_ID[i], 0));
		if(g_capbind == HD_VIDEOCAP_OUTFUNC_DIRECT)
		{
			hd_videocap_start(stream[i].cap_path);
			SENSOR_STATUS = SEN_STATUS_AWAKE;
			hd_videoproc_start(stream[i].proc_path);
			
		}
		else
		{
			hd_videoproc_start(stream[i].proc_path);
			hd_videocap_start(stream[i].cap_path);
			SENSOR_STATUS = SEN_STATUS_AWAKE;
		}

#if MD_SUPPORT
		if(APP_MODE>=1) {
			int policy;
			struct sched_param schedparam = {0};
			//MD
			// set videoproc parameter (MD)
			stream[i].proc_alg_max_dim.w = IMG_WIDTH;
			stream[i].proc_alg_max_dim.h = IMG_HEIGHT;
			ret = set_proc_param(stream[i].proc_alg_path, &stream[i].proc_alg_max_dim);
			if (ret != HD_OK) {
				printf("[ERR] %s set_proc_param fail=%d\n", __func__, ret);
				pthread_exit((void *)-1);
				return NULL;
			}
			hd_videoproc_start(stream[i].proc_alg_path);
			
			//MD
			// create thread
		#if 1
			ret = pthread_create(&md_thread_id, NULL, md_thread_api, (VOID*)&stream[i]);
		#else
			ret = pthread_create(&md_thread_id, NULL, md_thread_api, NULL);
		#endif
			if (ret < 0) {
				printf("create encode thread failed");
				while(1);
			}
		#if 1
			if (0 != pthread_getschedparam(md_thread_id, &policy, &schedparam)) {
					printf("pthread_getschedparam failed\r\n");
			} else {
				schedparam.sched_priority = 20;
				pthread_setschedparam(md_thread_id, policy, &schedparam);
			}
		#endif
		}
#endif

		// set videoproc parameter (AI PVD)
		stream[i].proc_pvd_max_dim.w = PVD_IMG_WIDTH;
		stream[i].proc_pvd_max_dim.h = PVD_IMG_HEIGHT;
		ret = set_proc_param(stream[i].proc_pvd_path, &stream[i].proc_pvd_max_dim);
		if (ret != HD_OK) {
			printf("set proc alg fail=%d\n", ret);
			pthread_exit((void *)-1);
			return NULL;
		}
		hd_videoproc_start(stream[i].proc_pvd_path);

		//// path for AI thead
		//{
		//	ai_thread_proc_path = stream[0].proc_path;
		//	ai_thread_venc_path = stream[0].enc_path;
		//	ai_thread_vcap_path = stream[0].cap_path;
			ai_thread_pvd_path[i] = stream[i].proc_pvd_path;
		//	printf("ai_thread_proc_path = 0x%x\r\n", (unsigned int)ai_thread_proc_path);
		//}
	}

#if 0
	// open capture, proc
	stream[0].proc_max_dim.w = sensor_info[SEL_SEN].size.w; //assign by user
	stream[0].proc_max_dim.h = sensor_info[SEL_SEN].size.h; //assign by user
	ret = open_module_cap_proc(&stream[0], &stream[0].proc_max_dim);
	if (ret != HD_OK) {
		printf("[ERR] %s open_module_cap_proc fail=%d\n", __func__, ret);
		pthread_exit((void *)-1);
		return NULL;
	}

	stream[1].proc_max_dim.w = sensor_info[SEL_SEN].size.w; //assign by user
	stream[1].proc_max_dim.h = sensor_info[SEL_SEN].size.h; //assign by user
	ret = open_module_cap_proc2(&stream[1], &stream[1].proc_max_dim);
	if (ret != HD_OK) {
		printf("[ERR] %s open_module_cap_proc2 fail=%d\n", __func__, ret);
		pthread_exit((void *)-1);
		return NULL;
	}

	// set videocap parameter
	stream[0].cap_dim.w = sensor_info[SEL_SEN].size.w; //assign by user
	stream[0].cap_dim.h = sensor_info[SEL_SEN].size.h; //assign by user
	ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim);
	if (ret != HD_OK) {
		printf("[ERR] %s set_cap_param fail=%d\n", __func__, ret);
		pthread_exit((void *)-1);
		return NULL;
	}

	stream[1].cap_dim.w = sensor_info[SEL_SEN].size.w; //assign by user
	stream[1].cap_dim.h = sensor_info[SEL_SEN].size.h; //assign by user
	ret = set_cap_param2(stream[1].cap_path, &stream[1].cap_dim);
	if (ret != HD_OK) {
		printf("[ERR] %s set_cap_param2 fail=%d\n", __func__, ret);
		pthread_exit((void *)-1);
		return NULL;
	}

	// set videoproc parameter (main)
	ret = set_proc_param(stream[0].proc_path, &stream[0].cap_dim);
	if (ret != HD_OK) {
		printf("[ERR] %s set_proc_param fail=%d\n", __func__, ret);
		pthread_exit((void *)-1);
		return NULL;
	}

	ret = set_proc_param(stream[1].proc_path, &stream[1].cap_dim);
	if (ret != HD_OK) {
		printf("[ERR] %s set_proc_param2 fail=%d\n", __func__, ret);
		pthread_exit((void *)-1);
		return NULL;
	}

	hd_videocap_bind(HD_VIDEOCAP_OUT(SEN_VCAP_ID_0, 0), HD_VIDEOPROC_0_IN_0);
	hd_videocap_bind(HD_VIDEOCAP_OUT(SEN_VCAP_ID_1, 0), HD_VIDEOPROC_1_IN_0);
	if(g_capbind == HD_VIDEOCAP_OUTFUNC_DIRECT)
	{
		for (i = 0; i < SENSOR_MAX_NUM; i++) {
			hd_videocap_start(stream[i].cap_path);
			SENSOR_STATUS = SEN_STATUS_AWAKE;
			hd_videoproc_start(stream[i].proc_path);
		}
		
	}
	else
	{
		for (i = 0; i < SENSOR_MAX_NUM; i++) {
			hd_videoproc_start(stream[i].proc_path);
			hd_videocap_start(stream[i].cap_path);
			SENSOR_STATUS = SEN_STATUS_AWAKE;
		}
	}
	
#if MD_SUPPORT
	if(APP_MODE>=1)
	{
		int policy;
		struct sched_param schedparam = {0};
		//MD
		// set videoproc parameter (MD)
		stream[0].proc_alg_max_dim.w = IMG_WIDTH;
		stream[0].proc_alg_max_dim.h = IMG_HEIGHT;
		ret = set_proc_param(stream[0].proc_alg_path, &stream[0].proc_alg_max_dim);
		if (ret != HD_OK) {
			printf("[ERR] %s set_proc_param fail=%d\n", __func__, ret);
			pthread_exit((void *)-1);
			return NULL;
		}
		hd_videoproc_start(stream[0].proc_alg_path);
		
		//MD
		// create thread
	#if 1
		ret = pthread_create(&md_thread_id, NULL, md_thread_api, (VOID*)&stream[0]);
	#else
		ret = pthread_create(&md_thread_id, NULL, md_thread_api, NULL);
	#endif
		if (ret < 0) {
			printf("create encode thread failed");
			while(1);
		}
	#if 1
		if (0 != pthread_getschedparam(md_thread_id, &policy, &schedparam)) {
				printf("pthread_getschedparam failed\r\n");
		} else {
			schedparam.sched_priority = 20;
			pthread_setschedparam(md_thread_id, policy, &schedparam);
		}
	#endif
	}
#endif

	for (i = 0; i < SENSOR_MAX_NUM; i++) {
		// set videoproc parameter (AI PVD)
		stream[i].proc_pvd_max_dim.w = PVD_IMG_WIDTH;
		stream[i].proc_pvd_max_dim.h = PVD_IMG_HEIGHT;
		ret = set_proc_param(stream[i].proc_pvd_path, &stream[i].proc_pvd_max_dim);
		if (ret != HD_OK) {
			printf("set proc alg fail=%d\n", ret);
			pthread_exit((void *)-1);
			return NULL;
		}
		hd_videoproc_start(stream[i].proc_pvd_path);

		//// path for AI thead
		//{
		//	ai_thread_proc_path = stream[0].proc_path;
		//	ai_thread_venc_path = stream[0].enc_path;
		//	ai_thread_vcap_path = stream[0].cap_path;
			ai_thread_pvd_path = stream[i].proc_pvd_path;
		//	printf("ai_thread_proc_path = 0x%x\r\n", (unsigned int)ai_thread_proc_path);
		//}
	}
	ai_thread_pvd_path = stream[0].proc_pvd_path;
#endif

	pthread_exit((void *)0);
	return NULL;
}

static HD_RESULT open_module_enc_new(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim, UINT32 sen_id)
{
	HD_RESULT ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_IN(0, sen_id), HD_VIDEOENC_OUT(0, sen_id), &p_stream->enc_path)) != HD_OK)\
		return ret;
#if OSG_SUPPORT
	int i;
	for(i = 0 ; i < MASK_NUMBER ; ++i)
	{
		if((ret = hd_videoenc_open(HD_VIDEOENC_IN(0, sen_id), HD_STAMP(i), &(p_stream->enc_mask_path[i]))) != HD_OK)
		{
			return ret;
			printf("fail to open enc mask\r\n");
		}
		if(set_enc_mask_param(stream[0].enc_mask_path[i], i))
		{
			return ret;
			printf("fail to set enc mask\r\n");
		}
	}
#endif

	return HD_OK;
}

static void *thread_enc(void *ptr)
{
	UINT32 i;
	VIDEO_RECORD* p_stream0 = (VIDEO_RECORD *)ptr;
	HD_RESULT ret;
	UINT32 enc_type = 0;

	// init capture, proc
	ret = init_module_enc();
	if (ret != HD_OK) {
		printf("[ERR] %s init_module_enc fail=%d\n", __func__, ret);
		pthread_exit((void *)-1);
		return NULL;
	}

	for (i = 0; i < g_sensor_num; i++) { // open capture, proc
		stream[i].proc_max_dim.w = sensor_info[SEL_SEN].size.w; //assign by user
		stream[i].proc_max_dim.h = sensor_info[SEL_SEN].size.h; //assign by user
		ret = open_module_enc_new(&stream[i], &stream[i].proc_max_dim, SEN_VCAP_ID[i]);
		if (ret != HD_OK) {
			printf("[ERR] %s open_module_enc fail=%d\n", __func__, ret);
			pthread_exit((void *)-1);
			return NULL;
		}

		stream[i].enc_max_dim.w = sensor_info[SEL_SEN].size.w;
		stream[i].enc_max_dim.h = sensor_info[SEL_SEN].size.h;
		ret = set_enc_cfg(stream[i].enc_path, &stream[i].enc_max_dim, BITRATE);
		if (ret != HD_OK) {
			printf("[ERR] %s set_enc_cfg[%d] fail=%d\n", __func__, i, ret);
			pthread_exit((void *)-1);
			return NULL;
		}

		stream[i].enc_dim.w = sensor_info[SEL_SEN].size.w;
		stream[i].enc_dim.h = sensor_info[SEL_SEN].size.h;
		enc_type = ENC_TYPE;
		ret = set_enc_param(stream[i].enc_path, &stream[i].enc_dim, enc_type, BITRATE);
		if (ret != HD_OK) {
			printf("[ERR] %s set_enc_param[%d] fail=%d\n", __func__, i, ret);
			pthread_exit((void *)-1);
			return NULL;
		}

		if (g_prcbind == 0xff) {

		} else {
			// bind video_record modules (main)
			hd_videoproc_bind(HD_VIDEOPROC_OUT(SEN_VCAP_ID[i], 0), HD_VIDEOENC_IN(0, SEN_VCAP_ID[i]));
		}
	}

	pthread_exit((void *)0);
	return NULL;
}

#define FLGKEY_UP                       0x00000001      // Key UP pressed
#define FLGKEY_DOWN                     0x00000002      // Key DOWN pressed
#define FLGKEY_LEFT                     0x00000004      // Key LEFT pressed
#define FLGKEY_RIGHT                    0x00000008      // Key RIGHT pressed
#define FLGKEY_ENTER                    0x00000010      // Key ENTER (SET) pressed

static void key_event(UINT32 uiKeyCode)
{
	if (uiKeyCode & FLGKEY_UP) {
		HD_DEBUG_MSG("DETKEY KEY UP\r\n");
		//trigger save and save to next buffer
		filesave_trigger();
		// trigger_filesave();
	}
	if (uiKeyCode & FLGKEY_DOWN) {
		HD_DEBUG_MSG("DETKEY KEY DOWN\r\n");
	}
	if (uiKeyCode & FLGKEY_LEFT) {
		HD_DEBUG_MSG("DETKEY KEY LEFT\r\n");
	}
	if (uiKeyCode & FLGKEY_RIGHT) {
		HD_DEBUG_MSG("DETKEY KEY RIGHT\r\n");
	}
	if (uiKeyCode & FLGKEY_ENTER) {
		HD_DEBUG_MSG("DETKEY KEY ENTER\r\n");
	}
}

static void *thread_gpio_button(void *ptr)
{
	int uiGPIOValue=1,uipre_GPIOValue=1;

	printf("##Start button!!!!!\r\n");
	gpio_set_dir(42, 0);

	while (1){
		uiGPIOValue = gpio_get_value(42);
		if( uiGPIOValue!= -1){
			if(!uipre_GPIOValue){
				//printf("KEY %d\n",(int)uipre_GPIOValue);	
				////////////////////////
				key_event(FLGKEY_UP);
			}
		}
		uipre_GPIOValue = uiGPIOValue;
		//printf("check 1 sec,%d\n",(int)hd_gettime_ms());	
		msleep(50);
	}
	key_event(FLGKEY_UP);
	pthread_exit((void *)0);
	return NULL;
}

static HD_RESULT Vproc_Pull(void)
{
	UINT i;
	HD_RESULT ret = HD_OK;
	UINTPTR phy_addr_main, vir_addr_main;
	UINT32 yuv_size;

	for (i = 0; i < g_sensor_num; i++) {
		ret |= hd_videoproc_pull_out_buf(stream[i].proc_path, &video_frame_Main[i], 100); // -1 = blocking mode
		if (ret != HD_OK) {
			printf("[WRN] %s hd_videoproc_pull_out_buf video_frame_Main[%d] fail=%d\n", __func__, i, ret);
		}
		// (void)fprintf(stdout, "video_frame_Main[%d] count = %llu\n", i, video_frame_Main[i].count);
	}
	if (ret != HD_OK) {
		return ret;
	}

	for (i = 0; i < g_sensor_num; i++) {
		ret = hd_videoproc_pull_out_buf(stream[i].proc_pvd_path, &video_frame_PVD[i], 100); // -1 = blocking mode, 0 = non-blocking mode, >0 = blocking-timeout mode
		if (ret != HD_OK) {
			printf("[WRN] %s hd_videoproc_pull_out_buf video_frame_PVD[%d] fail=%d\n", __func__, i, ret);
			// Release video_frame_Main
			HD_RESULT ret2 = HD_OK;
			ret2 = hd_videoproc_release_out_buf(stream[i].proc_path, &video_frame_Main[i]);
			if (ret2 != HD_OK) {
				printf("[WRN] %s hd_videoproc_release_out_buf[%d] fail=%d\n", __func__, i, ret2);
			}
			return ret;
		}
	}

	return ret;
}

static void *encode_thread(void *arg)
{
	VIDEO_RECORD* p_stream0 = (VIDEO_RECORD *)arg;
	HD_RESULT ret = HD_OK;
	HD_VIDEOENC_BS  data_pull;
	UINT32 i, j;
	UINTPTR vir_addr_main[SENSOR_MAX_NUM] = {0};
	HD_VIDEOENC_BUFINFO phy_buf_main[SENSOR_MAX_NUM] = {0};

	unsigned int get_index=0;
	unsigned int is_first_yuv = 0;

#define PHY2VIRT_MAIN(pa, va, pb) (va + (pa - pb.buf_info.phy_addr))

	//------ wait flag_enc_start ------
	printf("=============== Wait flag_enc_start ===================\r\n");
	while (p_stream0->flag_enc_start == 0) usleep(3 * 1000);
	printf("=============== flag_enc_start ===================\r\n");

	for (i = 0; i < g_sensor_num; i++) {
		hd_videoenc_start(stream[i].enc_path);
	}

	// query physical address of bs buffer ( this can ONLY query after hd_videoenc_start() is called !! )
	for (i = 0; i < g_sensor_num; i++) {
		hd_videoenc_get(stream[i].enc_path, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf_main[i]);
		
		// mmap for bs buffer (just mmap one time only, calculate offset to virtual address later)
		vir_addr_main[i] = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_main[i].buf_info.phy_addr, phy_buf_main[i].buf_info.buf_size);
	}


	//----- open output files -----

	//--------- pull data test ---------
	while (p_stream0->enc_exit == 0) {
		while(wait_enc_thd_done == 0 && (!exit_encode_thread)){
			msleep(1);
		}
		if(exit_encode_thread)
			break;
		//gpio_set_value(198, 1);

		//-------  Preparing for encode ---------//
		if(change_fps)
		{
			HD_H26XENC_RATE_CONTROL2 rc_param = {0};    
			//hd_videoenc_stop(ai_thread_venc_path);
			//--- HD_VIDEOENC_PARAM_OUT_RATE_CONTROL ---
			rc_param.rc_mode             = HD_RC_MODE_CBR;

			if(change_fps == NORMAL_FPS)
			{
				rc_param.cbr.bitrate         = BITRATE;
				rc_param.cbr.frame_rate_base = NORMAL_FPS;
				rc_param.cbr.frame_rate_incr = 1;
			}
			else{
				rc_param.cbr.bitrate         = BITRATE;
				rc_param.cbr.frame_rate_base = NORMAL_FPS;
				rc_param.cbr.frame_rate_incr = 1;
			}
			rc_param.cbr.init_i_qp       = 26;
			rc_param.cbr.min_i_qp        = 10;
			rc_param.cbr.max_i_qp        = 45;
			rc_param.cbr.init_p_qp       = 26;
			rc_param.cbr.min_p_qp        = 10;
			rc_param.cbr.max_p_qp        = 45;
			rc_param.cbr.static_time     = 4;
			rc_param.cbr.ip_weight       = 0;
			for (i = 0; i < g_sensor_num; i++) {
				ret = hd_videoenc_set(stream[i].enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL2, &rc_param);
				if (ret != HD_OK) {
					printf("set_enc_rate_control = %d\r\n", ret);
					//return ret;
				}
			}

			if(change_fps == NORMAL_FPS) {
				skip_ratio = 1;
				if(EN_ENC_SKIP_FRAME == 1) {
#if 0
					VENDOR_VIDEOENC_H26X_SKIP_FRM_CFG skip = {0};
					skip.b_enable = 0;
					skip.target_fr = target_fps; 
					skip.input_frm_cnt = target_fps;
					vendor_videoenc_set(ai_thread_venc_path, VENDOR_VIDEOENC_PARAM_OUT_H26X_SKIP_FRM, &skip);
#endif
					for (i = 0; i < g_sensor_num; i++) {
						hd_videoenc_start(stream[i].enc_path);
					}
				}
			}
			else{
				if(EN_ENC_SKIP_FRAME == 1) {
					skip_ratio = NORMAL_FPS/LOWPOWER_FPS;
#if 0
					VENDOR_VIDEOENC_H26X_SKIP_FRM_CFG skip = {0};
					skip.b_enable = 1;
					skip.target_fr = target_fps; 
					skip.input_frm_cnt = DEFAULT_ENC_FPS;
					vendor_videoenc_set(ai_thread_venc_path, VENDOR_VIDEOENC_PARAM_OUT_H26X_SKIP_FRM, &skip);
#endif
					for (i = 0; i < g_sensor_num; i++) {
						hd_videoenc_start(stream[i].enc_path);
					}
				}
				else
					skip_ratio = 1;
			}

			change_fps = 0; //add mutex protect better
		}
		do_venc_flag = skip_ratio;	
		// ------- End of preparing for encode  ---------//

		{
			for(i=0;(int)i<skip_ratio;i++)
			{
				for (j = 0; j < g_sensor_num; j++) {
					//printf("venc1 %d,%d\n",(int)hd_gettime_ms(),i);
					//printf("%s:%d\r\n", __func__, video_frame_Main.count);
					ret = hd_videoenc_push_in_buf(stream[j].enc_path, &video_frame_Main[j], NULL, 100); // blocking mode
					if (ret != HD_OK) {
						printf("[WRN] %s hd_videoenc_push_in_buf[%d] fail=%d\n", __func__, j, ret);
					}
				}
			}
		}

		for (i = 0; i < g_sensor_num; i++) {
			ret = hd_videoproc_release_out_buf(stream[i].proc_path, &video_frame_Main[i]);
			if (ret != HD_OK) {
				printf("[WRN] %s hd_videoproc_release_out_buf fail=%d\n", __func__, ret);
			}
		}
		
		
		//pull data
		if (!first_skip) {
			for (i = 0; i < g_sensor_num; i++) {
				(void)memset(&data_pull, 0, sizeof(data_pull));
				ret = hd_videoenc_pull_out_buf(stream[i].enc_path, &data_pull, 600); // timeout is error,too.
				//printf("venc pull %d\n",hd_gettime_ms());
				if (ret == HD_OK) {
					if(EN_AUTO_SAVE)
					{
						for (j=0; j< data_pull.pack_num; j++) {
							UINTPTR ptr = (UINTPTR)PHY2VIRT_MAIN(data_pull.video_pack[j].phy_addr, vir_addr_main[i], phy_buf_main[i]);
							UINT32 len = data_pull.video_pack[j].size;
							filesave_record(stream[i].fs_ctx, ptr, len);
						}
					}
					else
					{
						// Clean DRAM Buffer
						for (j=0; j< data_pull.pack_num; j++) {
							UINTPTR ptr = (UINTPTR)PHY2VIRT_MAIN(data_pull.video_pack[j].phy_addr, vir_addr_main[i], phy_buf_main[i]);
							UINT32 len = data_pull.video_pack[j].size;
						}
					}

					// release data
					ret = hd_videoenc_release_out_buf(stream[i].enc_path, &data_pull);
					if (ret != HD_OK) {
						printf("[WRN] %s hd_videoenc_release_out_buf fail=%d\n", __func__, ret);
					}
				}
				else //Add Error Handle
				{
					printf("[WRN] %s hd_videoenc_pull_out_buf[%d] fail=%d\n", __func__, i, ret);
				}
			}
		}
		first_skip = 0;
		

		do_venc_flag--;
		wait_enc_thd_done = 0;

		//gpio_set_value(198, 0);
	}

	// mummap for bs buffer
	for (i = 0; i < g_sensor_num; i++) {
		if (vir_addr_main[i]) {
			if (hd_common_mem_munmap((void *)vir_addr_main[i], phy_buf_main[i].buf_info.buf_size) != 0) {
				printf("[WRN] %s hd_common_mem_munmap fail=%d\n", __func__, ret);
			}
		}
	}

	// close output file
	pthread_exit((void *)0);
	return 0;

}

static HD_RESULT disable_dpc_func(UINT32 id) 
{
	HD_RESULT ret;
	UINT32 isp_id = id;
	static IQT_DPC_PARAM dpc = {0};

	dpc.id = isp_id;
	ret = vendor_isp_get_iq(IQT_ITEM_DPC_PARAM, &dpc);
	if (ret != HD_OK) {
		printf("vendor_isp_get_iq (IQT_ITEM_DPC_PARAM) fail\r\n");
		goto exit;
	}
	dpc.dpc.enable = 0;
	ret = vendor_isp_set_iq(IQT_ITEM_DPC_PARAM, &dpc);
	if (ret != HD_OK) {
		printf("vendor_isp_set_iq (IQT_ITEM_DPC_PARAM, enable (%d)) fail\r\n", (int)dpc.dpc.enable);
		goto exit;
	}

	exit:
	return ret;
}

static HD_RESULT disable_ecs_func(UINT32 id) 
{
	HD_RESULT ret;
	UINT32 isp_id = id;
	static IQT_SHADING_PARAM shading = {0};
	
	shading.id = isp_id;
	ret = vendor_isp_get_iq(IQT_ITEM_SHADING_PARAM, &shading);
	if (ret != HD_OK) {
		printf("vendor_isp_get_iq (IQT_ITEM_DPC_PARAM) fail\r\n");
		goto exit;
	}
	shading.shading.ecs_enable = 0;
	ret = vendor_isp_set_iq(IQT_ITEM_SHADING_PARAM, &shading);
	if (ret != HD_OK) {
		printf("vendor_isp_set_iq (IQT_ITEM_DPC_PARAM, enable (%d)) fail\r\n", (int)shading.shading.ecs_enable);
		goto exit;
	}

	exit:
	return ret;
}

static HD_RESULT enable_ca_la_skip_frame(UINT32 id)
{
	HD_RESULT ret;
	UINT32 isp_id = id;
	AWBT_CONVERGE converge = {0};

	AET_SKIP_LA skip_la = {0};
	// set ae skip frame
	skip_la.id = isp_id;
	skip_la.skip_la = 1;//AE_SKIP_FRAME_NUM; 
	ret = vendor_isp_set_ae(AET_ITEM_SKIP_LA, &skip_la);
	if (ret != HD_OK) {
		printf("vendor_isp_set_ae (AET_ITEM_SKIP_LA, skip (%d) frame) fail\r\n", (int)skip_la.skip_la);
		goto exit;
	}
	ret = vendor_isp_get_ae(AET_ITEM_SKIP_LA, &skip_la);
	if (ret != HD_OK) {
		printf("vendor_isp_get_ae (AET_ITEM_SKIP_LA) fail\r\n");
		goto exit;
	}
	printf("--[Param Confirm]AE skip frame = %d \n", (int)skip_la.skip_la);

	// set awb skip frame
	converge.id = isp_id;
	ret = vendor_isp_get_awb(AWBT_ITEM_CONVERGE, &converge);
	if (ret != HD_OK) {
		printf("vendor_isp_get_awb (AWBT_ITEM_CONVERGE) fail\r\n");
		goto exit;
	}
	converge.converge.skip_frame = AWB_SKIP_FRAME;
	ret = vendor_isp_set_awb(AWBT_ITEM_CONVERGE, &converge);
	if (ret != HD_OK) {
		printf("vendor_isp_set_awb (AWBT_ITEM_CONVERGE, skip (%d) frame) fail\r\n", (int)converge.converge.skip_frame);
		goto exit;
	}
	ret = vendor_isp_get_awb(AWBT_ITEM_CONVERGE, &converge);
	if (ret != HD_OK) {
		printf("vendor_isp_get_awb (AWBT_ITEM_CONVERGE) fail\r\n");
		goto exit;
	}
	printf("--[Param Confirm]AWB skip frame = %d \n", (int)converge.converge.skip_frame);
	
	exit:
	return ret;
}

static int ISP_init(void)
{
	int ret;
	AET_CFG_INFO cfg_info = {0};
	AWBT_CT_TO_CGAIN ct_to_cgain = {0};
	ISPT_C_GAIN c_gain = {0};
	ISPT_CT ct = {0};
	UINT32 prccfg = 0; 
	UINT32 ca_la_skip = 1;
	UINT32 dpc = 0;
	UINT32 ecs = 0;
	UINT32 i;
	UINT32 id;

	// vendor isp init
	vendor_isp_init();

	for (i = 0; i < g_sensor_num; i++) {

		(void)memset(&cfg_info, 0, sizeof(cfg_info));
		(void)memset(&ct_to_cgain, 0, sizeof(ct_to_cgain));
		(void)memset(&c_gain, 0, sizeof(c_gain));
		(void)memset(&ct, 0, sizeof(ct));
		
		cfg_info.id = SEN_VCAP_ID[i];
		ct_to_cgain.id = SEN_VCAP_ID[i];
		c_gain.id = SEN_VCAP_ID[i];
		ct.id = SEN_VCAP_ID[i];

		snprintf(cfg_info.path, AE_CFG_NAME_LENGTH - 1, "/mnt/app/isp/%s.cfg", sensor_info[SEL_SEN].isp_cfg_name);
		printf("sensor %d load %s \n", i, sensor_info[SEL_SEN].isp_cfg_name);

		vendor_isp_set_ae(AET_ITEM_RLD_CONFIG, &cfg_info);
		vendor_isp_set_awb(AWBT_ITEM_RLD_CONFIG, &cfg_info);
		vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);

		ct_to_cgain.ct_to_cgain.ct = 4700;
		vendor_isp_get_awb(AWBT_ITEM_CT_TO_CGAIN, &ct_to_cgain);

		c_gain.gain[0] = ct_to_cgain.ct_to_cgain.r_gain;
		c_gain.gain[1] = ct_to_cgain.ct_to_cgain.g_gain;
		c_gain.gain[2] = ct_to_cgain.ct_to_cgain.b_gain;
		vendor_isp_set_common(ISPT_ITEM_C_GAIN, &c_gain);

		ct.ct = 4700;
		vendor_isp_set_common(ISPT_ITEM_CT, &ct);
	}
	// disable GDC,CAC
	if (prccfg == 0) {
		/* Disable GDC/CAC */
		hd_common_sysconfig(0, (1<<16), 0, VENDOR_AI_CFG); //enable AI engine
		
		#if 0//defined(__LINUX)
		ldc.id = 0; //isp id = 0
		vendor_isp_get_iq(IQT_ITEM_LDC_PARAM, &ldc);
		ldc.ldc.geo_enable = 0;
		vendor_isp_set_iq(IQT_ITEM_LDC_PARAM, &ldc);
		#endif
		printf("[Low Power]Disable GDC/CAC\r\n");
	} else if (prccfg == 1) {
		/* Enable GDC/CAC */
		hd_common_sysconfig(HD_VIDEOPROC_CFG, 0, g_prccfg, 0); //default
	} else {
		printf("Invalid gdc value(%d). Set to Disable\r\n", (int)prccfg);
		hd_common_sysconfig(0, (1<<16), 0, VENDOR_AI_CFG); //enable AI engine
	}
	
	// set h26x svc mode
	if (g_svc == 0) {
		g_svc = HD_SVC_DISABLE;
	} else if (g_svc == 1) {
		g_svc = HD_SVC_2X;
		printf("[Low Power]Set SVC 2x Frame Mode.\r\n");
	} else if (g_svc == 2) {
		g_svc = HD_SVC_4X;
		printf("[Low Power]Set SVC 4x Frame Mode.\r\n");
	} else {
		printf("Invalid svc_mode value (%d). Set to SVC Disable\r\n", (int)g_svc);
		g_svc = HD_SVC_DISABLE;
	}

	for (i = 0; i < g_sensor_num; i++) {
		id = SEN_VCAP_ID[i];

		// set ca/la skip frame
		if (ca_la_skip == 1) {
			#if 1//defined(__LINUX)
			ret = enable_ca_la_skip_frame(id);
			if (ret != HD_OK) {
				printf("enable_ca_la_skip_frame fail=%d\n", ret);
				return HD_ERR_FAIL;
			}
			#endif
			printf("[Low Power]set ca/la skip frame.\r\n");
		}	
		
		// disable dpc
		if (dpc == 0) {
			ret = disable_dpc_func(id);
			if (ret != HD_OK) {
				printf("disable_dpc_func fail=%d\n", ret);
				return HD_ERR_FAIL;
			}
			printf("[Low Power]disable DPC.\r\n");
		}	
			
		// disable ecs
		if (ecs == 0) {
			ret = disable_ecs_func(id);
			if (ret != HD_OK) {
				printf("disable_ecs_func fail=%d\n", ret);
				return HD_ERR_FAIL;
			}
			printf("[Low Power]disable ECS.\r\n");
		}
	}
	
	return 0;
}

// Install driver in here becaue there has conflix between different sensor driver.
static void install_driver(void)
{
	char cmd[256];
	snprintf(cmd, sizeof(cmd), "insmod /lib/modules/5.10.168/hdal/sen_%1$s/nvt_sen_%1$s.ko sen_cfg_path=/mnt/app/sensor/sen_%1$s_538.cfg", sensor_info[SEL_SEN].name);
	printf("Insert sensor module : %s", cmd);
	system(cmd);
#if PD_USE_GPIO
	system("modprobe gpio_ext_irq.ko");
#endif
}

static void set_to_lowpower(void)
{
	////-------- DRAM Underclocking to 400 -------- //
	//printf(">> DRAM Underclocking from %d to 400 <<\r\n", LD_DDR_RATE);	
	//if(LD_DDR_RATE == 1066 || LD_DDR_RATE == 933 || LD_DDR_RATE == 793)
	//	system("echo w start 0 0 4 2 > /proc/ddr_slow_module/cmd");
	
	//--------  Set CPU freq  -------- //
	//printf(">> Set CPU freq to 960M HZ <<\r\n");
	//system("echo 960000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed");

	//--------  Close Ethernet  -------- //
	printf(">> Close Ethernet <<\r\n");
	system("rmmod ntkimethmac");
	}

static void set_to_normal(void)
{
	////-------- Raise the DRAM Speed to 933 MHz -------- //
	//printf(">> Raise the DRAM Speed from 400 to %d MHz <<\r\n", LD_DDR_RATE);
	//if(LD_DDR_RATE == 1066)
	//	system("echo w start 0 3 4 2 > /proc/ddr_slow_module/cmd");
	//else if(LD_DDR_RATE == 933)
	//	system("echo w start 0 2 4 2 > /proc/ddr_slow_module/cmd");
	//else if(LD_DDR_RATE == 793)
	//	system("echo w start 0 1 4 2 > /proc/ddr_slow_module/cmd");
	
	//--------  Set CPU freq  -------- //
	//printf(">> Set CPU freq to 800M HZ <<\r\n");
	//system("echo 800000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed");

	//-------- Resume Ethernet  -------- //
	printf(">> Resume Ethernet <<\r\n");
	system("sh /etc/init.d/S25_Net");
}

static void switch_system_mode(int mode)
{
	// Settings for NORMAL_MODE
	if(mode == NORMAL_MODE && SYS_STATUS == LOW_POWER_MODE)
	{
		printf(">>>>> Switch System Mode to NORMAL_MODE <<<<<\r\n");
		SYS_STATUS = NORMAL_MODE;
		set_to_normal();
		printf(">>>>> Switch to NORMAL_MODE Done <<<<<\r\n");
	}
	
	// Settings for LOW_POWER_MODE
	else if(mode == LOW_POWER_MODE && SYS_STATUS == NORMAL_MODE)
	{
		printf(">>>>> Switch System Mode to LOW_POWER_MODE <<<<<\r\n");
		SYS_STATUS = LOW_POWER_MODE;
		set_to_lowpower();
		printf(">>>>> Switch to LOW_POWER_MODE Done <<<<<\r\n");
		system("free");		// Check mem
	}
}

// UINT32 get_h26xfile_num(void)
// {
// 	FILE *f_out_nb;//
// 	char file_path_nb[32] = "/mnt/sd/1.txt";
// 	char str[100];
// 	UINT32 count=0,number=0,max_number=0;
// 	system("cd /mnt/sd;find -name \"dump_bs-*.dat\" > 1.txt");

// 	f_out_nb = fopen(file_path_nb,"rb");
// 	if(f_out_nb == NULL) {
//     	perror("open fail\n");
//     	return(-1);
// 	}
// 	while(1){	
// 		if( fgets (str, 100, f_out_nb)!=NULL ) {
// 		  /* output */
// 		  //printf("count%d,%s",count,str);
// 		  str[17] = '\n';
// 		  number = atoi(&str[11]);
// 		  //printf("nember %d\n",number);
// 		  if(number > max_number)
// 		  {
// 		  	max_number = number;
// 		  	//printf("max= %d\n",max_number);
// 		  }
// 		  count ++;
// 		}
// 		else
// 			break;
// 	}
// 	fclose(f_out_nb);
// 	printf("max= %d\n",max_number);
// 	return max_number;
// }

int PVD_ret_handle(int pvd_ret, int pvd_count)
{
	UINT32 i;
	if(pvd_ret == 1)
	{
		// If detected people
		printf(">P\r\n");
		pvd_count = PVD_THRESHOLD;
		set_frc(NORMAL_FPS);
		osg_blink(stream[0].enc_mask_path[0], 1);
		for (i = 0; i < g_sensor_num; i++) {
			sensor_wakeup(SEN_VCAP_ID[i]);
		}
		SENSOR_STATUS = SEN_STATUS_AWAKE;
		switch_system_mode(NORMAL_MODE);
	}
	else
	{
		if(pvd_count == 1)
		{
			// Back to Low power mode
			printf(">L<\r\n");
			if(SEN_SUPPORT_STREAMOUT_SLEEP)
			{
				for (i = 0; i < g_sensor_num; i++) {
					sensor_sleep(SEN_VCAP_ID[i]);
				}
				for (i = 0; i < g_sensor_num; i++) {
					wait_frmend(SEN_VCAP_ID[i]);
				}
				SENSOR_STATUS = SEN_STATUS_ASLEEP;
				set_frc(sensor_info[SEL_SEN].default_fps);
				switch_system_mode(LOW_POWER_MODE);
			}
			else
			{
				switch_system_mode(LOW_POWER_MODE);
				if(sensor_info[SEL_SEN].default_fps == 60)
					set_frc(sensor_info[SEL_SEN].default_fps - 20);
				else
					set_frc(sensor_info[SEL_SEN].default_fps);

				for (i = 0; i < g_sensor_num; i++) {
					wait_frmend(SEN_VCAP_ID[i]);
					sensor_sleep(SEN_VCAP_ID[i]);
				}
				SENSOR_STATUS = SEN_STATUS_ASLEEP;
			}
			set_powerdown(3);
			pvd_count --;
		}
		else if(pvd_count > 0)
		{
			if (pvd_count % 10 ==0)
				printf(">No P,%d<\r\n", pvd_count);
			pvd_count --;
		}
		osg_blink(stream[0].enc_mask_path[0], 0);
	}
	return pvd_count;
}

void Usage(void)
{
	unsigned int i = 0;
	printf("\033[1;37;40mUsage\033[0m \033[1;31;40m <sensor> <ddr_rate> \033[1;37;40m<mode> <enc_buf_size> <btn_save> <NORMAL_FPS> <bitrate> <md_threshold>\033[0m. \n");
	printf("Help:  ( <sensor> <ddr_rate> must be filled out. )\r\n");
	printf("\033[1;31;40m  <sensor> :\033[0m \r\n");
		printf(" ");
		for (i = 0; i < (sizeof(sensor_info) / sizeof(SENSOR_INFO)); i++) {
			printf("\033[1;31;40m %2d(%12s),\033[0m ", sensor_info[i].id, sensor_info[i].name);
		}
	printf("\n");
	printf("\033[1;31;40m  <ddr_rate> : 0(1066), 1(933), 2(793), 3(400) \033[0m \n");
	printf("  <mode> : 0(PVD only Mode), 1(Mix Mode, PVD + MD), 2(Mix Mode, but always skip PVD), 3(Mix Mode, but always do PVD).\r\n");
	printf("  <enc_buf_size>: Number for save h26x, if full then save to SD. 0(Disable save to SD).Default 4(MB)\r\n");
	printf("  <btn_save>: Button Save, 0(Disable), 1(Enable).Default 0.\r\n");
	printf("  <NORMAL_FPS>: fps number after people detect, defauit 12\r\n");
	printf("  <bitrate>: Number for encode bitrate, defauit 2(Mbps)\r\n");
	printf("  <md_threshold>: Number for MD threshold, defauit 5\r\n");
}

MAIN(argc, argv)
{
	int ret;
	int pthread_ret;
	int join_ret;
	pthread_t handle_cap_proc;
	pthread_t handle_enc;
	pthread_t handle_button;
	pthread_t handle_filesave;
	int policy;
	struct sched_param schedparam = {0};
    INT key;
	UINT32 i;
	printf("\n>>> AOV1 <<< (%s)\n\n", __DATE__ " - " __TIME__);
	
	if (argc < 3 || argc > 9) {
		Usage();
		return 0;
	}
	
	if (argc > 2) {
		SEL_SEN = atoi(argv[1]);
		switch(SEL_SEN)
		{
			case SEN_SEL_SC450AI:
				g_capbind = HD_VIDEOCAP_OUTFUNC_DIRECT;			// Direct Mode
				g_prccfg = HD_VIDEOPROC_CFG_STRIP_LV1;
				g_advPushRatio = 0;
				AE_SKIP_FRAME_NUM = 24;
				SEN_SUPPORT_STREAMOUT_SLEEP = TRUE;
				SEN_VCAP_ID[0] = 0;
				g_sensor_num = 1;
				break;

			case SEN_SEL_SC200AI:
				g_capbind = HD_VIDEOCAP_OUTFUNC_DIRECT;			// Direct Mode
				g_prccfg = HD_VIDEOPROC_CFG_STRIP_LV1;
				g_advPushRatio = 0;
				AE_SKIP_FRAME_NUM = 24;
				SEN_SUPPORT_STREAMOUT_SLEEP = TRUE;
				SEN_VCAP_ID[0] = 0;
				g_sensor_num = 1;
				break;
			case SEN_SEL_SC835HAI:
				g_capbind = HD_VIDEOCAP_OUTFUNC_DIRECT;			// Direct Mode
				g_prccfg = HD_VIDEOPROC_CFG_STRIP_LV1;
				g_advPushRatio = 0;
				AE_SKIP_FRAME_NUM = 8;
				SEN_SUPPORT_STREAMOUT_SLEEP = TRUE;
				SEN_VCAP_ID[0] = 0;
				g_sensor_num = 1;
				break;
			case SEN_SEL_OS04C10:
				g_capbind = HD_VIDEOCAP_OUTFUNC_DIRECT;			// Direct Mode
				g_prccfg = HD_VIDEOPROC_CFG_STRIP_LV1;
				g_advPushRatio = 0;
				AE_SKIP_FRAME_NUM = 24;
				SEN_SUPPORT_STREAMOUT_SLEEP = FALSE;
				SEN_VCAP_ID[0] = 0;
				g_sensor_num = 1;
				break;
			case SEN_SEL_OS04E10:
				g_capbind = HD_VIDEOCAP_OUTFUNC_DIRECT;			// Direct Mode
				g_prccfg = HD_VIDEOPROC_CFG_STRIP_LV1;
				g_advPushRatio = 0;
				AE_SKIP_FRAME_NUM = 16;
				SEN_SUPPORT_STREAMOUT_SLEEP = FALSE;
				SEN_VCAP_ID[0] = 0;
				g_sensor_num = 1;
				break;
			case SEN_SEL_IMX678:
				g_capbind = HD_VIDEOCAP_OUTFUNC_DIRECT;			// Direct Mode
				g_prccfg = HD_VIDEOPROC_CFG_STRIP_LV1;
				g_advPushRatio = 0;
				AE_SKIP_FRAME_NUM = 8;
				SEN_SUPPORT_STREAMOUT_SLEEP = FALSE;
				SEN_VCAP_ID[0] = 0;
				g_sensor_num = 1;
				break;
			case SEN_SEL_IMX415_DUAL:
				g_capbind = 0;									// Direct Mode
				g_prccfg = HD_VIDEOPROC_CFG_STRIP_LV1;
				g_advPushRatio = 0;
				AE_SKIP_FRAME_NUM = 24;
				SEN_SUPPORT_STREAMOUT_SLEEP = FALSE;
				SEN_VCAP_ID[0] = 0;
			#if MULTI_SENSOR
				SEN_VCAP_ID[1] = 2;
			#endif
				g_sensor_num = 2;
				break;
			default:
				printf("Sensor %d not support \n", SEL_SEN);
				Usage();
				return 0;
				break;
		}

		CURRENT_FPS = sensor_info[SEL_SEN].default_fps;
	
		LD_DDR_RATE = atoi(argv[2]);
		if( LD_DDR_RATE == 0)
			LD_DDR_RATE = 1066;
		else if( LD_DDR_RATE == 1)
			LD_DDR_RATE = 933;
		else if( LD_DDR_RATE == 2)
			LD_DDR_RATE = 793;
		else if( LD_DDR_RATE == 3)
			LD_DDR_RATE = 400;
		else
		{
			printf("[ERR] Input err, (%d)\n", LD_DDR_RATE);
			Usage();
			return 0;
		}	
	}
	if(argc > 3)
	{
		APP_MODE = atoi(argv[3]);
		if(APP_MODE == 0)
			printf("APP run with AI PVD only\n");
		else if(APP_MODE == 1)
			printf("APP run with AI PVD + MD\n");
		else if(APP_MODE == 2)
			printf("Mix Mode, but always skip PVD\n");
		else if(APP_MODE == 3)
			printf("Mix Mode, but always do PVD\n");
		else
		{
			printf("Input mode error %d!!\n", APP_MODE);
			Usage();
			return 0;
		}
	}
	if(argc > 4)
	{
		ENC_BUFF_SIZE = atoi(argv[4]);
		if(ENC_BUFF_SIZE == 0)
		{
			printf("Disable Save to SD card function\n");
			ENC_BUFF_SIZE = 1 * 1024 * 1024;
			EN_AUTO_SAVE = 0;
		}
		else
		{
			printf("Enable Save to SD card function, Buffer size = %d * 1024 * 1024\n", ENC_BUFF_SIZE);
			ENC_BUFF_SIZE = ENC_BUFF_SIZE * 1024 * 1024;
			EN_AUTO_SAVE = 1;
		}
	}
	if(argc > 5)
	{
		EN_BTN_SAVE = atoi(argv[5]);
		if(EN_BTN_SAVE != 1 )
		{
			EN_BTN_SAVE = 0;
			printf("Disable Button Save function\n");
		}
		else
			printf("Enable Button Save function\n");
	}
	if(argc > 6)
	{
        NORMAL_FPS = atoi(argv[6]);
        if(NORMAL_FPS <= 15 )
		{
                printf("NORMAL_FPS %d\n",NORMAL_FPS);
		}
		else
		{
                NORMAL_FPS = 12;
		}
	}
	if(argc > 7)
	{
		BITRATE = atoi(argv[7]);
		if(BITRATE <=32 )
			BITRATE = BITRATE*1024*1024;
		else
			BITRATE = 2*1024*1024;
	}
	if(argc > 8)
	{
		md_threshold = atoi(argv[8]);
		if(md_threshold <= 100 )
		{
			printf("md_threshold %d\n",md_threshold);
		}
		else
		{
			md_threshold = 5;
		}
	}
	
	{
		printf("  <SEL_SEN> : %d sensor : %s \n", SEL_SEN , sensor_info[SEL_SEN].name);
		printf("  <ddr_rate>: %d\r\n",LD_DDR_RATE);
        printf("  <mode> : %d\r\n",APP_MODE);
        printf("  <enc_buf_size>: %d Bytes\r\n",ENC_BUFF_SIZE);
        printf("  <btn_save>: %d\r\n",EN_BTN_SAVE);
		printf("  <NORMAL_FPS>: %d\r\n",NORMAL_FPS);
        printf("  <bitrate>: %d bps\r\n",BITRATE);
        printf("  <md_threshold>: %d\r\n",md_threshold);
    }
	
	{
		CHAR *model_name;

		if (HD_OK != ai_get_model_name(&model_name)) {
			(void)fprintf(stderr, "ai get model name failed\n");
			return 0;
		}
		//if (!check_exist("/proc/ddr_slow_module/cmd"))
		//{
		//	printf("File [%s] not exist, Exit AOV1...\r\n", "/proc/ddr_slow_module/cmd");
		//	return 0;
		//}
		if (!check_exist(model_name))
		{
			printf("File [%s] not exist, Exit AOV1...\r\n", model_name);
			return 0;
		}
	}
	
	// Init Flag
	exit_ai_thread = FALSE;
	start_ai_thread = FALSE;
	exit_encode_thread = FALSE;
	exit_filesave_thread = FALSE;
	ai_thread_flow_start = 0;
	stream[0].flag_enc_start = 0;
	stream[0].flag_md_pull = 1;
	
	// init hdal
	ret = hd_common_init(0);
	if (ret != HD_OK) {
		printf("[ERR] %s hd_common_init fail=%d\n", __func__, ret);
		return -1;
	}

	install_driver();
	ISP_init();

	// set project config for AI PVD
	hd_common_sysconfig(0, (1<<16), 0, VENDOR_AI_CFG); //enable AI engine

	// init memory
	ret = mem_init();
	if (ret != HD_OK) {
		printf("[ERR] %s mem_init fail=%d\n", __func__, ret);
		return -1;
	}

	//init gfx for AI PVD
	ret = hd_gfx_init();
	if (ret != HD_OK) {
		printf("hd_gfx_init fail\n");
		printf("[ERR] %s hd_gfx_init fail=%d\n", __func__, ret);
		return -1;
	}

	// call init
	{
		VENDOR_AI3_DEV_CFG dev_cfg = {0};

		ret = vendor_ai3_dev_init(&dev_cfg);
		if (ret != HD_OK) {
			printf("vendor_ai3_dev_init fail=%d\n", ret);
			return ret;
		}
	}

	UINT32 ai_buffer_total_size;
	NN_CFG_BUF_M ai_allbuf_mem = {0};
	if (HD_OK != ai_buffer_total_size_get(&ai_buffer_total_size)) {
		(void)fprintf(stderr, "get ai buffer size failed\n");
		return HD_ERR_NG;
	}
	ret = system_get_mem(DDR_ID, &(ai_allbuf_mem), (UINT32)ai_buffer_total_size, (INT32)0);
	if (ret != HD_OK) {
		printf("get pvd_buf fail=%d\n", ret);
		return ret;
	}

	ret = assign_ai_buf(&ai_allbuf_mem, &pvd_thread_parms);
	if (ret != HD_OK) {
		printf("ERR: ai buf assign fail (%d)!!\n", ret);
		return -1;
	}

	printf("pvdcnn_allbuf: pa=(%#x), va=(%#x), size=(%ld)\n", (unsigned int)pvd_thread_parms.pvd_mem.pa, (unsigned int)pvd_thread_parms.pvd_mem.va, pvd_thread_parms.pvd_mem.size);
	printf("limit_fdet_buf: pa=(%#x), va=(%#x), size=(%ld)\n", (unsigned int)pvd_thread_parms.limit_fdet_mem.pa, (unsigned int)pvd_thread_parms.limit_fdet_mem.va, pvd_thread_parms.limit_fdet_mem.size);

	// Create cap_proc
	pthread_ret = pthread_create(&handle_cap_proc, NULL, thread_cap_proc , (void *)stream);
	if (0 != pthread_ret) {
		printf("create thread_cap_proc failed, ret %d\r\n", pthread_ret);
		return -1;
	}
	if (0 != pthread_getschedparam(handle_cap_proc, &policy, &schedparam)) {
			printf("pthread_getschedparam failed\r\n");
	} else {
		schedparam.sched_priority = 20;
		pthread_setschedparam(handle_cap_proc, policy, &schedparam);
	}

    // Start vidoe encode
    // create encode_thread (pull_out bitstream)
    ret = pthread_create(&stream[0].enc_thread_id, NULL, encode_thread, (void *)stream);
    if (ret < 0) {
            printf("create encode thread failed");
            return -1;
    }

	// create AI PVD thread
	{
		ret = pthread_create(&nn_thread_id, NULL, nn_thread_api, (VOID *)(&pvd_thread_parms));
	    if (ret < 0) {
			printf("create process_thread failed");
			return -1;
		}
	}

	// Create enc
	pthread_ret = pthread_create(&handle_enc, NULL, thread_enc , (void *)stream);
	if (0 != pthread_ret) {
		printf("create handle_enc failed, ret %d\r\n", pthread_ret);
		return -1;
	}
	if (0 != pthread_getschedparam(handle_enc, &policy, &schedparam)) {
			printf("pthread_getschedparam failed\r\n");
	} else {
		schedparam.sched_priority = 19;
		pthread_setschedparam(handle_enc, policy, &schedparam);
	}
	
	// Create thread_filesave
	ret = filesave_init();
	if (ret != HD_OK) {
		printf("[ERR] %s filesave init failed: %d\n", __func__, ret);
		return -1;
	}
	for (i = 0; i < g_sensor_num; i++) {
		CHAR fs_id[32] = {0};
		snprintf(fs_id, sizeof(fs_id), "sensor%u", SEN_VCAP_ID[i]);
		stream[i].fs_ctx = filesave_get_serv(ENC_BUFF_SIZE, fs_id);
	}

	if(EN_BTN_SAVE==1)
	{
		// Create enc
		pthread_ret = pthread_create(&handle_button, NULL, thread_gpio_button , NULL);
		if (0 != pthread_ret) {
			printf("create handle_button failed, ret %d\r\n", pthread_ret);
			return -1;
		}
		if (0 != pthread_getschedparam(handle_button, &policy, &schedparam)) {
				printf("pthread_getschedparam failed\r\n");
		} else {
			schedparam.sched_priority = 20;
			pthread_setschedparam(handle_button, policy, &schedparam);
		}
	}

    // Join cap_proc
	pthread_ret = pthread_join(handle_cap_proc, (void *)&join_ret);
	if (0 != pthread_ret) {
		printf("thread_cap_proc pthread_join failed, ret %d\r\n", pthread_ret);
		return -1;
	}
	stream[0].flag_md_pull = 0;
    // Join enc
	pthread_ret = pthread_join(handle_enc, (void *)&join_ret);
	if (0 != pthread_ret) {
		printf("handle_enc pthread_join failed, ret %d\r\n", pthread_ret);
		return -1;
	}
	switch_system_mode(LOW_POWER_MODE);
	if(!SEN_SUPPORT_STREAMOUT_SLEEP && sensor_info[SEL_SEN].default_fps == 60)
	{
		set_frc(sensor_info[SEL_SEN].default_fps - 20);
	}
				
	// AE shall process AE_SKIP_FRAME_NUM frame for stable.
	//quick_ae();

	for (i = 0; i < g_sensor_num; i++) {
		wait_frmend(SEN_VCAP_ID[i]);
		sensor_sleep(SEN_VCAP_ID[i]);
	}
	SENSOR_STATUS = SEN_STATUS_ASLEEP;
	sleep(1);

	printf(">> Disable printk <<\r\n");
	system("echo 0 > /proc/sys/kernel/printk");
	
	system_powerdown();
	set_powerdown(1);
	SENSOR_STATUS = SEN_STATUS_AWAKE;
	
	
	ai_thread_flow_start = 1;
	stream[0].flag_enc_start = 1;
	int pull_ret = HD_ERR_FAIL;
	int pvd_count = 0;
	pvd_thread_parms.pvd_ret = 0;

	//long press until q print to quit.
	printf("Long press until q print to quit.\n");
	while (1)
	{
		
		check_t1 = hd_gettime_ms();

		if(pull_ret == HD_OK)
		{
			// Process previous frame
			wait_ai_thd_done = 1;		// Trigger AI
			wait_enc_thd_done = 1;		// Trigger Venc
		}
		else
		{
			// First run or frame pull fail
			printf("No frame, skip AI and Venc in this round.\r\n" );
			if(SEN_SUPPORT_STREAMOUT_SLEEP)
			{
				// Wait Frame end then sensor sleep
				for (i = 0; i < g_sensor_num; i++) {
					sensor_sleep(SEN_VCAP_ID[i]);
				}
				for (i = 0; i < g_sensor_num; i++) {
					wait_frmend(SEN_VCAP_ID[i]);
				}
				SENSOR_STATUS = SEN_STATUS_ASLEEP;
				// msleep((1 / sensor_info[SEL_SEN].default_fps) * 1000);
			}

		}

		if(SYS_STATUS == LOW_POWER_MODE)	
		{
			if(g_capbind == HD_VIDEOCAP_OUTFUNC_DIRECT)	// DRAM not support now
				quick_ae();
			if(!SEN_SUPPORT_STREAMOUT_SLEEP) // If sensor no support stream out sleep
			{
				msleep((1 / sensor_info[SEL_SEN].default_fps) * 1000);
				// Wait Frame end then sensor sleep
				for (i = 0; i < g_sensor_num; i++) {
					wait_frmend(SEN_VCAP_ID[i]);
					sensor_sleep(SEN_VCAP_ID[i]);
				}
				SENSOR_STATUS = SEN_STATUS_ASLEEP;
			}
		}
		
		// Wait thread done
		while ((wait_ai_thd_done == 1) || (wait_enc_thd_done == 1) ) {
			usleep(100);
		}

		// Process AI PVD result
		pvd_count = PVD_ret_handle(pvd_thread_parms.pvd_ret, pvd_count);

		// Pull current frame
		pull_ret = Vproc_Pull();

		// In Low Power Mode
		if(SYS_STATUS == LOW_POWER_MODE) {
			check_t5 = hd_gettime_ms();
			//printf("%d-%d\r\n", ai_frame_count, (int)(check_t5 - check_t1));
			//gpio_set_value(193, 0);
			system_powerdown();
			set_powerdown(1);
			//gpio_set_value(193, 1);
			SENSOR_STATUS = SEN_STATUS_AWAKE;	// system resume would wake up sensor, refer <sensor_name>.c

			if(SEN_SUPPORT_STREAMOUT_SLEEP) {
				for (i = 0; i < g_sensor_num; i++) {
					sensor_sleep(SEN_VCAP_ID[i]);
				}
				SENSOR_STATUS = SEN_STATUS_ASLEEP;
			}
		}
		else {
			// In Normal Mode
		}
		if(kbhit())
		{
			char ch = getche();
			if (ch =='q') { 
				printf(">> Exiting AOV1 ... <<\r\n");
				filesave_trigger();
				// trigger_filesave();
				printf(">> Wait file save <<\r\n");
				while(wait_filesave_done == 1) msleep(1);
				printf(">> file save done <<\r\n");
				system("sync");
				exit_filesave_thread = TRUE;
				//let encode_thread stop loop and exit
				stream[0].enc_exit = 1;
				exit_ai_thread = TRUE;
				exit_encode_thread = TRUE;
				
				break;
			}
		}
	}

	// destroy ai_turnkey
	pthread_join(nn_thread_id, NULL);

	// destroy encode thread
	pthread_join(stream[0].enc_thread_id, NULL);

	// stop video_record modules (main)
	for (int i = 0; i < SENSOR_MAX_NUM; i++) {
		hd_videoproc_stop(stream[i].proc_path);
		hd_videoproc_stop(stream[i].proc_pvd_path);
		hd_videocap_stop(stream[i].cap_path);
		hd_videoenc_stop(stream[i].enc_path);
	}

exit:
	ret = filesave_uninit();
	if (ret != HD_OK) {
		printf("[ERR] %s Filesave uninit failed = %d\n", __func__, ret);
	}

	// unint hd_gfx
	ret = hd_gfx_uninit();
	if (ret != HD_OK) {
		printf("[ERR] %s AI hd_gfx_uninit fail=%d\n", __func__, ret);
	}

	// close video_record modules (main)
	for (int i = 0; i < SENSOR_MAX_NUM; i++) {
		ret = close_module(&stream[i]);
		if (ret != HD_OK) {
			printf("[ERR] %s AI close_module fail=%d\n", __func__, ret);
		}
	}

	// uninit all modules
	ret = exit_module();
	if (ret != HD_OK) {
		printf("[ERR] %s AI exit_module fail=%d\n", __func__, ret);
	}

	// uninit memory
	ret = mem_exit();
	if (ret != HD_OK) {
		printf("[ERR] %s AI mem_exit fail=%d\n", __func__, ret);
	}
	
	// uninit hdal
	ret = hd_common_uninit();
	if (ret != HD_OK) {
		printf("[ERR] %s AI hd_common_uninit fail=%d\n", __func__, ret);
	}
	printf(">> Enable printk <<\r\n");
	system("echo 7 > /proc/sys/kernel/printk");
	set_to_normal();
	printf(">> Exit AOV1 <<\r\n");
	return 0;
}
