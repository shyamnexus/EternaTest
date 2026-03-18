#ifndef _NMEDIAVDODECPLATFORM_H
#define _NMEDIAVDODECPLATFORM_H
#include <kwrap/type.h>

extern void NMP_VdoDec_Lock_cpu(unsigned long *in_flags);
extern void NMP_VdoDec_Unlock_cpu(unsigned long *in_flags);
extern void nmp_vdodec_delay_m_sec(UINT32 m_sec);
extern uintptr_t nmp_vdodec_va2pa(uintptr_t va);
#endif


