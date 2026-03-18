/**
	@brief Header file of definition of vendor net ll cmd generation.

	@file vendor_ai_genll.h

	@ingroup vendor_ai_net

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _VENDOR_AI_GENLL_H_
#define _VENDOR_AI_GENLL_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "kwrap/type.h"
#include "vendor_ai.h"

/********************************************************************
 MACRO CONSTANT DEFINITIONS
********************************************************************/
//#define 


/********************************************************************
	TYPE DEFINITION
********************************************************************/

typedef enum {
	e_AI_BASEMEM_UNKNOWN = -1,

	e_AI_DEFAULT_MEM,   // for absolute address in jump, trigger, ...
	e_AI_BASEMEM0,
	e_AI_BASEMEM1,
	e_AI_BASEMEM2,
	e_AI_BASEMEM3,
    e_AI_BASEMEM4,
	
	e_AI_BASEMEM_NUM,

} e_AI_BASEMEM_CONST;

typedef struct {
	e_AI_BASEMEM_CONST type;
	UINT64 offset;
} AI_RELATIVE_BASEMEM;


/********************************************************************
	NUE2
********************************************************************/
typedef enum {
	e_AI_NUE2_MEANSUB_MODE_DC = 0,
	e_AI_NUE2_MEANSUB_MODE_PLANAR,
	e_AI_NUE2_MEANSUB_MODE_NUM
} e_AI_NUE2_MEANSUB_MODE;

typedef enum {
	e_AI_NUE2_YUV_MODE_YUV_420 = 0,
	e_AI_NUE2_YUV_MODE_YVU_420,
	e_AI_NUE2_YUV_MODE_YUV422_UYVY_packed,
	e_AI_NUE2_YUV_MODE_YUV422_YUYV_packed,
	e_AI_NUE2_YUV_MODE_YUV422_VYUY_packed,
	e_AI_NUE2_YUV_MODE_YUV422_YVYU_packed,
	e_AI_NUE2_YUV_MODE_NUM
} e_AI_NUE2_YUV_MODE;

typedef enum {
	e_AI_NUE2_ROTATE_MODE_90 = 0,
	e_AI_NUE2_ROTATE_MODE_90neg,
	e_AI_NUE2_ROTATE_MODE_180,
	e_AI_NUE2_ROTATE_MODE_NUM
} e_AI_NUE2_ROTATE_MODE;

typedef enum {
	e_AI_NUE2_FLIP_MODE_NOFLIP = 0,
	e_AI_NUE2_FLIP_MODE_XFLIP,
	e_AI_NUE2_FLIP_MODE_YFLIP,
	e_AI_NUE2_FLIP_MODE_XYFLIP,
	e_AI_NUE2_FLIP_MODE_NUM
} e_AI_NUE2_FLIP_MODE;

typedef enum {
	e_AI_NUE2_SCALE_MODE_SCALEDOWN = 0,
	e_AI_NUE2_SCALE_MODE_SCALEUP,
	e_AI_NUE2_SCALE_MODE_NUM
} e_AI_NUE2_SCALE_MODE;

typedef enum {
	e_AI_NUE2_MEAN_SCALESHIFT_MODE_TENSOR = 0,
	e_AI_NUE2_MEAN_SCALESHIFT_MODE_CHANNEL,
	e_AI_NUE2_MEAN_SCALESHIFT_MODE_NUM
} e_AI_NUE2_MEAN_SCALESHIFT_MODE;

typedef enum {
	e_AI_NUE2_IN_FMT_YUV420 = 0,
	e_AI_NUE2_IN_FMT_YONLY,
	e_AI_NUE2_IN_FMT_UVONLY,
	e_AI_NUE2_IN_FMT_RGB,
	e_AI_NUE2_IN_FMT_NUM
} e_AI_NUE2_IN_FMT;

typedef struct
{
	// main flow
	BOOL yuv2rgb_en;
	BOOL meansub_en;
	BOOL pad_en;
	BOOL rotate_en;

	UINT32 nue2_in_width;
	UINT32 nue2_in_height;

	BOOL nue2_out_signedness;

	e_AI_NUE2_IN_FMT nue2_in_fmt;
	e_AI_NUE2_YUV_MODE yuv_mode;

	e_AI_NUE2_MEANSUB_MODE meansub_mode;
	e_AI_NUE2_ROTATE_MODE rotate_mode;
	e_AI_NUE2_FLIP_MODE flip_mode;
	e_AI_NUE2_SCALE_MODE scale_mode_horizontal;
	e_AI_NUE2_SCALE_MODE scale_mode_vertical;

	e_AI_NUE2_MEAN_SCALESHIFT_MODE mean_scale_shift_mode;
} s_PARA_NUE2_FLOW;


