/**
    UCDC, Internal function declare

    @file       ucdc_int.h
    @ingroup    mUCDC

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/
#ifndef _UCDCINT_H
#define _UCDCINT_H

#include "kwrap/type.h"
#include "USBCDC.h"
#include "ucdc_cmd.h"
#include "kwrap/error_no.h"
#include "kwrap/task.h"
#include "kwrap/flag.h"
#include "kwrap/semaphore.h"
#include "kwrap/sxcmd.h"
#include "string.h"
/**
*   Internal configuration
*/
//@{
#define CFG_UCDC_INIT_KEY     MAKEFOURCC('N','O','V','A') ///< a key value 'N','O','V','A' for indicating system initial.
//@}

#define THIS_DBGLVL         2 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
///////////////////////////////////////////////////////////////////////////////
#define __MODULE__          UCDC
#define __DBGLVL__          THIS_DBGLVL
#define __DBGFLT__          "*" //*=All, [mark]=CustomClass
#include <kwrap/debug.h>
/**
    Flag Pattern
*/
//@{
//Task Operation
#define FLGUCDC_OPEN      FLGPTN_BIT(0)   //not used, currently.
#define FLGUCDC_CLOSE     FLGPTN_BIT(1)   //notify task don't do any command for task close
#define FLGUCDC_SUSPEND   FLGPTN_BIT(2)   //notify task don't do any command for task suspend
#define FLGUCDC_RESUME    FLGPTN_BIT(3)   //notify task start to receive command
#define FLGUCDC_CMD       FLGPTN_BIT(4)   //notify task there's command coming
#define FLGUCDC_IDLE      FLGPTN_BIT(5)   //task is idle or just finish his job
#define FLGUCDC_POLLING   FLGPTN_BIT(6)   //safe polling enter idle
#define FLGUCDC_BULKOUT2  FLGPTN_BIT(7)   //
#define FLGUCDC_BULKIN1   FLGPTN_BIT(8)   //
#define FLGUCDC_ABORTREAD FLGPTN_BIT(9)   //
//Task Finish State
#define FLGUCDC_OPENED    FLGPTN_BIT(28)  //indicate task can start to recevie command
#define FLGUCDC_STOPPED   FLGPTN_BIT(29)  //indicate task start to skip all command to wait terminate task
#define FLGUCDC_SUSPENDED FLGPTN_BIT(30)  //indicate task start to skip all command
#define FLGUCDC_RESUMED   FLGPTN_BIT(31)  //task start to receive command
//@}

#define CDC_BULKIN_EP          USB_EP1
#define CDC_BULKOUT_EP         USB_EP2
#define CDC_INTRIN_EP          USB_EP3

/**
    State Machine.

    Indicate the task state.
*/
typedef enum _UCDC_STATE {
	UCDC_STATE_UNKNOWN    = 0,    ///< unknown. only existing service is not opened.
	UCDC_STATE_OPEN_BEGIN,        ///< start to open a task
	UCDC_STATE_OPEN_END,          ///< finished to do open flow
	UCDC_STATE_CLOSE_BEGIN,       ///< start to close
	UCDC_STATE_CLOSE_END,         ///< task is terminated.
	UCDC_STATE_SUSPEND_BEGIN,     ///< start to enter suspend state
	UCDC_STATE_SUSPEND_END,       ///< task is in suspend state
	UCDC_STATE_RESUME_BEGIN,      ///< start to resume service
	UCDC_STATE_RESUME_END,        ///< finished to resume, task start to recevice command
	UCDC_STATE_CMD_BEGIN,         ///< start to do a command
	UCDC_STATE_CMD_END,           ///< command finished
	UCDC_STATE_CMD_CB_BEGIN,      ///< start to call a callback when command finished
	UCDC_STATE_CMD_CB_END,        ///< exit to callback when command finished
	UCDC_STATE_STATUS_CB_BEGIN,   ///< start to call a callback when status change
	UCDC_STATE_STATUS_CB_END,     ///< exit to callback when status change
	UCDC_STATE_ER_CB_BEGIN,       ///< start to call a callback when error occured
	UCDC_STATE_ER_CB_END,         ///< exit to callback when error occured
	UCDC_STATE_MAX_NUM,           ///< total state numbers
	ENUM_DUMMY4WORD(UCDC_STATE)
} UCDC_STATE;

