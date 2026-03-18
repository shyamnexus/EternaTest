/**
	NVT MMC header file
	This file will provide the MMC host controller data strcuture declaration
	@file       nvt-mmc.h
	@ingroup
	@note
	Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/
#ifndef __NVT_MMC_H
#define __NVT_MMC_H

#include <linux/types.h>
#include <linux/mmc/host.h>
#include <linux/bits.h>

enum {
	MMC_CTLR_VERSION_1 = 0,	/* DM644x and DM355 */
	MMC_CTLR_VERSION_2,	/* DA830 */
};

/*
    SDIO HW pin
*/
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

/*
    define SDIO pin type for nvt_mmcpinctl.c usage.
*/
typedef enum {
	SDIO_PIN_TYPE_PAD_DS,
	SDIO_PIN_TYPE_PAD_PIN,
	SDIO_PIN_TYPE_GPIO,
	SDIO_PIN_TYPE_NUM,
} SDIO_PIN_TYPE;

/*
    SDIO host number encoding.
*/
#define SDIO_HOST_ID_1                      (0)     /* SDIO host*/
#define SDIO_HOST_ID_2                      (1)     /* SDIO2 host*/
#define SDIO_HOST_ID_3                      (2)     /* SDIO3 host*/
#define SDIO_HOST_ID_COUNT                  (3)     /* SDIO host count*/

/*
    SDIO feature supported define
*/
#define SDIO1_SUPPORTED                 BIT(0)
#define SDIO2_SUPPORTED                 BIT(1)
#define SDIO3_SUPPORTED                 BIT(2)
#define SDIO_SUP_MASK                   (SDIO1_SUPPORTED | SDIO2_SUPPORTED | SDIO3_SUPPORTED)
#define SDIO_SUP_BUS_WIDTH_MASK         (SDIO3_SUPPORTED)
#define SDIO1_PINMUX_NUM                1
#define SDIO2_PINMUX_NUM                1
#define SDIO3_PINMUX_NUM                1

#define CHECK_SDIO_SUP(host_id)         ((SDIO_SUP_MASK & BIT(host_id)) != 0)
#define CHECK_SDIO_8_BITS_SUP(host_id)  ((SDIO_SUP_BUS_WIDTH_MASK & BIT(host_id)) != 0)

#define SDIO_UNUSED                     0

#define SDIO_PIN_NUM_4_BIT              (SDIO_PIN_DATA3 + 1)
#define SDIO_PIN_NUM_8_BIT              SDIO_PIN_NUM

#define SDIO_DELAY_PHASE_SEL_MAX        11
#define SDIO_DELAY_PHASE_UNIT_MAX       17


struct nvt_mmc_config {
	/* get_cd()/get_wp() may sleep */
	int	(*get_cd)(struct mmc_host *mmc);
	int	(*get_ro)(struct mmc_host *mmc);
	/* wires == 0 is equivalent to wires == 4 (4-bit parallel) */
	u8	wires;

	u32     max_freq;

	/* any additional host capabilities: OR'd in to mmc->f_caps */
	u32     caps;

	/* cd irq number */
	u32     cd_irq;

	/* Version of the MMC/SD controller */
	u8	version;

	/* Number of sg segments */
	u8	nr_sg;
};

static inline uint32_t nvt_mmc_get_indly(uint8_t host_id, uint32_t freq)
{
	return 0;
}

static inline uint32_t nvt_mmc_get_outdly(uint8_t host_id, uint32_t freq)
{
	if (host_id == SDIO_HOST_ID_1) {
		return 1;
	} else if (host_id == SDIO_HOST_ID_2) {
		return 2;
	} else {
		return 8;
	}
}

void nvt_setup_mmc(int module, struct nvt_mmc_config *config);

// nvt_mmcpinctl.c
uint32_t mmc_pinval_get(uint8_t host_id, uint8_t pin_type, uint32_t pinmux_cfg, uint8_t pin);

// fastboot_preload.c
int nvt_mmc_check_preload_finish(void);

#endif /* __ASM_ARCH_NA51000_NVT_MMC_H */
