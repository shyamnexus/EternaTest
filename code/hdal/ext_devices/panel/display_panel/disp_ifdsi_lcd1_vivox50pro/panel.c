/*
    Display object for driving DSI device

    @file       NT355510.c
    @ingroup
    @note       This panel MUST select ide clock to PLL1 ( 480 ). Once change to \n
				another frequence, the _IDE_FDCLK should be re calculated

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/
#include "dispdev_ifdsi.h"

#define PANEL_WIDTH     1080
#define PANEL_HEIGHT    2376

#define VIVOX50_IND_MSG(...)       debug_msg(__VA_ARGS__)

#define VIVOX50_ERR_MSG(...)       debug_msg(__VA_ARGS__)

#define VIVOX50_WRN_MSG(...)       debug_msg(__VA_ARGS__)
#define VIVOX50_TE_OFF              0
#define VIVOX50_TE_ON               1

#define VIVOX50_TE_PACKET           0
#define VIVOX50_TE_PIN              1

/*
    RGB888 = 1 pixel = 3bytes packet
    If DSI src = 240MHz, internal clock = 30MHz, data rate = 30MHz x 1bytes = 30MB / sec per lane
    2 lane = 60MB = 20Mpixel ' ide need > 20MHz
*/


#define DSI_FORMAT_RGB565          0    //ide use 480 & DSI use 480
#define DSI_FORMAT_RGB666P         1    //ide use 480 & DSI use 480
#define DSI_FORMAT_RGB666L         2    //ide use 480 & DSI use 480
#define DSI_FORMAT_RGB888          3    //ide use 480 & DSI use 480

#define DSI_OP_MODE_CMD_MODE       1
#define DSI_OP_MODE_VDO_MODE       0
#define DSI_PACKET_FORMAT          DSI_FORMAT_RGB888

