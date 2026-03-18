/*
 * GM DMA memory operation (MEM_TO_MEM)
 *
 * Copyright (C) 2012 Faraday Technology Corp.
 *
 * Author : Shuao-kai Li <easonli@faraday-tech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 */
#include <linux/dmaengine.h>
#include <kwrap/semaphore.h>

#include "dma_nvt.h"
#include "kwrap/cpu.h"
#include <linux/soc/nvt/fmem.h>

#define HWCPY_MAX_SIZE  (1 << 25)
#define EDMA_MAX_SIZE  (0xFFFFFFFF)
#define CG_HWCP_RELATED_REG_SIZE  (0x200)
#define HWCP_REG_SIZE  (0x500)

#if defined __LINUX
#define TYPEVAFLUSH
#endif

#define DMA_CHANNEL_REQ 4
#define NVT_DMA_CHAN_STATE_BUSY 1
#define NVT_DMA_CHAN_STATE_FREE 0

static ddr_chip_map_t ddr_chip_map[DDR_ID_MAX];

static VK_DEFINE_SEMAPHORE(sem_dmasys);

static ID SEMID_DMA;

struct dma_chan *chan[DMA_CHANNEL_REQ];

static volatile int chan_status[DMA_CHANNEL_REQ] = {0};

int hwcopy_platform_tsk_linear_copy(dma_addr_t dst, dma_addr_t src, size_t len,
				    nvtpcie_chipid_t src_chipid)
{
	struct dma_async_tx_descriptor *tx_desc = NULL;
	dma_cookie_t cookie;
	enum dma_status status;
	unsigned long dma_flags = 0;
	int ret = 0, i, chan_idx = -1;

	vos_sem_wait(SEMID_DMA);

	vk_down(&sem_dmasys);
	for (i = 0; i < DMA_CHANNEL_REQ; i++) {
		if (chan_status[i] == NVT_DMA_CHAN_STATE_FREE) {
			chan_idx = i;
			chan_status[i] = NVT_DMA_CHAN_STATE_BUSY;
			break;
		}
	}
	vk_up(&sem_dmasys);

	if (-1 == chan_idx) {
		pr_err("Not find free dma channel\r\n");
		ret = -1;
		goto dma_exit_copy;
	}

	//prepare dma parameters
	tx_desc = dmaengine_prep_dma_memcpy(chan[chan_idx], dst, src, len, dma_flags);
	if (NULL == tx_desc) {
		pr_err("dmaengine_prep_dma_memcpy failed\r\n");
		ret = -1;
		goto dma_exit_copy;
	}

	//trigger dma to run
	vk_down(&sem_dmasys);
	cookie = dmaengine_submit(tx_desc);
	if (dma_submit_error(cookie)) {
		pr_err("dmaengine_submit failed\r\n");
		ret = -1;
		goto dma_exit_copy;
	}

	//wait dma done
	status = dma_sync_wait(chan[chan_idx], cookie);
	if (DMA_COMPLETE != status) {
		pr_err("dma_sync_wait failed, status %d\r\n", status);
		ret = -1;
		goto dma_exit_copy;
	}

dma_exit_copy:
	chan_status[chan_idx] = NVT_DMA_CHAN_STATE_FREE;
	vk_up(&sem_dmasys);
	vos_sem_sig(SEMID_DMA);

	return ret;
}

