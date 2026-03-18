/*
	Novatek protected header file of NA51055 driver.

	The header file for Novatek protected APIs of NT96660's driver.

	@file       efuse_protected.h
	@ingroup    mIDriver
	@note       For Novatek internal reference, don't export to agent or customer.

	Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#ifndef _NVT_EFUSE_PROTECTED_H
#define _NVT_EFUSE_PROTECTED_H

#include <linux/soc/nvt/nvt_type.h>

typedef enum {
	EFUSE_AUTO_MODE = 0x0,
	EFUSE_MANUAL_MODE,
} EFUSE_MODE;

typedef enum {
	EFUSE_READ = 0x0,                               //A_READ
	EFUSE_MARGIN_READ,                              //Margin_A_READ
	EFUSE_PROGRAM,
} EFUSE_OPERATION_MODE;

typedef enum {
	EFUSE_REPAIR_DISABLE = 0x0,                     //Repair disable
	EFUSE_REPAIR_ENABLE,                            //Repair enable

	ENUM_DUMMY4WORD(EFUSE_REPAIR_EN)
} EFUSE_REPAIR_EN;

typedef enum {
	EFUSE_REPAIR_BANK0 = 0x0,                       //Repair bank0
	EFUSE_REPAIR_BANK1,                             //Repair bank1
	EFUSE_REPAIR_BANK2,                             //Repair bank2
	EFUSE_REPAIR_BANK3,                             //Repair bank3

	EFUSE_REPAIR_BANK_CNT,

	ENUM_DUMMY4WORD(EFUSE_REPAIR_BANK)
} EFUSE_REPAIR_BANK;

STATIC_ASSERT(EFUSE_REPAIR_BANK_CNT <= 4);

typedef enum {
	EFUSE_ARRAY_FUNCTION_SEL = 0x0,                 //Arrary function selected
	EFUSE_REDUNDANCY_FUNCTION_SEL,                  //Redundancy function selected

	ENUM_DUMMY4WORD(EFUSE_ARRAY_REDUNDANCY_SEL)

} EFUSE_ARRAY_REDUNDANCY_SEL;

typedef enum {
	EFUSE_KEY_MANAGER_NORMAL = 0x0,                 // Normal operation
	EFUSE_KEY_MANAGER_CRYPTO,                       // to destination crypto engine
	EFUSE_KEY_MANAGER_RSA,                          // to destination RSA engine
	EFUSE_KEY_MANAGER_HASH,                         // to destination HASH engine

	EFUSE_KEY_MANAGER_DST_CNT,
	ENUM_DUMMY4WORD(EFUSE_KEY_MANAGER_DST)
} EFUSE_KEY_MANAGER_DST;

typedef enum {
	EFUSE_MASK_AREA_TYPE = 0x0,
	EFUSE_READ_ONLY_AREA_TYPE,

	ENUM_DUMMY4WORD(EFUSE_USER_AREA_TYPE)

} EFUSE_USER_AREA_TYPE;


typedef enum {
	EFUSE_WRITE_NO_0_KEY_SET_FIELD = 0x0,
	EFUSE_WRITE_NO_1_KEY_SET_FIELD,
	EFUSE_WRITE_NO_2_KEY_SET_FIELD,
	EFUSE_WRITE_NO_3_KEY_SET_FIELD,
	EFUSE_WRITE_NO_4_KEY_SET_FIELD,
	EFUSE_WRITE_NO_5_KEY_SET_FIELD,
	EFUSE_WRITE_NO_6_KEY_SET_FIELD,
	EFUSE_WRITE_NO_7_KEY_SET_FIELD,


	EFUSE_WRITE_NO_8_KEY_SET_FIELD,
	EFUSE_WRITE_NO_9_KEY_SET_FIELD,
	EFUSE_WRITE_NO_10_KEY_SET_FIELD,
	EFUSE_WRITE_NO_11_KEY_SET_FIELD,

	EFUSE_WRITE_NO_12_KEY_SET_FIELD,
	EFUSE_WRITE_NO_13_KEY_SET_FIELD,
	EFUSE_WRITE_NO_14_KEY_SET_FIELD,
	EFUSE_WRITE_NO_15_KEY_SET_FIELD,

	EFUSE_WRITE_NO_16_KEY_SET_FIELD,
	EFUSE_WRITE_NO_17_KEY_SET_FIELD,
	EFUSE_WRITE_NO_18_KEY_SET_FIELD,
	EFUSE_WRITE_NO_19_KEY_SET_FIELD,

	EFUSE_TOTAL_KEY_SET_FIELD,

	EFUSE_CRYPTO_ENGINE_KEY_CNT = EFUSE_WRITE_NO_8_KEY_SET_FIELD,
	EFUSE_RSA_ENGINE_KEY_CNT = EFUSE_TOTAL_KEY_SET_FIELD,
	EFUSE_HASH_ENGINE_KEY_CNT = EFUSE_TOTAL_KEY_SET_FIELD,

	ENUM_DUMMY4WORD(EFUSE_WRITE_KEY_SET_TO_OTP_FIELD)
} EFUSE_WRITE_KEY_SET_TO_OTP_FIELD;

typedef enum {
	EFUSE_USB2_TRIM_DATA = 0x100,
	EFUSE_USBC_TRIM_DATA,
	EFUSE_DDR_H_TRIM_DATA,
	EFUSE_DDR_V_TRIM_DATA,
	EFUSE_HDMI_TRIM_DATA,

	EFUSE_THERMAL_TRIM_DATA,
	EFUSE_COREPOWER_LEAKAGE_TRIM_DATA,
	EFUSE_CVBS_VGA_TRIM_DATA,

	EFUSE_PCIE_REFCLK_TRIM_DATA,

	EFUSE_CYPHER_KEY_DATA_0,
	EFUSE_CYPHER_KEY_DATA_1,
	EFUSE_CYPHER_KEY_DATA_2,
	EFUSE_CYPHER_KEY_DATA_3,
	EFUSE_CYPHER_KEY_DATA_4,
	EFUSE_CYPHER_KEY_DATA_5,
	EFUSE_CYPHER_KEY_DATA_6,
	EFUSE_CYPHER_KEY_DATA_7,

	EFUSE_VER_PKG_UID,
//  EFUSE_VER_DIE_UID,
    EFUSE_VER_ENGINEER_SIM_PKG_UID,
	EFUSE_VER_EXTEND_UID,
	EFUSE_PARAM_CNT,
	ENUM_DUMMY4WORD(EFUSE_PARAM_DATA)
} EFUSE_PARAM_DATA;

typedef enum {
	EFUSE_OTP_1ST_KEY_SET_FIELD = 0x0,        // This if for secure boot
	EFUSE_OTP_2ND_KEY_SET_FIELD,
	EFUSE_OTP_3RD_KEY_SET_FIELD,
	EFUSE_OTP_4TH_KEY_SET_FIELD,
	EFUSE_OTP_5TH_KEY_SET_FIELD,
	EFUSE_OTP_TOTAL_KEY_SET_FIELD,
} EFUSE_OTP_KEY_SET_FIELD;

#define EFUSE_OTP_KEY_FIELD_CNT			4


// Capacity of Storage device
typedef struct {
	UINT32  uiWidth;                    // width
	UINT32  uiHeight;                   // height
} CDC_ABILITY, *PCDC_ABILITY;

/**
    @name eFuse Package extended ability

\n  For specific ability chech
*/
//@{
typedef enum {
	EFUSE_ABILITY_CDC_RESOLUTION    = 0x1001,       //< Check package resoltion of codec
	EFUSE_ABILITY_CDC_PLL_FREQ      = 0x1002,       //< Check package PLL freq of codec
	EFUSE_ABILITY_CLK_FREQ          = 0x1003,       //< Max freq
	EFUSE_ABILITY_HDMI              = 0x1004,       //< HDMI available or not
	EFUSE_ABILITY_VENC_FREQ         = 0x1005,       //< Query VENC max freq available or not
	EFUSE_ABILITY_VDEC_FREQ         = 0x1006,       //< Query VDEC max freq available or not
	EFUSE_ABILITY_VCAP2_AVL         = 0x1007,       //< Query VCAP2 available or not
	EFUSE_ABILITY_VCAP1_AVL         = 0x1008,       //< Quary VCAP1 available or not
	///< Context are :
	///< - @b UINT32 * : starting address of CDC_ABILITY
	///< - @b UINT32 * : @EFUSE_ABILITY_VENC_FREQ & EFUSE_ABILITY_VDEC_FREQ represent freq

	ENUM_DUMMY4WORD(EFUSE_PKG_ABILITY_LIST)
} EFUSE_PKG_ABILITY_LIST;
//@}

