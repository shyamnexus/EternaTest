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

#define PAD_REG_TO_BASE(reg) (((reg)/(4))*(32))

#define	PAD_CGPIO_BASE           PAD_REG_TO_BASE(0x00)      // 0x00
#define	PAD_JGPIO_BASE           PAD_REG_TO_BASE(0x04)      // 0x04
#define	PAD_PGPIO_BASE           PAD_REG_TO_BASE(0x08)      // 0x08~0x10
#define	PAD_DGPIO_BASE           PAD_REG_TO_BASE(0x18)      // 0x18
#define	PAD_EGPIO_BASE           PAD_REG_TO_BASE(0x20)      // 0x20~0x24
#define	PAD_SGPIO_BASE           PAD_REG_TO_BASE(0x28)      // 0x28~0x3C
#define	PAD_BGPIO_BASE           PAD_REG_TO_BASE(0x40)      // 0x40~0x44

#define	PAD_DS_CGPIO_BASE        PAD_REG_TO_BASE(0x50)      // 0x50~0x54
#define	PAD_DS_JGPIO_BASE        PAD_REG_TO_BASE(0x58)      // 0x58
#define	PAD_DS_PGPIO_BASE        PAD_REG_TO_BASE(0x60)      // 0x60~0x74
#define	PAD_DS_DGPIO_BASE        PAD_REG_TO_BASE(0x80)      // 0x80~0x84
#define	PAD_DS_EGPIO_BASE        PAD_REG_TO_BASE(0x90)      // 0x90~0x9C
#define	PAD_DS_SGPIO_BASE        PAD_REG_TO_BASE(0xA0)      // 0xA0~0xC8
#define	PAD_DS_BGPIO_BASE        PAD_REG_TO_BASE(0xE0)      // 0xE0~0xE8

