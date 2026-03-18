/**
	@brief Header file of vendor videoprocess module.\n
	This file contains the functions which is related to vendor videoprocess.

	@file vendor_videoprocess.h

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2020.  All rights reserved.
*/

#ifndef _VENDOR_VIDEOPROCESS_H_
#define _VENDOR_VIDEOPROCESS_H_

#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "hd_common.h"
#include "hd_videoprocess.h"
/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/
#define HD_VIDEO_DDR_ID_SHIFT            0
#define HD_VIDEO_DDR_ID_MASK             0x000f

#define HD_VIDEO_IPP_DIRECT_SHIFT        4
#define HD_VIDEO_IPP_DIRECT_MASK         0x00f0

#define HD_VIDEO_MULTI_FRAME_SHIFT       8
#define HD_VIDEO_MULTI_FRAME_MASK        0x0f00

#define HD_VIDEO_PIX_YCC_SHIFT           12
#define HD_VIDEO_PIX_YCC_MASK            0xf000
#define HD_VIDEO_PIX_YCC_FULL            0x0000
#define HD_VIDEO_PIX_YCC_BT601           0x1000
#define HD_VIDEO_PIX_YCC_BT709           0x2000

#define HD_VIDEOPROC_PIPE_COMBINE       0x00001000

#define	HD_VIDEOPROC_PIPE_VPE_LITE      0x000000F3 ///< do YUV-domain scale process by VPE LITE engine
#define HD_VIDEOPROC_PIPE_RAWCOLOR      0x0000003E
#define	HD_VIDEOPROC_PIPE_DRE           0x000000F5
#define HD_VIDEOPROC_PIPE_RAWALL_LITE   0x000000F8
#define HD_VIDEOPROC_PIPE_RAWCOLOR_LITE 0x00000038
#define HD_VIDEOPROC_PIPE_PRE           0x00000006  ///< 1 RAW frame to 1 RAW frame, do RAW-domain pre-process, support in-crop, in-direct, BNR, SHDR.
#define HD_VIDEOPROC_PIPE_BNR_RAWALL    0x000600F8  ///< RAWALL without in-crop, in-direct, SHDR
#define HD_VIDEOPROC_PIPE_BNR_RAWCOLOR  0x00060038  ///< RAWCOLOR without in-crop, SHDR


#define HD_VIDEOPROC_FUNC_DIRECT_SCALEUP 0x04000000 ///< enable scaling up in direct mode

#define HD_VIDEOPROC_FUNC_FUSION        0x10000000 ///< enable fusion for ISP-AI (multi frame)
#define HD_VIDEOPROC_FUNC_BNR           0x20000000 ///< enable Bayer 3DNR effect
#define HD_VIDEOPROC_FUNC_BNR_STA       0x40000000 ///< enable Bayer 3DNR statistic for ISP tools
#define HD_VIDEOPROC_FUNC_AIDED         0x80000000 ///< enable aided info for ISP-AI

#define VENDOR_VIDEOPROC_VSP_MAX_FRM_UM  	4
#define VENDOR_VIDEOPROC_DRE_MAX_LAYER_NUM 10

#define VENDOR_VIDEOPROC_DRE_MAX_FRM_NUM 2	//max dre blending function support frame number

#define VENDOR_VIDEOPROC_ISP_AI_EFFECT_MAX 16

#define HD_VPE_SPLIT_OUT_ID(src_id)  (src_id + 5)
#define HD_VPE_SPLIT2_OUT_ID(src_id)  (src_id + 9) //only for 539A

/**
	@name option of output function
*/
typedef enum _VENDOR_VIDEOPROC_OUTFUNC {
	VENDOR_VIDEOPROC_OUTFUNC_COPYMETA = 0x00001000, ///< using out path, copy in-frame meta data to out-frame
} VENDOR_VIDEOPROC_OUTFUNC;

/********************************************************************
	MACRO FUNCTION DEFINITIONS
********************************************************************/

/********************************************************************
	TYPE DEFINITION
********************************************************************/
typedef enum {
	VENDOR_VIDEOPROC_EIS_FUNC_NONE = 0,    //eis is disabled
	VENDOR_VIDEOPROC_EIS_FUNC_NORMAL,      //eis is enabled with NVT gyro data
	VENDOR_VIDEOPROC_EIS_FUNC_CUSTOMIZED,  //eis is enabled with user's gyro data and others
	VENDOR_VIDEOPROC_EIS_FUNC_USER_DATA,   //eis is enabled with NVT gyro data and user data
	ENUM_DUMMY4WORD(VENDOR_VIDEOPROC_EIS_FUNC)
} VENDOR_VIDEOPROC_EIS_FUNC;
typedef struct _VENDOR_VIDEOPROC_OUT_ONEBUF_MAX {
	UINT32 max_size;                         ///< output max size. onebuf allocate max buf
} VENDOR_VIDEOPROC_OUT_ONEBUF_MAX;

