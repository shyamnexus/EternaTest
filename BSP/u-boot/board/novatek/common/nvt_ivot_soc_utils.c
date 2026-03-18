/**
    NVT utilities for command customization

    @file       nvt_ivot_soc_utils.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <cpu_func.h>
#include <common.h>
#include <cli.h>
#include <image.h>
#include <fs.h>
#include <asm/io.h>
#include <nand.h>
#include <mmc.h>
#include <spi_flash.h>
#include <command.h>
#include <lzma/LzmaTools.h>
#include <linux/libfdt.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/modelext/emb_partition_info.h>
#include <asm/nvt-common/modelext/bin_info.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/shm_info.h>
#include <asm/nvt-common/nvt_ivot_optee.h>
#include <asm/nvt-common/nvt_ivot_efuse_smc.h>
#include <asm/nvt-common/nvt_ivot_sha_smc.h>
#include <asm/nvt-common/nvt_ivot_rsa_smc.h>
#include <asm/nvt-common/nvt_ivot_aes_smc.h>
#include <asm/nvt-common/nvt_ivot_ecdsa_smc.h>
#include <asm/arch/nvt_crypto.h>
#include <asm/arch/nvt_rsa.h>
#include <asm/arch/nvt_hash.h>
#if defined(CONFIG_SIGNATUREMETHOD_ECDSA)
#include <asm/arch/nvt_ecdsa.h>
#endif
#include <asm/arch/crypto.h>
#include <asm/arch/IOAddress.h>
#include <compiler.h>
#include <u-boot/zlib.h>
#include <u-boot/crc.h>
#include <stdlib.h>
#include "nvt_ivot_soc_utils.h"
#include "nvt_ivot_pack.h"
#include <malloc.h>

#if ((defined(CONFIG_NVT_IVOT_OPTEE_SUPPORT) && defined(CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT)) || defined(CONFIG_NVT_IVOT_SECBOOT_SUPPORT))
#if defined(CONFIG_TARGET_NS02401_A64)
#define otp_key_manager_wrapper(m)  otp_key_manager(EFUSE_ID_0, m)
#else
#define otp_key_manager_wrapper(m)  otp_key_manager(m)
#endif
#endif


#define UINT32_SWAP(data)           (((((u32)(data)) & 0x000000FF) << 24) | \
									 ((((u32)(data)) & 0x0000FF00) << 8) | \
									 ((((u32)(data)) & 0x00FF0000) >> 8) | \
									 ((((u32)(data)) & 0xFF000000) >> 24))   ///< Swap [31:24] with [7:0] and [23:16] with [15:8].

extern HEADINFO gHeadInfo;
int atoi(const char *str)
{
	return (int)simple_strtoul(str, '\0', 10);
}

int get_linux_major_version(void) {
    image_header_t * hdr = (image_header_t *)nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr;
    char major_version[3];

    const char *version_start = strstr(hdr->ih_name, "Linux-");
    if (version_start == NULL) {
        nvt_dbg(ERR, "%s fail\n", __func__);
        return -1;
    }

    version_start += strlen("Linux-");

    strncpy(major_version, version_start, 2);
    major_version[2] = '\0';

    int major = atoi(major_version);

    return major;
}

#if ((defined(CONFIG_NVT_IVOT_OPTEE_SUPPORT) && defined(CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT)) || defined(CONFIG_NVT_IVOT_SECBOOT_SUPPORT))
static void get_bininfo_size_offset(unsigned char* bininfo,unsigned int* size,unsigned int *offset)
{

	*size = bininfo[4] | bininfo[5]<<8 | bininfo[6] << 16 | bininfo[7] << 24;
	*offset = bininfo[0] | bininfo[1] << 8 | bininfo[2] << 16 | bininfo[3] <<24;
}

static void fill_data_r(UINT8 * s, UINT8 * target, int len)
{
	int   i;

	for(i=0; i<len; i++)
		target[len-i-1] = s[i];
}

static int nvt_chk_key(ulong addr, unsigned int size)
{
	HEADINFO *head_info = (HEADINFO *)addr;
	unsigned int key_size = 0;
	unsigned int i=0;
	unsigned int key_offset =0;
	unsigned char * key_buf = NULL;
	unsigned char sha256[32]={0};
	unsigned char key_buf_reverse[256]={0};
	int ret =0;


	get_bininfo_size_offset(head_info->BinInfo_1, &key_size, &key_offset);
	key_buf = (unsigned char *)(addr + key_offset);

#if defined(CONFIG_SIGNATUREMETHOD_ECDSA)
	/* Reverse X key */
	fill_data_r(key_buf, key_buf_reverse, (key_size / 2));
	/* Reverse Y key */
	fill_data_r((unsigned char *)(key_buf + (key_size / 2)), (unsigned char *)(key_buf_reverse + (key_size / 2)), (key_size / 2));

	NVT_SMC_SHA_DATA sha_data={0};
	sha_data.input_data = key_buf_reverse;
	sha_data.output_data = sha256;
	sha_data.input_size = 128;
	sha_data.sha_mode = NVT_SMC_SHA_MODE_SHA256;
	ret = nvt_ivot_optee_sha_operation(&sha_data);
	if(ret != 0)
	{
		printf("nvt_ivot_optee_sha_operation fail,ret:%d\n",ret);
		return ret;
	}
	printf(" ecdsa key reverse and sha256:\n");
	for(i=0; i< 32; i++)
	{
		printf("%x ",*(char *)(sha256+i));
	}

	NVT_SMC_EFUSE_DATA efuse_data = {0};
	efuse_data.cmd = NVT_SMC_EFUSE_COMPARE_KEY;
	efuse_data.key_data.field = EFUSE_OTP_2ND_KEY_SET_FIELD;
	memcpy(efuse_data.key_data.data, &sha256[0], 16);
	if(nvt_ivot_optee_efuse_operation(&efuse_data) !=1)
	{
		printf("compare key2 fail!, please check key in efuse or partition\n");
		return -1;
	}
	efuse_data.key_data.field = EFUSE_OTP_3RD_KEY_SET_FIELD;
	memcpy(efuse_data.key_data.data, &sha256[16], 16);
	if(nvt_ivot_optee_efuse_operation(&efuse_data) !=1)
	{
		printf("compare key3 fail!, please check key in efuse or partition\n");
		return -1;
	}

	printf("check ECDSA key ok\n");
#else
	printf("only check N key-> key_size:%d key_offset:%d\n",key_size,key_offset);
	fill_data_r(key_buf,key_buf_reverse,256);
	printf("check rsa 2048 key\n");
	NVT_SMC_SHA_DATA sha_data={0};
	sha_data.input_data = key_buf_reverse;
	sha_data.output_data = sha256;
	sha_data.input_size = 256;
	sha_data.sha_mode = NVT_SMC_SHA_MODE_SHA256;
	ret = nvt_ivot_optee_sha_operation(&sha_data);
	if(ret != 0)
	{
		printf("nvt_ivot_optee_sha_operation fail,ret:%d\n",ret);
		return ret;
	}
	printf(" rsa key reverse and sha256:\n");
	for(i=0; i< 32; i++)
	{
		printf("%x ",*(char *)(sha256+i));
	}
	printf("\n");
	/*************
		compare key hash from efuse
	**************/

	/*default set sha256 in 1,2 efuse key field,   0 efuse key field for aes key*/

	/* check low part-->sha256[0] ~ sha256[15] */

	NVT_SMC_EFUSE_DATA efuse_data = {0};
	efuse_data.cmd = NVT_SMC_EFUSE_COMPARE_KEY;
	efuse_data.key_data.field = EFUSE_OTP_2ND_KEY_SET_FIELD;
	memcpy(efuse_data.key_data.data, &sha256[0], 16);
	if(nvt_ivot_optee_efuse_operation(&efuse_data) !=1)
	{
		printf("compare key2 fail!, please check key in efuse or partition\n");
		return -1;
	}
	efuse_data.key_data.field = EFUSE_OTP_3RD_KEY_SET_FIELD;
	memcpy(efuse_data.key_data.data, &sha256[16], 16);
	if(nvt_ivot_optee_efuse_operation(&efuse_data) !=1)
	{
		printf("compare key3 fail!, please check key in efuse or partition\n");
		return -1;
	}

	printf("check rsa key ok\n");
#endif

	return 0;
}

/* Verify RSA Key */
static int nvt_chk_key_on_uboot(unsigned char *sha256)
{
	unsigned key_value[4]={0};
	int offset = 0;
	int field_index = 20;
	key_value[0] = sha256[offset+0] | sha256[offset+1] << 8 | sha256[offset+2] << 16 | sha256[offset+3] << 24;
	key_value[1] = sha256[offset+4] | sha256[offset+5] << 8 | sha256[offset+6] << 16 | sha256[offset+7] << 24;
	key_value[2] = sha256[offset+8] | sha256[offset+9] << 8 | sha256[offset+10] << 16 | sha256[offset+11] << 24;
	key_value[3] = sha256[offset+12] | sha256[offset+13] << 8 | sha256[offset+14] << 16 | sha256[offset+15] << 24;
	if(key_value[0] != otp_key_manager_wrapper(field_index) || key_value[1] != otp_key_manager_wrapper(field_index+1) || key_value[2] != otp_key_manager_wrapper(field_index+2) || key_value[3] != otp_key_manager_wrapper(field_index+3)){
		printf("key compare fail\n");
		printf("efuse key: %x %x %x %x\n",
				otp_key_manager_wrapper(field_index), otp_key_manager_wrapper(field_index+1),
				otp_key_manager_wrapper(field_index+2),otp_key_manager_wrapper(field_index+3));
		printf("compare key: %x %x %x %x\n",key_value[0],key_value[1],key_value[2],key_value[3]);
		return -1;
	}

	field_index = 24;
	offset = 16;
	key_value[0] = sha256[offset+0] | sha256[offset+1] << 8 | sha256[offset+2] << 16 | sha256[offset+3] << 24;
	key_value[1] = sha256[offset+4] | sha256[offset+5] << 8 | sha256[offset+6] << 16 | sha256[offset+7] << 24;
	key_value[2] = sha256[offset+8] | sha256[offset+9] << 8 | sha256[offset+10] << 16 | sha256[offset+11] << 24;
	key_value[3] = sha256[offset+12] | sha256[offset+13] << 8 | sha256[offset+14] << 16 | sha256[offset+15] << 24;
	if(key_value[0] != otp_key_manager_wrapper(field_index) || key_value[1] != otp_key_manager_wrapper(field_index+1) || key_value[2] != otp_key_manager_wrapper(field_index+2) || key_value[3] != otp_key_manager_wrapper(field_index+3)){
		printf("key compare fail\n");
		printf("efuse key: %x %x %x %x\n",
				otp_key_manager_wrapper(field_index), otp_key_manager_wrapper(field_index+1),
				otp_key_manager_wrapper(field_index+2),otp_key_manager_wrapper(field_index+3));
		printf("compare key: %x %x %x %x\n",key_value[0],key_value[1],key_value[2],key_value[3]);
		return -1;
	}

	return 0;
}

