/*
    Display object for driving MI device

    @file       panel.c
    @ingroup
    @note       This panel MUST select ide clock to PLL1 ( 480 ). Once change to \n
				another frequence, the _IDE_FDCLK should be re calculated

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/
#include "dispdev_ifmi.h"
#include "mi.h"



#define MICMD_CMD   0x01000000
#define MICMD_DATA  0x02000000

#define CMDDELAY_MS 0x80000000
#define CMDDELAY_US 0x40000000

/*
    panel Parameters for NT75761
*/
//@{
/*Used in MI*/
const T_PANEL_CMD t_cmd_mode_mi2[] = {
	{MICMD_CMD,     0x11},
	{CMDDELAY_MS,   150},
	{MICMD_CMD,     0x29},
	{MICMD_CMD,     0x2C},
	{CMDDELAY_MS,   10}
};

const T_PANEL_CMD *t_cmd_standby_mi2 = NULL;

const T_LCD_PARAM t_mode_mi2[] = {
	/***********       MI Serial Format 1      *************/
	{
		// tPANEL_PARAM
		{
			/* Old prototype */
			PINMUX_LCDMODE_MI_FMT0,   //!< LCDMode
#if (_FPGA_EMULATION_)
			0.6f,                          //!< fDCLK
#else
			1.164f,                        //!< fDCLK
#endif
			(0x60 + 96),                   //!< uiHSyncTotalPeriod
			96,                            //!< uiHSyncActivePeriod
			0x2D,                          //!< uiHSyncBackPorch
			0x24 + 68,                     //!< uiVSyncTotalPeriod
			68,                            //!< uiVSyncActivePeriod
			0x13,                          //!< uiVSyncBackPorchOdd
			0x13,                          //!< uiVSyncBackPorchEven
			96,                            //!< uiBufferWidth
			68,                            //!< uiBufferHeight
			96,                            //!< uiWindowWidth
			68,                            //!< uiWindowHeight
			FALSE,                         //!< bYCbCrFormat

			/* New added parameters */
			0x01,                           //!< uiHSyncSYNCwidth
			0x01                            //!< uiVSyncSYNCwidth
		},

		// tIDE_PARAM
		{
			/* Old prototype */
			PINMUX_LCD_SEL_PARALLE_MI_8BITS|PINMUX_LCD_SEL_TE_ENABLE,  //!< pinmux_select_lcd;
			ICST_CCIR601,                   //!< icst;
			{FALSE, FALSE},                   //!< dithering[2];
			DISPLAY_DEVICE_MI,              //!< **DONT-CARE**
			IDE_PDIR_RGB,                   //!< pdir;
			IDE_LCD_R,                      //!< odd;
			IDE_LCD_G,                      //!< even;
			TRUE,                           //!< hsinv;
			TRUE,                           //!< vsinv;
			FALSE,                          //!< hvldinv;
			FALSE,                          //!< vvldinv;
			TRUE,                           //!< clkinv;
			FALSE,                          //!< fieldinv;
			FALSE,                          //!< **DONT-CARE**
			FALSE,                          //!< interlace;
			FALSE,                          //!< **DONT-CARE**
			0x40,                           //!< ctrst;
			0x00,                           //!< brt;
			0x40,                           //!< cmults;
			FALSE,                          //!< cex;
			FALSE,                          //!< **DONT-CARE**
			TRUE,                           //!< **DONT-CARE**
			TRUE,                           //!< tv_powerdown;
			{0x00, 0x00},                   //!< **DONT-CARE**

			/* New added parameters */
			FALSE,                          //!< YCex
			FALSE,                          //!< HLPF
			{FALSE, FALSE, FALSE},          //!< subpixOdd[3]
			{FALSE, FALSE, FALSE},          //!< subpixEven[3]
			{IDE_DITHER_6BITS, IDE_DITHER_6BITS, IDE_DITHER_6BITS}, //!< DitherBits[3]
			FALSE                           //!< clk1/2
		},

		(T_PANEL_CMD *)t_cmd_mode_mi2,                 //!< pCmdQueue
		sizeof(t_cmd_mode_mi2) / sizeof(T_PANEL_CMD),  //!< nCmd
	}

};

const T_LCD_ROT *t_rot_mi2 = NULL;

