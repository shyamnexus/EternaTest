/**
	@brief Sample code of video liveview with osg.\n

	@file video_liveview_with_osg.c

	@author YongChang Qui

	@ingroup mhdal

	@note This file is modified from video_liveview.c.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "hdal.h"
#include "hd_debug.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <time.h>

// platform dependent
#if defined(__LINUX)
#include <signal.h>
#include <pthread.h>			//for pthread API
#define MAIN(argc, argv) 		int main(int argc, char** argv)
#define GETCHAR()				getchar()
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/signal.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <kwrap/util.h>		//for sleep API
#define sleep(x)    			vos_util_delay_ms(1000*(x))
#define msleep(x)    			vos_util_delay_ms(x)
#define usleep(x)   			vos_util_delay_us(x)
#include <kwrap/examsys.h> 	//for MAIN(), GETCHAR() API
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(hd_video_liveview_with_osg, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define DEBUG_MENU 		1

#define CHKPNT			printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
#define DBGH(x)			printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x)			printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)
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

#define SEN1_VCAP_ID 0

// Font Config
#define FONT_FILE "/mnt/sd/arial.ttf"
#define FONT_SIZE 48
static FT_Library g_ft_library;
static FT_Face    g_ft_face;
static int        g_ft_initialized = 0;

// Logo Config - modifiable location
#define LOGO_FILE "/mnt/sd/logo.bin"
#define LOGO_POS_X  10      // X position from left (pixels)
#define LOGO_POS_Y  100     // Y position from top (pixels)
static unsigned short* g_logo_data = NULL;
static int g_logo_w = 0;
static int g_logo_h = 0;

// Privacy Mask Config (example: center of screen)
#define PRIVACY_MASK_ENABLED 1
#define USE_ISP_LEVEL_MASK 1   // 1 = VIDEOPROC level mask, 0 = Encoder-level mask
#define PRIVACY_MASK_X1  800
#define PRIVACY_MASK_Y1  400
#define PRIVACY_MASK_X2  1120
#define PRIVACY_MASK_Y2  680

///////////////////////////////////////////////////////////////////////////////

#define VOUT_IDE1  0x1
#define VOUT_IDE2  0x2

// static UINT32 g_ide = VOUT_IDE1;
// static UINT32 vout_fmt = HD_VIDEO_PXLFMT_YUV420;

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

#define SEN_OUT_FMT		HD_VIDEO_PXLFMT_RAW10
#define CAP_OUT_FMT		HD_VIDEO_PXLFMT_RAW10
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

#define VDO_SIZE_W		2960
#define VDO_SIZE_H		1664

#define STAMP_WIDTH   1000
#define STAMP_HEIGHT  200

///////////////////////////////////////////////////////////////////////////////

static int query_osg_buf_size(void)
{
	HD_VIDEO_FRAME frame = {0};
	int stamp_size;

	frame.sign   = MAKEFOURCC('O','S','G','P');
	frame.dim.w  = STAMP_WIDTH;
	frame.dim.h  = STAMP_HEIGHT;
	frame.pxlfmt = HD_VIDEO_PXLFMT_ARGB4444;

	//get required buffer size for a single image
	stamp_size = hd_common_mem_calc_buf_size(&frame);
	if(!stamp_size){
		printf("fail to query buffer size\n");
		return -1;
	}

	//ping pong buffer needs double size
	stamp_size *= 2;

	//ping pong buffer size must be 128 aligned
	stamp_size = ALIGN_CEIL(stamp_size, 128);

	return stamp_size;
}

static HD_RESULT mem_init(UINT32 stamp_size)
{
	HD_RESULT              ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

	if(!stamp_size){
		printf("size of osg buffer is unknown\n");
		return -1;
	}

	// config common pool (cap)
	mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[0].blk_size = DBGINFO_BUFSIZE()+VDO_RAW_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, CAP_OUT_FMT)
        													+VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
        													+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
	mem_cfg.pool_info[0].blk_cnt = 2;
	mem_cfg.pool_info[0].ddr_id = DDR_ID0;
	// config common pool (main)
	mem_cfg.pool_info[1].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[1].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(VDO_SIZE_W, VDO_SIZE_H, HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[1].blk_cnt = 3;
	mem_cfg.pool_info[1].ddr_id = DDR_ID0;

	// config common pool (osg)
	mem_cfg.pool_info[2].type = HD_COMMON_MEM_OSG_POOL;
	mem_cfg.pool_info[2].blk_size = stamp_size;
	mem_cfg.pool_info[2].blk_cnt = 1;
	mem_cfg.pool_info[2].ddr_id = DDR_ID0;

	ret = hd_common_mem_init(&mem_cfg);
	return ret;
}

static unsigned int mem_alloc(UINT32 stamp_size, HD_COMMON_MEM_VB_BLK *stamp_blk, UINTPTR *stamp_pa)
{
	UINTPTR                 pa;
	HD_COMMON_MEM_VB_BLK    blk;

	if(!stamp_size){
		printf("stamp_size is unknown\n");
		return -1;
	}

	//get osd stamps' block
	blk = hd_common_mem_get_block(HD_COMMON_MEM_OSG_POOL, stamp_size, DDR_ID0);
	if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("get block fail\r\n");
		return -1;
	}

	if(stamp_blk)
		*stamp_blk = blk;

	//translate stamp block to physical address
	pa = hd_common_mem_blk2pa(blk);
	if (pa == 0) {
		printf("blk2pa fail, blk = 0x%x\r\n", blk);
		return -1;
	}

	if(stamp_pa)
		*stamp_pa = pa;

	return 0;
}

static HD_RESULT mem_exit(void)
{
	HD_RESULT ret = HD_OK;
	hd_common_mem_uninit();
	return ret;
}

///////////////////////////////////////////////////////////////////////////////

/*
static HD_RESULT get_cap_caps(HD_PATH_ID video_cap_ctrl, HD_VIDEOCAP_SYSCAPS *p_video_cap_syscaps)
{
	HD_RESULT ret = HD_OK;
    HD_DEVCOUNT video_cap_dev = {0};

	ret = hd_videocap_get(video_cap_ctrl, HD_VIDEOCAP_PARAM_DEVCOUNT, &video_cap_dev);
	if (ret != HD_OK) {
		return ret;
	}
	printf("##devcount %d\r\n", video_cap_dev.max_dev_count);

	ret = hd_videocap_get(video_cap_ctrl, HD_VIDEOCAP_PARAM_SYSCAPS, p_video_cap_syscaps);
	if (ret != HD_OK) {
		return ret;
	}
	return ret;
}

static HD_RESULT get_cap_sysinfo(HD_PATH_ID video_cap_ctrl)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOCAP_SYSINFO video_cap_sysinfo={0};

	ret = hd_videocap_get(video_cap_ctrl, HD_VIDEOCAP_PARAM_SYSINFO, &video_cap_sysinfo);
	if (ret != HD_OK) {
		return ret;
	}
	printf("Cur FPS: %d\r\n", video_cap_sysinfo.cur_fps);

	return ret;
}
*/


