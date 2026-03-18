#ifndef _ISP_API_INT_
#define _ISP_API_INT_

#include "isp_api.h"

//=============================================================================
// define
//=============================================================================
#define SIE_ID_MAX_NUM 5

//=============================================================================
// extern functions
//=============================================================================
extern BOOL isp_get_id_valid(UINT32 id);
extern UINT32 isp_get_sie_id(UINT32 id);
extern ER isp_api_get_direction(ISP_ID id, ISP_SENSOR_DIRECTION *sensor_direction);
extern ER isp_api_get_func(ISP_ID id, ISP_FUNC_INFO *isp_func);
extern ER isp_api_get_sensor_reg(ISP_ID id, UINT32 *addr, UINT32 *data);
extern ER isp_api_get_sensor_info(ISP_SENSOR_INFO *sensor_info);
extern ER isp_api_get_yuv(ISP_ID id, ISP_YUV_INFO *yuv_info);
extern ER isp_api_get_raw(ISP_ID id, ISP_RAW_INFO *raw_info, UINT32 vdo_plane);
extern ER isp_api_get_bnr_sta(ISP_ID id, ISP_BNR_STA_INFO *bnr_sta_info);
extern ER isp_api_get_3dnr_sta(ISP_ID id, ISP_3DNR_STA_INFO *_3dnr_sta_info);
extern BOOL isp_api_get_emu_enable(void);
extern ER isp_get_sie_func(ISP_ID id, ISP_FUNC_EN *fun_en);
extern ER isp_get_ipp_func(ISP_ID id, ISP_FUNC_EN *fun_en);
extern UINT32 isp_get_ipp_table(ISP_ID id);
extern UINT32 isp_get_ipp_indep(ISP_ID id);
extern void isp_get_sensor_mode_manual(ISP_ID id, ISP_SENSOR_MODE_MANUAL *manual);
extern UINT32 isp_api_get_low_power_lv(void);
extern ER isp_api_set_direction(ISP_ID id, ISP_SENSOR_DIRECTION *sensor_direction);
extern ER isp_api_set_sensor_reg(ISP_ID id, UINT32 *addr, UINT32 *data);
extern ER isp_api_set_sensor_sleep(ISP_ID id);
extern ER isp_api_set_sensor_wakeup(ISP_ID id);
extern ER isp_api_set_yuv(ISP_ID id);
extern ER isp_api_set_raw(ISP_ID id);
extern void isp_api_set_emu_enable(BOOL enable);
extern ER isp_api_set_emu_new_buf(ISP_ID id, CTL_SIE_ISP_SIM_BUF_NEW *param);
extern ER isp_api_set_emu_run(ISP_ID id, CTL_SIE_ISP_SIM_BUF_PUSH *param);
extern void isp_set_ipp_indep(ISP_ID id, BOOL enable);
extern void isp_set_sensor_mode_manual(ISP_ID id, ISP_SENSOR_MODE_MANUAL *manual);
extern void isp_api_set_low_power_lv(UINT32 lv);

#endif

