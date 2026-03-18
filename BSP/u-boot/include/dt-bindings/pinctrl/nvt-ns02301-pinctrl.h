/* SPDX-License-Identifier: GPL-2.0 */
/*
 * This header provides constants specific to Novatek NS02301 pinctrl bindings.
 */

#ifndef __DT_BINDINGS_PINCTRL_NS02301_PINCTRL_H
#define __DT_BINDINGS_PINCTRL_NS02301_PINCTRL_H

/**
    Function group

    @note For pinmux_init()
*/
#define PIN_FUNC_SDIO                   0
#define PIN_FUNC_NAND                   1
#define PIN_FUNC_ETH                    2
#define PIN_FUNC_I2C                    3
#define PIN_FUNC_PWM                    4
#define PIN_FUNC_PWMII                  5
#define PIN_FUNC_PWMIII                 6
#define PIN_FUNC_CCNT                   7
#define PIN_FUNC_SENSOR                 8
#define PIN_FUNC_SENSOR2                9
#define PIN_FUNC_SENSORMISC             10
#define PIN_FUNC_SENSORSYNC             11
#define PIN_FUNC_AUDIO                  12
#define PIN_FUNC_UART                   13
#define PIN_FUNC_CSI                    14
#define PIN_FUNC_REMOTE                 15
#define PIN_FUNC_SDP                    16
#define PIN_FUNC_SPI                    17
#define PIN_FUNC_SIF                    18
#define PIN_FUNC_MISC                   19
#define PIN_FUNC_LCD                    20
#define PIN_FUNC_TV                     21
#define PIN_FUNC_SEL_LCD                22
/*Pre-roll func*/
#define PIN_FUNC_PR_I2C                 23
#define PIN_FUNC_PR_UART                24
#define PIN_FUNC_PR_SENSOR              25
#define PIN_FUNC_PR_SENSORMISC          26
#define PIN_FUNC_PR_SPI                 27
#define PIN_FUNC_PR_PWM                 28
#define PIN_FUNC_PR_AUDIO               29
#define PIN_FUNC_MAX                    30

/**
    PIN config for SDIO

    @note For pinmux_init() with PIN_FUNC_SDIO.
*/
#define PIN_SDIO_CFG_NONE               0x0

#define PIN_SDIO_CFG_SDIO_1             0x1         ///< SDIO  (C_GPIO[13..18])

#define PIN_SDIO_CFG_SDIO2_1            0x10        ///< SDIO2_1 (C_GPIO[19..24])
#define PIN_SDIO_CFG_SDIO2_2            0x20        ///< SDIO2_2 (P_GPIO[2..7])

#define PIN_SDIO_CFG_SDIO3_1            0x100       ///< SDIO3 (C_GPIO[0..3] C_GPIO[10] C_GPIO[11])
#define PIN_SDIO_CFG_SDIO3_BUS_WIDTH    0x200       ///< SDIO3 bus width 8 bits (C_GPIO[0..7])
#define PIN_SDIO_CFG_SDIO3_DS           0x400       ///< SDIO3 data strobe      (C_GPIO[8])

// compatible with na51090, na51103
#define PIN_SDIO_CFG_1ST_PINMUX         PIN_SDIO_CFG_SDIO_1
#define PIN_SDIO2_CFG_1ST_PINMUX        PIN_SDIO_CFG_SDIO2_1

// host_id refer to SDIO_HOST_ID
#define PIN_SDIO_CFG_MASK(host_id)   (0xF << host_id)

/**
    PIN config for NAND

    @note For pinmux_init() with PIN_FUNC_NAND.
*/
#define PIN_NAND_CFG_NONE               0x0

#define PIN_NAND_CFG_NAND_1             0x1          ///< FSPI 4bit    (C_GPIO[0..3] C_GPIO[10] C_GPIO[12])
#define PIN_NAND_CFG_NAND_2             0x2          ///< FSPI 8bit    (C_GPIO[0..8] C_GPIO[10] C_GPIO[12])
#define PIN_NAND_CFG_NAND_CS1           0x4          ///< FSPI CS1     (C_GPIO[9])
#define PIN_NAND_CFG_NAND_TRIGA_1       0x10         ///< FLASH_TRIGA_1(C_GPIO[21..22])
#define PIN_NAND_CFG_NAND_TRIGA_2       0x20         ///< FLASH_TRIGA_2(P_GPIO[11..12])
/**
    PIN config for ETH

    @note For pinmux_init() for PIN_FUNC_ETH
*/
#define PIN_ETH_CFG_NONE                0x0

#define PIN_ETH_CFG_ETH_RMII_1          0x1          ///< ETH_RMII_1   (P_GPIO[0..5] P_GPIO[8])

#define PIN_ETH_CFG_ETH_LED_1           0x10         ///< ETH_LED_1    (D_GPIO[0] D_GPIO[1])
#define PIN_ETH_CFG_ETH_LED_2           0x20         ///< ETH_LED_2    (D_GPIO[5] D_GPIO[6])

#define PIN_ETH_CFG_ETH_EXTPHYCLK       0x100        ///< ETH_EXTPHYCLK  RMII_1(P_GPIO[7]

#define PIN_ETH_CFG_ETH_MDIO_1          0x1000       ///< ETH_MDIO_1    (P_GPIO[9] P_GPIO[10])

/**
    PIN config for I2C

    @note For pinmux_init() with PIN_FUNC_I2C.
*/
#define PIN_I2C_CFG_NONE                0x0

#define PIN_I2C_CFG_I2C_1               0x1         ///< I2C_1      (P_GPIO[21..22])
#define PIN_I2C_CFG_I2C_2               0x2         ///< I2C_2      (C_GPIO[13..14])
#define PIN_I2C_CFG_I2C_3               0x4         ///< I2C_3      (P_GPIO[9..10])
#define PIN_I2C_CFG_I2C_4               0x8         ///< I2C_4      (P_GPIO[11..12])
#define PIN_I2C_CFG_I2C_5               0x10        ///< I2C_5      (S_GPIO[15..16])

