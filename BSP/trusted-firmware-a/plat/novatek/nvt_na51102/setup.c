/*
 * Copyright (c) 2021, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <libfdt.h>
#include <platform_def.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <novatek_def.h>
#include <novatek_private.h>

#include <assert.h>
#include <rtc_reg.h>
#include <ddr_para.h>
#include <drivers/delay_timer.h>
#include <plat/common/platform.h>

extern void plat_boot_cpu_save_data(unsigned long cpu_data);
extern void plat_restore_cpu_data(unsigned long cpu_data);
extern void plat_restore_content(void);

#define RTC_INT_MAX_PWRALARM_DAY    0x1F
#define RTC_INT_OSC_ANALOG_CFG      0x5A86

void plat_inv_pagetable_cache(unsigned long ttbr0_el3, unsigned int addr)
{
	unsigned long lv1, lv2, offset;
	unsigned long addr_mask = 0x0000fffffffff000;

	inv_dcache_range(ttbr0_el3, 0x1000);
	lv1 = *(unsigned long *)ttbr0_el3 & addr_mask;
	inv_dcache_range(lv1, 0x1000);
	offset = (((unsigned long)addr>>21) & 0x1ff)<<3;
	lv2 = *(unsigned long *)(offset + lv1) & addr_mask;
	inv_dcache_range(lv2, 0x1000);
}

void plat_mmap_s3_data_area(unsigned int addr)
{
	unsigned long data_base = mmio_read_32(S3_DATA_PTR_ADDR);
	mmap_add_dynamic_region(addr, addr,
					0x10000, MT_MEMORY | MT_RW | MT_NON_CACHEABLE);
	if (read_sctlr_el3() & SCTLR_C_BIT)
		plat_inv_pagetable_cache(read_ttbr0_el3(), addr);

	plat_save_system_data((void *)data_base);
}

void plat_memcpy_word(void *dst, void *src, unsigned int len)
{
	unsigned int i;
	for (i = 0; i < len; i+=4)
		*(unsigned int *) (dst+i) = *(unsigned int *) (src+i);
}

void plat_jump_to_sram(void)
{
	__asm__ volatile(
			"SUB   SP, SP, #16\n"
			"STP   X29, X30, [SP]\n"
			"MOVZ X1, #0xf0e0, lsl #16\n"
			"ADD   X1, X1, #0xB0\n"
			"MOV   X2, #1\n"
			"LSL   X2, X2, #33\n"
			"ORR   X1, X2, X1\n"
			"MOV   X0, #3\n"
			"BLR   X1\n"
			"LDP   X29, X30, [SP]\n"
			"ADD   SP, SP, #16\n"
			: : :);
	mmio_write_32(UART0_BASE, 'A');
}

void plat_s3_jump_to_sram(void)
{
	unsigned long lr,s3_data_base;

	__asm__ volatile ("MOV %0, X30\n" : "=r" (lr));
	plat_boot_cpu_save_data(lr);

	s3_data_base = mmio_read_32(S3_DATA_PTR_ADDR);
	mmio_write_32(s3_data_base+0x10,(unsigned long)plat_restore_content);
	mmio_write_32(S3_ATF_DATA_PTR_ADDR, s3_data_base);
	dcsw_op_all(DCCISW);
	plat_jump_to_sram();
}

int plat_get_power_mode(void)
{
	if ((*(volatile unsigned int *)S3_MODE_ADDR) == 0x0)
		return POWER_KEEP_MODE;
	else
		return POWER_DOWN_MODE;
}
int setup(uint16_t socid, const void *fdt)
{
	int ret = 0;

	return ret;
}

#define YEAR_OFFSET 1900
uint32_t rtc_ydays[2][13] = {
        /* Normal years */
        { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
        /* Leap years */
        { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};
static int is_leap_year(unsigned int year)
{
	return (!(year % 4) && (year % 100)) || !(year % 400);
}

static void rtc_setreg( uint32_t ofs, uint32_t value)
{
	mmio_write_32( RTC_BASE + ofs , value);
}

uint32_t rtc_getreg(uint32_t ofs)
{
	return mmio_read_32( RTC_BASE + ofs );
}

static void pwbc_setreg( uint32_t ofs, uint32_t value)
{
	mmio_write_32(  PWBC_BASE + ofs , value);
}

uint32_t pwbc_getreg(uint32_t ofs)
{
	return mmio_read_32( PWBC_BASE + ofs);
}

void rtc_trigger_cset(void)
{
	int count;
	T_RTC_CTRL_REG ctrl_reg;

	/*Wait for RTC SRST done*/
	for (count=0; count<5; count++) {
		ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
		if (ctrl_reg.bit.srst == 0) {
			break;
		}
		mdelay(1);
	}

	/*Wait for RTC is ready for next CSET*/
	for (count=0; count<5; count++) {
		ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
		if (ctrl_reg.bit.cset == 0) {
			break;
		}
		mdelay(1);
	}

	/*Trigger CSET*/
	ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
	ctrl_reg.bit.cset = 1;
	ctrl_reg.bit.cset_inten = 0;
	rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);
}

