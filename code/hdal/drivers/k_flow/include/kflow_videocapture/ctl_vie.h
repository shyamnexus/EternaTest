/**
    VIE CTRL Layer

    @file       vie_ctrl.h
    @ingroup    mILibIPH
    @note

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/
#ifndef _CTL_VIE_H_
#define _CTL_VIE_H_

#define KFLOW_VIE_READY DISABLE

#include "kwrap/type.h"
#include "ctl_vie_spt.h"

typedef enum {
	CTL_VIE_CBEVT_BUFIO			= 0 | CTL_VIE_EVENT_ISR_TAG,	//sts ref. to CTL_VIE_BUF_IO_CFG
	CTL_VIE_CBEVT_ENG_VIE_ISR	= 1 | CTL_VIE_EVENT_ISR_TAG,	//sts ref. to CTL_VIE_INTE
	CTL_VIE_CBEVT_MAX = 2,
} CTL_VIE_CBEVT_TYPE;

/**
	VIE In/Out buffer config
*/
typedef enum {
	CTL_VIE_BUF_IO_NEW		= 0x00000001,
	CTL_VIE_BUF_IO_PUSH		= 0x00000002,
	CTL_VIE_BUF_IO_LOCK		= 0x00000004,
	CTL_VIE_BUF_IO_UNLOCK	= 0x00000008,
	CTL_VIE_BUF_IO_ALL		= 0xffffffff,
	ENUM_DUMMY4WORD(CTL_VIE_BUF_IO_CFG)
} CTL_VIE_BUF_IO_CFG;


typedef struct {
	CTL_VIE_CBEVT_TYPE cbevt;
	CTL_VIE_EVENT_FP fp;
	UINT32 sts;					//wait event status
} CTL_VIE_REG_CB_INFO;

/**
	vdo_frm reserved info
	reserved[0]:
	reserved[1]:
	reserved[2]:
	reserved[3]:
	reserved[4]:
	reserved[5]:
	reserved[6]: dram: dest crop construction of (w << 16 | h)
	reserved[7]:
*/
typedef struct {
	UINT32 buf_id;			// for public buffer control
	ULONG  buf_addr;		// virtual address
	ULONG  buf_addr_pa;		// physical address (in 64bits, isr cannot va2pa)
	VDO_FRAME vdo_frm;
} CTL_VIE_HEADER_INFO;

void kflow_ctl_vie_init(void);
void kflow_ctl_vie_uninit(void);
UINT32 ctl_vie_buf_query(UINT32 num);
INT32 ctl_vie_init(ULONG buf_addr, UINT32 buf_size);
INT32 ctl_vie_uninit(void);
ULONG ctl_vie_open(void *open_cfg);	// CTL_VIE_OPEN_CFG
INT32 ctl_vie_close(ULONG hdl);
INT32 ctl_vie_set(ULONG hdl, CTL_VIE_ITEM item, void *data);
INT32 ctl_vie_get(ULONG hdl, CTL_VIE_ITEM item, void *data);
INT32 ctl_vie_suspend(ULONG hdl, void *data);
INT32 ctl_vie_resume(ULONG hdl, void *data);
void ctl_vie_spt(CTL_VIE_ID id, CTL_VIE_SPT_ITEM item, void *spt);

#endif //_CTL_VIE_H_
