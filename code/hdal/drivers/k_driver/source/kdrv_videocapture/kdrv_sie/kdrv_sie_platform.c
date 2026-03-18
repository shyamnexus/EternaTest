#if defined(__LINUX)
#include <linux/slab.h>
#include <linux/clk.h>
#elif defined(__FREERTOS)
#include <malloc.h>
#endif
#include "kdrv_sie_debug_int.h"
#include "kdrv_sie_int.h"
#include "kdrv_sie_dbg.h"

static UINT32 kdrv_sie_apb_clk = 0;

void *kdrv_sie_os_malloc_wrap(UINT32 want_size)
{
	void *p_buf = NULL;

#if defined(__LINUX)
	p_buf = kmalloc(want_size, GFP_KERNEL);
#elif defined(__FREERTOS)
	p_buf = malloc(want_size);
#endif

	return p_buf;
}

void kdrv_sie_os_mfree_wrap(void *p_buf)
{
#if defined(__LINUX)
	kfree(p_buf);
#elif defined(__FREERTOS)
	free(p_buf);
#endif
}

void kdrv_sie_os_dma_flush(ULONG addr_va, UINT32 size, KDRV_SIE_DMA_FLUSH_DIR dir, KDRV_SIE_DMA_FLUSH_BUFTYPE type)
{
	if (dir == KDRV_SIE_DMA_FLUSH_DIR_FROM_DEV) {
		if (type == KDRV_SIE_DMA_FLUSH_BUFTYPE_NORMAL) {
			return vos_cpu_dcache_sync((VOS_ADDR)addr_va, ALIGN_CEIL(size, VOS_ALIGN_BYTES), VOS_DMA_FROM_DEVICE);
		} else if (type == KDRV_SIE_DMA_FLUSH_BUFTYPE_VDOBLK) {
			return vos_cpu_dcache_sync_vb((VOS_ADDR)addr_va, ALIGN_CEIL(size, VOS_ALIGN_BYTES), VOS_DMA_FROM_DEVICE);
		}
	} else if (dir == KDRV_SIE_DMA_FLUSH_DIR_TO_DEV) {
		if (type == KDRV_SIE_DMA_FLUSH_BUFTYPE_NORMAL) {
			return vos_cpu_dcache_sync((VOS_ADDR)addr_va, ALIGN_CEIL(size, VOS_ALIGN_BYTES), VOS_DMA_TO_DEVICE);
		} else if (type == KDRV_SIE_DMA_FLUSH_BUFTYPE_VDOBLK) {
			return vos_cpu_dcache_sync_vb((VOS_ADDR)addr_va, ALIGN_CEIL(size, VOS_ALIGN_BYTES), VOS_DMA_TO_DEVICE);
		}
	}
	kdrv_sie_dbg_err("dir %d type %d N.S.\r\n", dir, type);
}

UINT32 kdrv_sie_get_apb_clk(void)
{
#if defined(__LINUX)
	struct clk *parent_clk = NULL;
#endif
	UINT32 apb_clk = 120000000;

	if (kdrv_sie_apb_clk) {
		return kdrv_sie_apb_clk;  // clk_get_rate cannot in isr
	}

#if defined(__LINUX)
	parent_clk = clk_get(NULL, "apb_clk");
	if (IS_ERR(parent_clk)) {
		kdrv_sie_dbg_ind("get apb clk fail\r\n");
	} else {
		apb_clk = clk_get_rate(parent_clk);
		clk_put(parent_clk);
	}
#else
	if (pll_get_clock_freq(APBCLK_FREQ, &apb_clk) != E_OK) {
		kdrv_sie_dbg_ind("get apb clk fail\r\n");
	}
#endif

	kdrv_sie_apb_clk = apb_clk;
	kdrv_sie_dbg_ind("apb clock %d\r\n", kdrv_sie_apb_clk);
	return apb_clk;
}