#ifndef _TRKE_DRV_CTL_INT_H_
#define _TRKE_DRV_CTL_INT_H_

#include <kwrap/nvt_type.h>
#include <kwrap/spinlock.h>
#include <kwrap/task.h>
#include "kdrv_trke_ctl.h"
#include "trke_drv_ll_int.h"
#include "trke_drv_tsk_int.h"
#include "trke_drv_util_int.h"
#include "trke_eng.h"

#define SIM_HW_ENG DISABLE
#define TS_MAX_CNT 40
#define PRI_MAX_NUM 3
#define PRI_DEF_VAL (PRI_MAX_NUM >> 1)

enum trke_drv_fire_mode {
	TRKE_FIRE_SINGLE = 0,
	TRKE_FIRE_LL,
	TRKE_FIRE_MAX,
};

/* all jobs corresponding to link-list */
struct trke_drv_ctl_job {

    struct trke_drv_job_cfg *job_cfg; //point to org trke_drv_job_cfg

    struct vos_list_head list; //trke_drv_ctl_job

	void *parent; //keep trke_drv_ctl_job_head
};

struct trke_drv_ctl_job_callback_ops {
	unsigned long param[2]; //[0]:handle addr, [1]:kjob addr
    void (*callback)(unsigned long param0, unsigned long param1); //callback api
};

struct trke_drv_ctl_job_head {

	int real_trig_eng; //copy from proc_job_head->eng at first, it will adjust by sw_balance flow
					   //always use this item at drv, don't usd proc_job_head->eng
    struct trke_drv_job_head *proc_job_head; //point to lib job

	atomic_t proc_num; 	//for count ctl proc job number(only for debug)
    atomic_t err_num; 	//for count ctl err job number
    atomic_t done_num; 	//for count ctl done job number
    			  		//(if (err_num + done_num) to proc_job->num, callback to lib)

    struct trke_drv_job_cfg *last_proc_job; //point to last trke_drv_job_cfg in K-FLOW
    struct trke_drv_job_cfg *cur_proc_job; //only used for cpu trigger mode

    struct vos_list_head proc_list; //trke_drv_ctl_info_t->proc_list_root
    struct vos_list_head cb_list; //trke_drv_ctl_info_t->cb_list_root
	struct vos_list_head dbg_list; //trke_drv_ctl_info_t->dbg_list_root

	unsigned int bscore; //for software balance used, busy level 0(idle) ~ max(busy)

	unsigned long time[6]; //0:put, 1:start proc, 2:proc end, 3:cb, 4:eng trig, 5:eng isr
	atomic_t sts;

	struct trke_drv_ctl_job_callback_ops ops;
	u8 pri_info[2]; //0:put job pri, 1:real proc job pri
	unsigned long pri_time;
};

struct trke_drv_ctl_ts {
    u32 chip;
    u32 eng;
    u32 id;
	unsigned long ts[6];
	u8 job_num;
	u8 pri_info[2];
};

struct trke_drv_ctl_ch {
	u16 chip_id;
	u16 eng_id;
	atomic_t suspend_flag;

    vk_spinlock_t proc_list_lock;
	struct vos_list_head pri_list_root[PRI_MAX_NUM];  //keep proc job (trke_drv_ctl_job_head)
    struct vos_list_head proc_list_root; //keep proc job (trke_drv_ctl_job_head)

    vk_spinlock_t cb_list_lock;
    struct vos_list_head cb_list_root; //keep cb job (trke_drv_ctl_job_head)
	struct vos_list_head dbg_list_root; //keep cb job (trke_drv_ctl_job_head)

	atomic_t trke_drv_ctl_put_cnt;
	atomic_t trke_drv_ctl_cb_cnt;

    vk_spinlock_t eng_list_lock;
	u16 eng_wait_cnt;
    struct vos_list_head eng_wait_list_root; //keep wait trig ll table (trke_drv_ll_blk)
	u16 eng_trig_cnt;
    struct vos_list_head eng_trig_list_root; //keep busy ll table (trke_drv_ll_blk)

	atomic_t ss_trig;
	unsigned int bscore; //for software balance used, busy level 0(idle) ~ max(busy)

	////for software balance debug
	unsigned int auto_cnt;
	unsigned int force_cnt;
	unsigned int bscore_cnt;

	TRKE_ENG_HANDLE *eng_hdl;

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

	struct trke_drv_tsk ctl_proc_tsk;
	struct trke_drv_tsk ctl_cb_tsk; //for ctl cb tasklet

#if (SIM_HW_ENG == ENABLE)
	struct trke_drv_tsk sim_isr_proc; //sim hw isr callback
#endif

	vk_spinlock_t ts_lock;
	s32 ts_idx;
	struct trke_drv_ctl_ts ts[TS_MAX_CNT];

	unsigned long pri_time_thres;
};

struct trke_drv_ctl_chip {
	struct trke_drv_ctl_ch *last_trig_ch; //only support single handle put job.
										 //if multi handle put job case, must move this information to kdrv
};

struct trke_drv_ctl {
	u16 chip_num;
	u16 eng_num;
	u16 total_ch;
	enum trke_drv_fire_mode fire_mode; //0:single fire mode, 1:Link-List mode
	u16 gating_en;	//gating switch 1:enable, 0:disable

	unsigned long chip_hdl;
	struct trke_drv_ctl_chip *chip;
    vk_spinlock_t chip_lock;

	unsigned long ch_hdl;
	struct trke_drv_ctl_ch *ch;
    vk_spinlock_t ch_lock;

	unsigned long ctl_job_head_pool_hdl;						//cache memory for drv ctl job head

	unsigned long ctl_job_pool_hdl;								//keep ctl free job entry (trke_drv_ctl_job)
	struct trke_drv_ctl_job *ctl_job;
    vk_spinlock_t ctl_job_lock;
	u16 ctl_job_free_cnt;
	u16 ctl_job_max_cnt;
	struct vos_list_head ctl_job_free_root;

#if (PROC_DET_CMD == ENABLE)
	THREAD_HANDLE det_thread_hdl;
	int det_wakeup_flag;
#endif
};

int trke_drv_ctl_init(struct trke_drv_ctl *pctl, int chip_num, int eng_num);
int trke_drv_ctl_uninit(struct trke_drv_ctl *pctl);
int trke_drv_ctl_dump_info(void);
int trke_drv_ctl_dump_job_info(void);
int trke_drv_ctl_dump_ll_list_info(int dump_eng_ll, int dump_wait_ll);
int trke_drv_ctl_dump_ts(void);
void trke_drv_ctl_det_proc_switch(int enable);
void trke_drv_ctl_sw_balance_set_enable(u16 flag);
int trke_drv_ctl_dump_sw_balance_cnt(void);
void trke_drv_ctl_sw_balance_cnt_reset(void);
int trke_drv_ctl_dump_util(void *sfile);
int trke_drv_ctl_dump_gating(void *sfile);
void trke_drv_ctl_set_gating(int flag);
enum trke_drv_sts trke_drv_ctl_put_job_with_callback(struct trke_drv_job_head *head, struct trke_drv_ctl_job_callback_ops *ops, int pri_idx);
int trke_drv_ctl_set_dma_abort(BOOL flag);

extern int trke_drv_module_dump_info(void);
extern void trke_drv_eng_dump_reg(void);
#endif  //_TRKE_DRV_CTL_INT_H_
