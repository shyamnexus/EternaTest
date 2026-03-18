/**
    NVT PCIe memory management
    To handle RC and EP device address translation
    @file       nvt-pcie-lib.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/build_bug.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/semaphore.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/soc/nvt/nvtmem.h>

#include <linux/soc/nvt/nvt-pcie-edma.h>

#include "nvt-pcie-lib-internal.h"

typedef struct {
	nvtpcie_ddrid_t ddrid;
	nvtpcie_chipid_t chipid;
	int ddridx;
	phys_addr_t chip_base;
	unsigned long size;
} nvtpcie_ddr_map_t;

typedef struct {
	nvtpcie_chipid_t    ep_chipid;
	phys_addr_t         ep_local_pa;
	phys_addr_t         ep_free_pa;
	unsigned long       size;
	phys_addr_t         rc_map_pa;
	void*               rc_map_va;
	void*               rc_free_va;
	unsigned long       free_size;
} nvtpcie_ep_allocmem_t;

// local variables ------
static DEFINE_SEMAPHORE(sem_nvtpcie);

static nvtpcie_rc_downmap_t g_rc_downmap[MAX_EP_CNT * ATU_DOWNMAP_COUNT] = {0};
/*   HHHHHHHHLLLLLLLL(RC)HHHHHHHHLLLLLLLL                           0xHHHHHHHHLLLLLLLL(EP) */
//{0x0000000400000000, 0x0000000010000000, TYPE_NAME_MAU, CHIP_EP0, 0x0000000040000000, NULL},
//{0x0000000410000000, 0x0000000010000000, TYPE_NAME_MAU, CHIP_EP0, 0x0000000060000000, NULL},
//{0x0000000420000000, 0x0000000010000000, TYPE_NAME_MAU, CHIP_EP0, 0x0000000090000000, NULL},
//{0x0000000620000000, 0x0000000010000000, TYPE_NAME_APB, CHIP_EP0, 0x00000002F0000000, NULL},
//};

//Note: Assume all EPs use the same upstream table
static nvtpcie_ep_upmap_t g_ep_upmap[MAX_EP_CNT * ATU_UPMAP_COUNT] = {0};
/*   HHHHHHHHLLLLLLLL(EP)HHHHHHHHLLLLLLLL                           0xHHHHHHHHLLLLLLLL(RC) */
//{0x0000000C00000000, 0x0000000200000000, TYPE_NAME_MAU, CHIP_EP0, 0x0000000000000000},
//{0x0000000E00000000, 0x0000000010000000, TYPE_NAME_APB, CHIP_EP0, 0x00000002F0000000},
//};

static nvtpcie_ep_allocmem_t g_ep_allocmem[DDR_ID_MAX] = {};
static nvtpcie_ddr_map_t g_ddr_map[DDR_ID_MAX] = {0};
static u64 g_rcshm_size = 0;
static nvtpcie_chipid_t g_my_chipid = CHIP_MAX;

#if IS_ENABLED(CONFIG_PCI)
static struct nvt_ep_dev_info_notifier g_ep_info_notifier = {0};
#endif

static nvtpcie_ep_allocmem_t* _nvtpcie_get_ep_allocmem(nvtpcie_chipid_t ep_chipid)
{
	int idx;

	for (idx = 0; idx < ARRAY_SIZE(g_ep_allocmem); idx++) {
		if (g_ep_allocmem[idx].ep_chipid == ep_chipid) {
			return &g_ep_allocmem[idx];
		}
	}
	return NULL;
}

static int _nvtpcie_ddr_map_init(void)
{
	nvtmem_ddrinfo_t info = {0};
	int ddr_cnt[CHIP_MAX] = {0};
	int idx;
	int num;
	struct membank_s *p_info;
	nvtpcie_ddr_map_t *p_map;

	nvtmem_get_ddrinfo(&info);

	num = info.nr_banks;
	if (num > ARRAY_SIZE(g_ddr_map)) {
		num = ARRAY_SIZE(g_ddr_map);
		DBG_WRN("nr_banks %d > %ld\r\n", info.nr_banks, (unsigned long)ARRAY_SIZE(g_ddr_map));
	}

	for (idx = 0; idx < num; idx++) {
		p_info = &info.bank[idx];
		p_map = &g_ddr_map[idx];

		p_map->ddrid = p_info->ddrid;
		p_map->chipid = p_info->chip;
		p_map->ddridx = ddr_cnt[p_info->chip];
		p_map->chip_base = p_info->start;
		p_map->size = p_info->size;

		ddr_cnt[p_info->chip]++;
	}

	return 0;
}

unsigned long _nvtpcie_sysinfo_size(void)
{
	BUILD_BUG_ON(sizeof_field(nvtpcie_sysinfo_t, rc_downmap) != sizeof(g_rc_downmap));
	BUILD_BUG_ON(sizeof_field(nvtpcie_sysinfo_t, ep_upmap) != sizeof(g_ep_upmap));

	return sizeof(nvtpcie_sysinfo_t);
}

int _nvtpcie_sysinfo_set_map(void *p_iobuf)
{
	nvtpcie_sysinfo_t *p_sysinfo;

	if (NULL == p_iobuf) {
		DBG_ERR("p_iobuf is NULL\r\n");
		return -1;
	}
	p_sysinfo = p_iobuf;

	memcpy_toio(&p_sysinfo->rc_downmap[0], &g_rc_downmap[0], sizeof(g_rc_downmap));
	memcpy_toio(&p_sysinfo->ep_upmap[0], &g_ep_upmap[0], sizeof(g_ep_upmap));

	return 0;
}

int _nvtpcie_sysinfo_get_map(void *p_iobuf, nvtpcie_chipid_t ep_chipid)
{
	nvtpcie_sysinfo_t *p_sysinfo;

	if (NULL == p_iobuf) {
		DBG_ERR("p_iobuf is NULL\r\n");
		return -1;
	}
	p_sysinfo = p_iobuf;

	memcpy_fromio(&g_rc_downmap[0], &p_sysinfo->rc_downmap[0], sizeof(g_rc_downmap));
	memcpy_fromio(&g_ep_upmap[0], &p_sysinfo->ep_upmap[0], sizeof(g_ep_upmap));

	return 0;
}

