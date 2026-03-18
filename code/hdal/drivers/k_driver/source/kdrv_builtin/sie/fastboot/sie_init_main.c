#if defined(__KERNEL__)
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
#include <linux/soc/nvt/nvtmem.h>
#include <asm/signal.h>
#else
#include "io_address.h"
#include "interrupt.h"
#endif
#include <kwrap/dev.h>
#include "sie_init_main.h"
#include "sie_init_int.h"
#include "sie_eng.h"

#define SIE_RW_MODE SIE_REG_RW_MODE_DIRECT

#if 0
#endif
//=============================================================================
// function define
//=============================================================================
static SIE_BUILTIN_RESOURCE *p_sie_builtin_resource = NULL;

#if defined(_GROUP_KO_)
#undef __init
#undef __exit
#undef module_init
#undef module_exit
#define __init
#define __exit
#define module_init(x)
#define module_exit(x)
#endif

void *sie_builtin_os_malloc_wrap(UINT32 want_size)
{
	void *p_buf = NULL;

#if defined(__LINUX)
	p_buf = kmalloc(want_size, GFP_KERNEL);
#elif defined(__FREERTOS)
	p_buf = malloc(want_size);
#endif

	return p_buf;
}

void sie_builtin_os_mfree_wrap(void *p_buf)
{
#if defined(__LINUX)
	kfree(p_buf);
#elif defined(__FREERTOS)
	free(p_buf);
#endif
}

INT32 sie_builtin_resource_init(void)
{
	if (p_sie_builtin_resource == NULL) {
		if ((p_sie_builtin_resource = sie_builtin_os_malloc_wrap(sizeof(SIE_BUILTIN_RESOURCE) * SIE_BUILTIN_TOTAL_CH_COUNT)) == NULL) {
			DBG_ERR("allcate fail\r\n");
			return -1;
		}
	}
	return 0;
}

INT32 sie_builtin_resource_uninit(void)
{
	if (p_sie_builtin_resource) {
		sie_builtin_os_mfree_wrap(p_sie_builtin_resource);
		p_sie_builtin_resource = NULL;
	}
	return 0;
}

INT32 sie_builtin_resource_set(SIE_BUILTIN_RESOURCE *p_resource, UINT32 hdl_idx)
{
	if (hdl_idx >= SIE_BUILTIN_TOTAL_CH_COUNT) {
		DBG_ERR("idx %d overflow(chip: %d; eng: %d, minor %d)\r\n", hdl_idx, SIE_BUILTIN_CHIP_COUNT, SIE_BUILTIN_ENG_COUNT, SIE_BUILTIN_MINOR_COUNT);
		return -1;
	}

	if (p_sie_builtin_resource == NULL) {
		DBG_ERR("resource null\r\n");
		return -1;
	}

	memcpy((void *)&p_sie_builtin_resource[hdl_idx], p_resource, sizeof(SIE_BUILTIN_RESOURCE));
	return 0;
}

SIE_BUILTIN_RESOURCE *sie_builtin_resource_get(UINT32 hdl_idx)
{
	if (hdl_idx >= SIE_BUILTIN_TOTAL_CH_COUNT) {
		DBG_ERR("idx %d overflow(chip: %d; eng: %d, minor %d)\r\n", hdl_idx, SIE_BUILTIN_CHIP_COUNT, SIE_BUILTIN_ENG_COUNT, SIE_BUILTIN_MINOR_COUNT);
		return NULL;
	}

	if (p_sie_builtin_resource == NULL) {
		DBG_ERR("hdl_idx %d eng_idx %d resource null\r\n", hdl_idx, SIE_BUILTIN_CONV2_ENG_IDX(hdl_idx));
		return NULL;
	}

	return &p_sie_builtin_resource[SIE_BUILTIN_CONV2_ENG_IDX(hdl_idx)];
}