typedef enum {
	e_AI_NUE2_IO_BASE = 0,

	//engine input
	e_AI_NUE2_IO_INPUT0 = e_AI_NUE2_IO_BASE,
	e_AI_NUE2_IO_INPUT1,
	e_AI_NUE2_IO_INPUT2,

	//engine output
	e_AI_NUE2_IO_OUTPUT0,
	e_AI_NUE2_IO_OUTPUT1,
	e_AI_NUE2_IO_OUTPUT2,

	e_AI_NUE2_IO_NUM
} e_AI_NUE2_IO_CONST;

typedef struct{
	AI_RELATIVE_BASEMEM sa[e_AI_NUE2_IO_NUM];
} s_PARA_NUE2_ADDRESS;

typedef enum {
	e_AI_NUE2_MAIN_IN0 = 0,
	e_AI_NUE2_MAIN_IN1,
	e_AI_NUE2_MAIN_IN2,
	e_AI_NUE2_MAIN_IN_NUM
} e_AI_NUE2_MAIN_IN_IDX;

typedef enum {
	e_AI_NUE2_MAIN_OUT0 = 0,
	e_AI_NUE2_MAIN_OUT1,
	e_AI_NUE2_MAIN_OUT2,
	e_AI_NUE2_MAIN_OUT_NUM
} e_AI_NUE2_MAIN_OUT_IDX;

typedef enum {
	e_AI_NUE2_IO_DATA_FMT_UINT = 0,
	e_AI_NUE2_IO_DATA_FMT_INT,
	e_AI_NUE2_IO_DATA_FMT_NUM
} e_AI_NUE2_IO_DATA_FMT;

typedef struct
{
	INT32 h_dnrate;
	INT32 v_dnrate;

	INT32 h_sfact;
	INT32 v_sfact;

	INT32 ini_h_dnrate;
	INT32 ini_h_sfact;

	INT32 final_h_dnrate;
	INT32 final_h_sfact;

	BOOL h_filt_mode;
	INT32 h_filt_coef;

	BOOL v_filt_mode;
	INT32 v_filt_coef;

	INT32 h_out_scale_size;
	INT32 v_out_scale_size;

} s_PARA_NUE2_SCALE;

typedef struct
{
	INT32 sub_in_width;
	INT32 sub_in_height;

	INT32 sub_coef0;
	INT32 sub_coef1;
	INT32 sub_coef2;

	INT32 sub_dup;

} s_PARA_NUE2_MEANSUB;


typedef struct
{
	INT32 crop_start_x;
	INT32 crop_start_y;

	INT32 crop_width;
	INT32 crop_height;

	INT32 pad_start_x;
	INT32 pad_start_y;

	INT32 pad_out_width;
	INT32 pad_out_height;

	INT32 pad_val0;
	INT32 pad_val1;
	INT32 pad_val2;

} s_PARA_NUE2_PAD;

typedef struct
{
	INT32 shift_dir;
	INT32 shift;
	INT32 scale;
} e_AI_NUE2_SCALESHIFT_FUNC;

typedef struct
{
	e_AI_NUE2_SCALESHIFT_FUNC tensor_scale_shf;
	e_AI_NUE2_SCALESHIFT_FUNC ch_scale_shf[3];

} s_PARA_NUE2_MEANSHIFT;

typedef struct
{
	INT32 coef[9];

	INT32 bias[3];

} s_PARA_NUE2_YUV2RGB;

typedef struct
{
	UINT32 width;
	UINT32 height;
	UINT32 channel;
	UINT32 batch;
	
	BOOL signedness;
	UINT32 bitdepth;

	INT32 lofs;
	INT32 cofs;
	INT32 bofs;
} s_PARA_NUE2_IO_ATTRIBUTE;

