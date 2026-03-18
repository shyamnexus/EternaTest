/**
    IPL Ctrl Layer

    @file       ctl_ipp_int.h
    @ingroup    mIIPL_Ctrl
    @note       None

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/


#ifndef _CTL_IPP_INT_H
#define _CTL_IPP_INT_H

#include "ctl_ipp_util_int.h"
#include "ctl_ipp.h"

/*
	support two direct mode (2 sie direct to 1 ipp)
*/
#define CTL_IPP_TWO_DIRECT_ENABLE (DISABLE)

/*
	RAW encode/decode ratio
	520/560: 58%
	530/538/567: 50%
	690: 41%
*/
#define CTL_IPP_RAW_ENC_RATE 50

#define CTL_IPP_BUF_3DNR_NUM 2
#define CTL_IPP_BUF_3DNR_STA_NUM 1
#define CTL_IPP_BUF_DFG_NUM 2
#define CTL_IPP_BUF_WDR_NUM 2
#define CTL_IPP_BUF_PM_NUM 3
#define CTL_IPP_BUF_YOUT_NUM 4
#define CTL_IPP_BUF_VA_NUM 2
#define CTL_IPP_BUF_DBG_NUM 1
#define CTL_IPP_BUF_BNR_NUM 2
#define CTL_IPP_BUF_BNR_STA_NUM 1
#define CTL_IPP_BUF_COMM_BLK_NUM 1	// subout buffer which come from common blk. dont allocate as private buf. use for debug dumpinfo

#if defined(CONFIG_NVT_FPGA_EMULATION) || defined(_NVT_FPGA_)
#define CTL_IPP_HANDLE_LOCK_TIMEOUT_MS	(5000)
#else
#define CTL_IPP_HANDLE_LOCK_TIMEOUT_MS	(1000)
#endif

#define CTL_IPP_SLICE_HDL_NUM 2

typedef struct _CTL_IPP_HANDLE CTL_IPP_HANDLE;
typedef struct _CTL_IPP_DBG_TS_NODE CTL_IPP_DBG_TS_NODE;
typedef struct _IPP_EVENT_ROOT_ITEM IPP_EVENT_ROOT_ITEM;

typedef enum {
	CTRL_IPP_ENG_IFE = 0X00000001,
	CTRL_IPP_ENG_DCE = 0X00000002,
	CTRL_IPP_ENG_IPE = 0X00000004,
	CTRL_IPP_ENG_IME = 0X00000008,
	CTRL_IPP_ENG_PRE = 0X00000010,
	CTRL_IPP_ENG_ALL = CTRL_IPP_ENG_IFE | CTRL_IPP_ENG_DCE | CTRL_IPP_ENG_IPE | CTRL_IPP_ENG_IME | CTRL_IPP_ENG_PRE,
} CTRL_IPP_ENG;
/**********************************/
/*  ctl_ipl message queue         */
/**********************************/
#define CTL_IPP_MSG_STS_FREE    0x00000000
#define CTL_IPP_MSG_STS_LOCK    0x00000001

#define CTL_IPP_MSG_IGNORE					0x00000000
#define CTL_IPP_MSG_PROCESS					0x00000001  //[0]: handle, [1]: header address, [2]: buf_id(for unit release buffer)
#define CTL_IPP_MSG_DROP					0x00000002  //[0]: handle, [1]: header address, [2]: buf_id(for unit release buffer), this cmd is for erase queue used
//#define CTL_IPP_MSG_PROCESS_PATTERN_PASTE	0x00000003  567 remove
#define CTL_IPP_MSG_PROCESS_DIRECT			0x00000004  //[0]: handle, [1]: header address, [2]: direct event

typedef struct {
	UINT32 cmd;
	ULONG param[5];
	UINT32 rev[2];
	IPP_EVENT_FP drop_fp;
	CTL_IPP_LIST_HEAD list;
	UINT32 snd_time;
} CTL_IPP_MSG_EVENT;

ER ctl_ipp_msg_snd(UINT32 cmd, ULONG p1, ULONG p2, ULONG p3, IPP_EVENT_FP drop_fp);
ER ctl_ipp_msg_re_snd(UINT32 cmd, ULONG p1, ULONG p2, ULONG p3, IPP_EVENT_FP drop_fp, UINT32 snd_time);
ER ctl_ipp_msg_rcv(UINT32 *cmd, ULONG *p1, ULONG *p2, ULONG *p3, IPP_EVENT_FP *drop_fp, UINT32 *snd_t);
ER ctl_ipp_msg_flush(void);
ER ctl_ipp_erase_queue(CTL_IPP_HANDLE *p_hdl);
ER ctl_ipp_msg_reset_queue(void);
UINT32 ctl_ipp_get_free_queue_num(void);


