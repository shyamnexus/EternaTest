/**
    @brief Header file of NN engine definition of vendor net flow sample.

    @file nn_net.h

    @ingroup net_flow_sample

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _NN_NET_H_
#define _NN_NET_H_

/********************************************************************
    INCLUDE FILES
********************************************************************/
#ifdef __KERNEL__
#include <linux/kernel.h>
#endif
//#include "nn_verinfo.h"
#include "kwrap/type.h"

/********************************************************************
    MACRO CONSTANT DEFINITIONS
********************************************************************/
#define NN_USE_DSP                      FALSE

#define CONV_ROU_EXCLUSIVE				0  //0: original flow, 1: exclusive flow for CONV_ROU (share SRAM)

#define FLOW_AI_A						1  //always avaliable
#define FLOW_AI_NA						0  //not avaliable

#define FLOW_AI_JOB						1  //must enable FLOW_AI_DEV, FLOW_AI_NET

#define FLOW_AI_DEV						1
#define FLOW_AI_PROCID					1  //must enable FLOW_AI_DEV
#define FLOW_AI_UBUFID					1  //must enable FLOW_AI_DEV
#define FLOW_AI_DEBUG					1  //must enable FLOW_AI_DEV

#define FLOW_AI_NET						1  //must enable FLOW_AI_DEV

#define FLOW_AI_IO						1  //must enable FLOW_AI_DEV, FLOW_AI_NET

#define FLOW_AI_DYSCALE					1  //must enable FLOW_AI_DEV, FLOW_AI_NET

#define FLOW_AI_OP						1  //must enable FLOW_AI_DEV
#if defined(_BSP_NS02401_)
#define FLOW_AI_ISP						0  //must enable FLOW_AI_DEV, FLOW_AI_NET
#else
#define FLOW_AI_ISP						1  //must enable FLOW_AI_DEV, FLOW_AI_NET
#endif

#define NNISP_FRAME_MODE        		1

#define NN_DLI                          1 // 1: support NN_DLI flow and struct

#define CUST_PROC_ALLOC_MEM             1 // 1: cpu custnn using dynamic alloc variable for each proc_id (instead of static variable)

#if (!defined(__KERNEL__))   // only allow USER SPACE


#define NN_NET_MODULE_VERSION           "1.02.002"
#define NN_GEN_ADDR_TYPE_MASK           0xf000000000000000LL
#define NN_GEN_ADDR_MASK                0x00000000ffffffffLL
#define NN_GEN_NULL_ADDR_TYPE           0x0000000000000000LL
#define NN_GEN_NO_ADDR_UPD_TYPE         0x0000000000000000LL  // unsed ?
#define NN_GEN_MODEL_ADDR_TYPE          0x1000000000000000LL
#define NN_GEN_BUF_ADDR_TYPE            0x2000000000000000LL
#define NN_GEN_PAR_ADDR_TYPE            0x3000000000000000LL
#define NN_GEN_TCM_ADDR_TYPE            0x4000000000000000LL
#define NN_GEN_RSV_ADDR_TYPE            0x5000000000000000LL  // reserved

#define NN_LL_ADDR_MASK                 0x00000000ffffffffLL
#define NN_LL_CMD_MASK                  0xffffffff00000000LL

#define NN_MAX_DIM                      5


#define NN_LAYER_LIB_CUSTNN     		0x00000000
#define NN_LAYER_LIB_NVTNN      		0x10000000
#define NN_LAYER_TYPE_ID_MASK   		0x0fffffff

// subfmt
#define NN_CHIP_AI_SUBVER1_MASK         0x01         // don't use : historical
#define NN_CHIP_AI_SUBVER2_MASK         0x02         // don't use : historical
#define NN_CHIP_AI_SUBVER3_MASK         0x04         // don't use : historical
#define NN_CHIP_AI_SUBVER4_MASK         0x08         // if this is NOP aiparm format

// funct_fmt
#define NN_FUNCTION_FMT_BIT1_MASK       0x00000001   // reserved
#define NN_FUNCTION_FMT_BIT2_MASK       0x00000002   // reserved
#define NN_FUNCTION_FMT_BIT3_MASK       0x00000004   // reserved
#define NN_FUNCTION_FMT_BIT4_MASK       0x00000008   // reserved
#define NN_FUNCTION_FMT_BIT5_MASK       0x00000010   // reserved
#define FUNCTION_FMT_LEGAL_LSB          4

// blk header
#define DST_TAG  (UINT32)((UINT32)('O') | ((UINT32)('I')<<8) | ((UINT32)('N')<<16) | ((UINT32)('F')<<24))
#define MULTI_TAG  (UINT32)((UINT32)('I') | ((UINT32)('L')<<8) | ((UINT32)('I')<<16) | ((UINT32)('F')<<24))
#define MCTRL_TAG  (UINT32)((UINT32)('M') | ((UINT32)('C')<<8) | ((UINT32)('I')<<16) | ((UINT32)('F')<<24))
#define GRAPH_TAG  (UINT32)((UINT32)('G') | ((UINT32)('F')<<8) | ((UINT32)('I')<<16) | ((UINT32)('F')<<24))
#define JLIST_GRAPH_TAG  (UINT32)((UINT32)('J') | ((UINT32)('G')<<8) | ((UINT32)('I')<<16) | ((UINT32)('F')<<24))
#define JLIST_CMD_TAG  (UINT32)((UINT32)('J') | ((UINT32)('C')<<8) | ((UINT32)('I')<<16) | ((UINT32)('F')<<24))
#define JMISPPPU_BLK (UINT32)((UINT32)('J') | ((UINT32)('P')<<8) | ((UINT32)('I')<<16) | ((UINT32)('F')<<24))
#define JMISPPLCMD_BLK (UINT32)((UINT32)('P') | ((UINT32)('L')<<8) | ((UINT32)('I')<<16) | ((UINT32)('F')<<24))
#define JMISPSLICEINFO_BLK (UINT32)((UINT32)('J') | ((UINT32)('S')<<8) | ((UINT32)('I')<<16) | ((UINT32)('F')<<24))
#define JMISPHEADER_BLK (UINT32)((UINT32)('J') | ((UINT32)('M')<<8) | ((UINT32)('H')<<16) | ((UINT32)('D')<<24))
#define BUF_TAG  (UINT32)((UINT32)('B') | ((UINT32)('F')<<8) | ((UINT32)('I')<<16) | ((UINT32)('F')<<24))
#define IOMEM_TAG  (UINT32)((UINT32)('I') | ((UINT32)('M')<<8) | ((UINT32)('I')<<16) | ((UINT32)('F')<<24))
#define AIPARM_TAG  (UINT32)((UINT32)('A') | ((UINT32)('P')<<8) | ((UINT32)('I')<<16) | ((UINT32)('F')<<24))
#define DYNAMIC_BATCH_TAG (UINT32)((UINT32)('B') | ((UINT32)('M')<<8) | ((UINT32)('I')<<16) | ((UINT32)('F')<<24))
#define GBLKTILEHEADER_BLK (UINT32)((UINT32)('G') | ((UINT32)('T')<<8) | ((UINT32)('H')<<16) | ((UINT32)('D')<<24))
#define GBLKTILEFRAMEINFO_BLK (UINT32)((UINT32)('G') | ((UINT32)('T')<<8) | ((UINT32)('I')<<16) | ((UINT32)('F')<<24))
#define GBLKTILEPREPOST_BLK (UINT32)((UINT32)('G') | ((UINT32)('T')<<8) | ((UINT32)('P')<<16) | ((UINT32)('P')<<24))
#define BLKTAGSIZE 4  // [ size | tag | version | reserved ]

