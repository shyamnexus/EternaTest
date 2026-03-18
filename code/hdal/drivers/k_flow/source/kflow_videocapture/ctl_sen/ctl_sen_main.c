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
#include "ctl_sen_main.h"
#include "ctl_sen_proc.h"
#include "ctl_sen_dbg.h"
#include "ctl_sen_api.h"

//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================
#ifdef DEBUG
unsigned int ctl_sen_debug_level = NVT_DBG_IND;
module_param_named(ctl_sen_debug_level, ctl_sen_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(ctl_sen_debug_level, "Debug message level");
#endif

//=============================================================================
// Global variable
//=============================================================================
static CTL_SEN_DRV_INFO *pctl_sen_info;

//=============================================================================
// function declaration
//=============================================================================
int __init nvt_ctl_sen_module_init(void);
void __exit nvt_ctl_sen_module_exit(void);

//=============================================================================
// function define
//=============================================================================

static struct platform_driver nvt_ctl_sen_driver = {
	.driver = {
		.name   = MODULE_NAME,
		.owner = THIS_MODULE,
	},
};

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

int __init nvt_ctl_sen_module_init(void)
{
	int ret;

	if (register_printout_notifier(ctl_sen_dbg_panic_printout_handler) < 0) {
		ctl_sen_dbg_err("register_printout_notifier fail\n");
	}

	pctl_sen_info = kzalloc(sizeof(CTL_SEN_DRV_INFO), GFP_KERNEL);
	if (!pctl_sen_info) {
		ctl_sen_dbg_err("failed to allocate memory\n");
		return -ENOMEM;
	}

	ret = nvt_ctl_sen_proc_init(pctl_sen_info);
	if (ret) {
		ctl_sen_dbg_err("failed in creating proc.\n");
		goto FAIL_INIT;
	}

	ret = platform_driver_register(&nvt_ctl_sen_driver);
	if (ret) {
		ctl_sen_dbg_err("failed in platform_driver_register.\n");
		goto FAIL_PROC;
	}

	ret = kflow_ctl_sen_init();
	if (ret) {
		ctl_sen_dbg_err("kflow_ctl_sen_init fail\n");
	}

	return ret;

FAIL_PROC:
	nvt_ctl_sen_proc_remove(pctl_sen_info);

FAIL_INIT:
	kfree(pctl_sen_info);
	pctl_sen_info = NULL;
	return 0;
}

void __exit nvt_ctl_sen_module_exit(void)
{
	kflow_ctl_sen_uninit();

	nvt_ctl_sen_proc_remove(pctl_sen_info);

	kfree(pctl_sen_info);
	pctl_sen_info = NULL;
	platform_driver_unregister(&nvt_ctl_sen_driver);

	unregister_printout_notifier(ctl_sen_dbg_panic_printout_handler);
}

module_init(nvt_ctl_sen_module_init);
module_exit(nvt_ctl_sen_module_exit);

