/*
	Pinmux module internal header file

	@file		top_reg.h
	@ingroup
	@note		Nothing

	Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#ifndef _PINMUX_INT_H
#define _PINMUX_INT_H

#include <asm/nvt-common/rcw_macro.h>

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

/*
	PR_GPIO_ID_ENUM
*/
typedef enum
{
	GPIO_ID_EMUM_FUNC_PR,                  //< pinmux is mapping to PR-function
	GPIO_ID_EMUM_GPIO_PR,                  //< pinmux is mapping to gpio
	GPIO_ID_EMUM_FUNC_MAIN,                //< pinmux is mapping to MAIN-function

	ENUM_DUMMY4WORD(PR_GPIO_ID_ENUM)
} PR_GPIO_ID_ENUM;

// 0x00 TOP Control Register 0 (BOOT)
#define TOP_REG0_OFS                        0x00
union TOP_REG0 {
	uint32_t reg;
	struct {
	unsigned int BOOT_SRC:4;                // Boot Source Selection
	unsigned int EJTAG_SEL:1;               // EJTAG select
	unsigned int DEBUG_MODE_EN:1;           // debug mode enable
	unsigned int MPLL_CLKSELL:1;            // PLL clock output mux
	unsigned int DEBUG_MODE_SEL:2;          // MIPS debug mode select
	unsigned int APLL_FREQ_SEL:1;           // APLL frequenct select
	unsigned int reserved0:22;
	} bit;
};

// 0x04 TOP Control Register 1 (Storage)
#define TOP_REG1_OFS                        0x04
union TOP_REG1 {
	uint32_t reg;
	struct {
	unsigned int EXTROM_EN:1;               // EXTROM enable control
	unsigned int EJTAG_EN:1;                // EJTAG enable control
	unsigned int FSPI_EN:2;                 // FSPI enable control
	unsigned int FLASH_TRIGA:2;             // FLASH triga enable
	unsigned int FSPI_CS1_EN:1;
	unsigned int EJTAG_CS:1;                // EJTAG channel select
	unsigned int reserved0:8;
	unsigned int SDIO_EN:2;                 // SDIO enable control
	unsigned int SDIO2_EN:2;                // SDIO2 enable control
	unsigned int SDIO3_EN:2;                // SDIO3 enable control
	unsigned int reserved1:1;
	unsigned int SDIO3_BUS_WIDTH:1;         // Select SDIO3 bus width
	unsigned int SDIO3_DS_EN:1;             // Select SDIO3 data strobe
	unsigned int reserved2:7;
	} bit;
};

// 0x08 TOP Control Register 2 (LCD)
#define TOP_REG2_OFS                        0x08
union TOP_REG2 {
	uint32_t reg;
	struct {
	unsigned int LCD_TYPE:4;                // Pinmux of LCD interface
	unsigned int reserved0:2;
	unsigned int PLCD_DE:1;                 // Pinmux of DE (for parallel LCD)
	unsigned int CCIR_DATA_WIDTH:1;         // CCIR data width
	unsigned int CCIR_HVLD_VVLD:1;          // CCIR VVLD, HVLD select
	unsigned int CCIR_FIELD:1;              // CCIR FIELD select
	unsigned int reserved1:22;
	} bit;
};

// 0x0C TOP Control Register 3 (SENSOR)
#define TOP_REG3_OFS                        0x0C
union TOP_REG3 {
	uint32_t reg;
	struct {
	unsigned int SENSOR:4;                  // Pinmux of SN interface
	unsigned int SENSOR2:4;                 // Pinmux of SN2 interface
	unsigned int SN_CCIR_VSHS:2;           // Pinmux of SN2 CCIR VS/HS/FIELD
	unsigned int SN_MCLK:2;                 // Pinmux of SN MCLK
	unsigned int SN_VSHS:2;                 // Pinmux of SN VSHS
	unsigned int SN2_MCLK:2;                 // Pinmux of SN2 MCLK
	unsigned int SN2_VSHS:2;                 // Pinmux of SN2 VSHS
	unsigned int SN3_MCLK:1;                 // Pinmux of SN3 MCLK
	unsigned int SP_CLK:3;
	unsigned int SP_CLK2:3;
	unsigned int SN2_MCLK_SRC:1;            // Pinmux of SN2_MCLK_SRC (from CG)
	unsigned int SN2_XVSHS_SRC:1;           // Pinmux of SN2_XVSHS_SRC (TG)
	unsigned int SN_XVSXHS:2;               // Pinmux of SN XVS/XHS
	unsigned int SN2_XVSXHS:2;               // Pinmux of SN2 XVS/XHS
	unsigned int reserved0:1;
	} bit;
};