typedef struct
{
	s_PARA_NUE2_IO_ATTRIBUTE in[e_AI_NUE2_MAIN_IN_NUM];
	s_PARA_NUE2_IO_ATTRIBUTE out[e_AI_NUE2_MAIN_OUT_NUM];
} s_PARA_NUE2_COMMON;

typedef struct {
	s_PARA_NUE2_ADDRESS addr_ofs;	//for linked list mode
	s_PARA_NUE2_COMMON common;

	s_PARA_NUE2_FLOW flow;
	
	s_PARA_NUE2_SCALE scale_func;
	s_PARA_NUE2_MEANSUB meansub_func;
	s_PARA_NUE2_PAD pad_func;
	s_PARA_NUE2_MEANSHIFT mean_shift_func;
	s_PARA_NUE2_YUV2RGB yuv2rgb_func;
} s_PARA_NUE2;


/********************************************************************
	CAL
********************************************************************/
#define CAL_MAIN_STAGE_NUM 4
#define CAL_MAIN_GROUP_NUM 4


typedef enum {
	e_AI_CAL_IO_BASE = 0,
	//engine input
	e_AI_CAL_IO_IN0 = e_AI_CAL_IO_BASE,
	e_AI_CAL_IO_IN1,
	e_AI_CAL_IO_IN2,
	//input offset
	e_AI_CAL_IO_IN3,	//currently reserved
	e_AI_CAL_IO_IN4,	//currently reserved
	e_AI_CAL_IO_IN5,	//currently reserved
	//engine output
	e_AI_CAL_IO_OUT0,
	e_AI_CAL_IO_OUT1,
	//output offset
	e_AI_CAL_IO_IN6,
	e_AI_CAL_IO_NUM
} e_AI_CAL_IO_CONST;

typedef enum {
	e_AI_CAL_MAIN_IN0 = 0,
	e_AI_CAL_MAIN_IN1,
	e_AI_CAL_MAIN_IN2,
	e_AI_CAL_MAIN_IN_NUM
} e_AI_CAL_MAIN_IN_IDX;

typedef enum {
	e_AI_CAL_MAIN_OUT0 = 0,
	e_AI_CAL_MAIN_OUT1,
	e_AI_CAL_MAIN_OUT_NUM
} e_AI_CAL_MAIN_OUT_IDX;

typedef struct{
	AI_RELATIVE_BASEMEM sa[e_AI_CAL_IO_NUM];
} s_PARA_CAL_ADDRESS;

typedef enum {
	e_AI_CAL_FLOW_STANDARD_FP = 0,
	e_AI_CAL_FLOW_DIRECT_FX,
	e_AI_CAL_FLOW_TYPE_NUM
} e_AI_CAL_FLOW_TYPE;

typedef enum {
	e_AI_CAL_OP_TYPE_ROUNTER = 0,
	e_AI_CAL_OP_TYPE_STANDALONE,
	e_AI_CAL_OP_TYPE_NUM
} e_AI_CAL_OP_TYPE_SEL;

typedef enum {
	e_AI_CAL_GROUP_IDX0 = 0,
	e_AI_CAL_GROUP_IDX1,
	e_AI_CAL_GROUP_IDX2,
	e_AI_CAL_GROUP_IDX3,
	e_AI_CAL_GROUP_NUM
} e_AI_CAL_GROUP_IDX_SEL;

typedef enum {
	e_AI_CAL_STAGE_IDX0 = 0,
	e_AI_CAL_STAGE_IDX1,
	e_AI_CAL_STAGE_IDX2,
	e_AI_CAL_STAGE_IDX3,
	e_AI_CAL_STAGE_NUM
} e_AI_CAL_STAGE_IDX_SEL;

typedef enum {
	e_AI_CAL_GP0_OP_SIGN = 0,
	e_AI_CAL_GP0_OP_ABS,
	e_AI_CAL_GP0_OP_EXP2,
	e_AI_CAL_GP0_OP_EQUAL_T,
	e_AI_CAL_GP0_OP_GREATER_T,
	e_AI_CAL_GP0_OP_LESS_T,
	e_AI_CAL_GP0_OP_GREATEROREQUAL_T,
	e_AI_CAL_GP0_OP_LESSOREQUAL_T,
	e_AI_CAL_GP0_OP_NOTEQUAL_T,
	e_AI_CAL_GP0_OP_MUL_T,
	e_AI_CAL_GP0_OP_ADD1_T = 10,
	e_AI_CAL_GP0_OP_SUB1_T,
	e_AI_CAL_GP0_OP_MATMUL_T,
	e_AI_CAL_GP0_OP_CARTESIANPROD_T,
	e_AI_CAL_GP0_OP_OUTERPROD_T,
	e_AI_CAL_GP0_OP_THRESHOLD_S,
	e_AI_CAL_GP0_OP_ADD1_S,
	e_AI_CAL_GP0_OP_SUB1_S,
	e_AI_CAL_GP0_OP_NUM
} e_AI_CAL_GP0_OP;

