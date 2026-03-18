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
#include <image.h>
#include <fs.h>
#include <asm/io.h>
//#include <nand.h>
//#include <mmc.h>
//#include <spi_flash.h>
#include <command.h>
//#include <lzma/LzmaTools.h>
//#include <linux/libfdt.h>
#include <asm/nvt-common/nvt_types.h>
//#include <asm/nvt-common/modelext/emb_partition_info.h>
//#include <asm/nvt-common/modelext/bin_info.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/shm_info.h>
#include <asm/nvt-common/nvt_ivot_optee.h>
#include <asm/arch/efuse_protected.h>
#include <asm/nvt-common/nvt_ivot_efuse_smc.h>
#include <asm/arch/crypto.h>
#include <asm/arch/IOAddress.h>
#include <compiler.h>
//#include <u-boot/zlib.h>
//#include <u-boot/crc.h>
#include <stdlib.h>
#include "nvt_ivot_otp_cmd.h"
#include <asm/nvt-common/nvt_ivot_optee.h>
#include <asm/nvt-common/nvt_ivot_aes_smc.h>
#include <malloc.h>

#if defined(CONFIG_TARGET_NS02302_A64)
#include <asm/arch/hardware.h>
#endif

#if defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64) || defined(CONFIG_TARGET_NS02301) || defined(CONFIG_TARGET_NS02302_A64) || defined(CONFIG_TARGET_NS02401_A64) ||defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64) || defined(CONFIG_TARGET_NA51055)
//This is encrypt / decrypt sample code
#define SAMPLE_CODE_KEYMANAGER      0
#define SAMPLE_CODE_CPUFILLKEY      1
#define SAMPLE_CODE_TYPE            SAMPLE_CODE_KEYMANAGER

#define SAMPLE_CODE_ENC_TYPE_EBC    0
#define SAMPLE_CODE_ENC_TYPE_CBC    1
#define SAMPLE_CODE_ENC_TYPE        SAMPLE_CODE_ENC_TYPE_EBC

static UINT8  car_in[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
static UINT8  iv_in[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16};

#if defined(CONFIG_TARGET_NS02401_A64)
#define OTP_KEY_MANAGER(m)  otp_key_manager(EFUSE_ID_0, m)
#else
#define OTP_KEY_MANAGER(m)  otp_key_manager(m)
#endif
#if(SAMPLE_CODE_TYPE == SAMPLE_CODE_CPUFILLKEY)
static UINT8  cpu_key[16] = {0x04, 0x03, 0x02, 0x01, 0x08, 0x07, 0x06, 0x05, 0x12, 0x11, 0x10, 0x09, 0x16, 0x15, 0x14, 0x13};
#endif

int do_nvt_encrypt_cmd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int index_cnt;

	UINT32 test_buf;
	UINT32 test_buf2;

	UINT32 test_buf_align;
	UINT32 test_buf2_align;

	u8  *output;
	u8  *output2;

#if(SAMPLE_CODE_TYPE == SAMPLE_CODE_KEYMANAGER)
	u32      key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
#endif
	CRYPT_OP    crypt_op_param = {0};

	test_buf = (UINT32)(unsigned long)malloc(16 + 64 * 4);
	test_buf_align = (UINT32)((test_buf + 63) & 0xFFFFFFC0);

	test_buf2 = test_buf_align + 100;
	test_buf2_align = (UINT32)((test_buf2 + 63) & 0xFFFFFFC0);

	output = (UINT8 *)(unsigned long)test_buf_align;
	output2 = (UINT8 *)(unsigned long)test_buf2_align;

#if(SAMPLE_CODE_TYPE == SAMPLE_CODE_KEYMANAGER)
	if (strncmp(argv[1], "0", 1) == 0) {
		key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
	} else if (strncmp(argv[1], "1", 1) == 0) {
		key_set = EFUSE_OTP_2ND_KEY_SET_FIELD;
	} else if (strncmp(argv[1], "2", 1) == 0) {
		key_set = EFUSE_OTP_3RD_KEY_SET_FIELD;
	} else if (strncmp(argv[1], "3", 1) == 0) {
		key_set = EFUSE_OTP_4TH_KEY_SET_FIELD;
	} else if (strncmp(argv[1], "4", 1) == 0) {
		key_set = EFUSE_OTP_5TH_KEY_SET_FIELD;
	} else {
		return CMD_RET_USAGE;
	}
#endif

#if (SAMPLE_CODE_ENC_TYPE == SAMPLE_CODE_ENC_TYPE_EBC)
	crypt_op_param.op_mode = CRYPTO_EBC;
#else
	crypt_op_param.op_mode = CRYPTO_CBC;
#endif
	crypt_op_param.en_de_crypt = CRYPTO_ENCRYPT;
	crypt_op_param.src_addr = (UINT32)(unsigned long)car_in;
	crypt_op_param.dst_addr = (UINT32)(unsigned long)output;
	crypt_op_param.length   = 16;
	crypt_op_param.iv       = (UINT32)(unsigned long)iv_in;
#if(SAMPLE_CODE_TYPE == SAMPLE_CODE_KEYMANAGER)
	if (crypto_data_operation(key_set, crypt_op_param) != 0) {
		printf("Encrypt operation fail [%d] set\r\n", (int)(key_set + 1));
#else
	if (crypto_data_operation_by_key(cpu_key, crypt_op_param) != 0) {
		printf("Encrypt operation fail by cpu fill key\r\n");
		for (index_cnt = 0; index_cnt < 16; index_cnt++) {
			printf("%2x ", cpu_key[index_cnt]);
		}
#endif

	} else {
#if(SAMPLE_CODE_TYPE == SAMPLE_CODE_KEYMANAGER)
		printf("Encrypt operation success [%d] set\r\n", (int)(key_set + 1));
#endif
		printf("Source =>\r\n");
		for (index_cnt = 0; index_cnt < 16; index_cnt++) {
			printf("%2x ", car_in[index_cnt]);
		}
		printf("\r\n");
		printf("Destination =>\r\n");
		for (index_cnt = 0; index_cnt < 16; index_cnt++) {
			printf("%2x ", output[index_cnt]);
		}
		printf("\r\n");
	}
#if (SAMPLE_CODE_ENC_TYPE == SAMPLE_CODE_ENC_TYPE_EBC)
	crypt_op_param.op_mode = CRYPTO_EBC;
#else
	crypt_op_param.op_mode = CRYPTO_CBC;
#endif
	crypt_op_param.en_de_crypt = CRYPTO_DECRYPT;
	crypt_op_param.src_addr = (UINT32)(unsigned long)output;
	crypt_op_param.dst_addr = (UINT32)(unsigned long)output2;
	crypt_op_param.length   = 16;
#if(SAMPLE_CODE_TYPE == SAMPLE_CODE_KEYMANAGER)
	if (crypto_data_operation(key_set, crypt_op_param) != 0) {
		printf("Decrypt operation fail [%d] set\r\n", (int)(key_set + 1));
#else
	if (crypto_data_operation_by_key(cpu_key, crypt_op_param) != 0) {
		printf("Decrypt operation fail by cpu fill key\r\n");
		for (index_cnt = 0; index_cnt < 16; index_cnt++) {
			printf("%2x ", cpu_key[index_cnt]);
		}
#endif

	} else {
#if(SAMPLE_CODE_TYPE == SAMPLE_CODE_KEYMANAGER)
		printf("Decrypt operation success [%d] set\r\n", (int)(key_set + 1));
#endif
		for (index_cnt = 0; index_cnt < 16; index_cnt++) {
			printf("%2x ", output2[index_cnt]);
		}
		printf("\r\n");
	}


	printf("do_nvt_encrypt_cmd sample code done\r\n");

	if (test_buf) {
		free((unsigned char *)(unsigned long)test_buf);
	}

	return 0;
}
U_BOOT_CMD(
	nvt_encrypt,    2,  0,  do_nvt_encrypt_cmd,
	"To encrypt via key manager sample",
	"[keyset(0~4)]\n"
);


static UINT8 key_sample[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
static UINT8 encrypt_sample[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
static UINT8 key_sample0[16] = {0x04, 0x03, 0x02, 0x01, 0x08, 0x07, 0x06, 0x05, 0x12, 0x11, 0x10, 0x09, 0x16, 0x15, 0x14, 0x13};

/*
 * PRODUCTION KEYS - DO NOT SHARE OR COMMIT TO PUBLIC REPOSITORIES
 * AES Key: 7f6c177263398ec39aadb38e9652b6a9
 * RSA N Hash (from encrypt_boot tool output):
 *   2nd_key_field: 0x78d08926 0xbd5b7d5b 0xd6923e94 0x3e9198d3
 *   3rd_key_field: 0xff05b545 0xf1521e23 0x87282de7 0xa0a7fc9f
 */

//RSA (RSA's public key N value hash - from encrypt_boot tool output)
#ifdef CONFIG_SIGNATUREMETHOD_ECDSA
//ECDSA sample (ECDSA's public key checksum)
static UINT8 key_sample1[16] = {0x09, 0x1a, 0x21, 0x71, 0xf6, 0xa1, 0x51, 0x41, 0xe7, 0x3f, 0x74, 0x36, 0xe3, 0x49, 0x58, 0x87};
static UINT8 key_sample2[16] = {0x03, 0xa4, 0xed, 0x9e, 0xd3, 0xa7, 0x07, 0x97, 0x26, 0xac, 0x7e, 0x5d, 0x2f, 0x77, 0x23, 0xad};
#else
// RSA N hash from encrypt_boot: 2nd_key_field (key 1 = 2nd key set)
static UINT8 key_sample1[16] = {0x26, 0x89, 0xd0, 0x78, 0x5b, 0x7d, 0x5b, 0xbd, 0x94, 0x3e, 0x92, 0xd6, 0xd3, 0x98, 0x91, 0x3e};
// RSA N hash from encrypt_boot: 3rd_key_field (key 2 = 3rd key set)
static UINT8 key_sample2[16] = {0x45, 0xb5, 0x05, 0xff, 0x23, 0x1e, 0x52, 0xf1, 0xe7, 0x2d, 0x28, 0x87, 0x9f, 0xfc, 0xa7, 0xa0};
#endif
// AES-128 key - for key 0 (1st key set) - data encryption/decryption
static UINT8 key_sample3[16] = {0x7f, 0x6c, 0x17, 0x72, 0x63, 0x39, 0x8e, 0xc3, 0x9a, 0xad, 0xb3, 0x8e, 0x96, 0x52, 0xb6, 0xa9};

int do_nvt_write_key_cmd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	s32         result;
	int         index_cnt;
	UINT8       pKey[16];

	u32         key_set = EFUSE_OTP_2ND_KEY_SET_FIELD;
	CRYPT_OP    crypt_op_param;

	if (strncmp(argv[1], "0", 1) == 0) {
		key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
		memcpy((void *)pKey, (void *)key_sample3, 16);
	} else if (strncmp(argv[1], "1", 1) == 0) {
		key_set = EFUSE_OTP_2ND_KEY_SET_FIELD;

		memcpy((void *)pKey, (void *)key_sample1, 16);
	} else if (strncmp(argv[1], "2", 1) == 0) {
		key_set = EFUSE_OTP_3RD_KEY_SET_FIELD;
		memcpy((void *)pKey, (void *)key_sample2, 16);
	} else if (strncmp(argv[1], "3", 1) == 0) {
		key_set = EFUSE_OTP_4TH_KEY_SET_FIELD;
		memcpy((void *)pKey, (void *)key_sample3, 16);
	} else if (strncmp(argv[1], "4", 1) == 0) {
		key_set = EFUSE_OTP_5TH_KEY_SET_FIELD;
		memcpy((void *)pKey, (void *)key_sample3, 16);
	} else {
		return CMD_RET_USAGE;
	}
	result = otp_write_key(key_set, pKey);

	if (result < 0) {
		printf("Write [%d] key operation fail [%d] \r\n", (int)(key_set), result);
		return 0;
	} else {
		printf("Write [%d] key operation success\r\n", (int)(key_set));
	}
	return 0;
}
U_BOOT_CMD(
	nvt_write_key,  6,  0,  do_nvt_write_key_cmd,
	"To write key via OTP sample",
	"[Option] key set (0~4)=> 0 is dedicate for secure boot\n"
	"[0] => 1st key set (0 is dedicate for secure boot)\n"
	"[1] => 2nd key set\n"
	"[2] => 3rd key set\n"
	"[3] => 4th key set\n"
	"[4] => 5th key set\n"
);

int do_nvt_read_lock_cmd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	u32      key_set_read_lock = SECUREBOOT_1ST_KEY_SET_READ_LOCK;

	if (strncmp(argv[1], "0", 1) == 0) {
		key_set_read_lock = SECUREBOOT_1ST_KEY_SET_READ_LOCK;
	} else if (strncmp(argv[1], "1", 1) == 0) {
		key_set_read_lock = SECUREBOOT_2ND_KEY_SET_READ_LOCK;
	} else if (strncmp(argv[1], "2", 1) == 0) {
		key_set_read_lock = SECUREBOOT_3RD_KEY_SET_READ_LOCK;
	} else if (strncmp(argv[1], "3", 1) == 0) {
		key_set_read_lock = SECUREBOOT_4TH_KEY_SET_READ_LOCK;
	} else if (strncmp(argv[1], "4", 1) == 0) {
		key_set_read_lock = SECUREBOOT_5TH_KEY_SET_READ_LOCK;
	} else {
		return CMD_RET_USAGE;
	}

	enable_secure_boot(key_set_read_lock);
	return 0;
}
U_BOOT_CMD(
	nvt_read_lock,  6,  0,  do_nvt_read_lock_cmd,
	"To let key set X un-readable",
	"[Option] key set (0~4)=> 0 is dedicate for secure boot\n"
	"[0] => 1st key set (0 is dedicate for secure boot)\n"
	"[1] => 2nd key set\n"
	"[2] => 3rd key set\n"
	"[3] => 4th key set\n"
	"[4] => 5th key set\n"
);

static UINT8 read_key_data[16];

int do_nvt_read_key_set_cmd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	u32      key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
	unsigned int field_index = 0;
	unsigned int key_value = 0;
	unsigned int key_field_index = 0;
	int i = 0;
	int ret = 0;

	if (strncmp(argv[1], "0", 1) == 0) {
		key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
	} else if (strncmp(argv[1], "1", 1) == 0) {
		key_set = EFUSE_OTP_2ND_KEY_SET_FIELD;
	} else if (strncmp(argv[1], "2", 1) == 0) {
		key_set = EFUSE_OTP_3RD_KEY_SET_FIELD;
	} else if (strncmp(argv[1], "3", 1) == 0) {
		key_set = EFUSE_OTP_4TH_KEY_SET_FIELD;
	} else if (strncmp(argv[1], "4", 1) == 0) {
		key_set = EFUSE_OTP_5TH_KEY_SET_FIELD;
	} else {
		return CMD_RET_USAGE;
	}

	switch (key_set) {
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
		printf("key field error :%d\n", key_set);
		return -1;
	}
	for (i = 0; i < 4; i++) {
		key_field_index = field_index + i;
		key_value = OTP_KEY_MANAGER(key_field_index);
		printf("\033[0;33mkey[%2d][0x%08x]\033[0m\n", key_field_index, key_value);
		read_key_data[i * 4] = (unsigned char)(key_value & 0x000000ff);
		read_key_data[i * 4 + 1] = (unsigned char)((key_value & 0x0000ff00) >> 8);
		read_key_data[i * 4 + 2] = (unsigned char)((key_value & 0x00ff0000) >> 16);
		read_key_data[i * 4 + 3] = (unsigned char)((key_value & 0xff000000) >> 24);
	}
#if 1
	printf("key value:%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\r\n",
		   read_key_data[0], read_key_data[1], read_key_data[2], read_key_data[3],
		   read_key_data[4], read_key_data[5], read_key_data[6], read_key_data[7],
		   read_key_data[8], read_key_data[9], read_key_data[10], read_key_data[11],
		   read_key_data[12], read_key_data[13], read_key_data[14], read_key_data[15]);
#endif
	return 0;
}
U_BOOT_CMD(
	nvt_read_key_set,  6,  0,  do_nvt_read_key_set_cmd,
	"To read key set X ",
	"[Option] key set (0~4)\n"
	"[0] => 1st key set\n"
	"[1] => 2nd key set\n"
	"[2] => 3rd key set\n"
	"[3] => 4th key set\n"
	"[4] => 5th key set\n"
);

int do_nvt_lock_key_set_engine_access_right_cmd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	u32      key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
	unsigned int field_index = 0;
	unsigned int key_value = 0;
	int i = 0;
	int ret = 0;

	if (strncmp(argv[1], "0", 1) == 0) {
		key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
	} else if (strncmp(argv[1], "1", 1) == 0) {
		key_set = EFUSE_OTP_2ND_KEY_SET_FIELD;
	} else if (strncmp(argv[1], "2", 1) == 0) {
		key_set = EFUSE_OTP_3RD_KEY_SET_FIELD;
	} else if (strncmp(argv[1], "3", 1) == 0) {
		key_set = EFUSE_OTP_4TH_KEY_SET_FIELD;
	} else if (strncmp(argv[1], "4", 1) == 0) {
		key_set = EFUSE_OTP_5TH_KEY_SET_FIELD;
	} else {
		return CMD_RET_USAGE;
	}

	if (otp_set_key_engine_access_right(key_set) >= 0) {
		printf("=>Set key[%d]'s engine key manager access right success\r\n", key_set);
	} else {
		printf("=>Set key[%d]'s engine key manager access right fail\r\n", key_set);
	}
	return 0;
}
U_BOOT_CMD(
	nvt_key_set_engine_right,  6,  0,  do_nvt_lock_key_set_engine_access_right_cmd,
	"To read key set X ",
	"[Option] key set (0~4)\n"
	"[0] => 1st key set\n"
	"[1] => 2nd key set\n"
	"[2] => 3rd key set\n"
	"[3] => 4th key set\n"
	"[4] => 5th key set\n"
);