/**
    Crypto engine check
*/
typedef enum {
	SECUREBOOT_SECURE_EN = 0x00,        ///< Quary if secure enable or not
	SECUREBOOT_DATA_AREA_ENCRYPT,       ///< Quary if data area encrypt to cypher text or not
	SECUREBOOT_SIGN_RSA,                ///< Quary if Signature methed is RSA or not(AES)
	SECUREBOOT_SIGN_RSA_CHK,            ///< Quary if Signature hash checksum RSA key correct or not
	SECUREBOOT_JTAG_DISABLE_EN,         ///< Quary if JTAG is disable or not(TRUE : disable)

	SECUREBOOT_1ST_KEY_SET_PROGRAMMED,  ///< Quary if 1st key set programmed or not
	SECUREBOOT_2ND_KEY_SET_PROGRAMMED,  ///< Quary if 2nd key set programmed or not
	SECUREBOOT_3RD_KEY_SET_PROGRAMMED,  ///< Quary if 3rd key set programmed or not
	SECUREBOOT_4TH_KEY_SET_PROGRAMMED,  ///< Quary if 4th key set programmed or not
	SECUREBOOT_5TH_KEY_SET_PROGRAMMED,  ///< Quary if 5th key set programmed or not

	SECUREBOOT_1ST_KEY_SET_READ_LOCK,   ///< Quary if 1st key read lock already or not
	SECUREBOOT_2ND_KEY_SET_READ_LOCK,   ///< Quary if 2nd key read lock already or not
	SECUREBOOT_3RD_KEY_SET_READ_LOCK,   ///< Quary if 3rd key read lock already or not
	SECUREBOOT_4TH_KEY_SET_READ_LOCK,   ///< Quary if 4th key read lock already or not
	SECUREBOOT_5TH_KEY_SET_READ_LOCK,   ///< Quary if 5th key read lock already or not

	SECUREBOOT_STATUS_NUM,

	SECUREBOOT_NEW_KEY_RULE,

	SECUREBOOT_VENC_OVER_CLOCKING,      ///< Quary if VENC can over clocking or not
	SECUREBOOT_VDEC_OVER_CLOCKING,      ///< Quary if VDEC can over clocking or not

	SECUREBOOT_CPU_ULTRA_OVER_CLOCKING,
    SECUREBOOT_VDEC_OVER_CLOCKING_600,  ///< Quary if VDEC can over ultra clocking or not
} SECUREBOOT_STATUS;

