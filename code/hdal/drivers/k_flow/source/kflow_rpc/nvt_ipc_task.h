/**

    The header file of nvt_ipc tasks APIs.

    @file       nvt_ipc_task.h
    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#ifndef NVT_IPC_TASK_H
#define NVT_IPC_TASK_H
#include <kwrap/nvt_type.h>
#include <kflow_rpc/nvt_ipc.h>



extern int  nvt_ipc_task_init(void);
extern void nvt_ipc_task_exit(void);
extern void nvt_ipc_task_set_errcode(INT32 err, UINT32 cmd);
#endif
