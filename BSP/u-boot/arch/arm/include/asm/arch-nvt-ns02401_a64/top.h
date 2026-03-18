/*
	TOP controller header

	Sets the pinmux of each module.

	@file       top.h
	@ingroup    mIDrvSys_TOP
	@note       Refer NS02401 data sheet for PIN/PAD naming

	Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_NS02401_TOP_H
#define __ASM_ARCH_NS02401_TOP_H

#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/rcw_macro.h>
#include "nvt-ns02401-pinctrl.h"
#if !defined(__LINUX)
#include <stdio.h>
#endif

typedef u32  PIN_FUNC;                  ///< For top.h usage reference
typedef u32  PINMUX_LCD_SEL;            ///< For panel driver reference

/**
    Debug port select ID

    Debug port select value for pinmux_select_debugport().
*/
typedef enum {
	PINMUX_DEBUGPORT_CKG =          0x0000,     ///< CKGen
	PINMUX_DEBUGPORT_MAU =          0x0001,     ///< MAU
	PINMUX_DEBUGPORT_GPIO =         0x0002,     ///< GPIO
	PINMUX_DEBUGPORT_CPU =          0x0003,     ///< CPU
	PINMUX_DEBUGPORT_TIMER =        0x0004,     ///< Timer
	PINMUX_DEBUGPORT_TIMER2 =       0x0005,     ///< Timer2
	PINMUX_DEBUGPORT_TIMER3 =       0x0006,     ///< Timer3
	PINMUX_DEBUGPORT_CC =           0x0007,     ///< CC
	PINMUX_DEBUGPORT_WDT =          0x0008,     ///< WDT
	PINMUX_DEBUGPORT_INTC =         0x0009,     ///< INTC
	PINMUX_DEBUGPORT_PWM =          0x000A,     ///< PWM
	PINMUX_DEBUGPORT_REMOTE =       0x000B,     ///< Remote
	PINMUX_DEBUGPORT_TRNG =         0x000C,     ///< TRNG
	PINMUX_DEBUGPORT_SDIO =         0x000D,     ///< SDIO
	PINMUX_DEBUGPORT_SDIO2 =        0x000E,     ///< SDIO2
	PINMUX_DEBUGPORT_SDIO3 =        0x000F,     ///< SDIO3
	PINMUX_DEBUGPORT_SMC =          0x0010,     ///< NAND/SMC
	PINMUX_DEBUGPORT_DAI =          0x0011,     ///< DAI
	PINMUX_DEBUGPORT_DAI2 =         0x0012,     ///< DAI2
	PINMUX_DEBUGPORT_AUDIO =        0x0013,     ///< AUDIO
	PINMUX_DEBUGPORT_TSE =          0x0014,     ///< TSE
	PINMUX_DEBUGPORT_IFE =          0x0015,     ///< IFE
	PINMUX_DEBUGPORT_UART2 =        0x0016,     ///< UART2
	PINMUX_DEBUGPORT_UART3 =        0x0017,     ///< UART3
	PINMUX_DEBUGPORT_UART4 =        0x0018,     ///< UART4
	PINMUX_DEBUGPORT_UART5 =        0x0019,     ///< UART5
	PINMUX_DEBUGPORT_UART6 =        0x001A,     ///< UART6
	PINMUX_DEBUGPORT_UART7 =        0x001B,     ///< UART7
	PINMUX_DEBUGPORT_UART8 =        0x001C,     ///< UART8
	PINMUX_DEBUGPORT_UART9 =        0x001D,     ///< UART9
	PINMUX_DEBUGPORT_I2C =     	    0x001E,     ///< I2C
	PINMUX_DEBUGPORT_I2C2 =         0x001F,     ///< I2C2
	PINMUX_DEBUGPORT_I2C3 =         0x0020,     ///< I2C3
	PINMUX_DEBUGPORT_I2C4 =         0x0021,     ///< I2C4
	PINMUX_DEBUGPORT_I2C5 =         0x0022,     ///< I2C5
	PINMUX_DEBUGPORT_I2C6 =         0x0023,     ///< I2C6
	PINMUX_DEBUGPORT_I2C7 =         0x0024,     ///< I2C7
	PINMUX_DEBUGPORT_I2C8 =         0x0025,     ///< I2C8
	PINMUX_DEBUGPORT_I2C9 =         0x0026,     ///< I2C9
	PINMUX_DEBUGPORT_I2C10 =        0x0027,     ///< I2C10
	PINMUX_DEBUGPORT_I2C11 =        0x0028,     ///< I2C11
	PINMUX_DEBUGPORT_SPI =          0x0029,     ///< SPI
	PINMUX_DEBUGPORT_SPI2 =         0x002A,     ///< SPI2
	PINMUX_DEBUGPORT_SPI3 =         0x002B,     ///< SPI3
	PINMUX_DEBUGPORT_SPI4 =         0x002C,     ///< SPI4
	PINMUX_DEBUGPORT_SPI5 =         0x002D,     ///< SPI5
	PINMUX_DEBUGPORT_SIF =          0x002E,     ///< SIF
	PINMUX_DEBUGPORT_DRTC =         0x002F,     ///< DRTC
	PINMUX_DEBUGPORT_SDP =          0x0030,     ///< SDP
	PINMUX_DEBUGPORT_ADM =          0x0031,     ///< ADM
	PINMUX_DEBUGPORT_TSEN =         0x0032,     ///< TSEN
	PINMUX_DEBUGPORT_HASH =         0x0033,     ///< HASH
	PINMUX_DEBUGPORT_SCE =          0x0034,     ///< SCE
	PINMUX_DEBUGPORT_HWCP =         0x0035,     ///< HWCP
	PINMUX_DEBUGPORT_RSA =          0x0036,     ///< RSA
	PINMUX_DEBUGPORT_DSI =          0x0037,     ///< DSI
	PINMUX_DEBUGPORT_CSITX =        0x0038,     ///< CSITX
	PINMUX_DEBUGPORT_MPU =          0x0039,     ///< MPU
	PINMUX_DEBUGPORT_USB =          0x003A,     ///< USB
	PINMUX_DEBUGPORT_USB3 =         0x003B,     ///< USB3
	PINMUX_DEBUGPORT_ETH =          0x003C,     ///< ETH
	PINMUX_DEBUGPORT_HVYLD =        0x003D,     ///< HVYLD
	PINMUX_DEBUGPORT_UVCP =         0x003E,     ///< UVCP
	PINMUX_DEBUGPORT_UVCP2 =        0x003F,     ///< UVCP2
	PINMUX_DEBUGPORT_EFUSE =        0x0040,     ///< EFUSE
	PINMUX_DEBUGPORT_SATA =         0x0041,     ///< SATA
	PINMUX_DEBUGPORT_IDE =          0x0042,     ///< IDE
	PINMUX_DEBUGPORT_IDE2 =         0x0043,     ///< IDE2
	PINMUX_DEBUGPORT_HVYLD2 =       0x0044,     ///< HVYLD2
	PINMUX_DEBUGPORT_CSIRX =        0x0045,     ///< CSIRX
	PINMUX_DEBUGPORT_CSIRX2 =       0x0046,     ///< CSIRX2
	PINMUX_DEBUGPORT_CSIRX3 =       0x0047,     ///< CSIRX3
	PINMUX_DEBUGPORT_CSIRX4 =       0x0048,     ///< CSIRX4
	PINMUX_DEBUGPORT_CSIRX5 =       0x0049,     ///< CSIRX5
	PINMUX_DEBUGPORT_SIE =          0x004A,     ///< SIE
	PINMUX_DEBUGPORT_SIE2 =         0x004B,     ///< SIE2
	PINMUX_DEBUGPORT_SIE3 =         0x004C,     ///< SIE3
	PINMUX_DEBUGPORT_SIE4 =         0x004D,     ///< SIE4
	PINMUX_DEBUGPORT_SIE5 =         0x004E,     ///< SIE5
	PINMUX_DEBUGPORT_SIE6 =         0x004F,     ///< SIE6
	PINMUX_DEBUGPORT_VIE =          0x0050,     ///< VIE
	PINMUX_DEBUGPORT_VIE2 =         0x0051,     ///< VIE2
	PINMUX_DEBUGPORT_TGE =          0x0052,     ///< TGE
	PINMUX_DEBUGPORT_VPE =          0x0053,     ///< VPE
	PINMUX_DEBUGPORT_VPEL =         0x0054,     ///< VPEL
	PINMUX_DEBUGPORT_JPEG =         0x0055,     ///< JPEG
	PINMUX_DEBUGPORT_JPEG2 =        0x0056,     ///< JPEG2
	PINMUX_DEBUGPORT_GRAPHIC =      0x0057,     ///< GRAPHIC
	PINMUX_DEBUGPORT_GRAPHIC2 =     0x0058,     ///< GRAPHIC2
	PINMUX_DEBUGPORT_GRAPHIC3 =     0x0059,     ///< GRAPHIC3
	PINMUX_DEBUGPORT_ISE =          0x005A,     ///< ISE
	/// < 005B Reserved
	PINMUX_DEBUGPORT_MDBC =         0x005C,     ///< MDBC
	PINMUX_DEBUGPORT_DIS =          0x005D,     ///< DIS
	PINMUX_DEBUGPORT_DRE =          0x005E,     ///< DRE
	PINMUX_DEBUGPORT_IVE =          0x005F,     ///< IVE
	PINMUX_DEBUGPORT_SDE =          0x0060,     ///< SDE
	PINMUX_DEBUGPORT_TRKE =         0x0061,     ///< TRKE
	PINMUX_DEBUGPORT_CNN =          0x0062,     ///< CNN
	PINMUX_DEBUGPORT_CNN2 =         0x0063,     ///< CNN2
	PINMUX_DEBUGPORT_DCE =          0x0064,     ///< DCE
	PINMUX_DEBUGPORT_NUE =          0x0065,     ///< NUE
	PINMUX_DEBUGPORT_NUE2 =         0x0066,     ///< NUE2
	PINMUX_DEBUGPORT_IPE =          0x0067,     ///< IPE
	PINMUX_DEBUGPORT_IME =          0x0068,     ///< IME
	PINMUX_DEBUGPORT_HVYLD3 =       0x0069,     ///< HVYLD3
	PINMUX_DEBUGPORT_HDMI =         0x006A,     ///< HDMI
	PINMUX_DEBUGPORT_DSP =          0x006B,     ///< DSP
	PINMUX_DEBUGPORT_PMC =          0x006C,     ///< PMC
	PINMUX_DEBUGPORT_ETH2 =         0x006D,     ///< ETH2
	PINMUX_DEBUGPORT_STBCG =        0x006E,     ///< STBCG
	PINMUX_DEBUGPORT_VDEC =         0x006F,     ///< VDEC
	PINMUX_DEBUGPORT_PAD =          0x0070,     ///< PAD
	PINMUX_DEBUGPORT_JPEGL =        0x0071,     ///< JPEGL

	PINMUX_DEBUGPORT_GROUP_NONE =   0x0000,     ///< No debug port is output
	PINMUX_DEBUGPORT_GROUP1 =       0x0100,     ///< Output debug port to MC[18..0]
	PINMUX_DEBUGPORT_GROUP2 =       0x0200,     ///< Output debug port to LCD[18..0]

	ENUM_DUMMY4WORD(PINMUX_DEBUGPORT)
} PINMUX_DEBUGPORT;

