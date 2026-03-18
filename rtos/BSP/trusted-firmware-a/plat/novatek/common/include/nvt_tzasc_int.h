/*
 * Copyright (c) 2021, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DRIVERS_NVT_TZASC_INT_H
#define __DRIVERS_NVT_TZASC_INT_H

#include <stdint.h>

#ifndef ENUM_DUMMY4WORD
#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000
#endif

#ifndef ENABLE
#define ENABLE              1           ///< Feature is enabled
#endif

/*
    DMA channel mask

    Indicate which DMA channels are required to protect/detect

    @note For DMA_WRITEPROT_ATTR
*/
#if (defined(_BSP_NA51102_))
typedef struct {
	// ch 0
	uint32_t bRsv0:32;
	// ch 1
	uint32_t CPU_S: 1;
	uint32_t CPU_NS: 1;
	uint32_t bRsv1: 30;
	// ch 2
	uint32_t bRsv2:32;
	// ch 3
	uint32_t bRsv3:32;
	// ch 4
	uint32_t bRsv4:32;
	// ch 5
	uint32_t bRsv5:32;
	// ch 6
	uint32_t bRsv6:32;
	// ch 7
	uint32_t bRsv7:32;
} DMA_CH_MSK, *PDMA_CH_MSK;

/**
    DDR Arbiter ID

*/
typedef enum _DDR_ARB {
	DDR_ARB_1,                           ///< DDR Arbiter

	DDR_ARB_COUNT,                       //< Arbiter count

	ENUM_DUMMY4WORD(DDR_ARB)
} DDR_ARB;

typedef enum _DMA_WRITEPROT_SET {
	WPSET_0,            				// Write protect function set 0
	WPSET_1,            				// Write protect function set 1
	WPSET_2,            				// Write protect function set 2
	WPSET_3,            				// Write protect function set 3
	WPSET_4,            				// Write protect function set 4
	WPSET_5,            				// Write protect function set 5
	WPSET_COUNT,
	ENUM_DUMMY4WORD(DMA_WRITEPROT_SET)
} DMA_WRITEPROT_SET;

typedef enum _DMA_WRITEPROT_LEVEL {
	DMA_WPLEL_UNWRITE,      			// Not only detect write action but also denial access.
	DMA_WPLEL_DETECT,       			// Only detect write action but allow write access.
	DMA_RPLEL_UNREAD,       			// Not only detect read action but also denial access.
	DMA_RWPLEL_UNRW,        			// Not only detect read write action but also denial access.
	ENUM_DUMMY4WORD(DMA_WRITEPROT_LEVEL)
} DMA_WRITEPROT_LEVEL;

#define DMA_WP							DMA_WPLEL_UNWRITE
#define DMA_RP							DMA_RPLEL_UNREAD

typedef enum _DMA_PROT_MODE {
	DMA_PROT_IN,
	DMA_PROT_OUT,
	ENUM_DUMMY4WORD(DMA_PROT_MODE)
} DMA_PROT_MODE;

typedef struct _DMA_PROT_RGN_ATTR {
	bool            	en;            	// enable this region
	uint64_t            starting_addr; 	// DDR3:must be 4 words alignment
	unsigned int        size;          	// DDR3:must be 4 words alignment
} DMA_PROT_RGN_ATTR, *PDMA_PROT_RGN_ATTR;

typedef struct _DMA_WRITEPROT_ATTR {
	DMA_CH_MSK          mask;       	// DMA channel masks to be protected/detected
	DMA_WRITEPROT_LEVEL level;	    	// protect level
	DMA_PROT_MODE       protect_mode; 	// in or out region
	DMA_PROT_RGN_ATTR   protect_rgn_attr;
} DMA_WRITEPROT_ATTR, *PDMA_WRITEPROT_ATTR;

#else
#if defined(_BSP_NS02201_)
typedef struct {
	// ch 0
	uint32_t CPU_S: 1;
	uint32_t CPU_NS: 1;
	uint32_t bRsv0: 30;
	// ch 1
	uint32_t bRsv1:32;
	// ch 2
	uint32_t bRsv2:32;
	// ch 3
	uint32_t bRsv3:32;
	// ch 4
	uint32_t bRsv4:32;
	// ch 5
	uint32_t bRsv5:32;
	// ch 6
	uint32_t bRsv6:32;
	// ch 7
	uint32_t bRsv7:32;
} DMA_CH_MSK, *PDMA_CH_MSK;

