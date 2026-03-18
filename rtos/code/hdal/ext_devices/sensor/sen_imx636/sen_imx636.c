#if defined(__KERNEL__)
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <kdrv_builtin/kdrv_builtin.h>
#include "isp_builtin.h"
#else
#include "plat/gpio.h"
#endif
#include "kwrap/error_no.h"
#include "kwrap/type.h"
#include "kwrap/util.h"
#include <kwrap/verinfo.h>
#include "kflow_videocapture/ctl_sen.h"
#include "isp_api.h"

#include "sen_cfg.h"
#include "sen_common.h"
#include "sen_inc.h"

//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================
#if defined(__KERNEL__)
char *sen_cfg_path = "null";
module_param_named(sen_cfg_path, sen_cfg_path, charp, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(sen_cfg_path, "Path of cfg file");

#ifdef DEBUG
unsigned int sen_debug_level = THIS_DBGLVL;
module_param_named(sen_debug_level, sen_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(sen_debug_level, "Debug message level");
#endif
#endif

//=============================================================================
// version
//=============================================================================
VOS_MODULE_VERSION(nvt_sen_imx636, 1, 01, 000, 00);

//=============================================================================
// information
//=============================================================================
#define SEN_IMX636_MODULE_NAME     "sen_imx636"
#define SEN_MAX_MODE               1
#define SEN_I2C_ADDR 0x34>>1
#define SEN_I2C_COMPATIBLE "nvt,sen_imx636"
#include "sen_i2c.c"

//=============================================================================
// function declaration
//=============================================================================
static CTL_SEN_DRV_TAB *sen_get_drv_tab_imx636(void);
static void sen_pwr_ctrl_imx636(CTL_SEN_ID id, CTL_SEN_PWR_CTRL_FLAG flag, CTL_SEN_CLK_CB clk_cb);
static ER sen_open_imx636(CTL_SEN_ID id);
static ER sen_close_imx636(CTL_SEN_ID id);
static ER sen_sleep_imx636(CTL_SEN_ID id);
static ER sen_wakeup_imx636(CTL_SEN_ID id);
static ER sen_write_reg_imx636(CTL_SEN_ID id, CTL_SEN_CMD *cmd);
static ER sen_read_reg_imx636(CTL_SEN_ID id, CTL_SEN_CMD *cmd);
static ER sen_chg_mode_imx636(CTL_SEN_ID id, CTL_SENDRV_CHGMODE_OBJ chgmode_obj);
static ER sen_chg_fps_imx636(CTL_SEN_ID id, UINT32 fps);
static ER sen_set_info_imx636(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data);
static ER sen_get_info_imx636(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data);
static void sen_get_mode_basic_imx636(CTL_SENDRV_GET_MODE_BASIC_PARAM *mode_basic);
static void sen_get_attr_basic_imx636(CTL_SEN_ID id, CTL_SENDRV_GET_ATTR_BASIC_PARAM *data);
static void sen_get_attr_signal_imx636(CTL_SENDRV_GET_ATTR_SIGNAL_PARAM *data);
static ER sen_get_attr_cmdif_imx636(CTL_SEN_ID id, CTL_SENDRV_GET_ATTR_CMDIF_PARAM *data);
static ER sen_get_attr_if_imx636(CTL_SENDRV_GET_ATTR_IF_PARAM *data);
static void sen_get_fps_imx636(CTL_SEN_ID id, CTL_SENDRV_GET_FPS_PARAM *data);
static void sen_get_speed_imx636(CTL_SEN_ID id, CTL_SENDRV_GET_SPEED_PARAM *data);
static void sen_get_mode_mipi_imx636(CTL_SENDRV_GET_MODE_MIPI_PARAM *data);
static void sen_get_modesel_imx636(CTL_SENDRV_GET_MODESEL_PARAM *data);
static void sen_get_rowtime_imx636(CTL_SEN_ID id, CTL_SENDRV_GET_MODE_ROWTIME_PARAM *data);
static void sen_set_cur_fps_imx636(CTL_SEN_ID id, UINT32 fps);
static UINT32 sen_get_cur_fps_imx636(CTL_SEN_ID id);
static void sen_set_chgmode_fps_imx636(CTL_SEN_ID id, UINT32 fps);
static UINT32 sen_get_chgmode_fps_imx636(CTL_SEN_ID id);
//=============================================================================
// global variable
//=============================================================================
static UINT32 sen_map = SEN_PATH_1;

static SEN_PRESET sen_preset[CTL_SEN_ID_MAX] = {
	{10000, 1000},
	{10000, 1000},
	{10000, 1000},
	{10000, 1000},
	{10000, 1000},
	{10000, 1000},
	{10000, 1000},
	{10000, 1000}	
};

static SEN_DIRECTION sen_direction[CTL_SEN_ID_MAX] = {
	{FALSE, FALSE},
	{FALSE, FALSE},
	{FALSE, FALSE},
	{FALSE, FALSE},
	{FALSE, FALSE},
	{FALSE, FALSE},
	{FALSE, FALSE},
	{FALSE, FALSE}
};

static SEN_POWER sen_power[CTL_SEN_ID_MAX] = {
	//C_GPIO:+0x0; P_GPIO:+0x20; S_GPIO:+0x40; L_GPIO:+0x60; D_GPIO:0x80; H_GPIO:+0xA0; A_GPIO:0xC0; D_GPIO:0xE0;
	{CTL_SEN_CLK_SEL_SIEMCLK, CTL_SEN_IGNORE, 0x4C, 1, 1},
	{CTL_SEN_CLK_SEL_SIEMCLK2, CTL_SEN_IGNORE, 0x4D, 1, 1},
	{CTL_SEN_CLK_SEL_SIEMCLK2, CTL_SEN_IGNORE, 0x4D, 1, 1},
	{CTL_SEN_CLK_SEL_SIEMCLK2, CTL_SEN_IGNORE, 0x4D, 1, 1},
	{CTL_SEN_CLK_SEL_SIEMCLK2, CTL_SEN_IGNORE, 0x4D, 1, 1},
	{CTL_SEN_CLK_SEL_SIEMCLK2, CTL_SEN_IGNORE, 0x4D, 1, 1},
	{CTL_SEN_CLK_SEL_SIEMCLK2, CTL_SEN_IGNORE, 0x4D, 1, 1},
	{CTL_SEN_CLK_SEL_SIEMCLK2, CTL_SEN_IGNORE, 0x4D, 1, 1}
};
	
static SEN_I2C sen_i2c[CTL_SEN_ID_MAX] = {
	{SEN_I2C_ID_1, SEN_I2C_ADDR},
	{SEN_I2C_ID_2, SEN_I2C_ADDR},
	{SEN_I2C_ID_2, SEN_I2C_ADDR},
	{SEN_I2C_ID_2, SEN_I2C_ADDR},
	{SEN_I2C_ID_2, SEN_I2C_ADDR},
	{SEN_I2C_ID_2, SEN_I2C_ADDR},
	{SEN_I2C_ID_2, SEN_I2C_ADDR},
	{SEN_I2C_ID_2, SEN_I2C_ADDR}
};

static CTL_SENDRV_GET_ATTR_BASIC_PARAM basic_param = {
	SEN_IMX636_MODULE_NAME,
	CTL_SEN_VENDOR_OTHERS,
	SEN_MAX_MODE,
	CTL_SEN_SUPPORT_PROPERTY_NONE,
	0
};

static CTL_SENDRV_GET_ATTR_SIGNAL_PARAM signal_param = {
	CTL_SEN_SIGNAL_MASTER,
	{CTL_SEN_ACTIVE_HIGH, CTL_SEN_ACTIVE_HIGH, CTL_SEN_PHASE_RISING, CTL_SEN_PHASE_RISING, CTL_SEN_PHASE_RISING}
};

static CTL_SENDRV_I2C i2c = {
	{
		{CTL_SEN_I2C_W_ADDR_DFT,     0x34},
		{CTL_SEN_I2C_W_ADDR_OPTION1, 0xFF},
		{CTL_SEN_I2C_W_ADDR_OPTION2, 0xFF},
		{CTL_SEN_I2C_W_ADDR_OPTION3, 0xFF},
		{CTL_SEN_I2C_W_ADDR_OPTION4, 0xFF},
		{CTL_SEN_I2C_W_ADDR_OPTION5, 0xFF}
	}
};

static CTL_SENDRV_GET_SPEED_PARAM speed_param[SEN_MAX_MODE] = {
	{
		CTL_SEN_MODE_1,
		CTL_SEN_SIEMCLK_SRC_DFT,
		20000000,
		187500000, // dont't care
		187500000
	}
};

static CTL_SENDRV_GET_MODE_MIPI_PARAM mipi_param[SEN_MAX_MODE] = {
	{
		CTL_SEN_MODE_1,
		CTL_SEN_CLKLANE_1,
		CTL_SEN_DATALANE_2,
		{ {CTL_SEN_MIPI_MANUAL_8BIT, 0x30}, {CTL_SEN_MIPI_MANUAL_8BIT, 0x2A}, {CTL_SEN_MIPI_MANUAL_NONE, 0} },
		0,
		{0, 0, 0, 0},
		SEN_BIT_OFS_NONE
	}
};

static CTL_SENDRV_GET_MODE_BASIC_PARAM mode_basic_param[SEN_MAX_MODE] = {
	{
		CTL_SEN_MODE_1,
		CTL_SEN_IF_TYPE_MIPI,
		CTL_SEN_DATA_FMT_EVS,
		CTL_SEN_MODE_LINEAR,
		25000,
		1,
		CTL_SEN_STPIX_R,
		CTL_SEN_PIXDEPTH_8BIT,
		CTL_SEN_FMT_POGRESSIVE,
		{1280, 720},
		{{0, 0, 1280, 720}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
		{1280, 720},
		{0, 1000, 0, 1000},//don't care
		CTL_SEN_RATIO(16, 9),
		{1000, 1000},//don't care
		100
	}
};

static CTL_SEN_CMD imx636_mode_1[] = {
	//IMX636 1280*720, RAW8, 2-lane (EVT 3.0 format)
	{0x0000001C, 1, {0x00000001, 0x0}},
	{SEN_CMD_DELAY, 1, {1000, 0x0}},
	{0x00400004, 1, {0x00000001, 0x0}},
	{SEN_CMD_DELAY, 1, {1000, 0x0}},
	{0x00400004, 1, {0x00000000, 0x0}},
	{SEN_CMD_DELAY, 1, {1000, 0x0}},
	{0x0000B000, 1, {0x00000158, 0x0}},
	{SEN_CMD_DELAY, 1, {1, 0x0}},
	{0x0000B044, 1, {0x00000000, 0x0}},
	{0x0000B004, 1, {0x0000000A, 0x0}},
	{0x0000B040, 1, {0x00000000, 0x0}},
	{0x0000B0C8, 1, {0x00000000, 0x0}},
	{0x0000B040, 1, {0x00000000, 0x0}},
	{0x0000B040, 1, {0x00000000, 0x0}},
	{0x00000000, 1, {0x4F006442, 0x0}},
	{0x00000000, 1, {0x0F006442, 0x0}},
	{0x000000B8, 1, {0x00000400, 0x0}},
	{0x000000B8, 1, {0x00000400, 0x0}},
	{0x0000B07C, 1, {0x00000000, 0x0}},
	{0x0000B074, 1, {0x00000002, 0x0}},
	{0x0000B078, 1, {0x000000A0, 0x0}},
	{0x000000C0, 1, {0x00000110, 0x0}},
	{0x000000C0, 1, {0x00000210, 0x0}},
	{0x0000B120, 1, {0x00000001, 0x0}},
	{0x0000E120, 1, {0x00000000, 0x0}},
	{0x0000B068, 1, {0x00000004, 0x0}},
	{0x0000B07C, 1, {0x00000001, 0x0}},
	{SEN_CMD_DELAY, 1, {1, 0x0}},
	{0x0000B07C, 1, {0x00000003, 0x0}},
	{SEN_CMD_DELAY, 1, {1, 0x0}},
	{0x000000B8, 1, {0x00000401, 0x0}},
	{0x000000B8, 1, {0x00000409, 0x0}},
	{0x00000000, 1, {0x4F006442, 0x0}},
	{0x00000000, 1, {0x4F00644A, 0x0}},
	{0x0000B080, 1, {0x00000077, 0x0}},
	{0x0000B084, 1, {0x0000000F, 0x0}},
	{0x0000B088, 1, {0x00000037, 0x0}},
	{0x0000B08C, 1, {0x00000037, 0x0}},
	{0x0000B090, 1, {0x000000DF, 0x0}},
	{0x0000B094, 1, {0x00000057, 0x0}},
	{0x0000B098, 1, {0x00000037, 0x0}},
	{0x0000B09C, 1, {0x00000067, 0x0}},
	{0x0000B0A0, 1, {0x00000037, 0x0}},
	{0x0000B0A4, 1, {0x0000002F, 0x0}},
	{0x0000B0AC, 1, {0x00000028, 0x0}},
	{0x0000B0CC, 1, {0x00000001, 0x0}},
	{0x0000B000, 1, {0x000002F8, 0x0}},
	{0x0000B004, 1, {0x0000008A, 0x0}},
	{0x0000B01C, 1, {0x00000030, 0x0}},
	{0x0000B020, 1, {0x00002000, 0x0}},
	{0x0000B02C, 1, {0x000000FF, 0x0}},
	{0x0000B030, 1, {0x00003E80, 0x0}},
	{0x0000B028, 1, {0x00000FA0, 0x0}},
	{0x0000A000, 1, {0x000B0501, 0x0}},
	{SEN_CMD_DELAY, 1, {1, 0x0}},
	{0x0000A008, 1, {0x00002405, 0x0}},
	{SEN_CMD_DELAY, 1, {1, 0x0}},
	{0x0000A004, 1, {0x000B0501, 0x0}},
	{SEN_CMD_DELAY, 1, {1, 0x0}},
	{0x0000A020, 1, {0x00000150, 0x0}},
	{SEN_CMD_DELAY, 1, {1, 0x0}},
	{0x0000B040, 1, {0x00000007, 0x0}},
	{0x0000B064, 1, {0x00000006, 0x0}},
	{0x0000B040, 1, {0x0000000F, 0x0}},
	{SEN_CMD_DELAY, 1, {1, 0x0}},
	{0x0000B004, 1, {0x0000008A, 0x0}},
	{SEN_CMD_DELAY, 1, {1, 0x0}},
	{0x0000B0C8, 1, {0x00000003, 0x0}},
	{SEN_CMD_DELAY, 1, {1, 0x0}},
	{0x0000B044, 1, {0x00000001, 0x0}},
	{0x0000B000, 1, {0x000002F9, 0x0}},
	{0x00007008, 1, {0x00000001, 0x0}},
	{0x00007000, 1, {0x00070001, 0x0}},
	{0x00008000, 1, {0x0001E085, 0x0}},
	{0x00009008, 1, {0x00000644, 0x0}},
	{0x00000004, 1, {0xF0005042, 0x0}},
	{0x00000018, 1, {0x00000200, 0x0}},
	{0x00001014, 1, {0x11A1504D, 0x0}},
	//reduce sensitivity.
	{0x00001010, 1, {0x11a163f4, 0x0}},
	{0x00001018, 1, {0x11a137f0, 0x0}},
	//	
	{0x00009004, 1, {0x00000000, 0x0}},
	{SEN_CMD_DELAY, 1, {1, 0x0}},
	{0x00009000, 1, {0x00000200, 0x0}},
	// start
	{0x0000B000, 1, {0x000002F9, 0x0}},
	{0x00009028, 1, {0x00000000, 0x0}},
	//RegisterOperation::WriteField(0x00009008, 0x645, 0x00000001),
	{0x00009008, 1, {0x00000645, 0x0}},
	// Analog START
	{0x0000002C, 1, {0x0022C724, 0x0}},
	{0x00000004, 1, {0xF0005442, 0x0}},	
};

static UINT32 cur_sen_mode[CTL_SEN_ID_MAX] = {CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1};
static UINT32 cur_fps[CTL_SEN_ID_MAX] = {0};
static UINT32 chgmode_fps[CTL_SEN_ID_MAX] = {0};
static UINT32 power_ctrl_mclk[CTL_SEN_CLK_SEL_MAX] = {0};
static UINT32 reset_ctrl_count[CTL_SEN_ID_MAX] = {0};
static UINT32 pwdn_ctrl_count[CTL_SEN_ID_MAX] = {0};
static ISP_SENSOR_PRESET_CTRL preset_ctrl[CTL_SEN_ID_MAX] = {0};
static INT32 is_fastboot[CTL_SEN_ID_MAX];
static UINT32 fastboot_i2c_id[CTL_SEN_ID_MAX];
static UINT32 fastboot_i2c_addr[CTL_SEN_ID_MAX];
static BOOL i2c_valid[CTL_SEN_ID_MAX];

static CTL_SEN_DRV_TAB imx636_sen_drv_tab = {
	sen_open_imx636,
	sen_close_imx636,
	sen_sleep_imx636,
	sen_wakeup_imx636,
	sen_write_reg_imx636,
	sen_read_reg_imx636,
	sen_chg_mode_imx636,
	sen_chg_fps_imx636,
	sen_set_info_imx636,
	sen_get_info_imx636,
};

static CTL_SEN_DRV_TAB *sen_get_drv_tab_imx636(void)
{
	return &imx636_sen_drv_tab;
}

static void sen_pwr_ctrl_imx636(CTL_SEN_ID id, CTL_SEN_PWR_CTRL_FLAG flag, CTL_SEN_CLK_CB clk_cb)
{
	UINT32 i = 0;
	UINT32 reset_count = 0, pwdn_count = 0;
	DBG_IND("enter flag %d \r\n", flag);

	if ((flag == CTL_SEN_PWR_CTRL_TURN_ON) && ((!is_fastboot[id]) || (fastboot_i2c_id[id] != sen_i2c[id].id) || (fastboot_i2c_addr[id] != sen_i2c[id].addr))) {

		if (sen_power[id].pwdn_pin != CTL_SEN_IGNORE) {
			for ( i = 0; i < CTL_SEN_ID_MAX ; i++ ) {
				if ( pwdn_ctrl_count[i] == (sen_power[id].pwdn_pin)) {
					pwdn_count++;
				}
			}
			pwdn_ctrl_count[id] = (sen_power[id].pwdn_pin);

			if (!pwdn_count) {
				gpio_direction_output((sen_power[id].pwdn_pin), 0);
				gpio_set_value((sen_power[id].pwdn_pin), 0);
				gpio_set_value((sen_power[id].pwdn_pin), 1);
				vos_util_delay_ms(sen_power[id].stable_time);				
			}
		}

		if (clk_cb != NULL) {
			if (sen_power[id].mclk != CTL_SEN_IGNORE) {
				if (sen_power[id].mclk == CTL_SEN_CLK_SEL_SIEMCLK) {
					power_ctrl_mclk[CTL_SEN_CLK_SEL_SIEMCLK] += 1;
				} else if (sen_power[id].mclk == CTL_SEN_CLK_SEL_SIEMCLK2) {
					power_ctrl_mclk[CTL_SEN_CLK_SEL_SIEMCLK2] += 1;
				} else if (sen_power[id].mclk == CTL_SEN_CLK_SEL_SIEMCLK3) {
					power_ctrl_mclk[CTL_SEN_CLK_SEL_SIEMCLK3] += 1;
				} else {
					power_ctrl_mclk[CTL_SEN_CLK_SEL_SIEMCLK4] += 1;
				}
				if (1 == (power_ctrl_mclk[sen_power[id].mclk])) {
					clk_cb(sen_power[id].mclk, TRUE);
					vos_util_delay_ms(sen_power[id].stable_time);					
				}
			}
		}

		if (sen_power[id].rst_pin != CTL_SEN_IGNORE) {
			vos_util_delay_ms(sen_power[id].stable_time);
			for ( i = 0; i < CTL_SEN_ID_MAX ; i++ ) {
				if ( reset_ctrl_count[i] == (sen_power[id].rst_pin)) {
					reset_count++;
				}
			}
			reset_ctrl_count[id] = (sen_power[id].rst_pin);

			if (!reset_count) {
				gpio_direction_output((sen_power[id].rst_pin), 0);
				gpio_direction_output(((sen_power[id].rst_pin) + 1), 0); // evb test
				gpio_set_value((sen_power[id].rst_pin), 0);
				gpio_set_value(((sen_power[id].rst_pin) + 1), 0); // evb test				
				vos_util_delay_ms(sen_power[id].rst_time);
				gpio_set_value((sen_power[id].rst_pin), 1);
				gpio_set_value(((sen_power[id].rst_pin) + 1), 1); // evb test				
				vos_util_delay_ms(sen_power[id].stable_time);
			}
		}
	}

	if (flag == CTL_SEN_PWR_CTRL_TURN_OFF) {

		if (sen_power[id].rst_pin != CTL_SEN_IGNORE) {
			reset_ctrl_count[id] = 0;

			for ( i = 0; i < CTL_SEN_ID_MAX ; i++ ) {
				if ( reset_ctrl_count[i] == (sen_power[id].rst_pin)) {
					reset_count++;
				}
			}

			if (!reset_count) {
				gpio_direction_output((sen_power[id].rst_pin), 0);
				gpio_direction_output(((sen_power[id].rst_pin) + 1), 0); // evb test				
				gpio_set_value((sen_power[id].rst_pin), 0);
				gpio_set_value(((sen_power[id].rst_pin) + 1), 0); // evb test				
				vos_util_delay_ms(sen_power[id].stable_time);
			}
		}
		
		if (clk_cb != NULL) {
			if (sen_power[id].mclk != CTL_SEN_IGNORE) {			
				if (sen_power[id].mclk == CTL_SEN_CLK_SEL_SIEMCLK) {
					power_ctrl_mclk[CTL_SEN_CLK_SEL_SIEMCLK] -= 1;
				} else if (sen_power[id].mclk == CTL_SEN_CLK_SEL_SIEMCLK2) {
					power_ctrl_mclk[CTL_SEN_CLK_SEL_SIEMCLK2] -= 1;
				} else if (sen_power[id].mclk == CTL_SEN_CLK_SEL_SIEMCLK3) {
					power_ctrl_mclk[CTL_SEN_CLK_SEL_SIEMCLK3] -= 1;
				} else {
					power_ctrl_mclk[CTL_SEN_CLK_SEL_SIEMCLK4] -= 1;
				}
				if (!power_ctrl_mclk[sen_power[id].mclk]) {
					clk_cb(sen_power[id].mclk, FALSE);
					vos_util_delay_ms(sen_power[id].stable_time);					
				}
			}
		}
		
		if (sen_power[id].pwdn_pin != CTL_SEN_IGNORE) {
			pwdn_ctrl_count[id] = 0;

			for ( i = 0; i < CTL_SEN_ID_MAX ; i++ ) {
				if ( pwdn_ctrl_count[i] == (sen_power[id].pwdn_pin)) {
					pwdn_count++;
				}
			}

			if (!pwdn_count) {
				gpio_direction_output((sen_power[id].pwdn_pin), 0);
				gpio_set_value((sen_power[id].pwdn_pin), 0);
				vos_util_delay_ms(sen_power[id].stable_time);				
			}
		}		
	}
}

static CTL_SEN_CMD sen_set_cmd_info_imx636(UINT32 addr, UINT32 data_length, UINT32 data0, UINT32 data1)
{
	CTL_SEN_CMD cmd;

	cmd.addr = addr;
	cmd.data_len = data_length;
	cmd.data[0] = data0;
	cmd.data[1] = data1;
	return cmd;
}

static ER sen_open_imx636(CTL_SEN_ID id)
{
	ER rt = E_OK;

	preset_ctrl[id].mode = ISP_SENSOR_PRESET_DEFAULT;
	i2c_valid[id] = TRUE;
	if ((!is_fastboot[id]) || (fastboot_i2c_id[id] != sen_i2c[id].id) || (fastboot_i2c_addr[id] != sen_i2c[id].addr)) {
		rt = sen_i2c_init_driver(id, &sen_i2c[id]);

		if (rt != E_OK) {
			i2c_valid[id] = FALSE;

			DBG_ERR("init. i2c driver fail (%d) \r\n", id);
		}
	}

	return rt;
}

static ER sen_close_imx636(CTL_SEN_ID id)
{
	if ((!is_fastboot[id]) || (fastboot_i2c_id[id] != sen_i2c[id].id) || (fastboot_i2c_addr[id] != sen_i2c[id].addr)) {
		if (i2c_valid[id]) {
			sen_i2c_remove_driver(id);
		}
	} else {
		is_fastboot[id] = 0;
		#if defined(__KERNEL__)
		isp_builtin_uninit_i2c(id);
		#endif
	}

	i2c_valid[id] = FALSE;

	return E_OK;
}

static ER sen_sleep_imx636(CTL_SEN_ID id)
{
	DBG_IND("enter \r\n");
	return E_OK;
}

static ER sen_wakeup_imx636(CTL_SEN_ID id)
{
	DBG_IND("enter \r\n");
	return E_OK;
}

static ER sen_write_reg_imx636(CTL_SEN_ID id, CTL_SEN_CMD *cmd)
{
	struct i2c_msg msgs;
	unsigned char buf[8];
	int i;

	if (!i2c_valid[id]) {
		return E_NOSPT;
	}

	buf[0]     = (cmd->addr >> 24) & 0xFF;
	buf[1]     = (cmd->addr >> 16) & 0xFF;
	buf[2]     = (cmd->addr >> 8) & 0xFF;		
	buf[3]     = cmd->addr & 0xFF;
	buf[4]     = (cmd->data[0] >> 24) & 0xFF;	
	buf[5]     = (cmd->data[0] >> 16) & 0xFF;		
	buf[6]     = (cmd->data[0] >> 8) & 0xFF;			
	buf[7]     = cmd->data[0] & 0xFF;
	msgs.addr  = sen_i2c[id].addr;
	msgs.flags = 0;
	msgs.len   = 8;
	msgs.buf   = buf;

	if ((!is_fastboot[id]) || (fastboot_i2c_id[id] != sen_i2c[id].id) || (fastboot_i2c_addr[id] != sen_i2c[id].addr)) {
		i = 0;
		while(1){
			if (sen_i2c_transfer(id, &msgs, 1) == 0)
				break;
			i++;
			if (i == 5)
				return E_SYS;
		}
	} else {
		#if defined(__KERNEL__)
		isp_builtin_set_transfer_i2c(id, &msgs, 1);
		#endif
	}

	return E_OK;
}

static ER sen_read_reg_imx636(CTL_SEN_ID id, CTL_SEN_CMD *cmd)
{
	struct i2c_msg  msgs[2];
	unsigned char   tmp[4], tmp2[4];
	int i;

	if (!i2c_valid[id]) {
		return E_NOSPT;
	}
/*
	tmp[0]        = (cmd->addr >> 8) & 0xFF;
	tmp[1]        = cmd->addr & 0xFF;
	msgs[0].addr  = sen_i2c[id].addr;
	msgs[0].flags = 0;
	msgs[0].len   = 2;
	msgs[0].buf   = tmp;

	tmp2[0]       = 0;
	msgs[1].addr  = sen_i2c[id].addr;
	msgs[1].flags = 1;
	msgs[1].len   = 1;
	msgs[1].buf   = tmp2;
*/
	tmp[0]        = (cmd->addr >> 24) & 0xFF;
	tmp[1]        = (cmd->addr >> 16) & 0xFF;
	tmp[2]        = (cmd->addr >> 8) & 0xFF;
	tmp[3]        = cmd->addr & 0xFF;		
	msgs[0].addr  = sen_i2c[id].addr;
	msgs[0].flags = 0;
	msgs[0].len   = 4;
	msgs[0].buf   = tmp;

	tmp2[0]       = 0;
	tmp2[1]       = 0;
	tmp2[2]       = 0;
	tmp2[3]       = 0;			
	msgs[1].addr  = sen_i2c[id].addr;
	msgs[1].flags = 1;
	msgs[1].len   = 4;
	msgs[1].buf   = tmp2;



	if ((!is_fastboot[id]) || (fastboot_i2c_id[id] != sen_i2c[id].id) || (fastboot_i2c_addr[id] != sen_i2c[id].addr)) {
		i = 0;
		while(1){
			if (sen_i2c_transfer(id, msgs, 2) == 0)
				break;
			i++;
			if (i == 5)
				return E_SYS;
		}
	} else {
		#if defined(__KERNEL__)
		isp_builtin_set_transfer_i2c(id, msgs, 2);
		#endif
	}

	cmd->data[0] = (tmp2[0] << 24) | (tmp2[1] << 16) | (tmp2[2] << 8) | (tmp2[3]);

	return E_OK;
}

static UINT32 sen_get_cmd_tab_imx636(CTL_SEN_MODE mode, CTL_SEN_CMD **cmd_tab)
{
	switch (mode) {
	case CTL_SEN_MODE_1:
		*cmd_tab = imx636_mode_1;
		return sizeof(imx636_mode_1) / sizeof(CTL_SEN_CMD);

	default:
		DBG_ERR("sensor mode %d no cmd table\r\n", mode);
		*cmd_tab = NULL;
		return 0;
	}
}

static ER sen_chg_mode_imx636(CTL_SEN_ID id, CTL_SENDRV_CHGMODE_OBJ chgmode_obj)
{
	CTL_SEN_CMD *p_cmd_list = NULL, cmd;
	UINT32 idx, cmd_num = 0;
	ER rt = E_OK;

	cur_sen_mode[id] = chgmode_obj.mode;

	sen_set_cur_fps_imx636(id, mode_basic_param[cur_sen_mode[id]].dft_fps);
	sen_set_chgmode_fps_imx636(id, mode_basic_param[cur_sen_mode[id]].dft_fps);

	if (is_fastboot[id]) {
		preset_ctrl[id].mode = ISP_SENSOR_PRESET_CHGMODE;
		return E_OK;
	}

	// get & set sensor cmd table
	cmd_num = sen_get_cmd_tab_imx636(chgmode_obj.mode, &p_cmd_list);
	if (p_cmd_list == NULL) {
		DBG_ERR("%s: SenMode(%d) out of range!!! \r\n", __func__, chgmode_obj.mode);
		return E_SYS;
	}

	for (idx = 0; idx < cmd_num; idx++) {
		if (p_cmd_list[idx].addr == SEN_CMD_DELAY) {
			vos_util_delay_ms((p_cmd_list[idx].data[0] & 0xFF) | ((p_cmd_list[idx].data[1] & 0xFF) << 8));
		} else if (p_cmd_list[idx].addr == SEN_CMD_SETVD) {
		} else if (p_cmd_list[idx].addr == SEN_CMD_PRESET) {
		} else if (p_cmd_list[idx].addr == SEN_CMD_DIRECTION) {
		} else {
			cmd = sen_set_cmd_info_imx636(p_cmd_list[idx].addr, p_cmd_list[idx].data_len, p_cmd_list[idx].data[0], p_cmd_list[idx].data[1]);
			rt |= sen_write_reg_imx636(id, &cmd);
		}
	}

	preset_ctrl[id].mode = ISP_SENSOR_PRESET_CHGMODE;

	if (rt != E_OK) {
		DBG_ERR("write register error %d \r\n", (INT)rt);
		return rt;
	}

	return E_OK;
}

static ER sen_chg_fps_imx636(CTL_SEN_ID id, UINT32 fps)
{
	ER rt = E_OK;
	return rt;
}

static ER sen_set_info_imx636(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data)
{
	switch (drv_cfg_id) {
	default:
		return E_NOSPT;
	}
	return E_OK;
}

static ER sen_get_info_imx636(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data)
{
	ER rt = E_OK;

	switch (drv_cfg_id) {
	case CTL_SENDRV_CFGID_GET_ATTR_BASIC:
		sen_get_attr_basic_imx636(id, (CTL_SENDRV_GET_ATTR_BASIC_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_ATTR_SIGNAL:
		sen_get_attr_signal_imx636((CTL_SENDRV_GET_ATTR_SIGNAL_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_ATTR_CMDIF:
		rt = sen_get_attr_cmdif_imx636(id, (CTL_SENDRV_GET_ATTR_CMDIF_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_ATTR_IF:
		rt = sen_get_attr_if_imx636((CTL_SENDRV_GET_ATTR_IF_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_FPS:
		sen_get_fps_imx636(id, (CTL_SENDRV_GET_FPS_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_SPEED:
		sen_get_speed_imx636(id, (CTL_SENDRV_GET_SPEED_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_MODE_BASIC:
		sen_get_mode_basic_imx636((CTL_SENDRV_GET_MODE_BASIC_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_MODE_MIPI:
		sen_get_mode_mipi_imx636((CTL_SENDRV_GET_MODE_MIPI_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_MODESEL:
		sen_get_modesel_imx636((CTL_SENDRV_GET_MODESEL_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_MODE_ROWTIME:
		sen_get_rowtime_imx636(id, (CTL_SENDRV_GET_MODE_ROWTIME_PARAM *)(data));
		break;
	default:
		rt = E_NOSPT;
	}
	return rt;
}

static void sen_get_mode_basic_imx636(CTL_SENDRV_GET_MODE_BASIC_PARAM *mode_basic)
{
	UINT32 mode = mode_basic->mode;
	
	if (mode >= SEN_MAX_MODE) {
		mode = 0;
	}
	memcpy(mode_basic, &mode_basic_param[mode], sizeof(CTL_SENDRV_GET_MODE_BASIC_PARAM));
}

static void sen_get_attr_basic_imx636(CTL_SEN_ID id, CTL_SENDRV_GET_ATTR_BASIC_PARAM *data)
{
	memcpy(data, &basic_param, sizeof(CTL_SENDRV_GET_ATTR_BASIC_PARAM));
}

static void sen_get_attr_signal_imx636(CTL_SENDRV_GET_ATTR_SIGNAL_PARAM *data)
{
	memcpy(data, &signal_param, sizeof(CTL_SENDRV_GET_ATTR_SIGNAL_PARAM));
}

static ER sen_get_attr_cmdif_imx636(CTL_SEN_ID id, CTL_SENDRV_GET_ATTR_CMDIF_PARAM *data)
{
	data->type = CTL_SEN_CMDIF_TYPE_I2C;
	memcpy(&data->info, &i2c, sizeof(CTL_SENDRV_I2C));
	data->info.i2c.ch = sen_i2c[id].id;
	data->info.i2c.w_addr_info[0].w_addr = sen_i2c[id].addr;
	data->info.i2c.cur_w_addr_info.w_addr_sel = data->info.i2c.w_addr_info[0].w_addr_sel;
	data->info.i2c.cur_w_addr_info.w_addr = data->info.i2c.w_addr_info[0].w_addr;
	return E_OK;
}

static ER sen_get_attr_if_imx636(CTL_SENDRV_GET_ATTR_IF_PARAM *data)
{
	#if 1
	if (data->type == CTL_SEN_IF_TYPE_MIPI) {
		return E_OK;
	}
	return E_NOSPT;
	#else
	if (data->type == CTL_SEN_IF_TYPE_MIPI) {
		memcpy(&data->info.mipi, &mipi, sizeof(CTL_SENDRV_MIPI));
		return E_OK;
	}
	return E_NOSPT;
	#endif
}

static void sen_get_fps_imx636(CTL_SEN_ID id, CTL_SENDRV_GET_FPS_PARAM *data)
{
	data->cur_fps = sen_get_cur_fps_imx636(id);
	data->chg_fps = sen_get_chgmode_fps_imx636(id);
}

static void sen_get_speed_imx636(CTL_SEN_ID id, CTL_SENDRV_GET_SPEED_PARAM *data)
{
	UINT32 mode = data->mode;

	if (mode >= SEN_MAX_MODE) {
		mode = 0;
	}
	memcpy(data, &speed_param[mode], sizeof(CTL_SENDRV_GET_SPEED_PARAM));

	if (sen_power[id].mclk == CTL_SEN_CLK_SEL_SIEMCLK) {
		data->mclk_src = CTL_SEN_SIEMCLK_SRC_MCLK;
	} else if (sen_power[id].mclk == CTL_SEN_CLK_SEL_SIEMCLK2) {
		data->mclk_src = CTL_SEN_SIEMCLK_SRC_MCLK2;
	} else if (sen_power[id].mclk == CTL_SEN_CLK_SEL_SIEMCLK3) {
		data->mclk_src = CTL_SEN_SIEMCLK_SRC_MCLK3;
	} else if (sen_power[id].mclk == CTL_SEN_CLK_SEL_SIEMCLK4) {
		data->mclk_src = CTL_SEN_SIEMCLK_SRC_MCLK4;
	} else if (sen_power[id].mclk == CTL_SEN_IGNORE) {
		data->mclk_src = CTL_SEN_SIEMCLK_SRC_IGNORE;
	} else {
		DBG_ERR("mclk source is fail \r\n");
	}
}

static void sen_get_mode_mipi_imx636(CTL_SENDRV_GET_MODE_MIPI_PARAM *data)
{
	UINT32 mode = data->mode;
	
	if (mode >= SEN_MAX_MODE) {
		mode = 0;
	}
	memcpy(data, &mipi_param[mode], sizeof(CTL_SENDRV_GET_MODE_MIPI_PARAM));
}

static void sen_get_modesel_imx636(CTL_SENDRV_GET_MODESEL_PARAM *data)
{
	if (data->if_type != CTL_SEN_IF_TYPE_MIPI) {
		DBG_ERR("if_type %d N.S. \r\n", data->if_type);
		return;
	}

	if (data->data_fmt != CTL_SEN_DATA_FMT_RGB) {
		DBG_ERR("data_fmt %d N.S. \r\n", data->data_fmt);
		return;
	}

	if (data->frame_num == 1) {
		if ((data->size.w <= 1280) && (data->size.h <= 720)) {
			if (data->frame_rate <= 25000) {
				data->mode = CTL_SEN_MODE_1;
				return;
			}
		} 
	} 

	DBG_ERR("fail (frame_rate=%d,size=%d*%d,if_type=%d,data_fmt=%d,frame_num=%d,data_lane=%d,mode_type_sel=%llx,pixdepth=%lld) \r\n"
			, data->frame_rate, data->size.w, data->size.h, data->if_type, data->data_fmt, data->frame_num, data->data_lane, (UINT64)data->mode_type_sel,(UINT64)data->pixdepth);
	data->mode = CTL_SEN_MODE_1;
}

static void sen_get_rowtime_imx636(CTL_SEN_ID id, CTL_SENDRV_GET_MODE_ROWTIME_PARAM *data)
{
	data->row_time_step = 1;	
	data->row_time = 0;
}

static void sen_set_cur_fps_imx636(CTL_SEN_ID id, UINT32 fps)
{
	cur_fps[id] = fps;
}

static UINT32 sen_get_cur_fps_imx636(CTL_SEN_ID id)
{
	return cur_fps[id];
}

static void sen_set_chgmode_fps_imx636(CTL_SEN_ID id, UINT32 fps)
{
	chgmode_fps[id] = fps;
}

static UINT32 sen_get_chgmode_fps_imx636(CTL_SEN_ID id)
{
	return chgmode_fps[id];
}

#if defined(__FREERTOS)
void sen_get_i2c_id_imx636(CTL_SEN_ID id, UINT32 *i2c_id)
{
	*i2c_id = sen_i2c[id].id;
}

void sen_get_i2c_addr_imx636(CTL_SEN_ID id, UINT32 *i2c_addr)
{
	*i2c_addr = sen_i2c[id].addr;
}
int sen_init_imx636(SENSOR_DTSI_INFO *info)
{
	CTL_SEN_REG_OBJ reg_obj;
	CHAR node_path[64];
	CHAR compatible[64];
	UINT32 id;
	ER rt = E_OK;

	for (id = 0; id < CTL_SEN_ID_MAX; id++ ) {
		is_fastboot[id] = 0;
		fastboot_i2c_id[id] = 0xFFFFFFFF;
		fastboot_i2c_addr[id] = 0x0;
	}

	sprintf(compatible, "nvt,sen_imx636");
	if (sen_common_check_compatible(compatible)) {
		DBG_DUMP("compatible valid, using peri-dev.dtsi \r\n");
		sen_common_load_cfg_preset_compatible(compatible, &sen_preset);
		sen_common_load_cfg_direction_compatible(compatible, &sen_direction);
		sen_common_load_cfg_power_compatible(compatible, &sen_power);
		sen_common_load_cfg_i2c_compatible(compatible, &sen_i2c);
	} else if (info->addr != NULL) {
		DBG_DUMP("compatible not valid, using sensor.dtsi \r\n");
		sprintf(node_path, "/sensor/sen_cfg/sen_imx636");
		sen_common_load_cfg_map(info->addr, node_path, &sen_map);
		sen_common_load_cfg_preset(info->addr, node_path, &sen_preset);
		sen_common_load_cfg_direction(info->addr, node_path, &sen_direction);
		sen_common_load_cfg_power(info->addr, node_path, &sen_power);
		sen_common_load_cfg_i2c(info->addr, node_path, &sen_i2c);
	} else {
		DBG_WRN("DTSI addr is NULL \r\n");
	}

	memset((void *)(&reg_obj), 0, sizeof(CTL_SEN_REG_OBJ));
	reg_obj.pwr_ctrl = sen_pwr_ctrl_imx636;
	reg_obj.det_plug_in = NULL;
	reg_obj.drv_tab = sen_get_drv_tab_imx636();
	rt = ctl_sen_reg_sendrv("nvt_sen_imx636", &reg_obj);
	if (rt != E_OK) {
		DBG_WRN("register sensor driver fail \r\n");
	}

	return rt;
}

void sen_exit_imx636(void)
{
	ctl_sen_unreg_sendrv("nvt_sen_imx636");
}

#else
static int __init sen_init_imx636(void)
{
	INT8 cfg_path[MAX_PATH_NAME_LENGTH+1] = { '\0' };
	CFG_FILE_FMT *pcfg_file;
	CTL_SEN_REG_OBJ reg_obj;
	UINT32 id;
	ER rt = E_OK;

	for (id = 0; id < ISP_BUILTIN_ID_MAX_NUM; id++ ) {
		is_fastboot[id] = kdrv_builtin_is_fastboot();
		fastboot_i2c_id[id] = isp_builtin_get_i2c_id(id);
		fastboot_i2c_addr[id] = isp_builtin_get_i2c_addr(id);
	}

	// Parsing cfc file if exist
	if ((strstr(sen_cfg_path, "null")) || (strstr(sen_cfg_path, "NULL"))) {
		DBG_WRN("cfg file no exist \r\n");
		cfg_path[0] = '\0';
	} else {
		if ((sen_cfg_path != NULL) && (strlen(sen_cfg_path) <= MAX_PATH_NAME_LENGTH)) {
			strncpy((char *)cfg_path, sen_cfg_path, MAX_PATH_NAME_LENGTH);
		}

		if ((pcfg_file = sen_common_open_cfg(cfg_path)) != NULL) {
			DBG_MSG("load %s success \r\n", sen_cfg_path);
			sen_common_load_cfg_map(pcfg_file, &sen_map);
			sen_common_load_cfg_preset(pcfg_file, &sen_preset);
			sen_common_load_cfg_direction(pcfg_file, &sen_direction);
			sen_common_load_cfg_power(pcfg_file, &sen_power);
			sen_common_load_cfg_i2c(pcfg_file, &sen_i2c);
			sen_common_close_cfg(pcfg_file);
		} else {
			DBG_WRN("load cfg fail \r\n");
		}
	}

	memset((void *)(&reg_obj), 0, sizeof(CTL_SEN_REG_OBJ));
	reg_obj.pwr_ctrl = sen_pwr_ctrl_imx636;
	reg_obj.det_plug_in = NULL;
	reg_obj.drv_tab = sen_get_drv_tab_imx636();
	rt = ctl_sen_reg_sendrv("nvt_sen_imx636", &reg_obj);
	if (rt != E_OK) {
		DBG_WRN("register sensor driver fail \r\n");
	}

	return rt;
}

static void __exit sen_exit_imx636(void)
{
	ctl_sen_unreg_sendrv("nvt_sen_imx636");
}

module_init(sen_init_imx636);
module_exit(sen_exit_imx636);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION(SEN_IMX636_MODULE_NAME);
MODULE_LICENSE("GPL");
#endif

