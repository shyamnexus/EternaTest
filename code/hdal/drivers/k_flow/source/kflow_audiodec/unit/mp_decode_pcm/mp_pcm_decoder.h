/*
    @file       mp_pcm_decoder.h

    @brief      header file of pcm decoder
    @version    V1.01.001
    @date       2018/09/26

    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _MP_PCMDECODER_H
#define _MP_PCMDECODER_H

#include "kwrap/type.h"
#include "audio_decode.h"

//#define PCM_DECODEID  0x74776f73//twos


typedef struct {
	ULONG  startAddr;
	ULONG  endAddr;
	ULONG  maxAddr;
	ULONG  nowAddr; //addr of raw output
	ULONG  usedAddr;//addr of getting from audio
	UINT32 outputRawSize;//start from usedAddr, unused audio size
	UINT32 maxRawSize;//max unused audio size //2010/08/03 Meg Lin

} PCM_ADDRINFO;

extern ER MP_PCMDec_init(MP_AUDDEC_ID AudDecId, AUDIO_PLAYINFO *pobj);
extern ER MP_PCMDec_getInfo(MP_AUDDEC_ID AudDecId, MP_AUDDEC_GETINFO_TYPE type, ULONG *pparam1, ULONG *pparam2, ULONG *pparam3);
extern ER MP_PCMDec_setInfo(MP_AUDDEC_ID AudDecId, MP_AUDDEC_SETINFO_TYPE type, ULONG param1, ULONG param2, ULONG param3);
extern ER MP_PCMDec_decodeOne(MP_AUDDEC_ID AudDecId, UINT32 type, ULONG BsAddr, UINT32 BsSize);
extern ER MP_PCMDec_waitDecodeDone(MP_AUDDEC_ID AudDecId, UINT32 type, UINT32 *p1, UINT32 *p2, UINT32 *p3);




#endif //_MP_PCMDECODER_H