/**
    DDR Arbiter ID

*/
typedef enum _DDR_ARB {
	DDR_ARB_1,                           ///< DDR Arbiter
	DDR_ARB_2,                           ///< DDR Arbiter

	DDR_ARB_COUNT,                       //< Arbiter count

	ENUM_DUMMY4WORD(DDR_ARB)
} DDR_ARB;

typedef enum _DMA_WRITEPROT_SET {
	WPSET_0,            				// Write protect function set 0
	WPSET_1,            				// Write protect function set 1
	WPSET_2,            				// Write protect function set 2
	WPSET_3,            				// Write protect function set 3
	WPSET_4,            				// Write protect function set 4
	WPSET_5,            				// Write protect function set 5
	WPSET_COUNT,
	ENUM_DUMMY4WORD(DMA_WRITEPROT_SET)
} DMA_WRITEPROT_SET;

typedef enum _DMA_WRITEPROT_LEVEL {
	DMA_WPLEL_UNWRITE,      			// Not only detect write action but also denial access.
	DMA_WPLEL_DETECT,       			// Only detect write action but allow write access.
	DMA_RPLEL_UNREAD,       			// Not only detect read action but also denial access.
	DMA_RWPLEL_UNRW,        			// Not only detect read write action but also denial access.
	ENUM_DUMMY4WORD(DMA_WRITEPROT_LEVEL)
} DMA_WRITEPROT_LEVEL;

#define DMA_WP							DMA_WPLEL_UNWRITE
#define DMA_RP							DMA_RPLEL_UNREAD

typedef enum _DMA_PROT_MODE {
	DMA_PROT_IN,
	DMA_PROT_OUT,
	ENUM_DUMMY4WORD(DMA_PROT_MODE)
} DMA_PROT_MODE;

typedef struct _DMA_PROT_RGN_ATTR {
	bool            	en;            	// enable this region
	uint64_t            starting_addr; 	// DDR3:must be 4 words alignment
	unsigned int        size;          	// DDR3:must be 4 words alignment
} DMA_PROT_RGN_ATTR, *PDMA_PROT_RGN_ATTR;

typedef struct _DMA_WRITEPROT_ATTR {
	DMA_CH_MSK          mask;       	// DMA channel masks to be protected/detected
	DMA_WRITEPROT_LEVEL level;	    	// protect level
	DMA_PROT_MODE       protect_mode; 	// in or out region
	DMA_PROT_RGN_ATTR   protect_rgn_attr;
} DMA_WRITEPROT_ATTR, *PDMA_WRITEPROT_ATTR;
#elif defined(_BSP_NS02401_)
typedef struct {
	// ch 0
	uint32_t CPU_S: 1;
	uint32_t bRsv0: 31;
	// ch 1
	uint32_t bRsv1: 6;
	uint32_t CPU_NS: 1;
	uint32_t bRsv1_2: 25;
	// ch 2
	uint32_t bRsv2:32;
	// ch 3
	uint32_t bRsv3:32;
	// ch 4
	uint32_t bRsv4:32;
	// ch 5
	uint32_t bRsv5:32;
	// ch 6
	uint32_t bRsv6:32;
	// ch 7
	uint32_t bRsv7:32;
	uint32_t bRsv8:32;
	uint32_t bRsv9:32;
	uint32_t bRsv10:32;
	uint32_t bRsv11:32;
	uint32_t bRsv12:32;
} DMA_CH_MSK, *PDMA_CH_MSK;

/**
    DDR Arbiter ID

*/
typedef enum _DDR_ARB {
	DDR_ARB_1,                           ///< DDR Arbiter
	DDR_ARB_2,                           ///< DDR Arbiter

	DDR_ARB_COUNT,                       //< Arbiter count

	ENUM_DUMMY4WORD(DDR_ARB)
} DDR_ARB;

typedef enum _DMA_WRITEPROT_SET {
	WPSET_0,            				// Write protect function set 0
	WPSET_1,            				// Write protect function set 1
	WPSET_2,            				// Write protect function set 2
	WPSET_3,            				// Write protect function set 3
	WPSET_4,            				// Write protect function set 4
	WPSET_5,            				// Write protect function set 5
	WPSET_6,            				// Write protect function set 6
	WPSET_7,            				// Write protect function set 7
	WPSET_COUNT,
	ENUM_DUMMY4WORD(DMA_WRITEPROT_SET)
} DMA_WRITEPROT_SET;

