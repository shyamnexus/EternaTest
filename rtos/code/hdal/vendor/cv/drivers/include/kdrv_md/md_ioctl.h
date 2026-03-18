#ifndef __MD_IOCTL_CMD_H_
#define __MD_IOCTL_CMD_H_

#include "kwrap/ioctl.h"
#include <kwrap/nvt_type.h>

#define NEW_MD_IOCTL_IF 1

#if defined (__FREERTOS)
#define NVTMPP_OPEN(...) 0
#define NVTMPP_IOCTL nvtmpp_ioctl
#define NVTMPP_CLOSE(...)

#define NVTMD_OPEN(...) 0
#define NVTMD_IOCTL nvt_md_ioctl
#define NVTMD_CLOSE(...)
#else
#define NVTMPP_OPEN  open
#define NVTMPP_IOCTL ioctl
#define NVTMPP_CLOSE close

#define NVTMD_OPEN  open
#define NVTMD_IOCTL ioctl
#define NVTMD_CLOSE close
#endif

/* job status */
enum md_drv_sts {
    MD_DRV_STS_IDLE = 0,
    MD_DRV_STS_QUEUE,     // not process yet, in job list
    MD_DRV_STS_PROC,		// ready to process
    MD_DRV_STS_PROC_END,	// process done
    MD_DRV_STS_DONE,		// done by hw process
    MD_DRV_STS_FLUSH,     // stop by driver(not done)
    MD_DRV_STS_ERROR,		// error by driver(not done)
    MD_DRV_STS_OK,
	MD_DRV_STS_QFULL,
    MD_DRV_STS_MAX
};

// Structure definition
/**
    MDBC Engine Operation Selection.

    Select MDBC module operation.
*/
//@{
typedef enum {
	DRV_INIT          = 0,   ///< init mode
	DRV_NORM          = 1,   ///< normal mode
	ENUM_DUMMY4WORD(MDBC_MODE)
} MDBC_MODE;
//@}


/**
    MDBC Engine TYPE.

    Select MDBC module operation.
*/
//@{
typedef enum {
	DRV_MDBC              = 0,   ///< init mode
	ENUM_DUMMY4WORD(MDBC_TYPE)
} MDBC_TYPE;

/**
    MDBC Morphological process Sel.

    Select MDBC Morphological process type.
*/
//@{
typedef enum {
	DRV_Median         = 0,   ///< Median
	DRV_Dilate         = 1,   ///< Dilate
	DRV_Erode          = 2,   ///< Erode
	DRV_Bypass         = 3,   ///< Bypass
	ENUM_DUMMY4WORD(MDBC_MOR_SEL)
} MDBC_MOR_SEL;
//@}


/**
    MDBC Open Object.

    Open Object is used for opening MDBC module.
    \n Used for mdbc_open()

*/
//@{
typedef struct _MDBC_OPENOBJ {
	void (*FP_MDBCISR_CB)(UINT32 uiIntStatus); ///< isr callback function
	UINT32 uiMdbcClockSel;                     ///< MDBC clock selection
} MDBC_OPENOBJ;
//@}

/**
    MDBC Input Info.
*/
//@{
typedef struct {
	UINT8      update_nei_en;  	///< update neighbor enable
	UINT8      deghost_en;     	///< deghost enable
	UINT8      roi_en;        	///< roi 0~7 enable
	UINT8      chksum_en;      	///< check sum enable
	UINT8      bgmw_save_bw_en;	///< bandwidth saving enable
	UINT8      bc_y_only_en;	/// 0:YUV420, 1:Y only
	MDBC_TYPE  md_type;         ///< GMM or MD (530-add)
	UINT8      bc_model_mode;   ///<Background model format 0:YUV444, 1:YUV420
	UINT8	   out_bit_depth;   ///<0:1 bit, 1:8 bits
	UINT8	   md_lbsp_disable; ///<0:enable, 1:disable
} md_drv_ctrl_en;
//@}

/**
    MDBC Input Info.
*/
//@{
typedef struct {
	uintptr_t      uiInAddr0;   ///< Y address
	uintptr_t      uiInAddr1;   ///< UV address
	uintptr_t      uiInAddr2;   ///< last UV address
	uintptr_t      uiInAddr3;   ///< bgmodel address
	uintptr_t      uiInAddr4;   ///< var1 address
	uintptr_t      uiInAddr5;   ///< var2 address
	UINT32      uiLofs0;     ///< Y line-offset
	UINT32      uiLofs1;     ///< UV line-offset
} md_drv_input_info;
//@}

