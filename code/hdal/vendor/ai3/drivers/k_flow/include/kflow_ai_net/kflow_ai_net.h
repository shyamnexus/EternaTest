/**
	@brief Header file of definition of vendor net flow sample.

	@file net_flow_sample.h

	@ingroup net_flow_sample

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _KFLOW_AI_NET_H_
#define _KFLOW_AI_NET_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#ifdef __KERNEL__
#include <linux/ioctl.h>
#endif
#include "kflow_ai_net/kflow_ai_net_comm.h"
#include "kflow_ai_net/nn_net.h"
#include "kwrap/ioctl.h"
#include "kwrap/semaphore.h"

/********************************************************************
	TYPE DEFINITION
********************************************************************/
/**
	Parameters of net flow device memory
*/
typedef struct _VENDOR_AIS_FLOW_MEM_PARM {
	uintptr_t pa;
	uintptr_t va;
	UINT32 size;
} VENDOR_AIS_FLOW_MEM_PARM;

/**
	Parameters of net flow device memory: user-space and kernel-space mapping
*/
typedef struct _VENDOR_AIS_FLOW_MAP_MEM_PARM {
	VENDOR_AIS_FLOW_MEM_PARM user_parm;		// using in USER SPACE & KERNEL SPACE
	VENDOR_AIS_FLOW_MEM_PARM user_model;	// only using in USER SPACE
	VENDOR_AIS_FLOW_MEM_PARM user_buff;		// only using in USER SPACE
	VENDOR_AIS_FLOW_MEM_PARM kerl_parm;		// useless NOW
#if (defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02401_))
	VENDOR_AIS_FLOW_MEM_PARM tcm_buff;      // currently used tcm  pa,va,size  (ex. 1MB)
	VENDOR_AIS_FLOW_MEM_PARM tcm_buff_map;  // full tcm mmap/ioremap info      (ex. 4MB)
	VENDOR_AIS_FLOW_MEM_PARM fake_tcm_buff; // dram to simulate tcm buffer
#endif
} VENDOR_AIS_FLOW_MAP_MEM_PARM;

/**
*/
typedef struct _VENDOR_AIS_FLOW_MAP_MEM_INFO {
	VENDOR_AIS_FLOW_MAP_MEM_PARM parm;
	UINT32 net_id;
} VENDOR_AIS_FLOW_MAP_MEM_INFO;

/**
*/
typedef struct _VENDOR_AIS_FLOW_CORE_INFO {
	UINT32 info;
} VENDOR_AIS_FLOW_CORE_INFO;

/**
*/
typedef struct _VENDOR_AIS_FLOW_CORE_CFG {
	UINT32 schd;
	UINT32 chk_interval;
} VENDOR_AIS_FLOW_CORE_CFG;

/**
*/
typedef struct _VENDOR_AIS_FLOW_JOBLIST_INFO {
	UINT32 proc_id;
	UINT32 max_job_cnt;
	UINT32 job_cnt;
	UINT32 bind_cnt;
	UINT32 ddr_id;
} VENDOR_AIS_FLOW_JOBLIST_INFO;

/**
*/
typedef struct _VENDOR_AIS_FLOW_TRIG_INFO {
	uintptr_t parm_addr;
    UINT32 trig_src; //for trig
    UINT32 tot_trig_eng_times; //for trig
    UINT32 idea_cycle; //for getcfg
} VENDOR_AIS_FLOW_TRIG_INFO;

typedef struct _VENDOR_AIS_FLOW_JOB_INFO {
	UINT32 proc_id;
	UINT32 job_id;
	uintptr_t info;
	uintptr_t info2;
	UINT32 engine_id;
	UINT32 engine_op;
	UINT32 schd_parm;
    UINT32 job_priority;
	INT32 wait_ms;
	INT32 rv;
    VENDOR_AIS_FLOW_TRIG_INFO trig_info;
} VENDOR_AIS_FLOW_JOB_INFO;

/**
*/
/*
typedef struct _VENDOR_AIS_FLOW_JOB_WAI {
	UINT32 proc_id;
	UINT32 job_id;
} VENDOR_AIS_FLOW_JOB_WAI;

typedef struct _VENDOR_AIS_FLOW_JOB_SIG {
	UINT32 proc_id;
	UINT32 job_id;
} VENDOR_AIS_FLOW_JOB_SIG;
*/

