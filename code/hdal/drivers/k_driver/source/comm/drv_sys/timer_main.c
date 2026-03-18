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
#include <kwrap/dev.h>
#include <kwrap/verinfo.h>

#include "timer_platform.h"
#include "timer_drv.h"
#include "timer_reg.h"
#include "timer_main.h"
#include "timer_proc.h"
#include "timer_dbg.h"
#include "ddr_proc.h"
#include "hwclock_int.h"
#include "heavyload_proc.h"
#include <plat/nvt_jiffies.h>
#include "hwclock_proc.h"

//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================
#ifdef DEBUG
unsigned int timer_debug_level = NVT_DBG_ERR;
module_param_named(timer_debug_level, timer_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(timer_debug_level, "Debug message level");
#endif

//=============================================================================
// Global variable
//=============================================================================
static struct of_device_id xxx_match_table[] = {
	{	.compatible = "nvt,nvt_timer"},
	{}
};

//=============================================================================
// function declaration
//=============================================================================
static int nvt_timer_open(struct inode *inode, struct file *file);
static int nvt_timer_release(struct inode *inode, struct file *file);
static long nvt_timer_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static int nvt_timer_probe(struct platform_device *pdev);
static int nvt_timer_suspend(struct platform_device *pdev, pm_message_t state);
static int nvt_timer_resume(struct platform_device *pdev);
static int nvt_timer_remove(struct platform_device *pdev);
int __init nvt_timer_module_init(void);
void __exit nvt_timer_module_exit(void);

//=============================================================================
// function define
//=============================================================================
static int nvt_timer_open(struct inode *inode, struct file *file)
{
	XXX_DRV_INFO *pdrv_info;

	pdrv_info = container_of(inode->i_cdev, XXX_DRV_INFO, cdev);

	pdrv_info = container_of(inode->i_cdev, XXX_DRV_INFO, cdev);
	file->private_data = pdrv_info;

	if (nvt_timer_drv_open(&pdrv_info->module_info, MINOR(inode->i_rdev))) {
		nvt_dbg(ERR, "failed to open driver\n");
		return -1;
	}

	return 0;
}

static int nvt_timer_release(struct inode *inode, struct file *file)
{
	XXX_DRV_INFO *pdrv_info;

	pdrv_info = container_of(inode->i_cdev, XXX_DRV_INFO, cdev);
	nvt_timer_drv_release(&pdrv_info->module_info, MINOR(inode->i_rdev));
	return 0;
}

static long nvt_timer_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct inode *inode;
	PXXX_DRV_INFO pdrv;

	inode = file_inode(filp);
	pdrv = filp->private_data;

	return nvt_timer_drv_ioctl(MINOR(inode->i_rdev), &pdrv->module_info, cmd, arg);
}

struct file_operations nvt_timer_fops = {
	.owner   = THIS_MODULE,
	.open    = nvt_timer_open,
	.release = nvt_timer_release,
	.unlocked_ioctl = nvt_timer_ioctl,
	.llseek  = no_llseek,
};

