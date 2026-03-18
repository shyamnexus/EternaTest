/*
    LVDS/MIPI-CSI/HiSPi Sensor PHY Configuration Driver Global header

    LVDS/MIPI-CSI/HiSPi Sensor PHY Configuration Driver Global header

    @file       senphy.h
    @ingroup
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _SENPHY_H
#define _SENPHY_H


#include "kwrap/type.h"


/**
    Sensor PHY configuration Selection

    This is used in senphy_set_config() to specify which of the function is selected to assign new configuration.
*/
typedef enum {
	/*
		SENPHY General setting
	*/
	SENPHY_CONFIG_ID_PHY_CLKSEL,	///< PHY Group1(C0~C1/D0~D3) clock select:
									///< 	0x0: Power down
									///< 	0x1: 1C2D Mode x2 (C0/D0/D1 + C1/D2/D3)
									///< 	0x2: 1C4D Mode (C0/D0~D3)
									///< 	0x3: 1C8D mode (C1/D0~D7)
									///< 	     (SENPHY_CONFIG_ID_PHY_CLKSEL2 also needs to be 0x3, LVDS mode only)
	SENPHY_CONFIG_ID_PHY_CLKSEL2,	///< PHY Group2(C2~C3/D4~D7) clock select:
									///< 	0x0: Power down
									///< 	0x1: 1C2D Mode x2 (C2/D4/D5 + C3/D6/D7)
									///< 	0x2: 1C4D Mode (C2/D4~D7)
									///< 	0x3: 1C8D mode (C1/D0~D7)
									///< 	     (SENPHY_CONFIG_ID_PHY_CLKSEL also needs to be 0x3, LVDS mode only)

	SENPHY_CONFIG_CSI_MODE,			///< Senphy ctrl Mode enable: CSI
	SENPHY_CONFIG_CSI2_MODE,		///< Senphy ctrl Mode enable: CSI2
	SENPHY_CONFIG_CSI3_MODE,		///< Senphy ctrl Mode enable: CSI3
	SENPHY_CONFIG_CSI4_MODE,		///< Senphy ctrl Mode enable: CSI4
	SENPHY_CONFIG_LVDS_MODE,		///< Senphy ctrl Mode enable: LVDS
	SENPHY_CONFIG_LVDS2_MODE,		///< Senphy ctrl Mode enable: LVDS2
	SENPHY_CONFIG_LVDS3_MODE,		///< Senphy ctrl Mode enable: LVDS3
	SENPHY_CONFIG_LVDS4_MODE,		///< Senphy ctrl Mode enable: LVDS4

	SENPHY_CONFIG_ID_RTERM_D03,     ///< Set Group1(C0~C1/D0~D3) RTERM control. Valid value range from 0x0 to 0xF
	SENPHY_CONFIG_ID_RTERM_D47,     ///< Set Group2(C2~C3/D4~D7) RTERM control. Valid value range from 0x0 to 0xF

	SENPHY_CONFIG_ID_IADJ,          ///< (Group1) Analog Block Current Source adjustment.
									///< Set 0x0 is 20uA (Default value). Set 0x1 is 40uA. 0x2 is 60uA. 0x3 is 80uA.
	SENPHY_CONFIG_ID_IADJ2,         ///< (Group2) Analog Block Current Source adjustment.
									///< Set 0x0 is 20uA (Default value). Set 0x1 is 40uA. 0x2 is 60uA. 0x3 is 80uA.

	SENPHY_CONFIG_ID_INV_CK0,       ///< Clock Lane 0 pad pin P/N Swap Enable/Disable.
	SENPHY_CONFIG_ID_INV_CK1,       ///< Clock Lane 1 pad pin P/N Swap Enable/Disable.
	SENPHY_CONFIG_ID_INV_CK2,       ///< Clock Lane 2 pad pin P/N Swap Enable/Disable.
	SENPHY_CONFIG_ID_INV_CK3,       ///< Clock Lane 3 pad pin P/N Swap Enable/Disable.
	SENPHY_CONFIG_ID_INV_DAT0,      ///< Data Lane 0 pad pin P/N Swap Enable/Disable.
	SENPHY_CONFIG_ID_INV_DAT1,      ///< Data Lane 1 pad pin P/N Swap Enable/Disable.
	SENPHY_CONFIG_ID_INV_DAT2,      ///< Data Lane 2 pad pin P/N Swap Enable/Disable.
	SENPHY_CONFIG_ID_INV_DAT3,      ///< Data Lane 3 pad pin P/N Swap Enable/Disable.
	SENPHY_CONFIG_ID_INV_DAT4,      ///< Data Lane 4 pad pin P/N Swap Enable/Disable.
	SENPHY_CONFIG_ID_INV_DAT5,      ///< Data Lane 5 pad pin P/N Swap Enable/Disable.
	SENPHY_CONFIG_ID_INV_DAT6,      ///< Data Lane 6 pad pin P/N Swap Enable/Disable.
	SENPHY_CONFIG_ID_INV_DAT7,      ///< Data Lane 7 pad pin P/N Swap Enable/Disable.

	SENPHY_CONFIG_ID_DBGMSG,        ///< Sensor PHY Module Debug Message Enable/Disable. Default is disabled.


	/*
		SENPHY Lane delay setting
	*/
	SENPHY_CONFIG_ID_ENO_DLY,       ///< (Clock) Lane Enable to Valid Output Delay Count in (60/80/120)MHz.
	SENPHY_CONFIG_ID_DLY_EN,        ///< Set new PHY Delay/DeSkew Function Enable/Disable.
	SENPHY_CONFIG_ID_DLY_CLK0,      ///< Clock Lane 0 analog Delay Unit select. Valid value range from 0x0 to 0x7.
	SENPHY_CONFIG_ID_DLY_CLK1,      ///< Clock Lane 1 analog Delay Unit select. Valid value range from 0x0 to 0x7.
	SENPHY_CONFIG_ID_DLY_CLK2,      ///< Clock Lane 2 analog Delay Unit select. Valid value range from 0x0 to 0x7.
	SENPHY_CONFIG_ID_DLY_CLK3,      ///< Clock Lane 3 analog Delay Unit select. Valid value range from 0x0 to 0x7.
	SENPHY_CONFIG_ID_DLY_CLK4,      ///< Clock Lane 4 analog Delay Unit select. Valid value range from 0x0 to 0x7.
	SENPHY_CONFIG_ID_DLY_CLK5,      ///< Clock Lane 5 analog Delay Unit select. Valid value range from 0x0 to 0x7.
	SENPHY_CONFIG_ID_DLY_CLK6,      ///< Clock Lane 6 analog Delay Unit select. Valid value range from 0x0 to 0x7.
	SENPHY_CONFIG_ID_DLY_CLK7,      ///< Clock Lane 7 analog Delay Unit select. Valid value range from 0x0 to 0x7.

	SENPHY_CONFIG_ID_DLY_DAT0,      ///< Data Lane 0 analog Delay Unit select.  Valid value range from 0x0 to 0x7.
	SENPHY_CONFIG_ID_DLY_DAT1,      ///< Data Lane 1 analog Delay Unit select.  Valid value range from 0x0 to 0x7.
	SENPHY_CONFIG_ID_DLY_DAT2,      ///< Data Lane 2 analog Delay Unit select.  Valid value range from 0x0 to 0x7.
	SENPHY_CONFIG_ID_DLY_DAT3,      ///< Data Lane 3 analog Delay Unit select.  Valid value range from 0x0 to 0x7.
	SENPHY_CONFIG_ID_DLY_DAT4,      ///< Data Lane 4 analog Delay Unit select.  Valid value range from 0x0 to 0x7.
	SENPHY_CONFIG_ID_DLY_DAT5,      ///< Data Lane 5 analog Delay Unit select.  Valid value range from 0x0 to 0x7.
	SENPHY_CONFIG_ID_DLY_DAT6,      ///< Data Lane 6 analog Delay Unit select.  Valid value range from 0x0 to 0x7.
	SENPHY_CONFIG_ID_DLY_DAT7,      ///< Data Lane 7 analog Delay Unit select.  Valid value range from 0x0 to 0x7.

	/*
		SENPHY HSRX Control
	*/
	SENPHY_CONFIG_ID_SRRO_C0_D01,
	SENPHY_CONFIG_ID_SRRO_C1_D23,
	SENPHY_CONFIG_ID_SRRO_C2_D45,
	SENPHY_CONFIG_ID_SRRO_C3_D67,
	SENPHY_CONFIG_ID_ISEL_C0_D01,
	SENPHY_CONFIG_ID_ISEL_C1_D23,
	SENPHY_CONFIG_ID_ISEL_C2_D45,
	SENPHY_CONFIG_ID_ISEL_C3_D67,

	SENPHY_CONFIG_ID_C0_RS_CTLE,
	SENPHY_CONFIG_ID_C1_RS_CTLE,
	SENPHY_CONFIG_ID_C2_RS_CTLE,
	SENPHY_CONFIG_ID_C3_RS_CTLE,
	SENPHY_CONFIG_ID_D0_RS_CTLE,
	SENPHY_CONFIG_ID_D1_RS_CTLE,
	SENPHY_CONFIG_ID_D2_RS_CTLE,
	SENPHY_CONFIG_ID_D3_RS_CTLE,
	SENPHY_CONFIG_ID_D4_RS_CTLE,
	SENPHY_CONFIG_ID_D5_RS_CTLE,
	SENPHY_CONFIG_ID_D6_RS_CTLE,
	SENPHY_CONFIG_ID_D7_RS_CTLE,

	SENPHY_CONFIG_ID_C0_CS_CTLE,
	SENPHY_CONFIG_ID_C1_CS_CTLE,
	SENPHY_CONFIG_ID_C2_CS_CTLE,
	SENPHY_CONFIG_ID_C3_CS_CTLE,
	SENPHY_CONFIG_ID_D0_CS_CTLE,
	SENPHY_CONFIG_ID_D1_CS_CTLE,
	SENPHY_CONFIG_ID_D2_CS_CTLE,
	SENPHY_CONFIG_ID_D3_CS_CTLE,
	SENPHY_CONFIG_ID_D4_CS_CTLE,
	SENPHY_CONFIG_ID_D5_CS_CTLE,
	SENPHY_CONFIG_ID_D6_CS_CTLE,
	SENPHY_CONFIG_ID_D7_CS_CTLE,

	/* backward compatible */
	SENPHY_CONFIG_ID_PHY_CK_MODE,
	SENPHY_CONFIG_CK1_EN,			///< Clock lane 1 enable/disable. This shall be enabled during dual sensor application.
	SENPHY_CONFIG_CSI5_MODE,		///< Senphy2 Mode CSI5
	SENPHY_CONFIG_LVDS5_MODE,		///< Senphy2 Mode LVDS5
	SENPHY_CONFIG_ID_CURRDIV2,      ///< Current Divide by 2 function ENABLE/DISABLE.
	SENPHY_CONFIG_ID_CK0_RTERM,     ///< Clock lane 0 terminal resistance adjustment
	SENPHY_CONFIG_ID_CK1_RTERM,     ///< Clock lane 1 terminal resistance adjustment
	SENPHY_CONFIG_ID_CK2_RTERM,     ///< Clock lane 2 terminal resistance adjustment
	SENPHY_CONFIG_ID_CK3_RTERM,     ///< Clock lane 3 terminal resistance adjustment

	SENPHY_CONFIG_ID_D0_RTERM,      ///< Data lane 0 terminal resistance adjustment
	SENPHY_CONFIG_ID_D1_RTERM,      ///< Data lane 1 terminal resistance adjustment
	SENPHY_CONFIG_ID_D2_RTERM,      ///< Data lane 2 terminal resistance adjustment
	SENPHY_CONFIG_ID_D3_RTERM,      ///< Data lane 3 terminal resistance adjustment
	SENPHY_CONFIG_ID_D4_RTERM,      ///< Data lane 4 terminal resistance adjustment
	SENPHY_CONFIG_ID_D5_RTERM,      ///< Data lane 5 terminal resistance adjustment
	SENPHY_CONFIG_ID_D6_RTERM,      ///< Data lane 6 terminal resistance adjustment
	SENPHY_CONFIG_ID_D7_RTERM,      ///< Data lane 7 terminal resistance adjustment

	/*
		SENPHY#1 OFSCAL for csi 2.5Gbps
	*/
	SENPHY_CONFIG_ID_RTERM,			///< Set RTERM control. Valid value range from 0x0 to 0xF
	SENPHY_CONFIG_ID_ITRIM,			///< Set ITRIM control. Valid value range from 0x0 to 0x1F
	SENPHY_CONFIG_ID_EOC_EN,		///< EOC function enable / disable

	SENPHY_CONFIG_ID_D0DLY_MODE,	///< Data lane 0 delay mode select. 0: HW control, 1: SW control by DAT0_DLYADJ.
	SENPHY_CONFIG_ID_D1DLY_MODE,	///< Data lane 1 delay mode select. 0: HW control, 1: SW control by DAT1_DLYADJ.
	SENPHY_CONFIG_ID_D2DLY_MODE,	///< Data lane 2 delay mode select. 0: HW control, 1: SW control by DAT2_DLYADJ.
	SENPHY_CONFIG_ID_D3DLY_MODE,	///< Data lane 3 delay mode select. 0: HW control, 1: SW control by DAT3_DLYADJ.

	SENPHY_CONFIG_ID_CLK0_DLYADJ,	///< Clock lane 0 skew adjustment control signal. Valid value range from 0x0 to 0x2F
	SENPHY_CONFIG_ID_DAT0_DLYADJ,	///< Data lane 0 skew adjustment control signal. Valid value range from 0x0 to 0x2F
	SENPHY_CONFIG_ID_DAT1_DLYADJ,	///< Data lane 1 skew adjustment control signal. Valid value range from 0x0 to 0x2F
	SENPHY_CONFIG_ID_DAT2_DLYADJ,	///< Data lane 2 skew adjustment control signal. Valid value range from 0x0 to 0x2F
	SENPHY_CONFIG_ID_DAT3_DLYADJ,	///< Data lane 3 skew adjustment control signal. Valid value range from 0x0 to 0x2F

	SENPHY_CONFIG_ID_EQ_ISEL_G0,		///< EOC scan parameter EQ_ISEL G0~G3
	SENPHY_CONFIG_ID_EQ_ISEL_G1,
	SENPHY_CONFIG_ID_EQ_ISEL_G2,
	SENPHY_CONFIG_ID_EQ_ISEL_G3,

	SENPHY_CONFIG_ID_EQ_CLK_G0,			///< EOC scan parameter EQ_TEST_CLK_G0~G3
	SENPHY_CONFIG_ID_EQ_CLK_G1,
	SENPHY_CONFIG_ID_EQ_CLK_G2,
	SENPHY_CONFIG_ID_EQ_CLK_G3,

	SENPHY_CONFIG_ID_EQ_D0_G0,			///< EOC scan parameter EQ_TEST_D0_G0~G3
	SENPHY_CONFIG_ID_EQ_D0_G1,
	SENPHY_CONFIG_ID_EQ_D0_G2,
	SENPHY_CONFIG_ID_EQ_D0_G3,

	SENPHY_CONFIG_ID_EQ_D1_G0,			///< EOC scan parameter EQ_TEST_D1_G0~G3
	SENPHY_CONFIG_ID_EQ_D1_G1,
	SENPHY_CONFIG_ID_EQ_D1_G2,
	SENPHY_CONFIG_ID_EQ_D1_G3,

	SENPHY_CONFIG_ID_EQ_D2_G0,			///< EOC scan parameter EQ_TEST_D2_G0~G3
	SENPHY_CONFIG_ID_EQ_D2_G1,
	SENPHY_CONFIG_ID_EQ_D2_G2,
	SENPHY_CONFIG_ID_EQ_D2_G3,

	SENPHY_CONFIG_ID_EQ_D3_G0,			///< EOC scan parameter EQ_TEST_D3_G0~G3
	SENPHY_CONFIG_ID_EQ_D3_G1,
	SENPHY_CONFIG_ID_EQ_D3_G2,
	SENPHY_CONFIG_ID_EQ_D3_G3,

	SENPHY_CONFIG_ID_CSI_EQ_TEST_CLK,	///< Select EQ_CLK_Gx channel.
	SENPHY_CONFIG_ID_CSI_EQ_TEST_D0,	///< Select EQ_D0_Gx channel.
	SENPHY_CONFIG_ID_CSI_EQ_TEST_D1,	///< Select EQ_D1_Gx channel.
	SENPHY_CONFIG_ID_CSI_EQ_TEST_D2,	///< Select EQ_D2_Gx channel.
	SENPHY_CONFIG_ID_CSI_EQ_TEST_D3,	///< Select EQ_D3_Gx channel.
	SENPHY_CONFIG_ID_CSI_EQ_SW,
	SENPHY_CONFIG_ID_CSI_AI_PRE_AMP,	///< Speed control. 0: DATA_RATE <= 2Gbps, 1: DATA_RATE > 2Gbps
	SENPHY_CONFIG_ID_CSI_EQ_ISEL,		///< EQ_ID & Common mode select.
	SENPHY_CONFIG_ID_OFSCAL_CTL_SEL,	///< OFSCAL control select. 0: from EOC, 1: from CSI_EQ_XXX

	SENPHY_CONFIG_ID_CSI_EQ_ICTRL_CLK,	///< CLK EQ IDAC current control. Valid value range from 0x0 to 0x1F
	SENPHY_CONFIG_ID_CSI_EQ_ICTRL_D0,	///<  D0 EQ IDAC current control. Valid value range from 0x0 to 0x1F
	SENPHY_CONFIG_ID_CSI_EQ_ICTRL_D1,	///<  D1 EQ IDAC current control. Valid value range from 0x0 to 0x1F
	SENPHY_CONFIG_ID_CSI_EQ_ICTRL_D2,	///<  D2 EQ IDAC current control. Valid value range from 0x0 to 0x1F
	SENPHY_CONFIG_ID_CSI_EQ_ICTRL_D3,	///<  D3 EQ IDAC current control. Valid value range from 0x0 to 0x1F

	SENPHY_CONFIG_ID_EOC_TRIM_NO,		///< Select EOC trim no. Valid value range from 0x0 to 0x7

	SENPHY_CONFIG_ID_CK0S_SRRO,         ///< Clock lane 0 equalizer output slew rate control
	SENPHY_CONFIG_ID_CK1S_SRRO,         ///< Clock lane 1 equalizer output slew rate control
	SENPHY_CONFIG_ID_CK2S_SRRO,         ///< Clock lane 2 equalizer output slew rate control
	SENPHY_CONFIG_ID_CK3S_SRRO,         ///< Clock lane 3 equalizer output slew rate control

	SENPHY_CONFIG_ID_D0S_SRRO,          ///< Data lane 0 equalizer output slew rate control
	SENPHY_CONFIG_ID_D1S_SRRO,          ///< Data lane 1 equalizer output slew rate control
	SENPHY_CONFIG_ID_D2S_SRRO,          ///< Data lane 2 equalizer output slew rate control
	SENPHY_CONFIG_ID_D3S_SRRO,          ///< Data lane 3 equalizer output slew rate control
	SENPHY_CONFIG_ID_D4S_SRRO,          ///< Data lane 4 equalizer output slew rate control
	SENPHY_CONFIG_ID_D5S_SRRO,          ///< Data lane 5 equalizer output slew rate control
	SENPHY_CONFIG_ID_D6S_SRRO,          ///< Data lane 6 equalizer output slew rate control
	SENPHY_CONFIG_ID_D7S_SRRO,          ///< Data lane 7 equalizer output slew rate control

	ENUM_DUMMY4WORD(SENPHY_CONFIG_ID)
} SENPHY_CONFIG_ID;



