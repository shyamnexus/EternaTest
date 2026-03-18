#ifndef __H26XD_H__
#define __H26XD_H__

#include <kwrap/nvt_type.h>
#include <rcw_macro.h>

#ifndef __KERNEL__
typedef unsigned int	u32;
#endif /* __KERNEL__ */

/* rcw_macro.h */
#define ioread32(addr)			(u32)INW((addr))
#define iowrite32(val, addr) 	OUTW((addr), val)

extern ER h26xd_drv_open(void);
extern ER h26xd_drv_close(void);

extern u32 h26xd_drv_get_regbase(void);
extern void h26xd_drv_set_ll_start_addr(u32 base_va, u32 start_addr);
extern void h26xd_drv_trigger_ll(u32 base_va);
extern void h26xd_drv_set_wrap(u32 base_va, u32 *wrap, u32 type);
extern u32 h265d_drv_chk_result(u32 base_va, u32 first_golden, u32 extra_golden);
extern u32 h264d_drv_chk_result(u32 base_va, u32 first_golden, u32 extra_golden);

#endif /* __H26XD_H__ */
