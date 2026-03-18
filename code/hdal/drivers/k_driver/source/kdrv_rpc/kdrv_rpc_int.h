/*
    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.

    @file       kdrv_rpc_int.h

    @brief      kdrv_rpc internal header file

    @version    V1.00.000
    @author     Novatek FW Team
    @date       2017/02/13
*/
#ifndef _KDRV_RPC_INT_H
#define _KDRV_RPC_INT_H

#ifdef __KERNEL__
#include <linux/io.h>
#else
#include <stdio.h>
#include <string.h>
#endif
#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kwrap/sxcmd.h"
#include "kwrap/stdio.h"
#include "kwrap/flag.h"
#include "kwrap/task.h"
#include "kwrap/tasklet.h"
#include "cc.h"
#include "kdrv_type.h"
#include "kdrv_rpc.h"
#include "kdrv_rpc_queue.h"
#include "kdrv_rpc_debug.h"

typedef CC_ER (*CC_SEND_FUNC)(PCC_CMD p_cmd);

typedef struct _KDRV_RPC_TRIG_INFO {
	KDRV_CALLBACK_FUNC     *cb;
	VOID                   *user_data;
	UINT32                  flg_ptn;
	BOOL                    is_busy;
	KDRV_RPC_QUEUE_INFO    *p_queue;
	CC_SEND_FUNC            cc_send_cmd;
} KDRV_RPC_TRIG_INFO;

#endif

