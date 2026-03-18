/*
    LVDS/MIPI-CSI/HiSPi Sensor PHY Configuration Driver Global header

    LVDS/MIPI-CSI/HiSPi Sensor PHY Configuration Driver Global header

    @file       senphy.h
    @ingroup
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _DSIPHY_H
#define _DSIPHY_H


#include "kwrap/type.h"


#if (defined __FREERTOS)
#define DSIPHY_DATA_TYPE	UINT32
#else
#define DSIPHY_DATA_TYPE	UINT64
#endif


#define DSIPHY_PLL_DEFAULT_CLK                  480000000   // Hz
#define DSIPHY_PLL_CLK_MAX                      960000000   // Hz
#define DSIPHY_PLL_BAND_THD                     1100000000  // Hz
#define DSIPHY_HS_CLK_MAX                       2500000000  // Hz



typedef enum {
	DSIPHY_ID_1 = 0X0,	///< DSIPHY ID 1
	DSIPHY_ID_2,        ///< DSIPHY ID 2

	DSIPHY_ID_CNT,
	ENUM_DUMMY4WORD(DSIPHY_ID)
} DSIPHY_ID;

/**
    DSIPHY Functional Configuration Selection

    This definition is used in dsiphy_set_config()
    to assign new configurations.
*/
typedef enum {
	DSIPHY_CONFIG_ID_PHASE_DELAY_ENABLE,
	DSIPHY_CONFIG_ID_DAT0_PHASE_OFS,
	DSIPHY_CONFIG_ID_DAT1_PHASE_OFS,
	DSIPHY_CONFIG_ID_DAT2_PHASE_OFS,
	DSIPHY_CONFIG_ID_DAT3_PHASE_OFS,
	DSIPHY_CONFIG_ID_PHY_DRVING,
		
	DSIPHY_CONFIG_ID_CLK_PHASE_OFS,
	DSIPHY_CONFIG_ID_PHY_HS_CLK_INV,
	DSIPHY_CONFIG_ID_PHY_LP_RX_DAT0,
	
	ENUM_DUMMY4WORD(DSIPHY_CONFIG_ID)
} DSIPHY_CONFIG_ID;	

extern void     dsiphy_init(void);
extern void		dsiphy_set_base_addr(DSIPHY_DATA_TYPE addr);
extern void 	dsiphy_enable_config(DSIPHY_ID id, unsigned long dsiphy_freq );
extern void 	dsiphy_set_enable(DSIPHY_ID id, BOOL b_en);
extern void 	dsiphy_set_phase(int id);
extern void		dsiphy_open(DSIPHY_ID id, unsigned long dsiphy_freq );
extern UINT32	dsiphy_get_config(DSIPHY_ID id , DSIPHY_CONFIG_ID cfg_id);
extern ER		dsiphy_set_config(DSIPHY_ID id , DSIPHY_CONFIG_ID cfg_id, UINT32 ui_config);
extern int 		dsiphy_check_boostrap(void);

#if defined(__LINUX)

#endif
//=============================================================================
// EOC scan function
//=============================================================================

#define SENPHY_EOC_SCAN_DBG  0


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



//=============================================================================

#endif
