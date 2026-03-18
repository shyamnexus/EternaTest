#ifndef _KDRV_TGE_INT_PLATFORM_H_
#define _KDRV_TGE_INT_PLATFORM_H_

#include "kdrv_videocapture/kdrv_tge.h"
#ifdef __KERNEL__
#include "kdrv_tge_int_drv.h"
#endif

// wrapper function for kernel dependency

// see -> struct clk *clk_get(struct device *dev, const char *con_id)
void *kdrv_tge_platform_clk_get_wrap(void *dev, void *con_id);

// see -> void clk_put(struct clk *clk)
void kdrv_tge_platform_clk_put_wrap(void *clk);

// see -> struct clk *clk_get_parent(struct clk *clk)
void *kdrv_tge_platform_clk_get_parent_wrap(void *clk);

// see -> int clk_set_parent(struct clk *clk, struct clk *parent)
int kdrv_tge_platform_clk_set_parent_wrap(void *clk, void *parent);

// see -> static inline int clk_prepare_enable(struct clk *clk)
int kdrv_tge_platform_clk_prepare_enable_wrap(void *clk);

// see -> static inline void clk_disable_unprepare(struct clk *clk)
void kdrv_tge_platform_clk_disable_unprepare_wrap(void *clk);

// see -> unsigned long clk_get_rate(struct clk *clk)
ULONG kdrv_tge_platform_clk_get_rate_wrap(void *clk);


#ifdef __KERNEL__
INT32 kdrv_tge_platform_create_resource(TGE_MODULE_INFO *pmodule_info);
void kdrv_tge_platform_release_resource(void);
#endif
ER kdrv_tge_platform_lock(void);
void kdrv_tge_platform_unlock(void);
BOOL kdrv_tge_platform_is_fastboot(void);
ER kdrv_tge_platform_flsh_msh_clk_enable(KDRV_TGE_CLK_SRC clk_src, UINT64 *clk_rate);
ER kdrv_tge_platform_flsh_msh_clk_disable(KDRV_TGE_CLK_SRC clk_src);

INT32 kdrv_tge_int_printf(const char *fmtstr, ...);
#endif