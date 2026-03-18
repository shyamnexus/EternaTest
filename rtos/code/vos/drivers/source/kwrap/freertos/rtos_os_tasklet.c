/*-----------------------------------------------------------------------------*/
/* Include Header Files                                                        */
/*-----------------------------------------------------------------------------*/
#include <string.h>

#define __MODULE__    rtos_tasklet
#define __DBGLVL__    8 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#define __DBGFLT__    "*"
#include <kwrap/debug.h>
#include <kwrap/flag.h>
#include <kwrap/task.h>
#include <kwrap/tasklet.h>
#include <kwrap/util.h>

#include <FreeRTOS.h>
#include <task.h> //FreeRTOS header file

#define VOS_TASKLET_NAME            "vos_tasklet"
#define VOS_TASKLET_FLAG_NAME       "vos_tasklet_flag"

#define VOS_TASKLET_BIT_SCHED       FLGPTN_BIT(0)
#define VOS_TASKLET_BIT_DESTROY     FLGPTN_BIT(1)
#define VOS_TASKLET_BIT_TASKEND     FLGPTN_BIT(2)

#define VOS_TASKLET_TSKHDL(p_hdl)   ((TaskHandle_t)(p_hdl)->next)
#define VOS_TASKLET_FLGID(p_hdl)    ((ID)(p_hdl)->state)

#define VOS_TASKLET_CALLER()        ((ULONG)__builtin_return_address(0))
#define VOS_TASKLET_TIMEOUT         vos_util_msec_to_tick(10000)
/*-----------------------------------------------------------------------------*/
/* Extern Function Prototype                                                   */
/*-----------------------------------------------------------------------------*/
#if ( configUSE_NVT_PATH > 0 )
extern uint32_t ulPortIsInISR( void ); //rtos-code\driver\na51xxx\source\portable\port.c
#endif

/*-----------------------------------------------------------------------------*/
/* Local Types Declarations                                                    */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/
unsigned int rtos_tasklet_debug_level = NVT_DBG_WRN;
static int g_def_stksize = VOS_DEF_STACK_SIZE;
static int g_def_priority = (VK_TASK_HIGHEST_PRIORITY + 3);

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
void rtos_tasklet_init(void *param)
{
	return;
}

void rtos_tasklet_exit(void)
{
}

static THREAD_RETTYPE vos_tasklet_handle_func(void *param)
{
	struct vos_tasklet_struct *p_hdl;
	FLGPTN retptn = 0;
	ID flgid;

	p_hdl = (struct vos_tasklet_struct *)param;

	flgid = VOS_TASKLET_FLGID(p_hdl);

	while (1) {
		//Only wait and clear sched bit here, because we do not want to clear destroy bit automatically
		if (0 != vos_flag_wait(&retptn, flgid, VOS_TASKLET_BIT_SCHED, TWF_ORW|TWF_CLR)) {
			DBG_ERR("flag wait failed, flgid %d\r\n", flgid);
			break;
		}

		//Although the destroy bit is not waited, we still can get destroy bit after vos_tasklet_kill
		//Check the destroy bit before sched bit, because it may be trigger by vos_tasklet_kill
		if (retptn & VOS_TASKLET_BIT_DESTROY) {
			break;
		}

		if (retptn & VOS_TASKLET_BIT_SCHED) {
			if (NULL == p_hdl->func) {
				DBG_ERR("tasklet func is NULL\r\n");
				break; //unexpected path
			}
			p_hdl->func(p_hdl->data);
		}
	}
	vos_flag_set(flgid, VOS_TASKLET_BIT_TASKEND);

	THREAD_RETURN(0);
}

static int vos_tasklet_isvalid_hdl(struct vos_tasklet_struct *p_hdl)
{
	if (NULL == p_hdl) {
		return 0;
	}

	if (NULL == VOS_TASKLET_TSKHDL(p_hdl)) {
		return 0;
	}

	if (0 == VOS_TASKLET_FLGID(p_hdl)) {
		return 0;
	}

	return 1;
}

