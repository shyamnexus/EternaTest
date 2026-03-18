/**
    VPE_Ctrl Layer

    @file       ctl_vpe_int.h
    @ingroup    mIVPE_Ctrl
    @note

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/
#ifndef _CTL_VPE_INT_H_
#define _CTL_VPE_INT_H_

#include "kwrap/cpu.h"
#include "kwrap/util.h"
#include "kwrap/task.h"
#include "kwrap/flag.h"
#include "kwrap/semaphore.h"
#include "kwrap/spinlock.h"
#include "kwrap/type.h"
#include "kwrap/list.h"
#include "kdrv_videoprocess/vpe/vpe_drv_ctl.h"
#include "kdrv_videoprocess/vpe/vpe_drv_limit.h"
#include "kflow_videoprocess/ctl_vpe.h"
#include "kflow_videoprocess/ctl_vpe_isp.h"
#include "ctl_vpe_dbg.h"
#include "ctl_vpe_ver.h"
#if defined(__FREERTOS)
#include <string.h>
#include <stdio.h>
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

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) ((a) > (b) ? (b) : (a))
#endif

#endif

#define CTL_VPE_UTIL_OFFSETOF(TYPE, MEMBER)  ((ULONG)&((TYPE *)0)->MEMBER)
/* reserved[7] */
#define ctl_vpe_vdofrm_reserved_mask_frm_num_idx(frm_num, frm_id)   (((frm_num << 16) & 0xffff0000) | (frm_id & 0xff))

#define CTL_VPE_MSG_STS_FREE    0x00000000
#define CTL_VPE_MSG_STS_LOCK    0x00000001

#define CTL_VPE_MSG_IGNORE      0x00000000
/*
	process command param differ in different usage
	[0]: handle, [1]: header address, [2]: buf_id(for unit release buffer)
*/
#define CTL_VPE_MSG_PROCESS     0x00000001
#define CTL_VPE_MSG_DROP        0x00000002  //[0]: handle, [1]: header address, [2]: err, this cmd is for erase queue used
#define CTL_VPE_MSG_PROCESS_VSP 0x00000003  // cb event only [0]: handle, [1]: header address, [2]: err, this cmd is for erase queue used

#define CTL_VPE_QUE_FLG_PROC	FLGPTN_BIT(0)

#define CTL_VPE_TASK_TIMEOUT_MS			(5000)
#define CTL_VPE_JOB_TIMEOUT_MS			(5000)

#define CTL_VPE_TASK_PAUSE				FLGPTN_BIT(0)
#define CTL_VPE_TASK_PAUSE_END			FLGPTN_BIT(1)
#define CTL_VPE_TASK_RESUME				FLGPTN_BIT(2)
#define CTL_VPE_TASK_RESUME_END			FLGPTN_BIT(3)
#define CTL_VPE_TASK_IDLE				FLGPTN_BIT(4)
#define CTL_VPE_TASK_RESTART			FLGPTN_BIT(5)
#define CTL_VPE_TASK_FLUSH				FLGPTN_BIT(6)
#define CTL_VPE_TASK_EXIT				FLGPTN_BIT(7)
#define CTL_VPE_TASK_CHK				FLGPTN_BIT(8)
#define CTL_VPE_TASK_TRIG				FLGPTN_BIT(9)
#define CTL_VPE_TASK_TRIG_END			FLGPTN_BIT(10)
#define CTL_VPE_TASK_EXIT_END			FLGPTN_BIT(11)
#define CTL_VPE_TASK_JOBDONE			FLGPTN_BIT(12)
#define CTL_VPE_TASK_CHK_JOBNUM			FLGPTN_BIT(13)
#define CTL_VPE_TASK_LOCK				FLGPTN_BIT(14)
#define CTL_VPE_TASK_OOPS				FLGPTN_BIT(31)
#define CTL_VPE_PROC_TASK_INIT       	(CTL_VPE_TASK_PAUSE_END|CTL_VPE_TASK_TRIG_END|CTL_VPE_TASK_LOCK)
#define CTL_VPE_CB_TASK_INIT       		(CTL_VPE_TASK_PAUSE_END|CTL_VPE_TASK_TRIG_END|CTL_VPE_TASK_LOCK)

#define CTL_VPE_TASK_PRIORITY		(3)

