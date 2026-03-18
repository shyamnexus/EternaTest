/**
    NVT MMC pinmux control
    This file be a wapper to convert pinmux selection to pinmux value

    @file       nvt_mmcpinctl.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.

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
    {PAD_DS_CGPIO12, PAD_DS_CGPIO13,                                        // SDIO_PIN_CLK, SDIO_PIN_CMD
     PAD_DS_CGPIO14, PAD_DS_CGPIO15, PAD_DS_CGPIO16, PAD_DS_CGPIO17,        // SDIO_PINDATA0 ~ SDIO_PIN_DATA3
     SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED}                    // SDIO_PINDATA4 ~ SDIO_PIN_DATA7
};

// SDIO2 SDIO_PIN_TYPE_PAD_DS
static uint32_t nvt_mmcpinctl_pad_ds_2[SDIO2_PINMUX_NUM][SDIO_PIN_NUM] =
{
    {PAD_DS_CGPIO18, PAD_DS_CGPIO19,                                        // SDIO_PIN_CLK, SDIO_PIN_CMD
     PAD_DS_CGPIO20, PAD_DS_CGPIO21, PAD_DS_CGPIO22, PAD_DS_CGPIO23,        // SDIO_PINDATA0 ~ SDIO_PIN_DATA3
     SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED}                    // SDIO_PINDATA4 ~ SDIO_PIN_DATA7
};

// SDIO3 SDIO_PIN_TYPE_PAD_DS
static uint32_t nvt_mmcpinctl_pad_ds_3[SDIO3_PINMUX_NUM][SDIO_PIN_NUM] =
{
    {PAD_DS_CGPIO8, PAD_DS_CGPIO9,                                          // SDIO_PIN_CLK, SDIO_PIN_CMD
     PAD_DS_CGPIO0, PAD_DS_CGPIO1, PAD_DS_CGPIO2, PAD_DS_CGPIO3,            // SDIO_PINDATA0 ~ SDIO_PIN_DATA3
     PAD_DS_CGPIO4, PAD_DS_CGPIO5, PAD_DS_CGPIO6, PAD_DS_CGPIO7}            // SDIO_PINDATA4 ~ SDIO_PIN_DATA7
};

// SDIO1 SDIO_PIN_TYPE_PAD_PIN
static uint32_t nvt_mmcpinctl_pad_pin_1[SDIO1_PINMUX_NUM][SDIO_PIN_NUM] =
{
    {PAD_PIN_CGPIO12, PAD_PIN_CGPIO13,                                      // SDIO_PIN_CLK, SDIO_PIN_CMD
     PAD_PIN_CGPIO14, PAD_PIN_CGPIO15, PAD_PIN_CGPIO16, PAD_PIN_CGPIO17,    // SDIO_PINDATA0 ~ SDIO_PIN_DATA3
     SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED}                    // SDIO_PINDATA4 ~ SDIO_PIN_DATA7
};

// SDIO2 SDIO_PIN_TYPE_PAD_PIN
static uint32_t nvt_mmcpinctl_pad_pin_2[SDIO2_PINMUX_NUM][SDIO_PIN_NUM] =
{
    {PAD_PIN_CGPIO18, PAD_PIN_CGPIO19,                                      // SDIO_PIN_CLK, SDIO_PIN_CMD
     PAD_PIN_CGPIO20, PAD_PIN_CGPIO21, PAD_PIN_CGPIO22, PAD_PIN_CGPIO23,    // SDIO_PINDATA0 ~ SDIO_PIN_DATA3
     SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED}                    // SDIO_PINDATA4 ~ SDIO_PIN_DATA7
};

// SDIO3 SDIO_PIN_TYPE_PAD_PIN
static uint32_t nvt_mmcpinctl_pad_pin_3[SDIO3_PINMUX_NUM][SDIO_PIN_NUM] =
{
    {PAD_PIN_CGPIO8, PAD_PIN_CGPIO9,                                        // SDIO_PIN_CLK, SDIO_PIN_CMD
     PAD_PIN_CGPIO0, PAD_PIN_CGPIO1, PAD_PIN_CGPIO2, PAD_PIN_CGPIO3,        // SDIO_PINDATA0 ~ SDIO_PIN_DATA3
     PAD_PIN_CGPIO4, PAD_PIN_CGPIO5, PAD_PIN_CGPIO6, PAD_PIN_CGPIO7}        // SDIO_PINDATA4 ~ SDIO_PIN_DATA7
};

// SDIO1 SDIO_PIN_TYPE_GPIO
static uint32_t nvt_mmcpinctl_gpio_val_1[SDIO1_PINMUX_NUM][SDIO_PIN_NUM] =
{
    {C_GPIO(12), C_GPIO(13),                                                // SDIO_PIN_CLK, SDIO_PIN_CMD
     C_GPIO(14), C_GPIO(15), C_GPIO(16), C_GPIO(17),                        // SDIO_PINDATA0 ~ SDIO_PIN_DATA3
     SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED}                    // SDIO_PINDATA4 ~ SDIO_PIN_DATA7
};

// SDIO2 SDIO_PIN_TYPE_GPIO
static uint32_t nvt_mmcpinctl_gpio_val_2[SDIO2_PINMUX_NUM][SDIO_PIN_NUM] =
{
    {C_GPIO(18), C_GPIO(19),                                                // SDIO_PIN_CLK, SDIO_PIN_CMD
     C_GPIO(20), C_GPIO(21), C_GPIO(22), C_GPIO(23),                        // SDIO_PINDATA0 ~ SDIO_PIN_DATA3
     SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED, SDIO_UNUSED}                    // SDIO_PINDATA4 ~ SDIO_PIN_DATA7
};

// SDIO3 SDIO_PIN_TYPE_GPIO
static uint32_t nvt_mmcpinctl_gpio_val_3[SDIO3_PINMUX_NUM][SDIO_PIN_NUM] =
{
    {C_GPIO(8), C_GPIO(9),                                                  // SDIO_PIN_CLK, SDIO_PIN_CMD
     C_GPIO(0), C_GPIO(1), C_GPIO(2), C_GPIO(3),                            // SDIO_PINDATA0 ~ SDIO_PIN_DATA3
     C_GPIO(4), C_GPIO(5), C_GPIO(6), C_GPIO(7)}                            // SDIO_PINDATA4 ~ SDIO_PIN_DATA7
};

uint32_t mmc_pinval_get(uint8_t host_id, uint8_t pin_type, uint32_t pinmux_cfg, uint8_t pin)
{
    uint32_t pinval = SDIO_UNUSED;
    uint8_t pinmux_id = 0;          // SDIO have only one pinmux in ns02302

    switch (pin_type) {
    case SDIO_PIN_TYPE_PAD_DS:
        if (host_id == SDIO_HOST_ID_1) {
            pinval = nvt_mmcpinctl_pad_ds_1[pinmux_id][pin];
        }
        else if (host_id == SDIO_HOST_ID_2) {
            pinval = nvt_mmcpinctl_pad_ds_2[pinmux_id][pin];
        }
        else if (host_id == SDIO_HOST_ID_3) {
            pinval = nvt_mmcpinctl_pad_ds_3[pinmux_id][pin];
        }

        break;

    case SDIO_PIN_TYPE_PAD_PIN:
        if (host_id == SDIO_HOST_ID_1) {
            pinval = nvt_mmcpinctl_pad_pin_1[pinmux_id][pin];
        }
        else if (host_id == SDIO_HOST_ID_2) {
            pinval = nvt_mmcpinctl_pad_pin_2[pinmux_id][pin];
        }
        else if (host_id == SDIO_HOST_ID_3) {
            pinval = nvt_mmcpinctl_pad_pin_3[pinmux_id][pin];
        }

        break;

    case SDIO_PIN_TYPE_GPIO:
        if (host_id == SDIO_HOST_ID_1) {
            pinval = nvt_mmcpinctl_gpio_val_1[pinmux_id][pin];
        }
        else if (host_id == SDIO_HOST_ID_2) {
            pinval = nvt_mmcpinctl_gpio_val_2[pinmux_id][pin];
        }
        else if (host_id == SDIO_HOST_ID_3) {
            pinval = nvt_mmcpinctl_gpio_val_3[pinmux_id][pin];
        }

        break;

    default:
        break;
    }

    return pinval;
}

EXPORT_SYMBOL(mmc_pinval_get);

