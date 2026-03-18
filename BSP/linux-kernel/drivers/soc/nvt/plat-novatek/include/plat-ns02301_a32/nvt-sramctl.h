/**
	SRAM Control header file
	This file will handle core communications.
	@file       nvt-sramctl.h
	@ingroup
	@note
	Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#ifndef _NS02301_NVT_SRAMCTL_H
#define _NS02301_NVT_SRAMCTL_H
#include <linux/soc/nvt/rcw_macro.h>
#include <linux/soc/nvt/nvt_type.h>
#include <plat/hardware.h>
//#define ENUM_DUMMY4WORD(m)
//#define NVT_SRAM_PHY_BASE 0xF0011000
/*
    SRAM ShutDown ID

    This is for nvt_disable_sram_shutdown() and nvt_enable_sram_shutdown().
*/
typedef enum {
	ETH_SD = 0,		//< Shut Down ETH SRAM
	IDE_SD = 1,		//< Shut Down IDE SRAM
	TSE_SD,           //< Shut Down TSMUX SRAM
	GRAPH_SD,         //< Shut Down Graphic SRAM
	GRAPH2_SD,        //< Shut Down Graphic2 SRAM
	CPU_BRG_SD,       //< Shut Down CPU BRIDGE SRAM
	CFE_SD,          //< Shut Down CFE SRAM
	CPU_SD,          //< Shut Down CPU SRAM
	SDIO3_SD = 8,    //< Shut Down SDIO3 SRAM
	NAND_SD,          //< Shut Down xD/NAND SRAM
	RSA_SD,           //< Shut Down RSA SRAM
	SDP_SD,           //< Shut Down SDP SRAM
	SIE_SD = 12,       //< Shut Down SIE SRAM
	SIE2_SD,          //< Shut Down SIE2 SRAM
	SIE3_SD,          //< Shut Down SIE3 SRAM
	ISE_SD,           //< Shut Down ISE SRAM
	IFE_SD = 16,           //< Shut Down IFE SRAM
	H264_SD,     //< Shut Down H264 SRAM
	JPG_SD,           //< Shut Down JPG SRAM
	CNN_SD,           //< Shut Down CNN SRAM
	IVE_SD,           //< Shut Down IVE SRAM
	// 20
	NUE2_SD,          //< Shut Down NUE2 SRAM
	MDBC_SD,          //< Shut Down MDBC SRAM
	VPE_SD,      //< Shut Down VPE SRAM
	IPE_SD,       //< Shut Down IPE SRAM
	IME_SD,           //< Shut Down IME SRAM
	// 25
	USB_SD,           //< Shut Down USB SRAM
	YDCCD_SD = 27,           //< Shut Down YDCCD SRAM

	ENUM_DUMMY4WORD(SRAM_SD)
} SRAM_SD;

extern void nvt_disable_sram_shutdown(SRAM_SD id);
extern void nvt_enable_sram_shutdown(SRAM_SD id);

/*
    Pre-roll part SRAM ShutDown ID

    This is for nvt_disable_pr_sram_shutdown() and nvt_enable_pr_sram_shutdown().
*/
typedef enum {
	TCM1_SD,        //< Shut Down TCM1
	TCM2_SD,        //< Shut Down TCM2
	TCM3_SD,        //< Shut Down TCM3
	TCM4_SD,	//< Shut Dowm TCM4 (share controlled by MCU_SD)
	MCU_SD,         //< Shut Down MCU
	PRISP_SD = 5,   //< Shut Down PR-ISP

        ENUM_DUMMY4WORD(PR_SRAM_SD)
} PR_SRAM_SD;

extern void nvt_disable_pr_sram_shutdown(PR_SRAM_SD id);
extern void nvt_enable_pr_sram_shutdown(PR_SRAM_SD id);


#endif /* _NT96680_NVT_SRAMCTL_H */