/**
    @name Pad type pin ID.

    Pad type pin ID.

    Pad ID of pad_set_pull_updown (), pad_get_pull_updown ().
*/
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

	// JGPIO group
	PAD_PIN_JGPIO0      =    (PAD_JGPIO_BASE + 0),     ///< J_GPIO_0
	PAD_PIN_JGPIO1      =    (PAD_JGPIO_BASE + 2),     ///< J_GPIO_1
	PAD_PIN_JGPIO2      =    (PAD_JGPIO_BASE + 4),     ///< J_GPIO_2
	PAD_PIN_JGPIO3      =    (PAD_JGPIO_BASE + 6),     ///< J_GPIO_3
	PAD_PIN_JGPIO4      =    (PAD_JGPIO_BASE + 8),     ///< J_GPIO_4
	PAD_PIN_JGPIO5      =    (PAD_JGPIO_BASE + 10),    ///< J_GPIO_5

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

	// SGPIO group
	PAD_PIN_SGPIO0      =    (PAD_SGPIO_BASE + 0),     ///< S_GPIO_0
	PAD_PIN_SGPIO1      =    (PAD_SGPIO_BASE + 2),     ///< S_GPIO_1
	PAD_PIN_SGPIO2      =    (PAD_SGPIO_BASE + 4),     ///< S_GPIO_2
	PAD_PIN_SGPIO3      =    (PAD_SGPIO_BASE + 6),     ///< S_GPIO_3
	PAD_PIN_SGPIO4      =    (PAD_SGPIO_BASE + 8),     ///< S_GPIO_4
	PAD_PIN_SGPIO5      =    (PAD_SGPIO_BASE + 10),    ///< S_GPIO_5
	PAD_PIN_SGPIO6      =    (PAD_SGPIO_BASE + 12),    ///< S_GPIO_6
	PAD_PIN_SGPIO7      =    (PAD_SGPIO_BASE + 14),    ///< S_GPIO_7
	PAD_PIN_SGPIO8      =    (PAD_SGPIO_BASE + 16),    ///< S_GPIO_8
	PAD_PIN_SGPIO9      =    (PAD_SGPIO_BASE + 18),    ///< S_GPIO_9
	PAD_PIN_SGPIO10     =    (PAD_SGPIO_BASE + 20),    ///< S_GPIO_10
	PAD_PIN_SGPIO11     =    (PAD_SGPIO_BASE + 22),    ///< S_GPIO_11
	PAD_PIN_SGPIO12     =    (PAD_SGPIO_BASE + 24),    ///< S_GPIO_12
	PAD_PIN_SGPIO13     =    (PAD_SGPIO_BASE + 26),    ///< S_GPIO_13
	PAD_PIN_SGPIO14     =    (PAD_SGPIO_BASE + 28),    ///< S_GPIO_14
	PAD_PIN_SGPIO15     =    (PAD_SGPIO_BASE + 30),    ///< S_GPIO_15
	PAD_PIN_SGPIO16     =    (PAD_SGPIO_BASE + 32),    ///< S_GPIO_16
	PAD_PIN_SGPIO17     =    (PAD_SGPIO_BASE + 34),    ///< S_GPIO_17
	PAD_PIN_SGPIO18     =    (PAD_SGPIO_BASE + 36),    ///< S_GPIO_18
	PAD_PIN_SGPIO19     =    (PAD_SGPIO_BASE + 38),    ///< S_GPIO_19
	PAD_PIN_SGPIO20     =    (PAD_SGPIO_BASE + 40),    ///< S_GPIO_20
	PAD_PIN_SGPIO21     =    (PAD_SGPIO_BASE + 42),    ///< S_GPIO_21
	PAD_PIN_SGPIO22     =    (PAD_SGPIO_BASE + 44),    ///< S_GPIO_22
	PAD_PIN_SGPIO23     =    (PAD_SGPIO_BASE + 46),    ///< S_GPIO_23
	PAD_PIN_SGPIO24     =    (PAD_SGPIO_BASE + 48),    ///< S_GPIO_24
	PAD_PIN_SGPIO25     =    (PAD_SGPIO_BASE + 50),    ///< S_GPIO_25
	PAD_PIN_SGPIO26     =    (PAD_SGPIO_BASE + 52),    ///< S_GPIO_26
	PAD_PIN_SGPIO27     =    (PAD_SGPIO_BASE + 54),    ///< S_GPIO_27
	PAD_PIN_SGPIO28     =    (PAD_SGPIO_BASE + 56),    ///< S_GPIO_28
	PAD_PIN_SGPIO29     =    (PAD_SGPIO_BASE + 58),    ///< S_GPIO_29
	PAD_PIN_SGPIO30     =    (PAD_SGPIO_BASE + 60),    ///< S_GPIO_30
	PAD_PIN_SGPIO31     =    (PAD_SGPIO_BASE + 62),    ///< S_GPIO_31
	PAD_PIN_SGPIO32     =    (PAD_SGPIO_BASE + 64),    ///< S_GPIO_32
	PAD_PIN_SGPIO33     =    (PAD_SGPIO_BASE + 66),    ///< S_GPIO_33
	PAD_PIN_SGPIO34     =    (PAD_SGPIO_BASE + 68),    ///< S_GPIO_34
	PAD_PIN_SGPIO35     =    (PAD_SGPIO_BASE + 70),    ///< S_GPIO_35
	PAD_PIN_SGPIO36     =    (PAD_SGPIO_BASE + 72),    ///< S_GPIO_36
	PAD_PIN_SGPIO37     =    (PAD_SGPIO_BASE + 74),    ///< S_GPIO_37
	PAD_PIN_SGPIO38     =    (PAD_SGPIO_BASE + 76),    ///< S_GPIO_38
	PAD_PIN_SGPIO39     =    (PAD_SGPIO_BASE + 78),    ///< S_GPIO_39
	PAD_PIN_SGPIO40     =    (PAD_SGPIO_BASE + 80),    ///< S_GPIO_40
	PAD_PIN_SGPIO41     =    (PAD_SGPIO_BASE + 82),    ///< S_GPIO_41
	PAD_PIN_SGPIO42     =    (PAD_SGPIO_BASE + 84),    ///< S_GPIO_42
	PAD_PIN_SGPIO43     =    (PAD_SGPIO_BASE + 86),    ///< S_GPIO_43
	PAD_PIN_SGPIO44     =    (PAD_SGPIO_BASE + 88),    ///< S_GPIO_44
	PAD_PIN_SGPIO45     =    (PAD_SGPIO_BASE + 90),    ///< S_GPIO_45
	PAD_PIN_SGPIO46     =    (PAD_SGPIO_BASE + 92),    ///< S_GPIO_46
	PAD_PIN_SGPIO47     =    (PAD_SGPIO_BASE + 94),    ///< S_GPIO_47
	PAD_PIN_SGPIO48     =    (PAD_SGPIO_BASE + 96),    ///< S_GPIO_48
	PAD_PIN_SGPIO49     =    (PAD_SGPIO_BASE + 98),    ///< S_GPIO_49
	PAD_PIN_SGPIO50     =    (PAD_SGPIO_BASE + 100),   ///< S_GPIO_50
	PAD_PIN_SGPIO51     =    (PAD_SGPIO_BASE + 102),   ///< S_GPIO_51
	PAD_PIN_SGPIO52     =    (PAD_SGPIO_BASE + 104),   ///< S_GPIO_52
	PAD_PIN_SGPIO53     =    (PAD_SGPIO_BASE + 106),   ///< S_GPIO_53
	PAD_PIN_SGPIO54     =    (PAD_SGPIO_BASE + 108),   ///< S_GPIO_54
	PAD_PIN_SGPIO55     =    (PAD_SGPIO_BASE + 110),   ///< S_GPIO_55
	PAD_PIN_SGPIO56     =    (PAD_SGPIO_BASE + 112),   ///< S_GPIO_56
	PAD_PIN_SGPIO57     =    (PAD_SGPIO_BASE + 114),   ///< S_GPIO_57
	PAD_PIN_SGPIO58     =    (PAD_SGPIO_BASE + 116),   ///< S_GPIO_58
	PAD_PIN_SGPIO59     =    (PAD_SGPIO_BASE + 118),   ///< S_GPIO_59
	PAD_PIN_SGPIO60     =    (PAD_SGPIO_BASE + 120),   ///< S_GPIO_60
	PAD_PIN_SGPIO61     =    (PAD_SGPIO_BASE + 122),   ///< S_GPIO_61
	PAD_PIN_SGPIO62     =    (PAD_SGPIO_BASE + 124),   ///< S_GPIO_62
	PAD_PIN_SGPIO63     =    (PAD_SGPIO_BASE + 126),   ///< S_GPIO_63
	PAD_PIN_SGPIO64     =    (PAD_SGPIO_BASE + 128),   ///< S_GPIO_64
	PAD_PIN_SGPIO65     =    (PAD_SGPIO_BASE + 130),   ///< S_GPIO_65
	PAD_PIN_SGPIO66     =    (PAD_SGPIO_BASE + 132),   ///< S_GPIO_66
	PAD_PIN_SGPIO67     =    (PAD_SGPIO_BASE + 134),   ///< S_GPIO_67
	PAD_PIN_SGPIO68     =    (PAD_SGPIO_BASE + 136),   ///< S_GPIO_68
	PAD_PIN_SGPIO69     =    (PAD_SGPIO_BASE + 138),   ///< S_GPIO_69
	PAD_PIN_SGPIO70     =    (PAD_SGPIO_BASE + 140),   ///< S_GPIO_70
	PAD_PIN_SGPIO71     =    (PAD_SGPIO_BASE + 142),   ///< S_GPIO_71
	PAD_PIN_SGPIO72     =    (PAD_SGPIO_BASE + 144),   ///< S_GPIO_72
	PAD_PIN_SGPIO73     =    (PAD_SGPIO_BASE + 146),   ///< S_GPIO_73
	PAD_PIN_SGPIO74     =    (PAD_SGPIO_BASE + 148),   ///< S_GPIO_74
	PAD_PIN_SGPIO75     =    (PAD_SGPIO_BASE + 150),   ///< S_GPIO_75
	PAD_PIN_SGPIO76     =    (PAD_SGPIO_BASE + 152),   ///< S_GPIO_76
	PAD_PIN_SGPIO77     =    (PAD_SGPIO_BASE + 154),   ///< S_GPIO_77
	PAD_PIN_SGPIO78     =    (PAD_SGPIO_BASE + 156),   ///< S_GPIO_78
	PAD_PIN_SGPIO79     =    (PAD_SGPIO_BASE + 158),   ///< S_GPIO_79
	PAD_PIN_SGPIO80     =    (PAD_SGPIO_BASE + 160),   ///< S_GPIO_80
	PAD_PIN_SGPIO81     =    (PAD_SGPIO_BASE + 162),   ///< S_GPIO_81
	PAD_PIN_SGPIO82     =    (PAD_SGPIO_BASE + 164),   ///< S_GPIO_82
	PAD_PIN_SGPIO83     =    (PAD_SGPIO_BASE + 166),   ///< S_GPIO_83

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

	PAD_PIN_MAX         =     PAD_PIN_BGPIO16,
	ENUM_DUMMY4WORD(PAD_PIN)
} PAD_PIN;