#if IS_ENABLED(CONFIG_NVT_NT98336_PCI_EP_EVB)
int _nvtpcie_atu_board_init(void)
{
	u64 sys_addr, bus_addr, limit_addr, size;
	void __iomem *atu_base;
	u32 low32, high32;
	int rgn_idx = 0;
	unsigned int is_enable;

	nvtpcie_ep_upmap_t *p_upmap = &g_ep_upmap[0];

	DBG_INFO("parsing atu reg\r\n");

	atu_base = ioremap(NVT_PCIE_DBI_BASE_PHYS + PCIE_GET_ATU_OUTB_UNR_REG_OFFSET(rgn_idx), PAGE_SIZE);
	if (NULL == atu_base) {
		DBG_ERR("ioremap NVT_PCIE_DBI_BASE_PHYS rgn_idx %d failed\r\n", rgn_idx);
		return -1;
	}

	is_enable = (0 != (readl(atu_base + PCIE_ATU_UNR_REGION_CTRL2) & PCIE_ATU_ENABLE));
	if (is_enable) {
		low32 = readl(atu_base + PCIE_ATU_UNR_LOWER_BASE);
		high32 = readl(atu_base + PCIE_ATU_UNR_UPPER_BASE);
		sys_addr = (u64)low32 | ((u64)high32 << 32);

		low32 = readl(atu_base + PCIE_ATU_UNR_LIMIT);
		high32 = readl(atu_base + PCIE_ATU_UNR_UPPER_LIMIT);
		limit_addr = (u64)low32 | ((u64)high32 << 32);
		size = limit_addr - sys_addr + 1;

		low32 = readl(atu_base + PCIE_ATU_UNR_LOWER_TARGET);
		high32 = readl(atu_base + PCIE_ATU_UNR_UPPER_TARGET);
		bus_addr = (u64)low32 | ((u64)high32 << 32);

		DBG_INFO("region[%d] bus_addr 0x%llx, sys_addr 0x%llx, size 0x%llx, is_enable %d\r\n",
			rgn_idx, bus_addr, sys_addr, size, is_enable);

		//assume MAU only and region 0 only
		p_upmap->local_pa = sys_addr;
		p_upmap->size = size;
		NVTPCIE_STRNCPY(p_upmap->name, TYPE_NAME_MAU, sizeof(TYPE_NAME_MAU));
		p_upmap->ep_chipid = CHIP_EP_COMMON;
		p_upmap->rc_pa = CHIP_MAU_BASE;
		p_upmap++;
	}

	iounmap(atu_base);

	return 0;
}
#elif IS_ENABLED(CONFIG_PCI)
#if 0 //debug
static void _nvtpcie_dump_ep_engine(struct pci_ep_engine *ep)
{
	DBG_DUMP("ep_id %d\r\n", ep->ep_id);

	DBG_DUMP("pci_device_is_present %d\r\n", pci_device_is_present(ep->pci_dev));

	DBG_DUMP("pci_dev->bus->number = 0x%x\r\n", ep->pci_dev->bus->number);
	DBG_DUMP("pci_dev->devfn = 0x%x\r\n", ep->pci_dev->devfn);

	DBG_DUMP("ep in mmio start 0x%lx\r\n", ep->ep_inb_info.mmio[NVT_PCIE_ADDR_SYS_START]);
	DBG_DUMP("ep in mmio len 0x%lx\r\n", ep->ep_inb_info.mmio[NVT_PCIE_ADDR_SYS_LEN]);

	DBG_DUMP("ep out mmio start 0x%lx\r\n", ep->ep_outb_info.mmio[NVT_PCIE_ADDR_SYS_START]);
	DBG_DUMP("ep out mmio len 0x%lx\r\n", ep->ep_outb_info.mmio[NVT_PCIE_ADDR_SYS_LEN]);

	DBG_DUMP("ep out apb start 0x%lx\r\n", ep->ep_outb_info.apb[NVT_PCIE_ADDR_SYS_START]);
	DBG_DUMP("ep out apb len 0x%lx\r\n", ep->ep_outb_info.apb[NVT_PCIE_ADDR_SYS_LEN]);

	DBG_DUMP("rc out mmio start 0x%lx\r\n", ep->rc_outb_info.mmio[NVT_PCIE_ADDR_SYS_START]);
	DBG_DUMP("rc out mmio len 0x%lx\r\n", ep->rc_outb_info.mmio[NVT_PCIE_ADDR_SYS_LEN]);

	DBG_DUMP("rc out apb start 0x%lx\r\n", ep->rc_outb_info.apb[NVT_PCIE_ADDR_SYS_START]);
	DBG_DUMP("rc out apb len 0x%lx\r\n", ep->rc_outb_info.apb[NVT_PCIE_ADDR_SYS_LEN]);
}
#endif

static int _nvtpcie_get_ep_info(struct pci_ep_engine *ep)
{
	nvtpcie_rc_downmap_t *p_downmap = &g_rc_downmap[0];
	nvtpcie_ep_upmap_t *p_upmap = &g_ep_upmap[0];
	int rgn_idx; //ATU region index
	unsigned long rc_outb_mmio_base;
	unsigned long rc_outb_mmio_max;
	unsigned long rc_outb_bus_base;
	unsigned long ep_inb_mmio_sum = 0;

	if (NULL == ep) {
		DBG_ERR("ep list is empty\r\n");
		return -1;
	}

	if (ep->ep_idx >= MAX_EP_CNT) {
		DBG_ERR("Invalid ep_idx %d\r\n", ep->ep_idx);
		return -1;
	}

	//setup downmap
	p_downmap = &g_rc_downmap[ep->ep_idx * ATU_DOWNMAP_COUNT];
	memset(p_downmap, 0, sizeof(nvtpcie_rc_downmap_t)*ATU_DOWNMAP_COUNT);

	DBG_IND("rc_outb_info.mmio[NVT_PCIE_ADDR_SYS_START] = %lx\n", ep->rc_outb_info.mmio[NVT_PCIE_ADDR_SYS_START]);
	DBG_IND("rc_outb_info.mmio[NVT_PCIE_ADDR_SYS_LEN] = %lx\n", ep->rc_outb_info.mmio[NVT_PCIE_ADDR_SYS_LEN]);
	DBG_IND("rc_outb_info.mmio[NVT_PCIE_ADDR_BUS] = %lx\n", ep->rc_outb_info.mmio[NVT_PCIE_ADDR_BUS]);

	rc_outb_mmio_base = ep->rc_outb_info.mmio[NVT_PCIE_ADDR_SYS_START];
	rc_outb_mmio_max = ep->rc_outb_info.mmio[NVT_PCIE_ADDR_SYS_LEN];
	rc_outb_bus_base = ep->rc_outb_info.mmio[NVT_PCIE_ADDR_BUS];

	//check the total size of ep_inb_info.mmio
	for (rgn_idx = 0; rgn_idx < ATU_DOWNMAP_COUNT; rgn_idx++) {
		ep_inb_mmio_sum += ep->ep_inb_info.mmio[rgn_idx][NVT_PCIE_ADDR_SYS_LEN];
	}

	if (ep_inb_mmio_sum > rc_outb_mmio_max) {
		DBG_ERR("ep_inb_mmio_sum 0x%lx > rc_outb_mmio_max 0x%lx\r\n", ep_inb_mmio_sum, rc_outb_mmio_max);
		return -1;
	}

	//setup downmap mmio
	for (rgn_idx = 0; rgn_idx < ATU_DOWNMAP_COUNT; rgn_idx++) {
		unsigned long ep_inb_start = ep->ep_inb_info.mmio[rgn_idx][NVT_PCIE_ADDR_SYS_START];
		unsigned long ep_inb_len = ep->ep_inb_info.mmio[rgn_idx][NVT_PCIE_ADDR_SYS_LEN];
		unsigned long ep_inb_bus_offset = ep->ep_inb_info.mmio[rgn_idx][NVT_PCIE_ADDR_BUS] - rc_outb_bus_base;

		if (0 == ep_inb_len) {
			continue;
		}

		DBG_IND("ep_inb_info.mmio[%d][NVT_PCIE_ADDR_SYS_START] = %lx\n", rgn_idx, ep_inb_start);
		DBG_IND("ep_inb_info.mmio[%d][NVT_PCIE_ADDR_SYS_LEN] = %lx\n", rgn_idx, ep_inb_len);

		p_downmap->local_pa = rc_outb_mmio_base + ep_inb_bus_offset;
		p_downmap->size = ep_inb_len;
		NVTPCIE_STRNCPY(p_downmap->name, TYPE_NAME_MAU, sizeof(p_downmap->name));
		p_downmap->ep_chipid = CHIP_EP0 + ep->ep_idx; //ep_id is an index of ep drv
		p_downmap->ep_pa = ep_inb_start;
		p_downmap->ep_pci_dev = ep->pci_dev;
		p_downmap++;
	}

	//setup downmap apb
	p_downmap->local_pa = ep->rc_outb_info.apb[NVT_PCIE_ADDR_SYS_START];
	p_downmap->size = ep->rc_outb_info.apb[NVT_PCIE_ADDR_SYS_LEN];
	NVTPCIE_STRNCPY(p_downmap->name, TYPE_NAME_APB, sizeof(p_downmap->name));
	p_downmap->ep_chipid = CHIP_EP0 + ep->ep_idx; //ep_id is an index of ep drv
	p_downmap->ep_pa = CHIP_APB_BASE;
	p_downmap->ep_pci_dev = ep->pci_dev;

	//setup upmap
	p_upmap = &g_ep_upmap[ep->ep_idx * ATU_UPMAP_COUNT];
	memset(p_upmap, 0, sizeof(nvtpcie_ep_upmap_t)*ATU_UPMAP_COUNT);

	p_upmap->local_pa = ep->ep_outb_info.mmio[NVT_PCIE_ADDR_SYS_START];
	p_upmap->size = ep->ep_outb_info.mmio[NVT_PCIE_ADDR_SYS_LEN];
	NVTPCIE_STRNCPY(p_upmap->name, TYPE_NAME_MAU, sizeof(p_upmap->name));
	p_upmap->ep_chipid = CHIP_EP0 + ep->ep_idx;
	p_upmap->rc_pa = CHIP_MAU_BASE;
	p_upmap++;

	p_upmap->local_pa = ep->ep_outb_info.apb[NVT_PCIE_ADDR_SYS_START];
	p_upmap->size = ep->ep_outb_info.apb[NVT_PCIE_ADDR_SYS_LEN];
	NVTPCIE_STRNCPY(p_upmap->name, TYPE_NAME_APB, sizeof(p_upmap->name));
	p_upmap->ep_chipid = CHIP_EP0 + ep->ep_idx;
	p_upmap->rc_pa = CHIP_APB_BASE;

	_nvtpcie_shmem_udpate_sysinfo();

	return 0;
}

