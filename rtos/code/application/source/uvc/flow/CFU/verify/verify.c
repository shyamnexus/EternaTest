#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/aes.h"
#include "include/rsa.h"
#include "include/bignum.h"
#include "include/ctr_drbg.h"
#include "include/entropy.h"
#include "include/sha256.h"
#include "include/ecp.h"
#include "include/ecdsa.h"
#include "FileSysTsk.h"
#include "hid_def.h"
#include "PStore.h"
#include "nvtpack.h"
#include "lz.h"
#include "bin_info.h"
#include "dma.h"
#include "prjcfg.h"
#include "flow_updfw.h"
#include "kwrap/semaphore.h"
#include <compiler.h>
#include <kwrap/debug.h>
///////////////////////////////////////////////////////////////////////////////

#define ENCRYPT_CHECK       1
#define RSA_ENCRYPT_SIZE	256
#define SHA256_ALIGN	    0x40
#define ECPARAMS            MBEDTLS_ECP_DP_SECP256R1
#define FW_BUF_LEN          3*1024*1024
#define BIG_NUM_BUF_LEN     2160000
/* ==================================== */

//firmware's cfu header. It's 1024 bytes long. Bytes not covered by these fields are signature data
struct hash_file_header{
	unsigned int  version;
	int           size;
	int           hash_offset;
	int           hash_length;
	int           reserve[64];
};

static char release_public_key[65] = {0x04, 0x17, 0x22, 0xc6, 0x2f, 0x28, 
									  0x9a, 0x5c, 0x05, 0xb6, 0x2b, 0x42, 
									  0x52, 0xa6, 0x93, 0xb2, 0xdb, 0x8e, 
									  0x6c, 0x5f, 0x12, 0xd6, 0xd8, 0x35, 
									  0xd6, 0x82, 0x6a, 0xae, 0xa0, 0x7e, 
									  0xbd, 0x65, 0xef, 0xde, 0x1d, 0xab, 
									  0xb7, 0x09, 0xa2, 0xbd, 0x02, 0x9d, 
									  0xeb, 0x2a, 0x56, 0x20, 0x3e, 0x3c, 
									  0x9d, 0x09, 0xb9, 0xd0, 0x35, 0x2b, 
									  0x20, 0xcd, 0x25, 0xa5, 0x39, 0xe7, 
									  0x90, 0x29, 0x79, 0x32, 0x30};//these values are used only in the 1st boot where pstore has no key section

//bakcup debug/release public key because mbedtls's do_verify() is suspected to destroy public key
static char bak_release_public_key[sizeof(release_public_key)] = {0};

static unsigned char *firmware = NULL;
static unsigned char *bignum = NULL;
static int firmware_length = 0;
static char hash_data[1024] = {0};
static VERSION_CTRL backup_bank_info = {0}; //copy m_boot_bank_info to avoid accessing m_boot_bank_info from multithread
static BOOL cfu_abort_burn = 0;
static UINT32 swap_pending = 0;//if an update is successful, reject all subsequent updates until next reboot
static UINT32 offer_got = 0;//if an update offer is never gotten, reject update offer content

extern SEM_HANDLE CFU_SEM_ID;
extern VERSION_CTRL m_boot_bank_info;

