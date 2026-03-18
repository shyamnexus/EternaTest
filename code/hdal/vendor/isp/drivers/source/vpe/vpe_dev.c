#if defined(__FREERTOS)
#include "vpe_dev_int.h"
#else
#include "vpe_main.h"
#endif
#include "vpe_api_int.h"

//=============================================================================
// global variable
//=============================================================================
#if defined(__KERNEL__)
extern UINT32 vpe_id_list;
extern UINT32 vpe_idx_list;
extern UINT32 vpe_idx_num;
extern UINT32 vpe_2dlut_size;
#else
static VPE_DEV_INFO pdev_info;
#endif

//=============================================================================
// function declaration
//=============================================================================

//=============================================================================
// external functions
//=============================================================================
#if defined(__FREERTOS)
VPE_DEV_INFO *vpe_get_dev_info(void)
{
	return &pdev_info;
}

#else
//=============================================================================
// internal functions
//=============================================================================
INT32 vpe_dev_construct(VPE_DEV_INFO *pdev_info)
{
	#if defined(__KERNEL__)
	// initialize synchronization mechanism
	sema_init(&pdev_info->api_mutex, 1);
	sema_init(&pdev_info->ioc_mutex, 1);
	sema_init(&pdev_info->proc_mutex, 1);
	#endif

	vpe_api_reg_if(vpe_id_list, vpe_idx_list, vpe_idx_num, vpe_2dlut_size);

	return 0;
}

void vpe_dev_deconstruct(VPE_DEV_INFO *pdev_info)
{
	vpe_api_unreg_if();
}
#endif

