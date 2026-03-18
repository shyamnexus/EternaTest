/*
    RSA Framework driver

    Copyright Novatek Microelectronics Corp. 2023.  All rights reserved.
*/

#include <stdlib.h>
#include <string.h>
#include <kernel/panic.h>
#include <mbedtls/bignum.h>
#include "trace.h"
#include <utee_defines.h>
#include <util.h>
#include <crypto/crypto.h>
#include <plat/rsa.h>

#define DBG_IND(fmt, args...) //EMSG(fmt, ##args)
#define DBG_ERR(fmt, args...)   EMSG(fmt, ##args)
#define DBG_WRN(fmt, args...)   EMSG(fmt, ##args)
#define CHKPNT                  EMSG("CHK: %d, %s\r\n" , __LINE__, __func__)

enum public_key_type {
   /* Refers to the public key */
   PK_PUBLIC      = 0x0000,
   /* Refers to the private key */
   PK_PRIVATE     = 0x0001,

   /* Indicates standard output formats that can be read e.g. by OpenSSL or GnuTLS */
   PK_STD         = 0x1000,
   /* Indicates compressed public ECC key */
   PK_COMPRESSED  = 0x2000,
   /* Indicates ECC key with the curve specified by OID */
   PK_CURVEOID    = 0x4000
};

/** RSA PKCS style key */
typedef struct Rsa_key {
	/** Type of key, PK_PRIVATE or PK_PUBLIC */
	int type;
	/** The public exponent */
	void *e;
	/** The private exponent */
	void *d;
	/** The modulus */
	void *N;
	/** The p factor of N */
	void *p;
	/** The q factor of N */
	void *q;
	/** The 1/q mod p CRT param */
	void *qP;
	/** The d mod (p - 1) CRT param */
	void *dP;
	/** The d mod (q - 1) CRT param */
	void *dQ;
} rsa_key;

/**
   Compute an RSA modular exponentiation
   @param in         The input data to send into RSA
   @param inlen      The length of the input (octets)
   @param out        [out] The destination
   @param outlen     [in/out] The max size and resulting size of the output
   @param which      Which exponent to use, e.g. PK_PRIVATE or PK_PUBLIC
   @param key        The RSA key to use
   @return CRYPT_OK if successful
*/
int nvt_rsa_exptmod(const unsigned char *in, unsigned long inlen,
					unsigned char *out, unsigned long *outlen, int which,
					const rsa_key *key);

static int get_nvt_rsa_key_enum(unsigned long inlen, RSA_KEY *rsa_key_len)
{
	switch (inlen*8) {
	case 256:
		*rsa_key_len = RSA_KEY_256;
		return 0;
	case 512:
		*rsa_key_len = RSA_KEY_512;
		return 0;
	case 1024:
		*rsa_key_len = RSA_KEY_1024;
		return 0;
	case 2048:
		*rsa_key_len = RSA_KEY_2048;
		return 0;
#ifdef RSA4096_SUPPORT
	case 4096:
		*rsa_key_len = RSA_KEY_4096;
		return 0;
#endif
	default:
		return -1;
	}
}

static void nvt_bignum_bn2bin(const struct bignum *from, uint8_t *to, size_t to_len)
{
	size_t len = 0;

	len = crypto_bignum_num_bytes((struct bignum *)from);
	if (len > to_len) {
		DBG_ERR("len %d > to_len %d\r\n", (int)len, (int)to_len);
		return;
	}
	memcpy(to, ((mbedtls_mpi *)from)->p, len);
}

static void fill_data_r(UINT8 *s, UINT8 *target, INT len)
{
	int i;

	for(i=0; i<len; i++)
		target[len-i-1] = s[i];
}

int nvt_rsa_exptmod(const unsigned char *in, unsigned long inlen,
					unsigned char *out, unsigned long *outlen, int which,
					const rsa_key *key)
{
#define RSA_KEY_SIZE_MAX    (4096/8)
	int           ret = 0;
	RSA_KEY       nvt_rsa_key_enum;
	unsigned long rsa_word_len;
	UINT32        *hw_key = NULL;
	UINT32        *pt     = NULL;
	UINT32        *ct     = NULL;

	if (get_nvt_rsa_key_enum(inlen, &nvt_rsa_key_enum) < 0) {
		return -1;
	}

	hw_key = (UINT32 *)malloc(RSA_KEY_SIZE_MAX);
	if (!hw_key) {
		return -1;
	}
	pt = hw_key;
	ct = hw_key;

	rsa_open();
	rsa_setConfig(RSA_CONFIG_ID_KEY_WIDTH, nvt_rsa_key_enum);
	rsa_setConfig(RSA_CONFIG_ID_MODE,      RSA_MODE_NORMAL);
	rsa_word_len = inlen/4;
	nvt_bignum_bn2bin(key->N, (uint8_t *)hw_key, RSA_KEY_SIZE_MAX);
	rsa_setkey_n(hw_key, rsa_word_len, RSA_ORDER_LSB_FIRST);
	if (which == PK_PRIVATE) {
		DBG_IND("RSA decrypt inlen=%d\r\n", inlen);
		memset(hw_key, 0x00, inlen);
		nvt_bignum_bn2bin(key->d, (uint8_t *)hw_key, RSA_KEY_SIZE_MAX);
		rsa_setkey_ed(hw_key, rsa_word_len, RSA_ORDER_LSB_FIRST);
	}
	else if (which == PK_PUBLIC) {
		DBG_IND("RSA encrypt inlen=%d\r\n", inlen);
		memset(hw_key, 0x00, inlen);
		nvt_bignum_bn2bin(key->e, (uint8_t *)hw_key, RSA_KEY_SIZE_MAX);
		rsa_setkey_ed(hw_key, rsa_word_len, RSA_ORDER_LSB_FIRST);
	}
	else {
		DBG_ERR("Invalid public_key_type %d\r\n", (int)which);
		ret = -1;
		goto exit;
	}
	fill_data_r((UINT8 *)in, (UINT8 *)pt, inlen);
	rsa_pio_enable(pt, rsa_word_len, RSA_ORDER_LSB_FIRST);
	rsa_getOutput(ct, rsa_word_len);
	fill_data_r((UINT8 *)ct, (UINT8 *)out, inlen);
	*outlen = inlen;

exit:
	rsa_close();

	if (hw_key) {
		free(hw_key);
	}

	return ret;
}
