#ifndef _FASTS_ENSOR_H
#define _FASTS_ENSOR_H
#include <hdal.h>

/**
 * @brief SENSOR_DRIVER_READY
 * if rtos sensor driver for fastboot not ready,
 * disable it to by pass compiling error
 */
#define SENSOR_DRIVER_READY 1

#define _SEN_GET_EXPT_GAIN_I2C(sen, sen_id, expt, gain, i2c_id, i2c_addr) \
	sen_get_expt_##sen(sen_id, expt); \
	sen_get_gain_##sen(sen_id, gain); \
	sen_get_i2c_id_##sen(sen_id, i2c_id); \
	sen_get_i2c_addr_##sen(sen_id, i2c_addr);
#define SEN_GET_EXPT_GAIN_I2C(sen, sen_id, expt, gain, i2c_id, i2c_addr) _SEN_GET_EXPT_GAIN_I2C(sen, sen_id, expt, gain, i2c_id, i2c_addr)

#define _SEN_GET_CUR_MODE(sen, sen_id, cur_mode) \
	sen_get_cur_mode_##sen(sen_id, cur_mode);
#define SEN_GET_CUR_MODE(sen, sen_id, cur_mode) _SEN_GET_CUR_MODE(sen, sen_id, cur_mode)

#define _SEN_GET_MODE_PARAM(sen, sen_id, mode_basic) \
	sen_get_mode_param_##sen(sen_id, mode_basic);
#define SEN_GET_MODE_PARAM(sen, sen_id, mode_basic) _SEN_GET_MODE_PARAM(sen, sen_id, mode_basic)

#define _SEN_GET_ROW_TIME(sen, sen_id, row_time) \
	sen_get_row_time_##sen(sen_id, row_time);
#define SEN_GET_ROW_TIME(sen, sen_id, row_time) _SEN_GET_ROW_TIME(sen, sen_id, row_time)

typedef enum _SHDR_EN {
	SHDR_OFF = 0,
	SHDR_ON = 1,
} SHDR_EN;

typedef struct  _FAST2A_DESC {
	UINT32 data_lane;
	USIZE  vdo_size;
	UINT32 fps; // real fps * 100
	UINT32 expt_max;
} FAST2A_DESC;

typedef struct  _FAST_SENSOR_DESC {
	UINT32 id; // sensor name id, refer to sensor_builtin_print_info() intsensor_builtin.c
	CHAR   *name;
	USIZE  vdo_size;
	UINT32 fps; // real fps * 100
	UINT32 expt_max;
	HD_COMMON_VIDEO_IN_TYPE if_type;
	UINT32 data_lane;
	SHDR_EN shdr;
	UINT32 wait_det_ms;
	FAST2A_DESC fast2a;
} FAST_SENSOR_DESC;

void fast_open_sensor(void);
void fast_open_sensor2(void);
void fast_2a(void);
void fast_chgmode_sensor(void);
void fast_chgmode_sensor2(void);
int fast_2a_delay_ms(void);
void sensor_embedded_2a(void);
void sensor_preroll_2a(void);
void sensor_normal_2a(void);
int bridge_mem_plan_sensor1(void);
int bridge_mem_plan_sensor2(void);

#endif