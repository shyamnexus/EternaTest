/**
    PAD controller header

    PAD controller header

    @file       pad.h
    @ingroup    mIDrvSys_PAD
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef _PAD_H
#define _PAD_H

#include <linux/soc/nvt/nvt_type.h>

/*********** MAIN PAD ***********/
// PAD Pull UP/DOWN Register 0
#define PAD_PUPD0_REG_OFS                    0x00

#define PAD_PWR0_REG_OFS                     0x78

#define PAD_REG_TO_BASE(reg) (((reg)/(4))*(32))

#define PAD_CGPIO_BASE           PAD_REG_TO_BASE(0x00)      // 0x00~0x04
#define PAD_PGPIO_BASE           PAD_REG_TO_BASE(0x08)      // 0x08, 0x10
#define PAD_DGPIO_BASE           PAD_REG_TO_BASE(0x14)      // 0x14
#define PAD_HSIGPIO_BASE         PAD_REG_TO_BASE(0x1C)      // 0x1C

/* 4 level */
#define PAD_4L_DS_CGPIO_BASE     PAD_REG_TO_BASE(0x24)      // 0x24~0x28
#define PAD_4L_DS_PGPIO_BASE     PAD_REG_TO_BASE(0x2C)      // 0x2C~0x30
#define PAD_4L_DS_DGPIO_BASE     PAD_REG_TO_BASE(0x34)      // 0x34
#define PAD_4L_DS_HSIGPIO_BASE   PAD_REG_TO_BASE(0x3C)      // 0x3C

/* 8 level */
#define PAD_8L_DS_CGPIO_BASE     PAD_REG_TO_BASE(0x44)      // 0x44

/* 2 level */
#define PAD_2L_DS_HSIGPIO_BASE   PAD_REG_TO_BASE(0x48)      // 0x48

/*********** PR PAD ***********/
#define PR_PAD_PUPD0_REG_OFS                    0x00

#define PR_PAD_REG_TO_BASE(reg) (((reg)/(4))*(32))

#define PR_PAD_PGPIO_BASE        PR_PAD_REG_TO_BASE(0x00)      // 0x00
#define PR_PAD_SGPIO_BASE        PR_PAD_REG_TO_BASE(0x08)      // 0x08, 0x10
#define PR_PAD_AGPIO_BASE        PR_PAD_REG_TO_BASE(0x14)      // 0x14

/* 4 level */
#define PR_PAD_4L_DS_PGPIO_BASE  PR_PAD_REG_TO_BASE(0x18)      // 0x18
#define PR_PAD_4L_DS_SGPIO_BASE  PR_PAD_REG_TO_BASE(0x20)      // 0x20
#define PR_PAD_4L_DS_AGPIO_BASE  PR_PAD_REG_TO_BASE(0x34)      // 0x34

/* 2 level */
#define PR_PAD_2L_DS_SGPIO_BASE  PR_PAD_REG_TO_BASE(0x38)      // 0x38

#define PR_PREFIX 0x400

typedef enum {
	PAD_NONE                    =    0x00,      ///< none of pull up/down
	PAD_PULLDOWN                =    0x01,      ///< pull down
	PAD_PULLUP                  =    0x02,      ///< pull up
	PAD_KEEPER                  =    0x03,      ///< keeper
	PAD_STRONG_PULLUP           =    0x03,      ///< strong pull up

	ENUM_DUMMY4WORD(PAD_PULL)
} PAD_PULL;

typedef enum {
	PAD_DRIVINGSINK_LEVEL_0     =    0x00,
	PAD_DRIVINGSINK_LEVEL_1     =    0x01,
	PAD_DRIVINGSINK_LEVEL_2     =    0x02,
	PAD_DRIVINGSINK_LEVEL_3     =    0x03,
	PAD_DRIVINGSINK_LEVEL_4     =    0x04,
	PAD_DRIVINGSINK_LEVEL_5     =    0x05,
	PAD_DRIVINGSINK_LEVEL_6     =    0x06,
	PAD_DRIVINGSINK_LEVEL_7     =    0x07,
	PAD_DRIVINGSINK_LEVEL_8     =    0x08,
	PAD_DRIVINGSINK_LEVEL_9     =    0x09,
	PAD_DRIVINGSINK_LEVEL_10    =    0x0A,
	PAD_DRIVINGSINK_LEVEL_11    =    0x0B,
	PAD_DRIVINGSINK_LEVEL_12    =    0x0C,
	PAD_DRIVINGSINK_LEVEL_13    =    0x0D,
	PAD_DRIVINGSINK_LEVEL_14    =    0x0E,
	PAD_DRIVINGSINK_LEVEL_15    =    0x0F,

	ENUM_DUMMY4WORD(PAD_DRIVINGSINK)
} PAD_DRIVINGSINK;