#define DSI_TARGET_CLK             1200
#define _IDE_FDCLK					294000000
#define DSI_OP_MODE                DSI_OP_MODE_CMD_MODE//support command mode only
#define TE_ENABLE				0
#define TE_TYPE					VIVOX50_TE_PIN	// 1 : TE pin, 0: TE packet
/*
    panel Parameters for TCON VIVOX50
*/
//@{
/*Used in DSI*/
const T_PANEL_CMD t_cmd_mode_dsi[] = {
	//------------------------- Example ----------------------

	//setLp()
	//Exit sleep mode
	//[Header                          ]:[W]:[0xD01202C0]:[31]:[ 0]:[0x01001105]
	{DSICMD_CMD,     0x11},
	{DSICMD_DATA,    0x00},

	//VLIN limit current
	{DSICMD_CMD,     0xF0},
	{DSICMD_DATA,    0x5A},
	{DSICMD_DATA,    0x5A},
	{DSICMD_CMD,     0xB0},
	{DSICMD_DATA,    0x04},
	{DSICMD_CMD,     0xD5},
	{DSICMD_DATA,    0x24},
	{DSICMD_DATA,    0x9E},
	{DSICMD_DATA,    0x9E},
	{DSICMD_DATA,    0x00},
	{DSICMD_DATA,    0x20},
	{DSICMD_CMD,     0xF0},
	{DSICMD_DATA,    0xA5},
	{DSICMD_DATA,    0xA5},

	//OSC SEL
	{DSICMD_CMD,     0xF0},
	{DSICMD_DATA,    0x5A},
	{DSICMD_DATA,    0x5A},
	{DSICMD_CMD,     0xFC},
	{DSICMD_DATA,    0x5A},
	{DSICMD_DATA,    0x5A},
	{DSICMD_CMD,     0xB0},
	{DSICMD_DATA,    0x16},
	{DSICMD_CMD,     0xD1},
	{DSICMD_DATA,    0x21},

	{DSICMD_CMD,     0xD6},
	{DSICMD_DATA,    0x11},
	{DSICMD_CMD,     0xFC},
	{DSICMD_DATA,    0xA5},
	{DSICMD_DATA,    0xA5},
	{DSICMD_CMD,     0xF0},
	{DSICMD_DATA,    0xA5},
	{DSICMD_DATA,    0xA5},

	//DOT CLK NoNo
	{DSICMD_CMD,     0xF0},
	{DSICMD_DATA,    0x5A},
	{DSICMD_DATA,    0x5A},
	{DSICMD_CMD,     0xB0},
	{DSICMD_DATA,    0x12},
	{DSICMD_CMD,     0xD7},
	{DSICMD_DATA,    0x40},
	{DSICMD_CMD,     0xF0},
	{DSICMD_DATA,    0xA5},
	{DSICMD_DATA,    0xA5},
#if TE_ENABLE
	//TE on
	{DSICMD_CMD,     0x35},
	{DSICMD_DATA,    0x00},
	{CMDTEON,        VIVOX50_TE_ON},
	{CMDTEEXTPIN,    TE_TYPE},
#endif
	//ADD1
	{DSICMD_CMD,     0x2A},
	{DSICMD_DATA,    0x00},
	{DSICMD_DATA,    0x00},
	{DSICMD_DATA,    0x04},
	{DSICMD_DATA,    0x37},
	{DSICMD_CMD,     0x2B},
	{DSICMD_DATA,    0x00},
	{DSICMD_DATA,    0x00},
	{DSICMD_DATA,    0x09},
	{DSICMD_DATA,    0x47},
	{DSICMD_CMD,     0x60},
	{DSICMD_DATA,    0x00},


	//TSP Vs enable
	{DSICMD_CMD,     0xF0},
	{DSICMD_DATA,    0x5A},
	{DSICMD_DATA,    0x5A},
	{DSICMD_CMD,     0xDF},
	{DSICMD_DATA,    0x83},
	{DSICMD_DATA,    0x00},
	{DSICMD_DATA,    0x10},
	{DSICMD_CMD,     0xB0},
	{DSICMD_DATA,    0x01},
	{DSICMD_CMD,     0xE6},
	{DSICMD_DATA,    0x01},
	{DSICMD_CMD,     0xF0},
	{DSICMD_DATA,    0xA5},
	{DSICMD_DATA,    0xA5},

	//Brightness dimming: 1 frame no dimming
	{DSICMD_CMD,     0xF0},
	{DSICMD_DATA,    0x5A},
	{DSICMD_DATA,    0x5A},
	{DSICMD_CMD,     0xB0},
	{DSICMD_DATA,    0x06},
	{DSICMD_CMD,     0xB7},
	{DSICMD_DATA,    0x01},
	{DSICMD_CMD,     0xF0},
	{DSICMD_DATA,    0xA5},
	{DSICMD_DATA,    0xA5},
	{DSICMD_CMD,     0x53},
	{DSICMD_DATA,    0x20},

	//ADD2
	{DSICMD_CMD,     0x51},
	{DSICMD_DATA,    0x01},
	{DSICMD_DATA,    0x1A},


	//ACL dimming
	{DSICMD_CMD,     0xF0},
	{DSICMD_DATA,    0x5A},
	{DSICMD_DATA,    0x5A},
	{DSICMD_CMD,     0xB0},
	{DSICMD_DATA,    0xD9},
	{DSICMD_CMD,     0xBB},
	{DSICMD_DATA,    0x48},
	{DSICMD_CMD,     0xF0},
	{DSICMD_DATA,    0xA5},
	{DSICMD_DATA,    0xA5},
	{DSICMD_CMD,     0x55},
	{DSICMD_DATA,    0x00},


	{CMDDELAY_US,    1},
	{DSICMD_CMD,     0x29},
	{DSICMD_DATA,    0x00},

	//seed crc setting begin
	{DSICMD_CMD,     0xF0},
	{DSICMD_DATA,    0x5A},
	{DSICMD_DATA,    0x5A},
	{DSICMD_CMD,     0x81},
	{DSICMD_DATA,    0x90},
	{DSICMD_CMD,     0xB1},
	{DSICMD_DATA,    0x00},
	{DSICMD_DATA,    0xE8},
	{DSICMD_DATA,    0x05},
	{DSICMD_DATA,    0x00},
	{DSICMD_DATA,    0x05},
	{DSICMD_DATA,    0xE8},
	{DSICMD_DATA,    0x01},
	{DSICMD_DATA,    0x0F},
	{DSICMD_DATA,    0x01},
	{DSICMD_DATA,    0xEE},
	{DSICMD_DATA,    0x19},
	{DSICMD_DATA,    0xFF},
	{DSICMD_DATA,    0xE1},
	{DSICMD_DATA,    0xFF},
	{DSICMD_DATA,    0x0A},
	{DSICMD_DATA,    0xDF},
	{DSICMD_DATA,    0xF6},
	{DSICMD_DATA,    0xF7},
	{DSICMD_DATA,    0x02},
	{DSICMD_DATA,    0xFF},
	{DSICMD_DATA,    0xFF},
	{DSICMD_DATA,    0xFF},
	{DSICMD_CMD,     0xF0},
	{DSICMD_DATA,    0xA5},
	{DSICMD_DATA,    0xA5},

	//CRI TX payload 1  BRIGHTNESS
	{DSICMD_CMD,     0x51},
	{DSICMD_DATA,    0x01},
	{DSICMD_DATA,    0x1A},
};


