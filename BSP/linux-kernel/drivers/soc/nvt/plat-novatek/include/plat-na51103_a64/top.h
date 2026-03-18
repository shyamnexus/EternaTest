/*
	TOP controller header

	Sets the pinmux of each module.

	@file       top.h
	@ingroup    mIDrvSys_TOP
	@note       Refer NA51103 data sheet for PIN/PAD naming

	Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#ifndef __SOC_NVT_PLAT_NA51103_TOP_H
#define __SOC_NVT_PLAT_NA51103_TOP_H

#include <linux/soc/nvt/nvt_type.h>

/**
    @addtogroup mIDrvSys_TOP
*/
//@{


/*
	Follow the naming rule for pinctrl tool parsing:
	1. The name of enum variable should be "PIN_xxx_CFG", such as PIN_UART_CFG
	2. The name of enum should be prefixed with "PIN_xxx_CFG", such as PIN_UART_CFG_CH0_1ST_PINMUX
	3. The value of enum should be less than 0x80000000, such as PIN_UART_CFG_CH0_1ST_PINMUX = 0x01
*/

typedef enum {
	PINMUX_DEBUGPORT_CKG   = 0x00,     ///< CKGen

	PINMUX_DEBUGPORT_GPIO  = 0x01,     ///< GPIO
	PINMUX_DEBUGPORT_CPU   = 0x02,     ///< CPU
	PINMUX_DEBUGPORT_WDT   = 0x03,     ///< WDT
	PINMUX_DEBUGPORT_TIMER = 0x04,     ///< TIMER
	PINMUX_DEBUGPORT_PWM   = 0x05,     ///< PWM
	PINMUX_DEBUGPORT_REMOTE= 0x06,     ///< REMOTE
	PINMUX_DEBUGPORT_TRNG  = 0x07,     ///< TRNG
	PINMUX_DEBUGPORT_TSEN  = 0x08,     ///< TSEN
	PINMUX_DEBUGPORT_SMC   = 0x09,     ///< SMC,NAND
	PINMUX_DEBUGPORT_SDIO  = 0x0A,     ///< SDIO
	PINMUX_DEBUGPORT_DAI   = 0x0B,     ///< DAI
	PINMUX_DEBUGPORT_DAI2  = 0x0C,     ///< DAI2
	PINMUX_DEBUGPORT_DAI3  = 0x0D,     ///< DAI3
	PINMUX_DEBUGPORT_DAI4  = 0x0E,     ///< DAI4
	PINMUX_DEBUGPORT_DAI5  = 0x0F,     ///< DAI5
	PINMUX_DEBUGPORT_UART2 = 0x10,     ///< UART2
	PINMUX_DEBUGPORT_UART3 = 0x11,     ///< UART3
	PINMUX_DEBUGPORT_UART4 = 0x12,     ///< UART4
	PINMUX_DEBUGPORT_I2C   = 0x13,     ///< I2C
	PINMUX_DEBUGPORT_I2C2  = 0x14,     ///< I2C2
	PINMUX_DEBUGPORT_I2C3  = 0x15,     ///< I2C3
	PINMUX_DEBUGPORT_I2C4  = 0x16,     ///< I2C4
	PINMUX_DEBUGPORT_JPEG  = 0x17,     ///< JPEG
	PINMUX_DEBUGPORT_OSG   = 0x18,     ///< OSG
	PINMUX_DEBUGPORT_SSCA  = 0x19,     ///< SSCA
	PINMUX_DEBUGPORT_EFUSE = 0x1A,     ///< EFUSE
	PINMUX_DEBUGPORT_SCE   = 0x1B,     ///< SCE
	PINMUX_DEBUGPORT_HASH  = 0x1C,     ///< HASH
	PINMUX_DEBUGPORT_RSA   = 0x1D,     ///< RSA
	PINMUX_DEBUGPORT_USB   = 0x1E,     ///< USB
	PINMUX_DEBUGPORT_USB2  = 0x1F,     ///< USB2
	PINMUX_DEBUGPORT_ETH   = 0x20,     ///< ETH
	PINMUX_DEBUGPORT_ETH2  = 0x21,     ///< ETH2
	PINMUX_DEBUGPORT_SATA  = 0x22,     ///< SATA
	PINMUX_DEBUGPORT_SATA2 = 0x23,     ///< SATA2
	PINMUX_DEBUGPORT_HWCPY = 0x24,     ///< HYCPY
	PINMUX_DEBUGPORT_CNN   = 0x25,     ///< CNN
	PINMUX_DEBUGPORT_NUE   = 0x26,     ///< NUE
	PINMUX_DEBUGPORT_NUE2  = 0x27,     ///< NUE2
	PINMUX_DEBUGPORT_VPE   = 0x28,     ///< VPE
	PINMUX_DEBUGPORT_VENC  = 0x29,     ///< VENC
	PINMUX_DEBUGPORT_VDEC  = 0x2A,     ///< VDEC
	PINMUX_DEBUGPORT_DEI   = 0x2B,     ///< DEI
	PINMUX_DEBUGPORT_VCAP  = 0x2C,     ///< VCAP
	PINMUX_DEBUGPORT_LCD310= 0x2D,     ///< LCD310
	PINMUX_DEBUGPORT_LCD210= 0x2E,     ///< LCD210
	PINMUX_DEBUGPORT_HDMI  = 0x2F,     ///< HDMI
	PINMUX_DEBUGPORT_TV    = 0x30,     ///< TV
	PINMUX_DEBUGPORT_HVLD  = 0x31,     ///< HEAVYLOAD
	PINMUX_DEBUGPORT_HVLD2 = 0x32,     ///< HEAVYLOAD2
	PINMUX_DEBUGPORT_HDLD3 = 0x33,     ///< HEAVYLOAD3
	PINMUX_DEBUGPORT_DDRH_A= 0x34,     ///< DDR A
	PINMUX_DEBUGPORT_DDRH_B= 0x35,     ///< DDR B
	PINMUX_DEBUGPORT_HRTMR = 0x36,     ///< HR TIMER
	PINMUX_DEBUGPORT_HRTMR2= 0x37,     ///< HR TIMER2
	PINMUX_DEBUGPORT_SPI   = 0x38,     ///< SPI

	PINMUX_DEBUGPORT_GROUP_NONE =   0x0000,     ///< No debug port is output
	PINMUX_DEBUGPORT_GROUP1 =       0x0100,     ///< Output debug port to S_GPIO[17..0], P_GPIO[23]
	PINMUX_DEBUGPORT_GROUP2 =       0x0200,     ///< Output debug port to D_GPIO[9..0], P_GPIO[18..16], P_GPIO[7..4], P_GPIO[21..20]

	ENUM_DUMMY4WORD(PINMUX_DEBUGPORT)
} PINMUX_DEBUGPORT;

