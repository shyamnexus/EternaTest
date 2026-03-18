/**
	@brief Header file of vendor videoenc module.\n
	This file contains the functions which is related to vendor videoenc.

	@file vendor_videoenc.h

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _VENDOR_VIDEOENC_H_
#define _VENDOR_VIDEOENC_H_

#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/

/********************************************************************
	MACRO FUNCTION DEFINITIONS
********************************************************************/

/********************************************************************
	TYPE DEFINITION
********************************************************************/
typedef enum {
	VENDOR_VIDEOENC_PADDING_MODE_ZERO = 0,  // padding with zero (default)
	VENDOR_VIDEOENC_PADDING_MODE_COPY,      // padding with copied bounday-line pixels
	ENUM_DUMMY4WORD(VENDOR_VIDEOENC_PADDING_MODE)
} VENDOR_VIDEOENC_PADDING_MODE;

typedef enum {
	VENDOR_VIDEOENC_DEFAULT_TILE_SPLIT = 0,
	VENDOR_VIDEOENC_ONE_TILE_SPLIT,
	VENDOR_VIDEOENC_TILE_SPLIT_MODE_MAX,
	ENUM_DUMMY4WORD(VENDOR_VIDEOENC_TILE_SPLIT_MODE)
} VENDOR_VIDEOENC_TILE_SPLIT_MODE;

typedef enum {
    VENDOR_VIDEOENC_TILE_CTRL_DISABLE = 0,
    VENDOR_VIDEOENC_TILE_CTRL_USER,
    VENDOR_VIDEOENC_TILE_CTRL_DEFAULT,
    ENUM_DUMMY4WORD(VENDOR_VIDEOENC_TILE_CTRL_MODE)
} VENDOR_VIDEOENC_TILE_CTRL_MODE;

typedef struct _VENDOR_VIDEOENC_BS_RESERVED_SIZE_CFG {
	UINT32 reserved_size;
} VENDOR_VIDEOENC_BS_RESERVED_SIZE_CFG;

//------
typedef struct _VENDOR_VIDEOENC_TIMELAPSE_TIME_CFG {
	UINT32 timelapse_time;
} VENDOR_VIDEOENC_TIMELAPSE_TIME_CFG;

//------
typedef enum
{
	VENDOR_VIDEOENC_JPG_VBR_PRI_NONE = 0,
	VENDOR_VIDEOENC_JPG_VBR_PRI_QUALITY = 1,
	VENDOR_VIDEOENC_JPG_VBR_PRI_FRAMERATE = 2,
	ENUM_DUMMY4WORD(VENDOR_VIDEOENC_JPG_VBR_PRI_MODE)
} VENDOR_VIDEOENC_JPG_VBR_PRI_MODE;

typedef struct _VENDOR_VIDEOENC_JPG_RC_CFG {
	UINT32 vbr_mode_en;                 // 0: cbr , 1: vbr
	UINT32 min_quality;
	UINT32 max_quality;
	VENDOR_VIDEOENC_JPG_VBR_PRI_MODE vbr_priority; // 0: no priority, 1: quality mode (quality > frame rate), 2: frame rate mode (frame rate > quality)
	UINT32 min_framerate;                          // the minimal frame rate  
} VENDOR_VIDEOENC_JPG_RC_CFG;

//------
typedef struct _VENDOR_VIDEOENC_H26XENC_TRIG_SNAPSHOT {
	UINTPTR phy_addr;                    ///< [w]physical address of encoded data (user provide memory space to put JPEG result)
	UINT32 size;                        ///< [w]user buffer size provided
	                                    ///< [r]real size of encoded data
	UINT32 image_quality;               ///< [w]JPEG image quality. range 1~100
	UINT64 timestamp;                   ///< [r]Encoded timestamp (unit: microsecond)
} VENDOR_VIDEOENC_H26XENC_TRIG_SNAPSHOT;

//------
typedef enum {
	VENDOR_VIDEOENC_CODEC_264 = 0,
	VENDOR_VIDEOENC_CODEC_265 = 1
} VENDOR_VIDEOENC_RDO_CODEC;

