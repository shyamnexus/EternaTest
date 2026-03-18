/*
    Display object object Golden Sample for driving Memory interface panel

    @file       dispdev_gsinfmi.h
    @ingroup
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/
#ifndef _DISPDEV_GSINFMI_H
#define _DISPDEV_GSINFMI_H

#ifdef __KERNEL__
#include "dispdev_panlcomn.h"
#include "mi.h"
#include "pll.h"

#elif defined __FREERTOS

#include "dispdev_panlcomn.h"
#include "mi.h"
#include "pll.h"
#endif
// Default debug level
#ifndef __DBGLVL__
#define __DBGLVL__  2       // Output all message by default. __DBGLVL__ will be set to 1 via make parameter when release code.
#endif

// Default debug filter
#ifndef __DBGFLT__
#define __DBGFLT__  "*"     // Display everything when debug level is 2
#endif


#define CMDDELAY_MS 0x80000000
#define CMDDELAY_US 0x40000000

#define MICMD_CMD   0x01000000
#define MICMD_DATA  0x02000000




typedef struct {
	MI_DATAFMT      mi_fmt;
	FLOAT           mi_src_clk;
} MI_CONFIG, *PMI_CONFIG;




T_LCD_PARAM *dispdev_get_config_mode2(UINT32 *mode_number);
T_PANEL_CMD *dispdev_get_standby_cmd2(UINT32 *cmd_number);

T_LCD_PARAM *dispdev_get_config_mode(UINT32 *mode_number);
T_PANEL_CMD *dispdev_get_standby_cmd(UINT32 *cmd_number);

void dispdev_set_lcd_colour(void);
void dispdev_set_lcd_colour2(void);
void dispdev_write_lcd_mi(UINT32 addr, UINT32 value);
void dispdev_write_lcd_mi2(UINT32 addr, UINT32 value);
void dispdev_read_lcd_mi( UINT32 addr, UINT32 *pValue);
void dispdev_read_lcd_mi2( UINT32 addr, UINT32 *pValue);

T_LCD_ROT *dispdev_get_lcd_rotate_cmd(UINT32 *mode_number);
T_LCD_ROT *dispdev_get_lcd_rotate_cmd2(UINT32 *mode_number);

void dispdev_set_mi_config(MI_CONFIG *p_mi_config);
void dispdev_set_mi_config2(MI_CONFIG *p_mi_config);
void dispdev_set_mi_AutoCmd(void);
void dispdev_set_mi_AutoCmd2(void);

#endif