UINT32 cfu_init(void)
{
	PPSTORE_SECTION_HANDLE  pSection;
	UINT32  result = 1;
	
	SEM_WAIT(CFU_SEM_ID);

	memcpy(&backup_bank_info, &m_boot_bank_info, sizeof(VERSION_CTRL));
	memset(&bak_release_public_key, 0, sizeof(bak_release_public_key));
	firmware_length = 0;
	memset(hash_data, 0, sizeof(hash_data));
	cfu_abort_burn = 0;
	swap_pending = 0;
	offer_got = 0;

	//load public release key from cfu pstore
	if ((pSection = PStore_OpenSection("CfuRelKey", PS_RDWR)) != E_PS_SECHDLER) {
		result = PStore_ReadSection((UINT8*)release_public_key, 0, sizeof(release_public_key), pSection);
		PStore_CloseSection(pSection);
		if (result != E_PS_OK){
			DBG_ERR("fail to read public release key\r\n");
			goto exit;
		}
	}else{//if not exist, create new section for release key
#ifdef _MODEL_UVCCAM5_517L_DUAL_EVB_ 
		DisableNORWriteProtect();
#endif
		if ((pSection = PStore_OpenSection("CfuRelKey", PS_RDWR | PS_CREATE)) != E_PS_SECHDLER) {
			result = PStore_WriteSection((UINT8*)release_public_key, 0, sizeof(release_public_key), pSection);
			PStore_CloseSection(pSection);
#ifdef _MODEL_UVCCAM5_517L_DUAL_EVB_ 
			EnableNORWriteProtect();
#endif
			if (result != E_PS_OK){
				DBG_ERR("fail to write cfu release public key to pstore\r\n");
				goto exit;
			}
		}else{
			DBG_ERR("fail to create cfu release public key in pstore\r\n");
#ifdef _MODEL_UVCCAM5_517L_DUAL_EVB_ 
			EnableNORWriteProtect();
#endif
			goto exit;
		}
	}

	//bakcup release public key because mbedtls's do_verify() is suspected to destroy public key
	memcpy(bak_release_public_key, release_public_key, sizeof(release_public_key));
	
	result = 0;
	
exit:

	//if fail, zero-out
	if(result){
		memset(&backup_bank_info, 0, sizeof(VERSION_CTRL));
		memset(&release_public_key, 0, sizeof(release_public_key));
		memset(&bak_release_public_key, 0, sizeof(bak_release_public_key));
		firmware_length = 0;
		memset(hash_data, 0, sizeof(hash_data));
		cfu_abort_burn = 0;
		swap_pending = 0;
		offer_got = 0;
	}

	SEM_SIGNAL(CFU_SEM_ID);

	return result;
}

static int _cfu_setup_common_buffer(void)
{	
	if(firmware){
		return 0;
	}else{
		firmware = malloc(FW_BUF_LEN);
		if(!firmware){
			DBG_ERR("fail to allocate %d bytes for firmware buffer\n", FW_BUF_LEN);
			return -1;
		}
	}
	
	memset(firmware, 0, FW_BUF_LEN);
	
	return 0;
}

UINT32 _cfu_disable(void)
{
	if(firmware)
		free(firmware);

	if(bignum)
		free(bignum);

	firmware = NULL;
	bignum = NULL;

	return 0;
}

UINT32 cfu_get_firmware_version(void)
{
	int version;

	SEM_WAIT(CFU_SEM_ID); //lock against frimware burning is almost done(verify_and_burn_firmware())
	version = backup_bank_info.fw_version;
	SEM_SIGNAL(CFU_SEM_ID);

	return version;
}

static int validate_version(unsigned int version)
{
	unsigned int release = ((version & 0x0C)>>2);
	unsigned int test    = (version & 0x03);

	if(test == 0x02){
		DBG_ERR("test bits(bit0,bit1) can only be 00 or 01 or 11. 10 is not supported\n");
		return -1;
	}

	if(release != 0x00 && release != 0x02){
		DBG_ERR("release bits(bit3,bit2) can only be 00 or 10. current is %x\n", release);
		return -1;
	}

	return 0;
}

