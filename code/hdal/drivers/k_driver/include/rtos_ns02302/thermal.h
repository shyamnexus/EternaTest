/*
    Thermal driver header file

    Thermal driver header file.

    @file       rsa.h
    @ingroup    mIDrvMisc_RSA
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.
*/

#ifndef _RSA_H
#define _RSA_H

#include <kwrap/nvt_type.h>

/**
    @addtogroup mIDrvMisc_RSA
*/
/**
    Rsa engine clock rate
*/

typedef enum
{
    RSA_CLOCK_240MHz = 0x00,         ///< Select RSA clock rate 240 MHz
    RSA_CLOCK_320MHz,                ///< Select RSA clock rate 320 MHz
    RSA_CLOCK_Reserved,              ///< Select RSA clock rate Reserved
    RSA_CLOCK_PLL13,                 ///< Select RSA clock rate PLL13(for IPP/H265)

    RSA_CLOCK_RATE_NUM,
    ENUM_DUMMY4WORD(RSA_CLOCK_RATE)
} RSA_CLOCK_RATE;


//@{
/**
    Rsa engine Key Width
*/
typedef enum
{
    RSA_KEY_256 = 0x00,              ///< Select RSA key width for 256 bit
    RSA_KEY_512,                     ///< Select RSA key width for 512 bit
    RSA_KEY_1024,                    ///< Select RSA key width for 1024 bit
    RSA_KEY_2048,                    ///< Select RSA key width for 2048 bit
    RSA_KEY_4096,                    ///< Select RSA key width for 4096 bit

    RSA_KEY_NUM,
    ENUM_DUMMY4WORD(RSA_KEY)
} RSA_KEY;

/**
    Rsa engine Key Width
*/
typedef enum
{
    RSA_MODE_NORMAL = 0x00,           ///< Select RSA mode for encryption or decryption
    RSA_MODE_CRC_KEY_N,               ///< Select RSA mode for Key N CRC check
    RSA_MODE_CRC_KEY_ED = 0x03,       ///< Select RSA mode for Key E/D CRC check

    RSA_MODE_NUM,
    ENUM_DUMMY4WORD(RSA_MODE_CFG)
} RSA_MODE_CFG;


/**
    Rsa engine CRC32 Configuration
*/
typedef enum
{
    RSA_CRC_INIT = 0x00,               ///< Select RSA mode for encryption or decryption
    RSA_CRC_POLY,                     ///< Select RSA mode for Key N CRC check

    RSA_CRC_NUM,
    ENUM_DUMMY4WORD(RSA_CRC)
} RSA_CRC;


/**
    Rsa order
*/
typedef enum
{
    RSA_ORDER_LSB_FIRST = 0x00,        ///< Select RSA order for lsb byte first
    RSA_ORDER_MSB_FIRST,               ///< Select RSA order for msb byte first

    RSA_ORDER_NUM,
    ENUM_DUMMY4WORD(RSA_ORDER_CFG)
} RSA_ORDER_CFG;


/**
    Rsa configuration identifier

    @note For Rsa_setConfig()
*/
typedef enum
{
    RSA_CONFIG_ID_CLOCK_RATE,           ///< Select HASH engine internal clock rate usage
                                        ///< Context can be any of:
                                        ///< - @b RSA_CLOCK_240MHz:     Internal clock rate 240MHz
                                        ///< - @b RSA_CLOCK_320MHz:     Internal clock rate 320MHz
                                        ///< - @b RSA_CLOCK_PLL13:      Internal clock rate PLL13
    RSA_CONFIG_ID_KEY_WIDTH,            ///< Select HASH engine internal clock rate usage
                                        ///< Context can be any of:
                                        ///< - @b RSA_KEY_256
                                        ///< - @b RSA_KEY_512
                                        ///< - @b RSA_KEY_1024
                                        ///< - @b RSA_KEY_2048
                                        ///< - @b RSA_KEY_4096
    RSA_CONFIG_ID_MODE,                 ///< Select HASH engine internal clock rate usage
                                        ///< Context can be any of:
                                        ///< - @b RSA_MODE_NORMAL
                                        ///< - @b RSA_MODE_CRC_KEY_N
                                        ///< - @b RSA_MODE_CRC_KEY_ED
    RSA_CONFIG_ID_CRC32,                ///< Select HASH engine CRC32 Initial Value and Polynoms
                                        ///< Context can be any of:
                                        ///< - @b RSA_CRC_INIT
                                        ///< - @b RSA_CRC_POLY
    RSA_CONFIG_ID_REORDER,              ///< Select HASH engine internal clock rate usage
                                        ///< Context can be any of:
    ENUM_DUMMY4WORD(RSA_CONFIG_ID)
} RSA_CONFIG_ID;


extern ER 		thermal_open(void);
extern ER 		thermal_close(void);
extern INT32 	thermal_get_avg_value(void);
extern ER 		thermal_enable_debug_sel(UINT32 sel);
extern ER 		thermal_get_direct_value(UINT32 * ts_value, UINT32 * ad_ts_value);

#define _EMULATION_ 1

#if (_EMULATION_ == ENABLE)

ER    thermalTest_setTS_PD(BOOL uiConfig);
ER    thermalTest_setTS_EN(BOOL uiConfig);
ER    thermalTest_setTS_CLKEN(BOOL uiConfig);
ER    thermalTest_setTS_CLK_DIV(UINT32 uiConfig);
ER    thermalTest_setTS_AVG_NO(UINT32 uiConfig);
ER    thermalTest_setTS_Mask_NO(UINT32 uiConfig);
#endif

//@}

#endif // _RSA_H



