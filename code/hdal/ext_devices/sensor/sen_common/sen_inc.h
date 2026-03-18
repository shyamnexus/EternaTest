#ifndef _SEN_INC_H_
#define _SEN_INC_H_

#include "kflow_videocapture/ctl_sen_ext.h"

#if defined(__FREERTOS)
extern int sen_init_ar0237(SENSOR_DTSI_INFO *info);
extern void sen_get_i2c_id_f35(CTL_SEN_ID id, UINT32 *i2c_id);
extern void sen_get_i2c_addr_f35(CTL_SEN_ID id, UINT32 *i2c_addr);
extern void sen_get_gain_f35(CTL_SEN_ID id, void *param);
extern void sen_get_expt_f35(CTL_SEN_ID id, void *param);
extern int sen_init_f35(SENSOR_DTSI_INFO *info);
extern void sen_get_i2c_id_f37(CTL_SEN_ID id, UINT32 *i2c_id);
extern void sen_get_i2c_addr_f37(CTL_SEN_ID id, UINT32 *i2c_addr);
extern void sen_get_gain_f37(CTL_SEN_ID id, void *param);
extern void sen_get_expt_f37(CTL_SEN_ID id, void *param);
extern int sen_init_f37(SENSOR_DTSI_INFO *info);
extern void sen_get_i2c_id_imx290(CTL_SEN_ID id, UINT32 *i2c_id);
extern void sen_get_i2c_addr_imx290(CTL_SEN_ID id, UINT32 *i2c_addr);
extern void sen_get_gain_imx290(CTL_SEN_ID id, void *param);
extern void sen_get_expt_imx290(CTL_SEN_ID id, void *param);
extern int sen_init_imx290(SENSOR_DTSI_INFO *info);
extern void sen_get_i2c_id_imx415(CTL_SEN_ID id, UINT32 *i2c_id);
extern void sen_get_i2c_addr_imx415(CTL_SEN_ID id, UINT32 *i2c_addr);
extern void sen_get_gain_imx415(CTL_SEN_ID id, void *param);
extern void sen_get_expt_imx415(CTL_SEN_ID id, void *param);
extern int sen_init_imx415(SENSOR_DTSI_INFO *info);
extern void sen_get_i2c_id_os02k10(CTL_SEN_ID id, UINT32 *i2c_id);
extern void sen_get_i2c_addr_os02k10(CTL_SEN_ID id, UINT32 *i2c_addr);
extern void sen_get_gain_os02k10(CTL_SEN_ID id, void *param);
extern void sen_get_expt_os02k10(CTL_SEN_ID id, void *param);
extern int sen_init_os02k10(SENSOR_DTSI_INFO *info);

extern void sen_get_i2c_id_os04c10(CTL_SEN_ID id, UINT32 *i2c_id);
extern void sen_get_i2c_addr_os04c10(CTL_SEN_ID id, UINT32 *i2c_addr);
extern void sen_get_cur_mode_os04c10(CTL_SEN_ID id, UINT32 *cur_mode);
extern void sen_get_mode_param_os04c10(CTL_SEN_ID id, CTL_SENDRV_GET_MODE_BASIC_PARAM *mode_basic);
extern void sen_get_row_time_os04c10(CTL_SEN_ID id, UINT32 *row_time);
extern void sen_get_gain_os04c10(CTL_SEN_ID id, void *param);
extern void sen_get_expt_os04c10(CTL_SEN_ID id, void *param);
extern int sen_init_os04c10(SENSOR_DTSI_INFO *info);

extern void sen_get_i2c_id_os05a10(CTL_SEN_ID id, UINT32 *i2c_id);
extern void sen_get_i2c_addr_os05a10(CTL_SEN_ID id, UINT32 *i2c_addr);
extern void sen_get_gain_os05a10(CTL_SEN_ID id, void *param);
extern void sen_get_expt_os05a10(CTL_SEN_ID id, void *param);
extern int sen_init_os05a10(SENSOR_DTSI_INFO *info);
extern void sen_get_i2c_id_gc4653(CTL_SEN_ID id, UINT32 *i2c_id);
extern void sen_get_i2c_addr_gc4653(CTL_SEN_ID id, UINT32 *i2c_addr);
extern void sen_get_gain_gc4653(CTL_SEN_ID id, void *param);
extern void sen_get_expt_gc4653(CTL_SEN_ID id, void *param);
extern int sen_init_gc4653(SENSOR_DTSI_INFO *info);
extern void sen_get_i2c_id_imx477(CTL_SEN_ID id, UINT32 *i2c_id);
extern void sen_get_i2c_addr_imx477(CTL_SEN_ID id, UINT32 *i2c_addr);
extern void sen_get_gain_imx477(CTL_SEN_ID id, void *param);
extern void sen_get_expt_imx477(CTL_SEN_ID id, void *param);
extern int sen_init_imx477(SENSOR_DTSI_INFO *info);
extern int sen_init_ov2715(SENSOR_DTSI_INFO *info);
extern int sen_init_tc358743(SENSOR_DTSI_INFO *info);
extern int sen_init_tc358840(SENSOR_DTSI_INFO *info);
extern void sen_get_i2c_id_ov64b(CTL_SEN_ID id, UINT32 *i2c_id);
extern void sen_get_i2c_addr_ov64b(CTL_SEN_ID id, UINT32 *i2c_addr);
extern void sen_get_gain_ov64b(CTL_SEN_ID id, void *param);
extern void sen_get_expt_ov64b(CTL_SEN_ID id, void *param);
extern int sen_init_ov64b(SENSOR_DTSI_INFO *info);

#endif
#endif