INT32 sie_builtin_module_init(void)
{
	SIE_ENG_INIT_PARAM eng_param;
	SIE_BUILTIN_RESOURCE *p_resource = NULL;
	UINT32 i;
	UINT32 module_clk_tbl[SIE_MAX_ENG_NUM] = {
		SIE1_MAX_CLK_FREQ,
		SIE2_MAX_CLK_FREQ,
		SIE3_MAX_CLK_FREQ,
		SIE4_MAX_CLK_FREQ,
		SIE5_MAX_CLK_FREQ,
	};

	/* ssdrv - initialize */
	sie_eng_init(SIE_BUILTIN_CHIP_COUNT, SIE_BUILTIN_ENG_COUNT);

	for (i = 0; i < SIE_BUILTIN_ENG_COUNT; i++) {
		/* get resource */
		if ((p_resource = sie_builtin_resource_get(i)) == NULL) {
			DBG_ERR("null resource, idx %d\r\n", i);
			return -1;
		}
		memset((void *)&eng_param, 0, sizeof(SIE_ENG_INIT_PARAM));

		/* name */
		snprintf(eng_param.name, sizeof(eng_param.name), "sie_eng%d", (int)i);

		/* idx */
		eng_param.chip_id = KDRV_CHIP0;
		eng_param.eng_id  = i%SIE_BUILTIN_ENG_COUNT;   ///< ssdrv eng_id from 0 to SIE_ENG_COUNT

		/* system */
		eng_param.reg_rw_mode  = SIE_RW_MODE;
		eng_param.reg_io_base  = (ULONG)p_resource->p_io_addr;
		if (p_resource->reg_addr_va != NULL) {
			eng_param.p_sie_reg_st = (ULONG)p_resource->reg_addr_va;
		} else {
			eng_param.p_sie_reg_st = 0;
		}

		eng_param.irq_id = p_resource->irq_id;
		eng_param.isr_cb = NULL;

		/* clk */
#if defined(__FREERTOS)
		eng_param.clock_src = p_resource->clk_src;  ///< keep cur setting
#else
		eng_param.clock_src = p_resource->clk_src+1;  ///< keep setting if unknown clock source
#endif
		if (p_resource->clk_rate == 0) {
			eng_param.clock_rate = module_clk_tbl[i%SIE_MAX_ENG_NUM];
		} else {
			eng_param.clock_rate = p_resource->clk_rate;
		}
		eng_param.sie_clk = p_resource->clk;
//		eng_param.sie_bccclk = SIE_BCC_MAX_CLK_FREQ; //p_resource->bccclk, ToDo, for SIE1~12 bcc clock source
		eng_param.sie_pxclk = p_resource->pxclk;
		eng_param.sie_intclk = p_resource->intclk;
//		eng_param.pclk = p_resource->pclk; // program clock ?
		eng_param.sie_pxclkpad = p_resource->pxclk_pad[i];
		eng_param.sie_tsen_rxclk = p_resource->tsenrxclk[i];
		eng_param.sie_mclk = NULL;//p_resource->mclk[0];   ///< set NULL if not used
		if (p_resource->flag_addr_va != NULL) {
			eng_param.p_sie_reg_chg_flag = (UINT8 *)p_resource->flag_addr_va;
			memset(eng_param.p_sie_reg_chg_flag, 0, sizeof(UINT8) * SIE_ENG_REG_NUM);
		} else {
			eng_param.p_sie_reg_chg_flag = NULL;
		}

		DBG_IND("eng_id %d, clk_src %d, clk_rate %d\r\n", eng_param.eng_id, eng_param.clock_src, eng_param.clock_rate);
		/* ssdrv - init resource */
		sie_eng_init_resource(&eng_param);
	}
	return 0;
}

INT32 sie_builtin_module_uninit(void)
{
	sie_eng_release();
	return 0;
}

#if 0
#endif
//=============================================================================
// Rtos
//=============================================================================

