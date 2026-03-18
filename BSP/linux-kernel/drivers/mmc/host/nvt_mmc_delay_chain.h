#ifndef _SDIO_DELAY_CHAIN_H
#define _SDIO_DELAY_CHAIN_H

#include "nvt_mmchost.h"
#include <linux/uaccess.h>
#include <plat/pad.h>
#include <plat/top.h>

extern u32 host_dll_check[3];
extern u8 tuning_test;

extern u32 golden_unit[3];
extern u32 golden_data_phase[3];
extern u32 golden_cmd_phase[3];
extern u32 golden_data_dll[3];
extern u32 golden_cmd_dll[3];

extern void nvt_mmc_chain_type(void);
extern int nvt_mmc_auto_tuning_with_dLL(struct mmc_host *mmc, u32 opcode);
extern int nvt_mmc_auto_tuning_phase_unit(struct mmc_host *mmc, u32 opcode);
extern void nvt_mmc_hs400_complete_with_dLL(struct mmc_host *mmc);
extern void nvt_mmc_hs400_complete_phase_unit(struct mmc_host *mmc);

#endif