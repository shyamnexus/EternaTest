/**
    Public header file for arbiter module.

    This file is the header file that define the API and data type for arbiter
    module.

    @file       ddr_arb.h
    @ingroup    miDrvComm_Arb
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _DRAM_ARB_H
#define _DRAM_ARB_H

#if defined __UITRON || defined __ECOS
#include "Driver.h"
#else
#include "comm/driver.h"
#endif


/**
    @addtogroup miDrvComm_Arb
*/
//@{

/*
    DMA channel mask

    Indicate which DMA channels are required to protect/detect

    @note For DMA_WRITEPROT_ATTR
*/
typedef struct _DMA_CH_MSK {
	// ch 0
	UINT32 CPU_S: 1;   //CPU_S
	UINT32 CPU_NS: 1;
	UINT32 HVYLD: 1;
	//M1
	UINT32 PBUS: 1;
	UINT32 SDIO: 1;
	UINT32 SDIO2: 1;
	UINT32 SDIO3: 1;
	UINT32 SMC: 1;
	UINT32 ETHER: 1;
	UINT32 LARB: 1;
	UINT32 DAI: 1;
	UINT32 TSE: 1;
	UINT32 UVCP: 1;
	UINT32 HWCP: 1;
	UINT32 SCE: 1;
	UINT32 HASH: 1;
	UINT32 HVYLD2: 1;
	//M2
	UINT32 VENC: 1;
	UINT32 HVYLD3: 1;
	//M3
	UINT32 IFE: 1;
	UINT32 IPE: 1;
	UINT32 IME: 1;
	UINT32 ISE: 1;
	UINT32 DRE: 1;
	UINT32 SIE_1: 1;
	UINT32 SIE2_1: 1;
	UINT32 SIE3_1: 1;
	UINT32 SIE4_1: 1;
	UINT32 SIE5_1: 1;
	//M4
	UINT32 VPE: 1;
	//M5
	UINT32 IDE: 1;
	UINT32 CSITX: 1;
	UINT32 USB3: 1;
	UINT32 SIE_0: 1;
	UINT32 SIE2_0: 1;
	UINT32 SIE3_0: 1;
	UINT32 SIE4_0: 1;
	UINT32 SIE5_0: 1;
	UINT32 VIE: 1;
	//M6
	UINT32 IVE: 1;
	UINT32 MDBC: 1;
	UINT32 TRKE: 1;
	UINT32 NUE2: 1;
	UINT32 GRP: 1;
	UINT32 GRP2: 1;
	UINT32 GRP3: 1;
	UINT32 JPEG: 1;
	//M7
	UINT32 CONV: 1;
	UINT32 JM: 1;
	UINT32 JMISP: 1;
	UINT32 NUE: 1;
	UINT32 LSU: 1;
	UINT32 PPU: 1;
	//new add
	UINT32 SPI3: 1;
	UINT32 LARB2: 1;
	UINT32 SPI2: 1;
	UINT32 PRE: 1;
	UINT32 POU: 1;
	//delete
	UINT32 ETH: 1;
	UINT32 ETH2: 1;
	UINT32 DCE: 1;
	UINT32 SIE6_0: 1;
	UINT32 SIE6_1: 1;
	UINT32 VENC_0: 1;
	UINT32 VENC_1: 1;
	UINT32 VPE_0: 1;
	UINT32 VPE_1: 1;
	UINT32 SDE: 1;

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
	DMA_WPLEL_UNWRITE,      // Not only detect write action but also denial access.
	DMA_WPLEL_DETECT,       // Only detect write action but allow write access.
	DMA_RPLEL_UNREAD,       // Not only detect read action but also denial access.
	DMA_RWPLEL_UNRW,        // Not only detect read write action but also denial access.
	ENUM_DUMMY4WORD(DMA_WRITEPROT_LEVEL)
} DMA_WRITEPROT_LEVEL;

typedef enum _DMA_PROT_MODE {
	DMA_PROT_IN,
	DMA_PROT_OUT,
	ENUM_DUMMY4WORD(DMA_PROT_MODE)
} DMA_PROT_MODE;

typedef struct _DMA_PROT_RGN_ATTR {
	BOOL                en;            // enable this region
	ULONG               starting_addr; // DDR3:must be 4 words alignment
	UINT32              size;          // DDR3:must be 4 words alignment
} DMA_PROT_RGN_ATTR, *PDMA_PROT_RGN_ATTR;

