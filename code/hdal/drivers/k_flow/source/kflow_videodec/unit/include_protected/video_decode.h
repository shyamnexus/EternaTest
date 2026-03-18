/**
    Header file of video decoding codec library

    Exported header file of video decoding codec library.

    @file       VideoDecode.h
    @ingroup    mIAVCODEC
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/
#ifndef _VIDEODECODE_H
#define _VIDEODECODE_H

#include "dal_vdodec.h"  // TODO: use kdrv later
#include "kdrv_videodec/kdrv_videodec.h"
#include "kdrv_videojpeg/kdrv_videodec_jpeg.h"

#define MP_VDODEC_SHOW_MSG 				0

/**
    Definition of video pre-decode frame buffer number.
*/
//@{
#define MP_VDODEC_PREDEC_FRMNUM			3			///< Video Pre-decode Frame Number
//@}

typedef void (*MEDIAPLAY_REFFRMCB)(UINT32 pathID, uintptr_t uiYAddr, BOOL bIsRef);

/**
    Video Decoder ID
*/
typedef enum {
	MP_VDODEC_ID_1,                			///< video decoder ID 1
	MP_VDODEC_ID_2,                			///< video decoder ID 2
	MP_VDODEC_ID_3,                			///< video decoder ID 3
	MP_VDODEC_ID_4,                			///< video decoder ID 4
	MP_VDODEC_ID_5,                			///< video decoder ID 5
	MP_VDODEC_ID_6,                			///< video decoder ID 6
	MP_VDODEC_ID_7,                			///< video decoder ID 7
	MP_VDODEC_ID_8,                			///< video decoder ID 8
	MP_VDODEC_ID_9,                			///< video decoder ID 9
	MP_VDODEC_ID_10,               			///< video decoder ID 10
	MP_VDODEC_ID_11,               			///< video decoder ID 11
	MP_VDODEC_ID_12,               			///< video decoder ID 12
	MP_VDODEC_ID_13,               			///< video decoder ID 13
	MP_VDODEC_ID_14,               			///< video decoder ID 14
	MP_VDODEC_ID_15,               			///< video decoder ID 15
	MP_VDODEC_ID_16,               			///< video decoder ID 16
	MP_VDODEC_ID_17, 						///< video decoder ID 17
	MP_VDODEC_ID_18, 						///< video decoder ID 18
	MP_VDODEC_ID_19, 						///< video decoder ID 19
	MP_VDODEC_ID_20, 						///< video decoder ID 20
	MP_VDODEC_ID_21, 						///< video decoder ID 21
	MP_VDODEC_ID_22, 						///< video decoder ID 22
	MP_VDODEC_ID_23, 						///< video decoder ID 23
	MP_VDODEC_ID_24, 						///< video decoder ID 24
	MP_VDODEC_ID_25, 						///< video decoder ID 25
	MP_VDODEC_ID_26,						///< video decoder ID 26
	MP_VDODEC_ID_27,						///< video decoder ID 27
	MP_VDODEC_ID_28,						///< video decoder ID 28
	MP_VDODEC_ID_29,						///< video decoder ID 29
	MP_VDODEC_ID_30,						///< video decoder ID 30
	MP_VDODEC_ID_31,						///< video decoder ID 31
	MP_VDODEC_ID_32,						///< video decoder ID 32
	MP_VDODEC_ID_MAX,              			///< video decoder ID maximum
	ENUM_DUMMY4WORD(MP_VDODEC_ID)
} MP_VDODEC_ID;

/**
    type for CustomizeFunc() in MP_VDODEC_DECODER.
*/
//@{
typedef enum {
	MP_VDODEC_CUSTOM_DECODE1ST,						///< p1:(MP_VDODEC_1STV_INFO *) object(out)
	MP_VDODEC_CUSTOM_STREAMINIT,					///< p1:(MP_VDODEC_STREAM_INIT *) object(in)
	MP_VDODEC_CUSTOM_STREAMDECODE,					///< p1:(MP_VDODEC_STREAM_INFO *) object(in)
	MP_VDODEC_CUSTOM_STREAMWAITDONE,				///< p1:(UINT32 *) ER_Code(out)
	ENUM_DUMMY4WORD(MP_VDODEC_CUSTOM_TYPE)
} MP_VDODEC_CUSTOM_TYPE;
//@}

/**
    Video Decoding Slice Type
*/
typedef enum {
	MP_VDODEC_P_SLICE = 0,
	MP_VDODEC_B_SLICE = 1,
	MP_VDODEC_I_SLICE = 2,
	MP_VDODEC_IDR_SLICE = 3,
	MP_VDODEC_KP_SLICE = 4,
	ENUM_DUMMY4WORD(MP_VDODEC_SLICE_TYPE)
} MP_VDODEC_SLICE_TYPE;

