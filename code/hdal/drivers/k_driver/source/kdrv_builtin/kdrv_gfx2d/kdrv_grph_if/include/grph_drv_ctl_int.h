#ifndef _GRAPH_DRV_CTL_INT_H_
#define _GRAPH_DRV_CTL_INT_H_

#include <kwrap/nvt_type.h>
#include <kwrap/spinlock.h>
#include <kwrap/task.h>
#include "kdrv_grph_ctl.h"
#include "grph_drv_tsk_int.h"
#include "grph_drv_util_int.h"

#define GRAPH_DRV_ENG_MAX 3 // must be = eng numbers/chip
#define TS_MAX_CNT 40

enum graph_drv_fire_mode {
	GRAPH_FIRE_SINGLE = 0,
	//GRAPH_FIRE_LL,
	GRAPH_FIRE_MAX,
};

/* all jobs corresponding to link-list */
struct graph_drv_ctl_job {

    struct graph_drv_job_cfg *job_cfg; //point to org graph_drv_job_cfg

    struct vos_list_head list; //graph_drv_ctl_job

	void *parent; //keep graph_drv_ctl_job_head
};

struct graph_drv_ctl_job_callback_ops {
	unsigned long param[2]; //[0]:handle addr, [1]:kjob addr
    void (*callback)(unsigned long param0, unsigned long param1); //callback api
};

struct graph_drv_ctl_job_head {

	int real_trig_eng; //copy from proc_job_head->eng at first, it will adjust by sw_balance flow
					   //always use this item at drv, don't usd proc_job_head->eng
    struct graph_drv_job_head *proc_job_head; //point to lib job

	atomic_t proc_num; 	//for count ctl proc job number(only for debug)
    atomic_t err_num; 	//for count ctl err job number
    atomic_t done_num; 	//for count ctl done job number
    			  		//(if (err_num + done_num) to proc_job->num, callback to lib)

    struct graph_drv_job_cfg *last_proc_job; //point to last graph_drv_job_cfg in K-FLOW
    struct graph_drv_job_cfg *cur_proc_job; //only used for cpu trigger mode

    struct vos_list_head proc_list; //graph_drv_ctl_info_t->proc_list_root
    struct vos_list_head cb_list; //graph_drv_ctl_info_t->cb_list_root

	unsigned int bscore; //for software balance used, busy level 0(idle) ~ max(busy)

	unsigned long time[6]; //0:put, 1:start proc, 2:proc end, 3:cb, 4:eng trig, 5:eng isr
	atomic_t sts;

	struct graph_drv_ctl_job_callback_ops ops;
};

struct graph_drv_ctl_ts {
    u32 chip;
    u32 eng;
    u32 id;
	unsigned long ts[6];
	u8 job_num;
};

struct graph_eng_counter {
	atomic_t open_in;
	atomic_t open_out;
	atomic_t trg_in;
	atomic_t trg_out;
	atomic_t close_in;
	atomic_t close_out;
};

struct graph_drv_ctl_ch {
	u16 chip_id;
	u16 eng_id;

    vk_spinlock_t proc_list_lock;
    struct vos_list_head proc_list_root; //keep proc job (graph_drv_ctl_job_head)

    vk_spinlock_t cb_list_lock;
    struct vos_list_head cb_list_root; //keep cb job (graph_drv_ctl_job_head)

	atomic_t graph_drv_ctl_put_cnt;
	atomic_t graph_drv_ctl_cb_cnt;

    vk_spinlock_t eng_list_lock;
	u16 eng_wait_cnt;
    struct vos_list_head eng_wait_list_root; //keep wait trig ll table (graph_drv_ll_blk)
	u16 eng_trig_cnt;
    struct vos_list_head eng_trig_list_root; //keep busy ll table (graph_drv_ll_blk)

	atomic_t ss_trig;
	unsigned int bscore; //for software balance used, busy level 0(idle) ~ max(busy)

	////for software balance debug
	unsigned int auto_cnt;
	unsigned int force_cnt;
	unsigned int bscore_cnt;

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

	struct graph_drv_tsk ctl_proc_tsk;

	s32 ts_idx;
	struct graph_drv_ctl_ts ts[TS_MAX_CNT];
	struct graph_eng_counter eng_counter;
};

struct graph_drv_ctl_swb_info {
	int eng_id;
	struct vos_list_head list;
};

struct graph_drv_ctl_chip {
	struct graph_drv_ctl_swb_info swb_info[GRAPH_DRV_ENG_MAX];
	struct vos_list_head swb_list_root;
};

struct graph_drv_ctl {
	u16 chip_num;
	u16 eng_num;
	u16 total_ch;
	enum graph_drv_fire_mode fire_mode; //0:single fire mode, 1:Link-List mode
	u16 gating_en;	//gating switch 1:enable, 0:disable

	unsigned long chip_hdl;
	struct graph_drv_ctl_chip *chip;
    vk_spinlock_t chip_lock;

	unsigned long ch_hdl;
	struct graph_drv_ctl_ch *ch;
    vk_spinlock_t ch_lock;

	unsigned long ctl_job_head_pool_hdl;						//cache memory for drv ctl job head

#if (PROC_DET_CMD == ENABLE)
	THREAD_HANDLE det_thread_hdl;
	int det_wakeup_flag;
#endif
};

int graph_drv_ctl_init(struct graph_drv_ctl *pctl, int chip_num, int eng_num);
int graph_drv_ctl_uninit(struct graph_drv_ctl *pctl);
int graph_drv_ctl_dump_info(void);
int graph_drv_ctl_dump_job_info(void);
int graph_drv_ctl_dump_job_list_info(void);
int graph_drv_ctl_dump_ts(void);
void graph_drv_ctl_det_proc_switch(int enable);
void graph_drv_ctl_sw_balance_set_enable(u16 flag);
int graph_drv_ctl_dump_sw_balance_cnt(void);
void graph_drv_ctl_sw_balance_cnt_reset(void);
int graph_drv_ctl_dump_util(void *sfile);
int graph_drv_ctl_dump_gating(void *sfile);
void graph_drv_ctl_set_gating(int flag);
enum graph_drv_sts graph_drv_ctl_put_job_with_callback(struct graph_drv_job_head *head, struct graph_drv_ctl_job_callback_ops *ops);
int graph_drv_ctl_set_dma_abort(BOOL flag);

extern int graph_drv_module_dump_info(void);
extern void graph_drv_eng_dump_reg(void);
#endif  //_GRAPH_DRV_CTL_INT_H_
