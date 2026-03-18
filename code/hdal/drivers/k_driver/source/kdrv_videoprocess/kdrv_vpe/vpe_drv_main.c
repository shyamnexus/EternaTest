/**
 * @file vpe_drv.c
 *  vpe_drv lib
 *
 *
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#ifdef CONFIG_PM
#include <linux/soc/nvt/nvt-info.h>
#endif
#include "linux/soc/nvt/nvt-pcie-lib.h"
#include "vpe_drv_dbg_int.h"
#include "vpe_drv_ctl_int.h"
#include "vpe_drv_proc_int.h"
#include "vpe_drv_util_int.h"
#include "vpe_drv_init_int.h"
#include "vpe_drv_mm_int.h"

#include "vpe_eng.h"

struct drv_match_info {
	int ch_idx; //channel idx
	int sts;	//0:uninit, 1:init
	int act;	//0:disable, 1:enable
    unsigned char *drv_eng_name;
};

static unsigned int mod_init = 1;
module_param_named(mod_init, mod_init, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(mod_init, "module init");

extern u16 vpe_drv_max_job_queue_num;
module_param_named(max_job_queue_num, vpe_drv_max_job_queue_num, short, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(max_job_queue_num, "max job queue num");

extern u16 vpe_drv_sw_bal_mode;	//default enable, 0:disable
module_param_named(sw_bal_mode, vpe_drv_sw_bal_mode, short, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(sw_bal_mode, "sw_bal_mode");

extern u16 vpe_drv_sw_bal_mask;  //bit[0]: vpe0, bit[1]:vpe1.....
module_param_named(sw_bal_mask, vpe_drv_sw_bal_mask, short, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(sw_bal_mask, "sw_bal_mask");

extern u16 vpe_drv_max_proc_ll_num;
module_param_named(max_proc_ll_num, vpe_drv_max_proc_ll_num, short, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(max_proc_ll_num, "max ll proc num");

extern u16 vpe_drv_ctl_clk_type;
module_param_named(ctl_clk_type, vpe_drv_ctl_clk_type, short, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(ctl_clk_type, "ctl clk type");

#if (KDRV_INTERFACE == ENABLE)
extern unsigned int vpe_kdrv_handle_max;
module_param_named(kdrv_handle_max, vpe_kdrv_handle_max, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(kdrv_handle_max, "max kdrv handle num");
#endif

static long vpe_drv_module_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return 0;
}

static struct file_operations vpe_drv_fops = {
    .unlocked_ioctl = vpe_drv_module_ioctl,
};

static struct miscdevice vpe_drv_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = VPE_DRV_NAME,
	.fops = &vpe_drv_fops,
};

static struct drv_match_info drv_match_tab[] = {
	{ 0, 0, 1, "vpe_rc"},	//rc  eng0 (default act = 1)

};

#ifdef CONFIG_OF
static const struct of_device_id vpe_drv_dt_match[] = {
	{.compatible = "nvt,kdrv_vpe", .data = &drv_match_tab[0]},
	{},
};
MODULE_DEVICE_TABLE(of, vpe_drv_dt_match);
#endif

int vpe_drv_module_dump_info(void)
{
	int i, total_num, chip, eng;
	struct drv_match_info *match_info;

	vpe_drv_log("====module_ctl=====\n");
	vpe_drv_log("ch chip eng sts act\n");

	total_num = sizeof(drv_match_tab) / sizeof(struct drv_match_info);

	for (i = 0; i < total_num; i ++) {

		match_info = &drv_match_tab[i];
		if (match_info->act == 1) {
			chip = VPE_IDX_TO_CHIP_NUM(match_info->ch_idx);
			eng = VPE_IDX_TO_ENG_NUM(match_info->ch_idx);
			vpe_drv_log("%2d %4d %3d %3d %3d\n", match_info->ch_idx, chip, eng, match_info->sts, match_info->act);
		}
	}
	return 0;
}

static int vpe_drv_module_update_ep_info(void)
{
#ifdef CONFIG_NVT_PCIE_LIB
	int i, j;
	int act, idx;
	int rt;

	rt = 0;

	for (i = CHIP_EP0; i < CHIP_MAX; i ++) {

		act = vpe_drv_nvtpcie_downstream_active(i);

		if (act == 1) {
			rt += 1;
			for (j = 0; j < vpe_drv_eng_get_eng_num(); j ++) {
				idx = VPE_CHIP_ENG_TO_IDX((i - CHIP_EP0 + 1), j);

				//update match table infor
				drv_match_tab[idx].act = 1;
			}
		}
	}

	return rt;
#else
	return 0;
#endif
}

static int vpe_drv_module_remove_by_ch(struct vpe_drv_eng_info *eng)
{
	clk_unprepare(eng->mclk);
	clk_put(eng->mclk);
	iounmap(eng->vbase);
	release_mem_region(eng->pbase, eng->mem_len);
	return 0;
}

static int vpe_drv_module_remove(struct platform_device *pdev)
{
	int total_ch, i;
	struct vpe_drv_eng_info *eng;

	vpe_drv_info("start\n");

	total_ch = vpe_drv_eng_get_total_ch();

	for (i = 0; i < total_ch; i ++) {

		if (drv_match_tab[i].sts == 1) {
			eng = vpe_drv_eng_get_resource(i);
			if (eng) {
				vpe_drv_module_remove_by_ch(eng);
			}
			drv_match_tab[i].sts = 0;
		}
	}
	vpe_drv_info("end\n");
    return 0;
}

static int vpe_drv_module_probe_by_ch(struct platform_device *pdev, struct drv_match_info *info)
{
	struct vpe_drv_eng_info eng;

	vpe_drv_info("start .... %s\n", info->drv_eng_name);

    /* setup resource */
    eng.res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (unlikely((!eng.res))) {
        vpe_drv_err("fails: platform_get_resource IORESOURCE_MEM not OK\n");
        goto NOR_ERR;
    }

    eng.irq_no = platform_get_irq(pdev, 0);
    if (unlikely(eng.irq_no < 0)) {
        vpe_drv_err("fails: platform_get_irq not OK\n");
        goto NOR_ERR;
    }
	vpe_drv_info("irq_no = %d\n", eng.irq_no);

    eng.mem_len = eng.res->end - eng.res->start + 1;
	vpe_drv_info("reg(0x%lx 0x%lx 0x%lx)\n", (uintptr_t)eng.res->start, (uintptr_t)eng.res->end, eng.mem_len);
    eng.pbase = eng.res->start;
    if (unlikely(!request_mem_region(eng.pbase, eng.mem_len, pdev->name))) {
        vpe_drv_err("fails: request_mem_region not OK\n");
        goto NOR_ERR;
    }

    eng.vbase = ioremap(eng.pbase, eng.mem_len);
    if (unlikely(eng.vbase == 0)) {
        vpe_drv_err("fails: ioremap_nocache not OK\n");
        goto REL_MEM_REG;
    }

	snprintf(&eng.name[0], sizeof(eng.name), info->drv_eng_name);
	vpe_drv_info("eng name %s\n", &eng.name[0]);

	eng.mclk = clk_get(&pdev->dev, dev_name(&pdev->dev));
	vpe_drv_info("dev name %s\n", dev_name(&pdev->dev));
	if (unlikely(eng.mclk == 0)) {
		vpe_drv_err("fails: clk_get fail\n");
		goto IOUNMAP;
	}
	eng.clk_rate = clk_get_rate(eng.mclk);
	vpe_drv_info("%s clk def rate = %ld\n", info->drv_eng_name, eng.clk_rate);
	clk_prepare(eng.mclk);

	vpe_drv_eng_set_resource(info->ch_idx, &eng);
	vpe_drv_info("end \n");
	return 0;