#if defined(_BSP_NA51102_) || defined(_BSP_NS02302_)
/**
    Type of getting information from h26x video decoding codec library.
*/
typedef enum {
	MP_VDODEC_SUB_RATIO_DISABLE = 0,                ///< disable extra write output
	MP_VDODEC_SUB_RATIO_2x = 2,                     ///< enable, w : 1/2  h : 1/2
	MP_VDODEC_SUB_RATIO_4x = 4,                     ///< enable, w : 1/4  h : 1/4
	MP_VDODEC_SUB_RATIO_10_16x = 32,                ///< enable, w : 5/8  h : 5/8
	MP_VDODEC_SUB_RATIO_12_16x = 64,                ///< enable, w : 3/4  h : 3/4
	ENUM_DUMMY4WORD(MP_VDODEC_H26X_SUB_RATIO)
} MP_VDODEC_H26X_SUB_RATIO;

typedef enum {
	MP_VDODEC_INFO_H26XD_SET_INIT,
	MP_VDODEC_INFO_H26XD_SET_CB_FUNC,
	MP_VDODEC_INFO_H265D_SET_CABAC_BUF,
	MP_VDODEC_INFO_H265D_RELEASE_CABAC_BUF,
	MP_VDODEC_INFO_H26XD_GET_CHIP_INFO,            ///< KDRV_VDODEC_H26X_CHIP_INFO
	MP_VDODEC_INFO_H265D_GET_CABAC_NEED_BUF_SIZE,
	ENUM_DUMMY4WORD(MP_VDODEC_H26X_INFO_TYPE)
} MP_VDODEC_H26X_INFO_TYPE;
#endif

typedef KDRV_VDODEC_REFFRMCB MP_VDODEC_REFFRMCB;

/**
    Type of 1st video frame decode information.
*/
typedef struct {
	uintptr_t addr;                                 ///< [in] Video frame addr
	UINT32    size;                                 ///< [in] Video frame size
	uintptr_t decodeBuf;                            ///< [in] Output raw addr
	UINT32    decodeBufSize;                        ///< [in] Max size for outputing raw
	uintptr_t y_Addr;                               ///< [out] Output Y addr
	uintptr_t cb_Addr;                              ///< [out] Output cb addr
	uintptr_t cr_Addr;                              ///< [out] Output cr addr
	UINT32    width;                                ///< [out] Video frame width
	UINT32    height;                               ///< [out] Video frame height
	uintptr_t DescAddr;                             ///< [in] Description addr
	UINT32    DescLen;                              ///< [in] Description length
	UINT32    jpgType;                              ///< [out] Jpg type, JPG_FMT_YUV420 or others
	UINT32    decfrmnum;                            ///< [in] Decode Frame numeber
	uintptr_t frmaddr[MP_VDODEC_PREDEC_FRMNUM];     ///< [in] Video frames position
	UINT32    frmsize[MP_VDODEC_PREDEC_FRMNUM];     ///< [in] Video frames size
} MP_VDODEC_1STV_INFO;

/**
    Type defintion struture of video frame data YCbCr buffer information.
*/
typedef struct {
	uintptr_t yAddr;                                ///< Y address
	uintptr_t cbAddr;                               ///< CB address
	uintptr_t crAddr;                               ///< CR address
} MP_VDODEC_YCBCR_INFO;

/**
    Type defintion struture of video display setting information.
*/
typedef struct {
	UINT32    firstFramePos;                       ///<[in] First video frame position
	UINT32    firstFrameSize;                      ///<[in] First video frame size
	uintptr_t rawAddr;                             ///<[in] After decoded, Y addr (cb, cr counted)
	UINT32    rawSize;                             ///<[in] Size for YCbCr
	UINT32    rawWidth;                            ///<[in] Raw width
	UINT32    rawHeight;                           ///<[in] Raw height
	UINT32    rawType;                             ///<[in] Raw type, (jdcfg.fileformat)
	uintptr_t rawCbAddr;                           ///<[out] After decoded, cb addr
	uintptr_t rawCrAddr;                           ///<[out] After decoded, cr addr
	uintptr_t DescAddr;                            ///<[in] Description addr
	UINT32    DescLen;                             ///<[in] Description length
	UINT32    jpgType;                             ///<[in] Jpg type
} MP_VDODEC_DISPLAY_INFO;

/**
	Type defintion struture of video stream decoder init parameter.
*/
typedef struct {
	uintptr_t workbuf;                             ///<[in] workbuf Addr for codec H/W internal use
	UINT32    workbufsize;                         ///<[in] workbuf size for codec H/W internal use
	UINT32    rawWidth;                            ///<[in] Raw width
	UINT32    rawHeight;                           ///<[in] Raw height
	uintptr_t DescAddr;                            ///<[in] Description addr
	UINT32    DescLen;                             ///<[in] Description length
} MP_VDODEC_STREAM_INIT;

/**
	Type defintion struture of video stream decode parameter.
*/
typedef struct {
	uintptr_t bs_addr;                             ///<[in] bitstream addr
	UINT32    bs_size;                             ///<[in] bitstream size
	uintptr_t y_addr;                              ///<[out] After decoded, Y addr
	uintptr_t uv_addr;                             ///<[out] After decoded, UV addr
	UINT32    frameType;                           ///<[in] slice type for current decode frame
} MP_VDODEC_STREAM_INFO;

