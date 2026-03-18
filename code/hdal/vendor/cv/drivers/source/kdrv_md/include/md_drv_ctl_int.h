#ifndef _MD_DRV_CTL_INT_H_
#define _MD_DRV_CTL_INT_H_

#include <kwrap/nvt_type.h>
#include <kwrap/spinlock.h>
#include <kwrap/task.h>
#include "kdrv_md_ctl.h"
#include "md_drv_ll_int.h"
#include "md_drv_tsk_int.h"
#include "md_drv_util_int.h"
#include "md_eng.h"

#define SIM_HW_ENG DISABLE
#define TS_MAX_CNT 40
#define PRI_MAX_NUM 3
#define PRI_DEF_VAL (PRI_MAX_NUM >> 1)

enum md_drv_fire_mode {
	MD_FIRE_SINGLE = 0,
	MD_FIRE_LL, //not support mode
	MD_FIRE_MAX,
};

/* all jobs corresponding to link-list */
struct md_drv_ctl_job {

    struct md_drv_job_cfg *job_cfg; //point to org md_drv_job_cfg

    struct vos_list_head list; //md_drv_ctl_job

	void *parent; //keep md_drv_ctl_job_head
};

struct md_drv_ctl_job_callback_ops {
	unsigned long param[2]; //[0]:handle addr, [1]:kjob addr
    void (*callback)(unsigned long param0, unsigned long param1); //callback api
};

struct md_drv_ctl_job_head {

	int real_trig_eng; //copy from proc_job_head->eng at first, it will adjust by sw_balance flow
					   //always use this item at drv, don't usd proc_job_head->eng
    struct md_drv_job_head *proc_job_head; //point to lib job

	atomic_t proc_num; 	//for count ctl proc job number(only for debug)
    atomic_t err_num; 	//for count ctl err job number
    atomic_t done_num; 	//for count ctl done job number
    			  		//(if (err_num + done_num) to proc_job->num, callback to lib)

    struct md_drv_job_cfg *last_proc_job; //point to last md_drv_job_cfg in K-FLOW
    struct md_drv_job_cfg *cur_proc_job; //only used for cpu trigger mode

    struct vos_list_head proc_list; //md_drv_ctl_info_t->proc_list_root
    struct vos_list_head cb_list; //md_drv_ctl_info_t->cb_list_root
	struct vos_list_head dbg_list; //md_drv_ctl_info_t->dbg_list_root

	unsigned int bscore; //for software balance used, busy level 0(idle) ~ max(busy)

	unsigned long time[6]; //0:put, 1:start proc, 2:proc end, 3:cb, 4:eng trig, 5:eng isr
	atomic_t sts;

	struct md_drv_ctl_job_callback_ops ops;
	u8 pri_info[2]; //0:put job pri, 1:real proc job pri
	unsigned long pri_time;
};

struct md_drv_ctl_ts {
    u32 chip;
    u32 eng;
    u32 id;
	unsigned long ts[6];
	u8 job_num;
	u8 pri_info[2];
};

struct md_drv_ctl_ch {
	u16 chip_id;
	u16 eng_id;
	atomic_t suspend_flag;

    vk_spinlock_t proc_list_lock;
	struct vos_list_head pri_list_root[PRI_MAX_NUM];  //keep proc job (md_drv_ctl_job_head)
    struct vos_list_head proc_list_root; //keep proc job (md_drv_ctl_job_head)

    vk_spinlock_t cb_list_lock;
    struct vos_list_head cb_list_root; //keep cb job (md_drv_ctl_job_head)
	struct vos_list_head dbg_list_root; //keep cb job (md_drv_ctl_job_head)

	atomic_t md_drv_ctl_put_cnt;
	atomic_t md_drv_ctl_cb_cnt;

    vk_spinlock_t eng_list_lock;
	u16 eng_wait_cnt;
    struct vos_list_head eng_wait_list_root; //keep wait trig ll table (md_drv_ll_blk)
	u16 eng_trig_cnt;
    struct vos_list_head eng_trig_list_root; //keep busy ll table (md_drv_ll_blk)

