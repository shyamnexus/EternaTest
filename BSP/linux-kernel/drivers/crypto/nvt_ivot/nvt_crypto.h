#ifndef _NVT_CRYPTO_H_
#define _NVT_CRYPTO_H_

typedef enum {
    NVT_CRYPTO_MODE_DES = 0,            ///< Block Size => 64  bits, Key Size => 64  bits, Single Block Cipher
    NVT_CRYPTO_MODE_3DES,               ///< Block Size => 64  bits, Key Size => 192 bits, Single Block Cipher
    NVT_CRYPTO_MODE_AES_128,            ///< Block Size => 128 bits, Key Size => 128 bits, Single Block Cipher
    NVT_CRYPTO_MODE_AES_256,            ///< Block Size => 128 bits, Key Size => 256 bits, Single Block Cipher
    NVT_CRYPTO_MODE_MAX
} NVT_CRYPTO_MODE_T;

typedef enum {
    NVT_CRYPTO_OPMODE_ECB = 0,
    NVT_CRYPTO_OPMODE_CBC,
    NVT_CRYPTO_OPMODE_CFB,
    NVT_CRYPTO_OPMODE_OFB,
    NVT_CRYPTO_OPMODE_CTR,
    NVT_CRYPTO_OPMODE_GCM,
    NVT_CRYPTO_OPMODE_MAX
} NVT_CRYPTO_OPMODE_T;

typedef enum {
    NVT_CRYPTO_TYPE_ENCRYPT = 0,
    NVT_CRYPTO_TYPE_DECRYPT,
    NVT_CRYPTO_TYPE_MAX
} NVT_CRYPTO_TYPE_T;

typedef enum {
	NVT_CRYPTO_CCM_TYPE_NONE = 0,
    NVT_CRYPTO_CCM_TYPE_ENCRYPT,
    NVT_CRYPTO_CCM_TYPE_DECRYPT,
    NVT_CRYPTO_CCM_TYPE_MAX
} NVT_CRYPTO_CCM_TYPE_T;

typedef enum {
	NVT_CRYPTO_KEY_SRC_DESC0 = 0,
    NVT_CRYPTO_KEY_SRC_MANAGAMENT,
    NVT_CRYPTO_KEY_SRC_MAX
} NVT_CRYPTO_KEY_SRC_T;

typedef enum {
    NVT_CRYPTO_DMA_CH_0 = 0,
#ifdef NVT_CRYPTO_MULTI_DMA_CH_SUPPORT
    NVT_CRYPTO_DMA_CH_1,
    NVT_CRYPTO_DMA_CH_2,
    NVT_CRYPTO_DMA_CH_3,
#endif
    NVT_CRYPTO_DMA_CH_MAX
} NVT_CRYPTO_DMA_CH_T;

typedef enum {
    NVT_CRYPTO_STATE_IDLE = 0,
    NVT_CRYPTO_STATE_START,
    NVT_CRYPTO_STATE_BUSY,
    NVT_CRYPTO_STATE_DONE,
} NVT_CRYPTO_STATE_T;

#define NVT_CRYPTO_MAX_IV_SIZE            16             ///< 16  Bytes, 128 bits
#define NVT_CRYPTO_MAX_KEY_SIZE           32             ///< 32  Bytes, 256 bits
#define NVT_CRYPTO_MAX_DMA_BLOCK_NUM      32
#define NVT_CRYPTO_DMA_ALIGN_SIZE         4              ///< dma output length is word alignment

#define NVT_CRYPTO_CFG_REG                0x00           ///< crypto configuration
#define NVT_CRYPTO_CTRL_REG               0x04           ///< crypto control
#define NVT_CRYPTO_INT_ENB_REG            0x08           ///< interrupt enable
#define NVT_CRYPTO_INT_STS_REG            0x0C           ///< interrupt status
#define NVT_CRYPTO_KEY0_REG               0x10           ///< key 0   ~ 31  bit
#define NVT_CRYPTO_KEY1_REG               0x14           ///< key 32  ~ 63  bit
#define NVT_CRYPTO_KEY2_REG               0x18           ///< key 64  ~ 95  bit
#define NVT_CRYPTO_KEY3_REG               0x1C           ///< key 96  ~ 127 bit
#define NVT_CRYPTO_KEY4_REG               0x20           ///< key 128 ~ 159 bit
#define NVT_CRYPTO_KEY5_REG               0x24           ///< key 160 ~ 191 bit
#define NVT_CRYPTO_KEY6_REG               0x28           ///< key 192 ~ 223 bit
#define NVT_CRYPTO_KEY7_REG               0x2C           ///< key 224 ~ 255 bit
#define NVT_CRYPTO_IN0_REG                0x30           ///< input  data  0  ~ 31  bit
#define NVT_CRYPTO_IN1_REG                0x34           ///< input  data 32  ~ 63  bit
#define NVT_CRYPTO_IN2_REG                0x38           ///< input  data 64  ~ 95  bit
#define NVT_CRYPTO_IN3_REG                0x3C           ///< input  data 96  ~ 127 bit
#define NVT_CRYPTO_OUT0_REG               0x40           ///< output data  0  ~ 31  bit
#define NVT_CRYPTO_OUT1_REG               0x44           ///< output data 32  ~ 63  bit
#define NVT_CRYPTO_OUT2_REG               0x48           ///< output data 64  ~ 95  bit
#define NVT_CRYPTO_OUT3_REG               0x4C           ///< output data 96  ~ 127 bit
#define NVT_CRYPTO_DMA0_ADDR_REG          0x50           ///< DMA channel 0 descriptor starting low  address, DMA mode only
#define NVT_CRYPTO_DMA1_ADDR_REG          0x54           ///< DMA channel 1 descriptor starting low  address, DMA mode only
#define NVT_CRYPTO_DMA2_ADDR_REG          0x58           ///< DMA channel 2 descriptor starting low  address, DMA mode only
#define NVT_CRYPTO_DMA3_ADDR_REG          0x5C           ///< DMA channel 3 descriptor starting low  address, DMA mode only
#define NVT_CRYPTO_KEY_READ_REG           0x60           ///< crypto key readable control
#define NVT_CRYPTO_AXIBUS_CTR_REG         0x68           ///< crypto AXI Bus control
#define NVT_CRYPTO_DMA0_ADDR_H_REG        0x70           ///< DMA channel 0 descriptor starting high address, DMA mode only
#define NVT_CRYPTO_DMA1_ADDR_H_REG        0x74           ///< DMA channel 1 descriptor starting high address, DMA mode only
#define NVT_CRYPTO_DMA2_ADDR_H_REG        0x78           ///< DMA channel 2 descriptor starting high address, DMA mode only
#define NVT_CRYPTO_DMA3_ADDR_H_REG        0x7C           ///< DMA channel 3 descriptor starting high address, DMA mode only

#endif  /* _NVT_CRYPTO_H_ */
