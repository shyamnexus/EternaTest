#ifndef _DRE_DRV_CTL_TSK_H_
#define _DRE_DRV_CTL_TSK_H_

#define TASK_TYPE_TASKLET 1
#define TASK_TYPE_THREAD 2

#define TASK_TYPE TASK_TYPE_THREAD
#define TASK_PRIORITY 3

#include <kwrap/nvt_type.h>
#include <kwrap/tasklet.h>
#include <kwrap/task.h>

#define DRE_DRV_TSK_NAME_MAX 20
struct dre_drv_tsk {

#if (TASK_TYPE == TASK_TYPE_TASKLET)
	struct vos_tasklet_struct hdl;
#else
	ID flg;
	THREAD_HANDLE hdl;
	char name[DRE_DRV_TSK_NAME_MAX];
#endif

	void (*func)(unsigned long);
	unsigned long data;

};

void dre_drv_tsk_init(struct dre_drv_tsk *tsk, void (*func)(unsigned long), unsigned long data, char *name);
void dre_drv_tsk_uninit(struct dre_drv_tsk *tsk);
void dre_drv_tsk_trig(struct dre_drv_tsk *tsk);

#endif  //_DRE_DRV_CTL_TSK_H_
