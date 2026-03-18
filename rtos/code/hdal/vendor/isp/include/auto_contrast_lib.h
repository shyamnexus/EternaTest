
#ifndef __AUTOCONTRASTLIB_H__
#define __AUTOCONTRASTLIB_H__

#include "hd_type.h"
#include "vendor_isp.h"

#define E_OK                   0      //success
#define E_GET_DEV_FAIL        -1      //No ISP device
#define E_GET_EV_FAIL         -2      //Get EV value fail
#define E_NOT_READY           -3      //AE not nready for get EV value
#define E_SYS                 -4      //System fail
#define E_FAIL               -99      //Other fail

extern INT auto_contrast_cal(ISPT_HISTO_DATA *histo_data, IQT_YCURVE_PARAM *ycurve, IQT_GAMMA_PARAM *gamma, AET_STATUS_INFO *ae_status);
extern void auto_contrast_set_str(UINT32 str);
extern void auto_contrast_set_dbg_out(BOOL on);

#endif // __AUTOCONTRASTLIB_H__
