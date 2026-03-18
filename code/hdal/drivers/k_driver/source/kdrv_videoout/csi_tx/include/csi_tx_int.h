/*
    MIPI-CSI_TX Controller internal header

    MIPI-CSI_TX Controller internal header

    @file       csi_tx_int.h
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
*/
#ifndef __CSI_TX_INT_H__
#define __CSI_TX_INT_H__

#ifdef __KERNEL__
#include <rcw_macro.h>
#include <linux/module.h>
//#include <mach/ioaddress.h>
#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "csi_tx_dbg.h"
#include "csi_tx.h"
#include "csi_tx_reg.h"


#include <linux/io.h>



#else
#if defined(__FREERTOS)
#include <kwrap/debug.h>
#include <kwrap/spinlock.h>
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "csi_tx.h"
#include "csi_tx_reg.h"
#include "io_address.h"
#include "pll.h"
#include "pll_protected.h"
#include "top.h"
//#include <sys.h> // pcie
#else
#include "DrvCommon.h"
#include "pll.h"
#include "pll_protected.h"
//#include "csi_tx.h"
#include "csi_tx_reg.h"
#endif

#if 0 //if (_TC18039_)
#include "../../i2c/i2c.h"
#endif

#endif

#include <io_address.h>


// -----------------------------------------------------------------------------
//  CSI_TX register access definition
// -----------------------------------------------------------------------------
#ifdef __KERNEL__
extern CSI_TX_DATA_TYPE _CSI_TX_REG_BASE_ADDR;
#define CSI_TX_REG_ADDR(ofs)           (_CSI_TX_REG_BASE_ADDR+(ofs))
#define CSI_TX_GETREG(ofs)             INW(_CSI_TX_REG_BASE_ADDR+(ofs))
#define CSI_TX_SETREG(ofs, value)       OUTW(_CSI_TX_REG_BASE_ADDR+(ofs), (value))

extern CSI_TX_DATA_TYPE _CSI_TX2_REG_BASE_ADDR;
#define CSI_TX2_REG_ADDR(ofs)           (_CSI_TX2_REG_BASE_ADDR+(ofs))
#define CSI_TX2_GETREG(ofs)             INW(_CSI_TX2_REG_BASE_ADDR+(ofs))
#define CSI_TX2_SETREG(ofs, value)       OUTW(_CSI_TX2_REG_BASE_ADDR+(ofs), (value))

/*
extern CSI_TX_DATA_TYPE _CSI_TX_PHY_REG_BASE_ADDR;
#define CSI_TXPHY_REG_ADDR(ofs)           (_CSI_TX_PHY_REG_BASE_ADDR+(ofs))
#define CSI_TXPHY_GETREG(ofs)             INW(_CSI_TX_PHY_REG_BASE_ADDR+(ofs))
#define CSI_TXPHY_SETREG(ofs, value)       OUTW(_CSI_TX_PHY_REG_BASE_ADDR+(ofs), (value))
*/
#else



#if 1
#define CSI_TX_REG_ADDR(ofs)           (IOADDR_CSI_TX_REG_BASE+(ofs))
#define CSI_TX_GETREG(ofs)             INW(IOADDR_CSI_TX_REG_BASE+(ofs))
#define CSI_TX_SETREG(ofs, value)       OUTW(IOADDR_CSI_TX_REG_BASE+(ofs), (value))

#define CSI_TX2_REG_ADDR(ofs)           (IOADDR_CSI_TX2_REG_BASE+(ofs))
#define CSI_TX2_GETREG(ofs)             INW(IOADDR_CSI_TX2_REG_BASE+(ofs))
#define CSI_TX2_SETREG(ofs, value)       OUTW(IOADDR_CSI_TX2_REG_BASE+(ofs), (value))

#define CSI_TXPHY_REG_ADDR(ofs)           (IOADDR_DSI_PHY_REG_BASE+(ofs))
#define CSI_TXPHY_GETREG(ofs)             INW(IOADDR_DSI_PHY_REG_BASE+(ofs))
#define CSI_TXPHY_SETREG(ofs, value)       OUTW(IOADDR_DSI_PHY_REG_BASE+(ofs), (value))

