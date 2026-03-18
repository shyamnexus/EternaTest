/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _VOS_WAIT_H
#define _VOS_WAIT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Linux wait queue related types and methods
 */
#include <kwrap/list.h>
#include <kwrap/lockdep.h>
#include <kwrap/spinlock.h>

#if (defined(__UITRON) || defined(__ECOS))
#elif defined(__FREERTOS)
#elif defined(__LINUX) && defined(__KERNEL__)

/*----------------------------------------------------------------------------*/
/* Type declaration (Linux)                                                   */
/*----------------------------------------------------------------------------*/
typedef struct vk_wait_queue_entry vk_wait_queue_entry_t;

typedef int (*vk_wait_queue_func_t)(struct vk_wait_queue_entry *wq_entry, unsigned mode, int flags, void *key);

/* vk_wait_queue_entry::flags */
#define VK_WQ_FLAG_EXCLUSIVE    0x01
#define VK_WQ_FLAG_WOKEN        0x02
#define VK_WQ_FLAG_BOOKMARK     0x04

/*
 * A single wait-queue entry structure:
 */
struct vk_wait_queue_entry {
	unsigned int flags;
	void *private;
	vk_wait_queue_func_t func;
	struct vos_list_head entry;
};

struct vk_wait_queue_head {
	vk_raw_spinlock_t lock;
	struct vos_list_head head;
};
typedef struct vk_wait_queue_head vk_wait_queue_head_t;

/*----------------------------------------------------------------------------*/
/* Internal use only, do NOT use directly (Linux)                             */
/*----------------------------------------------------------------------------*/

/*
 * Macros for declaration and initialisaton of the datatypes
 */

#define __VK_WAIT_QUEUE_HEAD_INITIALIZER(name) {					\
	.lock		= __VK_SPIN_LOCK_UNLOCKED(name.lock),			\
	.head		= { &(name).head, &(name).head } }

#define VK_DECLARE_WAIT_QUEUE_HEAD(name) \
	struct vk_wait_queue_head name = __VK_WAIT_QUEUE_HEAD_INITIALIZER(name)

extern void __vk_init_waitqueue_head(struct vk_wait_queue_head *wq_head, const char *name, struct vk_lock_class_key *);

static inline void __vk_add_wait_queue(struct vk_wait_queue_head *wq_head, struct vk_wait_queue_entry *wq_entry)
{
	vos_list_add(&wq_entry->entry, &wq_head->head);
}

/*
 * Used for wake-one threads:
 */
static inline void __vk_add_wait_queue_entry_tail(struct vk_wait_queue_head *wq_head, struct vk_wait_queue_entry *wq_entry)
{
	vos_list_add_tail(&wq_entry->entry, &wq_head->head);
}

void __vk_wake_up(struct vk_wait_queue_head *wq_head, unsigned int mode, int nr, void *key);

#define ___vk_wait_cond_timeout(condition)						\
({										\
	bool __cond = (condition);						\
	if (__cond && !__ret)							\
		__ret = 1;							\
	__cond || !__ret;							\
})

#define ___vk_wait_is_interruptible(state)						\
	(!__builtin_constant_p(state) ||					\
		state == TASK_INTERRUPTIBLE || state == TASK_KILLABLE)		\

/*
 * The below macro ___wait_event() has an explicit shadow of the __ret
 * variable when used from the wait_event_*() macros.
 *
 * This is so that both can use the ___wait_cond_timeout() construct
 * to wrap the condition.
 *
 * The type inconsistency of the wait_event_*() __ret variable is also
 * on purpose; we use long where we can return timeout values and int
 * otherwise.
 */

#define ___vk_wait_event(wq_head, condition, state, exclusive, ret, cmd)		\
({										\
	__label__ __out;							\
	struct vk_wait_queue_entry __wq_entry;					\
	long __ret = ret;	/* explicit shadow */				\
										\
	vk_init_wait_entry(&__wq_entry, exclusive ? VK_WQ_FLAG_EXCLUSIVE : 0);	\
	for (;;) {								\
		long __int = vk_prepare_to_wait_event(&wq_head, &__wq_entry, state);\
										\
		if (condition)							\
			break;							\
										\
		if (___vk_wait_is_interruptible(state) && __int) {			\
			__ret = __int;						\
			goto __out;						\
		}								\
										\
		cmd;								\
	}									\
	vk_finish_wait(&wq_head, &__wq_entry);					\
__out:	__ret;									\
})

