/**
	@brief Header file of definition of ai cpu postproc.

	@file ai_cpu_postproc.h

	@ingroup ai_net_sample

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2023.	All rights reserved.
*/

#ifndef _AI_CPU_POSTPROC_H_
#define _AI_CPU_POSTPROC_H_

#include <string.h>
#include <math.h>
#include <arm_neon.h>
#include "vendor_ai.h"

/*-----------------------------------------------------------------------------*/
/* Macro Constant Definitions                                                  */
/*-----------------------------------------------------------------------------*/

// set NUM_PRIORBOX & OUT_NUM_LAYER
// network setting
#define OUT_NUM_LAYER		2
// priorbox setting
#define NUM_PRIORBOX		6
#define PRIORBOX_NEON          				FALSE
#define PRIORBOX_PRINT_PARM    				FALSE
#define NN_INPUTDATA_ASPECT_RATIO_NUM    	10
#define NN_PRIORBOX_SIZE_NUM            	4
#define NN_PRIORBOX_VAR_NUM             	4
#define NN_PRIORBOX_ASPECT_RATIO_NUM    	4
// detout setting
#define TOP_N       		5
// general setting
#define SETTING_CHK			TRUE
#define DBUG_INFO			TRUE
#define ALIGN_4(x) 			((((x) + 3) >> 2) << 2)


#undef MIN
#define MIN(a, b)           ((a) < (b) ? (a) : (b))
#undef MAX
#define MAX(a, b)           ((a) > (b) ? (a) : (b))
#undef ABS
#define ABS(a)              ((a) >= 0 ? (a) : (-a))
#undef CLAMP
#define CLAMP(x,min,max)    (((x) > (max)) ? (max) : (((x) > (min)) ? (x) : (min)))

#undef SWAP
#define SWAP(a, b, t)       (t) = (a); (a) = (b); (b) = (t)


/********************************************************************
	TYPE DEFINITION
********************************************************************/
typedef struct _INPUTDATA_PRIORBOX {
	UINT32 in_width;
	UINT32 in_height;
	UINT32 img_width;
	UINT32 img_height;
	FLOAT min_sizes[NN_PRIORBOX_SIZE_NUM];
	FLOAT max_sizes[NN_PRIORBOX_SIZE_NUM];
	UINT32 min_size_num;
	UINT32 max_size_num;
	FLOAT aspect_ratios[NN_INPUTDATA_ASPECT_RATIO_NUM];
	UINT32 aspect_ratio_num;
	UINT8 flip;
	UINT8 clip;
	FLOAT variances[NN_PRIORBOX_VAR_NUM];
	UINT32 variance_num;
	INT16 num_prior;
	FLOAT off_set;
}INPUTDATA_PRIORBOX;

typedef struct _OUTPUTDATA_PRIORBOX {
	FLOAT *out_point;
	FLOAT *out_var;
	INT16 size;
} OUTPUTDATA_PRIORBOX;

typedef enum {
	BOX_XMIN,
	BOX_YMIN,
	BOX_XMAX,
	BOX_YMAX
} BOX_COORD_E;

typedef enum {
	BOX_AREA,
	BOX_SCORE
} BOX_INFO_E;

typedef struct _BOX {
	FLOAT xmin;
	FLOAT ymin;
	FLOAT xmax;
	FLOAT ymax;
} BOX;

typedef struct _BOX_INFO {
	FLOAT area;
	FLOAT score;
} BOX_INFO;

typedef struct _NORM_BOX {
	FLOAT xmin;
	FLOAT ymin;
	FLOAT xmax;
	FLOAT ymax;
	FLOAT score;
} NORM_BOX;

typedef struct _BOX_IDX {
	FLOAT score;
	NORM_BOX *box;
} BOX_IDX;

typedef struct _NN_DETOUT_CALC_PRMS{
	NORM_BOX *all_loc_preds;
	FLOAT *all_conf_scores ;
	NORM_BOX *prior_bboxes;
	FLOAT *prior_variances;
	NORM_BOX *all_decode_bboxes;
	NORM_BOX *decode_bboxes_ ;
	BOX_IDX *decode_bboxes_idx;
	BOX *out_bboxes;
	BOX_INFO *out_bboxes_info;
} NN_DETOUT_CALC_PRMS;

