/**
    NVT common header

    @file       nvt-info.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef __SOC_NVT_IVOT_NVT_INFO_H
#define __SOC_NVT_IVOT_NVT_INFO_H

extern struct proc_dir_entry *nvt_info_dir_root;

void nvt_bootts_add_ts(char *name);

unsigned long nvt_get_time(void);
int nvt_get_suspend_mode(void);
#endif /* __SOC_NVT_IVOT_NVT_INFO_H */
