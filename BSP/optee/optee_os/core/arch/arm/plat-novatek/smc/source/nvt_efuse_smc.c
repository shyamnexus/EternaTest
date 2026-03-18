#include <kernel/tz_ssvce_pl310.h>
#include <kernel/cache_helpers.h>
#include <kernel/thread.h>
#include <mm/core_mmu.h>
#include <mm/core_memprot.h>
#include <sm/optee_smc.h>
#include "../include/nvt_efuse_smc.h"
#include "../include/nvt_smc_util.h"
#include <efuse_protected.h>
#include <tee/cache.h>
#include <optee_msg.h>
#include <crypto/crypto.h>

static int efuse_check_key_field(NVT_SMC_EFUSE_KEY_DATA *key_data)
{
	int ret=0;
	UINT32 query_option = SECUREBOOT_STATUS_NUM;
	if(key_data == NULL)
	{
		EMSG("key_data pa to va fail\r\n");
		return -1;
	}
	if(key_data->tag != SMC_EFUSE_KEY_TAG)
	{
		EMSG("check key data tag (%x) SMC_EFUSE_KEY_TAG:%x, fail \r\n",(int)key_data->tag,(int)SMC_EFUSE_KEY_TAG);
		return -1;
	}
	DMSG("key field:%d\r\n",key_data->field);

	switch(key_data->field) {
	case EFUSE_OTP_1ST_KEY_SET_FIELD:
		query_option = SECUREBOOT_1ST_KEY_SET_PROGRAMMED;
	break;

	case EFUSE_OTP_2ND_KEY_SET_FIELD:
		query_option = SECUREBOOT_2ND_KEY_SET_PROGRAMMED;
	break;

	case EFUSE_OTP_3RD_KEY_SET_FIELD:
		query_option = SECUREBOOT_3RD_KEY_SET_PROGRAMMED;
	break;

	case EFUSE_OTP_4TH_KEY_SET_FIELD:
		query_option = SECUREBOOT_4TH_KEY_SET_PROGRAMMED;
	break;

	case EFUSE_OTP_5TH_KEY_SET_FIELD:
		query_option = SECUREBOOT_5TH_KEY_SET_PROGRAMMED;
	break;

	default:
		query_option = SECUREBOOT_STATUS_NUM;
		EMSG("unknow check key field data %d\r\n", key_data->field);
		return -1;
	break;
	}

	if(quary_secure_boot(query_option))
		ret = 1;
	else
		ret = 0;
	return ret;
}

static int efuse_lock_read_key_field(NVT_SMC_EFUSE_KEY_DATA *key_data)
{
	int ret=0;
	if(key_data == NULL)
	{
		EMSG("key_data pa to va fail\r\n");
		return -1;
	}
	if(key_data->tag != SMC_EFUSE_KEY_TAG)
	{
		EMSG("check key data tag (%x) SMC_EFUSE_KEY_TAG:%x, fail \r\n",(int)key_data->tag,(int)SMC_EFUSE_KEY_TAG);
		return -1;
	}

	if(key_data->field < (UINT32)SECUREBOOT_READ_LOCK_KEY_SET_START || key_data->field > (UINT32)SECUREBOOT_READ_LOCK_KEY_SET_END) {
		EMSG("Read lock field error should be %d <= x <= %d\r\n",SECUREBOOT_READ_LOCK_KEY_SET_START, SECUREBOOT_READ_LOCK_KEY_SET_END);
		return -1;
	}
	DMSG("key field:%d\r\n",key_data->field);

	if(enable_secure_boot(key_data->field))
		ret = EFUSE_SUCCESS;
	else
		ret = -1;
	return ret;

}