extern ER       senphy_set_config(SENPHY_CONFIG_ID config_id, UINT32 config_value);
extern ER       senphy2_set_config(SENPHY_CONFIG_ID config_id, UINT32 config_value);
extern ER       senphy3_set_config(SENPHY_CONFIG_ID config_id, UINT32 config_value);

extern UINT32   senphy_get_config(SENPHY_CONFIG_ID config_id);

/*
    Shall be used by csi/lvds only
*/
typedef enum {
	SENPHY_SEL_MIPILVDS,
	SENPHY_SEL_MIPILVDS2,
	SENPHY_SEL_MIPILVDS3,
	SENPHY_SEL_MIPILVDS4,
	SENPHY_SEL_MIPILVDS5,
	SENPHY_SEL_MIPILVDS6,
	SENPHY_SEL_MIPILVDS7,
	SENPHY_SEL_MIPILVDS8,

	ENUM_DUMMY4WORD(SENPHY_SEL)
} SENPHY_SEL;


typedef enum {
	SENPHY_DATASEL_D0       = 0x0001,
	SENPHY_DATASEL_D1       = 0x0002,
	SENPHY_DATASEL_D2       = 0x0004,
	SENPHY_DATASEL_D3       = 0x0008,
	SENPHY_DATASEL_D4       = 0x0010,
	SENPHY_DATASEL_D5       = 0x0020,
	SENPHY_DATASEL_D6       = 0x0040,
	SENPHY_DATASEL_D7       = 0x0080,

	ENUM_DUMMY4WORD(SENPHY_DATASEL)
} SENPHY_DATASEL;

