#ifndef _NVT_ECDSA_SMC_H
#define _NVT_ECDSA_SMC_H
#define MAKEFOURCC(ch0, ch1, ch2, ch3) ((UINT32)(UINT8)(ch0) | ((UINT32)(UINT8)(ch1) << 8) | ((UINT32)(UINT8)(ch2) << 16) | ((UINT32)(UINT8)(ch3) << 24))

#define SMC_ECDSA_TAG         MAKEFOURCC('E', 'C', 'D', 'S')

typedef enum _NVT_SMC_ECDSA_KEY_WIDTH_T {
	NVT_SMC_ECDSA_KEY_WIDTH_192 = 192,
	NVT_SMC_ECDSA_KEY_WIDTH_224 = 224,
	NVT_SMC_ECDSA_KEY_WIDTH_256 = 256,   
	NVT_SMC_ECDSA_KEY_WIDTH_384 = 384,                        
	NVT_SMC_ECDSA_KEY_WIDTH_MAX = NVT_SMC_ECDSA_KEY_WIDTH_384
}NVT_SMC_ECDSA_KEY_WIDTH;

typedef enum _NVT_SMC_ECDSA_OPERATION {
	NVT_SMC_ECDSA_OPERATION_OPEN,
	NVT_SMC_ECDSA_OPERATION_UPDATE,
	NVT_SMC_ECDSA_OPERATION_CLOSE,
	NVT_SMC_ECDSA_OPERATION_MAX
} NVT_SMC_ECDSA_OPERATION;

typedef enum _NVT_SMC_ECDSA_MODE{
	NVT_SMC_ECDSA_SMC_VERIFY,
	NVT_SMC_ECDSA_SMC_SIGN,
	NVT_SMC_ECDSA_SMC_GPK
} NVT_SMC_ECDSA_MODE;

typedef struct _NVT_SMC_ECDSA_DATA {
	unsigned int          tag;          //should be MAKEFOURCC('E', 'C', 'D', 'S'), customer no need to set
	NVT_SMC_ECDSA_MODE      ecdsa_mode;
	NVT_SMC_ECDSA_OPERATION operation;
	NVT_SMC_ECDSA_KEY_WIDTH  key_w;   ///< key width
	uint8_t *src;                       ///< input  hash
	uint8_t *key_priv;                  ///< set private key
	uint8_t *key_pub_x;                 ///< set public key x
	uint8_t *key_pub_y;                 ///< set public key y
	uint8_t *sign_r;                    ///< set sign r
	uint8_t *sign_s;                    ///< set sign s
	uint8_t *randk;                     ///< set random k
	bool result;
} NVT_SMC_ECDSA_DATA;

int nvt_ivot_optee_ecdsa_operation(NVT_SMC_ECDSA_DATA *ecdsa_smc_data, unsigned int key_size, unsigned int signature_size); 

#endif  /* _NVT_ECDSA_SMC_H */