typedef enum {
	NN_PRIORBOX_CODE_CENTER = 0,
	NN_PRIORBOX_CODE_CORNER = 1,
	NN_PRIORBOX_CODE_CORNER_SIZE = 2,
	ENUM_DUMMY4WORD(NN_PRIROBOX_CODE_TYPE)
} NN_PRIORBOX_CODE_TYPE;

typedef enum {
	NN_DETOUT_IN_LOC = 0,
	NN_DETOUT_IN_CONF = 1,
	NN_DETOUT_IN_PRIOR = 2,
	NN_DETOUT_IN_NUM,
	ENUM_DUMMY4WORD(NN_DETOUT_IN_TYPE)
} NN_DETOUT_IN_TYPE;

typedef struct _NN_PRIORBOX_PARM {
	UINTPTR in_addr;
	UINTPTR out_addr;
	UINT32 in_width;
	UINT32 in_height;
	UINT32 img_width;
	UINT32 img_height;
	UINT32 out_channel_ofs;
	FLOAT min_sizes[NN_PRIORBOX_SIZE_NUM];
	FLOAT max_sizes[NN_PRIORBOX_SIZE_NUM];
	UINT32 min_size_num;
	UINT32 max_size_num;
	FLOAT aspect_ratios[NN_PRIORBOX_ASPECT_RATIO_NUM];
	UINT32 aspect_ratio_num;
	UINT8 flip;
	UINT8 clip;
	FLOAT variances[NN_PRIORBOX_VAR_NUM];
	UINT32 variance_num;
	FLOAT offset;
}NN_PRIORBOX_PARM;

typedef struct _NN_DETOUT_PARM {
	UINTPTR in_addr[NN_DETOUT_IN_NUM];
	UINTPTR out_addr;
	UINT16 batch_num;
	UINT32 num_classes;
	UINT32 num_priors;
	BOOL share_loc;
	UINT32 bg_lbl_id;
	NN_PRIORBOX_CODE_TYPE code_type;
	UINT32 keep_top_k;
	FLOAT conf_thresh;
	FLOAT nms_thresh;
	UINT32 top_k;
} NN_DETOUT_PARM;

typedef struct _CAFFE_PARAM_PRIORBOX {
	INT32 tensor_dim[4];
	FLOAT aspect_ratio[2];
	BOOL  flip;
	BOOL  clip;
	INT32 max_size;
	INT32 min_size;
	FLOAT offset;
	FLOAT variance[4];
} CAFFE_PARAM_PRIORBOX;

typedef struct _CAFFE_PARAM_NMS {
	FLOAT nms_threshold;
	INT32 top_k;
} CAFFE_PARAM_NMS;

typedef struct _CAFFE_PARAM_DETOUT {
	INT32 background_label_id;
	NN_PRIORBOX_CODE_TYPE code_type;
	FLOAT confidence_threshold;
	INT32 keep_top_k;
	CAFFE_PARAM_NMS nms_param;
	INT32 num_classes;
	BOOL share_location;
} CAFFE_PARAM_DETOUT;

/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
extern UINT32 model_input_dim[4];
extern CHAR g_outlayer_name[OUT_NUM_LAYER][256];

HD_RESULT set_priorbox(NN_PRIORBOX_PARM* priorbox, UINT32 num_priorbox);
HD_RESULT set_detout(FLOAT** conf_loc, NN_DETOUT_PARM* detout, NN_PRIORBOX_PARM* priorbox, UINT32 num_priorbox);
HD_RESULT release_buffer(VENDOR_AI3_BUF* buf, NN_DETOUT_PARM* detout, NN_PRIORBOX_PARM* priorbox, UINT32 num_priorbox, FLOAT** conf_loc);
HD_RESULT run_detout(NN_DETOUT_PARM* detout);

#endif
