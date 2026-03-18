/**
    DRE_Ctrl Layer

    @file       ctl_dre_int.h
    @ingroup    mIDRE_Ctrl
    @note

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/
#ifndef _CTL_DRE_INT_H_
#define _CTL_DRE_INT_H_

#include "kwrap/cpu.h"
#include "kwrap/util.h"
#include "kwrap/task.h"
#include "kwrap/flag.h"
#include "kwrap/semaphore.h"
#include "kwrap/spinlock.h"
#include "kwrap/type.h"
#include "kwrap/list.h"
#include "kdrv_videoprocess/dre/kdrv_dre_ctl.h"
#include "kflow_videoprocess/ctl_dre.h"
#include "kflow_videoprocess/ctl_dre_isp.h"
#include "ctl_dre_dbg.h"
#if defined(__FREERTOS)
#include <string.h>
#include <stdio.h>
#endif

#if defined(CONFIG_NVT_SMALL_HDAL)
#define CTL_DRE_MODULE_ENABLE	DISABLE
#else
#define CTL_DRE_MODULE_ENABLE	ENABLE
#endif

/**
	atomic operations
*/
#if defined(__LINUX)
/* use linux kernel atomic api */
#else
/* use gcc api */
typedef INT32 atomic_t;

#define ATOMIC_INIT(i) {(i)}
#define atomic_inc(x) __sync_add_and_fetch(x, 1)
#define atomic_dec(x) __sync_sub_and_fetch(x, 1)
#define atomic_read(x) __sync_add_and_fetch(x, 0)
#define atomic_set(x, v) __sync_bool_compare_and_swap(x, *x, v)
#define atomic_add(i, x) __sync_add_and_fetch(x, i)
#define atomic_xchg(x, v) __sync_val_compare_and_swap(x, *x, v)

#endif

#define CTL_DRE_UTIL_OFFSETOF(TYPE, MEMBER)  ((ULONG)&((TYPE *)0)->MEMBER)

#define CTL_DRE_MSG_STS_FREE    0x00000000
#define CTL_DRE_MSG_STS_LOCK    0x00000001

#define CTL_DRE_MSG_IGNORE      0x00000000
/*
	process command param differ in different usage
	[0]: handle, [1]: header address, [2]: buf_id(for unit release buffer)
*/
#define CTL_DRE_MSG_PROCESS     0x00000001
#define CTL_DRE_MSG_DROP        0x00000002  //[0]: handle, [1]: header address, [2]: err, this cmd is for erase queue used
#define CTL_DRE_MSG_PROCESS_VSP 0x00000003  // cb event only [0]: handle, [1]: header address, [2]: err, this cmd is for erase queue used

#define CTL_DRE_QUE_FLG_PROC	FLGPTN_BIT(0)

#define CTL_DRE_TASK_TIMEOUT_MS			(5000)
#define CTL_DRE_JOB_TIMEOUT_MS			(5000)

#define CTL_DRE_TASK_PAUSE				FLGPTN_BIT(0)
#define CTL_DRE_TASK_PAUSE_END			FLGPTN_BIT(1)
#define CTL_DRE_TASK_RESUME				FLGPTN_BIT(2)
#define CTL_DRE_TASK_RESUME_END			FLGPTN_BIT(3)
#define CTL_DRE_TASK_IDLE				FLGPTN_BIT(4)
#define CTL_DRE_TASK_RESTART			FLGPTN_BIT(5)
#define CTL_DRE_TASK_FLUSH				FLGPTN_BIT(6)
#define CTL_DRE_TASK_EXIT				FLGPTN_BIT(7)
#define CTL_DRE_TASK_CHK				FLGPTN_BIT(8)
#define CTL_DRE_TASK_TRIG				FLGPTN_BIT(9)
#define CTL_DRE_TASK_TRIG_END			FLGPTN_BIT(10)
#define CTL_DRE_TASK_EXIT_END			FLGPTN_BIT(11)
#define CTL_DRE_TASK_JOBDONE			FLGPTN_BIT(12)
#define CTL_DRE_TASK_CHK_JOBNUM			FLGPTN_BIT(13)
#define CTL_DRE_TASK_LOCK				FLGPTN_BIT(14)
#define CTL_DRE_TASK_OOPS				FLGPTN_BIT(31)
#define CTL_DRE_PROC_TASK_INIT       	(CTL_DRE_TASK_PAUSE_END|CTL_DRE_TASK_TRIG_END|CTL_DRE_TASK_LOCK)
#define CTL_DRE_CB_TASK_INIT       		(CTL_DRE_TASK_PAUSE_END|CTL_DRE_TASK_TRIG_END|CTL_DRE_TASK_LOCK)