UINT32 cfu_process_update_offer(CFU_UPDATE_OFFER *offer, CFU_UPDATE_OFFER_RESP *resp)
{
	unsigned int new_version, old_version, is_new_fw_dbg, is_old_fw_dbg;
	CFU_UPDATE_SPECIAL_OFFER *special = NULL;
	UINT8 token;

	if(!offer || !resp){
		DBG_ERR("offer(%p) or resp(%p) is NULL\r\n", offer, resp);
		return FIRMWARE_UPDATE_OFFER_REJECT;
	}

	token = offer->token;

	//always run special offer information before non-special offer information
	//because "end offer list" special offer information will be received after update content
	//flow check should not fail "end offer list" special offer information
	if(offer->component_id == 0xFF){
		DBG_DUMP("special offer information packet\r\n");
		special = (CFU_UPDATE_SPECIAL_OFFER*)offer;
		resp->token = special->token;

		if(special->information_code == OFFER_INFO_START_ENTIRE_TRANSACTION){
			DBG_DUMP("OFFER_INFO_START_ENTIRE_TRANSACTION\r\n");
		}else if(special->information_code == OFFER_INFO_START_OFFER_LIST){
			DBG_DUMP("OFFER_INFO_START_OFFER_LIST\r\n");
		}else if(special->information_code == OFFER_INFO_END_OFFER_LIST){
			DBG_DUMP("OFFER_INFO_END_OFFER_LIST\r\n");
		}else{
			DBG_ERR("special information code(%x) is not supported\r\n", special->information_code);
		}

		return FIRMWARE_UPDATE_OFFER_ACCEPT;
	}else if(offer->component_id != 0x01){
		DBG_ERR("component id(%x) is not supported\r\n", offer->component_id);
		resp->RR = ((token << 24) | FIRMWARE_OFFER_REJECT_INV_COMPONENT);
		return FIRMWARE_UPDATE_OFFER_REJECT;
	}

	//below code should be run if this offer is not special offer information

	if((offer->protocol_ver & 0x0F) != 0x4){
		DBG_ERR("invalid protocol version(%x), valid is 2\r\n", offer->protocol_ver & 0x0F);
		return FIRMWARE_UPDATE_CMD_NOT_SUPPORTED;
	}

	if(validate_version(offer->fw_ver)){
		DBG_ERR("version(0x%x) is invalid\n", offer->fw_ver);
		resp->RR = ((token << 24) | FIRMWARE_OFFER_REJECT_OLD_FW);
		return FIRMWARE_UPDATE_OFFER_REJECT;
	}

	old_version   = (backup_bank_info.fw_version & 0xFFFFFF00);
	is_old_fw_dbg = ((backup_bank_info.fw_version & 0x0C) ? 0 : 1);
	new_version   = (offer->fw_ver & 0xFFFFFF00);
	is_new_fw_dbg = ((offer->fw_ver & 0x0C) ? 0 : 1);
	DBG_DUMP("new ver(%x) , old ver(%x)\r\n", offer->fw_ver, backup_bank_info.fw_version);

	//an update was previously finished, don't allow subsequent updates until next reboot
	if(swap_pending){
		DBG_DUMP("a swap is pending...can't do cfu\r\n");
		resp->RR = ((token << 24) | FIRMWARE_UPDATE_OFFER_SWAP_PENDING);
		return FIRMWARE_UPDATE_OFFER_REJECT;
	}

	//version check with regard to spec's rule
	if(is_new_fw_dbg){//new firmware is debug version
		if(!is_old_fw_dbg && old_version != new_version){//debug firmware can't replace release firmware if versions are diffirent
			DBG_ERR("new firmware is debug version, can't replace release firmware\r\n");
			resp->RR = ((token << 24) | FIRMWARE_OFFER_REJECT_OLD_FW);
			return FIRMWARE_UPDATE_OFFER_REJECT;
		}
		//new firmware is debug version and old firmware is debug version too, no need to check version
	}else if(!is_old_fw_dbg){//new firmware is release version and old firmware is release version too
		//release firmware must check version
		if(old_version >= new_version){
			DBG_ERR("old firmware version(%x) >= new firmware version(%x)\r\n",
						backup_bank_info.fw_version, offer->fw_ver);
			resp->RR = ((token << 24) | FIRMWARE_OFFER_REJECT_OLD_FW);
			return FIRMWARE_UPDATE_OFFER_REJECT;
		}
	}//new firmware is release and old firmware is debug, always replace old firmware

	offer_got = 1;//raise offer_got to allow update offer content

	return FIRMWARE_UPDATE_OFFER_ACCEPT;
}

