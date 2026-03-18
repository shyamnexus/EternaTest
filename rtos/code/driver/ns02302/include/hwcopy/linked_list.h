#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

//#include "Type.h"
#include "kwrap/type.h"


extern UINT64 linked_list_null(UINT32 tab_idx);

extern UINT64 linked_list_update(UINT32 byte_en, UINT32 reg_ofs, UINT32 reg_val);

extern UINT64 linked_list_next_job(UINT32 next_job_addr, UINT32 tab_idx);

extern UINT64 linked_list_next_update(UINT32 next_update_addr);

extern UINT64 linked_list_offset_update(UINT32 byte_en, UINT32 reg_ofs, UINT32 reg_val);

extern UINT64 linked_list_special_update(UINT32 byte_en, UINT32 reg_ofs, UINT32 reg_val);


#ifdef __cplusplus
} //extern "C" {
#endif

#endif