/**
    MDBC Output Info.
*/
//@{
typedef struct {
	uintptr_t      uiOutAddr0;  ///< foreground address
	uintptr_t      uiOutAddr1;  ///< bgmodel address
	uintptr_t      uiOutAddr2;  ///< var1 address
	uintptr_t      uiOutAddr3;  ///< var2 address
} md_drv_output_info;
//@}

/**
    MDBC Model Match Parameters.
*/
//@{
typedef struct {
	UINT8 lbsp_th;                         ///< Lbsp threshold
	UINT8 d_colour;                        ///< D colour
	UINT8 r_colour;                        ///< R colour
	UINT8 d_lbsp;                          ///< D lbsp
	UINT8 r_lbsp;                          ///< R lbsp
	UINT8 model_num;                       ///< bg model number
	UINT8 t_alpha;                         ///< T_ALPHA
	UINT8 dw_shift;                        ///< DE_SHIFT
	UINT16 dlast_alpha;                    ///< D_LAST_ALPHA
	UINT8 min_match;                       ///< min match
	UINT16 dlt_alpha;                      ///< DLT_ALPHA
	UINT16 dst_alpha;                      ///< DST_ALPHA
	UINT8 uv_thres;                        ///< UV threshold
	UINT16 s_alpha;                        ///< S_ALPHA
	UINT32 dbg_lumDiff;                    ///< debug Lum Diff
	UINT8 dbg_lumDiff_en;                  ///< debug Lum Diff enable
	UINT8 model_num_ext;				   ///< Extend Background model number to 16
} md_drv_mdmatch_param;


//@}

/**
    MDBC Morphological process Parameters.
*/
//@{

typedef struct {
	UINT8 mor_th0;                         ///< mor0 threshold
	UINT8 mor_th1;                         ///< mor1 threshold
	UINT8 mor_th2;                         ///< mor2 threshold
	UINT8 mor_th3;                         ///< mor3 threshold
	UINT8 mor_th_dil;                      ///< mor dil threshold
    UINT8 mor_sel0;                        ///< Morphological type
    UINT8 mor_sel1;                        ///< Morphological type
    UINT8 mor_sel2;                        ///< Morphological type
    UINT8 mor_sel3;                        ///< Morphological type
    UINT8 bc_temporal_th;				   ///< temporal filter
} md_drv_mor_param;

//@}

/**
    MDBC Update Parameters.
*/
//@{

typedef struct {
	UINT8 minT;                      ///< Update min T
	UINT8 maxT;                      ///< Update max T
	UINT8 maxFgFrm;                  ///< Update T when Max foreground frame
	UINT16 deghost_dth;              ///< Deghost threshold for Dlast
	UINT8 deghost_sth;               ///< Deghost threshold for S1
	UINT8 stable_frm;                ///< stable frame
	UINT8 update_dyn;                ///< update dyn
	UINT8 va_distth;                 ///< Va distance threshold
	UINT8 t_distth;                  ///< T distance threshold
	UINT8 dbg_frmID;                 ///< debug frame ID
	UINT8 dbg_frmID_en;              ///< debug frame ID enable
	UINT16 dbg_rnd;                  ///< debug random
	UINT8 dbg_rnd_en;                ///< debug random enable
} md_drv_upd_param;

//@}

/**
    MDBC ROI Parameters.
*/
//@{

typedef struct {

	UINT16 roi_x;                     ///< roi x position
	UINT16 roi_y;                     ///< roi y position
	UINT16 roi_w;                     ///< roi width
	UINT16 roi_h;                     ///< roi height
	UINT8 roi_uv_thres;               ///< roi UV threshold
	UINT8 roi_lbsp_th;                ///< roi Lbsp threshold
	UINT8 roi_d_colour;               ///< roi D colour
	UINT8 roi_r_colour;               ///< roi R colour
	UINT8 roi_d_lbsp;                 ///< roi D lbsp
	UINT8 roi_r_lbsp;                 ///< roi R lbsp
	UINT8 roi_morph_en;               ///< roi morphological process enable
	UINT8 roi_minT;                   ///< Update roi min T
	UINT8 roi_maxT;                   ///< Update roi max T

} md_drv_roi_param;

//@}

/**
    MDBC structure - MDBC input size info.
*/
//@{
typedef struct {
	UINT16 width;              ///< MDBC input width
	UINT16 height;             ///< MDBC input Height
} md_drv_in_size;
//@}

