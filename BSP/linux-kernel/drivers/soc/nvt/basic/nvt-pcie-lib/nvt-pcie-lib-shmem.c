#include <linux/delay.h> //msleep
#include <linux/kernel.h> //align macro
#include <linux/of.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/semaphore.h>
#include <linux/dma-mapping.h>
#include <linux/soc/nvt/fmem.h>

#include "nvt-pcie-lib-internal.h"

typedef struct {
	int is_valid;
	void* fmem_hdl;
	phys_addr_t rcshm_pa; //physical address of RC shared memory
	phys_addr_t local_pa; //physical address of local memory (RC or EPn)
	void* local_va; //local virtual address which is mapped from local_pa
	u64 total_size; //total_size = dts_size + sys_size
	u64 dts_size; //rc user size defined by the dts node
	u32 sys_size; //add additional size for system info
} nvtpcie_shm_t;

typedef struct {
	phys_addr_t pa; //reg pa base (should read from dts)
	unsigned long cc_size; //reg size (should read from dts)
} nvtpcie_ccreg_t;

static DEFINE_SEMAPHORE(sem_shmblk);
static nvtpcie_shm_t g_shm = {0};
static nvtpcie_ccreg_t g_ccreg = {0};

static void _ccreg_write32(nvtpcie_chipid_t tar_chipid, u32 value, u8 offset)
{
	const phys_addr_t tar_pa = g_ccreg.pa;
	phys_addr_t mapped_pa;
	void* va;
	nvtpcie_chipid_t loc_chipid;

	DBG_IND("chip %d, value 0x%X, pa (0x%llX + 0x%X)\r\n", tar_chipid, value, (u64)tar_pa, offset);

	if (!IS_ALIGNED(offset, sizeof(u32))) {
		DBG_ERR("offset %d not aligned to %ld\r\n", offset, (unsigned long)sizeof(u32));
		return;
	}

	if (offset > g_ccreg.cc_size) {
		DBG_ERR("offset %d > max %ld\r\n", offset, g_ccreg.cc_size);
		return;
	}

	loc_chipid = nvtpcie_get_my_chipid();
	if (loc_chipid >= CHIP_MAX) {
		DBG_ERR("get chipid failed\r\n");
		return;
	}

	mapped_pa = nvtpcie_get_mapped_pa(loc_chipid, tar_chipid, tar_pa);
	if (NVTPCIE_INVALID_PA == mapped_pa) {
		DBG_ERR("get mapped pa failed\r\n");
		return;
	}

	va = ioremap(mapped_pa, PAGE_SIZE);
	if (NULL == va) {
		DBG_ERR("ioremap 0x%08llX failed\r\n", (u64)mapped_pa);
		return;
	}

	writel(value, va + offset);

	iounmap(va);
}

static u32 _ccreg_read32(nvtpcie_chipid_t tar_chipid, u8 offset)
{
	const phys_addr_t tar_pa = g_ccreg.pa;
	phys_addr_t mapped_pa;
	void* va;
	nvtpcie_chipid_t loc_chipid;
	u32 ret_val;

	DBG_IND("chip %d, tar_pa (0x%llX + 0x%X)\r\n", tar_chipid, (u64)tar_pa, offset);

	if (!IS_ALIGNED(offset, sizeof(u32))) {
		DBG_ERR("offset %d not aligned to %ld\r\n", offset, (unsigned long)sizeof(u32));
		return 0;
	}

	if (offset > g_ccreg.cc_size) {
		DBG_ERR("offset %d > max %ld\r\n", offset, g_ccreg.cc_size);
		return 0;
	}

	loc_chipid = nvtpcie_get_my_chipid();
	if (loc_chipid >= CHIP_MAX) {
		DBG_ERR("get chipid failed\r\n");
		return 0;
	}

	mapped_pa = nvtpcie_get_mapped_pa(loc_chipid, tar_chipid, tar_pa);
	if (NVTPCIE_INVALID_PA == mapped_pa) {
		DBG_ERR("get mapped pa failed\r\n");
		return 0;
	}

	va = ioremap(mapped_pa, PAGE_SIZE);
	if (NULL == va) {
		DBG_ERR("ioremap 0x%08llX failed\r\n", (u64)mapped_pa);
		return 0;
	}

	ret_val = readl(va + offset);

	iounmap(va);

	DBG_IND("ret_val 0x%X, offset %d\r\n", ret_val, offset);

	return ret_val;
}

