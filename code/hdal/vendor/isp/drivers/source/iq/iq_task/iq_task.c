#if defined(__FREERTOS)
#include "string.h"
#endif
#include "kwrap/flag.h"
#include "kwrap/semaphore.h"
#include "kwrap/task.h"
#include "kwrap/type.h"

#include "iq_dbg.h"
#include "iq_task_int.h"

#define IQ_SEM_WAIT (SEM_WAIT(iq_semi_id))
#define IQ_SEM_SIG  (SEM_SIGNAL(iq_semi_id))

static IQ_TASK_CTRL_OBJ iq_task_ctrl_obj[IQ_ID_MAX_NUM];

//=============================================================================
// function declaration
//=============================================================================
static void iq_task_wait_idle(IQ_ID id, BOOL clr_flag);
static void iq_task_proc_event(IQ_TASK_CTRL_OBJ *ctrl, IQ_EVENT_IDX event);
static void iq_task_proc_event_with_msg(ISP_TRIG_MSG msg, IQ_TASK_CTRL_OBJ *ctrl);
//static IQ_TASK_CTRL_OBJ *iq_get_ctrl_obj_without_id(void);
static IQ_TASK_CTRL_OBJ *iq_task_get_ctrl_obj(IQ_ID id);

ER iq_task_open(IQ_ID id, IQ_EVENT_OBJ *iq_event_tab)
{
	IQ_TASK_CTRL_OBJ *ctrl;
	ER rt = E_OK;

	if (iq_event_tab == NULL) {
		DBG_ERR("iq_event_tab NULL (%d) \r\n", id);
		return E_SYS;
	}

	if (id >= IQ_ID_MAX_NUM) {
		DBG_ERR("wrong IQ ID (%d)!! \r\n", id);
		return E_SYS;
	}

	ctrl = iq_task_get_ctrl_obj(id);
	if (ctrl == NULL) {
		DBG_ERR("iq_task_get_ctrl_obj NULL (%d) \r\n", id);
		return E_SYS;
	}
	if (ctrl->task_obj != NULL) {
		DBG_ERR("iq ctrl task already opened (%d) \r\n", id);
		return E_SYS;
	}

	ctrl->task_obj = &iq_task_obj[id];
	ctrl->id = id;
	if (ctrl->task_status == IQ_TASK_STATUS_CLOSE) {
		ctrl->task_status =                    IQ_TASK_STATUS_OPEN;
		ctrl->id_used =                        TRUE;
		ctrl->event_fp_tbl[EVENT_IDX_INIT] =   iq_event_tab->init;
		ctrl->event_fp_tbl[EVENT_IDX_UNINIT] = iq_event_tab->uninit;
		ctrl->event_with_msg_fp_tbl        =   iq_event_tab->process;
		ctrl->event_fp_tbl[EVENT_IDX_PAUSE] =  iq_event_tab->pause;
		ctrl->event_fp_tbl[EVENT_IDX_RESUME] = iq_event_tab->resume;

		clr_flg(ctrl->task_obj->flag_id, FLGPTN_BIT_ALL);
		THREAD_CREATE(ctrl->task_obj->task_id, iq_tsk, NULL, "iq_tsk");
		if (ctrl->task_obj->task_id == 0) {
			DBG_ERR("THREAD_CREATE fail (%ld) \r\n", (ULONG)ctrl->task_obj->task_id);
			return E_SYS;
		}
		THREAD_SET_PRIORITY(ctrl->task_obj->task_id, IQ_TASK_PRI);
		THREAD_RESUME(ctrl->task_obj->task_id);

		rt = E_OK;
	} else {
		DBG_ERR("ctrl status fail %d\r\n", ctrl->task_status);
		rt = E_SYS;
	}

	return rt;
}

