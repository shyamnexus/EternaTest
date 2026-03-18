/*
	Pinmux module internal header file

	@file		top_reg.h
	@ingroup
	@note		Nothing

	Copyright   Novatek Microelectronics Corp. 2024.  All rights reserved
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#ifndef _PINMUX_INT_H
#define _PINMUX_INT_H

#include <linux/soc/nvt/rcw_macro.h>
#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000

#define MUX_0                    0x00
#define MUX_1                    0x01
#define MUX_2                    0x02
#define MUX_3                    0x03
#define MUX_4                    0x04
#define MUX_5                    0x05
#define MUX_6                    0x06
#define MUX_7                    0x07
#define MUX_8                    0x08
#define MUX_9                    0x09
#define MUX_10                   0x0A

/*
	GPIO_ID_ENUM
*/
typedef enum
{
	GPIO_ID_EMUM_FUNC,                  //< pinmux is mapping to function
	GPIO_ID_EMUM_GPIO,                  //< pinmux is mapping to gpio

	ENUM_DUMMY4WORD(GPIO_ID_ENUM)
} GPIO_ID_ENUM;

// 0x00 TOP Control Register 0 (BOOT)
#define TOP_REG0_OFS                        0x00
union TOP_REG0 {
	uint32_t reg;
	struct {
	unsigned int BOOT_SRC:4;                // Boot Source Selection
	unsigned int MPLL_CLKSELL:1;            // PLL clock output mux
	unsigned int EJTAG_SEL:1;               // EJTAG select
	unsigned int ETH_MODE_SEL:2;            // Ethernet mode select
	unsigned int DEBUG_MODE_SEL:2;          // MIPS debug mode select
	unsigned int WDT_FAIL_RESET_EN:1;       // WDT fail reset function enable
	unsigned int VDD33_AUD_SEL:1;           // AUD power voltage select
	unsigned int VDD33_EMMC_SEL:1;          // EMMC power voltage select
	unsigned int PCIE_BOOT_SEL:1;           // PCIE boot select
	unsigned int LANE_MODE_SEL:2;           // PCIE Lane mode select
	unsigned int PCIE_REFCLK_BOOT_SEL:1;    // PCIE reference clock select
	unsigned int SD_MUX_SEL:1;              // SDIO mux select
	unsigned int reserved0:14;
	} bit;
};

// 0x04 TOP Control Register 1 (Storage)
#define TOP_REG1_OFS                        0x04
union TOP_REG1 {
	uint32_t reg;
	struct {
	unsigned int FSPI_EN:2;                 // FSPI enable control
	unsigned int FSPI_CS1_EN:1;             // FSPI CS1 enable control
	unsigned int reserved0:1;
	unsigned int SDIO_EN:2;                 // SDIO enable control
	unsigned int reserved1:2;
	unsigned int SDIO2_EN:1;                // SDIO2 enable control
	unsigned int reserved2:1;
	unsigned int SDIO2_BUS_WIDTH:1;         // Select SDIO2 bus width
	unsigned int SDIO2_DS_EN:1;             // Select SDIO2 data strobe
	unsigned int reserved3:4;
	unsigned int EJTAG_EN:1;                // EJTAG enable control
	unsigned int EXTROM_EN:1;		        // EXTROM enable control
	unsigned int reserved4:14;
	} bit;
};

// 0x08 TOP Control Register 2 (LCD)
#define TOP_REG2_OFS                        0x08
union TOP_REG2 {
	uint32_t reg;
	struct {
	unsigned int LCD_TYPE:4;                // Pinmux of LCD interface
	unsigned int PLCD_DE:1;                 // Pinmux of DE (for parallel LCD)
	unsigned int CCIR_DATA_WIDTH:1;         // CCIR data width
	unsigned int LCD_HS_VS:1;               // LCD H_sync / V_sync select
	unsigned int CCIR_FIELD:1;              // CCIR FIELD select
	unsigned int reserved0:24;
	} bit;
};

// 0x0C TOP Control Register 3 (ETH)
#define TOP_REG3_OFS                        0x0C
union TOP_REG3 {
	uint32_t reg;
	struct {
	unsigned int ETH:4;
	unsigned int ETH2:4;
	unsigned int ETH_EXT_PHY_CLK:1;
	unsigned int reserved0:7;
	unsigned int ETH2_EXT_PHY_CLK:1;
	unsigned int reserved1:15;
	} bit;
};