typedef struct {
	UINT8 avc_intra_4x4_cost_bias;			// default: 8, range: 0~31, //INTRA 4x4 block number decrease as the value increase
	UINT8 avc_intra_8x8_cost_bias;			// default: 8, range: 0~31, //INTRA 8x8 block number decrease as the value increase
	UINT8 avc_intra_16x16_cost_bias;		// default: 8, range: 0~31, //INTRA 16x16 block number decrease as the value increase
	UINT8 avc_inter_tu4_cost_bias;			// default: 8, range: 0~31, //INTER TU4 block number decrease as the value increase
	UINT8 avc_inter_tu8_cost_bias;			// default: 8, range: 0~31, //INTER TU8 block number decrease as the value increase
	UINT8 avc_inter_skip_cost_bias;			// default: 8, range: 0~31, //Skip mode number decrease as the value increase
} VENDOR_VIDEOENC_RDO_264;

typedef struct {
	UINT8 hevc_intra_32x32_cost_bias;		// default:  0, range: 0~15, //INTRA 32x32 block number decrease as the value increase
	UINT8 hevc_intra_16x16_cost_bias;		// default:  0, range: 0~15, //INTRA 16x16 block number decrease as the value increase
	UINT8 hevc_intra_8x8_cost_bias;			// default:  0, range: 0~15, //INTRA 8x8 block number decrease as the value increase
	INT8  hevc_inter_skip_cost_bias;		// default:  0, range: -16~15, //skip mode number decrease as the value increase
	INT8  hevc_inter_merge_cost_bias;		// default:  0, range: -16~15, //merge mode number decrease as the value increase
	UINT8 hevc_inter_64x64_cost_bias;		// default: 14, range: 0~31, //INTER 64x64 block number decrease as the value increase
	UINT8 hevc_inter_64x32_32x64_cost_bias;	// default: 28, range: 0~31, //INTER 64x32 and 32x64 block number decrease as the value increase
	UINT8 hevc_inter_32x32_cost_bias;		// default: 14, range: 0~31, //INTER 32x32 block number decrease as the value increase
	UINT8 hevc_inter_32x16_16x32_cost_bias;	// default: 28, range: 0~31, //INTER 32x16 and 16x32 block number decrease as the value increase
	UINT8 hevc_inter_16x16_cost_bias;		// default:  7, range: 0~31, //INTER 16x16 block number decrease as the value increase
} VENDOR_VIDEOENC_RDO_265;

typedef struct {
	VENDOR_VIDEOENC_RDO_CODEC		rdo_codec;	///< rdo codec. 0: h264, 1: h265
	union {
		VENDOR_VIDEOENC_RDO_264	rdo_264;        ///< parameter of h264 rdo
		VENDOR_VIDEOENC_RDO_265	rdo_265;        ///< parameter of h265 rdo
	} rdo_param;
} VENDOR_VIDEOENC_RDO_CFG;

//------
typedef struct _VENDOR_VIDEOENC_JND_CFG{
	UINT8           enable;                     ///< jnd enable. default: 0, range: 0~1 (0: disable, 1: enable)
	UINT8           str;                        ///< jnd filter strength. default: 7, range: 0~15
	UINT8           level;                      ///< jnd edge level. default: 11, range: 0~15
	UINT8           threshold;                  ///< jnd CM edge detection threshold. default: 5, range: 0~255
	UINT8           c_str;                      ///< jnd chroma filter strength. default: 7, range: 0~15
	UINT8           t_str;						///< jnd temporal filter strength, default 3, range: 0~15 (0: disable)
} VENDOR_VIDEOENC_JND_CFG;

//------
typedef struct _VENDOR_VIDEOENC_H26X_VBR_POLICY_CFG {
	UINT32 h26x_vbr_policy;
} VENDOR_VIDEOENC_H26X_VBR_POLICY_CFG;

//------
typedef struct _VENDOR_VIDEOENC_H26X_RC_GOP_CFG {
	UINT32 h26x_rc_gop;                 ///< update rc gop
} VENDOR_VIDEOENC_H26X_RC_GOP_CFG;

//------
typedef struct _VENDOR_VIDEOENC_H26X_ENC_GOP_CFG {
	UINT32 h26x_enc_gop;                ///< update enc gop & rc gop
} VENDOR_VIDEOENC_H26X_ENC_GOP_CFG;