#define PIN_I2C_CFG_I2C2_1              0x100        ///< I2C2_1     (C_GPIO[19..20])
#define PIN_I2C_CFG_I2C2_2              0x200        ///< I2C2_2     (P_GPIO[31..32])
#define PIN_I2C_CFG_I2C2_3              0x400        ///< I2C2_3     (S_GPIO[4..5])
#define PIN_I2C_CFG_I2C2_4              0x800        ///< I2C2_4     (HSI_GPIO[9..10])


/**
    PIN config for PWM

    @note For pinmux_init() with PIN_FUNC_PWM.
*/
#define PIN_PWM_CFG_NONE                0x0

#define PIN_PWM_CFG_PWM0_1              0x1          ///< PWM0_1  (P_GPIO[0])
#define PIN_PWM_CFG_PWM0_2              0x2          ///< PWM0_2  (C_GPIO[15])
#define PIN_PWM_CFG_PWM0_3              0x4          ///< PWM0_3  (S_GPIO[1])
#define PIN_PWM_CFG_PWM0_4              0x8          ///< PWM0_4  (C_GPIO[4])
#define PIN_PWM_CFG_PWM0_5              0x10         ///< PWM0_5  (S_GPIO[9])

#define PIN_PWM_CFG_PWM1_1              0x100         ///< PWM1_1  (P_GPIO[1])
#define PIN_PWM_CFG_PWM1_2              0x200         ///< PWM1_2  (C_GPIO[16])
#define PIN_PWM_CFG_PWM1_3              0x400         ///< PWM1_3  (S_GPIO[2])
#define PIN_PWM_CFG_PWM1_4              0x800         ///< PWM1_4  (C_GPIO[5])
#define PIN_PWM_CFG_PWM1_5              0x1000        ///< PWM1_5  (S_GPIO[10])

#define PIN_PWM_CFG_PWM2_1              0x10000        ///< PWM2_1  (P_GPIO[2])
#define PIN_PWM_CFG_PWM2_2              0x20000        ///< PWM2_2  (C_GPIO[17])
#define PIN_PWM_CFG_PWM2_3              0x40000        ///< PWM2_3  (S_GPIO[3])
#define PIN_PWM_CFG_PWM2_4              0x80000        ///< PWM2_4  (C_GPIO[6])
#define PIN_PWM_CFG_PWM2_5              0x100000       ///< PWM2_5  (S_GPIO[11])

#define PIN_PWM_CFG_PWM3_1              0x1000000       ///< PWM3_1  (P_GPIO[3])
#define PIN_PWM_CFG_PWM3_2              0x2000000       ///< PWM3_2  (C_GPIO[18])
#define PIN_PWM_CFG_PWM3_3              0x4000000       ///< PWM3_3  (S_GPIO[4])
#define PIN_PWM_CFG_PWM3_4              0x8000000       ///< PWM3_4  (C_GPIO[7])
#define PIN_PWM_CFG_PWM3_5              0x10000000      ///< PWM3_5  (S_GPIO[12])

/**
    PIN config for PWMII

    @note For pinmux_init() with PIN_FUNC_PWMII.
*/
#define PIN_PWMII_CFG_NONE                0x0

#define PIN_PWMII_CFG_PWM4_1              0x1          ///< PWM4_1  (P_GPIO[4])
#define PIN_PWMII_CFG_PWM4_2              0x2          ///< PWM4_2  (C_GPIO[13])
#define PIN_PWMII_CFG_PWM4_3              0x4          ///< PWM4_3  (S_GPIO[5])
#define PIN_PWMII_CFG_PWM4_4              0x8          ///< PWM4_4  (D_GPIO[3])
#define PIN_PWMII_CFG_PWM4_5              0x10         ///< PWM4_5  (S_GPIO[13])

#define PIN_PWMII_CFG_PWM5_1              0x100         ///< PWM5_1  (P_GPIO[5])
#define PIN_PWMII_CFG_PWM5_2              0x200         ///< PWM5_2  (C_GPIO[14])
#define PIN_PWMII_CFG_PWM5_3              0x400         ///< PWM5_3  (S_GPIO[6])
#define PIN_PWMII_CFG_PWM5_4              0x800         ///< PWM5_4  (D_GPIO[4])
#define PIN_PWMII_CFG_PWM5_5              0x1000        ///< PWM5_5  (S_GPIO[14])

#define PIN_PWMII_CFG_PWM6_1              0x10000        ///< PWM6_1  (P_GPIO[6])
#define PIN_PWMII_CFG_PWM6_2              0x20000        ///< PWM6_2  (C_GPIO[19])
#define PIN_PWMII_CFG_PWM6_3              0x40000        ///< PWM6_3  (S_GPIO[7])
#define PIN_PWMII_CFG_PWM6_4              0x80000        ///< PWM6_4  (D_GPIO[5])
#define PIN_PWMII_CFG_PWM6_5              0x100000       ///< PWM6_5  (S_GPIO[15])

#define PIN_PWMII_CFG_PWM7_1              0x1000000       ///< PWM7_1  (P_GPIO[7])
#define PIN_PWMII_CFG_PWM7_2              0x2000000       ///< PWM7_2  (C_GPIO[20])
#define PIN_PWMII_CFG_PWM7_3              0x4000000       ///< PWM7_3  (S_GPIO[8])
#define PIN_PWMII_CFG_PWM7_4              0x8000000       ///< PWM7_4  (D_GPIO[6])
#define PIN_PWMII_CFG_PWM7_5              0x10000000      ///< PWM7_5  (S_GPIO[16])

/**
    PIN config for PWMIII

    @note For pinmux_init() with PIN_FUNC_PWMIII.
*/
#define PIN_PWMIII_CFG_NONE                0x0

#define PIN_PWMIII_CFG_PWM8_1              0x1            ///< PWM8_1  (P_GPIO[8])
#define PIN_PWMIII_CFG_PWM8_2              0x2            ///< PWM8_2  (C_GPIO[21])
#define PIN_PWMIII_CFG_PWM8_3              0x4            ///< PWM8_3  (HSI_GPIO[6])
#define PIN_PWMIII_CFG_PWM8_4              0x8            ///< PWM8_4  (P_GPIO[25])

