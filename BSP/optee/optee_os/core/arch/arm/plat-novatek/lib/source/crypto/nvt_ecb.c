/*
    Crypto Framework driver

    Copyright Novatek Microelectronics Corp. 2023.  All rights reserved.
*/

#include <assert.h>
#include <platform_config.h>
#include <crypto/crypto.h>
#include <crypto/crypto_impl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tee_api_types.h>
#include <tee/tee_cryp_utl.h>
#include <util.h>
#include <plat/crypto.h>
#include <efuse_protected.h>
#include <drvcrypt.h>
#include <drvcrypt_cipher.h>
#include <initcall.h>
#include <kernel/panic.h>

#define DBG_IND(fmt, args...) //EMSG(fmt, ##args)
#define DBG_ERR(fmt, args...)   EMSG(fmt, ##args)
#define DBG_WRN(fmt, args...)   EMSG(fmt, ##args)
#define CHKPNT                  EMSG("CHK: %d, %s\r\n" , __LINE__, __func__)

#define CACHE_LINE_ALIGN        STACK_ALIGNMENT         ///< platform cache line size
#define CACHE_LINE_MASK         (CACHE_LINE_ALIGN-1)
#define CRYPTO_TMP_BUF_SIZE     1024

struct nvt_aes_state {
	CRYPT_DMA_SET   crypt_dma_set;
	CRYPT_SEG_DES   crypt_seg;
	int             block_len;
	int             otp_key_field;
};

struct nvt_ecb_ctx {
	struct crypto_cipher_ctx ctx;
	int                      cipher_algo;
	int                      opmode;
	int                      key_len;
	int                      (*update)(struct crypto_cipher_ctx *ctx, const unsigned char *pt, unsigned char *ct, unsigned long len);
	struct nvt_aes_state     state;
	char                     *tmp_pt;
	char                     *tmp_ct;
	char                     buffer[];
};

typedef enum {
	CYPHER_ALGO_DES = 0x00,               ///< Select Crypto mode DES
	CYPHER_ALGO_3DES,                     ///< Select Crypto mode 3DES
	CYPHER_ALGO_AES,                      ///< Select Crypto mode AES
	CYPHER_ALGO_NUM,
	ENUM_DUMMY4WORD(CYPHER_ALGO)
} CYPHER_ALGO;

#define MAKEFOURCC(ch0, ch1, ch2, ch3) ((UINT32)(UINT8)(ch0) | ((UINT32)(UINT8)(ch1) << 8) | ((UINT32)(UINT8)(ch2) << 16) | ((UINT32)(UINT8)(ch3) << 24))

#define NVT_AES_INIT_TAG        MAKEFOURCC('N', 'A', 'E', 'S')  ///< Nova aes init tag
#define NVT_OTP_KEY_TAG         MAKEFOURCC('N', 'O', 'T', 'P')  ///< Nova OTP key tag
#define NVT_OTP_KEY_KM_KEY_SET0 0x00000000
#define NVT_OTP_KEY_KM_KEY_SET1 0x00000001
#define NVT_OTP_KEY_KM_KEY_SET2 0x00000002
#define NVT_OTP_KEY_KM_KEY_SET3 0x00000003
#define NVT_OTP_KEY_KM_KEY_SET4 0x00000004

static const struct crypto_cipher_ops nvt_ecb_ops;

static struct nvt_ecb_ctx *to_ecb_ctx(struct crypto_cipher_ctx *ctx)
{
	assert(ctx && ctx->ops == &nvt_ecb_ops);

	return container_of(ctx, struct nvt_ecb_ctx, ctx);
}

