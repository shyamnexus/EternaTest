/**
    ISE_Ctrl Layer

    @file       ctl_ise_int.h
    @ingroup    mIISE_Ctrl
    @note

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/
#ifndef _CTL_ISE_INT_H_
#define _CTL_ISE_INT_H_

#include "kwrap/cpu.h"
#include "kwrap/util.h"
#include "kwrap/task.h"
#include "kwrap/flag.h"
#include "kwrap/semaphore.h"
#include "kwrap/spinlock.h"
#include "kwrap/type.h"
#include "kwrap/list.h"
#include "ctl_ise.h"
#include "ctl_ise_dbg.h"
#include "kdrv_gfx2d/kdrv_ise_ctl.h"
#include "kdrv_gfx2d/kdrv_ise_lmt.h"
#if defined(__FREERTOS)
#include <string.h>
#include <stdio.h>
#endif

#define CTL_ISE_MODULE_ENABLE	ENABLE

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

#define CTL_ISE_KDRV_JOB_IDENTY_ID    	0x15E00000
#define CTL_ISE_MASK_KDRV_JOB_ID(kdrv_id, path_id)	((CTL_ISE_KDRV_JOB_IDENTY_ID) | (kdrv_id) | (1 << path_id))
#define CTL_ISE_UTIL_OFFSETOF(TYPE, MEMBER)  ((ULONG)&((TYPE *)0)->MEMBER)

#define CTL_ISE_MSG_STS_FREE    0x00000000
#define CTL_ISE_MSG_STS_LOCK    0x00000001

#define CTL_ISE_MSG_IGNORE      0x00000000
/*
	process command param differ in different usage
	[0]: handle, [1]: header address, [2]: buf_id(for unit release buffer)
*/
#define CTL_ISE_MSG_PROCESS     0x00000001
#define CTL_ISE_MSG_DROP        0x00000002  //[0]: handle, [1]: header address, [2]: err, this cmd is for erase queue used

#define CTL_ISE_QUE_FLG_PROC	FLGPTN_BIT(0)

#define CTL_ISE_TASK_TIMEOUT_MS			(5000)
#define CTL_ISE_JOB_TIMEOUT_MS			(5000)

#define CTL_ISE_TASK_PAUSE				FLGPTN_BIT(0)
#define CTL_ISE_TASK_PAUSE_END			FLGPTN_BIT(1)
#define CTL_ISE_TASK_RESUME				FLGPTN_BIT(2)
#define CTL_ISE_TASK_RESUME_END			FLGPTN_BIT(3)
#define CTL_ISE_TASK_IDLE				FLGPTN_BIT(4)
#define CTL_ISE_TASK_RESTART			FLGPTN_BIT(5)
#define CTL_ISE_TASK_FLUSH				FLGPTN_BIT(6)
#define CTL_ISE_TASK_EXIT				FLGPTN_BIT(7)
#define CTL_ISE_TASK_CHK				FLGPTN_BIT(8)
#define CTL_ISE_TASK_TRIG				FLGPTN_BIT(9)
#define CTL_ISE_TASK_TRIG_END			FLGPTN_BIT(10)
#define CTL_ISE_TASK_EXIT_END			FLGPTN_BIT(11)
#define CTL_ISE_TASK_JOBDONE			FLGPTN_BIT(12)
#define CTL_ISE_TASK_CHK_JOBNUM			FLGPTN_BIT(13)
#define CTL_ISE_TASK_LOCK				FLGPTN_BIT(14)
#define CTL_ISE_TASK_OOPS				FLGPTN_BIT(31)
#define CTL_ISE_PROC_TASK_INIT       	(CTL_ISE_TASK_PAUSE_END|CTL_ISE_TASK_TRIG_END|CTL_ISE_TASK_LOCK)
#define CTL_ISE_CB_TASK_INIT       		(CTL_ISE_TASK_PAUSE_END|CTL_ISE_TASK_TRIG_END|CTL_ISE_TASK_LOCK)

#define CTL_ISE_TASK_PRIORITY			(3)

#define CTL_ISE_DBG_TS_NODE_MAX_NUM (16)
#define CTL_ISE_KDRV_JOB_MAX_NUM 	(15)
#define CTL_ISE_NVX2_BUF_RATIO      (75)
#define CTL_ISE_NVX2_BUF_BASE       (100)

typedef struct vos_list_head CTL_ISE_LIST_HEAD;

typedef enum {
	CTL_ISE_DBG_TS_SNDEVT,
	CTL_ISE_DBG_TS_ALLOC,
	CTL_ISE_DBG_TS_CONFIG_ED,
	CTL_ISE_DBG_TS_END_ISR,
	CTL_ISE_DBG_TS_CB_TSK_ST,
	CTL_ISE_DBG_TS_CB_TSK_ED,
	CTL_ISE_DBG_TS_MAX,
} CTL_ISE_DBG_TIMESTAMP;

