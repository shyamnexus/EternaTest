/*
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    @file       ctl_sen_export.c
*/

#include <linux/module.h>
#include "ctl_sen_int.h"
#include "ctl_sen_debug_infor_int.h"

EXPORT_SYMBOL(kflow_ctl_sen_init);
EXPORT_SYMBOL(kflow_ctl_sen_uninit);
EXPORT_SYMBOL(ctl_sen_init_buf);
EXPORT_SYMBOL(ctl_sen_uninit_buf);
EXPORT_SYMBOL(ctl_sen_buf_query);

EXPORT_SYMBOL(ctl_sen_init_map);
EXPORT_SYMBOL(ctl_sen_uninit_map);
EXPORT_SYMBOL(ctl_sen_add_map);
EXPORT_SYMBOL(ctl_sen_del_map);

EXPORT_SYMBOL(ctl_sen_senid_info);
EXPORT_SYMBOL(ctl_sen_get_senid);

EXPORT_SYMBOL(ctl_sen_open);
EXPORT_SYMBOL(ctl_sen_close);
EXPORT_SYMBOL(ctl_sen_sleep);
EXPORT_SYMBOL(ctl_sen_wakeup);
EXPORT_SYMBOL(ctl_sen_status);
EXPORT_SYMBOL(ctl_sen_pwr_ctrl);
EXPORT_SYMBOL(ctl_sen_chgmode);
EXPORT_SYMBOL(ctl_sen_set);
EXPORT_SYMBOL(ctl_sen_get);
EXPORT_SYMBOL(ctl_sen_write_reg);
EXPORT_SYMBOL(ctl_sen_read_reg);
EXPORT_SYMBOL(ctl_sen_wait_intruupt);

EXPORT_SYMBOL(ctl_sen_reg_sendrv);
EXPORT_SYMBOL(ctl_sen_unreg_sendrv);

EXPORT_SYMBOL(ctl_sen_dbg_dump_info);
EXPORT_SYMBOL(ctl_sen_dbg_dump_signal);
EXPORT_SYMBOL(ctl_sen_dbg_dump_process);