// 0x10 TOP Control Register 4 (I2C)
#define TOP_REG4_OFS                        0x10
union TOP_REG4 {
	uint32_t reg;
	struct {
	unsigned int I2C:4;
	unsigned int I2C2:4;
	unsigned int I2C3:4;
	unsigned int I2C4:4;
	unsigned int I2C5:4;
	unsigned int I2C6:4;
	unsigned int I2C7:4;
	unsigned int I2C8:4;
	} bit;
};

// 0x14 TOP Control Register 5 (EXT_CLK)
#define TOP_REG5_OFS                        0x14
union TOP_REG5 {
	uint32_t reg;
	struct {
	unsigned int EXT_CLK:4;
	unsigned int EXT2_CLK:4;
	unsigned int reserved0:8;
	unsigned int RTC_CLK:2;
	unsigned int RTC_DIV_OUT:1;
	unsigned int RTC_EXT_CLK:1;
	unsigned int reserved1:12;
	} bit;
};

// 0x18 TOP Control Register 6 (PWM)
#define TOP_REG6_OFS                        0x18
union TOP_REG6 {
	uint32_t reg;
	struct {
	unsigned int PWM:4;
	unsigned int PWM2:4;
	unsigned int PWM3:4;
	unsigned int PWM4:4;
	unsigned int PWM5:4;
	unsigned int PWM6:4;
	unsigned int reserved0:8;
	} bit;
};

//0x20 TOP Control Register 7 (System)
#define TOP_REG7_OFS                        0x20
union TOP_REG7 {
	uint32_t reg;
	struct {
	unsigned int RAM_SRC_SELECT:1;          // RAM SRC Select
	unsigned int reserved0:3;
	unsigned int RAM_SELECT:4;
	unsigned int reserved1:20;
	unsigned int CLKOUT_12M:1;
	unsigned int reserved2:3;
	} bit;
};

// 0x30 TOP Control Register 8 (Audio)
#define TOP_REG8_OFS                       0x30
union TOP_REG8 {
	uint32_t reg;
	struct {
	unsigned int I2S:2;
	unsigned int I2S_MCLK:2;
	unsigned int I2S2:2;
	unsigned int I2S2_MCLK:2;
	unsigned int I2S3:2;
	unsigned int I2S3_MCLK:2;
	unsigned int I2S4:2;
	unsigned int I2S4_MCLK:2;
	unsigned int reserved0:8;
	unsigned int HDMI_CEC:2;
	unsigned int HDMI2_CEC:2;
	unsigned int HDMI3_CEC:2;
	unsigned int reserved1:2;
	} bit;
};

// 0x34 TOP Control Register 9 (UART)
#define TOP_REG9_OFS                       0x34
union TOP_REG9 {
	uint32_t reg;
	struct {
	unsigned int UART:4;
	unsigned int UART2:4;
	unsigned int UART3:4;
	unsigned int UART4:4;
	unsigned int UART5:4;
	unsigned int UART_RTSCTS:2;
	unsigned int UART2_RTSCTS:2;
	unsigned int UART3_RTSCTS:2;
	unsigned int reserved0:6;
	} bit;
};

// 0x40 TOP Control Register 10 (Remote/SDP)
#define TOP_REG10_OFS                       0x40
union TOP_REG10 {
	uint32_t reg;
	struct {
	unsigned int REMOTE:8;
	unsigned int reserved0:8;
	unsigned int SDP:2;
	unsigned int reserved1:2;
	unsigned int SDP_RDY:2;
	unsigned int reserved2:10;
	} bit;
};

// 0x44 TOP Control Register 11 (SPI)
#define TOP_REG11_OFS                       0x44
union TOP_REG11 {
	uint32_t reg;
	struct {
	unsigned int SPI:4;
	unsigned int SPI2:4;
	unsigned int reserved0:12;
	unsigned int SPI_BUS_WIDTH:1;
	unsigned int SPI2_BUS_WIDTH:1;
	unsigned int reserved1:2;
	unsigned int SPI_RDY:1;
	unsigned int SPI2_RDY:1;
	unsigned int reserved2:6;
	} bit;
};

// 0x54 TOP Control Register 13 (SATA)
#define TOP_REG13_OFS                       0x54
union TOP_REG13 {
	uint32_t reg;
	struct {
	unsigned int SATA_LED:4;
	unsigned int SATA2_LED:4;
	unsigned int SATA3_LED:4;
	unsigned int SATA4_LED:4;
	unsigned int SATA5_LED:4;
	unsigned int SATA6_LED:4;
	unsigned int reserved0:8;
	} bit;
};

