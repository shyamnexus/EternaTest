/*
    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.

    @file       nvt_ipc_debug.h

    @version    V1.00.000
    @author     Novatek FW Team
    @date       2017/02/13
*/
#ifndef _NVT_IPC_DEBUG_H
#define _NVT_IPC_DEBUG_H

#define THIS_DBGLVL         NVT_DBG_WRN
#define __MODULE__          nvt_ipc
#define __DBGLVL__          8
#define __DBGFLT__          "*" // *=All, [mark]=CustomClass
#if defined (__UITRON)
#include "DebugModule.h"
#else
#include "kwrap/debug.h"
#endif
extern unsigned int nvt_ipc_debug_level;
#endif

