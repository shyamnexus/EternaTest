#include <kernel/boot.h>
#include <string.h>
#include <tee/cache.h>
#include <sm/optee_smc.h>
#include <mm/core_mmu.h>
#include <mm/core_memprot.h>
#include <crypto/crypto.h>
#include <optee_msg.h>
#include <nvt_types.h>

#include "../../include/crypto/nvt_sha_smc.h"
#include "../../include/nvt_smc_util.h"

static int transfer_sha_mode(unsigned int input_data, unsigned int *output_data)
{
	switch(input_data) {
		case NVT_SMC_SHA_MODE_SHA256:
			*output_data = TEE_ALG_SHA256;
			break;
		default:
			EMSG("transfer_sha_mode fail mode(%x)\r\n", input_data);
			return -1;
	}
	return 0;
}

static int transfer_sha_size(unsigned int mode)
{
	switch (mode) {
		case NVT_SMC_SHA_MODE_SHA256:
			return TEE_SHA256_HASH_SIZE;
		default:
			EMSG("sha mode (%d) error\r\n", mode);
			return 0;
	}
	return 0;
}

static int smc_sha_alloc(NVT_SMC_SHA_DATA *sha_data)
{
	unsigned int sha_mode = 0;
	int ret= 0;

	ret = transfer_sha_mode(sha_data->sha_mode, &sha_mode);
	if (ret != 0) {
		EMSG("transfer_sha_mode fail, ret:%d\r\n", ret);
		return ret;
	}

	ret = crypto_hash_alloc_ctx(&sha_data->ctx, sha_mode);
	if (ret != 0) {
		EMSG("crypto_hash_alloc_ctx fail ret:%d\n", ret);
		return ret;
	}

	ret = cache_operation(TEE_CACHEFLUSH, sha_data, sizeof(NVT_SMC_SHA_DATA));
	if (ret != TEE_SUCCESS) {
		EMSG("cache_operation TEE_CACHEFLUSH fail ret:%x\n", ret);
	}

	return ret;
}

static int smc_sha_init(NVT_SMC_SHA_DATA *sha_data)
{
	int ret = 0;
	unsigned int sha_mode = 0;

	ret = transfer_sha_mode(sha_data->sha_mode, &sha_mode);
	if (ret != 0) {
		EMSG("transfer_sha_mode fail, ret:%d\r\n", ret);
		return ret;
	}

	ret = crypto_hash_init(sha_data->ctx);
	if (ret != 0) {
		EMSG("crypto_hash_init fail ret:%d\n", ret);
		return ret;
	}
	return ret;
}

static int smc_sha_update(NVT_SMC_SHA_DATA *sha_data)
{
	int ret = 0;
	unsigned int   sha_mode = 0;
	unsigned char *intput_data = NULL;

	ret = transfer_sha_mode(sha_data->sha_mode, &sha_mode);
	if (ret != 0) {
		EMSG("transfer_sha_mode fail, ret:%d\r\n",ret);
		return ret;
	}

	if (sha_data->input_size == 0) {
		EMSG("intput size:%d error\r\n", sha_data->input_size);
		return -1;
	}

	intput_data = (unsigned char *)nvt_smc_pa_to_va((uintptr_t)sha_data->input_data, sha_data->input_size);
	if (!intput_data) {
		EMSG("intput_data pa=0x%08lx size=%d to va fail\r\n", (uintptr_t)sha_data->input_data, (int)sha_data->input_size);
		return -1;
	}

	ret = crypto_hash_update(sha_data->ctx, intput_data, sha_data->input_size);
	if (ret != 0) {
		EMSG("crypto_hash_update fail ret:%d\n", ret);
		return ret;
	}

	return ret;
}

