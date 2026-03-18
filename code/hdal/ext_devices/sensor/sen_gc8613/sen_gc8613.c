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
VOS_MODULE_VERSION(nvt_sen_gc8613, 1, 11, 000, 00);

//=============================================================================
// information
//=============================================================================
#define SEN_GC8613_MODULE_NAME    "sen_gc8613"
#define SEN_MAX_MODE              3
#define MAX_VD_PERIOD             0x3FFF
#define MAX_EXPOSURE_LINE         0x3FFF
#define MIN_EXPOSURE_LINE         1
#define MIN_HDR_EXPOSURE_LINE     1
#define NON_HDR_EXPOSURE_LINE     8
#define NON_EXPOSURE_LINE         8
#define MAX_SHORT_EXPOSURE_LINE   132 //vts - Win_height - dummy(20) = 2376 - 2184 - 60 = 132
#define SENSOR_ID                 0x8613
#define SEN_I2C_ADDR 0x62 >> 1
#define SEN_I2C_COMPATIBLE "nvt,sen_gc8613"

#include "sen_i2c.c"

//=============================================================================
// function declaration
//=============================================================================
static CTL_SEN_DRV_TAB *sen_get_drv_tab_gc8613(void);
static void sen_pwr_ctrl_gc8613(CTL_SEN_ID id, CTL_SEN_PWR_CTRL_FLAG flag, CTL_SEN_CLK_CB clk_cb);
static ER sen_open_gc8613(CTL_SEN_ID id);
static ER sen_close_gc8613(CTL_SEN_ID id);
static ER sen_sleep_gc8613(CTL_SEN_ID id);
static ER sen_wakeup_gc8613(CTL_SEN_ID id);
static ER sen_write_reg_gc8613(CTL_SEN_ID id, CTL_SEN_CMD *cmd);
static ER sen_read_reg_gc8613(CTL_SEN_ID id, CTL_SEN_CMD *cmd);
static ER sen_chg_mode_gc8613(CTL_SEN_ID id, CTL_SENDRV_CHGMODE_OBJ chgmode_obj);
static ER sen_chg_fps_gc8613(CTL_SEN_ID id, UINT32 fps);
static ER sen_set_info_gc8613(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data);
static ER sen_get_info_gc8613(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data);
static UINT32 sen_calc_chgmode_vd_gc8613(CTL_SEN_ID id, UINT32 fps);
static UINT32 sen_calc_exp_vd_gc8613(CTL_SEN_ID id, UINT32 fps);
static void sen_set_gain_gc8613(CTL_SEN_ID id, void *param);
static void sen_set_expt_gc8613(CTL_SEN_ID id, void *param);
static void sen_set_preset_gc8613(CTL_SEN_ID id, ISP_SENSOR_PRESET_CTRL *ctrl);
static void sen_set_flip_gc8613(CTL_SEN_ID id, CTL_SEN_FLIP *flip);
static ER sen_get_flip_gc8613(CTL_SEN_ID id, CTL_SEN_FLIP *flip);
#if defined(__FREERTOS)
void sen_get_gain_gc8613(CTL_SEN_ID id, void *param);
void sen_get_expt_gc8613(CTL_SEN_ID id, void *param);
#else
static void sen_get_gain_gc8613(CTL_SEN_ID id, void *param);
static void sen_get_expt_gc8613(CTL_SEN_ID id, void *param);
#endif
static void sen_get_min_expt_gc8613(CTL_SEN_ID id, void *param);
static void sen_get_mode_basic_gc8613(CTL_SENDRV_GET_MODE_BASIC_PARAM *mode_basic);
static void sen_get_attr_basic_gc8613(CTL_SENDRV_GET_ATTR_BASIC_PARAM *data);
static void sen_get_attr_signal_gc8613(CTL_SENDRV_GET_ATTR_SIGNAL_PARAM *data);
static ER sen_get_attr_cmdif_gc8613(CTL_SEN_ID id, CTL_SENDRV_GET_ATTR_CMDIF_PARAM *data);
static ER sen_get_attr_if_gc8613(CTL_SENDRV_GET_ATTR_IF_PARAM *data);
static void sen_get_fps_gc8613(CTL_SEN_ID id, CTL_SENDRV_GET_FPS_PARAM *data);
static void sen_get_speed_gc8613(CTL_SEN_ID id, CTL_SENDRV_GET_SPEED_PARAM *data);
static void sen_get_mode_mipi_gc8613(CTL_SENDRV_GET_MODE_MIPI_PARAM *data);
static void sen_get_modesel_gc8613(CTL_SENDRV_GET_MODESEL_PARAM *data);
static UINT32 sen_calc_rowtime_gc8613(CTL_SEN_ID id, CTL_SEN_MODE mode);
static void sen_get_rowtime_gc8613(CTL_SEN_ID id, CTL_SENDRV_GET_MODE_ROWTIME_PARAM *data);
static void sen_set_cur_fps_gc8613(CTL_SEN_ID id, UINT32 fps);
static UINT32 sen_get_cur_fps_gc8613(CTL_SEN_ID id);
static void sen_set_chgmode_fps_gc8613(CTL_SEN_ID id, UINT32 fps);
static UINT32 sen_get_chgmode_fps_gc8613(CTL_SEN_ID id);
static void sen_ext_get_probe_sen_gc8613(CTL_SEN_ID id, CTL_SENDRV_GET_PROBE_SEN_PARAM *data);
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
	SEN_GC8613_MODULE_NAME,
	CTL_SEN_VENDOR_OTHERS,
	SEN_MAX_MODE,
	CTL_SEN_SUPPORT_PROPERTY_MIRROR|CTL_SEN_SUPPORT_PROPERTY_FLIP|CTL_SEN_SUPPORT_PROPERTY_CHGFPS,
	0
};

static CTL_SENDRV_GET_ATTR_SIGNAL_PARAM signal_param = {
	CTL_SEN_SIGNAL_MASTER,
	{CTL_SEN_ACTIVE_HIGH, CTL_SEN_ACTIVE_HIGH, CTL_SEN_PHASE_RISING, CTL_SEN_PHASE_RISING, CTL_SEN_PHASE_RISING}
};

static CTL_SENDRV_I2C i2c = {
	{
		{CTL_SEN_I2C_W_ADDR_DFT,     0x62},
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
		27000000,
		324300000,
		158400000, //MIPI_data_rate*lane_number/bit_number/2
	},
	{
		CTL_SEN_MODE_2,
		CTL_SEN_SIEMCLK_SRC_DFT,
		24000000,
		348000000,
		300000000, //MIPI_data_rate*lane_number/bit_number/2
	},
	{
		CTL_SEN_MODE_3,
		CTL_SEN_SIEMCLK_SRC_DFT,
		24000000,
		337500000,
		170000000, //MIPI_data_rate*lane_number/bit_number/2
	}
};

static CTL_SENDRV_GET_MODE_MIPI_PARAM mipi_param[SEN_MAX_MODE] = {
	{
		CTL_SEN_MODE_1,
		CTL_SEN_CLKLANE_1,
		CTL_SEN_DATALANE_4,
		{{CTL_SEN_MIPI_MANUAL_NONE, 0}, {CTL_SEN_MIPI_MANUAL_NONE, 0}, {CTL_SEN_MIPI_MANUAL_NONE, 0}},
		0,
		{0, 0, 0, 0},
		SEN_BIT_OFS_NONE
	},
	{
		CTL_SEN_MODE_2,
		CTL_SEN_CLKLANE_1,
		CTL_SEN_DATALANE_4,
		{ {CTL_SEN_MIPI_MANUAL_NONE, 0}, {CTL_SEN_MIPI_MANUAL_NONE, 0}, {CTL_SEN_MIPI_MANUAL_NONE, 0} },
		0,
		{1, 0, 0, 0},
		SEN_BIT_OFS_0|SEN_BIT_OFS_1
	},
	{
		CTL_SEN_MODE_3,
		CTL_SEN_CLKLANE_1,
		CTL_SEN_DATALANE_4,
		{ {CTL_SEN_MIPI_MANUAL_NONE, 0}, {CTL_SEN_MIPI_MANUAL_NONE, 0}, {CTL_SEN_MIPI_MANUAL_NONE, 0} },
		0,
		{0, 0, 0, 0},
		SEN_BIT_OFS_NONE
	}
};

