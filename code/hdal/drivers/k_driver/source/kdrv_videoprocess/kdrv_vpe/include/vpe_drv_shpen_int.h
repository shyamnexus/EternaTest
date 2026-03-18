#ifndef _VPE_DRV_SHPEN_INT_H_
#define _VPE_DRV_SHPEN_INT_H_
#include "vpe_drv_ll_int.h"
#include "vpe_drv_ctl.h"
#include "vpe_eng.h"

#if (KDRV_VPE_DBS_GAIN_NUMS != VPE_DRV_SHARPEN_DBS_GAIN_NUMS)
#error "DBS size misatch"
#endif

int vpe_drv_shpen_proc(VPE_ENG_HANDLE *p_eng_hdl, struct vpe_drv_job_cfg *job_cfg);
void vpe_drv_shpen_dump_info(struct vpe_drv_shpn_info *shp);
#endif //_VPE_DRV_SHPEN_INT_H_
