#ifndef _KDRV_SIE_INT_INIT_H_
#define _KDRV_SIE_INT_INIT_H_

#include "sie_eng.h"
#include "sie_eng_limit.h"
#include "kdrv_builtin/sie_init.h"

#define KDRV_SIE_CHIP_COUNT     	(1)
#define KDRV_SIE_ENG_COUNT    		SIE_MAX_ENG_NUM  // (5)
#define KDRV_SIE_MINOR_COUNT    	(1)
#define KDRV_SIE_TOTAL_ENG_COUNT 	(KDRV_SIE_CHIP_COUNT * KDRV_SIE_ENG_COUNT)
#define KDRV_SIE_TOTAL_CH_COUNT 	(KDRV_SIE_CHIP_COUNT * KDRV_SIE_ENG_COUNT * KDRV_SIE_MINOR_COUNT)
#define KDRV_SIE_MCLK_COUNT         SIE_MCLK_MAX_NUM // (4)
#define KDRV_SIE_PXCLKPAD_COUNT     KDRV_SIE_ENG_COUNT

#define KDRV_SIE_ENG_BASE KDRV_VDOCAP_ENGINE0

/*
	chip_idx, eng_idx, minor_idx to idx mapping

	sample :
		hdl_idx_0  : chip_idx = 0, eng_idx = 0, minor_idx = 0
		hdl_idx_1  : chip_idx = 0, eng_idx = 0, minor_idx = 1
		hdl_idx_2  : chip_idx = 0, eng_idx = 1, minor_idx = 0
		hdl_idx_3  : chip_idx = 0, eng_idx = 1, minor_idx = 1
		hdl_idx_4  : chip_idx = 0, eng_idx = 2, minor_idx = 0
		hdl_idx_5  : chip_idx = 0, eng_idx = 2, minor_idx = 1
		hdl_idx_6  : chip_idx = 0, eng_idx = 3, minor_idx = 0
		hdl_idx_7  : chip_idx = 0, eng_idx = 3, minor_idx = 1
		hdl_idx_8  : chip_idx = 1, eng_idx = 0, minor_idx = 0
		hdl_idx_9  : chip_idx = 1, eng_idx = 0, minor_idx = 1
		hdl_idx_10 : chip_idx = 1, eng_idx = 1, minor_idx = 0
		hdl_idx_11 : chip_idx = 1, eng_idx = 1, minor_idx = 1
		hdl_idx_12 : chip_idx = 1, eng_idx = 2, minor_idx = 0
		hdl_idx_13 : chip_idx = 1, eng_idx = 2, minor_idx = 1
		hdl_idx_14 : chip_idx = 1, eng_idx = 3, minor_idx = 0
		hdl_idx_15 : chip_idx = 1, eng_idx = 3, minor_idx = 1

*/
#define KDRV_SIE_CHIP_IDX(chip)	(chip - KDRV_CHIP0)
#define KDRV_SIE_ENG_IDX(eng)	(eng - KDRV_SIE_ENG_BASE)
#define KDRV_SIE_CONV2_HDL_IDX(chip_idx, eng_idx, minor_idx) ((chip_idx * KDRV_SIE_ENG_COUNT * KDRV_SIE_MINOR_COUNT) + (eng_idx * KDRV_SIE_MINOR_COUNT) + minor_idx)
#define KDRV_SIE_CONV2_CHIP_IDX(hdl_idx) (hdl_idx / (KDRV_SIE_ENG_COUNT * KDRV_SIE_MINOR_COUNT))
#define KDRV_SIE_CONV2_ENG_IDX(hdl_idx) ((hdl_idx % (KDRV_SIE_ENG_COUNT * KDRV_SIE_MINOR_COUNT)) / KDRV_SIE_MINOR_COUNT)


typedef struct {
	/* [BEGIN] linux struct clk*, not used in freertos */
	void *clk;
	void *pclk;
	void *pxclk;
	void *intclk;
	void *pxclk_pad[KDRV_SIE_PXCLKPAD_COUNT];
	void *mclk[KDRV_SIE_MCLK_COUNT];
	/* [ END ] linux struct clk*, not used in freertos */

	void *p_io_addr;    /* virtual register address */
	ULONG phy_io_base;  /* physical register address */
	UINT32 phy_io_size; /* physical register size */
	INT32 irq_id;       /* interrupt id */

	void *reg_addr_va;	/* dram buffer for ssdrv register */
	void *flag_addr_va; /* dram buffer for ssdrv flag */
	UINT32 clk_src;		/* sie clock src sel for ssdrv, set by clock dtsi */
	UINT32 clk_rate;	/* sie clock rate for ssdrv, set by clock dtsi */
} KDRV_SIE_RESOURCE;

INT32 kdrv_sie_resource_init(void);
INT32 kdrv_sie_resource_uninit(void);
INT32 kdrv_sie_resource_set(KDRV_SIE_RESOURCE *p_resource, UINT32 idx);
KDRV_SIE_RESOURCE *kdrv_sie_resource_get(UINT32 idx);
#endif
