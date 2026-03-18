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

#ifndef __NVT_SRAMCTL_H
#define __NVT_SRAMCTL_H
#include <linux/soc/nvt/rcw_macro.h>
#include <linux/soc/nvt/nvt_type.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <plat/hardware.h>
//#define ENUM_DUMMY4WORD(m)
/*
    SRAM ShutDown ID

    This is for nvt_disable_sram_shutdown() and nvt_enable_sram_shutdown().
*/
typedef enum {
	/*0x1000*/
	MAU_SD = 0,       //< Shut Down MAU SRAM
	SDP_SD,           //< Shut Down SDP SRAM
	HWCP_SD,          //< Shut Down HWCP SRAM	
	TSE_SD,           //< Shut Down TSMUX SRAM
	GRAPH_SD,         //< Shut Down Graphic SRAM
	GRAPH2_SD,        //< Shut Down Graphic2 SRAM
	GRAPH3_SD,        //< Shut Down Graphic3 SRAM
	JPG_SD,           //< Shut Down JPG SRAM
	IVE_SD,           //< Shut Down IVE SRAM
	MDBC_SD,          //< Shut Down MDBC SRAM
	TRKE_SD,          //< Shut Down TRKE SRAM
	NUE2_SD,          //< Shut Down NUE2 SRAM
	ETH_SD,           //< Shut Down ETH SRAM
	IDE_SD,           //< Shut Down IDE SRAM
	RSA_SD,           //< Shut Down R1SA SRAM

	/*0x1004*/
	USB3_SD = 32,     //< Shut Down USB3 SRAM
	DSI_SD,           //< Shut Down DSI SRAM
	CSI_TX_SD,        //< Shut Down CSI TX SRAM
	MCU_SD,			  //< Shut Down MCU SRAM

	/*0x1008*/
	SIE_SD = 64,      //< Shut Down SIE SRAM
	SIE2_SD,          //< Shut Down SIE2 SRAM
	SIE3_SD,          //< Shut Down SIE3 SRAM
	SIE4_SD,          //< Shut Down SIE4 SRAM
	SIE5_SD,          //< Shut Down SIE5 SRAM
	VIE_SD,           //< Shut Down VIE SRAM
	IFE_SD,           //< Shut Down IFE SRAM
	SDIO3_SD,         //< Shut Down SDIO3 SRAM
	SMC_SD,           //< Shut Down SMC SRAM
	DRE_SD,           //< Shut Down DRE SRAM
	ISE_SD,           //< Shut Down ISE SRAM
	PRE_SD,           //< Shut Down PRE SRAM

	/*0x100C*/
	IPE_SD = 96,      //< Shut Down IPE SRAM
	IME_SD,           //< Shut Down IME SRAM
	SDIO_SD,          //< Shut Down SDIO SRAM
	SDIO2_SD,         //< Shut Down SDIO2 SRAM

	/*0x1010*/
	VPE_SD = 128,     //< Shut Down VPE SRAM

	/*0x1014*/
	VENC_SD = 160,    //< Shut Down VENC SRAM

	/*0x1018*/
	NUE_SD = 192,     //< Shut Down NUE SRAM
	LSU_SD,      	  //< Shut Down LSU SRAM
	PPU_SD,      	  //< Shut Down PPU SRAM
	TCM_SD,           //< Shut Down TCM SRAM
	TCM2_SD,          //< Shut Down TCM2 SRAM
	POU_SD,          //< Shut Down POU SRAM

	/*0x101C*/
	CONV_SD = 224,    //< Shut Down CONV SRAM

	///////////////////////////////////////////////
	H264_SD = VENC_SD,

	//ENUM_DUMMY4WORD(SRAM_SD)
} SRAM_SD;

struct nvt_sram_shutdown_info {
	unsigned int sram_shutdown_id;
	unsigned int  enable;
};

extern void nvt_disable_sram_shutdown(SRAM_SD id);
extern void nvt_enable_sram_shutdown(SRAM_SD id);
extern struct nvt_sram_shutdown_info nvt_sram_sd_tbl[];
extern int sram_shutdown_size;

#endif /* NVT_SRAMCTL_H */
