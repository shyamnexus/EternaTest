/*
    RSA driver header file

    Copyright Novatek Microelectronics Corp. 2021.  All rights reserved.
*/

#ifndef _RSA_H
#define _RSA_H

#include <kwrap/nvt_type.h>

/**
    Rsa engine clock rate
*/
typedef enum {
#if defined(_BSP_NA51090_)
    RSA_CLOCK_PLL2 = 0,                 ///< Select RSA clock rate PLL2 (400MHz)
    RSA_CLOCK_PLL13,                    ///< Select RSA clock rate PLL13(350MHz)
#else
    RSA_CLOCK_240MHz = 0,               ///< Select RSA clock rate 240 MHz
    RSA_CLOCK_320MHz,                   ///< Select RSA clock rate 320 MHz
    RSA_CLOCK_Reserved,                 ///< Select RSA clock rate Reserved
    RSA_CLOCK_PLL13,                    ///< Select RSA clock rate PLL13
#endif

    RSA_CLOCK_RATE_NUM,
    ENUM_DUMMY4WORD(RSA_CLOCK_RATE)
} RSA_CLOCK_RATE;

/**
    Rsa engine Key Width
*/
typedef enum {
    RSA_KEY_256 = 0x00,                 ///< Select RSA key width for 256 bit
    RSA_KEY_512,                        ///< Select RSA key width for 512 bit
    RSA_KEY_1024,                       ///< Select RSA key width for 1024 bit
    RSA_KEY_2048,                       ///< Select RSA key width for 2048 bit
    RSA_KEY_4096,                       ///< Select RSA key width for 4096 bit

    RSA_KEY_NUM,
    ENUM_DUMMY4WORD(RSA_KEY)
} RSA_KEY;

/**
    Rsa engine Key Width
*/
typedef enum {
    RSA_MODE_NORMAL = 0,                ///< Select RSA mode for encryption or decryption
    RSA_MODE_CRC_KEY_ED,                ///< Select RSA mode for Key E/D CRC check
    RSA_MODE_CRC_KEY_N = 3,             ///< Select RSA mode for Key N   CRC check

    RSA_MODE_NUM,
    ENUM_DUMMY4WORD(RSA_MODE_CFG)
} RSA_MODE_CFG;

/**
    Rsa order
*/
typedef enum {
    RSA_ORDER_LSB_FIRST = 0,            ///< Select RSA order for LSB byte first
    RSA_ORDER_MSB_FIRST,                ///< Select RSA order for MSB byte first

    RSA_ORDER_NUM,
    ENUM_DUMMY4WORD(RSA_ORDER_CFG)
} RSA_ORDER_CFG;


/**
    Rsa configuration identifier

    @note For Rsa_setConfig()
*/
typedef enum {
    RSA_CONFIG_ID_CLOCK_RATE,           ///< Select RSA engine internal clock rate usage
                                        ///< Context can be any of:
                                        ///< - @b RSA_CLOCK_PLL2 :     Internal clock rate 400MHz
                                        ///< - @b RSA_CLOCK_PLL13:     Internal clock rate 350MHz
    RSA_CONFIG_ID_KEY_WIDTH,            ///< Select RSA engine key width usage
                                        ///< Context can be any of:
                                        ///< - @b RSA_KEY_256
                                        ///< - @b RSA_KEY_512
                                        ///< - @b RSA_KEY_1024
                                        ///< - @b RSA_KEY_2048
                                        ///< - @b RSA_KEY_4096
    RSA_CONFIG_ID_MODE,                 ///< Select RSA engine operation mode usage
                                        ///< Context can be any of:
                                        ///< - @b RSA_MODE_NORMAL
                                        ///< - @b RSA_MODE_CRC_KEY_ED
                                        ///< - @b RSA_MODE_CRC_KEY_N

    RSA_CONFIG_ID_CRC32_POLY_INIT,      ///< Setup RSA CRC32 polynomial initial value

    RSA_CONFIG_ID_CRC32_POLY_PARAM,     ///< Setup RSA CRC32 polynomial parameter => P(x) = x32 + bit31*x31 + �K + bit16*x16 + �K + bit2*x2+bit1*x1 + bit0*x0

    RSA_CONFIG_ID_REORDER,              ///< Select RSA engine byte order usage
                                        ///< Context can be any of:
                                        ///< - @b RSA_ORDER_LSB_FIRST
                                        ///< - @b RSA_ORDER_MSB_FIRST
    ENUM_DUMMY4WORD(RSA_CONFIG_ID)
} RSA_CONFIG_ID;

/**********************************************************************************************
 * Public Function Prototype
 **********************************************************************************************/
extern ER   rsa_open(void);
extern ER   rsa_close(void);
extern ER   rsa_setConfig(RSA_CONFIG_ID cfgid, UINT32 cfgvalue);
extern void rsa_setkey_n(UINT32 *key, UINT32 len, UINT32 msb_first);
extern void rsa_setkey_ed(UINT32 *key, UINT32 len, UINT32 msb_first);
extern void rsa_pio_enable(UINT32 *data, UINT32 len, UINT32 msb_first);
extern void rsa_getOutput(UINT32 *Output, UINT32 len);

#endif /* _RSA_H */
