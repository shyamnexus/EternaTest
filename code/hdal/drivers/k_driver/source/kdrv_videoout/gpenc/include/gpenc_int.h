/*
    GPENC Controller internal header

    GPENC Controller internal header

    @file       gpenc_int.h
    @ingroup    mIDrvDisp_gpenc
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/
#ifndef __GPENC_INT_H__
#define __GPENC_INT_H__

#ifdef __KERNEL__
#include <rcw_macro.h>
#include <linux/module.h>

//#include <mach/ioaddress.h>
#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "gpenc_dbg.h"
#include "gpenc.h"
#include "gpenc_reg.h"
#else
#if defined(__FREERTOS)
#include <kwrap/debug.h>
#include <kwrap/spinlock.h>
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "gpenc.h"
#include "gpenc_reg.h"
#include "io_address.h"
#include "pll.h"
#include "pll_protected.h"
#include "top.h"
#else
#include "DrvCommon.h"
#include "pll.h"
#include "pll_protected.h"
#include "gpenc.h"
#include "gpenc_reg.h"
#endif
#endif
#include <nvt_api_ver.h>


// -----------------------------------------------------------------------------
// General definition
// -----------------------------------------------------------------------------
#define GPENC_DEBUG                       ENABLE

#define GPENC_LOG_CFG                     DISABLE


#if (GPENC_LOG_CFG == ENABLE)
#define GPENC_LOG_MSG(...)               DBG_DUMP(__VA_ARGS__)
#else
#define GPENC_LOG_MSG(...)
#endif

extern void (*gpenc_set_reg)(UINT32 offset, REGVALUE value);
extern REGVALUE (*gpenc_get_reg)(UINT32 offset);
extern NVT_API_CHK_DECLARE(gpenc);

extern void (*gpenc2_set_reg)(UINT32 offset, REGVALUE value);
extern REGVALUE (*gpenc2_get_reg)(UINT32 offset);
extern NVT_API_CHK_DECLARE(gpenc2);


#endif
