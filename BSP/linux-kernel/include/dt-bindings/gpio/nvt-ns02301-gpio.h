/*
 * This header provides constants for binding novatek,na*-gpio.
 *
 * The first cell in Novetek's GPIO specifier is the GPIO ID. The macros below
 * provide names for this.
 *
 * The second cell contains standard flag values specified in gpio.h.
 */

#ifndef _DT_BINDINGS_GPIO_NVT_NS02301_GPIO_H
#define _DT_BINDINGS_GPIO_NVT_NS02301_GPIO_H

#include <dt-bindings/gpio/gpio.h>

/* GPIO pin number translation */
#define NVT_GPIO_BASE_NUM    0xa0
//main part
#define C_GPIO(pin)                  (pin)
#define P_GPIO(pin)                  ((pin >= 25 && pin < 39) ? (pin + NVT_GPIO_BASE_NUM - 25) : ((pin == 39) ? (pin + 0x20 - 14) : (pin + 0x20)))
#define HSI_GPIO(pin)                (pin + 0x40)
#define D_GPIO(pin)                  (pin + 0x60)
#define INTERNAL_GPIO(pin)           (pin >= 1 ? ((pin - 1) + 0x60 + NVT_GPIO_BASE_NUM) : (pin + 0x80))

#define S_GPIO(pin)                  (pin + 0x20 + NVT_GPIO_BASE_NUM)
#define A_GPIO(pin)                  (pin + 0x40 + NVT_GPIO_BASE_NUM)

#define GPIO_RISING_EDGE             0x1
#define GPIO_FALLING_EDGE            0x0

#define GPIO_INTERRUPT               0x1
#define GPIO_POLLING                 0x0

#define GPIO_HIGH                    0x1
#define GPIO_LOW                     0x0

/* PAD pull pin number translation */
//main part
#define PAD_REG_TO_BASE(reg)         (((reg)/(4))*(32))
#define PAD_CGPIO(pin)               (2 * pin + PAD_REG_TO_BASE(0x00))
#define PAD_PGPIO(pin)               (2 * pin + PAD_REG_TO_BASE(0x08))
#define PAD_DGPIO(pin)               (2 * pin + PAD_REG_TO_BASE(0x14))
#define PAD_HSIGPIO(pin)             (2 * pin + PAD_REG_TO_BASE(0x1C))
//pre-roll
#define PAD_PRPGPIO(pin)             (2 * (pin-25) + PAD_REG_TO_BASE(0x00))
#define PAD_PRSGPIO(pin)             (2 * pin + PAD_REG_TO_BASE(0x08))
#define PAD_PRAGPIO(pin)             (2 * pin + PAD_REG_TO_BASE(0x14))
//LGPIO, DSIGPIO deprecated
#define PAD_LGPIO(pin)               (2 * pin + PAD_REG_TO_BASE(0x20))
#define PAD_DSIGPIO(pin)             (2 * pin + PAD_REG_TO_BASE(0x48))

#define PAD_NONE                     0x00
#define PAD_PULLDOWN                 0x01
#define PAD_PULLUP                   0x02
#define PAD_KEEPER                   0x03

/* PAD driving pin number translation */
//main part
#define PAD_DS_CGPIO(pin)            (2 * pin + PAD_REG_TO_BASE(0x24))
#define PAD_DS_PGPIO(pin)            (2 * pin + PAD_REG_TO_BASE(0x2C))
#define PAD_DS_DGPIO(pin)            (2 * pin + PAD_REG_TO_BASE(0x34))
#define PAD_DS_HSIGPIO(pin)          (2 * pin + PAD_REG_TO_BASE(0x3C))
//pre-roll
#define PAD_DS_PRPGPIO(pin)            (2 * pin + PAD_REG_TO_BASE(0x18))
#define PAD_DS_PRSGPIO(pin)          (2 * pin + PAD_REG_TO_BASE(0x20))
#define PAD_DS_PRAGPIO(pin)          (2 * pin + PAD_REG_TO_BASE(0x34))
//LGPIO, DSIGPIO deprecated
#define PAD_DS_DSIGPIO(pin)          (2 * pin + PAD_REG_TO_BASE(0x190))
#define PAD_DS_LGPIO(pin)            (2 * pin + PAD_REG_TO_BASE(0x140))

#define PAD_DS_L0                    0x00
#define PAD_DS_L1                    0x01
#define PAD_DS_L2                    0x02
#define PAD_DS_L3                    0x03
#define PAD_DS_L4                    0x04
#define PAD_DS_L5                    0x05
#define PAD_DS_L6                    0x06
#define PAD_DS_L7                    0x07
#define PAD_DS_L8                    0x08
#define PAD_DS_L9                    0x09
#define PAD_DS_L10                   0x0A
#define PAD_DS_L11                   0x0B
#define PAD_DS_L12                   0x0C
#define PAD_DS_L13                   0x0D
#define PAD_DS_L14                   0x0E
#define PAD_DS_L15                   0x0F

/* PAD power definition copied from pad.h */
#define PAD_POWERID_SN               0x10
#define PAD_POWERID_P1               0x20
#define PAD_POWERID_SD3              0x40

#define PAD_3P3V                     0x00
#define PAD_1P8V                     0x01

#endif
