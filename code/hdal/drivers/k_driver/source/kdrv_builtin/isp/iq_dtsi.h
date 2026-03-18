#ifndef _IQ_DTSI_H_
#define _IQ_DTSI_H_

#define SUB_NODE_LENGTH 64

typedef enum _IQ_DTSI_ITEM {
	IQ_DTSI_ITEM_OB_PARAM               =  0,///< data_type: [Set/Get] IQ_OB_PARAM
	IQ_DTSI_ITEM_NR_PARAM,                   ///< data_type: [Set/Get] IQ_NR_PARAM
	IQ_DTSI_ITEM_CFA_PARAM,                  ///< data_type: [Set/Get] IQ_CFA_PARAM
	IQ_DTSI_ITEM_RAW_VA_PARAM,               ///< data_type: [Set/Get] IQ_RAW_VA_PARAM
	IQ_DTSI_ITEM_VA_PARAM,                   ///< data_type: [Set/Get] IQ_VA_PARAM, not ready
	IQ_DTSI_ITEM_TONE_PARAM             =  5,///< data_type: [Set/Get] IQ_TONE_PARAM
	IQ_DTSI_ITEM_GAMMA_PARAM,                ///< data_type: [Set/Get] IQ_GAMMA_PARAM
	IQ_DTSI_ITEM_CCM_PARAM,                  ///< data_type: [Set/Get] IQ_CCM_PARAM
	IQ_DTSI_ITEM_COLOR_PARAM,                ///< data_type: [Set/Get] IQ_COLOR_PARAM
	IQ_DTSI_ITEM_CONTRAST_PARAM,             ///< data_type: [Set/Get] IQ_CONTRAST_PARAM
	IQ_DTSI_ITEM_EDGE_PARAM             = 10,///< data_type: [Set/Get] IQ_EDGE_PARAM
	IQ_DTSI_ITEM_3DNR_PARAM,                 ///< data_type: [Set/Get] IQ_3DNR_PARAM
	IQ_DTSI_ITEM_PFR_PARAM,                  ///< data_type: [Set/Get] IQ_PFR_PARAM
	IQ_DTSI_ITEM_WDR_PARAM,                  ///< data_type: [Set/Get] IQ_WDR_PARAM
	IQ_DTSI_ITEM_WDR_ENH_PARAM,              ///< data_type: [Set/Get] IQ_WDR_ENH_PARAM
	IQ_DTSI_ITEM_DEFOG_PARAM            = 15,///< data_type: [Set/Get] IQ_DEFOG_PARAM
	IQ_DTSI_ITEM_SHDR_PARAM,                 ///< data_type: [Set/Get] IQ_SHDR_PARAM
	IQ_DTSI_ITEM_COMPANDING_PARAM,           ///< data_type: [Set/Get] IQ_COMPANDING_PARAM
	IQ_DTSI_ITEM_RGBIR_PARAM,                ///< data_type: [Set/Get] IQ_RGBIR_PARAM, not ready
	IQ_DTSI_ITEM_RGBIR_ENH_PARAM,            ///< data_type: [Set/Get] IQ_RGBIR_ENH_PARAM, not ready
	IQ_DTSI_ITEM_POST_SHARPEN_1_PARAM   = 20,///< data_type: [Set/Get] IQ_POST_SHARPEN_2_PARAM, not ready
	IQ_DTSI_ITEM_POST_SHARPEN_2_PARAM,       ///< data_type: [Set/Get] IQ_POST_SHARPEN_2_PARAM, not ready
	IQ_DTSI_ITEM_YCURVE_PARAM,               ///< data_type: [Set/Get] IQ_YCURVE_PARAM
	IQ_DTSI_ITEM_CST_PARAM,                  ///< data_type: [Set/Get] IQ_CST_PARAM
	IQ_DTSI_ITEM_FPN_PARAM,                  ///< data_type: [Set/Get] IQ_FPN_PARAM
	// depend on module parameter
	IQ_DTSI_ITEM_DPC_PARAM              = 25,///< data_type: [Set/Get] IQ_DPC_PARAM
	IQ_DTSI_ITEM_EXPAND_DPC_PARAM,           ///< data_type: [Set/Get] IQ_EXPAND_DPC_PARAM, not ready
	IQ_DTSI_ITEM_SHADING_PARAM,              ///< data_type: [Set/Get] IQ_SHADING_PARAM
	IQ_DTSI_ITEM_SHADING_EXT_PARAM,          ///< data_type: [Set/Get] IQ_SHADING_EXT_PARAM
	// depend on module parameter

	IQ_DTSI_ITEM_MAX_NUM,
	ENUM_DUMMY4WORD(IQ_DTSI_ITEM)
} IQ_DTSI_ITEM;

typedef struct _IQ_DTSI {
	CHAR sub_node_name[SUB_NODE_LENGTH];
	INT32 size;
	UINT8 *param;
} IQ_DTSI;

extern void iq_dtsi_load(UINT32 id, void *param);
extern void iq_builtin_front_dtsi_load(UINT32 id, void *param);

#endif

