/*
	Pinmux module internal header file

	Pinmux module internal header file

	@file		top_reg.h
	@ingroup
	@note		Nothing

	Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#ifndef _PINMUX_INT_H
#define _PINMUX_INT_H

#include <linux/soc/nvt/rcw_macro.h>
#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000

#define TOP_REGGPIO_START_OFS    0x20
#define TOP_REGGPIO_END_OFS      0x80

#define UART_MUX_1               0x01
#define UART_MUX_2               0x02
#define UART_MUX_3               0x03

#define MUX_1                    0x01
#define MUX_2                    0x02
#define MUX_3                    0x03
#define MUX_4                    0x04
#define MUX_5                    0x05

#define SPI_BUS_WIDTH_2          0x00
#define SPI_BUS_WIDTH_4          0x01

#define MUX_DIS                  0x00
#define MUX_EN                   0x01

#define SDIO_BUS_WIDTH_4         0x00
#define SDIO_BUS_WIDTH_8         0x01
/*
	GPIO_ID_ENUM
*/
typedef enum
{
	GPIO_ID_EMUM_FUNC,                  //< pinmux is mapping to function
	GPIO_ID_EMUM_GPIO,                  //< pinmux is mapping to gpio

	ENUM_DUMMY4WORD(GPIO_ID_ENUM)
} GPIO_ID_ENUM;

//
//0x00 TOP Control Register 0
//
#define TOP_REG0_OFS                        0x00
union TOP_REG0 {
	uint32_t reg;
	struct {
	unsigned int BOOT_SRC:3;          
	unsigned int rev0:2;
	unsigned int EJTAG_SEL:1;
	unsigned int rev1:1;
	unsigned int PCIE_BOOT_SEL:2;
	unsigned int PCIE_SATA_COMBO_SEL:1;
	unsigned int PCIE_REFCLK_BOOT_SEL:1;
	unsigned int rev4:21;
	} bit;
};


//
//0x04 TOP Control Register 1
//
#define TOP_REG1_OFS                        0x04
union TOP_REG1 {
	uint32_t reg;
	struct {
	unsigned int SPI_EN:1;           // SPI EN
	unsigned int SPI_BUS_WIDTH:1;    // SPI Bus Width
	unsigned int SPI_CS1_EN:1;       // SPI CS1 En
	unsigned int rev0:1;
	unsigned int SDIO:2;             // SDIO selection
	unsigned int rev1:2;
	unsigned int SDIO2:1;            // SDIO2 enable/disable
	unsigned int rev2:1;
	unsigned int SDIO2_BUS_WIDTH:1;  // SDIO2 Bus Width
	unsigned int SDIO2_DS:1;         // SDIO2 DS enable/disable
	unsigned int rev3:4;
	unsigned int EJTAG_EN:1;         // EJTAG enable/disable
	unsigned int EXTROM_EN:1;        // External ROM enable/disable
	unsigned int USB3_PWR_EN:1;      // USB3 Power enable
	unsigned int rev4:13;
	} bit;
};

//
//0x08 TOP Control Register 2
//
#define TOP_REG2_OFS                        0x08
union TOP_REG2 {
	uint32_t reg;
	struct {
	unsigned int LCD310:2;           // LCD310
	unsigned int rev0:2;
	unsigned int LCD210:2;           // LCD210
	unsigned int rev1:2;
	unsigned int LCD310L:2;          // LCD310L
	unsigned int rev2:2;
	unsigned int LCD310_DE:1;        // LCD310 DE enable
	unsigned int rev3:19;
	} bit;
};

//
//0x0C TOP Control Register 3
//
#define TOP_REG3_OFS                        0x0C
union TOP_REG3 {
	uint32_t reg;
	struct {
	unsigned int ETH:3;              // ETH
	unsigned int rev0:1;
	unsigned int ETH2:3;             // ETH2
	unsigned int rev1:1;
	unsigned int ETH_REFCLK:1;       // ETH REFCLK
	unsigned int rev2:1;
	unsigned int ETH_RST:1;          // ETH RST
	unsigned int rev3:1;
	unsigned int ETH_MDC_MDIO:2;     // ETH MDC MDIO
	unsigned int rev4:2;
	unsigned int ETH2_REFCLK:1;      // ETH2 REFCLK
	unsigned int rev5:1;
	unsigned int ETH2_RST:1;         // ETH RST
	unsigned int rev6:1;
	unsigned int ETH2_MDC_MDIO:2;    // ETH2 MDC MDIO
	unsigned int rev7:10;
	} bit;
};


