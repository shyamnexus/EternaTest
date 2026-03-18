/**
    STBC module header

    STBC module header file.

    @file       stbc.h
    @ingroup    mIDrvSys_CG
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/

#ifndef _STBC_H
#define _STBC_H

#include <kwrap/nvt_type.h>

#define MCU_OFS_DATA_PORT0		(0x20)	// command port
#define MCU_OFS_DATA_PORT1		(0x24)
#define MCU_OFS_DATA_PORT2		(0x28)
#define MCU_OFS_DATA_PORT3		(0x2C)

// software defined control on data port 0
#define MCU_CMD_APB_READ		(1<<0)
#define MCU_CMD_APB_WRITE		(2<<0)
#define MCU_CMD_PWR_SET			(3<<0)
#define MCU_CMD_PWR_GET			(4<<0)
#define MCU_CMD_SFR_READ		(5<<0)
#define MCU_CMD_SFR_WRITE		(6<<0)
#define MCU_CMD_SLEEP			(7<<0)
#define MCU_CMD_ISOLATION		(8<<0)
#define MCU_CMD_TIMER_START		(9<<0)
#define MCU_CMD_TIMER_STOP		(10<<0)
#define MCU_CMD_TIMER_READ		(11<<0)
#define MCU_CMD_TIMER_CHECK		(12<<0)
#define MCU_CMD_TIMER_AUTO_TEST		(13<<0)
#define MCU_CMD_SUSPEND_USB		(14<<0)
#define MCU_CMD_MSK			(0x7FFFFFFF)
#define MCU_CMD_TRIG_MSK		(1<<31)


typedef enum {
	STBC_PWR_PAD0,
	STBC_PWR_PAD1,
	STBC_PWR_PAD2,

	ENUM_DUMMY4WORD(STBC_PWR_PAD)
} STBC_PWR_PAD;


extern ER stbc_open(void);
extern ER stbc_close(void);

extern UINT32 stbc_apb_read(uintptr_t addr);
extern void stbc_apb_write(uintptr_t addr, UINT32 value);

extern void stbc_power_pad_set(STBC_PWR_PAD pad, BOOL value);
extern UINT32 stbc_power_pad_get(STBC_PWR_PAD pad);

extern UINT8 stbc_sfr_read(UINT8 addr);
extern void stbc_sfr_write(UINT8 addr, UINT8 value);

extern void stbc_sleep(void);
extern BOOL stbc_cmd_is_busy(void);

extern void stbc_isolation(void);

extern void stbc_timer_start(UINT32 target);
extern void stbc_timer_stop(void);
extern UINT32 stbc_timer_read(void);
extern BOOL stbc_timer_expired(void);
extern void stbc_suspened_en(void);

#endif
