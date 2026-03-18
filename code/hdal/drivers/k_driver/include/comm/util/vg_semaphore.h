/*
 *   @file   vg_semaphore.h
 *
 *   @brief  vg semaphore header file.
 *
 *   A rewrite version of semaphore for vg.
 *
 *   Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
 */

#ifndef __VG_SEMAPHORE_H__
#define __VG_SEMAPHORE_H__

#include "kwrap/semaphore.h"
#include "kwrap/spinlock.h"

#define MAX_CONCURRENT_THREAD_COUNT     20
#define MAX_SEM_NAME_LEN                31
typedef struct {
	struct {
		void    *task;
		int     down_count;
	} thread_info[MAX_CONCURRENT_THREAD_COUNT];
	int max_thread_count;
	vk_spinlock_t spin_lock;
	struct vk_semaphore	sem;
	char name[MAX_SEM_NAME_LEN + 1];
} vg_semaphore_t;


/* recursive semaphore in single thread */
int vg_semaphore_init(vg_semaphore_t *p_sem, int max_thread_count, char *name);
void vg_semaphore_exit(vg_semaphore_t *p_sem);
void vg_semaphore_down(vg_semaphore_t *p_sem);
void vg_semaphore_up(vg_semaphore_t *p_sem);

/* normal sempahore */
void vg_sema_init(vg_semaphore_t *p_sem);
void vg_sema_down(vg_semaphore_t *p_sem);
void vg_sema_up(vg_semaphore_t *p_sem);
int vg_down_timeout(vg_semaphore_t *p_sem, unsigned int ms_timeout);
#endif /* __VG_SEMAPHORE_H__ */

