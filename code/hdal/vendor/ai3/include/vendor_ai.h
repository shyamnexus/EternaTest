/**
	@brief Header file of definition of vendor user-space net flow sample.

	@file vendor_ai.h

	@ingroup vendor_ai

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _VENDOR_AI3_NET_H_
#define _VENDOR_AI3_NET_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "hd_common.h"

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/
#ifndef VENDOR_AI_VERSION
#define VENDOR_AI_VERSION                           0x030000
#endif

/* ai buf format */
#define HD_VIDEO_PXLFMT_AI_SINT8                    0xa1080000 ///< signed int 8-bits
#define HD_VIDEO_PXLFMT_AI_UINT8                    0xa2080000 ///< unsigned int 8-bits
#define HD_VIDEO_PXLFMT_AI_SINT16                   0xa1100000 ///< signed int 16-bits
#define HD_VIDEO_PXLFMT_AI_UINT16                   0xa2100000 ///< unsigned int 16-bits
#define HD_VIDEO_PXLFMT_AI_FLOAT16                  0xa3100000 ///< float 16-bits
#define HD_VIDEO_PXLFMT_AI_SINT32                   0xa1200000 ///< signed int 32-bits
#define HD_VIDEO_PXLFMT_AI_UINT32                   0xa2200000 ///< unsigned int 32-bits
#define HD_VIDEO_PXLFMT_AI_FLOAT32                  0xa3200000 ///< float 32-bits

#define HD_VIDEO_PXLFMT_CLASS_AI                    0xa
#define HD_VIDEO_PXLFMT_TYPE_MASK                   0x0f000000
#define HD_VIDEO_PXLFMT_BITS_MASK                   0x00ff0000
#define HD_VIDEO_PXLFMT_INT_MASK                    0x0000ff00
#define HD_VIDEO_PXLFMT_FRAC_MASK                   0x000000ff
#define HD_VIDEO_PXLFMT_SIGN(pxlfmt)                ((((pxlfmt) & HD_VIDEO_PXLFMT_TYPE_MASK) >> 24) & 0x01)
#define HD_VIDEO_PXLFMT_BITS(pxlfmt)                (((pxlfmt) & HD_VIDEO_PXLFMT_BITS_MASK) >> 16)
#define HD_VIDEO_PXLFMT_INT(pxlfmt)                 (INT8)(((pxlfmt) & HD_VIDEO_PXLFMT_INT_MASK) >> 8)
#define HD_VIDEO_PXLFMT_FRAC(pxlfmt)                (INT8)((pxlfmt) & HD_VIDEO_PXLFMT_FRAC_MASK)
#define HD_VIDEO_PXLFMT_FMT(pxlfmt)                 ((pxlfmt) & (((HD_VIDEO_PXLFMT_CLASS_AI << 28) | HD_VIDEO_PXLFMT_TYPE_MASK) | HD_VIDEO_PXLFMT_BITS_MASK))

#define HD_VIDEO_PXLFMT_AI_SFIXED8(frac_bits)       (HD_VIDEO_PXLFMT_AI_SINT8 | (((8-(frac_bits)-1)&0xff) << 8) | ((frac_bits)&0xff))   ///< signed fixpoint 8-bits
#define HD_VIDEO_PXLFMT_AI_UFIXED8(frac_bits)       (HD_VIDEO_PXLFMT_AI_UINT8 | (((8-(frac_bits))&0xff) << 8) | ((frac_bits)&0xff))   ///< unsigned fixpoint 8-bits
#define HD_VIDEO_PXLFMT_AI_SFIXED16(frac_bits)      (HD_VIDEO_PXLFMT_AI_SINT16 | (((16-(frac_bits)-1)&0xff) << 8) | ((frac_bits)&0xff))   ///< signed fixpoint 16-bits
#define HD_VIDEO_PXLFMT_AI_UFIXED16(frac_bits)      (HD_VIDEO_PXLFMT_AI_UINT16 | (((16-(frac_bits))&0xff) << 8) | ((frac_bits)&0xff))   ///< unsigned fixpoint 16-bits
#define HD_VIDEO_PXLFMT_AI_SFIXED32(frac_bits)      (HD_VIDEO_PXLFMT_AI_SINT32 | (((32-(frac_bits)-1)&0xff) << 8) | ((frac_bits)&0xff))   ///< signed fixpoint 32-bits
#define HD_VIDEO_PXLFMT_AI_UFIXED32(frac_bits)      (HD_VIDEO_PXLFMT_AI_UINT32 | (((32-(frac_bits))&0xff) << 8) | ((frac_bits)&0xff))   ///< unsigned fixpoint 32-bits