typedef enum {
	// CGPIO group
	PAD_PIN_CGPIO0      = (PAD_CGPIO_BASE + 0),        ///< C_GPIO_0
	PAD_PIN_CGPIO1      = (PAD_CGPIO_BASE + 2),        ///< C_GPIO_1
	PAD_PIN_CGPIO2      = (PAD_CGPIO_BASE + 4),        ///< C_GPIO_2
	PAD_PIN_CGPIO3      = (PAD_CGPIO_BASE + 6),        ///< C_GPIO_3
	PAD_PIN_CGPIO4      = (PAD_CGPIO_BASE + 8),        ///< C_GPIO_4
	PAD_PIN_CGPIO5      = (PAD_CGPIO_BASE + 10),       ///< C_GPIO_5
	PAD_PIN_CGPIO6      = (PAD_CGPIO_BASE + 12),       ///< C_GPIO_6
	PAD_PIN_CGPIO7      = (PAD_CGPIO_BASE + 14),       ///< C_GPIO_7
	PAD_PIN_CGPIO8      = (PAD_CGPIO_BASE + 16),       ///< C_GPIO_8
	PAD_PIN_CGPIO9      = (PAD_CGPIO_BASE + 18),       ///< C_GPIO_9
	PAD_PIN_CGPIO10     = (PAD_CGPIO_BASE + 20),       ///< C_GPIO_10
	PAD_PIN_CGPIO11     = (PAD_CGPIO_BASE + 22),       ///< C_GPIO_11
	PAD_PIN_CGPIO12     = (PAD_CGPIO_BASE + 24),       ///< C_GPIO_12
	PAD_PIN_CGPIO13     = (PAD_CGPIO_BASE + 26),       ///< C_GPIO_13
	PAD_PIN_CGPIO14     = (PAD_CGPIO_BASE + 28),       ///< C_GPIO_14
	PAD_PIN_CGPIO15     = (PAD_CGPIO_BASE + 30),       ///< C_GPIO_15
	PAD_PIN_CGPIO16     = (PAD_CGPIO_BASE + 32),       ///< C_GPIO_16
	PAD_PIN_CGPIO17     = (PAD_CGPIO_BASE + 34),       ///< C_GPIO_17
	PAD_PIN_CGPIO18     = (PAD_CGPIO_BASE + 36),       ///< C_GPIO_18
	PAD_PIN_CGPIO19     = (PAD_CGPIO_BASE + 38),       ///< C_GPIO_19
	PAD_PIN_CGPIO20     = (PAD_CGPIO_BASE + 40),       ///< C_GPIO_20
	PAD_PIN_CGPIO21     = (PAD_CGPIO_BASE + 42),       ///< C_GPIO_21
	PAD_PIN_CGPIO22     = (PAD_CGPIO_BASE + 44),       ///< C_GPIO_22
	PAD_PIN_CGPIO23     = (PAD_CGPIO_BASE + 46),       ///< C_GPIO_23
	PAD_PIN_CGPIO24     = (PAD_CGPIO_BASE + 48),       ///< C_GPIO_24

	// PGPIO group
	PAD_PIN_PGPIO0      = (PAD_PGPIO_BASE + 0),        ///< P_GPIO_0
	PAD_PIN_PGPIO1      = (PAD_PGPIO_BASE + 2),        ///< P_GPIO_1
	PAD_PIN_PGPIO2      = (PAD_PGPIO_BASE + 4),        ///< P_GPIO_2
	PAD_PIN_PGPIO3      = (PAD_PGPIO_BASE + 6),        ///< P_GPIO_3
	PAD_PIN_PGPIO4      = (PAD_PGPIO_BASE + 8),        ///< P_GPIO_4
	PAD_PIN_PGPIO5      = (PAD_PGPIO_BASE + 10),       ///< P_GPIO_5
	PAD_PIN_PGPIO6      = (PAD_PGPIO_BASE + 12),       ///< P_GPIO_6
	PAD_PIN_PGPIO7      = (PAD_PGPIO_BASE + 14),       ///< P_GPIO_7
	PAD_PIN_PGPIO8      = (PAD_PGPIO_BASE + 16),       ///< P_GPIO_8
	PAD_PIN_PGPIO9      = (PAD_PGPIO_BASE + 18),       ///< P_GPIO_9
	PAD_PIN_PGPIO10     = (PAD_PGPIO_BASE + 20),       ///< P_GPIO_10
	PAD_PIN_PGPIO11     = (PAD_PGPIO_BASE + 22),       ///< P_GPIO_11
	PAD_PIN_PGPIO12     = (PAD_PGPIO_BASE + 24),       ///< P_GPIO_12
	PAD_PIN_PGPIO13     = (PAD_PGPIO_BASE + 26),       ///< P_GPIO_13
	PAD_PIN_PGPIO14     = (PAD_PGPIO_BASE + 28),       ///< P_GPIO_14
	PAD_PIN_PGPIO15     = (PAD_PGPIO_BASE + 30),       ///< P_GPIO_15
	PAD_PIN_PGPIO16     = (PAD_PGPIO_BASE + 32 + 32),  ///< P_GPIO_16 /* 567 proposal have 32b shift from PGPIO15 to PGPIO16 */
	PAD_PIN_PGPIO17     = (PAD_PGPIO_BASE + 32 + 34),  ///< P_GPIO_17
	PAD_PIN_PGPIO18     = (PAD_PGPIO_BASE + 32 + 36),  ///< P_GPIO_18
	PAD_PIN_PGPIO19     = (PAD_PGPIO_BASE + 32 + 38),  ///< P_GPIO_19
	PAD_PIN_PGPIO20     = (PAD_PGPIO_BASE + 32 + 40),  ///< P_GPIO_20
	PAD_PIN_PGPIO21     = (PAD_PGPIO_BASE + 32 + 42),  ///< P_GPIO_21
	PAD_PIN_PGPIO22     = (PAD_PGPIO_BASE + 32 + 44),  ///< P_GPIO_22
	PAD_PIN_PGPIO23     = (PAD_PGPIO_BASE + 32 + 46),  ///< P_GPIO_23
	PAD_PIN_PGPIO24     = (PAD_PGPIO_BASE + 32 + 48),  ///< P_GPIO_24
	PAD_PIN_PGPIO39     = (PAD_PGPIO_BASE + 32 + 50),  ///< P_GPIO_39

	// DGPIO group
	PAD_PIN_DGPIO0      = (PAD_DGPIO_BASE + 0),        ///< D_GPIO_0
	PAD_PIN_DGPIO1      = (PAD_DGPIO_BASE + 2),        ///< D_GPIO_1
	PAD_PIN_DGPIO2      = (PAD_DGPIO_BASE + 4),        ///< D_GPIO_2
	PAD_PIN_DGPIO3      = (PAD_DGPIO_BASE + 6),        ///< D_GPIO_3
	PAD_PIN_DGPIO4      = (PAD_DGPIO_BASE + 8),        ///< D_GPIO_4
	PAD_PIN_DGPIO5      = (PAD_DGPIO_BASE + 10),       ///< D_GPIO_5
	PAD_PIN_DGPIO6      = (PAD_DGPIO_BASE + 12),       ///< D_GPIO_6
	PAD_PIN_DGPIO7      = (PAD_DGPIO_BASE + 14),       ///< D_GPIO_7
	PAD_PIN_SYS_RST     = (PAD_DGPIO_BASE + 30),       ///< SYS_RST

	// HSIGPIO group
	PAD_PIN_HSIGPIO0      = (PAD_HSIGPIO_BASE + 0),        ///< HSI_GPIO_0
	PAD_PIN_HSIGPIO1      = (PAD_HSIGPIO_BASE + 2),        ///< HSI_GPIO_1
	PAD_PIN_HSIGPIO2      = (PAD_HSIGPIO_BASE + 4),        ///< HSI_GPIO_2
	PAD_PIN_HSIGPIO3      = (PAD_HSIGPIO_BASE + 6),        ///< HSI_GPIO_3
	PAD_PIN_HSIGPIO4      = (PAD_HSIGPIO_BASE + 8),        ///< HSI_GPIO_4
	PAD_PIN_HSIGPIO5      = (PAD_HSIGPIO_BASE + 10),       ///< HSI_GPIO_5
	PAD_PIN_HSIGPIO6      = (PAD_HSIGPIO_BASE + 12),       ///< HSI_GPIO_6
	PAD_PIN_HSIGPIO7      = (PAD_HSIGPIO_BASE + 14),       ///< HSI_GPIO_7
	PAD_PIN_HSIGPIO8      = (PAD_HSIGPIO_BASE + 16),       ///< HSI_GPIO_8
	PAD_PIN_HSIGPIO9      = (PAD_HSIGPIO_BASE + 18),       ///< HSI_GPIO_9
	PAD_PIN_HSIGPIO10     = (PAD_HSIGPIO_BASE + 20),       ///< HSI_GPIO_10
	PAD_PIN_HSIGPIO11     = (PAD_HSIGPIO_BASE + 22),       ///< HSI_GPIO_11


	PAD_PIN_MAX         =     PAD_PIN_HSIGPIO11, //PR_FIX

	// PR-PGPIO group
	PAD_PIN_PGPIO25     = (PR_PREFIX + PR_PAD_PGPIO_BASE + 0),        ///< P_GPIO_25
	PAD_PIN_PGPIO26     = (PR_PREFIX + PR_PAD_PGPIO_BASE + 2),        ///< P_GPIO_26
	PAD_PIN_PGPIO27     = (PR_PREFIX + PR_PAD_PGPIO_BASE + 4),        ///< P_GPIO_27
	PAD_PIN_PGPIO28     = (PR_PREFIX + PR_PAD_PGPIO_BASE + 6),        ///< P_GPIO_28
	PAD_PIN_PGPIO29     = (PR_PREFIX + PR_PAD_PGPIO_BASE + 8),        ///< P_GPIO_29
	PAD_PIN_PGPIO30     = (PR_PREFIX + PR_PAD_PGPIO_BASE + 10),        ///< P_GPIO_30
	PAD_PIN_PGPIO31     = (PR_PREFIX + PR_PAD_PGPIO_BASE + 12),        ///< P_GPIO_31
	PAD_PIN_PGPIO32     = (PR_PREFIX + PR_PAD_PGPIO_BASE + 14),        ///< P_GPIO_32
	PAD_PIN_PGPIO33     = (PR_PREFIX + PR_PAD_PGPIO_BASE + 16),        ///< P_GPIO_33
	PAD_PIN_PGPIO34     = (PR_PREFIX + PR_PAD_PGPIO_BASE + 18),        ///< P_GPIO_34
	PAD_PIN_PGPIO35     = (PR_PREFIX + PR_PAD_PGPIO_BASE + 20),        ///< P_GPIO_35
	PAD_PIN_PGPIO36     = (PR_PREFIX + PR_PAD_PGPIO_BASE + 22),        ///< P_GPIO_36
	PAD_PIN_PGPIO37     = (PR_PREFIX + PR_PAD_PGPIO_BASE + 24),        ///< P_GPIO_37
	PAD_PIN_PGPIO38     = (PR_PREFIX + PR_PAD_PGPIO_BASE + 26),        ///< P_GPIO_38

	// PR-SGPIO group
	PAD_PIN_SGPIO0      = (PR_PREFIX + PR_PAD_SGPIO_BASE + 0),        ///< S_GPIO_0
	PAD_PIN_SGPIO1      = (PR_PREFIX + PR_PAD_SGPIO_BASE + 2),        ///< S_GPIO_1
	PAD_PIN_SGPIO2      = (PR_PREFIX + PR_PAD_SGPIO_BASE + 4),        ///< S_GPIO_2
	PAD_PIN_SGPIO3      = (PR_PREFIX + PR_PAD_SGPIO_BASE + 6),        ///< S_GPIO_3
	PAD_PIN_SGPIO4      = (PR_PREFIX + PR_PAD_SGPIO_BASE + 8),        ///< S_GPIO_4
	PAD_PIN_SGPIO5      = (PR_PREFIX + PR_PAD_SGPIO_BASE + 10),       ///< S_GPIO_5
	PAD_PIN_SGPIO6      = (PR_PREFIX + PR_PAD_SGPIO_BASE + 12),       ///< S_GPIO_6
	PAD_PIN_SGPIO7      = (PR_PREFIX + PR_PAD_SGPIO_BASE + 14),       ///< S_GPIO_7
	PAD_PIN_SGPIO8      = (PR_PREFIX + PR_PAD_SGPIO_BASE + 16),       ///< S_GPIO_8
	PAD_PIN_SGPIO9      = (PR_PREFIX + PR_PAD_SGPIO_BASE + 18),       ///< S_GPIO_9
	PAD_PIN_SGPIO10     = (PR_PREFIX + PR_PAD_SGPIO_BASE + 20),       ///< S_GPIO_10
	PAD_PIN_SGPIO11     = (PR_PREFIX + PR_PAD_SGPIO_BASE + 22),       ///< S_GPIO_11
	PAD_PIN_SGPIO12     = (PR_PREFIX + PR_PAD_SGPIO_BASE + 24),       ///< S_GPIO_12
	PAD_PIN_SGPIO13     = (PR_PREFIX + PR_PAD_SGPIO_BASE + 26),       ///< S_GPIO_13
	PAD_PIN_SGPIO14     = (PR_PREFIX + PR_PAD_SGPIO_BASE + 28),       ///< S_GPIO_14
	PAD_PIN_SGPIO15     = (PR_PREFIX + PR_PAD_SGPIO_BASE + 30),       ///< S_GPIO_15
	PAD_PIN_SGPIO16     = (PR_PREFIX + PR_PAD_SGPIO_BASE + 32 + 32),  ///< P_GPIO_16 /* 567 proposal have 32b shift from SGPIO15 to SGPIO16 */

	// PR-AGPIO group
	PAD_PIN_AGPIO0      = (PR_PREFIX + PR_PAD_AGPIO_BASE + 0),        ///< A_GPIO_0
	PAD_PIN_AGPIO1      = (PR_PREFIX + PR_PAD_AGPIO_BASE + 2),        ///< A_GPIO_1
	PAD_PIN_AGPIO2      = (PR_PREFIX + PR_PAD_AGPIO_BASE + 4),        ///< A_GPIO_2
	PAD_PIN_AGPIO3      = (PR_PREFIX + PR_PAD_AGPIO_BASE + 6),        ///< A_GPIO_3

	PR_PAD_PIN_MAX      =   PAD_PIN_AGPIO3, //PR_FIX

	ENUM_DUMMY4WORD(PAD_PIN)
} PAD_PIN;

