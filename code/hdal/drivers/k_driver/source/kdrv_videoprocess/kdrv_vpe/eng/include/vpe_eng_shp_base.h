#ifndef _VPE_ENG_SHP_BASE_H_
#define _VPE_ENG_SHP_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "vpe_eng_handle.h"

typedef struct {
    UINT8 src_sel;
    UINT8 dbs_gain_en;	
    UINT8 threshold;
    UINT8 gain;
} VPE_SHPEN_EDGE_WEIGHT_INFO;


typedef struct {
    UINT8 fa;
    UINT8 fb;
    UINT8 fc;
} VPE_ES_DIRECT_SMTH_COEF;


extern VOID vpe_eng_set_shp_edge_weight_info_buf_reg(VPE_ENG_HANDLE *p_eng, VPE_SHPEN_EDGE_WEIGHT_INFO shpn_weight_info);
extern VOID vpe_eng_set_shp_noise_level_buf_reg(VPE_ENG_HANDLE *p_eng, UINT8 shpn_noise_lvl);
extern VOID vpe_eng_set_shp_edge_strength_buf_reg(VPE_ENG_HANDLE *p_eng, UINT8 edge_sharp_str1,  UINT8 edge_sharp_str2);
extern VOID vpe_eng_set_shp_flat_strength_buf_reg(VPE_ENG_HANDLE *p_eng, UINT8 flat_sharp_str);
extern VOID vpe_eng_set_shp_coring_threshold_buf_reg(VPE_ENG_HANDLE *p_eng, UINT8  coring_th);
extern VOID vpe_eng_set_shp_blend_ratio_buf_reg(VPE_ENG_HANDLE *p_eng, UINT8  blend_ratio);
extern VOID vpe_eng_set_shp_bright_halo_clip_buf_reg(VPE_ENG_HANDLE *p_eng, UINT8  bright_halo_clip);
extern VOID vpe_eng_set_shp_dark_halo_clip_buf_reg(VPE_ENG_HANDLE *p_eng, UINT8  dark_halo_clip);
extern VOID vpe_eng_set_shp_noise_curve_buf_reg(VPE_ENG_HANDLE *p_eng, UINT8 *noise_curve);
#ifdef __cplusplus
}
#endif


#endif //