/* ai buf format (addiitonal) */
#define HD_VIDEO_PXLFMT_BGR888_PLANAR               0x2B180888  ///< 3 plane, pixel=B(w,h), G(w,h), R(w,h)
#define HD_VIDEO_PXLFMT_UV                          0x51100044  ///< 1 plane, pixel=UV(w,h)
#define HD_VIDEO_PXLFMT_R8                          0x21080800  ///< 1 plane, pixel=R(w,h)
#define	HD_VIDEO_PXLFMT_G8                          0x21080080  ///< 1 plane, pixel=G(w,h)
#define	HD_VIDEO_PXLFMT_B8                          0x21080008  ///< 1 plane, pixel=B(w,h)

/* YUV420 NV12 / NV21 */
#define AI_PXLFMT_YUV420_NV12                       0x00000000  ///< YUV420 NV12 (default of 52x and 56x)
#define AI_PXLFMT_YUV420_NV21                       0x80000000  ///< YUV420 NV21

/* type of AI_PXLFMT */
#define AI_PXLFMT_TYPE(pxlfmt)    					(((pxlfmt) & (HD_VIDEO_PXLFMT_CLASS_MASK | HD_VIDEO_PXLFMT_PLANE_MASK | HD_VIDEO_PXLFMT_BPP_MASK)))

/* ai output buffer real access size */
#define AI_BUF_SIZE(w, h, c, b, fmt)  				(w * h * c * b * (HD_VIDEO_PXLFMT_BITS(fmt) >> 3))

#define ISP_POOL_ID(id)            (0x80 | (id))
#define POOL_ID(id)                (0x80 | (id))
/********************************************************************
	TYPE DEFINITION
********************************************************************/

typedef enum _AI3_PLUGIN_ID {
    AI3_PLUGIN_CPU = 0,
    AI3_PLUGIN_DSP = 1,
} AI3_PLUGIN_ID;

typedef enum _AI3_ISP_MODE {
    AI3_ISP_SINGLE_MODE = 0,
    AI3_ISP_MULTI_ISO_MODE = 1,
    AI3_ISP_MULTI_DEV_MODE = 2,
} AI3_ISP_MODE;
typedef enum _AI3_PROC_CFG_ID {
    AI3_PROC_CFG_ISP_POOL_ID = 0, ///< pool_id which applied to isp model, user must assign it, no default value
    AI3_PROC_CFG_ISP_MODE   = 1, ///< isp model share mode
} AI3_PROC_CFG_ID;

typedef enum _AI3_MODEL_TYPE {
    AI3_COMMON_MODEL = 0,
    AI3_AIISP_MODEL = 1,
    AI3_GTILE_MODEL = 2,
    AI3_DUBUF_MODEL = 3,
} AI3_MODEL_TYPE;

#define AI3_PROC_CFG_ISP_SHARE AI3_PROC_CFG_ISP_MODE
#define AI3_ISP_SIMGLE_MODE AI3_ISP_SINGLE_MODE
#define AI3_PROC_CFG_POOL_ID AI3_PROC_CFG_ISP_POOL_ID
#define AI3_DEV_PLUGIN_MAX   8
#define AI3_DEV_INFO_MAX 64

#define VENDOR_AI3_DEV_TIMELINE      0x00000001  //enable perf timeline
typedef enum _VENDOR_AI3_DEV_INFO_ID {
	VENDOR_AI3_DEV_TIMELINE_FUNC         = 0,   ///< set timeline func_mask
    VENDOR_AI3_DEV_TIMELINE_BUFSIZE      = 1,   ///< set timeline buffer size
	ENUM_DUMMY4WORD(VENDOR_AI3_DEV_INFO_ID)
} VENDOR_AI3_DEV_INFO_ID;

typedef struct _VENDOR_AI3_DEV_CFG {
    UINT32 ctrl;                            	///< ctrl, reserved
    UINT32 mode;                            	///< mode, reserved
    UINT64 info[AI3_DEV_INFO_MAX];          	///< info, reserved
} VENDOR_AI3_DEV_CFG;