//
//0x10 TOP Control Register 4
//
#define TOP_REG4_OFS                        0x10
union TOP_REG4 {
	uint32_t reg;
	struct {
	unsigned int I2C:2;              // I2C
	unsigned int rev0:2;
	unsigned int I2C2:2;             // I2C2
	unsigned int rev1:2;
	unsigned int I2C3:3;             // I2C3
	unsigned int rev2:1;
	unsigned int CLK_12M:1;          // 12M CLK
	unsigned int rev3:19;
	} bit;
};

//
//0x14 TOP Control Register 5
//
#define TOP_REG5_OFS                        0x14
union TOP_REG5 {
	uint32_t reg;
	struct {
	unsigned int rev0:16;
	unsigned int EXT_CLK:1;
	unsigned int rev1:3;
	unsigned int IRDA:4;             // IRDA
	unsigned int SATA_LED:2;         // SATA LED
	unsigned int SATA2_LED:2;        // SATA2 LED
	unsigned int SATA3_LED:2;        // SATA3 LED
	unsigned int RTC_CAL:1;          // RTC_CAL
	unsigned int TVDAC_TEST_CLK:1;   // TVDAC test clock
	} bit;
};

//
//0x18 TOP Control Register 6
//
#define TOP_REG6_OFS                        0x18
union TOP_REG6 {
	uint32_t reg;
	struct {
	unsigned int PWM:1;              // PWM
	unsigned int rev0:1;
	unsigned int PWM2:1;             // PWM2
	unsigned int rev1:1;
	unsigned int PWM3:1;             // PWM3
	unsigned int rev2:1;
	unsigned int PWM4:1;             // PWM4
	unsigned int rev3:25;
	} bit;
};

//
//0x1C TOP Control Register 7
//
#define TOP_REG7_OFS                        0x1C
union TOP_REG7 {
	uint32_t reg;
	struct {
	unsigned int UART:2;             // UART
	unsigned int rev0:2;
	unsigned int UART2:2;            // UART2
	unsigned int rev1:2;
	unsigned int UART3:1;            // UART3
	unsigned int rev2:3;
	unsigned int UART4:2;            // UART4
	unsigned int rev3:2;
	unsigned int UART5:2;            // UART5
	unsigned int rev4:2;
	unsigned int UART_CTS_RTS:2;     // UART CTS/RTS
	unsigned int UART2_CTS_RTS:2;    // UART2 CTS/RTS
	unsigned int rev5:8;
	} bit;
};

//0x20 TOP Control Register 8
//
#define TOP_REG8_OFS                        0x20
union TOP_REG8 {
	uint32_t reg;
	struct {
	unsigned int RAM_SRC_SEL:1;      // RAM SRC SEL
	unsigned int rev0:3;
	unsigned int RAL_SEL:4;          // RAM SEL
	unsigned int rev1:24;
	} bit;
};

//0x24 TOP Control Register 9
//
#define TOP_REG9_OFS                        0x24
union TOP_REG9 {
	uint32_t reg;
	struct {
	unsigned int CAP:2;
	unsigned int CAP1:2;
	unsigned int CAP2:2;
	unsigned int CAP3:2;
	unsigned int CAP4:2;
	unsigned int CAP5:2;
	unsigned int CAP6:2;
	unsigned int CAP7:2;
	unsigned int CAP_CLK:2;
	unsigned int CAP1_CLK:2;
	unsigned int CAP2_CLK:2;
	unsigned int CAP3_CLK:2;
	unsigned int CAP4_CLK:2;
	unsigned int CAP5_CLK:2;
	unsigned int CAP6_CLK:2;
	unsigned int CAP7_CLK:2;
	} bit;
};