int _nvtpcie_atu_common_init(void)
{
	if (CHIP_RC != nvtpcie_get_my_chipid()) {
		//skip for EPn
		return 0;
	}

	memset(&g_rc_downmap[0], 0x0, sizeof(g_rc_downmap));
	memset(&g_ep_upmap[0], 0x0, sizeof(g_ep_upmap));

	if (0 != nvt_pci_list_for_each_ep_drv_info(_nvtpcie_get_ep_info)) {
		DBG_ERR("list ep failed\r\n");
		return -1;
	}

	g_ep_info_notifier.p_func = _nvtpcie_get_ep_info;
	if (0 != nvt_pci_list_register_notifier(&g_ep_info_notifier)) {
		DBG_ERR("register notifier failed\r\n");
		return -1;
	}

	return 0;
}
#else //#if IS_ENABLED(CONFIG_PCI)
int _nvtpcie_atu_cascade_init(void)
{
	struct nvt_pcie_info *p_info;
	nvt_pcie_phy_map *p_drv_map;
	nvtpcie_rc_downmap_t *p_downmap = &g_rc_downmap[0];
	nvtpcie_ep_upmap_t *p_upmap = &g_ep_upmap[0];

	int chipid;
	int type_idx;
	phys_addr_t remote_pa;
	char *p_name;

	p_info = nvt_pcie_get_info();
	if (NULL == p_info) {
		DBG_IND("no pcie info\r\n");
		return -1;
	}

	BUILD_BUG_ON(CHIP_RC != 0); //to make sure chipid initial value is 0

	for (chipid = CHIP_RC; chipid < CHIP_MAX; chipid++) {
		for (type_idx = 0; type_idx < ATU_MAP_COUNT; type_idx++) {

			p_drv_map = &p_info->pcie_chip_map[chipid][type_idx];
			DBG_IND("chipid%d, type%d, addr 0x%llX, size 0x%lx\r\n",
				chipid, p_drv_map->type, (u64)p_drv_map->addr, (unsigned long)p_drv_map->size);

			if (0 == p_drv_map->size) {
				continue; //skip size 0
			}

			if (ATU_MAP_MAU == type_idx) {
				remote_pa = CHIP_MAU_BASE;
				p_name = TYPE_NAME_MAU;
			} else if (ATU_MAP_APB == type_idx) {
				remote_pa = CHIP_APB_BASE;
				p_name = TYPE_NAME_APB;
			} else {
				DBG_WRN("unknown type %d\r\n", type_idx);
				continue;
			}

			if (CHIP_RC == chipid) { //used by upstream
				p_upmap->local_pa = p_drv_map->addr;
				p_upmap->size = p_drv_map->size;
				NVTPCIE_STRNCPY(p_upmap->name, p_name, sizeof(p_upmap->name));
				p_upmap->ep_chipid = CHIP_EP_COMMON;
				p_upmap->rc_pa = remote_pa;
				p_upmap++;
			} else { //used by downstream
				p_downmap->local_pa = p_drv_map->addr;
				p_downmap->size = p_drv_map->size;
				NVTPCIE_STRNCPY(p_downmap->name, p_name, sizeof(p_downmap->name));
				p_downmap->ep_chipid = chipid;
				p_downmap->ep_pa = remote_pa;
				p_downmap->ep_pci_dev = NULL;
				p_downmap++;
			}
		}
	}

	return 0;
}
#endif //#if IS_ENABLED(CONFIG_PCI)

#if 0 //for future use
static nvtpcie_chipid_t _nvtpcie_ddrid_to_chipid(nvtpcie_ddrid_t ddrid)
{
	int idx;

	for (idx = 0; idx < ARRAY_SIZE(g_ddr_map); idx++) {
		if (g_ddr_map[idx].ddrid == ddrid) {
			return g_ddr_map[idx].chipid;
		}
	}

	DBG_WRN("Invalid ddrid %d\r\n", ddrid);
	return CHIP_MAX;
}
#endif