//@}

T_LCD_ROT *dispdev_get_lcd_rotate_cmd2(UINT32 *mode_number)
{
#if 0
		if (tRotMI != NULL) {
			*ModeNumber = sizeof(tRotMI) / sizeof(tLCD_ROT);
		} else
#endif
		{
			*mode_number = 0;
		}
		return (T_LCD_ROT *)t_rot_mi2;

}

T_LCD_PARAM *dispdev_get_config_mode2(UINT32 *mode_number)
{
	*mode_number = sizeof(t_mode_mi2) / sizeof(T_LCD_PARAM);
	return (T_LCD_PARAM *)t_mode_mi2;

}

T_PANEL_CMD *dispdev_get_standby_cmd2(UINT32 *cmd_number)
{
#if 0
	*cmd_number = sizeof(t_cmd_standby_mi) / sizeof(T_PANEL_CMD);
	return (T_PANEL_CMD *)t_cmd_standby_mi;
#else
	return 0;
#endif
}


void dispdev_set_mi_config2(MI_CONFIG *p_mi_config)
{

	BOOL	bTeSyncEn	= ENABLE;
	FLOAT	MiTargetClk = 6; // Max is 60MHz
	//UINT32  Div;
#if 0
	mi_setCtrl(SERIAL_3WSPI, MI_AUTOFMT3, p_mi_config->mi_fmt, FALSE, FALSE, bTeSyncEn, FALSE);
	mi_setSerialCsCtrl(TRUE);
	mi_setPolCtrl(FALSE, FALSE, FALSE, FALSE, SCLMOD0);
	mi_setSerialAutoRsBitValue(TRUE);
#else
	mi_setCtrl(PARALLEL_I80, MI_AUTOFMT3, p_mi_config->mi_fmt, FALSE, FALSE, bTeSyncEn, FALSE);
	mi_setPolCtrl(FALSE, FALSE, FALSE, FALSE, SCLMOD0);

#endif
#if 0
	Div = (UINT32)(pMiConfig->fMiSrcClk / MiTargetClk);
	pll_setClockRate(PLL_CLKSEL_MI_CLKDIV, PLL_MI_CLKDIV(Div - 1));
#else
	mi_setConfig(MI_CONFIG_ID_FREQ, MiTargetClk * 1000000);
#endif

}

void dispdev_set_mi_AutoCmd2(void)
{
	return;
}

void dispdev_write_lcd_mi2(UINT32 addr, UINT32 value)
{
	if (addr & MICMD_DATA) {
		mi_setCmd1(WrDATA, (value & 0xff));
	} else {
		mi_setCmd1(WrCMD, (value & 0xff));
	}

	mi_setCmdCnt(1);
	mi_waitCmdDone();
}


void dispdev_read_lcd_mi2( UINT32 addr, UINT32 *pValue)
{
	*pValue = 0;
	DBG_ERR("No MI Read reg implement!\r\n");
}


#if defined __FREERTOS
int panel_init2(void)
{
	PDISP_OBJ p_disp_obj;
	p_disp_obj = disp_get_display_object(DISP_2);
	
	p_disp_obj->dev_callback = &dispdev_get_lcd2_dev_obj;
    DBG_DUMP("Hello, panel2: NT75761\n");
    return 0;
}

void panel_exit2(void)
{
    DBG_DUMP("NT75761, Goodbye\r\n");
}

#elif defined __KERNEL__
static int __init panel_init2(void)
{
	PDISP_OBJ p_disp_obj;
	p_disp_obj = disp_get_display_object(DISP_2);
	
	p_disp_obj->dev_callback = &dispdev_get_lcd2_dev_obj;
    pr_info("Hello, panel2: NT75761\n");
    return 0;
}

static void __exit panel_exit2(void)
{
	PDISP_OBJ p_disp_obj;
	p_disp_obj = disp_get_display_object(DISP_2);
	
	p_disp_obj->dev_callback = NULL;
    printk(KERN_INFO "Goodbye\n");
}

module_init(panel_init2);
module_exit(panel_exit2);

MODULE_DESCRIPTION("NT75761 Panel");
MODULE_AUTHOR("Novatek Corp.");
MODULE_LICENSE("GPL");
#endif


