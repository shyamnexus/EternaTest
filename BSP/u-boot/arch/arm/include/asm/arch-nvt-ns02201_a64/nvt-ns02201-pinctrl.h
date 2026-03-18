/* SPDX-License-Identifier: GPL-2.0 */
/*
 * This header provides constants specific to Novatek NS02201 pinctrl bindings.
 */

#ifndef __DT_BINDINGS_PINCTRL_NS02201_PINCTRL_H
#define __DT_BINDINGS_PINCTRL_NS02201_PINCTRL_H

/**
    Function group

    @note For pinmux_init()
*/
#define PIN_FUNC_SDIO                   0
#define PIN_FUNC_NAND                   1
#define PIN_FUNC_ETH                    2
#define PIN_FUNC_I2C                    3
#define PIN_FUNC_I2CII                  4
#define PIN_FUNC_I2CIII                 5
#define PIN_FUNC_I2CIV                  6
#define PIN_FUNC_PWM                    7
#define PIN_FUNC_PWMII                  8
#define PIN_FUNC_CCNT                   9
#define PIN_FUNC_SENSOR                 10
#define PIN_FUNC_SENSOR2                11
#define PIN_FUNC_SENSORMISC             12
#define PIN_FUNC_SENSORMISCII           13
#define PIN_FUNC_SENSORSYNC             14
#define PIN_FUNC_SENSORSYNCII           15
#define PIN_FUNC_SENSORSYNCIII          16
#define PIN_FUNC_SENSORSYNCIV           17
#define PIN_FUNC_AUDIO                  18
#define PIN_FUNC_UART                   19
#define PIN_FUNC_UARTII                 20
#define PIN_FUNC_REMOTE                 21
#define PIN_FUNC_SDP                    22
#define PIN_FUNC_SPI                    23
#define PIN_FUNC_SIF                    24
#define PIN_FUNC_MISC                   25
#define PIN_FUNC_LCD                    26
#define PIN_FUNC_LCD2                   27
#define PIN_FUNC_LCD3                   28
#define PIN_FUNC_TV                     29
#define PIN_FUNC_LCD_PINOUT_SEL         30
#define PIN_FUNC_SEL_LCD                31
#define PIN_FUNC_SEL_LCD2               32
#define PIN_FUNC_PCIE                   33
#define PIN_FUNC_SEL_PCIE		34
#define PIN_FUNC_MAX                    35

/**
    PIN config for SDIO

    @note For pinmux_init() with PIN_FUNC_SDIO.
*/
#define PIN_SDIO_CFG_NONE               0x0

#define PIN_SDIO_CFG_SDIO_1             0x1         ///< SDIO  (C_GPIO[13..18])

#define PIN_SDIO_CFG_SDIO2_1            0x10        ///< SDIO2 (C_GPIO[19..24])

#define PIN_SDIO_CFG_SDIO3_1            0x100       ///< SDIO3 (C_GPIO[0..3] C_GPIO[8] C_GPIO[10])
#define PIN_SDIO_CFG_SDIO3_BUS_WIDTH    0x200       ///< SDIO3 bus width 8 bits (C_GPIO[4..7])
#define PIN_SDIO_CFG_SDIO3_DS           0x400       ///< SDIO3 data strobe      (C_GPIO[11])

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

#define PIN_NAND_CFG_NAND_1             0x1          ///< FSPI     (C_GPIO[0..3]  C_GPIO[8..9])
#define PIN_NAND_CFG_NAND_CS1           0x2          ///< FSPI CS1 (C_GPIO[10])

/**
    PIN config for ETH

    @note For pinmux_init() for PIN_FUNC_ETH
*/
#define PIN_ETH_CFG_NONE                0x0

#define PIN_ETH_CFG_ETH_RGMII_1         0x001       ///< ETH_RGMII_1		(L_GPIO[19..30]  D_GPIO[6..7])
#define PIN_ETH_CFG_ETH_RGMII_2         0x002       ///< ETH_RGMII_2		(S_GPIO[16..29])
#define PIN_ETH_CFG_ETH_RMII_1          0x010       ///< ETH_RMII_1			(L_GPIO[19..21]  L_GPIO[25..28]  D_GPIO[6..7])
#define PIN_ETH_CFG_ETH_RMII_2          0x020       ///< ETH_RMII_2			(S_GPIO[16..18]  L_GPIO[22..25]  S_GPIO[28..29])
#define PIN_ETH_CFG_ETH_EXTPHYCLK       0x100       ///< ETH_EXTPHYCLK	RGMII_1||RMII_1(D_GPIO[8]) RGMII_2||RMII_2(S_GPIO[30])
#define PIN_ETH_CFG_ETH_PTP             0x200       ///< ETH_PTP			(P_GPIO[28])

#define PIN_ETH_CFG_ETH2_RGMII_1        0x001000    ///< ETH2_RGMII_1       (L_GPIO[3..16])
#define PIN_ETH_CFG_ETH2_RGMII_2        0x002000    ///< ETH2_RGMII_2       (S_GPIO[0..13])
#define PIN_ETH_CFG_ETH2_RMII_1         0x010000    ///< ETH2_RMII_1        (L_GPIO[3..5]  L_GPIO[9..12] L_GPIO[15..16])
#define PIN_ETH_CFG_ETH2_RMII_2         0x020000    ///< ETH2_RMII_2        (S_GPIO[0..2]  S_GPIO[6..9] S_GPIO[12..13])
#define PIN_ETH_CFG_ETH2_EXTPHYCLK      0x100000    ///< ETH2_EXTPHYCLK	RGMII_1||RMII_1(L_GPIO[17]) RGMII_2||RMII_2(S_GPIO[14])
#define PIN_ETH_CFG_ETH2_PTP            0x200000    ///< ETH2_PTP           (P_GPIO[24])

/**
    PIN config for I2C

    @note For pinmux_init() with PIN_FUNC_I2C.
*/
#define PIN_I2C_CFG_NONE                0x0

#define PIN_I2C_CFG_I2C_1               0x1         ///< I2C_1      (S_GPIO[12..13])
#define PIN_I2C_CFG_I2C_2               0x2         ///< I2C_2      (P_GPIO[28..29])

#define PIN_I2C_CFG_I2C2_1              0x10        ///< I2C2_1     (S_GPIO[14..15])
#define PIN_I2C_CFG_I2C2_2              0x20        ///< I2C2_2     (P_GPIO[30..31])

#define PIN_I2C_CFG_I2C3_1              0x100       ///< I2C3_1     (S_GPIO[28..29])
#define PIN_I2C_CFG_I2C3_2              0x200       ///< I2C3_2     (P_GPIO[12..13])

#define PIN_I2C_CFG_I2C4_1              0x1000      ///< I2C4_1     (S_GPIO[30..31])
#define PIN_I2C_CFG_I2C4_2              0x2000      ///< I2C4_2     (P_GPIO[14..15])
#define PIN_I2C_CFG_I2C4_3              0x4000      ///< I2C4_3     (DSI_GPIO[4..5])

