/* SPDX-License-Identifier: GPL-2.0 */
/*
 * This header provides constants specific to Novatek NS02302 pinctrl bindings.
 */

#ifndef __DT_BINDINGS_PINCTRL_NS02302_PINCTRL_H
#define __DT_BINDINGS_PINCTRL_NS02302_PINCTRL_H

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
#define PIN_FUNC_SENSOR3                10
#define PIN_FUNC_SENSORMISC             11
#define PIN_FUNC_SENSORSYNC             12
#define PIN_FUNC_AUDIO                  13
#define PIN_FUNC_UART                   14
#define PIN_FUNC_UARTII                 15
#define PIN_FUNC_CSI                    16
#define PIN_FUNC_REMOTE                 17
#define PIN_FUNC_SDP                    18
#define PIN_FUNC_SPI                    19
#define PIN_FUNC_SIF                    20
#define PIN_FUNC_MISC                   21
#define PIN_FUNC_LCD                    22
#define PIN_FUNC_TV                     23
#define PIN_FUNC_SEL_LCD                24
#define PIN_FUNC_MAX                    25

/**
    PIN config for SDIO

    @note For pinmux_init() with PIN_FUNC_SDIO.
*/
#define PIN_SDIO_CFG_NONE               0x0

#define PIN_SDIO_CFG_SDIO_1             0x1         ///< SDIO  (MC[12..17])

#define PIN_SDIO_CFG_SDIO2_1            0x10        ///< SDIO2 (MC18..23])

#define PIN_SDIO_CFG_SDIO3_1            0x100       ///< SDIO3 (MC[0..3] MC[8..9])
#define PIN_SDIO_CFG_SDIO3_BUS_WIDTH    0x200       ///< SDIO3 bus width 8 bits (MC[4..7])
#define PIN_SDIO_CFG_SDIO3_DS           0x400       ///< SDIO3 data strobe      (MC[10])

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

#define PIN_NAND_CFG_NAND_1             0x1          ///< FSPI     (MC[0..3] MC[8] MC[11])
#define PIN_NAND_CFG_NAND_2             0x2          ///< FSPI     (MC[0..8] MC[10] MC[11])
#define PIN_NAND_CFG_NAND_CS1           0x4          ///< FSPI CS1 (MC[9])

/**
    PIN config for ETH

    @note For pinmux_init() for PIN_FUNC_ETH
*/
//==================================== Shall we change? =======================================
#define PIN_ETH_CFG_NONE                0x0

#define PIN_ETH_CFG_ETH_RMII_1          0x001       ///< ETH_RMII_1         (L_GPIO[0..6])
#define PIN_ETH_CFG_ETH_RMII_2          0x002       ///< ETH_RMII_2         (L_GPIO[11..13] L_GPIO[17..20])
#define PIN_ETH_CFG_ETH_RGMII           0x010       ///< ETH_RGMII_1        (L_GPIO[11..22])

#define PIN_ETH_CFG_ETH_EXTPHYCLK       0x100       ///< ETH_EXTPHYCLK	    RMII_1(L_GPIO[9]) RMII_2||RMGII(D_GPIO[0])
#define PIN_ETH_CFG_ETH_PTP             0x200       ///< ETH_PTP            (P_GPIO[21])
#define PIN_ETH_CFG_ETH_PTP_2           0x400       ///< ETH_PTP_2            (P_GPIO[12])

#define PIN_ETH_CFG_ETH_MDIO_1          0x1000      ///< ETH_MDIO_1         (L_GPIO[7..8])
#define PIN_ETH_CFG_ETH_MDIO_2          0x2000      ///< ETH_MDIO_2         (L_GPIO[23..24])

#define PIN_ETH_CFG_ETH_LED_1           0x10000       ///< ETH_PTP            (D_GPIO[0..1])
#define PIN_ETH_CFG_ETH_LED_2           0x20000       ///< ETH_PTP          JTAG_TDI /JTAG_TDO   (D_GPIO[5..6])

/**
    PIN config for I2C

    @note For pinmux_init() with PIN_FUNC_I2C.
*/
#define PIN_I2C_CFG_NONE                0x0

#define PIN_I2C_CFG_I2C_1               0x1         ///< I2C_1      (S_GPIO[4..5])

#define PIN_I2C_CFG_I2C2_1              0x10        ///< I2C2_1     (S_GPIO[10..11])
#define PIN_I2C_CFG_I2C2_2              0x20        ///< I2C2_2     (MC[17..18])

#define PIN_I2C_CFG_I2C3_1              0x100       ///< I2C3_1     (P_GPIO[21..22])
#define PIN_I2C_CFG_I2C3_2              0x200       ///< I2C3_2     (C_GPIO[12..13])
#define PIN_I2C_CFG_I2C3_3              0x400       ///< I2C3_3     (S_GPIO[12..13])