int nvt_chk_signature_on_uboot(ulong addr, unsigned int size)
{
	HEADINFO *head_info = (HEADINFO *)addr;
#if defined(CONFIG_SIGNATUREMETHOD_ECDSA)
	struct nvt_ecdsa_pio_t ecdsa;
#else
	struct nvt_rsa_pio_t rsa;
#endif
	unsigned int key_size = 0;
	unsigned int key_offset =0;
	unsigned char * key_buf = NULL;
	unsigned int signature_size = 0;
	unsigned int signature_offset = 0;
	unsigned char * signature_buf = NULL;
	unsigned int crypto_size = 0;
	unsigned int crypto_offset = 0;
	unsigned char * crypto_buf = NULL;
	unsigned int rsa_output[64]={0};
	unsigned int output_size = sizeof(rsa_output);
	unsigned int data_size =0;
	int ret = 0;
	struct nvt_hash_pio_t hash;
	unsigned char key_buf_reverse[256]={0};
	unsigned char sha256[32]={0};
	unsigned int current_sha256[64]={0};
	unsigned int sha256_align_size =0;
	unsigned int sha256_size = 0;

	get_bininfo_size_offset(head_info->BinInfo_1, &key_size, &key_offset);
	key_buf = (unsigned char *)(addr + key_offset);

	/* to avoid function hang, if got a huge key_size size (e.g:0xFFFFFFFF) */
	if (key_size > size) {
		printf("invalid encrypted-bin head_info->size = %d, input size = %d \n", key_size, size);
		return -1;
	}

	get_bininfo_size_offset(head_info->BinInfo_2, &signature_size, &signature_offset);
	signature_buf = (unsigned char *)(addr + signature_offset);

	get_bininfo_size_offset(head_info->BinInfo_3, &crypto_size, &crypto_offset);
	crypto_buf = (unsigned char *)(addr + crypto_offset);

	/* Align sha256 data to 64 */
	data_size = head_info->BinLength - crypto_offset;
	sha256_size = data_size;
	if (head_info->BinCtrl & SIGN_BINCTRL_LIMITED_SIZE) {
		// the sha made from part of content but rather whole content
		sha256_size = head_info->Resv1[HEADINFO_SIGN_RESV_IDX_LIMITED_SIZE];
	}
	if((sha256_size & 0x3f) != 0)
	{
		sha256_align_size = ((sha256_size/0x40) + 1)*0x40;
		memset((unsigned char *)(crypto_buf + sha256_size),0, sha256_align_size - sha256_size);
		printf("sha256 data not align 64 ,it will add padding automatically\n");
	} else {
		sha256_align_size = sha256_size;
	}


	/* 1. Get Hash from RSA Key */
	ret = nvt_hash_open();
	if (ret < 0) {
		printf("nvt hash open failed!\n");
		return ret;
	}

#if defined(CONFIG_SIGNATUREMETHOD_ECDSA)
	/* Reverse X key */
	fill_data_r(key_buf, key_buf_reverse, (key_size / 2));
	/* Reverse Y key */
	fill_data_r((unsigned char *)(key_buf + (key_size / 2)), (unsigned char *)(key_buf_reverse + (key_size / 2)), (key_size / 2));
	/* Hash Data */
	hash.src_size    = 128;
#else
	fill_data_r(key_buf,key_buf_reverse,256);
	/* Hash Data */
	hash.src_size    = 256;
#endif
	/* Hash Data */
	hash.mode        = NVT_HASH_MODE_SHA256;
	hash.src         = key_buf_reverse;
	hash.digest      = sha256;
	hash.digest_size = 32;
	ret = nvt_hash_pio_sha(&hash);
	if (ret < 0) {
		printf("nvt hash failed!\n");
		nvt_hash_close();
		return ret;
	}
	nvt_hash_close();

	/* 2. Verify hash of RSA key on Image and efuse */
	ret = nvt_chk_key_on_uboot(sha256);
	if (ret < 0) {
		printf("nvt check key failed!\n");
		return ret;
	}

#if defined(CONFIG_SIGNATUREMETHOD_ECDSA)
	unsigned char signature_buf_tmp[256]={0};
	unsigned char sha256_buf_tmp[32]={0};

	/* 3. Get Hash from signature */
	ret = nvt_hash_open();
	if (ret < 0) {
		printf("nvt hash open failed!\n");
		return ret;
	}

	/* Hash Data */
	hash.mode        = NVT_HASH_MODE_SHA256;
	hash.src         = crypto_buf;
	hash.digest      = (unsigned char *)current_sha256;
	hash.src_size    = sha256_align_size;
	hash.digest_size = 32;
	ret = nvt_hash_pio_sha(&hash);
	if (ret < 0) {
		printf("nvt hash failed!\n");
		nvt_hash_close();
		return ret;
	}
	nvt_hash_close();

	/* Reverse hash value */
	fill_data_r((unsigned char *)current_sha256, sha256_buf_tmp, 32);

	/* 4. Verify signature */

	/* Reverse R key */
	fill_data_r(signature_buf, signature_buf_tmp, (signature_size / 2));
	/* Reverse S key */
	fill_data_r((unsigned char *)(signature_buf + (signature_size / 2)), (unsigned char *)(signature_buf_tmp + (signature_size / 2)), (signature_size / 2));

	ret = nvt_ecdsa_open();
	if (ret < 0) {
		printf("nvt ecdsa open failed!\n");
		return ret;
	}

	ecdsa.key_w = NVT_ECDSA_KEY_WIDTH_256;
	ecdsa.mode = NVT_ECDSA_VERIFY;
	ecdsa.sign_r = signature_buf_tmp;
	ecdsa.sign_s = (unsigned char *)((unsigned long)signature_buf_tmp + (signature_size / 2));
	ecdsa.key_pub_x = key_buf_reverse;
	ecdsa.key_pub_y = (unsigned char *)((unsigned long)key_buf_reverse + (key_size / 2));
	ecdsa.src = (unsigned char *)sha256_buf_tmp;

	ret = nvt_ecdsa_init(ecdsa.key_w);
	if (ret < 0) {
		printf("nvt ecdsa initial failed!\n");
		return ret;
	}

	ret = nvt_ecdsa_pio_normal(&ecdsa);
	if (ret < 0) {
		printf("nvt_ecdsa_pio_normal failed!\n");
		nvt_ecdsa_close();
		return ret;
	}

	if (ecdsa.result == 0) {
		printf("nvt ecdsa verify failed! 0x%x\n", ecdsa.result);
		nvt_ecdsa_close();
		return -EINVAL;
	}

	nvt_ecdsa_close();
#else
	/* 3. Decrypt the signature to get the hash of the encrypted data */
	ret = nvt_rsa_open();
	if (ret < 0) {
		printf("nvt rsa open failed!\n");
		return ret;
	}

	/* rsa input data size should equal to n key size */
	unsigned char signature_buf_tmp[256]={0};
	memcpy(signature_buf_tmp,signature_buf,signature_size);

	rsa.key_w       = NVT_RSA_KEY_WIDTH_2048;
	rsa.src         = signature_buf_tmp;
	rsa.dst         = (unsigned char *)rsa_output;
	rsa.key_n       = key_buf;
	rsa.key_ed      = (unsigned char *)((unsigned long)key_buf + 256);
	rsa.src_size    = 256;
	rsa.dst_size    = output_size;
	rsa.key_n_size  = 256;
	rsa.key_ed_size = key_size - rsa.key_n_size;;

	ret = nvt_rsa_pio_normal(&rsa);
	if (ret < 0) {
		printf("nvt rsa decryption failed!\n");
		nvt_rsa_close();
		return ret;
	}
	nvt_rsa_close();

	/* 5. Get Hash from Crypto data */
	ret = nvt_hash_open();
	if (ret < 0) {
		printf("nvt hash open failed!\n");
		return ret;
	}

	fill_data_r(key_buf,key_buf_reverse,256);
	/* Hash Data */
	hash.mode        = NVT_HASH_MODE_SHA256;
	hash.src         = crypto_buf;
	hash.digest      = (unsigned char *)current_sha256;
	hash.src_size    = sha256_align_size;
	hash.digest_size = 32;
	ret = nvt_hash_pio_sha(&hash);
	if (ret < 0) {
		printf("nvt hash failed!\n");
		nvt_hash_close();
		return ret;
	}
	nvt_hash_close();

	/* 6. Verify Signature [ output from step 3 and 5 ]*/
	unsigned int *sha256_output = (unsigned int *)((unsigned long)rsa_output + 256 - 32);// the last 32 bytes are signature
	ret = memcmp(current_sha256, sha256_output, 32);
	if (ret != 0) {
		printf("Verify Signature failed!\n");
		int i;
		unsigned char rsa_output_t[256];
		memcpy(rsa_output,rsa_output_t,256);
		for (i = 0; i < 64; i++) {
			printf("%x %x\n", rsa_output_t[i*4], signature_buf_tmp[i*4]);
		}
		return ret;
	}

#endif
	return ret;
}

int nvt_chk_signature(ulong addr, unsigned int size)
{
	int ret=0;
	HEADINFO *head_info = (HEADINFO *)addr;
	unsigned int key_size = 0;
	unsigned int key_offset =0;
	unsigned char * key_buf = NULL;
	unsigned int signature_size = 0;
	unsigned int signature_offset = 0;
	unsigned char * signature_buf = NULL;
	unsigned int crypto_size = 0;
	unsigned int crypto_offset = 0;
	unsigned char * crypto_buf = NULL;
	unsigned int *sha256 = NULL;
	unsigned int rsa_output[64]={0};
	unsigned int current_sha256[64]={0};
	unsigned int output_size = sizeof(rsa_output);
	unsigned int data_size =0;
	unsigned int sha256_align_size =0;
	NVT_SMC_EFUSE_DATA efuse_data = {0};
	efuse_data.cmd = NVT_SMC_EFUSE_IS_RSA_KEY_CHECK;
	if(1 == nvt_ivot_optee_efuse_operation(&efuse_data))
	{
		ret = nvt_chk_key( addr, size);
		if(ret != 0)
		{
			printf("nvt_chk_public_key fail ret:%x\n",ret);
			return -1;
		}
	}
	else
	{
		printf("rsa key check not enable\n");
	}
	get_bininfo_size_offset(head_info->BinInfo_1, &key_size, &key_offset);
	key_buf = (unsigned char *)(addr + key_offset);


	get_bininfo_size_offset(head_info->BinInfo_2, &signature_size, &signature_offset);
	signature_buf = (unsigned char *)(addr + signature_offset);


	get_bininfo_size_offset(head_info->BinInfo_3, &crypto_size, &crypto_offset);
	crypto_buf = (unsigned char *)(addr + crypto_offset);

	data_size = head_info->BinLength - crypto_offset;
	if((data_size & 0x3f) != 0)
	{
		sha256_align_size = ((data_size/0x40) + 1)*0x40;
		memset((unsigned char *)(crypto_buf + data_size),0, sha256_align_size - data_size);
		printf("sha256 data not align 64 ,need add pending data\n");
	}
	else
	{
		sha256_align_size = data_size;
	}

#if defined(CONFIG_SIGNATUREMETHOD_ECDSA)
	unsigned char signature_buf_tmp[256] = {0};
	unsigned char sha256_buf_tmp[32] = {0};
	unsigned char key_buf_reverse[256]={0};
	bool result_buf_tmp[256] = {0};

	NVT_SMC_ECDSA_DATA ecdsa = {0};
	NVT_SMC_SHA_DATA sha_data = {0};
	sha_data.input_data = crypto_buf;
	sha_data.output_data = (unsigned char *)current_sha256;
	sha_data.input_size = sha256_align_size;
	sha_data.sha_mode = NVT_SMC_SHA_MODE_SHA256;
	ret = nvt_ivot_optee_sha_operation(&sha_data);
	if(ret != 0)
	{
		printf("nvt_ivot_optee_sha_operation fail,ret:%d\n",ret);
		return ret;
	}

	/* Reverse hash value */
	fill_data_r((unsigned char *)current_sha256, sha256_buf_tmp, 32);

	/* Reverse X key */
	fill_data_r(key_buf, key_buf_reverse, (key_size / 2));
	/* Reverse Y key */
	fill_data_r((unsigned char *)(key_buf + (key_size / 2)), (unsigned char *)(key_buf_reverse + (key_size / 2)), (key_size / 2));

	/* Reverse R key */
	fill_data_r(signature_buf, signature_buf_tmp, (signature_size / 2));
	/* Reverse S key */
	fill_data_r((unsigned char *)(signature_buf + (signature_size / 2)), (unsigned char *)(signature_buf_tmp + (signature_size / 2)), (signature_size / 2));

	ecdsa.key_w = NVT_SMC_ECDSA_KEY_WIDTH_256;
	ecdsa.ecdsa_mode = NVT_SMC_ECDSA_SMC_VERIFY;
	ecdsa.operation = NVT_SMC_ECDSA_OPERATION_OPEN;
	ecdsa.sign_r = signature_buf_tmp;
	ecdsa.sign_s = (unsigned char *)((unsigned long)signature_buf_tmp + (signature_size / 2));
	ecdsa.key_pub_x = key_buf_reverse;
	ecdsa.key_pub_y = (unsigned char *)((unsigned long)key_buf_reverse + (key_size / 2));
	ecdsa.src = (unsigned char *)sha256_buf_tmp;
	ecdsa.result = (bool *)result_buf_tmp;
	ret = nvt_ivot_optee_ecdsa_operation(&ecdsa, key_size, signature_size);
	if(ret != 0){
		printf("nvt_ivot_optee_ecdsa_operation open fail ret:%d\n",ret);
		return -1;
	}

	ecdsa.operation = NVT_SMC_ECDSA_OPERATION_UPDATE;
	ret = nvt_ivot_optee_ecdsa_operation(&ecdsa, key_size, signature_size);
	if(ret != 0){
		printf("nvt_ivot_optee_ecdsa_operation update fail ret:%d\n",ret);
		return -1;
	}

	ecdsa.operation = NVT_SMC_ECDSA_OPERATION_CLOSE;
	ret = nvt_ivot_optee_ecdsa_operation(&ecdsa, key_size, signature_size);
	if(ret != 0){
		printf("nvt_ivot_optee_ecdsa_operation close fail ret:%d\n",ret);
		return -1;
	}

	if (ecdsa.result == 0) {
		printf("nvt ecdsa verify failed!\n");
		return -1;
	}
#else
	NVT_SMC_RSA_DATA rsa_data={0};
	unsigned char signature_buf_tmp[256]={0};
	memcpy(signature_buf_tmp,signature_buf,signature_size);
	rsa_data.rsa_mode = NVT_SMC_RSA_MODE_2048;
	rsa_data.n_key = key_buf;
	rsa_data.n_key_size = 256;
	rsa_data.ed_key = (unsigned char *)((unsigned long)key_buf + 256);
	rsa_data.ed_key_size = key_size - rsa_data.n_key_size;
	rsa_data.input_data = signature_buf_tmp;
	rsa_data.input_size = 256;
	rsa_data.output_data = (unsigned char *)rsa_output;
	ret = nvt_ivot_optee_rsa_operation(&rsa_data);
	if(ret != 0){
		printf("nvt_ivot_optee_rsa_decrypt fail ret:%d\n",ret);
		return -1;
	}

	unsigned int i=0;
	printf("\nrsa decrypt size (%d) :\n", output_size);
	for(i=0;i< (output_size/4);i++)
	{
		printf("%08x ",rsa_output[i]);
	}
	printf("\n");

	printf("crypto_buf:%x offset:%x addr:%x\n",crypto_buf,crypto_offset,addr);
	printf("data size:%x\n",crypto_size);
	printf("data :%x %x %x %x %x %x\n",crypto_buf[0],crypto_buf[1],crypto_buf[2],crypto_buf[3],crypto_buf[4],crypto_buf[5]);

	NVT_SMC_SHA_DATA sha_data={0};
	sha_data.input_data = crypto_buf;
	sha_data.output_data = (unsigned char *)current_sha256;
	sha_data.input_size = sha256_align_size;
	sha_data.sha_mode = NVT_SMC_SHA_MODE_SHA256;
	ret = nvt_ivot_optee_sha_operation(&sha_data);
	if(ret != 0)
	{
		printf("nvt_ivot_optee_sha_operation fail,ret:%d\n",ret);
		return ret;
	}


	if(ret != 0)
	{
		printf("nvt_ivot_optee_sha256_compute fail,ret:%d\n",ret);
		return -1;
	}

	sha256 = (unsigned int *)((unsigned long)rsa_output + 256 - 32);
	printf("\n hash img:\n");
	for(i=0;i< 8;i++)
	{
		printf("%x ",current_sha256[i]);
	}
	printf("\n");
	printf("\n signature data:\n");
	for(i=0;i< 8;i++)
	{
		printf("%x ",sha256[i]);
	}
	printf("\n");

	if(memcmp(&current_sha256[0], sha256, 32)!= 0)
	{
		printf("compare hash fail!\n");
		return -1;
	}

#endif
	return 0;
}

