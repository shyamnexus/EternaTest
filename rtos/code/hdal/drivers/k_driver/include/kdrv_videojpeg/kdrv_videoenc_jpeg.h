
/**
 * @file kdrv_videoenc_jpeg.h
 * @brief type definition of KDRV API.
 * @author ALG2
 * @date in the year 2018
 */

#ifndef __KDRV_VIDEOENC_JPEG_H__
#define __KDRV_VIDEOENC_JPEG_H__

#include "kdrv_type.h"

#define KDRV_VDOJPGE_ID_MAX		33
#define MAX_MASK_PAL_NUM		16

#define JPEGE_BS_SG_NUM			2	/* bs segment number */
#define JPEGE_BS_SG_SZ_MIN		1024

/*
#define KDRV_VDOJPEG_PREFIX          'J'
#define KDRV_VDOJPEG_ID(chip, chn)   ((KDRV_VDOJPEG_PREFIX<<16)|(((chip)&0xFF)<<8)|((chn)&0xFF))
#define KDRV_VDOJPEG(id)             (((id)>>16)&0xFF)
#define KDRV_VDOJPEG_CHIP(id)        (((id)>>8)&0xFF)
#define KDRV_VDOJPEG_MINOR(id)       ((id)&0xFF)
*/

typedef struct {
	INT32 (*callback)(VOID *callback_info, VOID *user_data);
	INT32 (*reserve_buf)(uintptr_t phy_addr, int ddr_id);
	INT32 (*free_buf)(uintptr_t phy_addr, int ddr_id);
} KDRV_JPEGENC_CALLBACK_FUNC;

typedef enum {
    KDRV_VDOJPGE_OK = 0,
    KDRV_VDOJPGE_FAIL = 0xffff
} KDRV_VDOJPGE_STATUS;

typedef struct {
	BOOL enable;
	uintptr_t data_addr;
	UINT32 data_length;
} KDRV_VDOJPGE_USER_DATA;

/*********** jpeg encode ************/
typedef enum {
	// KDRV_JPEGYUV_FORMAT_422,				///< [r/w] JPEG input format is YUV422
	// KDRV_JPEGYUV_FORMAT_420,				///< [r/w] JPEG input format is YUV420
	// KDRV_JPEGYUV_FORMAT_420MB			///< [r/w] JPEG input format is YUV420mb
	KDRV_JPEGYUV_FORMAT_422 = 0,			///< [r/w] JPEG input format is YUV422
	KDRV_JPEGYUV_FORMAT_420 = 1,			///< [r/w] JPEG input format is YUV420
	KDRV_JPEGYUV_FORMAT_100 = 4				///< [r/w] JPEG input format is YUV100
} KDRV_JPEGYUV_FORMAT;

typedef struct {
	UINT32 retstart_interval;				///< [r/w] JPEG restart interval. default: 0, range: 0~65536
	UINT32 encode_width;					///< [r/w] JPEG encode width.
	UINT32 encode_height;					///< [r/w] JPEG encode height.
	KDRV_JPEGYUV_FORMAT in_fmt;				///< [r/w] JPEG input YUV format.0: YUV422,1:YUV420
} KDRV_JPEG_INFO;

/************ source decompression  ************/
typedef struct {
	BOOL enable;							///< [r/w] y and cbcr decompression enable.
	UINT32 width;
	UINT32 height;
	UINT32 y_lofst;
	UINT32 c_lofst;
} KDRV_VDOJPGE_YCC;

/************ rate control  ************/
typedef enum
{
	KDRV_JPGE_VBR_PRI_NONE = 0,
	KDRV_JPGE_VBR_PRI_QUALITY = 1,
	KDRV_JPGE_VBR_PRI_FRAMERATE = 2,
	ENUM_DUMMY4WORD(KDRV_VDOJPGE_VBR_PRI_MODE)
} KDRV_VDOJPGE_VBR_PRI_MODE;

typedef struct {
	UINT32 enable;                      ///< [w] RC enable
	UINT32 base_qp;                     ///< [w] base qp
	UINT32 vbr_mode;                    ///< [w] 0: CBR, 1: VBR
	UINT32 min_quality;                 ///< [w] min quality
	UINT32 max_quality;                 ///< [w] max quality
	UINT32 target_rate;                 ///< [w] bit rate
	UINT32 frame_rate_base;             ///< [w] frame rate = frame_rate_base / frame_rate_incr
	UINT32 frame_rate_incr;             ///< [w] frame rate = frame_rate_base / frame_rate_incr
	KDRV_VDOJPGE_VBR_PRI_MODE vbr_priority; ///< [w] 0: no priority, 1: quality mode (quality > frame rate), 2: frame rate mode (frame rate > quality)
	UINT32 min_frame_rate;              ///< [w] min frame rate for setting vbr priority to drop frame
} KDRV_VDOJPGE_RC;

