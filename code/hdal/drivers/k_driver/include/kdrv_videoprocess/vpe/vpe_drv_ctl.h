#ifndef _VPE_DRV_CTL_H_
#define _VPE_DRV_CTL_H_

#include <kwrap/nvt_type.h>
#include <kwrap/list.h>
#include <kwrap/util.h>
#include <kwrap/spinlock.h>

/* job status */
enum vpe_drv_sts {
    VPE_DRV_STS_IDLE = 0,
    VPE_DRV_STS_QUEUE,     // not process yet, in job list
    VPE_DRV_STS_PROC,		// ready to process
    VPE_DRV_STS_PROC_END,	// process done
    VPE_DRV_STS_DONE,		// done by hw process
    VPE_DRV_STS_FLUSH,     // stop by driver(not done)
    VPE_DRV_STS_ERROR,		// error by driver(not done)
    VPE_DRV_STS_OK,
    VPE_DRV_STS_QFULL,
    VPE_DRV_STS_MAX
};

struct vpe_drv_roi {
	UINT16 x;
	UINT16 y;
	UINT16 w;
	UINT16 h;
};

enum vpe_drv_fmt {
    VPE_DRV_YUV420_SP = 0,  //in: vpe 	   out: vpe
    VPE_DRV_YUV420_YCC,		//in: vpe 	   out: vpe,yuv compress format
    VPE_DRV_YUV422_YUYV, 	//in:          out: vpe
    VPE_DRV_YUV422_YVYU, 	//in:          out: vpe
    VPE_DRV_YUV422_UYVY, 	//in:          out: vpe
    VPE_DRV_YUV422_VYUY, 	//in:          out: vpe
    VPE_DRV_UNKNOW_FMT,
    VPE_DRV_FMT_MAX,
};

enum vpe_drv_drt {
	VPE_DRV_DRT_NONE = 0,
	VPE_DRV_DRT_PC2TV,
	VPE_DRV_DRT_TV2PC,
	VPE_DRV_DRT_MAX,
};

enum vpe_drv_scale_method_sel {
	VPE_DRV_SCALE_METHOD_BILINEAR,	//vpe: res0/1
	VPE_DRV_SCALE_METHOD_MAX
};

struct vpe_drv_in_info {
	UINT8 ddr_id;
	uintptr_t y_addr;
	uintptr_t uv_addr;

	enum vpe_drv_fmt fmt;
	struct vpe_drv_roi bg;
	struct vpe_drv_roi crop;
    enum vpe_drv_drt drt;
	UINT16 drt_pc2tv_weight; //0~256, 0:org (538:not support, 539A:support)

	UINT32 y_lofs;
	UINT32 uv_lofs;
};

enum vpe_drv_seg_op {
	VPE_DRV_SEG_OFF,
	VPE_DRV_SEG_LEFT_RIGHT,
	VPE_DRV_SEG_UP_DOWN,
	VPE_DRV_SEG_OP_MAX
};

#define VPE_DRV_SEG_NUM 2
#define VPE_DRV_SEG_POS_NUM 2
struct vpe_drv_seg_info {
	enum vpe_drv_seg_op op;

	UINT8 ddr_id[VPE_DRV_SEG_NUM];		//0: seg1, 1:seg2, seg0 ref path info(538:support idx0, 539A:support idx0/1)
	uintptr_t y_addr[VPE_DRV_SEG_NUM];	//0: seg1, 1:seg2, seg0 ref path info(538:support idx0, 539A:support idx0/1)
	uintptr_t uv_addr[VPE_DRV_SEG_NUM];	//0: seg1, 1:seg2, seg0 ref path info(538:support idx0, 539A:support idx0/1)
	UINT32 y_lofs[VPE_DRV_SEG_NUM];		//0: seg1, 1:seg2, seg0 ref path info(538:support idx0, 539A:support idx0/1)
	UINT32 uv_lofs[VPE_DRV_SEG_NUM];	//0: seg1, 1:seg2, seg0 ref path info(538:support idx0, 539A:support idx0/1)

	UINT32 pos[VPE_DRV_SEG_POS_NUM];	//0 <= pos[0] < pos[1] <= win width or height(538:support idx0, 539A:support idx0/1)

	UINT8 out_en; 	//538: fixed 0x3
	 				//539A: bit[0]:seg0, bit[1]:seg1, bit[2]:seg2
};

