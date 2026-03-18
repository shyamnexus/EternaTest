/**
	@brief Source file of kflow_ai_net.

	@file kflow_ai_core.h

	@ingroup kflow_ai_net

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#ifndef _KFLOW_AI_CORE_H_
#define _KFLOW_AI_CORE_H_

#include "kwrap/type.h"
#include "kwrap/error_no.h"

#include "kflow_ai_net/kflow_ai_net_comm.h"
#include "kflow_ai_net/nn_net.h"
#include "kflow_ai_net/kflow_ai_net_list.h" //for LIST_HEAD
#include "kflow_ai_net/kflow_ai_core_task.h" //for KFLOW_CORE_TASK_FUNC

///#define KFLOW_AI_DBG_CORE         0x00000000    //dump current jobs in wait queue, ready queue and run of all engines  // removed : it's unused, and should NOT use "0" as mask value
#define KFLOW_AI_DBG_BIND         0x00000001    //dump net graph before net proc
#define KFLOW_AI_DBG_SCHD         0x00000002    //dump schedule log while proc each job
#define KFLOW_AI_DBG_CTX          0x00000004    //dump context after proc each job
#define KFLOW_AI_DBG_OBUF         0x00000008    //dump buffer after proc each job

#define KFLOW_AI_DBG_TIME         0x00000100    //dump time after proc
#define KFLOW_AI_DBG_TIMELINE     0x00000200 	//dump timeline after proc
#define KFLOW_AI_DBG_TIMELINE_ALL 0x00000400    //dump timeline after proc
#define KFLOW_AI_DBG_BW           0x00001000    //dump bw after proc

#define KFLOW_AI_DBG_DDR          0xff000000    //ddrid


typedef struct _KFLOW_AI_BIND KFLOW_AI_BIND;
typedef struct _KFLOW_AI_JOB KFLOW_AI_JOB;
typedef struct _KFLOW_AI_NET KFLOW_AI_NET;
typedef struct _KFLOW_AI_CHANNEL_CTX KFLOW_AI_CHANNEL_CTX;
typedef struct _KFLOW_AI_ENGINE_CTX KFLOW_AI_ENGINE_CTX;

/**
    KFLOW AI engine id
*/
typedef enum {
	KFLOW_AI_ENGINE_CONV     = 0, //DLA1
	KFLOW_AI_ENGINE_ROU      = 1, //DLA2
	KFLOW_AI_ENGINE_NUE2     = 2, //DLA3
	KFLOW_AI_ENGINE_LSU      = 3,
	KFLOW_AI_ENGINE_UTIL	 = 4,
	KFLOW_AI_ENGINE_CAL	     = 5,
	KFLOW_AI_ENGINE_PPU	     = 6,
	KFLOW_AI_ENGINE_JOBM	 = 7,
    KFLOW_AI_ENGINE_JMISP	 = 8,
    KFLOW_AI_ENGINE_POU     = 10,
	KFLOW_AI_ENGINE_DLA12   = 11,
    KFLOW_AI_ENGINE_DLA13   = 12,
	KFLOW_AI_ENGINE_CPU     = 13,
	KFLOW_AI_ENGINE_DSP     = 14,
    KFLOW_AI_ENGINE_HTE     = 15,
	KFLOW_AI_ENGINE_MAX_ID,
	ENUM_DUMMY4WORD(KFLOW_AI_ENGINE_ID)
} KFLOW_AI_ENGINE_ID;

typedef struct _KFLOW_AI_BIND {
	LIST_HEAD list; //belong to graph's freelist or job's parent-list or job's child-list
	struct _KFLOW_AI_JOB *p_job;
} KFLOW_AI_BIND;

