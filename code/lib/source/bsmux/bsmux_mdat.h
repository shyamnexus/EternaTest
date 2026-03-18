/**
    @brief 		Header file of fileout library.

    @file 		bsmux_mdat.h

    @ingroup 	mBsMux

    @note		Nothing.

    Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _BSMUX_MDAT_H
#define _BSMUX_MDAT_H

/*-----------------------------------------------------------------------------*/
/* Specific  Functions                                                         */
/*-----------------------------------------------------------------------------*/
extern ER     bsmux_mdat_set_hdr_mem(UINT32 type, BSM_IO id, BSM_AD addr, BSM_SZ size); //type:BSMUX_HDRMEM_XXX
extern BSM_AD bsmux_mdat_get_hdr_mem(UINT32 type, BSM_IO id); //type:BSMUX_HDRMEM_XXX
extern ER     bsmux_mdat_rel_hdr_mem(UINT32 type, BSM_IO id, BSM_AD addr); //type:BSMUX_HDRMEM_XXX
extern BOOL   bsmux_mdat_chk_hdr_mem(UINT32 type, BSM_IO id); //type:BSMUX_HDRMEM_XXX
extern UINT32 bsmux_mdat_set_hdr_count(UINT32 value);
extern UINT32 bsmux_mdat_get_hdr_count(VOID);
extern ER     bsmux_mdat_nidx_make(BSM_IO id, UINT64 offset, UINT32 nidxid);
extern ER     bsmux_mdat_gps_make(BSM_IO id, UINT64 offset);
extern BSM_SZ bsmux_mdat_calc_header_size(BSM_IO id);

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
//common (16)
extern ER bsmux_mdat_dbg(BOOL value);
extern ER bsmux_mdat_tskobj_init(BSM_IO id, VOID *p_action);
extern ER bsmux_mdat_open(VOID);
extern ER bsmux_mdat_close(VOID);
extern BSM_SZ bsmux_mdat_memcpy(VOID *dest, VOID *p_src, BSM_SZ size, UINT32 method);
extern ER bsmux_mdat_get_need_size(BSM_IO id, VOID *p_size);
extern ER bsmux_mdat_set_need_size(BSM_IO id, BSM_AD addr, VOID *p_size);
extern ER bsmux_mdat_clean(BSM_IO id);
extern ER bsmux_mdat_update_vidsize(BSM_IO id, BSM_AD addr, VOID *p_bsq);
extern ER bsmux_mdat_release_buf(VOID *p_buf);
extern ER bsmux_mdat_gps_pad(BSM_IO id, VOID *p_bsq);
extern ER bsmux_mdat_add_thumb(BSM_IO id, VOID *p_bsq);
extern ER bsmux_mdat_add_last(BSM_IO id);
extern ER bsmux_mdat_put_last(BSM_IO id);
extern ER bsmux_mdat_add_meta(BSM_IO id, VOID *p_bsq);
extern BOOL bsmux_mdat_check_bufuse(BSM_IO id, VOID *p_bsq);
//sepcific (7)
extern ER bsmux_mdat_save_entry(BSM_IO id, VOID *p_bsq);
extern ER bsmux_mdat_nidx_pad(BSM_IO id);
extern ER bsmux_mdat_make_header(BSM_IO id, VOID *p_maker);
extern ER bsmux_mdat_update_header(BSM_IO id, VOID *p_maker);
extern ER bsmux_mdat_make_front_moov(BSM_IO id, VOID *p_maker, UINT32 minus1sec);

#endif //_BSMUX_MDAT_H
