/*
    HMAC Framework driver

    Copyright Novatek Microelectronics Corp. 2023.  All rights reserved.
*/

#include <assert.h>
#include <platform_config.h>
#include <crypto/crypto.h>
#include <crypto/crypto_impl.h>
#include <stdlib.h>
#include <string.h>
#include <tee_api_types.h>
#include <utee_defines.h>
#include <util.h>
#include <plat/hash.h>
#include <drvcrypt.h>
#include <drvcrypt_mac.h>
#include <initcall.h>

#define DBG_IND(fmt, args...) //EMSG(fmt, ##args)
#define DBG_ERR(fmt, args...)   EMSG(fmt, ##args)
#define DBG_WRN(fmt, args...)   EMSG(fmt, ##args)
#define CHKPNT                  EMSG("CHK: %d, %s\r\n" , __LINE__, __func__)

/******************************************************************************
 * Message digest functions
 ******************************************************************************/

extern int hash_memory(int hash, const unsigned char *in, unsigned long inlen, unsigned char *out, unsigned long *outlen);
extern int find_hash(const char *name);

#define CACHE_LINE_ALIGN         STACK_ALIGNMENT        ///< platform cache line size
#define CACHE_LINE_MASK          (CACHE_LINE_ALIGN-1)
#define HASH_TMP_BUF_SIZE        0x1000
#define HASH_RESV_PADDING_SIZE   128

#define INVWORD(B) ((UINT32) (((UINT32)((B)[0]))<<24 | ((UINT32)((B)[1]))<<16 | ((UINT32)((B)[2]))<< 8 | ((UINT32)((B)[3]))))

typedef struct Hmac_state {
	uint32_t    total;          ///< The number of Bytes processed
	uint32_t    state[8];       ///< digest
	uint32_t    hw_iv[8];
	uint32_t    key[16];
	uint32_t    trigger_count;
	char        buf[64+HASH_RESV_PADDING_SIZE] __aligned(4);
} hmac_state;

struct nvt_hmac_ctx {
	struct crypto_mac_ctx   ctx;
	size_t                  hash_size;
	HASH_MODE               hash_mode;
	int                     hash_idx;
	struct Hmac_state       sha;
};

static __aligned(CACHE_LINE_ALIGN) uint8_t hmac_tmp_buf[HASH_TMP_BUF_SIZE];

static const struct crypto_mac_ops nvt_hmac_ops;

static struct nvt_hmac_ctx *to_hmac_ctx(struct crypto_mac_ctx *ctx)
{
	assert(ctx && ctx->ops == &nvt_hmac_ops);

	return container_of(ctx, struct nvt_hmac_ctx, ctx);
}

static TEE_Result nvt_hmac_init(struct crypto_mac_ctx *ctx, const uint8_t *key, size_t len)
{
	struct nvt_hmac_ctx *hc = to_hmac_ctx(ctx);
	unsigned long outlen;

	hc->sha.total         = 0;
	hc->sha.trigger_count = 0;

	if (key == NULL) {
		return TEE_ERROR_BAD_PARAMETERS;
	}
	if (len > sizeof(hc->sha.key)) {
		outlen = sizeof(hc->sha.key);
		hash_memory(hc->hash_idx, key, len, (unsigned char *)&hc->sha.key, &outlen);
	}
	else {
		memcpy(hc->sha.key, key, len);
	}
	return TEE_SUCCESS;
}

