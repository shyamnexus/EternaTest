#ifndef _MP_MJPGDECODER_H
#define _MP_MJPGDECODER_H


#include "kwrap/type.h"
#define _SECTION(sec)


extern ER MP_MjpgDec_init(MP_VDODEC_ID VidDecId);
extern ER MP_MjpgDec_close(MP_VDODEC_ID VidDecId);
extern ER MP_MjpgDec_getInfo(MP_VDODEC_ID VidDecId, MP_VDODEC_GETINFO_TYPE type, VOID *p1, VOID *p2, VOID *p3);
extern ER MP_MjpgDec_setInfo(MP_VDODEC_ID VidDecId, MP_VDODEC_SETINFO_TYPE type, ULONG param1, ULONG param2, ULONG param3);
extern ER (*MP_MjpgDec_decodeOne)(MP_VDODEC_ID VidDecId, UINT32 type, VOID *pVidDecParam, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data);

#endif //_MP_MJPGDECODER_H