static int efuse_lock_engine_read_key_field(NVT_SMC_EFUSE_KEY_DATA *key_data)
{
	int ret =0;
	if(key_data == NULL)
	{
		EMSG("key_data pa to va fail\r\n");
		return -1;
	}
	if(key_data->tag != SMC_EFUSE_KEY_TAG)
	{
		EMSG("check key data tag (%x) SMC_EFUSE_KEY_TAG:%x, fail \r\n",key_data->tag,SMC_EFUSE_KEY_TAG);
		return -1;
	}
	DMSG("key field:%d will not allow RSA/HASH/Crypto to use\r\n",key_data->field);

	otp_set_key_engine_access_right(key_data->field);
	//charlie to_do
	//need modify to lock engine read key field
	ret = 0;
	//return value--> 0: ok, -1: fail
	//end charlie do_to
	return ret;
}

static int smc_efuse_write_key(NVT_SMC_EFUSE_KEY_DATA *key_data)
{
	if(key_data->tag != SMC_EFUSE_KEY_TAG)
	{
		EMSG("check key data tag (%x), fail \r\n",key_data->tag);
		return -1;
	}
	return otp_write_key(key_data->field, key_data->data);
}

static int efuse_read_key_field(NVT_SMC_EFUSE_KEY_DATA *key_data)
{

	unsigned int field_index = 0;
	unsigned int key_value =0;
	int i=0;
	int ret=0;
	if(key_data == NULL)
	{
		EMSG("key_data pa to va fail\r\n");
		return -1;
	}
	if(key_data->tag != SMC_EFUSE_KEY_TAG)
	{
		EMSG("check key data tag (%x) SMC_EFUSE_KEY_TAG:%x, fail \r\n",key_data->tag,SMC_EFUSE_KEY_TAG);
		return -1;
	}
	switch(key_data->field)
	{
		case EFUSE_OTP_1ST_KEY_SET_FIELD:
			field_index = 16;
			break;
		case EFUSE_OTP_2ND_KEY_SET_FIELD:
			field_index = 20;
			break;
		case EFUSE_OTP_3RD_KEY_SET_FIELD:
			field_index = 24;
			break;
		case EFUSE_OTP_4TH_KEY_SET_FIELD:
			field_index = 28;
			break;
		case EFUSE_OTP_5TH_KEY_SET_FIELD:
			field_index = 12;
			break;
		default:
			EMSG("key field error :%x\n",key_data->field);
			return -1;
	}
	for(i=0; i< 4;i++)
	{
		key_value = otp_key_manager(field_index + i);
		key_data->data[i*4] = (unsigned char)(key_value & 0x000000ff);
		key_data->data[i*4+1] = (unsigned char)((key_value & 0x0000ff00)>>8);
		key_data->data[i*4+2] = (unsigned char)((key_value & 0x00ff0000)>>16);
		key_data->data[i*4+3] = (unsigned char)((key_value & 0xff000000)>>24);
	}
	#if 1
	DMSG("key value:%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\r\n",
		key_data->data[0],key_data->data[1],key_data->data[2],key_data->data[3],
		key_data->data[4],key_data->data[5],key_data->data[6],key_data->data[7],
		key_data->data[8],key_data->data[9],key_data->data[10],key_data->data[11],
		key_data->data[12],key_data->data[13],key_data->data[14],key_data->data[15]);
	#endif
	ret = cache_operation(TEE_CACHEFLUSH, (void *)key_data, sizeof(NVT_SMC_EFUSE_KEY_DATA));
	if(ret != TEE_SUCCESS)
	{
		EMSG("cache_operation TEE_CACHEFLUSH fail ret:%x\n",ret);
	}
	return ret;
}