typedef enum _DMA_WRITEPROT_LEVEL {
	DMA_WPLEL_UNWRITE,      			// Not only detect write action but also denial access.
	DMA_WPLEL_DETECT,       			// Only detect write action but allow write access.
	DMA_RPLEL_UNREAD,       			// Not only detect read action but also denial access.
	DMA_RWPLEL_UNRW,        			// Not only detect read write action but also denial access.
	ENUM_DUMMY4WORD(DMA_WRITEPROT_LEVEL)
} DMA_WRITEPROT_LEVEL;

#define DMA_WP							DMA_WPLEL_UNWRITE
#define DMA_RP							DMA_RPLEL_UNREAD

typedef enum _DMA_PROT_MODE {
	DMA_PROT_IN,
	DMA_PROT_OUT,
	ENUM_DUMMY4WORD(DMA_PROT_MODE)
} DMA_PROT_MODE;

typedef struct _DMA_PROT_RGN_ATTR {
	bool            	en;            	// enable this region
	uint64_t            starting_addr; 	// DDR3:must be 4 words alignment
	unsigned int        size;          	// DDR3:must be 4 words alignment
} DMA_PROT_RGN_ATTR, *PDMA_PROT_RGN_ATTR;

typedef struct _DMA_WRITEPROT_ATTR {
	DMA_CH_MSK          mask;       	// DMA channel masks to be protected/detected
	DMA_WRITEPROT_LEVEL level;	    	// protect level
	DMA_PROT_MODE       protect_mode; 	// in or out region
	DMA_PROT_RGN_ATTR   protect_rgn_attr;
} DMA_WRITEPROT_ATTR, *PDMA_WRITEPROT_ATTR;

typedef struct _PROT_GP_TO_LPV_PARAM {
	//UINT32          gp;    	// protect group
	//UINT32			id;		// channel id in gp
	uint32_t			top_id;  // lpv top id
	uint32_t			reg_off;  // lpv top id
	uint32_t          start_bit;     // lpv register bit offset
	uint32_t          end_bit;       // lpv register bit offset
	uint32_t			lpv_id;			// user id
} PROT_GP_TO_LPV_PARAM, *PPROT_GP_TO_LPV_PARAM;
#else

typedef struct {
        // ch 0
        uint32_t bReserved0: 1;                       //< bit0: reserved (auto refresh)
        uint32_t bCPU_NS: 1;                          //< CPU ns access
        uint32_t bRsv0:30;
        // ch 32
        uint32_t bRsv1:32;
} DMA_CH_MSK, *PDMA_CH_MSK;

/**
    DDR Arbiter ID

*/
typedef enum _DDR_ARB {
        DDR_ARB_1,                           ///< DDR Arbiter
        DDR_ARB_2,                           ///< DDR Arbiter

        DDR_ARB_COUNT,                       //< Arbiter count

        ENUM_DUMMY4WORD(DDR_ARB)
} DDR_ARB;


typedef enum _DMA_WRITEPROT_SET {
        WPSET_0,            // Write protect function set 0
        WPSET_1,            // Write protect function set 1
        WPSET_2,            // Write protect function set 2
        WPSET_3,            // Write protect function set 3
        WPSET_4,            // Write protect function set 4
        OUT_WP,             // Out range write protect
        WPSET_COUNT,
        ENUM_DUMMY4WORD(DMA_WRITEPROT_SET)
} DMA_WRITEPROT_SET;

typedef enum _DMA_WRITEPROT_LEVEL {
        DMA_WP,      // Not only detect write action but also denial access.
        DMA_WD,       // Only detect write action but allow write access.
        DMA_RP,       // Not only detect read action but also denial access.
        DMA_RD,        // Only detect read action but allow read access.
        ENUM_DUMMY4WORD(DMA_WRITEPROT_LEVEL)
} DMA_WRITEPROT_LEVEL;

typedef struct _DMA_WRITEPROT_ATTR {
        DMA_CH_MSK          mask;       // DMA channel masks to be protected/detected
        DMA_WRITEPROT_LEVEL level;          // protect level
        uint64_t            starting_addr; // DDR3:must be 4 words alignment
        uint32_t            size;          // DDR3:must be 4 words alignment
} DMA_WRITEPROT_ATTR, *PDMA_WRITEPROT_ATTR;
#endif
#endif

#if (defined(_BSP_NA51102_) || defined(_BSP_NS02201_))
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