typedef struct _KFLOW_AI_JOB {
	UINT32 proc_id;
	UINT32 job_id;
	//runtime state
	LIST_HEAD list;
	//graph input
	INT32 parent_cnt;
	LIST_HEAD parent_list;
	//graph output
	INT32 child_cnt;
	LIST_HEAD child_list;
	//graph node info
	void* p_op_info; // operation info
	void* p_io_info; // input/output info
#if (defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02401_))
	//llcmd buffer pa
	uintptr_t parm_addr;
#endif
	//runtime state
	UINT32 state; //0 = init, 1 = wait, 2 = ready, 3 = run, 4 = done, 5 = stuck by sw_reset;
	INT32 exec_cnt;
	INT32 wait_cnt; //after ready: current waiting resource (def: parent_cnt)
	UINT32 engine_op;
    union {
        UINT32 engine_id;
        UINT32 dispatcher;
    };
	UINT32 schd_parm; //for FIFO: (none), for FAIR: (channel_id), for CAPACITY: (QoS bandwidth)
	INT32 wait_ms; ///<-1: blocking, 0: non-blocking, >0: non-blocking + timeout
	struct _KFLOW_AI_ENGINE_CTX* p_eng; //assigned engine
	struct _KFLOW_AI_CHANNEL_CTX* p_ch; //assigned channel
	UINT32 ts_wait_begin; // hwclock_get_longcounter();
	UINT32 ts_wait_end;
	UINT32 ts_ready_begin;
	UINT32 ts_ready_end;
	UINT64 ts_exec_begin;
	UINT64 ts_exec_end;
	UINT32 ts_exec_predict;   	// hw ideal time (ms) = hw ideal cycle / div
	UINT32 ts_exec_ideal_cycle; // hw ideal cycle [tool provided] 
	UINT32 ts_exec;  			// sw real time (ms)= ts_exec_end - ts_exec_begin
	UINT32 ts_exec_cycle;  		// hw real cycle
#if (defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02401_))
    UINT32 ts_exec_macc_cycle;  // hw real cycle - hw wait dma cycle
#endif
#ifdef KFLOW_AI_VP_ENV
	UINT32 ub_bw;
	UINT32 dram_bw;
#endif
    UINT32 trig_src; //for trig
    UINT32 tot_trig_eng_times; //for trig
    UINT32 idea_cycle; //for getcfg
    UINT32 omem_cnt; //for dump out buf
    UINT32 job_priority;
	UINT32 debug_func;
	//runtime func
	UINT32 bw_exec_bytes;
	UINT32 bw_exec_predict;
    NN_DATA_V30 real_sao[3]; //for nue2 dump
	KFLOW_CORE_TASK_FUNC deferred;
} KFLOW_AI_JOB;

typedef struct _KFLOW_AI_KFLOW_AI_TIMELINE_JOB {
    UINT32 job_type; //0:sw_job, 1:hw_job, 2:sw_api, 3:hw_job for conv0_ll1
	UINT32 proc_id;
    union {
        UINT32 job_id;
        UINT32 class;
    };
    union {
        UINT32 dispatcher;
        UINT32 param;
    };
    union {
        UINT64 ts_exec_begin;
        UINT32 ts_exec_cycle;  		// hw real cycle
    };
	UINT64 ts_exec_end;
    CHAR* ch_name;
} KFLOW_AI_TIMELINE_JOB;

typedef struct _KFLOW_AI_JOB_CMD {
	UINT32 proc_id;
	UINT32 job_id;
} KFLOW_AI_JOB_CMD;

typedef struct _KFLOW_AI_NET {
	UINT32 proc_id;
	UINT32 max_job_cnt;
	UINT32 job_cnt;
	UINT32 sub_job_cnt;
	UINT32* map;
	UINT32* sub_map;
	UINT32 map_id;
	UINT32 sub_map_id;
	struct _KFLOW_AI_JOB* job;
	struct _KFLOW_AI_JOB* sub_job;
	LIST_HEAD free_job_list;
	UINT32 bind_cnt;
	struct _KFLOW_AI_BIND* bind;
	LIST_HEAD free_bind_list;
	UINT32 src_cnt;
	UINT32 dest_cnt;
	UINT32 total_cnt;
	UINT32 src_wait_cnt;
	UINT32 dest_wait_cnt;
	UINT32 exec_cnt;
	UINT32 isr_trig_cnt;
	UINT32 debug_func;
	UINT32 flow_mask;
	UINT32 proc_rec; //00:isr trigger, 01:task trigger
	UINT32 ts_proc_begin; // hwclock_get_longcounter();
	UINT32 ts_proc_end;
	UINT32 blk_id;
	UINT32 blk_sz;
	UINT32 pool;
	void* addr;
	INT32 rv; //0=ok, -1=FAILED, -2=TIMEOUT, -3=ABORT
	// perf total
	UINT32 ts_total_predict;
	UINT32 ts_total;
	UINT32 bw_total_predict;
	UINT32 bw_total;
    UINT32 real_job_id; //for debug dump
} KFLOW_AI_NET;

