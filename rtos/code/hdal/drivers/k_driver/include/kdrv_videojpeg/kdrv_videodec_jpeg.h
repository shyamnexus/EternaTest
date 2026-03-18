/**
 * @file kdrv_videodec_jpeg.h
 * @brief type definition of KDRV API.
 * @author ALG2
 * @date in the year 2019
 */

#ifndef __KDRV_VIDEODEC_JPEG_H__
#define __KDRV_VIDEODEC_JPEG_H__

#include "kwrap/type.h"
#include "kdrv_type.h"
#if 0
#include "../kdrv_videodec/kdrv_videodec.h"

#else
#define JPEGD_BS_SG_NUM		2	/* bs segment number */
#define JPEGD_BS_SG_SZ_MIN	1024

typedef enum {
    KDRV_VDOJPGD_OK = 0,
    KDRV_VDOJPGD_FAIL = 0xffff
} KDRV_VDOJPGD_STATUS;
/*
typedef enum {
	KDRV_VDODEC_ID_1,							///< video decoder ID 1
	KDRV_VDODEC_ID_MAX,              			///< video decoder ID maximum
	ENUM_DUMMY4WORD(KDRV_VDODEC_ID)
} KDRV_VDODEC_ID;
*/
typedef enum {
	KDRV_VDOJPGD_YUV420 = 0,
	KDRV_VDOJPGD_YUV422,
	ENUM_DUMMY4WORD(KDRV_VDOJPGD_YUVFMT)
} KDRV_VDOJPGD_YUVFMT;

typedef enum {
    KDRV_JPGD_N_SUP_FMT		= 0xf1,		//not support format
    KDRV_JPGD_ERR_BS		= 0xf2,		//error bitstream
    KDRV_JPGD_ERR_PARAM     = 0xf3,
    KDRV_JPGD_BUF_N_ENOUGH	= 0xf4,		//buffer not enough
} KDRV_VDOJPGD_ERRCODE;

typedef struct {
	INT32 (*callback)(VOID *callback_info, VOID *user_data);
	INT32 (*reserve_buf)(UINT32 phy_addr, int ddr_id);
	INT32 (*free_buf)(UINT32 phy_addr, int ddr_id);
} KDRV_JPEGDEC_CALLBACK_FUNC;

/*********** decode init ***********/
/*
typedef struct {
	UINT32 width;								///<[w] decode image width
	UINT32 height;								///<[w] decode image heght
	UINT32 desc_addr;							///<[w] decode header address (sps + pps)
	UINT32 desc_size;							///<[w] decode header size
	UINT32 buf_addr;							///<[w] decoder internal buffer starting address
	UINT32 buf_size;							///<[w] decoder internal buffer size
	UINT32 display_width;						///<[w] display source width, e.g. 720 x 480, uiWidth = 736, uiDisplayWidth = 720
} KDRV_VDODEC_INIT;
*/

typedef struct {
	UINT16 align_w;								///< [r/w] reconstruct yuv align width
	UINT16 align_h;								///< [r/w] reconstruct yuv align height
} KDRV_VDOJPGD_RECYUV_WH;

/*********** decode one frame ***********/
typedef struct {
	UINT32 id;
	UINT32 bs_ddr_id;							///<[w] bit-stream ddr_id, new
	uintptr_t org_bs_addr_pa;					///< used by k-driver only. K-flow don't touch this field.
	UINT32 dst_ddr_id;							///<[w] dec out buffer ddr_id
	uintptr_t y_addr_va;                        ///<[w] raw y virtual address
	uintptr_t uv_addr_va;                       ///<[w] raw uv virtual address
	UINT32 raw_size;                            ///<[w] raw buffer size
	uintptr_t jpeg_hdr_addr;                    ///<[w] bit-stream header address (virtual address)
	UINT32 uiWidth;                             ///<[r] return decoded image width
	UINT32 uiHeight;                            ///<[r] return decoded image height
	UINT32 yuv_fmt;                             ///<[r] return decoded image format
	UINT32 errorcode;                           ///<[r] return error code (KDRV_VDOJPGD_ERRCODE)
	UINT32 fd;                                  ///<[w] record kflow fd
	UINT32 job_id;
	//KDRV_VDODEC_REFFRMCB vRefFrmCb;
	UINT32 engine_idx;
	uintptr_t y_addr_pa;						///<[w] y physical address
	uintptr_t uv_addr_pa;						///<[w] uv physical address
	uintptr_t bs_addr_pa[JPEGD_BS_SG_NUM];		///<[w] bs physical address
	uintptr_t bs_addr_va[JPEGD_BS_SG_NUM];		///<[w] bs virtual address
	UINT32 bs_buf_size[JPEGD_BS_SG_NUM];		///<[w] bs buffer size
} KDRV_VDOJPGD_PARAM;

