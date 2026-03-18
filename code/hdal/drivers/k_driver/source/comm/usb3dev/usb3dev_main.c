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
//#include <kwrap/dev.h>

#include "usb3dev_drv.h"
//#include "usb2dev_reg.h"
#include "usb3dev_main.h"
#include "usb3dev_proc.h"
#include "usb3dev_dbg.h"
#include <linux/dma-mapping.h>

//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================
#ifdef DEBUG
unsigned int usb3dev_debug_level = NVT_DBG_WRN;//NVT_DBG_WRN;
module_param_named(usb3dev_debug_level, usb3dev_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(usb3dev_debug_level, "Debug message level");
#endif

//=============================================================================
// Global variable
//=============================================================================
static struct of_device_id usb3dev_match_table[] = {
	{   .compatible = "nvt,nvt_dwc3_dev"},
	{}
};


static u32  u2_get_tx_swing;
char u2_tx_swing;
static u32  get_tx_swing;
char tx_swing;

static u32  u3_get_channel;
char u3_channel;

static u32  vbus_get_polling;
char vbus_poll;

//=============================================================================
// function declaration
//=============================================================================
static int nvt_usb3dev_open(struct inode *inode, struct file *file);
static int nvt_usb3dev_release(struct inode *inode, struct file *file);
static long nvt_usb3dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static int nvt_usb3dev_probe(struct platform_device *pdev);
static int nvt_usb3dev_suspend(struct platform_device *pdev, pm_message_t state);
static int nvt_usb3dev_resume(struct platform_device *pdev);
static int nvt_usb3dev_remove(struct platform_device *pdev);
int __init nvt_usb3dev_module_init(void);
void __exit nvt_usb3dev_module_exit(void);

//=============================================================================
// function define
//=============================================================================
static int nvt_usb3dev_open(struct inode *inode, struct file *file)
{
	USB2DEV_DRV_INFO *pdrv_info;

	usb3dev_api("%s\n", __func__);

	pdrv_info = container_of(inode->i_cdev, USB2DEV_DRV_INFO, cdev);

	pdrv_info = container_of(inode->i_cdev, USB2DEV_DRV_INFO, cdev);
	file->private_data = pdrv_info;

	if (nvt_usb3dev_drv_open(&pdrv_info->module_info, MINOR(inode->i_rdev))) {
		nvt_dbg(ERR, "failed to open driver\n");
		return -1;
	}

	return 0;
}

static int nvt_usb3dev_release(struct inode *inode, struct file *file)
{
	USB2DEV_DRV_INFO *pdrv_info;

	usb3dev_api("%s\n", __func__);

	pdrv_info = container_of(inode->i_cdev, USB2DEV_DRV_INFO, cdev);

	nvt_usb3dev_drv_release(&pdrv_info->module_info, MINOR(inode->i_rdev));
	return 0;
}

static long nvt_usb3dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct inode *inode;
	PUSB2DEV_DRV_INFO pdrv;

	//usb3dev_api("%s\n", __func__);

	inode = file_inode(filp);
	pdrv = filp->private_data;

	return nvt_usb3dev_drv_ioctl(MINOR(inode->i_rdev), &pdrv->module_info, cmd, arg);
}

struct file_operations nvt_usb3dev_fops = {
	.owner   = THIS_MODULE,
	.open    = nvt_usb3dev_open,
	.release = nvt_usb3dev_release,
	.unlocked_ioctl = nvt_usb3dev_ioctl,
	.llseek  = no_llseek,
};

