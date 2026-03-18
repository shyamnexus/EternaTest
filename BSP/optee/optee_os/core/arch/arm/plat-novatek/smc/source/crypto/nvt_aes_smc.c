#include <kernel/boot.h>
#include <string.h>
#include <tee/cache.h>
#include <sm/optee_smc.h>
#include <mm/core_mmu.h>
#include <mm/core_memprot.h>
#include <crypto/crypto.h>
#include <optee_msg.h>
#include <nvt_types.h>

#include "../../include/crypto/nvt_aes_smc.h"
#include "../../include/nvt_smc_util.h"

#define NVT_AES_KEY_MIN_FIELD   0
#define NVT_AES_KEY_MAX_FIELD   4

/*
    key_field define
    define from  lib/source/crypto/nvt_ecb.c
*/
#define NVT_OTP_KEY_KM_KEY_SET0 0x00000000
#define NVT_OTP_KEY_KM_KEY_SET1 0x00000001
#define NVT_OTP_KEY_KM_KEY_SET2 0x00000002
#define NVT_OTP_KEY_KM_KEY_SET3 0x00000003
#define NVT_OTP_KEY_KM_KEY_SET4 0x00000004

///< Nova OTP key tag  sync from core/arch/arm/plat-novatek/lib/source/crypto/nvt_ecb.c
#define NVT_OTP_KEY_TAG         MAKEFOURCC('N', 'O', 'T', 'P')

static void smc_aes_transfer_aes_mode(unsigned int input_data, unsigned int *output_data)
{
EMSG("4\n");
	switch (input_data) {
		case NVT_SMC_AES_MODE_CBC:
			*output_data = TEE_ALG_AES_CBC_NOPAD;
			break;
		case NVT_SMC_AES_MODE_ECB:
			EMSG("aes ECB not support now, change to CBC\r\n");
			*output_data = TEE_ALG_AES_CBC_NOPAD;
			break;
		default:
			EMSG("aes mode (%d) fail, using CBC mode\r\n", input_data);
			*output_data = TEE_ALG_AES_CBC_NOPAD;
			break;
	}
}

static void smc_aes_transfer_crypto(unsigned int input_data, unsigned int *output_data)
{
EMSG("3\n");
	switch (input_data) {
		case NVT_SMC_AES_CRYPTO_ENCRYPTION:
			*output_data = TEE_MODE_ENCRYPT;
			break;
		case NVT_SMC_AES_CRYPTO_DECRYPTION:
			*output_data = TEE_MODE_DECRYPT;
			break;
		default:
			EMSG("smc_aes_transfer_crypto type:%d error, change to TEE_MODE_DECRYPT\r\n", input_data);
			*output_data = TEE_MODE_DECRYPT;
			break;
	}
}

static void smc_aes_transfer_field(unsigned int input_data, int *output_data)
{
EMSG("2\n");
	switch (input_data) {
		case 0:
			*output_data = NVT_OTP_KEY_KM_KEY_SET0;
			break;
		case 1:
			*output_data = NVT_OTP_KEY_KM_KEY_SET1;
			break;
		case 2:
			*output_data = NVT_OTP_KEY_KM_KEY_SET2;
			break;
		case 3:
			*output_data = NVT_OTP_KEY_KM_KEY_SET3;
			break;
		case 4:
			*output_data = NVT_OTP_KEY_KM_KEY_SET4;
			break;
		default:
			EMSG("input data error(%d)\r\n", input_data);
			*output_data = -1;
			break;
	}
}

static int smc_aes_alloc(NVT_SMC_AES_DATA *aes_data)
{
	int ret = 0;
	unsigned int aes_mode = 0;
EMSG("9\n");
	smc_aes_transfer_aes_mode(aes_data->aes_mode, &aes_mode);

	ret = crypto_cipher_alloc_ctx(&aes_data->ctx, aes_mode);
	if (ret != TEE_SUCCESS) {
		EMSG("crypto_cipher_alloc_ctx fail ret:%x\n", ret);
		return -1;
	}

	ret = cache_operation(TEE_CACHEFLUSH, aes_data, sizeof(NVT_SMC_AES_DATA));
	if (ret != TEE_SUCCESS) {
		EMSG("cache_operation TEE_CACHEFLUSH fail ret:%x\n", ret);
	}

	return 0;
}

