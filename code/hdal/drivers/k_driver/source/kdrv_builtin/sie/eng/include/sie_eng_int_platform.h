/*
    Internal header file for SIE module.

    Copyright Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#ifndef _SIE_ENG_INT_PLATFORM_H_
#define _SIE_ENG_INT_PLATFORM_H_

#if defined (__LINUX)
#include <linux/export.h>
#include <linux/io.h>
#include <linux/slab.h>
#else
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#endif

#include "kwrap/type.h"
#include "kwrap/malloc.h"
#include "kwrap/spinlock.h"
#include "sie_eng_handle.h"

/*************************************************************************************
 *  Platform Common Definition
 *************************************************************************************/
#if !defined (__LINUX)
#define EXPORT_SYMBOL(x)
#endif

/*************************************************************************************
 *  Platform I/O read/write Definition
 *************************************************************************************/
#if defined (__LINUX)
#define SIE_ENG_GETREG(_base, _ofs)                 (ioread32((void __iomem *)((_base)+(_ofs))))
#else
#define SIE_ENG_GETREG(_base, _ofs)                 (((volatile UINT32 *)(_base))[(_ofs)>>2])
#endif

extern void (*SIE_ENG_SETREG)(uintptr_t vbase, unsigned int ofs, unsigned int val);

/*************************************************************************************
 *  Platform SpinLock Definition
 *************************************************************************************/
#define SIE_SPIN_LOCK_INIT(_lock)                   vk_spin_lock_init(&_lock)
#define SIE_SPIN_LOCK_IRQSAVE(_lock, _flags)        vk_spin_lock_irqsave(&_lock, _flags)
#define SIE_SPIN_UNLOCK_IRQRESTORE(_lock, _flags)   vk_spin_unlock_irqrestore(&_lock, _flags)

/*************************************************************************************
 *  Platform Memory Definition
 *************************************************************************************/
#if defined (__LINUX)
#define SIE_ENG_MALLOC(_size)                       vk_kmalloc(_size, GFP_KERNEL)
#define SIE_ENG_FREE(_p)                            vk_kfree(_p)
#else
#define SIE_ENG_MALLOC(_size)                       malloc(_size)
#define SIE_ENG_FREE(_p)                            free(_p)
#endif

/*************************************************************************************
 *  Public Function Prototype
 *************************************************************************************/
int  sie_eng_platform_request_irq(SIE_ENG_HANDLE *p_eng);
void sie_eng_platform_release_irq(SIE_ENG_HANDLE *p_eng);

int  sie_eng_platform_flg_set(SIE_ENG_HANDLE *p_eng, FLGPTN flg);
int  sie_eng_platform_flg_clear(SIE_ENG_HANDLE *p_eng, FLGPTN flg);
int  sie_eng_platform_flg_wait(SIE_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg);
int  sie_eng_platform_flg_wait_timeout(SIE_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timeout_ms);

int  sie_eng_platform_prepare_clk(SIE_ENG_HANDLE *p_eng);
int  sie_eng_platform_unprepare_clk(SIE_ENG_HANDLE *p_eng);

int  sie_eng_platform_enable_clk(SIE_ENG_HANDLE *p_eng);
int  sie_eng_platform_disable_clk(SIE_ENG_HANDLE *p_eng);
int  sie_eng_platform_enable_sensor_mclk(SIE_ENG_HANDLE *p_eng, SIE_ENG_MCLK_ID_T mclk_id);
int  sie_eng_platform_disable_sensor_mclk(SIE_ENG_HANDLE *p_eng, SIE_ENG_MCLK_ID_T mclk_id);
int  sie_eng_platform_enable_tsen_rxclk(SIE_ENG_HANDLE *p_eng);
int  sie_eng_platform_disable_tsen_rxclk(SIE_ENG_HANDLE *p_eng);

int  sie_eng_platform_set_clk_rate(SIE_ENG_HANDLE *p_eng, UINT32 clk_rate);
int  sie_eng_platform_set_clk_src(SIE_ENG_HANDLE *p_eng, SIE_ENG_CLKSRC clk_src);
int  sie_eng_platform_set_bcc_clk_src(SIE_ENG_HANDLE *p_eng, SIE_ENG_BCCCLKSRC bccclk_src);
int  sie_eng_platform_set_internal_clk_src(SIE_ENG_HANDLE *p_eng, SIE_ENG_INTCLKSRC intclk_src);
int  sie_eng_platform_set_pixel_clk_src(SIE_ENG_HANDLE *p_eng, SIE_ENG_PXCLKSRC clk_src);
int  sie_eng_platform_set_sensor_mclk_rate(SIE_ENG_HANDLE *p_eng, SIE_ENG_MCLK_ID_T mclk_id, UINT32 clk_rate);
int  sie_eng_platform_set_sensor_mclk_src(SIE_ENG_HANDLE *p_eng, SIE_ENG_MCLK_ID_T mclk_id, SIE_ENG_MCLKSRC clk_src);
int  sie_eng_platform_set_tsen_rxclk_src(SIE_ENG_HANDLE *p_eng, SIE_ENG_TSEN_RXCLKSRC clk_src);
int  sie_eng_platform_set_shared_sram_clk_src(SIE_ENG_HANDLE *p_eng, SIE_ENG_SHARED_SRAM_CLKSRC clk_src);

int  sie_eng_platform_disable_sram_shutdown(SIE_ENG_HANDLE *p_eng);
int  sie_eng_platform_enable_sram_shutdown(SIE_ENG_HANDLE *p_eng);

int  sie_eng_platform_disable_clk_auto_gating(SIE_ENG_HANDLE *p_eng);
int  sie_eng_platform_enable_clk_auto_gating(SIE_ENG_HANDLE *p_eng);
int  sie_eng_platform_get_clk_auto_gating(SIE_ENG_HANDLE *p_eng);

#endif /* _SIE_ENG_INT_PLATFORM_H_ */