#define PIN_PWMIII_CFG_PWM9_1              0x100          ///< PWM9_1  (P_GPIO[9])
#define PIN_PWMIII_CFG_PWM9_2              0x200          ///< PWM9_2  (C_GPIO[22])
#define PIN_PWMIII_CFG_PWM9_3              0x400          ///< PWM9_3  (HSI_GPIO[7])
#define PIN_PWMIII_CFG_PWM9_4              0x800          ///< PWM9_4  (P_GPIO[26])

#define PIN_PWMIII_CFG_PWM10_1             0x10000        ///< PWM10_1  (P_GPIO[10])
#define PIN_PWMIII_CFG_PWM10_2             0x20000        ///< PWM10_2  (C_GPIO[23])
#define PIN_PWMIII_CFG_PWM10_3             0x40000        ///< PWM10_3  (HSI_GPIO[8])
#define PIN_PWMIII_CFG_PWM10_4             0x80000        ///< PWM10_4  (P_GPIO[27])

#define PIN_PWMIII_CFG_PWM11_1             0x1000000      ///< PWM11_1  (P_GPIO[11])
#define PIN_PWMIII_CFG_PWM11_2             0x2000000      ///< PWM11_2  (C_GPIO[24])
#define PIN_PWMIII_CFG_PWM11_3             0x4000000      ///< PWM11_3  (HSI_GPIO[9])
#define PIN_PWMIII_CFG_PWM11_4             0x8000000      ///< PWM11_4  (P_GPIO[28])

/**
    PIN config for CCNT

    @note For pinmux_init() with PIN_FUNC_CCNT.
*/
#define PIN_CCNT_CFG_NONE               0x0

#define PIN_CCNT_CFG_CCNT_1             0x1          ///< PICNT_1  (P_GPIO[12])
#define PIN_CCNT_CFG_CCNT_2             0x2          ///< PICNT_2  (P_GPIO[18])
#define PIN_CCNT_CFG_CCNT_3             0x4          ///< PICNT_3  (P_GPIO[29])

#define PIN_CCNT_CFG_CCNT2_1            0x10         ///< PICNT2_1 (C_GPIO[23])
#define PIN_CCNT_CFG_CCNT2_2            0x20         ///< PICNT2_2 (P_GPIO[19])
#define PIN_CCNT_CFG_CCNT2_3            0x40         ///< PICNT2_3 (C_GPIO[21])
#define PIN_CCNT_CFG_CCNT2_4            0x80         ///< PICNT2_4 (P_GPIO[11])

#define PIN_CCNT_CFG_CCNT3_1            0x100        ///< PICNT3_1 (P_GPIO[17])
#define PIN_CCNT_CFG_CCNT3_2            0x200        ///< PICNT3_2 (P_GPIO[20])

/**
    PIN config for Sensor

    @note For pinmux_init() with PIN_FUNC_SENSOR.
*/
#define PIN_SENSOR_CFG_NONE                    0x0

#define PIN_SENSOR_CFG_12BITS                  0x10         ///< SN_D[0..11]/SN_PXCLK/SN_VD/SN_HD (P_GPIO[0..3], HSI_GPIO[2..9]/HSI_GPIO[10]/S_GPIO[2]/S_GPIO[3])
#define PIN_SENSOR_CFG_12BITS_2                0x20

#define PIN_SENSOR_CFG_CCIR8BITS               0x100        ///< SN1_CCIR_YC[0..7]/VD/HD/PXCLK  (HSI_GPIO[2..9]/the others selected by PIN_SENSOR_CFG_CCIR_VSHS)

#define PIN_SENSOR_CFG_CCIR16BITS              0x1000       ///< SN1_CCIR-16BIT_Y[0..7]/SN1_CCIR-16BIT_C[0..7]/VD/HD/PXCLK (HSI_GPIO[2..9]/P_GPIO[0..7]/the others selected by PIN_SENSOR_CFG_CCIR_VSHS)

#define PIN_SENSOR_CFG_CCIR_VSHS               0x10000      ///< CCIR_VD/CCIR_HD/CCIR_PXCLK (HSI_GPIO[0]/HSI_GPIO[1]/HSI_GPIO[10])

#define PIN_SENSOR_CFG_SN_VSHS                 0x100000     ///< VD/HD/PXCLK (HSI_GPIO[0]/HSI_GPIO[1]/HSI_GPIO[10])
/**
    PIN config for Sensor2

    @note For pinmux_init() with PIN_FUNC_SENSOR2.
*/
#define PIN_SENSOR2_CFG_NONE                    0x0

#define PIN_SENSOR2_CFG_12BITS                  0x10         ///< SN2_D[0..11]/PXCLK/VD/HD (P_GPIO[0..7], HSI_GPIO[6..9]/P_GPIO[8]/P_GPIO[9]/P_GPIO[10])

#define PIN_SENSOR2_CFG_CCIR8BITS               0x100        ///< SN2_CCIR_YC[0..7]/VD/HD/PXCLK  (HSI_GPIO[2..9]/P_GPIO[8]/P_GPIO[9]/P_GPIO[10])

#define PIN_SENSOR2_CFG_SN2_VSHS                0x100000     ///< VD/HD/PXCLK (P_GPIO[9]/P_GPIO[10]/P_GPIO[8])
/**
    PIN config for Sensor Misc

    @note For pinmux_init() with PIN_FUNC_SENSORMISC.
*/
#define PIN_SENSORMISC_CFG_NONE                 0x0

#define PIN_SENSORMISC_CFG_SN_MCLK_1            0x1           ///< SN1_MCLK_1 (S_GPIO[0])
#define PIN_SENSORMISC_CFG_SN_MCLK_2            0x2           ///< SN1_MCLK_2 (HSI_GPIO[11])
#define PIN_SENSORMISC_CFG_SN2_MCLK_1           0x10           ///< SN2_MCLK_1 (S_GPIO[1])
#define PIN_SENSORMISC_CFG_SN3_MCLK_1           0x100          ///< SN3_MCLK_1 (P_GPIO[12])