typedef struct _KFLOW_AI_UT {
	UINT32 ut_i, ut_f;
	UINT32 ut_hw_i, ut_hw_f;
#if (defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02401_))
	UINT32 ut_macc_i, ut_macc_f;
#endif
	UINT32 ut_ideal_i, ut_ideal_f;
} KFLOW_AI_UT;


typedef struct _KFLOW_AI_CHANNEL_CTX {
	char* name;
	LIST_HEAD* ready_queue;
	KFLOW_AI_JOB* run_job;
	UINT32 state;  //0: init, 1: open, 2: start, 3: error
	INT32 wait_ms; ///< last time
#if 1 //for LOAD_BALANCE
	UINT32 ts_ready_load;
	UINT32 ts_prev_end; //before idle, last job end
	UINT32 ts_this_begin; //after busy, first job begin
#endif
	UINT64 ts_perf_begin;
	UINT64 ts_perf_end;
	UINT64 ts_exec;
	UINT64 ts_exec_hw;
#ifdef KFLOW_AI_VP_ENV
	UINT64 ub_bw;
	UINT64 dram_bw;
#endif
#if (defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02401_))
	UINT64 ts_exec_macc;
#endif	
	UINT64 ts_exec_ideal;
	UINT64 lts_exec_all;
	UINT64 lts_exec;
    KFLOW_AI_UT last_ut;
	void (*open)(struct _KFLOW_AI_CHANNEL_CTX* p_ch);
	void (*close)(struct _KFLOW_AI_CHANNEL_CTX* p_ch);
#if NN_DLI
	void (*start)(struct _KFLOW_AI_CHANNEL_CTX* p_ch, KFLOW_AI_JOB* p_job); //start job
	void (*stop)(struct _KFLOW_AI_CHANNEL_CTX* p_ch, KFLOW_AI_JOB* p_job); //stop job
#endif
	void (*trigger)(struct _KFLOW_AI_CHANNEL_CTX* p_ch, KFLOW_AI_JOB* p_job); //proc job - begin
#if (defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02401_))
#ifdef KFLOW_AI_VP_ENV
	void (*onfinish)(struct _KFLOW_AI_CHANNEL_CTX* p_ch, KFLOW_AI_JOB* p_job, UINT32 cycle, UINT32 ub_bw, UINT32 dram_bw);
#else
	void (*onfinish)(struct _KFLOW_AI_CHANNEL_CTX* p_ch, KFLOW_AI_JOB* p_job, UINT32 cycle, UINT32 macc);
#endif
#else
	void (*onfinish)(struct _KFLOW_AI_CHANNEL_CTX* p_ch, KFLOW_AI_JOB* p_job, UINT32 cycle); //proc job - end
#endif	
	void (*reset)(struct _KFLOW_AI_CHANNEL_CTX* p_ch); // force abort while ctrl-c reset flow
	void (*reset2)(struct _KFLOW_AI_CHANNEL_CTX* p_ch); // force hw-reset while time-out
	void (*debug)(struct _KFLOW_AI_CHANNEL_CTX* p_ch, KFLOW_AI_JOB* p_job, UINT32 info);
} KFLOW_AI_CHANNEL_CTX;

typedef struct _KFLOW_AI_ENGINE_CTX {
	char* name;
	LIST_HEAD* wait_queue;
	UINT32 state;  //0: n/a, 1: init
	UINT32 channel_max; //user config max count
	UINT32 channel_count;
	KFLOW_AI_CHANNEL_CTX** p_ch;
	UINT32 attr; //0: fast, 1: slow
	KFLOW_AI_ENGINE_CTX* (*geteng)(KFLOW_AI_ENGINE_ID engine_id);
	ER (*init)(struct _KFLOW_AI_ENGINE_CTX* p_eng);
	void (*uninit)(struct _KFLOW_AI_ENGINE_CTX* p_eng);
	UINT32 (*getcfg)(struct _KFLOW_AI_ENGINE_CTX* p_eng, KFLOW_AI_JOB* p_job, UINT32 cfg);
} KFLOW_AI_ENGINE_CTX;

#define CORE_MAX   32

typedef struct _KFLOW_AI_CORE_UT {
    CHAR   name[8];  ///< name
    UINT32 time;     ///< time
    UINT32 util;     ///< utility
} KFLOW_AI_CORE_UT;

typedef struct _KFLOW_AI_PERF_UT {
    UINT32 core_count;
    KFLOW_AI_CORE_UT core[CORE_MAX];
} KFLOW_AI_PERF_UT;

