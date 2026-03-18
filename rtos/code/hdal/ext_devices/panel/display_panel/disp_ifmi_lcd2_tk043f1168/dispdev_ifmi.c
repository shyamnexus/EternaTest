/*
    Display device object Golden Sample for driving RGB/YUV memory interface panel

    @file       dispdev_ifmi.c
    @ingroup
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/
#include "dispdev_ifmi.h"
#include <kdrv_builtin/kdrv_builtin.h>


static void     disp_device_init(T_LCD_PARAM *p_mode);


static ER       dispdev_open_ifmi(void);
static ER       dispdev_close_ifmi(void);
static void     dispdev_write_lcd_reg(UINT32 ui_addr, UINT32 ui_value);
static UINT32   dispdev_read_lcd_reg(UINT32 ui_addr);
static void     dispdev_set_ifmi_ioctrl(FP p_io_ctrl_func);
static ER       dispdev_set_device_rotate(DISPDEV_LCD_ROTATE rot);
static ER       dispdev_get_lcd_size(PDISPDEV_GET_PRESIZE t_lcd_size);


/*
    Exporting display device object
*/
#if (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD1)
static DISPDEV_OBJ     disp_dev_ifmi_obj = { dispdev_open_ifmi, dispdev_close_ifmi, dispdev_set_device_rotate, dispdev_write_lcd_reg, dispdev_read_lcd_reg, dispdev_get_lcd_size, dispdev_set_ifmi_ioctrl, { NULL, NULL, NULL } };
static volatile DISPDEV_IOCTRL  p_ifmi_io_control1;
#elif (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD2)
static DISPDEV_OBJ     disp_dev_ifmi_obj2 = { dispdev_open_ifmi, dispdev_close_ifmi, dispdev_set_device_rotate, dispdev_write_lcd_reg, dispdev_read_lcd_reg, dispdev_get_lcd_size, dispdev_set_ifmi_ioctrl, { NULL, NULL, NULL } };
static volatile DISPDEV_IOCTRL  p_ifmi_io_control2;
#endif




static void disp_device_init(T_LCD_PARAM *p_mode)
{
	UINT32                  i;
	DISPDEV_IOCTRL_PARAM    dev_io_ctrl;
#if (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD1)
	DISPDEV_IOCTRL          p_disp_dev_control = p_ifmi_io_control1;
#elif (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD2)
	DISPDEV_IOCTRL          p_disp_dev_control = p_ifmi_io_control2;
#endif

	dispdev_set_lcd_colour2();
	p_disp_dev_control(DISPDEV_IOCTRL_GET_PANEL_ADJUST, &dev_io_ctrl);
	if (dev_io_ctrl.SEL.GET_PANEL_ADJUST.pfp_adjust != NULL) {
		dev_io_ctrl.SEL.GET_PANEL_ADJUST.pfp_adjust();
	} else if (p_mode->p_cmd_queue != NULL) {
		for (i = 0; i < p_mode->n_cmd; i++) {
			if (p_mode->p_cmd_queue[i].ui_address == CMDDELAY_MS) {
				dispdev_platform_delay_ms(p_mode->p_cmd_queue[i].ui_value);
			} else if (p_mode->p_cmd_queue[i].ui_address == CMDDELAY_US) {
				dispdev_platform_delay_us(p_mode->p_cmd_queue[i].ui_value);
			} else {
				dispanl_debug(("[0x%x, 0x%x]\n", p_mode->p_cmd_queue[i].ui_address, p_mode->p_cmd_queue[i].ui_value));
				dispdev_write_lcd_reg(p_mode->p_cmd_queue[i].ui_address, p_mode->p_cmd_queue[i].ui_value);
			}
		}
	}

}

