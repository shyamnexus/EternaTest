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
VOS_MODULE_VERSION(nvt_sen_rtd6122c, 1, 15, 001, 00);

//=============================================================================
// information
//=============================================================================
#define SEN_RTD6122C_MODULE_NAME     "sen_rtd6122c"
#define SEN_MAX_MODE               1

#define SEN_I2C_ADDR 0x34>>1
#define SEN_I2C_COMPATIBLE "nvt,sen_rtd6122c"
#define USE_I2C_INTERFACE 0

#include "sen_i2c.c"

//=============================================================================
// function declaration
//=============================================================================
static CTL_SEN_DRV_TAB *sen_get_drv_tab_rtd6122c(void);
static void sen_pwr_ctrl_rtd6122c(CTL_SEN_ID id, CTL_SEN_PWR_CTRL_FLAG flag, CTL_SEN_CLK_CB clk_cb);
static ER sen_open_rtd6122c(CTL_SEN_ID id);
static ER sen_close_rtd6122c(CTL_SEN_ID id);
static ER sen_sleep_rtd6122c(CTL_SEN_ID id);
static ER sen_wakeup_rtd6122c(CTL_SEN_ID id);
#ifdef USE_I2C_INTERFACE
static ER sen_write_reg_rtd6122c(CTL_SEN_ID id, CTL_SEN_CMD *cmd);
static ER sen_read_reg_rtd6122c(CTL_SEN_ID id, CTL_SEN_CMD *cmd);
#endif
static ER sen_chg_mode_rtd6122c(CTL_SEN_ID id, CTL_SENDRV_CHGMODE_OBJ chgmode_obj);
static ER sen_chg_fps_rtd6122c(CTL_SEN_ID id, UINT32 fps);
static ER sen_set_info_rtd6122c(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data);
static ER sen_get_info_rtd6122c(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data);

static void sen_get_mode_basic_rtd6122c(CTL_SENDRV_GET_MODE_BASIC_PARAM *mode_basic);
static void sen_get_attr_basic_rtd6122c(CTL_SEN_ID id, CTL_SENDRV_GET_ATTR_BASIC_PARAM *data);
static void sen_get_attr_signal_rtd6122c(CTL_SENDRV_GET_ATTR_SIGNAL_PARAM *data);
static ER sen_get_attr_cmdif_rtd6122c(CTL_SEN_ID id, CTL_SENDRV_GET_ATTR_CMDIF_PARAM *data);
static ER sen_get_attr_if_rtd6122c(CTL_SENDRV_GET_ATTR_IF_PARAM *data);
static void sen_get_fps_rtd6122c(CTL_SEN_ID id, CTL_SENDRV_GET_FPS_PARAM *data);
static void sen_get_speed_rtd6122c(CTL_SEN_ID id, CTL_SENDRV_GET_SPEED_PARAM *data);
static void sen_get_modesel_rtd6122c(CTL_SENDRV_GET_MODESEL_PARAM *data);
static void sen_set_cur_fps_rtd6122c(CTL_SEN_ID id, UINT32 fps);
static void sen_get_rowtime_rtd6122c(CTL_SEN_ID id, CTL_SENDRV_GET_MODE_ROWTIME_PARAM *data);
static UINT32 sen_get_cur_fps_rtd6122c(CTL_SEN_ID id);
static void sen_set_chgmode_fps_rtd6122c(CTL_SEN_ID id, UINT32 fps);
static UINT32 sen_get_chgmode_fps_rtd6122c(CTL_SEN_ID id);
static void sen_get_thermal_para_rtd6122c(CTL_SENDRV_GET_THERMAL_PARAM *data);
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
	SEN_RTD6122C_MODULE_NAME,
	CTL_SEN_VENDOR_OTHERS,
	SEN_MAX_MODE,
	CTL_SEN_SUPPORT_PROPERTY_NONE,
	0
};

static CTL_SENDRV_GET_ATTR_SIGNAL_PARAM signal_param = {
	CTL_SEN_SIGNAL_MASTER,
	{CTL_SEN_ACTIVE_HIGH, CTL_SEN_ACTIVE_HIGH, CTL_SEN_PHASE_RISING, CTL_SEN_PHASE_RISING, CTL_SEN_PHASE_FALLING}
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
		36750000,
		25000000,//
		30000000//PCLK = 25M
	}
};

