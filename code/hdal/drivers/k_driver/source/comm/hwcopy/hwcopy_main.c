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
#include "hwcopy_drv.h"
#include "hwcopy_reg.h"
#include "hwcopy_main.h"
#include "hwcopy_proc.h"
#include "hwcopy_dbg.h"
#include <linux/soc/nvt/nvt-pcie-lib.h>

//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================
#ifdef DEBUG
unsigned int hwcopy_debug_level = NVT_DBG_WRN;
module_param_named(hwcopy_debug_level, hwcopy_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(hwcopy_debug_level, "Debug message level");
#endif

//=============================================================================
// Global variable
//=============================================================================
static struct of_device_id hwcopy_match_table[] = {
	{	.compatible = "nvt,nvt_hwcopy"},
	//{       .compatible = "nvt,nvt_hwcopy_ep0"},
	{}
};

//=============================================================================
// function declaration
//=============================================================================
static int nvt_hwcopy_open(struct inode *inode, struct file *file);
static int nvt_hwcopy_release(struct inode *inode, struct file *file);
static long nvt_hwcopy_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static int nvt_hwcopy_probe(struct platform_device *pdev);
//static int nvt_hwcopy_suspend(struct platform_device *pdev, pm_message_t state);
//static int nvt_hwcopy_resume(struct platform_device *pdev);
static int nvt_hwcopy_remove(struct platform_device *pdev);
int __init nvt_hwcopy_module_init(void);
void __exit nvt_hwcopy_module_exit(void);

char module_name[128];

//=============================================================================
// function define
//=============================================================================
static int nvt_hwcopy_open(struct inode *inode, struct file *file)
{
	HWCOPY_DRV_INFO *pdrv_info;

	nvt_dbg(WRN, "\n");

	pdrv_info = container_of(inode->i_cdev, HWCOPY_DRV_INFO, cdev);
	file->private_data = pdrv_info;

	if (nvt_hwcopy_drv_open(&pdrv_info->module_info, MINOR(inode->i_rdev))) {
		nvt_dbg(ERR, "failed to open driver\n");
		return -1;
	}

	return 0;
}

static int nvt_hwcopy_release(struct inode *inode, struct file *file)
{
	HWCOPY_DRV_INFO *pdrv_info;

	pdrv_info = container_of(inode->i_cdev, HWCOPY_DRV_INFO, cdev);

	nvt_hwcopy_drv_release(&pdrv_info->module_info, MINOR(inode->i_rdev));
	return 0;
}

static long nvt_hwcopy_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct inode *inode;
	PHWCOPY_DRV_INFO pdrv;

	inode = file_inode(filp);
	pdrv = filp->private_data;

	return nvt_hwcopy_drv_ioctl(MINOR(inode->i_rdev), &pdrv->module_info, cmd, arg);
}

struct file_operations nvt_hwcopy_fops = {
	.owner   = THIS_MODULE,
	.open    = nvt_hwcopy_open,
	.release = nvt_hwcopy_release,
	.unlocked_ioctl = nvt_hwcopy_ioctl,
	.llseek  = no_llseek,
};

