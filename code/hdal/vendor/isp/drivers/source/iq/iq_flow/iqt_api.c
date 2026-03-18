#if defined(__FREERTOS)
#include "string.h"
#endif
#include "kwrap/error_no.h"
#include "kwrap/type.h"
#include "kflow_common/nvtmpp.h"
#include "isp_dev.h"

#include "iq_alg_int.h"
#include "iq_flow.h"
#include "iqt_api.h"
#include "iqt_cfg.h"
#include "iqt_dtsi.h"
#include "iq_ui_int.h"
#include "iq_nnsc_int.h"
#include "iq_dbg.h"
#include "iq_version.h"

typedef void (*iqt_fp)(ULONG addr);

//=============================================================================
// function declaration
//=============================================================================
static void iqt_api_get_version(ULONG addr);
static void iqt_api_get_size_tab(ULONG addr);
static void iqt_api_get_nr_lv(ULONG addr);
static void iqt_api_get_3dnr_lv(ULONG addr);
static void iqt_api_get_sharpness_lv(ULONG addr);
static void iqt_api_get_saturation_lv(ULONG addr);
static void iqt_api_get_contrast_lv(ULONG addr);
static void iqt_api_get_brightness_lv(ULONG addr);
static void iqt_api_get_night_mode(ULONG addr);
static void iqt_api_get_ycc_format(ULONG addr);
static void iqt_api_get_operation(ULONG addr);
static void iqt_api_get_imageeffect(ULONG addr);
static void iqt_api_get_ccid(ULONG addr);
static void iqt_api_get_hue_shift(ULONG addr);
static void iqt_api_get_tone_lv(ULONG addr);
static void iqt_api_get_gamma_lv(ULONG addr);
static void iqt_api_get_ob_param(ULONG addr);
static void iqt_api_get_nr_param(ULONG addr);
static void iqt_api_get_cfa_param(ULONG addr);
static void iqt_api_get_raw_va_param(ULONG addr);
static void iqt_api_get_va_param(ULONG addr);
static void iqt_api_get_tone_param(ULONG addr);
static void iqt_api_get_gamma_param(ULONG addr);
static void iqt_api_get_ccm_param(ULONG addr);
static void iqt_api_get_color_param(ULONG addr);
static void iqt_api_get_3dcc_param(ULONG addr);
static void iqt_api_get_3dcc_ext_param(ULONG addr);
static void iqt_api_get_contrast_param(ULONG addr);
static void iqt_api_get_edge_param(ULONG addr);
static void iqt_api_get_3dnr_param(ULONG addr);
static void iqt_api_get_dpc_param(ULONG addr);
static void iqt_api_get_expand_dpc_param(ULONG addr);
static void iqt_api_get_shading_param(ULONG addr);
static void iqt_api_get_shading_ext_param(ULONG addr);
static void iqt_api_get_fpn_param(ULONG addr);
static void iqt_api_get_pfr_param(ULONG addr);
static void iqt_api_get_wdr_param(ULONG addr);
static void iqt_api_get_wdr_enh_param(ULONG addr);
static void iqt_api_get_defog_param(ULONG addr);
static void iqt_api_get_shdr_param(ULONG addr);
static void iqt_api_get_rgbir_param(ULONG addr);
static void iqt_api_get_companding_param(ULONG addr);
static void iqt_api_get_shdr_mode(ULONG addr);
static void iqt_api_get_3dnr_misc_param(ULONG addr);
static void iqt_api_get_post_sharpen_2_param(ULONG addr);
static void iqt_api_get_post_sharpen_1_param(ULONG addr);
static void iqt_api_get_rgbir_enh_param(ULONG addr);
static void iqt_api_get_enh_iso(ULONG addr);
static void iqt_api_get_dr_level(ULONG addr);
static void iqt_api_get_cst_param(ULONG addr);
static void iqt_api_get_ycurve_param(ULONG addr);
static void iqt_api_get_fixth_param(ULONG addr);
static void iqt_api_get_ob_mode_manual(ULONG addr);
static void iqt_api_get_bnr_param(ULONG addr);
static void iqt_api_get_low_power_param(ULONG addr);
static void iqt_api_get_dg_mode_manual(ULONG addr);
static void iqt_api_get_aiisp_param(ULONG addr);
static void iqt_api_get_aiisp_custom_param(ULONG addr);
static void iqt_api_get_cg_mode_manual(ULONG addr);;
static void iqt_api_get_dark_enh_ratio(ULONG addr);
static void iqt_api_get_contrast_enh_ratio(ULONG addr);
static void iqt_api_get_green_enh_ratio(ULONG addr);
static void iqt_api_get_skin_enh_ratio(ULONG addr);
static void iqt_api_set_config(ULONG addr);
static void iqt_api_set_dtsi(ULONG addr);
static void iqt_api_set_nr_lv(ULONG addr);
static void iqt_api_set_3dnr_lv(ULONG addr);
static void iqt_api_set_sharpness_lv(ULONG addr);
static void iqt_api_set_saturation_lv(ULONG addr);
static void iqt_api_set_contrast_lv(ULONG addr);
static void iqt_api_set_brightness_lv(ULONG addr);
static void iqt_api_set_night_mode(ULONG addr);
static void iqt_api_set_ycc_format(ULONG addr);
static void iqt_api_set_operation(ULONG addr);
static void iqt_api_set_imageeffect(ULONG addr);
static void iqt_api_set_ccid(ULONG addr);
static void iqt_api_set_hue_shift(ULONG addr);
static void iqt_api_set_tone_lv(ULONG addr);
static void iqt_api_set_gamma_lv(ULONG addr);
static void iqt_api_set_ob_param(ULONG addr);
static void iqt_api_set_nr_param(ULONG addr);
static void iqt_api_set_cfa_param(ULONG addr);
static void iqt_api_set_raw_va_param(ULONG addr);
static void iqt_api_set_va_param(ULONG addr);
static void iqt_api_set_tone_param(ULONG addr);
static void iqt_api_set_gamma_param(ULONG addr);
static void iqt_api_set_ccm_param(ULONG addr);
static void iqt_api_set_color_param(ULONG addr);
static void iqt_api_set_3dcc_param(ULONG addr);
static void iqt_api_set_3dcc_ext_param(ULONG addr);
static void iqt_api_set_contrast_param(ULONG addr);
static void iqt_api_set_edge_param(ULONG addr);
static void iqt_api_set_3dnr_param(ULONG addr);
static void iqt_api_set_dpc_param(ULONG addr);
static void iqt_api_set_expand_dpc_param(ULONG addr);
static void iqt_api_set_shading_param(ULONG addr);
static void iqt_api_set_shading_ext_param(ULONG addr);
static void iqt_api_set_fpn_param(ULONG addr);
static void iqt_api_set_pfr_param(ULONG addr);
static void iqt_api_set_wdr_param(ULONG addr);
static void iqt_api_set_wdr_enh_param(ULONG addr);
static void iqt_api_set_defog_param(ULONG addr);
static void iqt_api_set_shdr_param(ULONG addr);
static void iqt_api_set_rgbir_param(ULONG addr);
static void iqt_api_set_companding_param(ULONG addr);
static void iqt_api_set_edge_dbg(ULONG addr);
static void iqt_api_set_3dnr_dbg(ULONG addr);
static void iqt_api_set_shdr_mode(ULONG addr);
static void iqt_api_set_3dnr_misc_param(ULONG addr);
static void iqt_api_set_post_sharpen_2_param(ULONG addr);
static void iqt_api_set_post_sharpen_1_param(ULONG addr);
static void iqt_api_set_rgbir_enh_param(ULONG addr);
static void iqt_api_set_cst_param(ULONG addr);
static void iqt_api_set_ycurve_param(ULONG addr);
static void iqt_api_set_fixth_param(ULONG addr);
static void iqt_api_set_ob_mode_manual(ULONG addr);
static void iqt_api_set_bnr_param(ULONG addr);
static void iqt_api_set_low_power_param(ULONG addr);
static void iqt_api_set_dg_mode_manual(ULONG addr);
static void iqt_api_set_aiisp_param(ULONG addr);
static void iqt_api_set_aiisp_custom_param(ULONG addr);
static void iqt_api_set_cg_mode_manual(ULONG addr);
static void iqt_api_set_bnr_dbg(ULONG addr);
static void iqt_api_set_shdr_dbg(ULONG addr);
static void iqt_api_set_lca_dbg(ULONG addr);
static void iqt_api_set_post_sharpen_1_dbg(ULONG addr);
static void iqt_api_set_post_sharpen_2_dbg(ULONG addr);
static void iqt_api_set_dark_enh_ratio(ULONG addr);
static void iqt_api_set_contrast_enh_ratio(ULONG addr);
static void iqt_api_set_green_enh_ratio(ULONG addr);
static void iqt_api_set_skin_enh_ratio(ULONG addr);
static void iqt_api_reserve(ULONG addr);

#define RESERVE_SIZE 0

static BOOL iqt_update[IQ_ID_MAX_NUM][IQT_ITEM_MAX];

