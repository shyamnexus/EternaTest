#include <asm/io.h>
#include <linux/slab.h> //kmalloc
#include <linux/time.h> //do_gettimeofday
#include <linux/soc/nvt/fmem.h> //fmem_lookup_pa
#include "nvt-pcie-lib-internal.h"

#define PERF_ALLOC_SIZE (1*1024*1024)

#define PERF_VAL(name) u64 name
#define PERF_MARK(p) p = ktime_get_real_ns()
#define PERF_DIFF(from, to) (to - from)

static void _perf_read_remote(int use_edma)
{
	nvtpcie_chipid_t ep_chipid = CHIP_EP0;
	phys_addr_t ep_pa = NVTPCIE_INVALID_PA;
	phys_addr_t rc_pa;
	void *rc_va = NULL;
	void *ep_va = NULL;
	unsigned long alloc_size = PERF_ALLOC_SIZE;

	unsigned long sum_size = 0;
	unsigned long sum_dur_us = 0;
	unsigned long dur_us;
	int loop_num = 10;

	PERF_VAL(t1_usec);
	PERF_VAL(t2_usec);

	ep_va = nvtpcie_alloc_epmem(ep_chipid, alloc_size, &ep_pa);
	if (NULL == ep_va) {
		DBG_ERR("alloc_epmem failed\r\n");
		goto exit;
	}

	rc_va = kmalloc(alloc_size, GFP_KERNEL);
	if (NULL == rc_va) {
		DBG_ERR("rc kmalloc failed\r\n");
		goto exit;
	}

	rc_pa = fmem_lookup_pa((uintptr_t)rc_va);
	if (-1UL == rc_pa) {
		DBG_ERR("get rc pa failed\r\n");
		goto exit;
	}

	do {
		if (use_edma) {
			PERF_MARK(t1_usec);
			if (0 != nvtpcie_edma_copy(ep_chipid, ep_pa, CHIP_RC, rc_pa, alloc_size)) {
				DBG_ERR("edma failed\r\n");
				goto exit;
			}
			PERF_MARK(t2_usec);
		} else {
			PERF_MARK(t1_usec);
			memcpy_fromio(rc_va, ep_va, alloc_size);
			PERF_MARK(t2_usec);
		}

		dur_us = PERF_DIFF(t1_usec, t2_usec);
		DBG_DUMP("loop_num %d, %ld bytes, dur_us %ld\r\n", loop_num, alloc_size, dur_us);

		sum_size += alloc_size;
		sum_dur_us += dur_us;
	} while (--loop_num);

	DBG_DUMP("avg: %ld KB/s (%ld bytes / %ld us)\r\n", sum_size / (sum_dur_us/1000), sum_size, sum_dur_us);

exit:
	if (rc_va) {
		kfree(rc_va);
	}
}

static void _perf_read_local(void)
{
	void *va_src = NULL;
	void *va_dst = NULL;

	unsigned long alloc_size = PERF_ALLOC_SIZE;
	unsigned long sum_size = 0;
	unsigned long sum_dur_us = 0;
	unsigned long dur_us;
	int loop_num = 10;

	PERF_VAL(t1_usec);
	PERF_VAL(t2_usec);

	va_src = kmalloc(alloc_size, GFP_KERNEL);
	if (NULL == va_src) {
		DBG_ERR("rc kmalloc src failed\r\n");
		goto exit;
	}

	va_dst = kmalloc(alloc_size, GFP_KERNEL);
	if (NULL == va_dst) {
		DBG_ERR("rc kmalloc dst failed\r\n");
		goto exit;
	}

	do {
		PERF_MARK(t1_usec);
		memcpy_fromio(va_dst, va_src, alloc_size);
		PERF_MARK(t2_usec);

		dur_us = PERF_DIFF(t1_usec, t2_usec);
		DBG_DUMP("loop_num %d, %ld bytes, dur_us %ld\r\n", loop_num, alloc_size, dur_us);

		sum_size += alloc_size;
		sum_dur_us += dur_us;
	} while (--loop_num);

	DBG_DUMP("avg: %ld KB/s (%ld bytes / %ld us)\r\n", sum_size / (sum_dur_us/1000), sum_size, sum_dur_us);

exit:
	if (va_src) {
		kfree(va_src);
	}

	if (va_dst) {
		kfree(va_dst);
	}
}

