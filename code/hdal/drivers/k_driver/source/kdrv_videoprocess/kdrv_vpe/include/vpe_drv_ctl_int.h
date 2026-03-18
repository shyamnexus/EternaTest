#ifndef _VPE_DRV_CTL_INT_H_
#define _VPE_DRV_CTL_INT_H_

#include <kwrap/nvt_type.h>
#include <kwrap/spinlock.h>
#include <kwrap/task.h>
#include "vpe_drv_ctl.h"
#include "vpe_drv_ll_int.h"
#include "vpe_drv_tsk_int.h"
#include "vpe_drv_util_int.h"
#include "vpe_eng.h"

#define SIM_HW_ENG DISABLE
#define TS_MAX_CNT 20
#define VPE_DRV_ENG_MAX 3 // must be = eng numbers/chip
#define VPE_DRV_ENG_COL_MAX VPE_DRV_ENG_MAX

enum vpe_drv_fire_mode {
	VPE_FIRE_SINGLE = 0,
	VPE_FIRE_LL,
	VPE_FIRE_MAX,
};

struct vpe_drv_lut2d_info {
	u8 ddr_id;
	void *va_addr;
	uintptr_t pa_addr;
	uint size;
};

/* all jobs corresponding to link-list */
struct vpe_drv_ctl_job {

    struct vpe_drv_job_cfg *job_cfg; //point to org vpe_drv_job_cfg

    struct vos_list_head list; //vpe_drv_ctl_job

	void *parent; //keep vpe_drv_ctl_job_head

	struct vpe_drv_lut2d_info lut2d;  //for dce 2d lut rot used
};

struct vpe_drv_ctl_eng_col_info {
	int st_idx;
	int end_idx;
	int img_total_col_num; //real col num
	int expect_col_num;
};

struct vpe_drv_ctl_eng_col_head {
    vk_spinlock_t lock; //for callback process
	int proc_num; 	//for count eng col job number
    int done_num; 	//for count eng col done job number
	atomic_t err_num; //for proc keep error cnt
    struct vpe_drv_ctl_ch *main_ch;

	struct vpe_drv_ctl_eng_col_info col[VPE_DRV_ENG_COL_MAX];
};

struct vpe_drv_ctl_job_callback_ops {
	unsigned long param[2]; //[0]:handle addr, [1]:kjob addr
    void (*callback)(unsigned long param0, unsigned long param1); //callback api
};

struct vpe_drv_ctl_job_head {

	int real_trig_eng; //copy from proc_job_head->eng at first, it will adjust by sw_balance flow
					   //always use this item at drv, don't usd proc_job_head->eng
    struct vpe_drv_job_head *proc_job_head; //point to lib job

	atomic_t proc_num; 	//for count ctl proc job number(only for debug)
    atomic_t err_num; 	//for count ctl err job number
    atomic_t done_num; 	//for count ctl done job number
    			  		//(if (err_num + done_num) to proc_job->num, callback to lib)

    struct vpe_drv_job_cfg *last_proc_job; //point to last vpe_drv_job_cfg in K-FLOW
    struct vpe_drv_job_cfg *cur_proc_job; //only used for cpu trigger mode

    struct vos_list_head proc_list; //vpe_drv_ctl_info_t->proc_list_root
    struct vos_list_head cb_list; //vpe_drv_ctl_info_t->cb_list_root

	unsigned int bscore; //for software balance used, busy level 0(idle) ~ max(busy)

	unsigned long time[6]; //0:put, 1:start proc, 2:proc end, 3:cb, 4:eng trig, 5:eng isr
	atomic_t sts;

	int eng_col_mode;	//0xff: disable eng col mode, 0:main job, 1~(VPE_DRV_ENG_COL_MAX - 1):sub job
	struct vpe_drv_ctl_eng_col_info eng_col;	//only for debug used
												//copy from eng_col_head->col
												//avoid eng_col_head buffer already freed condition
	struct vpe_drv_ctl_eng_col_head *eng_col_head;

	struct vpe_drv_ctl_job_callback_ops ops;
};

struct vpe_drv_ctl_ts {
    u32 chip;
    u32 eng;
    u32 id;
	unsigned long ts[6];
	u8 job_num;
};

struct vpe_drv_ctl_ch {
	u16 chip_id;
	u16 eng_id;

    vk_spinlock_t proc_list_lock;
    struct vos_list_head proc_list_root; //keep proc job (vpe_drv_ctl_job_head)

    vk_spinlock_t cb_list_lock;
    struct vos_list_head cb_list_root; //keep cb job (vpe_drv_ctl_job_head)

	atomic_t vpe_drv_ctl_put_cnt;
	atomic_t vpe_drv_ctl_cb_cnt;

	atomic_t vpe_drv_ctl_eng_col_put_cnt;
	atomic_t vpe_drv_ctl_eng_col_cb_cnt;

    vk_spinlock_t eng_list_lock;
	u16 eng_wait_cnt;
    struct vos_list_head eng_wait_list_root; //keep wait trig ll table (vpe_drv_ll_blk)
	u16 eng_trig_cnt;
    struct vos_list_head eng_trig_list_root; //keep busy ll table (vpe_drv_ll_blk)

