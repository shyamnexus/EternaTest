/*
    @file       spi_int.h

    @ingroup    mIDrvIO_SPI

    @brief      NT96660 SPI internal header file

    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.

*/

#ifndef _SPI_INT_H
#define _SPI_INT_H

//#include "DrvCommon.h"
#include <kwrap/nvt_type.h>
#include <kwrap/semaphore.h>
#include <kwrap/flag.h>
#include <kwrap/spinlock.h>
#include <comm/driver.h>
#include <io_address.h>
#include "top.h"
#include "spi_reg.h"
#include "cache_protected.h"
#include "pll_protected.h"

#define DRV_SUPPORT_IST             (ENABLE)
#define _EMULATION_ON_CPU2_	        (0)
#define GYRO_UPDATE_INTR_VERIFY     (ENABLE) //DISABLE
#define GYRO_SET_START_TOTAL_OP_CNT (0)     // for verification
#define GYRO_SET_UD_TOTAL_OP_CNT    (0)     // for verification


#ifdef _NVT_EMULATION_
#define _EMULATION_                 (ENABLE)
#else
#define _EMULATION_                 (DISABLE)
#endif

#ifdef _NVT_FPGA_
#define _FPGA_EMULATION_            (ENABLE)
#else
#define _FPGA_EMULATION_            (DISABLE)
#endif

#define SPI_DMA_DIR_SPITX            (1) //tx
#define SPI_DMA_DIR_SPIRX            (2) //rx
#define SPI_DMA_DIR_SPITXRX          (3) //both
//for 538 backward compatible
#define SPI_DMA_DIR_RAM2SPI          (1) //write
#define SPI_DMA_DIR_SPI2RAM          (0) //read

#define SPI_MAX_TRANSFER_CNT        (0xFFFFFF)  ///< max transfer count for spi_writeReadData()

#define SPI_SETREG(ofs,value)       OUTW(IOADDR_SPI_REG_BASE+(ofs),(value))
#define SPI_GETREG(ofs)             INW(IOADDR_SPI_REG_BASE+(ofs))

#define SPI2_SETREG(ofs,value)      OUTW(IOADDR_SPI2_REG_BASE+(ofs),(value))
#define SPI2_GETREG(ofs)            INW(IOADDR_SPI2_REG_BASE+(ofs))

#define SPI3_SETREG(ofs,value)      OUTW(IOADDR_SPI3_REG_BASE+(ofs),(value))
#define SPI3_GETREG(ofs)            INW(IOADDR_SPI3_REG_BASE+(ofs))

#define SPI4_SETREG(ofs,value)      OUTW(IOADDR_SPI4_REG_BASE+(ofs),(value))
#define SPI4_GETREG(ofs)            INW(IOADDR_SPI4_REG_BASE+(ofs))

#define SPI5_SETREG(ofs,value)      OUTW(IOADDR_SPI5_REG_BASE+(ofs),(value))
#define SPI5_GETREG(ofs)            INW(IOADDR_SPI5_REG_BASE+(ofs))

//max support gyro controllers
#define SPI_GYRO_CTRL_COUNT         (2) 

// Software Queue depth is 3 SIE Sync. Max 96 transfer per SIE Sync., Max 4 OP per transfer
#define SPI_GYRO_QUEUE_DEPTH        (SPI_GYRO_TRANSFER_MAX_DMA * SPI_GYRO_TRSLEN_MAX * 3 +1)

// 3 regs for transfer interrupt: SPI_GYROSEN_TRS_STS_REG_OFS, SPI_GYROSEN_TRS_STS2_REG_OFS, SPI_GYROSEN_TRS_STS3_REG_OFS
#define SPI_GYRO_TRS_IDX_SHIFT      (5)
#define SPI_GYRO_TRS_OFST_MASK      (0x1F)
#define SPI_GYRO_INT_MASK           (0xFFE0)