#if 1
/*
    Display Device API of open LCD with RGB/YUV Serial 8bits interface

    Display Device API of open LCD with RGB/YUV Serial 8bits interface.

    @param[in] p_devif   Please refer to dispdevctrl.h for details.

    @return
     - @b E_PAR:    Error p_devif content.
     - @b E_NOSPT:  Driver without supporting the specified LCD mode.
     - @b E_OK:     LCD Open done and success.
*/
static ER dispdev_open_ifmi(void)
{
	UINT32                  i;
	T_LCD_PARAM              *p_mode      = NULL;
	T_LCD_PARAM              *p_config_mode = NULL;
	PINMUX_LCDINIT          lcd_mode;
	MI_CONFIG              mi_config;
	UINT32                  mode_number;
	PINMUX_FUNC_ID          pin_func_id;
#if (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD1)
	DISPDEV_IOCTRL          p_disp_dev_control = p_ifmi_io_control1;
#elif (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD2)
	DISPDEV_IOCTRL          p_disp_dev_control = p_ifmi_io_control2;
#endif
#if defined(__KERNEL__)
    int fastboot = kdrv_builtin_is_fastboot();
#endif
	dispanl_debug(("openIFMI START\r\n"));

	if (p_disp_dev_control == NULL) {
		return E_PAR;
	}

	//
	//  Sortig the SysInit Config mode support
	//
#if (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD1)
	pin_func_id = PINMUX_FUNC_ID_LCD;
#elif ((DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD2))
	pin_func_id = PINMUX_FUNC_ID_LCD2;
#endif
	lcd_mode = dispdev_platform_get_disp_mode((UINT32)pin_func_id) & (~PINMUX_LCDMODE_AUTO_PINMUX);

#if (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD1)
	p_config_mode = dispdev_get_config_mode(&mode_number);
#elif ((DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD2))
	p_config_mode = dispdev_get_config_mode2(&mode_number);
#endif

	if ((p_config_mode != NULL) && (mode_number > 0)) {
		for (i = 0; i < mode_number; i++) {
			if (p_config_mode[i].panel.lcd_mode == lcd_mode) {
				p_mode = (T_LCD_PARAM *)&p_config_mode[i];
				break;
			}
		}
	}

	if (p_mode == NULL) {
		DBG_ERR("lcd_mode=%d not support\r\n", lcd_mode);
		return E_NOSPT;
	}

	if (mi_isOpened()) {
		mi_close();
	}

	if(mi_open()) return E_NOSPT;
	switch (lcd_mode) {
	case PINMUX_LCDMODE_MI_FMT0:
		mi_config.mi_fmt = MI_DATAFMT0;
		break;
	case PINMUX_LCDMODE_MI_FMT1:
		mi_config.mi_fmt = MI_DATAFMT1;
		break;
	case PINMUX_LCDMODE_MI_FMT2:
		mi_config.mi_fmt = MI_DATAFMT2;
		break;
	case PINMUX_LCDMODE_MI_FMT3:
		mi_config.mi_fmt = MI_DATAFMT3;
		break;
	case PINMUX_LCDMODE_MI_FMT4:
		mi_config.mi_fmt = MI_DATAFMT4;
		break;
	case PINMUX_LCDMODE_MI_FMT5:
		mi_config.mi_fmt = MI_DATAFMT5;
		break;
	case PINMUX_LCDMODE_MI_FMT6:
		mi_config.mi_fmt = MI_DATAFMT6;
		break;
	case PINMUX_LCDMODE_MI_FMT7:
		mi_config.mi_fmt = MI_DATAFMT7;
		break;
	case PINMUX_LCDMODE_MI_FMT8:
		mi_config.mi_fmt = MI_DATAFMT8;
		break;
	case PINMUX_LCDMODE_MI_FMT9:
		mi_config.mi_fmt = MI_DATAFMT9;
		break;
	case PINMUX_LCDMODE_MI_FMT10:
		mi_config.mi_fmt = MI_DATAFMT10;
		break;
	case PINMUX_LCDMODE_MI_FMT11:
		mi_config.mi_fmt = MI_DATAFMT11;
		break;
	case PINMUX_LCDMODE_MI_FMT12:
		mi_config.mi_fmt = MI_DATAFMT12;
		break;
	case PINMUX_LCDMODE_MI_FMT13:
		mi_config.mi_fmt = MI_DATAFMT13;
		break;
	case PINMUX_LCDMODE_MI_FMT14:
		mi_config.mi_fmt = MI_DATAFMT14;
		break;
	case PINMUX_LCDMODE_MI_SERIAL_BI:
		mi_config.mi_fmt = MI_DATAFMT1;
		break;

	default:
		mi_config.mi_fmt = MI_DATAFMT0;
		DBG_ERR("openIFMi not supported mode! \r\n");
		break;
	}	
#if _FPGA_EMULATION_
	mi_config.mi_src_clk = 24.0;
#else	
	//mi_config.mi_src_clk = 240.0;
#endif	
	dispdev_set_mi_config2(&mi_config);
	mi_setManual(TRUE);

	// Config & Enable Display physical engine
	dispdev_set_display(p_disp_dev_control, T_LCD_INF_MI, p_mode, pin_func_id);

	// Config & Enable display device
#if defined(__KERNEL__)
    if(!fastboot)
#endif
    disp_device_init(p_mode);
	dispdev_set_mi_AutoCmd2();
	mi_setSrc(0);// 2:IDE2, 0:IDE1

	mi_setManual(FALSE);
	mi_setEn(TRUE);

	dispanl_debug(("openIF8Bits Done\r\n"));

	return E_OK;
}

