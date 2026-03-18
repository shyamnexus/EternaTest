/*-----------------------------------------------------------------------------*/
/* Include Header Files                                                        */
/*-----------------------------------------------------------------------------*/
#include <linux/mutex.h>
#include <linux/spinlock.h>

#include <kwrap/debug.h>
#include <kwrap/mutex.h>

/*-----------------------------------------------------------------------------*/
/* Local Types Declarations                                                    */
/*-----------------------------------------------------------------------------*/
#define VOS_MUTEX_TAG_INITED MAKEFOURCC('V', 'M', 'T', 'X')

#define loc_cpu(flags) raw_spin_lock_irqsave(&my_lock, flags)
#define unl_cpu(flags) raw_spin_unlock_irqrestore(&my_lock, flags)

#ifndef sizeof_field
#define sizeof_field(TYPE, MEMBER) sizeof((((TYPE *)0)->MEMBER))
#endif

//Note: additional sizeof(u64) for the buf offset
STATIC_ASSERT(sizeof_field(struct vk_mutex, buf) >= (sizeof(struct mutex) + sizeof(u64)));

//Note:
//For RT Linux, spinlock is directed to an rt mutex and use atomic 64
//The buf should aligned to 64-bit (8 bytes)
#define GET_ALIGN_BUF(p) ALIGN_CEIL_8((unsigned long)(p)->buf)
/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/
static DEFINE_RAW_SPINLOCK(my_lock);

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
static void _vk_mutex_init_check_tag(struct vk_mutex *vos_mutex)
{
	unsigned long my_flags = 0;

	loc_cpu(my_flags);

	//after internal lock, check the tag again to make sure not inited yet
	if (VOS_MUTEX_TAG_INITED != vos_mutex->init_tag) {
		struct mutex *native_mutex = (struct mutex *)GET_ALIGN_BUF(vos_mutex);

		mutex_init(native_mutex);
		vos_mutex->init_tag = VOS_MUTEX_TAG_INITED;
	}

	unl_cpu(my_flags);
}

void vk_mutex_init(struct vk_mutex *vos_mutex)
{
	struct mutex *native_mutex = (struct mutex *)GET_ALIGN_BUF(vos_mutex);
	unsigned long my_flags = 0;

	loc_cpu(my_flags);

	mutex_init(native_mutex);
	vos_mutex->init_tag = VOS_MUTEX_TAG_INITED;

	unl_cpu(my_flags);
}
EXPORT_SYMBOL(vk_mutex_init);

void vk_mutex_lock(struct vk_mutex *vos_mutex)
{
	struct mutex *native_mutex = (struct mutex *)GET_ALIGN_BUF(vos_mutex);

	if (VOS_MUTEX_TAG_INITED != vos_mutex->init_tag) {
		_vk_mutex_init_check_tag(vos_mutex);
	}

	mutex_lock(native_mutex);
}
EXPORT_SYMBOL(vk_mutex_lock);

int vk_mutex_trylock(struct vk_mutex *vos_mutex)
{
	struct mutex *native_mutex = (struct mutex *)GET_ALIGN_BUF(vos_mutex);

	if (VOS_MUTEX_TAG_INITED != vos_mutex->init_tag) {
		_vk_mutex_init_check_tag(vos_mutex);
	}

	return mutex_trylock(native_mutex);
}
EXPORT_SYMBOL(vk_mutex_trylock);

void vk_mutex_unlock(struct vk_mutex *vos_mutex)
{
	struct mutex *native_mutex = (struct mutex *)GET_ALIGN_BUF(vos_mutex);

	if (VOS_MUTEX_TAG_INITED != vos_mutex->init_tag) {
		DBG_ERR("0x%lX not inited\r\n", (unsigned long)vos_mutex);
		return;
	}

	mutex_unlock(native_mutex);
}
EXPORT_SYMBOL(vk_mutex_unlock);

bool vk_mutex_is_locked(struct vk_mutex *vos_mutex)
{
	struct mutex *native_mutex = (struct mutex *)GET_ALIGN_BUF(vos_mutex);

	if (VOS_MUTEX_TAG_INITED != vos_mutex->init_tag) {
		DBG_ERR("0x%lX not inited\r\n", (unsigned long)vos_mutex);
		return 1;
	}

	return mutex_is_locked(native_mutex);
}
EXPORT_SYMBOL(vk_mutex_is_locked);