static TEE_Result nvt_hw_hmac_trigger(struct crypto_mac_ctx *ctx, uint8_t *data, size_t len, size_t pad_len)
{
	struct nvt_hmac_ctx *hc = to_hmac_ctx(ctx);
	BOOL    is_in_tmp = FALSE;
	uint8_t *src;

	if (((uintptr_t)data) & 0x3) {
		if (len > HASH_TMP_BUF_SIZE) {
			DBG_ERR("in addr=0x%08lx not word align\r\n", (uintptr_t)data);
			return TEE_ERROR_BAD_PARAMETERS;
		}
		is_in_tmp = TRUE;
	}
	DBG_IND("hash_mode=%d data=0x%08lx len=%d pad_len=%d\r\n", hc->hash_mode, (uintptr_t)data, (int)len, (int)pad_len);

	hash_open();

	if (is_in_tmp) {
		src = hmac_tmp_buf;
		memcpy((void *)src, data, len);
	}
	else {
		src = (uint8_t *)data;
	}
	if (hc->sha.trigger_count == 0) {
		hash_setConfig(HASH_CONFIG_ID_IV_SEL,  HASH_IV_Default);
		hash_setConfig(HASH_CONFIG_ID_IPAD,    HASH_IPAD_STATE);
		hash_setKey((UINT8 *)hc->sha.key, 64);
	}
	else {
		hash_setConfig(HASH_CONFIG_ID_IV_SEL,  HASH_IV_REG);
		hash_setConfig(HASH_CONFIG_ID_IPAD,    HASH_IPAD_NORMAL);
#if defined(_BSP_NA51055_)  ///< IV need to swap
		hc->sha.hw_iv[0] = INVWORD((UINT8 *)hc->sha.state);
		hc->sha.hw_iv[1] = INVWORD((UINT8 *)hc->sha.state+4);
		hc->sha.hw_iv[2] = INVWORD((UINT8 *)hc->sha.state+8);
		hc->sha.hw_iv[3] = INVWORD((UINT8 *)hc->sha.state+12);
		hc->sha.hw_iv[4] = INVWORD((UINT8 *)hc->sha.state+16);
		hc->sha.hw_iv[5] = INVWORD((UINT8 *)hc->sha.state+20);
		hc->sha.hw_iv[6] = INVWORD((UINT8 *)hc->sha.state+24);
		hc->sha.hw_iv[7] = INVWORD((UINT8 *)hc->sha.state+28);
#else
		hc->sha.hw_iv[0] = hc->sha.state[0];
		hc->sha.hw_iv[1] = hc->sha.state[1];
		hc->sha.hw_iv[2] = hc->sha.state[2];
		hc->sha.hw_iv[3] = hc->sha.state[3];
		hc->sha.hw_iv[4] = hc->sha.state[4];
		hc->sha.hw_iv[5] = hc->sha.state[5];
		hc->sha.hw_iv[6] = hc->sha.state[6];
		hc->sha.hw_iv[7] = hc->sha.state[7];
#endif
		hash_setIV((UINT8 *)hc->sha.hw_iv);
	}
	hash_setConfig(HASH_CONFIG_ID_BYPASS,  HASH_BYPASS_NORMAL);
	hash_setConfig(HASH_CONFIG_ID_ACCMODE, HASH_ACCMODE_DMA);
	hash_setConfig(HASH_CONFIG_ID_MODE,    hc->hash_mode);
	if (pad_len) {
		hash_setConfig(HASH_CONFIG_ID_DMA_PAD, HASH_DMA_PAD_ENABLE);
		hash_setConfig(HASH_CONFIG_ID_OPAD,    HASH_OPAD_STATE);
	} else {
		hash_setConfig(HASH_CONFIG_ID_DMA_PAD, HASH_DMA_PAD_DISABLE);
		hash_setConfig(HASH_CONFIG_ID_OPAD,    HASH_OPAD_NORMAL);
	}
	hash_dma_enable((vaddr_t)src, (UINT32)len, (UINT32)pad_len);
	hash_getOutput((UINT8 *)hc->sha.state, (UINT32)hc->hash_size);

	hc->sha.trigger_count++;

	hash_close();

	return TEE_SUCCESS;
}