//------
typedef struct _VENDOR_VIDEOENC_JPG_YUV_TRANS_CFG {
	UINT32 jpg_yuv_trans_en;
} VENDOR_VIDEOENC_JPG_YUV_TRANS_CFG;

//------
typedef struct _VENDOR_VIDEOENC_MIN_RATIO_CFG {
	UINT32              min_i_ratio;     ///< default: 1500 (unit : ms)
	UINT32              min_p_ratio;     ///< default: 1000 (unit : ms)
} VENDOR_VIDEOENC_MIN_RATIO_CFG;

//------
typedef struct _VENDOR_VIDEOENC_H26X_ENC_COLMV {
	UINT32              h26x_colmv_en;
} VENDOR_VIDEOENC_H26X_ENC_COLMV;

//------
typedef struct _VENDOR_VIDEOENC_H26X_COMM_RECFRM {
	UINT32              enable;
	UINT32              h26x_comm_base_recfrm_en;
	UINT32              h26x_comm_svc_recfrm_en;
	UINT32              h26x_comm_ltr_recfrm_en;
} VENDOR_VIDEOENC_H26X_COMM_RECFRM;

//------
typedef struct _VENDOR_VIDEOENC_FIT_WORK_MEMORY {
	BOOL                b_enable;
} VENDOR_VIDEOENC_FIT_WORK_MEMORY;

//------
typedef struct _VENDOR_VIDEOENC_LONG_START_CODE {
	UINT32              long_start_code_en;
} VENDOR_VIDEOENC_LONG_START_CODE;

//------
typedef struct _VENDOR_VIDEOENC_H26X_SVC_WEIGHT_MODE {
	UINT32 h26x_svc_weight_mode;
} VENDOR_VIDEOENC_H26X_SVC_WEIGHT_MODE;

//------
typedef struct _VENDOR_VIDEOENC_QUALITY_BASE_MODE {
	BOOL                quality_base_en;
} VENDOR_VIDEOENC_QUALITY_BASE_MODE;

//------
typedef struct _VENDOR_VIDEOENC_PADDING_MODE_CFG {
    VENDOR_VIDEOENC_PADDING_MODE    mode;
} VENDOR_VIDEOENC_PADDING_MODE_CFG;

//------
typedef struct _VENDOR_VIDEOENC_COMM_SRCOUT {
	BOOL                b_enable;
	BOOL                b_no_jpeg_enc;
	UINTPTR             addr;
	UINT32              size;
} VENDOR_VIDEOENC_COMM_SRCOUT;

//------
typedef struct _VENDOR_VIDEOENC_TIMER_TRIG_COMP {
	BOOL                b_enable;
} VENDOR_VIDEOENC_TIMER_TRIG_COMP;

//------
typedef struct _VENDOR_VIDEOENC_REQ_TARGET_I {
	BOOL                enable;           ///< request i-frame enable. default: 0, range: 0~1 (0: disable, 1: enable)
	UINT64              target_timestamp; ///< set request i-frame with target_timestamp
} VENDOR_VIDEOENC_REQ_TARGET_I;

//------
typedef struct _VENDOR_VIDEOENC_BR_TOLERANCE {
	UINT32              br_tolerance;    ///< default: 2, range: 0~6
} VENDOR_VIDEOENC_BR_TOLERANCE;

//------
typedef struct _VENDOR_VIDEOENC_JPG_COLOR_TO_GRAY_CFG {
	BOOL                enable;                ///< color to gray enable, default: 0, range: 0~1
	BOOL                color_to_gray;         ///< final image color to gray, default: 0, range: 0~1
	BOOL                src_color_to_gray;     ///< source image colot to gray, default: 0, range: 0~1
} VENDOR_VIDEOENC_JPG_COLOR_TO_GRAY_CFG;

//------
typedef struct _VENDOR_VIDEOENC_H26X_ENC_GDR {
	BOOL                enable;           ///< enable gdr. default: 0, range: 0~1 (0: disable, 1: enable)
	UINT32              period;           ///< intra refresh period. default: 0, range: 0~0xFFFFFFFF (0: always refresh, others: intra refresh frame period)
	UINT32              number;           ///< intra refresh row number. default: 1, range: 1 ~ number of macroblock/ctu row
	BOOL                enable_gdr_i_frm; ///< gdr i-frame scheme enable. default: 0, range: 0~1 (0: disable, 1: enable)
	BOOL                enable_gdr_qp;    ///< gdr qp scheme enable. default: 0, range: 0~1 (0: disable, 1: enable)
	UINT32              gdr_qp;           ///< gdr qp, range: 0~51
} VENDOR_VIDEOENC_H26X_ENC_GDR;

