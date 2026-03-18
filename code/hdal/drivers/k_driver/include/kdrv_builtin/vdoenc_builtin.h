#ifndef _VDOENC_BUILTIN_H
#define _VDOENC_BUILTIN_H

#include "kwrap/type.h"
#include "kdrv_ipp_builtin.h"
#include "sie_init.h"

typedef KDRV_IPP_BUILTIN_FMD_CB_INFO VDOENC_BUILTIN_FMD_INFO;

#define VDOENC_BUILTIN_YUVQ_MAX            18
#define VDOENC_MD_MAP_MAX_SIZE             8196           ///< same value as NMR_VDOENC_MD_MAP_MAX_SIZE
#define VDOENC_BUF_RESERVED_BYTES          4              ///< same value as NMR_VDOENC_BUF_RESERVED_BYTES
#define VDOENC_BUILTIN_JOBQ_MAX            150
#define PRE_ENC_NUM                        16

#define ISP_ID_MAX 5 ///< need to define in sie_init.h

#define VDOENC_BUILTIN_TSK_PRI          3               // task priority, SIE(2) > IPP(3) = VDOENC(3)

#define TNR_DBG 0
#define SPN_DBG 0

#define TNR_ENABLE 0
#define SPN_ENABLE 1

#define FLG_VDOENC_BUILTIN_IDLE         	FLGPTN_BIT(0) //0x00000001
#define FLG_VDOENC_BUILTIN_ENCODE       	FLGPTN_BIT(1) //0x00000002
#define FLG_VDOENC_BUILTIN_STOP         	FLGPTN_BIT(2) //0x00000004
#define FLG_VDOENC_BUILTIN_STOP_DONE    	FLGPTN_BIT(3) //0x00000008
#define FLG_VDOENC_BUILTIN_DONE         	FLGPTN_BIT(4) //0x00000010
#define FLG_VDOENC_BUILTIN_INIT_DONE        FLGPTN_BIT(5) //0x00000020

//frc
#define VDOENC_BUILTIN_FRC_UNIT            100000

#define VDOENC_BUILTIN_SAMPLE_OFF          0 //never trigger
#define VDOENC_BUILTIN_SAMPLE_CAPTURE      1 //trigger n frame, then stop
#define VDOENC_BUILTIN_SAMPLE_PREVIEW      2 //always trigger, do frc
#define VDOENC_BUILTIN_SAMPLE_ALL          3 //always trigger, no frc

typedef enum {
	BUILTIN_VDOENC_H265,
	BUILTIN_VDOENC_H264,
	BUILTIN_VDOENC_MJPEG,
} BUILTIN_VDOENC_CODEC;

typedef enum {
	// module 1
	BUILTIN_VDOENC_INIT_PARAM_ENC_EN,
	BUILTIN_VDOENC_INIT_PARAM_CODEC,
	BUILTIN_VDOENC_INIT_PARAM_DIRECT,
	BUILTIN_VDOENC_INIT_PARAM_WIDTH,
	BUILTIN_VDOENC_INIT_PARAM_HEIGHT,
	BUILTIN_VDOENC_INIT_PARAM_FRAME_RATE,
	BUILTIN_VDOENC_INIT_PARAM_PROFILE,
	BUILTIN_VDOENC_INIT_PARAM_LEVEL_IDC,
	BUILTIN_VDOENC_INIT_PARAM_GOP_NUM,
	BUILTIN_VDOENC_INIT_PARAM_LTR_INTERVAL,
	BUILTIN_VDOENC_INIT_PARAM_LTR_PRE_REF,
	BUILTIN_VDOENC_INIT_PARAM_GRAY_EN,
	BUILTIN_VDOENC_INIT_PARAM_SRC_OUT,
	BUILTIN_VDOENC_INIT_PARAM_SVC,
	BUILTIN_VDOENC_INIT_PARAM_ENTROPY,
	BUILTIN_VDOENC_INIT_PARAM_BSQ_MAX,
	BUILTIN_VDOENC_INIT_PARAM_SEC,
	BUILTIN_VDOENC_INIT_PARAM_JPEG_QUALITY,
	BUILTIN_VDOENC_INIT_PARAM_JPEG_FPS,
	BUILTIN_VDOENC_INIT_PARAM_JPEG_MAX_MEM_SIZE,
	BUILTIN_VDOENC_INIT_PARAM_DYNAMIC_CODEC,
	BUILTIN_VDOENC_INIT_PARAM_BS_START,
	BUILTIN_VDOENC_INIT_PARAM_BS_END,
	BUILTIN_VDOENC_INIT_PARAM_DYNAMIC_BYTERATE,
	BUILTIN_VDOENC_INIT_PARAM_RATE_CONTROL,
} BUILTIN_VDOENC_INIT_PARAM;

