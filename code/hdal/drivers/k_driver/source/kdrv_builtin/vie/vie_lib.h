/*
    Public header file for RDE module.

    This file is the header file that define the API and data type for VIE
    module.

    @file       vie_lib.h
    @ingroup    mIDrvIPPVIE
    @note

    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
*/

#ifndef _VIE_LIB_H
#define _VIE_LIB_H

#include "viep_lib.h"

#define VIE_CHANNEL_NUM 4

extern VIE_PADA_SRC _vie_padA_src;


/**
    VIE Entire Configuration.

    Structure of entire SIE configuration.

    This is for emulation only.
*/
//@{
typedef struct {
	UINT32					uiFuncEn;
	UINT32					uiIntrpEn;
	UINT32                  uiDbgEn;
	VIE_MAIN_INPUT_INFO     MainInInfo;
	VIE_PATGEN_INFO         PatGenInfo;
   
	VIE_BREAKPOINT_INFO     BreakPointInfo[VIE_CHANNEL_NUM];
	VIE_SRC_WIN_INFO        SrcWinInfo[VIE_CHANNEL_NUM];
	VIE_CRP_WIN_INFO        CrpWinInfo[VIE_CHANNEL_NUM];

	VIE_DRAM_OUT_INFO       DramOut0Info[VIE_CHANNEL_NUM];
	VIE_DRAM_OUT_INFO       DramOut1Info[VIE_CHANNEL_NUM];
	VIE_DRAM_OUT_CTRL       DramOutMode[VIE_CHANNEL_NUM];
	VIE_DRAM_SINGLE_OUT     DramSingleOutEn[VIE_CHANNEL_NUM];
	VIE_DVI_INFO            DviInfo[VIE_CHANNEL_NUM];
	VIE_MASK_INFO           MaskInfo[VIE_CHANNEL_NUM];
	UINT16                  bt656header[VIE_CHANNEL_NUM];

	VIE_YCC_PARAM_INFO      YccParamInfo;
	
} VIE_MODE_PARAM;
//@}

typedef void (*VIE_DRV_ISR_FP)(UINT32 int_sts, VIE_ENGINE_STATUS_INFO_CB *info);

/**
SIE API

@name   SIE_API
*/
//@{
extern ER vie_open(VIE_ENGINE_ID eng_id, VIE_OPENOBJ *pObjCB);
extern BOOL vie_isOpened(VIE_ENGINE_ID eng_id);
extern ER vie_close(VIE_ENGINE_ID eng_id);
extern ER vie_pause(VIE_ENGINE_ID eng_id, UINT32 ch);
extern ER vie_start(VIE_ENGINE_ID eng_id, UINT32 ch);
extern ER vie_setMode(VIE_ENGINE_ID eng_id, VIE_MODE_PARAM *pSieParam);
extern ER vie_chgParam(VIE_ENGINE_ID eng_id, VIE_CHANNEL_ID ch_id, void *pParam, VIE_CHANGE_FUN_PARAM_SEL FunSel);
extern ER vie_chgFuncEn(VIE_ENGINE_ID eng_id, VIE_CHANNEL_ID ch_id, VIE_FUNC_SWITCH_SEL FuncSwitch, UINT32 uiFuncSel);
extern ER vie_waitEvent(VIE_ENGINE_ID eng_id, VIE_WAIT_EVENT_SEL WaitEvent, BOOL bClrFlag);

extern void vie_set_dmaaobrt(VIE_ENGINE_ID eng_id);
extern void vie_get_dram_single_out(VIE_ENGINE_ID eng_id, VIE_CHANNEL_ID ch_id, VIE_DRAM_SINGLE_OUT *pDOut);
extern BOOL vie_get_load(VIE_ENGINE_ID eng_id, VIE_CHANNEL_ID ch_id);
extern void vie_get_sys_debug_info(VIE_ENGINE_ID eng_id, VIE_CHANNEL_ID ch_id, VIE_SYS_DEBUG_INFO *pDbugInfo);
extern void vie_set_checksum_en(VIE_ENGINE_ID eng_id, UINT32 uiEn);
extern void vie_reg_isr_cb(VIE_ENGINE_ID eng_id, VIE_DRV_ISR_FP fp);
extern void vie_reg_vdlatch_isr_cb(VIE_ENGINE_ID eng_id, VIE_DRV_ISR_FP fp);
extern UINT32 vie_get_ccir656_matchStatus(VIE_ENGINE_ID eng_id, VIE_CHANNEL_ID ch_id);

extern ER vie_chgBurstLengthByMode(VIE_ENGINE_ID eng_id, VIE_DRAM_BURST_MODE_SEL BurstMdSel);
extern ER vie_attach(VIE_ENGINE_ID eng_id, BOOL do_enable);
extern ER vie_detach(VIE_ENGINE_ID eng_id, BOOL do_disable);

extern INT32 vie_chk_limitation(VIE_ENGINE_ID eng_id, UINT32 flag);

#endif//_VIE_LIB_H

//@}


