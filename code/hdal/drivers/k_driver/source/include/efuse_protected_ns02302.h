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

#include <kwrap/error_no.h>
#include <kwrap/nvt_type.h>
#if defined(__FREERTOS)
#include "plat/pll.h"
#endif

#define DRV_PKG_IDBIT_MAX   32                      //Total 32 sets package type

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
	EFUSE_NORMAL_ARRAY_SEL = 0x0,                   //Array function
	EFUSE_TEST_FUNCTION_SEL,                        //Test mode

	ENUM_DUMMY4WORD(EFUSE_TEST_MODE_SEL)
} EFUSE_TEST_MODE_SEL;


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
	EFUSE_TEST_ARRAY_FUNCTION_SEL = 0x0,                 //Arrary function selected
	EFUSE_TEST_FUNCTIONAL_SEL,                  //Redundancy function selected

	ENUM_DUMMY4WORD(EFUSE_TEST_FUNC_SEL)

} EFUSE_TEST_FUNC_SEL;

typedef enum {
	EFUSE_INFO_DEGREE_0 = 0x0,                      // Only display efuse 512 bits
	EFUSE_INFO_DEGREE_1,                            // Display efuse 512 bits and translate field defint
	EFUSE_INFO_DEGREE_CNT,

	ENUM_DUMMY4WORD(EFUSE_INFO_DEGREE)
} EFUSE_INFO_DEGREE;

typedef enum {
	EFUSE_KEY_MANAGER_NORMAL = 0x0,                 // Normal operation
	EFUSE_KEY_MANAGER_CRYPTO,                       // to destination crypto engine
	EFUSE_KEY_MANAGER_RSA,                          // to destination RSA engine
	EFUSE_KEY_MANAGER_HASH,                         // to destination HASH engine

	EFUSE_KEY_MANAGER_DST_CNT,
	ENUM_DUMMY4WORD(EFUSE_KEY_MANAGER_DST)
} EFUSE_KEY_MANAGER_DST;

typedef enum {
	EFUSE_MACRO_A_SEL           = 0x1,              //MACRO A select
	EFUSE_MACRO_B_SEL           = 0x2,              //MACRO B select
	EFUSE_MACRO_DOUBLE_BIT_SEL  = 0x3,              //Double bit select

	ENUM_DUMMY4WORD(EFUSE_MACRO_SEL)
} EFUSE_MACRO_SEL;

typedef enum {
 #if defined(_NVT_EMULATION_)
	NT98538_PKG= 0x00,                             	// NT98538   package version
	NT96538_PKG,                             		// NT98538   package version
	NT98538_I_PKG,             						// NT98538 package version
	NT98538_Q_PKG,             						// NT98538 package version
	NT96538_I_PKG,             						// NT98538 package version
	NT96538_Q_PKG,             						// NT96538_Q package version

	NT98539_PKG,
	NT96539_PKG,

	NT98539_I_PKG,
	NT98539_Q_PKG,
	NT96539_I_PKG,
	NT96539_Q_PKG,

	NT98539A_PKG,
	NT98539A_I_PKG,
	NT98539A_Q_PKG,

	NT98538A_PKG,
	NT98538A_I_PKG,
	NT98538A_Q_PKG,

	NTDump_eFuse,
	NT98XXX_PKG_CNT = 19,
#else
	NT98538_PKG 	= 0x7C3E,                       // NT98538   package version
	NT96538_PKG 	= 0x7C5D,                       // NT98538   package version
	NT98538_I_PKG 	= 0x6C3E,             			// NT98538 package version
	NT98538_Q_PKG 	= 0x5C3E,             			// NT98538 package version
	NT96538_I_PKG 	= 0x6C5D,             			// NT98538 package version
	NT96538_Q_PKG 	= 0x5C5D,             			// NT96538_Q package version
	
	NT98539_PKG     = 0x7C7C,               		// NT98539 package version
	NT98539A_PKG    = 0x7CBA,               		// NT98539A package version
	NT96539_PKG     = 0x7C9B,               		// NT96539 package version

	NT98539_I_PKG   = 0x6C7C,               		// NT98539 WT package version
	NT98539_Q_PKG   = 0x5C7C,               		// NT98539 AECQ100 package version
	NT96539_I_PKG   = 0x6C9B,               		// NT96539 WT package version
	NT96539_Q_PKG   = 0x5C9B,               		// NT96539 AECQ100 package version

	NT98539A_I_PKG   = 0x6CBA,               		// NT98539 WT package version
	NT98539A_Q_PKG   = 0x5CBA,               		// NT98539 AECQ100 package version


	NT98XXX_PKG_CNT = 15,
#endif
	NT9853X_ENG_VER,                                // NT9853X Engineer version
	UNKNOWN_PKG_VER,                                // Unknow package version
	IC_PKG_CNT 		= NT98XXX_PKG_CNT,
	ENUM_DUMMY4WORD(NVT_PKG_UID)

} NVT_PKG_UID;
STATIC_ASSERT(IC_PKG_CNT <= DRV_PKG_IDBIT_MAX);

