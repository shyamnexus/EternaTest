/*
 * Copyright (c) 2023, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NOVATEK_PRIVATE_H
#define NOVATEK_PRIVATE_H

#include <stddef.h>

#define NVT_HEADINFO_CHIP_NAME "bl02302"
void nova_setup_page_tables(uintptr_t total_base, size_t total_size,
							uintptr_t code_start, uintptr_t code_limit,
							uintptr_t rodata_start, uintptr_t rodata_limit
						   );

void plat_tmr_init(void);
void nvt_tzasc_config(void);

/* RTC - PWBC related */
void nvt_pwbc_power_control(int reboot_sec);
void nvt_pwbc_power_off(void);

struct rtc_time {
	uintptr_t tm_sec;
	uintptr_t tm_min;
	uintptr_t tm_hour;
	uintptr_t tm_mday;
	uintptr_t tm_mon;
	uintptr_t tm_year;
	uintptr_t tm_wday;
	uintptr_t tm_yday;
	uintptr_t tm_isdst;
};

#endif /* NOVATEK_PRIVATE_H */