typedef void (*UCDC_FP_STATE_CB)(UCDC_STATE State); ///< State callback. If there's any state changed, the service will callback out.
typedef void (*UCDC_FP_ERROR_CB)(UCDC_ER er); ///< Error callback. If there's any error occured, the service will callback out.
/**
    Initial Data.

    For system inital via UCDC_Init().
*/
typedef struct _UCDC_INIT {
	UINT32              uiApiVer;   ///< just assign to UCDC_API_VERSION
	THREAD_HANDLE       TaskID;     ///< given a task id form project layer creation (can set 0 to use system default)
	ID                  SemID;      ///< given a semaphore id form project layer creation (can set 0 to use system default)
	ID                  FlagID;     ///< given a flag id form project layer creation (can set 0 to use system default)
	UCDC_FP_STATE_CB  fpStateCb;  ///< (Optional) system will call the callback when state change
	UCDC_FP_ERROR_CB  fpErrorCb;  ///< (Optional) if there is error occured, callback will be performed for debug
} UCDC_INIT, *PUCDC_INIT;

/**
*   Task condition
*/
typedef struct _UCDC_TASK_CONDITION {
	UINT32 uiInitKey;   ///< indicate module is initail
	BOOL   bOpened;     ///< indicate task is open
	BOOL   bSuspended;  ///< indicate task is suppend
	BOOL   bStopped;    ///< indicate task is close
} UCDC_TASK_CONDITION;

/**
*   Debug information
*/
typedef struct _UCDC_DBGINFO {
	UCDC_ER LastEr;               ///< last error code (if any success, this variable will not be clean)
	UCDC_WR LastWr;               ///< last warning code (if any success, this variable will not be clean)
	UCDC_CMD_IDX LastCmdIdx;      ///< last command index
	THREAD_HANDLE  LastCallerTask;  ///< last command caller task id
	UINT32         LastCallerAddr;  ///< last command caller address
} UCDC_DBGINFO;

/**
    Maxium Data.
*/
typedef union _UCDC_CMD_MAXDATA {
	union {
		UINT32              uiReserve;  ///< just reserve. for no any other structure
	} MAXDATA;
} UCDC_CMD_MAXDATA;

/**
    Command controller
*/
typedef struct _UCDC_CMD_CTRL {
	const UCDC_CMD_DESC  *pFuncTbl;   ///< command function table
	UINT32                  uiNumFunc;  ///< total items in command function table
	UCDC_CMD              Cmd;        ///< received UCDC_Cmd command data
	UCDC_CMD_MAXDATA      MaxInBuf;   ///< duplicated command input data
	UCDC_ER               erCmd;      ///< command error code after command finish
} UCDC_CMD_CTRL;

/*
*   UCDC Manager Control Object
*/
typedef struct _UCDC_CTRL {
	UCDC_INIT             Init;       ///< duplicated initial data
	UCDC_TASK_CONDITION   Cond;       ///< task condition object
	UCDC_STATE            State;      ///< task state machine
	UCDC_DBGINFO          DbgInfo;    ///< debug information
	UCDC_CMD_CTRL         CmdCtrl;    ///< command controllor data
	CDC_TYPE              cdc_type;
} UCDC_CTRL;

/*
*   UCDC Read Object
*/
typedef struct _UCDC_READ {
	UINT8  *pBuf;       ///< buffer pointer
	UINT32 *pBufSize;   ///< pointer to buffer size
} UCDC_READ, *PUCDC_READ;

