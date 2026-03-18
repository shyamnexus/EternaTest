#ifndef __KDRV_VIE_MAIN_H__
#define __KDRV_VIE_MAIN_H__
#include <linux/cdev.h>
#include <linux/types.h>

#include "kdrv_vie_int.h"
#include "vie_dbg.h"

#define MODULE_MINOR_ID      0
#define MODULE_MINOR_COUNT   1
#define MODULE_NAME          "kdrv_vie"

typedef struct module_info {

} KDRV_VIE_MODULE_INFO, *PKDRV_VIE_MODULE_INFO;

typedef struct kdrv_vie_drv_info {
	KDRV_VIE_MODULE_INFO module_info;
	struct class *pmodule_class;
	struct cdev cdev;
	dev_t dev_id;

    // proc entries
    struct proc_dir_entry *pproc_module_root;
    struct proc_dir_entry *pproc_help_entry;
    struct proc_dir_entry *pproc_cmd_entry;
	struct proc_dir_entry *pproc_info_entry;
	struct proc_dir_entry *pproc_dbglevel_entry;
	struct proc_dir_entry *pproc_gating_entry;

	struct seq_file *info_seq_file;
} KDRV_VIE_DRV_INFO, *PKDRV_VIE_DRV_INFO;


#endif
