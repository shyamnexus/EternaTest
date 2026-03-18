#ifndef _DRE_DRV_CTL_INT_H_
#define _DRE_DRV_CTL_INT_H_

#include <kwrap/nvt_type.h>
#include <kwrap/spinlock.h>
#include <kwrap/task.h>
#include "kdrv_dre_ctl.h"
#include "dre_drv_ll_int.h"
#include "dre_drv_tsk_int.h"
#include "dre_drv_util_int.h"
#include "dre_eng.h"
#include "dre_drv_fusion_int.h"

#define SIM_HW_ENG DISABLE
#define TS_MAX_CNT 40
#define PRI_MAX_NUM 3
#define PRI_DEF_VAL (PRI_MAX_NUM >> 1)
#define PRI_REC_NUM 2

#define TIME_REC_NUM 6
#define SW_TIME_REC_NUM 2

enum dre_drv_fire_mode {
	DRE_FIRE_SINGLE = 0,
	DRE_FIRE_LL,
	DRE_FIRE_MAX,
};

struct dre_drv_ctl_sw_job_info {
	int en;		//0:normal flow, 1: sw proc flow
    struct dre_drv_dcm_layer_cfg dcm_layer;
	//add cfg infor at here
};

/* all jobs corresponding to link-list */
struct dre_drv_ctl_job {

    struct dre_drv_job_cfg *job_cfg; //point to org dre_drv_job_cfg

    struct vos_list_head list; //dre_drv_ctl_job

	void *parent; //keep dre_drv_ctl_job_head

	int sw_proc_flag; //for ctrl flow used, 0:normal flow, 1: sw proc flow, don't modify
	struct dre_drv_ctl_sw_job_info sw_job_info;
};

struct dre_drv_ctl_job_callback_ops {
	unsigned long param[2]; //[0]:handle addr, [1]:kjob addr
    void (*callback)(unsigned long param0, unsigned long param1); //callback api
};

struct dre_drv_ctl_job_head {

	int real_trig_eng; //copy from proc_job_head->eng at first, it will adjust by sw_balance flow
					   //always use this item at drv, don't usd proc_job_head->eng
    struct dre_drv_job_head *proc_job_head; //point to lib job

	atomic_t proc_num; 	//for count ctl proc job number(only for debug)
    atomic_t err_num; 	//for count ctl err job number
    atomic_t done_num; 	//for count ctl done job number
    			  		//(if (err_num + done_num) to proc_job->num, callback to lib)

    struct dre_drv_job_cfg *last_proc_job; //point to last dre_drv_job_cfg in K-FLOW
    struct dre_drv_job_cfg *cur_proc_job; //only used for cpu trigger mode

    struct vos_list_head proc_list; //dre_drv_ctl_info_t->proc_list_root
    struct vos_list_head cb_list; //dre_drv_ctl_info_t->cb_list_root
	struct vos_list_head dbg_list; //dre_drv_ctl_info_t->dbg_list_root

	unsigned int bscore; //for software balance used, busy level 0(idle) ~ max(busy)

	unsigned long time[TIME_REC_NUM]; //0:put, 1:start proc, 2:proc end, 3:cb, 4:eng trig, 5:eng isr
	unsigned long sw_time[SW_TIME_REC_NUM]; //0:sw proc(us), 1:sw proc end(us)
	atomic_t sts;

	struct dre_drv_ctl_job_callback_ops ops;
	u8 pri_info[PRI_REC_NUM]; //0:put job pri, 1:real proc job pri
	unsigned long pri_time;
};

struct dre_drv_ctl_ts {
    u32 chip;
    u32 eng;
    u32 id;
	unsigned long ts[TIME_REC_NUM];
	unsigned long sw_ts[SW_TIME_REC_NUM];
	u8 job_num;
	u8 pri_info[PRI_REC_NUM];
};

struct dre_drv_ctl_ch {
	u16 chip_id;
	u16 eng_id;

    vk_spinlock_t proc_list_lock;
	struct vos_list_head pri_list_root[PRI_MAX_NUM];  //keep proc job (dre_drv_ctl_job_head)
    struct vos_list_head proc_list_root; //keep proc job (dre_drv_ctl_job_head)

    vk_spinlock_t cb_list_lock;
    struct vos_list_head cb_list_root; //keep cb job (dre_drv_ctl_job_head)
	struct vos_list_head dbg_list_root; //keep cb job (dre_drv_ctl_job_head)

	atomic_t dre_drv_ctl_put_cnt;
	atomic_t dre_drv_ctl_cb_cnt;

    vk_spinlock_t eng_list_lock;
	u16 eng_wait_cnt;
    struct vos_list_head eng_wait_list_root; //keep wait trig ll table (dre_drv_ll_blk)
	u16 eng_trig_cnt;
    struct vos_list_head eng_trig_list_root; //keep busy ll table (dre_drv_ll_blk)
   	u16 sw_trig_cnt;
    struct vos_list_head sw_trig_list_root; //keep busy ll table (dre_drv_ll_blk)

