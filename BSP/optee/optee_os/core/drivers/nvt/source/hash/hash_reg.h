/*
    HASH driver register definition

    Copyright Novatek Microelectronics Corp. 2021.  All rights reserved.
*/

#ifndef _HASH_REG_H
#define _HASH_REG_H

#include <rcw_macro.h>

REGDEF_OFFSET(HASH_CONFIG_REG,   0x00)
REGDEF_BEGIN(HASH_CONFIG_REG)
    REGDEF_BIT(SWRST                 ,1)
    REGDEF_BIT(HASH_EN               ,1)
    REGDEF_BIT(                      ,2)
    REGDEF_BIT(MODE                  ,2)
    REGDEF_BIT(                      ,2)
    REGDEF_BIT(IV_SEL                ,1)
    REGDEF_BIT(                      ,3)
    REGDEF_BIT(ACCMOD                ,1)
    REGDEF_BIT(DMA_PAD               ,1)
    REGDEF_BIT(                      ,2)
    REGDEF_BIT(BYPASS                ,1)
    REGDEF_BIT(                      ,3)
    REGDEF_BIT(INI                   ,1)
    REGDEF_BIT(IPAD                  ,1)
    REGDEF_BIT(OPAD                  ,1)
    REGDEF_BIT(                      ,9)
REGDEF_END(HASH_CONFIG_REG)

REGDEF_OFFSET(HASH_PADLEN_REG,   0x04)
REGDEF_BEGIN(HASH_PADLEN_REG)
    REGDEF_BIT(HASH_PAD_LEN          ,30)
    REGDEF_BIT(                      ,2)
REGDEF_END(HASH_PADLEN_REG)

REGDEF_OFFSET(HASH_INT_EN_REG,           0x08)
REGDEF_BEGIN(HASH_INT_EN_REG)
    REGDEF_BIT(HASH_BED_EN           ,1)    //0
    REGDEF_BIT(                      ,31)   //31..1
REGDEF_END(HASH_INT_EN_REG)

REGDEF_OFFSET(HASH_INT_STS_REG,          0x0C)
REGDEF_BEGIN(HASH_INT_STS_REG)
    REGDEF_BIT(HASH_BED_STS          ,1)    //0
    REGDEF_BIT(                      ,31)   //31..1
REGDEF_END(HASH_INT_STS_REG)

REGDEF_OFFSET(HASH_KEY0_REG,              0x10)
REGDEF_OFFSET(HASH_KEY1_REG,              0x14)
REGDEF_OFFSET(HASH_KEY2_REG,              0x18)
REGDEF_OFFSET(HASH_KEY3_REG,              0x1C)
REGDEF_OFFSET(HASH_KEY4_REG,              0x20)
REGDEF_OFFSET(HASH_KEY5_REG,              0x24)
REGDEF_OFFSET(HASH_KEY6_REG,              0x28)
REGDEF_OFFSET(HASH_KEY7_REG,              0x2C)
REGDEF_OFFSET(HASH_KEY8_REG,              0x30)
REGDEF_OFFSET(HASH_KEY9_REG,              0x34)
REGDEF_OFFSET(HASH_KEY10_REG,             0x38)
REGDEF_OFFSET(HASH_KEY11_REG,             0x3C)
REGDEF_OFFSET(HASH_KEY12_REG,             0x40)
REGDEF_OFFSET(HASH_KEY13_REG,             0x44)
REGDEF_OFFSET(HASH_KEY14_REG,             0x48)
REGDEF_OFFSET(HASH_KEY15_REG,             0x4C)

REGDEF_OFFSET(HASH_IV0_REG,               0x50)
REGDEF_OFFSET(HASH_IV1_REG,               0x54)
REGDEF_OFFSET(HASH_IV2_REG,               0x58)
REGDEF_OFFSET(HASH_IV3_REG,               0x5C)
REGDEF_OFFSET(HASH_IV4_REG,               0x60)
REGDEF_OFFSET(HASH_IV5_REG,               0x64)
REGDEF_OFFSET(HASH_IV6_REG,               0x68)
REGDEF_OFFSET(HASH_IV7_REG,               0x6C)