#else // pcie

	#if 0//pcie1
	#define CSI_TX_REG_ADDR(ofs)           ( APB_LOCAL2PCIE(IOADDR_CSI_TX_REG_BASE)+(ofs))
	#define CSI_TX_GETREG(ofs)             INW( APB_LOCAL2PCIE(IOADDR_CSI_TX_REG_BASE) +(ofs))
	#define CSI_TX_SETREG(ofs, value)       OUTW( (APB_LOCAL2PCIE(IOADDR_CSI_TX_REG_BASE)+(ofs)) , (value))

	#define CSI_TX2_REG_ADDR(ofs)           ( APB_LOCAL2PCIE(IOADDR_CSI_TX2_REG_BASE)+(ofs))
	#define CSI_TX2_GETREG(ofs)             INW( APB_LOCAL2PCIE(IOADDR_CSI_TX2_REG_BASE) +(ofs))
	#define CSI_TX2_SETREG(ofs, value)       OUTW( (APB_LOCAL2PCIE(IOADDR_CSI_TX2_REG_BASE)+(ofs)) , (value))

	#define CSI_TXPHY_REG_ADDR(ofs)           ( APB_LOCAL2PCIE(IOADDR_DSI_PHY_REG_BASE)+(ofs) )
	#define CSI_TXPHY_GETREG(ofs)             INW(APB_LOCAL2PCIE(IOADDR_DSI_PHY_REG_BASE)+(ofs))
	#define CSI_TXPHY_SETREG(ofs, value)       OUTW( (APB_LOCAL2PCIE(IOADDR_DSI_PHY_REG_BASE)+(ofs)), (value))

	#else // pcie2
	#define CSI_TX_REG_ADDR(ofs)           ( APB_LOCAL2PCIE2(IOADDR_CSI_TX_REG_BASE)+(ofs))
	#define CSI_TX_GETREG(ofs)             INW( APB_LOCAL2PCIE2(IOADDR_CSI_TX_REG_BASE) +(ofs))
	#define CSI_TX_SETREG(ofs, value)       OUTW( (APB_LOCAL2PCIE2(IOADDR_CSI_TX_REG_BASE)+(ofs)) , (value))

	#define CSI_TX2_REG_ADDR(ofs)           ( APB_LOCAL2PCIE2(IOADDR_CSI_TX2_REG_BASE)+(ofs))
	#define CSI_TX2_GETREG(ofs)             INW( APB_LOCAL2PCIE2(IOADDR_CSI_TX2_REG_BASE) +(ofs))
	#define CSI_TX2_SETREG(ofs, value)       OUTW( (APB_LOCAL2PCIE2(IOADDR_CSI_TX2_REG_BASE)+(ofs)) , (value))

	#define CSI_TXPHY_REG_ADDR(ofs)           ( APB_LOCAL2PCIE2(IOADDR_DSI_PHY_REG_BASE)+(ofs) )
	#define CSI_TXPHY_GETREG(ofs)             INW(APB_LOCAL2PCIE2(IOADDR_DSI_PHY_REG_BASE)+(ofs))
	#define CSI_TXPHY_SETREG(ofs, value)       OUTW( (APB_LOCAL2PCIE2(IOADDR_DSI_PHY_REG_BASE)+(ofs)), (value))
	#endif

#endif

#endif




//#define CSI_TX_PHY

// -----------------------------------------------------------------------------
// General definition
// -----------------------------------------------------------------------------
#define CSI_TX_DEBUG                       ENABLE

#define CSI_TX_LOG_CFG                     DISABLE

#define CSI_TX_ERR_MSG(...)               DBG_ERR(__VA_ARGS__)

#define CSI_TX_WRN_MSG(...)               DBG_WRN(__VA_ARGS__)

#if (CSI_TX_LOG_CFG == ENABLE)
#define CSI_TX_LOG_MSG(...)               DBG_DUMP(__VA_ARGS__)
#else
#define CSI_TX_LOG_MSG(...)
#endif


