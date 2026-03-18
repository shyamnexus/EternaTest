/**
    VIE Ctrl Layer, Algorithm function control

    @file       ctl_vie_isp_int.h
    @ingroup    mIVIE_Ctrl
    @note       None

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef _CTL_VIE_ISP_INT_H
#define _CTL_VIE_ISP_INT_H

#include "ctl_vie_utility_int.h"
#include "ctl_vie_isp.h"

// funtion type for vie
typedef INT32(*CTL_VIE_ISP_SET_FP)(CTL_VIE_ID, void *);
typedef INT32(*CTL_VIE_ISP_GET_FP)(CTL_VIE_ID, void *);

#define CTL_VIE_ISP_HANDLE_NAME_MAX_LENGTH  (32)
#define CTL_VIE_ISP_HANDLE_STATUS_FREE      (0)
#define CTL_VIE_ISP_HANDLE_STATUS_USED      (1)

typedef struct {
	CHAR name[CTL_VIE_ISP_HANDLE_NAME_MAX_LENGTH];
	ISP_EVENT_FP fp;
	ISP_EVENT evt;
	UINT32 status;
	CTL_VIE_ISP_CB_MSG cb_msg;
} CTL_VIE_ISP_HANDLE;

typedef struct {
	UINT32 id;
	CTL_VIE_HEADER_INFO vie_head_info;
} CTL_VIE_ISP_GET_IMG_INFO;

typedef enum {
	CTL_VIE_IQ_FUNC_NONE        = 0x00000000, ///< none
} CTL_VIE_ISP_IQ_FUNC;

typedef struct {
	BOOL skip_cb_en;
	CTL_VIE_ISP_IQ_FUNC iq_func_off;
} CTL_VIE_ISP_IQ_DBG_INFO;

void ctl_vie_isp_init(void);
UINT32 ctl_vie_isp_get_sts(void);
INT32 ctl_vie_isp_update_sta_out(CTL_VIE_ID id);
INT32 ctl_vie_isp_event_cb(CTL_VIE_ID id, UINT32 sts, UINT64 fc, void *param);
BOOL ctl_vie_isp_int_pull_img_out(UINT32 id, ULONG vie_header_addr);
void ctl_vie_isp_set_skip(UINT32 id, BOOL skip_en, UINT32 iq_func_off);
CTL_VIE_ISP_HANDLE *ctl_vie_isp_get_hdl(void);

#endif //_CTL_VIE_ISP_INT_H
