/*
    IDE Memory Interface internal header

    IDE Memory Interface internal header

    @file       mi_int.h
    @ingroup    mIDrvDisp_MI
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2010.  All rights reserved.
*/
#ifndef _MI_INT_H
#define _MI_INT_H

#ifdef __KERNEL__
#include <rcw_macro.h>
#include <linux/module.h>

//#include <mach/ioaddress.h>
#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#else
#if defined(__FREERTOS)
#include "rcw_macro.h"
#include "kwrap/type.h"
#include "io_address.h"
#else
#include "DrvCommon.h"
#endif
#endif
#include <nvt_api_ver.h>

//
//  Register access definition
//
#ifdef __KERNEL__

#define MI_REG_ADDR(ofs)           (_IOADDR_MI_REG_BASE +(ofs))
#define MI_GETREG(ofs)             INW(_IOADDR_MI_REG_BASE +(ofs))
#else

#define MI_GETREG(ofs)          INW((IOADDR_MI_REG_BASE+(ofs)))
#endif

#define MI_DEBUG    1
#if MI_DEBUG
#define mi_debug(msg)           DBG_IND msg
#else
#define mi_debug(msg)
#endif

//
//  MI internal register definition
//
#define MI_CMD_DONE_INT_EN      (0x1 << 0)
#define MI_FM_END_INT_EN        (0x1 << 1)
#define MI_FIFO_INT_EN          (0x1 << 2) //Fifo overflow
#define MI_SYNC_INT_EN          (0x1 << 3) //Sync interrupt

#define MI_CMD_DONE_INT_STS     (0x1 << 0)
#define MI_FM_END_INT_STS       (0x1 << 1)
#define MI_FIFO_INT_STS         (0x1 << 2)
#define MI_SYNC_INT_STS         (0x1 << 3)

extern void (*MI_SETREG)(UINT32 offset, REGVALUE value);
extern NVT_API_CHK_DECLARE(mi);

#endif