#define PIN_I2C_CFG_I2C4_1              0x1000      ///< I2C4_1     (P_GPIO[11..12])
#define PIN_I2C_CFG_I2C4_2              0x2000      ///< I2C4_2     (C_GPIO[6..7])
#define PIN_I2C_CFG_I2C4_3              0x4000      ///< I2C4_3     (S_GPIO[14..15])

#define PIN_I2C_CFG_I2C5_1              0x10000     ///< I2C5_1     (P_GPIO[2..3])

/**
    PIN config for PWM

    @note For pinmux_init() with PIN_FUNC_PWM.
*/
#define PIN_PWM_CFG_NONE                0x0

#define PIN_PWM_CFG_PWM0_1              0x1         ///< PWM0_1  (P_GPIO[0])
#define PIN_PWM_CFG_PWM0_2              0x2         ///< PWM0_2  (C_GPIO[14])
#define PIN_PWM_CFG_PWM0_3              0x4         ///< PWM0_3  (S_GPIO[1])
#define PIN_PWM_CFG_PWM0_4              0x8         ///< PWM0_4  (C_GPIO[4])
#define PIN_PWM_CFG_PWM0_5              0x10        ///< PWM0_5  (L_GPIO[0])

#define PIN_PWM_CFG_PWM1_1              0x100        ///< PWM1_1  (P_GPIO[1])
#define PIN_PWM_CFG_PWM1_2              0x200        ///< PWM1_2  (C_GPIO[15])
#define PIN_PWM_CFG_PWM1_3              0x400        ///< PWM1_3  (S_GPIO[2])
#define PIN_PWM_CFG_PWM1_4              0x800       ///< PWM1_4  (C_GPIO[5])
#define PIN_PWM_CFG_PWM1_5              0x1000       ///< PWM1_5  (L_GPIO[1])

#define PIN_PWM_CFG_PWM2_1              0x10000       ///< PWM2_1  (P_GPIO[2])
#define PIN_PWM_CFG_PWM2_2              0x20000       ///< PWM2_2  (C_GPIO[16])
#define PIN_PWM_CFG_PWM2_3              0x40000      ///< PWM2_3  (S_GPIO[3])
#define PIN_PWM_CFG_PWM2_4              0x80000      ///< PWM2_4  (C_GPIO[6])
#define PIN_PWM_CFG_PWM2_5              0x100000      ///< PWM2_5  (L_GPIO[2])

#define PIN_PWM_CFG_PWM3_1              0x1000000       ///< PWM3_1  (P_GPIO[3])
#define PIN_PWM_CFG_PWM3_2              0x2000000      ///< PWM3_2  (C_GPIO[17])
#define PIN_PWM_CFG_PWM3_3              0x4000000      ///< PWM3_3  (S_GPIO[4])
#define PIN_PWM_CFG_PWM3_4              0x8000000      ///< PWM3_4  (C_GPIO[7])
#define PIN_PWM_CFG_PWM3_5              0x10000000      ///< PWM3_5  (L_GPIO[3])

/**
    PIN config for PWMII

    @note For pinmux_init() with PIN_FUNC_PWMII.
*/
#define PIN_PWMII_CFG_NONE              0x0

#define PIN_PWMII_CFG_PWM4_1              0x1     ///< PWM4_1  (P_GPIO[4])
#define PIN_PWMII_CFG_PWM4_2              0x2     ///< PWM4_2  (C_GPIO[12])
#define PIN_PWMII_CFG_PWM4_3              0x4     ///< PWM4_3  (S_GPIO[5])
#define PIN_PWMII_CFG_PWM4_4              0x8     ///< PWM4_4  (D_GPIO[3])
#define PIN_PWMII_CFG_PWM4_5              0x10    ///< PWM4_5  (L_GPIO[4])

#define PIN_PWMII_CFG_PWM5_1              0x100     ///< PWM5_1  (P_GPIO[5])
#define PIN_PWMII_CFG_PWM5_2              0x200     ///< PWM5_2  (C_GPIO[13])
#define PIN_PWMII_CFG_PWM5_3              0x400     ///< PWM5_3  (S_GPIO[6])
#define PIN_PWMII_CFG_PWM5_4              0x800     ///< PWM5_4  (D_GPIO[4])
#define PIN_PWMII_CFG_PWM5_5              0x1000    ///< PWM5_5  (L_GPIO[5])