#define PIN_I2C_CFG_I2C5_1              0x10000     ///< I2C5_1     (S_GPIO[16..17])
#define PIN_I2C_CFG_I2C5_2              0x20000     ///< I2C5_2     (P_GPIO[16..17])
#define PIN_I2C_CFG_I2C5_3              0x40000     ///< I2C5_3     (C_GPIO[4..5])

/**
    PIN config for I2CII

    @note For pinmux_init() with PIN_FUNC_I2CII.
*/
#define PIN_I2CII_CFG_NONE              0x0

#define PIN_I2CII_CFG_I2C6_1            0x1         ///< I2C6_1  (S_GPIO[18..19])
#define PIN_I2CII_CFG_I2C6_2            0x2         ///< I2C6_2  (P_GPIO[18..19])
#define PIN_I2CII_CFG_I2C6_3            0x4         ///< I2C6_3  (C_GPIO[6..7])

#define PIN_I2CII_CFG_I2C7_1            0x10        ///< I2C7_1  (S_GPIO[20..21])
#define PIN_I2CII_CFG_I2C7_2            0x20        ///< I2C7_2  (P_GPIO[20..21])

#define PIN_I2CII_CFG_I2C8_1            0x100       ///< I2C8_1  (S_GPIO[22..23])
#define PIN_I2CII_CFG_I2C8_2            0x200       ///< I2C8_2  (P_GPIO[22..23])

#define PIN_I2CII_CFG_I2C9_1            0x1000      ///< I2C9_1  (P_GPIO[24..25])
#define PIN_I2CII_CFG_I2C9_2            0x2000      ///< I2C9_2  (L_GPIO[27..28])

#define PIN_I2CII_CFG_I2C10_1           0x10000     ///< I2C10_1 (P_GPIO[26..27])
#define PIN_I2CII_CFG_I2C10_2           0x20000     ///< I2C10_2 (D_GPIO[0..1])

#define PIN_I2CII_CFG_I2C11_1           0x100000    ///< I2C11_1 (P_GPIO[36..37])

/**
    PIN config for I2CIII

    @note For pinmux_init() with PIN_FUNC_I2CIII.
*/

#define PIN_I2CIII_CFG_NONE         0x0

#define PIN_I2CIII_CFG_I2C12_1      0x1         ///< I2C12_1	(P_GPIO[45..46])

#define PIN_I2CIII_CFG_I2C13_1      0x10        ///< I2C13_1	(P_GPIO[38..39])
#define PIN_I2CIII_CFG_I2C13_2      0x20        ///< I2C13_2	(D_GPIO[2..3])

#define PIN_I2CIII_CFG_I2C14_1      0x100       ///< I2C14_1	(P_GPIO[40..41])
#define PIN_I2CIII_CFG_I2C14_2      0x200       ///< I2C14_2	(D_GPIO[4..5])

#define PIN_I2CIII_CFG_I2C15_1      0x1000      ///< I2C15_1	(P_GPIO[0..1])
#define PIN_I2CIII_CFG_I2C15_2      0x2000      ///< I2C15_2	(DSI_GPIO[0..1])
#define PIN_I2CIII_CFG_I2C15_3      0x4000      ///< I2C15_3	(LGPIO [17..18])

#define PIN_I2CIII_CFG_I2C16_1      0x10000     ///< I2C16_1	(P_GPIO[2..3])
#define PIN_I2CIII_CFG_I2C16_2      0x20000     ///< I2C16_2	(DSI_GPIO[2..3])

#define PIN_I2CIII_CFG_I2C17_1      0x100000    ///< I2C17_1	(P_GPIO[4..5])
#define PIN_I2CIII_CFG_I2C17_2      0x200000    ///< I2C17_2	(DSI_GPIO[6..7])
#define PIN_I2CIII_CFG_I2C17_3      0x400000    ///< I2C17_3	(L_GPIO[21..22])



/**
    PIN config for I2CIV

    @note For pinmux_init() with PIN_FUNC_I2CIV.
*/

#define PIN_I2CIV_CFG_NONE          0x0

#define PIN_I2CIV_CFG_I2C18_1       0x1         ///< I2C18_1	(P_GPIO[6..7])
#define PIN_I2CIV_CFG_I2C18_2       0x2         ///< I2C18_2	(DSI_GPIO[8..9])
#define PIN_I2CIV_CFG_I2C18_3       0x4         ///< I2C18_3	(L_GPIO[23..24])

#define PIN_I2CIV_CFG_I2C19_1       0x10        ///< I2C19_1	(P_GPIO[8..9])
#define PIN_I2CIV_CFG_I2C19_2       0x20        ///< I2C19_2 	(DSI_GPIO[10..11])
#define PIN_I2CIV_CFG_I2C19_3       0x40        ///< I2C19_3	(L_GPIO[25..26])

#define PIN_I2CIV_CFG_I2C20_1       0x100       ///< I2C20_1	(P_GPIO[10..11])
#define PIN_I2CIV_CFG_I2C20_2       0x200       ///< I2C20_2	(L_GPIO[29..30])



/**
    PIN config for PWM

    @note For pinmux_init() with PIN_FUNC_PWM.
*/
#define PIN_PWM_CFG_NONE                0x0

#define PIN_PWM_CFG_PWM0_1              0x1          ///< PWM0_1  (P_GPIO[0])
#define PIN_PWM_CFG_PWM0_2              0x2          ///< PWM0_2  (DSI_GPIO[6])

#define PIN_PWM_CFG_PWM1_1              0x10         ///< PWM1_1  (P_GPIO[1])
#define PIN_PWM_CFG_PWM1_2              0x20         ///< PWM1_2  (DSI_GPIO[7])

#define PIN_PWM_CFG_PWM2_1              0x100        ///< PWM2_1  (P_GPIO[2])
#define PIN_PWM_CFG_PWM2_2              0x200        ///< PWM2_2  (DSI_GPIO[8])

#define PIN_PWM_CFG_PWM3_1              0x1000       ///< PWM3_1  (P_GPIO[3])
#define PIN_PWM_CFG_PWM3_2              0x2000       ///< PWM3_2  (DSI_GPIO[9])

#define PIN_PWM_CFG_PWM4_1              0x10000      ///< PWM4_1  (P_GPIO[4])
#define PIN_PWM_CFG_PWM4_2              0x20000      ///< PWM4_2  (D_GPIO[2])

#define PIN_PWM_CFG_PWM5_1              0x100000     ///< PWM5_1  (P_GPIO[5])
#define PIN_PWM_CFG_PWM5_2              0x200000     ///< PWM5_2  (D_GPIO[3])

/**
    PIN config for PWMII

    @note For pinmux_init() with PIN_FUNC_PWMII.
*/
#define PIN_PWMII_CFG_NONE              0x0