typedef struct _VENDOR_AIS_FLOW_CPU_WAI {
	UINT32 proc_id;
	uintptr_t job_id;
} VENDOR_AIS_FLOW_CPU_WAI;

typedef struct _VENDOR_AIS_FLOW_CPU_SIG {
	UINT32 proc_id;
	uintptr_t job_id;
} VENDOR_AIS_FLOW_CPU_SIG;

typedef struct _VENDOR_AIS_FLOW_DSP_WAI {
	UINT32 proc_id;
	uintptr_t job_id;
} VENDOR_AIS_FLOW_DSP_WAI;

typedef struct _VENDOR_AIS_FLOW_DSP_SIG {
	UINT32 proc_id;
	uintptr_t job_id;
} VENDOR_AIS_FLOW_DSP_SIG;

#define CORE_MAX   32

typedef struct _VENDOR_AIS_CORE_UT {
    CHAR   name[8];  ///< name
    UINT32 time;     ///< time
    UINT32 util;     ///< utility
} VENDOR_AIS_CORE_UT;

typedef struct _VENDOR_AIS_PERF_UT {
    UINT32 core_count;
    VENDOR_AIS_CORE_UT core[CORE_MAX];
} VENDOR_AIS_PERF_UT;

typedef struct _KFLOW_AI_OP_FC_CMDBUF {
	UINT32 proc_id;
	UINT32 buf_size;
    uintptr_t buf_pa;
    uintptr_t buf_va;
} KFLOW_AI_OP_FC_CMDBUF;

typedef struct _KFLOW_AI_OP_MAU_MAX_PARAM {
	UINT32 in_width;
	UINT32 in_byte;
	UINT32 in_obj;
	UINT32 mau_cal_mode;
	UINT32 mau_mat_op;
	UINT32 size;
} KFLOW_AI_OP_MAU_MAX_PARAM;

typedef KFLOW_AI_OP_FC_CMDBUF KFLOW_AI_OP_MAU_CMDBUF;

#define KFLOW_AI_MAX_CMD_LENGTH	200
typedef struct _KFLOW_AI_IOC_CMD_OUT {
	UINT32 proc_id;
	CHAR str[KFLOW_AI_MAX_CMD_LENGTH];
	UINT32 len;
} KFLOW_AI_IOC_CMD_OUT;

typedef struct _KFLOW_AI_IOC_CAT_OUT {
	UINT32 proc_id;
	UINT32 content_obj;
} KFLOW_AI_IOC_CAT_OUT;
/**
*/
typedef struct _VENDOR_AIS_FLOW_PROC_PARM {
	BOOL is_nonblock;
	UINT32 time_out_ms;		///< set 0 for disable time out
	uintptr_t net_addr;
	UINT32 start_layer;
	UINT32 end_layer;
} VENDOR_AIS_FLOW_PROC_PARM;

typedef struct _VENDOR_AI_NET_LAYER {
	UINT32 engine_id;
	UINT32 engine_op;
	UINT32 next_cnt;
	UINT32* next_layer_id;
	void* more_info;
} VENDOR_AI_NET_LAYER;

/**
*/
typedef struct _VENDOR_AIS_FLOW_PROC_INFO {
	VENDOR_AIS_FLOW_PROC_PARM parm;
	UINT32 net_id;
} VENDOR_AIS_FLOW_PROC_INFO;

#if (!defined(__KERNEL__))   // only allow USER SPACE

/**
*/
typedef struct _VENDOR_AIS_FLOW_PROC_INPUT_INFO {
	uintptr_t net_addr;
	NN_DATA_V30 imem[NN_IMEM_NUM];
	UINT32 net_id;
	UINT32 proc_idx;
} VENDOR_AIS_FLOW_PROC_INPUT_INFO;

/**
*/

typedef struct _VENDOR_AIS_FLOW_PROC_OUTPUT_INFO {
	uintptr_t net_addr;
	VENDOR_AIS_FLOW_MEM_PARM out_buf;
	UINT32 net_id;
	UINT32 layer_id;
	UINT32 port_id;
} VENDOR_AIS_FLOW_PROC_OUTPUT_INFO;

/**
*/
typedef struct _VENDOR_AIS_FLOW_UPDATE_INFO {
	UINT32 layer;
	UINT32 net_id;
} VENDOR_AIS_FLOW_UPDATE_INFO;
#endif