/**
    PIN config of LCD modes

    @note For pinmux_init() with PIN_FUNC_LCD or PIN_FUNC_LCD2.\n
    For example, you can use {PIN_FUNC_LCD, PINMUX_DISPMUX_SEL_LCD | PINMUX_LCDMODE_RGB_SERIAL}
    to tell display object that PIN_FUNC_LCD is located on primary LCD pinmux,
    and it's LCD mode is RGB serial.
*/
typedef enum {
	PINMUX_LCDMODE_RGB_SERIAL           =    0,     ///< LCD MODE is RGB Serial or UPS051
	PINMUX_LCDMODE_RGB_PARALL           =    1,     ///< LCD MODE is RGB Parallel (888)
	PINMUX_LCDMODE_YUV640               =    2,     ///< LCD MODE is YUV640
	PINMUX_LCDMODE_YUV720               =    3,     ///< LCD MODE is YUV720
	PINMUX_LCDMODE_RGBD360              =    4,     ///< LCD MODE is RGB Dummy 360
	PINMUX_LCDMODE_RGBD320              =    5,     ///< LCD MODE is RGB Dummy 320
	PINMUX_LCDMODE_RGB_THROUGH          =    6,     ///< LCD MODE is RGB through mode
	PINMUX_LCDMODE_CCIR601              =    7,     ///< LCD MODE is CCIR601
	PINMUX_LCDMODE_CCIR656              =    8,     ///< LCD MODE is CCIR656
	PINMUX_LCDMODE_RGB_PARALL666        =    9,     ///< LCD MODE is RGB Parallel 666
	PINMUX_LCDMODE_RGB_PARALL565        =   10,     ///< LCD MODE is RGB Parallel 565
	PINMUX_LCDMODE_RGB_PARALL_DELTA     =   11,     ///< LCD MODE is RGB Parallel Delta
	PINMUX_LCDMODE_MIPI                 =   12,     ///< LCD MODE is MIPI Display

	PINMUX_LCDMODE_MI_OFFSET            = 32,       ///< Memory LCD MODE offset
	PINMUX_LCDMODE_MI_FMT0              = 32 + 0,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT1              = 32 + 1,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT2              = 32 + 2,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT3              = 32 + 3,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT4              = 32 + 4,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT5              = 32 + 5,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT6              = 32 + 6,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT7              = 32 + 7,   ///< LCD MODE is Memory(Parallel Interface) 9bits
	PINMUX_LCDMODE_MI_FMT8              = 32 + 8,   ///< LCD MODE is Memory(Parallel Interface) 16bits
	PINMUX_LCDMODE_MI_FMT9              = 32 + 9,   ///< LCD MODE is Memory(Parallel Interface) 16bits
	PINMUX_LCDMODE_MI_FMT10             = 32 + 10,  ///< LCD MODE is Memory(Parallel Interface) 18bits
	PINMUX_LCDMODE_MI_FMT11             = 32 + 11,  ///< LCD MODE is Memory(Parallel Interface)  8bits
	PINMUX_LCDMODE_MI_FMT12             = 32 + 12,  ///< LCD MODE is Memory(Parallel Interface) 16bits
	PINMUX_LCDMODE_MI_FMT13             = 32 + 13,  ///< LCD MODE is Memory(Parallel Interface) 16bits
	PINMUX_LCDMODE_MI_FMT14             = 32 + 14,  ///< LCD MODE is Memory(Parallel Interface) 16bits
	PINMUX_LCDMODE_MI_SERIAL_BI         = 32 + 20,  ///< LCD MODE is Serial Interface bi-direction
	PINMUX_LCDMODE_MI_SERIAL_SEP        = 32 + 21,  ///< LCD MODE is Serial Interface separation

	PINMUX_LCDMODE_AUTO_PINMUX          = 0x01 << 22, ///< Set display device to GPIO mode when display device is closed. Select this filed will inform display object to switch to GPIO when display device is closed.


	PINMUX_HDMIMODE_OFFSET              = 64,       ///< HDMI MODE offset (not support HDMI. Below is backward compatible)
	PINMUX_HDMIMODE_640X480P60          = 64 + 1,   ///< HDMI Video format is 640x480 & Progressive 60fps
	PINMUX_HDMIMODE_720X480P60          = 64 + 2,   ///< HDMI Video format is 720x480 & Progressive 60fps & 4:3
	PINMUX_HDMIMODE_720X480P60_16X9     = 64 + 3,   ///< HDMI Video format is 720x480 & Progressive 60fps & 16:9
	PINMUX_HDMIMODE_1280X720P60         = 64 + 4,   ///< HDMI Video format is 1280x720 & Progressive 60fps
	PINMUX_HDMIMODE_1920X1080I60        = 64 + 5,   ///< HDMI Video format is 1920x1080 & Interlaced 60fps
	PINMUX_HDMIMODE_720X480I60          = 64 + 6,   ///< HDMI Video format is 720x480 & Interlaced 60fps
	PINMUX_HDMIMODE_720X480I60_16X9     = 64 + 7,   ///< HDMI Video format is 720x480 & Interlaced 60fps & 16:9
	PINMUX_HDMIMODE_720X240P60          = 64 + 8,   ///< HDMI Video format is 720x240 & Progressive 60fps
	PINMUX_HDMIMODE_720X240P60_16X9     = 64 + 9,   ///< HDMI Video format is 720x240 & Progressive 60fps & 16:9
	PINMUX_HDMIMODE_1440X480I60         = 64 + 10,  ///< HDMI Video format is 1440x480 & Interlaced 60fps
	PINMUX_HDMIMODE_1440X480I60_16X9    = 64 + 11,  ///< HDMI Video format is 1440x480 & Interlaced 60fps & 16:9
	PINMUX_HDMIMODE_1440X240P60         = 64 + 12,  ///< HDMI Video format is 1440x240 & Progressive 60fps
	PINMUX_HDMIMODE_1440X240P60_16X9    = 64 + 13,  ///< HDMI Video format is 1440x240 & Progressive 60fps & 16:9
	PINMUX_HDMIMODE_1440X480P60         = 64 + 14,  ///< HDMI Video format is 1440x480 & Progressive 60fps
	PINMUX_HDMIMODE_1440X480P60_16X9    = 64 + 15,  ///< HDMI Video format is 1440x480 & Progressive 60fps & 16:9
	PINMUX_HDMIMODE_720X576P50          = 64 + 17,  ///< HDMI Video format is 720x576 & Progressive 50fps
	PINMUX_HDMIMODE_720X576P50_16X9     = 64 + 18,  ///< HDMI Video format is 720x576 & Progressive 50fps & 16:9
	PINMUX_HDMIMODE_1280X720P50         = 64 + 19,  ///< HDMI Video format is 1280x720 & Progressive 50fps
	PINMUX_HDMIMODE_1920X1080I50        = 64 + 20,  ///< HDMI Video format is 1920x1080 & Interlaced 50fps
	PINMUX_HDMIMODE_720X576I50          = 64 + 21,  ///< HDMI Video format is 720x576 & Interlaced 50fps
	PINMUX_HDMIMODE_720X576I50_16X9     = 64 + 22,  ///< HDMI Video format is 720x576 & Interlaced 50fps & 16:9
	PINMUX_HDMIMODE_720X288P50          = 64 + 23,  ///< HDMI Video format is 720x288 & Progressive 50fps
	PINMUX_HDMIMODE_720X288P50_16X9     = 64 + 24,  ///< HDMI Video format is 720x288 & Progressive 50fps & 16:9
	PINMUX_HDMIMODE_1440X576I50         = 64 + 25,  ///< HDMI Video format is 1440x576 & Interlaced 50fps
	PINMUX_HDMIMODE_1440X576I50_16X9    = 64 + 26,  ///< HDMI Video format is 1440x576 & Interlaced 50fps & 16:9
	PINMUX_HDMIMODE_1440X288P50         = 64 + 27,  ///< HDMI Video format is 1440x288 & Progressive 50fps
	PINMUX_HDMIMODE_1440X288P50_16X9    = 64 + 28,  ///< HDMI Video format is 1440x288 & Progressive 50fps & 16:9
	PINMUX_HDMIMODE_1440X576P50         = 64 + 29,  ///< HDMI Video format is 1440x576 & Progressive 50fps
	PINMUX_HDMIMODE_1440X576P50_16X9    = 64 + 30,  ///< HDMI Video format is 1440x576 & Progressive 50fps & 16:9
	PINMUX_HDMIMODE_1920X1080P50        = 64 + 31,  ///< HDMI Video format is 1920x1080 & Progressive 50fps
	PINMUX_HDMIMODE_1920X1080P24        = 64 + 32,  ///< HDMI Video format is 1920x1080 & Progressive 24fps
	PINMUX_HDMIMODE_1920X1080P25        = 64 + 33,  ///< HDMI Video format is 1920x1080 & Progressive 25fps
	PINMUX_HDMIMODE_1920X1080P30        = 64 + 34,  ///< HDMI Video format is 1920x1080 & Progressive 30fps
	PINMUX_HDMIMODE_1920X1080I50_VT1250 = 64 + 39,  ///< HDMI Video format is 1920x1080 & Interlaced  50fps & V-total is 1250 lines
	PINMUX_HDMIMODE_1920X1080I100       = 64 + 40,  ///< HDMI Video format is 1920x1080 & Interlaced  100fps
	PINMUX_HDMIMODE_1280X720P100        = 64 + 41,  ///< HDMI Video format is 1280X720  & Progressive 100fps
	PINMUX_HDMIMODE_720X576P100         = 64 + 42,  ///< HDMI Video format is 720X576   & Progressive 100fps
	PINMUX_HDMIMODE_720X576P100_16X9    = 64 + 43,  ///< HDMI Video format is 720X576   & Progressive 100fps & 16:9
	PINMUX_HDMIMODE_720X576I100         = 64 + 44,  ///< HDMI Video format is 720X576  & Interlaced 100fps
	PINMUX_HDMIMODE_720X576I100_16X9    = 64 + 45,  ///< HDMI Video format is 720X576  & Interlaced 100fps & 16:9
	PINMUX_HDMIMODE_1920X1080I120       = 64 + 46,  ///< HDMI Video format is 1920X1080 & Interlaced 120fps
	PINMUX_HDMIMODE_1280X720P120        = 64 + 47,  ///< HDMI Video format is 1280X720  & Progressive 120fps
	PINMUX_HDMIMODE_720X480P120         = 64 + 48,  ///< HDMI Video format is 720X480   & Progressive 120fps
	PINMUX_HDMIMODE_720X480P120_16X9    = 64 + 49,  ///< HDMI Video format is 720X480   & Progressive 120fps & 16:9
	PINMUX_HDMIMODE_720X480I120         = 64 + 50,  ///< HDMI Video format is 720X480  & Interlaced 120fps
	PINMUX_HDMIMODE_720X480I120_16X9    = 64 + 51,  ///< HDMI Video format is 720X480  & Interlaced 120fps & 16:9
	PINMUX_HDMIMODE_720X576P200         = 64 + 52,  ///< HDMI Video format is 720X576  & Progressive 200fps
	PINMUX_HDMIMODE_720X576P200_16X9    = 64 + 53,  ///< HDMI Video format is 720X576  & Progressive 200fps & 16:9
	PINMUX_HDMIMODE_720X576I200         = 64 + 54,  ///< HDMI Video format is 720X576  & Interlaced 200fps
	PINMUX_HDMIMODE_720X576I200_16X9    = 64 + 55,  ///< HDMI Video format is 720X576  & Interlaced 200fps & 16:9
	PINMUX_HDMIMODE_720X480P240         = 64 + 56,  ///< HDMI Video format is 720X480  & Progressive 240fps
	PINMUX_HDMIMODE_720X480P240_16X9    = 64 + 57,  ///< HDMI Video format is 720X480  & Progressive 240fps & 16:9
	PINMUX_HDMIMODE_720X480I240         = 64 + 58,  ///< HDMI Video format is 720X480  & Interlaced 240fps
	PINMUX_HDMIMODE_720X480I240_16X9    = 64 + 59,  ///< HDMI Video format is 720X480  & Interlaced 240fps & 16:9

	PINMUX_DSI_1_LANE_CMD_MODE_RGB565           = 128 + 0, ///< DSI command mode with RGB565 format   (not support DSI. Below is backward compatible)
	PINMUX_DSI_1_LANE_CMD_MODE_RGB666P          = 128 + 1, ///< DSI command mode with RGB666 packed
	PINMUX_DSI_1_LANE_CMD_MODE_RGB666L          = 128 + 2, ///< DSI command mode with RGB666 loosely
	PINMUX_DSI_1_LANE_CMD_MODE_RGB888           = 128 + 3, ///< DSI command mode with RGB888

	PINMUX_DSI_1_LANE_VDO_SYNC_PULSE_RGB565     = 128 + 4, ///< DSI video sync pulse mode with RGB565 format
	PINMUX_DSI_1_LANE_VDO_SYNC_PULSE_RGB666P    = 128 + 5, ///< DSI video sync pulse mode with RGB666 packed
	PINMUX_DSI_1_LANE_VDO_SYNC_PULSE_RGB666L    = 128 + 6, ///< DSI video sync pulse mode with RGB666 loosely
	PINMUX_DSI_1_LANE_VDO_SYNC_PULSE_RGB888     = 128 + 7, ///< DSI video sync pulse mode with RGB888

	PINMUX_DSI_1_LANE_VDO_SYNC_EVENT_RGB565     = 128 + 8, ///< DSI video sync event burst mode with RGB565 format
	PINMUX_DSI_1_LANE_VDO_SYNC_EVENT_RGB666P    = 128 + 9, ///< DSI video sync event burst mode with RGB666 packed
	PINMUX_DSI_1_LANE_VDO_SYNC_EVENT_RGB666L    = 128 + 10, ///< DSI video sync event burst mode with RGB666 loosely
	PINMUX_DSI_1_LANE_VDO_SYNC_EVENT_RGB888     = 128 + 11, ///< DSI video sync event burst mode with RGB888

	PINMUX_DSI_2_LANE_CMD_MODE_RGB565           = 128 + 12, ///< DSI command mode with RGB565 format
	PINMUX_DSI_2_LANE_CMD_MODE_RGB666P          = 128 + 13, ///< DSI command mode with RGB666 packed
	PINMUX_DSI_2_LANE_CMD_MODE_RGB666L          = 128 + 14, ///< DSI command mode with RGB666 loosely
	PINMUX_DSI_2_LANE_CMD_MODE_RGB888           = 128 + 15, ///< DSI command mode with RGB888

	PINMUX_DSI_2_LANE_VDO_SYNC_PULSE_RGB565     = 128 + 16, ///< DSI video sync pulse mode with RGB565 format
	PINMUX_DSI_2_LANE_VDO_SYNC_PULSE_RGB666P    = 128 + 17, ///< DSI video sync pulse mode with RGB666 packed
	PINMUX_DSI_2_LANE_VDO_SYNC_PULSE_RGB666L    = 128 + 18, ///< DSI video sync pulse mode with RGB666 loosely
	PINMUX_DSI_2_LANE_VDO_SYNC_PULSE_RGB888     = 128 + 19, ///< DSI video sync pulse mode with RGB888

	PINMUX_DSI_2_LANE_VDO_SYNC_EVENT_RGB565     = 128 + 20, ///< DSI video sync event burst mode with RGB565 format
	PINMUX_DSI_2_LANE_VDO_SYNC_EVENT_RGB666P    = 128 + 21, ///< DSI video sync event burst mode with RGB666 packed
	PINMUX_DSI_2_LANE_VDO_SYNC_EVENT_RGB666L    = 128 + 22, ///< DSI video sync event burst mode with RGB666 loosely
	PINMUX_DSI_2_LANE_VDO_SYNC_EVENT_RGB888     = 128 + 23, ///< DSI video sync event burst mode with RGB888

	PINMUX_DSI_4_LANE_CMD_MODE_RGB565           = 128 + 24, ///< DSI command mode with RGB565 format
	PINMUX_DSI_4_LANE_CMD_MODE_RGB666P          = 128 + 25, ///< DSI command mode with RGB666 packed
	PINMUX_DSI_4_LANE_CMD_MODE_RGB666L          = 128 + 26, ///< DSI command mode with RGB666 loosely
	PINMUX_DSI_4_LANE_CMD_MODE_RGB888           = 128 + 27, ///< DSI command mode with RGB888

	PINMUX_DSI_4_LANE_VDO_SYNC_PULSE_RGB565     = 128 + 28, ///< DSI video sync pulse mode with RGB565 format
	PINMUX_DSI_4_LANE_VDO_SYNC_PULSE_RGB666P    = 128 + 29, ///< DSI video sync pulse mode with RGB666 packed
	PINMUX_DSI_4_LANE_VDO_SYNC_PULSE_RGB666L    = 128 + 30, ///< DSI video sync pulse mode with RGB666 loosely
	PINMUX_DSI_4_LANE_VDO_SYNC_PULSE_RGB888     = 128 + 31, ///< DSI video sync pulse mode with RGB888

	PINMUX_DSI_4_LANE_VDO_SYNC_EVENT_RGB565     = 128 + 32, ///< DSI video sync event burst mode with RGB565 format
	PINMUX_DSI_4_LANE_VDO_SYNC_EVENT_RGB666P    = 128 + 33, ///< DSI video sync event burst mode with RGB666 packed
	PINMUX_DSI_4_LANE_VDO_SYNC_EVENT_RGB666L    = 128 + 34, ///< DSI video sync event burst mode with RGB666 loosely
	PINMUX_DSI_4_LANE_VDO_SYNC_EVENT_RGB888     = 128 + 35, ///< DSI video sync event burst mode with RGB888



	ENUM_DUMMY4WORD(PINMUX_LCDINIT)
} PINMUX_LCDINIT;