void nvt_rtc_wait_cset_done(void)
{
	int count;
	T_RTC_STATUS_REG sts_reg;

	/*Wait for previous CSET done */
	for(count=0; count<10; count++) {
		sts_reg.reg = rtc_getreg(RTC_STATUS_REG_OFS);
		if (sts_reg.bit.cset_sts == 1) {
			break;
		}
		mdelay(5);
	}
	sts_reg.bit.cset_sts = 1 ;
	rtc_setreg(RTC_STATUS_REG_OFS, sts_reg.reg );

}


void rtc_set_pwralarm_en(int en)
{
	T_RTC_CTRL_REG ctrl_reg;

	ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
	ctrl_reg.bit.cset = 0;
	ctrl_reg.bit.cset_inten = 0;
	ctrl_reg.bit.pwralarmday_sel = 1;
	rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);

	ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
	ctrl_reg.bit.pwralarm_en = en;
	rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);
}

void nvt_rtc_wait_clr_alarm_sts(void)
{
	int count;
	T_RTC_STATUS_REG sts_reg;

	/*Wait for previous CSET done */
	for(count=0; count<20; count++) {
		sts_reg.reg = rtc_getreg(RTC_STATUS_REG_OFS);
		if (sts_reg.bit.pwralarm_ensts == 0) {
			break;
		}
		mdelay(500);
	}
	if(count == 20 ){
		NOTICE("nvt_rtc_wait_clr_alarm_sts time out  \r\n");
	}
}

static int nvt_rtc_read_time( struct rtc_time *tm)
{
	uint32_t days , months, years, month_days;
	T_RTC_TIMER_REG timer_reg = {0};
	T_RTC_DAYKEY_REG daykey_reg = {0};
	int count = 0;

    // Check day collision
	for (count=0; count<5; count++) {
		daykey_reg.reg = rtc_getreg(RTC_DAYKEY_REG_OFS);
		if (daykey_reg.bit.collision == 0) {
			break;
		}
		mdelay(5);
	}

	days = daykey_reg.bit.day;

	for (years = 0; days >= rtc_ydays[is_leap_year(years + 1900)][12]; years++) {
		days -= rtc_ydays[is_leap_year(years + 1900)][12];
	}

	for (months = 1; months < 13; months++) {
		if (days <= rtc_ydays[is_leap_year(years + 1900)][months]) {
			days -= rtc_ydays[is_leap_year(years + 1900)][months-1];
			months--;
			break;
		}
	}

	month_days = rtc_ydays[is_leap_year(years + 1900)][months+1] - \
		rtc_ydays[is_leap_year(years + 1900)][months];

	if (days == month_days) {
		months++;
		days = 1;
	} else
		days++;

    // Check time collision
	for (count=0; count<3; count++) {
		timer_reg.reg = rtc_getreg(RTC_TIMER_REG_OFS);
		if (timer_reg.bit.collision == 0) {
			break;
		}
		mdelay(1);
	}

	tm->tm_sec  = timer_reg.bit.sec;
	tm->tm_min  = timer_reg.bit.min;
	tm->tm_hour = timer_reg.bit.hour;
	tm->tm_mday = days;
	tm->tm_mon  = months;
	tm->tm_year = years;

	 return 0;
}

static void nvt_pwbc_add_one_sec(uint32_t* sec, uint32_t* min, uint32_t* hour, uint32_t* days)
{
	if (*sec == 59) {
		if (*min == 59) {
			*hour += 1;
			*min = 0;
			*sec = 0;
		} else {
			*min += 1;
			*sec = 0;
		}
	} else {
		*sec += 1;
	}

	if (*hour > 23) {
		*hour = 0;
		*days += 1;
	}
}

