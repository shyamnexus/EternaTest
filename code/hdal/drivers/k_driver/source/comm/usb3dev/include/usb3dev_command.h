/*
    USB 3.0 device controller driver Endpoint Command header file

    This file defines some Endpoint Command data structures for USB3 device

    @file       usb3dev_command.h
    @ingroup    mIDrvUSB3_Device
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.
*/
#ifndef _USB3DEV_COMMAND_H
#define _USB3DEV_COMMAND_H

#include "usb3dev_int.h"


//====================================
//  For EPCMD[1]: DWC_EPCMD_SET_EP_CFG
//====================================

/*
    This enum represents the bit fields in the Device Endpoint Command Parameter 0 Register (DEPCMDPAR0n)
    for the Set Endpoint Configuration(DWC_EPCMD_SET_EP_CFG) command.
*/
typedef enum {
	/** Endpoint Type           <i>Access: R_W</i> */
	DWC_EPCFG0_EPTYPE_BITS              = 0x00000006,
	DWC_EPCFG0_EPTYPE_SHIFT             = 1,

	/** Endpoint Type values */
	DWC_USB3_EP_TYPE_CONTROL            = 0,    /** @< */
	DWC_USB3_EP_TYPE_ISOC               = 1,    /** @< */
	DWC_USB3_EP_TYPE_BULK               = 2,    /** @< */
	DWC_USB3_EP_TYPE_INTR               = 3,

	/** Maximum Packet Size         <i>Access: R_W</i> */
	DWC_EPCFG0_MPS_BITS                 = 0x00003ff8,
	DWC_EPCFG0_MPS_SHIFT                = 3,

	/** Flow Control State          <i>Access: R_W</i> */
	DWC_EPCFG0_FLOW_CTRL_STATE_BIT      = 0x00010000,
	DWC_EPCFG0_FLOW_CTRL_STATE_SHIFT    = 16,

	/** Tx Fifo Number (IN endpoints only)  <i>Access: R_W</i> */
	DWC_EPCFG0_TXFNUM_BITS              = 0x003e0000,
	DWC_EPCFG0_TXFNUM_SHIFT             = 17,

	/** Burst Size              <i>Access: R_W</i> */
	DWC_EPCFG0_BRSTSIZ_BITS             = 0x03c00000,
	DWC_EPCFG0_BRSTSIZ_SHIFT            = 22,

	/** Data Sequence Num (old)     <i>Access: R_W</i> */
	DWC_EPCFG0_DSNUM_BITS               = 0x7c000000,
	DWC_EPCFG0_DSNUM_SHIFT              = 26,

	/** Ignore Data Sequence Num (old)  <i>Access: R_W</i> */
	DWC_EPCFG0_IGN_DSNUM_BIT            = 0x80000000,
	DWC_EPCFG0_IGN_DSNUM_SHIFT          = 31,

	/** Config Action (new)         <i>Access: R_W</i> */
	DWC_EPCFG0_CFG_ACTION_BITS          = 0xc0000000,
	DWC_EPCFG0_CFG_ACTION_SHIFT         = 30,

	/** Config Action values (new) */
	DWC_CFG_ACTION_INIT                 = 0,    /** @< */
	DWC_CFG_ACTION_RESTORE              = 1,    /** @< */
	DWC_CFG_ACTION_MODIFY               = 2,
} DWC_EPCMD1_PARAM0;