	atomic_t ss_trig;
	unsigned int bscore; //for software balance used, busy level 0(idle) ~ max(busy)

	////for software balance debug
	unsigned int auto_cnt;
	unsigned int force_cnt;
	unsigned int bscore_cnt;

	VPE_ENG_HANDLE *eng_hdl;

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

	struct vpe_drv_tsk ctl_proc_tsk;
	struct vpe_drv_tsk ctl_cb_tsk; //for ctl cb tasklet

#if (SIM_HW_ENG == ENABLE)
	struct vpe_drv_tsk sim_isr_proc; //sim hw isr callback
#endif

	s32 ts_idx;
	struct vpe_drv_ctl_ts ts[TS_MAX_CNT];

	VPE_DRV_CFG col_cfg;
};

struct vpe_drv_ctl_swb_info {
	int eng_id;
	struct vos_list_head list;
};

struct vpe_drv_ctl_chip {
	struct vpe_drv_ctl_ch *last_trig_ch; //only support single handle put job.
										 //if multi handle put job case, must move this information to kdrv
	struct vpe_drv_ctl_swb_info swb_info[VPE_DRV_ENG_MAX];
	struct vos_list_head swb_list_root;
};

struct vpe_drv_ctl_log_info {
	unsigned long time;		//time
	unsigned char event[4];
	unsigned int param[2];
};

struct vpe_drv_ctl {
	u16 chip_num;
	u16 eng_num;
	u16 total_ch;
	enum vpe_drv_fire_mode fire_mode; //0:single fire mode, 1:Link-List mode
	u16 gating_en;	//gating switch 1:enable, 0:disable

    vk_spinlock_t eng_lock;
	u16	eng_open;

	unsigned long chip_hdl;
	struct vpe_drv_ctl_chip *chip;
    vk_spinlock_t chip_lock;

	unsigned long ch_hdl;
	struct vpe_drv_ctl_ch *ch;
    vk_spinlock_t ch_lock;

	unsigned long ctl_job_head_pool_hdl;						//cache memory for drv ctl job head
	unsigned long ctl_job_eng_col_head_pool_hdl;				//cache memory for drv eng col head(for eng col used)

	unsigned long ctl_job_pool_hdl;								//keep ctl free job entry (vpe_drv_ctl_job)
	struct vpe_drv_ctl_job *ctl_job;
    vk_spinlock_t ctl_job_lock;
	u16 ctl_job_free_cnt;
	u16 ctl_job_max_cnt;
	struct vos_list_head ctl_job_free_root;

	unsigned long lut2d_pool_hdl;							//keep lut2d buffer
	void *lut2d_va;

#if (PROC_DET_CMD == ENABLE)
	THREAD_HANDLE det_thread_hdl;
	int det_wakeup_flag;
#endif

	uintptr_t log_buf_hdl;
    struct vpe_drv_ctl_log_info *log_pool;
	unsigned int log_flag;	//1: enable log, 0: disable log
	unsigned int log_idx;
	unsigned int log_max_num;
	unsigned int log_repeat;
    vk_spinlock_t log_lock;
};

int vpe_drv_ctl_init(struct vpe_drv_ctl *pctl, int chip_num, int eng_num);
int vpe_drv_ctl_uninit(struct vpe_drv_ctl *pctl);
int vpe_drv_ctl_dump_info(void);
int vpe_drv_ctl_dump_job_info(void);
int vpe_drv_ctl_dump_ll_list_info(int dump_eng_ll, int dump_wait_ll);
int vpe_drv_ctl_dump_ts(void);
void vpe_drv_ctl_det_proc_switch(int enable);
void vpe_drv_ctl_sw_balance_set_enable(u16 flag);
void vpe_drv_ctl_sw_balance_set_mask(u16 mask);
int vpe_drv_ctl_dump_sw_balance_cnt(void);
void vpe_drv_ctl_sw_balance_cnt_reset(void);
void vpe_drv_ctl_eng_col_set_enable(u16 flag);
int vpe_drv_ctl_dump_util(void *sfile);
int vpe_drv_ctl_dump_gating(void *sfile);
void vpe_drv_ctl_set_gating(int flag);
int vpe_drv_ctl_suspend(int ch_idx);
int vpe_drv_ctl_resume(int ch_idx);
enum vpe_drv_sts vpe_drv_ctl_put_job_with_callback(struct vpe_drv_job_head *head, struct vpe_drv_ctl_job_callback_ops *ops);
int vpe_drv_ctl_log_open(unsigned int max_num);
int vpe_drv_ctl_log_close(void);
int vpe_drv_ctl_log_trig(unsigned int st_flag);
void vpe_drv_ctl_log_dump(char *filename);
void vpe_drv_ctl_eng_open(void);
void vpe_drv_ctl_eng_close(void);

extern int vpe_drv_module_dump_info(void);
extern void vpe_drv_eng_dump_reg(void);
#endif  //_VPE_DRV_CTL_INT_H_