typedef enum {
	// CGPIO group
	PAD_DS_CGPIO0      = (PAD_4L_DS_CGPIO_BASE + 0),        ///< C_GPIO_0
	PAD_DS_CGPIO1      = (PAD_4L_DS_CGPIO_BASE + 2),        ///< C_GPIO_1
	PAD_DS_CGPIO2      = (PAD_4L_DS_CGPIO_BASE + 4),        ///< C_GPIO_2
	PAD_DS_CGPIO3      = (PAD_4L_DS_CGPIO_BASE + 6),       ///< C_GPIO_3
	PAD_DS_CGPIO4      = (PAD_4L_DS_CGPIO_BASE + 8),       ///< C_GPIO_4
	PAD_DS_CGPIO5      = (PAD_4L_DS_CGPIO_BASE + 10),       ///< C_GPIO_5
	PAD_DS_CGPIO6      = (PAD_4L_DS_CGPIO_BASE + 12),       ///< C_GPIO_6
	PAD_DS_CGPIO7      = (PAD_4L_DS_CGPIO_BASE + 14),       ///< C_GPIO_7
	PAD_DS_CGPIO8      = (PAD_4L_DS_CGPIO_BASE + 16),       ///< C_GPIO_8
	PAD_DS_CGPIO9      = (PAD_4L_DS_CGPIO_BASE + 18),       ///< C_GPIO_9
	PAD_DS_CGPIO10     = (PAD_4L_DS_CGPIO_BASE + 20),       ///< C_GPIO_10
	PAD_DS_CGPIO11     = (PAD_4L_DS_CGPIO_BASE + 22),       ///< C_GPIO_11
	PAD_DS_CGPIO12     = (PAD_4L_DS_CGPIO_BASE + 24),       ///< C_GPIO_12
	PAD_DS_CGPIO13     = (PAD_8L_DS_CGPIO_BASE + 0),       ///< C_GPIO_13
	PAD_DS_CGPIO14     = (PAD_8L_DS_CGPIO_BASE + 4),       ///< C_GPIO_14
	PAD_DS_CGPIO15     = (PAD_8L_DS_CGPIO_BASE + 8),       ///< C_GPIO_15
	PAD_DS_CGPIO16     = (PAD_8L_DS_CGPIO_BASE + 12),       ///< C_GPIO_16
	PAD_DS_CGPIO17     = (PAD_8L_DS_CGPIO_BASE + 16),       ///< C_GPIO_17
	PAD_DS_CGPIO18     = (PAD_8L_DS_CGPIO_BASE + 20),       ///< C_GPIO_18
	PAD_DS_CGPIO19     = (PAD_8L_DS_CGPIO_BASE + 24),       ///< C_GPIO_19
	PAD_DS_CGPIO20     = (PAD_4L_DS_CGPIO_BASE + 40),       ///< C_GPIO_20
	PAD_DS_CGPIO21     = (PAD_4L_DS_CGPIO_BASE + 42),       ///< C_GPIO_21
	PAD_DS_CGPIO22     = (PAD_4L_DS_CGPIO_BASE + 44),       ///< C_GPIO_22
	PAD_DS_CGPIO23     = (PAD_4L_DS_CGPIO_BASE + 46),       ///< C_GPIO_23
	PAD_DS_CGPIO24     = (PAD_4L_DS_CGPIO_BASE + 48),       ///< C_GPIO_24

	// PGPIO group
	PAD_DS_PGPIO0      = (PAD_4L_DS_PGPIO_BASE + 0),        ///< P_GPIO_0
	PAD_DS_PGPIO1      = (PAD_4L_DS_PGPIO_BASE + 2),        ///< P_GPIO_1
	PAD_DS_PGPIO2      = (PAD_4L_DS_PGPIO_BASE + 4),        ///< P_GPIO_2
	PAD_DS_PGPIO3      = (PAD_4L_DS_PGPIO_BASE + 6),       ///< P_GPIO_3
	PAD_DS_PGPIO4      = (PAD_4L_DS_PGPIO_BASE + 8),       ///< P_GPIO_4
	PAD_DS_PGPIO5      = (PAD_4L_DS_PGPIO_BASE + 10),       ///< P_GPIO_5
	PAD_DS_PGPIO6      = (PAD_4L_DS_PGPIO_BASE + 12),       ///< P_GPIO_6
	PAD_DS_PGPIO7      = (PAD_4L_DS_PGPIO_BASE + 14),       ///< P_GPIO_7
	PAD_DS_PGPIO8      = (PAD_4L_DS_PGPIO_BASE + 16),       ///< P_GPIO_8
	PAD_DS_PGPIO9      = (PAD_4L_DS_PGPIO_BASE + 18),       ///< P_GPIO_9
	PAD_DS_PGPIO10     = (PAD_4L_DS_PGPIO_BASE + 20),       ///< P_GPIO_10
	PAD_DS_PGPIO11     = (PAD_4L_DS_PGPIO_BASE + 22),       ///< P_GPIO_11
	PAD_DS_PGPIO12     = (PAD_4L_DS_PGPIO_BASE + 24),       ///< P_GPIO_12
	PAD_DS_PGPIO13     = (PAD_4L_DS_PGPIO_BASE + 26),       ///< P_GPIO_13
	PAD_DS_PGPIO14     = (PAD_4L_DS_PGPIO_BASE + 28),       ///< P_GPIO_14
	PAD_DS_PGPIO15     = (PAD_4L_DS_PGPIO_BASE + 30),       ///< P_GPIO_15
	PAD_DS_PGPIO16     = (PAD_4L_DS_PGPIO_BASE + 32),       ///< P_GPIO_16
	PAD_DS_PGPIO17     = (PAD_4L_DS_PGPIO_BASE + 34),       ///< P_GPIO_17
	PAD_DS_PGPIO18     = (PAD_4L_DS_PGPIO_BASE + 36),       ///< P_GPIO_18
	PAD_DS_PGPIO19     = (PAD_4L_DS_PGPIO_BASE + 38),       ///< P_GPIO_19
	PAD_DS_PGPIO20     = (PAD_4L_DS_PGPIO_BASE + 40),       ///< P_GPIO_20
	PAD_DS_PGPIO21     = (PAD_4L_DS_PGPIO_BASE + 42),       ///< P_GPIO_21
	PAD_DS_PGPIO22     = (PAD_4L_DS_PGPIO_BASE + 44),       ///< P_GPIO_22
	PAD_DS_PGPIO23     = (PAD_4L_DS_PGPIO_BASE + 46),       ///< P_GPIO_23
	PAD_DS_PGPIO24     = (PAD_4L_DS_PGPIO_BASE + 48),       ///< P_GPIO_24
	PAD_DS_PGPIO39     = (PAD_4L_DS_PGPIO_BASE + 50),       ///< P_GPIO_39

	// DGPIO group
	PAD_DS_DGPIO0      = (PAD_4L_DS_DGPIO_BASE + 0),        ///< D_GPIO_0
	PAD_DS_DGPIO1      = (PAD_4L_DS_DGPIO_BASE + 2),        ///< D_GPIO_1
	PAD_DS_DGPIO2      = (PAD_4L_DS_DGPIO_BASE + 4),        ///< D_GPIO_2
	PAD_DS_DGPIO3      = (PAD_4L_DS_DGPIO_BASE + 6),       ///< D_GPIO_3
	PAD_DS_DGPIO4      = (PAD_4L_DS_DGPIO_BASE + 8),       ///< D_GPIO_4
	PAD_DS_DGPIO5      = (PAD_4L_DS_DGPIO_BASE + 10),       ///< D_GPIO_5
	PAD_DS_DGPIO6      = (PAD_4L_DS_DGPIO_BASE + 12),       ///< D_GPIO_6
	PAD_DS_DGPIO7      = (PAD_4L_DS_DGPIO_BASE + 14),       ///< D_GPIO_7
	PAD_DS_SYS_RST     = (PAD_4L_DS_DGPIO_BASE + 28),       ///< SYS_RST

	// HSIGPIO group
	PAD_DS_HSIGPIO0      = (PAD_2L_DS_HSIGPIO_BASE + 0),        ///< HSI_GPIO_0
	PAD_DS_HSIGPIO1      = (PAD_2L_DS_HSIGPIO_BASE + 1),        ///< HSI_GPIO_1
	PAD_DS_HSIGPIO2      = (PAD_2L_DS_HSIGPIO_BASE + 2),        ///< HSI_GPIO_2
	PAD_DS_HSIGPIO3      = (PAD_2L_DS_HSIGPIO_BASE + 3),       ///< HSI_GPIO_3
	PAD_DS_HSIGPIO4      = (PAD_2L_DS_HSIGPIO_BASE + 4),       ///< HSI_GPIO_4
	PAD_DS_HSIGPIO5      = (PAD_2L_DS_HSIGPIO_BASE + 5),       ///< HSI_GPIO_5
	PAD_DS_HSIGPIO6      = (PAD_2L_DS_HSIGPIO_BASE + 6),       ///< HSI_GPIO_6
	PAD_DS_HSIGPIO7      = (PAD_2L_DS_HSIGPIO_BASE + 7),       ///< HSI_GPIO_7
	PAD_DS_HSIGPIO8      = (PAD_2L_DS_HSIGPIO_BASE + 8),       ///< HSI_GPIO_8
	PAD_DS_HSIGPIO9      = (PAD_2L_DS_HSIGPIO_BASE + 9),       ///< HSI_GPIO_9
	PAD_DS_HSIGPIO10     = (PAD_4L_DS_HSIGPIO_BASE + 20),       ///< HSI_GPIO_10
	PAD_DS_HSIGPIO11     = (PAD_4L_DS_HSIGPIO_BASE + 22),       ///< HSI_GPIO_11

	PAD_DS_MAX = PAD_DS_HSIGPIO9, // HSIGPIO9 2level case is the biggest reg in enum


	// PR-PGPIO group
	PAD_DS_PGPIO25      = (PR_PREFIX + PR_PAD_4L_DS_PGPIO_BASE + 0),        ///< P_GPIO_25
	PAD_DS_PGPIO26      = (PR_PREFIX + PR_PAD_4L_DS_PGPIO_BASE + 2),        ///< P_GPIO_26
	PAD_DS_PGPIO27      = (PR_PREFIX + PR_PAD_4L_DS_PGPIO_BASE + 4),        ///< P_GPIO_27
	PAD_DS_PGPIO28      = (PR_PREFIX + PR_PAD_4L_DS_PGPIO_BASE + 6),        ///< P_GPIO_28
	PAD_DS_PGPIO29      = (PR_PREFIX + PR_PAD_4L_DS_PGPIO_BASE + 8),        ///< P_GPIO_29
	PAD_DS_PGPIO30      = (PR_PREFIX + PR_PAD_4L_DS_PGPIO_BASE + 10),       ///< P_GPIO_30
	PAD_DS_PGPIO31      = (PR_PREFIX + PR_PAD_4L_DS_PGPIO_BASE + 12),       ///< P_GPIO_31
	PAD_DS_PGPIO32      = (PR_PREFIX + PR_PAD_4L_DS_PGPIO_BASE + 14),       ///< P_GPIO_32
	PAD_DS_PGPIO33      = (PR_PREFIX + PR_PAD_4L_DS_PGPIO_BASE + 16),       ///< P_GPIO_33
	PAD_DS_PGPIO34      = (PR_PREFIX + PR_PAD_4L_DS_PGPIO_BASE + 18),       ///< P_GPIO_34
	PAD_DS_PGPIO35      = (PR_PREFIX + PR_PAD_4L_DS_PGPIO_BASE + 20),       ///< P_GPIO_35
	PAD_DS_PGPIO36      = (PR_PREFIX + PR_PAD_4L_DS_PGPIO_BASE + 22),       ///< P_GPIO_36
	PAD_DS_PGPIO37      = (PR_PREFIX + PR_PAD_4L_DS_PGPIO_BASE + 24),       ///< P_GPIO_37
	PAD_DS_PGPIO38      = (PR_PREFIX + PR_PAD_4L_DS_PGPIO_BASE + 26),       ///< P_GPIO_38

	// PR-SGPIO group
	PAD_DS_SGPIO0      = (PR_PREFIX + PR_PAD_4L_DS_SGPIO_BASE + 0),        ///< S_GPIO_0
	PAD_DS_SGPIO1      = (PR_PREFIX + PR_PAD_4L_DS_SGPIO_BASE + 2),        ///< S_GPIO_1
	PAD_DS_SGPIO2      = (PR_PREFIX + PR_PAD_2L_DS_SGPIO_BASE + 4),        ///< S_GPIO_2
	PAD_DS_SGPIO3      = (PR_PREFIX + PR_PAD_2L_DS_SGPIO_BASE + 5),        ///< S_GPIO_3
	PAD_DS_SGPIO4      = (PR_PREFIX + PR_PAD_2L_DS_SGPIO_BASE + 6),        ///< S_GPIO_4
	PAD_DS_SGPIO5      = (PR_PREFIX + PR_PAD_2L_DS_SGPIO_BASE + 7),       ///< S_GPIO_5
	PAD_DS_SGPIO6      = (PR_PREFIX + PR_PAD_2L_DS_SGPIO_BASE + 8),       ///< S_GPIO_6
	PAD_DS_SGPIO7      = (PR_PREFIX + PR_PAD_2L_DS_SGPIO_BASE + 9),       ///< S_GPIO_7
	PAD_DS_SGPIO8      = (PR_PREFIX + PR_PAD_2L_DS_SGPIO_BASE + 10),       ///< S_GPIO_8
	PAD_DS_SGPIO9      = (PR_PREFIX + PR_PAD_2L_DS_SGPIO_BASE + 11),       ///< S_GPIO_9
	PAD_DS_SGPIO10      = (PR_PREFIX + PR_PAD_2L_DS_SGPIO_BASE + 12),       ///< S_GPIO_10
	PAD_DS_SGPIO11      = (PR_PREFIX + PR_PAD_2L_DS_SGPIO_BASE + 13),       ///< S_GPIO_11
	PAD_DS_SGPIO12      = (PR_PREFIX + PR_PAD_2L_DS_SGPIO_BASE + 14),       ///< S_GPIO_12
	PAD_DS_SGPIO13      = (PR_PREFIX + PR_PAD_2L_DS_SGPIO_BASE + 15),       ///< S_GPIO_13
	PAD_DS_SGPIO14      = (PR_PREFIX + PR_PAD_2L_DS_SGPIO_BASE + 16),       ///< S_GPIO_14
	PAD_DS_SGPIO15      = (PR_PREFIX + PR_PAD_2L_DS_SGPIO_BASE + 17),       ///< S_GPIO_15
	PAD_DS_SGPIO16      = (PR_PREFIX + PR_PAD_2L_DS_SGPIO_BASE + 18),    ///< S_GPIO_16 /* 567 proposal have 128b shift from SGPIO15 to SGPIO16 */

	// PR-ASGPIO group
	PAD_DS_AGPIO0      = (PR_PREFIX + PR_PAD_4L_DS_AGPIO_BASE + 0),        ///< A_GPIO_0
	PAD_DS_AGPIO1      = (PR_PREFIX + PR_PAD_4L_DS_AGPIO_BASE + 2),        ///< A_GPIO_1
	PAD_DS_AGPIO2      = (PR_PREFIX + PR_PAD_4L_DS_AGPIO_BASE + 4),        ///< A_GPIO_2
	PAD_DS_AGPIO3      = (PR_PREFIX + PR_PAD_4L_DS_AGPIO_BASE + 6),        ///< A_GPIO_3

	PR_PAD_DS_MAX = PAD_DS_SGPIO16, // SGPIO16 2level case is the biggest reg in enum

	ENUM_DUMMY4WORD(PAD_DS)
} PAD_DS;