// 0x10 TOP Control Register 4 (I2C/UART)
#define TOP_REG4_OFS                        0x10
union TOP_REG4 {
	uint32_t reg;
	struct {
	unsigned int I2C:3;
	unsigned int I2C2:3;
	unsigned int reserved0:10;
	unsigned int UART:3;
	unsigned int UART2:3;
	unsigned int UART3:3;
	unsigned int UART2_RTSCTS:2;
	unsigned int UART3_RTSCTS:2;
	unsigned int reserved1:3;
	} bit;
};

// 0x14 TOP Control Register 5 (PWM[1/2])
#define TOP_REG5_OFS                        0x14
union TOP_REG5 {
	uint32_t reg;
	struct {
	unsigned int PWM0:3;
	unsigned int PWM1:3;
	unsigned int PWM2:3;
	unsigned int PWM3:3;
	unsigned int PWM4:3;
	unsigned int PWM5:3;
	unsigned int PWM6:3;
	unsigned int PWM7:3;
	unsigned int PWM8:3;
	unsigned int PWM9:3;
	unsigned int reserved0:2;
	} bit;
};

// 0x18 TOP Control Register 6 (PWM[2/2])
#define TOP_REG6_OFS                        0x18
union TOP_REG6 {
	uint32_t reg;
	struct {
	unsigned int PWM10:3;
	unsigned int PWM11:3;
	unsigned int reserved0:26;
	} bit;
};

//0x1C TOP Control Register 7 (SPI/SIF)
#define TOP_REG7_OFS                        0x1C
union TOP_REG7 {
	uint32_t reg;
	struct {
	unsigned int SPI:3;
	unsigned int SPI2:3;
	unsigned int SPI3:3;
	unsigned int SPI3_RDY:3;
	unsigned int SPI_BUS_WIDTH:1;
	unsigned int SPI2_BUS_WIDTH:1;
	unsigned int SPI3_BUS_WIDTH:1;
	unsigned int reserved0:1;
	unsigned int SIF0:3;
	unsigned int SIF1:3;
	unsigned int SIF2:3;
	unsigned int SIF3:3;
	unsigned int SDP:2;
	unsigned int reserved1:2;
	} bit;
};

// 0x20 TOP Control Register 8 (System)
#define TOP_REG8_OFS                        0x20
union TOP_REG8 {
	uint32_t reg;
	struct {
	unsigned int RAM_SRC_SELECT:1;          // RAM SRC Select
	unsigned int reserved0:14;
	unsigned int FAST_BOOT_CH_SEL:2;        // Fast boot channel Select
	unsigned int PD_ISO_EN:1;
	unsigned int PWR_OFF_PD_AI:1;
	unsigned int PWR_OFF_STS_PD_AI:1;
	unsigned int FAST_BOOT_SCE_DIRMODE_EN:1;
	unsigned int FAST_BOOT_HASH_DIRMODE_EN:1;
	unsigned int reserved1:10;
	} bit;
};

// 0x24 TOP Control Register 9 (AUDIO/ETH)
#define TOP_REG9_OFS                       0x24
union TOP_REG9 {
	uint32_t reg;
	struct {
	unsigned int I2S:2;
	unsigned int I2S_MCLK:2;
	unsigned int DMIC_DATA0:1;
	unsigned int DMIC_DATA1:1;
	unsigned int DMIC:3;
	unsigned int EXT_EAC_MCLK:1;
	unsigned int reserved0:6;
	unsigned int ETH_MDIO_MUX:1;
	unsigned int ETH:1;
	unsigned int ETH_LED:2;
	unsigned int ETH_EXT_PHY_CLK:1;
	unsigned int reserved1:11;
	} bit;
};