typedef enum {
	SENPHY_CLKMAP_CK0,
	SENPHY_CLKMAP_CK1,
	SENPHY_CLKMAP_CK2,
	SENPHY_CLKMAP_CK3,
	SENPHY_CLKMAP_CK4,
	SENPHY_CLKMAP_CK5,
	SENPHY_CLKMAP_CK6,
	SENPHY_CLKMAP_CK7,

	SENPHY_CLKMAP_MAX,
	SENPHY_CLKMAP_OFF = SENPHY_CLKMAP_MAX,
	ENUM_DUMMY4WORD(SENPHY_CLKMAP)
} SENPHY_CLKMAP;

typedef enum {
	SENPHY_CLK_SEL_POWEWR_DONE = 0,
	SENPHY_CLK_SEL_1C2D,
	SENPHY_CLK_SEL_1C4D,
	SENPHY_CLK_SEL_1C8D,

	ENUM_DUMMY4WORD(SENPHY_PHY_CLK_SEL)
} SENPHY_PHY_CLK_SEL;

typedef enum {
	SENPHY_LANE_GROUP0 = 0,	// C0~C1, D0~D3
	SENPHY_LANE_GROUP1,		// C2~C3, D4~D7

	ENUM_DUMMY4WORD(SENPHY_LANE_GROUP)
} SENPHY_LANE_GROUP;

