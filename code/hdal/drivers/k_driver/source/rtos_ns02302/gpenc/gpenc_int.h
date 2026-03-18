/*
    Internal header file for GPENC module.

    This file is the internal header file that defines the variables, structures
    and function prototypes of GPENC module.

    @file       gpenc_int.h
    @ingroup    miDrvDispaly_GPENC
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _GPENC_INT_H
#define _GPENC_INT_H

//#include "DrvCommon.h"
#include <rcw_macro.h>
#include <kwrap/nvt_type.h>
#include <kwrap/semaphore.h>
#include <kwrap/flag.h>
#include <kwrap/spinlock.h>
#include <io_address.h>
//#include "sys.h"


/**
    @addtogroup miDrvDispaly_GPENC
*/
//@{
/*#if defined(_PCIE_EMU_ON_)
#define GPENC_REG_ADDR(ofs)           (IOADDR_GPENC_REG_BASE+(ofs))
#define GPENC_GETREG(ofs)             INW(APB_LOCAL2PCIE(IOADDR_GPENC_REG_BASE)+(ofs))
#define GPENC_SETREG(ofs, value)      pcie_bus_write_reg(APB_LOCAL2PCIE(IOADDR_GPENC_REG_BASE), ofs, (value))//OUTW(APB_LOCAL2PCIE(IOADDR_GPENC_REG_BASE)+(ofs), (value))

#define GPENC2_REG_ADDR(ofs)           (IOADDR_GPENC2_REG_BASE+(ofs))
#define GPENC2_GETREG(ofs)             INW(APB_LOCAL2PCIE(IOADDR_GPENC2_REG_BASE)+(ofs))
#define GPENC2_SETREG(ofs, value)      pcie_bus_write_reg(APB_LOCAL2PCIE(IOADDR_GPENC2_REG_BASE), ofs, (value))//OUTW(APB_LOCAL2PCIE(IOADDR_GPENC2_REG_BASE)+(ofs), (value))
#else
*/
#define GPENC_REG_ADDR(ofs)           (IOADDR_GPENC_REG_BASE+(ofs))
#define GPENC_GETREG(ofs)             INW(IOADDR_GPENC_REG_BASE+(ofs))
#define GPENC_SETREG(ofs, value)      OUTW(IOADDR_GPENC_REG_BASE+(ofs), (value))


#define GPENC2_REG_ADDR(ofs)           (IOADDR_GPENC2_REG_BASE+(ofs))
#define GPENC2_GETREG(ofs)             INW(IOADDR_GPENC2_REG_BASE+(ofs))
#define GPENC2_SETREG(ofs, value)      OUTW(IOADDR_GPENC2_REG_BASE+(ofs), (value))
//#endif

//@}

#endif
