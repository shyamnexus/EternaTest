
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

extern ER ecdsa_open(void);
extern ER ecdsa_close(void);
extern ER ecc_set_parameters(UINT32 eccSize);
extern ER ECDSA_set_privateKey(UINT8* privateKey);
extern ER ECDSA_GPK(UINT8* pubKey_x, UINT8* pubKey_y);
extern ER ECDSA_SIGN(UINT8* hash_msg, UINT8* randomK, UINT8* sign_r, UINT8* sign_s);
extern ER ECDSA_VERIFY(UINT8* hash_msg, UINT8* sign_r, UINT8* sign_s, UINT8* pubKey_x, UINT8* pubKey_y, bool* result, unsigned int key_size);

#endif
