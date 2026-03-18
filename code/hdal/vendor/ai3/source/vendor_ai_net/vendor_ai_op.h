/**
	@brief Header file of vendor ai net command.

	@file vendor_ai_net_op.h

	@ingroup vendor_ai_net

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _VENDOR_AI_OP_H_
#define _VENDOR_AI_OP_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#define VENDOR_AI_PREPROC_FUNC_CNT        ((UINT32)0x4)
#define VENDOR_AI_PREPROC_YUV2RGB_EN      ((UINT32)0x00000001)      ///< preprocessing from YUV to RGB enable
#define VENDOR_AI_PREPROC_SUB_EN          ((UINT32)0x00000002)      ///< preprocessing mean subtraction enable
#define VENDOR_AI_PREPROC_PAD_EN          ((UINT32)0x00000004)      ///< preprocessing padding enable
#define VENDOR_AI_PREPROC_ROT_EN          ((UINT32)0x00000008)      ///< preprocessing rotate enable

#define VENDOR_AI_PREPROC_YUV2RGB_EN      ((UINT32)0x00000001)      ///< preprocessing from YUV to RGB enable
#define VENDOR_AI_PREPROC_SUB_EN          ((UINT32)0x00000002)      ///< preprocessing mean subtraction enable
#define VENDOR_AI_PREPROC_PAD_EN          ((UINT32)0x00000004)      ///< preprocessing padding enable
#define VENDOR_AI_PREPROC_ROT_EN          ((UINT32)0x00000008)      ///< preprocessing rotate enable
#define VENDOR_AI_PREPROC_BT601_YUV2RGB_LIMIT_EN     ((UINT32)0x00000011)
#define VENDOR_AI_PREPROC_BT709_YUV2RGB_LIMIT_EN     ((UINT32)0x00000021)
#define VENDOR_AI_PREPROC_BT601_YUV2RGB_FULL_EN      ((UINT32)0x00000041)
#define VENDOR_AI_PREPROC_BT709_YUV2RGB_FULL_EN      ((UINT32)0x00000081)
/********************************************************************
	TYPE DEFINITION
********************************************************************/
typedef enum {
	AI_PREPROC_SRC_YUV420      = 0,     ///< source YUV420 format
	AI_PREPROC_SRC_YONLY       = 1,     ///< source YONLY format
	AI_PREPROC_SRC_UVPACK      = 2,     ///< source UVPACK format
	AI_PREPROC_SRC_RGB         = 3,     ///< source RGB format
	AI_PREPROC_SRC_YUV420_NV21 = 4,     ///< source YUV420_NV21 format
	ENUM_DUMMY4WORD(VENDOR_AI_PREPROC_SRC_FMT)
} VENDOR_AI_PREPROC_SRC_FMT;

typedef enum {
	AI_IO_INT8          = 0,
	AI_IO_UINT8         = 1,
	AI_IO_INT16         = 2,
	AI_IO_UINT16        = 3,
	AI_IO_INT12,                    ///< (for NT96680)
	AI_IO_UINT12,                   ///< (for NT96680)
	ENUM_DUMMY4WORD(VENDOR_AI_IO_TYPE)
} VENDOR_AI_IO_TYPE;

typedef struct _VENDOR_AI_SIZE {
	UINT16 width;
	UINT16 height;
	UINT16 channel;
} VENDOR_AI_SIZE;

typedef struct _VENDOR_AI_OFS {
	UINT32 line_ofs;                ///< line offset for multi-stripe mode or image mode
	UINT32 channel_ofs;             ///< channel offst for multi-stripe mode
	UINT32 batch_ofs;               ///< batch offset for batch mode
	UINT8 h_stripe_en;
	UINT8 v_stripe_en;
	UINT8 ch_stripe_en;
} VENDOR_AI_OFS;

typedef struct _VENDOR_AI_SCALE_KERPARM {
	VENDOR_AI_SIZE scl_out_size;                          ///< output size
    BOOL fact_update_en;                                ///< enable/disable updating down rate and scaling factor
} VENDOR_AI_SCALE_KERPARM;

typedef enum {
	AI_SUB_DC       = 0,            ///< mean subtraction DC-mode
	AI_SUB_PLANAR   = 1,            ///< mean subtraction Planner-mode
	ENUM_DUMMY4WORD(VENDOR_AI_SUB_MODE)
} VENDOR_AI_SUB_MODE;

