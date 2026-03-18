/*
    Novatek protected header file of NT96660's driver.

    The header file for Novatek protected APIs of NT96660's driver.

    @file       efuse_protected.h
    @ingroup    mIDriver
    @note       For Novatek internal reference, don't export to agent or customer.

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef _NVT_EFUSE_PROTECTED_H
#define _NVT_EFUSE_PROTECTED_H

#include <kwrap/nvt_type.h>

typedef enum {
	SCE_NO_0_KEY_SET_FIELD = 0x0,
	SCE_NO_1_KEY_SET_FIELD,
	SCE_NO_2_KEY_SET_FIELD,
	SCE_NO_3_KEY_SET_FIELD,
	SCE_NO_4_KEY_SET_FIELD,
	SCE_NO_5_KEY_SET_FIELD,
	SCE_NO_6_KEY_SET_FIELD,
	SCE_NO_7_KEY_SET_FIELD,


	SCE_NO_8_KEY_SET_FIELD,
	SCE_NO_9_KEY_SET_FIELD,
	SCE_NO_10_KEY_SET_FIELD,
	SCE_NO_11_KEY_SET_FIELD,

	SCE_NO_12_KEY_SET_FIELD,
	SCE_NO_13_KEY_SET_FIELD,
	SCE_NO_14_KEY_SET_FIELD,
	SCE_NO_15_KEY_SET_FIELD,

	EFUSE_TOTAL_KEY_SET_FIELD,

	EFUSE_CRYPTO_ENGINE_KEY_CNT = SCE_NO_8_KEY_SET_FIELD,
	EFUSE_RSA_ENGINE_KEY_CNT = EFUSE_TOTAL_KEY_SET_FIELD,
	EFUSE_HASH_ENGINE_KEY_CNT = EFUSE_TOTAL_KEY_SET_FIELD,

	ENUM_DUMMY4WORD(SCE_KEY_SET_TO_OTP_FIELD)
} SCE_KEY_SET_TO_OTP_FIELD;


typedef enum {
	EFUSE_OTP_1ST_KEY_SET_FIELD     = 0x0,        // This if for secure boot
	EFUSE_OTP_2ND_KEY_SET_FIELD     = 0x1,
	EFUSE_OTP_3RD_KEY_SET_FIELD     = 0x2,
	EFUSE_OTP_4TH_KEY_SET_FIELD     = 0x3,
	EFUSE_OTP_5TH_KEY_SET_FIELD     = 0x4,
	EFUSE_OTP_TOTAL_KEY_SET_FIELD   = 0x5,
} EFUSE_OTP_KEY_SET_FIELD;

STATIC_ASSERT(EFUSE_OTP_TOTAL_KEY_SET_FIELD <= 5);

#define EFUSE_OTP_KEY_FIELD_CNT         4

typedef enum {
	OTP_KEY_MANAGER_NONE = 0x0,                 // Not process key manager
	OTP_KEY_MANAGER_CRYPTO,                     // Key manager, destination is crypto engine(SCE)
	OTP_KEY_MANAGER_RSA,                        // Key manager, destination is RSA engine
	OTP_KEY_MANAGER_HASH,                       // Key manager, destination is HASH engine
	OTP_KEY_MANAGER_CNT,
} OTP_KEY_DESTINATION;

#if defined(_BSP_NA51102_) || defined(_BSP_NA51103_) || defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02301_)
#define KEY_MANAGER_CONTROL_ADDRESS             0x00000020
#define KEY_MANAGER_STATUS_ADDRESS              0x00000030
#define KEY_MANAGER_TIMING_CONFIG_ADDRESS       0x00000010
#define KEY_MANAGER_DESTINATION_ADDRESS         0x00000014
#define KEY_MANAGER_KEY_ENGINE_RIGHT_ADDRESS    0x00000040
#define KEY_MANAGER_KEY_ENGINE_RIGHT2_ADDRESS   0x00000044
#else
#define KEY_MANAGER_CONTROL_ADDRESS             0x00000020
#define KEY_MANAGER_STATUS_ADDRESS              0x00000030
#define KEY_MANAGER_TIMING_CONFIG_ADDRESS       0x00000010
#define KEY_MANAGER_DESTINATION_ADDRESS         0x00000040
#define KEY_MANAGER_KEY_INDEX_ADDRESS           0x00000044
#define KEY_MANAGER_KEY_ENGINE_RIGHT_ADDRESS    0x0000004C
#endif

/**
    Crypto engine check
*/
typedef enum {
	SECUREBOOT_SECURE_EN = 0x00,        ///< Quary if secure enable or not
	SECUREBOOT_DATA_AREA_ENCRYPT,       ///< Quary if data area encrypt to cypher text or not
	SECUREBOOT_SIGN_RSA,                ///< Quary if Signature methed is RSA or not(AES)
	SECUREBOOT_SIGN_RSA_CHK,            ///< Quary if Signature hash checksum RSA key correct or not
	SECUREBOOT_JTAG_DISABLE_EN,         ///< Quary if JTAG is disable or not(TRUE : disable)
#if defined(_BSP_NS02302_) || defined(_BSP_NS02301_)
	SECUREBOOT_SIGN_ECDSA,				///< Quary if Signature methed is ECDSA (once RSA ECDSA enable at the same time=> ECDSA priority high)
#endif
	SECUREBOOT_1ST_KEY_SET_PROGRAMMED,  ///< Quary if 1st key set programmed or not
	SECUREBOOT_2ND_KEY_SET_PROGRAMMED,  ///< Quary if 2nd key set programmed or not
	SECUREBOOT_3RD_KEY_SET_PROGRAMMED,  ///< Quary if 3rd key set programmed or not
	SECUREBOOT_4TH_KEY_SET_PROGRAMMED,  ///< Quary if 4th key set programmed or not
	SECUREBOOT_5TH_KEY_SET_PROGRAMMED,  ///< Quary if 5th key set programmed or not

	SECUREBOOT_READ_LOCK_KEY_SET_START,
	SECUREBOOT_1ST_KEY_SET_READ_LOCK = SECUREBOOT_READ_LOCK_KEY_SET_START,  ///< Quary if 1st key read lock already or not
	SECUREBOOT_2ND_KEY_SET_READ_LOCK,                                       ///< Quary if 2nd key read lock already or not
	SECUREBOOT_3RD_KEY_SET_READ_LOCK,                                       ///< Quary if 3rd key read lock already or not
	SECUREBOOT_4TH_KEY_SET_READ_LOCK,                                       ///< Quary if 4th key read lock already or not
	SECUREBOOT_5TH_KEY_SET_READ_LOCK,                                       ///< Quary if 5th key read lock already or not
	SECUREBOOT_READ_LOCK_KEY_SET_END = SECUREBOOT_5TH_KEY_SET_READ_LOCK,

	SECUREBOOT_DSP1_DISABLE,
	SECUREBOOT_DSP2_DISABLE,
	SECUREBOOT_GPU_DISABLE,
	SECUREBOOT_MCU_DISABLE,
	SECUREBOOT_STATUS_NUM,
	//NT98539A
	SECUREBOOT_SIGN_RSA_PSS,			///RSAPKCS2.1
	SECUREBOOT_SIGN_RSA_PKCS1P5,		///RSAPKCS1.5
	SECUREBOOT_HEADER_SIGNATURE,		
	//NT98539A

	SECUREBOOT_HDMI_AVAILABLE,          ///< Quary if HDMI is disabled
} SECUREBOOT_STATUS;