const T_PANEL_CMD t_cmd_standby_dsi[] = {
	{DSICMD_CMD,     0x28},         // Display OFF
	{CMDDELAY_MS,    10},
//    {DSICMD_CMD,     0x10},      // Sleep in
//    {CMDDELAY_MS,    10},
};

const T_LCD_PARAM t_mode_dsi[] = {
	/***********       MI Serial Format 1      *************/
	{
		// T_PANEL_PARAM
		{
			/* Old prototype */
			PINMUX_DSI_4_LANE_CMD_MODE_RGB888,
			_IDE_FDCLK,                             //!< fd_clk
			(0x400 + PANEL_WIDTH),                   //!< ui_hsync_total_period
			PANEL_WIDTH,                            //!< ui_hsync_active_period
			0x200,                                   //!< ui_hsync_back_porch
			0x20 + PANEL_HEIGHT,                    //!< ui_vsync_total_period
			PANEL_HEIGHT,                           //!< ui_vsync_active_period
			0x14,                                   //!< ui_vsync_back_porch_odd
			0x14,                                   //!< ui_vsync_back_porch_even
			PANEL_WIDTH,                            //!< ui_buffer_width
			PANEL_HEIGHT,                           //!< ui_buffer_height
			PANEL_WIDTH,                            //!< ui_window_width
			PANEL_HEIGHT,                           //!< ui_window_height
			FALSE,                                  //!< b_ycbcr_format

			/* New added parameters */
			0x01,                                   //!< ui_hsync_sync_width
			0x01                                    //!< ui_vsync_sync_width
		},

		// T_IDE_PARAM
		{
			/* Old prototype */
			PINMUX_LCD_SEL_MIPI,            //!< pinmux_select_lcd;
			ICST_CCIR601,                   //!< icst;
			{FALSE, FALSE},                  //!< dithering[2];
			DISPLAY_DEVICE_MIPIDSI,         //!< **DONT-CARE**
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
			FALSE,                          //!< yc_ex
			FALSE,                          //!< hlpf
			{FALSE, FALSE, FALSE},          //!< subpix_odd[3]
			{FALSE, FALSE, FALSE},          //!< subpix_even[3]
			{IDE_DITHER_5BITS, IDE_DITHER_6BITS, IDE_DITHER_5BITS}, //!< dither_bits[3]
			FALSE                           //!< clk1/2
		},

		(T_PANEL_CMD *)t_cmd_mode_dsi,                 //!< p_cmd_queue
		sizeof(t_cmd_mode_dsi) / sizeof(T_PANEL_CMD),  //!< n_cmd
	}
};

const T_LCD_ROT *t_rot_dsi = NULL;