int nvt_decrypt_aes_on_uboot(unsigned long addr)
{
	int ret = 0;
	struct nvt_crypto_pio_t crypto;
	HEADINFO *headinfo = (HEADINFO *)addr;
	unsigned int crypto_offset = 0;
	unsigned int crypto_size = 0;
	unsigned char *encrypt_buf = NULL;
	unsigned int total_size = headinfo->BinLength;
	get_bininfo_size_offset(headinfo->BinInfo_3, &crypto_size, &crypto_offset);
	/* crypto_size is alignmnet 16, so the uImage size and crypto_size may not the same */
	unsigned int decrypt_size = total_size - crypto_offset;
	UINT8  iv[16];

	if (headinfo->BinCtrl & SIGN_BINCTRL_NO_ENCRYPTIONDATA) {
		// signature only flow, the content without aes
		nvt_dbg(ERR, "skip decrypt aes\n");
		memcpy((unsigned char *)addr, (unsigned char *)(addr + crypto_offset), decrypt_size);
		return 0;
	}

	/* Decrypt Date By AES Key */
	ret = nvt_crypto_open();
	if (ret < 0) {
		printf("nvt crypto open failed!\n");
		return ret;
	}

	encrypt_buf = (unsigned char *)((unsigned long)addr + crypto_offset);
	memset(iv, 0, 16);
	/* AES Decryption with efuse test key */
	crypto.encrypt  = 0;
	crypto.opmode   = NVT_CRYPTO_OPMODE_CBC;
	crypto.src      = (unsigned char *)encrypt_buf;
	crypto.dst      = (unsigned char *)addr;
	crypto.key      = NULL;
	crypto.key_src  = NVT_CRYPTO_KEY_SRC_EFUSE;
	crypto.key_ofs  = 0;
	crypto.src_size = crypto_size;
	crypto.dst_size = decrypt_size;
	crypto.key_size = NVT_CRYPTO_AES_128_KEY_SIZE;
	crypto.iv	= iv;
	crypto.iv_size  = 16;
	ret = nvt_crypto_pio_aes(&crypto);
	if (ret < 0) {
		printf("nvt crypto ekey decrypt failed!\n");
		nvt_crypto_close();
		return ret;
	}
	nvt_crypto_close();

	if (decrypt_size > crypto_size) {
		/* there are some data not encrypt, need copy to buf */
		unsigned char *tmp = (unsigned char *)addr;
		memcpy(&tmp[crypto_size], &encrypt_buf[crypto_size], (decrypt_size - crypto_size));
	}

	return ret;
}

int nvt_decrypt_aes(unsigned long addr)
{
	NVT_SMC_EFUSE_DATA efuse_data = {0};
	efuse_data.cmd = NVT_SMC_EFUSE_IS_DATA_ENCRYPTED;
	if (1 != nvt_ivot_optee_efuse_operation(&efuse_data)) {
		printf("data encrypt not enable\n");
		return 0;
	}
	HEADINFO *headinfo = (HEADINFO *)addr;
	unsigned int crypto_offset = 0;
	unsigned int crypto_size = 0;
	unsigned char *encrypt_buf = NULL;
	unsigned char *bininfo = headinfo->BinInfo_3;
	int ret = 0;
	get_bininfo_size_offset(headinfo->BinInfo_3, &crypto_size, &crypto_offset);

	encrypt_buf = (unsigned char *)((unsigned long)addr + crypto_offset);

	unsigned int total_size = headinfo->BinLength;

	// crypto_size is alignmnet 16, so the uImage size and crypto_size may not the same
	unsigned int decrypt_size = total_size - crypto_offset;

	if (headinfo->BinCtrl & SIGN_BINCTRL_NO_ENCRYPTIONDATA) {
		// signature only flow, the content without aes
		nvt_dbg(ERR, "skip decrypt aes\n");
		memcpy((unsigned char *)addr, (unsigned char *)(addr + crypto_offset), decrypt_size);
		return 0;
	}

	NVT_SMC_AES_DATA aes_data = {0};
	aes_data.crypto_type = NVT_SMC_AES_CRYPTO_DECRYPTION;
	aes_data.aes_mode = NVT_SMC_AES_MODE_CBC;
	aes_data.key_buf = 0;
	memset(aes_data.IV, 0, 16);
	aes_data.efuse_field = 0;
	aes_data.key_size = 16;
	aes_data.input_data = (unsigned char *)encrypt_buf;
	aes_data.input_size = crypto_size;
	aes_data.output_data = (unsigned char *)addr;
	ret = nvt_ivot_optee_aes_operation(&aes_data);
	if (ret != 0) {
		printf("nvt_ivot_optee_aes_operation fail\r\n", ret);
		return -1;
	}
	if (decrypt_size > crypto_size) {
		//there are some data not encrypt, need copy to buf
		unsigned char *tmp = (unsigned char *)addr;
		memcpy(&tmp[crypto_size], &encrypt_buf[crypto_size], (decrypt_size - crypto_size));

	}

	return 0;
}
#endif /* CONFIG_NVT_IVOT_OPTEE_SUPPORT && CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT || CONFIG_NVT_IVOT_SECBOOT_SUPPORT */