#define SPI_GYRO_FRAME_MAX          (0x7FFFFFFF)
#define SPI_GYRO_UPDATE_REQ         (0xF0000001)   //means this frame needs to update
#define SPI_GYRO_UPDATE_NEXT        (0xF0000002)   //means this frame had been update
#define SPI_GYRO_FRAME_IGNORE       (0xFFFFFFFF)   //means this frame is in initial state
#define SPI_GYRO_OP_DATA_SIZE       (8)     // 64 bits = 8 bytes

#define SPI_GYRO_GET_FRAME_ID(frame_cnt)       ((frame_cnt) & 0x1)  // refer to SPI_GYRO_FRAME_ID


/*
    SPI Capability
*/
typedef enum {
	SPI_CAPABILITY_PIO = 0x0,           //< Support PIO mode
	SPI_CAPABILITY_DMA = 0x1,           //< Support DMA mode
	SPI_CAPABILITY_GYRO = 0x02,         //< Support Gyro polling mode

	SPI_CAPABILITY_1BIT = 0x000,        //< Support 1 bits full duplex mode
	SPI_CAPABILITY_2BITS = 0x100,       //< Support 2 bits half duplex mode
	SPI_CAPABILITY_4BITS = 0x200,       //< Support 4 bits half duplex mode

	ENUM_DUMMY4WORD(SPI_CAPABILITY)
} SPI_CAPABILITY;

/*
    SPI State
*/
typedef enum {
	SPI_STATE_IDLE,
	SPI_STATE_PIO,
	SPI_STATE_DMA,
	SPI_STATE_FLASH,
	SPI_STATE_FLASH_WAIT,
	SPI_STATE_GYRO,

	ENUM_DUMMY4WORD(SPI_STATE)
} SPI_STATE;

/*
    SPI Gyro State
*/
typedef enum {
	SPI_GYRO_STATE_IDLE,
	SPI_GYRO_STATE_RUN,
	SPI_GYRO_STATE_CHANGE_ISSUED,

	ENUM_DUMMY4WORD(SPI_GYRO_STATE)
} SPI_GYRO_STATE;

/*
    SPI Gyro Transfer Interrupt Register
*/
typedef enum {
	SPI_GYRO_TRS_INT_REG_0,
	SPI_GYRO_TRS_INT_REG_1,
	SPI_GYRO_TRS_INT_REG_2,
	SPI_GYRO_TRS_INT_REG_NUM,

	ENUM_DUMMY4WORD(SPI_GYRO_TRS_INT_REG)
} SPI_GYRO_TRS_INT_REG;

typedef enum {
	SPI_GYRO_FRAME_ID_EVEN,
	SPI_GYRO_FRAME_ID_ODD,
	SPI_GYRO_FRAME_ID_NUM,

	ENUM_DUMMY4WORD(SPI_GYRO_FRAME_ID)
} SPI_GYRO_FRAME_ID;

typedef enum {
	SPI_GYRO_TG_INFO_IDX_CUR,
	SPI_GYRO_TG_INFO_IDX_NEXT,
	SPI_GYRO_TG_INFO_IDX_NUM,

	ENUM_DUMMY4WORD(SPI_GYRO_TG_INFO_IDX)
} SPI_GYRO_TG_INFO_IDX;

/*
    SPI register default value
*/
typedef struct {
	UINT32  uiOffset;
	UINT32  uiValue;
	CHAR    *pName;
} SPI_REG_DEFAULT;

/*
    GYRO transfer interrupt reg series
*/
typedef struct {
    // uiReg[0] -> SPI_GYROSEN_TRS_STS_REG_OFS
    // uiReg[1] -> SPI_GYROSEN_TRS_STS2_REG_OFS
    // uiReg[2] -> SPI_GYROSEN_TRS_STS3_REG_OFS
	UINT32  uiReg[SPI_GYRO_TRS_INT_REG_NUM];
} SPI_GYRO_TRS_INT_REG_SERIES;