static HD_RESULT set_cap_cfg(HD_PATH_ID *p_video_cap_ctrl)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
	HD_PATH_ID video_cap_ctrl = 0;
	HD_VIDEOCAP_CTRL iq_ctl = {0};

#if 1
	snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_gc5603");
	cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux =  0;  //use @0 in peri-dev.dtsi
	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(0, 0);
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = HD_VIDEOCAP_SEN_IGNORE;
#else
    snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_ar0237ir");
    printf("sen_ar0237ir Parallel Interface\n");
	cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_P_RAW;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux =  0;  //use @0 in peri-dev.dtsi
#endif
	ret = hd_videocap_open(0, HD_VIDEOCAP_CTRL(SEN1_VCAP_ID), &video_cap_ctrl); //open this for device control
	if (ret != HD_OK) {
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
		video_in_param.frc = HD_VIDEO_FRC_RATIO(30,1);
		video_in_param.dim.w = p_dim->w;
		video_in_param.dim.h = p_dim->h;
		video_in_param.pxlfmt = SEN_OUT_FMT;
		video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
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
		video_out_param.pxlfmt = CAP_OUT_FMT;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT, &video_out_param);
		//printf("set_cap_param OUT=%d\r\n", ret);
	}

	return ret;
}



// Load logo from binary file (called once)
static int load_logo(void)
{
    if (g_logo_data) return 0; // Already loaded
    
    FILE* f = fopen(LOGO_FILE, "rb");
    if (!f) {
        printf("Failed to open logo file: %s\n", LOGO_FILE);
        return -1;
    }
    
    unsigned int w, h;
    if (fread(&w, 4, 1, f) != 1 || fread(&h, 4, 1, f) != 1) {
        printf("Failed to read logo header\n");
        fclose(f);
        return -1;
    }
    
    g_logo_w = w;
    g_logo_h = h;
    g_logo_data = malloc(w * h * sizeof(unsigned short));
    if (!g_logo_data) {
        printf("Failed to allocate logo buffer\n");
        fclose(f);
        return -1;
    }
    
    if (fread(g_logo_data, 2, w * h, f) != w * h) {
        printf("Failed to read logo pixels\n");
        free(g_logo_data);
        g_logo_data = NULL;
        fclose(f);
        return -1;
    }
    
    fclose(f);
    printf("Logo loaded: %dx%d from %s\n", g_logo_w, g_logo_h, LOGO_FILE);
    return 0;
}

