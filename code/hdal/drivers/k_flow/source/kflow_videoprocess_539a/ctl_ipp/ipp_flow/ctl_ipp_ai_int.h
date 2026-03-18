/**
    IPL Ctrl Layer

    @file       ctl_ipp_ai_int.h
    @ingroup    mIIPL_Ctrl
    @note       None

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/


#ifndef _CTL_IPP_AI_INT_H
#define _CTL_IPP_AI_INT_H

#include "ctl_ipp_util_int.h"
#include "ctl_ipp.h"

#if defined(CONFIG_NVT_FPGA_EMULATION) || defined(_NVT_FPGA_)
#define CTL_IPP_AI_HANDLE_LOCK_TIMEOUT_MS	(5000)
#else
#define CTL_IPP_AI_HANDLE_LOCK_TIMEOUT_MS	(1000)
#endif

/**********************************/
/*	ctl_ipp_ai message queue	  */
/**********************************/
#define CTL_IPP_AI_MSG_STS_FREE		0x00000000
#define CTL_IPP_AI_MSG_STS_LOCK		0x00000001

#define CTL_IPP_AI_MSG_IGNORE		0x00000000
#define CTL_IPP_AI_MSG_PROCESS		0x00000001  //[0]: handle, [1]: header address, [2]: buf_id(for unit release buffer)
#define CTL_IPP_AI_MSG_DROP			0x00000002  //[0]: handle, [1]: header address, [2]: buf_id(for unit release buffer), this cmd is for erase queue used

typedef struct {
	UINT32 cmd;
	ULONG param[5];
	UINT32 rev[2];
	CTL_IPP_LIST_HEAD list;
} CTL_IPP_AI_MSG_EVENT;

ER ctl_ipp_ai_msg_snd(UINT32 cmd, ULONG p1, ULONG p2, ULONG p3);
ER ctl_ipp_ai_msg_rcv(UINT32 *cmd, ULONG *p1, ULONG *p2, ULONG *p3);
ER ctl_ipp_ai_msg_flush(void);
ER ctl_ipp_ai_erase_queue(CTL_IPP_HANDLE *p_hdl);
ER ctl_ipp_ai_msg_reset_queue(void);
UINT32 ctl_ipp_ai_get_free_queue_num(void);

INT32 ctl_ipp_ai_process_d2d(CTL_IPP_HANDLE *p_hdl, CTL_IPP_INFO_LIST_ITEM *p_ctrl_info);
void ctl_ipp_ai_drop(CTL_IPP_HANDLE *p_hdl, CTL_IPP_INFO_LIST_ITEM *p_ctrl_info);
void ctl_ipp_ai_task_dumpinfo(int (*dump)(const char *fmt, ...));

ER ctl_ipp_ai_open_tsk(void);
ER ctl_ipp_ai_close_tsk(void);
ER ctl_ipp_ai_set_resume(BOOL b_flush_evt);
ER ctl_ipp_ai_set_pause(BOOL b_wait_end, BOOL b_flush_evt);
ER ctl_ipp_ai_wait_pause_end(void);
ER ctl_ipp_ai_wait_process_end(void);

#endif //_CTL_IPP_AI_INT_H