typedef struct _VENDOR_VIDEOPROC_LCA_FUNC {
	UINT32 enable;
} VENDOR_VIDEOPROC_LCA_FUNC;

typedef enum {
	VENDOR_VIDEOPROC_DRE_PROC_FUSION_SIMP = 0,  ///< Simplified process,
	VENDOR_VIDEOPROC_DRE_PROC_FUSION_COMP,      ///< Complete process, better quality and more memory needed
	VENDOR_VIDEOPROC_DRE_PROC_MODE_MAX,
} VENDOR_VIDEOPROC_DRE_PROC_MODE;

typedef struct _VENDOR_VIDEOPROC_VSP_CFG {
	UINT32     blend_frm_num;       ///< blending frame numbers
	UINT32     dewarp_out_x_ofs_max[VENDOR_VIDEOPROC_VSP_MAX_FRM_UM];	//no use
	UINT32     ovlp_width[VENDOR_VIDEOPROC_VSP_MAX_FRM_UM]; ///< the overlap width of each frame
	HD_DIM     scale_out_size[VENDOR_VIDEOPROC_VSP_MAX_FRM_UM]; ///< scale out size of each frame
	UINT32     dre_proc_mode;       ///< the dre fusion process mode, using enum VENDOR_VIDEOPROC_DRE_PROC_MODE
	UINT32     dre_quality_level;	///< the dre fusion quality , min: 0(minimum proc time, low quality), max: 6(maximum proc time, highest quality)
	UINTPTR    dre_fusion_tbl_pa[VENDOR_VIDEOPROC_VSP_MAX_FRM_UM];   ///< the physical address of dre fusion table (layer 0)
    UINTPTR    dre_fusion_tbl_pa_ext[VENDOR_VIDEOPROC_VSP_MAX_FRM_UM][VENDOR_VIDEOPROC_DRE_MAX_LAYER_NUM];   			 ///< the physical address of dre fusion table(all layer)
    UINT32     dre_fusion_tbl_lofs[VENDOR_VIDEOPROC_VSP_MAX_FRM_UM][VENDOR_VIDEOPROC_DRE_MAX_LAYER_NUM];	///< lineoffset of each layer dre fusion table
	UINTPTR    dre_work_buf_pa;                                      ///< the physical address of dre working buffer
	UINTPTR    dre_work_buf_size;                                    ///< the size of dre working buffer
	HD_DIM     vpe_out_size;										 ///< vpe ptz output
	UINT32     en_ptz;                                               ///< enable ptz
	UINT32     dis_fusion;                                           ///< disable funsion
	UINT32     smart_ver;                                            ///< smart stitch version
} VENDOR_VIDEOPROC_VSP_CFG;

typedef struct _VENDOR_VIDEOPROC_DRE_CFG {
	UINT32     dre_proc_mode;       ///< the dre fusion process mode, using enum VENDOR_VIDEOPROC_DRE_PROC_MODE
	UINT32     dre_quality_level;	///< the dre fusion quality , min: 0(minimum proc time, low quality), max: 6(maximum proc time, highest quality)
	UINTPTR    dre_fusion_tbl_pa;   ///< the physical address of dre fusion table (layer 0)
    UINTPTR    dre_fusion_tbl_pa_ext[VENDOR_VIDEOPROC_DRE_MAX_LAYER_NUM];   ///< the physical address of dre fusion table (all layer)
    UINT32     dre_fusion_tbl_lofs[VENDOR_VIDEOPROC_DRE_MAX_LAYER_NUM];		///< lineoffset of each layer dre fusion table
	UINTPTR    dre_work_buf_pa;     ///< the physical address of dre working buffer
	UINTPTR    dre_work_buf_size;   ///< the size of dre working buffer
	HD_URECT   in_crop[VENDOR_VIDEOPROC_DRE_MAX_FRM_NUM];//crop for input image, if needed
} VENDOR_VIDEOPROC_DRE_CFG;

typedef struct _VENDOR_VIDEOPROC_SLICE_MODE {
	UINT8 enable[HD_VP_MAX_OUT];                            ///< [in]  enable slice push on some path output
	UINT8 cnt;                                              ///< [in]  slice count
} VENDOR_VIDEOPROC_SLICE_MODE;

typedef struct _VENDOR_VIDEOPROC_VPE_CLEAR_WIN {
	UINT32   color;     ///< rectangle color, 0x00RRGGBB
	HD_URECT rect;      ///< coordinate and dimension
} VENDOR_VIDEOPROC_VPE_CLEAR_WIN;

