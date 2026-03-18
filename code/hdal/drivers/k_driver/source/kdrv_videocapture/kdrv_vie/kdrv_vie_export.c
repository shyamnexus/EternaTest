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
#include "kdrv_vie_int.h"
#include "kdrv_vie_debug_int.h"

EXPORT_SYMBOL(kdrv_vie_open);
EXPORT_SYMBOL(kdrv_vie_close);
EXPORT_SYMBOL(kdrv_vie_resume);
EXPORT_SYMBOL(kdrv_vie_suspend);
EXPORT_SYMBOL(kdrv_vie_set);
EXPORT_SYMBOL(kdrv_vie_get);
EXPORT_SYMBOL(kdrv_vie_trigger);
EXPORT_SYMBOL(kdrv_vie_buf_query);
EXPORT_SYMBOL(kdrv_vie_buf_init);
EXPORT_SYMBOL(kdrv_vie_buf_uninit);
EXPORT_SYMBOL(kdrv_vie_init);
EXPORT_SYMBOL(kdrv_vie_uninit);
EXPORT_SYMBOL(kdrv_vie_get_vie_limit);
//dbg
EXPORT_SYMBOL(kdrv_vie_dump_info);
EXPORT_SYMBOL(kdrv_vie_dbg_set_dbg_level);
EXPORT_SYMBOL(kdrv_vie_set_checksum_en);

EXPORT_SYMBOL(kdrv_vie_dump_intrpt_sts);
EXPORT_SYMBOL(kdrv_vie_set_err_log_rate);