static void _perf_write_remote(int use_edma)
{
	nvtpcie_chipid_t ep_chipid = CHIP_EP0;
	phys_addr_t ep_pa = NVTPCIE_INVALID_PA;
	phys_addr_t rc_pa;
	void *rc_va = NULL;
	void *ep_va = NULL;

	unsigned long alloc_size = PERF_ALLOC_SIZE;
	unsigned long sum_size = 0;
	unsigned long sum_dur_us = 0;
	unsigned long dur_us;
	int loop_num = 10;

	PERF_VAL(t1_usec);
	PERF_VAL(t2_usec);

	ep_va = nvtpcie_alloc_epmem(ep_chipid, alloc_size, &ep_pa);
	if (NULL == ep_va) {
		DBG_ERR("alloc_epmem failed\r\n");
		goto exit;
	}

	rc_va = kmalloc(alloc_size, GFP_KERNEL);
	if (NULL == rc_va) {
		DBG_ERR("rc kmalloc failed\r\n");
		goto exit;
	}

	rc_pa = fmem_lookup_pa((uintptr_t)rc_va);
	if (-1UL == rc_pa) {
		DBG_ERR("get rc pa failed\r\n");
		goto exit;
	}

	do {
		if (use_edma) {
			PERF_MARK(t1_usec);
			if (0 != nvtpcie_edma_copy(CHIP_RC, rc_pa, ep_chipid, ep_pa, alloc_size)) {
				DBG_ERR("edma failed\r\n");
				goto exit;
			}
			PERF_MARK(t2_usec);
		} else {
			PERF_MARK(t1_usec);
			memcpy_toio(ep_va, rc_va, alloc_size);
			PERF_MARK(t2_usec);
		}

		dur_us = PERF_DIFF(t1_usec, t2_usec);
		DBG_DUMP("loop_num %d, %ld bytes, dur_us %ld\r\n", loop_num, alloc_size, dur_us);

		sum_size += alloc_size;
		sum_dur_us += dur_us;
	} while (--loop_num);

	DBG_DUMP("avg: %ld KB/s (%ld bytes / %ld us)\r\n", sum_size / (sum_dur_us/1000), sum_size, sum_dur_us);

exit:
	if (rc_va) {
		kfree(rc_va);
	}
}

static void _perf_write_local(void)
{
	void *va_src = NULL;
	void *va_dst = NULL;

	unsigned long alloc_size = PERF_ALLOC_SIZE;
	unsigned long sum_size = 0;
	unsigned long sum_dur_us = 0;
	unsigned long dur_us;
	int loop_num = 10;

	PERF_VAL(t1_usec);
	PERF_VAL(t2_usec);

	va_src = kmalloc(alloc_size, GFP_KERNEL);
	if (NULL == va_src) {
		DBG_ERR("rc kmalloc src failed\r\n");
		goto exit;
	}

	va_dst = kmalloc(alloc_size, GFP_KERNEL);
	if (NULL == va_dst) {
		DBG_ERR("rc kmalloc dst failed\r\n");
		goto exit;
	}

	do {
		PERF_MARK(t1_usec);
		memcpy_toio(va_dst, va_src, alloc_size);
		PERF_MARK(t2_usec);

		dur_us = PERF_DIFF(t1_usec, t2_usec);
		DBG_DUMP("loop_num %d, %ld bytes, dur_us %ld\r\n", loop_num, alloc_size, dur_us);

		sum_size += alloc_size;
		sum_dur_us += dur_us;
	} while (--loop_num);

	DBG_DUMP("avg: %ld KB/s (%ld bytes / %ld us)\r\n", sum_size / (sum_dur_us/1000), sum_size, sum_dur_us);

exit:
	if (va_src) {
		kfree(va_src);
	}

	if (va_dst) {
		kfree(va_dst);
	}
}

