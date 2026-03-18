/**
	PAD controller header

	PAD controller header

	@file       pad.h
	@ingroup    mIDrvSys_PAD
	@note       Nothing

	Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#ifndef _PAD_H
#define _PAD_H

#include <linux/soc/nvt/nvt_type.h>

// PAD Pull UP/DOWN Register 0
#define PAD_PUPD0_REG_OFS                    0x00

#define PAD_REG_TO_BASE(reg) (((reg)/(4))*(32))

#define	PAD_CGPIO_BASE           PAD_REG_TO_BASE(0x00)      // 0x00~0x04
#define	PAD_PGPIO_BASE           PAD_REG_TO_BASE(0x08)      // 0x08~0x10
#define	PAD_DGPIO_BASE           PAD_REG_TO_BASE(0x18)      // 0x18
#define	PAD_EGPIO_BASE           PAD_REG_TO_BASE(0x20)      // 0x20~0x24
#define	PAD_BGPIO_BASE           PAD_REG_TO_BASE(0x30)      // 0x30~0x34
#define	PAD_JGPIO_BASE           PAD_REG_TO_BASE(0x38)      // 0x38
#define	PAD_AGPIO_BASE           PAD_REG_TO_BASE(0x3C)      // 0x3C

#define	PAD_DS_CGPIO_BASE        PAD_REG_TO_BASE(0x100)     // 0x100~0x108
#define	PAD_DS_PGPIO_BASE        PAD_REG_TO_BASE(0x110)     // 0x110~0x124
#define	PAD_DS_DGPIO_BASE        PAD_REG_TO_BASE(0x130)     // 0x130~0x134
#define	PAD_DS_EGPIO_BASE        PAD_REG_TO_BASE(0x140)     // 0x140~0x14C
#define	PAD_DS_BGPIO_BASE        PAD_REG_TO_BASE(0x150)     // 0x150~0x158
#define	PAD_DS_JGPIO_BASE        PAD_REG_TO_BASE(0x160)     // 0x160
#define	PAD_DS_AGPIO_BASE        PAD_REG_TO_BASE(0x164)     // 0x164

typedef enum {
	PAD_NONE                    =    0x00,      ///< none of pull up/down
	PAD_PULLDOWN                =    0x01,      ///< pull down
	PAD_PULLUP                  =    0x02,      ///< pull up
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
	PAD_PIN_CGPIO0      =    (PAD_CGPIO_BASE + 0),     ///< C_GPIO_0
	PAD_PIN_CGPIO1      =    (PAD_CGPIO_BASE + 2),     ///< C_GPIO_1
	PAD_PIN_CGPIO2      =    (PAD_CGPIO_BASE + 4),     ///< C_GPIO_2
	PAD_PIN_CGPIO3      =    (PAD_CGPIO_BASE + 6),     ///< C_GPIO_3
	PAD_PIN_CGPIO4      =    (PAD_CGPIO_BASE + 8),     ///< C_GPIO_4
	PAD_PIN_CGPIO5      =    (PAD_CGPIO_BASE + 10),    ///< C_GPIO_5
	PAD_PIN_CGPIO6      =    (PAD_CGPIO_BASE + 12),    ///< C_GPIO_6
	PAD_PIN_CGPIO7      =    (PAD_CGPIO_BASE + 14),    ///< C_GPIO_7
	PAD_PIN_CGPIO8      =    (PAD_CGPIO_BASE + 16),    ///< C_GPIO_8
	PAD_PIN_CGPIO9      =    (PAD_CGPIO_BASE + 18),    ///< C_GPIO_9
	PAD_PIN_CGPIO10     =    (PAD_CGPIO_BASE + 20),    ///< C_GPIO_10
	PAD_PIN_CGPIO11     =    (PAD_CGPIO_BASE + 22),    ///< C_GPIO_11
	PAD_PIN_CGPIO12     =    (PAD_CGPIO_BASE + 24),    ///< C_GPIO_12
	PAD_PIN_CGPIO13     =    (PAD_CGPIO_BASE + 26),    ///< C_GPIO_13
	PAD_PIN_CGPIO14     =    (PAD_CGPIO_BASE + 28),    ///< C_GPIO_14
	PAD_PIN_CGPIO15     =    (PAD_CGPIO_BASE + 30),    ///< C_GPIO_15
	PAD_PIN_CGPIO16     =    (PAD_CGPIO_BASE + 32),    ///< C_GPIO_16
	PAD_PIN_CGPIO17     =    (PAD_CGPIO_BASE + 34),    ///< C_GPIO_17

	// PGPIO group
	PAD_PIN_PGPIO0      =    (PAD_PGPIO_BASE + 0),     ///< P_GPIO_0
	PAD_PIN_PGPIO1      =    (PAD_PGPIO_BASE + 2),     ///< P_GPIO_1
	PAD_PIN_PGPIO2      =    (PAD_PGPIO_BASE + 4),     ///< P_GPIO_2
	PAD_PIN_PGPIO3      =    (PAD_PGPIO_BASE + 6),     ///< P_GPIO_3
	PAD_PIN_PGPIO4      =    (PAD_PGPIO_BASE + 8),     ///< P_GPIO_4
	PAD_PIN_PGPIO5      =    (PAD_PGPIO_BASE + 10),    ///< P_GPIO_5
	PAD_PIN_PGPIO6      =    (PAD_PGPIO_BASE + 12),    ///< P_GPIO_6
	PAD_PIN_PGPIO7      =    (PAD_PGPIO_BASE + 14),    ///< P_GPIO_7
	PAD_PIN_PGPIO8      =    (PAD_PGPIO_BASE + 16),    ///< P_GPIO_8
	PAD_PIN_PGPIO9      =    (PAD_PGPIO_BASE + 18),    ///< P_GPIO_9
	PAD_PIN_PGPIO10     =    (PAD_PGPIO_BASE + 20),    ///< P_GPIO_10
	PAD_PIN_PGPIO11     =    (PAD_PGPIO_BASE + 22),    ///< P_GPIO_11
	PAD_PIN_PGPIO12     =    (PAD_PGPIO_BASE + 24),    ///< P_GPIO_12
	PAD_PIN_PGPIO13     =    (PAD_PGPIO_BASE + 26),    ///< P_GPIO_13
	PAD_PIN_PGPIO14     =    (PAD_PGPIO_BASE + 28),    ///< P_GPIO_14
	PAD_PIN_PGPIO15     =    (PAD_PGPIO_BASE + 30),    ///< P_GPIO_15
	PAD_PIN_PGPIO16     =    (PAD_PGPIO_BASE + 32),    ///< P_GPIO_16
	PAD_PIN_PGPIO17     =    (PAD_PGPIO_BASE + 34),    ///< P_GPIO_17
	PAD_PIN_PGPIO18     =    (PAD_PGPIO_BASE + 36),    ///< P_GPIO_18
	PAD_PIN_PGPIO19     =    (PAD_PGPIO_BASE + 38),    ///< P_GPIO_19
	PAD_PIN_PGPIO20     =    (PAD_PGPIO_BASE + 40),    ///< P_GPIO_20
	PAD_PIN_PGPIO21     =    (PAD_PGPIO_BASE + 42),    ///< P_GPIO_21
	PAD_PIN_PGPIO22     =    (PAD_PGPIO_BASE + 44),    ///< P_GPIO_22
	PAD_PIN_PGPIO23     =    (PAD_PGPIO_BASE + 46),    ///< P_GPIO_23
	PAD_PIN_PGPIO24     =    (PAD_PGPIO_BASE + 48),    ///< P_GPIO_24
	PAD_PIN_PGPIO25     =    (PAD_PGPIO_BASE + 50),    ///< P_GPIO_25
	PAD_PIN_PGPIO26     =    (PAD_PGPIO_BASE + 52),    ///< P_GPIO_26
	PAD_PIN_PGPIO27     =    (PAD_PGPIO_BASE + 54),    ///< P_GPIO_27
	PAD_PIN_PGPIO28     =    (PAD_PGPIO_BASE + 56),    ///< P_GPIO_28
	PAD_PIN_PGPIO29     =    (PAD_PGPIO_BASE + 58),    ///< P_GPIO_29
	PAD_PIN_PGPIO30     =    (PAD_PGPIO_BASE + 60),    ///< P_GPIO_30
	PAD_PIN_PGPIO31     =    (PAD_PGPIO_BASE + 62),    ///< P_GPIO_31
	PAD_PIN_PGPIO32     =    (PAD_PGPIO_BASE + 64),    ///< P_GPIO_32
	PAD_PIN_PGPIO33     =    (PAD_PGPIO_BASE + 66),    ///< P_GPIO_33
	PAD_PIN_PGPIO34     =    (PAD_PGPIO_BASE + 68),    ///< P_GPIO_34
	PAD_PIN_PGPIO35     =    (PAD_PGPIO_BASE + 70),    ///< P_GPIO_35
	PAD_PIN_PGPIO36     =    (PAD_PGPIO_BASE + 72),    ///< P_GPIO_36
	PAD_PIN_PGPIO37     =    (PAD_PGPIO_BASE + 74),    ///< P_GPIO_37
	PAD_PIN_PGPIO38     =    (PAD_PGPIO_BASE + 76),    ///< P_GPIO_38
	PAD_PIN_PGPIO39     =    (PAD_PGPIO_BASE + 78),    ///< P_GPIO_39
	PAD_PIN_PGPIO40     =    (PAD_PGPIO_BASE + 80),    ///< P_GPIO_40
	PAD_PIN_PGPIO41     =    (PAD_PGPIO_BASE + 82),    ///< P_GPIO_41
	PAD_PIN_PGPIO42     =    (PAD_PGPIO_BASE + 84),    ///< P_GPIO_42
	PAD_PIN_PGPIO43     =    (PAD_PGPIO_BASE + 86),    ///< P_GPIO_43
	PAD_PIN_PGPIO44     =    (PAD_PGPIO_BASE + 88),    ///< P_GPIO_44
	PAD_PIN_PGPIO45     =    (PAD_PGPIO_BASE + 90),    ///< P_GPIO_45
	PAD_PIN_PGPIO46     =    (PAD_PGPIO_BASE + 92),    ///< P_GPIO_46
	PAD_PIN_PGPIO47     =    (PAD_PGPIO_BASE + 94),    ///< P_GPIO_47

	// DGPIO group
	PAD_PIN_DGPIO0      =    (PAD_DGPIO_BASE + 0),     ///< D_GPIO_0
	PAD_PIN_DGPIO1      =    (PAD_DGPIO_BASE + 2),     ///< D_GPIO_1
	PAD_PIN_DGPIO2      =    (PAD_DGPIO_BASE + 4),     ///< D_GPIO_2
	PAD_PIN_DGPIO3      =    (PAD_DGPIO_BASE + 6),     ///< D_GPIO_3
	PAD_PIN_DGPIO4      =    (PAD_DGPIO_BASE + 8),     ///< D_GPIO_4
	PAD_PIN_DGPIO5      =    (PAD_DGPIO_BASE + 10),    ///< D_GPIO_5
	PAD_PIN_DGPIO6      =    (PAD_DGPIO_BASE + 12),    ///< D_GPIO_6
	PAD_PIN_DGPIO7      =    (PAD_DGPIO_BASE + 14),    ///< D_GPIO_7
	PAD_PIN_DGPIO8      =    (PAD_DGPIO_BASE + 16),    ///< D_GPIO_8
	PAD_PIN_DGPIO9      =    (PAD_DGPIO_BASE + 18),    ///< D_GPIO_9
	PAD_PIN_DGPIO10     =    (PAD_DGPIO_BASE + 20),    ///< D_GPIO_10
	PAD_PIN_DGPIO11     =    (PAD_DGPIO_BASE + 22),    ///< D_GPIO_11
	PAD_PIN_SYS_RST     =    (PAD_DGPIO_BASE + 24),    ///< SYS_RST

	// EGPIO group
	PAD_PIN_EGPIO0      =    (PAD_EGPIO_BASE + 0),     ///< E_GPIO_0
	PAD_PIN_EGPIO1      =    (PAD_EGPIO_BASE + 2),     ///< E_GPIO_1
	PAD_PIN_EGPIO2      =    (PAD_EGPIO_BASE + 4),     ///< E_GPIO_2
	PAD_PIN_EGPIO3      =    (PAD_EGPIO_BASE + 6),     ///< E_GPIO_3
	PAD_PIN_EGPIO4      =    (PAD_EGPIO_BASE + 8),     ///< E_GPIO_4
	PAD_PIN_EGPIO5      =    (PAD_EGPIO_BASE + 10),    ///< E_GPIO_5
	PAD_PIN_EGPIO6      =    (PAD_EGPIO_BASE + 12),    ///< E_GPIO_6
	PAD_PIN_EGPIO7      =    (PAD_EGPIO_BASE + 14),    ///< E_GPIO_7
	PAD_PIN_EGPIO8      =    (PAD_EGPIO_BASE + 16),    ///< E_GPIO_8
	PAD_PIN_EGPIO9      =    (PAD_EGPIO_BASE + 18),    ///< E_GPIO_9
	PAD_PIN_EGPIO10     =    (PAD_EGPIO_BASE + 20),    ///< E_GPIO_10
	PAD_PIN_EGPIO11     =    (PAD_EGPIO_BASE + 22),    ///< E_GPIO_11
	PAD_PIN_EGPIO12     =    (PAD_EGPIO_BASE + 24),    ///< E_GPIO_12
	PAD_PIN_EGPIO13     =    (PAD_EGPIO_BASE + 26),    ///< E_GPIO_13
	PAD_PIN_EGPIO14     =    (PAD_EGPIO_BASE + 28),    ///< E_GPIO_14
	PAD_PIN_EGPIO15     =    (PAD_EGPIO_BASE + 30),    ///< E_GPIO_15
	PAD_PIN_EGPIO16     =    (PAD_EGPIO_BASE + 32),    ///< E_GPIO_16
	PAD_PIN_EGPIO17     =    (PAD_EGPIO_BASE + 34),    ///< E_GPIO_17
	PAD_PIN_EGPIO18     =    (PAD_EGPIO_BASE + 36),    ///< E_GPIO_18
	PAD_PIN_EGPIO19     =    (PAD_EGPIO_BASE + 38),    ///< E_GPIO_19
	PAD_PIN_EGPIO20     =    (PAD_EGPIO_BASE + 40),    ///< E_GPIO_20
	PAD_PIN_EGPIO21     =    (PAD_EGPIO_BASE + 42),    ///< E_GPIO_21
	PAD_PIN_EGPIO22     =    (PAD_EGPIO_BASE + 44),    ///< E_GPIO_22
	PAD_PIN_EGPIO23     =    (PAD_EGPIO_BASE + 46),    ///< E_GPIO_23
	PAD_PIN_EGPIO24     =    (PAD_EGPIO_BASE + 48),    ///< E_GPIO_23
	PAD_PIN_EGPIO25     =    (PAD_EGPIO_BASE + 50),    ///< E_GPIO_25
	PAD_PIN_EGPIO26     =    (PAD_EGPIO_BASE + 52),    ///< E_GPIO_26
	PAD_PIN_EGPIO27     =    (PAD_EGPIO_BASE + 54),    ///< E_GPIO_27
	PAD_PIN_EGPIO28     =    (PAD_EGPIO_BASE + 56),    ///< E_GPIO_28
	PAD_PIN_EGPIO29     =    (PAD_EGPIO_BASE + 58),    ///< E_GPIO_29
	PAD_PIN_EGPIO30     =    (PAD_EGPIO_BASE + 60),    ///< E_GPIO_30
	PAD_PIN_EGPIO31     =    (PAD_EGPIO_BASE + 62),    ///< E_GPIO_31

	// BGPIO group
	PAD_PIN_BGPIO0      =    (PAD_BGPIO_BASE + 0),     ///< B_GPIO_0
	PAD_PIN_BGPIO1      =    (PAD_BGPIO_BASE + 2),     ///< B_GPIO_1
	PAD_PIN_BGPIO2      =    (PAD_BGPIO_BASE + 4),     ///< B_GPIO_2
	PAD_PIN_BGPIO3      =    (PAD_BGPIO_BASE + 6),     ///< B_GPIO_3
	PAD_PIN_BGPIO4      =    (PAD_BGPIO_BASE + 8),     ///< B_GPIO_4
	PAD_PIN_BGPIO5      =    (PAD_BGPIO_BASE + 10),    ///< B_GPIO_5
	PAD_PIN_BGPIO6      =    (PAD_BGPIO_BASE + 12),    ///< B_GPIO_6
	PAD_PIN_BGPIO7      =    (PAD_BGPIO_BASE + 14),    ///< B_GPIO_7
	PAD_PIN_BGPIO8      =    (PAD_BGPIO_BASE + 16),    ///< B_GPIO_8
	PAD_PIN_BGPIO9      =    (PAD_BGPIO_BASE + 18),    ///< B_GPIO_9
	PAD_PIN_BGPIO10     =    (PAD_BGPIO_BASE + 20),    ///< B_GPIO_10
	PAD_PIN_BGPIO11     =    (PAD_BGPIO_BASE + 22),    ///< B_GPIO_11
	PAD_PIN_BGPIO12     =    (PAD_BGPIO_BASE + 24),    ///< B_GPIO_12
	PAD_PIN_BGPIO13     =    (PAD_BGPIO_BASE + 26),    ///< B_GPIO_13
	PAD_PIN_BGPIO14     =    (PAD_BGPIO_BASE + 28),    ///< B_GPIO_14
	PAD_PIN_BGPIO15     =    (PAD_BGPIO_BASE + 30),    ///< B_GPIO_15
	PAD_PIN_BGPIO16     =    (PAD_BGPIO_BASE + 32),    ///< B_GPIO_16

	// JGPIO group
	PAD_PIN_JGPIO0      =    (PAD_JGPIO_BASE + 0),     ///< J_GPIO_0
	PAD_PIN_JGPIO1      =    (PAD_JGPIO_BASE + 2),     ///< J_GPIO_1
	PAD_PIN_JGPIO2      =    (PAD_JGPIO_BASE + 4),     ///< J_GPIO_2
	PAD_PIN_JGPIO3      =    (PAD_JGPIO_BASE + 6),     ///< J_GPIO_3
	PAD_PIN_JGPIO4      =    (PAD_JGPIO_BASE + 8),     ///< J_GPIO_4
	PAD_PIN_JGPIO5      =    (PAD_JGPIO_BASE + 10),    ///< J_GPIO_5

	// AGPIO group
	PAD_PIN_AGPIO0      =    (PAD_AGPIO_BASE + 0),     ///< A_GPIO_0
	PAD_PIN_AGPIO1      =    (PAD_AGPIO_BASE + 2),     ///< A_GPIO_1
	PAD_PIN_AGPIO2      =    (PAD_AGPIO_BASE + 4),     ///< A_GPIO_2
	PAD_PIN_AGPIO3      =    (PAD_AGPIO_BASE + 6),     ///< A_GPIO_3

	PAD_PIN_MAX         =     PAD_PIN_AGPIO3,
	ENUM_DUMMY4WORD(PAD_PIN)
} PAD_PIN;

typedef enum {
	// CGPIO group
	PAD_DS_CGPIO0      =    (PAD_DS_CGPIO_BASE + 0),     ///< C_GPIO_0
	PAD_DS_CGPIO1      =    (PAD_DS_CGPIO_BASE + 4),     ///< C_GPIO_1
	PAD_DS_CGPIO2      =    (PAD_DS_CGPIO_BASE + 8),     ///< C_GPIO_2
	PAD_DS_CGPIO3      =    (PAD_DS_CGPIO_BASE + 12),    ///< C_GPIO_3
	PAD_DS_CGPIO4      =    (PAD_DS_CGPIO_BASE + 16),    ///< C_GPIO_4
	PAD_DS_CGPIO5      =    (PAD_DS_CGPIO_BASE + 20),    ///< C_GPIO_5
	PAD_DS_CGPIO6      =    (PAD_DS_CGPIO_BASE + 24),    ///< C_GPIO_6
	PAD_DS_CGPIO7      =    (PAD_DS_CGPIO_BASE + 28),    ///< C_GPIO_7
	PAD_DS_CGPIO8      =    (PAD_DS_CGPIO_BASE + 32),    ///< C_GPIO_8
	PAD_DS_CGPIO9      =    (PAD_DS_CGPIO_BASE + 36),    ///< C_GPIO_9
	PAD_DS_CGPIO10     =    (PAD_DS_CGPIO_BASE + 40),    ///< C_GPIO_10
	PAD_DS_CGPIO11     =    (PAD_DS_CGPIO_BASE + 44),    ///< C_GPIO_11
	PAD_DS_CGPIO12     =    (PAD_DS_CGPIO_BASE + 48),    ///< C_GPIO_12
	PAD_DS_CGPIO13     =    (PAD_DS_CGPIO_BASE + 52),    ///< C_GPIO_13
	PAD_DS_CGPIO14     =    (PAD_DS_CGPIO_BASE + 56),    ///< C_GPIO_14
	PAD_DS_CGPIO15     =    (PAD_DS_CGPIO_BASE + 60),    ///< C_GPIO_15
	PAD_DS_CGPIO16     =    (PAD_DS_CGPIO_BASE + 64),    ///< C_GPIO_16
	PAD_DS_CGPIO17     =    (PAD_DS_CGPIO_BASE + 68),    ///< C_GPIO_17

	// PGPIO group
	PAD_DS_PGPIO0      =    (PAD_DS_PGPIO_BASE + 0),     ///< P_GPIO_0
	PAD_DS_PGPIO1      =    (PAD_DS_PGPIO_BASE + 4),     ///< P_GPIO_1
	PAD_DS_PGPIO2      =    (PAD_DS_PGPIO_BASE + 8),     ///< P_GPIO_2
	PAD_DS_PGPIO3      =    (PAD_DS_PGPIO_BASE + 12),    ///< P_GPIO_3
	PAD_DS_PGPIO4      =    (PAD_DS_PGPIO_BASE + 16),    ///< P_GPIO_4
	PAD_DS_PGPIO5      =    (PAD_DS_PGPIO_BASE + 20),    ///< P_GPIO_5
	PAD_DS_PGPIO6      =    (PAD_DS_PGPIO_BASE + 24),    ///< P_GPIO_6
	PAD_DS_PGPIO7      =    (PAD_DS_PGPIO_BASE + 28),    ///< P_GPIO_7
	PAD_DS_PGPIO8      =    (PAD_DS_PGPIO_BASE + 32),    ///< P_GPIO_8
	PAD_DS_PGPIO9      =    (PAD_DS_PGPIO_BASE + 36),    ///< P_GPIO_9
	PAD_DS_PGPIO10     =    (PAD_DS_PGPIO_BASE + 40),    ///< P_GPIO_10
	PAD_DS_PGPIO11     =    (PAD_DS_PGPIO_BASE + 44),    ///< P_GPIO_11
	PAD_DS_PGPIO12     =    (PAD_DS_PGPIO_BASE + 48),    ///< P_GPIO_12
	PAD_DS_PGPIO13     =    (PAD_DS_PGPIO_BASE + 52),    ///< P_GPIO_13
	PAD_DS_PGPIO14     =    (PAD_DS_PGPIO_BASE + 56),    ///< P_GPIO_14
	PAD_DS_PGPIO15     =    (PAD_DS_PGPIO_BASE + 60),    ///< P_GPIO_15
	PAD_DS_PGPIO16     =    (PAD_DS_PGPIO_BASE + 64),    ///< P_GPIO_16
	PAD_DS_PGPIO17     =    (PAD_DS_PGPIO_BASE + 68),    ///< P_GPIO_17
	PAD_DS_PGPIO18     =    (PAD_DS_PGPIO_BASE + 72),    ///< P_GPIO_18
	PAD_DS_PGPIO19     =    (PAD_DS_PGPIO_BASE + 76),    ///< P_GPIO_19
	PAD_DS_PGPIO20     =    (PAD_DS_PGPIO_BASE + 80),    ///< P_GPIO_20
	PAD_DS_PGPIO21     =    (PAD_DS_PGPIO_BASE + 84),    ///< P_GPIO_21
	PAD_DS_PGPIO22     =    (PAD_DS_PGPIO_BASE + 88),    ///< P_GPIO_22
	PAD_DS_PGPIO23     =    (PAD_DS_PGPIO_BASE + 92),    ///< P_GPIO_23
	PAD_DS_PGPIO24     =    (PAD_DS_PGPIO_BASE + 96),    ///< P_GPIO_24
	PAD_DS_PGPIO25     =    (PAD_DS_PGPIO_BASE + 100),   ///< P_GPIO_25
	PAD_DS_PGPIO26     =    (PAD_DS_PGPIO_BASE + 104),   ///< P_GPIO_26
	PAD_DS_PGPIO27     =    (PAD_DS_PGPIO_BASE + 108),   ///< P_GPIO_27
	PAD_DS_PGPIO28     =    (PAD_DS_PGPIO_BASE + 112),   ///< P_GPIO_28
	PAD_DS_PGPIO29     =    (PAD_DS_PGPIO_BASE + 116),   ///< P_GPIO_29
	PAD_DS_PGPIO30     =    (PAD_DS_PGPIO_BASE + 120),   ///< P_GPIO_30
	PAD_DS_PGPIO31     =    (PAD_DS_PGPIO_BASE + 124),   ///< P_GPIO_31
	PAD_DS_PGPIO32     =    (PAD_DS_PGPIO_BASE + 128),   ///< P_GPIO_32
	PAD_DS_PGPIO33     =    (PAD_DS_PGPIO_BASE + 132),   ///< P_GPIO_33
	PAD_DS_PGPIO34     =    (PAD_DS_PGPIO_BASE + 136),   ///< P_GPIO_34
	PAD_DS_PGPIO35     =    (PAD_DS_PGPIO_BASE + 140),   ///< P_GPIO_35
	PAD_DS_PGPIO36     =    (PAD_DS_PGPIO_BASE + 144),   ///< P_GPIO_36
	PAD_DS_PGPIO37     =    (PAD_DS_PGPIO_BASE + 148),   ///< P_GPIO_37
	PAD_DS_PGPIO38     =    (PAD_DS_PGPIO_BASE + 152),   ///< P_GPIO_38
	PAD_DS_PGPIO39     =    (PAD_DS_PGPIO_BASE + 156),   ///< P_GPIO_39
	PAD_DS_PGPIO40     =    (PAD_DS_PGPIO_BASE + 160),   ///< P_GPIO_40
	PAD_DS_PGPIO41     =    (PAD_DS_PGPIO_BASE + 164),   ///< P_GPIO_41
	PAD_DS_PGPIO42     =    (PAD_DS_PGPIO_BASE + 168),   ///< P_GPIO_42
	PAD_DS_PGPIO43     =    (PAD_DS_PGPIO_BASE + 172),   ///< P_GPIO_43
	PAD_DS_PGPIO44     =    (PAD_DS_PGPIO_BASE + 176),   ///< P_GPIO_44
	PAD_DS_PGPIO45     =    (PAD_DS_PGPIO_BASE + 180),   ///< P_GPIO_45
	PAD_DS_PGPIO46     =    (PAD_DS_PGPIO_BASE + 184),   ///< P_GPIO_46
	PAD_DS_PGPIO47     =    (PAD_DS_PGPIO_BASE + 188),   ///< P_GPIO_47

	// DGPIO group
	PAD_DS_DGPIO0      =    (PAD_DS_DGPIO_BASE + 0),     ///< D_GPIO_0
	PAD_DS_DGPIO1      =    (PAD_DS_DGPIO_BASE + 4),     ///< D_GPIO_1
	PAD_DS_DGPIO2      =    (PAD_DS_DGPIO_BASE + 8),     ///< D_GPIO_2
	PAD_DS_DGPIO3      =    (PAD_DS_DGPIO_BASE + 12),    ///< D_GPIO_3
	PAD_DS_DGPIO4      =    (PAD_DS_DGPIO_BASE + 16),    ///< D_GPIO_4
	PAD_DS_DGPIO5      =    (PAD_DS_DGPIO_BASE + 20),    ///< D_GPIO_5
	PAD_DS_DGPIO6      =    (PAD_DS_DGPIO_BASE + 24),    ///< D_GPIO_6
	PAD_DS_DGPIO7      =    (PAD_DS_DGPIO_BASE + 28),    ///< D_GPIO_7
	PAD_DS_DGPIO8      =    (PAD_DS_DGPIO_BASE + 32),    ///< D_GPIO_8
	PAD_DS_DGPIO9      =    (PAD_DS_DGPIO_BASE + 36),    ///< D_GPIO_9
	PAD_DS_DGPIO10     =    (PAD_DS_DGPIO_BASE + 40),    ///< D_GPIO_10
	PAD_DS_DGPIO11     =    (PAD_DS_DGPIO_BASE + 44),    ///< D_GPIO_11
	PAD_DS_SYS_RST     =    (PAD_DS_DGPIO_BASE + 48),    ///< SYS_RST

	// EGPIO group
	PAD_DS_EGPIO0      =    (PAD_DS_EGPIO_BASE + 0),     ///< E_GPIO_0
	PAD_DS_EGPIO1      =    (PAD_DS_EGPIO_BASE + 4),     ///< E_GPIO_1
	PAD_DS_EGPIO2      =    (PAD_DS_EGPIO_BASE + 8),     ///< E_GPIO_2
	PAD_DS_EGPIO3      =    (PAD_DS_EGPIO_BASE + 12),    ///< E_GPIO_3
	PAD_DS_EGPIO4      =    (PAD_DS_EGPIO_BASE + 16),    ///< E_GPIO_4
	PAD_DS_EGPIO5      =    (PAD_DS_EGPIO_BASE + 20),    ///< E_GPIO_5
	PAD_DS_EGPIO6      =    (PAD_DS_EGPIO_BASE + 24),    ///< E_GPIO_6
	PAD_DS_EGPIO7      =    (PAD_DS_EGPIO_BASE + 28),    ///< E_GPIO_7
	PAD_DS_EGPIO8      =    (PAD_DS_EGPIO_BASE + 32),    ///< E_GPIO_8
	PAD_DS_EGPIO9      =    (PAD_DS_EGPIO_BASE + 36),    ///< E_GPIO_9
	PAD_DS_EGPIO10     =    (PAD_DS_EGPIO_BASE + 40),    ///< E_GPIO_10
	PAD_DS_EGPIO11     =    (PAD_DS_EGPIO_BASE + 44),    ///< E_GPIO_11
	PAD_DS_EGPIO12     =    (PAD_DS_EGPIO_BASE + 48),    ///< E_GPIO_12
	PAD_DS_EGPIO13     =    (PAD_DS_EGPIO_BASE + 52),    ///< E_GPIO_13
	PAD_DS_EGPIO14     =    (PAD_DS_EGPIO_BASE + 56),    ///< E_GPIO_14
	PAD_DS_EGPIO15     =    (PAD_DS_EGPIO_BASE + 60),    ///< E_GPIO_15
	PAD_DS_EGPIO16     =    (PAD_DS_EGPIO_BASE + 64),    ///< E_GPIO_16
	PAD_DS_EGPIO17     =    (PAD_DS_EGPIO_BASE + 68),    ///< E_GPIO_17
	PAD_DS_EGPIO18     =    (PAD_DS_EGPIO_BASE + 72),    ///< E_GPIO_18
	PAD_DS_EGPIO19     =    (PAD_DS_EGPIO_BASE + 76),    ///< E_GPIO_19
	PAD_DS_EGPIO20     =    (PAD_DS_EGPIO_BASE + 80),    ///< E_GPIO_20
	PAD_DS_EGPIO21     =    (PAD_DS_EGPIO_BASE + 84),    ///< E_GPIO_21
	PAD_DS_EGPIO22     =    (PAD_DS_EGPIO_BASE + 88),    ///< E_GPIO_22
	PAD_DS_EGPIO23     =    (PAD_DS_EGPIO_BASE + 92),    ///< E_GPIO_23
	PAD_DS_EGPIO24     =    (PAD_DS_EGPIO_BASE + 96),    ///< E_GPIO_24
	PAD_DS_EGPIO25     =    (PAD_DS_EGPIO_BASE + 100),   ///< E_GPIO_25
	PAD_DS_EGPIO26     =    (PAD_DS_EGPIO_BASE + 104),   ///< E_GPIO_26
	PAD_DS_EGPIO27     =    (PAD_DS_EGPIO_BASE + 108),   ///< E_GPIO_27
	PAD_DS_EGPIO28     =    (PAD_DS_EGPIO_BASE + 112),   ///< E_GPIO_28
	PAD_DS_EGPIO29     =    (PAD_DS_EGPIO_BASE + 116),   ///< E_GPIO_29
	PAD_DS_EGPIO30     =    (PAD_DS_EGPIO_BASE + 120),   ///< E_GPIO_30
	PAD_DS_EGPIO31     =    (PAD_DS_EGPIO_BASE + 124),   ///< E_GPIO_31

	// BGPIO group
	PAD_DS_BGPIO0      =    (PAD_DS_BGPIO_BASE + 0),     ///< B_GPIO_0
	PAD_DS_BGPIO1      =    (PAD_DS_BGPIO_BASE + 4),     ///< B_GPIO_1
	PAD_DS_BGPIO2      =    (PAD_DS_BGPIO_BASE + 8),     ///< B_GPIO_2
	PAD_DS_BGPIO3      =    (PAD_DS_BGPIO_BASE + 12),    ///< B_GPIO_3
	PAD_DS_BGPIO4      =    (PAD_DS_BGPIO_BASE + 16),    ///< B_GPIO_4
	PAD_DS_BGPIO5      =    (PAD_DS_BGPIO_BASE + 20),    ///< B_GPIO_5
	PAD_DS_BGPIO6      =    (PAD_DS_BGPIO_BASE + 24),    ///< B_GPIO_6
	PAD_DS_BGPIO7      =    (PAD_DS_BGPIO_BASE + 28),    ///< B_GPIO_7
	PAD_DS_BGPIO8      =    (PAD_DS_BGPIO_BASE + 32),    ///< B_GPIO_8
	PAD_DS_BGPIO9      =    (PAD_DS_BGPIO_BASE + 36),    ///< B_GPIO_9
	PAD_DS_BGPIO10     =    (PAD_DS_BGPIO_BASE + 40),    ///< B_GPIO_10
	PAD_DS_BGPIO11     =    (PAD_DS_BGPIO_BASE + 44),    ///< B_GPIO_11
	PAD_DS_BGPIO12     =    (PAD_DS_BGPIO_BASE + 48),    ///< B_GPIO_12
	PAD_DS_BGPIO13     =    (PAD_DS_BGPIO_BASE + 52),    ///< B_GPIO_13
	PAD_DS_BGPIO14     =    (PAD_DS_BGPIO_BASE + 56),    ///< B_GPIO_14
	PAD_DS_BGPIO15     =    (PAD_DS_BGPIO_BASE + 60),    ///< B_GPIO_15
	PAD_DS_BGPIO16     =    (PAD_DS_BGPIO_BASE + 64),    ///< B_GPIO_16

	// JGPIO group
	PAD_DS_JGPIO0      =    (PAD_DS_JGPIO_BASE + 0),     ///< J_GPIO_0
	PAD_DS_JGPIO1      =    (PAD_DS_JGPIO_BASE + 4),     ///< J_GPIO_1
	PAD_DS_JGPIO2      =    (PAD_DS_JGPIO_BASE + 8),     ///< J_GPIO_2
	PAD_DS_JGPIO3      =    (PAD_DS_JGPIO_BASE + 12),    ///< J_GPIO_3
	PAD_DS_JGPIO4      =    (PAD_DS_JGPIO_BASE + 16),    ///< J_GPIO_4
	PAD_DS_JGPIO5      =    (PAD_DS_JGPIO_BASE + 20),    ///< J_GPIO_5

	// AGPIO group
	PAD_DS_AGPIO0      =    (PAD_DS_AGPIO_BASE + 0),     ///< A_GPIO_0
	PAD_DS_AGPIO1      =    (PAD_DS_AGPIO_BASE + 4),     ///< A_GPIO_1
	PAD_DS_AGPIO2      =    (PAD_DS_AGPIO_BASE + 8),     ///< A_GPIO_2
	PAD_DS_AGPIO3      =    (PAD_DS_AGPIO_BASE + 12),    ///< A_GPIO_3

	PAD_DS_MAX = PAD_DS_AGPIO3,
	ENUM_DUMMY4WORD(PAD_DS)
} PAD_DS;


#define PAD_1P8V_SN_MAGIC       0x00005378
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
	PAD_POWERID_P1   =          0x20,      ///< Pad power id for P1  REG
	PAD_POWERID_SD3  =          0x40,      ///< Pad power id for SD3 REG

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