//------
typedef struct _VENDOR_VIDEOENC_TILE_SPLIT_MODE_CFG {
	VENDOR_VIDEOENC_TILE_SPLIT_MODE    mode;
} VENDOR_VIDEOENC_TILE_SPLIT_MODE_CFG;

//------
typedef struct _VENDOR_VIDEOENC_H26X_LOW_POWER_CFG {
	BOOL                b_enable;         ///< enable H26x low power. default: 0, range: 0~1 (0: disable, 1: enable)
	UINT8               mode;             ///< lpm mode, 0: default, 1: enhansed, suggest: 0
} VENDOR_VIDEOENC_H26X_LOW_POWER_CFG;

//------
typedef struct _VENDOR_VIDEOENC_H26X_DESC_CFG {
	UINT32              vps_paddr;        ///< vps physical addr
	UINT32              vps_size;         ///< vps size
	UINT32              sps_paddr;        ///< sps physical addr
	UINT32              sps_size;         ///< sps size
	UINT32              pps_paddr;        ///< pps physical addr
	UINT32              pps_size;         ///< pps size
} VENDOR_VIDEOENC_H26X_DESC_CFG;

//------
typedef struct _VENDOR_VIDEOENC_H26X_MAQ_DIFF {
	BOOL                b_enable;
	INT8                str;             ///< range: -15 ~ 15
	UINT8               start_idx;       ///< range: 0~10
	UINT8               end_idx;         ///< range: 0~10 and > start_idx
} VENDOR_VIDEOENC_H26X_MAQ_DIFF;

//------
typedef struct _VENDOR_VIDEOENC_USER_TILE_CTRL_CFG {
    VENDOR_VIDEOENC_TILE_CTRL_MODE mode;
    UINT32              max_width;
    UINT32              max_height;
} VENDOR_VIDEOENC_USER_TILE_CTRL_CFG;

//------
 typedef struct _VENDOR_VIDEOENC_USER_DATA_CFG {
    BOOL                b_enable;
    UINTPTR             data_addr;
    UINT32              data_length;
} VENDOR_VIDEOENC_USER_DATA_CFG;

//------
typedef struct _VENDOR_VIDEOENC_BG_RDO_CFG {
	BOOL                b_enable;
	UINT8               avc_bg_skip_bias;   ///< H264 skip bias of bg. default 8, range 0~31
	INT8                hevc_bg_skip_bias;  ///< H265 skip bias of bg. default 0, range -16~15
	INT8                hevc_bg_merge_bias; ///< H265 merge bias of bg. default 0, range -16~15
	UINT8               bg_bias_shift;      ///< shift bit of bg bias. default 0, range 0~3
	UINT8               mode;               ///< default 0, range 0~1
} VENDOR_VIDEOENC_BG_RDO_CFG;

//------
typedef struct _VENDOR_VIDEOENC_H26X_SLICE_SPLIT_CFG {
	UINT32              enable;               ///< enable multiple slice. default: 0, range: 0~1 (0: disable, 1: enable)
	UINT32              slice_row_num;        ///< number of macroblock/ctu rows occupied by a slice; number of rows in slice_0, range: 1 ~ number of macroblock/ctu row

	BOOL                slice_mode;           ///< multi-slice mode. default: 0, range: 0~1 (0: same number of rows per slice, 1: user defined rows in slices)
	UINT32              slice_row_num_ext[3]; ///< number of macroblock/ctu rows occupied by a slice; number of rows in slice_1 ~ slice_3, range: 1 ~ number of macroblock/ctu row
	INT32               slice_i_idx;          ///< I slice index in multi slice. default: -1(disable), range: -1 ~ macroblock/ctu rows*tile index -1

	BOOL                enable_slice_bs;      ///< enable slice bs data. default: 0, range: 0~1 (0: frame based for each pull out bs, 1: slice based for each pull out bs)
} VENDOR_VIDEOENC_H26X_SLICE_SPLIT_CFG;