/********************************************************************
    TYPE DEFINITION
********************************************************************/
/*
    "Hardware" CNN struct : CNN_LIB_PARA, NN_SVM_CLAS_PARA
*/
typedef enum {
	NN_FIRST                = 0x00,  // first id of NN_DLI_xxxx
	NN_CONV 				= 0,
	NN_DECONV,
	NN_SVM,
	NN_ROIPOOLING,
	NN_ELTWISE,
	NN_REORGANIZATION,
	NN_RESHAPE,
	NN_PROPOSAL,
    NN_POSTPROC,
	NN_SOFTMAX,
	NN_FC 					= 10,
	NN_PREPROC,
	NN_FC_POST,
	NN_POOL,
	NN_BNSCALE,
	NN_CUSTOMER,
	NN_ANCHOR,
	NN_UPSAMPLE,
	//NN_CUSTOMER,
	NN_SCALEUP,
	NN_PRELU,
	NN_SIGMOID,
	NN_PRIORBOX,
    NN_DETOUT,
	NN_DEPTHWISE,
    NN_FP2FIX,
	NN_LSTM,
	NN_REVERSE,
	NN_NORM,
	NN_MATMUL,
	NN_BN,
	NN_CORR,
	NN_RESIZE,
	NN_PAD,
	NN_GROUP,
	NN_UTIL,
	NN_LSU,
	NN_ROU,
	NN_ROU_DEPTHWISE,
	NN_GATHER,
	NN_CORR_SINGLE,
	NN_GATHER_ND,
	NN_TOPK,
	NN_GATHER_ELEMENT,
	NN_GRID_SAMPLE,
	NN_ONEHOT,
	NN_CPU_GRIDSAMPLE,
	NN_CUMSUM,
	NN_CPU_LRN,
	NN_LAST                        = 0xFF,  // last id of NN_xxxx (insert your new NN mode before this!
#if NN_DLI
	NN_DLI_FIRST                   = 0x100,  // first id of NN_DLI_xxxx
	NN_DLI_SQRT                    = 0x100,  // use NN_DLI_ElEMENTWISE_PARM
	NN_DLI_DIV                     = 0x101,  // use NN_DLI_ElEMENTWISE_PARM
	NN_DLI_EXP                     = 0x102,  // use NN_DLI_ElEMENTWISE_PARM
	NN_DLI_RESIZE                  = 0x103,  // use NN_DLI_RESIZE_PARM
	NN_DLI_LOG                     = 0x104,  // use NN_DLI_ElEMENTWISE_PARM
	NN_DLI_POW                     = 0x105,  // use NN_DLI_ElEMENTWISE_PARM
	NN_DLI_SIN                     = 0x106,  // use NN_DLI_ElEMENTWISE_PARM
	NN_DLI_EQUAL                   = 0x107,  // use NN_DLI_ElEMENTWISE_PARM
	NN_DLI_GREATER                 = 0x108,  // use NN_DLI_ElEMENTWISE_PARM
	NN_DLI_LESS                    = 0x109,  // use NN_DLI_ElEMENTWISE_PARM
	NN_DLI_FLOOR                   = 0x10A,  // use NN_DLI_ElEMENTWISE_PARM
	NN_DLI_ROUND                   = 0x10B,  // use NN_DLI_ElEMENTWISE_PARM
	NN_DLI_SOFTMAX                 = 0x10C,  // use NN_DLI_SOFTMAX_PARM
	NN_DLI_FC		               = 0x10D,	 // use NN_DLI_FC_PARM
	NN_DLI_INSTANCE_NORMALIZATION  = 0x10E,  // use NN_DLI_INSTANCE_NORMALIZATION_PARM
	NN_DLI_LAYER_NORMALIZATION     = 0x10F,  // use NN_DLI_LAYER_NORMALIZATION_PARM
	NN_DLI_DILATEDCONV			   = 0x110,  // use NN_DLI_DILATEDCONV_PARM
	NN_DLI_DECONVOLUTION           = 0x111,  // use NN_DLI_DECONVOLUTION_PARM
	NN_DLI_DECONVOLUTION_DEPTHWISE = 0x112,  // use NN_DLI_DECONVOLUTION_PARM
	NN_DLI_MATMUL                  = 0x113,  // use NN_DLI_MATMUL_PARM
	NN_DLI_PRIORBOX                = 0x114,  // use NN_DLI_PRIORBOX
	NN_DLI_POOLING				   = 0x115,  // use NN_DLI_POOLING_PARM
	NN_DLI_REVERSE				   = 0x116,  //use NN_DLI_REVERSE_PARM
	NN_DLI_FC_POST				   = 0x117,  //use NN_DLI_FC_POST_PARM
	NN_DLI_PERMUTE                 = 0x118,  // use NN_DLI_PERMUTE_PARM
	NN_DLI_NORM                    = 0x119,  // use NN_NORM_PARM
	NN_DLI_LAST                    = 0x3FF,  // last id of NN_DLI_xxxx (insert your new NN_DLI mode before this!
#endif
	ENUM_DUMMY4WORD(NN_MODE)
} NN_MODE;