/*
*   UCDC Write Object
*/
typedef struct _UCDC_WRITE {
	UINT8  *pBuf;       ///< buffer pointer
	UINT32 *pBufSize;   ///< pointer to buffer size
	UINT32 timeout;   ///< pointer to timeout value
} UCDC_WRITE, *PUCDC_WRITE;

/*
*   UCDC Config Object
*/
typedef struct _UCDC_CONFIG {
	CDC_CONFIG_ID  configID;    ///< config ID
	UINT32         value;       ///< config value
} UCDC_CONFIG, *PUCDC_CONFIG;

enum CDC_LineEncodingFormats_t {
	CDC_LINEENCODING_OneStopBit          = 0, /**< Each frame contains one stop bit. */
	CDC_LINEENCODING_OneAndAHalfStopBits = 1, /**< Each frame contains one and a half stop bits. */
	CDC_LINEENCODING_TwoStopBits         = 2, /**< Each frame contains two stop bits. */
};
/** Enum for the possible line encoding parity settings of a virtual serial port. */
enum CDC_LineEncodingParity_t {
	CDC_PARITY_None  = 0, /**< No parity bit mode on each frame. */
	CDC_PARITY_Odd   = 1, /**< Odd parity bit mode on each frame. */
	CDC_PARITY_Even  = 2, /**< Even parity bit mode on each frame. */
	CDC_PARITY_Mark  = 3, /**< Mark parity bit mode on each frame. */
	CDC_PARITY_Space = 4, /**< Space parity bit mode on each frame. */
};

UCDC_CTRL    *xUCDC_GetCtrl(void);
UCDC_ER       xUCDC_Err(UCDC_ER er);
UCDC_WR       xUCDC_Wrn(UCDC_WR wr);
UCDC_ER       xUCDC_Lock(void);
UCDC_ER       xUCDC_Unlock(void);
BOOL            xUCDC_ChkIdle(void);
UCDC_ER       xUCDC_WaitIdle(void);
UCDC_ER       xUCDC_SetIdle(void);
UCDC_ER       xUCDC_SetState(UCDC_STATE State);
UCDC_STATE    xUCDC_GetState(void);

/**
    UCDC main task.

    Task body. Just set it into user_systasktabl in SysCfg.c. The stack size
    suggest to 4096KB. If there are many callbacks have plug-in, the stack
    size maybe need to more.
*/
THREAD_DECLARE(UCDCTsk, arglist);

/**
    Initialize UCDC service.

    For using the UCDC service, the task id, flag id, semphore id have to
    prepare by project layer. Assign these ids to the UCDC_INIT strcuture
    when system starup (just set once).

    @param[in] pInit necessary inital data.
    @return
        - @b UCDC_ER_OK: inital success.
        - @b UCDC_ER_API_VERSION: UCDCApi.h and AppUCDC.a not match.
*/
UCDC_ER xUCDC_Init(const UCDC_INIT *pInit);

/**
    Start to open the UCDC service.

    After UCDC service is initialized, call UCDC_Open to start the
    service.

    @return
        - @b UCDC_ER_OK: open success.
        - @b UCDC_ER_STATE: may UCDC_Init does not be called yet.
        - @b UCDC_ER_STA_TASK: failed to start task(sta_tsk failed).
*/
UCDC_ER xUCDC_Open(USB_CDC_INFO *pClassInfo);

/**
    Stop to open the UCDC service.

    When UCDC service is running, call UCDC_Close to terminate the task
    service.

    @note: Don't call UCDC_Close in any UCDC service callback. In
    callback, the UCDC_AsyncClose is allowed to call to stop recevice receive
    data but task is not terminated. The UCDC_Close need to be called outside.

    @return
        - @b UCDC_ER_OK: close success.
        - @b UCDC_ER_INVALID_CALL: call UCDC_Close in callback.
        - @b UCDC_ER_STA_TASK: failed to start task(sta_tsk failed).
*/
UCDC_ER xUCDC_Close(VOID);