static IQT_INFO iqt_info = { {
	//id                             size
	{IQT_ITEM_VERSION,               sizeof(UINT32)                   },
	{IQT_ITEM_SIZE_TAB,              sizeof(IQT_INFO)                 },
	{IQT_ITEM_RLD_CONFIG,            sizeof(IQT_CFG_INFO)             },
	{IQT_ITEM_RLD_DTSI,              sizeof(IQT_DTSI_INFO)            },
	{IQT_ITEM_RESERVE_04,            RESERVE_SIZE                     },
	{IQT_ITEM_RESERVE_05,            RESERVE_SIZE                     },
	{IQT_ITEM_RESERVE_06,            RESERVE_SIZE                     },
	{IQT_ITEM_RESERVE_07,            RESERVE_SIZE                     },
	{IQT_ITEM_RESERVE_08,            RESERVE_SIZE                     },
	{IQT_ITEM_RESERVE_09,            RESERVE_SIZE                     },
	{IQT_ITEM_NR_LV,                 sizeof(IQT_NR_LV)                },
	{IQT_ITEM_SHARPNESS_LV,          sizeof(IQT_SHARPNESS_LV)         },
	{IQT_ITEM_SATURATION_LV,         sizeof(IQT_SATURATION_LV)        },
	{IQT_ITEM_CONTRAST_LV,           sizeof(IQT_CONTRAST_LV)          },
	{IQT_ITEM_BRIGHTNESS_LV,         sizeof(IQT_BRIGHTNESS_LV)        },
	{IQT_ITEM_NIGHT_MODE,            sizeof(IQT_NIGHT_MODE)           },
	{IQT_ITEM_YCC_FORMAT,            sizeof(IQT_YCC_FORMAT)           },
	{IQT_ITEM_OPERATION,             sizeof(IQT_OPERATION),           },
	{IQT_ITEM_IMAGEEFFECT,           sizeof(IQT_IMAGEEFFECT)          },
	{IQT_ITEM_CCID,                  sizeof(IQT_CCID)                 },
	{IQT_ITEM_HUE_SHIFT,             sizeof(IQT_HUE_SHIFT)            },
	{IQT_ITEM_TONE_LV,               sizeof(IQT_TONE_LV)              },
	{IQT_ITEM_3DNR_LV,               sizeof(IQT_3DNR_LV)              },
	{IQT_ITEM_GAMMA_LV,              sizeof(IQT_GAMMA_LV)             },
	{IQT_ITEM_RESERVE_24,            RESERVE_SIZE                     },
	{IQT_ITEM_RESERVE_25,            RESERVE_SIZE                     },
	{IQT_ITEM_RESERVE_26,            RESERVE_SIZE                     },
	{IQT_ITEM_RESERVE_27,            RESERVE_SIZE                     },
	{IQT_ITEM_RESERVE_28,            RESERVE_SIZE                     },
	{IQT_ITEM_RESERVE_29,            RESERVE_SIZE                     },
	{IQT_ITEM_RESERVE_30,            RESERVE_SIZE                     },
	{IQT_ITEM_OB_PARAM,              sizeof(IQT_OB_PARAM)             },
	{IQT_ITEM_NR_PARAM,              sizeof(IQT_NR_PARAM)             },
	{IQT_ITEM_CFA_PARAM,             sizeof(IQT_CFA_PARAM)            },
	{IQT_ITEM_VA_PARAM,              sizeof(IQT_VA_PARAM)             },
	{IQT_ITEM_GAMMA_PARAM,           sizeof(IQT_GAMMA_PARAM)          },
	{IQT_ITEM_CCM_PARAM,             sizeof(IQT_CCM_PARAM)            },
	{IQT_ITEM_COLOR_PARAM,           sizeof(IQT_COLOR_PARAM)          },
	{IQT_ITEM_CONTRAST_PARAM,        sizeof(IQT_CONTRAST_PARAM)       },
	{IQT_ITEM_EDGE_PARAM,            sizeof(IQT_EDGE_PARAM)           },
	{IQT_ITEM_3DNR_PARAM,            sizeof(IQT_3DNR_PARAM)           },
	{IQT_ITEM_DPC_PARAM,             sizeof(IQT_DPC_PARAM)            },
	{IQT_ITEM_SHADING_PARAM,         sizeof(IQT_SHADING_PARAM)        },
	{IQT_ITEM_RAW_VA_PARAM,          sizeof(IQT_RAW_VA_PARAM)         },
	{IQT_ITEM_PFR_PARAM,             sizeof(IQT_PFR_PARAM)            },
	{IQT_ITEM_WDR_PARAM,             sizeof(IQT_WDR_PARAM)            },
	{IQT_ITEM_DEFOG_PARAM,           sizeof(IQT_DEFOG_PARAM)          },
	{IQT_ITEM_SHDR_PARAM,            sizeof(IQT_SHDR_PARAM)           },
	{IQT_ITEM_RGBIR_PARAM,           sizeof(IQT_RGBIR_PARAM)          },
	{IQT_ITEM_COMPANDING_PARAM,      sizeof(IQT_COMPANDING_PARAM)     },
	{IQT_ITEM_EDGE_DBG,              sizeof(IQT_EDGE_DBG)             },
	{IQT_ITEM_3DNR_DBG,              sizeof(IQT_3DNR_DBG)             },
	{IQT_ITEM_SHDR_MODE,             sizeof(IQT_SHDR_MODE)            },
	{IQT_ITEM_3DNR_MISC_PARAM,       sizeof(IQT_3DNR_MISC_PARAM)      },
	{IQT_ITEM_RESERVE_54,            RESERVE_SIZE                     },
	{IQT_ITEM_DR_LEVEL,              sizeof(IQT_DR_LEVEL)             },
	{IQT_ITEM_RGBIR_ENH_PARAM,       sizeof(IQT_RGBIR_ENH_PARAM)      },
	{IQT_ITEM_ENH_ISO,               sizeof(IQT_ENH_ISO)              },
	{IQT_ITEM_RESERVE_58,            RESERVE_SIZE                     },
	{IQT_ITEM_POST_SHARPEN_2_PARAM,  sizeof(IQT_POST_SHARPEN_2_PARAM) },
	{IQT_ITEM_SHADING_EXT_PARAM,     sizeof(IQT_SHADING_EXT_PARAM)    },
	{IQT_ITEM_CST_PARAM,             sizeof(IQT_CST_PARAM)            },
	{IQT_ITEM_RESERVE_62,            RESERVE_SIZE                     },
	{IQT_ITEM_YCURVE_PARAM,          sizeof(IQT_YCURVE_PARAM)         },
	{IQT_ITEM_RESERVE_64,            RESERVE_SIZE                     },
	{IQT_ITEM_POST_SHARPEN_1_PARAM,  sizeof(IQT_POST_SHARPEN_1_PARAM) },
	{IQT_ITEM_SHDR_DBG,              sizeof(IQT_SHDR_DBG)             },
	{IQT_ITEM_LCA_DBG,               sizeof(IQT_LCA_DBG)              },
	{IQT_ITEM_POST_SHARPEN_1_DBG,    sizeof(IQT_POST_SHARPEN_1_DBG)   },
	{IQT_ITEM_POST_SHARPEN_2_DBG,    sizeof(IQT_POST_SHARPEN_2_DBG)   },
	{IQT_ITEM_TONE_PARAM,            sizeof(IQT_TONE_PARAM)           },
	{IQT_ITEM_EXPAND_DPC_PARAM,      sizeof(IQT_EXPAND_DPC_PARAM)     },
	{IQT_ITEM_WDR_ENH_PARAM,         sizeof(IQT_WDR_ENH_PARAM)        },
	{IQT_ITEM_RESERVE_73,            RESERVE_SIZE                     },
	{IQT_ITEM_RESERVE_74,            RESERVE_SIZE                     },
	{IQT_ITEM_RESERVE_75,            RESERVE_SIZE                     },
	{IQT_ITEM_RESERVE_76,            RESERVE_SIZE                     },
	{IQT_ITEM_FPN_PARAM,             sizeof(IQT_FPN_PARAM)            },
	{IQT_ITEM_3DCC_PARAM,            sizeof(IQT_3DCC_PARAM)           },
	{IQT_ITEM_3DCC_EXT_PARAM,        sizeof(IQT_3DCC_EXT_PARAM)       },
	{IQT_ITEM_RESERVE_80,            RESERVE_SIZE                     },
	{IQT_ITEM_FIXTH_PARAM,           sizeof(IQT_FIXTH_PARAM)          },
	{IQT_ITEM_OB_MODE_MANUAL,        sizeof(IQT_OB_MODE_MANUAL)       },
	{IQT_ITEM_BNR_PARAM,             sizeof(IQT_BNR_PARAM)            },
	{IQT_ITEM_BNR_DBG,               sizeof(IQT_BNR_DBG)              },
	{IQT_ITEM_LOW_POWER_PARAM,       sizeof(IQT_LOW_POWER_PARAM)      },
	{IQT_ITEM_DG_MODE_MANUAL,        sizeof(IQT_DG_MODE_MANUAL)       },
	{IQT_ITEM_AIISP_PARAM,           sizeof(IQT_AIISP_PARAM)          },
	{IQT_ITEM_AIISP_CUSTOM_PARAM,    sizeof(IQT_AIISP_CUSTOM_PARAM)   },
	{IQT_ITEM_CG_MODE_MANUAL,        sizeof(IQT_CG_MODE_MANUAL)       },
	{IQT_ITEM_RESERVE_90,            RESERVE_SIZE                     }
} };

static iqt_fp iqt_get_tab[IQT_ITEM_MAX] = {
	iqt_api_get_version,
	iqt_api_get_size_tab,
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_reserve,                    // 5
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_get_nr_lv,                  // 10
	iqt_api_get_sharpness_lv,
	iqt_api_get_saturation_lv,
	iqt_api_get_contrast_lv,
	iqt_api_get_brightness_lv,
	iqt_api_get_night_mode,             // 15
	iqt_api_get_ycc_format,
	iqt_api_get_operation,
	iqt_api_get_imageeffect,
	iqt_api_get_ccid,
	iqt_api_get_hue_shift,              // 20
	iqt_api_get_tone_lv,
	iqt_api_get_3dnr_lv,
	iqt_api_get_gamma_lv,
	iqt_api_reserve,
	iqt_api_reserve,                    // 25
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_reserve,                    // 30
	iqt_api_get_ob_param,
	iqt_api_get_nr_param,
	iqt_api_get_cfa_param,
	iqt_api_get_va_param,
	iqt_api_get_gamma_param,            // 35
	iqt_api_get_ccm_param,
	iqt_api_get_color_param,
	iqt_api_get_contrast_param,
	iqt_api_get_edge_param,
	iqt_api_get_3dnr_param,             // 40
	iqt_api_get_dpc_param,
	iqt_api_get_shading_param,
	iqt_api_get_raw_va_param,
	iqt_api_get_pfr_param,
	iqt_api_get_wdr_param,              // 45
	iqt_api_get_defog_param,
	iqt_api_get_shdr_param,
	iqt_api_get_rgbir_param,
	iqt_api_get_companding_param,
	iqt_api_reserve,                    // 50
	iqt_api_reserve,
	iqt_api_get_shdr_mode,
	iqt_api_get_3dnr_misc_param,
	iqt_api_reserve,
	iqt_api_get_dr_level,               // 55
	iqt_api_get_rgbir_enh_param,
	iqt_api_get_enh_iso,
	iqt_api_reserve,
	iqt_api_get_post_sharpen_2_param,
	iqt_api_get_shading_ext_param,      // 60
	iqt_api_get_cst_param,
	iqt_api_reserve,
	iqt_api_get_ycurve_param,
	iqt_api_reserve,
	iqt_api_get_post_sharpen_1_param,   // 65
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_get_tone_param,             // 70
	iqt_api_get_expand_dpc_param,
	iqt_api_get_wdr_enh_param,
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_reserve,                    // 75
	iqt_api_reserve,
	iqt_api_get_fpn_param,
	iqt_api_get_3dcc_param,
	iqt_api_get_3dcc_ext_param,
	iqt_api_reserve,                    // 80
	iqt_api_get_fixth_param,
	iqt_api_get_ob_mode_manual,
	iqt_api_get_bnr_param,
	iqt_api_reserve,
	iqt_api_get_low_power_param,        // 85
	iqt_api_get_dg_mode_manual,
	iqt_api_get_aiisp_param,
	iqt_api_get_aiisp_custom_param,
	iqt_api_get_cg_mode_manual,
	iqt_api_reserve                     // 90
};

