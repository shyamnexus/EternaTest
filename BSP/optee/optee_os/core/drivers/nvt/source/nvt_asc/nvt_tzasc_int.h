#ifndef __DRIVERS_NVT_TZASC_INT_H
#define __DRIVERS_NVT_TZASC_INT_H

#include <stdint.h>
#include <trace_levels.h>
#include <types_ext.h>
#include <util.h>
#include <plat/io_address.h>
#include <kwrap/nvt_type.h>

#ifndef ENUM_DUMMY4WORD
#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000
#endif
#if defined(_BSP_NA51102_) || defined(_BSP_NS02201_) || defined(_BSP_NS02301_)  || defined(_BSP_NA51089_) || defined(_BSP_NA51055_)
typedef enum _DMA_CH_GROUP {
	DMA_CH_GROUP0 = 0x0,    // represent channel 00-31
	DMA_CH_GROUP1,          // represent channel 32-63
	DMA_CH_GROUP2,          // represent channel 64-95
	DMA_CH_GROUP3,          // represent channel 96-127
	DMA_CH_GROUP4,          // represent channel 128-159
	DMA_CH_GROUP5,          // represent channel 160-191
	DMA_CH_GROUP6,          // represent channel 192-223
	DMA_CH_GROUP7,          // represent channel 224-255

	DMA_CH_GROUP_CNT,
	ENUM_DUMMY4WORD(DMA_CH_GROUP)
} DMA_CH_GROUP;
#elif (defined(_BSP_NS02401_))
typedef enum _DMA_CH_GROUP {
	DMA_CH_GROUP0 = 0x0,    // cpu0
	DMA_CH_GROUP1,          // cpu1
	DMA_CH_GROUP2,          // gpu
	DMA_CH_GROUP3,          // enc
	DMA_CH_GROUP4,          // vsp_0
	DMA_CH_GROUP5,          // vdec_0
	DMA_CH_GROUP6,			// vdec_1
	DMA_CH_GROUP7,			// vdec_2
	DMA_CH_GROUP8,			// io_3
	DMA_CH_GROUP9,			// io_4
	DMA_CH_GROUP10,			// sys
	DMA_CH_GROUP11,			// rt
	DMA_CH_GROUP12,			// cv

	DMA_CH_GROUP_CNT,
	ENUM_DUMMY4WORD(DMA_CH_GROUP)
} DMA_CH_GROUP;
#else
typedef enum _DMA_CH_GROUP {
	DMA_CH_GROUP0 = 0x0,
	DMA_CH_GROUP1,

	DMA_CH_GROUP_CNT,
	ENUM_DUMMY4WORD(DMA_CH_GROUP)
} DMA_CH_GROUP;
#endif
#endif