/*
    Close LCD display panel.

    Call this api to close the LCD panel device.

    @param[in] p_devif Please refer to dispdevctrl.h for details.

    @return
     - @b E_OK:     LCD Close done and success.
*/
static ER dispdev_close_ifmi(void)
{
	UINT32                  i, cmd_number;
	T_PANEL_CMD              *p_standby = NULL;
	DISPDEV_IOCTRL_PARAM    dev_io_ctrl;
	PINMUX_FUNC_ID          pin_func_id;
#if (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD1)
	DISPDEV_IOCTRL          p_disp_dev_control = p_ifmi_io_control1;
#elif (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD2)
	DISPDEV_IOCTRL          p_disp_dev_control = p_ifmi_io_control2;
#endif
	MI_TYPE     			Type = PARALLEL_I80;
	MI_AUTO_FMT             AutoType = MI_AUTOFMT0;
	MI_DATAFMT              DataFMT = MI_DATAFMT0;
	BOOL                    bRGBSWAP = FALSE;
	BOOL                    bBITSWAP = FALSE;
	BOOL                    bSync = FALSE;
	BOOL                    bPINMUX = FALSE;


	dispanl_debug(("closeIFMI START\r\n"));
	mi_getCtrl(&Type, &AutoType, &DataFMT, &bRGBSWAP, &bBITSWAP, &bSync, &bPINMUX);

	if (bSync == FALSE) {
		mi_setEn(FALSE);
	}	

	// Close Display Engine First
	dev_io_ctrl.SEL.SET_ENABLE.b_en = FALSE;
	p_disp_dev_control(DISPDEV_IOCTRL_SET_ENABLE, &dev_io_ctrl);
	p_disp_dev_control(DISPDEV_IOCTRL_WAIT_FRAMEEND, NULL);
	if (bSync == TRUE) {
		mi_setEn(FALSE);
	}	

#if (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD1)
	dev_io_ctrl.SEL.SET_CLK_EN.b_clk_en = FALSE;
	p_disp_dev_control(DISPDEV_IOCTRL_SET_CLK_EN, &dev_io_ctrl);
#elif (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD2)
	dev_io_ctrl.SEL.SET_CLK_EN.b_clk_en = FALSE;
	p_disp_dev_control(DISPDEV_IOCTRL_SET_CLK_EN, &dev_io_ctrl);
#endif
	mi_setManual(TRUE);
	mi_setCtrl(Type,   AutoType,  DataFMT,  bRGBSWAP,  bBITSWAP,  FALSE,  bPINMUX);

#if (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD1)
	p_standby = dispdev_get_standby_cmd(&cmd_number);
#elif (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD2)
	p_standby = dispdev_get_standby_cmd2(&cmd_number);
#endif

	if (p_standby != NULL) {
		// Set display device into Standby or powerdown.
		for (i = 0; i < cmd_number; i++) {
			if (p_standby[i].ui_address == CMDDELAY_MS) {
				dispdev_platform_delay_ms(p_standby[i].ui_value);
			} else if (p_standby[i].ui_address == CMDDELAY_US) {
				dispdev_platform_delay_us(p_standby[i].ui_value);
			} else {			
				dispdev_write_lcd_reg(p_standby[i].ui_address, p_standby[i].ui_value);
			}
		}
	}
	mi_close();

	// close pinmux if needed.
#if (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD1)
	pin_func_id = PINMUX_FUNC_ID_LCD;
#elif ((DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD2))
	pin_func_id = PINMUX_FUNC_ID_LCD2;
#endif
	if (dispdev_platform_get_disp_mode((UINT32)pin_func_id) & PINMUX_LCDMODE_AUTO_PINMUX) {
	  dispdev_platform_set_pinmux(pin_func_id, PINMUX_LCD_SEL_GPIO);
	}

	dispanl_debug(("closeIF8Bits Done\r\n"));
	return E_OK;
}

