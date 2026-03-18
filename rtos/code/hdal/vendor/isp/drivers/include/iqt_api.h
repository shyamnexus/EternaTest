#ifndef _IQT_API_H_
#define _IQT_API_H_

#if defined(__KERNEL__) || defined(__FREERTOS)
#include "iq_alg.h"
#include "iq_ui.h"
#include "iq_nnsc.h"
#endif

#define IQ_CFG_NAME_LENGTH 256
#define IQ_DTSI_NAME_LENGTH 256

#define IQT_ITEM_NNSC_OFFSET 100

/**
	IQ tuning item
*/
typedef enum _IQT_ITEM {
	IQT_ITEM_VERSION                                          = 0,///< data_type: [Get]     UINT32
	IQT_ITEM_SIZE_TAB,                                            ///< data_type: [Get]     IQT_INFO
	IQT_ITEM_RLD_CONFIG,                                          ///< data_type: [Set]     IQT_CFG_INFO
	IQT_ITEM_RLD_DTSI,                                            ///< data_type: [Set]     IQT_DTSI_INFO
	IQT_ITEM_RESERVE_04,
	IQT_ITEM_RESERVE_05,
	IQT_ITEM_RESERVE_06,
	IQT_ITEM_RESERVE_07,
	IQT_ITEM_RESERVE_08,
	IQT_ITEM_RESERVE_09,
	IQT_ITEM_NR_LV                                           = 10,///< data_type: [Set/Get] IQT_NR_LV
	IQT_ITEM_SHARPNESS_LV,                                        ///< data_type: [Set/Get] IQT_SHARPNESS_LV
	IQT_ITEM_SATURATION_LV,                                       ///< data_type: [Set/Get] IQT_SATURATION_LV
	IQT_ITEM_CONTRAST_LV,                                         ///< data_type: [Set/Get] IQT_CONTRAST_LV
	IQT_ITEM_BRIGHTNESS_LV,                                       ///< data_type: [Set/Get] IQT_BRIGHTNESS_LV
	IQT_ITEM_NIGHT_MODE                                      = 15,///< data_type: [Set/Get] IQT_NIGHT_MODE
	IQT_ITEM_YCC_FORMAT,                                          ///< data_type: [Set/Get] IQT_YCC_FORMAT
	IQT_ITEM_OPERATION,                                           ///< data_type: [Set/Get] IQT_OPERATION
	IQT_ITEM_IMAGEEFFECT,                                         ///< data_type: [Set/Get] IQT_IMAGEEFFECT
	IQT_ITEM_CCID,                                                ///< data_type: [Set/Get] IQT_CCID
	IQT_ITEM_HUE_SHIFT                                       = 20,///< data_type: [Set/Get] IQT_HUE_SHIFT
	IQT_ITEM_TONE_LV,                                             ///< data_type: [Set/Get] IQT_TONE_LV
	IQT_ITEM_3DNR_LV,                                             ///< data_type: [Set/Get] IQT_3DNR_LV
	IQT_ITEM_GAMMA_LV,                                            ///< data_type: [Set/Get] IQT_GAMMA_LV
	IQT_ITEM_RESERVE_24,
	IQT_ITEM_RESERVE_25,
	IQT_ITEM_RESERVE_26,
	IQT_ITEM_RESERVE_27,
	IQT_ITEM_RESERVE_28,
	IQT_ITEM_RESERVE_29,
	IQT_ITEM_RESERVE_30,
	IQT_ITEM_OB_PARAM,                                            ///< data_type: [Set/Get] IQT_OB_PARAM
	IQT_ITEM_NR_PARAM,                                            ///< data_type: [Set/Get] IQT_NR_PARAM
	IQT_ITEM_CFA_PARAM,                                           ///< data_type: [Set/Get] IQT_CFA_PARAM
	IQT_ITEM_VA_PARAM,                                            ///< data_type: [Set/Get] IQT_VA_PARAM
	IQT_ITEM_GAMMA_PARAM                                     = 35,///< data_type: [Set/Get] IQT_GAMMA_PARAM
	IQT_ITEM_CCM_PARAM,                                           ///< data_type: [Set/Get] IQT_CCM_PARAM
	IQT_ITEM_COLOR_PARAM,                                         ///< data_type: [Set/Get] IQT_COLOR_PARAM
	IQT_ITEM_CONTRAST_PARAM,                                      ///< data_type: [Set/Get] IQT_CONTRAST_PARAM
	IQT_ITEM_EDGE_PARAM,                                          ///< data_type: [Set/Get] IQT_EDGE_PARAM
	IQT_ITEM_3DNR_PARAM                                      = 40,///< data_type: [Set/Get] IQT_3DNR_PARAM
	IQT_ITEM_DPC_PARAM,                                           ///< data_type: [Set/Get] IQT_DPC_PARAM
	IQT_ITEM_SHADING_PARAM,                                       ///< data_type: [Set/Get] IQT_SHADING_PARAM
	IQT_ITEM_RAW_VA_PARAM,                                        ///< data_type: [Set/Get] IQT_RAW_VA_PARAM
	IQT_ITEM_PFR_PARAM,                                           ///< data_type: [Set/Get] IQT_PFR_PARAM
	IQT_ITEM_WDR_PARAM                                       = 45,///< data_type: [Set/Get] IQT_WDR_PARAM
	IQT_ITEM_DEFOG_PARAM,                                         ///< data_type: [Set/Get] IQT_DEFOG_PARAM
	IQT_ITEM_SHDR_PARAM,                                          ///< data_type: [Set/Get] IQT_SHDR_PARAM
	IQT_ITEM_RGBIR_PARAM,                                         ///< data_type: [Set/Get] IQT_RGBIR_PARAM
	IQT_ITEM_COMPANDING_PARAM,                                    ///< data_type: [Set/Get] IQT_COMPANDING_PARAM
	IQT_ITEM_EDGE_DBG                                        = 50,///< data_type: [Set]     IQT_EDGE_DBG
	IQT_ITEM_3DNR_DBG,                                            ///< data_type: [Set]     IQT_3DNR_DBG
	IQT_ITEM_SHDR_MODE,                                           ///< data_type: [Set/Get] IQT_SHDR_MODE
	IQT_ITEM_3DNR_MISC_PARAM,                                     ///< data_type: [Set/Get] IQT_3DNR_MISC_PARAM
	IQT_ITEM_RESERVE_54,
	IQT_ITEM_DR_LEVEL                                        = 55,///< data_type: [Get]     IQT_DR_LEVEL
	IQT_ITEM_RGBIR_ENH_PARAM,                                     ///< data_type: [Set/Get] IQT_RGBIR_ENH_PARAM
	IQT_ITEM_ENH_ISO,                                             ///< data_type: [Get]     IQT_ENH_ISO
	IQT_ITEM_RESERVE_58,
	IQT_ITEM_POST_SHARPEN_2_PARAM,                                ///< data_type: [Set/Get] IQT_POST_SHARPEN_2_PARAM
	IQT_ITEM_SHADING_EXT_PARAM                               = 60,///< data_type: [Set/Get] IQT_SHADING_EXT_PARAM
	IQT_ITEM_CST_PARAM,                                           ///< data_type: [Set/Get] IQT_CST_PARAM
	IQT_ITEM_RESERVE_62,
	IQT_ITEM_YCURVE_PARAM,                                        ///< data_type: [Set/Get] IQT_YCURVE_PARAM
	IQT_ITEM_RESERVE_64,
	IQT_ITEM_POST_SHARPEN_1_PARAM                            = 65,///< data_type: [Set/Get] IQT_POST_SHARPEN_1_PARAM
	IQT_ITEM_SHDR_DBG,                                            ///< data_type: [Set]     IQT_SHDR_DBG
	IQT_ITEM_LCA_DBG,                                             ///< data_type: [Set]     IQT_LCA_DBG
	IQT_ITEM_POST_SHARPEN_1_DBG,                                  ///< data_type: [Set]     IQT_POST_SHARPEN_1_DBG
	IQT_ITEM_POST_SHARPEN_2_DBG,                                  ///< data_type: [Set]     IQT_POST_SHARPEN_2_DBG
	IQT_ITEM_TONE_PARAM                                      = 70,///< data_type: [Set/Get] IQT_TONE_PARAM
	IQT_ITEM_EXPAND_DPC_PARAM,                                    ///< data_type: [Set/Get] IQT_EXPAND_DPC_PARAM
	IQT_ITEM_WDR_ENH_PARAM,                                       ///< data_type: [Set/Get] IQT_WDR_ENH_PARAM
	IQT_ITEM_RESERVE_73,
	IQT_ITEM_RESERVE_74,
	IQT_ITEM_RESERVE_75                                      = 75,
	IQT_ITEM_RESERVE_76,
	IQT_ITEM_FPN_PARAM,                                           ///< data_type: [Set/Get] IQT_FPN_DBG
	IQT_ITEM_3DCC_PARAM,                                          ///< data_type: [Set/Get] IQT_3DCC_PARAM
	IQT_ITEM_3DCC_EXT_PARAM,                                      ///< data_type: [Set/Get] IQT_3DCC_EXT_PARAM
	IQT_ITEM_RESERVE_80                                      = 80,
	IQT_ITEM_FIXTH_PARAM,                                         ///< data_type: [Set/Get] IQT_FIXTH_PARAM
	IQT_ITEM_OB_MODE_MANUAL,                                      ///< data_type: [Set/Get] IQT_OB_MODE_MANUAL
	IQT_ITEM_BNR_PARAM,                                           ///< data_type: [Set/Get] IQT_BNR_PARAM, 539A only
	IQT_ITEM_BNR_DBG,                                             ///< data_type: [Set]     IQT_BNR_DBG, 539A only
	IQT_ITEM_LOW_POWER_PARAM                                 = 85,///< data_type: [Set/Get] IQT_LOW_POWER_PARAM
	IQT_ITEM_DG_MODE_MANUAL,                                      ///< data_type: [Set/Get] IQT_DG_MODE_MANUAL
	IQT_ITEM_AIISP_PARAM,                                         ///< data_type: [Set/Get] IQT_AIISP_PARAM
	IQT_ITEM_AIISP_CUSTOM_PARAM,                                  ///< data_type: [Set/Get] IQT_AIISP_CUSTOM_PARAM
	IQT_ITEM_CG_MODE_MANUAL,                                      ///< data_type: [Set/Get] IQT_CG_MODE_MANUAL
	IQT_ITEM_RESERVE_90                                      = 90,
	IQT_ITEM_RESERVE_91,
	IQT_ITEM_RESERVE_92,
	IQT_ITEM_RESERVE_93,
	IQT_ITEM_RESERVE_94,
	IQT_ITEM_RESERVE_95                                      = 95,
	IQT_ITEM_RESERVE_96,
	IQT_ITEM_RESERVE_97,
	IQT_ITEM_RESERVE_98,
	IQT_ITEM_MAX                                             = 99,

	IQT_ITEM_NNSC_DARK_ENH_RATIO           = IQT_ITEM_NNSC_OFFSET,///< data_type: [Set/Get] IQT_DARK_ENH_RATIO
	IQT_ITEM_NNSC_CONTRAST_ENH_RATIO,                             ///< data_type: [Set/Get] IQT_CONTRAST_ENH_RATIO
	IQT_ITEM_NNSC_GREEN_ENH_RATIO,                                ///< data_type: [Set/Get] IQT_GREEN_ENH_RATIO
	IQT_ITEM_NNSC_SKIN_ENH_RATIO,                                 ///< data_type: [Set/Get] IQT_SKIN_ENH_RATIO
	IQT_ITEM_NNSC_RESERVE_104,
	IQT_ITEM_NNSC_RESERVE_105,
	IQT_ITEM_NNSC_RESERVE_106,
	IQT_ITEM_NNSC_RESERVE_107,
	IQT_ITEM_NNSC_RESERVE_108,
	IQT_ITEM_NNSC_RESERVE_109,
	IQT_ITEM_NNSC_MAX                                       = 110,

	ENUM_DUMMY4WORD(IQT_ITEM)
} IQT_ITEM;

