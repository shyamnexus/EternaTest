/*
    LVDS/MIPI-CSI/HiSPi Sensor PHY Configuration Driver internal header

    LVDS/MIPI-CSI/HiSPi Sensor PHY Configuration Driver internal header

    @file       senphy_int.h
    @ingroup
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.
*/

#ifndef _SENPHY_INT_H
#define _SENPHY_INT_H

#ifdef __KERNEL__
#include "dsiphy.h"
#include "dsiphy_reg.h"
#include "dsiphy_drv.h"

#define _SENPHY_TODO_

#else
#include "dsiphy.h"
#include "include/dsiphy_reg.h"
#include "rcw_macro.h"
#include "io_address.h"
#include "pll.h"
#include "pll_protected.h"
#endif


#ifdef __KERNEL__
extern DSIPHY_DATA_TYPE _DSIPHY_REG_BASE_ADDR;
#define DSIPHY_REG_ADDR(ofs)           (_DSIPHY_REG_BASE_ADDR+(ofs))
#define DSIPHY_GETREG(ofs)             INW( _DSIPHY_REG_BASE_ADDR+(ofs) )
#define DSIPHY_SETREG(ofs, value)      OUTW( _DSIPHY_REG_BASE_ADDR+(ofs), (value) )
#else
#define DSIPHY_REG_ADDR(ofs)           (IOADDR_DSI_PHY_REG_BASE+(ofs))
#define DSIPHY_GETREG(ofs)             INW( IOADDR_DSI_PHY_REG_BASE+(ofs))
#define DSIPHY_SETREG(ofs,value)       OUTW(IOADDR_DSI_PHY_REG_BASE+(ofs),(value))
#endif

#ifdef _BSP_NA51055_
#define DSIPHY_MODULES_NUMBER           2
#define DSIPHY_DATALANE_NUMBER          4
#else
#define DSIPHY_MODULES_NUMBER           8
#define DSIPHY_DATALANE_NUMBER          8
#endif


#define DSIPHY_DEBUG                   ENABLE
#if DSIPHY_DEBUG
#define dsiphy_debug(msg)              DBG_WRN  msg
#else
#define dsiphy_debug(msg)

#endif

#endif
