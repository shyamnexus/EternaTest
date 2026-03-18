/**
    IPL Ctrl Layer, Buffer ctrl

    @file       ctl_ipp_buf_int.h
    @ingroup    mIIPL_Ctrl
    @note       None

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/


#ifndef _CTL_IPP_BUF_INT_H
#define _CTL_IPP_BUF_INT_H

#include "kwrap/type.h"
#include "ctl_ipp_util_int.h"

#define CTL_IPP_BUF_NO_USE  0xffffffff

/* for single out check,
	this tag will be assign to the top of the buffer
	if this tag is not overwrite by engine, the buffer should not be pushed
*/
#define CTL_IPP_BUF_CHK_TAG MAKEFOURCC('N', 'O', 'U', 'T')

// for aided map meta sign
#define CTL_IPP_BUF_AIDED_MAP_TAG MAKEFOURCC('A', 'D', 'M', 'P')

// for ctl_ipp allocate nvtmpp buffer
#define CTL_IPP_BUF_MAKE_NVTMPP_MODULE MAKE_NVTMPP_MODULE('c', 't', 'l', '_', 'i', 'p', 'p', 0)

/**********************************/
/*	ctl_ipp_buf message queue	  */
/**********************************/
#define CTL_IPP_BUF_MSG_STS_FREE    0x00000000
#define CTL_IPP_BUF_MSG_STS_LOCK    0x00000001

#define CTL_IPP_BUF_MSG_TYPE_NUM 			(5)
#define CTL_IPP_BUF_MSG_IGNORE      0x00000000
#define CTL_IPP_BUF_MSG_PUSH		0x00000001	//[0]: buf_io_fp, [1]: ctrl_info address, [2]: reserve
#define CTL_IPP_BUF_MSG_RELEASE		0x00000002  //[0]: buf_io_fp, [1]: ctrl_info address, [2]: bufio_stop
#define CTL_IPP_BUF_MSG_PUSH_LOWDLY	0x00000003	//[0]: buf_io_fp, [1]: ctrl_info address, [2]: reserve, add to the head of list
#define CTL_IPP_BUF_MSG_PUSH_SLICE	0x00000004	//[0]: buf_io_fp, [1]: ctrl_info address, [2]: slice_idx

typedef struct {
	UINT32 cmd;
	ULONG param[5];
	UINT32 rev[2];
	CTL_IPP_LIST_HEAD list;
} CTL_IPP_BUF_MSG_EVENT;

typedef enum {
	CTRL_IPP_BUF_ITEM_MO = 0,
	CTRL_IPP_BUF_ITEM_MO_DBG,
	CTRL_IPP_BUF_ITEM_MV,
	CTRL_IPP_BUF_ITEM_MV_DBG,
	CTRL_IPP_BUF_ITEM_DFG,
	CTRL_IPP_BUF_ITEM_DFG_DBG,
	CTRL_IPP_BUF_ITEM_LCE,
	CTRL_IPP_BUF_ITEM_LCE_DBG,
	CTRL_IPP_BUF_ITEM_PM,
	CTRL_IPP_BUF_ITEM_PM_DBG,
	CTRL_IPP_BUF_ITEM_YOUT,
	CTRL_IPP_BUF_ITEM_YOUT_DBG,
	CTRL_IPP_BUF_ITEM_MS_ROI,
	CTRL_IPP_BUF_ITEM_MS_ROI_DBG,
	CTRL_IPP_BUF_ITEM_MS,
	CTRL_IPP_BUF_ITEM_MS_DBG,
	CTRL_IPP_BUF_ITEM_3DNR_STA,
	CTRL_IPP_BUF_ITEM_3DNR_STA_DBG,
	CTRL_IPP_BUF_ITEM_WDR,
	CTRL_IPP_BUF_ITEM_WDR_DBG,
	CTRL_IPP_BUF_ITEM_IFE_VA,
	CTRL_IPP_BUF_ITEM_IFE_VA_DBG,
	CTRL_IPP_BUF_ITEM_IPE_VA,
	CTRL_IPP_BUF_ITEM_IPE_VA_DBG,
	CTRL_IPP_BUF_ITEM_IME_VA,
	CTRL_IPP_BUF_ITEM_IME_VA_DBG,
	CTRL_IPP_BUF_ITEM_FCVG,
	CTRL_IPP_BUF_ITEM_FCVG_DBG,
	CTRL_IPP_BUF_ITEM_MAX,
} CTRL_IPP_BUF_ITEM;