static u32 _get_chksum(phys_addr_t pa, u64 dts_size, u32 sys_size)
{
	u32 sum = 0;

	sum += upper_32_bits(pa);
	sum += lower_32_bits(pa);
	sum += upper_32_bits(dts_size);
	sum += lower_32_bits(dts_size);
	sum += sys_size;

	return sum;
}

static void _ccreg_set_shminfo(phys_addr_t pa, u64 dts_size, u32 sys_size)
{
	nvtpcie_chipid_t chipid;
	int ep_count;

	ep_count = nvtpcie_get_ep_count();

	for (chipid = CHIP_EP0; chipid < (CHIP_EP0 + ep_count); chipid++) {
		_ccreg_write32(chipid, upper_32_bits(pa), OFFSET_SHM_ADDR_H);
		_ccreg_write32(chipid, lower_32_bits(pa), OFFSET_SHM_ADDR_L);
		_ccreg_write32(chipid, upper_32_bits(dts_size), OFFSET_SHM_DTS_SIZE_H);
		_ccreg_write32(chipid, lower_32_bits(dts_size), OFFSET_SHM_DTS_SIZE_L);
		_ccreg_write32(chipid, sys_size, OFFSET_SHM_SYS_SIZE);
		_ccreg_write32(chipid, _get_chksum(pa, dts_size, sys_size), OFFSET_SHM_CHKSUM);
		_ccreg_write32(chipid, 1, OFFSET_SHM_VALID);
	}
}

static void _ccreg_clear_shminfo(void)
{
	nvtpcie_chipid_t chipid;
	int ep_count;

	ep_count = nvtpcie_get_ep_count();

	for (chipid = CHIP_EP0; chipid < (CHIP_EP0 + ep_count); chipid++) {
		_ccreg_write32(chipid, 0, OFFSET_SHM_VALID);
		_ccreg_write32(chipid, 0, OFFSET_SHM_CHKSUM);
		_ccreg_write32(chipid, 0, OFFSET_SHM_ADDR_H);
		_ccreg_write32(chipid, 0, OFFSET_SHM_ADDR_L);
		_ccreg_write32(chipid, 0, OFFSET_SHM_DTS_SIZE_H);
		_ccreg_write32(chipid, 0, OFFSET_SHM_DTS_SIZE_L);
		_ccreg_write32(chipid, 0, OFFSET_SHM_SYS_SIZE);
	}
}

static int _ccreg_get_shminfo(phys_addr_t *out_pa, u64 *out_dts_size, u32 *out_sys_size)
{
	nvtpcie_chipid_t chipid;
	phys_addr_t pa;
	u64 dts_size;
	u32 sys_size;
	u32 tmp_u32[2];
	u32 chksum;
	u32 shm_valid = 0;
	int retry_cnt = 0;

	chipid = nvtpcie_get_my_chipid();
	if (chipid >= CHIP_MAX) {
		DBG_ERR("get chipid failed\r\n");
		return -1;
	}

	//read rcshm_pa and size from cc_cmd reg
	do {
		shm_valid = _ccreg_read32(chipid, OFFSET_SHM_VALID);
		if (shm_valid) {
			break;
		}

		if (0 == (retry_cnt % 10)) {
			DBG_INFO("wait rc shm valid\r\n");
		}
		msleep(100);
	} while (++retry_cnt < 300);

	if (0 == shm_valid) {
		DBG_ERR("shm not valid\r\n");
		return -1;
	}

	tmp_u32[0] = _ccreg_read32(chipid, OFFSET_SHM_ADDR_H);
	tmp_u32[1] = _ccreg_read32(chipid, OFFSET_SHM_ADDR_L);
	pa = (phys_addr_t)HL32_TO_U64(tmp_u32[0], tmp_u32[1]);

	tmp_u32[0] = _ccreg_read32(chipid, OFFSET_SHM_DTS_SIZE_H);
	tmp_u32[1] = _ccreg_read32(chipid, OFFSET_SHM_DTS_SIZE_L);
	dts_size = HL32_TO_U64(tmp_u32[0], tmp_u32[1]);

	sys_size = _ccreg_read32(chipid, OFFSET_SHM_SYS_SIZE);

	chksum = _get_chksum(pa, dts_size, sys_size);
	if (chksum != _ccreg_read32(chipid, OFFSET_SHM_CHKSUM)) {
		DBG_ERR("shm chksum not valid\r\n");
		return -1;
	}

	*out_pa = pa;
	*out_dts_size = dts_size;
	*out_sys_size = sys_size;

	return 0;
}

