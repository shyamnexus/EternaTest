#ifndef _IQ_FLOW_H_
#define _IQ_FLOW_H_

#include "isp_dev.h"
#include "kwrap/type.h"

//=============================================================================
// struct & definition
//=============================================================================
#define IQ_TBL_SIE_DPC     0x00010000
#define IQ_TBL_SIE_ECS     0x00020000
#define IQ_TBL_DCE_GDC     0x00040000
#define IQ_TBL_IPE_GAMMA   0x00100000
#define IQ_TBL_IPE_YCURVE  0x00200000
#define IQ_TBL_MASK        0xffff0000

//=============================================================================
// extern functions
//=============================================================================
extern UINT32 iq_info_buffer_size;
extern UINT32 iq_info_buffer_dpc_ofs;
extern UINT32 iq_info_buffer_ecs_ofs;
extern UINT32 iq_info_buffer_3dcc_ofs;
extern IQ_PARAM_PTR *iq_param[IQ_ID_MAX_NUM];
extern BOOL test_mode_en[IQ_ID_MAX_NUM];
extern IQ_OB_MODE test_mode_ob[IQ_ID_MAX_NUM];
extern IQ_DG_MODE test_mode_dg[IQ_ID_MAX_NUM];
extern IQ_CG_MODE test_mode_cg[IQ_ID_MAX_NUM];
extern BOOL companding_en[IQ_ID_MAX_NUM];
extern BOOL cfa_en[IQ_ID_MAX_NUM];
extern BOOL test_shdr_en[IQ_ID_MAX_NUM];
extern UINT32 test_ev_ratio[IQ_ID_MAX_NUM][ISP_SEN_MFRAME_MAX_NUM];
extern UINT32 test_tm_ratio[IQ_ID_MAX_NUM];
extern BOOL test_shdr_hbs_en[IQ_ID_MAX_NUM];
extern ISP_AE_HBS_PARAM test_hbs[IQ_ID_MAX_NUM];
extern UINT32 nr_r_ratio[IQ_ID_MAX_NUM];
extern UINT32 nr_b_ratio[IQ_ID_MAX_NUM];
extern UINT32 nr_ir_ratio[IQ_ID_MAX_NUM];
extern UINT32 residue_reset_num[IQ_ID_MAX_NUM];
extern BOOL tmnr_ae_still_en[IQ_ID_MAX_NUM];

extern BOOL iq_flow_get_id_valid(UINT32 id);
extern UINT32 iq_flow_get_info_map(UINT32 id);
extern ISP_MODULE *iq_get_module(void);
extern INT32 iq_flow_init(UINT32 id);
extern INT32 iq_flow_uninit(UINT32 id);
extern INT32 iq_flow_process(UINT32 id, ISP_TRIG_MSG msg, void *arg);

#endif