//------
#define VENDOR_VIDEOENC_WATERMARK_COUNT  8
typedef struct _VENDOR_VIDEOENC_WATERMARK_INFO {
	BOOL                enable;
	UINTPTR             addr;
	UINT8               len;   //0 ~15
	UINT8               uv;    //0~1
	UINT8               str;   //0~2
	UINT32              left_top;
} VENDOR_VIDEOENC_WATERMARK_INFO;

typedef struct _VENDOR_VIDEOENC_WATERMARK {
	VENDOR_VIDEOENC_WATERMARK_INFO watermark[VENDOR_VIDEOENC_WATERMARK_COUNT];
} VENDOR_VIDEOENC_WATERMARK;

//------
typedef struct _VENDOR_VIDEOENC_EMBEDDED_MD_CFG {
	BOOL                b_md_enable;            ///< [r/w] motion detect enable, range: 0 ~ 1
	INT8                md_delta_qp;            ///< [r/w] cu16 delatQP, range: -16 ~ 15, suggest: 0
	INT8                md_user_offset;         ///< [r/w] user offset, range: -16 ~ 15, suggest: 0
	UINT16              md_boundary_thr;        ///< [r/w] outlier threshold, range: 0 ~ 1000, suggest: 15

	BOOL                b_motion_filter_enable; ///< [r/w] motion filter enable, range: 0 ~ 1, 
	UINT8               motion_filter_thr;      ///< [r/w] motion filter threshold, range: 0 ~ 7, suggest: 1
	INT8                motion_filter_delta_qp; ///< [r/w] motion filter DQP, range: -16 ~ 15, suggest: 0
} VENDOR_VIDEOENC_EMBEDDED_MD_CFG;

#define VENDOR_VIDEOENC_SMART_BBOX_MAX_NUM 32
typedef enum _VENDOR_VIDEOENC_SMART_ROI_CLASS {
	VENDOR_VIDEOENC_SMART_ROI_CLASS0 = 0,              ///< class 0, highest priority, priority: 0>1>2>3>4
	VENDOR_VIDEOENC_SMART_ROI_CLASS1,                  ///< class 1
	VENDOR_VIDEOENC_SMART_ROI_CLASS2,                  ///< class 2
	VENDOR_VIDEOENC_SMART_ROI_CLASS3,                  ///< class 3
	VENDOR_VIDEOENC_SMART_ROI_CLASS4,                  ///< class 4, lowest priority
	VENDOR_VIDEOENC_SMART_ROI_CLASS_MAX,
	ENUM_DUMMY4WORD(VENDOR_VIDEOENC_SMART_ROI_CLASS)
} VENDOR_VIDEOENC_SMART_ROI_CLASS;

typedef enum _VENDOR_VIDEOENC_SMART_BBOX_POS {
	VENDOR_VIDEOENC_SMART_BBOX_POS_TOP_LEFT = 0,       ///< top left position of bounding box
	VENDOR_VIDEOENC_SMART_BBOX_POS_BOTTOM_RIGHT,       ///< bottom right position of bounding box
	VENDOR_VIDEOENC_SMART_BBOX_POS_MAX,
	ENUM_DUMMY4WORD(VENDOR_VIDEOENC_SMART_BBOX_POS)
} VENDOR_VIDEOENC_SMART_BBOX_POS;

typedef struct _VENDOR_VIDEOENC_BBOX {
	HD_UPOINT positions[VENDOR_VIDEOENC_SMART_BBOX_POS_MAX]; ///< top left and bottom right position of bounding box
	VENDOR_VIDEOENC_SMART_ROI_CLASS class_id;             ///< bounding box class
} VENDOR_VIDEOENC_BBOX;

typedef struct _VENDOR_VIDEOENC_SMART_ROI {
	BOOL enable;                                                   ///< enable AI smart encode, range: 0~1                                             ///< timestamp of bounding boxes
	UINT8 fg_str[VENDOR_VIDEOENC_SMART_ROI_CLASS_MAX];             ///< foreground strength of each classes, range: 0~255
	UINT8 mode;                                                    ///< 0: adjust qp only, 1: adjust qp + BGRDO
} VENDOR_VIDEOENC_SMART_ROI;