static int _nvtpcie_mem_test(void* va, unsigned long size)
{
	const u32 fixed_val = 0x55555555;
	int i;
	u32 *p_u32;

	memset_io(va, fixed_val, size);
	p_u32 = va;

	for (i = 0; i < size/sizeof(u32); i++) {
		if (*p_u32 != fixed_val) {
			DBG_ERR("va[%d] 0x%X != 0x%X\r\n", i, *p_u32, fixed_val);
			return -1;
		}
		p_u32++;
	}

	return 0;
}

static int _nvtpcie_reg_test(phys_addr_t pa)
{
	#define REG_TEST_VAL 0x12345678

	void __iomem *p_map;
	u32 org_val, new_val, chk_val;

	DBG_DUMP("%s pa 0x%llX\r\n", __func__, (u64)pa);

	p_map = ioremap(pa, PAGE_SIZE);
	if (NULL == p_map) {
		DBG_ERR("ioremap 0x%016llX failed\r\n", (u64)pa);
		return -1;
	}

	org_val = readl(p_map);
	DBG_DUMP("org_val = 0x%X\r\n", org_val);

	writel(REG_TEST_VAL, p_map);
	new_val = readl(p_map);
	DBG_DUMP("new_val = 0x%X\r\n", new_val);
	if (new_val == org_val) {
		DBG_ERR("writel new_val failed\r\n");
	}

	writel(org_val, p_map);
	chk_val = readl(p_map);
	DBG_DUMP("chk_val = 0x%X\r\n", chk_val);

	if (chk_val != org_val) {
		DBG_ERR("restore org_val failed\r\n");
	}

	iounmap(p_map);

	return 0;
}

void nvtpcie_test_perf(void)
{
	if (CHIP_RC != nvtpcie_get_my_chipid()) {
		DBG_DUMP("skip perf test for non-RC\r\n");
		return;
	}

	DBG_DUMP("\r\n[REMOTE READ (edma)]\r\n");
	_perf_read_remote(1);

	DBG_DUMP("\r\n[REMOTE READ (memcpy_fromio)]\r\n");
	_perf_read_remote(0);

	DBG_DUMP("\r\n[LOCAL READ (memcpy_fromio)]\r\n");
	_perf_read_local();

	DBG_DUMP("\r\n[REMOTE WRITE (edma)]\r\n");
	_perf_write_remote(1);

	DBG_DUMP("\r\n[REMOTE WRITE (memcpy_toio)]\r\n");
	_perf_write_remote(0);

	DBG_DUMP("\r\n[LOCAL WRITE (memcpy_toio)]\r\n");
	_perf_write_local();
}