/*
    Data format
*/
typedef struct _NN_FMT {
	INT8 frac_bits;
	INT8 int_bits;
	INT8 sign_bits;
	INT8 reserved;
} NN_FMT;

/*
    "Memory related" struct : NN_DATA, NN_BUFFER, NN_IOMEM
*/
typedef struct _NN_DATA {
	uintptr_t pa;
	uintptr_t va;
	UINT32 size;
	NN_FMT fmt;         // sign_bit[16]; int_bit[15..8]; frac_bit[7..0];
} NN_DATA;

typedef struct _NN_DATA_V30 {
	uintptr_t pa;
	uintptr_t va;
	UINT32 size;
	UINT32 reserved;
} NN_DATA_V30;

typedef struct _NN_CUSTOM_DIM {
	UINT32 dim[5];
	UINT32 ofs[4];
} NN_CUSTOM_DIM;

typedef struct _NN_IOMEM {
	uintptr_t imem_addr;
	uintptr_t omem_addr;
	UINT32 imem_cnt;                    ///< size = imem_cnt * sizeof(NN_DATA)
	UINT32 omem_cnt;                    ///< size = omem_cnt * sizeof(NN_DATA)
} NN_IOMEM;

#if 0
typedef enum {
	NN_GEN_ENG_UNKNOWN = -1,
	NN_GEN_ENG_VPE = 0,
	NN_GEN_ENG_CNN,
	NN_GEN_ENG_NUE,
	NN_GEN_ENG_CPU,
	NN_GEN_ENG_DSP,
	NN_GEN_ENG_CNN2,
	NN_GEN_ENG_NUE2,
    NN_GEN_ENG_EXT = 0xFFFF,     // for general backend
	ENUM_DUMMY4WORD(NN_GEN_ENG_TYPE)
} NN_GEN_ENG_TYPE;
#else
typedef enum {
    NN_GEN_ENG_UNKNOWN = -1,
    NN_GEN_ENG_CONV,
    NN_GEN_ENG_UTIL,
    NN_GEN_ENG_CAL,
    NN_GEN_ENG_LSU,
    NN_GEN_ENG_ROU,
    NN_GEN_ENG_NUE2,
    NN_GEN_ENG_PPU,
    NN_GEN_ENG_CPU,
    NN_GEN_ENG_DSP,
    NN_GEN_ENG_POU,
    NN_GEN_ENG_EXT = 0xFFFF,     // for general backend
    ENUM_DUMMY4WORD(NN_GEN_ENG_TYPE),
} NN_GEN_ENG_TYPE;
#endif

typedef enum {
	NN_GEN_TRIG_APP_AI_DRV    = 0,        ///< use KDRV_AI_MODE      (for DLA layer)
	NN_GEN_TRIG_LL_AI_DRV     = 1,        ///< use KDRV_AI_LL_MODE   (for DLA layer)
	NN_GEN_TRIG_COMMON        = 2,        ///<                       (for non-DLA layer)
	NN_GEN_TRIG_USER_AI_DRV   = 3,        ///< use KDRV_AI_USER_MODE (for DLA single OP)
    NN_GEN_TRIG_JOB_LL_AI_DRV = 4,        ///< trigger with job manager
	ENUM_DUMMY4WORD(NN_GEN_TRIG_SRC)
} NN_GEN_TRIG_SRC;

typedef struct __NN_INPUT_INFO_HEADER {
	UINT64 blk_size;
	UINT32 batch_info; // batch_mctrl_id(24bit) + buf_id(8bit)
	UINT32 input_stripe_num;
}NN_INPUT_INFO_HEADER;

typedef struct _NN_BUF_INFO_IN {
	uintptr_t buf_offset;
	INT32  buf_index;
} NN_BUF_INFO_IN;

typedef struct _NN_BUF_INFO_OUT {
	uintptr_t buf_offset;               ///< bit 0x8000000 indicate if this is TMP buffer
	INT32  buf_index;
	INT32  buf_size;
} NN_BUF_INFO_OUT;

typedef enum _NN_IN_BUF_ATTR {
	NN_IN_BUF_ATTR_ELTWISE_IN_SRC = 0x00000020,
	NN_IN_BUF_ATTR_CONV_IN_ISIMG  = 0x00000010,
	NN_IN_BUF_ATTR_PREPROC_IN_FMT = 0X0000000f,
	ENUM_DUMMY4WORD(NN_IN_BUF_ATTR)
} NN_IN_BUF_ATTR;

typedef enum _NN_OUT_BUF_ATTR {
	NN_OUT_BUF_ATTR_PRESERVE         = 0x80000000,
	NN_OUT_BUF_ATTR_PREPROC_OUT_FMT  = 0x0000000f,
	ENUM_DUMMY4WORD(NN_OUT_BUF_ATTR)
} NN_OUT_BUF_ATTR;