// Draw logo onto OSD buffer at configured position with black outline
static void draw_logo(unsigned short* buffer, int buf_w, int buf_h)
{
    if (!g_logo_data) {
        load_logo();
        if (!g_logo_data) return;
    }
    
    int start_x = LOGO_POS_X;
    int start_y = LOGO_POS_Y;
    
    // Thick outline offsets (2-pixel thick outline)
    static const int outline_offsets[][2] = {
        // Inner ring (1 pixel)
        {-1, -1}, {0, -1}, {1, -1},
        {-1,  0},          {1,  0},
        {-1,  1}, {0,  1}, {1,  1},
        // Outer ring (2 pixels)
        {-2, -2}, {-1, -2}, {0, -2}, {1, -2}, {2, -2},
        {-2, -1},                             {2, -1},
        {-2,  0},                             {2,  0},
        {-2,  1},                             {2,  1},
        {-2,  2}, {-1,  2}, {0,  2}, {1,  2}, {2,  2}
    };
    const int num_offsets = sizeof(outline_offsets) / sizeof(outline_offsets[0]);
    
    // First pass: Draw black outline
    for (int o = 0; o < num_offsets; o++) {
        int off_x = outline_offsets[o][0];
        int off_y = outline_offsets[o][1];
        
        for (int y = 0; y < g_logo_h; y++) {
            for (int x = 0; x < g_logo_w; x++) {
                unsigned short px = g_logo_data[y * g_logo_w + x];
                // Only draw outline for non-transparent pixels
                if ((px & 0xF000) != 0) {
                    int dst_x = start_x + x + off_x;
                    int dst_y = start_y + y + off_y;
                    
                    if (dst_x >= 0 && dst_x < buf_w && dst_y >= 0 && dst_y < buf_h) {
                        // Black with full alpha: 0xF000
                        buffer[dst_y * buf_w + dst_x] = 0xF000;
                    }
                }
            }
        }
    }
    
    // Second pass: Draw actual logo on top
    for (int y = 0; y < g_logo_h; y++) {
        for (int x = 0; x < g_logo_w; x++) {
            int dst_x = start_x + x;
            int dst_y = start_y + y;
            
            if (dst_x >= 0 && dst_x < buf_w && dst_y >= 0 && dst_y < buf_h) {
                unsigned short px = g_logo_data[y * g_logo_w + x];
                // Only draw if pixel has alpha (not transparent)
                if ((px & 0xF000) != 0) {
                    buffer[dst_y * buf_w + dst_x] = px;
                }
            }
        }
    }
}

// Internal function to draw a single character with specified color
static void draw_char_colored(unsigned short* buffer, int buf_w, int buf_h, 
                              int pen_x, int pen_y, FT_Bitmap* bitmap, 
                              unsigned short color) {
    int row, col;
    int p_x, p_y;
    unsigned char alpha;

    for (row = 0; row < bitmap->rows; row++) {
        for (col = 0; col < bitmap->width; col++) {
             p_x = pen_x + col;
             p_y = pen_y + row;

             // Boundary check
             if (p_x < 0 || p_x >= buf_w || p_y < 0 || p_y >= buf_h)
                 continue;

             alpha = bitmap->buffer[row * bitmap->pitch + col];
             
             // Only draw pixels with sufficient alpha
             if (alpha > 32) {
                 unsigned char a4 = alpha >> 4;
                 if (a4 > 0) {
                     // Combine alpha with color
                     unsigned short pixel = (a4 << 12) | (color & 0x0FFF);
                     buffer[p_y * buf_w + p_x] = pixel;
                 }
             }
        }
    }
}

