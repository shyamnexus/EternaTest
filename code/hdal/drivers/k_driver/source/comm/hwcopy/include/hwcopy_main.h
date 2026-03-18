#ifndef __HWCOPY_MAIN_H__
#define __HWCOPY_MAIN_H__
#include <linux/cdev.h>
#include <linux/types.h>
#include "hwcopy_drv.h"


#define MODULE_MINOR_ID      0
#define MODULE_MINOR_COUNT   1
#define MODULE_NAME          "nvt_hwcopy"
#define HWCP_MAX_CHIP  2

typedef struct hwcopy_drv_info {
	MODULE_INFO module_info;

	struct class *pmodule_class;
	struct device *pdevice[MODULE_MINOR_COUNT];
	struct resource *presource[MODULE_REG_NUM];
	struct cdev cdev;
	struct device *dev;
	dev_t dev_id;
	int chip_idx;

	// proc entries
	struct proc_dir_entry *pproc_module_root;
	struct proc_dir_entry *pproc_help_entry;
	struct proc_dir_entry *pproc_cmd_entry;
} HWCOPY_DRV_INFO, *PHWCOPY_DRV_INFO;


#endif
