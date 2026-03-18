#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/kdev_t.h>
#include <linux/clk.h>
#include <asm/signal.h>
#include <kwrap/dev.h>
//#include "uitron_wrapper/type.h"
#include "ai_drv.h"
#include "ai_main.h"
#if defined(__FREERTOS)
#include "kwrap/debug.h"
#else
//#include "kdrv_ai_dbg.h"
#include "kwrap/debug.h"
#endif
#include "kdrv_ai.h"
#include "ai_proc.h"
#include "../../../include/kdrv_ai_version.h"
//#include <mach/ftpmu010.h>

//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================
#ifdef DEBUG
unsigned int ai_debug_level = (NVT_DBG_IND | NVT_DBG_WRN | NVT_DBG_ERR);
module_param_named(ai_debug_level, ai_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(ai_debug_level, "Debug message level");
#endif

unsigned int ai_clk_gating[MODULE_CLK_NUM] = {0};
unsigned int ai_get_clk[MODULE_CLK_NUM] = {0};
struct clk* ai_clk[MODULE_CLK_NUM];
UINT32 ai_freq_from_dtsi[MODULE_CLK_NUM];
uintptr_t ai_eng_io_addr[MODULE_REG_NUM] = {0};
//=============================================================================
// Global variable
//=============================================================================
static struct of_device_id ai_match_table[] = {
	{   .compatible = "nvt,kdrv_ai"},
	{}
};


//=============================================================================
// function declaration
//=============================================================================
static int nvt_ai_open(struct inode *inode, struct file *file);
static int nvt_ai_release(struct inode *inode, struct file *file);
static long nvt_ai_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static int nvt_ai_probe(struct platform_device *pdev);
static int nvt_ai_suspend(struct platform_device *pdev, pm_message_t state);
static int nvt_ai_resume(struct platform_device *pdev);
static int nvt_ai_remove(struct platform_device *pdev);
int __init nvt_ai_module_init(void);
void __exit nvt_ai_module_exit(void);

//=============================================================================
// function define
//=============================================================================
static int nvt_ai_open(struct inode *inode, struct file *file)
{
	AI_DRV_INFO *pdrv_info;

	if ((inode == NULL) || (file == NULL)) {
		nvt_dbg(ERR, "invalid input\n");
		return -EINVAL;
	}

	pdrv_info = container_of(vos_icdev(inode), AI_DRV_INFO, cdev);
	file->private_data = pdrv_info;

	if (pdrv_info == NULL) {
		nvt_dbg(ERR, "invalid driver input\n");
		return -EINVAL;
	}

	if (nvt_ai_drv_open(&pdrv_info->module_info, MINOR(vos_irdev(inode)))) {
		nvt_dbg(ERR, "failed to open driver\n");
		return -1;
	}

	return 0;
}

static int nvt_ai_release(struct inode *inode, struct file *file)
{
	AI_DRV_INFO *pdrv_info;
	if ((inode == NULL) || (file == NULL)) {
		nvt_dbg(ERR, "invalid input\n");
		return -EINVAL;
	}

	pdrv_info = container_of(vos_icdev(inode), AI_DRV_INFO, cdev);
	if (pdrv_info == NULL) {
		nvt_dbg(ERR, "invalid driver input\n");
		return -EINVAL;
	}

	nvt_ai_drv_release(&pdrv_info->module_info, MINOR(vos_irdev(inode)));
	return 0;
}

static long nvt_ai_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct inode *inode;
	PAI_DRV_INFO pdrv;

	if (filp == NULL) {
		nvt_dbg(ERR, "invalid input\n");
		return -EINVAL;
	}

	inode = file_inode(filp);
	if (inode == NULL) {
		nvt_dbg(ERR, "invalid file_inode\n");
		return -EINVAL;
	}


	pdrv = filp->private_data;

	return nvt_ai_drv_ioctl(MINOR(vos_irdev(inode)), &pdrv->module_info, cmd, arg);
}

