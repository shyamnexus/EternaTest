/**
    NVT hdal memory operation handling
    Add a wrapper to handle the fmem memory handling api and version management API
    @file       nvtmem.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef __SOC_NVT_IVOT_NVTMEM_H
#define __SOC_NVT_IVOT_NVTMEM_H

#include <linux/list.h>
#include <asm/io.h>
#include <asm/setup.h>
#if defined(__aarch64__)
#include <linux/soc/nvt/fmem.h>
#include <linux/soc/nvt/nvt-pcie-lib.h>
#else
#include <linux/soc/nvt/fmem.h>
#include <linux/soc/nvt/nvt-pcie-lib.h>
#endif
#include <linux/soc/nvt/nvtmem_if.h>

#define NVT_HDAL_MEM_VERSION	"1.0.1"

struct nvtmem_ver_info_t {
	struct platform_device	*pdev;
	int			id;
	unsigned int 		version;
	unsigned int 		patch_level;
	unsigned int 		sub_level;
	unsigned int		extra_ver;
	struct list_head	list;
};


/* @this function is used to do hdal layer version control sys registeration.
 * @parm: pdev:
 *		platform device node
 * @parm: version:
 * 		format 00.00.00.00
 * @return:
 *		< 0 : Failed
 *		>= 0: success
 */
int nvtmem_version_register(struct platform_device *pdev, char *version);

/* @this function is used to do hdal layer version control sys unregisteration.
 * @parm: pdev:
 *		platform device node
 * @return:
 *		< 0 : Failed
 *		>= 0: success
 */
int nvtmem_version_unregister(struct platform_device *pdev);

/* @this function is used to allocate buffer from cma.
 * @parm: nvt_fmem_info:
 *		You should use nvt_fmem_mem_info_init to initiate nvt_fmem_mem_info_t data structure
 * @return:
 *		Null : Failed
 *		Non-Null: success
 */
void* nvtmem_alloc_buffer(struct nvt_fmem_mem_info_t *nvt_fmem_info);

/* @this function is used to free buffer from cma.
 * @parm: handle:
 *		pointer to allocated handle
 * @return:
 *		< 0 : Failed
 *		>= 0: success
 */
int nvtmem_release_buffer(void *handle);

/* @this function is used to allocate DSP buffer from dsp cma area.
 * @parm: nvt_fmem_info:
 *		You should use nvt_fmem_mem_info_init to initiate nvt_fmem_mem_info_t data structure
 * @return:
 *		Null : Failed
 *		Non-Null: success
 */
void* nvtmem_alloc_dsp_buffer(struct nvt_fmem_mem_info_t *nvt_fmem_info);

/* @this function is used to free DSP buffer from dsp cma area.
 * @parm: handle:
 *		pointer to allocated handle
 * @return:
 *		< 0 : Failed
 *		>= 0: success
 */
int nvtmem_release_dsp_buffer(void *handle);

/* @this function is used to allocate DSP2 buffer from dsp2 cma area.
 * @parm: nvt_fmem_info:
 *		You should use nvt_fmem_mem_info_init to initiate nvt_fmem_mem_info_t data structure
 * @return:
 *		Null : Failed
 *		Non-Null: success
 */
void* nvtmem_alloc_dsp2_buffer(struct nvt_fmem_mem_info_t *nvt_fmem_info);

/* @this function is used to free DSP2 buffer from dsp2 cma area.
 * @parm: handle:
 *		pointer to allocated handle
 * @return:
 *		< 0 : Failed
 *		>= 0: success
 */
int nvtmem_release_dsp2_buffer(void *handle);

/* @this function is used to do virtual address to physical address transform.
 * @parm: vaddr:
 *		virtual address
 * @return:
 *		-1UL : Failed
 *		>= 0: success
 */

/* @this function is used to allocate mcu buffer from mcu cma area.
 * @parm: nvt_fmem_info:
 *		You should use nvt_fmem_mem_info_init to initiate nvt_fmem_mem_info_t data structure
 * @return:
 *		Null : Failed
 *		Non-Null: success
 */
void* nvtmem_alloc_mcu_buffer(struct nvt_fmem_mem_info_t *nvt_fmem_info);

/* @this function is used to free mcu buffer from mcu cma area.
 * @parm: handle:
 *		pointer to allocated handle
 * @return:
 *		< 0 : Failed
 *		>= 0: success
 */
int nvtmem_release_mcu_buffer(void *handle);

/* @this function is used to do virtual address to physical address transform.
 * @parm: vaddr:
 *		virtual address
 * @return:
 *		-1UL : Failed
 *		>= 0: success
 */
phys_addr_t nvtmem_va_to_pa(uintptr_t vaddr);

typedef struct {
	int nr_banks;   /* how many banks in this chip */
	struct membank_s {
		phys_addr_t start;
		unsigned long size;
		nvtpcie_ddrid_t ddrid;
		nvtpcie_chipid_t chip;	/* chipid, located in which chip of the system */
	} bank[MAX_DDR_NR];
} nvtmem_ddrinfo_t;

/*
 * @This function shows memory layout for all DDR
 *
 * @function void nvtmem_get_ddrinfo(nvtmem_ddrinfo_t *info)
 * @input param: None
 * @output param: the layout of all DDRs managed by frammap
 * @return value: None.
 */
void nvtmem_get_ddrinfo(nvtmem_ddrinfo_t *info);
#endif /* __SOC_NVT_IVOT_NVTMEM_H */
