/*
    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.

    @file       kdrv_rpc_queue.h

    @brief      kdrv_rpc internal header file

    @version    V1.00.000
    @author     Novatek FW Team
    @date       2017/02/13
*/
#ifndef _KDRV_RPC_QUEUE_H
#define _KDRV_RPC_QUEUE_H

#define KDRV_RPC_QUEUE_ELEMENT_NUM   8

typedef struct _KDRV_RPC_QUEUE_ELEMENT {
	UINT32                   cmd_buf[3];     ///< command buffer
	KDRV_CALLBACK_FUNC      *p_cb_func;
	VOID                    *p_user_data;
	UINT32                   flg_ptn;
} KDRV_RPC_QUEUE_ELEMENT;


typedef struct _KDRV_RPC_QUEUE_INFO {
	UINT32                   in_idx;
	UINT32                   out_idx;
	KDRV_RPC_QUEUE_ELEMENT   element[KDRV_RPC_QUEUE_ELEMENT_NUM];
} KDRV_RPC_QUEUE_INFO;

extern INT32 kdrv_rpc_queue_init_p(void);
extern INT32 kdrv_rpc_queue_exit_p(void);
extern KDRV_RPC_QUEUE_INFO *kdrv_rpc_get_queue_by_coreid(KDRV_RPC_CORE_ID  core_id);
extern INT32 kdrv_rpc_queue_is_empty_p(KDRV_RPC_QUEUE_INFO *p_queue);
extern INT32 kdrv_rpc_queue_add_p(KDRV_RPC_QUEUE_INFO *p_queue, KDRV_RPC_QUEUE_ELEMENT *element);
extern INT32 kdrv_rpc_queue_del_p(KDRV_RPC_QUEUE_INFO *p_queue, KDRV_RPC_QUEUE_ELEMENT *element);
#endif