typedef enum {
	CTL_IPP_BUF_MO_1,
	CTL_IPP_BUF_MO_2,
	CTL_IPP_BUF_MO_3,
	CTL_IPP_BUF_MO_MAX,

	CTL_IPP_BUF_MO_DBG_1,
	CTL_IPP_BUF_MO_DBG_MAX,

	CTL_IPP_BUF_MV_1,
	CTL_IPP_BUF_MV_2,
	CTL_IPP_BUF_MV_3,
	CTL_IPP_BUF_MV_MAX,

	CTL_IPP_BUF_MV_DBG_1,
	CTL_IPP_BUF_MV_DBG_MAX,

	CTL_IPP_BUF_DFG_1,
	CTL_IPP_BUF_DFG_2,
	CTL_IPP_BUF_DFG_3,
	CTL_IPP_BUF_DFG_MAX,

	CTL_IPP_BUF_DFG_DBG_1,
	CTL_IPP_BUF_DFG_DBG_MAX,

	CTL_IPP_BUF_LCE_1,
	CTL_IPP_BUF_LCE_2,
	CTL_IPP_BUF_LCE_3,
	CTL_IPP_BUF_LCE_MAX,

	CTL_IPP_BUF_LCE_DBG_1,
	CTL_IPP_BUF_LCE_DBG_MAX,

	CTL_IPP_BUF_PM_1,
	CTL_IPP_BUF_PM_2,
	CTL_IPP_BUF_PM_3,
	CTL_IPP_BUF_PM_MAX,

	CTL_IPP_BUF_PM_DBG_1,
	CTL_IPP_BUF_PM_DBG_MAX,

	CTL_IPP_BUF_YOUT_1,
	CTL_IPP_BUF_YOUT_2,
	CTL_IPP_BUF_YOUT_3,
	CTL_IPP_BUF_YOUT_4,
	CTL_IPP_BUF_YOUT_MAX,

	CTL_IPP_BUF_YOUT_DBG_1,
	CTL_IPP_BUF_YOUT_DBG_MAX,

	CTL_IPP_BUF_MS_ROI_1,
	CTL_IPP_BUF_MS_ROI_2,
	CTL_IPP_BUF_MS_ROI_3,
	CTL_IPP_BUF_MS_ROI_MAX,

	CTL_IPP_BUF_MS_ROI_DBG_1,
	CTL_IPP_BUF_MS_ROI_DBG_MAX,

	CTL_IPP_BUF_MS_1,
	CTL_IPP_BUF_MS_2,
	CTL_IPP_BUF_MS_3,
	CTL_IPP_BUF_MS_MAX,

	CTL_IPP_BUF_MS_DBG_1,
	CTL_IPP_BUF_MS_DBG_MAX,

	CTL_IPP_BUF_3DNR_STA_1,
	CTL_IPP_BUF_3DNR_STA_2,
	CTL_IPP_BUF_3DNR_STA_3,
	CTL_IPP_BUF_3DNR_STA_MAX,

	CTL_IPP_BUF_3DNR_STA_DBG_1,
	CTL_IPP_BUF_3DNR_STA_DBG_MAX,

	CTL_IPP_BUF_WDR_1,
	CTL_IPP_BUF_WDR_2,
	CTL_IPP_BUF_WDR_3,
	CTL_IPP_BUF_WDR_MAX,

	CTL_IPP_BUF_WDR_DBG_1,
	CTL_IPP_BUF_WDR_DBG_MAX,

	CTL_IPP_BUF_IFE_VA_1,
	CTL_IPP_BUF_IFE_VA_2,
	CTL_IPP_BUF_IFE_VA_3,
	CTL_IPP_BUF_IFE_VA_MAX,

	CTL_IPP_BUF_IFE_VA_DBG_1,
	CTL_IPP_BUF_IFE_VA_DBG_MAX,

	CTL_IPP_BUF_IPE_VA_1,
	CTL_IPP_BUF_IPE_VA_2,
	CTL_IPP_BUF_IPE_VA_3,
	CTL_IPP_BUF_IPE_VA_MAX,

	CTL_IPP_BUF_IPE_VA_DBG_1,
	CTL_IPP_BUF_IPE_VA_DBG_MAX,

	CTL_IPP_BUF_IME_VA_1,
	CTL_IPP_BUF_IME_VA_2,
	CTL_IPP_BUF_IME_VA_3,
	CTL_IPP_BUF_IME_VA_MAX,

	CTL_IPP_BUF_IME_VA_DBG_1,
	CTL_IPP_BUF_IME_VA_DBG_MAX,

	CTL_IPP_BUF_FCVG_1,
	CTL_IPP_BUF_FCVG_2,
	CTL_IPP_BUF_FCVG_3,
	CTL_IPP_BUF_FCVG_MAX,

	CTL_IPP_BUF_FCVG_DBG_1,
	CTL_IPP_BUF_FCVG_DBG_MAX,

	CTL_IPP_DBG_BUF_MAX_CNT,
} CTL_IPP_DBG_PRI_BUF_IDX;

