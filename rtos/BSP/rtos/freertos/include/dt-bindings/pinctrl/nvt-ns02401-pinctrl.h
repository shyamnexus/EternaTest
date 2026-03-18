/* SPDX-License-Identifier: GPL-2.0 */
/*
 * This header provides constants specific to Novatek NS02401 pinctrl bindings.
 */

#ifndef __DT_BINDINGS_PINCTRL_NS02401_PINCTRL_H
#define __DT_BINDINGS_PINCTRL_NS02401_PINCTRL_H

/**
    Function group

    @note For pinmux_init()
*/
#define PIN_FUNC_SDIO                   0
#define PIN_FUNC_NAND                   1
#define PIN_FUNC_ETH                    2
#define PIN_FUNC_I2C                    3
#define PIN_FUNC_PWM                    4
#define PIN_FUNC_AUDIO                  5
#define PIN_FUNC_UART                   6
#define PIN_FUNC_REMOTE                 7
#define PIN_FUNC_SDP                    8
#define PIN_FUNC_SPI                    9
#define PIN_FUNC_MISC                   10
#define PIN_FUNC_LCD                    11
#define PIN_FUNC_TV                     12
#define PIN_FUNC_SEL_LCD                13
#define PIN_FUNC_PCIE                   14
#define PIN_FUNC_SEL_PCIE               15
#define PIN_FUNC_SATA_LED               16
#define PIN_FUNC_MAX                    17

// backward compatible:
#define PIN_FUNC_I2CIII                 99
#define PIN_FUNC_SENSORMISC             99
#define PIN_FUNC_LCD_PINOUT_SEL         99


/**
    PIN config for SDIO

    @note For pinmux_init() with PIN_FUNC_SDIO.
*/
#define PIN_SDIO_CFG_NONE               0x0

#define PIN_SDIO_CFG_SDIO_1             0x1         ///< SDIO   (P_GPIO[12..13], P_GPIO[15..18])
#define PIN_SDIO_CFG_SDIO_2             0x2         ///< SDIO_2 (P_GPIO[19..20], P_GPIO[23..25])
#define PIN_SDIO_CFG_SDIO_3             0x4         ///< SDIO_3 (E_GPIO[24..29])

#define PIN_SDIO_CFG_SDIO2_1            0x10        ///< SDIO2 (C_GPIO[8..13])
#define PIN_SDIO_CFG_SDIO2_BUS_WIDTH    0x20        ///< SDIO2 bus width 8 bits (C_GPIO[14..17])
#define PIN_SDIO_CFG_SDIO2_DS           0x40        ///< SDIO2 Data Strobe (C_GPIO[6])

#define PIN_SDIO_CFG_1ST_PINMUX         PIN_SDIO_CFG_SDIO_1
#define PIN_SDIO2_CFG_1ST_PINMUX        PIN_SDIO_CFG_SDIO2_1

// host_id refer to SDIO_HOST_ID
#define PIN_SDIO_CFG_MASK(host_id)   (0xF << (4 * host_id))

// backward compatible: SDIO
#define PIN_SDIO_CFG_SDIO3_1            0x0

/**
    PIN config for NAND

    @note For pinmux_init() with PIN_FUNC_NAND.
*/
#define PIN_NAND_CFG_NONE               0x0

#define PIN_NAND_CFG_NAND_1             0x1          ///< FSPI 4bit (C_GPIO[0..5])
#define PIN_NAND_CFG_NAND_8BIT          0x2          ///< FSPI 8bit (C_GPIO[0..6], C_GPIO[10..13])
#define PIN_NAND_CFG_NAND_CS1           0x4          ///< FSPI CS1  (C_GPIO[9])

/**
    PIN config for ETH

    @note For pinmux_init() for PIN_FUNC_ETH
*/
#define PIN_ETH_CFG_NONE                0x0

