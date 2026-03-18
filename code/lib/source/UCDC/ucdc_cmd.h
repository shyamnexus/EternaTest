/**
    UCDC, Service command function declare

    @file       ucdc_cmd.h
    @ingroup    mUCDC

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/
#ifndef _UCDCCMD_H
#define _UCDCCMD_H

#include "USBCDC.h"

/**
   Error Code.

   Any error code occured will display on uart or return.
*/
typedef enum _UCDC_ER {
	UCDC_ER_OK                =   0, ///< no error
	UCDC_ER_NOT_INIT          =  -1, ///< UCDC_Init() is not called or not success
	UCDC_ER_SYS               =  -2, ///< unknown system error
	UCDC_ER_LOCK              =  -3, ///< failed to lock API
	UCDC_ER_UNLOCK            =  -4, ///< failed to unlock API
	UCDC_ER_PARAM             =  -5, ///< wrong Parameters
	UCDC_ER_API_VERSION       =  -6, ///< wrong API version
	UCDC_ER_STA_TASK          =  -7, ///< failed to call sta_tsk
	UCDC_ER_WAIT_IDLE         =  -8, ///< failed to call wai_flg for idle
	UCDC_ER_SET_IDLE          =  -9, ///< failed to call set_flg to idle
	UCDC_ER_NOT_IN_IDLE       = -10, ///< if Prop.bEnterOnlyIdle is true, but system is not idle
	UCDC_ER_STATE             = -11, ///< service state machine is not correctly.
	UCDC_ER_INVALID_CALL      = -12, ///< invald to call this API
	UCDC_ER_OUT_DATA_VOLATILE = -13, ///< the parameter UCDC_CMD.out maybe volatile. (suggest to bExitCmdFinish=TRUE)
	UCDC_ER_CMD_MAXDATA       = -14, ///< internal error. all cmd data structure have to declare in UCDC_CMD_MAXDATA
	UCDC_ER_INVALID_CMD_IDX   = -15, ///< invalid command index
	UCDC_ER_CMD_MAP_NULL      = -16, ///< internal error. cmd function map to a null pointer
	UCDC_ER_CMD_NOT_MATCH     = -17, ///< internal error. g_UCDCCallTbl is not match to UCDC_CMD_IDX
	UCDC_ER_CMD_IN_DATA       = -18, ///< wrong UCDC_CMD.In Data
	UCDC_ER_CMD_OUT_DATA      = -19, ///< wrong UCDC_CMD.Out Data
	UCDC_ER_TMOUT             = -20, ///< time out
	ENUM_DUMMY4WORD(UCDC_ER)
} UCDC_ER;

/**
    Warning Code.

    The warning codes is only displayed via uart.
*/
typedef enum _UCDC_WR {
	UCDC_WR_OK                =   0, ///< no warning
	UCDC_WR_INIT_TWICE        =  -1, ///< UCDC_Init() is called twice
	UCDC_WR_ALREADY_CLOSED    =  -2, ///< service is already closed.
	UCDC_WR_SUSPEND_TWICE     =  -3, ///< UCDC_Suspend maybe is called twice
	UCDC_WR_NOT_IN_SUSPEND    =  -4, ///< call UCDC_Resume, but service isn't in suspend state before.
	UCDC_WR_CMD_SKIP          =  -5, ///< existing a command is skipped
	ENUM_DUMMY4WORD(UCDC_WR)
} UCDC_WR;