static int nvt_hwcopy_probe(struct platform_device *pdev)
{
// 	HWCOPY_DRV_INFO *pdrv_info;//info;
// 	const struct of_device_id *match;
	int ret = 0;
// 	unsigned int device_id, ucloop;
// 	struct property *prop;
// 	struct device_node *node = pdev->dev.of_node;
// 	char module_name[128];

// 	nvt_dbg(ERR, "%s \n", pdev->name);

// 	match = of_match_device(hwcopy_match_table, &pdev->dev);
// 	if (!match) {
// 		nvt_dbg(ERR, "Platform device not found \n");
// 		return -EINVAL;
// 	}

// 	prop = of_find_property(node, "device-id", &device_id);
// 	if (prop) {
// 		ret = of_property_read_u32(node, "device-id", &device_id);
// 		if(!ret){
// 			if (device_id >= HWCP_MAX_CHIP) {
// 				nvt_dbg(ERR, "device_id:%d excceds %d! \n", device_id, HWCP_MAX_CHIP);
// 				return -EINVAL;
// 			}
// 		}
// 		else{
// 			device_id = 0;
// 			nvt_dbg(ERR,"device-id read fail,ret=%d",ret);
// 		}
// 	}
// 	else device_id = 0;

// 	pdrv_info = kzalloc(sizeof(HWCOPY_DRV_INFO), GFP_KERNEL);
//         if (!pdrv_info) {
//                 nvt_dbg(ERR, "failed to allocate memory\n");
//                 return -ENOMEM;
//         }


// 	pdrv_info->chip_idx = device_id;
// 	pdrv_info->module_info.chip_idx = device_id;

// 	pdrv_info->dev = &pdev->dev;

// 	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
// 		pdrv_info->presource[ucloop] = platform_get_resource(pdev, IORESOURCE_MEM, ucloop);
// 		if (pdrv_info->presource[ucloop] == NULL) {
// 			nvt_dbg(ERR, "No IO memory resource defined:%d\n", ucloop);
// 			ret = -ENODEV;
// 			goto FAIL_FREE_BUF;
// 		}
// 	}

// 	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
// 		//nvt_dbg(IND, "%d. resource:0x%x size:0x%x\n", ucloop, pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
// 		if (!request_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]), pdev->name)) {
// 			nvt_dbg(ERR, "failed to request memory resource%d\n", ucloop);
// #if (MODULE_REG_NUM > 1)
// 			for ( ; ucloop > 0; ) {
// 				ucloop -= 1;
// 				release_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
// 			}
// #endif
// 			ret = -ENODEV;
// 			goto FAIL_FREE_BUF;
// 		}
// 	}

// 	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
// 		pdrv_info->module_info.io_addr[ucloop] = ioremap_nocache(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
// 		if (pdrv_info->module_info.io_addr[ucloop] == NULL) {
// 			nvt_dbg(ERR, "ioremap() failed in module%d\n", ucloop);
// #if (MODULE_REG_NUM > 1)
// 			for ( ; ucloop > 0; ) {
// 				ucloop -= 1;
// 			    iounmap(pdrv_info->module_info.io_addr[ucloop]);
// 			}
// #endif
// 			ret = -ENODEV;
// 			goto FAIL_FREE_RES;
// 		}
// 	}

// 	for (ucloop = 0 ; ucloop < MODULE_IRQ_NUM; ucloop++) {
// 		pdrv_info->module_info.iinterrupt_id[ucloop] = platform_get_irq(pdev, ucloop);
// 		//nvt_dbg(IND, "IRQ %d. ID%d\n", ucloop, pdrv_info->module_info.iinterrupt_id[ucloop]);
// 		if (pdrv_info->module_info.iinterrupt_id[ucloop] < 0) {
// 			nvt_dbg(ERR, "No IRQ resource defined\n");
// 			ret = -ENODEV;
// 			goto FAIL_FREE_REMAP;
// 		}
// 	}

// 	//Get clock source
// 	for (ucloop = 0 ; ucloop < MODULE_CLK_NUM; ucloop++) {
// 		pdrv_info->module_info.pclk[ucloop] = clk_get(&pdev->dev, dev_name(&pdev->dev));
// 		if (IS_ERR(pdrv_info->module_info.pclk[ucloop])) {
// 			nvt_dbg(ERR, "faile to get clock%d source\n", ucloop);

// 			ret = -ENODEV;
// 			goto FAIL_FREE_REMAP;
// 		}
// 	}
// 	if(device_id > 0){
// 		snprintf(module_name, sizeof(module_name), "nvt_hwcopy_ep%d", device_id-1);
// 	}
// 	else{
// 		snprintf(module_name, sizeof(module_name), "nvt_hwcopy");
// 	}

// 	//Dynamic to allocate Device ID
// 	if (alloc_chrdev_region(&pdrv_info->dev_id, MODULE_MINOR_ID, MODULE_MINOR_COUNT, module_name)) {
// 		nvt_dbg(ERR, "Can't get device ID\n");
// 		ret = -ENODEV;
// 		goto FAIL_FREE_REMAP;
// 	}

// 	nvt_dbg(ERR, "DevID Major:%d minor:%d\n", MAJOR(pdrv_info->dev_id), MINOR(pdrv_info->dev_id));

// 	/* Register character device for the volume */
// 	cdev_init(&pdrv_info->cdev, &nvt_hwcopy_fops);
// 	pdrv_info->cdev.owner = THIS_MODULE;

// 	if (cdev_add(&pdrv_info->cdev, pdrv_info->dev_id, MODULE_MINOR_COUNT)) {
// 		nvt_dbg(ERR, "Can't add cdev\n");
// 		ret = -ENODEV;
// 		goto FAIL_CDEV;
// 	}

// 	pdrv_info->pmodule_class = class_create(THIS_MODULE, module_name);
// 	if (IS_ERR(pdrv_info->pmodule_class)) {
// 		nvt_dbg(ERR, "failed in creating class.\n");
// 		ret = -ENODEV;
// 		goto FAIL_CDEV;
// 	}

// 	/* register your own device in sysfs, and this will cause udev to create corresponding device node */
// 	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT) ; ucloop++) {
// 		pdrv_info->pdevice[ucloop] = device_create(pdrv_info->pmodule_class, NULL
// 			, MKDEV(MAJOR(pdrv_info->dev_id), (ucloop + MODULE_MINOR_ID)), NULL
// 			, MODULE_NAME"%d", ucloop);

// 		if (IS_ERR(pdrv_info->pdevice[ucloop])) {
// 			nvt_dbg(ERR, "failed in creating device%d.\n", ucloop);
// #if (MODULE_REG_NUM > 1)
// 			for (; ucloop > 0 ; ucloop--)
// 				device_unregister(pdrv_info->pdevice[ucloop - 1]);
// #endif
// 			ret = -ENODEV;
// 			goto FAIL_CLASS;
// 		}
// 	}

// 	ret = nvt_hwcopy_proc_init(pdrv_info);
// 	if (ret) {
// 		nvt_dbg(ERR, "failed in creating proc.\n");
// 		goto FAIL_DEV;
// 	}

// 	ret = nvt_hwcopy_drv_init(&pdrv_info->module_info);

// 	platform_set_drvdata(pdev, pdrv_info);
// 	if (ret) {
// 		nvt_dbg(ERR, "failed in creating proc.\n");
// 		goto FAIL_DRV_INIT;
// 	}

	return ret;

// FAIL_DRV_INIT:
// 	nvt_hwcopy_proc_remove(pdrv_info);

// FAIL_DEV:
// 	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT) ; ucloop++)
// 		device_unregister(pdrv_info->pdevice[ucloop]);

// FAIL_CLASS:
// 	class_destroy(pdrv_info->pmodule_class);

// FAIL_CDEV:
// 	cdev_del(&pdrv_info->cdev);
// 	unregister_chrdev_region(pdrv_info->dev_id, MODULE_MINOR_COUNT);

// FAIL_FREE_REMAP:
// 	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
// 	    iounmap(pdrv_info->module_info.io_addr[ucloop]);

// FAIL_FREE_RES:
// 	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
// 		release_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));

// FAIL_FREE_BUF:
// 	kfree(pdrv_info);
// 	pdrv_info = NULL;
// 	platform_set_drvdata(pdev, pdrv_info);
// 	return ret;
}