#define CTL_VPE_DBG_TS_NODE_MAX_NUM (16)
#define CTL_VPE_VSP_JOB_MAX_NUM 	(12)	//max job: 3 vsp handle with 4 job
#define CTL_VPE_KDRV_JOB_MAX_NUM 	(30)	//total job num for all vpe
#define CTL_VPE_BUF_ALIGN 			(16)
#define CTL_VPE_NVX2_BUF_RATIO 		(75)
#define CTL_VPE_NVX2_BUF_BASE 		(100)
#define CTL_VPE_SCALE_RATIO_MAX 	(1599)
#define CTL_VPE_SCALE_RATIO_BASE 	(100)

typedef struct vos_list_head CTL_VPE_LIST_HEAD;

typedef struct {
	UINT32 handle_num_max;
	UINT32 vsp_job_max;			/* max number of vsp job, 		ref. to CTL_VPE_VSP_JOB_MAX_NUM  */
	UINT32 kdrv_job_max;		/* max number of kdrv job, 		ref. to CTL_VPE_KDRV_JOB_MAX_NUM */
} CTL_VPE_CTX_BUF_DBG_INFO;

typedef enum {
	CTL_VPE_DBG_TS_SNDEVT,
	CTL_VPE_DBG_TS_ALLOC,
	CTL_VPE_DBG_TS_CONFIG_ED,
	CTL_VPE_DBG_TS_END_ISR,
	CTL_VPE_DBG_TS_CB_TSK_ST,
	CTL_VPE_DBG_TS_CB_TSK_ED,
	CTL_VPE_DBG_TS_MAX,
} CTL_VPE_DBG_TIMESTAMP;

typedef enum {
	CTL_VPE_DBG_TS_TYPE_VPE,
	CTL_VPE_DBG_TS_TYPE_VPE_PTZ,
	CTL_VPE_DBG_TS_TYPE_VSP,
	CTL_VPE_DBG_TS_TYPE_MAX,
} CTL_VPE_DBG_TS_TYPE;

typedef struct {
	ULONG handle;
	UINT32 input_frm_count;
	UINT32 type;
	UINT32 ts_flow[CTL_VPE_DBG_TS_MAX];
	INT32 err;
	UINT8 status;
	CTL_VPE_LIST_HEAD list;
} CTL_VPE_DBG_TS_NODE;

typedef struct {
	UINT8 enable;
	CHAR name[16];
	CHAR path[16];
	UINT32 save_input;
	UINT32 save_output;
} CTL_VPE_DBG_SAVEYUV_CFG;

typedef struct {
	UINT8 dump_info;
	UINT8 dump_file;
	UINT32 timeout_ms;
} CTL_VPE_DBG_ENG_HANG;

typedef struct {
	UINT32 cmd;
	ULONG param[5];
	UINT32 rev[2];
	UINT32 timestamp;
	CTL_VPE_LIST_HEAD list;
} CTL_VPE_MSG_EVENT;

typedef struct {
	ISIZE size;
	UINT32 src_img_h_align;
} CTL_VPE_SRC_IMG;

typedef struct {
	CTL_VPE_SRC_IMG src_img;	/* debug only, information from input vdo frame */
	CTL_VPE_IN_CROP in_crop;
	USIZE dce_out_sz;
	CTL_VPE_OUT_PATH out_path[CTL_VPE_OUT_PATH_ID_MAX];
	CTL_VPE_OUT_EXT_PATH out_ext_path[CTL_VPE_OUT_PATH_ID_MAX];
	UINT32 out_path_h_align[CTL_VPE_OUT_PATH_ID_MAX];
	CTL_VPE_PAL_COLOR palette_color[CTL_VPE_PALETTE_MAX];
	CTL_VPE_MASK_INFO mask_info;
	UINT32 reserved;
} CTL_VPE_BASEINFO;

typedef struct {
	UINT32 frm_num;
	UINT32 frm_idx;
	UINT32 pre_crp_width;
	UINT32 pre_crp_height;
	UINT32 scl_out_width;
	UINT32 scl_out_height;
	UINT32 post_crp_mid_width;
	UINT32 post_crp_ovlp_width;
	UINT32 post_crp_out_height;
} CTL_VPE_VSP_CTL;

typedef enum {
	CTL_VPE_JOB_NOT_READY,
	CTL_VPE_JOB_KDRV_DONE,
} CTL_VPE_JOB_STS;