typedef enum {
	BUILTIN_VDOENC_DTSI_PARAM_ENC_EN,
	BUILTIN_VDOENC_DTSI_PARAM_CODECTYPE,
	BUILTIN_VDOENC_DTSI_PARAM_SRC_VPRC_DEV,
	BUILTIN_VDOENC_DTSI_PARAM_SRC_VPRC_PATH,
	BUILTIN_VDOENC_DTSI_PARAM_WIDTH,
	BUILTIN_VDOENC_DTSI_PARAM_HEIGHT,
	BUILTIN_VDOENC_DTSI_PARAM_INIT_BYTE_RATE,
	BUILTIN_VDOENC_DTSI_PARAM_FRAMERATE,
	BUILTIN_VDOENC_DTSI_PARAM_SEC,
	BUILTIN_VDOENC_DTSI_PARAM_GOP,
	BUILTIN_VDOENC_DTSI_PARAM_QP,
	BUILTIN_VDOENC_DTSI_PARAM_JPEG_QUALITY,
	BUILTIN_VDOENC_DTSI_PARAM_JPEG_FPS,
	BUILTIN_VDOENC_DTSI_PARAM_SVC_LAYER,
	BUILTIN_VDOENC_DTSI_PARAM_LTR_INTERVAL,
	BUILTIN_VDOENC_DTSI_PARAM_D2D,
	BUILTIN_VDOENC_DTSI_PARAM_GDC,
	BUILTIN_VDOENC_DTSI_PARAM_COLMV,
	BUILTIN_VDOENC_DTSI_PARAM_QUALITY_LV,
	BUILTIN_VDOENC_DTSI_PARAM_ISP_ID,
	BUILTIN_VDOENC_DTSI_PARAM_SRCOUT_SIZE,
	BUILTIN_VDOENC_DTSI_PARAM_RC_MODE,
	BUILTIN_VDOENC_DTSI_PARAM_INIT_IQP,
	BUILTIN_VDOENC_DTSI_PARAM_MIN_IQP,
	BUILTIN_VDOENC_DTSI_PARAM_MAX_IQP,
	BUILTIN_VDOENC_DTSI_PARAM_INIT_PQP,
	BUILTIN_VDOENC_DTSI_PARAM_MIN_PQP,
	BUILTIN_VDOENC_DTSI_PARAM_MAX_PQP,
	BUILTIN_VDOENC_DTSI_PARAM_RC_BYTE_RATE,
	BUILTIN_VDOENC_DTSI_PARAM_STATIC_TIME,
	BUILTIN_VDOENC_DTSI_PARAM_IP_WEIGHT,
	BUILTIN_VDOENC_DTSI_PARAM_KEY_P_PERIOD,
	BUILTIN_VDOENC_DTSI_PARAM_KP_WEIGHT,
	BUILTIN_VDOENC_DTSI_PARAM_P2_WEIGHT,
	BUILTIN_VDOENC_DTSI_PARAM_P3_WEIGHT,
	BUILTIN_VDOENC_DTSI_PARAM_LT_WEIGHT,
	BUILTIN_VDOENC_DTSI_PARAM_MOTION_AQ_STR,
	BUILTIN_VDOENC_DTSI_PARAM_STILL_FRM_CND,
	BUILTIN_VDOENC_DTSI_PARAM_MOTION_RATIO_THD,
	BUILTIN_VDOENC_DTSI_PARAM_I_PSNR_CND,
	BUILTIN_VDOENC_DTSI_PARAM_P_PSNR_CND,
	BUILTIN_VDOENC_DTSI_PARAM_KP_PSNR_CND,
	BUILTIN_VDOENC_DTSI_PARAM_CHANGE_POSITION,
	BUILTIN_VDOENC_DTSI_PARAM_SVC_WEIGHT_MODE,
	BUILTIN_VDOENC_DTSI_PARAM_ROTATE,
	BUILTIN_VDOENC_DTSI_PARAM_SKIP_FRM_EN,
	BUILTIN_VDOENC_DTSI_PARAM_SKIP_FRM_TARGET_FR,
	BUILTIN_VDOENC_DTSI_PARAM_SKIP_FRM_INPUT_CNT,
	BUILTIN_VDOENC_DTSI_PARAM_YCC_EN,
	BUILTIN_VDOENC_DTSI_PARAM_YCC_WIDTH,
	BUILTIN_VDOENC_DTSI_PARAM_YCC_HEIGHT,
	BUILTIN_VDOENC_DTSI_PARAM_YCC_YLOFF,
	BUILTIN_VDOENC_DTSI_PARAM_YCC_CLOFF,
	BUILTIN_VDOENC_DTSI_PARAM_FRC_SAMPLE_MODE,
	BUILTIN_VDOENC_DTSI_PARAM_FRC_RATE,
	BUILTIN_VDOENC_DTSI_PARAM_FRC_FRAMEPERSEC,
	BUILTIN_VDOENC_DTSI_PARAM_FRC_FRAMEPERSEC_NEW
} BUILTIN_VDOENC_DTSI_PARAM;

