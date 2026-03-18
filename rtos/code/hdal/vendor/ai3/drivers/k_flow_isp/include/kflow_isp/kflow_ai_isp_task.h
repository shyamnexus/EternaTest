/**
	@brief Source file of kflow_ai_isp_task.h.

	@file kflow_ai_isp_task.h

	@ingroup kflow_ai_isp

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/

#ifndef _KFLOW_AI_ISP_TASK_H_
#define _KFLOW_AI_ISP_TASK_H_

#include "kwrap/type.h"
#include "kwrap/platform.h"
#include "kwrap/task.h"
#include "kwrap/spinlock.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/error_no.h"

#include "kflow_ai_net/kflow_ai_net_list.h" //for LIST_HEAD

#define KFLOW_AI_ISP_TSK_PRI       3
#define KFLOW_AI_ISP_TSK_STKSIZE   4096
#define KFLOW_AI_ISP_TSK_JOBSIZE   256
extern THREAD_HANDLE _SECTION(".kercfg_data") KFLOW_AI_ISP_TSK_ID;
extern THREAD_DECLARE(kflow_ai_isp_tsk, arglist);
extern ID _SECTION(".kercfg_data") FLG_ID_AI_ISP;

typedef struct _KFLOW_ISP_TASK_FUNC {
	LIST_HEAD list;
	void* p_param;
	void (*p_exec)(void* p_param);
} KFLOW_ISP_TASK_FUNC;

typedef struct{ 
	UINT32 ipp_id;
	UINT32 proc_id;
	UINT32 path_id;
	UINT32 push_time;
	UINT32 ts_exec;
	KFLOW_ISP_TASK_FUNC deferred; 
}KFLOW_AI_ISP_JOB;

extern int kflow_ai_isp_tsk_get_cnt(void);
extern int kflow_ai_isp_tsk_reset(void);
extern int kflow_ai_isp_tsk_open(void);
extern int kflow_ai_isp_tsk_close(void);
extern void kflow_ai_isp_tsk_init_func(KFLOW_ISP_TASK_FUNC* p_func, void (*p_exec)(void* p_param), void* p_param);
extern void kflow_ai_isp_tsk_put_func(KFLOW_ISP_TASK_FUNC* p_func);

#endif //_KFLOW_AI_NET_ISP_TASK_H_
