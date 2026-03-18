/*
    Copyright   Novatek Microelectronics Corp. 2004.  All rights reserved.

    @file       UvacVideoTsk.c
    @ingroup    mISYSUVAC

    @brief      UVAC Video Task.
                UVAC Video Task.

*/

#include "UvacVideoTsk_api.h"

UVAC_OPS UVAC_U3 =
{
	UVAC_TYPE_U3,
	U3UVAC_Open,
	U3UVAC_Close,
	U3UVAC_GetNeedMemSize,
	U3UVAC_SetConfig,
	U3UVAC_SetEachStrmInfo,
	U3UVAC_ConfigVidReso,
	U3UVAC_ReadCdcData,
	U3UVAC_AbortCdcRead,
	U3UVAC_WriteCdcData,
	U3UVAC_PullOutStrm, //PullOutStrm
	U3UVAC_ReleaseOutStrm, //ReleaseOutStrm
	U3UVAC_WaitStrmDone,
	NULL, //GetUnitID
	NULL, //WriteIntData
	U3UVAC_ReadHidData,
	U3UVAC_WriteHidData,
	U3UVAC_GetConfig
};