//@}

T_LCD_ROT *dispdev_get_lcd_rotate_dsi_cmd(UINT32 *mode_number)
{
#if 0
	if (t_rot_dsi != NULL) {
		*mode_number = sizeof(t_rot_dsi) / sizeof(T_LCD_ROT);
	} else
#endif
	{
		*mode_number = 0;
	}
	return (T_LCD_ROT *)t_rot_dsi;
}

T_LCD_PARAM *dispdev_get_config_mode_dsi(UINT32 *mode_number)
{
	*mode_number = sizeof(t_mode_dsi) / sizeof(T_LCD_PARAM);
	return (T_LCD_PARAM *)t_mode_dsi;
}

T_PANEL_CMD *dispdev_get_standby_cmd_dsi(UINT32 *cmd_number)
{
	*cmd_number = sizeof(t_cmd_standby_dsi) / sizeof(T_PANEL_CMD);
	return (T_PANEL_CMD *)t_cmd_standby_dsi;
}


void dispdev_set_dsi_config(DSI_CONFIG *p_dsi_config)
{
	if(dsi_set_config(DSI_CONFIG_ID_FREQ, DSI_TARGET_CLK * 1000000)!= E_OK){
		DBG_DUMP("dsi_set_config, line %d fail\r\n", __LINE__);
	}
	if(dsi_set_config(DSI_CONFIG_ID_TLPX, 3)!= E_OK){
		DBG_DUMP("dsi_set_config, line %d fail\r\n", __LINE__);
	}
	if(dsi_set_config(DSI_CONFIG_ID_BTA_TA_GO, 0xF)!= E_OK){ //21
		DBG_DUMP("dsi_set_config, line %d fail\r\n", __LINE__);
	}
	if(dsi_set_config(DSI_CONFIG_ID_BTA_TA_SURE, 7)!= E_OK){ //0
		DBG_DUMP("dsi_set_config, line %d fail\r\n", __LINE__);
	}
	if(dsi_set_config(DSI_CONFIG_ID_BTA_TA_GET, 0x13)!= E_OK){ //20
		DBG_DUMP("dsi_set_config, line %d fail\r\n", __LINE__);
	}
	if(dsi_set_config(DSI_CONFIG_ID_THS_PREPARE, 3)!= E_OK){ // 4
		DBG_DUMP("dsi_set_config, line %d fail\r\n", __LINE__);
	}
	if(dsi_set_config(DSI_CONFIG_ID_THS_ZERO, 6)!= E_OK){
		DBG_DUMP("dsi_set_config, line %d fail\r\n", __LINE__);
	}
	if(dsi_set_config(DSI_CONFIG_ID_THS_TRAIL, 3)!= E_OK){ // 7  
		DBG_DUMP("dsi_set_config, line %d fail\r\n", __LINE__);
	}
	if(dsi_set_config(DSI_CONFIG_ID_THS_EXIT, 6)!= E_OK){
		DBG_DUMP("dsi_set_config, line %d fail\r\n", __LINE__);
	}
	if(dsi_set_config(DSI_CONFIG_ID_TCLK_PREPARE, 3)!= E_OK){
		DBG_DUMP("dsi_set_config, line %d fail\r\n", __LINE__);
	}
	if(dsi_set_config(DSI_CONFIG_ID_TCLK_ZERO, 0xf)!= E_OK){ // 16
		DBG_DUMP("dsi_set_config, line %d fail\r\n", __LINE__);
	}
	if(dsi_set_config(DSI_CONFIG_ID_TCLK_POST, 0xa)!= E_OK){  //  16
		DBG_DUMP("dsi_set_config, line %d fail\r\n", __LINE__);
	}
	if(dsi_set_config(DSI_CONFIG_ID_TCLK_PRE, 0)!= E_OK){   // 2
		DBG_DUMP("dsi_set_config, line %d fail\r\n", __LINE__);
	}
	if(dsi_set_config(DSI_CONFIG_ID_TCLK_TRAIL, 3)!= E_OK){ // 3
		DBG_DUMP("dsi_set_config, line %d fail\r\n", __LINE__);
	}
	if(dsi_set_config(DSI_CONFIG_ID_BTA_HANDSK_TMOUT_VAL, 0x40)!= E_OK){
		DBG_DUMP("dsi_set_config, line %d fail\r\n", __LINE__);
	}
	if(dsi_set_config(DSI_CONFIG_ID_DATALANE_NO, DSI_DATA_LANE_3)!= E_OK){
		DBG_DUMP("dsi_set_config, line %d fail\r\n", __LINE__);
	}
	if(dsi_set_config(DSI_CONFIG_ID_TE_BTA_INTERVAL, 0x1F)!= E_OK){
		DBG_DUMP("dsi_set_config, line %d fail\r\n", __LINE__);
	}
	if(dsi_set_config(DSI_CONFIG_ID_CLK_LP_CTRL, 0x1)!= E_OK){
		DBG_DUMP("dsi_set_config, line %d fail\r\n", __LINE__);
	}
	if(dsi_set_config(DSI_CONFIG_ID_SYNC_DLY_CNT, 0xF)!= E_OK){
		DBG_DUMP("dsi_set_config, line %d fail\r\n", __LINE__);
	}

}