IOUNMAP:
	iounmap(eng.vbase);

REL_MEM_REG:
	release_mem_region(eng.pbase, eng.mem_len);

NOR_ERR:
	vpe_drv_err("error leave \n");
    return -1;
}

static int vpe_drv_module_probe(struct platform_device *pdev)
{
    int ret;
	int total_ch;
	const struct of_device_id *match;
	struct drv_match_info *match_info;

	vpe_drv_info("start\n");

	match = of_match_device(vpe_drv_dt_match, &pdev->dev);
	if (!match) {
		vpe_drv_err("Platform device not found\n");
		return -EINVAL;
	}

	//vpe_drv_info("math name: %s\n", match->compatible);

	match_info = (struct drv_match_info *)match->data;

	//match device, but chip is not active, don't care
	if (match_info->act == 0) {
		vpe_drv_info("chip is not act\n");
		return 0;
	}

	if (match_info->sts != 0) {
		vpe_drv_err("error %s(%d %d) already init\n", match->compatible, match_info->ch_idx, match_info->sts);
		return -EINVAL;
	}

	total_ch = vpe_drv_eng_get_total_ch();
	if (total_ch == 0) {
		vpe_drv_err("error ch_num = 0 (%d x %d)\n", vpe_drv_eng_get_chip_num(), vpe_drv_eng_get_eng_num());
		return -EINVAL;
	}

	if (match_info->ch_idx >= total_ch) {
		vpe_drv_err("error %s match_ch_idx(%d) > total_ch(%d)\n", match->compatible, match_info->ch_idx, total_ch);
		return -EINVAL;
	}

	/***** request eng resource & information (linux) *****/
	ret = vpe_drv_module_probe_by_ch(pdev, match_info);
	if (ret < 0) {
		vpe_drv_err("probe ch(%d) fail\n", match_info->ch_idx);
		goto SET_DRV_DATA;
	}
	match_info->sts = 1;
	vpe_drv_info("end\n");
    return 0;

SET_DRV_DATA:
    return -1;
}