/**
    Pad type select

    Pad type select

    Pad type value for pad_set_pull_updown(), pad_get_pull_updown().
*/
typedef enum {
	PAD_NONE         =          0x00,      ///< none of pull up/down
	PAD_PULLDOWN     =          0x01,      ///< pull down
	PAD_PULLUP       =          0x02,      ///< pull up
	PAD_KEEPER       =          0x03,      ///< keeper

	ENUM_DUMMY4WORD(PAD_PULL)
} PAD_PULL;

/**
    Pad driving select

    Pad driving select

    Pad driving value for pad_set_drivingsink(), pad_get_drivingsink().
*/
typedef enum {
	PAD_DRIVINGSINK_LEVEL_0     =    0x00,     ///< 3mA  / 4mA  / 4mA  / 4mA
	PAD_DRIVINGSINK_LEVEL_1     =    0x01,     ///< 6mA  / 8mA  / 10mA / 8mA
	PAD_DRIVINGSINK_LEVEL_2     =    0x02,     ///< 9mA  / 12mA
	PAD_DRIVINGSINK_LEVEL_3     =    0x03,     ///< 12mA / 16mA
	PAD_DRIVINGSINK_LEVEL_4     =    0x04,     ///< 15mA
	PAD_DRIVINGSINK_LEVEL_5     =    0x05,     ///< 18mA
	PAD_DRIVINGSINK_LEVEL_6     =    0x06,     ///< 21mA
	PAD_DRIVINGSINK_LEVEL_7     =    0x07,     ///< 24mA

	ENUM_DUMMY4WORD(PAD_DRIVINGSINK)
} PAD_DRIVINGSINK;

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

	// JGPIO group
	PAD_DS_JGPIO0      =    (PAD_DS_JGPIO_BASE + 0),     ///< J_GPIO_0
	PAD_DS_JGPIO1      =    (PAD_DS_JGPIO_BASE + 4),     ///< J_GPIO_1
	PAD_DS_JGPIO2      =    (PAD_DS_JGPIO_BASE + 8),     ///< J_GPIO_2
	PAD_DS_JGPIO3      =    (PAD_DS_JGPIO_BASE + 12),    ///< J_GPIO_3
	PAD_DS_JGPIO4      =    (PAD_DS_JGPIO_BASE + 16),    ///< J_GPIO_4
	PAD_DS_JGPIO5      =    (PAD_DS_JGPIO_BASE + 20),    ///< J_GPIO_5

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
	PAD_DS_EGPIO24     =    (PAD_DS_EGPIO_BASE + 96),    ///< E_GPIO_23
	PAD_DS_EGPIO25     =    (PAD_DS_EGPIO_BASE + 100),   ///< E_GPIO_25
	PAD_DS_EGPIO26     =    (PAD_DS_EGPIO_BASE + 104),   ///< E_GPIO_26
	PAD_DS_EGPIO27     =    (PAD_DS_EGPIO_BASE + 108),   ///< E_GPIO_27
	PAD_DS_EGPIO28     =    (PAD_DS_EGPIO_BASE + 112),   ///< E_GPIO_28
	PAD_DS_EGPIO29     =    (PAD_DS_EGPIO_BASE + 116),   ///< E_GPIO_29
	PAD_DS_EGPIO30     =    (PAD_DS_EGPIO_BASE + 120),   ///< E_GPIO_30
	PAD_DS_EGPIO31     =    (PAD_DS_EGPIO_BASE + 124),   ///< E_GPIO_31

	// SGPIO group
	PAD_DS_SGPIO0      =    (PAD_DS_SGPIO_BASE + 0),     ///< S_GPIO_0
	PAD_DS_SGPIO1      =    (PAD_DS_SGPIO_BASE + 4),     ///< S_GPIO_1
	PAD_DS_SGPIO2      =    (PAD_DS_SGPIO_BASE + 8),     ///< S_GPIO_2
	PAD_DS_SGPIO3      =    (PAD_DS_SGPIO_BASE + 12),    ///< S_GPIO_3
	PAD_DS_SGPIO4      =    (PAD_DS_SGPIO_BASE + 16),    ///< S_GPIO_4
	PAD_DS_SGPIO5      =    (PAD_DS_SGPIO_BASE + 20),    ///< S_GPIO_5
	PAD_DS_SGPIO6      =    (PAD_DS_SGPIO_BASE + 24),    ///< S_GPIO_6
	PAD_DS_SGPIO7      =    (PAD_DS_SGPIO_BASE + 28),    ///< S_GPIO_7
	PAD_DS_SGPIO8      =    (PAD_DS_SGPIO_BASE + 32),    ///< S_GPIO_8
	PAD_DS_SGPIO9      =    (PAD_DS_SGPIO_BASE + 36),    ///< S_GPIO_9
	PAD_DS_SGPIO10     =    (PAD_DS_SGPIO_BASE + 40),    ///< S_GPIO_10
	PAD_DS_SGPIO11     =    (PAD_DS_SGPIO_BASE + 44),    ///< S_GPIO_11
	PAD_DS_SGPIO12     =    (PAD_DS_SGPIO_BASE + 48),    ///< S_GPIO_12
	PAD_DS_SGPIO13     =    (PAD_DS_SGPIO_BASE + 52),    ///< S_GPIO_13
	PAD_DS_SGPIO14     =    (PAD_DS_SGPIO_BASE + 56),    ///< S_GPIO_14
	PAD_DS_SGPIO15     =    (PAD_DS_SGPIO_BASE + 60),    ///< S_GPIO_15
	PAD_DS_SGPIO16     =    (PAD_DS_SGPIO_BASE + 64),    ///< S_GPIO_16
	PAD_DS_SGPIO17     =    (PAD_DS_SGPIO_BASE + 68),    ///< S_GPIO_17
	PAD_DS_SGPIO18     =    (PAD_DS_SGPIO_BASE + 72),    ///< S_GPIO_18
	PAD_DS_SGPIO19     =    (PAD_DS_SGPIO_BASE + 76),    ///< S_GPIO_19
	PAD_DS_SGPIO20     =    (PAD_DS_SGPIO_BASE + 80),    ///< S_GPIO_20
	PAD_DS_SGPIO21     =    (PAD_DS_SGPIO_BASE + 84),    ///< S_GPIO_21
	PAD_DS_SGPIO22     =    (PAD_DS_SGPIO_BASE + 88),    ///< S_GPIO_22
	PAD_DS_SGPIO23     =    (PAD_DS_SGPIO_BASE + 92),    ///< S_GPIO_23
	PAD_DS_SGPIO24     =    (PAD_DS_SGPIO_BASE + 96),    ///< S_GPIO_24
	PAD_DS_SGPIO25     =    (PAD_DS_SGPIO_BASE + 100),   ///< S_GPIO_25
	PAD_DS_SGPIO26     =    (PAD_DS_SGPIO_BASE + 104),   ///< S_GPIO_26
	PAD_DS_SGPIO27     =    (PAD_DS_SGPIO_BASE + 108),   ///< S_GPIO_27
	PAD_DS_SGPIO28     =    (PAD_DS_SGPIO_BASE + 112),   ///< S_GPIO_28
	PAD_DS_SGPIO29     =    (PAD_DS_SGPIO_BASE + 116),   ///< S_GPIO_29
	PAD_DS_SGPIO30     =    (PAD_DS_SGPIO_BASE + 120),   ///< S_GPIO_30
	PAD_DS_SGPIO31     =    (PAD_DS_SGPIO_BASE + 124),   ///< S_GPIO_31
	PAD_DS_SGPIO32     =    (PAD_DS_SGPIO_BASE + 128),   ///< S_GPIO_32
	PAD_DS_SGPIO33     =    (PAD_DS_SGPIO_BASE + 132),   ///< S_GPIO_33
	PAD_DS_SGPIO34     =    (PAD_DS_SGPIO_BASE + 136),   ///< S_GPIO_34
	PAD_DS_SGPIO35     =    (PAD_DS_SGPIO_BASE + 140),   ///< S_GPIO_35
	PAD_DS_SGPIO36     =    (PAD_DS_SGPIO_BASE + 144),   ///< S_GPIO_36
	PAD_DS_SGPIO37     =    (PAD_DS_SGPIO_BASE + 148),   ///< S_GPIO_37
	PAD_DS_SGPIO38     =    (PAD_DS_SGPIO_BASE + 152),   ///< S_GPIO_38
	PAD_DS_SGPIO39     =    (PAD_DS_SGPIO_BASE + 156),   ///< S_GPIO_39
	PAD_DS_SGPIO40     =    (PAD_DS_SGPIO_BASE + 160),   ///< S_GPIO_40
	PAD_DS_SGPIO41     =    (PAD_DS_SGPIO_BASE + 164),   ///< S_GPIO_41
	PAD_DS_SGPIO42     =    (PAD_DS_SGPIO_BASE + 168),   ///< S_GPIO_42
	PAD_DS_SGPIO43     =    (PAD_DS_SGPIO_BASE + 172),   ///< S_GPIO_43
	PAD_DS_SGPIO44     =    (PAD_DS_SGPIO_BASE + 176),   ///< S_GPIO_44
	PAD_DS_SGPIO45     =    (PAD_DS_SGPIO_BASE + 180),   ///< S_GPIO_45
	PAD_DS_SGPIO46     =    (PAD_DS_SGPIO_BASE + 184),   ///< S_GPIO_46
	PAD_DS_SGPIO47     =    (PAD_DS_SGPIO_BASE + 188),   ///< S_GPIO_47
	PAD_DS_SGPIO48     =    (PAD_DS_SGPIO_BASE + 192),   ///< S_GPIO_48
	PAD_DS_SGPIO49     =    (PAD_DS_SGPIO_BASE + 196),   ///< S_GPIO_49
	PAD_DS_SGPIO50     =    (PAD_DS_SGPIO_BASE + 200),   ///< S_GPIO_50
	PAD_DS_SGPIO51     =    (PAD_DS_SGPIO_BASE + 204),   ///< S_GPIO_51
	PAD_DS_SGPIO52     =    (PAD_DS_SGPIO_BASE + 208),   ///< S_GPIO_52
	PAD_DS_SGPIO53     =    (PAD_DS_SGPIO_BASE + 212),   ///< S_GPIO_53
	PAD_DS_SGPIO54     =    (PAD_DS_SGPIO_BASE + 216),   ///< S_GPIO_54
	PAD_DS_SGPIO55     =    (PAD_DS_SGPIO_BASE + 220),   ///< S_GPIO_55
	PAD_DS_SGPIO56     =    (PAD_DS_SGPIO_BASE + 224),   ///< S_GPIO_56
	PAD_DS_SGPIO57     =    (PAD_DS_SGPIO_BASE + 228),   ///< S_GPIO_57
	PAD_DS_SGPIO58     =    (PAD_DS_SGPIO_BASE + 232),   ///< S_GPIO_58
	PAD_DS_SGPIO59     =    (PAD_DS_SGPIO_BASE + 236),   ///< S_GPIO_59
	PAD_DS_SGPIO60     =    (PAD_DS_SGPIO_BASE + 240),   ///< S_GPIO_60
	PAD_DS_SGPIO61     =    (PAD_DS_SGPIO_BASE + 244),   ///< S_GPIO_61
	PAD_DS_SGPIO62     =    (PAD_DS_SGPIO_BASE + 248),   ///< S_GPIO_62
	PAD_DS_SGPIO63     =    (PAD_DS_SGPIO_BASE + 252),   ///< S_GPIO_63
	PAD_DS_SGPIO64     =    (PAD_DS_SGPIO_BASE + 256),   ///< S_GPIO_64
	PAD_DS_SGPIO65     =    (PAD_DS_SGPIO_BASE + 260),   ///< S_GPIO_65
	PAD_DS_SGPIO66     =    (PAD_DS_SGPIO_BASE + 264),   ///< S_GPIO_66
	PAD_DS_SGPIO67     =    (PAD_DS_SGPIO_BASE + 268),   ///< S_GPIO_67
	PAD_DS_SGPIO68     =    (PAD_DS_SGPIO_BASE + 272),   ///< S_GPIO_68
	PAD_DS_SGPIO69     =    (PAD_DS_SGPIO_BASE + 276),   ///< S_GPIO_69
	PAD_DS_SGPIO70     =    (PAD_DS_SGPIO_BASE + 280),   ///< S_GPIO_70
	PAD_DS_SGPIO71     =    (PAD_DS_SGPIO_BASE + 284),   ///< S_GPIO_71
	PAD_DS_SGPIO72     =    (PAD_DS_SGPIO_BASE + 288),   ///< S_GPIO_72
	PAD_DS_SGPIO73     =    (PAD_DS_SGPIO_BASE + 292),   ///< S_GPIO_73
	PAD_DS_SGPIO74     =    (PAD_DS_SGPIO_BASE + 296),   ///< S_GPIO_74
	PAD_DS_SGPIO75     =    (PAD_DS_SGPIO_BASE + 300),   ///< S_GPIO_75
	PAD_DS_SGPIO76     =    (PAD_DS_SGPIO_BASE + 304),   ///< S_GPIO_76
	PAD_DS_SGPIO77     =    (PAD_DS_SGPIO_BASE + 308),   ///< S_GPIO_77
	PAD_DS_SGPIO78     =    (PAD_DS_SGPIO_BASE + 312),   ///< S_GPIO_78
	PAD_DS_SGPIO79     =    (PAD_DS_SGPIO_BASE + 316),   ///< S_GPIO_79
	PAD_DS_SGPIO80     =    (PAD_DS_SGPIO_BASE + 320),   ///< S_GPIO_80
	PAD_DS_SGPIO81     =    (PAD_DS_SGPIO_BASE + 324),   ///< S_GPIO_81
	PAD_DS_SGPIO82     =    (PAD_DS_SGPIO_BASE + 328),   ///< S_GPIO_82
	PAD_DS_SGPIO83     =    (PAD_DS_SGPIO_BASE + 332),   ///< S_GPIO_83

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

	PAD_DS_MAX = PAD_DS_BGPIO16,
	ENUM_DUMMY4WORD(PAD_DS)
} PAD_DS;

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
	PAD_3P3V         =          0x00,      ///< Pad power is 3.3V
	PAD_AVDD         =          0x00,      ///< Pad power is AVDD ( for PAD_POWERID_ADC use)
	PAD_1P8V         =          0x01,      ///< Pad power is 1.8V
	PAD_PAD_VAD      =          0x01,      ///< Pad power is PAD_ADC_VAD ( for PAD_POWERID_ADC use)

	ENUM_DUMMY4WORD(PAD_POWER)
} PAD_POWER;