#define PIN_PWMII_CFG_PWM6_1              0x10000     ///< PWM6_1  (P_GPIO[6])
#define PIN_PWMII_CFG_PWM6_2              0x20000     ///< PWM6_2  (C_GPIO[18])
#define PIN_PWMII_CFG_PWM6_3              0x40000     ///< PWM6_3  (S_GPIO[7])
#define PIN_PWMII_CFG_PWM6_4              0x80000     ///< PWM6_4  (D_GPIO[5])
#define PIN_PWMII_CFG_PWM6_5              0x100000    ///< PWM6_5  (L_GPIO[6])

#define PIN_PWMII_CFG_PWM7_1              0x1000000     ///< PWM7_1  (P_GPIO[7])
#define PIN_PWMII_CFG_PWM7_2              0x2000000     ///< PWM7_2  (C_GPIO[19])
#define PIN_PWMII_CFG_PWM7_3              0x4000000     ///< PWM7_3  (S_GPIO[8])
#define PIN_PWMII_CFG_PWM7_4              0x8000000     ///< PWM7_4  (D_GPIO[6])
#define PIN_PWMII_CFG_PWM7_5              0x10000000    ///< PWM7_5  (L_GPIO[7])

/**
    PIN config for PWMIII

    @note For pinmux_init() with PIN_FUNC_PWMIII.
*/
#define PIN_PWMIII_CFG_NONE              0x0

#define PIN_PWMIII_CFG_PWM8_1              0x1     ///< PWM8_1  (P_GPIO[8])
#define PIN_PWMIII_CFG_PWM8_2              0x2     ///< PWM8_2  (C_GPIO[20])
#define PIN_PWMIII_CFG_PWM8_4              0x8     ///< PWM8_4  (D_GPIO[8])
#define PIN_PWMIII_CFG_PWM8_5              0x10    ///< PWM8_5  (L_GPIO[8])

#define PIN_PWMIII_CFG_PWM9_1              0x100     ///< PWM9_1  (P_GPIO[9])
#define PIN_PWMIII_CFG_PWM9_2              0x200     ///< PWM9_2  (C_GPIO[21])
#define PIN_PWMIII_CFG_PWM9_4              0x800     ///< PWM9_4  (D_GPIO[9])
#define PIN_PWMIII_CFG_PWM9_5              0x1000    ///< PWM9_5  (L_GPIO[9])

#define PIN_PWMIII_CFG_PWM10_1              0x10000     ///< PWM10_1  (P_GPIO[10])
#define PIN_PWMIII_CFG_PWM10_2              0x20000     ///< PWM10_2  (C_GPIO[22])
#define PIN_PWMIII_CFG_PWM10_4              0x80000     ///< PWM10_4  (D_GPIO[10])
#define PIN_PWMIII_CFG_PWM10_5              0x100000    ///< PWM10_5  (L_GPIO[10])

#define PIN_PWMIII_CFG_PWM11_1              0x1000000     ///< PWM10_1  (P_GPIO[11])
#define PIN_PWMIII_CFG_PWM11_2              0x2000000     ///< PWM10_2  (C_GPIO[23])
#define PIN_PWMIII_CFG_PWM11_4              0x8000000     ///< PWM10_4  (D_GPIO[7])
#define PIN_PWMIII_CFG_PWM11_5              0x10000000    ///< PWM10_5  (L_GPIO[11])

/**
    PIN config for CCNT

    @note For pinmux_init() with PIN_FUNC_CCNT.
*/
#define PIN_CCNT_CFG_NONE               0x0

#define PIN_CCNT_CFG_CCNT_1             0x1          ///< PICNT_1  (P_GPIO[12])
#define PIN_CCNT_CFG_CCNT_2             0x2          ///< PICNT_1  (P_GPIO[18])

#define PIN_CCNT_CFG_CCNT2_1            0x10         ///< PICNT2_1 (P_GPIO[14])
#define PIN_CCNT_CFG_CCNT2_2            0x20         ///< PICNT2_1 (P_GPIO[19])

#define PIN_CCNT_CFG_CCNT3_1            0x100        ///< PICNT3_1 (P_GPIO[17])
#define PIN_CCNT_CFG_CCNT3_2            0x200        ///< PICNT3_1 (P_GPIO[20])

/**
    PIN config for Sensor

    @note For pinmux_init() with PIN_FUNC_SENSOR.
*/
#define PIN_SENSOR_CFG_NONE                    0x0

#define PIN_SENSOR_CFG_12BITS                  0x100         ///< Assign HSI_GPIO0~11 to D0~D11, Assign SN_PXCLK / SN_VD / SN_HD to PXCLK / VD / HD (HSI_GPIO[0..11]/S_GPIO[1..3])
/**
    PIN config for Sensor2

    @note For pinmux_init() with PIN_FUNC_SENSOR2.
*/
#define PIN_SENSOR2_CFG_NONE                    0x0

