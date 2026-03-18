#ifndef __CTL_SEN_MAIN_H__
#define __CTL_SEN_MAIN_H__
#include <linux/cdev.h>
#include <linux/types.h>

#define MODULE_MINOR_ID      0
#define MODULE_MINOR_COUNT   1
#define MODULE_NAME          "nvt_ctl_sen_module"

typedef struct ctl_sen_info {
} CTL_SEN_MODULE_INFO, *PCTL_SEN_MODULE_INFO;

typedef struct ctl_sen_drv_info {
	CTL_SEN_MODULE_INFO module_info;

	struct class *pctl_sen_class;
	struct device *pdevice[MODULE_MINOR_COUNT];
	struct cdev cdev;
	dev_t dev_id;

	// proc entries
	struct proc_dir_entry *pproc_module_root;
	struct proc_dir_entry *pproc_help_entry;
	struct proc_dir_entry *pproc_cmd_entry;

	struct proc_dir_entry *pproc_info_entry;
	struct proc_dir_entry *pproc_signal_entry;
	struct proc_dir_entry *pproc_dbglevel_entry;

} CTL_SEN_DRV_INFO, *PCTL_SEN_DRV_INFO;


#endif