typedef struct _VENDOR_VIDEOPROC_ISP_AI {
    UINT32 path_id; ///<  fill 0 ... 3
    UINT32 proc_id; ///< proc_id of AI model
} VENDOR_VIDEOPROC_ISP_AI;

typedef struct _VENDOR_VIDEOPROC_VPE_MASK {
	UINT8            enable;            ///< enable/disable this mask
	HD_OSG_MASK_TYPE type;              ///< mask is solid or hollow.
	UINT32           color;             ///< mask color in rgb, mask palette index
	UINT32           alpha;             ///< mask transparency
	HD_UPOINT        position[4];       ///< 4 vertices' position
	UINT32           thickness;         ///< border width for hollow mask
} VENDOR_VIDEOPROC_VPE_MASK;

typedef struct _VENDOR_VIDEOPROC_VPE_MOSAIC {
	UINT8     enable;                   ///< enable/disable this mosaic
	UINT32    mosaic_blk_w;             ///< witdh of this mosaic
	UINT32    mosaic_blk_h;             ///< height of this mosaic
	HD_UPOINT position[4];              ///< 4 vertices' position
} VENDOR_VIDEOPROC_VPE_MOSAIC;

typedef struct _VENDOR_VIDEOPROC_FISHEYE_MASK {
	UINT8  enable;   ///< enable/disable this fisheye mask
	UINT32 cent_x;   ///< fish mask center x
	UINT32 cent_y;   ///< fish mask center y
	UINT32 valid_r;  ///< fish mask radius
	UINT8  decs_r;   ///< fish mask sharp range: [0, 255]
} VENDOR_VIDEOPROC_FISHEYE_MASK;

typedef struct _VENDOR_VIDEOPROC_POLYGON_MASK {
	UINT8            enable;           ///< enable/disable this polygon
	UINT8            num;              ///< how many positions. only 4, 6, 8, 10 are valid
	UINT32           color;            ///< polygon color in rgb
	UINT32           alpha;            ///< polygon transparency
	HD_UPOINT        position[10];     ///< 10 vertices' position
	UINT32           is_mosaic;        ///< is this polygon a mosaic
	UINT32           mosaic_blk_w;     ///< witdh of this mosaic
	UINT32           mosaic_blk_h;     ///< height of this mosaic
} VENDOR_VIDEOPROC_POLYGON_MASK;

typedef enum {
	VENDOR_VIDEOPROC_VPE_SPLIT_OFF,
	VENDOR_VIDEOPROC_VPE_SPLIT_LEFT_RIGHT,
	VENDOR_VIDEOPROC_VPE_SPLIT_UP_DOWN,    ///< only for 539A
	VENDOR_VIDEOPROC_VPE_SPLIT_OP_MAX
} VENDOR_VIDEOPROC_VPE_SPLIT_OP;

typedef struct _VENDOR_VIDEOPROC_VPE_SPLIT_INFO {
	VENDOR_VIDEOPROC_VPE_SPLIT_OP op;  ///< split mode
	UINT32 split_point;                ///< split position from source path */
	UINT32 depth;                      ///< output queue depth, set larger than 0 to allow pull_out
	HD_DIM bg;                         ///< output backgrond dimension
	HD_UPOINT dst_pos;                 ///< destination position base on background */
} VENDOR_VIDEOPROC_VPE_SPLIT_INFO;

typedef struct _VENDOR_VIDEOPROC_THERMAL_INFO {
	BOOL enable;                       ///< thermal mode enable
} VENDOR_VIDEOPROC_THERMAL_INFO;

typedef struct _VENDOR_VIDEOPROC_ISP_AI_EFFECT {
	UINT32 enable;                                          ///< AI-ISP effect map enable
	UINT32 path_id;                                         ///< AI-ISP path 0 ... 3
	UINT32 proc_id[VENDOR_VIDEOPROC_ISP_AI_EFFECT_MAX];     ///< proc_id of AI model for ISP effect 1 ... MAX
} VENDOR_VIDEOPROC_ISP_AI_EFFECT;

typedef struct _VENDOR_VIDEOPROC_ISP_AI_POSTPROC {
	UINT32 enable;                                          ///< AI-ISP postproc notify enable
} VENDOR_VIDEOPROC_ISP_AI_POSTPROC;

