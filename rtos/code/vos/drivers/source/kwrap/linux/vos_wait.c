/*
 * This file is duplicated from kernel/sched/wait.c and change all API names
 * The purpose is to replace normal spin_xxx by raw_spin_xxx for RT Linux
 */

#include <linux/hrtimer.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

#include <kwrap/nvt_type.h>
#include <kwrap/lockdep.h>
#include <kwrap/wait.h>

/*-----------------------------------------------------------------------------*/
/* Local Types Declarations                                                    */
/*-----------------------------------------------------------------------------*/
//Note:
//For RT Linux, spinlock is directed to an rt mutex and use atomic 64
//The buf should aligned to 64-bit (8 bytes)
#define GET_ALIGN_BUF(p) ALIGN_CEIL_8((unsigned long)(p)->buf)

/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/
STATIC_ASSERT(VK_WQ_FLAG_EXCLUSIVE == WQ_FLAG_EXCLUSIVE);
STATIC_ASSERT(VK_WQ_FLAG_WOKEN == WQ_FLAG_WOKEN);
STATIC_ASSERT(VK_WQ_FLAG_BOOKMARK == WQ_FLAG_BOOKMARK);
STATIC_ASSERT(sizeof(struct vk_wait_queue_entry) == sizeof(struct wait_queue_entry));

//vos wait.h needs vk_schedule(), so we put it here
//The original place of schedule() is in kernel/sched/core.c
void vk_schedule(void)
{
	schedule();
}
EXPORT_SYMBOL(vk_schedule);

//vos wait.h needs vk_schedule_timeout(), so we put it here
//The original place of schedule_timeout() is in kernel/sched/core.c
signed long vk_schedule_timeout(signed long timeout)
{
	return schedule_timeout(timeout);
}
EXPORT_SYMBOL(vk_schedule_timeout);

//put vk_schedule_hrtimeout next to vk_schedule_timeout
//The original place of schedule_hrtimeout() is in kernel/time/hrtimer.c
int vk_schedule_hrtimeout(void *vk_expires, unsigned int vk_hrtimer_mode)
{
	ktime_t *native_ktime = (ktime_t *)vk_expires;
	const enum hrtimer_mode native_mode = (enum hrtimer_mode)vk_hrtimer_mode;

	BUILD_BUG_ON(sizeof(enum hrtimer_mode) != sizeof(unsigned int));

	return schedule_hrtimeout(native_ktime, native_mode);
}
EXPORT_SYMBOL(vk_schedule_hrtimeout);

void vk_might_sleep(const char *file, int line)
{
#ifdef CONFIG_DEBUG_ATOMIC_SLEEP
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
	__might_sleep(file, line);
#else
	__might_sleep(file, line, 0);
#endif
	might_resched();
#else
	might_resched();
#endif
}
EXPORT_SYMBOL(vk_might_sleep);

int vk_autoremove_wake_function(struct vk_wait_queue_entry *wq_entry, unsigned mode, int sync, void *key)
{
	STATIC_ASSERT(sizeof(wait_queue_entry_t) == sizeof(struct vk_wait_queue_entry));

	int ret = default_wake_function((wait_queue_entry_t *)wq_entry, mode, sync, key);

	if (ret)
		vos_list_del_init(&wq_entry->entry);

	return ret;
}
EXPORT_SYMBOL(vk_autoremove_wake_function);

void __vk_init_waitqueue_head(struct vk_wait_queue_head *wq_head, const char *name, struct vk_lock_class_key *key)
{
	vk_raw_spin_lock_init(&wq_head->lock);
	vk_lockdep_set_class_and_name(&wq_head->lock, key, name);
	VOS_INIT_LIST_HEAD(&wq_head->head);
}
EXPORT_SYMBOL(__vk_init_waitqueue_head);

/*
 * Scan threshold to break wait queue walk.
 * This allows a waker to take a break from holding the
 * wait queue lock during the wait queue walk.
 */
#define VK_WAITQUEUE_WALK_BREAK_CNT 64

/*
 * The core wakeup function. Non-exclusive wakeups (nr_exclusive == 0) just
 * wake everything up. If it's an exclusive wakeup (nr_exclusive == small +ve
 * number) then we wake all the non-exclusive tasks and one exclusive task.
 *
 * There are circumstances in which we can try to wake a task which has already
 * started to run but is not in state TASK_RUNNING. try_to_wake_up() returns
 * zero in this (rare) case, and we handle it by continuing to scan the queue.
 */
static int __vk_wake_up_common(struct vk_wait_queue_head *wq_head, unsigned int mode,
			int nr_exclusive, int wake_flags, void *key,
			vk_wait_queue_entry_t *bookmark)
{
	vk_wait_queue_entry_t *curr, *next;
	int cnt = 0;

	vk_lockdep_assert_held(&wq_head->lock);

	if (bookmark && (bookmark->flags & VK_WQ_FLAG_BOOKMARK)) {
		curr = vos_list_next_entry(bookmark, entry);

		vos_list_del(&bookmark->entry);
		bookmark->flags = 0;
	} else
		curr = vos_list_first_entry(&wq_head->head, vk_wait_queue_entry_t, entry);

	if (&curr->entry == &wq_head->head)
		return nr_exclusive;

	vos_list_for_each_entry_safe_from(curr, next, &wq_head->head, entry) {
		unsigned flags = curr->flags;
		int ret;

		if (flags & VK_WQ_FLAG_BOOKMARK)
			continue;

		ret = curr->func(curr, mode, wake_flags, key);
		if (ret < 0)
			break;
		if (ret && (flags & VK_WQ_FLAG_EXCLUSIVE) && !--nr_exclusive)
			break;

		if (bookmark && (++cnt > VK_WAITQUEUE_WALK_BREAK_CNT) &&
				(&next->entry != &wq_head->head)) {
			bookmark->flags = VK_WQ_FLAG_BOOKMARK;
			vos_list_add_tail(&bookmark->entry, &next->entry);
			break;
		}
	}

	return nr_exclusive;
}

