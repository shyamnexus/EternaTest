#ifndef _VOS_TIMER_H_
#define _VOS_TIMER_H_
#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/
/* Including Files                                                            */
/*----------------------------------------------------------------------------*/
#include <kwrap/list.h>
#include <kwrap/lockdep.h>
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

/*----------------------------------------------------------------------------*/
/* Type declaration (Linux)                                                   */
/*----------------------------------------------------------------------------*/
struct vk_timer_list {
	struct vos_hlist_node entry;
	unsigned long expires;
	void (*function)(struct vk_timer_list *);
	u32 flags;
#ifdef VK_CONFIG_LOCKDEP
	struct vk_lockdep_map lockdep_map;
#endif
};

/*----------------------------------------------------------------------------*/
/* Internal use only, do NOT use directly (Linux)                             */
/*----------------------------------------------------------------------------*/
#define __vk_init_timer(_timer, _fn, _flags) \
	do { \
		static struct vk_lock_class_key __key; \
		vk_init_timer_key((_timer), (_fn), (_flags), #_timer, &__key); \
	} while (0)

void vk_init_timer_key(struct vk_timer_list *vos_timer,
		    void (*vos_func)(struct vk_timer_list *), unsigned int flags,
		    const char *name, struct vk_lock_class_key *vos_key);

/*----------------------------------------------------------------------------*/
/* Normal type API (Linux)                                                    */
/*----------------------------------------------------------------------------*/
#define vk_timer_setup(timer, callback, flags) \
	__vk_init_timer((timer), (callback), (flags))

void vk_add_timer(struct vk_timer_list *vos_timer);

int vk_del_timer(struct vk_timer_list * vos_timer);
int vk_del_timer_sync(struct vk_timer_list *vos_timer);

int vk_mod_timer(struct vk_timer_list *vos_timer, unsigned long expires);
int vk_mod_timer_pending(struct vk_timer_list *vos_timer, unsigned long expires);

int vk_timer_pending(const struct vk_timer_list *vos_timer);

#else
#error Not supported OS
#endif

#ifdef __cplusplus
}
#endif

#endif /* _VOS_TIMER_H_ */