static TEE_Result nvt_hmac_update(struct crypto_mac_ctx *ctx, const uint8_t *data, size_t len)
{
	struct nvt_hmac_ctx *hc = to_hmac_ctx(ctx);
	size_t   align_len;
	int      ret;
	size_t   fill;
	uint32_t left;

	if (!len)
		return TEE_SUCCESS;

	left = hc->sha.total & 0x3F;
	fill = 64 - left;
	hc->sha.total += (uint32_t) len;
	if (left && len >= fill) {
		memcpy((void *)(hc->sha.buf + left), data, fill);
		if ((ret = nvt_hw_hmac_trigger(ctx, (uint8_t *)hc->sha.buf, 64, 0)) != 0)
			return ret;

		data += fill;
		len  -= fill;
		left  = 0;
	}
	if (len >= 64) {
		align_len = len & (~0x3f);
		if((ret = nvt_hw_hmac_trigger(ctx, (uint8_t *)data, align_len, 0)) != 0)
			return ret;

		data += align_len;
		len  -= align_len;
	}
	if (len > 0) {
		memcpy((void *) (hc->sha.buf + left), data, len);
	}
	return TEE_SUCCESS;
}

static TEE_Result nvt_hmac_final(struct crypto_mac_ctx *ctx, uint8_t *digest, size_t len)
{
	struct nvt_hmac_ctx *hc = to_hmac_ctx(ctx);
	size_t left;

	if (len == 0) {
		return TEE_ERROR_BAD_PARAMETERS;
	}
	if (len > hc->hash_size) {
		len = hc->hash_size;
	}
	left = hc->sha.total & 0x3F;
	nvt_hw_hmac_trigger(ctx, (uint8_t *)hc->sha.buf, (UINT32)left, hc->sha.total);
	memcpy(digest, hc->sha.state, len);
	return TEE_SUCCESS;
}

static void nvt_hmac_free_ctx(struct crypto_mac_ctx *ctx)
{
	free(to_hmac_ctx(ctx));
}

static void nvt_hmac_copy_state(struct crypto_mac_ctx *dst_ctx, struct crypto_mac_ctx *src_ctx)
{
	struct nvt_hmac_ctx *src = to_hmac_ctx(src_ctx);
	struct nvt_hmac_ctx *dst = to_hmac_ctx(dst_ctx);

	assert(src->hash_mode == dst->hash_mode);
	dst->sha = src->sha;
}

static const struct crypto_mac_ops nvt_hmac_ops = {
	.init       = nvt_hmac_init,
	.update     = nvt_hmac_update,
	.final      = nvt_hmac_final,
	.free_ctx   = nvt_hmac_free_ctx,
	.copy_state = nvt_hmac_copy_state,
};

static TEE_Result nvt_hmac_alloc_ctx(struct crypto_mac_ctx **ctx_ret, int hash_mode)
{
	struct nvt_hmac_ctx *ctx = NULL;

	ctx = calloc(1, sizeof(*ctx));
	if (!ctx)
		return TEE_ERROR_OUT_OF_MEMORY;

	ctx->ctx.ops = &nvt_hmac_ops;
	if (hash_mode == HASH_MODE_HMAC_SHA1) {
		ctx->hash_size = TEE_SHA1_HASH_SIZE;
		ctx->hash_idx  = find_hash("sha1");
	}
	else {
		ctx->hash_size = TEE_SHA256_HASH_SIZE;
		ctx->hash_idx  = find_hash("sha256");
	}
	ctx->hash_mode = hash_mode;
	*ctx_ret = &ctx->ctx;
	return TEE_SUCCESS;
}

static TEE_Result nvt_macdrv_alloc_ctx(struct crypto_mac_ctx **ctx, uint32_t algo)
{
	TEE_Result res = TEE_ERROR_NOT_IMPLEMENTED;

	switch (algo) {
		case TEE_ALG_HMAC_SHA1:
			res = nvt_hmac_alloc_ctx(ctx, HASH_MODE_HMAC_SHA1);
			break;
		case TEE_ALG_HMAC_SHA256:
			res = nvt_hmac_alloc_ctx(ctx, HASH_MODE_HMAC_SHA256);
			break;
		default:
			return TEE_ERROR_NOT_IMPLEMENTED;
	}
	return res;
}

static TEE_Result nvt_hmac_drv_init(void)
{
	return drvcrypt_register_hmac(&nvt_macdrv_alloc_ctx);
}

driver_init_late(nvt_hmac_drv_init);