typedef enum {
	e_AI_CAL_GP1_OP_NEG = 0,
	e_AI_CAL_GP1_OP_RECIPROCAL,
	e_AI_CAL_GP1_OP_EXP,
	e_AI_CAL_GP1_OP_SQUAREROOT,
	e_AI_CAL_GP1_OP_SQUAREROOTINV,
	e_AI_CAL_GP1_OP_MAX,
	e_AI_CAL_GP1_OP_MIN,
	e_AI_CAL_GP1_OP_NOT,
	e_AI_CAL_GP1_OP_AND_T,
	e_AI_CAL_GP1_OP_OR_T,
	e_AI_CAL_GP1_OP_XOR_T = 10,
	e_AI_CAL_GP1_OP_DIV_S,
	e_AI_CAL_GP1_OP_MUL_S,
	e_AI_CAL_GP1_OP_NUM
} e_AI_CAL_GP1_OP;

typedef enum {
	e_AI_CAL_GP2_OP_DIAGONAL = 0,
	e_AI_CAL_GP2_OP_MEAN,
	e_AI_CAL_GP2_OP_PROD,
	e_AI_CAL_GP2_OP_SQUARE,
	e_AI_CAL_GP2_OP_LOG,
	e_AI_CAL_GP2_OP_POW_T,
	e_AI_CAL_GP2_OP_MAX_T,
	e_AI_CAL_GP2_OP_MIN_T,
	e_AI_CAL_GP2_OP_MOD_T,
	e_AI_CAL_GP2_OP_DIV_T,
	e_AI_CAL_GP2_OP_POW_S = 10,
	e_AI_CAL_GP2_OP_NUM
} e_AI_CAL_GP2_OP;

typedef enum {
	e_AI_CAL_GP3_OP_FLOOR = 0,
	e_AI_CAL_GP3_OP_CEIL,
	e_AI_CAL_GP3_OP_ROUND,
	e_AI_CAL_GP3_OP_CLAMP,
	e_AI_CAL_GP3_OP_SUM,
	e_AI_CAL_GP3_OP_ADD2_T,
	e_AI_CAL_GP3_OP_SUB2_T,
	e_AI_CAL_GP3_OP_ADD2_S,
	e_AI_CAL_GP3_OP_SUB2_S,
	e_AI_CAL_GP3_OP_NUM
} e_AI_CAL_GP3_OP;

typedef enum {
	e_AI_CAL_ST_OP_SORTING = 0,
	e_AI_CAL_ST_OP_GRIDSAMPLE,
	e_AI_CAL_ST_OP_L2NORM,
	e_AI_CAL_ST_OP_L2NORMINV,
	e_AI_CAL_ST_OP_L2VECNORM,
	e_AI_CAL_ST_OP_L1VECNORM,
	e_AI_CAL_ST_OP_BITSHIFT,
	e_AI_CAL_ST_OP_NUM
} e_AI_CAL_ST_OP;

typedef enum {
	e_AI_CAL_BROADCAST_NONE = 0,
	e_AI_CAL_BROADCAST_CH,
	e_AI_CAL_BROADCAST_C,
	e_AI_CAL_BROADCAST_S,
	e_AI_CAL_BROADCAST_NUM
} e_AI_CAL_BROADCAST_MODE;

typedef enum {
	e_AI_CAL_REDUCE_W = 0,
	e_AI_CAL_REDUCE_WH,
	e_AI_CAL_REDUCE_WHC,
	e_AI_CAL_REDUCE_NUM
} e_AI_CAL_REDUCE_AXIS_MODE;

typedef enum {
	e_AI_CAL_SORT_SEL_DESCEND = 0,
	e_AI_CAL_SORT_SEL_ASCEND,
	e_AI_CAL_SORT_SEL_MEDIAN,
	e_AI_CAL_SORT_SEL_NUM
} e_AI_CAL_SORT_SEL;

