/**
    NVT utilities for command customization

    @file       nvt_debug_utils.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <command.h>
#include <asm/nvt-common/nvt_common.h>
#include "../nvt_ivot_soc_utils.h"
#include "../nvt_ivot_pack.h"

#ifdef CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT
int do_nvt_update_all_cmd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	ulong addr = 0, size = 0;
	int ret = 0;

	if (argc != 3) {
		return CMD_RET_USAGE;
	}

	ret = cmd_get_data_size(argv[0], 4);
	if (ret < 0) {
		return 1;
	}

	addr = simple_strtoul(argv[1], NULL, 16);
	size = simple_strtoul(argv[2], NULL, 10);

	printf("%s addr: 0x%08lx, size: 0x%08lx(%lu) bytes %s\r\n", ANSI_COLOR_YELLOW, addr, size, size, ANSI_COLOR_RESET);

	ret = nvt_process_all_in_one(addr, size, 0);
	if (ret < 0) {
		printf("Update nvt all-in-one image failed.\n");
		return ret;
	}

	return 0;
}

U_BOOT_CMD(
	nvt_update_all, 3,  0,  do_nvt_update_all_cmd,
	"To Update all-in-one image from memory address and size \n",
	"[address(hex)][size(dec)] \n"
);
#endif

#ifdef CONFIG_NVT_IVOT_DDR_RANGE_SCAN_SUPPORT
int do_nvt_ddr_scan(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
#if (defined(CONFIG_TARGET_NA51090) || defined(CONFIG_TARGET_NA51090_A64) || defined(CONFIG_TARGET_NA51103) || defined(CONFIG_TARGET_NA51103_A64) || defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64) || defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64) || defined(CONFIG_TARGET_NS02302_A64))
	if (argc < 2) {
		nvt_ddr_scan(0);
	} else {
		nvt_ddr_scan(simple_strtoul(argv[1], NULL, 16));
	}
#else
	nvt_ddr_scan();
#endif
	return 0;
}
#if (defined(CONFIG_TARGET_NA51090) || defined(CONFIG_TARGET_NA51090_A64) || defined(CONFIG_TARGET_NA51103) || defined(CONFIG_TARGET_NA51103_A64) || defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64) || defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64) || defined(CONFIG_TARGET_NS02302_A64))
U_BOOT_CMD(
	nvt_ddr_scan, 2,    1,  do_nvt_ddr_scan,
	"nvt ddr scan",
	""
);
#else
U_BOOT_CMD(
	nvt_ddr_scan, 1,    1,  do_nvt_ddr_scan,
	"nvt ddr scan",
	""
);
#endif
#endif

#ifdef CONFIG_NVT_IVOT_STBC_PMC_SUPPORT
int do_nvt_stbc_pmc(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
#if defined(CONFIG_TARGET_NA51102_A64)
	if (!strncmp(argv[1], "debug", 5)) {
		nvt_stbc_pmc(1);
	} else {
		nvt_stbc_pmc(0);
	}
#else
	printf("Not support\r\n");
#endif
	return 0;
}

U_BOOT_CMD(
	nvt_stbc_pmc, 2, 1,  do_nvt_stbc_pmc,
	"nvt stbc pd3",
	"[Option] \n"
	"              [debug]:auto wakeup\n"
	"              [normal]not auto wakeup\n"
	"              [others]not auto wakeup\n"
);
#endif


int do_nvt_rtos_disable_cmd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	rtos_disable_anchor = 1;
	return 0;
}


U_BOOT_CMD(
	nvt_rtos_disable,  1,  0,  do_nvt_rtos_disable_cmd,
	"To disable uboot boot rtos",
	"In order to disable fastboot, u-boot boot linux directly but rather rtos\n"
);