typedef struct _VENDOR_VIDEOENC_SMART_BBOX {
	HD_DIM base_resolution;                                        ///< base resolution
	UINT8 bbox_num;                                                ///< number of bounding boxes, range: 0~VENDOR_VIDEOENC_SMART_BBOX_MAX_NUM
	VENDOR_VIDEOENC_BBOX bbox[VENDOR_VIDEOENC_SMART_BBOX_MAX_NUM];  ///< bounding boxes
	UINT64 timestamp;                                              ///< timestamp of bounding boxes
} VENDOR_VIDEOENC_SMART_BBOX;

//------
typedef struct _VENDOR_VIDEOENC_DYNAMIC_FR_CFG {
	BOOL   enable;                                       ///< enable dynamic frame rate, range: 0~1, default: 0
	UINT32 min_fps;                                      ///< the minimal frame rate, range: 1~fps-1, default: fps / 3
	UINT32 motion_sensitivity;                           ///< the bias for fps refresh, range: 0~15, default: 15
} VENDOR_VIDEOENC_DYNAMIC_FR_CFG;

//------
typedef struct _VENDOR_VIDEOENC_DYNAMIC_GOP_CFG {
	BOOL   enable;                                       ///< enable dynamic frame rate, range: 0~1, default: 0
	UINT32 max_gop;                                      ///< the maximal gop of dynamic gop, range: gop ~ 4096, default: gop * 5 ~ gop * 10
	UINT32 motion_sensitivity;                           ///< the bias for fps refresh, range: 0~15, default: 8
} VENDOR_VIDEOENC_DYNAMIC_GOP_CFG;

//------
typedef enum {
	VENDOR_VIDEOENC_SEI_SIGNED_VDO_ID_1 = 0,
	VENDOR_VIDEOENC_SEI_SIGNED_VDO_ID_2,
	VENDOR_VIDEOENC_SEI_SIGNED_VDO_ID_3,
	VENDOR_VIDEOENC_SEI_SIGNED_VDO_ID_4,
	VENDOR_VIDEOENC_SEI_SIGNED_VDO_ID_MAX,
	ENUM_DUMMY4WORD(VENDOR_VIDEOENC_SEI_SIGNED_VDO_ID)
} VENDOR_VIDEOENC_SEI_SIGNED_VDO_ID;

typedef struct _VENDOR_VIDEOENC_SEI_SIGNED_VDO_CFG {
	BOOL                enable;
	UINT32              machine_id[VENDOR_VIDEOENC_SEI_SIGNED_VDO_ID_MAX];
} VENDOR_VIDEOENC_SEI_SIGNED_VDO_CFG;

//------
typedef struct _VENDOR_VIDEOENC_H26X_PRIVACY_CFG {
	BOOL                enable;                                    ///< enable H26x privacy. default: 0, range: 0~1 (0: disable, 1: enable)
} VENDOR_VIDEOENC_H26X_PRIVACY_CFG;

//------
typedef struct _VENDOR_VIDEOENC_SKIP_BS_QUICK_ROLLBACK_CFG {
	BOOL                enable;
} VENDOR_VIDEOENC_SKIP_BS_QUICK_ROLLBACK_CFG;

//------
typedef struct _VENDOR_VIDEOENC_OSG_QP {
	UINT8               lpm_mode;
	UINT8               tnr_mode;
	UINT8               fro_mode;
} VENDOR_VIDEOENC_OSG_QP;

//------
typedef struct _VENDOR_VIDEOENC_OSG_COLOR_INVERT {
	UINT8               blk_num;
	UINT8               org_color_level;
	UINT8               inv_color_level;
	UINT8               nor_diff_th;
	UINT8               inv_diff_th;
	UINT8               sta_only_mode;
	UINT8               full_eval_mode;
	UINT8               eval_lum_targ;
} VENDOR_VIDEOENC_OSG_COLOR_INVERT;

//------
typedef struct _VENDOR_VIDEOENC_OSG_MASK {
	HD_OSG_MASK_TYPE    type;
	UINT32              color;
	UINT32              alpha;
	HD_UPOINT           position;
	HD_DIM              dim;
	UINT32              thickness;
	UINT32              layer;
	UINT32              region;
} VENDOR_VIDEOENC_OSG_MASK;