CFU_CONTENT_STATUS cfu_process_update_content(CFU_UPDATE_CONTENT *content)
{
	//if 1. an updated was finished but not yet reboot 2. update offer hasn't been received, reject
	if(swap_pending){
		DBG_DUMP("a swap is pending...can't do cfu\r\n");
		return FIRMWARE_UPDATE_SWAP_PENDING;
	}
	if(!offer_got){
		DBG_DUMP("offer is not received yet...can't do cfu\r\n");
		return FIRMWARE_UPDATE_ERROR_NO_OFFER;
	}

	if(!content){
		DBG_ERR("content is NULL\r\n");
		return FIRMWARE_UPDATE_ERROR_INVALID;
	}
	if(_cfu_setup_common_buffer()){
		DBG_ERR("fail to setup cfu common buffer\r\n");
		return FIRMWARE_UPDATE_ERROR_INVALID;
	}

	//check firmware buffer is large enough for new firmware
	if(content->fw_addr >= FW_BUF_LEN){
		DBG_ERR("fw_addr(0x%x) >= firmware buffer size(%d)\r\n", content->fw_addr, FW_BUF_LEN);
		return FIRMWARE_UPDATE_ERROR_INVALID_ADDR;
	}
	if(content->data_length > FW_BUF_LEN){
		DBG_ERR("data_length(0x%x) >= firmware buffer size(%d)\r\n", content->data_length, FW_BUF_LEN);
		return FIRMWARE_UPDATE_ERROR_INVALID;
	}
	if((content->fw_addr + content->data_length) > FW_BUF_LEN){
		DBG_ERR("fw_addr + data_length(0x%x) >= firmware buffer size(%d)\r\n", content->fw_addr, content->data_length, FW_BUF_LEN);
		return FIRMWARE_UPDATE_ERROR_INVALID_ADDR;
	}

	//reset firmware buffer for the 1st firmware data
	if (content->flags & FIRMWARE_UPDATE_FLAG_FIRST_BLOCK) {
		DBG_DUMP("Got first cfu packet!\r\n");
		memset(firmware, 0, FW_BUF_LEN);
	}

	memcpy(&firmware[content->fw_addr], content->data, content->data_length);

	//when the whole new firmware is received, extract signature(hash) and firmware into corresponding buffer
	if (content->flags & FIRMWARE_UPDATE_FLAG_LAST_BLOCK) {
		DBG_DUMP("Got last cfu packet!\r\n");

		memcpy(hash_data, firmware, sizeof(hash_data));

		memmove(firmware, &firmware[sizeof(hash_data)], FW_BUF_LEN - sizeof(hash_data));
		firmware_length = content->fw_addr + content->data_length - sizeof(hash_data);
		#if 0
		unsigned char *p = (unsigned char *)hash_data;
		DBG_ERR("sig:%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\r\n", 
					p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9],p[10],p[11],p[12],p[13],p[14],p[15]);
		p = firmware;
		DBG_ERR("fw:%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\r\n",
					p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9],p[10],p[11],p[12],p[13],p[14],p[15]);
		DBG_ERR("fw:...\r\n");
		p = &firmware[firmware_length - 16];
		DBG_ERR("fw:%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\r\n",
					p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9],p[10],p[11],p[12],p[13],p[14],p[15]);
		p = (unsigned char *)debug_public_key;
		DBG_ERR("dbgkey:%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\r\n",
					p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9],p[10],p[11],p[12],p[62],p[63],p[64]);
		p = (unsigned char *)release_public_key;
		DBG_ERR("relkey:%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\r\n",
					p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9],p[10],p[11],p[12],p[62],p[63],p[64]);
		#endif
	}

	return FIRMWARE_UPDATE_SUCCESS;
}

int verify(unsigned char *hash, unsigned int hash_length, unsigned char *sig, size_t sig_len, unsigned char *Q_binary, size_t Q_len)
{
    int ret = 1;
	mbedtls_ecdsa_context ctx_verify;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    const char *pers = "ecdsa";
	mbedtls_ecp_keypair ecp = { 0 };

	mbedtls_ecdsa_init( &ctx_verify );
    mbedtls_ctr_drbg_init( &ctr_drbg );
	mbedtls_ecp_keypair_init( &ecp );

    /*
     * Generate a key pair for signing
     */
    mbedtls_entropy_init( &entropy );
    if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                               (const unsigned char *) pers,
                               strlen( pers ) ) ) != 0 )
    {
        DBG_ERR( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\r\n", ret );
        goto exit;
    }

	if( ( ret = mbedtls_ecp_group_load( &ecp.grp, ECPARAMS ) != 0 ) )
    {
        DBG_ERR( " failed\n  ! mbedtls_ecp_group_load returned %d\r\n", ret );
        goto exit;
    }

	if( ( ret = mbedtls_ecp_group_copy( &ctx_verify.grp, &ecp.grp ) ) != 0 )
	{
		DBG_ERR( " failed\n  ! mbedtls_ecp_group_copy returned %d\r\n", ret );
		goto exit;
	}

	if( ( ret = mbedtls_ecp_point_read_binary( &ctx_verify.grp, &ctx_verify.Q, Q_binary, Q_len) ) != 0 )
	{
		DBG_ERR( " failed\n  ! mbedtls_ecp_point_read_binary returned %d\r\n", ret );
		goto exit;
	}

    if( ( ret  = mbedtls_ecdsa_read_signature( &ctx_verify, hash, hash_length, sig, sig_len ) ) != 0)
    {
        DBG_ERR( " failed\n  ! mbedtls_ecdsa_read_signature returned %d\r\n", ret );
        goto exit;
    }

	ret = 0;

exit:

    mbedtls_ecdsa_free( &ctx_verify );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );
	mbedtls_ecp_keypair_free( &ecp );

	return ret;
}