#ifndef TRUE
#define TRUE                                    1
#endif
#ifndef FALSE
#define FALSE                                   0
#endif
#define E_OK                                    0
#define EFUSE_SUCCESS                           E_OK
#define EFUSE_FREEZE_ERR                        -1001          // Programmed already, only can read
#define EFUSE_INACTIVE_ERR                      -1002          // This field is empty(not programmed yet)
#define EFUSE_INVALIDATE_ERR                    -1003          // This field force invalidate already
#define EFUSE_UNKNOW_PARAM_ERR                  -1004          // efuse param field not defined
#define EFUSE_OPS_ERR                           -1005          // efuse operation error
#define EFUSE_SECURITY_ERR                      -1006          // efuse under security mode => can not read back
#define EFUSE_PARAM_ERR                         -1007          // efuse param error
#define EFUSE_CONTENT_ERR                       -1008          // efuse operation error
#define EFUSE_POWER_ON_OFF_CALLBACK_ERR		    -1009			// efuse program power callback not define

#define OTP_HW_SECURE_EN                        (1 << 0)
#define OTP_FW_SECURE_EN                        (1 << 5)
#define OTP_DATA_ENCRYPT_EN                     (1 << 7)
#define OTP_SIGNATURE_RSA                       (1 << 1)
#define OTP_SIGNATURE_RSA_CHK_EN                (1 << 3)
#define OTP_SIGNATURE_ECDSA_CHKSUM_EN       	OTP_SIGNATURE_RSA_CHK_EN
#define OTP_JTAG_DISABLE_EN                     (1 << 2)
#define EFUSE_HDMI_USAGE_BIT            	    11
#define EFUSE_HDMI_USAGE_BIT_MASK       	    (0x7<<11)