typedef enum {
	BUILTIN_VDOENC_PATH_ID_0 = 0,
	BUILTIN_VDOENC_PATH_ID_1 = 1,
	BUILTIN_VDOENC_PATH_ID_2 = 2,
	BUILTIN_VDOENC_PATH_ID_3 = 3,
	BUILTIN_VDOENC_PATH_ID_4 = 4,
	BUILTIN_VDOENC_PATH_ID_5 = 5,
	BUILTIN_VDOENC_PATH_ID_MAX = 6,
} BUILTIN_VDOENC_PATH_ID;

typedef enum {
	BUILTIN_VDOENC_ISP_ITEM_TNR   = 0,      // support get/set, using KDRV_VDOENC_3DNR struct
	BUILTIN_VDOENC_ISP_ITEM_RATIO = 1,      // support get    , using KDRV_VDOENC_ISP_RATIO struct
	BUILTIN_VDOENC_ISP_ITEM_SPN   = 2,      // support get/set, using KDRV_H26XENC_SPN struct
} BUILTIN_VDOENC_ISP_ITEM;

typedef enum {
	BUILTIN_ISP_EVENT_NONE					= 0x00000000,
	BUILTIN_ISP_EVENT_ENC_TNR				= 0x00400000,   ///< ENC 3DNR
	BUILTIN_ISP_EVENT_ENC_SHARPEN			= 0x01000000,	///< ENC Sharpen
} BUILTIN_ISP_EVENT;

typedef enum {
	BUILTIN_VDOENC_RC_CBR = 1,
	BUILTIN_VDOENC_RC_VBR,
	BUILTIN_VDOENC_RC_VBR2,
	BUILTIN_VDOENC_RC_FIXQP,
	BUILTIN_VDOENC_RC_EVBR,
	BUILTIN_VDOENC_RC_RC_NOT_SUPPORT,
} BUILTIN_VDOENC_RC;

typedef struct {
	/* ime output buffer info */
	UINT32 width;
	UINT32 height;
	UINT32 fmt;

	/* venc internal buffer info */
	uintptr_t max_blk_addr;
	UINT32 max_blk_size;
} VDOENC_BUILTIN_INIT_INFO;

typedef struct {
	uintptr_t codec_mem_addr;
	UINT32 codec_mem_size;
	uintptr_t bs_start_addr;                               ///< [w] output bit-stream start address if enc space not enough
	uintptr_t bs_addr_1;                                   ///< [w] 1st output bit-stream address
	UINT32 bs_size_1;                                   ///< [w] 1st output bit-stream size
	uintptr_t bs_end_addr;                                 ///< [w] output bit-stream end address
	UINT32 bs_min_i_size;
	UINT32 bs_min_p_size;
	UINT32 interrupt;
	UINT32 timestamp;
	UINT32 base_qp;
	UINT32 frm_type;
} VDOENC_BUILTIN_PARAM;