#define DEV_INFO_CORE_MAX     0 ///< num of cores of this chip
#define DEV_INFO_UBUF_MAX     1 ///< num of ubufs of this chip
#define DEV_INFO_CORE_AIISP   2 ///< ai_maxisp in dtsi setting
#define DEV_INFO_UBUF_AIISP   3 ///< ai_maxubuf in dtsi setting
typedef struct _VENDOR_AI3_DEV_INFO {
	UINT32 config[AI3_DEV_INFO_MAX];  ///< dev info
} VENDOR_AI3_DEV_INFO;
/**
	@name ai implement version
*/
#define AI3_VER_LEN	32
typedef struct _VENDOR_AI3_VER {
	CHAR vendor_ai_impl_version[AI3_VER_LEN];	///< lib version
	CHAR kflow_ai_impl_version[AI3_VER_LEN];	///< kflow version
	CHAR kdrv_ai_impl_version[AI3_VER_LEN];		///< kdrv version
} VENDOR_AI3_VER;

typedef struct _VENDOR_AI3_VER2 {
	CHAR vendor_ai_impl_version[AI3_VER_LEN];	///< lib version
	CHAR kflow_ai_impl_version[AI3_VER_LEN];	///< kflow version
    CHAR kflow_ai_isp_impl_version[AI3_VER_LEN];	///< kflow_isp version
	CHAR kdrv_ai_impl_version[AI3_VER_LEN];		///< kdrv version
} VENDOR_AI3_VER2;

#define AI3_ENGINE_NAME_LEN   8
typedef struct _VENDOR_AI3_CORE_UT {
    CHAR   name[AI3_ENGINE_NAME_LEN];          ///< engine name
    UINT32 time;                               ///< time
    UINT32 util;                               ///< utility
} VENDOR_AI3_CORE_UT;

#define AI3_CORE_MAX   32
typedef struct _VENDOR_AI3_PERF_TIME_UT {
    UINT32 core_count;
    VENDOR_AI3_CORE_UT core[AI3_CORE_MAX];
} VENDOR_AI3_PERF_TIME_UT;

typedef struct _VENDOR_AI3_CFG_BUF {
    uintptr_t pa;                           	///< physical address
    uintptr_t va;                           	///< virtual address
    UINT32 size;                            	///< buffer size
} VENDOR_AI3_CFG_BUF;


/////////////////////////////////////////////////////////////////

#define CTRL_BUF_DEBUG      0x00000001
#define CTRL_JOB_DEBUG      0x00000002
#define CTRL_JOB_DUMPOUT    0x00000004
#define CTRL_JOB_DUMPCMD    0x00000008

#define CTRL_JOB_PERFTIME   0x00000010
#define CTRL_JOB_PERFBW     0x00000020

/* FLOATIN  FLOATOUT Automation */
#define CTRL_BUF_FLOATIN    0x00001000
#define CTRL_BUF_FLOATOUT   0x00002000

#define VENDOR_AI_JOB_PRI(pri)     (pri) ///< dla core bit
#define VENDOR_AI_JOB_PRI_DEFAULT  VENDOR_AI_JOB_PRI(0)

#define VENDOR_AI_DLA_UBUF(ch)                     (1L << (ch)) ///< dla UBUF bit

#define VENDOR_AI_DLA_CORE(ch)                     (1L << (ch)) ///< dla core bit
#define VENDOR_AI_CPU_CORE(ch)                     (1L << ((ch)+8)) ///< cpu core bit
#define VENDOR_AI_DSP_CORE(ch)                     (1L << ((ch)+16)) ///< dsp core bit
#define VENDOR_AI_GPU_CORE(ch)                     (1L << ((ch)+24)) ///< gpu core bit

#define VENDOR_AI_DLA_CORE_ALL                     0x000000ff ///< dla core all bits
#define VENDOR_AI_CPU_CORE_ALL                     0x0000ff00 ///< cpu core all bits
#define VENDOR_AI_DSP_CORE_ALL                     0x00ff0000 ///< dsp core all bits
#define VENDOR_AI_GPU_CORE_ALL                     0xff000000 ///< gpu core all bits

#define VENDOR_AI_CORE_MASK_DEFAULT                0xffffffff ////< all bits


typedef enum _AI3_PROC_BUF_ID {
    AI3_PROC_BUF_WORKBUF  = 0,
    AI3_PROC_BUF_RONLYBUF = 1,
} AI3_PROC_BUF_ID;

