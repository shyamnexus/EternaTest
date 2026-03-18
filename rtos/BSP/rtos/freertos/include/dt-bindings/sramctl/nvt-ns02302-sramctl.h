/**
    SRAM Control header file
    This file will handle core communications.
    @file       nvt-sramctl.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef __DT_BINDINGS_PINCTRL_NS02302_SRAMCTL_H
#define __DT_BINDINGS_PINCTRL_NS02302_SRAMCTL_H

/*
    SRAM ShutDown ID

    This is for nvt_disable_sram_shutdown() and nvt_enable_sram_shutdown().
*/
	/*0x1000*/
#define	MAU_SD		0		 //< Shut Down MAU SRAM
#define	SDP_SD		1		 //< Shut Down SDP SRAM
#define	HWCP_SD		2		 //< Shut Down HWCP SRAM
#define	TSE_SD		3		 //< Shut Down TSMUX SRAM
#define	GRAPH_SD	4		 //< Shut Down Graphic SRAM
#define	GRAPH2_SD	5		 //< Shut Down Graphic2 SRAM
#define	GRAPH3_SD	6		 //< Shut Down Graphic3 SRAM
#define	JPG_SD		7		 //< Shut Down JPG SRAM
#define	IVE_SD		8		 //< Shut Down IVE SRAM
#define	MDBC_SD		9		 //< Shut Down MDBC SRAM
#define	TRKE_SD		10		 //< Shut Down TRKE SRAM
#define	NUE2_SD		11		 //< Shut Down NUE2 SRAM
#define	ETH_SD		12		 //< Shut Down ETH SRAM
#define	IDE_SD		13		 //< Shut Down IDE SRAM
#define	RSA_SD		14		 //< Shut Down R1SA SRAM

#define	USB3_SD		32		 //< Shut Down USB3 SRAM
#define	DSI_SD		33		 //< Shut Down DSI SRAM
#define	CSI_TX_SD	34		 //< Shut Down CSI TX SRAM
#define	MCU_SD		35		 //< Shut Down MCU SRAM

#define	SIE_SD		64		 //< Shut Down SIE SRAM
#define	SIE2_SD		65		 //< Shut Down SIE2 SRAM
#define	SIE3_SD		66		 //< Shut Down SIE3 SRAM
#define	SIE4_SD		67		 //< Shut Down SIE4 SRAM
#define	SIE5_SD		68		 //< Shut Down SIE5 SRAM
#define	VIE_SD		69		 //< Shut Down VIE SRAM
#define	IFE_SD		70		 //< Shut Down IFE SRAM
#define	SDIO3_SD	71		 //< Shut Down SDIO3 SRAM
#define	SMC_SD		72		 //< Shut Down SMC SRAM
#define	DRE_SD		73		 //< Shut Down DRE SRAM
#define	ISE_SD		74		 //< Shut Down ISE SRAM
#define	PRE_SD		75		 //< Shut Down PRE SRAM

#define	IPE_SD		96		 //< Shut Down IPE SRAM
#define	IME_SD		97		 //< Shut Down IME SRAM
#define	SDIO_SD		98		 //< Shut Down SDIO SRAM
#define	SDIO2_SD	99		 //< Shut Down SDIO2 SRAM

#define	VPE_SD		128		 //< Shut Down VPE SRAM

#define	VENC_SD		160		 //< Shut Down VENC SRAM

#define	NUE_SD		192		 //< Shut Down NUE SRAM
#define	LSU_SD		193		 //< Shut Down LSU SRAM
#define	PPU_SD		194		 //< Shut Down PPU SRAM
#define	TCM_SD		195		 //< Shut Down TCM SRAM
#define	TCM2_SD		196		 //< Shut Down TCM2 SRAM
#define	POU_SD		197		//< Shut Down POU SRAM

#define	CONV_SD		224		//< Shut Down CONV SRAM

#define	H264_SD VENC_SD

#endif /* NVT_SRAMCTL_H */
