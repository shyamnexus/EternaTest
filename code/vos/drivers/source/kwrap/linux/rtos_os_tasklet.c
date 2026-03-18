/*-----------------------------------------------------------------------------*/
/* Include Header Files                                                        */
/*-----------------------------------------------------------------------------*/
#include <linux/interrupt.h>
#include <linux/module.h>
#include <kwrap/tasklet.h>

#define __MODULE__    rtos_tasklet
#define __DBGLVL__    8 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#define __DBGFLT__    "*"
#include <kwrap/debug.h>

/*-----------------------------------------------------------------------------*/
/* Local Types Declarations                                                    */
/*-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/
unsigned int rtos_tasklet_debug_level = NVT_DBG_WRN;

module_param_named(rtos_tasklet_debug_level, rtos_tasklet_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(rtos_tasklet_debug_level, "Debug message level");

STATIC_ASSERT(sizeof(struct vos_tasklet_struct) == sizeof(struct tasklet_struct));
STATIC_ASSERT(sizeof(vos_atomic_t) == sizeof(atomic_t));
/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
void vos_tasklet_init(struct vos_tasklet_struct *t, void (*func)(unsigned long), unsigned long data)
{
	tasklet_init((struct tasklet_struct *)t, func, data);
}
EXPORT_SYMBOL(vos_tasklet_init);

void vos_tasklet_hi_schedule(struct vos_tasklet_struct *t)
{
	tasklet_hi_schedule((struct tasklet_struct *)t);
}
EXPORT_SYMBOL(vos_tasklet_hi_schedule);

void vos_tasklet_schedule(struct vos_tasklet_struct *t)
{
	tasklet_schedule((struct tasklet_struct *)t);
}
EXPORT_SYMBOL(vos_tasklet_schedule);

void vos_tasklet_kill(struct vos_tasklet_struct *t)
{
	tasklet_kill((struct tasklet_struct *)t);
}
EXPORT_SYMBOL(vos_tasklet_kill);