//init/uninit
extern void kflow_ai_core_reset_engine(void);
extern void kflow_ai_core_add_engine(KFLOW_AI_ENGINE_ID engine_id, KFLOW_AI_ENGINE_CTX* p_eng);
extern UINT32 kflow_ai_core_get_engine_cnt(void);
extern KFLOW_AI_ENGINE_CTX* kflow_ai_core_get_engine(KFLOW_AI_ENGINE_ID engine_id);
extern UINT32 kflow_ai_core_get_channel_cnt(KFLOW_AI_ENGINE_CTX* p_eng);
extern KFLOW_AI_CHANNEL_CTX* kflow_ai_core_get_channel(KFLOW_AI_ENGINE_CTX* p_eng, UINT32 channel_id);
extern void kflow_ai_core_reset(void);
extern void kflow_ai_core_reset_path(UINT32 net_id);
extern ER kflow_ai_core_init(void);
extern ER kflow_ai_core_uninit(UINT32 is_reset);
extern void kflow_ai_sw_reset (void);
extern void kflow_ai_core_cfgschd(UINT32 schd);
extern void kflow_ai_core_cfgchk(UINT32 chk_interval);

//net
extern KFLOW_AI_NET* kflow_ai_core_net(UINT32 proc_id);
extern void kflow_ai_net_create(KFLOW_AI_NET* p_net, UINT32 max_job_cnt, UINT32 job_cnt, UINT32 bind_cnt, UINT32 ddr_id);
extern void kflow_ai_net_destory(KFLOW_AI_NET* p_net);
extern void kflow_ai_net_create_sub(KFLOW_AI_NET* p_net, UINT32 sub_job_cnt);
extern void kflow_ai_net_destory_sub(KFLOW_AI_NET* p_net);
extern KFLOW_AI_JOB* kflow_ai_net_add_job(KFLOW_AI_NET* p_net, UINT32 job_id);
extern KFLOW_AI_JOB* kflow_ai_net_add_sub_job(KFLOW_AI_NET* p_net, UINT32 sub_job_id);
extern KFLOW_AI_JOB* kflow_ai_net_job(KFLOW_AI_NET* p_net, UINT32 job_id);
extern KFLOW_AI_JOB* kflow_ai_net_sub_job(KFLOW_AI_NET* p_net, UINT32 job_id, uintptr_t parm_addr, UINT32 mode);
extern UINT32 kflow_ai_net_job_id(KFLOW_AI_NET* p_net, KFLOW_AI_JOB* p_job);
extern void kflow_ai_net_dump(KFLOW_AI_NET* p_net, UINT32 info);
extern void kflow_ai_net_set_debug(UINT32 proc_id, UINT32 func, UINT32 is_expand);
extern void kflow_ai_net_set_debug_all(UINT32 func, UINT32 is_expand);
extern void kflow_ai_net_get_debug(UINT32 proc_id, UINT32 *func);
extern void kflow_ai_net_get_debug_all(UINT32 *func);
extern void kflow_ai_net_set_timeline_func_mask(UINT32 func_mask);
extern void kflow_ai_net_flow(UINT32 mask);

