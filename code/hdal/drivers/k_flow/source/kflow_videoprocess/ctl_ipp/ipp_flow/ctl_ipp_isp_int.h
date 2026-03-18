/**
    IPL Ctrl Layer, ISP internal used api

    @file       ctl_ipp_isp_int.h
    @ingroup    mIIPL_Ctrl
    @note       None

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/


#ifndef _CTL_IPP_ISP_INT_H
#define _CTL_IPP_ISP_INT_H

#include "kflow_common/isp_if.h"
#include "ctl_ipp_isp.h"
#include "ctl_ipp_util_int.h"
#include "ctl_ipp_int.h"
#include "ipp_debug_int.h"

#define CTL_IPP_ISP_IQ_3DNR_STA_MAX_NUM	(8192)

#define CTL_IPP_ISP_HANDLE_MAX_NUMBER	(4)
#define CTL_IPP_ISP_HANDLE_NAME_MAX_LENGTH	(32)
#define CTL_IPP_ISP_HANDLE_STATUS_FREE		(0)
#define CTL_IPP_ISP_HANDLE_STATUS_USED		(1)

#define CTL_IPP_ISP_YUVOUT_DEBUG_EN		(DISABLE)
#define CTL_IPP_ISP_YUVOUT_DEBUG_NUM	(16)

#define CTL_IPP_ISP_CBTIME_DEBUG_EN		(ENABLE)
#define CTL_IPP_ISP_CBTIME_DEBUG_NUM	(30)	// max log number
#define CTL_IPP_ISP_CBTIME_DEBUG_PRINT	(16) 	// default print log number

#define CTL_IPP_ISP_ID_UNUSED	(0xFFFFFFFF)	// isp info not used yet

typedef INT32(*CTL_IPP_ISP_SET_FP)(ISP_ID id, CTL_IPP_ISP_ITEM item, void *data);
typedef INT32(*CTL_IPP_ISP_GET_FP)(ISP_ID id, CTL_IPP_ISP_ITEM item, void *data);

typedef struct {
	UINT32 ispid;
	ISP_EVENT evt;
	UINT32 raw_fc;
	UINT64 ts_start;
	UINT64 ts_end;
} CTL_IPP_ISP_CBTIME_LOG;

typedef struct {
	UINT32 ispid;
	UINT32 pid;
	UINT32 buf_id;
	ULONG buf_addr;
	UINT32 buf_size;
	UINT64 lock_ts;
	UINT64 unlock_ts;
} CTL_IPP_ISP_YUVOUT_LOG;

typedef struct {
	CHAR name[CTL_IPP_ISP_HANDLE_NAME_MAX_LENGTH];
	ISP_EVENT_FP fp;
	ISP_EVENT evt;
	UINT32 status;
	CTL_IPP_ISP_CB_MSG cb_msg;
} CTL_IPP_ISP_HANDLE;

typedef struct {
	KDRV_IFE_IQ_CFG ife;
	KDRV_IPE_IQ_CFG ipe;
	KDRV_IME_IQ_CFG ime;
} CTL_IPP_ISP_IQ_PARA;

typedef struct {
	UINT32 isp_id;
	CTL_IPP_INFO_LIST_ITEM *p_ctrl_info;
	CTL_IPP_ISP_IFE_VIG_CENT_RATIO ife_vig_cent_ratio;
	CTL_IPP_ISP_IME_LCA_DBG_X_POS_RATIO ime_lca_dbg_x_pos_ratio;

	// information storage used to input kdrv/eng
	CTL_IPP_ISP_ETH cur_eth;
	CTL_IPP_ISP_3DNR_STA cur_3dnr_sta;

	// information storage used to output user
	CTL_IPP_ISP_ETH rdy_eth;
	CTL_IPP_ISP_3DNR_STA rdy_3dnr_sta;
	CTL_IPP_ISP_IFE_VA_RST rdy_ife_va_rst;
	CTL_IPP_ISP_IFE_VA_INDEP_RST rdy_ife_va_indep_rst;
	CTL_IPP_ISP_IPE_VA_RST rdy_ipe_va_rst;
	CTL_IPP_ISP_IPE_VA_INDEP_RST rdy_ipe_va_indep_rst;
	CTL_IPP_ISP_IME_VA_RST rdy_ime_va_rst;
	CTL_IPP_ISP_IME_VA_INDEP_RST rdy_ime_va_indep_rst;
	CTL_IPP_ISP_IFE_HIST_RST rdy_ife_hist_rst;
	CTL_IPP_ISP_EDGE_STCS rdy_ipe_edge_stcs;
	CTL_IPP_ISP_DEFOG_STCS rdy_defog_stcs;

	CTL_IPP_ISP_STRP_INFO stripe_info;
	CTL_IPP_ISP_FPN_INFO fpn_info;
	CTL_IPP_ISP_IQ_PARA iq_para;
} CTL_IPP_ISP_INT_INFO;

/*
	get yuv out information for ipp_buf_task and ipp_isp
*/
typedef struct {
	CTL_IPP_HANDLE *p_hdl;
	CTL_IPP_ISP_YUV_OUT yuv_out;
} CTL_IPP_ISP_YUV_OUT_INFO;