/**
    PIN config for TV/HDMI

    @note For pinmux_init() for PIN_FUNC_TV or PIN_FUNC_HDMI.
    For example, you can use {PIN_FUNC_HDMI, PINMUX_TV_HDMI_CFG_NORMAL|PINMUX_HDMIMODE_1280X720P60}
    to tell display object that HDMI activation will disable PANEL,
    and HDMI mode is 1280x720 P60.
*/
typedef enum {
	PINMUX_TV_HDMI_CFG_GPIO = 0x00,                 ///< TV activation will disable PINMUX to GPIO
	PINMUX_TV_HDMI_CFG_NORMAL = 0x00,               ///< TV activation will disable PANEL which shared the same IDE
	PINMUX_TV_HDMI_CFG_PINMUX_ON = 0x01 << 28,      ///< TV activation will keep PINMUX setting

	PINMUX_TV_HDMI_CFG_MASK = 0x03 << 28,
	ENUM_DUMMY4WORD(PINMUX_TV_HDMI_CFG)
} PINMUX_TV_HDMI_CFG;

/**
    PIN config for HDMI, not support (Backward compatible)

    @note For pinmux_init() for PIN_FUNC_HDMI.\n
            For example, you can use {PIN_FUNC_HDMI, PINMUX_HDMI_CFG_CEC|PINMUX_TV_HDMI_CFG_NORMAL}\n
            to declare HDMI CEC pinmux is enabled.
*/
typedef enum {
	PINMUX_HDMI_CFG_GPIO = 0x00,                    ///< HDMI specific PIN to GPIO
	PINMUX_HDMI_CFG_HOTPLUG = 0x01 << 26,           ///< HDMI HOTPLUG.
	PINMUX_HDMI_CFG_CEC = 0x02 << 26,               ///< HDMI CEC.

	PINMUX_HDMI_CFG_MASK = 0x03 << 26,
	ENUM_DUMMY4WORD(PINMUX_HDMI_CFG)
} PINMUX_HDMI_CFG;

