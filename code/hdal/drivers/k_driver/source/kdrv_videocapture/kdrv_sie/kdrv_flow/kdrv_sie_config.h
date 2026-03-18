/*
    SIE KDRV configuration header file.

    SIE KDRV configuration header file. Define semaphore ID, flag ID, etc.

    @file       kdrv_sie_config.h
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/

#ifndef _KDRV_SIE_CONFIG_H
#define _KDRV_SIE_CONFIG_H

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
#include "kwrap/flag.h"
#include "kdrv_sie_int.h"
#include "kdrv_sie_dbg.h"

/**
    Semphore Configuration Infomation
*/
typedef struct {
	ID  	semphore_id;				///< Semaphore ID
	UINT32	max_counter;
} KDRV_SIE_SEM_TABLE;

extern KDRV_SIE_SEM_TABLE kdrv_sie_semtbl[KDRV_SIE_MAX_ENG];
void kdrv_sie_install_id(void);
void kdrv_sie_uninstall_id(void);
ID *kdrv_sie_get_sem_id(KDRV_SIE_PROC_ID id);
ID *kdrv_sie_get_flag_id(KDRV_SIE_PROC_ID id);

#endif