#define PIN_SENSORMISC_CFG_SN_XVSXHS_1          0x1000         ///< SN1_XVSXHS_1 (S_GPIO[2..3])
#define PIN_SENSORMISC_CFG_SN_XVSXHS_2          0x2000         ///< SN1_XVSXHS_2 (HSI_GPIO[7..8])
#define PIN_SENSORMISC_CFG_SN2_XVSXHS_1         0x4000        /// < SN2_XVSXHS_1 (S_GPIO[4..5])

#define PIN_SENSORMISC_CFG_FLASH_TRIGA_IN_1     0x100000      ///< FLASH_TRIG_IN_1 (C_GPIO[21..22])
#define PIN_SENSORMISC_CFG_FLASH_TRIGA_IN_2     0x200000      ///< FLASH_TRIG_IN_2 (P_GPIO[11..12])


#define PIN_SENSORMISC_CFG_ME_SHUT_IN_1         0x1000000     ///< SHUT_IN_1 (C_GPIO[20])
#define PIN_SENSORMISC_CFG_ME_SHUT_IN_2         0x2000000     ///< SHUT_IN_1 (P_GPIO[4])

#define PIN_SENSORMISC_CFG_ME_SHUT_OUT_1      0x4000000     ///< SHUT_OUT_1 (C_GPIO[23..24])
#define PIN_SENSORMISC_CFG_ME_SHUT_OUT_2      0x8000000     ///< SHUT_OUT_2 (P_GPIO[5..6])

/**
    PIN config for Sensor Sync

    @note For pinmux_init() with PIN_FUNC_SENSORSYNC.
*/
#define PIN_SENSORSYNC_CFG_NONE             0x0

#define PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SIEMCLK2 0x100       //SIEMCLK2(from CG)
#define PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SIEMCLK	0x200       //SIEMCLK (from CG)

#define PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN2   0x1000
#define PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN1   0x2000


/**
    PIN config for AUDIO

    @note For pinmux_init() with PIN_FUNC_AUDIO.
*/
#define PIN_AUDIO_CFG_NONE          0x0
#define PIN_AUDIO_CFG_I2S_1         0x1         ///< I2S_1       (PGPIO[9..12])
#define PIN_AUDIO_CFG_I2S_2         0x2         ///< I2S_2       (CGPIO[14..17])
#define PIN_AUDIO_CFG_I2S_3         0x4         ///< I2S_3       (PGPIO[26..29])

#define PIN_AUDIO_CFG_I2S_MCLK_1    0x10         ///< I2S_MCLK_1  (PGPIO[8])
#define PIN_AUDIO_CFG_I2S_MCLK_2    0x20         ///< I2S_MCLK_2  (CGPIO[13])
#define PIN_AUDIO_CFG_I2S_MCLK_3    0x40         ///< I2S_MCLK_3  (PGPIO[25])

#define PIN_AUDIO_CFG_DMIC_1        0x100      ///< DMIC_1 DM_CLK (PGPIO[19]) DATA0 (PGPIO[17]) DATA1 (PGPIO[18])
#define PIN_AUDIO_CFG_DMIC_2        0x200      ///< DMIC_2 DM_CLK (PGPIO[10]) DATA0 (PGPIO[9]) DATA1 (PGPIO[8])
#define PIN_AUDIO_CFG_DMIC_3        0x400      ///< DMIC_3 DM_CLK (CGPIO[13]) DATA0 (CGPIO[14]) DATA1 (CGPIO[15])
#define PIN_AUDIO_CFG_DMIC_4        0x800      ///< DMIC_4 DM_CLK (CGPIO[22]) DATA0 (CGPIO[23]) DATA1 (CGPIO[24])
#define PIN_AUDIO_CFG_PR_DMIC_1     0x1000     ///< PR_DMIC_1 DM_CLK (SGPIO[6]) DATA0 (SGPIO[5]) DATA1 (SGPIO[4])

#define PIN_AUDIO_CFG_DMIC_DATA0    0x10000
#define PIN_AUDIO_CFG_DMIC_DATA1    0x20000

#define PIN_AUDIO_CFG_EXT_EAC_MCLK  0x100000     ///< EXT_EAC_MCLK (CGPIO[19])

/**
    PIN config for UART

    @note For pinmux_init() with PIN_FUNC_UART.
*/
#define PIN_UART_CFG_NONE               0x0

#define PIN_UART_CFG_UART_1             0x1          ///< UART_1  (P_GPIO[23..24])

#define PIN_UART_CFG_UART2_1            0x10         ///< UART2_1 (P_GPIO[17..18]) RTSCTS (P_GPIO[19..20]) DIROE (P_GPIO[19])
#define PIN_UART_CFG_UART2_2            0x20         ///< UART2_2 (C_GPIO[17..18]) RTSCTS (C_GPIO[15..16]) DIROE (C_GPIO[16])
#define PIN_UART_CFG_UART2_3            0x40         ///< UART2_3 (P_GPIO[0..1]) RTSCTS (P_GPIO[2..3]) DIROE (P_GPIO[2])
#define PIN_UART_CFG_UART2_4            0x80         ///< UART2_4 (S_GPIO[9..10]) RTSCTS (S_GPIO[11..12]) DIROE (S_GPIO[11])

#define PIN_UART_CFG_UART3_1            0x100        ///< UART3_1 (P_GPIO[8..9]) RTSCTS (P_GPIO[10..11]) DIROE (P_GPIO[10])
#define PIN_UART_CFG_UART3_2            0x200        ///< UART3_2 (C_GPIO[23..24]) RTSCTS (C_GPIO[21..22]) DIROE (C_GPIO[22])
#define PIN_UART_CFG_UART3_3            0x400        ///< UART3_3 (P_GPIO[4..5]) RTSCTS (P_GPIO[6..7]) DIROE (P_GPIO[6])
#define PIN_UART_CFG_UART3_4            0x800        ///< UART3_4 (S_GPIO[0..1])
#define PIN_UART_CFG_UART3_5            0x1000       ///< UART3_5 (S_GPIO[4..5])
#define PIN_UART_CFG_UART3_6            0x2000       ///< UART3_6 (S_GPIO[13..14]) RTSCTS (S_GPIO[15..16]) DIROE (S_GPIO[15])

