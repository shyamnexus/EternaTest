#ifndef _VPE_COMMON_PARAM_INT_H_
#define _VPE_COMMON_PARAM_INT_H_

#ifdef __KERNEL__
#include "kwrap/type.h"
#endif
#include "ctl_vpe_isp.h"


/////////////////////////////////////////
// extern variable
/////////////////////////////////////////
extern CTL_VPE_ISP_DCE_CTL ctl_vpe_dce_ctl_init;
extern CTL_VPE_ISP_SHARPEN_PARAM ctl_vpe_sharpen_init;
extern CTL_VPE_ISP_DCE_2DLUT_PARAM ctl_vpe_2dlut_init;
extern CTL_VPE_ISP_YUV_CVT_PARAM ctl_vpe_cvt_init;
extern CTL_VPE_ISP_DCTG_CTL ctl_vpe_dctg_init;
extern CTL_VPE_ISP_FLIP_ROT_CTL ctl_vpe_flip_rot_init;

#endif

