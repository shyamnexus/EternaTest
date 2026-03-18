/*-----------------------------------------------------------------------------*/
/* Include Header Files                                                        */
/*-----------------------------------------------------------------------------*/
#include <asm/current.h>
#include <linux/kthread.h>

#include <kwrap/debug.h>
#include <kwrap/kthread.h>

/*-----------------------------------------------------------------------------*/
/* Local Types Declarations                                                    */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
struct vk_task_struct* vk_get_current(void)
{
	return (struct vk_task_struct *)current;
}
EXPORT_SYMBOL(vk_get_current);

struct vk_task_struct *vk_kthread_create(int (*threadfn)(void *data), void *data, const char name[])
{
	struct task_struct *native_tsk;

	native_tsk = kthread_create(threadfn, data, name);

	return (struct vk_task_struct *)native_tsk;
}
EXPORT_SYMBOL(vk_kthread_create);

struct vk_task_struct *vk_kthread_run(int (*threadfn)(void *data), void *data, const char name[])
{
	struct task_struct *native_tsk;

	native_tsk = kthread_run(threadfn, data, name);

	return (struct vk_task_struct *)native_tsk;
}
EXPORT_SYMBOL(vk_kthread_run);

int vk_wake_up_process(struct vk_task_struct *vos_tsk)
{
	struct task_struct *native_tsk = (struct task_struct *)vos_tsk;

	return wake_up_process(native_tsk);
}
EXPORT_SYMBOL(vk_wake_up_process);

int vk_kthread_stop(struct vk_task_struct *vos_tsk)
{
	struct task_struct *native_tsk = (struct task_struct *)vos_tsk;

	return kthread_stop(native_tsk);
}
EXPORT_SYMBOL(vk_kthread_stop);

bool vk_kthread_should_stop(void)
{
	return kthread_should_stop();
}
EXPORT_SYMBOL(vk_kthread_should_stop);

void vk_set_user_nice(struct vk_task_struct *vos_tsk, long nice)
{
	struct task_struct *native_tsk = (struct task_struct *)vos_tsk;

	set_user_nice(native_tsk, nice);
}
EXPORT_SYMBOL(vk_set_user_nice);

int vk_task_nice(const struct vk_task_struct *vos_tsk)
{
	struct task_struct *native_tsk = (struct task_struct *)vos_tsk;

	return task_nice(native_tsk);
}
EXPORT_SYMBOL(vk_task_nice);