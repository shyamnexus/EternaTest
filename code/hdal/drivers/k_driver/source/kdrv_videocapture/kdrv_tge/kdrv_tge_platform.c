#include "kwrap/type.h"

#ifdef __KERNEL__
#include <linux/clk.h>
#endif

void *kdrv_tge_platform_clk_get_wrap(void *dev, void *con_id)
{
#ifdef __KERNEL__
	return clk_get(dev, con_id);
#else
	return 0;
#endif
}

void kdrv_tge_platform_clk_put_wrap(void *clk)
{
#ifdef __KERNEL__
	clk_put(clk);
#endif
}

void *kdrv_tge_platform_clk_get_parent_wrap(void *clk)
{
#ifdef __KERNEL__
	return clk_get_parent(clk);
#else
	return 0;
#endif
}

int kdrv_tge_platform_clk_set_parent_wrap(void *clk, void *parent)
{
#ifdef __KERNEL__
	return clk_set_parent(clk, parent);
#else
	return 0;
#endif
}

int kdrv_tge_platform_clk_prepare_enable_wrap(void *clk)
{
#ifdef __KERNEL__
	return clk_prepare_enable(clk);
#else
	return 0;
#endif
}

void kdrv_tge_platform_clk_disable_unprepare_wrap(void *clk)
{
#ifdef __KERNEL__
	clk_disable_unprepare(clk);
#endif
}

ULONG kdrv_tge_platform_clk_get_rate_wrap(void *clk)
{
#ifdef __KERNEL__
	return clk_get_rate(clk);
#else
	return 0;
#endif
}