// 0x28 TOP Control Register 10 (MISC)
#define TOP_REG10_OFS                       0x28
union TOP_REG10 {
	uint32_t reg;
	struct {
	unsigned int REMOTE:2;
	unsigned int REMOTE_EXT:2;
	unsigned int PICNT:3;
	unsigned int PICNT2:3;
	unsigned int PICNT3:3;
	unsigned int RTC_CLK:1;
	unsigned int ME_SHUT_IN:2;
	unsigned int ME_SHUT_OUT:2;
	unsigned int reserved0:14;
	} bit;
};

// 0x2C TOP Control Register 11 (DMA_CH_PROT_IN_PD3FLOW)
#define TOP_REG11_OFS                       0x2C
union TOP_REG11 {
	uint32_t reg;
	struct {
	unsigned int CPU_AXI_BRG:1;
	unsigned int IP_AXI_BRG:1;
	unsigned int IFE_DMA_CH:1;
	unsigned int reserved0:29;
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
	unsigned int CGPIO_18:1;
	unsigned int CGPIO_19:1;
	unsigned int CGPIO_20:1;
	unsigned int CGPIO_21:1;
	unsigned int CGPIO_22:1;
	unsigned int CGPIO_23:1;
	unsigned int CGPIO_24:1;
	unsigned int reserved0:7;
	} bit;
};

// 0xA4 Reserved

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
	unsigned int reserved0:7;
	} bit;
};

// 0xAC PGPIO Control Register 1 (P_GPIO[2/2])
#define TOP_REGPGPIO1_OFS                   0xAC
union TOP_REGPGPIO1 {
	uint32_t reg;
	struct {
	unsigned int reserved0:7;
	unsigned int PGPIO_39:1;
	unsigned int reserved1:24;
	} bit;
};

// 0xB0~0xCC Reserved

// 0xD0 DGPIO Control Register 0 (D_GPIO)
#define TOP_REGDGPIO0_OFS                   0xD0
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
	unsigned int reserved0:24;
	} bit;
};

// 0xD4 Reserved

// 0xD8 HSIGPIO Control Register 0 (HSI_GPIO)
#define TOP_REGHSIGPIO0_OFS                  0xD8
union TOP_REGHSIGPIO0 {
        uint32_t reg;
        struct {
        unsigned int HSIGPIO_0:1;
        unsigned int HSIGPIO_1:1;
        unsigned int HSIGPIO_2:1;
        unsigned int HSIGPIO_3:1;
        unsigned int HSIGPIO_4:1;
        unsigned int HSIGPIO_5:1;
        unsigned int HSIGPIO_6:1;
        unsigned int HSIGPIO_7:1;
        unsigned int HSIGPIO_8:1;
        unsigned int HSIGPIO_9:1;
        unsigned int HSIGPIO_10:1;
        unsigned int HSIGPIO_11:1;
        unsigned int reserved0:20;
        } bit;
};

// 0xDC~0xEC Reserved

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

/*************** PR PART ***************/
// 0x0 PR_TOP Control Register 0 (PR_I2C)
#define PR_TOP_REG0_OFS                        0x0
union PR_TOP_REG0 {
	uint32_t reg;
	struct {
	unsigned int PR_I2C:4;
	unsigned int reserved0:28;
	} bit;
};

// 0x4 PR_TOP Control Register 1 (PR_UART)
#define PR_TOP_REG1_OFS                        0x4
union PR_TOP_REG1 {
	uint32_t reg;
	struct {
	unsigned int PR_UART:4;
	unsigned int PR_UART2:4;
	unsigned int reserved0:24;
	} bit;
};

// 0x8 PR_TOP Control Register 3 (PR_SENSOR)
#define PR_TOP_REG3_OFS                        0x8
union PR_TOP_REG3 {
	uint32_t reg;
	struct {
	unsigned int PR_SENSOR:4;
	unsigned int PR_SN_MCLK:4;
	unsigned int reserved0:8;
	unsigned int PR_SRAM:2;
	unsigned int PR_SRAM_SIO:2;
	unsigned int reserved1:12;
	} bit;
};

