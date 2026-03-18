/**
    NVT PCIe memory management
    To handle RC and EP device address translation
    @file       nvt-pcie-lib.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef __SOC_NVT_IVOT_NVT_PCIE_LIB_H
#define __SOC_NVT_IVOT_NVT_PCIE_LIB_H

#ifdef __KERNEL__
#include <linux/ioport.h> //for "struct resource"
#include <linux/types.h>
#define NVTPCIE_INVALID_PA      ((phys_addr_t)-1)
#else
#define NVTPCIE_INVALID_PA      ((unsigned long)-1)
#endif

#define NVTPCIE_IOC_MAGIC       'l'

#ifndef ENUM_DUMMY4WORD
#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000
#endif

typedef enum {
	DDR_ID0 = 0,
	DDR_ID1 = 1,
	DDR_ID2 = 2,
	DDR_ID3 = 3,
	DDR_ID4	= 4,
	DDR_ID5	= 5,
	DDR_ID6	= 6,
	DDR_ID7 = 7,
	DDR_ID_MAX,
} nvtpcie_ddrid_t;

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
	ENUM_DUMMY4WORD(nvtpcie_chipid_t),
} nvtpcie_chipid_t;

// ioctl definitions ----- begin
typedef struct {
	nvtpcie_chipid_t ret_chipid;
} NVTPCIE_IOARG_GET_MY_CHIPID;

typedef struct {
	nvtpcie_chipid_t ret_ep_count;
} NVTPCIE_IOARG_GET_EP_COUNT;

typedef struct {
	nvtpcie_chipid_t ep_chipid;
	int ret_is_active;
} NVTPCIE_IOARG_DOWNSTREAM_ACTIVE;

typedef struct {
	nvtpcie_chipid_t loc_chipid;
	nvtpcie_chipid_t tar_chipid;
	unsigned long tar_pa;
	unsigned long ret_mapped_pa;
} NVTPCIE_IOARG_GET_MAPPED_PA;

typedef struct {
	nvtpcie_chipid_t loc_chipid;
	unsigned long mapped_pa;
	unsigned long ret_unmapped_pa;
	nvtpcie_chipid_t ret_map_chipid;
} NVTPCIE_IOARG_GET_UNMAPPED_PA;

typedef struct {
	nvtpcie_chipid_t loc_chipid;
	nvtpcie_ddrid_t tar_ddrid;
	unsigned long tar_pa;
	unsigned long ret_pa;
} NVTPCIE_IOARG_GET_PCIE_ADDR;

typedef struct {
	int loc_busid;
	int tar_busid;
	unsigned long tar_pa;
	unsigned long ret_pa;
} NVTPCIE_IOARG_GET_PCIE_ADDR_BY_BUS;

typedef struct {
	unsigned long ret_pa; //local pa (RC), or mapped pcie pa (EP)
	unsigned long ret_size;
} NVTPCIE_IOARG_GET_SHMEM;

typedef struct {
	nvtpcie_chipid_t ep_chipid;
	unsigned long alloc_size;
	unsigned long ret_ep_pa; //EP local pa
	unsigned long ret_rc_mapped_pa; //RC mapped(pcie) pa which mapped to a block of EP memory
} NVTPCIE_IOARG_ALLOC_EPMEM;

typedef struct {
	char name[32];
	unsigned int size;
	unsigned long ret_pa;
} NVTPCIE_IOARG_SHMBLK_GET;

typedef struct {
	char name[32];
} NVTPCIE_IOARG_SHMBLK_RELEASE;

typedef struct {
	nvtpcie_chipid_t src_chipid;
	unsigned long src_pa;
	nvtpcie_chipid_t dst_chipid;
	unsigned long dst_pa;
	unsigned long len;
	int ret;
} NVTPCIE_IOARG_EDMA_COPY;

typedef struct {
	int src_busid;
	unsigned long src_pa;
	int dst_busid;
	unsigned long dst_pa;
	unsigned long len;
	int ret;
} NVTPCIE_IOARG_EDMA_COPY_BY_BUS;

#define NVTPCIE_IOCMD_GET_MY_CHIPID     _IOWR(NVTPCIE_IOC_MAGIC, 1, NVTPCIE_IOARG_GET_MY_CHIPID)
#define NVTPCIE_IOCMD_GET_EP_COUNT      _IOWR(NVTPCIE_IOC_MAGIC, 2, NVTPCIE_IOARG_GET_EP_COUNT)
#define NVTPCIE_IOCMD_DOWNSTREAM_ACTIVE _IOWR(NVTPCIE_IOC_MAGIC, 3, NVTPCIE_IOARG_DOWNSTREAM_ACTIVE)
#define NVTPCIE_IOCMD_GET_MAPPED_PA     _IOWR(NVTPCIE_IOC_MAGIC, 4, NVTPCIE_IOARG_GET_MAPPED_PA)
#define NVTPCIE_IOCMD_GET_UNMAPPED_PA   _IOWR(NVTPCIE_IOC_MAGIC, 5, NVTPCIE_IOARG_GET_UNMAPPED_PA)
#define NVTPCIE_IOCMD_GET_SHMEM         _IOWR(NVTPCIE_IOC_MAGIC, 6, NVTPCIE_IOARG_GET_SHMEM)
#define NVTPCIE_IOCMD_ALLOC_EPMEM       _IOWR(NVTPCIE_IOC_MAGIC, 7, NVTPCIE_IOARG_ALLOC_EPMEM)
#define NVTPCIE_IOCMD_SHMBLK_GET        _IOWR(NVTPCIE_IOC_MAGIC, 8, NVTPCIE_IOARG_SHMBLK_GET)
#define NVTPCIE_IOCMD_SHMBLK_RELEASE    _IOWR(NVTPCIE_IOC_MAGIC, 9, NVTPCIE_IOARG_SHMBLK_RELEASE)
#define NVTPCIE_IOCMD_EDMA_COPY         _IOWR(NVTPCIE_IOC_MAGIC, 10, NVTPCIE_IOARG_EDMA_COPY)
#define NVTPCIE_IOCMD_EDMA_COPY_BY_BUS  _IOWR(NVTPCIE_IOC_MAGIC, 11, NVTPCIE_IOARG_EDMA_COPY_BY_BUS)
#define NVTPCIE_IOCMD_GET_PCIE_ADDR     _IOWR(NVTPCIE_IOC_MAGIC, 12, NVTPCIE_IOARG_GET_PCIE_ADDR)
#define NVTPCIE_IOCMD_GET_PCIE_ADDR_BY_BUS  _IOWR(NVTPCIE_IOC_MAGIC, 13, NVTPCIE_IOARG_GET_PCIE_ADDR_BY_BUS)

// ioctl definitions ----- end

#ifdef __KERNEL__

#if IS_ENABLED(CONFIG_NVT_PCIE_LIB)
void *nvtpcie_alloc_epmem(nvtpcie_chipid_t ep_chipid, unsigned long alloc_size, phys_addr_t *ret_ep_pa);

int nvtpcie_get_ddrid_map(nvtpcie_ddrid_t ddrid, nvtpcie_chipid_t *out_chipid, nvtpcie_ddrid_t *out_ddridx);
nvtpcie_ddrid_t nvtpcie_get_ddrid(nvtpcie_chipid_t loc_chipid, phys_addr_t loc_pa);

phys_addr_t nvtpcie_get_upstream_pa(nvtpcie_chipid_t ep_chipid, phys_addr_t rc_pa);
phys_addr_t nvtpcie_get_downstream_pa(nvtpcie_chipid_t ep_chipid, phys_addr_t ep_pa);
phys_addr_t nvtpcie_get_pcie_addr(nvtpcie_chipid_t loc_chipid, nvtpcie_ddrid_t tar_ddrid, phys_addr_t tar_pa);
phys_addr_t nvtpcie_get_pcie_addr_by_bus(int loc_busid, int tar_busid, phys_addr_t tar_pa);
phys_addr_t nvtpcie_get_mapped_pa(nvtpcie_chipid_t loc_chipid, nvtpcie_chipid_t tar_chipid, phys_addr_t tar_pa);
phys_addr_t nvtpcie_get_unmapped_pa(nvtpcie_chipid_t loc_chipid, phys_addr_t mapped_pa, nvtpcie_chipid_t *ret_map_chipid);

int nvtpcie_is_common_pci(void);
#define nvtpcie_get_ep_count(...) nvtpcie_get_ep_count_na51090()
int nvtpcie_get_ep_count_na51090(void);
int nvtpcie_is_cascade(void);
int nvtpcie_downstream_active(nvtpcie_chipid_t ep_chipid);
int nvtpcie_conv_resource(nvtpcie_chipid_t ep_chipid, struct resource *resource);

void *nvtpcie_shmem_get_va(void);
phys_addr_t nvtpcie_shmem_get_pa(void);
unsigned long nvtpcie_shmem_get_size(void);
void nvtpcie_shmem_dump(unsigned long begin_offs, unsigned long length);

void* nvtpcie_shmblk_get(const char *name, unsigned int size);
void nvtpcie_shmblk_release(const char *name);

nvtpcie_chipid_t nvtpcie_get_my_chipid(void);

int nvtpcie_edma_copy(nvtpcie_chipid_t src_chipid, phys_addr_t src_pa, nvtpcie_chipid_t dst_chipid, phys_addr_t dst_pa, unsigned long len);
int nvtpcie_edma_copy_by_bus(int src_busid, phys_addr_t src_pa, int dst_busid, phys_addr_t dst_pa, unsigned long len);

#else //CONFIG_NVT_PCIE_LIB
#define DUMMY_FUNC static inline
DUMMY_FUNC void *nvtpcie_alloc_epmem(nvtpcie_chipid_t ep_chipid, unsigned long alloc_size, phys_addr_t *ret_ep_pa)
{
	return NULL;
}

DUMMY_FUNC int nvtpcie_get_ddrid_map(nvtpcie_ddrid_t ddrid, nvtpcie_chipid_t *out_chipid, nvtpcie_ddrid_t *out_ddridx)
{
	return -1;
}

DUMMY_FUNC nvtpcie_ddrid_t nvtpcie_get_ddrid(nvtpcie_chipid_t loc_chipid, phys_addr_t loc_pa)
{
	return DDR_ID_MAX;
}

DUMMY_FUNC phys_addr_t nvtpcie_get_upstream_pa(nvtpcie_chipid_t ep_chipid, phys_addr_t rc_pa)
{
	return NVTPCIE_INVALID_PA;
}

DUMMY_FUNC phys_addr_t nvtpcie_get_downstream_pa(nvtpcie_chipid_t ep_chipid, phys_addr_t ep_pa)
{
	return NVTPCIE_INVALID_PA;
}

DUMMY_FUNC phys_addr_t nvtpcie_get_pcie_addr(nvtpcie_chipid_t loc_chipid, nvtpcie_ddrid_t tar_ddrid, phys_addr_t tar_pa)
{
	return NVTPCIE_INVALID_PA;
}

DUMMY_FUNC phys_addr_t nvtpcie_get_pcie_addr_by_bus(int loc_busid, int tar_busid, phys_addr_t tar_pa)
{
	return NVTPCIE_INVALID_PA;
}

DUMMY_FUNC phys_addr_t nvtpcie_get_mapped_pa(nvtpcie_chipid_t loc_chipid, nvtpcie_chipid_t tar_chipid, phys_addr_t tar_pa)
{
	return NVTPCIE_INVALID_PA;
}

DUMMY_FUNC phys_addr_t nvtpcie_get_unmapped_pa(nvtpcie_chipid_t loc_chipid, phys_addr_t mapped_pa, nvtpcie_chipid_t *ret_map_chipid)
{
	return NVTPCIE_INVALID_PA;
}

DUMMY_FUNC int nvtpcie_is_common_pci(void)
{
	return 0;
}

#define nvtpcie_get_ep_count(...) nvtpcie_get_ep_count_na51090()
DUMMY_FUNC int nvtpcie_get_ep_count_na51090(void)
{
	return 0;
}

DUMMY_FUNC int nvtpcie_is_cascade(void)
{
	return 0;
}

DUMMY_FUNC int nvtpcie_downstream_active(nvtpcie_chipid_t ep_chipid)
{
	return 0;
}

DUMMY_FUNC int nvtpcie_conv_resource(nvtpcie_chipid_t ep_chipid, struct resource *resource)
{
	return -1;
}

DUMMY_FUNC void *nvtpcie_shmem_get_va(void)
{
	return NULL;
}

DUMMY_FUNC phys_addr_t nvtpcie_shmem_get_pa(void)
{
	return NVTPCIE_INVALID_PA;
}

DUMMY_FUNC unsigned long nvtpcie_shmem_get_size(void)
{
	return 0;
}

DUMMY_FUNC void nvtpcie_shmem_dump(unsigned long length)
{
	return;
}

DUMMY_FUNC void* nvtpcie_shmblk_get(const char *name, unsigned int size)
{
	return NULL;
}

DUMMY_FUNC void nvtpcie_shmblk_release(const char *name)
{
	return;
}

DUMMY_FUNC nvtpcie_chipid_t nvtpcie_get_my_chipid(void)
{
	return CHIP_RC;
}

DUMMY_FUNC int nvtpcie_edma_copy(nvtpcie_chipid_t src_chipid, phys_addr_t src_pa, nvtpcie_chipid_t dst_chipid, phys_addr_t dst_pa, unsigned long len)
{
	return -1;
}

DUMMY_FUNC int nvtpcie_edma_copy_by_bus(int src_busid, phys_addr_t src_pa, int dst_busid, phys_addr_t dst_pa, unsigned long len)
{
	return -1;
}
#endif //CONFIG_NVT_PCIE_LIB

#endif //__KERNEL__

#endif /* __SOC_NVT_IVOT_NVT_PCIE_LIB_H */