/************ encode one frame info *******************/
typedef struct {
    UINT32 job_id;
	UINT32 chn_id;										// channel id from flow
	uintptr_t y_addr;									///< [w] encoding y address
	uintptr_t c_addr;                                  	///< [w] encoding cb address
	UINT32 y_line_offset;                               ///< [w] Y line offset
	UINT32 c_line_offset;                              	///< [w] UV line offset
	UINT32 src_ddr_id;
	uintptr_t org_bs_addr_pa;							///< used by k-driver only. K-flow don't touch this field.

	uintptr_t bs_addr_va[JPEGE_BS_SG_NUM];				///< [w] 1st virtual address of output bitstream buffer
	// uintptr_t bs_addr_va2;							///< [w] 2nd virtual address of output bitstream buffer
	uintptr_t bs_addr_pa[JPEGE_BS_SG_NUM];				///< [w] 1st physical address of output bitstream buffer
	// uintptr_t bs_addr_pa2;							///< [w] 2nd physical address of output bitstream buffer
	UINT32 bs_buf_size[JPEGE_BS_SG_NUM];				///< [w] 1st bitstream buffer size
	// UINT32 bs_size_2;								///< [w] 2nd bitstream buffer size
	// uintptr_t bs_hdr_addr;							///< [w] header addr use bs_addr_va[0]
    UINT32 bs_hdr_size;                                 ///< [r] header size
	UINT32 bs_offset;

    UINT32 bs_ddr_id;
	UINT32 quality;                                   	///< [w] initial quality value for jpeg (1~100)
	UINT32 retstart_interval;							///< [w] JPEG restart interval. default: 0, range: 0~65536
	UINT32 encode_time;									///<  [==== RESERVED ===== ]HW encode time(unit: us)
	UINT32 encode_width;								///< [w] JPEG encode width.
	UINT32 encode_height;								///< [w] JPEG encode height.
	KDRV_JPEGYUV_FORMAT in_fmt;							///< [w] JPEG input YUV format.0: YUV422,1:YUV420
	UINT32 rotation_type;								///< [w] JPEG encode rotation. 1: Rotate counter-clockwise, 2: Rotate clockwise, 3: Rotate 180 degree clockwise
	uintptr_t user_data;
	/********** RC **********/
	UINT32 vbr_mode;                                    ///< [w] VBR mode setting
	UINT32 min_quality;                                 ///< [w] min quality value for jpeg (1~100)
	UINT32 max_quality;                                 ///< [w] max quality value for jpeg (1~100)
	UINT32 target_rate;                                 ///< [w] target bit rate (bytes)
	UINT32 frame_rate;                                  ///< [w] frame rate (base 1000)
	/********** RC **********/
	UINT32 fmt_trans_en;
	UINT32 engine_idx;
	BOOL   enc_slice_en;
	UINT32 sec_Width;									///< [r] Raw Image Width, This value should be 16-multiplier.
	UINT32 sec_Height;									///< [r] Raw Image Height, This value should be 8 or 16 multiplier, depends on output bs format.
	UINT32 slice_cnt;									///< [r] slice index
	BOOL uv_swap_en;									///< [W] jpeg source uv swap (0: yuv, 1:yvu)
	uintptr_t y_addr_pa;								///< [w] y physical address
	uintptr_t uv_addr_pa;								///< [w] uv physical address
	KDRV_VDOJPGE_YCC st_src_decompression;				///< [w] source decompression parameter
	UINT32 base_qp;										///< [r] base qp
	BOOL   jpeg_jfif_en;								///< [w] JPEG encode JFIF enable
	UINT32 errorcode;									///< [r] return error code (KDRV_VDOJPGE_ERRCODE)
} KDRV_VDOJPGE_PARAM;

typedef struct {
	KDRV_VDOJPGE_STATUS status;
	UINT32 job_id;
	UINT32 first_bs_len;
	UINT32 bs_len;
	UINT32 bs_offset;
	UINT32 base_qp;								///< [r] base qp
	UINT32 re_trigger_status;
	UINT32 errorcode;							///< [r] return error code (KDRV_VDOJPGE_ERRCODE)
} KDRV_VDOJPGE_RESULT;