typedef enum {
	e_AI_CAL_PADDING_MODE_ZERO = 0,
	e_AI_CAL_PADDING_MODE_BORDER,
	e_AI_CAL_PADDING_MODE_NUM
} e_AI_CAL_PADDING_MODE;

typedef enum {
	e_AI_CAL_FMT_UNKNOWN = -1,
	e_AI_CAL_INT2 = 0,
	e_AI_CAL_UINT2,
	e_AI_CAL_INT4,
	e_AI_CAL_UINT4,
	e_AI_CAL_INT8,
	e_AI_CAL_UINT8,
	e_AI_CAL_INT16,
	e_AI_CAL_UINT16,
	e_AI_CAL_FP16,
	e_AI_CAL_FP32,
	e_AI_CAL_UINT32,		// for indices out instead of reg setting
	e_AI_CAL_FMT_SEL_NUM,
} e_AI_CAL_FMT_SEL;

typedef enum {
	e_AI_CAL_FX = 0,
	e_AI_CAL_FP,
	e_AI_CAL_DATA_TYPE_NUM,
} e_AI_CAL_DATA_TYPE;

typedef enum {
	e_AI_CAL_OFFSET_TENSOR = 0,
	e_AI_CAL_OFFSET_CHANNEL,
	e_AI_CAL_OFFSET_TYPE_NUM,
} e_AI_CAL_OFFSET_TYPE;

typedef enum {
	e_AI_CAL_NORM_L1 = 0,
	e_AI_CAL_NORM_L2,
	e_AI_CAL_NORM_TYPE_NUM,
} e_AI_CAL_NORM_TYPE;


typedef struct
{
	UINT32 i_width[e_AI_CAL_MAIN_IN_NUM];
	UINT32 i_height[e_AI_CAL_MAIN_IN_NUM];
	UINT32 i_channel[e_AI_CAL_MAIN_IN_NUM];
	UINT32 i_bitdepth[e_AI_CAL_MAIN_IN_NUM];
	UINT32 i_datafmt[e_AI_CAL_MAIN_IN_NUM];
	UINT32 i_lofs_en[e_AI_CAL_MAIN_IN_NUM];
	UINT32 i_lofs[e_AI_CAL_MAIN_IN_NUM];
	UINT32 i_cofs_en[e_AI_CAL_MAIN_IN_NUM];
	UINT32 i_cofs[e_AI_CAL_MAIN_IN_NUM];

	UINT32 o_width[e_AI_CAL_MAIN_OUT_NUM];
	UINT32 o_height[e_AI_CAL_MAIN_OUT_NUM];
	UINT32 o_channel[e_AI_CAL_MAIN_OUT_NUM];
	UINT32 o_bitdepth[e_AI_CAL_MAIN_OUT_NUM];
	UINT32 o_datafmt[e_AI_CAL_MAIN_OUT_NUM];
	UINT32 o_lofs_en[e_AI_CAL_MAIN_OUT_NUM];
	UINT32 o_lofs[e_AI_CAL_MAIN_OUT_NUM];
	UINT32 o_cofs_en[e_AI_CAL_MAIN_OUT_NUM];
	UINT32 o_cofs[e_AI_CAL_MAIN_OUT_NUM];

	FLOAT i_byte[e_AI_CAL_MAIN_IN_NUM];
	FLOAT o_byte[e_AI_CAL_MAIN_OUT_NUM];

	e_AI_CAL_FMT_SEL i_fmt[e_AI_CAL_MAIN_IN_NUM];
	e_AI_CAL_FMT_SEL o_fmt[e_AI_CAL_MAIN_OUT_NUM];

	UINT32 i_fracbit[e_AI_CAL_MAIN_IN_NUM];
	UINT32 o_fracbit[e_AI_CAL_MAIN_OUT_NUM];
	// note that if offset == 0, it means the data is continuous

	// --- set by refresh_para() ---
	// o_width[e_AI_CAL_MAIN_OUT_NUM];
	// o_height[e_AI_CAL_MAIN_OUT_NUM];
	// o_channel[e_AI_CAL_MAIN_OUT_NUM];
	// o_bitdepth[e_AI_CAL_MAIN_OUT_NUM];
	// o_datafmt[e_AI_CAL_MAIN_OUT_NUM];
	// o_lofs_en[e_AI_CAL_MAIN_OUT_NUM];
	// o_lofs[e_AI_CAL_MAIN_OUT_NUM];
	// o_cofs_en[e_AI_CAL_MAIN_OUT_NUM];
	// o_cofs[e_AI_CAL_MAIN_OUT_NUM];

	// i_byte[e_AI_CAL_MAIN_IN_NUM];
	// o_byte[e_AI_CAL_MAIN_OUT_NUM];

	// i_fmt[e_AI_CAL_MAIN_IN_NUM];
	// o_fmt[e_AI_CAL_MAIN_OUT_NUM];
} s_PARA_CAL_COMMON;

