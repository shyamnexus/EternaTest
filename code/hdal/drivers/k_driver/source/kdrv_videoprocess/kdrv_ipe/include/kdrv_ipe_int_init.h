#ifndef _KDRV_IPE_INT_INIT_H_
#define _KDRV_IPE_INT_INIT_H_

#include "kwrap/type.h"

#define NVT_KDRV_IPE_CHIP_COUNT		(1)
#define NVT_KDRV_IPE_MINOR_COUNT	(1)
#define NVT_KDRV_IPE_TOTAL_CH_COUNT	(NVT_KDRV_IPE_CHIP_COUNT * NVT_KDRV_IPE_MINOR_COUNT)
#define NVT_KDRV_IPE_NAME          "kdrv_ipe"

#define KDRV_IPE_ENG_IDX(eng) (eng - KDRV_VIDEOPROCS_IPE_ENGINE0)
#define KDRV_IPE_CONV2_HDL_IDX(chip, eng) ((g_kdrv_ipe_ctl.eng_num * KDRV_CHIP0) + KDRV_IPE_ENG_IDX(eng));

typedef struct {
	void *p_mclk;		/* clk handle for linux */
	void *p_io_addr;	/* virtual register address */
	ULONG phy_io_base;	/* physical register address */
	UINT32 phy_io_size;	/* physical register size */
	INT32 irq_id;		/* interrupt id */
} NVT_KDRV_IPE_RESOURCE;

INT32 kdrv_ipe_resource_init(void);
INT32 kdrv_ipe_resource_uninit(void);
INT32 kdrv_ipe_resource_set(NVT_KDRV_IPE_RESOURCE *p_resource, UINT32 idx);
NVT_KDRV_IPE_RESOURCE* kdrv_ipe_resource_get(UINT32 idx);
#endif
