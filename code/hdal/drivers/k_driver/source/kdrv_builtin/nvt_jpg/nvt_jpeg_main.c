#ifdef __KERNEL__
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
#include <linux/version.h>
#include <linux/clk-provider.h> //__clk_is_enabled
#ifdef CONFIG_PM
#include <linux/soc/nvt/nvt-info.h>
#endif
#include <asm/signal.h>
#include <kwrap/dev.h>
#include <nvt_api_ver.h>
#include "jpeg_drv.h"
#include "jpeg_main.h"
#include "jpeg_proc.h"
#include "jpeg_dbg.h"
#include "jpeg_kdrv.h"
#include "jpeg_platform.h"

//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================
// 1.0.08: support JPEG RC
// 1.0.12: disable bs offset
// 1.0.13: add FREERTOS engine change
// 1.0.14: add proc of rate control
// 1.0.15: header file independently
// 1.0.16: add sw timeout and hw reset
// 1.0.17: sync va2pa (putjob)
// 1.0.18: add sw gating
// 1.0.19: bs buffer cache sync
// 1.0.20: decode bs size without header
// 1.0.21: revert bs size for stable version and fixed irq latency
// 1.0.22: fixed missing clock disable
// 1.0.23: decode bs size without header, add extra buffer
// 1.0.25: add version check function
// 1.0.27: fixed sw decode missing parameters and decode bs size
// 1.0.28: fine tune spinlock and clock api
// 1.0.29: osg graph add physical address
// 1.0.30: sync xvr modification
// 1.0.32: add return value and callback to flow
// 1.0.33: extend to 32 decode paths (flow)
// 1.0.34: add checksum to app header, modify debug level
// 1.0.35: checksum header default disable
// 1.0.36: increase jpeg max path to 33
// 1.0.37: modify sw decode flow (format422)
// 1.0.38: fixed remove module fail
// 1.0.39: callback base_qp fixed, change checksum app marker to app4
// 1.0.40: insufficient data stuff zeros, clk default disable, next buf ready
// 1.0.41: modify c_addr connected with y_addr (align 64x16)
// 1.0.42: support user data insert to header
// 1.0.45: support stop job
// 1.0.46: add busy condition in blocking mode
// 1.0.47: fixed auto test fail
// 1.0.48: fixed multi engine issue when slice mode enable
// 1.0.49: adjust mask line thickness
// 1.1.01: code sync from 530/690
// 1.1.02: support JPEG JFIF header and modify stop job flow when atomic
// 1.1.03: before set q_table, need to set format first
// 1.1.04: modified output format when decode transform
// 1.1.05: sync clock disable from 530
// 1.1.06: partition block and non-block mode
// 1.1.07: check first_done when jpeg_engine_init
// 1.1.08: clear transform parameter when decode
// 1.1.09: add return error code when encode fail
// 1.1.10: add osg mosaic block size setting
// 1.1.11: remove NOVA header (marker 0xE4)
// 1.1.12: support suspend/resume function
// 1.1.13: add api of new VBR (adjust priority of frame rate quality)
// 1.1.14: support block mode sw decode
// 1.1.15: support fastboot
// 1.1.16: 1. add engine index checker, 2. set io_addr when isr bottom, 3. check clk enable for hw reset
// 1.1.17: fix bug of preprocess pointer error
// 1.1.18: support encode 100 & decode 100
// 1.1.19: avoid enc/dec reentrant
// 1.1.20: fix bug of sw decoder & error handing
// 1.1.21: fix bug of sw decode and hw decode conflict
// 1.1.22: check bs addres when slice encode
// 1.1.23: [NA51115-1535] 1. update reset flow (dma abort), 2. when dma is not idle, must do reset
// 1.1.24: [IVOT_N12261_CO_66-218] add checker of min resolution
// 1.1.25: support set roi table
// 1.1.26: add api of decode paddding disable
#define DRV_JPG_VERSION				"1.1.26"

