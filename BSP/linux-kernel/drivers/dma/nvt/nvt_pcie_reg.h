/*
   NVT PCIE eDMA structure header

   @file       nvt_pcie_reg.h
   @ingroup
   @note
   Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 as
   published by the Free Software Foundation.
*/

#include <linux/soc/nvt/nvt-pcie-edma.h>

#define DMA_MAX_DESC		24
#define CALLBACK_MAX_TIME	10000000

#define DRV_VER "0.0.3"

struct nvt_desc_list {
	struct nvt_dma_desc		desc[DMA_MAX_DESC];
};

struct dmac_info {
	enum DMA_STATUS			status;
	dma_callback			callback;
	u8                      	priority;
};

struct nvt_dma {
	struct dma_pool		*dma_pool;
	wait_queue_head_t	*wait_queue;
	struct device       	*dev;
	spinlock_t		lock;
	void __iomem            *pci_va;
	void __iomem            *dma_va;
	struct tasklet_struct   read_tasklet;
	struct tasklet_struct   write_tasklet;
	struct dmac_info	*channel;
	unsigned int 		max_chan;
	int			irq;
	u8                      in_use;
};