// 0x60 TOP Control Register 14 (PCIE)
#define TOP_REG14_OFS                       0x60
union TOP_REG14 {
	uint32_t reg;
	struct {
	unsigned int PCIE_MODE_SEL:1;
	unsigned int PCIE2_MODE_SEL:1;
	unsigned int PCIE3_MODE_SEL:1;
	unsigned int PCIE4_MODE_SEL:1;
	unsigned int PCIE_RESETN_RELEASE:1;
	unsigned int reserved0:3;
	unsigned int PCIE_REFCLK_SRC:1;
	unsigned int PCIE_REFCLK_PAD_OUT_EN:1;
	unsigned int reserved1:22;
	} bit;
};

// 0xA0 CGPIO Control Register 0 (C_GPIO)
#define TOP_REGCGPIO0_OFS                   0xA0
union TOP_REGCGPIO0 {
	uint32_t reg;
	struct {
	unsigned int CGPIO_0:1;
	unsigned int CGPIO_1:1;
	unsigned int CGPIO_2:1;
	unsigned int CGPIO_3:1;
	unsigned int CGPIO_4:1;
	unsigned int CGPIO_5:1;
	unsigned int CGPIO_6:1;
	unsigned int CGPIO_7:1;
	unsigned int CGPIO_8:1;
	unsigned int CGPIO_9:1;
	unsigned int CGPIO_10:1;
	unsigned int CGPIO_11:1;
	unsigned int CGPIO_12:1;
	unsigned int CGPIO_13:1;
	unsigned int CGPIO_14:1;
	unsigned int CGPIO_15:1;
	unsigned int CGPIO_16:1;
	unsigned int CGPIO_17:1;
	unsigned int reserved0:14;
	} bit;
};

// 0xA4 JPGPIO Control Register 0 (J_GPIO)
#define TOP_REGJGPIO0_OFS                   0xA4
union TOP_REGJGPIO0 {
	uint32_t reg;
	struct {
	unsigned int JGPIO_0:1;
	unsigned int JGPIO_1:1;
	unsigned int JGPIO_2:1;
	unsigned int JGPIO_3:1;
	unsigned int JGPIO_4:1;
	unsigned int JGPIO_5:1;
	unsigned int reserved0:26;
	} bit;
};

// 0xA8 PGPIO Control Register 0 (P_GPIO[1/2])
#define TOP_REGPGPIO0_OFS                   0xA8
union TOP_REGPGPIO0 {
	uint32_t reg;
	struct {
	unsigned int PGPIO_0:1;
	unsigned int PGPIO_1:1;
	unsigned int PGPIO_2:1;
	unsigned int PGPIO_3:1;
	unsigned int PGPIO_4:1;
	unsigned int PGPIO_5:1;
	unsigned int PGPIO_6:1;
	unsigned int PGPIO_7:1;
	unsigned int PGPIO_8:1;
	unsigned int PGPIO_9:1;
	unsigned int PGPIO_10:1;
	unsigned int PGPIO_11:1;
	unsigned int PGPIO_12:1;
	unsigned int PGPIO_13:1;
	unsigned int PGPIO_14:1;
	unsigned int PGPIO_15:1;
	unsigned int PGPIO_16:1;
	unsigned int PGPIO_17:1;
	unsigned int PGPIO_18:1;
	unsigned int PGPIO_19:1;
	unsigned int PGPIO_20:1;
	unsigned int PGPIO_21:1;
	unsigned int PGPIO_22:1;
	unsigned int PGPIO_23:1;
	unsigned int PGPIO_24:1;
	unsigned int PGPIO_25:1;
	unsigned int PGPIO_26:1;
	unsigned int PGPIO_27:1;
	unsigned int PGPIO_28:1;
	unsigned int PGPIO_29:1;
	unsigned int PGPIO_30:1;
	unsigned int PGPIO_31:1;
	} bit;
};

// 0xAC PGPIO Control Register 1 (P_GPIO[2/2])
#define TOP_REGPGPIO1_OFS                   0xAC
union TOP_REGPGPIO1 {
	uint32_t reg;
	struct {
	unsigned int PGPIO_32:1;
	unsigned int PGPIO_33:1;
	unsigned int PGPIO_34:1;
	unsigned int PGPIO_35:1;
	unsigned int PGPIO_36:1;
	unsigned int PGPIO_37:1;
	unsigned int PGPIO_38:1;
	unsigned int PGPIO_39:1;
	unsigned int PGPIO_40:1;
	unsigned int PGPIO_41:1;
	unsigned int PGPIO_42:1;
	unsigned int PGPIO_43:1;
	unsigned int PGPIO_44:1;
	unsigned int PGPIO_45:1;
	unsigned int PGPIO_46:1;
	unsigned int PGPIO_47:1;
	unsigned int reserved0:16;
	} bit;
};

