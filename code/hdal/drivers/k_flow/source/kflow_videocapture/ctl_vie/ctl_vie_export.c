/*
    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.

    @file       kdrv_vie_export.c
    @ingroup

    @brief

    @note       Nothing.

    @version    V0.00.001
    @author     Lincy Lin
    @date       2020/04/28
*/

#include <linux/module.h>
#include "kdf_vie_int.h"
#include "ctl_vie_int.h"
#include "ctl_vie_debug_int.h"
#include "ctl_vie_isp_int.h"
#include "ctl_vie_utility_int.h"

EXPORT_SYMBOL(kflow_ctl_vie_init);
EXPORT_SYMBOL(kflow_ctl_vie_uninit);
EXPORT_SYMBOL(ctl_vie_buf_query);
EXPORT_SYMBOL(ctl_vie_init);
EXPORT_SYMBOL(ctl_vie_uninit);
EXPORT_SYMBOL(ctl_vie_set);
EXPORT_SYMBOL(ctl_vie_get);
EXPORT_SYMBOL(ctl_vie_open);
EXPORT_SYMBOL(ctl_vie_close);
EXPORT_SYMBOL(ctl_vie_suspend);
EXPORT_SYMBOL(ctl_vie_resume);
EXPORT_SYMBOL(ctl_vie_get_dbg_tab);
EXPORT_SYMBOL(ctl_vie_reg_dbg_tab);
EXPORT_SYMBOL(ctl_vie_set_dbg_lvl);
EXPORT_SYMBOL(ctl_vie_spt);
//dbg
EXPORT_SYMBOL(ctl_vie_dbg_set_msg_type);
EXPORT_SYMBOL(ctl_vie_dbg_dump_isr_ioctl);
EXPORT_SYMBOL(ctl_vie_dbg_dump_proc_time);
//isp
EXPORT_SYMBOL(ctl_vie_isp_evt_fp_reg);
EXPORT_SYMBOL(ctl_vie_isp_evt_fp_unreg);
EXPORT_SYMBOL(ctl_vie_isp_set);
EXPORT_SYMBOL(ctl_vie_isp_get);
EXPORT_SYMBOL(ctl_vie_isp_set_skip);
EXPORT_SYMBOL(ctl_vie_isp_get_hdl);
//kdf
EXPORT_SYMBOL(kdf_vie_buf_query);
EXPORT_SYMBOL(kdf_vie_set_ccir_header);
EXPORT_SYMBOL(kdf_vie_init);
EXPORT_SYMBOL(kdf_vie_uninit);
EXPORT_SYMBOL(kdf_vie_set);
EXPORT_SYMBOL(kdf_vie_get);
EXPORT_SYMBOL(kdf_vie_open);
EXPORT_SYMBOL(kdf_vie_close);
EXPORT_SYMBOL(kdf_vie_trigger);
EXPORT_SYMBOL(kdf_vie_suspend);
EXPORT_SYMBOL(kdf_vie_resume);
EXPORT_SYMBOL(kdf_vie_get_limit);

EXPORT_SYMBOL(kdf_vie_dump_intrpt_sts);
EXPORT_SYMBOL(kdf_vie_set_err_log_rate);

