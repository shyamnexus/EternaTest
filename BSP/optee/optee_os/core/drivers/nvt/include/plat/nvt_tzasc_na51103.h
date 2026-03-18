#ifndef __DRIVERS_NVT_TZASC_H
#define __DRIVERS_NVT_TZASC_H

#include <stdint.h>
#include <trace_levels.h>
#include <types_ext.h>
#include <util.h>
#include <plat/io_address.h>
#include <kwrap/nvt_type.h>

#ifndef ENUM_DUMMY4WORD
#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000
#endif

/**
    Driver callback function

    uiEvent is bitwise event, please refer to each module's document.

*/
typedef void (*DRV_CB)(uint32_t uiEvent);
/*
    DMA channel mask

    Indicate which DMA channels are required to protect/detect

    @note For DMA_WRITEPROT_ATTR
*/

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
	DMA_WP,      		// Not only detect write action but also denial access.
	DMA_WD,       		// Only detect write action but allow write access.
	DMA_RP,       		// Not only detect read action but also denial access.
	DMA_RD,        		// Only detect read action but allow read access.
	ENUM_DUMMY4WORD(DMA_WRITEPROT_LEVEL)
} DMA_WRITEPROT_LEVEL;

typedef struct _DMA_WRITEPROT_ATTR {
	DMA_CH_MSK          mask;       	// DMA channel masks to be protected/detected
	DMA_WRITEPROT_LEVEL level;	    	// protect level
	UINT64              starting_addr; 	// DDR3:must be 4 words alignment
	UINT32              size;          	// DDR3:must be 4 words alignment
} DMA_WRITEPROT_ATTR, *PDMA_WRITEPROT_ATTR;


void nvt_tzasc_init(void);
void tzc_enable_filters(DDR_ARB id, DMA_WRITEPROT_SET set, DMA_WRITEPROT_ATTR *p_attr);
void tzc_disable_filters(DDR_ARB id, DMA_WRITEPROT_SET set);
void nvt_wp_configure_region_enable(DMA_WRITEPROT_SET filter_set,
					vaddr_t  	region_base,
					uint32_t 	region_size,
					DMA_WRITEPROT_LEVEL sec_protected_level,
					uint32_t allow_ns_device_access);
void nvt_tzasc_config(void);

#endif