/**
    Function group

    @note For pinmux_init()
*/
typedef enum {
	PIN_FUNC_UART,      ///< UART. Configuration refers to PIN_UART_CFG.
	PIN_FUNC_I2C,       ///< I2C. Configuration refers to PIN_I2C_CFG.
	PIN_FUNC_SDIO,      ///< SDIO. Configuration refers to PIN_SDIO_CFG.
	PIN_FUNC_SPI,       ///< SPI. Configuration refers to PIN_SPI_CFG.
	PIN_FUNC_EXTCLK,    ///< EXTCLK. Configuration refers to PIN_EXTCLK_CFG
	PIN_FUNC_SSP,       ///< AUDIO/SSP. Configuration refers to PIN_SSP_CFG.
	PIN_FUNC_LCD,       ///< LCD interface. Configuration refers to PIN_LCD_CFG.
	PIN_FUNC_REMOTE,    ///< REMOTE. Configuration refers to PIN_REMOTE_CFG.
	PIN_FUNC_VCAP,      ///< VCAP. Configuration refers to PIN_VCAP_CFG
	PIN_FUNC_ETH,       ///< ETH. Configuration refers to PIN_ETH_CFG
	PIN_FUNC_MISC,      ///< MISC. Configuration refers to PIN_MISC_CFG
	PIN_FUNC_PWM,       ///< PWM. Configuration refers to PIN_PWM_CFG.
	PIN_FUNC_VCAPINT,   ///< VCAP INTERNAL MUX. Configuration refers to PIN_VCAPINT_CFG
	PIN_FUNC_MAX,

	ENUM_DUMMY4WORD(PIN_FUNC)
} PIN_FUNC;

typedef enum {
	PIN_UART_CFG_NONE,

	PIN_UART_CFG_CH_1ST_PINMUX  = 0x01,     ///< Enable UART.  (P_GPIO[1..0])
	PIN_UART_CFG_CH_2ND_PINMUX  = 0x02,     ///< Not support
	PIN_UART_CFG_CH_3RD_PINMUX  = 0x04,     ///< Not support
	PIN_UART_CFG_CH_CTSRTS      = 0x08,     ///< Enable UART_CTS/RTS (P_GPIO[3..2])

	PIN_UART_CFG_CH2_1ST_PINMUX = 0x10,     ///< Enable UART2. (P_GPIO[5..4])
	PIN_UART_CFG_CH2_2ND_PINMUX = 0x20,     ///< Enable UART2_2. (P_GPIO[15..14])
	PIN_UART_CFG_CH2_CTSRTS     = 0x40,     ///< Enable UART2_CTS/RTS. (E_GPIO[1][6])
	PIN_UART_CFG_CH2_3RD_PINMUX = 0x80,     ///< Enable UART2_3 (D_GPIO[1..0])

	PIN_UART_CFG_CH3_1ST_PINMUX = 0x100,    ///< Enable UART3. (P_GPIO[7..6])
	PIN_UART_CFG_CH3_RTS        = 0x200,    ///< Enalbe UART3_RTS (D_GPIO[5])

	PIN_UART_CFG_CH4_1ST_PINMUX = 0x1000,   ///< Enable UART4. (J_GPIO[4..3])
	PIN_UART_CFG_CH4_2ND_PINMUX = 0x2000,   ///< Enable UART4_2. (P_GPIO[9..8])
	PIN_UART_CFG_CH4_3RD_PINMUX = 0x4000,   ///< Enable UART4_3. (P_GPIO[17..16])
	PIN_UART_CFG_CH4_4TH_PINMUX = 0x8000,   ///< Enable UART4_4. (P_GPIO[21..20])

	PIN_UART_CFG_CH5_1ST_PINMUX = 0x10000,  ///< Not Support
	PIN_UART_CFG_CH5_2ND_PINMUX = 0x20000,  ///< Not Support

	PIN_UART_CFG_CH4_5TH_PINMUX = 0x100000, ///< Enable UART4_5. (D_GPIO[3..2])
	PIN_UART_CFG_CH4_6TH_PINMUX = 0x200000, ///< Enable UART4_6. (D_GPIO[8..7])

	ENUM_DUMMY4WORD(PIN_UART_CFG)
} PIN_UART_CFG;

