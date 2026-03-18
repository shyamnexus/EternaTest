/**
    Public header file for IPL utility

    This file is the header file that define the API and data type for IPL utility tool.

    @file       ipl_utility.h
    @ingroup    mILibIPLCom
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2009.  All rights reserved.
*/
#ifndef _IPP_UTIL_H_
#define _IPP_UTIL_H_

#include "kwrap/type.h"
#include "kwrap/error_no.h"
#include "kflow_common/type_vdo.h"

#define CTL_IPP_520_MAX_STRIPE_W 2688
#define CTL_IPP_528_MAX_STRIPE_W 4092
#define CTL_IPP_560_MAX_STRIPE_W 2688
#define CTL_IPP_530_MAX_STRIPE_W 4096
#define CTL_IPP_690_MAX_STRIPE_W 4096
#define CTL_IPP_690_MAX_STRIPE_W_DUAL 8192
#define CTL_IPP_538_MAX_STRIPE_W 4096
#define CTL_IPP_567_MAX_STRIPE_W 2880
/**
	IPP Error Type
*/
#define CTL_IPP_E_OK			(0)
#define CTL_IPP_E_ID			(-1)	///< illegal handle
#define CTL_IPP_E_QOVR			(-2)	///< queue overflow
#define CTL_IPP_E_NSPT			(-3)	///< not support function
#define CTL_IPP_E_STATE			(-4)	///< illegal state
#define CTL_IPP_E_PAR			(-5)	///< illegal parameter
#define CTL_IPP_E_INDATA		(-6)	///< input data error
#define CTL_IPP_E_SYS			(-7)	///< system error
#define CTL_IPP_E_FLUSH			(-8)	///< flush buffer
#define CTL_IPP_E_NOMEM			(-9)	///< no memory
#define CTL_IPP_E_TIMEOUT		(-10)	///< timeout
#define CTL_IPP_E_SHDR			(-11)	///< SHDR start and stop type for direct
#define CTL_IPP_E_DIR_DROP		(-12)	///< busy drop for direct
#define CTL_IPP_E_NULL_FNNC		(-13)	///< null fucntion
#define CTL_IPP_E_DROP			(-14)	///< buffer drop event
#define CTL_IPP_E_SKIP			(-15)	///< skip trigger process
#define CTL_IPP_E_DELAY			(-16)	///< delay trigger process
#define CTL_IPP_E_ERASE_QUE_NN	(-17)	///< drop frame for nnisp job
#define CTL_IPP_E_KDRV_OPEN		(-100)	///< kdrv open error
#define CTL_IPP_E_KDRV_CLOSE	(-101)	///< kdrv close error
#define CTL_IPP_E_KDRV_SET		(-102)	///< kdrv set error
#define CTL_IPP_E_KDRV_GET		(-103)	///< kdrv get error
#define CTL_IPP_E_KDRV_TRIG		(-104)	///< kdrv trigger error
#define CTL_IPP_E_KDRV_STRP		(-105)	///< kdrv stripe cal error
#define CTL_IPP_E_KDRV_DROP		(-106)	///< kdrv drop cb


/**
    IPP flip information
*/
typedef enum _CTL_IPP_FLIP_TYPE {
	CTL_IPP_FLIP_NONE = 0x00000000,         ///< no flip
	CTL_IPP_FLIP_H    = 0x00000001,         ///< H flip
	CTL_IPP_FLIP_V    = 0x00000002,         ///< V flip
	CTL_IPP_FLIP_H_V  = 0x00000003,         ///< H & V flip(Flip_H | Flip_V) don't modify
	CTL_IPP_FLIP_MAX,
	ENUM_DUMMY4WORD(CTL_IPP_FLIP_TYPE)
} CTL_IPP_FLIP_TYPE;

/**
    IPP In/Out buffer config
*/
typedef enum _CTL_IPP_BUF_IO_CFG {
	CTL_IPP_BUF_IO_NEW = 0,
	CTL_IPP_BUF_IO_PUSH,
	CTL_IPP_BUF_IO_LOCK,
	CTL_IPP_BUF_IO_UNLOCK,
	CTL_IPP_BUF_IO_START,
	CTL_IPP_BUF_IO_STOP,
	CTL_IPP_BFU_IO_DRAM_START,
	CTL_IPP_BFU_IO_DRAM_END,
	CTL_IPP_BUF_IO_MAX,
	ENUM_DUMMY4WORD(CTL_IPP_BUF_IO_CFG)
} CTL_IPP_BUF_IO_CFG;