#define CTL_DRE_TASK_PRIORITY		(3)

#define CTL_DRE_DBG_TS_NODE_MAX_NUM (16)
#define CTL_DRE_KDRV_JOB_MAX_NUM 	(16)
#define CTL_DRE_ADDR_ALIGN 			(16)
#define CTL_DRE_NVX2_BUF_RATIO 		(75)
#define CTL_DRE_NVX2_BUF_BASE 		(100)
#define CTL_DRE_LMT_MIN_LAYER_NUM		4

typedef struct vos_list_head CTL_DRE_LIST_HEAD;

typedef enum {
	CTL_DRE_DBG_TS_SNDEVT,
	CTL_DRE_DBG_TS_ALLOC,
	CTL_DRE_DBG_TS_CONFIG_ED,
	CTL_DRE_DBG_TS_END_ISR,
	CTL_DRE_DBG_TS_CB_TSK_ST,
	CTL_DRE_DBG_TS_CB_TSK_ED,
	CTL_DRE_DBG_TS_MAX,
} CTL_DRE_DBG_TIMESTAMP;

typedef enum {
	CTL_DRE_DBG_TS_TYPE_DRE,
	CTL_DRE_DBG_TS_TYPE_VSP,
	CTL_DRE_DBG_TS_TYPE_MAX,
} CTL_DRE_DBG_TS_TYPE;

typedef struct {
	ULONG handle;
	UINT32 input_frm_count;
	UINT32 type;
	UINT32 ts_flow[CTL_DRE_DBG_TS_MAX];
	INT32 err;
	UINT8 status;
	CTL_DRE_LIST_HEAD list;
} CTL_DRE_DBG_TS_NODE;

typedef struct {
	UINT8 enable;
	CHAR name[16];
	CHAR path[16];
	UINT32 save_input;
	UINT32 save_output;
} CTL_DRE_DBG_SAVEYUV_CFG;

typedef struct {
	UINT8 dump_info;
	UINT8 dump_file;
	UINT32 timeout_ms;
} CTL_DRE_DBG_ENG_HANG;

typedef struct {
	UINT32 cmd;
	ULONG param[5];
	UINT32 rev[2];
	UINT32 timestamp;
	CTL_DRE_LIST_HEAD list;
} CTL_DRE_MSG_EVENT;

typedef struct {
	CTL_DRE_BUF_ADDR addr;
	CTL_DRE_BUF_ADDR addr_uv;
	USIZE size;
	UINT32 lofs;
	UINT32 lofs_uv;
	VDO_PXLFMT fmt;
} CTL_DRE_SRC_IMG;

typedef struct {
	CTL_DRE_BUF_ADDR addr;
	CTL_DRE_BUF_ADDR addr_uv;
	USIZE size;
	USIZE bg_size;
	UPOINT dst_pos;		/* destination position base on background */
	UINT32 lofs;
	UINT32 lofs_uv;
	VDO_PXLFMT fmt;
} CTL_DRE_OUT_IMG;

typedef struct {
	CTL_DRE_PROC_MODE proc_mode;
	UINT32 quality_level;	//min:0, max: 6
	CTL_DRE_IN_CROP in_crop;
	CTL_DRE_OUT_PATH out_path;
	CTL_DRE_PIXEL_MASK  pixel_mask;
	UINT32 reserved;
} CTL_DRE_RT_CTL_INFO;

typedef struct {
	UINT32 blend_width[CTL_DRE_LMT_MAX_VSP_FRM_NUM];
	UINT32 center_width[CTL_DRE_LMT_MAX_VSP_FRM_NUM];
	UINT32 frm_num;
	UINT32 bg_width;
} CTL_DRE_VSP_CFG;

typedef struct {
	CTL_DRE_SRC_IMG src1_img;
	CTL_DRE_SRC_IMG src2_img;
	CTL_DRE_OUT_IMG out_img;
	CTL_DRE_VSP_CFG vsp_cfg;
	CTL_DRE_RT_CTL_INFO rtc_info;
	UINT32 reserved;
} CTL_DRE_BASEINFO;