ER iq_task_close(IQ_ID id)
{
	IQ_TASK_CTRL_OBJ *ctrl;
	ER rt = E_OK;

	ctrl = iq_task_get_ctrl_obj(id);
	if (ctrl == NULL) {
		DBG_ERR("iq_task_get_ctrl_obj NULL (%d) \r\n", id);
		return E_SYS;
	}
	if (ctrl->task_obj == NULL) {
		DBG_ERR("iq ctrl task NULL (%d) \r\n", id);
		return E_SYS;
	}

	if ((ctrl->task_status == IQ_TASK_STATUS_START) || (ctrl->task_status == IQ_TASK_STATUS_PAUSE)) {
		iq_task_wait_idle(id, TRUE);
		ctrl->task_status = IQ_TASK_STATUS_OPEN;
		set_flg(ctrl->task_obj->flag_id, FLGPTN_UNINIT);
	}

	if (ctrl->task_status == IQ_TASK_STATUS_OPEN) {
		iq_task_wait_idle(id, TRUE);
		set_flg(ctrl->task_obj->flag_id, FLGPTN_STOP);
		THREAD_DESTROY(ctrl->task_obj->task_id);

		ctrl->task_status =                    IQ_TASK_STATUS_CLOSE;
		ctrl->event_fp_tbl[EVENT_IDX_INIT] =   NULL;
		ctrl->event_fp_tbl[EVENT_IDX_UNINIT] = NULL;
		ctrl->event_with_msg_fp_tbl =          NULL;
		ctrl->event_fp_tbl[EVENT_IDX_PAUSE] =  NULL;
		ctrl->event_fp_tbl[EVENT_IDX_RESUME] = NULL;
		ctrl->task_obj = NULL;
		rt = E_OK;
	} else {
		DBG_ERR("ctrl status fail %d\r\n", ctrl->task_status);
		rt = E_SYS;
	}

	return rt;
}

ER iq_task_init(IQ_ID id)
{
	IQ_TASK_CTRL_OBJ *ctrl;
	ER rt = E_OK;

	ctrl = iq_task_get_ctrl_obj(id);
	if (ctrl == NULL) {
		DBG_ERR("iq_task_get_ctrl_obj NULL (%d) \r\n", id);
		return E_SYS;
	}
	if (ctrl->task_obj == NULL) {
		DBG_ERR("iq ctrl task NULL (%d) \r\n", id);
		return E_SYS;
	}

	if (ctrl->task_status == IQ_TASK_STATUS_OPEN) {
		iq_task_wait_idle(id, TRUE);
		ctrl->task_status = IQ_TASK_STATUS_START;
		set_flg(ctrl->task_obj->flag_id, FLGPTN_INIT);
		iq_task_wait_idle(id, FALSE);
		rt = E_OK;
	} else {
		DBG_ERR("ctrl status fail %d\r\n", ctrl->task_status);
		rt = E_SYS;
	}

	return rt;
}

ER iq_task_uninit(IQ_ID id)
{
	IQ_TASK_CTRL_OBJ *ctrl;
	ER rt = E_OK;

	ctrl = iq_task_get_ctrl_obj(id);
	if (ctrl == NULL) {
		DBG_ERR("iq_task_get_ctrl_obj NULL (%d) \r\n", id);
		return E_SYS;
	}
	if (ctrl->task_obj == NULL) {
		DBG_ERR("iq ctrl task NULL (%d) \r\n", id);
		return E_SYS;
	}

	if (ctrl->task_status == IQ_TASK_STATUS_START) {
		iq_task_wait_idle(id, TRUE);
		ctrl->task_status = IQ_TASK_STATUS_OPEN;
		set_flg(ctrl->task_obj->flag_id, FLGPTN_UNINIT);
		rt = E_OK;
	} else {
		DBG_ERR("ctrl status fail %d\r\n", ctrl->task_status);
		rt = E_SYS;
	}

	return rt;
}

