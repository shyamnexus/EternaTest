#ifndef __CSI_TX_MAIN_H__
#define __CSI_TX_MAIN_H__
#include <linux/cdev.h>
#include <linux/types.h>
#include "csi_tx_drv.h"


#define MODULE_MINOR_ID      0
#define MODULE_MINOR_COUNT   1
#define MODULE_NAME          "nvt_csi_tx"

typedef struct xxx_drv_info {
	MODULE_INFO module_info;

	struct class *pmodule_class;
	struct device *pdevice[MODULE_MINOR_COUNT];
	struct resource *presource[MODULE_REG_NUM];
	struct cdev cdev;
	dev_t dev_id;

	// proc entries
	struct proc_dir_entry *pproc_module_root;
	struct proc_dir_entry *pproc_help_entry;
	struct proc_dir_entry *pproc_cmd_entry;
} CSI_TX_DRV_INFO, *PCSI_TX_DRV_INFO;


#endif