/*
typedef struct {
	uintptr_t bs_addr_va;
	uintptr_t bs_addr_pa;
	uintptr_t bs_size;
	UINT32 shift_offset;
} KDRV_VDODEC_BS_INFO;

typedef struct {
	UINT32 hdr_bs_addr;		///< [input]  : decode sps/pps heeader biststream addr
	UINT32 hdr_bs_len;		///< [input]  : decode sps/pps heeader biststream size
	UINT32 width;			///< [output] : return width
	UINT32 height;			///< [output] : return height
	UINT32 size;			///< [output] : return memory requirement size
} KDRV_VDODEC_MEM_INFO;

typedef struct {
	BOOL   present_flag;													///< VUI present flag, 0 : vui not encoded, 1 : vui encoded and parameter return as below.
	UINT32 sar_width;                                                       ///< Horizontal size of the sample aspect ratio. default: 0, range: 0~65535
	UINT32 sar_height;                                                      ///< Vertical size of the sample aspect rat. default: 0, range: 0~65535
	UINT8 matrix_coef;                                                      ///< Matrix coefficients are used to derive the luma and Chroma signals from green, blue, and red primaries. default: 2, range: 0~255
	UINT8 transfer_characteristics;                                         ///< The opto-electronic transfers characteristic of the source pictures. default: 2, range: 0~255
	UINT8 colour_primaries;                                                 ///< Chromaticity coordinates the source primaries. default: 2, range: 0~255
	UINT8 video_format;                                                     ///< Indicate the representation of pictures. default: 5, range: 0~7
	UINT8 color_range;                                                      ///< Indicate the black level and range of the luma and Chroma signals. default: 0, range: 0~1 (0: Not full range, 1: Full range)
	BOOL timing_present_flag;                                               ///< timing info present flag. default: 0, range: 0~1 (0: disable, 1: enable)
} KDRV_VDODEC_VUI_INFO;
*/
typedef struct {
    KDRV_VDOJPGD_STATUS status;
    UINT32 job_id;

    UINT32 img_width;
	UINT32 img_height;
	UINT32 bg_width;
	UINT32 bg_height;
	KDRV_PIX_FMT img_fmt;
	UINT32 errorcode;                           ///<[r] return error code (KDRV_VDOJPGD_ERRCODE)
} KDRV_VDOJPGD_RESULT;
/* get number of chips/engines/channels in whole system */
typedef struct {
	UINT8 chip;
	UINT8 engine;
	UINT8 chan;
} KDRV_VDOJPGD_CHIP_INFO;

typedef enum {
	VDODEC_SET_JPEG_FREQ,						///< set JPEG engine frequency
	VDODEC_SET_JPEG_CLOSE,						///< set video decode channe close
	VDODEC_SET_JPEG_CB_FUNC,		         	///< set video decode callback
	VDODEC_SET_JPEG_STOP_JOB,
	VDODEC_SET_JPEG_DEC_Y_OUT_ONLY,            ///< set decode y out only (uv disable)
	ENUM_DUMMY4WORD(KDRV_VDOJPGD_SET_PARAM_ID)
} KDRV_VDOJPGD_SET_PARAM_ID;