#define NVT_EFUSE_VERSION                               0x22041508
#define efuse_check_available(name)                     efuse_check_available2(name, NVT_EFUSE_VERSION)
#define efuse_check_available_extend(param1, param2)    efuse_check_available_extend2(param1, param2, NVT_EFUSE_VERSION)

#define EFUSE_SUCCESS                   	E_OK
#define EFUSE_FREEZE_ERR               	 	-1001          // Programmed already, only can read
#define EFUSE_INACTIVE_ERR              	-1002          // This field is empty(not programmed yet)
#define EFUSE_INVALIDATE_ERR            	-1003          // This field force invalidate already
#define EFUSE_UNKNOW_PARAM_ERR          	-1004          // efuse param field not defined
#define EFUSE_OPS_ERR                   	-1005          // efuse operation error
#define EFUSE_SECURITY_ERR              	-1006          // efuse under security mode => can not read back
#define EFUSE_PARAM_ERR                 	-1007          // efuse param error
#define EFUSE_CONTENT_ERR               	-1008          // efuse operation error
#define OTP_HW_SECURE_EN                	(1 << 0)
#define OTP_FW_SECURE_EN                	(1 << 5)
#define OTP_DATA_ENCRYPT_EN             	(1 << 7)
#define OTP_SIGNATURE_RSA               	(1 << 1)
#define OTP_SIGNATURE_RSA_CHK_EN        	(1 << 3)
#define OTP_JTAG_DISABLE_EN             	(1 << 2)

