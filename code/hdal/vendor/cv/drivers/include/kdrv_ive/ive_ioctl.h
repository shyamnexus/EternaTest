#ifndef __IVE_IOCTL_CMD_H_
#define __IVE_IOCTL_CMD_H_

#include "kwrap/ioctl.h"
#include <kwrap/nvt_type.h>

#define NEW_IVE_IOCTL_IF 1

#if defined (__FREERTOS)
#define NVTMPP_OPEN(...) 0
#define NVTMPP_IOCTL nvtmpp_ioctl
#define NVTMPP_CLOSE(...)

#define NVTIVE_OPEN(...) 0
#define NVTIVE_IOCTL nvt_ive_ioctl
#define NVTIVE_CLOSE(...)
#else
#define NVTMPP_OPEN  open
#define NVTMPP_IOCTL ioctl
#define NVTMPP_CLOSE close

#define NVTIVE_OPEN  open
#define NVTIVE_IOCTL ioctl
#define NVTIVE_CLOSE close
#endif

#if defined(__FREERTOS)
#else
#define MODULE_REG_LIST_NUM	 11

typedef struct reg_info {
	unsigned int uiAddr;
	unsigned int uiValue;
} REG_INFO;

typedef struct reg_info_list {
	unsigned int uiCount;
	REG_INFO RegList[MODULE_REG_LIST_NUM];
} REG_INFO_LIST;
#endif

#define IVE_DRV_GEN_FILT_NUM	   10
#define IVE_DRV_EDGE_COEFF_NUM	   25
#define IVE_DRV_EDGE_TH_LUT_TAP   15
#define IVE_DRV_MORPH_NEIGH_NUM   24
#define IVE_DRV_YUV_COEFF_NUM     9
#define IVE_DRV_YUV_BIAS_NUM      3
#define IVE_DRV_DMACPY_HOR_FAC_NUM     5


/* job status */
enum ive_drv_sts {
    IVE_DRV_STS_IDLE = 0,
    IVE_DRV_STS_QUEUE,     // not process yet, in job list
    IVE_DRV_STS_PROC,		// ready to process
    IVE_DRV_STS_PROC_END,	// process done
    IVE_DRV_STS_DONE,		// done by hw process
    IVE_DRV_STS_FLUSH,     // stop by driver(not done)
    IVE_DRV_STS_ERROR,		// error by driver(not done)
    IVE_DRV_STS_OK,
	IVE_DRV_STS_QFULL,
    IVE_DRV_STS_MAX
};

enum ive_drv_op_mode {
    IVE_DRV_OP_CANNY_EDGE = 0,
    IVE_DRV_OP_VA,
    IVE_DRV_OP_REV1,//IVE_DRV_OP_CSC,
    IVE_DRV_OP_HISTOGRAM,
    IVE_DRV_OP_NCC,
	IVE_DRV_OP_DMA_COPY,
	IVE_DRV_OP_THRES_LUT,
	IVE_DRV_OP_IMG_OP,
	IVE_DRV_OP_16BIT_TO_8BIT,
	IVE_DRV_OP_REV2,//IVE_DRV_OP_CCL,
	IVE_DRV_OP_ST_CORNER,
    IVE_DRV_OP_LBP,
    IVE_DRV_OP_REV3,//IVE_DRV_OP_2D_LUT,
    IVE_DRV_OP_PACKED_IMG,
	IVE_DRV_OP_MAX,
};


struct ive_drv_addr {
	//uintptr_t va;
	uintptr_t pa;
};

struct ive_drv_roi {
	UINT16 w;
	UINT16 h;
	UINT16 lofs[3];		//lofs[2]: only valid at out_info or 3'rd planar input of IVE_DRV_OP_PACKED_IMG
};

struct ive_drv_in_info {
	struct ive_drv_addr addr[3];
	UINT8 ddr_id;
	struct ive_drv_roi roi;
};

struct ive_drv_out_info {
	struct ive_drv_addr addr[3];
	UINT8 ddr_id;
	struct ive_drv_roi roi;
};

