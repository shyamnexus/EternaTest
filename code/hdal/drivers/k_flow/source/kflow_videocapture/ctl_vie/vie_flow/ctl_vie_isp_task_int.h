/**
    CTL VIE Layer

    @file       ctl_vie_isp_task_int.h
    @ingroup    mIIPL_Ctrl
    @note       None

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/


#ifndef _CTL_VIE_ISP_TASK_INT_H
#define _CTL_VIE_ISP_TASK_INT_H

#include "kwrap/list.h"
#include "ctl_vie_dbg.h"

/**********************************/
/*  ctl_vie_isp_message queue     */
/**********************************/
#define CTL_VIE_ISP_MSG_STS_FREE        0x00000000
#define CTL_VIE_ISP_MSG_STS_LOCK        0x00000001

#define CTL_VIE_ISP_MSG_IGNORE          0x00000000
#define CTL_VIE_ISP_MSG_CBEVT_ISP       0x00000001

typedef struct vos_list_head CTL_VIE_LIST_HEAD;
INT32 ctl_vie_isp_cb_proc(UINT32 id, UINT32 evt, UINT32 cb_fp, UINT32 frame_cnt);

typedef struct {
	UINT32 cmd;
	ULONG param[5];
	UINT32 rev[2];
	CTL_VIE_LIST_HEAD list;
} CTL_VIE_ISP_MSG_EVENT;

INT32 ctl_vie_isp_msg_snd(UINT32 cmd, ULONG p1, ULONG p2, ULONG p3, ULONG p4);
INT32 ctl_vie_isp_msg_rcv(UINT32 *cmd, ULONG *p1, ULONG *p2, ULONG *p3, ULONG *p4);
INT32 ctl_vie_isp_msg_flush(void);
INT32 ctl_vie_isp_erase_queue(UINT32 handle);
INT32 ctl_vie_isp_msg_reset_queue(void);
UINT32 ctl_vie_isp_get_free_queue_num(void);

/**********************************/
/*  ctl_vie_isp task ctrl         */
/**********************************/
INT32 ctl_vie_isp_open_tsk(void);
INT32 ctl_vie_isp_close_tsk(void);
INT32 ctl_vie_isp_task_set_resume(BOOL b_flush_evt);
INT32 ctl_vie_isp_task_set_pause(BOOL b_wait_end, BOOL b_flush_evt);
INT32 ctl_vie_isp_task_wait_pause_end(void);
void ctl_vie_isp_queue_flush(CTL_VIE_ID id);

#endif //_CTL_VIE_ISP_TASK_INT_H