typedef enum {
	KDRV_JPGE_BUFFER_FULL = 0xf1,				//buffer full
	KDRV_JPGE_TIMEOUT = 0xF3,
	KDRV_JPGE_START_ERR = 0xF5,
	KDRV_JPGE_SKIP_FRAME = 0xF7,
	ENUM_DUMMY4WORD(KDRV_VDOJPGE_ERRCODE)
} KDRV_VDOJPGE_ERRCODE;

typedef struct {
	UINT8  type;			// graph type. 0: argb1555, 4: 1 bit plaette, 5: 2 bit palette, 6: 4 bit palette
	UINT16 width;			// graph width
	UINT16 height;			// graph height
	UINT16 line_offset;		// line offset
	uintptr_t addr;			// graph address (virtual address)
	uintptr_t addr_pa;		// graph address (physical address)
    UINT32 ddr_id;
} KDRV_VDOJPGE_OSG_GRAP;

typedef struct {
	UINT8  mode;			// display mode. 0: graph, 1: mask, 2: mosaic
	UINT16 x_str;			// y start position (2-pixel align)
	UINT16 y_str;			// x start position (2-pixel align)
	UINT8  bg_alpha;		// background transparency
	UINT8  fg_alpha;		// foreground/mask transparency
	UINT8  mask_type;		// mask type. 0: solid mask, 1: hollow mask
	UINT8  mask_bd_size;	// mask hollow border size. 0: 2-pixel, 1: 4-pixel, 2: 6-pixel, 3: 8-pixel
	UINT8  mask_y[2];		// mask palette of Y0/Y1
	UINT8  mask_cb;			// mask palette of Cb
	UINT8  mask_cr;			// mask palette of Cr
	UINT8  mosaic_blk_sz;	// mosaic block size
} KDRV_VDOJPGE_OSG_DISP;

typedef struct {
	BOOL enable;		// colorkey enable
	BOOL alpha_en;		// colorkey alpha enable
	UINT8 alpha;		// colorkey alpha
	UINT8 red;			// colorkey red for blending
	UINT8 green;		// colorkey green for blending
	UINT8 blue;			// colorkey blue for blending
} KDRV_VDOJPGE_OSG_COLORKEY;

typedef struct {
	UINT32 overlap_type;
} KDRV_VDOJPGE_OSG_GLOBAL;

typedef struct {
	UINT8						layer_idx;
	UINT32						win_idx;
	BOOL						enable;
	BOOL						roi_invalid;
	KDRV_VDOJPGE_OSG_GRAP		st_grap;
	KDRV_VDOJPGE_OSG_DISP		st_disp;
	KDRV_VDOJPGE_OSG_COLORKEY	st_key;
	KDRV_VDOJPGE_OSG_GLOBAL		st_global;
} KDRV_VDOJPGE_OSG_WIN;

typedef struct {
	UINT8	rgb2yuv[3][3];
} KDRV_VDOJPGE_OSG_RGB;

typedef struct {
	UINT8 pal_idx;
	UINT8 alpha;
	UINT8 red;
	UINT8 green;
	UINT8 blue;
} KDRV_VDOJPGE_OSG_PAL;

/************ mask ************/
typedef struct {
	UINT32 pos_x;             ///< [r/w] mask window postion x. range: 0 ~ encode width - 1
	UINT32 pos_y;             ///< [r/w] mask window postion y. range: 0 ~ encode height - 1
} KDRV_VDOJPGE_MASK_POS;

typedef struct {
	UINT8 mask_idx;						///< [r/w] index of mask. default: 0, range: 0~7
	UINT8 enable;						///< [r/w] enable mask window. default: 0, range: 0~1 (0: disable, 1: enable)
	UINT8 mosaic_en;					///< [r/w] mask window de-identified method. range: 0~1 (0: orignial, 1: mosaic)
	UINT8 line_hit_opt;					///< [r/w] mask window line hit operation. 0: and(inside), 1: or(outside), 2: border, 3: single line or concave border, 4: full concave mask
	UINT32 alpha;						///< [r/w] mask window blending alpha. range : 0 ~ 256
	UINT8 alpha_type;					///< [r/w] mosaic: 0: alpha blending with source and mosaic, 1: alpha blending with source after mosiac and mask color
	KDRV_VDOJPGE_MASK_POS st_pos[4];	///< [r/w] mask window position configure settings.
	UINT8 pal_sel;
	UINT8 hit_width[4];
	UINT8 clockwise;
	UINT8 line_chk_th;
	UINT32 mask_hollow_type;			///< [r/w] mask window hollow or not. (0: solid, 1: hollow)
	UINT32 mask_hollow_size;
	UINT32 mask_width;
	UINT32 mask_height;
} KDRV_VDOJPGE_MASK_WIN;