typedef enum {
	CTL_DRE_JOB_BEGIN,
	CTL_DRE_JOB_KDRV_DONE,
} CTL_DRE_JOB_STS;

typedef struct {
	UINT32 tag;
	struct dre_drv_job_head kdrv_job;
	struct dre_drv_job_cfg kdrv_cfg;
	CTL_DRE_JOB_STS job_status;
	void *owner;					/* ctl dre handle */
	void *p_vsp;					/* optional, if is belong to a vsp job */
	CTL_DRE_BUF_ADDR work_buf_addr;
	CTL_DRE_FUSION_WT_IN fusion_wt_buf[CTL_DRE_LMT_MAX_LAYER_NUM];
	UINT32 vsp_frm_idx;
	CTL_DRE_EVT in_evt;
	CTL_DRE_EVT in_evt2;
	CTL_DRE_OUT_BUF_INFO buf_info;
	CTL_DRE_DBG_TS_NODE *p_dbg_node;
	CTL_DRE_LIST_HEAD list;
	CTL_DRE_LIST_HEAD fired_list;	/* for handle fired_job_list */
} CTL_DRE_JOB;

typedef struct {
	UINT32 tag;
	UINT32 frm_num;
	UINT32 done_mask;
	UINT32 dre_proc_bits;
	UINT32 dre_done_bits;
	UINT32 dre_skip_bits;
	void *owner;					/* ctl_dre handle */
	CTL_DRE_OUT_BUF_INFO buf_info;	/* buffer info for main path */
	CTL_DRE_DBG_TS_NODE *p_dbg_node;
	CTL_DRE_LIST_HEAD list;
	CTL_DRE_LIST_HEAD dre_head;		/* list head of dre jobs to trigger when dre job done */
} CTL_DRE_VSP_JOB;

typedef struct {
	CTL_DRE_BUF_ADDR work_buf_addr;
	UINT32 work_buf_size;
} CTL_DRE_WORK_BUF;

typedef enum {
	CTL_DRE_ISP_UPDATE_MSNR_LUT  	= 0x00000001,
	CTL_DRE_ISP_UPDATE_YCMOD_LUT 	= 0x00000002,
	CTL_DRE_ISP_UPDATE_NR_PARAM 	= 0x00000004,
	CTL_DRE_ISP_UPDATE_FUSION   	= 0x00000008,
} CTL_DRE_ISP_UPDATE;

typedef struct {
	CTL_DRE_ISP_MSNR_PARAM    msnr;
	CTL_DRE_ISP_YCMOD_PARAM   ycmod;
	CTL_DRE_ISP_NR_PARAM      nr_param;
	CTL_DRE_ISP_FUSION_WT_TBL weight_tbl;
	CTL_DRE_ISP_UPDATE 		  update;
} CTL_DRE_ISP_PARAM;

/**
	VSP flow information
	READY -> DRE1/2/3/4 = DONE -> READY

	todo: how to check expired or drop?
*/
#define CTL_DRE_VSP_MAX_JOB_NUM		(4)

typedef struct {
	/* config by set_api */
	UINT8 enable;

	CTL_DRE_VSP_JOB *p_cur_job;
	vk_spinlock_t lock;
} CTL_DRE_VSP_INFO;

typedef struct {
	UINT32 buf_size;
} CTL_DRE_BUF_PRI_INFO;

typedef struct {
	atomic_t err_sys;
	atomic_t err_par;
	atomic_t err_isp_par;
	atomic_t err_nomem;
	atomic_t err_qovr;
	atomic_t err_indata;
	atomic_t err_kdrv_trig;
	atomic_t err_kdrv_drop;
} CTL_DRE_DBG_ERR_ACCU_INFO;

