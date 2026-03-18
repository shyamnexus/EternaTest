/**
    Public header file for PRE module.

    @file       pre_eng.h
    @ingroup    mIIPPPRE

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/


#ifndef _PRE_ENG_H_
#define _PRE_ENG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kdrv_type.h"

#include "pre_eng_handle.h"
#include "pre_eng_base.h"
#include "pre_eng_limit.h"


#define PRE_ENG_REG_NUM     952

#define PRE_LOADTYPE_PRE_START_LOAD     0
#define PRE_LOADTYPE_FRAME_END_LOAD     1
#define PRE_LOADTYPE_FRAME_START_LOAD   2
#define PRE_LOADTYPE_GLOBAL_LOAD    3


/*****************************************************************************/

typedef enum  {
	PRE_ENG_INTERRUPT_FMD             = 0x00000001,  ///< enable interrupt: frame end
	PRE_ENG_INTERRUPT_DEC1_ERR        = 0x00000002,  ///< enable interrupt: rde decode error interrupt
	PRE_ENG_INTERRUPT_DEC2_ERR        = 0x00000004,  ///< enable interrupt: rde decode error interrupt
	PRE_ENG_INTERRUPT_LLEND           = 0x00000008,  ///< enable interrupt: LinkedList end interrupt
	PRE_ENG_INTERRUPT_LLERR           = 0x00000010,  ///< enable interrupt: LinkedList error  interrupt
	PRE_ENG_INTERRUPT_LLERR2          = 0x00000020,  ///< enable interrupt: LinkedList error2 interrupt
	PRE_ENG_INTERRUPT_LLJOBEND        = 0x00000040,  ///< enable interrupt: LinkedList job end interrupt
	PRE_ENG_INTERRUPT_BUFOVFL         = 0x00000080,  ///< enable interrupt: buffer overflow interrupt
	PRE_ENG_INTERRUPT_RING_BUF_ERR    = 0x00000100,  ///< enable interrupt: ring buffer error interrupt
	PRE_ENG_INTERRUPT_FRAME_ERR       = 0x00000200,  ///< enable interrupt: frame error interrupt (for direct mode)
	//PRE_ENG_INTERRUPT_DEC3_ERR        = 0x00000400,  ///< enable interrupt: rde decode error interrupt
	//PRE_ENG_INTERRUPT_RING_BUF_ERR2   = 0x00000800,  ///< enable interrupt: ring buffer error 2 interrupt
	PRE_ENG_INTERRUPT_SIE_FRAME_START = 0x00001000,  ///< enable interrupt: SIE frame start interrupt (for direct mode)
	PRE_ENG_INTERRUPT_FRAME_START     = 0x00004000,  ///< enable interrupt: SIE frame start interrupt (for direct mode)

	PRE_ENG_INTERRUPT_ALL             = (PRE_ENG_INTERRUPT_FMD | PRE_ENG_INTERRUPT_DEC1_ERR | PRE_ENG_INTERRUPT_DEC2_ERR | PRE_ENG_INTERRUPT_LLEND | PRE_ENG_INTERRUPT_LLERR |
										 PRE_ENG_INTERRUPT_LLERR2 | PRE_ENG_INTERRUPT_LLJOBEND | PRE_ENG_INTERRUPT_BUFOVFL | PRE_ENG_INTERRUPT_RING_BUF_ERR | PRE_ENG_INTERRUPT_FRAME_ERR |
										 PRE_ENG_INTERRUPT_SIE_FRAME_START | PRE_ENG_INTERRUPT_FRAME_START)
} PRE_ENG_INTERRUPT;

typedef struct {
	PRE_OPMODE mode;
	PRE_BITDEPTH in_bits;
	PRE_BITDEPTH out_bits;
	PRE_BAYERFMTSEL bayer_fmt;
	UINT32 cfa_pat;
	UINT32 binning;
	UINT32 fusion_num;
} PRE_ENG_CONTROL_INFO;



typedef struct {
	UINT32 ofs;
	UINT32 val;
} PRE_ENG_REG;
/*****************************************************************************/

extern INT32 pre_eng_init(UINT32 chip_num, UINT32 eng_num);
extern INT32 pre_eng_release(void);
extern INT32 pre_eng_init_resource(PRE_ENG_HANDLE *p_eng);
extern PRE_ENG_HANDLE *pre_eng_get_handle(UINT32 chip_id, UINT32 eng_id);
extern void pre_eng_reg_isr_callback(PRE_ENG_HANDLE *p_eng, PRE_ISR_CB cb);
extern INT32 pre_eng_open(PRE_ENG_HANDLE *p_eng);
extern INT32 pre_eng_close(PRE_ENG_HANDLE *p_eng);
extern void pre_eng_trig_single_hw_reg(PRE_ENG_HANDLE *p_eng);
extern void pre_eng_trig_ll_hw_reg(PRE_ENG_HANDLE *p_eng, ULONG ll_addr, UINT32 ll_addr_msb);
extern void pre_eng_write_hw_reg(PRE_ENG_HANDLE *p_eng, uintptr_t reg_ofs, UINT32 val);
extern void pre_eng_hard_reset_hw_reg(PRE_ENG_HANDLE *p_eng);
extern void pre_eng_isr_hw_reg(PRE_ENG_HANDLE *p_eng);
extern void pre_eng_int_soft_reset_hw_reg(PRE_ENG_HANDLE *p_eng);
extern void pre_eng_set_hdr_shutdown_enable_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL enable);
extern void pre_eng_set_va_shutdown_enable_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL enable);
extern void pre_eng_set_bnr_shutdown_enable_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL enable);
extern void pre_eng_set_outlier_shutdown_enable_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL enable);
extern void pre_eng_set_enable_int_hw_reg(PRE_ENG_HANDLE *p_eng, UINT32 intr);
extern void pre_eng_ll_fire_hw_reg(PRE_ENG_HANDLE *p_eng);