/**
    Pad power VAD for  PAD_POWERID_ADC

    Pad power VAD value for PAD_POWER_STRUCT.
*/
typedef enum {
	PAD_VAD_2P9V     =          0x00,      ///< Pad power VAD = 2.9V <ADC>
	PAD_VAD_3P0V     =          0x01,      ///< Pad power VAD = 3.0V <ADC>
	PAD_VAD_3P1V     =          0x03,      ///< Pad power VAD = 3.1V <ADC>

	PAD_VAD_2P4V     =          0x100,     ///< Pad power VAD = 2.4V <CSI>
	PAD_VAD_2P5V     =          0x101,     ///< Pad power VAD = 2.5V <CSI>
	PAD_VAD_2P6V     =          0x103,     ///< Pad power VAD = 2.6V <CSI>


	ENUM_DUMMY4WORD(PAD_VAD)
} PAD_VAD;


/**
    PAD power structure

    PAD power setting for pad_setPower()
*/
typedef struct {
	PAD_POWERID         pad_power_id;         ///< Pad power id
	PAD_POWER           pad_power;           ///< Pad power
	BOOL                bias_current;       ///< Regulator bias current selection
	///< - @b FALSE: disable
	///< - @b TRUE: enable
	BOOL                opa_gain;           ///< Regulator OPA gain/phase selection
	///< - @b FALSE: disable
	///< - @b TRUE: enable
	BOOL                pull_down;          ///< Regulator output pull down control
	///< - @b FALSE: none
	///< - @b TRUE: pull down enable
	BOOL                enable;            ///< Regulator enable
	///< - @b FALSE: disable
	///< - @b TRUE: enable
	PAD_VAD             pad_vad;             ///< Pad VAD of PAD_POWERID_ADC when PAD_PAD_VAD = 1
} PAD_POWER_STRUCT;