// 0xC PR_TOP Control Register 4 (PR_SYSTEM)
#define PR_TOP_REG4_OFS                        0xC
union PR_TOP_REG4 {
	uint32_t reg;
	struct {
	unsigned int ISO_ZONE1_EN:1;
	unsigned int ISO_ZONE2_EN:1;
	unsigned int ISO_ZONE3_EN:1;
	unsigned int ISO_ZONE4_EN:1;
	unsigned int reserved0:12;
	unsigned int PWR_OFF_PD_USB:1;
	unsigned int PWR_OFF_STS_PD_USB:1;
	unsigned int reserved1:13;
	unsigned int PWR_OFF_TST_MODE:1;
	} bit;
};

// 0x10 PR_TOP Control Register 5 (PR_SPI_WG)
#define PR_TOP_REG5_OFS                        0x10
union PR_TOP_REG5 {
	uint32_t reg;
	struct {
	unsigned int PR_SPI_WG:4;
	unsigned int reserved0:12;
	unsigned int PR_DMIC_DATA0:1;
	unsigned int PR_DMIC:1;
	unsigned int reserved1:14;
	} bit;
};

// 0x14 PR_TOP Control Register 6 (PR_PWM)
#define PR_TOP_REG6_OFS                        0x14
union PR_TOP_REG6 {
	uint32_t reg;
	struct {
	unsigned int PR_PWM0:4;
	unsigned int PR_PWM1:4;
	unsigned int reserved0:24;
	} bit;
};

// 0x28 PR_SGPIO Control Register 0 (PR_S_GPIO[1/2])
#define PR_TOP_REGSGPIO0_OFS                  0x28
union PR_TOP_REGSGPIO0 {
	uint32_t reg;
	struct {
		unsigned int SGPIO_0:2;
		unsigned int SGPIO_1:2;
		unsigned int SGPIO_2:2;
		unsigned int SGPIO_3:2;
		unsigned int SGPIO_4:2;
		unsigned int SGPIO_5:2;
		unsigned int SGPIO_6:2;
		unsigned int SGPIO_7:2;
		unsigned int SGPIO_8:2;
		unsigned int SGPIO_9:2;
		unsigned int SGPIO_10:2;
		unsigned int SGPIO_11:2;
		unsigned int SGPIO_12:2;
		unsigned int SGPIO_13:2;
		unsigned int SGPIO_14:2;
		unsigned int SGPIO_15:2;
		unsigned int SGPIO_16:2;
        } bit;
};

// 0x2C PR_SGPIO Control Register 1 (PR_S_GPIO[2/2])
#define PR_TOP_REGSGPIO1_OFS                  0x2C
union PR_TOP_REGSGPIO1 {
	uint32_t reg;
	struct {
		unsigned int SGPIO_16:2;
		unsigned int reserved0:30;
        } bit;
};

// 0x30 PR_PGPIO Control Register 0 (PR_P_GPIO)
#define PR_TOP_REGPGPIO0_OFS                  0x30
union PR_TOP_REGPGPIO0 {
	uint32_t reg;
	struct {
		unsigned int PGPIO_25:2;
		unsigned int PGPIO_26:2;
		unsigned int PGPIO_27:2;
		unsigned int PGPIO_28:2;
		unsigned int PGPIO_29:2;
		unsigned int PGPIO_30:2;
		unsigned int PGPIO_31:2;
		unsigned int PGPIO_32:2;
		unsigned int PGPIO_33:2;
		unsigned int PGPIO_34:2;
		unsigned int PGPIO_35:2;
		unsigned int PGPIO_36:2;
		unsigned int PGPIO_37:2;
		unsigned int PGPIO_38:2;
		unsigned int reserved0:4;
        } bit;
};

// 0x38 PR_AGPIO Control Register 0 (PR_A_GPIO)
#define PR_TOP_REGAGPIO0_OFS                  0x38
union PR_TOP_REGAGPIO0 {
	uint32_t reg;
	struct {
		unsigned int AGPIO_0:2;
		unsigned int AGPIO_1:2;
		unsigned int AGPIO_2:2;
		unsigned int AGPIO_3:2;
		unsigned int reserved0:24;
        } bit;
};

#endif