#define PIN_ETH_CFG_ETH_RGMII_1         0x1          ///< ETH_RGMII      (E_GPIO[2..15])
#define PIN_ETH_CFG_ETH_RMII_1          0x10         ///< ETH_RMII       (E_GPIO[2..4], E_GPIO[7], E_GPIO[9..11], E_GPIO[14..15])
#define PIN_ETH_CFG_ETH_SGMII_1         0x100        ///< ETH_SGRMII     (E_GPIO[14..15])
#define PIN_ETH_CFG_ETH_EXTPHYCLK       0x1000       ///< ETH_EXTPHYCLK	 (E_GPIO[0])

#define PIN_ETH_CFG_ETH2_RGMII_1        0x10000      ///< ETH2_RGMII     (E_GPIO[18..31])
#define PIN_ETH_CFG_ETH2_RMII_1         0x100000     ///< ETH2_RMII      (E_GPIO[18..20], E_GPIO[23], E_GPIO[25..27], E_GPIO[30..31])
#define PIN_ETH_CFG_ETH2_SGMII_1        0x1000000    ///< ETH2_SGRMII    (E_GPIO[30..31])
#define PIN_ETH_CFG_ETH2_EXTPHYCLK      0x10000000   ///< ETH2_EXTPHYCLK (E_GPIO[16])


/**
    PIN config for I2C

    @note For pinmux_init() with PIN_FUNC_I2C.
*/
#define PIN_I2C_CFG_NONE                0x0

#define PIN_I2C_CFG_I2C_1               0x1      ///< I2C    (P_GPIO[12..13])

#define PIN_I2C_CFG_I2C2_1              0x10     ///< I2C2   (P_GPIO[19..20])

#define PIN_I2C_CFG_I2C3_1              0x100    ///< I2C3   (P_GPIO[32..33])
#define PIN_I2C_CFG_I2C3_2              0x200    ///< I2C3_2 (C_GPIO[16..17])
#define PIN_I2C_CFG_I2C3_3              0x400    ///< I2C3_3 (D_GPIO[10..11])
#define PIN_I2C_CFG_I2C3_4              0x800    ///< I2C3_4 (P_GPIO[6..7])

#define PIN_I2C_CFG_I2C4_1              0x1000   ///< I2C4   (D_GPIO[3..4])
#define PIN_I2C_CFG_I2C4_2              0x2000   ///< I2C4_2 (J_GPIO[0..1])
#define PIN_I2C_CFG_I2C4_3              0x4000   ///< I2C4_3 (P_GPIO[26..27])
#define PIN_I2C_CFG_I2C4_4              0x8000   ///< I2C4_4 (P_GPIO[10..11])

#define PIN_I2C_CFG_I2C5_1              0x10000  ///< I2C5   (D_GPIO[6..7])
#define PIN_I2C_CFG_I2C5_2              0x20000  ///< I2C5_2 (J_GPIO[3..4])
#define PIN_I2C_CFG_I2C5_3              0x40000  ///< I2C5_3 (C_GPIO[12..13])
#define PIN_I2C_CFG_I2C5_4              0x80000  ///< I2C5_4 (P_GPIO[36..37])

#define PIN_I2C_CFG_I2C6_1              0x100000    ///< I2C6  (P_GPIO[39..40])

#define PIN_I2C_CFG_I2C7_1              0x1000000   ///< I2C7  (P_GPIO[42..43])

#define PIN_I2C_CFG_I2C8_1              0x10000000  ///< I2C8  (P_GPIO[45..46])

// backward compatible: I2C
#define PIN_I2C_CFG_I2C2_2              0x0      ///< I2C2_2     (P_GPIO[30..31])

// backward compatible: I2CIII
#define PIN_I2CIII_CFG_I2C13_1          0x0
#define PIN_I2CIII_CFG_I2C14_1          0x0



/**
    PIN config for PWM

    @note For pinmux_init() with PIN_FUNC_PWM.
*/
#define PIN_PWM_CFG_NONE                0x0