static int _ccreg_dts_parsing(void)
{
	struct device_node *p_node = NULL;
	u64 u64arr[2];

	p_node = of_find_node_by_path(CC_NODE_PATH);
	if (NULL == p_node) {
		DBG_INFO("no node %s\r\n", CC_NODE_PATH);
		return -1;
	}

	if (0 != of_property_read_u64_array(p_node, "reg", &u64arr[0], ARRAY_SIZE(u64arr))) {
		DBG_ERR("read reg failed\r\n");
		of_node_put(p_node);
		return -1;
	}

	g_ccreg.pa = (phys_addr_t)u64arr[0];
	g_ccreg.cc_size = (unsigned long)u64arr[1];

	DBG_IND("my cc reg pa 0x%llX, size 0x%lX\r\n", (u64)g_ccreg.pa, g_ccreg.cc_size);
	of_node_put(p_node);
	return 0;
}

int _nvtpcie_shmem_udpate_sysinfo(void)
{
	if (!g_shm.is_valid) {
		return 0; //skip if not inited
	}

	if (0 != _nvtpcie_sysinfo_set_map(g_shm.local_va + g_shm.dts_size)) {
		DBG_ERR("sysinfo set failed\r\n");
		return -1;
	}

	return 0;
}

void _nvtpcie_shmem_dump_sysinfo(void)
{
	if (!g_shm.is_valid) {
		DBG_ERR("shmem not valid\r\n");
		return;
	}

	DBG_WRN("sys va 0x%llx, sys_size 0x%x\r\n", (u64)(g_shm.local_va + g_shm.dts_size), g_shm.sys_size);

	nvtpcie_shmem_dump(g_shm.dts_size, g_shm.sys_size);
}

void _nvtpcie_shmem_dump_shmblk(void)
{
	nvtpcie_sysinfo_t *p_sysinfo;
	nvtpcie_shmblk_t *p_shmblk;
	int idx;

	BUILD_BUG_ON(!IS_ALIGNED(sizeof(nvtpcie_shmblk_t), 32));

	p_sysinfo = _nvtpcie_shmem_get_sysinfo_va();
	if (NULL == p_sysinfo) {
		DBG_ERR("get_sysinfo_va failed\r\n");
		return;
	}
	p_shmblk = &p_sysinfo->shmblk[0];

	DBG_DUMP("\r\nshmblk\r\n");

	for (idx = 0; idx < SHMBLK_NUM; idx++, p_shmblk++) {
		DBG_DUMP("[%02d] \"%s\", 0x%08X\r\n",
			idx, p_shmblk->name, p_shmblk->size);
	}
}

void* _nvtpcie_shmem_get_sysinfo_va(void)
{
	if (!g_shm.is_valid) {
		DBG_ERR("shmem not valid\r\n");
		return NULL;
	}

	return g_shm.local_va + g_shm.dts_size;
}

