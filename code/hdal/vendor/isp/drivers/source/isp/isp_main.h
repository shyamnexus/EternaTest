#ifndef _ISP_MAIN_H_
#define _ISP_MAIN_H_

#ifdef __KERNEL__
#include <linux/miscdevice.h>
#include <kwrap/dev.h>
#endif

#include "isp_dev_int.h"

//=============================================================================
// ISP device name
//=============================================================================
#define ISP_DEV_NAME "nvt_isp"
#define ISP_MODULE_MINOR_COUNT 1

//=============================================================================
// struct & definition
//=============================================================================
#ifdef __KERNEL__
typedef struct _ISP_DRV_INFO {
	ISP_DEV_INFO dev_info;

	// proc entries
	struct proc_dir_entry *proc_root;
	struct proc_dir_entry *proc_info;
	struct proc_dir_entry *proc_command;
	struct proc_dir_entry *proc_dbglevel;
	struct proc_dir_entry *proc_help;
} ISP_DRV_INFO;

typedef struct isp_vos_drv {
	struct class *pmodule_class;
	struct device *pdevice[ISP_MODULE_MINOR_COUNT];
	struct cdev cdev;
	dev_t dev_id;
} ISP_VOS_DRV, *PISP_VOS_DRV;
#endif

//=============================================================================
// extern functions
//=============================================================================
extern ISP_DEV_INFO *isp_get_dev_info(void);

#endif
