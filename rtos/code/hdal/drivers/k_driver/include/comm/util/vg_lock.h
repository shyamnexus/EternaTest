/*
 *   @file   vg_lock.h
 *
 *   @brief  vg lock header file.
 *
 *   A rewrite version of lock function for vg.
 *
 *   Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
 */

#ifndef __VG_LOCK_H__
#define __VG_LOCK_H__

#include "kwrap/spinlock.h"

#define USE_VG_LOCK          1
#define VG_LOCK_DEBUG        0
#define VG_LOCK_DEBUG_TIME   0

#if USE_VG_LOCK

#define VG_LOCK_ST_MAGIC        0x6894744F
#define VG_LOCK_FNC_NAME_LEN    32

typedef struct {
	vk_spinlock_t  lock;
	struct task_struct  *task;
	unsigned int lock_cnt;
	unsigned int magic;
	unsigned long flags;

#if VG_LOCK_DEBUG_TIME
	// A -> spin_lock -> B -> (run somthing) -> C -> spin_unlock
	unsigned long wait_time;  // wait_time = B - A
	unsigned long lock_time;  // lock_time = C - B
	unsigned long tmp_jiffies;
#endif
} vg_spinlock_t;

#define __TYPE_SPINLOCK                 0
#define __TYPE_SPINLOCK_IRQ             1
#define __TYPE_SPINLOCK_IRQSAVE         2
#define __TYPE_SEMAPHORE                3

void __vg_raw_lock_init(vg_spinlock_t *vg_spin_lock, int type, const char *func);
void __vg_raw_lock(vg_spinlock_t *vg_spin_lock, unsigned long *flags, int type, const char *func);
void __vg_raw_unlock(vg_spinlock_t *vg_spin_lock, unsigned long *flags, int type, const char *func);

#if VG_LOCK_DEBUG
void vg_lock_debug_set_tag(vg_spinlock_t *vg_spin_lock, int tag);
#endif

#define VG_DEFINE_SPINLOCK(lockname)                    \
	vg_spinlock_t lockname = {                          \
														.lock = __VK_SPIN_LOCK_UNLOCKED(lockname.lock),    \
														.task = NULL,                                   \
														.lock_cnt = 0,                                  \
														.magic = VG_LOCK_ST_MAGIC                       \
							 };

#define vg_spin_lock_init(p_vg_spin_lock)                   __vg_raw_lock_init(p_vg_spin_lock, __TYPE_SPINLOCK, __func__)

#define vg_spin_lock(p_vg_spin_lock)                        __vg_raw_lock(p_vg_spin_lock, 0, __TYPE_SPINLOCK, __func__)

#define vg_spin_lock_irq(p_vg_spin_lock)                    __vg_raw_lock(p_vg_spin_lock, 0, __TYPE_SPINLOCK_IRQ, __func__)

#define vg_spin_lock_irqsave(p_vg_spin_lock, flags)         __vg_raw_lock(p_vg_spin_lock, &flags, __TYPE_SPINLOCK_IRQSAVE, __func__)

#define vg_spin_unlock(p_vg_spin_lock)                      __vg_raw_unlock(p_vg_spin_lock, 0, __TYPE_SPINLOCK, __func__)

#define vg_spin_unlock_irqrestore(p_vg_spin_lock, flags)    __vg_raw_unlock(p_vg_spin_lock, &flags, __TYPE_SPINLOCK_IRQSAVE, __func__)

#else

#define vg_spinlock_t                                       vk_spinlock_t

#define VG_DEFINE_SPINLOCK(lockname)                        VK_DEFINE_SPINLOCK(lockname)

#define vg_spin_lock_init(p_vg_spin_lock)                   vk_spin_lock_init(p_vg_spin_lock)

#define vg_spin_lock(p_vg_spin_lock)                        vk_spin_lock(p_vg_spin_lock)

#define vg_spin_lock_irq(p_vg_spin_lock)                    vk_spin_lock_irq(p_vg_spin_lock)

#define vg_spin_lock_irqsave(p_vg_spin_lock, flags)         vk_spin_lock_irqsave(p_vg_spin_lock, flags)

#define vg_spin_unlock(p_vg_spin_lock)                      vk_spin_unlock(p_vg_spin_lock)

#define vg_spin_unlock_irqrestore(p_vg_spin_lock, flags)    vk_spin_unlock_irqrestore(p_vg_spin_lock, flags)


#endif


#endif /* __VG_LOCK_H__ */