int hwcopy_platform_tsk_linear_set(dma_addr_t dst, u32 value, size_t len,
				   nvtpcie_chipid_t src_chipid)
{
	struct dma_async_tx_descriptor *tx_desc = NULL;
	dma_cookie_t cookie;
	enum dma_status status;
	unsigned long dma_flags = 0;
	int ret = 0, i, chan_idx = -1;

	vos_sem_wait(SEMID_DMA);

	vk_down(&sem_dmasys);
	for (i = 0; i < DMA_CHANNEL_REQ; i++) {
		if (chan_status[i] == NVT_DMA_CHAN_STATE_FREE) {
			chan_idx = i;
			chan_status[i] = NVT_DMA_CHAN_STATE_BUSY;
			break;
		}
	}
	vk_up(&sem_dmasys);

	if (-1 == chan_idx) {
		pr_err("Not find free dma channel\r\n");
		ret = -1;
		goto dma_exit_set;
	}

	//prepare dma parameters
	tx_desc = dmaengine_prep_dma_memset(chan[chan_idx], dst, value, len, dma_flags);
	if (NULL == tx_desc) {
		pr_err("dmaengine_prep_dma_memcpy failed\r\n");
		ret = -1;
		goto dma_exit_set;
	}

	//trigger dma to run
	vk_down(&sem_dmasys);
	cookie = dmaengine_submit(tx_desc);
	if (dma_submit_error(cookie)) {
		pr_err("dmaengine_submit failed\r\n");
		ret = -1;
		goto dma_exit_set;
	}

	//wait dma done
	status = dma_sync_wait(chan[chan_idx], cookie);
	if (DMA_COMPLETE != status) {
		pr_err("dma_sync_wait failed, status %d\r\n", status);
		ret = -1;
		goto dma_exit_set;
	}

dma_exit_set:
	chan_status[chan_idx] = NVT_DMA_CHAN_STATE_FREE;
	vk_up(&sem_dmasys);
	vos_sem_sig(SEMID_DMA);

	return ret;
}

int frm_dump_register(void)
{
	unsigned int index = 0, val = 0, ret = 0;
	// CG Register
	void *clk_va = ioremap(NVT_CG_BASE_PHYS, CG_HWCP_RELATED_REG_SIZE);
	// HWCPY Register
	void *hwcpy_va = ioremap(NVT_HWCP_PHY_BASE_PHYS, HWCP_REG_SIZE);

	// Dump HWCPY Register
	pr_info("DEBUG!! : === Dump HWCPY Register (0x%lx) ===\n", NVT_HWCP_PHY_BASE_PHYS);
	for (index = 0; index < HWCP_REG_SIZE ; index += sizeof(unsigned int)) {
		pr_info("DEBUG!! : hwcpy 0x%lx: 0x%x\n", NVT_HWCP_PHY_BASE_PHYS + index, readl((void*)(hwcpy_va + index)));
	}
	pr_info("\n");

	// Dump CG Register
	pr_info("DEBUG!! : === Dump CG Register (0x%lx) ===\n", NVT_CG_BASE_PHYS);
	val = readl((void*)(clk_va + 0x20));
	pr_info("DEBUG!! : Peripheral Clock Select(0x20) 0x%x, HWCP_CLKSEL 0x%x\n", val, (val & 0xC0000) >> 18);

	val = readl((void*)(clk_va + 0x74));
	pr_info("DEBUG!! : Module Clock Enable(0x74) 0x%x, HWCP_CLKEN 0x%x\n", val, (val & 0x200) >> 9);

	val = readl((void*)(clk_va + 0x94));
	pr_info("DEBUG!! : System Reset(0x94) 0x%x, HWCP_RSRN 0x%x\n", val, (val & 0x200) >> 9);

	val = readl((void*)(clk_va + 0xC0));
	pr_info("DEBUG!! : Clock Auto Gating(0xC0) 0x%x, HWCP_CLK_AUTO_GAT_EN 0x%x\n", val, (val & 0x1000000) >> 24);

	val = readl((void*)(clk_va + 0xD0));
	pr_info("DEBUG!! : Auto Gating(0xD0) 0x%x, HWCP_PCLK_AUTO_GAT_EN 0x%x\n", val, (val & 0x200000) >> 21);

	val = readl((void*)(clk_va + 0x150));
	pr_info("DEBUG!! : Program Clock Enable(0x150) 0x%x, HWCP_PROG_CLKEN 0x%x\n", val, (val & 0x8000000) >> 27);
	pr_info("\n");

	iounmap(hwcpy_va);
	iounmap(clk_va);

	return ret;
}

