/**
    NVT timer function
    @file       nvt-timer.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef __SOC_NVT_IVOT_NVT_TIMER_H
#define __SOC_NVT_IVOT_NVT_TIMER_H

#include <linux/semaphore.h>

typedef int (*nvt_timer_callback_t)(void);

struct nvt_timer_t {
	struct device		*dev;
	struct clk		*clk;
	struct list_head	list;
	void __iomem		*va;
	u64			count;
	u64			target_count;
	unsigned long		freq;
	nvt_timer_callback_t	callback;
	spinlock_t		lock;
	unsigned int		index;
	bool			irqen;
	bool			in_use;
	bool			freerun;
};

int nvt_timer_get_count(unsigned int num, u64 *count);
int nvt_timer_shutdown(unsigned int num);
int nvt_timer_setup(unsigned int num, bool freerun, u64 count, nvt_timer_callback_t callback);
int nvt_timer_reload(unsigned int num, u64 count);
int nvt_timer_stop(unsigned int num);
int nvt_timer_start(unsigned int num);
void nvt_timer_dump(void);

void nvt_timer_proc_exit(void);
int nvt_timer_proc_init(void);
void hrtimer_enable_extcnt(u32 en_mask);
u32 hrtimer_get_extcnt_index(int ext_cnt_id);
u64 hrtimertest_get_extcnt_count(int ext_cnt_id, int buf_index);

#endif /* __SOC_NVT_IVOT_NVT_TIMER_H */
