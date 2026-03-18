#ifndef __AI_MAIN_H__
#define __AI_MAIN_H__
#ifdef __KERNEL__
#include <linux/cdev.h>
#include <linux/types.h>
#include "ai_drv.h"


#define MODULE_MINOR_ID      0
#define MODULE_MINOR_COUNT   1
#define MODULE_NAME          "nvt_ai_module"

typedef struct ai_drv_info {
	AI_INFO module_info;

	struct class *pmodule_class;
	struct device *pdevice[MODULE_MINOR_COUNT];
	struct resource *presource[MODULE_REG_NUM];
	struct cdev cdev;
	dev_t dev_id;

	// proc entries
	struct proc_dir_entry *pproc_module_root;
	struct proc_dir_entry *pproc_help_entry;
	struct proc_dir_entry *pproc_cmd_entry;
	struct proc_dir_entry *pproc_module_cnn;
	struct proc_dir_entry *pproc_module_nue;
	struct proc_dir_entry *pproc_module_nue2;
	struct proc_dir_entry *pproc_module_cnn_gating_proc;
	struct proc_dir_entry *pproc_module_nue_gating_proc;
	struct proc_dir_entry *pproc_module_nue2_gating_proc;
} AI_DRV_INFO, *PAI_DRV_INFO;

unsigned int kdrv_ai_get_gating(int engine_index);
void kdrv_ai_set_gating(int flag,int engine_index);
#endif
#endif
