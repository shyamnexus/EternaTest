

#ifndef _IME_ENG_VA_BASE_H_
#define _IME_ENG_VA_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif



#include "kwrap/type.h"
#include "ime_eng_int_comm.h"


//use for 34 bit-->32 bit
#define IME_VA_VALUE_SHIT   (4)


typedef enum _IME_EMU_VA_FLTR_SYMM_SEL_ {
	IME_VA_FLTR_MIRROR  = 0,   ///<
	IME_VA_FLTR_INVERSE = 1,   ///<
	ENUM_DUMMY4WORD(IME_VA_FLTR_SYMM_SEL)
} IME_VA_FLTR_SYMM_SEL;

typedef enum _IME_EMU_VA_FLTR_SIZE_SEL_ {
	IME_VA_FLTR_SIZE_1  = 0,   ///<
	IME_VA_FLTR_SIZE_3  = 1,   ///<
	IME_VA_FLTR_SIZE_5  = 2,   ///<
	IME_VA_FLTR_SIZE_7  = 3,   ///<
	ENUM_DUMMY4WORD(IME_VA_FLTR_SIZE_SEL)
} IME_VA_FLTR_SIZE_SEL;


typedef struct IME_VA_FLTR_PARAM_ {
	UINT8                   tap_a;     ///<
	INT8                    tap_b;      ///<
	INT8                    tap_c;      ///<
	INT8                    tap_d;      ///<
	IME_VA_FLTR_SYMM_SEL    filt_symm;   ///<
	IME_VA_FLTR_SIZE_SEL    fltr_size;   ///<
	UINT8                   div;      ///<
	UINT16                  th_low;      ///<
	UINT16                  th_high;      ///<
} IME_VA_FLTR_PARAM;


typedef struct _IME_VA_FLTR_GROUP_PARAM_ {
	IME_VA_FLTR_PARAM       filt_h;
	IME_VA_FLTR_PARAM       filt_v;
	//BOOL                    linemax_en;   ///<
	BOOL                    cnt_en;       ///<
	UINT8                   energy_wet;     ///< Edge strength weight
} IME_VA_FLTR_GROUP_PARAM;


typedef struct _IME_VA_WIN_PARAM_ {
	UINT32              win_stx;    ///< Variation accumulation - horizontal starting point
	UINT32              win_sty;    ///< Variation accumulation - vertical starting point
	UINT32              win_numx;    ///< Variation accumulation - horizontal window number
	UINT32              win_numy;    ///< Variation accumulation - vertical window number
	UINT32              win_szx;    ///< Variation accumulation - horizontal window size
	UINT32              win_szy;    ///< Variation accumulation - vertical window size
	UINT32              win_spx;    ///< Variation accumulation - horizontal window spacing
	UINT32              win_spy;    ///< Variation accumulation - vertical window spacing
} IME_VA_WIN_PARAM;

typedef struct _IME_INDEP_VA_PARAM_ {
	BOOL                indep_va_en;
	UINT32              win_stx;    ///< horizontal window start
	UINT32              win_sty;    ///< vertical window start
	UINT32              win_szx;    ///< horizontal window size
	UINT32              win_szy;    ///< vertical window size
	//BOOL                linemax_g1_en;
	//BOOL                linemax_g2_en;
} IME_INDEP_VA_PARAM;


typedef enum _IME_EMU_VA_OUTSEL_ {
	IME_VA_OUT_GROUP1  = 0,   ///<
	IME_VA_OUT_BOTH    = 1,   ///<
	ENUM_DUMMY4WORD(IME_VA_OUTSEL)
} IME_VA_OUTSEL;


typedef struct _IME_VA_RSLT_ {
	UINT32 *p_g1_h;
	UINT32 *p_g1_v;
	UINT32 *p_g2_h;
	UINT32 *p_g2_v;
	UINT32 *p_g1_h_cnt;
	UINT32 *p_g1_v_cnt;
	UINT32 *p_g2_h_cnt;
	UINT32 *p_g2_v_cnt;
} IME_VA_RSLT;


typedef struct _IME_VA_SETTING_ {
	BOOL va_en;
	UINT32 win_num_x;
	UINT32 win_num_y;
	IME_VA_OUTSEL outsel;
	ULONG address;
	UINT32 lineoffset;
} IME_VA_SETTING;


