#ifndef __SOC_NVT_IVOT_NVT_PCIE_LIB_H
#define __SOC_NVT_IVOT_NVT_PCIE_LIB_H

#define NVTPCIE_INVALID_PA ((phys_addr_t) - 1)

typedef enum {
	DDR_ID0 = 0,
	DDR_ID1 = 1,
	DDR_ID2 = 2,
	DDR_ID3 = 3,
	DDR_ID4 = 4,
	DDR_ID5 = 5,
	DDR_ID6 = 6,
	DDR_ID7 = 7,
	DDR_ID_MAX,
} nvtpcie_ddrid_t;

typedef enum {
	PCI_DEV_ID_NA51090 = 0x0005,
	PCI_DEV_ID_NS02201 = 0x0006,
	PCI_DEV_ID_NS02401 = 0x0007,
	PCI_DEV_ID_ALL,
} nvt_pci_dev_id_t;

typedef enum {
	CHIP_RC = 0,
	CHIP_EP0 = 1,
	CHIP_EP1 = 2,
	CHIP_EP2 = 3,
	CHIP_EP3 = 4,
	CHIP_EP4 = 5,
	CHIP_EP5 = 6,
	CHIP_EP6 = 7,
	CHIP_EP7 = 8,
	CHIP_EP8 = 9,
	CHIP_MAX,
} nvtpcie_chipid_t;

struct pci_ep_info {
	void __iomem *apb_va;
	struct pci_dev *pci_dev;
	unsigned long ep_out_apb_pa;
	unsigned long ep_out_apb_len;
	int dev_id;
	int ep_idx;
};

typedef int (*nvt_ep_func_ptr_t)(struct pci_ep_info *ep_info_t);

static inline int nvtpcie_is_common_pci(void)
{
#if IS_ENABLED(CONFIG_PCI)
	return 1;
#else
	return 0;
#endif
}

static inline nvtpcie_chipid_t nvtpcie_get_my_chipid(void)
{
#if defined(CONFIG_PCI)
	return CHIP_RC;
#elif defined(CONFIG_NVT_NT98690_PCI_EP_EVB)
	return CHIP_EP0;
#else
	return CHIP_RC;
#endif
}

static inline phys_addr_t nvtpcie_get_pcie_addr(nvtpcie_chipid_t loc_chipid, nvtpcie_ddrid_t tar_ddrid, phys_addr_t tar_pa)
{
	return NVTPCIE_INVALID_PA;
}

static inline void *nvtpcie_alloc_epmem(nvtpcie_chipid_t ep_chipid, unsigned long alloc_size, phys_addr_t *ret_ep_pa)
{
	return NULL;
}

static inline int nvtpcie_downstream_active(nvtpcie_chipid_t ep_chipid)
{
	return 0;
}

#if defined(CONFIG_PCI) || defined(CONFIG_NVT_NT98690_PCI_EP_EVB)
/*
 * Get EP local physical(pcie) address to access RC memory
 * Used when EP would like to access data of RC
 * @param[in] ep_chipid: local ep chip id
 * @param[in] rc_pa: rc physical address to be accessed
 *
 * @return
 * - @b On success, physical address is returned
 * - @b On error, NVTPCIE_INVALID_PA is returned
 */
phys_addr_t nvtpcie_get_upstream_pa(nvtpcie_chipid_t ep_chipid, phys_addr_t rc_pa);

/*
 * Get RC local physical(pcie) address to access EP memory
 * Used when RC would like to access data of EP
 * @param[in] ep_chipid: target ep chip id
 * @param[in] ep_pa: target ep physical address to be accessed
 *
 * @return
 * - @b On success, physical address is returned
 * - @b On error, NVTPCIE_INVALID_PA is returned
 */
phys_addr_t nvtpcie_get_downstream_pa(nvtpcie_chipid_t ep_chipid, phys_addr_t ep_pa);

/*
 * Get mapped physical(pcie) address to access remote(or local) memory
 * Automatically determine upstream or downstream to convert address
 * @param[in] loc_chipid: local chip id (RC or EPn)
 * @param[in] tar_chipid: target chip id (RC or EPn), could be the same as loc_chipid
 * @param[in] tar_pa: target physical address
 * @note This API is almost the same as nvtpcie_get_pcie_addr, but parameters are different.
 *
 * @return
 * - @b On success, physical(pcie) address is returned
 * - @b On error, NVTPCIE_INVALID_PA is returned
 */