void vos_tasklet_init(struct vos_tasklet_struct *p_hdl, void (*func)(unsigned long), unsigned long data)
{
	TaskHandle_t tsk_hdl;
	ID flgid = 0;

	if (NULL == p_hdl) {
		DBG_ERR("p_hdl is NULL\r\n");
		return;
	}

	if (NULL == func) {
		DBG_ERR("func is NULL\r\n");
		return;
	}

	if (E_OK != vos_flag_create(&flgid, NULL, VOS_TASKLET_FLAG_NAME)) {
		DBG_ERR("create flag failed\r\n");
		return;
	}

	tsk_hdl = vos_task_create(vos_tasklet_handle_func, p_hdl, VOS_TASKLET_NAME, g_def_priority, g_def_stksize);
	if (NULL == tsk_hdl) {
		DBG_ERR("create task failed, caller 0x%lx\r\n", VOS_TASKLET_CALLER());
		p_hdl->next = NULL;

		if (E_OK != vos_flag_destroy(flgid)) {
			DBG_ERR("destroy flag failed\r\n");
			return;
		}
	}

	p_hdl->next = (void *)tsk_hdl; //use next as the inner task handle
	p_hdl->state = (unsigned long)flgid; //use state as the flag handle
	//p_hdl->count //no use
	p_hdl->func = func;
	p_hdl->data = data;

	if (0 != vos_task_resume(tsk_hdl)) {
		DBG_ERR("resume failed, caller 0x%lx\r\n", VOS_TASKLET_CALLER());
	}
}

static void _vos_tasklet_sched_common(struct vos_tasklet_struct *p_hdl, ULONG caller)
{
	ID flgid;

	if (!vos_tasklet_isvalid_hdl(p_hdl)) {
		DBG_ERR("tasklet 0x%lx not inited, caller 0x%lx\r\n", p_hdl, caller);
		return;
	}

	flgid = VOS_TASKLET_FLGID(p_hdl);

	if (vos_flag_chk(flgid, VOS_TASKLET_BIT_SCHED | VOS_TASKLET_BIT_DESTROY)) {
		return; //skip if already scheduled or vos_tasklet_kill is called
	}

	if (ulPortIsInISR()) {
		if (E_OK != vos_flag_iset(flgid, VOS_TASKLET_BIT_SCHED)) {
			DBG_ERR("flag iset failed, flgid %d, p_hdl 0x%lx\r\n", flgid, p_hdl);
		}
	} else {
		if (E_OK != vos_flag_set(flgid, VOS_TASKLET_BIT_SCHED)) {
			DBG_ERR("flag set failed, flgid %d, p_hdl 0x%lx\r\n", flgid, p_hdl);
		}
	}
}

void vos_tasklet_hi_schedule(struct vos_tasklet_struct *p_hdl)
{
	_vos_tasklet_sched_common(p_hdl, VOS_TASKLET_CALLER());
}

void vos_tasklet_schedule(struct vos_tasklet_struct *p_hdl)
{
	_vos_tasklet_sched_common(p_hdl, VOS_TASKLET_CALLER());
}

void vos_tasklet_kill(struct vos_tasklet_struct *p_hdl)
{
	ID flgid = 0;
	FLGPTN retptn = 0;
	ER ret_flag;

	if (!vos_tasklet_isvalid_hdl(p_hdl)) {
		DBG_ERR("tasklet 0x%lx not inited, caller 0x%lx\r\n", p_hdl, VOS_TASKLET_CALLER());
		return;
	}

	flgid = VOS_TASKLET_FLGID(p_hdl);

	//notify tasklet to exit
	//1. set sched and destroy bits together, sched bit is used to release the waiting flag
	//2. the destroy should be checked first, because the sched bit is not real
	if (E_OK != vos_flag_set(flgid, VOS_TASKLET_BIT_SCHED | VOS_TASKLET_BIT_DESTROY)) {
		DBG_ERR("flag set failed, flgid %d, p_hdl 0x%lx\r\n", flgid, p_hdl);
		return;
	}

	//wait tasklet function end
	ret_flag = vos_flag_wait_timeout(&retptn, flgid, VOS_TASKLET_BIT_TASKEND, TWF_ORW, VOS_TASKLET_TIMEOUT);
	if (E_OK != ret_flag) {
		DBG_ERR("flag wait err %d, flgid %d, p_hdl 0x%lx\r\n", ret_flag, flgid, p_hdl);
		if (E_TMOUT == ret_flag) {
			DBG_WRN("tasklet should not call THREAD_RETURN\r\n");
		}
	}

	//clear tasklet handle
	memset(p_hdl, 0x0, sizeof(struct vos_tasklet_struct));

	//release flag
	if (E_OK != vos_flag_destroy(flgid)) {
		DBG_ERR("flag %d destroy failed\r\n", flgid);
	}
}


