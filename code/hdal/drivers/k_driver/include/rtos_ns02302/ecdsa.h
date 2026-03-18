
#ifndef _ECDSA_H
#define _ECDSA_H

#include <kwrap/nvt_type.h>


enum key {
	Key0,
	Key1,
	Key2,
	Key3,
	Key4,
	Key5,
	Key6,
	Key7,
	Key8,
	Key9,
	Key10,
	Key11,

	Key_Total
};

typedef union {
	UINT8 msg[48];
	UINT32 key_msg[12];
} KEY_INFO;

typedef struct {
	UINT32 bits_type;
	UINT8 A_Value[48];
	UINT8 prime[48];
	UINT8 order[48];
	UINT8 base_x[48];
	UINT8 base_y[48];
} ECC_INFO;

typedef struct {
	UINT8 x[48];
	UINT8 y[48];
} PUBLIC_KEY_PAIR;

typedef struct {
	UINT8 r[48];
	UINT8 s[48];
} SIGNATURE_PAIR;

extern const ECC_INFO NVT_SECP384R1;
extern const ECC_INFO NVT_SECP256R1;
extern const ECC_INFO NVT_SECP224R1;
extern const ECC_INFO NVT_SECP192R1;

extern void ecdsa_clkSel(UINT32 clkSel);
extern ER ecdsa_open(UINT32 clkRate);
extern ER ecdsa_close(void);
extern ER ecc_set_parameters(UINT32 eccSize);
extern ER ECDSA_set_privateKey(UINT8* privateKey);
extern ER ECDSA_GPK(PUBLIC_KEY_PAIR* pubKey);
extern ER ECDSA_SIGN(UINT8* hash_msg, UINT8* randomK, SIGNATURE_PAIR* sign);
extern ER ECDSA_VERIFY(UINT8* hash_msg, SIGNATURE_PAIR* sign, PUBLIC_KEY_PAIR* pubKey, BOOL* result);

extern BOOL ecdsa_verify(UINT32 *hash_msg, UINT32 *pub_x, UINT32 *pub_y, UINT32 *sign_r, UINT32 *sign_s);

#endif