#define __vk_wait_event(wq_head, condition)					\
	(void)___vk_wait_event(wq_head, condition, TASK_UNINTERRUPTIBLE, 0, 0,	\
			    vk_schedule())

#define __vk_wait_event_cmd(wq_head, condition, cmd1, cmd2)			\
	(void)___vk_wait_event(wq_head, condition, TASK_UNINTERRUPTIBLE, 0, 0,	\
			    cmd1; vk_schedule(); cmd2)

#define __vk_wait_event_interruptible(wq_head, condition)				\
	___vk_wait_event(wq_head, condition, TASK_INTERRUPTIBLE, 0, 0,		\
		      vk_schedule())

#define __vk_wait_event_interruptible_timeout(wq_head, condition, timeout)		\
	___vk_wait_event(wq_head, ___vk_wait_cond_timeout(condition),			\
		      TASK_INTERRUPTIBLE, 0, timeout,				\
		      __ret = vk_schedule_timeout(__ret))

/*
 * Other kernel APIs
 */
void vk_schedule(void);
signed long vk_schedule_timeout(signed long timeout);
int vk_schedule_hrtimeout(void *vk_expires, unsigned int vk_hrtimer_mode);
void vk_might_sleep(const char *file, int line);

/*----------------------------------------------------------------------------*/
/* Normal type API (Linux)                                                    */
/*----------------------------------------------------------------------------*/
#define vk_init_waitqueue_head(wq_head)						\
	do {									\
		static struct vk_lock_class_key __key;				\
										\
		__vk_init_waitqueue_head((wq_head), #wq_head, &__key);		\
	} while (0)

/**
 * waitqueue_active -- locklessly test for waiters on the queue
 * @wq_head: the waitqueue to test for waiters
 *
 * returns true if the wait list is not empty
 *
 * NOTE: this function is lockless and requires care, incorrect usage _will_
 * lead to sporadic and non-obvious failure.
 *
 * Use either while holding wait_queue_head::lock or when used for wakeups
 * with an extra smp_mb() like:
 *
 *      CPU0 - waker                    CPU1 - waiter
 *
 *                                      for (;;) {
 *      @cond = true;                     prepare_to_wait(&wq_head, &wait, state);
 *      smp_mb();                         // smp_mb() from set_current_state()
 *      if (waitqueue_active(wq_head))         if (@cond)
 *        wake_up(wq_head);                      break;
 *                                        schedule();
 *                                      }
 *                                      finish_wait(&wq_head, &wait);
 *
 * Because without the explicit smp_mb() it's possible for the
 * waitqueue_active() load to get hoisted over the @cond store such that we'll
 * observe an empty wait list while the waiter might not observe @cond.
 *
 * Also note that this 'optimization' trades a spin_lock() for an smp_mb(),
 * which (when the lock is uncontended) are of roughly equal cost.
 */
static inline int vk_waitqueue_active(struct vk_wait_queue_head *wq_head)
{
	return !vos_list_empty(&wq_head->head);
}

#define vk_wake_up(x)			__vk_wake_up(x, TASK_NORMAL, 1, NULL)

extern void vk_init_wait_entry(struct vk_wait_queue_entry *wq_entry, int flags);

/**
 * wait_event - sleep until a condition gets true
 * @wq_head: the waitqueue to wait on
 * @condition: a C expression for the event to wait for
 *
 * The process is put to sleep (TASK_UNINTERRUPTIBLE) until the
 * @condition evaluates to true. The @condition is checked each time
 * the waitqueue @wq_head is woken up.
 *
 * wake_up() has to be called after changing any variable that could
 * change the result of the wait condition.
 */
#define vk_wait_event(wq_head, condition)						\
do {										\
	vk_might_sleep(__FILE__, __LINE__);								\
	if (condition)								\
		break;								\
	__vk_wait_event(wq_head, condition);					\
} while (0)

#define __vk_wait_event_timeout(wq_head, condition, timeout)			\
	___vk_wait_event(wq_head, ___vk_wait_cond_timeout(condition),			\
		      TASK_UNINTERRUPTIBLE, 0, timeout,				\
		      __ret = vk_schedule_timeout(__ret))

/**
 * wait_event_timeout - sleep until a condition gets true or a timeout elapses
 * @wq_head: the waitqueue to wait on
 * @condition: a C expression for the event to wait for
 * @timeout: timeout, in jiffies
 *
 * The process is put to sleep (TASK_UNINTERRUPTIBLE) until the
 * @condition evaluates to true. The @condition is checked each time
 * the waitqueue @wq_head is woken up.
 *
 * wake_up() has to be called after changing any variable that could
 * change the result of the wait condition.
 *
 * Returns:
 * 0 if the @condition evaluated to %false after the @timeout elapsed,
 * 1 if the @condition evaluated to %true after the @timeout elapsed,
 * or the remaining jiffies (at least 1) if the @condition evaluated
 * to %true before the @timeout elapsed.
 */
#define vk_wait_event_timeout(wq_head, condition, timeout)				\
({										\
	long __ret = timeout;							\
	vk_might_sleep(__FILE__, __LINE__);								\
	if (!___vk_wait_cond_timeout(condition))					\
		__ret = __vk_wait_event_timeout(wq_head, condition, timeout);	\
	__ret;									\
})

/**
 * wait_event_interruptible - sleep until a condition gets true
 * @wq_head: the waitqueue to wait on
 * @condition: a C expression for the event to wait for
 *
 * The process is put to sleep (TASK_INTERRUPTIBLE) until the
 * @condition evaluates to true or a signal is received.
 * The @condition is checked each time the waitqueue @wq_head is woken up.
 *
 * wake_up() has to be called after changing any variable that could
 * change the result of the wait condition.
 *
 * The function will return -ERESTARTSYS if it was interrupted by a
 * signal and 0 if @condition evaluated to true.
 */
#define vk_wait_event_interruptible(wq_head, condition)				\
({										\
	int __ret = 0;								\
	vk_might_sleep(__FILE__, __LINE__);								\
	if (!(condition))							\
		__ret = __vk_wait_event_interruptible(wq_head, condition);		\
	__ret;									\
})

/**
 * wait_event_interruptible_timeout - sleep until a condition gets true or a timeout elapses
 * @wq_head: the waitqueue to wait on
 * @condition: a C expression for the event to wait for
 * @timeout: timeout, in jiffies
 *
 * The process is put to sleep (TASK_INTERRUPTIBLE) until the
 * @condition evaluates to true or a signal is received.
 * The @condition is checked each time the waitqueue @wq_head is woken up.
 *
 * wake_up() has to be called after changing any variable that could
 * change the result of the wait condition.
 *
 * Returns:
 * 0 if the @condition evaluated to %false after the @timeout elapsed,
 * 1 if the @condition evaluated to %true after the @timeout elapsed,
 * the remaining jiffies (at least 1) if the @condition evaluated
 * to %true before the @timeout elapsed, or -%ERESTARTSYS if it was
 * interrupted by a signal.
 */
#define vk_wait_event_interruptible_timeout(wq_head, condition, timeout)	\
({										\
	long __ret = timeout;							\
	vk_might_sleep(__FILE__, __LINE__);								\
	if (!___vk_wait_cond_timeout(condition))					\
		__ret = __vk_wait_event_interruptible_timeout(wq_head,		\
						condition, timeout);		\
	__ret;									\
})

/*
 * Waitqueues which are removed from the waitqueue_head at wakeup time
 */
long vk_prepare_to_wait_event(struct vk_wait_queue_head *wq_head, struct vk_wait_queue_entry *wq_entry, int state);
void vk_finish_wait(struct vk_wait_queue_head *wq_head, struct vk_wait_queue_entry *wq_entry);

#else
#error Not supported OS
#endif

#ifdef __cplusplus
}
#endif

#endif /* _VOS_WAIT_H */