#define PIN_PWM_CFG_PWM_1               0x1         ///< PWM   (P_GPIO[28])
#define PIN_PWM_CFG_PWM2_1              0x10        ///< PWM2  (P_GPIO[29])
#define PIN_PWM_CFG_PWM3_1              0x100       ///< PWM3  (P_GPIO[30])
#define PIN_PWM_CFG_PWM4_1              0x1000      ///< PWM4  (P_GPIO[31])
#define PIN_PWM_CFG_PWM5_1              0x10000     ///< PWM5  (D_GPIO[0])
#define PIN_PWM_CFG_PWM6_1              0x100000    ///< PWM6  (D_GPIO[5])


/**
    PIN config for AUDIO (I2S)

    @note For pinmux_init() with PIN_FUNC_AUDIO.
*/
#define PIN_AUDIO_CFG_NONE          0x0
#define PIN_AUDIO_CFG_I2S_1         0x1          ///< I2S_1       (P_GPIO[15..18])
#define PIN_AUDIO_CFG_I2S_2         0x2          ///< I2S_2       (P_GPIO[34..37])
#define PIN_AUDIO_CFG_I2S_MCLK_1    0x4          ///< I2S_MCLK_1  (P_GPIO[14])
#define PIN_AUDIO_CFG_I2S_MCLK_2    0x8          ///< I2S_MCLK_2  (P_GPIO[38])

#define PIN_AUDIO_CFG_I2S2_1        0x10         ///< I2S2_1      (P_GPIO[22..25])
#define PIN_AUDIO_CFG_I2S2_2        0x20         ///< I2S2_2      (D_GPIO[1..4])
#define PIN_AUDIO_CFG_I2S2_3        0x40         ///< I2S2_3      (D_GPIO[6..9])
#define PIN_AUDIO_CFG_I2S2_MCLK_1   0x80         ///< I2S2_MCLK_1 (P_GPIO[21])
#define PIN_AUDIO_CFG_I2S2_MCLK_2   0x100        ///< I2S2_MCLK_2 (D_GPIO[0])
#define PIN_AUDIO_CFG_I2S2_MCLK_3   0x200        ///< I2S2_MCLK_3 (D_GPIO[5])

#define PIN_AUDIO_CFG_I2S3_1        0x1000       ///< I2S3_1      (C_GPIO[14..17])
#define PIN_AUDIO_CFG_I2S3_MCLK_1   0x4000       ///< I2S3_MCLK_1 (P_GPIO[28])

#define PIN_AUDIO_CFG_I2S4_1        0x10000      ///< I2S4_1      (E_GPIO[25..28])
#define PIN_AUDIO_CFG_I2S4_MCLK_1   0x40000      ///< I2S4_MCLK_1 (E_GPIO[24])


/**
    PIN config for UART

    @note For pinmux_init() with PIN_FUNC_UART.
*/
#define PIN_UART_CFG_NONE               0x0

#define PIN_UART_CFG_UART_1             0x1          ///< UART_1  (P_GPIO[0..1])
#define PIN_UART_CFG_UART_2             0x2          ///< UART_2  (P_GPIO[26..27])

#define PIN_UART_CFG_UART2_1            0x10         ///< UART2_1 (P_GPIO[4..5])
#define PIN_UART_CFG_UART2_2            0x20         ///< UART2_2 (C_GPIO[8..9])

#define PIN_UART_CFG_UART3_1            0x100        ///< UART3_1 (P_GPIO[8..9])
#define PIN_UART_CFG_UART3_2            0x200        ///< UART3_2 (D_GPIO[1..2])

#define PIN_UART_CFG_UART4_1            0x1000       ///< UART4_1 (P_GPIO[10..11])
#define PIN_UART_CFG_UART4_2            0x2000       ///< UART4_2 (P_GPIO[12..13])
#define PIN_UART_CFG_UART4_3            0x4000       ///< UART4_3 (P_GPIO[19..20])

#define PIN_UART_CFG_UART5_1            0x10000      ///< UART5_1 (D_GPIO[8..9])
#define PIN_UART_CFG_UART5_2            0x20000      ///< UART5_2 (C_GPIO[14..15])

