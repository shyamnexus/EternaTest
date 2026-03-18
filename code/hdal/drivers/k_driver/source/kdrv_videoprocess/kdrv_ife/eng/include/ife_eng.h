/**
    Public header file for IFE module.

    @file       ife_eng.h
    @ingroup    mIIPPIFE

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/


#ifndef _IFE_ENG_H_
#define _IFE_ENG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kdrv_type.h"

#include "ife_eng_handle.h"
#include "ife_eng_base.h"
#include "ife_eng_limit.h"


#define IFE_ENG_REG_NUM     867

#define IFE_LOADTYPE_IFE_START_LOAD     0
#define IFE_LOADTYPE_FRAME_END_LOAD     1
#define IFE_LOADTYPE_FRAME_START_LOAD   2
#define IFE_LOADTYPE_GLOBAL_LOAD    3


/*****************************************************************************/

typedef enum  {
	IFE_ENG_INTERRUPT_FMD             = 0x00000001,  ///< enable interrupt: frame end
	IFE_ENG_INTERRUPT_DEC1_ERR        = 0x00000002,  ///< enable interrupt: rde decode error interrupt
	IFE_ENG_INTERRUPT_DEC2_ERR        = 0x00000004,  ///< enable interrupt: rde decode error interrupt
	IFE_ENG_INTERRUPT_LLEND           = 0x00000008,  ///< enable interrupt: LinkedList end interrupt
	IFE_ENG_INTERRUPT_LLERR           = 0x00000010,  ///< enable interrupt: LinkedList error  interrupt
	IFE_ENG_INTERRUPT_LLERR2          = 0x00000020,  ///< enable interrupt: LinkedList error2 interrupt
	IFE_ENG_INTERRUPT_LLJOBEND        = 0x00000040,  ///< enable interrupt: LinkedList job end interrupt
	IFE_ENG_INTERRUPT_BUFOVFL         = 0x00000080,  ///< enable interrupt: buffer overflow interrupt
	IFE_ENG_INTERRUPT_RING_BUF_ERR    = 0x00000100,  ///< enable interrupt: ring buffer error interrupt
	IFE_ENG_INTERRUPT_FRAME_ERR       = 0x00000200,  ///< enable interrupt: frame error interrupt (for direct mode)
	//IFE_ENG_INTERRUPT_DEC3_ERR        = 0x00000400,  ///< enable interrupt: rde decode error interrupt
	//IFE_ENG_INTERRUPT_RING_BUF_ERR2   = 0x00000800,  ///< enable interrupt: ring buffer error 2 interrupt
	IFE_ENG_INTERRUPT_SIE_FRAME_START = 0x00001000,  ///< enable interrupt: SIE frame start interrupt (for direct mode)
	IFE_ENG_INTERRUPT_FRAME_START     = 0x00004000,  ///< enable interrupt: SIE frame start interrupt (for direct mode)

	IFE_ENG_INTERRUPT_NN_ISP_P0_SLICE_READY = 0x00010000,  ///< enable interrupt: Ife NN_ISP path 0 send 1 slice data to ring buffer
	IFE_ENG_INTERRUPT_NN_ISP_P0_SLICE_CLEAR = 0x00020000,  ///< enable interrupt: Ife NN_ISP path 0 get 1 slice data done form output
	IFE_ENG_INTERRUPT_NN_ISP_P1_SLICE_READY = 0x00040000,  ///< enable interrupt: Ife NN_ISP path 1 send 1 slice data to ring
	IFE_ENG_INTERRUPT_NN_ISP_P1_SLICE_CLEAR = 0x00080000,  ///< enable interrupt: Ife NN_ISP path 1 get 1 slice data done form output
	IFE_ENG_INTERRUPT_ALL             = (IFE_ENG_INTERRUPT_FMD | IFE_ENG_INTERRUPT_DEC1_ERR | IFE_ENG_INTERRUPT_DEC2_ERR | IFE_ENG_INTERRUPT_LLEND | IFE_ENG_INTERRUPT_LLERR |
										 IFE_ENG_INTERRUPT_LLERR2 | IFE_ENG_INTERRUPT_LLJOBEND | IFE_ENG_INTERRUPT_BUFOVFL | IFE_ENG_INTERRUPT_RING_BUF_ERR | IFE_ENG_INTERRUPT_FRAME_ERR |
										 IFE_ENG_INTERRUPT_NN_ISP_P0_SLICE_READY | IFE_ENG_INTERRUPT_NN_ISP_P0_SLICE_CLEAR | IFE_ENG_INTERRUPT_NN_ISP_P1_SLICE_READY | 
										 IFE_ENG_INTERRUPT_NN_ISP_P1_SLICE_CLEAR | IFE_ENG_INTERRUPT_SIE_FRAME_START | IFE_ENG_INTERRUPT_FRAME_START)
} IFE_ENG_INTERRUPT;