typedef enum {
    IVE_DRV_GENFILT_IN_Y = 0,
    IVE_DRV_GENFILT_IN_UV_420,
    IVE_DRV_GENFILT_IN_UV_422,
	IVE_DRV_GENFILT_IN_MAX,
} IVE_DRV_GENFILT_IN_FMT;

struct IVE_DRV_GEN_FILT{
    UINT8 enable;
    UINT8 coeff[IVE_DRV_GEN_FILT_NUM];
    IVE_DRV_GENFILT_IN_FMT in_fmt;
    UINT8 sub_ratio; //0~3
};

typedef enum{
    IVE_DRV_ORST_MODE_MEDIAN = 0,
    IVE_DRV_ORST_MODE_MAX = 1,
    IVE_DRV_ORST_MODE_MIN = 2,
}IVE_DRV_ORST_MODE;


struct IVE_DRV_ORST_FILT{
    UINT8 enable;
    IVE_DRV_ORST_MODE mode;
};

typedef enum{
    IVE_DRV_EDGE_MODE_BI_DIR = 0, // if canny out fmt is not 0, only can be set BI_DIR
    IVE_DRV_EDGE_MODE_NO_DIR = 1,
    IVE_DRV_EDGE_MODE_ALPHA_BLENDING = 2,
}IVE_DRV_EDGE_MODE;

typedef enum{
    IVE_DRV_EDGE_KERNEL_5x5 = 0,
    IVE_DRV_EDGE_KERNEL_7x7 = 1,
}IVE_DRV_EDGE_KERNEL_TYPE;

struct IVE_DRV_EDGE_FILT {
    UINT8 enable;
    IVE_DRV_EDGE_MODE mode;
    //IVE_DRV_EDGE_KERNEL_TYPE kernel;
    UINT8 shift_bit;
    UINT8 angle_slp;
    UINT8 alpha_blending_fact;
    INT8 coeff[2][IVE_DRV_EDGE_COEFF_NUM];
};


struct IVE_DRV_NON_MAX_SUP {
    UINT8 enable;
    UINT8 edge_mag_th;
};


struct IVE_DRV_THRES_LUT{
    UINT8 enable;
    UINT8 mode; // 0~5 for canny & 1~11 for Lut path
    UINT16 low_th;
    UINT16 high_th;
    UINT8 th_min_val;
    UINT8 th_mid_val;
    UINT8 th_max_val;
};

typedef enum{
    IVE_DRV_POSTPROC_MODE_MORPH = 0,
    IVE_DRV_POSTPROC_MODE_HYSTER = 1,
}IVE_DRV_POSTPROC_MODE;

typedef enum{
    IVE_DRV_POSTPROC_MORPH_DILATION = 0,
    IVE_DRV_POSTPROC_MORPH_EROSION = 1,
}IVE_DRV_POSTPROC_MORPH_OP;


struct IVE_DRV_POST_PROC {
    UINT8 enable;
    IVE_DRV_POSTPROC_MODE mode;
    IVE_DRV_POSTPROC_MORPH_OP morph_op;
    UINT8 hyster_low_th;
    UINT8 hyster_high_th;
    UINT8 mask_en_bit[IVE_DRV_MORPH_NEIGH_NUM]; // 0~1
    //IVE_DRV_THRES_LUT_TH th_val;
};

typedef enum{
    IVE_DRV_CANNY_OUT_8BIT = 0,
    IVE_DRV_CANNY_OUT_12BIT = 1, // 8bit edge + 4bit theta pack
    IVE_DRV_CANNY_OUT_16BIT = 2, // 8bit grad_x + 8bit grad_y
    IVE_DRV_CANNY_OUT_20BIT = 3, // 16bit edge + 4bit theta pack
    IVE_DRV_CANNY_OUT_32BIT = 4, // 16bit grad_x + 16bit grad_y
    //IVE_DRV_CANNY_OUT_8BIT_2CH = 5, // 8bit grad_x on ch0 + 8bit grad_y on ch1
    //IVE_DRV_CANNY_OUT_16BIT_2CH = 6, // 16bit grad_x on ch0 + 16bit grad_y on ch1
    IVE_DRV_CANNY_OUT_MAX
}IVE_DRV_CANNY_OUT_SEL;


