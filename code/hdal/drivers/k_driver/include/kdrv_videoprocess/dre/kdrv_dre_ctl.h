#ifndef _DRE_DRV_CTL_H_
#define _DRE_DRV_CTL_H_

#include <kwrap/nvt_type.h>
#include <kwrap/list.h>
#include <kwrap/util.h>
#include <kwrap/spinlock.h>

#define DRE_DRV_YCMOD_LUT_NUM       16
#define DRE_DRV_MSNR_LUN_NUM        128
#define DRE_DRV_JOINT_CH_NUM        3
#define DRE_DRV_S_WEIGHT_NUM        6
#define DRE_DRV_RTH_LUT_NUM         8
#define DRE_DRV_SPA_FILT_NUM        3
#define DRE_DRV_WT_LUT_NUM          17

#define MAX_LAYER 10

/* job status */
enum dre_drv_sts {
    DRE_DRV_STS_IDLE = 0,
    DRE_DRV_STS_QUEUE,      // not process yet, in job list
    DRE_DRV_STS_PROC,		// ready to process
    DRE_DRV_STS_PROC_END,	// process done
    DRE_DRV_STS_DONE,		// done by hw process
    DRE_DRV_STS_FLUSH,      // stop by driver(not done)
    DRE_DRV_STS_ERROR,		// error by driver(not done)
    DRE_DRV_STS_OK,
    DRE_DRV_STS_QFULL,		// queue full
    DRE_DRV_STS_SW_PROC,	// sw process
    DRE_DRV_STS_SW_PROC_END,// sw process end
    DRE_DRV_STS_MAX
};

enum dre_drv_fmt {
    DRE_DRV_YUV420_SP = 0,    //Y + UV pack
    DRE_DRV_YUV422_SP, 	  	  //Y + UV pack
    DRE_DRV_FMT_MAX,
};

enum dre_drv_func {
    DRE_DRV_FUNC_FUSION = 0,  
    DRE_DRV_FUNC_NR,
    DRE_DRV_FUNC_MAX,
};

struct dre_drv_addr {
	uintptr_t va;
	uintptr_t pa;
    UINT8 ddr_id;
};

struct dre_drv_dram {
	struct dre_drv_addr addr;
	UINT32 lofs; // should be 4-bytes align
};

struct dre_drv_working_buf {
	struct dre_drv_addr addr;
	UINT32 size;
};

struct dre_drv_roi {
	UINT32 w;
	UINT32 h;
};

struct dre_drv_patch_color {
	UINT8 y;
	UINT8 u;
	UINT8 v;
};

struct dre_drv_fusion_cfg {
    struct dre_drv_roi img_size;
    enum dre_drv_fmt fmt;
    struct dre_drv_dram in_img1[2]; // 0:Y, 1:UV
    struct dre_drv_dram in_img2[2];
    struct dre_drv_dram out_img[2];
    struct dre_drv_dram fusion_wt_map_in[MAX_LAYER];
    UINT8 fusion_wt_tbl[DRE_DRV_WT_LUT_NUM];
    UINT8 fusion_wt_sel; // 0:map, 1:tbl[17]
    UINT8 reserve_src_buf; //0: use the src buffer to be the working buffer, 1: do not overwrite the src buffer
    UINT8 user_max_layer; // 4~10, control the  max layer number by user 
    struct dre_drv_working_buf working_buf;
    struct dre_drv_dram pixel_mask[2]; // for 539a only 0:img1, 1:img2, enable patching mode while both 2 pa != 0
    struct dre_drv_patch_color patch_color; // for 539a only
    struct dre_drv_dram patch_out_img1[2]; // for 539a only, if addr == 0, patching image wll output to src1 addr
    struct dre_drv_dram patch_out_img2[2]; // for 539a only, if addr == 0, patching image wll output to src2 addr
};

struct dre_drv_nr_cfg {
    struct dre_drv_roi img_size;
    enum dre_drv_fmt fmt;
    struct dre_drv_dram in_img[2]; // 0:Y, 1:UV
    struct dre_drv_dram out_img[2];
    UINT8 reserve_src_buf; //0: use the src buffer to be the working buffer, 1: do not overwrite the src buffer
    UINT8 user_max_layer; // 4~10, control the  max layer number by user 
    struct dre_drv_working_buf working_buf;
};

struct dre_drv_func_cfg {
    enum dre_drv_func func;
    union {
        struct dre_drv_fusion_cfg fusion_cfg;
        struct dre_drv_nr_cfg nr_cfg;
    };
};


typedef struct{
	UINT8 img0_spa_coeff[DRE_DRV_SPA_FILT_NUM]; // 3x3, [0] is center, [1] is cross, [2] is diagonal
	UINT8 img1_spa_coeff[DRE_DRV_SPA_FILT_NUM];
}DRE_DRV_FUSION_FILT;

typedef struct{
	BOOL joint_y[DRE_DRV_JOINT_CH_NUM]; // NR Joint Bilateral 
	BOOL joint_u[DRE_DRV_JOINT_CH_NUM];
	BOOL joint_v[DRE_DRV_JOINT_CH_NUM];
	BOOL outl_en[DRE_DRV_JOINT_CH_NUM];
}DRE_DRV_NR_JOINT_OUTL;

typedef struct{
	UINT8 s_weight[DRE_DRV_S_WEIGHT_NUM]; //NR 5x5 Spatial Filter
	UINT8 rth_y_lut[DRE_DRV_RTH_LUT_NUM]; //NR Range Threshold LUT
	UINT8 rth_u_lut[DRE_DRV_RTH_LUT_NUM];
	UINT8 rth_v_lut[DRE_DRV_RTH_LUT_NUM];
}DRE_DRV_NR_FILT;