typedef struct {
	IFE_OPMODE mode;
	IFE_BITDEPTH in_bits;
	IFE_BITDEPTH out_bits;
	IFE_BAYERFMTSEL bayer_fmt;
	UINT32 cfa_pat;
	UINT32 binning;
	UINT32 fusion_num;
} IFE_ENG_CONTROL_INFO;



typedef struct {
	UINT32 ofs;
	UINT32 val;
} IFE_ENG_REG;
/*****************************************************************************/
#if 0


IFE_ENG_REG ife_eng_gen_ctl_reg(IFE_ENG_CONTROL_INFO *info);
IFE_ENG_REG ife_eng_gen_inte_en_reg(UINT32 inte);
IFE_ENG_REG ife_eng_gen_stripe_reg(UINT32 hn, UINT32 hl, UINT32 hm);
IFE_ENG_REG ife_eng_gen_size_reg0(UINT32 width, UINT32 height);
IFE_ENG_REG ife_eng_gen_size_reg1(UINT32 crop_width, UINT32 crop_height);
IFE_ENG_REG ife_eng_gen_size_reg2(UINT32 crop_width, UINT32 crop_height);
IFE_ENG_REG ife_eng_gen_dma_in_reg0(UINT32 addr);
IFE_ENG_REG ife_eng_gen_lofs_reg0(UINT32 lofs, UINT32 h_start_shift);
IFE_ENG_REG ife_eng_gen_dma_in_reg1(UINT32 addr);
IFE_ENG_REG ife_eng_gen_in_lofs_reg1(UINT32 lofs);
IFE_ENG_REG ife_eng_gen_dma_out_reg0(UINT32 addr);
IFE_ENG_REG ife_eng_gen_out_lofs_reg(UINT32 lofs);
IFE_ENG_REG ife_eng_gen_dram_setting_reg(UINT32 input_burst_mode, UINT32 output_burst_mode, UINT32 loop_line, UINT32 loop_en);
#endif

extern INT32 ife_eng_init(UINT32 chip_num, UINT32 eng_num);
extern INT32 ife_eng_release(void);
extern INT32 ife_eng_init_resource(IFE_ENG_HANDLE *p_eng);
extern IFE_ENG_HANDLE *ife_eng_get_handle(UINT32 chip_id, UINT32 eng_id);
extern void ife_eng_reg_isr_callback(IFE_ENG_HANDLE *p_eng, IFE_ISR_CB cb);
extern INT32 ife_eng_open(IFE_ENG_HANDLE *p_eng);
extern INT32 ife_eng_close(IFE_ENG_HANDLE *p_eng);
extern void ife_eng_trig_single_hw_reg(IFE_ENG_HANDLE *p_eng);
extern void ife_eng_trig_ll_hw_reg(IFE_ENG_HANDLE *p_eng, ULONG ll_addr, UINT32 ll_addr_msb);
extern void ife_eng_write_hw_reg(IFE_ENG_HANDLE *p_eng, uintptr_t reg_ofs, UINT32 val);
extern void ife_eng_hard_reset_hw_reg(IFE_ENG_HANDLE *p_eng);
extern void ife_eng_isr_hw_reg(IFE_ENG_HANDLE *p_eng);
extern void ife_eng_int_soft_reset_hw_reg(IFE_ENG_HANDLE *p_eng);
extern void ife_eng_set_enable_int_hw_reg(IFE_ENG_HANDLE *p_eng, UINT32 intr);
extern void ife_eng_ll_fire_hw_reg(IFE_ENG_HANDLE *p_eng);

