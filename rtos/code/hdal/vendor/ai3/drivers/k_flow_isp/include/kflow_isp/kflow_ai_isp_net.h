/**
	@brief Header file of definition of vendor net flow sample.

	@file net_flow_sample.h

	@ingroup net_flow_sample

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _KFLOW_AI_ISP_NET_H_
#define _KFLOW_AI_ISP_NET_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#ifdef __KERNEL__
#include <linux/ioctl.h>
#endif
#include "kwrap/ioctl.h"
#include "kwrap/semaphore.h"
#include "kflow_ai_net/nn_net.h"

/********************************************************************
	TYPE DEFINITION
********************************************************************/

#define NN_IMEM_NUM 20 


typedef struct _VENDOR_AIS_ISP_MEM_PARM {
	uintptr_t pa;
	uintptr_t va;
	UINT32 size;
} VENDOR_AIS_ISP_MEM_PARM;

typedef struct _VENDOR_AIS_ISP_JMISP_INFO_PARM {
	VENDOR_AIS_ISP_MEM_PARM jmisp_info ; 
	UINT32 workbuf_size;
	ULONG reserved[5];
} VENDOR_AIS_ISP_JMISP_INFO_PARM;

/**
	Parameters of net ISP device memory: user-space and kernel-space mapping
*/
typedef struct _VENDOR_AIS_ISP_MAP_MEM_PARM {
	VENDOR_AIS_ISP_MEM_PARM user_parm;
	VENDOR_AIS_ISP_MEM_PARM user_model;
	VENDOR_AIS_ISP_MEM_PARM user_buff;
	VENDOR_AIS_ISP_MEM_PARM kerl_parm;
#if (defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02401_))
	VENDOR_AIS_ISP_MEM_PARM tcm_buff;      // currently used tcm  pa,va,size  (ex. 1MB)
	VENDOR_AIS_ISP_MEM_PARM tcm_buff_map;  // full tcm mmap/ioremap info      (ex. 4MB)
	VENDOR_AIS_ISP_MEM_PARM fake_tcm_buff; // dram to simulate tcm buffer
#endif
} VENDOR_AIS_ISP_MAP_MEM_PARM;

/**
*/
typedef struct _VENDOR_AIS_ISP_MAP_MEM_INFO {
	VENDOR_AIS_ISP_MAP_MEM_PARM parm;
	UINT32 net_id;
} VENDOR_AIS_ISP_MAP_MEM_INFO;

typedef union {
    struct {
        uint8_t sub_stripe_en       : 1;
        uint8_t inppu_pipe_en       : 1;
        uint8_t outppu_pipe_en      : 1;
        uint8_t buf_type            : 1;   // 0: pipe buf in dram; 1: pipe buf in ub;
        uint8_t reuse_pipe_buf      : 1;
    } bit;
    uint8_t byte;
} VENDOR_AIS_ISP_PPU_PIPE_MODE;

typedef struct _VENDOR_AIS_ISP_PARM {
    VENDOR_AIS_ISP_MAP_MEM_INFO p_info ;
    UINT32 input_num ;				// isp model input num
    UINT32 each_ubuf_budget;        // ubuf budget of each core (Byte)
    UINT32 each_dram_iobuf_size;    // dram working buff size of each core (Byte)
    UINT8 total_ubuf_size;          // total ubuf size (MB)
    UINT8 aiisp_mode;               // 0: hw mode 1: sw mode
    UINT8 core_num;                 // num of jmisp
    VENDOR_AIS_ISP_PPU_PIPE_MODE pipe_mode;
    uint8_t pre_in0_en;
    uint8_t reserved_blk0[2];
    uint32_t each_ai_parm_size;
    uint8_t reserved_blk1[28];
    uintptr_t jmisp_info ; 
    UINT32 jmisp_info_size ; 
    uintptr_t p_pl_cmd_list ; 
    UINT32 jmisp_pl_size ; 
    uintptr_t p_ppu_cmd_list ;
    UINT32 jmisp_ppu_size ; 
	uintptr_t p_ai_parm_list ;
	UINT32 ai_parm_size ;
} VENDOR_AIS_ISP_PARM;



typedef struct _VENDOR_AIS_ISP_INPUT_INFO {
	NN_DATA_V30 imem[NN_IMEM_NUM-1]; // expand last for backward compatible
	uintptr_t mctrl_id;
	uintptr_t port_id;
	UINT32 eng_type;
	UINT32 ref_frame_id;
	uintptr_t parm_addr ; 
	uintptr_t in_buff_ofs ; 
	UINT32 net_id;
} VENDOR_AIS_ISP_INPUT_INFO;

typedef struct _VENDOR_AIS_ISP_UBUF_INFO {
	UINT32  proc_id;    // [GET][FREE] set : current proc_id
	INT32   idx;        // [GET] get : available ubuf idx with given free size , (-1) means get fail  [FREE] set : ubuf idx
	UINT32  size_in_mb; // [GET] set : required size(MB)                                              [FREE] set : ubuf size(MB)
} VENDOR_AIS_ISP_UBUF_INFO;

typedef struct _VENDOR_AIS_ISP_POOL_INFO {
	UINT32  proc_id;    
	UINT32  pool_id;    
	UINT32  need_ubuf_size; 
	UINT32  need_core_num;
	UINT32  IsShare_workbuf ; 
} VENDOR_AIS_ISP_POOL_INFO;

typedef struct _VENDOR_AIS_ISP_MASK {
	UINT32  proc_id;    
	UINT32  mask;      // could be core_mask or ub_mask
} VENDOR_AIS_ISP_MASK ;

