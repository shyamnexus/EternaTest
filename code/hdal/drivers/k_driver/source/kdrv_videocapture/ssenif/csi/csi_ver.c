#include <kwrap/verinfo.h>
#include "csi_int.h"

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

VOS_MODULE_VERSION(CSI_DRV_NAME, CSI_VER_MAJOR, CSI_VER_MINOR, CSI_VER_PATCH, CSI_VER_EXT);
