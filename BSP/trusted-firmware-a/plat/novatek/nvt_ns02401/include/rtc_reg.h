/*
 * Copyright (c) 2024, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DRIVERS_NVT_TZRTC_REG_H
#define __DRIVERS_NVT_TZRTC_REG_H


#ifndef ENUM_DUMMY4WORD
#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000
#endif

#define REGVALUE        uint32_t
#define UBITFIELD		unsigned int 	/* Unsigned bit field */

// Macros for Register Cache Word (RCW) type definition
//
// Each RCW type should be exactly the same size with REGVALUE type
// For example, to declare a Register Cache Word type:
//
//     #define rcwname_OFS   0x00   /* the name of RCW corresponding register address offset
//                                     should be in specific format with "_OFS" appended */
//     REGDEF_BEGIN(rcwname)
//         REGDEF_BIT(field1, 8)    /* declare field1 as 8 bits width */
//         REGDEF_BIT(field2, 8)    /* declare field1 as 8 bits width */
//         REGDEF_BIT(, 16)         /* pad reserved (not-used) bits to fill RCW type same as REGVALUE size */
//     REGDEF_END(rcwname)
//
// Register Cache Word type defintion header
#define REGDEF_BEGIN(name)      \
typedef union                   \
{                               \
    REGVALUE    reg;            \
    struct                      \
    {

// Register Cache Word bit defintion
#define REGDEF_BIT(field, bits) \
    UBITFIELD   field : bits;

// Register Cache Word type defintion trailer
#define REGDEF_END(name)        \
    } bit;                      \
} T_##name;                     \


// RTC //
#define RTC_TIMER_REG_OFS    0x00
//REGDEF_OFFSET(RTC_TIMER_OFS, 0x00)
REGDEF_BEGIN(RTC_TIMER_REG)
REGDEF_BIT(Sec, 6)
REGDEF_BIT(Min, 6)
REGDEF_BIT(Hour, 5)
REGDEF_BIT(, 15)
REGDEF_END(RTC_TIMER_REG)

#define RTC_DAYKEY_REG_OFS    0x04
//REGDEF_OFFSET(RTC_DAYKEY_REG, 0x04)
REGDEF_BEGIN(RTC_DAYKEY_REG)
REGDEF_BIT(Day, 16)
REGDEF_BIT(Key, 4)
REGDEF_BIT(, 12)
REGDEF_END(RTC_DAYKEY_REG)


// RTC Alarm Register
#define  RTC_ALARM_REG_OFS 0X08
//REGDEF_OFFSET(RTC_ALARM_REG, 0x08)
REGDEF_BEGIN(RTC_ALARM_REG)
REGDEF_BIT(sec, 6)
REGDEF_BIT(min, 6)
REGDEF_BIT(hour, 5)
REGDEF_BIT(, 15)
REGDEF_END(RTC_ALARM_REG)

// RTC Status Register
#define RTC_STATUS_REG_OFS 0x0C
//REGDEF_OFFSET(RTC_STATUS_REG, 0x0C)
REGDEF_BEGIN(RTC_STATUS_REG)
REGDEF_BIT(Alarm_Sts, 1)
REGDEF_BIT(SRST_Sts, 1)
REGDEF_BIT(Ready, 1)
REGDEF_BIT(, 2)
REGDEF_BIT(SRST_SIGSTS, 1)
REGDEF_BIT(, 2)
REGDEF_BIT(CSET_Sts, 1)
REGDEF_BIT(, 23)
REGDEF_END(RTC_STATUS_REG)

// RTC Control Register
#define RTC_CTRL_REG_OFS 0x10
//REGDEF_OFFSET(RTC_CTRL_REG, 0x10)
REGDEF_BEGIN(RTC_CTRL_REG)
REGDEF_BIT(Alarm_IntEn, 1)
REGDEF_BIT(SRST, 1)
REGDEF_BIT(CSET, 1)
REGDEF_BIT(Time_Sel, 1)
REGDEF_BIT(Day_Sel, 1)
REGDEF_BIT(Key_Sel, 1)
REGDEF_BIT(PWRAlarmTime_Sel, 1)
REGDEF_BIT(PWRAlarmDay_Sel, 1)
REGDEF_BIT(CSET_IntEn, 1)
REGDEF_BIT(, 3)
REGDEF_BIT(DA0_Sel, 1)
REGDEF_BIT(, 3)
REGDEF_BIT(PWR_EN2_CONFIG, 1)
REGDEF_BIT(PWR_EN3_SEL, 1)
REGDEF_BIT(, 14)
REGDEF_END(RTC_CTRL_REG)

