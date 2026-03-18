/*
    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.

    @file       kdrv_rpc_proc.h

    @brief      kdrv_rpc internal header file

    @version    V1.00.000
    @author     Novatek FW Team
    @date       2017/02/13
*/
#ifndef _KDRV_RPC_PROC_H
#define _KDRV_RPC_PROC_H

#include "kdrv_rpc_main.h"
extern int  kdrv_rpc_proc_init(PKDRV_RPC_DRV_INFO pdrv_info);
extern void kdrv_rpc_proc_exit(PKDRV_RPC_DRV_INFO pdrv_info);
#endif