/**
    KDF SIE Layer

    @file       kdf_sie_int.h
    @ingroup    mIIPL_Ctrl
    @note       None

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/


#ifndef _KDF_SIE_INT_H
#define _KDF_SIE_INT_H

#include "ctl_sie_utility.h"
#include "kdrv_sie.h"
#include "ctl_sie_dbg.h"

typedef struct {
	UINT32 ctl_id;
	UINT32 kdrv_id;
	UINT32 chip;
	KDRV_SIE_ISRCB isrcb_fp;    //isr cb function pointer
} KDF_SIE_HDL;

typedef struct {
	UINT32 context_num;
	UINT32 context_used;
	UINT32 contex_size;;
	ULONG start_addr;
	UINT32 req_size;
} KDF_SIE_HDL_CONTEXT;

UINT32 kdf_sie_buf_query(UINT32 num);
INT32 kdf_sie_init(ULONG buf_addr, UINT32 buf_size);
INT32 kdf_sie_uninit(void);
INT32 kdf_sie_set_mclk(UINT32 id, void *data);
ULONG kdf_sie_open(CTL_SIE_ID id, void *data);
INT32 kdf_sie_close(ULONG hdl);
INT32 kdf_sie_set(ULONG hdl, UINT64 item, UINT64 item_int, void *data);
INT32 kdf_sie_get(ULONG hdl, UINT64 item, UINT64 item_int, void *data);
INT32 kdf_sie_trigger(ULONG hdl, void *data);
INT32 kdf_sie_suspend(ULONG hdl, void *data);
INT32 kdf_sie_resume(ULONG hdl, void *data);
INT32 kdf_sie_get_limit(UINT32 id, void *data);
INT32 kdf_sie_dump_fb_info(VOS_FILE fd);
void kdf_sie_dump_intrpt_sts(ULONG dump_id_bit, int (*dump)(const char *fmt, ...));
void kdf_sie_set_err_log_rate(CTL_SIE_ID id, UINT32 err_log_rate);

#endif //_KDF_SIE_INT_H
