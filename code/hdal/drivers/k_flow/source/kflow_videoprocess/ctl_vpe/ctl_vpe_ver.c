#ifndef _KFLOW_CTL_VPE_VERINFO_H_
#define _KFLOW_CTL_VPE_VERINFO_H_
#include "kwrap/verinfo.h"
#include "ctl_vpe_ver.h"

#if defined(_GROUP_KO_)
#undef __init
#undef __exit
#undef module_init
#undef module_exit
#define __init
#define __exit
#define module_init(x)
#define module_exit(x)
#ifndef MODULE // in built-in case
#undef MODULE_VERSION
#undef VOS_MODULE_VERSION
#define MODULE_VERSION(x)
#define VOS_MODULE_VERSION(a1, a2, a3, a4, a5)
#endif
#endif

VOS_MODULE_VERSION(CTL_VPE_DRV_NAME, CTL_VPE_VER_MAJOR, CTL_VPE_VER_MINOR, CTL_VPE_VER_PATCH, CTL_VPE_VER_EXT);
#endif