static int nvt_hwcopy_remove(struct platform_device *pdev)
{
	// PHWCOPY_DRV_INFO pdrv_info;
	// unsigned char ucloop;

	// nvt_dbg(IND, "\n");

	// pdrv_info = platform_get_drvdata(pdev);

	// nvt_hwcopy_drv_remove(&pdrv_info->module_info);

	// nvt_hwcopy_proc_remove(pdrv_info);

	// for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT) ; ucloop++)
	// 	device_unregister(pdrv_info->pdevice[ucloop]);

	// class_destroy(pdrv_info->pmodule_class);
	// cdev_del(&pdrv_info->cdev);
	// unregister_chrdev_region(pdrv_info->dev_id, MODULE_MINOR_COUNT);

	// for (ucloop = 0 ; ucloop < MODULE_CLK_NUM; ucloop++)
	// 	clk_put(pdrv_info->module_info.pclk[ucloop]);

	// for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
	//     iounmap(pdrv_info->module_info.io_addr[ucloop]);

	// for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
	// 	release_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));

	// kfree(pdrv_info);
	// pdrv_info = NULL;
	// platform_set_drvdata(pdev, pdrv_info);
	return 0;
}
/*
static int nvt_hwcopy_suspend(struct platform_device *pdev, pm_message_t state)
{
	PHWCOPY_DRV_INFO pdrv_info;;

	nvt_dbg(IND, "start\n");

	pdrv_info = platform_get_drvdata(pdev);
	nvt_hwcopy_drv_suspend(&pdrv_info->module_info);

	nvt_dbg(IND, "finished\n");
	return 0;
}


static int nvt_hwcopy_resume(struct platform_device *pdev)
{
	PHWCOPY_DRV_INFO pdrv_info;;

	nvt_dbg(IND, "start\n");

	pdrv_info = platform_get_drvdata(pdev);
	nvt_hwcopy_drv_resume(&pdrv_info->module_info);

	nvt_dbg(IND, "finished\n");
	return 0;
}
*/
static struct platform_driver nvt_hwcopy_driver = {
	.driver = {
			.name   = "nvt_hwcopy",
			.owner = THIS_MODULE,
			.of_match_table = hwcopy_match_table,
		      },
	.probe      = nvt_hwcopy_probe,
	.remove     = nvt_hwcopy_remove,
	//.suspend = nvt_hwcopy_suspend,
	//.resume = nvt_hwcopy_resume
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

int __init nvt_hwcopy_module_init(void)
{
	int ret;

	nvt_dbg(WRN, "\n");
	ret = platform_driver_register(&nvt_hwcopy_driver);

	return 0;
}

void __exit nvt_hwcopy_module_exit(void)
{
	nvt_dbg(WRN, "\n");
	platform_driver_unregister(&nvt_hwcopy_driver);
}

module_init(nvt_hwcopy_module_init);
module_exit(nvt_hwcopy_module_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("Hwcopy driver");
MODULE_VERSION("1.00.008");
MODULE_LICENSE("GPL");
