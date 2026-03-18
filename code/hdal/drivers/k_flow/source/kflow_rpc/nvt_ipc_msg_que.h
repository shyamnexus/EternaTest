/**

    The header file of NvtIPC message queues operations,...etc

    @file       NvtIpcMsgQue.h
    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#ifndef NVTIPC_MSGQUE_H
#define NVTIPC_MSGQUE_H

#include "nvt_ipc_common.h"
#include "nvt_ipc_int.h"


typedef struct _NVTIPC_MSGQUE {
	UINT32            init_key;
	UINT32            shared_key;
	UINT32            msq_id;      // msg queue id
	UINT32            in_idx;
	UINT32            out_idx;
	NVTIPC_MSG        element[NVTIPC_MSG_ELEMENT_NUM];
#if __IPC_PROF_MEASURE__
	UINT64            prof_start_time;
	UINT32            prof_msg_count;
	UINT32            prof_max_msg_count;
#endif
} NVTIPC_MSGQUE;

typedef struct _NVTIPC_MSGBUF {
	UINT32            pingpon_idx;
	NVTIPC_SEND_MSG  *sndmsg[2];
	SEM_HANDLE        semid;      // semaphore id
} NVTIPC_MSGBUF;





typedef struct _NVTIPC_MSQINFO {
	char              *syscmd_buf;
	char              *syscmd_phyaddr;
	NVTIPC_MSGQUE      msq_tbl[NVTIPC_MSG_QUEUE_NUM];
	char               msq_tokenstr[NVTIPC_MSG_QUEUE_NUM][NVTIPC_MSG_QUEUE_TOKEN_STR_MAXLEN + 1];
	NVTIPC_SHM_CTRL   *shm_ctrl;
	SEM_HANDLE         cores_semid[NVTIPC_SEM_NUM];      // semaphore id
	SEM_HANDLE         local_semid[NVTIPC_SEM_NUM];      // semaphore id
	NVTIPC_MSGBUF      msgbuf_tbl[NVTIPC_SENDTO_MAX];
} NVTIPC_MSQINFO;


typedef struct _NVTIPC_CPU2_READY_MSG {
	NVTIPC_SYS_CMD     sys_cmd_id;           ///< system command ID.
} NVTIPC_CPU2_READY_MSG;

typedef struct _NVTIPC_SEM_INIT_MSG {
	NVTIPC_SEM_CMD     cmd_id;              ///< command ID.
	ULONG              sem_ctrl;            ///< semaphore control pointer
} NVTIPC_SEM_INIT_MSG;



extern INT32            nvt_ipc_msg_que_init(ULONG shm_addr);
extern void             nvt_ipc_msg_que_exit(void);
extern NVTIPC_SEND_MSG *nvt_ipc_msg_que_lock_sndmsg_buf(NVTIPC_SENDTO send_to);
extern void             nvt_ipc_msg_que_unlock_sndmsg_buf(NVTIPC_SENDTO send_to);
extern char            *nvt_ipc_msg_que_get_syscmd_buf(void);
extern NVTIPC_SHM_CTRL *nvt_ipc_msg_que_get_shmctrl(void);
extern void             nvt_ipc_msg_que_shmctrl_lock_cores(UINT32 semid);
extern void             nvt_ipc_msg_que_shmctrl_unlock_cores(UINT32 semid);
extern void             nvt_ipc_msg_que_shmctrl_lock_local(UINT32 semid);
extern void             nvt_ipc_msg_que_shmctrl_unlock_local(UINT32 semid);
extern NVTIPC_MSGQUE   *nvt_ipc_msg_que_id2que(UINT32 msqid);
extern INT32            nvt_ipc_msg_que_key2id(INT32 key);
extern INT32            nvt_ipc_msg_que_get(INT32 key);
extern INT32            nvt_ipc_msg_que_rel(UINT32 msqid);
extern void             nvt_ipc_msg_que_dequeue(NVTIPC_MSGQUE *p_msgque, NVTIPC_MSG *p_msg);
extern UINT32           nvt_ipc_msg_que_isempty(NVTIPC_MSGQUE *p_msgque);
extern UINT32           nvt_ipc_msg_que_isvalid(NVTIPC_MSGQUE *p_msgque, UINT32 sharedkey);
extern INT32            nvt_ipc_msg_que_post(UINT32 msqid, void  *p_msg, UINT32 msgsz);
extern UINT16           nvt_ipc_msg_que_ftok(const char *p_name);
extern char            *nvt_ipc_msg_que_key2token(UINT32 key);
extern void             nvt_ipc_msg_que_dump_debuginfo(void);
extern ULONG            nvt_ipc_msg_que_get_phyaddr(ULONG virtual_addr);
extern void             nvt_ipc_msg_que_chk_allrel(void);

#endif
