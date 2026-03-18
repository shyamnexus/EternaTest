/**
    UCDC, Task running operation declare

    @file       ucdc_run.h
    @ingroup    mUCDC

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/
#ifndef _UCDCRUN_H
#define _UCDCRUN_H

#include "ucdc_int.h"

UCDC_ER       xUCDCRun_Open(void);
UCDC_ER       xUCDCRun_Close(void);
UCDC_ER       xUCDCRun_Suspend(void);
UCDC_ER       xUCDCRun_Resume(void);
UCDC_ER       xUCDCRun_Cmd(const UCDC_CMD *pCmd);

#endif