// RTC Data Register
#define RTC_DATA_REG_OFS 0x14
//REGDEF_OFFSET(RTC_DATA_REG, 0x14)
REGDEF_BEGIN(RTC_DATA_REG)
REGDEF_BIT(rtc_data0, 16)
REGDEF_BIT(, 16)
REGDEF_END(RTC_DATA_REG)

// RTC Power Button Control Register
#define RTC_PWBC_REG_OFS 0x18
//REGDEF_OFFSET(RTC_PWBC_REG, 0x18)
REGDEF_BEGIN(RTC_PWBC_REG)
REGDEF_BIT(Reset_SDT_Timer, 1)
REGDEF_BIT(PWR_Off, 1)
REGDEF_BIT(, 1)
REGDEF_BIT(PWRAlarm_En, 1)
REGDEF_BIT(PWRAlarm_Dis, 1)
REGDEF_BIT(PWR_EN2_LATCH, 1)
REGDEF_BIT(PWR_EN3_CTRL, 1)
REGDEF_BIT(, 17)
REGDEF_BIT(PWR_SW1_IntEn, 1)
REGDEF_BIT(PWR_SW2_IntEn, 1)
REGDEF_BIT(, 6)
REGDEF_END(RTC_PWBC_REG)

// RTC Power Button Control Status Register
#define RTC_PWBCSTS_REG_OFS 0X1C
//REGDEF_OFFSET(RTC_PWBCSTS_REG, 0x1C)
REGDEF_BEGIN(RTC_PWBCSTS_REG)           // --> Register "RTC_PWBCSTS_REG" begin ---
REGDEF_BIT(PWRAlarm_EnSts, 1)
REGDEF_BIT(CorePWR_Valid, 1)
REGDEF_BIT(, 6)
REGDEF_BIT(PWROnSrc_SW1, 1)
REGDEF_BIT(PWROnSrc_SW2, 1)
REGDEF_BIT(PWROnSrc_SW3, 1)
REGDEF_BIT(PWROnSrc_SW4, 1)
REGDEF_BIT(PWROn_PwrEn2_Log, 1)
REGDEF_BIT(, 2)
REGDEF_BIT(PWROnSrc_PWRAlarm, 1)
REGDEF_BIT(PWR_SW1_Pin, 1)
REGDEF_BIT(PWR_SW2_Pin, 1)
REGDEF_BIT(PWR_SW3_Pin, 1)
REGDEF_BIT(PWR_SW4_Pin, 1)
REGDEF_BIT(PWR_EN2_Sts, 1)
REGDEF_BIT(, 3)
REGDEF_BIT(PWR_SW1_Sts, 1)
REGDEF_BIT(PWR_SW2_Sts, 1)
REGDEF_BIT(, 6)
REGDEF_END(RTC_PWBCSTS_REG)


// RTC Power Alarm Timer Register
#define RTC_PWRALM_REG_OFS 0x20
//REGDEF_OFFSET(RTC_PWRALM_REG, 0x20)
REGDEF_BEGIN(RTC_PWRALM_REG)
REGDEF_BIT(Sec, 6)
REGDEF_BIT(Min, 6)
REGDEF_BIT(Hour, 5)
REGDEF_BIT(, 3)
REGDEF_BIT(Day, 5)
REGDEF_BIT(, 7)
REGDEF_END(RTC_PWRALM_REG)   

// RTC OSC Analog Register
#define RTC_OSCAN_REG_OFS 0X24
//REGDEF_OFFSET(RTC_OSCAN_REG, 0x24)
REGDEF_BEGIN(RTC_OSCAN_REG)
REGDEF_BIT(OSC_AnalogCfg, 8)
REGDEF_BIT(, 24)
REGDEF_END(RTC_OSCAN_REG)

//@}
#endif