#define PIN_PWMII_CFG_PWM6_1            0x1          ///< PWM6_1  (P_GPIO[6])
#define PIN_PWMII_CFG_PWM6_2            0x2          ///< PWM6_2  (D_GPIO[4])

#define PIN_PWMII_CFG_PWM7_1            0x10         ///< PWM7_1  (P_GPIO[7])
#define PIN_PWMII_CFG_PWM7_2            0x20         ///< PWM7_2  (D_GPIO[5])

#define PIN_PWMII_CFG_PWM8_1            0x100        ///< PWM8_1  (P_GPIO[8])
#define PIN_PWMII_CFG_PWM8_2            0x200        ///< PWM8_2  (C_GPIO[4])
#define PIN_PWMII_CFG_PWM8_3            0x400        ///< PWM8_3  (C_GPIO[19])

#define PIN_PWMII_CFG_PWM9_1            0x1000       ///< PWM9_1  (P_GPIO[9])
#define PIN_PWMII_CFG_PWM9_2            0x2000       ///< PWM9_2  (C_GPIO[5])
#define PIN_PWMII_CFG_PWM9_3            0x4000       ///< PWM9_3  (C_GPIO[20])

#define PIN_PWMII_CFG_PWM10_1           0x10000      ///< PWM10_1 (P_GPIO[10])
#define PIN_PWMII_CFG_PWM10_2           0x20000      ///< PWM10_2 (C_GPIO[6])
#define PIN_PWMII_CFG_PWM10_3           0x40000      ///< PWM10_3 (C_GPIO[21])

#define PIN_PWMII_CFG_PWM11_1           0x100000     ///< PWM11_1 (P_GPIO[11])
#define PIN_PWMII_CFG_PWM11_2           0x200000     ///< PWM11_2 (C_GPIO[7])
#define PIN_PWMII_CFG_PWM11_3           0x400000     ///< PWM11_3 (C_GPIO[22])

#define PIN_PWMII_CFG_PWM12_1           0x1000000    ///< PWM12_1 (P_GPIO[42])

/**
    PIN config for CCNT

    @note For pinmux_init() with PIN_FUNC_CCNT.
*/
#define PIN_CCNT_CFG_NONE               0x0

#define PIN_CCNT_CFG_CCNT_1             0x1          ///< PICNT_1  (P_GPIO[21])

#define PIN_CCNT_CFG_CCNT2_1            0x10         ///< PICNT2_1 (P_GPIO[22])

#define PIN_CCNT_CFG_CCNT3_1            0x100        ///< PICNT3_1 (P_GPIO[23])

/**
    PIN config for Sensor

    @note For pinmux_init() with PIN_FUNC_SENSOR.
*/
#define PIN_SENSOR_CFG_NONE                    0x0

#define PIN_SENSOR_CFG_12BITS                  0x100         ///< SN_D[0..11]/SN_PXCLK/SN_VD/SN_HD (S_GPIO[3..7], S_GPIO[9..15]/S_GPIO[2]/S_GPIO/[0]/S_GPIO[1])

#define PIN_SENSOR_CFG_CCIR8BITS_A             0x10000       ///< CCIR601/656. CCIR_A_YC[0..7]/CCIR_A_CLK (S_GPIO[3..7], (S_GPIO[9..11]/S_GPIO[2])
#define PIN_SENSOR_CFG_CCIR8BITS_B             0x20000       ///< CCIR656. CCIR_B_YC[0..7]/CCIR_B_CLK (S_GPIO[12..19]/S_GPIO[20])
#define PIN_SENSOR_CFG_CCIR8BITS_AB            0x40000       ///< CCIR601/656 + CCIR656. CCIR_A_YC[0..7]/CCIR_A_CLK (S_GPIO[3..7], S_GPIO[9..11]/S_GPIO[2]) + CCIR_B_YC[0..7]/CCIR_B_CLK (S_GPIO[12..19]/S_GPIO[20])

#define PIN_SENSOR_CFG_CCIR16BITS              0x10          ///< CCIR601/656. CCIR_Y[0..7]/CCIR_C[0..7]/CCIR_CLK (S_GPIO[12..19]/S_GPIO[3..7], S_GPIO[9..11]/S_GPIO[2])
#define PIN_SENSOR_CFG_CCIR_VSHS               0x100000      ///< For CCIR601. CCIR_VD/CCIR_HD/CCIR_FIELD (S_GPIO[0]/S_GPIO[1]/S_GPIO[8])

#define PIN_SENSOR_CFG_SN1_MCLK_1              0x200         ///< Enable SN1_MCLK_1 (S_GPIO[8]) for Sensor2
#define PIN_SENSOR_CFG_SN2_MCLK_1              0x400         ///< Enable SN2_MCLK_1 (S_GPIO[9]) for Sensor2

/**
    PIN config for Sensor2

    @note For pinmux_init() with PIN_FUNC_SENSOR2.
*/
#define PIN_SENSOR2_CFG_NONE                    0x0

#define PIN_SENSOR2_CFG_12BITS                  0x100         ///< SN_D[0..11]/SN_PXCLK/SN_VD/SN_HD (P_GPIO[11..14][0..7]/P_GPIO[8]/P_GPIO[9]/P_GPIO[10])

#define PIN_SENSOR2_CFG_CCIR8BITS_A             0x10000       ///< CCIR601/656. CCIR_A_YC[0..7]/CCIR_A_CLK (P_GPIO[0..7]/P_GPIO[8])
#define PIN_SENSOR2_CFG_CCIR8BITS_B             0x20000       ///< CCIR656. CCIR_B_YC[0..7]/CCIR_B_CLK (P_GPIO[12..19]/P_GPIO[11])
#define PIN_SENSOR2_CFG_CCIR8BITS_AB            0x40000       ///< CCIR601/656 + CCIR656. CCIR_A_YC[0..7]/CCIR_A_CLK (P_GPIO[0..7]/P_GPIO[8]) + CCIR_B_YC[0..7]/CCIR_B_CLK (P_GPIO[12..19]/P_GPIO[11])

#define PIN_SENSOR2_CFG_CCIR16BITS              0x10          ///< CCIR601/656. CCIR_Y[0..7]/CCIR_C[0..7]/CCIR_CLK (P_GPIO[0..7]/P_GPIO[12..19]/P_GPIO[8])
#define PIN_SENSOR2_CFG_CCIR_VSHS               0x100000      ///< For CCIR601. CCIR_VD/CCIR_HD/CCIR_FIELD (P_GPIO[9]/P_GPIO[10]/P_GPIO[11])

#define PIN_SENSOR2_CFG_SN5_MCLK_2              0x200         ///< Enable SN5_MCLK_2 (P_GPIO[20]) for Sensor2

