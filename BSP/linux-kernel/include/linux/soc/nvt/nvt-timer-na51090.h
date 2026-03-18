/**
    NVT timer function
    @file       nvt-timer-na51090.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef __SOC_NVT_IVOT_NVT_TIMER_NA51090_H
#define __SOC_NVT_IVOT_NVT_TIMER_NA51090_H

#define MAX_VALUE 0xFFFFFFFF
#define SHIFT       32ULL
#define MAX_HZ 3000000
#define MAX_DIV_BIT 255
#define TIMER_DIV_SRC0_DEF 0xFFFFFF00
#define TIMER_DIV_SRC1_DEF 0xFFFF00FF
#define TIMER_FREE_RUN(src) (0x3 + (src * 0x4))
#define TIMER_ONE_SHOT(src) (0x1 + (src * 0x4))
#define TIMER_DIV_BASE 0x30
#define TIMER_DIV_VAL(src) (1 << ( 16 + src) )
#define TIMER_IRQ_STS 0x10
#define TIMER_IRQ_BASE 0x20
#define TIMER_IRQ_VAL(num) (1 << num)
#define TIMER_CTL_BASE(num) (0x10 * num + 0x100)
#define TIMER_TVAL_BASE(num) (TIMER_CTL_BASE(num) + 0x04)
#define TIMER_VALUE_BASE(num) (TIMER_CTL_BASE(num) + 0x08)
#define TIMER_RLD_BASE(num) (TIMER_CTL_BASE(num) + 0x0C)
#define TIMER_ENABLE 1
#define TIMER_DISABLE 0
#define TIMER_FREQ_SHIFT(src) (src * 8)
#define TIMER_UP(src) (0x1 + 0x4 * src)
#define TIMER_DOWN (~1UL)

#define MAX_TIMER_NUM 20
#define LINUX_TIMER_NUM0 18
#define LINUX_TIMER_NUM1 19

typedef int (*nvt_timer_callback_t)(void);

struct nvt_timer_t {
	struct device *dev;
	unsigned int in_use;
	struct clk *clk;
	unsigned int clksrc[MAX_TIMER_NUM];
	nvt_timer_callback_t callback[MAX_TIMER_NUM];
};

int nvt_timer_get_count(unsigned int num, unsigned int *count);
int nvt_timer_shutdown(unsigned int num);
int nvt_timer_setup(unsigned int num, unsigned int clksrc, bool freerun, unsigned int count, nvt_timer_callback_t callback);
int nvt_timer_reload(unsigned int num, unsigned int count);
int nvt_timer_stop(unsigned int num);
int nvt_timer_start(unsigned int num);
void nvt_timer_dump(void);

void nvt_timer_proc_exit(void);
int nvt_timer_proc_init(struct platform_device *pdev);

#endif /* __SOC_NVT_IVOT_NVT_TIMER_NA51090_H */
