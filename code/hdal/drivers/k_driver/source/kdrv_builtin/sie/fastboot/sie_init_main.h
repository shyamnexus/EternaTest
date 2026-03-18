#ifndef __SIE_INIT_MAIN_H__
#define __SIE_INIT_MAIN_H__
#if defined(__KERNEL__)
#include <linux/cdev.h>
#endif
#include "kwrap/type.h"
#include "sie_eng_limit.h"

#define MODULE_MINOR_ID      0
#define MODULE_MINOR_COUNT   1
#define MODULE_NAME          "kdrv_sie"
#define MODULE_NAME_COMPTB   "nvt,nvt_sie"
#define MODULE_NAME_CLASS    "nvt_kdrv_sie"

#define MODULE_NAME_CLK_DEV_1	"clk_sie1"
#define MODULE_NAME_CLK_DEV_2	"clk_sie2"
#define MODULE_NAME_CLK_DEV_3	"clk_sie3"
#define MODULE_NAME_CLK_DEV_4	"clk_sie4"
#define MODULE_NAME_CLK_DEV_5	"clk_sie5"

#define MODULE_NAME_CLK_OUT_DEV_1	"2f0310000.sie1"
#define MODULE_NAME_CLK_OUT_DEV_2	"2f0311000.sie2"
#define MODULE_NAME_CLK_OUT_DEV_3	"2f0312000.sie3"
#define MODULE_NAME_CLK_OUT_DEV_4	"2f0313000.sie4"
#define MODULE_NAME_CLK_OUT_DEV_5	"2f0314000.sie5"

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

#define SIE_BUILTIN_CHIP_COUNT     	(1)
#define SIE_BUILTIN_ENG_COUNT    	SIE_MAX_ENG_NUM
#define SIE_BUILTIN_MINOR_COUNT    	(1)
#define SIE_BUILTIN_TOTAL_ENG_COUNT 	(SIE_BUILTIN_CHIP_COUNT *SIE_BUILTIN_ENG_COUNT)
#define SIE_BUILTIN_TOTAL_CH_COUNT 	(SIE_BUILTIN_CHIP_COUNT * SIE_BUILTIN_ENG_COUNT * SIE_BUILTIN_MINOR_COUNT)

#define SIE_BUILTIN_CONV2_ENG_IDX(hdl_idx) ((hdl_idx % (SIE_BUILTIN_ENG_COUNT * SIE_BUILTIN_MINOR_COUNT)) / SIE_BUILTIN_MINOR_COUNT)

typedef struct module_info {

} SIE_BUILTIN_MODULE_INFO, *PSIE_BUILTIN_MODULE_INFO;

typedef struct {
	/* [BEGIN] linux struct clk*, not used in freertos */
	void *clk;
	void *pclk;
	void *pxclk;
	void *intclk;
	void *pxclk_pad[SIE_MAX_ENG_NUM];
	void *mclk[SIE_MCLK_MAX_NUM];
	void *tsenrxclk[SIE_MAX_ENG_NUM];
	/* [ END ] linux struct clk*, not used in freertos */

	void *p_io_addr;    /* virtual register address */
	ULONG phy_io_base;  /* physical register address */
	UINT32 phy_io_size; /* physical register size */
	INT32 irq_id;       /* interrupt id */

	void *reg_addr_va;	/* dram buffer for ssdrv register */
	void *flag_addr_va; /* dram buffer for ssdrv flag */
	UINT32 clk_src;		/* sie clock src sel for ssdrv, set by clock dtsi */
	UINT32 clk_rate;	/* sie clock rate for ssdrv, set by clock dtsi */
} SIE_BUILTIN_RESOURCE;

#if defined(__KERNEL__)
typedef struct {
	SIE_BUILTIN_MODULE_INFO module_info;
	struct device *p_device[SIE_BUILTIN_ENG_COUNT];
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
} SIE_BUILTIN_DRV_INFO, *PSIE_BUILTIN_DRV_INFO;
#endif

INT32 sie_builtin_rtos_init(void);
INT32 sie_builtin_rtos_uninit(void);
INT32 sie_builtin_module_init(void);
INT32 sie_builtin_module_uninit(void);

#endif
