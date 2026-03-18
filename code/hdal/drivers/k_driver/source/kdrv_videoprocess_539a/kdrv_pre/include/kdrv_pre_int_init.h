#ifndef _KDRV_PRE_INT_INIT_H_
#define _KDRV_PRE_INT_INIT_H_

#include "kwrap/type.h"

#include "kdrv_pre_int.h"

#define NVT_KDRV_PRE_CHIP_COUNT		(1)
#define NVT_KDRV_PRE_MINOR_COUNT	(1)
#define NVT_KDRV_PRE_TOTAL_CH_COUNT	(NVT_KDRV_PRE_CHIP_COUNT * NVT_KDRV_PRE_MINOR_COUNT)
#define NVT_KDRV_PRE_NAME          "kdrv_pre"

#define KDRV_PRE_ENG_IDX(eng) (eng - KDRV_VIDEOPROCS_PRE_ENGINE0)
#define KDRV_PRE_CONV2_HDL_IDX(chip, eng) ((g_kdrv_pre_ctl.eng_num * KDRV_CHIP0) + KDRV_PRE_ENG_IDX(eng));

typedef struct {
	void *p_mclk;		/* clk handle for linux */
	void *p_io_addr;	/* virtual register address */
	ULONG phy_io_base;	/* physical register address */
	UINT32 phy_io_size;	/* physical register size */
	INT32 irq_id;		/* interrupt id */
} NVT_KDRV_PRE_RESOURCE;

INT32 kdrv_pre_resource_init(void);
INT32 kdrv_pre_resource_uninit(void);
INT32 kdrv_pre_resource_set(NVT_KDRV_PRE_RESOURCE *p_resource, UINT32 idx);
NVT_KDRV_PRE_RESOURCE* kdrv_pre_resource_get(UINT32 idx);

#endif