// Smart font: Draw character with thick black outline and white fill
// This ensures visibility on both bright and dark backgrounds
static void draw_char(unsigned short* buffer, int buf_w, int buf_h, 
                      int pen_x, int pen_y, FT_Bitmap* bitmap) {
    // Color definitions (ARGB4444 without alpha, alpha added in draw function)
    const unsigned short COLOR_BLACK = 0x0000;  // R=0, G=0, B=0
    const unsigned short COLOR_WHITE = 0x0FFF;  // R=F, G=F, B=F
    
    // Thick outline offsets (2-pixel thick outline)
    static const int outline_offsets[][2] = {
        // Inner ring (1 pixel)
        {-1, -1}, {0, -1}, {1, -1},
        {-1,  0},          {1,  0},
        {-1,  1}, {0,  1}, {1,  1},
        // Outer ring (2 pixels)
        {-2, -2}, {-1, -2}, {0, -2}, {1, -2}, {2, -2},
        {-2, -1},                             {2, -1},
        {-2,  0},                             {2,  0},
        {-2,  1},                             {2,  1},
        {-2,  2}, {-1,  2}, {0,  2}, {1,  2}, {2,  2}
    };
    const int num_offsets = sizeof(outline_offsets) / sizeof(outline_offsets[0]);
    
    // First pass: Draw black outline by rendering the glyph at offset positions
    for (int i = 0; i < num_offsets; i++) {
        draw_char_colored(buffer, buf_w, buf_h,
                         pen_x + outline_offsets[i][0],
                         pen_y + outline_offsets[i][1],
                         bitmap, COLOR_BLACK);
    }
    
    // Second pass: Draw white text on top (center position)
    draw_char_colored(buffer, buf_w, buf_h, pen_x, pen_y, bitmap, COLOR_WHITE);
}

// Render dynamic timestamp to the buffer
static int update_osd_timestamp(unsigned short* buffer, int width, int height)
{
   time_t rawtime;
   struct tm * timeinfo;
   char time_str[32];
   int len, n, error;
   int pen_x = 10, pen_y = FONT_SIZE; // Base line position

   if (!g_ft_initialized) {
       printf("FreeType not initialized!\n");
       return -1;
   }

   // 1. Get current time
   time(&rawtime);
   timeinfo = localtime(&rawtime);
   strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
   len = strlen(time_str);

   // 2. Clear buffer (Transparent black)
   memset(buffer, 0, width * height * sizeof(unsigned short));

   // 3. Render string
   for (n = 0; n < len; n++) {
       // Load glyph
       error = FT_Load_Char(g_ft_face, time_str[n], FT_LOAD_RENDER);
       if (error) continue;

       // Draw to buffer
       // bitmap_top is distance from baseline to top of glyph. 
       // We want to draw at pen_y (baseline), so we subtract bitmap_top to get top-left y.
       draw_char(buffer, width, height, 
                 pen_x + g_ft_face->glyph->bitmap_left, 
                 pen_y - g_ft_face->glyph->bitmap_top, 
                 &g_ft_face->glyph->bitmap);

       // Advance pen
       pen_x += g_ft_face->glyph->advance.x >> 6;
   }
   
   // 4. Draw Logo
   draw_logo(buffer, width, height);
   
   return 0;
}