#define PIN_UART_CFG_CH0_1ST_PINMUX PIN_UART_CFG_CH_1ST_PINMUX
#define PIN_UART_CFG_CH0_CTSRTS PIN_UART_CFG_CH_CTSRTS

typedef enum {
	PIN_I2C_CFG_NONE,

	PIN_I2C_CFG_CH_1ST_PINMUX	= 0x01, 	///< Enable I2C. (P_GPIO[9..8])
	PIN_I2C_CFG_CH_2ND_PINMUX	= 0x02, 	///< Enable I2C_2. (D_GPIO[1..0])

	PIN_I2C_CFG_CH2_1ST_PINMUX	= 0x10, 	///< Enable I2C2. (P_GPIO[15..14])
	PIN_I2C_CFG_CH2_2ND_PINMUX	= 0x20, 	///< Enable I2C2_2. (D_GPIO[3..2])
	PIN_I2C_CFG_CH2_3RD_PINMUX	= 0x40, 	///< Enable I2C2_3. (D_GPIO[6..5])
	PIN_I2C_CFG_CH2_4TH_PINMUX	= 0x80, 	///< Enable I2C2_4. (J_GPIO[1..0])

	PIN_I2C_CFG_CH2_5TH_PINMUX	= 0x100, 	///< Enable I2C2_5. (P_GPIO[5..4])

	PIN_I2C_CFG_CH3_1ST_PINMUX	= 0x1000, 	///< Enable I2C3. (P_GPIO[21..20])
	PIN_I2C_CFG_CH3_2ND_PINMUX	= 0x2000, 	///< Enable I2C3_2. (P_GPIO[17..16])
	PIN_I2C_CFG_CH3_3RD_PINMUX  = 0x4000,   ///< Not Support
	PIN_I2C_CFG_CH3_4TH_PINMUX  = 0x8000,   ///< Not Support

	PIN_I2C_CFG_HDMI_1ST_PINMUX	= 0x10000,	///< Enable HDMI_I2C. (P_GPIO[30..29])

	ENUM_DUMMY4WORD(PIN_I2C_CFG)
} PIN_I2C_CFG;

#define PIN_I2C_CFG_CH0_1ST_PINMUX PIN_I2C_CFG_CH_1ST_PINMUX
#define PIN_I2C_CFG_CH1_1ST_PINMUX PIN_I2C_CFG_CH2_1ST_PINMUX


typedef enum {
	PIN_SDIO_CFG_NONE,

	PIN_SDIO_CFG_1ST_PINMUX = 0x01,	 ///< SDIO (P_GPIO[15..14], P_GPIO[19..17], P_GPIO[23])
	PIN_SDIO_CFG_2ND_PINMUX = 0x02,	 ///< SDIO_2 (D_GPIO[9..6]), P_GPIO[21..22]

	PIN_SDIO2_CFG_1ST_PINMUX = 0x10, ///< Not Support
	PIN_SDIO2_CFG_BUS_WIDTH  = 0x20, ///< Not Support
	PIN_SDIO2_CFG_DS         = 0x40, ///< Not Support

    PIN_SDIO_CFG_SDIO3_1         = 0x100,    ///< SDIO3.
    PIN_SDIO_CFG_SDIO3_BUS_WIDTH = 0x200,    ///< SDIO3 bus width 8 bits
    PIN_SDIO_CFG_SDIO3_DS        = 0x400,    ///< SDIO3 data strobe
	
	ENUM_DUMMY4WORD(PIN_SDIO_CFG)
} PIN_SDIO_CFG;

// host_id refer to SDIO_HOST_ID
#define PIN_SDIO_CFG_MASK(host_id)   (0xF << host_id)


typedef enum {
	PIN_SPI_CFG_NONE,

	PIN_SPI_CFG_CH_1ST_PINMUX  = 0x01,   ///< Enable SPI. (C_GPIO[3..0])

	PIN_SPI_CFG_CH_BUS_WIDTH   = 0x10,   ///< SPI bus width = 4 (C_GPIO[5..4])
	PIN_SPI_CFG_CH_CS1         = 0x20,   ///< Enable SPI CS1. (C_GPIO[6])

	PIN_SPI_CFG_CH2_1ST_PINMUX = 0x100,  ///< Enable SPI2. (J_GPIO[1..0], P_GPIO[2])
	PIN_SPI_CFG_CH2_2ND_PINMUX = 0x200,  ///< Enalbe SPI2_2. (P_GPIO[21..20], D_GPIO[6])

	PIN_SPI_CFG_CH2_BUS_WIDTH  = 0x1000, ///< SPI2 bus with = 2 (P_GPIO[3] or D_GPIO[7])

	ENUM_DUMMY4WORD(PIN_SPI_CFG)
} PIN_SPI_CFG;


