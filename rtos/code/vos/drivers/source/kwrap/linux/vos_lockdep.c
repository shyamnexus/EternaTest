/*-----------------------------------------------------------------------------*/
/* Include Header Files                                                        */
/*-----------------------------------------------------------------------------*/
#include <linux/lockdep.h>
#include <linux/spinlock.h>

#include <kwrap/debug.h>
#include <kwrap/lockdep.h>
#include <kwrap/spinlock.h>

/*-----------------------------------------------------------------------------*/
/* Local Types Declarations                                                    */
/*-----------------------------------------------------------------------------*/
#ifndef sizeof_field
#define sizeof_field(TYPE, MEMBER) sizeof((((TYPE *)0)->MEMBER))
#endif

//Note:
//For RT Linux, spinlock is directed to an rt mutex and use atomic 64
//The buf should aligned to 64-bit (8 bytes)
#define GET_ALIGN_BUF(p) ALIGN_CEIL_8((unsigned long)(p)->buf)
/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/
#ifdef CONFIG_LOCKDEP
STATIC_ASSERT(VK_MAX_LOCKDEP_SUBCLASSES == MAX_LOCKDEP_SUBCLASSES);
STATIC_ASSERT(VK_NR_LOCKDEP_CACHING_CLASSES == NR_LOCKDEP_CACHING_CLASSES);

STATIC_ASSERT(sizeof(struct vk_lockdep_subclass_key) == sizeof(struct lockdep_subclass_key));
STATIC_ASSERT(sizeof(struct vk_lock_class_key) == sizeof(struct lock_class_key));
STATIC_ASSERT(sizeof(struct vk_lockdep_map) >= sizeof(struct lockdep_map));
#endif

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
void vk_lockdep_assert_held(vk_raw_spinlock_t *p_voslock)
{
	raw_spinlock_t *p_spinlock = (raw_spinlock_t *)GET_ALIGN_BUF(p_voslock);

	lockdep_assert_held(p_spinlock);
}
EXPORT_SYMBOL(vk_lockdep_assert_held);

void vk_lockdep_set_class_and_name(vk_raw_spinlock_t *p_voslock, struct vk_lock_class_key *key, const char *name)
{
	raw_spinlock_t *p_spinlock = (raw_spinlock_t *)GET_ALIGN_BUF(p_voslock);

	(void)(p_spinlock); //to prevent unused-variable if no CONFIG_LOCKDEP
	lockdep_set_class_and_name(p_spinlock, (void *)key, name);
}
EXPORT_SYMBOL(vk_lockdep_set_class_and_name);