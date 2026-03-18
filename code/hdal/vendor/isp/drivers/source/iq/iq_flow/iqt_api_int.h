#ifndef _IQT_API_INT_H_
#define _IQT_API_INT_H_

//=============================================================================
// extern functions
//=============================================================================
extern UINT32 iqt_api_get_item_size(IQT_ITEM item);
extern ER iqt_api_get_cmd(IQT_ITEM item, ULONG addr);
extern ER iqt_api_set_cmd(IQT_ITEM item, ULONG addr);
extern UINT32 iqt_get_param_update(UINT32 id, IQT_ITEM item);
extern void iqt_set_param_update(UINT32 id, IQT_ITEM item);
extern void iqt_reset_param_update(UINT32 id, IQT_ITEM item);

extern ER iqt_api_nnsc_get_cmd(IQT_ITEM item, ULONG addr);
extern ER iqt_api_nnsc_set_cmd(IQT_ITEM item, ULONG addr);

#endif