/**
*/
typedef struct _VENDOR_AIS_FLOW_VERS {
	UINT32 proc_id;         ///< proc_id
	UINT32 chip_id;         ///< chip_id
	UINT32 gentool_vers;    ///< gentool version
	INT    rv;              ///< version check result
} VENDOR_AIS_FLOW_VERS;

typedef struct _VENDOR_AIS_FLOW_ID {
	UINT32 net_id;
	UINT32 ai_support_net_max;
} VENDOR_AIS_FLOW_ID;

typedef struct _VENDOR_AIS_MEM_INFO {
	UINT64 value[8];
} VENDOR_AIS_MEM_INFO;

#if (!defined(__KERNEL__))   // only allow USER SPACE

/**
*/
typedef struct _VENDOR_AIS_DIFF_MODEL_INFO {
	UINT32 input_width;
	UINT32 input_height;
	UINT32 id;
} VENDOR_AIS_DIFF_MODEL_INFO;
#endif

typedef struct _VENDOR_AIS_FLOW_UPDATE_NET_INFO {
	VENDOR_AIS_FLOW_MAP_MEM_PARM map_parm;
	uintptr_t net_info_pa;
	UINT32 net_info_size;
	UINT32 model_id;
	UINT32 net_id;
    UINT32 job_id;
    uintptr_t parm_addr;
} VENDOR_AIS_FLOW_UPDATE_NET_INFO;

typedef struct _VENDOR_AIS_FLOW_REAL_JOB_OUT_INFO {
	UINT32 proc_id;
    UINT32 job_id;
    NN_DATA_V30 real_sao[3];
} VENDOR_AIS_FLOW_REAL_JOB_OUT_INFO;

#if (!defined(__KERNEL__))   // only allow USER SPACE

typedef struct _VENDOR_AIS_DIFF_BATCH_MODEL_INFO {
	UINT32 batch_num;
	UINT32 id;
} VENDOR_AIS_DIFF_BATCH_MODEL_INFO;

#endif

#define VENDOR_AI_VERSION_LEN	20
typedef struct _KFLOW_AI_IOC_VERSION {
	UINT32 vendor_ai_version;
	CHAR vendor_ai_impl_version[VENDOR_AI_VERSION_LEN];
} KFLOW_AI_IOC_VERSION;

typedef struct _KFLOW_AI_MODEL_VERSION {
	UINT32 proc_id;         ///< proc_id
	UINT32 nn_chip;         ///< NN_CHIP_FMT + NN_CHIP_ID
	UINT32 gentool_vers;    ///< gentool version
	UINT32 real_chip;       ///< CHIP_ID
} KFLOW_AI_MODEL_VERSION;

typedef struct _KFLOW_AI_IOC_DEBUG_INFO {
	UINT32 kcmd_proc_id;
	UINT32 kcmd_proc_trace;
	UINT32 kcmd_group_dump_bmp;
	UINT32 kcmd_iomem_dump_bmp;
	INT32 kcmd_set_preserve_bufid;
} KFLOW_AI_IOC_DEBUG_INFO;

typedef struct _KFLOW_AI_IOC_DEBUG_LVL {
	UINT32 lvl;
} KFLOW_AI_IOC_DEBUG_LVL;

#if (!defined(__KERNEL__))   // only allow USER SPACE

typedef struct _VENDOR_AIS_MAP_TABLE_INFO {
	UINT32 proc_id;
	UINT32 max_batch;
} VENDOR_AIS_MAP_TABLE_INFO;
#endif

typedef struct _VENDOR_AIS_SET_DEBUG_INFO {
	UINT32 proc_id;
	UINT32 debug_item;  // (1) set 0 to clear all debug  (2) bit mask OR of KFLOW_AI_DBG_OBUF, KFLOW_AI_DBG_TIME, ...  , available value see kflow_ai_core.h
} VENDOR_AIS_SET_DEBUG_INFO;

typedef struct _VENDOR_AIS_UBUF_INFO {
	UINT32  proc_id;    // [GET][FREE] set : current proc_id
	INT32   idx;        // [GET] get : available ubuf idx with given free size , (-1) means get fail  [FREE] set : ubuf idx
	UINT32  size_in_mb; // [GET] set : required size(MB)                                              [FREE] set : ubuf size(MB)
} VENDOR_AIS_UBUF_INFO;