static int nvt_usb3dev_probe(struct platform_device *pdev)
{
	USB2DEV_DRV_INFO *pdrv_info;//info;
	const struct of_device_id *match;
	int ret = 0;
	unsigned char ucloop;
	u32 vbus_gpio_pin[2];

	nvt_dbg(IND, "%s\n", pdev->name);

	usb3dev_api("%s\n", __func__);

	match = of_match_device(usb3dev_match_table, &pdev->dev);
	if (!match) {
		printk("Platform device not found \n");
		return -EINVAL;
	}

	pdrv_info = kzalloc(sizeof(USB2DEV_DRV_INFO), GFP_KERNEL);
	if (!pdrv_info) {
		printk("failed to allocate memory\n");
		return -ENOMEM;
	}

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		pdrv_info->presource[ucloop] = platform_get_resource(pdev, IORESOURCE_MEM, ucloop);
		if (pdrv_info->presource[ucloop] == NULL) {
			printk("No IO memory resource defined:%d\n", ucloop);
			ret = -ENODEV;
			goto FAIL_FREE_BUF;
		}
	}

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		nvt_dbg(IND, "%d. resource:0x%lx size:0x%lx\n", ucloop, (uintptr_t)pdrv_info->presource[ucloop]->start, (uintptr_t)resource_size(pdrv_info->presource[ucloop]));
	}

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		pdrv_info->module_info.io_addr[ucloop] = ioremap(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
		usb3dev_api("start 0x%08lX  mapped to 0x%08lX\n", (uintptr_t)pdrv_info->presource[ucloop]->start, (uintptr_t)pdrv_info->module_info.io_addr[ucloop]);
		if (pdrv_info->module_info.io_addr[ucloop] == NULL) {
			printk("ioremap() failed in module%d\n", ucloop);
			ret = -ENODEV;
			goto FAIL_FREE_RES;
		}
	}

	for (ucloop = 0 ; ucloop < MODULE_IRQ_NUM; ucloop++) {
		pdrv_info->module_info.iinterrupt_id[ucloop] = platform_get_irq(pdev, ucloop);
		usb3dev_api("IRQ %d. ID%d\n", ucloop, pdrv_info->module_info.iinterrupt_id[ucloop]);
		if (pdrv_info->module_info.iinterrupt_id[ucloop] < 0) {
			nvt_dbg(ERR, "No IRQ resource defined\n");
			ret = -ENODEV;
			goto FAIL_FREE_REMAP;
		}
	}

	//Dynamic to allocate Device ID
	if (alloc_chrdev_region(&pdrv_info->dev_id, MODULE_MINOR_ID, MODULE_MINOR_COUNT, MODULE_NAME)) {
		nvt_dbg(ERR, "Can't get device ID\n");
		ret = -ENODEV;
		goto FAIL_FREE_REMAP;
	}


	nvt_dbg(IND, "DevID Major:%d minor:%d\n" \
			, MAJOR(pdrv_info->dev_id), MINOR(pdrv_info->dev_id));

	/* Register character device for the volume */
	cdev_init(&pdrv_info->cdev, &nvt_usb3dev_fops);
	pdrv_info->cdev.owner = THIS_MODULE;

	if (cdev_add(&pdrv_info->cdev, pdrv_info->dev_id, MODULE_MINOR_COUNT)) {
		printk("Can't add cdev\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

	pdrv_info->pmodule_class = class_create(THIS_MODULE, MODULE_NAME);
	if (IS_ERR(pdrv_info->pmodule_class)) {
		printk("failed in creating class.\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

	/* register your own device in sysfs, and this will cause udev to create corresponding device node */
	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT) ; ucloop++) {
		pdrv_info->pdevice[ucloop] = device_create(pdrv_info->pmodule_class, NULL
									 , MKDEV(MAJOR(pdrv_info->dev_id), (ucloop + MINOR(pdrv_info->dev_id))), NULL
									 , MODULE_NAME"%d", ucloop);

		if (IS_ERR(pdrv_info->pdevice[ucloop])) {
			printk("failed in creating device%d.\n", ucloop);
			ret = -ENODEV;
			goto FAIL_CLASS;
		}
	}

	if (of_property_read_u32_array(pdev->dev.of_node, "vbus_gpio", vbus_gpio_pin, 2)) {
		pdrv_info->module_info.vbus_gpio_pin = -1;
		pdrv_info->module_info.vbus_gpio_pol = -1;
	} else {
		pdrv_info->module_info.vbus_gpio_pin = vbus_gpio_pin[0];
		pdrv_info->module_info.vbus_gpio_pol = vbus_gpio_pin[1];
	}

	if (of_property_read_u32(pdev->dev.of_node, "u2_tx_swing", &u2_get_tx_swing)) {
		u2_tx_swing = -1;
	} else {
		u2_tx_swing = (char)(u2_get_tx_swing & 0xFF);
	}

	if (of_property_read_u32(pdev->dev.of_node, "u3_tx_swing", &get_tx_swing)) {
		tx_swing = -1;
	} else {
		tx_swing = (char)(get_tx_swing & 0xFF);
	}

	if (of_property_read_u32(pdev->dev.of_node, "channel", &u3_get_channel)) {
		u3_channel = 0;
	} else {
		if (u3_get_channel) {
			u3_channel = 1;
		} else {
			u3_channel = 0;
		}
	}

	if (of_property_read_u32(pdev->dev.of_node, "vbus_polling", &vbus_get_polling)) {
		vbus_poll = 0;
	} else {
		printk("Open vbus polling\n");
		vbus_poll = 1;
	}

	ret = nvt_usb3dev_proc_init(pdrv_info);
	if (ret) {
		printk("failed in creating proc.\n");
		goto FAIL_DEV;
	}

#if 0
	{
		dma_addr_t		dma=0;
		unsigned char   *va;


	    if (dma_set_coherent_mask(pdrv_info->pdevice[0], DMA_BIT_MASK(32)))
	    {
	        printk("Bad mask.\n");
	    }

		va = dma_alloc_coherent(pdrv_info->pdevice[0], 0x10000, &dma, GFP_KERNEL);
		printk("TESTTEST. va=0x%08X pa = 0x%08X\n", (u32)va, (u32)dma);
		va[0] = 0xAA;
		va[1] = 0xAB;
		va[2] = 0x55;
		va[3] = 0x77;
	}
#endif

	ret = nvt_usb3dev_drv_init(&pdrv_info->module_info, pdrv_info->pdevice[0]);

	platform_set_drvdata(pdev, pdrv_info);
	if (ret) {
		printk("failed in creating proc.\n");
		goto FAIL_DRV_INIT;
	}

	return ret;

FAIL_DRV_INIT:
	nvt_usb3dev_proc_remove(pdrv_info);

FAIL_DEV:
	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT) ; ucloop++) {
		device_unregister(pdrv_info->pdevice[ucloop]);
	}