int nvtpcie_shmem_init(u64 dts_size)
{
	nvtpcie_chipid_t chipid;

	chipid = nvtpcie_get_my_chipid();
	if (chipid >= CHIP_MAX) {
		DBG_ERR("get chipid failed\r\n");
		return -1;
	}

	if (0 != _ccreg_dts_parsing()) {
		return -1;
	}

	//set pa to be mapped
	if (CHIP_RC == chipid) { //I am RC
		struct nvt_fmem_mem_info_t info = {0};
		phys_addr_t ep_local_pa; //EP local pa which mapped to rcshm_pa

		g_shm.dts_size = dts_size;
		g_shm.sys_size = _nvtpcie_sysinfo_size();
		g_shm.total_size = g_shm.dts_size + g_shm.sys_size;
		/*
		 * |----------|
		 * | dts_size |\
		 * |----------|-) total shared memory size
		 * | sys_size |/
		 * |----------|
		 */

		if (0 != nvt_fmem_mem_info_init(&info, NVT_FMEM_ALLOC_NONCACHE, (size_t)g_shm.total_size, NULL)) {
			DBG_ERR("fmem into init failed\r\n");
			return -1;
		}

		g_shm.fmem_hdl = fmem_alloc_from_ker(&info);
		if (NULL == g_shm.fmem_hdl) {
			DBG_ERR("alloc shmem %llx bytes failed\n", g_shm.total_size);
			return -1;
		}
		g_shm.rcshm_pa = info.paddr;
		g_shm.local_pa = info.paddr;
		g_shm.local_va = info.vaddr;
		g_shm.is_valid = 1;

		if (0 != _nvtpcie_sysinfo_set_map(_nvtpcie_shmem_get_sysinfo_va())) {
			DBG_ERR("set map failed\r\n");
		}

		//Assume all EPs use the same upstream table
		ep_local_pa =  nvtpcie_get_upstream_pa(CHIP_EP_COMMON, g_shm.rcshm_pa);
		if (NVTPCIE_INVALID_PA == g_shm.local_pa) {
			DBG_ERR("get local_pa failed\r\n");
			return -1;
		}

		//write ep_pa_to_rc and size to cc_cmd reg
		_ccreg_set_shminfo(ep_local_pa, g_shm.dts_size, g_shm.sys_size);

	} else { //I am EP
		void *p_sysinfo_va;

		//read rcshm_pa and size from cc_cmd reg
		if (0 != _ccreg_get_shminfo(&g_shm.local_pa, &g_shm.dts_size, &g_shm.sys_size)) {
			DBG_ERR("get_shminfo failed\r\n");
			return -1;
		}
		g_shm.total_size = g_shm.dts_size + g_shm.sys_size;

		//mapping
		g_shm.local_va = ioremap(g_shm.local_pa, g_shm.total_size);
		if (NULL == g_shm.local_va) {
			DBG_ERR("ioremap failed\r\n");
			return -1;
		}
		p_sysinfo_va = g_shm.local_va + g_shm.dts_size;

		if (0 != _nvtpcie_sysinfo_get_map(p_sysinfo_va, chipid)) {
			DBG_ERR("atu buf drain failed\r\n");
		}

		g_shm.rcshm_pa = nvtpcie_get_unmapped_pa(chipid, g_shm.local_pa, NULL);
		if (NVTPCIE_INVALID_PA == g_shm.rcshm_pa) {
			DBG_ERR("unmapped local_pa failed\r\n");
		}

		g_shm.fmem_hdl = NULL; //not valid for EP
		g_shm.is_valid = 1;
	}

	DBG_INFO("mychipid %d, rcshm_pa 0x%08llX, local_pa 0x%08llX, local_va 0x%08llX, dts_size 0x%llX\r\n",
		chipid, (u64)g_shm.rcshm_pa, (u64)g_shm.local_pa, P2U64(g_shm.local_va), g_shm.dts_size);

	return 0;
}

