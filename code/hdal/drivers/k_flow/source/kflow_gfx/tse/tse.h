/*
    Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.

    @file       kflow_tse.h

    @brief      kflow tse

    @version    V1.00.000
    @author     Novatek FW Team
    @date       2022/01/21
*/
#ifndef _KFLOW_TSE_H
#define _KFLOW_TSE_H

#if defined(__LINUX)
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#endif

extern int  kflow_tse_init(void);

extern void kflow_tse_exit(void);

extern int  kflow_tse_ioctl(int f, unsigned int cmd, void *arg);

extern int  kflow_tse_copy_from_user(void *dst, void *src, int len);
#endif

