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
VOS_MODULE_VERSION(nvt_sen_imx226, 1, 1, 000, 00);

//=============================================================================
// information
//=============================================================================
#define SEN_IMX226_MODULE_NAME     "sen_imx226"
#define SEN_IMX226_MODULE_NAME_REG "nvt_sen_imx226"
#define SEN_MAX_MODE               2
#define MAX_VD_PERIOD              0xFFFF
#define MAX_EXPOSURE_LINE          0xFFFF
#define HD_SYNC                    8
#define VD_SYNC                    8//3119
#define SPI_WRITE_CHIP_ID          0x81
#define SIF_CHANNEL                SIF_CH0//SIF_CH3
#define SIF_VD_SYNC                1

#if (SIF_VD_SYNC)
#define GAIN_REG_NUM               7
#endif
#define TRANS_PROTOCOL_SIF 1
#define TRANS_PROTOCOL_SPI 0
#define TRANS_PROTOCOL TRANS_PROTOCOL_SIF

#if (TRANS_PROTOCOL == TRANS_PROTOCOL_SIF)
#else
#ifdef __KERNEL__
#include "sen_spi.c"
#endif
#endif
//=============================================================================
// function declaration
//=============================================================================
static CTL_SEN_DRV_TAB *sen_get_drv_tab_imx226(void);
static void sen_pwr_ctrl_imx226(CTL_SEN_ID id, CTL_SEN_PWR_CTRL_FLAG flag, CTL_SEN_CLK_CB clk_cb);
static ER sen_open_imx226(CTL_SEN_ID id);
static ER sen_close_imx226(CTL_SEN_ID id);
static ER sen_sleep_imx226(CTL_SEN_ID id);
static ER sen_wakeup_imx226(CTL_SEN_ID id);
static ER sen_write_reg_imx226(CTL_SEN_ID id, CTL_SEN_CMD *cmd);
static ER sen_read_reg_imx226(CTL_SEN_ID id, CTL_SEN_CMD *cmd);
static ER sen_chg_mode_imx226(CTL_SEN_ID id, CTL_SENDRV_CHGMODE_OBJ chgmode_obj);
static ER sen_chg_fps_imx226(CTL_SEN_ID id, UINT32 fps);
static ER sen_set_info_imx226(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data);
static ER sen_get_info_imx226(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data);
static UINT32 sen_calc_chgmode_vd_imx226(CTL_SEN_ID id, UINT32 fps);
static UINT32 sen_calc_exp_vd_imx226(CTL_SEN_ID id, UINT32 fps);
static void sen_set_gain_imx226(CTL_SEN_ID id, void *param);
static void sen_set_expt_imx226(CTL_SEN_ID id, void *param);
static void sen_set_preset_imx226(CTL_SEN_ID id, ISP_SENSOR_PRESET_CTRL *ctrl);
static void sen_set_flip_imx226(CTL_SEN_ID id, CTL_SEN_FLIP *flip);
static ER sen_get_flip_imx226(CTL_SEN_ID id, CTL_SEN_FLIP *flip);
#if defined(__FREERTOS)
void sen_get_gain_imx226(CTL_SEN_ID id, void *param);
void sen_get_expt_imx226(CTL_SEN_ID id, void *param);
#else
static void sen_get_gain_imx226(CTL_SEN_ID id, void *param);
static void sen_get_expt_imx226(CTL_SEN_ID id, void *param);
#endif
static void sen_get_min_expt_imx226(CTL_SEN_ID id, void *param);
static void sen_get_mode_basic_imx226(CTL_SENDRV_GET_MODE_BASIC_PARAM *mode_basic);
static void sen_get_attr_basic_imx226(CTL_SEN_ID id, CTL_SENDRV_GET_ATTR_BASIC_PARAM *data);
static void sen_get_attr_signal_imx226(CTL_SENDRV_GET_ATTR_SIGNAL_PARAM *data);
static ER sen_get_attr_cmdif_imx226(CTL_SEN_ID id, CTL_SENDRV_GET_ATTR_CMDIF_PARAM *data);
static ER sen_get_attr_if_imx226(CTL_SENDRV_GET_ATTR_IF_PARAM *data);
static void sen_get_fps_imx226(CTL_SEN_ID id, CTL_SENDRV_GET_FPS_PARAM *data);
static void sen_get_speed_imx226(CTL_SEN_ID id, CTL_SENDRV_GET_SPEED_PARAM *data);
static void sen_get_mode_lvds_imx226(CTL_SENDRV_GET_MODE_LVDS_PARAM *data);
static void sen_ext_get_mode_tge_imx226(CTL_SENDRV_GET_MODE_TGE_PARAM *data);
static void sen_get_modesel_imx226(CTL_SENDRV_GET_MODESEL_PARAM *data);
static UINT32 sen_calc_rowtime_imx226(CTL_SEN_ID id, CTL_SEN_MODE mode);
static UINT32 sen_calc_rowtime_step_imx226(CTL_SEN_ID id, CTL_SEN_MODE mode);
static void sen_get_rowtime_imx226(CTL_SEN_ID id, CTL_SENDRV_GET_MODE_ROWTIME_PARAM *data);
static void sen_set_cur_fps_imx226(CTL_SEN_ID id, UINT32 fps);
static UINT32 sen_get_cur_fps_imx226(CTL_SEN_ID id);
static void sen_set_chgmode_fps_imx226(CTL_SEN_ID id, UINT32 fps);
static UINT32 sen_get_chgmode_fps_imx226(CTL_SEN_ID id);
static ER sen_get_min_shr_imx226(CTL_SEN_MODE mode, UINT32 *min_shr, UINT32 *min_exp);
#if (SIF_VD_SYNC)
static ER sen_write_reg_sync_vd_imx226(CTL_SEN_ID id, CTL_SEN_CMD *cmd);
#endif
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

#if (TRANS_PROTOCOL == TRANS_PROTOCOL_SIF)
#else
#if defined(__KERNEL__)
static SEN_SPI sen_spi[CTL_SEN_ID_MAX] = {
	{SEN_SPI_ID_1, 10000000, SEN_SPI_FMT_24BIT, SPI_MODE_3 | SPI_LSB_FIRST},
	{SEN_SPI_ID_2, 10000000, SEN_SPI_FMT_24BIT, SPI_MODE_3 | SPI_LSB_FIRST},
	{SEN_SPI_ID_2, 10000000, SEN_SPI_FMT_24BIT, SPI_MODE_3 | SPI_LSB_FIRST},
	{SEN_SPI_ID_1, 10000000, SEN_SPI_FMT_24BIT, SPI_MODE_3 | SPI_LSB_FIRST},
	{SEN_SPI_ID_1, 10000000, SEN_SPI_FMT_24BIT, SPI_MODE_3 | SPI_LSB_FIRST},
	{SEN_SPI_ID_1, 10000000, SEN_SPI_FMT_24BIT, SPI_MODE_3 | SPI_LSB_FIRST},
	{SEN_SPI_ID_1, 10000000, SEN_SPI_FMT_24BIT, SPI_MODE_3 | SPI_LSB_FIRST},
	{SEN_SPI_ID_1, 10000000, SEN_SPI_FMT_24BIT, SPI_MODE_3 | SPI_LSB_FIRST}
};
#endif
#endif

static CTL_SENDRV_GET_ATTR_BASIC_PARAM basic_param = {
	SEN_IMX226_MODULE_NAME,
	CTL_SEN_VENDOR_SONY,
	SEN_MAX_MODE,
	CTL_SEN_SUPPORT_PROPERTY_FLIP|CTL_SEN_SUPPORT_PROPERTY_CHGFPS,
#if (SIF_VD_SYNC)	
	0
#else 
	1
#endif		
};

static CTL_SENDRV_GET_ATTR_SIGNAL_PARAM signal_param = {
	CTL_SEN_SIGNAL_SLAVE,
	{CTL_SEN_ACTIVE_HIGH, CTL_SEN_ACTIVE_HIGH, CTL_SEN_PHASE_RISING, CTL_SEN_PHASE_RISING, CTL_SEN_PHASE_RISING}
};

static CTL_SENDRV_I2C i2c = { //dummy
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
		72000000,
		72000000,
		192000000
	},
	{
		CTL_SEN_MODE_2,
		CTL_SEN_SIEMCLK_SRC_DFT,
		72000000,
		72000000,
		230400000,
	}	
};

static CTL_SENDRV_GET_MODE_LVDS_PARAM lvds_param[SEN_MAX_MODE] = {
	{
		CTL_SEN_MODE_1,
		CTL_SEN_CLKLANE_1,
		CTL_SEN_DATALANE_8,
		{ 0, 1, 2, 3, 4, 5, 6, 7},
		{0, 0, 0, 0},
		SEN_BIT_OFS_NONE,
		0,
		CTL_SEN_DATAIN_BIT_ORDER_LSB,
		0,
	},
	{
		CTL_SEN_MODE_2,
		CTL_SEN_CLKLANE_1,
		CTL_SEN_DATALANE_8,
		{ 0, 1, 2, 3, 4, 5, 6, 7},
		{0, 0, 0, 0},
		SEN_BIT_OFS_NONE,
		0,
		CTL_SEN_DATAIN_BIT_ORDER_LSB,
		0,
	}	
};