/**
    MDBC All Parameters.

    This is used for MDBC setmode.

*/
#define MD_LV_MAX 4
#define MD_ROI_MAX 8
typedef struct _md_param_cfg {
	MDBC_MODE                   mode;          ///< MDBC operation mode
	md_drv_ctrl_en              ctrl_en;     ///< MDBC control enable
    md_drv_input_info           in_info;        ///< MDBC input address & lofs
    md_drv_output_info          out_info;       ///< MDBC output address
	md_drv_in_size              size;          ///< MDBC size
    md_drv_mdmatch_param        mdmatch_para;   ///< MDBC Model Match
    md_drv_mor_param            mor_para;       ///< MDBC Morphological process
    md_drv_upd_param            upd_para;       ///< MDBC Update
    md_drv_roi_param            roi_para[MD_ROI_MAX];      ///< MDBC ROI0 ~ 7
	UINT32                      intrpt_en;       ///< MDBC Interrupt enable
} md_param_cfg;

typedef struct _md_drv_process_cfg {
//----------set------------------------
	UINT32 chip;
	UINT32 eng;
	UINT32 id;
    BOOL instant; // 0: non-blocking 1: blocking
    UINT8 priority;
    void* job_cfg;   //md_param_cfg, if not have next job, set this to null
//----------get------------------------
    unsigned int handle;
}md_drv_process_cfg;

struct md_drv_rlt_info {
	UINT32 lum_diff;     ///< MDBC LumDiff
	UINT32 frm_id;       ///< MDBC FrmID
	UINT32 rnd;      	///< MDBC Rnd
};

typedef struct _md_drv_query_cfg {
    unsigned int handle;
    BOOL instant; // 0: non-blocking 1: blocking
    BOOL job_finish;

	struct md_drv_rlt_info rlt;
}md_drv_query_cfg;

//============================================================================
// IOCTL command
//============================================================================
#define MD_IOC_COMMON_TYPE 'M'
#if !NEW_MD_IOCTL_IF
#define MD_IOC_START                   _VOS_IO(MD_IOC_COMMON_TYPE, 1)
#define MD_IOC_STOP                    _VOS_IO(MD_IOC_COMMON_TYPE, 2)

#define MD_IOC_READ_REG                _VOS_IOWR(MD_IOC_COMMON_TYPE, 3, void*)
#define MD_IOC_WRITE_REG               _VOS_IOWR(MD_IOC_COMMON_TYPE, 4, void*)
#define MD_IOC_READ_REG_LIST           _VOS_IOWR(MD_IOC_COMMON_TYPE, 5, void*)
#define MD_IOC_WRITE_REG_LIST          _VOS_IOWR(MD_IOC_COMMON_TYPE, 6, void*)

#define MD_IOC_OPEN                     _VOS_IOWR(MD_IOC_COMMON_TYPE,  7, void*)
#define MD_IOC_CLOSE                    _VOS_IOWR(MD_IOC_COMMON_TYPE,  8, void*)
#define MD_IOC_OPENCFG                  _VOS_IOWR(MD_IOC_COMMON_TYPE,  9, void*)

#define MD_IOC_SET_PARAM                _VOS_IOWR(MD_IOC_COMMON_TYPE,  10, void*)
#define MD_IOC_GET_PARAM                _VOS_IOWR(MD_IOC_COMMON_TYPE,  11, void*)
#define MD_IOC_TRIGGER                  _VOS_IOWR(MD_IOC_COMMON_TYPE,  12, void*)
#define MD_IOC_GET_REG                  _VOS_IOWR(MD_IOC_COMMON_TYPE,  13, void*)
#define MD_IOC_SET_DMA_ABORT			_VOS_IOWR(MD_IOC_COMMON_TYPE,  14, void*)
#define MD_IOC_GET_DMA_ABORT			_VOS_IOWR(MD_IOC_COMMON_TYPE,  15, void*)
#define MD_IOC_INIT                     _VOS_IOWR(MD_IOC_COMMON_TYPE,  16, void*)
#define MD_IOC_UNINIT                   _VOS_IOWR(MD_IOC_COMMON_TYPE,  17, void*)
#define MD_IOC_GET_VER                  _VOS_IOWR(MD_IOC_COMMON_TYPE,  18, void*)
#else
#define MD_IOC_GET_VER               	_VOS_IOWR(MD_IOC_COMMON_TYPE, 56, void*)
#define MD_IOC_TRIGGER_JOB              _VOS_IOWR(MD_IOC_COMMON_TYPE, 60, md_drv_process_cfg *)
#define MD_IOC_QUERY_JOB                _VOS_IOWR(MD_IOC_COMMON_TYPE, 61, md_drv_query_cfg *)
#endif

/* Add other command ID here*/
#if defined(__FREERTOS)
int nvt_md_ioctl(int fd, unsigned int uiCmd, void *p_arg);
extern INT32 kdrv_md_rtos_init(void);
extern INT32 kdrv_md_rtos_uninit(void);
#endif


#endif