typedef struct {
	UINT8 mosaic_blk_w;
	UINT8 mosaic_blk_h;
	UINT8 pal_y[MAX_MASK_PAL_NUM];
	UINT8 pal_cb[MAX_MASK_PAL_NUM];
	UINT8 pal_cr[MAX_MASK_PAL_NUM];
} KDRV_VDOJPGE_MASK_INIT;
/************ end of mask ************/

/************ ctg ************/
typedef struct {
	BOOL enable;						///< [r/w] color to gray enable
	BOOL color_to_gray;					///< [r/w] final image color to gray
	BOOL src_color_to_gray;				///< [r/w] source image colot to gray
} KDRV_VDOJPGE_GRAY;
/************ end of ctg ************/

/************ roi ************/
#define MAX_JPEG_ROI_NUM	11
typedef struct {
	UINT32 roi_pos_x;
	UINT32 roi_pos_y;
	UINT32 roi_width;
	UINT32 roi_height;
	UINT32 roi_enable;
	UINT32 roi_threshold;
} KDRV_VDOJPGE_ROI_PARAM;

typedef struct {
	KDRV_VDOJPGE_ROI_PARAM stRoiParam[MAX_JPEG_ROI_NUM];
} KDRV_VDOJPGE_ROI;
/************ end of roi ************/

/*********** start of pad ***********/
typedef struct {
	BOOL disable;		// disable padding when resolution is not 16x alignment
	UINT32 mode;		// reserved value
} KDRV_VDOJPGE_PAD;
/************ end of pad ************/


/* get number of chips/engines/channels in whole system */
typedef struct {
	UINT8 chip;		/* how many chips are supported */
	UINT8 engine;	/* how many engines are supported */
	UINT8 chan;		/* how many channels are supported */
} KDRV_VDOJPGE_CHIP_INFO;

typedef struct {
	UINT8 chip;		/* index */
	UINT8 engine; 	/* index */
	UINT32 putjob_cnt;
	UINT32 callback_cnt;	/* including callback_err_cnt */
	UINT32 callback_err_cnt;
	UINT32 enc_done_cnt;	/* all cases including timeout */
	UINT32 dec_done_cnt;	/* all cases including timeout */
	UINT32 enc_err_cnt;
	UINT32 dec_err_cnt;
	UINT32 kflow_error;
} KDRV_VDOJPGE_COUNTER;

typedef enum {
	VDOJPGE_SET_JPEG_FREQ,					///< set JPEG engine freq
	VDOJPGE_SET_JPEG_INFO,
	VDOJPGE_SET_JPEG_RC,					///< set JPEG rate control, value type is KDRV_VDOJPGE_RC
	VDOJPGE_SET_JPEG_CB_FUNC,				///< set JPEG callback function, value type is KDRV_JPEG_CALLBACK_FUNC
	VDOJPGE_SET_JPEG_PARAM,					///< set JPEG parameters, value type is KDRV_VDOJPGE_PARAM
	VDOJPGE_SET_JPEG_OSG_WIN,				///< set JPEG osg window, value type is KDRV_VDOJPGE_OSG_WIN
	VDOJPGE_SET_JPEG_OSG_RGB,				///< set JPEG osg rgb, value type is KDRV_VDOJPGE_OSG_RGB
	VDOJPGE_SET_JPEG_OSG_PAL,				///< set JPEG osg palette, value type is KDRV_VDOJPGE_OSG_PAL
	VDOJPGE_SET_JPEG_ROTATION,				//8
	VDOJPGE_SET_JPEG_MASK_INIT,				//9
	VDOJPGE_SET_JPEG_MASK_WIN,				//10
	VDOJPGE_SET_JPEG_GRAY,					//11
	VDOJPGE_SET_JPEG_USER_DATA,				//12
	VDOJPGE_SET_JPEG_STOP_JOB,				//13
	VDOJPGE_SET_JPEG_ROI,					//14
	VDOJPGE_SET_JPEG_OSG_GLOBAL,			//15 set JPEG osg global, value type is KDRV_VDOJPGE_OSG_WIN
	VDOJPGE_SET_JPEG_PAD,					//< set JPEG padding mode
	ENUM_DUMMY4WORD(KDRV_VDOJPGE_SET_PARAM_ID)
} KDRV_VDOJPGE_SET_PARAM_ID;

