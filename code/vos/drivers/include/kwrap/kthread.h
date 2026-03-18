#ifndef _VOS_KTHREAD_H_
#define _VOS_KTHREAD_H_
#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/
/* Including Files                                                            */
/*----------------------------------------------------------------------------*/
#include <kwrap/nvt_type.h>

#if (defined(__UITRON) || defined(__ECOS))

#elif defined(__FREERTOS)

#elif defined(__LINUX) && defined(__KERNEL__)

/*----------------------------------------------------------------------------*/
/* Type declaration (Linux)                                                   */
/*----------------------------------------------------------------------------*/
struct vk_task_struct; //a dummy structure, no practical use

#define vk_current vk_get_current()
/*----------------------------------------------------------------------------*/
/* Internal use only, do NOT use directly (Linux)                             */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Normal type API (Linux)                                                    */
/*----------------------------------------------------------------------------*/
struct vk_task_struct* vk_get_current(void);
struct vk_task_struct* vk_kthread_create(int (*threadfn)(void *data), void *data, const char name[]);
struct vk_task_struct* vk_kthread_run(int (*threadfn)(void *data), void *data, const char name[]);

//Return: 1 if the process was woken up, 0 if it was already running
//Ref: kernel/sched/core.c wake_up_process()
int vk_wake_up_process(struct vk_task_struct *vos_tsk);

int vk_kthread_stop(struct vk_task_struct *vos_tsk);
bool vk_kthread_should_stop(void);

void vk_set_user_nice(struct vk_task_struct *vos_tsk, long nice);
int vk_task_nice(const struct vk_task_struct *vos_tsk);

#else
#error Not supported OS
#endif

#ifdef __cplusplus
}
#endif

#endif /* _VOS_KTHREAD_H_ */

