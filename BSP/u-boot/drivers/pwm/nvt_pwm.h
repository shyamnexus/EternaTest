/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) Novatek Inc.
 */

#ifndef PWM_NVT_H
#define PWM_NVT_H

#include <common.h>
#include <dm.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/hardware.h>
#include <clk.h>

int do_nvt_pwm_set_pinmux(int pwm_id);
void nvt_pwm_set_pinmux(int pwm_id, u32 pwm_pinmux);
void nvt_pwm_set_div(int pwm_id, u32 div);
void pwm_enable_clk(int pwm_id);
bool pwmid_is_valid(int pwm_id);


#ifndef DBG_ERR
#define DBG_ERR(fmtstr, args...)  printf("\033[0;31mERR:%s() \033[0m" fmtstr, __func__, ##args)
#endif

#endif
