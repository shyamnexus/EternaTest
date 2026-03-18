#ifndef _KDRV_VDOCDC_COMN_H_
#define _KDRV_VDOCDC_COMN_H_

#include "kwrap/type.h"
#include "kwrap/verinfo.h"

#include "kdrv_videoenc/kdrv_videoenc.h"

#include "h26x_def.h"

typedef struct _vdoenc_info_{
	H26XENC_VAR enc_var;
	BOOL   b_enable;
	BOOL   b_rst_i_frm;
	BOOL   b_is_i_frm;
	BOOL   b_emode_en;
	BOOL   b_gdpr_svn;
	//// smart ////
	BOOL   smart_roi_update;

	UINT64 setup_time;
	UINT64 start_time;
	UINT64 end_time;
	void (*vdoenc_3dnr_cb)(UINT32 path_id, ULONG config);
	void (*vdoenc_spn_cb)(UINT32 path_id, ULONG config);
	void (*vdoenc_slice_low_latency_cb)(void *info);
	BOOL   b_md_valid;
	void (*vdoenc_isp_param_cb)(UINT32 path_id, ULONG config);
} vdoenc_info;

typedef struct _vdodec_info_{
	H26XDEC_VAR dec_var;
	BOOL   b_enable;
	UINT64 setup_time;
	UINT64 start_time;
	UINT64 end_time;
	INT32 (*vdodec_cb)(VOID *callback_info, VOID *user_data);
} vdodec_info;

typedef struct _vdoenc_int_info_{
	UINT32 int_val;
    UINT8  int_name[16];
} vdoenc_int_info;

//extern vdoenc_info g_enc_info[KDRV_VDOENC_ID_MAX];
extern vdoenc_info *g_enc_info;
extern int vdoenc_max_channel;

#define KDRV_VDOCDC_VERSION_0 (1)	//28
#define KDRV_VDOCDC_VERSION_1 (00)	//20,24,
#define KDRV_VDOCDC_VERSION_2 (000)	//8,12,16
#define KDRV_VDOCDC_VERSION_3 (00)	//0,4

#endif // _KDRV_VDOCDC_COMN_H_