/**********************************/
/*  ctl_ipl handle, process       */
/**********************************/
#define CTL_IPP_HANDLE_NAME_MAX             (16)

#define CTL_IPP_HANDLE_STS_MASK             (0x55AA0000)
#define CTL_IPP_HANDLE_STS_FREE             (CTL_IPP_HANDLE_STS_MASK | 0x01)
#define CTL_IPP_HANDLE_STS_READY            (CTL_IPP_HANDLE_STS_MASK | 0x02)
#define CTL_IPP_HANDLE_STS_DIR_START        (CTL_IPP_HANDLE_STS_MASK | 0x11)
#define CTL_IPP_HANDLE_STS_DIR_READY        (CTL_IPP_HANDLE_STS_MASK | 0x12)
#define CTL_IPP_HANDLE_STS_DIR_STOP_S1      (CTL_IPP_HANDLE_STS_MASK | 0x13)
#define CTL_IPP_HANDLE_STS_DIR_STOP_S2      (CTL_IPP_HANDLE_STS_MASK | 0x14)
#define CTL_IPP_HANDLE_STS_DIR_SLEEP      	(CTL_IPP_HANDLE_STS_MASK | 0x15)
#define CTL_IPP_HANDLE_STS_DIR_STOP_S3      (CTL_IPP_HANDLE_STS_MASK | 0x16)
#define CTL_IPP_HANDLE_STS_DIR_TSK_IDLE     (CTL_IPP_HANDLE_STS_MASK | 0x17)
#define CTL_IPP_HANDLE_STS_DIR_TSK_PROC     (CTL_IPP_HANDLE_STS_MASK | 0x18)

#define CTL_IPP_HANDLE_3DNR_WRN_LOG_BIT	(0x00000001)

#define CTL_IPP_EVT_TAG_NONE					(0x00000000)
#define CTL_IPP_EVT_TAG_SUBOUT_FOR_CAP			(0x00000001)
#define CTL_IPP_EVT_TAG_MAINFRM_FOR_CAP			(0x00000002)
#define CTL_IPP_EVT_TAG_SKIP_ISP_FOR_FASTBOOT	(0x00000003)

#define CTL_IPP_TPLNR_REF_BUF_WAIT_PUSH		(0x1)
#define CTL_IPP_TPLNR_REF_BUF_WAIT_RELEASE	(0x2)

#define CTL_IPP_DIR_DROP	(0x5A5A0000)

#define CTL_IPP_SLICE_IDX_FRM_START			(0xFF)
#define CTL_IPP_SLICE_IDX_FRM_END			(0xEE)

#define CTL_IPP_INFO_STS_JOB_TRIG			(1 << 0)
#define CTL_IPP_INFO_STS_JOB_START			(1 << 1)
#define CTL_IPP_INFO_STS_JOB_DONE			(1 << 2)
#define CTL_IPP_INFO_STS_JOB_DROP			(1 << 3)
#define CTL_IPP_INFO_STS_JOB_TRIG_CHK		(CTL_IPP_INFO_STS_JOB_TRIG | CTL_IPP_INFO_STS_JOB_START | CTL_IPP_INFO_STS_JOB_DONE)
#define CTL_IPP_INFO_STS_JOB_TRIG_CHK_START	(CTL_IPP_INFO_STS_JOB_TRIG | CTL_IPP_INFO_STS_JOB_START)
#define CTL_IPP_INFO_STS_JOB_TRIG_CHK_DONE	(CTL_IPP_INFO_STS_JOB_TRIG | CTL_IPP_INFO_STS_JOB_DONE)

typedef struct {
	UINT32 flag;
	UINT32 ptn;
} CTL_IPP_HANDLE_LOCKINFO;

typedef struct {
	INT32 buf_rel_msg;		// save buffer release msg for debug log
	INT32 buf_rel_path_msg;	// save path buffer release msg for debug log
} CTL_IPP_DBG_OUT_INFO;