static CTL_SENDRV_GET_MODE_BASIC_PARAM mode_basic_param[SEN_MAX_MODE] = {
	{
		CTL_SEN_MODE_1,
		CTL_SEN_IF_TYPE_LVDS,
		CTL_SEN_DATA_FMT_RGB,
		CTL_SEN_MODE_LINEAR,
		2797,
		1,
		CTL_SEN_STPIX_R,
		CTL_SEN_PIXDEPTH_12BIT,
		CTL_SEN_FMT_POGRESSIVE,
		{4168, 3062},
		{{132, 40, 4000, 3000}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
		{4000, 3000},
		{0, 825, 0, 3120},
		CTL_SEN_RATIO(4, 3),
		{1000, 179040},
		100
	},
	{
		CTL_SEN_MODE_2,
		CTL_SEN_IF_TYPE_LVDS,
		CTL_SEN_DATA_FMT_RGB,
		CTL_SEN_MODE_LINEAR,
		3297,
		1,
		CTL_SEN_STPIX_R,
		CTL_SEN_PIXDEPTH_10BIT,
		CTL_SEN_FMT_POGRESSIVE,
		{4168, 3062},
		{{132, 40, 4000, 3000}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
		{4000, 3000},
		{0, 700, 0, 3120},
		CTL_SEN_RATIO(4, 3),
		{1000, 179040},
		100
	}
};

static CTL_SEN_CMD imx226_mode_1[] = {
	{0x0000, 1, {0x06, 0x0}},	
	{0x0001, 1, {0x00, 0x0}},
	{0x0002, 1, {0x00, 0x0}},
	{0x0003, 1, {0x11, 0x0}},
	{0x0004, 1, {0x00, 0x0}},
	{0x0005, 1, {0x07, 0x0}},
	{0x0006, 1, {0x00, 0x0}},
	{0x0007, 1, {0x00, 0x0}},
	{0x0008, 1, {0x00, 0x0}},
	{0x0012, 1, {0x00, 0x0}},
	{0x001A, 1, {0x00, 0x0}},
	{0x0026, 1, {0x74, 0x0}},
	{0x0027, 1, {0x74, 0x0}},
	{0x0028, 1, {0x74, 0x0}},
	{0x004D, 1, {0x10, 0x0}},
	{0x0054, 1, {0x04, 0x0}},
	{0x0057, 1, {0x24, 0x0}},
	{0x007E, 1, {0x20, 0x0}},
	{0x007F, 1, {0x01, 0x0}},
	{0x0080, 1, {0x00, 0x0}},
	{0x0081, 1, {0x00, 0x0}},
	{0x0082, 1, {0x00, 0x0}},
	{0x0083, 1, {0x00, 0x0}},
	{0x0084, 1, {0x00, 0x0}},
	{0x0085, 1, {0x00, 0x0}},
	{0x0086, 1, {0x00, 0x0}},
	{0x0087, 1, {0x00, 0x0}},
	{0x0095, 1, {0x00, 0x0}},
	{0x0096, 1, {0x00, 0x0}},
	{0x0097, 1, {0x00, 0x0}},
	{0x0098, 1, {0x00, 0x0}},
	{0x0099, 1, {0x00, 0x0}},
	{0x009A, 1, {0x00, 0x0}},
	{0x009B, 1, {0x00, 0x0}},
	{0x009C, 1, {0x00, 0x0}},
	{0x00B6, 1, {0x47, 0x0}},
	{0x00B7, 1, {0x00, 0x0}},
	{0x00B8, 1, {0x00, 0x0}},
	{0x00B9, 1, {0x00, 0x0}},
	{0x00BA, 1, {0x00, 0x0}},
	{0x00BB, 1, {0x00, 0x0}},
	{0x00BC, 1, {0x00, 0x0}},
	{0x00BD, 1, {0x00, 0x0}},
	{0x00BE, 1, {0x00, 0x0}},
	{0x00BF, 1, {0x00, 0x0}},
	{0x00C0, 1, {0x00, 0x0}},
	{0x00C1, 1, {0x00, 0x0}},
	{0x00C2, 1, {0x00, 0x0}},
	{0x00C3, 1, {0x00, 0x0}},
	{0x00C4, 1, {0x00, 0x0}},
	{0x00C5, 1, {0x00, 0x0}},
	{0x00C6, 1, {0x00, 0x0}},
	{0x00C7, 1, {0x00, 0x0}},
	{0x00C8, 1, {0x00, 0x0}},
	{0x00C9, 1, {0x00, 0x0}},
	{0x00CA, 1, {0x00, 0x0}},
	{0x00CB, 1, {0x00, 0x0}},
	{0x00CC, 1, {0x00, 0x0}},
	{0x00CE, 1, {0x00, 0x0}},   
	{0x0210, 1, {0x0A, 0x0}},
	{0x0211, 1, {0x00, 0x0}},
	{0x0212, 1, {0x31, 0x0}},
	{0x0213, 1, {0x01, 0x0}},
	{0x021C, 1, {0x64, 0x0}},
	{0x021D, 1, {0x00, 0x0}},
	{0x021E, 1, {0xEF, 0x0}},
	{0x021F, 1, {0x00, 0x0}},
	{0x0222, 1, {0x30, 0x0}},
	{0x0223, 1, {0x01, 0x0}},
	{0x0313, 1, {0x10, 0x0}},
	{0x0352, 1, {0x3C, 0x0}},
	{0x0353, 1, {0x00, 0x0}},
	{0x0356, 1, {0x3B, 0x0}},
	{0x0357, 1, {0x00, 0x0}},
	{0x0366, 1, {0x01, 0x0}},
	{0x0371, 1, {0x01, 0x0}},
	{0x0528, 1, {0x0F, 0x0}},
	{0x0529, 1, {0x0F, 0x0}},
	{0x052C, 1, {0x1F, 0x0}},
	{0x052D, 1, {0x01, 0x0}},
	{0x052E, 1, {0x04, 0x0}},
	{0x0534, 1, {0x10, 0x0}},
	{0x057A, 1, {0x0B, 0x0}},
	{0x057B, 1, {0x00, 0x0}},
	{0x057D, 1, {0x0A, 0x0}},
	{0x057E, 1, {0x00, 0x0}},
	{0x0582, 1, {0x07, 0x0}},
	{0x0617, 1, {0x10, 0x0}},
	{0x0650, 1, {0x00, 0x0}},
	{0x065C, 1, {0x07, 0x0}},
	{0x0700, 1, {0x19, 0x0}},
	{0x0701, 1, {0x19, 0x0}},
	{SEN_CMD_SETVD, 1, {0x0, 0x0}},
	{SEN_CMD_PRESET, 1, {0x0, 0x0}},
	{SEN_CMD_DIRECTION, 1, {0x0, 0x0}},
	{SEN_CMD_DELAY, 1, { 1, 0x0}},
	{0x0000, 1, {0x04, 0x0}},
	{SEN_CMD_DELAY, 1, { 20, 0x0}},
	{0x0001, 1, {0x11, 0x0}},
};

static CTL_SEN_CMD imx226_mode_2[] = {
	{0x0000, 1, {0x06, 0x0}},
	{0x0001, 1, {0x00, 0x0}},
	{0x0002, 1, {0x00, 0x0}},
	{0x0003, 1, {0x11, 0x0}},
	{0x0004, 1, {0x00, 0x0}},
	{0x0005, 1, {0x01, 0x0}},
	{0x0006, 1, {0x00, 0x0}},
	{0x0007, 1, {0x00, 0x0}},
	{0x0008, 1, {0x00, 0x0}},
	{0x0012, 1, {0x00, 0x0}},
	{0x001A, 1, {0x00, 0x0}},
	{0x0026, 1, {0x74, 0x0}},
	{0x0027, 1, {0x74, 0x0}},
	{0x0028, 1, {0x74, 0x0}},
	{0x004D, 1, {0x10, 0x0}},
	{0x0054, 1, {0x04, 0x0}},
	{0x0057, 1, {0x24, 0x0}},
	{0x007E, 1, {0x20, 0x0}},
	{0x007F, 1, {0x01, 0x0}},
	{0x0080, 1, {0x00, 0x0}},
	{0x0081, 1, {0x00, 0x0}},
	{0x0082, 1, {0x00, 0x0}},
	{0x0083, 1, {0x00, 0x0}},
	{0x0084, 1, {0x00, 0x0}},
	{0x0085, 1, {0x00, 0x0}},
	{0x0086, 1, {0x00, 0x0}},
	{0x0087, 1, {0x00, 0x0}},
	{0x0095, 1, {0x00, 0x0}},
	{0x0096, 1, {0x00, 0x0}},
	{0x0097, 1, {0x00, 0x0}},
	{0x0098, 1, {0x00, 0x0}},
	{0x0099, 1, {0x00, 0x0}},
	{0x009A, 1, {0x00, 0x0}},
	{0x009B, 1, {0x00, 0x0}},
	{0x009C, 1, {0x00, 0x0}},
	{0x00B6, 1, {0x47, 0x0}},
	{0x00B7, 1, {0x00, 0x0}},
	{0x00B8, 1, {0x00, 0x0}},
	{0x00B9, 1, {0x00, 0x0}},
	{0x00BA, 1, {0x00, 0x0}},
	{0x00BB, 1, {0x00, 0x0}},
	{0x00BC, 1, {0x00, 0x0}},
	{0x00BD, 1, {0x00, 0x0}},
	{0x00BE, 1, {0x00, 0x0}},
	{0x00BF, 1, {0x00, 0x0}},
	{0x00C0, 1, {0x00, 0x0}},
	{0x00C1, 1, {0x00, 0x0}},
	{0x00C2, 1, {0x00, 0x0}},
	{0x00C3, 1, {0x00, 0x0}},
	{0x00C4, 1, {0x00, 0x0}},
	{0x00C5, 1, {0x00, 0x0}},
	{0x00C6, 1, {0x00, 0x0}},
	{0x00C7, 1, {0x00, 0x0}},
	{0x00C8, 1, {0x00, 0x0}},
	{0x00C9, 1, {0x00, 0x0}},
	{0x00CA, 1, {0x00, 0x0}},
	{0x00CB, 1, {0x00, 0x0}},
	{0x00CC, 1, {0x00, 0x0}},
	{0x00CE, 1, {0x00, 0x0}},   
	{0x0210, 1, {0x0A, 0x0}},
	{0x0211, 1, {0x00, 0x0}},
	{0x0212, 1, {0x31, 0x0}},
	{0x0213, 1, {0x01, 0x0}},
	{0x021C, 1, {0x64, 0x0}},
	{0x021D, 1, {0x00, 0x0}},
	{0x021E, 1, {0xEF, 0x0}},
	{0x021F, 1, {0x00, 0x0}},
	{0x0222, 1, {0x30, 0x0}},
	{0x0223, 1, {0x01, 0x0}},
	{0x0313, 1, {0x10, 0x0}},
	{0x0352, 1, {0x3C, 0x0}},
	{0x0353, 1, {0x00, 0x0}},
	{0x0356, 1, {0x3B, 0x0}},
	{0x0357, 1, {0x00, 0x0}},
	{0x0366, 1, {0x01, 0x0}},
	{0x0371, 1, {0x01, 0x0}},
	{0x0528, 1, {0x0F, 0x0}},
	{0x0529, 1, {0x0F, 0x0}},
	{0x052C, 1, {0x1F, 0x0}},
	{0x052D, 1, {0x01, 0x0}},
	{0x052E, 1, {0x04, 0x0}},
	{0x0534, 1, {0x10, 0x0}},
	{0x057A, 1, {0x0B, 0x0}},
	{0x057B, 1, {0x00, 0x0}},
	{0x057D, 1, {0x0A, 0x0}},
	{0x057E, 1, {0x00, 0x0}},
	{0x0582, 1, {0x07, 0x0}},
	{0x0617, 1, {0x10, 0x0}},
	{0x0650, 1, {0x00, 0x0}},
	{0x065C, 1, {0x07, 0x0}},
	{0x0700, 1, {0x19, 0x0}},
	{0x0701, 1, {0x19, 0x0}},
	{SEN_CMD_SETVD, 1, {0x0, 0x0}},
	{SEN_CMD_PRESET, 1, {0x0, 0x0}},
	{SEN_CMD_DIRECTION, 1, {0x0, 0x0}},
	{SEN_CMD_DELAY, 1, { 1, 0x0}},
	{0x0000, 1, {0x04, 0x0}},
	{SEN_CMD_DELAY, 1, { 20, 0x0}},
	{0x0001, 1, {0x11, 0x0}},	
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
static UINT32 rotate_reg[CTL_SEN_ID_MAX]= {0};
static UINT32 first_exp_flag[CTL_SEN_ID_MAX] = {0};
#if (TRANS_PROTOCOL == TRANS_PROTOCOL_SIF)
#if (SIF_VD_SYNC)
static _ALIGNED(4) UINT32 sif_buf[GAIN_REG_NUM] = {0};
#endif
#endif

#if (TRANS_PROTOCOL == TRANS_PROTOCOL_SIF)
#else
#if defined(__KERNEL__)
static BOOL spi_valid[CTL_SEN_ID_MAX];
#endif
#endif

static CTL_SEN_DRV_TAB imx226_sen_drv_tab = {
	sen_open_imx226,
	sen_close_imx226,
	sen_sleep_imx226,
	sen_wakeup_imx226,
	sen_write_reg_imx226,
	sen_read_reg_imx226,
	sen_chg_mode_imx226,
	sen_chg_fps_imx226,
	sen_set_info_imx226,
	sen_get_info_imx226,
};

static CTL_SEN_DRV_TAB *sen_get_drv_tab_imx226(void)
{
	return &imx226_sen_drv_tab;
}

static void sen_pwr_ctrl_imx226(CTL_SEN_ID id, CTL_SEN_PWR_CTRL_FLAG flag, CTL_SEN_CLK_CB clk_cb)
{
	UINT32 i = 0;
	UINT32 reset_count = 0, pwdn_count = 0;
	DBG_IND("enter flag %d \r\n", flag);

	if (flag == CTL_SEN_PWR_CTRL_TURN_ON) {
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

		#if (TRANS_PROTOCOL == TRANS_PROTOCOL_SIF)
		sif_open(SIF_CHANNEL);
		sif_setConfig(SIF_CHANNEL, SIF_CONFIG_ID_BUSMODE,  SIF_MODE_00);
		//  Input Soure of SIE VD channel 1~8
		#if (SIF_VD_SYNC)
		sif_setConfig(SIF_CHANNEL, SIF_CONFIG_ID_BURST_SRC , SIF_VD_SIE3);
		#endif
		// Bus clock = 36 MHz
		sif_setConfig(SIF_CHANNEL, SIF_CONFIG_ID_BUSCLOCK, 36000000);
		// 0 half bus clock = 0 ns
		sif_setConfig(SIF_CHANNEL, SIF_CONFIG_ID_SENS,     20);
		// 0 half bus clock = 0 ns
		sif_setConfig(SIF_CHANNEL, SIF_CONFIG_ID_SENH,     20);
		// 16 bits per transmission
		sif_setConfig(SIF_CHANNEL, SIF_CONFIG_ID_LENGTH,   32);
		// MSb shift out first
		sif_setConfig(SIF_CHANNEL, SIF_CONFIG_ID_DIR,      SIF_DIR_LSB);
		#endif

	}

	if (flag == CTL_SEN_PWR_CTRL_TURN_OFF) {
		
		#if (TRANS_PROTOCOL == TRANS_PROTOCOL_SIF)
		sif_close(SIF_CHANNEL);
		#endif

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
	}
}

static CTL_SEN_CMD sen_set_cmd_info_imx226(UINT32 addr, UINT32 data_length, UINT32 data0, UINT32 data1)
{
	CTL_SEN_CMD cmd;

	cmd.addr = addr;
	cmd.data_len = data_length;
	cmd.data[0] = data0;
	cmd.data[1] = data1;
	return cmd;
}

static ER sen_open_imx226(CTL_SEN_ID id)
{
	ER rt = E_OK;

	preset_ctrl[id].mode = ISP_SENSOR_PRESET_DEFAULT;

	#if (TRANS_PROTOCOL == TRANS_PROTOCOL_SPI)
	#if defined(__KERNEL__)
	spi_valid[id] = TRUE;
	rt = sen_spi_init_driver(id, &sen_spi[id]);
	if (rt != E_OK) {
		spi_valid[id] = FALSE;
		DBG_ERR("init. spi driver fail (%d) \r\n", id);
	}
	#else
	DBG_ERR("RTOS not support spi driver \r\n");
	#endif
	#endif

	return rt;
}

static ER sen_close_imx226(CTL_SEN_ID id)
{
	#if (TRANS_PROTOCOL == TRANS_PROTOCOL_SPI)
	#if defined(__KERNEL__)
	if (spi_valid[id]) {
		sen_spi_remove_driver(id);
	}
	spi_valid[id] = FALSE;
	
	#else
	DBG_ERR("RTOS not support spi driver \r\n");
	#endif
	#endif

	return E_OK;
}

static ER sen_sleep_imx226(CTL_SEN_ID id)
{
	DBG_IND("enter \r\n");
	return E_OK;
}

static ER sen_wakeup_imx226(CTL_SEN_ID id)
{
	DBG_IND("enter \r\n");
	return E_OK;
}

static ER sen_write_reg_imx226(CTL_SEN_ID id, CTL_SEN_CMD *cmd)
{

	#if (TRANS_PROTOCOL == TRANS_PROTOCOL_SIF)
	unsigned char buf[4];

	buf[0]     = SPI_WRITE_CHIP_ID;
	buf[1]     = (cmd->addr >> 8) & 0xFF;
	buf[2]     = cmd->addr & 0xFF;
	buf[3]     = cmd->data[0] & 0xFF;
	
	sif_send(SIF_CHANNEL, ((buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | (buf[0])), 0, 0);

	return E_OK;

	#else
	#if defined(__KERNEL__)
	struct spi_msg msgs;
	unsigned char buf[4];

	if (!spi_valid[id]) {
		return E_NOSPT;
	}

	buf[0]     = SPI_WRITE_CHIP_ID;
	buf[1]     = (cmd->addr >> 8) & 0xFF;
	buf[2]     = cmd->addr & 0xFF;
	buf[3]     = cmd->data[0] & 0xFF;
	msgs.len   = 4;
	msgs.buf   = buf;

	if (sen_spi_transfer(id, &msgs, 1) != 0) {
		return E_SYS;
	}

	return E_OK;
	#else
	return E_NOSPT;
	#endif
	#endif
}

#if (TRANS_PROTOCOL == TRANS_PROTOCOL_SIF)
#if (SIF_VD_SYNC)
static ER sen_write_reg_sync_vd_imx226(CTL_SEN_ID id, CTL_SEN_CMD *cmd)
{
	UINT32 i;
	ER rt = E_OK;
		
	for (i = 0; i < GAIN_REG_NUM; i++) {
		sif_buf[i] = ((cmd[i].data[0] & 0xFF) << 24) | ((cmd[i].addr & 0xFF) << 16) | (((cmd[i].addr >> 8) & 0xFF) << 8) | (SPI_WRITE_CHIP_ID);
	}

	sif_send_burst_DMA(SIF_CHANNEL, (ULONG)&sif_buf[0], 1, GAIN_REG_NUM * 4, 0, 0);

	return rt;
}
#endif
#endif

static ER sen_read_reg_imx226(CTL_SEN_ID id, CTL_SEN_CMD *cmd)
{
	#if (TRANS_PROTOCOL == TRANS_PROTOCOL_SIF)
	DBG_ERR("imx226 can not support read reg \r\n");
	cmd->data[0] = 0x00;
	return E_OK;
	#else
	#if defined(__KERNEL__)
	DBG_ERR("imx226 can not support read reg \r\n");
	cmd->data[0] = 0x00;
	return E_OK;
	#else
	return E_NOSPT;
	#endif
	#endif
}

static UINT32 sen_get_cmd_tab_imx226(CTL_SEN_MODE mode, CTL_SEN_CMD **cmd_tab)
{
	switch (mode) {
	case CTL_SEN_MODE_1:
		*cmd_tab = imx226_mode_1;
		return sizeof(imx226_mode_1) / sizeof(CTL_SEN_CMD);

	case CTL_SEN_MODE_2:
		*cmd_tab = imx226_mode_2;
		return sizeof(imx226_mode_2) / sizeof(CTL_SEN_CMD);

	default:
		DBG_ERR("sensor mode %d no cmd table\r\n", mode);
		*cmd_tab = NULL;
		return 0;
	}
}

static ER sen_chg_mode_imx226(CTL_SEN_ID id, CTL_SENDRV_CHGMODE_OBJ chgmode_obj)
{
	ISP_SENSOR_CTRL sensor_ctrl = {0};
	CTL_SENDRV_GET_MODE_TGE_PARAM tge_info;
	CTL_SEN_CMD *p_cmd_list = NULL, cmd;
	CTL_SEN_FLIP flip = CTL_SEN_FLIP_NONE;
	UINT32 sensor_vd;
	UINT32 idx, cmd_num = 0, new_tge_vd = 0, new_tge_vd_sync = 0;
	ER rt = E_OK;

	cur_sen_mode[id] = chgmode_obj.mode;

	if (is_fastboot[id]) {
		#if defined(__KERNEL__)
		ISP_BUILTIN_SENSOR_CTRL *p_sensor_ctrl_temp;

		p_sensor_ctrl_temp = isp_builtin_get_sensor_gain(id);
		sensor_ctrl.gain_ratio[0] = p_sensor_ctrl_temp->gain_ratio[0];
		sensor_ctrl.gain_ratio[1] = p_sensor_ctrl_temp->gain_ratio[1];
		sensor_ctrl.gain_ratio[2] = p_sensor_ctrl_temp->gain_ratio[2];		
		p_sensor_ctrl_temp = isp_builtin_get_sensor_expt(id);
		sensor_ctrl.exp_time[0] = p_sensor_ctrl_temp->exp_time[0];
		sensor_ctrl.exp_time[1] = p_sensor_ctrl_temp->exp_time[1];
		sensor_ctrl.exp_time[2] = p_sensor_ctrl_temp->exp_time[2];		
		sen_set_chgmode_fps_imx226(id, isp_builtin_get_chgmode_fps(id));
		sen_set_cur_fps_imx226(id, isp_builtin_get_chgmode_fps(id));
		sen_set_gain_imx226(id, &sensor_ctrl);
		sen_set_expt_imx226(id, &sensor_ctrl);
		#endif
		preset_ctrl[id].mode = ISP_SENSOR_PRESET_CHGMODE;

		return E_OK;
	}

	// get & set sensor cmd table
	cmd_num = sen_get_cmd_tab_imx226(chgmode_obj.mode, &p_cmd_list);
	if (p_cmd_list == NULL) {
		DBG_ERR("%s: SenMode(%d) out of range!!! \r\n", __func__, chgmode_obj.mode);
		return E_SYS;
	}

	memset((void *)&tge_info, 0, sizeof(CTL_SENDRV_GET_MODE_TGE_PARAM));
	tge_info.mode = cur_sen_mode[id];	
	sen_ext_get_mode_tge_imx226(&tge_info);

	if (basic_param.property & CTL_SEN_SUPPORT_PROPERTY_CHGFPS) {
		sensor_vd = sen_calc_chgmode_vd_imx226(id, chgmode_obj.frame_rate);
		new_tge_vd_sync = (mode_basic_param[cur_sen_mode[id]].dft_fps) * (tge_info.signal.vd_period) / sen_get_chgmode_fps_imx226(id) + VD_SYNC - (tge_info.signal.vd_period);
		new_tge_vd =(mode_basic_param[cur_sen_mode[id]].dft_fps) * (tge_info.signal.vd_period) / sen_get_chgmode_fps_imx226(id);
		ctl_sen_set(ctl_sen_get_senid(id, 0, SEN_IMX226_MODULE_NAME_REG), CTL_SEN_CFGID_TGE_VD_SYNC, (void *)(&new_tge_vd_sync));	
		ctl_sen_set(ctl_sen_get_senid(id, 0, SEN_IMX226_MODULE_NAME_REG), CTL_SEN_CFGID_TGE_VD_PERIOD, (void *)(&new_tge_vd));		
		if (new_tge_vd != sensor_vd) {
			DBG_WRN(" sensor_vd = %d, new_tge_vd = %d \r\n",sensor_vd, new_tge_vd);
		}
	} else {
		DBG_WRN(" not support fps adjust \r\n");
		sen_set_cur_fps_imx226(id, mode_basic_param[cur_sen_mode[id]].dft_fps);
		sen_set_chgmode_fps_imx226(id, mode_basic_param[cur_sen_mode[id]].dft_fps);
		sensor_vd = mode_basic_param[cur_sen_mode[id]].signal_info.vd_period;
		new_tge_vd_sync = VD_SYNC;
		new_tge_vd = tge_info.signal.vd_period;
		ctl_sen_set(ctl_sen_get_senid(id, 0, SEN_IMX226_MODULE_NAME_REG), CTL_SEN_CFGID_TGE_VD_SYNC, (void *)(&new_tge_vd_sync));	
		ctl_sen_set(ctl_sen_get_senid(id, 0, SEN_IMX226_MODULE_NAME_REG), CTL_SEN_CFGID_TGE_VD_PERIOD, (void *)(&new_tge_vd));
		if (new_tge_vd != sensor_vd) {
			DBG_WRN(" sensor_vd = %d, new_tge_vd = %d \r\n",sensor_vd, new_tge_vd);
		}			
	}

	for (idx = 0; idx < cmd_num; idx++) {
		if (p_cmd_list[idx].addr == SEN_CMD_DELAY) {
			vos_util_delay_ms((p_cmd_list[idx].data[0] & 0xFF) | ((p_cmd_list[idx].data[1] & 0xFF) << 8));
		} else if (p_cmd_list[idx].addr == SEN_CMD_SETVD) {
		} else if (p_cmd_list[idx].addr == SEN_CMD_PRESET) {
			switch (preset_ctrl[id].mode) {
				default:
				case ISP_SENSOR_PRESET_DEFAULT:
					sensor_ctrl.gain_ratio[0] = sen_preset[id].gain_ratio;
					sensor_ctrl.exp_time[0] = sen_preset[id].expt_time;
					if (mode_basic_param[cur_sen_mode[id]].mode_type == CTL_SEN_MODE_STAGGER_HDR) {
						if (mode_basic_param[cur_sen_mode[id]].frame_num == 2) {
							sensor_ctrl.exp_time[1] = sen_preset[id].expt_time >> 4;
							sensor_ctrl.gain_ratio[1] = sensor_ctrl.gain_ratio[0];
						}
					}
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
			first_exp_flag[id] = 0;
			sen_set_gain_imx226(id, &sensor_ctrl);
			sen_set_expt_imx226(id, &sensor_ctrl);
			first_exp_flag[id] = 1;
		} else if (p_cmd_list[idx].addr == SEN_CMD_DIRECTION) {
			if (sen_direction[id].mirror) {
				flip |= CTL_SEN_FLIP_H;
			}
			if (sen_direction[id].flip) {
				flip |= CTL_SEN_FLIP_V;
			}
			sen_set_flip_imx226(id, &flip);
		} else {
			cmd = sen_set_cmd_info_imx226(p_cmd_list[idx].addr, p_cmd_list[idx].data_len, p_cmd_list[idx].data[0], p_cmd_list[idx].data[1]);
			rt |= sen_write_reg_imx226(id, &cmd);
		}
	}

	preset_ctrl[id].mode = ISP_SENSOR_PRESET_CHGMODE;

	if (rt != E_OK) {
		DBG_ERR("write register error %d \r\n", (INT)rt);
		return rt;
	}

	return E_OK;
}

static ER sen_chg_fps_imx226(CTL_SEN_ID id, UINT32 fps)
{
	CTL_SENDRV_GET_MODE_TGE_PARAM tge_info;
	UINT32 sensor_vd;
	UINT32 new_tge_vd = 0, new_tge_vd_sync = 0;
	ER rt = E_OK;

	memset((void *)&tge_info, 0, sizeof(CTL_SENDRV_GET_MODE_TGE_PARAM));
	tge_info.mode = cur_sen_mode[id];		
	sen_ext_get_mode_tge_imx226(&tge_info);

	if (basic_param.property & CTL_SEN_SUPPORT_PROPERTY_CHGFPS) {
		sensor_vd = sen_calc_chgmode_vd_imx226(id, fps);
		new_tge_vd_sync = (mode_basic_param[cur_sen_mode[id]].dft_fps) * (tge_info.signal.vd_period) / sen_get_chgmode_fps_imx226(id) + VD_SYNC - (tge_info.signal.vd_period);		
		new_tge_vd = (mode_basic_param[cur_sen_mode[id]].dft_fps) * (tge_info.signal.vd_period) / sen_get_chgmode_fps_imx226(id);
		ctl_sen_set(ctl_sen_get_senid(id, 0, SEN_IMX226_MODULE_NAME_REG), CTL_SEN_CFGID_TGE_VD_SYNC, (void *)(&new_tge_vd_sync));
		ctl_sen_set(ctl_sen_get_senid(id, 0, SEN_IMX226_MODULE_NAME_REG), CTL_SEN_CFGID_TGE_VD_PERIOD, (void *)(&new_tge_vd));		
		if (new_tge_vd != sensor_vd) {
			DBG_WRN(" sensor_vd = %d, new_tge_vd = %d \r\n",sensor_vd, new_tge_vd);
		}
	} else {
		DBG_WRN(" not support fps adjust \r\n");
		sen_set_cur_fps_imx226(id, mode_basic_param[cur_sen_mode[id]].dft_fps);
		sen_set_chgmode_fps_imx226(id, mode_basic_param[cur_sen_mode[id]].dft_fps);
		sensor_vd = mode_basic_param[cur_sen_mode[id]].signal_info.vd_period;
		new_tge_vd_sync = VD_SYNC;		
		new_tge_vd = tge_info.signal.vd_period;
		ctl_sen_set(ctl_sen_get_senid(id, 0, SEN_IMX226_MODULE_NAME_REG), CTL_SEN_CFGID_TGE_VD_SYNC, (void *)(&new_tge_vd_sync));
		ctl_sen_set(ctl_sen_get_senid(id, 0, SEN_IMX226_MODULE_NAME_REG), CTL_SEN_CFGID_TGE_VD_PERIOD, (void *)(&new_tge_vd));
		if (new_tge_vd != sensor_vd) {
			DBG_WRN(" sensor_vd = %d, new_tge_vd = %d \r\n",sensor_vd, new_tge_vd);
		}		
	}

	return rt;
}

static ER sen_set_info_imx226(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data)
{
	switch (drv_cfg_id) {
	case CTL_SENDRV_CFGID_SET_EXPT:
		sen_set_expt_imx226(id, data);
		break;
	case CTL_SENDRV_CFGID_SET_GAIN:
		sen_set_gain_imx226(id, data);
		break;
	case CTL_SENDRV_CFGID_FLIP_TYPE:
		sen_set_flip_imx226(id, (CTL_SEN_FLIP *)(data));
		break;
	case CTL_SENDRV_CFGID_USER_DEFINE1:
		sen_set_preset_imx226(id, (ISP_SENSOR_PRESET_CTRL *)(data));
		break;
	default:
		return E_NOSPT;
	}
	return E_OK;
}

static ER sen_get_info_imx226(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data)
{
	ER rt = E_OK;

	switch (drv_cfg_id) {
	case CTL_SENDRV_CFGID_GET_EXPT:
		sen_get_expt_imx226(id, data);
		break;
	case CTL_SENDRV_CFGID_GET_GAIN:
		sen_get_gain_imx226(id, data);
		break;
	case CTL_SENDRV_CFGID_GET_ATTR_BASIC:
		sen_get_attr_basic_imx226(id, (CTL_SENDRV_GET_ATTR_BASIC_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_ATTR_SIGNAL:
		sen_get_attr_signal_imx226((CTL_SENDRV_GET_ATTR_SIGNAL_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_ATTR_CMDIF:
		rt = sen_get_attr_cmdif_imx226(id, (CTL_SENDRV_GET_ATTR_CMDIF_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_ATTR_IF:
		rt = sen_get_attr_if_imx226((CTL_SENDRV_GET_ATTR_IF_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_FPS:
		sen_get_fps_imx226(id, (CTL_SENDRV_GET_FPS_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_SPEED:
		sen_get_speed_imx226(id, (CTL_SENDRV_GET_SPEED_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_MODE_BASIC:
		sen_get_mode_basic_imx226((CTL_SENDRV_GET_MODE_BASIC_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_MODE_LVDS:
		sen_get_mode_lvds_imx226((CTL_SENDRV_GET_MODE_LVDS_PARAM *)(data));
		break;		
	case CTL_SENDRV_CFGID_GET_MODESEL:
		sen_get_modesel_imx226((CTL_SENDRV_GET_MODESEL_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_MODE_ROWTIME:
		sen_get_rowtime_imx226(id, (CTL_SENDRV_GET_MODE_ROWTIME_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_FLIP_TYPE:
		rt = sen_get_flip_imx226(id, (CTL_SEN_FLIP *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_MODE_TGE:
		sen_ext_get_mode_tge_imx226((CTL_SENDRV_GET_MODE_TGE_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_USER_DEFINE2:
		sen_get_min_expt_imx226(id, data);
		break;
	default:
		rt = E_NOSPT;
	}
	return rt;
}

static UINT32 sen_calc_chgmode_vd_imx226(CTL_SEN_ID id, UINT32 fps)
{
	UINT32 sensor_vd;

	if (1 > fps) {
		DBG_ERR("sensor fps can not small than (%d),change to dft sensor fps (%d) \r\n", fps, mode_basic_param[cur_sen_mode[id]].dft_fps);	
		fps = mode_basic_param[cur_sen_mode[id]].dft_fps;
	}
	sensor_vd = (mode_basic_param[cur_sen_mode[id]].signal_info.vd_period) * (mode_basic_param[cur_sen_mode[id]].dft_fps) / fps;

	sen_set_chgmode_fps_imx226(id, fps);
	sen_set_cur_fps_imx226(id, fps);

	if (sensor_vd > MAX_VD_PERIOD) {
		DBG_ERR("sensor vd out of sensor driver range (%d) \r\n", sensor_vd);
		sensor_vd = MAX_VD_PERIOD;
		fps = (mode_basic_param[cur_sen_mode[id]].signal_info.vd_period) * (mode_basic_param[cur_sen_mode[id]].dft_fps) / sensor_vd;
		sen_set_chgmode_fps_imx226(id, fps);
		sen_set_cur_fps_imx226(id, fps);
	}

	if(sensor_vd < (mode_basic_param[cur_sen_mode[id]].signal_info.vd_period)) {
		DBG_ERR("sensor vd out of sensor driver range (%d) \r\n", sensor_vd);
		sensor_vd = mode_basic_param[cur_sen_mode[id]].signal_info.vd_period;
		sen_set_chgmode_fps_imx226(id, mode_basic_param[cur_sen_mode[id]].dft_fps);
		sen_set_cur_fps_imx226(id, mode_basic_param[cur_sen_mode[id]].dft_fps);
	}

	return sensor_vd;
}

static UINT32 sen_calc_exp_vd_imx226(CTL_SEN_ID id, UINT32 fps)
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

	if(sensor_vd < (mode_basic_param[cur_sen_mode[id]].signal_info.vd_period)) {
		DBG_ERR("sensor vd out of sensor driver range (%d) \r\n", sensor_vd);
		sensor_vd = mode_basic_param[cur_sen_mode[id]].signal_info.vd_period;
	}

	return sensor_vd;
}

static void sen_set_gain_imx226(CTL_SEN_ID id, void *param)
{
	ISP_SENSOR_CTRL *sensor_ctrl = (ISP_SENSOR_CTRL *)param;
	UINT32 data1[ISP_SEN_MFRAME_MAX_NUM] = {0};
	UINT32 data2[ISP_SEN_MFRAME_MAX_NUM] = {0};	
	UINT32 frame_cnt, total_frame;
	UINT32 digital_gain = 0, analog_gain = 0;
	UINT32 apgc01 = 0, apgc02 = 0;
	static UINT32 delay_a_gain[CTL_SEN_ID_MAX] = {0};
	static UINT32 delay_apgc01[CTL_SEN_ID_MAX] = {0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E};		
	static UINT32 delay_apgc02[CTL_SEN_ID_MAX] = {0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D};		
	#if (SIF_VD_SYNC)
	#if (TRANS_PROTOCOL == TRANS_PROTOCOL_SIF)
	CTL_SEN_CMD cmd2[GAIN_REG_NUM];
	UINT32 reg_idx = 0;
	CTL_SEN_CMD cmd;	
	#endif
	#else
	CTL_SEN_CMD cmd;
	#endif
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

		if (22380 >= (sensor_ctrl->gain_ratio[frame_cnt])) {
			digital_gain = 0;
			analog_gain = (2048 - (2048000 / (sensor_ctrl->gain_ratio[frame_cnt])));
		} else if (44760 >= (sensor_ctrl->gain_ratio[frame_cnt])) {
			digital_gain = 1;
			analog_gain = (2048 - (4096000 / (sensor_ctrl->gain_ratio[frame_cnt])));
		} else if (89520 >= (sensor_ctrl->gain_ratio[frame_cnt])) {
			digital_gain = 2;
			analog_gain = ( 2048 - (8192000 / (sensor_ctrl->gain_ratio[frame_cnt])));
		} else {
			digital_gain = 3;
			analog_gain = (2048 - (16384000 / (sensor_ctrl->gain_ratio[frame_cnt])));
		}
		// collect the gain setting
		data1[frame_cnt] = analog_gain;
		data2[frame_cnt] = digital_gain;		
	}
	
	if (mode_basic_param[cur_sen_mode[id]].pixel_depth == CTL_SEN_PIXDEPTH_10BIT) {
		if (0x147 > data1[0]) {
			apgc01 = 0x1E;
			apgc02 = 0x1D;
		} else if (0x401 > data1[0]) {
			apgc01 = 0x20;
			apgc02 = 0x1F;
		} else if (0x52D > data1[0]) {
			apgc01 = 0x25;
			apgc02 = 0x24;
		} else if (0x697 > data1[0]) {
			apgc01 = 0x27;
			apgc02 = 0x26;
		} else if (0x74C > data1[0]) {
			apgc01 = 0x2B;
			apgc02 = 0x2A;
		} else {
			apgc01 = 0x2F;
			apgc02 = 0x2E;
		}
	} else if (mode_basic_param[cur_sen_mode[id]].pixel_depth == CTL_SEN_PIXDEPTH_12BIT) {
		apgc01 = 0x3C;
		apgc02 = 0x3B;
	}

	if (!first_exp_flag[id]) {
		cmd = sen_set_cmd_info_imx226(0x000A, 1, (data1[0]) & 0x07, 0x0);
		rt |= sen_write_reg_imx226(id, &cmd);
		cmd = sen_set_cmd_info_imx226(0x0009, 1, data1[0] & 0xFF, 0x0);
		rt |= sen_write_reg_imx226(id, &cmd);

		// Write digital gain
		cmd = sen_set_cmd_info_imx226(0x0011, 1, (data2[0] & 0x03), 0);
		rt |= sen_write_reg_imx226(id, &cmd);
		
		if (mode_basic_param[cur_sen_mode[id]].pixel_depth == CTL_SEN_PIXDEPTH_10BIT) {
			// Write APGC01 amd APGC02
			cmd = sen_set_cmd_info_imx226(0x0353, 1, (apgc01 >> 8) & 0x01, 0);
			rt |= sen_write_reg_imx226(id, &cmd);
			cmd = sen_set_cmd_info_imx226(0x0352, 1, apgc01 & 0xFF, 0);
			rt |= sen_write_reg_imx226(id, &cmd);	
			cmd = sen_set_cmd_info_imx226(0x0357, 1, (apgc02 >> 8) & 0x01, 0);
			rt |= sen_write_reg_imx226(id, &cmd);
			cmd = sen_set_cmd_info_imx226(0x0356, 1, apgc02 & 0xFF, 0);
			rt |= sen_write_reg_imx226(id, &cmd);
		} else if (mode_basic_param[cur_sen_mode[id]].pixel_depth == CTL_SEN_PIXDEPTH_12BIT) {
			// Write APGC01 amd APGC02
			cmd = sen_set_cmd_info_imx226(0x0353, 1, (apgc01 >> 8) & 0x01, 0);
			rt |= sen_write_reg_imx226(id, &cmd);
			cmd = sen_set_cmd_info_imx226(0x0352, 1, apgc01 & 0xFF, 0);
			rt |= sen_write_reg_imx226(id, &cmd);	
			cmd = sen_set_cmd_info_imx226(0x0357, 1, (apgc02 >> 8) & 0x01, 0);
			rt |= sen_write_reg_imx226(id, &cmd);
			cmd = sen_set_cmd_info_imx226(0x0356, 1, apgc02 & 0xFF, 0);
			rt |= sen_write_reg_imx226(id, &cmd);		
		}
	} else {
		#if (SIF_VD_SYNC)
		#if (TRANS_PROTOCOL == TRANS_PROTOCOL_SIF)
		// Write analog gain
		cmd2[reg_idx] = sen_set_cmd_info_imx226(0x000A, 1, (delay_a_gain[id] >> 8) & 0x07, 0x0);
		reg_idx++;
		cmd2[reg_idx] = sen_set_cmd_info_imx226(0x0009, 1, delay_a_gain[id] & 0xFF, 0x0);
		reg_idx++;
		cmd2[reg_idx] = sen_set_cmd_info_imx226(0x0011, 1, (data2[0] & 0x03), 0);
		reg_idx++;
		if (mode_basic_param[cur_sen_mode[id]].pixel_depth == CTL_SEN_PIXDEPTH_10BIT) {
			cmd2[reg_idx] = sen_set_cmd_info_imx226(0x0353, 1, (delay_apgc01[id] >> 8) & 0x01, 0);
			reg_idx++;
			cmd2[reg_idx] = sen_set_cmd_info_imx226(0x0352, 1, delay_apgc01[id] & 0xFF, 0);
			reg_idx++;
			cmd2[reg_idx] = sen_set_cmd_info_imx226(0x0357, 1, (delay_apgc02[id] >> 8) & 0x01, 0);
			reg_idx++;	
			cmd2[reg_idx] = sen_set_cmd_info_imx226(0x0356, 1, delay_apgc02[id] & 0xFF, 0);
		} else if (mode_basic_param[cur_sen_mode[id]].pixel_depth == CTL_SEN_PIXDEPTH_12BIT) {
			cmd2[reg_idx] = sen_set_cmd_info_imx226(0x0353, 1, (apgc01 >> 8) & 0x01, 0);
			reg_idx++;
			cmd2[reg_idx] = sen_set_cmd_info_imx226(0x0352, 1, apgc01 & 0xFF, 0);
			reg_idx++;
			cmd2[reg_idx] = sen_set_cmd_info_imx226(0x0357, 1, (apgc02 >> 8) & 0x01, 0);
			reg_idx++;	
			cmd2[reg_idx] = sen_set_cmd_info_imx226(0x0356, 1, apgc02 & 0xFF, 0);			
		}
			rt |= sen_write_reg_sync_vd_imx226(id, cmd2);
		#endif
		#else
		// Write analog gain
		cmd = sen_set_cmd_info_imx226(0x000A, 1, (delay_a_gain[id] >> 8) & 0x07, 0x0);
		rt |= sen_write_reg_imx226(id, &cmd);
		cmd = sen_set_cmd_info_imx226(0x0009, 1, delay_a_gain[id] & 0xFF, 0x0);
		rt |= sen_write_reg_imx226(id, &cmd);

		// Write digital gain
		cmd = sen_set_cmd_info_imx226(0x0011, 1, (data2[0] & 0x03), 0);
		rt |= sen_write_reg_imx226(id, &cmd);
		
		if (mode_basic_param[cur_sen_mode[id]].pixel_depth == CTL_SEN_PIXDEPTH_10BIT) {
			// Write APGC01 amd APGC02
			cmd = sen_set_cmd_info_imx226(0x0353, 1, (delay_apgc01[id] >> 8) & 0x01, 0);
			rt |= sen_write_reg_imx226(id, &cmd);
			cmd = sen_set_cmd_info_imx226(0x0352, 1, delay_apgc01[id] & 0xFF, 0);
			rt |= sen_write_reg_imx226(id, &cmd);	
			cmd = sen_set_cmd_info_imx226(0x0357, 1, (delay_apgc02[id] >> 8) & 0x01, 0);
			rt |= sen_write_reg_imx226(id, &cmd);
			cmd = sen_set_cmd_info_imx226(0x0356, 1, delay_apgc02[id] & 0xFF, 0);
			rt |= sen_write_reg_imx226(id, &cmd);
		} else if (mode_basic_param[cur_sen_mode[id]].pixel_depth == CTL_SEN_PIXDEPTH_12BIT) {
			// Write APGC01 amd APGC02
			cmd = sen_set_cmd_info_imx226(0x0353, 1, (apgc01 >> 8) & 0x01, 0);
			rt |= sen_write_reg_imx226(id, &cmd);
			cmd = sen_set_cmd_info_imx226(0x0352, 1, apgc01 & 0xFF, 0);
			rt |= sen_write_reg_imx226(id, &cmd);	
			cmd = sen_set_cmd_info_imx226(0x0357, 1, (apgc02 >> 8) & 0x01, 0);
			rt |= sen_write_reg_imx226(id, &cmd);
			cmd = sen_set_cmd_info_imx226(0x0356, 1, apgc02 & 0xFF, 0);
			rt |= sen_write_reg_imx226(id, &cmd);		
		}		
		#endif
	}

	delay_a_gain[id] = data1[0];
	delay_apgc01[id] = apgc01;
	delay_apgc02[id] = apgc02;

	if (rt != E_OK) {
		DBG_ERR("write register error %d \r\n", (INT)rt);
	}
}

static ER sen_get_min_shr_imx226(CTL_SEN_MODE mode, UINT32 *min_shr, UINT32 *min_exp)
{
    ER rt = E_OK;

    switch (mode) {
    case CTL_SEN_MODE_1:	
    case CTL_SEN_MODE_2:	    	
        *min_shr = 8;
        *min_exp = 4;
        break;

    default:
        DBG_ERR("sensor mode %d no cmd table\r\n", mode);
        break;
    }
	return rt;
}

static void sen_set_expt_imx226(CTL_SEN_ID id, void *param)
{
	ISP_SENSOR_CTRL *sensor_ctrl = (ISP_SENSOR_CTRL *)param;
	CTL_SENDRV_GET_MODE_TGE_PARAM tge_info;
	UINT32 line[ISP_SEN_MFRAME_MAX_NUM];
	UINT32 frame_cnt, total_frame;
	CTL_SEN_CMD cmd;
	UINT32 expt_time = 0, sensor_vd = 0, chgmode_fps = 0, cur_fps = 0, clac_fps = 0, t_row = 0;
	UINT32 min_shr = 0, min_exp_line = 0;
	UINT32 shr = 0;
	UINT32 new_tge_vd = 0, new_tge_vd_sync = 0;	
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
		t_row = sen_calc_rowtime_imx226(id, cur_sen_mode[id]);
		if (0 == t_row) {
			DBG_WRN("t_row  = 0, must >= 1 \r\n");
			t_row = 1;
		}
		line[frame_cnt] = (sensor_ctrl->exp_time[frame_cnt]) * 10 / t_row;

		sen_get_min_shr_imx226(cur_sen_mode[id], &min_shr, &min_exp_line);

		// Limit minimun exposure line
		if (line[frame_cnt] < min_exp_line) {
			line[frame_cnt] = min_exp_line;
		}
	}

	// Write exposure line
	// Get fps
	chgmode_fps = sen_get_chgmode_fps_imx226(id);

	// Calculate exposure time
	t_row = sen_calc_rowtime_imx226(id, cur_sen_mode[id]);
	expt_time = (line[0]) * t_row / 10;
	temp_line[0] = line[0];

	// Calculate fps
	if (0 == expt_time) {
		DBG_WRN("expt_time  = 0, must >= 1 \r\n");
		expt_time = 1;
	}
	clac_fps = 100000000 / expt_time;

	cur_fps = (clac_fps < chgmode_fps) ? clac_fps : chgmode_fps;
	sen_set_cur_fps_imx226(id, cur_fps);

	// Calculate new vd
	sensor_vd = sen_calc_exp_vd_imx226(id, cur_fps);

 	// Calculate new tge vd
	memset((void *)&tge_info, 0, sizeof(CTL_SENDRV_GET_MODE_TGE_PARAM));
	tge_info.mode = cur_sen_mode[id];
	sen_ext_get_mode_tge_imx226(&tge_info);
	
	new_tge_vd_sync = (mode_basic_param[cur_sen_mode[id]].dft_fps) * (tge_info.signal.vd_period) / cur_fps + VD_SYNC - (tge_info.signal.vd_period);
	new_tge_vd = (mode_basic_param[cur_sen_mode[id]].dft_fps) * (tge_info.signal.vd_period) / cur_fps;
	ctl_sen_set(ctl_sen_get_senid(id, 0, SEN_IMX226_MODULE_NAME_REG), CTL_SEN_CFGID_TGE_VD_SYNC, (void *)(&new_tge_vd_sync));
	ctl_sen_set(ctl_sen_get_senid(id, 0, SEN_IMX226_MODULE_NAME_REG), CTL_SEN_CFGID_TGE_VD_PERIOD, (void *)(&new_tge_vd));

	//Check max vts
	if (sensor_vd > MAX_VD_PERIOD) {
		DBG_ERR("max vts overflow\r\n");
		sensor_vd = MAX_VD_PERIOD;
	}

	//Check max exp line reg
	if (line[0] > MAX_EXPOSURE_LINE) {
		DBG_ERR("max line overflow \r\n");
		line[0] = MAX_EXPOSURE_LINE;
	}

	//calculate shr
	if (line[0] > (sensor_vd - min_shr)) {
		shr = min_shr;
	} else {
		shr = sensor_vd - line[0];
	}
	compensation_ratio[id][0] = 100 * temp_line[0] / (sensor_vd - shr);	

	// set exposure line to sensor
	cmd = sen_set_cmd_info_imx226(0x000C, 1, (shr >> 8) & 0xFF, 0);
	rt |= sen_write_reg_imx226(id, &cmd);	
	cmd = sen_set_cmd_info_imx226(0x000B, 1, shr & 0xFF, 0);
	rt |= sen_write_reg_imx226(id, &cmd);
	
	if (rt != E_OK) {
		DBG_ERR("write register error %d \r\n", (INT)rt);
	}
}

static void sen_set_preset_imx226(CTL_SEN_ID id, ISP_SENSOR_PRESET_CTRL *ctrl)
{
	memcpy(&preset_ctrl[id], ctrl, sizeof(ISP_SENSOR_PRESET_CTRL));
}

static void sen_set_flip_imx226(CTL_SEN_ID id, CTL_SEN_FLIP *flip)
{
	CTL_SEN_CMD cmd;
	ER rt = E_OK;

	if (basic_param.property & CTL_SEN_SUPPORT_PROPERTY_MIRROR) {
		DBG_WRN("no support mirror \r\n");
	}
	if (basic_param.property & CTL_SEN_SUPPORT_PROPERTY_FLIP) {
		if (*flip & CTL_SEN_FLIP_V) {
			rotate_reg[id] |= 0x1;
		} else {
			rotate_reg[id] &= (~0x01);
		}
	} else {
		DBG_WRN("no support flip \r\n");
	}
	cmd = sen_set_cmd_info_imx226(0x001A, 1, rotate_reg[id], 0x0);
	rt |= sen_write_reg_imx226(id, &cmd);

	if (rt != E_OK) {
		DBG_ERR("write register error %d \r\n", (INT)rt);
	}
}

static ER sen_get_flip_imx226(CTL_SEN_ID id, CTL_SEN_FLIP *flip)
{
	ER rt = E_OK;

	*flip = CTL_SEN_FLIP_NONE;
	if (basic_param.property & CTL_SEN_SUPPORT_PROPERTY_MIRROR) {
		DBG_WRN("no support mirror \r\n");
	}

	if (basic_param.property & CTL_SEN_SUPPORT_PROPERTY_FLIP) {
		if (rotate_reg[id] & 0x01) {
			*flip |= CTL_SEN_FLIP_V;
		}
	} else {
		DBG_WRN("no support flip \r\n");
	}

	return rt;
}

#if defined(__FREERTOS)
void sen_get_gain_imx226(CTL_SEN_ID id, void *param)
#else
static void sen_get_gain_imx226(CTL_SEN_ID id, void *param)
#endif
{
	ISP_SENSOR_CTRL *sensor_ctrl = (ISP_SENSOR_CTRL *)param;

	sensor_ctrl->gain_ratio[0] = sensor_ctrl_last[id].gain_ratio[0];
	sensor_ctrl->gain_ratio[1] = sensor_ctrl_last[id].gain_ratio[1];
}

#if defined(__FREERTOS)
void sen_get_expt_imx226(CTL_SEN_ID id, void *param)
#else
static void sen_get_expt_imx226(CTL_SEN_ID id, void *param)
#endif
{
	ISP_SENSOR_CTRL *sensor_ctrl = (ISP_SENSOR_CTRL *)param;

	sensor_ctrl->exp_time[0] = sensor_ctrl_last[id].exp_time[0];
	sensor_ctrl->exp_time[1] = sensor_ctrl_last[id].exp_time[1];
}

static void sen_get_min_expt_imx226(CTL_SEN_ID id, void *param)
{
	UINT32 *min_exp_time = (UINT32 *)param;
	UINT32 t_row = 0, min_shr = 0, min_exp_line = 0;

	sen_get_min_shr_imx226(cur_sen_mode[id], &min_shr, &min_exp_line);

	t_row = sen_calc_rowtime_imx226(id, cur_sen_mode[id]);
	*min_exp_time = t_row * min_exp_line / 10 + 1;
}

static void sen_get_mode_basic_imx226(CTL_SENDRV_GET_MODE_BASIC_PARAM *mode_basic)
{
	UINT32 mode = mode_basic->mode;
	
	if (mode >= SEN_MAX_MODE) {
		mode = 0;
	}
	memcpy(mode_basic, &mode_basic_param[mode], sizeof(CTL_SENDRV_GET_MODE_BASIC_PARAM));
}

static void sen_get_attr_basic_imx226(CTL_SEN_ID id, CTL_SENDRV_GET_ATTR_BASIC_PARAM *data)
{
	memcpy(data, &basic_param, sizeof(CTL_SENDRV_GET_ATTR_BASIC_PARAM));
}

static void sen_get_attr_signal_imx226(CTL_SENDRV_GET_ATTR_SIGNAL_PARAM *data)
{
	memcpy(data, &signal_param, sizeof(CTL_SENDRV_GET_ATTR_SIGNAL_PARAM));
}

static ER sen_get_attr_cmdif_imx226(CTL_SEN_ID id, CTL_SENDRV_GET_ATTR_CMDIF_PARAM *data)
{
	#if (TRANS_PROTOCOL == TRANS_PROTOCOL_SIF)
	//don't care
	data->type = CTL_SEN_CMDIF_TYPE_I2C;
	memcpy(&data->info, &i2c, sizeof(CTL_SENDRV_I2C));
	#else
	data->type = CTL_SEN_CMDIF_TYPE_I2C;
	memcpy(&data->info, &i2c, sizeof(CTL_SENDRV_I2C));
	#endif
	return E_OK;	
}

static ER sen_get_attr_if_imx226(CTL_SENDRV_GET_ATTR_IF_PARAM *data)
{
	#if 1
	if (data->type == CTL_SEN_IF_TYPE_LVDS) {
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

static void sen_get_fps_imx226(CTL_SEN_ID id, CTL_SENDRV_GET_FPS_PARAM *data)
{
	data->cur_fps = sen_get_cur_fps_imx226(id);
	data->chg_fps = sen_get_chgmode_fps_imx226(id);
}

static void sen_get_speed_imx226(CTL_SEN_ID id, CTL_SENDRV_GET_SPEED_PARAM *data)
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

static void sen_get_mode_lvds_imx226(CTL_SENDRV_GET_MODE_LVDS_PARAM *data)
{
	UINT32 mode = data->mode;
	
	if (mode >= SEN_MAX_MODE) {
		mode = 0;
	}
	memcpy(data, &lvds_param[mode], sizeof(CTL_SENDRV_GET_MODE_LVDS_PARAM));
}

static void sen_ext_get_mode_tge_imx226(CTL_SENDRV_GET_MODE_TGE_PARAM *data)
{
	if (data->mode == CTL_SEN_MODE_1) {
		data->signal.hd_sync = HD_SYNC;
		data->signal.hd_period = 825;
		data->signal.vd_sync = VD_SYNC;
		data->signal.vd_period = 3120;
	} else if (data->mode == CTL_SEN_MODE_2) {
		data->signal.hd_sync = HD_SYNC;
		data->signal.hd_period = 700;
		data->signal.vd_sync = VD_SYNC;
		data->signal.vd_period = 3120;
	}
}

static void sen_get_modesel_imx226(CTL_SENDRV_GET_MODESEL_PARAM *data)
{
	if (data->if_type != CTL_SEN_IF_TYPE_LVDS) {
		DBG_ERR("if_type %d N.S. \r\n", data->if_type);
		return;
	}

	if (data->data_fmt != CTL_SEN_DATA_FMT_RGB) {
		DBG_ERR("data_fmt %d N.S. \r\n", data->data_fmt);
		return;
	}

	if (data->frame_num == 1) {
		if ((data->size.w <= 4000) && (data->size.h <= 3000)) {
			if (data->pixdepth == CTL_SEN_PIXDEPTH_12BIT) {
				if (data->frame_rate <= 2797) {
					data->mode = CTL_SEN_MODE_1;
					return;
				}
			} else if (data->pixdepth == CTL_SEN_PIXDEPTH_10BIT) {
				if (data->frame_rate <= 3297) {
					data->mode = CTL_SEN_MODE_2;
					return;
				}
			}
		}
	} 					

	DBG_ERR("fail (frame_rate=%d,size=%d*%d,if_type=%d,data_fmt=%d,frame_num=%d,data_lane=%d,mode_type_sel=%llx,pixdepth=%lld) \r\n"
			, data->frame_rate, data->size.w, data->size.h, data->if_type, data->data_fmt, data->frame_num, data->data_lane, (UINT64)data->mode_type_sel,(UINT64)data->pixdepth);
	data->mode = CTL_SEN_MODE_1;
}

static UINT32 sen_calc_rowtime_step_imx226(CTL_SEN_ID id, CTL_SEN_MODE mode)
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

static UINT32 sen_calc_rowtime_imx226(CTL_SEN_ID id, CTL_SEN_MODE mode)
{
	UINT32 row_time = 0;

	if (mode >= SEN_MAX_MODE) {
		mode = cur_sen_mode[id];
	}

	//Precision * 10
	row_time = 10 * (mode_basic_param[mode].signal_info.hd_period) / ((speed_param[mode].pclk) / 1000000);

	return row_time;
}

static void sen_get_rowtime_imx226(CTL_SEN_ID id, CTL_SENDRV_GET_MODE_ROWTIME_PARAM *data)
{
	data->row_time_step = sen_calc_rowtime_step_imx226(id, data->mode);	
	data->row_time = sen_calc_rowtime_imx226(id, data->mode) * (data->row_time_step);
}

static void sen_set_cur_fps_imx226(CTL_SEN_ID id, UINT32 fps)
{
	cur_fps[id] = fps;
}

static UINT32 sen_get_cur_fps_imx226(CTL_SEN_ID id)
{
	return cur_fps[id];
}

static void sen_set_chgmode_fps_imx226(CTL_SEN_ID id, UINT32 fps)
{
	chgmode_fps[id] = fps;
}

static UINT32 sen_get_chgmode_fps_imx226(CTL_SEN_ID id)
{
	return chgmode_fps[id];
}

#if defined(__FREERTOS)
int sen_init_imx226(SENSOR_DTSI_INFO *info)
{
	CTL_SEN_REG_OBJ reg_obj;
	CHAR node_path[64];
	CHAR compatible[64];
	ER rt = E_OK;

	sprintf(compatible, "nvt,sen_imx226");
	if (sen_common_check_compatible(compatible)) {
		DBG_DUMP("compatible valid, using peri-dev.dtsi \r\n");
		sen_common_load_cfg_preset_compatible(compatible, &sen_preset);
		sen_common_load_cfg_direction_compatible(compatible, &sen_direction);
		sen_common_load_cfg_power_compatible(compatible, &sen_power);
	} else if (info->addr != NULL) {
		DBG_DUMP("compatible not valid, using sensor.dtsi \r\n");
		sprintf(node_path, "/sensor/sen_cfg/sen_imx226");
		sen_common_load_cfg_map(info->addr, node_path, &sen_map);
		sen_common_load_cfg_preset(info->addr, node_path, &sen_preset);
		sen_common_load_cfg_direction(info->addr, node_path, &sen_direction);
		sen_common_load_cfg_power(info->addr, node_path, &sen_power);
	} else {
		DBG_WRN("DTSI addr is NULL \r\n");
	}

	memset((void *)(&reg_obj), 0, sizeof(CTL_SEN_REG_OBJ));
	reg_obj.pwr_ctrl = sen_pwr_ctrl_imx226;
	reg_obj.det_plug_in = NULL;
	reg_obj.drv_tab = sen_get_drv_tab_imx226();
	rt = ctl_sen_reg_sendrv("SEN_IMX226_MODULE_NAME_REG", &reg_obj);
	if (rt != E_OK) {
		DBG_WRN("register sensor driver fail \r\n");
	}

	return rt;
}

void sen_exit_imx226(void)
{
	ctl_sen_unreg_sendrv("SEN_IMX226_MODULE_NAME_REG");
}

#else
static int __init sen_init_imx226(void)
{
	INT8 cfg_path[MAX_PATH_NAME_LENGTH+1] = { '\0' };
	CFG_FILE_FMT *pcfg_file;
	CTL_SEN_REG_OBJ reg_obj;
	ER rt = E_OK;

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
			#if (TRANS_PROTOCOL == TRANS_PROTOCOL_SIF)
			#else
			#if defined(__KERNEL__)
			sen_common_load_cfg_spi(pcfg_file, &sen_spi);
			#endif
			#endif
			sen_common_close_cfg(pcfg_file);
		} else {
			DBG_WRN("load cfg fail \r\n");
		}
	}

	memset((void *)(&reg_obj), 0, sizeof(CTL_SEN_REG_OBJ));
	reg_obj.pwr_ctrl = sen_pwr_ctrl_imx226;
	reg_obj.det_plug_in = NULL;
	reg_obj.drv_tab = sen_get_drv_tab_imx226();
	rt = ctl_sen_reg_sendrv("nvt_sen_imx226", &reg_obj);
	if (rt != E_OK) {
		DBG_WRN("register sensor driver fail \r\n");
	}

	return rt;
}

static void __exit sen_exit_imx226(void)
{
	ctl_sen_unreg_sendrv("nvt_sen_imx226");
}

module_init(sen_init_imx226);
module_exit(sen_exit_imx226);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION(SEN_IMX226_MODULE_NAME);
MODULE_LICENSE("GPL");
#endif

