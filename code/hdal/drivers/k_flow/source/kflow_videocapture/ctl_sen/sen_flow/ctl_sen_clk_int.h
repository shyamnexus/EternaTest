#ifndef _CTL_SEN_CLK_INT_H_
#define _CTL_SEN_CLK_INT_H_

#include "ctl_sen_int.h"
#include "ctl_sen_debug_infor_int.h"

#define CG_STR_MCLK     "sn_mclk1"
#define CG_STR_MCLK2    "sn_mclk2"
#define CG_STR_MCLK3    "sn_mclk3"
#define CG_STR_MCLK4    "sn_mclk4"
#define CG_STR_MCLK5    "sn_mclk5"
#define CG_STR_MCLK6    "sn_mclk6"
#define CG_STR_MCLK7    "sn_mclk7"
#define CG_STR_MCLK8    "sn_mclk8"
#if CTL_SEN_KDRV_CG_READY
#include "../../../../../k_driver/source/include/pll_protected.h"
#else
#define CG_STR_SPCLK    ""
#define CG_STR_SPCLK2   ""
#endif

#define CG_STR_FIX480   "fix480m"
#define CG_STR_FIX320   "fix320m"
#define CG_STR_PLL4     "pll4"
#define CG_STR_PLL5     "pll5"
#define CG_STR_PLL6     "pll6"
#define CG_STR_PLL11    "pll11"
#define CG_STR_PLL12    "pll12"
#define CG_STR_PLL20    "pll20"
#define CG_STR_PLL23    "pll23"
#define CG_STR_CSI      "2f0280000.csi"
#define CG_STR_CSI2     "2f0330000.csi2"
#define CG_STR_CSI3     "2f04a0000.csi3"
#define CG_STR_CSI4     "2f04b0000.csi4"
#define CG_STR_CSI5     "2f0308000.csi5"
#define CG_STR_CSI6     "2f030a000.csi6"
#define CG_STR_CSI7     "2f030c000.csi7"
#define CG_STR_CSI8     "2f030e000.csi8"