typedef enum {
	EFUSE_MASK_AREA_TYPE = 0x0,
	EFUSE_READ_ONLY_AREA_TYPE,

	ENUM_DUMMY4WORD(EFUSE_USER_AREA_TYPE)

} EFUSE_USER_AREA_TYPE;

typedef enum {
	EFUSE_ETHERNET_TRIM_DATA = 0x100,
	EFUSE_USBC_TRIM_DATA,
	EFUSE_DDRP_TRIM_DATA,
	EFUSE_THERMAL_TRIM_DATA,

	EFUSE_MIPI_TX_RX_TRIM_DATA,

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
	EFUSE_ADC_TRIM_A_DATA,
	EFUSE_ADC_TRIM_B_DATA,

	EFUSE_PARAM_CNT,
	ENUM_DUMMY4WORD(EFUSE_PARAM_DATA)
} EFUSE_PARAM_DATA;

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

	EFUSE_TOTAL_KEY_SET_FIELD,

	EFUSE_CRYPTO_ENGINE_KEY_CNT = EFUSE_WRITE_NO_8_KEY_SET_FIELD,
	EFUSE_RSA_ENGINE_KEY_CNT = EFUSE_TOTAL_KEY_SET_FIELD,
	EFUSE_HASH_ENGINE_KEY_CNT = EFUSE_TOTAL_KEY_SET_FIELD,

	ENUM_DUMMY4WORD(EFUSE_WRITE_KEY_SET_TO_OTP_FIELD)
} EFUSE_WRITE_KEY_SET_TO_OTP_FIELD;


typedef enum {
	EFUSE_WRITE_1ST_KEY_SET_FIELD = EFUSE_WRITE_NO_0_KEY_SET_FIELD,
	EFUSE_WRITE_2ND_KEY_SET_FIELD = EFUSE_WRITE_NO_1_KEY_SET_FIELD,
	EFUSE_WRITE_3RD_KEY_SET_FIELD = EFUSE_WRITE_NO_2_KEY_SET_FIELD,
	EFUSE_WRITE_4TH_KEY_SET_FIELD = EFUSE_WRITE_NO_3_KEY_SET_FIELD,
	EFUSE_TOTAL_KEY_SET,

	ENUM_DUMMY4WORD(EFUSE_WRITE_KEY_SET)
} EFUSE_WRITE_KEY_SET;

STATIC_ASSERT(EFUSE_TOTAL_KEY_SET_FIELD <= 16);

STATIC_ASSERT(EFUSE_RSA_ENGINE_KEY_CNT == 16);
STATIC_ASSERT(EFUSE_HASH_ENGINE_KEY_CNT == 16);
STATIC_ASSERT(EFUSE_CRYPTO_ENGINE_KEY_CNT == 8);

typedef enum {
	EFUSE_OTP_1ST_KEY_SET_FIELD = EFUSE_WRITE_NO_0_KEY_SET_FIELD,        // This if for secure boot
	EFUSE_OTP_2ND_KEY_SET_FIELD,
	EFUSE_OTP_3RD_KEY_SET_FIELD,
	EFUSE_OTP_4TH_KEY_SET_FIELD,
	EFUSE_OTP_5TH_KEY_SET_FIELD,
	EFUSE_OTP_TOTAL_KEY_SET_FIELD,
} EFUSE_OTP_KEY_SET_FIELD;