struct vpe_drv_out_info {
	UINT8 en;
	UINT8 ddr_id;
	uintptr_t y_addr;
	uintptr_t uv_addr;
	enum vpe_drv_fmt fmt;
	struct vpe_drv_roi sca_in_crop;  //w,h = 0 means disable
	struct vpe_drv_roi sca; 		 //w,h = 0 means disable, x, y is no used
	struct vpe_drv_roi sca_out_crop; //w,h = 0 means disable(tc crop)
	struct vpe_drv_roi bg;
	struct vpe_drv_roi win;
	struct vpe_drv_roi rlt;
	struct vpe_drv_roi pip; 		 //w,h = 0 means disable
    UINT8 out_bg_sel;				 //palette index
    enum vpe_drv_drt drt;
	UINT16 drt_pc2tv_weight; //0~256, 0:org (538:not support, 539A:support)

	UINT32 y_lofs;
	UINT32 uv_lofs;

	struct vpe_drv_seg_info seg;
};

#define VPE_DRV_SHARPEN_DBS_GAIN_NUMS 17
#define VPE_DRV_SHAPREN_NOISE_CURVE_NUMS 17
struct vpe_drv_shpn_info {
    UINT8 sharpen_en;
    UINT8 sharpen_out_sel;
    UINT8 edge_weight_src_sel; // Select source of edge weight calculation, 0~1
    UINT8 edge_weight_th; // Edge weight coring threshold, 0~255
    UINT8 edge_weight_gain; // Edge weight gain, 0~255
    UINT8 noise_level; // Noise Level, 0~255
    UINT8 noise_curve[VPE_DRV_SHAPREN_NOISE_CURVE_NUMS]; // 17 control points of noise modulation curve, 0~255
    UINT8 blend_inv_gamma; // Blending ratio of HPF results, 0~128
    UINT8 edge_sharp_str1; // Sharpen strength1 of edge region, 0~255
    UINT8 edge_sharp_str2; // Sharpen strength2 of edge region, 0~255
    UINT8 flat_sharp_str; // Sharpen strength of flat region,0~255
    UINT8 coring_th; // Coring threshold, 0~255
    UINT8 bright_halo_clip; // Bright halo clip ratio, 0~255
    UINT8 dark_halo_clip; // Dark halo clip ratio, 0~255

	UINT8 dbs_gain_en;	   // only valid at dce_en = 1
	UINT8 dbs_gain[VPE_DRV_SHARPEN_DBS_GAIN_NUMS]; // 17 control points of noise modulation curve, 0~255(u3.5)
	UINT32 quad_area_clamping; //17 bits (u1.16)
};

struct vpe_drv_pal_color{
	UINT8 pal_y;
	UINT8 pal_cb;
	UINT8 pal_cr;
};

struct vpe_drv_dce_fov {
    UINT8 fovbound;		//538:0/1, 539A:0/1/2
    UINT16 boundy;
    UINT16 boundu;
    UINT16 boundv;
};

enum vpe_drv_dce_mode {
	VPE_DRV_DCE_2D_ONLY = 0x1,
	VPE_DRV_DCE_MODE_MAX
};

enum vpe_drv_dce_rot {
	VPE_DRV_DCE_ROT_NONE = 0,
	VPE_DRV_DCE_ROT_90,
	VPE_DRV_DCE_ROT_180,
	VPE_DRV_DCE_ROT_270,
	VPE_DRV_DCE_ROT_0_H_FLIP,
	VPE_DRV_DCE_ROT_90_H_FLIP,
	VPE_DRV_DCE_ROT_180_H_FLIP,
	VPE_DRV_DCE_ROT_270_H_FLIP,
	VPE_DRV_DCE_ROT_MANUAL,
	VPE_DRV_DCE_ROT_MAX
};

enum vpe_drv_dce_rot_ratio {
	VPE_DRV_DCE_ROT_RAT_FIX_ASPECT_RATIO_UP,
	VPE_DRV_DCE_ROT_RAT_FIX_ASPECT_RATIO_DOWN,
	VPE_DRV_DCE_ROT_RAT_FIT_ROI,
	VPE_DRV_DCE_ROT_RAT_MANUAL,
	VPE_DRV_DCE_ROT_RAT_MAX
};

#define VPE_DRV_DCE_ROT_RAD_MIN 0
#define VPE_DRV_DCE_ROT_RAD_MAX 411774
struct vpe_drv_dce_rot_param {
	UINT32 rad; //0 ~ VPE_DRV_DCE_ROT_RAD_MAX, rad = degree * 205887 / 180
	UINT32 flip; //0:disable, 1:h_flip, 2:v_flip, 3:hv_flip
	enum vpe_drv_dce_rot_ratio ratio_mode;
	UINT16 ratio; //100:1x 1 ~ max(depend image size & rot deg)
};

