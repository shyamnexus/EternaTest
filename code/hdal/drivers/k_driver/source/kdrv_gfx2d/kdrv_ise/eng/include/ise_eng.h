/**
    Public header file for ISE module.

    @file       ise_eng.h
    @ingroup    mIIPPISE

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/


#ifndef _ISE_ENG_H_
#define _ISE_ENG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ise_eng_base.h"

#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kdrv_type.h"
#include "ise_eng_limit.h"


#define ssdrv_change_cmd 0

//#define ISE_ENG_REG_NUMS  44    // for NT98520
#define ISE_ENG_REG_NUMS  68    // for NT98530

/*****************************************************************************/
typedef void (*ISE_ISR_CB)(void *eng, UINT32 status, void *reserve);

typedef struct {
	UINT32 ofs;
	UINT32 val;
} ISE_ENG_REG;


extern INT32 ise_eng_init(UINT32 chip_num, UINT32 eng_num);
extern INT32 ise_eng_release(void);

extern ISE_ENG_HANDLE* ise_eng_get_handle(UINT32 chip_id, UINT32 eng_id);
extern void ise_eng_reg_isr_callback(ISE_ENG_HANDLE *p_eng, ISE_ISR_CB cb);
extern INT32 ise_eng_open(ISE_ENG_HANDLE *p_eng);
extern INT32 ise_eng_close(ISE_ENG_HANDLE *p_eng);
extern void ise_eng_trig_single_hw_reg(ISE_ENG_HANDLE *p_eng);
extern void ise_eng_trig_ll_hw_reg(ISE_ENG_HANDLE *p_eng, ULONG ll_addr);
extern void ise_eng_stop_single_hw_reg(ISE_ENG_HANDLE *p_eng);
extern void ise_eng_clear_flag_frame_end(UINT32 eng_id);

extern void ise_eng_write_hw_reg(ISE_ENG_HANDLE *p_eng, UINT32 reg_ofs, UINT32 val);
extern void ise_eng_hard_reset_hw_reg(ISE_ENG_HANDLE *p_eng);

extern VOID ise_eng_wait_flag_frame_end(BOOL is_clear_flag, UINT32 eng_id);
extern VOID ise_eng_wait_flag_linked_list_end(BOOL is_clear_flag, UINT32 eng_id);
extern INT32 ise_eng_chk_limitation(ULONG reg_base_addr, ULONG reg_flag_addr);


extern void ise_eng_isr_hw_reg(ISE_ENG_HANDLE *p_eng);
extern INT32 ise_eng_init_resource(ISE_ENG_HANDLE *p_eng);

extern void ise_eng_dump(void);

extern UINT32 ise_eng_get_reg_base_buf_size(UINT32 eng_id);
extern UINT32 ise_eng_get_reg_flag_buf_size(UINT32 eng_id);

extern VOID ise_eng_set_reg_buf(ISE_ENG_HANDLE *p_eng, ULONG reg_base_addr, ULONG reg_flag_addr);

////////////////////////////////////////////////////////////////////////////////
extern ISE_ENG_HANDLE* ise_eng_get_handle(UINT32 chip_id, UINT32 eng_id);
////////////////////////////////////////////////////////////////////////////////
extern VOID ise_eng_dma_channel_enable_hw_reg(ISE_ENG_HANDLE *p_eng, BOOL set_en);
extern UINT8 ise_eng_get_dma_channel_status_hw_reg(ISE_ENG_HANDLE *p_eng);
extern BOOL ise_ssdrv_proc_ver(void);
#endif //_ISE_ENG_H_
