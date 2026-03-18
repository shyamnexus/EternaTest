#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/kdev_t.h>
#include <linux/clk.h>
#include <asm/signal.h>
#include "ctl_vie_drv.h"
#include "ctl_vie_main.h"
#include "ctl_vie_proc.h"
#include "ctl_vie_dbg.h"

//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================
#ifdef DEBUG
unsigned int ctl_vie_debug_level = NVT_DBG_WRN;
module_param_named(ctl_vie_debug_level, ctl_vie_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(ctl_vie_debug_level, "Debug message level");
#endif

//=============================================================================
// Global variable
//=============================================================================
static CTL_VIE_DRV_INFO *pctl_vie_info;

//=============================================================================
// function declaration
//=============================================================================
#if defined(_GROUP_KO_)
#undef __init
#undef __exit
#undef module_init
#undef module_exit
#define __init
#define __exit
#define module_init(x)
#define module_exit(x)
#ifndef MODULE // in built-in case
#undef MODULE_VERSION
#undef VOS_MODULE_VERSION
#define MODULE_VERSION(x)
#define VOS_MODULE_VERSION(a1, a2, a3, a4, a5)
#endif
#endif

int __init nvt_ctl_vie_init(void);
void __exit nvt_ctl_vie_exit(void);

//=============================================================================
// function define
//=============================================================================
int __init nvt_ctl_vie_init(void)
{
	int ret;

	pctl_vie_info = kzalloc(sizeof(CTL_VIE_DRV_INFO), GFP_KERNEL);
	if (!pctl_vie_info) {
		ctl_vie_dbg_err("allocate mem fail\n");
		return -ENOMEM;
	}

	ret = nvt_ctl_vie_proc_init(pctl_vie_info);
	if (ret) {
		ctl_vie_dbg_err("creat proc. fail\n");
		goto FAIL_INIT;
	}

	ret = nvt_ctl_vie_drv_init(&pctl_vie_info->module_info);
	if (ret) {
		ctl_vie_dbg_err("vie drv init fail\n");
		goto FAIL_PROC;
	}

	return ret;

FAIL_PROC:
	nvt_ctl_vie_proc_remove(pctl_vie_info);

FAIL_INIT:
	kfree(pctl_vie_info);
	pctl_vie_info = NULL;
	return 0;
}

void __exit nvt_ctl_vie_exit(void)
{
	nvt_ctl_vie_drv_release(&pctl_vie_info->module_info, 0);

	nvt_ctl_vie_proc_remove(pctl_vie_info);

	kfree(pctl_vie_info);
	pctl_vie_info = NULL;
}

module_init(nvt_ctl_vie_init);
module_exit(nvt_ctl_vie_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("ctl_vie driver");
MODULE_LICENSE("GPL");
