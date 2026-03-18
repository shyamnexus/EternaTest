/**
    NVT hdal dma memory operation handling
    Add a wrapper to handle the dma memory handling API
    @file       nvtdma.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef __NVTDMA_H
#define __NVTDMA_H

#include <comm/dma_util.h>
#include <linux/soc/nvt/nvtmem.h>


/* @This function uses to do memcpy by DMA
 * @input param: (dst_ddr_id + dst_paddr): dst physical address. (src_ddr_id + src_paddr): source physical address.
 * @input param: size: must be 4 bytes alignment.
 * @return value: 0 for success, others for fail.
 */
int nvtmem_dma_memcpy(nvtpcie_ddrid_t dst_ddr_id, phys_addr_t dst_paddr, nvtpcie_ddrid_t src_ddr_id, phys_addr_t src_paddr, int size);

/* @This function uses to do memset by DMA
 * @input param: (ddr_id + paddr): physical address to do memset
 * @input param: size: must be 4 bytes alignment.
 * @input param: pattern: the pattern to fill with
 * @return value: 0 for success, others for fail.
 */
int nvtmem_dma_memset(nvtpcie_ddrid_t ddr_id, phys_addr_t paddr, int size, u32 pattern);
#endif /* __NVTDMA_H */
