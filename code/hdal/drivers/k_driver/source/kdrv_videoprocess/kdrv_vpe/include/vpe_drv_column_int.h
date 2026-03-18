#ifndef _VPE_DRV_COLUMN_INT_H_
#define _VPE_DRV_COLUMN_INT_H_
#include "vpe_drv_ctl.h"
#include "vpe_eng.h"

#if (VPE_ENG_RES_MAX != VPE_DRV_OUT_MAX)
#error "path size misatch"
#endif

int vpe_drv_column_proc(VPE_ENG_HANDLE *p_eng_hdl, struct vpe_drv_job_cfg *job_cfg, VPE_DRV_CFG *out_cfg, int expect_eng_col_num);
void vpe_drv_set_col_dbg(unsigned char dbg_en);
int vpe_drv_ctl_chk_col(struct vpe_drv_job_cfg *job_cfg, int expect_eng_col_num);
#endif //_VPE_DRV_COLUMN_INT_H_