typedef enum _VENDOR_VIDEOPROC_PARAM_ID {
	VENDOR_VIDEOPROC_PARAM_HEIGHT_ALIGN,          ///< using device id, refer to VENDOR_VIDEOPROC_USRDATA_CFG struct
	VENDOR_VIDEOPROC_PARAM_IN_DEPTH,	  	      ///< using in id
	VENDOR_VIDEOPROC_PARAM_DMA_ABORT,		      ///< using device id, do DMA abort.
	VENDOR_VIDEOPROC_PARAM_SLICE_MODE,	       	  ///< set with ctrl path, enable slice mode.
	VENDOR_VIDEOPROC_PARAM_USER_CROP_TRIG,        ///< using path id, trigger user crop flow
	VENDOR_VIDEOPROC_CFG_DIS_SCALERATIO,	      ///< config scale-ratio of DIS func (1100, 1200, 1400)
	VENDOR_VIDEOPROC_CFG_DIS_SUBSAMPLE,		      ///< config sub-sample of DIS func (0, 1, 2)
	VENDOR_VIDEOPROC_PARAM_STRIP,		          ///< set with ctrl path, set strip level.
	VENDOR_VIDEOPROC_PARAM_OUT_ONEBUF_MAX,        ///< using out path, using VENDOR_VIDEOPROC_OUT_ONEBUF_MAX struct (output frame max dim)
	VENDOR_VIDEOPROC_PARAM_LINEOFFSET_ALIGN,      ///< using out path, config line offset align
	VENDOR_VIDEOPROC_PARAM_VSP_CFG,            	  ///< vsp parameters config
	VENDOR_VIDEOPROC_PARAM_DRE_CFG,               ///< using out path, for dre parameter
	VENDOR_VIDEOPROC_PARAM_EIS_FUNC,              ///< using ctrl path, referring to VENDOR_VIDEOPROC_EIS_FUNC.
	VENDOR_VIDEOPROC_PARAM_VPE_CLEAR_WIN,         ///< using out path, config clear window for VPE only
    VENDOR_VIDEOPROC_PARAM_FUNC_CONFIG,           ///< support get/set with i/o path, using HD_VIDEOPROC_FUNC_CONFIG struct (path func config)
	VENDOR_VIDEOPROC_PARAM_AI_CB,                 ///< config AI callback
	VENDOR_VIDEOPROC_PARAM_ISP_AI_START,          ///< start ISP AI and config API mapping
	VENDOR_VIDEOPROC_PARAM_ISP_AI_STOP,           ///< stop ISP AI and config API mapping
	VENDOR_VIDEOPROC_PARAM_ISP_CB,                ///< Get AI to ISP callback
	VENDOR_VIDEOPROC_PARAM_VPE_MASK,              ///< using path id, set vpe mask, use VENDOR_VIDEOPROC_VPE_MASK struct
	VENDOR_VIDEOPROC_PARAM_VPE_MOSAIC,            ///< using path id, set vpe mosaic, use VENDOR_VIDEOPROC_VPE_MOSAIC struct
	VENDOR_VIDEOPROC_PARAM_FISHEYE_MASK,          ///< using path id, set fisheye mask, use VENDOR_VIDEOPROC_FISHEYE_MASK struct
	VENDOR_VIDEOPROC_PARAM_VPROC_POLYGON_MASK,    ///< using path id, set polygon mask for vproc, use VENDOR_VIDEOPROC_POLYGON_MASK struct
	VENDOR_VIDEOPROC_PARAM_VPE_POLYGON_MASK,      ///< using path id, set polygon mask for vpe, use VENDOR_VIDEOPROC_POLYGON_MASK struct
	VENDOR_VIDEOPROC_PARAM_VPE_SPLIT_INFO,        ///< using out path, refer to VENDOR_VIDEOPROC_VPE_SPLIT_INFO struct
	VENDOR_VIDEOPROC_PARAM_THERMAL_INFO,          ///< using ctrl path, referring to VENDOR_VIDEOPROC_THERMAL_INFO.
	VENDOR_VIDEOPROC_PARAM_LCA_FUNC,              ///< using ctrl path, referring to VENDOR_VIDEOPROC_EIS_FUNC.
	VENDOR_VIDEOPROC_PARAM_ISP_AI_EFFECT,         ///< using ctrl path, config ISP effect and AI model mapping, referring to VENDOR_VIDEOPROC_ISP_AI_EFFECT.
	VENDOR_VIDEOPROC_PARAM_ISP_AI_POSTPROC,       ///< using ctrl path, config on PRE to notify POST to enable AI-ISP process, referring to VENDOR_VIDEOPROC_ISP_AI_POSTPROC.
	ENUM_DUMMY4WORD(VENDOR_VIDEOPROC_PARAM_ID)
} VENDOR_VIDEOPROC_PARAM_ID;
/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
HD_RESULT vendor_videoproc_set(UINT32 id, VENDOR_VIDEOPROC_PARAM_ID param_id, VOID *p_param);
HD_RESULT vendor_videoproc_get(UINT32 id, VENDOR_VIDEOPROC_PARAM_ID param_id, VOID *p_param);


#ifdef __cplusplus
}
#endif

#endif