/**
    PIN config for Sensor Misc

    @note For pinmux_init() with PIN_FUNC_SENSORMISC.
*/
#define PIN_SENSORMISC_CFG_NONE                 0x0

#define PIN_SENSORMISC_CFG_SN_MCLK_1            0x1           ///< SN1_MCLK_1 (S_GPIO[8])

#define PIN_SENSORMISC_CFG_SN2_MCLK_1           0x2           ///< SN2_MCLK_1 (S_GPIO[9])

#define PIN_SENSORMISC_CFG_SN3_MCLK_1           0x4           ///< SN3_MCLK_1 (S_GPIO[10])

#define PIN_SENSORMISC_CFG_SN4_MCLK_1           0x8           ///< SN4_MCLK_1 (S_GPIO[11])

#define PIN_SENSORMISC_CFG_SN5_MCLK_1           0x10          ///< SN5_MCLK_1 (S_GPIO[24])
#define PIN_SENSORMISC_CFG_SN5_MCLK_2           0x20          ///< SN5_MCLK_2 (P_GPIO[20])

#define PIN_SENSORMISC_CFG_SN6_MCLK_1           0x40          ///< SN6_MCLK_1 (S_GPIO[25])
#define PIN_SENSORMISC_CFG_SN6_MCLK_2           0x80          ///< SN6_MCLK_2 (P_GPIO[21])

#define PIN_SENSORMISC_CFG_SN7_MCLK_1           0x100         ///< SN7_MCLK_1 (S_GPIO[26])

#define PIN_SENSORMISC_CFG_SN8_MCLK_1           0x200         ///< SN8_MCLK_1 (S_GPIO[27])

#define PIN_SENSORMISC_CFG_SN_XVSXHS_1          0x400         ///< SN1_XVSXHS_1 (S_GPIO[0..1])
#define PIN_SENSORMISC_CFG_SN_XVSXHS_2          0x800         ///< SN1_XVSXHS_2 (S_GPIO[28..29])

#define PIN_SENSORMISC_CFG_SN2_XVSXHS_1         0x1000        ///< SN2_XVSXHS_1 (S_GPIO[2..3])
#define PIN_SENSORMISC_CFG_SN2_XVSXHS_2         0x2000        ///< SN2_XVSXHS_2 (S_GPIO[30..31])

#define PIN_SENSORMISC_CFG_SN3_XVSXHS_1         0x4000        ///< SN3_XVSXHS_1 (S_GPIO[4..5])

#define PIN_SENSORMISC_CFG_SN4_XVSXHS_1         0x8000        ///< SN4_XVSXHS_1 (S_GPIO[6..7])

#define PIN_SENSORMISC_CFG_SN5_XVSXHS_1         0x10000       ///< SN5_XVSXHS_1 (S_GPIO[16..17])

#define PIN_SENSORMISC_CFG_SN6_XVSXHS_1         0x20000       ///< SN6_XVSXHS_1 (S_GPIO[18..19])

#define PIN_SENSORMISC_CFG_SN7_XVSXHS_1         0x40000       ///< SN7_XVSXHS_1 (S_GPIO[20..21])

#define PIN_SENSORMISC_CFG_SN8_XVSXHS_1         0x80000       ///< SN8_XVSXHS_1 (S_GPIO[22..23])

#define PIN_SENSORMISC_CFG_FLASH_TRIGA_IN_1     0x100000      ///< FLASH_TRIG_IN_1 (S_GPIO[18])
#define PIN_SENSORMISC_CFG_FLASH_TRIGA_IN_2     0x200000      ///< FLASH_TRIG_IN_2 (S_GPIO[6])

#define PIN_SENSORMISC_CFG_FLASH_TRIGA1_OUT_1   0x400000      ///< FLASH_TRIG_OUT_1 (S_GPIO[19])
#define PIN_SENSORMISC_CFG_FLASH_TRIGA1_OUT_2   0x800000      ///< FLASH_TRIG_OUT_2 (S_GPIO[7])

#define PIN_SENSORMISC_CFG_ME_SHUT_IN_A         0x1000000     ///< SHUT_IN_A_1 (S_GPIO[26])
#define PIN_SENSORMISC_CFG_ME_SHUT_IN_B         0x2000000     ///< SHUT_IN_B_1 (S_GPIO[27])

#define PIN_SENSORMISC_CFG_ME_SHUT_OUT_A_1      0x4000000     ///< SHUT_OUT_A_1 (P_GPIO[0..1])
#define PIN_SENSORMISC_CFG_ME_SHUT_OUT_A_2      0x8000000     ///< SHUT_OUT_A_2 (D_GPIO[2..3])

#define PIN_SENSORMISC_CFG_ME_SHUT_OUT_B_1      0x10000000    ///< SHUT_OUT_B_1 (P_GPIO[2..3])
#define PIN_SENSORMISC_CFG_ME_SHUT_OUT_B_2      0x20000000    ///< SHUT_OUT_B_2 (D_GPIO[4..5])


/**
    PIN config for Sensor MiscII

    @note For pinmux_init() with PIN_FUNC_SENSORMISC.
*/
#define PIN_SENSORMISCII_CFG_NONE                       0x0

#define PIN_SENSORMISCII_CFG_FLASH_TRIGA2_OUT_1         0x1       ///< FLASH_TRIGA2_OUT_1 (S_GPIO[20])

#define PIN_SENSORMISCII_CFG_FLASH_TRIGA3_OUT_1         0x2       ///< FLASH_TRIGA3_OUT_1 (S_GPIO[21])

#define PIN_SENSORMISCII_CFG_FLASH_TRIGA4_OUT_1         0x4       ///< FLASH_TRIGA4_OUT_1 (S_GPIO[22])

#define PIN_SENSORMISCII_CFG_FLASH_TRIGB_IN_1           0x10       ///< FLASH_TRIGB_IN_1  (S_GPIO[23])

#define PIN_SENSORMISCII_CFG_FLASH_TRIGB1_OUT_1         0x20       ///< FLASH_TRIGB1_OUT_1 (S_GPIO[24])

#define PIN_SENSORMISCII_CFG_FLASH_TRIGB2_OUT_1         0x40       ///< FLASH_TRIGB2_OUT_1 (S_GPIO[25])

#define PIN_SENSORMISCII_CFG_FLASH_TRIGB3_OUT_1         0x80       ///< FLASH_TRIGB3_OUT_1 (S_GPIO[28])

#define PIN_SENSORMISCII_CFG_FLASH_TRIGB4_OUT_1         0x100      ///< FLASH_TRIGB4_OUT_1 (S_GPIO[29])



/**
    PIN config for Sensor Sync

    @note For pinmux_init() with PIN_FUNC_SENSORSYNC.
*/
#define PIN_SENSORSYNC_CFG_NONE             0x0