#if (!defined(__KERNEL__))   // only allow USER SPACE

typedef struct _VENDOR_AIS_BIND_CORE_INFO {
	UINT32 proc_id;
	UINT32 core_mask;
} VENDOR_AIS_BIND_CORE_INFO;
#endif

#if (!defined(__KERNEL__))   // only allow USER SPACE

typedef struct _VENDOR_AIS_REMAP_UBUF_ADDR_INFO {
	UINT32 proc_id;
	VENDOR_AIS_FLOW_MEM_PARM new_ubuf;
} VENDOR_AIS_REMAP_UBUF_ADDR_INFO;
#endif

typedef struct _VENDOR_AIS_FLOW_GBLK_DRV_INFO {
	UINT32 proc_id;
	uintptr_t drv_info_pa;
} VENDOR_AIS_FLOW_GBLK_DRV_INFO;

typedef struct _VENDOR_AIS_FLOW_POSTPROC_DRV_INFO {
	UINT32 proc_id;
	uintptr_t drv_info_pa;
} VENDOR_AIS_FLOW_POSTPROC_DRV_INFO;

// this struct declare is moved from kflow_ai_net_flow.c to here => let other .c to extern kflow_ai_net_global to get baseaddr
typedef struct _KFLOW_AI_NET_GLOBAL {
	VENDOR_AIS_FLOW_MAP_MEM_PARM    *g_ai_map_mem;
	VENDOR_AIS_FLOW_MAP_MEM_PARM    *g_ai_user_mem_in_kerl;
	BOOL                                                    *g_ai_net_state;
	SEM_HANDLE                                              *g_ai_state_SEM_ID;
	BOOL                                                    mem_dump;
	VENDOR_AIS_MEM_INFO                     mem_info;
} KFLOW_AI_NET_GLOBAL;

typedef struct _VENDOR_AIS_GBlkTile_POOL_INFO {
	UINT32  proc_id;    
	UINT32  pool_id;    
	UINT32  need_ubuf_num; 
	UINT32  need_core_num;
} VENDOR_AIS_GBlkTile_POOL_INFO;

typedef struct _VENDOR_AIS_GBlkTile_MASK {
	UINT32  proc_id;    
	UINT32  mask;      // could be core_mask or ub_mask
} VENDOR_AIS_GBlkTile_MASK ;

typedef enum _VENDOR_AI3_PERF_TIME_CLASS_ID {
    PERF_NET_GET            = 0,
    PERF_NET_SET            = 1,
    PERF_NET_OPEN        	= 2,
    PERF_NET_START          = 3,
    PERF_NET_PROC           = 4,
    PERF_NET_STOP  	        = 5,
    PERF_NET_CLOSE          = 6,
    PERF_FLOAT_TO_FIXED     = 7,
    PERF_FIXED_TO_FLOAT  	= 8,
    ENUM_DUMMY4WORD(VENDOR_AI3_PERF_TIME_CLASS_ID)
} VENDOR_AI3_PERF_TIME_CLASS_ID;

typedef struct _VENDOR_AIS_FLOW_TIMELINE_JOB_INFO {
	UINT32 proc_id;
    UINT32 class;
    UINT32 param[8];
    UINT64 ts_start;
    UINT64 ts_end;
} VENDOR_AIS_FLOW_TIMELINE_JOB_INFO;

typedef struct _VENDOR_AIS_FLOW_TIMELINE_INFO {
	UINT32 timeline_func;
    UINT32 timeline_buf_size;
    UINT32 reserve[8];
} VENDOR_AIS_FLOW_TIMELINE_INFO;

typedef struct _VENDOR_AIS_DTSI_INFO {
	UINT32 ai_maxisp;
	UINT32 ai_maxubuf;
	UINT32 reserve[8];
} VENDOR_AIS_DTSI_INFO;

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/
#define VENDOR_AIS_FLOW_DEV_NAME                "kflow_ai_net"

#define VENDOR_AIS_FLOW_IOC_MAGIC               'f'