/**
    Suspend the service task.

    When UCDC service is running, call UCDC_Suspend to stop recevice
    command. Any command will be skiped by service task. The function exit after
    task finish his suspend.

    @return
        - @b UCDC_ER_OK: suspend success.
        - @b UCDC_ER_STATE: may UCDC_Open does not be opened yet.
*/
UCDC_ER xUCDC_Suspend(VOID);

/**
    Resume the service task.

    When UCDC service is suspended, call UCDC_Resume to start to
    recevice command. The function exit after task finish his resume.

    @return
        - @b UCDC_ER_OK: suspend success.
        - @b UCDC_ER_STATE: may UCDC_Open does not be opened yet.
*/
UCDC_ER xUCDC_Resume(VOID);

/**
    Set a command action to UCDC service.

    Configure UCDC_CMD and send it to the service to do some action via
    UCDC_Cmd.

    @param[in] pCmd command description.
    @return
        - @b UCDC_ER_OK: send command success.
        - @b UCDC_ER_CMD_NOT_MATCH: internal error. Please contact to module
             maintenance staff.
        - @b UCDC_ER_CMD_IN_DATA: the UCDC_CMD.In maybe assing wrong object
             / structure
        - @b UCDC_ER_CMD_IN_DATA: the UCDC_CMD.Out maybe assing wrong object
             / structure
        - @b UCDC_ER_STATE: may UCDC_Open does not be opened yet.
        - @b UCDC_ER_NOT_IN_IDLE: If UCDC_CMD.UCDC_PROP.bEnterOnlyIdle set
             to TRUE and service is not in idle. Notice you this command is
             skipped.
        - @b UCDC_ER_OUT_DATA_VOLATILE: If the command include the output data
             and set UCDC_CMD.UCDC_PROP.bExitCmdFinish to FALSE, the finish
             callback have to plug-in a callback function to UCDC_CMD
             .UCDC_PROP.fpFinishCb and UCDC_CMD.Out have to point to a
             global data structure.
        - @b UCDC_ER_CMD_MAXDATA: internal error. Please contact to module
             maintenance staff.
*/
UCDC_ER xUCDC_Cmd(const UCDC_CMD *pCmd);

/**
    Suspend task service, but exit this function without finish suspend.

    This function is for advance. If you don't want to wait UCDC_Suspend()
    finish too long and you want to pipe some flow, you can do the flow that
    UCDC_AsyncSuspend() -> Do Something -> UCDC_Suspend();

    @note: UCDC_Suspend() is also called necessary to confirm the suspend is
           finished.

    @return
        - @b UCDC_ER_OK: suspend success.
        - @b UCDC_ER_STATE: may UCDC_Open does not be opened yet.
*/
UCDC_ER xUCDC_AsyncSuspend(VOID);

/**
    Stop task service, but exit this function without finish close.

    This function is for advance. If you don't want to wait UCDC_Close()
    finish too long and you want to pipe some flow, you can do the flow that
    UCDC_AsyncClose() -> Do Something -> UCDC_Close();

    @note: UCDC_AsyncClose() is also called necessary to confirm the suspend is
           finished.

    @return
        - @b UCDC_ER_OK: suspend success.
        - @b UCDC_ER_STATE: may UCDC_Open does not be opened yet.
*/
UCDC_ER xUCDC_AsyncClose(VOID);

/**
    Dump UCDC application library information.

    The information includes version, build date, last error code, last warning
    code, last command, last state and check internal data.

    @return
        - @b UCDC_ER_OK: suspend success.
*/
UCDC_ER xUCDC_DumpInfo(VOID);

void xUCDC_OpenNeededFIFO(void);
void xUCDC_EventCallback(UINT32 uiEvent);
void xUCDC_ClassRequestHandler(void);

void xUCDC_SS_OpenNeededFIFO(uintptr_t event);
void xUCDC_SS_EventCallback(uintptr_t event);
void xUCDC_SS_ClassRequestHandler(uintptr_t uiEvent);
void xUCDC_SS_ClassRequestDone(uintptr_t uiEvent);

#endif