extern void pre_eng_set_load_hw_reg(PRE_ENG_HANDLE *p_eng, UINT32 load_type);
extern void pre_eng_stop_single_hw_reg(PRE_ENG_HANDLE *p_eng);

extern UINT32 pre_eng_get_reg_base_buf_size(VOID);  // unit: byte
extern UINT32 pre_eng_get_reg_flag_buf_size(VOID);   // unit: byte
extern VOID pre_eng_set_reg_buf(PRE_ENG_HANDLE *p_eng, ULONG reg_base_addr, ULONG reg_flag_addr);


extern VOID pre_eng_wait_flag_frame_end(PRE_ENG_HANDLE *p_eng, BOOL is_clear_flag);
extern VOID pre_eng_wait_flag_linked_list_end(PRE_ENG_HANDLE *p_eng, BOOL is_clear_flag);
extern VOID pre_eng_wait_flag_linked_list_job_end(PRE_ENG_HANDLE *p_eng, BOOL is_clear_flag);
extern VOID pre_eng_wait_flag_sie_frame_start(PRE_ENG_HANDLE *p_eng, BOOL is_clear_flag);
extern VOID pre_eng_clear_flag_frame_end(PRE_ENG_HANDLE *p_eng);
extern VOID pre_eng_clear_flag_linked_list_end(PRE_ENG_HANDLE *p_eng);
extern VOID pre_eng_clear_flag_linked_list_job_end(PRE_ENG_HANDLE *p_eng);

extern INT32 pre_eng_chk_limitation(ULONG reg_base_addr, ULONG reg_flag_addr);

extern VOID pre_eng_dma_channel_enable_hw_reg(PRE_ENG_HANDLE *p_eng, BOOL set_en);
extern UINT8 pre_eng_get_dma_channel_status_hw_reg(PRE_ENG_HANDLE *p_eng);

extern VOID pre_eng_set_dbg_level(UINT32 level);

extern VOID pre_eng_set_axi_disable_hw_reg(PRE_ENG_HANDLE *p_eng, BOOL enable);

extern VOID pre_eng_get_va_result_buf_reg(PRE_VA_SETTING *p_va_info, PRE_VA_WIN_PARAM *p_va_win, PRE_VA_RSLT *p_va_rslt);
extern VOID pre_eng_get_indep_va_win_rslt_hw_reg(PRE_ENG_HANDLE *p_eng, PRE_INDEP_VA_WIN_RSLT *p_indepva_rslt, UINT32 win_idx);

extern VOID pre_eng_get_va_normalization_result_buf_reg(PRE_VA_SETTING *p_va_info, PRE_VA_WIN_PARAM *p_va_win, PRE_VA_RSLT *p_va_rslt, UINT32 im_width, UINT32 im_height);
extern VOID pre_eng_get_indep_va_win_normalization_rslt_hw_reg(PRE_ENG_HANDLE *p_eng, PRE_INDEP_VA_WIN_RSLT *p_indepva_rslt, UINT32 win_idx, UINT32 im_width, UINT32 im_height);


#if (defined(_NVT_EMULATION_) == ON)

extern ULONG pre_get_dram_in_addr(PRE_ENG_HANDLE *p_eng);

extern ULONG pre_get_dram_in_addr2(PRE_ENG_HANDLE *p_eng);

extern UINT32 pre_get_dram_in_lofs(PRE_ENG_HANDLE *p_eng);

extern UINT32 pre_get_dram_in_lofs2(PRE_ENG_HANDLE *p_eng);

extern ULONG pre_get_dram_out_addr(PRE_ENG_HANDLE *p_eng);

extern UINT32 pre_get_dram_out_lofs(PRE_ENG_HANDLE *p_eng);

extern UINT32 pre_get_in_vsize(PRE_ENG_HANDLE *p_eng);

extern UINT32 pre_get_in_hsize(PRE_ENG_HANDLE *p_eng);

extern BOOL pre_end_time_out_status;

extern UINT32 pre_get_dram_va_out_lofs(PRE_ENG_HANDLE *p_eng);

extern ULONG pre_get_dram_va_out_addr(PRE_ENG_HANDLE *p_eng);

#endif




#endif
