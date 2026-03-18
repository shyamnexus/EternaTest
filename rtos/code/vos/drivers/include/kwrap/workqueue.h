#ifndef _VOS_WORKQUEUE_H_
#define _VOS_WORKQUEUE_H_
#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/
/* Including Files                                                            */
/*----------------------------------------------------------------------------*/
#include <kwrap/nvt_type.h>
#include <kwrap/lockdep.h>
#include <kwrap/timer.h>

#if (defined(__UITRON) || defined(__ECOS))

#elif defined(__FREERTOS)

/*----------------------------------------------------------------------------*/
/* Type declaration (FreeRTOS)                                                */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Internal use only, do NOT use directly (FreeRTOS)                          */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Normal type API (FreeRTOS)                                                 */
/*----------------------------------------------------------------------------*/

#elif defined(__LINUX) && defined(__KERNEL__)

#if VOS_RT_LINUX
#define VK_DWORK_BUF_SZ 62
#else
#if defined (CONFIG_LOCK_STAT)
#define VK_DWORK_BUF_SZ 62
#else
#define VK_DWORK_BUF_SZ 46
#endif
#endif

/*----------------------------------------------------------------------------*/
/* Type declaration (Linux)                                                   */
/*----------------------------------------------------------------------------*/
struct vk_workqueue_struct;
struct vk_work_struct;
struct vk_delayed_work;

typedef void (*vk_work_func_t)(struct vk_work_struct *vos_work);

struct vk_delayed_work {
	unsigned long long init_tag;
	unsigned int buf[VK_DWORK_BUF_SZ];
};
/*----------------------------------------------------------------------------*/
/* Internal use only, do NOT use directly (Linux)                             */
/*----------------------------------------------------------------------------*/
void __vk_init_delayed_work(struct vk_delayed_work *vos_dwork, vk_work_func_t func, unsigned int flags);

/*----------------------------------------------------------------------------*/
/* Normal type API (Linux)                                                    */
/*----------------------------------------------------------------------------*/
#define VK_INIT_DELAYED_WORK(p_dwork, p_func) __vk_init_delayed_work(p_dwork, p_func, 0)

struct vk_workqueue_struct* vk_create_workqueue(const char *name);

bool vk_queue_delayed_work(struct vk_workqueue_struct *vos_wq, struct vk_delayed_work *vos_dwork, unsigned long delay_jiffies);
bool vk_queue_delayed_work_on(int cpu, struct vk_workqueue_struct *vos_wq, struct vk_delayed_work *vos_dwork, unsigned long delay_jiffies);

bool vk_cancel_delayed_work(struct vk_delayed_work *vos_dwork);
bool vk_cancel_delayed_work_sync(struct vk_delayed_work *vos_dwork);

void vk_destroy_workqueue(struct vk_workqueue_struct *vos_wq);

#else
#error Not supported OS
#endif

#ifdef __cplusplus
}
#endif

#endif /* _VOS_WORKQUEUE_H_ */