STATIC_ASSERT(EFUSE_OTP_TOTAL_KEY_SET_FIELD <= 5);
#define EFUSE_OTP_KEY_FIELD_CNT			4

typedef enum {
	EFUSE_CONFIG_MACRO_SEL = 0x1000,
	EFUSE_CONFIG_TIMING,
	EFUSE_CONFIG_GOLDEN_LSB,
	EFUSE_CONFIG_GOLDEN_MSB,
	EFUSE_CONFIG_DBG_SEL,
	ENUM_DUMMY4WORD(EFUSE_CONFIG_ID)
} EFUSE_CONFIG_ID;

typedef enum {
	EFUSE_TEST_ADDR_0 = 0x0,
	EFUSE_TEST_ADDR_1,
	EFUSE_TEST_ADDR_2,
	EFUSE_TEST_ADDR_3,
	ENUM_DUMMY4WORD(EFUSE_TEST_ADDR)
} EFUSE_TEST_ADDR;


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
	EFUSE_ABILITY_DSP               = 0x1007,       //< DSP exist or not
	EFUSE_ABILITY_CPU_PLL_FREQ      = 0x1008,       //<  CPU max freq
	EFUSE_ABILITY_CONV_PLL_FREQ     = 0x1009,       //< CONV max freq

	EFUSE_ABILITY_VPE_FREQ      	= 0x100A,       //<  VPE max freq
	EFUSE_ABILITY_LVDS_FREQ     	= 0x100B,       //< LVDS max freq
	EFUSE_ABILITY_ROU_LSU_CAL_PPU_POU_PLL_FREQ 	= 0x100C,       //< ROU LSU CAL PPU POU max freq
	EFUSE_ABILITY_CODEC_TURBO_MODE = 0x100D, //< Query if codec turbo mode 16MP30	

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
	SECUREBOOT_SIGN_ECDSA,				///< Quary if Signature methed is ECDSA (once RSA ECDSA enable at the same time=> ECDSA priority high)

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

	SECUREBOOT_CPU_OVER_CLOCKING,       ///< Quary if CPU can over clocking or not
	SECUREBOOT_CONV_OVER_CLOCKING,      ///< Quary if CONV can over clocking or not
	SECUREBOOT_DDR_OVER_CLOCKING,       ///< Quary if CPU can over clocking or not
	SECUREBOOT_VENC_OVER_CLOCKING,      ///< Quary if CONV can over clocking or not


	//NT98539A
	SECUREBOOT_VPE_OVER_CLOCKING,       ///< Quary if VPE can over clocking or not
	SECUREBOOT_LVDS_OVER_CLOCKING,      ///< Quary if LVDS can over clocking or not

	SECUREBOOT_SIGN_RSA_PSS,            ///RSAPKCS2.1
	SECUREBOOT_SIGN_RSA_PKCS1P5,        ///RSAPKCS1.5
	SECUREBOOT_HEADER_SIGNATURE,
	//NT98539A
} SECUREBOOT_STATUS;

#define NVT_EFUSE_VERSION                               0x22041508
#define efuse_check_available(name)                     efuse_check_available2(name, NVT_EFUSE_VERSION)
#define efuse_check_available_extend(param1, param2)    efuse_check_available_extend2(param1, param2, NVT_EFUSE_VERSION)

#define EFUSE_SUCCESS                   E_OK
#define EFUSE_FREEZE_ERR                -1001          // Programmed already, only can read
#define EFUSE_INACTIVE_ERR              -1002          // This field is empty(not programmed yet)
#define EFUSE_INVALIDATE_ERR            -1003          // This field force invalidate already
#define EFUSE_UNKNOW_PARAM_ERR          -1004          // efuse param field not defined
#define EFUSE_OPS_ERR                   -1005          // efuse operation error
#define EFUSE_SECURITY_ERR              -1006          // efuse under security mode => can not read back
#define EFUSE_PARAM_ERR                 -1007          // efuse param error
#define EFUSE_CONTENT_ERR               -1008          // efuse operation error
#define EFUSE_POWER_ON_OFF_CALLBACK_ERR -1009           // efuse program power callback not define
#define OTP_HW_SECURE_EN                (1 << 0)
#define OTP_FW_SECURE_EN                (1 << 5)
#define OTP_DATA_ENCRYPT_EN             (1 << 7)
#define OTP_SIGNATURE_RSA               (1 << 1)
#define OTP_SIGNATURE_RSA_CHK_EN        (1 << 3)
#define OTP_SIGNATURE_ECDSA_CHKSUM_EN   OTP_SIGNATURE_RSA_CHK_EN
#define OTP_JTAG_DISABLE_EN             (1 << 2)



