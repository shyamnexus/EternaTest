/**
    @brief 		Header file of fileout library.

    @file 		bsmux_tsk.h

    @ingroup 	mBsMux

    @note		Nothing.

    Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _BSMUX_TSK_H
#define _BSMUX_TSK_H

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
extern VOID ISF_BsMux_Tsk(VOID);

/**
    Tsk: trigger tsk motion
*/
extern VOID bsmux_tsk_init(VOID);
extern VOID bsmux_tsk_start(VOID);
extern VOID bsmux_tsk_bs_in(VOID);
extern VOID bsmux_tsk_getall(VOID);
extern VOID bsmux_tsk_stop(VOID);
extern VOID bsmux_tsk_wait_idle(VOID);
extern VOID bsmux_tsk_destroy(VOID);

/**
    Tsk: set tsk status init/idle/resume/run/suspend
*/
extern ER bsmux_tsk_status_init(BSM_IO id);
extern ER bsmux_tsk_status_idle(BSM_IO id);
extern ER bsmux_tsk_status_resume(BSM_IO id);
extern ER bsmux_tsk_status_run(BSM_IO id);
extern ER bsmux_tsk_status_suspend(BSM_IO id);
extern ER bsmux_tsk_status_dbg(BOOL value);
extern ER bsmux_tsk_action_dbg(BOOL value);

/**
    Tsk: trig tsk do getall/cutfile
*/
extern ER bsmux_tsk_trig_getall(BSM_IO id);
extern ER bsmux_tsk_trig_cutfile(BSM_IO id);

extern ER bsmux_tsk_still(BSM_IO id);
extern ER bsmux_tsk_awake(BSM_IO id);

#endif //_BSMUX_TSK_H
