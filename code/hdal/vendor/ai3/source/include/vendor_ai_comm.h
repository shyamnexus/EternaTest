/**
	@brief Header file of definition of vendor user-space net flow sample.

	@file net_flow_user_sample.h

	@ingroup net_flow_user_sample

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _VENDOR_AI_COMM_H_
#define _VENDOR_AI_COMM_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include <stdio.h>
#include <string.h>
#include "hd_type.h"
#include "kflow_ai_net/kflow_ai_net.h"
#include "vendor_ai_net/nn_parm.h"
#include "vendor_ai_internal.h"
#include "vendor_ai.h"
#include "vendor_ai_plugin.h"
#include "vendor_ai_net/vendor_ai_net_group.h"

#if defined(__FREERTOS)
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h>
#endif

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/
#define NN_SUPPORT_NET_MAX		AI_SUPPORT_NET_MAX

/********************************************************************
	TYPE DEFINITION
********************************************************************/
/*-----------------------------------------------------------------------------*/
/* Local Constant Definitions												   */
/*-----------------------------------------------------------------------------*/
#define DEFAULT_PROC_CNT   4

/*-----------------------------------------------------------------------------*/
/* Local Types Declarations 												   */
/*-----------------------------------------------------------------------------*/
typedef struct _SRC_LARER_INFO {
	INT32  cnt;           ///< counter for set multiple input
	UINT32 in_buf_num;    ///< number of input buffers (if larger than 1 means multiple input)
} SRC_LARER_INFO;

typedef struct _VENDOR_AI_FLOAT_IN_BUF {
	uintptr_t va;        ///< virtual address
    uintptr_t pa;        ///< physical address
	UINT32 input_idx;    ///< input idx
    UINT32 width;        ///< input width
    UINT32 height;       ///< input height
} VENDOR_AI_FLOAT_IN_BUF;

typedef struct _VENDOR_AI_FLOAT_OUT_BUF {
	uintptr_t va;        ///< virtual address
    uintptr_t pa;        ///< physical address
	INT32 layer_id;    ///< output layer idx 
	INT32 port_id;    ///< output port idx 
} VENDOR_AI_FLOAT_OUT_BUF;

typedef struct _VENDOR_AI_NET_PRIV {
	VENDOR_AIS_FLOW_MEM_PARM     init_buf;
	VENDOR_AIS_FLOW_MEM_PARM     work_buf; // whole work buf, including io_buf
	VENDOR_AIS_FLOW_MEM_PARM     rbuf_buf; // whole read only buf
	VENDOR_AIS_FLOW_MEM_PARM     io_buf;   // io_buf
	VENDOR_AIS_FLOW_MEM_PARM     group_buf;
	SRC_LARER_INFO               src_layer;
	INT                          io_buf_alloc;
	INT                          init_buf_alloc;
	INT                          b_simplify_bin;
	INT                          fastboot_dump;
	VOID*						 usr_info;
	UINT32                       ext_info_cnt; // total counts of external info
} VENDOR_AI_NET_PRIV;

typedef struct _VENDOR_AI_DIFF_MODEL_RESINFO {
	UINT32 curr_dim_id;						///< current resolution id
    UINT32 curr_batch_id;					///< current batch id
	HD_DIM curr_dim;						///< current resolution
    HD_DIM new_dim;							///< resolution to be updated
    UINT32 curr_batch_num;					///< current batch num
    UINT32 new_batch_num;					///< batch num to be updated
} VENDOR_AI_DIFF_MODEL_RESINFO;

typedef struct _VENDOR_AI_COMMON_INFO {
	VENDOR_AI_PROC_SCHD           schd;
	UINT32						  chk_interval;
	UINT32                        share_model_mode;
	UINT32                        is_multi;
	UINT32                        init_num;
	UINT32                        not_check_input_align;
	UINT32                        not_check_output_align;
    VENDOR_AI_PERF_TIMELINE       timeline_info;
} VENDOR_AI_COMMON_INFO;

typedef struct _VENDOR_AI_BATCH_INFO {
	UINT32 id;						///< batch id
	UINT32 num;						///< batch number
	UINT32 enable;
} VENDOR_AI_BATCH_INFO;


typedef struct _VENDOR_AI_CPU_TASK {
	UINT32 proc_id;
	int en;
	pthread_t  thread_id;
	int cpu_id;
} VENDOR_AI_CPU_TASK;

#if (CUST_PROC_ALLOC_MEM==1)
typedef struct _VENDOR_AI_CUSTOM_CTX {
	uintptr_t* next_cust_parm_addr;
	UINT32* next_cust_match_in_idx;
} VENDOR_AI_CUSTOM_CTX;
#endif

typedef struct _VENDOR_AI_DSP_TASK {
	UINT32 proc_id;
	int en;
	pthread_t  thread_id;
	int dsp_id;
} VENDOR_AI_DSP_TASK;