static int do_sha256(unsigned char *input_data, unsigned int input_size, unsigned char *output_data)
{

	int ret =0;
	int align_size = 0;

	if(input_size& (SHA256_ALIGN - 1))
	{
		DBG_IND("sha256 input size not alignmen 0x%x, we will add peding data 0 for compute sha256\r\n",SHA256_ALIGN);
		align_size = ((input_size/SHA256_ALIGN)+1)*SHA256_ALIGN;
		DBG_IND("add %d '0' pedning\r\n",align_size - input_size);
		if(align_size > FW_BUF_LEN){
			DBG_ERR("align size(%d) > firmware size(%d)\r\n", align_size, FW_BUF_LEN);
			return -1;
		}

		memset((unsigned char *)(input_data+input_size),0,align_size - input_size);
		ret = mbedtls_sha256_ret(input_data, align_size, output_data, 0);
	}
	else
	{
		ret = mbedtls_sha256_ret(input_data, input_size, output_data, 0);
	}
	if (ret< 0)
	{
		DBG_ERR("mbedtls_sha256_ret fail ret:%x\r\n",ret);
		return ret;
	}

	return 0;

}

int do_verify(char *public_key, unsigned int key_size, unsigned char *fw_buffer, unsigned int fw_size, unsigned char *encrypted_hash, unsigned int hash_size)
{
	unsigned char signature_buffer[32]={0};
	int ret=0;

	if(!public_key || !key_size || !fw_buffer || !fw_size || !encrypted_hash || !hash_size){
		DBG_ERR("public key(%p) size(%d) or firmware(%p) size(%d) or encrypted hash(%p) size(%d) is NULL\r\n", public_key,key_size,fw_buffer,fw_size,encrypted_hash,hash_size);
		return -1;
	}

	DBG_IND("=========\r\n");
	DBG_IND("do signature\r\n");
	ret = do_sha256(fw_buffer,fw_size,signature_buffer);
	if(ret != 0)
	{
		DBG_ERR("sha256 fail ret:%d\r\n",ret);
		return -1;
	}
	#if 0 //ENCRYPT_CHECK
		printf("signature: \n");
		unsigned int i=0;
		for(i = 0 ; i< sizeof(signature_buffer); i++){
			printf("%02x ",signature_buffer[i]);
		}
		printf("\n");
	#endif

	ret = verify(signature_buffer, sizeof(signature_buffer), encrypted_hash, hash_size, (unsigned char*)public_key, key_size);
	if(ret < 0)
	{
		DBG_ERR("do_rsa_encrypt fail ret:%d\r\n",ret);
		return ret;
	}

	return 0;
}

static unsigned int get_hash_size(void)
{
	struct hash_file_header *header = (struct hash_file_header *)hash_data;
	int hash_length;

	SEM_WAIT(CFU_SEM_ID);
	hash_length = header->hash_length;
	SEM_SIGNAL(CFU_SEM_ID);

	if(hash_length == 0 || hash_length >= 1024){
		DBG_ERR("invalid hash length(%d), must between 1 and 1024\r\n", hash_length);
		return 0;
	}

	return hash_length;
}

static unsigned char* get_hash_data(void)
{
	struct hash_file_header *header = (struct hash_file_header *)hash_data;
	int offset, length;

	SEM_WAIT(CFU_SEM_ID);
	offset = header->hash_offset;
	length = header->hash_length;
	SEM_SIGNAL(CFU_SEM_ID);

	if((offset + length) >= 1024){
		DBG_ERR("invalid hash offset(%d) length(%d), sum must < 1024\r\n", offset, length);
		return NULL;
	}

	return ((unsigned char*)header + offset);
}

