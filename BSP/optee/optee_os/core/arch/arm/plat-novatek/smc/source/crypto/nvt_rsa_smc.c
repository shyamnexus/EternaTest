#include <kernel/boot.h>
#include <string.h>
#include <tee/cache.h>
#include <sm/optee_smc.h>
#include <mm/core_mmu.h>
#include <mm/core_memprot.h>
#include <crypto/crypto.h>
#include <optee_msg.h>
#include <plat/rsa.h>

#include "../../include/crypto/nvt_rsa_smc.h"
#include "../../include/nvt_smc_util.h"

#if 0
static void data_reverse(unsigned char *input_data, unsigned int size)
{
	unsigned int  i;
	unsigned char tmp;

	for (i=0; i<(size/2); i++) {
		tmp                      = input_data[size - 1 - i];
		input_data[size - 1 - i] = input_data[i];
		input_data[i]            = tmp;
	}
}
#endif

static void fill_data_r(unsigned char *s, unsigned char *target, int len)
{
	int i;
	for (i=0; i<len; i++) {
		target[len-i-1] = s[i];
	}
}

static int transfer_rsa_mode(unsigned int input_data, unsigned int *output_data)
{
	switch(input_data) {
		case NVT_SMC_RSA_MODE_1024:
			*output_data = RSA_KEY_1024;
			break;
		case NVT_SMC_RSA_MODE_2048:
			*output_data = RSA_KEY_2048;
			break;
#ifdef RSA4096_SUPPORT
		case NVT_SMC_RSA_MODE_4096:
			*output_data = RSA_KEY_4096;
			break;
#endif
		default:
			EMSG("RSA mode (%x) fail\r\n", input_data);
			return -1;
			break;
	}
	return 0;
}

static int smc_rsa_open(NVT_SMC_RSA_DATA *rsa_data)
{
#define RSA_KEY_SIZE_MAX     (4096/8)       ///< bytes
	int ret = 0;
	unsigned int  rsa_mode  = 0;
	unsigned char *n_key    = NULL;
	unsigned char *ed_key   = NULL;
	UINT32        *key_r    = NULL;
	UINT32         key_len;

	/* check key size */
	if (!rsa_data->n_key_size  ||
		!rsa_data->ed_key_size ||
		rsa_data->n_key_size  > RSA_KEY_SIZE_MAX ||
		rsa_data->ed_key_size > RSA_KEY_SIZE_MAX) {
		EMSG("n key size (%d) ed key size (%d) error\r\n", rsa_data->n_key_size, (int)rsa_data->ed_key_size);
		return -1;
	}

	/* get rsa mode */
	ret = transfer_rsa_mode(rsa_data->rsa_mode, &rsa_mode);
	if (ret < 0) {
		EMSG("rsa mode(%d) transfer fail\r\n", rsa_data->rsa_mode);
		return -1;
	}

	/* alloc key buffer for data reverse to hardward engine */
	key_r = (UINT32 *)malloc(RSA_KEY_SIZE_MAX);
	if (!key_r) {
		EMSG("alloc rsa key buffer fali\r\n");
		ret = -1;
		goto exit;
	}

	n_key = (unsigned char *)nvt_smc_pa_to_va((uintptr_t)rsa_data->n_key, rsa_data->n_key_size);
	if (!n_key) {
		EMSG("key_n pa=0x%08lx size=%d to va fail\r\n", (uintptr_t)rsa_data->n_key, (int)rsa_data->n_key_size);
		ret = -1;
		goto exit;
	}

	ed_key = (unsigned char *)nvt_smc_pa_to_va((uintptr_t)rsa_data->ed_key, rsa_data->ed_key_size);
	if (!ed_key) {
		EMSG("ed key pa=0x%08lx size=%d to va fail\r\n", (uintptr_t)rsa_data->ed_key, (int)rsa_data->ed_key_size);
		ret = -1;
		goto exit;
	}

	/* Open and Set Mode */
	rsa_open();
	rsa_setConfig(RSA_CONFIG_ID_MODE,      RSA_MODE_NORMAL);
	rsa_setConfig(RSA_CONFIG_ID_KEY_WIDTH, rsa_mode);

	/* Set Key_N */
	fill_data_r(n_key, (unsigned char *)key_r, rsa_data->n_key_size);
	key_len = rsa_data->n_key_size/4;
	if (rsa_data->n_key_size%4) {
		key_len++;
		memset((void *)(((uintptr_t)key_r)+rsa_data->n_key_size), 0, RSA_KEY_SIZE_MAX-rsa_data->n_key_size);
	}
	rsa_setkey_n(key_r, key_len, RSA_ORDER_LSB_FIRST);

	/* Set Key_ED */
	fill_data_r(ed_key, (unsigned char *)key_r, rsa_data->ed_key_size);
	key_len = rsa_data->ed_key_size/4;
	if (rsa_data->ed_key_size%4) {
		key_len++;
		memset((void *)(((uintptr_t)key_r)+rsa_data->ed_key_size), 0, RSA_KEY_SIZE_MAX-rsa_data->ed_key_size);
	}
	rsa_setkey_ed(key_r, key_len, RSA_ORDER_LSB_FIRST);

exit:
	if (key_r) {
		free(key_r);
	}

	return ret;
}

