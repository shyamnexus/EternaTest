/*
    Internal header file for arbiter module.

    This file is the internal header file that defines the variables, structures
    and function prototypes of arbiter module.

    @file       ddr_arb_int.h
    @ingroup    miDrvComm_Arb
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _DDR_ARB_INT_H
#define _DDR_ARB_INT_H

#if defined(__UITRON) || defined(__ECOS)
#include "ddr_arb.h"
#elif defined(__FREERTOS)
#include "comm/ddr_arb.h"
#include "plat/dma.h"
#include <kwrap/nvt_type.h>
#else
#include <linux/slab.h>
#include <linux/soc/nvt/rcw_macro.h>
#include "kwrap/type.h"
#include "comm/ddr_arb.h"
#include "comm/timer.h"


#define islower(c)    ((c) >= 'a')
#define toupper(c)    (islower((c)) ? ((c) + 'A' - 'a') : (c))


/**
    DMA controller ID

*/
typedef enum {
	DMA_ID_1,                           ///< DMA Controller

	DMA_ID_COUNT,                       //< DMA controller count

	ENUM_DUMMY4WORD(DMA_ID)
} DMA_ID;
#endif

/*    QOS ID    This is for nvt_set_qos. level = 0(low)~15(high)*/
typedef enum {
	//Top1
	CPU_QOS   =  0,
	PBUS_QOS,
	HVYLD_QOS,
	HVYLD2_QOS,
	HVYLD3_QOS,
	LARB_QOS,
	LARB2_QOS,
	ETH_QOS,
	IDE_QOS,
	DAI_QOS,
	SCE_QOS,
	HASH_QOS,
	UVCP_QOS,
	TSE_QOS,
	HWCP_QOS,
	GRP_QOS,
	GRP2_QOS,
	GRP3_QOS,
	JPG_QOS,
	IVE_QOS,
	MDBC_QOS,
	TRKE_QOS,
	NUE2_QOS,
	SPI3_QOS,
	SPI2_QOS,
	//Top2
	USB3_QOS = 32,
	CSITX_QOS,
	//Top3
	SIE_0_QOS = 64,
	SIE2_0_QOS,
	SIE3_0_QOS,
	SIE4_0_QOS,
	SIE5_0_QOS,
	SIE_1_QOS,
	SIE2_1_QOS,
	SIE3_1_QOS,
	SIE4_1_QOS,
	SIE5_1_QOS,
	IFE_QOS,
	ISE_QOS,
	DRE_QOS,
	SDIO3_QOS,
	SMC_QOS,
	VIE_QOS,
	PRE_QOS,
	//Top4
	IPE_QOS = 96,
	IME_QOS,
	SDIO_QOS,
	SDIO2_QOS,
	//Top5
	VPE_0_QOS = 128,
	VPE_1_QOS,
	//Top6
	VENC_0_QOS = 160,
	VENC_1_QOS,
	//Top7
	JM_QOS = 192,
	JMISP_QOS,
	NUE_QOS,
	LSU_QOS,
	PPU_QOS,
	PPU_POU,
	//Top8
	CONV_0_QOS = 224,
	CONV_1_QOS,

	//should be remove
	SIE6_0_QOS,
	SIE6_1_QOS,
	DCE_QOS,
	SDE_QOS,

	ENUM_DUMMY4WORD(QOS_ID)
} QOS_ID;

