#ifndef _KDRV_RPC_MAIN_H__
#define _KDRV_RPC_MAIN_H__
#include <linux/cdev.h>
#include <linux/types.h>
#include "kdrv_rpc_drv.h"


#define MODULE_MINOR_ID      0
#define MODULE_MINOR_COUNT   1
#define MODULE_NAME          "kdrv_rpc"

typedef struct _KDRV_RPC_DRV_INFO {
	KDRV_RPC_MODULE_INFO module_info;
	struct class    *pmodule_class;
	struct device   *pdevice;
	struct resource *presource[MODULE_REG_NUM];
	//void __iomem    *io_addr[MODULE_REG_NUM];
	//int              iinterrupt_id[MODULE_IRQ_NUM];
	struct           cdev cdev;
	dev_t            dev_id;
	// proc entries
	struct proc_dir_entry *pproc_module_root;
	struct proc_dir_entry *pproc_debug_entry;
} KDRV_RPC_DRV_INFO, *PKDRV_RPC_DRV_INFO;


#endif