static iqt_fp iqt_set_tab[IQT_ITEM_MAX] = {
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_set_config,
	iqt_api_set_dtsi,
	iqt_api_reserve,
	iqt_api_reserve,                    // 5
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_set_nr_lv,                  // 10
	iqt_api_set_sharpness_lv,
	iqt_api_set_saturation_lv,
	iqt_api_set_contrast_lv,
	iqt_api_set_brightness_lv,
	iqt_api_set_night_mode,             // 15
	iqt_api_set_ycc_format,
	iqt_api_set_operation,
	iqt_api_set_imageeffect,
	iqt_api_set_ccid,
	iqt_api_set_hue_shift,              // 20
	iqt_api_set_tone_lv,
	iqt_api_set_3dnr_lv,
	iqt_api_set_gamma_lv,
	iqt_api_reserve,
	iqt_api_reserve,                    // 25
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_reserve,                    // 30
	iqt_api_set_ob_param,
	iqt_api_set_nr_param,
	iqt_api_set_cfa_param,
	iqt_api_set_va_param,
	iqt_api_set_gamma_param,            // 35
	iqt_api_set_ccm_param,
	iqt_api_set_color_param,
	iqt_api_set_contrast_param,
	iqt_api_set_edge_param,
	iqt_api_set_3dnr_param,             // 40
	iqt_api_set_dpc_param,
	iqt_api_set_shading_param,
	iqt_api_set_raw_va_param,
	iqt_api_set_pfr_param,
	iqt_api_set_wdr_param,              // 45
	iqt_api_set_defog_param,
	iqt_api_set_shdr_param,
	iqt_api_set_rgbir_param,
	iqt_api_set_companding_param,
	iqt_api_set_edge_dbg,               // 50
	iqt_api_set_3dnr_dbg,
	iqt_api_set_shdr_mode,
	iqt_api_set_3dnr_misc_param,
	iqt_api_reserve,
	iqt_api_reserve,                    // 55
	iqt_api_set_rgbir_enh_param,
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_set_post_sharpen_2_param,
	iqt_api_set_shading_ext_param,      // 60
	iqt_api_set_cst_param,
	iqt_api_reserve,
	iqt_api_set_ycurve_param,
	iqt_api_reserve,
	iqt_api_set_post_sharpen_1_param,   // 65
	iqt_api_set_shdr_dbg,
	iqt_api_set_lca_dbg,
	iqt_api_set_post_sharpen_1_dbg,
	iqt_api_set_post_sharpen_2_dbg,
	iqt_api_set_tone_param,             // 70
	iqt_api_set_expand_dpc_param,
	iqt_api_set_wdr_enh_param,
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_reserve,                    // 75
	iqt_api_reserve,
	iqt_api_set_fpn_param,
	iqt_api_set_3dcc_param,
	iqt_api_set_3dcc_ext_param,
	iqt_api_reserve,                    // 80
	iqt_api_set_fixth_param,
	iqt_api_set_ob_mode_manual,
	iqt_api_set_bnr_param,
	iqt_api_set_bnr_dbg,
	iqt_api_set_low_power_param,        // 85
	iqt_api_set_dg_mode_manual,
	iqt_api_set_aiisp_param,
	iqt_api_set_aiisp_custom_param,
	iqt_api_set_cg_mode_manual,
	iqt_api_reserve                     // 90
};

static void iqt_api_get_version(ULONG addr)
{
	UINT32 *data = (UINT32 *)addr;

	*data = iq_get_version();
}

static void iqt_api_get_size_tab(ULONG addr)
{
	memcpy((IQT_INFO *)addr, &iqt_info, sizeof(IQT_INFO));
}

static void iqt_api_get_nr_lv(ULONG addr)
{
	IQT_NR_LV *data = (IQT_NR_LV *)addr;
	INT32 lv = iq_ui_get_info(data->id, IQ_UI_ITEM_NR_LV);

	data->lv = lv;
}

static void iqt_api_get_3dnr_lv(ULONG addr)
{
	IQT_3DNR_LV *data = (IQT_3DNR_LV *)addr;
	INT32 lv = iq_ui_get_info(data->id, IQ_UI_ITEM_3DNR_LV);

	data->lv = lv;
}

static void iqt_api_get_sharpness_lv(ULONG addr)
{
	IQT_SHARPNESS_LV *data = (IQT_SHARPNESS_LV *)addr;
	INT32 lv = iq_ui_get_info(data->id, IQ_UI_ITEM_SHARPNESS_LV);

	data->lv = lv;
}

static void iqt_api_get_saturation_lv(ULONG addr)
{
	IQT_SATURATION_LV *data = (IQT_SATURATION_LV *)addr;
	INT32 lv = iq_ui_get_info(data->id, IQ_UI_ITEM_SATURATION_LV);

	data->lv = lv;
}

static void iqt_api_get_contrast_lv(ULONG addr)
{
	IQT_CONTRAST_LV *data = (IQT_CONTRAST_LV *)addr;
	INT32 lv = iq_ui_get_info(data->id, IQ_UI_ITEM_CONTRAST_LV);

	data->lv = lv;
}

static void iqt_api_get_brightness_lv(ULONG addr)
{
	IQT_BRIGHTNESS_LV *data = (IQT_BRIGHTNESS_LV *)addr;
	INT32 lv = iq_ui_get_info(data->id, IQ_UI_ITEM_BRIGHTNESS_LV);

	data->lv = lv;
}

static void iqt_api_get_night_mode(ULONG addr)
{
	IQT_NIGHT_MODE *data = (IQT_NIGHT_MODE *)addr;
	INT32 mode = iq_ui_get_info(data->id, IQ_UI_ITEM_NIGHT_MODE);

	data->mode = mode;
}

static void iqt_api_get_ycc_format(ULONG addr)
{
	IQT_YCC_FORMAT *data = (IQT_YCC_FORMAT *)addr;
	INT32 format = iq_ui_get_info(data->id, IQ_UI_ITEM_YCC_FORMAT);

	data->format = format;
}

static void iqt_api_get_operation(ULONG addr)
{
	IQT_OPERATION *data = (IQT_OPERATION *)addr;
	INT32 value = iq_ui_get_info(data->id, IQ_UI_ITEM_OPERATION);

	data->operation = value;
}

static void iqt_api_get_imageeffect(ULONG addr)
{
	IQT_IMAGEEFFECT *data = (IQT_IMAGEEFFECT *)addr;
	INT32 effect = iq_ui_get_info(data->id, IQ_UI_ITEM_IMAGEEFFECT);

	data->effect = effect;
}

static void iqt_api_get_ccid(ULONG addr)
{
	IQT_CCID *data = (IQT_CCID *)addr;
	INT32 ccid = iq_ui_get_info(data->id, IQ_UI_ITEM_CCID);

	data->ccid = ccid;
}

static void iqt_api_get_hue_shift(ULONG addr)
{
	IQT_HUE_SHIFT *data = (IQT_HUE_SHIFT *)addr;
	INT32 hue_shift = iq_ui_get_info(data->id, IQ_UI_ITEM_HUE_SHIFT);

	data->hue_shift = hue_shift;
}

static void iqt_api_get_tone_lv(ULONG addr)
{
	IQT_TONE_LV *data = (IQT_TONE_LV *)addr;
	INT32 tone_lv = iq_ui_get_info(data->id, IQ_UI_ITEM_TONE_LV);

	data->lv = tone_lv;
}

static void iqt_api_get_gamma_lv(ULONG addr)
{
	IQT_GAMMA_LV *data = (IQT_GAMMA_LV *)addr;
	INT32 gamma_lv = iq_ui_get_info(data->id, IQ_UI_ITEM_GAMMA_LV);

	data->lv = gamma_lv;
}

static void iqt_api_get_ob_param(ULONG addr)
{
	IQT_OB_PARAM *data = (IQT_OB_PARAM *)addr;

	memcpy(&(data->ob), iq_param[data->id]->ob, sizeof(IQ_OB_PARAM));
}

static void iqt_api_get_nr_param(ULONG addr)
{
	IQT_NR_PARAM *data = (IQT_NR_PARAM *)addr;

	memcpy(&(data->nr), iq_param[data->id]->nr, sizeof(IQ_NR_PARAM));
}

static void iqt_api_get_cfa_param(ULONG addr)
{
	IQT_CFA_PARAM *data = (IQT_CFA_PARAM *)addr;

	memcpy(&(data->cfa), iq_param[data->id]->cfa, sizeof(IQ_CFA_PARAM));
}

static void iqt_api_get_raw_va_param(ULONG addr)
{
	IQT_RAW_VA_PARAM *data = (IQT_RAW_VA_PARAM *)addr;

	memcpy(&(data->raw_va), iq_param[data->id]->raw_va, sizeof(IQ_RAW_VA_PARAM));
}

static void iqt_api_get_va_param(ULONG addr)
{
	IQT_VA_PARAM *data = (IQT_VA_PARAM *)addr;

	memcpy(&(data->va), iq_param[data->id]->va, sizeof(IQ_VA_PARAM));
}

static void iqt_api_get_tone_param(ULONG addr)
{
	IQT_TONE_PARAM *data = (IQT_TONE_PARAM *)addr;

	memcpy(&(data->tone), iq_param[data->id]->tone, sizeof(IQ_TONE_PARAM));
}

static void iqt_api_get_gamma_param(ULONG addr)
{
	IQT_GAMMA_PARAM *data = (IQT_GAMMA_PARAM *)addr;

	memcpy(&(data->gamma), iq_param[data->id]->gamma, sizeof(IQ_GAMMA_PARAM));
}

static void iqt_api_get_ccm_param(ULONG addr)
{
	IQT_CCM_PARAM *data = (IQT_CCM_PARAM *)addr;

	memcpy(&(data->ccm), iq_param[data->id]->ccm, sizeof(IQ_CCM_PARAM));
}
static void iqt_api_get_color_param(ULONG addr)
{
	IQT_COLOR_PARAM *data = (IQT_COLOR_PARAM *)addr;

	memcpy(&(data->color), iq_param[data->id]->color, sizeof(IQ_COLOR_PARAM));
}