#define OTP_VDEC_OVER_CLOCKING_BIT      	14
#define OTP_VENC_OVER_CLOCKING_BIT      	17

#define OTP_CPU_ULTRA_OVER_CLOCKING_BIT		18
#define OTP_VDEC_OVER_CLOCKING_600_AVL_BIT  19

#define OTP_VDEC_OVER_CLOCKING_AVL      	(1 << OTP_VDEC_OVER_CLOCKING_BIT)

#define OTP_VENC_OVER_CLOCKING_AVL      	(1 << OTP_VENC_OVER_CLOCKING_BIT)
#define OTP_CPU_ULTRA_OVER_CLOCKING			(1 << OTP_CPU_ULTRA_OVER_CLOCKING_BIT)
#define OTP_VDEC_OVER_CLOCKING_600_AVL		(1 << OTP_VDEC_OVER_CLOCKING_600_AVL_BIT)


#define OTP_1ST_KEY_PROGRAMMED_BIT      	27
#define OTP_2ND_KEY_PROGRAMMED_BIT      	28
#define OTP_3RD_KEY_PROGRAMMED_BIT      	29
#define OTP_4TH_KEY_PROGRAMMED_BIT      	30
#define OTP_5TH_KEY_PROGRAMMED_BIT      	31

#define OTP_1ST_KEY_PROGRAMMED          	(1 << OTP_1ST_KEY_PROGRAMMED_BIT)
#define OTP_2ND_KEY_PROGRAMMED          	(1 << OTP_2ND_KEY_PROGRAMMED_BIT)
#define OTP_3RD_KEY_PROGRAMMED          	(1 << OTP_3RD_KEY_PROGRAMMED_BIT)
#define OTP_4TH_KEY_PROGRAMMED          	(1 << OTP_4TH_KEY_PROGRAMMED_BIT)
#define OTP_5TH_KEY_PROGRAMMED          	(1 << OTP_5TH_KEY_PROGRAMMED_BIT)

#define OTP_1ST_KEY_READ_LOCK_BIT       	22
#define OTP_2ND_KEY_READ_LOCK_BIT       	23
#define OTP_3RD_KEY_READ_LOCK_BIT       	24
#define OTP_4TH_KEY_READ_LOCK_BIT       	25
#define OTP_5TH_KEY_READ_LOCK_BIT       	26

#define OTP_1ST_KEY_READ_LOCK           	(1 << OTP_1ST_KEY_READ_LOCK_BIT)
#define OTP_2ND_KEY_READ_LOCK           	(1 << OTP_2ND_KEY_READ_LOCK_BIT)
#define OTP_3RD_KEY_READ_LOCK           	(1 << OTP_3RD_KEY_READ_LOCK_BIT)
#define OTP_4TH_KEY_READ_LOCK           	(1 << OTP_4TH_KEY_READ_LOCK_BIT)
#define OTP_5TH_KEY_READ_LOCK           	(1 << OTP_5TH_KEY_READ_LOCK_BIT)

