#ifndef _SENSOR_BUILTIN_H_
#define _SENSOR_BUILTIN_H_

#if defined(__KERNEL__)
#include <linux/slab.h>
#else
#include "plat/kdrv_i2c.h"
#endif

#include "kwrap/type.h"
#include "isp_builtin_int.h"

//=============================================================================
// version
//=============================================================================
#define SENSOR_BUILTIN_VERSION 0x02000100

//=============================================================================
// define
//=============================================================================
#if defined(__KERNEL__)
#define MALLOC(x) kmalloc((x), GFP_KERNEL)
#define FREE(x) kfree((x))
#else
#define MALLOC(x) malloc((x))
#define FREE(x) free((x))
typedef INT32 (*ISP_BUILTIN_I2C_CB_FP)(UINT32 id, struct i2c_msg *msgs, INT32 num);
#endif

#define hdr_expline_clamp(expl, l, u)    ((expl > u) ? u : ((expl < l) ? l : expl))

//=============================================================================
// struct & enum definition
//=============================================================================
typedef struct _SENSOR_BUILTIN_I2C_INFO {
	struct i2c_client  *iic_client;
	struct i2c_adapter *iic_adapter;
} SENSOR_BUILTIN_I2C_INFO;

typedef struct _SENSOR_BUILTIN_I2C_CMD {
	UINT32 addr;                      ///< address
	UINT32 data_len;                  ///< data length(bytes)
	UINT32 data[2];                   ///< data idx1(LSB) -> idx2(MSB)
} SENSOR_BUILTIN_I2C_CMD;

typedef struct _SENSOR_BUILTIN_DFT_PARAM {
	UINT32 row_time;
	UINT32 vd;
	UINT32 dft_fps;
	UINT32 frame_num;
	UINT32 max_gain;
} SENSOR_BUILTIN_DFT_PARAM;

//=============================================================================
// internal functions
//=============================================================================
extern SENSOR_BUILTIN_I2C_CMD sensor_builtin_set_cmd_info(UINT32 addr, UINT32 data_length, UINT32 data0, UINT32 data1);
extern UINT32 sensor_builtin_calc_log_2(UINT32 devider, UINT32 devident);
extern ER sensor_builtin_write_i2c_2b1b(UINT32 id, SENSOR_BUILTIN_I2C_CMD *cmd);
extern ER sensor_builtin_read_i2c_2b1b(UINT32 id, SENSOR_BUILTIN_I2C_CMD *cmd);
extern ER sensor_builtin_write_i2c_1b1b(UINT32 id, SENSOR_BUILTIN_I2C_CMD *cmd);
extern ER sensor_builtin_read_i2c_1b1b(UINT32 id, SENSOR_BUILTIN_I2C_CMD *cmd);
extern UINT32 sensor_builtin_os04c10_get_min_expt_time(UINT32 id, SENSOR_BUILTIN_DFT_PARAM *sensor_param);
extern void sensor_builtin_os04c10_set_gain(UINT32 id, ISP_BUILTIN_SENSOR_CTRL *sensor_ctrl, SENSOR_BUILTIN_DFT_PARAM *sensor_param);
extern void sensor_builtin_os04c10_set_expt(UINT32 id, ISP_BUILTIN_SENSOR_CTRL *sensor_ctrl, SENSOR_BUILTIN_DFT_PARAM *sensor_param);

//=============================================================================
// extern functions
//=============================================================================
#if defined(__FREERTOS)
extern void sensor_builtin_reg_i2c_cb(void *cb_fp);
#endif
extern UINT32 sensor_builtin_get_row_time(UINT32 id);
extern UINT32 sensor_builtin_get_min_expt_time(UINT32 id);
extern UINT32 isp_builtin_get_sensor_min_expt_time(UINT32 id);
extern void sensor_builtin_print_info(UINT32 id);
extern void sensor_builtin_set_gain(UINT32 id, ISP_BUILTIN_SENSOR_CTRL *value);
extern void sensor_builtin_set_expt(UINT32 id, ISP_BUILTIN_SENSOR_CTRL *value);
extern INT32 sensor_builtin_set_transfer_i2c(UINT32 id, void *msgs, INT32 num);
extern void sensor_builtin_set_dft_param(UINT32 id, SENSOR_BUILTIN_DFT_PARAM *param);
extern void sensor_builtin_init_i2c(UINT32 id);
extern void sensor_builtin_uninit_i2c(UINT32 id);

#endif