#define PIN_UART_CFG_UART2_RTSCTS       0x100000
#define PIN_UART_CFG_UART2_DIROE        0x200000

#define PIN_UART_CFG_UART3_RTSCTS       0x400000
#define PIN_UART_CFG_UART3_DIROE        0x800000

/**
    PIN config for CSI

    @note For pinmux_init() with PIN_FUNC_CSI.
*/
#define PIN_CSI_CFG_NONE             0x0

#define PIN_CSI_CFG_CSI_1C4D          0x1          ///< CSI_1C4D         (HSI_GPIO[0..9])
#define PIN_CSI_CFG_CSICSI2_1C2D      0x2          ///< CSICSI2_1C2D     (HSI_GPIO[0..11])
#define PIN_CSI_CFG_CSI_1C2D          0x4          ///< CSI_1C2D         (HSI_GPIO[0..5])
#define PIN_CSI_CFG_CSI2_1C2D         0x8          ///< CSI2_1C2D        (HSI_GPIO[6..11])

/**
    PIN config for REMOTE

    @note For pinmux_init() with PIN_FUNC_REMOTE.
*/
#define PIN_REMOTE_CFG_NONE             0x0

#define PIN_REMOTE_CFG_REMOTE_1         0x1          ///< REMOTE_1     (P_GPIO[17])
#define PIN_REMOTE_CFG_REMOTE_2         0x2          ///< REMOTE_2     (C_GPIO[16])

#define PIN_REMOTE_CFG_REMOTE_EXT_1     0x10          ///< REMOTE_EXT_1 (P_GPIO[18])
#define PIN_REMOTE_CFG_REMOTE_EXT_2     0x20          ///< REMOTE_EXT_2 (C_GPIO[17])

/**
    PIN config for SDP

    @note For pinmux_init() with PIN_FUNC_SDP.
*/
#define PIN_SDP_CFG_NONE                0x0

#define PIN_SDP_CFG_SDP_1               0x1          ///< SDP_1 (C_GPIO[13..17])
#define PIN_SDP_CFG_SDP_2               0x2          ///< SDP_2 (P_GPIO[4..8])
#define PIN_SDP_CFG_SDP_3               0x4          ///< SDP_3 (P_GPIO[28..32])

/**
    PIN config for SPI

    @note For pinmux_init() with PIN_FUNC_SPI.
*/
#define PIN_SPI_CFG_NONE                0x0

#define PIN_SPI_CFG_SPI_1               0x1          ///< SPI_1  (C_GPIO[19..21])  BUS_WIDTH (C_GPIO[22])
#define PIN_SPI_CFG_SPI_2               0x2          ///< SPI1_2 (S_GPIO[6..8])    BUS_WIDTH (S_GPIO[5])
#define PIN_SPI_CFG_SPI_BUS_WIDTH       0x8

#define PIN_SPI_CFG_SPI2_1              0x10         ///< SPI2_1 (P_GPIO[9..11])   BUS_WIDTH (P_GPIO[12])
#define PIN_SPI_CFG_SPI2_2              0x20         ///< SPI2_2 (P_GPIO[0..2])    BUS_WIDTH (P_GPIO[3])
#define PIN_SPI_CFG_SPI2_3              0x40         ///< SPI2_3 (S_GPIO[9..11])    BUS_WIDTH (S_GPIO[12])
#define PIN_SPI_CFG_SPI2_4              0x80         ///< SPI2_4 (S_GPIO[13..15])  BUS_WIDTH (S_GPIO[16])
#define PIN_SPI_CFG_SPI2_BUS_WIDTH      0x100

#define PIN_SPI_CFG_SPI3_1              0x1000        ///< SPI3_1 (P_GPIO[17..19])  BUS_WIDTH (P_GPIO[20])
#define PIN_SPI_CFG_SPI3_2              0x2000        ///< SPI3_2 (C_GPIO[13..15])  BUS_WIDTH (C_GPIO[16])
#define PIN_SPI_CFG_SPI3_3              0x4000        ///< SPI3_3 (P_GPIO[4..6])    BUS_WIDTH (P_GPIO[7])
#define PIN_SPI_CFG_SPI3_4              0x8000        ///< SPI3_4 (P_GPIO[25..27])  BUS_WIDTH (P_GPIO[28])
#define PIN_SPI_CFG_SPI3_BUS_WIDTH      0x10000

#define PIN_SPI_CFG_SPI3_RDY_1          0x100000     ///< SPI3RDY_1 (P_GPIO[21])
#define PIN_SPI_CFG_SPI3_RDY_2          0x200000     ///< SPI3RDY_2 (C_GPIO[17])
#define PIN_SPI_CFG_SPI3_RDY_3          0x400000     ///< SPI3RDY_3 (P_GPIO[8])
#define PIN_SPI_CFG_SPI3_RDY_4          0x800000     ///< SPI3RDY_4 (P_GPIO[29])

/**
    PIN config for SIF

    *note For pinmux_init() with PIN_FUNC_SIF.
*/
#define PIN_SIF_CFG_NONE                0x0

#define PIN_SIF_CFG_SIF0_1              0x1          ///< SIF0_1 (S_GPIO[6..8])

#define PIN_SIF_CFG_SIF1_1              0x10         ///< SIF1_1 (S_GPIO[3..5])

#define PIN_SIF_CFG_SIF2_1              0x100        ///< SIF2_1 (P_GPIO[10..12])
#define PIN_SIF_CFG_SIF2_2              0x200        ///< SIF2_2 (P_GPIO[17..19])
#define PIN_SIF_CFG_SIF2_3              0x400        ///< SIF2_3 (D_GPIO[3..5])
#define PIN_SIF_CFG_SIF2_4              0x800        ///< SIF2_4 (P_GPIO[25..27])

#define PIN_SIF_CFG_SIF3_1              0x1000       ///< SIF3_1 (P_GPIO[7..9])
#define PIN_SIF_CFG_SIF3_2              0x2000       ///< SIF3_2 (S_GPIO[9..11])