void iq_task_trig(UINT32 id, void *arg)
{
	IQ_TASK_CTRL_OBJ *ctrl;
	ISP_IQ_TRIG_OBJ *trig_obj = (ISP_IQ_TRIG_OBJ *)arg;

	if (id >= IQ_ID_MAX_NUM) {
		DBG_ERR("wrong IQ ID (%d)!! \r\n", id);
		return ;
	}

	ctrl = iq_task_get_ctrl_obj(id);
	if (ctrl == NULL) {
		DBG_ERR("iq_task_get_ctrl_obj NULL (%d) \r\n", id);
		return;
	}
	if (ctrl->task_obj == NULL) {
		DBG_ERR("iq ctrl task NULL (%d) \r\n", id);
		return;
	}

	if ((ctrl->task_status == IQ_TASK_STATUS_START) || (ctrl->task_status == IQ_TASK_STATUS_PAUSE)) {
		ctrl->id = id;
		switch (trig_obj->msg) {
		case ISP_TRIG_IQ_SIE:
			memcpy(&ctrl->trig_obj.sie, &trig_obj->sie, sizeof(ISP_IQ_SIE_TRIG_OBJ));
			clr_flg(ctrl->task_obj->flag_id, FLGPTN_IDLE);
			set_flg(ctrl->task_obj->flag_id, FLGPTN_PROC_SIE);
			break;
		case ISP_TRIG_IQ_SIE_IMM:
			memcpy(&ctrl->trig_obj.sie, &trig_obj->sie, sizeof(ISP_IQ_SIE_TRIG_OBJ));
			iq_task_proc_event_with_msg(ISP_TRIG_IQ_SIE, ctrl);
			break;
		case ISP_TRIG_IQ_IPP:
			memcpy(&ctrl->trig_obj.ipp, &trig_obj->ipp, sizeof(ISP_IQ_IPP_TRIG_OBJ));
			clr_flg(ctrl->task_obj->flag_id, FLGPTN_IDLE);
			set_flg(ctrl->task_obj->flag_id, FLGPTN_PROC_IPP);
			break;
		case ISP_TRIG_IQ_IPP_IMM:
			memcpy(&ctrl->trig_obj.ipp, &trig_obj->ipp, sizeof(ISP_IQ_IPP_TRIG_OBJ));
			iq_task_proc_event_with_msg(ISP_TRIG_IQ_IPP, ctrl);
			break;
		case ISP_TRIG_IQ_ENC:
			memcpy(&ctrl->trig_obj.enc, &trig_obj->enc, sizeof(ISP_IQ_ENC_TRIG_OBJ));
			clr_flg(ctrl->task_obj->flag_id, FLGPTN_IDLE);
			set_flg(ctrl->task_obj->flag_id, FLGPTN_PROC_ENC);
			break;
		case ISP_TRIG_IQ_ENC_IMM:
			memcpy(&ctrl->trig_obj.enc, &trig_obj->enc, sizeof(ISP_IQ_ENC_TRIG_OBJ));
			iq_task_proc_event_with_msg(ISP_TRIG_IQ_ENC, ctrl);
			break;
		default:
			DBG_ERR("ctrl->trig_obj.msg fail %d!!\r\n", ctrl->trig_obj.msg);
			break;
		}
	}
}

ER iq_task_pause(IQ_ID id)
{
	IQ_TASK_CTRL_OBJ *ctrl;
	ER rt = E_OK;

	if (id >= IQ_ID_MAX_NUM) {
		DBG_ERR("wrong IQ ID (%d)!! \r\n", id);
		return E_SYS;
	}

	ctrl = iq_task_get_ctrl_obj(id);
	if (ctrl == NULL) {
		DBG_ERR("iq_task_get_ctrl_obj NULL (%d)  \r\n", id);
		return E_SYS;
	}
		if (ctrl->task_obj == NULL) {
		DBG_ERR("iq ctrl task NULL (%d) \r\n", id);
		return E_SYS;
	}

	if (ctrl->task_status == IQ_TASK_STATUS_START) {
		ctrl->task_status = IQ_TASK_STATUS_PAUSE;
		clr_flg(ctrl->task_obj->flag_id, FLGPTN_IDLE);
		set_flg(ctrl->task_obj->flag_id, FLGPTN_PAUSE);
		rt = E_OK;
	} else {
		DBG_ERR("status fail %d!!\r\n", ctrl->task_status);
		rt = E_SYS;
	}

	return rt;
}

ER iq_task_resume(IQ_ID id)
{
	IQ_TASK_CTRL_OBJ *ctrl;
	ER rt = E_OK;

	if (id >= IQ_ID_MAX_NUM) {
		DBG_ERR("wrong IQ ID (%d)!! \r\n", id);
		return E_SYS;
	}

	ctrl = iq_task_get_ctrl_obj(id);
	if (ctrl == NULL) {
		DBG_ERR("iq_task_get_ctrl_obj NULL (%d) \r\n", id);
		return E_SYS;
	}
	if (ctrl->task_obj == NULL) {
		DBG_ERR("iq ctrl task NULL (%d) \r\n", id);
		return E_SYS;
	}

	if (ctrl->task_status == IQ_TASK_STATUS_PAUSE) {
		ctrl->task_status = IQ_TASK_STATUS_START;
		clr_flg(ctrl->task_obj->flag_id, FLGPTN_IDLE);
		set_flg(ctrl->task_obj->flag_id, FLGPTN_RESUME);
		rt = E_OK;
	} else {
		DBG_ERR("status fail %d!!\r\n", ctrl->task_status);
		rt = E_SYS;
	}

	return rt;
}

