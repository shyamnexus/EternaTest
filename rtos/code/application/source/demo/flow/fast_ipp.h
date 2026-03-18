#ifndef _FAST_IPP_H
#define _FAST_IPP_H

int fast_ipp(void);
int fast_ipp_run_ll(void);
int bridge_mem_plan_sie(void);
int bridge_mem_plan_ipp(void);
int ise_scale(uintptr_t out_addr, int out_lofs, int out_w, int out_h, uintptr_t in_addr, int in_lofs, int in_w, int in_h, int io_pack_fmt, int scl_method);

#endif