#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT) || defined(CONFIG_NVT_LINUX_SPINORNAND_BOOT) || defined(CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT) || defined(CONFIG_NVT_LINUX_SPINOR_BOOT) || defined(CONFIG_NVT_LINUX_EMMC_BOOT)
int nvt_part_config(char *p_cmdline, EMB_PARTITION *partition_ptr)
{
	EMB_PARTITION *p_partition = NULL;
	uint32_t i, part_num, pst_part_num, ro_attr;
	int exist_linux = 0;
	int ret = 0;
	char cmd[512] = {0};
	char cmd_remain[512] = {0};
	char buf[40] = {0};

#if defined(CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT)
#if defined(CONFIG_NVT_SPI_NAND) || defined(CONFIG_NVT_SPI_NOR_NAND)
	struct mtd_info *nand = get_nand_dev_by_index(nand_curr_device);
#elif defined(CONFIG_NVT_IVOT_EMMC)
	struct mmc *mmc = find_mmc_device(CONFIG_NVT_IVOT_EMMC);
#endif /* CONFIG_NVT_SPI_NAND */
#endif /* CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */

#if defined(CONFIG_NVT_LINUX_EMMC_BOOT)
	/* MMC device doesn't need to init mtdparts parameter */
	if (p_cmdline == NULL) {
		return 0;
	}
#endif /* CONFIG_NVT_LINUX_EMMC_BOOT */

	if (partition_ptr == NULL) {
		p_partition = emb_partition_info_data_curr;
		ret = nvt_getfdt_emb((ulong)nvt_fdt_buffer, p_partition);
		if (ret < 0) {
			printf("failed to get partition resource.\r\n");
			return -1;
		}
	} else {
		/* Receive partition table from argu */
		p_partition = partition_ptr;
	}

	// set mtdids
	memset(cmd, 0, sizeof(cmd));
	// NOR and NAND => mtdparts, EMMC => nvtemmcpart
#if defined(CONFIG_NVT_IVOT_EMMC)
	if (p_cmdline != NULL) {
		/* To find if mtdparts string existed. If yes, it needs to expand mtdparts environment */
		char *nvtemmcpart_off = NULL;
		nvtemmcpart_off = strstr((char *)p_cmdline, "nvtemmcpart=");
		if (nvtemmcpart_off) {
			p_cmdline = nvtemmcpart_off;
			*p_cmdline = '\0';
		}
		strcat(p_cmdline, "nvtemmcpart=");
	}
#endif

	part_num = 0;
	pst_part_num = 0;
	ro_attr = 0;
	EMB_PARTITION *p = p_partition;
	u64 PartitionSize = 0, PartitionOffset = 0;
	nvt_flash_type_t FlashType = p->FlashType;
	nvt_flash_type_t FlashTypePrevious = p->FlashType;

	// init mtdids and mtdparts
	switch(FlashType) {
		case FLASHTYPE_NOR:
			sprintf(cmd, "nor0=spi_nor.0");
			ret = env_set("mtdids", cmd);
			if (p_cmdline != NULL) {
				/* To find if mtdparts string existed. If yes, it needs to expand mtdparts environment */
				char *mtdparts_off = NULL;
				mtdparts_off = strstr((char *)p_cmdline, "mtdparts=");
				if (mtdparts_off) {
				p_cmdline = mtdparts_off;
				*p_cmdline = '\0';
				}
				strcat(p_cmdline, "mtdparts=spi_nor.0:");
			}

			sprintf(cmd, "mtdparts=spi_nor.0:");
			break;
		case FLASHTYPE_NAND:
			sprintf(cmd, "nand0=spi_nand.0");
			ret = env_set("mtdids", cmd);
			sprintf(cmd, "mtdparts=spi_nand.0:");
			break;
		case FLASHTYPE_EMMC:
#if defined(CONFIG_NVT_IVOT_EMMC)
			sprintf(cmd, "mmcblk%d:", CONFIG_NVT_IVOT_EMMC);
#else
			/* no emmc support*/
#endif
			break;
		default:
			printf("%s %s:flash type %d is not support %s\r\n", ANSI_COLOR_RED, __func__, FlashType, ANSI_COLOR_RESET);
			break;
	}

	/* To parse mtdparts for rootfs partition table */
	for (i = 0 ; i < EMB_PARTITION_INFO_COUNT ; i++) {
		p = p_partition + i;
		PartitionSize = p->PartitionSize;
		PartitionOffset = p->PartitionOffset;
		FlashType = p->FlashType;

		if (p->EmbType != EMBTYPE_UNKNOWN && PartitionSize == 0) {
			printf("%s skip mtdpart config of partition[%d], because its size is 0\n%s", ANSI_COLOR_YELLOW, i, ANSI_COLOR_RESET);
			continue;
		}

		if(FlashType != FlashTypePrevious){
			if (PartitionSize <= 0) {
				// printf("%s stop when [%d], because its size <= 0\n%s", ANSI_COLOR_YELLOW, i, ANSI_COLOR_RESET);
				break;
			}

			switch(FlashType) {
				case FLASHTYPE_NOR:
				if(env_get("mtdids") != NULL){
					sprintf(buf, env_get("mtdids"));
				}
				strcat(buf, ",nor0=spi_nor.0");
				env_set("mtdids", buf);

				sprintf(buf, ";spi_nor.0:");
				break;
			case FLASHTYPE_NAND:
				if(env_get("mtdids") != NULL){
					sprintf(buf, env_get("mtdids"));
				}
				strcat(buf, ",nand0=spi_nand.0");
				env_set("mtdids", buf);

				sprintf(buf, ";spi_nand.0:");

				break;
			case FLASHTYPE_EMMC:
				/* MMC device doesn't need mtdparts */
				break;
			default:
				printf("%s %s:flash type %d is not support %s\r\n", ANSI_COLOR_RED, __func__, FlashType, ANSI_COLOR_RESET);
				return -1;
				break;
			}
			strcat(cmd, buf);
			FlashTypePrevious = FlashType;
		}

		switch (p->EmbType) {
			case EMBTYPE_LOADER:
				if(FlashType == FLASHTYPE_EMMC) {
					if (p_cmdline != NULL) {
						sprintf(buf, "0x%llx@0x%llx(loader)ro,", PartitionSize, PartitionOffset);
						/* loader does not need to be recorded in nvtemmcpart*/
						strcat(cmd_remain, buf);
					}
				} else {
					/* loader does not need to be recorded in mtdparts*/
				}
				break;
			case EMBTYPE_FDT:
			case EMBTYPE_FDTFAST:
				if(FlashType == FLASHTYPE_EMMC) {
					if (p_cmdline != NULL) {
						if (p->OrderIdx == 0) {
							sprintf(buf, "0x%llx@0x%llx(fdt)ro,", PartitionSize, PartitionOffset);
						} else {
							sprintf(buf, "0x%llx@0x%llx(fdt%d)ro,", PartitionSize, PartitionOffset, p->OrderIdx);
						}
						strcat(p_cmdline, buf);
						strcat(cmd_remain, buf);

					}
				} else {
					if (p->OrderIdx == 0) {
						sprintf(buf, "0x%llx@0x%llx(fdt),", PartitionSize, PartitionOffset);
					} else {
						sprintf(buf, "0x%llx@0x%llx(fdt%d),", PartitionSize, PartitionOffset, p->OrderIdx);
					}
					strcat(cmd, buf);
				}
				break;
			case EMBTYPE_ATF:
				if(FlashType == FLASHTYPE_EMMC) {
					if (p_cmdline != NULL) {
						sprintf(buf, "0x%llx@0x%llx(atf)ro,", PartitionSize, PartitionOffset);
						strcat(p_cmdline, buf);
						strcat(cmd_remain, buf);
					}
				} else {
					sprintf(buf, "0x%llx@0x%llx(atf),", PartitionSize, PartitionOffset);
					strcat(cmd, buf);
				}
				break;
			case EMBTYPE_UBOOT:
				if(FlashType == FLASHTYPE_EMMC) {
					if (p_cmdline != NULL) {
						sprintf(buf, "0x%llx@0x%llx(uboot)ro,", PartitionSize, PartitionOffset);
						strcat(p_cmdline, buf);
						strcat(cmd_remain, buf);
					}
				} else {
					sprintf(buf, "0x%llx@0x%llx(uboot),", PartitionSize, PartitionOffset);
					strcat(cmd, buf);
				}
				break;
			case EMBTYPE_UENV:
				sprintf(buf, "0x%llx@0x%llx(uenv),", PartitionSize, PartitionOffset);
				if(FlashType == FLASHTYPE_EMMC) {
					if (p_cmdline != NULL) {
						strcat(p_cmdline, buf);
						strcat(cmd_remain, buf);
					}
				} else {
					strcat(cmd, buf);
				}
				break;
			case EMBTYPE_LINUX:
				exist_linux = 1;
				if(FlashType == FLASHTYPE_EMMC) {
					if (p_cmdline != NULL) {
						sprintf(buf, "0x%llx@0x%llx(linux)ro,", PartitionSize, PartitionOffset);
						strcat(p_cmdline, buf);
						strcat(cmd_remain, buf);
					}
				} else {
					sprintf(buf, "0x%llx@0x%llx(linux),", PartitionSize, PartitionOffset);
					strcat(cmd, buf);
				}
				break;
			case EMBTYPE_RAMFS:
				sprintf(buf, "0x%llx@0x%llx(ramfs),", PartitionSize, PartitionOffset);
				if(FlashType == FLASHTYPE_EMMC) {
					strcat(p_cmdline, buf);
					strcat(cmd_remain, buf);
				} else {
					strcat(cmd, buf);
				}
				break;
			case EMBTYPE_ROOTFS:
				sprintf(buf, "0x%llx@0x%llx(rootfs%d),", PartitionSize, PartitionOffset, p->OrderIdx);
				if(FlashType == FLASHTYPE_EMMC) {
					if (p_cmdline != NULL) {
						strcat(p_cmdline, buf);
						strcat(cmd, buf);
						/* Multiple rootfs partitions if OrderIdx > 0 */
						ro_attr = p->OrderIdx;
					}
				} else {
					strcat(cmd, buf);
				}
				break;
			case EMBTYPE_ROOTFSL:
				sprintf(buf, "0x%llx@0x%llx(rootfsl%d),", PartitionSize, PartitionOffset, p->OrderIdx);
				if(FlashType == FLASHTYPE_EMMC) {
					if (p_cmdline != NULL) {
						strcat(p_cmdline, buf);
						strcat(cmd, buf);
						/* Multiple rootfs partitions if OrderIdx > 0 */
						ro_attr = p->OrderIdx;
					}
				} else {
					strcat(cmd, buf);
				}
				break;
			case EMBTYPE_RTOS:
				if(FlashType == FLASHTYPE_EMMC) {
					if (p_cmdline != NULL) {
						if (p->OrderIdx == 0) {
							sprintf(buf, "0x%llx@0x%llx(rtos)ro,", PartitionSize, PartitionOffset);
						} else {
							sprintf(buf, "0x%llx@0x%llx(rtos%d)ro,", PartitionSize, PartitionOffset, p->OrderIdx);
						}
						strcat(p_cmdline, buf);
						strcat(cmd_remain, buf);
					}
				} else {
					if (p->OrderIdx == 0) {
						sprintf(buf, "0x%llx@0x%llx(rtos),", PartitionSize, PartitionOffset);
					} else {
						sprintf(buf, "0x%llx@0x%llx(rtos%d),", PartitionSize, PartitionOffset, p->OrderIdx);
					}
					strcat(cmd, buf);
				}
				break;
			case EMBTYPE_APP:
				sprintf(buf, "0x%llx@0x%llx(app),", PartitionSize, PartitionOffset);
				if(FlashType == FLASHTYPE_EMMC) {
					if (p_cmdline != NULL) {
						strcat(p_cmdline, buf);
						strcat(cmd, buf);
					}
				} else {
					strcat(cmd, buf);
				}
				break;
			case EMBTYPE_TEEOS:
				sprintf(buf, "0x%llx@0x%llx(teeos),", PartitionSize, PartitionOffset);
				if(FlashType == FLASHTYPE_EMMC) {
					if (p_cmdline != NULL) {
						strcat(p_cmdline, buf);
						strcat(cmd_remain, buf);
					}
				} else {
					strcat(cmd, buf);
				}
				break;
			case EMBTYPE_AI:
				if(FlashType == FLASHTYPE_EMMC) {
					if (p_cmdline != NULL) {
						sprintf(buf, "0x%llx@0x%llx(ai)ro,", PartitionSize, PartitionOffset);
						strcat(p_cmdline, buf);
						strcat(cmd_remain, buf);
					}
				} else {
					sprintf(buf, "0x%llx@0x%llx(ai),", PartitionSize, PartitionOffset);
					strcat(cmd, buf);
				}
				break;
			case EMBTYPE_USER:
				sprintf(buf, "0x%llx@0x%llx(user%u_%d),", PartitionSize, PartitionOffset, GET_USER_PART_NUM(p->EmbType), p->OrderIdx);
				if(FlashType == FLASHTYPE_EMMC) {
					if (p_cmdline != NULL) {
						strcat(p_cmdline, buf);
						strcat(cmd_remain, buf);
					}
				} else {
					strcat(cmd, buf);
				}
				break;
			case EMBTYPE_USERRAW:
				sprintf(buf, "0x%llx@0x%llx(userraw%u_%d),", PartitionSize, PartitionOffset, GET_USER_PART_NUM(p->EmbType), p->OrderIdx);
				if(FlashType == FLASHTYPE_EMMC) {
					if (p_cmdline != NULL) {
						strcat(p_cmdline, buf);
						strcat(cmd_remain, buf);
					}
				} else {
					strcat(cmd, buf);
				}
				break;
			case EMBTYPE_BANK:
				if(FlashType == FLASHTYPE_EMMC) {
					if (p_cmdline != NULL) {
						if (p->OrderIdx == 0) {
							sprintf(buf, "0x%llx@0x%llx(bank)ro,", PartitionSize, PartitionOffset);
						} else {
							sprintf(buf, "0x%llx@0x%llx(bank%d)ro,", PartitionSize, PartitionOffset, p->OrderIdx);
						}
						strcat(p_cmdline, buf);
						strcat(cmd_remain, buf);
					}
				} else {
					if (p->OrderIdx == 0) {
						sprintf(buf, "0x%llx@0x%llx(bank),", PartitionSize, PartitionOffset);
					} else {
						sprintf(buf, "0x%llx@0x%llx(bank%d),", PartitionSize, PartitionOffset, p->OrderIdx);
					}
					strcat(cmd, buf);
				}
				break;
			case EMBTYPE_MCU:
				if(FlashType == FLASHTYPE_EMMC) {
					if (p_cmdline != NULL) {
						sprintf(buf, "0x%llx@0x%llx(mcu)ro,", PartitionSize, PartitionOffset);
						strcat(p_cmdline, buf);
						strcat(cmd_remain, buf);
					}
				} else {
					sprintf(buf, "0x%llx@0x%llx(mcu),", PartitionSize, PartitionOffset);
					strcat(cmd, buf);
				}
				break;
			default:
				break;
		}
	}

#if defined(CONFIG_NVT_SPI_NOR) ||defined(CONFIG_NVT_SPI_NAND) || defined(CONFIG_NVT_SPI_NOR_NAND)
	/* To handle uboot mtd env config */
	cmd[strlen(cmd) - 1] = '\0';
	ret = env_set("mtdparts", cmd);

	sprintf(cmd, "printenv mtdparts");
	run_command(cmd, 0);
#endif

	// for pure rtos project, dont enter to make rootfs cmdline
	if (p_cmdline != NULL && exist_linux) {
		/* To add entire nand mtd device */
#if defined(CONFIG_NVT_IVOT_EMMC)
		sprintf(buf, "0x%llx@0(total),", mmc->capacity / MMC_MAX_BLOCK_LEN);
		strcat(p_cmdline, buf);
		strcat(cmd, cmd_remain);
		cmd[strlen(cmd) - 1] = ' ';

		/* If linux major version is greater than 6, blkdevparts is required
		 * cmd_remain is used to store partitions other than rootfs and app
		 */
		ret = env_set("blkdevparts", cmd);
#endif
		p_cmdline[strlen(p_cmdline) - 1] = ' ';

#if defined(NVT_LINUX_RAMDISK_SUPPORT)
		// common root=/dev/ram0 is defined in nvt-peri.dtsi
#else
		/*
		 * some common para about rootfs are defined in nvt-peri.dtsi
		 * We add dynamically changing parameters through the following process
		 */

		// get rootfs partitiotn num
		ret = nvt_getfdt_rootfs_mtd_num((ulong)nvt_fdt_buffer, &part_num, &ro_attr);

		if (ret < 0) {
			printf("%s We can't find your root device, \
					please check if your dts is without partition_rootfs name %s\n",
					ANSI_COLOR_RED, ANSI_COLOR_RESET);
			return -1;
		}

		// to save bootargs for get_part function to get rootfs0 type.
		env_set("bootargs",p_cmdline);

		loff_t part_off = 0, part_size = 0;
		get_part_and_flash_type("rootfs0", &part_off, &part_size, &FlashType);

		/* _NVT_ROOTFS_TYPE_ */
		switch(FlashType) {
			case FLASHTYPE_NOR:
#if defined(NVT_UBIFS_SUPPORT)
				sprintf(buf, " ubi.mtd=%d ", part_num);
#else
				// To handle non-ubifs rootfs type (squashfs.jffs2)
				sprintf(buf, " root=/dev/mtdblock%d ", part_num);
#endif
				break;
			case FLASHTYPE_NAND:
#if defined(_NVT_ROOTFS_TYPE_NAND_SQUASH_NEW_)
				sprintf(buf, " root=/dev/romblock%d ", part_num);
#elif defined(NVT_UBIFS_SUPPORT) || defined(NVT_SQUASH_SUPPORT)
				// ubi. squashfs over ubi
				sprintf(buf, " ubi.mtd=%d ", part_num);
#else
                // To handle non-ubifs rootfs type (squashfs/jffs2)
                sprintf(buf, " root=/dev/mtdblock%d ", part_num);
#endif
				break;
			case FLASHTYPE_EMMC:
#if defined(NVT_EXT4_SUPPORT) || (defined(CONFIG_NVT_LINUX_EMMC_BOOT) && defined(NVT_SQUASH_SUPPORT))
				sprintf(buf, " root=/dev/mmcblk%dp%d nvt_pst=/dev/mmcblk%dp%d ", CONFIG_NVT_IVOT_EMMC, part_num, CONFIG_NVT_IVOT_EMMC, pst_part_num);
#endif
				break;
			default:
				printf("%s %s:storage type %d is not support %s\r\n", ANSI_COLOR_RED, __func__, FlashType, ANSI_COLOR_RESET);
		}
		strcat(p_cmdline, buf);
#endif // #if defined(NVT_ROOTFS_TYPE_RAMDISK)
	}

	return ret;
}
#endif /* CONFIG_NVT_LINUX_SPINAND_BOOT || CONFIG_NVT_LINUX_AUTODETECT || CONFIG_NVT_LINUX_SPINOR_BOOT || defined(CONFIG_NVT_LINUX_EMMC_BOOT) */