//=== [dts sample] ===
//nvt-pcie-lib-cfg {
//	compatible = "nvt,nvt-pcie-lib";
//	rc_shm_size = <0x00000000 0x00100000>; /* [SIZE-H] [SIZE-L] */
//	ep_alloc_mem = <0x1 0x0 0x20000000 0x0 0x04000000>; // < [DDRID] [ADDR-H] [ADDR-L] [SIZE-H] [SIZE-L]
//};
static int _nvtpcie_dts_lib_cfg(void)
{
	struct device_node *p_node = NULL;
	u32 *p_dts_array = NULL;
	int total_elems;
	int ret = 0;
	int dts_idx, table_idx = 0;

	if (CHIP_RC != nvtpcie_get_my_chipid()) {
		//skip for EPn
		return 0;
	}

	//get node first
	p_node = of_find_node_by_path(DTS_PATH_LIB_CFG);
	if (NULL == p_node) {
		DBG_INFO("no node %s\r\n", DTS_PATH_LIB_CFG);
		return -1;
	}

	//alloc array buffer
	p_dts_array = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (p_dts_array == NULL) {
		DBG_ERR("kzalloc failed\r\n");
		ret = -1;
		goto exit;
	}

	//parsing rc_shm_size (skip if failed)
	if (0 == of_property_read_u32_array(p_node, PROP_RC_SHM_SIZE, p_dts_array, U32_ELEMS_RC_SHM_SIZE)) {
		g_rcshm_size = HL32_TO_U64(p_dts_array[0], p_dts_array[1]);
		DBG_INFO("dts: rc_shm_size 0x%llX\r\n", g_rcshm_size);
	} else {
		DBG_INFO("skip prop %s\r\n", PROP_RC_SHM_SIZE);
	}

	//parsing ep_alloc_mem
	total_elems = of_property_count_u32_elems(p_node, PROP_EP_ALLOC_MEM);
	if (total_elems < 0) {
		DBG_INFO("skip prop %s count %d\r\n", PROP_EP_ALLOC_MEM, total_elems);
		ret = -1;
		goto exit;
	}

	if (total_elems * sizeof(u32) > PAGE_SIZE) {
		DBG_ERR("prop %s too large\r\n", PROP_EP_ALLOC_MEM);
		ret = -1;
		goto exit;
	}

	if (total_elems % U32_ELEMS_EP_ALLOC_MEM) {
		DBG_ERR("total_elems %d invalid\r\n", total_elems);
		ret = -1;
		goto exit;
	}

	if ((total_elems / U32_ELEMS_EP_ALLOC_MEM) > ARRAY_SIZE(g_ep_allocmem)) {
		DBG_ERR("array size not enough\r\n");
		ret = -1;
		goto exit;
	}

	if (0 != of_property_read_u32_array(p_node, PROP_EP_ALLOC_MEM, p_dts_array, total_elems)) {
		DBG_ERR("prop %s read failed\r\n", PROP_RC_SHM_SIZE);
		ret = -1;
		goto exit;
	}

	DBG_IND("total_elems = %d\r\n", total_elems);

	for (dts_idx = 0; dts_idx < total_elems; dts_idx += U32_ELEMS_EP_ALLOC_MEM) {
		g_ep_allocmem[table_idx].ep_chipid = (nvtpcie_chipid_t)p_dts_array[dts_idx+0];
		g_ep_allocmem[table_idx].ep_local_pa = (phys_addr_t)HL32_TO_U64(p_dts_array[dts_idx+1], p_dts_array[dts_idx+2]);
		g_ep_allocmem[table_idx].size = (unsigned long)HL32_TO_U64(p_dts_array[dts_idx+3], p_dts_array[dts_idx+4]);
		g_ep_allocmem[table_idx].free_size = g_ep_allocmem[table_idx].size;
		DBG_INFO("dts: ep_alloc_mem[%d] ep_chipid %d ep_local_pa 0x%08llX size 0x%lX\r\n",
			table_idx, g_ep_allocmem[table_idx].ep_chipid, (u64)g_ep_allocmem[table_idx].ep_local_pa, g_ep_allocmem[table_idx].size);

		table_idx++;
	}

exit:
	if (p_dts_array) {
		kfree(p_dts_array);
	}
	of_node_put(p_node);
	return ret;
}

//=== [dts sample] ===
//ep_info {
//	enable = <1>; // 1 : boot to linux shell
//	linux_addr = <0x0>; /* This attribute is used to load ep ramdisk address, the address will be redefined by rc uboot runtime. */
//		ramdisk_addr = <0x0>; /* This attribute is used to load ep ramdisk address, the address will be redefined by rc uboot runtime. */
//		uboot_env {             /* EP uboot env init values */
//			mytest1="XXX";
//			mytest2="BBB";
//	};
static int _nvtpcie_dts_ep_info(void)
{
#if IS_ENABLED(CONFIG_PCI)
	struct device_node *p_node = NULL;
	u32 u32prop = 0;
	int ret = 0;

	p_node = of_find_node_by_path(DTS_PATH_EP_INFO);
	if (NULL == p_node) {
		DBG_INFO("no node %s\r\n", DTS_PATH_EP_INFO);
		g_my_chipid = CHIP_RC;
		return 0;
	}

	//if daughter is defined, it acts as RC
	if (0 == of_property_read_u32(p_node, PROP_EPINFO_DAUGHTER, &u32prop)) {
		DBG_INFO("daughter = <%d>\r\n", u32prop);
		if (1 == u32prop) {
			g_my_chipid = CHIP_RC;
			goto exit;
		}
	}

	if (0 == of_property_read_u32(p_node, PROP_EPINFO_CHIP_ID, &u32prop)) {
		g_my_chipid = u32prop;
		goto exit;
	} else {
		//backward-compatible if chip_id not found
		g_my_chipid = CHIP_EP0;
		goto exit;
	}

exit:
	of_node_put(p_node);
	return ret;

#else //IS_ENABLED(CONFIG_PCI)
	return 0;
#endif
}



void _nvtpcie_dump_rc_downmap(void)
{
	nvtpcie_rc_downmap_t *p_map;
	int idx;

	p_map = &g_rc_downmap[0];
	DBG_DUMP("\r\nrc_dumpmap\r\n");
	DBG_DUMP("               local_pa,               size,             name, ep_chipid, ep_pa\r\n");
	for (idx = 0; idx < ARRAY_SIZE(g_rc_downmap); idx++, p_map++) {
		if (0 == p_map->size) {
			continue;
		}
		DBG_DUMP("[%02d] 0x%016llX, 0x%016lx, %16s,  %8d, 0x%016llX\r\n",
			idx, (u64)p_map->local_pa, p_map->size, p_map->name, p_map->ep_chipid, (u64)p_map->ep_pa);
	}
}

void _nvtpcie_dump_ep_upmap(void)
{
	nvtpcie_ep_upmap_t *p_map;
	int idx;

	p_map = &g_ep_upmap[0];

	DBG_DUMP("\r\nep_upmap\r\n");
	DBG_DUMP("               local_pa,               size,             name, ep_chipid, rc_pa\r\n");
	for (idx = 0; idx < ARRAY_SIZE(g_ep_upmap); idx++, p_map++) {
		if (0 == p_map->size) {
			continue;
		}
		DBG_DUMP("[%02d] 0x%016llX, 0x%016lx, %16s,  %8d, 0x%016llX\r\n",
			idx, (u64)p_map->local_pa, p_map->size, p_map->name, p_map->ep_chipid, (u64)p_map->rc_pa);
	}
}

void _nvtpcie_dump_ddr_map(void)
{
	nvtpcie_ddr_map_t *p_map;
	int idx;

	p_map = &g_ddr_map[0];

	DBG_DUMP("\r\nddr_map\r\n");
	DBG_DUMP("       ddrid,   chipid,   ddridx,          chip_base,       size\r\n");
	for (idx = 0; idx < ARRAY_SIZE(g_ddr_map); idx++, p_map++) {
		DBG_DUMP("[%d] %8d, %8d, %8d, 0x%016llX, 0x%08lx\r\n",
			idx, p_map->ddrid, p_map->chipid, p_map->ddridx, (u64)p_map->chip_base, p_map->size);
	}
}

void _nvtpcie_dump_ep_allocmem(void)
{
	nvtpcie_ep_allocmem_t *p_epmem;
	int idx;

	p_epmem = &g_ep_allocmem[0];

	DBG_DUMP("\r\nep_allocmem\r\n");
	DBG_DUMP("   ep_chipid,        ep_local_pa,          rc_map_pa,          rc_map_va, total size,  free size\r\n");

	for (idx = 0; idx < ARRAY_SIZE(g_ep_allocmem); idx++, p_epmem++) {
		DBG_DUMP("[%d] %8d, 0x%016llX, 0x%016llX, 0x%016llX, 0x%08lX, 0x%08lX\r\n",
			idx, p_epmem->ep_chipid, (u64)p_epmem->ep_local_pa, (u64)p_epmem->rc_map_pa, P2U64(p_epmem->rc_map_va), p_epmem->size, p_epmem->free_size);
	}
}