typedef struct{
	DRE_DRV_NR_JOINT_OUTL nr_joint_outl;
	DRE_DRV_NR_FILT       nr_filter;
}DRE_DRV_NR_PARAM;

typedef enum{
	KDRV_DRE_1_STEP = 0,
	KDRV_DRE_2_STEP = 1,
}DRE_DRV_YCMOD_STEP;

typedef struct{
	BOOL en;
	UINT8 sel;
	UINT8 lutsel;
	UINT8 cbofs;
	UINT8 crofs;
	DRE_DRV_YCMOD_STEP stepy;
	DRE_DRV_YCMOD_STEP stepc;
	UINT8 y_lut[DRE_DRV_YCMOD_LUT_NUM]; // data range: 0~16
	UINT8 c_lut[DRE_DRV_YCMOD_LUT_NUM]; // data range: 0~16
}DRE_DRV_YCMOD_PARAM;

typedef struct{
	BOOL lut_en[3]; // 0:y, 1:u, 2:v
	UINT8 msnr_lut[3][DRE_DRV_MSNR_LUN_NUM]; // 0:y, 1:u, 2:v
}DRE_DRV_MSNR_PARAM;

struct dre_drv_iq_cfg {
    BOOL dcm_fliter_en;
	DRE_DRV_MSNR_PARAM    msnr;
	DRE_DRV_YCMOD_PARAM   ycmod;
	DRE_DRV_NR_PARAM      nr_param;
    DRE_DRV_FUSION_FILT   spa_param; // Spatial Filter
};


struct dre_drv_job_cfg {

    /******* add configs at here *******/
    //struct dre_drv_fusion_cfg fusion_cfg;
    struct dre_drv_func_cfg func_cfg;
    struct dre_drv_iq_cfg iq_cfg;
    /******* add configs at here *******/

    enum dre_drv_sts sts; //job ststus
    struct vos_list_head list; //job link list
};

#define KDRV_DRE_MODE struct dre_drv_job_cfg

struct dre_drv_job_head {

	UINT32 chip;
	UINT32 eng;
	UINT32 id;

    UINT16 num; //total job number

    struct vos_list_head job_list_root; //job list

    void (*callback)(struct dre_drv_job_head *head); //job callback to notify job finish
};

extern enum dre_drv_sts dre_drv_ctl_put_job(struct dre_drv_job_head *head);
extern int dre_drv_eng_get_chip_num(void);
extern int dre_drv_eng_get_eng_num(void);

#define KDRV_DRE 0xA0
extern void dre_drv_ctl_log(unsigned char e0, unsigned char e1, unsigned char e2, unsigned char e3, unsigned int p0, unsigned int p1);
/******************************************************************************************/
/*******                                kdrv interface                              *******/
/******************************************************************************************/
typedef VOID (*dre_drv_cb)(UINT32 trig_id, UINT32 p0, UINT32 p1);

#define KDRV_DRE_PARAM_BASE 0x12340000
enum kdrv_dre_param_id {
	KDRV_DRE_PARAM_QUEUE_MAX_NUM = KDRV_DRE_PARAM_BASE,		//[   /get] data type UINT32
	KDRV_DRE_PARAM_DMA_ABORT,								//[set/   ] no parameter
	KDRV_DRE_PARAM_BLK_MODE,								//[set/get] data type UINT32(0:disable(def), 1:enable)
	KDRV_DRE_PARAM_PRI,										//[set/get] data type UINT32(0(high) ~ 2(low), 1(def))
	KDRV_DRE_PARAM_MAX
};
#define KDRV_DRE_PARAM_ID_MAX (KDRV_DRE_PARAM_MAX - KDRV_DRE_PARAM_BASE)

/************************* API *************************/
/**
     dre drv init

     @return Description of data returned.
         - @b >= 0: pass
         - @b <  0: fail
*/
INT32 kdrv_dre_rtos_init(void);

/**
     dre drv uninit

     @return Description of data returned.
         - @b >= 0: pass
         - @b <  0: fail
*/
INT32 kdrv_dre_rtos_uninit(void);
/**
     allocate dre hw resource

     @param[in] chip designed chip index
     @param[in] eng  designed eng index

     @return Description of data returned.
         - @b >= 0: allocte resouce success, return hw resouce handle
         - @b <  0: allocate fail
*/
ULONG kdrv_dre_open(UINT32 chip, UINT32 eng);

/**
     free dre hw resource

     @param[in] hw resouce handle

     @return Description of data returned.
         - @b >= 0: free success
         - @b <  0: free fail
*/
INT32 kdrv_dre_close(ULONG handle);

/**
     get information

     @param[in] handler hw resouce handle
     @param[in] param_id enum kdrv_dre_param_id
     @param[in,out] p_param parameter, data type ref. enum kdrv_dre_param_id

     @return Description of data returned.
         - @b >= 0: success
         - @b <  0: fail
*/
INT32 kdrv_dre_get(ULONG handle, enum kdrv_dre_param_id param_id, void *p_param);

/**
     set information

     @param[in] handler hw resouce handle
     @param[in] param_id enum kdrv_dre_param_id
     @param[in,out] p_param parameter, data type ref. enum kdrv_dre_param_id

     @return Description of data returned.
         - @b >= 0: success
         - @b <  0: fail
*/
INT32 kdrv_dre_set(ULONG handle, enum kdrv_dre_param_id param_id, void *p_param);

/**
     trigger hw engine

     @param[in] id hw resouce handle
     @param[in] job information

     @return Description of data returned.
         - @b = DRE_DRV_STS_OK: success
*/
enum dre_drv_sts kdrv_dre_trigger(ULONG handle, struct dre_drv_job_head *head);
#endif  /* _DRE_DRV_CTL_H_ */