static int init_freetype(void)
{
    int error;

    error = FT_Init_FreeType(&g_ft_library);
    if (error) {
        printf("FT_Init_FreeType failed: %d\n", error);
        return -1;
    }

    error = FT_New_Face(g_ft_library, FONT_FILE, 0, &g_ft_face);
    if (error == FT_Err_Unknown_File_Format) {
        printf("Font format unsupported\n");
        return -1;
    } else if (error) {
        printf("Font file could not be opened: %s (err: %d). Please copy a .ttf file there.\n", FONT_FILE, error);
        return -1;
    }

    error = FT_Set_Pixel_Sizes(g_ft_face, 0, FONT_SIZE);
    if (error) {
        printf("FT_Set_Pixel_Sizes failed\n");
        return -1;
    }

    g_ft_initialized = 1;
    printf("FreeType Initialized successfully. Font: %s\n", FONT_FILE);
    return 0;
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
		return ret;

	if (p_max_dim != NULL ) {
		video_cfg_param.pipe = HD_VIDEOPROC_PIPE_RAWALL;
		video_cfg_param.isp_id = SEN1_VCAP_ID;
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

	video_ctrl_param.func = 0;
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
		video_path_config.max_mem.codec_type = HD_CODEC_TYPE_H265;
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
	return ret;
}

static HD_RESULT set_enc_param(HD_PATH_ID video_enc_path, HD_DIM *p_dim, UINT32 bitrate)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOENC_IN  video_in_param = {0};
	HD_VIDEOENC_OUT video_out_param = {0};
	HD_H26XENC_RATE_CONTROL rc_param = {0};

	if (p_dim != NULL) {
		video_in_param.dir     = HD_VIDEO_DIR_NONE;
		video_in_param.pxl_fmt = HD_VIDEO_PXLFMT_YUV420;
		video_in_param.dim.w   = p_dim->w;
		video_in_param.dim.h   = p_dim->h;
		video_in_param.frc     = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_IN, &video_in_param);
		if (ret != HD_OK) return ret;

        // H.265 Settings
        video_out_param.codec_type         = HD_CODEC_TYPE_H265;
        video_out_param.h26x.profile       = HD_H265E_MAIN_PROFILE; // Fixed enum name
        video_out_param.h26x.level_idc     = HD_H265E_LEVEL_5;
        video_out_param.h26x.gop_num       = 30; // 1 sec GOP
        video_out_param.h26x.ltr_interval  = 0;
        video_out_param.h26x.ltr_pre_ref   = 0;
        video_out_param.h26x.gray_en       = 0;
        video_out_param.h26x.source_output = 0;
        video_out_param.h26x.svc_layer     = HD_SVC_DISABLE;
        video_out_param.h26x.entropy_mode  = HD_H265E_CABAC_CODING;
        ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &video_out_param);
        if (ret != HD_OK) return ret;

        // Rate Control
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
        if (ret != HD_OK) return ret;
	}
	return ret;
}

static int set_stamp_param(HD_PATH_ID stamp_path, UINTPTR stamp_pa, UINT32 stamp_size, unsigned short *novatek_logo)
{
	HD_OSG_STAMP_BUF  buf;
	HD_OSG_STAMP_IMG  img;
	HD_OSG_STAMP_ATTR attr;

	if(!stamp_pa){
		printf("stamp buffer is not allocated\n");
		return -1;
	}
	if(stamp_pa & 0x3f){
		printf("stamp ping pong buffer(0x%x) is not 64 aligned\n", (unsigned long)stamp_pa);
		return -1;
	}

	memset(&buf, 0, sizeof(HD_OSG_STAMP_BUF));
	buf.type      = HD_OSG_BUF_TYPE_PING_PONG;
	buf.p_addr    = stamp_pa;
	buf.size      = stamp_size;

    // Use VIDEOENC param
    if (hd_videoenc_set(stamp_path, HD_VIDEOENC_PARAM_IN_STAMP_BUF, &buf) != HD_OK) {
		printf("fail to set stamp buffer\n");
		return -1;
	}

	memset(&img, 0, sizeof(HD_OSG_STAMP_IMG));
	img.fmt        = HD_VIDEO_PXLFMT_ARGB4444; // VPRC also supports this
	img.dim.w      = STAMP_WIDTH;
	img.dim.h      = STAMP_HEIGHT;
	img.p_addr     = (uintptr_t)novatek_logo;

	if(hd_videoenc_set(stamp_path, HD_VIDEOENC_PARAM_IN_STAMP_IMG, &img) != HD_OK){
		printf("fail to set stamp image\n");
		return -1;
	}

	memset(&attr, 0, sizeof(HD_OSG_STAMP_ATTR));
	attr.position.x = 0;
	attr.position.y = 0;
	attr.alpha      = 255;
	// attr.layer      = 0; // VPRC might not use layer/region in same way, preserving logic
	// attr.region     = 0;

	return hd_videoenc_set(stamp_path, HD_VIDEOENC_PARAM_IN_STAMP_ATTR, &attr);
}

