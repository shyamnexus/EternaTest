/**
 * @file graph_drv.c
 *  graph_drv lib
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

#include "linux/soc/nvt/nvt-pcie-lib.h"
#include "grph_drv_dbg_int.h"
#include "grph_drv_ctl_int.h"
#include "grph_drv_proc_int.h"
#include "grph_drv_util_int.h"
#include "grph_drv_init_int.h"
#include "grph_drv_mm_int.h"

struct drv_match_info {
	int ch_idx; //channel idx
	int sts;	//0:uninit, 1:init
	int act;	//0:disable, 1:enable
    unsigned char *drv_eng_name;
};

static unsigned int graph_mod_init = 1;
module_param_named(graph_mod_init, graph_mod_init, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(graph_mod_init, "module init");

extern u16 graph_drv_max_job_queue_num;
module_param_named(graph_drv_max_job_queue_num, graph_drv_max_job_queue_num, short, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(graph_drv_max_job_queue_num, "max job queue num");

//force disable sw_bal_mode
//extern u16 graph_drv_sw_bal_mode;	//default enable, 0:disable
//module_param_named(graph_drv_sw_bal_mode, graph_drv_sw_bal_mode, short, S_IRUGO | S_IWUSR);
//MODULE_PARM_DESC(graph_drv_sw_bal_mode, "sw_bal_mode");

extern u16 graph_drv_flow_clk_type;	//default enable, 0:disable
module_param_named(graph_drv_flow_clk_type, graph_drv_flow_clk_type, short, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(graph_drv_flow_clk_type, "ctl clk type");

#if (KDRV_INTERFACE == ENABLE)
extern unsigned int graph_kdrv_handle_max;
module_param_named(graph_kdrv_handle_max, graph_kdrv_handle_max, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(graph_kdrv_handle_max, "max kdrv handle num");
#endif

static long graph_drv_module_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return 0;
}

static struct file_operations graph_drv_fops = {
    .unlocked_ioctl = graph_drv_module_ioctl,
};

static struct miscdevice graph_drv_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = GRAPH_DRV_NAME,
	.fops = &graph_drv_fops,
};

static struct drv_match_info drv_match_tab[] = {
	{ 0, 0, 1, "graph_rc"},	//rc  eng0 (default act = 1)
};

#ifdef CONFIG_OF
static const struct of_device_id graph_drv_dt_match[] = {
	{.compatible = "nvt,kdrv_graph", .data = &drv_match_tab[0]},
	{},
};
MODULE_DEVICE_TABLE(of, graph_drv_dt_match);
#endif

int graph_drv_module_dump_info(void)
{
	int i, total_num, chip, eng;
	struct drv_match_info *match_info;

	graph_drv_log("====module_ctl=====\n");
	graph_drv_log("ch chip eng sts act\n");

	total_num = sizeof(drv_match_tab) / sizeof(struct drv_match_info);

	for (i = 0; i < total_num; i ++) {

		match_info = &drv_match_tab[i];
		if (match_info->act == 1) {
			chip = GRAPH_IDX_TO_CHIP_NUM(match_info->ch_idx);
			eng = GRAPH_IDX_TO_ENG_NUM(match_info->ch_idx);
			graph_drv_log("%2d %4d %3d %3d %3d\n", match_info->ch_idx, chip, eng, match_info->sts, match_info->act);
		}
	}
	return 0;
}

static int graph_drv_module_update_ep_info(void)
{
#ifdef CONFIG_NVT_PCIE_LIB
	int i, j;
	int act, idx;
	int rt;

	rt = 0;

	for (i = CHIP_EP0; i < CHIP_MAX; i ++) {

		act = graph_drv_nvtpcie_downstream_active(i);

		if (act == 1) {
			rt += 1;
			for (j = 0; j < graph_drv_eng_get_eng_num(); j ++) {
				idx = GRAPH_CHIP_ENG_TO_IDX((i - CHIP_EP0 + 1), j);

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

static int __init graph_drv_module_initial(void)
{
    int ret = 0, total_chip_num;

    graph_drv_dump("%s Version: %s %s %s\n", GRAPH_DRV_NAME, GRAPH_DRV_MODULE_VERSION, __DATE__, __TIME__);
    //graph_ssdrv_proc_ver();
    graph_drv_info("init start\n");

	//initial mm ctl system
	ret = graph_drv_mm_init();
	if (ret < 0) {
		graph_drv_err("drv mm init fail\n");
		return -1;
	}

	total_chip_num = graph_drv_module_update_ep_info();
	total_chip_num = total_chip_num + 1; //add rc
	graph_drv_info("total chip num = %d\n", total_chip_num);
	graph_drv_eng_set_chip_num(total_chip_num);

	/* alloc eng_info memory */
	graph_drv_eng_init();

	/* register user interface */
    ret = misc_register(&graph_drv_misc_dev);
    if (ret < 0) {
        graph_drv_err("register %s misc device fail!\n", GRAPH_DRV_NAME);
        return -1;
    }

	/* init ctl flow */
	if (graph_mod_init) {
		ret = graph_drv_module_init();
		if (ret < 0) {
	        graph_drv_err("graph_drv_module_init failed!\n");
			goto err_misc_reg;
		}
	}
    graph_drv_info("init end\n");
    return 0;

err_misc_reg:
	misc_deregister(&graph_drv_misc_dev);
    return ret;
}

static void __exit graph_drv_module_exit(void)
{
    graph_drv_info("exit start\n");

	graph_drv_module_uninit();

	misc_deregister(&graph_drv_misc_dev);

	graph_drv_eng_uninit();

	graph_drv_mm_uninit();

    graph_drv_info("exit end\n");
}

#if defined(_GROUP_KO_)
int nvt_kdrv_graph_module_init(void)
{
	return graph_drv_module_initial();
}

void nvt_kdrv_graph_module_exit(void)
{
	graph_drv_module_exit();
}
#endif

module_init(graph_drv_module_initial);
module_exit(graph_drv_module_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_LICENSE("GPL");
MODULE_VERSION(GRAPH_DRV_MODULE_VERSION);