#define OTP_1ST_KEY_PROGRAMMED_BIT              27
#define OTP_2ND_KEY_PROGRAMMED_BIT              28
#define OTP_3RD_KEY_PROGRAMMED_BIT              29
#define OTP_4TH_KEY_PROGRAMMED_BIT              30
#define OTP_5TH_KEY_PROGRAMMED_BIT              31

#if defined(_BSP_NS02302_) || defined(_BSP_NS02301_)
#define OTP_SIGNATURE_RSAPSS_BIT           		16
#define OTP_SIGNATURE_ECDSA_BIT					17
#define OTP_SIGNATURE_ECDSA              		(1 << OTP_SIGNATURE_ECDSA_BIT)
#else
#define OTP_DSP1_DISABLE_BIT                    17
#define OTP_DSP2_DISABLE_BIT                    9
#define OTP_GPU_DISABLE_BIT                     16
#define OTP_MCU_DISABLE_BIT                     0
#define HDMI_1ST_AVAILABLE              	    (0)
#define HDMI_1ST_DISABLE                	    (0x1<<EFUSE_HDMI_USAGE_BIT)
#define HDMI_2ND_AVAILABLE              	    (0x3<<EFUSE_HDMI_USAGE_BIT)
#define HDMI_2ND_DISABLE                	    (0x7<<EFUSE_HDMI_USAGE_BIT)
#endif

//2nd addr[9]
#if defined(_BSP_NS02302_)
#define OTP_HEADER_SIGNATURE_BIT				17
#define OTP_SIGNATURE_RSAPKCS1P5_BIT			16
#define OTP_SIGNATURE_RSAPSS                	(1 << OTP_SIGNATURE_RSAPSS_BIT)
#endif

#define OTP_1ST_KEY_PROGRAMMED                  (1 << OTP_1ST_KEY_PROGRAMMED_BIT)
#define OTP_2ND_KEY_PROGRAMMED                  (1 << OTP_2ND_KEY_PROGRAMMED_BIT)
#define OTP_3RD_KEY_PROGRAMMED                  (1 << OTP_3RD_KEY_PROGRAMMED_BIT)
#define OTP_4TH_KEY_PROGRAMMED                  (1 << OTP_4TH_KEY_PROGRAMMED_BIT)
#define OTP_5TH_KEY_PROGRAMMED                  (1 << OTP_5TH_KEY_PROGRAMMED_BIT)
#define OTP_DSP1_DISABLE                        (1 << OTP_DSP1_DISABLE_BIT)
#define OTP_DSP2_DISABLE                        (1 << OTP_DSP2_DISABLE_BIT)
#define OTP_GPU_DISABLE                         (1 << OTP_GPU_DISABLE_BIT)
#define OTP_MCU_DISABLE                         (1 << OTP_MCU_DISABLE_BIT)
#define OTP_1ST_KEY_READ_LOCK_BIT               22
#define OTP_2ND_KEY_READ_LOCK_BIT               23
#define OTP_3RD_KEY_READ_LOCK_BIT               24
#define OTP_4TH_KEY_READ_LOCK_BIT               25
#define OTP_5TH_KEY_READ_LOCK_BIT               26