void nvtpcie_test_api(void)
{
	#define TEST_PA_LOCAL_1     0x0000000001110000ULL
	#define TEST_PA_LOCAL_2     0x0000000042220000ULL
	#define TEST_PA_LOCAL_R     0x00000002F0110084ULL
	//--------------------------0xHHHHHHHHLLLLLLLLULL
	#define TEST_PA_RC_REMOTE_1 0x0000000401110000ULL
	#define TEST_PA_RC_REMOTE_2 0x0000000442220000ULL
	#define TEST_PA_RC_REMOTE_R 0x0000000620110000ULL
	//--------------------------0xHHHHHHHHLLLLLLLLULL
	#define TEST_PA_EP_REMOTE_1 0x0000000C01110000ULL
	#define TEST_PA_EP_REMOTE_2 0x0000000C42220000ULL
	#define TEST_PA_EP_REMOTE_R 0x0000000E00110000ULL
	//--------------------------0xHHHHHHHHLLLLLLLLULL

	DBG_DUMP("\r\nnvtpcie_get_ddrid_map\r\n");
	_nvtpcie_dump_ddr_map();

	DBG_DUMP("nvtpcie_get_ep_count = %d\r\n", nvtpcie_get_ep_count());

	DBG_DUMP("nvtpcie_is_common_pci = %d\r\n", nvtpcie_is_common_pci());

	DBG_DUMP("\r\nnvtpcie_get_ddrid test\r\n");
	{
		nvtpcie_chipid_t chipid;
		phys_addr_t pa;
		nvtpcie_ddrid_t ret_ddrid;

		chipid = CHIP_RC;
		pa = TEST_PA_LOCAL_1;
		ret_ddrid = nvtpcie_get_ddrid(chipid, pa);
		DBG_DUMP("chipid %d, pa 0x%016llX => ddrid %d\r\n", chipid, (u64)pa, ret_ddrid);

		chipid = CHIP_RC;
		pa = TEST_PA_LOCAL_2;
		ret_ddrid = nvtpcie_get_ddrid(chipid, pa);
		DBG_DUMP("chipid %d, pa 0x%016llX => ddrid %d\r\n", chipid, (u64)pa, ret_ddrid);

		chipid = CHIP_RC;
		pa = TEST_PA_LOCAL_R;
		ret_ddrid = nvtpcie_get_ddrid(chipid, pa);
		DBG_DUMP("chipid %d, pa 0x%016llX => ddrid %d\r\n", chipid, (u64)pa, ret_ddrid);

		chipid = CHIP_RC;
		pa = TEST_PA_RC_REMOTE_1;
		ret_ddrid = nvtpcie_get_ddrid(chipid, pa);
		DBG_DUMP("chipid %d, pa 0x%016llX => ddrid %d\r\n", chipid, (u64)pa, ret_ddrid);

		chipid = CHIP_RC;
		pa = TEST_PA_RC_REMOTE_2;
		ret_ddrid = nvtpcie_get_ddrid(chipid, pa);
		DBG_DUMP("chipid %d, pa 0x%016llX => ddrid %d\r\n", chipid, (u64)pa, ret_ddrid);

		chipid = CHIP_RC;
		pa = TEST_PA_RC_REMOTE_R;
		ret_ddrid = nvtpcie_get_ddrid(chipid, pa);
		DBG_DUMP("chipid %d, pa 0x%016llX => ddrid %d\r\n", chipid, (u64)pa, ret_ddrid);

		chipid = CHIP_EP0;
		pa = TEST_PA_LOCAL_1;
		ret_ddrid = nvtpcie_get_ddrid(chipid, pa);
		DBG_DUMP("chipid %d, pa 0x%016llX => ddrid %d\r\n", chipid, (u64)pa, ret_ddrid);

		chipid = CHIP_EP0;
		pa = TEST_PA_LOCAL_2;
		ret_ddrid = nvtpcie_get_ddrid(chipid, pa);
		DBG_DUMP("chipid %d, pa 0x%016llX => ddrid %d\r\n", chipid, (u64)pa, ret_ddrid);

		chipid = CHIP_EP0;
		pa = TEST_PA_LOCAL_R;
		ret_ddrid = nvtpcie_get_ddrid(chipid, pa);
		DBG_DUMP("chipid %d, pa 0x%016llX => ddrid %d\r\n", chipid, (u64)pa, ret_ddrid);

		chipid = CHIP_EP0;
		pa = TEST_PA_EP_REMOTE_1;
		ret_ddrid = nvtpcie_get_ddrid(chipid, pa);
		DBG_DUMP("chipid %d, pa 0x%016llX => ddrid %d\r\n", chipid, (u64)pa, ret_ddrid);

		chipid = CHIP_EP0;
		pa = TEST_PA_EP_REMOTE_2;
		ret_ddrid = nvtpcie_get_ddrid(chipid, pa);
		DBG_DUMP("chipid %d, pa 0x%016llX => ddrid %d\r\n", chipid, (u64)pa, ret_ddrid);

		chipid = CHIP_EP0;
		pa = TEST_PA_EP_REMOTE_R;
		ret_ddrid = nvtpcie_get_ddrid(chipid, pa);
		DBG_DUMP("chipid %d, pa 0x%016llX => ddrid %d\r\n", chipid, (u64)pa, ret_ddrid);
	}

	DBG_DUMP("\r\nnvtpcie_get_upstream_pa\r\n");
	{
		nvtpcie_chipid_t ep_chipid;
		phys_addr_t rc_pa;
		phys_addr_t ep_up_pa;

		ep_chipid = CHIP_EP0;
		rc_pa = TEST_PA_LOCAL_1;

		ep_up_pa = nvtpcie_get_upstream_pa(ep_chipid, rc_pa);
		DBG_DUMP("ep_chipid %d, rc_pa 0x%016llX, ep_ups_pa 0x%016llX\r\n", ep_chipid, (u64)rc_pa, (u64)ep_up_pa);

		ep_chipid = CHIP_EP0;
		rc_pa = TEST_PA_LOCAL_2;
		ep_up_pa = nvtpcie_get_upstream_pa(ep_chipid, rc_pa);
		DBG_DUMP("ep_chipid %d, rc_pa 0x%016llX, ep_ups_pa 0x%016llX\r\n", ep_chipid, (u64)rc_pa, (u64)ep_up_pa);

		ep_chipid = CHIP_EP0;
		rc_pa = TEST_PA_LOCAL_R;
		ep_up_pa = nvtpcie_get_upstream_pa(ep_chipid, rc_pa);
		DBG_DUMP("ep_chipid %d, rc_pa 0x%016llX, ep_ups_pa 0x%016llX\r\n", ep_chipid, (u64)rc_pa, (u64)ep_up_pa);
	}

	DBG_DUMP("\r\nnvtpcie_get_downstream_pa\r\n");
	{
		nvtpcie_chipid_t ep_chipid;
		phys_addr_t ep_pa;
		phys_addr_t rc_down_pa;

		ep_chipid = CHIP_EP0;
		ep_pa = TEST_PA_LOCAL_1;
		rc_down_pa = nvtpcie_get_downstream_pa(ep_chipid, ep_pa);
		DBG_DUMP("ep_chipid %d, ep_pa 0x%016llX, rc_down_pa 0x%016llX\r\n", ep_chipid, (u64)ep_pa, (u64)rc_down_pa);

		ep_chipid = CHIP_EP0;
		ep_pa = TEST_PA_LOCAL_2;
		rc_down_pa = nvtpcie_get_downstream_pa(ep_chipid, ep_pa);
		DBG_DUMP("ep_chipid %d, ep_pa 0x%016llX, rc_down_pa 0x%016llX\r\n", ep_chipid, (u64)ep_pa, (u64)rc_down_pa);

		ep_chipid = CHIP_EP0;
		ep_pa = TEST_PA_LOCAL_R;
		rc_down_pa = nvtpcie_get_downstream_pa(ep_chipid, ep_pa);
		DBG_DUMP("ep_chipid %d, ep_pa 0x%016llX, rc_down_pa 0x%016llX\r\n", ep_chipid, (u64)ep_pa, (u64)rc_down_pa);
		if (0 != _nvtpcie_reg_test(rc_down_pa)) {
			DBG_ERR("ep_chipid %d reg_test failed\r\n", ep_chipid);
		}
	}

	DBG_DUMP("\r\nnvtpcie_get_pcie_addr\r\n");
	{
		nvtpcie_chipid_t loc_chipid;
		nvtpcie_ddrid_t tar_ddrid;
		phys_addr_t tar_pa;
		phys_addr_t pcie_pa;

		//local
		loc_chipid = CHIP_RC;
		tar_ddrid = DDR_ID0;
		tar_pa = TEST_PA_LOCAL_1;
		pcie_pa = nvtpcie_get_pcie_addr(loc_chipid, tar_ddrid, tar_pa);
		DBG_DUMP("     local: loc_chipid %d, tar_ddrid %d, tar_pa 0x%016llX, pcie_pa 0x%016llX\r\n",
			loc_chipid, tar_ddrid, (u64)tar_pa, (u64)pcie_pa);

		//downstream
		loc_chipid = CHIP_RC;
		tar_ddrid = DDR_ID2;
		tar_pa = TEST_PA_LOCAL_1;
		pcie_pa = nvtpcie_get_pcie_addr(loc_chipid, tar_ddrid, tar_pa);
		DBG_DUMP("downstream: loc_chipid %d, tar_ddrid %d, tar_pa 0x%016llX, pcie_pa 0x%016llX\r\n",
			loc_chipid, tar_ddrid, (u64)tar_pa, (u64)pcie_pa);

		//upstream
		loc_chipid = CHIP_EP0;
		tar_ddrid = DDR_ID0;
		tar_pa = TEST_PA_LOCAL_1;
		pcie_pa = nvtpcie_get_pcie_addr(loc_chipid, tar_ddrid, tar_pa);
		DBG_DUMP("  upstream: loc_chipid %d, tar_ddrid %d, tar_pa 0x%016llX, pcie_pa 0x%016llX\r\n",
			loc_chipid, tar_ddrid, (u64)tar_pa, (u64)pcie_pa);
	}

	DBG_DUMP("\r\nnvtpcie_get_mapped_pa and nvtpcie_get_unmapped_pa\r\n");
	{
		nvtpcie_chipid_t loc_chipid;
		nvtpcie_chipid_t tar_chipid;
		nvtpcie_chipid_t map_chipid;
		phys_addr_t tar_pa;
		phys_addr_t mapped_pa;
		phys_addr_t unmapped_pa;
		phys_addr_t ans_pa;

		//local
		loc_chipid = CHIP_RC;
		tar_chipid = CHIP_RC;
		map_chipid = CHIP_MAX;
		tar_pa = TEST_PA_LOCAL_1;

		mapped_pa = nvtpcie_get_mapped_pa(loc_chipid, tar_chipid, tar_pa);
		if (mapped_pa != tar_pa) {
			DBG_ERR("     local: loc_chipid %d, tar_chipid %d, tar_pa 0x%016llX, mapped_pa 0x%016llX\r\n",
				loc_chipid, tar_chipid, (u64)tar_pa, (u64)mapped_pa);
		}

		unmapped_pa = nvtpcie_get_unmapped_pa(loc_chipid, mapped_pa, &map_chipid);
		if ((unmapped_pa != tar_pa) || (map_chipid != tar_chipid)) {
			DBG_ERR("     local: loc_chipid %d, tar_chipid %d, map_chipid %d, tar_pa 0x%016llX, mapped_pa 0x%016llX, umapped_pa 0x%016llX\r\n",
				loc_chipid, tar_chipid, map_chipid, (u64)tar_pa, (u64)mapped_pa, (u64)unmapped_pa);
		}

		//downstream
		loc_chipid = CHIP_RC;
		tar_chipid = CHIP_EP0;
		map_chipid = CHIP_MAX;
		tar_pa = TEST_PA_LOCAL_1;

		mapped_pa = nvtpcie_get_mapped_pa(loc_chipid, tar_chipid, tar_pa);
		ans_pa = nvtpcie_get_downstream_pa(CHIP_EP0, tar_pa);
		if (mapped_pa != ans_pa) {
			DBG_ERR("downstream: loc_chipid %d, tar_chipid %d, tar_pa 0x%016llX, mapped_pa 0x%016llX\r\n",
				loc_chipid, tar_chipid, (u64)tar_pa, (u64)mapped_pa);
		}

		unmapped_pa = nvtpcie_get_unmapped_pa(loc_chipid, mapped_pa, &map_chipid);
		if ((unmapped_pa != tar_pa) || (map_chipid != tar_chipid)) {
			DBG_ERR("downstream: loc_chipid %d, tar_chipid %d, tar_pa 0x%016llX, mapped_pa 0x%016llX, umapped_pa 0x%016llX\r\n",
				loc_chipid, tar_chipid, (u64)tar_pa, (u64)mapped_pa, (u64)unmapped_pa);
		}

		//upstream
		loc_chipid = CHIP_EP0;
		tar_chipid = CHIP_RC;
		map_chipid = CHIP_MAX;
		tar_pa = TEST_PA_LOCAL_1;

		mapped_pa = nvtpcie_get_mapped_pa(loc_chipid, tar_chipid, tar_pa);
		ans_pa = nvtpcie_get_upstream_pa(CHIP_EP0, tar_pa);
		if (mapped_pa != ans_pa) {
			DBG_ERR("  upstream: loc_chipid %d, tar_chipid %d, tar_pa 0x%016llX, mapped_pa 0x%016llX\r\n",
				loc_chipid, tar_chipid, (u64)tar_pa, (u64)mapped_pa);
		}

		unmapped_pa = nvtpcie_get_unmapped_pa(loc_chipid, mapped_pa, &map_chipid);
		if ((unmapped_pa != tar_pa) || (map_chipid != tar_chipid)) {
			DBG_ERR("  upstream: loc_chipid %d, tar_chipid %d, tar_pa 0x%016llX, mapped_pa 0x%016llX, umapped_pa 0x%016llX\r\n",
				loc_chipid, tar_chipid, (u64)tar_pa, (u64)mapped_pa, (u64)unmapped_pa);
		}
	}

	DBG_DUMP("\r\nnvtpcie_shmem_xxx test\r\n");
	{
		phys_addr_t pa;
		void * va;
		unsigned long size;

		pa = nvtpcie_shmem_get_pa();
		if (NVTPCIE_INVALID_PA == pa) {
			DBG_ERR("nvtpcie_shmem_get_pa failed\r\n");
		}

		va = nvtpcie_shmem_get_va();
		if (NULL == va) {
			DBG_ERR("nvtpcie_shmem_get_va failed\r\n");
		}

		size = nvtpcie_shmem_get_size();
		if (0 == size) {
			DBG_ERR("nvtpcie_shmem_get_size failed\r\n");
		}

		if (0 != _nvtpcie_mem_test(va, size)) {
			DBG_ERR("shmem test failed\r\n");
		}

		DBG_DUMP("shmem pa 0x%016llX, va 0x%016llX, size 0x%lX\r\n", (u64)pa, P2U64(va), size);
	}

	DBG_DUMP("\r\nnvtpcie_alloc_epmem\r\n");
	_nvtpcie_dump_ep_allocmem();
	{
		nvtpcie_chipid_t ep_chipid;
		unsigned long len;
		phys_addr_t ret_ep_pa = NVTPCIE_INVALID_PA;
		void *p_rc_va;

		ep_chipid = CHIP_EP0;
		len = 0x10000;
		p_rc_va = nvtpcie_alloc_epmem(ep_chipid, len, &ret_ep_pa);
		if (NULL == p_rc_va) {
			DBG_ERR("nvtpcie_alloc_epmem failed\r\n");
		} else {
			DBG_DUMP("alloc_epmem 1: ep_chipid %d, len %ld, p_rc_va 0x%016llX, ret_ep_pa 0x%016llX\r\n",
				ep_chipid, len, P2U64(p_rc_va), (u64)ret_ep_pa);
			if (0 != _nvtpcie_mem_test(p_rc_va, len)) {
				DBG_ERR("alloc_epmem 1: mem test failed\r\n");
			}
		}

		ep_chipid = CHIP_EP0;
		len = 0x10000;
		p_rc_va = nvtpcie_alloc_epmem(ep_chipid, len, &ret_ep_pa);
		if (NULL == p_rc_va) {
			DBG_ERR("nvtpcie_alloc_epmem failed\r\n");
		} else {
			DBG_DUMP("alloc_epmem 2: ep_chipid %d, len %ld, p_rc_va 0x%016llX, ret_ep_pa 0x%016llX\r\n",
				ep_chipid, len, P2U64(p_rc_va), (u64)ret_ep_pa);
			if (0 != _nvtpcie_mem_test(p_rc_va, len)) {
				DBG_ERR("alloc_epmem 2: mem test failed\r\n");
			}
		}

		ep_chipid = CHIP_EP0;
		len = 0x10000;
		p_rc_va = nvtpcie_alloc_epmem(ep_chipid, len, &ret_ep_pa);
		if (NULL == p_rc_va) {
			DBG_ERR("nvtpcie_alloc_epmem failed\r\n");
		} else {
			DBG_DUMP("alloc_epmem 3: ep_chipid %d, len %ld, p_rc_va 0x%016llX, ret_ep_pa 0x%016llX\r\n",
				ep_chipid, len, P2U64(p_rc_va), (u64)ret_ep_pa);
			if (0 != _nvtpcie_mem_test(p_rc_va, len)) {
				DBG_ERR("alloc_epmem 3: mem test failed\r\n");
			}
		}
	}

	DBG_DUMP("\r\nnvtpcie_downstream_active\r\n");
	{
		nvtpcie_chipid_t ep_chipid;
		int is_active;

		ep_chipid = CHIP_EP0;
		is_active = nvtpcie_downstream_active(ep_chipid);
		DBG_DUMP("ep_chipid %d, is_active %d\r\n", ep_chipid, is_active);
	}

	DBG_DUMP("\r\nnvtpcie_edma_copy test\r\n");
	{
		nvtpcie_chipid_t src_chipid;
		phys_addr_t src_pa;
		nvtpcie_chipid_t dst_chipid;
		phys_addr_t dst_pa;
		unsigned long len = 0x10000; //dma max 4GB (636)

		src_chipid = CHIP_RC;
		src_pa = TEST_PA_LOCAL_1;
		dst_chipid = CHIP_EP0;
		dst_pa = TEST_PA_LOCAL_1;
		DBG_DUMP("edma_copy: chip %d pa 0x%016llX => chip %d pa 0x%016llX\r\n",
			src_chipid, (u64)src_pa, dst_chipid, (u64)dst_pa);
		if (0 != nvtpcie_edma_copy(src_chipid, src_pa, dst_chipid, dst_pa, len)) {
			DBG_ERR("nvtpcie_edma_copy 1 failed\r\n");
		}

		src_chipid = CHIP_EP0;
		src_pa = TEST_PA_LOCAL_1;
		dst_chipid = CHIP_RC;
		dst_pa = TEST_PA_LOCAL_1;
		DBG_DUMP("edma_copy: chip %d pa 0x%016llX => chip %d pa 0x%016llX\r\n",
			src_chipid, (u64)src_pa, dst_chipid, (u64)dst_pa);
		if (0 != nvtpcie_edma_copy(src_chipid, src_pa, dst_chipid, dst_pa, len)) {
			DBG_ERR("nvtpcie_edma_copy 2 failed\r\n");
		}
		DBG_DUMP("edma test done\r\n");
	}
}