/**
    PIN config for USB

    @note For pinmux_init() for PIN_FUNC_USB.\n
            For example, you can use {PIN_FUNC_USB, PINMUX_USB_CFG_DEVICE}\n
            to select USB as device.
*/
typedef enum {
	PINMUX_USB_CFG_NONE = 0x00,                     ///< USB as device
	PINMUX_USB_CFG_DEVICE = 0x00,                   ///< USB as device
	PINMUX_USB_CFG_HOST = 0x01,                     ///< USB as host

	ENUM_DUMMY4WORD(PINMUX_USB_CFG)
} PINMUX_USB_CFG;

/**
    Pinmux power domain isolated function enable/disable

    @note For pinmux_set_direct_channel()
*/
typedef enum {
	PINMUX_ISOFUNC_DISABLE = 0x00,               ///< DISABLE power domain isolated function
	PINMUX_ISOFUNC_ENABLE = 0x01,                ///< ENABLE  power domain isolated function
	ENUM_DUMMY4WORD(PINMUX_POWER_ISOFUNC)
} PINMUX_POWER_ISOFUNC;

/**
    Pinmux Function identifier

    @note For pinmux_getDispMode(), pinmux_setPinmux().
*/
typedef enum {
	PINMUX_FUNC_ID_LCD,                             ///< 1st Panel (LCD), pinmux can be:
	///< - @b PINMUX_LCD_SEL_GPIO
	///< - @b PINMUX_LCD_SEL_CCIR656
	///< - @b PINMUX_LCD_SEL_CCIR656_16BITS
	///< - @b PINMUX_LCD_SEL_CCIR601
	///< - @b PINMUX_LCD_SEL_CCIR601_16BITS
	///< - @b PINMUX_LCD_SEL_SERIAL_RGB_6BITS
	///< - @b PINMUX_LCD_SEL_SERIAL_RGB_8BITS
	///< - @b PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS
	///< - @b PINMUX_LCD_SEL_PARALLE_RGB565
	///< - @b PINMUX_LCD_SEL_PARALLE_RGB666
	///< - @b PINMUX_LCD_SEL_PARALLE_RGB888
	///< - @b PINMUX_LCD_SEL_RGB_16BITS
	///< - @b PINMUX_LCD_SEL_MIPI
	///< - @b PINMUX_LCD_SEL_PARALLE_MI_8BITS
	///< - @b PINMUX_LCD_SEL_PARALLE_MI_9BITS
	///< - @b PINMUX_LCD_SEL_PARALLE_MI_16BITS
	///< - @b PINMUX_LCD_SEL_PARALLE_MI_18BITS
	///< - @b PINMUX_LCD_SEL_SERIAL_MI_SDIO
	///< - @b PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO
	///< ORed with
	///< - @b PINMUX_LCD_SEL_DE_ENABLE
	///< - @b PINMUX_LCD_SEL_TE_ENABLE
	///< - @b PINMUX_LCD_SEL_HVLD_VVLD
	///< - @b PINMUX_LCD_SEL_FIELD
	PINMUX_FUNC_ID_LCD2,                            ///< 2nd Panel (LCD), pinmux can be:
	///< - @b PINMUX_LCD_SEL_GPIO
	///< - @b PINMUX_LCD_SEL_CCIR656
	///< - @b PINMUX_LCD_SEL_CCIR601
	///< - @b PINMUX_LCD_SEL_SERIAL_RGB_6BITS
	///< - @b PINMUX_LCD_SEL_SERIAL_RGB_8BITS
	///< - @b PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS
	///< - @b PINMUX_LCD_SEL_PARALLE_MI_8BITS
	///< - @b PINMUX_LCD_SEL_PARALLE_MI_9BITS
	///< - @b PINMUX_LCD_SEL_SERIAL_MI_SDIO
	///< - @b PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO
	///< ORed with
	///< - @b PINMUX_LCD_SEL_DE_ENABLE
	PINMUX_FUNC_ID_LCD3,                            ///< 3nd Panel (LCD), pinmux can be:
	PINMUX_FUNC_ID_TV,                              ///< TV, pinmux can be:
	///< - @b PINMUX_LCD_SEL_GPIO
	PINMUX_FUNC_ID_HDMI,                            ///< HDMI, pinmux can be:
	///< - @b PINMUX_LCD_SEL_GPIO
	PINMUX_FUNC_ID_COUNT,                           //< Total function count

	ENUM_DUMMY4WORD(PINMUX_FUNC_ID)
} PINMUX_FUNC_ID;