/**
	IPP OUT PATH ID
*/
typedef enum _CTL_IPP_OUT_PATH_ID{
	CTL_IPP_OUT_PATH_ID_1 = 0,
	CTL_IPP_OUT_PATH_ID_2 = 1,
	CTL_IPP_OUT_PATH_ID_3 = 2,
	CTL_IPP_OUT_PATH_ID_4 = 3,
	CTL_IPP_OUT_PATH_ID_5 = 4,
	CTL_IPP_OUT_PATH_ID_6 = 5,	// CFA output
	CTL_IPP_OUT_PATH_ID_7 = 6,	// IPE output
	CTL_IPP_OUT_PATH_ID_8 = 7,	// SUBISP output
	CTL_IPP_OUT_PATH_ID_9 = 8,	// BNR output
	CTL_IPP_OUT_PATH_ID_10 = 9,
	CTL_IPP_OUT_PATH_ID_MAX = 10,
	CTL_IPP_OUT_PATH_ID_IME_MAX = (CTL_IPP_OUT_PATH_ID_5+1), // IME path only
	CTL_IPP_OUT_PATH_ID_REF = CTL_IPP_OUT_PATH_ID_5,
	CTL_IPP_OUT_PATH_ID_CFA = CTL_IPP_OUT_PATH_ID_6,
	CTL_IPP_OUT_PATH_ID_IPE_OUT = CTL_IPP_OUT_PATH_ID_7,	// for RAWCOLOR flow
	CTL_IPP_OUT_PATH_ID_SUBISP = CTL_IPP_OUT_PATH_ID_8,		// for RAWCOLOR flow
	CTL_IPP_OUT_PATH_ID_PRE_OUT = CTL_IPP_OUT_PATH_ID_1,	// for DIRECT PRE flow
	CTL_IPP_OUT_PATH_ID_BNR_REF = CTL_IPP_OUT_PATH_ID_9,	// for BNR ref out. internal used
} CTL_IPP_OUT_PATH_ID;

/**
	IPP Scale Method
*/
typedef enum _CTL_IPP_IME_SCALER {
	CTL_IPP_SCL_BICUBIC     = 0,    ///< bicubic interpolation
	CTL_IPP_SCL_BILINEAR    = 1,    ///< bilinear interpolation
	CTL_IPP_SCL_NEAREST     = 2,    ///< nearest interpolation
	CTL_IPP_SCL_INTEGRATION = 3,    ///< integration interpolation,support only scale down
	CTL_IPP_SCL_AUTO        = 4,    ///< automatical calculation
	CTL_IPP_SCL_METHOD_MAX,
	ENUM_DUMMY4WORD(CTL_IPP_SCL_METHOD)
} CTL_IPP_SCL_METHOD;

typedef struct {
	UINT32 scl_th;                  /* scale done ratio threshold, [31..16]output, [15..0]input */
	CTL_IPP_SCL_METHOD method_l;    /* scale method when scale down ratio <= scl_th(output/input) */
	CTL_IPP_SCL_METHOD method_h;    /* scale method when scale down ratio >  scl_th(output/input) */
} CTL_IPP_SCL_METHOD_SEL;


