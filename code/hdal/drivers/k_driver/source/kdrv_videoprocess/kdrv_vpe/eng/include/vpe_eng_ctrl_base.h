#ifndef _VPE_ENG_CTRL_BASE_H_
#define _VPE_ENG_CTRL_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "vpe_eng_handle.h"

typedef enum {
	VPE_ENG_SRC_RESERVED_0 = 0,//reserved for YUV422
	VPE_ENG_SRC_RESERVED_1 = 1,//reserved for YUV422 MB
	VPE_ENG_SRC_RESERVED_2 = 2,//reserved
	VPE_ENG_SRC_RESERVED_3 = 3,//reserved for YUV420 SCE
	VPE_ENG_SRC_YUV420_SP  = 4,//YUV420 SP
	VPE_ENG_SRC_RESERVED_5 = 5,//reserved for LLC
	VPE_ENG_SRC_RESERVED_6 = 6,//reserved for LLC TUREMODE
	VPE_ENG_SRC_YCC_YUV420 = 7,//YCC_YUV420
	VPE_ENG_SRC_RESERVED_8 = 8,//YCC_YUV420_MB
	//VPE_ENG_SRC_YUV420_HEVC  = 9,//YUV420 SP HEVC
    VPE_ENG_SRC_RESERVED_10  = 10,//YUV420_MB
	VPE_ENG_SRC_MAX
} VPE_ENG_SRC_FMT;



typedef enum _VPE_ENG_ID {
	VPE_ID  = 0,
	VPE_ID_MAX_NUM,
	ENUM_DUMMY4WORD(VPE_ENG_ID)
} VPE_ENG_ID;

typedef enum{
    VPE_ENG_DES_YUV422_YUYV = 0,//reserved for YUV422
	VPE_ENG_DES_YUV422_YVYU = 1,//reserved
    VPE_ENG_DES_YUV420_SP   =2,//YUV420 SP
    VPE_ENG_DES_RESERVED_3 = 3,//reserved for SCE_YUV420 
    VPE_ENG_DES_RESERVED_4 = 4,//reserved for YUV420_Y_ONLY
    VPE_ENG_DES_RESERVED_5 = 5,//reserved for SCE_UIV420_Y_ONLY
	VPE_ENG_DES_YCC_YUV420 = 6,	//YCC_YUV420
	VPE_ENG_DES_YUV422_UYVY = 7,
	VPE_ENG_DES_YUV422_VYUY = 8,
    VPE_ENG_DES_MAX
}VPE_ENG_DES_FMT;

typedef enum{
    VPE_ENG_SCA_BILINEAR = 0,//reserved for YUV422
	VPE_ENG_SCA_RESERVED1 = 1,//reserved
    VPE_ENG_SCA_METHOD_MAX
}VPE_ENG_SCA_METHOD;


typedef enum{
    VPE_ENG_ROT_BYPASS  = 0,
    VPE_ENG_ROT_RESERVED_1  = 1, //reserved		
    VPE_ENG_ROT_RESERVED_2  = 2, //reserved for CW
    VPE_ENG_ROT_RESERVED_3  = 3, //reserved for CCW
	VPE_ENG_ROT_RESERVED_4	= 4, //reserved for V-FLIP
	VPE_ENG_ROT_RESERVED_5	= 5, //reserved for H-FLIP
	VPE_ENG_ROT_RESERVED_6	= 5, //reserved for V-H-FLIP
    VPE_ENG_ROT_MAX
}VPE_ENG_ROT;

typedef enum {
	VPE_ENG_DRT_BYPASS = 0,
	VPE_ENG_DRT_PC2TV = 2,
	VPE_ENG_DRT_TV2PC = 3,
} VPE_ENG_DRT;

typedef enum {
	VPE_ENG_FUNC_OFF = 0,
	VPE_ENG_FUNC_ON = 1,
} VPE_ENG_FUNC_EN;


typedef enum {
	VPE_MTN_MAP_Y = 0,
	VPE_MTN_MAP_CB,
	VPE_MTN_MAP_CR,
	VPE_MTN_MAP_FCS_CB,
	VPE_MTN_MAP_FCS_CR,
} VPE_MTN_MAP_CH;

typedef struct {
	UINT8 map_size;
	UINT8 pfet_en[VPE_ENG_COL_MAX];
	UINT16 pfet_hlen[VPE_ENG_COL_MAX];
	UINT16 pfet_vlen;
	UINT16 pfet_hstep;
	UINT16 pfet_vstep;
	UINT16 pfet_hoft;
	UINT16 pfet_voft;
} VPE_ENG_CACHE_CFG;

typedef struct {
	UINT16 cgain_y;
	INT16 cofs_y;
	UINT16 cgain_u;
	INT16 cofs_u;
	UINT16 cgain_v;
	INT16 cofs_v;
	UINT8 cg_drng;//0: full range, 1: tv range
} VPE_ENG_CGOFS_CFG;

