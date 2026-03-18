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
#define MUX_6                    0x06
#define MUX_7                    0x07
#define MUX_8                    0x08
#define MUX_9                    0x09
#define MUX_A                    0x0A
#define MUX_B                    0x0B

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
	unsigned int rev1:26;
	} bit;
};


//
//0x04 TOP Control Register 1
//
#define TOP_REG1_OFS                        0x04
union TOP_REG1 {
	uint32_t reg;
	struct {
	unsigned int FSPI_EN:1;          // FSPI EN
	unsigned int FSPI_BUS_WIDTH:1;   // FSPI Bus Width
	unsigned int FSPI_CS1_EN:1;      // FSPI CS1 En
	unsigned int rev0:1;
	unsigned int SDIO:2;             // SDIO selection
	unsigned int SPI:2;              // SPI Selection
	unsigned int SPI_BUS_WIDTH:1;    // SPI Bus Width
	unsigned int rev1:7;
	unsigned int EJTAG_EN:1;         // EJTAG enable/disable
	unsigned int EXTROM_EN:1;        // External ROM enable/disable
	unsigned int rev2:14;
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
	unsigned int ETH_RST:1;          // ETH RST (not used)
	unsigned int rev3:1;
	unsigned int ETH_MDC_MDIO:1;     // ETH MDC MDIO
	unsigned int rev4:3;
	unsigned int ETH2_REFCLK:1;      // ETH2 REFCLK
	unsigned int rev5:1;
	unsigned int ETH2_RST:1;         // ETH RST  (not used)
	unsigned int rev6:1;
	unsigned int ETH2_MDC_MDIO:1;    // ETH2 MDC MDIO
	unsigned int rev7:10;
	unsigned int ETH_PHY_SEL:1;      // ETH PHY SEL
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
	unsigned int I2C2:3;             // I2C2
	unsigned int rev1:1;
	unsigned int I2C3:2;             // I2C3
	unsigned int rev2:2;
	unsigned int I2C_HDMI:1;         // I2C_HDMI
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
	unsigned int ETH_ACT_LED:2;
	unsigned int rev0:2;
	unsigned int ETH_LINK_LED:2;
	unsigned int rev1:10;
	unsigned int EXT_CLK:1;          // EXT CLK
	unsigned int EXT2_CLK:1;         // EXT2 CLK
	unsigned int EXT3_CLK:1;         // EXT CLK 2'nd pinmux
	unsigned int EXT4_CLK:1;         // EXT2 CLK 2'nd pinmux
	unsigned int IRDA:4;             // IRDA
	unsigned int SATA_LED:2;         // SATA LED
	unsigned int SATA2_LED:2;        // SATA2 LED
	unsigned int rev2:2;
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
	unsigned int rev2:27;
	} bit;
};

//
//0x1C TOP Control Register 7
//
#define TOP_REG7_OFS                        0x1C
union TOP_REG7 {
	uint32_t reg;
	struct {
	unsigned int UART:1;             // UART
	unsigned int rev0:3;
	unsigned int UART2:2;            // UART2
	unsigned int rev1:2;
	unsigned int UART3:1;            // UART3
	unsigned int rev2:3;
	unsigned int UART4:3;            // UART4
	unsigned int rev3:1;
	unsigned int UART5:2;            // UART5 (not used)
	unsigned int rev4:2;
	unsigned int UART_CTS_RTS:2;     // UART CTS/RTS
	unsigned int UART2_CTS_RTS:2;    // UART2 CTS/RTS
	unsigned int UART3_RTS:1;        // UART3 RTS
	unsigned int rev5:7;
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
	unsigned int RAL_SEL:4;          // RAM SEL (not used)
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
	unsigned int rev0:8;
	unsigned int CAP_CLK:2;
	unsigned int CAP1_CLK:2;
	unsigned int CAP2_CLK:2;
	unsigned int CAP3_CLK:2;
	unsigned int rev1:8;
	} bit;
};

//0x28 TOP Control Register 10
//
#define TOP_REG10_OFS                        0x28
union TOP_REG10 {
	uint32_t reg;
	struct {
	unsigned int rev0:32;
	} bit;
};

//0x2C TOP Control Register 11
//
#define TOP_REG11_OFS                        0x2C
union TOP_REG11 {
	uint32_t reg;
	struct {
	unsigned int I2S:1;
	unsigned int rev0:1;
	unsigned int I2S_MCLK:1;
	unsigned int rev1:1;
	unsigned int I2S_TX:1;
	unsigned int rev2:1;
	unsigned int I2S_RX:1;
	unsigned int rev3:1;
	unsigned int I2S2:1;
	unsigned int rev4:1;
	unsigned int I2S2_MCLK:1;
	unsigned int rev5:1;
	unsigned int I2S2_TX:1;
	unsigned int rev6:1;
	unsigned int I2S2_RX:1;
	unsigned int rev7:1;
	unsigned int I2S3:2;
	unsigned int I2S3_MCLK:2;
	unsigned int I2S3_TX:2;
	unsigned int I2S3_RX:2;
	unsigned int I2S4:2;
	unsigned int I2S4_MCLK:2;
	unsigned int I2S4_TX:2;
	unsigned int I2S4_RX:2;
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
	unsigned int reserved0:24;
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
	unsigned int reserved0:32;
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
	unsigned int rev0:16;
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
	unsigned int reserved0:22;
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
	unsigned int rev0:28;
	} bit;
};

//
// 0xC8 SGPIO Control 2 Register
//
#define TOP_REGSGPIO2_OFS                    0xC8
union TOP_REGSGPIO2 {
	uint32_t reg;
	struct {
	unsigned int reserved0:32;
	} bit;
};

//
// 0xE0 BGPIO Control Register
//
#define TOP_REGBGPIO0_OFS                    0xE0
union TOP_REGBGPIO0 {
	uint32_t reg;
	struct {
	unsigned int reserved0:32;
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

// 0xFC Debug Port Register
#define TOP_REG_DEBUG_OFS                    0xFC
union TOP_REG_DEBUG {
	uint32_t reg;
	struct {
	unsigned int DEBUG_SEL:8;                // Debug Port Selection
	unsigned int DEBUG_OUTSEL:2;             // Debug Port output select
	unsigned int reserved1:22;
	} bit;
};

#endif