phys_addr_t nvtpcie_get_mapped_pa(nvtpcie_chipid_t loc_chipid, nvtpcie_chipid_t tar_chipid, phys_addr_t tar_pa);

/*
 * Get an unmapped physical address of the original chip
 * Revert a mapped physical(pcie) address to its local address
 * @param[in] map_chipid: the local chip id (RC or EPn) of the mapped_pa
 * @param[in] mapped_pa: the mapped physical address at the local chip
 * @param[out] ret_map_chipid: return the chip id which mapped_pa is mapped to (set NULL if not needed)
 *
 * @return
 * - @b On success, a physical address is returned
 * - @b On error, NVTPCIE_INVALID_PA is returned
 */
phys_addr_t nvtpcie_get_unmapped_pa(nvtpcie_chipid_t loc_chipid, phys_addr_t mapped_pa, nvtpcie_chipid_t *ret_map_chipid);
phys_addr_t nvtpcie_get_pcie_addr_by_bus(int loc_busid, int tar_busid, phys_addr_t tar_pa);

/*
 * Convert a resource by the given chipid
 * Convert the start and end addresses of a resource by the given chipid
 * @param[in] chipid: target chip id (Assumed to be EP, but RC is acceptable with no change)
 * @param[in][out] resource: the resource to be converted (data will be modified)
 *
 * @return
 * - @b On success, 0 is returned
 * - @b On error, -EINVAL is returned
 */
int nvtpcie_conv_resource(nvtpcie_chipid_t chipid, struct resource *resource);
int nvtpcie_edma_copy_by_bus(int src_busid, phys_addr_t src_pa, int dst_busid, phys_addr_t dst_pa, unsigned long len);
int nvt_pci_list_for_each_ep_drv_info(nvt_ep_func_ptr_t nvt_ep_func_ptr);
int nvtpcie_get_ep_count(nvt_pci_dev_id_t id);
void *nvtpcie_epinb_map(int ep_idx, unsigned long ep_pa, unsigned long size, unsigned long *out_rc_pa);
int nvtpcie_gpio_direction_output(nvtpcie_chipid_t id, unsigned gpio, int value);
int nvtpcie_gpio_set_value(nvtpcie_chipid_t id, unsigned int gpio, int value);
#else
static inline phys_addr_t nvtpcie_get_upstream_pa(nvtpcie_chipid_t ep_chipid, phys_addr_t rc_pa)
{
	return NVTPCIE_INVALID_PA;
}

static inline phys_addr_t nvtpcie_get_downstream_pa(nvtpcie_chipid_t ep_chipid, phys_addr_t ep_pa)
{
	return NVTPCIE_INVALID_PA;
}

static inline phys_addr_t nvtpcie_get_mapped_pa(nvtpcie_chipid_t loc_chipid, nvtpcie_chipid_t tar_chipid, phys_addr_t tar_pa)
{
	return NVTPCIE_INVALID_PA;
}

static inline phys_addr_t nvtpcie_get_unmapped_pa(nvtpcie_chipid_t loc_chipid, phys_addr_t mapped_pa, nvtpcie_chipid_t *ret_map_chipid)
{
	return NVTPCIE_INVALID_PA;
}

static inline int nvtpcie_conv_resource(nvtpcie_chipid_t chipid, struct resource *resource)
{
	return -EINVAL;
}

static inline int nvtpcie_edma_copy_by_bus(int src_busid, phys_addr_t src_pa, int dst_busid, phys_addr_t dst_pa, unsigned long len)
{
	return -EINVAL;
}

static inline int nvt_pci_list_for_each_ep_drv_info(nvt_ep_func_ptr_t nvt_ep_func_ptr)
{
	return -EINVAL;
}

static inline int nvtpcie_get_ep_count(nvt_pci_dev_id_t id)
{
	return 0;
}

static inline void *nvtpcie_epinb_map(int ep_idx, unsigned long ep_pa, unsigned long size, unsigned long *out_rc_pa)
{
	return NULL;
}

static inline phys_addr_t nvtpcie_get_pcie_addr_by_bus(int loc_busid, int tar_busid, phys_addr_t tar_pa)
{
	return NVTPCIE_INVALID_PA;
}

static inline int nvtpcie_gpio_direction_output(nvtpcie_chipid_t id, unsigned gpio, int value)
{
	return -EINVAL;
}

static inline int nvtpcie_gpio_set_value(nvtpcie_chipid_t id, unsigned int gpio, int value)
{
	return -EINVAL;
}
#endif /* CONFIG_PCI */

#endif