#define PAD_1P8V_SN_MAGIC       0x00005378
#define PAD_1P8V_SN2_MAGIC       0x00005378
#define PAD_1P8V_P1_MAGIC       0x00008031
#define PAD_1P8V_SD3_MAGIC      0x00534433

/**
    Pad power ID select

    Pad power ID for PAD_POWER_STRUCT.
*/
typedef enum {
	PAD_POWERID_MC0  =          0x00,      ///< Pad power id for MC0 LDO
	PAD_POWERID_MC1  =          0x01,      ///< Pad power id for MC1 LDO
	PAD_POWERID_ADC  =          0x02,      ///< Pad power id for ADC (backward compatible)
	PAD_POWERID_CSI  =          0x04,      ///< Pad power id for CSI (backward compatible)
	PAD_POWERID_SN   =          0x10,      ///< Pad power id for SN  REG
	PAD_POWERID_SN2  =          0x20,      ///< Pad power id for SN  REG
	PAD_POWERID_P1   =          0x40,      ///< Pad power id for P1  REG
	PAD_POWERID_SD3  =          0x80,      ///< Pad power id for SD3 REG

	ENUM_DUMMY4WORD(PAD_POWERID)
} PAD_POWERID;

/**
    Pad power select

    Pad power value for PAD_POWER_STRUCT.
*/
typedef enum {
	PAD_3P3V         =          0x00,      ///< Pad power is 1.8V
	PAD_1P8V         =          0x01,      ///< Pad power is 3.3V
	PAD_AVDD         =          0x00,      ///< Pad power is AVDD ( for PAD_POWERID_ADC use) (backward compatible)
	PAD_PAD_VAD      =          0x01,      ///< Pad power is PAD_ADC_VAD ( for PAD_POWERID_ADC use) (backward compatible)

	ENUM_DUMMY4WORD(PAD_POWER)
} PAD_POWER;