// Struct of Parameter
typedef struct IQT_INFO {
	UINT32 size_tab[IQT_ITEM_MAX][2];
} IQT_INFO;

typedef struct _IQT_CFG_INFO {
	IQ_ID id;
	CHAR path[IQ_CFG_NAME_LENGTH];
} IQT_CFG_INFO;

typedef struct _IQT_DTSI_INFO {
	IQ_ID id;
	CHAR node_path[IQ_DTSI_NAME_LENGTH];
	CHAR file_path[IQ_DTSI_NAME_LENGTH];
	UINT8 *buf_addr;
} IQT_DTSI_INFO;

typedef struct _IQT_NR_LV {
	IQ_ID id;
	IQ_UI_NR_LV lv;
} IQT_NR_LV;

typedef struct _IQT_3DNR_LV {
	IQ_ID id;
	IQ_UI_3DNR_LV lv;
} IQT_3DNR_LV;

typedef struct _IQT_SHARPNESS_LV {
	IQ_ID id;
	IQ_UI_SHARPNESS_LV lv;
} IQT_SHARPNESS_LV;

typedef struct _IQT_SATURATION_LV {
	IQ_ID id;
	IQ_UI_SATURATION_LV lv;
} IQT_SATURATION_LV;

typedef struct _IQT_CONTRAST_LV {
	IQ_ID id;
	IQ_UI_CONTRAST_LV lv;
} IQT_CONTRAST_LV;