#define CTL_IPP_BUF_INFO_DESC_LEN 32
typedef struct _CTL_IPP_BUF_INFO {
	UINT8 used;
	UINT32 bid;
	NVTMPP_VB_BLK blk;
	CTL_IPP_BUF_ADDR addr;
	CTL_IPP_BUF_TYPE buf_type;
	UINT32 buf_size;
	UINT32 lock_cnt;
	CTL_IPP_HANDLE *p_ctrl_hdl;
	CTL_IPP_INFO_LIST_ITEM *p_ctrl_info;
	CTL_IPP_OUT_BUF_INFO *p_out_buf_info;
	CHAR desc[CTL_IPP_BUF_INFO_DESC_LEN];
	CHAR hdl_name[CTL_IPP_HANDLE_NAME_MAX];
	UINT64 frm_cnt;
	UINT32 evt_cnt;
} CTL_IPP_BUF_INFO;

typedef struct _CTL_IPP_BUF_ALLOC_INFO {
	UINT32 buf_size;
	CHAR desc[CTL_IPP_BUF_INFO_DESC_LEN];
	CTL_IPP_BUF_TYPE buf_type;
	CTL_IPP_HANDLE *p_ctrl_hdl;
	CTL_IPP_INFO_LIST_ITEM *p_ctrl_info;
	CTL_IPP_OUT_BUF_INFO *p_out_buf_info;
} CTL_IPP_BUF_ALLOC_INFO;

ER ctl_ipp_buf_msg_snd(UINT32 cmd, ULONG p1, ULONG p2, ULONG p3);
ER ctl_ipp_buf_msg_rcv(UINT32 *cmd, ULONG *p1, ULONG *p2, ULONG *p3);
ER ctl_ipp_buf_msg_flush(void);
ER ctl_ipp_buf_erase_queue(CTL_IPP_HANDLE *p_hdl);
ER ctl_ipp_buf_erase_path_in_queue(CTL_IPP_HANDLE *p_hdl, UINT32 pid);
ER ctl_ipp_buf_msg_reset_queue(void);
UINT32 ctl_ipp_buf_get_free_queue_num(void);
void ctl_ipp_buf_msg_dump(int (*dump)(const char *fmt, ...));
void ctl_ipp_buf_info_dump(int (*dump)(const char *fmt, ...));