/**
	IPP Func Enable
*/
typedef enum _CTL_IPP_FUNC {
	CTL_IPP_FUNC_NONE			=	0x00000000,
	CTL_IPP_FUNC_WDR			=	0x00000001,
	CTL_IPP_FUNC_SHDR			=	0x00000002,	// no use. use frm_num in vdo_frame to decide shdr. use CTL_IPP_FUNC_FUSION_WEIGHT to decide fusion weight output
	CTL_IPP_FUNC_DEFOG			=	0x00000004,
	CTL_IPP_FUNC_3DNR			=	0x00000008,
	CTL_IPP_FUNC_DATASTAMP		=	0x00000010,
	CTL_IPP_FUNC_PRIMASK		=	0x00000020,
	CTL_IPP_FUNC_PM_PIXELIZTION	=	0x00000040,
	CTL_IPP_FUNC_YUV_SUBOUT		=	0x00000080,
	CTL_IPP_FUNC_IPE_VA_SUBOUT	=	0x00000100,
	CTL_IPP_FUNC_3DNR_STA		=	0x00000200,
	CTL_IPP_FUNC_GDC			=	0x00000400,
	CTL_IPP_FUNC_DIRECT_SCL_UP	=	0x00000800,	//enable for support direct mode + scale up
	CTL_IPP_FUNC_IME_VA_SUBOUT	=	0x00001000,
	CTL_IPP_FUNC_IFE_VA_SUBOUT	=	0x00002000,
	CTL_IPP_FUNC_LCA			=	0x00004000, // LCA function enable, not LCA output (CTL_IPP_FUNC_YUV_SUBOUT, which not support for now)
	CTL_IPP_FUNC_PRE_VA_SUBOUT	=	0x00008000,
	CTL_IPP_FUNC_BNR			=	0x00010000,
	CTL_IPP_FUNC_BNR_STA		=	0x00020000,
	CTL_IPP_FUNC_AIDED_MAP		=	0x00040000, // allocate SHDR + BNR Sigma + BNR Gamma buffer
	CTL_IPP_FUNC_FUSION_WEIGHT	=	0x00080000, // SHDR fusion weight output, must use in fnum >= 2
	CTL_IPP_FUNC_MAX			=	0xffffffff,	// max function

	// [deprecated] backward compatible
	CTL_IPP_FUNC_VA_SUBOUT		=	CTL_IPP_FUNC_IPE_VA_SUBOUT,
} CTL_IPP_FUNC;

/**
	IPP Crop mode
*/
typedef enum _CTL_IPP_IN_CROP_MODE {
	CTL_IPP_IN_CROP_AUTO = 0,	/* reference input header information */
	CTL_IPP_IN_CROP_NONE,		/* no crop, process full image */
	CTL_IPP_IN_CROP_USER,		/* user define crop window */
	CTL_IPP_IN_CROP_MODE_MAX,
} CTL_IPP_IN_CROP_MODE;

/**
	IPP Color space type
*/
typedef enum _CTL_IPP_OUT_COLOR_SPACE {
	CTL_IPP_OUT_COLOR_FULL,		/* full range */
	CTL_IPP_OUT_COLOR_BT601,	/* BT.601 */
	CTL_IPP_OUT_COLOR_BT709,	/* BT.709 */
	CTL_IPP_OUT_COLOR_MAX
} CTL_IPP_OUT_COLOR_SPACE;

/**
	IPP Data stamp
*/

/* CTL_IPP_CBEVT_DATASTAMP callback structure*/
typedef struct {
} CTL_IPP_DS_CB_OUTPUT_INFO;

typedef struct {
} CTL_IPP_DS_CB_INPUT_INFO;


/**
	IPP Privacy Mask
*/
typedef enum {
	CTL_IPP_PM_SET_ID_1 = 0,
	CTL_IPP_PM_SET_ID_2,
	CTL_IPP_PM_SET_ID_3,
	CTL_IPP_PM_SET_ID_4,
	CTL_IPP_PM_SET_ID_5,
	CTL_IPP_PM_SET_ID_6,
	CTL_IPP_PM_SET_ID_7,
	CTL_IPP_PM_SET_ID_8,
	CTL_IPP_PM_SET_ID_MAX
} CTL_IPP_PM_SET_ID;

typedef enum {
	CTL_IPP_PM_MASK_TYPE_YUV,
	CTL_IPP_PM_MASK_TYPE_PXL,
	CTL_IPP_PM_MASK_TYPE_MAX,
	ENUM_DUMMY4WORD(CTL_IPP_PM_MASK_TYPE)
} CTL_IPP_PM_MASK_TYPE;

typedef enum {
	CTL_IPP_PM_MASK_SHAPE_HOLLOW,		// support yuv     mask type, support pm0/2/4/6, 			pm1/3/5/7 must be disabled
	CTL_IPP_PM_MASK_SHAPE_POLYGON_4,	// support yuv/pxl mask type, support pm0/1/2/3/4/5/6/7
	CTL_IPP_PM_MASK_SHAPE_POLYGON_6,	// support yuv/pxl mask type, support pm0/2/4/6
	CTL_IPP_PM_MASK_SHAPE_POLYGON_8,	// support yuv/pxl mask type, support pm0/2/4/6, 			pm1/3/5/7 must be disabled
	CTL_IPP_PM_MASK_SHAPE_POLYGON_10,	// support yuv/pxl mask type, support pm0/2/4/6, 			pm1/3/5/7 must be disabled
} CTL_IPP_PM_MASK_SHAPE;