#define OTP_NEW_KEY_RULE_ID_BIT         	16
#define OTP_NEW_KEY_RULE_ID             	(1 << OTP_NEW_KEY_RULE_ID_BIT)


#define OTP_1ST_KEY_BIT_START           	(0xF << 16)
#define OTP_2ND_KEY_BIT_START           	(0xF << 20)
#define OTP_3RD_KEY_BIT_START           	(0xF << 24)
#define OTP_4TH_KEY_BIT_START           	(0xF << 28)
#define OTP_4TH_KEY_BIT_START_NEW       	((0x1 << 28) | (0x7 << 9))  //original = bit[31..28] change to bit[11..9][28]
#define OTP_5TH_KEY_BIT_START           	(0xF << 12)


// System parameter usage
#define EFUSE_SUCCESS_PRI            	EFUSE_SUCCESS  // Operation success from primary field
#define EFUSE_SUCCESS_SEC            	1              // Operation success from secondary field

typedef struct {
	UINT32  uiTPDGroup;
	UINT32  uiStrobeWriteGroup;
	UINT32  uiStrobeReadGroup;
	UINT32  uiNormalTimingGroup;

} EFUSE_TIMING_GROUP;

BOOL quary_secure_boot(SECUREBOOT_STATUS scu_status);
BOOL enable_secure_boot(SECUREBOOT_STATUS scu_status);
BOOL quary_secure_boot_ep(UINT32 id, SECUREBOOT_STATUS scu_status);
BOOL enable_secure_boot_ep(UINT32 id, SECUREBOOT_STATUS scu_status);

BOOL cmd_efuse_show_avl(void);
BOOL cmd_efuse_show_avl_ep(UINT32 id);
/*
     efuse_readParamOps

     efuse get system parameter (trim data)

     @note for EFUSE_PARAM_DATA

     @param[in]     param   efuse system internal data field
     @param[out]     data   trim data(if success)

     @return
        - @b EFUSE_SUCCESS              success
        - @b EFUSE_UNKNOW_PARAM_ERR     unknow system internal data field
        - @b EFUSE_INVALIDATE_ERR       system internal data field invalidate
        - @b EFUSE_OPS_ERR              efuse operation error
*/
extern INT32    efuse_readParamOps(EFUSE_PARAM_DATA param, UINT16 *data);

/*
     efuse_read_param_ops_ep

     efuse read system parameter

     @note for EFUSE_PARAM_DATA

	 @param[in] 	   id  	EP ID
     @param[in]     param   efuse system internal data field
     @param[in]      data   data ready to programmed

     @return
        - @b EFUSE_SUCCESS              success
        - @b EFUSE_UNKNOW_PARAM_ERR     unknow system internal data field
        - @b EFUSE_INVALIDATE_ERR       system internal data field invalidate
        - @b EFUSE_OPS_ERR              efuse operation error
*/
extern INT32 efuse_read_param_ops_ep(UINT32 id, EFUSE_PARAM_DATA param, UINT16 *data);

/**
    otp_write_key_ep

    Write specific key into specific key set (0~4)

    @Note: key set 0 is for secure boot use
	@param[in] id  				EP ID
    @param[in] key_set_index   	key set (0~4)
    @param[in] ucKey           	key (16bytes)
    @return Description of data returned.
        - @b EFUSE_SUCCESS		: Success
        - @b EFUSE_OPS_ERR		: OTP operation error
        - @b EFUSE_FREEZE_ERR	: Key field already programmed
        - @b EFUSE_CONTENT_ERR	: Write key and read back and compare fail
*/
extern INT32 otp_write_key_ep(UINT32 ep_id, EFUSE_OTP_KEY_SET_FIELD key_set_index, UINT8 *uc_key);


/**
    otp_read_key

    Read specific key into specific key set (0~4)

    @Note: key set 0 is for secure boot use

	@param[in] id  				EP ID
    @param[in] key_set_index   	key set (0~3)
    @param[in] ucKey           	key (16bytes)
    @return Description of data returned.
        - @b EFUSE_SUCCESS		: Success
        - @b EFUSE_OPS_ERR		: OTP operation error
*/
extern INT32 otp_read_key_ep(UINT32 id, EFUSE_OTP_KEY_SET_FIELD key_set_index, UINT8 *uc_key);

