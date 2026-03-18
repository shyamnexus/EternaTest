/**
    KDF VIE Layer

    @file       kdf_vie_int.h
    @ingroup    mIIPL_Ctrl
    @note       None

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/


#ifndef _KDF_VIE_INT_H
#define _KDF_VIE_INT_H

#include "ctl_vie_utility.h"
#include "kdrv_vie.h"

typedef struct {
	UINT32 ctl_id;
	UINT32 kdrv_id;
	UINT32 chip;
	KDRV_VIE_ISRCB isrcb_fp;    //isr cb function pointer
} KDF_VIE_HDL;

typedef struct {
	UINT32 context_num;
	UINT32 context_used;
	UINT32 contex_size;;
	ULONG  start_addr;
	UINT32 req_size;
} KDF_VIE_HDL_CONTEXT;


UINT32 kdf_vie_buf_query(void);
INT32 kdf_vie_init(ULONG buf_addr, UINT32 buf_size);
INT32 kdf_vie_uninit(void);
INT32 kdf_vie_set_mclk(UINT32 id, void *data);
INT32 kdf_vie_set_ccir_header(UINT32 id, void *data);
ULONG kdf_vie_open(CTL_VIE_ID id, void *data);
INT32 kdf_vie_close(ULONG hdl);
INT32 kdf_vie_set(ULONG hdl, UINT64 item, void *data);
INT32 kdf_vie_get(ULONG hdl, UINT64 item, void *data);
INT32 kdf_vie_trigger(ULONG hdl, void *data);
INT32 kdf_vie_suspend(ULONG hdl, void *data);
INT32 kdf_vie_resume(ULONG hdl, void *data);
INT32 kdf_vie_get_limit(UINT32 id, void *data);

void kdf_vie_dump_intrpt_sts(ULONG dump_id_bit, int (*dump)(const char *fmt, ...));
void kdf_vie_set_err_log_rate(CTL_VIE_ID id, UINT32 err_log_rate);

#endif //_KDF_VIE_INT_H