typedef struct _IQT_BRIGHTNESS_LV {
	IQ_ID id;
	IQ_UI_BRIGHTNESS_LV lv;
} IQT_BRIGHTNESS_LV;

typedef struct _IQT_NIGHT_MODE {
	IQ_ID id;
	IQ_UI_NIGHT_MODE mode;
} IQT_NIGHT_MODE;

typedef struct _IQT_YCC_FORMAT {
	IQ_ID id;
	IQ_UI_YCC_FORMAT format;
} IQT_YCC_FORMAT;

typedef struct _IQT_OPERATION {
	IQ_ID id;
	IQ_UI_OPERATION operation;
} IQT_OPERATION;

typedef struct _IQT_IMAGEEFFECT {
	IQ_ID id;
	IQ_UI_IMAGEEFFECT effect;
} IQT_IMAGEEFFECT;

typedef struct _IQT_CCID {
	IQ_ID id;
	IQ_UI_CCID ccid;
} IQT_CCID;

typedef struct _IQT_HUE_SHIFT {
	IQ_ID id;
	IQ_UI_HUE_SHIFT hue_shift;
} IQT_HUE_SHIFT;

typedef struct _IQT_TONE_LV {
	IQ_ID id;
	IQ_UI_TONE_LV lv;
} IQT_TONE_LV;