/**
    PIN config for MISC

    @note For pinmux_init() for PIN_FUNC_MISC.
*/
#define PIN_MISC_CFG_NONE                   0x0

#define PIN_MISC_CFG_RTC_CLK_1              0x1          ///< RTC_CLK_1     (P_GPIO[19])

#define PIN_MISC_CFG_SP_CLK_1               0x10         ///< SP_CLK_1  (P_GPIO[20])
#define PIN_MISC_CFG_SP_CLK_2               0x20         ///< SP_CLK_2  (C_GPIO[24])
#define PIN_MISC_CFG_SP_CLK_3               0x40         ///< SP_CLK_3  (D_GPIO[3])
#define PIN_MISC_CFG_SP_CLK_4               0x80         ///< SP_CLK_4  (C_GPIO[22])
#define PIN_MISC_CFG_SP_CLK_5               0x100        ///< SP_CLK_5  (P_GPIO[12])



#define PIN_MISC_CFG_SP2_CLK_1              0x1000        ///< SP2_CLK_1 (P_GPIO[21])
#define PIN_MISC_CFG_SP2_CLK_2              0x2000        ///< SP2_CLK_2 (C_GPIO[11])
#define PIN_MISC_CFG_SP2_CLK_3              0x4000        ///< SP2_CLK_3 (D_GPIO[4])
#define PIN_MISC_CFG_SP2_CLK_4              0x8000        ///< SP2_CLK_4 (P_GPIO[30])

#define PIN_MISC_CFG_CPU_ICE                0x10000000   ///< CPU_ICE   (D_GPIO[2..6])
/*
#define PIN_MISC_CFG_SATA_LED_1             0x1000       ///< SATA_LED_1 (D_GPIO[4])
#define PIN_MISC_CFG_SATA2_LED_1            0x2000       ///< SATA2_LED_1	(D_GPIO[4])

#define PIN_MISC_CFG_ADC                    0x100000     ///< ADC (A_GPIO[0..3])

#define PIN_MISC_CFG_MIPI_MODE_SEL_1C4D     0x200000     ///< MIPI(DSI_GPIO[0..9])
#define PIN_MISC_CFG_MIPI_MODE_SEL_1C2D     0x400000     ///< MIPI(DSI_GPIO[0..11])


#define PIN_MISC_CFG_MIPI_CK0_SEL_DSI       0x1000000
#define PIN_MISC_CFG_MIPI_CK0_SEL_CSI_TX    0x2000000

#define PIN_MISC_CFG_MIPI_CK1_SEL_DSI       0x4000000
#define PIN_MISC_CFG_MIPI_CK1_SEL_CSI_TX    0x8000000

#define PIN_MISC_CFG_CPU_ICE                0x10000000   ///< CPU_ICE	(D_GPIO[10..14])
*/

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
                                                                ///< When PIN_FUNC_SEL_LCD  CCIR_YC[0..7]/CCIR_CLK (L_GPIO[0..8])
                                                                ///< When PIN_FUNC_SEL_LCD2  CCIR_YC[0..7]/CCIR_CLK (L_GPIO[13..21])

#define PINMUX_LCD_SEL_CCIR656_16BITS           2               ///< CCIR-656 16 bits. CCIR_Y[0..7]/CCIR_CLK/CCIR_C[0..7] (L_GPIO[0..8] L_GPIO[9..16])

#define PINMUX_LCD_SEL_CCIR601                  3               ///< CCIR-601 8 bits.
                                                                ///< When PIN_FUNC_SEL_LCD  CCIR_YC[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD (L_GPIO[0..10])
                                                                ///< When PIN_FUNC_SEL_LCD2, CCIR_YC[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD/CCIR_FIELD (L_GPIO[13..23] L_GPIO[25])

#define PINMUX_LCD_SEL_CCIR601_16BITS           4               ///< CCIR-601 16 bits. CCIR_Y[0..7]/CCIR_CLK/CCIR_C[0..7]/CCIR_VD/CCIR_HD (L_GPIO[0..7] L_GPIO[8] L_GPIO[9..16] L_GPIO[17..18])

#define PINMUX_LCD_SEL_SERIAL_RGB_6BITS         5               ///< Serial RGB 6 bits.
                                                                ///< When PIN_FUNC_SEL_LCD  RGB_D[2..7]/RGB_CLK/RGB_VD/RGB_HD (L_GPIO[2..10])
                                                                ///< When PIN_FUNC_SEL_LCD2  RGB_D[2..7]/RGB_CLK/RGB_VD/RGB_HD (L_GPIO[15..23])

#define PINMUX_LCD_SEL_SERIAL_RGB_8BITS         6               ///< Serial RGB 8 bits.
                                                                ///< When PIN_FUNC_SEL_LCD  RGB_D[0..7]/RGB_CLK/RGB_VD/RGB_HD (L_GPIO[0..10])
                                                                ///< When PIN_FUNC_SEL_LCD2  RGB_D[0..7]/RGB_CLK/RGB_VD/RGB_HD (L_GPIO[13..23])

#define PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS       7               ///< Serial YCbCr 8 bits.
                                                                ///< When PIN_FUNC_SEL_LCD  CCIR_YC[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD (L_GPIO[0..10])
                                                                ///< When PIN_FUNC_SEL_LCD2  CCIR_YC[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD (L_GPIO[13..23])

#define PINMUX_LCD_SEL_PARALLE_RGB565           8               ///< Parallel RGB565. RGB_C0_[0..4]/RGB_DCLK/RGB_VS/RGB_HS/RGB_C1_[0..5]/RGB_C2_[0..4] (L_GPIO[0..8] L_GPIO[11..18])

#define PINMUX_LCD_SEL_PARALLE_RGB666           9               ///< Parallel RGB666. RGB_C0_[0..5]/RGB_DCLK/RGB_VS/RGB_HS/RGB_C1_[0..5]/RGB_C2_[0..5] (L_GPIO[2..8] L_GPIO[11..18] L_GPIO[22..27])

