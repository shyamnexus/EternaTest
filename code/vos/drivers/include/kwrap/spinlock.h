#ifndef _VOS_SPINLOCK_H_
#define _VOS_SPINLOCK_H_
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
typedef struct {
	UINT32 init_tag;
	UINT32 is_lock;
} vk_raw_spinlock_t;

typedef vk_raw_spinlock_t vk_spinlock_t;

/*----------------------------------------------------------------------------*/
/* Internal use only, do NOT use directly (FreeRTOS)                          */
/*----------------------------------------------------------------------------*/
#define RTOS_SPINLOCK_INITED_TAG MAKEFOURCC('R', 'S', 'P', 'N')
#define __VK_SPIN_LOCK_INITIALIZER(lock) { .init_tag = RTOS_SPINLOCK_INITED_TAG, .is_lock = 0 }
#define __VK_SPIN_LOCK_UNLOCKED(lock) __VK_SPIN_LOCK_INITIALIZER(lock)

#define __VK_RAW_SPIN_LOCK_INITIALIZER(lock) { .init_tag = RTOS_SPINLOCK_INITED_TAG, .is_lock = 0 }
#define __VK_RAW_SPIN_LOCK_UNLOCKED(lock) __VK_RAW_SPIN_LOCK_INITIALIZER(lock)

unsigned long _vk_spin_lock_irqsave(vk_spinlock_t *lock);
unsigned long _vk_raw_spin_lock_irqsave(vk_spinlock_t *lock);

/*----------------------------------------------------------------------------*/
/* Normal type spinlock (FreeRTOS)                                            */
/*----------------------------------------------------------------------------*/
#define VK_DEFINE_SPINLOCK(x) vk_spinlock_t x = __VK_SPIN_LOCK_UNLOCKED(x)

void vk_spin_lock_init(vk_spinlock_t *lock);
void vk_spin_unlock_irqrestore(vk_spinlock_t *lock, unsigned long flags);
#define vk_spin_lock_irqsave(lock, flags) do { \
	flags = _vk_spin_lock_irqsave(lock); } while (0)

/*----------------------------------------------------------------------------*/
/* Raw type spinlock (FreeRTOS)                                              */
/*----------------------------------------------------------------------------*/
#define VK_DEFINE_RAW_SPINLOCK(x) vk_raw_spinlock_t x = __VK_RAW_SPIN_LOCK_UNLOCKED(x)

void vk_raw_spin_lock_init(vk_spinlock_t *lock);
void vk_raw_spin_unlock_irqrestore(vk_spinlock_t *lock, unsigned long flags);
#define vk_raw_spin_lock_irqsave(lock, flags) do { \
	flags = _vk_raw_spin_lock_irqsave(lock); } while (0)

#elif defined(__LINUX) && defined(__KERNEL__)
#if VOS_RT_LINUX
#define VK_SPIN_BUF_SZ 62 //VK_SPIN_BUF_SZ * 4(bytes) > min(192 bytes)
#else
#if defined (CONFIG_LOCK_STAT)
#define VK_SPIN_BUF_SZ 62
#else
#define VK_SPIN_BUF_SZ 22 //VK_SPIN_BUF_SZ * 4(bytes) > min(56 bytes)
#endif
#endif

/*----------------------------------------------------------------------------*/
/* Type declaration (Linux)                                                   */
/*----------------------------------------------------------------------------*/
typedef struct vk_spinlock {
	unsigned long long init_tag;
	unsigned int buf[VK_SPIN_BUF_SZ];
} vk_raw_spinlock_t;

typedef vk_raw_spinlock_t vk_spinlock_t;

/*----------------------------------------------------------------------------*/
/* Internal use only, do NOT use directly (Linux)                             */
/*----------------------------------------------------------------------------*/
#define __VK_SPIN_LOCK_INITIALIZER(lock) { .init_tag = 0, .buf = {0} }
#define __VK_SPIN_LOCK_UNLOCKED(lock) __VK_SPIN_LOCK_INITIALIZER(lock)

#define __VK_RAW_SPIN_LOCK_INITIALIZER(lock) { .init_tag = 0, .buf = {0} }
#define __VK_RAW_SPIN_LOCK_UNLOCKED(lock) __VK_RAW_SPIN_LOCK_INITIALIZER(lock)

unsigned long _vk_spin_lock_irqsave(vk_spinlock_t *p_voslock);
unsigned long _vk_spin_lock_irqsave_nested(vk_spinlock_t *p_voslock, int subclass);
unsigned long _vk_raw_spin_lock_irqsave(vk_raw_spinlock_t *p_voslock);
unsigned long _vk_raw_spin_lock_irqsave_nested(vk_raw_spinlock_t *p_voslock, int subclass);

/*----------------------------------------------------------------------------*/
/* Normal type spinlock (Linux)                                               */
/*----------------------------------------------------------------------------*/
#define VK_DEFINE_SPINLOCK(x) vk_spinlock_t x = __VK_SPIN_LOCK_UNLOCKED(x)

void vk_spin_lock_init(vk_spinlock_t *p_voslock);
void vk_spin_unlock_irqrestore(vk_spinlock_t *p_voslock, unsigned long flags);
#define vk_spin_lock_irqsave(p_voslock, flags) do { \
	flags = _vk_spin_lock_irqsave(p_voslock); } while (0)
#define vk_spin_lock_irqsave_nested(p_voslock, flags, subclass) do { \
	flags = _vk_spin_lock_irqsave_nested(p_voslock, subclass); } while (0)

void vk_spin_lock(vk_spinlock_t *p_voslock);
void vk_spin_lock_bh(vk_spinlock_t *p_voslock);
void vk_spin_lock_irq(vk_spinlock_t *p_voslock);
void vk_spin_unlock(vk_spinlock_t *p_voslock);
void vk_spin_unlock_bh(vk_spinlock_t *p_voslock);
void vk_spin_unlock_irq(vk_spinlock_t *p_voslock);

void vk_spin_dump_list(void); //for debug, default disabled

/*----------------------------------------------------------------------------*/
/* Raw type spinlock (Linux)                                                  */
/*----------------------------------------------------------------------------*/
#define VK_RAW_DEFINE_SPINLOCK(x) vk_raw_spinlock_t x = __VK_RAW_SPIN_LOCK_UNLOCKED(x)

void vk_raw_spin_lock_init(vk_raw_spinlock_t *p_voslock);
void vk_raw_spin_unlock_irqrestore(vk_raw_spinlock_t *p_voslock, unsigned long flags);
#define vk_raw_spin_lock_irqsave(p_voslock, flags) do { \
	flags = _vk_raw_spin_lock_irqsave(p_voslock); } while (0)
#define vk_raw_spin_lock_irqsave_nested(p_voslock, flags, subclass) do { \
	flags = _vk_raw_spin_lock_irqsave_nested(p_voslock, subclass); } while (0)

void vk_raw_spin_lock(vk_raw_spinlock_t *p_voslock);
void vk_raw_spin_lock_bh(vk_raw_spinlock_t *p_voslock);
void vk_raw_spin_lock_irq(vk_raw_spinlock_t *p_voslock);
void vk_raw_spin_unlock(vk_raw_spinlock_t *p_voslock);
void vk_raw_spin_unlock_bh(vk_raw_spinlock_t *p_voslock);
void vk_raw_spin_unlock_irq(vk_raw_spinlock_t *p_voslock);

#else
#error Not supported OS
#endif


#ifdef __cplusplus
}
#endif

#endif /* _VOS_SPINLOCK_H_ */

