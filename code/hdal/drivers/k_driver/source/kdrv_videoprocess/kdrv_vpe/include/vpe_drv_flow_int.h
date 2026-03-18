#ifndef _VPE_DRV_FLOW_INT_H_
#define _VPE_DRV_FLOW_INT_H_

#include "vpe_drv_ctl_int.h"
#include "vpe_drv_ll_int.h"
#include "vpe_eng.h"

#define VPE_MAX_LL_CMD_NUM (625 + (300 * 3)) // max 4 sw_col, 625 for 1st sw_col, and 300 for 2nd~4th sw_col

int vpe_drv_flow_proc(u16 chip_id, u16 eng_id, VPE_ENG_HANDLE *eng_hdl, struct vpe_drv_ctl_eng_col_info *eng_col,
                    struct vpe_drv_job_cfg *job_cfg, struct vpe_drv_ll_blk *proc_ll_blk, VPE_DRV_CFG *col_cfg, struct vpe_drv_lut2d_info *lut2d);
int vpe_drv_flow_isr_callback(VPE_ENG_HANDLE *eng_hdl, struct vpe_drv_job_cfg *job_cfg, int ll_idx, int sub_job_num);
int vpe_drv_flow_dump_job_cfg(struct vpe_drv_job_cfg *job_cfg, int dump_flag);
unsigned int vpe_drv_flow_predict_bscore(struct vpe_drv_job_head *proc_job_head);
int vpe_drv_flow_sw_balance_proc(struct vpe_drv_job_head *proc_job_head);
int vpe_drv_flow_cal_eng_col(struct vpe_drv_job_head *proc_job_head, struct vpe_drv_ctl_eng_col_info *col, int *eng_col_num, int col_max);
void vpe_drv_flow_set_msg(int flag);
#endif //_VPE_DRV_FLOW_INT_H_
