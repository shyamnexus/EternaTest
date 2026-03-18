/*
   Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.

   SPDX-License-Identifier: BSD-2-Clause.
 */

#ifndef __NVT_CRYPTO_TA_H__
#define __NVT_CRYPTO_TA_H__

/* UUID of the AES example trusted application */
#define TA_NVT_CRYPTO_UUID \
	{ 0x5901ceec, 0x18f5, 0x43f7, \
		{ 0xa4, 0xde, 0xb5, 0x72, 0xd2, 0xd0, 0x78, 0xf4 } }


/* Chipher */
#define TA_ALGO_AES_ECB			      0x10000001
#define TA_ALGO_AES_CBC			      0x10000002
#define TA_ALGO_AES_CFB			      0x10000003
#define TA_ALGO_AES_OFB			      0x10000004
#define TA_ALGO_AES_CTR			      0x10000005
#define TA_ALGO_AES_GCM			      0x10000006
#define TA_ALGO_DES_ECB_NOPAD         0x10000010
#define TA_ALGO_DES_CBC_NOPAD         0x10000011
#define TA_ALGO_DES3_ECB_NOPAD        0x10000012
#define TA_ALGO_DES3_CBC_NOPAD        0x10000013

/* HASH */
#define TA_ALGO_SHA1                  0x20000001
#define TA_ALGO_SHA256                0x20000002
#define TA_ALGO_HMAC_SHA1             0x20000003
#define TA_ALGO_HMAC_SHA256           0x20000004




#define TA_AES_SIZE_128BIT		(128 / 8)
#define TA_AES_SIZE_256BIT		(256 / 8)

#define TA_AES_MODE_ENCODE		1
#define TA_AES_MODE_DECODE		0

/*
 * TA_AES_CMD_SET_KEY - Allocate resources for the AES ciphering
 * param[0] (memref) key data, size shall equal key length
 * param[1] unused
 * param[2] unused
 * param[3] unused
 */
#define MAKEFOURCC(ch0, ch1, ch2, ch3) ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) | ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24))   ///<

#define NVT_OTP_KEY_TAG         MAKEFOURCC('N', 'O', 'T', 'P')  ///< Nova OTP key tag
#define NVT_OTP_KEY_KM_KEY_SET0 0x00000000
#define NVT_OTP_KEY_KM_KEY_SET1 0x00000001
#define NVT_OTP_KEY_KM_KEY_SET2 0x00000002
#define NVT_OTP_KEY_KM_KEY_SET3 0x00000003
#define NVT_OTP_KEY_KM_KEY_SET4 0x00000004


/*
 * TA_AES_CMD_PREPARE - Allocate resources for the AES ciphering
 * param[0] (value) a: TA_AES_ALGO_xxx, b: unused
 * param[1] (value) a: key size in bytes, b: unused
 * param[2] (value) a: TA_AES_MODE_ENCODE/_DECODE, b: unused
 * param[3] unused
 */
#define TA_AES_CMD_PREPARE		0

/*
 * TA_AES_CMD_SET_KEY - Allocate resources for the AES ciphering
 * param[0] (memref) key data, size shall equal key length
 * param[1] unused
 * param[2] unused
 * param[3] unused
 */
#define TA_AES_CMD_SET_KEY		1

/*
 * TA_AES_CMD_SET_IV - reset IV
 * param[0] (memref) initial vector, size shall equal block length
 * param[1] unused
 * param[2] unused
 * param[3] unused
 */
#define TA_AES_CMD_SET_IV		2

/*
 * TA_AES_CMD_CIPHER - Cipher input buffer into output buffer
 * param[0] (memref) input buffer
 * param[1] (memref) output buffer (shall be bigger than input buffer)
 * param[2] unused
 * param[3] unused
 */
#define TA_AES_CMD_CIPHER		3





/*
 * void TEE_DigestUpdate(TEE_OperationHandle operation,
 *              void *chunk, size_t chunkSize);
 * in       params[0].value.a = operation
 * in       params[1].memref = chunk
 */
#define TA_CRYPT_CMD_DIGEST_UPDATE      12

/*
 * TEE_Result TEE_DigestDoFinal(TEE_OperationHandle operation,
 *              const void *chunk, size_t chunkLen,
 *              void *hash, size_t *hashLen);
 * in       params[0].value.a = operation
 * in       params[1].memref = chunk
 * out      params[2].memref = hash
 */
#define TA_CRYPT_CMD_DIGEST_DO_FINAL    13

#endif /* __NVT_CRYPTO_TA_H__ */
