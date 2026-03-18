/**
    NVT logfile internal header file
    This file provides internal macros or definitions used by NVT logfile
    @file nvt_logfile_internal.h
    @ingroup
    @note
    Copyright Novatek Microelectronics Corp. 2022. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#ifndef __NVT_LOGFILE_INTERNAL_H__
#define __NVT_LOGFILE_INTERNAL_H__
#include <linux/types.h>
#include <linux/soc/nvt/nvt-pcie-lib.h>

#define NVT_LOGFILE_VERSION     "1.00.002"
#define NVT_LOGFILE_NAME        "nvt_logfile"

#define LOGFILE_BUFFER_SIZE     60*1024
#define LOGFILE_BASE            'l'
#define LOGFILE_IOCTL_FD        _IOR(LOGFILE_BASE, 0, int)

#define LOGFILE_EP_CNT          (CHIP_MAX - CHIP_EP0)

#define LOGFILE_FLAG_USING      (0x1 << 0)

typedef struct {
	unsigned long size;
	phys_addr_t rc_buf_pa[LOGFILE_EP_CNT];
	unsigned char flag[LOGFILE_EP_CNT];
} logfile_shmblk_t;

#define MAKEFOURCC(ch0, ch1, ch2, ch3)	((u32)(u8)(ch0) | ((u32)(u8)(ch1) << 8) | ((u32)(u8)(ch2) << 16) | ((u32)(u8)(ch3) << 24 ))

#define DBG_INFO(fmt, args...) printk("%s: "fmt, NVT_LOGFILE_NAME, ##args)

#ifndef DBG_ERR
#define DBG_ERR(fmt, args...) printk("ERR:%s() "fmt, __func__, ##args)
#endif

#ifndef DBG_WRN
#define DBG_WRN(fmt, args...) printk("WRN:%s() "fmt, __func__, ##args)
#endif

#ifndef DBG_DUMP
#define DBG_DUMP(fmt, args...) printk(fmt, ##args)
#endif

#ifndef DBG_IND
#if 0
#define DBG_IND(fmt, args...) printk("IND:%s() "fmt, __func__, ##args)
#else
#define DBG_IND(fmt, args...)
#endif
#endif

#ifndef CHKPNT
#define CHKPNT printk("CHK: %d, %s\r\n", __LINE__, __func__)
#endif

struct dma_buf* logfile_dmabuf_get_obj(void);

#endif
