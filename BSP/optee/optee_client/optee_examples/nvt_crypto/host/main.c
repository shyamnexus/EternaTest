/*
   Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.

   SPDX-License-Identifier: BSD-2-Clause.
 */

#include <err.h>
#include <stdio.h>
#include <string.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* To the the UUID (found the the TA's h-file(s)) */
#include <nvt_crypto_ta.h>

#define AES_TEST_BUFFER_SIZE	4096
#define AES_TEST_KEY_SIZE	16
#define AES_BLOCK_SIZE		16

#define DECODE			    0
#define ENCODE			    1

/* TEE resources */
struct test_ctx {
	TEEC_Context ctx;
	TEEC_Session sess;
};

void prepare_tee_session(struct test_ctx *ctx)
{
	TEEC_UUID uuid = TA_NVT_CRYPTO_UUID;
	uint32_t origin;
	TEEC_Result res;

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx->ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	/* Open a session with the TA */
	res = TEEC_OpenSession(&ctx->ctx, &ctx->sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, origin);
}

void terminate_tee_session(struct test_ctx *ctx)
{
	TEEC_CloseSession(&ctx->sess);
	TEEC_FinalizeContext(&ctx->ctx);
}

void prepare_aes(struct test_ctx *ctx, int encode)
{
	TEEC_Operation op;
	uint32_t origin;
	TEEC_Result res;

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
					 TEEC_VALUE_INPUT,
					 TEEC_VALUE_INPUT,
					 TEEC_NONE);

	op.params[0].value.a = TA_ALGO_AES_ECB;
	op.params[1].value.a = TA_AES_SIZE_128BIT;
	op.params[2].value.a = encode ? TA_AES_MODE_ENCODE :
					TA_AES_MODE_DECODE;
	res = TEEC_InvokeCommand(&ctx->sess, TA_AES_CMD_PREPARE,
				 &op, &origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InvokeCommand(PREPARE) failed 0x%x origin 0x%x",
			res, origin);
}

void set_key(struct test_ctx *ctx, char *key, size_t key_sz)
{
	TEEC_Operation op;
	uint32_t origin;
	TEEC_Result res;

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_NONE, TEEC_NONE, TEEC_NONE);

	op.params[0].tmpref.buffer = key;
	op.params[0].tmpref.size = key_sz;

	res = TEEC_InvokeCommand(&ctx->sess, TA_AES_CMD_SET_KEY,
				 &op, &origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InvokeCommand(SET_KEY) failed 0x%x origin 0x%x",
			res, origin);
}

void set_iv(struct test_ctx *ctx, char *iv, size_t iv_sz)
{
	TEEC_Operation op;
	uint32_t origin;
	TEEC_Result res;

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					  TEEC_NONE, TEEC_NONE, TEEC_NONE);
	op.params[0].tmpref.buffer = iv;
	op.params[0].tmpref.size = iv_sz;

	res = TEEC_InvokeCommand(&ctx->sess, TA_AES_CMD_SET_IV,
				 &op, &origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InvokeCommand(SET_IV) failed 0x%x origin 0x%x",
			res, origin);
}

void cipher_buffer(struct test_ctx *ctx, char *in, char *out, size_t sz)
{
	TEEC_Operation op;
	uint32_t origin;
	TEEC_Result res;

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_MEMREF_TEMP_OUTPUT,
					 TEEC_NONE, TEEC_NONE);
	op.params[0].tmpref.buffer = in;
	op.params[0].tmpref.size = sz;
	op.params[1].tmpref.buffer = out;
	op.params[1].tmpref.size = sz;

	res = TEEC_InvokeCommand(&ctx->sess, TA_AES_CMD_CIPHER,
				 &op, &origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InvokeCommand(CIPHER) failed 0x%x origin 0x%x",
			res, origin);
}

int main(void)
{
	struct test_ctx ctx;
	char key[AES_TEST_KEY_SIZE];
	char iv[AES_BLOCK_SIZE];
	char clear[AES_TEST_BUFFER_SIZE];
	char ciph[AES_TEST_BUFFER_SIZE];
	char temp[AES_TEST_BUFFER_SIZE];
	uint32_t otp_key[4], i;



	for (i= 0; i < 5;i++) {

		//printf("Prepare session with the TA\n");
		prepare_tee_session(&ctx);

		//printf("Prepare encode operation\n");
		prepare_aes(&ctx, ENCODE);

		//printf("Load key in TA\n");
		memset(key, 0xa5, sizeof(key)); /* Load some dummy value */
		#if 1
		otp_key[0] = NVT_OTP_KEY_TAG;
		otp_key[1] = NVT_OTP_KEY_KM_KEY_SET0+i;
		otp_key[2] = NVT_OTP_KEY_TAG;
		otp_key[3] = NVT_OTP_KEY_TAG;
		memcpy(key, otp_key, sizeof(otp_key));
		#endif
		set_key(&ctx, key, AES_TEST_KEY_SIZE);

		//printf("Reset ciphering operation in TA (provides the initial vector)\n");
		memset(iv, 0, sizeof(iv)); /* Load some dummy value */
		set_iv(&ctx, iv, AES_BLOCK_SIZE);

		printf("Encode buffer from TA with key %d\r\n", i);
		memset(clear, 0x5a, sizeof(clear)); /* Load some dummy value */
		cipher_buffer(&ctx, clear, ciph, AES_TEST_BUFFER_SIZE);

		printf("cipher = 0x%x, 0x%x, 0x%x, 0x%x\r\n", ciph[0], ciph[1], ciph[2], ciph[3]);

		printf("Prepare decode operation\r\n");
		prepare_aes(&ctx, DECODE);

		printf("Load key in TA\r\n");
		memset(key, 0xa5, sizeof(key)); /* Load some dummy value */
		#if 1
		otp_key[0] = NVT_OTP_KEY_TAG;
		otp_key[1] = NVT_OTP_KEY_KM_KEY_SET0+i;
		otp_key[2] = NVT_OTP_KEY_TAG;
		otp_key[3] = NVT_OTP_KEY_TAG;
		memcpy(key, otp_key, sizeof(otp_key));
		#endif
		set_key(&ctx, key, AES_TEST_KEY_SIZE);

		printf("Reset ciphering operation in TA (provides the initial vector)\n");
		memset(iv, 0, sizeof(iv)); /* Load some dummy value */
		set_iv(&ctx, iv, AES_BLOCK_SIZE);

		printf("Decode buffer from TA\n");
		cipher_buffer(&ctx, ciph, temp, AES_TEST_BUFFER_SIZE);
		sleep(1);
		/* Check decoded is the clear content */
		if (memcmp(clear, temp, AES_TEST_BUFFER_SIZE))
			printf("Clear text and decoded text differ => ERROR\r\n\r\n");
		else
			printf("Clear text and decoded text match\r\n\r\n");
		sleep(1);
		terminate_tee_session(&ctx);
	}
	return 0;
}