typedef struct
{
	UINT32 width;
	UINT32 height;
	UINT32 channel;
} s_PARA_CAL_GP_SIZE;

typedef struct
{
	BOOL shift_dir;
	INT32 shift;
	INT32 scale;
	INT32 offset;
} s_PARA_CAL_T_SFO;


typedef struct
{
	e_AI_CAL_FLOW_TYPE flow_type;
	e_AI_CAL_OP_TYPE_SEL op_type;
	BOOL in1_en;
	BOOL in2_en;
	BOOL out1_en;
	BOOL stage_en[CAL_MAIN_STAGE_NUM];
	BOOL stage_role_en[CAL_MAIN_STAGE_NUM];
	BOOL in_remain_en;
	UINT32 remain_idx;
	e_AI_CAL_GROUP_IDX_SEL stage_group_idx[CAL_MAIN_STAGE_NUM];
	s_PARA_CAL_GP_SIZE in0_gp_size[CAL_MAIN_GROUP_NUM];
	s_PARA_CAL_GP_SIZE in1_gp_size;			//currently only group-0 use
	e_AI_CAL_STAGE_IDX_SEL in1_stage_idx;
	e_AI_CAL_STAGE_IDX_SEL in2_stage_idx;
	e_AI_CAL_GP0_OP gp0_op;
	e_AI_CAL_GP1_OP gp1_op;
	e_AI_CAL_GP2_OP gp2_op;
	e_AI_CAL_GP3_OP gp3_op;
	e_AI_CAL_ST_OP st_op;

	// --- set by refresh_para() ---
	// flow_type;
} s_PARA_CAL_FLOW;

typedef struct
{
	e_AI_CAL_BROADCAST_MODE in_broadcast[e_AI_CAL_MAIN_IN_NUM];
	e_AI_CAL_REDUCE_AXIS_MODE gp_reduce_axis[CAL_MAIN_GROUP_NUM];
	e_AI_CAL_SORT_SEL sort_sel;
	e_AI_CAL_PADDING_MODE gridsample_padding_mode;
	e_AI_CAL_OFFSET_TYPE in_ofs_type[e_AI_CAL_MAIN_IN_NUM];
	e_AI_CAL_OFFSET_TYPE out_ofs_type[e_AI_CAL_MAIN_OUT_NUM];
	s_PARA_CAL_T_SFO in_t_sf_ofs[e_AI_CAL_MAIN_IN_NUM];
	s_PARA_CAL_T_SFO out_t_sf_ofs[e_AI_CAL_MAIN_OUT_NUM];
	BOOL sort_indices_en;
	BOOL max_indices_en;
	BOOL min_indices_en;
	BOOL gridsample_align_cor_en;
	UINT32 root_refine_iter;
	UINT32 sort_top_n;
	UINT32 bitshift_dir;
	UINT32 gp_scalar_val[CAL_MAIN_GROUP_NUM];
	UINT32 log_base_alpha_val;
	UINT32 thres_val;
	UINT32 thres_replace_val;
	UINT32 clamp_low_bound_val;
	UINT32 clamp_high_bound_val;
	UINT32 indices_gap;

	// --- set by refresh_para() ---
	// indices_gap
} s_PARA_CAL_ATTRIBUTE;

typedef struct {
	s_PARA_CAL_ADDRESS addr_ofs;	//for linked list mode
	s_PARA_CAL_COMMON common;
	s_PARA_CAL_FLOW flow;
	s_PARA_CAL_ATTRIBUTE attr;
} s_PARA_CAL;

