#ifndef _FLOW_BOOT_LINUX_H
#define _FLOW_BOOT_LINUX_H
#include <sys_linuxboot.h>

/**
 * disabling BOTH AI_MODEL and AI_DETECT can reduce some rtos memory and cpu usage
 */
#define CFG_LOAD_AI_MODEL   DISABLE // conflict with CFG_LOAD_AI_DETECT
#if defined(_RTOS2A_PREROLL_)
#define CFG_LOAD_AI_DETECT  DISABLE
#else
#define CFG_LOAD_AI_DETECT  ENABLE  // conflict with CFG_LOAD_AI_MODEL
#endif
#define CFG_LOAD_AUDIOIO    DISABLE
#define CFG_LOAD_SOUND      DISABLE
#define CFG_RTOS_IPP        ENABLE
#define CFG_RTOS_VENC       ENABLE

#if (CFG_LOAD_AI_MODEL) && (CFG_LOAD_AI_DETECT)
#error "CFG_LOAD_AI_MODEL and CFG_LOAD_AI_DETECT are only one can be enable!"
#endif

int load_ai_model_nowait(LINUXBOOT_INFO *p_info);
int operation_ai_prepare(int net_num);
int operation_ai_set_img(uintptr_t y, uintptr_t uv, unsigned int lofs_y, unsigned int w, unsigned int h, unsigned int fmt);
int operation_ai_detect(int net_num);
int operation_ai_get_result(void);
int fast_open_audioout(void);
int fast_trig_audioout(void);
int fast_open_panel(void);
int flow_boot_linux(void);

#endif