#define PINMUX_LCD_SEL_PARALLE_RGB888           10              ///< Parallel RGB888. RGB_C0_[0..7]/RGB_DCLK/RGB_VS/RGB_HS/RGB_C1_[0..7]/RGB_C2_[0..7] (L_GPIO[0..18] L_GPIO[20..27])

#define PINMUX_LCD_SEL_RGB_16BITS               11              ///< RGB 16 bits. CCIR_Y[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD/CCIR_C[0..7] (L_GPIO[0..18])

#define PINMUX_LCD_SEL_MIPI                     12              ///< MIPI DSI

#define PINMUX_LCD_SEL_PARALLE_MI_8BITS         13              ///< Parallel MI 8 bits.
                                                                ///< When PIN_FUNC_SEL_LCD  MPU_D[0..7]/MPU_CS/MPU_RS/MPU_WR/MPU_RD (L_GPIO[0..7] L_GPIO[8..11])
                                                                ///< When PIN_FUNC_SEL_LCD2  MPU2_D[0..7]/MPU2_RS/MPU2_CS/MPU2_WR/MPU2_RD (L_GPIO[12..19] L_GPIO[21..24])

#define PINMUX_LCD_SEL_PARALLE_MI_9BITS         14              ///< Parallel MI 9 bits.
                                                                ///< When PIN_FUNC_SEL_LCD  MPU_D[0..8]/MPU_CS/MPU_RS/MPU_WR/MPU_RD (L_GPIO[0..7][12] L_GPIO[8..11])
                                                                ///< When PIN_FUNC_SEL_LCD2  MPU2_D[0..8]/MPU2_RS/MPU2_CS/MPU2_WR/MPU2_RD (L_GPIO[12..20] L_GPIO[21..24])

#define PINMUX_LCD_SEL_PARALLE_MI_16BITS        15              ///< Parallel MI 16 bits. MPU_D[0..15]/MPU_CS/MPU_RS/MPU_WR/MPU_RD (L_GPIO[0..7][12..19] L_GPIO[8..11])

#define PINMUX_LCD_SEL_PARALLE_MI_18BITS        16              ///< Parallel MI 18 bits. MPU_D[0..17]/MPU_CS/MPU_RS/MPU_WR/MPU_RD (L_GPIO[0..7][12..21] L_GPIO[8..11])

#define PINMUX_LCD_SEL_SERIAL_MI_SDIO           17              ///< Serial MI SDIO bi-direction.
                                                                ///< When PIN_FUNC_SEL_LCD  MPU_CLK/MPU_RS/MPU_CS/MPU_SDIO (L_GPIO[8..10] L_GPIO[7])
                                                                ///< When PIN_FUNC_SEL_LCD2  MPU_CLK/MPU_RS/MPU_CS/MPU_SDIO (L_GPIO[15..17] L_GPIO[14])

#define PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO        18              ///< Serial MI SDI/SDO seperate.
                                                                ///< When PIN_FUNC_SEL_LCD  MPU_CLK/MPU_RS/MPU_CS/MPU_SDO/MPU_SDI (L_GPIO[8..10] L_GPIO[5..6])
                                                                ///< When PIN_FUNC_SEL_LCD2  MPU_CLK/MPU_RS/MPU_CS/MPU_SDO/MPU_SDI (L_GPIO[15..17] L_GPIO[12..13])

#define PINMUX_LCD_SEL_TE_ENABLE                (0x01 << 23)    ///< TE Enable.
                                                                ///< When PIN_FUNC_SEL_LCD  DSI_TE (DSI_GPIO[12])
                                                                ///< When PIN_FUNC_SEL_LCD2  DSI_TE (DSI_GPIO[13])
                                                                ///< When PIN_FUNC_SEL_LCD and PARALLE_MI  MPU_TE (L_GPIO[11])
                                                                ///< When PIN_FUNC_SEL_LCD2 and PARALLE_MI  MPU_TE (L_GPIO[18])
                                                                ///< When PIN_FUNC_SEL_LCD and SERIAL_MI  MI_TE (L_GPIO[22])
                                                                ///< When PIN_FUNC_SEL_LCD2 and SERIAL_MI  MI_TE (L_GPIO[25])

#define PINMUX_LCD_SEL_TE2_ENABLE               (0x01 << 28)    ///< TE2 Enable.


#define PINMUX_LCD_SEL_DE_ENABLE                (0x01 << 24)    ///< DE Enable.
                                                                ///< When PIN_FUNC_SEL_LCD  CCIR_DE  LCD_TYPE = 0x1~0x2(When CCIR_DATA_WIDTH = 1) LCD_TYPE = 0x3 and 0x7~0x8 and 0xA (L_GPIO[19])LCD_TYPE = 0x1~0x2(When CCIR_DATA_WIDTH = 0)LCD_TYPE = 0x4~0x6 (L_GPIO[11])
                                                                ///< When PIN_FUNC_SEL_LCD2  CCIR2_DE (L_GPIO[24])

#define PINMUX_LCD_SEL_HVLD_VVLD                (0x01 << 25)    ///< HVLD/VVLD Enable (For CCIR-601 8 bits). CCIR_HVLD/CCIR_VVLD (L_GPIO[12..13])

#define PINMUX_LCD_SEL_FIELD                    (0x01 << 26)    ///< FIELD Enable (For CCIR-601). CCIR_FIELD: CCIR-601 8 bits(L_GPIO[14])  CCIR-601 16 bits(L_GPIO[20])

#define PINMUX_LCD_SEL_NO_HVSYNC                (0x1 << 27)     ///< No HSYNC/VSYNC (backward compatible)

#define PINMUX_LCD_SEL_FEATURE_MSK              (0x3F << 23)


/********************** Pre-roll part ENUM **********************/
/**
    PIN config for PR_I2C

    @note For pinmux_init() with PIN_FUNC_PR_I2C.
*/
#define PIN_PR_I2C_CFG_NONE                0x0

#define PIN_PR_I2C_CFG_I2C_1               0x1         ///< PR_I2C_1      (S_GPIO[7..8])
#define PIN_PR_I2C_CFG_I2C_2               0x2         ///< PR_I2C_2      (P_GPIO[29..30])

