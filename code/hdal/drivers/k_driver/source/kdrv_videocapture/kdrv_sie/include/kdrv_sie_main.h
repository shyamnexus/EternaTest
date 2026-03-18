#ifndef __KDRV_SIE_MAIN_H__
#define __KDRV_SIE_MAIN_H__
#include <linux/cdev.h>
#include "kdrv_sie_int_init.h"

#define MODULE_MINOR_ID      0
#define MODULE_MINOR_COUNT   1
#define MODULE_NAME          "kdrv_sie"
#define MODULE_NAME_COMPTB   "nvt,nvt_sie"
#define MODULE_NAME_CLASS    "nvt_kdrv_sie"

#define MODULE_NAME_CLK_DEV_1	"2f0310000.sie1"
#define MODULE_NAME_CLK_DEV_2	"2f0311000.sie2"
#define MODULE_NAME_CLK_DEV_3	"2f0312000.sie3"
#define MODULE_NAME_CLK_DEV_4	"2f0313000.sie4"
#define MODULE_NAME_CLK_DEV_5	"2f0314000.sie5"

#define MODULE_NAME_PCLK_DEV_1	"sie1_pclk"
#define MODULE_NAME_PCLK_DEV_2	"sie2_pclk"
#define MODULE_NAME_PCLK_DEV_3	"sie3_pclk"
#define MODULE_NAME_PCLK_DEV_4	"sie4_pclk"
#define MODULE_NAME_PCLK_DEV_5	"sie5_pclk"

#define MODULE_NAME_PXCLK_PAD_1	 "sie_pxclkpad"
#define MODULE_NAME_PXCLK_PAD_2	 "sie_pxclkpad"
#define MODULE_NAME_PXCLK_PAD_3	 "sie_pxclkpad"
#define MODULE_NAME_PXCLK_PAD_4	 "sie_pxclkpad"
#define MODULE_NAME_PXCLK_PAD_5	 "sie_pxclkpad"

#define MODULE_NAME_PXCLK_DEV_1	 "sie1_pxclk"
#define MODULE_NAME_PXCLK_DEV_3	 "sie3_pxclk"
#define MODULE_NAME_PXCLK_DEV_5	 "sie5_pxclk"

#define MODULE_NAME_MCLK_1		"sn_mclk1"
#define MODULE_NAME_MCLK_2		"sn_mclk2"
#define MODULE_NAME_MCLK_3		"sn_mclk3"
#define MODULE_NAME_MCLK_4		"sn_mclk4"

#define MODULE_NAME_INTCLK_DEV_2	"sie2_intclk"
#define MODULE_NAME_INTCLK_DEV_4	"sie4_intclk"

#define MODULE_NAME_TSENRXCLK_DEV_5	"sie5_tsen_rxclk"

typedef struct module_info {

} KDRV_SIE_MODULE_INFO, *PKDRV_SIE_MODULE_INFO;

typedef struct {
	KDRV_SIE_MODULE_INFO module_info;
	struct device *p_device[KDRV_SIE_TOTAL_CH_COUNT];
	struct class *pclass;
	struct cdev cdev;
	dev_t dev_id;

    // proc entries
    struct proc_dir_entry *pproc_module_root;
    struct proc_dir_entry *pproc_help_entry;
    struct proc_dir_entry *pproc_cmd_entry;
	struct proc_dir_entry *pproc_info_entry;
	struct proc_dir_entry *pproc_dbglevel_entry;
	struct proc_dir_entry *pproc_gating_entry;

	struct seq_file *info_seq_file;
} KDRV_SIE_DRV_INFO, *PKDRV_SIE_DRV_INFO;


#endif
