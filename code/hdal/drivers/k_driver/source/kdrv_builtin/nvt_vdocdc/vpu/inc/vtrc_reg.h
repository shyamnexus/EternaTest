#ifndef _VTRC_REG_H_
#define _VTRC_REG_H_

#include "h26x.h"
#include "vtrc.h"

#define VTRC_SWRST					0x00000001
#define VTRC_START					0x00000002
#define VTRC_BUSY					0x00000004

// CTRL bit set //
#define VTRC_SWRST_BIT				0
#define VTRC_START_BIT				1
#define VTRC_BUSY_BIT				2
#define VTRC_AE_32B_BURST_BIT		3
#define VTRC_DMA_CH_DIS_BIT			4
#define VTRC_CLOCK_EN_BIT	        5
#define VTRC_PCLOCK_EN_BIT	        6
#define VTRC_SHA256_EN_BIT			7
#define VTRC_BVALIED_EN_BIT			19

typedef struct _VTRC_HW_REG{
	#if 0	// using for nt98690 link-list ctrl, nt98530 not ready )
	volatile UINT32 CTRL;						// 0x000 
	volatile UINT32 INT_FLAG;                   // 0x004
	volatile UINT32 SRAM_PW_CFG;				// 0x008
	volatile UINT32 HW_VERSION;                 // 0x00C
	volatile UINT32 LLC_CFG_0;					// 0x010
	volatile UINT32 LLC_CFG_1;					// 0x014
	volatile UINT32 LLC_CFG_2;					// 0x018
	volatile UINT32 RES_1C; 					// 0x01C
	#endif
	volatile VTRCRegSet REG_SET;			// 0x2000 ~  0x20C8	
} VTRC_HW_REG;

#endif