static int set_enc_mask_param(HD_PATH_ID mask_path)
{
#if PRIVACY_MASK_ENABLED && !USE_ISP_LEVEL_MASK
    // Encoder-level mask (only affects this encoder/stream)
    HD_OSG_MASK_ATTR attr;

    memset(&attr, 0, sizeof(HD_OSG_MASK_ATTR));

    // Privacy Mask using configurable coordinates
    attr.position[0].x = PRIVACY_MASK_X1;
    attr.position[0].y = PRIVACY_MASK_Y1;
    attr.position[1].x = PRIVACY_MASK_X2;
    attr.position[1].y = PRIVACY_MASK_Y1;
    attr.position[2].x = PRIVACY_MASK_X2;
    attr.position[2].y = PRIVACY_MASK_Y2;
    attr.position[3].x = PRIVACY_MASK_X1;
    attr.position[3].y = PRIVACY_MASK_Y2;
    attr.type          = HD_OSG_MASK_TYPE_SOLID;
    attr.alpha         = 255;
    attr.color         = 0xFF000000; // Opaque Black

    printf("Setting ENCODER-level privacy mask at (%d,%d)-(%d,%d) - THIS STREAM ONLY\n",
           PRIVACY_MASK_X1, PRIVACY_MASK_Y1, PRIVACY_MASK_X2, PRIVACY_MASK_Y2);
    return hd_videoenc_set(mask_path, HD_VIDEOENC_PARAM_IN_MASK_ATTR, &attr);
#else
    (void)mask_path;
    return HD_OK;
#endif
}

#if USE_ISP_LEVEL_MASK && PRIVACY_MASK_ENABLED
static int set_isp_mask_param(HD_PATH_ID isp_mask_path)
{
    HD_OSG_MASK_ATTR attr;

    if (!isp_mask_path) {
        printf("VPROC mask path not available\n");
        return -1;
    }

    memset(&attr, 0, sizeof(HD_OSG_MASK_ATTR));

    // Privacy Mask at VIDEOPROC level - global-like
    attr.position[0].x = PRIVACY_MASK_X1;
    attr.position[0].y = PRIVACY_MASK_Y1;
    attr.position[1].x = PRIVACY_MASK_X2;
    attr.position[1].y = PRIVACY_MASK_Y1;
    attr.position[2].x = PRIVACY_MASK_X2;
    attr.position[2].y = PRIVACY_MASK_Y2;
    attr.position[3].x = PRIVACY_MASK_X1;
    attr.position[3].y = PRIVACY_MASK_Y2;
    attr.type          = HD_OSG_MASK_TYPE_SOLID;
    attr.alpha         = 255;
    attr.color         = 0xFF000000; // Opaque Black

    printf("Setting VIDEOPROC-level mask at (%d,%d)-(%d,%d)\n",
           PRIVACY_MASK_X1, PRIVACY_MASK_Y1, PRIVACY_MASK_X2, PRIVACY_MASK_Y2);
    
    HD_RESULT ret = hd_videoproc_set(isp_mask_path, HD_VIDEOPROC_PARAM_IN_MASK_ATTR, &attr);
    if (ret != HD_OK) {
        printf("VPROC mask set returned: %d\n", ret);
    } else {
        printf("VIDEOPROC mask set successfully!\n");
    }
    return HD_OK;
}
#endif

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

	HD_DIM  enc_max_dim;
	HD_DIM  enc_dim;

	// (3)
	HD_VIDEOENC_SYSCAPS enc_syscaps; // Changed from out to enc
	HD_PATH_ID enc_path;             // Changed from out to enc

	// osg
	HD_PATH_ID stamp_path;
	HD_PATH_ID mask_path;

	// ISP-level mask (global for all streams)
#if USE_ISP_LEVEL_MASK
	HD_PATH_ID isp_mask_path;  // VIDEOCAP level mask - applies to ALL streams
#endif

	HD_COMMON_MEM_VB_BLK stamp_blk;
	UINTPTR stamp_pa;
	UINT32 stamp_size;
	unsigned short *novatek_logo;

} VIDEO_LIVEVIEW;

static HD_RESULT init_module(void)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_init()) != HD_OK) return ret;
	if ((ret = hd_videoproc_init()) != HD_OK) return ret;
	if ((ret = hd_videoenc_init()) != HD_OK) return ret; // Changed to enc
	return HD_OK;
}

static HD_RESULT open_module(VIDEO_LIVEVIEW *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;
	// set videocap config
	ret = set_cap_cfg(&p_stream->cap_ctrl);
	if (ret != HD_OK) return HD_ERR_NG;

	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim);
	if (ret != HD_OK) return HD_ERR_NG;

    // Open Main Path
	if ((ret = hd_videocap_open(HD_VIDEOCAP_IN(SEN1_VCAP_ID, 0), HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), &p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &p_stream->proc_path)) != HD_OK)
		return ret;

    // Open Stamp on VIDEOENC 0
    if((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_STAMP_0, &p_stream->stamp_path)) != HD_OK)
        return ret;

    // Open Mask on VIDEOENC 0
    if((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_MASK_0, &p_stream->mask_path)) != HD_OK)
        return ret;