typedef struct {
	BOOL   enable;
	uintptr_t y_addr;
	uintptr_t c_addr;
	UINT32 width;
	UINT32 height;
	UINT32 y_line_offset;
	UINT32 c_line_offset;
	UINT32 timestamp;
	BOOL   release_flag;
	BOOL   isRtosYUV;
} VDOENC_BUILTIN_YUV_INFO;

typedef struct {
	uintptr_t Addr;    							//BS buffer starting address
	UINT32 Size;    							//BS buffer size
	UINT32 temproal_id;
	UINT32 re_encode_en;
	UINT32 timestamp;
	UINT32 nxt_frm_type;
	UINT32 base_qp;
	UINT32 bs_size_1;
	UINT32 frm_type;
	UINT32 encode_time;
	BOOL isKeyFrame;
	BOOL isRtosBS;
} VDOENC_BUILTIN_BS_INFO, *PVDOENC_BUILTIN_BS_INFO;

typedef struct {
	UINT32 pathID;
} VDOENC_BUILTIN_JOB_INFO, *PVDOENC_BUILTIN_JOB_INFO;

typedef struct {
	UINT32                     	BeginTag;
	UINT32                      Front;                  ///< Front pointer
	UINT32                      Rear;                   ///< Rear pointer
	UINT32                      bFull;                  ///< Full flag
	VDOENC_BUILTIN_YUV_INFO     *Queue;
	UINT32                     	EndTag;
} VDOENC_BUILTIN_YUVQ, *PVDOENC_BUILTIN_YUVQ;

typedef struct {
	UINT32                     	BeginTag;
	UINT32                      Front;                  ///< Front pointer
	UINT32                      Rear;                   ///< Rear pointer
	UINT32                      bFull;                  ///< Full flag
	VDOENC_BUILTIN_BS_INFO      *Queue;
	UINT32                     	EndTag;
} VDOENC_BUILTIN_BSQ, *PVDOENC_BUILTIN_BSQ;

typedef struct {
	UINT32                      sample_mode;
	UINT32                      rate;
	UINT32                      frm_counter;
	UINT32                      rate_counter;
	UINT32                      output_counter;
	UINT32                      framepersecond;
	UINT32                      framepersecond_new;
} VDOENC_BUILTIN_FRC;

typedef struct {
	UINT32                        BeginTag;
	VDOENC_BUILTIN_YUVQ           yuvQueue;
	VDOENC_BUILTIN_BSQ            bsQueue;
	VDOENC_BUILTIN_PARAM          venc_param;
	VDOENC_BUILTIN_FRC            frc;
	UINT32                     	  EndTag;
} VDOENC_BUILTIN_OBJ, *PVDOENC_BUILTIN_OBJ;

typedef struct {
	UINT32                      Front;                  ///< Front pointer
	UINT32                      Rear;                   ///< Rear pointer
	UINT32                      bFull;                  ///< Full flag
	VDOENC_BUILTIN_JOB_INFO     *Queue;
} VDOENC_BUILTIN_JOBQ, *PVDOENC_BUILTIN_JOBQ;

