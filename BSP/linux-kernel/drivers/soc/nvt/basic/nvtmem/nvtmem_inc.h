#ifndef __NVTMEM_INC_H__
#define __NVTMEM_INC_H__

#include <linux/soc/nvt/fmem.h>
#include <linux/soc/nvt/nvt-pcie-lib.h>
/* structure to record the allocated memory nodes
*/
struct frm_vm_struct {
	struct nvt_fmem_mem_info_t nvt_fmem_mem_info;
	struct nvt_fmem_mem_cma_info_t* handle;
	struct list_head alloc_list;
};

/* If the open() is called, we will create one information is this open
*/
struct ap_open_info_t{
	struct list_head open_list;
	struct list_head alloc_list_head;
	void *filep; //index
	alloc_type_t alloc_type;
	int alloc_cnt;
};

struct ddr_alloc_list{
	int minor_id;
	int open_cnt;
	struct list_head open_list_head;
};

/* This struct is used to record the whole ddr to local ddr & chip (internal)
 */
typedef struct {
	nvtpcie_ddrid_t loc_ddrid;
	nvtpcie_chipid_t chipid;
	phys_addr_t	start;
	unsigned long size;
	int	active;
} ddr_chip_map_t;

#endif