/**
    Pinmux selection for Storage

    @note For PINMUX_FUNC_ID_SDIO, PINMUX_FUNC_ID_SPI, PINMUX_FUNC_ID_NAND
*/
typedef enum {
	PINMUX_STORAGE_SEL_INACTIVE,                    ///< Inactive storage
	PINMUX_STORAGE_SEL_ACTIVE,                      ///< Active storage
	PINMUX_STORAGE_SEL_INEXIST,                     ///< Inexist storage
	PINMUX_STORAGE_SEL_EXIST,                       ///< Exist storage
	ENUM_DUMMY4WORD(PINMUX_STORAGE_SEL)
} PINMUX_STORAGE_SEL;

/*
    Pinmux Function identifier for driver only

    @note For pinmux_setPinmux().
*/
typedef enum {
	PINMUX_FUNC_ID_SDIO = 0x8000000,                ///< SDIO, pinmux can be:
	///< - @b PINMUX_STORAGE_SEL_INACTIVE
	///< - @b PINMUX_STORAGE_SEL_ACTIVE
	PINMUX_FUNC_ID_SDIO2,                           ///< SDIO2, pinmux can be:
	///< - @b PINMUX_STORAGE_SEL_INACTIVE
	///< - @b PINMUX_STORAGE_SEL_ACTIVE
	PINMUX_FUNC_ID_SDIO3,                           ///< SDIO3, pinmux can be:
	///< - @b PINMUX_STORAGE_SEL_INACTIVE
	///< - @b PINMUX_STORAGE_SEL_ACTIVE
	PINMUX_FUNC_ID_SPI,                             ///< SPI, pinmux can be:
	///< - @b PINMUX_STORAGE_SEL_INACTIVE
	///< - @b PINMUX_STORAGE_SEL_ACTIVE
	PINMUX_FUNC_ID_NAND,                            ///< NAND, pinmux can be:
	///< - @b PINMUX_STORAGE_SEL_INACTIVE
	///< - @b PINMUX_STORAGE_SEL_ACTIVE
	PINMUX_FUNC_ID_BMC,                             ///< BMC, pinmux can be:
	///< - @b PINMUX_STORAGE_SEL_INACTIVE
	///< - @b PINMUX_STORAGE_SEL_ACTIVE
	PINMUX_FUNC_ID_USB_VBUSI,                       ///< USB VBUSI, pinmux can be:
	///< - @b PINMUX_USB_SEL_INACTIVE
	///< - @b PINMUX_USB_SEL_ACTIVE
	PINMUX_FUNC_ID_USB2_VBUSI,                      ///< USB2 VBUSI, pinmux can be:
	///< - @b PINMUX_USB_SEL_INACTIVE
	///< - @b PINMUX_USB_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C1_1ST,                        ///< I2C channel1 1st pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C1_2ND,                        ///< I2C channel1 2nd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C2_1ST,                        ///< I2C channel2 1st pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C2_2ND,                        ///< I2C channel2 2nd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C3_1ST,                        ///< I2C channel3 1st pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C3_2ND,                        ///< I2C channel3 2nd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C3_3RD,                        ///< I2C channel3 3rd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C4_1ST,                        ///< I2C channel4 1st pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C4_2ND,                        ///< I2C channel4 2nd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C4_3RD,                        ///< I2C channel4 3rd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C4_4TH,                        ///< I2C channel4 4th pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C5_1ST,                        ///< I2C channel5 1st pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C5_2ND,                        ///< I2C channel5 2nd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_SN_VDHD,                         ///< SN VD/HD, pinmux can be:
	///< - @b PINMUX_SENSOR_SEL_INACTIVE
	///< - @b PINMUX_SENSOR_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_0,                           ///< PWM channel 0
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_1,                           ///< PWM channel 1
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_2,                           ///< PWM channel 2
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_3,                           ///< PWM channel 3
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_4,                           ///< PWM channel 4
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_5,                           ///< PWM channel 5
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_6,                           ///< PWM channel 6
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_7,                           ///< PWM channel 7
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_8,                           ///< PWM channel 8
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_9,                           ///< PWM channel 9
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_10,                          ///< PWM channel 10
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_11,                          ///< PWM channel 11
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_12,                          ///< PWM channel 12
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_13,                          ///< PWM channel 13
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_14,                          ///< PWM channel 14
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_15,                          ///< PWM channel 15
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_16,                          ///< PWM channel 16
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_17,                          ///< PWM channel 17
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_18,                          ///< PWM channel 18
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_19,                          ///< PWM channel 19
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_0,                           ///< SIF channel 0
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_1,                           ///< SIF channel 1
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_2,                           ///< SIF channel 2
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_3,                           ///< SIF channel 3
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_4,                           ///< SIF channel 4
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_5,                           ///< SIF channel 5
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_6,                           ///< SIF channel 6
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_7,                           ///< SIF channel 7
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_UART_1,                          ///< UART channel 1
	///< - @b PINMUX_UART_SEL_INACTIVE
	///< - @b PINMUX_UART_SEL_ACTIVE
	PINMUX_FUNC_ID_UART_2,                          ///< UART channel 2
	///< - @b PINMUX_UART_SEL_INACTIVE
	///< - @b PINMUX_UART_SEL_ACTIVE
	PINMUX_FUNC_ID_UART_3,                          ///< UART channel 3
	///< - @b PINMUX_UART_SEL_INACTIVE
	///< - @b PINMUX_UART_SEL_ACTIVE
	PINMUX_FUNC_ID_UART_4,                          ///< UART channel 4
	///< - @b PINMUX_UART_SEL_INACTIVE
	///< - @b PINMUX_UART_SEL_ACTIVE
	PINMUX_FUNC_ID_I2S,                             ///< I2S
	///< - @b PINMUX_I2S_SEL_INACTIVE
	///< - @b PINMUX_I2S_SEL_ACTIVE
	PINMUX_FUNC_ID_I2S_MCLK,                        ///< I2S MCLK
	///< - @b PINMUX_I2S_SEL_INACTIVE
	///< - @b PINMUX_I2S_SEL_ACTIVE
	PINMUX_FUNC_ID_DSP_PERI_CH_EN,                  ///< DSP PERI channel enable
	///< - @b PINMUX_DSP_CH_INACTIVE
	///< - @b PINMUX_DSP_CH_ACTIVE
	PINMUX_FUNC_ID_DSP_IOP_CH_EN,                   ///< DSP IOP channel enable
	///< - @b PINMUX_DSP_CH_INACTIVE
	///< - @b PINMUX_DSP_CH_ACTIVE
	PINMUX_FUNC_ID_DSP2_PERI_CH_EN,                 ///< DSP2 PERI channel enable
	///< - @b PINMUX_DSP_CH_INACTIVE
	///< - @b PINMUX_DSP_CH_ACTIVE
	PINMUX_FUNC_ID_DSP2_IOP_CH_EN,                  ///< DSP2 IOP channel enable
	///< - @b PINMUX_DSP_CH_INACTIVE
	///< - @b PINMUX_DSP_CH_ACTIVE

	ENUM_DUMMY4WORD(PINMUX_FUNC_ID_DRV)
} PINMUX_FUNC_ID_DRV;

