#ifndef _MP_H264DECODER_H
#define _MP_H264DECODER_H

#include "kwrap/type.h"
#include "video_decode.h"
#ifdef __KERNEL__
#include <linux/module.h>
#endif

extern ER MP_H264Dec_init(MP_VDODEC_ID VidDecId, MP_VDODEC_INIT *pVidDecInit);
extern ER MP_H264Dec_close(MP_VDODEC_ID VidDecId);
extern ER MP_H264Dec_getInfo(MP_VDODEC_ID VidDecId, MP_VDODEC_GETINFO_TYPE type, VOID *p1, VOID *p2, VOID *p3);
extern ER MP_H264Dec_setInfo(MP_VDODEC_ID VidDecId, MP_VDODEC_SETINFO_TYPE type, ULONG param1, ULONG param2, ULONG param3);
extern ER (*MP_H264Dec_decodeOne)(MP_VDODEC_ID VidDecId, UINT32 type, VOID *pVidDecParam, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data);
extern ER MP_H264Dec_triggerDec(MP_VDODEC_ID VidDecId, VOID *pVidDecParam);
extern ER MP_H264Dec_waitDecReady(MP_VDODEC_ID VidDecId, VOID *pVidDecParam);

#endif //_MP_H264DECODER_H