//0x28 TOP Control Register 10
//
#define TOP_REG10_OFS                        0x28
union TOP_REG10 {
	uint32_t reg;
	struct {
	unsigned int PCIE_MODE_SEL:1;    // PCIE MODE SEL
	unsigned int rev0:3;
	unsigned int PCIE_RESTN_RSE:1;   // PCIE_RSTN release
	unsigned int rev1:1;
	unsigned int PCIE_CLKREQN:1;     // PCIE CLKREQN (JGPIO0)
	unsigned int rev2:1;
	unsigned int PCIE_REFCLK_SRC:1;  // PCIE REFCLK source
	unsigned int PCIE_REFCLK_PAD_OUT_EN:1;  // PCIE REFCLK PAD OUT enable
	unsigned int rev3:22;
	} bit;
};

//0x2C TOP Control Register 11
//
#define TOP_REG11_OFS                        0x2C
union TOP_REG11 {
	uint32_t reg;
	struct {
	unsigned int I2S:2;
	unsigned int I2S_MCLK:2;
	unsigned int I2S_TX:2;
	unsigned int I2S_RX:2;
	unsigned int I2S2:2;
	unsigned int I2S2_MCLK:2;
	unsigned int I2S2_TX:2;
	unsigned int I2S2_RX:2;
	unsigned int I2S3:3;
	unsigned int rev0:1;
	unsigned int I2S3_MCLK:3;
	unsigned int rev1:1;
	unsigned int I2S3_TX:3;
	unsigned int rev2:1;
	unsigned int I2S3_RX:3;
	unsigned int rev4:1;
	} bit;
};

//0x30 TOP Control Register 12
//
#define TOP_REG12_OFS                        0x30
union TOP_REG12 {
	uint32_t reg;
	struct {
	unsigned int I2S4:2;
	unsigned int I2S4_MCLK:2;
	unsigned int I2S4_TX:2;
	unsigned int I2S4_RX:2;
	unsigned int rev0:24;
	} bit;
};


//
// 0xA0 CGPIO Control Register
//
#define TOP_REGCGPIO0_OFS                    0xA0
union TOP_REGCGPIO0 {
	uint32_t reg;
	struct {
	unsigned int CGPIO_0:1;          // pinmux of CGPIO 0
	unsigned int CGPIO_1:1;          // pinmux of CGPIO 1
	unsigned int CGPIO_2:1;          // pinmux of CGPIO 2
	unsigned int CGPIO_3:1;          // pinmux of CGPIO 3
	unsigned int CGPIO_4:1;          // pinmux of CGPIO 4
	unsigned int CGPIO_5:1;          // pinmux of CGPIO 5
	unsigned int CGPIO_6:1;          // pinmux of CGPIO 6
	unsigned int CGPIO_7:1;          // pinmux of CGPIO 7
	unsigned int CGPIO_8:1;          // pinmux of CGPIO 8
	unsigned int CGPIO_9:1;          // pinmux of CGPIO 9
	unsigned int CGPIO_10:1;         // pinmux of CGPIO 10
	unsigned int CGPIO_11:1;         // pinmux of CGPIO 11
	unsigned int CGPIO_12:1;         // pinmux of CGPIO 12
	unsigned int CGPIO_13:1;         // pinmux of CGPIO 13
	unsigned int reserved0:18;
	} bit;
};

//
// 0xA4 JGPIO Control Register
//
#define TOP_REGJGPIO0_OFS                    0xA4
union TOP_REGJGPIO0 {
	uint32_t reg;
	struct {
	unsigned int JGPIO_0:1;          // pinmux of JGPIO 0
	unsigned int JGPIO_1:1;          // pinmux of JGPIO 1
	unsigned int JGPIO_2:1;          // pinmux of JGPIO 2
	unsigned int JGPIO_3:1;          // pinmux of JGPIO 3
	unsigned int JGPIO_4:1;          // pinmux of JGPIO 4
	unsigned int JGPIO_5:1;          // pinmux of JGPIO 5
	unsigned int reserved0:26;
	} bit;
};