/*
     efuse_check_available

     efuse check specific library can use @ specific package ID or not

     @param[in]   name   library name
     @param[in]   param  version code

     @return IC revision of specific package revision
        - @b   TRUE     Usable library
        - @b   FALSE    Un-usable library
*/
extern BOOL efuse_check_available2(const CHAR *name, UINT32 param);

/*
     efuse_check_available_extend

     efuse check extend specific functionality

     @param[in]   param1   extend ability list
     @param[in]   param2   specific param of ability
     @param[in]   param3   version code

     @return IC revision of specific package revision
        - @b   TRUE     support
        - @b   FALSE    not support
*/
extern BOOL efuse_check_available_extend2(EFUSE_PKG_ABILITY_LIST param1, UINT32 param2, UINT32 param3);

/*
     efuse_check_available_extend_ep

     efuse check extend specific functionality

     @param[in]   param1   extend ability list
     @param[in]   param2   specific param of ability

     @return IC revision of specific package revision
        - @b   TRUE     support
        - @b   FALSE    not support
*/
extern BOOL efuse_check_available_extend_ep(UINT32 ep_id, EFUSE_PKG_ABILITY_LIST param1, UINT32 param2);
/*
     efuse_get_unique_id

     efuse get unique id (56bits)

     @param[out]     id_L   LSB of ID (32bit)
     @param[out]     id_H   MSB of ID (17bit)

     @return IC success or fail
        - @b   	EFUSE_SUCCESS      	success
        - @b	EFUSE_OPS_ERR		fail
*/
extern ER efuse_get_unique_id(UINT32 *id_L, UINT32 *id_H);

/*
     efuse_get_unique_id_ep

     efuse get efuse_get_unique_id_ep id (56bits)

	 @param[in]      ep_id  EP ID
     @param[out]     id_L   LSB of ID (32bit)
     @param[out]     id_H   MSB of ID (17bit)

     @return IC success or fail
        - @b   	EFUSE_SUCCESS      	success
        - @b	EFUSE_OPS_ERR		fail
*/
ER efuse_get_unique_id_ep(UINT32 ep_id, UINT32 *id_L, UINT32 *id_H);

/**
    otp_set_key_read_lock

    Once otp_set_key_read_lock set, this key set field will not allow read value by CPU

    (Only can operate by key manager)

    @Note: key set 0 is for secure boot use

    @param[in] key_set_index   key set (0~4)
    @return Description of data returned.
        - @b EFUSE_SUCCESS		: Success
        - @b EFUSE_OPS_ERR		: OTP operation error
*/
extern INT32 otp_set_key_read_lock(EFUSE_OTP_KEY_SET_FIELD key_set_index);

/**
    otp_set_key_read_lock_ep

    Once otp_set_key_read_lock_ep set, this key set field will not allow read value by CPU

    (Only can operate by key manager)

    @Note: key set 0 is for secure boot use

	@param[in] ep_id  			EP ID
    @param[in] key_set_index   	key set (0~4)
    @return Description of data returned.
        - @b EFUSE_SUCCESS		: Success
        - @b EFUSE_OPS_ERR		: OTP operation error
*/
extern INT32 otp_set_key_read_lock_ep(UINT32 ep_id, EFUSE_OTP_KEY_SET_FIELD key_set_index);

/**
    otp_write_key

    Write specific key into specific key set (0~4)

    @Note: key set 0 is for secure boot use

    @param[in] key_set_index   key set (0~4)
    @param[in] ucKey           key (16bytes)
    @return Description of data returned.
        - @b EFUSE_SUCCESS		: Success
        - @b EFUSE_OPS_ERR		: OTP operation error
        - @b EFUSE_FREEZE_ERR	: Key field already programmed
        - @b EFUSE_CONTENT_ERR	: Write key and read back and compare fail
*/
extern INT32 otp_write_key(EFUSE_OTP_KEY_SET_FIELD key_set_index, UINT8 *uc_key);

