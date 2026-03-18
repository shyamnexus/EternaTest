/**
    @brief 		Header file of fileout library.

    @file 		bsmux_util.h

    @ingroup 	mBsMux

    @note		Nothing.

    Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _BSMUX_UTIL_H
#define _BSMUX_UTIL_H

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/

/* =========================================================================== */
/* =                           Common Utility API                            = */
/* =========================================================================== */
extern ER bsmux_util_get_mid(BSM_IO id, VOID *maker_id);
extern ER bsmux_util_set_minfo(UINT32 type, ULONG p1, ULONG p2, ULONG p3); //type: MEDIAWRITE_SETINFO_TYPE
extern ER bsmux_util_get_minfo(BSM_IO id, UINT32 type, VOID *p1, VOID *p2, VOID *p3); //type: MEDIAWRITE_GETINFO_TYPE
extern ER bsmux_util_set_param(BSM_IO id, UINT32 param, ULONG value); //type: BSMUXER_PARAM
extern ER bsmux_util_get_param(BSM_IO id, UINT32 param, VOID *p_value); //type: BSMUXER_PARAM
extern ER bsmux_util_wait_ready(BSM_IO id);
extern ER bsmux_util_prepare_buf(BSM_IO id, UINT32 type, BSM_AD addr, UINT64 size, UINT64 pos); //type: BSMUX_BS2CARD_NORMAL
extern ER bsmux_util_mem_dbg(BOOL value);
extern ER bsmux_util_strgbuf_init(BSM_IO id);
extern ER bsmux_util_strgbuf_handle_entry(BSM_IO id, VOID *p_bsq);
extern ER bsmux_util_strgbuf_handle_buffer(BSM_IO id, VOID *p_bsq);
extern ER bsmux_util_strgbuf_handle_space(BSM_IO id, VOID *p_buf);
extern ER bsmux_util_strgbuf_handle_header(BSM_IO id);
extern VOID bsmux_util_put_version(UINT32 date, UINT32 ver1, UINT32 ver2);
extern VOID bsmux_util_get_version(VOID *p_version);
extern VOID bsmux_util_show_setting(BSM_IO id);
extern VOID bsmux_util_set_result(UINT32 result);
extern VOID bsmux_util_chk_frm_sync(BSM_IO id, VOID *p_data);
extern VOID bsmux_util_dump_info(BSM_IO id);
extern VOID bsmux_util_dump_buffer(VOID *p_buf, BSM_SZ length);
extern BOOL bsmux_util_is_invalid_id(BSM_IO id);
extern BOOL bsmux_util_is_null_obj(VOID *p_obj);
extern UINT32 bsmux_util_is_not_normal(VOID);
extern BSM_SZ bsmux_util_set_writeblock_size(BSM_IO id);
extern BSM_AD bsmux_util_get_buf_pa(BSM_IO id, BSM_AD va);
extern BSM_AD bsmux_util_get_buf_va(BSM_IO id, BSM_AD pa);
extern BSM_SZ bsmux_util_calc_align(BSM_SZ input, BSM_SZ alignsize, UINT32 type); //type: NMEDIAREC_ALIGN_ROUND
extern UINT32 bsmux_util_calc_sec(VOID *p_setting);
extern UINT32 bsmux_util_calc_entry_per_sec(BSM_IO id);
extern UINT32 bsmux_util_calc_frm_num(BSM_IO id, UINT32 type); //type: BSMUX_TYPE
extern UINT32 bsmux_util_pset_padding_size(BSM_IO id, BSM_SZ size); //fast put flow ver3
extern BSM_SZ bsmux_util_calc_padding_size(BSM_IO id); //fast put flow ver3
extern BSM_SZ bsmux_util_calc_reserved_size(BSM_IO id);
extern BSM_SZ bsmux_util_memcpy(VOID *dest, VOID *src, BSM_SZ size, UINT32 method);
extern BSM_SZ bsmux_util_muxalign(BSM_IO id, BSM_SZ src);
extern UINT64 bsmux_util_calc_timestamp(BSM_IO id, UINT64 time_start);
extern BOOL bsmux_util_check_tag(BSM_IO id, VOID *p_bsq);
extern BOOL bsmux_util_check_firstI(BSM_IO id, VOID *p_bsq);
extern BOOL bsmux_util_check_frmidx(BSM_IO id, VOID *p_bsq);
extern BOOL bsmux_util_check_duration(BSM_IO id, VOID *p_bsq);
extern BOOL bsmux_util_check_buflocksts(BSM_IO id, VOID *p_bsq);
extern BOOL bsmux_util_check_buflockdur(BSM_IO id, VOID *p_bsq);
/* =========================================================================== */
/* =                           Plugin Utility API                            = */
/* =========================================================================== */
extern ER bsmux_util_plugin(BSM_IO id);
//common (16)
extern ER bsmux_util_plugin_dbg(BOOL value);
extern ER bsmux_util_tskobj_init(BSM_IO id, VOID *p_action);
extern ER bsmux_util_engine_open(VOID);
extern ER bsmux_util_engine_close(VOID);
extern BSM_SZ bsmux_util_engcpy(VOID *dest, VOID *p_src, BSM_SZ size, UINT32 method);
extern ER bsmux_util_plugin_get_size(BSM_IO id, VOID *p_size);
extern ER bsmux_util_plugin_set_size(BSM_IO id, BSM_AD addr, VOID *p_size);
extern ER bsmux_util_plugin_clean(BSM_IO id);
extern ER bsmux_util_update_vidinfo(BSM_IO id, BSM_AD addr, VOID *p_bsq);
extern ER bsmux_util_release_buf(VOID *p_buf);
extern ER bsmux_util_add_gps(BSM_IO id, VOID *p_bsq);
extern ER bsmux_util_add_thumb(BSM_IO id, VOID *p_bsq);
extern ER bsmux_util_add_lasting(BSM_IO id);
extern ER bsmux_util_put_lasting(BSM_IO id);
extern ER bsmux_util_add_meta(BSM_IO id, VOID *p_bsq);
extern BOOL bsmux_util_check_bufuse(BSM_IO id, VOID *p_bsq);
//sepcific (7)
extern ER bsmux_util_save_entry(BSM_IO id, VOID *p_bsq);
extern ER bsmux_util_nidx_pad(BSM_IO id);
extern ER bsmux_util_make_header(BSM_IO id);
extern ER bsmux_util_update_header(BSM_IO id); //@note BSM:updateHeader status Waiting_Hit, donothing
extern ER bsmux_util_make_pes(BSM_IO id, VOID *p_bsq);
extern ER bsmux_util_make_pat(BSM_IO id);
extern ER bsmux_util_make_moov(BSM_IO id, UINT32 minus1sec);

#endif //_BSMUX_UTIL_H