/**
    PIN config for PR_UART

    @note For pinmux_init() with PIN_FUNC_PR_UART.
*/
#define PIN_PR_UART_CFG_NONE               0x0

#define PIN_PR_UART_CFG_UART_1             0x1          ///< PR_UART_1  (S_GPIO[2..3])
#define PIN_PR_UART_CFG_UART_2             0x2          ///< PR_UART_1  (S_GPIO[15..16])

#define PIN_PR_UART_CFG_UART2_1            0x10         ///< PR_UART2_1 (P_GPIO[31..32])
#define PIN_PR_UART_CFG_UART2_2            0x20         ///< PR_UART2_2 (S_GPIO[9..10])

/**
    PIN config for PR_Sensor

    @note For pinmux_init() with PIN_FUNC_PR_SENSOR.
*/
#define PIN_PR_SENSOR_CFG_NONE                    0x0

#define PIN_PR_SENSOR_CFG_SVT_2BIT                0x10         ///< PR_SN_D[0..1]/PR_SPI_SCK/PR_SPI_VS/PR_SPI_HS/ (PR_SN_PXCLK/PR_SN_VS/PR_SN_HS)
#define PIN_PR_SENSOR_CFG_SVT_4BIT                0x20         ///< PR_SN_D[0..3]/PR_SPI_SCK/PR_SPI_VS/PR_SPI_HS/ (S_GPIO[11..14], S_GPIO[10], S_GPIO[2], S_GPIO[3])

#define PIN_PR_SENSOR_CFG_DVP               	  0x100        ///< PR_SN_D[0..7]/PR_SN_PXCLK/PR_SN_VS/PR_SN_HS (S_GPIO[9..16], S_GPIO[1], S_GPIO[2], S_GPIO[3])

#define PIN_PR_SENSOR_CFG_AO_1BIT                 0x1000       ///< PR_SN_D[0]/PR_SPI_CSK (S_GPIO[11], S_GPIO[10])
#define PIN_PR_SENSOR_CFG_AO_2BIT                 0x2000       ///< PR_SN_D[1..0]/PR_SPI_CSK (S_GPIO[11..12], S_GPIO[10])
#define PIN_PR_SENSOR_CFG_AO_4BIT                 0x4000       ///< PR_SN_D[3..0]/PR_SPI_CSK (S_GPIO[11..14], S_GPIO[10])

#define PIN_PR_SENSOR_CFG_SPI_PIXART_2BIT         0x10000      ///< PR_SN_D[1..0]/PR_SPI_SCK/PR_SPI_CSN (S_GPIO[11..12], S_GPIO[10], S_GPIO[9])
#define PIN_PR_SENSOR_CFG_SPI_PIXART_4BIT         0x20000      ///< PR_SN_D[3..0]/PR_SPI_SCK/PR_SPI_CSN (S_GPIO[11..14], S_GPIO[10], S_GPIO[9])

/**
    PIN config for PR_Sensor Misc

    @note For pinmux_init() with PIN_FUNC_PR_SENSORMISC.
*/
#define PIN_PR_SENSORMISC_CFG_NONE                0x0

#define PIN_PR_SENSORMISC_CFG_SN_MCLK_1           0x1            ///< PR_SN1_MCLK_1 (S_GPIO[0])

#define PIN_PR_SENSORMISC_CFG_SRAM_1              0x1000         ///< CE/CLK/SIO (P_GPIO[33], P_GPIO[34], SIO is selected by PIN_PR_SENSORMISC_CFG_SRAM_SIO_(1,2,4)BIT)
#define PIN_PR_SENSORMISC_CFG_SRAM_2              0x2000         ///< CE/CLK/SIO (P_GPIO[25], P_GPIO[26], SIO is selected by PIN_PR_SENSORMISC_CFG_SRAM_SIO_(1,2,4)BIT)

#define PIN_PR_SENSORMISC_CFG_SRAM_SIO_1BIT       0x10000        /// SRAM_1/SRAM_2 (P_GPIO[35], P_GPIO[27])
#define PIN_PR_SENSORMISC_CFG_SRAM_SIO_2BIT       0x20000        /// SRAM_1/SRAM_2 (P_GPIO[35..36], P_GPIO[27..28])
#define PIN_PR_SENSORMISC_CFG_SRAM_SIO_4BIT       0x40000        /// SRAM_1/SRAM_2 (P_GPIO[35..38], P_GPIO[27..30])

/**
    PIN config for PR_SPI

    @note For pinmux_init() with PIN_FUNC_PR_SPI.
*/
#define PIN_PR_SPI_CFG_NONE                0x0

#define PIN_PR_SPI_CFG_SPI_WG_1            0x1          ///< PR_SPI_WG_1  (P_GPIO[25..26])
#define PIN_PR_SPI_CFG_SPI_WG_2            0x2          ///< PR_SPI_WG_2  (S_GPIO[11..12])

/**
    PIN config for PR_PWM

    @note For pinmux_init() with PIN_FUNC_PR_PWM.
*/
#define PIN_PR_PWM_CFG_NONE                0x0

#define PIN_PR_PWM_CFG_PWM0_1              0x1          ///< PR_PWM0_1  (P_GPIO[27])
#define PIN_PR_PWM_CFG_PWM0_2              0x2          ///< PR_PWM0_2  (S_GPIO[13])


#define PIN_PR_PWM_CFG_PWM1_1              0x100         ///< PR_PWM1_1  (P_GPIO[28])
#define PIN_PR_PWM_CFG_PWM1_2              0x200         ///< PR_PWM1_2  (S_GPIO[14])

/**
    PIN config for PR_AUDIO

    @note For pinmux_init() with PIN_FUNC_PR_AUDIO.
*/
#define PIN_PR_AUDIO_CFG_NONE          0x0

#define PIN_PR_AUDIO_CFG_DMIC_1        0x1     ///< PR_DMIC_1 DM_CLK (SGPIO[6]) DATA0 (SGPIO[5]) DATA1 (SGPIO[4])


#endif