#define PIN_SENSORSYNC_CFG_SN_MCLKSRC_SN	0x01
#define PIN_SENSORSYNC_CFG_SN_MCLKSRC_SN2	0x02
#define PIN_SENSORSYNC_CFG_SN_MCLKSRC_SN3	0x04
#define PIN_SENSORSYNC_CFG_SN_MCLKSRC_SN4	0x08
#define PIN_SENSORSYNC_CFG_SN_MCLKSRC_SN5	0x10
#define PIN_SENSORSYNC_CFG_SN_MCLKSRC_SN6	0x20
#define PIN_SENSORSYNC_CFG_SN_MCLKSRC_SN7	0x40
#define PIN_SENSORSYNC_CFG_SN_MCLKSRC_SN8	0x80

#define PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN   0x0100
#define PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN2	0x0200
#define PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN3	0x0400
#define PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN4	0x0800
#define PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN5	0x1000
#define PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN6	0x2000
#define PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN7	0x4000
#define PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN8	0x8000


#define PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN   0x010000
#define PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN2  0x020000
#define PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN3	0x040000
#define PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN4	0x080000
#define PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN5	0x100000
#define PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN6	0x200000
#define PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN7	0x400000
#define PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN8	0x800000

#define PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN	0x01000000
#define PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN2	0x02000000
#define PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN3	0x04000000
#define PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN4	0x08000000
#define PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN5	0x10000000
#define PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN6	0x20000000
#define PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN7	0x40000000
#define PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN8	0x80000000

/**
    PIN config for Sensor Sync II

    @note For pinmux_init() with PIN_FUNC_SENSORSYNCII.
*/
#define PIN_SENSORSYNCII_CFG_NONE               0x0

#define PIN_SENSORSYNCII_CFG_SN5_MCLKSRC_SN     0x01
#define PIN_SENSORSYNCII_CFG_SN5_MCLKSRC_SN2	0x02
#define PIN_SENSORSYNCII_CFG_SN5_MCLKSRC_SN3	0x04
#define PIN_SENSORSYNCII_CFG_SN5_MCLKSRC_SN4	0x08
#define PIN_SENSORSYNCII_CFG_SN5_MCLKSRC_SN5	0x10
#define PIN_SENSORSYNCII_CFG_SN5_MCLKSRC_SN6	0x20
#define PIN_SENSORSYNCII_CFG_SN5_MCLKSRC_SN7	0x40
#define PIN_SENSORSYNCII_CFG_SN5_MCLKSRC_SN8	0x80

#define PIN_SENSORSYNCII_CFG_SN6_MCLKSRC_SN     0x0100
#define PIN_SENSORSYNCII_CFG_SN6_MCLKSRC_SN2	0x0200
#define PIN_SENSORSYNCII_CFG_SN6_MCLKSRC_SN3	0x0400
#define PIN_SENSORSYNCII_CFG_SN6_MCLKSRC_SN4	0x0800
#define PIN_SENSORSYNCII_CFG_SN6_MCLKSRC_SN5	0x1000
#define PIN_SENSORSYNCII_CFG_SN6_MCLKSRC_SN6	0x2000
#define PIN_SENSORSYNCII_CFG_SN6_MCLKSRC_SN7	0x4000
#define PIN_SENSORSYNCII_CFG_SN6_MCLKSRC_SN8	0x8000

#define PIN_SENSORSYNCII_CFG_SN7_MCLKSRC_SN     0x010000
#define PIN_SENSORSYNCII_CFG_SN7_MCLKSRC_SN2	0x020000
#define PIN_SENSORSYNCII_CFG_SN7_MCLKSRC_SN3	0x040000
#define PIN_SENSORSYNCII_CFG_SN7_MCLKSRC_SN4	0x080000
#define PIN_SENSORSYNCII_CFG_SN7_MCLKSRC_SN5	0x100000
#define PIN_SENSORSYNCII_CFG_SN7_MCLKSRC_SN6	0x200000
#define PIN_SENSORSYNCII_CFG_SN7_MCLKSRC_SN7	0x400000
#define PIN_SENSORSYNCII_CFG_SN7_MCLKSRC_SN8	0x800000

#define PIN_SENSORSYNCII_CFG_SN8_MCLKSRC_SN     0x01000000
#define PIN_SENSORSYNCII_CFG_SN8_MCLKSRC_SN2	0x02000000
#define PIN_SENSORSYNCII_CFG_SN8_MCLKSRC_SN3	0x04000000
#define PIN_SENSORSYNCII_CFG_SN8_MCLKSRC_SN4	0x08000000
#define PIN_SENSORSYNCII_CFG_SN8_MCLKSRC_SN5	0x10000000
#define PIN_SENSORSYNCII_CFG_SN8_MCLKSRC_SN6	0x20000000
#define PIN_SENSORSYNCII_CFG_SN8_MCLKSRC_SN7	0x40000000
#define PIN_SENSORSYNCII_CFG_SN8_MCLKSRC_SN8	0x80000000

/**
	PIN config for Sensor Sync III

	@note For pinmux_init() with PIN_FUNC_SENSORSYNCIII.
*/

#define PIN_SENSORSYNCIII_CFG_NONE              0x0

#define PIN_SENSORSYNCIII_CFG_SN_XVSXHSSRC_SN   0x01
#define PIN_SENSORSYNCIII_CFG_SN_XVSXHSSRC_SN2  0x02
#define PIN_SENSORSYNCIII_CFG_SN_XVSXHSSRC_SN3  0x04
#define PIN_SENSORSYNCIII_CFG_SN_XVSXHSSRC_SN4  0x08
#define PIN_SENSORSYNCIII_CFG_SN_XVSXHSSRC_SN5  0x10
#define PIN_SENSORSYNCIII_CFG_SN_XVSXHSSRC_SN6  0x20
#define PIN_SENSORSYNCIII_CFG_SN_XVSXHSSRC_SN7  0x40
#define PIN_SENSORSYNCIII_CFG_SN_XVSXHSSRC_SN8  0x80

#define PIN_SENSORSYNCIII_CFG_SN2_XVSXHSSRC_SN  0x0100
#define PIN_SENSORSYNCIII_CFG_SN2_XVSXHSSRC_SN2 0x0200
#define PIN_SENSORSYNCIII_CFG_SN2_XVSXHSSRC_SN3 0x0400
#define PIN_SENSORSYNCIII_CFG_SN2_XVSXHSSRC_SN4 0x0800
#define PIN_SENSORSYNCIII_CFG_SN2_XVSXHSSRC_SN5 0x1000
#define PIN_SENSORSYNCIII_CFG_SN2_XVSXHSSRC_SN6 0x2000
#define PIN_SENSORSYNCIII_CFG_SN2_XVSXHSSRC_SN7 0x4000
#define PIN_SENSORSYNCIII_CFG_SN2_XVSXHSSRC_SN8 0x8000