/*
#define VENDOR_AIS_FLOW_IOC_GET_MEM_TYPE        _VOS_IOR (VENDOR_AIS_FLOW_IOC_MAGIC,  1, VENDOR_AIS_FLOW_IOC_PARM)
#define VENDOR_AIS_FLOW_IOC_SET_MEM_TYPE        _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC,  2, VENDOR_AIS_FLOW_IOC_PARM)
#define VENDOR_AIS_FLOW_IOC_GET_MEM_OFFSET      _VOS_IOR (VENDOR_AIS_FLOW_IOC_MAGIC,  3, VENDOR_AIS_FLOW_IOC_PARM)
#define VENDOR_AIS_FLOW_IOC_SET_MEM_OFFSET      _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC,  4, VENDOR_AIS_FLOW_IOC_PARM)
#define VENDOR_AIS_FLOW_IOC_GET_MEM_LIMITSZ     _VOS_IOR (VENDOR_AIS_FLOW_IOC_MAGIC,  5, VENDOR_AIS_FLOW_IOC_PARM)
#define VENDOR_AIS_FLOW_IOC_SET_MEM_LIMITSZ     _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC,  6, VENDOR_AIS_FLOW_IOC_PARM)
#define VENDOR_AIS_FLOW_IOC_SET_INIT            _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC,  7, VENDOR_AIS_FLOW_IOC_INIT_PARM *)
#define VENDOR_AIS_FLOW_IOC_SET_UNINIT          _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC,  8, VENDOR_AIS_FLOW_IOC_PARM)
#define VENDOR_AIS_FLOW_IOC_GET_CONFIG          _VOS_IOR (VENDOR_AIS_FLOW_IOC_MAGIC,  9, VENDOR_AIS_FLOW_IOC_PARM)
#define VENDOR_AIS_FLOW_IOC_SET_CONFIG          _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 10, VENDOR_AIS_FLOW_IOC_PARM)
#define VENDOR_AIS_FLOW_IOC_SET_PROCESS         _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 11, VENDOR_AIS_FLOW_IOC_PARM)
#define VENDOR_AIS_FLOW_IOC_GET_RESULT          _VOS_IOR (VENDOR_AIS_FLOW_IOC_MAGIC, 12, VENDOR_AIS_FLOW_IOC_PARM)
#define VENDOR_AIS_FLOW_IOC_GET_MEM             _VOS_IOR (VENDOR_AIS_FLOW_IOC_MAGIC, 13, VENDOR_AIS_FLOW_IOC_PARM)
#define VENDOR_AIS_FLOW_IOC_SET_MEM             _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 14, VENDOR_AIS_FLOW_IOC_PARM)
*/
#if (FLOW_AI_DEV == 1)
#define VENDOR_AIS_FLOW_IOC_VERS                _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC,  1, VENDOR_AIS_FLOW_VERS)
#define VENDOR_AIS_FLOW_IOC_GET_VER             _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC,  2, void*)
#define VENDOR_AIS_FLOW_IOC_GET_NUM             _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC,  3, VENDOR_AIS_FLOW_ID)
#define VENDOR_AIS_FLOW_IOC_MEM_INFO       		_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC,  4, VENDOR_AIS_MEM_INFO)
#define VENDOR_AIS_FLOW_IOC_GET_CHIP_ID         _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC,  5, void*)
#define VENDOR_AIS_FLOW_IOC_GET_DTSI_INFO       _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC,  6, VENDOR_AIS_DTSI_INFO)
#endif

#if (FLOW_AI_DEV == 1)
#define VENDOR_AIS_FLOW_IOC_NET_RESET           _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 10, VENDOR_AIS_FLOW_ID)
#define VENDOR_AIS_FLOW_IOC_NET_INIT            _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 11, VENDOR_AIS_FLOW_ID)
#define VENDOR_AIS_FLOW_IOC_NET_UNINIT          _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 12, VENDOR_AIS_FLOW_ID)
#endif
#define VENDOR_AIS_FLOW_IOC_NET_LOCK            _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 13, VENDOR_AIS_FLOW_ID)
#define VENDOR_AIS_FLOW_IOC_NET_UNLOCK          _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 14, VENDOR_AIS_FLOW_ID)