typedef struct _VENDOR_AIS_ISP_WEIGHT_LOC {
	UINT32  proc_id; 
	UINT8   weight_loc; 
}VENDOR_AIS_ISP_WEIGHT_LOC; 

typedef struct{ 
  UINT32 proc_id;
  ULONG pa;                 ///< physical address
  ULONG va; 
  UINT32 width;
  UINT32 height;
  UINT32 line_ofs;
  UINT32 fmt; // hdal fmt 
  UINT32 proc_time;
}NN_ISP_FRAME;

#define NN_ISP_AIISP_PARAM_MAX  32
typedef struct{
  UINT32 proc_id;
  UINT32 param_num;
  UINT32 param_size[NN_ISP_AIISP_PARAM_MAX];
  ULONG  param_phyaddr[NN_ISP_AIISP_PARAM_MAX];
}NN_ISP_ISP_PARAM;

#define NN_CHK_IPP_VER  1  // the MAX version of IPP which current SDK available to check
typedef struct {
	UINT32 version;        // (in)IPP struct version
	UINT32 proc_id;        // (in)IPP want to check which proc_id
	ULONG reserved[31];
} NN_IPP_CHK_PARAM;
STATIC_ASSERT(sizeof(NN_IPP_CHK_PARAM) <= 0x100);

#define NN_CHK_AI_VER  1   // current AI version. VER should "+1" if following struct changed
typedef struct {
	UINT32 version;        // (out)AI struct version, should be set as NN_CHK_AI_VER
	UINT32 fmt;            // (out)for IPP, but only BPP(b'23~b'16) is actually valid
	ULONG reserved[31];
} NN_AI_CHK_PARAM;
STATIC_ASSERT(sizeof(NN_AI_CHK_PARAM) <= 0x100);

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/
#define VENDOR_AIS_ISP_DEV_NAME                "kflow_ai_isp"

#define VENDOR_AIS_ISP_IOC_MAGIC               'f'


#define KFLOW_AI_ISP_IOC_RESET_ISP_NET				_VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 1, UINT32)
#define KFLOW_AI_ISP_IOC_INIT_ISP_NET				_VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 2, UINT32)
#define KFLOW_AI_ISP_IOC_UNINIT_ISP_NET				_VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 3, UINT32)
#define KFLOW_AI_ISP_IOC_SET_ISP_CB                 _VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 4, uintptr_t)
#define KFLOW_AI_ISP_IOC_GET_ISP_CB                 _VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 5, uintptr_t)
#define KFLOW_AI_ISP_IOC_SET_ISP_INPUT              _VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 6, VENDOR_AIS_ISP_INPUT_INFO)
#define KFLOW_AI_ISP_IOC_PARS_ISP_NET               _VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 7, VENDOR_AIS_ISP_PARM)
#define KFLOW_AI_ISP_IOC_CLOSE_ISP_NET              _VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 8, UINT32)
#define KFLOW_AI_ISP_IOC_GET_VER					_VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 9, void*)
#define KFLOW_AI_ISP_IOC_GET_UBUF_ISP				_VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 10, VENDOR_AIS_ISP_UBUF_INFO)
#define KFLOW_AI_ISP_IOC_FREE_UBUF_ISP				_VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 11, VENDOR_AIS_ISP_UBUF_INFO)
#define KFLOW_AI_ISP_IOC_WEIGHT_LOC					_VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 12, VENDOR_AIS_ISP_WEIGHT_LOC)
#define KFLOW_AI_ISP_IOC_SET_POOL_INFO				_VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 13, VENDOR_AIS_ISP_POOL_INFO)
#define KFLOW_AI_ISP_IOC_GET_PROC_ID_BY_POOL	    _VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 14, VENDOR_AIS_ISP_POOL_INFO)
#define KFLOW_AI_ISP_IOC_DBG_CAL_SLICE				_VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 15, NN_ISP_FRAME)
#define KFLOW_AI_ISP_IOC_DBG_PUSH_FRAME				_VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 16, VENDOR_AIS_ISP_INPUT_INFO)
#define KFLOW_AI_ISP_IOC_GET_JOBM_CORE     _VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 17, VENDOR_AIS_ISP_UBUF_INFO)
#define KFLOW_AI_ISP_IOC_DBG_SET_REF_FRAME			_VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 18, VENDOR_AIS_ISP_INPUT_INFO)
#define KFLOW_AI_ISP_IOC_SET_ISP_INPUT_INFO         _VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 19, VENDOR_AIS_ISP_INPUT_INFO)
#define KFLOW_AI_ISP_IOC_DBG_SET_ISP_PARAM			_VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 20, NN_ISP_ISP_PARAM)
#define KFLOW_AI_ISP_IOC_GET_SET_CORE_MASK			_VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 21, VENDOR_AIS_ISP_MASK)
#define KFLOW_AI_ISP_IOC_GET_SET_UB_MASK			_VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 22, VENDOR_AIS_ISP_MASK)
#define KFLOW_AI_ISP_IOC_GET_JMISP_INFO_BUF			_VOS_IOWR(VENDOR_AIS_ISP_IOC_MAGIC , 23, VENDOR_AIS_ISP_JMISP_INFO_PARM)



#if defined (__UITRON) || defined(__ECOS)  || defined (__FREERTOS)
int vendor_ais_isp_miscdev_ioctl(int fd, unsigned int cmd, void *arg);
int kflow_ai_isp_net_ioctl(int fd, unsigned int cmd, void *arg);
#endif

#endif  /* _KFLOW_AI_NET_H_ */
