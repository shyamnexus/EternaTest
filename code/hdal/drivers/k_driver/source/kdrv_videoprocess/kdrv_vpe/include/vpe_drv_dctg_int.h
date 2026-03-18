#ifndef _VPE_DRV_DCTG_INT_H_
#define _VPE_DRV_DCTG_INT_H_
#include "vpe_drv_ll_int.h"
#include "vpe_drv_ctl.h"
#include "vpe_eng.h"

#define DCTG_2DLUT_MAX 512

int vpe_drv_dctg_proc(VPE_ENG_HANDLE *p_eng_hdl, struct vpe_drv_job_cfg *job_cfg);
void vpe_drv_dctg_dump_info(struct vpe_drv_dctg_param *dctg);
#endif //_VPE_DRV_TMNR_INT_H_