//
// 0xA8 PGPIO Control Register
//
#define TOP_REGPGPIO0_OFS                    0xA8
union TOP_REGPGPIO0 {
	uint32_t reg;
	struct {
	unsigned int PGPIO_0:1;          // pinmux of PGPIO 0
	unsigned int PGPIO_1:1;          // pinmux of PGPIO 1
	unsigned int PGPIO_2:1;          // pinmux of PGPIO 2
	unsigned int PGPIO_3:1;          // pinmux of PGPIO 3
	unsigned int PGPIO_4:1;          // pinmux of PGPIO 4
	unsigned int PGPIO_5:1;          // pinmux of PGPIO 5
	unsigned int PGPIO_6:1;          // pinmux of PGPIO 6
	unsigned int PGPIO_7:1;          // pinmux of PGPIO 7
	unsigned int PGPIO_8:1;          // pinmux of PGPIO 8
	unsigned int PGPIO_9:1;          // pinmux of PGPIO 9
	unsigned int PGPIO_10:1;          // pinmux of PGPIO 10
	unsigned int PGPIO_11:1;          // pinmux of PGPIO 11
	unsigned int PGPIO_12:1;          // pinmux of PGPIO 12
	unsigned int PGPIO_13:1;          // pinmux of PGPIO 13
	unsigned int PGPIO_14:1;          // pinmux of PGPIO 14
	unsigned int PGPIO_15:1;          // pinmux of PGPIO 15
	unsigned int PGPIO_16:1;          // pinmux of PGPIO 16
	unsigned int PGPIO_17:1;          // pinmux of PGPIO 17
	unsigned int PGPIO_18:1;          // pinmux of PGPIO 18
	unsigned int PGPIO_19:1;          // pinmux of PGPIO 19
	unsigned int PGPIO_20:1;          // pinmux of PGPIO 20
	unsigned int PGPIO_21:1;          // pinmux of PGPIO 21
	unsigned int PGPIO_22:1;          // pinmux of PGPIO 22
	unsigned int PGPIO_23:1;          // pinmux of PGPIO 23
	unsigned int PGPIO_24:1;          // pinmux of PGPIO 24
	unsigned int PGPIO_25:1;          // pinmux of PGPIO 25
	unsigned int PGPIO_26:1;          // pinmux of PGPIO 26
	unsigned int PGPIO_27:1;          // pinmux of PGPIO 27
	unsigned int PGPIO_28:1;          // pinmux of PGPIO 28
	unsigned int PGPIO_29:1;          // pinmux of PGPIO 29
	unsigned int PGPIO_30:1;          // pinmux of PGPIO 30
	unsigned int PGPIO_31:1;          // pinmux of PGPIO 31
	} bit;
};

//
// 0xAC PGPIO1 Control Register
//
#define TOP_REGPGPIO1_OFS                    0xAC
union TOP_REGPGPIO1 {
	uint32_t reg;
	struct {
	unsigned int PGPIO_32:1;          // pinmux of PGPIO 32
	unsigned int PGPIO_33:1;          // pinmux of PGPIO 33
	unsigned int PGPIO_34:1;          // pinmux of PGPIO 34
	unsigned int PGPIO_35:1;          // pinmux of PGPIO 35
	unsigned int PGPIO_36:1;          // pinmux of PGPIO 36
	unsigned int PGPIO_37:1;          // pinmux of PGPIO 37
	unsigned int PGPIO_38:1;          // pinmux of PGPIO 38
	unsigned int PGPIO_39:1;          // pinmux of PGPIO 39
	unsigned int PGPIO_40:1;          // pinmux of PGPIO 40
	unsigned int PGPIO_41:1;          // pinmux of PGPIO 41
	unsigned int PGPIO_42:1;          // pinmux of PGPIO 42
	unsigned int PGPIO_43:1;          // pinmux of PGPIO 43
	unsigned int PGPIO_44:1;          // pinmux of PGPIO 44
	unsigned int reserved0:19;
	} bit;
};