#if (FLOW_AI_NET == 1)
#define VENDOR_AIS_FLOW_IOC_REMAP_ADDR          _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 15, VENDOR_AIS_FLOW_MAP_MEM_INFO)
#define VENDOR_AIS_FLOW_IOC_UNMAP_ADDR          _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 16, VENDOR_AIS_FLOW_MAP_MEM_INFO)
#endif
#if (FLOW_AI_DYSCALE == 1)
#define VENDOR_AIS_FLOW_IOC_PROC_UPDATE_INFO    _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 26, VENDOR_AIS_FLOW_PROC_INFO)
#define VENDOR_AIS_FLOW_IOC_PROC_UPDATE_TRIG    _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 27, VENDOR_AIS_FLOW_JOB_INFO)
#define VENDOR_AIS_FLOW_IOC_PROC_UPDATE_OUT     _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 28, VENDOR_AIS_FLOW_REAL_JOB_OUT_INFO)
#define VENDOR_AIS_FLOW_IOC_PROC_UPDATE_SUB     _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 29, VENDOR_AIS_FLOW_PROC_INFO)
#endif

#if (FLOW_AI_JOB == 1)
//CORE
#define VENDOR_AIS_FLOW_IOC_CORE_RESET         	_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 30, VENDOR_AIS_FLOW_CORE_INFO)
#define VENDOR_AIS_FLOW_IOC_CORE_INIT         	_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 31, VENDOR_AIS_FLOW_CORE_INFO)
#define VENDOR_AIS_FLOW_IOC_CORE_UNINIT         _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 32, VENDOR_AIS_FLOW_CORE_INFO)
//JOB
#define VENDOR_AIS_FLOW_IOC_NEW_JOBLIST         _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 33, VENDOR_AIS_FLOW_JOBLIST_INFO)
#define VENDOR_AIS_FLOW_IOC_DEL_JOBLIST         _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 34, VENDOR_AIS_FLOW_JOBLIST_INFO)
#define VENDOR_AIS_FLOW_IOC_DUMP_JOBLIST        _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 35, VENDOR_AIS_FLOW_JOBLIST_INFO)
#endif
#if (FLOW_AI_DEV == 1)
#define VENDOR_AIS_FLOW_IOC_COMMON_LOCK         _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 36, VENDOR_AIS_FLOW_ID)
#define VENDOR_AIS_FLOW_IOC_COMMON_UNLOCK       _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 37, VENDOR_AIS_FLOW_ID)
#endif
#if (FLOW_AI_JOB == 1)
#define VENDOR_AIS_FLOW_IOC_CORE_CFGSCHD        _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 39, VENDOR_AIS_FLOW_CORE_INFO)

#define VENDOR_AIS_FLOW_IOC_CLR_JOB         	_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 40, VENDOR_AIS_FLOW_JOB_INFO)
#endif
#if (FLOW_AI_JOB == 1)
#define VENDOR_AIS_FLOW_IOC_SET_JOB         	_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 41, VENDOR_AIS_FLOW_JOB_INFO)
#define VENDOR_AIS_FLOW_IOC_BIND_JOB         	_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 42, VENDOR_AIS_FLOW_JOB_INFO)
#define VENDOR_AIS_FLOW_IOC_UNBIND_JOB         	_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 43, VENDOR_AIS_FLOW_JOB_INFO)
#define VENDOR_AIS_FLOW_IOC_PUSH_JOB         	_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 44, VENDOR_AIS_FLOW_JOB_INFO)
#define VENDOR_AIS_FLOW_IOC_PULL_JOB         	_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 45, VENDOR_AIS_FLOW_JOB_INFO)
#define VENDOR_AIS_FLOW_IOC_LOCK_JOB         	_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 46, VENDOR_AIS_FLOW_JOB_INFO)
#define VENDOR_AIS_FLOW_IOC_UNLOCK_JOB         	_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 47, VENDOR_AIS_FLOW_JOB_INFO)
#endif
#if (FLOW_AI_OP == 1)
#define VENDOR_AIS_FLOW_IOC_SET_JOB2         	_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 48, VENDOR_AIS_FLOW_JOB_INFO)
#endif
#define VENDOR_AIS_FLOW_IOC_SET_USAGE_LIMIT     _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 49, UINT32)
#if (FLOW_AI_JOB == 1)
#define VENDOR_AIS_FLOW_IOC_WAI_JOB         	_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 50, VENDOR_AIS_FLOW_JOB_WAI)
#define VENDOR_AIS_FLOW_IOC_SIG_JOB         	_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 51, VENDOR_AIS_FLOW_JOB_SIG)
#define VENDOR_AIS_FLOW_IOC_WAI_CPU         	_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 52, VENDOR_AIS_FLOW_CPU_WAI)
#define VENDOR_AIS_FLOW_IOC_SIG_CPU         	_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 53, VENDOR_AIS_FLOW_CPU_SIG)
#define VENDOR_AIS_FLOW_IOC_WAI_DSP         	_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 54, VENDOR_AIS_FLOW_DSP_WAI)
#define VENDOR_AIS_FLOW_IOC_SIG_DSP         	_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 55, VENDOR_AIS_FLOW_DSP_SIG)