typedef enum {
	SENPHY_HSRX_MODE_CSI = 0,
	SENPHY_HSRX_MODE_LVDS,

	ENUM_DUMMY4WORD(SENPHY_HSRX_MODE)
} SENPHY_HSRX_MODE;

typedef enum {
	SENPHY_HSRX_RATE_LV0 = 0, // Lv0: data rate <=  500Mbps
	SENPHY_HSRX_RATE_LV1,     // Lv1: data rate  =  501Mbps ~ 1000Mpbs
	SENPHY_HSRX_RATE_LV2,     // Lv2: data rate  = 1001Mbps ~ 1500Mbps

	ENUM_DUMMY4WORD(SENPHY_HSRX_RATE)
} SENPHY_HSRX_RATE;

extern void     senphy_init(void);
extern ER       senphy_set_power(SENPHY_SEL phy_select, BOOL b_enable);
extern ER       senphy_enable(SENPHY_SEL phy_select, BOOL b_enable);
extern ER       senphy_set_valid_lanes(SENPHY_SEL phy_select, SENPHY_DATASEL data_select);
extern ER       senphy_set_clock_map(SENPHY_SEL phy_select, SENPHY_CLKMAP clk_select);
extern ER       senphy_set_clock2_map(SENPHY_SEL phy_select, SENPHY_CLKMAP clk_select);
extern void     senphy_lane_pn_swap(void);
extern void     senphy_hsrx_control(SENPHY_LANE_GROUP grp, SENPHY_HSRX_MODE mode, SENPHY_HSRX_RATE lv);