static int nvt_hw_crypto_trigger(struct crypto_cipher_ctx *ctx, const unsigned char *pt, unsigned char *ct, unsigned long len, CRYPTO_TYPE type)
{
	struct nvt_ecb_ctx *c = to_ecb_ctx(ctx);
	uintptr_t src, dst;
	uint32_t  align_len, left;
	BOOL      is_in_tmp  = FALSE;
	BOOL      is_out_tmp = FALSE;

	if (((uintptr_t)pt) & 0x3) {
		if (len > CRYPTO_TMP_BUF_SIZE) {
			DBG_ERR("in addr=0x%08lx not word align and size=%d over tmp buffer size=%d\r\n", (uintptr_t)ct, (int)len, CRYPTO_TMP_BUF_SIZE);
			return TEE_ERROR_BAD_PARAMETERS;
		}
		is_in_tmp = TRUE;
	}

	if (((uintptr_t)ct) & CACHE_LINE_MASK) {
		if (len > CRYPTO_TMP_BUF_SIZE) {
			DBG_ERR("out addr=0x%08lx not align(%d)\r\n", (uintptr_t)ct, CACHE_LINE_ALIGN);
			return TEE_ERROR_BAD_PARAMETERS;
		}
		is_out_tmp = TRUE;
	}

	left      = (uint32_t)len & CACHE_LINE_MASK;
	align_len = len - left;
	DBG_IND("ctx=0x%08lx mode=%d type=%d opmode=%d len=%d\r\n", (uintptr_t)ctx, (int)c->state.crypt_dma_set.mode, (int)c->state.crypt_dma_set.type, (int)c->state.crypt_dma_set.opmode, (int)len);

	crypto_open();

	if (align_len) {
		if (is_in_tmp) {
			src = (uintptr_t)c->tmp_pt;
			memcpy((void *)c->tmp_pt, pt, align_len);
		}
		else {
			src = (uintptr_t)pt;
		}

		if (is_out_tmp) {
			dst = (uintptr_t)c->tmp_ct;
		}
		else {
			dst = (uintptr_t)ct;
		}

		/* setup operation */
		c->state.crypt_dma_set.type = type;
		c->state.crypt_seg.src_addr = src;
		c->state.crypt_seg.dst_addr = dst;
		c->state.crypt_seg.len      = align_len;
		if (c->state.crypt_dma_set.key_src == CRYPTO_KEY_KM) {
			DBG_IND("key_src=%d otp_key_field=%d\r\n", (int)c->state.crypt_dma_set.key_src, (int)c->state.otp_key_field);
#ifdef CFG_NVT_OPT
			otp_set_key_destination(OTP_KEY_MANAGER_CRYPTO, c->state.otp_key_field, c->key_len/4);
#else
			DBG_ERR("not support otp key\r\n");
#endif
		}
		DBG_IND("ctx=0x%08lx key_src=%d keylen=%u src=0x%08lx dst=0x%08lx len=%d\r\n", (uintptr_t)ctx, (int)c->state.crypt_dma_set.key_src, (int)c->key_len, src, dst, (int)align_len);

		crypto_dma_setDesTab(0, (CRYPT_DMA_SET *)&c->state.crypt_dma_set, (CRYPT_SEG_DES *)&c->state.crypt_seg, 1);
		if (is_out_tmp) {
			memcpy(ct, (void *)dst, align_len);
		}

		/* copy next IV */
		memcpy(c->state.crypt_dma_set.iv, c->state.crypt_dma_set.cv, sizeof(c->state.crypt_dma_set.cv));
	}

	if (left) {
		c->state.crypt_dma_set.type = type;
		c->state.crypt_seg.src_addr = (uintptr_t)c->tmp_pt;
		c->state.crypt_seg.dst_addr = (uintptr_t)c->tmp_ct;
		c->state.crypt_seg.len      = left;
		memcpy((void *)c->tmp_pt, (void *)(pt + align_len), left);
		if (c->state.crypt_dma_set.key_src == CRYPTO_KEY_KM) {
			DBG_IND("key_src=%d, otp_key_field=%d\r\n", (int)c->state.crypt_dma_set.key_src, (int)c->state.otp_key_field);
#ifdef CFG_NVT_OPT
			otp_set_key_destination(OTP_KEY_MANAGER_CRYPTO, c->state.otp_key_field, c->key_len/4);
#else
			DBG_ERR("not support otp key\r\n");
#endif
		}
		DBG_IND("ctx=0x%08lx key_src=%d keylen=%d src=0x%08lx dst=0x%08lx len=%d\r\n", (uintptr_t)ctx, (int)c->state.crypt_dma_set.key_src, (int)c->key_len, (uintptr_t)c->state.crypt_seg.src_addr, (uintptr_t)c->state.crypt_seg.dst_addr, (int)left);

		crypto_dma_setDesTab(0, (CRYPT_DMA_SET *)&c->state.crypt_dma_set, (CRYPT_SEG_DES *)&c->state.crypt_seg, 1);

		/* copy output data to buffer */
		memcpy((void *)(ct + align_len), (void *)c->tmp_ct, left);

		/* copy next IV */
		memcpy(c->state.crypt_dma_set.iv, c->state.crypt_dma_set.cv, sizeof(c->state.crypt_dma_set.cv));
	}

	crypto_close();

	return TEE_SUCCESS;
}

