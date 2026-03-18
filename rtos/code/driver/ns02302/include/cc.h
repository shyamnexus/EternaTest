/**
    Header file for CC (Core Communicator) module.

    This file is the header file that define the API and data type
    for CC module.

    @file       CC.h
    @ingroup    mIDrvSys_CC
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _CC_H
#define _CC_H

#include <kwrap/nvt_type.h>

#ifdef _NVT_EMULATION_
#ifndef _EMULATION_
#define _EMULATION_ ENABLE
#endif
#else
#ifndef _EMULATION_
#define _EMULATION_ DISABLE
#endif
#endif

#ifdef _EMULATION_ON_CPU2_
#define _EMULATION_ON_CPU2_ ENABLE
#else
#define _EMULATION_ON_CPU2_ DISABLE
#endif

/**
    @addtogroup mIDrvSys_CC
*/
//@{

/**
    core communication core ID number

    @note For cc_hwLockResource(), cc_hwUnlockResource(), cc_getHwResGrant(),cc_getHwResReqFlag(), cc_startCore(), cc_stopCore(), cc_configCoreOutstanging()
*/
#if 0
typedef enum {
	CC_CORE_CA53_CORE1_ENUM = 0,   ///< 1st core
	CC_CORE_CA53_CORE2_ENUM,

	CC_CORE_DSP_ENUM,
	CC_CORE_DSP2_ENUM,

	ENUM_DUMMY4WORD(CC_CORE_ID_ENUM)

} CC_CORE_ID_ENUM;
#endif

typedef enum {
	CC_CORE_CA53_CORE1 = 1,   ///< 1st core
	CC_CORE_CA53_CORE2,

	CC_CORE_DSP,
	CC_CORE_DSP2,


	ENUM_DUMMY4WORD(CC_CORE_ID)

} CC_CORE_ID;


/**
    core communication event ID number

    @note For cc_waitCoreEntrySleep()
*/
typedef enum {
	CC_EVT_FROM_CPU1        = 0x00000001,           ///< Event receive from CPU1
	CC_EVT_FROM_CPU2        = 0x00000002,           ///< Event receive from CPU2
	CC_EVT_FROM_DSP         = 0x00000004,           ///< Event receive from DSP
	CC_EVT_FROM_DSP2        = 0x00000008,           ///< Event receive from DSP2

	CC_ACK_FROM_CPU1        = 0x00010000,           ///< CPU1 ack request
	CC_ACK_FROM_CPU2        = 0x00020000,           ///< CPU2 ack request
	CC_ACK_FROM_DSP         = 0x00040000,           ///< DSP ack request
	CC_ACK_FROM_DSP2        = 0x00080000,           ///< DSP ack request

//	CC_CPU2_GOES_SLEEP      = 0x10000000,           ///< CPU2 entry sleep mode
	///< @note  call __asm__("wait")instruction
	CC_DSP_GOES_LIGHT_SLEEP = 0x20000000,           ///< DSP entry light sleep mode
	CC_DSP_GOES_STANDBY     = 0x40000000,           ///< DSP entry standbymode


	CC_EVT_ERR              = 0xFFFFFFFF,
	ENUM_DUMMY4WORD(CC_EVENT)

} CC_EVENT;

/*
    core request configuration 1-4
*/
typedef enum {
	CC_CORE_REQ_1 = 0x0,
	CC_CORE_REQ_2,
	CC_CORE_REQ_3,
	CC_CORE_REQ_4,

	CC_CORE_REQ_MAX,
	ENUM_DUMMY4WORD(CC_CORE_REQ_NUM)
} CC_CORE_REQ_NUM;