typedef enum {
	AI_ROT_90   = 0,              ///< rotate 90 degree
	AI_ROT_270  = 1,              ///< rotate 270 degree
	AI_ROT_180  = 2,              ///< rotate 180 degree
	ENUM_DUMMY4WORD(VENDOR_AI_ROT_MODE)
} VENDOR_AI_ROT_MODE;

typedef enum {
	AI_SUB_DUP_1x = 0,              ///< mean subtraction input 1x duplicate
	AI_SUB_DUP_2x = 1,              ///< mean subtraction input 2x duplicate
	AI_SUB_DUP_4x = 2,              ///< mean subtraction input 4x duplicate
	AI_SUB_DUP_8x = 3,              ///< mean subtraction input 8x duplicate
	ENUM_DUMMY4WORD(VENDOR_AI_SUB_DUP_RATE)
} VENDOR_AI_SUB_DUP_RATE;

typedef struct _VENDOR_AI_SUB_KERPARM {
	VENDOR_AI_SUB_MODE sub_mode;                  ///< mean subtraction mode selection
	VENDOR_AI_SUB_DUP_RATE dup_rate;              ///< mean subtraction input duplicate rate
	UINT32 sub_in_w;                            ///< mean subtraction input width
	UINT32 sub_in_h;                            ///< mean subtraction input height
    UINT8 sub_dc_coef[3];                       ///< mean subtraction DC mode coefficient
    INT8 sub_shf;                               ///< shift after mean subtraction; right shift(>0), left shift(<0)
} VENDOR_AI_SUB_KERPARM;

typedef struct _VENDOR_AI_CROPPAD_KERPARM {
	UINT32 crop_x;                           ///< starting X of cropping
	UINT32 crop_y;                           ///< starting Y of cropping
	UINT32 crop_w;                           ///< width of cropping
    UINT32 crop_h;                           ///< height of cropping
    UINT32 pad_out_x;                        ///< starting X of padding after crop
    UINT32 pad_out_y;                        ///< starting Y of padding after crop
    UINT32 pad_out_w;                        ///< width of output
    UINT32 pad_out_h;                        ///< height of output
    UINT32 pad_val[3];                       ///< padding value
} VENDOR_AI_CROPPAD_KERPARM;

typedef struct _VENDOR_AI_ROTATE_KERPARM {
	VENDOR_AI_ROT_MODE rot_mode;               ///< rotate degree selection
} VENDOR_AI_ROTATE_KERPARM;

typedef struct _VENDOR_AI_PREPROC_PARM {
    UINT8 func_list[VENDOR_AI_PREPROC_FUNC_CNT];
    VENDOR_AI_PREPROC_SRC_FMT src_fmt;                ///< input source format
    VENDOR_AI_PREPROC_SRC_FMT rst_fmt;                ///< output source format
	VENDOR_AI_IO_TYPE in_type;                        ///< input type
	VENDOR_AI_IO_TYPE out_type;                       ///< output type
	VENDOR_AI_SIZE in_size;                           ///< input size
	VENDOR_AI_OFS in_ofs[3];                          ///< input offset (line-offset, channel-offset, batch-offset)
	VENDOR_AI_OFS out_ofs[3];                         ///< output offset (line-offset, channel-offset, batch-offset)
	uintptr_t in_addr[3];                           ///< input channel address
	uintptr_t out_addr[3];                          ///< output channel address
	VENDOR_AI_SCALE_KERPARM scale_ker;                ///< scaling parameters
	VENDOR_AI_SUB_KERPARM sub_ker;                    ///< mean subtraction parameters
	VENDOR_AI_CROPPAD_KERPARM pad_ker;                ///< crop and padding parameters
	VENDOR_AI_ROTATE_KERPARM rotate_ker;          	///< rotate parameters
} VENDOR_PREPROC_PARM;

#define NUE2_BUF 50  // value of get_nue2_max_ll_cmd_lines()
typedef struct _VENDOR_AI_PROC_WORK {	
	NN_GEN_MODE_CTRL mctrl;
	VENDOR_AI_LL_HEAD ll_head;
	UINT64 data[NUE2_BUF]; // 1K for VENDOR_AI_FC_PARM	
} VENDOR_AI_OP_WORK;


#endif  /* _VENDOR_AI_OP_H_ */