/**
    PAD DLY structure

    PAD delay setting for pad_set_delaychain()
*/
typedef struct {
	unsigned char  vo_clk_dly;    ///< vo clk delay
	unsigned char  vga_hs_dly;    ///< vga hs delay
	unsigned char  vga_vs_dly;    ///< vga vs delay
	BOOL           vo_clk_inv;
	///< - @b FALSE: not invert
	///< - @b TRUE: invert
	BOOL           vga_hs_inv;
	///< - @b FALSE: not invert
	///< - @b TRUE: invert
	BOOL           vga_vs_inv;
	///< - @b FALSE: not invert
	///< - @b TRUE: invert
} PAD_DLY_STRUCT;


extern ER pad_set_pull_updown(PAD_PIN pin, PAD_PULL pulltype);
extern ER pad_get_pull_updown(PAD_PIN pin, PAD_PULL *pulltype);
extern ER pad_set_drivingsink(PAD_DS name, PAD_DRIVINGSINK driving);
extern ER pad_get_drivingsink(PAD_DS name, PAD_DRIVINGSINK *driving);
extern ER pad_set_power(PAD_POWER_STRUCT *pad_power);
extern void pad_get_power(PAD_POWER_STRUCT *pad_power);
extern ER pad_set_delaychain(PAD_DLY_STRUCT *del_sel);
extern void pad_get_delaychain(PAD_DLY_STRUCT *del_sel);


extern ER pad_set_pull_updown_ep(PAD_PIN pin, PAD_PULL pulltype, int ep_ch);
extern ER pad_get_pull_updown_ep(PAD_PIN pin, PAD_PULL *pulltype, int ep_ch);
extern ER pad_set_drivingsink_ep(PAD_DS name, PAD_DRIVINGSINK driving, int ep_ch);
extern ER pad_get_drivingsink_ep(PAD_DS name, PAD_DRIVINGSINK *driving, int ep_ch);
extern ER pad_set_power_ep(PAD_POWER_STRUCT *pad_power, int ep_ch);
extern void pad_get_power_ep(PAD_POWER_STRUCT *pad_power, int ep_ch);
extern ER pad_set_delaychain_ep(PAD_DLY_STRUCT *del_sel, int ep_ch);
extern void pad_get_delaychain_ep(PAD_DLY_STRUCT *del_sel, int ep_ch);


#endif
