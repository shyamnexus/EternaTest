#ifndef _VOS_LOCKDEP_H_
#define _VOS_LOCKDEP_H_

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/
/* Including Files                                                            */
/*----------------------------------------------------------------------------*/
#include <kwrap/nvt_type.h>
#include <kwrap/spinlock.h>

#if (defined(__UITRON) || defined(__ECOS))
#elif defined(__FREERTOS)
#elif defined(__LINUX) && defined(__KERNEL__)

#if defined(__LINUX) && defined(__KERNEL__)
#include <linux/version.h>
#endif

/*----------------------------------------------------------------------------*/
/* Type declaration (Linux)                                                   */
/*----------------------------------------------------------------------------*/
#define VK_CONFIG_LOCKDEP
#define VK_CONFIG_LOCK_STAT

#define VK_MAX_LOCKDEP_SUBCLASSES       8UL
#define VK_NR_LOCKDEP_CACHING_CLASSES   2


struct vk_lockdep_subclass_key {
	char __one_byte;
} __attribute__ ((__packed__));

#if defined(__LINUX) && defined(__KERNEL__)
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
struct vk_hlist_head {
	struct vk_hlist_node *first;
};

struct vk_hlist_node {
	struct vk_hlist_node *next, **pprev;
};

struct vk_lock_class_key {
    union {
		struct vk_hlist_node		hash_entry;
		struct vk_lockdep_subclass_key subkeys[VK_MAX_LOCKDEP_SUBCLASSES];
	};
};
#else

struct vk_lock_class_key {
	struct vk_lockdep_subclass_key subkeys[VK_MAX_LOCKDEP_SUBCLASSES];
};
#endif
#else
struct vk_lock_class_key {
	struct vk_lockdep_subclass_key subkeys[VK_MAX_LOCKDEP_SUBCLASSES];
};
#endif

struct vk_lock_class; //a dummy structure, for struct vk_lockdep_map

struct vk_lockdep_map {
	struct vk_lock_class_key *key;
	struct vk_lock_class *class_cache[VK_NR_LOCKDEP_CACHING_CLASSES];
	const char *name;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0) && LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0) )
	short			wait_type_outer; /* can be taken in this context */
	short			wait_type_inner; /* presents this context */
#elif (LINUX_VERSION_CODE > KERNEL_VERSION(6, 6, 0) )
	u8				wait_type_outer; /* can be taken in this context */
	u8				wait_type_inner; /* presents this context */
	u8				lock_type;
#endif
#ifdef VK_CONFIG_LOCK_STAT
	int cpu;
	unsigned long ip;
#endif
};

/*----------------------------------------------------------------------------*/
/* Internal use only, do NOT use directly (Linux)                             */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Normal type API (Linux)                                                    */
/*----------------------------------------------------------------------------*/
void vk_lockdep_assert_held(vk_raw_spinlock_t *p_voslock);
void vk_lockdep_set_class_and_name(vk_raw_spinlock_t *p_voslock, struct vk_lock_class_key *key, const char *name);

#else
#error Not supported OS
#endif

#ifdef __cplusplus
}
#endif

#endif /* _VOS_LOCKDEP_H_ */

