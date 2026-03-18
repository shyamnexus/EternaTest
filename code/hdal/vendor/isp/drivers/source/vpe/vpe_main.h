#ifndef _VPE_MAIN_H_
#define _VPE_MAIN_H_

#ifdef __KERNEL__
#include <linux/miscdevice.h>
#include <kwrap/dev.h>
#endif

#include "vpe_dev_int.h"

//=============================================================================
// VPE device name
//=============================================================================
#define VPE_DEV_NAME "nvt_vpe"
#define VPE_MODULE_MINOR_COUNT 1

//=============================================================================
// struct & definition
//=============================================================================
#ifdef __KERNEL__
typedef struct _VPE_DRV_INFO {
	VPE_DEV_INFO dev_info;
} VPE_DRV_INFO;

typedef struct vpe_vos_drv {
	struct class *pmodule_class;
	struct device *pdevice[VPE_MODULE_MINOR_COUNT];
	struct cdev cdev;
	dev_t dev_id;
} VPE_VOS_DRV, *PVPE_VOS_DRV;
#endif

//=============================================================================
// extern functions
//=============================================================================
extern VPE_DEV_INFO *vpe_get_dev_info(void);

#endif

