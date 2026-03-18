/**
    NVT emmc info header
 
    @file       nvt_ivot_emmc.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2020.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef __SOC_NVT_IVOT_NVT_IVOT_EMMC_H
#define __SOC_NVT_IVOT_NVT_IVOT_EMMC_H

#include <linux/blk_types.h>

struct nvt_ivot_emmc_partition {
	u64 start;
	u64 num;
	bool	active;
	const char *name; // store partition label name(from device tree definition) 
	const char *part_type; // store partition type(from device tree definition) 
	struct device_node *of_node;
	int partno;
	char card_id[32];
	char disk_name[32];
};

struct nvt_ivot_emmc_total_partitions {
	struct nvt_ivot_emmc_partition *part_info;
	unsigned int nr_parts;
	unsigned short rootfs_num;
};

#ifdef CONFIG_MMC_NVT_EMMC_INFO
struct hd_struct *nvt_add_partition(struct gendisk *disk, int partno,
		sector_t start, sector_t len, int flags);
#endif

#endif /* __SOC_NVT_IVOT_NVT_IVOT_EMMC_H */