	atomic_t ss_trig;
	unsigned int bscore; //for software balance used, busy level 0(idle) ~ max(busy)

	////for software balance debug
	unsigned int auto_cnt;
	unsigned int force_cnt;
	unsigned int bscore_cnt;

	DRE_ENG_HANDLE *eng_hdl;

	//for det hw util
	unsigned long eng_st;
	unsigned long eng_et;
	unsigned long eng_dt; //us
	unsigned long sw_eng_dt; //us

    vk_spinlock_t proc_time_lock;
	unsigned long st_time;
	unsigned long end_time;
	unsigned long eng_time; //us
	unsigned long eng_util;
	unsigned long sw_eng_time; //us
	unsigned long sw_eng_util;
	unsigned long det_interval;
	unsigned long last_update_time;
	unsigned int  st_cb_cnt;
	unsigned int  fps;

	struct dre_drv_tsk ctl_proc_tsk;
	struct dre_drv_tsk ctl_cb_tsk; //for ctl cb tasklet
	struct dre_drv_tsk ctl_sw_proc_tsk; //for sw proc task

#if (SIM_HW_ENG == ENABLE)
	struct dre_drv_tsk sim_isr_proc; //sim hw isr callback
#endif

	vk_spinlock_t ts_lock;
	s32 ts_idx;
	struct dre_drv_ctl_ts ts[TS_MAX_CNT];

	unsigned long pri_time_thres;

	struct dre_drv_fusion_layer fusion_layer;
};

struct dre_drv_ctl_chip {
	struct dre_drv_ctl_ch *last_trig_ch; //only support single handle put job.
										 //if multi handle put job case, must move this information to kdrv
};

struct dre_drv_ctl_log_info {
	unsigned long time;		//time
	unsigned char event[4];
	unsigned int param[2];
};

struct dre_drv_ctl {
	u16 chip_num;
	u16 eng_num;
	u16 total_ch;
	enum dre_drv_fire_mode fire_mode; //0:single fire mode, 1:Link-List mode
	u16 gating_en;	//gating switch 1:enable, 0:disable

	unsigned long chip_hdl;
	struct dre_drv_ctl_chip *chip;
    vk_spinlock_t chip_lock;

	unsigned long ch_hdl;
	struct dre_drv_ctl_ch *ch;
    vk_spinlock_t ch_lock;

	unsigned long ctl_job_head_pool_hdl;						//cache memory for drv ctl job head

	unsigned long ctl_job_pool_hdl;								//keep ctl free job entry (dre_drv_ctl_job)
	struct dre_drv_ctl_job *ctl_job;
    vk_spinlock_t ctl_job_lock;
	u16 ctl_job_free_cnt;
	u16 ctl_job_max_cnt;
	struct vos_list_head ctl_job_free_root;

#if (PROC_DET_CMD == ENABLE)
	THREAD_HANDLE det_thread_hdl;
	int det_wakeup_flag;
#endif

	uintptr_t log_buf_hdl;
    struct dre_drv_ctl_log_info *log_pool;
	unsigned int log_flag;	//1: enable log, 0: disable log
	unsigned int log_idx;
	unsigned int log_max_num;
	unsigned int log_repeat;
    vk_spinlock_t log_lock;
};

int dre_drv_ctl_init(struct dre_drv_ctl *pctl, int chip_num, int eng_num);
int dre_drv_ctl_uninit(struct dre_drv_ctl *pctl);
int dre_drv_ctl_dump_info(void);
int dre_drv_ctl_dump_job_info(void);
int dre_drv_ctl_dump_ll_list_info(int dump_eng_ll, int dump_wait_ll);
int dre_drv_ctl_dump_ts(void);
void dre_drv_ctl_det_proc_switch(int enable);
void dre_drv_ctl_sw_balance_set_enable(u16 flag);
int dre_drv_ctl_dump_sw_balance_cnt(void);
void dre_drv_ctl_sw_balance_cnt_reset(void);
int dre_drv_ctl_dump_util(void *sfile);
int dre_drv_ctl_dump_gating(void *sfile);
void dre_drv_ctl_set_gating(int flag);
enum dre_drv_sts dre_drv_ctl_put_job_with_callback(struct dre_drv_job_head *head, struct dre_drv_ctl_job_callback_ops *ops, int pri_idx);
int dre_drv_ctl_log_open(unsigned int max_num);
int dre_drv_ctl_log_close(void);
int dre_drv_ctl_log_trig(unsigned int st_flag);
void dre_drv_ctl_log_dump(char *filename);
int dre_drv_ctl_set_dma_abort(BOOL flag);

extern int dre_drv_module_dump_info(void);
extern void dre_drv_eng_dump_reg(void);
#endif  //_DRE_DRV_CTL_INT_H_