unsigned int jpeg_debug_level = NVT_DBG_WARN;//NVT_DBG_INFO;//(NVT_DBG_INFO | NVT_DBG_WARN | NVT_DBG_ERR);
#ifdef DEBUG
module_param_named(jpeg_debug_level, jpeg_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(jpeg_debug_level, "Debug message level");
#endif

JPEG_ENGINE_DATA jpeg_eng_data[JPEG_MAX_ENTITYS] = {0};

//=============================================================================
// Global variable
//=============================================================================
static struct of_device_id jpg_match_table[] = {
	{ .compatible = "nvt,jpeg"	},
	// { .compatible = "nvt,jpeg_2"	},
	// { .compatible = "nvt,jpeg_lite"	},
	{}
};

UINT32 first_done = 0;

//=============================================================================
// function declaration
//=============================================================================
static int nvt_builtinjpg_open(struct inode *inode, struct file *file);
static int nvt_builtinjpg_release(struct inode *inode, struct file *file);
//static long nvt_builtinjpg_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static int nvt_builtinjpg_probe(struct platform_device *pdev);
#ifndef CONFIG_PM
static int nvt_builtinjpg_suspend(struct platform_device *pdev, pm_message_t state);
static int nvt_builtinjpg_resume(struct platform_device *pdev);
#endif
static int nvt_builtinjpg_remove(struct platform_device *pdev);
int __init nvt_builtinjpg_module_init(void);
void __exit nvt_builtinjpg_module_exit(void);

//=============================================================================
// function define
//=============================================================================
char *nvt_jpeg_get_version(void)
{
	return DRV_JPG_VERSION;
}

static int nvt_builtinjpg_open(struct inode *inode, struct file *file)
{
	JPG_DRV_INFO *pdrv_info;

	pdrv_info = container_of(inode->i_cdev, JPG_DRV_INFO, cdev);

	pdrv_info = container_of(inode->i_cdev, JPG_DRV_INFO, cdev);
	file->private_data = pdrv_info;

	if (nvt_jpg_drv_open(&pdrv_info->module_info, MINOR(inode->i_rdev))) {
		DBG_ERR("failed to open driver\n");
		return -1;
	}

	return 0;
}

static int nvt_builtinjpg_release(struct inode *inode, struct file *file)
{
	JPG_DRV_INFO *pdrv_info;

	pdrv_info = container_of(inode->i_cdev, JPG_DRV_INFO, cdev);
	nvt_jpg_drv_release(&pdrv_info->module_info, MINOR(inode->i_rdev));

	return 0;
}
#if 0
static long nvt_builtinjpg_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct inode *inode;
	PJPG_DRV_INFO pdrv;

	inode = file_inode(filp);
	pdrv = filp->private_data;

	return nvt_builtinjpg_drv_ioctl(MINOR(inode->i_rdev), &pdrv->module_info, cmd, arg);
}
#endif

struct file_operations nvt_builtinjpg_fops = {
	.owner   = THIS_MODULE,
	.open    = nvt_builtinjpg_open,
	.release = nvt_builtinjpg_release,
	//.unlocked_ioctl = nvt_builtinjpg_ioctl,
	.llseek  = no_llseek,
};

