#ifndef _VPE_DRV_DCE_INT_H_
#define _VPE_DRV_DCE_INT_H_
#include "vpe_drv_ll_int.h"
#include "vpe_drv_util_int.h"
#include "vpe_drv_ctl_int.h"
#include "vpe_eng.h"

#define DCE_2DLUT_MAX 1024
#define DCE_2DLUT_MIN 2

#define DCE_2DLUT_ROT_X_MAX 2
#define DCE_2DLUT_ROT_Y_MAX 2
#define DCE_2DLUT_ROT_W_ALIGN VPE_DRV_ALIGN_ROUNDUP(DCE_2DLUT_ROT_X_MAX, 4)
#define DCE_2DLUT_TAB_TOTAL_NUM (DCE_2DLUT_ROT_W_ALIGN * DCE_2DLUT_ROT_Y_MAX)
#define DCE_2DLUT_BUF_SIZE (DCE_2DLUT_TAB_TOTAL_NUM << 2) 				    	//real buffer size
#if (COHERENT_CACHE_BUF == 1)
#define DCE_2DLUT_ALLOC_SIZE VPE_DRV_ALIGN_ROUNDUP(DCE_2DLUT_BUF_SIZE, 64)  	//for  buffer allocation used to meet address align 64
#else
#define DCE_2DLUT_ALLOC_SIZE VPE_DRV_ALIGN_ROUNDUP(DCE_2DLUT_BUF_SIZE, 16)  	//for  buffer allocation used to meet address align 16
#endif

int vpe_drv_dce_set_col(VPE_ENG_HANDLE *p_eng_hdl, struct vpe_drv_job_cfg *job_cfg, VPE_DRV_CFG *out_cfg, int sw_col_idx);
int vpe_drv_dce_proc(VPE_ENG_HANDLE *p_eng_hdl, struct vpe_drv_job_cfg *job_cfg, struct vpe_drv_lut2d_info *lut2d);
void vpe_drv_dce_dump_info(struct vpe_drv_dce_param *dce);
#endif //_VPE_DRV_TMNR_INT_H_