void nvtpcie_test_shmblk(void)
{
	#define TEST_SHMBLK_NAME "test_shmblk"

	typedef struct {
		unsigned long addr;
		unsigned long size;
		unsigned int count;
	} my_shmblk_test_t;

	my_shmblk_test_t* p_shm_va;

	DBG_DUMP("before set shmblk\r\n");
	_nvtpcie_shmem_dump_shmblk();

	p_shm_va = nvtpcie_shmblk_get(TEST_SHMBLK_NAME, sizeof(my_shmblk_test_t));
	if (NULL == p_shm_va) {
		DBG_ERR("shmblk_get failed\r\n");
		return;
	}

	p_shm_va->addr = 0x12345678;
	p_shm_va->size = 0x87654321;
	p_shm_va->count++;

	DBG_DUMP("shmblk[%s] addr = 0x%lx\r\n", TEST_SHMBLK_NAME, p_shm_va->addr);
	DBG_DUMP("shmblk[%s] size = 0x%lx\r\n", TEST_SHMBLK_NAME, p_shm_va->size);
	DBG_DUMP("shmblk[%s] count = %d\r\n", TEST_SHMBLK_NAME, p_shm_va->count);

	DBG_DUMP("after set shmblk\r\n");
	_nvtpcie_shmem_dump_shmblk();

/*
	nvtpcie_shmblk_free(TEST_SHMBLK_NAME);

	DBG_DUMP("after free shmblk\r\n");
	_nvtpcie_shmem_dump_shmblk();
*/
}