#if IS_ENABLED(CONFIG_PCI)
nvtpcie_chipid_t nvtpcie_get_my_chipid(void)
{
	if (CHIP_MAX == g_my_chipid) {
		DBG_ERR("not inited yet\r\n");
	}
	return g_my_chipid;
}
#else
nvtpcie_chipid_t nvtpcie_get_my_chipid(void)
{
	#define TYPE1_BIST_HDR_TYPE_LAT_CACHE_LINE_SIZE_REG 0xC
	#define HEADER_TYPE_MASK    (0x7)
	#define HEADER_TYPE_OFFSET  (16)

	void __iomem *va_dbi;
	u32 reg;
	struct nvt_pcie_info *p_info;

	if (CHIP_MAX != g_my_chipid) {
		return g_my_chipid; //already inited, return directly
	}

	p_info = nvt_pcie_get_info();
	if (NULL == p_info) {
		return CHIP_RC;
	}

	va_dbi = p_info->dbi_ptr;
	if (NULL == va_dbi) {
		return CHIP_RC;
	}

	reg = readl(va_dbi + TYPE1_BIST_HDR_TYPE_LAT_CACHE_LINE_SIZE_REG);

	if ((reg >> HEADER_TYPE_OFFSET) & HEADER_TYPE_MASK) {
		g_my_chipid = CHIP_RC;
	} else {
		g_my_chipid = CHIP_EP0;
	}

	return g_my_chipid;
}
#endif
EXPORT_SYMBOL(nvtpcie_get_my_chipid);

/**
	Check whether CONFIG_PCI is enabled or not

	This function returns the CONFIG_PCI status of kernel configurations

	@return
		- @b If CONFIG_PCI is enabled, 1 is returned
		- @b Others, 0 is returned
*/
int nvtpcie_is_common_pci(void)
{
#if IS_ENABLED(CONFIG_PCI)
	return 1;
#else
	return 0;
#endif
}
EXPORT_SYMBOL(nvtpcie_is_common_pci);

/**
	Allocate memory from EP

	This function is used by RC to allocate EP memory

	@param[in] ep_chipid:   ep chipid you want to allocate memory from
	@param[in] alloc_size:  size to be allocated
	@param[out] ret_ep_pa:  EP local physical address
	@return
		- @b virtual address of allocated memory (from RC view). NULL if failed
		- @b NULL if an error occurred
*/
void *nvtpcie_alloc_epmem(nvtpcie_chipid_t ep_chipid, unsigned long alloc_size, phys_addr_t *ret_ep_pa)
{
	void *ret_rc_va = NULL;
	nvtpcie_ep_allocmem_t *p_epmem;

	if (!ISVALID_EP_CHIPID(ep_chipid)) {
		DBG_ERR("Invalid ep_chipid %d\r\n", ep_chipid);
		return NULL;
	}

	p_epmem = _nvtpcie_get_ep_allocmem(ep_chipid);
	if (NULL == p_epmem) {
		DBG_ERR("Invalid ep_chipid %d\r\n", ep_chipid);
		return NULL;
	}

	alloc_size = PAGE_ALIGN(alloc_size);

	down(&sem_nvtpcie);

	//ioremap for the first time
	if (NULL == p_epmem->rc_map_va) {
		p_epmem->rc_map_pa = nvtpcie_get_downstream_pa(ep_chipid, p_epmem->ep_local_pa);
		if (NVTPCIE_INVALID_PA == p_epmem->rc_map_pa) {
			DBG_ERR("downstream ep_base_pa 0x%08llX failed\r\n", (u64)p_epmem->ep_local_pa);
			goto exit;
		}
		p_epmem->rc_map_va = ioremap(p_epmem->rc_map_pa, p_epmem->size);
		if (NULL == p_epmem->rc_map_va) {
			DBG_ERR("ioremap failed\r\n");
			goto exit;
		}
		p_epmem->rc_free_va = p_epmem->rc_map_va;
		p_epmem->ep_free_pa = p_epmem->ep_local_pa;
		p_epmem->free_size = p_epmem->size;
	}

	if (alloc_size > p_epmem->free_size) {
		DBG_ERR("free %ld < alloc %ld\r\n", p_epmem->free_size, alloc_size);
		goto exit;
	}

	//set return values
	if (ret_ep_pa) {
		*ret_ep_pa = p_epmem->ep_free_pa;
	}
	ret_rc_va = p_epmem->rc_free_va;

	//update new free values
	p_epmem->rc_free_va += alloc_size;
	p_epmem->ep_free_pa += alloc_size;
	p_epmem->free_size -= alloc_size;

exit:
	up(&sem_nvtpcie);

	return ret_rc_va;
}
EXPORT_SYMBOL(nvtpcie_alloc_epmem);

/**
	Convert a specific ddrid to target chipid and ddridx

	@param[in] ddrid:   which ddrid you want to convert
	@param[out] out_chipid: target chipid
	@param[out] out_ddridx: target ddridx
	@return
		- @b On success, 0 is returned
		- @b On error, -1 is returned
*/
int nvtpcie_get_ddrid_map(nvtpcie_ddrid_t ddrid, nvtpcie_chipid_t *out_chipid, nvtpcie_ddrid_t *out_ddridx)
{
	int idx;

	if (!ISVALID_DDRID(ddrid)) {
		DBG_ERR("Invalid ddrid %d\r\n", ddrid);
		return -1;
	}

	for (idx = 0; idx < ARRAY_SIZE(g_ddr_map); idx++) {
		if (g_ddr_map[idx].ddrid == ddrid) {
			//found it
			if (out_chipid) {
				*out_chipid = g_ddr_map[idx].chipid;
			}
			if (out_ddridx) {
				*out_ddridx = g_ddr_map[idx].ddridx;
			}
			return 0;
		}
	}

	DBG_ERR("map not found, ddrid %d\r\n", ddrid);
	return -1;
}
EXPORT_SYMBOL(nvtpcie_get_ddrid_map);