typedef enum {
	CTL_SEN_UTI_CLK_SIEMCLK_ENABLE,          ///< [set] Set SIEMCLK ENABLE/DISABLE
	CTL_SEN_UTI_CLK_SIEMCLK_SOURCE,          ///< [set] Set SIEMCLK Souce. Please use CTL_SEN_CLKSRC_SEL
	CTL_SEN_UTI_CLK_SIEMCLK_FREQUENCY,       ///< [set] Set SIEMCLK Frequency. Unit in Hertz.

	CTL_SEN_UTI_CLK_SIEMCLK2_ENABLE,         ///< [set] Set SIEMCLK2 ENABLE/DISABLE
	CTL_SEN_UTI_CLK_SIEMCLK2_SOURCE,         ///< [set] Set SIEMCLK2 Souce. Please use CTL_SEN_CLKSRC_SEL
	CTL_SEN_UTI_CLK_SIEMCLK2_FREQUENCY,      ///< [set] Set SIEMCLK2 Frequency. Unit in Hertz.

	CTL_SEN_UTI_CLK_SIEMCLK3_ENABLE,         ///< [set] Set SIEMCLK3 ENABLE/DISABLE
	CTL_SEN_UTI_CLK_SIEMCLK3_SOURCE,         ///< [set] Set SIEMCLK3 Souce. Please use CTL_SEN_CLKSRC_SEL
	CTL_SEN_UTI_CLK_SIEMCLK3_FREQUENCY,      ///< [set] Set SIEMCLK3 Frequency. Unit in Hertz.

	CTL_SEN_UTI_CLK_SIEMCLK4_ENABLE,         ///< [set] Set SIEMCLK4 ENABLE/DISABLE
	CTL_SEN_UTI_CLK_SIEMCLK4_SOURCE,         ///< [set] Set SIEMCLK4 Souce. Please use CTL_SEN_CLKSRC_SEL
	CTL_SEN_UTI_CLK_SIEMCLK4_FREQUENCY,      ///< [set] Set SIEMCLK4 Frequency. Unit in Hertz.

	CTL_SEN_UTI_CLK_SIEMCLK5_ENABLE,         ///< [set] Set SIEMCLK5 ENABLE/DISABLE
	CTL_SEN_UTI_CLK_SIEMCLK5_SOURCE,         ///< [set] Set SIEMCLK5 Souce. Please use CTL_SEN_CLKSRC_SEL
	CTL_SEN_UTI_CLK_SIEMCLK5_FREQUENCY,      ///< [set] Set SIEMCLK5 Frequency. Unit in Hertz.

	CTL_SEN_UTI_CLK_SIEMCLK6_ENABLE,         ///< [set] Set SIEMCLK6 ENABLE/DISABLE
	CTL_SEN_UTI_CLK_SIEMCLK6_SOURCE,         ///< [set] Set SIEMCLK6 Souce. Please use CTL_SEN_CLKSRC_SEL
	CTL_SEN_UTI_CLK_SIEMCLK6_FREQUENCY,      ///< [set] Set SIEMCLK6 Frequency. Unit in Hertz.

	CTL_SEN_UTI_CLK_SIEMCLK7_ENABLE,         ///< [set] Set SIEMCLK7 ENABLE/DISABLE
	CTL_SEN_UTI_CLK_SIEMCLK7_SOURCE,         ///< [set] Set SIEMCLK7 Souce. Please use CTL_SEN_CLKSRC_SEL
	CTL_SEN_UTI_CLK_SIEMCLK7_FREQUENCY,      ///< [set] Set SIEMCLK7 Frequency. Unit in Hertz.

	CTL_SEN_UTI_CLK_SIEMCLK8_ENABLE,         ///< [set] Set SIEMCLK8 ENABLE/DISABLE
	CTL_SEN_UTI_CLK_SIEMCLK8_SOURCE,         ///< [set] Set SIEMCLK8 Souce. Please use CTL_SEN_CLKSRC_SEL
	CTL_SEN_UTI_CLK_SIEMCLK8_FREQUENCY,      ///< [set] Set SIEMCLK8 Frequency. Unit in Hertz.

	CTL_SEN_UTI_CLK_SPCLK_ENABLE,            ///< [set] Set SPCLK ENABLE/DISABLE
	CTL_SEN_UTI_CLK_SPCLK_SOURCE,            ///< [set] Set SPCLK Souce. Please use CTL_SEN_CLKSRC_SEL
	CTL_SEN_UTI_CLK_SPCLK_FREQUENCY,         ///< [set] Set SPCLK Frequency. Unit in Hertz.

	CTL_SEN_UTI_CLK_SPCLK2_ENABLE,           ///< [set] Set SPCLK2 ENABLE/DISABLE
	CTL_SEN_UTI_CLK_SPCLK2_SOURCE,           ///< [set] Set SPCLK2 Souce. Please use CTL_SEN_CLKSRC_SEL
	CTL_SEN_UTI_CLK_SPCLK2_FREQUENCY,        ///< [set] Set SPCLK2 Frequency. Unit in Hertz.

	CTL_SEN_UTI_CLK_PLL4_ENABLE,             ///< [set/get] Set PLL4 ENABLE/DISABLE
	CTL_SEN_UTI_CLK_PLL4_FREQUENCY,          ///< [set/get] Set PLL4 Frequency

	CTL_SEN_UTI_CLK_PLL5_ENABLE,             ///< [set/get] Set PLL5 ENABLE/DISABLE
	CTL_SEN_UTI_CLK_PLL5_FREQUENCY,          ///< [set/get] Set PLL5 Frequency

	CTL_SEN_UTI_CLK_PLL6_ENABLE,             ///< [set/get] Set PLL6 ENABLE/DISABLE
	CTL_SEN_UTI_CLK_PLL6_FREQUENCY,          ///< [set/get] Set PLL6 Frequency

	CTL_SEN_UTI_CLK_PLL11_ENABLE,            ///< [set/get] Set PLL11 ENABLE/DISABLE
	CTL_SEN_UTI_CLK_PLL11_FREQUENCY,         ///< [set/get] Set PLL11 Frequency

	CTL_SEN_UTI_CLK_PLL12_ENABLE,            ///< [set/get] Set PLL12 ENABLE/DISABLE
	CTL_SEN_UTI_CLK_PLL12_FREQUENCY,         ///< [set/get] Set PLL12 Frequency

	CTL_SEN_UTI_CLK_PLL20_ENABLE,            ///< [set/get] Set PLL20 ENABLE/DISABLE
	CTL_SEN_UTI_CLK_PLL20_FREQUENCY,         ///< [set/get] Set PLL20 Frequency

	CTL_SEN_UTI_CLK_PLL23_ENABLE,            ///< [set/get] Set PLL23 ENABLE/DISABLE
	CTL_SEN_UTI_CLK_PLL23_FREQUENCY,         ///< [set/get] Set PLL23 Frequency

	CTL_SEN_UTI_CLK_MIPILVDS_FREQUENCY,      ///< [get] Get MIPI_LVDS Frequency
	CTL_SEN_UTI_CLK_MIPILVDS2_FREQUENCY,     ///< [get] Get MIPI_LVDS2 Frequency
	CTL_SEN_UTI_CLK_MIPILVDS3_FREQUENCY,     ///< [get] Get MIPI_LVDS3 Frequency
	CTL_SEN_UTI_CLK_MIPILVDS4_FREQUENCY,     ///< [get] Get MIPI_LVDS4 Frequency
	CTL_SEN_UTI_CLK_MIPILVDS5_FREQUENCY,     ///< [get] Get MIPI_LVDS5 Frequency
	CTL_SEN_UTI_CLK_MIPILVDS6_FREQUENCY,     ///< [get] Get MIPI_LVDS6 Frequency
	CTL_SEN_UTI_CLK_MIPILVDS7_FREQUENCY,     ///< [get] Get MIPI_LVDS7 Frequency
	CTL_SEN_UTI_CLK_MIPILVDS8_FREQUENCY,     ///< [get] Get MIPI_LVDS8 Frequency

} CTL_SEN_UTI_CLK;

