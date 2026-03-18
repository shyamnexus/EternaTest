/**
    @brief 		Header file of fileout library.

    @file 		bsmux_ts.h

    @ingroup 	mBsMux

    @note		Nothing.

    Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _BSMUX_TS_H
#define _BSMUX_TS_H

/*-----------------------------------------------------------------------------*/
/* Specific  Functions                                                         */
/*-----------------------------------------------------------------------------*/
extern ER bsmux_ts_make_pmt(BSM_IO id, BSM_AD pmt_addr);
extern ER bsmux_ts_make_pcr(BSM_IO id, BSM_AD pcr_addr, UINT64 pcr_value);
extern UINT64 bsmux_ts_get_pts(UINT32 pes_header_len, UINT32 pts_len, BSM_AD bs_addr);

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
//common (16)
extern ER bsmux_ts_dbg(BOOL value);
extern ER bsmux_ts_tskobj_init(BSM_IO id, VOID *p_action);
extern ER bsmux_ts_open(VOID);
extern ER bsmux_ts_close(VOID);
extern BSM_SZ bsmux_ts_mux(VOID *dest, VOID *p_src, BSM_SZ size, UINT32 method);
extern ER bsmux_ts_get_need_size(BSM_IO id, VOID *p_size);
extern ER bsmux_ts_set_need_size(BSM_IO id, BSM_AD addr, VOID *p_size);
extern ER bsmux_ts_clean(BSM_IO id);
extern ER bsmux_ts_update_vidinfo(BSM_IO id, BSM_AD addr, VOID *p_bsq);
extern ER bsmux_ts_release_buf(VOID *p_buf);
extern ER bsmux_ts_add_gps_data(BSM_IO id, VOID *p_bsq);
extern ER bsmux_ts_add_thumb(BSM_IO id, VOID *p_bsq);
extern ER bsmux_ts_add_last(BSM_IO id);
extern ER bsmux_ts_put_last(BSM_IO id);
extern ER bsmux_ts_add_meta(BSM_IO id, VOID *p_bsq);
extern BOOL bsmux_ts_check_bufuse(BSM_IO id, VOID *p_bsq);
//sepcific (6)
extern ER bsmux_ts_save_entry(BSM_IO id, VOID *p_bsq);
extern ER bsmux_ts_nidx_pad(BSM_IO id);
extern ER bsmux_ts_make_header(BSM_IO id, VOID *p_maker);
extern ER bsmux_ts_update_header(BSM_IO id, VOID *p_maker);
extern ER bsmux_ts_make_pes(BSM_IO id, VOID *p_src);
extern ER bsmux_ts_make_pat(BSM_IO id, BSM_AD pat_addr);

#endif //_BSMUX_TS_H
