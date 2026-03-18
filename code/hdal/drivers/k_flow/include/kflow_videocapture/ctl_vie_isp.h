/**
    VIE Ctrl Layer, Algorithm Interface

    @file       ctl_vie_isp.h
    @ingroup    mIVIE_Ctrl
    @note       None

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/


#ifndef _CTL_VIE_ISP_H
#define _CTL_VIE_ISP_H

#include "kflow_common/isp_if.h"
#include "kflow_common/type_vdo.h"


#define CTL_VIE_ISP_STA_NUM				8	//ref. to CTL_VIE_MAX_ENG

typedef struct {
} CTL_VIE_IQ_PARAM;

typedef struct {
	UINT32 vie_bp_3;	//set bp value when trig evt include ISP_EVT_VIE_BP3
} CTL_VIE_BP;

typedef struct {
	URECT in_crp_win;	//vie input crop window
	USIZE out_sz;		//vie output size
} CTL_VIE_ISP_IO_SIZE;

typedef enum {
	CTL_VIE_ISP_CB_MSG_NONE 	= 0x00000000,
	CTL_VIE_ISP_CB_MSG_MAX,
	ENUM_DUMMY4WORD(CTL_VIE_ISP_CB_MSG)
} CTL_VIE_ISP_CB_MSG;

typedef struct {
	UINT32 buf_id;		///for public buffer control
	ULONG  buf_addr;	// virtual address
	ULONG  buf_addr_pa;	// physical address
	VDO_FRAME vdo_frm;
} CTL_VIE_ISP_HEADER_INFO;

typedef struct {
	UINT32 buf_id;		///out, public buffer id
	ULONG  buf_addr;	///out, public buffer virtual address
	ULONG  buf_addr_pa;	///out, public buffer physical address
	UINT32 frm_cnt;		///in,  current image frame count
	UINT32 buf_size;	///in,  require buffer size
} CTL_VIE_ISP_SIM_BUF_NEW;

typedef struct {
	CTL_VIE_ISP_HEADER_INFO isp_head_info;
	BOOL sim_end;	//simulation end flag, kflow will set back to previous status
} CTL_VIE_ISP_SIM_BUF_PUSH;

typedef enum {
	CTL_VIE_ISP_STS_CLOSE = 0,
	CTL_VIE_ISP_STS_READY,
	CTL_VIE_ISP_STS_RUN,
	CTL_VIE_ISP_STS_MAX,
	ENUM_DUMMY4WORD(CTL_VIE_ISP_STATUS)
} CTL_VIE_ISP_STATUS;

typedef enum {
	CTL_VIE_ISP_ITEM_FUNC_EN = 0,	///< [GET],  	data_type: ISP_FUNC_EN
	CTL_VIE_ISP_ITEM_STATUS,		///< [GET],  	data_type: CTL_VIE_ISP_STATUS
	CTL_VIE_ISP_ITEM_IO_SIZE,		///< [GET],  	data_type: CTL_VIE_ISP_IO_SIZE
	CTL_VIE_ISP_ITEM_DUPL_SRC_ID,	///< [GET],  	data_type: CTL_VIE_ID
	CTL_VIE_ISP_ITEM_MULTI_FRM_GRP,	///< [GET],  	data_type: CTL_SEN_OUTPUT_DEST
	CTL_VIE_ISP_ITEM_IQ_PARAM,		///< [SET],  	data_type: CTL_VIE_IQ_PARAM
	CTL_VIE_ISP_ITEM_IMG_OUT,		///< [SET/GET], data_type: CTL_VIE_ISP_HEADER_INFO, note that set is used to unlock buffer
	CTL_VIE_ISP_ITEM_SIM_BUF_NEW,	///< [SET], 	data_type: CTL_VIE_ISP_SIM_BUF_NEW
	CTL_VIE_ISP_ITEM_SIM_BUF_PUSH,	///< [SET], 	data_type: CTL_VIE_ISP_SIM_BUF_PUSH
	CTL_VIE_ISP_ITEM_MAX,
	CTL_VIE_ISP_ISR_CTX = 0x80000000,///< combine this bit for announce this item is from isr context
	ENUM_DUMMY4WORD(CTL_VIE_ISP_ITEM)
} CTL_VIE_ISP_ITEM;

INT32 ctl_vie_isp_evt_fp_reg(CHAR *name, ISP_EVENT_FP fp, ISP_EVENT evt, CTL_VIE_ISP_CB_MSG cb_msg);
INT32 ctl_vie_isp_evt_fp_unreg(CHAR *name);
INT32 ctl_vie_isp_set(ISP_ID id, CTL_VIE_ISP_ITEM item, void *data);
INT32 ctl_vie_isp_get(ISP_ID id, CTL_VIE_ISP_ITEM item, void *data);

#endif