#define OTP_1ST_KEY_READ_LOCK                   (1 << OTP_1ST_KEY_READ_LOCK_BIT)
#define OTP_2ND_KEY_READ_LOCK                   (1 << OTP_2ND_KEY_READ_LOCK_BIT)
#define OTP_3RD_KEY_READ_LOCK                   (1 << OTP_3RD_KEY_READ_LOCK_BIT)
#define OTP_4TH_KEY_READ_LOCK                   (1 << OTP_4TH_KEY_READ_LOCK_BIT)
#define OTP_5TH_KEY_READ_LOCK                   (1 << OTP_5TH_KEY_READ_LOCK_BIT)


#define OTP_SIGNATURE_RSAPKCS1P5				(1 << OTP_SIGNATURE_RSAPKCS1P5_BIT)	//bit[16]
#define OTP_HEADER_SIGNATURE_EN					(1 << OTP_HEADER_SIGNATURE_BIT)		//bit[17]

#define OTP_1ST_KEY_BIT_START                   (0xF << 16)
#define OTP_2ND_KEY_BIT_START                   (0xF << 20)
#define OTP_3RD_KEY_BIT_START                   (0xF << 24)
#define OTP_4TH_KEY_BIT_START                   (0xF << 28)
#define OTP_5TH_KEY_BIT_START                   (0xF << 12)

#if defined(_BSP_NS02401_)
/*
    @name eFuse enum - base address
*/
//@{
typedef enum {
	EFUSE_ID_0 = 0,   	///< efuse ID 0
	EFUSE_ID_1 = 1,     ///< efuse ID 1
} EFUSE_ENGINE;
//@}
#endif

#define is_secure_enable()                      quary_secure_boot(SECUREBOOT_SECURE_EN)
#define is_data_area_encrypted()                quary_secure_boot(SECUREBOOT_DATA_AREA_ENCRYPT)
#define is_signature_rsa()                      quary_secure_boot(SECUREBOOT_SIGN_RSA)
#define is_signature_rsa_chsum_enable()         quary_secure_boot(SECUREBOOT_SIGN_RSA_CHK)
#define is_signature_aes()                      !quary_secure_boot(SECUREBOOT_SIGN_RSA)
#define is_JTAG_DISABLE_en()                    quary_secure_boot(SECUREBOOT_JTAG_DISABLE_EN)

#if defined(_BSP_NS02302_) || defined(_BSP_NS02301_)
#define is_signature_ecdsa()                	quary_secure_boot(SECUREBOOT_SIGN_ECDSA)
#define is_signature_chsum_enable()     		quary_secure_boot(SECUREBOOT_SIGN_RSA_CHK)
#endif

#define is_1st_key_programmed()                 quary_secure_boot(SECUREBOOT_1ST_KEY_SET_PROGRAMMED)
#define is_2nd_key_programmed()                 quary_secure_boot(SECUREBOOT_2ND_KEY_SET_PROGRAMMED)
#define is_3rd_key_programmed()                 quary_secure_boot(SECUREBOOT_3RD_KEY_SET_PROGRAMMED)
#define is_4th_key_programmed()                 quary_secure_boot(SECUREBOOT_4TH_KEY_SET_PROGRAMMED)
#define is_5th_key_programmed()                 quary_secure_boot(SECUREBOOT_5TH_KEY_SET_PROGRAMMED)


#define is_1st_key_read_lock()                  quary_secure_boot(SECUREBOOT_1ST_KEY_SET_READ_LOCK)
#define is_2nd_key_read_lock()                  quary_secure_boot(SECUREBOOT_2ND_KEY_SET_READ_LOCK)
#define is_3rd_key_read_lock()                  quary_secure_boot(SECUREBOOT_3RD_KEY_SET_READ_LOCK)
#define is_4th_key_read_lock()                  quary_secure_boot(SECUREBOOT_4TH_KEY_SET_READ_LOCK)
#define is_5th_key_read_lock()                  quary_secure_boot(SECUREBOOT_5TH_KEY_SET_READ_LOCK)
#define is_hdmi_available()                 	quary_secure_boot(SECUREBOOT_HDMI_AVAILABLE)
#define is_DSP1_available()                 	!quary_secure_boot(SECUREBOOT_DSP1_DISABLE)
#define is_DSP2_available()                 	!quary_secure_boot(SECUREBOOT_DSP2_DISABLE)
#define is_GPU_available()                  	!quary_secure_boot(SECUREBOOT_GPU_DISABLE)
#define is_MCU_available()                  	!quary_secure_boot(SECUREBOOT_MCU_DISABLE)
#if defined(_BSP_NA51090_)
#define is_new_key_rule()						FALSE
#endif

