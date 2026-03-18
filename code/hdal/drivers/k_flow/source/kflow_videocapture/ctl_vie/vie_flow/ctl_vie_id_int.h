/**
    Ctl VIE Layer

    .

    @file       ctl_vie_id_int.h
    @ingroup    mISYSAlg
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/
#ifndef _CTL_VIE_ID_INT_H_
#define _CTL_VIE_ID_INT_H_

#if defined (__LINUX)
#include <linux/string.h>
#endif
#include "kwrap/flag.h"
#include "kwrap/task.h"
#include "ctl_vie_int.h"
#include "ctl_vie_utility_int.h"

/* define task priority/stack size */
#define CTL_VIE_BUF_TSK_PRI       2
#define CTL_VIE_BUF_TSK_STKSIZE   1024

#define CTL_VIE_ISP_TSK_PRI       3
#define CTL_VIE_ISP_TSK_STKSIZE   1024

// ==================================================================
// ctl vie flag
// ==================================================================
#define CTL_VIE_FLG_LOCK            FLGPTN_BIT(0)
#define CTL_VIE_FLG_END             FLGPTN_BIT(1)
#define CTL_VIE_FLG_CNT             FLGPTN_BIT(2)
#define CTL_VIE_FLG_VD              FLGPTN_BIT(3)
#define CTL_VIE_FLG_TIMEOUT         FLGPTN_BIT(4)
//#define CTL_VIE_FLG_CH_SEN_MODE     FLGPTN_BIT(5)
#define CTL_VIE_FLG_KDRV_END        FLGPTN_BIT(6)
#define CTL_VIE_FLG_ISR_PRC_END     FLGPTN_BIT(7)
#define CTL_VIE_FLG_VD_DBG          FLGPTN_BIT(8)
#define CTL_VIE_FLG_CLOSE           FLGPTN_BIT(9)

#define CTL_VIE_FLG_INIT        (CTL_VIE_FLG_LOCK|CTL_VIE_FLG_END|CTL_VIE_FLG_CNT|CTL_VIE_FLG_ISR_PRC_END)

/**********************************************************/
/*              ctl_vie buf task flag pattern             */
/**********************************************************/
#define CTL_VIE_BUF_TASK_PAUSE          FLGPTN_BIT(0)
#define CTL_VIE_BUF_TASK_PAUSE_END      FLGPTN_BIT(1)
#define CTL_VIE_BUF_TASK_RESUME         FLGPTN_BIT(2)
#define CTL_VIE_BUF_TASK_RESUME_END     FLGPTN_BIT(3)
#define CTL_VIE_BUF_TASK_IDLE           FLGPTN_BIT(4)
#define CTL_VIE_BUF_TASK_RESTART        FLGPTN_BIT(5)
#define CTL_VIE_BUF_TASK_FLUSH          FLGPTN_BIT(6)
#define CTL_VIE_BUF_TASK_RES            FLGPTN_BIT(7)
#define CTL_VIE_BUF_TASK_CHK            FLGPTN_BIT(8)
#define CTL_VIE_BUF_QUEUE_PROC          FLGPTN_BIT(9)
#define CTL_VIE_BUF_EXIT_END            FLGPTN_BIT(10)
#define CTL_VIE_BUF_CB_OUT			  	FLGPTN_BIT(11)
#define CTL_VIE_BUF_TASK_LOCK			FLGPTN_BIT(12)
#define CTL_VIE_BUF_PROC_INIT       (CTL_VIE_BUF_TASK_PAUSE_END|CTL_VIE_BUF_TASK_LOCK)

/**********************************************************/
/*              ctl_vie isp flag pattern                 */
/**********************************************************/
#define CTL_VIE_ISP_TASK_PAUSE          FLGPTN_BIT(0)
#define CTL_VIE_ISP_TASK_PAUSE_END      FLGPTN_BIT(1)
#define CTL_VIE_ISP_TASK_RESUME         FLGPTN_BIT(2)
#define CTL_VIE_ISP_TASK_RESUME_END     FLGPTN_BIT(3)
#define CTL_VIE_ISP_TASK_IDLE           FLGPTN_BIT(4)
#define CTL_VIE_ISP_TASK_RESTART        FLGPTN_BIT(5)
#define CTL_VIE_ISP_TASK_FLUSH          FLGPTN_BIT(6)
#define CTL_VIE_ISP_TASK_RES            FLGPTN_BIT(7)
#define CTL_VIE_ISP_TASK_CHK            FLGPTN_BIT(8)
#define CTL_VIE_ISP_TASK_EXIT_END       FLGPTN_BIT(9)
#define CTL_VIE_ISP_QUEUE_PROC          FLGPTN_BIT(10)
#define CTL_VIE_ISP_CFG_LOCK            FLGPTN_BIT(11)
#define CTL_VIE_ISP_GET_IMG_END         FLGPTN_BIT(12)
#define CTL_VIE_ISP_GET_IMG_READY       FLGPTN_BIT(13)
#define CTL_VIE_ISP_TASK_CB_OUT         FLGPTN_BIT(14)
#define CTL_VIE_ISP_TASK_LOCK	  		FLGPTN_BIT(15)

#define CTL_VIE_ISP_PROC_INIT   (CTL_VIE_ISP_CFG_LOCK | CTL_VIE_ISP_GET_IMG_END|CTL_VIE_ISP_TASK_PAUSE_END|CTL_VIE_ISP_TASK_LOCK)

/*************************************************/
extern THREAD_DECLARE(ctl_vie_buf_tsk, p1);
extern THREAD_DECLARE(ctl_vie_isp_tsk, p1);

extern THREAD_HANDLE g_ctl_vie_buf_tsk_id;
extern ID g_ctl_vie_buf_flg_id;

extern ID g_ctl_vie_flg_id[CTL_VIE_MAX_SUPPORT_ID];
extern void ctl_vie_install_id(void);
extern void ctl_vie_uninstall_id(void);

extern THREAD_HANDLE g_ctl_vie_isp_tsk_id;
extern ID g_ctl_vie_isp_flg_id;
extern void ctl_vie_isp_install_id(void);
extern void ctl_vie_isp_uninstall_id(void);

extern ID ctl_vie_get_flag_id(CTL_VIE_ID id);
extern ID ctl_vie_get_isp_flag_id(void);
extern ID ctl_vie_get_buf_flag_id(void);

#endif //_CTL_VIE_ID_INT_H_