static CTL_SENDRV_GET_MODE_BASIC_PARAM mode_basic_param[SEN_MAX_MODE] = {
	{
		CTL_SEN_MODE_1,
		CTL_SEN_IF_TYPE_TDIO,
		CTL_SEN_DATA_FMT_THERMAL,
		CTL_SEN_MODE_THERMAL,
		2500,
		1,
		CTL_SEN_STPIX_Y_ONLY,
		CTL_SEN_PIXDEPTH_8BIT,
		CTL_SEN_FMT_POGRESSIVE,
		{678, 522}, //654 + 22 + 2, 522
		{{2, 0, 676, 522}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
		{676, 522},
		{0, 100, 0, 100},//don't care
		CTL_SEN_RATIO(4,3),
		{1000, 1000},//don"t care
		100
	}
};


static CTL_SENDRV_GET_THERMAL_PARAM thermal_param[SEN_MAX_MODE] = {
	{
		CTL_SEN_MODE_1, // CTL_SEN_MODE mode;
		CTL_SEN_THERMAL_TX_CFG_MODE_SD0, // CTL_SEN_THERMAL_TX_CFG_MODE tx_mode;
		96, // UINT16 tx_len;
		{0x00000000, // reset
		 0xffffffff,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000}, // UINT32 tx_data[CTL_SEN_THERMAL_CFG_NUM];
		{CTL_SEN_THERMAL_TX_LEVEL_LOW,
		 CTL_SEN_THERMAL_TX_LEVEL_LOW,
		 CTL_SEN_THERMAL_TX_LEVEL_LOW,
		 CTL_SEN_THERMAL_TX_LEVEL_LOW,
		 CTL_SEN_THERMAL_TX_LEVEL_LOW,
		 CTL_SEN_THERMAL_TX_LEVEL_LOW,
		 CTL_SEN_THERMAL_TX_LEVEL_LOW}, //CTL_SEN_THERMAL_TX_LEVEL_SEL tx_lvl[CTL_SEN_THERMAL_LVL_NUM];
		0,	// tx_period, 539A supported
		{0, 0}, // tx_blanking[2], 539A supported
		CTL_SEN_THERMAL_TX_SYNC_CODE_MODE_SD0, // CTL_SEN_THERMAL_TX_SYNC_CODE_MODE sync_mode;
		14, // UINT16 fs_code_len;
		14, // UINT16 ls_code_len;
		{0x000032ff,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000}, //UINT32 fs_code[CTL_SEN_THERMAL_FSCODE_NUM];
		{0x00003aff,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000}, //UINT32 ls_code[CTL_SEN_THERMAL_LSCODE_NUM];
		9, //UINT16 fs_delay;
		9, //UINT16 ls_delay;
	
		/*539A supported*/
		FALSE, // tx_combine_en
		FALSE, // tx_fs_en
		CTL_SEN_THERMAL_TX_FS_IDLE_MODE_NS, // CTL_SEN_THERMAL_TX_FS_IDLE_MODE_SEL
		CTL_SEN_THERMAL_TX_FS_INV_NS, // CTL_SEN_THERMAL_TX_FS_INV_SEL
		{0, 0, 0}, // CTL_SEN_THERMAL_TX_FS_CLKCNT
		/*539A supported*/

		CTL_SEN_THERMAL_TX_OOC_MODE_SD0_1_2PXL_M1, //CTL_SEN_THERMAL_TX_OOC_MODE_SEL ooc_mode;
		CTL_SEN_THERMAL_TX_OOC_BITDEPTH_7, //CTL_SEN_THERMAL_TX_OOC_BITDEPTH_SEL ooc_bitdepth;
		TRUE, //BOOL ooc_data_swap;
		2, //UINT8 ooc_data_r_shift;
        0, //UINT8 ooc_err_det_code;
		0, //UINT8 ooc_err_rep_code;
        654, //UINT16 ooc_width;
        522, //UINT16 ooc_height;
        0, //UINT8 ooc_dummy_top_cnt;
        0, //UINT8 ooc_dummy_bot_cnt;
        0, //UINT8 ooc_dummy_val;
        {0,
         2797,
         0}, //UINT16 ooc_vblanking[CTL_SEN_THERMAL_BNK_NUM];
        {0,
         7,
         501},//UINT16 ooc_hblanking[CTL_SEN_THERMAL_BNK_NUM];
		CTL_SEN_THERMAL_RX_DECODE_MODE_TX_FS_LS, //CTL_SEN_THERMAL_RX_DECODE_MODE rx_mode;
		CTL_SEN_THERMAL_RX_OUT_FMT_LSB, //CTL_SEN_THERMAL_RX_OUT_FMT out_fmt;
		FALSE, //BOOL hi_byte_inv;
		FALSE, //BOOL lo_byte_inv;
		0, //UINT8 hi_byte_r_shift;
		0, //UINT8 lo_byte_r_shift;
		FALSE, //BOOL two_byte_swap;
		FALSE, //BOOL two_byte_inv;
		1 //UINT8 two_byte_r_shift;
	}
};

static UINT32 cur_sen_mode[CTL_SEN_ID_MAX] = {CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1};
static UINT32 cur_fps[CTL_SEN_ID_MAX] = {0};
static UINT32 chgmode_fps[CTL_SEN_ID_MAX] = {0};
static UINT32 power_ctrl_mclk[CTL_SEN_CLK_SEL_MAX] = {0};
static UINT32 reset_ctrl_count[CTL_SEN_ID_MAX] = {0};
static UINT32 pwdn_ctrl_count[CTL_SEN_ID_MAX] = {0};
static INT32 is_fastboot[CTL_SEN_ID_MAX];
static UINT32 fastboot_i2c_id[CTL_SEN_ID_MAX];
static UINT32 fastboot_i2c_addr[CTL_SEN_ID_MAX];
static BOOL i2c_valid[CTL_SEN_ID_MAX];

static CTL_SEN_DRV_TAB rtd6122c_sen_drv_tab = {
	sen_open_rtd6122c,
	sen_close_rtd6122c,
	sen_sleep_rtd6122c,
	sen_wakeup_rtd6122c,
	sen_write_reg_rtd6122c,
	sen_read_reg_rtd6122c,
	sen_chg_mode_rtd6122c,
	sen_chg_fps_rtd6122c,
	sen_set_info_rtd6122c,
	sen_get_info_rtd6122c,
};

static CTL_SEN_DRV_TAB *sen_get_drv_tab_rtd6122c(void)
{
	return &rtd6122c_sen_drv_tab;
}

static void sen_pwr_ctrl_rtd6122c(CTL_SEN_ID id, CTL_SEN_PWR_CTRL_FLAG flag, CTL_SEN_CLK_CB clk_cb)
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
				gpio_set_value((sen_power[id].rst_pin), 0);
				vos_util_delay_ms(sen_power[id].rst_time);
				gpio_set_value((sen_power[id].rst_pin), 1);
				vos_util_delay_ms(sen_power[id].stable_time);
			}
		}
	}

	if (flag == CTL_SEN_PWR_CTRL_TURN_OFF) {

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
			}
		}

		if (sen_power[id].rst_pin != CTL_SEN_IGNORE) {
			reset_ctrl_count[id] = 0;

			for ( i = 0; i < CTL_SEN_ID_MAX ; i++ ) {
				if ( reset_ctrl_count[i] == (sen_power[id].rst_pin)) {
					reset_count++;
				}
			}

			if (!reset_count) {
				gpio_direction_output((sen_power[id].rst_pin), 0);
				gpio_set_value((sen_power[id].rst_pin), 0);
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
				}
			}
		}
	}
}