static int smc_efuse_compare_key(NVT_SMC_EFUSE_KEY_DATA *key_data)
{
	unsigned char * input_data = NULL;
	unsigned int field_index = 0;
	unsigned key_value[4]={0};

	if(key_data == NULL)
	{
		EMSG("key_data pa to va fail\r\n");
		return -1;
	}
	if(key_data->tag != SMC_EFUSE_KEY_TAG)
	{
		EMSG("check key data tag (%x) SMC_EFUSE_KEY_TAG:%x, fail \r\n",key_data->tag,SMC_EFUSE_KEY_TAG);
		return -1;
	}

	input_data = key_data->data;
	if(input_data == NULL)
	{
		EMSG("key data pa to va fail\r\n");
		return -1;
	}
	switch(key_data->field)
	{
		case EFUSE_OTP_1ST_KEY_SET_FIELD:
			field_index = 16;
			break;
		case EFUSE_OTP_2ND_KEY_SET_FIELD:
			field_index = 20;
			break;
		case EFUSE_OTP_3RD_KEY_SET_FIELD:
			field_index = 24;
			break;
		case EFUSE_OTP_4TH_KEY_SET_FIELD:
			field_index = 28;
			break;
		case EFUSE_OTP_5TH_KEY_SET_FIELD:
			field_index = 12;
			break;
		default:
			EMSG("key field error :%x\n",key_data->field);
			return -1;
	}
	key_value[0] = input_data[0] | input_data[1] << 8 | input_data[2] << 16 | input_data[3] << 24;
	key_value[1] = input_data[4] | input_data[5] << 8 | input_data[6] << 16 | input_data[7] << 24;
	key_value[2] = input_data[8] | input_data[9] << 8 | input_data[10] << 16 | input_data[11] << 24;
	key_value[3] = input_data[12] | input_data[13] << 8 | input_data[14] << 16 | input_data[15] << 24;
	if(key_value[0] != otp_key_manager(field_index) || key_value[1] != otp_key_manager(field_index+1) ||
			key_value[2] != otp_key_manager(field_index+2) || key_value[3] != otp_key_manager(field_index+3)){

		EMSG("key compare fail\n");
		DMSG("efuse key: %x %x %x %x\n",
			otp_key_manager(field_index), otp_key_manager(field_index+1),
			otp_key_manager(field_index+2),otp_key_manager(field_index+3));
                DMSG("compare key: %x %x %x %x\n",key_value[0],key_value[1],key_value[2],key_value[3]);
                return 0;
        }
	return 1; //1: true , 0: false, others: error

}


static int smc_efuse_trigger_key_set(NVT_SMC_EFUSE_KEY_DATA *key_data)
{
	if(key_data->tag != SMC_EFUSE_KEY_TAG)
	{
		EMSG("check key data tag (%x), fail \r\n",key_data->tag);
		return -1;
	}

	return otp_set_key_destination(OTP_KEY_MANAGER_CRYPTO, key_data->field * 4, 4);
}

static int smc_efuse_rng_random(NVT_SMC_EFUSE_KEY_DATA *key_data)
{
	UINT32 test_buf;
	if(key_data->tag != SMC_EFUSE_KEY_TAG)
	{
		EMSG("check key data tag (%x), fail \r\n",key_data->tag);
		return -1;
	}

	if(crypto_rng_read(&test_buf, 4) == TEE_SUCCESS) {
		EMSG("random = 0x%08x\r\n", test_buf);
		return TEE_SUCCESS;
	} else {
		return -1;
	}
}