typedef struct _VENDOR_AI_NET_CFG_PROC {

	VENDOR_AI_NET_CFG_MODEL 	    cfg_model;	   // user_parm  only
	VENDOR_AI_NET_CFG_MODEL 	    cfg_share_model; // user_model only
	VENDOR_AI_NET_CFG_JOB_OPT	    job_opt;
	VENDOR_AI_NET_CFG_BUF_OPT	    buf_opt;
	VENDOR_AI_DIFF_MODEL_RESINFO    diff_resinfo;
	VENDOR_AI_NET_CFG_INTLBUF 	    intlbuf;         // internal buffer
	INT                             b_is_used;		// get_id, release_id
	VENDOR_AI_BATCH_INFO			batch_info;
	INT                             b_is_using_TCM; // old, don't use
	INT                             b_is_jmisp_model;
	BOOL                            b_is_using_pool;  // if ai-pool-cfg exit 
	BOOL                            b_is_need_ubuf;   // if this model need ubuf
	UINT32                          ubuf_need_size;   // how many ubuf size(Bytes) need
	UINT32                          ubuf_need_size_mb;// how many ubuf size(MB) need
	INT32                           ubuf_idx;         // success request ubuf idx (0~3), one for 1 MB, idx=(-1) mean invalid
	VENDOR_AI_NET_CFG_BUF_CTRL      buf_ctrl;
    UINT32                          job_priority;
    UINT32                          curr_core_mask;
    UINT32                          used_core_mask;
    UINT32                          new_core_mask;
    INT                             b_is_set_core_mask;
    INT                             b_is_debug_model_bin;
    VENDOR_AI_ENGINE_PLUGIN         engine_plug[4]; // IVOT_N12047_CO-610 : move from comm_info to each proc_id
#if (CUST_PROC_ALLOC_MEM==1)
	VENDOR_AI_CUSTOM_CTX            custom_ctx;   // cpu NN_CUSTOM variable
#endif
	UINT32                          model_type_attr;

	//global block tile
	INT                             b_is_gblktile_model;
} VENDOR_AI_NET_CFG_PROC;

typedef struct _VENDOR_AI_NET_INFO_PROC {
	VENDOR_AI_NET_INFO            info;
	VENDOR_AI3_BUF				  input_img;
	VENDOR_AI3_BUF				  input2_img;
    VENDOR_AI3_BUF				  input3_img;
	VENDOR_AI3_BUF				  output_img;
	VENDOR_AI3_BUF				  output2_img;
    VENDOR_AI3_BUF				  output3_img;
	VENDOR_AIS_FLOW_MAP_MEM_PARM  mem_manager;
	VENDOR_AI_NET_CFG_WORKBUF 	  workbuf;         // record whole working buffer, which is allocated by user(VENDOR_AI_NET_PARAM_CFG_WORKBUF) or hdal(_vendor_ai_net_work_buf_alloc).
	VENDOR_AI_NET_CFG_WORKBUF 	  float_in_buf_head;     // float in buffer head 
	VENDOR_AI_NET_CFG_WORKBUF 	  float_in_buf;         // float in buffer info at the moment 
	VENDOR_AI_NET_CFG_WORKBUF 	  float_out_buf_head;     // float out buffer head  (gblktile_model only)
	VENDOR_AI_NET_CFG_WORKBUF 	  float_out_buf;         // float out buffer info at the moment (gblktile_model only)
    VENDOR_AI_NET_CFG_WORKBUF 	  float_cmd_buf;        // float command buf
    //VENDOR_AI_NET_CFG_WORKBUF 	  float_out_cmd_buf;    // float out command buf
	VENDOR_AI_NET_CFG_WORKBUF 	  iobuf;           // io buffer
	VENDOR_AI_NET_CFG_RONLYBUF	  rbuf;			   // read only buffer
	VENDOR_AI_NET_CFG_WORKBUF 	  fake_ubuf;           // fake ubuf
	//private data
	VENDOR_AI_NET_PRIV		        priv;
	VENDOR_AI3_NET_INFO             net_info ;
	UINT32                          debug_mask;
    UINT32                          tmp_proc_id; //tmp_proc_id for float in, float out
	UINT32                          debug_value;
    UINT32                          force_timeline_all;
	UINT32                          design_in_model; // AIISP design-in model
	UINT32                          not_check_input_align; // not check input alignment
	UINT32                          not_check_output_align; // not check output alignment
	//gen_init
	UINT32							gen_init;
	VENDOR_AIS_FLOW_MAP_MEM_PARM	map_mem;

	//job-group
	VENDOR_AI_NET_GROUP_MEM 		group_priv;

	//task to exetute cpu and dps
	VENDOR_AI_CPU_TASK				cpu_task;
	VENDOR_AI_DSP_TASK				dsp_task;

	//global block tile
	KDRV_AI_JMISP_PATH_INFO         *p_jmisp_path_info;

	//single postproc info
	KDRV_AI_JMISP_PATH_INFO         *postproc_jmisp_path_info;

    UINT64                          float_to_fixed_start;
    UINT64                          float_to_fixed_end;
    UINT64                          fixed_to_float_start;
    UINT64                          fixed_to_float_end;

	//reserved
	UINT32 mode;                            	// mode
#if 0
	//ep related
	VENDOR_AI_EP_BUF_INFO           ep_buf_info;
	
#endif
} VENDOR_AI_NET_INFO_PROC;