typedef enum {
	CTL_IPP_PM_PXL_BLK_08,
	CTL_IPP_PM_PXL_BLK_16,
	CTL_IPP_PM_PXL_BLK_32,
	CTL_IPP_PM_PXL_BLK_64,
	CTL_IPP_PM_PXL_BLK_MAX
} CTL_IPP_PM_PXL_BLK;

typedef struct {
	UINT32 reserved;
	BOOL func_en;					/* privacy mask set enable */
	CTL_IPP_PM_MASK_SHAPE msk_shape;/* privacy mask shape selection */
	IPOINT pm_coord[10];			/* privacy mask (outside) 10 coordinates(horizontal and vertical direction), must be Convex Hull */
	IPOINT pm_coord_2[4];			/* privacy mask (inner)    4 coordinates(horizontal and vertical direction), must be Convex Hull */
	CTL_IPP_PM_MASK_TYPE msk_type;	/* privacy mask type selection */
	UINT32 color[3];				/* privacy mask using YUV color, range: [0, 255] */
	UINT32 alpha_weight;			/* privacy mask alpha weight, range: [0, 255] */
} CTL_IPP_PM;

typedef enum  {
	CTL_IPP_PM_PXL_SRC_DRAM = 0,	///< from DRAM. no 3DNR effect
	CTL_IPP_PM_PXL_SRC_LCA  = 1,	///< from LCA. take effect by 3DNR
	CTL_IPP_PM_PXL_SRC_CURF = 2,	///< from current frame. no 3DNR effect
	ENUM_DUMMY4WORD(CTL_IPP_PM_PXL_SRC_SEL)
} CTL_IPP_PM_PXL_SRC_SEL;

typedef struct {
	UINT8  enable;
	UINT32 cent_x;   ///< fish mask center x
	UINT32 cent_y;   ///< fish mask center y
	UINT32 valid_r;  ///< fish mask radius
	UINT8  decs_r;   ///< fish mask sharp range: [0, 255]
} CTL_IPP_PM_FISHEYE;

/* CTL_IPP_CBEVT_PRIMASK callback structure */
typedef struct {
	CTL_IPP_PM_PXL_BLK pxl_blk_size;		/* block size selectino for pixelation mask */
	CTL_IPP_PM_PXL_SRC_SEL pxl_src_sel;		/* color source for pixelation mask */
	CTL_IPP_PM mask[CTL_IPP_PM_SET_ID_MAX];	/* square mask info */
	CTL_IPP_PM_FISHEYE fisheye;				/* fisheye mask info */
} CTL_IPP_PM_CB_OUTPUT_INFO;

typedef struct {
	ULONG ctl_ipp_handle;	/* CTL_IPP_HANDLE */
	USIZE img_size;
} CTL_IPP_PM_CB_INPUT_INFO;

/**
	Engine ISR message
*/

typedef enum {
	CTL_IPP_RHE_INTE_FMD = 0x00000001,
} CTL_IPP_RHE_INTE_STS;

typedef enum {
	CTL_IPP_IFE_INTE_FMD 			 = 0x00000001,	///< enable interrupt: frame end
	CTL_IPP_IFE_INTE_DEC1_ERR        = 0x00000002,  ///< enable interrupt: rde decode error interrupt
	CTL_IPP_IFE_INTE_DEC2_ERR        = 0x00000004,  ///< enable interrupt: rde decode error interrupt
	CTL_IPP_IFE_INTE_LLEND           = 0x00000008,  ///< enable interrupt: LinkedList end interrupt
	CTL_IPP_IFE_INTE_LLERR           = 0x00000010,  ///< enable interrupt: LinkedList error  interrupt
	CTL_IPP_IFE_INTE_LLERR2          = 0x00000020,  ///< enable interrupt: LinkedList error2 interrupt
	CTL_IPP_IFE_INTE_LLJOBEND        = 0x00000040,  ///< enable interrupt: LinkedList job end interrupt
	CTL_IPP_IFE_INTE_BUFOVFL         = 0x00000080,  ///< enable interrupt: buffer overflow interrupt
	CTL_IPP_IFE_INTE_RING_BUF_ERR    = 0x00000100,  ///< enable interrupt: ring buffer error interrupt
	CTL_IPP_IFE_INTE_FRAME_ERR       = 0x00000200,  ///< enable interrupt: frame error interrupt (for direct mode)
	CTL_IPP_IFE_INTE_SIE_FRAME_START = 0x00001000,  ///< enable interrupt: SIE frame start interrupt (for direct mode)
} CTL_IPP_IFE_INTE_STS;

