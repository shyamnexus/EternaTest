/**
    @brief 		Header file of fileout library.

    @file 		bsmux_ctrl.h

    @ingroup 	mBsMux

    @note		Nothing.

    Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _BSMUX_CTRL_H
#define _BSMUX_CTRL_H

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/

/* =========================================================================== */
/* =                           MEM Ctrl Interface                            = */
/* =========================================================================== */
extern ER bsmux_ctrl_mem_alloc(BSM_IO id);
extern ER bsmux_ctrl_mem_free(BSM_IO id);
extern ER bsmux_ctrl_mem_init(BSM_IO id);
extern ER bsmux_ctrl_mem_cal_range(BSM_IO id, VOID *p_size);
extern ER bsmux_ctrl_mem_set_range(BSM_IO id, ULONG value);
extern ER bsmux_ctrl_mem_det_range(BSM_IO id);
extern ER bsmux_ctrl_mem_cal_buffer(BSM_IO id, VOID *p_size);
extern ER bsmux_ctrl_mem_set_buffer(BSM_IO id, VOID *p_buf);
extern ER bsmux_ctrl_mem_get_bufinfo(BSM_IO id, UINT32 type, VOID *p_value); //type: BSMUX_CTRL_INFO
extern ER bsmux_ctrl_mem_set_bufinfo(BSM_IO id, UINT32 type, ULONG value); //type: BSMUX_CTRL_INFO
extern ER bsmux_ctrl_mem_dbg(BOOL value);

/* =========================================================================== */
/* =                           BSQ Ctrl Interface                            = */
/* =========================================================================== */
extern UINT32 bsmux_ctrl_bs_getnum(BSM_IO id, UINT32 lock);
extern UINT32 bsmux_ctrl_bs_active(BSM_IO id, UINT32 lock);
extern ER bsmux_ctrl_bs_calcgopnum(BSM_IO id, VOID *p_num, VOID *p_sec);
extern ER bsmux_ctrl_bs_calcqueue(BSM_IO id, VOID *p_size);
extern ER bsmux_ctrl_bs_initqueue(BSM_IO id, BSM_AD addr, BSM_SZ size);
extern ER bsmux_ctrl_bs_enqueue(BSM_IO id, VOID *p_bsq);
extern ER bsmux_ctrl_bs_dequeue(BSM_IO id, VOID *p_bsq);
extern ER bsmux_ctrl_bs_predequeue(BSM_IO id, VOID *p_bsq);
extern ER bsmux_ctrl_bs_rollback(BSM_IO id, UINT32 sec, UINT32 check);
extern ER bsmux_ctrl_bs_reorder(BSM_IO id, UINT32 sec);
extern ER bsmux_ctrl_bs_copy2strgbuf(BSM_IO id, VOID *p_bsq);
extern ER bsmux_ctrl_bs_copy2buffer(BSM_IO id, VOID *p_bsq);
extern ER bsmux_ctrl_bs_putall2card(BSM_IO id);
extern ER bsmux_ctrl_bs_init_fileinfo(BSM_IO id);
extern ER bsmux_ctrl_bs_get_fileinfo(BSM_IO id, UINT32 type, VOID *p_value); //type: BSMUX_FILE_INFO
extern ER bsmux_ctrl_bs_set_fileinfo(BSM_IO id, UINT32 type, ULONG value); //type: BSMUX_FILE_INFO
extern ER bsmux_ctrl_bs_add_lastI(BSM_IO id, UINT32 bsqnum);
extern UINT32 bsmux_ctrl_bs_get_lastI(BSM_IO id, UINT32 back_sec);
extern ER bsmux_ctrl_bs_dbg(BOOL value);

/* =========================================================================== */
/* =                      External Main Ctrl Interface                       = */
/* =========================================================================== */
extern ER bsmux_ctrl_init(VOID);
extern ER bsmux_ctrl_open(BSM_IO id);
extern ER bsmux_ctrl_start(BSM_IO id);
extern ER bsmux_ctrl_stop(BSM_IO id);
extern ER bsmux_ctrl_close(BSM_IO id);
extern ER bsmux_ctrl_uninit(VOID);
extern ER bsmux_ctrl_in(BSM_IO id, VOID *p_data);
extern ER bsmux_ctrl_extend(BSM_IO id);
extern ER bsmux_ctrl_pause(BSM_IO id);
extern ER bsmux_ctrl_resume(BSM_IO id);
extern ER bsmux_ctrl_emergency(BSM_IO id);
extern ER bsmux_ctrl_trig_emr(BSM_IO id, BSM_IO pause_id);
extern ER bsmux_ctrl_trig_cutnow(BSM_IO id);

#endif //_BSMUX_CTRL_H