typedef struct {
	UINT32 tag;
	CHAR name[16];
	CTL_DRE_RT_CTL_INFO rtc_info;
	CTL_DRE_BASEINFO 	ctl_info;	/* info that is currently used */
	CTL_DRE_VSP_INFO 	vsp_info;	/* vsp flow configuration */
	CTL_DRE_BUFCFG 		bufcfg;
	CTL_DRE_WT_CONFIG   wt_cfg;
	CTL_DRE_BUF_PRI_INFO private_buf;
	CTL_DRE_EVENT_FP cb_fp[CTL_DRE_CBEVT_MAX];
	CTL_DRE_LIST_HEAD fired_job_list_root;
	ULONG kdrv_hdl;
	UINT32 kdrv_id;
	UINT32 isp_id;
	UINT32 in_job_cnt;
	UINT32 out_job_cnt;
	UINT32 out_job_err_cnt;		/* number in out_job_cnt with (err_msg != E_OK) */
	UINT32 out_job_inq_cnt;		/* job number in cb queue */
	UINT32 job_skip_cnt;   		/* counter1 for skip trigger kdrv cnt, for fake kdrv callback using */
	atomic_t kdrv_in_job_cnt;	/* counter for job sending to kdriver */
	atomic_t kdrv_out_job_cnt;	/* counter for job done from kdriver */
	atomic_t vsp_in_job_cnt;	/* counter for vsp job alloc */
	atomic_t vsp_out_job_cnt;	/* counter for vsp job done regardless err_msg */
	atomic_t vsp_err_job_cnt;	/* counter for vsp_out_job_cnt with err_msg != E_OK */
	atomic_t in_buf_rls_cnt;
	atomic_t out_buf_new_cnt;
	atomic_t out_buf_push_cnt;
	atomic_t out_buf_drop_cnt;
	vk_spinlock_t lock;
	vk_spinlock_t job_done_lock;/* for jobdone sequence push out using */
	ID sem;						/* for dre api racing protect(ex: close & sndevt at same time) */
	UINT32 sndevt_rwlock;		/* rwlock for sndevt, bit0: writer lock, bit4~30: reader count, bit31: protect bit */

	CTL_DRE_ISP_PARAM isp_param;
	vk_spinlock_t isp_lock;

	CTL_DRE_LIST_HEAD list;
	CTL_DRE_DBG_ERR_ACCU_INFO dbg_err_accu_info;
	UINT32 reserved[8];
} CTL_DRE_HANDLE;

typedef struct {
	CHAR name[16];
	CTL_DRE_LIST_HEAD free_list_head;
	CTL_DRE_LIST_HEAD used_list_head;
	ULONG start_addr;
	UINT32 total_size;
	UINT32 blk_num;
	UINT32 blk_size;
	UINT32 cur_free_num;
	UINT32 max_used_num;
	vk_spinlock_t lock;
	ID flg_id;
} CTL_DRE_MEM_POOL;

typedef struct {
	vk_spinlock_t common_lock;
	CTL_DRE_MEM_POOL hdl_pool;
	CTL_DRE_MEM_POOL job_pool;
	CTL_DRE_MEM_POOL vsp_job_pool;
	CTL_DRE_MEM_POOL in_evt_que;
	CTL_DRE_MEM_POOL cb_evt_que;
	CTL_DRE_WORK_BUF work_buf;
	UINT32 kdrv_open_cnt;
	UINT32 kdrv_job_threshold;

	THREAD_HANDLE proc_tsk_id;
	ID proc_tsk_flg_id;
	vk_spinlock_t proc_tsk_lock;

	THREAD_HANDLE cb_tsk_id;
	ID cb_tsk_flg_id;
	vk_spinlock_t cb_tsk_lock;

	/* DEBUG */
	UINT32 dump_file_at_eng_hang;
	CTL_DRE_MEM_POOL dbg_ts_pool;
	CTL_DRE_DBG_SAVEYUV_CFG dbg_saveyuv_cfg;
	CTL_DRE_DBG_ENG_HANG dbg_eng_hang;
} CTL_DRE_CTL;

typedef struct {
	ULONG hdl;
	ULONG data;
	UINT32 buf_id;
	INT32 err;
	void *p_modify_info;
} CTL_DRE_MSG_MODIFY_CB_DATA;