#define PIN_SENSORSYNCIII_CFG_SN3_XVSXHSSRC_SN  0x010000
#define PIN_SENSORSYNCIII_CFG_SN3_XVSXHSSRC_SN2 0x020000
#define PIN_SENSORSYNCIII_CFG_SN3_XVSXHSSRC_SN3 0x040000
#define PIN_SENSORSYNCIII_CFG_SN3_XVSXHSSRC_SN4 0x080000
#define PIN_SENSORSYNCIII_CFG_SN3_XVSXHSSRC_SN5 0x100000
#define PIN_SENSORSYNCIII_CFG_SN3_XVSXHSSRC_SN6 0x200000
#define PIN_SENSORSYNCIII_CFG_SN3_XVSXHSSRC_SN7 0x400000
#define PIN_SENSORSYNCIII_CFG_SN3_XVSXHSSRC_SN8 0x800000

#define PIN_SENSORSYNCIII_CFG_SN4_XVSXHSSRC_SN  0x01000000
#define PIN_SENSORSYNCIII_CFG_SN4_XVSXHSSRC_SN2 0x02000000
#define PIN_SENSORSYNCIII_CFG_SN4_XVSXHSSRC_SN3 0x04000000
#define PIN_SENSORSYNCIII_CFG_SN4_XVSXHSSRC_SN4 0x08000000
#define PIN_SENSORSYNCIII_CFG_SN4_XVSXHSSRC_SN5 0x10000000
#define PIN_SENSORSYNCIII_CFG_SN4_XVSXHSSRC_SN6 0x20000000
#define PIN_SENSORSYNCIII_CFG_SN4_XVSXHSSRC_SN7 0x40000000
#define PIN_SENSORSYNCIII_CFG_SN4_XVSXHSSRC_SN8 0x80000000

/**
        PIN config for Sensor Sync IV

        @note For pinmux_init() with PIN_FUNC_SENSORSYNCIV.
*/

#define PIN_SENSORSYNCIV_CFG_NONE               0x0

#define PIN_SENSORSYNCIV_CFG_SN5_XVSXHSSRC_SN	0x01
#define PIN_SENSORSYNCIV_CFG_SN5_XVSXHSSRC_SN2	0x02
#define PIN_SENSORSYNCIV_CFG_SN5_XVSXHSSRC_SN3	0x04
#define PIN_SENSORSYNCIV_CFG_SN5_XVSXHSSRC_SN4	0x08
#define PIN_SENSORSYNCIV_CFG_SN5_XVSXHSSRC_SN5	0x10
#define PIN_SENSORSYNCIV_CFG_SN5_XVSXHSSRC_SN6	0x20
#define PIN_SENSORSYNCIV_CFG_SN5_XVSXHSSRC_SN7	0x40
#define PIN_SENSORSYNCIV_CFG_SN5_XVSXHSSRC_SN8	0x80

#define PIN_SENSORSYNCIV_CFG_SN6_XVSXHSSRC_SN	0x0100
#define PIN_SENSORSYNCIV_CFG_SN6_XVSXHSSRC_SN2	0x0200
#define PIN_SENSORSYNCIV_CFG_SN6_XVSXHSSRC_SN3	0x0400
#define PIN_SENSORSYNCIV_CFG_SN6_XVSXHSSRC_SN4	0x0800
#define PIN_SENSORSYNCIV_CFG_SN6_XVSXHSSRC_SN5	0x1000
#define PIN_SENSORSYNCIV_CFG_SN6_XVSXHSSRC_SN6	0x2000
#define PIN_SENSORSYNCIV_CFG_SN6_XVSXHSSRC_SN7	0x4000
#define PIN_SENSORSYNCIV_CFG_SN6_XVSXHSSRC_SN8	0x8000

#define PIN_SENSORSYNCIV_CFG_SN7_XVSXHSSRC_SN	0x010000
#define PIN_SENSORSYNCIV_CFG_SN7_XVSXHSSRC_SN2	0x020000
#define PIN_SENSORSYNCIV_CFG_SN7_XVSXHSSRC_SN3	0x040000
#define PIN_SENSORSYNCIV_CFG_SN7_XVSXHSSRC_SN4	0x080000
#define PIN_SENSORSYNCIV_CFG_SN7_XVSXHSSRC_SN5	0x100000
#define PIN_SENSORSYNCIV_CFG_SN7_XVSXHSSRC_SN6	0x200000
#define PIN_SENSORSYNCIV_CFG_SN7_XVSXHSSRC_SN7	0x400000
#define PIN_SENSORSYNCIV_CFG_SN7_XVSXHSSRC_SN8	0x800000

#define PIN_SENSORSYNCIV_CFG_SN8_XVSXHSSRC_SN	0x01000000
#define PIN_SENSORSYNCIV_CFG_SN8_XVSXHSSRC_SN2	0x02000000
#define PIN_SENSORSYNCIV_CFG_SN8_XVSXHSSRC_SN3	0x04000000
#define PIN_SENSORSYNCIV_CFG_SN8_XVSXHSSRC_SN4	0x08000000
#define PIN_SENSORSYNCIV_CFG_SN8_XVSXHSSRC_SN5	0x10000000
#define PIN_SENSORSYNCIV_CFG_SN8_XVSXHSSRC_SN6	0x20000000
#define PIN_SENSORSYNCIV_CFG_SN8_XVSXHSSRC_SN7	0x40000000
#define PIN_SENSORSYNCIV_CFG_SN8_XVSXHSSRC_SN8	0x80000000


/**
    PIN config for AUDIO

    @note For pinmux_init() with PIN_FUNC_AUDIO.
*/
#define PIN_AUDIO_CFG_NONE          0x0
#define PIN_AUDIO_CFG_I2S_1         0x1         ///< I2S_1       (PGPIO[24..27])
#define PIN_AUDIO_CFG_I2S_2         0x2         ///< I2S_2       (CGPIO[20..23])

#define PIN_AUDIO_CFG_I2S_MCLK_1    0x4         ///< I2S_MCLK_1  (PGPIO[28])
#define PIN_AUDIO_CFG_I2S_MCLK_2    0x8         ///< I2S_MCLK_2  (CGPIO[17])

#define PIN_AUDIO_CFG_I2S2_1        0x10        ///< I2S2_1      (PGPIO[0..3])
#define PIN_AUDIO_CFG_I2S2_2        0x20        ///< I2S2_2      (LGPIO[19..22])

#define PIN_AUDIO_CFG_I2S2_MCLK_1   0x40         ///< I2S2_MCLK_1 (PGPIO[12])
#define PIN_AUDIO_CFG_I2S2_MCLK_2   0x80         ///< I2S2_MCLK_2 (LGPIO[23])

#define PIN_AUDIO_CFG_I2S3_1        0x100        ///< I2S3_1      (PGPIO[4..7])
#define PIN_AUDIO_CFG_I2S3_2        0x200        ///< I2S3_2      (DGPIO[1..4])

