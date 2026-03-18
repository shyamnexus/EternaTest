/**
	SRAM Control header file
	This file will handle core communications.
	@file       nvt-sramctl.h
	@ingroup
	@note
	Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#ifndef _NA51090_NVT_SRAMCTL_H
#define _NA51090_NVT_SRAMCTL_H
#include <linux/soc/nvt/rcw_macro.h>
#include <linux/soc/nvt/nvt_type.h>
#include <plat/hardware.h>
//#define ENUM_DUMMY4WORD(m)
#define NVT_SRAM_PHY_BASE 0x2F0011000
/*
    SRAM ShutDown ID

    This is for nvt_disable_sram_shutdown() and nvt_enable_sram_shutdown().
*/
typedef enum {
	MAU_SD = 0,       //< Shut Down MAU SRAM
	MAU2_SD,          //< Shut Down MAU2 SRAM
	SMC_SD,           //< Shut Down SMC SRAM
	JPG_SD,           //< Shut Down JPG SRAM
	OSG_SD = 4,       //< Shut Down OSG SRAM
	SSCA_SD,          //< Shut Down SSCA SRAM
	//5
	AGE_SD,           //< Shut Down AGE SRAM
	USB_SD = 7,       //< Shut Down USB SRAM
	USB2_SD,          //< Shut Down USB2 SRAM
	ETH_SD,           //< Shut Down ETH SRAM
	ETH2_SD,          //< Shut Down ETH2 SRAM
	//10
	SDIO2_SD,         //< Shut Down SDIO2 SRAM
	HWCP_SD,          //< Shut Down HWCP SRAM

	CNN_SD = 16,      //< Shut Down CNN SRAM
	VPE_SD,           //< Shut Down VPE SRAM
	VPEL_SD,          //< Shut Down VPEL SRAM
	SATA3_SD,         //< Shut Down SATA3 SRAM
	//19

	VENC_SD = 24,     //< Shut Down VENC SRAM
	USB3_SD,          //< Shut Down USB3 SRAM
	//25

	VDEC_SD = 32,     //< Shut Down VDEC SRAM
	DEI_SD,           //< Shut Down DEI SRAM
	DEI2_SD,          //< Shut Down DEI2 SRAM
	//34

	VCAP_SD = 40,     //< Shut Down VCAP SRAM
	VCAP2_SD,         //< Shut Down VCAP2 SRAM
	LCD310_SD,        //< Shut Down LCD310 SRAM
	LCD3102_SD,       //< Shut Down LCD3102 SRAM
	LCD210_SD,        //< Shut Down LCD210 SRAM
	//44

	VENC2_SD = 48,    //< Shut Down VENC2 SRAM
	SATA_SD,          //< Shut Down SATA SRAM
	SATA2_SD,         //< Shut Down SATA2 SRAM
	//51

	CNN2_SD = 56,     //< Shut Down CNN2 SRAM
	CNN3_SD,          //< Shut Down CNN3 SRAM
	NUE_SD,           //< Shut Down NUE SRAM
	NUE2_SD,          //< Shut Down NUE2 SRAM
	IVE_SD,           //< Shut Down IVE SRAM
	RSA_SD,           //< Shut Down RSA SRAM

	//ENUM_DUMMY4WORD(SRAM_SD)
} SRAM_SD;


/*
    QOS ID

    This is for nvt_set_qos. level = 0(low)~15(high)
*/
typedef enum {
	CNN_QOS   =  0,
	VPE_QOS   =  4,
	VPEL_QOS  =  8,
	VDEC_QOS  = 12,
	DEI_QOS   = 16,
	DEI2_QOS  = 20,
	VCAP_QOS  = 24,
	VCAP2_QOS = 28,

	LCD_QOS   =  0+32,
	LCD2_QOS  =  4+32,
	GPDEC_QOS =  8+32,
	DAI_QOS   = 12+32,
	DAI2_QOS  = 16+32,
	DAI5_QOS  = 20+32,
	CNN2_QOS  = 24+32,
	CNN3_QOS  = 28+32,

	NUE_QOS   =  0+64,
	NUE2_QOS  =  4+64,
	IVE_QOS   =  8+64,
	SCE_QOS   = 12+64,
	HASH_QOS  = 16+64,
	AGE_QOS   = 20+64,
	DAI3_QOS  = 24+64,
	DAI4_QOS  = 28+64,

	ETH_QOS   =  0+96,
	ETH2_QOS  =  4+96,
	GPENC_QOS =  8+96,
	GPENC2_QOS= 12+96,
	HVYLD_QOS = 16+96,
	HWCP_QOS  = 20+96,
	JPG_QOS   = 24+96,
	LARB_QOS  = 28+96,

	OSG_QOS   =  0+128,
	SATA_QOS  =  4+128,
	SATA2_QOS =  8+128,
	SATA3_QOS = 12+128,
	PCIE_QOS  = 16+128,
	SDIO_QOS  = 20+128,
	SDIO2_QOS = 24+128,
	SMC_QOS   = 28+128,

	SSCA_QOS  =  0+160,
	USB_QOS   =  4+160,
	USB2_QOS  =  8+160,
	USB3_QOS  = 12+160,

	//ENUM_DUMMY4WORD(QOS_ID)
} QOS_ID;
/*
    QOS LEVEL

    This is for nvt_set_qos. level = 0(low)~15(high)
*/
typedef enum {
	QOS_LEVEL_0   =  0,
	QOS_LEVEL_1,
	QOS_LEVEL_2,
	QOS_LEVEL_3,
	QOS_LEVEL_4,
	QOS_LEVEL_5,
	QOS_LEVEL_6,
	QOS_LEVEL_7,
	QOS_LEVEL_8,
	QOS_LEVEL_9,
	QOS_LEVEL_10,
	QOS_LEVEL_11,
	QOS_LEVEL_12,
	QOS_LEVEL_13,
	QOS_LEVEL_14,
	QOS_LEVEL_15,

	//ENUM_DUMMY4WORD(QOS_LEVEL)
} QOS_LEVEL;

extern void nvt_disable_sram_shutdown(SRAM_SD id);
extern void nvt_enable_sram_shutdown(SRAM_SD id);
extern void nvt_set_qos(QOS_ID id, QOS_LEVEL level);

#endif /* _NT98636_NVT_SRAMCTL_H */