/**
	Get ddrid by the local chipid and physical address

	@param[in] loc_chipid: local chipid
	@param[in] loc_pa: local physical address (including mapped pcie address)
	@return
		- @b On success, a valid ddrid is returned
		- @b On error, DDR_ID_MAX is returned
*/
nvtpcie_ddrid_t nvtpcie_get_ddrid(nvtpcie_chipid_t loc_chipid, phys_addr_t loc_pa)
{
	nvtpcie_chipid_t tar_chipid;
	phys_addr_t tar_pa;
	nvtpcie_ddr_map_t *p_map;
	int idx;

	//set initial values, which will be changed if needed
	tar_chipid = loc_chipid;
	tar_pa = loc_pa;

	//convert the chipid and the pa to the target view
	if (loc_chipid == CHIP_RC) {
		nvtpcie_rc_downmap_t *p_downmap;

		for (idx = 0; idx < ARRAY_SIZE(g_rc_downmap); idx++) {
			p_downmap = &g_rc_downmap[idx];
			if ((loc_pa >= p_downmap->local_pa) &&
				(loc_pa < (p_downmap->local_pa + p_downmap->size))) {
				tar_chipid = p_downmap->ep_chipid;
				tar_pa = loc_pa - p_downmap->local_pa + p_downmap->ep_pa;
				break;
			}
		}
	} else if (ISVALID_EP_CHIPID(loc_chipid)) {
		nvtpcie_ep_upmap_t *p_upmap;

		for (idx = 0; idx < ARRAY_SIZE(g_ep_upmap); idx++) {
			p_upmap = &g_ep_upmap[idx];
			if ((loc_pa >= p_upmap->local_pa) &&
				(loc_pa < (p_upmap->local_pa + p_upmap->size))) {
				tar_chipid = CHIP_RC;
				tar_pa = loc_pa - p_upmap->local_pa + p_upmap->rc_pa;
				break;
			}
		}
	} else {
		DBG_ERR("Invalid loc_chipid %d\r\n", loc_chipid);
		return DDR_ID_MAX;
	}

	//search local ddr mapping (target view)
	for (idx = 0; idx < ARRAY_SIZE(g_ddr_map); idx++) {
		p_map = &g_ddr_map[idx];

		if (p_map->chipid != tar_chipid)
			continue;

		if ((tar_pa >= p_map->chip_base) &&
			(tar_pa < (p_map->chip_base + p_map->size))) {
			return p_map->ddrid;
		}
	}
	return DDR_ID_MAX;
}
EXPORT_SYMBOL(nvtpcie_get_ddrid);

/**
	Get EP local physical(pcie) address to access RC memory

	Used when EP would like to access data of RC

	@param[in] ep_chipid: local ep chip id
	@param[in] rc_pa: rc physical address to be accessed
	@return
		- @b On success, physical address is returned
		- @b On error, NVTPCIE_INVALID_PA is returned
*/
phys_addr_t nvtpcie_get_upstream_pa(nvtpcie_chipid_t ep_chipid, phys_addr_t rc_pa)
{
	nvtpcie_ep_upmap_t *p_map;
	int idx;

	if (!ISVALID_EP_CHIPID(ep_chipid)) {
		DBG_ERR("Invalid ep_chipid %d\r\n", ep_chipid);
		return NVTPCIE_INVALID_PA;
	}

	for (idx = 0; idx < ARRAY_SIZE(g_ep_upmap); idx++) {
		p_map = &g_ep_upmap[idx];

		if ((ep_chipid == p_map->ep_chipid) &&
			(rc_pa >= p_map->rc_pa) &&
			(rc_pa < (p_map->rc_pa + p_map->size))) {
			return (p_map->local_pa + (rc_pa - p_map->rc_pa));
		}
	}

	return NVTPCIE_INVALID_PA;
}
EXPORT_SYMBOL(nvtpcie_get_upstream_pa);

/**
	Get RC local physical(pcie) address to access EP memory

	Used when RC would like to access data of EP

	@param[in] ep_chipid: target ep chip id
	@param[in] ep_pa: target ep physical address to be accessed
	@return
		- @b On success, physical address is returned
		- @b On error, NVTPCIE_INVALID_PA is returned
*/
phys_addr_t nvtpcie_get_downstream_pa(nvtpcie_chipid_t ep_chipid, phys_addr_t ep_pa)
{
	nvtpcie_rc_downmap_t *p_map;
	int idx;

	if (!ISVALID_EP_CHIPID(ep_chipid)) {
		DBG_ERR("Invalid ep_chipid %d\r\n", ep_chipid);
		return NVTPCIE_INVALID_PA;
	}

	for (idx = 0; idx < ARRAY_SIZE(g_rc_downmap); idx++) {
		p_map = &g_rc_downmap[idx];

		if ((ep_chipid == p_map->ep_chipid) &&
			(ep_pa >= p_map->ep_pa) &&
			(ep_pa < (p_map->ep_pa + p_map->size))) {
			return (p_map->local_pa + (ep_pa - p_map->ep_pa));
		}
	}

	return NVTPCIE_INVALID_PA;
}
EXPORT_SYMBOL(nvtpcie_get_downstream_pa);

/**
	Get local physical(pcie) address to access remote memory

	Automatically determine upstream or downstream to convert address

	@param[in] loc_chipid: local chip id (RC or EPn)
	@param[in] tar_ddrid: target ddr id
	@param[in] tar_pa: target physical address to be accessed
	@return
		- @b On success, physical(pcie) address is returned
		- @b On error, NVTPCIE_INVALID_PA is returned
*/
phys_addr_t nvtpcie_get_pcie_addr(nvtpcie_chipid_t loc_chipid, nvtpcie_ddrid_t tar_ddrid, phys_addr_t tar_pa)
{
	nvtpcie_chipid_t tar_chipid;

	if (!ISVALID_CHIPID(loc_chipid)) {
		return NVTPCIE_INVALID_PA;
	}

	if (!ISVALID_DDRID(tar_ddrid)) {
		DBG_ERR("Invalid tar_ddrid %d\r\n", tar_ddrid);
		return NVTPCIE_INVALID_PA;
	}

	if (0 != nvtpcie_get_ddrid_map(tar_ddrid, &tar_chipid, NULL)) {
		DBG_ERR("get tar_ddrid %d map failed\r\n", tar_ddrid);
		return NVTPCIE_INVALID_PA;
	}

	if (loc_chipid == tar_chipid) {//the same chip, use pa directly
		return tar_pa;
	} else if (CHIP_RC == loc_chipid) { //downstream RC->EP
		return nvtpcie_get_downstream_pa(tar_chipid, tar_pa);
	} else { //upstream EP->RC
		return nvtpcie_get_upstream_pa(loc_chipid, tar_pa);
	}
}
EXPORT_SYMBOL(nvtpcie_get_pcie_addr);

phys_addr_t nvtpcie_get_pcie_addr_by_bus(int loc_busid, int tar_busid, phys_addr_t tar_pa)
{
	nvtpcie_chipid_t loc_chipid;
	nvtpcie_chipid_t tar_chipid;

	if (BUSID_RC_COMMON == loc_busid) {
		loc_chipid = CHIP_RC;
	} else {
		loc_chipid = CHIP_EP_COMMON;
	}

	if (BUSID_RC_COMMON == tar_busid) {
		tar_chipid = CHIP_RC;
	} else {
		tar_chipid = CHIP_EP_COMMON;
	}

	return nvtpcie_get_mapped_pa(loc_chipid, tar_chipid, tar_pa);
}
EXPORT_SYMBOL(nvtpcie_get_pcie_addr_by_bus);

/**
	Get mapped physical(pcie) address to access remote(or local) memory

	Automatically determine upstream or downstream to convert address

	@param[in] loc_chipid: local chip id (RC or EPn)
	@param[in] tar_chipid: target chip id (RC or EPn), could be the same as loc_chipid
	@param[in] tar_pa: target physical address
	@note This API is almost the same as nvtpcie_get_pcie_addr, but parameters are different.
	@return
		- @b On success, physical(pcie) address is returned
		- @b On error, NVTPCIE_INVALID_PA is returned
*/
phys_addr_t nvtpcie_get_mapped_pa(nvtpcie_chipid_t loc_chipid, nvtpcie_chipid_t tar_chipid, phys_addr_t tar_pa)
{
	if (!ISVALID_CHIPID(loc_chipid)) {
		return NVTPCIE_INVALID_PA;
	}

	if (!ISVALID_CHIPID(tar_chipid)) {
		return NVTPCIE_INVALID_PA;
	}

	if (loc_chipid == tar_chipid) {//the same chip, use pa directly
		return tar_pa;
	} else if (CHIP_RC == loc_chipid) { //downstream RC->EP
		return nvtpcie_get_downstream_pa(tar_chipid, tar_pa);
	} else { //upstream EP->RC
		return nvtpcie_get_upstream_pa(loc_chipid, tar_pa);
	}
}
EXPORT_SYMBOL(nvtpcie_get_mapped_pa);

