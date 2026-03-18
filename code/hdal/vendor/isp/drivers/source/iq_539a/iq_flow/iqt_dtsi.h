#ifndef _IQT_DTSI_H_
#define _IQT_DTSI_H_

#define SUB_NODE_LENGTH 64

#include "iq_alg_int.h"

typedef enum _IQT_DTSI_ITEM {
	IQT_DTSI_ITEM_OB_PARAM               =  0,///< data_type: [Set/Get] IQ_OB_PARAM
	IQT_DTSI_ITEM_NR_PARAM,                   ///< data_type: [Set/Get] IQ_NR_PARAM
	IQT_DTSI_ITEM_CFA_PARAM,                  ///< data_type: [Set/Get] IQ_CFA_PARAM
	IQT_DTSI_ITEM_VA_PARAM,                   ///< data_type: [Set/Get] IQ_VA_PARAM
	IQT_DTSI_ITEM_GAMMA_PARAM,                ///< data_type: [Set/Get] IQ_GAMMA_PARAM
	IQT_DTSI_ITEM_CCM_PARAM              =  5,///< data_type: [Set/Get] IQ_CCM_PARAM
	IQT_DTSI_ITEM_COLOR_PARAM,                ///< data_type: [Set/Get] IQ_COLOR_PARAM
	IQT_DTSI_ITEM_CONTRAST_PARAM,             ///< data_type: [Set/Get] IQ_CONTRAST_PARAM
	IQT_DTSI_ITEM_EDGE_PARAM,                 ///< data_type: [Set/Get] IQ_EDGE_PARAM
	IQT_DTSI_ITEM_3DNR_PARAM,                 ///< data_type: [Set/Get] IQ_3DNR_PARAM
	IQT_DTSI_ITEM_PFR_PARAM              = 10,///< data_type: [Set/Get] IQ_PFR_PARAM
	IQT_DTSI_ITEM_WDR_PARAM,                  ///< data_type: [Set/Get] IQ_WDR_PARAM
	IQT_DTSI_ITEM_DEFOG_PARAM,                ///< data_type: [Set/Get] IQ_DEFOG_PARAM
	IQT_DTSI_ITEM_SHDR_PARAM,                 ///< data_type: [Set/Get] IQ_SHDR_PARAM
	IQT_DTSI_ITEM_COMPANDING_PARAM,           ///< data_type: [Set/Get] IQ_COMPANDING_PARAM
	IQT_DTSI_ITEM_RGBIR_PARAM            = 15,///< data_type: [Set/Get] IQ_RGBIR_PARAM
	IQT_DTSI_ITEM_RGBIR_ENH_PARAM,            ///< data_type: [Set/Get] IQ_RGBIR_ENH_PARAM
	IQT_DTSI_ITEM_POST_SHARPEN_2_PARAM,       ///< data_type: [Set/Get] IQ_POST_SHARPEN_2_PARAM
	// depend on module parameter
	IQT_DTSI_ITEM_DPC_PARAM,                  ///< data_type: [Set/Get] IQ_DPC_PARAM
	IQT_DTSI_ITEM_EXPAND_DPC_PARAM,           ///< data_type: [Set/Get] IQ_EXPAND_DPC_PARAM
	IQT_DTSI_ITEM_SHADING_PARAM          = 20,///< data_type: [Set/Get] IQ_SHADING_PARAM
	IQT_DTSI_ITEM_SHADING_EXT_PARAM,          ///< data_type: [Set/Get] IQ_SHADING_EXT_PARAM
	// depend on module parameter
	IQT_DTSI_ITEM_YCURVE_PARAM,               ///< data_type: [Set/Get] IQ_YCURVE_PARAM
	IQT_DTSI_ITEM_CST_PARAM,                  ///< data_type: [Set/Get] IQ_CST_PARAM
	IQT_DTSI_ITEM_POST_SHARPEN_1_PARAM,       ///< data_type: [Set/Get] IQ_POST_SHARPEN_1_PARAM
	IQT_DTSI_ITEM_TONE_PARAM             = 25,///< data_type: [Set/Get] IQ_TONE_PARAM
	IQT_DTSI_ITEM_WDR_ENH_PARAM,              ///< data_type: [Set/Get] IQ_WDR_ENH_PARAM
	IQT_DTSI_ITEM_RAW_VA_PARAM,               ///< data_type: [Set/Get] IQ_RAW_VA_PARAM
	IQT_DTSI_ITEM_FPN_PARAM,                  ///< data_type: [Set/Get] IQ_FPN_PARAM
	IQT_DTSI_ITEM_3DCC_PARAM,                 ///< data_type: [Set/Get] IQ_3DCC_PARAM
	IQT_DTSI_ITEM_3DCC_EXT_PARAM         = 30,///< data_type: [Set/Get] IQ_3DCC_EXT_PARAM
	IQT_DTSI_ITEM_BNR_PARAM,                  ///< data_type: [Set/Get] IQ_BNR_PARAM
	IQT_DTSI_ITEM_MAX_NUM,
	ENUM_DUMMY4WORD(IQT_DTSI_ITEM)
} IQT_DTSI_ITEM;

typedef struct _IQT_DTSI {
	CHAR sub_node_name[SUB_NODE_LENGTH];
	UINT8 *pdata;
	INT32 size;
} IQT_DTSI;

extern ER iqt_dtsi_load(UINT8 *node_path, UINT8 *file_path, UINT8 *buf_addr, IQALG_INFO *iq_info, void *param);

#endif

