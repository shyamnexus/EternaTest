#ifndef _NVT_RSA_H_
#define _NVT_RSA_H_

typedef enum {
    NVT_RSA_MODE_NORMAL = 0,                          ///< normal mode, encryption or decryption
    NVT_RSA_MODE_CRC_KEY_ED,                          ///< E/D key check by crc32
    NVT_RSA_MODE_CRC_KEY_N = 3,                       ///< N key check by crc32
    NVT_RSA_MODE_MAX
} NVT_RSA_MODE_T;

typedef enum {
    NVT_RSA_KEY_256 = 0,                              ///< key width 256  bits
    NVT_RSA_KEY_512,                                  ///< key width 512  bits
    NVT_RSA_KEY_1024,                                 ///< key width 1024 bits
    NVT_RSA_KEY_2048,                                 ///< key width 2048 bits
    NVT_RSA_KEY_4096,                                 ///< key width 4096 bits, NOT support in NA51055
    NVT_RSA_KEY_MAX
} NVT_RSA_KEY_T;

#define NVT_RSA_BUFFER_LEN             512            ///< 512 bytes

#define NVT_RSA_CFG_REG                0x00           ///< configuration
#define NVT_RSA_CTRL_REG               0x04           ///< control
#define NVT_RSA_INT_ENB_REG            0x08           ///< interrupt enable
#define NVT_RSA_INT_STS_REG            0x0C           ///< interrupt status
#define NVT_RSA_KEY_N_REG              0x10           ///< key N register
#define NVT_RSA_KEY_N_ADDR_REG         0x14           ///< key N config ram address
#define NVT_RSA_KEY_ED_REG             0x18           ///< key E/D register
#define NVT_RSA_KEY_ED_ADDR_REG        0x1C           ///< key E/D config ram address
#define NVT_RSA_DATA_REG               0x20           ///< input data register
#define NVT_RSA_DATA_ADDR_REG          0x24           ///< input data config ram address
#define NVT_RSA_KEY_READ_REG           0x28           ///< rsa key readable control
#define NVT_RSA_CRC32_DEFAULT_REG      0x30           ///< crc32 default value
#define NVT_RSA_CRC32_POLY_REG         0x34           ///< crc32 polynomial parameter
#define NVT_RSA_CRC32_OUTPUT_REG       0x38           ///< rsa key crc32 result

#endif  /* _NVT_RSA_H_ */