#define PIN_SENSOR2_CFG_12BITS                  0x100         ///< 12 bits parallel data sensor SN2 (SN2_D0~D11 are HSI_GPIO12~23, PXCLK / VS / HS are S_GPIO7 / S_GPIO8 / S_GPIO9) (HSI_GPIO[12..23]/S_GPIO[7..9])
#define PIN_SENSOR2_CFG_CCIR8BITS_A             0x10000       ///< A_1_YC0~7 are HSI_GPIO16~23 A_1_PXCLK is S_GPIO7 (HSI_GPIO[16..23]/S_GPIO[7])
#define PIN_SENSOR2_CFG_CCIR8BITS_B             0x20000       ///< Assign HSI_GPIO8~15 to B_1_YC0~7, Assign S_GPIO[1] to B_1_PXCLK (HSI_GPIO[8..15]/S_GPIO[1])
#define PIN_SENSOR2_CFG_CCIR8BITS_AB            0x40000       ///< A_1_YC0~7 are HSI_GPIO16~23 A_1_PXCLK is S_GPIO7, VD / HD selected by SN2_CCIR_VSHS,  B_1_YC0~7 are HSI_GPIO8~15, B_1_PXCLK is S_GPIO[1] (HSI_GPIO[16..23]/S_GPIO[7]/HSI_GPIO[8..15]/S_GPIO[1])

#define PIN_SENSOR2_CFG_CCIR16BITS              0x10          ///< CCIR_1_Y0~7 are HSI_GPIO8~15, CCIR_1_C0~7 are HSI_GPIO16~23, CCIR_1_PXCLK is S_GPIO7, VD / HD selected by SN2_CCIR_VSHS (HSI_GPIO[8..23]/S_GPIO[7])
#define PIN_SENSOR2_CFG_CCIR_VSHS              0x100000      ///< SN VD / HD (S_GPIO[8]/S_GPIO[9])

/**
    PIN config for Sensor3

    @note For pinmux_init() with PIN_FUNC_SENSOR3.
*/
#define PIN_SENSOR3_CFG_NONE                    0x0

#define PIN_SENSOR3_CFG_12BITS                  0x100         ///< Assign P_GPIO1~12 to D0~D11, Assign P_GPIO0 / DGPIO11 / DGPIO12 to PXCLK / VD / HD (P_GPIO[0..12]/D_GPIO[11..12])

#define PIN_SENSOR3_CFG_CCIR8BITS_A             0x10000       ///< Assign P_GPIO1~8 to A_2_YC0~7, Assign P_GPIO0 to A_2_PXCLK, VD / HD selected by SN3_CCIR_VSHS (P_GPIO[0..8])

#define PIN_SENSOR3_CFG_CCIR16BITS              0x10          ///< Assign P_GPIO1~8 to CCIR_2_Y0~7, Assign P_GPIO9~12 to CCIR_2_C0~3, Assign DGPIO11~14 to CCIR_2_C4~7, Assign P_GPIO0 to CCIR_2_PXCLK (P_GPIO[0..12]/DGPIO[11..14])
#define PIN_SENSOR3_CFG_CCIR_VSHS               0x100000      ///< SN VD / HD (D_GPIO[11]/D_GPIO[12])

/**
    PIN config for Sensor Misc

    @note For pinmux_init() with PIN_FUNC_SENSORMISC.
*/
#define PIN_SENSORMISC_CFG_NONE                 0x0

#define PIN_SENSORMISC_CFG_SN_MCLK_1            0x1           ///< SN1_MCLK_1 (S_GPIO[0])

#define PIN_SENSORMISC_CFG_SN2_MCLK_1           0x10           ///< SN2_MCLK_1 (S_GPIO[1])
#define PIN_SENSORMISC_CFG_SN2_MCLK_2           0x20           ///< SN2_MCLK_2 (P_GPIO[18])

#define PIN_SENSORMISC_CFG_SN3_MCLK_1           0x100           ///< SN3_MCLK_1 (S_GPIO[6])

#define PIN_SENSORMISC_CFG_SN4_MCLK_1           0x1000           ///< SN4_MCLK_1 (S_GPIO[7])
#define PIN_SENSORMISC_CFG_SN4_MCLK_2           0x2000           ///< SN4_MCLK_2 (P_GPIO[19])

#define PIN_SENSORMISC_CFG_SN_XVSXHS_1          0x10000         ///< SN1_XVSXHS_1 (S_GPIO[2..3])

#define PIN_SENSORMISC_CFG_SN2_XVSXHS_1         0x20000        ///< SN2_XVSXHS_1 (S_GPIO[8..9])

#define PIN_SENSORMISC_CFG_SN3_XVSXHS_1         0x40000        ///< SN3_XVSXHS_1 (P_GPIO[19..20])