#define PIN_UART_CFG_UART_RTSCTS_1      0x100000     ///< UART_RTS/CTS   (P_GPIO[2..3])
#define PIN_UART_CFG_UART_RTSCTS_2      0x200000     ///< UART_2_RTS/CTS (P_GPIO[28..29])

#define PIN_UART_CFG_UART2_RTSCTS_1     0x1000000    ///< UART2_RTS/CTS   (P_GPIO[6..7])
#define PIN_UART_CFG_UART2_RTSCTS_2     0x2000000    ///< UART2_2_RTS/CTS (P_GPIO[10..11])

#define PIN_UART_CFG_UART3_RTSCTS_1     0x10000000   ///< UART3_RTS/CTS   (D_GPIO[8..9])
#define PIN_UART_CFG_UART3_RTSCTS_2     0x20000000   ///< UART3_2_RTS/CTS (D_GPIO[3..4])


/**
    PIN config for REMOTE (IrDA)

    @note For pinmux_init() with PIN_FUNC_REMOTE.
*/
#define PIN_REMOTE_CFG_NONE             0x0

#define PIN_REMOTE_CFG_REMOTE_1         0x1        ///< REMOTE_1   (C_GPIO[13])
#define PIN_REMOTE_CFG_REMOTE_2         0x2        ///< REMOTE_2   (C_GPIO[15])
#define PIN_REMOTE_CFG_REMOTE_3         0x4        ///< REMOTE_3   (C_GPIO[17])
#define PIN_REMOTE_CFG_REMOTE_4         0x8        ///< REMOTE_4   (J_GPIO[2])
#define PIN_REMOTE_CFG_REMOTE_5         0x10       ///< REMOTE_5   (P_GPIO[21])
#define PIN_REMOTE_CFG_REMOTE_6         0x20       ///< REMOTE_6   (P_GPIO[38])
#define PIN_REMOTE_CFG_REMOTE_7         0x40       ///< REMOTE_7   (D_GPIO[1])
#define PIN_REMOTE_CFG_REMOTE_8         0x80       ///< REMOTE_8   (D_GPIO[7])
#define PIN_REMOTE_CFG_REMOTE_9         0x100      ///< REMOTE_9   (D_GPIO[10])
#define PIN_REMOTE_CFG_REMOTE_10        0x200      ///< REMOTE_10  (E_GPIO[16])


/**
    PIN config for SDP

    @note For pinmux_init() with PIN_FUNC_SDP.
*/
#define PIN_SDP_CFG_NONE                0x0

#define PIN_SDP_CFG_SDP_1               0x1          ///< SDP_1 (P_GPIO[22..25])
#define PIN_SDP_CFG_SDP_2               0x2          ///< SDP_2 (D_GPIO[1..4])
#define PIN_SDP_CFG_SDP_RDY_1           0x10         ///< SDP_RDY_1 (P_GPIO[21])
#define PIN_SDP_CFG_SDP_RDY_2           0x20         ///< SDP_RDY_2 (D_GPIO[0])

/**
    PIN config for SPI

    @note For pinmux_init() with PIN_FUNC_SPI.
*/
#define PIN_SPI_CFG_NONE                0x0

#define PIN_SPI_CFG_SPI_1               0x1          ///< SPI   (P_GPIO[22..24])  BUS_WIDTH (P_GPIO[25])
#define PIN_SPI_CFG_SPI_BUS_WIDTH       0x8

#define PIN_SPI_CFG_SPI2_1              0x10         ///< SPI2   (C_GPIO[14..16])  BUS_WIDTH (C_GPIO[17])
#define PIN_SPI_CFG_SPI2_2              0x20         ///< SPI2_2 (P_GPIO[34..36])  BUS_WIDTH (P_GPIO[37])
#define PIN_SPI_CFG_SPI2_3              0x40         ///< SPI2_3 (P_GPIO[0..2])    BUS_WIDTH (P_GPIO[3])
#define PIN_SPI_CFG_SPI2_4              0x80         ///< SPI2_4 (D_GPIO[1..3])    BUS_WIDTH (D_GPIO[4])
#define PIN_SPI_CFG_SPI2_BUS_WIDTH      0x100