#define AI3_PROC_BUF_MAX   8
typedef struct _VENDOR_AI3_PROC_MEM {
    VENDOR_AI3_CFG_BUF buf[AI3_PROC_BUF_MAX];	///< proc require buffer, using AI3_PROC_BUF_ID as index
} VENDOR_AI3_PROC_MEM;

#define MODEL_INFO_CORE_NUM   0
#define MODEL_INFO_UBUF_NUM   1
#define AI3_MODEL_CFG_MAX 16
typedef struct _VENDOR_AI3_MODEL_INFO {
    VENDOR_AI3_CFG_BUF model_buf;           	///< model buffer
    VENDOR_AI3_PROC_MEM proc_mem;           	///< pre-proc require buffer
    UINT32 type;                            	///< model type, using emu AI3_MODEL_TYPE
    UINT32 attr;                            	///< model attr, reserved
    UINT32 ctrl;                            	///< ctrl, bitwise mask using CTRL_XXXX
    UINT32 mode;                            	///< mode, reserved
    UINT64 config[AI3_MODEL_CFG_MAX];       	///< require config, reserved
} VENDOR_AI3_MODEL_INFO;

#define AI3_PROC_CFG_MAX   64
typedef struct _VENDOR_AI3_PROC_CFG {
    VENDOR_AI3_CFG_BUF model_buf;           	///< model buffer
    VENDOR_AI3_PROC_MEM proc_mem;           	///< pre-proc require buffer
    UINT32 ctrl;                            	///< ctrl, bitwise mask using CTRL_XXXX
    UINT32 mode;                            	///< mode, reserved
    void*  plugin[AI3_DEV_PLUGIN_MAX];      	///< plugin , using emu AI3_PLUGIN_ID as index
    UINT64 config[AI3_PROC_CFG_MAX];        	///< require config, reserved, using emu AI3_PROC_CFG_ID as index
} VENDOR_AI3_PROC_CFG;

#define AI3_NET_INFO_MAX   32
typedef struct _VENDOR_AI3_NET_INFO {
    // job info
    UINT32 layer_cnt;                       	///< layer count
    UINT32 bind_cnt;                        	///< bind count
    // buf info
    UINT32 buf_cnt;                         	///< io buffer count
    // open buf info
    UINT32 in_buf_cnt;                      	///< in buffer count
    UINT32 out_buf_cnt;                     	///< out buffer count
    UINT32* in_path_list;                   	///< in buffer path list
    UINT32* out_path_list;                  	///< out buffer path list
    // others
    UINT32 info[AI3_NET_INFO_MAX];          	///< other info, reserved
} VENDOR_AI3_NET_INFO;

typedef struct _VENDOR_AI3_BUF_INFO {
    HD_VIDEO_PXLFMT fmt;                        ///< format
    UINT32 width;                               ///< width
    UINT32 height;                              ///< height
    UINT32 channel;                             ///< channel
    UINT32 batch_num;                           ///< number of batch
    UINT32 time;                                ///< number of time
    CHAR   layout[8];                           ///< layout order of W,H,C,N
    CHAR  *name;                                ///< buffer name, eg: "mylayer.out0"
} VENDOR_AI3_BUF_INFO;

typedef struct _VENDOR_AI3_IO_INFO {
    VENDOR_AI3_CFG_BUF model_buf;               ///< model buffer
    UINT32 in_buf_cnt;                          ///< in buffer count
    UINT32 out_buf_cnt;                         ///< out buffer count
    VENDOR_AI3_BUF_INFO* in_buf_info;           ///< in buffer info  (array)
    VENDOR_AI3_BUF_INFO* out_buf_info;          ///< out buffer info (array)
} VENDOR_AI3_IO_BUF_INFO;