static int nvt_timer_probe(struct platform_device *pdev)
{
	XXX_DRV_INFO *pdrv_info;
	const struct of_device_id *match;
	int ret = 0;
	int loop;
	u32 ep_en = 0x0;

	struct device_node* of_node = of_find_node_by_path("/ep_info");

	if (of_node) {
                ret = of_property_read_u32(of_node, "enable", &ep_en);
                if(ret < 0){
                        nvt_dbg(ERR, "ep_en not found ret=%d\n",ret);
                        ep_en=0;
                }
        }

	match = of_match_device(xxx_match_table, &pdev->dev);
	if (!match) {
		nvt_dbg(ERR, "Platform device not found \n");
		return -EINVAL;
	}

	pdrv_info = kzalloc(sizeof(XXX_DRV_INFO), GFP_KERNEL);
	if (!pdrv_info) {
		nvt_dbg(ERR, "failed to allocate memory\n");
		return -ENOMEM;
	}

	for (loop = 0 ; loop < MODULE_REG_NUM ; loop++) {
		pdrv_info->presource[loop] = platform_get_resource(pdev, IORESOURCE_MEM, loop);
		if (pdrv_info->presource[loop] == NULL) {
			nvt_dbg(ERR, "No IO memory resource defined:%d\n", loop);
			ret = -ENODEV;
			goto FAIL_FREE_BUF;
		}
	}

	for (loop = 0 ; loop < MODULE_REG_NUM ; loop++) {
		if (!request_mem_region(pdrv_info->presource[loop]->start, resource_size(pdrv_info->presource[loop]), pdev->name)) {
			nvt_dbg(ERR, "failed to request memory resource%d\n", loop);
#if (MODULE_REG_NUM > 1)
			for (; loop > 0 ;) {
				loop -= 1;
				release_mem_region(pdrv_info->presource[loop]->start, resource_size(pdrv_info->presource[loop]));
			}
#endif
			ret = -ENODEV;
			goto FAIL_FREE_BUF;
		}
	}

	for (loop = 0 ; loop < MODULE_REG_NUM ; loop++) {
		pdrv_info->module_info.io_addr[loop] = ioremap(pdrv_info->presource[loop]->start, resource_size(pdrv_info->presource[loop]));
		if (pdrv_info->module_info.io_addr[loop] == NULL) {
			nvt_dbg(ERR, "ioremap() failed in module%d\n", loop);
#if (MODULE_REG_NUM > 1)
			for (; loop > 0 ;) {
				loop -= 1;
			    iounmap(pdrv_info->module_info.io_addr[loop]);
			}
#endif
			ret = -ENODEV;
			goto FAIL_FREE_RES;
		}
	}

	for (loop = 0 ; loop < MODULE_IRQ_NUM; loop++) {
		pdrv_info->module_info.iinterrupt_id[loop] = platform_get_irq(pdev, loop);
		if (pdrv_info->module_info.iinterrupt_id[loop] < 0) {
			nvt_dbg(ERR, "No IRQ resource defined\n");
			ret = -ENODEV;
			goto FAIL_FREE_REMAP;
		}
	}

	if(ep_en == 0){
		//Get clock source
		for (loop = 0 ; loop < MODULE_CLK_NUM; loop++) {
			pdrv_info->module_info.pclk[loop] = clk_get(&pdev->dev, dev_name(&pdev->dev));
			if (IS_ERR(pdrv_info->module_info.pclk[loop])) {
				nvt_dbg(ERR, "faile to get clock%d source\n", loop);

				ret = -ENODEV;
				goto FAIL_FREE_REMAP;
			}
		}
	}else nvt_dbg(ERR, "ep don't get clock source\n");

	pdrv_info->module_info.ep_en = ep_en;
	//Dynamic to allocate Device ID
	if (vos_alloc_chrdev_region(&pdrv_info->dev_id, MODULE_MINOR_COUNT, MODULE_NAME)) {
		nvt_dbg(ERR, "Can't get device ID\n");
		ret = -ENODEV;
		goto FAIL_FREE_REMAP;
	}

	/* Register character device for the volume */
	cdev_init(&pdrv_info->cdev, &nvt_timer_fops);
	pdrv_info->cdev.owner = THIS_MODULE;

	if (cdev_add(&pdrv_info->cdev, pdrv_info->dev_id, MODULE_MINOR_COUNT)) {
		nvt_dbg(ERR, "Can't add cdev\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

	pdrv_info->pmodule_class = class_create(THIS_MODULE, MODULE_NAME);
	if (IS_ERR(pdrv_info->pmodule_class)) {
		nvt_dbg(ERR, "failed in creating class.\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

	/* register your own device in sysfs, and this will cause udev to create corresponding device node */
	for (loop = 0 ; loop < (MODULE_MINOR_COUNT) ; loop++) {
		pdrv_info->pdevice[loop] = device_create(pdrv_info->pmodule_class, NULL
			, MKDEV(MAJOR(pdrv_info->dev_id), (loop + MINOR(pdrv_info->dev_id))), NULL
			, MODULE_NAME"%d", loop);

		if (IS_ERR(pdrv_info->pdevice[loop])) {
			nvt_dbg(ERR, "failed in creating device%d.\n", loop);

#if (MODULE_MINOR_COUNT > 1)
			for (; loop > 0 ; loop--) {
				device_unregister(pdrv_info->pdevice[loop - 1]);
			}
#endif
			ret = -ENODEV;
			goto FAIL_CLASS;
		}
	}

	ret = nvt_timer_proc_init(pdrv_info);
	if (ret) {
		nvt_dbg(ERR, "failed in creating proc.\n");
		goto FAIL_DEV;
	}
	ret = heavyload_proc_init(pdrv_info);
	if (ret) {
		nvt_dbg(ERR, "failed in creating heavyload proc.\n");
		goto FAIL_DEV;
	}

#if 0
	ret = nvt_ddr_proc_init(pdrv_info);
	if (ret) {
		nvt_dbg(ERR, "failed in creating ddr proc.\n");
		goto FAIL_PROC_INIT;
	}
#endif

	ret = nvt_timer_drv_init(&pdrv_info->module_info);
	if (ret) {
		nvt_dbg(ERR, "failed in creating proc.\n");
		goto FAIL_PROC_INIT;
	}

	ret = hwclock_proc_init(pdrv_info);
	if (ret) {
		nvt_dbg(ERR, "failed in creating hwclock proc.\n");
		goto FAIL_HWCLOCK_PROC_INIT;
	}

	// hwclock should init after timer driver
	hwclock_init();

	platform_set_drvdata(pdev, pdrv_info);

#if 1
	ret = nvt_jiffies_proc_init();
	if (ret) {
		nvt_dbg(ERR, "failed in creating nvt_jiffies proc.\n");
		goto FAIL_JIFFIES_PROC_INIT;
	}
#endif

	return ret;

#if 1
FAIL_JIFFIES_PROC_INIT:
	nvt_jiffies_proc_remove();
#endif

FAIL_HWCLOCK_PROC_INIT:
	hwclock_proc_remove(pdrv_info);

//FAIL_DRV_INIT:
	//heavyload_proc_remove(pdrv_info);

FAIL_PROC_INIT:
	nvt_timer_proc_remove(pdrv_info);

FAIL_DEV:
	for (loop = 0 ; loop < (MODULE_MINOR_COUNT) ; loop++)
		device_unregister(pdrv_info->pdevice[loop]);

FAIL_CLASS:
	class_destroy(pdrv_info->pmodule_class);

FAIL_CDEV:
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, MODULE_MINOR_COUNT);

FAIL_FREE_REMAP:
	for (loop = 0 ; loop < MODULE_REG_NUM ; loop++)
		iounmap(pdrv_info->module_info.io_addr[loop]);

FAIL_FREE_RES:
	for (loop = 0 ; loop < MODULE_REG_NUM ; loop++)
		release_mem_region(pdrv_info->presource[loop]->start, resource_size(pdrv_info->presource[loop]));

FAIL_FREE_BUF:
	kfree(pdrv_info);
	return ret;
}

static int nvt_timer_remove(struct platform_device *pdev)
{
	PXXX_DRV_INFO pdrv_info;
	unsigned char ucloop;

	nvt_dbg(IND, "\n");

	pdrv_info = platform_get_drvdata(pdev);

	hwclock_exit();

	nvt_timer_drv_remove(&pdrv_info->module_info);

	nvt_timer_proc_remove(pdrv_info);

	heavyload_proc_remove(pdrv_info);
#if 1
	nvt_jiffies_proc_remove();
#endif

	hwclock_proc_remove(pdrv_info);

	if (pdrv_info->module_info.ep_en == 0){
		for (ucloop = 0 ; ucloop < MODULE_CLK_NUM; ucloop++)
			clk_put(pdrv_info->module_info.pclk[ucloop]);
	}
	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT) ; ucloop++)
		device_unregister(pdrv_info->pdevice[ucloop]);

	class_destroy(pdrv_info->pmodule_class);
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, MODULE_MINOR_COUNT);

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
	    iounmap(pdrv_info->module_info.io_addr[ucloop]);

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
		release_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));

	kfree(pdrv_info);
	return 0;
}