typedef struct
{
    unsigned int uiEncId;
    unsigned int uiRCMode;
    unsigned int uiPicSize;  // resolution
    //unsigned int uiCtuSize;
    unsigned int uiInitIQp;
    unsigned int uiMinIQp;
    unsigned int uiMaxIQp;
    unsigned int uiInitPQp;
    unsigned int uiMinPQp;
    unsigned int uiMaxPQp;
    unsigned int uiBitRate;
    unsigned int uiFrameRateBase;
    unsigned int uiFrameRateIncr; // frame rate = uiFrameRateBase / uiFrameRateIncr
    unsigned int uiGOP;
    //int          iIPQPOffset;
    unsigned int uiRowLevelRCEnable;
    unsigned int uiStaticTime;
    unsigned int uiChangePos;
    unsigned int uiFixIQp;
    unsigned int uiFixPQp;
    unsigned int uiKeyPPeriod; // 0: disable, 1: frame rate, others: r frame period
    int          iIPWeight;
    int          iKPWeight;

	int          iP2Weight;
	int          iP3Weight;
	int          HP_period; // SVC weight update period
	unsigned int uiSvcBAMode;	// SVC mode: 0: weight = qp (origin SVC RC), 1: weight = bit size
	int          iLTWeight;

    int          iMotionAQStrength;
    unsigned int uiStillFrameCnd;
    unsigned int uiMotionRatioThd;
    unsigned int uiIPsnrCnd;
    unsigned int uiPPsnrCnd;
    unsigned int uiKeyPPsnrCnd;
    unsigned int uiLTRInterval;
    unsigned int uiSVCLayer;
	unsigned int uiMinStillPercent;
	unsigned int uiMinStillIQp;
	unsigned int uiMaxFrameSize;
	unsigned int uiBRTolerance;
}VDOENC_BUILTIN_RC_PARAM;

typedef struct {
	BOOL  bEnable;			///< [r/w] Enable sharpen
	UINT8 ucConEng; 		///< [r/w] The weight of the local contrast  0~8
	UINT16 usSlopConEng;	///< [r/w] Transition region slop 0~4095
	UINT8 ucBHC;			///< [r/w] Bright halo clip ratio 0~128
	UINT8 ucDHC;			///< [r/w] Dark halo clip ratio 0~128
	UINT8 ucEWT;			///< [r/w] Edge weight coring threshold 0~255
	UINT8 ucEWG;			///< [r/w] Edge weight gain 0~255
	UINT8 ucEdgeSharpStr1;	///< [r/w] Sharpen strength 1 of edge region 0~255
	UINT8 ucCT; 			///< [r/w] Coring threshold 0~255
	UINT8 ucNL; 			///< [r/w] Noise level 0~255
	UINT8 ucBIG;			///< [r/w] Blending ratio of HPF results 0~255
	UINT16 usFlatTh;		///< [r/w] flat region threshold 0~2047
	UINT16 usEdgeTh;		///< [r/w] Edge region threshold 0~2047
	UINT8 ucEdgeStr;		///< [r/w] Edge weight strength 0~255
	UINT8 ucTransitionStr;	///< [r/w] Transition region weight strength 0~255
	UINT8 ucMotionStr;		///< [r/w] Motion region edge weight 0~255
	UINT8 ucStaticStr;		///< [r/w] Static region edge weight strength 0~255
	UINT8 ucFlatStr;		///< [r/w] Flag region weight strength 0~255
	UINT8 ucNC[17]; 		///< [r/w] 17 control points of noise modulation curve 0~255
	UINT16 usEWG[9];		///< [r/w] 9 control points of EWG
	BOOL bShowSharpInfo;	///< [r/w] Show debug mode info(0: normal, 1: regionclass)
} VDOENC_BUILTIN_H26XENC_SPN;

typedef struct {
	UINT32 codectype;
	INT32  vprc_src_dev;
	INT32  vprc_src_path;
	UINT32 width;
	UINT32 height;
	UINT32 isp_id;
	BOOL   skip_frm_en;
	UINT32 skip_frm_target_fr;
	UINT32 skip_frm_input_cnt;
	BOOL   ycc_en;
	UINT32 ycc_width;
	UINT32 ycc_height;
	UINT32 ycc_yloff;
	UINT32 ycc_cloff;
} VDOENC_BUILTIN_DTSI_PARAM;

typedef void (*VDOENC_BUILTIN_START_ENC_CB)(VDOENC_BUILTIN_YUV_INFO *p_info, UINT32 reserved);
INT32 vdoenc_builtin_reg_start_enc_cb(VDOENC_BUILTIN_START_ENC_CB fp);

typedef void (*VDOENC_BUILTIN_RESULT_CB)(UINT32 reserved);
INT32 vdoenc_builtin_reg_result_cb(VDOENC_BUILTIN_RESULT_CB fp);