typedef enum {
	VPE_ENG_DATA_CH_Y = 0,
	VPE_ENG_DATA_CH_UV = 1,
	VPE_ENG_DATA_CH_MAX
} VPE_ENG_DATA_CH;







typedef struct {
	UINT8 frm_done;
	//UINT8 md_tamper;
	//UINT8 md_scene_change;
	UINT8 dma_err;
} VPE_ENG_LL_STS;
typedef struct {
	UINT8 ycc_qthr_enc[8];
	UINT8 ycc_qtbl_idx_enc[16];

}VPE_YCC_QTBL;

extern VOID vpe_eng_set_pipe1_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 shp_out);
extern ER vpe_eng_set_pipe2_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 src_out, UINT32 sca_in);
extern VOID vpe_eng_set_src_path_sel_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 src_path_sel);
extern VOID vpe_eng_set_dctg_enable_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 dctg_en);
extern void vpe_eng_set_dctg_distor_enable_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 dctg_distor_en);
extern void vpe_eng_set_lut2d_column_offset_buf_reg(VPE_ENG_HANDLE *p_eng, UINT16 lut2d_col_st_int, UINT32 lut2d_col_st_frac);
extern VOID vpe_eng_set_dc_2d_lut_load_enable_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 dc_2d_lut_load_en);
extern VOID vpe_eng_set_privacy_mask_enable_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 mask_privacy_en);
extern VOID vpe_eng_set_sharpen_enable_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 shp_en);
extern ER vpe_eng_set_src_format_buf_reg(VPE_ENG_HANDLE *p_eng, VPE_ENG_SRC_FMT src_fmt);
extern VOID vpe_eng_set_src_drt_buf_reg(VPE_ENG_HANDLE *p_eng, VPE_ENG_DRT src_drt);
extern VOID vpe_eng_set_src_drt_weight_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 pc2tv_weight);
extern VOID vpe_eng_set_src_uv_swap_enable_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 src_uv_swap_en);
extern VOID vpe_eng_set_shp_tuning_out_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 shp_tuning_out);
extern VOID vpe_eng_set_global_src_size_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 bg_w, UINT32 bg_h);
extern VOID vpe_eng_set_global_proc_size_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 merge_w, UINT32 proc_h);
extern VOID vpe_eng_set_global_proc_ystart_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 proc_y);
extern VOID vpe_eng_set_global_dce_size_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 dce_w, UINT32 dce_h);
extern VOID vpe_eng_set_vcache_buf_reg(VPE_ENG_HANDLE *p_eng, VPE_ENG_CACHE_CFG *p_vcache);
extern ER vpe_eng_get_llsts_hw_reg(VPE_ENG_HANDLE *p_eng, UINT32 frm_idx, VPE_ENG_LL_STS *p_llsts);
extern VOID vpe_eng_set_ycc_qtbl_buf_reg(VPE_ENG_HANDLE *p_eng, VPE_YCC_QTBL ycc_qtbl);
extern ER vpe_eng_set_ptz_enable_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 ptz_en);
extern VOID vpe_eng_set_color_gain_ofs_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 cgofs_en,VPE_ENG_CGOFS_CFG cgofs_cfg);
extern ER vpe_eng_set_global_src_lineoft_buf_reg(VPE_ENG_HANDLE *p_eng, VPE_ENG_DATA_CH data_ch, UINT32 bg_lineoft);
extern VOID  vpe_eng_set_pfet_hlen_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 pfet_hlen);
extern VOID  vpe_eng_set_pfet_en_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 pfet_en);
extern INT32 vpe_eng_chk_cs_idle(VPE_ENG_HANDLE *p_eng);
extern VOID vpe_eng_set_absolute_dc_2d_lut_prec_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 absolute_prec_sel);
extern VOID vpe_eng_set_relative_dc_2d_lut_coord_enable_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 relative_2dlut_coord_en);
extern VOID vpe_eng_set_relative_dc_2d_lut_prec_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 relative_prec_sel);
extern ER vpe_eng_set_src_sideinfo_lineoft_buf_reg(VPE_ENG_HANDLE *p_eng, VPE_ENG_DATA_CH data_ch, UINT32 sideinfo_lineoft);
extern VOID  vpe_eng_set_pixel_mask_op_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 pixel_mask_op);
extern ER  vpe_eng_set_pixel_mask_seg_pos_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 seg_pos_id, UINT32 seg_pos);
extern ER  vpe_eng_set_pixel_mask_seg_out_en_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 seg_id, UINT32 seg_out_en);
extern ER  vpe_eng_set_pixel_mask_lineoft_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 seg_id, UINT32 seg_lineoft);
extern ER  vpe_eng_set_pixel_mask_out_start_buf_reg(VPE_ENG_HANDLE *p_eng, UINT32 seg_id, UINT16 seg_out_x, UINT16 seg_out_y);
extern void vpe_eng_set_pixel_mask_out_height_buf_reg(VPE_ENG_HANDLE *p_eng, UINT16 seg_out_h);

#ifdef __cplusplus
}
#endif


#endif //