// -----------------------------------------------------------------------------
// CSI_TX Escape control (0x24,0x28)
// -----------------------------------------------------------------------------
typedef enum {
	CSI_TX_SET_DAT0_ESC_START = 0x0,
	CSI_TX_SET_DAT1_ESC_START,
	CSI_TX_SET_DAT2_ESC_START,
	CSI_TX_SET_DAT3_ESC_START,

	CSI_TX_SET_DAT0_ESC_STOP,
	CSI_TX_SET_DAT1_ESC_STOP,
	CSI_TX_SET_DAT2_ESC_STOP,
	CSI_TX_SET_DAT3_ESC_STOP,

	CSI_TX_SET_CLK_ULP_SEL,

	CSI_TX_SET_DAT0_ESC_CMD,
	CSI_TX_SET_DAT1_ESC_CMD,
	CSI_TX_SET_DAT2_ESC_CMD,
	CSI_TX_SET_DAT3_ESC_CMD,

	ENUM_DUMMY4WORD(CSI_TX_CFG_ESCAPE_CMD_CTRL)
} CSI_TX_CFG_ESCAPE_CMD_CTRL;

// -----------------------------------------------------------------------------
// Interrupt enabled register (0x80)
// -----------------------------------------------------------------------------
#define CSI_TX_TX_DIS_INTEN                0x00000001  // bit[0]
#define CSI_TX_FRM_START_INTEN             0x00000002  // bit[1]
#define CSI_TX_FRM_END_INTEN               0x00000004  // bit[2]
#define CSI_TX_FIFO_UNDER_INTEN            0x00000008  // bit[3]
#define CSI_TX_FRM_TOTAL_END_INTEN         0x00000040  // bit[6]
#define CSI_TX_LINEBUF_EMPTY_INTEN         0x00000080  // bit[7]


#define DAT0_ESC_DONE_INTEN             0x01000000  // bit[24]
#define DAT1_ESC_DONE_INTEN             0x02000000  // bit[25]
#define DAT2_ESC_DONE_INTEN             0x04000000  // bit[26]
#define DAT3_ESC_DONE_INTEN             0x08000000  // bit[27]
#define CLK_ULPS_DONE_INTEN             0x10000000  // bit[28]
#define DAT_ESC_DONE_STS_MASK           0x0F000000  // bit[28]
#define CSI_TX_ALL_INTEN		        0x1F0000FF  // ALL INTEN
// bit[29]
// bit[30]
// bit[31]

// -----------------------------------------------------------------------------
// Interrupt enabled register (0x88)
// -----------------------------------------------------------------------------
#define CSI_TX_TX_DIS_STS               0x00000001  // bit[0]
#define CSI_TX_FRM_START_STS            0x00000002  // bit[1]
#define CSI_TX_FRM_END_STS              0x00000004  // bit[2]
#define CSI_TX_FIFO_UNDER_STS           0x00000008  // bit[3]
#define CSI_TX_FRM_WIDTH_ERR_STS        0x00000010  // bit[4]
#define CSI_TX_FRM_HEIGHT_ERR_STS       0x00000020  // bit[5]
#define CSI_TX_FRM_TOTAL_END_STS        0x00000040  // bit[6]
#define CSI_TX_LINEBUF_EMPTY_STS        0x00000080  // bit[7]


#define DAT0_ESC_DONE_STS               0x01000000  // bit[24]
#define DAT1_ESC_DONE_STS               0x02000000  // bit[25]
#define DAT2_ESC_DONE_STS               0x04000000  // bit[26]
#define DAT3_ESC_DONE_STS               0x08000000  // bit[27]
#define CLK_ULPS_DONE_STS               0x10000000  // bit[28]
// bit[29]
// bit[30]
// bit[31]