/**********************************/
/*	ctl_ipp_buf task ctrl		  */
/**********************************/
ER ctl_ipp_buf_open_tsk(void);
ER ctl_ipp_buf_close_tsk(void);
ER ctl_ipp_buf_set_resume(BOOL b_flush_evt);
ER ctl_ipp_buf_set_pause(BOOL b_wait_end, BOOL b_flush_evt);
ER ctl_ipp_buf_wait_pause_end(void);
CTL_IPP_BUF_INFO *ctl_ipp_buf_get_info_by_addr(CTL_IPP_BUF_ADDR addr, CTL_IPP_BUF_TYPE buf_type);
CTL_IPP_BUF_INFO *ctl_ipp_buf_info_new(CTL_IPP_BUF_ALLOC_INFO *p_alloc_info, const CHAR *info_str);
INT32 ctl_ipp_buf_info_push(CTL_IPP_BUF_INFO *p_buf_info, const CHAR *info_str);
INT32 ctl_ipp_buf_info_lock(CTL_IPP_BUF_INFO *p_buf_info, const CHAR *info_str);
INT32 ctl_ipp_buf_info_unlock(CTL_IPP_BUF_INFO *p_buf_info, const CHAR *info_str);
INT32 ctl_ipp_buf_info_release(CTL_IPP_BUF_INFO *p_buf_info, const CHAR *info_str);
void ctl_ipp_buf_push(IPP_EVENT_FP bufio_fp, CTL_IPP_INFO_LIST_ITEM *ctrl_info, UINT32 rev);
void ctl_ipp_buf_push_low_delay(IPP_EVENT_FP bufio_fp, CTL_IPP_INFO_LIST_ITEM *ctrl_info, UINT64 timestamp);
void ctl_ipp_buf_push_slice(IPP_EVENT_FP bufio_fp, CTL_IPP_INFO_LIST_ITEM *ctrl_info, UINT32 slice_idx);
void ctl_ipp_buf_release(IPP_EVENT_FP bufio_fp, CTL_IPP_INFO_LIST_ITEM *ctrl_info, UINT32 bufio_stop, INT32 err_msg);
void ctl_ipp_buf_release_path(IPP_EVENT_FP bufio_fp, CTL_IPP_INFO_LIST_ITEM *ctrl_info, UINT32 bufio_stop, UINT32 pid, INT32 err_msg);
void ctl_ipp_buf_iostop_path(IPP_EVENT_FP bufio_fp, CTL_IPP_HANDLE *p_hdl, UINT32 pid);
void ctl_ipp_buf_iostart_path(IPP_EVENT_FP bufio_fp, CTL_IPP_HANDLE *p_hdl, UINT32 pid);
void ctl_ipp_buf_frm_end(IPP_EVENT_FP bufio_fp, CTL_IPP_INFO_LIST_ITEM *ctrl_info, UINT32 rev);
void ctl_ipp_buf_frm_start(IPP_EVENT_FP bufio_fp, CTL_IPP_INFO_LIST_ITEM *ctrl_info, UINT32 rev);

/**********************************/
/*	ctl_ipp_buf					  */
/**********************************/

/**
	Init buffer information
*/
INT32 ctl_ipp_buf_init(UINT32 num, CTL_IPP_BUF_ADDR buf_addr, UINT32 is_query);

/**
	Uninit buffer information
*/
void ctl_ipp_buf_uninit(void);

/**
	Get buffer from bufio_cb
*/
void ctl_ipp_buf_alloc(CTL_IPP_HANDLE *ctrl_hdl, CTL_IPP_INFO_LIST_ITEM *ctrl_info);

/**
	Get private buffer
*/
void ctl_ipp_buf_pri_alloc(CTL_IPP_HANDLE *ctrl_hdl, CTL_IPP_INFO_LIST_ITEM *ctrl_info);

/**
	Get private buffer(for common task)
*/
CTL_IPP_BUF_ADDR ctl_ipp_buf_task_alloc(CTRL_IPP_BUF_ITEM item, CTL_IPP_INFO_LIST_ITEM *p_ctrl_info, CTL_IPP_BUF_ADDR out_addr, USIZE out);

/**
	search private buffer
*/
CTL_IPP_BUF_ADDR ctl_ipp_buf_pri_search(CTL_IPP_HANDLE *ctrl_hdl, CTL_IPP_BUF_ADDR last_rdy_addr, UINT32 buf_item, CTL_IPP_BUF_PRI_LOF_INFO *lofs_info);

/**
	Iint private buffer information
*/
void ctl_ipp_buf_pri_init(CTL_IPP_BUF_PRI_INFO *p_buf_info);

/**
	buf io fp wrapper
*/
void ctl_ipp_buf_fp_wrapper(CTL_IPP_HANDLE *p_hdl, IPP_EVENT_FP bufio_fp, CTL_IPP_BUF_IO_CFG cfg, CTL_IPP_OUT_BUF_INFO *p_buf, CTL_IPP_INFO_LIST_ITEM *ctrl_info);

#endif // _CTL_IPP_BUF_INT_H