typedef struct {
	UINT32 tag;
	CTL_VPE_FLOW_TYPE flow;
	CTL_VPE_VSP_CTL vsp_ctl;
	struct vpe_drv_job_head kdrv_job;
	struct vpe_drv_job_cfg kdrv_cfg;
	CTL_VPE_JOB_STS job_status;
	void *owner;					/* ctl vpe handle */
	void *p_vsp;					/* optional, if is belong to a vsp job */
	CTL_VPE_EVT in_evt;
	CTL_VPE_OUT_BUF_INFO buf_info[CTL_VPE_OUT_PATH_ID_MAX];
	CTL_VPE_DBG_TS_NODE *p_dbg_node;
	CTL_VPE_LIST_HEAD list;
	CTL_VPE_LIST_HEAD fired_list;	/* for handle fired_job_list */
} CTL_VPE_JOB;

typedef struct {
	UINT32 tag;
	UINT32 frm_num;
	UINT32 done_mask;
	UINT32 vpe_proc_bits;
	UINT32 vpe_done_bits;
	UINT32 vpe_skip_bits;
	void *owner;					/* ctl_vpe handle */
	CTL_VPE_OUT_BUF_INFO buf_info[2];	/* buffer info for main/blend path */
	CTL_VPE_DBG_TS_NODE *p_dbg_node;
	CTL_VPE_LIST_HEAD list;
} CTL_VPE_VSP_JOB;

/* special case for 2dlut isp param
	base on CTL_VPE_ISP_DCE_2DLUT_PARAM, but lut change to pointer
*/
typedef struct {
    UINT16 lut2d_width; 	//2~1024
    UINT16 lut2d_height; 	//2~1024
    UINT16 lut2d_lofs;		//4x align
	ULONG lut_paddr;
} CTL_VPE_INT_DCE_USER_2DLUT_PARAM;

/* special case for 2dlut isp param
	base on CTL_VPE_ISP_DCE_2DLUT_PARAM, but lut change to pointer
*/
typedef struct {
	UINT8	xofs_i;
	UINT32 	xofs_f;
	UINT8 	yofs_i;
	UINT32 	yofs_f;
    UINT16 lut2d_width; 		//2~1025
    UINT16 lut2d_height; 		//2~1025
    UINT16 lut2d_lofs; 			//align 4
	CTL_VPE_BUF_ADDR lut_addr;
	USIZE 	out_size;			//2dlut output size
	UINT8 lut2d_precision;		//0:abs(s14.2), 1:abs(s15.1), 2:abs(s16.0), 3:rela(s2.14), 4:rela(s3.13), 5:rela(s4.12), 6:rela(s5.11)
} CTL_VPE_ISP_INT_DCE_2DLUT_PARAM;

typedef struct {
	UINT32 isp_id;
	CTL_VPE_ISP_FLOW_TYPE isp_flow;
	CTL_VPE_ISP_SHARPEN_PARAM 		sharpen;
	CTL_VPE_ISP_DCE_CTL 			dce_ctl;
	CTL_VPE_ISP_INT_DCE_2DLUT_PARAM	dce_2dlut_param;
	CTL_VPE_ISP_YUV_CVT_PARAM 		yuv_cvt_param;
	CTL_VPE_ISP_DCTG_CTL 			dctg_ctl;
	CTL_VPE_ISP_COLOR_OFS_PARAM 	color_ofs_param;
	CTL_VPE_ISP_ISD2_PARAM 			isd2_param;
	CTL_VPE_ISP_FLIP_ROT_CTL 		flip_rot_ctl;
} CTL_VPE_ISP_PARAM;

/**
	VSP flow information
	READY -> VPE1/2/3/4 + DRE1/2/3/4 = DONE -> READY

	todo: how to check expired or drop?
*/
#define CTL_VPE_VSP_MAX_JOB_NUM		(4)
#define CTL_VPE_VSP_OVLP_RATIO_BASE	(1000)

typedef struct {
	/* config by set_api */
	UINT8 enable;
	UINT32 user_ovlp_width[CTL_VPE_VSP_FRM_NUM_MAX];
	UINT32 pre_crp_width[CTL_VPE_VSP_FRM_NUM_MAX];
	UINT32 pre_crp_height[CTL_VPE_VSP_FRM_NUM_MAX];
	USIZE scl_out_size[CTL_VPE_VSP_FRM_NUM_MAX];
	UINT32 post_crp_mid_width[CTL_VPE_VSP_FRM_NUM_MAX];
	UINT32 post_crp_ovlp_width[CTL_VPE_VSP_FRM_NUM_MAX];
	UINT32 post_crp_height;
	UINT32 center_bg_width;
	UINT32 blend_bg_width;
	CTL_VPE_VSP_JOB *p_cur_job;
	vk_spinlock_t lock;
} CTL_VPE_VSP_INFO;

typedef struct {
	ULONG vsp_pri_buf_addr;
	UINT32 vsp_pri_buf_size;
} CTL_VPE_BUF_PRI_ITEM_INFO;