static int smc_aes_init(NVT_SMC_AES_DATA *aes_data)
{
	int ret   = 0;
	int field = 0;
	unsigned char *key_buf   = NULL;
	unsigned int  aes_mode   = 0;
	unsigned int  aes_crypto = 0;
	unsigned int  efuse_key_buf[4];
EMSG("5\n");
	smc_aes_transfer_aes_mode(aes_data->aes_mode,  &aes_mode);
	smc_aes_transfer_crypto(aes_data->crypto_type, &aes_crypto);

	if (aes_data->efuse_field == -1) {
		//using aes key in buffer
		EMSG("use key from buffer\r\n");

		if (aes_data->key_size < 16) {
			EMSG("key size (%d) too small\r\n", (int)aes_data->key_size);
			return -1;
		}

		key_buf = (unsigned char *)nvt_smc_pa_to_va((uintptr_t)aes_data->key_buf, aes_data->key_size);
		if (!key_buf) {
			EMSG("key buf pa=0x%08lx size=%d to va fail\r\n", (uintptr_t)aes_data->key_buf, (int)aes_data->key_size);
			return -1;
		}
		ret = crypto_cipher_init(aes_data->ctx, aes_crypto, key_buf, aes_data->key_size, NULL, 0, aes_data->IV, 16);
	}
	else if ((aes_data->efuse_field <= NVT_AES_KEY_MAX_FIELD) && (aes_data->efuse_field >= NVT_AES_KEY_MIN_FIELD)) {
		//using aes key in efuse
		EMSG("use key from efuse\r\n");

		smc_aes_transfer_field(aes_data->efuse_field, &field);
		if (field < 0) {
			EMSG("smc_aes_transfer_field fail (%d)\r\n", field);
			return -1;
		}

		efuse_key_buf[0] = NVT_OTP_KEY_TAG;
		efuse_key_buf[1] = field;
		efuse_key_buf[2] = NVT_OTP_KEY_TAG;
		efuse_key_buf[3] = NVT_OTP_KEY_TAG;
		ret = crypto_cipher_init(aes_data->ctx, aes_crypto, (unsigned char *)efuse_key_buf, 16, NULL, 0, aes_data->IV, 16);
	}
	else {
		EMSG("aes_data->efuse_field (%d) error\r\n", aes_data->efuse_field);
		return -1;
	}

	if (ret != TEE_SUCCESS) {
		EMSG("crypto_cipher_init fail ret:%x\n", ret);
		return -1;
	}

	return 0;
}

static int smc_aes_update(NVT_SMC_AES_DATA *aes_data)
{
	int ret = 0;
	unsigned char *output_data = NULL;
	unsigned char *input_data  = NULL;
	unsigned int   aes_mode    = 0;
	unsigned int   aes_crypto  = 0;
EMSG("1\n");
	smc_aes_transfer_aes_mode(aes_data->aes_mode,  &aes_mode);
	smc_aes_transfer_crypto(aes_data->crypto_type, &aes_crypto);

	if (aes_data->output_size < 16) {
		EMSG("output size (%u) toot small, need bigger than 16 bytes\r\n", aes_data->output_size);
		return -1;
	}

	if ((aes_data->input_size & 0xf ) != 0) {
		EMSG("intput size (%u) not align 16\r\n", aes_data->input_size);
		return -1;
	}

	output_data = (unsigned char *)nvt_smc_pa_to_va((uintptr_t)aes_data->output_data, aes_data->output_size);
	if (!output_data) {
		EMSG("output data pa=0x%08lx size=%d to va fail\r\n", (uintptr_t)aes_data->output_data, (int)aes_data->output_size);
		return -1;
	}

	input_data = (unsigned char *)nvt_smc_pa_to_va((uintptr_t)aes_data->input_data, aes_data->input_size);
	if (!input_data) {
		EMSG("input data pa=0x%08lx size=%d to va fail\r\n", (uintptr_t)aes_data->input_data, (int)aes_data->input_size);
		return -1;
	}

	ret = crypto_cipher_update(aes_data->ctx, aes_crypto, true, input_data, aes_data->input_size, output_data);
	if (ret != TEE_SUCCESS) {
		EMSG("crypto_cipher_update fail ret:%x\n", ret);
		return ret;
	}

	ret = cache_operation(TEE_CACHEFLUSH, (void *)output_data, aes_data->input_size);   // output size should equal input size
	if (ret != TEE_SUCCESS) {
		EMSG("cache_operation TEE_CACHEFLUSH fail ret:%x\n", ret);
		return ret;
	}

	return 0;
}