#define IN_BUF_INDEX(p_mctrl, buf_idx)    (((NN_BUF_INFO_IN*)((p_mctrl)->in_bufinfo_addr))[buf_idx].buf_index)
#define IN_BUF_OFFSET(p_mctrl, buf_idx)   (((NN_BUF_INFO_IN*)((p_mctrl)->in_bufinfo_addr))[buf_idx].buf_offset)
#define IN_BUF_NUM(p_mctrl)               (((p_mctrl)->mode==NN_DEPTHWISE || (p_mctrl)->mode==NN_CORR || (p_mctrl)->mode==NN_ROU_DEPTHWISE)? (((p_mctrl)->in_bufinfo_cnt&0xFF000000)>>24):((p_mctrl)->in_bufinfo_cnt))
#define IN_BUF_NUM_REAL(p_mctrl)          (((p_mctrl)->mode==NN_DEPTHWISE || (p_mctrl)->mode==NN_CORR || (p_mctrl)->mode==NN_ROU_DEPTHWISE)? (((p_mctrl)->in_bufinfo_cnt&0x00FFFFFF)):((p_mctrl)->in_bufinfo_cnt))
#define OUT_BUF_INDEX(p_mctrl, buf_idx)   (((NN_BUF_INFO_OUT*)((p_mctrl)->out_bufinfo_addr))[buf_idx].buf_index)
#define OUT_BUF_OFFSET(p_mctrl, buf_idx)  (((NN_BUF_INFO_OUT*)((p_mctrl)->out_bufinfo_addr))[buf_idx].buf_offset & 0x7fffffff)
#define OUT_BUF_NUM(p_mctrl)              (((p_mctrl)->mode==NN_DEPTHWISE || (p_mctrl)->mode==NN_CORR || (p_mctrl)->mode==NN_ROU_DEPTHWISE)? (((p_mctrl)->out_bufinfo_cnt&0xFF000000)>>24):((p_mctrl)->out_bufinfo_cnt))
#define OUT_BUF_NUM_REAL(p_mctrl)         (((p_mctrl)->mode==NN_DEPTHWISE || (p_mctrl)->mode==NN_CORR || (p_mctrl)->mode==NN_ROU_DEPTHWISE)? (((p_mctrl)->out_bufinfo_cnt&0x00FFFFFF)):((p_mctrl)->out_bufinfo_cnt))
#define OUT_BUF_SIZE(p_mctrl, buf_idx)    (((NN_BUF_INFO_OUT*)((p_mctrl)->out_bufinfo_addr))[buf_idx].buf_size)
#define OUT_BUF_IS_TMP(p_mctrl, buf_idx) ((((NN_BUF_INFO_OUT*)((p_mctrl)->out_bufinfo_addr))[buf_idx].buf_offset & 0x80000000) >> 31)

#define IN_BUF_ATTR_GET(p_mctrl, attr_parm) (((p_mctrl)->in_buf_attr & attr_parm) / (attr_parm & ~(attr_parm-1)))
#define IN_BUF_ATTR_SET(p_mctrl, attr_parm, value)  do{(p_mctrl)->in_buf_attr = ((p_mctrl)->in_buf_attr & ~(attr_parm)) | ((value * (attr_parm & ~(attr_parm-1))) & attr_parm);}while(0)
#define OUT_BUF_ATTR_GET(p_mctrl, attr_parm) (((p_mctrl)->out_buf_attr & attr_parm) / (attr_parm & ~(attr_parm-1)))
#define OUT_BUF_ATTR_SET(p_mctrl, attr_parm, value)  do{(p_mctrl)->out_buf_attr = ((p_mctrl)->out_buf_attr & ~(attr_parm)) | ((value * (attr_parm & ~(attr_parm-1))) & attr_parm);}while(0)

#define NN_LL_CMD_GET_MODE(cmd)        (0x000000000000000f & cmd)
#define NN_JOBLL_CMD_GET_MODE(cmd)     (0x000000000000000f & cmd)

#define GET_MSB(v)          (((v) & 0xFFFFFFFF00000000) >> 32)
#define GET_LSB(v)          (((v) & 0x00000000FFFFFFFF))
#if defined(_BSP_NS02401_)
#define GET_TRUE_MSB(v)     (((v) & 0x000000FF00000000) >> 32)
#else
#define GET_TRUE_MSB(v)     (((v) & 0x0000000F00000000) >> 32)
#endif
#define MAKE_U64(msb,lsb)    (((UINT64)(msb) << 32) | (lsb))

#ifndef MAKEEIGHTCC
#define MAKEEIGHTCC(ch0, ch1, ch2, ch3, ch4, ch5, ch6, ch7)  ((UINT64)(UINT8)(ch0) | ((UINT64)(UINT8)(ch1) << 8) | ((UINT64)(UINT8)(ch2) << 16) | ((UINT64)(UINT8)(ch3) << 24) | ((UINT64)(UINT8)(ch4) << 32) | ((UINT64)(UINT8)(ch5) << 40) | ((UINT64)(UINT8)(ch6) << 48) | ((UINT64)(UINT8)(ch7) << 56))
#endif

typedef enum _MODEL_TYPE_ATTR {
	MODEL_TYPE_ATTR_OPMODE     = 0x0000001F, // b'00 ~ b'04 => op_mode
	MODEL_TYPE_ATTR_BITMODE    = 0x000000E0, // b'05 ~ b'07 => bit_mdoe
	MODEL_TYPE_ATTR_DYSCALE    = 0X00000100, // b'08 ~ b'08 => dyScale
	MODEL_TYPE_ATTR_DYBATCH    = 0X00000200, // b'09 ~ b'09 => dyBatch
	MODEL_TYPE_ATTR_IS_LSTM    = 0X00000400, // b'10 ~ b'10 => isLSTM
	MODEL_TYPE_ATTR_IS_AIISP   = 0X00000800, // b'11 ~ b'11 => aiisp
	MODEL_TYPE_ATTR_LL_SHRINK  = 0X00001000, // b'12 ~ b'12 => llcmdShrink
	MODEL_TYPE_ATTR_DIS_GRAPH  = 0X00002000, // b'13 ~ b'13 => disable graphInfo
	MODEL_TYPE_ATTR_USE_TILING = 0X0001C000, // b'14 ~ b'16 => useTiling
	ENUM_DUMMY4WORD(MODEL_TYPE_ATTR)
} MODEL_TYPE_ATTR;

#define MODEL_TYPE_ATTR_GET(model_type, attr_parm) ((model_type & attr_parm) / (attr_parm & ~(attr_parm-1)))

typedef struct _NN_GEN_CHIP_CONFIG {
	UINT64 model_bin_prefix;
    UINT32 id;
    CHAR gentool_vers[15];
	UINT8 reserved_1[5];
	UINT64 funct_fmt_bit;
	UINT8 reserved_2[24];
} NN_GEN_CHIP_CONFIG;

typedef struct _NN_GEN_MODE_CTRL {
	NN_GEN_TRIG_SRC trig_src;
	NN_GEN_ENG_TYPE eng;
	NN_MODE mode;
	uint32_t layer_index;
	uint32_t nn_layer_index;
	uint32_t tot_trig_eng_times;
	NN_IOMEM iomem;
	uintptr_t addr;
	uintptr_t in_bufinfo_addr;
	uintptr_t out_bufinfo_addr;
	uintptr_t prev_layer_idx_addr;
	uintptr_t next_layer_idx_addr;
	uintptr_t job_list_addr;
    uintptr_t prev_job_list_idx_addr;
    uintptr_t next_job_list_idx_addr;
	uint32_t size;
	uint32_t in_bufinfo_cnt;
	uint32_t out_bufinfo_cnt;
	uint32_t prev_num;
	uint32_t next_num;
    uint32_t job_list_prev_num;
    uint32_t job_list_next_num;
    uint32_t job_list_size;
	uint32_t idea_cycle;
	uint32_t in_buf_attr;
	uint32_t out_buf_attr;
}__attribute__((packed)) NN_GEN_MODE_CTRL;

