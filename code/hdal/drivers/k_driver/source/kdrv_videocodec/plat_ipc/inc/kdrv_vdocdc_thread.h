#ifndef _KDRV_VDOCDC_THREAD_H_
#define _KDRV_VDOCDC_THREAD_H_

#include "kwrap/type.h"
#include "kwrap/task.h"
#include "kdrv_type.h"

#include "kdrv_videoenc/kdrv_videoenc.h"
#include "kdrv_videodec/kdrv_videodec.h"

typedef enum _KDRV_VDOCDC_MODE_{
	VDOCDC_ENC_MODE = 0,
	VDOCDC_DEC_MODE,
	VDOCDC_NOT_SUPPORT
} KDRV_VDOCDC_MODE;

typedef enum _KDRV_VDOCDC_JOB_STATUS_{
	VDOCDC_JOB_IDLE = 0,
	VDOCDC_JOB_STANDBY,
	VDOCDC_JOB_RUN_START,
	VDOCDC_JOB_RUN_ONGOING,
	VDOCDC_JOB_RUN_RETRIGGER,
	VDOCDC_JOB_RUN_DONE,
	VDOCDC_JOB_FINISH,
} KDRV_VDOCDC_JOB_STATUS;

typedef struct _KDRV_VDOCDC_JOB_{
	UINT64 setup_time;
	UINT64 start_time;
	UINT64 end_time;

	uintptr_t apb_addr;
	UINT32 interrupt;
	KDRV_VDOCDC_JOB_STATUS status;

	void *next_job;

	UINT32 id;
	KDRV_VDOCDC_MODE codec_mode;
	KDRV_CALLBACK_FUNC *callback;
	void *user_data;
	void *p_param;
	void (*vdoenc_osdmask_cb)(UINT32 path_id, UINT32 isDo);

	void *info;
} KDRV_VDOCDC_JOB;

int kdrv_vdocdc_create_thread(void);
int kdrv_vdocdc_remove_thread(void);
int kdrv_vdocdc_add_job(KDRV_VDOCDC_MODE codec_mode, UINT32 id, uintptr_t uiVaApbAddr, KDRV_CALLBACK_FUNC *p_cb_func, void *user_data, void *p_param, void *job_info);
int kdrv_vdocdc_stop_job(UINT32 stop_job_id);

int kdrv_vdocdc_set_llc_mem(UINT32 addr, UINT32 size);
UINT32 kdrv_vdocdc_get_llc_mem(void);
BOOL kdrv_vdocdc_get_job_done(void);
BOOL kdrv_vdocdc_get_enc_job_done(void);
BOOL kdrv_vdocdc_get_dec_job_done(void);
void kdrv_vdocdc_set_osdmask_cb(KDRV_VDOENC_OSDMASKCB *pOsdMaskCB);

#endif // _KDRV_VDOCDC_THREAD_H_
