/*
    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.

    @file       nvt_ipc_proc.h

    @brief      nvt_ipc internal header file

    @version    V1.00.000
    @author     Novatek FW Team
    @date       2017/02/13
*/
#ifndef _NVT_IPC_PROC_H
#define _NVT_IPC_PROC_H
#include "nvt_ipc_main.h"

extern int  nvt_ipc_proc_init(PNVT_IPC_DRV_INFO pdrv_info);
extern void nvt_ipc_proc_exit(PNVT_IPC_DRV_INFO pdrv_info);
#endif