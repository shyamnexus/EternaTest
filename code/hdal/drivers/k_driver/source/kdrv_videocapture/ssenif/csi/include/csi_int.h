/*
    MIPI CSI Controller internal header

    MIPI CSI Controller internal header

    @file       csi_int.h
    @ingroup    mIDrvIO_CSI
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/
#ifndef _CSI_INT_H
#define _CSI_INT_H

#if defined(_NVT_FPGA_) || defined(CONFIG_NVT_FPGA_EMULATION)
#define _FPGA_EMULATION_ 1
#else
#define _FPGA_EMULATION_ 0
#endif

#include <nvt_api_ver.h>

#ifdef __KERNEL__
#include "csi_reg.h"
#include "csi_rule.h"
#include <rcw_macro.h>
#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "csi_dbg.h"
#include <linux/io.h>
#include "../../senphy.h"
#include <plat/top.h>
#else
#if defined(__FREERTOS)
#include "csi_reg.h"
#include "csi_rule.h"
#include "../../senphy.h"
#include "kwrap/error_no.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"

#include "rcw_macro.h"
#include "io_address.h"
#include "interrupt.h"
#include "pll.h"
#include "pll_protected.h"
#include "top.h"
//#include "../../../../../../../../lib/include/Utility/SwTimer.h"
#include "Utility/SwTimer.h"
#else
#include "DrvCommon.h"
#include "pll.h"
#include "pll_protected.h"
#include "../SENPHY/senphy_int.h"
#include "SwTimer.h"
#endif
#endif

#define _CSI_VER_TOKEN_STR(a)  #a
#define CSI_VER_TOKEN_STR(a)  _CSI_VER_TOKEN_STR(a)

#define CSI_DRV_NAME "kdrv_ssenif/csi"
#define CSI_VER_MAJOR 1
#define CSI_VER_MINOR 01
#define CSI_VER_PATCH 001
#define CSI_VER_EXT 00
#define CSI_DRV_MODULE_VERSION CSI_VER_TOKEN_STR(CSI_VER_MAJOR) "." CSI_VER_TOKEN_STR(CSI_VER_MINOR) "." CSI_VER_TOKEN_STR(CSI_VER_PATCH) "." CSI_VER_TOKEN_STR(CSI_VER_EXT)


#define CSI_FORCE_OFF               DISABLE
#define CSI_DEBUG                   DISABLE
#define CSI_ECO_ENABLE              DISABLE
#define CSI_ECO_HWRST               DISABLE
#define CSI_ENABLE_DESKEW           ENABLE
#define CSI_ENABLE_DAT_INV_DONE     ENABLE	// for senphy P/N swap
#define CSI_CLK_SWITCH              DISABLE	// 538 not support

//
//  MIPI-CSI register access definition
//
#ifndef __KERNEL__
#define CSI_REG_ADDR(ofs)           (IOADDR_CSI_REG_BASE+(ofs))
#define CSI_GETREG(ofs)             INW(IOADDR_CSI_REG_BASE+(ofs))

#define CSI2_REG_ADDR(ofs)          (IOADDR_CSI2_REG_BASE+(ofs))
#define CSI2_GETREG(ofs)            INW(IOADDR_CSI2_REG_BASE+(ofs))

#define CSI3_REG_ADDR(ofs)          (IOADDR_CSI3_REG_BASE+(ofs))
#define CSI3_GETREG(ofs)            INW(IOADDR_CSI3_REG_BASE+(ofs))

#define CSI4_REG_ADDR(ofs)          (IOADDR_CSI4_REG_BASE+(ofs))
#define CSI4_GETREG(ofs)            INW(IOADDR_CSI4_REG_BASE+(ofs))

#define CSI5_REG_ADDR(ofs)          (IOADDR_CSI5_REG_BASE+(ofs))
#define CSI5_GETREG(ofs)            INW(IOADDR_CSI5_REG_BASE+(ofs))


#define CSI_SIE_REG_ADDR(ofs)       (IOADDR_SIE_REG_BASE+(ofs))
#define CSI_SIE_GETREG(ofs)         INW(IOADDR_SIE_REG_BASE+(ofs))
#define CSI_SIE_SETREG(ofs, value)   OUTW(IOADDR_SIE_REG_BASE+(ofs), (value))
#define FLGPTN_CSI				FLGPTN_BIT(0)
#define FLGPTN_CSI2				FLGPTN_BIT(0)
#define FLGPTN_CSI3				FLGPTN_BIT(0)
#define FLGPTN_CSI4				FLGPTN_BIT(0)
#define FLGPTN_CSI5				FLGPTN_BIT(0)
#else
extern ULONG _CSI_REG_BASE_ADDR;
#define CSI_REG_ADDR(ofs)           (_CSI_REG_BASE_ADDR+(ofs))
#define CSI_GETREG(ofs)             ioread32((void *)(_CSI_REG_BASE_ADDR+(ofs)))

extern ULONG _CSI2_REG_BASE_ADDR;
#define CSI2_REG_ADDR(ofs)          (_CSI2_REG_BASE_ADDR+(ofs))
#define CSI2_GETREG(ofs)            ioread32((void *)(_CSI2_REG_BASE_ADDR+(ofs)))

extern ULONG _CSI3_REG_BASE_ADDR;
#define CSI3_REG_ADDR(ofs)          (_CSI3_REG_BASE_ADDR+(ofs))
#define CSI3_GETREG(ofs)            ioread32((void *)(_CSI3_REG_BASE_ADDR+(ofs)))

extern ULONG _CSI4_REG_BASE_ADDR;
#define CSI4_REG_ADDR(ofs)          (_CSI4_REG_BASE_ADDR+(ofs))
#define CSI4_GETREG(ofs)            ioread32((void *)(_CSI4_REG_BASE_ADDR+(ofs)))

extern ULONG _CSI5_REG_BASE_ADDR;
#define CSI5_REG_ADDR(ofs)          (_CSI5_REG_BASE_ADDR+(ofs))
#define CSI5_GETREG(ofs)            ioread32((void *)(_CSI5_REG_BASE_ADDR+(ofs)))


extern ULONG _SIE_CSI_REG_BASE_ADDR;
#define CSI_SIE_REG_ADDR(ofs)       (0xFDC00000+(ofs))
#define CSI_SIE_GETREG(ofs)         INW(0xFDC00000+(ofs))
#define CSI_SIE_SETREG(ofs, value)   OUTW(0xFDC00000+(ofs), (value))

// CSI module clock source
#define CSI_CLK_SRC_60   0
#define CSI_CLK_SRC_120  1
#define CSI_CLK_SRC_240  2

#endif
extern void (*CSI_SETREG)(UINT32 offset, REGVALUE value);
extern void (*CSI2_SETREG)(UINT32 offset, REGVALUE value);
extern void (*CSI3_SETREG)(UINT32 offset, REGVALUE value);
extern void (*CSI4_SETREG)(UINT32 offset, REGVALUE value);
extern void (*CSI5_SETREG)(UINT32 offset, REGVALUE value);
extern NVT_API_CHK_DECLARE(csi);


#if _FPGA_EMULATION_
#define CSI_DEFAULT_INT0            (0)
#define CSI_DEFAULT_INT1            (0)
#define CSI_DEFAULT_INT0_FS         (0)
#else
#define CSI_DEFAULT_INT0            (0x40F0F203)
#define CSI_DEFAULT_INT1            (0x000000FF)
#define CSI_DEFAULT_INT0_FS         (0x00000040)
#endif

#define CSI_PSC_INT1                (0x0FF00000)	// PSC_SHORT & PSC_DONE, D0~D3
#define CSI_PSC_SHORT_INT1          (0x0F000000)

#if CSI_DEBUG
#define CSI_OPTION_MSG_PUT          DISABLE
#define CSI2_OPTION_MSG_PUT         DISABLE
#endif

#define CSI_INT_BANK0               0
#define CSI_INT_BANK1               1


// CSI Internal Usage Definition
#define CSI_SWAP_MSK                0x3
#define CSI_SWAP_MSK7               0x7
#define CSI_SWAP_DATALANE0_OFS      0
#define CSI_SWAP_DATALANE1_OFS      4
#define CSI_SWAP_DATALANE2_OFS      8
#define CSI_SWAP_DATALANE3_OFS      12

#define CSI_PINSWAP_OFS             16
#define CSI_LPSWAP_OFS             0

#define CSI_ANALOG_DLY_MSK          0x7

//void csi_install_cmd(void);
BOOL csi_print_info_to_uart(CHAR *strcmd);
BOOL csi2_print_info_to_uart(CHAR *strcmd);
BOOL csi3_print_info_to_uart(CHAR *strcmd);
BOOL csi4_print_info_to_uart(CHAR *strcmd);
BOOL csi5_print_info_to_uart(CHAR *strcmd);
BOOL csi6_print_info_to_uart(CHAR *strcmd);
BOOL csi7_print_info_to_uart(CHAR *strcmd);
BOOL csi8_print_info_to_uart(CHAR *strcmd);

void csi_error_parser(CSI_ID csi_id, UINT32 sts0, UINT32 sts1, UINT32 line_sta1);

#endif