extern void     senphy2_init(void);
extern ER       senphy2_set_power(SENPHY_SEL phy_select, BOOL b_enable);
extern ER       senphy2_enable(SENPHY_SEL phy_select, BOOL b_enable);
extern ER       senphy2_set_valid_lanes(SENPHY_SEL phy_select, SENPHY_DATASEL data_select);
extern ER       senphy2_set_clock_map(SENPHY_SEL phy_select, SENPHY_CLKMAP clk_select);
extern ER       senphy2_set_clock2_map(SENPHY_SEL phy_select, SENPHY_CLKMAP clk_select);

extern void     senphy3_init(void);
extern ER       senphy3_set_power(SENPHY_SEL phy_select, BOOL b_enable);
extern ER       senphy3_enable(SENPHY_SEL phy_select, BOOL b_enable);
extern ER       senphy3_set_valid_lanes(SENPHY_SEL phy_select, SENPHY_DATASEL data_select);
extern ER       senphy3_set_clock_map(SENPHY_SEL phy_select, SENPHY_CLKMAP clk_select);
extern ER       senphy3_set_clock2_map(SENPHY_SEL phy_select, SENPHY_CLKMAP clk_select);


//=============================================================================
// EOC scan function
//=============================================================================

#define SENPHY_EOC_SCAN_DBG  0