typedef struct {
	CTL_VPE_PRIVATE_BUF buf_info;
	CTL_VPE_BUF_PRI_ITEM_INFO buf_item;
} CTL_VPE_BUF_PRI_INFO;

typedef struct {
	atomic_t err_sys;
	atomic_t err_par;
	atomic_t err_isp_par;
	atomic_t err_nomem;
	atomic_t err_qovr;
	atomic_t err_indata;
	atomic_t err_kdrv_trig;
	atomic_t err_kdrv_drop;
} CTL_VPE_DBG_ERR_ACCU_INFO;

typedef struct {
	UINT32 tag;
	CHAR name[16];
	CTL_VPE_FLOW_TYPE flow;
	CTL_VPE_BASEINFO rtc_info;	/* info that keep setting from api, wait for set apply */
	CTL_VPE_BASEINFO ctl_info;	/* info that is currently used */
	CTL_VPE_VSP_INFO vsp_info;	/* vsp flow configuration */
	CTL_VPE_BUFCFG bufcfg;
	CTL_VPE_BUF_PRI_INFO private_buf;
	CTL_VPE_EVENT_FP cb_fp[CTL_VPE_CBEVT_MAX];
	CTL_VPE_LIST_HEAD fired_job_list_root;
	ULONG kdrv_hdl;
	UINT32 kdrv_id;
	UINT32 in_job_cnt;
	UINT32 out_job_cnt;
	UINT32 out_job_err_cnt;		/* number in out_job_cnt with (err_msg != E_OK) */
	UINT32 out_job_inq_cnt;		/* job number in cb queue */
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
	ID sem;						/* for vpe api racing protect(ex: close & sndevt at same time) */
	UINT32 sndevt_rwlock;		/* rwlock for sndevt, bit0: writer lock, bit4~30: reader count, bit31: protect bit */

	CTL_VPE_ISP_PARAM isp_param;
	vk_spinlock_t isp_lock;

	CTL_VPE_LIST_HEAD list;
	CTL_VPE_DBG_ERR_ACCU_INFO dbg_err_accu_info;
	UINT32 reserved[8];
} CTL_VPE_HANDLE;

typedef struct {
	CHAR name[16];
	CTL_VPE_LIST_HEAD free_list_head;
	CTL_VPE_LIST_HEAD used_list_head;
	CTL_VPE_BUF_ADDR start_addr;
	UINT32 total_size;
	UINT32 blk_num;
	UINT32 blk_size;
	UINT32 cur_free_num;
	UINT32 max_used_num;
	vk_spinlock_t lock;
	ID flg_id;
} CTL_VPE_MEM_POOL;

typedef struct {
	vk_spinlock_t common_lock;
	CTL_VPE_MEM_POOL hdl_pool;
	CTL_VPE_MEM_POOL job_pool;
	CTL_VPE_MEM_POOL vsp_job_pool;
	CTL_VPE_MEM_POOL in_evt_que;
	CTL_VPE_MEM_POOL cb_evt_que;
	UINT32 kdrv_job_threshold;

	THREAD_HANDLE proc_tsk_id;
	ID proc_tsk_flg_id;
	vk_spinlock_t proc_tsk_lock;

	THREAD_HANDLE cb_tsk_id;
	ID cb_tsk_flg_id;
	vk_spinlock_t cb_tsk_lock;

	/* DEBUG */
	UINT32 dump_file_at_eng_hang;
	CTL_VPE_MEM_POOL dbg_ts_pool;
	CTL_VPE_DBG_SAVEYUV_CFG dbg_saveyuv_cfg;
	CTL_VPE_DBG_ENG_HANG dbg_eng_hang;
} CTL_VPE_CTL;

typedef struct {
	ULONG hdl;
	ULONG data;
	UINT32 buf_id;
	INT32 err;
	void *p_modify_info;
} CTL_VPE_MSG_MODIFY_CB_DATA;

