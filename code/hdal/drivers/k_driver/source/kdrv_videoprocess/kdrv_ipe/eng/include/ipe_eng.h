/**
    Public header file for IPE module.

    @file       ipe_eng.h
    @ingroup    mIIPPIPE

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/


#ifndef _IPE_ENG_H_
#define _IPE_ENG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kdrv_type.h"

#include "ipe_eng_handle.h"
#include "ipe_eng_base.h"
#include "ipe_eng_limt.h"

#define IPE_ENG_REG_NUM     1584

#define IPE_ENG_MAXVSTRP    8192
#define IPE_ENG_MINVSTRP    16

#define IPE_ENG_CFA_FREQ_BLEND_LUT_SZ 16
#define IPE_ENG_CFA_LUMA_WEIGHT_LUT_SZ 17
#define IPE_ENG_CFA_FCS_STRENGTH_LUT_SZ 16


/*****************************************************************************/
//typedef void (*IPE_ISR_CB)(void *eng, UINT32 status, void *reserve);

typedef enum {
	IPE_ENG_INTERRUPT_FRM_END           = 0x00000002,
	IPE_ENG_INTERRUPT_ST                = 0x00000004,
	IPE_ENG_INTERRUPT_FRMSTART          = 0x00000008,
	IPE_ENG_INTERRUPT_YCOUTEND          = 0x00000010,
	IPE_ENG_INTERRUPT_DMAIN0END         = 0x00000020,
	IPE_ENG_INTERRUPT_DFG_SUBOUT_END    = 0x00000040,
    IPE_ENG_INTERRUPT_LCE_SUBOUT_END    = 0x00000080,
	IPE_ENG_INTERRUPT_VAOUTEND          = 0x00000100,
	IPE_ENG_INTERRUPT_LL_DONE           = 0x00000200,
	IPE_ENG_INTERRUPT_LL_JOB_END        = 0x00000400,
	IPE_ENG_INTERRUPT_CFA_SUBOUT_END    = 0x00000800,
	IPE_ENG_INTERRUPT_LL_ERROR          = 0x00001000,
	IPE_ENG_INTERRUPT_LL_ERROR2         = 0x00002000,
	IPE_ENG_INTERRUPT_FRAMEERR          = 0x00004000,
	IPE_ENG_INTERRUPT_DMAIN1END         = 0x00008000,
	IPE_ENG_INTERRUPT_ENC_OUTOVEL       = 0x00010000,	
	IPE_ENG_INTERRUPT_ALL               = (IPE_ENG_INTERRUPT_FRM_END | IPE_ENG_INTERRUPT_ST | IPE_ENG_INTERRUPT_FRMSTART |
	                                       IPE_ENG_INTERRUPT_YCOUTEND | IPE_ENG_INTERRUPT_DMAIN0END | IPE_ENG_INTERRUPT_DFG_SUBOUT_END |
										   IPE_ENG_INTERRUPT_LCE_SUBOUT_END | IPE_ENG_INTERRUPT_VAOUTEND | IPE_ENG_INTERRUPT_LL_DONE |
										   IPE_ENG_INTERRUPT_LL_JOB_END | IPE_ENG_INTERRUPT_CFA_SUBOUT_END |
										   IPE_ENG_INTERRUPT_LL_ERROR | IPE_ENG_INTERRUPT_LL_ERROR2 | IPE_ENG_INTERRUPT_FRAMEERR | 
										   IPE_ENG_INTERRUPT_DMAIN1END | IPE_ENG_INTERRUPT_ENC_OUTOVEL),
} IPE_ENG_INTERRUPT;

typedef enum {
	/* 3 channel */
	IPE_ENG_FMT_Y_PACK_UV444 = 0,       ///< 444 format y planar UV pack UVUVUVUVUV.....
	IPE_ENG_FMT_Y_PACK_UV422,       ///< 422 format y planar UV pack UVUVUVUVUV.....
	IPE_ENG_FMT_Y_PACK_UV420,       ///< 420 format y planar UV pack UVUVUVUVUV.....
	/* extend */
	IPE_ENG_FMT_Y,              ///< Only Y information
	IPE_ENG_BAYER,              ///< bayer
} IPE_ENG_FMT;

