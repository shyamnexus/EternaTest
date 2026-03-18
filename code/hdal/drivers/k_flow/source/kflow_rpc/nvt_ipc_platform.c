#if defined __KERNEL__
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/dma-mapping.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif
#include "kflow_rpc/nvt_ipc.h"
#include "nvt_ipc_platform.h"


void *nvt_ipc_vmalloc(unsigned long size)
{
#if defined(__KERNEL__)
	return vmalloc(size);
#else
    return malloc(size);
#endif
}

void nvt_ipc_vfree(void *v_buff)
{
#if defined(__KERNEL__)
	vfree(v_buff);
#else
    free(v_buff);
#endif
}


unsigned long nvt_ipc_copy_from_user(void *to, const void *from, unsigned long n)
{
#if defined(__KERNEL__)
	return copy_from_user(to, from, n);
#else
	return 0;
#endif
}

unsigned long nvt_ipc_copy_to_user(void *to, const void *from, unsigned long n)
{
#if defined(__KERNEL__)
	return copy_to_user(to, from, n);
#else
	return 0;
#endif
}

#if defined(__KERNEL__)
EXPORT_SYMBOL(kflow_nvt_ipc_ftok);
EXPORT_SYMBOL(kflow_nvt_ipc_msg_get);
EXPORT_SYMBOL(kflow_nvt_ipc_msg_rel);
EXPORT_SYMBOL(kflow_nvt_ipc_msg_snd);
EXPORT_SYMBOL(kflow_nvt_ipc_msg_rcv);
EXPORT_SYMBOL(kflow_nvt_ipc_get_shm_addr);
EXPORT_SYMBOL(kflow_nvt_ipc_set_dsp_log_addr);
#endif