typedef enum {
	VDOJPGE_GET_JPEG_RC,					///< get JPEG rate control, value type is KDRV_VDOJPGE_RC
	VDOJPGE_GET_JPEG_OSG_WIN,				///< get JPEG osg window, value type is KDRV_VDOJPGE_OSG_WIN
	VDOJPGE_GET_JPEG_OSG_RGB,				///< get JPEG osg rgb, value type is KDRV_VDOJPGE_OSG_RGB
	VDOJPGE_GET_JPEG_OSG_PAL,				///< get JPEG osg palette, value type is KDRV_VDOJPGE_OSG_PAL
	VDOJPGE_GET_JPEG_FREQ, 					///< get JPEG engine frequency
	VDOJPGE_GET_CHIP_INFO,					///< get JPEG number of chip/engine/channel info without opening the channel,value type is KDRV_VDOJPGE_CHIP_INFO
	VDOJPGE_GET_COUNTER,					///< get JPEG per chip/engine counter, value type is KDRV_VDOJPGE_COUNTER
	ENUM_DUMMY4WORD(KDRV_VDOJPGE_GET_PARAM_ID)
} KDRV_VDOJPGE_GET_PARAM_ID;

extern KDRV_VDOJPGE_RESULT  g_enc_result;
/*!
 * @fn INT32 kdrv_videoenc_jpeg_open(UINT32 chip, UINT32 engine)
 * @brief open hardware engine
 * @param chip		the chip id of hardware
 * @param engine	the engine id of hardware
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_videoenc_jpeg_open(UINT32 chip, UINT32 engine);

/*!
 * @fn INT32 kdrv_videoenc_jpeg_close(UINT32 chip, UINT32 engine)
 * @brief close hardware engine
 * @param chip		the chip id of hardware
 * @param engine	the engine id of hardware
 * @return return 0 on success, -1 on error
 */
#if defined(__LINUX)
INT32 kdrv_videoenc_jpeg_close(UINT32 kdrv_id);
#else
INT32 kdrv_videoenc_jpeg_close(UINT32 chip, UINT32 engine);
#endif

/*!
 * @fn INT32 kdrv_videoenc_jpeg_trigger(KDRV_VDOJPGE_ID handler, KDRV_H26XENC_PARAM *p_enc_param,
								  VOID *p_cb_func,
								  VOID *user_data);
 * @brief trigger hardware engine
 * @param handler 				the handler of hardware
 * @param p_enc_param		 	the encode one frame settings
 * @param p_cb_func 			the callback function
 * @param user_data 			the private user data
 * @return return 0 on success, -1 on error
 */
// INT32 kdrv_videoenc_jpeg_trigger(UINT32 kdrv_id, KDRV_VDOJPGE_PARAM *p_enc_param, VOID *p_cb_func, VOID *p_user_data); //xvr
INT32 kdrv_videoenc_jpeg_trigger(UINT32 kdrv_id, KDRV_VDOJPGE_PARAM *p_enc_param, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data);

/*!
 * @fn INT32 kdrv_videoenc_jpeg_get(KDRV_VDOJPGE_ID handler, DEC_ID id, VOID *param)
 * @brief set parameters to hardware engine
 * @param handler	the handler of hardware
 * @param id 		the id of parameters
 * @param param 	the parameters
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_videoenc_jpeg_get(UINT32 kdrv_id, KDRV_VDOJPGE_GET_PARAM_ID param_id, VOID *param);

/*!
 * @fn INT32 kdrv_videoenc_jpeg_set(KDRV_VDOJPGE_ID handler, DEC_ID id, VOID *param)
 * @brief get parameters to hardware engine
 * @param handler	the handler of hardware
 * @param id 		the id of parameters
 * @param param 	the parameters
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_videoenc_jpeg_set(UINT32 kdrv_id, KDRV_VDOJPGE_SET_PARAM_ID param_id, VOID *param);

extern INT32 kdrv_videoenc_jpeg_open(UINT32 chip, UINT32 engine);
#if defined(__LINUX)
extern INT32 kdrv_videoenc_jpeg_close(UINT32 kdrv_id);
#else
extern INT32 kdrv_videoenc_jpeg_close(UINT32 chip, UINT32 engine);
#endif
// extern INT32 kdrv_videoenc_jpeg_trigger(UINT32 id, KDRV_VDOJPGE_PARAM *p_enc_param, VOID *p_cb_func, VOID *p_user_data); //xvr
extern INT32 kdrv_videoenc_jpeg_trigger(UINT32 kdrv_id, KDRV_VDOJPGE_PARAM *p_enc_param, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data);
extern INT32 kdrv_videoenc_jpeg_get(UINT32 kdrv_id, KDRV_VDOJPGE_GET_PARAM_ID param_id, VOID *param);
extern INT32 kdrv_videoenc_jpeg_set(UINT32 kdrv_id, KDRV_VDOJPGE_SET_PARAM_ID param_id, VOID *param);


#endif
