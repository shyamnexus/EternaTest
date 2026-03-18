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
	SDIO_SD,          //< Shut Down SDIO SRAM
	HWCP_SD,          //< Shut Down HWCP SRAM
	CNN_SD = 13,      //< Shut Down CNN SRAM
	NUE_SD,           //< Shut Down NUE SRAM
	NUE2_SD,          //< Shut Down NUE2 SRAM
	//15
	VPE_SD = 16,      //< Shut Down VPE SRAM
	VENC_SD,          //< Shut Down VENC SRAM
	VDEC_SD,          //< Shut Down VDEC SRAM
	DEI_SD,           //< Shut Down DEI SRAM
	VCAP_SD = 20,     //< Shut Down VCAP SRAM
	//20
	LCD310_SD,        //< Shut Down LCD310 SRAM
	LCD210_SD,        //< Shut Down LCD210 SRAM
	SATA_SD,          //< Shut Down SATA SRAM
	SATA2_SD,         //< Shut Down SATA2 SRAM
	//24

	RSA_SD = 32,      //< Shut Down RSA SRAM

	//ENUM_DUMMY4WORD(SRAM_SD)
} SRAM_SD;
#define SDIO2_SD SDIO_SD //backward compatible

/*
    QOS ID

    This is for nvt_set_qos. level = 0(low)~15(high)
*/
typedef enum {
	DAI3_QOS    =  0,
	DAI4_QOS    =  4,
	GPENC_QOS   =  8,
	GPENC2_QOS  = 12,
	JPG_QOS     = 16,
	LARB_QOS    = 20,
	OSG_QOS     = 24,
	SMC_QOS     = 28,

	AGE_QOS     =  0+32,
	HVYLD2_QOS  =  4+32,
	HWCP_QOS    =  8+32,
	SDIO_QOS    = 12+32,
	SSCA_QOS    = 16+32,

	CNN_QOS     =  0+64,
	NUE_QOS     =  4+64,
	NUE2_QOS    =  8+64,
	VPE_QOS     = 12+64,
	VPE2_QOS    = 16+64,
	ETH_QOS     = 20+64,
	ETH2_QOS    = 24+64,
	USB_QOS     = 28+64,

	USB2_QOS    =  0+96,

	VENC_QOS    =  0+128,
	HVYLD3_QOS  =  4+128,

	VDEC_QOS    =  0+160,
	VDEC2_QOS   =  4+160,
	DEI_QOS     =  8+160,
	SATA_QOS    = 12+160,
	SATA2_QOS   = 16+160,
	SCE_QOS     = 20+160,
	HASH_QOS    = 24+160,

	VCAP_QOS    =  0+192,
	VCAP2_QOS   =  4+192,
	VCAP3_QOS   =  8+192,
	LCD310_QOS  = 12+192,
	LCD210_QOS  = 16+192,
	GPDEC_QOS   = 20+192,
	DAI_QOS     = 24+192,
	DAI2_QOS    = 28+192,

	DAI5_QOS    =  0+224,

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