static int nvt_hw_aes_encrypt(struct crypto_cipher_ctx *ctx, const unsigned char *pt, unsigned char *ct, unsigned long len)
{
	return nvt_hw_crypto_trigger(ctx, pt, ct, len, CRYPTO_TYPE_ENCRYPT);
}

static int nvt_hw_aes_decrypt(struct crypto_cipher_ctx *ctx, const unsigned char *pt, unsigned char *ct, unsigned long len)
{
	return nvt_hw_crypto_trigger(ctx, pt, ct, len, CRYPTO_TYPE_DECRYPT);
}

static TEE_Result nvt_cypher2mode(size_t key1_len, CYPHER_ALGO cypher_algo, CRYPTO_MODE *mode, int *block_len)
{
	switch (cypher_algo) {
		case CYPHER_ALGO_DES:
			if (key1_len != 8) {
				return TEE_ERROR_NOT_SUPPORTED;
			}
			*mode      = CRYPTO_MODE_DES;
			*block_len = 8;
			return TEE_SUCCESS;
		case CYPHER_ALGO_3DES:
			if (key1_len != 24) {
				return TEE_ERROR_NOT_SUPPORTED;
			}
			*mode      = CRYPTO_MODE_3DES;
			*block_len = 8;
			return TEE_SUCCESS;
		case CYPHER_ALGO_AES:
			if (key1_len == 16) {
				*mode      = CRYPTO_MODE_AES128;
				*block_len = 16;
				return TEE_SUCCESS;
			} else if (key1_len == 32) {
				*mode      = CRYPTO_MODE_AES256;
				*block_len = 16;
				return TEE_SUCCESS;
			}
			return TEE_ERROR_NOT_SUPPORTED;
		default:
			EMSG("cypher_algo = %d, key1_len = %d\r\n", (int)cypher_algo, (int)key1_len);
			return TEE_ERROR_NOT_SUPPORTED;
	}
}

static TEE_Result nvt_ecb_init(struct crypto_cipher_ctx *ctx,
				   TEE_OperationMode mode, const uint8_t *key1,
				   size_t key1_len, const uint8_t *key2 __unused,
				   size_t key2_len __unused,
				   const uint8_t *iv, size_t iv_len)
{
	struct nvt_ecb_ctx *c = to_ecb_ctx(ctx);
	TEE_Result ret;
	UINT32     otp_key[4];

	if (key1 == NULL) {
		DBG_ERR("key1 is NULL\r\n");
		return TEE_ERROR_BAD_PARAMETERS;
	}

	if (mode == TEE_MODE_ENCRYPT) {
		c->update = nvt_hw_aes_encrypt;
	} else {
		c->update = nvt_hw_aes_decrypt;
	}
	DBG_IND("ctx=0x%08lx opmode=%d Enc/Dec=%d key1=0x%08lx key1_len=%d iv_len=%d\r\n", (uintptr_t)ctx, (int)c->opmode, (int)mode, (uintptr_t)key1, (int)key1_len, (int)iv_len);
	c->key_len = key1_len;
	ret = nvt_cypher2mode(c->key_len, c->cipher_algo, &c->state.crypt_dma_set.mode, &c->state.block_len);
	if (ret != TEE_SUCCESS) {
		DBG_ERR("Invalid key1_len=%d cipher_algo=%d\r\n", (int)c->key_len, (int) c->cipher_algo);
		return TEE_ERROR_NOT_SUPPORTED;
	}
	DBG_IND("cipher_algo=%d mode=%d\r\n", (int) c->cipher_algo, c->state.crypt_dma_set.mode);
	memcpy(c->state.crypt_dma_set.key, key1, c->key_len);
	c->state.crypt_dma_set.key_src = CRYPTO_KEY_NORMAL;
	if (CYPHER_ALGO_AES == c->cipher_algo && c->key_len >= 16) {
		memcpy(otp_key, key1, sizeof(otp_key));
		if (otp_key[0] == NVT_OTP_KEY_TAG && otp_key[2] == NVT_OTP_KEY_TAG && otp_key[3] == NVT_OTP_KEY_TAG) {
			if (otp_key[1] > NVT_OTP_KEY_KM_KEY_SET4) {
				DBG_ERR("Invalid key set %d\r\n", (int)otp_key[1]);
				return TEE_ERROR_BAD_PARAMETERS;
			}
			c->state.crypt_dma_set.key_src = CRYPTO_KEY_KM;
			c->state.otp_key_field         = (otp_key[1] << 2);
		}
	}
	c->state.crypt_dma_set.opmode  = c->opmode;
	c->state.crypt_dma_set.get_s0  = 0;
	c->state.crypt_seg.block_inten = 0;
	c->state.crypt_seg.non_flush   = 0;
	c->state.crypt_seg.last_block  = 1;
	if (iv_len > 16) {
		DBG_ERR("iv_len=%d exceeds limit 16\r\n", (int)iv_len);
		return TEE_ERROR_NOT_SUPPORTED;
	}
	if (iv) {
		memcpy(c->state.crypt_dma_set.iv, iv, iv_len);
	}
	return TEE_SUCCESS;
}

