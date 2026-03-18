#ifndef _KFLOW_CTL_IPP_VERINFO_H_
#define _KFLOW_CTL_IPP_VERINFO_H_
#include "kwrap/verinfo.h"
#include "ctl_ipp_verinfo.h"
#include "ctl_ipp_dbg.h"

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

VOS_MODULE_VERSION(kflow_ctl_ipp, 5, 05, 086, 00);

BOOL ctl_ipp_ver_chk(UINT32 item, CTL_IPP_VER_CHK_INFO *info)
{
	switch (item) {
	case CTL_IPP_VER_CHK_CAP_NO_3DNR_REF:
		return TRUE;
	default:
		CTL_IPP_DBG_TRC_PART(CTL_IPP_DBG_TRC_VER, "ver chk fail (%u)\r\n", item);
		return FALSE;
	}
}

#endif
