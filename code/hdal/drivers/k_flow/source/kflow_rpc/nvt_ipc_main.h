#ifndef _NVT_IPC_MAIN_H__
#define _NVT_IPC_MAIN_H__
#include <linux/cdev.h>
#include <linux/types.h>
#include "nvt_ipc_drv.h"


#define MODULE_MINOR_ID      0
#define MODULE_MINOR_COUNT   1
#define MODULE_NAME          "nvt_ipc"

typedef struct _NVT_IPC_DRV_INFO {
	NVT_IPC_MODULE_INFO    module_info;
	struct class          *pmodule_class;
	struct device         *pdevice;
	struct cdev            cdev;
	dev_t                  dev_id;
	// proc entries
	struct proc_dir_entry *pproc_module_root;
	struct proc_dir_entry *pproc_info_entry;
} NVT_IPC_DRV_INFO, *PNVT_IPC_DRV_INFO;


#endif