#define OTP_1ST_KEY_PROGRAMMED_BIT      27
#define OTP_2ND_KEY_PROGRAMMED_BIT      28
#define OTP_3RD_KEY_PROGRAMMED_BIT      29
#define OTP_4TH_KEY_PROGRAMMED_BIT      30
#define OTP_5TH_KEY_PROGRAMMED_BIT      31

#define OTP_SIGNATURE_RSAPSS_BIT        16
#define OTP_SIGNATURE_ECDSA_BIT         17

//2nd addr[9]
#define OTP_HEADER_SIGNATURE_BIT        17
#define OTP_SIGNATURE_RSAPKCS1P5_BIT    16
#define OTP_DDR_OVER_CLOCKING_BIT       15
#define OTP_CPU_OVER_CLOCKING_BIT       14
#define OTP_CONV_OVER_CLOCKING_BIT      13
#define OTP_VENC_OVER_CLOCKING_BIT      12

//539A only
#define OTP_VPE_OVER_CLOCKING_BIT      	11
#define OTP_LVDS_OVER_CLOCKING_BIT      10

#define OTP_SIGNATURE_ECDSA             (1 << OTP_SIGNATURE_ECDSA_BIT)
#define OTP_SIGNATURE_RSAPSS            (1 << OTP_SIGNATURE_RSAPSS_BIT)

#define OTP_1ST_KEY_PROGRAMMED          (1 << OTP_1ST_KEY_PROGRAMMED_BIT)
#define OTP_2ND_KEY_PROGRAMMED          (1 << OTP_2ND_KEY_PROGRAMMED_BIT)
#define OTP_3RD_KEY_PROGRAMMED          (1 << OTP_3RD_KEY_PROGRAMMED_BIT)
#define OTP_4TH_KEY_PROGRAMMED          (1 << OTP_4TH_KEY_PROGRAMMED_BIT)
#define OTP_5TH_KEY_PROGRAMMED          (1 << OTP_5TH_KEY_PROGRAMMED_BIT)

#define OTP_1ST_KEY_READ_LOCK_BIT       22
#define OTP_2ND_KEY_READ_LOCK_BIT       23
#define OTP_3RD_KEY_READ_LOCK_BIT       24
#define OTP_4TH_KEY_READ_LOCK_BIT       25
#define OTP_5TH_KEY_READ_LOCK_BIT       26

#define OTP_1ST_KEY_READ_LOCK           (1 << OTP_1ST_KEY_READ_LOCK_BIT)
#define OTP_2ND_KEY_READ_LOCK           (1 << OTP_2ND_KEY_READ_LOCK_BIT)
#define OTP_3RD_KEY_READ_LOCK           (1 << OTP_3RD_KEY_READ_LOCK_BIT)
#define OTP_4TH_KEY_READ_LOCK           (1 << OTP_4TH_KEY_READ_LOCK_BIT)
#define OTP_5TH_KEY_READ_LOCK           (1 << OTP_5TH_KEY_READ_LOCK_BIT)

//2nd
//539A only
#define OTP_LVDS_OVER_CLOCKING      	(1 << OTP_LVDS_OVER_CLOCKING_BIT)	//bit[10]
#define OTP_VPE_OVER_CLOCKING      		(1 << OTP_VPE_OVER_CLOCKING_BIT)	//bit[11]
//539A only
#define OTP_VENC_OVER_CLOCKING          (1 << OTP_VENC_OVER_CLOCKING_BIT)   //bit[12]
#define OTP_CONV_OVER_CLOCKING          (1 << OTP_CONV_OVER_CLOCKING_BIT)   //bit[13]
#define OTP_CPU_OVER_CLOCKING           (1 << OTP_CPU_OVER_CLOCKING_BIT)    //bit[14]
#define OTP_DDR_OVER_CLOCKING           (1 << OTP_DDR_OVER_CLOCKING_BIT)    //bit[15]
#define OTP_SIGNATURE_RSAPKCS1P5        (1 << OTP_SIGNATURE_RSAPKCS1P5_BIT) //bit[16]
#define OTP_HEADER_SIGNATURE_EN         (1 << OTP_HEADER_SIGNATURE_BIT)     //bit[17]