/* api that use mem_pool as queue of CTL_VPE_MSG_EVENT */
typedef INT32 (*CTL_VPE_MSG_FLUSH_CB)(ULONG hdl, ULONG data, UINT32 buf_id, ULONG user_data, INT32 err);
typedef INT32 (*CTL_VPE_MSG_MODIFY_CB)(CTL_VPE_MSG_MODIFY_CB_DATA *p_data);
INT32 ctl_vpe_msg_snd(UINT32 cmd, ULONG p1, ULONG p2, ULONG p3, ULONG p4, CTL_VPE_MEM_POOL *p_que);
INT32 ctl_vpe_msg_snd_multi(UINT32 num, UINT32 *cmd, ULONG *p1, ULONG *p2, UINT32 *p3, ULONG *p4, CTL_VPE_MEM_POOL *p_que);
INT32 ctl_vpe_msg_rcv(UINT32 *cmd, ULONG *p1, ULONG *p2, ULONG *p3, ULONG *p4, UINT32 *time, CTL_VPE_MEM_POOL *p_que);
INT32 ctl_vpe_msg_flush(CTL_VPE_MEM_POOL *p_que, CTL_VPE_HANDLE *p_hdl, CTL_VPE_MSG_FLUSH_CB flush_cb, UINT32 event);
INT32 ctl_vpe_msg_modify(CTL_VPE_MEM_POOL *p_que, CTL_VPE_HANDLE *p_hdl, CTL_VPE_MSG_MODIFY_CB modify_cb, void *p_data);
INT32 ctl_vpe_msg_init_queue(CTL_VPE_MEM_POOL *p_que);


#if 0
#endif

CTL_VPE_HANDLE *ctl_vpe_int_get_handle_by_isp_id(UINT32 isp_id);
UINT32 ctl_vpe_util_ylof(VDO_PXLFMT fmt, UINT32 width);
UINT32 ctl_vpe_util_y2uvlof(VDO_PXLFMT fmt, UINT32 y_lof);
UINT32 ctl_vpe_util_y2uvwidth(VDO_PXLFMT fmt, UINT32 y_w);
UINT32 ctl_vpe_util_y2uvheight(VDO_PXLFMT fmt, UINT32 y_h);
UINT32 ctl_vpe_util_yuv_size(VDO_PXLFMT fmt, UINT32 y_width, UINT32 y_height);

CTL_VPE_VSP_JOB *ctl_vpe_int_alloc_vsp_job(void);
void ctl_vpe_int_free_vsp_job(CTL_VPE_VSP_JOB *p_job);

INT32 ctl_vpe_process_d2d(CTL_VPE_HANDLE *p_hdl, CTL_VPE_JOB *p_job, void *cb_fp);
INT32 ctl_vpe_inbuf_cb_wrapper(CTL_VPE_HANDLE *p_hdl, CTL_VPE_EVENT_FP bufio_fp, CTL_VPE_EVT *p_evt, CTL_VPE_BUF_IO bufio);
INT32 ctl_vpe_outbuf_cb_wrapper(CTL_VPE_HANDLE *p_hdl, CTL_VPE_EVENT_FP bufio_fp, CTL_VPE_OUT_BUF_INFO *p_buf, CTL_VPE_BUF_IO bufio);

#if 0
#endif
void ctl_vpe_set_dbg_lvl(CTL_VPE_DBG_LVL dbg_lvl);
void ctl_vpe_process_dbg_dump_kflow_isp_param(CTL_VPE_ISP_PARAM *p_isp, int (*dump)(const char *fmt, ...));
void ctl_vpe_process_dbg_dump_kflow_cfg(CTL_VPE_BASEINFO *p_base, int (*dump)(const char *fmt, ...));
void ctl_vpe_process_dbg_dump_vsp_cfg(CTL_VPE_VSP_INFO *p_info, CTL_VPE_JOB *p_job, int (*dump)(const char *fmt, ...));
void ctl_vpe_process_dbg_dump_kdrv_cfg(CTL_VPE_JOB *p_job, int (*dump)(const char *fmt, ...));

UINT32 ctl_vpe_process_dbg_dump_cfg(UINT32 op, UINT32 val);

CTL_VPE_DBG_TS_NODE* ctl_vpe_dbg_get_job_ts_node(void);
void ctl_vpe_dbg_set_job_ts(CTL_VPE_DBG_TS_NODE* p_node, UINT32 evt, UINT32 timestamp);
void ctl_vpe_dbg_set_job_ts_done(CTL_VPE_DBG_TS_NODE* p_node, INT32 err);

void ctl_vpe_dump_job_ts(int (*dump)(const char *fmt, ...));
void ctl_vpe_save_yuv_cfg(CHAR *name, CHAR *path, UINT32 save_input, UINT32 save_output);
void ctl_vpe_dbg_eng_hang_cfg(CTL_VPE_DBG_ENG_HANG *p_eng_hang);
int ctl_vpe_panic_printout_handler(uintptr_t data);
int ctl_vpe_panic_handler(uintptr_t data);

int ctl_vpe_int_printf(const char *fmtstr, ...);

#endif