REGDEF_OFFSET(HASH_OUTPUT_0_REG,          0x70)
REGDEF_OFFSET(HASH_OUTPUT_1_REG,          0x74)
REGDEF_OFFSET(HASH_OUTPUT_2_REG,          0x78)
REGDEF_OFFSET(HASH_OUTPUT_3_REG,          0x7C)
REGDEF_OFFSET(HASH_OUTPUT_4_REG,          0x80)
REGDEF_OFFSET(HASH_OUTPUT_5_REG,          0x84)
REGDEF_OFFSET(HASH_OUTPUT_6_REG,          0x88)
REGDEF_OFFSET(HASH_OUTPUT_7_REG,          0x8C)

REGDEF_OFFSET(HASH_PIO_INPUT_REG,         0x90)

REGDEF_OFFSET(HASH_DMA_START_ADDR_REG,    0x94)
REGDEF_BEGIN(HASH_DMA_START_ADDR_REG)
    REGDEF_BIT(HASH_SRC_ADDR         ,32)
REGDEF_END(HASH_DMA_START_ADDR_REG)

REGDEF_OFFSET(HASH_DMA_DST_ADDR_REG,      0x98)
REGDEF_BEGIN(HASH_DMA_DST_ADDR_REG)
    REGDEF_BIT(HASH_DST_ADDR         ,32)
REGDEF_END(HASH_DMA_DST_ADDR_REG)

REGDEF_OFFSET(HASH_TRANSSIZE_REG,         0x9C)
REGDEF_BEGIN(HASH_TRANSSIZE_REG)
    REGDEF_BIT(HASH_TRANS_SIZE       ,30)
    REGDEF_BIT(                      ,2)
REGDEF_END(HASH_TRANSSIZE_REG)

REGDEF_OFFSET(HASH_KEY_READ_REG,          0xA0)
REGDEF_BEGIN(HASH_KEY_READ_REG)
    REGDEF_BIT(KEY0                  ,1)
    REGDEF_BIT(KEY1                  ,1)
    REGDEF_BIT(KEY2                  ,1)
    REGDEF_BIT(KEY3                  ,1)
    REGDEF_BIT(KEY4                  ,1)
    REGDEF_BIT(KEY5                  ,1)
    REGDEF_BIT(KEY6                  ,1)
    REGDEF_BIT(KEY7                  ,1)
    REGDEF_BIT(KEY8                  ,1)
    REGDEF_BIT(KEY9                  ,1)
    REGDEF_BIT(KEY10                 ,1)
    REGDEF_BIT(KEY11                 ,1)
    REGDEF_BIT(KEY12                 ,1)
    REGDEF_BIT(KEY13                 ,1)
    REGDEF_BIT(KEY14                 ,1)
    REGDEF_BIT(KEY15                 ,1)
    REGDEF_BIT(                      ,16)
REGDEF_END(HASH_KEY_READ_REG)

REGDEF_OFFSET(HASH_DMA_START_ADDR_H_REG,    0xA4)
REGDEF_BEGIN(HASH_DMA_START_ADDR_H_REG)
    REGDEF_BIT(HASH_SRC_ADDR_H         ,8)
    REGDEF_BIT(                        ,24)
REGDEF_END(HASH_DMA_START_ADDR_H_REG)

REGDEF_OFFSET(HASH_DMA_DST_ADDR_H_REG,      0xA8)
REGDEF_BEGIN(HASH_DMA_DST_ADDR_H_REG)
    REGDEF_BIT(HASH_DST_ADDR_H         ,8)
    REGDEF_BIT(                        ,24)
REGDEF_END(HASH_DMA_DST_ADDR_H_REG)

REGDEF_OFFSET(HASH_AXI_CHANNEL_REG,    0xB0)
REGDEF_BEGIN(HASH_AXI_CHANNEL_REG)
    REGDEF_BIT(                      ,16)
    REGDEF_BIT(MONI_CH_DIS           ,1)
    REGDEF_BIT(                      ,1)
    REGDEF_BIT(DMA_WRAP_EN           ,1)
    REGDEF_BIT(DMA_LOCK_DIS          ,1)
    REGDEF_BIT(                      ,12)
REGDEF_END(HASH_AXI_CHANNEL_REG)


#endif  /* _HASH_REG_H */
