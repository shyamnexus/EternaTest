/*
    Header file of mov recorder.

    Exported header file of mov fileformat writer.

    @file       movRec.h
    @ingroup    mIAVMOV
    @note       Nothing.
    @version    V1.00.000
    @date       2019/05/05
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/
#ifndef _MP_TSREC_H
#define _MP_TSREC_H

/*-----------------------------------------------------------------------------*/
/* Macro Constant Definitions                                                  */
/*-----------------------------------------------------------------------------*/
#define TSWRITER_COUNT      16

#define TS_VIDEOPES_HEADERLENGTH  14  //(9+PTS 5bytes)
#define TS_AUDIOPES_HEADERLENGTH  14  //(9+PTS 5bytes)
#define TS_VIDEO_264_NAL_LENGTH    6
#define TS_VIDEO_265_NAL_LENGTH    7
#define TS_AUDIO_ADTS_LENGTH       0
#define TS_PTS_LENGTH              5
#define TS_PES_PACKET_LENGTH_END   6  //(start_code 3 +Stream_id 1 +PES_packet_length 2)

#define TS_IDTYPE_VIDPATHID      2//video pathid
#define TS_IDTYPE_AUDPATHID      3//audio pathid

#define TS_CODEC_MJPG            1       ///< motion jpeg
#define TS_CODEC_H264            2       ///< h.264
#define TS_CODEC_H265            3      ///< h.265

/*-----------------------------------------------------------------------------*/
/* Extern Functions                                                            */
/*-----------------------------------------------------------------------------*/
extern ER TSWriteLib_Initialize(UINT32 idnum);
extern ER TSWriteLib_SetMemBuf(UINT32 idnum, ULONG startAddr, UINT32 size);
extern ER TSWriteLib_GetInfo(MEDIAWRITE_GETINFO_TYPE type, VOID *pparam1, VOID *pparam2, VOID *pparam3);
extern ER TSWriteLib_SetInfo(MEDIAWRITE_SETINFO_TYPE type, ULONG param1, ULONG param2, ULONG param3);

extern PCONTAINERMAKER ts_getContainerMaker(void);
extern void TSWriteLib_MakePesHeader(UINT32 fstskid, ULONG srcAddr, UINT32 payload_size, UINT32 idtype);

extern void TS_Write_PTS(UINT8 *q, UINT64 pts);
extern void TS_Write_VidNAL(UINT32 fstskid, UINT8 *q);
extern void TS_Write_AudADTS(UINT8 *q, UINT32 bsSize);
extern void TS_Write_Stuffing(UINT8 *q, UINT32 size);

#endif