/**
    PIN config for MISC

    @note For pinmux_init() for PIN_FUNC_MISC.
*/
#define PIN_MISC_CFG_NONE                   0x0

#define PIN_MISC_CFG_RTC_CLK_1              0x1          ///< RTC_CLK     (P_GPIO[2])
#define PIN_MISC_CFG_RTC_EXT_CLK_1          0x2          ///< RTC_EXT_CLK (P_GPIO[9])
#define PIN_MISC_CFG_RTC_DIV_OUT_1          0x4          ///< RTC_DIV_OUT (P_GPIO[8])
#define PIN_MISC_CFG_CLKOUT                 0x8          ///< 12M_CLKOUT  (D_GPIO[5])

#define PIN_MISC_CFG_EXT_CLK_1              0x10         ///< EXT_CLK    (P_GPIO[26])
#define PIN_MISC_CFG_EXT_CLK_2              0x20         ///< EXT_CLK_2  (P_GPIO[8])
#define PIN_MISC_CFG_EXT2_CLK_1             0x40         ///< EXT2_CLK   (P_GPIO[27])
#define PIN_MISC_CFG_EXT2_CLK_2             0x80         ///< EXT2_CLK_2 (D_GPIO[11])

#define PIN_MISC_CFG_ADC                    0x1000000    ///< ADC       (A_GPIO[0..3])
#define PIN_MISC_CFG_HDMI_CEC               0x2000000    ///< HDMI_CEC  (D_GPIO[5])
#define PIN_MISC_CFG_HDMI2_CEC              0x4000000    ///< HDMI2_CEC (D_GPIO[10])
#define PIN_MISC_CFG_HDMI3_CEC              0x8000000    ///< HDMI3_CEC (D_GPIO[11])

#define PIN_MISC_CFG_CPU_ICE                0x10000000   ///< CPU_ICE	(J_GPIO[0..4])

// backward compatible: MISC
#define PIN_MISC_CFG_MIPI_MODE_SEL_1C4D     0x0
#define PIN_MISC_CFG_MIPI_CK0_SEL_DSI       0x0
#define PIN_SENSORMISC_CFG_SN_MCLK_1        0x0
#define PIN_SENSORMISC_CFG_SN2_MCLK_1       0x0


/**
    PIN config for SATA LED

    @note For pinmux_init() for PIN_FUNC_SATA_LED.
*/
#define PIN_SATA_CFG_NONE                   0x0
#define PIN_SATA_CFG_SATA_LED_1             0x1          ///< SATA_LED   (P_GPIO[36])
#define PIN_SATA_CFG_SATA_LED_2             0x2          ///< SATA_LED_2 (P_GPIO[2])
#define PIN_SATA_CFG_SATA_LED_3             0x4          ///< SATA_LED_3 (P_GPIO[10])
#define PIN_SATA_CFG_SATA_LED_4             0x8          ///< SATA_LED_4 (J_GPIO[2])

#define PIN_SATA_CFG_SATA2_LED_1            0x10         ///< SATA2_LED   (P_GPIO[37])
#define PIN_SATA_CFG_SATA2_LED_2            0x20         ///< SATA2_LED_2 (P_GPIO[3])
#define PIN_SATA_CFG_SATA2_LED_3            0x40         ///< SATA2_LED_3 (P_GPIO[11])
#define PIN_SATA_CFG_SATA2_LED_4            0x80         ///< SATA2_LED_4 (J_GPIO[3])

#define PIN_SATA_CFG_SATA3_LED_1            0x100        ///< SATA3_LED   (P_GPIO[38])
#define PIN_SATA_CFG_SATA3_LED_2            0x200        ///< SATA3_LED_2 (J_GPIO[4])

#define PIN_SATA_CFG_SATA4_LED_1            0x1000       ///< SATA4_LED   (E_GPIO[20])
#define PIN_SATA_CFG_SATA4_LED_2            0x2000       ///< SATA4_LED_2 (E_GPIO[0])
#define PIN_SATA_CFG_SATA4_LED_3            0x4000       ///< SATA4_LED_3 (P_GPIO[16])