typedef enum {
	CTL_IPP_DCE_INTE_FST       = 0x00000001,   // enable interrupt: frame start
	CTL_IPP_DCE_INTE_FMD       = 0x00000002,   // enable interrupt: frame end
	CTL_IPP_DCE_INTE_STPERR    = 0x00000008,   // enable interrupt: DCE stripe error
	CTL_IPP_DCE_INTE_LBOVF     = 0x00000010,   // enable interrupt: DCE line buffer overflow error
	CTL_IPP_DCE_INTE_STPOB     = 0x00000040,   // enable interrupt: DCE stripe boundary overflow error
	CTL_IPP_DCE_INTE_YBACK     = 0x00000080,   // enable interrupt: DCE Y coordinate backward skip error
	CTL_IPP_DCE_INTE_LL_END    = 0x00000100,   // enable interrupt: Linked List done
	CTL_IPP_DCE_INTE_LL_ERR    = 0x00000200,   // enable interrupt: Linked List error command
	CTL_IPP_DCE_INTE_LL_ERR2   = 0x00000400,   // enable interrupt: Linked List error in direct mdoe
	CTL_IPP_DCE_INTE_LL_JOBEND = 0x00000800,   // enable interrupt: Linked List job end
	CTL_IPP_DCE_INTE_FRAMEERR  = 0x00001000,   // enable interrupt: frame start error in direct mode
} CTL_IPP_DCE_INTE_STS;

typedef enum {
	CTL_IPP_IPE_INTE_FMD			= 0x00000002,		///< enable interrupt: frame done
	CTL_IPP_IPE_INTE_STP			= 0x00000004,		///< enable interrupt: current stripe done
	CTL_IPP_IPE_INTE_FMS			= 0x00000008,		///< enable interrupt: frame start
	CTL_IPP_IPE_INTE_YCC_OUT_END	= 0x00000010,		///< enable interrupt: YCC DRAM output done
	CTL_IPP_IPE_INTE_GMA_IN_END		= 0x00000020,		///< enable interrupt: DRAM input done (gamma)
	CTL_IPP_IPE_INTE_DEFOG_IN_END	= 0x00000040,		///< enable interrupt: DRAM input done (defog)
	CTL_IPP_IPE_INTE_VA_OUT_END		= 0x00000080,		///< enable interrupt: DRAM VA output done
	CTL_IPP_IPE_INTE_LL_DONE		= 0x00000100,		///< enable interrupt: Linked List done
	CTL_IPP_IPE_INTE_LL_JOBEND		= 0x00000200,		///< enable interrupt: Linked List job end
	CTL_IPP_IPE_INTE_LL_ERR			= 0x00000400,		///< enable interrupt: Linked List error command
	CTL_IPP_IPE_INTE_LL_ERR2		= 0x00000800,		///< enable interrupt: Linked List error in direct mode
	CTL_IPP_IPE_INTE_FRAMEERR		= 0x00001000,		///< enable interrupt: frame start error in direct mode
} CTL_IPP_IPE_INTE_STS;

typedef enum {
	CTL_IPP_IME_INTE_LL_END              = 0x00000001,
	CTL_IPP_IME_INTE_LL_ERR              = 0x00000002,
	CTL_IPP_IME_INTE_LL_LATE             = 0x00000004,
	CTL_IPP_IME_INTE_LL_JEND             = 0x00000008,
	CTL_IPP_IME_INTE_BP1                 = 0x00000010,
	CTL_IPP_IME_INTE_BP2                 = 0x00000020,
	CTL_IPP_IME_INTE_BP3                 = 0x00000040,
	CTL_IPP_IME_INTE_TMNR_SLICE_END      = 0x00000080,
	CTL_IPP_IME_INTE_TMNR_MOT_END        = 0x00000100,
	CTL_IPP_IME_INTE_TMNR_MV_END         = 0x00000200,
	CTL_IPP_IME_INTE_TMNR_STA_END        = 0x00000400,
	CTL_IPP_IME_INTE_P1_ENC_OVR          = 0x00000800,
	CTL_IPP_IME_INTE_TMNR_ENC_OVR        = 0x00001000,
	CTL_IPP_IME_INTE_TMNR_DEC_ERR        = 0x00002000,
	CTL_IPP_IME_INTE_FRM_ERR             = 0x00004000,
	CTL_IPP_IME_INTE_FRM_START           = 0x20000000,  ///< frame-start enable
	CTL_IPP_IME_INTE_STRP_END            = 0x40000000,  ///< stripe-end enable
	CTL_IPP_IME_INTE_FMD             	 = 0x80000000,  ///< frame-end enable
} CTL_IPP_IME_INTE_STS;