typedef enum{
    IVE_DRV_CSC_IN_YUV420 = 0,
    IVE_DRV_CSC_IN_YVU420 = 1,
    IVE_DRV_CSC_IN_UYVY = 2,
    IVE_DRV_CSC_IN_YUYV = 3,
    IVE_DRV_CSC_IN_VYUY = 4,
    IVE_DRV_CSC_IN_YVYU = 5,
}IVE_DRV_CSC_YUV_IN_FMT;

typedef enum{
    IVE_DRV_CSC_OUT_RGB_PLANAR = 0,
    IVE_DRV_CSC_OUT_RGB_PACKED = 1,
    IVE_DRV_CSC_OUT_HSV_PLANAR = 2,
    IVE_DRV_CSC_OUT_HSV_PACKED = 3,
    IVE_DRV_CSC_OUT_LAB_PLANAR = 4,
    IVE_DRV_CSC_OUT_LAB_PACKED = 5,
}IVE_DRV_CSC_YUV_OUT_FMT;

typedef enum{
    IVE_DRV_CSC_LAB_D65 = 0,
    IVE_DRV_CSC_LAB_D50 = 1,
}IVE_DRV_CSC_LAB_FMT;


typedef enum{
    IVE_DRV_HIST_MODE_0 = 0, // output histogram to addr0
    IVE_DRV_HIST_MODE_1 = 1, //  output cdf in addr0
    IVE_DRV_HIST_MODE_2 = 2, //  output histogram equalization
}IVE_DRV_HIST_MODE;


typedef enum{
    IVE_DRV_IMG_OP_ADD = 0, // (coef_a * in0 + coef_b * in1) >> shift_bit
    IVE_DRV_IMG_OP_SUB = 1, // abs(in0-in1) or (in0 - in1) >> 1
    IVE_DRV_IMG_OP_AND = 2, // in0 & in1
    IVE_DRV_IMG_OP_OR = 3, // in0 | in1
    IVE_DRV_IMG_OP_XOR = 4, // in0 ^ in1
}IVE_DRV_IMG_OP_MODE;

typedef enum{
    IVE_DRV_IMG_SUB_ABS = 0,
    IVE_DRV_IMG_SUB_IN0_SUB_IN1 = 1, // (in0 - in1) >> 1
}IVE_DRV_IMG_SUB_MODE;


typedef enum{
    IVE_DRV_CCL_8_CONN = 0,
    IVE_DRV_CCL_4_CONN = 1,
}IVE_DRV_CCL_MODE;

typedef enum{
    IVE_DRV_CCL_TRIG_1ST = 0,
    IVE_DRV_CCL_TRIG_2ND = 1,
}IVE_DRV_CCL_TRIG_MODE;


struct ive_drv_canny_edge_info {
	//set info
    struct IVE_DRV_GEN_FILT gen_filter;
    struct IVE_DRV_ORST_FILT orst_filter;
    struct IVE_DRV_EDGE_FILT edge_filter;
    struct IVE_DRV_NON_MAX_SUP non_max_sup;
    //struct IVE_DRV_INTEGRAL_IMG integral_img;
    struct IVE_DRV_THRES_LUT lut_threshold;
    struct IVE_DRV_POST_PROC post_proc;
    IVE_DRV_CANNY_OUT_SEL out_sel;
};

typedef enum{
    IVE_DRV_INTGAL_IN_Y = 0,
    IVE_DRV_INTGAL_IN_UV = 1,
    IVE_DRV_INTGAL_IN_HSV_RGB = 2,
}IVE_DRV_INTGAL_IN_FMT;