#define VPE_DRV_PIX_MASK_SEG_NUM 3
#define VPE_DRV_PIX_MASK_SEG_POS_NUM 2
struct vpe_drv_dce_pix_mask_param {
	enum vpe_drv_seg_op op;

	UINT8 ddr_id[VPE_DRV_PIX_MASK_SEG_NUM];
	uintptr_t addr[VPE_DRV_PIX_MASK_SEG_NUM];
	UINT32 lofs[VPE_DRV_PIX_MASK_SEG_NUM];

	UINT32 pos[VPE_DRV_PIX_MASK_SEG_POS_NUM]; //0 <= pos[0] < pos[1] <= dce out width

	UINT8 out_en; //bit[0]:seg0, bit[1]:seg1, bit[2]:seg2
};


#define VPE_DRV_GEO_LUT_NUMS 65
struct vpe_drv_dce_param {
    UINT8 dce_en;
    enum vpe_drv_dce_mode dce_mode;
    UINT16 lut2d_width; //2~1024
    UINT16 lut2d_height; //2~1024
    UINT16 lut2d_lofs; //lut2d_width align 4
    UINT8 lsb_rand;
    UINT16 lens_r;
    struct vpe_drv_dce_fov fov_setting;
    UINT8 xofs_i;
    UINT32 xofs_f;
    UINT8 yofs_i;
    UINT32 yofs_f;
	UINT16 geo_lut[VPE_DRV_GEO_LUT_NUMS];
	UINT8 dce_2d_lut_en;		//0:lut tab from dctg, 1:lut tab from dce_2d_addr
	UINT8 dce_2d_precision;		//0:abs(s14.2), 1:abs(s15.1), 2:abs(s16.0), 3:rela(s2.14), 4:rela(s3.13), 5:rela(s4.12), 6:rela(s5.11)
	uintptr_t dce_2d_addr;
    UINT8 dce_2d_ddr_id;

	struct vpe_drv_roi out; //dce out size, only for VPE_DRV_DCE_2D_ONLY mode & dce_2d_lut_en = 1
							//w,h = 0 means w,h = src_in, x, y is no used

 	enum vpe_drv_dce_rot rot; //only valid at dce_mode = VPE_DRV_DCE_2D_ONLY & dce_2d_lut_en = 0
 	struct vpe_drv_dce_rot_param rot_param; //only valid at VPE_DRV_DCE_ROT_MANUAL

	struct vpe_drv_dce_pix_mask_param pix_mask; //538:not support, 539A:support
};

enum vpe_drv_ptz_proj_type {
	VPE_DRV_PTZ_PROJ_EQUIRECTANGULAR = 0,
	VPE_DRV_PTZ_PROJ_CYLINDRICAL,
	VPE_DRV_PTZ_PROJ_MAX
};

struct vpe_drv_ptz_param {
    UINT8 ptz_en;
	enum vpe_drv_ptz_proj_type proj_type;
    UINT32 long_aov;		//0~pi pi=205887
    UINT32 lati_aov;		//0~pi pi=205887
    INT32 pan_angle;   		//-pi~pi pi=205887
    INT32 tilt_angle;   	//-pi~pi pi=205887
    INT32 rot_angle;   		//-pi~pi pi=205887
    UINT16 zoom_step; 		//0~1023, 128 segments each 2x, 1x:0, 2x:128, 4x:256
    UINT32 cam_long_aov;	//0~pi pi=205887
    UINT32 cam_lati_aov;	//0~pi pi=205887
    UINT16 cam_w;			//single camera image width in a stitch image
	UINT16 cam_h;			//single camera image height in a stitch image
    INT32 stitch_overlap_angle;   //-pi~pi pi=205887
} ;


struct vpe_drv_dctg_param {
    UINT8 dctg_en;
    UINT8 mount_type; // 0:celiling 1:floor 2:wall
    UINT16 lens_r;
    UINT16 lens_cent_x;
    UINT16 lens_cent_y;
    UINT32 long_aov;		//0~pi pi=205887
    UINT32 lati_aov;		//0~pi pi=205887
    INT32 pan_angle;   		//-pi~pi pi=205887
    INT32 tilt_angle;   	//-pi~pi pi=205887
    INT32 rot_angle;   		//-pi~pi pi=205887
    UINT8 adj_v_perspect;	//0:disable, 1:manual 2:auto
    INT32 adj_v_angle;		//s5.16, -tan(80)*65536 ~ tan(80)*65536 (only valid at adj_v_perspect = manual mode)