#ifdef  CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT
int nvt_runfw_bin_chk_valid(ulong addr)
{
	unsigned int size = gHeadInfo.Resv1[0];
	NVTPACK_VERIFY_OUTPUT np_verify = {0};
	NVTPACK_GET_PARTITION_INPUT np_get_input;
	NVTPACK_MEM mem_in = {(void *)addr, size};

	memset(&np_get_input, 0, sizeof(np_get_input));

	if (nvtpack_verify(&mem_in, &np_verify) != NVTPACK_ER_SUCCESS) {
		printf("verify failed.\r\n");
		return -1;
	}
	if (np_verify.ver != NVTPACK_VER_16072017) {
		printf("wrong all-in-one bin version\r\n");
		return -1;
	}

	return 0;
}

static int nvt_on_partition_enum_copy_to_dest(unsigned int id, NVTPACK_MEM *p_mem, void *p_user_data)
{
	EMB_PARTITION *pEmb = (EMB_PARTITION *)p_user_data;
	ulong src_addr = 0, dst_addr = 0, dst_size = 0;
	image_header_t *hdr = NULL;
	int len;
	//NVTPACK_BFC_HDR *pBFC = (NVTPACK_BFC_HDR *)p_mem->p_data;
#ifdef CONFIG_ARM64
	bool ret;
#endif

	/* Copy to dest address */
	switch (pEmb[id].EmbType) {
	case EMBTYPE_LOADER:
		printf("RUNFW: Ignore T.bin loader\n");
		break;
	case EMBTYPE_FDT:
	case EMBTYPE_FDTFAST:
		/**
		 * for some all-in-one bin, there are many bin with fdt format in it.
		 * Only system fdt (nvt-evb.bin) need to copy to nvt_fdt_buffer
		 * Check if /model property is existing, which comes from nvt-basic.dtsi
		 * to indicate system fdt.
		 */
		if (fdt_getprop(p_mem->p_data, 0, "model", &len) != NULL) {
			dst_addr = (ulong)nvt_fdt_buffer;
			dst_size = p_mem->len;
			src_addr = (ulong)p_mem->p_data;
			printf("RUNFW: Copy FDT from 0x%lx to 0x%lx with length %lu bytes\r\n",
				   (ulong)p_mem->p_data, (ulong)dst_addr, dst_size);
		}
		break;
	case EMBTYPE_UBOOT:
	case EMBTYPE_DSP:
	case EMBTYPE_RTOS:
	case EMBTYPE_AI:
	case EMBTYPE_MCU:
		break;
	case EMBTYPE_LINUX:
		dst_addr = nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr;
		dst_size = p_mem->len;
		src_addr = (ulong)p_mem->p_data;
		printf("RUNFW: Copy Linux from 0x%08lx to 0x%08lx with length %lu bytes\r\n",
			   (ulong)p_mem->p_data, (ulong)dst_addr, dst_size);
#ifdef CONFIG_ARM64
		char cmd[256] = {0};

		sprintf(cmd, "0x%lx", nvt_memory_cfg[MEMTYPE_LINUX].addr);
		env_set("kernel_comp_addr_r", cmd);
		sprintf(cmd, "0x%lx", dst_size);
		env_set("kernel_comp_size", cmd);
#endif
		break;
	case EMBTYPE_ROOTFS:
#ifdef NVT_LINUX_RAMDISK_SUPPORT

		/* Check if the image type is Ramdisk, if not skip it */
#ifdef CONFIG_NVT_BIN_CHKSUM_SUPPORT
		src_addr = (ulong)p_mem->p_data;
		src_addr += 64;
#endif
		hdr = (image_header_t *)src_addr;
		if (image_check_type(hdr, IH_TYPE_RAMDISK) == 0 || pEmb[id].OrderIdx != 0) {
			nvt_dbg(MSG, "not a ramdisk rootfs skip it : id%d\n", id);
			return 0;
		}

		nvt_ramdisk_size = ALIGN_CEIL(p_mem->len, 4096);
		nvt_ramdisk_addr = nvt_memory_cfg[MEMTYPE_LINUX].addr + nvt_memory_cfg[MEMTYPE_LINUX].size - nvt_ramdisk_size - 0x10000;
		dst_addr = nvt_ramdisk_addr;
		dst_size = nvt_ramdisk_size;
		src_addr = (ulong)p_mem->p_data;
#ifdef CONFIG_NVT_BIN_CHKSUM_SUPPORT
		// Skip nvt head info
		src_addr += 64;
		dst_size -= 64;
#endif
		printf("RUNFW: Copy RAMDISK from 0x%lx to 0x%lx with length %lu bytes\r\n",
			   src_addr, dst_addr, dst_size);


#endif /* NVT_LINUX_RAMDISK_SUPPORT */
		break;
	default:
		break;
	}

	if (dst_size != 0) {
		memmove((void *)dst_addr, (void *)src_addr, dst_size);
		flush_dcache_range(round_down((ulong)dst_addr, CONFIG_SYS_CACHELINE_SIZE), round_up((ulong)(dst_addr + dst_size), CONFIG_SYS_CACHELINE_SIZE));
	}
	return 0;
}

#define GZ_BUF_SIZE 0x4000 //16KB are enough
unsigned char gz_buf[0x4000] = {0};
unsigned char *mp_gz_buf_curr = gz_buf;
static void *gz_alloc(void *x, unsigned items, unsigned size)
{
	void *p = NULL;
	if (mp_gz_buf_curr + ALIGN_CEIL_4(size) > gz_buf + GZ_BUF_SIZE) {
		printf("gzip temp memory not enough, need more: %d bytes\r\n", ALIGN_CEIL_4(size));
		return p;
	}

	p = (void *)(mp_gz_buf_curr);
	mp_gz_buf_curr += ALIGN_CEIL_4(size);
	return p;
}

static void gz_free(void *x, void *addr, unsigned nb)
{
	mp_gz_buf_curr = gz_buf;
}

int gz_uncompress(unsigned char *in, unsigned char *out,
				  unsigned int insize, unsigned int outsize)
{
	int err;
	z_stream stream = {0};
	stream.next_in = (Bytef *)in;
	stream.avail_in = insize;
	stream.next_out = (Bytef *)out;
	stream.avail_out = outsize;
	stream.zalloc = (alloc_func)gz_alloc;
	stream.zfree = (free_func)gz_free;
	stream.opaque = (voidpf)0;
	err = inflateInit(&stream);
	if (err != Z_OK) {
		printf("Failed to inflateInit, err = %d\r\n", err);
		inflateEnd(&stream);
		return -1;
	}

	err = inflate(&stream, Z_NO_FLUSH);

	inflateEnd(&stream);

	if (err == Z_STREAM_END) {
		return stream.total_out;
	}

	return 0;
}

static int nvt_runfw_bin_unpack_to_dest(ulong addr)
{
	u32 i;
	EMB_PARTITION *pEmb = emb_partition_info_data_curr;
	int ret = 0;
	unsigned int size = gHeadInfo.Resv1[0];
	NVTPACK_GET_PARTITION_INPUT np_get_input;
	NVTPACK_ENUM_PARTITION_INPUT np_enum_input;
	NVTPACK_MEM mem_in = {(void *)addr, size};
	NVTPACK_MEM mem_out = {0};

	memset(&np_get_input, 0, sizeof(np_get_input));
	memset(&np_enum_input, 0, sizeof(np_enum_input));

	ret = nvt_getfdt_emb((ulong)nvt_fdt_buffer, pEmb);
	if (ret < 0) {
		printf("failed to get current partition resource.\r\n");
		return -1;
	}

	/* Check for all partition existed */
	for (i = 1; i < EMB_PARTITION_INFO_COUNT; i++) {
		if (pEmb[i].PartitionSize != 0) {
			switch (pEmb[i].EmbType) {
			case EMBTYPE_FDT:
			case EMBTYPE_FDTFAST:
			case EMBTYPE_UBOOT:
			case EMBTYPE_LINUX:
#ifdef NVT_LINUX_RAMDISK_SUPPORT
			case EMBTYPE_ROOTFS:
#endif
				//check this type exist in all-in-one
				np_get_input.id = i;
				np_get_input.mem = mem_in;
				if (nvtpack_check_partition(pEmb[i], &np_get_input, &mem_out) != NVTPACK_ER_SUCCESS) {
					printf("RUNFW boot: need partition[%d]\n", i);
					return -1;
				}
				break;
			}
		}
	}

	/* Enum all partition to do necessary handling */
	np_enum_input.mem = mem_in;
	np_enum_input.p_user_data = pEmb;
	/* Loading images */
	np_enum_input.fp_enum = nvt_on_partition_enum_copy_to_dest;

	if (nvtpack_enum_partition(&np_enum_input) != NVTPACK_ER_SUCCESS) {
		printf("failed sanity.\r\n");
		return -1;
	}

	/* Make sure images had been already copied */
	flush_dcache_all();

	return 0;
}

