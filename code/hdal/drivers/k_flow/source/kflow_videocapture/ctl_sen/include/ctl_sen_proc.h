#ifndef __CTL_SEN_PROC_H_
#define __CTL_SEN_PROC_H_
#include "ctl_sen_main.h"
//#include <linux/proc_fs.h>	// for isp build error

int nvt_ctl_sen_proc_init(PCTL_SEN_DRV_INFO pdrv_info);
int nvt_ctl_sen_proc_remove(PCTL_SEN_DRV_INFO pdrv_info);


#endif