typedef struct _IQT_GAMMA_LV {
	IQ_ID id;
	IQ_UI_GAMMA_LV lv;
} IQT_GAMMA_LV;

typedef struct _IQT_OB_PARAM {
	IQ_ID id;
	IQ_OB_PARAM ob;
} IQT_OB_PARAM;

typedef struct _IQT_NR_PARAM {
	IQ_ID id;
	IQ_NR_PARAM nr;
} IQT_NR_PARAM;

typedef struct _IQT_CFA_PARAM {
	IQ_ID id;
	IQ_CFA_PARAM cfa;
} IQT_CFA_PARAM;

typedef struct _IQT_RAW_VA_PARAM {
	IQ_ID id;
	IQ_RAW_VA_PARAM raw_va;
} IQT_RAW_VA_PARAM;

typedef struct _IQT_VA_PARAM {
	IQ_ID id;
	IQ_VA_PARAM va;
} IQT_VA_PARAM;

typedef struct _IQT_TONE_PARAM {
	IQ_ID id;
	IQ_TONE_PARAM tone;
} IQT_TONE_PARAM;

typedef struct _IQT_GAMMA_PARAM {
	IQ_ID id;
	IQ_GAMMA_PARAM gamma;
} IQT_GAMMA_PARAM;

typedef struct _IQT_CCM_PARAM {
	IQ_ID id;
	IQ_CCM_PARAM ccm;
} IQT_CCM_PARAM;

