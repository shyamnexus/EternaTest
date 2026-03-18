/**
    Public header file for MDBC module.

    @file       mdbc_eng.h
    @ingroup    mIIPPMDBC

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/


#ifndef _MD_ENG_H_
#define _MD_ENG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "kdrv_type.h"
#include "md_eng_base.h"
#include "md_eng_handle.h"
#include "md_eng_dma_base.h"

#define MD_ENG_REG_NUMS  136

#define MD_SSD_DRV_NAME "Ssdrv_md"
#define MD_SSD_DRV_MODULE_VERSION "1.00.Beta.01"

/*****************************************************************************/
typedef struct {
	UINT32 ofs;
	UINT32 val;
} MD_ENG_REG;

typedef enum {
	MD_ENG_INTERRUPT_FRM_END            = 0x00000001,
    //MD_ENG_INTERRUPT_LL_END             = 0x00000100,
    //MDBC_ENG_INTERRUPT_CHKSUM_MISMATCH0         = 0x00020000,
    //MDBC_ENG_INTERRUPT_CHKSUM_MISMATCH1         = 0x00040000,
    //MDBC_ENG_INTERRUPT_CHKSUM_MISMATCH2         = 0x00080000,
    //MDBC_ENG_INTERRUPT_CCL_LABEL_OVFL           = 0x00100000,
    //MDBC_ENG_INTERRUPT_CCL_PIXEL_NUM_OVFL       = 0x00200000,
	
	MD_ENG_INTERRUPT_ALL                = (MD_ENG_INTERRUPT_FRM_END),
} MDBC_ENG_INTERRUPT;

typedef void (*MD_ISR_CB)(void *eng, UINT32 status, void *reserve);

extern INT32 md_eng_init(UINT32 chip_num, UINT32 eng_num);
extern INT32 md_eng_release(void);

extern MD_ENG_HANDLE* md_eng_get_handle(UINT32 chip_id, UINT32 eng_id);
extern void md_eng_reg_isr_callback(MD_ENG_HANDLE *p_eng, MD_ISR_CB cb);
extern INT32 md_eng_open(MD_ENG_HANDLE *p_eng);
extern INT32 md_eng_close(MD_ENG_HANDLE *p_eng);
extern void md_eng_trig_single_hw_reg(MD_ENG_HANDLE *p_eng);
extern INT32 md_eng_init_resource(MD_ENG_HANDLE *p_eng);

extern UINT32 md_eng_get_reg_base_buf_size(UINT32 eng_id);
extern UINT32 md_eng_get_reg_flag_buf_size(UINT32 eng_id);

extern VOID md_eng_set_reg_buf(MD_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr);
extern VOID md_eng_dma_channel_enable_hw_reg(MD_ENG_HANDLE *p_eng, BOOL set_en);
extern BOOL md_ssdrv_proc_ver(void);
extern INT32 md_eng_set_intrpt_en(MD_ENG_HANDLE *p_eng,  UINT32 int_en);
extern void md_eng_isr_hw_reg(MD_ENG_HANDLE *p_eng);
extern VOID md_eng_write_reg(MD_ENG_HANDLE *p_eng, UINT32 ofs, UINT32 val);
extern VOID md_eng_clr_intr_status(MD_ENG_HANDLE *p_eng, UINT32 uiIntrStatus);
extern UINT32 md_eng_wait_flg(MD_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg);
#endif //_MD_ENG_H_