#define PIN_SATA_CFG_SATA5_LED_1            0x10000      ///< SATA5_LED   (E_GPIO[21])
#define PIN_SATA_CFG_SATA5_LED_2            0x20000      ///< SATA5_LED_2 (P_GPIO[26])
#define PIN_SATA_CFG_SATA5_LED_3            0x40000      ///< SATA5_LED_3 (P_GPIO[17])

#define PIN_SATA_CFG_SATA6_LED_1            0x100000     ///< SATA6_LED   (E_GPIO[22])
#define PIN_SATA_CFG_SATA6_LED_2            0x200000     ///< SATA6_LED_2 (P_GPIO[27])
#define PIN_SATA_CFG_SATA6_LED_3            0x400000     ///< SATA6_LED_3 (P_GPIO[18])


/**
    PIN location of LCD

    @note For pinmux_init() with PIN_FUNC_LCD or PIN_FUNC_LCD2.
    For example  you can use {PIN_FUNC_LCD  PINMUX_DISPMUX_SEL_LCD2|PINMUX_LCDMODE_XXX}
    to tell display object that PIN_FUNC_LCD is located on secondary LCD pinmux.
*/
#define PINMUX_DISPMUX_SEL_NONE                 (0x00 << 28)    ///< PINMUX none
#define PINMUX_DISPMUX_SEL_LCD                  (0x01 << 28)    ///< PINMUX at LCD interface
#define PINMUX_DISPMUX_SEL_LCD2                 (0x02 << 28)    ///< PINMUX at LCD2 interface

#define PINMUX_DISPMUX_SEL_MASK                 (0x03 << 28)

/**
    pinmux pinout selection of LCD

	@note: For the 635 platform, LCD_PINOUT_TYPE is fixed as LCD3.

*/

#define PINMUX_LCD_TYPE_PINOUT_SEL_NONE          0x0
#define PINMUX_LCD_TYPE_PINOUT_SEL_LCD           0x1
#define PINMUX_LCD_TYPE_PINOUT_SEL_LCD2          0x2
#define PINMUX_LCD_TYPE_PINOUT_SEL_LCDLITE       0x4

#define PINMUX_LCD2_TYPE_PINOUT_SEL_LCD          0x10
#define PINMUX_LCD2_TYPE_PINOUT_SEL_LCD2         0x20
#define PINMUX_LCD2_TYPE_PINOUT_SEL_LCDLITE      0x40

/**
    Pinmux selection for LCD

    @note For pinmux_init() with PIN_FUNC_SEL_LCD or PIN_FUNC_SEL_LCD2.
    For example  you can use {PIN_FUNC_SEL_LCD  PINMUX_LCD_SEL_RGB_16BITS | PINMUX_LCD_SEL_DE_ENABLE}
    to tell pinmux driver that the register of primary LCD should be set to RGB 16 bits
    and the register of PLCD_DE should be set.
*/
#define PINMUX_LCD_SEL_GPIO                     0               ///< GPIO

#define PINMUX_LCD_SEL_CCIR656                  1               ///< CCIR-656 8 bits.
                                                                ///< CCIR_CLK(B_GPIO[0]), CCIR_D[0..7](B_GPIO[1..8]) and PLCD_DE

#define PINMUX_LCD_SEL_CCIR656_16BITS           2               ///< CCIR-656 16 bits.
                                                                ///< CCIR_CLK(B_GPIO[0]), CCIR_D[0..15](B_GPIO[1..16]) and PLCD_DE

#define PINMUX_LCD_SEL_CCIR601                  3               ///< CCIR-601 8 bits.
                                                                ///< CCIR_CLK(B_GPIO[0]), CCIR_D[0..7](B_GPIO[1..8]) and PLCD_DE and LCD_HS_VS and CCIR_FIELD

