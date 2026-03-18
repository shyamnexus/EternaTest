#ifndef _MD_DRV_MOTION_DET_INT_H_
#define _MD_DRV_MOTION_DET_INT_H_

#include "kdrv_md_ctl.h"
#include "md_eng.h"

int md_drv_motion_det_proc(MD_ENG_HANDLE *p_eng_hdl, struct md_drv_job_cfg *job_cfg);
void md_drv_motion_dump_job_info(struct md_drv_job_cfg *job_cfg);
void md_drv_motion_dump_mdbc_info(struct md_drv_job_cfg *job_cfg);
#endif //_MD_DRV_MOTION_DET_INT_H_