static ulong uimage_gen_dcrc(const image_header_t *hdr)
{
	ulong data = image_get_data(hdr);
	ulong len = image_get_data_size(hdr);
	return crc32_wd(0, (unsigned char *)data, len, CHUNKSZ_CRC32);
}

static ulong uimage_gen_hcrc(const image_header_t *hdr)
{
	ulong hcrc;
	ulong len = image_get_header_size();
	image_header_t header;

	/* Copy header so we can blank CRC field for re-calculation */
	memmove(&header, (char *)hdr, image_get_header_size());
	image_set_hcrc(&header, 0);

	return crc32(0, (unsigned char *)&header, len);
}

static int uimage_gen_fake_header(image_header_t *hdr,
								  unsigned long size,
								  unsigned long type)
{
	if (!hdr) {
		printf("uimage_gen_fake_header():hdr is NULL\n");
		return -1;
	}

	hdr->ih_magic   = UINT32_SWAP(0x27051956);
	hdr->ih_hcrc    = UINT32_SWAP(0);
	hdr->ih_time    = UINT32_SWAP(0);
	hdr->ih_size    = UINT32_SWAP(size);
	if (type == 2) {
#ifdef CONFIG_ARM64
		hdr->ih_load    = UINT32_SWAP(0x80000);
		hdr->ih_ep      = UINT32_SWAP(0x80000);
#else
		hdr->ih_load    = UINT32_SWAP(0x8000);
		hdr->ih_ep      = UINT32_SWAP(0x8000);
#endif
	} else {
		hdr->ih_load    = UINT32_SWAP(0);
		hdr->ih_ep      = UINT32_SWAP(0);
	}

	hdr->ih_dcrc    = UINT32_SWAP(0);
	hdr->ih_os      = 5;
#ifdef CONFIG_ARM64
	hdr->ih_arch    = IH_ARCH_ARM64;
#else
	hdr->ih_arch    = IH_ARCH_ARM;
#endif
	hdr->ih_type    = type;
	hdr->ih_comp    = 0;
	hdr->ih_name[0] = 0;
	hdr->ih_dcrc    = UINT32_SWAP(uimage_gen_dcrc(hdr));
	hdr->ih_hcrc    = UINT32_SWAP(uimage_gen_hcrc(hdr));

	return 0;
}

inline static int is_gzip_image(unsigned long image)
{
	return (((NVTPACK_BFC_HDR *)image)->uiFourCC ==
			MAKEFOURCC('B', 'C', 'L', '1'));
}

static int nvt_ker_img_ungzip_linux(unsigned long linux_addr,
									unsigned long workbuf)
{
	unsigned long gzip_image_size = 0;
	unsigned long image_size = 0;
	NVTPACK_BFC_HDR *bfc = NULL;
	char cmd[256] = {0};

	if (!is_gzip_image(linux_addr)) {
		nvt_dbg(IND, "not gzip linux\n");
		return 0;
	}

	//extract linux's size(gzipped & original)
	bfc             = (NVTPACK_BFC_HDR *)linux_addr;
	gzip_image_size = UINT32_SWAP(bfc->uiSizeComp);
	image_size      = UINT32_SWAP(bfc->uiSizeUnComp);

	if (gzip_image_size + sizeof(NVTPACK_BFC_HDR) > nvt_memory_cfg[MEMTYPE_LINUXTMP].addr) {
		nvt_dbg(ERR, "fail to gz_uncompress linux, gzip image over size (%lu)\n", image_size);
		return -1;
	}

	//copy gzipped linux to working buffer
	memcpy((char *)workbuf, (char *)linux_addr,
		   gzip_image_size + sizeof(NVTPACK_BFC_HDR));

	//uncompress linux
	nvt_dbg(IND, "gz_uncompress linux\n");
	if (image_size > nvt_memory_cfg[MEMTYPE_KERNEL_IMG].size) {
		nvt_dbg(ERR, "fail to gz_uncompress linux, image over size (%lu)\n", image_size);
		return -1;
	}
	if (gz_uncompress((unsigned char *)(workbuf + sizeof(NVTPACK_BFC_HDR)),
					  (void *)(linux_addr + sizeof(image_header_t)),
					  gzip_image_size,
					  image_size) == 0) {
		nvt_dbg(ERR, "fail to gz_uncompress linux\n");
		return -1;
	}

	//generate a fake uboot header so that uboot can recognize and boot linux
	nvt_dbg(IND, "generating uImage header for linux\n");
	if (uimage_gen_fake_header((image_header_t *)linux_addr,
							   image_size, 2)) {
		nvt_dbg(ERR, "fail to generate fake uboot header for linux\n");
		return -1;
	}

	sprintf(cmd, "0x%lx", image_size);
	env_set("kernel_comp_size", cmd);

	return 0;
}

static int nvt_ramdisk_img_ungzip_linux(unsigned long ramdisk_addr,
										unsigned long workbuf)
{
	unsigned long gzip_image_size = 0;
	unsigned long image_size = 0;
	NVTPACK_BFC_HDR *bfc = NULL;

	if (!is_gzip_image(ramdisk_addr)) {
		nvt_dbg(IND, "not gzip ramdisk\n");
		return 0;
	}

	//extract ramdisk's size(gzipped & original)
	bfc = (NVTPACK_BFC_HDR *)ramdisk_addr;
	gzip_image_size = UINT32_SWAP(bfc->uiSizeComp);
	image_size = UINT32_SWAP(bfc->uiSizeUnComp);

	//copy gzipped ramdisk to a working buffer
	if ((gzip_image_size + sizeof(NVTPACK_BFC_HDR)) > nvt_memory_cfg[MEMTYPE_LINUXTMP].addr) {
		nvt_dbg(ERR, "fail to gz_uncompress linux, gzip image over linux tmp (%lu)\n", image_size);
		return -1;
	}

	memcpy((char *)workbuf, (char *)ramdisk_addr,
		   gzip_image_size + sizeof(NVTPACK_BFC_HDR));

	//uncompress ramdisk
	nvt_dbg(IND, "gz_uncompress ramdisk\n");
	if (image_size > nvt_memory_cfg[MEMTYPE_LINUX].size) {
		nvt_dbg(ERR, "fail to gz_uncompress linux, image over size (%lu)\n", image_size);
		return -1;
	}

	if (gz_uncompress((unsigned char *)(workbuf + sizeof(NVTPACK_BFC_HDR)),
					  (void *)(ramdisk_addr + sizeof(image_header_t)),
					  gzip_image_size,
					  image_size) == 0) {
		nvt_dbg(ERR, "fail to gz_uncompress ramdisk\n");
		return -1;
	}

	//generate a fake uboot header so that uboot can recognize ramdisk
	nvt_dbg(IND, "generating uImage header for ramdisk\n");
	if (uimage_gen_fake_header((image_header_t *)ramdisk_addr,
							   image_size, 3)) {
		nvt_dbg(ERR, "fail to generate fake uboot header for ramdisk\n");
		return -1;
	}

	return 0;
}