struct file_operations nvt_ai_fops = {
	.owner   = THIS_MODULE,
	.open    = nvt_ai_open,
	.release = nvt_ai_release,
	.unlocked_ioctl = nvt_ai_ioctl,
	.llseek  = no_llseek,
};


unsigned int kdrv_ai_get_gating(int engine_index)
{
	return ai_clk_gating[engine_index];
}

void kdrv_ai_set_gating(int flag,int engine_index)
{
	if(ai_get_clk[engine_index]){
		if (ai_clk_gating[engine_index] != flag) {
			ai_clk_gating[engine_index] = flag;

			if (ai_clk_gating[engine_index] == 1) {
				clk_set_phase(ai_clk[engine_index], 1);
				nvt_dbg(IND, "%s_%d, clk_set_phase to 1.\r\n", __FUNCTION__, __LINE__);
			} else {
				clk_set_phase(ai_clk[engine_index], 0);
				nvt_dbg(IND, "%s_%d, clk_set_phase to 0.\r\n", __FUNCTION__, __LINE__);
			}
			/*if(engine_index==2){
				if (ai_clk_gating[engine_index] == 1) {
					clk_set_phase(ai_clk[3], 1);
				} else {
					clk_set_phase(ai_clk[3], 0);
				}
			}*/
		}
	}else{
		nvt_dbg(ERR, "[kdrv_ai_set_gating] Did not get clock source.\n");
	}
}
static int nvt_ai_probe(struct platform_device *pdev)
{
	AI_DRV_INFO *pdrv_info;
	const struct of_device_id *match;
	int ret = 0;
	int ucloop;
	uintptr_t tcm_va;
	unsigned long tcm_value = 0x00000010;
	
	if (pdev == NULL) {
		nvt_dbg(ERR, "invalid input\n");
		return -EINVAL;
	}
	//nvt_dbg(IND, "kdrv %s\n", pdev->name);

	match = of_match_device(ai_match_table, &pdev->dev);
	if (!match) {
		nvt_dbg(ERR, "Platform device not found \n");
		return -EINVAL;
	}

	pdrv_info = kzalloc(sizeof(AI_DRV_INFO), GFP_KERNEL);
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
		//nvt_dbg(IND, "%d. resource:0x%x size:0x%x\n", ucloop, pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
		if (!request_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]), pdev->name)) {
			nvt_dbg(ERR, "failed to request memory resource%d\n", ucloop);
			while (ucloop >= 0) {
				release_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
				ucloop--;
			}
			ret = -ENODEV;
			goto FAIL_FREE_BUF;
		}
	}
	
	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		pdrv_info->module_info.io_addr[ucloop] = ioremap(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
		if (pdrv_info->module_info.io_addr[ucloop] == NULL) {
			nvt_dbg(ERR, "ioremap() failed in module%d\n", ucloop);
			while (ucloop >= 0) {
				iounmap(pdrv_info->module_info.io_addr[ucloop]);
				ucloop--;
			}
			ret = -ENODEV;
			goto FAIL_FREE_RES;
		}
	}
	
	tcm_va = (uintptr_t)ioremap(0x2f07f0040, 4);
	iowrite32(tcm_value, (void*)(tcm_va));
	iounmap((void*)tcm_va);

	for (ucloop = 0 ; ucloop < MODULE_IRQ_NUM; ucloop++) {
		pdrv_info->module_info.iinterrupt_id[ucloop] = platform_get_irq(pdev, ucloop);
		//nvt_dbg(IND, "IRQ %d. ID%d\n", ucloop, pdrv_info->module_info.iinterrupt_id[ucloop]);
		if (pdrv_info->module_info.iinterrupt_id[ucloop] < 0) {
			nvt_dbg(ERR, "No IRQ resource defined\n");
			ret = -ENODEV;
			goto FAIL_FREE_REMAP;
		}
	}

	//Get clock source
	for (ucloop = 0 ; ucloop < MODULE_CLK_NUM; ucloop++) {
		char pstr1[64];
		if (nvt_get_chip_id() == CHIP_NS02302 && ucloop==(MODULE_CLK_NUM-4)) {
			break;
		}
		//pdrv_info->module_info.pclk[ucloop] = clk_get(&pdev->dev, dev_name(&pdev->dev));
		if(ucloop<8){
			snprintf(pstr1, 16, "2%08x.ai", (UINT32)pdrv_info->presource[ucloop]->start);
		}else{
			// get additional conv auto gating
			snprintf(pstr1, 16, "2%08x.ai_c%d", (UINT32)pdrv_info->presource[0]->start, ucloop - 7);
		}
		//DBG_IND("resource-start: %s, %s\r\n", pstr1, dev_name(&pdev->dev));
		ai_clk[ucloop] = clk_get(&pdev->dev, pstr1/*dev_name(&pdev->dev)*/);
		if (IS_ERR(ai_clk[ucloop])) {
			nvt_dbg(ERR, "faile to get clock%d source\n", ucloop);

			ret = -ENODEV;
			goto FAIL_FREE_REMAP;
		}		
		//clk_prepare_enable(ai_clk[ucloop]);
		ai_get_clk[ucloop] = 1;
	}
	//nvt_dbg(IND, " get clock source\r\n");

	if (nvt_get_chip_id() == CHIP_NS02302) {
	if (of_property_read_u32_array(pdev->dev.of_node, "clock-frequency", ai_freq_from_dtsi, MODULE_CLK_NUM-4)) {
        nvt_dbg(IND, "%s_%d, no dtsi input for clock-frequency.\r\n", __FUNCTION__, __LINE__);
	}
	} else if (nvt_get_chip_id() == CHIP_NS02402) {
	if (of_property_read_u32_array(pdev->dev.of_node, "clock-frequency", ai_freq_from_dtsi, MODULE_CLK_NUM)) {
        nvt_dbg(IND, "%s_%d, no dtsi input for clock-frequency.\r\n", __FUNCTION__, __LINE__);
	}
	}
	
	//printk("clk %d %d %d %d\n", (unsigned int) ai_freq_from_dtsi[0], (unsigned int) ai_freq_from_dtsi[1], 
	//							(unsigned int) ai_freq_from_dtsi[2], (unsigned int) ai_freq_from_dtsi[3]);


	//Dynamic to allocate Device ID
	if (vos_alloc_chrdev_region(&pdrv_info->dev_id, MODULE_MINOR_COUNT, MODULE_NAME)) {
		nvt_dbg(ERR, "Can't get device ID\n");
		ret = -ENODEV;
		goto FAIL_FREE_REMAP;
	}

	//nvt_dbg(IND, "DevID Major:%d minor:%d\n", MAJOR(pdrv_info->dev_id), MINOR(pdrv_info->dev_id));

	/* Register character device for the volume */
	cdev_init(&pdrv_info->cdev, &nvt_ai_fops);
	pdrv_info->cdev.owner = THIS_MODULE;

	if (cdev_add(&pdrv_info->cdev, pdrv_info->dev_id, MODULE_MINOR_COUNT)) {
		nvt_dbg(ERR, "Can't add cdev\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

	pdrv_info->pmodule_class = class_create(THIS_MODULE, "kdrv_ai");

	if (IS_ERR(pdrv_info->pmodule_class)) {
		nvt_dbg(ERR, "failed in creating class.\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

	/* register your own device in sysfs, and this will cause udev to create corresponding device node */
	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT) ; ucloop++) {
		pdrv_info->pdevice[ucloop] = device_create(pdrv_info->pmodule_class, NULL
									 , MKDEV(MAJOR(pdrv_info->dev_id), (ucloop + MINOR(pdrv_info->dev_id))), NULL
									 , MODULE_NAME"%d", ucloop);

		if (IS_ERR(pdrv_info->pdevice[ucloop])) {
			nvt_dbg(ERR, "failed in creating device%d.\n", ucloop);

			while (ucloop >= 0) {
				device_unregister(pdrv_info->pdevice[ucloop]);
				ucloop--;
			}
			ret = -ENODEV;
			goto FAIL_CLASS;
		}
	}

	ret = nvt_ai_proc_init(pdrv_info);
	if (ret) {
		nvt_dbg(ERR, "failed in creating proc.\n");
		goto FAIL_DEV;
	}

	ret = nvt_ai_drv_init(&pdrv_info->module_info);

	platform_set_drvdata(pdev, pdrv_info);
	if (ret) {
		nvt_dbg(ERR, "failed in creating proc.\n");
		goto FAIL_DRV_INIT;
	}

	return ret;


FAIL_DRV_INIT:
	nvt_ai_proc_remove(pdrv_info);

FAIL_DEV:
	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT) ; ucloop++) {
		device_unregister(pdrv_info->pdevice[ucloop]);
	}

