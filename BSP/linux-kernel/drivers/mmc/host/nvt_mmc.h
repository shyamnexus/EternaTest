/**
    NVT mmc function
    Define parameters and initial register value
    @file       nvt_mmc.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef _NVT_MMC_H
#define _NVT_MMC_H

#define MMC_MODULE_VER      "3.12.006"

extern int nvt_mmc_get_cd_polled(struct mmc_host *mmc);
extern int nvt_mmc_get_ro_polled(struct mmc_host *mmc);
extern int nvt_mmcsd_probe(struct platform_device *pdev, const struct of_device_id *of_dt_ids, const char *proc_path);
extern int nvt_mmcsd_remove(struct platform_device *pdev);
extern int nvt_mmcsd_suspend(struct device *dev);
extern int nvt_mmcsd_resume(struct device *dev);

extern void nvt_rescan_card(unsigned id, unsigned insert);


// for delay chain
extern void nvt_mmc_chain_type(void);
extern int nvt_mmc_auto_tuning_with_dLL(struct mmc_host *mmc, u32 opcode);
extern int nvt_mmc_auto_tuning_phase_unit(struct mmc_host *mmc, u32 opcode);
extern void nvt_mmc_hs400_complete_with_dLL(struct mmc_host *mmc);
extern void nvt_mmc_hs400_complete_phase_unit(struct mmc_host *mmc);

#endif