/**
    otp_read_key

    Read specific key into specific key set (0~4)

    @Note: key set 0 is for secure boot use

    @param[in] key_set_index   key set (0~3)
    @param[in] ucKey           key (16bytes)
    @return Description of data returned.
        - @b EFUSE_SUCCESS		: Success
        - @b EFUSE_OPS_ERR		: OTP operation error
*/
extern INT32 otp_read_key(EFUSE_OTP_KEY_SET_FIELD key_set_index, UINT8 *uc_key);

/**
    trigger_efuse_key

    Durung encrypt or decrypt, configure specific key set as AES key(0~4)

    @Note: key set 0 is for secure boot use (This API is for crypto framework use)

	@param[in]	key_dst			Crypto engone / RSA / Hash engine
    @param[in] 	key_word_ofs   	No. of key word offset index(total 20 words->640 bits => 5 sets of keys)
    @param[in]  key_word_cnt	word count of key(AES128 = 4 / AES256 = 8)
    @return Description of data returned.
        - @b E_OK:   					Success
        - @b EFUSE_OPS_ERR:				Operation error
        - @b EFUSE_UNKNOW_PARAM_ERR:	Param error
*/
extern INT32 trigger_efuse_key(EFUSE_KEY_MANAGER_DST key_dst, UINT32 key_word_ofs, UINT32 key_word_cnt);

/*
     otp_version

     otp_version (56bits)
*/
extern void otp_version(void);

/*
     otp_version_ep

     otp_version_ep (56bits)
*/
extern void otp_version_ep(void);

/**
    otp_secure_en

    Enable secure boot (Rom treat loader as secure boot flow)
*/
#define otp_secure_en()						enable_secure_boot(SECUREBOOT_SECURE_EN)

/**
    otp_data_area_encrypt_en

    ROM treat loader data area as cypher text enable
*/
#define otp_data_area_encrypt_en()			enable_secure_boot(SECUREBOOT_DATA_AREA_ENCRYPT)

/**
    otp_signature_rsa_en

    ROM treat loader by use RSA as loader's signature
*/
#define otp_signature_rsa_en()				enable_secure_boot(SECUREBOOT_SIGN_RSA)

/**
    otp_signature_rsa_chksum_en

    Once use RSA as signature, enable RSA public checksum(use SHA256) enable
*/

#define otp_signature_rsa_chksum_en()		enable_secure_boot(SECUREBOOT_SIGN_RSA_CHK)

/**
    otp_jtag_dis

    Disable JTAG

    @Note: Can not re enable once disabled
*/
#define otp_jtag_dis()						enable_secure_boot(SECUREBOOT_JTAG_DISABLE_EN)

//Query API collection
#define is_secure_enable()					quary_secure_boot(SECUREBOOT_SECURE_EN)
#define is_data_area_encrypted()			quary_secure_boot(SECUREBOOT_DATA_AREA_ENCRYPT)
#define is_signature_rsa()					quary_secure_boot(SECUREBOOT_SIGN_RSA)
#define is_signature_rsa_chsum_enable()		quary_secure_boot(SECUREBOOT_SIGN_RSA_CHK)
#define is_signature_aes()					!quary_secure_boot(SECUREBOOT_SIGN_RSA)
#define is_JTAG_DISABLE_en()				quary_secure_boot(SECUREBOOT_JTAG_DISABLE_EN)

#define is_1st_key_programmed()				quary_secure_boot(SECUREBOOT_1ST_KEY_SET_PROGRAMMED)
#define is_2nd_key_programmed()				quary_secure_boot(SECUREBOOT_2ND_KEY_SET_PROGRAMMED)
#define is_3rd_key_programmed()				quary_secure_boot(SECUREBOOT_3RD_KEY_SET_PROGRAMMED)
#define is_4th_key_programmed()				quary_secure_boot(SECUREBOOT_4TH_KEY_SET_PROGRAMMED)
#define is_5th_key_programmed()				quary_secure_boot(SECUREBOOT_5TH_KEY_SET_PROGRAMMED)

