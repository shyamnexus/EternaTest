/**
    Public header file for dal_md

    This file is the header file that defines the API and data types for vendor_md.

    @file       vendor_md.h
    @ingroup    vendor_md

    Copyright   Novatek Microelectronics Corp. 2019.    All rights reserved.
*/
#ifndef _VENDOR_MD_H_
#define _VENDOR_MD_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"

/********************************************************************
	TYPE DEFINITION
********************************************************************/

/**
    KDRV MD structure - trigger parameter
*/
typedef struct _VENDOR_MD_TRIGGER_PARAM {
	BOOL    is_nonblock;
	UINT32	time_out_ms;	///< set 0 for disable time out
} VENDOR_MD_TRIGGER_PARAM;

// Structure definition
/**
    MDBC Engine Operation Selection.

    Select MDBC module operation.
*/
//@{
typedef enum {
	KDRV_INIT              = 0,   ///< init mode
	KDRV_NORM              = 1,   ///< normal mode
	ENUM_DUMMY4WORD(VENDOR_MDBC_MODE)
} VENDOR_MDBC_MODE;
//@}

/**
    MDBC Morphological process Sel.

    Select MDBC Morphological process type.
*/
//@{
typedef enum {
	KDRV_Median         = 0,   ///< Median
	KDRV_Dilate         = 1,   ///< Dilate
	KDRV_Erode          = 2,   ///< Erode
	KDRV_Bypass         = 3,   ///< Bypass
	ENUM_DUMMY4WORD(VENDOR_MDBC_MOR_SEL)
} VENDOR_MDBC_MOR_SEL;
//@}

typedef enum {
	VENDOR_MDBC_ABORT_ENABLE = 0,
	VENDOR_MDBC_ABORT_DISABLE = 1,
	ENUM_DUMMY4WORD(VENDOR_MDBC_ABORT)
} VENDOR_MDBC_ABORT;


/**
    MDBC Input Info.
*/
//@{
typedef struct {
	UINT32      update_nei_en;  ///< update neighbor enable
	UINT32      deghost_en;     ///< deghost enable
	UINT32      roi_en0;        ///< roi0 enable
	UINT32      roi_en1;        ///< roi1 enable
	UINT32      roi_en2;        ///< roi2 enable
	UINT32      roi_en3;        ///< roi3 enable
	UINT32      roi_en4;        ///< roi4 enable
	UINT32      roi_en5;        ///< roi5 enable
	UINT32      roi_en6;        ///< roi6 enable
	UINT32      roi_en7;        ///< roi7 enable
	UINT32      chksum_en;      ///< check sum enable
	UINT32      bgmw_save_bw_en;///< bandwidth saving enable
	UINT32      bc_y_only_en;
	UINT32      md_type;         ///< GMM or MD (530-add)
	//UINT32      gmm_evt_out_sel; ///< (530-add)
	//UINT32      gmm_clr;	     ///< (530-add)
	UINT32      bc_model_mode;	 ///< (530-add)
	UINT32      md_lbsp_disable; ///< (539A-add)
} VENDOR_MDBC_CONTROL_EN;
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
} VENDOR_MDBC_INPUT_INFO;
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
} VENDOR_MDBC_OUTPUT_INFO;
//@}

/**
    MDBC Model Match Parameters.
*/
//@{
typedef struct _VENDOR_MDBC_MDMATCH_PARAM {
	UINT32 lbsp_th;                         ///< Lbsp threshold
	UINT32 d_colour;                        ///< D colour
	UINT32 r_colour;                        ///< R colour
	UINT32 d_lbsp;                          ///< D lbsp
	UINT32 r_lbsp;                          ///< R lbsp
	UINT32 model_num;                       ///< bg model number
	UINT32 t_alpha;                         ///< T_ALPHA
	UINT32 dw_shift;                        ///< DE_SHIFT
	UINT32 dlast_alpha;                     ///< D_LAST_ALPHA
	UINT32 min_match;                       ///< min match
	UINT32 dlt_alpha;                       ///< DLT_ALPHA
	UINT32 dst_alpha;                       ///< DST_ALPHA
	UINT32 uv_thres;                        ///< UV threshold
	UINT32 s_alpha;                         ///< S_ALPHA
	UINT32 dbg_lumDiff;                     ///< debug Lum Diff
	UINT32 dbg_lumDiff_en;                  ///< debug Lum Diff enable
	UINT32 model_num_ext;                   ///< (539A-add)
} VENDOR_MDBC_MDMATCH_PARAM;
//@}