/* api that use mem_pool as queue of CTL_DRE_MSG_EVENT */
typedef INT32 (*CTL_DRE_MSG_FLUSH_CB)(ULONG hdl, ULONG data, UINT32 buf_id, UINT32 frm_idx, ULONG user_data, INT32 err);
typedef INT32 (*CTL_DRE_MSG_MODIFY_CB)(CTL_DRE_MSG_MODIFY_CB_DATA *p_data);
INT32 ctl_dre_msg_snd(UINT32 num, UINT32 cmd, ULONG p1, ULONG p2, ULONG p3, ULONG p4, CTL_DRE_MEM_POOL *p_que);
INT32 ctl_dre_msg_rcv(UINT32 *cmd, ULONG *p1, ULONG *p2, ULONG *p3, ULONG *p4, ULONG *p5, UINT32 *time, CTL_DRE_MEM_POOL *p_que);
INT32 ctl_dre_msg_flush(CTL_DRE_MEM_POOL *p_que, CTL_DRE_HANDLE *p_hdl, CTL_DRE_MSG_FLUSH_CB flush_cb, UINT32 event);
INT32 ctl_dre_msg_modify(CTL_DRE_MEM_POOL *p_que, CTL_DRE_HANDLE *p_hdl, CTL_DRE_MSG_MODIFY_CB modify_cb, void *p_data);
INT32 ctl_dre_msg_init_queue(CTL_DRE_MEM_POOL *p_que);


#if 0
#endif

CTL_DRE_HANDLE *ctl_dre_int_get_handle_by_isp_id(UINT32 isp_id);

UINT32 ctl_dre_util_y2uvlof(VDO_PXLFMT fmt, UINT32 y_lof);
UINT32 ctl_dre_util_y2uvwidth(VDO_PXLFMT fmt, UINT32 y_w);
UINT32 ctl_dre_util_y2uvheight(VDO_PXLFMT fmt, UINT32 y_h);
UINT32 ctl_dre_util_yuv_size(VDO_PXLFMT fmt, UINT32 y_width, UINT32 y_height);
void ctl_dre_calc_plane_offset(VDO_FRAME *vdo_frm, URECT in_roi, UINT32 *addr_offset);

CTL_DRE_WORK_BUF *ctl_dre_int_get_work_buf(void);
CTL_DRE_VSP_JOB *ctl_dre_int_alloc_vsp_job(void);
void ctl_dre_int_free_vsp_job(CTL_DRE_VSP_JOB *p_job);

INT32 ctl_dre_process_d2d(CTL_DRE_HANDLE *p_hdl, CTL_DRE_JOB *p_job, void *cb_fp);
INT32 ctl_dre_inbuf_cb_wrapper(CTL_DRE_HANDLE *p_hdl, CTL_DRE_EVENT_FP bufio_fp, CTL_DRE_EVT *p_evt, CTL_DRE_BUF_IO bufio);
INT32 ctl_dre_outbuf_cb_wrapper(CTL_DRE_HANDLE *p_hdl, CTL_DRE_EVENT_FP bufio_fp, CTL_DRE_OUT_BUF_INFO *p_buf, CTL_DRE_BUF_IO bufio);

#if 0
#endif
void ctl_dre_set_dbg_lvl(CTL_DRE_DBG_LVL dbg_lvl);
void ctl_dre_process_dbg_dump_kflow_isp_param(CTL_DRE_ISP_PARAM *p_isp, int (*dump)(const char *fmt, ...));
void ctl_dre_process_dbg_dump_kflow_cfg(CTL_DRE_BASEINFO *p_base, int (*dump)(const char *fmt, ...));
void ctl_dre_process_dbg_dump_vsp_cfg(CTL_DRE_VSP_INFO *p_info, int (*dump)(const char *fmt, ...));
void ctl_dre_process_dbg_dump_kdrv_cfg(CTL_DRE_JOB *p_job, int (*dump)(const char *fmt, ...));

UINT32 ctl_dre_process_dbg_dump_cfg(UINT32 op, UINT32 val);

CTL_DRE_DBG_TS_NODE* ctl_dre_dbg_get_job_ts_node(void);
void ctl_dre_dbg_set_job_ts(CTL_DRE_DBG_TS_NODE* p_node, UINT32 evt, UINT32 timestamp);
void ctl_dre_dbg_set_job_ts_done(CTL_DRE_DBG_TS_NODE* p_node, INT32 err);

void ctl_dre_dump_job_ts(int (*dump)(const char *fmt, ...));
void ctl_dre_save_yuv_cfg(CHAR *name, CHAR *path, UINT32 save_input, UINT32 save_output);
void ctl_dre_dbg_eng_hang_cfg(CTL_DRE_DBG_ENG_HANG *p_eng_hang);
int ctl_dre_panic_printout_handler(uintptr_t data);
int ctl_dre_panic_handler(uintptr_t data);

int ctl_dre_int_printf(const char *fmtstr, ...);

#endif