int vdoenc_builtin_get_of_node_venc(void);
int VdoEnc_builtin_get_dtsi_param(UINT32 pathID, BUILTIN_VDOENC_DTSI_PARAM param);
int  H264Enc_builtin_init(UINT32 pathID, VDOENC_BUILTIN_INIT_INFO *p_info);
int  H265Enc_builtin_init(UINT32 pathID, VDOENC_BUILTIN_INIT_INFO *p_info);
int  MJPGEnc_builtin_init(UINT32 pathID, VDOENC_BUILTIN_INIT_INFO *p_info);
BOOL _VdoEnc_Builtin_PutYuv(UINT32 pathID, VDOENC_BUILTIN_YUV_INFO *pYuvInfo);
BOOL _VdoEnc_builtin_PutJob_H26X(UINT32 pathID);
BOOL _VdoEnc_builtin_PutJob_JPEG(UINT32 pathID);
void _VdoEnc_builtin_Get_SrcYuv_Info(VDOENC_BUILTIN_FMD_INFO *pSrc, VDOENC_BUILTIN_YUV_INFO *pDst, UINT32 ipp_pathID);
extern void (*VdoEnc_BuiltIn_trig)(VDOENC_BUILTIN_FMD_INFO *p_info, UINT32 reserved);

int VdoEnc_BuiltIn_TskStart_H26X(void);
int VdoEnc_BuiltIn_TskStop_H26X(void);
int VdoEnc_BuiltIn_TskStart_JPEG(void);
int VdoEnc_BuiltIn_TskStop_JPEG(void);
void VdoEnc_BuiltIn_Install_ID(void);
void VdoEnc_BuiltIn_Uninstall_ID(void);
int  VdoEnc_builtin_init(VDOENC_BUILTIN_INIT_INFO *p_info);
void* vdoenc_builtin_alloc(int size);
void vdoenc_builtin_free(void *buf);
#if defined(__FREERTOS)
int  VdoEnc_builtin_init_lite(VDOENC_BUILTIN_INIT_INFO *p_info);
#endif


extern BOOL VdoEnc_Builtin_GetBS(UINT32 pathID, VDOENC_BUILTIN_BS_INFO *builtin_bs_info);
extern UINT32 VdoEnc_Builtin_HowManyInBSQ(UINT32 pathID);
extern BOOL VdoEnc_Builtin_GetEncVar(UINT32 pathID, void *kdrv_vdoenc_var);
extern UINT32 VdoEnc_Builtin_SetParam(UINT32 pathID, UINT32 Param, ULONG Value);
extern UINT32 VdoEnc_Builtin_GetParam(UINT32 pathID, UINT32 Param, VOID *pValue);
extern UINT32 VdoEnc_Builtin_CheckBuiltinStop(UINT32 pathID);
extern VOID VdoEnc_Builtin_FreeQueMem(UINT32 pathID);
extern BOOL vdoenc_builtin_evt_fp_reg(CHAR *name, SIE_FB_ISR_FP fp);
extern BOOL vdoenc_builtin_evt_fp_unreg(CHAR *name);
extern BOOL vdoenc_builtin_set(UINT32 id, BUILTIN_VDOENC_ISP_ITEM item, void *data);
extern VOID vdoenc_builtin_set_pre_init_flag(BOOL isPreInit);
extern VOID vdoenc_builtin_set_pre_num(UINT32 pathID, ULONG pre_num);
extern VOID vdoenc_builtin_get_pre_num(UINT32 pathID, VOID *pre_num);
extern VOID vdoenc_builtin_set_en_lnx_h26x_int_first(ULONG enable);
extern VOID vdoenc_builtin_get_en_lnx_h26x_int_first(VOID *enable);
extern VOID vdoenc_builtin_set_enc_obj_addr(ULONG enc_obj_addr);
extern VOID vdoenc_builtin_get_enc_obj_addr(VOID *enc_obj_addr);
extern VOID vdoenc_builtin_set_enc_var_addr(ULONG enc_var_addr);
extern VOID vdoenc_builtin_get_enc_var_addr(VOID *enc_var_addr);
extern VOID vdoenc_builtIn_disable_h26x_int(VOID);
#endif //_VDOENC_BUILTIN_H
