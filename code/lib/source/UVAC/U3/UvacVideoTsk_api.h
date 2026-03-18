/////////////////////////////////////////////////////////////////
/*
    Copyright (c) 2004~  Novatek Microelectronics Corporation

    @file UvacVideoTsk.h the pc-cam device firmware

    @hardware: Novatek 96610

    @version

    @date

*//////////////////////////////////////////////////////////////////
#ifndef _UVACVIDEOTSKAPI_H
#define _UVACVIDEOTSKAPI_H

#include "../uvac_compat.h"
#include "UVAC.h"

extern void U3UVAC_InstallID(void);
extern void U3UVAC_UnInstallID(void);
extern void U3UVAC_Close(void);
extern UINT32 U3UVAC_GetNeedMemSize(void);
extern void U3UVAC_SetConfig(UVAC_CONFIG_ID ConfigID, ULONG Value);
extern ULONG U3UVAC_GetConfig(UVAC_CONFIG_ID ConfigID, UINT32 param);
extern void U3UVAC_SetEachStrmInfo(PUVAC_STRM_FRM pStrmFrm);
extern void U3UVC_SetTestImg(UINT32 imgAddr, UINT32 imgSize);
extern ER U3UVAC_ConfigVidReso(PUVAC_VID_RESO pVidReso, UINT32 cnt);
extern UINT32 U3UVAC_Open(UVAC_INFO *pClassInfo);
extern ER U3UVAC_ReadCdcData(CDC_COM_ID ComID, void *p_buf, UINT32 buffer_size, INT32 timeout);
extern void U3UVAC_AbortCdcRead(CDC_COM_ID ComID);
extern ER U3UVAC_WriteCdcData(CDC_COM_ID ComID, void *p_buf, UINT32 buffer_size, INT32 timeout);
extern ER U3UVAC_WaitStrmDone(UVAC_STRM_PATH path);
extern ER U3UVAC_PullOutStrm(PUVAC_STRM_FRM pStrmFrm, INT32 wait_ms);
extern ER U3UVAC_ReleaseOutStrm(PUVAC_STRM_FRM pStrmFrm);
extern INT32 U3UVAC_WriteHidData(void *p_buf, UINT32 buffer_size, INT32 timeout);
extern INT32 U3UVAC_ReadHidData(void *p_buf, UINT32 buffer_size, INT32 timeout);

extern UVAC_OPS UVAC_U3;

#endif  // _UVACVIDEOTSK_H