/**
    MDBC Morphological process Parameters.
*/
//@{
typedef struct _VENDOR_MDBC_MOR_PARAM {
	UINT32 mor_th0;                         ///< mor0 threshold
	UINT32 mor_th1;                         ///< mor1 threshold
	UINT32 mor_th2;                         ///< mor2 threshold
	UINT32 mor_th3;                         ///< mor3 threshold
	UINT32 mor_th_dil;                      ///< mor dil threshold
    UINT32 mor_sel0;                        ///< Morphological type
    UINT32 mor_sel1;                        ///< Morphological type
    UINT32 mor_sel2;                        ///< Morphological type
    UINT32 mor_sel3;                        ///< Morphological type
    UINT32 bc_temporal_th;				    ///< temporal filter
} VENDOR_MDBC_MOR_PARAM;
//@}

/**
    MDBC Update Parameters.
*/
//@{
typedef struct _VENDOR_MDBC_UPD_PARAM {
	UINT32 minT;                      ///< Update min T
	UINT32 maxT;                      ///< Update max T
	UINT32 maxFgFrm;                  ///< Update T when Max foreground frame
	UINT32 deghost_dth;               ///< Deghost threshold for Dlast
	UINT32 deghost_sth;               ///< Deghost threshold for S1
	UINT32 stable_frm;                ///< stable frame
	UINT32 update_dyn;                ///< update dyn
	UINT32 va_distth;                 ///< Va distance threshold
	UINT32 t_distth;                  ///< T distance threshold
	UINT32 dbg_frmID;                 ///< debug frame ID
	UINT32 dbg_frmID_en;              ///< debug frame ID enable
	UINT32 dbg_rnd;                   ///< debug random
	UINT32 dbg_rnd_en;                ///< debug random enable
} VENDOR_MDBC_UPD_PARAM;
//@}

/**
    MDBC ROI Parameters.
*/
//@{
typedef struct _VENDOR_MDBC_ROI_PARAM {
	UINT32 roi_x;                     ///< roi x position
	UINT32 roi_y;                     ///< roi y position
	UINT32 roi_w;                     ///< roi width
	UINT32 roi_h;                     ///< roi height
	UINT32 roi_uv_thres;              ///< roi UV threshold
	UINT32 roi_lbsp_th;               ///< roi Lbsp threshold
	UINT32 roi_d_colour;              ///< roi D colour
	UINT32 roi_r_colour;              ///< roi R colour
	UINT32 roi_d_lbsp;                ///< roi D lbsp
	UINT32 roi_r_lbsp;                ///< roi R lbsp
	UINT32 roi_morph_en;              ///< roi morphological process enable
	UINT32 roi_minT;                  ///< Update roi min T
	UINT32 roi_maxT;                  ///< Update roi max T
} VENDOR_MDBC_ROI_PARAM;
//@}

/**
    MDBC structure - MDBC input size info.
*/
//@{
typedef struct {
	UINT32 uiMdbcWidth;              ///< MDBC input width
	UINT32 uiMdbcHeight;             ///< MDBC input Height
} VENDOR_MDBC_IN_SIZE;
//@}

/**
    GMM Input Info.
*/
//@{
/*	
typedef struct {
	UINT32      MD_TBG ;  			///< TB
	UINT32      MD2_UPD_ON;     	///< MD2 enable
	UINT32      MD_TAMPER_TYPE;     ///< Tamper type
	UINT32      MD_SCENE_CHANGE_TH; ///< Scene change threshold
    UINT32      MD_TAMPER_COUNT;    ///< tamper count
	UINT32	    MD_SCENE_CHANGE_COUNT; ///< scene change count
} VENDOR_GMM_CONTROL_INFO;
*/
//@}

/**
    GMM TEMPER Info.
*/
//@{
/*	
typedef struct {
	UINT32      MD_TAMPER_EDG_TEX ;  			///< update neighbor enable
	UINT32      MD_TAMPER_EDG_TH;     	///< deghost enable
	UINT32      MD_TAMPER_AVG_TEX;     ///< roi0 enable
	UINT32      MD_TAMPER_AVG_TH;   ///< roi1 enable
} VENDOR_GMM_TEMPER_INFO;
*/
//@}

/**
    GMM level model Parameters.
*/
//@{
/*
typedef struct _VENDOR_GMM_LV_PARAM {
	UINT32 MD_LV_ALPHA;                    ///< mor0 threshold
	UINT32 MD_LV_ONE_MIN_ALPHA;            ///< mor1 threshold	
	UINT32 MD_LV_MODEL_UPDATE;             ///<  update model enable
	UINT32 MD_LV_TB;                       ///<  threshold on the squared Mahalan
	UINT32 MD_LV_SIGMA;                    ///< 
    UINT32 MD_LV_TG;                       ///< threshold on the squared Mahalan
    UINT32 MD_LV_PRUNE;                    ///< prune = (alpha * CT)*32    
	UINT32 MD_LV_LUM_DIFF_TH;             ///< uma Difference Threshold
    UINT32 MD_LV_TEX_DIFF_TH;             ///< Texture Difference Threshold
	UINT32 MD_LV_TEX_RATIO_TH;            ///< Texture Ratio Threshold
	UINT32 MD_LV_GM_MD2_TH;
	UINT32 MD_LV_TEX_TH;
	UINT32 MD_LV_INIT_WEIGHT;        
} VENDOR_GMM_LV_PARAM;
*/
//@}