	UINT8 distor_lut_en;
	UINT32 max_diag_dist;   //2^17*180/(max_diag_dist*pi) def. max_diag_dist=90
	UINT32 max_inci_angle;
	UINT8 adj_fov_aspect_ratio_en; //538:not support, 539A:support

	struct vpe_drv_ptz_param ptz;
} ;

#define VPE_DRV_COL_NUM_PER_JOB 1

struct vpe_drv_color_ofs {
	UINT8 cgofs_en;
	UINT16 y_gain;		//1x = 64 max:7.99x val: 0 ~ 511
	INT16 y_ofs;		//signed s9bit -511 ~ 512
	UINT16 u_gain;		//1x = 64 max:7.99x val: 0 ~ 511
	INT16 u_ofs;		//signed s9bit -511 ~ 512
	UINT16 v_gain;		//1x = 64 max:7.99x val: 0 ~ 511
	INT16 v_ofs;		//signed s9bit -511 ~ 512
	UINT8 is_tv_range;  //if in drt = VPE_DRV_DRT_NONE, data range must be set(base on dram data range)
};

enum vpe_drv_mask_shape {
	VPE_DRV_MASK_SHAPE_4 = 0,
	VPE_DRV_MASK_SHAPE_6,
	VPE_DRV_MASK_SHAPE_8,
	VPE_DRV_MASK_SHAPE_10,
	VPE_DRV_MASK_SHAPE_MAX
};

#define VPE_DRV_MASK_PT_IGNORE 0x7fffffff
struct vpe_drv_mask_pt {
    INT32 x;
    INT32 y;
};

#define VPE_DRV_MASK_PT_MAX 10
struct vpe_drv_mask_win {
	UINT8 en;				//0:disable, 1:enable
    UINT8 mosaic_en;		//0:original 1:mosaic
    UINT8 pal_sel;			//0~7. ref palette[VPE_DRV_PALETTE_MAX]
    UINT16 alpha;			//alpha 0 ~ 256, 0: 100%, 256:0%
    struct vpe_drv_mask_pt roi_pt[VPE_DRV_MASK_PT_MAX]; //roi_pt[4-9] valid at shape_6/8/10
	UINT8 area;				//0:inside, 1:outside, 2:border(border only valid at VPE_DRV_MASK_SHAPE_4)
	enum vpe_drv_mask_shape shape; //valid at win idx[0/2/4/6]
};

#define VPE_DRV_MASK_WIN_MAX 8
struct vpe_drv_mask_info {
	UINT8 mosaic_blk_sz;			//0:8x8, 1:16x16, 2:32x32, 3:64x64
    struct vpe_drv_mask_win win[VPE_DRV_MASK_WIN_MAX]; //win idx[0-7] support shape_4
    												   //win idx[0/2/4/6] support shape_6/8/10,
    												   //when win[idx] set shape_6/shape_8, win[idx+1] must be disable
};

#define VPE_DRV_OUT_MAX 4
#define VPE_DRV_PALETTE_MAX 8
struct vpe_drv_job_cfg {

    /******* add configs at here *******/
	struct vpe_drv_in_info in;
	struct vpe_drv_out_info out[VPE_DRV_OUT_MAX];
	struct vpe_drv_color_ofs cgain_param;
	struct vpe_drv_shpn_info sharpen_param;
	struct vpe_drv_pal_color palette[VPE_DRV_PALETTE_MAX];
    struct vpe_drv_dce_param dce_param;
    struct vpe_drv_dctg_param dctg_param;
	struct vpe_drv_mask_info mask_param;
    /******* add configs at here *******/


	INT32 (*flow_cb)(struct vpe_drv_job_cfg *job_cfg, void *p1, void *p2);

	UINT32 trig_mode; //0: normal mode, 1: depend on prev job trig ch (invalid, always fixed 0)
	UINT32 eng_col_mode; //0: disable, 1: enable (invalid, always fixed 0)

    enum vpe_drv_sts sts; //job ststus
    struct vos_list_head list; //job link list
};

struct vpe_drv_job_head {

	UINT32 chip;
	UINT32 eng;
	UINT32 id;

    UINT16 num; //total job number

    struct vos_list_head job_list_root; //job list

    void (*callback)(struct vpe_drv_job_head *head); //job callback to notify job finish
};

