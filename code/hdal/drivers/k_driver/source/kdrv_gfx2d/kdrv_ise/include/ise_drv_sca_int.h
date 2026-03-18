#ifndef _ISE_DRV_SCA_INT_H_
#define _ISE_DRV_SCA_INT_H_
#include "ise_drv_ll_int.h"
#include "kdrv_ise_ctl.h"
#include "ise_eng.h"

int ise_drv_sca_proc(ISE_ENG_HANDLE *eng_hdl, struct ise_drv_job_cfg *job_cfg);
int ise_drv_sca_chk_align(struct ise_drv_in_info *src, struct ise_drv_out_info *dst);
int ise_drv_sca_chk_scale_ratio(unsigned int in_size, unsigned int out_size, char *msg);
int ise_drv_sca_cache_flush(struct ise_drv_addr *addr, struct ise_drv_roi roi, enum ise_drv_fmt fmt, int dir);
void ise_drv_sca_dump_src_info(struct ise_drv_in_info *in);
void ise_drv_sca_dump_dst_info(struct ise_drv_out_info *out);
void ise_drv_sca_dump_scale_info(struct ise_drv_iq_info *iq);

#endif //_ISE_DRV_SCA_INT_H_