#define OTP_1ST_KEY_BIT_START           (0xF << 16)
#define OTP_2ND_KEY_BIT_START           (0xF << 20)
#define OTP_3RD_KEY_BIT_START           (0xF << 24)
#define OTP_4TH_KEY_BIT_START           (0xF << 28)
#define OTP_5TH_KEY_BIT_START           (0xF << 12)


// System parameter usage
#define EFUSE_SUCCESS_PRI               EFUSE_SUCCESS  // Operation success from primary field
#define EFUSE_SUCCESS_SEC               1              // Operation success from secondary field

typedef struct {
	UINT32  uiTPDGroup;
	UINT32  uiStrobeWriteGroup;
	UINT32  uiStrobeReadGroup;
	UINT32  uiNormalTimingGroup;

} EFUSE_TIMING_GROUP;

BOOL quary_secure_boot(SECUREBOOT_STATUS scu_status);
BOOL enable_secure_boot(SECUREBOOT_STATUS scu_status);
//BOOL cmd_efuse_show_avl(void);

typedef struct {
	UINT32  uiMaskAreaData0;
	UINT32  uiMaskAreaData1;
	UINT32  uiMaskAreaData2;
	UINT32  uiMaskAreaData3;

} EFUSE_MASK_AREA;

typedef struct {
	UINT32  uiReadOnlyAreaData0;
	UINT32  uiReadOnlyAreaData1;
	UINT32  uiReadOnlyAreaData2;
	UINT32  uiReadOnlyAreaData3;

} EFUSE_READ_ONLY_AREA;

extern ER       efuse_close(void);
extern INT32    efuse_read(UINT32 rowAddress);
extern INT32    efuse_readData(UINT32 rowAddress);
extern INT32    efuse_manual_read(UINT32 rowAddress);
extern INT32    efuse_manual_read_data(UINT32 rowAddress);
//extern void   efuse_clear_latch_data_field(void);
extern void     efuse_config(EFUSE_TIMING_GROUP *timCfg);
extern INT32    efuse_redundantReadData(UINT32 rowAddress);
extern void     efuse_dumpWholeBits(EFUSE_INFO_DEGREE degree);
extern INT32    efuse_writeData(UINT32 rowAddress, UINT32 data);
extern ER       efuse_program(UINT32 rowAddress, UINT32 colAddress);
extern INT32    efuse_config_key_set(EFUSE_WRITE_KEY_SET_TO_OTP_FIELD key_set_index);
extern INT32    efuse_write_key(EFUSE_WRITE_KEY_SET_TO_OTP_FIELD key_set_index, UINT8 *ucKey);
extern ER       efuse_repair(EFUSE_REPAIR_BANK uiBank, UINT32 uiFixedData, UINT32 uiRowAddr, UINT32 uiColumnAddr, BOOL bEnable);
extern ER       efuse_open(EFUSE_MODE mode, EFUSE_OPERATION_MODE opMode, EFUSE_REPAIR_EN repair, EFUSE_ARRAY_REDUNDANCY_SEL arraySel);
extern ER       efuse_setConfig(EFUSE_MODE mode, EFUSE_OPERATION_MODE opMode, EFUSE_REPAIR_EN repair, EFUSE_ARRAY_REDUNDANCY_SEL arraySel);
extern ER 		efuse_set_config(EFUSE_CONFIG_ID uiCfgID, UINT32 uiConfig);
extern INT32 	trigger_jtag_match_enable(UINT32 pass_msb, UINT32 pass_lsb);



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
extern INT32 efuse_readParamOps(EFUSE_PARAM_DATA param, UINT16 *data);