#define PIN_SENSORMISC_CFG_SN4_XVSXHS_1         0x80000        ///< SN4_XVSXHS_1 (P_GPIO[21..22])

#define PIN_SENSORMISC_CFG_FLASH_TRIG_IN_1     0x100000      ///< FLASH_TRIG_IN_1 (S_GPIO[14])
#define PIN_SENSORMISC_CFG_FLASH_TRIG_IN_2     0x200000      ///< FLASH_TRIG_IN_2 (S_GPIO[6])

#define PIN_SENSORMISC_CFG_FLASH_TRIG_OUT_1   0x400000      ///< FLASH_TRIG_OUT_1 (S_GPIO[15])
#define PIN_SENSORMISC_CFG_FLASH_TRIG_OUT_2   0x800000      ///< FLASH_TRIG_OUT_2 (S_GPIO[7])

#define PIN_SENSORMISC_CFG_ME_SHUT_IN_1         0x1000000     ///< MSH_1 (S_GPIO[13])
#define PIN_SENSORMISC_CFG_ME_SHUT_IN_2         0x2000000     ///< MSH_2 (S_GPIO[8])

#define PIN_SENSORMISC_CFG_ME_SHUT_OUT_1      0x4000000     ///< MSH_1_SHUT0 / MSH_1_SHUT1 (P_GPIO[0..1])
#define PIN_SENSORMISC_CFG_ME_SHUT_OUT_2      0x8000000     ///< MSH_2_SHUT0 / MSH_2_SHUT1 (D_GPIO[8..9])

/**
    PIN config for Sensor Sync

    @note For pinmux_init() with PIN_FUNC_SENSORSYNC.
*/
#define PIN_SENSORSYNC_CFG_NONE             0x0

#define PIN_SENSORSYNC_CFG_SN_MCLKSRC_SN	0x01
#define PIN_SENSORSYNC_CFG_SN_MCLKSRC_SN2	0x02
#define PIN_SENSORSYNC_CFG_SN_MCLKSRC_SN3	0x04
#define PIN_SENSORSYNC_CFG_SN_MCLKSRC_SN4	0x08

#define PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN   0x010
#define PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN2	0x020
#define PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN3	0x040
#define PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN4	0x080

#define PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN   0x0100
#define PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN2  0x0200
#define PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN3	0x0400
#define PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN4	0x0800

#define PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN	0x01000
#define PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN2	0x02000
#define PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN3	0x04000
#define PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN4	0x08000

#define PIN_SENSORSYNC_CFG_SN_XVSXHSSRC_SN	    0x010000
#define PIN_SENSORSYNC_CFG_SN_XVSXHSSRC_SN2	    0x020000
#define PIN_SENSORSYNC_CFG_SN_XVSXHSSRC_SN3	    0x040000
#define PIN_SENSORSYNC_CFG_SN_XVSXHSSRC_SN4	    0x080000

#define PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN	    0x0100000
#define PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN2	0x0200000
#define PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN3	0x0400000
#define PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN4	0x0800000

#define PIN_SENSORSYNC_CFG_SN3_XVSXHSSRC_SN	    0x01000000
#define PIN_SENSORSYNC_CFG_SN3_XVSXHSSRC_SN2	0x02000000
#define PIN_SENSORSYNC_CFG_SN3_XVSXHSSRC_SN3	0x04000000
#define PIN_SENSORSYNC_CFG_SN3_XVSXHSSRC_SN4	0x08000000

#define PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN	    0x010000000
#define PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN2	0x020000000
#define PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN3	0x040000000
#define PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN4	0x080000000

/**
    PIN config for AUDIO

    @note For pinmux_init() with PIN_FUNC_AUDIO.
*/
#define PIN_AUDIO_CFG_NONE          0x0
#define PIN_AUDIO_CFG_I2S_1         0x1         ///< I2S_1       (PGPIO[17..20])
#define PIN_AUDIO_CFG_I2S_2         0x2         ///< I2S_2       (CGPIO[4..7])

#define PIN_AUDIO_CFG_I2S_MCLK_1    0x4         ///< I2S_MCLK_1  (PGPIO[22])
#define PIN_AUDIO_CFG_I2S_MCLK_2    0x8         ///< I2S_MCLK_2  (DGPIO[7])

#define PIN_AUDIO_CFG_DMIC_1        0x10      ///< (PGPIO[7..9])
#define PIN_AUDIO_CFG_DMIC_2        0x20      ///< (DGPIO[8..10])
#define PIN_AUDIO_CFG_DMIC_3        0x40      ///< (CGPIO[12..14])

#define PIN_AUDIO_CFG_DMIC_DATA0    0x100
#define PIN_AUDIO_CFG_DMIC_DATA1    0x200

#define PIN_AUDIO_CFG_EXT_EAC_MCLK  0x1000     ///< EXT_EAC_MCLK (PGPIO[21])