static void iqt_api_get_3dcc_param(ULONG addr)
{
	IQT_3DCC_PARAM *data = (IQT_3DCC_PARAM *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	if (iq_info->final_ipp.ipe_3dcc != NULL) {
		data->_3dcc.enable = iq_info->final_ipp.ipe_3dcc->enable;
		memcpy(data->_3dcc.manual_3dcc_lut, iq_info->iq_ref_set._3dcc_ext->manual_3dcc_lut, sizeof(UINT32) * IQ_3DCC_LEN);
	}
}

static void iqt_api_get_3dcc_ext_param(ULONG addr)
{
	IQT_3DCC_EXT_PARAM *data = (IQT_3DCC_EXT_PARAM *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	if (iq_info->iq_ref_set._3dcc_ext != NULL) {
		memcpy(data->_3dcc_ext_if._3dcc_lut, iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[data->_3dcc_ext_if._3dcc_idx], sizeof(UINT32) * IQ_3DCC_LEN);
	}
}

static void iqt_api_get_contrast_param(ULONG addr)
{
	IQT_CONTRAST_PARAM *data = (IQT_CONTRAST_PARAM *)addr;

	memcpy(&(data->contrast), iq_param[data->id]->contrast, sizeof(IQ_CONTRAST_PARAM));
}

static void iqt_api_get_edge_param(ULONG addr)
{
	IQT_EDGE_PARAM *data = (IQT_EDGE_PARAM *)addr;

	memcpy(&(data->edge), iq_param[data->id]->edge, sizeof(IQ_EDGE_PARAM));
}

static void iqt_api_get_3dnr_param(ULONG addr)
{
	IQT_3DNR_PARAM *data = (IQT_3DNR_PARAM *)addr;

	memcpy(&(data->_3dnr), iq_param[data->id]->_3dnr, sizeof(IQ_3DNR_PARAM));
}

static void iqt_api_get_dpc_param(ULONG addr)
{
	IQT_DPC_PARAM *data = (IQT_DPC_PARAM *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	if (iq_info->final_sie.sie_dpc != NULL) {
		data->dpc.enable = iq_info->final_sie.sie_dpc->enable;
		memcpy(data->dpc.table, iq_info->final_sie.sie_dpc->table, sizeof(UINT32) * IQ_DPC_MAX_NUM);
	}
}

static void iqt_api_get_expand_dpc_param(ULONG addr)
{
	IQT_EXPAND_DPC_PARAM *data = (IQT_EXPAND_DPC_PARAM *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	if (iq_info->final_sie.sie_dpc != NULL) {
		data->expand_dpc.enable = iq_info->final_sie.sie_dpc->expand_en;
		data->expand_dpc.size = iq_info->final_sie.sie_dpc->dp_buffer_size;
		data->expand_dpc.table_phyaddr = iq_info->final_sie.sie_dpc->expand_table_phyaddr;
	}
}

static void iqt_api_get_shading_param(ULONG addr)
{
	IQT_SHADING_PARAM *data = (IQT_SHADING_PARAM *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	data->shading.vig_enable = iq_info->final_ipp.ife_vig.enable;
	data->shading.vig_center_x = iq_info->final_ipp.ife_cent_ratio.ch0.x;
	data->shading.vig_center_y = iq_info->final_ipp.ife_cent_ratio.ch0.y;
	data->shading.vig_reduce_th = iq_info->iq_ref_set.vig_reduce_th;
	data->shading.vig_zero_th = iq_info->iq_ref_set.vig_zero_th;
	memcpy(data->shading.vig_lut, iq_info->iq_ref_set.vig_lut, sizeof(UINT16) * IQ_SHADING_VIG_LEN);

	data->shading.mode = iq_info->iq_ref_set.ecs_mode;
	data->shading.ecs_smooth_l_m_ct_lower = iq_info->iq_ref_set.ecs_smooth_l_m_ct_lower;
	data->shading.ecs_smooth_l_m_ct_upper = iq_info->iq_ref_set.ecs_smooth_l_m_ct_upper;
	data->shading.ecs_smooth_m_h_ct_lower = iq_info->iq_ref_set.ecs_smooth_m_h_ct_lower;
	data->shading.ecs_smooth_m_h_ct_upper = iq_info->iq_ref_set.ecs_smooth_m_h_ct_upper;
	if (iq_info->final_sie.sie_ecs != NULL) {
		data->shading.ecs_enable = iq_info->final_sie.sie_ecs->enable;
		data->shading.ecs_dthr_enable = iq_info->final_sie.sie_ecs->dthr_enable;
		memcpy(data->shading.ecs_map_tbl, iq_info->iq_ref_set.ecs_ext->manual_ecs_tbl, sizeof(UINT32) * IQ_SHADING_ECS_LEN);
	}
}

static void iqt_api_get_shading_ext_param(ULONG addr)
{
	IQT_SHADING_EXT_PARAM *data = (IQT_SHADING_EXT_PARAM *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	if (iq_info->iq_ref_set.ecs_ext != NULL) {
		memcpy(data->shading_ext_if.ecs_map_tbl, iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[data->shading_ext_if.ecs_map_idx], sizeof(UINT32) * IQ_SHADING_ECS_LEN);
	}
}

static void iqt_api_get_fpn_param(ULONG addr)
{
	IQT_FPN_PARAM *data = (IQT_FPN_PARAM *)addr;

	memcpy(&(data->fpn), iq_param[data->id]->fpn, sizeof(IQ_FPN_PARAM));
}

static void iqt_api_get_pfr_param(ULONG addr)
{
	IQT_PFR_PARAM *data = (IQT_PFR_PARAM *)addr;

	memcpy(&(data->pfr), iq_param[data->id]->pfr, sizeof(IQ_PFR_PARAM));
}

static void iqt_api_get_wdr_param(ULONG addr)
{
	IQT_WDR_PARAM *data = (IQT_WDR_PARAM *)addr;

	memcpy(&(data->wdr), iq_param[data->id]->wdr, sizeof(IQ_WDR_PARAM));
}

static void iqt_api_get_wdr_enh_param(ULONG addr)
{
	IQT_WDR_ENH_PARAM *data = (IQT_WDR_ENH_PARAM *)addr;

	memcpy(&(data->wdr_enh), iq_param[data->id]->wdr_enh, sizeof(IQ_WDR_ENH_PARAM));
}

static void iqt_api_get_defog_param(ULONG addr)
{
	IQT_DEFOG_PARAM *data = (IQT_DEFOG_PARAM *)addr;

	memcpy(&(data->defog), iq_param[data->id]->defog, sizeof(IQ_DEFOG_PARAM));
}

static void iqt_api_get_shdr_param(ULONG addr)
{
	IQT_SHDR_PARAM *data = (IQT_SHDR_PARAM *)addr;

	memcpy(&(data->shdr), iq_param[data->id]->shdr, sizeof(IQ_SHDR_PARAM));
}

static void iqt_api_get_rgbir_param(ULONG addr)
{
	IQT_RGBIR_PARAM *data = (IQT_RGBIR_PARAM *)addr;

	memcpy(&(data->rgbir), iq_param[data->id]->rgbir, sizeof(IQ_RGBIR_PARAM));
}

static void iqt_api_get_companding_param(ULONG addr)
{
	IQT_COMPANDING_PARAM *data = (IQT_COMPANDING_PARAM *)addr;

	memcpy(&(data->companding), iq_param[data->id]->companding, sizeof(IQ_COMPANDING_PARAM));
}

static void iqt_api_get_shdr_mode(ULONG addr)
{
	IQT_SHDR_MODE *data = (IQT_SHDR_MODE *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	data->shdr_mode = iq_info->final_ipp.ife_fusion.fu_ctrl.mode;
}

static void iqt_api_get_3dnr_misc_param(ULONG addr)
{
	IQT_3DNR_MISC_PARAM *data = (IQT_3DNR_MISC_PARAM *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	memcpy(&(data->_3dnr_misc.md_roi), &(iq_info->final_ipp.ime_tmnr.md_roi_param), sizeof(CTL_IME_ISP_TMNR_MD_ROI));
	memcpy(&(data->_3dnr_misc.mc_roi), &(iq_info->final_ipp.ime_tmnr.mc_roi_param), sizeof(CTL_IME_ISP_TMNR_MC_ROI));
	data->_3dnr_misc.roi_mv_th = iq_info->final_ipp.ime_tmnr.ps_param.roi_mv_th;
	data->_3dnr_misc.ds_th_roi = iq_info->final_ipp.ime_tmnr.ps_param.ds_th_roi;
}

static void iqt_api_get_post_sharpen_2_param(ULONG addr)
{
	IQT_POST_SHARPEN_2_PARAM *data = (IQT_POST_SHARPEN_2_PARAM *)addr;

	memcpy(&(data->post_sharpen_2), iq_param[data->id]->post_sharpen_2, sizeof(IQ_POST_SHARPEN_2_PARAM));
}

static void iqt_api_get_post_sharpen_1_param(ULONG addr)
{
	IQT_POST_SHARPEN_1_PARAM *data = (IQT_POST_SHARPEN_1_PARAM *)addr;

	memcpy(&(data->post_sharpen_1), iq_param[data->id]->post_sharpen_1, sizeof(IQ_POST_SHARPEN_1_PARAM));
}

static void iqt_api_get_rgbir_enh_param(ULONG addr)
{
	IQT_RGBIR_ENH_PARAM *data = (IQT_RGBIR_ENH_PARAM *)addr;

	memcpy(&(data->rgbir_enh), iq_param[data->id]->rgbir_enh, sizeof(IQ_RGBIR_ENH_PARAM));
}

static void iqt_api_get_enh_iso(ULONG addr)
{
	IQT_ENH_ISO *data = (IQT_ENH_ISO *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	data->enh_iso = iq_info->final_ipp.enh_gain;
}

static void iqt_api_get_dr_level(ULONG addr)
{
	IQT_DR_LEVEL *data = (IQT_DR_LEVEL *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	data->dr_level = iq_info->dr_level;
}

static void iqt_api_get_cst_param(ULONG addr)
{
	IQT_CST_PARAM *data = (IQT_CST_PARAM *)addr;

	memcpy(&(data->cst), iq_param[data->id]->cst, sizeof(IQ_CST_PARAM));
}

static void iqt_api_get_ycurve_param(ULONG addr)
{
	IQT_YCURVE_PARAM *data = (IQT_YCURVE_PARAM *)addr;

	memcpy(&(data->ycurve), iq_param[data->id]->ycurve, sizeof(IQ_YCURVE_PARAM));
}

static void iqt_api_get_fixth_param(ULONG addr)
{
	IQT_FIXTH_PARAM *data = (IQT_FIXTH_PARAM *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	memcpy(&(data->fixth), &(iq_info->iq_ref_set.fixth), sizeof(IQ_FIXTH_PARAM));
}

static void iqt_api_get_ob_mode_manual(ULONG addr)
{
	IQT_OB_MODE_MANUAL *data = (IQT_OB_MODE_MANUAL *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	memcpy(&(data->ob_mode_manual), &(iq_info->ob_mode_manual), sizeof(IQ_OB_MODE_MANUAL));
}

static void iqt_api_get_bnr_param(ULONG addr)
{
	DBG_WRN("NT98538 not support BNR. \r\n");
	return;
}

static void iqt_api_get_low_power_param(ULONG addr)
{
	DBG_WRN("NT98538 not support low power control. \r\n");
	return;
}

static void iqt_api_get_dg_mode_manual(ULONG addr)
{
	IQT_DG_MODE_MANUAL *data = (IQT_DG_MODE_MANUAL *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	memcpy(&(data->dg_mode_manual), &(iq_info->dg_mode_manual), sizeof(IQ_DG_MODE_MANUAL));
}

static void iqt_api_get_aiisp_param(ULONG addr)
{
	DBG_WRN("NT98538 not support IQT_ITEM_AIISP_PARAM. \r\n");
}

static void iqt_api_get_aiisp_custom_param(ULONG addr)
{
	DBG_WRN("NT98538 not support IQT_AIISP_CUSTOM_PARAM. \r\n");
}

static void iqt_api_get_cg_mode_manual(ULONG addr)
{
	IQT_CG_MODE_MANUAL *data = (IQT_CG_MODE_MANUAL *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	memcpy(&(data->cg_mode_manual), &(iq_info->cg_mode_manual), sizeof(IQ_CG_MODE_MANUAL));
}

ER iqt_api_get_cmd(IQT_ITEM item, ULONG addr)
{
	if (item >= IQT_ITEM_MAX) {
		DBG_WRN("item(%d) out of range\r\n", item);
		return E_SYS;
	}
	if (iqt_get_tab[item] == NULL) {
		DBG_WRN("iqt_get_tab(%d) NULL!!\r\n", item);
		return E_SYS;
	}
	if ((item != IQT_ITEM_VERSION) && (item != IQT_ITEM_SIZE_TAB) && ((*((UINT32 *)addr) >= IQ_ID_MAX_NUM) || (!iq_flow_get_id_valid(*((UINT32 *)addr))))) {
		DBG_WRN("item(%d) id(%d) not valid\r\n", item, *((UINT32 *)addr));
		return E_SYS;
	}

	iqt_get_tab[item](addr);
	return E_OK;
}

UINT32 iqt_get_param_update(UINT32 id, IQT_ITEM item)
{
	return iqt_update[id][item];
}

static void iqt_api_set_config(ULONG addr)
{
	IQT_CFG_INFO *data = (IQT_CFG_INFO *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;
	CFG_FILE_FMT *pcfg_file;
	UINT32 i;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	data->path[IQ_CFG_NAME_LENGTH-1] = '\0';
	pcfg_file = iqt_cfg_open((INT8 *)data->path);
	if (pcfg_file != NULL) {
		iqt_cfg_load(pcfg_file, iq_info, iq_param[data->id]);
		for (i = IQT_ITEM_OB_PARAM; i < IQT_ITEM_MAX; i++) {
			iqt_update[data->id][i] = TRUE;
		}
		iqt_cfg_close(pcfg_file);

		memcpy(iq_info->cfg_path, data->path, sizeof(data->path));
		iq_info->cfg_valid = TRUE;
	} else {
		DBG_WRN("load cfg fail \r\n");
	}
}

static void iqt_api_set_dtsi(ULONG addr)
{
	IQT_DTSI_INFO *data = (IQT_DTSI_INFO *)addr;
	IQ_ID id = data->id;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;
	ER rt = E_OK;
	UINT32 i;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	//DBG_MSG("id = %d, node path = %s, file path = %s \n", id, data->node_path, data->file_path);
	data->node_path[IQ_DTSI_NAME_LENGTH-1] = '\0';
	data->file_path[IQ_DTSI_NAME_LENGTH-1] = '\0';
	rt = iqt_dtsi_load((UINT8 *)data->node_path, (UINT8 *)data->file_path, data->buf_addr, iq_info, iq_param[id]);

	if (rt != E_OK) {
		DBG_WRN("load dtsi fail \r\n");
		return;
	}
	for (i = IQT_ITEM_OB_PARAM; i < IQT_ITEM_MAX; i++) {
		iqt_update[data->id][i] = TRUE;
	}
}

static void iqt_api_set_nr_lv(ULONG addr)
{
	IQT_NR_LV *data = (IQT_NR_LV *)addr;

	iq_ui_set_info(data->id, IQ_UI_ITEM_NR_LV, data->lv);
}

static void iqt_api_set_3dnr_lv(ULONG addr)
{
	IQT_3DNR_LV *data = (IQT_3DNR_LV *)addr;

	iq_ui_set_info(data->id, IQ_UI_ITEM_3DNR_LV, data->lv);
}

static void iqt_api_set_sharpness_lv(ULONG addr)
{
	IQT_SHARPNESS_LV *data = (IQT_SHARPNESS_LV *)addr;

	iq_ui_set_info(data->id, IQ_UI_ITEM_SHARPNESS_LV, data->lv);
}

static void iqt_api_set_saturation_lv(ULONG addr)
{
	IQT_SATURATION_LV *data = (IQT_SATURATION_LV *)addr;

	iq_ui_set_info(data->id, IQ_UI_ITEM_SATURATION_LV, data->lv);
}

static void iqt_api_set_contrast_lv(ULONG addr)
{
	IQT_CONTRAST_LV *data = (IQT_CONTRAST_LV *)addr;

	iq_ui_set_info(data->id, IQ_UI_ITEM_CONTRAST_LV, data->lv);
}

static void iqt_api_set_brightness_lv(ULONG addr)
{
	IQT_BRIGHTNESS_LV *data = (IQT_BRIGHTNESS_LV *)addr;

	iq_ui_set_info(data->id, IQ_UI_ITEM_BRIGHTNESS_LV, data->lv);
}

static void iqt_api_set_night_mode(ULONG addr)
{
	IQT_NIGHT_MODE *data = (IQT_NIGHT_MODE *)addr;

	iq_ui_set_info(data->id, IQ_UI_ITEM_NIGHT_MODE, data->mode);
}

static void iqt_api_set_ycc_format(ULONG addr)
{
	IQT_YCC_FORMAT *data = (IQT_YCC_FORMAT *)addr;

	iq_ui_set_info(data->id, IQ_UI_ITEM_YCC_FORMAT, data->format);
}

static void iqt_api_set_operation(ULONG addr)
{
	IQT_OPERATION *data = (IQT_OPERATION *)addr;

	iq_ui_set_info(data->id, IQ_UI_ITEM_OPERATION, data->operation);
}

static void iqt_api_set_imageeffect(ULONG addr)
{
	IQT_IMAGEEFFECT *data = (IQT_IMAGEEFFECT *)addr;

	iq_ui_set_info(data->id, IQ_UI_ITEM_IMAGEEFFECT, data->effect);
}

static void iqt_api_set_ccid(ULONG addr)
{
	IQT_CCID *data = (IQT_CCID *)addr;

	iq_ui_set_info(data->id, IQ_UI_ITEM_CCID, data->ccid);
}

static void iqt_api_set_hue_shift(ULONG addr)
{
	IQT_HUE_SHIFT *data = (IQT_HUE_SHIFT *)addr;

	iq_ui_set_info(data->id, IQ_UI_ITEM_HUE_SHIFT, data->hue_shift);
}

static void iqt_api_set_tone_lv(ULONG addr)
{
	IQT_TONE_LV *data = (IQT_TONE_LV *)addr;

	iq_ui_set_info(data->id, IQ_UI_ITEM_TONE_LV, data->lv);
}

static void iqt_api_set_gamma_lv(ULONG addr)
{
	IQT_GAMMA_LV *data = (IQT_GAMMA_LV *)addr;

	iqt_update[data->id][IQT_ITEM_GAMMA_PARAM] = TRUE;
	iq_ui_set_info(data->id, IQ_UI_ITEM_GAMMA_LV, data->lv);
}

static void iqt_api_set_ob_param(ULONG addr)
{
	IQT_OB_PARAM *data = (IQT_OB_PARAM *)addr;

	memcpy(iq_param[data->id]->ob, &(data->ob), sizeof(IQ_OB_PARAM));
	iqt_update[data->id][IQT_ITEM_OB_PARAM] = TRUE;
}

static void iqt_api_set_nr_param(ULONG addr)
{
	IQT_NR_PARAM *data = (IQT_NR_PARAM *)addr;

	memcpy(iq_param[data->id]->nr, &(data->nr), sizeof(IQ_NR_PARAM));
	iqt_update[data->id][IQT_ITEM_NR_PARAM] = TRUE;
}

static void iqt_api_set_cfa_param(ULONG addr)
{
	IQT_CFA_PARAM *data = (IQT_CFA_PARAM *)addr;

	memcpy(iq_param[data->id]->cfa, &(data->cfa), sizeof(IQ_CFA_PARAM));
	iqt_update[data->id][IQT_ITEM_CFA_PARAM] = TRUE;
}

static void iqt_api_set_raw_va_param(ULONG addr)
{
	IQT_RAW_VA_PARAM *data = (IQT_RAW_VA_PARAM *)addr;

	memcpy(iq_param[data->id]->raw_va, &(data->raw_va), sizeof(IQ_RAW_VA_PARAM));
	iqt_update[data->id][IQT_ITEM_RAW_VA_PARAM] = TRUE;
}

static void iqt_api_set_va_param(ULONG addr)
{
	IQT_VA_PARAM *data = (IQT_VA_PARAM *)addr;

	memcpy(iq_param[data->id]->va, &(data->va), sizeof(IQ_VA_PARAM));
	iqt_update[data->id][IQT_ITEM_VA_PARAM] = TRUE;
}

static void iqt_api_set_tone_param(ULONG addr)
{
	IQT_TONE_PARAM *data = (IQT_TONE_PARAM *)addr;

	memcpy(iq_param[data->id]->tone, &(data->tone), sizeof(IQ_TONE_PARAM));
	iqt_update[data->id][IQT_ITEM_TONE_PARAM] = TRUE;
}

static void iqt_api_set_gamma_param(ULONG addr)
{
	IQT_GAMMA_PARAM *data = (IQT_GAMMA_PARAM *)addr;

	memcpy(iq_param[data->id]->gamma, &(data->gamma), sizeof(IQ_GAMMA_PARAM));
	iqt_update[data->id][IQT_ITEM_GAMMA_PARAM] = TRUE;
}

static void iqt_api_set_ccm_param(ULONG addr)
{
	IQT_CCM_PARAM *data = (IQT_CCM_PARAM *)addr;

	memcpy(iq_param[data->id]->ccm, &(data->ccm), sizeof(IQ_CCM_PARAM));
	iqt_update[data->id][IQT_ITEM_CCM_PARAM] = TRUE;
}

static void iqt_api_set_color_param(ULONG addr)
{
	IQT_COLOR_PARAM *data = (IQT_COLOR_PARAM *)addr;

	memcpy(iq_param[data->id]->color, &(data->color), sizeof(IQ_COLOR_PARAM));
	iqt_update[data->id][IQT_ITEM_COLOR_PARAM] = TRUE;
}

static void iqt_api_set_3dcc_param(ULONG addr)
{
	IQT_3DCC_PARAM *data = (IQT_3DCC_PARAM *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	if (iq_info->final_ipp.ipe_3dcc != NULL) {
		iq_info->final_ipp.ipe_3dcc->enable = data->_3dcc.enable;
		memcpy(iq_info->iq_ref_set._3dcc_ext->manual_3dcc_lut, data->_3dcc.manual_3dcc_lut, sizeof(UINT32) * IQ_3DCC_LEN);
	} else if (data->_3dcc.enable == TRUE) {
		DBG_WRN("3DCC is unsupported, please check IQ module parameter!! \r\n");
	}
	iqt_update[data->id][IQT_ITEM_3DCC_PARAM] = TRUE;
	iqt_update[data->id][IQT_ITEM_3DCC_EXT_PARAM] = TRUE;
}

static void iqt_api_set_3dcc_ext_param(ULONG addr)
{
	IQT_3DCC_EXT_PARAM *data = (IQT_3DCC_EXT_PARAM *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	if (iq_info->iq_ref_set._3dcc_ext != NULL) {
		memcpy(iq_info->iq_ref_set._3dcc_ext->auto_lut.auto_3dcc_lut[data->_3dcc_ext_if._3dcc_idx], data->_3dcc_ext_if._3dcc_lut, sizeof(UINT32) * IQ_3DCC_LEN);
	}
	iqt_update[data->id][IQT_ITEM_3DCC_PARAM] = TRUE;
	iqt_update[data->id][IQT_ITEM_3DCC_EXT_PARAM] = TRUE;
}

static void iqt_api_set_contrast_param(ULONG addr)
{
	IQT_CONTRAST_PARAM *data = (IQT_CONTRAST_PARAM *)addr;

	memcpy(iq_param[data->id]->contrast, &(data->contrast), sizeof(IQ_CONTRAST_PARAM));
	iqt_update[data->id][IQT_ITEM_CONTRAST_PARAM] = TRUE;
}

static void iqt_api_set_edge_param(ULONG addr)
{
	IQT_EDGE_PARAM *data = (IQT_EDGE_PARAM *)addr;

	memcpy(iq_param[data->id]->edge, &(data->edge), sizeof(IQ_EDGE_PARAM));
	iqt_update[data->id][IQT_ITEM_EDGE_PARAM] = TRUE;
}

static void iqt_api_set_3dnr_param(ULONG addr)
{
	IQT_3DNR_PARAM *data = (IQT_3DNR_PARAM *)addr;

	memcpy(iq_param[data->id]->_3dnr, &(data->_3dnr), sizeof(IQ_3DNR_PARAM));
	iqt_update[data->id][IQT_ITEM_3DNR_PARAM] = TRUE;
}

static void iqt_api_set_dpc_param(ULONG addr)
{
	IQT_DPC_PARAM *data = (IQT_DPC_PARAM *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	if (iq_info->final_sie.sie_dpc != NULL) {
		iq_info->final_sie.sie_dpc->enable = data->dpc.enable;
		memcpy(iq_info->final_sie.sie_dpc->table, data->dpc.table, sizeof(UINT32) * IQ_DPC_MAX_NUM);
	} else if (data->dpc.enable == TRUE) {
		DBG_WRN("DPC is unsupported, please check IQ module parameter!! \r\n");
	}
	iqt_update[data->id][IQT_ITEM_DPC_PARAM] = TRUE;
	iqt_update[data->id][IQT_ITEM_EXPAND_DPC_PARAM] = TRUE;
}

static void iqt_api_set_expand_dpc_param(ULONG addr)
{
	IQT_EXPAND_DPC_PARAM *data = (IQT_EXPAND_DPC_PARAM *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	if ((iq_info->final_sie.sie_dpc != NULL) && (data->expand_dpc.table_phyaddr != 0)) {
		iq_info->final_sie.sie_dpc->expand_en = data->expand_dpc.enable;
		iq_info->final_sie.sie_dpc->expand_table_viraddr = nvtmpp_sys_pa2va(data->expand_dpc.table_phyaddr);
		iq_info->final_sie.sie_dpc->expand_table_phyaddr = data->expand_dpc.table_phyaddr;
		iq_info->final_sie.sie_dpc->dp_buffer_size = data->expand_dpc.size;
		iq_info->final_sie.sie_dpc->dp_total_size = (data->expand_dpc.enable == TRUE) ? data->expand_dpc.size : (IQ_DPC_MAX_NUM * sizeof(UINT32));
	} else {
		if (iq_info->final_sie.sie_dpc == NULL) {
			DBG_WRN("DPC is unsupported, please check IQ module parameter!! \r\n");
		} else if (data->expand_dpc.table_phyaddr == 0) {
			DBG_WRN("table_phyaddr is not available, expand_en force to 0 !! \r\n");
		} else {
			DBG_WRN("iqt_api_set_expand_dpc_param fail!! \r\n");
		}
	}
	iqt_update[data->id][IQT_ITEM_DPC_PARAM] = TRUE;
}

static void iqt_api_set_shading_param(ULONG addr)
{
	IQT_SHADING_PARAM *data = (IQT_SHADING_PARAM *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	iq_info->final_ipp.ife_vig.enable = data->shading.vig_enable;
	iq_info->final_ipp.ife_cent_ratio.ch0.x = data->shading.vig_center_x;
	iq_info->final_ipp.ife_cent_ratio.ch0.y = data->shading.vig_center_y;
	iq_info->final_ipp.ife_cent_ratio.ch1.x = data->shading.vig_center_x;
	iq_info->final_ipp.ife_cent_ratio.ch1.y = data->shading.vig_center_y;
	iq_info->final_ipp.ife_cent_ratio.ch2.x = data->shading.vig_center_x;
	iq_info->final_ipp.ife_cent_ratio.ch2.y = data->shading.vig_center_y;
	iq_info->final_ipp.ife_cent_ratio.ch3.x = data->shading.vig_center_x;
	iq_info->final_ipp.ife_cent_ratio.ch3.y = data->shading.vig_center_y;
	iq_info->iq_ref_set.vig_reduce_th = data->shading.vig_reduce_th;
	iq_info->iq_ref_set.vig_zero_th = data->shading.vig_zero_th;
	memcpy(iq_info->iq_ref_set.vig_lut, data->shading.vig_lut, sizeof(UINT16) * IQ_SHADING_VIG_LEN);

	iq_info->iq_ref_set.ecs_mode = data->shading.mode;
	iq_info->iq_ref_set.ecs_smooth_l_m_ct_lower = data->shading.ecs_smooth_l_m_ct_lower;
	iq_info->iq_ref_set.ecs_smooth_l_m_ct_upper = data->shading.ecs_smooth_l_m_ct_upper;
	iq_info->iq_ref_set.ecs_smooth_m_h_ct_lower = data->shading.ecs_smooth_m_h_ct_lower;
	iq_info->iq_ref_set.ecs_smooth_m_h_ct_upper = data->shading.ecs_smooth_m_h_ct_upper;
	if (iq_info->final_sie.sie_ecs != NULL) {
		iq_info->final_sie.sie_ecs->enable = data->shading.ecs_enable;
		iq_info->final_sie.sie_ecs->dthr_enable = data->shading.ecs_dthr_enable;
		memcpy(iq_info->iq_ref_set.ecs_ext->manual_ecs_tbl, data->shading.ecs_map_tbl, sizeof(UINT32) * IQ_SHADING_ECS_LEN);
	} else if (data->shading.ecs_enable == TRUE) {
		DBG_WRN("ECS is unsupported, please check IQ module parameter!! \r\n");
	}
	iqt_update[data->id][IQT_ITEM_SHADING_PARAM] = TRUE;
	iqt_update[data->id][IQT_ITEM_SHADING_EXT_PARAM] = TRUE;
}

static void iqt_api_set_shading_ext_param(ULONG addr)
{
	IQT_SHADING_EXT_PARAM *data = (IQT_SHADING_EXT_PARAM *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	if (iq_info->iq_ref_set.ecs_ext != NULL) {
		memcpy(iq_info->iq_ref_set.ecs_ext->auto_tbl.ecs_map_tbl[data->shading_ext_if.ecs_map_idx], data->shading_ext_if.ecs_map_tbl, sizeof(UINT32) * IQ_SHADING_ECS_LEN);
	}
	iqt_update[data->id][IQT_ITEM_SHADING_PARAM] = TRUE;
	iqt_update[data->id][IQT_ITEM_SHADING_EXT_PARAM] = TRUE;
}

static void iqt_api_set_fpn_param(ULONG addr)
{
	IQT_FPN_PARAM *data = (IQT_FPN_PARAM *)addr;

	memcpy(iq_param[data->id]->fpn, &(data->fpn), sizeof(IQ_FPN_PARAM));
	iqt_update[data->id][IQT_ITEM_FPN_PARAM] = TRUE;
}

static void iqt_api_set_pfr_param(ULONG addr)
{
	IQT_PFR_PARAM *data = (IQT_PFR_PARAM *)addr;

	memcpy(iq_param[data->id]->pfr, &(data->pfr), sizeof(IQ_PFR_PARAM));
	iqt_update[data->id][IQT_ITEM_PFR_PARAM] = TRUE;
}

static void iqt_api_set_wdr_param(ULONG addr)
{
	IQT_WDR_PARAM *data = (IQT_WDR_PARAM *)addr;

	memcpy(iq_param[data->id]->wdr, &(data->wdr), sizeof(IQ_WDR_PARAM));
	iqt_update[data->id][IQT_ITEM_WDR_PARAM] = TRUE;
	iqt_update[data->id][IQT_ITEM_WDR_ENH_PARAM] = TRUE;
}

static void iqt_api_set_wdr_enh_param(ULONG addr)
{
	IQT_WDR_ENH_PARAM *data = (IQT_WDR_ENH_PARAM *)addr;

	memcpy(iq_param[data->id]->wdr_enh, &(data->wdr_enh), sizeof(IQ_WDR_ENH_PARAM));
	iqt_update[data->id][IQT_ITEM_WDR_PARAM] = TRUE;
	iqt_update[data->id][IQT_ITEM_WDR_ENH_PARAM] = TRUE;
}

static void iqt_api_set_defog_param(ULONG addr)
{
	IQT_DEFOG_PARAM *data = (IQT_DEFOG_PARAM *)addr;

	memcpy(iq_param[data->id]->defog, &(data->defog), sizeof(IQ_DEFOG_PARAM));
	iqt_update[data->id][IQT_ITEM_DEFOG_PARAM] = TRUE;
}

static void iqt_api_set_shdr_param(ULONG addr)
{
	IQT_SHDR_PARAM *data = (IQT_SHDR_PARAM *)addr;

	memcpy(iq_param[data->id]->shdr, &(data->shdr), sizeof(IQ_SHDR_PARAM));
	iqt_update[data->id][IQT_ITEM_SHDR_PARAM] = TRUE;
}

static void iqt_api_set_rgbir_param(ULONG addr)
{
	IQT_RGBIR_PARAM *data = (IQT_RGBIR_PARAM *)addr;

	memcpy(iq_param[data->id]->rgbir, &(data->rgbir), sizeof(IQ_RGBIR_PARAM));
	iqt_update[data->id][IQT_ITEM_RGBIR_PARAM] = TRUE;
}

static void iqt_api_set_companding_param(ULONG addr)
{
	IQT_COMPANDING_PARAM *data = (IQT_COMPANDING_PARAM *)addr;

	memcpy(iq_param[data->id]->companding, &(data->companding), sizeof(IQ_COMPANDING_PARAM));
	iqt_update[data->id][IQT_ITEM_COMPANDING_PARAM] = TRUE;
}

static void iqt_api_set_edge_dbg(ULONG addr)
{
	IQT_EDGE_DBG *data = (IQT_EDGE_DBG *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	memcpy(&(iq_info->final_ipp.ipe_edgedbg), &(data->edge_dbg), sizeof(CTL_IPE_ISP_EDGEDBG));
	iqt_update[data->id][IQT_ITEM_EDGE_DBG] = TRUE;
}

static void iqt_api_set_3dnr_dbg(ULONG addr)
{
	IQT_3DNR_DBG *data = (IQT_3DNR_DBG *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	memcpy(&(iq_info->final_ipp.ime_tmnr.dbg_param), &(data->_3dnr_dbg), sizeof(CTL_IME_ISP_TMNR_DBG));
	iqt_update[data->id][IQT_ITEM_3DNR_DBG] = TRUE;
}

static void iqt_api_set_shdr_mode(ULONG addr)
{
	IQT_SHDR_MODE *data = (IQT_SHDR_MODE *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	iq_info->final_ipp.ife_fusion.fu_ctrl.mode = data->shdr_mode;
	iqt_update[data->id][IQT_ITEM_SHDR_PARAM] = TRUE;     // update fcurve!!
	iqt_update[data->id][IQT_ITEM_SHDR_MODE] = TRUE;
}

static void iqt_api_set_3dnr_misc_param(ULONG addr)
{
	IQT_3DNR_MISC_PARAM *data = (IQT_3DNR_MISC_PARAM *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	memcpy(&(iq_info->final_ipp.ime_tmnr.md_roi_param), &(data->_3dnr_misc.md_roi), sizeof(CTL_IME_ISP_TMNR_MD_ROI));
	memcpy(&(iq_info->final_ipp.ime_tmnr.mc_roi_param), &(data->_3dnr_misc.mc_roi), sizeof(CTL_IME_ISP_TMNR_MC_ROI));
	iq_info->final_ipp.ime_tmnr.ps_param.roi_mv_th = data->_3dnr_misc.roi_mv_th;
	iq_info->final_ipp.ime_tmnr.ps_param.ds_th_roi = data->_3dnr_misc.ds_th_roi;
}

static void iqt_api_set_post_sharpen_2_param(ULONG addr)
{
	IQT_POST_SHARPEN_2_PARAM *data = (IQT_POST_SHARPEN_2_PARAM *)addr;

	memcpy(iq_param[data->id]->post_sharpen_2, &(data->post_sharpen_2), sizeof(IQ_POST_SHARPEN_2_PARAM));
	iqt_update[data->id][IQT_ITEM_POST_SHARPEN_2_PARAM] = TRUE;
}

static void iqt_api_set_post_sharpen_1_param(ULONG addr)
{
	IQT_POST_SHARPEN_1_PARAM *data = (IQT_POST_SHARPEN_1_PARAM *)addr;

	memcpy(iq_param[data->id]->post_sharpen_1, &(data->post_sharpen_1), sizeof(IQ_POST_SHARPEN_1_PARAM));
	iqt_update[data->id][IQT_ITEM_POST_SHARPEN_1_PARAM] = TRUE;
}

static void iqt_api_set_rgbir_enh_param(ULONG addr)
{
	IQT_RGBIR_ENH_PARAM *data = (IQT_RGBIR_ENH_PARAM *)addr;

	memcpy(iq_param[data->id]->rgbir_enh, &(data->rgbir_enh), sizeof(IQ_RGBIR_ENH_PARAM));
	iqt_update[data->id][IQT_ITEM_RGBIR_ENH_PARAM] = TRUE;
}

static void iqt_api_set_cst_param(ULONG addr)
{
	IQT_CST_PARAM *data = (IQT_CST_PARAM *)addr;

	memcpy(iq_param[data->id]->cst, &(data->cst), sizeof(IQ_CST_PARAM));
	iqt_update[data->id][IQT_ITEM_CST_PARAM] = TRUE;
}

static void iqt_api_set_ycurve_param(ULONG addr)
{
	IQT_YCURVE_PARAM *data = (IQT_YCURVE_PARAM *)addr;

	memcpy(iq_param[data->id]->ycurve, &(data->ycurve), sizeof(IQ_YCURVE_PARAM));
	iqt_update[data->id][IQT_ITEM_YCURVE_PARAM] = TRUE;
}

static void iqt_api_set_fixth_param(ULONG addr)
{
	IQT_FIXTH_PARAM *data = (IQT_FIXTH_PARAM *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	memcpy(&(iq_info->iq_ref_set.fixth), &(data->fixth), sizeof(IQ_FIXTH_PARAM));
	iqt_update[data->id][IQT_ITEM_FIXTH_PARAM] = TRUE;
}

static void iqt_api_set_ob_mode_manual(ULONG addr)
{
	IQT_OB_MODE_MANUAL *data = (IQT_OB_MODE_MANUAL *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	memcpy(&(iq_info->ob_mode_manual), &(data->ob_mode_manual), sizeof(IQ_OB_MODE_MANUAL));

	if ((data->ob_mode_manual.manual_mode != IQ_OB_SIE) && (data->ob_mode_manual.manual_mode != IQ_OB_IFE_F) && (data->ob_mode_manual.manual_mode != IQ_OB_IFE)) {
		iq_info->ob_mode_manual.manual_enable = FALSE;
		iq_info->ob_mode_manual.manual_mode = IQ_OB_SIE;
	
		DBG_WRN("manual_mode illegal, set enable = %d manual_mode = %d \r\n", iq_info->ob_mode_manual.manual_enable, iq_info->ob_mode_manual.manual_mode);
	}
}

static void iqt_api_set_bnr_param(ULONG addr)
{
	DBG_WRN("NT98538 not support BNR. \r\n");
	return;
}

static void iqt_api_set_low_power_param(ULONG addr)
{
	DBG_WRN("NT98538 not support low power control. \r\n");
	return;
}

static void iqt_api_set_bnr_dbg(ULONG addr)
{
	DBG_WRN("NT98538 not support BNR. \r\n");
	return;
}

static void iqt_api_set_dg_mode_manual(ULONG addr)
{
	IQT_DG_MODE_MANUAL *data = (IQT_DG_MODE_MANUAL *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	memcpy(&(iq_info->dg_mode_manual), &(data->dg_mode_manual), sizeof(IQ_DG_MODE_MANUAL));

	if ((data->dg_mode_manual.manual_mode != IQ_DG_SIE) && (data->dg_mode_manual.manual_mode != IQ_DG_IFE_F) && (data->dg_mode_manual.manual_mode != IQ_DG_IFE)) {
		iq_info->dg_mode_manual.manual_enable = FALSE;
		iq_info->dg_mode_manual.manual_mode = IQ_DG_SIE;
	
		DBG_WRN("manual_mode illegal, set enable = %d manual_mode = %d \r\n", iq_info->dg_mode_manual.manual_enable, iq_info->dg_mode_manual.manual_mode);
	}
}

static void iqt_api_set_aiisp_param(ULONG addr)
{
	DBG_WRN("NT98538 not support IQT_ITEM_AIISP_PARAM. \r\n");
}

static void iqt_api_set_aiisp_custom_param(ULONG addr)
{
	DBG_WRN("NT98538 not support IQT_ITEM_AIISP_CUSTOM_PARAM. \r\n");
}

static void iqt_api_set_cg_mode_manual(ULONG addr)
{
	IQT_CG_MODE_MANUAL *data = (IQT_CG_MODE_MANUAL *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	memcpy(&(iq_info->cg_mode_manual), &(data->cg_mode_manual), sizeof(IQ_CG_MODE_MANUAL));

	if ((data->cg_mode_manual.manual_mode != IQ_CG_SIE) && (data->cg_mode_manual.manual_mode != IQ_CG_IFE_F) && (data->cg_mode_manual.manual_mode != IQ_CG_IFE)) {
		iq_info->cg_mode_manual.manual_enable = FALSE;
		iq_info->cg_mode_manual.manual_mode = IQ_CG_IFE;
	
		DBG_WRN("manual_mode illegal, set enable = %d manual_mode = %d \r\n", iq_info->cg_mode_manual.manual_enable, iq_info->cg_mode_manual.manual_mode);
	}
}

static void iqt_api_set_shdr_dbg(ULONG addr)
{
	IQT_SHDR_DBG *data = (IQT_SHDR_DBG *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	memcpy(&(iq_info->final_ipp.ife_fusion.dbg), &(data->shdr_dbg), sizeof(CTL_IFE_ISP_FUSION_DBG));
	iqt_update[data->id][IQT_ITEM_SHDR_DBG] = TRUE;
}

static void iqt_api_set_lca_dbg(ULONG addr)
{
	IQT_LCA_DBG *data = (IQT_LCA_DBG *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	memcpy(&(iq_info->final_ipp.ime_lca.dbg), &(data->lca_dbg), sizeof(CTL_IME_ISP_LCA_DBG_PARAM));
	memcpy(&(iq_info->final_ipp.ime_lca_dbg_x_pos), &(data->lca_dbg_x_pos), sizeof(CTL_IPP_ISP_IME_LCA_DBG_X_POS_RATIO));
	iqt_update[data->id][IQT_ITEM_LCA_DBG] = TRUE;
}

static void iqt_api_set_post_sharpen_1_dbg(ULONG addr)
{
	IQT_POST_SHARPEN_1_DBG *data = (IQT_POST_SHARPEN_1_DBG *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	memcpy(&(iq_info->final_ipp.ime_sharpen.dbg_en), &(data->post_sharpen_1_dbg), sizeof(BOOL));
	iqt_update[data->id][IQT_ITEM_POST_SHARPEN_1_DBG] = TRUE;
}

static void iqt_api_set_post_sharpen_2_dbg(ULONG addr)
{
	IQT_POST_SHARPEN_2_DBG *data = (IQT_POST_SHARPEN_2_DBG *)addr;
	ISP_MODULE *iq_module = iq_get_module();
	void *iq_private = iq_module->private;
	IQALG_INFO *iq_info;

	iq_info = (IQALG_INFO *)((ULONG)iq_private + iq_info_buffer_size * iq_flow_get_info_map(data->id));

	memcpy(&(iq_info->final_enc.post_sharpen.bShowSharpInfo), &(data->post_sharpen_2_dbg), sizeof(BOOL));
	iqt_update[data->id][IQT_ITEM_POST_SHARPEN_2_DBG] = TRUE;
}

static void iqt_api_reserve(ULONG addr)
{
	return;
}

ER iqt_api_set_cmd(IQT_ITEM item, ULONG addr)
{
	if (item >= IQT_ITEM_MAX) {
		DBG_WRN("item(%d) out of range\r\n", item);
		return E_SYS;
	}
	if (iqt_set_tab[item] == NULL) {
		DBG_WRN("iqt_set_tab(%d) NULL!!\r\n", item);
		return E_SYS;
	}
	if ((*((UINT32 *)addr) >= IQ_ID_MAX_NUM) || (!iq_flow_get_id_valid(*((UINT32 *)addr)))) {
		DBG_WRN("id(%d) not valid\r\n", *((UINT32 *)addr));
		return E_SYS;
	}

	iqt_set_tab[item](addr);
	return E_OK;
}

void iqt_set_param_update(UINT32 id, IQT_ITEM item)
{
	iqt_update[id][item] = TRUE;
}

void iqt_reset_param_update(UINT32 id, IQT_ITEM item)
{
	iqt_update[id][item] = FALSE;
}

static IQT_NNSC_INFO iqt_nnsc_info = { {
	//id                             size
	{IQT_ITEM_NNSC_DARK_ENH_RATIO,        sizeof(IQT_DARK_ENH_RATIO)       },
	{IQT_ITEM_NNSC_CONTRAST_ENH_RATIO,    sizeof(IQT_CONTRAST_ENH_RATIO)   },
	{IQT_ITEM_NNSC_GREEN_ENH_RATIO,       sizeof(IQT_GREEN_ENH_RATIO)      },
	{IQT_ITEM_NNSC_SKIN_ENH_RATIO,        sizeof(IQT_SKIN_ENH_RATIO)       },
	{IQT_ITEM_NNSC_RESERVE_104,           RESERVE_SIZE                     },
	{IQT_ITEM_NNSC_RESERVE_105,           RESERVE_SIZE                     },
	{IQT_ITEM_NNSC_RESERVE_106,           RESERVE_SIZE                     },
	{IQT_ITEM_NNSC_RESERVE_107,           RESERVE_SIZE                     },
	{IQT_ITEM_NNSC_RESERVE_108,           RESERVE_SIZE                     },
	{IQT_ITEM_NNSC_RESERVE_109,           RESERVE_SIZE                     },
} };

static iqt_fp iqt_nnsc_get_tab[IQT_ITEM_NNSC_MAX - IQT_ITEM_NNSC_OFFSET] = {
	iqt_api_get_dark_enh_ratio,   // 100
	iqt_api_get_contrast_enh_ratio,
	iqt_api_get_green_enh_ratio,
	iqt_api_get_skin_enh_ratio,
	iqt_api_reserve,
	iqt_api_reserve,              // 105
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_reserve,
};

static iqt_fp iqt_nnsc_set_tab[IQT_ITEM_NNSC_MAX - IQT_ITEM_NNSC_OFFSET] = {
	iqt_api_set_dark_enh_ratio,   // 100
	iqt_api_set_contrast_enh_ratio,
	iqt_api_set_green_enh_ratio,
	iqt_api_set_skin_enh_ratio,
	iqt_api_reserve,
	iqt_api_reserve,              // 105
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_reserve,
	iqt_api_reserve,
};

static void iqt_api_get_dark_enh_ratio(ULONG addr)
{
	IQT_DARK_ENH_RATIO *data = (IQT_DARK_ENH_RATIO *)addr;
	UINT32 ratio = iq_nnsc_get_info(data->id, IQ_NNSC_ITEM_DARK_ENH_RATIO);

	data->ratio = ratio;
}

static void iqt_api_get_contrast_enh_ratio(ULONG addr)
{
	IQT_CONTRAST_ENH_RATIO *data = (IQT_CONTRAST_ENH_RATIO *)addr;
	UINT32 ratio = iq_nnsc_get_info(data->id, IQ_NNSC_ITEM_CONTRAST_ENH_RATIO);

	data->ratio = ratio;
}

static void iqt_api_get_green_enh_ratio(ULONG addr)
{
	IQT_GREEN_ENH_RATIO *data = (IQT_GREEN_ENH_RATIO *)addr;
	UINT32 ratio = iq_nnsc_get_info(data->id, IQ_NNSC_ITEM_GREEN_ENH_RATIO);

	data->ratio = ratio;
}

static void iqt_api_get_skin_enh_ratio(ULONG addr)
{
	IQT_SKIN_ENH_RATIO *data = (IQT_SKIN_ENH_RATIO *)addr;
	UINT32 ratio = iq_nnsc_get_info(data->id, IQ_NNSC_ITEM_SKIN_ENH_RATIO);

	data->ratio = ratio;
}

ER iqt_api_nnsc_get_cmd(IQT_ITEM item, ULONG addr)
{
	UINT32 item_remap = item - IQT_ITEM_NNSC_OFFSET;

	if ((item < IQT_ITEM_NNSC_OFFSET) || (item >= IQT_ITEM_NNSC_MAX)) {
		DBG_WRN("item(%d) out of range\r\n", item);
		return E_SYS;
	}
	if (iqt_nnsc_get_tab[item_remap] == NULL) {
		DBG_WRN("iqt_nnsc_get_tab(%d) NULL!!\r\n", item_remap);
		return E_SYS;
	}
	if ((*((UINT32 *)addr) >= IQ_ID_MAX_NUM) || (!iq_flow_get_id_valid(*((UINT32 *)addr)))) {
		DBG_WRN("item(%d) id(%d) not valid\r\n", item, *((UINT32 *)addr));
		return E_SYS;
	}

	iqt_nnsc_get_tab[item_remap](addr);
	return E_OK;
}

static void iqt_api_set_dark_enh_ratio(ULONG addr)
{
	IQT_DARK_ENH_RATIO *data = (IQT_DARK_ENH_RATIO *)addr;

	iq_nnsc_set_info(data->id, IQ_NNSC_ITEM_DARK_ENH_RATIO, data->ratio);
}

static void iqt_api_set_contrast_enh_ratio(ULONG addr)
{
	IQT_CONTRAST_ENH_RATIO *data = (IQT_CONTRAST_ENH_RATIO *)addr;

	iq_nnsc_set_info(data->id, IQ_NNSC_ITEM_CONTRAST_ENH_RATIO, data->ratio);
}

static void iqt_api_set_green_enh_ratio(ULONG addr)
{
	IQT_GREEN_ENH_RATIO *data = (IQT_GREEN_ENH_RATIO *)addr;

	iq_nnsc_set_info(data->id, IQ_NNSC_ITEM_GREEN_ENH_RATIO, data->ratio);
}

static void iqt_api_set_skin_enh_ratio(ULONG addr)
{
	IQT_SKIN_ENH_RATIO *data = (IQT_SKIN_ENH_RATIO *)addr;

	iq_nnsc_set_info(data->id, IQ_NNSC_ITEM_SKIN_ENH_RATIO, data->ratio);
}

ER iqt_api_nnsc_set_cmd(IQT_ITEM item, ULONG addr)
{
	UINT32 item_remap = item - IQT_ITEM_NNSC_OFFSET;

	if ((item < IQT_ITEM_NNSC_OFFSET) || (item >= IQT_ITEM_NNSC_MAX)) {
		DBG_WRN("item(%d) out of range\r\n", item);
		return E_SYS;
	}
	if (iqt_nnsc_set_tab[item_remap] == NULL) {
		DBG_WRN("iqt_set_tab(%d) NULL!!\r\n", item);
		return E_SYS;
	}
	if ((*((UINT32 *)addr) >= IQ_ID_MAX_NUM) || (!iq_flow_get_id_valid(*((UINT32 *)addr)))) {
		DBG_WRN("item(%d) id(%d) not valid\r\n", item, *((UINT32 *)addr));
		return E_SYS;
	}

	iqt_nnsc_set_tab[item_remap](addr);
	return E_OK;
}

UINT32 iqt_api_get_item_size(IQT_ITEM item)
{
	if (item < IQT_ITEM_MAX) {
		return iqt_info.size_tab[item][1];
	} else if ((item >= IQT_ITEM_NNSC_OFFSET) && (item < IQT_ITEM_NNSC_MAX)) {
		return iqt_nnsc_info.size_tab[item - IQT_ITEM_NNSC_OFFSET][1];
	} else {
		return 0;
	}
}