int frm_dma_init(void)
{
	nvtmem_ddrinfo_t ddr_info = {0};
	int bank_idx, idx = 0, i;
	dma_cap_mask_t mask;

	vk_down(&sem_dmasys);

	vos_sem_create(&SEMID_DMA, DMA_CHANNEL_REQ, "SEMID_DMA");

	dma_cap_zero(mask);
	dma_cap_set(DMA_MEMCPY, mask);

	for (i = 0; i < DMA_CHANNEL_REQ; i++) {
		chan[i] = dma_request_channel(mask, NULL, NULL);
		if (NULL == chan[i]) {
			pr_err("dma_request_channel failed. Need to insmod nvt_hwcpy driver before request channel\r\n");
			return -1;
		}
	}

	nvtmem_get_ddrinfo(&ddr_info);
	memset(&ddr_chip_map[0], 0x0, sizeof(ddr_chip_map));

	/* active: It's used to indicate whether the dram is mapped or not */
	for (bank_idx = 0; bank_idx < ddr_info.nr_banks; bank_idx++) {
		ddr_chip_map[idx].start = (uintptr_t)ddr_info.bank[bank_idx].start;
		ddr_chip_map[idx].size = ddr_info.bank[bank_idx].size;
		ddr_chip_map[idx].ddrid = ddr_info.bank[bank_idx].ddrid;
		ddr_chip_map[idx].chipid = ddr_info.bank[bank_idx].chip;
		ddr_chip_map[idx].active = 1;
		idx++;
	}

	vk_up(&sem_dmasys);

	return 0;
}

void frm_dma_exit(void)
{
	int ddrid = 0, i;

	vk_down(&sem_dmasys);

	for (ddrid = 0; ddrid < DDR_ID_MAX; ddrid++) {
		ddr_chip_map[ddrid].active = 0;
	}

	vos_sem_destroy(SEMID_DMA);

	for (i = 0; i < DMA_CHANNEL_REQ; i++) {
		dma_release_channel(chan[i]);
	}

	vk_up(&sem_dmasys);
}

static int dmacopy_rc2rc(dma_addr_t dst_pa, dma_addr_t src_pa, size_t len)
{
	size_t remain_size = len;
	size_t cur_size;
	int ret = 0;

	while (remain_size) {
		cur_size = (remain_size > HWCPY_MAX_SIZE) ? HWCPY_MAX_SIZE : remain_size;

		ret = hwcopy_platform_tsk_linear_copy(dst_pa, src_pa, cur_size, CHIP_RC);
		if (ret) {
			break;
		}

		remain_size -= cur_size;
		dst_pa += cur_size;
		src_pa += cur_size;
	}

	return ret;
}

#if IS_ENABLED(CONFIG_NVT_PCIE_LIB)
static int dmacopy_ep2ep(dma_addr_t dst_pa, dma_addr_t src_pa, size_t len)
{
	void __iomem *src_va, *dst_va;

    src_va = ioremap(src_pa, len);
    if (!src_va) {
        pr_err("src ioremap fail\n");
        return -ENOMEM;
    }

    dst_va = ioremap(dst_pa, len);
    if (!dst_va) {
        pr_err("dst ioremap fail\n");
		iounmap(src_va); //release previous ioremap of src_pa
        return -ENOMEM;
    }

	/* EP only support CPU, not support I/O */
    memcpy(dst_va, src_va, len);

    iounmap(src_va);
    iounmap(dst_va);

    return 0;
}

static int dmacopy_rc_ep_edma(nvtpcie_chipid_t dst_chipid, dma_addr_t dst_pa,
			nvtpcie_chipid_t src_chipid, dma_addr_t src_pa, size_t len)
{
	size_t remain_size = len;
	size_t cur_size;
	int ret = 0;

	while (remain_size) {
		cur_size = (remain_size > HWCPY_MAX_SIZE) ? HWCPY_MAX_SIZE : remain_size;

		ret = nvtpcie_edma_copy(src_chipid, src_pa, dst_chipid, dst_pa, cur_size);
		if (ret) {
			break;
		}

		remain_size -= cur_size;
		dst_pa += cur_size;
		src_pa += cur_size;
	}

	return ret;
}
#endif

/* dmac_side: DMAc location in RC or EP.
 */