/**
    PIN config for UART

    @note For pinmux_init() with PIN_FUNC_UART.
*/
#define PIN_UART_CFG_NONE               0x0

#define PIN_UART_CFG_UART_1             0x1          ///< UART_1  (P_GPIO[23..24])

#define PIN_UART_CFG_UART2_1            0x10         ///< UART2_1 (P_GPIO[17..20])
#define PIN_UART_CFG_UART2_2            0x20         ///< UART2_2 (C_GPIO[14..17])

#define PIN_UART_CFG_UART3_1            0x100        ///< UART3_1 (P_GPIO[13..16])
#define PIN_UART_CFG_UART3_2            0x200        ///< UART3_2 (C_GPIO[20..23])
#define PIN_UART_CFG_UART3_4            0x400        ///< UART3_4 (S_GPIO[4]/S_GPIO[6])
#define PIN_UART_CFG_UART3_5            0x800        ///< UART3_5 (C_GPIO[2..3])

#define PIN_UART_CFG_UART4_1            0x1000       ///< UART4_1 (P_GPIO[0..3])
#define PIN_UART_CFG_UART4_2            0x2000       ///< UART4_2 (C_GPIO[4..7])

#define PIN_UART_CFG_UART5_1            0x10000      ///< UART5_1 (P_GPIO[4..7])

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
#define PIN_UARTII_CFG_UART6_1          0x1          ///< UART6_1 (P_GPIO[8..11])

#define PIN_UARTII_CFG_UART6_RTSCTS     0x10000
#define PIN_UARTII_CFG_UART6_DIROE      0x20000

/**
    PIN config for CSI

    @note For pinmux_init() with PIN_FUNC_CSI.
*/
#define PIN_CSI_CFG_NONE             0x0

#define PIN_CSI_CFG_CSI          0x1          ///< (HSI_GPIO[0..5])
#define PIN_CSI_CFG_CSI2         0x10          ///< (HSI_GPIO[6..11])
#define PIN_CSI_CFG_CSI3         0x100          ///< (HSI_GPIO[12..17])
#define PIN_CSI_CFG_CSI4         0x1000          ///< (HSI_GPIO[18..23])

/**
    PIN config for REMOTE

    @note For pinmux_init() with PIN_FUNC_REMOTE.
*/
#define PIN_REMOTE_CFG_NONE             0x0

#define PIN_REMOTE_CFG_REMOTE_1         0x1          ///< (P_GPIO[17])
#define PIN_REMOTE_CFG_REMOTE_2         0x2          ///< (C_GPIO[15])
#define PIN_REMOTE_CFG_REMOTE_3         0x4          ///< (D_GPIO[2])
#define PIN_REMOTE_CFG_REMOTE_EXT_1     0x20          ///< (P_GPIO[9])

/**
    PIN config for SDP

    @note For pinmux_init() with PIN_FUNC_SDP.
*/
#define PIN_SDP_CFG_NONE                0x0

#define PIN_SDP_CFG_SDP_1               0x1          ///< SDP_1 (C_GPIO[12..16])
#define PIN_SDP_CFG_SDP_2               0x2          ///< SDP_2 (P_GPIO[4..8])

/**
    PIN config for SPI

    @note For pinmux_init() with PIN_FUNC_SPI.
*/
#define PIN_SPI_CFG_NONE                0x0

#define PIN_SPI_CFG_SPI_1               0x1          ///< SPI_1  (C_GPIO[18..20])  BUS_WIDTH (C_GPIO[21])
#define PIN_SPI_CFG_SPI_2               0x2          ///< SPI1_2 (S_GPIO[9..11])  BUS_WIDTH (S_GPIO[12])
#define PIN_SPI_CFG_SPI_3               0x4          ///< SPI1_3 (P_GPIO[8..10])    BUS_WIDTH (P_GPIO[11])
#define PIN_SPI_CFG_SPI_BUS_WIDTH       0x8

#define PIN_SPI_CFG_SPI2_1              0x10         ///< SPI2_1 (P_GPIO[13..15])  BUS_WIDTH (P_GPIO[16])
#define PIN_SPI_CFG_SPI2_BUS_WIDTH      0x20

#define PIN_SPI_CFG_SPI3_1              0x100        ///< SPI3_1 (P_GPIO[17..19])  BUS_WIDTH (P_GPIO[20])
#define PIN_SPI_CFG_SPI3_2              0x200        ///< SPI3_2 (C_GPIO[12..14])  BUS_WIDTH (C_GPIO[15])
#define PIN_SPI_CFG_SPI3_BUS_WIDTH      0x400