int nvtpcie_shmem_exit(void)
{
	if (!g_shm.is_valid) {
		return 0;
	}

	if (g_shm.local_va) {
		if (g_shm.fmem_hdl) { //I am RC
			_ccreg_clear_shminfo();
			if (0 != fmem_release_from_ker(g_shm.fmem_hdl)) {
				DBG_ERR("rel 0x%08llX from ker failed\r\n", P2U64(g_shm.local_va));
				return -1;
			}
		} else { //I am EP
			iounmap(g_shm.local_va);
		}
		g_shm.local_va = NULL;
	}

	g_shm.is_valid = 0;

	return 0;
}

void *nvtpcie_shmem_get_va(void)
{
	if (!g_shm.is_valid) {
		DBG_ERR("shmem not valid\r\n");
		return NULL;
	}

	return g_shm.local_va;
}
EXPORT_SYMBOL(nvtpcie_shmem_get_va);

phys_addr_t nvtpcie_shmem_get_pa(void)
{
	if (!g_shm.is_valid) {
		DBG_ERR("shmem not valid\r\n");
		return NVTPCIE_INVALID_PA;
	}

	return g_shm.local_pa;
}
EXPORT_SYMBOL(nvtpcie_shmem_get_pa);

unsigned long nvtpcie_shmem_get_size(void)
{
	if (!g_shm.is_valid) {
		DBG_ERR("shmem not valid\r\n");
		return 0;
	}

	//Note: return dts_size, because sysinfo is not known by users
	return g_shm.dts_size;
}
EXPORT_SYMBOL(nvtpcie_shmem_get_size);

void nvtpcie_shmem_dump(unsigned long begin_offs, unsigned long length)
{
	unsigned long pa;
	unsigned long offs;
	unsigned int str_len;
	unsigned int cnt;
	char str_dumpmem[128];
	u32 u32array[4];
	u32 *p_u32;
	char *p_char;

	if (!g_shm.is_valid) {
		DBG_ERR("shmem not valid\r\n");
		return;
	}

	begin_offs = ALIGN_DOWN(begin_offs, sizeof(u32));

	if (0 == length || (length + begin_offs) > g_shm.total_size) {
		DBG_DUMP("Invalid length %ld, max 0x%llX\r\n", length, g_shm.total_size);
		return;
	}

	pa = g_shm.local_pa + begin_offs;
	p_u32 = g_shm.local_va + begin_offs;

	DBG_DUMP("\r\ndump va=%08lX, pa=%08lX length=%08lX to console:\r\n", (unsigned long)p_u32, pa, length);

	for (offs = 0; offs < length; offs += sizeof(u32array)) {
		u32array[0] = *p_u32++;
		u32array[1] = *p_u32++;
		u32array[2] = *p_u32++;
		u32array[3] = *p_u32++;

		str_len = snprintf(str_dumpmem, sizeof(str_dumpmem), "%08lX : %08X %08X %08X %08X  ",
			(pa + offs), u32array[0], u32array[1], u32array[2], u32array[3]);

		p_char = (char *)&u32array[0];
		for (cnt = 0; cnt < sizeof(u32array); cnt++, p_char++) {
			if (*p_char < 0x20 || *p_char >= 0x80)
				str_len += snprintf(str_dumpmem+str_len, 64-str_len, ".");
			else
				str_len += snprintf(str_dumpmem+str_len, 64-str_len, "%c", *p_char);
		}

		DBG_DUMP("%s\r\n", str_dumpmem);
	}
	DBG_DUMP("\r\n\r\n");
}
EXPORT_SYMBOL(nvtpcie_shmem_dump);