typedef struct _IQT_COLOR_PARAM {
	IQ_ID id;
	IQ_COLOR_PARAM color;
} IQT_COLOR_PARAM;

typedef struct _IQT_CONTRAST_PARAM {
	IQ_ID id;
	IQ_CONTRAST_PARAM contrast;
} IQT_CONTRAST_PARAM;

typedef struct _IQT_EDGE_PARAM {
	IQ_ID id;
	IQ_EDGE_PARAM edge;
} IQT_EDGE_PARAM;

typedef struct _IQT_3DNR_PARAM {
	IQ_ID id;
	IQ_3DNR_PARAM _3dnr;
} IQT_3DNR_PARAM;

typedef struct _IQT_3DNR_MISC_PARAM {
	IQ_ID id;
	IQ_3DNR_MISC_PARAM _3dnr_misc;
} IQT_3DNR_MISC_PARAM;

typedef struct _IQT_DPC_PARAM {
	IQ_ID id;
	IQ_DPC_PARAM dpc;
} IQT_DPC_PARAM;

typedef struct _IQT_EXPAND_DPC_PARAM {
	IQ_ID id;
	IQ_EXPAND_DPC_PARAM expand_dpc;
} IQT_EXPAND_DPC_PARAM;

typedef struct _IQT_SHADING_PARAM {
	IQ_ID id;
	IQ_SHADING_PARAM shading;
} IQT_SHADING_PARAM;

typedef struct _IQT_SHADING_EXT_PARAM {
	IQ_ID id;
	IQ_SHADING_EXT_PARAM_IF shading_ext_if;
} IQT_SHADING_EXT_PARAM;

typedef struct _IQT_FPN_PARAM {
	IQ_ID id;
	IQ_FPN_PARAM fpn;
} IQT_FPN_PARAM;

typedef struct _IQT_PFR_PARAM {
	IQ_ID id;
	IQ_PFR_PARAM pfr;
} IQT_PFR_PARAM;

typedef struct _IQT_WDR_PARAM {
	IQ_ID id;
	IQ_WDR_PARAM wdr;
} IQT_WDR_PARAM;

typedef struct _IQT_WDR_ENH_PARAM {
	IQ_ID id;
	IQ_WDR_ENH_PARAM wdr_enh;
} IQT_WDR_ENH_PARAM;

typedef struct _IQT_DEFOG_PARAM {
	IQ_ID id;
	IQ_DEFOG_PARAM defog;
} IQT_DEFOG_PARAM;

typedef struct _IQT_SHDR_PARAM {
	IQ_ID id;
	IQ_SHDR_PARAM shdr;
} IQT_SHDR_PARAM;

typedef struct _IQT_RGBIR_PARAM {
	IQ_ID id;
	IQ_RGBIR_PARAM rgbir;
} IQT_RGBIR_PARAM;

typedef struct _IQT_COMPANDING_PARAM {
	IQ_ID id;
	IQ_COMPANDING_PARAM companding;
} IQT_COMPANDING_PARAM;

typedef struct _IQT_POST_SHARPEN_2_PARAM {
	IQ_ID id;
	IQ_POST_SHARPEN_2_PARAM post_sharpen_2;
} IQT_POST_SHARPEN_2_PARAM;

typedef struct _IQT_POST_SHARPEN_1_PARAM {
	IQ_ID id;
	IQ_POST_SHARPEN_1_PARAM post_sharpen_1;
} IQT_POST_SHARPEN_1_PARAM;

typedef struct _IQT_RGBIR_ENH_PARAM {
	IQ_ID id;
	IQ_RGBIR_ENH_PARAM rgbir_enh;
} IQT_RGBIR_ENH_PARAM;