INT32 sie_builtin_rtos_init(void)
{
#if defined(__FREERTOS)
	UINT32 i, j;
	INT_ID irq_tbl[SIE_MAX_ENG_NUM] = {
		INT_ID_SIE,
		INT_ID_SIE2,
		INT_ID_SIE3,
		INT_ID_SIE4,
		INT_ID_SIE5
	};
	ULONG ioaddr_tbl[SIE_MAX_ENG_NUM] = {
		IOADDR_SIE_REG_BASE,
		IOADDR_SIE2_REG_BASE,
		IOADDR_SIE3_REG_BASE,
		IOADDR_SIE4_REG_BASE,
		IOADDR_SIE5_REG_BASE
	};

	if (p_sie_builtin_resource == NULL) {
		p_sie_builtin_resource = sie_builtin_os_malloc_wrap(sizeof(SIE_BUILTIN_RESOURCE) * SIE_BUILTIN_TOTAL_ENG_COUNT);
	}

	for (i = 0; i < SIE_BUILTIN_ENG_COUNT; i++) {
		p_sie_builtin_resource[i].clk = NULL;
		p_sie_builtin_resource[i].pclk = NULL;
		p_sie_builtin_resource[i].pxclk = NULL;
		p_sie_builtin_resource[i].intclk = NULL;
		for (j = 0; j < SIE_BUILTIN_ENG_COUNT; j++) {
			p_sie_builtin_resource[i].pxclk_pad[j] = NULL;
		}
		for (j = 0; j < SIE_MCLK_MAX_NUM; j++) {
			p_sie_builtin_resource[i].mclk[j] = NULL;
		}
		p_sie_builtin_resource[i].p_io_addr = (void *)ioaddr_tbl[SIE_BUILTIN_CONV2_ENG_IDX(i)];
		p_sie_builtin_resource[i].phy_io_base = ioaddr_tbl[SIE_BUILTIN_CONV2_ENG_IDX(i)];
		p_sie_builtin_resource[i].phy_io_size = SIE_ENG_REG_NUM << 2;
		p_sie_builtin_resource[i].irq_id = irq_tbl[SIE_BUILTIN_CONV2_ENG_IDX(i)];
		if (SIE_RW_MODE == SIE_REG_RW_MODE_BUFFERED) {
			p_sie_builtin_resource[i].reg_addr_va  = sie_builtin_os_malloc_wrap(SIE_ENG_REG_NUM << 2);
			p_sie_builtin_resource[i].flag_addr_va = sie_builtin_os_malloc_wrap(sizeof(UINT8) * SIE_ENG_REG_NUM);
		} else {
			p_sie_builtin_resource[i].reg_addr_va  = 0;
			p_sie_builtin_resource[i].flag_addr_va = 0;
		}
		p_sie_builtin_resource[i].clk_src = 0;
		p_sie_builtin_resource[i].clk_rate = 0;
	}
#endif
	return 0;
}

INT32 sie_builtin_rtos_uninit(void)
{
#if defined(__FREERTOS)
	UINT32 i;

	if (p_sie_builtin_resource) {
		for (i = 0; i < SIE_BUILTIN_TOTAL_ENG_COUNT; i++) {
			if (p_sie_builtin_resource[i].p_io_addr) {
				sie_builtin_os_mfree_wrap(p_sie_builtin_resource[i].p_io_addr);
				p_sie_builtin_resource[i].p_io_addr = NULL;
			}
			if (p_sie_builtin_resource[i].reg_addr_va) {
				sie_builtin_os_mfree_wrap(p_sie_builtin_resource[i].reg_addr_va);
				p_sie_builtin_resource[i].reg_addr_va = NULL;
			}
			if (p_sie_builtin_resource[i].flag_addr_va) {
				sie_builtin_os_mfree_wrap(p_sie_builtin_resource[i].flag_addr_va);
				p_sie_builtin_resource[i].flag_addr_va = NULL;
			}
		}
		sie_builtin_os_mfree_wrap(p_sie_builtin_resource);
		p_sie_builtin_resource = NULL;
	}
#endif
	return 0;
}

//=============================================================================
// Linux
//=============================================================================
#if defined(__KERNEL__)
//=============================================================================
// Global variable
//=============================================================================
static struct of_device_id sie_builtin_match_table[] = {
	{   .compatible = MODULE_NAME_COMPTB},
	{}
};
//=============================================================================
// function declaration
//=============================================================================
int __init nvt_sie_builtin_module_init(void);
void __exit nvt_sie_builtin_module_exit(void);

static int nvt_sie_builtin_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int nvt_sie_builtin_release(struct inode *inode, struct file *file)
{
	return 0;
}

static long nvt_sie_builtin_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return 0;
}

struct file_operations nvt_sie_builtin_fops = {
	.owner   = THIS_MODULE,
	.open    = nvt_sie_builtin_open,
	.release = nvt_sie_builtin_release,
	.unlocked_ioctl = nvt_sie_builtin_ioctl,
	.llseek  = no_llseek,
};

