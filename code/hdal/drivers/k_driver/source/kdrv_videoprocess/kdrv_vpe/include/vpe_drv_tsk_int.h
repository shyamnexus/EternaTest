#ifndef _VPE_DRV_CTL_TSK_H_
#define _VPE_DRV_CTL_TSK_H_

#define TASK_TYPE_TASKLET 1
#define TASK_TYPE_THREAD 2
#if defined(__LINUX)
#define TASK_TYPE TASK_TYPE_TASKLET
#else
#define TASK_TYPE TASK_TYPE_THREAD
#define TASK_PRIORITY 3
#endif

#include <kwrap/nvt_type.h>
#include <kwrap/tasklet.h>
#include <kwrap/task.h>

#define VPE_DRV_TSK_NAME_MAX 20
struct vpe_drv_tsk {

#if (TASK_TYPE == TASK_TYPE_TASKLET)
	struct vos_tasklet_struct hdl;
#else
	ID flg;
	THREAD_HANDLE hdl;
	char name[VPE_DRV_TSK_NAME_MAX];
#endif

	void (*func)(unsigned long);
	unsigned long data;

};

void vpe_drv_tsk_init(struct vpe_drv_tsk *tsk, void (*func)(unsigned long), unsigned long data, char *name);
void vpe_drv_tsk_uninit(struct vpe_drv_tsk *tsk);
void vpe_drv_tsk_trig(struct vpe_drv_tsk *tsk);

#endif  //_VPE_DRV_CTL_TSK_H_