typedef struct _IQT_DR_LEVEL {
	IQ_ID id;
	UINT32 dr_level;
} IQT_DR_LEVEL;

typedef struct _IQT_ENH_ISO {
	IQ_ID id;
	UINT32 enh_iso;
} IQT_ENH_ISO;

typedef struct _IQT_CST_PARAM {
	IQ_ID id;
	IQ_CST_PARAM cst;
} IQT_CST_PARAM;

typedef struct _IQT_YCURVE_PARAM {
	IQ_ID id;
	IQ_YCURVE_PARAM ycurve;
} IQT_YCURVE_PARAM;

typedef struct _IQT_3DCC_PARAM {
	IQ_ID id;
	IQ_3DCC_PARAM _3dcc;
} IQT_3DCC_PARAM;

typedef struct _IQT_3DCC_EXT_PARAM {
	IQ_ID id;
	IQ_3DCC_EXT_PARAM_IF _3dcc_ext_if;
} IQT_3DCC_EXT_PARAM;

typedef struct _IQT_FIXTH_PARAM {
	IQ_ID id;
	IQ_FIXTH_PARAM fixth;
} IQT_FIXTH_PARAM;

typedef struct _IQT_OB_MODE_MANUAL {
	IQ_ID id;
	IQ_OB_MODE_MANUAL ob_mode_manual;
} IQT_OB_MODE_MANUAL;

typedef struct _IQT_BNR_PARAM {
	IQ_ID id;
	IQ_BNR_PARAM bnr;
} IQT_BNR_PARAM;

typedef struct _IQT_LOW_POWER_PARAM {
	IQ_ID id;
	IQ_LOW_POWER_PARAM low_power;
} IQT_LOW_POWER_PARAM;

typedef struct _IQT_DG_MODE_MANUAL {
	IQ_ID id;
	IQ_DG_MODE_MANUAL dg_mode_manual;
} IQT_DG_MODE_MANUAL;

typedef struct _IQT_AIISP_PARAM {
	IQ_ID id;
	IQ_AIISP_PARAM aiisp;
} IQT_AIISP_PARAM;

typedef struct _IQT_AIISP_CUSTOM_PARAM {
	IQ_ID id;
	IQ_AIISP_CUSTOM_PARAM aiisp_custom;
} IQT_AIISP_CUSTOM_PARAM;

typedef struct _IQT_CG_MODE_MANUAL {
	IQ_ID id;
	IQ_CG_MODE_MANUAL cg_mode_manual;
} IQT_CG_MODE_MANUAL;

// Struct of NNSC Parameter
typedef struct _IQT_NNSC_INFO {
	UINT32 size_tab[IQT_ITEM_NNSC_MAX - IQT_ITEM_NNSC_OFFSET][2];
} IQT_NNSC_INFO;

typedef struct _IQT_DARK_ENH_RATIO {
	IQ_ID id;
	IQ_NNSC_DARK_ENH_RATIO ratio;
} IQT_DARK_ENH_RATIO;

typedef struct _IQT_CONTRAST_ENH_RATIO {
	IQ_ID id;
	IQ_NNSC_CONTRAST_ENH_RATIO ratio;
} IQT_CONTRAST_ENH_RATIO;

typedef struct _IQT_GREEN_ENH_RATIO {
	IQ_ID id;
	IQ_NNSC_GREEN_ENH_RATIO ratio;
} IQT_GREEN_ENH_RATIO;

typedef struct _IQT_SKIN_ENH_RATIO {
	IQ_ID id;
	IQ_NNSC_SKIN_ENH_RATIO ratio;
} IQT_SKIN_ENH_RATIO;

// NOTE: kernel
#if defined(__KERNEL__) || defined(__FREERTOS)
typedef enum _IQT_EDGE_DBG_MODE_SEL {
	IQT_EDGE_DBG_MODE_REGION,
	IQT_EDGE_DBG_MODE_WEIGHT,
	IQT_EDGE_DBG_MODE_STRENGTH,
	ENUM_DUMMY4WORD(IQT_EDGE_DBG_MODE_SEL)
} IQT_EDGE_DBG_MODE_SEL;

