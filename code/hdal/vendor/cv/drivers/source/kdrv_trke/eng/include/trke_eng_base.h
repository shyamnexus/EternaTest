#ifndef _TRKE_ENG_BASE_H_
#define _TRKE_ENG_BASE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/nvt_type.h"
#include "trke_eng_handle.h"

typedef enum _TRKE_ENG_ID {
	TRKE_ID  = 0,
	TRKE_ID_MAX_NUM,
	ENUM_DUMMY4WORD(TRKE_ENG_ID)
} TRKE_ENG_ID;


typedef struct {
	UINT32 point_num;	
	UINT32 max_search_range;
	UINT32 iter_cnt;
	BOOL is_initial_flow;
	BOOL is_skip_point;
	UINT32 eps;
	UINT32 min_eug_val_thres;	
	UINT32 pyr_index;
	UINT32 max_pyr_level;
	UINT32 max_search_range_disable;
	UINT32 pyr_search_range;
	UINT32 layer_search_range ;
	UINT32 Patchsize ;
} TRKE_ENG_CONTROL_INFO;

typedef struct  {
	BOOL STATUS[500];        ///< STATUS
} TRKE_ENG_STATUS;


typedef struct {
    uintptr_t addr;
    UINT32 lnfst;
} TRKE_ENG_DRAM_INFO;


typedef struct {
	
	UINT16 img_width;
    UINT16 img_height;
	TRKE_ENG_DRAM_INFO in0;
    TRKE_ENG_DRAM_INFO in1;
	TRKE_ENG_DRAM_INFO in2;
	TRKE_ENG_DRAM_INFO in3;
    TRKE_ENG_DRAM_INFO out0;
    TRKE_ENG_DRAM_INFO out1;
	TRKE_ENG_CONTROL_INFO FuncInfo;             
	TRKE_ENG_STATUS PtStatus;    
	
} TRKE_ENG_STRUCT;



//-----------------------------------  DMA PARAM ---------------------------------//

extern VOID trke_eng_set_img_size(TRKE_ENG_HANDLE *p_eng, UINT32 img_w, UINT32 img_h);
extern VOID trke_eng_set_control_param(TRKE_ENG_HANDLE *p_eng,TRKE_ENG_CONTROL_INFO FuncInfo );
extern UINT32 trke_eng_get_status(TRKE_ENG_HANDLE *p_eng, int idx);
extern VOID trke_eng_set_status(TRKE_ENG_HANDLE *p_eng, int idx, UINT32 PtStatus);


#endif //_TRKE_ENG_BASE_H_






