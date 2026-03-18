#ifndef _MD_ENG_BASE_H_
#define _MD_ENG_BASE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/nvt_type.h"
#include "md_eng_handle.h"

typedef enum _MD_ENG_ID {
	MD_ID  = 0,
	MD_ID_MAX_NUM,
	ENUM_DUMMY4WORD(MD_ENG_ID)
} MD_ENG_ID;

// Structure definition
/**
    MD Engine Operation Selection.

    Select MD module operation.
*/
//@{
typedef enum {
	INIT              = 0,   ///< init mode
	NORM              = 1,   ///< normal mode
	ENUM_DUMMY4WORD(MD_MODE)
} MD_MODE;
//@}


/**
    MD Engine TYPE.

    Select MD module operation.
*/
//@{
typedef enum {
	MD              = 0,   ///< init mode
	GMM               = 1,   ///< normal mode
	ENUM_DUMMY4WORD(MD_TYPE)
} MD_TYPE;

/**
    MD Morphological process Sel.

    Select MD Morphological process type.
*/
//@{
typedef enum {
	Median         = 0,   ///< Median
	Dilate         = 1,   ///< Dilate
	Erode          = 2,   ///< Erode
	Bypass         = 3,   ///< Bypass
	ENUM_DUMMY4WORD(MD_MOR_SEL)
} MD_MOR_SEL;
//@}

/**
    MD Input Info.
*/
//@{
typedef struct {
	UINT8      update_nei_en;  ///< update neighbor enable
	UINT8      deghost_en;     ///< deghost enable
	UINT8      roi_en0;        ///< roi0 enable
	UINT8      roi_en1;        ///< roi1 enable
	UINT8      roi_en2;        ///< roi2 enable
	UINT8      roi_en3;        ///< roi3 enable
	UINT8      roi_en4;        ///< roi4 enable
	UINT8      roi_en5;        ///< roi5 enable
	UINT8      roi_en6;        ///< roi6 enable
	UINT8      roi_en7;        ///< roi7 enable
	UINT8      chksum_en;      ///< check sum enable
	UINT8      bgmw_save_bw_en;///< bandwidth saving enable
	UINT8      bc_y_only_en;
	//UINT8      MD_TYPE;         ///< GMM or MD (530-add)
	//UINT8      GMM_EVT_OUT_SEL; ///< (530-add)
	//UINT8      GMM_CLR;	     ///< (530-add)
	UINT8      BC_MODEL_MODE;   ///<Background model format (530-add)
	UINT8      MD_OUT_BIT_DEPTH; ///<output bitdepth select (690-add)
	UINT8      MD_LBSP_DISABLE; ///< LBSP disable
} MD_CONTROL_EN;
//@}

/**
    MD Model Match Parameters.
*/
//@{
typedef struct _MD_MDMATCH_PARAM {
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
	UINT8 model_num_ext;                   ///< bg model number(16 model)
} MD_MDMATCH_PARAM;
//@}

/**
    MDBC Morphological process Parameters.
*/
//@{
typedef struct _MD_MOR_PARAM {
	UINT8 mor_th0;                         ///< mor0 threshold
	UINT8 mor_th1;                         ///< mor1 threshold
	UINT8 mor_th2;                         ///< mor2 threshold
	UINT8 mor_th3;                         ///< mor3 threshold
	UINT8 mor_th_dil;                      ///< mor dil threshold
    UINT8 mor_sel0;                        ///< Morphological type
    UINT8 mor_sel1;                        ///< Morphological type
    UINT8 mor_sel2;                        ///< Morphological type
    UINT8 mor_sel3;                        ///< Morphological type
    UINT8 bc_temporal_th;                  ///< Temporal filter
} MD_MOR_PARAM;
//@}

/**
    MDBC Update Parameters.
*/
//@{
typedef struct _MD_UPD_PARAM {
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
} MD_UPD_PARAM;
//@}

/**
    MD ROI Parameters.
*/
//@{
typedef struct _MD_ROI_PARAM {
	UINT32 roi_x;                    ///< roi x position
	UINT32 roi_y;                    ///< roi y position
	UINT32 roi_w;                    ///< roi width
	UINT32 roi_h;                    ///< roi height
	UINT8 roi_uv_thres;              ///< roi UV threshold
	UINT8 roi_lbsp_th;               ///< roi Lbsp threshold
	UINT8 roi_d_colour;              ///< roi D colour
	UINT8 roi_r_colour;              ///< roi R colour
	UINT8 roi_d_lbsp;                ///< roi D lbsp
	UINT8 roi_r_lbsp;                ///< roi R lbsp
	UINT8 roi_morph_en;              ///< roi morphological process enable
	UINT8 roi_minT;                  ///< Update roi min T
	UINT8 roi_maxT;                  ///< Update roi max T
} MD_ROI_PARAM;
//@}

/**
    GMM Input Info.
*/
//@{
typedef struct {
	UINT16      MD_TBG;  				///< TB
	UINT8       MD2_UPD_ON;     		///< MD2 enable
	UINT8       MD_TAMPER_TYPE;     	///< Tamper type
	UINT8       MD_SCENE_CHANGE_TH; 	///< Scene change threshold
    UINT32      MD_TAMPER_COUNT;    	///< tamper count
	UINT32	    MD_SCENE_CHANGE_COUNT;	///< scene change count
} GMM_CONTROL_INFO;
//@}

