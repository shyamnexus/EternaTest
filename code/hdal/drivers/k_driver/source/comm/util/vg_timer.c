/*
 *   @file   vg_timer.c
 *
 *   @brief  VG Timer functions.
 *
 *   Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
 */

#define __VG_TIMER_C__

#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/version.h>
#include "log.h"
#include "vg_timer.h"

#include "kwrap/util.h"

#define TMR_PANIC(fmt, args...) \
	printm("VG", fmt, ##args);    \
	printk(fmt, ##args);          \
	damnit("VG");


#define VG_TIMER_MAGIC  0x17117717

static int vg_timer_thread(void *p_data)
{
	vg_timer_t *p_vg_timer = (vg_timer_t *) p_data;

	set_user_nice(current, p_vg_timer->nice);
	p_vg_timer->p_destroy_task = current;
	while (!kthread_should_stop()) {
		if (wait_event_timeout(p_vg_timer->wait_queue, p_vg_timer->is_thread_job != 0,
				       vos_util_msec_to_tick(1000)) <= 0) {
			continue;
		}

		if (p_vg_timer->is_thread_job == 2) {
			break;
		}

		if (p_vg_timer->is_thread_job == 1) {
			p_vg_timer->is_thread_job = 0;
			p_vg_timer->func(p_vg_timer->data);
		}

	}

	if (p_vg_timer->free_when_thread_end == 1) {
		/* no need to free */;
	}
	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
static void vg_timer_wake_up_fn(unsigned long data)
{
	vg_timer_t *p_vg_timer = (vg_timer_t *) data;
#else
static void vg_timer_wake_up_fn(struct timer_list *t)
{
	vg_timer_t *p_vg_timer = from_timer(p_vg_timer, t, timer);
#endif

	unsigned long flags;

	vk_spin_lock_irqsave(&p_vg_timer->thread_lock, flags);
	p_vg_timer->is_thread_job = 1;
	wake_up(&p_vg_timer->wait_queue);
	vk_spin_unlock_irqrestore(&p_vg_timer->thread_lock, flags);
}

/**************************************************************************************************/
/**************************************************************************************************/

int  vg_timer_exit(vg_timer_t *p_vg_timer)
{
	unsigned long flags;
	int is_idle_to_stop = 0;

	if (p_vg_timer->st_magic != VG_TIMER_MAGIC) {
		TMR_PANIC("Invalid vg_timer(0x%p)\n", p_vg_timer);
		goto exit;
	}

	is_idle_to_stop = 1;
	vk_spin_lock_irqsave(&p_vg_timer->thread_lock, flags);
	p_vg_timer->is_thread_job = 2;
	wake_up(&p_vg_timer->wait_queue);
	vg_timer_cancel(p_vg_timer);
	vk_spin_unlock_irqrestore(&p_vg_timer->thread_lock, flags);

	if (p_vg_timer->p_destroy_task != current) {
		kthread_stop(p_vg_timer->task);
	} else {
		p_vg_timer->free_when_thread_end = 1;
	}

exit:
	return 0;
}
EXPORT_SYMBOL(vg_timer_exit);

int  vg_timer_init(vg_timer_t *p_vg_timer, char *name)
{
	int ret = -1;
	struct task_struct *task;

	if (!p_vg_timer) {
		goto exit;
	}

	p_vg_timer->is_thread_job = 0;
	init_waitqueue_head(&p_vg_timer->wait_queue);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	init_timer(&p_vg_timer->timer);
#endif
	p_vg_timer->nice = 0;
	p_vg_timer->st_magic = VG_TIMER_MAGIC;
	p_vg_timer->free_when_thread_end = 0;
	vk_spin_lock_init(&p_vg_timer->thread_lock);
	sprintf(p_vg_timer->name, "%s", name);
	strncpy(p_vg_timer->name, name, MAX_TIMER_NAME_LEN);
	task = kthread_create(vg_timer_thread, p_vg_timer, "%s%s", "gm_", name);
	if (IS_ERR(task)) {
		TMR_PANIC("Create timer thread failed. name(%s)\n", name);
		goto exit;

	}
	p_vg_timer->task = task;
	wake_up_process(task);

	ret = 0;
exit:
	return ret;
}
EXPORT_SYMBOL(vg_timer_init);


int  vg_timer_start(vg_timer_t *vg_timer, unsigned long delay_ms,
		    void (*func)(void *data), void *data)
{
	vg_timer_t *p_vg_timer = (vg_timer_t *) vg_timer;
	struct timer_list *p_timer;
	unsigned long flags;

	vk_spin_lock_irqsave(&p_vg_timer->thread_lock, flags);
	if (vg_timer->st_magic != VG_TIMER_MAGIC) {
		TMR_PANIC("Invalid vg_timer(0x%p)\n", vg_timer);
		goto exit;
	}
	if (delay_ms > 10000) {
		TMR_PANIC("Probably Error! Timer delay(%lums) very large?\n", delay_ms);
		goto exit;
	}

	p_timer = &vg_timer->timer;
	if (func) {
		p_vg_timer->func = func;
		p_vg_timer->data = data;
	} else {
		goto exit;
	}

	if (delay_ms) {
		if (!timer_pending(p_timer)) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
			p_timer->function = vg_timer_wake_up_fn;
			p_timer->data = (unsigned long) vg_timer;
			p_timer->expires = jiffies + vos_util_msec_to_tick(delay_ms);
			add_timer(p_timer);
#else
			timer_setup(p_timer, vg_timer_wake_up_fn, 0);
			p_timer->expires = jiffies + vos_util_msec_to_tick(delay_ms);
			add_timer(p_timer);
#endif

		}
		vk_spin_unlock_irqrestore(&p_vg_timer->thread_lock, flags);
	} else {
		vk_spin_unlock_irqrestore(&p_vg_timer->thread_lock, flags); //unlock to avoid nested spinlock
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
		vg_timer_wake_up_fn((unsigned long) vg_timer);
#else
		vg_timer_wake_up_fn(p_timer);
#endif
	}
	return 0;

exit:
	vk_spin_unlock_irqrestore(&p_vg_timer->thread_lock, flags);
	return -1;
}
EXPORT_SYMBOL(vg_timer_start);

int  vg_timer_cancel(vg_timer_t *vg_timer)
{
	if (vg_timer->st_magic != VG_TIMER_MAGIC) {
		TMR_PANIC("Invalid vg_timer(0x%p)\n", vg_timer);
		return 0;
	}
	return del_timer_sync(&vg_timer->timer);
}
EXPORT_SYMBOL(vg_timer_cancel);



void  vg_timer_set_nice(vg_timer_t *vg_timer, int nice)
{
	if (vg_timer->st_magic != VG_TIMER_MAGIC) {
		TMR_PANIC("Invalid vg_timer(0x%p)\n", vg_timer);
		return;
	}
	vg_timer->nice = nice;

}
EXPORT_SYMBOL(vg_timer_set_nice);

void  vg_hrtimer_init(vg_hrtimer_t *sim_clk_timer)
{
	sim_clk_timer->st_magic = VG_TIMER_MAGIC;
	hrtimer_init(&sim_clk_timer->sim_ref_clk, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
}
EXPORT_SYMBOL(vg_hrtimer_init);

int  vg_hrtimer_start(vg_hrtimer_t *sim_clk_timer, enum hrtimer_restart(*handler)(struct hrtimer *timer), unsigned long delay_ms)

{
	if (sim_clk_timer->st_magic != VG_TIMER_MAGIC) {
		TMR_PANIC("Invalid vg_hrtimer(0x%p)\n", sim_clk_timer);
		return 0;
	}
	if (!handler) {
		panic("vg_hrtimer_start:check hrtimer callback fail\n");
	}
	sim_clk_timer->sim_ref_clk.function = handler;
	hrtimer_start(&sim_clk_timer->sim_ref_clk, ktime_set(delay_ms / 1000, (delay_ms % 1000) * 1000000), HRTIMER_MODE_REL);
	return 1;
}
EXPORT_SYMBOL(vg_hrtimer_start);

int  vg_hrtimer_cancel(vg_hrtimer_t *sim_clk_timer)
{
	int ret;
	ret = hrtimer_cancel(&sim_clk_timer->sim_ref_clk);
	sim_clk_timer->st_magic = 0;
	return ret;
}
EXPORT_SYMBOL(vg_hrtimer_cancel);