/*    AXI channel disable*/
typedef enum {
	//M0
	M0_CPU,
	M0_CPU_NS,
	M0_HVYLD,
	//M1
	M1_PBUS,
	M1_SDIO,
	M1_SDIO2,
	M1_SDIO3,
	M1_SMC,
	M1_ETHER,
	M1_LARB,
	M1_DAI,
	M1_TSE,
	M1_UVCP,
	M1_HWCP,
	M1_SCE,
	M1_HASH,
	M1_HVYLD2,
	//M2
	M2_VENC,
	M2_HVYLD3,
	//M3
	M3_IFE,
	M3_IPE,
	M3_IME,
	M3_ISE,
	M3_DRE,
	M3_SIE_1,
	M3_SIE2_1,
	M3_SIE3_1,
	M3_SIE4_1,
	M3_SIE5_1,
	//M4
	M4_VPE,
	//M5
	M5_IDE,
	M5_CSITX,
	M5_USB3,
	M5_SIE_0,
	M5_SIE2_0,
	M5_SIE3_0,
	M5_SIE4_0,
	M5_SIE5_0,
	M5_VIE,
	//M6
	M6_IVE,
	M6_MDBC,
	M6_TRKE,
	M6_NUE2,
	M6_GRP,
	M6_GRP2,
	M6_GRP3,
	M6_JPEG,
	//M7
	M7_CONV,
	M7_JM,
	M7_JMISP,
	M7_NUE,
	M7_LSU,
	M7_PPU,
	//new add
	M1_SPI3,
	M1_LARB2,
	M1_SPI2,
	M3_PRE,
	M7_POU,


	ENUM_DUMMY4WORD(DMA_CH)
} DMA_CH, *PDMA_CH;

/*    AXI channel disable*/
typedef enum {
	//Top1
	CPU_DISABLE   =  0,
	PBUS_DISABLE,
	HVYLD_DISABLE,
	HVYLD2_DISABLE,
	HVYLD3_DISABLE,
	LARB_DISABLE,
	LARB2_DISABLE,
	ETH_DISABLE,
	IDE_DISABLE,
	DAI_DISABLE,
	SCE_DISABLE,
	HASH_DISABLE,
	UVCP_DISABLE,
	TSE_DISABLE,
	HWCP_DISABLE,
	GRP_DISABLE,
	GRP2_DISABLE,
	GRP3_DISABLE,
	JPG_DISABLE,
	IVE_DISABLE,
	MDBC_DISABLE,
	TRKE_DISABLE,
	NUE2_DISABLE,
	SPI3_DISABLE,
	SPI2_DISABLE,

	//Top2
	USB3_DISABLE = 32,
	CSITX_DISABLE,
	//Top3
	SIE_0_DISABLE = 64,
	SIE2_0_DISABLE,
	SIE3_0_DISABLE,
	SIE4_0_DISABLE,
	SIE5_0_DISABLE,
	SIE_1_DISABLE,
	SIE2_1_DISABLE,
	SIE3_1_DISABLE,
	SIE4_1_DISABLE,
	SIE5_1_DISABLE,
	IFE_DISABLE,
	ISE_DISABLE,
	DRE_DISABLE,
	SDIO3_DISABLE,
	SMC_DISABLE,
	VIE_DISABLE,
	PRE_DISABLE,
	//Top4
	IPE_DISABLE = 96,
	IME_DISABLE,
	SDIO_DISABLE,
	SDIO2_DISABLE,
	//Top5
	VPE_0_DISABLE = 128,
	VPE_1_DISABLE,
	//Top6
	VENC_0_DISABLE = 160,
	VENC_1_DISABLE,
	//Top7
	JM_DISABLE = 192,
	JMISP_DISABLE,
	NUE_DISABLE,
	LSU_DISABLE,
	PPU_DISABLE,
	POU_DISABLE,
	//Top8
	CONV_0_DISABLE = 224,
	CONV_1_DISABLE,

	//should be remove
	SIE6_0_DISABLE,
	SIE6_1_DISABLE,
	DCE_DISABLE,
	SDE_DISABLE,
	ENUM_DUMMY4WORD(AXI_CH)
} AXI_CH;

/*    QOS LEVEL    This is for nvt_set_qos. level = 0(low)~15(high)*/
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
	QOS_LEVEL_15,	//

	ENUM_DUMMY4WORD(QOS_LEVEL)
} QOS_LEVEL;

typedef enum _DMA_PRI {
	DMA_URGENT,
	DMA_HIGH,
	DMA_MID,
	DMA_LOW,

	ENUM_DUMMY4WORD(DMA_PRI)
} DMA_PRI;

typedef enum _DMA_MON_CH {
	DMA_MON_0,   								// Monitor set 0
	DMA_MON_1,    								// Monitor set 1
	DMA_MON_2,                   				// Monitor set 2
	DMA_MON_3,                 					// Monitor set 3
	ENUM_DUMMY4WORD(DMA_MON_CH)
} DMA_MON_CH;

