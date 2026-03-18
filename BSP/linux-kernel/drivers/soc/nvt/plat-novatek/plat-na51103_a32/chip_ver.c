/**
    NVT chip version function
    This file will get chip version
    @file       chip_ver.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/string.h>
#include <linux/printk.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/soc/nvt/nvt-io.h>
#include <plat/hardware.h>

//
// ROM code version number
//
#define TOP_ROMVER_A            0x00000101
#define TOP_ROMVER_B            0x00000102

u32 nvt_get_chip_ver(phys_addr_t base)
{
	unsigned int chip_ver;
	void __iomem *rom_base;

	rom_base = ioremap(base + 0x7ffc, 4);
	chip_ver = nvt_readl(rom_base);
	iounmap(rom_base);
    if(chip_ver == TOP_ROMVER_A) {
        return CHIPVER_A;
    } else if(chip_ver == TOP_ROMVER_B) {
        return CHIPVER_B;
    } else {
		pr_info("force to Ver B\n");
        return CHIPVER_B;
    }

	return chip_ver;
}
EXPORT_SYMBOL_GPL(nvt_get_chip_ver);