/**
    Pad power VAD for  PAD_POWERID_ADC

    Pad power VAD value for PAD_POWER_STRUCT.
*/
typedef enum {
	PAD_VAD_2P9V     =          0x00,      ///< Pad power VAD = 2.9V <ADC> (backward compatible)
	PAD_VAD_3P0V     =          0x01,      ///< Pad power VAD = 3.0V <ADC> (backward compatible)
	PAD_VAD_3P1V     =          0x03,      ///< Pad power VAD = 3.1V <ADC> (backward compatible)

	PAD_VAD_2P4V     =          0x100,     ///< Pad power VAD = 2.4V <CSI> (backward compatible)
	PAD_VAD_2P5V     =          0x101,     ///< Pad power VAD = 2.5V <CSI> (backward compatible)
	PAD_VAD_2P6V     =          0x103,     ///< Pad power VAD = 2.6V <CSI> (backward compatible)

	ENUM_DUMMY4WORD(PAD_VAD)
} PAD_VAD;

/**
    PAD power structure

    PAD power setting for pad_setPower()
*/
typedef struct {
	PAD_POWERID         pad_power_id;      ///< Pad power id
	PAD_POWER           pad_power;         ///< Pad power (for set)
	PAD_POWER           pad_status;        ///< Pad cell MS1/MS2 status (for get)
	BOOL                bias_current;      ///< Regulator bias current selection (backward compatible)
	///< - @b FALSE: disable
	///< - @b TRUE: enable
	BOOL                opa_gain;          ///< Regulator OPA gain/phase selection (backward compatible)
	///< - @b FALSE: disable
	///< - @b TRUE: enable
	BOOL                pull_down;         ///< Regulator output pull down control (backward compatible)
	///< - @b FALSE: none
	///< - @b TRUE: pull down enable
	BOOL                enable;            ///< Regulator enable (backward compatible)
	///< - @b FALSE: disable
	///< - @b TRUE: enable
	PAD_VAD             pad_vad;           ///< Pad VAD of PAD_POWERID_ADC when PAD_PAD_VAD = 1 (backward compatible)
} PAD_POWER_STRUCT;

extern ER pad_set_pull_updown(PAD_PIN pin, PAD_PULL pulltype);
extern ER pad_get_pull_updown(PAD_PIN pin, PAD_PULL *pulltype);
extern ER pad_set_drivingsink(PAD_DS name, PAD_DRIVINGSINK driving);
extern ER pad_get_drivingsink(PAD_DS name, PAD_DRIVINGSINK *driving);
extern ER pad_set_power(PAD_POWER_STRUCT *pad_power);
extern void pad_get_power(PAD_POWER_STRUCT *pad_power);

#endif
