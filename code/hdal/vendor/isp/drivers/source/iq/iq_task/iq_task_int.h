#ifndef _IQ_TASK_INT_H_
#define _IQ_TASK_INT_H_

#include "kwrap/semaphore.h"
#include "kwrap/task.h"
#include "kwrap/type.h"

#include "isp_api.h"

#include "iq_alg.h"
#include "iqt_api.h"

#define IQ_TASK_PRI 9

#define FLGPTN_INIT           FLGPTN_BIT(0)
#define FLGPTN_UNINIT         FLGPTN_BIT(1)
#define FLGPTN_PROC_SIE       FLGPTN_BIT(2)
#define FLGPTN_PROC_IPP       FLGPTN_BIT(3)
#define FLGPTN_PROC_ENC       FLGPTN_BIT(4)
#define FLGPTN_PAUSE          FLGPTN_BIT(5)
#define FLGPTN_RESUME         FLGPTN_BIT(6)
#define FLGPTN_STOP           FLGPTN_BIT(7)
#define FLGPTN_IDLE           FLGPTN_BIT(31)

/**
	IQ status
*/
typedef enum {
	IQ_TASK_STATUS_CLOSE   = 0,
	IQ_TASK_STATUS_OPEN    = 1,
	IQ_TASK_STATUS_START   = 2,
	IQ_TASK_STATUS_PAUSE   = 3,
	ENUM_DUMMY4WORD(IQ_TASK_STATUS)
} IQ_TASK_STATUS;

/**
	IQ event index
*/
typedef enum {
	EVENT_IDX_INIT,
	EVENT_IDX_UNINIT,
	EVENT_IDX_PAUSE,
	EVENT_IDX_RESUME,
	EVENT_IDX_MAX,
	ENUM_DUMMY4WORD(IQ_EVENT_IDX)
} IQ_EVENT_IDX;

typedef struct {
	THREAD_HANDLE task_id;
	ID            flag_id;
} IQ_TASK_OBJ;

/**
	iq event function pointer
*/
typedef INT32 (*IQ_EVENT_FP)(UINT32 id);
typedef INT32 (*IQ_EVENT_WITH_MSG_FP)(UINT32 id, ISP_TRIG_MSG msg, void *arg);

/**
	IQ event function
*/
typedef struct {
	IQ_EVENT_FP init;               ///< init event
	IQ_EVENT_FP uninit;             ///< uninit event
	IQ_EVENT_WITH_MSG_FP process;   ///< process event
	IQ_EVENT_FP pause;              ///< pause event
	IQ_EVENT_FP resume;             ///< resume event
} IQ_EVENT_OBJ;

typedef struct {
	BOOL id_used;
	IQ_ID id;
	IQ_TASK_STATUS task_status;
	IQ_EVENT_FP event_fp_tbl[EVENT_IDX_MAX];
	IQ_EVENT_WITH_MSG_FP event_with_msg_fp_tbl;
	IQ_TASK_OBJ *task_obj;
	ISP_IQ_TRIG_OBJ trig_obj;
} IQ_TASK_CTRL_OBJ;

/**
	Open IQ process
	@param[in] id          iq process id
	@param[in] obj         iq event object
	@return Description of data returned.
		ER: E_OK success
*/
extern ER iq_task_open(IQ_ID id, IQ_EVENT_OBJ *iq_event_tab);

/**
	Close IQ process
	@param[in] id          iq process id
	@return Description of data returned.
		ER: E_OK success
*/
extern ER iq_task_close(IQ_ID id);

/**
	Start IQ process
	@param[in] id           iq process id (IQ_ID)
	@param[in] wait_finish  wait start event finish
	@return Description of  data returned.
		ER: E_OK success
*/
extern ER iq_task_init(IQ_ID id);
extern ER iq_task_uninit(IQ_ID id);

/**
	trigger event, for IPL hook
	@param[in] id           iq process id (IQ_ID)
*/
extern void iq_task_trig(UINT32 id, void *arg);

/**
	Pause IQ process(for IPL control)
	@param[in] id           iq process id (IQ_ID)
	@param[in] wait_finish  wait pause event finish
	@return Description of  data returned.
		ER: E_OK success
*/
extern ER iq_task_pause(IQ_ID id);

/**
	Resume IQ process(for IPL control)
	@param[in] id          iq process id (IQPROC_ID)
	@return Description of data returned.
		ER: E_OK success
*/
extern ER iq_task_resume(IQ_ID id);

/**
	install IQ control id
*/
extern void iq_task_id_install(void);
extern void iq_task_id_uninstall(void);
extern THREAD_DECLARE(iq_tsk, arglist);
extern IQ_TASK_OBJ _SECTION(".kercfg_data") iq_task_obj[IQ_ID_MAX_NUM];
extern SEM_HANDLE  _SECTION(".kercfg_data") iq_semi_id;

#endif