/*
#ifdef USE_I2C_INTERFACE
static CTL_SEN_CMD sen_set_cmd_info_rtd6122c(UINT32 addr, UINT32 data_length, UINT32 data0, UINT32 data1)
{
	CTL_SEN_CMD cmd;

	cmd.addr = addr;
	cmd.data_len = data_length;
	cmd.data[0] = data0;
	cmd.data[1] = data1;
	return cmd;
}
#endif
*/

#if defined(__KERNEL__)
static void sen_load_cfg_from_compatible_rtd6122c(struct device_node *of_node)
{
	DBG_DUMP("compatible valid, using peri-dev.dtsi \r\n");
	sen_common_load_cfg_preset_compatible(of_node, &sen_preset);
	sen_common_load_cfg_direction_compatible(of_node, &sen_direction);
	sen_common_load_cfg_power_compatible(of_node, &sen_power);
	sen_common_load_cfg_i2c_compatible(of_node, &sen_i2c);
}
#endif

static ER sen_open_rtd6122c(CTL_SEN_ID id)
{
	ER rt = E_OK;

	#if defined(__KERNEL__)
#ifdef USE_I2C_INTERFACE
	sen_i2c_reg_cb(sen_load_cfg_from_compatible_rtd6122c);
#endif
	#endif

	i2c_valid[id] = TRUE;
	if ((!is_fastboot[id]) || (fastboot_i2c_id[id] != sen_i2c[id].id) || (fastboot_i2c_addr[id] != sen_i2c[id].addr)) {
#ifdef USE_I2C_INTERFACE
		rt = sen_i2c_init_driver(id, &sen_i2c[id]);
#endif
		if (rt != E_OK) {
			i2c_valid[id] = FALSE;

			DBG_ERR("init. i2c driver fail (%d) \r\n", id);
		}
	}

	return rt;
}