//
// 0xB0 EGPIO Control Register
//
#define TOP_REGEGPIO0_OFS                    0xB0
union TOP_REGEGPIO0 {
	uint32_t reg;
	struct {
	unsigned int EGPIO_0:1;          // pinmux of EGPIO 0
	unsigned int EGPIO_1:1;          // pinmux of EGPIO 1
	unsigned int EGPIO_2:1;          // pinmux of EGPIO 2
	unsigned int EGPIO_3:1;          // pinmux of EGPIO 3
	unsigned int EGPIO_4:1;          // pinmux of EGPIO 4
	unsigned int EGPIO_5:1;          // pinmux of EGPIO 5
	unsigned int EGPIO_6:1;          // pinmux of EGPIO 6
	unsigned int EGPIO_7:1;          // pinmux of EGPIO 7
	unsigned int EGPIO_8:1;          // pinmux of EGPIO 8
	unsigned int EGPIO_9:1;          // pinmux of EGPIO 9
	unsigned int EGPIO_10:1;          // pinmux of EGPIO 10
	unsigned int EGPIO_11:1;          // pinmux of EGPIO 11
	unsigned int EGPIO_12:1;          // pinmux of EGPIO 12
	unsigned int EGPIO_13:1;          // pinmux of EGPIO 13
	unsigned int EGPIO_14:1;          // pinmux of EGPIO 14
	unsigned int EGPIO_15:1;          // pinmux of EGPIO 15
	unsigned int EGPIO_16:1;          // pinmux of EGPIO 16
	unsigned int EGPIO_17:1;          // pinmux of EGPIO 17
	unsigned int EGPIO_18:1;          // pinmux of EGPIO 18
	unsigned int EGPIO_19:1;          // pinmux of EGPIO 19
	unsigned int EGPIO_20:1;          // pinmux of EGPIO 20
	unsigned int EGPIO_21:1;          // pinmux of EGPIO 21
	unsigned int EGPIO_22:1;          // pinmux of EGPIO 22
	unsigned int EGPIO_23:1;          // pinmux of EGPIO 23
	unsigned int EGPIO_24:1;          // pinmux of EGPIO 24
	unsigned int EGPIO_25:1;          // pinmux of EGPIO 25
	unsigned int EGPIO_26:1;          // pinmux of EGPIO 26
	unsigned int EGPIO_27:1;          // pinmux of EGPIO 27
	unsigned int EGPIO_28:1;          // pinmux of EGPIO 28
	unsigned int EGPIO_29:1;          // pinmux of EGPIO 29
	unsigned int EGPIO_30:1;          // pinmux of EGPIO 30
	unsigned int EGPIO_31:1;          // pinmux of EGPIO 31
	} bit;
};

//
// 0xB4 DGPIO Control Register
//
#define TOP_REGDGPIO0_OFS                    0xB4
union TOP_REGDGPIO0 {
	uint32_t reg;
	struct {
	unsigned int DGPIO_0:1;          // pinmux of DGPIO 0
	unsigned int DGPIO_1:1;          // pinmux of DGPIO 1
	unsigned int DGPIO_2:1;          // pinmux of DGPIO 2
	unsigned int DGPIO_3:1;          // pinmux of DGPIO 3
	unsigned int DGPIO_4:1;          // pinmux of DGPIO 4
	unsigned int DGPIO_5:1;          // pinmux of DGPIO 5
	unsigned int DGPIO_6:1;          // pinmux of DGPIO 6
	unsigned int DGPIO_7:1;          // pinmux of DGPIO 7
	unsigned int DGPIO_8:1;          // pinmux of DGPIO 8
	unsigned int DGPIO_9:1;          // pinmux of DGPIO 9
	unsigned int DGPIO_10:1;          // pinmux of DGPIO 10
	unsigned int DGPIO_11:1;          // pinmux of DGPIO 11
	unsigned int reserved0:20;
	} bit;
};