typedef struct _NN_GEN_MODEL_HEAD {
	NN_GEN_CHIP_CONFIG chip;            ///< configuration of chip
	UINT32 mode_ctrl_num;           // number of mode control
	UINT32 layer_num;               // total layers
	UINT32 user_parm_size;          // total user_parm size including weight alignment
	UINT32 user_parm_non_align;     // total user_parm size
	UINT32 model_size;              // model size
	UINT32 parm_size;               // parameters size
	UINT32 io_buff_size;            // IO buffer size
	UINT32 iomem_size;              // io memory information size (sai/sao)
	UINT32 layer_id_list_size;      // pre/last layer id list size
	UINT32 external_size;           // output buffer dim/lofs info
	UINT32 bufinfo_size;            // bufinfo total size (buf id/offset/size)
	UINT32 joblist_size;            // joblist total size
	UINT32 joblist_graph_size;      // joblist graph size
	UINT32 quanlist_size;           // quantization info size
	UINT32 jmisp_ppu_size;          // jmisp ppu info size
	UINT32 jmisp_pl_size;           // path list info size
	UINT32 jmisp_info_size;         // jmisp info size
	UINT32 model_type;              // record op_mode, bitmode, isDyScale, isDyBatch, isLSTM, isAIISP ... // using MODEL_TYPE_ATTR_GET() to get each attr value
	UINT32 gblktile_info_size;      // global block tiling info size
  	UINT32 gblktile_pre_post_size;	// global block tile preproc/postproc info size
  	UINT32 reserved_blk3;           // reserved           
  	UINT32 reserved_blk4;           // reserved
  	UINT32 reserved_blk5;           // reserved
  	UINT32 reserved_blk6;           // reserved
  	UINT32 reserved_blk7;           // reserved
  	UINT32 reserved_blk8;           // reserved
} NN_GEN_MODEL_HEAD;

typedef struct _NN_GEN_NET_INFO {
    NN_GEN_MODEL_HEAD *p_head;
    NN_GEN_MODE_CTRL *p_mctrl;
	uintptr_t p_id_list; // graph info blk
	uintptr_t p_iomem_list; // iomeme blk
	uintptr_t p_ai_parm_list; // aiparm blk
	uintptr_t p_buf_list; // buf blk
	uintptr_t p_job_list; // job list blk
	uintptr_t p_jod_id_list; // job_graph_blk
	uintptr_t p_quan_info_list;// quan_blk
	uintptr_t p_ppu_cmd_list; // ppu_cmd blk
	uintptr_t p_pl_cmd_list; // pl_cmd blk
	uintptr_t p_jmisp_header_blk; // jmisp info head blk
	uintptr_t p_jmisp_info_blk; // jmisp info blk
	uintptr_t p_gblktile_header_blk; // global block tile header blk
	uintptr_t p_gblktile_info_blk; // global block tile info blk
	uintptr_t p_prepost_cmd_blk; // preproc postproc cmd blk
} NN_GEN_NET_INFO;

typedef union {
    struct {
        uint8_t sub_stripe_en       : 1;
        uint8_t inppu_pipe_en       : 1;
        uint8_t outppu_pipe_en      : 1;
        uint8_t buf_type            : 1;   // 0: pipe buf in dram; 1: pipe buf in ub;
        uint8_t reuse_pipe_buf      : 1;
    } bit;
    uint8_t byte;
} AI_PPU_PIPE_MODE;

typedef struct {
    uint32_t each_ubuf_budget;        // ubuf budget of each core (Byte)
    uint32_t each_dram_iobuf_size;    // dram working buff size of each core (Byte)
    uint8_t total_ubuf_size;          // total ubuf size (MB)
    uint8_t aiisp_mode;               // 0: sw mode 1: hw mode
    uint8_t core_num;
    uint8_t weight_loc;               // 0: weight in dram  1: weight in ubuf
    AI_PPU_PIPE_MODE pipe_mode;
    uint8_t pre_in0_en;
    uint8_t reserved_blk0[2];
    uint32_t each_ai_parm_size;
    uint8_t reserved_blk1[28];
} AI_JMISP_INFO_HEADER;

typedef enum {
    e_GBLKTILE_IOBUF_UNKNOWN   = -1,
    e_GBLKTILE_IOBUF_FRAME0,
    e_GBLKTILE_IOBUF_FRAME1,
    e_GBLKTILE_IOBUF_FRAME2,             // reserved_blk0
    e_GBLKTILE_IOBUF_FRAME3,             // reserved_blk1
    e_GBLKTILE_IOBUF_FRAME4,             // reserved_blk2
    e_GBLKTILE_IOBUF_FRAME5,             // reserved_blk3
    e_GBLKTILE_IOBUF_NUM,
} GBLKTILE_IOBUF_CONST;

typedef struct {
    INT32 frame_width;
    INT32 frame_height;
} GBLKTILE_FrameInfo;

typedef struct {
    INT32 block_width;          // indicate input of PPU
    INT32 block_overlap_width;  // indicate input of PPU
    INT32 block_height;         // indicate input of PPU
    INT32 block_overlap_height; // indicate input of PPU
} GBLKTILE_BlockInfo;

typedef struct {
    uint32_t each_ubuf_budget;        // ubuf budget of each core (Byte)
    uint32_t each_dram_iobuf_size;    // dram working buff size of each core (Byte)
    uint8_t total_ubuf_size;          // total ubuf size (MB)
    uint8_t core_num;
    uint8_t weight_loc;               // 0: weight in dram  1: weight in ubuf
    uint8_t pre_in0_en;
    uint8_t reserved_blk[36];
} GBLKTILE_INFO_HEADER;