extern enum vpe_drv_sts vpe_drv_ctl_put_job(struct vpe_drv_job_head *head);
extern int vpe_drv_eng_get_chip_num(void);
extern int vpe_drv_eng_get_eng_num(void);


struct vpe_drv_src_align_info {
	enum vpe_drv_fmt fmt;
	struct vpe_drv_roi bg;
	struct vpe_drv_roi crop;
	UINT16 lofs[2]; //y, uv
};

struct vpe_drv_dst_align_info {
	enum vpe_drv_fmt fmt;
	struct vpe_drv_roi bg;
	struct vpe_drv_roi dest;
	struct vpe_drv_roi rlt;
	struct vpe_drv_roi hole;
	UINT16 lofs[2]; //y, uv
	UINT8 seg_pos[4]; //x, y, min diff x, min diff y
};

extern const struct vpe_drv_src_align_info* vpe_drv_sca_get_src_align_tab(int *tab_num);
extern const struct vpe_drv_dst_align_info* vpe_drv_sca_get_dst_align_tab(int *tab_num);
extern int vpe_drv_ctl_chk_eng_idle(int chip_id);

#define KFLOW_VPE 0x50
#define KDRV_VPE 0xA0
extern void vpe_drv_ctl_log(unsigned char e0, unsigned char e1, unsigned char e2, unsigned char e3, unsigned int p0, unsigned int p1);
/******************************************************************************************/
/*******                                kdrv interface                              *******/
/******************************************************************************************/
struct vpe_drv_out_param {
	UINT32 id; // 0 ~ (VPE_DRV_OUT_MAX - 1)
	struct vpe_drv_out_info info;
};


struct vpe_drv_trig_sts {
	UINT32 ser_num;	//job id
	UINT32 sts;		//0: not in queue, 1: in queue
};

typedef VOID (*vpe_drv_cb)(UINT32 trig_id, UINT32 p0, UINT32 p1);

#define KDRV_VPE_PARAM_BASE 0x12340000
enum kdrv_vpe_param_id {
	KDRV_VPE_PARAM_QUEUE_MAX_NUM = KDRV_VPE_PARAM_BASE,		//[   /get] data type UINT32
	KDRV_VPE_PARAM_BLK_MODE,								//[set/get] data type UINT32(0:disable(def), 1:enable)
	KDRV_VPE_PARAM_MAX
};
#define KDRV_VPE_PARAM_ID_MAX (KDRV_VPE_PARAM_MAX - KDRV_VPE_PARAM_BASE)

/************************* API *************************/
/**
     vpe drv init

     @return Description of data returned.
         - @b >= 0: pass
         - @b <  0: fail
*/
INT32 kdrv_vpe_rtos_init(void);

/**
     vpe drv uninit

     @return Description of data returned.
         - @b >= 0: pass
         - @b <  0: fail
*/
INT32 kdrv_vpe_rtos_uninit(void);
/**
     allocate vpe hw resource

     @param[in] chip designed chip index
     @param[in] eng  designed eng index

     @return Description of data returned.
         - @b >= 0: allocte resouce success, return hw resouce handle
         - @b <  0: allocate fail
*/
ULONG kdrv_vpe_open(UINT32 chip, UINT32 eng);

/**
     free vpe hw resource

     @param[in] hw resouce handle

     @return Description of data returned.
         - @b >= 0: free success
         - @b <  0: free fail
*/
INT32 kdrv_vpe_close(ULONG handle);

/**
     get information

     @param[in] handler hw resouce handle
     @param[in] param_id enum kdrv_vpe_param_id
     @param[in,out] p_param parameter, data type ref. enum kdrv_vpe_param_id

     @return Description of data returned.
         - @b >= 0: success
         - @b <  0: fail
*/
INT32 kdrv_vpe_get(ULONG handle, enum kdrv_vpe_param_id param_id, void *p_param);

/**
     set information

     @param[in] handler hw resouce handle
     @param[in] param_id enum kdrv_vpe_param_id
     @param[in,out] p_param parameter, data type ref. enum kdrv_vpe_param_id

     @return Description of data returned.
         - @b >= 0: success
         - @b <  0: fail
*/
INT32 kdrv_vpe_set(ULONG handle, enum kdrv_vpe_param_id param_id, void *p_param);

/**
     trigger hw engine

     @param[in] id hw resouce handle
     @param[in] job information

     @return Description of data returned.
         - @b = VPE_DRV_STS_OK: success
*/
enum vpe_drv_sts kdrv_vpe_trigger(ULONG handle, struct vpe_drv_job_head *head);
#endif  /* _VPE_DRV_CTL_H_ */