typedef enum {
	PIN_EXTCLK_CFG_NONE,

	PIN_EXTCLK_CFG_CH_1ST_PINMUX  = 0x01,   ///< Enable EXT_CLK. (P_GPIO[22])

	PIN_EXTCLK_CFG_CH2_1ST_PINMUX = 0x02,   ///< Enable EXT2_CLK. (P_GPIO[23])

	PIN_EXTCLK_CFG_CH3_1ST_PINMUX = 0x04,   ///< Enable EXT3_CLK.(EXT_2_CLK) (S_GPIO[9])

	PIN_EXTCLK_CFG_CH4_1ST_PINMUX = 0x08,   ///< Enable EXT4_CLK.(EXT2_2_CLK) (S_GPIO[18])

	ENUM_DUMMY4WORD(PIN_EXTCLK_CFG)
} PIN_EXTCLK_CFG;
#define PIN_EXTCLK_CFG_CH0_1ST_PINMUX PIN_EXTCLK_CFG_CH_1ST_PINMUX
#define PIN_EXTCLK_CFG_CH1_1ST_PINMUX PIN_EXTCLK_CFG_CH2_1ST_PINMUX

typedef enum {
	PIN_SSP_CFG_NONE,

	PIN_SSP_CFG_CH_1ST_PINMUX		= 0x01,   	///< Enable I2S. (P_GPIO[11..10])
	PIN_SSP_CFG_CH_1ST_MCLK         = 0x02,     ///< Enable I2S_MCLK. (P_GPIO[22])
	PIN_SSP_CFG_CH_1ST_TX           = 0x04,     ///< Enable I2S_TX. (P_GPIO[13])
	PIN_SSP_CFG_CH_1ST_RX           = 0x08,     ///< Enable I2S_RX. (P_GPIO[12])

	PIN_SSP_CFG_CH2_1ST_PINMUX 		= 0x10,  	///< Enable I2S2. (P_GPIO[17..16])
	PIN_SSP_CFG_CH2_1ST_MCLK        = 0x20,     ///< Enable I2S2_MCLK (P_GPIO[23])
	PIN_SSP_CFG_CH2_1ST_TX          = 0x40,     ///< Enable I2S2_TX (P_GPIO[19])
	PIN_SSP_CFG_CH2_1ST_RX          = 0x80,     ///< Enable I2S2_RX (P_GPIO[18])

	PIN_SSP_CFG_CH3_1ST_PINMUX		= 0x100, 	///< Enable I2S3. (D_GPIO[7..6])
	PIN_SSP_CFG_CH3_2ND_PINMUX		= 0x200, 	///< Enable I2S3_2. (P_GPIO[15..14])
	PIN_SSP_CFG_CH3_3RD_PINMUX		= 0x400, 	///< Enable I2S3_3. (P_GPIO[5], P_GPIO[14])

	PIN_SSP_CFG_CH3_1ST_MCLK		= 0x1000, 	///< Enable I2S3_MCLK. (D_GPIO[5])
	PIN_SSP_CFG_CH3_2ND_MCLK		= 0x2000, 	///< Not Support
	PIN_SSP_CFG_CH3_3RD_MCLK		= 0x4000, 	///< Enable I2S3_3_MCLK. (P_GPIO[4])

	PIN_SSP_CFG_CH3_1ST_TX   		= 0x10000, 	///< Enable I2S3_TX. (D_GPIO[9])
	PIN_SSP_CFG_CH3_2ND_TX	    	= 0x20000, 	///< Enable I2S3_2_TX. (P_GPIO[19])
	PIN_SSP_CFG_CH3_3RD_TX		    = 0x40000, 	///< Not Support

	PIN_SSP_CFG_CH3_1ST_RX   		= 0x100000, ///< Enable I2S3_RX. (D_GPIO[8])
	PIN_SSP_CFG_CH3_2ND_RX	    	= 0x200000,	///< Not Support
	PIN_SSP_CFG_CH3_3RD_RX		    = 0x400000,	///< Enable I2S3_3_RX. (P_GPIO[15])

	PIN_SSP_CFG_CH4_1ST_PINMUX 		= 0x1000000,///< Enable I2S4. (J_GPIO[2..1])
	PIN_SSP_CFG_CH4_1ST_MCLK        = 0x2000000,///< Enable I2S4_MCLK (J_GPIO[0])
	PIN_SSP_CFG_CH4_1ST_TX          = 0x4000000,///< Enable I2S4_TX (J_GPIO[4])
	PIN_SSP_CFG_CH4_1ST_RX          = 0x8000000,///< Enable I2S4_RX (J_GPIO[3])

	PIN_SSP_CFG_CH4_2ND_PINMUX 		= 0x10000000,///< Enable I2S4_2. (P_GPIO[9..8])
	PIN_SSP_CFG_CH4_2ND_TX          = 0x20000000,///< Enable I2S4_2_TX (P_GPIO[13])

	ENUM_DUMMY4WORD(PIN_SSP_CFG)
} PIN_SSP_CFG;