typedef struct _DMA_WRITEPROT_ATTR {
	DMA_CH_MSK          mask;       // DMA channel masks to be protected/detected
	DMA_WRITEPROT_LEVEL level;	    // protect level
	DMA_PROT_MODE       protect_mode; // in or out region
	DMA_PROT_RGN_ATTR   protect_rgn_attr;
	UINT32              starting_addr; // DDR3:must be 4 words alignment
	UINT32              size;          // DDR3:must be 4 words alignment
} DMA_WRITEPROT_ATTR, *PDMA_WRITEPROT_ATTR;

/*
    @name DMA outstanding

    DMA outstanding setting

    @note Used in dma_set_channel_outstanding()
*/
typedef enum {
	DMA_CH_OUTSTANDING_CNN_0,           // CNN input
	DMA_CH_OUTSTANDING_CNN_1,           // CNN input
	DMA_CH_OUTSTANDING_CNN_2,           // CNN input
	DMA_CH_OUTSTANDING_CNN_3,           // CNN input
	DMA_CH_OUTSTANDING_CNN_4,           // CNN output
	DMA_CH_OUTSTANDING_CNN_5,           // CNN output

	DMA_CH_OUTSTANDING_CNN2_0,
	DMA_CH_OUTSTANDING_CNN2_1,
	DMA_CH_OUTSTANDING_CNN2_2,
	DMA_CH_OUTSTANDING_CNN2_3,
	DMA_CH_OUTSTANDING_CNN2_4,
	DMA_CH_OUTSTANDING_CNN2_5,

	DMA_CH_OUTSTANDING_NUE_0,           // NUE input
	DMA_CH_OUTSTANDING_NUE_1,           // NUE input
	DMA_CH_OUTSTANDING_NUE_2,           // NUE output

	DMA_CH_OUTSTANDING_CNN_ALL,
	DMA_CH_OUTSTANDING_CNN2_ALL,
	DMA_CH_OUTSTANDING_NUE_ALL,
	ENUM_DUMMY4WORD(DMA_CH_OUTSTANDING)
} DMA_CH_OUTSTANDING;


typedef enum _DRAM_CONSUME_BW_DEGREE {
	DRAM_CONSUME_EASY_LOADING = 0x0,
	DRAM_CONSUME_NORMAL_LOADING,
	DRAM_CONSUME_HEAVY_LOADING,

	DRAM_CONSUME_CH_DISABLE,

	ENUM_DUMMY4WORD(DRAM_CONSUME_BW_DEGREE)
} DRAM_CONSUME_BW_DEGREE;


typedef struct _DRAM_CONSUME_ATTR {
	DRAM_CONSUME_BW_DEGREE  load_degree;
	ULONG                   addr;
	UINT32                  size;
	BOOL					is_start;
} DRAM_CONSUME_ATTR, *PDRAM_CONSUME_ATTR;

// Arbiter Driver API
// check protect set status by arb_get_wp
extern void arb_enable_wp(DDR_ARB id, DMA_WRITEPROT_SET set,
                DMA_WRITEPROT_ATTR *p_attr);
extern void arb_disable_wp(DDR_ARB id, DMA_WRITEPROT_SET set);

//return value
//0: not in use
//1: in use
//-1: err
extern int arb_get_wp(DDR_ARB id, DMA_WRITEPROT_SET set);

extern ER arb_init(void);
extern void arb_set_priority(BOOL is_direct);
extern ER dma_get_channel_outstanding(DMA_CH_OUTSTANDING channel, BOOL *enable);
extern ER dma_set_channel_outstanding(DMA_CH_OUTSTANDING channel, BOOL enable);

extern int dram_consume_cfg(PDRAM_CONSUME_ATTR attr);
extern int dram_consume_start(void);
extern int dram_consume_stop(void);







/* @this function is used to perform hardware checksum
 * @parm: id:
 *      memory address on which ddr
 * @parm: phy_addr:
 *		memory physical address (must be word alignment)
 * @parm: len:
 * 		memory length (must be word alignment)
 * @return:
 *		checksum value
 *
 */
extern unsigned short arb_chksum(DDR_ARB id, uintptr_t phy_addr, unsigned long len);
//@}


//ch: 0: CPU, 1: CNN, 2: CNN2, 3: NUE, 4: NUE2, 5: ISE, 6:CNN+CNN2+NUE+NUE2+DSP
//rw: 0: write, 1: read, 2: both
//dram: 0: DRAM1, 1: DRAM2
extern int mau_ch_mon_start(int ch, int rw, int dram);
extern UINT64 mau_ch_mon_stop(int ch, int dram);

extern void dma_reset_data_count(DDR_ARB id);
extern UINT64 dma_get_data_count(DDR_ARB id);

#endif
