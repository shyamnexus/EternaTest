#ifndef _CTL_VIE_EVENT_INT_H_
#define _CTL_VIE_EVENT_INT_H_

/**
    ctl_vie_event_int.h


    @file       ctl_vie_event_int.h
    @ingroup    mISYSAlg
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/
#include "ctl_vie_event.h"
#include "ctl_vie_utility_int.h"

#define VIE_EVENT_ROOT_MAX		8
#define VIE_EVENT_ENTRY_MAX		6

#define VIE_EVENT_CTRL_MASK 0xffff0000
#define VIE_EVENT_NAME_MAX 10

#define VIE_FREE_FLAG_PAGE_SIZE			32//sizeof(FLGPTN)
#define VIE_FREE_FLAG_START				(VIE_EVENT_ROOT_MAX + 1)

#define VIE_EVENT_FLAG_MAX			2
#define VIE_EVENT_FREE_ENTRY_ITEM_MAX_NUM ((VIE_EVENT_FLAG_MAX * VIE_FREE_FLAG_PAGE_SIZE) - VIE_FREE_FLAG_START)

#define VIE_EVENT_FREE_ENTRY_ITEM_NUM (VIE_EVENT_ROOT_MAX * VIE_EVENT_ENTRY_MAX)
#if (VIE_EVENT_FREE_ENTRY_ITEM_NUM > VIE_EVENT_FREE_ENTRY_ITEM_MAX_NUM)
#error "VIE_EVENT_POOL_FREE_NUM must be <= VIE_EVENT_FREE_ENTRY_ITEM_MAX_NUM"
#endif

#define VIE_EVENT_STS_FREE			0x00000100
#define VIE_EVENT_STS_CTL_MASK		0xffff0000

#define VIE_EVENT_EVENT_CTL_MASK	VIE_EVENT_STS_CTL_MASK

typedef struct {
	CHAR name[VIE_EVENT_NAME_MAX];
	UINT32 enter_ts;
	CTL_VIE_EVENT_FP fp;
	UINT32 exit_ts;
	UINT32 sts;
	CTL_VIE_LIST_HEAD list;
} VIE_EVENT_ENTRY_ITEM;

typedef struct {
	CHAR name[VIE_EVENT_NAME_MAX];
	UINT32 sts;
	CTL_VIE_LIST_HEAD root_list;
	UINT32 root_lock;
	CTL_VIE_LIST_HEAD entry_list[VIE_EVENT_ENTRY_MAX];
	UINT32 entry_lock[VIE_EVENT_ENTRY_MAX];
} VIE_EVENT_ROOT_ITEM;

typedef struct {
	UINT32 flag;
	UINT32 ptn;
} VIE_EVENT_LOCK_INFO;

/**
	sample:
	open -> register -> proc -> proc -> proc -> proc -> proc -> close
	open -> register -> proc -> register -> proc_isr -> proc -> proc -> close

*/
/**
     vie_event_reset

     @note clear all status
*/
extern void vie_event_reset(void);

/**
     vie_event_open

     @note open a process handle

     @param[in] name      prcess name

     @return Description of data returned.
		- @b 0:	open fail
*/
extern ULONG vie_event_open(CHAR *name);

/**
     vie_event_close

     @note close a process handle & flush all register function.

     @param[in] handle

     @return Description of data returned.
		- @b VIE_EVENT_OK: success
		- @b VIE_EVENT_NG: fail
*/
extern UINT32 vie_event_close(ULONG handle);

/**
     vie_event_register

     @note register callback function

     @param[in] handle
     @param[in] event: event id
     @param[in] fp: callback event function pointer
     @param[in] name: event name

     @return Description of data returned.
		- @b VIE_EVENT_OK: success
		- @b VIE_EVENT_NG: fail
*/
extern UINT32 vie_event_register(ULONG handle, UINT32 event, CTL_VIE_EVENT_FP fp, CHAR *name);

/**
     vie_event_unregister

     @note unregister callback function

     @param[in] handle
     @param[in] event: event id
     @param[in] fp: callback event function pointer

     @return Description of data returned.
		- @b VIE_EVENT_OK: success
		- @b VIE_EVENT_NG: fail
*/
extern UINT32 vie_event_unregister(ULONG handle, UINT32 event, CTL_VIE_EVENT_FP fp);

/**
     vie_event_proc

     @note trigger event

     @param[in] handle
     @param[in] event: event id
     @param[in] p_in: input data structure
     @param[out] p_out: output data structure

     @return Description of data returned.
		- @b VIE_EVENT_OK: success
		- @b VIE_EVENT_NG: fail
*/
extern INT32 vie_event_proc(ULONG handle, UINT32 event, void *p_in, void *p_out);

/**
     vie_event_proc_isr

     @note trigger isr event

     @param[in] handle
     @param[in] event: event id
     @param[in] p_in: input data structure
     @param[out] p_out: output data structure

     @return Description of data returned.
		- @b VIE_EVENT_OK: success
		- @b VIE_EVENT_NG: fail
*/
extern INT32 vie_event_proc_isr(ULONG handle, UINT32 event, void *p_in, void *p_out);

/**
     vie_event_proc_isr

     @note trigger isr event

     @param[in] handle
     @param[in] event: event id
     @param[in] p_in: input data structure
     @param[out] p_out: output data structure

     @return Description of data returned.
		- @b VIE_EVENT_OK: success
		- @b VIE_EVENT_NG: fail
*/
extern void vie_event_dump(int (*dump)(const char *fmt, ...));
#endif //_CTL_VIE_EVENT_INT_H_