#if defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64)

#define HDMI_PHYBASE_REG_OFS        0x200
#define HDMI_GETREG(addr)           readl(IOADDR_HDMI_REG_BASE + addr)
#define HDMI2_GETREG(addr)          readl(IOADDR_HDMI2_REG_BASE + addr)
/*
    For TC72670 HDMI PHY Control at FPGA
*/
static UINT32 otp_hdmitx_read_phy(UINT32 ui_offset)
{
	if (ui_offset >= 0x80) {
		return HDMI_GETREG(HDMI_PHYBASE_REG_OFS  + 0x200 + (ui_offset << 2));
	} else {
		return HDMI_GETREG(HDMI_PHYBASE_REG_OFS + (ui_offset << 2));
	}
}

static UINT32 otp_hdmitx2_read_phy(UINT32 ui_offset)
{
	if (ui_offset >= 0x80) {
		return HDMI2_GETREG(HDMI_PHYBASE_REG_OFS  + 0x200 + (ui_offset << 2));
	} else {
		return HDMI2_GETREG(HDMI_PHYBASE_REG_OFS + (ui_offset << 2));
	}
}

#define _u3phy_setreg(base, ofs, val)       writel(val, (base + ofs))
#define _u3phy_getreg(base, ofs)            readl((base + ofs))

#define u3_2phy_setreg(ofs, val)            _u3phy_setreg(IOADDR_USB3_2PHY_REG_BASE + 0x2000, ofs, val)
#define u3_2phy_getreg(ofs)                 _u3phy_getreg(IOADDR_USB3_2PHY_REG_BASE + 0x2000, ofs)

#define u3phy_setreg(ofs, val)              _u3phy_setreg(IOADDR_USB3_PHY_REG_BASE + 0x2000, ofs, val)
#define u3phy_getreg(ofs)                   _u3phy_getreg(IOADDR_USB3_PHY_REG_BASE + 0x2000, ofs)

#define u2phy_setreg(ofs, val)              _u3phy_setreg(IOADDR_USB3_2PHY_REG_BASE + 0x1000, ofs, val)
#define u2phy_getreg(ofs)                   _u3phy_getreg(IOADDR_USB3_2PHY_REG_BASE + 0x1000, ofs)

#define USB3_2PHY_SETREG(ofs, val)          u3_2phy_setreg((ofs<<2), val)
#define USB3_2PHY_GETREG(ofs)               u3_2phy_getreg((ofs<<2))

#define USB3PHY_SETREG(ofs, val)            u3phy_setreg((ofs<<2), val)
#define USB3PHY_GETREG(ofs)                 u3phy_getreg((ofs<<2))

#define USB2PHY_SETREG(ofs, val)            u2phy_setreg((ofs<<2), val)
#define USB2PHY_GETREG(ofs)                 u2phy_getreg((ofs<<2))

#define SATAPHY_SETREG(ofs, val)            writel(val, (IOADDR_SATA0_TOP_REG_BASE + 0x1000 + (ofs<<2)))
#define SATAPHY_GETREG(ofs)                 readl((IOADDR_SATA0_TOP_REG_BASE + 0x1000 + (ofs<<2)))

#define SATA2PHY_SETREG(ofs, val)           writel(val, (IOADDR_USB3_2PHY_REG_BASE + 0x2000 + (ofs<<2)))
#define SATA2PHY_GETREG(ofs)                readl((IOADDR_USB3_2PHY_REG_BASE + 0x2000 + (ofs<<2)))

#define SENPHY_GETREG(ofs)                  readl((IOADDR_SENPHY_REG_BASE + (ofs)))
#define SENPHY2_GETREG(ofs)                 readl((IOADDR_SENPHY2_REG_BASE+ (ofs)))
#endif

int do_nvt_secure_en_cmd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
#if defined(CONFIG_TARGET_NS02302_A64)
	u32 chip_id = nvt_get_chip_id();