typedef enum {
	PIN_LCD_CFG_NONE,

	PIN_LCD_CFG_LCD310_RGB888_1ST_PINMUX = 0x01,   ///< Enable LCD310 RGB888. (S_GPIO[17..0], P_GPIO[15..14], P_GPIO[12..10], P_GPIO[18..16])
	PIN_LCD_CFG_LCD310_BT1120_1ST_PINMUX = 0x02,   ///< Enable LCD310 BT1120. (S_GPIO[8..0], S_GPIO[17..10])
	PIN_LCD_CFG_LCD310_BT1120_2ND_PINMUX = 0x04,   ///< Enable LCD310_2 BT1120. (E_GPIO[15..0], D_GPIO[5])

	PIN_LCD_CFG_LCD210_BT1120_1ST_PINMUX = 0x10,   ///< Enable LCD210 BT1120. (S_GPIO[8..0], S_GPIO[17..10])
	PIN_LCD_CFG_LCD210_BT1120_2ND_PINMUX = 0x20,   ///< Enable LCD210_2 BT1120. (E_GPIO[15..0], D_GPIO[5])

	PIN_LCD_CFG_LCD310L_RGB888_1ST_PINMUX= 0x100,  ///< Not Support
	PIN_LCD_CFG_LCD310L_BT1120_1ST_PINMUX= 0x200,  ///< Not Support

	PIN_LCD_CFG_LCD310_DE_PINMUX         = 0x1000, ///< Enable LCD310_DE. (P_GPIO[19])
	
	ENUM_DUMMY4WORD(PIN_LCD_CFG)
} PIN_LCD_CFG;


typedef enum {
	PIN_REMOTE_CFG_NONE,

	PIN_REMOTE_CFG_1ST_PINMUX  = 0x001,   ///< Enable Remote/IrDA. (J_GPIO[1])
	PIN_REMOTE_CFG_2ND_PINMUX  = 0x002,   ///< Enable Remote/IrDA. (J_GPIO[2])
	PIN_REMOTE_CFG_3RD_PINMUX  = 0x004,   ///< Enable Remote/IrDA. (J_GPIO[3])
	PIN_REMOTE_CFG_4TH_PINMUX  = 0x008,   ///< Enable Remote/IrDA. (J_GPIO[4])
	PIN_REMOTE_CFG_5TH_PINMUX  = 0x010,   ///< Enable Remote/IrDA. (P_GPIO[4])
	PIN_REMOTE_CFG_6TH_PINMUX  = 0x020,   ///< Enable Remote/IrDA. (P_GPIO[5])
	PIN_REMOTE_CFG_7TH_PINMUX  = 0x040,   ///< Enable Remote/IrDA. (D_GPIO[0])
	PIN_REMOTE_CFG_8TH_PINMUX  = 0x080,   ///< Enable Remote/IrDA. (D_GPIO[1])
	PIN_REMOTE_CFG_9TH_PINMUX  = 0x100,   ///< Enable Remote/IrDA. (D_GPIO[2])
	PIN_REMOTE_CFG_10TH_PINMUX = 0x200,   ///< Enable Remote/IrDA. (D_GPIO[3])
	PIN_REMOTE_CFG_11TH_PINMUX = 0x400,   ///< Enable Remote/IrDA. (D_GPIO[4])
	PIN_REMOTE_CFG_12TH_PINMUX = 0x800,   ///< Not Support
	PIN_REMOTE_CFG_13TH_PINMUX = 0x1000,  ///< Not Support

	ENUM_DUMMY4WORD(PIN_REMOTE_CFG)
} PIN_REMOTE_CFG;