FAIL_CLASS:
	class_destroy(pdrv_info->pmodule_class);

FAIL_CDEV:
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, MODULE_MINOR_COUNT);

FAIL_FREE_REMAP:
	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		iounmap(pdrv_info->module_info.io_addr[ucloop]);
	}

FAIL_FREE_RES:
	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		release_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
	}
	
FAIL_FREE_BUF:
	kfree(pdrv_info);
	return ret;
}

static int nvt_ai_remove(struct platform_device *pdev)
{
	PAI_DRV_INFO pdrv_info;
	unsigned char ucloop;
	if (pdev == NULL) {
		nvt_dbg(ERR, "invalid input\n");
		return -EINVAL;
	}

	nvt_dbg(IND, "\n");

	pdrv_info = platform_get_drvdata(pdev);

	nvt_ai_drv_remove(&pdrv_info->module_info);

	nvt_ai_proc_remove(pdrv_info);

	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT) ; ucloop++) {
		device_unregister(pdrv_info->pdevice[ucloop]);
	}

	class_destroy(pdrv_info->pmodule_class);
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, MODULE_MINOR_COUNT);

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		iounmap(pdrv_info->module_info.io_addr[ucloop]);
	}

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		release_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
	}
	
#if 0
	ftpmu010_deregister_reg(ai_fd);
