#ifndef _DRE_DRV_FUSION_INT_H_
#define _DRE_DRV_FUSION_INT_H_

#include "dre_drv_ll_int.h"
#include "kdrv_dre_ctl.h"
#include "dre_eng.h"

enum dre_drv_fusion_type {
    DRE_DRV_DCM = 0,    //decomposition
    DRE_DRV_RCS,  	  	//reconstruction
    DRE_DRV_PATCH,        //image patching
    DRE_DRV_FUSION_TYPE_MAX,
};

struct dre_drv_dcm_layer_cfg {
	UINT8 mode; // 0~4
    struct dre_drv_roi L0_size;
    struct dre_drv_roi L1_size;
    struct dre_drv_dram in_img1[2]; // 0:Y, 1:UV
    struct dre_drv_dram in_img2[2]; // 0:Y, 1:UV
    struct dre_drv_dram lap_img;
    struct dre_drv_dram gau_img[2]; // 0:gau[0], 1:gau[1]
    struct dre_drv_dram fusion_wt_map_in;
    struct dre_drv_dram fusion_wt_map_out;
	UINT8 status; // 0: normal, 1: the last hw layer, 2:sw layer
};

struct dre_drv_rcs_layer_cfg {
	UINT8 mode; // 0~4
    struct dre_drv_roi L0_size;
    struct dre_drv_roi L1_size;
    struct dre_drv_dram L0_img; // YUV444 diff
    struct dre_drv_dram L1_img; // 1/4 YUV444 diff
    struct dre_drv_dram out_img[2]; // 0: y or yuv, 1:uv
	UINT8 status; // 0: normal, 1: the last hw layer
};

struct dre_drv_patch_cfg {
	UINT8 enable;
    struct dre_drv_roi img_size;
    struct dre_drv_dram in_img1[2]; // 0:Y, 1:UV
    struct dre_drv_dram in_img2[2]; // 0:Y, 1:UV
    struct dre_drv_dram mask[2]; // 0: img1's mask, 1:img2's mask
    struct dre_drv_dram out_img1[2]; // 0:Y, 1:UV
    struct dre_drv_dram out_img2[2]; // 0:Y, 1:UV
    struct dre_drv_patch_color color;    
};

struct dre_drv_fusion_layer {
    UINT8 img_num;
    UINT8 dcm_layer_num;
    UINT8 user_layer_max;
    UINT8 use_sw_proc; // 0:no, 1:yes
    UINT8 wt_src_sel; // 0:dram, 1:lut
    struct dre_drv_dcm_layer_cfg dcm_layer[MAX_LAYER];
    struct dre_drv_rcs_layer_cfg rcs_layer[MAX_LAYER];
    struct dre_drv_patch_cfg patch_img;
};

int dre_drv_fusion_proc(DRE_ENG_HANDLE *p_eng_hdl, struct dre_drv_job_cfg *job_cfg, struct dre_drv_fusion_layer *layer_cfg);
int dre_drv_nr_proc(DRE_ENG_HANDLE *p_eng_hdl, struct dre_drv_job_cfg *job_cfg, struct dre_drv_fusion_layer *layer_cfg);
void dre_drv_fusion_dump_cfg_info(struct dre_drv_fusion_cfg *fusion);
void dre_drv_nr_dump_cfg_info(struct dre_drv_nr_cfg *nr);
void dre_drv_fusion_dump_iq_info(struct dre_drv_iq_cfg *iq_cfg);
void dre_drv_fusion_dump_layer_info(struct dre_drv_fusion_layer *layer);
int dre_drv_fusion_set_layer(DRE_ENG_HANDLE *p_eng_hdl, struct dre_drv_fusion_layer *layer_cfg, UINT8 layer_idx, UINT8 rcs_flow);
int dre_drv_set_patching_image(DRE_ENG_HANDLE *p_eng_hdl, struct dre_drv_fusion_layer *layer_cfg);

#endif //_DRE_DRV_FUSION_INT_H_