typedef enum _DMA_MON_MODE {
	DMA_MON_W,   								// Monitor write
	DMA_MON_R,    								// Monitor read
	DMA_MON_RW,                   				// Monitor read/write
	ENUM_DUMMY4WORD(DMA_MON_MODE)
} DMA_MON_MODE;

typedef struct _DMA_MON_ATTR {
	DMA_CH_MSK        mask;
	DMA_MON_MODE				mode;  // Monitor mode
} DMA_MON_ATTR, *PDMA_MON_ATTR;

typedef enum _DMA_DIRECTION {
	DMA_DIRECTION_READ,     // DMA read (DRAM -> Module)
	DMA_DIRECTION_WRITE,    // DMA write (DRAM <- Module)
	DMA_DIRECTION_BOTH,     // DMA read and write

	ENUM_DUMMY4WORD(DMA_DIRECTION)
} DMA_DIRECTION;

typedef enum _DMA_CH_GROUP {
	DMA_CH_GROUP0 = 0x0,    // represent channel 00-31
	DMA_CH_GROUP1,          // represent channel 32-63
	DMA_CH_GROUP2,          // should be delete


	DMA_CH_GROUP_CNT,
	ENUM_DUMMY4WORD(DMA_CH_GROUP)
} DMA_CH_GROUP;

typedef enum _DMA_HEAVY_LOAD_CH {
	DMA_HEAVY_LOAD_CH0 = 0x0,                   // Heavy load channel 0
	DMA_HEAVY_LOAD_CH1,                         // Heavy load channel 1
	DMA_HEAVY_LOAD_CH2,                         // Heavy load channel 2
	DMA_HEAVY_LOAD_CH_CNT,
	ENUM_DUMMY4WORD(DMA_HEAVY_LOAD_CH)
} DMA_HEAVY_LOAD_CH;

typedef enum _DMA_HEAVY_LOAD_TEST_METHOD {
	DMA_HEAVY_LOAD_RW_BURST_INTERLEAVE = 0x0,   // Heavy load test method 0 => RW burst interleave
	DMA_HEAVY_LOAD_READ_AFTER_WRITE_ALL_BUF,    // Heavy load test method 1 => Read after write all buffer
	DMA_HEAVY_LOAD_READ_ONLY,                   // Heavy load test method 2 => Read only
	DMA_HEAVY_LOAD_WRITE_ONLY,                  // Heavy load test method 3 => Write only
	ENUM_DUMMY4WORD(DMA_HEAVY_LOAD_TEST_METHOD)
} DMA_HEAVY_LOAD_TEST_METHOD;

typedef struct _DMA_HEAVY_LOAD_PARAM {
	UINT32                      test_times;    // Heavy load test times, 0 is not available
	DMA_HEAVY_LOAD_TEST_METHOD  test_method;   // Heavy load test method, 0 is not available
	UINT32                      burst_len;     // Heavy load pattern burst length(0~127)
	ULONG                       start_addr;    // Heavy load start address(word alignment)
	UINT32                      dma_size;      // Heavy load test dma size(word alignment)
	BOOL                        outstanding_en;
	UINT32                      outstanding;   // 0, 1, 2, 3
	BOOL	                    random_pattern;
	BOOL                        is_phy_addr;   // indicate start_addr is physical addr or not
} DMA_HEAVY_LOAD_PARAM, *PDMA_HEAVY_LOAD_PARAM;


UINT32 dma_get_utilization(DDR_ARB id);
UINT32 dma_get_efficiency(DDR_ARB id);

ER dma_set_system_priority(BOOL is_direct);


#define ARB_SETREG(ofs,value)	OUTW(IOADDR_DDR_ARB_REG_BASE+(ofs),(value))
#define ARB_GETREG(ofs)		INW(IOADDR_DDR_ARB_REG_BASE+(ofs))