/*
    This enum represents the bit fields in the Device Endpoint Command Parameter 1 Register (DEPCMDPAR1n)
    for the Set Endpoint Configuration (DEPCFG) command.
*/
typedef enum {
	/** Interrupt number */
	DWC_EPCFG1_INTRNUM_BITS     = 0x0000003f,
	DWC_EPCFG1_INTRNUM_SHIFT    = 0,

	/** Stream Completed */
	DWC_EPCFG1_XFER_CMPL_BIT    = 0x00000100,
	DWC_EPCFG1_XFER_CMPL_SHIFT  = 8,

	/** Stream In Progress */
	DWC_EPCFG1_XFER_IN_PROG_BIT = 0x00000200,
	DWC_EPCFG1_XFER_IN_PROG_SHIFT   = 9,

	/** Stream Not Ready */
	DWC_EPCFG1_XFER_NRDY_BIT    = 0x00000400,
	DWC_EPCFG1_XFER_NRDY_SHIFT  = 10,

	/** Rx FIFO Underrun / Tx FIFO Overrun */
	DWC_EPCFG1_FIFOXRUN_BIT     = 0x00000800,
	DWC_EPCFG1_FIFOXRUN_SHIFT   = 11,

	/** Back-to-Back Setup Packets Received  */
	DWC_EPCFG1_SETUP_PNDG_BIT   = 0x00001000,
	DWC_EPCFG1_SETUP_PNDG_SHIFT = 12,

	/** Endpoint Command Complete */
	DWC_EPCFG1_EPCMD_CMPL_BIT   = 0x00002000,
	DWC_EPCFG1_EPCMD_CMPL_SHIFT = 13,

	/** Endpoint EBC Mode */
	DWC_EPCFG1_EBC_MODE_BIT     = 0x00008000,
	DWC_EPCFG1_EBC_MODE_SHIFT   = 15,

	/** Endpoint bInterval */
	DWC_EPCFG1_BINTERVAL_BITS   = 0x00ff0000,
	DWC_EPCFG1_BINTERVAL_SHIFT  = 16,

	/** Endpoint Stream Capability */
	DWC_EPCFG1_STRM_CAP_BIT     = 0x01000000,
	DWC_EPCFG1_STRM_CAP_SHIFT   = 24,

	/** Endpoint Direction */
	DWC_EPCFG1_EP_DIR_BIT       = 0x02000000,
	DWC_EPCFG1_EP_DIR_SHIFT     = 25,

	/** Endpoint Number */
	DWC_EPCFG1_EP_NUM_BITS      = 0x3c000000,
	DWC_EPCFG1_EP_NUM_SHIFT     = 26,
} DWC_EPCMD1_PARAM1;








//=========================================================================================
//  This enum represents the bit fields in the Device Endpoint Command Register (DEPCMDn).
//=========================================================================================
typedef enum {
	/* Command Type            <i>Access: R_W</i> */
	DWC_EPCMD_TYP_BITS              = 0x0ff,
	DWC_EPCMD_TYP_SHIFT             = 0,

	/* Command Type values */
	DWC_EPCMD_SET_EP_CFG            = 1,
	DWC_EPCMD_SET_XFER_CFG          = 2,
	DWC_EPCMD_GET_EP_STATE          = 3,
	DWC_EPCMD_SET_STALL             = 4,
	DWC_EPCMD_CLR_STALL             = 5,
	DWC_EPCMD_START_XFER            = 6,
	DWC_EPCMD_UPDATE_XFER           = 7,
	DWC_EPCMD_END_XFER              = 8,
	DWC_EPCMD_START_NEW_CFG         = 9,

	/* Command Interrupt on Complete   <i>Access: R_W</i> */
	DWC_EPCMD_IOC_BIT               = 0x100,
	DWC_EPCMD_IOC_SHIFT             = 8,

	/* Command Active          <i>Access: R_W</i> */
	DWC_EPCMD_ACT_BIT               = 0x400,
	DWC_EPCMD_ACT_SHIFT             = 10,

	/* High Priority / Force RM Bit    <i>Access: R_W</i> */
	DWC_EPCMD_HP_FRM_BIT            = 0x800,
	DWC_EPCMD_HP_FRM_SHIFT          = 11,

	/* Command Completion Status       <i>Access: R_W</i> */
	DWC_EPCMD_CMPL_STS_BITS         = 0xf000,
	DWC_EPCMD_CMPL_STS_SHIFT        = 12,

	/* Stream Number or uFrame (input) <i>Access: R_W</i> */
	DWC_EPCMD_STR_NUM_OR_UF_BITS    = 0xffff0000,
	DWC_EPCMD_STR_NUM_OR_UF_SHIFT   = 16,

	/* Transfer Resource Index (output)    <i>Access: R_W</i> */
	DWC_EPCMD_XFER_RSRC_IDX_BITS    = 0x007f0000,
	DWC_EPCMD_XFER_RSRC_IDX_SHIFT   = 16,

	ENUM_DUMMY4WORD(DWC_EPCMD)
} DWC_EPCMD;