typedef struct _IQT_EDGE_DBG_MODE {
	BOOL enable;
	IQT_EDGE_DBG_MODE_SEL mode_sel;
} IQT_EDGE_DBG_MODE;

typedef struct _IQT_EDGE_DBG {
	IQ_ID id;
	IQT_EDGE_DBG_MODE edge_dbg;
} IQT_EDGE_DBG;

typedef struct _IQT_3DNR_DBG_MODE {
	UINT8 dbg_mv0;
	UINT8 dbg_mode;
} IQT_3DNR_DBG_MODE;

typedef struct _IQT_3DNR_DBG {
	IQ_ID id;
	IQT_3DNR_DBG_MODE _3dnr_dbg;
} IQT_3DNR_DBG;

typedef enum _IQT_SHDR_DBG_MODE_SEL {
	IQT_SHDR_DBG_MODE_DISABLE,
	IQT_SHDR_DBG_MODE_L_EXPOS_WEIGHT,
	IQT_SHDR_DBG_MODE_S_EXPOS_WEIGHT,
	IQT_SHDR_DBG_MODE_N_CURVE_WEIGHT,
	IQT_SHDR_DBG_MODE_D_CURVE_WEIGHT,
	IQT_SHDR_DBG_MODE_UNKNOWN,
	ENUM_DUMMY4WORD(IQT_SHDR_DBG_MODE_SEL)
} IQT_SHDR_DBG_MODE_SEL;

typedef struct _IQT_SHDR_DBG_MODE {
	BOOL enable;
	IQT_SHDR_DBG_MODE_SEL mode;
} IQT_SHDR_DBG_MODE;

typedef struct _IQT_SHDR_DBG {
	IQ_ID id;
	IQT_SHDR_DBG_MODE shdr_dbg;
} IQT_SHDR_DBG;

typedef enum _IQT_LCA_DBG_SEL {
	IQT_LCA_DBG_COLOR_MODE          = 0,
	IQT_LCA_DBG_EDGE_INFORMATION    = 8,
	IQT_LCA_DBG_UNKNOWN,
	ENUM_DUMMY4WORD(IQT_LCA_DBG_SEL)
} IQT_LCA_DBG_SEL;

typedef struct _IQT_LCA_DBG_PARAM {
	BOOL  enable;
	IQT_LCA_DBG_SEL ch_sel;
	UINT8 ch_ofs;
	UINT16 x_pos;
} IQT_LCA_DBG_PARAM;

typedef struct _IQT_LCA_DBG_X_POS_RATIO {
	UINT32 ratio_base;
	UINT32 x_pos_ratio;
} IQT_LCA_DBG_X_POS_RATIO;

typedef struct _IQT_LCA_DBG {
	IQ_ID id;
	IQT_LCA_DBG_PARAM lca_dbg;
	IQT_LCA_DBG_X_POS_RATIO lca_dbg_x_pos;
} IQT_LCA_DBG;

typedef struct _IQT_POST_SHARPEN_1_DBG {
	IQ_ID id;
	BOOL post_sharpen_1_dbg;
} IQT_POST_SHARPEN_1_DBG;

typedef struct _IQT_POST_SHARPEN_2_DBG {
	IQ_ID id;
	BOOL post_sharpen_2_dbg;
} IQT_POST_SHARPEN_2_DBG;

typedef struct _IQT_BNR_DBG {
	IQ_ID id;
	BOOL enable;
} IQT_BNR_DBG;
#endif
// NOTE: kernel

typedef enum _IQT_SHDR_MODE_SEL {
	IQT_SHDR_MODE_FUSION = 0,
	IQT_SHDR_MODE_SHORT = 1,
	IQT_SHDR_MODE_LONG = 2,
	ENUM_DUMMY4WORD(IQT_SHDR_MODE_SEL)
} IQT_SHDR_MODE_SEL;

typedef struct _IQT_SHDR_MODE {
	IQ_ID id;
	IQT_SHDR_MODE_SEL shdr_mode;
} IQT_SHDR_MODE;

#endif