/********************************************************************
	PPU
********************************************************************/
typedef enum {
	e_AI_PPU_MODE_PAD = 0,
	e_AI_PPU_MODE_CROP,
	e_AI_PPU_PAD_CROP_MODE_NUM
} e_AI_PPU_PAD_CROP_MODE;

typedef enum {
	e_AI_PPU_PAD_EDGE = 0,
	e_AI_PPU_PAD_CONSTANT = 1,
	e_AI_PPU_PAD_MODE_NUM
} e_AI_PPU_PAD_MODE;

typedef enum {
	e_AI_PPU_PAD_EDGE_MODE_1PIXEL = 0,
	e_AI_PPU_PAD_EDGE_MODE_2PIXEL = 1,
	e_AI_PPU_PAD_EDGE_MODE_NUM
} e_AI_PPU_PAD_EDGE_MODE;

typedef enum {
	e_AI_PPU_MEAN_SCALESHIFT_MODE_TENSOR = 0,
	e_AI_PPU_MEAN_SCALESHIFT_MODE_CHANNEL,
	e_AI_PPU_MEAN_SCALESHIFT_MODE_NUM
} e_AI_PPU_MEAN_SCALESHIFT_MODE;

typedef enum {
	e_AI_PPU_IO_FMT_YUV420 = 0,
	e_AI_PPU_IO_FMT_RGB,
	e_AI_PPU_IO_FMT_BAYER12BIT,
	e_AI_PPU_IO_FMT_BAYER16BIT,
	e_AI_PPU_IO_FMT_YONLY,
	e_AI_PPU_IO_FMT_NUM
} e_AI_PPU_IO_FMT;

typedef struct
{
	UINT32 ppu_in_width;
	UINT32 ppu_in_height;

	BOOL ppu_out_signedness;

	// main flow
	BOOL meansub_en;
	e_AI_PPU_IO_FMT ppu_in_fmt;
	e_AI_PPU_IO_FMT ppu_out_fmt;
	e_AI_PPU_PAD_CROP_MODE pad_crop_mode;
	e_AI_PPU_PAD_MODE pad_mode;
	e_AI_PPU_PAD_EDGE_MODE pad_edge_mode_horizontal;
	e_AI_PPU_PAD_EDGE_MODE pad_edge_mode_vertical;

	e_AI_PPU_MEAN_SCALESHIFT_MODE mean_scale_shift_mode;
} s_PARA_PPU_FLOW;


typedef enum {
	e_AI_PPU_IO_BASE = 0,

	//engine input
	e_AI_PPU_IO_INPUT0 = e_AI_PPU_IO_BASE,
	e_AI_PPU_IO_INPUT1,
	e_AI_PPU_IO_INPUT2,

	//engine output
	e_AI_PPU_IO_OUTPUT0,
	e_AI_PPU_IO_OUTPUT1,
	e_AI_PPU_IO_OUTPUT2,

	e_AI_PPU_IO_NUM
} e_AI_PPU_IO_CONST;

typedef struct{
	AI_RELATIVE_BASEMEM sa[e_AI_PPU_IO_NUM];
} s_PARA_PPU_ADDRESS;

typedef enum {
	e_AI_PPU_MAIN_IN0 = 0,
	e_AI_PPU_MAIN_IN1,
	e_AI_PPU_MAIN_IN2,
	e_AI_PPU_MAIN_IN_NUM
} e_AI_PPU_MAIN_IN_IDX;

typedef enum {
	e_AI_PPU_MAIN_OUT0 = 0,
	e_AI_PPU_MAIN_OUT1,
	e_AI_PPU_MAIN_OUT2,
	e_AI_PPU_MAIN_OUT_NUM
} e_AI_PPU_MAIN_OUT_IDX;

typedef enum {
	e_AI_PPU_IO_DATA_FMT_UINT = 0,
	e_AI_PPU_IO_DATA_FMT_INT,
	e_AI_PPU_IO_DATA_FMT_NUM
} e_AI_PPU_IO_DATA_FMT;


typedef struct
{
	INT32 sub_coef0;
	INT32 sub_coef1;
	INT32 sub_coef2;

} s_PARA_PPU_MEANSUB;