#define PIN_SPI_CFG_SPI4_1              0x1000       ///< SPI4_1 (P_GPIO[0..3])  BUS_WIDTH (P_GPIO[4])
#define PIN_SPI_CFG_SPI4_2              0x2000       ///< SPI4_2 (C_GPIO[4..6])    BUS_WIDTH (C_GPIO[7])
#define PIN_SPI_CFG_SPI4_BUS_WIDTH      0x4000

#define PIN_SPI_CFG_SPI5_1              0x10000      ///< SPI5_1 (P_GPIO[4..6])  BUS_WIDTH (P_GPIO[7])
#define PIN_SPI_CFG_SPI5_2              0x20000      ///< SPI5_2 (D_GPIO[11..13]]) BUS_WIDTH (D_GPIO[14])
#define PIN_SPI_CFG_SPI5_BUS_WIDTH      0x40000

#define PIN_SPI_CFG_SPI3_RDY_1          0x100000     ///< SPI3RDY_1 (P_GPIO[12])
#define PIN_SPI_CFG_SPI3_RDY_2          0x200000     ///< SPI3RDY_2 (C_GPIO[16])

/**
    PIN config for SIF

    *note For pinmux_init() with PIN_FUNC_SIF.
*/
#define PIN_SIF_CFG_NONE                0x0

#define PIN_SIF_CFG_SIF0_1              0x1          ///< SIF0_1 (S_GPIO[4..6])

#define PIN_SIF_CFG_SIF1_1              0x10         ///< SIF1_1 (S_GPIO[9..11])

#define PIN_SIF_CFG_SIF2_1              0x100        ///< SIF2_1 (L_GPIO[22..24])
#define PIN_SIF_CFG_SIF2_2              0x200        ///< SIF2_2 (P_GPIO[18..20])
#define PIN_SIF_CFG_SIF2_3              0x400       ///< SIF3_3 (D_GPIO[8..10])

#define PIN_SIF_CFG_SIF3_1              0x1000       ///< SIF3_1 (P_GPIO[13..15])

/**
    PIN config for MISC

    @note For pinmux_init() for PIN_FUNC_MISC.
*/
#define PIN_MISC_CFG_NONE                   0x0

#define PIN_MISC_CFG_RTC_CLK_1              0x1          ///< RTC_CLK_1     (P_GPIO[1])

#define PIN_MISC_CFG_SP_CLK_1               0x10         ///< SP_CLK_1  (P_GPIO[17])
#define PIN_MISC_CFG_SP_CLK_2               0x20         ///< SP_CLK_1  (L_GPIO[23])
#define PIN_MISC_CFG_SP_CLK_3               0x40         ///< SP_CLK_1  (D_GPIO[3])

#define PIN_MISC_CFG_SP2_CLK_1              0x100        ///< SP2_CLK_1 (P_GPIO[18])
#define PIN_MISC_CFG_SP2_CLK_2              0x200        ///< SP2_CLK_1 (C_GPIO[9])
#define PIN_MISC_CFG_SP2_CLK_3              0x400        ///< SP2_CLK_1 (D_GPIO[4])

#define PIN_MISC_CFG_ADC                    0x100000     ///< ADC (A_GPIO[0..2])

#define PIN_MISC_CFG_MIPI_SEL_DSI       0x1000000
#define PIN_MISC_CFG_MIPI_SEL_CSI_TX    0x2000000

#define PIN_MISC_CFG_CPU_ICE                0x10000000   ///< CPU_ICE	(D_GPIO[2..6])

/**
    PIN location of LCD

    @note For pinmux_init() with PIN_FUNC_LCD or PIN_FUNC_LCD2.
    For example  you can use {PIN_FUNC_LCD  PINMUX_DISPMUX_SEL_LCD2|PINMUX_LCDMODE_XXX}
    to tell display object that PIN_FUNC_LCD is located on secondary LCD pinmux.
*/
#define PINMUX_DISPMUX_SEL_NONE                 (0x00 << 28)    ///< PINMUX none
#define PINMUX_DISPMUX_SEL_MASK                 (0x01 << 28)

/**
    Pinmux selection for LCD

    @note For pinmux_init() with PIN_FUNC_SEL_LCD
    For example  you can use {PIN_FUNC_SEL_LCD  PINMUX_LCD_SEL_RGB_16BITS | PINMUX_LCD_SEL_DE_ENABLE}
    to tell pinmux driver that the register of primary LCD should be set to RGB 16 bits
    and the register of PLCD_DE should be set.
*/

#define PINMUX_LCD_SEL_GPIO                     0               ///< GPIO

#define PINMUX_LCD_SEL_CCIR656                  1               ///< CCIR-656 8 bits. (L_GPIO[8]) CCIR_DATA_WIDTH 8bit (L_GPIO[0..7])