static TEE_Result nvt_ecb_update(struct crypto_cipher_ctx *ctx,
								 bool last_block __unused,
								 const uint8_t *data, size_t len, uint8_t *dst)
{
	struct nvt_ecb_ctx *c = to_ecb_ctx(ctx);

	if (c->update && c->update(ctx, data, dst, len) == 0)
		return TEE_SUCCESS;
	else
		return TEE_ERROR_BAD_STATE;
}

static void nvt_ecb_final(struct crypto_cipher_ctx *ctx __unused)
{
	return;
}

static void nvt_ecb_free_ctx(struct crypto_cipher_ctx *ctx)
{
	free(to_ecb_ctx(ctx));
}

static void nvt_ecb_copy_state(struct crypto_cipher_ctx *dst_ctx, struct crypto_cipher_ctx *src_ctx)
{
	struct nvt_ecb_ctx *src = to_ecb_ctx(src_ctx);
	struct nvt_ecb_ctx *dst = to_ecb_ctx(dst_ctx);

	assert(src->cipher_algo == dst->cipher_algo);
	dst->update = src->update;
	dst->state  = src->state;
}

static const struct crypto_cipher_ops nvt_ecb_ops = {
	.init       = nvt_ecb_init,
	.update     = nvt_ecb_update,
	.final      = nvt_ecb_final,
	.free_ctx   = nvt_ecb_free_ctx,
	.copy_state = nvt_ecb_copy_state,
};

static TEE_Result nvt_ecb_alloc_ctx(struct crypto_cipher_ctx **ctx_ret, CYPHER_ALGO cipher_algo, CRYPTO_OPMODE opmode)
{
	struct nvt_ecb_ctx *c = NULL;
	uint32_t alloc_size;

	alloc_size = sizeof(struct nvt_ecb_ctx) + CACHE_LINE_ALIGN + ROUNDUP(CRYPTO_TMP_BUF_SIZE, CACHE_LINE_ALIGN)*2;

	c = (struct nvt_ecb_ctx *)calloc(1, alloc_size);
	if (!c)
		return TEE_ERROR_OUT_OF_MEMORY;

	c->tmp_pt = (char *)ROUNDUP((uintptr_t)&c->buffer, CACHE_LINE_ALIGN);
	c->tmp_ct = (char *)(((uintptr_t)c->tmp_pt) + ROUNDUP(CRYPTO_TMP_BUF_SIZE, CACHE_LINE_ALIGN));

	if (((uintptr_t)c->tmp_ct+ROUNDUP(CRYPTO_TMP_BUF_SIZE, CACHE_LINE_ALIGN)) > ((uintptr_t)c + alloc_size)) {
		DBG_ERR("c=0x%08lx size=%u tmp_ct=0x%08lx over boundary\r\n", (uintptr_t)c, alloc_size, (uintptr_t)c->tmp_ct);
		free(c);
		return TEE_ERROR_GENERIC;
	}

	if ((uintptr_t)c->tmp_pt & CACHE_LINE_MASK) {
		DBG_ERR("tmp_pt=0x%08lx not algin(%d)\r\n", (uintptr_t)c->tmp_pt, CACHE_LINE_ALIGN);
		free(c);
		return TEE_ERROR_GENERIC;
	}

	if ((uintptr_t)c->tmp_ct & CACHE_LINE_MASK) {
		DBG_ERR("tmp_ct=0x%08lx not algin(%d)\r\n", (uintptr_t)c->tmp_ct, CACHE_LINE_ALIGN);
		free(c);
		return TEE_ERROR_GENERIC;
	}

	c->ctx.ops     = &nvt_ecb_ops;
	c->cipher_algo = cipher_algo;
	c->opmode      = opmode;
	DBG_IND("ctx=0%08lx cipher_algo=%d opmode=%d\r\n", (uintptr_t)&c->ctx, cipher_algo, opmode);
	*ctx_ret = &c->ctx;

	return TEE_SUCCESS;
}

