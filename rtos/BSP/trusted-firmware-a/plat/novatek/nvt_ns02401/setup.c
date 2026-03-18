/*
 * Copyright (c) 2024, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <libfdt.h>
#include <platform_def.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <novatek_def.h>
#include <novatek_private.h>

#include <assert.h>
#include <rtc_reg.h>
#include <drivers/delay_timer.h>
#include <plat/common/platform.h>

#define RTC_INT_MAX_PWRALARM_DAY    0x1F
#define RTC_INT_OSC_ANALOG_CFG      0x9

int setup(uint16_t socid, const void *fdt)
{
	int ret = 0;

	return ret;
}

#ifdef RTC_BASE
// RTC
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

void rtc_trigger_cset(void)
{
    int count;
	T_RTC_CTRL_REG ctrl_reg;
	T_RTC_STATUS_REG sts_reg;
	
	/*Wait for RTC SRST done*/
	for (count=0; count<5; count++) {
		sts_reg.reg = rtc_getreg(RTC_STATUS_REG_OFS);
		if (sts_reg.bit.SRST_Sts == 0) {
			break;
		}
		mdelay(1);
	}

	/*Wait for RTC is ready for next CSET*/
	for (count=0; count<5; count++) {
		ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
		if (ctrl_reg.bit.CSET == 0) {
			break;
		}
		mdelay(1);
	}

	/*Trigger CSET*/
	ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
	ctrl_reg.bit.CSET = 1;
	ctrl_reg.bit.CSET_IntEn = 0;
	rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);
}

void nvt_rtc_wait_cset_done(void)
{
	int count;
	T_RTC_STATUS_REG sts_reg;

	/*Wait for previous CSET done */
	for(count=0; count<10; count++) {
		sts_reg.reg = rtc_getreg(RTC_STATUS_REG_OFS);
		if (sts_reg.bit.CSET_Sts == 1) {
			NOTICE("nvt_rtc_wait_cset_done - 1\r\n");
			break;
		}
		mdelay(10);
	}
	NOTICE("nvt_rtc_wait_cset_done - 2\r\n");
	sts_reg.bit.CSET_Sts = 1 ;
	rtc_setreg(RTC_STATUS_REG_OFS, sts_reg.reg );

}

void rtc_set_pwralarm_en(int en)
{
	T_RTC_PWBC_REG pwbc_reg;
	
	pwbc_reg.reg = rtc_getreg(RTC_PWBC_REG_OFS);
	if (en == 1 ){
		pwbc_reg.bit.PWRAlarm_En = 1;
	} else {
		pwbc_reg.bit.PWRAlarm_Dis = 1;
	}
	rtc_setreg(RTC_PWBC_REG_OFS, pwbc_reg.reg);
}

void nvt_rtc_wait_clr_alarm_sts(void)
{
	int count;
	T_RTC_PWBCSTS_REG pwbc_reg;

	/*Wait for previous CSET done */
	for(count=0; count<20; count++) {
		pwbc_reg.reg = rtc_getreg(RTC_PWBCSTS_REG_OFS);
		if (pwbc_reg.bit.PWRAlarm_EnSts == 0) {
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
	
	daykey_reg.reg = rtc_getreg(RTC_DAYKEY_REG_OFS);
	days = daykey_reg.bit.Day;
	
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

	timer_reg.reg = rtc_getreg(RTC_TIMER_REG_OFS);
	
	tm->tm_sec  = timer_reg.bit.Sec;
	tm->tm_min  = timer_reg.bit.Min;
	tm->tm_hour = timer_reg.bit.Hour;
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

void nvt_pwbc_reset_shutdown_timer(void)
{
    T_RTC_PWBC_REG pwbc_reg;

	pwbc_reg.reg = rtc_getreg(RTC_PWBC_REG_OFS);
	pwbc_reg.bit.Reset_SDT_Timer = 1;
	rtc_setreg(RTC_PWBC_REG_OFS, pwbc_reg.reg);

}

void nvt_pwbc_power_off(void)
{
    int count;
	T_RTC_CTRL_REG ctrl_reg;
	T_RTC_PWBC_REG pwbc_reg;

	/*Wait for RTC is ready for next CSET*/
	for (count=0; count<5; count++) {
		ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
		if (ctrl_reg.bit.CSET == 0) {
			break;
		}
		mdelay(1);
	}
	
	pwbc_reg.reg = rtc_getreg(RTC_PWBC_REG_OFS);
	pwbc_reg.bit.PWR_Off = 1;
	rtc_setreg(RTC_PWBC_REG_OFS, pwbc_reg.reg);

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
	current_days = daykey_reg.bit.Day;

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

	ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
	ctrl_reg.bit.CSET_IntEn = 0;
	ctrl_reg.bit.CSET = 0;
	ctrl_reg.bit.PWRAlarmTime_Sel = 1;
	ctrl_reg.bit.PWRAlarmDay_Sel = 1;
	ctrl_reg.bit.Alarm_IntEn = 1;
	rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);

	pwralarm_reg.reg = 0;
	pwralarm_reg.bit.Sec = tm->tm_sec;
	pwralarm_reg.bit.Min = tm->tm_min;
	pwralarm_reg.bit.Hour = tm->tm_hour;
	pwralarm_reg.bit.Day = alarm_days;
	rtc_setreg(RTC_PWRALM_REG_OFS, pwralarm_reg.reg);
	
	/*Set OSC analog parameter*/
	oscan_reg.reg = rtc_getreg(RTC_OSCAN_REG_OFS);
	oscan_reg.bit.OSC_AnalogCfg = RTC_INT_OSC_ANALOG_CFG;
	rtc_setreg(RTC_OSCAN_REG_OFS, oscan_reg.reg);
	rtc_trigger_cset();

	nvt_rtc_wait_cset_done();
	rtc_set_pwralarm_en(1);
	
#if 0
	pwralarm_reg.reg = rtc_getreg(RTC_PWRALM_REG_OFS);
	NOTICE("alarm sec = %d \n",(uint32_t)pwralarm_reg.bit.Sec );
	NOTICE("alarm min = %d \n",(uint32_t)pwralarm_reg.bit.Min);
	NOTICE("alarm hour = %d \n",(uint32_t)pwralarm_reg.bit.Hour);
	NOTICE("alarm days = %d \n",(uint32_t)pwralarm_reg.bit.Day);
	
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
#endif