#define is_1st_key_read_lock()				quary_secure_boot(SECUREBOOT_1ST_KEY_SET_READ_LOCK)
#define is_2nd_key_read_lock()				quary_secure_boot(SECUREBOOT_2ND_KEY_SET_READ_LOCK)
#define is_3rd_key_read_lock()				quary_secure_boot(SECUREBOOT_3RD_KEY_SET_READ_LOCK)
#define is_4th_key_read_lock()				quary_secure_boot(SECUREBOOT_4TH_KEY_SET_READ_LOCK)
#define is_5th_key_read_lock()				quary_secure_boot(SECUREBOOT_5TH_KEY_SET_READ_LOCK)
#define is_new_key_rule()					quary_secure_boot(SECUREBOOT_NEW_KEY_RULE)


//EP API
/**
    otp_jtag_dis

    Disable JTAG

    @Note: Can not re enable once disabled
*/
#define otp_jtag_dis_ep(id)						quary_secure_boot_ep(id, SECUREBOOT_JTAG_DISABLE_EN)

//Query API collection
#define is_secure_enable_ep(id)					quary_secure_boot_ep(id, SECUREBOOT_SECURE_EN)
#define is_data_area_encrypted_ep(id)			quary_secure_boot_ep(id, SECUREBOOT_DATA_AREA_ENCRYPT)
#define is_signature_rsa_ep(id)					quary_secure_boot_ep(id, SECUREBOOT_SIGN_RSA)
#define is_signature_rsa_chsum_enable_ep(id)	quary_secure_boot_ep(id, SECUREBOOT_SIGN_RSA_CHK)
#define is_signature_aes_ep(id)					!quary_secure_boot_ep(id, SECUREBOOT_SIGN_RSA)
#define is_JTAG_DISABLE_en_ep(id)				quary_secure_boot_ep(id, SECUREBOOT_JTAG_DISABLE_EN)

#define is_1st_key_programmed_ep(id)			quary_secure_boot_ep(id, SECUREBOOT_1ST_KEY_SET_PROGRAMMED)
#define is_2nd_key_programmed_ep(id)			quary_secure_boot_ep(id, SECUREBOOT_2ND_KEY_SET_PROGRAMMED)
#define is_3rd_key_programmed_ep(id)			quary_secure_boot_ep(id, SECUREBOOT_3RD_KEY_SET_PROGRAMMED)
#define is_4th_key_programmed_ep(id)			quary_secure_boot_ep(id, SECUREBOOT_4TH_KEY_SET_PROGRAMMED)
#define is_5th_key_programmed_ep(id)			quary_secure_boot_ep(id, SECUREBOOT_5TH_KEY_SET_PROGRAMMED)

#define is_1st_key_read_lock_ep(id)				quary_secure_boot_ep(id, SECUREBOOT_1ST_KEY_SET_READ_LOCK)
#define is_2nd_key_read_lock_ep(id)				quary_secure_boot_ep(id, SECUREBOOT_2ND_KEY_SET_READ_LOCK)
#define is_3rd_key_read_lock_ep(id)				quary_secure_boot_ep(id, SECUREBOOT_3RD_KEY_SET_READ_LOCK)
#define is_4th_key_read_lock_ep(id)				quary_secure_boot_ep(id, SECUREBOOT_4TH_KEY_SET_READ_LOCK)
#define is_5th_key_read_lock_ep(id)				quary_secure_boot_ep(id, SECUREBOOT_5TH_KEY_SET_READ_LOCK)
#define is_new_key_rule_ep(id)					quary_secure_boot_ep(id, SECUREBOOT_NEW_KEY_RULE)
#endif