void nvt_pwbc_trigger_cset(void)
{
   	T_PWBC_CTRL2_REG pwbcctrl2_reg;

	/*Trigger CSET*/
	pwbcctrl2_reg.reg = pwbc_getreg(PWBC_CTRL2_REG_OFS);
	pwbcctrl2_reg.bit.pwbc_cset = 1;
	pwbcctrl2_reg.bit.pwbc_cset_inten = 0;
	pwbc_setreg(PWBC_CTRL2_REG_OFS, pwbcctrl2_reg.reg);
}

void nvt_pwbc_wait_cset_done(void)
{
	int count;
	T_PWBC_STS_REG pwbc_sts_reg;

	/*Wait for previous CSET done */
	for (count=0; count<10; count++) {
		pwbc_sts_reg.reg = pwbc_getreg(PWBC_STS_REG_OFS);
		if (pwbc_sts_reg.bit.pwbc_cset_done == 1) {
			break;
		}
		mdelay(5);
	}

	// clear cset done
	pwbc_sts_reg.bit.pwbc_cset_done =1;
	pwbc_setreg(PWBC_STS_REG_OFS, pwbc_sts_reg.reg);
}


void nvt_pwbc_reset_shutdown_timer(void)
{
	int	 count;
	T_PWBC_CTRL_REG pwbcctrl_reg;
	T_PWBC_CTRL2_REG pwbcctrl2_reg;

	/*Wait for previous CSET done */
	for (count=0; count<5; count++) {
		pwbcctrl2_reg.reg = pwbc_getreg(PWBC_CTRL2_REG_OFS);
		if (pwbcctrl2_reg.bit.pwbc_cset == 0) {
			break;
		}
		mdelay(1);
	}

	pwbcctrl2_reg.reg = pwbc_getreg(PWBC_CTRL2_REG_OFS);
	pwbcctrl2_reg.bit.pwbc_cset = 0;
	pwbcctrl2_reg.bit.seq_time1_sel = 1;
	pwbc_setreg(PWBC_CTRL2_REG_OFS, pwbcctrl2_reg.reg);

	pwbcctrl_reg.reg = pwbc_getreg(PWBC_CTRL_REG_OFS);
	pwbcctrl_reg.bit.reset_sdt_timer = 1;
	pwbc_setreg(PWBC_CTRL_REG_OFS, pwbcctrl_reg.reg);

	nvt_pwbc_trigger_cset();
	nvt_pwbc_wait_cset_done();
}

void nvt_pwbc_power_off(void)
{
	int	 count;
	T_PWBC_CTRL_REG		pwbcctrl_reg;
	T_PWBC_CTRL2_REG	pwbcctrl2_reg;

	/*Wait for previous CSET done */
	for (count=0; count<5; count++) {
		pwbcctrl2_reg.reg = pwbc_getreg(PWBC_CTRL2_REG_OFS);
		if (pwbcctrl2_reg.bit.pwbc_cset == 0) {
			break;
		}
		mdelay(1);
	}

	pwbcctrl2_reg.reg = pwbc_getreg(PWBC_CTRL2_REG_OFS);
	pwbcctrl2_reg.bit.pwbc_cset = 0;
	pwbcctrl2_reg.bit.seq_time1_sel = 1;
	pwbc_setreg(PWBC_CTRL2_REG_OFS, pwbcctrl2_reg.reg);


	pwbcctrl_reg.reg = pwbc_getreg(PWBC_CTRL_REG_OFS);
	pwbcctrl_reg.bit.pwr_off = 1;
	pwbcctrl_reg.bit.reset_sdt_timer = 1;
	pwbc_setreg(PWBC_CTRL_REG_OFS, pwbcctrl_reg.reg);

	/*Trigger CSET*/
	pwbcctrl2_reg.reg = pwbc_getreg(PWBC_CTRL2_REG_OFS);
	pwbcctrl2_reg.bit.pwbc_cset = 1;
	pwbc_setreg(PWBC_CTRL2_REG_OFS, pwbcctrl2_reg.reg);
}