//return shmblk va
void* nvtpcie_shmblk_get(const char *name, unsigned int size)
{
	nvtpcie_sysinfo_t *p_sysinfo;
	nvtpcie_shmblk_t *p_shmblk;
	int idx;

	if (NULL == name) {
		DBG_ERR("name is NULL\r\n");
		return NULL;
	}

	if (0 == size) {
		DBG_ERR("size is 0\r\n");
		return NULL;
	}

	if (size > SHMBLK_BUF_SIZE) {
		//Note:
		//The shmblk buffer is not designed to exchange big data
		//It is designed to put some shared info only, e.g. address and size.
		DBG_ERR("size %d > max %d\r\n", size, SHMBLK_BUF_SIZE);
		return NULL;
	}

	p_sysinfo = _nvtpcie_shmem_get_sysinfo_va();
	if (NULL == p_sysinfo) {
		DBG_ERR("get_sysinfo_va failed\r\n");
		return NULL;
	}

	down(&sem_shmblk);

	//check the same name
	for (idx = 0; idx < SHMBLK_NUM; idx++) {
		p_shmblk = &p_sysinfo->shmblk[idx];

		if (0 == p_shmblk->size)
			continue; //skip empty

		if (strncmp(p_shmblk->name, name, SHMBLK_NAME_SIZE))
			continue; //skip non-matched name

		//found the same name
		if ((p_shmblk->head_tag != SHMBLK_TAG) ||
			(p_shmblk->tail_tag != SHMBLK_TAG)) {
			DBG_ERR("tag is damaged, head_tag 0x%X, tail_tag 0x%X\r\n",	p_shmblk->head_tag, p_shmblk->tail_tag);
			up(&sem_shmblk);
			return NULL;
		}

		if (size != p_shmblk->size) {
			DBG_ERR("shmblk [%s] new size(%d) != old size(%d)\r\n", p_shmblk->name, size, p_shmblk->size);
			up(&sem_shmblk);
			return NULL;
		}

		DBG_IND("shmblk alloc [%s] exist\r\n", p_shmblk->name);
		up(&sem_shmblk);
		return &p_shmblk->buf[0];
	}

	//find an empty one
	for (idx = 0; idx < SHMBLK_NUM; idx++) {
		p_shmblk = &p_sysinfo->shmblk[idx];

		if (0 != p_shmblk->size) {
			//skip non-empty
			continue;
		}

		p_shmblk->head_tag = SHMBLK_TAG;
		p_shmblk->tail_tag = SHMBLK_TAG;
		p_shmblk->size = size;
		strncpy(p_shmblk->name, name, SHMBLK_NAME_SIZE - 1);
		p_shmblk->name[SHMBLK_NAME_SIZE - 1] = '\0';

		DBG_IND("shmblk alloc [%s] new\r\n", p_shmblk->name);
		up(&sem_shmblk);
		return &p_shmblk->buf[0];
	}

	DBG_ERR("shmblk is full\r\n");
	return NULL;
}
EXPORT_SYMBOL(nvtpcie_shmblk_get);

void nvtpcie_shmblk_release(const char *name)
{
	nvtpcie_sysinfo_t *p_sysinfo;
	nvtpcie_shmblk_t *p_shmblk;
	int idx;

	if (NULL == name) {
		DBG_ERR("name is NULL\r\n");
		return;
	}

	p_sysinfo = _nvtpcie_shmem_get_sysinfo_va();
	if (NULL == p_sysinfo) {
		DBG_ERR("get_sysinfo_va failed\r\n");
		return;
	}

	down(&sem_shmblk);

	for (idx = 0; idx < SHMBLK_NUM; idx++) {
		p_shmblk = &p_sysinfo->shmblk[idx];

		if (0 == p_shmblk->size)
			continue; //skip empty

		if (strncmp(p_shmblk->name, name, SHMBLK_NAME_SIZE))
			continue; //skip non-matched name

		//match here

		if ((p_shmblk->head_tag != SHMBLK_TAG) ||
			(p_shmblk->tail_tag != SHMBLK_TAG)) {
			DBG_ERR("shmblk [%s] is damaged, head_tag 0x%X, tail_tag 0x%X\r\n",
				p_shmblk->name, p_shmblk->head_tag, p_shmblk->tail_tag);
			break;
		}

		DBG_INFO("shmblk free [%s]\r\n", p_shmblk->name);
		memset(p_shmblk, 0, sizeof(nvtpcie_shmblk_t));
		break;
	}

	up(&sem_shmblk);
}
EXPORT_SYMBOL(nvtpcie_shmblk_release);
