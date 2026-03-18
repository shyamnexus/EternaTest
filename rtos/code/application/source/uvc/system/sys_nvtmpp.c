#include <libfdt.h>
#include <compiler.h>
#include <plat/rtosfdt.h>
#include <kflow_common/nvtmpp.h>
#include <hdal.h>
#include "sys_nvtmpp.h"

#if 0
int nvtmpp_init(void)
{
#if 1
	const void *p_fdt = fdt_get_base();
	int addr_cells, size_cells;
	int len;
	NVTMPP_ER          ret;
	NVTMPP_SYS_CONF_S  nvtmpp_sys_conf;
	unsigned int *p_data;
	int nodeoffset, len_offset;

	if (p_fdt == NULL) {
		printf("p_fdt is NULL. \n");
		return -1;
	}

	nodeoffset = fdt_path_offset(p_fdt, "/hdal-memory");
	if (nodeoffset < 0) {
		printf("failed to offset for /hdal-memory %d \r\n", nodeoffset);
		return -1;
	}
	addr_cells = fdt_address_cells(p_fdt, nodeoffset);
	size_cells = fdt_size_cells(p_fdt, nodeoffset);

	// read /hdal-memory/media
	nodeoffset = fdt_path_offset(p_fdt, "/hdal-memory/media");
	if (nodeoffset < 0) {
		printf("failed to offset for /hdal-memory/media = %d \n", nodeoffset);
		return -1;
	}
	const void *nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
	p_data = (unsigned int *)nodep;
	if (len == 0 || nodep == NULL) {
		printf("failed to access /nvt_memory_cfg/rtos/reg.\n");
		return -1;
	}
	//printf("hdal-memory len = %d, addr_cells = %d, size_cells = %d\r\n", len, addr_cells, size_cells);
	nvtmpp_install_id();
	memset((void *)&nvtmpp_sys_conf, 0x00, sizeof(nvtmpp_sys_conf));
	len_offset = (addr_cells + size_cells) * 4;
	if (addr_cells == 1 && size_cells == 1) {
		if (len >= len_offset * 1) {
			nvtmpp_sys_conf.ddr_mem[NVTMPP_DDR_1].virt_addr = (uintptr_t)be32_to_cpu(p_data[0]);
			nvtmpp_sys_conf.ddr_mem[NVTMPP_DDR_1].phys_addr = nvtmpp_sys_conf.ddr_mem[NVTMPP_DDR_1].virt_addr;
			nvtmpp_sys_conf.ddr_mem[NVTMPP_DDR_1].size = (UINT32)be32_to_cpu(p_data[1]);
		}
		if (len >= len_offset * 2) {
			nvtmpp_sys_conf.ddr_mem[NVTMPP_DDR_2].virt_addr = (uintptr_t)be32_to_cpu(p_data[len_offset/4]);
			nvtmpp_sys_conf.ddr_mem[NVTMPP_DDR_2].phys_addr = nvtmpp_sys_conf.ddr_mem[NVTMPP_DDR_2].virt_addr;
			nvtmpp_sys_conf.ddr_mem[NVTMPP_DDR_2].size = (UINT32)be32_to_cpu(p_data[len_offset/4 + 1]);
		}
	} else if (addr_cells == 4 && size_cells == 2) {
		if (len >= len_offset * 1) {
			nvtmpp_sys_conf.ddr_mem[NVTMPP_DDR_1].virt_addr = (uintptr_t)be32_to_cpu(p_data[3]);
			nvtmpp_sys_conf.ddr_mem[NVTMPP_DDR_1].phys_addr = nvtmpp_sys_conf.ddr_mem[NVTMPP_DDR_1].virt_addr;
			nvtmpp_sys_conf.ddr_mem[NVTMPP_DDR_1].size = (UINT32)be32_to_cpu(p_data[5]);
		}
		if (len >= len_offset * 2) {
			nvtmpp_sys_conf.ddr_mem[NVTMPP_DDR_2].virt_addr = (uintptr_t)be32_to_cpu(p_data[len_offset/4 + 3]);
			nvtmpp_sys_conf.ddr_mem[NVTMPP_DDR_2].phys_addr = nvtmpp_sys_conf.ddr_mem[NVTMPP_DDR_2].virt_addr;
			nvtmpp_sys_conf.ddr_mem[NVTMPP_DDR_2].size = (UINT32)be32_to_cpu(p_data[len_offset/4 + 5]);
		}
	} else {
		printf("Unknown dts format\r\n");
		return -1;
	}
	ret = nvtmpp_sys_init(&nvtmpp_sys_conf);
	if (NVTMPP_ER_OK != ret) {
		printf("nvtmpp sys init err: %d\r\n", ret);
		return -1;
	}
	return 0;
#else
	return 0;
#endif
}
#endif