/**
	@name ai parameter id
*/
typedef enum _VENDOR_AI3_CFG_ID {

	// get after init
	VENDOR_AI3_CFG_VER                 =   0,   ///< get    , implement version, using VENDOR_AI3_VER struct

	// get anytime
	VENDOR_AI3_CFG_MODEL_INFO          =   1,   ///< get    , model info, using VENDOR_AI3_MODEL_INFO struct
	VENDOR_AI3_CFG_IO_CNT              =   2,   ///< get    , query in/out buffer count, using VENDOR_AI3_IO_BUF_INFO struct
	VENDOR_AI3_CFG_IO_INFO             =   3,   ///< get    , query in/out buffer info,  using VENDOR_AI3_IO_BUF_INFO struct
    VENDOR_AI3_CFG_VER2                =   4,   ///< get    , implement version, using VENDOR_AI3_VER2 struct
    VENDOR_AI3_CFG_EOP_MAX             =   5,   ///< get    , elementary op workbuf size, only eop using
    VENDOR_AI3_CFG_DEV_INFO            =   6,   ///< get    , ai_maxisp, ai_maxubuf in dtsi
	VENDOR_AI3_CFG_AI_CB               =  20,
	VENDOR_AI3_CFG_ISP_CB              =  21,
    VENDOR_AI3_CFG_CORE_USAGE_LIMIT    =  22,   ///< set    , set core usage limit of normal network (range 0~100, default 100)
    VENDOR_AI3_CFG_NOTCHECK_INPUT_ALIGN  =  23, ///< set    ,  set not check input align
    VENDOR_AI3_CFG_NOTCHECK_OUTPUT_ALIGN =  24, ///< set    ,  set not check output align
	ENUM_DUMMY4WORD(VENDOR_AI3_CFG_ID)
} VENDOR_AI3_CFG_ID;

typedef enum _VENDOR_AI3_PERF_ID {
	VENDOR_AI3_PERF_ID_TIME_UT         =   1,   ///< perf time utility with struct VENDOR_AI3_PERF_TIME_UT
    VENDOR_AI3_PERF_ID_TIMELINE        =   2,   ///  dump all net timeline to /mnt/sd/kflow_ai_timeline_all.html
	ENUM_DUMMY4WORD(VENDOR_AI3_PERF_ID)
} VENDOR_AI3_PERF_ID;

/////////////////////////////////////////////////////////////////

#define AI3_BUF_INFO_MAX	32
/**
	@name input buffer / output buffer
*/
typedef struct _VENDOR_AI3_BUF {
	UINT32 sign;                            	///< signature = MAKEFOURCC('A','B','U','F')
	UINT32 chunk_size;                      	///< sizeof(this sturct) - 8;
	uintptr_t pa;                           	///< physical address
	uintptr_t va;                           	///< virtual address
	UINT32 size;                            	///< buffer size
	UINT32 size_real;             				///< real effective buffer size
	HD_VIDEO_PXLFMT fmt;                    	///< format
	UINT32 width;                           	///< width
	UINT32 height;                          	///< height
	UINT32 channel;                         	///< channel
	UINT32 batch_num;                       	///< number of batch
	UINT32 time;                            	///< number of time
	UINT64 line_ofs;                        	///< line offset
	UINT64 channel_ofs;                     	///< channel offset
	UINT64 batch_ofs;                       	///< batch offset
	UINT64 time_ofs;                        	///< time offset
	union {
		UINT32 reserve;                     	///< reserved
		FLOAT scale_ratio;                  	///< for quant & dequant
	};
	INT32 zero_point;      					///< for quant & dequant (for asymmtrix-quant)
	CHAR   layout[8];                       	///< layout order of W,H,C,N
	CHAR  *name;                            	///< buffer name, eg: "mylayer.out0"
	struct _VENDOR_AI3_BUF *p_next;                    ///< pointer to next buf
	UINT32 info[AI3_BUF_INFO_MAX];          	///< other info, reserved
} VENDOR_AI3_BUF;

typedef enum _VENDOR_AI3_NET_PARAM_ID {

    VENDOR_AI3_NET_PARAM_SHAPE_DIM_IMM           = 16, ///< get/set, set resolution by dim, using HD_DIM struct, MUST set after START
    VENDOR_AI3_NET_PARAM_BATCH_N_IMM             = 17, ///< get/set, set batch number, using UINT32
	VENDOR_AI3_NET_PARAM_CUSTOM_INFO			 = 18, ///<     set, set user info
    VENDOR_AI3_NET_PARAM_JOB_PRI                 = 20, ///<     set, set job priority
    VENDOR_AI3_NET_PARAM_CORE_MASK               = 21, ///<     set, set core mask
    VENDOR_AI3_NET_PARAM_UBUF_MASK               = 22, ///<     set, set ub mask 
    VENDOR_AI3_NET_PARAM_CFG_WORKBUF_IMM         = 54, ///<     set, set workbuf, using VENDOR_AI3_CFG_BUF struct, MUST set after START
    	
    ENUM_DUMMY4WORD(VENDOR_AI3_NET_PARAM_ID)
} VENDOR_AI3_NET_PARAM_ID;

