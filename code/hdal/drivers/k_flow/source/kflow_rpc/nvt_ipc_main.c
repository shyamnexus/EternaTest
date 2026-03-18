/**

    This file handles the main flow nvt_ipc driver operations,...etc

    @file       nvt_ipc_main.c
    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/slab.h>
//#include <linux/bootmem.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/syscalls.h>
#include <linux/signal.h>
#include <linux/kthread.h>
#include <linux/hrtimer.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/suspend.h>
#ifdef CONFIG_OF
/* For open firmware. */
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#endif
#include <asm/page.h>
#include <asm/io.h>
#include <asm/cacheflush.h>
#include <kwrap/type.h>
#include <kwrap/flag.h>
#include <kflow_rpc/nvt_ipc.h>
#include <kflow_rpc/nvt_ipc_ioctl.h>
#include "nvt_ipc_common.h"
#include "nvt_ipc_debug.h"
#include "nvt_ipc_main.h"
#include "nvt_ipc_msg_que.h"
#include "nvt_ipc_proc.h"


#ifdef DEBUG
unsigned int nvt_ipc_debug_level = THIS_DBGLVL;
module_param_named(nvt_ipc_debug_level, nvt_ipc_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(nvt_ipc_debug_level, "nvt_ipc debug level");
#endif


#define DRV_VERSION               "1.00.000"
#define COPY_FROM_USER_ERR_STR    "copy_from_user"
#define COPY_TO_USER_ERR_STR      "copy_to_user"


DEFINE_MUTEX(nvt_ipc_mutex);

static NVT_IPC_DRV_INFO  *g_p_drv_info;

extern long nvt_ipc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);


static long nvt_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret;

	ret = nvt_ipc_ioctl(file, cmd, arg);

	return ret;
}


static int nvt_ipc_open(struct inode *inode, struct file *file)
{
	struct nvt_ipc_device *p_hdl_dev;

	p_hdl_dev = kzalloc(sizeof(struct nvt_ipc_device), GFP_KERNEL);
	if (!p_hdl_dev) {
		return -ENOMEM;
	}
	file->private_data = p_hdl_dev;
	return 0;
}

static int nvt_ipc_release(struct inode *inode, struct file *file)
{
	struct nvt_ipc_device *p_hdl_dev = file->private_data;
	char  *token;
	int    i;
	NVTIPC_MSGQUE  *p_msgque;

	mutex_lock(&nvt_ipc_mutex);
	for (i = 0; i < NVTIPC_MSG_QUEUE_NUM ; i++) {
		if (p_hdl_dev->req_queue_bits & (1 << i)) {
			p_msgque = nvt_ipc_msg_que_id2que(i);
			if (p_msgque == NULL) {
				continue;
			}
			token = nvt_ipc_msg_que_key2token(p_msgque->shared_key);
			if (token) {
				DBG_DUMP("Rel Core2 NvtIpcMsgQue %d, token=%s\r\n", i, token);
			} else {
				DBG_DUMP("Rel Core2 NvtIpcMsgQue %d, \r\n", i);
			}
			nvt_ipc_msg_que_rel(i);
		}
	}
	#if __IPC_SIG_SUPPORT__
	nvt_ipc_sig_rm_pid_from_wait_list(current->pid);
	#endif
	kfree(p_hdl_dev);
	file->private_data = NULL;
	mutex_unlock(&nvt_ipc_mutex);
	return 0;
}



static const struct file_operations nvt_ipc_fops = {
	.owner              = THIS_MODULE,
	.unlocked_ioctl     = nvt_unlocked_ioctl,
	.open               = nvt_ipc_open,
	.release            = nvt_ipc_release,
};

/* Match table for device tree binding */
static const struct of_device_id nvt_ipc_of_match[] = {
	{ .compatible = "nvt,nvt_ipc",},
	{},
};
MODULE_DEVICE_TABLE(of, nvt_ipc_of_match);