/*
     efuse_get_package_version

     efuse get IC package version

     @return IC revision of specific package revision
        - @b   NT96660_PKG      NT96660 package version
        - @b   NT96663_PKG      NT96663 package version
        - @b   NT96665_PKG      NT96665 package version
        - @b   NT9666X_ENG_VER  Engineer sample(This IC only for engineer sample)
        - @b   UNKNOWN_PKG_VER  Unknown IC package version(system must halt)
*/
extern UINT32 efuse_getPkgVersion(void);



/*
     efuse_setPkgVersion

     efuse set IC package version

     @note only support under emulation enable

     @return IC revision of specific package revision
        - @b    EFUSE_SUCCESS           Operation success
        - @b    E_NOSPT                 Not support
        - @b    EFUSE_FREEZE_ERR        Programmed already, only can read
        - @b    EFUSE_INVALIDATE_ERR    This field invalidate already
        - @b    EFUSE_UNKNOW_PARAM_ERR  efuse param field not defined
        - @b    EFUSE_OPS_ERR           efuse operation error

*/
extern INT32  efuse_setPkgVersion(NVT_PKG_UID uPKG);

/*
     efuse_check_available2

     efuse check specific library can use @ specific package ID or not

     @param[in]   name   library name

     @return IC revision of specific package revision
        - @b   TRUE     Usable library
        - @b   FALSE    Un-usable library
*/
extern BOOL efuse_check_available2(const CHAR * name, UINT32 param);

/*
     efuse_check_available_extend

     efuse check extend specific functionality

     @param[in]   param1   extend ability list
     @param[in]   param2   specific param of ability

     @return IC revision of specific package revision
        - @b   TRUE     support
        - @b   FALSE    not support
*/
extern BOOL efuse_check_available_extend2(EFUSE_PKG_ABILITY_LIST param1, UINT32 param2, UINT32 param3);

/*
     efuse_get_unique_id

     efuse get unique id (56bits)

     @param[out]     id_L   LSB of ID (32bit)
     @param[out]     id_H   MSB of ID (17bit)

     @return IC success or fail
        - @b   	EFUSE_SUCCESS      	success
        - @b	EFUSE_OPS_ERR		fail
*/
extern ER       efuse_get_unique_id(UINT32 * id_L, UINT32 * id_H);
extern UINT32 	efuse_is_secure_en(void);
extern UINT32 	efuse_getDieVersion(void);

/**
	efuse_setKeyDestination

	Set Destination of efuse get key field value

	@note for EFUSE_KEY_MANAGER_DST EFUSE_WRITE_KEY_SET_TO_OTP_FIELD

	@param[in] key_dst			Destination engine (Crypto/RSA/HASH)
	@param[in] key_field_set	No. of key field (0~15)

	Example: (Optional)
	@code
	{
		// Destination to 1st field of crypto engine
		efuse_setKeyDestination(EFUSE_KEY_MANAGER_CRYPTO, 0);
	}
	@endcode

	@return Success or fail
	- @b   E_OK    	Success
	- @b   E_SYS  	Parameter error
*/
extern ER     	efuse_setKeyDestination(EFUSE_KEY_MANAGER_DST key_dst, EFUSE_WRITE_KEY_SET_TO_OTP_FIELD key_field_set);

/**
    efuse_otp_set_key

    Durung encrypt or decrypt, configure specific key set as AES key(0~3)

    @Note: key set 0 is for secure boot use

    @param[in] key_set_index   key set (0~3)
    @return Description of data returned.
        - @b E_OK:   Success
*/
extern INT32 	efuse_otp_set_key(EFUSE_OTP_KEY_SET_FIELD key_set_index);
#if 0
extern INT32  efuse_setCypherKey(UINT8 *key, UINT32 keyLen);
#endif
extern INT32  	efuse_getCypherKey(UINT8 *key, UINT32 keyLen);

extern INT16 	efuse_getFirstPackageUID(void);
extern INT16 	efuse_setFirstPackageUID(UINT16 data);
/**
    otp_secure_en

    Enable secure boot (Rom treat loader as secure boot flow)
*/
#define otp_secure_en()                     enable_secure_boot(SECUREBOOT_SECURE_EN)

/**
    otp_data_area_encrypt_en

    ROM treat loader data area as cypher text enable
*/
#define otp_data_area_encrypt_en()          enable_secure_boot(SECUREBOOT_DATA_AREA_ENCRYPT)