static int get_bank_info(UINT32 uiFwAddr, UINT32 uiFwSize, VERSION_CTRL *bank_info)
{
	// check sanity
	NVTPACK_ER nvtpack_er;
	NVTPACK_VERIFY_OUTPUT verify = {0};
	NVTPACK_MEM mem_nvtpack = {0};
	mem_nvtpack.p_data = (unsigned char *)uiFwAddr;
	mem_nvtpack.len = uiFwSize;
	if ((nvtpack_er = nvtpack_verify(&mem_nvtpack, &verify)) != 0) {
		DBG_ERR("nvtpack_verify failed, er = %d\r\n", nvtpack_er);
		return -1;
	}

	// get rtos from nvtpack
	extern int get_rtos_id_form_nvtpack(void);
	int rtos_nvtpack_idx = get_rtos_id_form_nvtpack();
	if (rtos_nvtpack_idx < 0) {
		DBG_ERR("get_rtos_id_form_nvtpack() failed\r\n");
		return -1;
	}
	NVTPACK_GET_PARTITION_INPUT get_input = {0};
	NVTPACK_MEM mem_rtos = {0};
	get_input.id = rtos_nvtpack_idx;
	get_input.mem = mem_nvtpack;
	if ((nvtpack_er = nvtpack_get_partition(&get_input, &mem_rtos)) != NVTPACK_ER_SUCCESS) {
		DBG_ERR("failed to nvtpack_get_partition, er = %d\r\n", nvtpack_er);
		return -1;
	}
	
	// get rtos version from nvtpack
	int nvtpack_rtos_fw_type = 0;
	unsigned int nvtpack_rtos_date = 0;
	unsigned int nvtpack_rtos_ver = 0;
	NVTPACK_BFC_HDR *p_nvtpack_rtos_bfc = (NVTPACK_BFC_HDR *)mem_rtos.p_data;
	if (p_nvtpack_rtos_bfc->uiFourCC != MAKEFOURCC('B', 'C', 'L', '1')) {
		DBG_ERR("nvtpack-rtos invalid bfc.\r\n");
		return -1;
	} else {
		const int unlz_preload_size = ALIGN_CEIL_4(_BIN_INFO_OFS_ + sizeof(BININFO));
		const int unlz_tmp_size = ALIGN_CEIL_4(128 * 1024); //128KB enough
		unsigned char *p_unlz_tmp_fw = (unsigned char *)malloc(unlz_tmp_size);

		if (p_unlz_tmp_fw == NULL) {
			DBG_ERR("alloc unlz_tmp_fw failed.\n");
		}

		//unlzma some to get version
		if ((be32_to_cpu(p_nvtpack_rtos_bfc->uiAlgorithm) & 0xFF) == 11) {
			DBG_ERR("nvtpack-rtos  unsupported lzma data.\r\n");
			free(p_unlz_tmp_fw);
			return -1;
		} else {
			int decoded = LZ_Uncompress((unsigned char *)&p_nvtpack_rtos_bfc[1], p_unlz_tmp_fw, unlz_preload_size);
			if (decoded > unlz_tmp_size) {
				DBG_ERR("unlz_tmp_size is too small, require: %d.\n", decoded);
			}
		}
		
		BININFO *p_bininfo = (BININFO *)(p_unlz_tmp_fw + _BIN_INFO_OFS_);
		nvtpack_rtos_ver = p_bininfo->Resv[BININFO_RESV_IDX_VER];
		DBG_DUMP("nvtpack-rtos ver: %08X\r\n", nvtpack_rtos_ver);

		nvtpack_rtos_date = p_bininfo->Resv[BININFO_RESV_IDX_DATE_SEC];
		DBG_DUMP("nvtpack-rtos date: %u\r\n", nvtpack_rtos_date);

		nvtpack_rtos_fw_type = p_bininfo->Resv[BININFO_RESV_IDX_RELEASE_BUILD];
		DBG_DUMP("nvtpack-rtos fw_type: %u\r\n", nvtpack_rtos_fw_type);

		free(p_unlz_tmp_fw);
	}
	
	if(!bank_info){
		DBG_ERR("bank_info is NULL\r\n");
		return -1;
	}

	//fill bank info
	bank_info->fourcc = MAKEFOURCC('V','E','R','C');
	bank_info->bank_id = backup_bank_info.bank_id^1;
	bank_info->fw_type = nvtpack_rtos_fw_type;
	bank_info->fw_version = nvtpack_rtos_ver;
	bank_info->fw_date =  nvtpack_rtos_date;
	bank_info->update_cnt = backup_bank_info.update_cnt + 1;

	return 0;
}

static int setup_bignum_buf(void)
{
	extern int set_ecdsa_bignum_array(unsigned char *buf, UINT32 len);
	
	if(bignum){
		memset(bignum, 0, BIG_NUM_BUF_LEN);
		return 0;
	}else{
		bignum = malloc(BIG_NUM_BUF_LEN);
		if(!bignum){
			DBG_ERR("fail to allocate %d bytes for bignum buffer\n", BIG_NUM_BUF_LEN);
			return -1;
		}
	}

	memset(bignum, 0, BIG_NUM_BUF_LEN);
	if(set_ecdsa_bignum_array(bignum, BIG_NUM_BUF_LEN)){
		DBG_ERR("fail to set common pool2 as big number array\r\n");
		return -1;
	}

	return 0;
}