int frm_dma_memcpy(nvtpcie_ddrid_t dst_ddrid, phys_addr_t dst_pa,
		   nvtpcie_ddrid_t src_ddrid, phys_addr_t src_pa, size_t len)
{
	int ret;

#if IS_ENABLED(CONFIG_NVT_PCIE_LIB)
	//nvt-pcie-lib is enabled, EP may exists
	nvtpcie_chipid_t src_chipid = CHIP_RC;
	nvtpcie_chipid_t dst_chipid = CHIP_RC;

	//get chipid from ddrid
	if (0 != nvtpcie_get_ddrid_map(src_ddrid, &src_chipid, NULL)) {
		pr_err("nvtpcie_get_ddrid_map failed (src)\n");
		return -1;
	}

	if (0 == nvtpcie_get_ddrid_map(dst_ddrid, &dst_chipid, NULL)) {
		pr_err("nvtpcie_get_ddrid_map failed (dst)\n");
		return -1;
	}

	if (src_chipid != dst_chipid) {
		//cross PCIe, use edma
		ret = dmacopy_rc_ep_edma(dst_chipid, dst_pa, src_chipid, src_pa, len);
	} else {
		if (CHIP_RC == nvtpcie_get_my_chipid()) {
			//rc to rc
			ret = dmacopy_rc2rc(dst_pa, src_pa, len);
		} else {
			//ep to ep
			ret = dmacopy_ep2ep(dst_pa, src_pa, len);
		}
	}

	return ret;

#else //IS_ENABLED(CONFIG_NVT_PCIE_LIB)
	//nvt-pcie-lib is disabled, should be RC only
	ret = dmacopy_rc2rc(dst_pa, src_pa, len);
	return ret;
#endif
}

int frm_dma_memset(nvtpcie_ddrid_t ddr_id, dma_addr_t dst_pa, size_t len,
		   u32 pattern)
{
	int ret;
	size_t cur_size;
	size_t remain_size = len;
	nvtpcie_chipid_t dst_chipid = CHIP_RC;

#if IS_ENABLED(CONFIG_NVT_PCIE_LIB)
	if (nvtpcie_get_my_chipid() == CHIP_RC) {
		//get dst_chipid
		if (0 != nvtpcie_get_ddrid_map(ddr_id, &dst_chipid, NULL)) {
			pr_err("call nvtpcie_get_ddrid_map return fail\n");
			return -1;
		}
		//update dst_pa by the new dst_chipid
		dst_pa = nvtpcie_get_pcie_addr(dst_chipid, ddr_id, dst_pa);
	} else { //CHIP_EPx
		memset_io((void *)dst_pa, pattern, len);
		return 0;
	}
#endif

	do {
		cur_size = (remain_size > HWCPY_MAX_SIZE) ? HWCPY_MAX_SIZE : remain_size;

		ret = hwcopy_platform_tsk_linear_set(dst_pa, pattern, cur_size, dst_chipid);
		if (ret) {
			break;
		}

		remain_size -= cur_size;
		dst_pa += cur_size;
	} while (remain_size);

	return ret;
}

/* @This function uses to do memcpy by DMA
 * @input param: (dst_ddrid + dst_pa): dst physical address. (src_ddrid + src_pa): source physical address.
 * @return value: 0 for success, others for fail.
 */
int nvtmem_dma_memcpy(nvtpcie_ddrid_t dst_ddrid, phys_addr_t dst_pa,
		      nvtpcie_ddrid_t src_ddrid, phys_addr_t src_pa, unsigned int size)
{
	int ret;

	if (!ddr_chip_map[src_ddrid].active) {
		pr_err("Not find src ddr id %d\n", src_ddrid);
		ret = -1;
		goto exit_memcpy;
	}

	if (!ddr_chip_map[dst_ddrid].active) {
		pr_err("Not find dst ddr id %d\n", dst_ddrid);
		ret = -1;
		goto exit_memcpy;
	}

	ret = frm_dma_memcpy(dst_ddrid, dst_pa, src_ddrid, src_pa, size);

	if (ret) {
		pr_err("DEBUG!! Func: %s, src_ddrid %d, dst_ddrid %d, src_pa(0x%llx), dst_pa(0x%llx), size(%d) \n",
			__func__, src_ddrid, dst_ddrid, (u64)src_pa, (u64)dst_pa, size);

		frm_dump_register();
	}

exit_memcpy:

	return ret;
}
EXPORT_SYMBOL(nvtmem_dma_memcpy);