#ifdef CONFIG_PM
static int nvt_timer_suspend(struct platform_device *pdev, pm_message_t state)
{
	PXXX_DRV_INFO pdrv_info;;

	nvt_dbg(IND, "start\n");

	pdrv_info = platform_get_drvdata(pdev);
	nvt_timer_drv_suspend(&pdrv_info->module_info);

	nvt_dbg(IND, "finished\n");
	return 0;
}


static int nvt_timer_resume(struct platform_device *pdev)
{
	PXXX_DRV_INFO pdrv_info;;

	nvt_dbg(IND, "start\n");

	pdrv_info = platform_get_drvdata(pdev);
	nvt_timer_drv_resume(&pdrv_info->module_info);

	nvt_dbg(IND, "finished\n");
	return 0;
}
#else
static int nvt_timer_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int nvt_timer_resume(struct platform_device *pdev)
{
	return 0;
}
#endif

static struct platform_driver nvt_timer_driver = {
	.driver = {
				.name   = "nvt_timer",
				.owner = THIS_MODULE,
				.of_match_table = xxx_match_table,
				},
	.probe      = nvt_timer_probe,
	.remove     = nvt_timer_remove,
	.suspend = nvt_timer_suspend,
	.resume = nvt_timer_resume
};

#if 0
float f_test(float a, float b)
{
	return (a*13.2 + b) / 1.3;
}
#endif

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

int __init nvt_timer_module_init(void)
{
	int ret;
	float a;
	float b = 3;

	a = 128.0 * 10;
	ret = a / b;

	ret = platform_driver_register(&nvt_timer_driver);

	return 0;
}

void __exit nvt_timer_module_exit(void)
{
	nvt_dbg(ERR, "aaaaaaaaa\n");
	platform_driver_unregister(&nvt_timer_driver);
}

module_init(nvt_timer_module_init);
module_exit(nvt_timer_module_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("nvt timer driver");
MODULE_VERSION("1.00.004");
MODULE_LICENSE("GPL");
