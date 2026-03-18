/**
    Public header file for IPL version info

    This file is the header file that define the API and data type for IPL version info.

    @file       ctl_ipp_verinfo.h
    @ingroup    mILibIPLCom
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2024.  All rights reserved.
*/
#ifndef _CTL_IPP_VERINFO_H_
#define _CTL_IPP_VERINFO_H_

#include "kwrap/type.h"

// special checking for hdal and ipp version matched. add item here & ctl_ipp_ver_chk()
#define CTL_IPP_VER_CHK_CAP_NO_3DNR_REF 0

typedef struct {
} CTL_IPP_VER_CHK_INFO;

BOOL ctl_ipp_ver_chk(UINT32 item, CTL_IPP_VER_CHK_INFO *info);

#endif //_CTL_IPP_VERINFO_H_