static int nvt_ipc_probe(struct platform_device *pdev)
{
	NVT_IPC_DRV_INFO *p_drv_info;
	const struct of_device_id *match;
	int ret = 0;

	//printk("%s:%s\r\n", __func__, pdev->name);
	match = of_match_device(nvt_ipc_of_match, &pdev->dev);
	if (!match) {
		DBG_ERR("[%s] OF not found \r\n", MODULE_NAME);
		return -EINVAL;
	}
	p_drv_info = kzalloc(sizeof(NVT_IPC_DRV_INFO), GFP_KERNEL);
	if (!p_drv_info) {
		DBG_ERR("[%s]failed to allocate memory\r\n", MODULE_NAME);
		return -ENOMEM;
	}
	g_p_drv_info = p_drv_info;
	//Dynamic to allocate Device ID
	if (alloc_chrdev_region(&p_drv_info->dev_id, MODULE_MINOR_ID, MODULE_MINOR_COUNT, MODULE_NAME)) {
		DBG_ERR("Can't get device ID\n");
		ret = -ENODEV;
		goto FAIL_FREE_REMAP;
	}

	DBG_IND("DevID Major:%d minor:%d\n" \
			, MAJOR(p_drv_info->dev_id), MINOR(p_drv_info->dev_id));
	/* Register character device for the volume */
	cdev_init(&p_drv_info->cdev, &nvt_ipc_fops);
	p_drv_info->cdev.owner = THIS_MODULE;

	if (cdev_add(&p_drv_info->cdev, p_drv_info->dev_id, 1)) {
		DBG_ERR("[%s]can't add cdev\r\n", MODULE_NAME);
		ret = -ENODEV;
		goto FAIL_CDEV;
	}
	// create class
	p_drv_info->pmodule_class = class_create(THIS_MODULE, MODULE_NAME);
	if (IS_ERR(p_drv_info->pmodule_class)) {
		DBG_ERR("[%s]failed in creating class.\r\n", MODULE_NAME);
		ret = -ENODEV;
		goto FAIL_CDEV;
	}
	// register
	p_drv_info->pdevice = device_create(p_drv_info->pmodule_class, NULL, MKDEV(MAJOR(p_drv_info->dev_id), 0), NULL, MODULE_NAME);
	if (IS_ERR(p_drv_info->pdevice)) {
		DBG_ERR("[%s]creating device fail.\r\n", MODULE_NAME);
		ret = -ENODEV;
		goto FAIL_CLASS;
	}
	platform_set_drvdata(pdev, p_drv_info);
	if (nvt_ipc_drv_init(&p_drv_info->module_info) < 0) {
		DBG_ERR("init drv fail.\r\n");
		ret = -1;
		goto FAIL_DEV;
	}
	nvt_ipc_proc_init(p_drv_info);
	return ret;
FAIL_DEV:
	device_unregister(p_drv_info->pdevice);
FAIL_CLASS:
	class_destroy(p_drv_info->pmodule_class);
FAIL_CDEV:
	cdev_del(&p_drv_info->cdev);
	unregister_chrdev_region(p_drv_info->dev_id, 1);
FAIL_FREE_REMAP:
	kfree(p_drv_info);
	p_drv_info = NULL;
	platform_set_drvdata(pdev, p_drv_info);
	DBG_ERR("probe fail\r\n");
	return ret;
}

static int nvt_ipc_remove(struct platform_device *pdev)
{
	NVT_IPC_DRV_INFO *p_drv_info;

	printk("%s\n", __func__);
	p_drv_info = platform_get_drvdata(pdev);

	nvt_ipc_drv_exit(&p_drv_info->module_info);
	nvt_ipc_proc_exit(p_drv_info);
	device_unregister(p_drv_info->pdevice);
	class_destroy(p_drv_info->pmodule_class);
	cdev_del(&p_drv_info->cdev);
	unregister_chrdev_region(p_drv_info->dev_id, 1);
	kfree(p_drv_info);
	return 0;
}

#if 0

static int nvt_ipc_suspend(struct device *dev)
{
	return nvt_ipc_drv_suspend();
}

static int nvt_ipc_resume(struct device *dev)
{
	return nvt_ipc_drv_resume();
}

static const struct dev_pm_ops nvt_ipc_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(nvt_ipc_suspend, nvt_ipc_resume)
};
#endif



static struct platform_driver nvt_ipc_driver = {
	.probe   =  nvt_ipc_probe,
	.remove  =  nvt_ipc_remove,
	.driver  = {
		.name =  MODULE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = nvt_ipc_of_match,
		#if 0
		.pm = &nvt_ipc_pm_ops,
		#endif
	},
};


int __init nvt_ipc_init(void)
{
	//DBG_DUMP("nvt_ipc_init \r\n");
	return platform_driver_register(&nvt_ipc_driver);
}



void __exit nvt_ipc_exit(void)
{
	platform_driver_unregister(&nvt_ipc_driver);
}


module_init(nvt_ipc_init);
module_exit(nvt_ipc_exit);
MODULE_AUTHOR("Novatek Microelectronics Corp.");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("nvt-ipc driver for NVT SOC");
MODULE_VERSION(DRV_VERSION);