/**

*/
//@{
typedef struct _VENDOR_MD_PARAM {
	VENDOR_MDBC_MODE            mode;          ///< MDBC operation mode
	VENDOR_MDBC_CONTROL_EN      controlEn;     ///< MDBC control enable
	UINT32                      uiIntEn;       ///< MDBC Interrupt enable
    VENDOR_MDBC_INPUT_INFO      InInfo;        ///< MDBC input address & lofs
    VENDOR_MDBC_OUTPUT_INFO     OutInfo;       ///< MDBC output address
	uintptr_t                   uiLLAddr;      ///< MDBC LinkedList address
	VENDOR_MDBC_IN_SIZE         Size;          ///< MDBC size
    VENDOR_MDBC_MDMATCH_PARAM   MdmatchPara;   ///< MDBC Model Match
    VENDOR_MDBC_MOR_PARAM       MorPara;       ///< MDBC Morphological process
    VENDOR_MDBC_UPD_PARAM       UpdPara;       ///< MDBC Update
    VENDOR_MDBC_ROI_PARAM       ROIPara0;      ///< MDBC ROI0
    VENDOR_MDBC_ROI_PARAM       ROIPara1;      ///< MDBC ROI1
    VENDOR_MDBC_ROI_PARAM       ROIPara2;      ///< MDBC ROI2
    VENDOR_MDBC_ROI_PARAM       ROIPara3;      ///< MDBC ROI3
    VENDOR_MDBC_ROI_PARAM       ROIPara4;      ///< MDBC ROI4
    VENDOR_MDBC_ROI_PARAM       ROIPara5;      ///< MDBC ROI5
    VENDOR_MDBC_ROI_PARAM       ROIPara6;      ///< MDBC ROI6
    VENDOR_MDBC_ROI_PARAM       ROIPara7;      ///< MDBC ROI7
    
	//GMM param
	//VENDOR_GMM_CONTROL_INFO    	GmmControl;
	//VENDOR_GMM_TEMPER_INFO     	GmmTemperPara;
	//VENDOR_GMM_LV_PARAM        	GmmParaLv0;
	//VENDOR_GMM_LV_PARAM        	GmmParaLv1;
	//VENDOR_GMM_LV_PARAM        	GmmParaLv2;
	//VENDOR_GMM_LV_PARAM        	GmmParaLv3;	
} VENDOR_MD_PARAM;
//@}

//@{
typedef struct _VENDOR_MD_REG_DATA {
	UINT32                   uiLumDiff;     ///< MDBC LumDiff
	UINT32                   uiFrmID;       ///< MDBC FrmID
	UINT32                   uiRnd;      	///< MDBC Rnd
} VENDOR_MD_REG_DATA;
//@}


/**
    KDRV MD structure - md open object
*/
typedef struct _VENDOR_MD_OPENCFG {
	UINT32          clock_sel;                      ///< Engine clock selection
} VENDOR_MD_OPENCFG, *pVENDOR_MD_OPENCFG;

/**
    parameter data for kdrv_md_set, kdrv_md_get
*/
typedef enum {
	VENDOR_MD_PARAM_OPENCFG = 0,        ///< set engine open info,  data_type: KDRV_MD_OPENCFG
	VENDOR_MD_PARAM_ALL,                ///< set app parameters, data_type: KDRV_MD_PARAM
	VENDOR_MD_PARAM_ISR_CB,			    ///< set isr callback, data_type: KDRV_MD_ISRCB
	VENDOR_MD_PARAM_GET_REG,			///< 
	VENDOR_MD_DMA_ABORT,
	
	VENDOR_MD_PARAM_MAX,
	VENDOR_MD_PARAM_REV = 0x80000000,
	ENUM_DUMMY4WORD(VENDOR_MD_PARAM_ID)
} VENDOR_MD_PARAM_ID;


/********************************************************************
    EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
extern INT32 vendor_md_init(VOID);
extern INT32 vendor_md_uninit(VOID);
extern VOID  vendor_md_get_version(void *p_param);
extern INT32 vendor_md_query(void *p_query_param);
extern INT32 vendor_md_trigger(VENDOR_MD_PARAM *p_param, BOOL instant, unsigned int *handle);
#endif //_VENDOR_MD_H_