//------
typedef struct _VENDOR_VIDEOENC_OSG_MOSAIC {
	HD_UPOINT           position;
	HD_DIM              dim;
	UINT32              layer;
	UINT32              region;
	UINT32              mosaic_blk_w;
	UINT32              mosaic_blk_h;
} VENDOR_VIDEOENC_OSG_MOSAIC;

typedef struct _VENDOR_VIDEOENC_OSG_STAMP_ATTR {
	HD_OSG_STAMP_ATTR attr;
	UINT32            bg_alpha;
} VENDOR_VIDEOENC_OSG_STAMP_ATTR;

//------
typedef struct _VENDOR_VIDEOENC_DMA_ABORT {
	UINT32              dma_abort_en;
} VENDOR_VIDEOENC_DMA_ABORT;

//------
typedef struct _VENDOR_VIDEOENC_BS_QUEUE_CNT_CFG {
	UINT32 queue_cnt;
} VENDOR_VIDEOENC_BS_QUEUE_CNT_CFG;

//------
typedef struct _VENDOR_VIDEOENC_JPG_UVC_SLICE_ENC_CFG {
	BOOL enable;
} VENDOR_VIDEOENC_JPG_UVC_SLICE_ENC_CFG;

//------
typedef enum _VENDOR_VIDEOENC_PARAM_ID {
	VENDOR_VIDEOENC_PARAM_OUT_BS_RESERVED_SIZE,   ///< IPC only.  support set with i/o path, using VENDOR_VIDEOENC_BS_RESERVED_SIZE_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_TIMELAPSE_TIME,     ///< IPC only.  support set with i/o path, using VENDOR_VIDEOENC_TIMELAPSE_TIME_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_JPG_RC,             ///< IPC only.  support set with i/o path, using VENDOR_VIDEOENC_JPG_RC_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_TRIG_SNAPSHOT,      ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_H26XENC_TRIG_SNAPSHOT struct
	VENDOR_VIDEOENC_PARAM_OUT_RDO,                ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_RDO_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_JND,                ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_JND_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_H26X_VBR_POLICY,    ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_H26X_VBR_POLICY_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_H26X_RC_GOP,        ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_H26X_RC_GOP_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_H26X_ENC_GOP,       ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_H26X_ENC_GOP_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_JPG_YUV_TRANS,      ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_JPG_YUV_TRANS_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_MIN_RATIO,          ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_MIN_RATIO_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_COLMV,              ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_H26X_ENC_COLMV struct
	VENDOR_VIDEOENC_PARAM_OUT_FIT_WORK_MEMORY,    ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_FIT_WORK_MEMORY struct
	VENDOR_VIDEOENC_PARAM_OUT_LONG_START_CODE,    ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_LONG_START_CODE struct
	VENDOR_VIDEOENC_PARAM_OUT_H26X_SVC_WEIGHT_MODE,    ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_H26X_SVC_WEIGHT_MODE struct
	VENDOR_VIDEOENC_PARAM_OUT_QUALITY_BASE,       ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_QUALITY_BASE_MODE struct
	VENDOR_VIDEOENC_PARAM_OUT_DMA_ABORT,          ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_DMA_ABORT struct
	VENDOR_VIDEOENC_PARAM_OUT_COMM_RECFRM,		  ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_H26X_COMM_RECFRM struct
	VENDOR_VIDEOENC_PARAM_OUT_PADDING_MODE,       ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_PADDING_MODE_CFG struct
	VENDOR_VIDEOENC_PARAM_IN_STAMP_QP,            ///< IPC only.  support set with i/o path, using VENDOR_VIDEOENC_OSG_QP struct
	VENDOR_VIDEOENC_PARAM_IN_STAMP_COLOR_INVERT,  ///< IPC only.  support set with i/o path, using VENDOR_VIDEOENC_OSG_COLOR_INVERT struct
	VENDOR_VIDEOENC_PARAM_IN_STAMP_OVERLAP     ,  ///< IPC only.  support set with i/o path, using int
	VENDOR_VIDEOENC_PARAM_IN_STAMP_ATTR,          ///< IPC only.  support set with i/o path, using VENDOR_VIDEOENC_OSG_STAMP_ATTR struct
	VENDOR_VIDEOENC_PARAM_IN_MASK_ATTR,           ///< IPC only.  support set with i/o path, using VENDOR_VIDEOENC_OSG_MASK struct
	VENDOR_VIDEOENC_PARAM_IN_MOSAIC_ATTR,         ///< IPC only.  support set with i/o path, using VENDOR_VIDEOENC_OSG_MOSAIC struct
	VENDOR_VIDEOENC_PARAM_IN_MOSAIC_SIZE,         ///< IPC only.  support set with i/o path, using HD_OSG_MOSAIC_ATTR struct. Only mosaic_blk_w and mosaic_blk_h are valid
	VENDOR_VIDEOENC_PARAM_IN_QP_MOSAIC,           ///< IPC only.  support set with i/o path, using int
	VENDOR_VIDEOENC_PARAM_OUT_BS_QUEUE_CNT,       ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_BS_QUEUE_CNT_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_COMM_SRCOUT,        ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_COMM_SRCOUT struct
	VENDOR_VIDEOENC_PARAM_OUT_TIMER_TRIG_COMP,    ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_TIMER_TRIG_COMP struct
	VENDOR_VIDEOENC_PARAM_OUT_REQ_TARGET_I,       ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_REQ_TARGET_I struct
	VENDOR_VIDEOENC_PARAM_OUT_BR_TOLERANCE,       ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_BR_TOLERANCE struct
	VENDOR_VIDEOENC_PARAM_OUT_JPG_COLOR_TO_GRAY,  ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_JPG_COLOR_TO_GRAY_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_H26X_ENC_GDR,       ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_H26X_ENC_GDR struct
	VENDOR_VIDEOENC_PARAM_OUT_TILE_SPLIT_MODE,    ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_TILE_SPLIT_MODE_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_H26X_LOW_POWER,     ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_H26X_LOW_POWER_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_JPG_UVC_SLICE_ENC,  ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_JPG_UVC_SLICE_ENC_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_H26X_DESC,          ///< IPC only   support get with i/o path, using VENDOR_VIDEOENC_H26X_DESC_CFG struct
	VENDOR_VIDEOENC_PARAM_META_ALLC,              ///< IPC only   support set with i/o path, using VENDOR_META_ALLOC struct
	VENDOR_VIDEOENC_PARAM_OUT_H26X_MAQ_DIFF,      ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_H26X_MAQ_DIFF struct
	VENDOR_VIDEOENC_PARAM_OUT_USER_TILE_CTRL,     ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_USER_TILE_CTRL_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_USER_DATA,          ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_USER_DATA_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_BG_RDO,             ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_BG_RDO_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_H26X_SLICE_SPLIT,   ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_H26X_SLICE_SPLIT_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_WATERMARK,          ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_WATERMARK struct
	VENDOR_VIDEOENC_PARAM_OUT_EMBEDDED_MD,        ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_EMBEDDED_MD_CFG struct
	VENDOR_VIDEOENC_PARAM_SMART_ROI,              ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_SMART_ROI struct
	VENDOR_VIDEOENC_PARAM_SMART_BBOX,             ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_SMART_BBOX struct
	VENDOR_VIDEOENC_PARAM_DYNAMIC_FR,             ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_DYNAMIC_FR_CFG struct
	VENDOR_VIDEOENC_PARAM_DYNAMIC_GOP,            ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_DYNAMIC_GOP_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_SEI_SIGNED_VDO,     ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_SEI_SIGNED_VDO_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_H26X_PRIVACY,       ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_H26X_PRIVACY_CFG struct
	VENDOR_VIDEOENC_PARAM_OUT_SKIP_BS_QUICK_ROLLBACK, ///< IPC only   support set with i/o path, using VENDOR_VIDEOENC_SKIP_BS_QUICK_ROLLBACK_CFG struct
	ENUM_DUMMY4WORD(VENDOR_VIDEOENC_PARAM_ID)
} VENDOR_VIDEOENC_PARAM_ID;

/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
HD_RESULT vendor_videoenc_set(HD_PATH_ID path_id, VENDOR_VIDEOENC_PARAM_ID id, VOID *p_param);
HD_RESULT vendor_videoenc_get(HD_PATH_ID path_id, VENDOR_VIDEOENC_PARAM_ID id, VOID *p_param);

#ifdef __cplusplus
}
#endif

#endif