typedef enum {
	IPE_ENG_DROP_RIGHT = 0,
	IPE_ENG_DROP_LEFT,
} IPE_ENG_SUB_H_SEL;

typedef enum {
	IPE_ENG_OUT_ORIGINAL = 0,
//	IPE_ENG_OUT_DIRECTION = 2,
//	IPE_ENG_OUT_ETH = 3,
} IPE_ENG_DRAM_OUT_SEL;
/*
typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	CHAR name[16];

	UINT32 clock_rate;
	void *pclk;         // linux struct clk*, not used in freertos

	ULONG reg_io_base;
	UINT32 irq_id;

	IPE_ISR_CB isr_cb;

    volatile NT98538_IPE_REGISTER_STRUCT *p_ipe_reg_st;
    volatile UINT8 *p_ipe_reg_chg_flag;


} IPE_ENG_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	IPE_ENG_HANDLE *p_eng;
} IPE_ENG_CTL;
*/

/*****************************************************************************/

/**
    IPE load type selection

    Select ipe load type
*/
typedef enum {
	IPE_FRMEND_LOAD = 0,            ///< IPE frame end load
	IPE_START_LOAD = 1,             ///< IPE start load
	IPE_DIRECT_START_LOAD = 2,      ///< IPE start load
	ENUM_DUMMY4WORD(IPE_LOAD_TYPE)
} IPE_LOAD_TYPE;


typedef enum {
	IPE_ENG_OPMODE_D2D = 0, ///< Engine D2D
	IPE_ENG_OPMODE_DIRECT,  ///< IFE -> DCE -> IPE -> IME
	IPE_ENG_OPMODE_ALL_DIRECT, ///< DCE -> IPE -> IME
	IPE_ENG_OPMODE_MAX
} IPE_ENG_OPMODE;

typedef enum {
	IPE_ENG_ETH_OUT_2BITS = 0,             ///< 2 bits/pixel
	IPE_ENG_ETH_OUT_8BITS = 1,             ///< 8 bits/pixel
	ENUM_DUMMY4WORD(IPE_ENG_ETH_OUTFMT)
} IPE_ENG_ETH_OUTFMT;

typedef enum {
	IPE_ENG_ETH_OUT_ORIGINAL = 0,             ///< original size
	IPE_ENG_ETH_OUT_DOWNSAMPLED = 1,          ///< downsampled by 2x2
	IPE_ENG_ETH_OUT_BOTH = 2,                 ///< both origianl size and downsample size
	ENUM_DUMMY4WORD(IPE_ENG_ETH_OUTSEL)
} IPE_ENG_ETH_OUTSEL;

typedef struct {
	UINT32 ofs;
	UINT32 val;
} IPE_ENG_REG;

typedef struct {
	BOOL enable;
	UINT8 y_en;     ////<DMA channel for Y/subsample eth
	UINT8 c_en;     ////<DMA channel for C/region/eth
	UINT8 va_en;    ////<DMA channel for VA
	UINT8 defog_en; ////<DMA channel for defog subout
	UINT8 lce_en;   ////<DMA channel for lce subout
	UINT8 ir_en;    ////<DMA channel for ir subout
	//UINT8 indep_va_en;    ////<DMA channel for indep va subout
	//UINT8 eth_en;   ////<DMA channel for eth subout
} IPE_ENG_DMA_OUT_INFO;

typedef struct {
	UINT32 h_n;         ///< h stripe size in each stripe, used for fixed size mode
	UINT32 h_l;         ///< h stripe size of last stripe, used for fixed size mode
	UINT32 h_m;         ///< h stripe number
	UINT32 v_n;         ///< v stripe size in each stripe, used for fixed size mode
	UINT32 v_l;         ///< v stripe size of last stripe, used for fixed size mode
	UINT32 v_m;         ///< v stripe number
} IPE_ENG_STRIPE_HV_INFO;

typedef struct {
	UINT32 width;
	UINT32 height;
	UINT32 dram_o_en;
	IPE_ENG_DRAM_OUT_SEL dram_o_sel;
	//IPE_ENG_ETH_OUTFMT eth_o_fmt;
	//IPE_ENG_ETH_OUTSEL eth_o_sel;
} IPE_ENG_STRIPE_CAL_INFO;