static int nvt_boot_linux_bin_auto(void)
{
	char cmd[256] = {0};
	int ret = 0;
	image_header_t *hdr;
	unsigned long size;
#ifdef CONFIG_ENCRYPT_KERNEL
	unsigned int is_secure = 0;
	#if defined(CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT) && defined(CONFIG_NVT_IVOT_OPTEE_SUPPORT)
	NVT_SMC_EFUSE_DATA efuse_data = {0};
	efuse_data.cmd = NVT_SMC_EFUSE_IS_SECURE;

	is_secure = nvt_ivot_optee_efuse_operation(&efuse_data);
	#elif defined(CONFIG_NVT_IVOT_SECBOOT_SUPPORT)
	is_secure = is_secure_enable();
	#endif
#endif

	loff_t part_off = 0, part_size = 0;
	nvt_flash_type_t flash_type = FLASHTYPE_SIZE;
	u32 align_size = 0;
	u64 align_off = 0;
	get_part_and_flash_type("linux", &part_off, &part_size, &flash_type);

	if (nvt_detect_fw_tbin()) {
		/* Check fw is valid */
		nvt_runfw_bin_chk_valid((ulong)nvt_memory_cfg[MEMTYPE_ALL_IN_ONE].addr);
		/* Copy linux binary to destination address */
		ret = nvt_runfw_bin_unpack_to_dest((ulong)nvt_memory_cfg[MEMTYPE_ALL_IN_ONE].addr);
		if (ret < 0) {
			return ret;
		}
	} else {
#ifdef CONFIG_ENCRYPT_KERNEL
		// get uImage size firstly
		switch(flash_type) {
			case FLASHTYPE_NOR:
				align_size = ALIGN_CEIL(sizeof(image_header_t), ARCH_DMA_MINALIGN);
				sprintf(cmd, "sf read 0x%lx 0x%llx 0x%x", nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr, part_off, is_secure == 0 ? align_size : sizeof(HEADINFO));
				run_command(cmd, 0);
				break;
			case FLASHTYPE_NAND:
				sprintf(cmd, "nand read 0x%lx linux 0x%x", nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr, is_secure == 0 ? sizeof(image_header_t) : sizeof(HEADINFO));
				run_command(cmd, 0);
				break;
			case FLASHTYPE_EMMC:
				align_off = ALIGN_CEIL(part_off, MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN;
				sprintf(cmd, "mmc read 0x%lx 0x%llx 0x%x", nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr, align_off, 2);
				run_command(cmd, 0);
				break;
			default:
				printf("%s %s:storage type %d is not support %s\r\n", ANSI_COLOR_RED, __func__, flash_type, ANSI_COLOR_RESET);
		}
		if (is_secure == 0) {
			hdr = (image_header_t *)nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr;
			size = image_get_data_size(hdr) + sizeof(image_header_t);
		} else {
			HEADINFO *encrypt_header = (HEADINFO *)(nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr);
			size = encrypt_header->BinLength;
		}
#else
		// get uImage size firstly
		switch(flash_type) {
			case FLASHTYPE_NOR:
				align_size = ALIGN_CEIL(sizeof(image_header_t), ARCH_DMA_MINALIGN);
				sprintf(cmd, "sf read 0x%lx 0x%llx 0x%x", nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr, part_off, align_size);
				run_command(cmd, 0);
				break;
			case FLASHTYPE_NAND:
				sprintf(cmd, "nand read 0x%lx linux 0x%lx", nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr, sizeof(image_header_t));
				run_command(cmd, 0);
				break;
			case FLASHTYPE_EMMC:
				align_off = ALIGN_CEIL(part_off, MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN;
				sprintf(cmd, "mmc read 0x%lx 0x%llx 0x%x", nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr, align_off, 2);
				run_command(cmd, 0);
				break;
			default:
				printf("%s %s:storage type %d is not support %s\r\n", ANSI_COLOR_RED, __func__, flash_type, ANSI_COLOR_RESET);
		}

		hdr = (image_header_t *)nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr;
		size = image_get_data_size(hdr) + sizeof(image_header_t);
#endif /* CONFIG_ENCRYPT_KERNEL */

		// Reading Linux kernel image
		switch(flash_type) {
			case FLASHTYPE_NOR:
				align_size = ALIGN_CEIL(size, ARCH_DMA_MINALIGN);
				sprintf(cmd, "sf read 0x%lx 0x%llx 0x%x", nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr, part_off, align_size);
				run_command(cmd, 0);
				break;
			case FLASHTYPE_NAND:
				sprintf(cmd, "nand read 0x%lx linux 0x%lx", nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr, size);
				run_command(cmd, 0);
				break;
			case FLASHTYPE_EMMC:
				align_size = ALIGN_CEIL(size, MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN;
				sprintf(cmd, "mmc read 0x%lx 0x%llx 0x%x", nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr, align_off, align_size);
				run_command(cmd, 0);
				break;
			default:
				printf("%s %s:flash type %d is not support %s\r\n", ANSI_COLOR_RED, __func__, flash_type, ANSI_COLOR_RESET);
		}

		nvt_ker_img_size = size;
#ifdef CONFIG_ARM64
		nvt_ker_img_addr = nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr;
		char cmd[256] = {0};

		sprintf(cmd, "0x%lx", nvt_memory_cfg[MEMTYPE_LINUX].addr);
		env_set("kernel_comp_addr_r", cmd);

		if ((size < sizeof(image_header_t)) || (size > nvt_memory_cfg[MEMTYPE_KERNEL_IMG].size)) {
			printf("%s: kernel img size: %lx\n", __func__, size );
			return -1;
		}
		sprintf(cmd, "0x%lx", size);
		env_set("kernel_comp_size", cmd);
#else
		nvt_ker_img_addr = nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr;
#endif

	}
#ifdef CONFIG_ENCRYPT_KERNEL
	if (is_secure != 0) {
#ifdef CONFIG_NVT_IVOT_SECBOOT_SUPPORT
#ifdef CONFIG_CHECK_SIGNATURE_BEFORE_BOOT
		if(nvt_chk_signature_on_uboot((ulong)nvt_ker_img_addr, size)) {
			printf("check signature fail\n");
			return -1;
		}
#endif /* CONFIG_CHECK_SIGNATURE_BEFORE_BOOT */

		/* Decrypt Date */
		ret = nvt_decrypt_aes_on_uboot(nvt_ker_img_addr);
#elif defined(CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT)
#ifdef CONFIG_CHECK_SIGNATURE_BEFORE_BOOT
		if(nvt_chk_signature((ulong)nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr, size)) {
			printf("check signature fail\n");
			return -1;
		}
#endif /* CONFIG_CHECK_SIGNATURE_BEFORE_BOOT */

		/* Decrypt Date */
		ret = nvt_decrypt_aes((unsigned long)nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr);
#endif
		if (ret < 0) {
			nvt_dbg(ERR, "nvt_decrypt_aes fail ret %d\r\n", ret);
			return -1;
		}
		nvt_dbg(ERR, "nvt_decrypt_aes PASS\r\n");
	}
#endif

#ifndef CONFIG_NVT_SPI_NONE
	/* To check whether the kernel is nvt gzip format or not */
	ret = nvt_ker_img_ungzip_linux(nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr,
								   nvt_memory_cfg[MEMTYPE_LINUXTMP].addr);
	if (ret < 0) {
		nvt_dbg(ERR, "The nvt kernel image unzip failed with ret=%d \r\n", ret);
		return -1;
	}

	nvt_dbg(IND, "linux_addr:0x%lx\n", nvt_ker_img_addr);
	nvt_dbg(IND, "linux_size:0x%lx\n", nvt_ker_img_size);
#endif
	return 0;
}

static u32 nvt_get_32bits_data(ulong addr, bool is_little_endian)
{
	u_int32_t value = 0;
	u_int8_t *pb = (u_int8_t *)addr;
	if (is_little_endian) {
		if (addr & 0x3) { //NOT word aligned
			value = (*pb);
			value |= (*(pb + 1)) << 8;
			value |= (*(pb + 2)) << 16;
			value |= (*(pb + 3)) << 24;
		} else {
			value = *(u_int32_t *)addr;
		}
	} else {
		value = (*pb) << 24;
		value |= (*(pb + 1)) << 16;
		value |= (*(pb + 2)) << 8;
		value |= (*(pb + 3));
	}
	return value;
}

#if defined(NVT_LINUX_RAMDISK_SUPPORT)
static int nvt_boot_rootfs_bin_auto(void)
{
	u64 ramdisk_partition_addr;
	int ret;
	char cmd[256] = {0};
	image_header_t *hdr;
	unsigned int hdr_tmp = 0;
	u64 align_off = 0;
	u32 align_size = 0;
	loff_t part_off = 0, part_size = 0;
	nvt_flash_type_t flash_type = FLASHTYPE_SIZE;

#ifdef CONFIG_ENCRYPT_ROOTFS
	unsigned int is_secure = 0;
	#if defined(CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT) && defined(CONFIG_NVT_IVOT_OPTEE_SUPPORT)
	NVT_SMC_EFUSE_DATA efuse_data = {0};
	efuse_data.cmd = NVT_SMC_EFUSE_IS_SECURE;

	is_secure = nvt_ivot_optee_efuse_operation(&efuse_data);
	#elif defined(CONFIG_NVT_IVOT_SECBOOT_SUPPORT)
	is_secure = is_secure_enable();
	#endif
#endif

	// get rootfs partition info from fdt
	get_part_and_flash_type("rootfs0", &part_off, &part_size, &flash_type);
	ramdisk_partition_addr = part_off;
	printf("%s: ramdisk_partition_addr: 0x%llx\n", __func__, ramdisk_partition_addr);

	nvt_ramdisk_addr = (ulong)memalign(CONFIG_SYS_CACHELINE_SIZE, 0x20000);
	if (!nvt_ramdisk_addr) {
		printf("nvt_ramdisk_addr alloc fail\n");
		return -1;
	}

#ifdef CONFIG_ENCRYPT_ROOTFS
	// read roofs header firstly
	switch(flash_type) {
		case FLASHTYPE_NOR:
			align_size = ALIGN_CEIL(sizeof(image_header_t), ARCH_DMA_MINALIGN);
			sprintf(cmd, "sf read 0x%lx 0x%llx 0x%x", nvt_ramdisk_addr, ramdisk_partition_addr, is_secure == 0 ? align_size : sizeof(HEADINFO));
			run_command(cmd, 0);
			break;
		case FLASHTYPE_NAND:
			sprintf(cmd, "nand read 0x%lx 0x%llx 0x%lx", nvt_ramdisk_addr, ramdisk_partition_addr, is_secure == 0 ? sizeof(image_header_t) : sizeof(HEADINFO));
			run_command(cmd, 0);
			break;
		case FLASHTYPE_EMMC:
			align_off = ALIGN_CEIL(ramdisk_partition_addr, MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN;
			sprintf(cmd, "mmc read 0x%lx 0x%llx 0x%x", nvt_ramdisk_addr, align_off, 2);
			run_command(cmd, 0);
			break;
		default:
			printf("%s %s:flash type %d is not support %s\r\n", ANSI_COLOR_RED, __func__, flash_type, ANSI_COLOR_RESET);
			break;
	}

	if (is_secure == 0) {
		if (is_gzip_image(nvt_ramdisk_addr)) {
			// fastboot ramdisk
			NVTPACK_BFC_HDR *bfc = (NVTPACK_BFC_HDR *)nvt_ramdisk_addr;
			nvt_ramdisk_size = UINT32_SWAP(bfc->uiSizeUnComp) + sizeof(NVTPACK_BFC_HDR);
		} else {
			// non fastboot ramdisk
			hdr = (image_header_t *)nvt_ramdisk_addr;
			nvt_ramdisk_size = image_get_data_size(hdr) + sizeof(image_header_t);
		}
	} else {
		HEADINFO *encrypt_header = (HEADINFO *)nvt_ramdisk_addr;
		nvt_ramdisk_size = encrypt_header->BinLength;
	}

#else

	// read roofs header firstly
	switch(flash_type) {
		case FLASHTYPE_NOR:
			align_size = ALIGN_CEIL(sizeof(image_header_t), ARCH_DMA_MINALIGN);
			sprintf(cmd, "sf read 0x%lx 0x%llx 0x%x", nvt_ramdisk_addr, ramdisk_partition_addr, align_size);
			run_command(cmd, 0);
			break;
		case FLASHTYPE_NAND:
			sprintf(cmd, "nand read 0x%lx 0x%llx 0x%lx", nvt_ramdisk_addr, ramdisk_partition_addr, sizeof(image_header_t));
			run_command(cmd, 0);
			break;
		case FLASHTYPE_EMMC:
			align_off = ALIGN_CEIL(ramdisk_partition_addr, MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN;
			sprintf(cmd, "mmc read 0x%lx 0x%llx 0x%x", nvt_ramdisk_addr, align_off, 2);
			run_command(cmd, 0);
			break;
		default:
			printf("%s %s:flash type %d is not support %s\r\n", ANSI_COLOR_RED, __func__, flash_type, ANSI_COLOR_RESET);
			break;
	}

	if (is_gzip_image(nvt_ramdisk_addr)) {
		// fastboot ramdisk
		NVTPACK_BFC_HDR *bfc = (NVTPACK_BFC_HDR *)nvt_ramdisk_addr;
		nvt_ramdisk_size = UINT32_SWAP(bfc->uiSizeUnComp) + sizeof(NVTPACK_BFC_HDR);
	} else {
		// non-fastboot ramdisk
		hdr = (image_header_t *)nvt_ramdisk_addr;
		nvt_ramdisk_size = image_get_data_size(hdr) + sizeof(image_header_t);
	}
#endif /* CONFIG_ENCRYPT_ROOTFS */

	nvt_ramdisk_size = ALIGN_CEIL(nvt_ramdisk_size, 4096);
	if (nvt_ramdisk_addr) {
		free((void *)nvt_ramdisk_addr);
	}
	nvt_ramdisk_addr = nvt_memory_cfg[MEMTYPE_LINUX].addr + nvt_memory_cfg[MEMTYPE_LINUX].size - nvt_ramdisk_size - 0x10000;

	/* Check whether the Linux and ramdisk region is overlap or not */
	if (nvt_ramdisk_addr > nvt_ker_img_addr) {
		if ((nvt_ker_img_addr + nvt_ker_img_size) > nvt_ramdisk_addr) {
			nvt_dbg(ERR, "Image overlap: linux addr 0x%08lx linux size 0x%08lx ramdisk addr 0x%08lx ramdisk size 0x%08lx\n",
					nvt_ker_img_addr, nvt_ker_img_size, nvt_ramdisk_addr, nvt_ker_img_addr);
			return -1;
		}
	} else {
		if ((nvt_ramdisk_addr + nvt_ramdisk_size) > nvt_ker_img_addr) {
			nvt_dbg(ERR, "Image overlap: linux addr 0x%08lx linux size 0x%08lx ramdisk addr 0x%08lx ramdisk size 0x%08lx\n",
					nvt_ker_img_addr, nvt_ker_img_size, nvt_ramdisk_addr, nvt_ker_img_addr);
			return -1;
		}
	}

	//nvt_ramdisk_size = ALIGN_CEIL(p_mem->len, 4096);
	nvt_dbg(IND, "ramdisk_addr:0x%lx\n", nvt_ramdisk_addr);
	nvt_dbg(IND, "ramdisk_size:0x%lx\n", nvt_ramdisk_size);

	switch(flash_type) {
		case FLASHTYPE_NOR:
			align_size = ALIGN_CEIL(nvt_ramdisk_size, ARCH_DMA_MINALIGN);
			sprintf(cmd, "sf read 0x%lx 0x%llx 0x%x", nvt_ramdisk_addr, ramdisk_partition_addr, align_size);
			run_command(cmd, 0);
			break;
		case FLASHTYPE_NAND:
			sprintf(cmd, "nand read 0x%lx 0x%llx 0x%lx", nvt_ramdisk_addr, ramdisk_partition_addr, nvt_ramdisk_size);
			run_command(cmd, 0);
			break;
		case FLASHTYPE_EMMC:
			align_size = ALIGN_CEIL(nvt_ramdisk_size, MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN;
			align_off = ALIGN_CEIL(ramdisk_partition_addr, MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN;
			sprintf(cmd, "mmc read 0x%lx 0x%llx 0x%x", nvt_ramdisk_addr, align_off, align_size);
			run_command(cmd, 0);
			break;
		default:
			printf("%s %s:flash type %d is not support %s\r\n", ANSI_COLOR_RED, __func__, flash_type, ANSI_COLOR_RESET);
			break;
	}

#ifdef CONFIG_ENCRYPT_ROOTFS
	if (is_secure != 0) {
#ifdef CONFIG_NVT_IVOT_SECBOOT_SUPPORT
#ifdef CONFIG_CHECK_SIGNATURE_BEFORE_BOOT
		if(nvt_chk_signature_on_uboot((ulong)nvt_ramdisk_addr, nvt_ramdisk_size)) {
			printf("check signature fail\n");
			return -1;
		}
#endif /* CONFIG_CHECK_SIGNATURE_BEFORE_BOOT */

		/* Decrypt Date */
		ret = nvt_decrypt_aes_on_uboot(nvt_ramdisk_addr);
#elif defined(CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT)
#ifdef CONFIG_CHECK_SIGNATURE_BEFORE_BOOT
		if(nvt_chk_signature((ulong)nvt_ramdisk_addr, nvt_ramdisk_size)) {
			printf("check signature fail\n");
			return -1;
		}
#endif /* CONFIG_CHECK_SIGNATURE_BEFORE_BOOT */

		/* Decrypt Date */
		ret = nvt_decrypt_aes((unsigned long)nvt_ramdisk_addr);
#endif
		if (ret < 0) {
			nvt_dbg(ERR, "nvt_decrypt_aes fail ret %d\r\n", ret);
			return -1;
		}
		nvt_dbg(ERR, "nvt_decrypt_aes PASS\r\n");
	}
	nvt_ramdisk_addr = nvt_ramdisk_addr + 64;
#endif

	/* To check whether the ramdisk format is nvt gzip format or not */
	ret = nvt_ramdisk_img_ungzip_linux(nvt_ramdisk_addr,
									   nvt_memory_cfg[MEMTYPE_LINUXTMP].addr);
	if (ret < 0) {
		nvt_dbg(ERR, "The nvt ramdisk image unzip failed with ret=%d \r\n", ret);
		return -1;
	}

	return 0;
}
#endif /* NVT_LINUX_RAMDISK_SUPPORT */

static int nvt_boot_fw_init_detect(void)
{
	ulong rtos_addr = 0, rtos_size = 0;
	printf("NVT firmware boot.....\n");
	rtos_addr = nvt_memory_cfg[MEMTYPE_RTOS].addr;
	rtos_size = nvt_memory_cfg[MEMTYPE_RTOS].size;

#if defined(CONFIG_TARGET_NS02301) || defined(CONFIG_TARGET_NS02302_A64)
	if (rtos_size) {
		/**
		 * 567, 538 fastboot use amp,
		 * on amp, uboot always boot on cpu[1]
		 * the boot sequence needs always boot rtos to bring up cpu[0]' linux-kernel
		 */
		nvt_boot_rtos_bin_auto();
	}
#endif
	if (rtos_size == 0 || rtos_disable_anchor == 1) {
		nvt_boot_linux_bin_auto();
#if defined(NVT_LINUX_RAMDISK_SUPPORT)
		if (!nvt_detect_fw_tbin()) {
			nvt_boot_rootfs_bin_auto();
		}
#endif
	} else {
		nvt_boot_rtos_bin_auto();
	}

	return 0;
}

#endif /* CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */

static int nvt_set_bootargs_by_dts(const void *fdt_addr)
{
	int  nodeoffset;
	void *val = NULL;

	if (fdt_addr == NULL) {
		nvt_dbg(ERR, "fdt_addr is NULL\n");
		return -1;
	}

	nodeoffset = fdt_path_offset((void *)fdt_addr, "/chosen");
	if (nodeoffset > 0) {
		val = (void *)fdt_getprop((const void *)fdt_addr, nodeoffset, "bootargs", NULL);
		if (val != NULL) {
			if (env_get("bootargs") != NULL) {
				char buf[1024] = {0};
				sprintf(buf, "%s", env_get("bootargs"));
				strcat(buf, val);
				env_set("bootargs", (void *)buf);
			} else {
				env_set("bootargs", (void *)val);
			}
		} else {
			nvt_dbg(ERR, "can not find bootargs on device tree\n");
			return -1;
		}
	} else {
		nvt_dbg(ERR, "can not find /chosen on device tree\n");
		return -1;
	}
	return 0;
}

#if defined(CONFIG_NVT_IVOT_EMMC)
int update_bootargs_for_blkdevparts() {
	char temp_cmdline[512] = {0};
	char buf[512] = {0};
	int ret = 0;
#ifdef CONFIG_USE_BOOTARGS
	sprintf(temp_cmdline, "%s ", CONFIG_BOOTARGS);
#endif
	sprintf(buf, "blkdevparts=%s ", env_get("blkdevparts"));

	if (strlcat(temp_cmdline, buf, sizeof(temp_cmdline)) >= sizeof(temp_cmdline)) {
		printf("bootargs overflow after %s\n", __func__);
        ret = -1;
	}

	env_set("bootargs", temp_cmdline);

	return ret;
}
#endif

int do_nvt_boot_cmd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = 0;
	char cmdline[512] = {0};
	char cmd[256] = {0};
	char buf[255] = {0};
	unsigned int is_secure = 0;

	nvt_dbg(FUNC, "boot time: %lu(us)\n", get_nvt_timer0_cnt());
	/*
	 * Loading linux kernel
	 */
#ifdef  CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT
	ret = nvt_boot_fw_init_detect();
	if (ret < 0) {
		printf("boot firmware init failed\n");
		return ret;
	}
#else /* !CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */
	/* To do customized boot */
	return 0;
#endif /* CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */

#if defined(CONFIG_NVT_IVOT_EMMC)
	if (get_linux_major_version() > 5){
		ret = update_bootargs_for_blkdevparts();
		if (ret < 0) {
			printf("get_linux_major_version failed\n");
			return ret;
		}
	}
#endif

	if (nvt_set_bootargs_by_dts(nvt_fdt_buffer) < 0) {
		return -1;
	}

	sprintf(buf, "%s ", env_get("bootargs"));
    strcat(cmdline, buf);

	nvt_dbg(FUNC, "boot time: %lu(us)\n", get_nvt_timer0_cnt());
	nvt_tm0_cnt_end = get_nvt_timer0_cnt();
	/* boot time recording */
#ifdef CONFIG_ARM64
	sprintf(buf, "bootts=%lu,%lu print-fatal-signals=1 ", nvt_tm0_cnt_beg, nvt_tm0_cnt_end);
#else
	sprintf(buf, "bootts=%lu,%lu resume user_debug=0xff ", nvt_tm0_cnt_beg, nvt_tm0_cnt_end);
#endif
	strcat(cmdline, buf);
	env_set("bootargs", cmdline);
	nvt_dbg(IND, "bootargs:%s\n", env_get("bootargs"));

	/* To assign relocated fdt address */
	sprintf(buf, "0x%lx ", nvt_memory_cfg[MEMTYPE_LINUX].addr + nvt_memory_cfg[MEMTYPE_LINUX].size);
	env_set("fdt_high", buf);

	/* The following will setup the lmb memory parameters for bootm cmd */
	sprintf(buf, "0x%lx ", nvt_memory_cfg[MEMTYPE_LINUX].addr + nvt_memory_cfg[MEMTYPE_LINUX].size);
	env_set("bootm_size", buf);
	env_set("bootm_mapsize", buf);
	sprintf(buf, "0x%lx ", nvt_memory_cfg[MEMTYPE_LINUX].addr);
	env_set("bootm_low", buf);

	image_print_contents((void *)nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr);
	/*
	 * Issue boot command
	 */
#if defined(NVT_LINUX_RAMDISK_SUPPORT)
	sprintf(buf, "0x%lx ", nvt_ramdisk_addr + nvt_ramdisk_size);
	env_set("initrd_high", buf);
	if (nvt_ramdisk_addr == 0) {
		printf("!Stop boot because your ramdisk address is wrong!!! addr: 0x%lx \n", nvt_ramdisk_addr);
		cli_loop();
	}
	nvt_dbg(FUNC, "boot time: %lu(us)\n", get_nvt_timer0_cnt());
	/************************************************************************************************
	 * ARM64: uImage.lzma => Uimage header + Image(LZMA compression type)
	 * The booti will be responsible for decompress Image, so we need to skip the image_header_t size
	 * ARM32: uImage => Uboot doesn't need to decompress kernel image, the compressed image will be decompressed by Linux stage.
	 ************************************************************************************************/
#ifdef CONFIG_ARM64
	sprintf(cmd, "booti %lx %lx %lx", nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr + sizeof(image_header_t), nvt_ramdisk_addr, (ulong)nvt_fdt_buffer);
#else
	sprintf(cmd, "bootm %lx %lx %lx", nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr, nvt_ramdisk_addr, (ulong)nvt_fdt_buffer);
#endif
	printf("%s\n", cmd);
#else
	printf("%s Linux Image is at %x, uboot fdt image is at %lx, loader tmp fdt address is at %x %s\n", ANSI_COLOR_YELLOW, nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr, (ulong)nvt_fdt_buffer, nvt_readl((ulong)nvt_shminfo_boot_fdt_addr), ANSI_COLOR_RESET);
#ifdef CONFIG_ARM64
	sprintf(cmd, "booti %lx - %lx", nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr + sizeof(image_header_t), (ulong)nvt_fdt_buffer);
#else
	sprintf(cmd, "bootm %lx - %lx", nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr, (ulong)nvt_fdt_buffer);
#endif
	printf("%s\n", cmd);
#endif /* NVT_LINUX_RAMDISK_SUPPORT */
	nvt_dbg(FUNC, "boot time: %lu(us)\n", get_nvt_timer0_cnt());
	nvt_dbg(FUNC, "Uboot boot time: \n\tstart:\t%lu us\n\tending:\t%lu us \r\n", nvt_tm0_cnt_beg, nvt_tm0_cnt_end);
	run_command(cmd, 0);
	return 0;
}

U_BOOT_CMD(
	nvt_boot,   1,  0,  do_nvt_boot_cmd,
	"To do nvt platform boot init.", "\n"
);

#if defined(CONFIG_NVT_LINUX_EMMC_BOOT)
int do_nvt_mmc_erase_all_cmd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int curr_device = -1;
	bool force_init = false;
	char cmd[256] = {0};
	u32 cnt;
	struct mmc *mmc;

	if (argc != 2)
		return CMD_RET_USAGE;

	curr_device = (int)hextoul(argv[1], NULL);

	mmc = find_mmc_device(curr_device);
	if (!mmc) {
		printf("no mmc device at slot %x\n", curr_device);
		return CMD_RET_FAILURE;
	}

	if (!mmc_getcd(mmc))
		force_init = true;

	if (force_init)
		mmc->has_init = 0;

	if (IS_ENABLED(CONFIG_MMC_SPEED_MODE_SET))
		mmc->user_speed_mode = MMC_MODES_END;

	if (mmc_init(mmc))
		return CMD_RET_FAILURE;

#ifdef CONFIG_BLOCK_CACHE
	struct blk_desc *bd = mmc_get_blk_desc(mmc);
	blkcache_invalidate(bd->if_type, bd->devnum);
#endif

	if (IS_SD(mmc) || mmc->version < MMC_VERSION_4_41) {
		printf("Device: %s is SD Card!\n", mmc->cfg->name);
		return CMD_RET_FAILURE;
	}

	printf("Device: %s\n", mmc->cfg->name);
#if CONFIG_IS_ENABLED(MMC_WRITE)
	printf("Erase Group Size: 0x%lx\n", ((u64)mmc->erase_grp_size));
#endif
	sprintf(cmd, "mmc dev %d", curr_device);
	run_command(cmd, 0);

	printf("User Capacity: 0x%lx\n", ((u64)mmc->capacity_user));
	cnt = (u32)(mmc->capacity_user/mmc->erase_grp_size);
	printf("Erase User Partition ...\n");
	sprintf(cmd, "mmc partconf 2 1 1 0");
	printf("%s\n", cmd);
	run_command(cmd, 0);
	sprintf(cmd, "mmc erase 0x0 0x%x", cnt);
	run_command(cmd, 0);

	printf("Boot Capacity: 0x%lx\n", ((u64)mmc->capacity_boot));
	cnt = (u32)(mmc->capacity_boot/mmc->erase_grp_size);
	printf("Erase Boot Partition #0 ...\n");
	sprintf(cmd, "mmc partconf 2 1 1 1");
	printf("%s\n", cmd);
	run_command(cmd, 0);
	sprintf(cmd, "mmc erase 0x0 0x%x", cnt);
	run_command(cmd, 0);

	printf("Erase Boot Partition #1 ...\n");
	sprintf(cmd, "mmc partconf 2 1 1 2");
	printf("%s\n", cmd);
	run_command(cmd, 0);
	sprintf(cmd, "mmc erase 0x0 0x%x", cnt);
	run_command(cmd, 0);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	nvt_mmc_erase_all,   2,  0,  do_nvt_mmc_erase_all_cmd,
	"Erase User Partition and Boot Partition in mmc dev.", "dev\n"
);
#endif /* CONFIG_NVT_LINUX_EMMC_BOOT */






