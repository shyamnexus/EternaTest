/*
 *   @file   vg_timer.h
 *
 *   @brief  vg timer header.
 *
 *   A rewrite version of timer function for vg.
 *
 *   Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
 */

#ifndef __VG_TIMER_H__
#define __VG_TIMER_H__

#include <linux/timer.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include "kwrap/spinlock.h"


#define MAX_TIMER_NAME_LEN  31
typedef struct vg_timer_tag {
	void (*func)(void *);
	void *data;

	struct task_struct *task;
	struct timer_list  timer;
	int nice;
	unsigned int st_magic;
	int is_thread_job;  //0:init state, 1:will do it at least once, 2:prepare to exit kthread
	vk_spinlock_t thread_lock;
	char name[MAX_TIMER_NAME_LEN + 1];
	struct task_struct *p_destroy_task;
	int free_when_thread_end;
	wait_queue_head_t wait_queue;

} vg_timer_t;

typedef struct vg_hrtimer_tag {
	unsigned int st_magic;
	struct hrtimer sim_ref_clk;
} vg_hrtimer_t;

int  vg_timer_init(vg_timer_t *vg_timer, char *name);
int  vg_timer_exit(vg_timer_t *vg_timer);
int  vg_timer_start(vg_timer_t *vg_timer, unsigned long delay_ms,
		    void (*func)(void *data), void *data);
int  vg_timer_cancel(vg_timer_t *vg_timer);
void  vg_timer_set_nice(vg_timer_t *vg_timer, int nice);

void  vg_hrtimer_init(vg_hrtimer_t *sim_clk_timer);
int  vg_hrtimer_start(vg_hrtimer_t *sim_clk_timer, enum hrtimer_restart(*handler)(struct hrtimer *timer), unsigned long delay_ms);
int  vg_hrtimer_cancel(vg_hrtimer_t *sim_clk_timer);

#endif /* __VG_TIMER_H__ */

