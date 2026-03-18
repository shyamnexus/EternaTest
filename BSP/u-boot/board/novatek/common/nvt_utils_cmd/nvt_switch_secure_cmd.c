/*
 * Copyright (c) 2021 Novatek Inc.
 *
 */

#include <stdlib.h>
#include <common.h>
#include <command.h>
#include <console.h>
#include <asm/arch/efuse_protected.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/nvt_ivot_efuse_smc.h>

struct nvt_image {
	unsigned long long addr;
	unsigned long long size;
};

struct nvt_image loader = {
	.addr = 0x0,
	.size = 0x20000,
};

struct nvt_image dts = {
	.addr = 0x40000,
	.size = 0x20000,
};

struct nvt_image atf = {
	.addr = 0xC0000,
	.size = 0x20000,
};

struct nvt_image uboot = {
	.addr = 0x100000,
	.size = 0xC0000,
};

struct nvt_image encrypted_loader = {
	.addr = 0x200000,
	.size = 0x20000,
};

struct nvt_image encrypted_dts = {
	.addr = 0x220000,
	.size = 0x40000,
};

struct nvt_image encrypted_atf = {
	.addr = 0x260000,
	.size = 0x20000,
};

struct nvt_image encrypted_uboot = {
	.addr = 0x280000,
	.size = 0xC0000,
};

const char *warn =
"Warning: "
"the cmd will switch to secure boot mode!\n"
"         "
"There is no reliable way to recover them.\n"
"         "
"Use this command only for testing purposes if you\n"
"         "
"are sure of what you are doing!\n"
"\nReally do this cmd? <y/N>\n";

static int do_nvt_switch_sec(struct cmd_tbl * cmdtp, int flag, int argc, char *const argv[])
{
	char cmd[128];
	int is_secure = 0;
	int pass_yes = argc > 1 && !strcmp("-y", argv[1]);
#if (defined(CONFIG_NVT_IVOT_OPTEE_SUPPORT) && defined(CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT))
	NVT_SMC_EFUSE_DATA efuse_data = {0};
	efuse_data.cmd = NVT_SMC_EFUSE_IS_SECURE;
	is_secure = nvt_ivot_optee_efuse_operation(&efuse_data);
#elif defined(CONFIG_NVT_IVOT_SECBOOT_SUPPORT)
	is_secure = is_secure_enable();
#endif
	if (is_secure)
		return 0;
	
	if (!pass_yes) {
		puts(warn);
		if (!confirm_yesno()) {
			puts("aborted!!!\n");
			return 1;
		}
	}

	/* Write key */
	if (run_command("nvt_write_key 0", 0)) {
		nvt_dbg(ERR, "nvt_write_key 0 fail\n");
		return -1;
	}

	if (run_command("nvt_write_key 1", 0)) {
		nvt_dbg(ERR, "nvt_write_key 1 fail\n");
		return -1;
	}

	if (run_command("nvt_write_key 2", 0)) {
		nvt_dbg(ERR, "nvt_write_key 2 fail\n");
		return -1;
	}

	/* Switch to secure uboot */
	nvt_dbg(IND, "Switch to secure uboot\n");
	sprintf(cmd, "nand erase 0x%llx 0x%lx", uboot.addr, uboot.size);
	run_command(cmd, 0);
	sprintf(cmd, "nand read 0x%llx 0x%llx 0x%lx", nvt_memory_cfg[MEMTYPE_LINUXTMP].addr, encrypted_uboot.addr , encrypted_uboot.size);
	run_command(cmd, 0);
	sprintf(cmd, "nand write 0x%llx 0x%llx 0x%lx", nvt_memory_cfg[MEMTYPE_LINUXTMP].addr, uboot.addr, uboot.size);
	run_command(cmd, 0);

	nvt_dbg(IND, "Switch to secure ATF\n");
	sprintf(cmd, "nand erase 0x%llx 0x%lx", atf.addr, atf.size);
	run_command(cmd, 0);
	sprintf(cmd, "nand read 0x%llx 0x%llx 0x%lx", nvt_memory_cfg[MEMTYPE_LINUXTMP].addr, encrypted_atf.addr, encrypted_atf.size);
	run_command(cmd, 0);
	sprintf(cmd, "nand write 0x%llx 0x%llx 0x%lx", nvt_memory_cfg[MEMTYPE_LINUXTMP].addr, atf.addr, atf.size);
	run_command(cmd, 0);

	nvt_dbg(IND, "Switch to secure DTS\n");
	sprintf(cmd, "nand erase 0x%llx 0x%lx", dts.addr, dts.size);
	run_command(cmd, 0);
	sprintf(cmd, "nand read 0x%llx 0x%llx 0x%lx", nvt_memory_cfg[MEMTYPE_LINUXTMP].addr, encrypted_dts.addr, encrypted_dts.size);
	run_command(cmd, 0);
	sprintf(cmd, "nand write 0x%llx 0x%llx 0x%lx", nvt_memory_cfg[MEMTYPE_LINUXTMP].addr, dts.addr, dts.size);
	run_command(cmd, 0);

	nvt_dbg(IND, "Switch to secure Loader\n");
	sprintf(cmd, "nand erase 0x%llx 0x%lx", loader.addr, loader.size);
	run_command(cmd, 0);
	sprintf(cmd, "nand read 0x%llx 0x%llx 0x%lx", nvt_memory_cfg[MEMTYPE_LINUXTMP].addr, encrypted_loader.addr, encrypted_loader.size);
	run_command(cmd, 0);
	sprintf(cmd, "nand write 0x%llx 0x%llx 0x%lx", nvt_memory_cfg[MEMTYPE_LINUXTMP].addr, loader.addr, loader.size);
	run_command(cmd, 0);

	/* Enable secure mode */
#if defined(CONFIG_SIGNATUREMETHOD_ECDSA)
	if (run_command("nvt_secure_en data_encrypt", 0)) {
		nvt_dbg(ERR, "nvt_secure_en data_encrypt fail\n");
		return -1;
	}

	if (run_command("nvt_secure_en ecdsa_en", 0)) {
		nvt_dbg(ERR, "nvt_secure_en ecdsa_en fail\n");
		return -1;
	}

	if (run_command("nvt_secure_en ecdsa_chksum_en", 0)) {
		nvt_dbg(ERR, "nvt_secure_en ecdsa_chksum_en fail\n");
		return -1;
	}
#else
	if (run_command("nvt_secure_en data_encrypt", 0)) {
		nvt_dbg(ERR, "nvt_secure_en data_encrypt fail\n");
		return -1;
	}

	if (run_command("nvt_secure_en rsa_en", 0)) {
		nvt_dbg(ERR, "nvt_secure_en rsa_en fail\n");
		return -1;
	}

	if (run_command("nvt_secure_en rsa_chk_en", 0)) {
		nvt_dbg(ERR, "nvt_secure_en rsa_chk_en fail\n");
		return -1;
	}
#endif
	if (run_command("nvt_secure_en secure_en", 0)) {
		nvt_dbg(ERR, "nvt_write_key 2 fail\n");
		return -1;
	}

	/* Reboot to secure boot */
	if (run_command("reset", 0)) {
		nvt_dbg(ERR, "reset\n");
		return -1;
	}
}

/***************************************************/
U_BOOT_CMD(
	nvt_switch_sec, 2, 0, do_nvt_switch_sec,
	"nvt_switch_sec operation",
	"\nnvt_switch_sec - for switch to secure boot mode [NOTE!!! the cmd will set efuse and can not be rebcak]"
	"\nnvt_switch_sec -y to confirm to use"
);
