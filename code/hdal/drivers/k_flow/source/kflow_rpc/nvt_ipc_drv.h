/**

    The header file of nvt_ipc tasks APIs.

    @file       nvt_ipc_task.h
    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#ifndef NVT_IPC_DRV_H
#define NVT_IPC_DRV_H

#include <kwrap/mem.h>
#include "nvt_ipc_int.h"
#include "nvt_ipc_sig.h"

typedef struct _NVT_IPC_MODULE_INFO {
	ID                flg_id;
	VOS_MEM_HDL       mem_hdl;
	ULONG             shm_phy_base;       ///< share memory physical address
	ULONG             shm_virt_base;      ///< share memory non-cache address
	unsigned int      shm_size;           ///< share memory size
	#if __IPC_SIG_SUPPORT__
	NVTIPC_SIG_CTRL   sig_ctrl;
	#endif
} NVT_IPC_MODULE_INFO, *PNVT_IPC_MODULE_INFO;

int                   nvt_ipc_drv_init(PNVT_IPC_MODULE_INFO pmodule_info);
void                  nvt_ipc_drv_exit(PNVT_IPC_MODULE_INFO pmodule_info);
int                   nvt_ipc_drv_resume(void);
extern PNVT_IPC_MODULE_INFO  (*nvt_ipc_drv_get_module_info)(void);
void                  nvt_ipc_drv_dump_debug(void);

#endif