/********************************************************************
	SINGLE OPERATION
********************************************************************/
typedef struct _VENDOR_AI3_OP_FC_MEM {
    VENDOR_AI3_CFG_BUF buf;                     ///< work buf require 
    HD_DIM input1 ; 
    HD_DIM input2 ; 
    UINT32 swap_out;                            ///< output to temp_buf
    UINT32 out_fmt;
} VENDOR_AI3_OP_FC_MEM;

typedef struct _VENDOR_AI3_OP_MATRIX_MEM {
    VENDOR_AI3_CFG_BUF buf;                     ///< work buf require 
    HD_DIM input1 ; 
    HD_DIM input2 ; 
    UINT32 out_fmt;
} VENDOR_AI3_OP_MATRIX_MEM;

typedef struct _VENDOR_AI3_OP_PREPROC_MEM {
    UINT32 ctrl;                                ///< reserved
    VENDOR_AI3_CFG_BUF buf;                     ///< work buf require buffer
} VENDOR_AI3_OP_PREPROC_MEM;

typedef struct _VENDOR_AI3_OP_TOPNSORT_MEM {
    VENDOR_AI3_CFG_BUF buf;                     ///< work buf require 
    HD_DIM input1;
} VENDOR_AI3_OP_TOPNSORT_MEM;

typedef struct _VENDOR_AI3_OP_POSTPROC_MEM {
    UINT32 ctrl;                                ///< reserved
    VENDOR_AI3_CFG_BUF buf;                     ///< work buf require buffer
} VENDOR_AI3_OP_POSTPROC_MEM;

typedef struct _VENDOR_AI3_OP_FC_CFG {
    VENDOR_AI3_OP_FC_MEM op_mem;                ///< assign work buf (must give va, pa, size)
} VENDOR_AI3_OP_FC_CFG;

typedef struct _VENDOR_AI3_OP_MATRIX_CFG {
    VENDOR_AI3_OP_MATRIX_MEM op_mem;            ///< assign work buf (must give va, pa, size)
} VENDOR_AI3_OP_MATRIX_CFG;

typedef struct _VENDOR_AI3_OP_PREPROC_CFG {
    VENDOR_AI3_OP_PREPROC_MEM  op_mem;          ///< assign work buf (must give va, pa, size)
} VENDOR_AI3_OP_PREPROC_CFG;

typedef struct _VENDOR_AI3_OP_POSTPROC_CFG {
    VENDOR_AI3_OP_POSTPROC_MEM  op_mem;         ///< assign work buf (must give va, pa, size)
} VENDOR_AI3_OP_POSTPROC_CFG;

typedef struct _VENDOR_AI3_OP_PREPROC_PARAM {
    HD_DIM scale_dim;                           ///< scale dim, set (0,0) to OFF
    VENDOR_AI3_BUF p_out_sub;                   ///< out subtract plane, packed format, set NULL to OFF
    UINT32 out_sub_color[3];                    ///< out subtract constant value, set [0,0,0] to OFF
    HD_IRECT in_crop_win;                       ///< in crop window, coord is based on in_buf.dim, set (0,0,0,0) to OFF (TODO)
    HD_IRECT out_crop_win;                      ///< out crop window, coord is based on scale dim, set (0,0,0,0) to OFF (TODO)
    HD_IPOINT out_crop_pt;                      ///< out crop window, coord is based on out_buf.dim, set (0,0) to OFF (TODO)
    HD_DIM pad_dim;                             ///< out padding window, coord is based on out_buf.dim, set (0,0) to OFF (TODO)
    UINT32 pad_color[3];                        ///< out padding constant value, set [???] to OFF (TODO)
    UINT32 dir;                                 ///< out direct: mirror-x, mirror-y, rotate, set 0 to OFF, use HD_VIDEO_DIR_XXXX flags (TODO)
} VENDOR_AI3_OP_PREPROC_PARAM;

