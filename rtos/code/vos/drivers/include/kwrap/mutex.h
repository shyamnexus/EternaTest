#ifndef _VOS_MUTEX_H_
#define _VOS_MUTEX_H_
#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/
/* Including Files                                                            */
/*----------------------------------------------------------------------------*/
#include <kwrap/nvt_type.h>

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
#define VK_MUTEX_BUF_SZ 78
#else
#if defined (CONFIG_LOCK_STAT)
#define VK_MUTEX_BUF_SZ 78
#else
#define VK_MUTEX_BUF_SZ 38
#endif
#endif

/*----------------------------------------------------------------------------*/
/* Type declaration (Linux)                                                   */
/*----------------------------------------------------------------------------*/
struct vk_mutex {
	unsigned long long init_tag;
	unsigned int buf[VK_MUTEX_BUF_SZ];
};

/*----------------------------------------------------------------------------*/
/* Internal use only, do NOT use directly (Linux)                             */
/*----------------------------------------------------------------------------*/
#define __VK_MUTEX_INITIALIZER(lockname) { .init_tag = 0, .buf = {0} }

/*----------------------------------------------------------------------------*/
/* Normal type API (Linux)                                                    */
/*----------------------------------------------------------------------------*/
#define VK_DEFINE_MUTEX(mutexname) \
	struct vk_mutex mutexname = __VK_MUTEX_INITIALIZER(mutexname)

void vk_mutex_init(struct vk_mutex *vos_mutex);

void vk_mutex_lock(struct vk_mutex *vos_mutex);
int vk_mutex_trylock(struct vk_mutex *vos_mutex);

void vk_mutex_unlock(struct vk_mutex *vos_mutex);

bool vk_mutex_is_locked(struct vk_mutex *vos_mutex);

#else
#error Not supported OS
#endif

#ifdef __cplusplus
}
#endif

#endif /* _VOS_MUTEX_H_ */

