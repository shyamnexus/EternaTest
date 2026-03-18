#ifndef _VPE_DRV_CGAIN_INT_H_
#define _VPE_DRV_CGAIN_INT_H_
#include "vpe_drv_ll_int.h"
#include "vpe_drv_ctl.h"
#include "vpe_eng.h"

int vpe_drv_cgain_proc(VPE_ENG_HANDLE *p_eng_hdl, struct vpe_drv_job_cfg *job_cfg);
void vpe_drv_cgain_dump_info(struct vpe_drv_color_ofs *cgain);
#endif //_VPE_DRV_CGAIN_INT_H_