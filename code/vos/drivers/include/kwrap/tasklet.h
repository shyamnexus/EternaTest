#ifndef _VOS_TASKLET_H_
#define _VOS_TASKLET_H_
#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------*/
/* Including Files                                                                                                                 */
/*-----------------------------------------------------------------------------*/
#include <kwrap/nvt_type.h>

#if defined(__LINUX) && defined(__KERNEL__)
#include <linux/version.h>
#endif

typedef struct {
	int counter;
} vos_atomic_t;

struct vos_tasklet_struct
{
	struct vos_tasklet_struct *next; //FreeRTOS: used as a task handle
	unsigned long state; //FreeRTOS: used as a flag handle
	vos_atomic_t count;
#if defined(__LINUX) && defined(__KERNEL__)
	#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
	bool usecallback;
	#endif
#endif
	void (*func)(unsigned long);
	unsigned long data;
};

void vos_tasklet_init(struct vos_tasklet_struct *t, void (*func)(unsigned long), unsigned long data);
void vos_tasklet_hi_schedule(struct vos_tasklet_struct *t);
void vos_tasklet_schedule(struct vos_tasklet_struct *t);
void vos_tasklet_kill(struct vos_tasklet_struct *t);

#ifdef __cplusplus
}
#endif

#endif /* _VOS_TASKLET_H_ */