#define PINMUX_LCD_SEL_CCIR656_16BITS           2               ///< CCIR-656 16 bits. (L_GPIO[8]) CCIR_DATA_WIDTH 16bit (L_GPIO[0..7]/L_GPIO[12..19])

#define PINMUX_LCD_SEL_CCIR601                  3               ///< CCIR-601 8 bits. (L_GPIO[8..10]) CCIR_DATA_WIDTH 8bit (L_GPIO[0..7]) CCIR_FIELD CCIR_DATA_WIDTH = 0(L_GPIO[14]) / CCIR_DATA_WIDTH = 1 (L_GPIO[20])
#define PINMUX_LCD_SEL_CCIR601_16BITS           4               ///< CCIR-601 16 bits.(L_GPIO[8..10]) CCIR_DATA_WIDTH 16bit (L_GPIO[0..7]/L_GPIO[12..19]) CCIR_FIELD CCIR_DATA_WIDTH = 0(L_GPIO[14]) / CCIR_DATA_WIDTH = 1 (L_GPIO[20])

#define PINMUX_LCD_SEL_SERIAL_RGB_6BITS         5               ///< Serial RGB 6 bits. (L_GPIO[2..10])

#define PINMUX_LCD_SEL_SERIAL_RGB_8BITS         6               ///< Serial RGB 8 bits. (L_GPIO[0..10])

#define PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS       7               ///< Serial YCbCr 8 bits. (L_GPIO[0..10])

#define PINMUX_LCD_SEL_PARALLE_RGB565           8               ///< Parallel RGB565. (L_GPIO[0..10]/L_GPIO[12..19])

#define PINMUX_LCD_SEL_PARALLE_RGB666           9               ///< Parallel RGB666. (L_GPIO[0..10]/L_GPIO[12..21])

#define PINMUX_LCD_SEL_PARALLE_RGB888           10              ///< Parallel RGB888. (L_GPIO[0..10]/L_GPIO[12..21]/P_GPIO[13..16]/D_GPIO[0..1])

#define PINMUX_LCD_SEL_RGB_16BITS               11              ///< RGB 16 bits. (L_GPIO[0..10]/L_GPIO[12..19])

#define PINMUX_LCD_SEL_MIPI                     12              ///< MIPI DSI (L_GPIO[22])

#define PINMUX_LCD_SEL_PARALLE_MI_8BITS         13              ///< Parallel MI 8 bits. (L_GPIO[0..7])

#define PINMUX_LCD_SEL_PARALLE_MI_9BITS         14              ///< Parallel MI 9 bits. (L_GPIO[0..7]/L_GPIO[12])

#define PINMUX_LCD_SEL_PARALLE_MI_16BITS        15              ///< Parallel MI 16 bits. (L_GPIO[0..7]/L_GPIO[12..19])

#define PINMUX_LCD_SEL_PARALLE_MI_18BITS        16              ///< Parallel MI 18 bits. (L_GPIO[0..7]/L_GPIO[12..21])

#define PINMUX_LCD_SEL_SERIAL_MI_SDIO           17              ///< Serial MI SDIO bi-direction.
                                                                ///< When PIN_FUNC_SEL_LCD  MPU_CLK/MPU_RS/MPU_CS/MPU_SDIO (L_GPIO[8..10] L_GPIO[7])

#define PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO        18              ///< Serial MI SDI/SDO seperate.
                                                                ///< When PIN_FUNC_SEL_LCD  MPU_CLK/MPU_RS/MPU_CS/MPU_SDO/MPU_SDI (L_GPIO[8..10] L_GPIO[5..6])

#define PINMUX_LCD_SEL_TE_ENABLE                (0x01 << 23)    ///< TE Enable.
                                                                ///< When PIN_FUNC_SEL_LCD  DSI_TE (DSI_GPIO[12])
                                                                ///< When PIN_FUNC_SEL_LCD and PARALLE_MI  MPU_TE (L_GPIO[11])
                                                                ///< When PIN_FUNC_SEL_LCD and SERIAL_MI  MI_TE (L_GPIO[22])

#define PINMUX_LCD_SEL_DE_ENABLE                (0x01 << 24)    ///< DE Enable. (L_GPIO[11])

#define PINMUX_LCD_SEL_HVLD_VVLD                (0x01 << 25)    ///< HVLD/VVLD Enable (For CCIR-601 8 bits). not in 538?

#define PINMUX_LCD_SEL_FIELD                    (0x01 << 26)    ///< FIELD Enable (For CCIR-601).  not in 538?

#define PINMUX_LCD_SEL_NO_HVSYNC                (0x1 << 27)     ///< No HSYNC/VSYNC (backward compatible)

#define PINMUX_LCD_SEL_FEATURE_MSK              (0x3F << 23)

#endif
