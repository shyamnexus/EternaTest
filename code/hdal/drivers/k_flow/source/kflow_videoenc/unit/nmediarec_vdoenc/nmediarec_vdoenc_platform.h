#ifndef _NMEDIAVDOENCPLATFORM_H
#define _NMEDIAVDOENCPLATFORM_H
#include <kwrap/type.h>
#if defined(__FREERTOS)
#include <stddef.h>
#endif

extern void NMR_VdoEnc_Lock_cpu(unsigned long *in_flags);
extern void NMR_VdoEnc_Unlock_cpu(unsigned long *in_flags);
extern void NMR_VdoEnc_Lock_cpu2(unsigned long *in_flags);
extern void NMR_VdoEnc_Unlock_cpu2(unsigned long *in_flags);
extern void* nmr_vdoenc_alloc(int size);
extern void nmr_vdoenc_free(void *buf);
extern void nmr_vdoenc_delay_m_sec(UINT32 m_sec);
extern uintptr_t nmr_vdoenc_va2pa(uintptr_t va);
extern void *nmr_vdoenc_ioremap_nocache_wrap(phys_addr_t offset, size_t size);
extern void nmr_vdoenc_iounmap_wrap(void *addr);
#endif