typedef enum{
    IVE_DRV_INTGAL_OUT_SUM = 0, // 32bit
    IVE_DRV_INTGAL_OUT_SQUARE = 1, // 64bit
    IVE_DRV_INTGAL_OUT_SQUARE_SUM = 2, // 64bit
}IVE_DRV_INTGAL_OUT_FMT;

struct IVE_DRV_INTEGRAL_IMG {
    UINT8 enable;
    IVE_DRV_INTGAL_IN_FMT in_fmt;
    IVE_DRV_INTGAL_OUT_FMT out_fmt;
};

typedef enum{
    IVE_DRV_MAP_MODE_DIRECT = 0, // 32bit
    IVE_DRV_MAP_MODE_INTERPOLATE = 1, // 64bit
}IVE_DRV_MAP_MODE;

struct IVE_DRV_MAP{
    UINT8 enable;
    //IVE_DRV_MAP_MODE mode;
    //UINT8 idx_shift;
};

struct ive_drv_va_info {
	//set info
	UINT8 input_fmt; // 0:8bit, 1:16bit
    struct IVE_DRV_GEN_FILT gen_filter;
    struct IVE_DRV_MAP map;
    //struct IVE_DRV_EDGE_FILT edge_filter;
    //struct IVE_DRV_THRES_LUT lut_threshold;
    struct IVE_DRV_INTEGRAL_IMG integral_img;
};

struct ive_drv_thres_lut_info{
    UINT8 enable;
    UINT8 mode; // 1~5 for canny & 1~11 for Lut path
    UINT16 low_th;
    UINT16 high_th;
    UINT8 th_min_val;
    UINT8 th_mid_val;
    UINT8 th_max_val;
};


struct ive_drv_copy_info {
	//set info
    struct IVE_DRV_GEN_FILT gen_filter;
    UINT8 hor_seg_size;
    UINT16 ver_seg_size;
    UINT8 element_size;
    struct IVE_DRV_EDGE_FILT edge_filter;
    UINT8 sobel_out_sel; // 0: 24bit (8 + 8 + 8), 1: 40bit (16 + 16 + 8)
    UINT8 input_fmt; // 0: y format, 1: 24bit (8 + 8 + 8), 2: 40bit (16 + 16 + 8)
};

struct ive_drv_ncc_info {
	//set info

	//get info
	UINT64 ncc_numerator;	//return value
	UINT64 ncc_quad_sum0;	//return value
	UINT64 ncc_quad_sum1;	//return value
};

struct ive_drv_histo_info {
	//set info
    IVE_DRV_HIST_MODE mode;
	UINT32 coef_a;
    UINT32 coef_b;
    UINT8 shift_bit;

	//get info
	//UINT32 cdf_min;		//return value
};
#if 0
struct ive_drv_csc_info {
	//set info
    IVE_DRV_CSC_YUV_IN_FMT in_fmt;
    IVE_DRV_CSC_YUV_OUT_FMT out_fmt;
    UINT8 hue_shift; // 0~127
    IVE_DRV_CSC_LAB_FMT lab_fmt;
    UINT8 gamma_en; // 0~1
    INT16 coef[IVE_DRV_YUV_COEFF_NUM]; // -16384 ~ 16383
    INT16 bias[IVE_DRV_YUV_BIAS_NUM]; // -256 ~ 255

	//get info
};

struct ive_drv_ccl_info {
	//set info
    IVE_DRV_CCL_MODE mode;
    IVE_DRV_CCL_TRIG_MODE trig_mode;
    UINT8 region_x_num;
    UINT16 region_y_num;// 1~256
    UINT8 foreground_val;
    UINT16 region_w;
    UINT16 region_h;

	//get info
};
#endif

struct ive_drv_img_op_info {
	//set info
    IVE_DRV_IMG_OP_MODE mode;
    IVE_DRV_IMG_SUB_MODE sub_mode;
    UINT32 coef_a;
    UINT32 coef_b;
    UINT8 shift_bit;

	//get info
};