/**
    core communication configurated ID number

    @note For cc_setConfig()
*/
typedef enum {
	CC_CONFIG_ID_CORE2_BASE_ADDRESS = 1,            ///< Configured CPU exception base address
	///< @note  represent code start point running @ Core2
	///< Context is :
	///< - @b UINT32 : address of base address
	CC_CONFIG_ID_DSP_BASE_ADDRESS,                  ///< Configured CPU exception base address
	///< @note  represent code start point running @ DSP
	///< - @b UINT32 : address of base address

	CC_CONFIG_ID_CTRL_SRST,                         ///< CC controller will reset to default value except MIPS2 & DSP base address
	///< - @b NULL

	CC_CONFIG_ID_SET_MIPS1_REQ,                     ///< Set MIPS1 request slot number of each process cycle
	///< Context is :
	///< - @b CC_CORE_REQ_NUM : slot number of each process cycle(1-4)

	CC_CONFIG_ID_SET_MIPS2_REQ,                     ///< Set MIPS2 request slot number of each process cycle
	///< Context is :
	///< - @b CC_CORE_REQ_NUM : slot number of each process cycle(1-4)

	CC_CONFIG_ID_SET_DSP_EPP_REQ,                   ///< Set DSP EPP request slot number of each process cycle
	///< Context is :
	///< - @b CC_CORE_REQ_NUM : slot number of each process cycle(1-4)

	CC_CONFIG_ID_SET_DSP_EDP_REQ,                   ///< Set DSP EDP request slot number of each process cycle
	///< Context is :
	///< - @b CC_CORE_REQ_NUM : slot number of each process cycle(1-4)

	CC_CONFIG_ID_SET_DSP_PINT_BASE_ADDRESS,         ///< Set DSP Program_Int source data base address
	///< Context is :
	///< - @b UINT32 : address of base address

	CC_CONFIG_ID_SET_DSP_ISDM_BASE_ADDRESS,         ///< Set DSP Scaler memory source data base address
	///< Context is :
	///< - @b UINT32 : address of base address

	CC_CONFIG_ID_SET_DSP_IVDM_BASE_ADDRESS,         ///< Set DSP Vector memory source data base address
	///< Context is :
	///< - @b UINT32 : address of base address

	CC_CONFIG_ID_SET_DSP2_DSP_BASE1_ADDRESS,        ///< Set DSP DSP2 base1 address
	///< Context is :
	///< - @b UINT32 : address of base address

	CC_CONFIG_ID_SET_DSP2_DSP_BASE2_ADDRESS,        ///< Set DSP DSP2 base1 address
	///< Context is :
	///< - @b UINT32 : address of base address

	CC_CONFIG_ID_SET_DSP2_DSP_BASE3_ADDRESS,        ///< Set DSP DSP2 base1 address
	///< Context is :
	///< - @b UINT32 : address of base address

	CC_CONFIG_ID_SET_ACK_INTEN,                     ///< Set CoreX ack signal inten
	///< Context is :
	///< - @b UINT32 : point of PCC_ACK_INTEN

	CC_CONFIG_ID_SET_INT_ID,


	ENUM_DUMMY4WORD(CC_CONFIG_ID)
} CC_CONFIG_ID;

//@}
// Core Communicator handler
typedef void (*CC_HANDLER)(UINT32 uiEvent);

typedef struct {
	UINT32      uiCMDID;        ///< Command ID, a serial number from 1 to 0xFFFF
	///< @note      For cc_getCoreXCMD() only, generated by CC driver.
	UINT32      uiCMDOperation; ///< Command operation, from 0 to 0xFFFF
	UINT32     *pData;          ///< Command data
	///< @note      Not the address of data that will be processed by this command
	UINT32      uiDataSize;     ///< Command data size in bytes
	///< @note      Not the size of data that will be processed by this command
} CC_CMD, *PCC_CMD;

typedef struct {
	UINT32      CoreID;         ///< core ID
	///< @note      For cc_getCoreXCMD() only, generated by CC driver.
	UINT32      uiCCEvt;        ///< CC event

	BOOL        bEn;            ///< enable or disable
	///< @note      Not the size of data that will be processed by this command
} CC_ACK_INTEN, *PCC_ACK_INTEN;