extern UINT32 ipe_eng_get_reg_base_buf_size(VOID);
extern UINT32 ipe_eng_get_reg_flag_buf_size(VOID);
extern VOID ipe_eng_set_reg_buf(IPE_ENG_HANDLE *p_eng, ULONG reg_base_addr, ULONG reg_flag_addr);


extern INT32 ipe_eng_init(UINT32 chip_num, UINT32 eng_num);
extern INT32 ipe_eng_release(void);
extern INT32 ipe_eng_init_resource(IPE_ENG_HANDLE *p_eng);

extern IPE_ENG_HANDLE *ipe_eng_get_handle(UINT32 chip_id, UINT32 eng_id);
extern VOID ipe_eng_reg_isr_callback(IPE_ENG_HANDLE *p_eng, IPE_ISR_CB cb);
extern INT32 ipe_eng_open(IPE_ENG_HANDLE *p_eng);
extern INT32 ipe_eng_close(IPE_ENG_HANDLE *p_eng);
extern INT32 ipe_eng_cal_hv_stripe(IPE_ENG_STRIPE_CAL_INFO *p_calinfo, IPE_ENG_OPMODE mode, IPE_ENG_STRIPE_HV_INFO *p_info);

extern VOID ipe_eng_load_hw_reg(IPE_ENG_HANDLE *p_eng, UINT32 load_type);

extern VOID ipe_eng_trig_single_hw_reg(IPE_ENG_HANDLE *p_eng);
extern VOID ipe_eng_trig_ll_hw_reg(IPE_ENG_HANDLE *p_eng, UINT32 addr_lsb32, UINT32 addr_msb32);


extern VOID ipe_eng_write_hw_reg(IPE_ENG_HANDLE *p_eng, ULONG reg_ofs, UINT32 val);
extern VOID ipe_eng_stop_single_hw_reg(IPE_ENG_HANDLE *p_eng);
#if (defined(_NVT_EMULATION_) == ON)
extern VOID ipe_eng_set_gamma_rw_enable_hw_reg(IPE_ENG_HANDLE *p_eng, IPE_RW_SRAM_ENUM rw_sel, IPE_RWGAM_OPT opt);
extern VOID ipe_eng_set_3dcc_rw_enable_hw_reg(IPE_ENG_HANDLE *p_eng, IPE_RW_SRAM_ENUM rw_sel);

#endif
extern VOID ipe_eng_isr_hw_reg(IPE_ENG_HANDLE *p_eng);
extern VOID ipe_eng_hard_reset_hw_reg(IPE_ENG_HANDLE *p_eng);

extern VOID ipe_eng_load_gamma_hw_reg(IPE_ENG_HANDLE *p_eng, IPE_RWGAM_OPT opt);
extern VOID ipe_eng_read_gamma_hw_reg(IPE_ENG_HANDLE *p_eng, BOOL cpu_read_en);

extern VOID ipe_eng_load_3dcc_hw_reg(IPE_ENG_HANDLE *p_eng);
extern VOID ipe_eng_read_3dcc_hw_reg(IPE_ENG_HANDLE *p_eng, BOOL cpu_read_en);


extern ER ipe_eng_get_burst_length_hw_reg(IPE_ENG_HANDLE *p_eng, IPE_BURST_LENGTH *p_burstlen);
extern ER ipe_eng_set_burst_length_hw_reg(IPE_ENG_HANDLE *p_eng, IPE_BURST_LENGTH *p_burstlen);
extern IPE_INDEP_VA_PARAM ipe_eng_get_indep_win_info_hw_reg(IPE_ENG_HANDLE *p_eng, UINT32 win_idx);

extern UINT32 ipe_eng_get_va_out_addr_hw_reg(IPE_ENG_HANDLE *p_eng);