FAIL_CLASS:
	class_destroy(pdrv_info->pmodule_class);

FAIL_CDEV:
	cdev_del(&pdrv_info->cdev);
	unregister_chrdev_region(pdrv_info->dev_id, MODULE_MINOR_COUNT);

FAIL_FREE_REMAP:
	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		iounmap(pdrv_info->module_info.io_addr[ucloop]);
	}

FAIL_FREE_RES:
FAIL_FREE_BUF:
	kfree(pdrv_info);
	//coverity[assigned_pointer]
	pdrv_info = NULL;
	return ret;
}

static int nvt_usb3dev_remove(struct platform_device *pdev)
{
	PUSB2DEV_DRV_INFO pdrv_info;
	unsigned char ucloop;

	nvt_dbg(IND, "\n");

	usb3dev_api("%s\n", __func__);

	pdrv_info = platform_get_drvdata(pdev);

	nvt_usb3dev_drv_remove(&pdrv_info->module_info, pdrv_info->pdevice[0]);

	nvt_usb3dev_proc_remove(pdrv_info);

	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT) ; ucloop++) {
		device_unregister(pdrv_info->pdevice[ucloop]);
	}

	class_destroy(pdrv_info->pmodule_class);
	cdev_del(&pdrv_info->cdev);
	unregister_chrdev_region(pdrv_info->dev_id, MODULE_MINOR_COUNT);

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		iounmap(pdrv_info->module_info.io_addr[ucloop]);
	}

	kfree(pdrv_info);
	//coverity[assigned_pointer]
	pdrv_info = NULL;
	return 0;
}

#ifdef CONFIG_PM
static int nvt_usb3dev_suspend(struct platform_device *pdev, pm_message_t state)
{
	PUSB2DEV_DRV_INFO pdrv_info;
	//nvt_dbg(IND, "start\n");
	usb3dev_api("%s\n", __func__);

	pdrv_info = platform_get_drvdata(pdev);
	nvt_usb3dev_drv_suspend(&pdrv_info->module_info);
	//nvt_dbg(IND, "finished\n");
	return 0;
}


static int nvt_usb3dev_resume(struct platform_device *pdev)
{
	PUSB2DEV_DRV_INFO pdrv_info;
	//nvt_dbg(IND, "start\n");
	usb3dev_api("%s\n", __func__);
	pdrv_info = platform_get_drvdata(pdev);
	nvt_usb3dev_drv_resume(&pdrv_info->module_info);
	//nvt_dbg(IND, "finished\n");
	return 0;
}
#else
static int nvt_usb3dev_suspend(struct platform_device *pdev, pm_message_t state)
{
	//PUSB2DEV_DRV_INFO pdrv_info;
	//nvt_dbg(IND, "start\n");
	usb3dev_api("%s\n", __func__);
	//pdrv_info = platform_get_drvdata(pdev);
	//nvt_usb3dev_drv_suspend(&pdrv_info->module_info);
	//nvt_dbg(IND, "finished\n");
	return 0;
}


static int nvt_usb3dev_resume(struct platform_device *pdev)
{
	//PUSB2DEV_DRV_INFO pdrv_info;
	//nvt_dbg(IND, "start\n");
	usb3dev_api("%s\n", __func__);
	//pdrv_info = platform_get_drvdata(pdev);
	//nvt_usb3dev_drv_resume(&pdrv_info->module_info);
	//nvt_dbg(IND, "finished\n");
	return 0;
}
#endif

static struct platform_driver nvt_usb3dev_driver = {
	.driver = {
		.name   = "nvt_usb3dev",
		.owner = THIS_MODULE,
		.of_match_table = usb3dev_match_table,
	},
	.probe      = nvt_usb3dev_probe,
	.remove     = nvt_usb3dev_remove,
	.suspend = nvt_usb3dev_suspend,
	.resume = nvt_usb3dev_resume
};

int __init nvt_usb3dev_module_init(void)
{
	int ret;

	nvt_dbg(IND, "\n");

	usb3dev_api("%s\n", __func__);

	ret = platform_driver_register(&nvt_usb3dev_driver);

	return 0;
}

void __exit nvt_usb3dev_module_exit(void)
{
	nvt_dbg(IND, "\n");

	usb3dev_api("%s\n", __func__);

	platform_driver_unregister(&nvt_usb3dev_driver);

}

module_init(nvt_usb3dev_module_init);
module_exit(nvt_usb3dev_module_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("usb3dev driver");
MODULE_LICENSE("GPL");