int nvt_rtc_set_alarm( struct rtc_time *tm)
{
	int year_looper;
	uint32_t current_days = 0, alarm_days = 0;
	T_RTC_DAYKEY_REG daykey_reg;
	T_RTC_CTRL_REG ctrl_reg;
	T_RTC_PWRALM_REG pwralarm_reg;
	T_RTC_OSCAN_REG oscan_reg;

	daykey_reg.reg = rtc_getreg(RTC_DAYKEY_REG_OFS);
	current_days = daykey_reg.bit.day;

	for (year_looper = 0; year_looper < tm->tm_year; year_looper++)
		alarm_days += rtc_ydays[is_leap_year(year_looper + 1900)][12];

	alarm_days += rtc_ydays[is_leap_year(year_looper + 1900)][tm->tm_mon];
	tm->tm_mday--; /*subtract the day which is not ended*/
	alarm_days += tm->tm_mday;

	/*Check date parameter for maximum register setting*/
	if ((alarm_days < current_days) || ((alarm_days - current_days) > \
	RTC_INT_MAX_PWRALARM_DAY)) {
		NOTICE("Invalid parameter!\n");
		return 0;
	}

	rtc_set_pwralarm_en(1);

	ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
	ctrl_reg.bit.cset_inten = 0;
	ctrl_reg.bit.cset = 0;
	ctrl_reg.bit.pwralarmtime_sel = 1;
	ctrl_reg.bit.pwralarmday_sel = 1;
	ctrl_reg.bit.alarm_inten = 1;

	rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);

	pwralarm_reg.reg = 0;
	pwralarm_reg.bit.sec = tm->tm_sec;
	pwralarm_reg.bit.min = tm->tm_min;
	pwralarm_reg.bit.hour = tm->tm_hour;
	pwralarm_reg.bit.day = alarm_days;
	rtc_setreg(RTC_PWRALM_REG_OFS, pwralarm_reg.reg);

	/*Set OSC analog parameter*/
	oscan_reg.reg = rtc_getreg(RTC_OSCAN_REG_OFS);
	oscan_reg.bit.osc_analogcfg = RTC_INT_OSC_ANALOG_CFG;
	rtc_setreg(RTC_OSCAN_REG_OFS, oscan_reg.reg);
	rtc_trigger_cset();

	nvt_rtc_wait_cset_done();

#if 0
	pwralarm_reg.reg = rtc_getreg(RTC_PWRALM_REG_OFS);
	NOTICE("alarm sec = %d \n",(uint32_t)pwralarm_reg.bit.sec );
	NOTICE("alarm min = %d \n",(uint32_t)pwralarm_reg.bit.min);
	NOTICE("alarm hour = %d \n",(uint32_t)pwralarm_reg.bit.hour);
	NOTICE("alarm days = %d \n",(uint32_t)pwralarm_reg.bit.day);

	ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
	NOTICE("ctrl_reg.reg = 0x%x \n",(uint32_t)ctrl_reg.reg);
	NOTICE("pwralarm_reg = 0x%x \n",(uint32_t)pwralarm_reg.reg);
#endif

	return 0;
}

void nvt_pwbc_power_control(int reboot_sec)
{

	struct	rtc_time cur_time;
	uint32_t  sec, min, hour, days;
	int i;

	rtc_set_pwralarm_en(0);
	rtc_trigger_cset();

	nvt_rtc_wait_cset_done();
	nvt_rtc_wait_clr_alarm_sts();

	nvt_rtc_read_time(&cur_time);

	sec  = cur_time.tm_sec;
	min  = cur_time.tm_min;
	hour = cur_time.tm_hour;
	days = cur_time.tm_mday;

#if 0
	NOTICE("RTC Time sec = %d \r\n", sec);
	NOTICE("RTC Time min = %d \r\n", min);
	NOTICE("RTC Time hour = %d \r\n", hour);
	NOTICE("RTC Time days = %d \r\n", days);
#endif

	for (i = 0;i < reboot_sec; i++){
		nvt_pwbc_add_one_sec(&sec, &min, &hour, &days);
	}

	cur_time.tm_sec  = sec;
	cur_time.tm_min  = min;
	cur_time.tm_hour = hour;
	cur_time.tm_mday = days;

	// set pwr_alarm time
	nvt_rtc_set_alarm(&cur_time);

	// set reset shutdown timer and power off
	nvt_pwbc_reset_shutdown_timer();
	nvt_pwbc_power_off();
}