extern BOOL ipe_eng_get_va_outsel_hw_reg(IPE_ENG_HANDLE *p_eng);
extern VOID ipe_eng_get_va_rslt_hw_reg(IPE_ENG_HANDLE *p_eng, UINT32 *p_g1_h, UINT32 *p_g1_v, UINT32 *p_g1_hcnt, UINT32 *p_g1_vcnt, UINT32 *p_g2_h, UINT32 *p_g2_v, UINT32 *p_g2_hcnt, UINT32 *p_g2_vcnt, ULONG buff_addr);

extern VOID ipe_eng_get_va_result_buf_reg(IPE_VA_SETTING *p_va_info, IPE_VA_RSLT *p_va_rslt);
extern VOID ipe_eng_get_va_normalization_result_buf_reg(IPE_VA_SETTING *p_va_info, IPE_IMG_SIZE size, IPE_VA_RSLT *p_va_rslt);

//extern VOID ipe_eng_get_dual_va_result_buf_reg(IPE_DUAL_VA_SETTING *p_va_info, IPE_VA_RSLT *p_va_rslt);
//extern VOID ipe_eng_get_dual_va_normalization_result_buf_reg(IPE_DUAL_VA_SETTING *p_va_info, IPE_IMG_SIZE size, IPE_VA_RSLT *p_va_rslt);


extern VOID ipe_eng_get_indep_va_win_rslt_hw_reg(IPE_ENG_HANDLE *p_eng, IPE_INDEP_VA_WIN_RSLT *p_indepva_rslt, UINT32 win_idx);
//extern VOID ipe_eng_get_merged_indep_va_win_rslt_hw_reg(IPE_INDEP_VA_WIN_MERGE* indepva_rslt_left, IPE_INDEP_VA_WIN_MERGE* indepva_rslt_right);
extern VOID ipe_eng_get_indep_va_win_normalization_rslt_hw_reg(IPE_ENG_HANDLE *p_eng, IPE_IMG_SIZE size, IPE_INDEP_VA_WIN_RSLT *p_indepva_rslt, UINT32 win_idx);

extern VOID ipe_eng_get_defog_stcs_hw_reg(IPE_ENG_HANDLE *p_eng, DEFOG_STCS_RSLT *stcs);
extern VOID ipe_eng_get_defog_airlight_setting_hw_reg(IPE_ENG_HANDLE *p_eng, UINT16 *p_val);
extern UINT16 ipe_eng_cal_defog_airlight_nfactor(UINT16 *airlight_vals);

extern UINT32 ipe_eng_set_gamma_curve_buf_size(VOID);
extern ER ipe_eng_set_gamma_curve_to_dram(UINT32 *p_gamma_buffer, UINT32 *r_lut, UINT32 *g_lut, UINT32 *b_lut);

extern UINT32 ipe_eng_set_y_curve_buf_size(VOID);
extern ER ipe_eng_set_y_curve_to_dram(UINT32 *p_ycurve_buffer, UINT32 *y_lut);

extern UINT32 ipe_eng_set_3dcc_buf_size(VOID);
extern ER ipe_eng_set_3dcc_to_dram(UINT32 *lp_3dcc_buffer, UINT32 *lp_3dcc_lut);


extern ER ipe_get_edge_stcs_hw_reg(IPE_ENG_HANDLE *p_eng, IPE_EDGE_STCS_RSLT *stcs_result);
//extern ER ipe_get_merged_edge_stcs_hw_reg(IPE_EDGE_STCS_MERGE *left_stcs_result, IPE_EDGE_STCS_MERGE *right_stcs_result);
//extern VOID ipe_eng_get_merged_defog_stcs_hw_reg(DEFOG_STCS_MERGE *stcs_left, DEFOG_STCS_MERGE *stcs_right);


extern VOID ipe_eng_global_load_hw_reg(IPE_ENG_HANDLE *p_eng, UINT32 set_en);
extern VOID ipe_eng_set_dma_channel_enable_hw_reg(IPE_ENG_HANDLE *p_eng, BOOL set_en);
extern UINT8 ipe_eng_get_dma_channel_status_hw_reg(IPE_ENG_HANDLE *p_eng);

extern VOID ipe_eng_set_axi_bus_control_hw_reg(IPE_ENG_HANDLE *p_eng, UINT32 set_en);