static void iq_task_wait_idle(IQ_ID id, BOOL clr_flag)
{
	FLGPTN flag;
	IQ_TASK_CTRL_OBJ *ctrl;

	ctrl = iq_task_get_ctrl_obj(id);
	if (ctrl == NULL) {
		DBG_ERR("iq_task_get_ctrl_obj(id = %d) NULL\r\n", id);
		return;
	}

	if (clr_flag) {
		wai_flg(&flag, ctrl->task_obj->flag_id, FLGPTN_IDLE, TWF_ORW | TWF_CLR);
	} else {
		wai_flg(&flag, ctrl->task_obj->flag_id, FLGPTN_IDLE, TWF_ORW);
	}
}

THREAD_DECLARE(iq_tsk, arglist)
{
	FLGPTN flag = FLGPTN_IDLE;
	IQ_TASK_CTRL_OBJ *ctrl;
	UINT32 trig = 1;
	UINT32 i = 0;

	THREAD_ENTRY();
	for (i = 0; i < IQ_ID_MAX_NUM; i++) {
		ctrl = iq_task_get_ctrl_obj(i);
		if (ctrl->id_used == TRUE) {
			ctrl->id_used = FALSE;
			break;
		}
	}

	//enter proc status
	while (trig) {
		set_flg(ctrl->task_obj->flag_id, FLGPTN_IDLE);
		PROFILE_TASK_IDLE();
		wai_flg(&flag, ctrl->task_obj->flag_id, FLGPTN_INIT | FLGPTN_UNINIT | FLGPTN_PROC_SIE | FLGPTN_PROC_IPP | FLGPTN_PROC_ENC | FLGPTN_PAUSE | FLGPTN_RESUME | FLGPTN_STOP, TWF_ORW | TWF_CLR);
		PROFILE_TASK_BUSY();

		if (flag & FLGPTN_INIT) {
			//DBG_MSG("IQ_INIT \n");
			iq_task_proc_event(ctrl, EVENT_IDX_INIT);
		}

		if (flag & FLGPTN_UNINIT) {
			iq_task_proc_event(ctrl, EVENT_IDX_UNINIT);
			//DBG_MSG("IQ_UNINIT %d \n", ctrl->id);
		}

		if (flag & FLGPTN_RESUME) {
			DBG_MSG("IQ_RESUME \n");
			iq_task_proc_event(ctrl, EVENT_IDX_RESUME);
		}

		if (flag & FLGPTN_PAUSE) {
			DBG_MSG("IQ_PAUSE \n");
			iq_task_proc_event(ctrl, EVENT_IDX_PAUSE);
		}

		if (flag & FLGPTN_PROC_SIE) {
			iq_task_proc_event_with_msg(ISP_TRIG_IQ_SIE, ctrl);
		}

		if (flag & FLGPTN_PROC_IPP) {
			iq_task_proc_event_with_msg(ISP_TRIG_IQ_IPP, ctrl);
		}

		if (flag & FLGPTN_PROC_ENC) {
			iq_task_proc_event_with_msg(ISP_TRIG_IQ_ENC, ctrl);
		}

		if (flag & FLGPTN_STOP) {
			DBG_MSG("IQ_STOP %d \n", ctrl->id);
			trig = 0;
		}
	}

	#if defined(__KERNEL__)
	return 0;
	#endif
}

static void iq_task_proc_event(IQ_TASK_CTRL_OBJ *ctrl, IQ_EVENT_IDX event)
{
	if (ctrl->event_fp_tbl[event] != NULL) {
		ctrl->event_fp_tbl[event](ctrl->id);
	}
}

static void iq_task_proc_event_with_msg(ISP_TRIG_MSG msg, IQ_TASK_CTRL_OBJ *ctrl)
{
	if (ctrl->event_with_msg_fp_tbl != NULL) {
		ctrl->event_with_msg_fp_tbl(ctrl->id, msg, &ctrl->trig_obj);
	}
}

#if 0
static IQ_TASK_CTRL_OBJ *iq_get_ctrl_obj_without_id(void)
{
	UINT32 i;
	ID task_id;
	IQ_TASK_CTRL_OBJ *ctrl;

	get_tid(&task_id);
	for (i = 0; i < IQ_ID_MAX_NUM; i++) {
		ctrl = iq_task_get_ctrl_obj(i);

		if (ctrl->task_obj != NULL) {
			if (ctrl->task_obj->task_id == (UINT32)task_id) {
				return ctrl;
			}
		}
	}
	return NULL;
}
#endif

static IQ_TASK_CTRL_OBJ *iq_task_get_ctrl_obj(IQ_ID id)
{
	if (id >= IQ_ID_MAX_NUM) {
		DBG_ERR("input fail %d\r\n", id);
		return NULL;
	}
	return &iq_task_ctrl_obj[id];
}
