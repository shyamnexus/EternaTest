#include <kernel/boot.h>
#include <string.h>
#include <tee/cache.h>
#include <sm/optee_smc.h>
#include <mm/core_mmu.h>
#include <mm/core_memprot.h>
#include <crypto/crypto.h>
#include <optee_msg.h>
#include <plat/ecdsa.h>

#include "../../include/crypto/nvt_ecdsa_smc.h"
#include "../../include/nvt_smc_util.h"

static int smc_ecdsa_open(NVT_SMC_ECDSA_DATA *ecdsa_data)
{
	int ret = 0;

	/* Open and Set Mode */
	ret = ecdsa_open();
	ret = ecc_set_parameters(ecdsa_data->key_w);

	return ret;
}

static int smc_ecdsa_update(NVT_SMC_ECDSA_DATA *ecdsa_data)
{
	int ret = 0;

	switch (ecdsa_data->ecdsa_mode)
	{
	case NVT_ECDSA_VERIFY: {

			unsigned char *src = NULL;
			unsigned char *r = NULL;
			unsigned char *s = NULL;
			unsigned char *x = NULL;
			unsigned char *y = NULL;
			unsigned int size = (ecdsa_data->key_w / 8);

			r = (unsigned char *)nvt_smc_pa_to_va((uintptr_t)ecdsa_data->sign_r, size);
			s = (unsigned char *)nvt_smc_pa_to_va((uintptr_t)ecdsa_data->sign_s, size);
			x = (unsigned char *)nvt_smc_pa_to_va((uintptr_t)ecdsa_data->key_pub_x, size);
			y = (unsigned char *)nvt_smc_pa_to_va((uintptr_t)ecdsa_data->key_pub_y, size);
			src = (unsigned char *)nvt_smc_pa_to_va((uintptr_t)ecdsa_data->src, 32);

			ret = ECDSA_VERIFY(src, r, s, x, y, &ecdsa_data->result, size);
		}
		break;

	case NVT_ECDSA_SIGN: {

			unsigned char *src = NULL;
			unsigned char *r = NULL;
			unsigned char *s = NULL;
			unsigned char *randk = NULL;
			unsigned int size = (ecdsa_data->key_w / 8);

			r = (unsigned char *)nvt_smc_pa_to_va((uintptr_t)ecdsa_data->sign_r, size);
			s = (unsigned char *)nvt_smc_pa_to_va((uintptr_t)ecdsa_data->sign_s, size);
			src = (unsigned char *)nvt_smc_pa_to_va((uintptr_t)ecdsa_data->src, 32);
			randk = (unsigned char *)nvt_smc_pa_to_va((uintptr_t)ecdsa_data->randk, size);

			ret = ECDSA_SIGN(src, randk, r, s);
		}		
		break;

	case NVT_ECDSA_GPK: {
		
			unsigned char *x = NULL;
			unsigned char *y = NULL;
			unsigned char *key_priv = NULL;
			unsigned int size = (ecdsa_data->key_w / 8);

			x = (unsigned char *)nvt_smc_pa_to_va((uintptr_t)ecdsa_data->key_pub_x, size);
			y = (unsigned char *)nvt_smc_pa_to_va((uintptr_t)ecdsa_data->key_pub_y, size);
			key_priv = (unsigned char *)nvt_smc_pa_to_va((uintptr_t)ecdsa_data->key_priv, size);

			ECDSA_set_privateKey(key_priv);
			ret = ECDSA_GPK(x,y);
		}
		break;
	
	default:
		break;
	}


	return ret;
}

static int smc_ecdsa_close(void)
{
	ecdsa_close();
	return 0;
}

int nvt_ivot_optee_ecdsa_operation(NVT_SMC_ECDSA_DATA *ecdsa_smc_data)
{
	int ret = 0;
	NVT_SMC_ECDSA_DATA *ecdsa_data = NULL;

	ecdsa_data = (NVT_SMC_ECDSA_DATA *)nvt_smc_pa_to_va((uintptr_t)ecdsa_smc_data, sizeof(NVT_SMC_ECDSA_DATA));
	if (!ecdsa_data) {
		EMSG("ecdsa_data pa=0x%08lx to va NULL\r\n", (uintptr_t)ecdsa_smc_data);
		return -1;
	}

	if (ecdsa_data->tag != SMC_ECDSA_TAG) {
		EMSG("check tag fail,tag:%x\r\n", ecdsa_data->tag);
		return -1;
	}

	switch (ecdsa_data->operation) {
		case NVT_SMC_ECDSA_OPERATION_OPEN:
			DMSG("NVT_SMC_ECDSA_OPERATION_OPEN\r\n");
			ret = smc_ecdsa_open(ecdsa_data);
			break;
		case NVT_SMC_ECDSA_OPERATION_UPDATE:
			DMSG("NVT_SMC_ECDSA_OEPRATION_UPDATE\r\n");
			ret = smc_ecdsa_update(ecdsa_data);
			break;
		case NVT_SMC_ECDSA_OPERATION_CLOSE:
			DMSG("NVT_SMC_ECDSA_OPERATION_FREE\r\n");
			ret = smc_ecdsa_close();
			break;
		default:
			EMSG("ecdsa operation fail (%x)\r\n",ecdsa_data->operation);
			ret = -1;
			break;
	}
	return ret;
}
