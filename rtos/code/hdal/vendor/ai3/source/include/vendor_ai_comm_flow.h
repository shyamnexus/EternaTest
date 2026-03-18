/**
	@brief Header file of definition of vendor user-space net flow sample.

	@file net_flow_user_sample.h

	@ingroup net_flow_user_sample

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _VENDOR_AI_COMM_FLOW_H_
#define _VENDOR_AI_COMM_FLOW_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "kflow_isp/kflow_ai_isp_net.h"
#include "kflow_ai_net/kflow_ai_net.h"
#define DEFAULT_DEVICE  	"/dev/" VENDOR_AIS_FLOW_DEV_NAME
#define DEFAULT_ISP_DEVICE  	"/dev/" VENDOR_AIS_ISP_DEV_NAME
#if defined(__LINUX_USER__)
#include <sys/ioctl.h>
#include <sys/time.h>
#include <pthread.h>
#endif
#if defined(__FREERTOS)
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h>
#endif

#if defined(__LINUX_USER__)
#define KFLOW_AI_OPEN  open
#define KFLOW_AI_IOCTL ioctl
#define KFLOW_AI_CLOSE close
#define KFLOW_AI_ISP_OPEN  open
#define KFLOW_AI_ISP_IOCTL ioctl
#define KFLOW_AI_ISP_CLOSE close
#endif
#if defined (__FREERTOS)
#define KFLOW_AI_OPEN(...) 1
#define KFLOW_AI_IOCTL vendor_ais_flow_miscdev_ioctl
#define KFLOW_AI_CLOSE(...) 1
#define KFLOW_AI_ISP_OPEN(...)  1
#if defined (__UITRON) || defined(__ECOS)  || defined (__FREERTOS)
#define KFLOW_AI_ISP_IOCTL vendor_ais_flow_miscdev_ioctl // TODO_RTOS : THIS IS WRONG !! aiisp doesn't have rtos version "vendor_ais_isp_miscdev_ioctl" now!! call wrong flow ioctl (vendor_ais_flow_miscdev_ioctl) first to pass build
#else
#define KFLOW_AI_ISP_IOCTL vendor_ais_isp_miscdev_ioctl
#endif
#define KFLOW_AI_ISP_CLOSE(...) 1
#endif


/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/

extern VOID vendor_ai_errno_location(VOID);

extern VOID* vendor_ai_malloc(size_t size);
extern VOID vendor_ai_free(VOID* ptr, size_t size);

extern HD_RESULT vendor_ais_init_fd(void);
extern HD_RESULT vendor_ais_init_all(void);
extern HD_RESULT vendor_ais_uninit_all(void);
extern HD_RESULT vendor_ais_uninit_fd(void);

extern HD_RESULT vendor_ais_cfgschd(UINT32 schd);
extern HD_RESULT vendor_ais_cfgchk(UINT32 chk_interval);

extern HD_RESULT vendor_ais_init_net(void);
extern HD_RESULT vendor_ais_uninit_net(void);

extern HD_RESULT vendor_ais_lock_net(UINT32 net_id);
extern HD_RESULT vendor_ais_unlock_net(UINT32 net_id);




#endif  /* _VENDOR_AI_COMM_FLOW_H_ */