#ifdef CONFIG_PM
static int vpe_drv_module_suspend(struct device *dev)
{
	int rt;
	const struct of_device_id *match;
	struct drv_match_info *match_info;
	struct vpe_drv_eng_info *eng;

	match = of_match_device(vpe_drv_dt_match, dev);
	if (!match) {
		vpe_drv_err("Platform device not found\n");
		return -EINVAL;
	}
	vpe_drv_info("match name: %s\n", match->compatible);

	match_info = (struct drv_match_info *)match->data;
	rt = vpe_drv_ctl_suspend(match_info->ch_idx);
	if (rt < 0) {
		vpe_drv_err("%s suspend fail\n", match->compatible);
		return -EINVAL;
	}

	eng = vpe_drv_eng_get_resource(match_info->ch_idx);
	if (eng == 0) {
		vpe_drv_err("%s get_resource fail\n", match->compatible);
		return -EINVAL;
	}

	eng->clk_phase = clk_get_phase(eng->mclk);
	vpe_drv_info("clk_phase = %d\n", eng->clk_phase);

	eng->clk_en_cnt = 0;
	/* 0: not power off, 1: power off */
	if (nvt_get_suspend_mode()) {
		while(__clk_is_enabled(eng->mclk)) {
			clk_disable(eng->mclk);
			eng->clk_en_cnt += 1;
		}
	}
	vpe_drv_info("clk_en_cnt = %d\n", eng->clk_en_cnt);
	return 0;
}

static int vpe_drv_module_resume(struct device *dev)
{
	int rt;
	const struct of_device_id *match;
	struct drv_match_info *match_info;
	struct vpe_drv_eng_info *eng;

	match = of_match_device(vpe_drv_dt_match, dev);
	if (!match) {
		vpe_drv_err("Platform device not found\n");
		return -EINVAL;
	}
	vpe_drv_info("match name: %s\n", match->compatible);

	match_info = (struct drv_match_info *)match->data;

	eng = vpe_drv_eng_get_resource(match_info->ch_idx);
	if (eng == 0) {
		vpe_drv_err("%s get_resource fail\n", match->compatible);
		return -EINVAL;
	}

	vpe_drv_info("clk_en_cnt = %d\n", eng->clk_en_cnt);
	while(eng->clk_en_cnt != 0) {
		clk_enable(eng->mclk);
		eng->clk_en_cnt -= 1;
	}

	vpe_drv_info("clk_phase = %d\n", eng->clk_phase);
	clk_set_phase(eng->mclk, eng->clk_phase);

	rt = vpe_drv_ctl_resume(match_info->ch_idx);
	if (rt < 0) {
		vpe_drv_err("%s resume fail\n", match->compatible);
		return -EINVAL;
	}
	return 0;
}

static const struct dev_pm_ops vpe_drv_pm_ops = {
	.suspend = vpe_drv_module_suspend,
	.resume = vpe_drv_module_resume,
};
#endif


static struct platform_driver vpe_drv = {
    .driver = {
            .name = VPE_DRV_NAME,
            .owner = THIS_MODULE,
#ifdef CONFIG_OF
			.of_match_table = of_match_ptr(vpe_drv_dt_match),
#endif

#ifdef CONFIG_PM
			.pm = &vpe_drv_pm_ops,
#endif
	},

	.probe  = vpe_drv_module_probe,
	.remove = vpe_drv_module_remove,
};

static int __init vpe_drv_module_initial(void)
{
    int ret = 0, total_chip_num;

    vpe_drv_dump("%s Version: %s %s %s\n", VPE_DRV_NAME, VPE_DRV_MODULE_VERSION, __DATE__, __TIME__);
    vpe_ssdrv_proc_ver();
    vpe_drv_info("init start\n");

	//initial mm ctl system
	ret = vpe_drv_mm_init();
	if (ret < 0) {
		vpe_drv_err("drv mm init fail\n");
		return -1;
	}

	total_chip_num = vpe_drv_module_update_ep_info();
	total_chip_num = total_chip_num + 1; //add rc
	vpe_drv_info("total chip num = %d\n", total_chip_num);
	vpe_drv_eng_set_chip_num(total_chip_num);

	/* alloc eng_info memory */
	vpe_drv_eng_init();

    /* register vpe_drv */
    ret = platform_driver_register(&vpe_drv);
    if(ret) {
        vpe_drv_err("register %s driver failed!\n", VPE_DRV_NAME);
		return -1;
    }

	/* register user interface */
    ret = misc_register(&vpe_drv_misc_dev);
    if (ret < 0) {
        vpe_drv_err("register %s misc device fail!\n", VPE_DRV_NAME);
        goto err_pf_reg;
    }

	/* init ctl flow */
	if (mod_init) {
		ret = vpe_drv_module_init();
		if (ret < 0) {
	        vpe_drv_err("vpe_drv_module_init failed!\n");
			goto err_misc_reg;
		}
	}
    vpe_drv_info("init end\n");
    return 0;

err_misc_reg:
	misc_deregister(&vpe_drv_misc_dev);

err_pf_reg:
	platform_driver_unregister(&vpe_drv);

    return ret;
}

static void __exit vpe_drv_module_exit(void)
{
    vpe_drv_info("exit start\n");

	vpe_drv_module_uninit();

	misc_deregister(&vpe_drv_misc_dev);

	platform_driver_unregister(&vpe_drv);

	vpe_drv_eng_uninit();

	vpe_drv_mm_uninit();

    vpe_drv_info("exit end\n");
}

module_init(vpe_drv_module_initial);
module_exit(vpe_drv_module_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_LICENSE("GPL");