#define is_hdmi_disable()                 		!quary_secure_boot(SECUREBOOT_HDMI_AVAILABLE)
#define is_DSP1_disable()                       quary_secure_boot(SECUREBOOT_DSP1_DISABLE)
#define is_DSP2_disable()                       quary_secure_boot(SECUREBOOT_DSP2_DISABLE)
#define is_GPU_disable()                        quary_secure_boot(SECUREBOOT_GPU_DISABLE)
#define is_MCU_disable()                        quary_secure_boot(SECUREBOOT_MCU_DISABLE)

extern void     otp_init(void);
extern void     otp_showinfo(void);
/**
    otp_set_key_destination

    Durung encrypt or decrypt, configure specific key set as AES key(0~3)

    @Note: key set 0 is for secure boot use

    @param[in]  key_dst         Crypto engone / RSA / Hash engine
    @param[in]  key_word_ofs    No. of key word offset index(total 20 words->640 bits => 5 sets of keys)
    @param[in]  key_word_cnt    word count of key(AES128 = 4 / AES256 = 8)
    @return Description of data returned.
        - @b E_OK:                      Success
        - @b EFUSE_OPS_ERR:             Operation error
        - @b EFUSE_UNKNOW_PARAM_ERR:    Param error
*/
extern INT32    otp_set_key_destination(OTP_KEY_DESTINATION key_dst, UINT32 key_word_ofs, UINT32 key_word_cnt);
extern INT32    otp_set_key_engine_access_right(EFUSE_OTP_KEY_SET_FIELD key_set_index);
extern INT32    otp_write_key(EFUSE_OTP_KEY_SET_FIELD key_set_index, UINT8 *uc_key);
extern BOOL     quary_secure_boot(SECUREBOOT_STATUS scu_status);
extern BOOL     enable_secure_boot(SECUREBOOT_STATUS scu_status);
extern UINT32   otp_key_manager(UINT32 rowAddress);

/**
    efuse_secure_en

    Enable secure boot
*/
#define efuse_secure_en()               enable_secure_boot(SECUREBOOT_SECURE_EN)
/**
    efuse_jtag_dis

    Disable JTAG

    @Note: Can not re enable once disabled
*/
#if defined(_BSP_NS02302_) || defined(_BSP_NS02301_)
#define efuse_signature_ecdsa_en()      enable_secure_boot(SECUREBOOT_SIGN_ECDSA)
#define efuse_signature_chksum_en() 	enable_secure_boot(SECUREBOOT_SIGN_RSA_CHK)
#else
#define efuse_DSP1_dis()                enable_secure_boot(SECUREBOOT_DSP1_DISABLE)
#define efuse_DSP2_dis()                enable_secure_boot(SECUREBOOT_DSP2_DISABLE)
#define efuse_GPU_dis()                 enable_secure_boot(SECUREBOOT_GPU_DISABLE)
#define efuse_MCU_dis()                 enable_secure_boot(SECUREBOOT_MCU_DISABLE)
#endif
#define efuse_jtag_dis()                enable_secure_boot(SECUREBOOT_JTAG_DISABLE_EN)
#define efuse_data_area_encrypt_en()    enable_secure_boot(SECUREBOOT_DATA_AREA_ENCRYPT)
#define efuse_signature_rsa_en()        enable_secure_boot(SECUREBOOT_SIGN_RSA)
#define efuse_signature_rsa_chksum_en() enable_secure_boot(SECUREBOOT_SIGN_RSA_CHK)

#endif

