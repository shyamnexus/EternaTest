#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/of_device.h>

#include <asm/signal.h>

#include <kwrap/dev.h>
#include "kwrap/type.h"

#include "nvt_vdocdc_main.h"
#include "nvt_vdocdc_dbg.h"
#include "nvt_vdocdc_drv.h"

#ifdef CONFIG_PM
#include <linux/soc/nvt/nvt-info.h>
#include "h26x.h"
#endif

//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================
#ifdef DEBUG
//unsigned int kdrv_vdocdc_debug_level = (NVT_DBG_WRN | NVT_DBG_ERR);
module_param_named(nvt_vdocdc_debug_level, nvt_vdocdc_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(nvt_vdocdc_debug_level, "Debug message level");
#endif

//=============================================================================
// Global variable
//=============================================================================
static struct of_device_id nvt_vdocdc_match_table[] = {
	{	.compatible = "nvt,nvt_venc"},
	{}
};

struct file_operations nvt_vdocdc_fops = {
    .owner   = THIS_MODULE,
    .llseek  = no_llseek,
};

static int nvt_vdocdc_probe(struct platform_device * pdev)
{
	NVT_VDOCDC_DRV_INFO *pdrv_info;
	unsigned char ucloop;
	int ret = 0;

	nvt_dbg(INFO, "%s\n", pdev->name);

	if (of_match_device(nvt_vdocdc_match_table, &pdev->dev) == NULL) {
		nvt_dbg(ERR, "Platform device not found \n");
		return -EINVAL;
	}

	pdrv_info = kzalloc(sizeof(NVT_VDOCDC_DRV_INFO), GFP_KERNEL);
    if (!pdrv_info) {
        nvt_dbg(ERR, "failed to allocate memory\n");
        return -ENOMEM;
    }

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		pdrv_info->presource[ucloop] = platform_get_resource(pdev, IORESOURCE_MEM, ucloop);
		if (pdrv_info->presource[ucloop] == NULL) {
			nvt_dbg(ERR, "No IO memory resource defined:%d\n", ucloop);
			ret = -ENODEV;
			goto FAIL_FREE_BUF;
		}
	}

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		if (!request_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]), pdev->name)) {
			nvt_dbg(ERR, "failed to request memory resource(%d) %08x, %08x\n", ucloop, pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
			for (; ucloop > 0 ;) {
				ucloop -= 1;
				release_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
			}
			ret = -ENODEV;
			goto FAIL_FREE_BUF;
		}
		nvt_dbg(INFO, "resource(%d): addr(0x%x), size(0x%x)\n", ucloop, pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
	}

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		pdrv_info->module_info.io_addr[ucloop] = ioremap(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
		if (pdrv_info->module_info.io_addr[ucloop] == NULL) {
			nvt_dbg(ERR, "ioremap() failed in module%d\n", ucloop);
			for (; ucloop > 0 ;) {
				ucloop -= 1;
			    iounmap(pdrv_info->module_info.io_addr[ucloop]);
			}
			ret = -ENODEV;
			goto FAIL_FREE_RES;
		}
		nvt_dbg(INFO, "io addr(%d): addr(0x%x), size(0x%08x)\n", ucloop, (UINT32)pdrv_info->module_info.io_addr[ucloop], (UINT32)resource_size(pdrv_info->presource[ucloop]));
	}

	for (ucloop = 0 ; ucloop < MODULE_IRQ_NUM; ucloop++) {
		pdrv_info->module_info.iinterrupt_id[ucloop] = platform_get_irq(pdev, ucloop);
		nvt_dbg(INFO, "IRQ %d. ID%d\n", ucloop,pdrv_info->module_info.iinterrupt_id[ucloop]);
		if (pdrv_info->module_info.iinterrupt_id[ucloop] < 0) {
			nvt_dbg(ERR, "No IRQ resource defined\n");
			ret = -ENODEV;
			goto FAIL_FREE_REMAP;
		}
		nvt_dbg(INFO, "IRQ(%d): ID(%d)\n", ucloop, pdrv_info->module_info.iinterrupt_id[ucloop]);
	}

	for (ucloop = 0 ; ucloop < MODULE_CLK_NUM; ucloop++) {
		char pstr1[16];
		snprintf(pstr1, 16, "venc_clk");
		pdrv_info->module_info.pclk[ucloop] = clk_get(&pdev->dev, pstr1);
		if (IS_ERR(pdrv_info->module_info.pclk[ucloop])) {
			nvt_dbg(ERR, "faile to get clock%d source (%s)\n", ucloop, pstr1);
			ret = -ENODEV;
			goto FAIL_FREE_REMAP;
		}
		nvt_dbg(IND, "clock(%d): %d\n", ucloop, (UINT32)(clk_get_rate(pdrv_info->module_info.pclk[ucloop])));
	}

	//Dynamic to allocate Device ID
	if (vos_alloc_chrdev_region(&pdrv_info->dev_id, MODULE_MINOR_COUNT, MODULE_NAME)) {
		nvt_dbg(ERR, "Can't get device ID\n");
		ret = -ENODEV;
		goto FAIL_FREE_REMAP;
	}
	nvt_dbg(INFO, "DevID Major:%d minor:%d\n", MAJOR(pdrv_info->dev_id), MINOR(pdrv_info->dev_id));

	/* Register character device for the volume */
	cdev_init(&pdrv_info->cdev, &nvt_vdocdc_fops);
	pdrv_info->cdev.owner = THIS_MODULE;

	if (cdev_add(&pdrv_info->cdev, pdrv_info->dev_id, MODULE_MINOR_COUNT)) {
		nvt_dbg(ERR, "Can't add cdev\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

	pdrv_info->pmodule_class = class_create(THIS_MODULE, MODULE_NAME);
	if(IS_ERR(pdrv_info->pmodule_class)) {
		nvt_dbg(ERR, "failed in creating class.\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

	/* register your own device in sysfs, and this will cause udev to create corresponding device node */
	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT) ; ucloop++) {
		pdrv_info->pdevice[ucloop] = device_create(pdrv_info->pmodule_class, NULL
			, MKDEV(MAJOR(pdrv_info->dev_id), (ucloop + MINOR(pdrv_info->dev_id))), NULL
			, MODULE_NAME"%d", ucloop);

		if(IS_ERR(pdrv_info->pdevice[ucloop])) {
			nvt_dbg(ERR, "failed in creating device%d.\n", ucloop);
			for (; ucloop > 0 ;) {
				ucloop -= 1;
				device_unregister(pdrv_info->pdevice[ucloop]);
			}
			ret = -ENODEV;
			goto FAIL_CLASS;
		}
	}

	if ((ret = nvt_vdocdc_drv_init(&pdrv_info->module_info)) != 0) {
		nvt_dbg(ERR, "failed in creating proc.\n");
		goto FAIL_DEV;
	}

	platform_set_drvdata(pdev, pdrv_info);

	return ret;

FAIL_DEV:
	for (ucloop = 0 ; ucloop < MODULE_MINOR_COUNT ; ucloop++)
		device_unregister(pdrv_info->pdevice[ucloop]);

FAIL_CLASS:
	class_destroy(pdrv_info->pmodule_class);

FAIL_CDEV:
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, MODULE_MINOR_COUNT);

FAIL_FREE_REMAP:
	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
		iounmap(pdrv_info->module_info.io_addr[ucloop]);

FAIL_FREE_RES:
	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
		release_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));