	atomic_t ss_trig;
	struct md_drv_ctl_job_head *ss_ctl_job_head; //keep ss job_head, control by ss_strig
	unsigned int bscore; //for software balance used, busy level 0(idle) ~ max(busy)

	////for software balance debug
	unsigned int auto_cnt;
	unsigned int force_cnt;
	unsigned int bscore_cnt;

	MD_ENG_HANDLE *eng_hdl;

	//for det hw util
	unsigned long eng_st;
	unsigned long eng_et;
	unsigned long eng_dt; //us

    vk_spinlock_t proc_time_lock;
	unsigned long st_time;
	unsigned long end_time;
	unsigned long eng_time;
	unsigned long eng_util;
	unsigned long det_interval;
	unsigned long last_update_time;
	unsigned int  st_cb_cnt;
	unsigned int  fps;

	struct md_drv_tsk ctl_proc_tsk;
	struct md_drv_tsk ctl_cb_tsk; //for ctl cb tasklet

#if (SIM_HW_ENG == ENABLE)
	struct md_drv_tsk sim_isr_proc; //sim hw isr callback
#endif

	vk_spinlock_t ts_lock;
	s32 ts_idx;
	struct md_drv_ctl_ts ts[TS_MAX_CNT];

	unsigned long pri_time_thres;
};

struct md_drv_ctl_chip {
	struct md_drv_ctl_ch *last_trig_ch; //only support single handle put job.
										 //if multi handle put job case, must move this information to kdrv
};

struct md_drv_ctl {
	u16 chip_num;
	u16 eng_num;
	u16 total_ch;
	enum md_drv_fire_mode fire_mode; //0:single fire mode, 1:Link-List mode
	u16 gating_en;	//gating switch 1:enable, 0:disable

    vk_spinlock_t eng_lock;
	u16	eng_open;

	unsigned long chip_hdl;
	struct md_drv_ctl_chip *chip;
    vk_spinlock_t chip_lock;

	unsigned long ch_hdl;
	struct md_drv_ctl_ch *ch;
    vk_spinlock_t ch_lock;

	unsigned long ctl_job_head_pool_hdl;						//cache memory for drv ctl job head

	unsigned long ctl_job_pool_hdl;								//keep ctl free job entry (md_drv_ctl_job)
	struct md_drv_ctl_job *ctl_job;
    vk_spinlock_t ctl_job_lock;
	u16 ctl_job_free_cnt;
	u16 ctl_job_max_cnt;
	struct vos_list_head ctl_job_free_root;

#if (PROC_DET_CMD == ENABLE)
	THREAD_HANDLE det_thread_hdl;
	int det_wakeup_flag;
#endif
};

int md_drv_ctl_init(struct md_drv_ctl *pctl, int chip_num, int eng_num);
int md_drv_ctl_uninit(struct md_drv_ctl *pctl);
int md_drv_ctl_dump_info(void);
int md_drv_ctl_dump_job_info(void);
int md_drv_ctl_dump_ll_list_info(int dump_eng_ll, int dump_wait_ll);
int md_drv_ctl_dump_ts(void);
void md_drv_ctl_det_proc_switch(int enable);
void md_drv_ctl_sw_balance_set_enable(u16 flag);
int md_drv_ctl_dump_sw_balance_cnt(void);
void md_drv_ctl_sw_balance_cnt_reset(void);
int md_drv_ctl_dump_util(void *sfile);
int md_drv_ctl_dump_gating(void *sfile);
void md_drv_ctl_set_gating(int flag);
int md_drv_ctl_suspend(int ch_idx);
int md_drv_ctl_resume(int ch_idx);
enum md_drv_sts md_drv_ctl_put_job_with_callback(struct md_drv_job_head *head, struct md_drv_ctl_job_callback_ops *ops, int pri_idx);
int md_drv_ctl_set_dma_abort(BOOL flag);
void md_drv_ctl_eng_open(void);
void md_drv_ctl_eng_close(void);

extern int md_drv_module_dump_info(void);
extern void md_drv_eng_dump_reg(void);
#endif  //_MD_DRV_CTL_INT_H_