/**
	EOC scan parameter:

		|   DC gain |   DATA_RATE <= 2Gbps  |   DATA_RATE > 2Gbps   |
		|   amp     |           0           |           1           |
		|   eq_test |     G0    |     G1    |     G2    |    G3     |
		|   eq_isel |  G0 |  G1 |  G0 |  G1 |  G2 |  G3 |  G2 |  G3 |
		| Result No.|   0 |   1 |   2 |   3 |   4 |   5 |   6 |   7 |

*/
typedef struct {
	UINT32        EQ_ISEL[4];	// eq_isel G0~G3
	UINT32        EQ_CLK[4];	// eq_test_clk G0~G3
	UINT32        EQ_D0[4];		// eq_test_d0 G0~G3
	UINT32        EQ_D1[4];		// eq_test_d1 G0~G3
	UINT32        EQ_D2[4];		// eq_test_d2 G0~G3
	UINT32        EQ_D3[4];		// eq_test_d3 G0~G3

	UINT32        RTERM;
	UINT32        ITRIM;
} CSI_EOC_PARMS;

/**
	EOC scan result

	[index]: scan result No. 0 ~ 7
*/
typedef struct {
	UINT32        Result_clk[8];
	UINT32        Result_d0[8];
	UINT32        Result_d1[8];
	UINT32        Result_d2[8];
	UINT32        Result_d3[8];
} CSI_EOC_SCAN_RESULT;

#if 1 //defined(__FREERTOS)
extern void       senphy_eoc_parameter_init (void);
extern ER         senphy_eoc_scan (void);
extern ER         senphy_eoc_get_scan_result (CSI_EOC_SCAN_RESULT *test);
extern void       senphy_eoc_set_trim(void);
#endif

//=============================================================================

#endif
