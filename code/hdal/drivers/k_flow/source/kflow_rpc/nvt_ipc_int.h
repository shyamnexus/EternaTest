/**

    The internal header file of nvt_ipc driver.

    @file       NvtIpcInt.h
    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#ifndef _NVTIPC_INT_H
#define _NVTIPC_INT_H

#if defined __KERNEL__
#include <linux/io.h>
#include <linux/string.h>
#else
#include <stdio.h>
#include <string.h>
#endif
#include <kwrap/type.h>
#include <kwrap/semaphore.h>
#include <kflow_rpc/nvt_ipc.h>


#define __CORE1  1
#define __CORE2  2
#define __CORE3  3
#define __CORE   __CORE1

#define __COREIPC__                   1

#define __SYSCMD_WAIT_FINISH__        1

#define __SEMCMD_WAIT_ACK__           0

#define __IPC_PROF_MEASURE__          1

#define __IPC_SEND_BUF_CACHEABLE__    0

#define UNIT_TEST_IPC                 0

#define __IPC_SIG_SUPPORT__           0

// flag define

// reserved 16 bits
#define FLG_NVTIPC_QUE_ID1                FLGPTN_BIT(0)
#define FLG_NVTIPC_QUE_ID2                FLGPTN_BIT(1)
#define FLG_NVTIPC_QUE_ID3                FLGPTN_BIT(2)
#define FLG_NVTIPC_QUE_ID4                FLGPTN_BIT(3)
#define FLG_NVTIPC_QUE_ID5                FLGPTN_BIT(4)
#define FLG_NVTIPC_QUE_ID6                FLGPTN_BIT(5)
#define FLG_NVTIPC_QUE_ID7                FLGPTN_BIT(6)
#define FLG_NVTIPC_QUE_ID8                FLGPTN_BIT(7)
///...
#define FLG_NVTIPC_QUE_ID16               FLGPTN_BIT(15)

#define FLG_NVTIPC_ERR_PRINT              FLGPTN_BIT(20)
#define FLG_NVTIPC_SYSCALL_ACK            FLGPTN_BIT(21)
#define FLG_NVTIPC_UART_ACK               FLGPTN_BIT(22)
#define FLG_NVTIPC_PERF_ACK               FLGPTN_BIT(23)
#define FLG_NVTIPC_GET_LONG_COUNTER_ACK   FLGPTN_BIT(24)
#define FLG_NVTIPC_DSP_READY              FLGPTN_BIT(25)

#define FLG_NVTIPC_RCV_EXIT               FLGPTN_BIT(28)
#define FLG_NVTIPC_RCV_EXIT_DONE          FLGPTN_BIT(29)
#define FLG_NVTIPC_EXIT_DONE              FLGPTN_BIT(30)
#define FLG_NVTIPC_EXIT                   FLGPTN_BIT(31)


#define SYSCALL_MAX_WAIT_TIME            30000  // mini-seconds
#define UART_MAX_WAIT_TIME               60000  // mini-seconds
#define PERF_MAX_WAIT_TIME               10000  // mini-seconds
#define LONG_COUNTER_MAX_WAIT_TIME       10000  // mini-seconds
#define IPC_WAIT_TIMEOUT_CNT             100000000   // cpu cycle -> 10sec
#define IPC_LOCK_SEMBITS_TIMEOUT_CNT     100         // cpu cycle



//#define NVTIPC_MSG_QUEUE_NUM             16
//#define NVTIPC_MSG_ELEMENT_NUM           16
//#define NVTIPC_MSG_ELEMENT_SIZE          24 //  bytes



#define NVTIPC_SYS_CMD_MAX_LEN               256 //  bytes
#define NVTIPC_MSG_QUEUE_TOKEN_STR_MAXLEN    32  //  bytes



#define NVTIPC_SYS_KEY             (0x0000FFFF)
#define IPC_SEM_QUEUE_TOKEN_STR    "ipcsem"
#define NVTIPC_SYS_QUEUE_ID        (0)
#define NVTIPC_SEM_QUEUE_ID        (1)

#define CFG_NVTIPC_INIT_KEY         MAKEFOURCC('N', 'I', 'P', 'C')


struct nvt_ipc_device {
	UINT32             req_queue_bits;
};


extern ULONG nvt_ipc_get_phy_addr(ULONG noncache_addr);

#define ipc_get_phyaddr(addr)       nvt_ipc_get_phy_addr(addr)
#define ipc_get_non_cacheaddr(addr) nvt_ipc_get_noncache_addr(addr)
#define ipc_store_cache(pbuf, len)  { void *ptr; for (ptr = pbuf; ptr < (pbuf + len+PAGE_SIZE); ptr += PAGE_SIZE) { flush_dcache_page(pfn_to_page(PFN_DOWN(virt_to_phys(ptr)))); } }
#define TIMER0_COUNTER_REG         (NVT_PERIPHERAL_VIRT_BASE+0x00040108)
#define GET_NON_CACHE_ADDR(addr)    (((addr) & 0x5FFFFFFF)|0x60000000)




extern void       kflow_nvt_ipc_syscmd_ack(NVTIPC_SYS_CMD   syscmd);
extern NVTIPC_I32 kflow_nvt_ipc_msg_snd_p(NVTIPC_U32 msqid, NVTIPC_SENDTO send_to, void  *p_msg, NVTIPC_U32 msgsz, NVTIPC_U32 ioctl);
extern void nvt_ipc_dummy_read(void);


#endif //_NVTIPC_INT_H