#if 0
#endif

static CHAR *clk_name[SIE_MAX_ENG_NUM] = {
	MODULE_NAME_CLK_DEV_1,
	MODULE_NAME_CLK_DEV_2,
	MODULE_NAME_CLK_DEV_3,
	MODULE_NAME_CLK_DEV_4,
	MODULE_NAME_CLK_DEV_5,
};

static CHAR *clk_out_name[SIE_MAX_ENG_NUM] = {
	MODULE_NAME_CLK_OUT_DEV_1,
	MODULE_NAME_CLK_OUT_DEV_2,
	MODULE_NAME_CLK_OUT_DEV_3,
	MODULE_NAME_CLK_OUT_DEV_4,
	MODULE_NAME_CLK_OUT_DEV_5,
};

static CHAR *pclk_name[SIE_MAX_ENG_NUM] = {
	MODULE_NAME_PCLK_DEV_1,
	MODULE_NAME_PCLK_DEV_2,
	MODULE_NAME_PCLK_DEV_3,
	MODULE_NAME_PCLK_DEV_4,
	MODULE_NAME_PCLK_DEV_5,
};

static CHAR *pxclk_name[SIE_MAX_ENG_NUM] = {
	MODULE_NAME_PXCLK_DEV_1,
	NULL,
	MODULE_NAME_PXCLK_DEV_3,
	NULL,
	MODULE_NAME_PXCLK_DEV_5,
};

static CHAR *intclk_name[SIE_MAX_ENG_NUM] = {
	NULL,
	MODULE_NAME_INTCLK_DEV_2,
	NULL,
	MODULE_NAME_INTCLK_DEV_4,
	NULL,
};


static CHAR *pxclkpad_name[SIE_MAX_ENG_NUM] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

static CHAR *mclk_name[SIE_MAX_ENG_NUM] = {
	MODULE_NAME_MCLK_1,
	MODULE_NAME_MCLK_2,
	MODULE_NAME_MCLK_3,
	MODULE_NAME_MCLK_4,
	NULL,
};

static CHAR *tsenrxclk_name[SIE_MAX_ENG_NUM] = {
	NULL,
	NULL,
	NULL,
	NULL,
	MODULE_NAME_TSENRXCLK_DEV_5,
};