typedef struct _IME_INDEP_VA_WIN_RSLT_ {
	UINT32              va_g1_h;
	UINT32              va_g1_v;
	UINT32              va_g2_h;
	UINT32              va_g2_v;
	UINT32              va_cnt_g1_h;
	UINT32              va_cnt_g1_v;
	UINT32              va_cnt_g2_h;
	UINT32              va_cnt_g2_v;
} IME_INDEP_VA_WIN_RSLT;


typedef struct _IME_VA_LDG_PARAM_ {
	UINT8              low_slope;       ///< Dark region gain slope
	UINT8              high_slope;      ///< Bright region gain slope
	UINT8              low_th;          ///< Darkness threshold
	UINT8              high_th;         ///< Brightness threshold
	UINT8              low_gain;        ///< Dark region minima gain
	UINT8              high_gain;       ///< Bright region minima gain
} IME_VA_LDG_PARAM;


typedef struct {
	BOOL va_en;
	UINT32 win_num_x;
	UINT32 win_num_y;
	UINT32 win_start_x;
	//UINT32 win_start_y;
	UINT32 win_size_x;
	//UINT32 win_size_y;
	UINT32 win_skip_x;
	//UINT32 win_skip_y;
	//UINT32 dual_width;
	UINT32 dual_cut_pos;

	IME_VA_OUTSEL outsel;
	ULONG address_pipe1;
	ULONG address_pipe2;
	ULONG address_out;
	ULONG lineoffset;
} IME_DUAL_VA_SETTING;


typedef struct {
	UINT32              *p_va_g1_h;
	UINT32              *p_va_g1_v;
	UINT32              *p_va_g2_h;
	UINT32              *p_va_g2_v;
	UINT32              *p_va_cnt_g1_h;
	UINT32              *p_va_cnt_g1_v;
	UINT32              *p_va_cnt_g2_h;
	UINT32              *p_va_cnt_g2_v;
} IME_INDEP_VA_WIN_MERGE;




//extern IME_VA_SETTING ime_va_ring_setting[2];

extern VOID ime_eng_set_va_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);
extern VOID ime_eng_set_va_win_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set0_en, UINT32 set1_en, UINT32 set2_en, UINT32 set3_en, UINT32 set4_en);


extern VOID ime_eng_set_va_out_sel_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);
extern VOID ime_eng_set_va_filter_g1_buf_reg(IME_ENG_HANDLE *p_eng, IME_VA_FLTR_GROUP_PARAM *p_va_fltr_g1);
extern VOID ime_eng_set_va_filter_g2_buf_reg(IME_ENG_HANDLE *p_eng, IME_VA_FLTR_GROUP_PARAM *p_va_fltr_g2);


extern VOID ime_eng_set_va_mode_enable_buf_reg(IME_ENG_HANDLE *p_eng, IME_VA_FLTR_GROUP_PARAM *p_va_fltr_g1, IME_VA_FLTR_GROUP_PARAM *p_va_fltr_g2);

extern VOID ime_eng_set_va_win_info_buf_reg(IME_ENG_HANDLE *p_eng, IME_VA_WIN_PARAM *p_va_win);

extern VOID ime_eng_set_va_indep_win_buf_reg(IME_ENG_HANDLE *p_eng, IME_INDEP_VA_PARAM  *p_indep_va_win_info, UINT32 win_idx);


extern VOID ime_eng_set_dma_out_va_addr_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_addr_lsb32, UINT32 set_addr_msb32);
extern VOID ime_eng_set_dma_out_va_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_lofs, UINT32 set_start_ofs);

extern VOID ime_eng_set_va_edge_output_counter_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_sel,  UINT8 set_luma_th);
extern VOID ime_eng_set_va_ldg_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT8 set_en);
extern VOID ime_eng_set_va_ldg_control_buf_reg(IME_ENG_HANDLE *p_eng, IME_VA_LDG_PARAM *p_set_param);

extern VOID ime_eng_set_va_dual_config_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 start_offset, UINT32 left_overlap, UINT32 right_overlap);



#ifdef __cplusplus
}
#endif


#endif