static ER sen_close_rtd6122c(CTL_SEN_ID id)
{
	if ((!is_fastboot[id]) || (fastboot_i2c_id[id] != sen_i2c[id].id) || (fastboot_i2c_addr[id] != sen_i2c[id].addr)) {
		if (i2c_valid[id]) {
#ifdef USE_I2C_INTERFACE
			sen_i2c_remove_driver(id);
#endif
		}
	} else {
		is_fastboot[id] = 0;
		#if defined(__KERNEL__)
#ifdef USE_I2C_INTERFACE
		isp_builtin_uninit_i2c(id);
#endif
		#endif
	}

	i2c_valid[id] = FALSE;

	return E_OK;
}

static ER sen_sleep_rtd6122c(CTL_SEN_ID id)
{
	DBG_IND("enter \r\n");
	return E_OK;
}

static ER sen_wakeup_rtd6122c(CTL_SEN_ID id)
{
	DBG_IND("enter \r\n");
	return E_OK;
}

#ifdef USE_I2C_INTERFACE
static ER sen_write_reg_rtd6122c(CTL_SEN_ID id, CTL_SEN_CMD *cmd)
{
	struct i2c_msg msgs;
	unsigned char buf[3];
	int i;

	if (!i2c_valid[id]) {
		return E_NOSPT;
	}

	buf[0]     = (cmd->addr >> 8) & 0xFF;
	buf[1]     = cmd->addr & 0xFF;
	buf[2]     = cmd->data[0] & 0xFF;
	msgs.addr  = sen_i2c[id].addr;
	msgs.flags = 0;
	msgs.len   = 3;
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

static ER sen_read_reg_rtd6122c(CTL_SEN_ID id, CTL_SEN_CMD *cmd)
{
	struct i2c_msg  msgs[2];
	unsigned char   tmp[2], tmp2[2];
	int i;

	if (!i2c_valid[id]) {
		return E_NOSPT;
	}

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

	cmd->data[0] = tmp2[0];

	return E_OK;
}
#endif

static ER sen_chg_mode_rtd6122c(CTL_SEN_ID id, CTL_SENDRV_CHGMODE_OBJ chgmode_obj)
{
	ER rt = E_OK;

	cur_sen_mode[id] = chgmode_obj.mode;
	sen_set_cur_fps_rtd6122c(id, mode_basic_param[cur_sen_mode[id]].dft_fps);
	sen_set_chgmode_fps_rtd6122c(id, mode_basic_param[cur_sen_mode[id]].dft_fps);

	if (is_fastboot[id]) {
		return E_OK;
	}

	if (rt != E_OK) {
		DBG_ERR("write register error %d \r\n", (INT)rt);
		return rt;
	}

	return E_OK;
}

static ER sen_chg_fps_rtd6122c(CTL_SEN_ID id, UINT32 fps)
{
	ER rt = E_OK;
	return rt;
}

static ER sen_set_info_rtd6122c(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data)
{
	switch (drv_cfg_id) {
	default:
		return E_NOSPT;
	}
	return E_OK;
}

static ER sen_get_info_rtd6122c(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data)
{
	ER rt = E_OK;

	switch (drv_cfg_id) {
	case CTL_SENDRV_CFGID_GET_ATTR_BASIC:
		sen_get_attr_basic_rtd6122c(id, (CTL_SENDRV_GET_ATTR_BASIC_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_ATTR_SIGNAL:
		sen_get_attr_signal_rtd6122c((CTL_SENDRV_GET_ATTR_SIGNAL_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_ATTR_CMDIF:
		rt = sen_get_attr_cmdif_rtd6122c(id, (CTL_SENDRV_GET_ATTR_CMDIF_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_ATTR_IF:
		rt = sen_get_attr_if_rtd6122c((CTL_SENDRV_GET_ATTR_IF_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_FPS:
		sen_get_fps_rtd6122c(id, (CTL_SENDRV_GET_FPS_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_SPEED:
		sen_get_speed_rtd6122c(id, (CTL_SENDRV_GET_SPEED_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_MODE_BASIC:
		sen_get_mode_basic_rtd6122c((CTL_SENDRV_GET_MODE_BASIC_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_MODESEL:
		sen_get_modesel_rtd6122c((CTL_SENDRV_GET_MODESEL_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_MODE_THERMAL:
		sen_get_thermal_para_rtd6122c((CTL_SENDRV_GET_THERMAL_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_MODE_ROWTIME:
		sen_get_rowtime_rtd6122c(id, (CTL_SENDRV_GET_MODE_ROWTIME_PARAM *)(data));
		break;
	default:
		rt = E_NOSPT;
	}
	return rt;
}

static void sen_get_mode_basic_rtd6122c(CTL_SENDRV_GET_MODE_BASIC_PARAM *mode_basic)
{
	UINT32 mode = mode_basic->mode;

	if (mode >= SEN_MAX_MODE) {
		mode = 0;
	}
	memcpy(mode_basic, &mode_basic_param[mode], sizeof(CTL_SENDRV_GET_MODE_BASIC_PARAM));
}

static void sen_get_attr_basic_rtd6122c(CTL_SEN_ID id, CTL_SENDRV_GET_ATTR_BASIC_PARAM *data)
{
	memcpy(data, &basic_param, sizeof(CTL_SENDRV_GET_ATTR_BASIC_PARAM));
}

static void sen_get_attr_signal_rtd6122c(CTL_SENDRV_GET_ATTR_SIGNAL_PARAM *data)
{
	memcpy(data, &signal_param, sizeof(CTL_SENDRV_GET_ATTR_SIGNAL_PARAM));
}

static ER sen_get_attr_cmdif_rtd6122c(CTL_SEN_ID id, CTL_SENDRV_GET_ATTR_CMDIF_PARAM *data)
{
	data->type = CTL_SEN_CMDIF_TYPE_I2C;
	memcpy(&data->info, &i2c, sizeof(CTL_SENDRV_I2C));
	data->info.i2c.ch = sen_i2c[id].id;
	data->info.i2c.w_addr_info[0].w_addr = sen_i2c[id].addr;
	data->info.i2c.cur_w_addr_info.w_addr_sel = data->info.i2c.w_addr_info[0].w_addr_sel;
	data->info.i2c.cur_w_addr_info.w_addr = data->info.i2c.w_addr_info[0].w_addr;
	return E_OK;
}

static ER sen_get_attr_if_rtd6122c(CTL_SENDRV_GET_ATTR_IF_PARAM *data)
{
	#if 1
	if (data->type == CTL_SEN_IF_TYPE_TDIO) {
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

static void sen_get_fps_rtd6122c(CTL_SEN_ID id, CTL_SENDRV_GET_FPS_PARAM *data)
{
	data->cur_fps = sen_get_cur_fps_rtd6122c(id);
	data->chg_fps = sen_get_chgmode_fps_rtd6122c(id);
}

static void sen_get_speed_rtd6122c(CTL_SEN_ID id, CTL_SENDRV_GET_SPEED_PARAM *data)
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

static void sen_get_thermal_para_rtd6122c(CTL_SENDRV_GET_THERMAL_PARAM *data)
{
	UINT32 mode = data->mode;

	if (mode >= SEN_MAX_MODE) {
		mode = 0;
	}

	memcpy(data, &thermal_param[mode], sizeof(CTL_SENDRV_GET_THERMAL_PARAM));
}

static void sen_get_modesel_rtd6122c(CTL_SENDRV_GET_MODESEL_PARAM *data)
{
	if (data->if_type != CTL_SEN_IF_TYPE_TDIO) {
		DBG_ERR("if_type %d N.S. \r\n", data->if_type);
		return;
	}
/*
	if (data->data_fmt != CTL_SEN_DATA_FMT_RGB) {
		DBG_ERR("data_fmt %d N.S. \r\n", data->data_fmt);
		return;
	}
*/
	if (data->frame_num == 1) {
		if ((data->size.w <= 678) && (data->size.h <= 522)) {
			if (data->frame_rate <= 2500) {
				data->mode = CTL_SEN_MODE_1;
				return;
			}
		}
	}

	DBG_ERR("fail (frame_rate=%d,size=%d*%d,if_type=%d,data_fmt=%d,frame_num=%d,data_lane=%d,mode_type_sel=%llx,pixdepth=%lld) \r\n"
			, data->frame_rate, data->size.w, data->size.h, data->if_type, data->data_fmt, data->frame_num, data->data_lane, (UINT64)data->mode_type_sel,(UINT64)data->pixdepth);
	data->mode = CTL_SEN_MODE_1;
}


static void sen_get_rowtime_rtd6122c(CTL_SEN_ID id, CTL_SENDRV_GET_MODE_ROWTIME_PARAM *data)
{
	data->row_time_step = 1;
	data->row_time = 0;
}

static void sen_set_cur_fps_rtd6122c(CTL_SEN_ID id, UINT32 fps)
{
	cur_fps[id] = fps;
}

static UINT32 sen_get_cur_fps_rtd6122c(CTL_SEN_ID id)
{
	return cur_fps[id];
}

static void sen_set_chgmode_fps_rtd6122c(CTL_SEN_ID id, UINT32 fps)
{
	chgmode_fps[id] = fps;
}

static UINT32 sen_get_chgmode_fps_rtd6122c(CTL_SEN_ID id)
{
	return chgmode_fps[id];
}

#if defined(__FREERTOS)
void sen_get_i2c_id_rtd6122c(CTL_SEN_ID id, UINT32 *i2c_id)
{
	*i2c_id = sen_i2c[id].id;
}

void sen_get_i2c_addr_rtd6122c(CTL_SEN_ID id, UINT32 *i2c_addr)
{
	*i2c_addr = sen_i2c[id].addr;
}
int sen_init_rtd6122c(SENSOR_DTSI_INFO *info)
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

	sprintf(compatible, "nvt,sen_rtd6122c");
	if (sen_common_check_compatible(compatible)) {
		DBG_DUMP("compatible valid, using peri-dev.dtsi \r\n");
		sen_common_load_cfg_preset_compatible(compatible, &sen_preset);
		sen_common_load_cfg_direction_compatible(compatible, &sen_direction);
		sen_common_load_cfg_power_compatible(compatible, &sen_power);
		sen_common_load_cfg_i2c_compatible(compatible, &sen_i2c);
	} else if (info->addr != NULL) {
		DBG_DUMP("compatible not valid, using sensor.dtsi \r\n");
		sprintf(node_path, "/sensor/sen_cfg/sen_rtd6122c");
		sen_common_load_cfg_map(info->addr, node_path, &sen_map);
		sen_common_load_cfg_preset(info->addr, node_path, &sen_preset);
		sen_common_load_cfg_direction(info->addr, node_path, &sen_direction);
		sen_common_load_cfg_power(info->addr, node_path, &sen_power);
		sen_common_load_cfg_i2c(info->addr, node_path, &sen_i2c);
	} else {
		DBG_WRN("DTSI addr is NULL \r\n");
	}

	memset((void *)(&reg_obj), 0, sizeof(CTL_SEN_REG_OBJ));
	reg_obj.pwr_ctrl = sen_pwr_ctrl_rtd6122c;
	reg_obj.det_plug_in = NULL;
	reg_obj.drv_tab = sen_get_drv_tab_rtd6122c();
	rt = ctl_sen_reg_sendrv("nvt_sen_rtd6122c", &reg_obj);
	if (rt != E_OK) {
		DBG_WRN("register sensor driver fail \r\n");
	}

	return rt;
}

void sen_exit_rtd6122c(void)
{
	ctl_sen_unreg_sendrv("nvt_sen_rtd6122c");
}

#else
static int __init sen_init_rtd6122c(void)
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
	reg_obj.pwr_ctrl = sen_pwr_ctrl_rtd6122c;
	reg_obj.det_plug_in = NULL;
	reg_obj.drv_tab = sen_get_drv_tab_rtd6122c();
	rt = ctl_sen_reg_sendrv("nvt_sen_rtd6122c", &reg_obj);
	if (rt != E_OK) {
		DBG_WRN("register sensor driver fail \r\n");
	}

	return rt;
}

static void __exit sen_exit_rtd6122c(void)
{
	ctl_sen_unreg_sendrv("nvt_sen_rtd6122c");
}

module_init(sen_init_rtd6122c);
module_exit(sen_exit_rtd6122c);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION(SEN_RTD6122C_MODULE_NAME);
MODULE_LICENSE("GPL");
#endif