/**
	Get an unmapped physical address of the original chip

	Revert a mapped physical(pcie) address to its local address

	@param[in] map_chipid: the local chip id (RC or EPn) of the mapped_pa
	@param[in] mapped_pa: the mapped physical address at the local chip
	@param[out] ret_map_chipid: return the chip id which mapped_pa is mapped to (set NULL if not needed)
	@return
		- @b On success, a physical address is returned
		- @b On error, NVTPCIE_INVALID_PA is returned
*/
phys_addr_t nvtpcie_get_unmapped_pa(nvtpcie_chipid_t loc_chipid, phys_addr_t mapped_pa, nvtpcie_chipid_t *ret_map_chipid)
{
	int idx;

	if (!ISVALID_CHIPID(loc_chipid)) {
		return NVTPCIE_INVALID_PA;
	}

	//search in downmap or upmap table
	if (loc_chipid == CHIP_RC) {
		for (idx = 0; idx < ARRAY_SIZE(g_rc_downmap); idx++) {
			nvtpcie_rc_downmap_t *p_downmap = &g_rc_downmap[idx];
			if ((mapped_pa >= p_downmap->local_pa) &&
				(mapped_pa < (p_downmap->local_pa + p_downmap->size))) {
				if (ret_map_chipid) {
					*ret_map_chipid = p_downmap->ep_chipid;
				}
				return (mapped_pa - p_downmap->local_pa);
			}
		}
	} else { //CHIP_EPn
		for (idx = 0; idx < ARRAY_SIZE(g_ep_upmap); idx++) {
			nvtpcie_ep_upmap_t *p_upmap = &g_ep_upmap[idx];
			if ((mapped_pa >= p_upmap->local_pa) &&
				(mapped_pa < (p_upmap->local_pa + p_upmap->size))) {
				if (ret_map_chipid) {
					*ret_map_chipid = CHIP_RC;
				}
				return (mapped_pa - p_upmap->local_pa);
			}
		}
	}

	//check whether it is a local unmapped address
	for (idx = 0; idx < ARRAY_SIZE(g_ddr_map); idx++) {
		nvtpcie_ddr_map_t *p_map = &g_ddr_map[idx];
		if (p_map->chipid == loc_chipid) {
			if ((mapped_pa >= p_map->chip_base) &&
				(mapped_pa < (p_map->chip_base + p_map->size))) {
				if (ret_map_chipid) {
					*ret_map_chipid = p_map->chipid;
				}
				return mapped_pa;
			}
		}
	}

	DBG_ERR("Invalid loc_chipid %d or mapped_pa 0x%08llX\r\n", loc_chipid, (u64)mapped_pa);
	return NVTPCIE_INVALID_PA;
}
EXPORT_SYMBOL(nvtpcie_get_unmapped_pa);

/**
	Convert a resource by the given chipid

	Convert the start and end addresses of a resource by the given chipid

	@param[in] chipid: target chip id (Assumed to be EP, but RC is acceptable with no change)
	@param[in][out] resource: the resource to be converted (data will be modified)
	@return
		- @b On success, 0 is returned
		- @b On error, -1 is returned
*/
int nvtpcie_conv_resource(nvtpcie_chipid_t chipid, struct resource *resource)
{
	phys_addr_t ep_addr;
	resource_size_t res_size;

	if (!ISVALID_CHIPID(chipid)) {
		DBG_ERR("Invalid chipid %d\r\n", chipid);
		return -1;
	}

	if (NULL == resource) {
		DBG_ERR("resource is NULL\r\n");
		return -1;
	}

	if (CHIP_RC != nvtpcie_get_my_chipid()) {
		DBG_ERR("should be called at RC\n");
		return -1;
	}

	if (CHIP_RC == chipid) {
		//if chipid is RC, just bypass it
		return 0;
	}

	ep_addr = nvtpcie_get_downstream_pa(chipid, resource->start);
	if (NVTPCIE_INVALID_PA == ep_addr) {
		DBG_ERR("conv ep_chipid %d %pR failed\n", chipid, resource);
		return -1;
	}

	res_size = resource_size(resource);

	resource->start = ep_addr;
	resource->end = ep_addr + res_size - 1;

	return 0;
}
EXPORT_SYMBOL(nvtpcie_conv_resource);

/**
	Get EP active status

	To check ep is active or not

	@param[in] ep_chipid:   ep chipid to be checked
	@return
		- @b If active, 1 is returned
		- @b If not active, 0 is returned
*/
#if IS_ENABLED(CONFIG_PCI)
int nvtpcie_downstream_active(nvtpcie_chipid_t ep_chipid)
{
	int idx;
	nvtpcie_rc_downmap_t *p_map;

	if (CHIP_RC != nvtpcie_get_my_chipid()) {
		DBG_ERR("RC Only\r\n");
		return 0; //only valid at RC. it will crash if called at EP
	}

	for (idx = 0; idx < ARRAY_SIZE(g_rc_downmap); idx++) {
		p_map = &g_rc_downmap[idx];
		if (ep_chipid == p_map->ep_chipid) {
			if (p_map->ep_pci_dev) {
				return pci_device_is_present(p_map->ep_pci_dev);
			}
		}
	}

	return 0;
}
#else
int nvtpcie_downstream_active(nvtpcie_chipid_t ep_chipid)
{
	struct nvt_pcie_info *p_info;

	if (!ISVALID_EP_CHIPID(ep_chipid)) {
		//Do not print error here, since querying is a normal case
		return 0;
	}

	p_info = nvt_pcie_get_info();
	if (NULL == p_info) {
		return 0;
	}

	return ((p_info->link_masks & (1<<ep_chipid)) != 0);
}
#endif
EXPORT_SYMBOL(nvtpcie_downstream_active);

/**
	Get valid EP counts

	To get counts of EP which are connected to RC

	@return
		- @b EP counts
*/
int nvtpcie_get_ep_count_na51090(void)
{
#if IS_ENABLED(CONFIG_PCI)
	return nvt_pci_list_ep_drv_info_num();
#else
	struct nvt_pcie_info *p_info;
	int shift_bit;
	int count = 0;

	p_info = nvt_pcie_get_info();
	if (NULL == p_info) {
		return 0;
	}

	for (shift_bit = CHIP_EP0; shift_bit < CHIP_MAX; shift_bit++) {
		if (p_info->link_masks & (0x1 << shift_bit)) {
			count++;
		}
	}

	return count;
#endif
}
EXPORT_SYMBOL(nvtpcie_get_ep_count_na51090);