#define PROT_SETREG(ofs,value)	OUTW(IOADDR_PROT_REG_BASE+(ofs),(value))
#define PROT_GETREG(ofs)		INW(IOADDR_PROT_REG_BASE+(ofs))
/*#define PROT_SETREG(ofs,value)	OUTW(IOADDR_DDR_ARB_REG_BASE+0x4300+(ofs),(value))
#define PROT_GETREG(ofs)		INW(IOADDR_DDR_ARB_REG_BASE+0x4300+(ofs))

#define MON_SETREG(ofs,value)	OUTW(IOADDR_DDR_ARB_REG_BASE+0x4100+(ofs),(value))
#define MON_GETREG(ofs)			INW(IOADDR_DDR_ARB_REG_BASE+0x4100+(ofs))*/


#define HVY_SETREG(ofs,value)	OUTW(IOADDR_DDR_HVY_REG_BASE+(ofs),(value))
#define HVY_GETREG(ofs)		INW(IOADDR_DDR_HVY_REG_BASE+(ofs))
#define HVY2_SETREG(ofs,value)	OUTW(IOADDR_DDR_HVY2_REG_BASE+(ofs),(value))
#define HVY2_GETREG(ofs)		INW(IOADDR_DDR_HVY2_REG_BASE+(ofs))
#define HVY3_SETREG(ofs,value)	OUTW(IOADDR_DDR_HVY3_REG_BASE+(ofs),(value))
#define HVY3_GETREG(ofs)		INW(IOADDR_DDR_HVY3_REG_BASE+(ofs))

#define NVT_QOS_SETREG(ofs,value) OUTW(IOADDR_DDR_QOS_REG_BASE+(ofs),(value))
#define NVT_QOS_GETREG(ofs)		INW(IOADDR_DDR_QOS_REG_BASE+(ofs))
#define NVT_LPV_SETREG(ofs,base,value) OUTW((IOADDR_LPV_REG_BASE)+(ofs),(value))
#define NVT_LPV_GETREG(ofs,base)		INW((IOADDR_LPV_REG_BASE)+(ofs))




typedef void (*dma_hvyload_callback_func)(void);

// Register setting
UINT32 dma_get_monitor_period(DDR_ARB id);
void dma_set_monitor_period(void);

ER dma_enable_heavyload(DDR_ARB id, DMA_HEAVY_LOAD_CH channel, PDMA_HEAVY_LOAD_PARAM hvy_param);
BOOL dma_wait_heavyload_done_polling(int ch, dma_hvyload_callback_func call_back_hdl);
void dma_trig_heavyload(DDR_ARB id, UINT32 channel);
void dma_stop_heavyload(DDR_ARB id, UINT32 channel);
//int dma_enable_monitor(DMA_ID id, int axi_channel, UINT32 mask, DMA_DIRECTION direction);// axi_channel: 0~7
extern void dma_enable_monitor(DDR_ARB id, DMA_MON_CH ch, DMA_MON_ATTR attr);
//UINT64 dma_disable_monitor(DMA_ID id, int axi_channel);
extern void dma_disable_monitor(DDR_ARB id, int axi_channel);
void dma_reset_monitor_cnt(DDR_ARB id, DMA_MON_CH ch);
UINT64 dma_get_monitor_count(DDR_ARB id, DMA_MON_CH ch);
UINT32 dma_get_monitor_ch(DMA_ID id, int axi_channel);
void axi_channel_en_dis(AXI_CH ch, BOOL en, BOOL rw);

UINT64 dma_get_ddr_bandwidth(DDR_ARB id, void *param);

UINT32 dma_get_heavyload_chksum(DMA_ID id, DMA_HEAVY_LOAD_CH ch, UINT32 burst, UINT32 addr, UINT32 length);
void dma_set_heavyload_pattern(DMA_HEAVY_LOAD_CH ch, UINT32 pattern0, UINT32 pattern1, UINT32 pattern2, UINT32 pattern3);

extern void nvt_set_qos(QOS_ID id, QOS_LEVEL r_level, QOS_LEVEL w_level);

extern void dma_set_priority(DDR_ARB id, DMA_CH ch, DMA_PRI w_pri, DMA_PRI r_pri);
extern void dma_enable_module_eff(DDR_ARB id, DMA_MON_ATTR attr);
extern void dma_disable_module_eff(DDR_ARB id);

void dma_disable_monitor_eff(DMA_ID id, int axi_channel);
int dma_enable_monitor_eff(DMA_ID id, int axi_channel, UINT32 mask, DMA_DIRECTION direction);
#endif