/**
    Pinmux selection for boot source

    @note For BOOT_SRC_ENUM
*/
typedef enum
{
	BOOT_SRC_SPINOR,                    //< Boot from SPI NOR
	BOOT_SRC_CARD,                      //< Boot from CARD
	BOOT_SRC_SPINAND,                   //< Boot from SPI NAND with on die ECC (2K page)
	BOOT_SRC_SPINAND_RS,                //< Boot from SPI NAND (Reed Solomon ECC) (2K page)
	BOOT_SRC_ETH,                       //< Boot from Ethernet
	BOOT_SRC_USB_HS,                    //< Boot from USB (High Speed)
	BOOT_SRC_SPINAND_4K,                //< Boot from SPI NAND with on die ECC (4K page)
	BOOT_SRC_RESERVED,                  //< RESERVED
	BOOT_SRC_EMMC_4BIT,                 //< Boot from eMMC 4-bit
	BOOT_SRC_EMMC_8BIT,                 //< Boot from eMMC 8-bit
	BOOT_SRC_SPINAND_RS_4K,             //< Boot from SPI NAND (Reed Solomon ECC) (4K page)
	BOOT_SRC_USB_FS,                    //< Boot from USB (Full Speed)
	BOOT_PCIE_EP,
	BOOT_ETH_RMII_OUT,
	BOOT_ETH_RMII_IN,
	ENUM_DUMMY4WORD(BOOT_SRC_ENUM)
} BOOT_SRC_ENUM;

typedef struct {
	PIN_FUNC    pin_function;           ///< PIN Function group
	UINT32      config;                 ///< Configuration for pinFunction
} PIN_GROUP_CONFIG;


/*
enum CHIP_ID {
	CHIP_NA51055 = 0x4821,
	CHIP_NA51084 = 0x5021,
	CHIP_NA51089 = 0x7021,
	CHIP_NA51090 = 0xBC21,
	CHIP_NA51102 = 0x5221,
};*/

int nvt_pinmux_capture(PIN_GROUP_CONFIG *pinmux_config, int count);
int nvt_pinmux_update(PIN_GROUP_CONFIG *pinmux_config, int count);
int pinmux_set_config(PINMUX_FUNC_ID id, uint32_t pinmux);
extern void gpio_func_show(void);
int nvt_pinmux_probe(void);
//UINT32 nvt_get_chip_id(void);
PINMUX_LCDINIT pinmux_get_dispmode(PINMUX_FUNC_ID id);

int nvt_pinmux_init(void);
int pinmux_select_debugport(PINMUX_DEBUGPORT uiDebug);
UINT32 top_get_bs(void);

#endif