extern ER       cc_open(void);
extern ER       cc_close(void);

extern void     cc_registerCore1Handler(CC_HANDLER Hdl);
extern void     cc_registerCore2Handler(CC_HANDLER Hdl);
extern void     cc_registerCore3Handler(CC_HANDLER Hdl);
extern void     cc_registerCore4Handler(CC_HANDLER Hdl);

extern ER       cc_sendCore1CMD(PCC_CMD pCMD);
extern ER       cc_sendCore2CMD(PCC_CMD pCMD);
extern ER       cc_sendCore3CMD(PCC_CMD pCMD);
extern ER       cc_sendCore4CMD(PCC_CMD pCMD);
extern ER       cc_sendCoreXCMD(UINT32 fromCore, UINT32 toCore);

extern void     cc_getCore1CMD(PCC_CMD pCMD);
extern void     cc_getCore2CMD(PCC_CMD pCMD);
extern void     cc_getCore3CMD(PCC_CMD pCMD);
extern void     cc_getCore4CMD(PCC_CMD pCMD);

extern void     cc_ackCore1CMD(void);
extern void     cc_ackCore2CMD(void);
extern void     cc_ackCore3CMD(void);
extern void     cc_ackCore4CMD(void);
extern void     cc_ackCoreXCMD(UINT32 fromCore, UINT32 toCore);

//extern void     cc_setCore2Base(UINT32 uiBaseAddr);
//extern void     cc_setCore3BootVector(UINT32 uiBVAddr);
//extern UINT32   cc_getCore2Base(void);
//extern UINT32   cc_getCore3BootVector(void);
extern void 	cc_warm_reset(void);
extern void     cc_stopCore(CC_CORE_ID uiCoreID);
extern ER       cc_startCore(CC_CORE_ID uiCoreID);
//extern void     cc_configCoreOutstanging(CC_CORE_ID uiCoreID, BOOL bArbEn, BOOL bApbEn);
//extern CC_EVENT cc_waitCoreEntrySleep(CC_EVENT uiEvt);

extern void     	cc_setConfig(CC_CONFIG_ID ConfigID, ULONG uiConfig);
extern INT32    	cc_getHwResGrant(CC_CORE_ID uiCoreID);
extern INT32    	cc_getHwResReqFlag(CC_CORE_ID uiCoreID);
extern CC_CORE_ID 	cc_getCoreCount(void);


#define CC_RES_ID_MAX      32

typedef enum {
	CC_RES_ID_0 = 0x0,
	CC_RES_ID_1,
	CC_RES_ID_2,
	CC_RES_ID_3,
	CC_RES_ID_4,
	CC_RES_ID_5,

	// Insert member before this line
	// Don't change the order of following three members
	CC_RES_ID_NUM,
	ENUM_DUMMY4WORD(CC_RES_ID)

} CC_RES_ID;

STATIC_ASSERT(CC_RES_ID_NUM <= CC_RES_ID_MAX);

#define CC_RESOURCE_IPC     CC_RES_ID_0     //< IPC resource between multi-cpu
#define CC_RESOURCE_RTC     CC_RES_ID_1     //< RTC resource between multi-cpu
#define CC_RESOURCE_CKG     CC_RES_ID_2     //< CKG resource between multi-cpu
#define CC_RESOURCE_TOP     CC_RES_ID_3     //< TOP resource between multi-cpu
#define CC_RESOURCE_GPIO    CC_RES_ID_4     //< GPIO resource between multi-cpu
#define CC_RESOURCE_INTC    CC_RES_ID_5     //< INTC resource between multi-cpu

extern ER       cc_hwLockResource(CC_CORE_ID uiCoreID, CC_RES_ID uiResource);
extern ER       cc_hwUnlockResource(CC_CORE_ID uiCoreID, CC_RES_ID uiResource);


#endif