INT32 __set_clk_uti(CTL_SEN_UTI_CLK param_id, UINT32 p_param);
#define _set_clk(param_id, p_param) __set_clk_uti((CTL_SEN_UTI_CLK)(param_id), (UINT32)(p_param))

INT32 __get_clk_uti(CTL_SEN_UTI_CLK param_id, VOID *p_param);
#define _get_clk(param_id, p_param) __get_clk_uti((CTL_SEN_UTI_CLK)(param_id), (VOID *)(p_param))

INT32 ctl_sen_mclk_prepare(void *hdl, CTL_SEN_MODE senmode);
INT32 ctl_sen_mclk_unprepare(void *hdl, CTL_SEN_MODE senmode);
BOOL ctl_sen_mclk_freq_cmp(CTL_SEN_MAP_HDL *map_hdl, CTL_SEN_MODE senmode_0, CTL_SEN_MODE senmode_1);
void ctl_sen_mclk_set_en(CTL_SEN_CLK_SEL mclksel, BOOL en);
INT32 ctl_sen_mclk_get_en(CTL_SEN_CLK_SEL mclksel, BOOL *en);
INT32 ctl_sen_mclk_chk_en(void *hdl, CTL_SEN_MODE senmode, BOOL *mclk_en);
INT32 ctl_sen_get_sendrv_mclk_info(CTL_SEN_MAP_HDL *map_hdl, CTL_SEN_MODE senmode, CTL_SEN_CLK_SEL *mclk_sel, UINT32 *mclk_freq);
INT32 ctl_sen_get_tge_mclk_info(CTL_SEN_MAP_HDL *map_hdl, CTL_SEN_CLK_SEL *mclk_sel);

#endif // _CTL_SEN_CLK_INT_H_