typedef enum {
	CTL_IPP_IFE2_INTE_LL_END  = 0x00000001,
	CTL_IPP_IFE2_INTE_LL_ERR  = 0x00000002,
	CTL_IPP_IFE2_INTE_LL_LATE = 0x00000004,
	CTL_IPP_IFE2_INTE_OVFL    = 0x40000000,
	CTL_IPP_IFE2_INTE_FMD     = 0x80000000,   ///< enable interrupt: frame end
} CTL_IPP_IFE2_INTE_STS;

/**
	CTL IPP SIE ID (sync with CTL_SIE_ID in ctl_sie_utility.h)
*/
#define CTL_IPP_SIE_ID_UNKNOWN 0xFF
typedef enum _CTL_IPP_SIE_ID {
	CTL_IPP_SIE_ID_1 = 0,	///< process id 0
	CTL_IPP_SIE_ID_2,	    ///< process id 1
	CTL_IPP_SIE_ID_3,	    ///< process id 2
	CTL_IPP_SIE_ID_4,	    ///< process id 3
	CTL_IPP_SIE_ID_5,	    ///< process id 4
	CTL_IPP_SIE_ID_6,	    ///< process id 5
	CTL_IPP_SIE_ID_7,	    ///< process id 6
	CTL_IPP_SIE_ID_8,	    ///< process id 7
	CTL_IPP_SIE_ID_9,	    ///< process id 8
	CTL_IPP_SIE_ID_10,	    ///< process id 9
	CTL_IPP_SIE_ID_11,	    ///< process id 10
	CTL_IPP_SIE_ID_12,	    ///< process id 11
	CTL_IPP_SIE_ID_MAX_NUM,
	ENUM_DUMMY4WORD(CTL_IPP_SIE_ID)
} CTL_IPP_SIE_ID;

#define CTL_IPP_IFE_INTE_STS_ERR_MASK (CTL_IPP_IFE_INTE_DEC1_ERR | CTL_IPP_IFE_INTE_DEC2_ERR | CTL_IPP_IFE_INTE_BUFOVFL | CTL_IPP_IFE_INTE_RING_BUF_ERR | CTL_IPP_IFE_INTE_FRAME_ERR | CTL_IPP_IFE_INTE_LLERR | CTL_IPP_IFE_INTE_LLERR2)
#define CTL_IPP_DCE_INTE_STS_ERR_MASK (CTL_IPP_DCE_INTE_STPERR | CTL_IPP_DCE_INTE_LBOVF | CTL_IPP_DCE_INTE_STPOB | CTL_IPP_DCE_INTE_YBACK | CTL_IPP_DCE_INTE_LL_ERR | CTL_IPP_DCE_INTE_LL_ERR2)
#define CTL_IPP_IPE_INTE_STS_ERR_MASK (CTL_IPP_IPE_INTE_FRAMEERR | CTL_IPP_IPE_INTE_LL_ERR | CTL_IPP_IPE_INTE_LL_ERR2)
#define CTL_IPP_IME_INTE_STS_ERR_MASK (CTL_IPP_IME_INTE_LL_ERR | CTL_IPP_IME_INTE_P1_ENC_OVR | CTL_IPP_IME_INTE_TMNR_ENC_OVR | CTL_IPP_IME_INTE_TMNR_DEC_ERR | CTL_IPP_IME_INTE_FRM_ERR)
#define CTL_IPP_IFE2_INTE_STS_ERR_MASK (CTL_IPP_IFE2_INTE_LL_ERR | CTL_IPP_IFE2_INTE_OVFL)

#endif //_IPP_UTIL_H_