//
// 0xC0 SGPIO Control Register
//
#define TOP_REGSGPIO0_OFS                    0xC0
union TOP_REGSGPIO0 {
	uint32_t reg;
	struct {
	unsigned int SGPIO_0:1;          // pinmux of SGPIO 0
	unsigned int SGPIO_1:1;          // pinmux of SGPIO 1
	unsigned int SGPIO_2:1;          // pinmux of SGPIO 2
	unsigned int SGPIO_3:1;          // pinmux of SGPIO 3
	unsigned int SGPIO_4:1;          // pinmux of SGPIO 4
	unsigned int SGPIO_5:1;          // pinmux of SGPIO 5
	unsigned int SGPIO_6:1;          // pinmux of SGPIO 6
	unsigned int SGPIO_7:1;          // pinmux of SGPIO 7
	unsigned int SGPIO_8:1;          // pinmux of SGPIO 8
	unsigned int SGPIO_9:1;          // pinmux of SGPIO 9
	unsigned int SGPIO_10:1;          // pinmux of SGPIO 10
	unsigned int SGPIO_11:1;          // pinmux of SGPIO 11
	unsigned int SGPIO_12:1;          // pinmux of SGPIO 12
	unsigned int SGPIO_13:1;          // pinmux of SGPIO 13
	unsigned int SGPIO_14:1;          // pinmux of SGPIO 14
	unsigned int SGPIO_15:1;          // pinmux of SGPIO 15
	unsigned int SGPIO_16:1;          // pinmux of SGPIO 16
	unsigned int SGPIO_17:1;          // pinmux of SGPIO 17
	unsigned int SGPIO_18:1;          // pinmux of SGPIO 18
	unsigned int SGPIO_19:1;          // pinmux of SGPIO 19
	unsigned int SGPIO_20:1;          // pinmux of SGPIO 20
	unsigned int SGPIO_21:1;          // pinmux of SGPIO 21
	unsigned int SGPIO_22:1;          // pinmux of SGPIO 22
	unsigned int SGPIO_23:1;          // pinmux of SGPIO 23
	unsigned int SGPIO_24:1;          // pinmux of SGPIO 24
	unsigned int SGPIO_25:1;          // pinmux of SGPIO 25
	unsigned int SGPIO_26:1;          // pinmux of SGPIO 26
	unsigned int SGPIO_27:1;          // pinmux of SGPIO 27
	unsigned int SGPIO_28:1;          // pinmux of SGPIO 28
	unsigned int SGPIO_29:1;          // pinmux of SGPIO 29
	unsigned int SGPIO_30:1;          // pinmux of SGPIO 30
	unsigned int SGPIO_31:1;          // pinmux of SGPIO 31
	} bit;
};

//
// 0xC4 SGPIO Control 1 Register
//
#define TOP_REGSGPIO1_OFS                    0xC4
union TOP_REGSGPIO1 {
	uint32_t reg;
	struct {
	unsigned int SGPIO_32:1;          // pinmux of SGPIO 32
	unsigned int SGPIO_33:1;          // pinmux of SGPIO 33
	unsigned int SGPIO_34:1;          // pinmux of SGPIO 34
	unsigned int SGPIO_35:1;          // pinmux of SGPIO 35
	unsigned int SGPIO_36:1;          // pinmux of SGPIO 36
	unsigned int SGPIO_37:1;          // pinmux of SGPIO 37
	unsigned int SGPIO_38:1;          // pinmux of SGPIO 38
	unsigned int SGPIO_39:1;          // pinmux of SGPIO 39
	unsigned int SGPIO_40:1;          // pinmux of SGPIO 40
	unsigned int SGPIO_41:1;          // pinmux of SGPIO 41
	unsigned int SGPIO_42:1;          // pinmux of SGPIO 42
	unsigned int SGPIO_43:1;          // pinmux of SGPIO 43
	unsigned int SGPIO_44:1;          // pinmux of SGPIO 44
	unsigned int SGPIO_45:1;          // pinmux of SGPIO 45
	unsigned int SGPIO_46:1;          // pinmux of SGPIO 46
	unsigned int SGPIO_47:1;          // pinmux of SGPIO 47
	unsigned int SGPIO_48:1;          // pinmux of SGPIO 48
	unsigned int SGPIO_49:1;          // pinmux of SGPIO 49
	unsigned int SGPIO_50:1;          // pinmux of SGPIO 50
	unsigned int SGPIO_51:1;          // pinmux of SGPIO 51
	unsigned int SGPIO_52:1;          // pinmux of SGPIO 52
	unsigned int SGPIO_53:1;          // pinmux of SGPIO 53
	unsigned int SGPIO_54:1;          // pinmux of SGPIO 54
	unsigned int SGPIO_55:1;          // pinmux of SGPIO 55
	unsigned int SGPIO_56:1;          // pinmux of SGPIO 56
	unsigned int SGPIO_57:1;          // pinmux of SGPIO 57
	unsigned int SGPIO_58:1;          // pinmux of SGPIO 58
	unsigned int SGPIO_59:1;          // pinmux of SGPIO 59
	unsigned int SGPIO_60:1;          // pinmux of SGPIO 60
	unsigned int SGPIO_61:1;          // pinmux of SGPIO 61
	unsigned int SGPIO_62:1;          // pinmux of SGPIO 62
	unsigned int SGPIO_63:1;          // pinmux of SGPIO 63
	} bit;
};

