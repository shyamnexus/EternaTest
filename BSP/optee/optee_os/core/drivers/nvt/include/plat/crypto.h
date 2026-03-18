/*
    Crypto driver header file

    Copyright Novatek Microelectronics Corp. 2021.  All rights reserved.
*/

#ifndef _CRYPTO_H
#define _CRYPTO_H

#include <kwrap/nvt_type.h>

#if defined(_BSP_NA51090_) || defined(_BSP_NS02201_) || defined(_BSP_NS02302_)
#define CRYPTO_DMA_36BIT_SUPPORT
#endif

/**
    Crypto engine crypto mode
*/
typedef enum
{
    CRYPTO_MODE_DES = 0,                      ///< Select Crypto mode DES
    CRYPTO_MODE_3DES,                         ///< Select Crypto mode 3DES
    CRYPTO_MODE_AES128,                       ///< Select Crypto mode AES-128
    CRYPTO_MODE_AES256,                       ///< Select Crypto mode AES-256
    CRYPTO_MODE_NUM,
    ENUM_DUMMY4WORD(CRYPTO_MODE)
} CRYPTO_MODE;


/**
    Crypto engine crypto Operating mode
*/
typedef enum
{
    CRYPTO_OPMODE_EBC = 0,                     ///< Select Crypto opmode EBC
    CRYPTO_OPMODE_CBC,                         ///< Select Crypto opmode CBC
    CRYPTO_OPMODE_CFB,                         ///< Select Crypto opmode CFB
    CRYPTO_OPMODE_OFB,                         ///< Select Crypto opmode OFB
    CRYPTO_OPMODE_CTR,                         ///< Select Crypto opmode CTR
    CRYPTO_OPMODE_GCM,                         ///< Select Crypto opmode GCM
    CRYPTO_OPMODE_NUM,
    ENUM_DUMMY4WORD(CRYPTO_OPMODE)
} CRYPTO_OPMODE;


/**
    Crypto engine encrypt or decrypt
*/
typedef enum
{
    CRYPTO_TYPE_ENCRYPT = 0,                   ///< Select Crypto engine encrypt
    CRYPTO_TYPE_DECRYPT,                       ///< Select Crypto engine decrypt
    CRYPTO_TYPE_NUM,

    ENUM_DUMMY4WORD(CRYPTO_TYPE)
} CRYPTO_TYPE;


/**
    Crypto engine key source
*/
typedef enum
{
    CRYPTO_KEY_NORMAL = 0,                     ///< Select Crypto engine from Descriptor 0 Key Field.
    CRYPTO_KEY_KM,                             ///< Select Crypto engine from Key Managament.
    CRYPTO_KEY_NUM,

    ENUM_DUMMY4WORD(CRYPTO_KEY)
} CRYPTO_KEY;


/**
    Crypto engine clock rate
*/
typedef enum
{
#if defined(_BSP_NA51090_)
    CRYPTO_CLOCK_PLL2 = 0,                     ///< Select Crypto clock rate PLL2 (400MHz)
    CRYPTO_CLOCK_PLL13,                        ///< Select Crypto clock rate PLL13(350MHz)
#else
    CRYPTO_CLOCK_240MHz = 0,                   ///< Select Crypto clock rate 240 MHz
    CRYPTO_CLOCK_320MHz,                       ///< Select Crypto clock rate 320 MHz
    CRYPTO_CLOCK_Reserved,                     ///< Select Crypto clock rate Reserved
    CRYPTO_CLOCK_PLL13,
#endif

    CRYPTO_CLOCK_RATE_NUM,
    ENUM_DUMMY4WORD(CRYPTO_CLOCK_RATE)
} CRYPTO_CLOCK_RATE;