typedef enum {
	PIN_VCAP_CFG_NONE,

	PIN_VCAP_CFG_CAP0_1ST_PINMUX     = 0x1,         ///< Enable CAP. (S_GPIO[8..1])
	PIN_VCAP_CFG_CAP1_1ST_PINMUX     = 0x2,         ///< Enable CAP1. (S_GPIO[17..10])
	PIN_VCAP_CFG_CAP2_1ST_PINMUX     = 0x4,         ///< Enable CAP2. (S_GPIO[26..19])
	PIN_VCAP_CFG_CAP3_1ST_PINMUX     = 0x8,         ///< Enable CAP3. (S_GPIO[35..28])
	PIN_VCAP_CFG_CAP4_1ST_PINMUX     = 0x10,        ///< Not Support
	PIN_VCAP_CFG_CAP5_1ST_PINMUX     = 0x20,        ///< Not Support
	PIN_VCAP_CFG_CAP6_1ST_PINMUX     = 0x40,        ///< Not Support
	PIN_VCAP_CFG_CAP7_1ST_PINMUX     = 0x80,        ///< Not Support

	PIN_VCAP_CFG_CAP0_CLK_1ST_PINMUX = 0x100,       ///< Enable CAP_CLK. (S_GPIO[0])
	PIN_VCAP_CFG_CAP0_CLK_2ND_PINMUX = 0x200,       ///< Enable CAP_2_CLK. (P_GPIO[23])
	PIN_VCAP_CFG_CAP1_CLK_1ST_PINMUX = 0x400,       ///< Enable CAP1_CLK. (S_GPIO[9])
	PIN_VCAP_CFG_CAP1_CLK_2ND_PINMUX = 0x800,       ///< Enable CAP1_2_CLK. (S_GPIO[0])
	PIN_VCAP_CFG_CAP2_CLK_1ST_PINMUX = 0x1000,      ///< Enable CAP2_CLK. (S_GPIO[18])
	PIN_VCAP_CFG_CAP2_CLK_2ND_PINMUX = 0x2000,      ///< Enable CAP2_2_CLK. (S_GPIO[9])
	PIN_VCAP_CFG_CAP3_CLK_1ST_PINMUX = 0x4000,      ///< Enable CAP3_CLK. (S_GPIO[27])
	PIN_VCAP_CFG_CAP3_CLK_2ND_PINMUX = 0x8000,      ///< Enable CAP3_2_CLK. (S_GPIO[18])

	PIN_VCAP_CFG_CAP4_CLK_1ST_PINMUX = 0x10000,     ///< Not Support
	PIN_VCAP_CFG_CAP4_CLK_2ND_PINMUX = 0x20000,     ///< Not Support
	PIN_VCAP_CFG_CAP5_CLK_1ST_PINMUX = 0x40000,     ///< Not Support
	PIN_VCAP_CFG_CAP5_CLK_2ND_PINMUX = 0x80000,     ///< Not Support
	PIN_VCAP_CFG_CAP6_CLK_1ST_PINMUX = 0x100000,    ///< Not Support
	PIN_VCAP_CFG_CAP6_CLK_2ND_PINMUX = 0x200000,    ///< Not Support
	PIN_VCAP_CFG_CAP7_CLK_1ST_PINMUX = 0x400000,    ///< Not Support
	PIN_VCAP_CFG_CAP7_CLK_2ND_PINMUX = 0x800000,    ///< Not Support

	
	PIN_VCAP_CFG_CAP3_CLK_3RD_PINMUX = 0x1000000,   ///< Enable CAP3_3_CLK. (P_GPIO[14])

	ENUM_DUMMY4WORD(PIN_VCAP_CFG)
}PIN_VCAP_CFG;


typedef enum {
	PIN_ETH_CFG_NONE,

	PIN_ETH_CFG_RGMII_1ST_PINMUX  = 0x001,    ///< Enable RGMII. (E_GPIO[13..2])
	PIN_ETH_CFG_RGMII_2ND_PINMUX  = 0x002,    ///< Not Support

	PIN_ETH_CFG_RMII_1ST_PINMUX   = 0x010,    ///< Enabl RMII. (E_GPIO[5..2], E_GPIO[11..9])
	PIN_ETH_CFG_RMII_2ND_PINMUX   = 0x020,    ///< Not Support

	PIN_ETH_CFG_REFCLK_PINMUX     = 0x100,    ///< Enable REFCLK. (E_GPIO[0])
	PIN_ETH_CFG_RST_PINMUX        = 0x200,    ///< Not Support
	PIN_ETH_CFG_MDC_MDIO_PINMUX   = 0x400,    ///< Enable MDC/MDIO. (E_GPIO[15..14])

	PIN_ETH2_CFG_RGMII_1ST_PINMUX  = 0x1000,  ///< Enable ETH2 RGMII.(S_GPIO[13..2])
	PIN_ETH2_CFG_RGMII_2ND_PINMUX  = 0x2000,  ///< Not Support

	PIN_ETH2_CFG_RMII_1ST_PINMUX   = 0x10000, ///< Enable RMII.(S_GPIO[5..2], S_GPIO[11..9])
	PIN_ETH2_CFG_RMII_2ND_PINMUX   = 0x20000, ///< Not Support

	PIN_ETH2_CFG_REFCLK_PINMUX     = 0x100000,///< Enable REFCLK. (S_GPIO[0])
	PIN_ETH2_CFG_RST_PINMUX        = 0x200000,///< Not Support
	PIN_ETH2_CFG_MDC_MDIO_PINMUX   = 0x400000,///< Enable MDC/MDIO. (S_GPIO[15..14])

	PIN_ETH_CFG_ACT_LED_1ST_PINMUX  = 0x1000000, ///< ETH_ACT_LED (J_GPIO[1])
	PIN_ETH_CFG_ACT_LED_2ND_PINMUX  = 0x2000000, ///< ETH_2_ACT_LED (D_GPIO[8])
	PIN_ETH_CFG_LINK_LED_1ST_PINMUX = 0x1000000, ///< ETH_LINK_LED (J_GPIO[2])
	PIN_ETH_CFG_LINK_LED_2ND_PINMUX = 0x2000000, ///< ETH_2_LINK_LED (D_GPIO[9])

	PIN_ETH_CFG_PHY_SEL             = 0x10000000, ///< ETH PHY Sel from GMAC0 or GMAC1

	ENUM_DUMMY4WORD(PIN_ETH_CFG)
}PIN_ETH_CFG;


