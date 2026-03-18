/**
    Header file of media fileformat reading library

    Exported header file of media fileformat reading library.

    @file       AVFile_ParserTs.h
    @ingroup    mIMEDIAREAD
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#include "kwrap/type.h"
#include "avfile/MediaReadLib.h"

#ifndef _AVFILE_PARSERTS_H
#define _AVFILE_PARSERTS_H

#define TS_CHECK_COUNT 10
#define TS_PACKET_SIZE 188
#define TS_VID_PESHEADER_LENGTH 14    // length of pes header
#define TS_AUD_PESHEADER_LENGTH 14
#define TS_VID_SPSPPS_LENGTH 33
#define TS_VID_264_NAL_LENGTH 6       // length of nal (0x00 00 00 01 09 10)
#define TS_VID_265_NAL_LENGTH 7       // length of nal (0x00 00 00 01 46 01 10)
#define TS_AUD_ADTS_LENGTH 7

/* pids */
#define PAT_PID 0x0000
#define TS_GPS_PID            768
#define TS_THUMBNAIL_PID      769

/* table ids */
#define PAT_TID 0x00
#define PMT_TID 0x02

/* filter amount */
#define FILTER_NUM 5

#define TSVID_ENTRY_SIZE 18000
#define TSAUD_ENTRY_SIZE 18000

enum MpegTSFilterType {
	MPEGTS_PES,
	MPEGTS_SECTION,
	MPEGTS_PCR,
};

typedef enum {
	PESTYPE_NONE,
	PESTYPE_AUDIO,
	PESTYPE_VIDEO,
} CurPesType;

typedef enum {
	SEEKRESULT_AHEAD,
	SEEKRESULT_BEHIND,
	SEEKRESULT_ONEMORE,
} SeekResult;

typedef struct _MpegTSFilter {
	UINT32 pid;
	UINT32 es_id;
	UINT32 last_cc; /* last cc code (-1 if first packet) */
	UINT32 program_number;
	UINT64 last_pcr;
	enum MpegTSFilterType type;
	//union {
	//    MpegTSPESFilter pes_filter;
	//    MpegTSSectionFilter section_filter;
	//} u;
} MpegTSFilter;

typedef struct _TsFilterManager {
	MpegTSFilter vidFilter;
	MpegTSFilter audFilter;
	MpegTSFilter patFilter;
	MpegTSFilter pmtFilter;
	MpegTSFilter pcrFilter;
} TsFilterManager;

typedef struct _TsPlayInfo {
	UINT64        pcrValue;
	UINT64        pcrStartValue;
	UINT64        pcrEndValue;
	UINT64        firstPts;
	UINT64        lastPts;
	UINT32        totalMs;
	UINT32        totalVidFrame;
	UINT32        vidWidth;
	UINT32        vidHeight;
	UINT32        vidTkWidth;
	UINT32        vidTkHeight;
	ULONG         demuxAddr;
	UINT32        vidFPS;
	UINT32        audFrmIndex;
	UINT32        audFrameSize;
	UINT32        audChannel;
	UINT32        vidFrmIndex;
	UINT32        vidFrameSize;
	UINT32        gopNumber;
	ULONG         thumbAddr;
	UINT32        thumbSize;
	UINT32        keyFrameDist;
	UINT32        vidCodecType;
	UINT32        audCodecType;
	UINT32        vidNalLength;
	UINT16        h264VidDescLen;
	_ALIGNED(32) unsigned char h264VidDesc[0xB0];
	CurPesType    currentPesType;
	BOOL          demuxFinished;
} TsPlayInfo;

typedef struct _NAL_INFO {
	UINT32 width;
	UINT32 height;
} NAL_INFO;

typedef struct {
	ULONG  memAddr;
	UINT32 size;
	UINT16 key_frame;
	UINT32 frameIdx;
} TS_FRAME_ENTRY;

typedef struct {
	UINT8  data[188];
	UINT32 pid;
	UINT32 splitted1stHalfSize;
} SPLITTED_TS_PACKET;

typedef struct {
	UINT64 targetPts;
	UINT64 diff;
	SeekResult result;
} TS_SEEK_INFO;

/**
    @addtogroup mIMEDIAREAD
*/
//@{

/**
    @name   Media File Container Function Instances
*/
//@{

/**
    Get media TS file container structure

    Get media file container function structure.

    @return CONTAINERPARSER pointer
*/
extern CONTAINERPARSER *MP_TsReadLib_GerFormatParser(void);

