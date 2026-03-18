/*
    @file       h26x_reg.h
    @ingroup    mIH26X

    @brief      Header file for H264 module register.
                This file is the header file that define register for H264 module.

    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2009.  All rights reserved.
*/

/*
    @addtogroup mIH264
*/

#ifndef _H26X_REG_H_
#define _H26X_REG_H_

#include "h26x.h"

#define H26X_SWRST					0x00000010
#define H26X_START					0x00000100
#define H26X_LOAD					0x80000000
#define H26X_INIT_INT_STATUS		0x05101711

// CTRL bit set //
#define H26X_SWRST_BIT				4
#define H26X_START_BIT				8
#define H26X_BUSY_BIT				12
#define H26X_DRM_SEL_BIT			16
#define H26X_BSDMA_CMD_EN_BIT		20
#define H26X_BSOUT_EN_BIT			24
#define H26X_SLICE_SET_EN_BIT		26
#define H26X_DMA_CH_DIS_BIT			27
#define H26X_CHKSUM_EN_BIT			28
#define H26X_CLOCK_EN_BIT	        29
#define H26X_CHK_SUN_MUX_EN_BIT		30
#define H26X_PCLOCK_EN_BIT	        31

typedef struct _H26X_HW_REG{
	volatile UINT32 CTRL;                       // 0x000
	volatile UINT32 INT_FLAG;                   // 0x004
	volatile UINT32 SRAM_PW_CFG;				// 0x008
	volatile UINT32 HW_VERSION;                 // 0x00C
	volatile UINT32 LLC_CFG_0;					// 0x010
	volatile UINT32 LLC_CFG_1;					// 0x014
	volatile UINT32 LLC_CFG_2;					// 0x018
	volatile UINT32 RES_01C; 					// 0x01C
	volatile H26XRegSet REG_SET;				// 0x020  ~  0xEB4
}H26X_HW_REG;

typedef struct _H26X_HW_WRP_REG{
	volatile UINT32 CODEC_EN;					// 0x000
	volatile UINT32 DMA_WRP;					// 0x004
	volatile UINT32 DMA_AXI;					// 0x008
	volatile UINT32 DMA_AXI_2;					// 0x00C
	volatile UINT32 WRP_0_CFG;					// 0x010
	volatile UINT32 WRP_1_CFG;					// 0x014
	volatile UINT32 WRP_2_CFG;					// 0x018
	volatile UINT32 WRP_0_CACHE;				// 0x01C
	volatile UINT32 WRP_1_CACHE;				// 0x020
	volatile UINT32 WRP_RESERVE_24_84[25];		// 0x024 ~ 0x084
	volatile UINT32 WRP_SUM1;					// 0x088
	volatile UINT32 WRP_SUM2;					// 0x08C
	volatile UINT32 WRP_DBG1;					// 0x090
	volatile UINT32 WRP_DBG2;					// 0x094
	volatile UINT32 WRP_DBG3;					// 0x098
	volatile UINT32 WRP_DBG4;					// 0x09C
	volatile UINT32 WRP_DBG5;					// 0x0A0
	volatile UINT32 WRP_DBG6;					// 0x0A4
	volatile UINT32 WRP_DBG7;					// 0x0A8
	volatile UINT32 WRP_DBG8;					// 0x0AC
}H26X_HW_WRP_REG;


#endif