#define PINMUX_LCD_SEL_CCIR601_16BITS           4               ///< CCIR-601 16 bits.
                                                                ///< CCIR_CLK(B_GPIO[0]), CCIR_D[0..15](B_GPIO[1..16]) and PLCD_DE and LCD_HS_VS and CCIR_FIELD

#define PINMUX_LCD_SEL_SERIAL_RGB_6BITS         5               ///< Serial RGB 6 bits. not support.

#define PINMUX_LCD_SEL_SERIAL_RGB_8BITS         6               ///< Serial RGB 8 bits. not support.

#define PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS       7               ///< Serial YCbCr 8 bits. not support.

#define PINMUX_LCD_SEL_PARALLE_RGB565           8               ///< Parallel RGB565. LCD3_VO_CLK (B_GPIO[0]) / LCD3_VO_R[0..4](B_GPIO[1..5]) / LCD3_VO_G[0..5](B_GPIO[9..14]) /
                                                                ///<  LCD3_VO_B[0..4](P_GPIO[32..33], P_GPIO[8..9], D_GPIO[5]) and PLCD_DE and LCD_HS_VS

#define PINMUX_LCD_SEL_PARALLE_RGB666           9               ///< Parallel RGB666. LCD3_VO_CLK (B_GPIO[0]) / LCD3_VO_R[0..5](B_GPIO[1..6]) / LCD3_VO_G[0..5](B_GPIO[9..14]) /
                                                                ///<  LCD3_VO_B[0..5](P_GPIO[32..33], P_GPIO[8..9], D_GPIO[5..6]) and PLCD_DE and LCD_HS_VS

#define PINMUX_LCD_SEL_PARALLE_RGB888           10              ///< Parallel RGB888. LCD3_VO_CLK (B_GPIO[0]) / LCD3_VO_R[0..7](B_GPIO[1..8]) / LCD3_VO_G[0..7](B_GPIO[9..16]) /
                                                                ///<  LCD3_VO_B[0..7](P_GPIO[32..33], P_GPIO[8..9], D_GPIO[5..8]) and PLCD_DE and LCD_HS_VS

#define PINMUX_LCD_SEL_RGB_16BITS               11              ///< RGB 16 bits. not support.

#define PINMUX_LCD_SEL_MIPI                     12              ///< MIPI DSI. not support.

#define PINMUX_LCD_SEL_PARALLE_MI_8BITS         13              ///< Parallel MI 8 bits. not support.

#define PINMUX_LCD_SEL_PARALLE_MI_9BITS         14              ///< Parallel MI 9 bits. not support.

#define PINMUX_LCD_SEL_PARALLE_MI_16BITS        15              ///< Parallel MI 16 bits. not support.

#define PINMUX_LCD_SEL_PARALLE_MI_18BITS        16              ///< Parallel MI 18 bits. not support.

#define PINMUX_LCD_SEL_SERIAL_MI_SDIO           17              ///< Serial MI SDIO bi-direction. not support.

#define PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO        18              ///< Serial MI SDI/SDO seperate. not support.

#define PINMUX_LCD_SEL_TE_ENABLE                (0x01 << 23)    ///< TE Enable. not support.

#define PINMUX_LCD_SEL_TE2_ENABLE               (0x01 << 28)    ///< TE2 Enable. not support.

#define PINMUX_LCD_SEL_DE_ENABLE                (0x01 << 24)    ///< DE Enable. (D_GPIO9 to DE)

#define PINMUX_LCD_SEL_HVLD_VVLD                (0x01 << 25)    ///< HVLD/VVLD Enable (For CCIR-601 8 bits). not support.

#define PINMUX_LCD_SEL_FIELD                    (0x01 << 26)    ///< FIELD Enable (For CCIR-601). CCIR_FIELD(D_GPIO[8]).

#define PINMUX_LCD_SEL_NO_HVSYNC                (0x01 << 27)    ///< No HSYNC/VSYNC. LCD_HS/LCD_VS(D_GPIO[10..11]).

#define PINMUX_LCD_SEL_FEATURE_MSK              (0x3F << 23)



#endif