typedef enum {
	PIN_MISC_CFG_NONE,

	PIN_MISC_CFG_CPU_ICE 			  = 0x00000001,///< Enable CPU ICE @ CPU ICE interface
	PIN_MISC_CFG_VGA_HS  			  = 0x00000002,///< Enable VGA_HS (P_GPIO[27])
	PIN_MISC_CFG_VGA_VS  			  = 0x00000004,///< Enable VGA_VS (P_GPIO[28])
	PIN_MISC_CFG_BMC                  = 0x00000008,///< Enable BMC (D_GPIO[8..5])
	PIN_MISC_CFG_RTC_CAL_OUT		  = 0x00000010,///< Enable RTC_CAL_OUT (P_GPIO[2])
	PIN_MISC_CFG_DAC_RAMP_TP		  = 0x00000020,///< Enable DAC_RAMP_TP (P_GPIO[5])
	PIN_MISC_CFG_HDMI_HOTPLUG		  = 0x00000040,///< Enable HDMI_HPD (P_GPIO[31])

	PIN_MISC_CFG_SATA_LED_1ST_PINMUX  = 0x00100000,///< Enable SATA activity LED. (J_GPIO[3])
	PIN_MISC_CFG_SATA_LED_2ND_PINMUX  = 0x00200000,///< Enable SATA activity LED. (D_GPIO[3])
	PIN_MISC_CFG_SATA_LED_3RD_PINMUX  = 0x00400000,///< Not Support

	PIN_MISC_CFG_SATA2_LED_1ST_PINMUX = 0x01000000,///< Enable SATA2 activity LED. (J_GPIO[4])
	PIN_MISC_CFG_SATA2_LED_2ND_PINMUX = 0x02000000,///< Enable SATA2 activity LED. (D_GPIO[4])
	PIN_MISC_CFG_SATA2_LED_3RD_PINMUX = 0x04000000,///< Not Support

	PIN_MISC_CFG_SATA3_LED_1ST_PINMUX = 0x10000000,///< Not Support
	PIN_MISC_CFG_SATA3_LED_2ND_PINMUX = 0x20000000,///< Not Support
	PIN_MISC_CFG_SATA3_LED_3RD_PINMUX = 0x40000000,///< Not Support

	ENUM_DUMMY4WORD(PIN_MISC_CFG)
}PIN_MISC_CFG;


typedef enum {
	PIN_PWM_CFG_NONE,

	PIN_PWM_CFG_CH_1ST_PINMUX = 0x01,  ///< Enable PWM. (P_GPIO[24])

	PIN_PWM_CFG_CH2_1ST_PINMUX = 0x10, ///< Enable PWM2. (P_GPIO[25])

	PIN_PWM_CFG_CH3_1ST_PINMUX = 0x100,///< Enable PWM3. (P_GPIO[19])

	PIN_PWM_CFG_CH4_1ST_PINMUX = 0x1000,///< Not Support

	ENUM_DUMMY4WORD(PIN_PWM_CFG)
} PIN_PWM_CFG;

#define PIN_PWM_CFG_CH1_1ST_PINMUX PIN_PWM_CFG_CH2_1ST_PINMUX

/*typedef enum {
	PIN_MIPI_CFG_NONE = 0x0,

	PIN_MIPI_CFG_PHY_CLK0  = 0x1,           ///< Enable PHY  CLK0. (S_GPIO[41..40])
	PIN_MIPI_CFG_PHY_CLK1  = 0x2,           ///< Enable PHY  CLK1. (S_GPIO[47..46])
	PIN_MIPI_CFG_PHY2_CLK0 = 0x4,			///< Enable PHY2 CLK0. (S_GPIO[53..52])
	PIN_MIPI_CFG_PHY2_CLK1 = 0x8,			///< Enable PHY2 CLK1. (S_GPIO[59..58])
	PIN_MIPI_CFG_PHY3_CLK0 = 0x10,			///< Enable PHY3 CLK0. (S_GPIO[65..64])
	PIN_MIPI_CFG_PHY3_CLK1 = 0x20,			///< Enable PHY3 CLK1. (S_GPIO[71..70])
	PIN_MIPI_CFG_PHY4_CLK0 = 0x40,			///< Enable PHY4 CLK0. (S_GPIO[77..76])
	PIN_MIPI_CFG_PHY4_CLK1 = 0x80,			///< Enable PHY4 CLK1. (S_GPIO[83..82])

	PIN_MIPI_CFG_PHY_DAT0  = 0x100,			///< Enable PHY  DAT0. (S_GPIO[37..36])
	PIN_MIPI_CFG_PHY_DAT1  = 0x200,			///< Enable PHY  DAT1. (S_GPIO[39..38])
	PIN_MIPI_CFG_PHY_DAT2  = 0x400,			///< Enable PHY  DAT2. (S_GPIO[43..42])
	PIN_MIPI_CFG_PHY_DAT3  = 0x800,			///< Enable PHY  DAT3. (S_GPIO[45..44])

	PIN_MIPI_CFG_PHY2_DAT0 = 0x1000,		///< Enable PHY2 DAT0. (S_GPIO[49..48])
	PIN_MIPI_CFG_PHY2_DAT1 = 0x2000,		///< Enable PHY2 DAT1. (S_GPIO[51..50])
	PIN_MIPI_CFG_PHY2_DAT2 = 0x4000,		///< Enable PHY2 DAT2. (S_GPIO[54..55])
	PIN_MIPI_CFG_PHY2_DAT3 = 0x8000,		///< Enable PHY2 DAT3. (S_GPIO[57..56])

	PIN_MIPI_CFG_PHY3_DAT0 = 0x10000,		///< Enable PHY3 DAT0. (S_GPIO[61..60])
	PIN_MIPI_CFG_PHY3_DAT1 = 0x20000,		///< Enable PHY3 DAT1. (S_GPIO[63..62])
	PIN_MIPI_CFG_PHY3_DAT2 = 0x40000,		///< Enable PHY3 DAT2. (S_GPIO[67..66])
	PIN_MIPI_CFG_PHY3_DAT3 = 0x80000,		///< Enable PHY3 DAT3. (S_GPIO[69..68])

	PIN_MIPI_CFG_PHY4_DAT0 = 0x100000,		///< Enable PHY4 DAT0. (S_GPIO[73..72])
	PIN_MIPI_CFG_PHY4_DAT1 = 0x200000,		///< Enable PHY4 DAT1. (S_GPIO[75..74])
	PIN_MIPI_CFG_PHY4_DAT2 = 0x400000,		///< Enable PHY4 DAT2. (S_GPIO[79..78])
	PIN_MIPI_CFG_PHY4_DAT3 = 0x800000,		///< Enable PHY4 DAT3. (S_GPIO[81..80])

	ENUM_DUMMY4WORD(PIN_MIPI_CFG)
} PIN_MIPI_CFG;
*/