//
// 0xC8 SGPIO Control 2 Register
//
#define TOP_REGSGPIO2_OFS                    0xC8
union TOP_REGSGPIO2 {
	uint32_t reg;
	struct {
	unsigned int SGPIO_64:1;          // pinmux of SGPIO 64
	unsigned int SGPIO_65:1;          // pinmux of SGPIO 65
	unsigned int SGPIO_66:1;          // pinmux of SGPIO 66
	unsigned int SGPIO_67:1;          // pinmux of SGPIO 67
	unsigned int SGPIO_68:1;          // pinmux of SGPIO 68
	unsigned int SGPIO_69:1;          // pinmux of SGPIO 69
	unsigned int SGPIO_70:1;          // pinmux of SGPIO 70
	unsigned int SGPIO_71:1;          // pinmux of SGPIO 71
	unsigned int SGPIO_72:1;          // pinmux of SGPIO 72
	unsigned int SGPIO_73:1;          // pinmux of SGPIO 73
	unsigned int SGPIO_74:1;          // pinmux of SGPIO 74
	unsigned int SGPIO_75:1;          // pinmux of SGPIO 75
	unsigned int SGPIO_76:1;          // pinmux of SGPIO 76
	unsigned int SGPIO_77:1;          // pinmux of SGPIO 77
	unsigned int SGPIO_78:1;          // pinmux of SGPIO 78
	unsigned int SGPIO_79:1;          // pinmux of SGPIO 79
	unsigned int SGPIO_80:1;          // pinmux of SGPIO 80
	unsigned int SGPIO_81:1;          // pinmux of SGPIO 81
	unsigned int SGPIO_82:1;          // pinmux of SGPIO 82
	unsigned int SGPIO_83:1;          // pinmux of SGPIO 83
	unsigned int reserved0:12;
	} bit;
};

//
// 0xE0 BGPIO Control Register
//
#define TOP_REGBGPIO0_OFS                    0xE0
union TOP_REGBGPIO0 {
	uint32_t reg;
	struct {
	unsigned int BGPIO_0:1;          // pinmux of BGPIO 0
	unsigned int BGPIO_1:1;          // pinmux of BGPIO 1
	unsigned int BGPIO_2:1;          // pinmux of BGPIO 2
	unsigned int BGPIO_3:1;          // pinmux of BGPIO 3
	unsigned int BGPIO_4:1;          // pinmux of BGPIO 4
	unsigned int BGPIO_5:1;          // pinmux of BGPIO 5
	unsigned int BGPIO_6:1;          // pinmux of BGPIO 6
	unsigned int BGPIO_7:1;          // pinmux of BGPIO 7
	unsigned int BGPIO_8:1;          // pinmux of BGPIO 8
	unsigned int BGPIO_9:1;          // pinmux of BGPIO 9
	unsigned int BGPIO_10:1;          // pinmux of BGPIO 10
	unsigned int BGPIO_11:1;          // pinmux of BGPIO 11
	unsigned int BGPIO_12:1;          // pinmux of BGPIO 12
	unsigned int BGPIO_13:1;          // pinmux of BGPIO 13
	unsigned int BGPIO_14:1;          // pinmux of BGPIO 14
	unsigned int BGPIO_15:1;          // pinmux of BGPIO 15
	unsigned int BGPIO_16:1;          // pinmux of BGPIO 16
	unsigned int reserved0:15;
	} bit;
};

//
//0xF0 Version Code Register
//
#define TOP_VERSION_REG_OFS                 0xF0
union TOP_VERSION_REG {
	uint32_t reg;
	struct {
	unsigned int reserved0:8;
	unsigned int ROM_CODE_VERSION:4; // ROM Code Version
	unsigned int ECO_VERSION:4;      // ECO Version
	unsigned int CHIP_ID:16;         // Chip ID
	} bit;
};

#endif