typedef enum {
	CTL_ISE_OUT_PATH_IDX_1 = (CTL_ISE_OUT_PATH_ID_1 << 1),	/* output path idx */
	CTL_ISE_OUT_PATH_IDX_2 = (CTL_ISE_OUT_PATH_ID_2 << 1),
	CTL_ISE_OUT_PATH_IDX_3 = (CTL_ISE_OUT_PATH_ID_3 << 1),
	CTL_ISE_OUT_PATH_IDX_4 = (CTL_ISE_OUT_PATH_ID_4 << 1),
	CTL_ISE_OUT_PATH_IDX_MAX,
} CTL_ISE_OUT_PATH_IDX;

typedef struct {
	ULONG handle;
	UINT32 input_frm_count;
	UINT32 ts_flow[CTL_ISE_DBG_TS_MAX];
	INT32 err;
	UINT8 status;
	CTL_ISE_LIST_HEAD list;
} CTL_ISE_DBG_TS_NODE;

typedef struct {
	UINT8 enable;
	CHAR name[16];
	CHAR path[16];
	UINT32 save_input;
	UINT32 save_output;
} CTL_ISE_DBG_SAVEYUV_CFG;

typedef struct {
	UINT8 dump_info;
	UINT8 dump_file;
	UINT32 timeout_ms;
} CTL_ISE_DBG_ENG_HANG;

typedef struct {
	UINT32 cmd;
	ULONG param[5];
	UINT32 rev[2];
	UINT32 timestamp;
	CTL_ISE_LIST_HEAD list;
} CTL_ISE_MSG_EVENT;

typedef struct {
	ISIZE size;
	UINT32 src_img_h_align;
} CTL_ISE_SRC_IMG;

typedef struct {
	CTL_ISE_SRC_IMG src_img;	/* debug only, information from input vdo frame */
	CTL_ISE_IN_CROP in_crop;
	CTL_ISE_OUT_PATH out_path[CTL_ISE_OUT_PATH_ID_MAX];
	UINT32 out_path_h_align[CTL_ISE_OUT_PATH_ID_MAX];
	UINT32 reserved;
} CTL_ISE_BASEINFO;

typedef enum {
	CTL_ISE_JOB_NOT_READY,
	CTL_ISE_JOB_KDRV_DONE,
	CTL_ISE_JOB_READY_FOR_PUSH,
} CTL_ISE_JOB_STS;

typedef struct {
	UINT32 tag;
	struct ise_drv_job_head kdrv_job;
	struct ise_drv_job_cfg kdrv_cfg[CTL_ISE_OUT_PATH_ID_MAX];
	struct ise_drv_job_cfg kdrv_uv_cfg[CTL_ISE_OUT_PATH_ID_MAX];
	void *owner;
	CTL_ISE_EVT in_evt;
	CTL_ISE_BASEINFO base_info;
	CTL_ISE_OUT_BUF_INFO buf_info[CTL_ISE_OUT_PATH_ID_MAX];
	CTL_ISE_DBG_TS_NODE *p_dbg_node;
	CTL_ISE_LIST_HEAD list;
	CTL_ISE_LIST_HEAD fired_list;	/* for handle fired_job_list */
	/* job ctl */
	CTL_ISE_JOB_STS job_status;
	UINT32 proc_path_mask;
	UINT32 job_done_bits;
	UINT32 job_fail_bits;
	UINT32 job_skip_bits;
} CTL_ISE_JOB;

typedef struct {
	atomic_t err_sys;
	atomic_t err_par;
	atomic_t err_isp_par;
	atomic_t err_nomem;
	atomic_t err_qovr;
	atomic_t err_indata;
	atomic_t err_kdrv_trig;
	atomic_t err_kdrv_drop;
} CTL_ISE_DBG_ERR_ACCU_INFO;

typedef struct {
	UINT32 tag;
	CHAR name[16];
	CTL_ISE_BASEINFO rtc_info;	/* info that keep setting from api, wait for set apply */
	CTL_ISE_BASEINFO ctl_info;	/* info that is currently used */
	CTL_ISE_EVENT_FP cb_fp[CTL_ISE_CBEVT_MAX];
	CTL_ISE_LIST_HEAD fired_job_list_root;
	CTL_ISE_JOB *p_fired_job;	//remove
	ULONG kdrv_hdl;
	UINT32 kdrv_id;
	UINT32 in_job_cnt;
	UINT32 out_job_cnt;
	UINT32 out_job_err_cnt;		/* number in out_job_cnt with (err_msg != E_OK) */
	UINT32 out_job_inq_cnt;		/* job number in cb queue */
	atomic_t kdrv_in_job_cnt;	/* counter for job sending to kdriver */
	atomic_t kdrv_out_job_cnt;	/* counter for job done from kdriver */
	atomic_t in_buf_rls_cnt;
	atomic_t out_buf_new_cnt;
	atomic_t out_buf_push_cnt;
	atomic_t out_buf_drop_cnt;
	vk_spinlock_t lock;
	ID sem;						/* for ise api racing protect(ex: close & sndevt at same time) */
	UINT32 sndevt_rwlock;		/* rwlock for sndevt, bit0: writer lock, bit4~30: reader count, bit31: protect bit */

	CTL_ISE_LIST_HEAD list;
	CTL_ISE_DBG_ERR_ACCU_INFO dbg_err_accu_info;
	UINT32 reserved[8];
} CTL_ISE_HANDLE;

