#ifndef __DMA_NVT_H__
#define __DMA_NVT_H__

#define DMA_CACHE_LINE_SZ   1 //support byte-aligned, set to 1
#include <comm/kdrv_hwcopy.h>
#include <linux/soc/nvt/nvt-pcie-lib.h>
#include <linux/soc/nvt/nvtmem.h>
#include <plat/hardware.h>

enum dma_kind {
	APB_DMA,        /* RC DMAc */
	AHB_DMA,        /* RC DMAc */
	AXI_DMA,        /* RC DMAc/HWCPY */
	EP_DMA,
	EP_AHB_DMA = EP_DMA,    /* EP DMAc */
	EP_AXI_DMA,     /* EP DMAc/PCIE DMA */
};

typedef struct {
	phys_addr_t start;
	unsigned long size;
	nvtpcie_ddrid_t ddrid;
	nvtpcie_chipid_t chipid;
	int active;
} ddr_chip_map_t;

int frm_dma_init(void);
void frm_dma_exit(void);

int dma_proc_init(void *data_ptr);
int dma_proc_deinit(void *data_ptr);

int nvtmem_dma_memcpy(nvtpcie_ddrid_t dst_ddr_id, phys_addr_t dst_paddr,
					  nvtpcie_ddrid_t src_ddr_id, phys_addr_t src_paddr, unsigned int size);
int nvtmem_dma_memset(nvtpcie_ddrid_t ddr_id, phys_addr_t paddr, unsigned int size, u32 pattern);
UINT32 nvtmem_dma_flush_dev2mem(uintptr_t addr, UINT32 size);
UINT32 nvtmem_dma_flush_mem2dev(uintptr_t addr, UINT32 size);
uintptr_t nvtmem_dma_va2pa(uintptr_t va);
uintptr_t nvtmem_dma_pa2va_remap(uintptr_t pa, UINT32 size);
VOID nvtmem_dma_pa2va_unmap(uintptr_t va, uintptr_t pa);
#endif /* __DMA_NVT_H__ */