// 0xB0 EGPIO Control Register 0 (E_GPIO)
#define TOP_REGEGPIO0_OFS                   0xB0
union TOP_REGEGPIO0 {
	uint32_t reg;
	struct {
	unsigned int EGPIO_0:1;
	unsigned int EGPIO_1:1;
	unsigned int EGPIO_2:1;
	unsigned int EGPIO_3:1;
	unsigned int EGPIO_4:1;
	unsigned int EGPIO_5:1;
	unsigned int EGPIO_6:1;
	unsigned int EGPIO_7:1;
	unsigned int EGPIO_8:1;
	unsigned int EGPIO_9:1;
	unsigned int EGPIO_10:1;
	unsigned int EGPIO_11:1;
	unsigned int EGPIO_12:1;
	unsigned int EGPIO_13:1;
	unsigned int EGPIO_14:1;
	unsigned int EGPIO_15:1;
	unsigned int EGPIO_16:1;
	unsigned int EGPIO_17:1;
	unsigned int EGPIO_18:1;
	unsigned int EGPIO_19:1;
	unsigned int EGPIO_20:1;
	unsigned int EGPIO_21:1;
	unsigned int EGPIO_22:1;
	unsigned int EGPIO_23:1;
	unsigned int EGPIO_24:1;
	unsigned int EGPIO_25:1;
	unsigned int EGPIO_26:1;
	unsigned int EGPIO_27:1;
	unsigned int EGPIO_28:1;
	unsigned int EGPIO_29:1;
	unsigned int EGPIO_30:1;
	unsigned int EGPIO_31:1;
	} bit;
};

// 0xB4 DGPIO Control Register 0 (D_GPIO)
#define TOP_REGDGPIO0_OFS                   0xB4
union TOP_REGDGPIO0 {
	uint32_t reg;
	struct {
	unsigned int DGPIO_0:1;
	unsigned int DGPIO_1:1;
	unsigned int DGPIO_2:1;
	unsigned int DGPIO_3:1;
	unsigned int DGPIO_4:1;
	unsigned int DGPIO_5:1;
	unsigned int DGPIO_6:1;
	unsigned int DGPIO_7:1;
	unsigned int DGPIO_8:1;
	unsigned int DGPIO_9:1;
	unsigned int DGPIO_10:1;
	unsigned int DGPIO_11:1;
	unsigned int reserved0:20;
	} bit;
};

// 0xB8 ~ 0xDC Reserved

// 0xE0 BGPIO Control Register 0 (B_GPIO)
#define TOP_REGBGPIO0_OFS                    0xE0
union TOP_REGBGPIO0 {
	uint32_t reg;
	struct {
	unsigned int BGPIO_0:1;
	unsigned int BGPIO_1:1;
	unsigned int BGPIO_2:1;
	unsigned int BGPIO_3:1;
	unsigned int BGPIO_4:1;
	unsigned int BGPIO_5:1;
	unsigned int BGPIO_6:1;
	unsigned int BGPIO_7:1;
	unsigned int BGPIO_8:1;
	unsigned int BGPIO_9:1;
	unsigned int BGPIO_10:1;
	unsigned int BGPIO_11:1;
	unsigned int BGPIO_12:1;
	unsigned int BGPIO_13:1;
	unsigned int BGPIO_14:1;
	unsigned int BGPIO_15:1;
	unsigned int BGPIO_16:1;
	unsigned int reserved0:15;
	} bit;
};

// 0xE8 AGPIO Control Register 0 (A_GPIO)
#define TOP_REGAGPIO0_OFS                    0xE8
union TOP_REGAGPIO0 {
	uint32_t reg;
	struct {
	unsigned int AGPIO_0:1;
	unsigned int AGPIO_1:1;
	unsigned int AGPIO_2:1;
	unsigned int AGPIO_3:1;
	unsigned int reserved0:28;
	} bit;
};

// 0xF0 Version Code Register
#define TOP_VERSION_REG_OFS                  0xF0
union TOP_VERSION_REG {
	uint32_t reg;
	struct {
	unsigned int reserved0:8;
	unsigned int ROM_CODE_VERSION:4;         // ROM Code Version
	unsigned int ECO_VERSION:4;              // ECO Version
	unsigned int CHIP_ID:16;                 // Chip ID
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