typedef struct {
    GBLKTILE_FrameInfo   frameInfo;
    GBLKTILE_BlockInfo   blockInfo;
    UINT64 pa;
    UINT64 va;
    INT32 match_mctrl_id; // initialize as -1
    UINT32 bitdepth;
    UINT32 buffer_sz;
    uint8_t is_out;
    uint8_t reserved_blk[63];
}__attribute__((packed)) GBLKTILE_IOBUF_INFO;

typedef enum {
	NN_RGB888,
	NN_BGR888,
	NN_YUV420,
	NN_Y_ONLY,
	NN_UV_PAC,
	NN_FEATURE,
} NN_IN_OUT_BUF_FMT;

typedef enum {
	NN_BITDEPTH_8,
	NN_BITDEPTH_16,
	NN_BITDEPTH_32,
} NN_BITDEPTH_FMT;

typedef struct _NN_IN_OUT_BUF_INFO {
	char    layer_name[192];
	char    layer_type[96];
	UINT16  caffe_layer_index;
	UINT16  fusion_layer_index;
	UINT32  input_fmt; // NN_IN_OUT_BUF_FMT
	UINT32  width;
	UINT32  height;
	UINT32  channel;
	UINT32  batch;
	UINT32  time;
	UINT8   out_bitdepth;
	UINT8   out_sign_bit_num;
	INT8    out_int_bit_num;
	INT8    out_frac_bit_num;
	INT32   out_offset_num; // 16-bit
	float   out_scale_ratio;
	UINT64 	out_lofs;
	UINT64 	out_ch_ofs;
	UINT64 	out_batch_ofs;
	UINT64 	out_time_ofs;
	UINT32  ext_id;
	char    data_order[8];
	UINT32 addr_offset;
	UINT64 quan_addr;
}NN_IN_OUT_BUF_INFO;

typedef struct _NN_IN_OUT_BUF_INFO_1 {
	char    layer_name[192];
	char    layer_type[96];
	UINT16  caffe_layer_index;
	UINT16  fusion_layer_index;
	UINT32  input_fmt; // NN_IN_OUT_BUF_FMT
	UINT32  width;
	UINT32  height;
	UINT32  channel;
	UINT32  batch;
	UINT32  time;
	UINT8   out_bitdepth;
	UINT8   out_sign_bit_num;
	INT8    out_int_bit_num;
	INT8    out_frac_bit_num;
	INT32   out_offset_num; // 16-bit
	float   out_scale_ratio;
	UINT64 	out_lofs;
	UINT64 	out_ch_ofs;
	UINT64 	out_batch_ofs;
	UINT64 	out_time_ofs;
	UINT32  ext_id;
	char    data_order[8];
	UINT32 addr_offset;
	UINT64 quan_addr;
	UINT32  mctrl_id;
	UINT32  buf_id;
}NN_IN_OUT_BUF_INFO_1;

typedef struct _NN_IN_OUT_BUF_INFO2 {
	UINT32 caffe_layer_index;
	UINT32 fusion_layer_index;
	UINT32 width;
	UINT32 height;
	UINT32 channel;
	UINT32 batch;
	UINT32 time;
	UINT32 out_lofs;
	UINT32 out_ch_ofs;
	UINT8  padding[4]; // only for 64-bits
}NN_IN_OUT_BUF_INFO2;
typedef struct _NN_LAYER_OUTPUT_INFO
{
	char layer_name[100];
	UINT16 caffe_layer_index;
	UINT16 fusion_layer_index;
	UINT16 out_width;
	UINT16 out_height;
	UINT16 out_channel;
	UINT16 out_batch;
	UINT16 out_time;
	UINT16 reserve;
	UINT8 out_bitdepth;
	UINT8 out_sign_bit_num;
	UINT8 out_int_bit_num;
	UINT8 out_frac_bit_num;
	float out_scale_ratio;
	UINT16 out_lofs;
	UINT16 out_ch_ofs;
	UINT32 out_batch_ofs;
	UINT32 out_time_ofs;
	char   layer_type[96];
	char   data_order[8];
	UINT32 reserved[8];
} NN_LAYER_OUTPUT_INFO;
typedef struct _NN_IN_OUT_FMT {
	char   model_fmt[20];
	UINT16 model_width;
	UINT16 model_height;
	UINT16 model_channel;
	UINT16 model_batch;
	char   in_fmt[20];
	UINT16 in_channel;
	UINT16 reserve;
} NN_IN_OUT_FMT;

typedef struct _VENDOR_AI_NET_INPUT_INFO {
	CHAR   name[192];                       ///< buffer name, eg: "mylayer.out0"
	UINT32 width;                           ///< width
	UINT32 height;                          ///< height
	UINT32 channel;                         ///< channel
	UINT32 batch;                           ///< number of batch
} VENDOR_AI_NET_INPUT_INFO;

typedef struct _NN_IN_STRIPE_INFO {
	UINT32 offset;
	UINT32 mctrl;
} NN_IN_STRIPE_INFO;

/*ll*/
#if defined(_BSP_NS02401_)
typedef struct _LINK_LIST_NULL_CMD {
    unsigned mode: 4;
	unsigned lock_unit_enable : 1;
	unsigned : 3;
	unsigned table_index : 8;
    unsigned : 16;
	unsigned : 32;
} __attribute__((packed)) LINK_LIST_NULL_CMD ;
#else
typedef struct _LINK_LIST_NULL_CMD {
    unsigned mode: 4;
	unsigned : 4;
	unsigned table_index : 8;
    unsigned : 16;
	unsigned : 32;
} __attribute__((packed)) LINK_LIST_NULL_CMD ;
#endif

typedef struct _LINK_LIST_UPD_CMD {
    unsigned mode: 4;
    unsigned byte_en: 4;
    unsigned reg_ofs: 12;
    unsigned : 12;
    unsigned value : 32;
} __attribute__((packed)) LINK_LIST_UPD_CMD ;

#if defined(_BSP_NS02401_)
typedef struct _LINK_LIST_JUMP_CMD {
    unsigned mode: 4;
    unsigned : 12;
    unsigned address : 32;
    unsigned msb_addr : 8;
    unsigned : 4;
	unsigned addr_type : 4;
} __attribute__((packed)) LINK_LIST_JUMP_CMD;

