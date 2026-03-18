/*
    VIE KDRV configuration header file.

    VIE KDRV configuration header file. Define semaphore ID, flag ID, etc.

    @file       kdrv_vie_config.h
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/

#ifndef _KDRV_VIE_CONFIG_H
#define _KDRV_VIE_CONFIG_H

#if defined(__LINUX)
#include <linux/string.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#else
#include "pll_protected.h"
#include "pll.h"
#endif
#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include <kwrap/flag.h>
#include "kdrv_vie_int.h"
#include "vie_dbg.h"


/**
    Semphore Configuration Infomation
*/
typedef struct {
	ID  	semphore_id;				///< Semaphore ID
	UINT32	max_counter;
} KDRV_VIE_SEM_TABLE;

extern KDRV_VIE_SEM_TABLE kdrv_vie_semtbl[KDRV_VIE_MAX_ENG * KDRV_VIE_MAX_VDO_CH];
void kdrv_vie_install_id(void);
void kdrv_vie_uninstall_id(void);
SEM_HANDLE *kdrv_vie_get_sem_id(KDRV_VIE_PROC_ID id);

#endif