#if defined __FREERTOS
int panel_init(void)
{
	unsigned char *fdt_addr = (unsigned char *)fdt_get_base();
	int nodeoffset;
	uint32_t *cell = NULL;
	DISPDEV_PARAM   disp_dev;
	PDISP_OBJ p_disp_obj;
	uint32_t value = 0;
	p_disp_obj = disp_get_display_object(DISP_1);
    if(fdt_addr == NULL)
		return -1;

	nodeoffset = fdt_path_offset((const void*)fdt_addr, "/logo");
	if (nodeoffset >= 0) {
		cell = (uint32_t*)fdt_getprop((const void*)fdt_addr, nodeoffset, "enable", NULL);
		if (cell > 0) {
			 value = be32_to_cpu(cell[0]);
			 disp_dev.SEL.SET_LOGOMODE.b_en_logo = value;
			p_disp_obj->dev_ctrl(DISPDEV_SET_LOGOMODE, &disp_dev);
			if(value == 0){
				dsi_reset();
			}
		} else {
			dsi_reset();
			DBG_WRN("panel dtsi logo mode enable is not set!\r\n");
		}
	} else {
		dsi_reset();
		DBG_WRN("can not find logo node!\r\n");
	}

	p_disp_obj->dev_callback = &dispdev_get_lcd1_dev_obj;
    DBG_DUMP("Hello, panel: VIVOX50PRO\n");
    return 0;
}

void panel_exit(void)
{
    DBG_DUMP("VIVOX50PRO, Goodbye\r\n");
}

#elif defined __KERNEL__
static int __init panel_init(void)
{
	DISPDEV_PARAM   disp_dev;
	struct device_node *panel;
	u32 value = 0;
	PDISP_OBJ p_disp_obj;
	p_disp_obj = disp_get_display_object(DISP_1);
	panel = of_find_node_by_name(NULL, "logo");
	if (panel) {
		if (!of_property_read_u32(panel, "enable", &value)) {
			disp_dev.SEL.SET_LOGOMODE.b_en_logo = value;
			p_disp_obj->dev_ctrl(DISPDEV_SET_LOGOMODE, &disp_dev);
			if(value == 0){
				dsi_reset();
			}
		} else {
			dsi_reset();
			DBG_WRN("panel dtsi logo mode enable is not set!\r\n");
		}
	} else {
		dsi_reset();
		DBG_WRN("can not find logo node!\r\n");
	}

	p_disp_obj->dev_callback = &dispdev_get_lcd1_dev_obj;
    pr_info("Hello, panel: VIVOX50PRO\n");
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

MODULE_DESCRIPTION("VIVOX50PRO Panel");
MODULE_AUTHOR("Novatek Corp.");
MODULE_LICENSE("GPL");
#endif