#define PIN_AUDIO_CFG_I2S3_MCLK_1   0x400        ///< I2S3_MCLK_1 (PGPIO[13])
#define PIN_AUDIO_CFG_I2S3_MCLK_2   0x800        ///< I2S3_MCLK_2 (DGPIO[0])

#define PIN_AUDIO_CFG_I2S4_1        0x1000       ///< I2S4_1      (PGPIO[38..41])
#define PIN_AUDIO_CFG_I2S4_2        0x2000       ///< I2S4_2      (PGPIO[26..29])

#define PIN_AUDIO_CFG_I2S4_MCLK_1   0x4000       ///< I2S4_MCLK_1 (PGPIO[29])
#define PIN_AUDIO_CFG_I2S4_MCLK_2   0x8000       ///< I2S4_MCLK_2 (LGPIO[30])

#define PIN_AUDIO_CFG_DMIC_1        0x10000      ///< DMIC_1 (PGPIO[26]) DATA0 (PGPIO[27]) DATA1 (PGPIO[25])
#define PIN_AUDIO_CFG_DMIC_DATA0    0x20000
#define PIN_AUDIO_CFG_DMIC_DATA1    0x40000

#define PIN_AUDIO_CFG_EXT_EAC_MCLK  0x100000     ///< EXT_EAC_MCLK (CGPIO[16])

/**
    PIN config for UART

    @note For pinmux_init() with PIN_FUNC_UART.
*/
#define PIN_UART_CFG_NONE               0x0

#define PIN_UART_CFG_UART_1             0x1          ///< UART_1  (P_GPIO[32..33])

#define PIN_UART_CFG_UART2_1            0x10         ///< UART2_1 (P_GPIO[30..31]) RTSCTS (P_GPIO[28..29]) DIROE (P_GPIO[28])

#define PIN_UART_CFG_UART3_1            0x100        ///< UART3_1 (P_GPIO[24..25]) RTSCTS (P_GPIO[26..27]) DIROE (P_GPIO[26])

#define PIN_UART_CFG_UART4_1            0x1000       ///< UART4_1 (P_GPIO[20..21]) RTSCTS (P_GPIO[22..23]) DIROE (P_GPIO[22])
#define PIN_UART_CFG_UART4_2            0x2000       ///< UART4_2 (A_GPIO[0..1])   RTSCTS (A_GPIO[2..3])   DIROE (A_GPIO[2])

#define PIN_UART_CFG_UART5_1            0x10000      ///< UART5_1 (P_GPIO[16..17]) RTSCTS (P_GPIO[18..19]) DIROE (P_GPIO[18])
#define PIN_UART_CFG_UART5_2            0x20000      ///< UART5_2 (DSI_GPIO[0..1]) RTSCTS (DSI_GPIO[2..3]) DIROE (DSI_GPIO[2])

#define PIN_UART_CFG_UART2_RTSCTS       0x100000
#define PIN_UART_CFG_UART2_DIROE        0x200000

#define PIN_UART_CFG_UART3_RTSCTS       0x400000
#define PIN_UART_CFG_UART3_DIROE        0x800000

#define PIN_UART_CFG_UART4_RTSCTS       0x1000000
#define PIN_UART_CFG_UART4_DIROE        0x2000000

#define PIN_UART_CFG_UART5_RTSCTS       0x4000000
#define PIN_UART_CFG_UART5_DIROE        0x8000000

/**
    PIN config for UARTII

    @note For pinmux_init() with PIN_FUNC_UARTII.
*/
#define PIN_UARTII_CFG_NONE             0x0

#define PIN_UARTII_CFG_UART6_1          0x1          ///< UART6_1 (P_GPIO[12..13]) RTSCTS (P_GPIO[14..15]) DIROE (P_GPIO[14])
#define PIN_UARTII_CFG_UART6_2          0x2          ///< UART6_2 (DSI_GPIO[4..5]) RTSCTS (DSI_GPIO[6..7]) DIROE (DSI_GPIO[6])

#define PIN_UARTII_CFG_UART7_1          0x10         ///< UART7_1 (P_GPIO[8..9])   RTSCTS (P_GPIO[10..11]) DIROE (P_GPIO[10])
#define PIN_UARTII_CFG_UART7_2          0x20         ///< UART7_2 (C_GPIO[4..5])   RTSCTS (C_GPIO[6..7])   DIROE (C_GPIO[6])

#define PIN_UARTII_CFG_UART8_1          0x100        ///< UART8_1 (P_GPIO[4..5])   RTSCTS (P_GPIO[6..7])   DIROE (P_GPIO[6])
#define PIN_UARTII_CFG_UART8_2          0x200        ///< UART8_2 (C_GPIO[17..18]) RTSCTS (C_GPIO[19..20]) DIROE (C_GPIO[21])

#define PIN_UARTII_CFG_UART9_1          0x1000       ///< UART9_1 (P_GPIO[0..1])   RTSCTS (P_GPIO[2..3])   DIROE (P_GPIO[2])
#define PIN_UARTII_CFG_UART9_2          0x2000       ///< UART9_2 (D_GPIO[5..6])   RTSCTS (D_GPIO[7..8])   DIROE (D_GPIO[8])

#define PIN_UARTII_CFG_UART6_RTSCTS     0x10000
#define PIN_UARTII_CFG_UART6_DIROE      0x20000

#define PIN_UARTII_CFG_UART7_RTSCTS     0x40000
#define PIN_UARTII_CFG_UART7_DIROE      0x80000

#define PIN_UARTII_CFG_UART8_RTSCTS     0x100000
#define PIN_UARTII_CFG_UART8_DIROE      0x200000

#define PIN_UARTII_CFG_UART9_RTSCTS     0x400000
#define PIN_UARTII_CFG_UART9_DIROE      0x800000

/**
    PIN config for REMOTE

    @note For pinmux_init() with PIN_FUNC_REMOTE.
*/
#define PIN_REMOTE_CFG_NONE             0x0

#define PIN_REMOTE_CFG_REMOTE_1         0x1          ///< REMOTE_1     (P_GPIO[31])
#define PIN_REMOTE_CFG_REMOTE_EXT_1     0x2          ///< REMOTE_EXT_1 (P_GPIO[30])

/**
    PIN config for SDP

    @note For pinmux_init() with PIN_FUNC_SDP.
*/
#define PIN_SDP_CFG_NONE                0x0

#define PIN_SDP_CFG_SDP_1               0x1          ///< SDP_1 (P_GPIO[15..19])
#define PIN_SDP_CFG_SDP_2               0x2          ///< SDP_2 (C_GPIO[17..21])

/**
    PIN config for SPI

    @note For pinmux_init() with PIN_FUNC_SPI.
*/
#define PIN_SPI_CFG_NONE                0x0