typedef struct {
	CTL_IPP_BUF_INFO *p_subout_buf;
	CTL_IPP_BUF_ADDR shdr_buf_addr;
	UINT32 shdr_buf_lofs;
	CTL_IPP_BUF_ADDR sigma_buf_addr;
	UINT32 sigma_buf_lofs;
	CTL_IPP_BUF_ADDR gamma_buf_addr;
	UINT32 gamma_buf_lofs;
} CTL_IPP_VDOFRM_META_INFO;

typedef struct {
	CTL_IPP_BASEINFO info;
	CTL_IPP_LIST_HEAD list;
	UINT8 lock_cnt;
	UINT8 idx;
	UINT32 sts;				/* slice mode need stop trigger timer cb when ipp is stop */
	UINT32 info_sts;		/* dual ipp need wait previous frame done before trig next frame to prevent ref in issue */
	CTL_IPP_HANDLE *owner;	/* ipp handle which own this info */
	UINT32 evt_tag;	/* for capture mode */
	UINT8 auto_cap_ref; // indicate if use basic ref path info that auto set by flow for capture reduce buf mode

	VDO_FRAME *p_vdoin_info;	/* normal case: input vdoframe; direct mode: 0 no input vdoframe */
	UINT32 input_buf_id;		/* normal case: input buf id; direct mode: input raw frame count */
	CTL_IPP_BUF_INFO *p_buf_info[CTL_IPP_OUT_PATH_ID_MAX];
	CTL_IPP_OUT_BUF_INFO buf_info[CTL_IPP_OUT_PATH_ID_MAX];
	CTL_IPP_DBG_OUT_INFO dbg_out_info[CTL_IPP_OUT_PATH_ID_MAX];	/* for output path debug usage */
	CTL_IPP_VDOFRM_META_INFO vdofrm_meta;

	UINT32 kdrv_evt_count;
	CTL_IPP_DBG_TS_NODE *p_dbg_ts;	/* debug timestamp log */
	BOOL kdrv_job_is_block;	/* kdrv job is split by size that need to use blocking mode */
	BOOL job_num_added;		/* nodify that ctl_ipp_kdrv_cur_job_num had been increased by this job, and can be decreased later. this prevent dropping frame & decreasing job num before increase job num, which may lead job num already 0 err */
} CTL_IPP_INFO_LIST_ITEM;

typedef struct {
	CTL_IPP_LIST_HEAD free_head;
	CTL_IPP_LIST_HEAD used_head;
	UINT32 count;
	CTL_IPP_INFO_LIST_ITEM *info;
} CTL_IPP_INFO_POOL;

typedef struct {
	UINT32 lofs;
	UINT32 size;
	CHAR *name;
} CTL_IPP_BUF_PRI_LOF_INFO;