#if USE_ISP_LEVEL_MASK && PRIVACY_MASK_ENABLED
    // Open VIDEOPROC-level mask (global-like, before encoding)
    // Note: VIDEOCAP mask not supported on this platform
    printf("Opening VIDEOPROC-level mask (ISP-like, global)\n");
    if((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_MASK_0, &p_stream->isp_mask_path)) != HD_OK) {
        printf("Warning: Failed to open VPROC mask path: %d\n", ret);
        p_stream->isp_mask_path = 0;
    }
#endif

    // Use H.265 Encoder 0
    if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_VIDEOENC_0_OUT_0, &p_stream->enc_path)) != HD_OK)
        return ret;

    // Set Encoder Config
    ret = set_enc_cfg(p_stream->enc_path, &p_stream->enc_max_dim, 4*1024*1024); // 4Mbps
    if (ret != HD_OK) return HD_ERR_NG;

	return HD_OK;
}

static HD_RESULT close_module(VIDEO_LIVEVIEW *p_stream)
{
	// HD_RESULT ret;
#if USE_ISP_LEVEL_MASK && PRIVACY_MASK_ENABLED
	if (p_stream->isp_mask_path) {
		hd_videoproc_stop(p_stream->isp_mask_path);
		hd_videoproc_close(p_stream->isp_mask_path);
	}
#endif
	hd_videocap_close(p_stream->cap_path);
	hd_videoproc_close(p_stream->proc_path);
    hd_videoenc_close(p_stream->enc_path); // Close Enc
	hd_videoenc_close(p_stream->stamp_path); // Close Stamp
    hd_videoenc_close(p_stream->mask_path);  // Close Mask
	return HD_OK;
}

static HD_RESULT exit_module(void)
{
	hd_videocap_uninit();
	hd_videoproc_uninit();
	hd_videoenc_uninit(); // Uninit Enc
	return HD_OK;
}

//load a stamp(novatek logo)
static int init_novatek_logo(unsigned short *novatek_logo, int idx)
{
    // Replaced with dynamic FreeType rendering
    if (!g_ft_initialized) {
        if (init_freetype() != 0) {
            return -1;
        }
    }
    // Initial render
    update_osd_timestamp(novatek_logo, STAMP_WIDTH, STAMP_HEIGHT);
	return 0;
}

/*
static void show_help(void)
{
	printf("usage:\n");
	printf(" arg1: out_type => 0:cvbs, 1:lcd, 2:hdmi\n");
	printf(" arg2: hdmi_id  => 6:640x480, 8:1280x720, 10:1920x1080\n");
}
*/