#if (defined(_NVT_EMULATION_) == ON)
//for emulation
extern void ipe_eng_set_stop(IPE_ENG_HANDLE *p_eng);
extern void ipe_eng_set_start_frmend_load(IPE_ENG_HANDLE *p_eng);
extern void ipe_eng_set_start_frmStart_load(IPE_ENG_HANDLE *p_eng);

extern void ipe_eng_set_start(IPE_ENG_HANDLE *p_eng);
//extern void ipe_eng_clear_int(UINT32 intr);
extern void ipe_eng_clear_int(IPE_ENG_HANDLE *p_eng, UINT32 intr);


UINT32 ipe_eng_read_hw_reg(IPE_ENG_HANDLE *p_eng, ULONG reg_ofs);
UINT32 ipe_get_int_status_hw_reg(IPE_ENG_HANDLE *p_eng);
extern VOID ipe_enable_int_hw_reg(IPE_ENG_HANDLE *p_eng, UINT32 intr);


VOID ipe_eng_set_axi_dma_wrapper_control_hw_reg(IPE_ENG_HANDLE *p_eng, UINT32 dma_wrapper_en, UINT32 random_seed);



#endif

extern void ipe_eng_set_dbg_level(UINT32 level);


#if defined (_NVT_EMULATION_)
extern BOOL ipe_end_time_out_status;
#endif

#if 0
//IPE_ENG_REG ipe_eng_gen_mode_reg0(UINT32 mode, UINT32 toime, UINT32 todma, UINT32 dma_sel, UINT32 in_fmt, UINT32 out_fmt, UINT32 sub_h_sel , UINT32 ovlp);
IPE_ENG_REG ipe_eng_gen_mode_reg1(UINT32 func_en);
IPE_ENG_REG ipe_eng_gen_dma_reg0(IPE_ENG_DMA_OUT_INFO *dma_out_info);
IPE_ENG_REG ipe_eng_gen_stripe_reg0(UINT32 hn, UINT32 hl, UINT32 hm);
IPE_ENG_REG ipe_eng_gen_stripe_reg1(UINT32 vl);
IPE_ENG_REG ipe_eng_gen_dma_reg1(UINT32 ll_sai);
IPE_ENG_REG ipe_eng_gen_dma_reg2(UINT32 y_sai);
IPE_ENG_REG ipe_eng_gen_dma_reg3(UINT32 c_sai);
IPE_ENG_REG ipe_eng_gen_dma_reg4(UINT32 ycurve_sai);
IPE_ENG_REG ipe_eng_gen_dma_reg5(UINT32 gamma_sai);
IPE_ENG_REG ipe_eng_gen_dma_reg6(UINT32 y_sai_lofs, UINT32 inrand_en, UINT32 inrand_rst);
IPE_ENG_REG ipe_eng_gen_dma_reg7(UINT32 c_sai_lofs);
IPE_ENG_REG ipe_eng_gen_dma_ycc_chl_reg0(UINT32 y_sao);
IPE_ENG_REG ipe_eng_gen_dma_ycc_chl_reg1(UINT32 c_sao);
IPE_ENG_REG ipe_eng_gen_dma_ycc_chl_reg2(UINT32 y_sao_lofs);
IPE_ENG_REG ipe_eng_gen_dma_ycc_chl_reg3(UINT32 c_sao_lofs);
IPE_ENG_REG ipe_eng_gen_va_chl_reg0(UINT32 va_sao);
IPE_ENG_REG ipe_eng_gen_va_chl_reg1(UINT32 va_sao_lofs);
IPE_ENG_REG ipe_eng_gen_inte_en_buf_reg(UINT32 inte);
IPE_ENG_REG ipe_eng_gen_cst_reg0(INT16 *p_cst, UINT32 cstp_rat);
IPE_ENG_REG ipe_eng_gen_cst_reg1(INT16 *p_cst, UINT32 cstoff_sel);
IPE_ENG_REG ipe_eng_gen_cst_reg2(INT16 *p_cst);
IPE_ENG_REG ipe_eng_gen_cc_adj_buf_reg(UINT32 cbofs, UINT32 crofs, UINT32 con, UINT32 rand_en, UINT32 rand);
#endif

#ifdef __cplusplus
}
#endif


#endif
