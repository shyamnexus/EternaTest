/**
    NVT Logfile
    To store serial messages for debug
    @file       nvt_logfile.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef __SOC_NVT_LOGFILE_H
#define __SOC_NVT_LOGFILE_H
#include <linux/dma-buf.h>

typedef void (*logfile_save_str_ptr)(const char *instr, size_t count, int kernel_space);
typedef void (*logfile_save_syserr_ptr)(void);

void logfile_set_printk_ptr(logfile_save_str_ptr p); //printk.c (str)
void logfile_set_tty_ptr(logfile_save_str_ptr p); //tty_io.c (str)
void logfile_set_panic_ptr(logfile_save_syserr_ptr p); //panic.c (syserr)

#if IS_ENABLED(CONFIG_NVT_LOGFILE)

void logfile_save_str(const char *instr, size_t count, int kernel_space);
void logfile_save_syserr(void);

#else

static void logfile_save_str(const char *instr, size_t count, int kernel_space)
{
	return;
}
static void logfile_save_syserr(void)
{
	return;
}

#endif

#endif /* __SOC_NVT_LOGFILE_H */