typedef struct {
	CTL_IPP_BUF_PRI_LOF_INFO mo[CTL_IPP_BUF_3DNR_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO mo_dbg[CTL_IPP_BUF_DBG_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO mv[CTL_IPP_BUF_3DNR_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO mv_dbg[CTL_IPP_BUF_DBG_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO defog_subout[CTL_IPP_BUF_DFG_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO defog_subout_dbg[CTL_IPP_BUF_DBG_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO lce[CTL_IPP_BUF_DFG_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO lce_dbg[CTL_IPP_BUF_DBG_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO pm[CTL_IPP_BUF_PM_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO pm_dbg[CTL_IPP_BUF_DBG_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO ms[CTL_IPP_BUF_3DNR_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO ms_dbg[CTL_IPP_BUF_DBG_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO ms_roi[CTL_IPP_BUF_3DNR_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO ms_roi_dbg[CTL_IPP_BUF_DBG_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO _3dnr_fcvg[CTL_IPP_BUF_3DNR_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO _3dnr_fcvg_dbg[CTL_IPP_BUF_DBG_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO _3dnr_sta[CTL_IPP_BUF_3DNR_STA_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO _3dnr_sta_dbg[CTL_IPP_BUF_DBG_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO wdr_subout[CTL_IPP_BUF_WDR_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO wdr_subout_dbg[CTL_IPP_BUF_DBG_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO ife_va[CTL_IPP_BUF_VA_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO ife_va_dbg[CTL_IPP_BUF_DBG_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO ipe_va[CTL_IPP_BUF_VA_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO ipe_va_dbg[CTL_IPP_BUF_DBG_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO ime_va[CTL_IPP_BUF_VA_NUM];
	CTL_IPP_BUF_PRI_LOF_INFO ime_va_dbg[CTL_IPP_BUF_DBG_NUM];

	/* this is used to keep sie yout for isp flow */
	CTL_IPP_BUF_PRI_LOF_INFO yout[CTL_IPP_BUF_YOUT_NUM];
} CTL_IPP_BUF_PRI_ITEM_INFO;

typedef struct {
	CTL_IPP_PRIVATE_BUF buf_info;
	CTL_IPP_BUF_PRI_ITEM_INFO buf_item;
} CTL_IPP_BUF_PRI_INFO;

typedef enum {
	CTL_IPP_OUTBUF_CNT_EVT_NEW = 0,
	CTL_IPP_OUTBUF_CNT_EVT_PUSH,
	CTL_IPP_OUTBUF_CNT_EVT_LOCK,
	CTL_IPP_OUTBUF_CNT_EVT_UNLOCK,
	CTL_IPP_OUTBUF_CNT_EVT_NEW_FAILED,
	CTL_IPP_OUTBUF_CNT_EVT_MAX,
} CTL_IPP_OUTBUF_CNT_EVT;

typedef enum {
	CTL_IPP_INBUF_CNT_EVT_END = 0,
	CTL_IPP_INBUF_CNT_EVT_DROP,
	CTL_IPP_INBUF_CNT_EVT_START,
	CTL_IPP_INBUF_CNT_EVT_DIRECT_END,
	CTL_IPP_INBUF_CNT_EVT_DIRECT_DROP,
	CTL_IPP_INBUF_CNT_EVT_MAX,
} CTL_IPP_INBUF_CNT_EVT;

typedef struct _CTL_IPP_HANDLE {
	CHAR name[CTL_IPP_HANDLE_NAME_MAX];
	UINT32 id;						/* serialized id number for accessing array of handle info */
	UINT32 sts;
	UINT32 dir_prv_sts;				/* direct mode previous status, which used for checking if process event is legal */
	UINT32 dir_tsk_sts;				/* used for checking ctl_ipp_process is done. see CTL_IPP_HANDLE_STS_DIR_TSK_IDLE/CTL_IPP_HANDLE_STS_DIR_TSK_PROC. direct task mode only */
	CTL_IPP_DIRECT_IPP_STS dir_cb_sts; /* used for sie cb check ipp status (ex. is ipp prepared by hdal) */
	UINT64 set_items;				/* record set api called by hdal, to maintain correct get/set order */
	UINT64 get_items;				/* record get api called by hdal, to maintain correct get/set order */
	UINT32 lock;
	UINT32 pro_end_lock;
	vk_spinlock_t spinlock;			/* spinlock for handle, current use to protect info between set_apply and direct_process */
	CTL_IPP_LIST_HEAD list;

	CTL_IPP_FLOW_TYPE flow;
	IPP_EVENT_ROOT_ITEM *ipp_evt_hdl;
	IPP_EVENT_FP evt_outbuf_fp;
	IPP_EVENT_FP evt_inbuf_fp;
	IPP_EVENT_FP dir_sie_cb_fp;		/* direct mode ipp callback to sie. event: CTL_IPP_DIRECT_SIE_EVENT */
	CTL_IPP_FUNC func_en;			/* keep func_en for get_cmd */
	CTL_IPP_BASEINFO ctrl_info;     /* info that is currently used */
	CTL_IPP_BASEINFO rtc_info;      /* info that keep settings from api, wait for set_apply */
	CTL_IPP_INFO_LIST_ITEM *p_last_rdy_ctrl_info;	/* last ready ctrl info, update when proc_en */
	CTL_IPP_INFO_LIST_ITEM *p_last_cfg_ctrl_info;	/* last cfg for direct mode, update when sie bp*/
	CTL_IPP_BUFCFG bufcfg;
	CTL_IPP_BUF_PRI_INFO private_buf;
	BOOL dbg_exb_alloc;				/* dmawp extra buffer allocated for private buffer */
	UINT8 buf_push_order[CTL_IPP_OUT_PATH_ID_MAX];			/* push order of each path */
	UINT8 buf_push_pid_sequence[CTL_IPP_OUT_PATH_ID_MAX];	/* push pid sequence sorted base on order */
	UINT32 buf_io_started[CTL_IPP_OUT_PATH_ID_MAX];	/* 0 --> buf_io_start is not yet called, 1 --> buf_io_start already called */
	UINT32 is_first_handle;			/* first time ipp open in fastboot flow, for special buffer control */

	/* ideal case
		ctl_snd_evt_cnt = proc_end_cnt + drop_cnt
	*/
	atomic_t ctl_frm_str_cnt;	/* ife frame start count */
	atomic_t ctl_snd_evt_cnt;	/* ctl send event count */
	atomic_t ctl_resnd_evt_cnt;	/* ctl re-send event count */
	atomic_t proc_end_cnt;		/* process end count */
	atomic_t drop_cnt;			/* drop frame count, include ctl_drop, kdrv_drop */
	atomic_t kdrv_snd_evt_cnt;	/* kdrv send event count, for predict apply frame */
	atomic_t kdrv_out_evt_cnt;	/* kdrv done event count */

	atomic_t buf_push_evt_cnt;	/* push event in buf task queue, for hdal to drop input */
	atomic_t dir_in_rel_cnt;	/* direct mode input buffer release count(CTL_IPP_DIRECT_IN_RE) */
	atomic_t dir_in_drop_cnt;	/* direct mode input buffer drop count(CTL_IPP_DIRECT_IN_DROP) */
	atomic_t dir_skip_cnt;		/* direct mode skip process cfg count(CTL_IPP_DIRECT_SKIP) */
	atomic_t in_buf_op_cnt[CTL_IPP_INBUF_CNT_EVT_MAX];								/* input buffer cnt for end, drop, direct_end, direct_drop */
	atomic_t out_buf_op_cnt[CTL_IPP_OUT_PATH_ID_MAX][CTL_IPP_OUTBUF_CNT_EVT_MAX];	/* output buffer cnt for new, push, lock, unlock, new_failed */

	UINT32 snd_evt_time;
	UINT32 rev_evt_time;

	UINT32 isp_id[CTL_IPP_ALGID_MAX];			/* isp id(iq, wdr_lib...) */

	/* Direct mode ipp bind sie_id */
	CTL_IPP_SIE_ID dir_sie_id[CTL_IPP_HDR_MAX_FRAME_NUM][CTL_IPP_COMB_NUM_MAX];

	/* Direct mode engine id number of max engine number (CTL_IPP_ENG_NUM) */
	UINT32 dir_eng_id;

	/* Dual direct mode shared same engine id for two ipp handle */
	UINT32 two_dir_eng_id;

	/* debug info */
	UINT32 last_3dnr_disable_frm;	/* latest frame number that disable 3dnr */
	CTL_IPP_INFO_LIST_ITEM *p_last_rdy_ctrl_info_dbg; /* last ready ctrl info for debug */

	/* kdrv use */
	UINT32 dev_id;
	void *node;

	/* slice push */
	TIMER_ID slice_timer;
	UINT32 slice_timer_interval;
	CTL_IPP_INFO_LIST_ITEM *p_slice_push_info;
	UINT8 slice_cnt;
	UINT8 slice_path_bit;
	atomic_t slice_cur_idx;

	/* sie rowtime (for checking ife buf overflow. direct mode only) */
	UINT32 sie_rowtime;

	/* AI info */
	CTL_IPP_AI_CB ai_cb;

	/* fastboot dtsi info */
	CTL_IPP_FBOOT_DTSI_INFO dtsi_info;
} CTL_IPP_HANDLE;

typedef struct {
	CTL_IPP_LIST_HEAD free_head;
	CTL_IPP_LIST_HEAD used_head;
	UINT32 flg_num;
	ID *flg_id;
	UINT32 pro_end_flg_num;
	ID *pro_end_flg_id;
	UINT32 handle_num;
	CTL_IPP_HANDLE *handle;
} CTL_IPP_HANDLE_POOL;

typedef enum {
	CTL_IPP_ISP_TYPE_IQ_TRIG = 0,
	CTL_IPP_ISP_TYPE_IQ_GET,
	CTL_IPP_ISP_TYPE_MAX,
} CTL_IPP_ISP_TYPE;

typedef struct {
	UINT32 job_head_num;
	UINT32 job_cfg_num;
	UINT32 ll_blk_num;
	vk_spinlock_t spinlock;	/* spinlock for handle, current use to protect info between set_apply and direct_process */
} CTL_IPP_INT_INFO;

/*
	IPP run-time change into
*/
typedef struct {
	UINT8 used;
	CTL_IPP_HANDLE *hdl;
	CTL_IPP_AI_INFO ai_info;
} CTL_IPP_RTC_INFO;

/* resv1 */
#define CTL_IPP_VDOFRM_RESV1_MSK_PIX_YCC(resv1)		((resv1) & VDO_PIX_YCC_MASK)		// bit 12..15
/* reserved[7] */
#define CTL_IPP_VDOFRM_RES_MSK_ENCRATE(reserved)	(((reserved) >> 0) & 0x0000007f)	// bit 0..6
#define CTL_IPP_VDOFRM_RES_MSK_ENCAGGR(reserved)	(((reserved) >> 7) & 0x00000001)	// bit 7

// direct info for ctl_ipp_process_direct
typedef struct {
	BOOL is_set[CTL_IPP_DIR_INFO_NUM];
	CTL_IPP_DIRECT_EVENT event[CTL_IPP_ENG_NUM]; // queue sie cb event for dual direct progressive
	UINT32 queue_num[CTL_IPP_DIR_INFO_NUM]; // if there is job in queue for dual direct progressive
	CTL_IPP_DIRECT_CB_FROM_SIE_INFO cb_info[CTL_IPP_DIR_INFO_NUM];
} CTL_IPP_DIRECT_PROC_INFO;

typedef INT32(*CTL_IPP_IOCTL_FP)(CTL_IPP_HANDLE *, void *);
typedef INT32(*CTL_IPP_SET_FP)(CTL_IPP_HANDLE *, void *);
typedef INT32(*CTL_IPP_GET_FP)(CTL_IPP_HANDLE *, void *);
typedef void (*CTL_IPP_TIMER_FP)(ULONG event);

void ctl_ipp_handle_reset(CTL_IPP_HANDLE *p_hdl);
void ctl_ipp_handle_lock(UINT32 lock);
void ctl_ipp_handle_unlock(UINT32 lock);
void ctl_ipp_handle_wait_proc_end(UINT32 lock, BOOL bclr_flag, BOOL b_timeout_en);
void ctl_ipp_handle_set_proc_end(UINT32 lock);
ER ctl_ipp_handle_wait_cfg_end(UINT32 lock, BOOL bclr_flag, BOOL b_timeout_en);
void ctl_ipp_handle_set_cfg_end(UINT32 lock);
ER ctl_ipp_handle_wait_trig_start(UINT32 lock, BOOL bclr_flag, BOOL b_timeout_en);
void ctl_ipp_handle_set_trig_start(UINT32 lock);
INT32 ctl_ipp_handle_validate(CTL_IPP_HANDLE *p_hdl, const CHAR *func_name);
CTL_IPP_HANDLE *ctl_ipp_get_hdl_by_ispid(UINT32 id);
UINT32 ctl_ipp_get_hdl_sts_by_ispid(UINT32 id);
INT32 ctl_ipp_process(CTL_IPP_HANDLE *p_hdl, VDO_FRAME *p_vdoin_info, UINT32 buf_id, UINT32 snd_time, UINT32 proc_cmd, UINT32 dir_evt, CTL_IPP_DIRECT_PROC_INFO *p_proc_info);
INT32 ctl_ipp_drop_frm(CTL_IPP_HANDLE *p_hdl, VDO_FRAME *p_vdoin_info, UINT32 in_buf_id, CTL_IPP_INFO_LIST_ITEM *p_ctrl_info, INT32 err_msg);
ER ctl_ipp_kdrv_job_add(BOOL is_blocking);
ER ctl_ipp_kdrv_job_free(BOOL is_blocking);
void ctl_ipp_hdl_wait_trig_clr(void);
void ctl_ipp_hdl_wait_trig_set(UINT32 id);
UINT8 ctl_ipp_hdl_wait_trig_get(UINT32 id);
INT32 ctl_ipp_parse_vdofrm_meta(VDO_FRAME *p_vdoin_info, CTL_IPP_INFO_LIST_ITEM *p_ctrl_info);

UINT32 ctl_ipp_info_pool_init(UINT32 num, CTL_IPP_BUF_ADDR buf_addr, UINT32 is_query);
void ctl_ipp_info_pool_free(void);
void ctl_ipp_info_release(CTL_IPP_INFO_LIST_ITEM *p_info);
CTL_IPP_INFO_LIST_ITEM *ctl_ipp_info_get_entry_by_baseinfo(CTL_IPP_BASEINFO *p_base);
CTL_IPP_INFO_LIST_ITEM *ctl_ipp_info_alloc(CTL_IPP_HANDLE *owner);
void ctl_ipp_info_lock(CTL_IPP_INFO_LIST_ITEM *p_info);
void ctl_ipp_info_set_sts(CTL_IPP_INFO_LIST_ITEM *p_info, UINT32 sts);
BOOL ctl_ipp_info_trig_check(CTL_IPP_HANDLE *p_hdl);
void ctl_ipp_info_dump(int (*dump)(const char *fmt, ...));

CTL_IPP_INT_INFO *ctl_ipp_get_int_info(void);
INT32 ctl_ipp_get_stripe_result(CTL_IPP_HANDLE *p_hdl, CTL_IPP_BASEINFO *p_base);
INT32 ctl_ipp_int_ime_path_adj(CTL_IPP_HANDLE *p_hdl, CTL_IPP_BASEINFO *p_base, const CHAR *info_str);
INT32 ctl_ipp_process_raw(CTL_IPP_HANDLE *p_hdl, VDO_FRAME *p_vdoin_info, UINT32 buf_id);
INT32 ctl_ipp_process_ccir(CTL_IPP_HANDLE *p_hdl, VDO_FRAME *p_vdoin_info, UINT32 buf_id);
INT32 ctl_ipp_process_ime_d2d(CTL_IPP_HANDLE *p_hdl, VDO_FRAME *p_vdoin_info, UINT32 buf_id);
INT32 ctl_ipp_process_ipe_d2d(CTL_IPP_HANDLE *p_hdl, VDO_FRAME *p_vdoin_info, UINT32 buf_id);
INT32 ctl_ipp_process_pre_d2d(CTL_IPP_HANDLE *p_hdl, VDO_FRAME *p_vdoin_info, UINT32 buf_id);
INT32 ctl_ipp_process_direct(CTL_IPP_HANDLE *p_hdl, CTL_IPP_DIRECT_PROC_INFO *p_proc_info, UINT32 buf_id);
INT32 ctl_ipp_process_rawcolor(CTL_IPP_HANDLE *p_hdl, VDO_FRAME *p_vdoin_info, UINT32 buf_id);

INT32 ctl_ipp_datastamp_cb(CTL_IPP_BASEINFO *p_base, CTL_IPP_DS_CB_INPUT_INFO *in, CTL_IPP_DS_CB_OUTPUT_INFO *out);
INT32 ctl_ipp_primask_cb(CTL_IPP_BASEINFO *p_base, CTL_IPP_PM_CB_INPUT_INFO *in, CTL_IPP_PM_CB_OUTPUT_INFO *out);
INT32 ctl_ipp_preset_cb(CTL_IPP_BASEINFO *p_base);
INT32 ctl_ipp_kdrv_job_cb(VOID *callback_info, VOID *user_data);
void ctl_ipp_slice_timer_cb0(ULONG event);
void ctl_ipp_slice_timer_cb1(ULONG event);

void ctl_ipp_wait_kdrv_que_done(CTL_IPP_HANDLE *p_hdl);
ER ctl_ipp_open_tsk(void);
ER ctl_ipp_close_tsk(void);
ER ctl_ipp_set_resume(BOOL b_flush_evt);
ER ctl_ipp_set_pause(BOOL b_wait_end, BOOL b_flush_evt);
ER ctl_ipp_wait_pause_end(void);

void ctl_ipp_direct_set_tsk_en(BOOL en);
BOOL ctl_ipp_direct_get_tsk_en(void);
void ctl_ipp_direct_set_is_stop(BOOL en, CTL_IPP_HANDLE *hdl);
BOOL ctl_ipp_direct_get_is_stop(CTL_IPP_HANDLE *hdl);
void ctl_ipp_direct_set_is_force(BOOL en);
BOOL ctl_ipp_direct_get_is_force(void);
UINT32 ctl_ipp_direct_get_hdl_num(void);
void ctl_ipp_direct_set_start_cnt(UINT32 cnt);
void ctl_ipp_direct_inc_start_cnt(CTL_IPP_HANDLE *p_hdl);
void ctl_ipp_direct_dec_start_cnt(CTL_IPP_HANDLE *p_hdl);
UINT32 ctl_ipp_direct_get_start_cnt(void);
CTL_IPP_DUAL_DIRECT_MODE ctl_ipp_direct_get_mode(void);

#endif //_CTL_IPP_INT_H