#define VENDOR_AIS_FLOW_IOC_CORE_CFGCHK         _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 56, VENDOR_AIS_FLOW_CORE_INFO)
#endif

#if (FLOW_AI_DEBUG == 1)
#define KFLOW_AI_IOC_CMD_OUT_INIT     			_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 60, KFLOW_AI_IOC_CMD_OUT)
#define KFLOW_AI_IOC_CMD_OUT_UNINIT     		_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 61, KFLOW_AI_IOC_CMD_OUT)
#define KFLOW_AI_IOC_CMD_OUT_PROG_DEBUG  		_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 62, KFLOW_AI_IOC_CMD_OUT)
#define KFLOW_AI_IOC_CMD_OUT_RUN_DEBUG  		_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 63, KFLOW_AI_IOC_CMD_OUT)
#define KFLOW_AI_IOC_CMD_OUT_WAI     			_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 64, KFLOW_AI_IOC_CMD_OUT)
#define KFLOW_AI_IOC_CMD_OUT_SIG     			_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 65, KFLOW_AI_IOC_CMD_OUT)
#endif
#if (FLOW_AI_DEBUG == 1)
#define KFLOW_AI_IOC_SET_VERSION     			_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 66, KFLOW_AI_IOC_VERSION)
#endif
#if (FLOW_AI_DEBUG == 1)
#define KFLOW_AI_IOC_GET_DEBUG_INFO    			_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 67, KFLOW_AI_IOC_DEBUG_INFO)
#endif
#if (FLOW_AI_JOB == 1)
#define KFLOW_AI_IOC_CMD_PERF_UT_BEGIN     		_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 68, UINT32)
#define KFLOW_AI_IOC_CMD_PERF_UT_END			_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 69, VENDOR_AIS_PERF_UT)
#endif
#if (FLOW_AI_OP == 1)
#define VENDOR_AIS_FLOW_IOC_OP_LOCK             _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 70, VENDOR_AIS_FLOW_ID)
#define VENDOR_AIS_FLOW_IOC_OP_UNLOCK           _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 71, VENDOR_AIS_FLOW_ID)
#endif
#if (FLOW_AI_DEBUG == 1)
#define KFLOW_AI_IOC_MSG_OUT_WAI     			_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 72, KFLOW_AI_IOC_CMD_OUT)
#define KFLOW_AI_IOC_MSG_OUT_SIG     			_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 73, KFLOW_AI_IOC_CMD_OUT)
#endif
#if (FLOW_AI_DEV == 1)
#define KFLOW_AI_IOC_CMD_DMA_ABORT     			_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 80, void*)
#define KFLOW_AI_IOC_CACHE_CHECK                _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 81, uintptr_t)
#endif

#if (FLOW_AI_PROCID == 1)
#define VENDOR_AIS_FLOW_IOC_GET_ID              _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 92, void*)
#define VENDOR_AIS_FLOW_IOC_RELEASE_ID          _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 93, void*)
#endif
#if (FLOW_AI_DEV == 1)
#define VENDOR_AIS_FLOW_IOC_OPEN_PATH           _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 94, void*)
#define VENDOR_AIS_FLOW_IOC_CLOSE_PATH          _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 95, void*)
#define VENDOR_AIS_FLOW_IOC_MULTI_PROCESS       _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 96, void*)
#endif
#if (FLOW_AI_DEBUG == 1)
#define KFLOW_AI_IOC_GET_DEBUG_LVL    			_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 99, KFLOW_AI_IOC_DEBUG_LVL)
#endif
#if (FLOW_AI_UBUFID == 1)
#define VENDOR_AIS_FLOW_IOC_GET_UBUF_NORMAL     _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 104, VENDOR_AIS_UBUF_INFO)
#define VENDOR_AIS_FLOW_IOC_FREE_UBUF_NORMAL    _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 105, VENDOR_AIS_UBUF_INFO)
#endif
#if (FLOW_AI_DEBUG == 1)
#define VENDOR_AIS_FLOW_IOC_SET_DEBUG     		_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 110, VENDOR_AIS_SET_DEBUG_INFO)
#define VENDOR_AIS_FLOW_IOC_GET_DEBUG     		_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 111, VENDOR_AIS_SET_DEBUG_INFO)
#endif

