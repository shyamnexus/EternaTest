#ifndef _VPE_DEV_INT_H_
#define _VPE_DEV_INT_H_

#if defined(__FREERTOS)
#else
#include <linux/semaphore.h>
#endif

#include "vpe_alg_int.h"

//=============================================================================
// struct & definition
//=============================================================================
#define VPE_MAX_NAME_SIZE       16

typedef struct _VPE_DEV_INFO {
	#if defined(__FREERTOS)
	#else
	// ioctl usage
	ULONG *ioctl_buf;
	// synchronization mechanism
	struct semaphore api_mutex;
	struct semaphore ioc_mutex;
	struct semaphore proc_mutex;
	#endif
} VPE_DEV_INFO;

typedef struct _VPE_MODULE {
	CHAR     name[VPE_MAX_NAME_SIZE];        // module name
	void     *private;                       // private date
} VPE_MODULE;

//=============================================================================
// extern functions
//=============================================================================
#if defined(__FREERTOS)
extern VPE_DEV_INFO *vpe_get_dev_info(void);
#endif
extern INT32 vpe_dev_construct(VPE_DEV_INFO *pdev_info);
extern void vpe_dev_deconstruct(VPE_DEV_INFO *pdev_info);
extern VPE_PARAM_PTR *vpe_param[VPE_ID_MAX_NUM][VPE_IDX_MAX_NUM];
extern VPE_PARAM_PTR *vpelite_param[VPE_ID_MAX_NUM];

#endif

