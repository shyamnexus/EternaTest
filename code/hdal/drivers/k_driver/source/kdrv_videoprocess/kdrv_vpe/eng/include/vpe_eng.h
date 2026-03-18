/**
    Public header file for VPE module.

    @file       vpe_eng.h
    @ingroup    mIIPPVPE

    @brief

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/


#ifndef _VPE_ENG_H_
#define _VPE_ENG_H_

#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kwrap/semaphore.h"
#include "vpe_eng_int_dbg.h"
#include "vpe_eng_handle.h"
#include "vpe_eng_int_reg.h"
#include "vpe_eng_pal_base.h"
#include "vpe_eng_shp_base.h"
#include "vpe_eng_dma_base.h"
#include "vpe_eng_ctrl_base.h"
#include "vpe_eng_sca_base.h"
#include "vpe_eng_dce_base.h"
#include "vpe_eng_mask_base.h"

#define VPE_SSD_DRV_NAME "Ssdrv_vpe"
#define VPE_SSD_DRV_MODULE_VERSION "1.02.14"
/*****************************************************************************/


typedef struct {
	UINT32 ofs;
	UINT32 val;
} VPE_ENG_REG;

typedef enum {
	VPE_ENG_INTERRUPT_FRM_END            = 0x00000001,
    VPE_ENG_INTERRUPT_LL_ERR             = 0X00000002,
	VPE_ENG_INTERRUPT_LL_END             = 0x00000004,

    VPE_ENG_INTERRUPT_DMA0_ERR           = 0x00000008,
	VPE_ENG_INTERRUPT_DMA1_ERR           = 0x00000010,

	
    VPE_ENG_INTERRUPT_RES0_YCC_ENC_OVFL  = 0x00000800,
	VPE_ENG_INTERRUPT_RES1_YCC_ENC_OVFL  = 0x00001000,
	VPE_ENG_INTERRUPT_RES2_YCC_ENC_OVFL  = 0x00002000,
	VPE_ENG_INTERRUPT_RES3_YCC_ENC_OVFL  = 0x00004000,
	VPE_ENG_INTERRUPT_YCC_DEC_ERR        = 0x00008000,
	VPE_ENG_INTERRUPT_ALL                = (VPE_ENG_INTERRUPT_FRM_END | VPE_ENG_INTERRUPT_LL_ERR |VPE_ENG_INTERRUPT_LL_END | VPE_ENG_INTERRUPT_DMA0_ERR |
	                                        VPE_ENG_INTERRUPT_DMA1_ERR | VPE_ENG_INTERRUPT_RES0_YCC_ENC_OVFL | VPE_ENG_INTERRUPT_RES1_YCC_ENC_OVFL | 
	                                        VPE_ENG_INTERRUPT_RES2_YCC_ENC_OVFL | VPE_ENG_INTERRUPT_RES3_YCC_ENC_OVFL | VPE_ENG_INTERRUPT_YCC_DEC_ERR),
} VPE_ENG_INTERRUPT;

typedef enum {
    RET_ERROR = -1,
    RET_OK = 0,
} RET_S;

typedef enum {
    ALIGN_NA = -1,
    ALIGN_None = 0,
    ALIGN_1 = 1,
    ALIGN_2 = 2,
    ALIGN_4 = 4,
    ALIGN_8 = 8,
    ALIGN_16 = 16,
    ALIGN_32 = 32,
    ALIGN_MAX
} ALIGN_DEF;

typedef enum {
	ALIGM_TYPE_DES_W = 0,
	ALIGM_TYPE_DES_H,

	ALIGM_TYPE_OUT_W,
	ALIGM_TYPE_OUT_H,
	ALIGM_TYPE_OUT_X,
	ALIGM_TYPE_OUT_Y,

	ALIGM_TYPE_RLT_W,
	ALIGM_TYPE_RLT_H,
	ALIGM_TYPE_RLT_X,
	ALIGM_TYPE_RLT_Y,

	ALIGM_TYPE_PIP_W,
	ALIGM_TYPE_PIP_H,
	ALIGM_TYPE_PIP_X,
	ALIGM_TYPE_PIP_Y,
    ALIGM_TYPE_SRC_COL_W,
    ALIGM_TYPE_SRC_COL_H,
	ALIGM_TYPE_PROC_X,
	ALIGM_TYPE_PROC_Y,
	ALIGM_TYPE_PROC_W,
	ALIGM_TYPE_PROC_H,

	ALIGM_TYPE_SRC_W,
	ALIGM_TYPE_SRC_H,

	ALIGM_TYPE_SCA_W,
	ALIGM_TYPE_SCA_H,

	ALIGM_TYPE_SCA_CROP_X,
	ALIGM_TYPE_SCA_CROP_Y,
	ALIGM_TYPE_SCA_CROP_W,
	ALIGM_TYPE_SCA_CROP_H,

	ALIGM_TYPE_TC_CROP_X,
	ALIGM_TYPE_TC_CROP_Y,
	ALIGM_TYPE_TC_CROP_W,
	ALIGM_TYPE_TC_CROP_H,

	ALIGM_TYPE_MAX,
} ALIGM_TYPE_DEF;


extern UINT32 vpe_eng_get_reg_base_buf_size(UINT32 eng_id);
extern UINT32 vpe_eng_get_reg_flag_buf_size(UINT32 eng_id);
extern VOID vpe_eng_set_reg_buf(VPE_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr);
extern INT32 vpe_eng_init(UINT32 chip_num, UINT32 eng_num);
extern INT32 vpe_eng_init_resource(VPE_ENG_HANDLE *p_eng);
extern INT32 vpe_eng_release(void);
extern VPE_ENG_HANDLE* vpe_eng_get_handle(UINT32 chip_id, UINT32 eng_id);
extern VOID vpe_eng_isr_hw_reg(VPE_ENG_HANDLE *p_eng);
extern INT32 vpe_eng_reg_isr_callback(VPE_ENG_HANDLE *p_eng, VPE_ISR_CB cb);
extern INT32 vpe_eng_open(VPE_ENG_HANDLE *p_eng);
extern INT32 vpe_eng_close(VPE_ENG_HANDLE *p_eng);
extern INT32 vpe_eng_write_hw_reg(VPE_ENG_HANDLE *p_eng, UINT32 reg_ofs, UINT32 val);
extern INT32 vpe_eng_trig_ll_hw_reg(VPE_ENG_HANDLE *p_eng, uintptr_t ll_addr);
extern INT32 vpe_eng_trig_single_hw_reg(VPE_ENG_HANDLE *p_eng);
extern INT32 vpe_eng_gen_int_en(VPE_ENG_HANDLE *p_eng,  VPE_ENG_INTERRUPT int_en);
extern BOOL vpe_ssdrv_proc_ver(void);
extern UINT32 vpe_eng_clr_flg(VPE_ENG_HANDLE *p_eng, FLGPTN flg);
extern UINT32 vpe_eng_wait_flg(VPE_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg);
extern UINT32 vpe_eng_wait_flg_timeout(VPE_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timeout_tick);

extern VOID vpe_eng_enable_clk(VPE_ENG_HANDLE *p_eng);
extern VOID vpe_eng_disable_clk(VPE_ENG_HANDLE *p_eng);
extern VOID vpe_eng_enable_gating(VPE_ENG_HANDLE *p_eng);
extern VOID vpe_eng_disable_gating(VPE_ENG_HANDLE *p_eng);
extern VOID vpe_eng_set_axi_dma_wrapper_control_hw_reg(VPE_ENG_HANDLE *p_eng, UINT32 dma_wrapper_en, UINT32 random_seed);

#endif //_VPE_ENG_H_