static int smc_aes_final(NVT_SMC_AES_DATA *aes_data)
{
	unsigned int aes_mode = 0;
EMSG("7\n");
	smc_aes_transfer_aes_mode(aes_data->aes_mode, &aes_mode);
	crypto_cipher_final(aes_data->ctx);
	return 0;
}

static int smc_aes_free(NVT_SMC_AES_DATA *aes_data)
{
	unsigned int aes_mode = 0;
EMSG("6\n");
	smc_aes_transfer_aes_mode(aes_data->aes_mode, &aes_mode);
	crypto_cipher_free_ctx(aes_data->ctx);
	return 0;
}

static int smc_aes_crypto(NVT_SMC_AES_DATA *aes_data)
{
	int ret =0;
EMSG("8\n");
	switch (aes_data->operation) {
		case NVT_SMC_AES_OPERATION_ALLOC:
			EMSG("do NVT_SMC_AES_OPERATION_ALLOC\r\n");
			ret = smc_aes_alloc(aes_data);
			break;
		case NVT_SMC_AES_OPERATION_INIT:
			EMSG("do NVT_SMC_AES_OPERATION_INIT\r\n");
			ret = smc_aes_init(aes_data);
			break;
		case NVT_SMC_AES_OPERATION_UPDATE:
			EMSG("do NVT_SMC_AES_OPERATION_UPDATE\r\n");
			ret = smc_aes_update(aes_data);
			break;
		case NVT_SMC_AES_OPERATION_FINAL:
			EMSG("do NVT_SMC_AES_OPERATION_FINAL\r\n");
			ret = smc_aes_final(aes_data);
			break;
		case NVT_SMC_AES_OPERATION_FREE:
			EMSG("do NVT_SMC_AES_OPERATION_FREE\r\n");
			ret = smc_aes_free(aes_data);
			break;
		default:
			EMSG("aes operation (%d) error\r\n", aes_data->operation);
			ret = -1;
			break;
	}
	return ret;
}

int nvt_ivot_optee_aes_operation(NVT_SMC_AES_DATA *aes_smc_data)
{
	NVT_SMC_AES_DATA *aes_data = NULL;
EMSG("A\n");
	aes_data = (NVT_SMC_AES_DATA *)nvt_smc_pa_to_va((uintptr_t)aes_smc_data, sizeof(NVT_SMC_AES_DATA));
	if (!aes_data) {
		EMSG("aes data pa=0x%08lx to va fail\r\n", (uintptr_t)aes_smc_data);
		return -1;
	}

	if (aes_data->tag != SMC_AES_TAG) {
		EMSG("check tag fail, tag:%x\r\n", aes_data->tag);
		return -1;
	}

	if (aes_data->crypto_type == NVT_SMC_AES_CRYPTO_DECRYPTION) {
		return smc_aes_crypto(aes_data);
	}
	else if (aes_data->crypto_type == NVT_SMC_AES_CRYPTO_ENCRYPTION) {
		return smc_aes_crypto(aes_data);
	}
	else {
		EMSG("aes_smc_data->crypto_type error, %d\r\n", aes_smc_data->crypto_type);
		return -1;
	}
	EMSG("smc_aes_operation something wrong!\r\n");

	return -1;
}