typedef struct _VENDOR_AI_INFO {
	VENDOR_AI_NET_CFG_PROC *cfg;
	VENDOR_AI_NET_INFO_PROC  **proc;
} VENDOR_AI_INFO;


typedef enum {
	GET_PORT_TYPE_IN,
	GET_PORT_TYPE_OUT
} GET_PORT_TYPE;

/*-----------------------------------------------------------------------------*/
/* Local Macros Declarations												   */
/*-----------------------------------------------------------------------------*/
/*
#define DBG_ERR(fmtstr, args...) printf("\033[0;31mERR:%s() \033[0m" fmtstr, __func__, ##args)
#define DBG_WRN(fmtstr, args...) printf("\033[0;33mWRN:%s() \033[0m" fmtstr, __func__, ##args)
#define DBG_DUMP(fmtstr, args...) printf(fmtstr, ##args)
*/

#define VENDOR_AI_GET_LAYER(id)                   ((id) & 0x0000ffff)
#define VENDOR_AI_GET_IN(id)                      (((id) & 0x0fff0000) >> 16)
#define VENDOR_AI_GET_OUT(id)                     (((id) & 0x0fff0000) >> 16)

#define VENDOR_AI_PARAM_TYPE_LAYER                0x80000000
#define VENDOR_AI_PARAM_TYPE_IN                   0x90000000
#define VENDOR_AI_PARAM_TYPE_OUT                  0xa0000000
#define VENDOR_AI_PARAM_BUF_ID                    0x60000000
#define VENDOR_AI_PARAM_EXT_ID                    0x70000000
#define VENDOR_AI_PARAM_OUT_ID                    0xb0000000
#define VENDOR_AI_GET_PARAM_TYPE(id)              ((id) & 0xf0000000)


/*-----------------------------------------------------------------------------*/
/* Debug Variables & Functions                                                 */
/*-----------------------------------------------------------------------------*/
//--- NOTE : those define should sync with "kflow_ai_net/kflow_ai_core.h"   <== can't just include this header... will result in build error for redefinition of "struct list_head"
#define KFLOW_AI_DBG_BIND         0x00000001  //dump net graph before net proc
#define KFLOW_AI_DBG_SCHD         0x00000002  //dump schedule log while proc each job
#define KFLOW_AI_DBG_CTX          0x00000004  //dump context after proc each job
#define KFLOW_AI_DBG_OBUF         0x00000008  //dump buffer after proc each job

#define KFLOW_AI_DBG_TIME         0x00000100  //dump time after proc
#define KFLOW_AI_DBG_TIMELINE     0x00000200  //dump timeline after proc
#define KFLOW_AI_DBG_TIMELINE_ALL 0x00000400  //dump timeline after proc
#define KFLOW_AI_DBG_BW			  0x00001000	//dump bw after proc

#define KFLOW_AI_DBG_DDR		  0xff000000	//ddrid


/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/

extern BOOL _vendor_ai_is_init(VOID);
extern HD_RESULT _vendor_ai_validate(UINT32 proc_id);
extern HD_RESULT _vendor_ai_dump_engine (void);
extern VENDOR_AI_COMMON_INFO* _vendor_ai_common_info(void);
extern VENDOR_AI_PROC_STATE* _vendor_ai_state(UINT32 proc_id);
extern VENDOR_AI_NET_CFG_PROC* _vendor_ai_cfg(UINT32 proc_id);

extern HD_RESULT _vendor_ai_alloc_proc (UINT32 proc_id);
extern void _vendor_ai_free_proc (UINT32 proc_id);

extern VENDOR_AI_NET_INFO_PROC* _vendor_ai_info(UINT32 proc_id);
extern NN_GEN_MODEL_HEAD *_vendor_ai_net_get_head(UINT32 proc_id);
extern NN_GEN_MODE_CTRL *_vendor_ai_net_get_mctrl(UINT32 proc_id);
extern INT _vendor_ai_cfg_model(UINT32 proc_id, VENDOR_AI_NET_CFG_MODEL *cfg_model);
extern HD_RESULT vendor_ai_comm_lock(void);
extern HD_RESULT vendor_ai_comm_unlock(void);
extern UINT64 vendor_ai_comm_gettime_us(VOID);
extern HD_RESULT vendor_ai_check_input_align_set (UINT32 b_not_check);
extern HD_RESULT vendor_ai_check_output_align_set (UINT32 b_not_check);
#endif  /* _VENDOR_AI_COMM_H_ */
