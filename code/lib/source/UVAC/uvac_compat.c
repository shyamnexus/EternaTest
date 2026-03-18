/*
    USB UVAC U2/U3 Compatible Header File


    @file       uvac_compat.h
    @ingroup    mILibUsbUVAC
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.
*/

#include "uvac_compat.h"
#include "U3/UvacVideoTsk_api.h"
#include "U3/UvacDbg.h"
#include "kwrap/type.h"
#include "kwrap/error_no.h"

extern UVAC_OPS UVAC_U3;

UVAC_OPS *guvac_ops = &UVAC_U3;

/**
    Abort the function of UVAC_ReadCdcData().

    @param[in] ComID   COM ID.
*/
void UVAC_AbortCdcRead(CDC_COM_ID ComID);


/**
    Open UVAC(USB VIDEO/AUDIO-Device-Class) module.

    @param[in] pClassInfo Information needed for opening UVAC Task. The user must prepare all the information needed.

    @return
     - @b E_OK:  The UVAC Task open done and success.
     - @b E_SYS: Buffer allocation failed.
     - @b E_PAR: Parameter error.
*/
UINT32 UVAC_Open(UVAC_INFO *pClassInfo)
{
	UINT32 ret = 0;

	if (NULL == guvac_ops) {
		DBG_ERR("UVAC not init\r\n");
		return 0;
	}

	if (guvac_ops->Open) {
		ret = guvac_ops->Open(pClassInfo);
	} else {
		DBG_ERR("Not supported\r\n");
	}

	return ret;
}

/**
    Close UVAC module.
*/
void UVAC_Close(void)
{
	if (NULL == guvac_ops) {
		DBG_ERR("UVAC not init\r\n");
		return;
	}

	if (guvac_ops->Close) {
		guvac_ops->Close();
	} else {
		DBG_ERR("Not supported\r\n");
	}

	return;
}

/**
    Get needed buffer size for UVAC module.
*/
UINT32 UVAC_GetNeedMemSize(void)
{
	UINT32 ret = 0;

	if (NULL == guvac_ops) {
		DBG_ERR("UVAC not init\r\n");
		return 0;
	}

	if (guvac_ops->GetNeedMemSize) {
		ret = guvac_ops->GetNeedMemSize();
	} else {
		DBG_ERR("Not supported\r\n");
	}

	return ret;
}

/**
    Set UVAC configuration

    Assign new configuration of the specified ConfigID.

    @param[in] ConfigID         Configuration identifier
    @param[in] Value            Configuration context for ConfigID
*/
void UVAC_SetConfig(UVAC_CONFIG_ID ConfigID, ULONG Value)
{
	if (NULL == guvac_ops) {
		DBG_ERR("UVAC not init\r\n");
		return;
	}

	if (guvac_ops->SetConfig) {
		guvac_ops->SetConfig(ConfigID, Value);
	} else {
		DBG_ERR("Not supported\r\n");
	}

	return;
}

/**
    Set the stream of a frame information to UVAC lib.
*/
void UVAC_SetEachStrmInfo(PUVAC_STRM_FRM pStrmFrm)
{
	if (NULL == guvac_ops) {
		DBG_ERR("UVAC not init\r\n");
		return;
	}

	if (guvac_ops->SetEachStrmInfo) {
		guvac_ops->SetEachStrmInfo(pStrmFrm);
	} else {
		DBG_ERR("Not supported\r\n");
	}

	return;
}

/**
    Set video resolution.
*/
ER UVAC_ConfigVidReso(PUVAC_VID_RESO pVidReso, UINT32 cnt)
{
	ER ret = E_OK;

	if (NULL == guvac_ops) {
		DBG_ERR("UVAC not init\r\n");
		return E_SYS;
	}

	if (guvac_ops->ConfigVidReso) {
		ret = guvac_ops->ConfigVidReso(pVidReso, cnt);
	} else {
		DBG_ERR("Not supported\r\n");
	}

	return ret;
}





/**
    Read data from USB host via CDC class.
    This API doesn't return until there is any data coming from USB host or CDC_AbortRead() is invoked.

    @param[in] ComID   COM ID.
    @param[in] pBuf   Buffer pointer.
    @param[in,out] pBufSize  Input length to read, output actual transfered length.

    @return
        - @b E_OK:  Success.
        - @b E_SYS: Failed
        - @b E_RSATR: UVAC_AbortCdcRead() is invoked by another task.
*/
INT32 UVAC_ReadCdcData(CDC_COM_ID ComID, void *p_buf, UINT32 buffer_size, INT32 timeout)
{
	ER ret = E_OK;

	if (NULL == guvac_ops) {
		DBG_ERR("UVAC not init\r\n");
		return E_SYS;
	}

	if (guvac_ops->ReadCdcData) {
		ret = guvac_ops->ReadCdcData(ComID, p_buf, buffer_size, timeout);
	}

	return ret;
}

/**
    Abort the function of UVAC_ReadCdcData().

    @param[in] ComID   COM ID.
*/
void UVAC_AbortCdcRead(CDC_COM_ID ComID)
{
	if (NULL == guvac_ops) {
		DBG_ERR("UVAC not init\r\n");
		return;
	}

	if (guvac_ops->AbortCdcRead) {
		guvac_ops->AbortCdcRead(ComID);
	}

	return;
}

