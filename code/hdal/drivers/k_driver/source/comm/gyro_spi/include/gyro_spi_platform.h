/*
    @file       spi_int.h

    @ingroup    mIDrvIO_SPI

    @brief      NT98538 SPI internal header file

    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.

*/

#ifndef _SPI_PLATFORM_H
#define _SPI_PLATFORM_H

#include <linux/soc/nvt/fmem.h>
#include <linux/clk.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include "kwrap/spinlock.h"
#include "kwrap/nvt_type.h"
#include "kwrap/flag.h"
#include "kwrap/semaphore.h"
#include "kwrap/cpu.h"
#include <plat/top.h>
#include "comm/gyro_spi.h"


#define FLGPTN_SPI          (1<<0)
#define FLGPTN_SPI2         (1<<1)
#define FLGPTN_SPI3         (1<<2)
#define FLGPTN_SPI4         (1<<3)
#define FLGPTN_SPI5         (1<<4)
#define FLGPTN_SPI_TIMEOUT  (1<<5)
#define FLGPTN_SPI_TIMEOUT2 (1<<6)
#define FLGPTN_SPI_TIMEOUT3 (1<<7)
#define FLGPTN_SPI_TIMEOUT4 (1<<8)
#define FLGPTN_SPI_TIMEOUT5 (1<<9)
#define FLGPTN_SPI_DMA      (1<<10)
#define FLGPTN_SPI_DMA2     (1<<11)
#define FLGPTN_SPI_DMA3     (1<<12)
#define FLGPTN_SPI_DMA4     (1<<13)
#define FLGPTN_SPI_DMA5     (1<<14)

#ifndef DRV_CB
typedef void (*DRV_CB)(ULONG event);
#endif

void spi_platform_cache_sync_dma_to_dev(VOS_ADDR addr, ULONG size);
void spi_platform_cache_sync_dma_from_dev(VOS_ADDR addr, ULONG size);
void spi_platform_get_clk_freq(UINT32 *freq);
void spi_platform_set_clk_freq(UINT32 freq);
void spi_platform_init(SPI_LINUX_ACT_CB act_cb);
void spi_platform_uninit(void);
unsigned long spi_platform_spinlock_lock(void);
void spi_platform_spinlock_unlock(unsigned long flags);
ER spi_platform_wait_sem(SPI_ID spiID);
ER spi_platform_sig_sem(SPI_ID spiID);
void spi_platform_clr_flg(FLGPTN flags);
void spi_platform_set_flg(FLGPTN flags);
void spi_platform_wait_flg(PFLGPTN flagPtr, FLGPTN flags, UINT mode);
#endif