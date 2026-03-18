/**
 * @file kdrv_rpc.h
 * @brief type definition of KDRV API.
 * @author PSW
 * @date in the year 2018
 */

#ifndef __KDRV_RPC_H__
#define __KDRV_RPC_H__

#include "kdrv_type.h"


/**
   Error Code.

   Any error code occurred will display on uart or return.
*/
typedef enum _KDRV_RPC_ER {
	KDRV_RPC_ER_OK                      =   0, ///< no error
	KDRV_RPC_ER_TIMEOUT                 =  -1, ///< timeout
	KDRV_RPC_ER_PARAM                   =  -2, ///< input parameter error
	KDRV_RPC_ER_QUEUE_FULL              =  -3, ///< rpc trigger command queue full
	KDRV_RPC_ER_SYS                     =  -4, ///< rpc system error
	ENUM_DUMMY4WORD(KDRV_RPC_ER)
} KDRV_RPC_ER;

typedef enum _KDRV_RPC_CORE_ID {
	KDRV_RPC_CORE_CPU1 = 0x1,             ///< Core ARM CA53 1st core
	KDRV_RPC_CORE_CPU2 = 0x2,             ///< Core ARM CA53 2nd core
	KDRV_RPC_CORE_DSP1 = 0x3,             ///< Core DSP1
	KDRV_RPC_CORE_DSP2 = 0x4,             ///< Core DSP2
	KDRV_RPC_CORE_MAX,
	ENUM_DUMMY4WORD(KDRV_RPC_CORE_ID)
} KDRV_RPC_CORE_ID;

typedef struct _KDRV_RPC_TRIGGER_PARAM {
	KDRV_RPC_CORE_ID dst_core_id;         ///< Destination Core ID
	UINT32           cmd_buf[3];          ///< command buffer
} KDRV_RPC_TRIGGER_PARAM;

typedef struct _KDRV_RPC_EVENT_PARAM {
	VOID       *p_cb_func;                ///< Event callback function
} KDRV_RPC_EVENT_PARAM;

typedef enum _KDRV_RPC_PARAM_ID {
	RPC_PARAM_EVENT = 0,                  ///< [set] KDRV_RPC_EVENT_PARAM
	RPC_PARAM_DSP_PROG_INT_ADDR,          ///< [set] UINT32 value
	RPC_PARAM_DSP_DATA_INT_ADDR,          ///< [set] UINT32 value
	RPC_PARAM_DSP_SHM_ADDR,               ///< [set] UINT32 value
	RPC_PARAM_DSP_LOG_ADDR,               ///< [set] UINT32 value
	RPC_PARAM_MAX
} KDRV_RPC_PARAM_ID;


typedef struct _KDRV_RPC_TRIG_CB_INFO {
	KDRV_RPC_ER      rtn;                 ///< trigger command return value
} KDRV_RPC_TRIG_CB_INFO;

typedef struct _KDRV_RPC_EVENT_CB_INFO {
	KDRV_RPC_CORE_ID src_core_id;         ///< Source Core ID
	UINT32           cmd_buf[3];          ///< command buffer
} KDRV_RPC_EVENT_CB_INFO;


typedef void (*RPC_EVENT_CB)(KDRV_RPC_EVENT_CB_INFO *p_cb_info);

// for rtos init/exit driver
#if defined(__FREERTOS)
INT32  kdrv_rpc_init(void);
void  kdrv_rpc_exit(void);
#endif

/*!
 * @fn INT32 kdrv_rpc_open(UINT32 chip, UINT32 engine)
 * @brief open hardware engine
 * @param chip		the chip id of hardware
 * @param engine	the engine id of hardware
 * @return return 0 on success, < 0 on error
 */
INT32 kdrv_rpc_open(UINT32 chip, UINT32 engine);

/*!
 * @fn INT32 kdrv_rpc_close(UINT32 chip, UINT32 engine)
 * @brief close hardware engine
 * @param chip		the chip id of hardware
 * @param engine	the engine id of hardware
 * @return return 0 on success, < 0 on error
 */
INT32 kdrv_rpc_close(UINT32 chip, UINT32 engine);

/*!
 * @fn INT32 kdrv_rpc_trigger(INT32 id, KDRV_RPC_TRIGGER_PARAM *p_rpc_param,
							     KDRV_CALLBACK_FUNC *p_cb_func,
								 VOID *p_user_data);
 * @brief trigger hardware engine
 * @param id 				    the id of hardware
 * @param p_rpc_param		    the parameter for trigger
 * @param p_cb_func 			the callback function
 * @param p_user_data 			the private user data
 * @return return 0 on success, < 0 on error
 */
INT32 kdrv_rpc_trigger(UINT32 id, KDRV_RPC_TRIGGER_PARAM *p_rpc_param,
						  KDRV_CALLBACK_FUNC *p_cb_func,
						  VOID *p_user_data);

/*!
 * @fn INT32 kdrv_rpc_get(UINT32 id, KDRV_RPC_PARAM_ID parm_id, VOID *p_param)
 * @brief get parameters from hardware engine
 * @param id 	    the id of hardware
 * @param parm_id   the id of parameters
 * @param p_param 	the parameters
 * @return return 0 on success, < 0 on error
 */
INT32 kdrv_rpc_get(UINT32 id, KDRV_RPC_PARAM_ID parm_id, VOID *p_param);

/*!
 * @fn INT32 kdrv_rpc_set(UINT32 id, KDRV_RPC_PARAM_ID parm_id, VOID *p_param)
 * @brief set parameters to hardware engine
 * @param id 	    the id of hardware
 * @param parm_id   the id of parameters
 * @param p_param 	the parameters
 * @return return 0 on success, < 0 on error
 */
INT32 kdrv_rpc_set(UINT32 id, KDRV_RPC_PARAM_ID parm_id, VOID *p_param);


#endif
