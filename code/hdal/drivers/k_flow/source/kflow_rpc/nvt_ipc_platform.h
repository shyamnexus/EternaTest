#ifndef _NVT_IPC_PLATFORM_H
#define _NVT_IPC_PLATFORM_H

extern void *nvt_ipc_vmalloc(unsigned long size);
extern void nvt_ipc_vfree(void *v_buff);
extern unsigned long nvt_ipc_copy_from_user(void *to, const void *from, unsigned long n);
extern unsigned long nvt_ipc_copy_to_user(void *to, const void *from, unsigned long n);
#endif
