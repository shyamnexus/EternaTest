/**

    The header file of NvtIPC ioctl operations.

    @file       nvt_ipc_ioctl.h
    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#ifndef __NVT_IPC_IOCTL_H
#define __NVT_IPC_IOCTL_H

#include "kwrap/ioctl.h"

#define NVTIPC_IOC_MAGIC 'C'

#define NVTIPC_MSG_QUEUE_TOKEN_STR_MAXLEN    32  //  bytes
#define NVTIPC_MSG_ELEMENT_SIZE              32  ///<  queue element size


typedef struct {
    void*        addr;
    unsigned int size;
}NVTIPC_IOC_MEM_BUF,*PNVTIOC_IPC_MEM_BUF;


typedef struct {
    NVTIPC_KEY      key;                          ///< [in]
    NVTIPC_I32      rtn;                          ///< [out]
}NVTIPC_IOC_MSGQUE_GET_S,*PNVTIPC_IOC_MSGQUE_GET_S;

typedef struct {
    NVTIPC_U32      msqid;                        ///< [in]
    NVTIPC_I32      rtn;                          ///< [out]
}NVTIPC_IOC_MSGQUE_REL_S,*PNVTIPC_IOC_MSGQUE_REL_S;

typedef struct {
    char            path[NVTIPC_MSG_QUEUE_TOKEN_STR_MAXLEN+1]; ///< [in]
    NVTIPC_KEY      rtn;                                       ///< [out]
}NVTIPC_IOC_FTOK_S,*PNVTIPC_IOC_FTOK_S;


typedef struct {
    NVTIPC_U32      msqid;                         ///< [in]
    NVTIPC_SENDTO   send_to;                       ///< [in]
    char            msg[NVTIPC_MSG_ELEMENT_SIZE];  ///< [in]
    NVTIPC_U32      msgsz;                         ///< [in]
    NVTIPC_I32      rtn;                           ///< [out]
}NVTIPC_IOC_MSG_SND_S,*PNVTIPC_IOC_MSG_SND_S;

typedef struct {
	NVTIPC_U32      msqid;                        ///< [in]
	NVTIPC_U32      msgsz;                        ///< [in]
	NVTIPC_I32      timeout_ms;                   ///< [in]
	char            msg[NVTIPC_MSG_ELEMENT_SIZE];///< [out]
	NVTIPC_I32      rtn;                          ///< [out]
} NVTIPC_IOC_MSG_RCV_S, *PNVTIPC_IOC_MSG_RCV_S;


typedef struct {
	NVTIPC_I32      rtn;                          ///< [out]
} NVTIOC_IPC_SIG_WAIT_S, *PNVTIOC_IPC_SIG_WAIT_S;


typedef struct {
	NVTIPC_I32      sig;                          ///< [in]
	NVTIPC_I32      rtn;                          ///< [out]
} NVTIOC_IPC_SIG_ACK_S, *PNVTIOC_IPC_SIG_ACK_S;

typedef struct {
	NVTIPC_U32      core_id;                      ///< [in]
	NVTIPC_I32      timeout_ms;                   ///< [in]
	NVTIPC_I32      rtn;                          ///< [out]
} NVTIPC_IOC_WIAT_CORE_RDY_S, *PNVTIPC_IOC_WIAT_CORE_RDY_S;

#define NVTIOC_IPC_MSGQUE_GET     _VOS_IOWR(NVTIPC_IOC_MAGIC, 0, NVTIPC_IOC_MSGQUE_GET_S)
#define NVTIOC_IPC_MSGQUE_REL     _VOS_IOWR(NVTIPC_IOC_MAGIC, 1, NVTIPC_IOC_MSGQUE_REL_S)
#define NVTIOC_IPC_CONSOLE_CMD    _VOS_IOWR(NVTIPC_IOC_MAGIC, 2, NVTIPC_IOC_MEM_BUF)
#define NVTIOC_IPC_SYS_CMD        _VOS_IOWR(NVTIPC_IOC_MAGIC, 3, unsigned int)
#define NVTIOC_IPC_FTOK           _VOS_IOWR(NVTIPC_IOC_MAGIC, 4, NVTIPC_IOC_FTOK_S)
#define NVTIOC_IPC_MSG_SND        _VOS_IOWR(NVTIPC_IOC_MAGIC, 5, NVTIPC_IOC_MSG_SND_S)
#define NVTIOC_IPC_MSG_RCV        _VOS_IOWR(NVTIPC_IOC_MAGIC, 6, NVTIPC_IOC_MSG_RCV_S)
#define NVTIOC_IPC_SIG_WAIT       _VOS_IOWR(NVTIPC_IOC_MAGIC, 7, NVTIOC_IPC_SIG_WAIT_S)
#define NVTIOC_IPC_SIG_ACK        _VOS_IOWR(NVTIPC_IOC_MAGIC, 8, NVTIOC_IPC_SIG_ACK_S)
#define NVTIOC_IPC_INIT     	  _VOS_IOWR(NVTIPC_IOC_MAGIC, 9, unsigned int)
#define NVTIOC_IPC_EXIT     	  _VOS_IOWR(NVTIPC_IOC_MAGIC, 10, unsigned int)
#define NVTIOC_IPC_WIAT_CORE_RDY  _VOS_IOWR(NVTIPC_IOC_MAGIC, 11, NVTIPC_IOC_WIAT_CORE_RDY_S)

#if defined(__FREERTOS)
int nvt_ipc_ioctl(int fd, unsigned int cmd, void *p_arg);
#endif
#endif