typedef struct {
	CHAR name[16];
	CTL_ISE_LIST_HEAD free_list_head;
	CTL_ISE_LIST_HEAD used_list_head;
	CTL_ISE_BUF_ADDR start_addr;
	UINT32 total_size;
	UINT32 blk_num;
	UINT32 blk_size;
	UINT32 cur_free_num;
	UINT32 max_used_num;
	vk_spinlock_t lock;
	ID flg_id;
} CTL_ISE_MEM_POOL;

typedef struct {
	vk_spinlock_t common_lock;
	CTL_ISE_MEM_POOL hdl_pool;
	CTL_ISE_MEM_POOL job_pool;
	CTL_ISE_MEM_POOL in_evt_que;
	CTL_ISE_MEM_POOL cb_evt_que;
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
	CTL_ISE_MEM_POOL dbg_ts_pool;
	CTL_ISE_DBG_SAVEYUV_CFG dbg_saveyuv_cfg;
	CTL_ISE_DBG_ENG_HANG dbg_eng_hang;
} CTL_ISE_CTL;

typedef struct {
	ULONG hdl;
	ULONG data;
	UINT32 buf_id;
	INT32 err;
	void *p_modify_info;
} CTL_ISE_MSG_MODIFY_CB_DATA;

/* api that use mem_pool as queue of CTL_ISE_MSG_EVENT */
typedef INT32 (*CTL_ISE_MSG_FLUSH_CB)(ULONG hdl, ULONG data, UINT32 buf_id, INT32 err);
typedef INT32 (*CTL_ISE_MSG_MODIFY_CB)(CTL_ISE_MSG_MODIFY_CB_DATA *p_data);
INT32 ctl_ise_msg_snd(UINT32 cmd, ULONG p1, ULONG p2, ULONG p3, CTL_ISE_MEM_POOL *p_que);
INT32 ctl_ise_msg_rcv(UINT32 *cmd, ULONG *p1, ULONG *p2, ULONG *p3, UINT32 *time, CTL_ISE_MEM_POOL *p_que);
INT32 ctl_ise_msg_flush(CTL_ISE_MEM_POOL *p_que, CTL_ISE_HANDLE *p_hdl, CTL_ISE_MSG_FLUSH_CB flush_cb);
INT32 ctl_ise_msg_modify(CTL_ISE_MEM_POOL *p_que, CTL_ISE_HANDLE *p_hdl, CTL_ISE_MSG_MODIFY_CB modify_cb, void *p_data);
INT32 ctl_ise_msg_init_queue(CTL_ISE_MEM_POOL *p_que);


#if 0
#endif

UINT32 ctl_ise_util_ylof(VDO_PXLFMT fmt, UINT32 width);
UINT32 ctl_ise_util_y2uvlof(VDO_PXLFMT fmt, UINT32 y_lof);
UINT32 ctl_ise_util_y2uvwidth(VDO_PXLFMT fmt, UINT32 y_w);
UINT32 ctl_ise_util_y2uvheight(VDO_PXLFMT fmt, UINT32 y_h);
UINT32 ctl_ise_util_yuv_size(VDO_PXLFMT fmt, UINT32 y_width, UINT32 y_height);

INT32 ctl_ise_process_d2d(CTL_ISE_HANDLE *p_hdl, CTL_ISE_JOB *p_job, void *cb_fp);
INT32 ctl_ise_inbuf_cb_wrapper(CTL_ISE_HANDLE *p_hdl, CTL_ISE_EVENT_FP bufio_fp, CTL_ISE_EVT *p_evt, CTL_ISE_BUF_IO bufio);
INT32 ctl_ise_outbuf_cb_wrapper(CTL_ISE_HANDLE *p_hdl, CTL_ISE_EVENT_FP bufio_fp, CTL_ISE_OUT_BUF_INFO *p_buf, CTL_ISE_BUF_IO bufio);

#if 0
#endif
void ctl_ise_set_dbg_lvl(CTL_ISE_DBG_LVL dbg_lvl);
void ctl_ise_process_dbg_dump_kflow_cfg(CTL_ISE_BASEINFO *p_base, int (*dump)(const char *fmt, ...));
void ctl_ise_process_dbg_dump_kdrv_cfg(CTL_ISE_JOB *p_job, int (*dump)(const char *fmt, ...));
UINT32 ctl_ise_process_dbg_dump_cfg(UINT32 op, UINT32 val);

void ctl_ise_dump_job_ts(int (*dump)(const char *fmt, ...));
void ctl_ise_save_yuv_cfg(CHAR *name, CHAR *path, UINT32 save_input, UINT32 save_output);
void ctl_ise_dbg_eng_hang_cfg(CTL_ISE_DBG_ENG_HANG *p_eng_hang);
int ctl_ise_panic_printout_handler(uintptr_t data);
int ctl_ise_panic_handler(uintptr_t data);

int ctl_ise_int_printf(const char *fmtstr, ...);

#endif