typedef enum {
	CTL_IPP_ISP_INT_EVT_TRIG_START = 0,
	CTL_IPP_ISP_INT_EVT_TRIG_END,
	CTL_IPP_ISP_INT_EVT_MAX
} CTL_IPP_ISP_INT_EVENT;

UINT32 ctl_ipp_isp_init(UINT32 num, CTL_IPP_BUF_ADDR buf_addr, UINT32 is_query);
INT32 ctl_ipp_isp_uninit(void);
INT32 ctl_ipp_isp_alloc_int_info(UINT32 id);
INT32 ctl_ipp_isp_release_int_info(UINT32 id);
INT32 ctl_ipp_isp_update_int_info(UINT32 old_id, UINT32 new_id);
INT32 ctl_ipp_isp_event_cb(ISP_ID id, ISP_EVENT evt, CTL_IPP_INFO_LIST_ITEM *p_info, void *param);
INT32 ctl_ipp_isp_event_cb_proc(ISP_ID id, ISP_EVENT evt, UINT32 raw_frame_cnt);
void ctl_ipp_isp_int_set_dft_param_all(CTL_IPP_HANDLE *p_hdl, CTL_IPP_ISP_INT_INFO *p_info);
INT32 ctl_ipp_isp_int_upd(CTL_IPP_HANDLE *p_hdl, CTL_IPP_ISP_ITEM item, void *data);
INT32 ctl_ipp_isp_int_get(CTL_IPP_HANDLE *p_hdl, CTL_IPP_ISP_ITEM item, void *data);
INT32 ctl_ipp_isp_int_set_yuv_out(CTL_IPP_HANDLE *p_hdl, CTL_IPP_OUT_BUF_INFO *p_buf);
UINT32 ctl_ipp_isp_int_id_validate(UINT32 id);
void ctl_ipp_isp_dump(int (*dump)(const char *fmt, ...));
void ctl_ipp_isp_dbg_cbtime_set_threshold(UINT32 threshold_us, UINT32 max_catch_number);
void ctl_ipp_isp_dbg_cbtime_dump(int (*dump)(const char *fmt, ...), BOOL print_all);
void ctl_ipp_isp_drv_init(void);
void ctl_ipp_isp_drv_uninit(void);
ER ctl_ipp_isp_evt_fp_dbg_mode(UINT32 dbg_mode);
CTL_IPP_ISP_INT_INFO *ctl_ipp_isp_get_int_info(UINT32 id);


/**********************************/
/*	ctl_ipp_isp message queue	  */
/**********************************/
#define CTL_IPP_ISP_MSG_STS_FREE    0x00000000
#define CTL_IPP_ISP_MSG_STS_LOCK    0x00000001

#define CTL_IPP_ISP_MSG_IGNORE      0x00000000
#define CTL_IPP_ISP_MSG_PROCESS     0x00000001	//[0]: ID [1]: EVENT, [2]: RAW_FRAME_COUNT
#define CTL_IPP_ISP_MSG_DROP		0x00000002  //[0]: ID [1]: EVENT, [2]: RAW_FRAME_COUNT

typedef struct {
	UINT32 cmd;
	ULONG param[5];
	UINT32 rev[2];
	CTL_IPP_LIST_HEAD list;
} CTL_IPP_ISP_MSG_EVENT;

ER ctl_ipp_isp_msg_snd(UINT32 cmd, ULONG p1, ULONG p2, ULONG p3);
ER ctl_ipp_isp_msg_rcv(UINT32 *cmd, ULONG *p1, ULONG *p2, ULONG *p3);
ER ctl_ipp_isp_msg_flush(void);
ER ctl_ipp_isp_erase_queue(ULONG id);
ER ctl_ipp_isp_msg_reset_queue(void);
ER ctl_ipp_isp_msg_snd_chkdrop(UINT32 msg, ULONG addr);
UINT32 ctl_ipp_isp_get_free_queue_num(void);

/**********************************/
/*	ctl_ipp_isp task ctrl		  */
/**********************************/
ER ctl_ipp_isp_open_tsk(void);
ER ctl_ipp_isp_close_tsk(void);
ER ctl_ipp_isp_set_resume(BOOL b_flush_evt);
ER ctl_ipp_isp_set_pause(BOOL b_wait_end, BOOL b_flush_evt);
ER ctl_ipp_isp_wait_pause_end(void);
ER ctl_ipp_isp_wait_process_end(void);
void ctl_ipp_isp_process(UINT32 id, UINT32 event, UINT32 raw_frame_cnt);
void ctl_ipp_isp_drop(UINT32 id, UINT32 event, UINT32 raw_frame_cnt);
void ctl_ipp_isp_task_dumpinfo(int (*dump)(const char *fmt, ...));

#endif