extern void ife_eng_set_load_hw_reg(IFE_ENG_HANDLE *p_eng, UINT32 load_type);
extern void ife_eng_stop_single_hw_reg(IFE_ENG_HANDLE *p_eng);

extern UINT32 ife_eng_get_reg_base_buf_size(VOID);  // unit: byte
extern UINT32 ife_eng_get_reg_flag_buf_size(VOID);   // unit: byte
extern VOID ife_eng_set_reg_buf(IFE_ENG_HANDLE *p_eng, ULONG reg_base_addr, ULONG reg_flag_addr);


extern VOID ife_eng_wait_flag_frame_end(IFE_ENG_HANDLE *p_eng, BOOL is_clear_flag);
extern VOID ife_eng_wait_flag_linked_list_end(IFE_ENG_HANDLE *p_eng, BOOL is_clear_flag);
extern VOID ife_eng_wait_flag_linked_list_job_end(IFE_ENG_HANDLE *p_eng, BOOL is_clear_flag);
extern VOID ife_eng_wait_flag_sie_frame_start(IFE_ENG_HANDLE *p_eng, BOOL is_clear_flag);
extern VOID ife_eng_clear_flag_frame_end(IFE_ENG_HANDLE *p_eng);
extern VOID ife_eng_clear_flag_linked_list_end(IFE_ENG_HANDLE *p_eng);
extern VOID ife_eng_clear_flag_linked_list_job_end(IFE_ENG_HANDLE *p_eng);

extern INT32 ife_eng_chk_limitation(ULONG reg_base_addr, ULONG reg_flag_addr);

extern VOID ife_eng_dma_channel_enable_hw_reg(IFE_ENG_HANDLE *p_eng, BOOL set_en);
extern UINT8 ife_eng_get_dma_channel_status_hw_reg(IFE_ENG_HANDLE *p_eng);

extern VOID ife_eng_set_dbg_level(UINT32 level);

extern VOID ife_eng_set_axi_disable_hw_reg(IFE_ENG_HANDLE *p_eng, BOOL enable);
extern void ife_eng_get_wdr_hist_hw_reg(IFE_ENG_HANDLE *p_eng, IFE_ENG_WDR_HIST_RESULT *p_hist_result);

extern VOID ife_eng_wait_flag_nn_isp_p0_slice_ready(IFE_ENG_HANDLE *p_eng, BOOL is_clear_flag);
extern VOID ife_eng_clear_flag_nn_isp_p0_slice_ready(IFE_ENG_HANDLE *p_eng);
extern VOID ife_eng_wait_flag_nn_isp_p0_slice_clear(IFE_ENG_HANDLE *p_eng, BOOL is_clear_flag);
extern VOID ife_eng_clear_flag_nn_isp_p0_slice_clear(IFE_ENG_HANDLE *p_eng);


extern VOID ife_eng_wait_flag_nn_isp_p1_slice_ready(IFE_ENG_HANDLE *p_eng, BOOL is_clear_flag);
extern VOID ife_eng_clear_flag_nn_isp_p1_slice_ready(IFE_ENG_HANDLE *p_eng);
extern VOID ife_eng_wait_flag_nn_isp_p1_slice_clear(IFE_ENG_HANDLE *p_eng, BOOL is_clear_flag);
extern VOID ife_eng_clear_flag_nn_isp_p1_slice_clear(IFE_ENG_HANDLE *p_eng);

extern VOID ife_eng_get_va_result_buf_reg(IFE_VA_SETTING *p_va_info, IFE_VA_WIN_PARAM *p_va_win, IFE_VA_RSLT *p_va_rslt);
extern VOID ife_eng_get_indep_va_win_rslt_hw_reg(IFE_ENG_HANDLE *p_eng, IFE_INDEP_VA_WIN_RSLT *p_indepva_rslt, UINT32 win_idx);

