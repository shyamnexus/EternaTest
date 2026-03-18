#ifndef __MODULE_MAIN_H__
#define __MODULE_MAIN_H__

#ifdef __KERNEL__
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#endif

#include "jpeg_drv.h"
#include "jpeg_kdrv.h"

/*
* 0.1.0: 560 initial driver version
* 0.1.1: fix bug: add offset of address when rotation, va to pa address error
* 0.1.2: fix bug: disable transfer va to pa osg address when osg mask enable
* 0.1.3: fix bug: always set padding info
* 0.1.4: add api of new VBR (adjust priority of frame rate quality)
*/

#define JPEG_VER_STR	"0.1.4"

#define MODULE_MINOR_ID      0
#define MODULE_MINOR_COUNT   1 //_BSP_NS02302_
#define MODULE_NAME          "nvt_jpg"

#ifdef __KERNEL__
typedef struct jpg_drv_info {
	JPG_MODULE_INFO module_info;

	struct class *pmodule_class;
	struct device *pdevice[MODULE_MINOR_COUNT];
	struct resource *presource[MODULE_REG_NUM];
	struct cdev cdev;
	dev_t dev_id;

	// proc entries
	struct proc_dir_entry *pproc_module_root;
	struct proc_dir_entry *pproc_help_entry;
	struct proc_dir_entry *pproc_cmd_entry;
} JPG_DRV_INFO, *PJPG_DRV_INFO;
#endif

#endif