struct ive_drv_16_to_8_info {
	//set info
    UINT8 mode; // 0~4
    UINT32 coef_a;
    UINT32 coef_b;
    INT8 coef_bias;
    UINT8 shift_bit;
    //UINT32 hist_cdf_min;

	//get info
};

struct ive_drv_st_info {
	//set info
    UINT8 mode; // 0~1
    UINT8 quality_lvl;
    //UINT8 min_dist;
    //UINT16 max_corner_num;
    //UINT16 sort_topn;
    //UINT16 max_eigen_val;
    UINT8 blk_dist;
    //UINT16 strp_blk_x_num; // = 1280 / blk_dist --> calculate by driver
    //UINT16 blk_x_num; --> calculate by driver
    //UINT16 blk_y_num; --> calculate by driver
    //UINT32 point_num_from_1stage; // = blk_x_num * blk_y_num --> calculate by driver
    UINT8 thres_mode; // 0: auto, 1: manual
    UINT32 thres_value; // only vaild when thres_mode == 1
	//get info
    //UINT16 corner_num;
};

typedef enum{
    IVE_DRV_LBP_MODE_NORM = 0,
    IVE_DRV_LBP_MODE_ABS = 1,
}IVE_DRV_LBP_MODE;

struct ive_drv_lbp_info {
    IVE_DRV_LBP_MODE mode;
    INT16 threshold; // INT8 in mode0, and UINT8 in mode1
};

typedef struct _ive_param_cfg {
	struct ive_drv_in_info in;
	struct ive_drv_out_info out;
	enum ive_drv_op_mode op;
	union {
		struct ive_drv_canny_edge_info canny_edge;
		struct ive_drv_ncc_info ncc;
		//struct ive_drv_csc_info csc;
		struct ive_drv_histo_info histo;
		//struct ive_drv_ccl_info ccl;
        struct ive_drv_thres_lut_info lut_thres;
		struct ive_drv_img_op_info img_op;
		struct ive_drv_16_to_8_info bit16to8;
		struct ive_drv_copy_info copy;
		struct ive_drv_st_info st;
		struct ive_drv_va_info va;
		struct ive_drv_lbp_info lbp;
	};
	int apply_next; //1: get result and set to next job
    //enum ive_drv_sts sts; //job ststus
	void* next_job_cfg;   //ive_param_cfg , if not have next job, set this to null
} ive_param_cfg;

typedef struct _ive_drv_process_cfg {
//----------set------------------------
	UINT32 chip;
	UINT32 eng;
	UINT32 id;
    BOOL instant; // 0: non-blocking 1: blocking
    UINT16 num; //total job number
    UINT8 priority;
    void *job_cfg;   //ive_param_cfg, if not have next job, set this to null
//----------get------------------------
    unsigned int handle;
}ive_drv_process_cfg;

typedef struct _ive_drv_query_cfg {
    unsigned int handle;
    BOOL instant; // 0: non-blocking 1: blocking
    BOOL job_finish;
}ive_drv_query_cfg;

//============================================================================
// IOCTL command
//============================================================================
#define IVE_IOC_COMMON_TYPE 'M'
#if !NEW_IVE_IOCTL_IF
#define IVE_IOC_START					_VOS_IO(IVE_IOC_COMMON_TYPE, 1)
#define IVE_IOC_STOP					_VOS_IO(IVE_IOC_COMMON_TYPE, 2)

#define IVE_IOC_READ_REG				_VOS_IOWR(IVE_IOC_COMMON_TYPE, 3, void*)
#define IVE_IOC_WRITE_REG				_VOS_IOWR(IVE_IOC_COMMON_TYPE, 4, void*)
#define IVE_IOC_READ_REG_LIST			_VOS_IOWR(IVE_IOC_COMMON_TYPE, 5, void*)
#define IVE_IOC_WRITE_REG_LIST			_VOS_IOWR(IVE_IOC_COMMON_TYPE, 6, void*)