static int nvt_builtinjpg_probe(struct platform_device *pdev)
{
	JPG_DRV_INFO *pdrv_info;//info;
	struct device_node *node = pdev->dev.of_node;
	const struct of_device_id *match;
	int ret = 0;
	// unsigned char ucloop;
	unsigned int dev_id = 0;
	char *tmp_module_name;
	extern NVT_API_CHK_DECLARE(nvt_jpeg);

	NVT_API_CHK_CALL(nvt_jpeg);

	DBG_INFO("%s\n", pdev->name);
	// printk("[%s], JPEG here! \n", __func__);
	// printk("dev id = %x\n", (unsigned int)(pdev->id));
	// printk("dev num_resources = %x\n", (unsigned int)(pdev->num_resources));
	// printk("jpeg_debug_level = %x\n", (unsigned int)(jpeg_debug_level));

	match = of_match_device(jpg_match_table, &pdev->dev);
	if (!match) {
		DBG_ERR("Platform device not found \n");
		printk("Platform device not found! \n");
		return -EINVAL;
	}

	pdrv_info = kzalloc(sizeof(JPG_DRV_INFO), GFP_KERNEL);
	if (!pdrv_info) {
		DBG_ERR("failed to allocate memory\n");
		return -ENOMEM;
	}

	if (of_property_read_u32(node, "device-id", &dev_id)) {
		DBG_ERR("can't find device-id in jpeg dtsi! \n");
		ret = -ENODEV;
		goto FAIL_FREE_RES;
	}
	pdrv_info->dev_id = dev_id;
	pdrv_info->module_info.engine_idx = dev_id;
	pdrv_info->module_info.dev_id = dev_id; //for nvt_builtinjpg_remove
	// printk("dev_id = %d \n", dev_id);
	// printk("2 pdrv_info->dev_id = %d \n", pdrv_info->dev_id);
	// printk("2 pdrv_info->module_info.engine_idx = %d \n", pdrv_info->module_info.engine_idx);

	// for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
	{
		pdrv_info->presource[dev_id] = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		if (pdrv_info->presource[dev_id] == NULL) {
			DBG_ERR("No IO memory resource defined:%d\n", dev_id);
			ret = -ENODEV;
			goto FAIL_FREE_BUF;
		}
        // printk("%d. resource:0x%llx size:0x%llx\n", dev_id, pdrv_info->presource[dev_id]->start, resource_size(pdrv_info->presource[dev_id]));
	}

	// for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
	{
		DBG_FUNC("%d. resource:0x%lx size:0x%x\n", dev_id, (unsigned long)pdrv_info->presource[dev_id]->start, (unsigned int)resource_size(pdrv_info->presource[dev_id]));
		if (!request_mem_region(pdrv_info->presource[dev_id]->start, resource_size(pdrv_info->presource[dev_id]), pdev->name)) {
			DBG_ERR("failed to request memory resource%d\n", dev_id);

			if (dev_id == 0) {
				release_mem_region(pdrv_info->presource[dev_id]->start, resource_size(pdrv_info->presource[dev_id]));
			}

			//for (; ucloop > 0 ;) {
			//	ucloop -= 1;
			//	release_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
			//}

			ret = -ENODEV;
			goto FAIL_FREE_BUF;
		}
	}
	//printk("resource start = 0x%x! \n", (unsigned int)(pdrv_info->presource[0]->start));
	//printk("resource size = 0x%x! \n", (unsigned int)(resource_size(pdrv_info->presource[0])));

	// for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
	{
		pdrv_info->module_info.io_addr[dev_id] = ioremap(pdrv_info->presource[dev_id]->start, resource_size(pdrv_info->presource[dev_id]));
		// printk("io_addr=0x%lx, resource_size=0x%x \n", (unsigned long)pdrv_info->presource[dev_id]->start, (int)resource_size(pdrv_info->presource[dev_id]));
		if (pdrv_info->module_info.io_addr[dev_id] == NULL) {
			DBG_ERR("ioremap() failed in module%d\n", dev_id);

			// if (ucloop == 0)
			{
				iounmap(pdrv_info->module_info.io_addr[dev_id]);
			}
			//for (; ucloop > 0 ;) {
			//	ucloop -= 1;
			//	iounmap(pdrv_info->module_info.io_addr[ucloop]);
			//}

			ret = -ENODEV;
			goto FAIL_FREE_RES;
		}
	}

	// for (ucloop = 0 ; ucloop < MODULE_IRQ_NUM; ucloop++)
	{
		pdrv_info->module_info.iinterrupt_id[dev_id] = platform_get_irq(pdev, 0);
		// printk("IRQ %d. ID%d\n", dev_id, pdrv_info->module_info.iinterrupt_id[dev_id]);
		if (pdrv_info->module_info.iinterrupt_id[dev_id] < 0) {
			DBG_ERR("No IRQ resource defined\n");
			ret = -ENODEV;
			goto FAIL_FREE_REMAP;
		}
	}
	// printk("IRQ %d. ID%d\n", (int)(dev_id), (int)(pdrv_info->module_info.iinterrupt_id[dev_id]));

	//Get clock source
	// for (ucloop = 0 ; ucloop < MODULE_CLK_NUM; ucloop++)
	{
		if (dev_id == 0)
			pdrv_info->module_info.mclk[dev_id] = clk_get(&pdev->dev, "jpeg1_clk");
		if (dev_id == 1)
			pdrv_info->module_info.mclk[dev_id] = clk_get(&pdev->dev, "jpeg2_clk");
		if (dev_id == 2)
			pdrv_info->module_info.mclk[dev_id] = clk_get(&pdev->dev, "jpegl_clk");

		if (IS_ERR(pdrv_info->module_info.mclk[dev_id])) {
			DBG_ERR("JPEG fail to get clock%d source\n", dev_id);
			printk("JPEG fail to get clock 0 source\n");
			ret = -ENODEV;
			goto FAIL_FREE_REMAP;
		}
	}

	//global
	if (dev_id < JPEG_MAX_ENG) {
		memset(&jpeg_eng_data[dev_id], 0, sizeof(JPEG_ENGINE_DATA));
		jpeg_eng_data[dev_id].chip_idx = 0;
		jpeg_eng_data[dev_id].engine_idx = dev_id;
		jpeg_eng_data[dev_id].engine_base_pa = (uintptr_t)pdrv_info->presource[dev_id]->start;
		jpeg_eng_data[dev_id].engine_base_va = (uintptr_t)pdrv_info->module_info.io_addr[dev_id];
		jpeg_eng_data[dev_id].active = 1;
		jpeg_eng_data[dev_id].mclk = pdrv_info->module_info.mclk[dev_id];
		printk("jpeg dev_id=%d, engine_idx=%d, io_addr(va)=0x%lx, base_addr(pa)=0x%lx, mclk=0x%lx \n",
					dev_id, jpeg_eng_data[dev_id].engine_idx, (unsigned long)jpeg_eng_data[dev_id].engine_base_va, (unsigned long)jpeg_eng_data[dev_id].engine_base_pa, (unsigned long)jpeg_eng_data[dev_id].mclk);
	}

	//Dynamic to allocate Device ID
	if (vos_alloc_chrdev_region(&pdrv_info->dev_id, MODULE_MINOR_COUNT, MODULE_NAME)) {
		DBG_ERR("Can't get device ID\n");
		ret = -ENODEV;
		goto FAIL_FREE_REMAP;
	}
	// printk("DevID Major:%d minor:%d\n", MAJOR(pdrv_info->dev_id), MINOR(pdrv_info->dev_id));

	/* Register character device for the volume */
	cdev_init(&pdrv_info->cdev, &nvt_builtinjpg_fops);
	pdrv_info->cdev.owner = THIS_MODULE;

	if (cdev_add(&pdrv_info->cdev, pdrv_info->dev_id, MODULE_MINOR_COUNT)) {
		DBG_ERR("Can't add cdev\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

	switch (dev_id) {
		case 2:
			tmp_module_name = "nvt_jpgl";
			break;
		case 1:
			tmp_module_name = "nvt_jpg2";
			break;
		case 0:
		default:
			tmp_module_name = "nvt_jpg";
			break;
	}
	// printk("tmp_module_name = %s", tmp_module_name);
	pdrv_info->pmodule_class = class_create(THIS_MODULE, tmp_module_name);
	if (IS_ERR(pdrv_info->pmodule_class)) {
		DBG_ERR("failed in creating class.\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

	/* register your own device in sysfs, and this will cause udev to create corresponding device node */
	// for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT) ; ucloop++)
	{
		pdrv_info->pdevice[dev_id] = device_create(pdrv_info->pmodule_class, NULL
									 , MKDEV(MAJOR(pdrv_info->dev_id), (dev_id + MINOR(pdrv_info->dev_id))), NULL
									 , MODULE_NAME"%d", dev_id);

		if (IS_ERR(pdrv_info->pdevice[dev_id])) {
			DBG_ERR("failed in creating device%d.\n", dev_id);

			//for (; ucloop > 0 ; ucloop--) {
			//	device_unregister(pdrv_info->pdevice[ucloop - 1]);
			//}
			// if (ucloop == 0) {
				device_unregister(pdrv_info->pdevice[dev_id]);
		    // }

			ret = -ENODEV;
			goto FAIL_CLASS;
		}
	}

	if (first_done == 0) {
		ret = nvt_builtinjpg_proc_init(pdrv_info);
		if (ret) {
			DBG_ERR("failed in creating proc.\n");
			goto FAIL_DEV;
		}
	}

	ret = nvt_jpg_drv_init(&pdrv_info->module_info);
	if (ret) {
		DBG_ERR("failed in creating proc.\n");
		goto FAIL_DRV_INIT;
	}

	platform_set_drvdata(pdev, pdrv_info);
	// printk(" @@@@@@@@@@@@@ pdev->dev address:0x%lx, 0x%lx \n", (unsigned long)&pdev->dev, (unsigned long)pdrv_info);

	first_done = 1;
	return ret;

FAIL_DRV_INIT:
	nvt_builtinjpg_proc_remove(pdrv_info);

FAIL_DEV:
	// for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT) ; ucloop++)
		device_unregister(pdrv_info->pdevice[dev_id]);

FAIL_CLASS:
	class_destroy(pdrv_info->pmodule_class);

FAIL_CDEV:
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, MODULE_MINOR_COUNT);

FAIL_FREE_REMAP:
	// for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
		iounmap(pdrv_info->module_info.io_addr[dev_id]);

FAIL_FREE_RES:
	// for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
		release_mem_region(pdrv_info->presource[dev_id]->start, resource_size(pdrv_info->presource[dev_id]));

FAIL_FREE_BUF:
	kfree(pdrv_info);
	//pdrv_info = NULL;
	//DBG_INFO("pdrv_info = 0x%lx\n", (unsigned long)pdrv_info);
	return ret;
}

static int nvt_builtinjpg_remove(struct platform_device *pdev)
{
	PJPG_DRV_INFO pdrv_info;
	// unsigned char ucloop;
	unsigned int dev_id;

	pdrv_info = platform_get_drvdata(pdev);
	dev_id = pdrv_info->module_info.dev_id;
	// DBG_WRN("nvt_builtinjpg_remove(%d), dev_id(%d), pdrv_info->dev_id(%d) \n", __LINE__, dev_id, pdrv_info->dev_id);

	nvt_jpg_drv_remove(&pdrv_info->module_info);

	nvt_builtinjpg_proc_remove(pdrv_info);

	// DBG_WRN("nvt_builtinjpg_remove ######################### %d, pdrv_info->pdevice(%lx), dev_id(%d) \n", __LINE__, (unsigned long)pdrv_info->pdevice[dev_id], dev_id);
	// for (ucloop = 0 ; ucloop < MODULE_MINOR_COUNT ; ucloop++)
		device_unregister(pdrv_info->pdevice[dev_id]);

	class_destroy(pdrv_info->pmodule_class);
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, MODULE_MINOR_COUNT);

	// for (ucloop = 0 ; ucloop < MODULE_CLK_NUM ; ucloop++)
	// 	clk_put(pdrv_info->module_info.mclk[ucloop]);

	// for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
		iounmap(pdrv_info->module_info.io_addr[dev_id]);

	// for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
		release_mem_region(pdrv_info->presource[dev_id]->start, resource_size(pdrv_info->presource[dev_id]));

	kfree(pdrv_info);
	//pdrv_info = NULL;
	//DBG_INFO("pdrv_info = 0x%lx\n", (unsigned long)pdrv_info);

	platform_set_drvdata(pdev, NULL);
	
	return 0;
}

#ifdef CONFIG_PM
static int nvt_builtinjpg_suspend(struct device *dev)
{
	PJPG_DRV_INFO pdrv_info;
	unsigned int eng_data_idx;

	if (nvt_get_suspend_mode() == 0) //0: not power off, 1: power off
		return 0;

	pdrv_info = dev_get_drvdata(dev);
	// printk(" @@@@@@@@@@@@@ pdev->dev address:0x%lx, 0x%lx \n", (unsigned long)&pdev->dev, (unsigned long)pdrv_info);
	if (pdrv_info == NULL)
		return -EINVAL;

	eng_data_idx = jpeg_lookup_engine_idx((uintptr_t)pdrv_info->module_info.io_addr[pdrv_info->module_info.engine_idx]);
	if (eng_data_idx >= JPEG_MAX_ENTITYS) {
		DBG_ERR("over max entity id(%d) \n", JPEG_MAX_ENTITYS);
		return -EINVAL;
	}

	//clock
	if (__clk_is_enabled(jpeg_eng_data[eng_data_idx].mclk))
		clk_disable_unprepare(jpeg_eng_data[eng_data_idx].mclk);
	else
		clk_unprepare(jpeg_eng_data[eng_data_idx].mclk);

	return 0;
}
static int nvt_builtinjpg_resume(struct device *dev)
{
	PJPG_DRV_INFO pdrv_info;
	unsigned int eng_data_idx;

	if (nvt_get_suspend_mode() == 0) //0: not power off, 1: power off
		return 0;

	pdrv_info = dev_get_drvdata(dev);
	if (pdrv_info == NULL)
		return -EINVAL;

	eng_data_idx = jpeg_lookup_engine_idx((uintptr_t)pdrv_info->module_info.io_addr[pdrv_info->module_info.engine_idx]);
	if (eng_data_idx >= JPEG_MAX_ENTITYS) {
		DBG_ERR("over max entity id(%d) \n", JPEG_MAX_ENTITYS);
		return -EINVAL;
	}

	//clock
	clk_prepare(pdrv_info->module_info.mclk[pdrv_info->module_info.engine_idx]);
	if (__clk_is_enabled(jpeg_eng_data[eng_data_idx].mclk))
		clk_disable(jpeg_eng_data[eng_data_idx].mclk); //enable in start_handler() or add_queue()

	// printk(" @@@@@@@@@@@@@ clk_get_rate[eng_data_idx=%d] = %ld \n", eng_data_idx, (unsigned long)clk_get_rate(jpeg_eng_data[eng_data_idx].mclk));

	return 0;
}
static const struct dev_pm_ops nvt_jpg_pm_ops = {
	.suspend = nvt_builtinjpg_suspend,
	.resume = nvt_builtinjpg_resume,
};
#else
static int nvt_builtinjpg_suspend(struct platform_device *pdev, pm_message_t state)
{
	PJPG_DRV_INFO pdrv_info;;

	DBG_INFO("jpeg builtin suspend start\n");

	pdrv_info = platform_get_drvdata(pdev);
	nvt_jpg_drv_suspend(&pdrv_info->module_info);

	DBG_INFO("jpeg builtin suspend finished\n");
	return 0;
}
static int nvt_builtinjpg_resume(struct platform_device *pdev)
{
	PJPG_DRV_INFO pdrv_info;;

	DBG_INFO("jpeg builtin resume start\n");

	pdrv_info = platform_get_drvdata(pdev);
	nvt_jpg_drv_resume(&pdrv_info->module_info);

	DBG_INFO("jpeg builtin resume finished\n");
	return 0;
}
#endif //CONFIG_PM

static struct platform_driver nvt_builtinjpg_driver/*[JPEG_MAX_ENG]*/ = {
	// {
		.driver = {
			.name = "kdrv_jpg",
			.owner = THIS_MODULE,
			.of_match_table = jpg_match_table,
#ifdef CONFIG_PM
			.pm = &nvt_jpg_pm_ops,
#endif
		},
		.probe = nvt_builtinjpg_probe,
		.remove = nvt_builtinjpg_remove,
#ifndef CONFIG_PM
		.suspend = nvt_builtinjpg_suspend,
		.resume = nvt_builtinjpg_resume
#endif
	// },
	// {
	// 	.driver = {
	// 		.name   = "jpeg_2",
	// 		.owner = THIS_MODULE,
	// 		.of_match_table = &jpg_match_table[1],
	// 	},
	// 	.probe      = nvt_builtinjpg_probe,
	// 	.remove     = nvt_builtinjpg_remove,
	// 	.suspend = nvt_builtinjpg_suspend,
	// 	.resume = nvt_builtinjpg_resume
	// },
	// {
	// 	.driver = {
	// 		.name   = "jpeg_lite",
	// 		.owner = THIS_MODULE,
	// 		.of_match_table = &jpg_match_table[2],
	// 	},
	// 	.probe      = nvt_builtinjpg_probe,
	// 	.remove     = nvt_builtinjpg_remove,
	// 	.suspend = nvt_builtinjpg_suspend,
	// 	.resume = nvt_builtinjpg_resume
	// },
};

int __init nvt_builtinjpg_module_init(void)
{
	int ret;

	printk("\n ============== nvt_jpg version: %s ============== \n", DRV_JPG_VERSION);
	ret = platform_driver_register(&nvt_builtinjpg_driver);
	// ret = platform_driver_register(&nvt_builtinjpg_driver[1]);
	// ret = platform_driver_register(&nvt_builtinjpg_driver[2]);

	// jpeg_kdrv_init();
	if (first_done == 1)
		ret = jpeg_engine_init();
	else
		printk("jpeg first_done not yet \n");

	return 0;
}

void __exit nvt_builtinjpg_module_exit(void)
{
	// jpeg_kdrv_close();
	jpeg_engine_exit();

	platform_driver_unregister(&nvt_builtinjpg_driver);
	// platform_driver_unregister(&nvt_builtinjpg_driver[1]);
	// platform_driver_unregister(&nvt_builtinjpg_driver[2]);
}

//module_init(nvt_builtinjpg_module_init);
//module_exit(nvt_builtinjpg_module_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("jpg driver");
MODULE_LICENSE("GPL");

EXPORT_SYMBOL(jpeg_debug_level);
//MODULE_VERSION(JPEG_VER_STR);
#endif