static TEE_Result nvt_cipherdrv_alloc_ctx(void **ctx, uint32_t algo)
{
	TEE_Result res = TEE_ERROR_NOT_IMPLEMENTED;
	
	switch (algo) {
	case TEE_ALG_AES_ECB_NOPAD:
		res = nvt_ecb_alloc_ctx((struct crypto_cipher_ctx**)ctx, CYPHER_ALGO_AES, CRYPTO_OPMODE_EBC);
		break;
	case TEE_ALG_AES_CBC_NOPAD:
		res = nvt_ecb_alloc_ctx((struct crypto_cipher_ctx**)ctx, CYPHER_ALGO_AES, CRYPTO_OPMODE_CBC);
		break;
	/*
	case TEE_ALG_AES_CTR:
		res = nvt_ecb_alloc_ctx(ctx, CYPHER_ALGO_AES, CRYPTO_OPMODE_CTR);
		break;
	*/
	case TEE_ALG_AES_CFB:
		res = nvt_ecb_alloc_ctx((struct crypto_cipher_ctx**)ctx, CYPHER_ALGO_AES, CRYPTO_OPMODE_CFB);
		break;
	case TEE_ALG_AES_OFB:
		res = nvt_ecb_alloc_ctx((struct crypto_cipher_ctx**)ctx, CYPHER_ALGO_AES, CRYPTO_OPMODE_OFB);
		break;
	case TEE_ALG_DES_ECB_NOPAD:
		res = nvt_ecb_alloc_ctx((struct crypto_cipher_ctx**)ctx, CYPHER_ALGO_DES, CRYPTO_OPMODE_EBC);
		break;
	/*
	case TEE_ALG_DES3_ECB_NOPAD:
		res = nvt_ecb_alloc_ctx(ctx, CYPHER_ALGO_3DES, CRYPTO_OPMODE_EBC);
		break;
	*/
	case TEE_ALG_DES_CBC_NOPAD:
		res = nvt_ecb_alloc_ctx((struct crypto_cipher_ctx**)ctx, CYPHER_ALGO_DES, CRYPTO_OPMODE_CBC);
		break;
	/*
	case TEE_ALG_DES3_CBC_NOPAD:
		res = nvt_ecb_alloc_ctx(ctx, CYPHER_ALGO_3DES, CRYPTO_OPMODE_CBC);
		break;
	*/
	default:
		return TEE_ERROR_NOT_IMPLEMENTED;
	}

	return res;
}

static TEE_Result nvt_cipherdrv_init_ctx(struct drvcrypt_cipher_init *dinit)
{
	struct crypto_cipher_ctx *ctx = dinit->ctx;
	TEE_OperationMode mode = TEE_MODE_DECRYPT;

	if (dinit->encrypt)
		mode = TEE_MODE_ENCRYPT;

	return ctx->ops->init(dinit->ctx, mode,
			      dinit->key1.data, dinit->key1.length,
			      dinit->key2.data, dinit->key2.length,
			      dinit->iv.data, dinit->iv.length);
}

static TEE_Result nvt_cipherdrv_update_ctx(struct drvcrypt_cipher_update *dupdate)
{
	struct crypto_cipher_ctx *ctx = dupdate->ctx;

	return ctx->ops->update(ctx, dupdate->last, dupdate->src.data,
				dupdate->src.length, dupdate->dst.data);
}

static void nvt_cipherdrv_final(void *context)
{
	struct crypto_cipher_ctx *ctx = context;

	ctx->ops->final(ctx);
}

static void nvt_cipherdrv_copy_state(void *out, void *in)
{
	struct crypto_cipher_ctx *dst_ctx = out;
	struct crypto_cipher_ctx *src_ctx = in;

	src_ctx->ops->copy_state(dst_ctx, src_ctx);
}

static void nvt_cipherdrv_free_ctx(void *context)
{
	struct crypto_cipher_ctx *ctx = context;

	ctx->ops->free_ctx(ctx);
}
static struct drvcrypt_cipher driver_cipher = {
	.alloc_ctx = nvt_cipherdrv_alloc_ctx,
	.free_ctx = nvt_cipherdrv_free_ctx,
	.init = nvt_cipherdrv_init_ctx,
	.update = nvt_cipherdrv_update_ctx,
	.final = nvt_cipherdrv_final,
	.copy_state = nvt_cipherdrv_copy_state,
};

static TEE_Result nvt_cipher_drv_init(void)
{
	return drvcrypt_register_cipher(&driver_cipher);
}
driver_init_late(nvt_cipher_drv_init);