/**
    GMM TEMPER Info.
*/
//@{
typedef struct {
	UINT8      MD_TAMPER_EDG_TEX;	///< update neighbor enable
	UINT8      MD_TAMPER_EDG_TH;	///< deghost enable
	UINT8      MD_TAMPER_AVG_TEX;	///< roi0 enable
	UINT8      MD_TAMPER_AVG_TH;	///< roi1 enable
} GMM_TEMPER_INFO;
//@}


/**
    GMM level model Parameters.
*/
//@{
typedef struct _MD_LV_PARAM {
	UINT16 MD_LV_ALPHA;				///< mor0 threshold
	UINT16 MD_LV_ONE_MIN_ALPHA;		///< mor1 threshold
	UINT8 MD_LV_MODEL_UPDATE;		///<  update model enable
	UINT8 MD_LV_TB;					///<  threshold on the squared Mahalan
	UINT8 MD_LV_SIGMA;				///<
    UINT8 MD_LV_TG;					///< threshold on the squared Mahalan
    UINT16 MD_LV_PRUNE;				///< prune = (alpha * CT)*32
	UINT8 MD_LV_LUM_DIFF_TH;		///< uma Difference Threshold
    UINT16 MD_LV_TEX_DIFF_TH;		///< Texture Difference Threshold
	UINT8 MD_LV_TEX_RATIO_TH;		///< Texture Ratio Threshold
	UINT16 MD_LV_GM_MD2_TH;
	UINT16 MD_LV_TEX_TH;
	UINT16 MD_LV_INIT_WEIGHT;
} MD_LV_PARAM;
//@}

//-----------------------------------  DMA PARAM ---------------------------------//

extern VOID md_eng_set_img_size(MD_ENG_HANDLE *p_eng, UINT32 img_w, UINT32 img_h);
extern VOID md_eng_set_md_mode(MD_ENG_HANDLE *p_eng, MD_MODE mode);
extern VOID md_eng_set_md_control_param(MD_ENG_HANDLE *p_eng,MD_CONTROL_EN FuncInfo );
extern VOID md_eng_set_mdmatch_param(MD_ENG_HANDLE *p_eng,MD_MDMATCH_PARAM pMdmatchParam );

extern VOID md_eng_set_morph_param(MD_ENG_HANDLE *p_eng, MD_MOR_PARAM pMdmatchParam );
extern VOID md_eng_set_update_param(MD_ENG_HANDLE *p_eng, MD_UPD_PARAM pUpdateParam );
extern UINT32 md_eng_get_LumDiff(MD_ENG_HANDLE *p_eng);
extern UINT32 md_eng_get_FrmID(MD_ENG_HANDLE *p_eng);
extern UINT32 md_eng_get_RND(MD_ENG_HANDLE *p_eng);

extern VOID md_eng_set_ROI0Param(MD_ENG_HANDLE *p_eng, MD_ROI_PARAM pROIParam);
extern VOID md_eng_set_ROI1Param(MD_ENG_HANDLE *p_eng, MD_ROI_PARAM pROIParam);
extern VOID md_eng_set_ROI2Param(MD_ENG_HANDLE *p_eng, MD_ROI_PARAM pROIParam);
extern VOID md_eng_set_ROI3Param(MD_ENG_HANDLE *p_eng, MD_ROI_PARAM pROIParam);
extern VOID md_eng_set_ROI4Param(MD_ENG_HANDLE *p_eng, MD_ROI_PARAM pROIParam);
extern VOID md_eng_set_ROI5Param(MD_ENG_HANDLE *p_eng, MD_ROI_PARAM pROIParam);
extern VOID md_eng_set_ROI6Param(MD_ENG_HANDLE *p_eng, MD_ROI_PARAM pROIParam);
extern VOID md_eng_set_ROI7Param(MD_ENG_HANDLE *p_eng, MD_ROI_PARAM pROIParam);

extern VOID md_eng_set_gmm_control_param(MD_ENG_HANDLE *p_eng, GMM_CONTROL_INFO pGmmControlInfo);
extern VOID md_eng_set_gmm_param(MD_ENG_HANDLE *p_eng, GMM_TEMPER_INFO pGmmTemperInfo);
extern VOID md_eng_set_gmm_lv0_param(MD_ENG_HANDLE *p_eng, MD_LV_PARAM pGmmLvInfo);
extern VOID md_eng_set_gmm_lv1_param(MD_ENG_HANDLE *p_eng, MD_LV_PARAM pGmmLvInfo);
extern VOID md_eng_set_gmm_lv2_param(MD_ENG_HANDLE *p_eng, MD_LV_PARAM pGmmLvInfo);
extern VOID md_eng_set_gmm_lv3_param(MD_ENG_HANDLE *p_eng, MD_LV_PARAM pGmmLvInfo);

#endif //_MD_ENG_BASE_H_