typedef struct _VENDOR_AI3_OP_POSTPROC_PARAM {
    HD_DIM scale_dim;                           ///< scale dim, set (0,0) to OFF (TODO)
    VENDOR_AI3_BUF p_out_sub;                   ///< out subtract plane, packed format, set NULL to OFF (TODO)
    UINT32 out_sub_color[3];                    ///< out subtract constant value, set [0,0,0] to OFF (TODO)
    HD_IRECT in_crop_win;                       ///< in crop window, coord is based on in_buf.dim, set (0,0,0,0) to OFF (TODO)
    HD_IRECT out_crop_win;                      ///< out crop window, coord is based on scale dim, set (0,0,0,0) to OFF (TODO)
    HD_IPOINT out_crop_pt;                      ///< out crop window, coord is based on out_buf.dim, set (0,0) to OFF (TODO)
    HD_DIM pad_dim;                             ///< out padding window, coord is based on out_buf.dim, set (0,0) to OFF (TODO)
    UINT32 pad_color[3];                        ///< out padding constant value, set [???] to OFF (TODO)
    UINT32 dir;                                 ///< out direct: mirror-x, mirror-y, rotate, set 0 to OFF, use HD_VIDEO_DIR_XXXX flags (TODO)
} VENDOR_AI3_OP_POSTPROC_PARAM;

typedef enum {
    VENDOR_AI3_MATRIX_MUL   = 0,                ///< matrix multiple matrix
    VENDOR_AI3_MATRIX_ADD   = 1,                ///< matrix add matrix
    VENDOR_AI3_MATRIX_SUB   = 2,                ///< matrix sub matrix

    ENUM_DUMMY4WORD(VENDOR_AI3_MATRIX_OP)
} VENDOR_AI3_MATRIX_OP;

typedef enum {
	VENDOR_AI3_SORT_DESC   	= 0,                ///< Descending order
	VENDOR_AI3_SORT_ASC  	= 1,                ///< Ascending order

	ENUM_DUMMY4WORD(VENDOR_AI3_TOPNSORT_OP)
} VENDOR_AI3_TOPNSORT_OP;

typedef struct _VENDOR_AI3_OP_TOPNSORT_CFG {    ///< topn
    VENDOR_AI3_CFG_BUF buf;                     ///< assign work buf (must give va, pa, size)
} VENDOR_AI3_OP_TOPNSORT_CFG;

/********************************************************************
	ELEMENTARY OPERATION
********************************************************************/
typedef struct _VENDOR_AI3_OP_TOPN_SORT_CFG {   ///< topn
    VENDOR_AI3_CFG_BUF buf;                     ///< assign work buf (must give va, pa, size)
	VENDOR_AI3_TOPNSORT_OP op;	                ///< topn sort operation, range [0, 1]
	UINT32 n;						            ///< top n number to sort, range [1, 128]
} VENDOR_AI3_OP_TOPN_SORT_CFG;

typedef struct _VENDOR_AI3_EOP_WORKBUF {
	uintptr_t pa;               ///< physical address
	uintptr_t va;               ///< virtual address
	UINT32 size;                ///< buffer size
} VENDOR_AI3_EOP_WORKBUF;

typedef struct _VENDOR_AI3_EOP_CFG {
	UINT32 ctrl;                ///< reserved
	VENDOR_AI3_CFG_BUF buf;     ///< work buf require 
    UINT8 eop;
	HD_DIM input1 ; 
    HD_DIM input2 ;             ///< only FC using
    UINT32 swap_out;            ///< only FC using, output to temp_buf
} VENDOR_AI3_EOP_CFG;

/********************************************************************
	SINGLE OPERATION DEFINITION
********************************************************************/

///< TODO


/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
extern HD_RESULT vendor_ai3_dev_init (VENDOR_AI3_DEV_CFG* p_dev_cfg);
extern HD_RESULT vendor_ai3_dev_uninit (VOID);

// general config
extern HD_RESULT vendor_ai3_dev_set (VENDOR_AI3_CFG_ID cfg_id, void* p_param);
extern HD_RESULT vendor_ai3_dev_get (VENDOR_AI3_CFG_ID cfg_id, void* p_param);

extern HD_RESULT vendor_ai3_dev_perf_begin (VENDOR_AI3_PERF_ID perf_id);
extern HD_RESULT vendor_ai3_dev_perf_end (VENDOR_AI3_PERF_ID perf_id, void* p_param);

// static network
extern HD_RESULT vendor_ai3_net_open (UINT32* p_proc_id, VENDOR_AI3_PROC_CFG* p_proc_cfg, VENDOR_AI3_NET_INFO* p_net_info);
extern HD_RESULT vendor_ai3_net_close (UINT32 proc_id);
extern HD_RESULT vendor_ai3_net_start (UINT32 proc_id);
extern HD_RESULT vendor_ai3_net_stop (UINT32 proc_id);
extern HD_RESULT vendor_ai3_net_proc (UINT32 proc_id);
extern HD_RESULT vendor_ai3_net_set (UINT32 proc_id, VENDOR_AI3_NET_PARAM_ID param_id, void* p_param);
extern HD_RESULT vendor_ai3_net_get (UINT32 proc_id, VENDOR_AI3_NET_PARAM_ID param_id, void* p_param);