int nvt_ivot_optee_efuse_operation(NVT_SMC_EFUSE_DATA *efuse_smc_data)
{
	int ret=0;
	NVT_SMC_EFUSE_DATA *efuse_data = (NVT_SMC_EFUSE_DATA *)nvt_smc_pa_to_va((uintptr_t)efuse_smc_data, sizeof(NVT_SMC_EFUSE_DATA));
	if (efuse_data == NULL)
	{
		EMSG("efuse data pa to va fail\r\n");
		return -1;
	}
	if(efuse_data->tag != SMC_EFUSE_TAG)
	{
		EMSG("check tag fail,tag:%x\r\n",efuse_data->tag);
		return -1;
	}
	switch(efuse_data->cmd)
	{
		case NVT_SMC_EFUSE_IS_SECURE:
			DMSG("NVT_SMC_EFUSE_IS_SECURE\r\n");
			ret = is_secure_enable();
			break;
		case NVT_SMC_EFUSE_IS_DATA_ENCRYPTED:
			DMSG("NVT_SMC_EFUSE_IS_DATA_ENCRYPTED\r\n");
			ret = is_data_area_encrypted();
			break;
		case NVT_SMC_EFUSE_IS_RSA_KEY_CHECK:
			DMSG("NVT_SMC_EFUSE_IS_RSA_KEY_CHECK\r\n");
			ret = is_signature_rsa_chsum_enable();
			break;
		case NVT_SMC_EFUSE_IS_SIGNATURE_RSA:
			DMSG("NVT_SMC_EFUSE_IS_SIGNATURE_RSA\r\n");
			ret = is_signature_rsa();
			break;
		case NVT_SMC_EFUSE_WRITE_KEY:
			DMSG("NVT_SMC_EFUSE_WRITE_KEY\r\n");
			ret = smc_efuse_write_key(&efuse_data->key_data);
			break;
		case NVT_SMC_EFUSE_COMPARE_KEY:
			DMSG("NVT_SMC_EFUSE_COMPARE_KEY\r\n");
			ret = smc_efuse_compare_key(&efuse_data->key_data);
			break;
		case NVT_SMC_EFUSE_ENABLE_SECURE:
			DMSG("NVT_SMC_EFUSE_ENABLE_SECURE\r\n");
			efuse_secure_en();
			break;
		case NVT_SMC_EFUSE_ENABLE_SIGNATURE_RSA:
			DMSG("NVT_SMC_EFUSE_ENABLE_SIGNATURE_RSA\r\n");
			efuse_signature_rsa_en();
			break;
		case NVT_SMC_EFUSE_ENABLE_DATA_ENCRYPTED:
			DMSG("NVT_SMC_EFUSE_ENABLE_DATA_ENCRYPTED\r\n");
			efuse_data_area_encrypt_en();
			break;
		case NVT_SMC_EFUSE_ENABLE_RSA_KEY_CHECK:
			DMSG("NVT_SMC_EFUSE_ENABLE_RSA_KEY_CHECK\r\n");
			efuse_signature_rsa_chksum_en();
			break;
		case NVT_SMC_EFUSE_CHECK_KEY_FIELD:
			DMSG("NVT_SMC_EFUSE_CHECK_KEY_FIELD\r\n");
			ret = efuse_check_key_field(&efuse_data->key_data);
			break;
		case NVT_SMC_EFUSE_READ_KEY_FIELD:
			DMSG("NVT_SMC_EFUSE_READ_KEY_FIELD\r\n");
			ret = efuse_read_key_field(&efuse_data->key_data);
			break;
		case NVT_SMC_EFUSE_LOCK_READ_KEY_FIELD:
			DMSG("NVT_SMC_EFUSE_LOCK_READ_KEY_FIELD\r\n");
			ret = efuse_lock_read_key_field(&efuse_data->key_data);
			break;
		case NVT_SMC_EFUSE_LOCK_ENGINE_READ_KEY_FIELD:
			DMSG("NVT_SMC_EFUSE_LOCK_ENGINE_READ_KEY_FIELD\r\n");
			ret = efuse_lock_engine_read_key_field(&efuse_data->key_data);
			break;
		case NVT_SMC_EFUSE_TRIGGER_KEY_SET:
			DMSG("NVT_SMC_EFUSE_TRIGGER_KEY_SET\r\n");
			ret = smc_efuse_trigger_key_set(&efuse_data->key_data);
			break;

		case NVT_SMC_EFUSE_TRNG:
			DMSG("NVT_SMC_EFUSE_TRNG\r\n");
			ret = smc_efuse_rng_random(&efuse_data->key_data);
			break;
		default:
			EMSG("efuse cmd(%x) error\r\n",efuse_data->cmd);
			ret =-1;
			break;

	}
	return ret;
}

