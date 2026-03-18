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


#define PANEL_WIDTH     480
#define PANEL_HEIGHT    800

#define HVALIDST    0x39  //uiHSyncBackPorch(HPB) -> 53
#define VVALIDST    0x15  //uiVSyncBackPorchOdd/Even
#define HSYNCT      0x02
#define VSYNCT      0x02


#define MICMD_CMD   0x01000000
#define MICMD_DATA  0x02000000

#define CMDDELAY_MS 0x80000000
#define CMDDELAY_US 0x40000000

/*
    panel Parameters for TK043F1168
*/
//@{
/*Used in MI*/
const T_PANEL_CMD t_cmd_mode_mi[] = {
	{MICMD_CMD, 0xB9}, //Set_EXTC
	{MICMD_DATA, 0xFF},
	{MICMD_DATA, 0x83},
	{MICMD_DATA, 0x69},
	
	{MICMD_CMD, 0xB1},	//Set Power 
	{MICMD_DATA, 0x85},
	{MICMD_DATA, 0x00},
	{MICMD_DATA, 0x34},
	{MICMD_DATA, 0x0A},
	{MICMD_DATA, 0x00},
	{MICMD_DATA, 0x0F},
	{MICMD_DATA, 0x0F},
	{MICMD_DATA, 0x2A},
	{MICMD_DATA, 0x32},
	{MICMD_DATA, 0x3F},
	{MICMD_DATA, 0x3F},
	{MICMD_DATA, 0x01}, //update VBIAS
	{MICMD_DATA, 0x23},
	{MICMD_DATA, 0x01},
	{MICMD_DATA, 0xE6},
	{MICMD_DATA, 0xE6},
	{MICMD_DATA, 0xE6},
	{MICMD_DATA, 0xE6},
	{MICMD_DATA, 0xE6},
	
	{MICMD_CMD, 0xB2}, // SET Display 480x800
	{MICMD_DATA, 0x00},
	{MICMD_DATA, 0x20},
	{MICMD_DATA, 0x0A},
	{MICMD_DATA, 0x0A},
	{MICMD_DATA, 0x70},
	{MICMD_DATA, 0x00},
	{MICMD_DATA, 0xFF},
	{MICMD_DATA, 0x00},
	{MICMD_DATA, 0x00},
	{MICMD_DATA, 0x00},
	{MICMD_DATA, 0x00},
	{MICMD_DATA, 0x03},
	{MICMD_DATA, 0x03},
	{MICMD_DATA, 0x00},
	{MICMD_DATA, 0x01},
	
	{MICMD_CMD, 0xB4}, // SET Display 480x800
	{MICMD_DATA, 0x00},
	{MICMD_DATA, 0x18},
	{MICMD_DATA, 0x80},
	{MICMD_DATA, 0x10},
	{MICMD_DATA, 0x01},
	{MICMD_CMD, 0xB6}, // SET VCOM
	{MICMD_DATA, 0x2C},
	{MICMD_DATA, 0x2C},
	
	{MICMD_CMD, 0xD5}, //SET GIP
	{MICMD_DATA, 0x00},
	{MICMD_DATA, 0x05},
	{MICMD_DATA, 0x03},
	{MICMD_DATA, 0x00},
	{MICMD_DATA, 0x01},
	{MICMD_DATA, 0x09},
	{MICMD_DATA, 0x10},
	{MICMD_DATA, 0x80},
	{MICMD_DATA, 0x37},
	{MICMD_DATA, 0x37},
	{MICMD_DATA, 0x20},
	{MICMD_DATA, 0x31},
	{MICMD_DATA, 0x46},
	{MICMD_DATA, 0x8A},
	{MICMD_DATA, 0x57},
	{MICMD_DATA, 0x9B},
	{MICMD_DATA, 0x20},
	{MICMD_DATA, 0x31},
	{MICMD_DATA, 0x46},
	{MICMD_DATA, 0x8A},
	{MICMD_DATA, 0x57},
	{MICMD_DATA, 0x9B},
	{MICMD_DATA, 0x07},
	{MICMD_DATA, 0x0F},
	{MICMD_DATA, 0x02},
	{MICMD_DATA, 0x00},
	{MICMD_CMD, 0xE0}, //SET GAMMA
	{MICMD_DATA, 0x00},
	{MICMD_DATA, 0x08},
	{MICMD_DATA, 0x0D},
	{MICMD_DATA, 0x2D},
	{MICMD_DATA, 0x34},
	{MICMD_DATA, 0x3F},
	{MICMD_DATA, 0x19},
	{MICMD_DATA, 0x38},
	{MICMD_DATA, 0x09},
	{MICMD_DATA, 0x0E},
	{MICMD_DATA, 0x0E},
	{MICMD_DATA, 0x12},
	{MICMD_DATA, 0x14},
	{MICMD_DATA, 0x12},
	{MICMD_DATA, 0x14},
	{MICMD_DATA, 0x13},
	{MICMD_DATA, 0x19},
	{MICMD_DATA, 0x00},
	{MICMD_DATA, 0x08},
	
	{MICMD_DATA, 0x0D},
	{MICMD_DATA, 0x2D},
	{MICMD_DATA, 0x34},
	{MICMD_DATA, 0x3F},
	{MICMD_DATA, 0x19},
	{MICMD_DATA, 0x38},
	{MICMD_DATA, 0x09},
	{MICMD_DATA, 0x0E},
	{MICMD_DATA, 0x0E},
	{MICMD_DATA, 0x12},
	{MICMD_DATA, 0x14},
	{MICMD_DATA, 0x12},
	{MICMD_DATA, 0x14},
	{MICMD_DATA, 0x13},
	{MICMD_DATA, 0x19},
	{MICMD_CMD, 0xC1}, //set DGC
	{MICMD_DATA, 0x01}, //enable DGC function
	{MICMD_DATA, 0x02}, //SET R-GAMMA
	{MICMD_DATA, 0x08},
	{MICMD_DATA, 0x12},
	{MICMD_DATA, 0x1A},
	{MICMD_DATA, 0x22},
	{MICMD_DATA, 0x2A},
	{MICMD_DATA, 0x31},
	{MICMD_DATA, 0x36},
	{MICMD_DATA, 0x3F},
	{MICMD_DATA, 0x48},
	{MICMD_DATA, 0x51},
	{MICMD_DATA, 0x58},
	{MICMD_DATA, 0x60},
	{MICMD_DATA, 0x68},
	{MICMD_DATA, 0x70},
	{MICMD_DATA, 0x78},
	{MICMD_DATA, 0x80},
	{MICMD_DATA, 0x88},
	{MICMD_DATA, 0x90},
	{MICMD_DATA, 0x98},
	{MICMD_DATA, 0xA0},
	{MICMD_DATA, 0xA7},
	{MICMD_DATA, 0xAF},
	{MICMD_DATA, 0xB6},
	{MICMD_DATA, 0xBE},
	{MICMD_DATA, 0xC7},
	{MICMD_DATA, 0xCE},
	{MICMD_DATA, 0xD6},
	{MICMD_DATA, 0xDE},
	{MICMD_DATA, 0xE6},
	{MICMD_DATA, 0xEF},
	{MICMD_DATA, 0xF5},
	{MICMD_DATA, 0xFB},
	{MICMD_DATA, 0xFC},
	{MICMD_DATA, 0xFE},
	{MICMD_DATA, 0x8C},
	{MICMD_DATA, 0xA4},
	{MICMD_DATA, 0x19},
	{MICMD_DATA, 0xEC},
	{MICMD_DATA, 0x1B},
	{MICMD_DATA, 0x4C},
	
	{MICMD_DATA, 0x40},
	{MICMD_DATA, 0x02}, //SET G-Gamma
	{MICMD_DATA, 0x08},
	{MICMD_DATA, 0x12},
	{MICMD_DATA, 0x1A},
	{MICMD_DATA, 0x22},
	{MICMD_DATA, 0x2A},
	{MICMD_DATA, 0x31},
	{MICMD_DATA, 0x36},
	{MICMD_DATA, 0x3F},
	{MICMD_DATA, 0x48},
	{MICMD_DATA, 0x51},
	{MICMD_DATA, 0x58},
	{MICMD_DATA, 0x60},
	{MICMD_DATA, 0x68},
	{MICMD_DATA, 0x70},
	{MICMD_DATA, 0x78},
	{MICMD_DATA, 0x80},
	{MICMD_DATA, 0x88},
	{MICMD_DATA, 0x90},
	{MICMD_DATA, 0x98},
	{MICMD_DATA, 0xA0},
	{MICMD_DATA, 0xA7},
	{MICMD_DATA, 0xAF},
	{MICMD_DATA, 0xB6},
	{MICMD_DATA, 0xBE},
	{MICMD_DATA, 0xC7},
	{MICMD_DATA, 0xCE},
	{MICMD_DATA, 0xD6},
	{MICMD_DATA, 0xDE},
	{MICMD_DATA, 0xE6},
	{MICMD_DATA, 0xEF},
	{MICMD_DATA, 0xF5},
	{MICMD_DATA, 0xFB},
	{MICMD_DATA, 0xFC},
	{MICMD_DATA, 0xFE},
	{MICMD_DATA, 0x8C},
	{MICMD_DATA, 0xA4},
	{MICMD_DATA, 0x19},
	{MICMD_DATA, 0xEC},
	{MICMD_DATA, 0x1B},
	{MICMD_DATA, 0x4C},
	{MICMD_DATA, 0x40},
	{MICMD_DATA, 0x02}, //SET B-Gamma
	{MICMD_DATA, 0x08},
	{MICMD_DATA, 0x12},
	{MICMD_DATA, 0x1A},
	{MICMD_DATA, 0x22},
	{MICMD_DATA, 0x2A},
	{MICMD_DATA, 0x31},
	{MICMD_DATA, 0x36},
	{MICMD_DATA, 0x3F},
	{MICMD_DATA, 0x48},
	{MICMD_DATA, 0x51},
	{MICMD_DATA, 0x58},
	{MICMD_DATA, 0x60},
	{MICMD_DATA, 0x68},
	{MICMD_DATA, 0x70},
	{MICMD_DATA, 0x78},
	
	{MICMD_DATA, 0x80},
	{MICMD_DATA, 0x88},
	{MICMD_DATA, 0x90},
	{MICMD_DATA, 0x98},
	{MICMD_DATA, 0xA0},
	{MICMD_DATA, 0xA7},
	{MICMD_DATA, 0xAF},
	{MICMD_DATA, 0xB6},
	{MICMD_DATA, 0xBE},
	{MICMD_DATA, 0xC7},
	{MICMD_DATA, 0xCE},
	{MICMD_DATA, 0xD6},
	{MICMD_DATA, 0xDE},
	{MICMD_DATA, 0xE6},
	{MICMD_DATA, 0xEF},
	{MICMD_DATA, 0xF5},
	{MICMD_DATA, 0xFB},
	{MICMD_DATA, 0xFC},
	{MICMD_DATA, 0xFE},
	{MICMD_DATA, 0x8C},
	{MICMD_DATA, 0xA4},
	{MICMD_DATA, 0x19},
	{MICMD_DATA, 0xEC},
	{MICMD_DATA, 0x1B},
	{MICMD_DATA, 0x4C},
	{MICMD_DATA, 0x40},
	{MICMD_CMD, 0x3A},
	{MICMD_DATA, 0x55},//55:565, 66:666, 77:888
	{MICMD_CMD, 0x36},
	{MICMD_DATA, 0x00},
	{MICMD_CMD, 0x13}, //set normal mode
	{MICMD_CMD, 0x11}, //Sleep Out
	{CMDDELAY_MS, 120},
	{MICMD_CMD, 0x29}, //Display On	
	{CMDDELAY_MS, 100},
	{MICMD_CMD, 0x2A},
	{MICMD_DATA, 0x00},
	{MICMD_DATA, 0x00},
	{MICMD_DATA, 0x01},
	{MICMD_DATA, 0xDF},
	{MICMD_CMD, 0x2B},
	{MICMD_DATA, 0x00},
	{MICMD_DATA, 0x00},
	{MICMD_DATA, 0x03},
	{MICMD_DATA, 0x1F},
	{MICMD_CMD, 0x2C},
	{CMDDELAY_MS,  10},
};