typedef struct _LINK_LIST_NEXTLL_CMD {
    unsigned mode: 4;
	unsigned lock_unit_enable: 1;
	unsigned : 3;
	unsigned table_index : 8;
    unsigned : 16;
	unsigned : 32;
} __attribute__((packed)) LINK_LIST_NEXTLL_CMD ;
#else
typedef struct _LINK_LIST_JUMP_CMD {
    unsigned mode: 4;
    unsigned : 12;
    unsigned address : 32;
    unsigned msb_addr : 4;
    unsigned : 8;
	unsigned addr_type : 4;
} __attribute__((packed)) LINK_LIST_JUMP_CMD;

typedef struct _LINK_LIST_NEXTLL_CMD {
    unsigned mode: 4;
	unsigned : 4;
	unsigned table_index : 8;
    unsigned : 16;
	unsigned : 32;
} __attribute__((packed)) LINK_LIST_NEXTLL_CMD ;
#endif

/*JOBM*/
#if defined(_BSP_NS02401_)
typedef struct _JOB_LIST_TRIG_CMD {
    unsigned mode: 4;
    unsigned unit_sel: 4;
    unsigned flag_id: 8;
    unsigned conv_mask: 8;
	unsigned ll_sel: 1;
    unsigned : 3;
	unsigned lock_arb_enable: 1;
	unsigned : 3;
    unsigned address : 32;
    unsigned msb_addr : 8;
	unsigned job_idx : 12;
    unsigned : 8;
	unsigned addr_type : 4;
} JOB_LIST_TRIG_CMD ;

typedef struct _JOB_LIST_JUMP_CMD {
    unsigned mode: 4;
    unsigned : 12;
    unsigned address : 32;
    unsigned msb_addr : 8;
    unsigned : 4;
	unsigned addr_type : 4;
} __attribute__((packed)) JOB_LIST_JUMP_CMD;
#else
typedef struct _JOB_LIST_TRIG_CMD {
    unsigned mode: 4;
    unsigned unit_sel: 4;
    unsigned flag_id: 8;
    unsigned conv_mask: 8;
    unsigned : 8;
    unsigned address : 32;
    unsigned msb_addr : 4;
    unsigned : 4;
    unsigned job_idx : 12;
    unsigned : 8;
	unsigned addr_type : 4;
} JOB_LIST_TRIG_CMD ;

typedef struct _JOB_LIST_JUMP_CMD {
    unsigned mode: 4;
    unsigned : 12;
    unsigned address : 32;
    unsigned msb_addr : 4;
    unsigned : 8;
	unsigned addr_type : 4;
} __attribute__((packed)) JOB_LIST_JUMP_CMD;
#endif

typedef struct _JOB_LIST_WAIT_CMD {
    unsigned mode: 4;
    unsigned : 4;
    unsigned flag_id: 8;
    unsigned : 16;
} JOB_LIST_WAIT_CMD ;

typedef struct _JOB_LIST_NULL_CMD {
    unsigned mode: 4;
    unsigned : 28;
} JOB_LIST_NULL_CMD ;

typedef struct _JOB_LIST_UPD_CMD {
    unsigned mode: 4;
    unsigned byte_en: 4;
    unsigned offset: 12;
    unsigned : 12;
    unsigned value : 32;
} JOB_LIST_UPD_CMD ;

typedef struct _JOB_LIST_MOV_CMD {
    unsigned mode: 4;
    unsigned : 4;
    unsigned src_reg_ofs: 12;
    unsigned dst_reg_ofs: 12;
} JOB_LIST_MOV_CMD ;

/*JMISP*/
#if defined(_BSP_NS02401_)
typedef struct _PATH_LIST_TRIG_CMD {
    unsigned mode: 4;
    unsigned func_sel: 4;
    unsigned flag_id: 8;
    unsigned : 12;
	unsigned : 1;
    unsigned : 3;
    unsigned address : 32;
    unsigned msb_addr : 8;
    unsigned func_idx : 12;
    unsigned : 8;
	unsigned addr_type : 4;
} __attribute__((packed)) PATH_LIST_TRIG_CMD ;
#else
typedef struct _PATH_LIST_TRIG_CMD {
    unsigned mode: 4;
    unsigned func_sel: 4;
    unsigned flag_id: 8;
    unsigned : 8;
    unsigned : 8;
    unsigned address : 32;
    unsigned msb_addr : 4;
    unsigned : 4;
    unsigned func_idx : 12;
    unsigned : 8;
	unsigned addr_type : 4;
} __attribute__((packed)) PATH_LIST_TRIG_CMD ;
#endif

typedef struct _PATH_LIST_WAIT_CMD {
    unsigned mode: 4;
    unsigned : 4;
    unsigned flag_id: 8;
    unsigned : 16;
} __attribute__((packed)) PATH_LIST_WAIT_CMD ;

typedef struct _PATH_LIST_NULL_CMD {
    unsigned mode: 4;
    unsigned : 28;
} __attribute__((packed)) PATH_LIST_NULL_CMD ;

typedef struct _PATH_LIST_UPD_CMD {
    unsigned mode: 4;
    unsigned byte_en: 4;
    unsigned reg_ofs: 12;
    unsigned : 12;
    unsigned value : 32;
} __attribute__((packed)) PATH_LIST_UPD_CMD ;

#if defined(_BSP_NS02401_)
typedef struct _PATH_LIST_JUMP_CMD {
    unsigned mode: 4;
    unsigned : 12;
    unsigned address : 32;
    unsigned msb_addr : 8;
    unsigned : 4;
	unsigned addr_type : 4;
} __attribute__((packed)) PATH_LIST_JUMP_CMD;

typedef struct _PATH_LIST_CONDITION_JUMP_CMD {
    unsigned mode: 4;
    unsigned reg_ofs: 12;
    unsigned address : 32;
    unsigned msb_addr : 8;
    unsigned byte_en: 4;
	unsigned addr_type : 4;
    unsigned condi_val: 32;
} __attribute__((packed)) PATH_LIST_CONDITION_JUMP_CMD;
#else
typedef struct _PATH_LIST_JUMP_CMD {
    unsigned mode: 4;
    unsigned : 12;
    unsigned address : 32;
    unsigned msb_addr : 4;
    unsigned : 8;
	unsigned addr_type : 4;
} __attribute__((packed)) PATH_LIST_JUMP_CMD;

