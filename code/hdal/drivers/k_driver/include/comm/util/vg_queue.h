/*
 *   @file   vg_semaphore.h
 *
 *   @brief  vg queue header file.
 *
 *   A rewrite version of semaphore for vg.
 *
 *   Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
 */

#ifndef __VG_QUEUE_H__
#define __VG_QUEUE_H__

void vg_queue_init(void);
void vg_queue_exit(void);
void *vg_create_queue(void);
int vg_put_to_queue(void *que_hdl, int data_type, uintptr_t data);
int vg_skip_to_queue(void *que_hdl, int data_type);
int vg_get_from_queue(void *que_hdl, int *data_type, uintptr_t *p_data, int wait_ms);
int vg_add_count_to_queue(void *que_hdl);
int vg_get_count_of_queue(void *que_hdl, int *p_todo_counts, int *p_done_counts);
int vg_destroy_queue(void *que_hdl);
int is_vg_queue_empty(void *que_hdl);
void vg_clear_queue(void *que_hdl);


#endif /* __VG_QUEUE_H__ */