typedef struct {
	UINT16 align_w;                                ///< [in] reconstruct yuv align width
	UINT16 align_h;                                ///< [in] reconstruct yuv align height
} MP_VDODEC_RECYUV_WH;

/**
    Init structure
*/
typedef KDRV_VDODEC_INIT MP_VDODEC_INIT;

/**
    Decode structure
*/
typedef KDRV_VDODEC_PARAM MP_VDODEC_PARAM;

typedef KDRV_VDOJPGD_PARAM MP_VDOJPGD_PARAM;

typedef KDRV_VDODEC_MEM_INFO MP_VDODEC_MEMINFO;

typedef KDRV_VDODEC_GET_PARAM_ID MP_VDODEC_GETINFO_TYPE;

typedef KDRV_VDODEC_SET_PARAM_ID MP_VDODEC_SETINFO_TYPE;

#ifdef VDODEC_LL
typedef KDRV_CALLBACK_FUNC MP_VDODEC_CALLBACK_FUNC;
#endif

//#endif
/**
	Type defintion struture of video decoder object functions.
*/
//#if defined(_BSP_NA51000_) || defined(_BSP_NA51055_) || defined(_BSP_NA51089_)
//typedef struct {
//	ER(*Initialize)(MP_VDODEC_INIT *ptr);                                                       ///< initialize decoder
//	ER(*Close)(void);                                                                           ///< close decoder
//	ER(*GetInfo)(MP_VDODEC_GETINFO_TYPE type, VOID *pparam1, VOID *pparam2, VOID *pparam3);     ///< get info from codec library
//	ER(*SetInfo)(MP_VDODEC_SETINFO_TYPE type, ULONG param1, ULONG param2, ULONG param3);        ///< set parameters to codec library
//	ER(*DecodeOne)(UINT32 type, UINT32 *pSize, VOID *ptr);                                      ///< decoding one video frame and wait ready
//	ER(*TriggerDec)(MP_VDODEC_PARAM *ptr);                                                      ///< trigger decoding one video frame but not wait ready (only for direct path)
//	ER(*WaitDecReady)(MP_VDODEC_PARAM *ptr);                                                    ///< wait for decoding ready
//	ER(*AdjustBPS)(UINT32 type, UINT32 param1, UINT32 param2, UINT32 param3);                   ///< Not used
//	ER(*CustomizeFunc)(UINT32 type, void *pobj);                                                ///< customize function, if needed
//	UINT32 checkID;                                                                             ///< check ID for decoders
//} MP_VDODEC_DECODER, *PMP_VDODEC_DECODER;

//#elif defined(_BSP_NA51102_) || defined(_BSP_NS02302_)
typedef struct {
	ER(*Initialize)(void);                                                       ///< initialize decoder
	ER(*Close)(void);                                                                           ///< close decoder
	ER(*GetInfo)(MP_VDODEC_GETINFO_TYPE type, VOID *pparam1, VOID *pparam2, VOID *pparam3);     ///< get info from codec library
	ER(*SetInfo)(MP_VDODEC_SETINFO_TYPE type, ULONG param1, ULONG param2, ULONG param3);        ///< set parameters to codec library
	ER(*DecodeOne)(UINT32 type, VOID *pVidDecParam, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data); ///< decoding one video frame and wait ready
	ER(*TriggerDec)(VOID *ptr);                                                      ///< trigger decoding one video frame but not wait ready (only for direct path)
	ER(*WaitDecReady)(VOID *ptr);                                                    ///< wait for decoding ready
	ER(*AdjustBPS)(UINT32 type, UINT32 param1, UINT32 param2, UINT32 param3);                   ///< Not used
	ER(*CustomizeFunc)(UINT32 type, void *pobj);                                                ///< customize function, if needed
	UINT32 checkID;                                                                             ///< check ID for decoders
} MP_VDODEC_DECODER, *PMP_VDODEC_DECODER;

typedef struct {
	ER(*Initialize)(MP_VDODEC_INIT *ptr);             ///< initialize decoder
	ER(*Close)(void);                                       ///< close decoder
	ER(*GetInfo)(MP_VDODEC_GETINFO_TYPE type, VOID *pparam1, VOID *pparam2, VOID *pparam3);   ///< get info from codec library
	ER(*SetInfo)(MP_VDODEC_SETINFO_TYPE type, ULONG param1, ULONG param2, ULONG param3);      ///< set parameters to codec library
	ER(*DecodeOne)(UINT32 type, VOID *pVidDecParam, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data); ///< decoding one video frame and wait ready
	ER(*TriggerDec)(VOID *ptr);                                                      ///< trigger decoding one video frame but not wait ready (only for direct path)
	ER(*WaitDecReady)(VOID *ptr);                                                    ///< wait for decoding ready
	ER(*AdjustBPS)(UINT32 type, UINT32 param1, UINT32 param2, UINT32 param3);                   ///< Not used
	ER(*CustomizeFunc)(UINT32 type, void *pobj);                                                ///< customize function, if needed
	UINT32 checkID;                                                                             ///< check ID for decoders
} MP_VDODEC_H26X_DECODER, *PMP_VDODEC_H26X_DECODER;
//#endif
//@}

#endif //_VIDEODECODE_H