const T_PANEL_CMD *t_cmd_standby_mi = NULL;

const T_LCD_PARAM t_mode_mi[] = {
	/***********       MI Serial Format 1      *************/
	{
		// tPANEL_PARAM
		{
			/* Old prototype */
			PINMUX_LCDMODE_MI_FMT8,   //!< LCDMode
#ifdef _NVT_FPGA_
			3000000,                        //!< fDCLK
#else
			32000000,                       //!< fDCLK, if mi = 64MHz, fmt8 ide max clock = 64/2
#endif
			(0xC0+PANEL_WIDTH),          //!< uiHSyncTotalPeriod
			PANEL_WIDTH,                            //!< uiHSyncActivePeriod
			0x60,                          //!< uiHSyncBackPorch
			0x52 + PANEL_HEIGHT,                     //!< uiVSyncTotalPeriod
			PANEL_HEIGHT,                            //!< uiVSyncActivePeriod
			VVALIDST,                          //!< uiVSyncBackPorchOdd
			VVALIDST,                          //!< uiVSyncBackPorchEven
			PANEL_WIDTH,                            //!< uiBufferWidth
			PANEL_HEIGHT,                            //!< uiBufferHeight
			PANEL_WIDTH,                            //!< uiWindowWidth
			PANEL_HEIGHT,                            //!< uiWindowHeight
			FALSE,                         //!< bYCbCrFormat

			/* New added parameters */
			HSYNCT,                           //!< uiHSyncSYNCwidth
			HSYNCT                            //!< uiVSyncSYNCwidth
		},

		// tIDE_PARAM
		{
			/* Old prototype */
			PINMUX_LCD_SEL_PARALLE_MI_16BITS,  //!< pinmux_select_lcd;
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

		(T_PANEL_CMD *)t_cmd_mode_mi,                 //!< pCmdQueue
		sizeof(t_cmd_mode_mi) / sizeof(T_PANEL_CMD),  //!< nCmd
	}

};

const T_LCD_ROT *t_rot_mi = NULL;

//@}

T_LCD_ROT *dispdev_get_lcd_rotate_cmd(UINT32 *mode_number)
{
#if 0
		if (tRotMI != NULL) {
			*ModeNumber = sizeof(tRotMI) / sizeof(tLCD_ROT);
		} else
#endif
		{
			*mode_number = 0;
		}
		return (T_LCD_ROT *)t_rot_mi;

}

T_LCD_PARAM *dispdev_get_config_mode(UINT32 *mode_number)
{
	*mode_number = sizeof(t_mode_mi) / sizeof(T_LCD_PARAM);
	return (T_LCD_PARAM *)t_mode_mi;

}

T_PANEL_CMD *dispdev_get_standby_cmd(UINT32 *cmd_number)
{
#if 0
	*cmd_number = sizeof(t_cmd_standby_mi) / sizeof(T_PANEL_CMD);
	return (T_PANEL_CMD *)t_cmd_standby_mi;
#else
	return 0;
#endif
}


void dispdev_set_mi_config(MI_CONFIG *p_mi_config)
{

	BOOL	bTeSyncEn	= FALSE;
#if defined(_NVT_FPGA_)
	FLOAT	MiTargetClk = 6; // Max is 60MHz
#else
	FLOAT	MiTargetClk = 64; // source = 192MHz
#endif
	//UINT32  Div;

	mi_setCtrl(PARALLEL_I80, MI_AUTOFMT3, p_mi_config->mi_fmt, FALSE, FALSE, bTeSyncEn, FALSE);
	//mi_setSerialCsCtrl(TRUE);
	mi_setPolCtrl(FALSE, FALSE, FALSE, FALSE, SCLMOD0);
	//mi_setSerialAutoRsBitValue(TRUE);
	//mi_setSyncTiming(0,0,0,1); //for test bSyncSel=1
#if 0
	Div = (UINT32)(pMiConfig->fMiSrcClk / MiTargetClk);
	pll_setClockRate(PLL_CLKSEL_MI_CLKDIV, PLL_MI_CLKDIV(Div - 1));
#else
	mi_setConfig(MI_CONFIG_ID_FREQ, MiTargetClk * 1000000);
#endif

}

void dispdev_set_mi_AutoCmd(void)
{
	return;
}

void dispdev_write_lcd_mi(UINT32 addr, UINT32 value)
{
	if (addr & MICMD_DATA) {
		mi_setCmd1(WrDATA, (value & 0xff));
	} else {
		mi_setCmd1(WrCMD, (value & 0xff));
	}

	mi_setCmdCnt(1);
	mi_waitCmdDone();
}

void dispdev_set_lcd_colour(void)
{
	UINT32 i=0;
	
	dispdev_write_lcd_mi(MICMD_CMD, 0x2D);
	for (i=0; i<=63; i++)
	{
		dispdev_write_lcd_mi(MICMD_DATA, i*8);   
	}

	for (i=0; i<=63; i++)
	{
		dispdev_write_lcd_mi(MICMD_DATA, i*4);   
	}

	for (i=0; i<=63; i++)
	{
		dispdev_write_lcd_mi(MICMD_DATA, i*8);   
	}	
}

void dispdev_read_lcd_mi( UINT32 addr, UINT32 *pValue)
{
	UINT8 temp[4]={0};

	mi_setCmd1(WrCMD, (addr & 0xff));
	mi_setCmd2(RdDATA, 0x0);
	mi_setCmd3(RdDATA, 0x0);
	mi_setCmd4(RdDATA, 0x0);
	mi_setCmd5(RdDATA, 0x0);

	mi_setCmdCnt(5);
	mi_waitCmdDone();

	
	temp[0]= mi_getReadBack(2)& 0xff;
	temp[1]= mi_getReadBack(3)& 0xff;
	temp[2]= mi_getReadBack(4)& 0xff;
	temp[3]= mi_getReadBack(5)& 0xff;
	*pValue = (temp[0]<<24)|(temp[1]<<16)|(temp[2]<<8)|(temp[3]<<0);
	
}


#if defined __FREERTOS
int panel_init(void)
{
	PDISP_OBJ p_disp_obj;
	p_disp_obj = disp_get_display_object(DISP_1);
	
	p_disp_obj->dev_callback = &dispdev_get_lcd1_dev_obj;
    DBG_DUMP("Hello, panel: TK043F1168\n");
    return 0;
}

void panel_exit(void)
{
    DBG_DUMP("TK043F1168, Goodbye\r\n");
}

#elif defined __KERNEL__
static int __init panel_init(void)
{
	PDISP_OBJ p_disp_obj;
	p_disp_obj = disp_get_display_object(DISP_1);
	
	p_disp_obj->dev_callback = &dispdev_get_lcd1_dev_obj;
    pr_info("Hello, panel: TK043F1168\n");
    return 0;
}

static void __exit panel_exit(void)
{
	PDISP_OBJ p_disp_obj;
	p_disp_obj = disp_get_display_object(DISP_1);
	
	p_disp_obj->dev_callback = NULL;
    printk(KERN_INFO "Goodbye\n");
}

module_init(panel_init);
module_exit(panel_exit);

MODULE_DESCRIPTION("TK043F1168 Panel");
MODULE_AUTHOR("Novatek Corp.");
MODULE_LICENSE("GPL");
#endif