// single op topn
extern HD_RESULT vendor_ai3_op_eval_topnsort (VENDOR_AI3_OP_TOPNSORT_MEM* mem);
extern HD_RESULT vendor_ai3_op_init_topnsort (UINT32* p_op_id, VENDOR_AI3_OP_TOPNSORT_CFG* cfg);
extern HD_RESULT vendor_ai3_op_do_topnsort (UINT32 op_id, VENDOR_AI3_BUF* src, VENDOR_AI3_BUF* dst_value, VENDOR_AI3_BUF* dst_idx, VENDOR_AI3_TOPNSORT_OP op, UINT32 n);
extern HD_RESULT vendor_ai3_op_uninit_topnsort (UINT32 op_id);

// single op preproc
extern HD_RESULT vendor_ai3_op_eval_preprocess (VENDOR_AI3_OP_PREPROC_MEM* mem);
extern HD_RESULT vendor_ai3_op_init_preprocess (UINT32* p_op_id, VENDOR_AI3_OP_PREPROC_CFG* cfg);
extern HD_RESULT vendor_ai3_op_do_preprocess (UINT32 op_id, VENDOR_AI3_BUF* src, VENDOR_AI3_BUF* dst, VENDOR_AI3_OP_PREPROC_PARAM* param);
extern HD_RESULT vendor_ai3_op_uninit_preprocess (UINT32 op_id);

// postproc op
extern HD_RESULT vendor_ai3_op_eval_postprocess (VENDOR_AI3_OP_POSTPROC_MEM* mem); 
extern HD_RESULT vendor_ai3_op_init_postprocess (UINT32* p_op_id, VENDOR_AI3_OP_POSTPROC_CFG* cfg);
extern HD_RESULT vendor_ai3_op_do_postprocess (UINT32 op_id, VENDOR_AI3_BUF* src, VENDOR_AI3_BUF* dst, VENDOR_AI3_OP_POSTPROC_PARAM* param);
extern HD_RESULT vendor_ai3_op_uninit_postprocess (UINT32 op_id);

// single op fc
extern HD_RESULT vendor_ai3_op_eval_fullyconnect (VENDOR_AI3_OP_FC_MEM* mem);
extern HD_RESULT vendor_ai3_op_init_fullyconnect (UINT32* p_op_id, VENDOR_AI3_OP_FC_CFG* cfg);
extern HD_RESULT vendor_ai3_op_do_fullyconnect (UINT32 op_id, VENDOR_AI3_BUF* src1, VENDOR_AI3_BUF* src2, VENDOR_AI3_BUF* dst1);
extern HD_RESULT vendor_ai3_op_uninit_fullyconnect (UINT32 op_id);

// single op matrix
extern HD_RESULT vendor_ai3_op_eval_matrix (VENDOR_AI3_OP_MATRIX_MEM* mem);
extern HD_RESULT vendor_ai3_op_init_matrix (UINT32* p_op_id, VENDOR_AI3_OP_MATRIX_CFG* cfg);
extern HD_RESULT vendor_ai3_op_do_matrix (UINT32 op_id, VENDOR_AI3_BUF* src1, VENDOR_AI3_BUF* src2, VENDOR_AI3_BUF* dst1, VENDOR_AI3_MATRIX_OP mtx_op);
extern HD_RESULT vendor_ai3_op_uninit_matrix (UINT32 op_id);

// elementary op
extern HD_RESULT vendor_ai3_eop_open(UINT32* proc_id, UINT32 op_cmd, VENDOR_AI3_EOP_WORKBUF* wbuf);
extern HD_RESULT vendor_ai3_eop_start(UINT32 proc_id);
extern HD_RESULT vendor_ai3_eop_proc(UINT32 proc_id, UINT32 op_cmd, VENDOR_AI3_BUF* src, VENDOR_AI3_BUF* dst, void* op_cfg, BOOL is_wait_done);
extern HD_RESULT vendor_ai3_eop_close(UINT32 proc_id);
extern HD_RESULT vendor_ai3_eop_stop(UINT32 proc_id);

#ifdef __cplusplus
}
#endif

#endif  /* _VENDOR_AI3_NET_H_ */