extern ER TsReadLib_ConfigFilters(uintptr_t addr, UINT32 size);
extern ER TsReadLib_ParsePAT(UINT8 *data);
extern ER TsReadLib_ParsePMT(UINT8 *data);
extern ER TsReadLib_ParseADTS(UINT8 *data);
extern ER TsReadLib_ParseStuffing(UINT8 *data, UINT32 *size);
extern UINT64 TsReadLib_ParsePCR(UINT8 *data);
extern UINT64 TsReadLib_ParsePTS(UINT8 *data);
extern BOOL TsReadLib_AESDECRYP2(UINT8 *PlanText,UINT8 *uiKey,UINT8 *CypherText);
extern ER TsReadLib_Decrypt(uintptr_t addr, UINT32 size);
extern INT32 TsReadLib_IsDecrypt(void);
extern ER TsReadLib_SetDecryptKey(UINT8 *key);
extern ER TsReadLib_DemuxTsStream(uintptr_t srcAddr, UINT32 readSize, uintptr_t dstAddr, UINT32 *usedSize);
extern ER TsReadLib_Parse1stVideo(uintptr_t hdrAddr, UINT32 hdrSize, void *pobj);
extern ER TsReadLib_SaveFrameSize2Table(void);
extern void TsReadLib_SaveIncompletePacketInfo(UINT32 remainLength, UINT8 *data);
extern void TsReadLib_SaveIncompleteFrameInfo(void);
extern void TsReadLib_RecoverFrame(UINT8 *data, uintptr_t dstAddr, UINT32 *buff_offset);
extern void TsReadLib_RecoverTsPacket(UINT8 *data, uintptr_t dstAddr, UINT32 *buff_offset,
									  UINT32 splitted2ndHalfSize);
extern void TsReadLib_ClearFrameEntry(void);
extern BOOL TsReadLib_DemuxFinished(void);
extern void TsReadLib_setDemuxFinished(BOOL finished);
extern void TsReadLib_registerTSDemuxCallback(void (*demuxCB)(UINT32, UINT32));
extern void TsReadLib_ParseEndInfo(uintptr_t srcAddr, UINT32 readSize);
extern void TsReadLib_PrepareSeek(UINT32 vidIdx, UINT32 audIdx);
extern ULONG TsReadLib_SeekTsStream(uintptr_t srcAddr, UINT32 readSize, UINT64 targetPts);
extern void TsReadLib_ParseThumbInfo(uintptr_t srcAddr, UINT32 readSize, uintptr_t dstAddr);
extern UINT32 TsReadLib_GetNextIFrame(UINT32 uiFrmIdx, UINT32 uiSkipI);
extern ER TsReadLib_SearchIFrame(uintptr_t srcAddr, UINT32 srcSize, uintptr_t dstAddr, UINT32 *dstSize, UINT32 *direction);
//extern ER   TsReadLib_ParseNextIFrame(); // [TestSkip]
/**
    Parse Thumbnail Information

    Parse Thumbnail Information

    @param[in] hdrAddr    not used.
    @param[in] hdrSize    not used.
    @param[in] pobj       MEDIA_FIRST_INFO object

    @return ER
*/
//extern ER MovReadLib_ParseThumbnail(UINT32 hdrAddr, UINT32 hdrSize, void *pobj);


/**
    Parse Video Information

    Parse Video Information

    @param[in] hdrAddr    not used.
    @param[in] hdrSize    not used.
    @param[in] pobj       MEDIA_FIRST_INFO object

    @return ER
*/
//extern ER MovReadLib_ParseVideoInfo(UINT32 hdrAddr, UINT32 hdrSize, void *pobj);
/**
    Register Callback function

    Register callback function for file read and output message.

    @param[out] pobj   CONTAINERPARSER object.

    @return ER
*/
extern ER TsReadLib_RegisterObjCB(void *pobj);//pContainer = (CONTAINERPARSER *)pobj;



//-------------------------------------- nal parser --------------------------------------
extern INT32 TsReadLib_ReadBit(void);
extern INT32 TsReadLib_ReadBits(INT32 n);
extern void  TsReadLib_SkipBit(UINT32 num);
extern INT32 TsReadLib_ReadExpGolCode(void);
extern void  TsReadLib_SkipExpGolCode(void);
extern INT32 TsReadLib_ReadSE(void);
extern void  TsReadLib_SkipSE(void);
extern void TsReadLib_Parse(const unsigned char *pStart, unsigned short nLen);

#endif //_AVFILE_PARSERTS_H