/* @This function uses to do memset by DMA
 * @input param: (ddr_id + paddr): physical address to do memset
 * @input param: pattern: the pattern to fill with
 * @return value: 0 for success, others for fail.
 */
int nvtmem_dma_memset(nvtpcie_ddrid_t ddr_id, phys_addr_t paddr, unsigned int size, u32 pattern)
{
	int ret;

	if (!ddr_chip_map[ddr_id].active) {
		pr_err("Not find ddr id %d\n", ddr_id);
		ret = -1;
		goto exit_memset;
	}

	ret = frm_dma_memset(ddr_id, paddr, size, pattern);

	if (ret) {
		pr_err("DEBUG!! Func: %s, ddr_id %d, paddr(0x%llx), size(%d) \n",
			__func__, ddr_id, (u64)paddr, size);

		frm_dump_register();
	}

exit_memset:

	return ret;
}
EXPORT_SYMBOL(nvtmem_dma_memset);

UINT32 nvtmem_dma_flush_dev2mem(uintptr_t addr, UINT32 size)
{
#if defined __UITRON || defined __ECOS || defined __FREERTOS
    //vos_cpu_dcache_sync_vb(addr, size, VOS_DMA_FROM_DEVICE);
	dma_flushReadCache(addr, size);
	return addr;

#else
	UINT32 ret;

#if 0
	if (size > 256*1024) {
		flush_cache_all();
		ret = 1;
	} else
#endif
	{
#ifdef  TYPEVAFLUSH
        //printk("addr=%ld,size=%d,%d\r\n",(VOS_ADDR)addr,size,VOS_DMA_TO_DEVICE); // debug.
        //uintptr_t address = (uintptr_t) addr;
		vos_cpu_dcache_sync((VOS_ADDR)addr, size, VOS_DMA_BIDIRECTIONAL);
#endif
		ret = 0;
	}

	return ret;
#endif
}

UINT32 nvtmem_dma_flush_mem2dev(uintptr_t addr, UINT32 size)
{
#if defined __UITRON || defined __ECOS || defined __FREERTOS
	dma_flushWriteCache(addr, size);
	return addr;

#else
	UINT32 ret;

#if 0
	if (size > 256*1024) {
		flush_cache_all();
		ret = 1;
	} else
#endif
	{
#ifdef  TYPEVAFLUSH
        //uintptr_t address = (uintptr_t) addr;
		vos_cpu_dcache_sync((VOS_ADDR)addr, size, VOS_DMA_TO_DEVICE);
#endif
		ret = 0;
	}

	return ret;
#endif
}

uintptr_t nvtmem_dma_va2pa(uintptr_t va)
{
#if defined __UITRON || defined __ECOS || defined __FREERTOS
	return dma_getPhyAddr(va);
#else
#ifdef  TYPEVAFLUSH
	return (UINT64)fmem_lookup_pa(va);
#else
	return va;
#endif

#endif
}

uintptr_t nvtmem_dma_pa2va_remap(uintptr_t pa, UINT32 size)
{
	uintptr_t va = 0;
#ifdef  TYPEVAFLUSH
	if (size == 0) {
		return va;
	}
	if (pfn_valid(__phys_to_pfn(pa))) {
		va = (uintptr_t)__va(pa);
	} else {
		va = (uintptr_t)ioremap(pa, PAGE_ALIGN(size));
	}
#else
	va = pa;
#endif
/*	if (va > 0) {
		vos_cpu_dcache_sync(va, size, VOS_DMA_TO_DEVICE);
	}
*/
	return va;
}

VOID nvtmem_dma_pa2va_unmap(uintptr_t va, uintptr_t pa)
{
	if (va == 0) {
		return;
	}
#ifdef  TYPEVAFLUSH
	if (!pfn_valid(__phys_to_pfn(pa))) {
		iounmap((VOID *)va);
	}
#endif
}