#endif

	kfree(pdrv_info);
	return 0;
}

static int nvt_ai_suspend(struct platform_device *pdev, pm_message_t state)
{
	PAI_DRV_INFO pdrv_info;

	if (pdev == NULL) {
		nvt_dbg(ERR, "invalid input\n");
		return -EINVAL;
	}
	nvt_dbg(IND, "start\n");

	pdrv_info = platform_get_drvdata(pdev);
	nvt_ai_drv_suspend(&pdrv_info->module_info);

	nvt_dbg(IND, "finished\n");
	return 0;
}


static int nvt_ai_resume(struct platform_device *pdev)
{
	PAI_DRV_INFO pdrv_info;;

	if (pdev == NULL) {
		nvt_dbg(ERR, "invalid input\n");
		return -EINVAL;
	}
	nvt_dbg(IND, "start\n");

	pdrv_info = platform_get_drvdata(pdev);
	nvt_ai_drv_resume(&pdrv_info->module_info);

	nvt_dbg(IND, "finished\n");
	return 0;
}


static struct platform_driver nvt_ai_driver = {
	.driver = {
		.name   = "kdrv_ai",
		.owner = THIS_MODULE,
		.of_match_table = ai_match_table,
	},
	.probe      = nvt_ai_probe,
	.remove     = nvt_ai_remove,
	.suspend = nvt_ai_suspend,
	.resume = nvt_ai_resume
};

int __init nvt_ai_module_init(void)
{
	int ret;
	
	ret = platform_driver_register(&nvt_ai_driver);
	return 0;
}

void __exit nvt_ai_module_exit(void)
{
	platform_driver_unregister(&nvt_ai_driver);
}

module_init(nvt_ai_module_init);
module_exit(nvt_ai_module_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("AI driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(KDRV_AI_IMPL_VERSION);
