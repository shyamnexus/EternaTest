/*-----------------------------------------------------------------------------*/
/* Include Header Files                                                        */
/*-----------------------------------------------------------------------------*/
#include <linux/spinlock.h>
#include <linux/timer.h>

#include <kwrap/debug.h>
#include <kwrap/timer.h>

/*-----------------------------------------------------------------------------*/
/* Local Types Declarations                                                    */
/*-----------------------------------------------------------------------------*/
#define VOS_TIMER_TAG_INITED MAKEFOURCC('V', 'T', 'M', 'R')

#ifndef sizeof_field
#define sizeof_field(TYPE, MEMBER) sizeof((((TYPE *)0)->MEMBER))
#endif

STATIC_ASSERT(sizeof(struct vk_timer_list) >= sizeof(struct timer_list));
STATIC_ASSERT(sizeof_field(struct vk_timer_list, entry) == sizeof_field(struct timer_list, entry));
STATIC_ASSERT(sizeof_field(struct vk_timer_list, expires) == sizeof_field(struct timer_list, expires));
STATIC_ASSERT(sizeof_field(struct vk_timer_list, function) == sizeof_field(struct timer_list, function));
STATIC_ASSERT(sizeof_field(struct vk_timer_list, flags) == sizeof_field(struct timer_list, flags));

//Note:
//For RT Linux, spinlock is directed to an rt mutex and use atomic 64
//The buf should aligned to 64-bit (8 bytes)
#define GET_ALIGN_BUF(p) ALIGN_CEIL_8((unsigned long)(p)->buf)
/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
void vk_init_timer_key(struct vk_timer_list *vos_timer,
		    void (*vos_func)(struct vk_timer_list *), unsigned int flags,
		    const char *name, struct vk_lock_class_key *vos_key)
{
	struct timer_list *native_timer = (struct timer_list *)vos_timer;

#ifdef CONFIG_LOCKDEP
	struct lock_class_key *native_key = (struct lock_class_key *)vos_key;
	init_timer_key(native_timer, (void *)vos_func, flags, name, native_key);
#else
	init_timer_key(native_timer, (void *)vos_func, flags, NULL, NULL);
#endif
}
EXPORT_SYMBOL(vk_init_timer_key);

void vk_add_timer(struct vk_timer_list *vos_timer)
{
	struct timer_list *native_timer = (struct timer_list *)vos_timer;

	add_timer(native_timer);
}
EXPORT_SYMBOL(vk_add_timer);

int vk_del_timer(struct vk_timer_list * vos_timer)
{
	struct timer_list *native_timer = (struct timer_list *)vos_timer;

	return del_timer(native_timer);
}
EXPORT_SYMBOL(vk_del_timer);

int vk_del_timer_sync(struct vk_timer_list *vos_timer)
{
	struct timer_list *native_timer = (struct timer_list *)vos_timer;

	return del_timer_sync(native_timer);
}
EXPORT_SYMBOL(vk_del_timer_sync);


int vk_mod_timer(struct vk_timer_list *vos_timer, unsigned long expires)
{
	struct timer_list *native_timer = (struct timer_list *)vos_timer;

	return mod_timer(native_timer, expires);
}
EXPORT_SYMBOL(vk_mod_timer);

int vk_mod_timer_pending(struct vk_timer_list *vos_timer, unsigned long expires)
{
	struct timer_list *native_timer = (struct timer_list *)vos_timer;

	return mod_timer_pending(native_timer, expires);
}
EXPORT_SYMBOL(vk_mod_timer_pending);

int vk_timer_pending(const struct vk_timer_list * vos_timer)
{
	struct timer_list *native_timer = (struct timer_list *)vos_timer;

	return timer_pending(native_timer);
}
EXPORT_SYMBOL(vk_timer_pending);
