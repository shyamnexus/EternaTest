/*
    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.

    @file       ctl_dre_export.c
    @ingroup

    @brief

    @note       Nothing.

    @version    V0.00.001
    @author     XL Wei
    @date       2020/05/12
*/
#include <linux/module.h>
#include "kflow_videoprocess/ctl_dre.h"
#include "kflow_videoprocess/ctl_dre_isp.h"
#include "ctl_dre_int.h"

#if CTL_DRE_MODULE_ENABLE
EXPORT_SYMBOL(ctl_dre_open);
EXPORT_SYMBOL(ctl_dre_close);
EXPORT_SYMBOL(ctl_dre_ioctl);
EXPORT_SYMBOL(ctl_dre_set);
EXPORT_SYMBOL(ctl_dre_get);
EXPORT_SYMBOL(ctl_dre_query);
EXPORT_SYMBOL(ctl_dre_init);
EXPORT_SYMBOL(ctl_dre_uninit);

EXPORT_SYMBOL(ctl_dre_isp_evt_fp_reg);
EXPORT_SYMBOL(ctl_dre_isp_evt_fp_unreg);
EXPORT_SYMBOL(ctl_dre_isp_set);
EXPORT_SYMBOL(ctl_dre_isp_get);
EXPORT_SYMBOL(ctl_dre_dump_all);
#endif
