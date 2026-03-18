#ifndef __MODULE_STBC_MAIN_H__
#define __MODULE_STBC_MAIN_H__
#include <linux/cdev.h>
#include <linux/types.h>
#include "stbc_drv.h"


#define MODULE_MINOR_ID      0
#define MODULE_MINOR_COUNT   1
#define MODULE_NAME          "nvt_stbc"

typedef struct stbc_drv_info {
	MODULE_INFO module_info;

	struct class *pmodule_class;
	struct device *pdevice[MODULE_MINOR_COUNT];
	struct resource *presource[MODULE_REG_NUM];
	struct cdev cdev;
	dev_t dev_id;

	// proc entries
	struct proc_dir_entry *pproc_module_root;
	struct proc_dir_entry *pproc_help_entry;
//	struct proc_dir_entry *pproc_avl_entry;
	struct proc_dir_entry *pproc_stbc_entry;

	struct proc_dir_entry *pproc_stbc_root;
//	struct proc_dir_entry *pproc_dram1_entry;
//	struct proc_dir_entry *pproc_dram2_entry;
//	struct proc_dir_entry *pproc_dram1_heavyload_entry;
//	struct proc_dir_entry *pproc_dram2_heavyload_entry;
} STBC_DRV_INFO, *PSTBC_DRV_INFO;


#endif