/*
    Write LCD Register

    Write LCD Register

    @param[in] p_devif   Please refer to dispdevctrl.h for details.
    @param[in] ui_addr   LCD Register Offset.
    @param[in] ui_value  Assigned LCD Register value

    @return void
*/
static void dispdev_write_lcd_reg(UINT32 ui_addr, UINT32 ui_value)
{
	dispdev_write_lcd_mi2(ui_addr, ui_value);
	//return E_OK;
}

/*
    PreGet LCD size

    PreGet LCD size

    @param[in] p_devif   Please refer to dispdevctrl.h for details.
    @param[in] ui_addr   LCD Register Offset.
    @param[in] ui_value  Assigned LCD Register value

    @return void
*/
static ER dispdev_get_lcd_size(DISPDEV_GET_PRESIZE *t_size)
{
	UINT32                  i;
	T_LCD_PARAM              *p_mode      = NULL;
	T_LCD_PARAM              *p_config_mode = NULL;
	PINMUX_LCDINIT          lcd_mode;
	UINT32                  mode_number;
	PINMUX_FUNC_ID          pin_func_id;
	//DISPDEV_IOCTRL_PARAM    dev_io_ctrl;

	t_size->ui_buf_width = 0;
	t_size->ui_buf_height = 0;
	t_size->ui_win_width = 0;
	t_size->ui_win_height = 0;

	dispanl_debug(("%s\n", __FUNCTION__));
	//
	//  Sortig the SysInit Config mode support
	//
#if (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD1)
	pin_func_id = PINMUX_FUNC_ID_LCD;
#elif((DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD2))
	pin_func_id = PINMUX_FUNC_ID_LCD2;
#endif
	lcd_mode = dispdev_platform_get_disp_mode(pin_func_id);
	dispanl_debug(("lcd_mode=%d\n", (int)lcd_mode));

#if (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD1)
	p_config_mode = dispdev_get_config_mode(&mode_number);
#elif((DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD2))
	p_config_mode = dispdev_get_config_mode2(&mode_number);
#endif
	if ((p_config_mode != NULL) && (mode_number > 0)) {
		for (i = 0; i < mode_number; i++) {
			if (p_config_mode[i].panel.lcd_mode == lcd_mode) {
				p_mode = (T_LCD_PARAM *)&p_config_mode[i];
				break;
			}
		}
	}

	if (p_mode == NULL) {
		dispanl_debug(("lcd_mode=%d not support\r\n", (int)lcd_mode));
		return E_NOSPT;
	}

	t_size->ui_buf_width = p_mode->panel.ui_buffer_width;
	t_size->ui_buf_height = p_mode->panel.ui_buffer_height;
	t_size->ui_win_width = p_mode->panel.ui_window_width;
	t_size->ui_win_height = p_mode->panel.ui_window_height;

	return E_OK;
}