#if (FLOW_AI_DEBUG == 1)
#define KFLOW_AI_IOC_CMD_CHECK_IS_INIT     		_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 115, UINT32)
#endif
#if (FLOW_AI_DEBUG == 1)
#define VENDOR_AIS_GET_OBUF_PATH				_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 116, void*)
#endif

#define VENDOR_AIS_GET_MAX_ISP				    _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 117, void*)
#define VENDOR_AIS_SW_RESET                     _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 118, VENDOR_AIS_FLOW_ID)
#if (FLOW_AI_DEBUG == 1)
#define KFLOW_AI_IOC_CAT_CHECK_IS_INIT     		_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 130, UINT32)
#define KFLOW_AI_IOC_CAT_OUT_INIT     			_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 131, KFLOW_AI_IOC_CAT_OUT)
#define KFLOW_AI_IOC_CAT_OUT_UNINIT     		_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 132, KFLOW_AI_IOC_CAT_OUT)
#define KFLOW_AI_IOC_CAT_OUT_WAI     			_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 133, KFLOW_AI_IOC_CAT_OUT)
#define KFLOW_AI_IOC_CAT_OUT_SIG     			_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 134, KFLOW_AI_IOC_CAT_OUT)
#define KFLOW_AIS_CAT_STR_OUT				    _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 135, void*)
#endif
#define KFLOW_AI_IOC_GBLK_HAS_POOL		 		_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 139, BOOL)
#define VENDOR_AIS_FLOW_IOC_NEW_SUB_JOBLIST     _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 140, VENDOR_AIS_FLOW_JOBLIST_INFO)
#define VENDOR_AIS_FLOW_IOC_DEL_SUB_JOBLIST     _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 141, VENDOR_AIS_FLOW_JOBLIST_INFO)
#define VENDOR_AIS_FLOW_IOC_SET_SUB_JOB         _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 142, VENDOR_AIS_FLOW_JOB_INFO)
#define KFLOW_AI_IOC_CMD_PERF_TIMELINE_BEGIN    _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 143, void*)
#define KFLOW_AI_IOC_CMD_PERF_TIMELINE_END		_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 144, void*)
#define KFLOW_AI_IOC_SET_GBLK_POOL_INFO 		_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 145, void*)
#define KFLOW_AI_IOC_GET_GBLK_POOL_UBUF		    _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 146, void*)
#define KFLOW_AI_IOC_GET_GBLK_POOL_JOBM_CORE	_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 147, void*)
#define KFLOW_AI_IOC_CMD_SET_TIMELINE_JOB_INFO  _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 148, VENDOR_AIS_FLOW_TIMELINE_JOB_INFO)
#define KFLOW_AI_IOC_CMD_SET_TIMELINE_INFO      _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 149, VENDOR_AIS_FLOW_TIMELINE_INFO)

#define KFLOW_AI_IOC_GET_GBLK_DRV_INFO          _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 150, VENDOR_AIS_FLOW_GBLK_DRV_INFO)
#define KFLOW_AI_IOC_SET_GBLK_CORE_MASK     	_VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 151, VENDOR_AIS_GBlkTile_MASK)
#define KFLOW_AI_IOC_SET_GBLK_UB_MASK     	    _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 152, VENDOR_AIS_GBlkTile_MASK)
#define KFLOW_AI_IOC_WAIT_MISCDEV_RELEASE_DONE  _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 160, VENDOR_AIS_FLOW_GBLK_DRV_INFO)

#define KFLOW_AI_IOC_GET_POSTPROC_DRV_INFO      _VOS_IOWR(VENDOR_AIS_FLOW_IOC_MAGIC, 170, VENDOR_AIS_FLOW_POSTPROC_DRV_INFO)

#if defined (__UITRON) || defined(__ECOS)  || defined (__FREERTOS)
int vendor_ais_flow_miscdev_ioctl(int fd, unsigned int cmd, void *arg);
int kflow_ai_net_ioctl(int fd, unsigned int cmd, void *arg);
#endif

#endif  /* _KFLOW_AI_NET_H_ */
