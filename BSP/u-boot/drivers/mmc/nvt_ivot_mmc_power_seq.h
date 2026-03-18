
#ifndef __NVT_IVOT_MMC_POWER_H__
#define __NVT_IVOT_MMC_POWER_H__

#include "nvt_ivot_mmc_a64.h"
#include "nvt_ivot_mmcreg.h"
#include "nvt_ivot_mmcplat.h"
#include <linux/bitops.h>

#define PIN_SDIO_BUS_WIDTH 8 + 2

#ifdef PIN_SDIO_CFG_SDIO1_BUS_WIDTH
#define SDIO_PIN_MAX    8
#else
#define SDIO_PIN_MAX    4
#endif

#ifdef PIN_SDIO_CFG_SDIO2_BUS_WIDTH
#define SDIO2_PIN_MAX   8
#else
#define SDIO2_PIN_MAX   4
#endif

#ifdef PIN_SDIO_CFG_SDIO3_BUS_WIDTH
#define SDIO3_PIN_MAX   8
#else
#define SDIO3_PIN_MAX   4
#endif

// This define means all of nvt SDIO pinmux
// if this ic not define, pinmux becomes 0
#ifdef PIN_SDIO_CFG_SDIO_2
#define SDIO1_2_SUP PIN_SDIO_CFG_SDIO_2
#else
#define SDIO1_2_SUP 0
#endif
#ifdef PIN_SDIO_CFG_SDIO_3
#define SDIO1_3_SUP PIN_SDIO_CFG_SDIO_3
#else
#define SDIO1_3_SUP 0
#endif
#ifdef PIN_SDIO_CFG_SDIO2_1
#define SDIO2_1_SUP PIN_SDIO_CFG_SDIO2_1
#else
#define SDIO2_1_SUP 0
#endif
#ifdef PIN_SDIO_CFG_SDIO2_2
#define SDIO2_2_SUP PIN_SDIO_CFG_SDIO2_2
#else
#define SDIO2_2_SUP 0
#endif
#ifdef PIN_SDIO_CFG_SDIO3_1
#define SDIO3_1_SUP PIN_SDIO_CFG_SDIO3_1
#else
#define SDIO3_1_SUP 0
#endif

// This define means all of nvt ic support SDIO
typedef enum {
    SDIO1_1_EMB,
    SDIO1_2_EMB,
    SDIO1_3_EMB,
    SDIO2_1_EMB,
    SDIO2_2_EMB,
    SDIO3_1_EMB,
    SDIO_MAX_EMB
} SDIO_DEFINE;

typedef enum {
	SDIO_PIN_CLK,
	SDIO_PIN_CMD,
	SDIO_PIN_DATA0,
	SDIO_PIN_DATA1,
	SDIO_PIN_DATA2,
	SDIO_PIN_DATA3,
	SDIO_PIN_DATA4,
	SDIO_PIN_DATA5,
	SDIO_PIN_DATA6,
	SDIO_PIN_DATA7,
	SDIO_PIN_NUM,
} SDIO_PIN;

extern void nvt_mmc_pinmux_update(uintptr_t* pinmux_set, uintptr_t* width_set, u32 isWidth8, u32 value);
extern void nvt_pad_set_pull_updown(uintptr_t sdio_pad_reg, uintptr_t sdio_pad_mask, uintptr_t sdio_pad_shift, uintptr_t value);
extern void nvt_set_gpio_inout(u32 gpio_pin, u32 direction, u32 value);
extern void sdiohost_power_up(struct mmc_nvt_host *host);
extern void sdiohost_power_down(struct mmc_nvt_host *host);
extern void sdiohost_power_cycle(struct mmc_nvt_host *host, uint32_t delay_ms);

#endif