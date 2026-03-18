/*
    @file       mp_ppcm_encoder.h

    @brief      header file of Packed-PCM encoder
    @version    V1.01.001
    @date       2018/08/30

    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _MP_PPCMENCODER_H
#define _MP_PPCMENCODER_H

#include "kwrap/type.h"
#include "audio_encode.h"

extern ER MP_PPCMEnc_init(MP_AUDENC_ID AudEncId);
extern ER MP_PPCMEnc_getInfo(MP_AUDENC_ID AudEncId, MP_AUDENC_GETINFO_TYPE type, ULONG *p1, ULONG *p2, ULONG *p3);
extern ER MP_PPCMEnc_setInfo(MP_AUDENC_ID AudEncId, MP_AUDENC_SETINFO_TYPE type, ULONG param1, ULONG param2, ULONG param3);
extern ER MP_PPCMEnc_encodeOne(MP_AUDENC_ID AudEncId, UINT32 type, ULONG outputAddr, UINT32 *pSize, MP_AUDENC_PARAM *ptr);
//extern ER MP_PPCMEnc_customizeFunc(UINT32 type, void *ptr);//2012/07/09 Meg

#endif //_MP_PPCMENCODER_H