static CTL_SENDRV_GET_MODE_BASIC_PARAM mode_basic_param[SEN_MAX_MODE] = {
	{
		CTL_SEN_MODE_1,
		CTL_SEN_IF_TYPE_MIPI,
		CTL_SEN_DATA_FMT_RGB,
		CTL_SEN_MODE_LINEAR,
		3000,
		1,
		CTL_SEN_STPIX_R,
		CTL_SEN_PIXDEPTH_10BIT,
		CTL_SEN_FMT_POGRESSIVE,
		{3848, 2168},
		{{0, 2, 3840, 2160}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
		{3840, 2160},
		{0, 4800, 0,2250},
		CTL_SEN_RATIO(16, 9),
		{1000, 1063376},
		100
	},
	{
		CTL_SEN_MODE_2,
		CTL_SEN_IF_TYPE_MIPI,
		CTL_SEN_DATA_FMT_RGB,
		CTL_SEN_MODE_STAGGER_HDR,
		3000,
		2,
		CTL_SEN_STPIX_R,
		CTL_SEN_PIXDEPTH_10BIT,
		CTL_SEN_FMT_POGRESSIVE,
		{3840, 2168},
		{{0, 4, 3840, 2160}, {0, 4, 3840, 2160}, {0, 0, 0, 0}, {0, 0, 0, 0}},
		{3840, 2160},
		{0, 4880, 0, 2376},
		CTL_SEN_RATIO(16, 9),
		{1000, 1063376},
		100
	},
	{
		CTL_SEN_MODE_3,
		CTL_SEN_IF_TYPE_MIPI,
		CTL_SEN_DATA_FMT_RGB,
		CTL_SEN_MODE_LINEAR,
		3000,
		1,
		CTL_SEN_STPIX_R,
		CTL_SEN_PIXDEPTH_14BIT,
		CTL_SEN_FMT_POGRESSIVE,
		{3840, 2168},
		{{0, 4, 3840, 2160}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
		{3840, 2160},
		{0, 4900, 0, 2296},
		CTL_SEN_RATIO(16, 9),
		{1000, 1063376},
		100
	}
};

static CTL_SEN_CMD gc8613_mode_1[] = {
//version 1.3
//mclk 27Mhz,mipi 4lane,raw10
//mipi 792Mbps/lane
//vts = 2250,frmae rate= 30fps
//window 3848 2168
//row_time 14.8us
	{0x03fe, 1, {0xf0, 0x00}},
	{0x03fe, 1, {0x00, 0x00}},
	{0x03fe, 1, {0x10, 0x00}},
	{0x0a38, 1, {0x01, 0x00}},
	{0x0a20, 1, {0x19, 0x00}},
	{0x061b, 1, {0x17, 0x00}},
	{0x061c, 1, {0x50, 0x00}},
	{0x061d, 1, {0x06, 0x00}},
	{0x061e, 1, {0x78, 0x00}},
	{0x061f, 1, {0x05, 0x00}},
	{0x0a21, 1, {0x10, 0x00}},
	{0x0a31, 1, {0xb0, 0x00}},
	{0x0a34, 1, {0x40, 0x00}},
	{0x0a35, 1, {0x08, 0x00}},
	{0x0a37, 1, {0x46, 0x00}},
	{0x0314, 1, {0x50, 0x00}},
	{0x0315, 1, {0x00, 0x00}},
	{0x031c, 1, {0xce, 0x00}},
	{0x0219, 1, {0x47, 0x00}},
	{0x0342, 1, {0x03, 0x00}},
	{0x0343, 1, {0x20, 0x00}},
	{0x0259, 1, {0x08, 0x00}},
	{0x025a, 1, {0x96, 0x00}},
	{0x0340, 1, {0x08, 0x00}},
	{0x0341, 1, {0xca, 0x00}},
	{0x0351, 1, {0x00, 0x00}},
	{0x0345, 1, {0x02, 0x00}},
	{0x0347, 1, {0x02, 0x00}},
	{0x0348, 1, {0x0f, 0x00}},
	{0x0349, 1, {0x18, 0x00}},
	{0x034a, 1, {0x08, 0x00}},
	{0x034b, 1, {0x88, 0x00}},
	{0x034f, 1, {0xf0, 0x00}},
	{0x0094, 1, {0x0f, 0x00}},
	{0x0095, 1, {0x08, 0x00}},
	{0x0096, 1, {0x08, 0x00}},
	{0x0097, 1, {0x78, 0x00}}, // out Y 2168
	{0x0099, 1, {0x0c, 0x00}},
	{0x009b, 1, {0x0c, 0x00}},
	{0x060c, 1, {0x06, 0x00}},
	{0x060e, 1, {0x20, 0x00}},
	{0x060f, 1, {0x0f, 0x00}},
	{0x070c, 1, {0x06, 0x00}},
	{0x070e, 1, {0x20, 0x00}},
	{0x070f, 1, {0x0f, 0x00}},
	{0x0087, 1, {0x50, 0x00}},
	{0x0907, 1, {0xd5, 0x00}},
	{0x0909, 1, {0x06, 0x00}},
	{0x0902, 1, {0x0b, 0x00}},
	{0x0904, 1, {0x08, 0x00}},
	{0x0908, 1, {0x09, 0x00}},
	{0x0903, 1, {0xc5, 0x00}},
	{0x090c, 1, {0x09, 0x00}},
	{0x0905, 1, {0x10, 0x00}},
	{0x0906, 1, {0x00, 0x00}},
	{0x072a, 1, {0x7c, 0x00}},
	{0x0724, 1, {0x2b, 0x00}},
	{0x0727, 1, {0x2b, 0x00}},
	{0x072b, 1, {0x1c, 0x00}},
	{0x073e, 1, {0x40, 0x00}},
	{0x0078, 1, {0x88, 0x00}},
	{0x0618, 1, {0x01, 0x00}},
	{0x1466, 1, {0x12, 0x00}},
	{0x1468, 1, {0x10, 0x00}},
	{0x1467, 1, {0x10, 0x00}},
	{0x0709, 1, {0x40, 0x00}},
	{0x0719, 1, {0x40, 0x00}},
	{0x1469, 1, {0x80, 0x00}},
	{0x146a, 1, {0xc0, 0x00}},
	{0x146b, 1, {0x03, 0x00}},
	{0x1480, 1, {0x02, 0x00}},
	{0x1481, 1, {0x80, 0x00}},
	{0x1484, 1, {0x08, 0x00}},
	{0x1485, 1, {0xc0, 0x00}},
	{0x1430, 1, {0x80, 0x00}},
	{0x1407, 1, {0x10, 0x00}},
	{0x1408, 1, {0x16, 0x00}},
	{0x1409, 1, {0x03, 0x00}},
	{0x1434, 1, {0x04, 0x00}},
	{0x1447, 1, {0x75, 0x00}},
	{0x1470, 1, {0x10, 0x00}},
	{0x1471, 1, {0x13, 0x00}},
	{0x1438, 1, {0x00, 0x00}},
	{0x143a, 1, {0x00, 0x00}},
	{0x024b, 1, {0x02, 0x00}},
	{0x0245, 1, {0xc7, 0x00}},
	{0x025b, 1, {0x07, 0x00}},
	{0x02bb, 1, {0x77, 0x00}},
	{0x0612, 1, {0x01, 0x00}},
	{0x0613, 1, {0x26, 0x00}},
	{0x0243, 1, {0x66, 0x00}},
	{0x0087, 1, {0x53, 0x00}},
	{0x0053, 1, {0x05, 0x00}},
	{0x0063, 1, {0x00, 0x00}},//
	{0x0089, 1, {0x02, 0x00}},
	{0x0002, 1, {0xeb, 0x00}},
	{0x005a, 1, {0x0c, 0x00}},
	{0x0040, 1, {0x83, 0x00}},
	{0x0075, 1, {0x54, 0x00}},
	{0x0205, 1, {0x0c, 0x00}},
	{0x0202, 1, {0x01, 0x00}},
	{0x0203, 1, {0x27, 0x00}},
	{0x061a, 1, {0x02, 0x00}},
	{0x03fe, 1, {0x00, 0x00}},
	{0x0106, 1, {0x78, 0x00}},
	{0x0136, 1, {0x03, 0x00}},
	{0x0181, 1, {0xf0, 0x00}},
	{0x0185, 1, {0x01, 0x00}},
	{0x0180, 1, {0x46, 0x00}},
	{0x0106, 1, {0x38, 0x00}},
	{0x010d, 1, {0xca, 0x00}},
	{0x010e, 1, {0x12, 0x00}},
	{0x0113, 1, {0x02, 0x00}},
	{0x0114, 1, {0x03, 0x00}},
	{0x0115, 1 ,{0x12, 0x00}},
	{0x0004, 1 ,{0x0f, 0x00}},
	{0x0219, 1 ,{0x47, 0x00}},
	{0x0054, 1 ,{0x98, 0x00}},
	{0x0076, 1 ,{0x01, 0x00}},
	{0x0052, 1 ,{0x02, 0x00}},
	{0x021a, 1 ,{0x10, 0x00}},
	{0x0430, 1 ,{0x21, 0x00}},
	{0x0431, 1 ,{0x21, 0x00}},
	{0x0432, 1 ,{0x21, 0x00}},
	{0x0433, 1 ,{0x21, 0x00}},
	{0x0434, 1 ,{0x61, 0x00}},
	{0x0435, 1 ,{0x61, 0x00}},
	{0x0436, 1 ,{0x61, 0x00}},
	{0x0437, 1 ,{0x61, 0x00}},
	{0x0704, 1 ,{0x07, 0x00}},
	{0x0706, 1 ,{0x02, 0x00}},
	{0x0716, 1 ,{0x02, 0x00}},
	{0x0708, 1 ,{0xc8, 0x00}},
	{0x0718, 1 ,{0xc8, 0x00}},
	//otp autoload
	{0x031f, 1 ,{0x01, 0x00}},
	{0x031f, 1 ,{0x00, 0x00}},
	{0x0a67, 1 ,{0x80, 0x00}},
	{0x0a54, 1 ,{0x0e, 0x00}},
	{0x0a65, 1 ,{0x10, 0x00}},
	{0x0a98, 1 ,{0x04, 0x00}},
	{0x05be, 1 ,{0x00, 0x00}},
	{0x05a9, 1 ,{0x01, 0x00}},
	{0x0089, 1 ,{0x02, 0x00}},
	{0x0aa0, 1 ,{0x00, 0x00}},
	{0x0023, 1 ,{0x00, 0x00}},
	{0x0022, 1 ,{0x00, 0x00}},
	{0x0025, 1 ,{0x00, 0x00}},
	{0x0024, 1 ,{0x00, 0x00}},
	{0x0028, 1 ,{0x0f, 0x00}},
	{0x0029, 1 ,{0x18, 0x00}},
	{0x002a, 1 ,{0x08, 0x00}},
	{0x002b, 1 ,{0x88, 0x00}},
	{0x0317, 1 ,{0x1c, 0x00}},
	{0x0a70, 1 ,{0x03, 0x00}},
	{0x0a82, 1 ,{0x00, 0x00}},
	{0x0a83, 1 ,{0xe0, 0x00}},
	{0x0a71, 1 ,{0x00, 0x00}},
	{0x0a72, 1 ,{0x02, 0x00}},
	{0x0a73, 1 ,{0x60, 0x00}},
	{0x0a75, 1 ,{0x41, 0x00}},
	{0x0a70, 1 ,{0x03, 0x00}},
	{0x0a5a, 1 ,{0x80, 0x00}},
	{SEN_CMD_DELAY, 1, {20, 0x0}},
	{0x0089, 1 ,{0x02, 0x00}},
	{0x05be, 1 ,{0x01, 0x00}},
	{0x0a70, 1 ,{0x00, 0x00}},
	{0x0080, 1 ,{0x02, 0x00}},
	{0x0a67, 1 ,{0x00, 0x00}},
	{SEN_CMD_SETVD, 1, {0x00, 0x0}},
	{SEN_CMD_PRESET, 1, {0x00, 0x0}},
	{SEN_CMD_DIRECTION, 1, {0x00, 0x0}},
	{0x0100, 1, {0x09, 0x00}},
};

static CTL_SEN_CMD gc8613_mode_2[] = {
	//version 1.4
	//mclk 24Mhz,mipi 4lane,raw10
	//mipi 1500Mbps/lane
	//vts = 2376,frmae rate= 30fps@hdr
	//window 3840 2160
	//row_time 14.02us
	{0x03fe,1,{0xf0,0x0}},
	{0x03fe,1,{0x00,0x0}},
	{0x03fe,1,{0x10,0x0}},
	{0x0a38,1,{0x01,0x0}},
	{0x0a20,1,{0x19,0x0}},
	{0x061b,1,{0x17,0x0}},
	{0x061c,1,{0x44,0x0}},
	{0x061d,1,{0x05,0x0}},
	{0x061e,1,{0x91,0x0}},
	{0x061f,1,{0x05,0x0}},
	{0x0a21,1,{0x10,0x0}},
	{0x0a30,1,{0x00,0x0}},
	{0x0a31,1,{0xfa,0x0}},
	{0x0a34,1,{0x40,0x0}},
	{0x0a35,1,{0x08,0x0}},
	{0x0a37,1,{0x44,0x0}},
	{0x0314,1,{0x50,0x0}},
	{0x031c,1,{0xce,0x0}},
	{0x0219,1,{0x47,0x0}},
	{0x0342,1,{0x02,0x0}},
	{0x0343,1,{0x62,0x0}},
	{0x0259,1,{0x08,0x0}},
	{0x025a,1,{0x96,0x0}},
	{0x0340,1,{0x09,0x0}},
	{0x0341,1,{0x48,0x0}},
	{0x0351,1,{0x00,0x0}},
	{0x0345,1,{0x02,0x0}},
	{0x0347,1,{0x02,0x0}},
	{0x0348,1,{0x0f,0x0}},
	{0x0349,1,{0x18,0x0}},
	{0x034a,1,{0x08,0x0}},
	{0x034b,1,{0x88,0x0}},
	{0x034f,1,{0xf0,0x0}},
	{0x0094,1,{0x0f,0x0}},
	{0x0095,1,{0x00,0x0}},
	{0x0096,1,{0x08,0x0}},
	{0x0097,1,{0x78,0x0}}, // out Y 2168
	{0x0099,1,{0x0c,0x0}},
	{0x009b,1,{0x0c,0x0}},
	{0x060c,1,{0x06,0x0}},
	{0x060e,1,{0x20,0x0}},
	{0x060f,1,{0x0f,0x0}},
	{0x070c,1,{0x06,0x0}},
	{0x070e,1,{0x20,0x0}},
	{0x070f,1,{0x0f,0x0}},
	{0x0087,1,{0x50,0x0}},
	{0x0907,1,{0xd5,0x0}},
	{0x0909,1,{0x06,0x0}},
	{0x0902,1,{0x0b,0x0}},
	{0x0904,1,{0x08,0x0}},
	{0x0908,1,{0x09,0x0}},
	{0x0903,1,{0xc5,0x0}},
	{0x090c,1,{0x09,0x0}},
	{0x0905,1,{0x10,0x0}},
	{0x0906,1,{0x00,0x0}},
	{0x0724,1,{0x2b,0x0}},
	{0x0727,1,{0x2b,0x0}},
	{0x072b,1,{0x1c,0x0}},
	{0x072a,1,{0x7c,0x0}},
	{0x073e,1,{0x40,0x0}},
	{0x0078,1,{0x88,0x0}},
	{0x0618,1,{0x01,0x0}},
	{0x1466,1,{0x12,0x0}},
	{0x1468,1,{0x10,0x0}},
	{0x1467,1,{0x10,0x0}},
	{0x1478,1,{0x10,0x0}},
	{0x1477,1,{0x10,0x0}},
	{0x0709,1,{0x40,0x0}},
	{0x0719,1,{0x40,0x0}},
	{0x1469,1,{0x80,0x0}},
	{0x146a,1,{0x20,0x0}},
	{0x146b,1,{0x03,0x0}},
	{0x1479,1,{0x80,0x0}},
	{0x147a,1,{0x20,0x0}},
	{0x147b,1,{0x03,0x0}},
	{0x1480,1,{0x02,0x0}},
	{0x1481,1,{0x80,0x0}},
	{0x1482,1,{0x02,0x0}},
	{0x1483,1,{0x80,0x0}},
	{0x1484,1,{0x08,0x0}},
	{0x1485,1,{0xc0,0x0}},
	{0x1486,1,{0x08,0x0}},
	{0x1487,1,{0xc0,0x0}},
	{0x1407,1,{0x10,0x0}},
	{0x1408,1,{0x16,0x0}},
	{0x1409,1,{0x03,0x0}},
	{0x1434,1,{0x04,0x0}},
	{0x1447,1,{0x75,0x0}},
	{0x140d,1,{0x04,0x0}},
	{0x1461,1,{0x10,0x0}},
	{0x146c,1,{0x10,0x0}},
	{0x146d,1,{0x10,0x0}},
	{0x146e,1,{0x2e,0x0}},
	{0x146f,1,{0x30,0x0}},
	{0x1474,1,{0x34,0x0}},
	{0x1470,1,{0x10,0x0}},
	{0x1471,1,{0x13,0x0}},
	{0x143a,1,{0x00,0x0}},
	{0x024b,1,{0x02,0x0}},
	{0x0245,1,{0xc7,0x0}},
	{0x025b,1,{0x07,0x0}},
	{0x02bb,1,{0x77,0x0}},
	{0x0612,1,{0x01,0x0}},
	{0x0613,1,{0x26,0x0}},
	{0x0243,1,{0x66,0x0}},
	{0x0087,1,{0x53,0x0}},
	{0x0053,1,{0x05,0x0}},
	{0x0089,1,{0x02,0x0}},
	{0x0002,1,{0xeb,0x0}},
	{0x005a,1,{0x0c,0x0}},
	{0x0040,1,{0x83,0x0}},
	{0x0075,1,{0x58,0x0}},
	{0x0205,1,{0x0c,0x0}},
	{0x0202,1,{0x06,0x0}},
	{0x0203,1,{0x27,0x0}},
	{0x061a,1,{0x02,0x0}},
	{0x0122,1,{0x12,0x0}},
	{0x0123,1,{0x50,0x0}},
	{0x0126,1,{0x0f,0x0}},
	{0x0129,1,{0x10,0x0}},
	{0x012a,1,{0x20,0x0}},
	{0x012b,1,{0x10,0x0}},
	{0x03fe,1,{0x00,0x0}},
	{0x0106,1,{0x78,0x0}},
	{0x0136,1,{0x03,0x0}},
	{0x0181,1,{0xf0,0x0}},
	{0x0185,1,{0x01,0x0}},
	{0x0180,1,{0x46,0x0}},
	{0x0106,1,{0x38,0x0}},
	{0x010d,1,{0xc0,0x0}},
	{0x010e,1,{0x12,0x0}},
	{0x0113,1,{0x02,0x0}},
	{0x0114,1,{0x03,0x0}},
	{0x0220,1,{0xcf,0x0}},
	{0x0222,1,{0x49,0x0}},
	{0x0107,1,{0x89,0x0}},
	{0x0619,1,{0x01,0x0}},
	{0x023b,1,{0x58,0x0}},
	{0x023e,1,{0x00,0x0}},
	{0x023f,1,{0x84,0x0}},
	{0x0220,1,{0xcf,0x0}},
	{0x021b,1,{0x69,0x0}},
	{0x0004,1,{0x0f,0x0}},
	{0x000e,1,{0x07,0x0}},
	{0x0219,1,{0x47,0x0}},
	{0x0054,1,{0x98,0x0}},
	{0x0076,1,{0x01,0x0}},
	{0x0052,1,{0x02,0x0}},
	{0x021a,1,{0x10,0x0}},
	{0x0430,1,{0x21,0x0}},
	{0x0431,1,{0x21,0x0}},
	{0x0432,1,{0x21,0x0}},
	{0x0433,1,{0x21,0x0}},
	{0x0434,1,{0x61,0x0}},
	{0x0435,1,{0x61,0x0}},
	{0x0436,1,{0x61,0x0}},
	{0x0437,1,{0x61,0x0}},
	{0x0438,1,{0x21,0x0}},
	{0x0439,1,{0x21,0x0}},
	{0x043a,1,{0x21,0x0}},
	{0x043b,1,{0x21,0x0}},
	{0x043c,1,{0x61,0x0}},
	{0x043d,1,{0x61,0x0}},
	{0x043e,1,{0x61,0x0}},
	{0x043f,1,{0x61,0x0}},
	{0x0704,1,{0x07,0x0}},
	{0x0706,1,{0x02,0x0}},
	{0x0716,1,{0x02,0x0}},
	{0x0708,1,{0xc8,0x0}},
	{0x0718,1,{0xc8,0x0}},
	{0x031f,1,{0x01,0x0}},
	{0x031f,1,{0x00,0x0}},
	{0x0a67,1,{0x80,0x0}},
	{0x0a54,1,{0x0e,0x0}},
	{0x0a65,1,{0x10,0x0}},
	{0x0a98,1,{0x04,0x0}},
	{0x05be,1,{0x00,0x0}},
	{0x05a9,1,{0x01,0x0}},
	{0x0089,1,{0x02,0x0}},
	{0x0aa0,1,{0x00,0x0}},
	{0x0023,1,{0x00,0x0}},
	{0x0022,1,{0x00,0x0}},
	{0x0025,1,{0x00,0x0}},
	{0x0024,1,{0x00,0x0}},
	{0x0028,1,{0x0f,0x0}},
	{0x0029,1,{0x18,0x0}},
	{0x002a,1,{0x08,0x0}},
	{0x002b,1,{0x88,0x0}},
	{0x0317,1,{0x1c,0x0}},
	{0x0a70,1,{0x03,0x0}},
	{0x0a82,1,{0x00,0x0}},
	{0x0a83,1,{0xe0,0x0}},
	{0x0a71,1,{0x00,0x0}},
	{0x0a72,1,{0x02,0x0}},
	{0x0a73,1,{0x60,0x0}},
	{0x0a75,1,{0x41,0x0}},
	{0x0a70,1,{0x03,0x0}},
	{0x0a5a,1,{0x80,0x0}},
	{SEN_CMD_DELAY, 1, {20, 0x0}},
	{0x0089,1,{0x02,0x0}},
	{0x05be,1,{0x01,0x0}},
	{0x0a70,1,{0x00,0x0}},
	{0x0080,1,{0x02,0x0}},
	{0x0a67,1,{0x00,0x0}},
	{0x0020,1,{0x01,0x0}},
	{0x0115,1,{0x12,0x0}}, 	
	{SEN_CMD_SETVD, 1, {0x00, 0x0}},
	{SEN_CMD_PRESET, 1, {0x00, 0x0}},
	{SEN_CMD_DIRECTION, 1, {0x00, 0x0}},
	{0x0100,1,{0x09,0x0}}, 
};

static CTL_SEN_CMD gc8613_mode_3[] = {
	//version 1.6 
	//<MODE_18 type="MODE_18_GC8613_3840*2160_30fps_raw14_4lane_DAG_1001_YN002">
	//mclk 24MHz, mipiclk 1188Mbps, wpclk 216MHz, rpclk 172.8MHz
	//rowtime 14.52us, vts 2296
	//darksun on, HDR off, fixposition off, DAG on
	{0x03fe, 1, {0xf0, 0x00}},
	{0x03fe, 1, {0x00, 0x00}},
	{0x03fe, 1, {0x10, 0x00}},
	{0x0a38, 1, {0x01, 0x00}},
	{0x0a20, 1, {0x19, 0x00}},
	{0x061b, 1, {0x17, 0x00}},
	{0x061c, 1, {0x48, 0x00}},
	{0x061d, 1, {0x05, 0x00}},
	{0x061e, 1, {0x5a, 0x00}},
	{0x061f, 1, {0x05, 0x00}},
	{0x0a21, 1, {0x10, 0x00}},
	{0x0a31, 1, {0xc6, 0x00}},
	{0x0a34, 1, {0x40, 0x00}},
	{0x0a35, 1, {0x08, 0x00}},
	{0x0a37, 1, {0x44, 0x00}},
	{0x0314, 1, {0x70, 0x00}},
	{0x0315, 1, {0x00, 0x00}},
	{0x031c, 1, {0xce, 0x00}},
	{0x0219, 1, {0x47, 0x00}},
	{0x0342, 1, {0x03, 0x00}},
	{0x0343, 1, {0x10, 0x00}},
	{0x0259, 1, {0x08, 0x00}},
	{0x025a, 1, {0x98, 0x00}},
	{0x0340, 1, {0x08, 0x00}},
	{0x0341, 1, {0xf8, 0x00}},
	{0x0345, 1, {0x02, 0x00}},
	{0x0347, 1, {0x02, 0x00}},
	{0x0348, 1, {0x0f, 0x00}},
	{0x0349, 1, {0x18, 0x00}},
	{0x034a, 1, {0x08, 0x00}},
	{0x034b, 1, {0x88, 0x00}},
	{0x034f, 1, {0xf0, 0x00}},
	{0x0094, 1, {0x0f, 0x00}},
	{0x0095, 1, {0x00, 0x00}},
	{0x0096, 1, {0x08, 0x00}},
	{0x0097, 1, {0x78, 0x00}}, // out Y 2168
	{0x0099, 1, {0x0c, 0x00}},
	{0x009b, 1, {0x0c, 0x00}},
	{0x060c, 1, {0x06, 0x00}},
	{0x060e, 1, {0x20, 0x00}},
	{0x060f, 1, {0x0f, 0x00}},
	{0x070c, 1, {0x06, 0x00}},
	{0x070e, 1, {0x20, 0x00}},
	{0x070f, 1, {0x0f, 0x00}},
	{0x0087, 1, {0x50, 0x00}},
	{0x0907, 1, {0xd5, 0x00}},
	{0x0909, 1, {0x06, 0x00}},
	{0x0902, 1, {0x0b, 0x00}},
	{0x0904, 1, {0x08, 0x00}},
	{0x0908, 1, {0x09, 0x00}},
	{0x0903, 1, {0xc5, 0x00}},
	{0x090c, 1, {0x09, 0x00}},
	{0x0905, 1, {0x10, 0x00}},
	{0x0906, 1, {0x00, 0x00}},
	{0x0724, 1, {0x2b, 0x00}},
	{0x0727, 1, {0x2b, 0x00}},
	{0x072b, 1, {0x1c, 0x00}},
	{0x072a, 1, {0x7c, 0x00}},
	{0x073e, 1, {0x40, 0x00}},
	{0x0078, 1, {0x88, 0x00}},
	{0x0268, 1, {0x40, 0x00}},
	{0x0269, 1, {0x44, 0x00}},
	{0x0351, 1, {0x54, 0x00}},
	{0x0618, 1, {0x01, 0x00}},
	{0x1466, 1, {0x45, 0x00}},
	{0x1468, 1, {0x46, 0x00}},
	{0x1467, 1, {0x46, 0x00}},
	{0x0709, 1, {0x40, 0x00}},
	{0x0719, 1, {0x40, 0x00}},
	{0x1469, 1, {0xf0, 0x00}},
	{0x146a, 1, {0xd0, 0x00}},
	{0x146b, 1, {0x03, 0x00}},
	{0x1480, 1, {0x07, 0x00}},
	{0x1481, 1, {0x80, 0x00}},
	{0x1484, 1, {0x0b, 0x00}},
	{0x1485, 1, {0xc0, 0x00}},
	{0x1430, 1, {0x80, 0x00}},
	{0x1407, 1, {0x10, 0x00}},
	{0x1408, 1, {0x16, 0x00}},
	{0x1409, 1, {0x03, 0x00}},
	{0x1434, 1, {0x04, 0x00}},
	{0x1447, 1, {0x75, 0x00}},
	{0x1470, 1, {0x10, 0x00}},
	{0x1471, 1, {0x13, 0x00}},
	{0x0122, 1, {0x0b, 0x00}},
	{0x0123, 1, {0x30, 0x00}},
	{0x0124, 1, {0x04, 0x00}},
	{0x0125, 1, {0x30, 0x00}},
	{0x0126, 1, {0x0f, 0x00}},
	{0x0127, 1, {0x15, 0x00}},
	{0x0128, 1, {0xa8, 0x00}},
	{0x0129, 1, {0x0c, 0x00}},
	{0x012a, 1, {0x18, 0x00}},
	{0x012b, 1, {0x18, 0x00}},
	{0x1438, 1, {0x00, 0x00}},
	{0x143a, 1, {0x00, 0x00}},
	{0x024b, 1, {0x02, 0x00}},
	{0x0245, 1, {0xc7, 0x00}},
	{0x025b, 1, {0x07, 0x00}},
	{0x02bb, 1, {0x77, 0x00}},
	{0x0612, 1, {0x01, 0x00}},
	{0x0613, 1, {0x24, 0x00}},
	{0x0243, 1, {0x66, 0x00}},
	{0x0087, 1, {0x53, 0x00}},
	{0x0053, 1, {0x05, 0x00}},
	{0x0089, 1, {0x00, 0x00}},
	{0x0002, 1, {0xeb, 0x00}},
	{0x005a, 1, {0x0c, 0x00}},
	{0x0040, 1, {0x83, 0x00}},
	{0x0075, 1, {0x68, 0x00}},
	{0x0205, 1, {0x0c, 0x00}},
	{0x0202, 1, {0x03, 0x00}},
	{0x0203, 1, {0x27, 0x00}},
	{0x061a, 1, {0x02, 0x00}},
	{0x0213, 1, {0x64, 0x00}},
	{0x0265, 1, {0x01, 0x00}},
	{0x0618, 1, {0x05, 0x00}},
	{0x026e, 1, {0x74, 0x00}},
	{0x0270, 1, {0x02, 0x00}},
	{0x0709, 1, {0x00, 0x00}},
	{0x0719, 1, {0x00, 0x00}},
	{0x0812, 1, {0xdb, 0x00}},
	{0x0822, 1, {0x0f, 0x00}},
	{0x0821, 1, {0x18, 0x00}},
	{0x0002, 1, {0xef, 0x00}},
	{0x0813, 1, {0xfb, 0x00}},
	{0x0070, 1, {0x88, 0x00}},
	{0x03fe, 1, {0x00, 0x00}},
	{0x0106, 1, {0x78, 0x00}},
	{0x0136, 1, {0x00, 0x00}},
	{0x0181, 1, {0xf0, 0x00}},
	{0x0185, 1, {0x01, 0x00}},
	{0x0180, 1, {0x46, 0x00}},
	{0x0106, 1, {0x38, 0x00}},
	{0x010d, 1, {0x40, 0x00}},
	{0x010e, 1, {0x1a, 0x00}},
	{0x0111, 1, {0x2d, 0x00}},
	{0x0112, 1, {0x03, 0x00}},
	{0x0114, 1, {0x03, 0x00}},
	//{0x0100, 1, {0x09, 0x00}},
	{0x79cf, 1, {0x01, 0x00}},
	{0x0219, 1, {0x47, 0x00}},
	{0x0054, 1, {0x98, 0x00}},
	{0x0076, 1, {0x01, 0x00}},
	{0x0052, 1, {0x02, 0x00}},
	{0x021a, 1, {0x10, 0x00}},
	{0x0430, 1, {0x05, 0x00}},
	{0x0431, 1, {0x05, 0x00}},
	{0x0432, 1, {0x05, 0x00}},
	{0x0433, 1, {0x05, 0x00}},
	{0x0434, 1, {0x70, 0x00}},
	{0x0435, 1, {0x70, 0x00}},
	{0x0436, 1, {0x70, 0x00}},
	{0x0437, 1, {0x70, 0x00}},
	{0x0004, 1, {0x0f, 0x00}},
	{0x0704, 1, {0x03, 0x00}},
	{0x071d, 1, {0xdc, 0x00}},
	{0x071e, 1, {0x05, 0x00}},
	{0x0706, 1, {0x02, 0x00}},
	{0x0716, 1, {0x02, 0x00}},
	{0x0708, 1, {0xc8, 0x00}},
	{0x0718, 1, {0xc8, 0x00}},
	{0x071d, 1, {0xdc, 0x00}},
	{0x071e, 1, {0x05, 0x00}},
	{0x1469, 1, {0x80, 0x00}},
	//otp autoload
	{0x031f, 1, {0x01, 0x00}},
	{0x031f, 1, {0x00, 0x00}},
	{0x0a67, 1, {0x80, 0x00}},
	{0x0a54, 1, {0x0e, 0x00}},
	{0x0a65, 1, {0x10, 0x00}},
	{0x0a98, 1, {0x04, 0x00}},
	{0x05be, 1, {0x00, 0x00}},
	{0x05a9, 1, {0x01, 0x00}},
	{0x0089, 1, {0x02, 0x00}},
	{0x0aa0, 1, {0x00, 0x00}},
	{0x0023, 1, {0x00, 0x00}},
	{0x0022, 1, {0x00, 0x00}},
	{0x0025, 1, {0x00, 0x00}},
	{0x0024, 1, {0x00, 0x00}},
	{0x0028, 1, {0x0f, 0x00}},
	{0x0029, 1, {0x18, 0x00}},
	{0x002a, 1, {0x08, 0x00}},
	{0x002b, 1, {0x88, 0x00}},
	{0x0317, 1, {0x1c, 0x00}},
	{0x0a70, 1, {0x03, 0x00}},
	{0x0a82, 1, {0x00, 0x00}},
	{0x0a83, 1, {0xe0, 0x00}},
	{0x0a71, 1, {0x00, 0x00}},
	{0x0a72, 1, {0x02, 0x00}},
	{0x0a73, 1, {0x60, 0x00}},
	{0x0a75, 1, {0x41, 0x00}},
	{0x0a70, 1, {0x03, 0x00}},
	{0x0a5a, 1, {0x80, 0x00}},
	{SEN_CMD_DELAY, 1, {20, 0x0}}, //sleep 20
	{0x0089, 1, {0x00, 0x00}},
	{0x05be, 1, {0x01, 0x00}},
	{0x0a70, 1, {0x00, 0x00}},
	{0x0080, 1, {0x02, 0x00}},
	{0x0a67, 1, {0x00, 0x00}},
	{0x024b, 1, {0x02, 0x00}},
	{0x0220, 1, {0x80, 0x00}},
	{0x0058, 1, {0x00, 0x00}},
	{0x0059, 1, {0x04, 0x00}},
	{SEN_CMD_SETVD, 1, {0x00, 0x0}},
	{SEN_CMD_PRESET, 1, {0x00, 0x0}},
	{SEN_CMD_DIRECTION, 1, {0x00, 0x0}},
	{0x0100,1,{0x09,0x0}},
};

static UINT32 cur_sen_mode[CTL_SEN_ID_MAX] = {CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1};
static UINT32 cur_fps[CTL_SEN_ID_MAX] = {0};
static UINT32 chgmode_fps[CTL_SEN_ID_MAX] = {0};
static UINT32 power_ctrl_mclk[CTL_SEN_CLK_SEL_MAX] = {0};
static UINT32 reset_ctrl_count[CTL_SEN_ID_MAX] = {0};
static UINT32 pwdn_ctrl_count[CTL_SEN_ID_MAX] = {0};
static ISP_SENSOR_CTRL sensor_ctrl_last[CTL_SEN_ID_MAX] = {0};
static ISP_SENSOR_PRESET_CTRL preset_ctrl[CTL_SEN_ID_MAX] = {0};
static UINT32 compensation_ratio[CTL_SEN_ID_MAX][ISP_SEN_MFRAME_MAX_NUM] = {0};
static INT32 is_fastboot[CTL_SEN_ID_MAX];
static UINT32 fastboot_i2c_id[CTL_SEN_ID_MAX];
static UINT32 fastboot_i2c_addr[CTL_SEN_ID_MAX];
static BOOL i2c_valid[CTL_SEN_ID_MAX];

static CTL_SEN_DRV_TAB gc8613_sen_drv_tab = {
	sen_open_gc8613,
	sen_close_gc8613,
	sen_sleep_gc8613,
	sen_wakeup_gc8613,
	sen_write_reg_gc8613,
	sen_read_reg_gc8613,
	sen_chg_mode_gc8613,
	sen_chg_fps_gc8613,
	sen_set_info_gc8613,
	sen_get_info_gc8613
};

static CTL_SEN_DRV_TAB *sen_get_drv_tab_gc8613(void)
{
	return &gc8613_sen_drv_tab;
}

static void sen_pwr_ctrl_gc8613(CTL_SEN_ID id, CTL_SEN_PWR_CTRL_FLAG flag, CTL_SEN_CLK_CB clk_cb)
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

static CTL_SEN_CMD sen_set_cmd_info_gc8613(UINT32 addr, UINT32 data_length, UINT32 data0, UINT32 data1)
{
	CTL_SEN_CMD cmd;

	cmd.addr = addr;
	cmd.data_len = data_length;
	cmd.data[0] = data0;
	cmd.data[1] = data1;
	return cmd;
}

#if defined(__KERNEL__)
static void sen_load_cfg_from_compatible_gc8613(struct device_node *of_node)
{
	DBG_DUMP("compatible valid, using peri-dev.dtsi \r\n");
	sen_common_load_cfg_preset_compatible(of_node, &sen_preset);
	sen_common_load_cfg_direction_compatible(of_node, &sen_direction);
	sen_common_load_cfg_power_compatible(of_node, &sen_power);
	sen_common_load_cfg_i2c_compatible(of_node, &sen_i2c);
}
#endif

static ER sen_open_gc8613(CTL_SEN_ID id)
{
	ER rt = E_OK;

	#if defined(__KERNEL__)
	sen_i2c_reg_cb(sen_load_cfg_from_compatible_gc8613);
	#endif

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

static ER sen_close_gc8613(CTL_SEN_ID id)
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

static ER sen_sleep_gc8613(CTL_SEN_ID id)
{
	return E_OK;
}

static ER sen_wakeup_gc8613(CTL_SEN_ID id)
{
	return E_OK;
}

static ER sen_write_reg_gc8613(CTL_SEN_ID id, CTL_SEN_CMD *cmd)
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

static ER sen_read_reg_gc8613(CTL_SEN_ID id, CTL_SEN_CMD *cmd)
{
	struct i2c_msg msgs[2];
	unsigned char tmp[2], tmp2[2];
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

static UINT32 sen_get_cmd_tab_gc8613(CTL_SEN_MODE mode, CTL_SEN_CMD **cmd_tab)
{
	switch (mode) {
	case CTL_SEN_MODE_1:
		*cmd_tab = gc8613_mode_1;
		return sizeof(gc8613_mode_1) / sizeof(CTL_SEN_CMD);

	case CTL_SEN_MODE_2:
		*cmd_tab = gc8613_mode_2;
		return sizeof(gc8613_mode_2) / sizeof(CTL_SEN_CMD);

	case CTL_SEN_MODE_3:
		*cmd_tab = gc8613_mode_3;
		return sizeof(gc8613_mode_3) / sizeof(CTL_SEN_CMD);

	default:
		DBG_ERR("sensor mode %d no cmd table \r\n", mode);
		*cmd_tab = NULL;
		return 0;
	}
}

static ER sen_chg_mode_gc8613(CTL_SEN_ID id, CTL_SENDRV_CHGMODE_OBJ chgmode_obj)
{
	ISP_SENSOR_CTRL sensor_ctrl = {0};
	CTL_SEN_CMD *p_cmd_list = NULL, cmd;
	CTL_SEN_FLIP flip = CTL_SEN_FLIP_NONE;
	UINT32 sensor_vd;
	UINT32 idx, cmd_num = 0;
	ER rt = E_OK;

	cur_sen_mode[id] = chgmode_obj.mode;

	if (is_fastboot[id]) {
		#if defined(__KERNEL__)
		ISP_BUILTIN_SENSOR_CTRL *p_sensor_ctrl_temp;

		p_sensor_ctrl_temp = isp_builtin_get_sensor_gain(id);
		sensor_ctrl.gain_ratio[0] = p_sensor_ctrl_temp->gain_ratio[0];
		sensor_ctrl.gain_ratio[1] = p_sensor_ctrl_temp->gain_ratio[1];
		p_sensor_ctrl_temp = isp_builtin_get_sensor_expt(id);
		sensor_ctrl.exp_time[0] = p_sensor_ctrl_temp->exp_time[0];
		sensor_ctrl.exp_time[1] = p_sensor_ctrl_temp->exp_time[1];
		sen_set_chgmode_fps_gc8613(id, isp_builtin_get_chgmode_fps(id));
		sen_set_cur_fps_gc8613(id, isp_builtin_get_chgmode_fps(id));
		sen_set_gain_gc8613(id, &sensor_ctrl);
		sen_set_expt_gc8613(id, &sensor_ctrl);
		#endif
		preset_ctrl[id].mode = ISP_SENSOR_PRESET_CHGMODE;

		return E_OK;
	}

	// get & set sensor cmd table
	cmd_num = sen_get_cmd_tab_gc8613(chgmode_obj.mode, &p_cmd_list);
	if (p_cmd_list == NULL) {
		DBG_ERR("%s: SenMode(%d) out of range!!! \r\n", __func__, chgmode_obj.mode);
		return E_SYS;
	}

	if (basic_param.property & CTL_SEN_SUPPORT_PROPERTY_CHGFPS) {
		sensor_vd = sen_calc_chgmode_vd_gc8613(id, chgmode_obj.frame_rate);
	} else {
		DBG_WRN(" not support fps adjust \r\n");
		sen_set_cur_fps_gc8613(id, mode_basic_param[cur_sen_mode[id]].dft_fps);
		sen_set_chgmode_fps_gc8613(id, mode_basic_param[cur_sen_mode[id]].dft_fps);
		sensor_vd = mode_basic_param[cur_sen_mode[id]].signal_info.vd_period;
	}

	for (idx = 0; idx < cmd_num; idx++) {
		if (p_cmd_list[idx].addr == SEN_CMD_DELAY) {
			vos_util_delay_ms((p_cmd_list[idx].data[0] & 0xFF) | ((p_cmd_list[idx].data[1] & 0xFF) << 8));
		} else if (p_cmd_list[idx].addr == SEN_CMD_SETVD) {
			cmd = sen_set_cmd_info_gc8613(0x0340, 1, (sensor_vd >> 8) & 0x3F, 0x00);
			rt |= sen_write_reg_gc8613(id, &cmd);
			cmd = sen_set_cmd_info_gc8613(0x0341, 1, sensor_vd & 0xFF, 0x00);
			rt |= sen_write_reg_gc8613(id, &cmd);
		} else if (p_cmd_list[idx].addr == SEN_CMD_PRESET) {
			switch (preset_ctrl[id].mode) {
				default:
				case ISP_SENSOR_PRESET_DEFAULT:
					sensor_ctrl.gain_ratio[0] = sen_preset[id].gain_ratio;
					sensor_ctrl.exp_time[0] = sen_preset[id].expt_time;
					break;

				case ISP_SENSOR_PRESET_CHGMODE:
					memcpy(&sensor_ctrl, &sensor_ctrl_last[id], sizeof(ISP_SENSOR_CTRL));
					break;

				case ISP_SENSOR_PRESET_AE:
					sensor_ctrl.exp_time[0] = preset_ctrl[id].exp_time[0];
					sensor_ctrl.exp_time[1] = preset_ctrl[id].exp_time[1];
					sensor_ctrl.gain_ratio[0] = preset_ctrl[id].gain_ratio[0];
					sensor_ctrl.gain_ratio[1] = preset_ctrl[id].gain_ratio[1];
				break;
			}
			sen_set_gain_gc8613(id, &sensor_ctrl);
			sen_set_expt_gc8613(id, &sensor_ctrl);
		} else if (p_cmd_list[idx].addr == SEN_CMD_DIRECTION) {
			if (sen_direction[id].mirror) {
				flip |= CTL_SEN_FLIP_H;
			}
			if (sen_direction[id].flip) {
				flip |= CTL_SEN_FLIP_V;
			}
			sen_set_flip_gc8613(id, &flip);
		} else {
			cmd = sen_set_cmd_info_gc8613(p_cmd_list[idx].addr, p_cmd_list[idx].data_len, p_cmd_list[idx].data[0], p_cmd_list[idx].data[1]);
			rt |= sen_write_reg_gc8613(id, &cmd);
		}
	}

	preset_ctrl[id].mode = ISP_SENSOR_PRESET_CHGMODE;

	if (rt != E_OK) {
		DBG_ERR("write register error %d \r\n", (INT)rt);
		return rt;
	}

	return E_OK;
}

static ER sen_chg_fps_gc8613(CTL_SEN_ID id, UINT32 fps)
{
	CTL_SEN_CMD cmd;
	UINT32 sensor_vd;
	ER rt = E_OK;

	if (basic_param.property & CTL_SEN_SUPPORT_PROPERTY_CHGFPS) {
		sensor_vd = sen_calc_chgmode_vd_gc8613(id, fps);
	} else {
		DBG_WRN(" not support fps adjust \r\n");
		sen_set_cur_fps_gc8613(id, mode_basic_param[cur_sen_mode[id]].dft_fps);
		sen_set_chgmode_fps_gc8613(id, mode_basic_param[cur_sen_mode[id]].dft_fps);
		sensor_vd = mode_basic_param[cur_sen_mode[id]].signal_info.vd_period;
	}

	cmd = sen_set_cmd_info_gc8613(0x0340, 1, (sensor_vd >> 8) & 0x3F, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x0341, 1, sensor_vd & 0xFF, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);

	return rt;
}

static ER sen_set_info_gc8613(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data)
{
	switch (drv_cfg_id) {
	case CTL_SENDRV_CFGID_SET_EXPT:
		sen_set_expt_gc8613(id, data);
		break;
	case CTL_SENDRV_CFGID_SET_GAIN:
		sen_set_gain_gc8613(id, data);
		break;
	case CTL_SENDRV_CFGID_FLIP_TYPE:
		sen_set_flip_gc8613(id, (CTL_SEN_FLIP *)(data));
		break;
	case CTL_SENDRV_CFGID_USER_DEFINE1:
		sen_set_preset_gc8613(id, (ISP_SENSOR_PRESET_CTRL *)(data));
		break;
	default:
		return E_NOSPT;
	}

	return E_OK;
}

static ER sen_get_info_gc8613(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data)
{
	ER rt = E_OK;

	switch (drv_cfg_id) {
	case CTL_SENDRV_CFGID_GET_EXPT:
		sen_get_expt_gc8613(id, data);
		break;
	case CTL_SENDRV_CFGID_GET_GAIN:
		sen_get_gain_gc8613(id, data);
		break;
	case CTL_SENDRV_CFGID_GET_ATTR_BASIC:
		sen_get_attr_basic_gc8613((CTL_SENDRV_GET_ATTR_BASIC_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_ATTR_SIGNAL:
		sen_get_attr_signal_gc8613((CTL_SENDRV_GET_ATTR_SIGNAL_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_ATTR_CMDIF:
		rt = sen_get_attr_cmdif_gc8613(id, (CTL_SENDRV_GET_ATTR_CMDIF_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_ATTR_IF:
		rt = sen_get_attr_if_gc8613((CTL_SENDRV_GET_ATTR_IF_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_FPS:
		sen_get_fps_gc8613(id, (CTL_SENDRV_GET_FPS_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_SPEED:
		sen_get_speed_gc8613(id, (CTL_SENDRV_GET_SPEED_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_MODE_BASIC:
		sen_get_mode_basic_gc8613((CTL_SENDRV_GET_MODE_BASIC_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_MODE_MIPI:
		sen_get_mode_mipi_gc8613((CTL_SENDRV_GET_MODE_MIPI_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_MODESEL:
		sen_get_modesel_gc8613((CTL_SENDRV_GET_MODESEL_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_MODE_ROWTIME:
		sen_get_rowtime_gc8613(id, (CTL_SENDRV_GET_MODE_ROWTIME_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_FLIP_TYPE:
		rt = sen_get_flip_gc8613(id, (CTL_SEN_FLIP *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_PROBE_SEN:
		sen_ext_get_probe_sen_gc8613(id, (CTL_SENDRV_GET_PROBE_SEN_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_USER_DEFINE2:
		sen_get_min_expt_gc8613(id, data);
		break;
	default:
		rt = E_NOSPT;
	}

	return rt;
}

static UINT32 sen_calc_chgmode_vd_gc8613(CTL_SEN_ID id, UINT32 fps)
{
	UINT32 sensor_vd;

	if (1 > fps) {
		DBG_ERR("sensor fps can not small than (%d),change to dft sensor fps (%d) \r\n", fps, mode_basic_param[cur_sen_mode[id]].dft_fps);
		fps = mode_basic_param[cur_sen_mode[id]].dft_fps;
	}
	sensor_vd = (mode_basic_param[cur_sen_mode[id]].signal_info.vd_period) * (mode_basic_param[cur_sen_mode[id]].dft_fps) / fps;

	sen_set_chgmode_fps_gc8613(id, fps);
	sen_set_cur_fps_gc8613(id, fps);

	if (sensor_vd > MAX_VD_PERIOD) {
		DBG_ERR("sensor vd out of sensor driver range (%d) \r\n", sensor_vd);
		sensor_vd = MAX_VD_PERIOD;
		fps = (mode_basic_param[cur_sen_mode[id]].signal_info.vd_period) * (mode_basic_param[cur_sen_mode[id]].dft_fps) / sensor_vd;
		sen_set_chgmode_fps_gc8613(id, fps);
		sen_set_cur_fps_gc8613(id, fps);
	}

	if (sensor_vd < mode_basic_param[cur_sen_mode[id]].signal_info.vd_period) {
		DBG_ERR("sensor vd out of sensor driver range (%d) \r\n", sensor_vd);
		sensor_vd = mode_basic_param[cur_sen_mode[id]].signal_info.vd_period;
		sen_set_chgmode_fps_gc8613(id, mode_basic_param[cur_sen_mode[id]].dft_fps);
		sen_set_cur_fps_gc8613(id, mode_basic_param[cur_sen_mode[id]].dft_fps);
	}

	return sensor_vd;
}

static UINT32 sen_calc_exp_vd_gc8613(CTL_SEN_ID id, UINT32 fps)
{
	UINT32 sensor_vd;

	if (1 > fps) {
		DBG_ERR("sensor fps can not small than (%d),change to dft sensor fps (%d) \r\n", fps, mode_basic_param[cur_sen_mode[id]].dft_fps);	
		fps = mode_basic_param[cur_sen_mode[id]].dft_fps;
	}
	sensor_vd = (mode_basic_param[cur_sen_mode[id]].signal_info.vd_period) * (mode_basic_param[cur_sen_mode[id]].dft_fps) / fps;

	if (sensor_vd > MAX_VD_PERIOD) {
		DBG_ERR("sensor vd out of sensor driver range (%d) \r\n", sensor_vd);
		sensor_vd = MAX_VD_PERIOD;

	}

	if (sensor_vd < mode_basic_param[cur_sen_mode[id]].signal_info.vd_period) {
		DBG_ERR("sensor vd out of sensor driver range (%d) \r\n", sensor_vd);
		sensor_vd = mode_basic_param[cur_sen_mode[id]].signal_info.vd_period;
	}

	return sensor_vd;
}

typedef struct gain_set {
	UINT32 gain_reg1;
	UINT32 gain_reg2;
	UINT32 gain_reg3;
	UINT32 gain_reg4;
	UINT32 gain_reg5;
	UINT32 gain_reg6;
	UINT32 total_gain;
} gain_set_t;

static gain_set_t gain_table[] = // gain*1000
{
	{0x00, 0x00, 0x00, 0x0d, 0x0d, 0x77, 1000},
	{0x90, 0x02, 0x00, 0x0e, 0x0e, 0x77, 1156},
	{0x01, 0x00, 0x00, 0x0e, 0x0e, 0x77, 1406},
	{0x91, 0x02, 0x00, 0x0f, 0x0f, 0x77, 1641},
	{0x02, 0x00, 0x00, 0x0f, 0x0f, 0x77, 1969},
	{0x00, 0x00, 0x00, 0x0d, 0x0d, 0x75, 2219},
	{0x90, 0x02, 0x00, 0x0d, 0x0d, 0x75, 2563},
	{0x01, 0x00, 0x00, 0x0e, 0x0e, 0x75, 3125},
	{0x91, 0x02, 0x00, 0x0e, 0x0e, 0x75, 3734},
	{0x02, 0x00, 0x00, 0x0f, 0x0f, 0x75, 4438},
	{0x92, 0x02, 0x00, 0x0f, 0x0f, 0x75, 5328},
	{0x03, 0x00, 0x00, 0x10, 0x10, 0x75, 6359},
	{0x93, 0x02, 0x00, 0x10, 0x10, 0x75, 7641},
	{0x00, 0x00, 0x01, 0x11, 0x11, 0x75, 8313},
	{0x90, 0x02, 0x01, 0x12, 0x12, 0x75, 9875},
	{0x01, 0x00, 0x01, 0x13, 0x13, 0x75, 12000},
	{0x91, 0x02, 0x01, 0x14, 0x14, 0x75, 14828},
	{0x02, 0x00, 0x01, 0x15, 0x15, 0x75, 16375},
	{0x92, 0x02, 0x01, 0x16, 0x16, 0x75, 19641},
	{0x03, 0x00, 0x01, 0x17, 0x17, 0x75, 23438},
	{0x93, 0x02, 0x01, 0x18, 0x18, 0x75, 27531},
	{0x04, 0x00, 0x01, 0x19, 0x19, 0x75, 33063},
	{0x94, 0x02, 0x01, 0x1b, 0x1b, 0x75, 39375},
	{0x05, 0x00, 0x01, 0x1d, 0x1d, 0x75, 47641},
	{0x95, 0x02, 0x01, 0x1e, 0x1e, 0x75, 57313},
	{0x06, 0x00, 0x01, 0x20, 0x20, 0x75, 68234},
	{0x96, 0x02, 0x01, 0x22, 0x22, 0x75, 81359},
	{0xb6, 0x04, 0x01, 0x23, 0x23, 0x75, 95391},
	{0x86, 0x06, 0x01, 0x24, 0x24, 0x75, 114344},
	{0x06, 0x08, 0x01, 0x26, 0x26, 0x75, 132922},
};

#define NUM_OF_GAINSET (sizeof(gain_table) / sizeof(gain_set_t))

typedef struct gain_wdr_set {
	UINT32 gain_reg1;
	UINT32 gain_reg2;
	UINT32 gain_reg3;
	UINT32 gain_reg4;
	UINT32 gain_reg5;
	UINT32 gain_reg6;
	UINT32 total_gain;
} gain_wdr_set_t;

static gain_wdr_set_t gain_wdr_table[] = // gain*1000
{
	// 0614  0615  0225  1467  1468  1447
	{0x00, 0x00, 0x00, 0x10, 0x10, 0x77,  1000},
	{0x90, 0x02, 0x00, 0x11, 0x11, 0x77,  1156},
	{0x01, 0x00, 0x00, 0x11, 0x11, 0x77,  1406},
	{0x91, 0x02, 0x00, 0x12, 0x12, 0x77,  1640},
	{0x02, 0x00, 0x00, 0x12, 0x12, 0x77,  1968},
	{0x00, 0x00, 0x00, 0x10, 0x10, 0x75,  2218},
	{0x90, 0x02, 0x00, 0x11, 0x11, 0x75,  2562},
	{0x01, 0x00, 0x00, 0x11, 0x11, 0x75,  3125},
	{0x91, 0x02, 0x00, 0x12, 0x12, 0x75,  3734},
	{0x02, 0x00, 0x00, 0x12, 0x12, 0x75,  4437},
	{0x92, 0x02, 0x00, 0x13, 0x13, 0x75,  5328},
	{0x03, 0x00, 0x00, 0x14, 0x14, 0x75,  6359},
	{0x93, 0x02, 0x00, 0x15, 0x15, 0x75,  7640},
	{0x00, 0x00, 0x01, 0x17, 0x17, 0x75,  8312},
	{0x90, 0x02, 0x01, 0x18, 0x18, 0x75,  9875},
	{0x01, 0x00, 0x01, 0x19, 0x19, 0x75, 12000},
	{0x91, 0x02, 0x01, 0x1a, 0x1a, 0x75, 14828},
	{0x02, 0x00, 0x01, 0x1b, 0x1b, 0x75, 16375},
	{0x92, 0x02, 0x01, 0x1c, 0x1c, 0x75, 19640},
	{0x03, 0x00, 0x01, 0x1e, 0x1e, 0x75, 23437},
	{0x93, 0x02, 0x01, 0x20, 0x20, 0x75, 27531},
	{0x04, 0x00, 0x01, 0x22, 0x22, 0x75, 33062},
	{0x94, 0x02, 0x01, 0x24, 0x24, 0x75, 39375},
	{0x05, 0x00, 0x01, 0x26, 0x26, 0x75, 47640},
	{0x95, 0x02, 0x01, 0x28, 0x28, 0x75, 57312},
	{0x06, 0x00, 0x01, 0x28, 0x28, 0x75, 68234},
};

#define NUM_OF_GAINWDRSET (sizeof(gain_wdr_table) / sizeof(gain_wdr_set_t))

static UINT8 gain_DAG_YN_regValTable[22][10] = {
	//614	615	 225   1467	 1468  26e	 270   1447	  b8	b9
	{0x00, 0x00, 0x00, 0x46, 0x46, 0x74, 0x02, 0x77, 0x01, 0x00},
	{0x90, 0x02, 0x00, 0x47, 0x47, 0x74, 0x02, 0x77, 0x01, 0x0a},
	{0x01, 0x00, 0x00, 0x47, 0x47, 0x77, 0x02, 0x77, 0x01, 0x1a},
	{0x91, 0x02, 0x00, 0x48, 0x48, 0x77, 0x02, 0x77, 0x01, 0x29},
	{0x02, 0x00, 0x00, 0x48, 0x48, 0x79, 0x02, 0x77, 0x01, 0x3e},
	{0x00, 0x00, 0x00, 0x46, 0x46, 0x74, 0x02, 0x75, 0x02, 0x0d},
	{0x90, 0x02, 0x00, 0x47, 0x47, 0x74, 0x02, 0x75, 0x02, 0x24},
	{0x01, 0x00, 0x00, 0x47, 0x47, 0x77, 0x02, 0x75, 0x03, 0x08},
	{0x91, 0x02, 0x00, 0x48, 0x48, 0x79, 0x02, 0x75, 0x03, 0x2e},
	{0x02, 0x00, 0x00, 0x49, 0x49, 0x7a, 0x02, 0x75, 0x04, 0x1b},
	{0x92, 0x02, 0x00, 0x4b, 0x4b, 0x7b, 0x02, 0x75, 0x05, 0x14},
	{0x03, 0x00, 0x00, 0x4c, 0x4c, 0x7c, 0x02, 0x75, 0x06, 0x17},
	{0x93, 0x02, 0x00, 0x4d, 0x4d, 0x7d, 0x02, 0x75, 0x07, 0x29},
	{0x00, 0x00, 0x01, 0x4f, 0x4f, 0x7e, 0x02, 0x75, 0x08, 0x13},
	{0x90, 0x02, 0x01, 0x50, 0x50, 0x7f, 0x02, 0x75, 0x09, 0x38},
	{0x01, 0x00, 0x01, 0x51, 0x51, 0x7f, 0x02, 0x75, 0x0c, 0x00},
	{0x91, 0x02, 0x01, 0x53, 0x53, 0x7f, 0x02, 0x75, 0x0e, 0x35},
	{0x02, 0x00, 0x01, 0x54, 0x54, 0x7f, 0x02, 0x75, 0x10, 0x18},
	{0x92, 0x02, 0x01, 0x56, 0x56, 0x7f, 0x02, 0x75, 0x13, 0x29},
	{0x03, 0x00, 0x01, 0x58, 0x58, 0x7f, 0x02, 0x75, 0x17, 0x1c},
	{0x93, 0x02, 0x01, 0x5a, 0x5a, 0x7f, 0x01, 0x75, 0x1b, 0x22},
	{0x04, 0x00, 0x01, 0x5c, 0x5c, 0x7f, 0x01, 0x75, 0x21, 0x04},
};

UINT32 gain_DAG_YN_gainLevelTable[23] = {
		64  ,
		74  ,
		90  ,
		105 ,
		126 ,
		142 ,
		164 ,
		200 ,
		239 ,
		284 ,
		341 ,
		407 ,
		489 ,
		532 ,
		632 ,
		768 ,
		949 ,
		1048,
		1257,
		1500,
		1762,
		2116,
		0xffffffff,
};

static int gain_DAG_YN_total = sizeof(gain_DAG_YN_gainLevelTable) / sizeof(UINT32);

static void sen_set_gain_dag_yn_gc8613(CTL_SEN_ID id, UINT32 gain)
{
	CTL_SEN_CMD cmd;
	ER rt = E_OK;
	int i;

	UINT32 tol_dig_gain = 0;

	for(i = 0; i < gain_DAG_YN_total; i++)
	{
		if((gain_DAG_YN_gainLevelTable[i] <= gain)&&(gain < gain_DAG_YN_gainLevelTable[i+1]))
			break;
	}

	tol_dig_gain = gain*64/gain_DAG_YN_gainLevelTable[i];

	cmd = sen_set_cmd_info_gc8613(0x031d, 1, 0x2d, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x0614, 1, gain_DAG_YN_regValTable[i][0], 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x0615, 1, gain_DAG_YN_regValTable[i][1], 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x026e, 1, gain_DAG_YN_regValTable[i][5], 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x0270, 1, gain_DAG_YN_regValTable[i][6], 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x031d, 1, 0x28, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);

	cmd = sen_set_cmd_info_gc8613(0x0225, 1, gain_DAG_YN_regValTable[i][2], 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x1467, 1, gain_DAG_YN_regValTable[i][3], 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x1468, 1, gain_DAG_YN_regValTable[i][4], 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x1447, 1, gain_DAG_YN_regValTable[i][7], 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x00b8, 1, gain_DAG_YN_regValTable[i][8], 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x00b9, 1, gain_DAG_YN_regValTable[i][9], 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);

	cmd = sen_set_cmd_info_gc8613(0x0064, 1, (tol_dig_gain>>6), 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x0065, 1, (tol_dig_gain&0x3f), 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);

	if (rt != E_OK) {
		DBG_ERR("write register error %d \r\n", (INT)rt);
	}
}

static void sen_set_gain_gc8613(CTL_SEN_ID id, void *param)
{
	ISP_SENSOR_CTRL *sensor_ctrl = (ISP_SENSOR_CTRL *)param;
	UINT32 data1[ISP_SEN_MFRAME_MAX_NUM] = {0};
	UINT32 data2[ISP_SEN_MFRAME_MAX_NUM] = {0};
	UINT32 data3[ISP_SEN_MFRAME_MAX_NUM] = {0};
	UINT32 frame_cnt, total_frame;
	UINT32 i, tbl_idx = NUM_OF_GAINSET, tbl_wdr_idx = NUM_OF_GAINWDRSET, digital_gain = 0;
	CTL_SEN_CMD cmd;
	ER rt = E_OK;

	sensor_ctrl_last[id].gain_ratio[0] = sensor_ctrl->gain_ratio[0];
	sensor_ctrl_last[id].gain_ratio[1] = sensor_ctrl->gain_ratio[1];

	// Calculate sensor gain
	if (mode_basic_param[cur_sen_mode[id]].frame_num == 0) {
		DBG_WRN("total_frame = 0, force to 1 \r\n");
		total_frame = 1;
	} else {
		total_frame = mode_basic_param[cur_sen_mode[id]].frame_num;
	}

	for (frame_cnt = 0; frame_cnt < total_frame; frame_cnt++) {
		if (100 <= (compensation_ratio[id][frame_cnt])) {
			sensor_ctrl->gain_ratio[frame_cnt] = (sensor_ctrl->gain_ratio[frame_cnt]) * (compensation_ratio[id][frame_cnt]) / 100;
		}
		if (sensor_ctrl->gain_ratio[frame_cnt] < (mode_basic_param[cur_sen_mode[id]].gain.min)) {
			sensor_ctrl->gain_ratio[frame_cnt] = (mode_basic_param[cur_sen_mode[id]].gain.min);
		} else if (sensor_ctrl->gain_ratio[frame_cnt] > (mode_basic_param[cur_sen_mode[id]].gain.max)) {
			sensor_ctrl->gain_ratio[frame_cnt] = (mode_basic_param[cur_sen_mode[id]].gain.max);
		}
		
		if (mode_basic_param[cur_sen_mode[id]].mode == CTL_SEN_MODE_3) // DAG mode
			return sen_set_gain_dag_yn_gc8613(id, sensor_ctrl->gain_ratio[frame_cnt]*64/1000);

		if (mode_basic_param[cur_sen_mode[id]].mode_type == CTL_SEN_MODE_LINEAR) {
			for (i = 0; i < NUM_OF_GAINSET; i++) {
				if (gain_table[i].total_gain > sensor_ctrl->gain_ratio[frame_cnt]) {
					tbl_idx = i;
					break;
				}
			}

			// collect the gain setting
			if (tbl_idx < 1) {
				digital_gain = sensor_ctrl->gain_ratio[frame_cnt] * 1000 / gain_table[frame_cnt].total_gain;
				data1[frame_cnt] = (gain_table[frame_cnt].gain_reg3 << 16) | (gain_table[frame_cnt].gain_reg2 << 8) | (gain_table[frame_cnt].gain_reg1);
				data2[frame_cnt] = (gain_table[frame_cnt].gain_reg6 << 16) | (gain_table[frame_cnt].gain_reg5 << 8) | (gain_table[frame_cnt].gain_reg4);
			} else {
				digital_gain = sensor_ctrl->gain_ratio[frame_cnt] * 1000 / gain_table[tbl_idx-1].total_gain;
				data1[frame_cnt] = (gain_table[tbl_idx-1].gain_reg3 << 16) | (gain_table[tbl_idx-1].gain_reg2 << 8) | (gain_table[tbl_idx-1].gain_reg1);
				data2[frame_cnt] = (gain_table[tbl_idx-1].gain_reg6 << 16) | (gain_table[tbl_idx-1].gain_reg5 << 8) | (gain_table[tbl_idx-1].gain_reg4);
			}

			if (digital_gain <= 1000) {
				data3[frame_cnt] = 64;
			} else {
				data3[frame_cnt] = digital_gain * 64 / 1000;
			}
		} else if (mode_basic_param[cur_sen_mode[id]].mode_type == CTL_SEN_MODE_STAGGER_HDR) {
			for (i = 0; i < NUM_OF_GAINWDRSET; i++) {
				if (gain_wdr_table[i].total_gain > sensor_ctrl->gain_ratio[frame_cnt]) {
					tbl_wdr_idx = i;
					break;
				}
			}

			// collect the gain setting
			if (tbl_wdr_idx < 1) {
				digital_gain = sensor_ctrl->gain_ratio[frame_cnt] * 1000 / gain_wdr_table[frame_cnt].total_gain;
				data1[frame_cnt] = (gain_wdr_table[frame_cnt].gain_reg3 << 16) | (gain_wdr_table[frame_cnt].gain_reg2 << 8) | (gain_wdr_table[frame_cnt].gain_reg1);
				data2[frame_cnt] = (gain_wdr_table[frame_cnt].gain_reg6 << 16) | (gain_wdr_table[frame_cnt].gain_reg5 << 8) | (gain_wdr_table[frame_cnt].gain_reg4);
			} else {
				digital_gain = sensor_ctrl->gain_ratio[frame_cnt] * 1000 / gain_wdr_table[tbl_wdr_idx-1].total_gain;
				data1[frame_cnt] = (gain_wdr_table[tbl_wdr_idx-1].gain_reg3 << 16) | (gain_wdr_table[tbl_wdr_idx-1].gain_reg2 << 8) | (gain_wdr_table[tbl_wdr_idx-1].gain_reg1);
				data2[frame_cnt] = (gain_wdr_table[tbl_wdr_idx-1].gain_reg6 << 16) | (gain_wdr_table[tbl_wdr_idx-1].gain_reg5 << 8) | (gain_wdr_table[tbl_wdr_idx-1].gain_reg4);
			}

			if (digital_gain <= 1000) {
				data3[frame_cnt] = 64;
			} else {
				data3[frame_cnt] = digital_gain * 64 / 1000;
			}
		}
	}

	cmd = sen_set_cmd_info_gc8613(0x031d, 1, 0x2d, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x0614, 1, data1[0] & 0xFF, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x0615, 1, (data1[0] >> 8) & 0xFF, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x031d, 1, 0x28, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x0225, 1, (data1[0] >> 16) & 0xFF, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x1467, 1, data2[0] & 0xFF, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x1468, 1, (data2[0] >> 8) & 0xFF, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x1447, 1, (data2[0] >> 16) & 0xFF, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);

	cmd = sen_set_cmd_info_gc8613(0x0064, 1, (data3[0] >> 6)& 0xFF, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x0065, 1, (data3[0] & 0x3F), 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	if (mode_basic_param[cur_sen_mode[id]].mode_type == CTL_SEN_MODE_STAGGER_HDR) {	
		cmd = sen_set_cmd_info_gc8613(0x007b, 1, (data3[1] >> 6)& 0xFF, 0x00);
		rt = sen_write_reg_gc8613(id, &cmd);
		cmd = sen_set_cmd_info_gc8613(0x007c, 1, (data3[1] & 0x3F), 0x00);
		rt = sen_write_reg_gc8613(id, &cmd);
	}

	if (rt != E_OK) {
		DBG_ERR("write register error %d \r\n", (INT)rt);
	}
}

static void sen_set_expt_gc8613(CTL_SEN_ID id, void *param)
{
	ISP_SENSOR_CTRL *sensor_ctrl = (ISP_SENSOR_CTRL *)param;
	UINT32 line[ISP_SEN_MFRAME_MAX_NUM];
	UINT32 frame_cnt, total_frame;
	CTL_SEN_CMD cmd;
	UINT32 expt_time = 0, sensor_vd = 0, chgmode_fps = 0, cur_fps = 0, clac_fps = 0, t_row = 0;
	UINT32 temp_line[ISP_SEN_MFRAME_MAX_NUM] = {0};
	ER rt = E_OK;

	sensor_ctrl_last[id].exp_time[0] = sensor_ctrl->exp_time[0];
	sensor_ctrl_last[id].exp_time[1] = sensor_ctrl->exp_time[1];

	if (mode_basic_param[cur_sen_mode[id]].frame_num == 0) {
		DBG_WRN("total_frame = 0, force to 1 \r\n");
		total_frame = 1;
	} else {
		total_frame = mode_basic_param[cur_sen_mode[id]].frame_num;
	}

	// Calculate exposure line
	for (frame_cnt = 0; frame_cnt < total_frame; frame_cnt++) {
		// Calculates the exposure setting
		t_row = sen_calc_rowtime_gc8613(id, cur_sen_mode[id]);
		if (t_row == 0) {
			DBG_WRN("t_row = 0, must >= 1 \r\n");
			t_row = 1;
		}
		line[frame_cnt] = (sensor_ctrl->exp_time[frame_cnt]) * 10 / t_row;

		if (mode_basic_param[cur_sen_mode[id]].mode_type == CTL_SEN_MODE_LINEAR) {
			// Limit minimun exposure line
			if (line[frame_cnt] < MIN_EXPOSURE_LINE) {
				line[frame_cnt] = MIN_EXPOSURE_LINE;
			}
		} else if (mode_basic_param[cur_sen_mode[id]].mode_type == CTL_SEN_MODE_STAGGER_HDR) {
			// Limit minimun exposure line
			if (line[frame_cnt] < MIN_HDR_EXPOSURE_LINE) {
				line[frame_cnt] = MIN_HDR_EXPOSURE_LINE;
			}
		}
	}

	// Write exposure line
	// Get fps
	chgmode_fps = sen_get_chgmode_fps_gc8613(id);

	// calculate exposure time
	t_row = sen_calc_rowtime_gc8613(id, cur_sen_mode[id]);
	if (mode_basic_param[cur_sen_mode[id]].mode_type == CTL_SEN_MODE_LINEAR) {
		expt_time = (line[0]) * t_row / 10;
		temp_line[0] = line[0];
	} else if (mode_basic_param[cur_sen_mode[id]].mode_type == CTL_SEN_MODE_STAGGER_HDR) {	
		expt_time = (line[0] + line[1]) * t_row / 10;
		temp_line[0] = line[0];
		temp_line[1] = line[1];
	}

	// calculate fps
	if (expt_time == 0) {
		DBG_WRN("expt_time = 0, must >= 1 \r\n");
		expt_time = 1;
	}
	clac_fps = 100000000 / expt_time;

	cur_fps = (clac_fps < chgmode_fps) ? clac_fps : chgmode_fps;
	sen_set_cur_fps_gc8613(id, cur_fps);

	// calculate new vd
	sensor_vd = sen_calc_exp_vd_gc8613(id, cur_fps);

	// check max vts
	if (sensor_vd > MAX_VD_PERIOD) {
		DBG_ERR("max vts overflow \r\n");
		sensor_vd = MAX_VD_PERIOD;
	}

	// write change mode VD
	cmd = sen_set_cmd_info_gc8613(0x0340, 1, (sensor_vd >> 8) & 0x3F, 0x00);
	rt = sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x0341, 1, sensor_vd & 0xFF, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);

	if (mode_basic_param[cur_sen_mode[id]].mode_type == CTL_SEN_MODE_LINEAR) {
		// Check max exp line reg
		if (line[0] > MAX_EXPOSURE_LINE) {
			DBG_ERR("max line overflow \r\n");
			line[0] = MAX_EXPOSURE_LINE;
		}

		// Check max exp line
		if (line[0] > (sensor_vd - NON_EXPOSURE_LINE)) {
			line[0] = sensor_vd - NON_EXPOSURE_LINE;
		}
		
		compensation_ratio[id][0] = 100 * temp_line[0] / line[0];

		// Set exposure line to sensor
		cmd = sen_set_cmd_info_gc8613(0x0202, 1, (line[0] >> 8) & 0x3F, 0x00);
		rt |= sen_write_reg_gc8613(id, &cmd);
		cmd = sen_set_cmd_info_gc8613(0x0203, 1, line[0] & 0xFF, 0x00);
		rt |= sen_write_reg_gc8613(id, &cmd);

	} else if (mode_basic_param[cur_sen_mode[id]].mode_type == CTL_SEN_MODE_STAGGER_HDR) {
		// Check max exp line reg
		if (line[0] > MAX_EXPOSURE_LINE) {
			DBG_ERR("max line overflow \r\n");
			line[0] = MAX_EXPOSURE_LINE;
		}

		// Check max exp line reg
		if (line[1] > MAX_EXPOSURE_LINE) {
			DBG_ERR("max line overflow \r\n");
			line[1] = MAX_EXPOSURE_LINE;
		}

		// Check max exp line reg
		if (line[1] > MAX_SHORT_EXPOSURE_LINE) {
			line[1] = MAX_SHORT_EXPOSURE_LINE;
		}

		if (line[0] > (sensor_vd - line[1] - NON_HDR_EXPOSURE_LINE)) {
			line[0] = sensor_vd - line[1] - NON_HDR_EXPOSURE_LINE;
		}

		compensation_ratio[id][0] = 100 * temp_line[0] / line[0];
		compensation_ratio[id][1] = 100 * temp_line[1] / line[1];

		// set Long exposure line to sensor
		cmd = sen_set_cmd_info_gc8613(0x0202, 1, (line[0] >> 8) & 0x3F, 0x00);
		rt |= sen_write_reg_gc8613(id, &cmd);
		cmd = sen_set_cmd_info_gc8613(0x0203, 1, line[0] & 0xFF, 0x00);
		rt |= sen_write_reg_gc8613(id, &cmd);
		
		// set Short exposure line to sensor
		cmd = sen_set_cmd_info_gc8613(0x0200, 1, (line[1] >> 8) & 0x3F, 0x00);
		rt |= sen_write_reg_gc8613(id, &cmd);
		cmd = sen_set_cmd_info_gc8613(0x0201, 1, (line[1] & 0xFF), 0x00);
		rt |= sen_write_reg_gc8613(id, &cmd);
	}

	if (rt != E_OK) {
		DBG_ERR("write register error %d \r\n", (INT)rt);
	}
}

static void sen_set_preset_gc8613(CTL_SEN_ID id, ISP_SENSOR_PRESET_CTRL *ctrl)
{
	memcpy(&preset_ctrl[id], ctrl, sizeof(ISP_SENSOR_PRESET_CTRL));
}

static void sen_set_flip_gc8613(CTL_SEN_ID id, CTL_SEN_FLIP *flip)
{
	CTL_SEN_CMD cmd;
	UINT32 temp_mir = 0, temp_flp = 0;
	ER rt = E_OK;

	cmd = sen_set_cmd_info_gc8613(0x022C, 1, 0x00, 0x00);
	rt |= sen_read_reg_gc8613(id, &cmd);
	temp_flp = cmd.data[0];

	cmd = sen_set_cmd_info_gc8613(0x0063, 1, 0x00, 0x00);
	rt |= sen_read_reg_gc8613(id, &cmd);
	temp_mir = cmd.data[0];

	if (basic_param.property & CTL_SEN_SUPPORT_PROPERTY_FLIP) {
		if (*flip & CTL_SEN_FLIP_V) {
			temp_flp &= (~0x01);
		} else {
			temp_flp |= 0x1;
		}
	} else {
		DBG_WRN("no support flip \r\n");
	}

	if (basic_param.property & CTL_SEN_SUPPORT_PROPERTY_MIRROR) {
		if (*flip & CTL_SEN_FLIP_H) {
			if(temp_flp == 0x01) {
				temp_mir &= (~0x07);
				temp_mir |= 0x02;
			} else{
				temp_mir &= (~0x07);
			}
		} else {
			if(temp_flp == 0x01) {
				temp_mir |= 0x07;
			} else{
				temp_mir &= (~0x07);
				temp_mir |= 0x05;
			}
		}
	} else {
		DBG_WRN("no support mirror \r\n");
	}

	cmd = sen_set_cmd_info_gc8613(0x022c, 1, temp_flp, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x0063, 1, temp_mir, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);

	cmd = sen_set_cmd_info_gc8613(0x0a67, 1, 0x80, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x0a98, 1, 0x04, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x05be, 1, 0x00, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x05a9, 1, 0x01, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x0a70, 1, 0x03, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x0a73, 1, (0x60 + (temp_mir&0x03)), 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x0a5a, 1, 0x80, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	vos_util_delay_ms(20);
	cmd = sen_set_cmd_info_gc8613(0x05be, 1, 0x01, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x0a70, 1, 0x00, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x0080, 1, 0x02, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);
	cmd = sen_set_cmd_info_gc8613(0x0a67, 1, 0x00, 0x00);
	rt |= sen_write_reg_gc8613(id, &cmd);

	if (rt != E_OK) {
		DBG_ERR("write register error %d \r\n", (INT)rt);
	}
}

static ER sen_get_flip_gc8613(CTL_SEN_ID id, CTL_SEN_FLIP *flip)
{
	CTL_SEN_CMD cmd;
	CTL_SEN_CMD cmd2;
	ER rt = E_OK;

	cmd = sen_set_cmd_info_gc8613(0x022c, 1, 0x00, 0x00);
	rt = sen_read_reg_gc8613(id, &cmd);

	cmd2 = sen_set_cmd_info_gc8613(0x0063, 1, 0x00, 0x00);
	rt = sen_read_reg_gc8613(id, &cmd2);

	*flip = CTL_SEN_FLIP_NONE;

	if (basic_param.property & CTL_SEN_SUPPORT_PROPERTY_FLIP) {
		if (!(cmd.data[0] & 0x01)) {
			*flip |= CTL_SEN_FLIP_V;
		}
	} else {
		DBG_WRN("no support flip \r\n");
	}

	if (basic_param.property & CTL_SEN_SUPPORT_PROPERTY_MIRROR) {
		if (cmd.data[0]) {
			if (mode_basic_param[cur_sen_mode[id]].mode_type == CTL_SEN_MODE_LINEAR) {
				if ((cmd2.data[0] & 0x02) && (0x13 != cmd2.data[0])) {
					*flip |= CTL_SEN_FLIP_H;
				}
			} else if (mode_basic_param[cur_sen_mode[id]].mode_type == CTL_SEN_MODE_STAGGER_HDR) {		
				if ((cmd2.data[0] & 0x02) && (0x3f != cmd2.data[0])) {
					*flip |= CTL_SEN_FLIP_H;
				}
			}
		} else {
			if (0 == (cmd2.data[0] & 0x01)) {
				*flip |= CTL_SEN_FLIP_H;
			}
		}
	} else {
		DBG_WRN("no support mirror \r\n");
	}

	return rt;
}

#if defined(__FREERTOS)
void sen_get_gain_gc8613(CTL_SEN_ID id, void *param)
#else
static void sen_get_gain_gc8613(CTL_SEN_ID id, void *param)
#endif
{
	ISP_SENSOR_CTRL *sensor_ctrl = (ISP_SENSOR_CTRL *)param;

	sensor_ctrl->gain_ratio[0] = sensor_ctrl_last[id].gain_ratio[0];
	sensor_ctrl->gain_ratio[1] = sensor_ctrl_last[id].gain_ratio[1];
}

#if defined(__FREERTOS)
void sen_get_expt_gc8613(CTL_SEN_ID id, void *param)
#else
static void sen_get_expt_gc8613(CTL_SEN_ID id, void *param)
#endif
{
	ISP_SENSOR_CTRL *sensor_ctrl = (ISP_SENSOR_CTRL *)param;

	sensor_ctrl->exp_time[0] = sensor_ctrl_last[id].exp_time[0];
	sensor_ctrl->exp_time[1] = sensor_ctrl_last[id].exp_time[1];
}

static void sen_get_min_expt_gc8613(CTL_SEN_ID id, void *param)
{
	UINT32 *min_exp_time = (UINT32 *)param;
	UINT32 t_row;

	t_row = sen_calc_rowtime_gc8613(id, cur_sen_mode[id]);
	
	if (mode_basic_param[cur_sen_mode[id]].mode_type == CTL_SEN_MODE_LINEAR) {
		*min_exp_time = t_row * MIN_EXPOSURE_LINE / 10 + 1; // if linear and shdr mode is different,the value need to separate.
	} else if (mode_basic_param[cur_sen_mode[id]].mode_type == CTL_SEN_MODE_STAGGER_HDR) {
		*min_exp_time = t_row * MIN_HDR_EXPOSURE_LINE / 10 + 1; // if linear and shdr mode is different,the value need to separate.
	}
}

static void sen_get_mode_basic_gc8613(CTL_SENDRV_GET_MODE_BASIC_PARAM *mode_basic)
{
	UINT32 mode = mode_basic->mode;

	if (mode >= SEN_MAX_MODE) {
		mode = 0;
	}

	memcpy(mode_basic, &mode_basic_param[mode], sizeof(CTL_SENDRV_GET_MODE_BASIC_PARAM));
}

static void sen_get_attr_basic_gc8613(CTL_SENDRV_GET_ATTR_BASIC_PARAM *data)
{
	memcpy(data, &basic_param, sizeof(CTL_SENDRV_GET_ATTR_BASIC_PARAM));
}

static void sen_get_attr_signal_gc8613(CTL_SENDRV_GET_ATTR_SIGNAL_PARAM *data)
{
	memcpy(data, &signal_param, sizeof(CTL_SENDRV_GET_ATTR_SIGNAL_PARAM));
}

static ER sen_get_attr_cmdif_gc8613(CTL_SEN_ID id, CTL_SENDRV_GET_ATTR_CMDIF_PARAM *data)
{
	data->type = CTL_SEN_CMDIF_TYPE_I2C;
	memcpy(&data->info, &i2c, sizeof(CTL_SENDRV_I2C));
	data->info.i2c.ch = sen_i2c[id].id;
	data->info.i2c.w_addr_info[0].w_addr = sen_i2c[id].addr;
	data->info.i2c.cur_w_addr_info.w_addr_sel = data->info.i2c.w_addr_info[0].w_addr_sel;
	data->info.i2c.cur_w_addr_info.w_addr = data->info.i2c.w_addr_info[0].w_addr;
	return E_OK;
}

static ER sen_get_attr_if_gc8613(CTL_SENDRV_GET_ATTR_IF_PARAM *data)
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

static void sen_ext_get_probe_sen_gc8613(CTL_SEN_ID id, CTL_SENDRV_GET_PROBE_SEN_PARAM *data)
{
	CTL_SEN_CMD cmd;
	UINT32 read_data_h = 0, read_data_l = 0, sensor_id = 0;
	ER rt = E_OK;

	cmd = sen_set_cmd_info_gc8613(0x03F0, 1, 0x00, 0x00);
	rt |= sen_read_reg_gc8613(id, &cmd);
	read_data_h = cmd.data[0];

	cmd = sen_set_cmd_info_gc8613(0x03F1, 1, 0x00, 0x00);
	rt |= sen_read_reg_gc8613(id, &cmd);
	read_data_l = cmd.data[0];

	sensor_id = ((read_data_h & 0xFF) << 8) | (read_data_l & 0xFF);

	if (SENSOR_ID == sensor_id) {
		data->probe_rst = 0;
	} else if (rt != E_OK) { 
		data->probe_rst = rt;
	} else { // (SENSOR_ID != sensor_id) {
		data->probe_rst = -1;
	}
}

static void sen_get_fps_gc8613(CTL_SEN_ID id, CTL_SENDRV_GET_FPS_PARAM *data)
{
	data->cur_fps = sen_get_cur_fps_gc8613(id);
	data->chg_fps = sen_get_chgmode_fps_gc8613(id);
}

static void sen_get_speed_gc8613(CTL_SEN_ID id, CTL_SENDRV_GET_SPEED_PARAM *data)
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

static void sen_get_mode_mipi_gc8613(CTL_SENDRV_GET_MODE_MIPI_PARAM *data)
{
	UINT32 mode = data->mode;

	if (mode >= SEN_MAX_MODE) {
		mode = 0;
	}

	memcpy(data, &mipi_param[mode], sizeof(CTL_SENDRV_GET_MODE_MIPI_PARAM));
}

static void sen_get_modesel_gc8613(CTL_SENDRV_GET_MODESEL_PARAM *data)
{
	if (data->if_type != CTL_SEN_IF_TYPE_MIPI) {
		DBG_ERR("if_type %d N.S. \r\n", data->if_type);
		return;
	}

	if (data->data_fmt != CTL_SEN_DATA_FMT_RGB) {
		DBG_ERR("data_fmt %d N.S. \r\n", data->data_fmt);
		return;
	}

	data->mode = CTL_SEN_MODE_UNKNOWN;

	if (data->frame_num == 1){
		if ((data->size.w <= 3840) && (data->size.h <= 2160)) {
			if (data->frame_rate <= 3000) {
				if (data->pixdepth == 14)
					data->mode = CTL_SEN_MODE_3;
				else
					data->mode = CTL_SEN_MODE_1;
			}
		}
	} else if (data->frame_num == 2) {
		if ((data->size.w <= 3840) && (data->size.h <= 2160)) {
			if (data->frame_rate <= 3000) {
				data->mode = CTL_SEN_MODE_2;
			}
		}	
	}

	if (data->mode == CTL_SEN_MODE_UNKNOWN) {
		DBG_ERR("fail (frame_rate=%d,size=%d*%d,if_type=%d,data_fmt=%d,frame_num=%d,data_lane=%d,mode_type_sel=%llx,pixdepth=%lld) \r\n"
			, data->frame_rate, data->size.w, data->size.h, data->if_type, data->data_fmt, data->frame_num, data->data_lane, (UINT64)data->mode_type_sel,(UINT64)data->pixdepth);
		data->mode = CTL_SEN_MODE_1;
	}
	else {
		// for debug, change DBG_MSG to DBG_ERR
		DBG_MSG("mode=%d (frame_rate=%d,size=%d*%d,if_type=%d,data_fmt=%d,frame_num=%d,data_lane=%d,mode_type_sel=%llx,pixdepth=%lld) \r\n"
			, data->mode, data->frame_rate, data->size.w, data->size.h, data->if_type, data->data_fmt, data->frame_num, data->data_lane, (UINT64)data->mode_type_sel,(UINT64)data->pixdepth);
	}
}

static UINT32 sen_calc_rowtime_step_gc8613(CTL_SEN_ID id, CTL_SEN_MODE mode)
{
	UINT32 div_step = 1;

	if (mode >= SEN_MAX_MODE) {
		mode = cur_sen_mode[id];
	}

	if (mode_basic_param[mode].mode_type == CTL_SEN_MODE_LINEAR) {
		div_step = 1;
	}

	return div_step;
}

static UINT32 sen_calc_rowtime_gc8613(CTL_SEN_ID id, CTL_SEN_MODE mode)
{
	UINT32 row_time = 0;

	if (mode >= SEN_MAX_MODE) {
		mode = cur_sen_mode[id];
	}

	// precision * 100
	row_time = 100 * mode_basic_param[mode].signal_info.hd_period / (speed_param[mode].pclk / 100000);

	return row_time;
}

static void sen_get_rowtime_gc8613(CTL_SEN_ID id, CTL_SENDRV_GET_MODE_ROWTIME_PARAM *data)
{
	data->row_time_step = sen_calc_rowtime_step_gc8613(id, data->mode);	
	data->row_time = sen_calc_rowtime_gc8613(id, data->mode) * (data->row_time_step);
}

static void sen_set_cur_fps_gc8613(CTL_SEN_ID id, UINT32 fps)
{
	cur_fps[id] = fps;
}

static UINT32 sen_get_cur_fps_gc8613(CTL_SEN_ID id)
{
	return cur_fps[id];
}

static void sen_set_chgmode_fps_gc8613(CTL_SEN_ID id, UINT32 fps)
{
	chgmode_fps[id] = fps;
}

static UINT32 sen_get_chgmode_fps_gc8613(CTL_SEN_ID id)
{
	return chgmode_fps[id];
}

#if defined(__FREERTOS)
void sen_get_i2c_id_gc8613(CTL_SEN_ID id, UINT32 *i2c_id)
{
	*i2c_id = sen_i2c[id].id;
}

void sen_get_i2c_addr_gc8613(CTL_SEN_ID id, UINT32 *i2c_addr)
{
	*i2c_addr = sen_i2c[id].addr;
}

int sen_init_gc8613(SENSOR_DTSI_INFO *info)
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

	sprintf(compatible, "nvt,sen_gc8613");
	if (sen_common_check_compatible(compatible)) {
		DBG_DUMP("compatible valid, using peri-dev.dtsi \r\n");
		sen_common_load_cfg_preset_compatible(compatible, &sen_preset);
		sen_common_load_cfg_direction_compatible(compatible, &sen_direction);
		sen_common_load_cfg_power_compatible(compatible, &sen_power);
		sen_common_load_cfg_i2c_compatible(compatible, &sen_i2c);
	} else if (info->addr != NULL) {
		DBG_DUMP("compatible not valid, using sensor.dtsi \r\n");
		sprintf(node_path, "/sensor/sen_cfg/sen_gc8613");
		sen_common_load_cfg_map(info->addr, node_path, &sen_map);
		sen_common_load_cfg_preset(info->addr, node_path, &sen_preset);
		sen_common_load_cfg_direction(info->addr, node_path, &sen_direction);
		sen_common_load_cfg_power(info->addr, node_path, &sen_power);
		sen_common_load_cfg_i2c(info->addr, node_path, &sen_i2c);
	} else {
		DBG_WRN("DTSI addr is NULL \r\n");
	}

	memset((void *)(&reg_obj), 0, sizeof(CTL_SEN_REG_OBJ));
	reg_obj.pwr_ctrl = sen_pwr_ctrl_gc8613;
	reg_obj.det_plug_in = NULL;
	reg_obj.drv_tab = sen_get_drv_tab_gc8613();
	rt = ctl_sen_reg_sendrv("nvt_sen_gc8613", &reg_obj);
	if (rt != E_OK) {
		DBG_WRN("register sensor driver fail \r\n");
	}

	return rt;
}

void sen_exit_gc8613(void)
{
	ctl_sen_unreg_sendrv("nvt_sen_gc8613");
}

#else
static int __init sen_init_gc8613(void)
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
	reg_obj.pwr_ctrl = sen_pwr_ctrl_gc8613;
	reg_obj.det_plug_in = NULL;
	reg_obj.drv_tab = sen_get_drv_tab_gc8613();
	rt = ctl_sen_reg_sendrv("nvt_sen_gc8613", &reg_obj);

	if (rt != E_OK) {
		DBG_WRN("register sensor driver fail \r\n");
	}

	return rt;
}

static void __exit sen_exit_gc8613(void)
{
	ctl_sen_unreg_sendrv("nvt_sen_gc8613");
}

module_init(sen_init_gc8613);
module_exit(sen_exit_gc8613);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION(SEN_GC8613_MODULE_NAME);
MODULE_LICENSE("GPL");
#endif

