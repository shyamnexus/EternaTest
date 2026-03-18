/*
	TOP controller header

	Sets the pinmux of each module.

	@file       top.h
	@ingroup    mIDrvSys_TOP
	@note       Refer NA51090 data sheet for PIN/PAD naming

	Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_NA51090_TOP_H
#define __ASM_ARCH_NA51090_TOP_H

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
	PINMUX_DEBUGPORT_CKG =          0x0000,     ///< CKGen

	// TBD///////////////////////


	PINMUX_DEBUGPORT_GROUP_NONE =   0x0000,     ///< No debug port is output
	PINMUX_DEBUGPORT_GROUP1 =       0x0100,     ///< Output debug port to S_GPIO[18..0]
	PINMUX_DEBUGPORT_GROUP2 =       0x0200,     ///< Output debug port to D_GPIO[2..0], E_GPIO[15..0]

	ENUM_DUMMY4WORD(PINMUX_DEBUGPORT)
} PINMUX_DEBUGPORT;


typedef enum {
	PIN_UART_CFG_NONE,

	PIN_UART_CFG_CH_1ST_PINMUX = 0x01,      ///< Enable UART.  (P_GPIO[1..0])
	PIN_UART_CFG_CH_2ND_PINMUX = 0x02,      ///< Enable UART_2. (P_GPIO[16..15])
	PIN_UART_CFG_CH_3RD_PINMUX = 0x04,      ///< Enable UART_3. (D_GPIO[8..7])
	PIN_UART_CFG_CH_CTSRTS     = 0x08,      ///< Enable UART_CTS/RTS. (P_GPIO[3..2]), UART_2_CTS/RTS (P_GPIO[18..17]), UART_3_CTS/RTS(D_GPIO[10..9])

	PIN_UART_CFG_CH2_1ST_PINMUX = 0x10,     ///< Enable UART2. (P_GPIO[5..4])
	PIN_UART_CFG_CH2_2ND_PINMUX = 0x20,     ///< Enable UART2_2. (P_GPIO[9..8])
	PIN_UART_CFG_CH2_CTSRTS     = 0x40,     ///< Enable UART2_CTS/RTS. (P_GPIO[7..6]), UART2_2_CTS/RTS(C_GPIO[11..10])

	PIN_UART_CFG_CH3_1ST_PINMUX = 0x100,    ///< Enable UART3. (P_GPIO[9..8])

	PIN_UART_CFG_CH4_1ST_PINMUX = 0x1000,   ///< Enable UART4. (P_GPIO[11..10])
	PIN_UART_CFG_CH4_2ND_PINMUX = 0x2000,   ///< Enable UART4_2. (P_GPIO[13..12])

	PIN_UART_CFG_CH5_1ST_PINMUX = 0x10000,  ///< Enable UART5. (J_GPIO[4..3])
	PIN_UART_CFG_CH5_2ND_PINMUX = 0x20000,  ///< Enable UART5_2. {P_GPIO[20..19]}

	ENUM_DUMMY4WORD(PIN_UART_CFG)
} PIN_UART_CFG;

#define PIN_UART_CFG_CH0_1ST_PINMUX PIN_UART_CFG_CH_1ST_PINMUX
#define PIN_UART_CFG_CH0_CTSRTS PIN_UART_CFG_CH_CTSRTS

typedef enum {
	PIN_I2C_CFG_NONE,

	PIN_I2C_CFG_CH_1ST_PINMUX	= 0x01, 	///< Enable I2C. (P_GPIO[13..12])
	PIN_I2C_CFG_CH_2ND_PINMUX	= 0x02, 	///< Enable I2C_2. (D_GPIO[10..9])

	PIN_I2C_CFG_CH2_1ST_PINMUX	= 0x010, 	///< Enable I2C2. (P_GPIO[20..19])

	PIN_I2C_CFG_CH3_1ST_PINMUX	= 0x1000, 	///< Enable I2C3. (P_GPIO[34..33])
	PIN_I2C_CFG_CH3_2ND_PINMUX	= 0x2000, 	///< Enable I2C3_2. (J_GPIO[1..0])
	PIN_I2C_CFG_CH3_3RD_PINMUX  = 0x4000,   ///< Enable I2C3_3. (P_GPIO[7..6])
	PIN_I2C_CFG_CH3_4TH_PINMUX  = 0x8000,   ///< Enable I2C3_4. (C_GPIO[11..10])

	PIN_I2C_CFG_HDMI_1ST_PINMUX	= 0x10000,	///< Enable HDMI_I2C. (P_GPIO[38..37])

	ENUM_DUMMY4WORD(PIN_I2C_CFG)
} PIN_I2C_CFG;

#define PIN_I2C_CFG_CH0_1ST_PINMUX PIN_I2C_CFG_CH_1ST_PINMUX
#define PIN_I2C_CFG_CH1_1ST_PINMUX PIN_I2C_CFG_CH2_1ST_PINMUX


typedef enum {
	PIN_SDIO_CFG_NONE,

	PIN_SDIO_CFG_1ST_PINMUX = 0x01,  ///< SDIO. (P_GPIO[13..12], P_GPIO[18..15])
	PIN_SDIO_CFG_2ND_PINMUX = 0x02,  ///< Not Support

	PIN_SDIO2_CFG_1ST_PINMUX = 0x10, ///< SDIO2. (C_GPIO[13..8])
	PIN_SDIO2_CFG_BUS_WIDTH  = 0x20, ///< SDIO2 bus width = 8. (C_GPIO[5..2])
	PIN_SDIO2_CFG_DS         = 0x40, ///< SDIO2 Data strobe. (C_GPIO[6])

    PIN_SDIO_CFG_SDIO3_1         = 0x100,    ///< SDIO3.
    PIN_SDIO_CFG_SDIO3_BUS_WIDTH = 0x200,    ///< SDIO3 bus width 8 bits
    PIN_SDIO_CFG_SDIO3_DS        = 0x400,    ///< SDIO3 data strobe

	ENUM_DUMMY4WORD(PIN_SDIO_CFG)
} PIN_SDIO_CFG;

// host_id refer to SDIO_HOST_ID
#define PIN_SDIO_CFG_MASK(host_id)   (0xF << host_id)


typedef enum {
	PIN_SPI_CFG_NONE,

	PIN_SPI_CFG_CH_1ST_PINMUX = 0x01,   ///< Enable SPI. (C_GPIO[3..0])

	PIN_SPI_CFG_CH_BUS_WIDTH  = 0x10,   ///< SPI bus width = 4 (C_GPIO[5..4])
	PIN_SPI_CFG_CH_CS1        = 0x20,   ///< Enable SPI CS1. (C_GPIO[6])

	ENUM_DUMMY4WORD(PIN_SPI_CFG)
} PIN_SPI_CFG;


typedef enum {
	PIN_EXTCLK_CFG_NONE,

	PIN_EXTCLK_CFG_CH_1ST_PINMUX  = 0x01,   ///< Enable EXT_CLK. (P_GPIO[26])

	PIN_EXTCLK_CFG_CH2_1ST_PINMUX = 0x02,   ///< Enable EXT2_CLK. (P_GPIO[27])

	PIN_EXTCLK_CFG_CH3_1ST_PINMUX = 0x04,   ///< Enable EXT3_CLK. (P_GPIO[28])

	PIN_EXTCLK_CFG_CH4_1ST_PINMUX = 0x08,   ///< Enable EXT4_CLK. (P_GPIO[29])

	ENUM_DUMMY4WORD(PIN_EXTCLK_CFG)
} PIN_EXTCLK_CFG;
#define PIN_EXTCLK_CFG_CH0_1ST_PINMUX PIN_EXTCLK_CFG_CH_1ST_PINMUX
#define PIN_EXTCLK_CFG_CH1_1ST_PINMUX PIN_EXTCLK_CFG_CH2_1ST_PINMUX

typedef enum {
	PIN_SSP_CFG_NONE,

	PIN_SSP_CFG_CH_1ST_PINMUX		= 0x01,   	///< Enable I2S. (P_GPIO[16..15])
	PIN_SSP_CFG_CH_1ST_MCLK         = 0x02,     ///< Enable I2S_MCLK. (P_GPIO[14])
	PIN_SSP_CFG_CH_1ST_TX           = 0x04,     ///< Enable I2S_TX. (P_GPIO[18])
	PIN_SSP_CFG_CH_1ST_RX           = 0x08,     ///< Enable I2S_RX. (P_GPIO[17])

	PIN_SSP_CFG_CH2_1ST_PINMUX 		= 0x10,  	///< Enable I2S2. (P_GPIO[23..22])
	PIN_SSP_CFG_CH2_1ST_MCLK        = 0x20,     ///< Enable I2S2_MCLK (P_GPIO[21])
	PIN_SSP_CFG_CH2_1ST_TX          = 0x40,     ///< Enable I2S2_TX (P_GPIO[25])
	PIN_SSP_CFG_CH2_1ST_RX          = 0x80,     ///< Enable I2S2_RX (P_GPIO[24])

	PIN_SSP_CFG_CH3_1ST_PINMUX		= 0x100, 	///< Enable I2S3. (D_GPIO[5..4])
	PIN_SSP_CFG_CH3_2ND_PINMUX		= 0x200, 	///< Enable I2S3_2. (P_GPIO[25], P_GPIO[21])
	PIN_SSP_CFG_CH3_3RD_PINMUX		= 0x400, 	///< Enable I2S3_3. (J_GPIO[2], J_GPIO[1])

	PIN_SSP_CFG_CH3_1ST_MCLK		= 0x1000, 	///< Enable I2S3_MCLK. (D_GPIO[3])
	PIN_SSP_CFG_CH3_2ND_MCLK		= 0x2000, 	///< Enable I2S3_2_MCLK. (P_GPIO[32])
	PIN_SSP_CFG_CH3_3RD_MCLK		= 0x4000, 	///< Enable I2S3_3_MCLK. (J_GPIO[0])

	PIN_SSP_CFG_CH3_1ST_TX   		= 0x10000, 	///< Enable I2S3_TX. (D_GPIO[7])
	PIN_SSP_CFG_CH3_2ND_TX	    	= 0x20000, 	///< Enable I2S3_2_TX. (P_GPIO[18])
	PIN_SSP_CFG_CH3_3RD_TX		    = 0x40000, 	///< Enable I2S3_3_TX. (J_GPIO[4])

	PIN_SSP_CFG_CH3_1ST_RX   		= 0x100000, ///< Enable I2S3_RX. (D_GPIO[6])
	PIN_SSP_CFG_CH3_2ND_RX	    	= 0x200000,	///< Enable I2S3_2_RX. (P_GPIO[14])
	PIN_SSP_CFG_CH3_3RD_RX		    = 0x400000,	///< Enable I2S3_3_RX. (J_GPIO[3])

	PIN_SSP_CFG_CH4_1ST_PINMUX 		= 0x1000000,///< Enable I2S4. (P_GPIO[41..40])
	PIN_SSP_CFG_CH4_1ST_MCLK        = 0x2000000,///< Enable I2S4_MCLK (P_GPIO[9])
	PIN_SSP_CFG_CH4_1ST_TX          = 0x4000000,///< Enable I2S4_TX (P_GPIO[8])
	PIN_SSP_CFG_CH4_1ST_RX          = 0x8000000,///< Enable I2S4_RX (P_GPIO[9])

	ENUM_DUMMY4WORD(PIN_SSP_CFG)
} PIN_SSP_CFG;

typedef enum {
	PIN_LCD_CFG_NONE,

	PIN_LCD_CFG_LCD310_RGB888_1ST_PINMUX = 0x01,   ///< Enable LCD310 RGB888. (B_GPIO[16..0], P_GPIO[18..12], P_GPIO[26], P_GPIO[32], P_GPIO[40])
	PIN_LCD_CFG_LCD310_BT1120_1ST_PINMUX = 0x02,   ///< Enable LCD310 BT1120. (B_GPIO[16..0])

	PIN_LCD_CFG_LCD210_BT1120_1ST_PINMUX = 0x10,   ///< Enable LCD210 BT1120. (B_GPIO[16..0])

	PIN_LCD_CFG_LCD310L_RGB888_1ST_PINMUX= 0x100,  ///< Enable LCD310L RGB888. (B_GPIO[16..0], P_GPIO[18..12], P_GPIO[26], P_GPIO[32], P_GPIO[40])
	PIN_LCD_CFG_LCD310L_BT1120_1ST_PINMUX= 0x200,  ///< Enable LCD310L BT1120. (B_GPIO[16..0])

	PIN_LCD_CFG_LCD310_DE_PINMUX         = 0x1000, ///< Enable LCD310_DE. (P_GPIO[41])

	ENUM_DUMMY4WORD(PIN_LCD_CFG)
} PIN_LCD_CFG;


typedef enum {
	PIN_REMOTE_CFG_NONE,

	PIN_REMOTE_CFG_1ST_PINMUX  = 0x001,   ///< Enable Remote/IrDA. {J_GPIO[0]}
	PIN_REMOTE_CFG_2ND_PINMUX  = 0x002,   ///< Enable Remote/IrDA. {J_GPIO[1]}
	PIN_REMOTE_CFG_3RD_PINMUX  = 0x004,   ///< Enable Remote/IrDA. {P_GPIO[44]}
	PIN_REMOTE_CFG_4TH_PINMUX  = 0x008,   ///< Enable Remote/IrDA. {S_GPIO[82]}
	PIN_REMOTE_CFG_5TH_PINMUX  = 0x010,   ///< Enable Remote/IrDA. {S_GPIO[83]}
	PIN_REMOTE_CFG_6TH_PINMUX  = 0x020,   ///< Enable Remote/IrDA. {P_GPIO[12]}
	PIN_REMOTE_CFG_7TH_PINMUX  = 0x040,   ///< Enable Remote/IrDA. {P_GPIO[13]}
	PIN_REMOTE_CFG_8TH_PINMUX  = 0x080,   ///< Enable Remote/IrDA. {P_GPIO[19]}
	PIN_REMOTE_CFG_9TH_PINMUX  = 0x100,   ///< Enable Remote/IrDA. {P_GPIO[20]}
	PIN_REMOTE_CFG_10TH_PINMUX = 0x200,   ///< Enable Remote/IrDA. {P_GPIO[33]}
	PIN_REMOTE_CFG_11TH_PINMUX = 0x400,   ///< Enable Remote/IrDA. {D_GPIO[2]}
	PIN_REMOTE_CFG_12TH_PINMUX = 0x800,   ///< Enable Remote/IrDA. {D_GPIO[9]}
	PIN_REMOTE_CFG_13TH_PINMUX = 0x1000,  ///< Enable Remote/IrDA. {D_GPIO[10]}

	ENUM_DUMMY4WORD(PIN_REMOTE_CFG)
} PIN_REMOTE_CFG;


typedef enum {
	PIN_VCAP_CFG_NONE,

	PIN_VCAP_CFG_CAP0_1ST_PINMUX     = 0x1,         ///< Enable CAP. (S_GPIO[8..1])
	PIN_VCAP_CFG_CAP1_1ST_PINMUX     = 0x2,         ///< Enable CAP1. (S_GPIO[17..10])
	PIN_VCAP_CFG_CAP2_1ST_PINMUX     = 0x4,         ///< Enable CAP2. (S_GPIO[26..19])
	PIN_VCAP_CFG_CAP3_1ST_PINMUX     = 0x8,         ///< Enable CAP3. (S_GPIO[35..28])
	PIN_VCAP_CFG_CAP4_1ST_PINMUX     = 0x10,        ///< Enable CAP4. (S_GPIO[44..37])
	PIN_VCAP_CFG_CAP5_1ST_PINMUX     = 0x20,        ///< Enable CAP5. (S_GPIO[56..49])
	PIN_VCAP_CFG_CAP6_1ST_PINMUX     = 0x40,        ///< Enable CAP6. (S_GPIO[68..61])
	PIN_VCAP_CFG_CAP7_1ST_PINMUX     = 0x80,        ///< Enable CAP7. (S_GPIO[80..73])

	PIN_VCAP_CFG_CAP0_CLK_1ST_PINMUX = 0x100,       ///< Enable CAP_CLK. (S_GPIO[0])
	PIN_VCAP_CFG_CAP0_CLK_2ND_PINMUX = 0x200,       ///< Enable CAP_2_CLK. (S_GPIO[9])
	PIN_VCAP_CFG_CAP1_CLK_1ST_PINMUX = 0x400,       ///< Enable CAP1_CLK. (S_GPIO[9])
	PIN_VCAP_CFG_CAP1_CLK_2ND_PINMUX = 0x800,       ///< Enable CAP1_2_CLK. (S_GPIO[18])
	PIN_VCAP_CFG_CAP2_CLK_1ST_PINMUX = 0x1000,      ///< Enable CAP2_CLK. (S_GPIO[18])
	PIN_VCAP_CFG_CAP2_CLK_2ND_PINMUX = 0x2000,      ///< Enable CAP2_2_CLK. (S_GPIO[27])
	PIN_VCAP_CFG_CAP3_CLK_1ST_PINMUX = 0x4000,      ///< Enable CAP3_CLK. (S_GPIO[27])
	PIN_VCAP_CFG_CAP3_CLK_2ND_PINMUX = 0x8000,      ///< Enable CAP3_2_CLK. (P_GPIO[27])

	PIN_VCAP_CFG_CAP4_CLK_1ST_PINMUX = 0x10000,     ///< Enable CAP4_CLK. (S_GPIO[45])
	PIN_VCAP_CFG_CAP4_CLK_2ND_PINMUX = 0x20000,     ///< Enable CAP4_2_CLK. (S_GPIO[36])
	PIN_VCAP_CFG_CAP5_CLK_1ST_PINMUX = 0x40000,     ///< Enable CAP5_CLK. (S_GPIO[57])
	PIN_VCAP_CFG_CAP5_CLK_2ND_PINMUX = 0x80000,     ///< Enable CAP5_2_CLK. (S_GPIO[48])
	PIN_VCAP_CFG_CAP6_CLK_1ST_PINMUX = 0x100000,    ///< Enable CAP6_CLK. (S_GPIO[69])
	PIN_VCAP_CFG_CAP6_CLK_2ND_PINMUX = 0x200000,    ///< Enable CAP6_2_CLK. (S_GPIO[60])
	PIN_VCAP_CFG_CAP7_CLK_1ST_PINMUX = 0x400000,    ///< Enable CAP7_CLK. (S_GPIO[81])
	PIN_VCAP_CFG_CAP7_CLK_2ND_PINMUX = 0x800000,    ///< Enable CAP7_2_CLK. (S_GPIO[72])

	ENUM_DUMMY4WORD(PIN_VCAP_CFG)
}PIN_VCAP_CFG;


typedef enum {
	PIN_ETH_CFG_NONE,

	PIN_ETH_CFG_RGMII_1ST_PINMUX  = 0x001,    ///< Enable RGMII.
	PIN_ETH_CFG_RGMII_2ND_PINMUX  = 0x002,    ///< Enable RGMII.

	PIN_ETH_CFG_RMII_1ST_PINMUX   = 0x010,    ///< Enable RMII.
	PIN_ETH_CFG_RMII_2ND_PINMUX   = 0x020,    ///< Enable RMII.

	PIN_ETH_CFG_REFCLK_PINMUX     = 0x100,    ///< Enable REFCLK. (E_GPIO[0] or S_GPIO[0])
	PIN_ETH_CFG_RST_PINMUX        = 0x200,    ///< Enable RST. (E_GPIO[1] or S_GPIO[1])
	PIN_ETH_CFG_MDC_MDIO_PINMUX   = 0x400,    ///< Enable MDC/MDIO. (E_GPIO[15..14] or S_GPIO[15..14])

	PIN_ETH2_CFG_RGMII_1ST_PINMUX  = 0x1000,  ///< Enable RGMII.
	PIN_ETH2_CFG_RGMII_2ND_PINMUX  = 0x2000,  ///< Enable RGMII.

	PIN_ETH2_CFG_RMII_1ST_PINMUX   = 0x10000, ///< Enable RMII.
	PIN_ETH2_CFG_RMII_2ND_PINMUX   = 0x20000, ///< Enable RMII.

	PIN_ETH2_CFG_REFCLK_PINMUX     = 0x100000,///< Enable REFCLK. (E_GPIO[16] or S_GPIO[18])
	PIN_ETH2_CFG_RST_PINMUX        = 0x200000,///< Enable RST. (E_GPIO[17] or S_GPIO[19])
	PIN_ETH2_CFG_MDC_MDIO_PINMUX   = 0x400000,///< Enable MDC/MDIO. (E_GPIO[31..30] or S_GPIO[33..32])

	ENUM_DUMMY4WORD(PIN_ETH_CFG)
}PIN_ETH_CFG;


typedef enum {
	PIN_MISC_CFG_NONE,

	PIN_MISC_CFG_CPU_ICE 			  = 0x00000001,///< Enable CPU ICE @ CPU ICE interface
	PIN_MISC_CFG_VGA_HS  			  = 0x00000002,///< Enable VGA_HS
	PIN_MISC_CFG_VGA_VS  			  = 0x00000004,///< Enable VGA_VS
	PIN_MISC_CFG_BMC                  = 0x00000008,///< Enable BMC
	PIN_MISC_CFG_RTC_CAL_OUT		  = 0x00000010,///< Enable RTC_CAL_OUT
	PIN_MISC_CFG_DAC_RAMP_TP		  = 0x00000020,///< Enable DAC_RAMP_TP
	PIN_MISC_CFG_HDMI_HOTPLUG		  = 0x00000040,///< Enable HDMI_HPD
	PIN_MISC_CFG_12M_CLK                      = 0x00000080,///< Enable 12Mhz clock

	PIN_MISC_CFG_PCIE_RSTN			  = 0x00000100,	///< PCIE_RSTN (control J_GPIO[5] pinmux)
	PIN_MISC_CFG_PCIE_REFCLK_OUTEN		  = 0x00000200,	///< PCIE REFCLK pad output enable (pad name: pcie_clkp, pcie_clkn)

	PIN_MISC_CFG_RSTN_HIGHZ		  	  = 0x00000400,	///< RSTN high-Z (for PCIE_RSTN high-Z. Else it will output LOW)

	PIN_MISC_CFG_SATA_LED_1ST_PINMUX  = 0x00100000,///< Enable SATA activity LED. (P_GPIO[42])
	PIN_MISC_CFG_SATA_LED_2ND_PINMUX  = 0x00200000,///< Enable SATA activity LED. (P_GPIO[8])
	PIN_MISC_CFG_SATA_LED_3RD_PINMUX  = 0x00400000,///< Enable SATA activity LED. (D_GPIO[3])

	PIN_MISC_CFG_SATA2_LED_1ST_PINMUX = 0x01000000,///< Enable SATA2 activity LED. (P_GPIO[43])
	PIN_MISC_CFG_SATA2_LED_2ND_PINMUX = 0x02000000,///< Enable SATA2 activity LED. (P_GPIO[9])
	PIN_MISC_CFG_SATA2_LED_3RD_PINMUX = 0x04000000,///< Enable SATA2 activity LED. (D_GPIO[4])

	PIN_MISC_CFG_SATA3_LED_1ST_PINMUX = 0x10000000,///< Enable SATA3 activity LED. (J_GPIO[44])
	PIN_MISC_CFG_SATA3_LED_2ND_PINMUX = 0x20000000,///< Enable SATA3 activity LED. (P_GPIO[11])
	PIN_MISC_CFG_SATA3_LED_3RD_PINMUX = 0x40000000,///< Enable SATA3 activity LED. (D_GPIO[5])

	ENUM_DUMMY4WORD(PIN_MISC_CFG)
}PIN_MISC_CFG;


typedef enum {
	PIN_PWM_CFG_NONE,

	PIN_PWM_CFG_CH_1ST_PINMUX = 0x01,  ///< Enable PWM. (P_GPIO[30])

	PIN_PWM_CFG_CH2_1ST_PINMUX = 0x10, ///< Enable PWM2. (P_GPIO[31])

	PIN_PWM_CFG_CH3_1ST_PINMUX = 0x100,///< Enable PWM3. (P_GPIO[32])

	PIN_PWM_CFG_CH4_1ST_PINMUX = 0x1000,///< Enable PWM4. (D_GPIO[0])

	ENUM_DUMMY4WORD(PIN_PWM_CFG)
} PIN_PWM_CFG;

#define PIN_PWM_CFG_CH1_1ST_PINMUX PIN_PWM_CFG_CH2_1ST_PINMUX

typedef enum {
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
	PIN_FUNC_MIPI,      ///< MIPI. Confiuration refers to PIN_MIPI_CFG.
	//PIN_FUNC_VCAPINT,   ///< VCAP INTERNAL MUX. Configuration refers to PIN_VCAPINT_CFG
	PIN_FUNC_MAX,

	ENUM_DUMMY4WORD(PIN_FUNC)
} PIN_FUNC;

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
#endif