static void __vk_wake_up_common_lock(struct vk_wait_queue_head *wq_head, unsigned int mode,
			int nr_exclusive, int wake_flags, void *key)
{
	unsigned long flags;
	vk_wait_queue_entry_t bookmark;

	bookmark.flags = 0;
	bookmark.private = NULL;
	bookmark.func = NULL;
	VOS_INIT_LIST_HEAD(&bookmark.entry);

	vk_raw_spin_lock_irqsave(&wq_head->lock, flags);
	nr_exclusive = __vk_wake_up_common(wq_head, mode, nr_exclusive, wake_flags, key, &bookmark);
	vk_raw_spin_unlock_irqrestore(&wq_head->lock, flags);

	while (bookmark.flags & VK_WQ_FLAG_BOOKMARK) {
		vk_raw_spin_lock_irqsave(&wq_head->lock, flags);
		nr_exclusive = __vk_wake_up_common(wq_head, mode, nr_exclusive,
						wake_flags, key, &bookmark);
		vk_raw_spin_unlock_irqrestore(&wq_head->lock, flags);
	}
}

/**
 * __wake_up - wake up threads blocked on a waitqueue.
 * @wq_head: the waitqueue
 * @mode: which threads
 * @nr_exclusive: how many wake-one or wake-many threads to wake up
 * @key: is directly passed to the wakeup function
 *
 * If this function wakes up a task, it executes a full memory barrier before
 * accessing the task state.
 */
void __vk_wake_up(struct vk_wait_queue_head *wq_head, unsigned int mode,
			int nr_exclusive, void *key)
{
	__vk_wake_up_common_lock(wq_head, mode, nr_exclusive, 0, key);
}
EXPORT_SYMBOL(__vk_wake_up);

void vk_init_wait_entry(struct vk_wait_queue_entry *wq_entry, int flags)
{
	wq_entry->flags = flags;
	wq_entry->private = current;
	wq_entry->func = vk_autoremove_wake_function;
	VOS_INIT_LIST_HEAD(&wq_entry->entry);
}
EXPORT_SYMBOL(vk_init_wait_entry);

long vk_prepare_to_wait_event(struct vk_wait_queue_head *wq_head, struct vk_wait_queue_entry *wq_entry, int state)
{
	unsigned long flags;
	long ret = 0;

	vk_raw_spin_lock_irqsave(&wq_head->lock, flags);
	if (unlikely(signal_pending_state(state, current))) {
		/*
		 * Exclusive waiter must not fail if it was selected by wakeup,
		 * it should "consume" the condition we were waiting for.
		 *
		 * The caller will recheck the condition and return success if
		 * we were already woken up, we can not miss the event because
		 * wakeup locks/unlocks the same wq_head->lock.
		 *
		 * But we need to ensure that set-condition + wakeup after that
		 * can't see us, it should wake up another exclusive waiter if
		 * we fail.
		 */
		vos_list_del_init(&wq_entry->entry);
		ret = -ERESTARTSYS;
	} else {
		if (vos_list_empty(&wq_entry->entry)) {
			if (wq_entry->flags & VK_WQ_FLAG_EXCLUSIVE)
				__vk_add_wait_queue_entry_tail(wq_head, wq_entry);
			else
				__vk_add_wait_queue(wq_head, wq_entry);
		}
		set_current_state(state);
	}
	vk_raw_spin_unlock_irqrestore(&wq_head->lock, flags);

	return ret;
}
EXPORT_SYMBOL(vk_prepare_to_wait_event);

/**
 * finish_wait - clean up after waiting in a queue
 * @wq_head: waitqueue waited on
 * @wq_entry: wait descriptor
 *
 * Sets current thread back to running state and removes
 * the wait descriptor from the given waitqueue if still
 * queued.
 */
void vk_finish_wait(struct vk_wait_queue_head *wq_head, struct vk_wait_queue_entry *wq_entry)
{
	unsigned long flags;

	__set_current_state(TASK_RUNNING);
	/*
	 * We can check for list emptiness outside the lock
	 * IFF:
	 *  - we use the "careful" check that verifies both
	 *    the next and prev pointers, so that there cannot
	 *    be any half-pending updates in progress on other
	 *    CPU's that we haven't seen yet (and that might
	 *    still change the stack area.
	 * and
	 *  - all other users take the lock (ie we can only
	 *    have _one_ other CPU that looks at or modifies
	 *    the list).
	 */
	if (!vos_list_empty_careful(&wq_entry->entry)) {
		vk_raw_spin_lock_irqsave(&wq_head->lock, flags);
		vos_list_del_init(&wq_entry->entry);
		vk_raw_spin_unlock_irqrestore(&wq_head->lock, flags);
	}
}
EXPORT_SYMBOL(vk_finish_wait);
