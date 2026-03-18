/*
    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.

    @file       kdrv_rpc_debug.h

    @brief      kdrv_rpc debug api header file

    @version    V1.00.000
    @author     Novatek FW Team
    @date       2017/02/13
*/
#ifndef _KDRV_RPC_DEBUG_H
#define _KDRV_RPC_DEBUG_H

#define THIS_DBGLVL         NVT_DBG_WRN
#define __MODULE__          kdrv_rpc
#define __DBGLVL__          8
#define __DBGFLT__          "*" // *=All, [mark]=CustomClass
#if defined (__UITRON)
#include "DebugModule.h"
#else
#include <kwrap/debug.h>
#endif
extern unsigned int kdrv_rpc_debug_level;
#endif

