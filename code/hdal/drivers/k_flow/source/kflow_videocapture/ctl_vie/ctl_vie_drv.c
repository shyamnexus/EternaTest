#if defined __LINUX
#include <linux/wait.h>
#include <linux/param.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/clk.h>
#endif
#include "ctl_vie.h"
#include "ctl_vie_drv.h"
#include "ctl_vie_dbg.h"
#include "ctl_vie_event_id_int.h"
#include "ctl_vie_id_int.h"
#include "ctl_vie_buf_int.h"
#include "ctl_vie_isp_int.h"
#include "ctl_vie_isp_task_int.h"
#include "kdf_vie_int.h"
/*===========================================================================*/
/* Function declaration                                                      */
/*===========================================================================*/

/*===========================================================================*/
/* Define                                                                    */
/*===========================================================================*/
typedef irqreturn_t (*irq_handler_t)(int, void *);

/*===========================================================================*/
/* Global variable                                                           */
/*===========================================================================*/

/*===========================================================================*/
/* Function define                                                           */
/*===========================================================================*/
int nvt_ctl_vie_drv_release(PMODULE_INFO pmodule_info, unsigned char uc_if)
{
	kflow_ctl_vie_uninit();
	return 0;
}

int nvt_ctl_vie_drv_init(MODULE_INFO *pmodule_info)
{
	kflow_ctl_vie_init();
	return 0;
}

int nvt_ctl_vie_drv_remove(MODULE_INFO *pmodule_info)
{
	return 0;
}

int nvt_ctl_vie_drv_suspend(MODULE_INFO *pmodule_info)
{
	ctl_vie_dbg_ind("\n");

	/* Add suspend operation here*/

	return 0;
}

int nvt_ctl_vie_drv_resume(MODULE_INFO *pmodule_info)
{
	ctl_vie_dbg_ind("\n");
	/* Add resume operation here*/

	return 0;
}