#endif
	if (!strncmp(argv[1], "secure_en", 9)) {
		printf("=>[secure_en]\r\n");
		efuse_secure_en();
		printf("is_secure_enable()=%d\r\n", is_secure_enable());
	} else if (!strncmp(argv[1], "data_encrypt", 12)) {
		printf("=>[efuse_data_area_encrypt_en]\r\n");
		efuse_data_area_encrypt_en();
		printf("is_data_area_encrypted()=%d\r\n", is_data_area_encrypted());
	} else if (!strncmp(argv[1], "rsa_en", 6)) {
		printf("=>[efuse_signature_rsa_en]\r\n");
		efuse_signature_rsa_en();
		printf("is_signature_rsa()=%d\r\n", is_signature_rsa());
	} else if (!strncmp(argv[1], "rsapss_en", 9)) {
#if defined(CONFIG_TARGET_NS02302_A64)
		if (nvt_get_chip_id() != CHIP_NS02402) {
			printf("=>[rsa_pss(pkcs2.1) not support]\r\n");
		} else {
			printf("=>[rsa_pss(pkcs2.1) enable]\r\n");
			efuse_signature_rsapss_en();
			printf("is_signature_rsapss_enable()=%d\r\n", is_signature_rsapss());
		}
#elif defined(CONFIG_TARGET_NS02401_A64)
		printf("=>[rsa_pss(pkcs2.1) enable]\r\n");
		efuse_signature_rsapss_en();
		printf("is_signature_rsapss_enable()=%d\r\n", is_signature_rsapss());
#endif
	}  else if (!strncmp(argv[1], "rsapkcs1p5_en", 13)) {
#if defined(CONFIG_TARGET_NS02302_A64)
		if (nvt_get_chip_id() == CHIP_NS02402) {
			printf("=>[rsa_pkcs1.5 enable]\r\n");
			efuse_signature_rsapkcs1p5_en();
			printf("is_signature_rsa_pkcs1.5_enable()=%d\r\n", is_signature_rsapkcs1p5());
		}
#elif defined(CONFIG_TARGET_NS02401_A64)
		printf("=>[rsa_pkcs1.5 enable]\r\n");
		efuse_signature_rsapkcs1p5_en();
		printf("is_signature_rsa_pkcs1.5_enable()=%d\r\n", is_signature_rsapkcs1p5());
#endif
	}  else if (!strncmp(argv[1], "header_signature_en", 19)) {
#if defined(CONFIG_TARGET_NS02302_A64)
		if (nvt_get_chip_id() == CHIP_NS02402) {
			printf("=>[header signature enable]\r\n");
			efuse_header_signature_en();
			printf("is_header_signature_en()=%d\r\n", is_header_signature_en());
		}
#elif defined(CONFIG_TARGET_NS02401_A64)
		printf("=>[header signature enable]\r\n");
		efuse_header_signature_en();
		printf("is_header_signature_en()=%d\r\n", is_header_signature_en());
#endif
	}  else if (!strncmp(argv[1], "ecdsa_en", 8)) {
#if defined(CONFIG_TARGET_NS02302_A64) || defined(CONFIG_TARGET_NS02301) || defined(CONFIG_TARGET_NS02401_A64)
		printf("=>[efuse_signature_ecdsa_en]\r\n");
		efuse_signature_ecdsa_en();
		printf("is_signature_ecdsa()=%d\r\n", is_signature_ecdsa());
#else
		printf("=>[efuse_signature_ecdsa_en] not support\r\n");
#endif
	} else if (!strncmp(argv[1], "rsa_chk_en", 10)) {
		printf("=>[efuse_signature_rsa_chksum_en]\r\n");
		efuse_signature_rsa_chksum_en();
		printf("is_signature_rsa_chsum_enable()=%d\r\n", is_signature_rsa_chsum_enable());
	}  else if (!strncmp(argv[1], "ecdsa_chksum_en", 15)) {
#if defined(CONFIG_TARGET_NS02302_A64) || defined(CONFIG_TARGET_NS02301) || defined(CONFIG_TARGET_NS02401_A64)
		printf("=>[efuse_signature_ecdsa_chksum_en]\r\n");
		efuse_signature_chksum_en();
		printf("efuse_signature_ecdsa_chksum_en()=%d\r\n", is_signature_chsum_enable());
#else
		printf("=>[efuse_signature_ecdsa_chksum_en] not support\r\n");
#endif
	} else if (!strncmp(argv[1], "jtag_disable", 12)) {
		printf("=>[jtag_disable]\r\n");
		efuse_jtag_dis();
		printf("is_JTAG_DISABLE_en()=%d\r\n", is_JTAG_DISABLE_en());
	} else if (!strncmp(argv[1], "uniqueue_id", 11)) {
		UINT32 h, l;
		printf("=>[uniqueue_id]=>");
		if (efuse_get_unique_id(&l, &h) != EFUSE_SUCCESS) {
			printf("unique ID[0x%08x][0x%08x] error\r\n", (int)h, (int)l);
		} else {
			printf("unique ID[0x%08x][0x%08x] success\r\n", (int)h, (int)l);
		}
	} else if (!strncmp(argv[1], "trim", 4)) {
#if defined(CONFIG_TARGET_NA51090_A64)
		u32     trim;
		u32     remap_data;
		int     code;
		BOOL    is_found;
		code = otp_key_manager(OTP_USB_TRIM_FIELD);
		if (code == -33) {
			printf("Read usb trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			printf("[USB] = 0x%08x\r\n", (int)trim);
		}

		code = otp_key_manager(OTP_DRH_TRIM_FIELD);
		if (code == -33) {
			printf("Read DRH trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			printf("[DRH] = 0x%08x\r\n", (int)trim);
		}

		code = otp_key_manager(OTP_DRV_TRIM_FIELD);
		if (code == -33) {
			printf("Read DRV trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			printf("[DRV] = 0x%08x\r\n", (int)trim);
		}

		code = otp_key_manager(OTP_HDMI_SET1_TRIM_FIELD);
		if (code == -33) {
			printf("Read HDMI trim error\r\n");
			return -1;
		} else {
			is_found = extract_2_set_type_trim_valid(TRIM_HDMI_FIELD, code, (u32 *)&trim);
			printf("[HDMI] = 0x%08x\r\n", (int)trim);
		}

		code = otp_key_manager(OTP_PCIe_SET2_TRIM_FIELD);
		if (code == -33) {
			printf("Read PCIe trim error\r\n");
			return -1;
		} else {
			is_found = extract_2_set_type_trim_valid(TRIM_PCIE_REFCLK, code, (u32 *)&trim);
			printf("[PCIe] = 0x%08x\r\n", (int)trim);
		}

		code = otp_key_manager(OTP_THERMAL_TRIM_FIELD);
		if (code == -33) {
			printf("Read thermal trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			printf("[Thermal] = 0x%08x\r\n", (int)trim);
		}

		code = otp_key_manager(OTP_CVBS_VGA_TRIM_FIELD);
		if (code == -33) {
			printf("Read CVBS/VBA trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			printf("[CVBS/VGA] = 0x%08x\r\n", (int)trim);
		}

		code = otp_key_manager(OTP_CORE_POWER_TRIM_FIELD);
		if (code == -33) {
			printf("Read CORE power trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			printf("[Core power trim] = 0x%08x\r\n", (int)trim);
			remap_data = extract_trim_to_pwm_duty(PWM0_CORE_POWER, trim);
			printf("remap_data %d\n", remap_data);
		}
#elif defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64)
		u32     trim = 0;
//		u32     value;
		u32     addr;
		int     code;
		BOOL    is_found;
		u32     HDMI_H_resistor_1_4, HDMI_H_LANE_VL_1_4;
		u32     HDMI_H_resistor_2_0, HDMI_H_LANE_VL_2_0;

		u32     reg_HDMI_H_resistor_1_4, reg_HDMI_H_LANE_VL_1_4;
		u32     reg_HDMI_H_resistor_2_0, reg_HDMI_H_LANE_VL_2_0;

		u32     HDMI_V_resistor_1_4, HDMI_V_LANE_VL_1_4;
		u32     HDMI_V_resistor_2_0, HDMI_V_LANE_VL_2_0;

		u32     reg_HDMI_V_resistor_1_4, reg_HDMI_V_LANE_VL_1_4;
		u32     reg_HDMI_V_resistor_2_0, reg_HDMI_V_LANE_VL_2_0;

		u32     USB3_1_USB2_dual_phy_int_12K_resistor;              //0x2A
		u32     USB3_1_USB2_dual_phy_TX_term;                       //0x2A
		u32     USB3_1_USB2_dual_phy_RX_term;                       //0x2A

		u32     USB2_single_phy_RINT_resistor;                      //0x39
		u32     USB3_2_SATA2_Internal_12K_resistor;                 //0x05
		u32     USB3_2_SATA2_TX_TRIM, USB3_2_SATA2_RX_TRIM;         //0x05

		u32     reg_USB3_2_SATA2_Internal_12K_resistor;             //0x05
		u32     reg_USB3_2_SATA2_TX_TRIM, reg_USB3_2_SATA2_RX_TRIM; //0x05

		u32     SATA1_Internal_12K_resistor;                        //0x00
		u32     SATA1_TX_TRIM, SATA1_RX_TRIM;                       //0x00

		u32     reg_SATA1_Internal_12K_resistor;                    //0x00
		u32     reg_SATA1_TX_TRIM, reg_SATA1_RX_TRIM;               //0x00

		u32     dsi_hs_output_data_swing;
		u32     dsi_hs_output_clk_swing;
		u32     reg_dsi_hs_output_data_swing;
		u32     reg_dsi_hs_output_clk_swing;

		u32     temp, temp1;
		u32     hdmi_reg_09;
		u32     hdmi_reg_30;

		//DSI
		addr = 0x3A;
		code = otp_key_manager(EFUSE_TRIM_DSITXPLL_RG_LEVEL_LDO_PRI);
		if (code == -33) {
			//!!!Please apply default value here!!!
			printf("Read DSI trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				printf("DSI TX output swing(data/clk) Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)trim);
				//bit[3..0]
				dsi_hs_output_data_swing = (trim & 0xF);
				//bit[7..4]
				dsi_hs_output_clk_swing = ((trim >> 4) & 0xF);

				printf(" DSI output data swing Trim data success [0x%04x]\r\n", (int)dsi_hs_output_data_swing);
				printf(" DSI output  clk swing Trim data success [0x%04x]\r\n", (int)dsi_hs_output_clk_swing);
				if (otp_get_pkg_version() == NT98692_PKG) {
					reg_dsi_hs_output_data_swing = ((readl(IOADDR_DSIPHY_REG_BASE + 0x8) >> 8) & 0x8) | ((readl(IOADDR_DSIPHY_REG_BASE + 0x8) >> 16) & 0x7);
					reg_dsi_hs_output_clk_swing  = ((readl(IOADDR_DSIPHY_REG_BASE + 0x8) >> 7) & 0x8) | ((readl(IOADDR_DSIPHY_REG_BASE + 0x8) >> 12) & 0x7);

					if (dsi_hs_output_data_swing == reg_dsi_hs_output_data_swing) {
						printf("DSI HS output data swing Trim [0x%04x] == Reg[0x%04x][O]\r\n", (int)dsi_hs_output_data_swing, (int)reg_dsi_hs_output_data_swing);
					} else {
						printf("DSI HS output data swing Trim [0x%04x] != Reg[0x%04x][X]\r\n", (int)dsi_hs_output_data_swing, (int)reg_dsi_hs_output_data_swing);
					}

					if (dsi_hs_output_clk_swing == reg_dsi_hs_output_clk_swing) {
						printf("DSI HS output clock swing Trim [0x%04x] == Reg[0x%04x][O]\r\n", (int)dsi_hs_output_clk_swing, (int)reg_dsi_hs_output_clk_swing);
					} else {
						printf("DSI HS output clock swing Trim [0x%04x] != Reg[0x%04x][X]\r\n", (int)dsi_hs_output_clk_swing, (int)reg_dsi_hs_output_clk_swing);
					}
				}
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][DSI resistor] = 0x%08x\r\n", is_found, (int)code);
			}
		}
		//USB2
		addr = 0x39;
		code = otp_key_manager(EFUSE_TRIM_USB_LDO_PRI);
		if (code == -33) {
			//!!!Please apply default value here!!!
			printf("Read USB2 RINT trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				USB2_single_phy_RINT_resistor = (trim & 0x1F);

				//USB2 single phy internal resistor 0x1 <= x <= 0x14
				if (USB2_single_phy_RINT_resistor > 0x14) {
					pr_info("USB2 single phy RINT Trim data error [0x%02x]=0x%04x > 0x14\r\n", (int)addr, (int)USB2_single_phy_RINT_resistor);
				} else if (USB2_single_phy_RINT_resistor < 0x1) {
					pr_info("USB2 single phy RINT Trim data error [0x%02x]=0x%04x < 0x1\r\n", (int)addr, (int)USB2_single_phy_RINT_resistor);
				} else {
					pr_info("USB2 single phy RINT trim 0x01 <= data[0x%04x] <= 0x14 success\r\n", (int)USB2_single_phy_RINT_resistor);
				}
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][USB2 resistor] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		//USB3
		addr = 0x2A;
		code = otp_key_manager(EFUSE_TRIM_DATA_USB3_1_USB2_DUAL_PHY_PRI);
		if (code == -33) {
			//!!!Please apply default value here!!!
			printf("Read USB3_1_USB2 dual phy trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				//12K resistor 1<= x <= 20(0x14)
				printf("  USB3_1 Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)trim);
				printf("  12K_resistor[4..0] valid from 1(0x1) <-> 20(0x14)\r\n");
				USB3_1_USB2_dual_phy_int_12K_resistor = trim & 0x1F;
				USB3_1_USB2_dual_phy_TX_term = ((trim >> 5) & 0xF);
				USB3_1_USB2_dual_phy_RX_term = ((trim >> 9) & 0xF);

				if (USB3_1_USB2_dual_phy_int_12K_resistor > 0x14) {
					printf("12K_resistor Trim data error addr[0x%02x] = 0x%04x > 20(0x14)\r\n", (int)addr, (int)USB3_1_USB2_dual_phy_int_12K_resistor);
				} else if (USB3_1_USB2_dual_phy_int_12K_resistor < 0x1) {
					printf("12K_resistor Trim data error addr[0x%02x] = 0x%04x < 1(0x1)\r\n", (int)addr, (int)USB3_1_USB2_dual_phy_int_12K_resistor);
				} else {
					printf("  *12K_resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)USB3_1_USB2_dual_phy_int_12K_resistor);
				}

				//TX term bit[8..5] 0x2 <= x <= 0xE
				if (USB3_1_USB2_dual_phy_TX_term > 0xE) {
					printf("USB(SATA) TX Trim data error addr[0x%02x] = 0x%04x > 0xE\r\n", (int)addr, (int)USB3_1_USB2_dual_phy_TX_term);
				} else if (USB3_1_USB2_dual_phy_TX_term < 0x2) {
					printf("USB(SATA) TX Trim data error addr[0x%02x] = 0x%04x < 0x2\r\n", (int)addr, (int)USB3_1_USB2_dual_phy_TX_term);
				} else {
					printf("  *USB(SATA) TX Trim data range success 0x2 <= [0x%04x] <= 0xE\r\n", (int)USB3_1_USB2_dual_phy_TX_term);
				}
				//RX term bit[12..9] 0x8 <= x <= 0xE
				if (USB3_1_USB2_dual_phy_RX_term > 0xE) {
					printf("USB(SATA) RX Trim data error addr[0x%02x] = 0x%04x > 0xE\r\n", (int)addr, (int)USB3_1_USB2_dual_phy_RX_term);
				} else if (USB3_1_USB2_dual_phy_RX_term < 0x8) {
					printf("USB(SATA) RX Trim data error addr[0x%02x] = 0x%04x < 0x8\r\n", (int)addr, (int)USB3_1_USB2_dual_phy_RX_term);
				} else {
					printf("  *USB(SATA) RX Trim data range success 0x8 <= [0x%04x] <= 0xE\r\n", (int)USB3_1_USB2_dual_phy_RX_term);
				}
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][USB3_1 USB2 dual phy] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		addr = 0x00;
		code = otp_key_manager(EFUSE_TRIM_DATA_SATA1_PRI);
		if (code == -33) {
			//!!!Please apply default value here!!!
			printf("Read SATA1 trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				//12K resistor 1<= x <= 20(0x14)
				printf("  SATA1 Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)trim);
				printf("  SATA1 12K_resistor[4..0] valid from 1(0x1) <-> 20(0x14)\r\n");
				SATA1_Internal_12K_resistor = trim & 0x1F;
				SATA1_TX_TRIM = ((trim >> 5) & 0xF);
				SATA1_RX_TRIM = ((trim >> 9) & 0xF);

				SATAPHY_SETREG(0xFF, 0x00);
				reg_SATA1_Internal_12K_resistor = ((SATAPHY_GETREG(0x1A1) >> 2) & 0x1f);
				temp = SATAPHY_GETREG(0x10);
				reg_SATA1_TX_TRIM = ((temp >> 4) & 0xF);

				//Siwtch to bank 0x2
				SATAPHY_SETREG(0xFF, 0x02);
				temp = SATAPHY_GETREG(0x0);
				reg_SATA1_RX_TRIM = (temp & 0xF);


				if (SATA1_Internal_12K_resistor > 0x14) {
					printf("SATA1 12K_resistor Trim data error [0x%02x]=0x%04x > 20(0x14)\r\n", (int)addr, (int)SATA1_Internal_12K_resistor);
				} else if (SATA1_Internal_12K_resistor < 0x1) {
					printf("SATA1 12K_resistor Trim data error [0x%02x]=0x%04x < 1(0x1)\r\n", (int)addr, (int)SATA1_Internal_12K_resistor);
				} else {
					printf("  *SATA1 12K_resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)SATA1_Internal_12K_resistor);
				}

				//TX term bit[8..5] 0x2 <= x <= 0xE
				if (SATA1_TX_TRIM > 0xE) {
					printf("SATA1 TX Trim data error [0x%02x]=0x%04x > 0xE\r\n", (int)addr, (int)SATA1_TX_TRIM);
				} else if (SATA1_TX_TRIM < 0x2) {
					printf("SATA1 TX Trim data error [0x%02x]=0x%04x < 0x2\r\n", (int)addr, (int)SATA1_TX_TRIM);
				} else {
					printf("  *SATA1 TX Trim data range success 0x2 <= [0x%04x] <= 0xE\r\n", (int)SATA1_TX_TRIM);
				}
				//RX term bit[12..9] 0x8 <= x <= 0xE
				if (SATA1_RX_TRIM > 0xE) {
					printf("SATA1 RX Trim data error [0x%02x]=0x%04x > 0xE\r\n", (int)addr, (int)SATA1_RX_TRIM);
				} else if (SATA1_RX_TRIM < 0x8) {
					printf("SATA1 RX Trim data error [0x%02x]=0x%04x < 0x8\r\n", (int)addr, (int)SATA1_RX_TRIM);
				} else {
					printf("  *SATA1 RX Trim data range success 0x8 <= [0x%04x] <= 0xE\r\n", (int)SATA1_RX_TRIM);
				}

				if (SATA1_Internal_12K_resistor == reg_SATA1_Internal_12K_resistor) {
					printf("sata_trim_rint_sel Trim [0x%04x] == Reg[0x%04x][O]\r\n", (int)SATA1_Internal_12K_resistor, (int)reg_SATA1_Internal_12K_resistor);
				} else {
					printf("sata_trim_rint_sel Trim [0x%04x] != Reg[0x%04x][X]\r\n", (int)SATA1_Internal_12K_resistor, (int)reg_SATA1_Internal_12K_resistor);
				}
				if (SATA1_TX_TRIM == reg_SATA1_TX_TRIM) {
					printf("SATA_TX_TRIM Trim [0x%04x] == Reg[0x%04x][O]\r\n", (int)SATA1_TX_TRIM, (int)reg_SATA1_TX_TRIM);
				} else {
					printf("SATA_TX_TRIM Trim [0x%04x] != Reg[0x%04x][X]\r\n", (int)SATA1_TX_TRIM, (int)reg_SATA1_TX_TRIM);
				}
				if (SATA1_RX_TRIM == reg_SATA1_RX_TRIM) {
					printf("SATA_RX_TRIM Trim [0x%04x] == Reg[0x%04x][O]\r\n", (int)SATA1_RX_TRIM, (int)reg_SATA1_RX_TRIM);
				} else {
					printf("SATA_RX_TRIM Trim [0x%04x] != Reg[0x%04x][X]\r\n", (int)SATA1_RX_TRIM, (int)reg_SATA1_RX_TRIM);
				}
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][SATA1 trim] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		addr = 0x5;
		code = otp_key_manager(EFUSE_TRIM_DATA_USB_SATA_PRI);
		if (code == -33) {
			//!!!Please apply default value here!!!
			printf("Read USB3_2 & SATA2 trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				//12K resistor 1<= x <= 20(0x14)
				USB3_2PHY_SETREG(0xFF, 0x00);
				reg_USB3_2_SATA2_Internal_12K_resistor = USB3_2PHY_GETREG(0x1A0);
				reg_USB3_2_SATA2_Internal_12K_resistor -= 0x40;
				temp = USB3_2PHY_GETREG(0x10);
				reg_USB3_2_SATA2_TX_TRIM = ((temp >> 4) & 0xF);

				//Siwtch to bank 0x2
				USB3_2PHY_SETREG(0xFF, 0x02);
				temp = USB3_2PHY_GETREG(0x0);
				reg_USB3_2_SATA2_RX_TRIM = (temp & 0xF);

				printf("  USB3_2(SATA2) Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)trim);
				printf("  USB3_2(SATA2) 12K_resistor[4..0] valid from 1(0x1) <-> 20(0x14)\r\n");
				USB3_2_SATA2_Internal_12K_resistor = trim & 0x1F;
				USB3_2_SATA2_TX_TRIM = ((trim >> 5) & 0xF);
				USB3_2_SATA2_RX_TRIM = ((trim >> 9) & 0xF);

				if (USB3_2_SATA2_Internal_12K_resistor > 0x14) {
					printf("12K_resistor Trim data error [0x%02x]=0x%04x > 20(0x14)\r\n", (int)addr, (int)USB3_2_SATA2_Internal_12K_resistor);
				} else if (USB3_2_SATA2_Internal_12K_resistor < 0x1) {
					printf("12K_resistor Trim data error [0x%02x]=0x%04x < 1(0x1)\r\n", (int)addr, (int)USB3_2_SATA2_Internal_12K_resistor);
				} else {
					printf("  *12K_resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)USB3_2_SATA2_Internal_12K_resistor);
				}

				//TX term bit[8..5] 0x2 <= x <= 0xE
				if (USB3_2_SATA2_TX_TRIM > 0xE) {
					printf("USB(SATA) TX Trim data error [0x%02x]=0x%04x > 0xE\r\n", (int)addr, (int)USB3_2_SATA2_TX_TRIM);
				} else if (USB3_2_SATA2_TX_TRIM < 0x2) {
					printf("USB(SATA) TX Trim data error [0x%02x]=0x%04x < 0x2\r\n", (int)addr, (int)USB3_2_SATA2_TX_TRIM);
				} else {
					printf("  *USB(SATA) TX Trim data range success 0x2 <= [0x%04x] <= 0xE\r\n", (int)USB3_2_SATA2_TX_TRIM);
				}
				//RX term bit[12..9] 0x8 <= x <= 0xE
				if (USB3_2_SATA2_RX_TRIM > 0xE) {
					printf("USB(SATA) RX Trim data error [0x%02x]=0x%04x > 0xE\r\n", (int)addr, (int)USB3_2_SATA2_RX_TRIM);
				} else if (USB3_2_SATA2_RX_TRIM < 0x8) {
					printf("USB(SATA) RX Trim data error [0x%02x]=0x%04x < 0x8\r\n", (int)addr, (int)USB3_2_SATA2_RX_TRIM);
				} else {
					printf("  *USB(SATA) RX Trim data range success 0x8 <= [0x%04x] <= 0xE\r\n", (int)USB3_2_SATA2_RX_TRIM);
				}

				if (USB3_2_SATA2_Internal_12K_resistor == reg_USB3_2_SATA2_Internal_12K_resistor) {
					printf("   u3_trim_rint_sel Trim [0x%04x] == Reg[0x%04x][O]\r\n", (int)USB3_2_SATA2_Internal_12K_resistor, (int)reg_USB3_2_SATA2_Internal_12K_resistor);
				} else {
					printf("   u3_trim_rint_sel Trim [0x%04x] != Reg[0x%04x][X]\r\n", (int)USB3_2_SATA2_Internal_12K_resistor, (int)reg_USB3_2_SATA2_Internal_12K_resistor);
				}
				if (USB3_2_SATA2_TX_TRIM == reg_USB3_2_SATA2_TX_TRIM) {
					printf("   USB_SATA_TX_TRIM Trim [0x%04x] == Reg[0x%04x][O]\r\n", (int)USB3_2_SATA2_TX_TRIM, (int)reg_USB3_2_SATA2_TX_TRIM);
				} else {
					printf("   USB_SATA_TX_TRIM Trim [0x%04x] != Reg[0x%04x][X]\r\n", (int)USB3_2_SATA2_TX_TRIM, (int)reg_USB3_2_SATA2_TX_TRIM);
				}
				if (USB3_2_SATA2_RX_TRIM == reg_USB3_2_SATA2_RX_TRIM) {
					printf("   USB_SATA_RX_TRIM Trim [0x%04x] == Reg[0x%04x][O]\r\n", (int)USB3_2_SATA2_RX_TRIM, (int)reg_USB3_2_SATA2_RX_TRIM);
				} else {
					printf("   USB_SATA_RX_TRIM Trim [0x%04x] != Reg[0x%04x][X]\r\n", (int)USB3_2_SATA2_RX_TRIM, (int)reg_USB3_2_SATA2_RX_TRIM);
				}
				//SATA2
				//Siwtch to bank 0x0
				SATA2PHY_SETREG(0xFF, 0x00);
				reg_USB3_2_SATA2_Internal_12K_resistor = ((SATA2PHY_GETREG(0x1A1) >> 2) & 0x1f);
				temp = SATA2PHY_GETREG(0x10);
				reg_USB3_2_SATA2_TX_TRIM = ((temp >> 4) & 0xF);

				//Siwtch to bank 0x2
				SATA2PHY_SETREG(0xFF, 0x02);
				temp = SATA2PHY_GETREG(0x0);
				reg_USB3_2_SATA2_RX_TRIM = (temp & 0xF);

				if (USB3_2_SATA2_Internal_12K_resistor == reg_USB3_2_SATA2_Internal_12K_resistor) {
					printf("   [SATA2]u3_trim_rint_sel Trim [0x%04x] == Reg[0x%04x][O]\r\n", (int)USB3_2_SATA2_Internal_12K_resistor, (int)reg_USB3_2_SATA2_Internal_12K_resistor);
				} else {
					printf("   [SATA2]u3_trim_rint_sel Trim [0x%04x] != Reg[0x%04x][X]\r\n", (int)USB3_2_SATA2_Internal_12K_resistor, (int)reg_USB3_2_SATA2_Internal_12K_resistor);
				}
				if (USB3_2_SATA2_TX_TRIM == reg_USB3_2_SATA2_TX_TRIM) {
					printf("   [SATA2]USB_SATA_TX_TRIM Trim [0x%04x] == Reg[0x%04x][O]\r\n", (int)USB3_2_SATA2_TX_TRIM, (int)reg_USB3_2_SATA2_TX_TRIM);
				} else {
					printf("   [SATA2]USB_SATA_TX_TRIM Trim [0x%04x] != Reg[0x%04x][X]\r\n", (int)USB3_2_SATA2_TX_TRIM, (int)reg_USB3_2_SATA2_TX_TRIM);
				}
				if (USB3_2_SATA2_RX_TRIM == reg_USB3_2_SATA2_RX_TRIM) {
					printf("   [SATA2]USB_SATA_RX_TRIM Trim [0x%04x] == Reg[0x%04x][O]\r\n", (int)USB3_2_SATA2_RX_TRIM, (int)reg_USB3_2_SATA2_RX_TRIM);
				} else {
					printf("   [SATA2]USB_SATA_RX_TRIM Trim [0x%04x] != Reg[0x%04x][X]\r\n", (int)USB3_2_SATA2_RX_TRIM, (int)reg_USB3_2_SATA2_RX_TRIM);
				}
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][USB3_2 & SATA2 trim] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		//0x3
		addr = EFUSE_TRIM_DATA_HDMI_H_PHY_RESISTOR_PRI;
		code = otp_key_manager(EFUSE_TRIM_DATA_HDMI_H_PHY_RESISTOR_PRI);
		if (code == -33) {
			//!!!Please apply default value here!!!
			printf("Read HDMI H PHY Resistor trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				HDMI_H_resistor_1_4 = trim & 0x1F;
				//bit[9..5]
				HDMI_H_resistor_2_0 = ((trim >> 5) & 0x1F);

				if (HDMI_H_resistor_1_4 > 0x14) {
					printf("(H)1.4 HDMI resistor Trim data error [0x%02x] = 0x%04x > 0x14\r\n", (int)addr, (int)HDMI_H_resistor_1_4);
				} else if (HDMI_H_resistor_1_4 < 0x1) {
					printf("(H)1.4 HDMI resistor Trim data error [0x%02x]=0x%04x < 0x1\r\n", (int)addr, (int)HDMI_H_resistor_1_4);
				} else {
					printf(" *(H)1.4 HDMI resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)HDMI_H_resistor_1_4);
				}

				if (HDMI_H_resistor_2_0 > 0x14) {
					printf("(H)2.0 HDMI resistor Trim data error [0x%02x]=0x%04x > 0x14\r\n", (int)addr, (int)HDMI_H_resistor_2_0);
				} else if (HDMI_H_resistor_2_0 < 0x1) {
					printf("(H)2.0 HDMI resistor Trim data error [0x%02x]=0x%04x < 0x1\r\n", (int)addr, (int)HDMI_H_resistor_2_0);
				} else {
					printf(" *(H)2.0 HDMI resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)HDMI_H_resistor_2_0);
				}

				if (otp_get_pkg_version() == NT98692_PKG) {
					hdmi_reg_09 = otp_hdmitx_read_phy(0x9);
					hdmi_reg_30 = otp_hdmitx_read_phy(0x30);
					if (hdmi_reg_09 & 0x1) {
						if (hdmi_reg_30 == 0x0) {
							reg_HDMI_H_resistor_1_4 = (otp_hdmitx_read_phy(0xc) & 0x1F);
							if (HDMI_H_resistor_1_4 == reg_HDMI_H_resistor_1_4) {
								printf("HDMI enable as 1.4\r\n");
								printf("HDMI_resistor_1_4[0x%04x] == [0x%04x][O]\r\n", HDMI_H_resistor_1_4, reg_HDMI_H_resistor_1_4);
							} else {
								printf("HDMI enable as 1.4 => make sure HDMI is probed\r\n");
								printf("HDMI_resistor_1_4[0x%04x] != [0x%04x][X]\r\n", HDMI_H_resistor_1_4, reg_HDMI_H_resistor_1_4);
							}
						} else if (hdmi_reg_30 == 0x6) {
							reg_HDMI_H_resistor_2_0 = (otp_hdmitx_read_phy(0xc) & 0x1F);
							if (HDMI_H_resistor_2_0 == reg_HDMI_H_resistor_2_0) {
								printf("HDMI enable as 2.0\r\n");
								printf("HDMI_resistor_2_0[0x%04x] == [0x%04x][O]\r\n", HDMI_H_resistor_2_0, reg_HDMI_H_resistor_2_0);
							} else {
								printf("HDMI enable as 2.0 => make sure HDMI is probed\r\n");
								printf("HDMI_resistor_2_0[0x%04x] != [0x%04x][X]\r\n", HDMI_H_resistor_2_0, reg_HDMI_H_resistor_2_0);
							}
						} else {
							printf("Unknow HDMI speed !!!\r\n");
						}
					}
				} else {
					printf("Not 692 do not compare\r\n");
				}
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][HDMI H Resistor] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		addr = 0x1;
		code = otp_key_manager(EFUSE_TRIM_DATA_HDMI_2_0_H_PHY_VL_TRIM_PRI);
		if (code == -33) {
			//!!!Please apply default value here!!!
			printf("Read HDMI H PHY 2.0 VLANE trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				printf("  (H)HDMI 2.0 VLane tunning Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)trim);
				//bit[11..0]
				HDMI_H_LANE_VL_2_0 = trim & 0xFFF;
				printf("  *(H)2.0 HDMI VLane Trim data success [0x%04x](no limitation)\r\n", (int)HDMI_H_LANE_VL_2_0);

				if (otp_get_pkg_version() == NT98692_PKG) {
					hdmi_reg_09 = otp_hdmitx_read_phy(0x9);
					hdmi_reg_30 = otp_hdmitx_read_phy(0x30);
					//bit[11..0]
					//where
					//b[ 2..0]@efuse == reg_sw_fine_ctl_0[2:0]= VL trim CH0(fine-tune)->REG_0D[2:0]@0x0D
					//b[ 5..3]@efuse == reg_sw_fine_ctl_1[2:0]= VL trim CH1(fine-tune)->REG_0D[6:4]@0x0D
					//b[ 8..6]@efuse == reg_sw_fine_ctl_2[2:0]= VL trim CH2(fine-tune)->REG_0E[2:0]@0x0E
					//b[11..9]@efuse == reg_sw_fine_ctl_3[2:0]= VL trim CH3(fine-tune)->REG_0E[6:4]@0x0E
					if (hdmi_reg_09 & 0x1) {
						if (hdmi_reg_30 == 0x0) {
							printf("Now is = HDMI 1.4 and trim is HDMI2.0's trim(ignore)!\r\n");
						} else if (hdmi_reg_30 == 0x6) {
							temp = otp_hdmitx_read_phy(0x0D);
							temp1 = otp_hdmitx_read_phy(0x0E);

							if ((HDMI_H_LANE_VL_2_0 & 0x7) == (temp & 0x7)) {
								printf("reg_sw_fine_ctl_0[2:0]=>Trim[0x%04x] == register[0x%04x][O]\r\n", (HDMI_H_LANE_VL_2_0 & 0x7), (temp & 0x7));
							} else {
								printf("reg_sw_fine_ctl_0[2:0]=>Trim[0x%04x] != register[0x%04x][X]\r\n", (HDMI_H_LANE_VL_2_0 & 0x7), (temp & 0x7));
							}
							if (((HDMI_H_LANE_VL_2_0 & (0x7 << 3)) >> 3) == ((temp & (0x7 << 4)) >> 4)) {
								printf("reg_sw_fine_ctl_1[2:0]=>Trim[0x%04x] == register[0x%04x][O]\r\n", ((HDMI_H_LANE_VL_2_0 & (0x7 << 3)) >> 3), ((temp & (0x7 << 4)) >> 4));
							} else {
								printf("reg_sw_fine_ctl_1[2:0]=>Trim[0x%04x] != register[0x%04x][X]\r\n", ((HDMI_H_LANE_VL_2_0 & (0x7 << 3)) >> 3), ((temp & (0x7 << 4)) >> 4));
							}

							if (((HDMI_H_LANE_VL_2_0 & (0x7 << 6)) >> 6) == (temp1 & 0x7)) {
								printf("reg_sw_fine_ctl_2[2:0]=>Trim[0x%04x] == register[0x%04x][O]\r\n", ((HDMI_H_LANE_VL_2_0 & (0x7 << 6)) >> 6), (temp1 & 0x7));
							} else {
								printf("reg_sw_fine_ctl_2[2:0]=>Trim[0x%04x] != register[0x%04x][X]\r\n", ((HDMI_H_LANE_VL_2_0 & (0x7 << 6)) >> 6), (temp1 & 0x7));
							}
							if (((HDMI_H_LANE_VL_2_0 & (0x7 << 9)) >> 9) == ((temp1 & (0x7 << 4)) >> 4)) {
								printf("reg_sw_fine_ctl_3[2:0]=>Trim[0x%04x] == register[0x%04x][O]\r\n", ((HDMI_H_LANE_VL_2_0 & (0x7 << 9)) >> 9), ((temp1 & (0x7 << 4)) >> 4));
							} else {
								printf("reg_sw_fine_ctl_3[2:0]=>Trim[0x%04x] != register[0x%04x][X]\r\n", ((HDMI_H_LANE_VL_2_0 & (0x7 << 9)) >> 9), ((temp1 & (0x7 << 4)) >> 4));
							}
						} else {
							printf("Unknow HDMI speed !!!\r\n");
						}
					} else {
						printf("!!!HDMI not enabled => make sure HDMI is probed\r\n");
						printf("2.0 HDMI VLane Trim data = 0x%04x\r\n", (int)HDMI_H_LANE_VL_2_0);
					}
				}
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][HDMI H PHY 2.0 VLANE] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		addr = 0x2F;
		code = otp_key_manager(EFUSE_TRIM_DATA_HDMI_1_4_H_PHY_VL_TRIM_PRI);
		if (code == -33) {
			//!!!Please apply default value here!!!
			printf("Read HDMI H PHY 1.4 VLANE trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				printf("  (H)HDMI 1.4 VLane tunning Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)trim);
				//bit[11..0]
				HDMI_H_LANE_VL_1_4 = trim & 0xFFF;
				printf("  *(H)1.4 HDMI VLane Trim data success [0x%04x](no limitation)\r\n", (int)HDMI_H_LANE_VL_1_4);

				if (otp_get_pkg_version() == NT98692_PKG) {
					hdmi_reg_09 = otp_hdmitx_read_phy(0x9);
					hdmi_reg_30 = otp_hdmitx_read_phy(0x30);
					//bit[11..0]
					HDMI_H_LANE_VL_1_4 = trim & 0xFFF;
					if (hdmi_reg_09 & 0x1) {
						if (hdmi_reg_30 == 0x0) {
							temp = otp_hdmitx_read_phy(0x0D);
							temp1 = otp_hdmitx_read_phy(0x0E);
							if ((HDMI_H_LANE_VL_1_4 & 0x7) == (temp & 0x7)) {
								printf("reg_sw_fine_ctl_0[2:0]=>Trim[0x%04x] == register[0x%04x][O]\r\n", (HDMI_H_LANE_VL_1_4 & 0x7), (temp & 0x7));
							} else {
								printf("reg_sw_fine_ctl_0[2:0]=>Trim[0x%04x] != register[0x%04x][X]\r\n", (HDMI_H_LANE_VL_1_4 & 0x7), (temp & 0x7));
							}
							if (((HDMI_H_LANE_VL_1_4 & (0x7 << 3)) >> 3) == ((temp & (0x7 << 4)) >> 4)) {
								printf("reg_sw_fine_ctl_1[2:0]=>Trim[0x%04x] == register[0x%04x][O]\r\n", ((HDMI_H_LANE_VL_1_4 & (0x7 << 3)) >> 3), ((temp & (0x7 << 4)) >> 4));
							} else {
								printf("reg_sw_fine_ctl_1[2:0]=>Trim[0x%04x] != register[0x%04x][X]\r\n", ((HDMI_H_LANE_VL_1_4 & (0x7 << 3)) >> 3), ((temp & (0x7 << 4)) >> 4));
							}

							if (((HDMI_H_LANE_VL_1_4 & (0x7 << 6)) >> 6) == (temp1 & 0x7)) {
								printf("reg_sw_fine_ctl_2[2:0]=>Trim[0x%04x] == register[0x%04x][O]\r\n", ((HDMI_H_LANE_VL_1_4 & (0x7 << 6)) >> 6), (temp1 & 0x7));
							} else {
								printf("reg_sw_fine_ctl_2[2:0]=>Trim[0x%04x] != register[0x%04x][X]\r\n", ((HDMI_H_LANE_VL_1_4 & (0x7 << 6)) >> 6), (temp1 & 0x7));
							}
							if (((HDMI_H_LANE_VL_1_4 & (0x7 << 9)) >> 9) == ((temp1 & (0x7 << 4)) >> 4)) {
								printf("reg_sw_fine_ctl_3[2:0]=>Trim[0x%04x] == register[0x%04x][O]\r\n", ((HDMI_H_LANE_VL_1_4 & (0x7 << 9)) >> 9), ((temp1 & (0x7 << 4)) >> 4));
							} else {
								printf("reg_sw_fine_ctl_3[2:0]=>Trim[0x%04x] != register[0x%04x][X]\r\n", ((HDMI_H_LANE_VL_1_4 & (0x7 << 9)) >> 9), ((temp1 & (0x7 << 4)) >> 4));
							}
						} else if (hdmi_reg_30 == 0x6) {
							printf("Now is = HDMI 2.0 and trim is HDMI1.4's trim(ignore)!\r\n");
						} else {
							printf("Unknow HDMI speed !!!\r\n");
						}
					} else {
						printf("!!!HDMI not enabled => make sure HDMI is probed\r\n");
						printf("2.0 HDMI VLane Trim data = 0x%04x\r\n", (int)HDMI_H_LANE_VL_1_4);
					}
				} else {
					printf("%d no HDMI(ignore)!\r\n", otp_get_pkg_version());
				}
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][HDMI H PHY 1.4 VLANE] = 0x%08x\r\n", is_found, (int)code);
			}
		}
		//V
		addr = 0x2E;
		code = otp_key_manager(EFUSE_TRIM_DATA_HDMI_V_PHY_RESISTOR_PRI);
		if (code == -33) {
			//!!!Please apply default value here!!!
			printf("Read HDMI V PHY Resistor trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				HDMI_V_resistor_1_4 = trim & 0x1F;
				//bit[9..5]
				HDMI_V_resistor_2_0 = ((trim >> 5) & 0x1F);

				if (HDMI_V_resistor_1_4 > 0x14) {
					printf("(V)1.4 HDMI resistor Trim data error [0x%02x] = 0x%04x > 0x14\r\n", (int)addr, (int)HDMI_V_resistor_1_4);
				} else if (HDMI_V_resistor_1_4 < 0x1) {
					printf("(V)1.4 HDMI resistor Trim data error [0x%02x]=0x%04x < 0x1\r\n", (int)addr, (int)HDMI_V_resistor_1_4);
				} else {
					printf(" *(V)1.4 HDMI resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)HDMI_V_resistor_1_4);
				}

				if (HDMI_V_resistor_2_0 > 0x14) {
					printf("(V)2.0 HDMI resistor Trim data error [0x%02x]=0x%04x > 0x14\r\n", (int)addr, (int)HDMI_V_resistor_2_0);
				} else if (HDMI_V_resistor_2_0 < 0x1) {
					printf("(V)2.0 HDMI resistor Trim data error [0x%02x]=0x%04x < 0x1\r\n", (int)addr, (int)HDMI_V_resistor_2_0);
				} else {
					printf(" *(V)2.0 HDMI resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)HDMI_V_resistor_2_0);
				}

				hdmi_reg_09 = otp_hdmitx2_read_phy(0x9);
				hdmi_reg_30 = otp_hdmitx2_read_phy(0x30);
				if (hdmi_reg_09 & 0x1) {
					if (hdmi_reg_30 == 0x0) {
						reg_HDMI_V_resistor_1_4 = (otp_hdmitx2_read_phy(0xc) & 0x1F);
						if (HDMI_V_resistor_1_4 == reg_HDMI_V_resistor_1_4) {
							printf("HDMI2 enable as 1.4\r\n");
							printf("HDMI_resistor_1_4[0x%04x] == [0x%04x][O]\r\n", HDMI_V_resistor_1_4, reg_HDMI_V_resistor_1_4);
						} else {
							printf("HDMI enable as 1.4 => make sure HDMI2 is probed\r\n");
							printf("HDMI_resistor_1_4[0x%04x] != [0x%04x][X]\r\n", HDMI_V_resistor_1_4, reg_HDMI_V_resistor_1_4);
						}
					} else if (hdmi_reg_30 == 0x6) {
						reg_HDMI_V_resistor_2_0 = (otp_hdmitx2_read_phy(0xc) & 0x1F);
						if (HDMI_V_resistor_2_0 == reg_HDMI_V_resistor_2_0) {
							printf("HDMI2 enable as 2.0\r\n");
							printf("HDMI_resistor_2_0[0x%04x] == [0x%04x][O]\r\n", HDMI_V_resistor_2_0, reg_HDMI_V_resistor_2_0);
						} else {
							printf("HDM2 enable as 2.0 => make sure HDMI2 is probed\r\n");
							printf("HDMI_resistor_2_0[0x%04x] != [0x%04x][X]\r\n", HDMI_V_resistor_2_0, reg_HDMI_V_resistor_2_0);
						}
					} else {
						printf("Unknow HDMI2 speed !!!\r\n");
					}
				} else {
					printf("!!!HDMI2 not enabled => make sure HDMI2 is probed\r\n");
					printf("1.4 HDMI2 resistor Trim data = 0x%04x\r\n", (int)HDMI_V_resistor_1_4);
					printf("2.0 HDMI2 resistor Trim data = 0x%04x\r\n", (int)HDMI_V_resistor_2_0);
				}
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][HDMI VH Resistor] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		addr = 0x2C;
		code = otp_key_manager(EFUSE_TRIM_DATA_HDMI_2_0_V_PHY_VL_TRIM_PRI);
		if (code == -33) {
			//!!!Please apply default value here!!!
			printf("Read HDMI V PHY 2.0 VLANE trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				printf("  (V)HDMI 2.0 VLane tunning Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)trim);
				//bit[11..0]
				HDMI_V_LANE_VL_2_0 = trim & 0xFFF;
				printf("  *(V)2.0 HDMI VLane Trim data success [0x%04x](no limitation)\r\n", (int)HDMI_V_LANE_VL_2_0);

				hdmi_reg_09 = otp_hdmitx2_read_phy(0x9);
				hdmi_reg_30 = otp_hdmitx2_read_phy(0x30);
				//bit[11..0]
				//where
				//b[ 2..0]@efuse == reg_sw_fine_ctl_0[2:0]= VL trim CH0(fine-tune)->REG_0D[2:0]@0x0D
				//b[ 5..3]@efuse == reg_sw_fine_ctl_1[2:0]= VL trim CH1(fine-tune)->REG_0D[6:4]@0x0D
				//b[ 8..6]@efuse == reg_sw_fine_ctl_2[2:0]= VL trim CH2(fine-tune)->REG_0E[2:0]@0x0E
				//b[11..9]@efuse == reg_sw_fine_ctl_3[2:0]= VL trim CH3(fine-tune)->REG_0E[6:4]@0x0E
				if (hdmi_reg_09 & 0x1) {
					if (hdmi_reg_30 == 0x0) {
						printf("Now is = HDMI2 1.4 and trim is HDMI2.0's trim(ignore)!\r\n");
					} else if (hdmi_reg_30 == 0x6) {
						temp = otp_hdmitx2_read_phy(0x0D);
						temp1 = otp_hdmitx2_read_phy(0x0E);
						if ((HDMI_V_LANE_VL_2_0 & 0x7) == (temp & 0x7)) {
							printf("reg_sw_fine_ctl_0[2:0]=>Trim[0x%04x] == register[0x%04x][O]\r\n", (HDMI_V_LANE_VL_2_0 & 0x7), (temp & 0x7));
						} else {
							printf("reg_sw_fine_ctl_0[2:0]=>Trim[0x%04x] != register[0x%04x][X]\r\n", (HDMI_V_LANE_VL_2_0 & 0x7), (temp & 0x7));
						}
						if (((HDMI_V_LANE_VL_2_0 & (0x7 << 3)) >> 3) == ((temp & (0x7 << 4)) >> 4)) {
							printf("reg_sw_fine_ctl_1[2:0]=>Trim[0x%04x] == register[0x%04x][O]\r\n", ((HDMI_V_LANE_VL_2_0 & (0x7 << 3)) >> 3), ((temp & (0x7 << 4)) >> 4));
						} else {
							printf("reg_sw_fine_ctl_1[2:0]=>Trim[0x%04x] != register[0x%04x][X]\r\n", ((HDMI_V_LANE_VL_2_0 & (0x7 << 3)) >> 3), ((temp & (0x7 << 4)) >> 4));
						}

						if (((HDMI_V_LANE_VL_2_0 & (0x7 << 6)) >> 6) == (temp1 & 0x7)) {
							printf("reg_sw_fine_ctl_2[2:0]=>Trim[0x%04x] == register[0x%04x][O]\r\n", ((HDMI_V_LANE_VL_2_0 & (0x7 << 6)) >> 6), (temp1 & 0x7));
						} else {
							printf("reg_sw_fine_ctl_2[2:0]=>Trim[0x%04x] != register[0x%04x][X]\r\n", ((HDMI_V_LANE_VL_2_0 & (0x7 << 6)) >> 6), (temp1 & 0x7));
						}
						if (((HDMI_V_LANE_VL_2_0 & (0x7 << 9)) >> 9) == ((temp1 & (0x7 << 4)) >> 4)) {
							printf("reg_sw_fine_ctl_3[2:0]=>Trim[0x%04x] == register[0x%04x][O]\r\n", ((HDMI_V_LANE_VL_2_0 & (0x7 << 9)) >> 9), ((temp1 & (0x7 << 4)) >> 4));
						} else {
							printf("reg_sw_fine_ctl_3[2:0]=>Trim[0x%04x] != register[0x%04x][X]\r\n", ((HDMI_V_LANE_VL_2_0 & (0x7 << 9)) >> 9), ((temp1 & (0x7 << 4)) >> 4));
						}
					} else {
						printf("Unknow HDMI2 speed !!!\r\n");
					}
				} else {
					printf("!!!HDMI2 not enabled => make sure HDMI2 is probed\r\n");
					printf("2.0 HDMI2 VLane Trim data = 0x%04x\r\n", (int)HDMI_V_LANE_VL_2_0);
				}
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][HDMI V PHY 2.0 VLANE] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		addr = 0x2D;
		code = otp_key_manager(EFUSE_TRIM_DATA_HDMI_1_4_V_PHY_VL_TRIM_PRI);
		if (code == -33) {
			//!!!Please apply default value here!!!
			printf("Read HDMI V PHY 1.4 VLANE trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				printf("  (V)HDMI 1.4 VLane tunning Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)trim);
				//bit[11..0]
				HDMI_H_LANE_VL_1_4 = trim & 0xFFF;
				printf("  *(V)1.4 HDMI VLane Trim data success [0x%04x](no limitation)\r\n", (int)HDMI_H_LANE_VL_1_4);

				hdmi_reg_09 = otp_hdmitx2_read_phy(0x9);
				hdmi_reg_30 = otp_hdmitx2_read_phy(0x30);
				//bit[11..0]
				HDMI_H_LANE_VL_1_4 = trim & 0xFFF;
				if (hdmi_reg_09 & 0x1) {
					if (hdmi_reg_30 == 0x0) {
						temp = otp_hdmitx2_read_phy(0x0D);
						temp1 = otp_hdmitx2_read_phy(0x0E);
						if ((HDMI_H_LANE_VL_1_4 & 0x7) == (temp & 0x7)) {
							printf("reg_sw_fine_ctl_0[2:0]=>Trim[0x%04x] == register[0x%04x][O]\r\n", (HDMI_H_LANE_VL_1_4 & 0x7), (temp & 0x7));
						} else {
							printf("reg_sw_fine_ctl_0[2:0]=>Trim[0x%04x] != register[0x%04x][X]\r\n", (HDMI_H_LANE_VL_1_4 & 0x7), (temp & 0x7));
						}
						if (((HDMI_H_LANE_VL_1_4 & (0x7 << 3)) >> 3) == ((temp & (0x7 << 4)) >> 4)) {
							printf("reg_sw_fine_ctl_1[2:0]=>Trim[0x%04x] == register[0x%04x][O]\r\n", ((HDMI_H_LANE_VL_1_4 & (0x7 << 3)) >> 3), ((temp & (0x7 << 4)) >> 4));
						} else {
							printf("reg_sw_fine_ctl_1[2:0]=>Trim[0x%04x] != register[0x%04x][X]\r\n", ((HDMI_H_LANE_VL_1_4 & (0x7 << 3)) >> 3), ((temp & (0x7 << 4)) >> 4));
						}

						if (((HDMI_H_LANE_VL_1_4 & (0x7 << 6)) >> 6) == (temp1 & 0x7)) {
							printf("reg_sw_fine_ctl_2[2:0]=>Trim[0x%04x] == register[0x%04x][O]\r\n", ((HDMI_H_LANE_VL_1_4 & (0x7 << 6)) >> 6), (temp1 & 0x7));
						} else {
							printf("reg_sw_fine_ctl_2[2:0]=>Trim[0x%04x] != register[0x%04x][X]\r\n", ((HDMI_H_LANE_VL_1_4 & (0x7 << 6)) >> 6), (temp1 & 0x7));
						}
						if (((HDMI_H_LANE_VL_1_4 & (0x7 << 9)) >> 9) == ((temp1 & (0x7 << 4)) >> 4)) {
							printf("reg_sw_fine_ctl_3[2:0]=>Trim[0x%04x] == register[0x%04x][O]\r\n", ((HDMI_H_LANE_VL_1_4 & (0x7 << 9)) >> 9), ((temp1 & (0x7 << 4)) >> 4));
						} else {
							printf("reg_sw_fine_ctl_3[2:0]=>Trim[0x%04x] != register[0x%04x][X]\r\n", ((HDMI_H_LANE_VL_1_4 & (0x7 << 9)) >> 9), ((temp1 & (0x7 << 4)) >> 4));
						}
					} else if (hdmi_reg_30 == 0x6) {
						printf("Now is = HDMI2 2.0 and trim is HDMI1.4's trim(ignore)!\r\n");
					} else {
						printf("Unknow HDMI2 speed !!!\r\n");
					}
				} else {
					printf("!!!HDMI2 not enabled => make sure HDMI2 is probed\r\n");
					printf("1.4 HDMI2 VLane Trim data = 0x%04x\r\n", (int)HDMI_H_LANE_VL_1_4);
				}

				printf(" is cpu overclock[%d]\r\n", quary_secure_boot(SECUREBOOT_CPU_OVER_CLOCKING));
				printf("is VDEC overclock[%d]\r\n", quary_secure_boot(SECUREBOOT_VDEC_OVER_CLOCKING));
				printf(" is cpu downclock[%d]\r\n", quary_secure_boot(SECUREBOOT_CPU_LOWER_CLOCKING));
				printf("  is DSP1 disable[%d]\r\n", quary_secure_boot(SECUREBOOT_DSP1_DISABLE));
				printf("  is DSP2 disable[%d]\r\n", quary_secure_boot(SECUREBOOT_DSP2_DISABLE));
				printf("   is GPU disable[%d]\r\n", quary_secure_boot(SECUREBOOT_GPU_DISABLE));
				printf("   is MCU disable[%d]\r\n", quary_secure_boot(SECUREBOOT_MCU_DISABLE));
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][HDMI V PHY 1.4 VLANE] = 0x%08x\r\n", is_found, (int)code);
			}
		}
#elif defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64)
		u32     trim;
		int     code;
		BOOL    is_found;
		u32     RTC_Trim_15_8, ANA_REG_3_0, ANA_REG_7_6;
		u32     USB_SATA_Internal_12K_resistor;
		u32     USB_SATA_TX_TRIM, USB_SATA_RX_TRIM;
		u32     USB_PLL_LDO, USB_RX_LDO;
		u32     HDMI_resistor_1_4, HDMI_LANE_VL_1_4;
		u32     HDMI_resistor_2_0, HDMI_LANE_VL_2_0;
		u32     THERMAL;
		u32     I_Trim_2_5_G, R_Term_2_5_G;
		u32     clock_lane_trim[4];
		u32     data_lane_trim[8];
		//RTC
		code = otp_key_manager(EFUSE_RTC_TRIM_DATA);
		if (code == -33) {
			//!!!Please apply default value here!!!
			printf("Read RTC trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				//bit[7..0]
				RTC_Trim_15_8 = (trim & 0xFF);
				//bit[9..8]
				ANA_REG_7_6   = ((trim >> 8) & 0x3);
				//bit[13..10]
				ANA_REG_3_0   = ((trim >> 10) & 0xF);
				printf("is found [%d][%d][    ETH] = 0x%08x\r\n", is_found, EFUSE_RTC_TRIM_DATA, (int)trim);
				printf("   *RTC_3_0 Trim [0x%04x]\r\n", (int)ANA_REG_3_0);
				printf("   *RTC_7_6 Trim [0x%04x]\r\n", (int)ANA_REG_7_6);
				printf("  *RTC_15_8 Trim [0x%04x]\r\n", (int)RTC_Trim_15_8);
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][RTC] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		code = otp_key_manager(EFUSE_HDMI_RESISTOR_TRIM_DATA);
		if (code == -33) {
			printf("[%d] data = NULL\n", EFUSE_HDMI_RESISTOR_TRIM_DATA);
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				//bit[4..0]
				HDMI_resistor_1_4 = trim & 0x1F;
				//bit[9..5]
				HDMI_resistor_2_0 = ((trim >> 5) & 0x1F);
				if (HDMI_resistor_1_4 > 0x14) {
					printf("1.4 HDMI resistor Trim data error 0x%04x > 0x14\r\n", (int)HDMI_resistor_1_4);
				} else if (HDMI_resistor_1_4 < 0x1) {
					printf("1.4 HDMI resistor Trim data error 0x%04x < 0x1\r\n", (int)HDMI_resistor_1_4);
				} else {
					printf("  *1.4 HDMI resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)HDMI_resistor_1_4);
				}
				if (HDMI_resistor_2_0 > 0x14) {
					printf("2.0 HDMI resistor Trim data error 0x%04x > 0x14\r\n", (int)HDMI_resistor_2_0);
				} else if (HDMI_resistor_2_0 < 0x1) {
					printf("2.0 HDMI resistor Trim data error 0x%04x < 0x1\r\n", (int)HDMI_resistor_2_0);
				} else {
					printf("  *2.0 HDMI resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)HDMI_resistor_2_0);
				}
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][HDMI resistor] = 0x%08x\r\n", is_found, (int)code);
			}
		}
		code = otp_key_manager(EFUSE_HDMI_2_0_VLANE_TRIM_DATA);
		if (code == -33) {
			printf("[%d] data = NULL\n", EFUSE_HDMI_2_0_VLANE_TRIM_DATA);
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				printf("  HDMI 2.0 VLane tunning Trim = 0x%04x\r\n", (int)trim);
				//bit[11..0]
				HDMI_LANE_VL_2_0 = trim & 0xFFF;
				printf("  *2.0 HDMI VLane Trim data range success 0x0 <= [0x%04x] <= 0xFFF\r\n", (int)HDMI_LANE_VL_2_0);
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][HDMI 2.0 VLNE] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		code = otp_key_manager(EFUSE_HDMI_1_4_VLANE_TRIM_DATA);
		if (code == -33) {
			printf("[%d] data = NULL\n", EFUSE_HDMI_1_4_VLANE_TRIM_DATA);
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				printf("  HDMI 1.4 VLane tunning Trim = 0x%04x\r\n", (int)trim);
				//bit[11..0]
				HDMI_LANE_VL_1_4 = trim & 0xFFF;
				printf("  *1.4 HDMI VLane Trim data range success 0x0 <= [0x%04x] <= 0xFFF\r\n", (int)HDMI_LANE_VL_1_4);
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][HDMI 1.4 VLNE] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		code =  otp_key_manager(EFUSE_USB_SATA_TRIM_DATA);
		if (code == -33) {
			printf("[%d] data = NULL\n", EFUSE_USB_SATA_TRIM_DATA);
		} else {
			//12K resistor 1<= x <= 20(0x14)
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				printf("  USB(SATA) Trim data = 0x%04x\r\n", (int)trim);
				USB_SATA_Internal_12K_resistor = trim & 0x1F;
				USB_SATA_TX_TRIM = ((trim >> 5) & 0xF);
				USB_SATA_RX_TRIM = ((trim >> 9) & 0xF);
				if (USB_SATA_Internal_12K_resistor > 0x14) {
					printf("12K_resistor Trim data error 0x%04x > 20(0x14)\r\n", (int)USB_SATA_Internal_12K_resistor);
				} else if (USB_SATA_Internal_12K_resistor < 0x1) {
					printf("12K_resistor Trim data error 0x%04x < 1(0x1)\r\n", (int)USB_SATA_Internal_12K_resistor);
				} else {
					printf("  *12K_resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)USB_SATA_Internal_12K_resistor);
				}
				//TX term bit[8..5] 0x2 <= x <= 0xE
				if (USB_SATA_TX_TRIM > 0xE) {
					printf("USB(SATA) TX Trim data error 0x%04x > 0xE\r\n", (int)USB_SATA_TX_TRIM);
				} else if (USB_SATA_TX_TRIM < 0x2) {
					printf("USB(SATA) TX Trim data error 0x%04x < 0x2\r\n", (int)USB_SATA_TX_TRIM);
				} else {
					printf("  *USB(SATA) TX Trim data range success 0x2 <= [0x%04x] <= 0xE\r\n", (int)USB_SATA_TX_TRIM);
				}
				//RX term bit[12..9] 0x8 <= x <= 0xE
				if (USB_SATA_RX_TRIM > 0xE) {
					printf("USB(SATA) RX Trim data error 0x%04x > 0xE\r\n", (int)USB_SATA_RX_TRIM);
				} else if (USB_SATA_RX_TRIM < 0x8) {
					printf("USB(SATA) RX Trim data error 0x%04x < 0x8\r\n", (int)USB_SATA_RX_TRIM);
				} else {
					printf("  *USB(SATA) RX Trim data 0x8 <= [0x%04x] <= 0xE\r\n", (int)USB_SATA_RX_TRIM);
				}
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][USB trim] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		code =  otp_key_manager(EFUSE_USB_LDO_TRIM_DATA);
		if (code == -33) {
			printf("[%d] data = NULL\n", EFUSE_USB_LDO_TRIM_DATA);
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				printf("  USB(RX/PLL LDO) Trim data = 0x%04x\r\n", (int)trim);
				USB_PLL_LDO = (trim & 0x1);
				USB_RX_LDO = ((trim >> 1) & 0x1);

				//RX term bit[12..9] 0x8 <= x <= 0xE
				printf("USB PLL LDO[%d]\r\n", (int)USB_PLL_LDO);
				printf("USB  RX LDO[%d]\r\n", (int)USB_RX_LDO);
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][ USB LDO] = 0x%08x\r\n", is_found, (int)code);
			}
		}
		code = otp_key_manager(EFUSE_THERMAL_TRIM_DATA);
		if (code == -33) {
			printf("[%d] data = NULL\n", EFUSE_THERMAL_TRIM_DATA);
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				printf("Thermal Trim = 0x%04x => 58(0x3A) ~ 138(0x8A)\r\n", (int)trim);
				THERMAL = (trim & 0xFF);
				if (THERMAL > 138 || THERMAL < 58) {
					printf(" Thermal Trim data range  error THERMAL > 138 or THERMAL < 58 ==> [%d]\r\n", (int)THERMAL);
				} else {
					printf(" Thermal Trim data range success 58 <= THERMAL [%d] <= 138\r\n", (int)THERMAL);
				}
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][ Thermal] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		code = otp_key_manager(EFUSE_CSI_2_5G_I_TRIM_R_TERM_TRIM_DATA);
		if (code == -33) {
			printf("[%d] data = NULL\n", EFUSE_CSI_2_5G_I_TRIM_R_TERM_TRIM_DATA);
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				printf("CSI 2.5G I TRIM R TERM Trim = 0x%04x\r\n", (int)trim);
				//bit[5..0]
				I_Trim_2_5_G = (trim & 0x3F);
				//bit[9..6]
				R_Term_2_5_G = ((trim >> 6) & 0xF);
				printf(" CSI 2.5G I trim Trim data [%d]\r\n", (int)I_Trim_2_5_G);
				printf(" CSI 2.5G R term Trim data [%d]\r\n", (int)R_Term_2_5_G);
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][CSI2.5G I term R term] = 0x%08x\r\n", is_found, (int)code);
			}
		}
		code = otp_key_manager(EFUSE_CSI_1_5G_D1_D0_C3_C0_TRIM_DATA);
		if (code == -33) {
			printf("[%d] data = NULL\n", EFUSE_CSI_1_5G_D1_D0_C3_C0_TRIM_DATA);
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				printf("CSI 1.5G clock lane 0~3 & data lane 0~3 Trim data = 0x%04x\r\n", (int)trim);
				//bit[11..0]
				clock_lane_trim[0] = (trim & 0x3);
				clock_lane_trim[1] = ((trim >> 2) & 0x3);
				clock_lane_trim[2] = ((trim >> 4) & 0x3);
				clock_lane_trim[3] = ((trim >> 6) & 0x3);
				data_lane_trim[0] = ((trim >> 8) & 0x3);
				data_lane_trim[1] = ((trim >> 10) & 0x3);
				printf(" CSI 1.5G clock lane 0 Trim data range success [%d]\r\n", (int)clock_lane_trim[0]);
				printf(" CSI 1.5G clock lane 1 Trim data range success [%d]\r\n", (int)clock_lane_trim[1]);
				printf(" CSI 1.5G clock lane 2 Trim data range success [%d]\r\n", (int)clock_lane_trim[2]);
				printf(" CSI 1.5G clock lane 3 Trim data range success [%d]\r\n", (int)clock_lane_trim[3]);
				printf(" CSI 1.5G  data lane 0 Trim data range success [%d]\r\n", (int)data_lane_trim[0]);
				printf(" CSI 1.5G  data lane 1 Trim data range success [%d]\r\n", (int)data_lane_trim[1]);
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][CSI1.5G I term R term] = 0x%08x\r\n", is_found, (int)code);
			}
		}
		code = otp_key_manager(EFUSE_CSI_1_5G_D7_D2_TRIM_DATA);
		if (code == -33) {
			printf("[%d] data = NULL\n", EFUSE_CSI_1_5G_D7_D2_TRIM_DATA);
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				printf("CSI 1.5G data lane 2~7 Trim = 0x%04x\r\n", (int)trim);
				//bit[11..0]
				data_lane_trim[2] = (trim & 0x3);
				data_lane_trim[3] = ((trim >> 2) & 0x3);
				data_lane_trim[4] = ((trim >> 4) & 0x3);
				data_lane_trim[5] = ((trim >> 6) & 0x3);
				data_lane_trim[6] = ((trim >> 8) & 0x3);
				data_lane_trim[7] = ((trim >> 10) & 0x3);
				printf(" CSI 1.5G data lane 2 Trim data range success [%d]\r\n", (int)data_lane_trim[2]);
				printf(" CSI 1.5G data lane 3 Trim data range success [%d]\r\n", (int)data_lane_trim[3]);
				printf(" CSI 1.5G data lane 4 Trim data range success [%d]\r\n", (int)data_lane_trim[4]);
				printf(" CSI 1.5G data lane 5 Trim data range success [%d]\r\n", (int)data_lane_trim[5]);
				printf(" CSI 1.5G data lane 6 Trim data range success [%d]\r\n", (int)data_lane_trim[6]);
				printf(" CSI 1.5G data lane 7 Trim data range success [%d]\r\n", (int)data_lane_trim[7]);
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][CSI1.5G    D7 D2 trim] = 0x%08x\r\n", is_found, (int)code);
			}
		}
#elif defined(CONFIG_TARGET_NA51103)
		u32     trim;
		u32     remap_data;
		int     code;
		int     code2;
		BOOL    is_found;

		u32     TX_TRIM_ROUT, TX_TRIM_DAC, TX_TRIM_SEL_RX, TX_TRIM_SEL_TX;
		u32     CVBS_TRIM, VGA_TRIM;
		u32     THERMAL;
		u32     usb_internal_12K_resistor;
		u32     HDMI_resistor;
		u32     HDMI_LANE_VL;
		u32     HDMI_LANE0;
		u32     HDMI_LANE1;
		u32     HDMI_LANE2;
		u32     HDMI_CLK;

		u32     DDR_RZQ, DDR_LDO;
		u32     core_pwr_adj, core_pwr_lek;
		u32     BIAS_VLDOSEL_PI, BIAS_RX_VLDO_SEL, BIAS_LDOSEL_PLL;
		u32     BIAS_VREF_SEL, TX_ZTX_CTL;



		//Ethernet
		code = otp_key_manager(OTP_ETHERNET_TRIM_FIELD);
		if (code == -33) {
			//!!!Please apply default value here!!!
			printf("Read ethernet trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				TX_TRIM_ROUT    = trim & 0x7;
				TX_TRIM_DAC     = ((trim >> 3) & 0x1F);
				TX_TRIM_SEL_RX  = ((trim >> 8) & 0x7);
				TX_TRIM_SEL_TX  = ((trim >> 11) & 0x7);
				printf("is found [%d][%d][    ETH] = 0x%08x\r\n", is_found, OTP_ETHERNET_TRIM_FIELD, (int)trim);
				printf("  	=> TX_TRIM_ROUT[0x%04x]\r\n", (int)TX_TRIM_ROUT);
				printf("  	=>  TX_TRIM_DAC[0x%04x]\r\n", (int)TX_TRIM_DAC);
				printf("  	=> TX_TRIM_ROUT[0x%04x]\r\n", (int)TX_TRIM_SEL_RX);
				printf("  	=> TX_TRIM_ROUT[0x%04x]\r\n", (int)TX_TRIM_SEL_TX);
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][ETH] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		//SATA
		code = otp_key_manager(OTP_SATA_TRIM_FIELD);
		code2 = otp_key_manager(OTP_SATA_REMAIN_TRIM_FIELD);
		if (code == -33 || code2 == -33) {
			//!!!Please apply default value here!!!
			printf("Read SATA trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				BIAS_VLDOSEL_PI = trim & 0xF;
				BIAS_RX_VLDO_SEL = ((trim >> 4) & 0xF);
				BIAS_LDOSEL_PLL = ((trim >> 8) & 0xF);
				BIAS_VREF_SEL   = ((code2 >> 8) & 0xF);
				TX_ZTX_CTL      = ((code2 >> 12) & 0xF);

				printf("is found [%d][%d](%d)[ SATA]= 0x%08x\r\n", is_found, OTP_SATA_TRIM_FIELD, OTP_SATA_REMAIN_TRIM_FIELD, (int)trim);
				printf("  	=>  BIAS_VLDOSEL_PI[0x%04x]\r\n", (int)BIAS_VLDOSEL_PI);
				printf("  	=> BIAS_RX_VLDO_SEL[0x%04x]\r\n", (int)BIAS_RX_VLDO_SEL);
				printf("  	=>  BIAS_LDOSEL_PLL[0x%04x]\r\n", (int)BIAS_LDOSEL_PLL);
				printf("  	=>    BIAS_VREF_SEL[0x%04x]\r\n", (int)BIAS_VREF_SEL);
				printf("  	=>       TX_ZTX_CTL[0x%04x]\r\n", (int)TX_ZTX_CTL);
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][SATA1] = 0x%08x\r\n", is_found, (int)code);
				printf("is found [%d][SATA2] = 0x%08x\r\n", is_found, (int)code2);
			}
		}


		code = otp_key_manager(OTP_THERMAL_TRIM_FIELD);
		if (code == -33) {
			printf("Read thermal trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				//bit[8..0]
				THERMAL = trim & 0x1FF;
				printf("is found [%d][%d][ THERMAL] = 0x%08x\r\n", is_found, OTP_THERMAL_TRIM_FIELD, (int)trim);
				printf("  	=> THERMAL[0x%04x]\r\n", (int)THERMAL);
			} else {
				printf("is found [%d][THERMAL] = 0x%08x\r\n", is_found, (int)code);
			}
		}


		code = otp_key_manager(OTP_USB_SET1_TRIM_FIELD);
		if (code == -33) {
			printf("Read USB trim error\r\n");
			return -1;
		} else {
			is_found = extract_2_set_type_trim_valid(TRIM_USB_FIELD, code, (u32 *)&trim);
			if (is_found) {
				usb_internal_12K_resistor = trim;
				printf("is found [%d][%d][USB] = 0x%04x\r\n", is_found, OTP_USB_SET1_TRIM_FIELD, (int)trim);
				printf("  	=> 12K_resistor[0x%04x]\r\n", (int)usb_internal_12K_resistor);
			} else {
				printf("is found [%d][ USB] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		//ZQK & LDO
		code = otp_key_manager(OTP_DDR_RZQ_SET1_TRIM_FIELD);
		if (code == -33) {
			printf("Read DDR RZQ trim error\r\n");
			return -1;
		} else {
			is_found = extract_2_set_type_trim_valid(TRIM_DDR_RZQ_FIELD, code, (u32 *)&trim);
			if (is_found) {
				DDR_RZQ = trim;
				printf("is found [%d][%d][USB] = 0x%04x\r\n", is_found, OTP_DDR_RZQ_SET1_TRIM_FIELD, (int)trim);
				printf("  	=> DDR_RZQ[0x%04x]\r\n", (int)DDR_RZQ);
			} else {
				printf("is found [%d][DDR_RZQ] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		code = otp_key_manager(OTP_DDR_LDO_SET2_TRIM_FIELD);
		if (code == -33) {
			printf("Read DDR LDO trim error\r\n");
			return -1;
		} else {
			is_found = extract_2_set_type_trim_valid(TRIM_DDR_LDO_FIELD, code, (u32 *)&trim);
			if (is_found) {
				DDR_LDO = trim >> 8;
				printf("is found [%d][%d][USB] = 0x%04x\r\n", is_found, OTP_DDR_LDO_SET2_TRIM_FIELD, (int)trim);
				printf("  	=> DDR_LDO[0x%04x]\r\n", (int)DDR_LDO);
			} else {
				printf("is found [%d][DDR_LDO] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		//Core power
		code = otp_key_manager(OTP_CORE_POWER_ADJ_SET1_TRIM_FIELD);
		if (code == -33) {
			printf("Read Core PWR adj trim error\r\n");
			return -1;
		} else {
			is_found = extract_2_set_type_trim_valid(TRIM_CORE_PWR_ADJ_FIELD, code, (u32 *)&trim);
			if (is_found) {
				core_pwr_adj = (trim & 0xF);
				printf("is found [%d][%d][CorePwr adj] = 0x%04x\r\n", is_found, TRIM_CORE_PWR_ADJ_FIELD, (int)trim);
				printf("  	=> core_pwr_adj[0x%04x]\r\n", (int)core_pwr_adj);
			} else {
				printf("is found [%d][CorePwr adj] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		code = otp_key_manager(OTP_CORE_POWER_LEK_SET2_TRIM_FIELD);
		if (code == -33) {
			printf("Read Core PWR lek trim error\r\n");
			return -1;
		} else {
			is_found = extract_2_set_type_trim_valid(TRIM_CORE_PWR_LEK_FIELD, code, (u32 *)&trim);
			if (is_found) {
				core_pwr_lek = trim >> 8;
				printf("is found [%d][%d][CorePwr lek] = 0x%04x\r\n", is_found, OTP_CORE_POWER_LEK_SET2_TRIM_FIELD, (int)trim);
				printf("  	=> core_pwr_lek[0x%04x]\r\n", (int)core_pwr_lek);
			} else {
				printf("is found [%d][CorePwr lek] = 0x%08x\r\n", is_found, (int)code);
			}
		}


		code = otp_key_manager(OTP_HDMI_TRIM_FIELD);
		code2 = otp_key_manager(OTP_HDMI_VALE_VL_REMAIN_TRIM_FIELD);
		if (code == -33 || code2 == -33) {
			//!!!Please apply default value here!!!
			printf("Read HDMI trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				//bit[6..0]
				HDMI_resistor = trim & 0x1F;
				HDMI_LANE_VL = (trim >> 5) | (((code2 >> 9) & 0x7) << 9);
				HDMI_LANE0  = (HDMI_LANE_VL & 0x7);
				HDMI_LANE1  = ((HDMI_LANE_VL >> 3) & 0x7);
				HDMI_LANE2  = ((HDMI_LANE_VL >> 6) & 0x7);
				HDMI_CLK    = ((HDMI_LANE_VL >> 9) & 0x7);

				printf("is found [%d][%d][HDMI]= 0x%08x->0x%08x\r\n", is_found, OTP_HDMI_TRIM_FIELD, (int)trim, (int)code2);
				printf("  	=> HDMI_resistor[0x%04x]\r\n", (int)HDMI_resistor);
				printf("  	=>  HDMI_LANE_VL[0x%04x]\r\n", (int)HDMI_LANE_VL);
				printf("  	=>  HDMI_LANE0[0x%04x]\r\n", (int)HDMI_LANE0);
				printf("  	=>  HDMI_LANE1[0x%04x]\r\n", (int)HDMI_LANE1);
				printf("  	=>  HDMI_LANE2[0x%04x]\r\n", (int)HDMI_LANE2);
				printf("  	=>    HDMI_CLK[0x%04x]\r\n", (int)HDMI_CLK);
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][HDMI] = 0x%08x\r\n", is_found, (int)code);
			}
		}


		code = otp_key_manager(OTP_CVBS_VGA_TRIM_FIELD);
		if (code == -33) {
			printf("Read CVBS & VGA trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				//bit[6..0]
				CVBS_TRIM = trim & 0x7F;
				//bit[13..7] as VGA[6..0] bit[7] always = 1
				VGA_TRIM = ((trim >> 7) | 0x80);
				printf("is found [%d][%d][CVBSVGA] = 0x%08x\r\n", is_found, OTP_CVBS_VGA_TRIM_FIELD, (int)trim);
				printf("  	=> CVBS_TRIM[0x%04x]\r\n", (int)CVBS_TRIM);
				printf("  	=>  VGA_TRIM[0x%04x]\r\n", (int)VGA_TRIM);
			} else {
				printf("is found [%d][CVBS] = 0x%08x\r\n", is_found, (int)code);
			}
		}
#elif defined(CONFIG_TARGET_NS02302_A64)
		u32     trim;
		u32     remap_data;
		int     code;
		int     code2;
		BOOL    is_found;

		u32     TX_TRIM_ROUT, TX_TRIM_DAC, TX_TRIM_SEL_RX, TX_TRIM_SEL_TX;
		u32     REG_TX_TRIM_ROUT, REG_TX_TRIM_DAC, REG_TX_TRIM_SEL_RX, REG_TX_TRIM_SEL_TX;
		u32     USB_Int_12K_resistor;
		u32     USB_TX_TRIM, USB_RX_TRIM;

		u32     reg_USB_Internal_12K_resistor;
		u32     reg_USB_TX_TRIM/*, reg_USB_RX_TRIM*/;

		u32     MIPI_RX_Rterm_GRP1, MIPI_RX_Rterm_GRP2, MIPI_TX_iadj;

		u32     RZQ_DATA, RZQ_SIGN;
		u32     DDR_LDO;
		u32     THERMAL;
		u32     trim_A, trim_B;
		//Ethernet
		code = otp_key_manager(EFUSE_TRIM_DATA_ETHERNET_PRI);
		if (code == -33) {
			//!!!Please apply default value here!!!
			printf("Read ethernet trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				TX_TRIM_ROUT    = trim & 0x7;
				TX_TRIM_DAC     = ((trim >> 3) & 0x1F);
				TX_TRIM_SEL_RX  = ((trim >> 8) & 0x7);
				TX_TRIM_SEL_TX  = ((trim >> 11) & 0x7);
				printf("is found [    ETH] = 0x%08x\r\n", (int)trim);
				printf("  	=> TX_TRIM_ROUT[0x%04x]\r\n", (int)TX_TRIM_ROUT);
				printf("  	=>  TX_TRIM_DAC[0x%04x]\r\n", (int)TX_TRIM_DAC);
				printf("  	=> TX_TRIM_ROUT[0x%04x]\r\n", (int)TX_TRIM_SEL_RX);
				printf("  	=> TX_TRIM_ROUT[0x%04x]\r\n", (int)TX_TRIM_SEL_TX);

				//0xF02B3B74 bit[5..3]
				REG_TX_TRIM_ROUT    = (readl(IOADDR_ETH_REG_BASE + 0x3B74) >> 3) & 0x7;
				//0xF02B3B78 bit[4..0]
				REG_TX_TRIM_DAC     = (readl(IOADDR_ETH_REG_BASE + 0x3B78) >> 0) & 0x1F;
				//0xF02B3B68 bit[7..5]
				REG_TX_TRIM_SEL_RX  = (readl(IOADDR_ETH_REG_BASE + 0x3B68) >> 5) & 0x7;
				//0xF02B3B68 bit[4..2]
				REG_TX_TRIM_SEL_TX  = (readl(IOADDR_ETH_REG_BASE + 0x3B68) >> 2) & 0x7;

				if ((TX_TRIM_ROUT == REG_TX_TRIM_ROUT) && (TX_TRIM_DAC == REG_TX_TRIM_DAC) && (TX_TRIM_SEL_RX == REG_TX_TRIM_SEL_RX) && (TX_TRIM_SEL_TX == REG_TX_TRIM_SEL_TX)) {
					printf("  p[0x%02x]=REG[0x%02x]\r\n", (int)TX_TRIM_ROUT, (int)REG_TX_TRIM_ROUT);
					printf("   TX_TRIM_DAC[0x%02x]=REG[0x%02x]\r\n", (int)TX_TRIM_DAC, (int)REG_TX_TRIM_DAC);
					printf("TX_TRIM_SEL_RX[0x%02x]=REG[0x%02x]\r\n", (int)TX_TRIM_SEL_RX, (int)REG_TX_TRIM_SEL_RX);
					printf("TX_TRIM_SEL_TX[0x%02x]=REG[0x%02x]\r\n", (int)TX_TRIM_SEL_TX, (int)REG_TX_TRIM_SEL_TX);
					printf("Eth trim SUCCESS\r\n");
				} else {
					printf("  TX_TRIM_ROUT[0x%02x]<=>REG[0x%02x]\r\n", (int)TX_TRIM_ROUT, (int)REG_TX_TRIM_ROUT);
					printf("   TX_TRIM_DAC[0x%02x]<=>REG[0x%02x]\r\n", (int)TX_TRIM_DAC, (int)REG_TX_TRIM_DAC);
					printf("TX_TRIM_SEL_RX[0x%02x]<=>REG[0x%02x]\r\n", (int)TX_TRIM_SEL_RX, (int)REG_TX_TRIM_SEL_RX);
					printf("TX_TRIM_SEL_TX[0x%02x]<=>REG[0x%02x]\r\n", (int)TX_TRIM_SEL_TX, (int)REG_TX_TRIM_SEL_TX);
					printf("Eth trim Fail\r\n");
				}
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][ETH] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		//USB
		code = otp_key_manager(EFUSE_TRIM_DATA_USB_PRI);
		if (code == -33) {
			//!!!Please apply default value here!!!
			printf("Read usb trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				USB_Int_12K_resistor = trim & 0x1F;
				USB_TX_TRIM = ((trim >> 5) & 0xF);
				USB_RX_TRIM = ((trim >> 9) & 0xF);
				printf("12K_resistor Trim = 0x%04x\r\n", USB_Int_12K_resistor);
				printf("      USB TX Trim = 0x%04x\r\n", (int)USB_TX_TRIM);
				printf("      USB RX Trim = 0x%04x\r\n", (int)USB_RX_TRIM);

				reg_USB_Internal_12K_resistor = readl(IOADDR_USB3PHY_REG_BASE + 0x2680);
				reg_USB_Internal_12K_resistor &= 0x1F;
				reg_USB_TX_TRIM = readl(IOADDR_USB3PHY_REG_BASE + 0x2040);
				reg_USB_TX_TRIM = ((reg_USB_TX_TRIM >> 4) & 0xF);

				if (USB_Int_12K_resistor == reg_USB_Internal_12K_resistor) {
					printf("   u3_trim_rint_sel Trim [0x%04x] == Reg[0x%04x][O]\r\n", (int)USB_Int_12K_resistor, (int)reg_USB_Internal_12K_resistor);
					printf("   usb trim success\r\n");
				} else {
					printf("   u3_trim_rint_sel Trim [0x%04x] != Reg[0x%04x][X]\r\n", (int)USB_Int_12K_resistor, (int)reg_USB_Internal_12K_resistor);
					printf("   usb trim fail\r\n");
				}
				if (USB_TX_TRIM == reg_USB_TX_TRIM) {
					printf("   USB_TX_TRIM Trim [0x%04x] == Reg[0x%04x][O]\r\n", (int)USB_TX_TRIM, (int)reg_USB_TX_TRIM);
					printf("   usb trim success\r\n");
				} else {
					printf("   USB_TX_TRIM Trim [0x%04x] != Reg[0x%04x][X]\r\n", (int)USB_TX_TRIM, (int)reg_USB_TX_TRIM);
					printf("   usb trim fail\r\n");
				}

			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][USB] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		//DDR
		code = otp_key_manager(EFUSE_TRIM_DATA_DDR_RZQ_LDO_PRI);
		if (code == -33) {
			//!!!Please apply default value here!!!
			printf("Read DDR trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				//0x00~0x07 =>  0 -> 7
				//0x11~0x17 => -7 ~ -1
				RZQ_DATA = trim & 0xF;
				RZQ_SIGN = (trim >> 4) & 0x01;
				DDR_LDO = ((trim >> 5) & 0xF);
				printf(" DDR RZQ Trim = 0x%04x\r\n", RZQ_DATA);
				printf("RZQ_SIGN Trim = %d\r\n", (int)RZQ_SIGN);
				printf(" DDR_LDO Trim = 0x%04x\r\n", (int)DDR_LDO);
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][DDR] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		//TRIMA
		code = otp_key_manager(EFUSE_TRIM_DATA_A_PRI);
		if (code == -33) {
			//!!!Please apply default value here!!!
			printf("Read trim_A error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				trim_A = trim & 0x7F;
				printf("]trimA = 0x%04x\r\n", (int)trim_A);
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][TRIM_A] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		//TRIMB
		code = otp_key_manager(EFUSE_TRIM_DATA_B_PRI);
		if (code == -33) {
			//!!!Please apply default value here!!!
			printf("Read trim_B error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				trim_B = trim & 0x1FF;
				printf("]trimB = 0x%04x\r\n", (int)trim_B);
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][TRIM_B] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		//Thermal
		code = otp_key_manager(EFUSE_TRIM_DATA_THERMAL_PRI);
		if (code == -33) {
			printf("Read thermal trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				//bit[8..0]
				THERMAL = trim & 0x1FF;
				printf("is found [ THERMAL] = 0x%08x\r\n", (int)trim);
				printf("  	=> THERMAL[0x%04x]\r\n", (int)THERMAL);
			} else {
				printf("is found [%d][THERMAL] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		//MIPI
		code = otp_key_manager(EFUSE_TRIM_DATA_MIPI_TX_RX_PRI);
		if (code == -33) {
			printf("Read MIPI trim error\r\n");
			return -1;
		} else {
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if (is_found) {
				MIPI_RX_Rterm_GRP1  = trim & 0xF;               // bit[3..0] all available
				MIPI_RX_Rterm_GRP2  = ((trim >> 4) & 0xF);      // bit[7..4]
				MIPI_TX_iadj        = ((trim >> 8) & 0x7);     // bit[10..8]
				printf("  	=>	MIPI_RX_Rterm_GRP1[0x%04x]\r\n", (int)MIPI_RX_Rterm_GRP1);
				printf("  	=>	MIPI_RX_Rterm_GRP2[0x%04x]\r\n", (int)MIPI_RX_Rterm_GRP2);
				printf("  	=> 	MIPI_TX_iadj[0x%04x]\r\n", (int)MIPI_TX_iadj);
			} else {
				printf("is found [%d][MIPI-TX-RX] = 0x%08x\r\n", is_found, (int)code);
			}
		}
#endif
	} else if (!strncmp(argv[1], "quary", 5)) {
		printf("=>[quary]\r\n");
		printf("             is_secure_enable()=%d\r\n", is_secure_enable());
		printf("       is_data_area_encrypted()=%d\r\n", is_data_area_encrypted());
		printf("             is_signature_rsa()=%d\r\n", is_signature_rsa());
#if defined(CONFIG_TARGET_NS02302_A64)
		if (chip_id == CHIP_NS02402) {
			printf("          is_signature_rsapss()=%d\r\n", is_signature_rsapss());
			printf("      is_signature_rsapkcs1p5()=%d\r\n", is_signature_rsapkcs1p5());
			printf("       is_header_signature_en()=%d\r\n", is_header_signature_en());
		}
#elif defined(CONFIG_TARGET_NS02401_A64)
		printf("          is_signature_rsapss()=%d\r\n", is_signature_rsapss());
		printf("      is_signature_rsapkcs1p5()=%d\r\n", is_signature_rsapkcs1p5());
		printf("       is_header_signature_en()=%d\r\n", is_header_signature_en());
#endif
#if defined(CONFIG_TARGET_NS02302_A64) || defined(CONFIG_TARGET_NS02301) || defined(CONFIG_TARGET_NS02401_A64)
		printf("           is_signature_ecdsa()=%d\r\n", is_signature_ecdsa());
#endif
		printf("    is_signature_chsum_enable()=%d\r\n", is_signature_rsa_chsum_enable());
		printf("           is_JTAG_DISABLE_en()=%d\r\n", is_JTAG_DISABLE_en());
		printf("        is_1st_key_programmed()=%d\r\n", is_1st_key_programmed());
		printf("        is_2nd_key_programmed()=%d\r\n", is_2nd_key_programmed());
		printf("        is_3rd_key_programmed()=%d\r\n", is_3rd_key_programmed());
		printf("        is_4th_key_programmed()=%d\r\n", is_4th_key_programmed());
		printf("        is_5th_key_programmed()=%d\r\n", is_5th_key_programmed());
		printf("         is_1st_key_read_lock()=%d\r\n", is_1st_key_read_lock());
		printf("         is_2nd_key_read_lock()=%d\r\n", is_2nd_key_read_lock());
		printf("         is_3rd_key_read_lock()=%d\r\n", is_3rd_key_read_lock());
		printf("         is_4th_key_read_lock()=%d\r\n", is_4th_key_read_lock());
		printf("         is_5th_key_read_lock()=%d\r\n", is_5th_key_read_lock());
#if defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64)
		printf("              is_hdmi_disable()=%d\r\n", is_hdmi_disable());
		printf("              is_DSP1_disable()=%d\r\n", is_DSP1_disable());
		printf("              is_DSP2_disable()=%d\r\n", is_DSP2_disable());
		printf("               is_GPU_disable()=%d\r\n", is_GPU_disable());
		printf("               is_MCU_disable()=%d\r\n", is_MCU_disable());
#endif
#ifdef CONFIG_NVT_PCIE_MAPPING
		printf("             is_ep_secure_enable()=%d\r\n", is_ep_secure_enable());
		printf("       is_ep_data_area_encrypted()=%d\r\n", is_ep_data_area_encrypted());
		printf("             is_ep_signature_rsa()=%d\r\n", is_ep_signature_rsa());
		printf("is_ep_signature_rsa_chsum_enable()=%d\r\n", is_ep_signature_rsa_chsum_enable());
#endif
	}
#if defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64)
	else if (!strncmp(argv[1], "DSP1_disable", 12)) {
		printf("=>[DSP1_disable]\r\n");
		efuse_DSP1_dis();
		printf("is_DSP1_DISABLE_en()=%d\r\n", is_DSP1_disable());
	} else if (!strncmp(argv[1], "DSP2_disable", 12)) {
		printf("=>[DSP2_disable]\r\n");
		efuse_DSP2_dis();
		printf("is_DSP2_DISABLE_en()=%d\r\n", is_DSP2_disable());
	} else if (!strncmp(argv[1], "GPU_disable", 11)) {
		printf("=>[GPU_disable]\r\n");
		efuse_GPU_dis();
		printf("is_GPU_DISABLE_en()=%d\r\n", is_GPU_disable());
	} else if (!strncmp(argv[1], "MCU_disable", 11)) {
		printf("=>[MCU_disable]\r\n");
		efuse_MCU_dis();
		printf("is_MCU_DISABLE_en()=%d\r\n", is_MCU_disable());
	}
#endif
	else {
		return CMD_RET_USAGE;
	}
	return 0;
}


U_BOOT_CMD(
	nvt_secure_en,  2,  0,  do_nvt_secure_en_cmd,
	"secure enable related API",
	"[Option] \n"
	"              [secure_en]\n"
	"              [data_encrypt]\n"
	"              [rsa_en]\n"
#if defined(CONFIG_TARGET_NS02302_A64) || defined(CONFIG_TARGET_NS02301) || defined(CONFIG_TARGET_NS02401_A64)
#if defined(CONFIG_TARGET_NS02302_A64) || defined(CONFIG_TARGET_NS02401_A64)
	"              [rsapss_en](=>pkcs2.1)\n"
	"              [rsapkcs1p5_en]\n"
	"              [header_signature_en]\n"
#endif
	"              [ecdsa_en]\n"
	"              [ecdsa_chksum_en]\n"
#endif
	"              [rsa_chk_en]\n"
	"              [jtag_disable]\n"
#if defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64)
	"              [DSP1_disable]\n"
	"              [DSP2_disable]\n"
	"              [ GPU_disable]\n"
	"              [ MCU_disable]\n"
#endif
	"              [quary]\n"
	"              [uniqueue_id]\n"
#if (defined(CONFIG_TARGET_NA51090) || defined(CONFIG_TARGET_NS02302_A64) || defined(CONFIG_TARGET_NA51090_A64) || defined(CONFIG_TARGET_NA51103) || defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64) || defined(CONFIG_TARGET_NS02401_A64))
	"              [trim]\n"
#endif
);

int do_nvt_jtag_dis_cmd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	efuse_jtag_dis();
	return 0;
}

U_BOOT_CMD(
	nvt_jtag_disable,  1,  0,  do_nvt_jtag_dis_cmd,
	"jtag disable",
	"After jtag disable, JTAG will disable forever\n"
);
#endif

