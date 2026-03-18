/*
   NVT PCIE eDMA header

   @file       nvt-pcie-edma.h
   @ingroup
   @note
   Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 as
   published by the Free Software Foundation.
*/

#include <linux/irqreturn.h>

#if defined(CONFIG_NVT_PCIE_CASCADE_DRV)
int nvt_pcie_edma_read(nvtpcie_chipid_t tid, phys_addr_t src_paddr, phys_addr_t dst_paddr, u32 len);
int nvt_pcie_edma_write(nvtpcie_chipid_t tid, phys_addr_t src_paddr, phys_addr_t dst_paddr, u32 len);
#else

enum DMA_STATUS {
	DMA_IDLE = 0,
	DMA_IN_PROCESS,
	DMA_FINISH,
	DMA_ABORT,
	DMA_ERR,
	DMA_TIMEOUT,
	DMA_BUSY,
	ALLOC_POOL_FAIL,
};

enum transfer_direction {
	DMA_READ = 0,
	DMA_WRITE,
};

#pragma pack(push, 1)
struct nvt_dma_descriptor {
	u32	chan_ctrl;
	u32	len;
	u64	src_paddr;
	u64	dst_paddr;
};

struct nvt_dma_enddesc {
	u32	chan_ctrl;
	u32	reserve;
	u64	llp;
};
#pragma pack(pop)

struct nvt_dma_desc {
	union {
		struct nvt_dma_enddesc          enddesc;
		struct nvt_dma_descriptor       element;
	};
};

typedef void (*dma_callback)(u32 chan, enum DMA_STATUS status);

int nvt_alloc_edmac_chan(void);
void nvt_free_edmac_chan(int chan);

/* The maximum number of packets per burst is 31 */
int nvt_pcie_config_edma_chan(int chan, unsigned char max_packet);
int nvt_pcie_edma_read(int chan, phys_addr_t src_paddr, phys_addr_t dst_paddr, u32 len, dma_callback callback, bool blocking);
int nvt_pcie_edma_write(int chan, phys_addr_t src_paddr, phys_addr_t dst_paddr, u32 len, dma_callback callback, bool blocking);

irqreturn_t nvt_pcie_drv_isr(int irq, void *data);
#endif /* CONFIG_NVT_PCIE_CASCADE_DRV */

#ifdef CONFIG_NVT_IVOT_PLAT_NS02201
void nvt_edma_reinit(void);
#endif

