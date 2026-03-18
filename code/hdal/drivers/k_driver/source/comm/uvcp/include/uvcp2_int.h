/*
    UVCP module internal header file

    UVCP module internal header file

    @file       uvcp_int.h
    @ingroup    mIDrvIO_UVCP
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2020.  All rights reserved.
*/

#ifndef _UVCP2_INT_H
#define _UVCP2_INT_H

#ifndef __KERNEL__
#include "kwrap/error_no.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/spinlock.h"
#include "kwrap/task.h"
#include "kwrap/util.h"

#include "rcw_macro.h"
#include "kwrap/type.h"
#include "io_address.h"
#include "interrupt.h"
#include "uvcp2.h"
#include "dma.h"
#include "dma_protected.h"
#else
#include "uvcp2.h"

#include <linux/soc/nvt/rcw_macro.h>
#include "kwrap/type.h"
#include "uvcp_drv.h"
#include "uvcp_dbg.h"
#include <linux/soc/nvt/fmem.h>

#include "kwrap/error_no.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/spinlock.h"
#include "kwrap/task.h"
#include "kwrap/util.h"
#include "kwrap/platform.h"

#endif


#ifdef __KERNEL__

#define _UVCP_KERN_SELFTEST 0


#if _UVCP_KERN_SELFTEST
#define dma_flushWriteCache(addr, size)  fmem_dcache_sync((void *)addr, size, DMA_TO_DEVICE)
#define dma_flushReadCache(addr, size)   fmem_dcache_sync((void *)addr, size, DMA_FROM_DEVICE)
#define dma_getPhyAddr(parm)             fmem_lookup_pa(parm)
#else
#define dma_flushWriteCache(addr, size)
#define dma_flushReadCache(addr, size)
#define dma_getPhyAddr(parm) parm
#endif
#endif

#define FLGPTN_UVCP2      FLGPTN_BIT(0)
#define FLGPTN_UVCP2_BRK  FLGPTN_BIT(1)

#define UVCP2_POLLING DISABLE


#endif