typedef struct {
    UINT32 uiOpDelay;
    UINT32 uiTransDelay;
    UINT32 uiOffsetDelay;
    UINT32 uiTransferCount;
    UINT32 uiTransferLen;
    UINT32 uiOp0Length;
    UINT32 uiOp1Length;
    UINT32 uiOp2Length;
    UINT32 uiOp3Length;

    UINT32 uiOpDelay_odd;
    UINT32 uiTransDelay_odd;
    UINT32 uiOffsetDelay_odd;
    UINT32 uiTransferCount_odd;
    UINT32 uiTransferLen_odd;
    UINT32 uiOp0Length_odd;
    UINT32 uiOp1Length_odd;
    UINT32 uiOp2Length_odd;
    UINT32 uiOp3Length_odd;

    // DMA mode
    UINT32 uiTotalOpCnt;
    UINT32 uiTotalOpCnt_odd;

    // PIO mode
    UINT32 uiSyncEndTrsId;
    SPI_GYRO_TRS_INT_REG_SERIES vTrsIntEnRegs;
} SPI_GYRO_TG_INFO;

typedef struct {
    UINT32 uiBufAddr;       //current buffer addr (for dma mode or pio mode)
    UINT32 uiNextBufAddr;   //next buffer addr for dma mode (for dma mode only)
    UINT32 uiTotalOpCnt;    //buffer size
    UINT32 uiRemainOpCnt;   //how many op data still not retrive yet via user(for dma mode only)
    UINT32 uiUpdatedFrame;  //frame update status
    UINT32 uiTGInfoPending; //0: no next TGinfo                1: exist a pending next TGinfo that needs to update
    SPI_GYRO_TG_INFO gyroTGInfo[SPI_GYRO_TG_INFO_IDX_NUM];    // store SPI_GYRO_TG_INFO before config updated, [0]: current frame [1]: next frame
} SPI_GYRO_FRAME_STATUS;

typedef struct {
    UINT32 uiFrameCnt;      //The frame no. By using this info we can know current frame is belong to even/odd frame
    UINT32 uiErrState;      //The frame error status
    SPI_GYRO_FRAME_STATUS frameStatus[SPI_GYRO_FRAME_ID_NUM]; //even/odd frame status
} SPI_GYRO_STATUS;


extern ID FLG_ID_SPI;
extern vk_spinlock_t spi_spinlock;
extern SPI_STATE vSpiState[SPI_ID_COUNT];
extern const UINT32 *P_HOST_CAP;
extern const PLL_CLKFREQ vSpiPllClkID[SPI_ID_COUNT];
extern SPI_RDY_ACT_LEVEL spiRdyActiveLevel;
extern const FLGPTN vSpiFlags[SPI_ID_COUNT];

extern BOOL bSpiGyroUnitIsClk;
extern UINT32 vSpiGyroVdSrc[SPI_GYRO_CTRL_COUNT];
extern UINT32 vSpiGyroSyncEndOffset[SPI_GYRO_CTRL_COUNT];
extern SPI_GYRO_CB vSpiGyroHdl[SPI_ID_COUNT];
extern UINT32 uiSpiGyroIntMsk[SPI_GYRO_TRS_INT_REG_NUM];

extern UINT32 pinmux_getPinmux(PIN_FUNC id);
extern REGVALUE spi_getReg(SPI_ID spiID, UINT32 offset);
extern void spi_setReg(SPI_ID spiID, UINT32 offset, REGVALUE value);
extern void spi_waitClrFlag(SPI_ID spiID);

extern ER spi_rstGyroQueue(SPI_ID spiID);
extern void spi_initGyro(unsigned int irq);
extern void spi_initGyroStatus(void);
extern void spi_isrGyroHandle(SPI_ID spiID, UINT32 intr_status);
extern UINT32 spi_gyro_id_hash(SPI_ID spiID);
#endif
//@}