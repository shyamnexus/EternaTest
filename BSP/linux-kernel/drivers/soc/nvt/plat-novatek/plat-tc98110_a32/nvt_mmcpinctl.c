/**
    NVT MMC pinmux control
    This file be a wapper to convert pinmux selection to pinmux value

    @file       nvt_mmcpinctl.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <plat/nvt-mmc.h>
#include <plat/top.h>
#include <plat/nvt-gpio.h>
#include <plat/pad.h>

// SDIO1 SDIO_PIN_TYPE_PAD_DS
static uint32_t nvt_mmcpinctl_pad_ds_1[SDIO1_PINMUX_NUM][SDIO_PIN_NUM] =
{
    // PIN_SDIO_CFG_1ST_PINMUX
    {PAD_DS_PGPIO18, PAD_DS_PGPIO17,                                        // SDIO_PIN_CLK, SDIO_PIN_CMD
     PAD_DS_PGPIO19, PAD_DS_PGPIO14, PAD_DS_PGPIO15, PAD_DS_PGPIO23,        // SDIO_PINDATA0 ~ SDIO_PIN_DATA3
     SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED},                   // SDIO_PINDATA4 ~ SDIO_PIN_DATA7
    // PIN_SDIO_CFG_2ND_PINMUX
    {PAD_DS_DGPIO7, PAD_DS_DGPIO6,
     PAD_DS_DGPIO8, PAD_DS_DGPIO9, PAD_DS_PGPIO20, PAD_DS_PGPIO21,
     SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED}
};

// SDIO1 SDIO_PIN_TYPE_PAD_PIN
static uint32_t nvt_mmcpinctl_pad_pin_1[SDIO1_PINMUX_NUM][SDIO_PIN_NUM] =
{
    // PIN_SDIO_CFG_1ST_PINMUX
    {SDIO_UNUSED, PAD_PIN_PGPIO17,                                          // SDIO_PIN_CLK, SDIO_PIN_CMD
     PAD_PIN_PGPIO19, PAD_PIN_PGPIO14, PAD_PIN_PGPIO15, PAD_PIN_PGPIO23,    // SDIO_PINDATA0 ~ SDIO_PIN_DATA3
     SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED},                   // SDIO_PINDATA4 ~ SDIO_PIN_DATA7
    // PIN_SDIO_CFG_2ND_PINMUX
    {SDIO_UNUSED, PAD_PIN_DGPIO6,
     PAD_PIN_DGPIO8, PAD_PIN_DGPIO9, PAD_PIN_PGPIO20, PAD_PIN_PGPIO21,
     SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED}
};

// SDIO1 SDIO_PIN_TYPE_GPIO
static uint32_t nvt_mmcpinctl_gpio_val_1[SDIO1_PINMUX_NUM][SDIO_PIN_NUM] =
{
    // PIN_SDIO_CFG_1ST_PINMUX
    {P_GPIO(18), P_GPIO(17),                                                // SDIO_PIN_CLK, SDIO_PIN_CMD
     P_GPIO(19), P_GPIO(14), P_GPIO(15), P_GPIO(23),                        // SDIO_PINDATA0 ~ SDIO_PIN_DATA3
     SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED},                   // SDIO_PINDATA4 ~ SDIO_PIN_DATA7
    // PIN_SDIO_CFG_2ND_PINMUX
    {D_GPIO(7), D_GPIO(6),
     D_GPIO(8), D_GPIO(9), P_GPIO(20), P_GPIO(21),
     SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED}
};

uint32_t mmc_pinval_get(uint8_t host_id, uint8_t pin_type, uint32_t pinmux_cfg, uint8_t pin)
{
    uint32_t pinval = SDIO_UNUSED;
    uint8_t pinmux_id = 0;

    pinmux_cfg &= PIN_SDIO_CFG_MASK(host_id);

    // get pinmux_id from pinmux_cfg
    if (host_id == SDIO_HOST_ID_1) {
        if (pinmux_cfg & PIN_SDIO_CFG_1ST_PINMUX) {
            pinmux_id = 0;
        }
        else if (pinmux_cfg & PIN_SDIO_CFG_2ND_PINMUX) {
            pinmux_id = 1;
        }
    }

    switch (pin_type) {
    case SDIO_PIN_TYPE_PAD_DS:
        if (host_id == SDIO_HOST_ID_1) {
            pinval = nvt_mmcpinctl_pad_ds_1[pinmux_id][pin];
        }

        break;

    case SDIO_PIN_TYPE_PAD_PIN:
        if (host_id == SDIO_HOST_ID_1) {
            pinval = nvt_mmcpinctl_pad_pin_1[pinmux_id][pin];
        }

        break;

    case SDIO_PIN_TYPE_GPIO:
        if (host_id == SDIO_HOST_ID_1) {
            pinval = nvt_mmcpinctl_gpio_val_1[pinmux_id][pin];
        }

        break;

    default:
        break;
    }

    return pinval;
}

EXPORT_SYMBOL(mmc_pinval_get);

