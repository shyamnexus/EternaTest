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
    {PAD_DS_PGPIO12, PAD_DS_PGPIO13,                                        // SDIO_PIN_CLK, SDIO_PIN_CMD
     PAD_DS_PGPIO15, PAD_DS_PGPIO16, PAD_DS_PGPIO17, PAD_DS_PGPIO18,        // SDIO_PINDATA0 ~ SDIO_PIN_DATA3
     SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED}                    // SDIO_PINDATA4 ~ SDIO_PIN_DATA7
};

// SDIO2 SDIO_PIN_TYPE_PAD_DS
static uint32_t nvt_mmcpinctl_pad_ds_2[SDIO2_PINMUX_NUM][SDIO_PIN_NUM] =
{
    {PAD_DS_CGPIO8, PAD_DS_CGPIO9,                                          // SDIO_PIN_CLK, SDIO_PIN_CMD
     PAD_DS_CGPIO10, PAD_DS_CGPIO11, PAD_DS_CGPIO12, PAD_DS_CGPIO13,        // SDIO_PINDATA0 ~ SDIO_PIN_DATA3
     PAD_DS_CGPIO2, PAD_DS_CGPIO3, PAD_DS_CGPIO4, PAD_DS_CGPIO5}            // SDIO_PINDATA4 ~ SDIO_PIN_DATA7
};

// SDIO1 SDIO_PIN_TYPE_PAD_PIN
static uint32_t nvt_mmcpinctl_pad_pin_1[SDIO1_PINMUX_NUM][SDIO_PIN_NUM] =
{
    {PAD_PIN_PGPIO12, PAD_PIN_PGPIO13,                                      // SDIO_PIN_CLK, SDIO_PIN_CMD
     PAD_PIN_PGPIO15, PAD_PIN_PGPIO16, PAD_PIN_PGPIO17, PAD_PIN_PGPIO18,    // SDIO_PINDATA0 ~ SDIO_PIN_DATA3
     SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED}                    // SDIO_PINDATA4 ~ SDIO_PIN_DATA7
};

// SDIO2 SDIO_PIN_TYPE_PAD_PIN
static uint32_t nvt_mmcpinctl_pad_pin_2[SDIO2_PINMUX_NUM][SDIO_PIN_NUM] =
{
    {PAD_PIN_CGPIO8, PAD_PIN_CGPIO9,                                        // SDIO_PIN_CLK, SDIO_PIN_CMD
     PAD_PIN_CGPIO10, PAD_PIN_CGPIO11, PAD_PIN_CGPIO12, PAD_PIN_CGPIO13,    // SDIO_PINDATA0 ~ SDIO_PIN_DATA3
     PAD_PIN_CGPIO2, PAD_PIN_CGPIO3, PAD_PIN_CGPIO4, PAD_PIN_CGPIO5}        // SDIO_PINDATA4 ~ SDIO_PIN_DATA7
};

// SDIO1 SDIO_PIN_TYPE_GPIO
static uint32_t nvt_mmcpinctl_gpio_val_1[SDIO1_PINMUX_NUM][SDIO_PIN_NUM] =
{
    {P_GPIO(12), P_GPIO(13),                                                // SDIO_PIN_CLK, SDIO_PIN_CMD
     P_GPIO(15), P_GPIO(16), P_GPIO(17), P_GPIO(18),                        // SDIO_PINDATA0 ~ SDIO_PIN_DATA3
     SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED}                    // SDIO_PINDATA4 ~ SDIO_PIN_DATA7
};

// SDIO2 SDIO_PIN_TYPE_GPIO
static uint32_t nvt_mmcpinctl_gpio_val_2[SDIO2_PINMUX_NUM][SDIO_PIN_NUM] =
{
    {C_GPIO(8), C_GPIO(9),                                                  // SDIO_PIN_CLK, SDIO_PIN_CMD
     C_GPIO(10), C_GPIO(11), C_GPIO(12), C_GPIO(13),                        // SDIO_PINDATA0 ~ SDIO_PIN_DATA3
     C_GPIO(2), C_GPIO(3), C_GPIO(4), C_GPIO(5)}                            // SDIO_PINDATA4 ~ SDIO_PIN_DATA7
};

uint32_t mmc_pinval_get(uint8_t host_id, uint8_t pin_type, uint32_t pinmux_cfg, uint8_t pin)
{
    uint32_t pinval = SDIO_UNUSED;
    uint8_t pinmux_id = 0;          // SDIO have only one pinmux in na51090

    switch (pin_type) {
    case SDIO_PIN_TYPE_PAD_DS:
        if (host_id == SDIO_HOST_ID_1) {
            pinval = nvt_mmcpinctl_pad_ds_1[pinmux_id][pin];
        }
        else if (host_id == SDIO_HOST_ID_2) {
            pinval = nvt_mmcpinctl_pad_ds_2[pinmux_id][pin];
        }

        break;

    case SDIO_PIN_TYPE_PAD_PIN:
        if (host_id == SDIO_HOST_ID_1) {
            pinval = nvt_mmcpinctl_pad_pin_1[pinmux_id][pin];
        }
        else if (host_id == SDIO_HOST_ID_2) {
            pinval = nvt_mmcpinctl_pad_pin_2[pinmux_id][pin];
        }

        break;

    case SDIO_PIN_TYPE_GPIO:
        if (host_id == SDIO_HOST_ID_1) {
            pinval = nvt_mmcpinctl_gpio_val_1[pinmux_id][pin];
        }
        else if (host_id == SDIO_HOST_ID_2) {
            pinval = nvt_mmcpinctl_gpio_val_2[pinmux_id][pin];
        }

        break;

    default:
        break;
    }

    return pinval;
}

EXPORT_SYMBOL(mmc_pinval_get);

