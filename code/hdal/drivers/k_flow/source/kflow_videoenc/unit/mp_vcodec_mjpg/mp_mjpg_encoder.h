#ifndef _MP_MJPGENCODER_H
#define _MP_MJPGENCODER_H


#include "kwrap/type.h"
#define _SECTION(sec)

#define MP_VDOENC_MJPEG_ID_MAX   (MP_VDOENC_ID_MAX+1)  // make pseudo path for internal JPEG encode snapshot
#define MP_VDOENC_MJPEG_ID_33    (MP_VDOENC_ID_MAX)

typedef struct _MJPEG_INFO_PRIV {
	UINT32    width;
	UINT32    height;
	UINT32    yuv_format;
} MJPEG_INFO_PRIV;

extern ER MP_MjpgEnc_init(MP_VDOENC_ID VidEncId, MP_VDOENC_INIT *pVidEncInit);
extern ER MP_MjpgEnc_close(MP_VDOENC_ID VidEncId);
extern ER MP_MjpgEnc_getInfo(MP_VDOENC_ID VidEncId, MP_VDOENC_GETINFO_TYPE type, VOID *p1, VOID *p2, VOID *p3);
extern ER MP_MjpgEnc_setInfo(MP_VDOENC_ID VidEncId, MP_VDOENC_SETINFO_TYPE type, ULONG param1, ULONG param2, ULONG param3);
#ifdef VDOENC_LL
extern ER (*MP_MjpgEnc_encodeOne)(MP_VDOENC_ID VidEncId, UINT32 type, MP_VDOENC_PARAM *ptr, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data);
#else
extern ER (*MP_MjpgEnc_encodeOne)(MP_VDOENC_ID VidEncId, UINT32 type, uintptr_t outputAddr, UINT32 *pSize, MP_VDOENC_PARAM *pVidEncParam);
#endif

#endif //_MP_MJPGENCODER_H