#define PIN_SPI_CFG_SPI_1               0x1          ///< SPI_1  (P_GPIO[12..14])  BUS_WIDTH (P_GPIO[15])
#define PIN_SPI_CFG_SPI_2               0x2          ///< SPI1_2 (DSI_GPIO[0..2])  BUS_WIDTH (DSI_GPIO[3])
#define PIN_SPI_CFG_SPI_3               0x4          ///< SPI1_3 (C_GPIO[4..6])    BUS_WIDTH (C_GPIO[7])
#define PIN_SPI_CFG_SPI_BUS_WIDTH       0x8

#define PIN_SPI_CFG_SPI2_1              0x10         ///< SPI2_1 (S_GPIO[12..14])  BUS_WIDTH (S_GPIO[15])
#define PIN_SPI_CFG_SPI2_2              0x20         ///< SPI2_2 (P_GPIO[16..18])  BUS_WIDTH (P_GPIO[19])
#define PIN_SPI_CFG_SPI2_3              0x40         ///< SPI2_3 (S_GPIO[28..30])  BUS_WIDTH (S_GPIO[31])
#define PIN_SPI_CFG_SPI2_BUS_WIDTH      0x80

#define PIN_SPI_CFG_SPI3_1              0x100        ///< SPI3_1 (P_GPIO[20..22])  BUS_WIDTH (P_GPIO[23])
#define PIN_SPI_CFG_SPI3_2              0x200        ///< SPI3_2 (C_GPIO[19..21])  BUS_WIDTH (C_GPIO[22])
#define PIN_SPI_CFG_SPI3_BUS_WIDTH      0x400

#define PIN_SPI_CFG_SPI4_1              0x1000       ///< SPI4_1 (P_GPIO[24..26])  BUS_WIDTH (P_GPIO[27])
#define PIN_SPI_CFG_SPI4_2              0x2000       ///< SPI4_2 (D_GPIO[2..4])    BUS_WIDTH (D_GPIO[5])
#define PIN_SPI_CFG_SPI4_3              0x4000       ///< SPI4_3 (S_GPIO[20..22])  BUS_WIDTH (S_GPIO[23])
#define PIN_SPI_CFG_SPI4_BUS_WIDTH      0x8000

#define PIN_SPI_CFG_SPI5_1              0x10000      ///< SPI5_1 (P_GPIO[28..30])  BUS_WIDTH (P_GPIO[31])
#define PIN_SPI_CFG_SPI5_2              0x20000      ///< SPI5_2 (L_GPIO[21..22] L_GPIO[24]) BUS_WIDTH (L_GPIO[23])
#define PIN_SPI_CFG_SPI5_BUS_WIDTH      0x40000

#define PIN_SPI_CFG_SPI3_RDY_1          0x100000     ///< SPI3RDY_1 (P_GPIO[27])
#define PIN_SPI_CFG_SPI3_RDY_2          0x200000     ///< SPI3RDY_2 (C_GPIO[23])

/**
    PIN config for SIF

    *note For pinmux_init() with PIN_FUNC_SIF.
*/
#define PIN_SIF_CFG_NONE                0x0

#define PIN_SIF_CFG_SIF0_1              0x1          ///< SIF0_1 (P_GPIO[24..26])
#define PIN_SIF_CFG_SIF0_2              0x2          ///< SIF0_2 (S_GPIO[20..22])

#define PIN_SIF_CFG_SIF1_1              0x10         ///< SIF1_1 (P_GPIO[28..30])
#define PIN_SIF_CFG_SIF1_2              0x20         ///< SIF1_2 (S_GPIO[28..30])

#define PIN_SIF_CFG_SIF2_1              0x100        ///< SIF2_1 (P_GPIO[20..22])
#define PIN_SIF_CFG_SIF2_2              0x200        ///< SIF2_2 (S_GPIO[12..14])

#define PIN_SIF_CFG_SIF3_1              0x1000       ///< SIF3_1 (P_GPIO[12..14])
#define PIN_SIF_CFG_SIF3_2              0x2000       ///< SIF3_2 (L_GPIO[21..23])
#define PIN_SIF_CFG_SIF3_3              0x4000       ///< SIF3_3 (DSI_GPIO[10..12])

#define PIN_SIF_CFG_SIF4_1              0x10000      ///< SIF4_1 (P_GPIO[16..18])
#define PIN_SIF_CFG_SIF4_2              0x20000      ///< SIF4_2 (L_GPIO[28..30])

#define PIN_SIF_CFG_SIF5_1              0x100000     ///< SIF5_1 (P_GPIO[8..10])
#define PIN_SIF_CFG_SIF5_2              0x200000     ///< SIF5_2 (C_GPIO[4..6])

/**
    PIN config for MISC

    @note For pinmux_init() for PIN_FUNC_MISC.
*/
#define PIN_MISC_CFG_NONE                   0x0

#define PIN_MISC_CFG_RTC_CLK_1              0x1          ///< RTC_CLK_1     (P_GPIO[29])
#define PIN_MISC_CFG_RTC_EXT_CLK_1          0x2          ///< RTC_EXT_CLK_1 (P_GPIO[23])
#define PIN_MISC_CFG_RTC_DIV_OUT_1          0x4          ///< RTC_DIV_OUT_1 (P_GPIO[22])

#define PIN_MISC_CFG_SP_CLK_1               0x10         ///< SP_CLK_1  (D_GPIO[6])

#define PIN_MISC_CFG_SP2_CLK_1              0x100        ///< SP2_CLK_1 (DSI_GPIO[10])

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

/**
    PIN config for pcie

    @note For pinmux_init() for PIN_FUNC_PCIE.
*/
#define PIN_PCIE_CFG_NONE                   0x0

#define PIN_PCIE_CFG_CTRL1_MODE_EP          0x1          ///< PCIE controller 1 mode select (not pad related)
#define PIN_PCIE_CFG_CTRL2_MODE_EP          0x2          ///< PCIE controller 2 mode select (not pad related)

#define PIN_PCIE_CFG_RSTN		    0x10         ///< PCIE_RSTN (P_GPIO[43])

#define PIN_PCIE_CFG_REFCLK_OUTEN	    0x100        ///< PCIE REFCLK pad output enable (pad name: pcie_clkp, pcie_clkn)

/**
    Pinmux selection for PCIE

*/
#define PINMUX_PCIEMUX_SEL_NONE			(0x00 << 28)    ///< PINMUX none
#define PINMUX_PCIEMUX_SEL_RSTN                 (0x01 << 28)    ///< PINMUX at LCD interface
#define PINMUX_PCIEMUX_SEL_MASK                 (0x03 << 28)

#define PINMUX_PCIEMUX_RSTN_OUT_LOW             (1)    		///< RSTN output low (for PINMUX_PCIEMUX_SEL_RSTN)
#define PINMUX_PCIEMUX_RSTN_HIGHZ		(2)    		///< RSTN high-Z (for PINMUX_PCIEMUX_SEL_RSTN)

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



#endif
