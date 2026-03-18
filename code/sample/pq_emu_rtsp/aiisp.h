#ifndef _AIISP_H_
#define _AIISP_H_

//=============================================================================
// define
//=============================================================================
#define VENDOR_AI_CFG       0x000f0000  //vendor ai config

#define LOW_ISO_EFFECT      0
#define NORMAL_ISO_EFFECT   1
#define HIGH_ISO_EFFECT     2

//=============================================================================
// struct & enum definition
//=============================================================================
typedef enum _VIDEO_AIISP_ID {
	VIDEO_AIISP_0 = 0,
	VIDEO_AIISP_1,
	VIDEO_AIISP_3,
	VIDEO_AIISP_4,
	VIDEO_AIISP_MAX,
	ENUM_DUMMY4WORD(VIDEO_AIISP_ID)
} VIDEO_AIISP_ID;

//=============================================================================
// extern functions
//=============================================================================
extern HD_RESULT aiisp_get_model_name(UINT32 path, CHAR *ai_name);
extern UINT32 aiisp_get_proc_id(UINT32 path);
extern HD_RESULT aiisp_get_ai_cb(uintptr_t *isp_cb);
extern HD_RESULT aiisp_set_model_name(UINT32 path, CHAR *ai_name);
extern HD_RESULT aiisp_init(void);
extern HD_RESULT aiisp_open(VIDEO_AIISP_ID aiisp_id);
extern HD_RESULT aiisp_start(VIDEO_AIISP_ID aiisp_id);
extern HD_RESULT aiisp_stop(VIDEO_AIISP_ID aiisp_id);
extern HD_RESULT aiisp_close(VIDEO_AIISP_ID aiisp_id);
extern HD_RESULT aiisp_uninit(void);

#endif