MAIN(argc, argv)
{
	HD_RESULT ret;
	VIDEO_LIVEVIEW stream[1] = {0}; //0: main stream



	//allocate buffer for OSD
	stream[0].novatek_logo = malloc(STAMP_WIDTH * STAMP_HEIGHT * sizeof(unsigned short));
	if(!stream[0].novatek_logo){
		printf("fail to allocate logo buffer\n");
		return -1;
	}

	// init stamp data
	stream[0].stamp_blk  = 0;
	stream[0].stamp_pa   = 0;
	stream[0].stamp_size = query_osg_buf_size();
	if(stream[0].stamp_size <= 0){
		printf("query_osg_buf_size() fail\n");
		goto exit_free;
	}

	// init hdal
	ret = hd_common_init(0);
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
		goto exit_free;
	}

	// init memory
	ret = mem_init(stream[0].stamp_size);
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
		goto exit_hdal;
	}

    // Set configuration for RTSP Stream (Headless)
    hd_common_sysconfig(HD_VIDEOPROC_CFG, 0, HD_VIDEOPROC_CFG_STRIP_LV2, 0);

	// init all modules
	ret = init_module();
	if (ret != HD_OK) {
		printf("init fail=%d\n", ret);
		goto exit_mem;
	}

    // Configuration for MAX DIM
    stream[0].cap_dim.w = VDO_SIZE_W;
    stream[0].cap_dim.h = VDO_SIZE_H;
    stream[0].proc_max_dim.w = VDO_SIZE_W;
    stream[0].proc_max_dim.h = VDO_SIZE_H;
    stream[0].enc_max_dim.w = VDO_SIZE_W;
    stream[0].enc_max_dim.h = VDO_SIZE_H;
    
    // Encoder Output Dimension (Downscale to 1080p)
    stream[0].enc_dim.w = 1920;
    stream[0].enc_dim.h = 1080;

	// open video_liveview modules (main)
	ret = open_module(&stream[0], &stream[0].proc_max_dim);
	if (ret != HD_OK) {
		printf("open fail=%d\n", ret);
		goto exit_module;
	}

	// set cap parameter (main)
	ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim);
	if (ret != HD_OK) {
		printf("set cap parameter fail=%d\n", ret);
		goto exit_close;
	}

	// set proc parameter (main)
	ret = set_proc_param(stream[0].proc_path, &stream[0].enc_dim);
	if (ret != HD_OK) {
		printf("set proc parameter fail=%d\n", ret);
		goto exit_close;
	}

	// set enc parameter (main)
	ret = set_enc_param(stream[0].enc_path, &stream[0].enc_dim, 4000*1000); // 4Mbps
	if (ret != HD_OK) {
		printf("set enc parameter fail=%d\n", ret);
		goto exit_close;
	}

	// set stamp parameter
	ret = mem_alloc(stream[0].stamp_size, &stream[0].stamp_blk, &stream[0].stamp_pa);
	if (ret < 0) {
		printf("mem_alloc fail\n");
		goto exit_close;
	}
	// Initial FreeType / Logo load
    init_novatek_logo(stream[0].novatek_logo, 0);

    // Apply Stamp to VPRC
	ret = set_stamp_param(stream[0].stamp_path, stream[0].stamp_pa, stream[0].stamp_size, stream[0].novatek_logo);
	if (ret < 0) {
		printf("set stamp parameter fail\n");
		goto exit_close;
	}

    // set mask parameter
    ret = set_enc_mask_param(stream[0].mask_path);
    if (ret != HD_OK) {
        printf("set encoder mask parameter fail\n");
        // Don't exit, just warn
    }

#if USE_ISP_LEVEL_MASK && PRIVACY_MASK_ENABLED
    // Set ISP-level mask (global for all streams)
    ret = set_isp_mask_param(stream[0].isp_mask_path);
    if (ret != HD_OK) {
        printf("set ISP mask parameter fail: %d\n", ret);
        // Don't exit, just warn
    }
#endif

    // Bind: Cap -> Proc -> Enc
    hd_videocap_bind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0), HD_VIDEOPROC_0_IN_0);
    hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOENC_0_IN_0);

    // Start
	hd_videocap_start(stream[0].cap_path);
	hd_videoproc_start(stream[0].proc_path);
	hd_videoenc_start(stream[0].stamp_path); // Start Stamp on VENC
    hd_videoenc_start(stream[0].mask_path);  // Start Mask
	hd_videoenc_start(stream[0].enc_path);    // Start Enc

#if USE_ISP_LEVEL_MASK && PRIVACY_MASK_ENABLED
    // Start VIDEOPROC-level mask
    if (stream[0].isp_mask_path) {
        hd_videoproc_start(stream[0].isp_mask_path);
        printf("VIDEOPROC-level mask STARTED!\n");
    }
#endif

    printf("RTSP Streaming started with OSD!\n");

	// Start RTSP Server
	printf("Starting nvtrtspd_ipc...\n");
	system("nvtrtspd_ipc &");
	sleep(1);


    // Main Loop (Timestamp update)
    while(1) {
        if (g_ft_initialized) {
            update_osd_timestamp(stream[0].novatek_logo, STAMP_WIDTH, STAMP_HEIGHT);
            // Update OSD content to hardware
            set_stamp_param(stream[0].stamp_path, stream[0].stamp_pa, stream[0].stamp_size, stream[0].novatek_logo);
        }
        sleep(1);
    }

exit_close:
	ret = close_module(&stream[0]);
    // Unbind chain
    hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_0);
    hd_videocap_unbind(HD_VIDEOCAP_OUT(SEN1_VCAP_ID, 0));

exit_module:
	ret = exit_module();

exit_mem:
    // Release memory
	if (stream[0].stamp_blk != HD_COMMON_MEM_VB_INVALID_BLK)
		hd_common_mem_release_block(stream[0].stamp_blk);
	ret = mem_exit();

exit_hdal:
	ret = hd_common_uninit();

exit_free:
	if (stream[0].novatek_logo)
		free(stream[0].novatek_logo);

	return 0;
}
