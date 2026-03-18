/*
 * Copyright Novatek Inc
 *
 * SPDX-License-Identifier:      GPL-2.0+
 */

#ifndef __ARCH_GPIO_H
#define __ARCH_GPIO_H

#define NVT_GPIO_BASE_NUM            0xA0

//main part
#define C_GPIO(pin)                  (pin)
#define P_GPIO(pin)                  ((pin == 39) ? (0x39) : ((pin <= 24) ? (pin+0x20) : ((pin-25)+NVT_GPIO_BASE_NUM)))  //pgpio39 is open-drain ouput only pin
#define HSI_GPIO(pin)                (pin + 0x40)
#define D_GPIO(pin)                  (pin + 0x60)

//pre-roll part
#define S_GPIO(pin)                  (pin + 0xC0)
#define A_GPIO(pin)                  (pin + 0xE0)
#define INTERNAL_GPIO(pin)           (pin + 0x80)

//deprecated, not used in 567
#define DSI_GPIO(pin)                (pin)
#define L_GPIO(pin)                  (pin)

/*
 * Empty file - cmd_gpio.c requires this. The implementation
 * is in drivers/gpio/na51xxx_gpio.c instead of inlined here.
 */

#endif /* __ARCH_GPIO_H */