#define DWC_ENDXFER_FORCE           1
#define DWC_ENDXFER_NODELAY         2


/*
 * This enum represents the bit fields in the Device Generic Command Register (DGCMDn).
 */
typedef enum {
	/* Command Type         <i>Access: R_W</i> */
	DWC_DGCMD_TYP_BITS                      = 0x0ff,
	DWC_DGCMD_TYP_SHIFT                     = 0,

	/* Command Type values */
	DWC_DGCMD_SET_PERIODIC_PARAMS           = 2,
	DWC_DGCMD_XMIT_FUNC_WAKE_DEV_NOTIF      = 3,
	DWC_DGCMD_SET_SCRATCHPAD_ARRAY_ADR_LO   = 4,
	DWC_DGCMD_SET_SCRATCHPAD_ARRAY_ADR_HI   = 5,
	DWC_DGCMD_XMIT_HOST_ROLE_REQUEST        = 6,
	DWC_DGCMD_XMIT_DEV_NOTIF                = 7,
	DWC_DGCMD_SELECTED_FIFO_FLUSH           = 9,
	DWC_DGCMD_ALL_FIFO_FLUSH                = 10,
	DWC_DGCMD_SET_EP_NRDY                   = 12,
	DWC_DGCMD_RUN_SOC_BUS_LOOPBK_TST        = 16,

	/* Command Interrupt on Complete    <i>Access: R_W</i> */
	DWC_DGCMD_IOC_BIT                       = 0x100,
	DWC_DGCMD_IOC_SHIFT                     = 8,

	/* Command Active           <i>Access: R_W</i> */
	DWC_DGCMD_ACT_BIT                       = 0x400,
	DWC_DGCMD_ACT_SHIFT                     = 10,

	/* Command Status           <i>Access: R_W</i> */
	DWC_DGCMD_STS_BITS                      = 0xf000,
	DWC_DGCMD_STS_SHIFT                     = 12,

	/* Command Status values */
	DWC_DGCMD_STS_ERROR                     = 15,

	ENUM_DUMMY4WORD(DWC_DGCMD)
} DWC_DGCMD;







//
//	Device Controller EP Commands
//
void    usb3_dep_cfg(USB3_PEP PhyEP, UINT32 uiParam0, UINT32 uiParam1, UINT32 uiParam2);
void    usb3_dep_xfercfg(USB3_PEP PhyEP, UINT32 depstrmcfg);
UINT8   usb3_dep_startxfer(USB3_PEP PhyEP, uintptr_t dma_addr, UINT32 stream_or_uf);
void    usb3_dep_updatexfer(USB3_PEP PhyEP, UINT32 tri);
void    usb3_dep_endxfer(USB3_PEP PhyEP, UINT32 tri, UINT32 flags);
void    usb3_dep_startnewcfg(UINT32 rsrcidx);
void    usb3_dep_set_stall(USB3_PEP PhyEP);
void    usb3_dep_clr_stall(USB3_PEP PhyEP, BOOL clr_pend);


//
//	Device Controller Device Commands
//
void    usb3_dev_periodic_parameter(UINT32 SEL);
void    usb3_dev_flush_fifo(USB3_PEP PhyEP);
void    usb3_dev_flush_all_fifo(void);
void    usb3_dev_set_nrdy(USB3_PEP PhyEP);
void    usb3_dev_loopback_test(BOOL bEnable);

#endif