/*
    Read LCD Register

    Read LCD Register

    @param[in] p_devif   Please refer to dispdevctrl.h for details.
    @param[in] ui_addr   LCD Register Offset.

    @return Read back LCD Register value
*/
static UINT32  dispdev_read_lcd_reg(UINT32 ui_addr)
{
	UINT32                  ui_value = 0x0;

	dispdev_read_lcd_mi2(ui_addr, &ui_value);
	return ui_value;
}

/*
    Set Display Device IOCTRL function pointer
*/
static void dispdev_set_ifmi_ioctrl(FP p_io_ctrl_func)
{
#if (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD1)
	p_ifmi_io_control1 = (DISPDEV_IOCTRL)p_io_ctrl_func;
#elif (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD2)
	p_ifmi_io_control2 = (DISPDEV_IOCTRL)p_io_ctrl_func;
#endif
}

/*
    Set LCD rotation
*/
static ER dispdev_set_device_rotate(DISPDEV_LCD_ROTATE rot)
{
	UINT32          mode_no, index, i;
	T_LCD_ROT        *p_cmd_rot = NULL;
	BOOL            b_valid = FALSE;

#if (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD1)
	p_cmd_rot = dispdev_get_lcd_rotate_cmd(&mode_no);
#elif (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD2)
	p_cmd_rot = dispdev_get_lcd_rotate_cmd2(&mode_no);
#endif

	// Search Valid Rotattion Config table index
	for (index = 0; index < mode_no; index++) {
		if (p_cmd_rot[index].rotation == rot) {
			b_valid = TRUE;
			break;
		}
	}

	if (b_valid == FALSE) {
		DBG_ERR("DispDev NO valid rotate!\r\n\r\n");
		return E_PAR;
	}

	mi_setEn(FALSE);
	dispdev_platform_delay_ms(40);
	mi_setManual(TRUE);

	if (p_cmd_rot[index].p_cmd_queue != NULL) {
		for (i = 0; i < p_cmd_rot[index].n_cmd; i++) {
			if (p_cmd_rot[index].p_cmd_queue[i].ui_address == CMDDELAY_MS) {
				dispdev_platform_delay_ms(p_cmd_rot[index].p_cmd_queue[i].ui_value);
			} else if (p_cmd_rot[index].p_cmd_queue[i].ui_address == CMDDELAY_US) {
				dispdev_platform_delay_us(p_cmd_rot[index].p_cmd_queue[i].ui_value);
			} else {
				dispdev_write_lcd_reg(p_cmd_rot[index].p_cmd_queue[i].ui_address, p_cmd_rot[index].p_cmd_queue[i].ui_value);
			}
		}
	}
	dispdev_set_mi_AutoCmd2();

	mi_setManual(FALSE);
	mi_setEn(TRUE);

	return E_OK;
}

#endif


#if (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD1)
/**
    Get LCD 1 Device Object

    Get LCD 1 Device Object. This Object is used to control the LCD Device.

    @return The pointer of LCD Display Device object.
*/
PDISPDEV_OBJ dispdev_get_lcd1_dev_obj(void)
{
	return &disp_dev_ifmi_obj;
}

#elif (DISPLCDSEL_IFMI_TYPE == DISPLCDSEL_IFMI_LCD2)
/**
    Get LCD 2 Device Object

    Get LCD 2 Device Object. This Object is used to control the LCD Device.

    @return The pointer of LCD Display Device object.
*/

PDISPDEV_OBJ dispdev_get_lcd2_dev_obj(void)
{
	return &disp_dev_ifmi_obj2;
}
#endif