static int smc_sha_final(NVT_SMC_SHA_DATA *sha_data)
{
	int ret = 0;
	unsigned int   sha_mode = 0;
	unsigned char *output_data = NULL;
	unsigned int   output_size = 0;

	ret = transfer_sha_mode(sha_data->sha_mode, &sha_mode);
	if (ret != 0) {
		EMSG("transfer_sha_mode fail, ret:%d\r\n",ret);
		return ret;
	}

	output_size = transfer_sha_size(sha_data->sha_mode);
	if (output_size == 0) {
		EMSG("transfer_sha_size fail\r\n");
		return -1;
	}

	output_data = (unsigned char *)nvt_smc_pa_to_va( (uintptr_t)sha_data->output_data, output_size);
	if (output_data == NULL) {
		EMSG("output_data pa=0x%08lx size=%d to va fail\r\n", (uintptr_t)sha_data->output_data, (int)output_size);
		return -1;
	}

	ret = crypto_hash_final(sha_data->ctx, output_data, output_size);
	if (ret) {
		EMSG("crypto_hash_final fail ret:%d\n", ret);
		return ret;
	}
	sha_data->output_size = output_size;

	ret = cache_operation(TEE_CACHEFLUSH, sha_data, sizeof(NVT_SMC_SHA_DATA)); ////flush output size
	if(ret != TEE_SUCCESS) {
		EMSG("cache_operation TEE_CACHEFLUSH fail ret:%x\n",ret);
	}

	ret = cache_operation(TEE_CACHEFLUSH, output_data, sizeof(output_size));  ///flush output data
	if(ret != TEE_SUCCESS) {
		EMSG("cache_operation TEE_CACHEFLUSH fail ret:%x\n",ret);
	}

	return ret;
}

static int smc_sha_free(NVT_SMC_SHA_DATA *sha_data)
{
	int ret = 0;
	unsigned int sha_mode = 0;

	ret = transfer_sha_mode(sha_data->sha_mode, &sha_mode);
	if (ret != 0) {
		EMSG("transfer_sha_mode fail, ret:%d\r\n", ret);
		return ret;
	}
	crypto_hash_free_ctx(sha_data->ctx);
	return ret;

}

int nvt_ivot_optee_sha_operation(NVT_SMC_SHA_DATA *sha_smc_data)
{
	int ret = 0;
	NVT_SMC_SHA_DATA *sha_data = NULL;

	sha_data = (NVT_SMC_SHA_DATA *)nvt_smc_pa_to_va((uintptr_t)sha_smc_data,sizeof(NVT_SMC_SHA_DATA));
	if (!sha_data) {
		EMSG("sha data pa=0x%08lx to va fail\r\n", (uintptr_t)sha_smc_data);
		return -1;
	}

	if (sha_data->tag != SMC_SHA_TAG) {
		EMSG("check tag fail, tag:%x\r\n",sha_data->tag);
		return -1;
	}

	switch(sha_data->operation) {
		case NVT_SMC_SHA_OPERATION_ALLOC:
			DMSG("NVT_SMC_SHA_OPERATION_ALLOC\r\n");
			ret = smc_sha_alloc(sha_data);
			break;
		case NVT_SMC_SHA_OPERATION_INIT:
			DMSG("NVT_SMC_SHA_OPERATION_INIT\r\n");
			ret = smc_sha_init(sha_data);
			break;
		case NVT_SMC_SHA_OPERATION_UPDATE:
			DMSG("NVT_SMC_SHA_OPERATION_UPDATE\r\n");
			ret = smc_sha_update(sha_data);
			break;
		case NVT_SMC_SHA_OPERATION_FINAL:
			DMSG("NVT_SMC_SHA_OPERATION_FINAL\r\n");
			ret = smc_sha_final(sha_data);
			break;
		case NVT_SMC_SHA_OPERATION_FREE:
			DMSG("NVT_SMC_SHA_OPERATION_FREE\r\n");
			ret = smc_sha_free(sha_data);
			break;
		default:
			EMSG("sha_data->operation (%d) error\r\n", sha_data->operation);
			ret =-1;
			break;
	}

	return ret;
}