/**
    otp_signature_rsa_en

    ROM treat loader by use RSA as loader's signature
*/
#define otp_signature_rsa_en()              enable_secure_boot(SECUREBOOT_SIGN_RSA)

/**
    otp_signature_ecdsa_en

    ROM treat loader by use ECDSA as loader's signature
*/
#define efuse_signature_ecdsa_en()          enable_secure_boot(SECUREBOOT_SIGN_ECDSA)

/**
    otp_signature_rsa_chksum_en

    Once use RSA as signature, enable RSA public checksum(use SHA256) enable
*/
#define otp_signature_rsa_chksum_en()       enable_secure_boot(SECUREBOOT_SIGN_RSA_CHK)

/**
    otp_signature_ecdsa_chksum_en

    Once use ECDSA as signature, enable ECDSA public checksum(use SHA256) enable
*/
#define efuse_signature_chksum_en()         enable_secure_boot(SECUREBOOT_SIGN_RSA_CHK)

#define efuse_header_signature_en()         enable_secure_boot(SECUREBOOT_HEADER_SIGNATURE)
#define efuse_signature_rsapss_en()         enable_secure_boot(SECUREBOOT_SIGN_RSA_PSS)
#define efuse_signature_rsapkcs1p5_en()     enable_secure_boot(SECUREBOOT_SIGN_RSA_PKCS1P5)

/**
    otp_jtag_dis

    Disable JTAG

    @Note: Can not re enable once disabled
*/
#define otp_jtag_dis()                      enable_secure_boot(SECUREBOOT_JTAG_DISABLE_EN)

//Query API collection
#define is_secure_enable()                  quary_secure_boot(SECUREBOOT_SECURE_EN)
#define is_data_area_encrypted()            quary_secure_boot(SECUREBOOT_DATA_AREA_ENCRYPT)
#define is_signature_rsa()                  quary_secure_boot(SECUREBOOT_SIGN_RSA)
#define is_signature_rsapss()               quary_secure_boot(SECUREBOOT_SIGN_RSA_PSS)
#define is_signature_rsapkcs1p5()           quary_secure_boot(SECUREBOOT_SIGN_RSA_PKCS1P5)
#define is_signature_ecdsa()                quary_secure_boot(SECUREBOOT_SIGN_ECDSA)
#define is_signature_rsa_chsum_enable()     quary_secure_boot(SECUREBOOT_SIGN_RSA_CHK)
#define is_signature_chsum_enable()         quary_secure_boot(SECUREBOOT_SIGN_RSA_CHK)
#define is_signature_aes()                  !quary_secure_boot(SECUREBOOT_SIGN_RSA)
#define is_JTAG_DISABLE_en()                quary_secure_boot(SECUREBOOT_JTAG_DISABLE_EN)
#define is_header_signature_en()            quary_secure_boot(SECUREBOOT_HEADER_SIGNATURE)

#define is_1st_key_programmed()             quary_secure_boot(SECUREBOOT_1ST_KEY_SET_PROGRAMMED)
#define is_2nd_key_programmed()             quary_secure_boot(SECUREBOOT_2ND_KEY_SET_PROGRAMMED)
#define is_3rd_key_programmed()             quary_secure_boot(SECUREBOOT_3RD_KEY_SET_PROGRAMMED)
#define is_4th_key_programmed()             quary_secure_boot(SECUREBOOT_4TH_KEY_SET_PROGRAMMED)
#define is_5th_key_programmed()             quary_secure_boot(SECUREBOOT_5TH_KEY_SET_PROGRAMMED)

#define is_1st_key_read_lock()              quary_secure_boot(SECUREBOOT_1ST_KEY_SET_READ_LOCK)
#define is_2nd_key_read_lock()              quary_secure_boot(SECUREBOOT_2ND_KEY_SET_READ_LOCK)
#define is_3rd_key_read_lock()              quary_secure_boot(SECUREBOOT_3RD_KEY_SET_READ_LOCK)
#define is_4th_key_read_lock()              quary_secure_boot(SECUREBOOT_4TH_KEY_SET_READ_LOCK)
#define is_5th_key_read_lock()              quary_secure_boot(SECUREBOOT_5TH_KEY_SET_READ_LOCK)
#endif