CFU_CONTENT_STATUS verify_and_burn_firmware(void)
{
	UINT32  result;
	unsigned int hash_size;
	unsigned int is_product_fw = 0, new_version, old_version, is_new_fw_dbg, is_old_fw_dbg;
	unsigned char *hash = NULL;
//	struct hash_file_header new_header;//hash_data will be modified in do_verify(),
	                                   //must backup hash_data to update header_in_flash
	VERSION_CTRL verctrl = {0};
	struct hash_file_header *new_fw_header = (struct hash_file_header *)hash_data;

	DBG_DUMP("ver(%x) size(%d) offset(%d) len(%d)\r\n", 
				new_fw_header->version,
				new_fw_header->size, 
				new_fw_header->hash_offset,
				new_fw_header->hash_length);

	//if 1. an updated was finished but not yet reboot 2. update offer hasn't been received, reject
	if(swap_pending){
		DBG_DUMP("a swap is pending...can't do cfu\r\n");
		return FIRMWARE_UPDATE_SWAP_PENDING;
	}
	if(!offer_got){
		DBG_DUMP("offer is not received yet...can't do cfu\r\n");
		return FIRMWARE_UPDATE_ERROR_NO_OFFER;
	}
	if(!firmware){
		DBG_ERR("cfu common buffer is NULL\r\n");
		return FIRMWARE_UPDATE_ERROR_INVALID;
	}
	
	if(get_bank_info((UINT32)firmware, firmware_length, &verctrl)){
		DBG_ERR("fail to get bank info\r\n");
		return FIRMWARE_UPDATE_ERROR_INVALID;
	}

	if(verctrl.fw_version != new_fw_header->version){
		DBG_ERR("version of signature(%x) != version of BinInfo(%x)\r\n", new_fw_header->version, verctrl.fw_version);
		return FIRMWARE_UPDATE_ERROR_INVALID;
	}

	if(validate_version(verctrl.fw_version)){
		DBG_ERR("version(0x%x) is invalid\n", verctrl.fw_version);
		return FIRMWARE_UPDATE_ERROR_VERSION;
	}

	old_version   = (backup_bank_info.fw_version & 0xFFFFFF00);
	is_old_fw_dbg = ((backup_bank_info.fw_version & 0x0C) ? 0 : 1);
	new_version   = (verctrl.fw_version & 0xFFFFFF00);
	is_new_fw_dbg = ((verctrl.fw_version & 0x0C) ? 0 : 1);
	is_product_fw = ((verctrl.fw_version & 0x03) == 0x03);

	//parse hash file to extract encrypted hash
	hash_size = get_hash_size();
	if(!hash_size){
		DBG_ERR("fail to get encrypted_hash's length\r\n");
		return FIRMWARE_UPDATE_ERROR_INVALID;
	}
	hash = get_hash_data();
	if(hash == NULL)
	{
		DBG_ERR("get_hash_data fail\r\n");
		return FIRMWARE_UPDATE_ERROR_INVALID;
	}
	//bakcup header because mbedtls's do_verify() is suspected to destroy hash data
//	memcpy(&new_header, hash_data, sizeof(new_header));

	//restore debug/release public key because mbedtls's do_verify() was suspected to destroy public key in previous update
	memcpy(release_public_key, bak_release_public_key, sizeof(release_public_key));
	
	if(setup_bignum_buf()){
		DBG_ERR("fail to allocate bignum buffer from nvtmpp\r\n");
		return FIRMWARE_UPDATE_ERROR_INVALID;
	}

	//verify signature
	if(is_product_fw){
		DBG_IND("verifing: product key\r\n");
		result = do_verify(release_public_key, sizeof(release_public_key), firmware, firmware_length, hash, hash_size);
	}else{
	#if 0
		DBG_IND("verifing: test key\r\n");
		result = do_verify(debug_public_key, sizeof(debug_public_key), firmware, firmware_length, hash, hash_size);
	#else
		//DBG_IND("verifing: test key\r\n");
		//result = do_verify(debug_public_key, sizeof(debug_public_key), firmware, firmware_length, hash, hash_size);
		DBG_ERR("only product sign is supported\n");
		result = -1;
	#endif
	}
	if(result){
		DBG_ERR("verifying firmware fail\r\n");
		return FIRMWARE_UPDATE_ERROR_SIGNATURE;
	}else
		DBG_IND("signature verification pass\r\n");

	SEM_WAIT(CFU_SEM_ID);
	//version check with regard to spec's rule
	if(is_new_fw_dbg){//new firmware is debug version
		if(!is_old_fw_dbg && (new_version != old_version)){//debug firmware can't replace release firmware if versions are diffirent
			DBG_ERR("new firmware is debug version, can't replace release firmware\r\n");
			SEM_SIGNAL(CFU_SEM_ID);
			return FIRMWARE_UPDATE_ERROR_VERSION;
		}
		//new firmware is debug version and old firmware is debug version too, no need to check version
	}else if(!is_old_fw_dbg){//new firmware is release version and old firmware is release version too
		//release firmware must check version
		if(old_version >= new_version){
			DBG_ERR("old firmware version(%x) >= old firmware version(%x)\r\n",
						backup_bank_info.fw_version, new_fw_header->version);
			SEM_SIGNAL(CFU_SEM_ID);
			return FIRMWARE_UPDATE_ERROR_VERSION;
		}
	}//new firmware is release and old firmware is debug, always replace old firmware
	SEM_SIGNAL(CFU_SEM_ID);

	//write firmware into flash
	cfu_abort_burn = 0;
	*(unsigned int*)verctrl.hash = hash_size;
	memcpy(&(verctrl.hash[4]), hash, hash_size);
	memcpy(verctrl.reserved, new_fw_header->reserve, sizeof(verctrl.reserved));
#ifdef _MODEL_UVCCAM5_517L_DUAL_EVB_ 
		DisableNORWriteProtect();
#endif
	result = flow_updfw(firmware, firmware_length, &cfu_abort_burn, &verctrl);
#ifdef _MODEL_UVCCAM5_517L_DUAL_EVB_ 
	EnableNORWriteProtect();
#endif
	if(result){
		DBG_ERR("flow_updfw() fail\r\n");
		return FIRMWARE_UPDATE_ERROR_COMPLETE;
	}else
		DBG_DUMP("flow_updfw() %d bytes ok\r\n", firmware_length);

	SEM_WAIT(CFU_SEM_ID);
	swap_pending = 1;
	offer_got = 0;
	memcpy(&backup_bank_info, &verctrl, sizeof(VERSION_CTRL));
	SEM_SIGNAL(CFU_SEM_ID);

	return FIRMWARE_UPDATE_SUCCESS;
}

