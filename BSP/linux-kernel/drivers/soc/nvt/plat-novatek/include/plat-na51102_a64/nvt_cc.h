/**

    Header file for CC (Core Communicator) module.

    This file is the header file that define the API and data type
    for CC module.

    @file       nvt_cc.h
    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef _CC_H
#define _CC_H

typedef unsigned int        CC_U32;     ///< Unsigned 32 bits data type
typedef   signed int        CC_I32;     ///< signed   32 bits data type
typedef unsigned long       CC_ULONG;   ///<

typedef enum {
	CC_CORE_CA53_CORE1 = 0x1,         ///< 1st Core CA53
	CC_CORE_CA53_CORE2 = 0x2,         ///< 2nd Core CA53
	CC_CORE_CA53_CORE3 = 0x3,         ///< 3th Core CA53
	CC_CORE_CA53_CORE4 = 0x4,         ///< 4th Core CA53
	CC_CORE_DSP1       = 0x3,         ///< 1st Core DSP
	CC_CORE_DSP2       = 0x4,         ///< 2nd Core DSP
	CC_CORE_DUMMY      = 0xFFFFFFFF,
} CC_CORE_ID;

typedef enum {
	CC_EVT_FROM_CPU1        = 0x00000001,           ///< Event receive from CPU1
	CC_EVT_FROM_CPU2        = 0x00000002,           ///< Event receive from CPU2
	CC_EVT_FROM_DSP1        = 0x00000004,           ///< Event receive from DSP
	CC_EVT_FROM_DSP2        = 0x00000008,           ///< Event receive from DSP2

	CC_CPU2_GOES_SLEEP      = 0x00000100,           ///< CPU2 entry sleep mode, @note  call __asm__("wait")instruction
	CC_DSP_GOES_LIGHT_SLEEP = 0x00000200,           ///< DSP entry light sleep mode
	CC_DSP_GOES_STANDBY     = 0x00000400,           ///< DSP entry standbymode

	CC_ACK_FROM_CPU1        = 0x00010000,           ///< CPU1 ack request
	CC_ACK_FROM_CPU2        = 0x00020000,           ///< CPU2 ack request
	CC_ACK_FROM_DSP1        = 0x00040000,           ///< DSP ack request
	CC_ACK_FROM_DSP2        = 0x00080000,           ///< DSP ack request

	CC_EVT_ERR              = 0xFFFFFFFF,
//    ENUM_DUMMY4WORD(CC_EVENT)

} CC_EVENT;

typedef enum _CC_ER {
	CC_ER_OK                      =  0x00000000, ///< no error
	CC_ER_PARM                    =  -1,         ///< parameter error
	CC_ER_TMOUT                   =  -2,         ///< cc timeout
} CC_ER;

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
	CC_RES_ID_DUMMY  = 0xFFFFFFFF,
} CC_RES_ID;

//STATIC_ASSERT(CC_RES_ID_NUM <= CC_RES_ID_MAX);

#define CC_RESOURCE_IPC     CC_RES_ID_0     //< IPC resource between multi-cpu
#define CC_RESOURCE_RTC     CC_RES_ID_1     //< RTC resource between multi-cpu
#define CC_RESOURCE_CKG     CC_RES_ID_2     //< CKG resource between multi-cpu
#define CC_RESOURCE_TOP     CC_RES_ID_3     //< TOP resource between multi-cpu
#define CC_RESOURCE_GPIO    CC_RES_ID_4     //< GPIO resource between multi-cpu
#define CC_RESOURCE_INTC    CC_RES_ID_5     //< INTC resource between multi-cpu



/**
    @addtogroup mIDrvSys_CC
*/
//@{

// Core Communicator handler
typedef void (*CC_HANDLER)(CC_U32 event);

typedef struct {
	CC_U32           cmd_buf[3];
} CC_CMD, *PCC_CMD;

typedef struct {
	void        *reg_base;
} CC_DEVICE;


extern void     cc_register_cb_handler(CC_HANDLER hdl);

extern CC_ER    cc_send_core_cmd(CC_CORE_ID core_id, PCC_CMD p_cmd);

extern void     cc_get_core_cmd(CC_CORE_ID core_id, PCC_CMD p_cmd);

extern void     cc_corex_ack_cmd(CC_CORE_ID core_id);

extern CC_ER    cc_corex_send_cmd(CC_CORE_ID core_id, PCC_CMD p_cmd);

extern CC_ER    cc_hwlock_resource(CC_CORE_ID core_id, CC_RES_ID res_id);

extern CC_ER    cc_hwunlock_resource(CC_CORE_ID core_id, CC_RES_ID res_id);

extern void     cc_set_reg(CC_U32 reg_offset, CC_U32 value);


//@}

#endif /* _CC_H */