typedef enum {
	PIN_VCAPINT_CFG_NONE,
/*
	PIN_VCAPINT_CFG_CAP0_CFG_0    = 0x1,           ///< VCAP internal mux select
	PIN_VCAPINT_CFG_CAP0_CFG_2    = 0x2,           ///< VCAP internal mux select
	PIN_VCAPINT_CFG_CAP0_CFG_3    = 0x4,           ///< VCAP internal mux select
	PIN_VCAPINT_CFG_CAP0_CFG_4    = 0x8,           ///< VCAP internal mux select

	PIN_VCAPINT_CFG_CAP1_CFG_0    = 0x100,         ///< VCAP internal mux select
	PIN_VCAPINT_CFG_CAP1_CFG_2    = 0x200,         ///< VCAP internal mux select
	PIN_VCAPINT_CFG_CAP1_CFG_3    = 0x400,         ///< VCAP internal mux select
	PIN_VCAPINT_CFG_CAP1_CFG_4    = 0x800,         ///< VCAP internal mux select

	PIN_VCAPINT_CFG_CAP2_CFG_0    = 0x10000,       ///< VCAP internal mux select
	PIN_VCAPINT_CFG_CAP2_CFG_1    = 0x20000,       ///< VCAP internal mux select
	PIN_VCAPINT_CFG_CAP2_CFG_2    = 0x40000,       ///< VCAP internal mux select
	PIN_VCAPINT_CFG_CAP2_CFG_3    = 0x80000,       ///< VCAP internal mux select
	PIN_VCAPINT_CFG_CAP2_CFG_4    = 0x100000,      ///< VCAP internal mux select

	PIN_VCAPINT_CFG_CAP3_CFG_0    = 0x1000000,     ///< VCAP internal mux select
	PIN_VCAPINT_CFG_CAP3_CFG_1    = 0x2000000,     ///< VCAP internal mux select
	PIN_VCAPINT_CFG_CAP3_CFG_2    = 0x4000000,     ///< VCAP internal mux select
	PIN_VCAPINT_CFG_CAP3_CFG_3    = 0x8000000,     ///< VCAP internal mux select
	PIN_VCAPINT_CFG_CAP3_CFG_4    = 0x10000000,    ///< VCAP internal mux select
*/
	ENUM_DUMMY4WORD(PIN_VCAPINT_CFG)
}PIN_VCAPINT_CFG;


/**
    Pinmux group

    @note For pinmux_init()
*/
typedef struct {
	PIN_FUNC    pin_function;            ///< PIN Function group
	UINT32      config;                 ///< Configuration for pinFunction
} PIN_GROUP_CONFIG;

//@}    //addtogroup mIHALSysCfg


enum CHIP_ID {
	CHIP_NA51055 = 0x4821,
	CHIP_NA51084 = 0x5021,
	CHIP_NA51089 = 0x7021,
	CHIP_NA51090 = 0xBC21,
	CHIP_NA51102 = 0x5221,
	CHIP_NA51103 = 0x8B20,
	CHIP_NS02201 = 0xF221,
	CHIP_NS02301 = 0x7721,
	CHIP_NS02302 = 0x5A21,
	CHIP_NS02401 = 0xBB21,
	CHIP_NS02402 = 0x5B21,
};

int nvt_pinmux_capture(PIN_GROUP_CONFIG *pinmux_config, int count);
int nvt_pinmux_update(PIN_GROUP_CONFIG *pinmux_config, int count);
int nvt_pinmux_capture_ep(PIN_GROUP_CONFIG *pinmux_config, int count, int ep);
int nvt_pinmux_update_ep(PIN_GROUP_CONFIG *pinmux_config, int count, int ep);
UINT32 nvt_get_chip_id(void);
int nvt_pinmux_init(void);
int pinmux_select_debugport(PINMUX_DEBUGPORT uiDebug);
UINT32 top_get_bs(void);
#endif /* __SOC_NVT_PLAT_NA51103_TOP_H */