#include <kwrap/examsys.h>
#include "FileSysTsk.h"
#include "FileDB.h"
static int cfu_test(int fail)
{
	CFU_CONTENT_STATUS status;
	int i;
	FST_FILE filehdl;
	INT32 ifs_ret;
	UINT32 fileLen = 11264;

	memset(firmware, 0, FW_BUF_LEN);

	filehdl = FileSys_OpenFile("A:\\test.bin", FST_OPEN_READ);
	if (!filehdl) {
		DBG_ERR("fail to open A:\\test.bin\r\n");
		return -1;
	}

	ifs_ret = FileSys_ReadFile(filehdl, firmware, &fileLen, 0, 0);
	FileSys_CloseFile(filehdl);
	if (ifs_ret != 0) {
		DBG_ERR("read file fail!\r\n");
		return -1;
	}

	firmware_length = fileLen - sizeof(hash_data);
	DBG_DUMP("firmware_length=%d\r\n", firmware_length);

	memcpy(hash_data, firmware, sizeof(hash_data));

	memmove(firmware, &firmware[sizeof(hash_data)], FW_BUF_LEN - sizeof(hash_data));
	
	if(fail == 1){
		DBG_DUMP("tampering firmware data\n");
		firmware[5121] = 88;
		firmware[5120] = 87;
	}else if(fail == 2){
		DBG_DUMP("tampering hash data\n");
		hash_data[301] = 88;
		hash_data[302] = 87;
		//memset(hash_data, 0x11, 30);
	}

	status = verify_and_burn_firmware();
	if(status == FIRMWARE_UPDATE_SUCCESS)
		DBG_DUMP("verify_and_burn_firmware() ok\r\n");
	else{
		DBG_ERR("verify_and_burn_firmware() fail\r\n");
		return -1;
	}

	for(i = 0 ; i < firmware_length ; ++i){
		if(firmware[i] == (i & 0xFF))
			continue;
		DBG_ERR("data validation[%d] = %x fail, expected %d\r\n", i, firmware[i], (255 & 0xFF));
		return -1;
	}
	DBG_DUMP("data validation ok\r\n");

	return 0;
}

EXAMFUNC_ENTRY(cfu, argc, argv)
{
	int fail = 0;
	if(argc >= 2)
		fail = atoi(argv[1]);
	if(fail)
		DBG_DUMP("intentional fail = %d\n", fail);
	
	cfu_test(fail);
	
	return 0;
}