#define IVE_IOC_OPEN                      _VOS_IOWR(IVE_IOC_COMMON_TYPE,  7, void*)
#define IVE_IOC_CLOSE                     _VOS_IOWR(IVE_IOC_COMMON_TYPE,  8, void*)
#define IVE_IOC_OPENCFG					  _VOS_IOWR(IVE_IOC_COMMON_TYPE,  9, void*)
#define IVE_IOC_SET_IMG_INFO              _VOS_IOWR(IVE_IOC_COMMON_TYPE, 10, void*)
#define IVE_IOC_GET_IMG_INFO              _VOS_IOWR(IVE_IOC_COMMON_TYPE, 11, void*)
#define IVE_IOC_SET_IMG_DMA_IN            _VOS_IOWR(IVE_IOC_COMMON_TYPE, 12, void*)
#define IVE_IOC_GET_IMG_DMA_IN            _VOS_IOWR(IVE_IOC_COMMON_TYPE, 13, void*)
#define IVE_IOC_SET_IMG_DMA_OUT           _VOS_IOWR(IVE_IOC_COMMON_TYPE, 14, void*)
#define IVE_IOC_GET_IMG_DMA_OUT           _VOS_IOWR(IVE_IOC_COMMON_TYPE, 15, void*)
#define IVE_IOC_SET_GENERAL_FILTER        _VOS_IOWR(IVE_IOC_COMMON_TYPE, 16, void*)
#define IVE_IOC_GET_GENERAL_FILTER        _VOS_IOWR(IVE_IOC_COMMON_TYPE, 17, void*)
#define IVE_IOC_SET_ORST_FILTER           _VOS_IOWR(IVE_IOC_COMMON_TYPE, 18, void*)
#define IVE_IOC_GET_ORST_FILTER           _VOS_IOWR(IVE_IOC_COMMON_TYPE, 19, void*)
#define IVE_IOC_SET_EDGE_FILTER           _VOS_IOWR(IVE_IOC_COMMON_TYPE, 20, void*)
#define IVE_IOC_GET_EDGE_FILTER           _VOS_IOWR(IVE_IOC_COMMON_TYPE, 21, void*)
#define IVE_IOC_SET_NON_MAX_SUP           _VOS_IOWR(IVE_IOC_COMMON_TYPE, 22, void*)
#define IVE_IOC_GET_NON_MAX_SUP           _VOS_IOWR(IVE_IOC_COMMON_TYPE, 23, void*)
#define IVE_IOC_SET_THRES_LUT             _VOS_IOWR(IVE_IOC_COMMON_TYPE, 24, void*)
#define IVE_IOC_GET_THRES_LUT             _VOS_IOWR(IVE_IOC_COMMON_TYPE, 25, void*)
#define IVE_IOC_SET_POSTPROC              _VOS_IOWR(IVE_IOC_COMMON_TYPE, 26, void*)
#define IVE_IOC_GET_POSTPROC              _VOS_IOWR(IVE_IOC_COMMON_TYPE, 27, void*)
#define IVE_IOC_SET_INTEGRAL_IMG          _VOS_IOWR(IVE_IOC_COMMON_TYPE, 28, void*)
#define IVE_IOC_GET_INTEGRAL_IMG          _VOS_IOWR(IVE_IOC_COMMON_TYPE, 29, void*)
#define IVE_IOC_SET_ITER_REGION_VOTE      _VOS_IOWR(IVE_IOC_COMMON_TYPE, 30, void*)
#define IVE_IOC_GET_ITER_REGION_VOTE      _VOS_IOWR(IVE_IOC_COMMON_TYPE, 31, void*)
#define IVE_IOC_TRIGGER                   _VOS_IOWR(IVE_IOC_COMMON_TYPE, 32, void*)
#define IVE_IOC_SET_OUTSEL                _VOS_IOWR(IVE_IOC_COMMON_TYPE, 33, void*)
#define IVE_IOC_GET_OUTSEL                _VOS_IOWR(IVE_IOC_COMMON_TYPE, 34, void*)
#define IVE_IOC_SET_IRV                   _VOS_IOWR(IVE_IOC_COMMON_TYPE, 35, void*)
#define IVE_IOC_GET_IRV                   _VOS_IOWR(IVE_IOC_COMMON_TYPE, 36, void*)
#define IVE_IOC_SET_FLOWCT                _VOS_IOWR(IVE_IOC_COMMON_TYPE, 37, void*)
#define IVE_IOC_GET_FLOWCT                _VOS_IOWR(IVE_IOC_COMMON_TYPE, 38, void*)
#define IVE_IOC_SET_FUNCTION_MODE         _VOS_IOWR(IVE_IOC_COMMON_TYPE, 39, void*)
#define IVE_IOC_GET_FUNCTION_MODE         _VOS_IOWR(IVE_IOC_COMMON_TYPE, 40, void*)
//#define IVE_IOC_SET_CSC                   _VOS_IOWR(IVE_IOC_COMMON_TYPE, 41, void*)
//#define IVE_IOC_GET_CSC                   _VOS_IOWR(IVE_IOC_COMMON_TYPE, 42, void*)
#define IVE_IOC_SET_HIST                  _VOS_IOWR(IVE_IOC_COMMON_TYPE, 43, void*)
#define IVE_IOC_GET_HIST                  _VOS_IOWR(IVE_IOC_COMMON_TYPE, 44, void*)
#define IVE_IOC_GET_NCC                   _VOS_IOWR(IVE_IOC_COMMON_TYPE, 45, void*)
#define IVE_IOC_SET_DMA_COPY              _VOS_IOWR(IVE_IOC_COMMON_TYPE, 46, void*)
#define IVE_IOC_GET_DMA_COPY              _VOS_IOWR(IVE_IOC_COMMON_TYPE, 47, void*)
#define IVE_IOC_SET_IMG_OP                _VOS_IOWR(IVE_IOC_COMMON_TYPE, 48, void*)
#define IVE_IOC_GET_IMG_OP                _VOS_IOWR(IVE_IOC_COMMON_TYPE, 49, void*)
#define IVE_IOC_SET_16TO8BIT              _VOS_IOWR(IVE_IOC_COMMON_TYPE, 50, void*)
#define IVE_IOC_GET_16TO8BIT              _VOS_IOWR(IVE_IOC_COMMON_TYPE, 51, void*)
//#define IVE_IOC_SET_CCL                   _VOS_IOWR(IVE_IOC_COMMON_TYPE, 52, void*)
//#define IVE_IOC_GET_CCL                   _VOS_IOWR(IVE_IOC_COMMON_TYPE, 53, void*)
#define IVE_IOC_SET_ST_CORNER             _VOS_IOWR(IVE_IOC_COMMON_TYPE, 54, void*)
#define IVE_IOC_GET_ST_CORNER             _VOS_IOWR(IVE_IOC_COMMON_TYPE, 55, void*)
#define IVE_IOC_GET_VERSION               _VOS_IOWR(IVE_IOC_COMMON_TYPE, 56, void*)
#define IVE_IOC_INIT                      _VOS_IOWR(IVE_IOC_COMMON_TYPE, 57, void*)
#define IVE_IOC_UNINIT                    _VOS_IOWR(IVE_IOC_COMMON_TYPE, 58, void*)
#else
#define IVE_IOC_GET_VERSION               _VOS_IOWR(IVE_IOC_COMMON_TYPE, 56, void*)
#define IVE_IOC_TRIGGER_JOB               _VOS_IOWR(IVE_IOC_COMMON_TYPE, 60, ive_drv_process_cfg *)
#define IVE_IOC_QUERY_JOB                 _VOS_IOWR(IVE_IOC_COMMON_TYPE, 61, ive_drv_query_cfg *)
#endif
/* Add other command ID here*/
#if defined(__FREERTOS)
int nvt_ive_ioctl(int fd, unsigned int uiCmd, void *p_arg);
extern INT32 kdrv_ive_rtos_init(void);
extern INT32 kdrv_ive_rtos_uninit(void);
#endif


#endif
