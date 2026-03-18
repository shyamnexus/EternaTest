/**
    SRAM Control header file
    This file will handle core communications.
    @file       nvt-sramctl.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef _NS02201_NVT_SRAMCTL_H
#define _NS02201_NVT_SRAMCTL_H
#include <linux/soc/nvt/rcw_macro.h>
#include <linux/soc/nvt/nvt_type.h>
#include <plat/hardware.h>
//#define ENUM_DUMMY4WORD(m)
/*
    SRAM ShutDown ID

    This is for nvt_disable_sram_shutdown() and nvt_enable_sram_shutdown().
*/

typedef enum {
	/*0x1000*/
	AGE_SD = 0,			//< Shut Down AGE SRAM
	N25_SD,				//< Shut Down N25 SRAM
	GRAPH3_SD,			//< Shut Down Graphic3 SRAM
	GRAPH2_SD,			//< Shut Down Graphic2 SRAM
	GRAPH_SD,			//< Shut Down Graphic SRAM
	SDP_SD,				//< Shut Down SDP SRAM
	TSE_SD,				//< Shut Down TSE SRAM
	SYSM_SD,			//< Shut Down SYSTEM SRAM
	HWCP_SD,			//< Shut Down HWCOPY SRAM
	RSA_SD,				//< Shut Down RSA SRAM
	MAU2_SD,			//< Shut Down MAU2 SRAM
	MAU_SD,				//< Shut Down MAU SRAM
	VIE2_SD,
	VIE_SD,
	SIE12_SD,
	SIE11_SD,
	SIE10_SD,
	SIE9_SD,
	SIE8_SD,
	SIE7_SD,
	SIE6_SD,
	SIE5_SD,
	SIE4_SD,
	SIE3_SD,
	SIE2_SD,
	SIE_SD,
	ETH2_SD,
	ETH_SD,
	CSI_TX2_SD,
	CSI_TX_SD,
	DSI2_SD,
	DSI_SD,

	/*0x1004*/
	LCD3_SD,
	LCD2_SD,
	LCD_SD,
	PPU_SD,
	NUE2_SD,
	TRKE_SD,
	MDBC_SD,
	IVE_SD,
	SDE_SD,
	DIS_SD,
	LSU_SD,
	CAL_SD,
	ROU_SD,
	UTL_SD,
	CONV4_SD,
	CONV3_SD,
	CONV2_SD,
	CONV_SD,
	TCM4_SD,
	TCM3_SD,
	TCM2_SD,
	TCM_SD,
	MALI_SD,
	DSP_SD,
	DSP2_SD,
	USB2_SD,
	USB3_SD,
	JPG3_SD,
	JPG2_SD,
	JPG_SD,
	VDEC_SD,
	VENC_SD,

	/*0x1008*/
	VTRC_SD,
	PCIE2_SD,
	PCIE_SD,
	SATA2_SD,
	SATA_SD,
	USB3_2_SD,
	VPE3_SD,
	VPE_SD,
	DRE_SD,
	ISE_SD,
	IFE_SD,
	IME_SD,
	IPE_SD,
	SMC_SD,
	SDIO3_SD,
	VPE2_SD,
	ISE2_SD,
	IFE2_SD,
	SDIO2_SD,
	SDIO_SD,
	IME2_SD,
	IPE2_SD,
	XOR_SD,

	///////////////////////////////////////////////
	H264_SD = VENC_SD,

	//ENUM_DUMMY4WORD(SRAM_SD)
} SRAM_SD;

extern void nvt_disable_sram_shutdown(SRAM_SD id);
extern void nvt_enable_sram_shutdown(SRAM_SD id);

#endif /* NVT_SRAMCTL_H */