typedef struct _PATH_LIST_CONDITION_JUMP_CMD {
    unsigned mode: 4;
    unsigned reg_ofs: 12;
    unsigned address : 32;
    unsigned msb_addr : 4;
    unsigned : 4;
    unsigned byte_en: 4;
	unsigned addr_type : 4;
    unsigned condi_val: 32;
} __attribute__((packed)) PATH_LIST_CONDITION_JUMP_CMD;
#endif

typedef struct _PATH_LIST_MOV_CMD {
    unsigned mode: 4;
    unsigned : 4;
    unsigned src_reg_ofs: 12;
	unsigned dst_reg_ofs: 12;
} __attribute__((packed)) PATH_LIST_MOV_CMD;

typedef struct _PATH_LIST_LOGIC_WAIT_CMD {
    unsigned mode: 4;
    unsigned : 4;
    unsigned op: 4;
    unsigned : 20;
    unsigned long long flag_status: 64;
} __attribute__((packed)) PATH_LIST_LOGIC_WAIT_CMD;

typedef struct _PATH_LIST_ALU_CMD {
    unsigned mode: 4;
    unsigned : 4;
    unsigned op: 4;
    unsigned : 4;
    unsigned reg_ofs: 12;
    unsigned : 4;
	unsigned value: 32;
} __attribute__((packed)) PATH_LIST_ALU_CMD;

//-------- PROC --------
#define AI_SUPPORT_PROC_MAX         (0xffff)
#if defined (__UITRON) || defined(__ECOS)  || defined (__FREERTOS)
#define AI_SUPPORT_PROC_DEF         (16)   //  16 PROC
#else
#define AI_SUPPORT_PROC_DEF         (128)  // 128 PROC
#endif
//-------- UBUF --------
#define NN_UBUF_STATUS_FREE         (-100) // used for  =>  INT32 g_ai_net_ubuf_status[AI_SUPPORT_UBUF_MAX]  at kflow_ai_net_flow.c

#if defined(_BSP_NS02201_)
#define NN_GEN_TCM_PHYSICAL_ADDR    0x00000002f1000000LL      // TCM addr => 0x2_F100_0000  ~  0x2_F13F_FFFF
#define AI_SUPPORT_UBUF_MAX         (4)        // 4 block
#define AI_SUPPORT_UBUF_BLOCK_SIZE  (0x100000) // 1 MB   => total 4*1MB = 4MB
#define AI_SUPPORT_DSP              (1) //690 support dsp flow
#elif defined(_BSP_NS02302_)
#define NN_GEN_TCM_PHYSICAL_ADDR    0x00000002f1000000LL      // TCM addr => (0x2_F100_0000~0x2_F10B_FFFF) + (0x2_F100_0000~0x2_F10B_FFFF). It can be used as continuous 1.5MB starting from 0x2_F100_0000, if with TCM interleave mode enabled.
#define AI_SUPPORT_UBUF_MAX         (3)        // 3 block
#define AI_SUPPORT_UBUF_BLOCK_SIZE  (0x80000)  // 512 KB => total 3*512KB = 1.5MB
#define AI_SUPPORT_DSP              (0) //538 not support dsp flow
#elif defined(_BSP_NS02401_)
#define NN_GEN_TCM_PHYSICAL_ADDR    0x00000004f1000000LL      // TCM addr => (0x4_F100_0000~0x4_F10B_FFFF) + (0x4_F110_0000~0x4_F11B_FFFF). It can be used as continuous 1.5MB starting from 0x4_F100_0000, if with TCM interleave mode enabled.
#define AI_SUPPORT_UBUF_MAX         (3)        // 3 block
#define AI_SUPPORT_UBUF_BLOCK_SIZE  (0x80000)  // 512 KB => total 3*512KB = 1.5MB
#define AI_SUPPORT_DSP              (0) //635 not support dsp flow
#endif

#define NN_GEN_TCM_SIZE             (AI_SUPPORT_UBUF_MAX*AI_SUPPORT_UBUF_BLOCK_SIZE)

#else // only for KERNEL SPACE

////////////////////////////////////////////////////////////////////////////////

typedef struct _NN_DATA_V30 {
	uintptr_t pa;
	uintptr_t va;
	UINT32 size;
	UINT32 reserved;
} NN_DATA_V30;

typedef struct _NN_IOMEM {
	uintptr_t imem_addr;
	uintptr_t omem_addr;
	UINT32 imem_cnt;                    ///< size = imem_cnt * sizeof(NN_DATA)
	UINT32 omem_cnt;                    ///< size = omem_cnt * sizeof(NN_DATA)
} NN_IOMEM;

////////////////////////////////////////////////////////////////////////////////

extern UINT32 kflow_ai_get_net_supported_num(VOID);

//-------- PROC --------
#define AI_SUPPORT_PROC_MAX         (0xffff)
#if defined (__UITRON) || defined(__ECOS)  || defined (__FREERTOS)
#define AI_SUPPORT_PROC_DEF         (16)   //  16 PROC
#else
#define AI_SUPPORT_PROC_DEF         (128)  // 128 PROC
#endif
//-------- UBUF --------
#define NN_UBUF_STATUS_FREE         (-100) // used for  =>  INT32 g_ai_net_ubuf_status[AI_SUPPORT_UBUF_MAX]  at kflow_ai_net_flow.c

#if defined(_BSP_NS02201_)
#define AI_SUPPORT_UBUF_MAX         (4)        // 4 block
#define AI_SUPPORT_UBUF_BLOCK_SIZE  (0x100000) // 1 MB   => total 4*1MB = 4MB
#define AI_SUPPORT_DSP              (1) //690 support dsp flow
#elif defined(_BSP_NS02302_)
#define AI_SUPPORT_UBUF_MAX         (3)        // 3 block
#define AI_SUPPORT_UBUF_BLOCK_SIZE  (0x80000)  // 512 KB => total 3*512KB = 1.5MB
#define AI_SUPPORT_DSP              (0) //538 not support dsp flow
#elif defined(_BSP_NS02401_)
#define AI_SUPPORT_UBUF_MAX         (3)        // 3 block
#define AI_SUPPORT_UBUF_BLOCK_SIZE  (0x80000)  // 512 KB => total 3*512KB = 1.5MB
#define AI_SUPPORT_DSP              (0) //635 not support dsp flow
#endif

////////////////////////////////////////////////////////////////////////////////


#endif


#endif  /* _NN_NET_H_ */


