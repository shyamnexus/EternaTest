/**
    NVT utilities for pice customization

    @file       nvt_pcie_boot_utils.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <linux/libfdt.h>
#include <asm/nvt-common/nvt_common.h>
#include "nvt_ivot_pack.h"
#include <malloc.h>
#include <asm/nvt-common/modelext/bin_info.h>
#include <fdt_support.h>
#include <env.h>
#include <mapmem.h>
#include <image.h>

#define CMD_BUFFER_SIZE 255

/**************************************************************************
 *
 *
 * Novatek EP Boot Flow
 *
 *
 **************************************************************************/
static int nvt_ep_boot_prepare_env_and_boot(u64 linux_addr, u64 linux_size)
{
	char buf[CMD_BUFFER_SIZE], cmd[CMD_BUFFER_SIZE];
	u64 kernel_size;
	unsigned int nvt_ep_boot_ramdisk_addr, nvt_ep_boot_ramdisk_size, size;
	image_header_t *hdr;
	int ret;

	/* Get Ramdisk size and address */
	hdr = (image_header_t *)(unsigned long)nvt_memory_cfg[MEMTYPE_LINUXTMP].addr;
	size = image_get_data_size(hdr) + sizeof(image_header_t);
	nvt_ep_boot_ramdisk_size = ALIGN_CEIL(size, 4096);
	nvt_ep_boot_ramdisk_addr = nvt_memory_cfg[MEMTYPE_LINUX].addr + nvt_memory_cfg[MEMTYPE_LINUX].size - nvt_ep_boot_ramdisk_size - 0x10000;

	/* Copy Ramdisk to targe address */
	memmove((void *)(unsigned long)nvt_ep_boot_ramdisk_addr, (void *)(unsigned long)nvt_memory_cfg[MEMTYPE_LINUXTMP].addr, nvt_ep_boot_ramdisk_size);

	sprintf(buf, "0x%08x ", nvt_ep_boot_ramdisk_addr + nvt_ep_boot_ramdisk_size);
	env_set("initrd_high", buf);

	/* To assign relocated fdt address */
	sprintf(buf, "0x%08x ", linux_addr + linux_size);
	env_set("fdt_high", buf);

	/* The following will setup the lmb memory parameters for bootm cmd */
	sprintf(buf, "0x%08x ", linux_addr + linux_size);
	env_set("bootm_size", buf);
	env_set("bootm_mapsize", buf);

	sprintf(buf, "0x%08x ", linux_addr);
	env_set("bootm_low", buf);
	env_set("kernel_comp_addr_r", buf);

#if defined(CONFIG_ENCRYPT_KERNEL)
	HEADINFO *encrypt_header = (HEADINFO *)(nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr);
	kernel_size = encrypt_header->BinLength;

	#if defined(CONFIG_NVT_IVOT_OPTEE_SUPPORT) && defined(CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT)
	if(nvt_chk_signature((ulong)nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr, kernel_size)) {
		printf("check signature fail\n");
		return -1;
	}
	nvt_dbg(IND, "Verify encrypted linux PASS\r\n");

	ret = nvt_decrypt_aes((unsigned long)nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr);
	#elif defined(CONFIG_NVT_IVOT_SECBOOT_SUPPORT)
	if(nvt_chk_signature_on_uboot((ulong)nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr, kernel_size)) {
		printf("check signature fail\n");
		return -1;
	}
	nvt_dbg(IND, "Verify encrypted linux PASS\r\n");

	ret = nvt_decrypt_aes_on_uboot(nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr);
	#endif

	if (ret < 0) {
		nvt_dbg(ERR, "nvt_decrypt_aes fail ret %d\r\n", ret);
		return -1;
	}
	nvt_dbg(ERR, "nvt_decrypt_aes PASS\r\n");
#else /* !CONFIG_ENCRYPT_KERNEL */
	hdr = (image_header_t *)(unsigned long)nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr;
	kernel_size = image_get_data_size(hdr);

#endif
	sprintf(buf, "0x%x", kernel_size);
	env_set("kernel_comp_size", buf);
	run_command("pri", 0);

	sprintf(cmd, "booti");

	sprintf(cmd, "%s %x %lx %lx", cmd, nvt_memory_cfg[MEMTYPE_KERNEL_IMG].addr + sizeof(image_header_t), (unsigned long)nvt_ep_boot_ramdisk_addr, (unsigned long)gd->fdt_blob);
	printf("%s\n", cmd);
	run_command(cmd, 0);

	return 0;
}

static int nvt_ep_boot_flow(void)
{
	int ret;

#ifdef CONFIG_OF_CONTROL
	env_set_hex("fdtcontroladdr",
			(unsigned long)map_to_sysmem(gd->fdt_blob));
#endif

	/* Prepare env for boot to linux and boot */
	ret = nvt_ep_boot_prepare_env_and_boot(nvt_memory_cfg[MEMTYPE_LINUX].addr, nvt_memory_cfg[MEMTYPE_LINUX].size);

	return ret;
}

/**************************************************************************
 *
 *
 * Novatek RC Boot EP Entry
 *
 *
 **************************************************************************/

int do_nvt_pcie(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = -1;
	ret = nvt_ep_boot_flow();
	return ret;
}

U_BOOT_CMD(
	nvt_pcie, 2,    1,  do_nvt_pcie,
	"nvt RC boot EP",
	""
);