typedef enum {
	VDODEC_GET_JPEG_FREQ, 						///< get JPEG engine frequency
	VDODEC_GET_JPEG_RECYUV_WH,					///< get jpeg align width/height
	VDODEC_GET_JPEG_BS_SHIFT,					///< get JPEG header size for 4-byte align, KDRV_VDODEC_BS_INFO
	VDODEC_GET_JPEG_INFO,
	VDODEC_GET_JPEG_CHIP_INFO,					///< get JPEG number of chip/engine/channel info without opening the channel,value type is KDRV_VDOJPGD_CHIP_INFO
	ENUM_DUMMY4WORD(KDRV_VDOJPGD_GET_PARAM_ID)
}KDRV_VDOJPGD_GET_PARAM_ID;

/*!
 * @fn INT32 kdrv_videodec_open(UINT32 chip, UINT32 engine)
 * @brief open hardware engine
 * @param chip		the chip id of hardware
 * @param engine	the engine id of hardware
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_videodec_jpeg_open(UINT32 chip, UINT32 engine);

/*!
 * @fn INT32 kdrv_videodec_close(UINT32 chip, UINT32 engine)
 * @brief close hardware engine
 * @param chip		the chip id of hardware
 * @param engine	the engine id of hardware
 * @return return 0 on success, -1 on error
 */
#if defined(__LINUX)
INT32 kdrv_videodec_jpeg_close(UINT32 kdrv_id);
#else
INT32 kdrv_videodec_jpeg_close(UINT32 chip, UINT32 engine);
#endif

/*!
 * @fn INT32 kdrv_videodec_trigger(KDRV_VDODEC_ID handler, KDRV_VDODEC_PARAM *p_dec_param,
								  USR_CALLBACK_FUNC *p_cb_func,
								  VOID *user_data);
 * @brief trigger hardware engine
 * @param handler 				the handler of hardware
 * @param p_enc_info		 	the decode one frame settings
 * @param p_cb_func 			the callback function
 * @param user_data 			the private user data
 * @return return 0 on success, -1 on error
 */
//INT32 kdrv_videodec_jpeg_trigger(UINT32 handle, VOID *p_param, VOID *p_user_priv);
//INT32 kdrv_videodec_jpeg_trigger(UINT32 id, KDRV_VDODEC_PARAM *p_dec_param, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data);
INT32 kdrv_videodec_jpeg_trigger(UINT32 id, KDRV_VDOJPGD_PARAM *p_dec_param, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data);

						  //KDRV_CALLBACK_FUNC *p_cb_func,
						  //VOID *p_user_data);

/*!
 * @fn INT32 kdrv_videodec_get(UINT32 handler, DEC_ID id, VOID *param)
 * @brief set parameters to hardware engine
 * @param handler	the handler of hardware
 * @param id 		the id of parameters
 * @param param 	the parameters
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_videodec_jpeg_get(UINT32 id, KDRV_VDOJPGD_GET_PARAM_ID parm_id, VOID *param);

/*!
 * @fn INT32 kdrv_videodec_set(KDRV_VDODEC_ID handler, DEC_ID id, VOID *param)
 * @brief get parameters to hardware engine
 * @param handler	the handler of hardware
 * @param id 		the id of parameters
 * @param param 	the parameters
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_videodec_jpeg_set(UINT32 id, KDRV_VDOJPGD_SET_PARAM_ID parm_id, VOID *param);
#endif	// use kdrv_videodec.h

extern INT32 kdrv_videodec_jpeg_open(UINT32 chip, UINT32 engine);
#if defined(__LINUX)
extern INT32 kdrv_videodec_jpeg_close(UINT32 kdrv_id);
#else
extern INT32 kdrv_videodec_jpeg_close(UINT32 chip, UINT32 engine);
#endif
extern INT32 kdrv_videodec_jpeg_trigger(UINT32 kdrv_id, KDRV_VDOJPGD_PARAM *p_dec_param, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data);
extern INT32 kdrv_videodec_jpeg_get(UINT32 kdrv_id, KDRV_VDOJPGD_GET_PARAM_ID parm_id, VOID *param);
extern INT32 kdrv_videodec_jpeg_set(UINT32 ikdrv_idd, KDRV_VDOJPGD_SET_PARAM_ID parm_id, VOID *param);

#endif