//dump
extern void kflow_ai_engine_dump(KFLOW_AI_ENGINE_CTX* p_eng, UINT32 info);
extern void kflow_ai_core_dump(void);
extern void kflow_ai_reg_dump(void);
extern void kflow_ai_mem_dump(UINT32 mask);
extern void kflow_ai_perf(UINT32 func, UINT32 sample);
extern void kflow_ai_cat_perf(UINT32 func, int (*dump)(const char *fmt, ...));
#ifdef KFLOW_AI_VP_ENV
extern void kflow_ai_cycle_bw_dump(void);
#endif
//clr/set job
extern void kflow_ai_core_clr_job(KFLOW_AI_NET* p_net, KFLOW_AI_JOB* p_job);
//extern void kflow_ai_core_set_job(KFLOW_AI_NET* p_net, KFLOW_AI_JOB* p_job, UINT32 engine_id, UINT32 engine_op, void* p_op_info, void* p_io_info, INT32 wait_ms);
extern void kflow_ai_core_set_job(KFLOW_AI_NET* p_net, KFLOW_AI_JOB* p_job, void* p_op_info, void* p_io_info, INT32 wait_ms);
//clr bind
extern void kflow_ai_core_clr_bind(KFLOW_AI_NET* p_net, KFLOW_AI_BIND* p_bind);
//bind/unbind job
extern void kflow_ai_core_bind_job(KFLOW_AI_NET* p_net, KFLOW_AI_JOB* p_job, KFLOW_AI_JOB* p_next_job);
extern void kflow_ai_core_sum_job(KFLOW_AI_NET* p_net, uintptr_t* src_count, uintptr_t* dest_count);
//push job (cannot modify graph)
extern void kflow_ai_core_push_begin(KFLOW_AI_NET* p_net);
extern void kflow_ai_core_push_end(KFLOW_AI_NET* p_net);
extern void kflow_ai_core_push_job(KFLOW_AI_NET* p_net, KFLOW_AI_JOB* p_job, UINT32 schd_parm, UINT32 job_priority);
extern INT32 kflow_ai_core_lock_job(KFLOW_AI_NET* p_net, KFLOW_AI_JOB* p_job);
extern INT32 kflow_ai_core_unlock_job(KFLOW_AI_NET* p_net, KFLOW_AI_JOB* p_job);
//pull job
extern void kflow_ai_core_pull_begin(KFLOW_AI_NET* p_net);
extern void kflow_ai_core_pull_end(KFLOW_AI_NET* p_net);
extern void kflow_ai_core_pull_ready(KFLOW_AI_NET* p_net, KFLOW_AI_JOB* p_job);
extern INT32 kflow_ai_core_pull_job(KFLOW_AI_NET* p_net, KFLOW_AI_JOB** p_job);

//consume job (cannot modify graph)
extern void kflow_ai_core_run_job(KFLOW_AI_CHANNEL_CTX* p_ch, KFLOW_AI_JOB* p_job);
#if NN_DLI
extern void kflow_ai_core_start_job(KFLOW_AI_CHANNEL_CTX* p_ch, KFLOW_AI_JOB* p_job);
extern void kflow_ai_core_stop_job(KFLOW_AI_CHANNEL_CTX* p_ch, KFLOW_AI_JOB* p_job);
#endif
extern void kflow_ai_core_trig_job(KFLOW_AI_CHANNEL_CTX* p_ch, KFLOW_AI_JOB* p_job);
#if (defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02401_))
#ifdef KFLOW_AI_VP_ENV
extern void kflow_ai_core_onfinish_job(KFLOW_AI_CHANNEL_CTX* p_ch, KFLOW_AI_JOB* p_job, UINT32 cycle, UINT32 ub_bw, UINT32 dram_bw);
#else
extern void kflow_ai_core_onfinish_job(KFLOW_AI_CHANNEL_CTX* p_ch, KFLOW_AI_JOB* p_job, UINT32 cycle, UINT32 macc);
#endif
#else
extern void kflow_ai_core_onfinish_job(KFLOW_AI_CHANNEL_CTX* p_ch, KFLOW_AI_JOB* p_job, UINT32 cycle);
#endif
extern void kflow_ai_core_dump_job(UINT32 info, UINT32 act, KFLOW_AI_JOB* p_job);


extern void kflow_ai_core_get_perf_ut(KFLOW_AI_PERF_UT* p_perf_ut);

extern void kflow_ai_reset_net_path(UINT32 proc_id);

extern void kflow_ai_core_fix_job_parm_addr(UINT32 proc_id, UINT32 job_id, uintptr_t parm_addr);
extern void kflow_ai_core_fix_sub_job_parm_addr(UINT32 proc_id, UINT32 job_id, uintptr_t parm_addr);
extern void kflow_ai_core_fix_tot_trig_eng_times(UINT32 proc_id, UINT32 job_id, UINT32 tot_trig_eng_times);
extern void kflow_ai_core_fix_out_info(UINT32 proc_id, UINT32 job_id, NN_DATA_V30* p_sao);

extern ER kflow_ai_core_perf_timeline_begin(void);
extern ER kflow_ai_core_perf_timeline_end(void);
extern ER kflow_ai_core_set_timeline_job_num(UINT32 size);
extern ER kflow_ai_core_set_user_timeline_job (UINT32 proc_id, UINT32 class, UINT32 *param, UINT32 ts_start, UINT32 ts_end);
extern void kflow_ai_core_set_usage_limit(UINT32 usage_limit) ; 
extern int kflow_core_set_timeline_path(char *path);
#endif //_KFLOW_AI_CORE_H_
