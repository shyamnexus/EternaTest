#ifndef _VPE_DRV_SCA_INT_H_
#define _VPE_DRV_SCA_INT_H_
#include "vpe_drv_ll_int.h"
#include "vpe_drv_ctl.h"
#include "vpe_eng.h"

int vpe_drv_sca_proc(VPE_ENG_HANDLE *p_eng_hdl, struct vpe_drv_job_cfg *job_cfg, VPE_DRV_CFG *out_cfg);
int vpe_drv_sca_set_col(VPE_ENG_HANDLE *p_eng_hdl, struct vpe_drv_job_cfg *job_cfg, VPE_DRV_CFG *out_cfg, int sw_col_idx);
int vpe_drv_sca_chk_roi(struct vpe_drv_in_info *src, struct vpe_drv_out_info *dst, int dst_max_num, struct vpe_drv_dce_param *dce_param, struct vpe_drv_dctg_param *dctg_param);
void vpe_drv_sca_dump_src_info(struct vpe_drv_in_info *in);
void vpe_drv_sca_dump_dst_info(struct vpe_drv_out_info *out, struct vpe_drv_dce_param *dce_param);
void vpe_drv_sca_set_pfet_flag(unsigned int flag);
unsigned int vpe_drv_sca_get_pfet_flag(void);
#endif //_VPE_DRV_SCA_INT_H_
