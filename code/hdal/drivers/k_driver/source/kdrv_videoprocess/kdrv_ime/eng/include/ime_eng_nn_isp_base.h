
#ifndef _IME_ENG_NN_ISP_BASE_H_
#define _IME_ENG_NN_ISP_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif


//#include "kwrap/type.h"
#include "ime_eng_int_comm.h"


typedef enum _IME_NN_ISP_PATH_ {
	IME_NN_ISP_PATH2_3DNR = 0,   ///< 3dnr
	IME_NN_ISP_PATH3_2DNR = 1,   ///< 2dnr
	ENUM_DUMMY4WORD(IME_NN_ISP_PATH_SEL)
} IME_NN_ISP_PATH_SEL;

//------------------------------------------------


extern VOID ime_eng_set_nn_isp_enable_buf_reg(IME_ENG_HANDLE *p_eng, IME_NN_ISP_PATH_SEL path_sel, UINT32 set_en);

extern VOID ime_eng_set_nn_isp_fw_handshake_enable_buf_reg(IME_ENG_HANDLE *p_eng, IME_NN_ISP_PATH_SEL path_sel, UINT32 set_en);

extern VOID ime_eng_set_nn_isp_slice_info_buf_reg(IME_ENG_HANDLE *p_eng, IME_NN_ISP_PATH_SEL path_sel, UINT32 ring_buf_height, UINT32 slice_height, UINT32 slice_ovlap);

extern VOID ime_eng_set_nn_isp_output_height_buf_reg(IME_ENG_HANDLE *p_eng, IME_NN_ISP_PATH_SEL path_sel, UINT32 buf_height);

extern VOID ime_eng_set_nn_isp_yuv_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, IME_NN_ISP_PATH_SEL path_sel, UINT32 lofs);

extern VOID ime_eng_set_nn_isp_slice_input_buf_y_addr_buf_reg(IME_ENG_HANDLE *p_eng, IME_NN_ISP_PATH_SEL path_sel, UINT32 addr_lsb, UINT32 addr_msb);

extern VOID ime_eng_set_nn_isp_slice_input_buf_uv_addr_buf_reg(IME_ENG_HANDLE *p_eng, IME_NN_ISP_PATH_SEL path_sel, UINT32 addr_lsb, UINT32 addr_msb);

extern VOID ime_eng_set_nn_isp_slice_output_ring_buf_y_addr_buf_reg(IME_ENG_HANDLE *p_eng, IME_NN_ISP_PATH_SEL path_sel, UINT32 addr_lsb, UINT32 addr_msb);

extern VOID ime_eng_set_nn_isp_slice_output_ring_buf_uv_addr_buf_reg(IME_ENG_HANDLE *p_eng, IME_NN_ISP_PATH_SEL path_sel, UINT32 addr_lsb, UINT32 addr_msb);

extern VOID ime_eng_set_nn_isp_texture_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, IME_NN_ISP_PATH_SEL path_sel, UINT32 txt_lofs);

extern VOID ime_eng_set_nn_isp_texture_map_addr_buf_reg(IME_ENG_HANDLE *p_eng, IME_NN_ISP_PATH_SEL path_sel, UINT32 addr_lsb, UINT32 addr_msb);


extern VOID ime_eng_set_nn_isp_motion_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, IME_NN_ISP_PATH_SEL path_sel, UINT32 mot_lofs);

extern VOID ime_eng_set_nn_isp_motion_map_input_addr_buf_reg(IME_ENG_HANDLE *p_eng, IME_NN_ISP_PATH_SEL path_sel, UINT32 addr_lsb, UINT32 addr_msb);

extern VOID ime_eng_set_nn_isp_motion_map_output_addr_buf_reg(IME_ENG_HANDLE *p_eng, IME_NN_ISP_PATH_SEL path_sel, UINT32 addr_lsb, UINT32 addr_msb);

extern UINT32 ime_eng_cal_nn_isp_buf_output_lines(UINT32 height, UINT32 slice_base_height, UINT32 slice_ovlap, UINT32 *p_buf_out_lines);

extern UINT32 ime_eng_cal_nn_isp_spectial_case_enable(UINT32 height, UINT32 slice_height, UINT32 slice_overlap);

#ifdef __cplusplus
}
#endif


#endif // _IME_DBCS_REG_