/**
    Write data to USB host via CDC class.

    This function is a blocking API and will return after data is sent to USB host.

    @param[in] ComID   COM ID.
    @param[in] pBuf  Buffer pointer
    @param[in,out] pBufSize  Input length to read, output actual transfered length. Valid length is 0x1 to 0x7fffff.

    @return
        - @b E_OK: Start to transfer.
        - @b E_SYS: Failed.
*/
INT32 UVAC_WriteCdcData(CDC_COM_ID ComID, void *p_buf, UINT32 buffer_size, INT32 timeout)
{
	ER ret = E_OK;

	if (NULL == guvac_ops) {
		DBG_ERR("UVAC not init\r\n");
		return E_SYS;
	}

	if (guvac_ops->WriteCdcData) {
		ret = guvac_ops->WriteCdcData(ComID, p_buf, buffer_size, timeout);
	}

	return ret;
}

ER UVAC_PullOutStrm(PUVAC_STRM_FRM pStrmFrm, INT32 wait_ms)
{
	UINT32 ret = 0;

	if (NULL == guvac_ops) {
		DBG_ERR("UVAC not init\r\n");
		return E_SYS;
	}

	if (guvac_ops->PullOutStrm) {
		ret = guvac_ops->PullOutStrm(pStrmFrm, wait_ms);
	} else {
		DBG_ERR("Not supported\r\n");
	}

	return ret;
}

ER UVAC_ReleaseOutStrm(PUVAC_STRM_FRM pStrmFrm)
{
	UINT32 ret = 0;

	if (NULL == guvac_ops) {
		DBG_ERR("UVAC not init\r\n");
		return E_SYS;
	}

	if (guvac_ops->ReleaseOutStrm) {
		ret = guvac_ops->ReleaseOutStrm(pStrmFrm);
	} else {
		DBG_ERR("Not supported\r\n");
	}

	return ret;
}

ER UVAC_WaitStrmDone(UVAC_STRM_PATH path)
{
	ER ret = E_OK;

	if (NULL == guvac_ops) {
		DBG_ERR("UVAC not init\r\n");
		return E_SYS;
	}

	if (guvac_ops->WaitStrmDone) {
		ret = guvac_ops->WaitStrmDone(path);
	} else {
		DBG_ERR("Not supported\r\n");
	}

	return ret;
}

UINT8 UVAC_GetUnitID(UVC_UNIT_TYPE unit)
{
	UINT8 ret = 0;

	if (NULL == guvac_ops) {
		DBG_ERR("UVAC not init\r\n");
		return 0;
	}

	if (guvac_ops->GetUnitID) {
		ret = guvac_ops->GetUnitID(unit);
	} else {
		DBG_ERR("Not supported\r\n");
	}

	return ret;
}

ER UVAC_WriteIntData(UINT8 *pBuf, UINT32 *pBufSize)
{
	ER ret = E_OK;

	if (NULL == guvac_ops) {
		DBG_ERR("UVAC not init\r\n");
		return E_SYS;
	}

	if (guvac_ops->WriteIntData) {
		ret = guvac_ops->WriteIntData(pBuf, pBufSize);
	} else {
		DBG_ERR("Not supported\r\n");
	}

	return ret;
}

INT32 UVAC_ReadHidData(void *p_buf, UINT32 buffer_size, INT32 timeout)
{
	ER ret = E_OK;

	if (NULL == guvac_ops) {
		DBG_ERR("UVAC not init\r\n");
		return E_SYS;
	}

	if (guvac_ops->ReadHidData) {
		ret = guvac_ops->ReadHidData(p_buf, buffer_size, timeout);
	} else {
		DBG_ERR("Not supported\r\n");
	}

	return ret;
}

INT32 UVAC_WriteHidData(void *p_buf, UINT32 buffer_size, INT32 timeout)
{
	ER ret = E_OK;

	if (NULL == guvac_ops) {
		DBG_ERR("UVAC not init\r\n");
		return E_SYS;
	}

	if (guvac_ops->WriteHidData) {
		ret = guvac_ops->WriteHidData(p_buf, buffer_size, timeout);
	} else {
		DBG_ERR("Not supported\r\n");
	}

	return ret;
}

void UVAC_SetObject(UVAC_TYPE type)
{
	if (type == UVAC_TYPE_U3) {
		guvac_ops = &UVAC_U3;
	} else {
		DBG_ERR("Not supported type=%d\r\n", type);
	}

	return;
}

ULONG UVAC_GetConfig(UVAC_CONFIG_ID ConfigID, UINT32 param)
{
	ULONG ret = 0;

	if (NULL == guvac_ops) {
		DBG_ERR("UVAC not init\r\n");
		return E_SYS;
	}

	if (guvac_ops->GetConfig) {
		ret = guvac_ops->GetConfig(ConfigID, param);
	} else {
		DBG_ERR("Not supported\r\n");
	}

	return ret;
}

