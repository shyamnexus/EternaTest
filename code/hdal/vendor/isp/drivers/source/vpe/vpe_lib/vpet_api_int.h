#ifndef _VPET_API_INT_H_
#define _VPET_API_INT_H_

#include "vpet_api.h"

//=============================================================================
// extern functions
//=============================================================================
extern UINT32 vpet_api_get_item_size(VPET_ITEM item);
extern BOOL vpet_get_param_update(UINT32 id, UINT32 idx, VPET_ITEM item);
extern ER vpet_api_get_cmd(VPET_ITEM item, ULONG addr);
extern void vpet_set_param_update(UINT32 id, UINT32 idx, VPET_ITEM item, BOOL en);
extern ER vpet_api_set_cmd(VPET_ITEM item, ULONG addr);

#endif