typedef struct
{
	INT32 pad_top_num;
	INT32 pad_bottom_num;

	INT32 pad_left_num;
	INT32 pad_right_num;

	INT32 pad_val0;
	INT32 pad_val1;
	INT32 pad_val2;

} s_PARA_PPU_PAD;

typedef struct
{
	INT32 crop_start_x;
	INT32 crop_start_y;

	INT32 crop_out_width;
	INT32 crop_out_height;

} s_PARA_PPU_CROP;

typedef struct
{
	INT32 shift_dir;
	INT32 shift;
	INT32 scale;
} e_AI_PPU_SCALESHIFT_FUNC;

typedef struct
{
	e_AI_PPU_SCALESHIFT_FUNC tensor_scale_shf;
	e_AI_PPU_SCALESHIFT_FUNC ch_scale_shf[3];

} s_PARA_PPU_MEANSHIFT;

typedef struct
{
	INT32 coef[9];
	INT32 bias[3];

} s_PARA_PPU_FMT_CONVERT;
#if !defined(_BSP_NS02201_)
typedef struct
{
	BOOL shift_dir;
	INT32 shift;
	INT32 scale;
	INT32 offset;
} s_PARA_PPU_QUAN_INFO;
#endif
typedef struct
{
	UINT32 width;
	UINT32 height;
	UINT32 channel;
	UINT32 batch;
	
	BOOL signedness;
	UINT32 bitdepth;

	BOOL lofs_en;
	BOOL cofs_en;
	BOOL bofs_en;

	INT32 lofs;
	INT32 cofs;
	INT32 bofs;
#if !defined(_BSP_NS02201_)
	s_PARA_PPU_QUAN_INFO quan_info;
#endif
} s_PARA_PPU_IO_ATTRIBUTE;

typedef struct
{
	s_PARA_PPU_IO_ATTRIBUTE in[e_AI_PPU_MAIN_IN_NUM];
	s_PARA_PPU_IO_ATTRIBUTE out[e_AI_PPU_MAIN_OUT_NUM];

	// --- set by refresh_para() ---
	// in[e_AI_PPU_MAIN_IN0]:
	//     width
	//	   height
	//	   channel
	//     batch
	//     bitdepth
	//     signedness
	// in[e_AI_PPU_MAIN_IN1]:
	//     width
	//	   height
	//	   channel
	//     batch
	//     bitdepth
	//     signedness
	// in[e_AI_PPU_MAIN_IN2]:
	//     width
	//	   height
	//	   channel
	//     batch
	//     bitdepth
	//     signedness
	// out[e_AI_PPU_MAIN_OUT0]:
	//     width
	//	   height
	//	   channel
	//     batch
	//     bitdepth
	//     signedness
	// out[e_AI_PPU_MAIN_OUT1]:
	//     width
	//	   height
	//	   channel
	//     batch
	//     bitdepth
	//     signedness
	// out[e_AI_PPU_MAIN_OUT2]:
	//     width
	//	   height
	//	   channel
	//     batch
	//     bitdepth
	//     signedness

} s_PARA_PPU_COMMON;

typedef struct {
	s_PARA_PPU_ADDRESS addr_ofs;	//for linked list mode
	s_PARA_PPU_COMMON common;
	s_PARA_PPU_FLOW flow;
	s_PARA_PPU_MEANSUB meansub_func;
	s_PARA_PPU_PAD pad_func;
	s_PARA_PPU_CROP crop_func;
	s_PARA_PPU_MEANSHIFT mean_shift_func;
	s_PARA_PPU_FMT_CONVERT fmt_convert_func;
} s_PARA_PPU;

/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
extern UINT32 get_nue2_max_ll_cmd_lines(VOID);
extern UINT32 nn_gen_nue2_ll_cmd(s_PARA_NUE2* p_parm, UINT64 *p_ll_cmd, UINT32 ll_cmd_sz);
extern UINT32 get_cal_max_ll_cmd_lines(VOID);
extern UINT32 nn_gen_cal_ll_cmd(s_PARA_CAL* p_parm, UINT64 *p_ll_cmd, UINT32 ll_cmd_sz);
extern UINT32 get_ppu_max_ll_cmd_lines(VOID);
extern UINT32 nn_gen_ppu_ll_cmd(s_PARA_PPU* p_parm, UINT64 *p_ll_cmd, UINT32 ll_cmd_sz);

#endif  /* _VENDOR_AI_GENLL_H_ */
