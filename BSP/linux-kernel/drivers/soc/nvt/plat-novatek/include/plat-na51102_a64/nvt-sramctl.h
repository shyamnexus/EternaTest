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

#ifndef _NA51102_NVT_SRAMCTL_H
#define _NA51102_NVT_SRAMCTL_H
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
	MAU_SD = 0,       //< Shut Down MAU SRAM
	SDIO_SD,          //< Shut Down SDIO SRAM
	SDIO2_SD,         //< Shut Down SDIO2 SRAM
	SDIO3_SD,         //< Shut Down SDIO3 SRAM
	SMC_SD,           //< Shut Down SMC SRAM
	TSE_SD,           //< Shut Down TSMUX SRAM
	IFE_SD,           //< Shut Down IFE SRAM
	SDP_SD,           //< Shut Down SDP SRAM
	HWCP_SD,          //< Shut Down HWCP SRAM	
	RSA_SD,           //< Shut Down RSA SRAM
	DSI_SD,           //< Shut Down DSI SRAM
	CSI_TX_SD,        //< Shut Down CSI TX SRAM
	USB_SD,           //< Shut Down USB SRAM
	USB3_SD,          //< Shut Down USB3 SRAM
	ETH_SD,           //< Shut Down ETH SRAM
	SATA_SD,          //< Shut Down SATA SRAM
	IDE_SD,           //< Shut Down IDE SRAM
	IDE2_SD,    	  //< Shut Down IDE2 SRAM
	SIE_SD,           //< Shut Down SIE SRAM
	SIE2_SD,          //< Shut Down SIE2 SRAM
	SIE3_SD,          //< Shut Down SIE3 SRAM
	SIE4_SD,          //< Shut Down SIE4 SRAM
	SIE5_SD,          //< Shut Down SIE5 SRAM
	SIE6_SD,          //< Shut Down SIE6 SRAM
	VIE_SD,           //< Shut Down VIE SRAM
	VIE2_SD,          //< Shut Down VIE2 SRAM
	VPE_SD,           //< Shut Down VPE SRAM
	VPEL_SD,          //< Shut Down VPEL SRAM
	JPG_SD,           //< Shut Down JPG SRAM
	JPG2_SD,          //< Shut Down JPG2 SRAM
	GRAPH_SD,         //< Shut Down Graphic SRAM
	GRAPH2_SD,        //< Shut Down Graphic2 SRAM

	/*0x1004*/
	GRAPH3_SD,        //< Shut Down Graphic3 SRAM
	ISE_SD,           //< Shut Down ISE SRAM
	ISE2_SD,          //< Shut Down ISE2 SRAM
	MDBC_SD,          //< Shut Down MDBC SRAM
	DIS_SD,           //< Shut Down DIS SRAM
	DRE_SD,           //< Shut Down DRE SRAM
	IVE_SD,           //< Shut Down IVE SRAM
	SDE_SD,     	  //< Shut Down SDE SRAM
	TRKE_SD,          //< Shut Down TRKE SRAM
	CNN_SD,           //< Shut Down CNN SRAM
	CNN2_SD,          //< Shut Down CNN2 SRAM
	DCE_SD,           //< Shut Down DCE SRAM
	NUE_SD,      	  //< Shut Down NUE SRAM
	NUE2_SD,          //< Shut Down NUE2 SRAM
	TCM_SD,           //< Shut Down TCM SRAM
	IPE_SD,           //< Shut Down IPE SRAM
	IME_SD,           //< Shut Down IME SRAM
	VENC_SD,          //< Shut Down VENC SRAM
	DSP_SD,           //< Shut Down DSP SRAM
	ETH2_SD,          //< Shut Down ETH2 SRAM
	VDEC_SD,          //< Shut Down VDEC SRAM
	JPGL_SD,          //< Shut Down JPGL SRAM

	///////////////////////////////////////////////
	H264_SD = VENC_SD,

	//ENUM_DUMMY4WORD(SRAM_SD)
} SRAM_SD;

extern void nvt_disable_sram_shutdown(SRAM_SD id);
extern void nvt_enable_sram_shutdown(SRAM_SD id);

#endif /* NVT_SRAMCTL_H */