/**
	Get pcie link mode is cascade or not

	To get pcie link mode is cascade or not

	@return
		- @b If on cascade mode, 1 is returned
		- @b If not cascade mode, 0 is returned
*/
int nvtpcie_is_cascade(void)
{
#if IS_ENABLED(CONFIG_PCI) || IS_ENABLED(CONFIG_NVT_NT98336_PCI_EP_EVB)
	return 0;
#else
	return 1;
#endif
}
EXPORT_SYMBOL(nvtpcie_is_cascade);
/**
	Copy data across PCIe

	Use PCIe edma to copy data across PCIe

	@param[in] src_chipid:  src chipid to be copied from
	@param[in] src_pa:      src local physical address (not pcie-mapped address)
	@param[in] dst_chipid:  dst chipid to be copied to
	@param[in] dst_pa:      dst local physical address (not pcie-mapped address)
	@param[in] len:         the length to be copied

	@return
		- @b On succeed, 0 is returned
		- @b On error, -1 is returned
*/
#if defined(CONFIG_NVT_EDMA) || defined(CONFIG_NVT_PCIE_CASCADE_DRV)
int nvtpcie_edma_copy(nvtpcie_chipid_t src_chipid, phys_addr_t src_pa, nvtpcie_chipid_t dst_chipid, phys_addr_t dst_pa, unsigned long len)
{
	phys_addr_t	src_pa_pcie, dst_pa_pcie;
	nvtpcie_chipid_t my_chipid, remote_chipid;
	int	is_remote_read = 0;
	int ret, chan;

	//error check
	if (0 == len) {
		return 0;
	}

	if (src_chipid == dst_chipid) {
		DBG_ERR("only support dma across pcie\r\n");
		return -1;
	}

	if (!ISVALID_CHIPID(src_chipid)) {
		DBG_ERR("Invalid src_chipid %d\r\n", src_chipid);
		return -1;
	}

	if (!ISVALID_CHIPID(dst_chipid)) {
		DBG_ERR("Invalid dst_chipid %d\r\n", dst_chipid);
		return -1;
	}

#if IS_ENABLED(CONFIG_NVT_NT98336_PCI_EP_EVB)
	my_chipid = CHIP_EP_COMMON;
#else
	my_chipid = nvtpcie_get_my_chipid();
	if (!ISVALID_CHIPID(my_chipid)) {
		DBG_ERR("Fail to get chipid\r\n");
		return -1;
	}
#endif

	if (my_chipid == dst_chipid) {
		//remote read
		is_remote_read = 1;
		remote_chipid = src_chipid;
	} else if (my_chipid == src_chipid) {
		//remote write
		is_remote_read = 0;
		remote_chipid = dst_chipid;
	} else {
		DBG_ERR("Invalid chipid, my(%d), src(%d), dst(%d)\r\n", my_chipid, src_chipid, dst_chipid);
		return -1;
	}

	//convert src_pa to a pcie address
	src_pa_pcie = nvtpcie_get_mapped_pa(my_chipid, src_chipid, src_pa);
	if (NVTPCIE_INVALID_PA == src_pa_pcie) {
		DBG_ERR("Convert chipid %d -> %d, src_pa 0x%llX failed\r\n", my_chipid, src_chipid, (u64)src_pa);
		return -1;
	}

	//convert dst_pa to a pcie address
	dst_pa_pcie = nvtpcie_get_mapped_pa(my_chipid, dst_chipid, dst_pa);
	if (NVTPCIE_INVALID_PA == dst_pa_pcie) {
		DBG_ERR("Convert chipid %d -> %d, dst_pa 0x%llX failed\r\n", my_chipid, dst_chipid, (u64)dst_pa);
		return -1;
	}

	//call hw engine
#ifdef CONFIG_NVT_EDMA
	chan = nvt_alloc_edmac_chan();
	if (is_remote_read) {
		ret = nvt_pcie_edma_read(chan, src_pa_pcie, dst_pa_pcie, len, NULL, true);
	} else {
		ret = nvt_pcie_edma_write(chan, src_pa_pcie, dst_pa_pcie, len, NULL, true);
	}
	nvt_free_edmac_chan(chan);
#else
	if (is_remote_read) {
		ret = nvt_pcie_edma_read(remote_chipid, src_pa_pcie, dst_pa_pcie, len);
	} else {
		ret = nvt_pcie_edma_write(remote_chipid, src_pa_pcie, dst_pa_pcie, len);
	}
#endif

	if (ret != 0) {
		DBG_ERR("nvtpcie_edma_%s(%d, 0x%llX, 0x%llX, 0x%lX) failed\r\n",
			is_remote_read ? "read" : "write",
			remote_chipid, (u64)src_pa_pcie, (u64)dst_pa_pcie, len);
		return -1;
	}

	return 0;
}
#else
int nvtpcie_edma_copy(nvtpcie_chipid_t src_chipid, phys_addr_t src_pa, nvtpcie_chipid_t dst_chipid, phys_addr_t dst_pa, unsigned long len)
{
	DBG_ERR("%s - not supported \r\n", __func__);
	return -1;
}
#endif
EXPORT_SYMBOL(nvtpcie_edma_copy);

int nvtpcie_edma_copy_by_bus(int src_busid, phys_addr_t src_pa, int dst_busid, phys_addr_t dst_pa, unsigned long len)
{
	nvtpcie_chipid_t src_chipid;
	nvtpcie_chipid_t dst_chipid;

	if (BUSID_RC_COMMON == src_busid) {
		src_chipid = CHIP_RC;
	} else {
		src_chipid = CHIP_EP_COMMON;
	}

	if (BUSID_RC_COMMON == dst_busid) {
		dst_chipid = CHIP_RC;
	} else {
		dst_chipid = CHIP_EP_COMMON;
	}

	return nvtpcie_edma_copy(src_chipid, src_pa, dst_chipid, dst_pa, len);
}
EXPORT_SYMBOL(nvtpcie_edma_copy_by_bus);

static int __init nvtpcie_initcall(void)
{
	DBG_INFO("ver %s\n", NVT_PCIE_LIB_VERSION);

	if (0 != _nvtpcie_dts_ep_info()) {
		//do not return fail, just disable it
	}

	if (0 != _nvtpcie_dts_lib_cfg()) {
		//do not return fail, just disable it
	}

	if (0 != nvtpcie_proc_init()) {
		DBG_ERR("proc_init failed\r\n");
		return -1;
	}

	if (0 != nvtpcie_platdrv_init()) {
		DBG_ERR("platdrv_init failed\r\n");
		return -1;
	}

	if (0 != _nvtpcie_ddr_map_init()) {
		DBG_ERR("ddr_map_init failed\r\n");
		return -1;
	}

#if IS_ENABLED(CONFIG_NVT_NT98336_PCI_EP_EVB)
	if (0 != _nvtpcie_atu_board_init()) {
		DBG_INFO("board atu disabled\r\n");
		//do not return fail, just skip it
	}
#elif IS_ENABLED(CONFIG_PCI)
	if (0 != _nvtpcie_atu_common_init()) {
		DBG_INFO("common atu disabled\r\n");
		//do not return fail, just skip it
	}
#else
	if (0 != _nvtpcie_atu_cascade_init()) {
		DBG_INFO("cascade disabled\r\n");
		//do not return fail, just skip it
	}
#endif

	if (0 != nvtpcie_shmem_init(g_rcshm_size)) {
		DBG_INFO("shmem disabled\r\n");
		//do not return fail, just disable it
	}

	return 0;
}

//Note: nvtpcie_initcall should be initialized later than modules below:
//nvt-pcie-cascade-drv: to get some pcie info
//fmem: to use fmem_alloc_from_ker
device_initcall(nvtpcie_initcall);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("NVT PCIe library");
MODULE_LICENSE("GPL");
MODULE_VERSION(NVT_PCIE_LIB_VERSION);