static int nvt_sie_builtin_probe_get_resource(struct platform_device *p_dev, SIE_BUILTIN_DRV_INFO *p_drv_info, int idx)
{
	SIE_BUILTIN_RESOURCE dev_resource;
	struct resource *p_resource;
	int ret = 0;
	UINT32 i;
	struct device_node *node, *from = NULL;
	UINT32 current_rate = 0, parent_idx = 0;

	memset((void *)&dev_resource, 0, sizeof(SIE_BUILTIN_RESOURCE));

	/* register your own device in sysfs, and this will cause udev to create corresponding device node */
	p_drv_info->p_device[idx] = device_create(p_drv_info->pclass, NULL
									 , MKDEV(MAJOR(p_drv_info->dev_id), (idx + MINOR(p_drv_info->dev_id))), NULL
									 , MODULE_NAME"%d", idx);

	if (IS_ERR(p_drv_info->p_device[idx])) {
		DBG_ERR("failed in creating device%d.\n", (int)idx);
		ret = -ENODEV;
		goto FAIL_NONE;
	}

	/* get resource */
	p_resource = platform_get_resource(p_dev, IORESOURCE_MEM, idx);
	if (p_resource == NULL) {
		DBG_ERR("No IO memory resource defined:%d\n", (int)idx);
		ret = -ENODEV;
		goto FAIL_DEVICE_UNREG;
	}

	if (!request_mem_region(p_resource->start, resource_size(p_resource), p_dev->name)) {
		DBG_ERR("failed to request memory resource%d\n", (int)idx);
		ret = -ENODEV;
		goto FAIL_DEVICE_UNREG;
	}

	dev_resource.phy_io_base = p_resource->start;
	dev_resource.phy_io_size = resource_size(p_resource);
	dev_resource.p_io_addr = ioremap(dev_resource.phy_io_base, dev_resource.phy_io_size);
	if (dev_resource.p_io_addr == NULL) {
		DBG_ERR("ioremap() failed in module%d\n", (int)idx);
		ret = -ENODEV;
		goto FAIL_RELEASE_MEM;
	}

	/* get irq id */
	dev_resource.irq_id = platform_get_irq(p_dev, idx);
	if (dev_resource.irq_id < 0) {
		DBG_ERR("idx %d No IRQ resource defined\n", idx);
		ret = -ENODEV;
		goto FAIL_IOUNMAP;
	}

	/* get clock resource */
	// module clk
	dev_resource.clk = devm_clk_get(&p_dev->dev, clk_out_name[idx]);
	if (IS_ERR(dev_resource.clk)) {
		DBG_ERR("idx %d fail to get %s source\n", idx, clk_out_name[idx]);
		ret = -ENODEV;
		goto FAIL_IOUNMAP;
	}

	//clk rate and src
	node = of_find_node_by_name(from, clk_name[idx]);
	if (node) {
		if (!of_property_read_u32(node, "parent_idx", &parent_idx)) {			// get clk src from dtsi
			dev_resource.clk_src = parent_idx;
		}
		if (!of_property_read_u32(node, "current_rate", &current_rate)) {			// get current rate from dtsi
			dev_resource.clk_rate = current_rate;
		}
	}

	// pclk
	dev_resource.pclk = devm_clk_get(&p_dev->dev, pclk_name[idx]);
	if (IS_ERR(dev_resource.pclk)) {
		DBG_ERR("idx %d fail to get %s source\n", idx, pclk_name[idx]);
		ret = -ENODEV;
		goto FAIL_IOUNMAP;
	}
	// pxclk
	if (pxclk_name[idx] != NULL) {
		dev_resource.pxclk = devm_clk_get(&p_dev->dev, pxclk_name[idx]);
		if (IS_ERR(dev_resource.pxclk)) {
			DBG_ERR("idx %d fail to get %s source\n", idx, pxclk_name[idx]);
			ret = -ENODEV;
			goto FAIL_IOUNMAP;
		}
	}
	// pxclk_pad
	for (i = 0; i < SIE_MAX_ENG_NUM; i++) {
		if (pxclkpad_name[i] != NULL) {
			dev_resource.pxclk_pad[i] = devm_clk_get(&p_dev->dev, pxclkpad_name[i]);
			if (IS_ERR(dev_resource.pxclk_pad[i])) {
				DBG_ERR("idx %d fail to get %s source\n", i, pxclkpad_name[i]);
				ret = -ENODEV;
				goto FAIL_IOUNMAP;
			}
		}
	}
	// mclk
	for (i = 0; i < SIE_MCLK_MAX_NUM; i++) {
		if (mclk_name[i] != NULL) {
			dev_resource.mclk[i] = devm_clk_get(&p_dev->dev, mclk_name[i]);
			if (IS_ERR(dev_resource.mclk[i])) {
				DBG_ERR("idx %d fail to get %s source\n", i, mclk_name[i]);
				ret = -ENODEV;
				goto FAIL_IOUNMAP;
			}
		}
	}

	// int clk
	if (intclk_name[idx] != NULL) {
		dev_resource.intclk = devm_clk_get(&p_dev->dev, intclk_name[idx]);
		if (IS_ERR(dev_resource.intclk)) {
			DBG_ERR("idx %d fail to get %s source\n", idx, intclk_name[idx]);
			ret = -ENODEV;
			goto FAIL_IOUNMAP;
		}
	}

	// tsen clk
	for (i = 0; i < SIE_MAX_ENG_NUM; i++) {
		if (tsenrxclk_name[i] != NULL) {
			dev_resource.tsenrxclk[i] = devm_clk_get(&p_dev->dev, tsenrxclk_name[i]);
			if (IS_ERR(dev_resource.tsenrxclk[i])) {
				DBG_ERR("idx %d fail to get %s source\n", i, tsenrxclk_name[i]);
				ret = -ENODEV;
				goto FAIL_IOUNMAP;
			}
		}
	}

	if (SIE_RW_MODE == SIE_REG_RW_MODE_BUFFERED) {
		dev_resource.reg_addr_va 	= sie_builtin_os_malloc_wrap(SIE_ENG_REG_NUM << 2);
		dev_resource.flag_addr_va 	= sie_builtin_os_malloc_wrap(sizeof(UINT8) * SIE_ENG_REG_NUM);
	} else {
		dev_resource.reg_addr_va 	= NULL;
		dev_resource.flag_addr_va 	= NULL;
	}

	/* set resource */
	sie_builtin_resource_set(&dev_resource, idx);

#if 0
	DBG_ERR("\r\n[dump info] - BEGIN\r\n");
	DBG_DUMP("[%-12s] %d \r\n", 	"idx", 			idx);
	DBG_DUMP("[%-12s] %d \r\n", 	"irq_id", 		dev_resource.irq_id);
	DBG_DUMP("[%-12s] %lx \r\n", 	"p_io_addr", 	(ULONG)dev_resource.p_io_addr);
	DBG_DUMP("[%-12s] %lx \r\n", 	"phy_io_base", 	(ULONG)dev_resource.phy_io_base);
	DBG_DUMP("[%-12s] %lx \r\n", 	"phy_io_size", 	(ULONG)dev_resource.phy_io_size);
	DBG_DUMP("[%-12s] %d \r\n", 	"phy_io_size", 	dev_resource.phy_io_size);
	DBG_DUMP("[%-12s] %lx \r\n", 	"reg_addr_va",  (ULONG)dev_resource.reg_addr_va);
	DBG_DUMP("[%-12s] %lx \r\n", 	"flag_addr_va", (ULONG)dev_resource.flag_addr_va);
	DBG_ERR("[dump info] - END\r\n");
#endif
	return ret;

FAIL_IOUNMAP:
	iounmap(dev_resource.p_io_addr);

FAIL_RELEASE_MEM:
	release_mem_region(p_resource->start, resource_size(p_resource));

FAIL_DEVICE_UNREG:
	device_unregister(p_drv_info->p_device[idx]);

FAIL_NONE:
	return ret;
}

