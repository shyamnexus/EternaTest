#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// define
//=============================================================================

//=============================================================================
// struct & enum definition
//=============================================================================
typedef struct _SENSOR_INFO_ {
	UINT32 sensor_num;
	USIZE size;
	CHAR sensor_name[32];
	CHAR isp_cfg_name[32];
	CHAR fpn_name_f[64];
	CHAR fpn_name_e[64];
} SENSOR_INFO;

typedef struct _MODEL_PARAM_NAME_ {
	UINT32 num;
	UINT32 param_size[AIISP_PARAM_MAX];                      ///< The size of each parameter
	CHAR param_name[AIISP_PARAM_MAX][32];                    ///< The name of each parameter
} MODEL_PARAM_NAME;

extern HD_RESULT isp_init(void);
extern HD_RESULT isp_init_sensor(SENSOR_INFO *sensor_info, char *chip_name, UINT32 shdr, UINT32 isp_id);
extern HD_RESULT isp_set_config(BOOL manual, UINT32 config, BOOL enable, UINT32 isp_id);
extern HD_RESULT isp_update_param_name(UINT32 id, MODEL_PARAM_NAME *p_param_name, UINT32 isp_id);
extern void *isp_smart_param_thread(void *arg);

#ifdef __cplusplus
}
#endif