/**
    Command Index.

    The command of UCDC service support. Set this index to the UCDC_CMD
    and call the UCDC_Cmd to do command when service is opened.
*/
typedef enum _UCDC_CMD_IDX {
	UCDC_CMD_IDX_UNKNOWN  = 0,          ///< unknown command. maybe forget to set UCDC_CMD.Idx1
	UCDC_CMD_IDX_ABORT_READ,            ///< abort reading data
	UCDC_CMD_IDX_GET_DATA_COUNT,        ///< get data count sent from USB host
	UCDC_CMD_IDX_ASYNC_WRITE_DATA,      ///< asynchronous write data
	UCDC_CMD_IDX_WRITE_DATA,            ///< synchronous write data
	UCDC_CMD_IDX_ASYNC_WAIT_WRITE_DONE, ///< wait asynchronous write done
	UCDC_CMD_IDX_WRITE_DATA_TIMEOUT,    ///< synchronous write data with timeout
	UCDC_CMD_IDX_MAX_NUM,               ///< total command numbers
	ENUM_DUMMY4WORD(UCDC_CMD_IDX)
} UCDC_CMD_IDX;

/**
    Finish information.

    When UCDC_PROP.fpFinishCb assign a callback, the data will send to this
    callback after command finish.
*/
typedef struct _UCDC_FINISH {
	UCDC_CMD_IDX  Idx;        ///< Finished command index
	UCDC_ER       er;         ///< command finish result.
	VOID           *pUserData;  ///< If UCDC_PROP.pUserData is assigned a userdata pointer. service will take it to UCDC_PROP.fpFinishCb
} UCDC_FINISH, *PUCDC_FINISH;

typedef void (*UCDC_CB_FINISH)(const UCDC_FINISH *pInfo); ///< Command finish callback. When any command is finished, the service will callback out.
/**
    Command Property.

    Indicate how to run the current command.
*/
typedef struct _UCDC_PROP {
	BOOL                bEnterOnlyIdle; ///< do job only system idle. if system is busy, the UCDC_ER_NOT_IN_IDLE returned.
	BOOL                bExitCmdFinish; ///< exit API when job finished.
	UCDC_CB_FINISH    fpFinishCb;     ///< (Optional) system will call the callback when job is finished
	VOID               *pUserData;      ///< (Optional) when CbFinish is called, assigned userdata can get from CbFinish parameter.
} UCDC_PROP, *PUCDC_PROP;

/**
    Command Input/Output Data.

    Including data pointer and data size depend on UCDC_CMD_IDX.
*/
typedef struct _UCDC_DATA {
	VOID   *pData;      ///< input/output data pointer
	UINT32  uiNumByte;  ///< input/output data size
} UCDC_DATA, *PUCDC_DATA;

/**
    Command Description.

    The necessary information for running a command via UCDC_Cmd().
*/
typedef struct _UCDC_CMD {
	UCDC_CMD_IDX  Idx;    ///< index
	UCDC_PROP     Prop;   ///< property
	UCDC_DATA     In;     ///< input data description (depend on Idx)
	UCDC_DATA     Out;    ///< output data description (depend on Idx)
} UCDC_CMD, *PUCDC_CMD;


typedef UCDC_ER(*UCDC_FP_CMD)(const UCDC_CMD *pCmd);  ///< for Command Function Table

/**
    Command Function Table Element
*/
typedef struct _UCDC_CMD_DESC {
	UCDC_CMD_IDX  Idx;            ///< Command Index
	UCDC_FP_CMD   fpCmd;          ///< mapped function pointer
	UINT32          uiNumByteIn;    ///< require data in size
	UINT32          uiNumByteOut;   ///< require data out size
} UCDC_CMD_DESC;

const UCDC_CMD_DESC *xUCDC_GetCallTbl(UINT32 *pNum);

UCDC_ER xUCDCCmd_AbortRead(const UCDC_CMD *pCmd);

UCDC_ER xUCDCCmd_GetDataCount(const UCDC_CMD *pCmd);

UCDC_ER xUCDCCmd_AsyncWriteData(const UCDC_CMD *pCmd);

UCDC_ER xUCDCCmd_AsyncWaitWriteDone(const UCDC_CMD *pCmd);

//UCDC_ER xUCDCCmd_AbortWrite(const UCDC_CMD* pCmd);

UCDC_ER xUCDCCmd_WriteData(const UCDC_CMD *pCmd);

UCDC_ER xUCDCCmd_WriteDataTimeout(const UCDC_CMD *pCmd);

#endif