static int nvt_sie_builtin_remove_rls_resource(struct platform_device *p_dev, SIE_BUILTIN_DRV_INFO *p_drv_info, int idx)
{
	SIE_BUILTIN_RESOURCE *p_dev_resource;
	UINT32 i;

	p_dev_resource = sie_builtin_resource_get(idx);
	if (p_dev_resource) {
		release_mem_region(p_dev_resource->phy_io_base, p_dev_resource->phy_io_size);
		iounmap(p_dev_resource->p_io_addr);
		if (p_dev_resource->clk) {
			devm_clk_put(&p_dev->dev, p_dev_resource->clk);
		}

		if (p_dev_resource->pclk) {
			devm_clk_put(&p_dev->dev, p_dev_resource->pclk);
		}

		if (p_dev_resource->pxclk) {
			devm_clk_put(&p_dev->dev, p_dev_resource->pxclk);
		}

		if (p_dev_resource->intclk) {
			devm_clk_put(&p_dev->dev, p_dev_resource->intclk);
		}

		for (i = 0; i < SIE_MAX_ENG_NUM; i++) {
			if (p_dev_resource->pxclk_pad[i]) {
				devm_clk_put(&p_dev->dev, p_dev_resource->pxclk_pad[i]);
			}

			if (p_dev_resource->tsenrxclk[i]) {
				devm_clk_put(&p_dev->dev, p_dev_resource->tsenrxclk[i]);
			}
		}

		for (i = 0; i < SIE_MCLK_MAX_NUM; i++) {
			if (p_dev_resource->mclk[i]) {
				devm_clk_put(&p_dev->dev, p_dev_resource->mclk[i]);
			}
		}

		if (p_dev_resource->flag_addr_va) {
			sie_builtin_os_mfree_wrap(p_dev_resource->flag_addr_va);
		}

		if (p_dev_resource->reg_addr_va) {
			sie_builtin_os_mfree_wrap(p_dev_resource->reg_addr_va);
		}
	}

	if (p_drv_info && p_drv_info->p_device[idx]) {
		device_unregister(p_drv_info->p_device[idx]);
	}
	return 0;
}
static int nvt_sie_builtin_probe(struct platform_device *pdev)
{
	SIE_BUILTIN_DRV_INFO *pdrv_info = NULL;
	const struct of_device_id *match;
	int ret = 0;
	int ucloop;

	match = of_match_device(sie_builtin_match_table, &pdev->dev);
	if (!match) {
		DBG_ERR("Platform device not found \n");
		return -EINVAL;
	}

	pdrv_info = kzalloc(sizeof(SIE_BUILTIN_DRV_INFO), GFP_KERNEL);
	if (!pdrv_info) {
		DBG_ERR("failed to allocate memory\n");
		return -ENOMEM;
	}

	/* Dynamic to allocate Device ID */
	if (vos_alloc_chrdev_region(&pdrv_info->dev_id, SIE_BUILTIN_TOTAL_CH_COUNT, MODULE_NAME)) {
		DBG_ERR("Can't get device ID\n");
		ret = -ENODEV;
		goto FAIL_FREE_BUF;
	}

	/* Register character device for the volume */
	cdev_init(&pdrv_info->cdev, &nvt_sie_builtin_fops);
	pdrv_info->cdev.owner = THIS_MODULE;

	if (cdev_add(&pdrv_info->cdev, pdrv_info->dev_id, SIE_BUILTIN_TOTAL_CH_COUNT)) {
		DBG_ERR("Can't add cdev\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

	pdrv_info->pclass = class_create(THIS_MODULE, MODULE_NAME_CLASS);
	if (IS_ERR(pdrv_info->pclass)) {
		DBG_ERR("failed in creating class.\n");
		ret = -ENODEV;
		goto FAIL_CLASS;
	}

	/* alloc sie_builtin_builtin memory */
	sie_builtin_resource_init();

	/* request resources */
	for (ucloop = 0 ; ucloop < SIE_BUILTIN_TOTAL_CH_COUNT; ucloop++) {
		ret = nvt_sie_builtin_probe_get_resource(pdev, pdrv_info, ucloop);
		if (ret) {
			while (ucloop > 0) {
				ucloop--;
				nvt_sie_builtin_remove_rls_resource(pdev, pdrv_info, ucloop);
			}
			ret = -ENODEV;
			goto FAIL_PROC;
		}
	}
	platform_set_drvdata(pdev, pdrv_info);
	sie_builtin_module_init();

	return ret;

FAIL_PROC:

FAIL_CLASS:
	class_destroy(pdrv_info->pclass);

FAIL_CDEV:
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, SIE_BUILTIN_MINOR_COUNT);

FAIL_FREE_BUF:
	if (pdrv_info != NULL) {
		kfree(pdrv_info);

		// coverity[assigned_pointer]
		pdrv_info = NULL;
	}

	return ret;
}

static int nvt_sie_builtin_remove(struct platform_device *pdev)
{
	PSIE_BUILTIN_DRV_INFO pdrv_info = NULL;
	unsigned char ucloop;

	pdrv_info = platform_get_drvdata(pdev);

	sie_builtin_module_uninit();

	for (ucloop = 0 ; ucloop < SIE_BUILTIN_TOTAL_CH_COUNT; ucloop++) {
		nvt_sie_builtin_remove_rls_resource(pdev, pdrv_info, ucloop);
	}
	sie_builtin_resource_uninit();

	class_destroy(pdrv_info->pclass);
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, SIE_BUILTIN_TOTAL_CH_COUNT);

	// coverity[check_after_deref]
	if (pdrv_info != NULL) {
		kfree(pdrv_info);

		// coverity[assigned_pointer]
		pdrv_info = NULL;
	}

	return 0;
}

static struct platform_driver nvt_sie_builtin_driver = {
	.driver = {
		.name = MODULE_NAME,
		.of_match_table = sie_builtin_match_table,
	},
	.probe = nvt_sie_builtin_probe,
	.remove = nvt_sie_builtin_remove,
};

int __init nvt_sie_builtin_module_init(void)
{
	return platform_driver_register(&nvt_sie_builtin_driver);
}

void __exit nvt_sie_builtin_module_exit(void)
{
	platform_driver_unregister(&nvt_sie_builtin_driver);
}

module_init(nvt_sie_builtin_init);
module_exit(nvt_sie_builtin_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("SIE Kdriver");
MODULE_LICENSE("GPL");
#endif