/**
    Crypto configuration identifier

    @note For crypto_setConfig()
*/
typedef enum
{
    CRYPTO_CONFIG_ID_CLOCK_RATE,        ///< Select Crypto engine internal clock rate usage
                                        ///< Context can be any of:
                                        ///< - @b CRYPTO_CLOCK_PLL2 :  Internal clock rate 400MHz
                                        ///< - @b CRYPTO_CLOCK_PLL13:  Internal clock rate 350MHz
    CRYPTO_CONFIG_ID_MODE,              ///< Select Crypto engine Algorithm
                                        ///< Context can be any of:
                                        ///< - @b CRYPTO_MODE_DES:      Select Crypto mode DES
                                        ///< - @b CRYPTO_MODE_3DES:     Select Crypto mode 3DES
                                        ///< - @b CRYPTO_MODE_AES128:   Select Crypto mode AES128
                                        ///< - @b CRYPTO_MODE_AES256:   Select Crypto mode AES256
    CRYPTO_CONFIG_ID_OPMODE,            ///< Select Crypto engine Operating mode
                                        ///< Context can be any of:
                                        ///< - @b CRYPTO_OPMODE_EBC:    Select Crypto opmode EBC
                                        ///< - @b CRYPTO_OPMODE_CBC:    Select Crypto opmode CBC
                                        ///< - @b CRYPTO_OPMODE_CFB:    Select Crypto opmode CFB
                                        ///< - @b CRYPTO_OPMODE_OFB:    Select Crypto opmode OFB
                                        ///< - @b CRYPTO_OPMODE_CTR:    Select Crypto opmode CTR
                                        ///< - @b CRYPTO_OPMODE_GCM:    Select Crypto opmode GCM
                                        ///< - @b CRYPTO_OPMODE_HwCopy: Select Crypto bypass mode
    CRYPTO_CONFIG_ID_TYPE,              ///< Select Crypto engine encrypt or decrypt
                                        ///< Context can be any of:
                                        ///< - @b CRYPTO_TYPE_ENCRYPT:  Select Crypto engine encrypt
                                        ///< - @b CRYPTO_TYPE_DECRYPT:  Select Crypto engine decrypt

    CRYPTO_CONFIG_ID_KEY,               ///< Select Crypto engine key source
                                        ///< Context can be any of:
                                        ///< - @b CRYPTO_KEY_NORMAL:    Select Crypto engine from Descriptor 0 Key Field.
                                        ///< - @b CRYPTO_KEY_KM:        Select Crypto engine from Key Managament.

    ENUM_DUMMY4WORD(CRYPTO_CONFIG_ID)
} CRYPTO_CONFIG_ID;


typedef struct {
    CRYPTO_MODE     uiMode;
    CRYPTO_OPMODE   uiOPMode;
    CRYPTO_TYPE     uiType;
    UINT8           ucInput[16];
    UINT8           ucKey[32];
    UINT8           ucOutput[16];
} CRYPT_SET, *pCRYPT_SET;

typedef struct CRYPT_SEG_DES {
    uintptr_t       src_addr;
    uintptr_t       dst_addr;
    UINT32          len;
    BOOL            last_block;
    BOOL            block_inten;
    BOOL            non_flush;

    struct CRYPT_SEG_DES *pNext;
} CRYPT_SEG_DES, *pCRYPT_SEG_DES;

typedef struct {
    CRYPTO_TYPE     type;
    CRYPTO_MODE     mode;
    CRYPTO_OPMODE   opmode;
    BOOL            key_src;
    BOOL            get_s0;
    UINT8           key[32];
    UINT8           iv[16];
    UINT8           cnt[16];
    UINT8           cv[16];
    UINT8           s0[16];
    UINT8           ghash[16];
    UINT8           tag[16];
} CRYPT_DMA_SET, *pCRYPT_DMA_SET;

#define SCE_DMA_CH0                      (0)
#define SCE_DMA_CH1                      (1)
#define SCE_DMA_CH2                      (2)
#define SCE_DMA_CH3                      (3)
#define SCE_DMA_COUNT                    (4)

typedef struct _CRYPT_OP {
    CRYPTO_OPMODE   op_mode;            ///< Operation Mode
    CRYPTO_TYPE     en_de_crypt;        ///< Encrypt or decrypt (CRYPTO_ENCRYPT or CRYPTO_DECRYPT)
    uintptr_t       src_addr;           ///< Source address
    uintptr_t       dst_addr;           ///< Destination address
    UINT32          length;             ///< length
} CRYPT_OP;

/**********************************************************************************************
 * Public Function Prototype
 **********************************************************************************************/
extern ER   crypto_open(void);
extern ER   crypto_close(void);
extern ER   crypto_setConfig(CRYPTO_CONFIG_ID configID, UINT32 configContext);
extern void crypto_setKey(UINT8 *Key);
extern void crypto_setInput(UINT8 *Input);
extern void crypto_getOutput(UINT8 *Output);
extern void crypto_pio_enable(void);
extern void crypto_dma_setDesTab(UINT32 id, pCRYPT_DMA_SET ptrDmaCrypt, pCRYPT_SEG_DES ptrSeg, UINT32 uiDesNum);

#endif /* _CRYPTO_H */
