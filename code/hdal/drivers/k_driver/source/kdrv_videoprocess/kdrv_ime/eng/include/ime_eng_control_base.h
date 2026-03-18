
#ifndef _IME_ENG_CTRL_BASE_H_
#define _IME_ENG_CTRL_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "kwrap/type.h"
#include "ime_eng_int_comm.h"



/**
    IME enum -  low delay mode path selection
*/
typedef enum _IME_LOW_DELAY_CHL_SEL_ {
	IME_LOW_DELAY_CHL_REFOUT = 0,   ///< IME 3dnr reference output
	IME_LOW_DELAY_CHL_PATH1  = 1,   ///< IME output path1
	IME_LOW_DELAY_CHL_PATH2  = 2,   ///< IME output path2
	IME_LOW_DELAY_CHL_PATH3  = 3,   ///< IME output path3
	ENUM_DUMMY4WORD(IME_LOW_DELAY_CHL_SEL)
} IME_LOW_DELAY_CHL_SEL;


/**
    IME enum - burst length selection
*/
typedef enum _IME_BURST_SEL {
    IME_BURST_64W   = 0,  ///< burst size: 64 word
	IME_BURST_32W   = 1,  ///< burst size: 32 word	
	ENUM_DUMMY4WORD(IME_BURST_SEL)
} IME_BURST_SEL;


/**
    IME enum - burst length selection
*/
typedef enum _IME_BREAK_POINT_MODE_SEL_ {
	IME_BP_LINE_MODE   = 0,   ///< line based break-point
	IME_BP_PIXEL_MODE   = 1,  ///< pixel based break-point
	ENUM_DUMMY4WORD(IME_BREAK_POINT_MODE_SEL)
} IME_BREAK_POINT_MODE_SEL;



/**
    IME structure - change DMA channel burst length parameters
*/
typedef struct _IME_BURST_LENGTH_ {
	uint8_t bst_input_y;                ///< input Y, recommend: 32 words
	uint8_t bst_input_u;                ///< input U, recommend: 32 words
	uint8_t bst_input_v;                ///< input V, recommend: 32 words
	uint8_t bst_output_path1_y;         ///< output path1 Y, recommend: 64 words
	uint8_t bst_output_path1_u;         ///< output path1 U, recommend: 64 words
	uint8_t bst_output_path1_v;         ///< output path1 V, recommend: 32 words
	uint8_t bst_output_path2_y;         ///< output path2 Y, recommend: 64 words
	uint8_t bst_output_path2_u;         ///< output path2 U, recommend: 64 words
	uint8_t bst_output_path3_y;         ///< output path3 Y, recommend: 32 words
	uint8_t bst_output_path3_u;         ///< output path3 U, recommend: 32 words
	uint8_t bst_output_path4_y;         ///< output path4 Y, recommend: 32 words
	uint8_t bst_output_path4_u;         ///< output path4 Y, recommend: 32 words
	uint8_t bst_input_pm_pxl;           ///< privacy mask input, recommend: 32 words
	uint8_t bst_subout_pxl;             ///< privacy mask subout, recommend: 32 words
	uint8_t bst_input_tmnr_fcp;         ///< data stamp input, recommend: 32 words
	uint8_t bst_output_tmnr_fcp;        ///< privacy mask input pixelation, recommend: 32 words
	uint8_t bst_tmnr_input_y;           ///< 3DNR of input luma, recommend: 64 words
	uint8_t bst_tmnr_input_c;           ///< 3DNR of input chroma, recommend: 64 words
	uint8_t bst_tmnr_output_y;          ///< 3DNR of output luma, recommend: 64 words
	uint8_t bst_tmnr_output_c;          ///< 3DNR of output chroma, recommend: 64 words
	uint8_t bst_tmnr_input_mv;          ///< 3DNR of input motion vector, recommend: 32 words
	uint8_t bst_tmnr_output_mv;         ///< 3DNR of output motion vector, recommend: 32 words
	//uint8_t bst_tmnr_input_mo_sta;      ///< 3DNR of input motion STA, recommend: 32 words
	//uint8_t bst_tmnr_output_mo_sta;     ///< 3DNR of output motion STA, recommend: 32 words
	uint8_t bst_tmnr_output_mo_sta_roi; ///< 3DNR of output motion STA for ROI, recommend: 32 words
	uint8_t bst_tmnr_output_sta;        ///< 3DNR of output statistic, recommend: 32 words
	uint8_t bst_va_output;              ///< VA output, recommend: 32 words
} IME_BURST_LENGTH;

//--------------------------------------------------------------

extern VOID ime_eng_set_direct_path_control_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 src_ctrl);

extern VOID ime_eng_set_linked_list_addr_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_addr_lsb32, UINT32 set_addr_msb32);

extern VOID ime_eng_set_break_point_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_bp0, UINT32 set_bp1, UINT32 set_bp2, IME_BREAK_POINT_MODE_SEL set_bp_mode);

extern VOID ime_eng_set_single_output_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en, UINT32 set_ch);

extern VOID ime_eng_set_low_delay_mode_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en, UINT32 set_sel);

extern VOID ime_eng_set_dram_out_status_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en, UINT8 set_bit);

extern VOID ime_eng_set_interrupt_status_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_val);
extern VOID ime_eng_set_interrupt_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_val);
extern VOID ime_eng_set_linked_list_terminate_hw_reg(ULONG eng_base_addr, UINT32 set_terminate);

extern VOID ime_eng_set_axi_bus_control_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);

extern VOID ime_eng_set_line_cycle_interrupt_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_path, UINT32 set_lines);

extern VOID ime_eng_set_nn_isp_p2_in_buf_ready_hw_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);
extern VOID ime_eng_set_nn_isp_p2_out_buf_clear_hw_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);
extern VOID ime_eng_set_nn_isp_p3_in_buf_ready_hw_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);
extern VOID ime_eng_set_nn_isp_p3_out_buf_clear_hw_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);



#if (IME_GET_API_EN == 1)
extern UINT32 ime_eng_get_start_status_hw_reg(IME_ENG_HANDLE *p_eng, ULONG eng_base_addr);
extern UINT32 ime_eng_get_direct_path_control_hw_reg(IME_ENG_HANDLE *p_eng, ULONG eng_base_addr);

extern UINT32 ime_eng_get_interrupt_enable_hw_reg(ULONG eng_base_addr);
extern UINT32 ime_eng_get_interrupt_status_hw_reg(ULONG eng_base_addr);

extern UINT32 ime_eng_get_function_enable_status_hw_reg(ULONG eng_base_addr);

extern INT32 ime_eng_get_burst_size_hw_reg(ULONG eng_base_addr, UINT32 chl_num);



#endif

#ifdef __cplusplus
}
#endif


#endif