FAIL_FREE_BUF:
	kfree(pdrv_info);
	pdrv_info = NULL;

	return ret;
}

static int nvt_vdocdc_remove(struct platform_device * pdev)
{
	NVT_VDOCDC_DRV_INFO *pdrv_info;
	unsigned char ucloop;

	nvt_dbg(INFO, "\n");

	pdrv_info = platform_get_drvdata(pdev);

	nvt_vdocdc_drv_remove(&pdrv_info->module_info);

	for (ucloop = 0 ; ucloop < MODULE_MINOR_COUNT; ucloop++) {
		clk_put(pdrv_info->module_info.pclk[ucloop]);
	}

	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT ) ; ucloop++)
		device_unregister(pdrv_info->pdevice[ucloop]);

	class_destroy(pdrv_info->pmodule_class);
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, MODULE_MINOR_COUNT);

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
	    iounmap(pdrv_info->module_info.io_addr[ucloop]);

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
		release_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));

    kfree(pdrv_info);
	pdrv_info = NULL;

	return 0;
}

#ifdef CONFIG_PM
static int nvt_h26x_suspend(struct device *dev)
{
	NVT_VDOCDC_DRV_INFO *eng_info;

	if (nvt_get_suspend_mode() == 1) { /* 0: not power off, 1: power off */
		eng_info = dev_get_drvdata(dev);
		if (eng_info == NULL) {
			return -EINVAL;
		}
	}

	return 0;
}

static int nvt_h26x_resume(struct device *dev)
{
	NVT_VDOCDC_DRV_INFO *eng_info;

	if (nvt_get_suspend_mode() == 1) { /* 0: not power off, 1: power off */
		eng_info = dev_get_drvdata(dev);
		if (eng_info == NULL) {
			return -EINVAL;
		}

		h26x_init_wrapper(0);
		h26x_setChkSumEn(0, TRUE);
		h26x_setCodecClock(0, TRUE);
		h26x_setCodecPClock(0, TRUE);
		h26x_setSRAMMode(0, 0x32, 1, 0, 1);
		h26x_setLLDummyWriteNum(0, H26X_MAX_DUMMY_WRITE);
		h26x_setDramBurstLen(0, 0);
		h26x_setPatchCount(0, 0);
	}

	return 0;
}

static const struct dev_pm_ops nvt_h26x_pm_pos = {
	.suspend = nvt_h26x_suspend,
	.resume = nvt_h26x_resume,
};
#endif //CONFIG_PM

static struct platform_driver nvt_vdocdc_driver = {
    .driver = {
		        .name  = "nvt_h26x",
				.owner = THIS_MODULE,
				.of_match_table = nvt_vdocdc_match_table,
#ifdef CONFIG_PM
				.pm = &nvt_h26x_pm_pos,
#endif
		      },

	.probe   = nvt_vdocdc_probe,
    .remove  = nvt_vdocdc_remove,
};

#if 0
int __init kdrv_vdocdc_module_init(void)
{
	int ret;

    //nvt_dbg(WARN, "\n");
	ret = platform_driver_register(&kdrv_vdocdc_driver);

	return 0;
}

void __exit kdrv_vdocdc_module_exit(void)
{
    //nvt_dbg(WARN, "\n");
	platform_driver_unregister(&kdrv_vdocdc_driver);
}

module_init(kdrv_vdocdc_module_init);
module_exit(kdrv_vdocdc_module_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("vdocdc driver");
MODULE_LICENSE("GPL");
#else
int nvt_vdocdc_module_init(void)
{
	int ret;

    //nvt_dbg(WARN, "\n");
	ret = platform_driver_register(&nvt_vdocdc_driver);

	return 0;
}

void nvt_vdocdc_module_exit(void)
{
    //nvt_dbg(WARN, "\n");
	platform_driver_unregister(&nvt_vdocdc_driver);
}

#endif
