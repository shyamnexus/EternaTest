#ifndef _MD_DRV_CTL_TSK_H_
#define _MD_DRV_CTL_TSK_H_

#define TASK_TYPE_TASKLET 1
#define TASK_TYPE_THREAD 2

#define TASK_TYPE TASK_TYPE_THREAD
#define TASK_PRIORITY 3

#include <kwrap/nvt_type.h>
#include <kwrap/tasklet.h>
#include <kwrap/task.h>

#define MD_DRV_TSK_NAME_MAX 20
struct md_drv_tsk {

#if (TASK_TYPE == TASK_TYPE_TASKLET)
	struct vos_tasklet_struct hdl;
#else
	ID flg;
	THREAD_HANDLE hdl;
	char name[MD_DRV_TSK_NAME_MAX];
#endif

	void (*func)(unsigned long);
	unsigned long data;

};

void md_drv_tsk_init(struct md_drv_tsk *tsk, void (*func)(unsigned long), unsigned long data, char *name);
void md_drv_tsk_uninit(struct md_drv_tsk *tsk);
void md_drv_tsk_trig(struct md_drv_tsk *tsk);

#endif  //_MD_DRV_CTL_TSK_H_