static int smc_rsa_update(NVT_SMC_RSA_DATA *rsa_data)
{
#define RSA_DATA_SIZE_MAX     (4096/8)       ///< bytes
	int ret = 0;
	unsigned char *input_data  = NULL;
	unsigned char *output_data = NULL;
	UINT32        *data_r      = NULL;
	UINT32         data_len;

	/* check data size */
	if (!rsa_data->input_size || rsa_data->input_size > RSA_DATA_SIZE_MAX) {
		EMSG("input_size = %d error\r\n", (int)rsa_data->input_size);
		return -1;
	}

	/* alloc data buffer for data reverse to hardward engine */
	data_r = (UINT32 *)malloc(RSA_DATA_SIZE_MAX);
	if (!data_r) {
		EMSG("alloc rsa data buffer falied!\r\n");
		ret = -1;
		goto exit;
	}

	input_data = (unsigned char *)nvt_smc_pa_to_va((uintptr_t)rsa_data->input_data, (int)rsa_data->input_size);
	if (!input_data) {
		EMSG("encrypt_data pa=0x%08lx size=%d to va fail\r\n", (uintptr_t)rsa_data->input_data, (int)rsa_data->input_size);
		ret = -1;
		goto exit;
	}

	output_data = (unsigned char *)nvt_smc_pa_to_va((uintptr_t)rsa_data->output_data, rsa_data->input_size);
	if (!output_data) {
		EMSG("output_data pa=0x%08lx size=%d to va fail\r\n", (uintptr_t)rsa_data->output_data, rsa_data->input_size);
		ret = -1;
		goto exit;
	}

	fill_data_r(input_data, (unsigned char *)data_r, rsa_data->input_size);
	data_len = rsa_data->input_size/4;
	if (rsa_data->input_size%4) {
		data_len++;
		memset((void *)(((uintptr_t)data_r)+rsa_data->input_size), 0, RSA_KEY_SIZE_MAX-rsa_data->input_size);
	}
	rsa_pio_enable(data_r, data_len, RSA_ORDER_LSB_FIRST);
	rsa_getOutput(data_r, data_len);
	fill_data_r((unsigned char *)data_r, output_data, rsa_data->input_size);

	ret = cache_operation(TEE_CACHEFLUSH, (void *)output_data, rsa_data->input_size);
	if (ret != TEE_SUCCESS) {
		EMSG("cache_operation TEE_CACHEFLUSH fail ret:%d\r\n", ret);
	}

exit:
	if (data_r) {
		free(data_r);
	}

	return ret;
}

static int smc_rsa_close(void)
{
	rsa_close();
	return 0;
}

int nvt_ivot_optee_rsa_operation(NVT_SMC_RSA_DATA *rsa_smc_data)
{
	int ret = 0;
	NVT_SMC_RSA_DATA *rsa_data = NULL;

	rsa_data = (NVT_SMC_RSA_DATA *)nvt_smc_pa_to_va((uintptr_t)rsa_smc_data, sizeof(NVT_SMC_RSA_DATA));
	if (!rsa_data) {
		EMSG("rsa_data pa=0x%08lx to va NULL\r\n", (uintptr_t)rsa_smc_data);
		return -1;
	}

	if (rsa_data->tag != SMC_RSA_TAG) {
		EMSG("check tag fail,tag:%x\r\n", rsa_data->tag);
		return -1;
	}

	switch (rsa_data->operation) {
		case NVT_SMC_RSA_OPERATION_OPEN:
			DMSG("NVT_SMC_RSA_OPERATION_OPEN\r\n");
			ret = smc_rsa_open(rsa_data);
			break;
		case NVT_SMC_RSA_OEPRATION_UPDATE:
			DMSG("NVT_SMC_RSA_OEPRATION_UPDATE\r\n");
			ret = smc_rsa_update(rsa_data);
			break;
		case NVT_SMC_RSA_OPERATION_CLOSE:
			DMSG("NVT_SMC_RSA_OPERATION_FREE\r\n");
			ret = smc_rsa_close();
			break;
		default:
			EMSG("rsa operation fail (%x)\r\n",rsa_data->operation);
			ret = -1;
			break;
	}
	return ret;
}