extern VOID ife_eng_get_va_normalization_result_buf_reg(IFE_VA_SETTING *p_va_info, IFE_VA_WIN_PARAM *p_va_win, IFE_VA_RSLT *p_va_rslt, UINT32 im_width, UINT32 im_height);
extern VOID ife_eng_get_indep_va_win_normalization_rslt_hw_reg(IFE_ENG_HANDLE *p_eng, IFE_INDEP_VA_WIN_RSLT *p_indepva_rslt, UINT32 win_idx, UINT32 im_width, UINT32 im_height);

extern VOID ife_eng_set_nn_isp_p0_ring_buf_slice_clear_hw_reg(IFE_ENG_HANDLE *p_eng, UINT32 set_en);
extern VOID ife_eng_set_nn_isp_p0_out_buf_slice_ready_hw_reg(IFE_ENG_HANDLE *p_eng, UINT32 set_en);
extern VOID ife_eng_set_nn_isp_p1_ring_buf_slice_clear_hw_reg(IFE_ENG_HANDLE *p_eng, UINT32 set_en);
extern VOID ife_eng_set_nn_isp_p1_out_buf_slice_ready_hw_reg(IFE_ENG_HANDLE *p_eng, UINT32 set_en);
extern UINT32 ife_eng_set_nn_isp_p0_ring_buf_read_set_slice_ready_status(IFE_ENG_HANDLE *p_eng);
extern UINT32 ife_eng_set_nn_isp_p0_ring_buf_read_set_slice_clear_status(IFE_ENG_HANDLE *p_eng);
extern UINT32 ife_eng_set_nn_isp_p1_ring_buf_read_set_slice_ready_status(IFE_ENG_HANDLE *p_eng);
extern UINT32 ife_eng_set_nn_isp_p1_ring_buf_read_set_slice_clear_status(IFE_ENG_HANDLE *p_eng);

#if (defined(_NVT_EMULATION_) == ON)

extern ULONG ife_get_dram_in_addr(IFE_ENG_HANDLE *p_eng);

extern ULONG ife_get_dram_in_addr2(IFE_ENG_HANDLE *p_eng);

extern UINT32 ife_get_dram_in_lofs(IFE_ENG_HANDLE *p_eng);

extern UINT32 ife_get_dram_in_lofs2(IFE_ENG_HANDLE *p_eng);

extern ULONG ife_get_dram_out_addr(IFE_ENG_HANDLE *p_eng);

extern ULONG ife_get_dram_out_uv_addr(IFE_ENG_HANDLE *p_eng);

extern UINT32 ife_get_dram_out_lofs(IFE_ENG_HANDLE *p_eng);

extern UINT32 ife_get_dram_out_uv_lofs(IFE_ENG_HANDLE *p_eng);

extern UINT32 ife_get_in_vsize(IFE_ENG_HANDLE *p_eng);

extern UINT32 ife_get_in_hsize(IFE_ENG_HANDLE *p_eng);

extern BOOL ife_end_time_out_status;

extern UINT32 ife_get_dram_subimg_in_lofs(IFE_ENG_HANDLE *p_eng);

extern ULONG ife_get_dram_subimg_out_addr(IFE_ENG_HANDLE *p_eng);

extern UINT32 ife_get_dram_subimg_out_lofs(IFE_ENG_HANDLE *p_eng);

extern UINT32 ife_get_in_sub_vsize(IFE_ENG_HANDLE *p_eng);

extern UINT32 ife_get_in_sub_hsize(IFE_ENG_HANDLE *p_eng);

extern ULONG ife_get_dram_subimg_in_addr(IFE_ENG_HANDLE *p_eng);

extern UINT32 ife_get_subisp_out_vsize(IFE_ENG_HANDLE *p_eng);

extern UINT32 ife_get_subisp_out_hsize(IFE_ENG_HANDLE *p_eng);

extern UINT32 ife_get_dram_va_out_lofs(IFE_ENG_HANDLE *p_eng);

extern ULONG ife_get_dram_va_out_addr(IFE_ENG_HANDLE *p_eng);

#endif




#endif
