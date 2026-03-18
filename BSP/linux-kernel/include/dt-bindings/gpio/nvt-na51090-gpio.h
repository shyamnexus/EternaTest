/*
 * This header provides constants for binding novatek,na*-gpio.
 *
 * The first cell in Novetek's GPIO specifier is the GPIO ID. The macros below
 * provide names for this.
 *
 * The second cell contains standard flag values specified in gpio.h.
 */

#ifndef _DT_BINDINGS_GPIO_NVT_GPIO_H
#define _DT_BINDINGS_GPIO_NVT_GPIO_H

#include <dt-bindings/gpio/gpio.h>

#define NVT_GPIO_BASE_NUM    0x160

/* na51090 platform */
#define C_GPIO(pin)                  (pin)
#define J_GPIO(pin)                  (pin + 0x20)
#define P_GPIO(pin)                  (pin + 0x40)
#define E_GPIO(pin)                  (pin + 0x80)
#define D_GPIO(pin)                  (pin + 0xA0)
#define S_GPIO(pin)                  (pin + 0xC0)
#define B_GPIO(pin)                  (pin + 0x120)

#define EP_C_GPIO(ep, pin)           ((ep+1)*NVT_GPIO_BASE_NUM + pin)
#define EP_J_GPIO(ep, pin)           ((ep+1)*NVT_GPIO_BASE_NUM + pin + 0x20)
#define EP_P_GPIO(ep, pin)           ((ep+1)*NVT_GPIO_BASE_NUM + pin + 0x40)
#define EP_E_GPIO(ep, pin)           ((ep+1)*NVT_GPIO_BASE_NUM + pin + 0x80)
#define EP_D_GPIO(ep, pin)           ((ep+1)*NVT_GPIO_BASE_NUM + pin + 0xA0)
#define EP_S_GPIO(ep, pin)           ((ep+1)*NVT_GPIO_BASE_NUM + pin + 0xC0)
#define EP_B_GPIO(ep, pin)           ((ep+1)*NVT_GPIO_BASE_NUM + pin + 0x120)

#define GPIO_RISING_EDGE             0x1
#define GPIO_FALLING_EDGE            0x0

#define GPIO_INTERRUPT               0x1
#define GPIO_POLLING                 0x0

#define GPIO_HIGH                    0x1
#define GPIO_LOW                     0x0
#endif