#define CSI_TX_ESC_SET_TRIGGER(lane, ctx, en);\
	{                                          \
		switch (lane) {                        \
		default:                               \
		case CSI_TX_DATA_LANE_0:                  \
			ctx.bit.DAT0_ESC_TRIG = en;        \
			break;                             \
		case CSI_TX_DATA_LANE_1:                  \
			ctx.bit.DAT1_ESC_TRIG = en;        \
			break;                             \
		case CSI_TX_DATA_LANE_2:                  \
			ctx.bit.DAT2_ESC_TRIG = en;        \
			break;                             \
		case CSI_TX_DATA_LANE_3:                  \
			ctx.bit.DAT3_ESC_TRIG = en;        \
			break;                             \
		}                                      \
	}
#define CSI_TX_ESC_SET_START(lane, ctx, en);  \
	{                                          \
		switch (lane) {                        \
		default:                               \
		case CSI_TX_DATA_LANE_0:                  \
			ctx.bit.DAT0_ESC_START = en;       \
			break;                             \
		case CSI_TX_DATA_LANE_1:                  \
			ctx.bit.DAT1_ESC_START = en;       \
			break;                             \
		case CSI_TX_DATA_LANE_2:                  \
			ctx.bit.DAT2_ESC_START = en;       \
			break;                             \
		case CSI_TX_DATA_LANE_3:                  \
			ctx.bit.DAT3_ESC_START = en;       \
			break;                             \
		}                                      \
	}
#define CSI_TX_ESC_SET_STOP(lane, ctx, en);   \
	{                                          \
		switch (lane) {                        \
		default:                               \
		case CSI_TX_DATA_LANE_0:                  \
			ctx.bit.DAT0_ESC_STOP = en;        \
			break;                             \
		case CSI_TX_DATA_LANE_1:                  \
			ctx.bit.DAT1_ESC_STOP = en;        \
			break;                             \
		case CSI_TX_DATA_LANE_2:                  \
			ctx.bit.DAT2_ESC_STOP = en;        \
			break;                             \
		case CSI_TX_DATA_LANE_3:                  \
			ctx.bit.DAT3_ESC_STOP = en;        \
			break;                             \
		}                                      \
	}
#define CSI_TX_ESC_SET_CMD(lane, ctx, en);    \
	{                                          \
		switch (lane) {                        \
		default:                               \
		case CSI_TX_DATA_LANE_0:                  \
			ctx.bit.DAT0_ESC_CMD = en;         \
			break;                             \
		case CSI_TX_DATA_LANE_1:                  \
			ctx.bit.DAT1_ESC_CMD = en;         \
			break;                             \
		case CSI_TX_DATA_LANE_2:                  \
			ctx.bit.DAT2_ESC_CMD = en;         \
			break;                             \
		case CSI_TX_DATA_LANE_3:                  \
			ctx.bit.DAT3_ESC_CMD = en;         \
			break;                             \
		}                                      \
	}
//#define CSI_TX_ESC_SET_TRIGGER(lane, ctx, en)   (lane == CSI_TX_DATA_LANE_0)?(ctx.bit.DAT0_ESC_TRIG=en):(ctx.bit.DAT1_ESC_TRIG=en)

//#define CSI_TX_ESC_SET_START(lane, ctx, en)     (lane == CSI_TX_DATA_LANE_0)?(ctx.bit.DAT0_ESC_START=en):(ctx.bit.DAT1_ESC_START=en)

//#define CSI_TX_ESC_SET_STOP(lane, ctx, en)    (lane == CSI_TX_DATA_LANE_0)?(ctx.bit.DAT0_ESC_STOP=en):(ctx.bit.DAT1_ESC_STOP=en)

//#define CSI_TX_ESC_SET_CMD(lane, ctx, cmd)    (lane == CSI_TX_DATA_LANE_0)?(ctx.bit.DAT0_ESC_CMD=cmd):(ctx.bit.DAT1_ESC_CMD=cmd)


extern CSI_TX_MODESEL  csi_tx_get_mode(void);
#if defined(__FREERTOS) && defined(_NVT_FPGA_)
//extern void csi_tx_phy_init(void);
extern ER csi_tx_phy_writereg(UINT32 ui_offset, UINT32 ui_value);
extern ER csi_tx_phy_readreg(UINT32 ui_offset, UINT32 *pui_value);
#endif
#endif
