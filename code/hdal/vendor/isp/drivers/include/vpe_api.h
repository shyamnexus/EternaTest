#ifndef _VPE_API_H_
#define _VPE_API_H_

#if defined(__KERNEL__) || defined(__FREERTOS)
extern void vpe_api_reg_if(UINT32 id_list, UINT32 idx_list, UINT32 idx_num, UINT32 _2dlut_size);
extern void vpe_api_unreg_if(void);
#endif

//=============================================================================
// struct & enum definition
//=============================================================================


//=============================================================================
// extern functions
//=============================================================================
#if defined(__KERNEL__) || defined(__FREERTOS)

#endif

#endif

