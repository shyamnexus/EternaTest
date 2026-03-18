/*
    LVDS/MIPI-CSI/HiSPi Sensor PHY Configuration Driver

    LVDS/MIPI-CSI/HiSPi Sensor PHY Configuration Driver

    @file       senphy.c
    @ingroup
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifdef __KERNEL__
#include <rcw_macro.h>
#include <kwrap/type.h>
#include <kwrap/semaphore.h>
#include <kwrap/flag.h>
#include <kwrap/spinlock.h>

#include "dsiphy_int.h"
#include "dsiphy_dbg.h"
#include "rule_check.h"
#include "plat/pad.h"
//#include "../csi.h"
//#include "../csi/include/csi_platform.h"
#include <plat/efuse_protected.h>
#include "dsiphy_platform.h"
#include <plat/hardware.h>
//extern void *csi_platform_ioremap(unsigned long addr, unsigned int size);
#else
#include <kwrap/nvt_type.h>
#include <kwrap/error_no.h>
#include <kwrap/spinlock.h>
#include "dsiphy_int.h"
#include "rule_check.h"
#include "pad.h"
#include "efuse_protected.h"
#include "include/dsiphy_platform.h"
#define __MODULE__    rtos_senphy
#define __DBGLVL__ 8
#include <kwrap/debug.h>
#endif
#include "dsiphy.h"
#include <kwrap/util.h>

#define DSIPHY2_T 0

DSIPHY_DATA_TYPE _DSIPHY_REG_BASE_ADDR;
unsigned long dsiphy_ck1_freq = 0;
unsigned long dsiphy_ck2_freq = 0;

void dsiphy_set_base_addr(DSIPHY_DATA_TYPE addr)
{
    nvt_dbg(IND, "addr = 0x%x\n",(UINT32)addr );
	_DSIPHY_REG_BASE_ADDR = addr;
}
/* only usr for disphy frequency up to 1.5G */
void dsiphy_read_trim(UINT32* dsiphy_hs_data_driving  , UINT32* dsiphy_hs_clk_driving )
{
#if 0 // FPGA Not ready
	// read efuse trim 
	UINT16    value;
	INT32     trim;
	//INT32     addr = 0X3A;

	trim = efuse_readParamOps(EFUSE_DSITXPLL_RG_LEVEL_LDO_TRIM_DATA, &value);
	if (trim == EFUSE_SUCCESS) {
		//DBG_ERR("DSI TX output swing(data/clk) Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
		//bit[3..0]
		*dsiphy_hs_data_driving = (value & 0x000F);
		//bit[7..4]
		*dsiphy_hs_clk_driving = ((value >> 4) & 0x000F);
		//DBG_FUNC(" DSIPHY dsiphy_hs_data_driving Trim data success [0x%04x]\r\n", *dsiphy_hs_data_driving);
		//DBG_FUNC(" DSIPHY dsiphy_hs_clk_driving Trim data success [0x%04x]\r\n", *dsiphy_hs_clk_driving);
	} else {
#endif
		//DBG_ERR("addr[%02x] error %d\r\n", (int)addr, (int)value);	
		// Default value data (0x08) bit11 = 0 bit18..16 = 2  
		*dsiphy_hs_data_driving = 6;
		// Default value clock (0x08) bit10 = 1 bit14..12 = 1  
		*dsiphy_hs_clk_driving = 1;
#if 0 // FPGA Not ready
	}
#endif
}

// Fine tune phy setting from SE //
void dsiphy_enable_config(DSIPHY_ID id, unsigned long dsiphy_freq )
{
	UINT32	dsiphy_data_driving=0, dsiphy_clock_driving=0;
    T_DSIPHY_CONTROL_REG      dsiphy_ctrl_reg;

	UINT32 target_freq = dsiphy_freq;
	 
	dsiphy_ctrl_reg.reg		= DSIPHY_GETREG(DSIPHY_CONTROL_REG_OFS);
	 
	if (target_freq > 960000000) {
		// > 960M 
		dsiphy_ctrl_reg.bit.DSIPHY_EN_PEM 	= 0X1;
		dsiphy_ctrl_reg.bit.DSIPHY_SR 		= 0X1;
		dsiphy_ctrl_reg.bit.DSIPHY_SF		= 0X1;
	} else { // <= 960M // Driving default value
		dsiphy_ctrl_reg.bit.DSIPHY_EN_PEM 	= 0x0;
		dsiphy_ctrl_reg.bit.DSIPHY_SR 		= 0X0;
		dsiphy_ctrl_reg.bit.DSIPHY_SF		= 0X0;
	}

	// apply trim value
	if (target_freq > 1500000000) {
		// read dsiphy efuse trim
		dsiphy_read_trim(&dsiphy_data_driving , &dsiphy_clock_driving );

		// data lane driving bit11 bit18..16
		dsiphy_ctrl_reg.bit.DSIPHY_ISEL_D = (dsiphy_data_driving >> 3) & 0x0001;
		dsiphy_ctrl_reg.bit.DSIPHY_IADJ_D = dsiphy_data_driving & 0x0007;

		//clock lane driving bit10 bit14..12
		dsiphy_ctrl_reg.bit.DSIPHY_ISEL_C = (dsiphy_clock_driving >> 3) & 0x0001;
		dsiphy_ctrl_reg.bit.DSIPHY_IADJ_C = dsiphy_clock_driving & 0x0007;
	} else {
		// data lane driving bit11 bit18..16
		dsiphy_ctrl_reg.bit.DSIPHY_ISEL_D	= 0x0;
		dsiphy_ctrl_reg.bit.DSIPHY_IADJ_D	= 0x2;

		//clock lane driving bit10 bit14..12
		dsiphy_ctrl_reg.bit.DSIPHY_ISEL_C	= 0x0;
		dsiphy_ctrl_reg.bit.DSIPHY_IADJ_C	= 0x2;
	}

	DSIPHY_SETREG(DSIPHY_CONTROL_REG_OFS, dsiphy_ctrl_reg.reg);

}
	

void dsiphy_set_enable(DSIPHY_ID id , BOOL b_en)
{
	T_DSIPHY_ENABLE_REG    reg_enable;

	reg_enable.reg = DSIPHY_GETREG(DSIPHY_ENABLE_REG_OFS);

	if (reg_enable.bit.DSIPHY_EN != b_en) {
        //DBG_WRN(" dsiphy_set_phy_enable:0x%x 0x%x\r\n", reg_enable.reg, b_en);
		reg_enable.bit.DSIPHY_EN = b_en;
		DSIPHY_SETREG(DSIPHY_ENABLE_REG_OFS, reg_enable.reg);
	} else {
		 DBG_IND("%s.dsiphy already Enable \r\n",__func__);
	}

	if (b_en == FALSE) {
		dsiphy_ck1_freq = 0;
		dsiphy_ck2_freq = 0;
	}

}

int dsiphy_check_boostrap(void)
{
	T_DSIPHY_CURRENT_REG    dsiphy_current_reg;
	int BST, ret;

	/* Disable MIPI TX when DSI_PROT_EN BST[7] is 0 */
#ifdef __KERNEL__
	void __iomem *top_reg_addr = NULL;
	top_reg_addr = ioremap(NVT_TOP_BASE_PHYS, 0x1000);
	BST = readl(top_reg_addr);
	ret = -ENODEV;
#else
	BST = INW(IOADDR_TOP_REG_BASE);
	ret = E_SYS;
#endif

	if (BST & (0x1 << 7)) {
		//Set bypass before phy_en
		dsiphy_current_reg.reg 	= DSIPHY_GETREG(DSIPHY_CURRENT_REG_OFS);
		dsiphy_current_reg.bit.DSIPHY_LDO_OUTPUT_BYPASS = 0x1;
		DSIPHY_SETREG(DSIPHY_CURRENT_REG_OFS, dsiphy_current_reg.reg);
		ret = 0;
	} else {
		nvt_dbg(ERR, "Disable MIPI TX when DSI_PROT_EN BST[7] is 0\n");
	}

	return ret;
}

void dsiphy_open(DSIPHY_ID id, unsigned long dsiphy_freq)
{
#if  !defined(_NVT_FPGA_)
	T_DSIPHY_CONTROL_REG    dsiphy_ctrl_reg;
	T_DSIPHY_CURRENT_REG    dsiphy_current_reg;
	T_DSIPHY_CTRL_PLL1_REG  dsiphy_ctrl_pll_1_reg;
	T_DSIPHY_CTRL_CLOCK_REG dsiphy_clk_reg;
#endif
	UINT32 target_freq = dsiphy_freq;

    DBG_DUMP("phy_enable_script freq: %lld\r\n", (long long int)target_freq);

#if !defined(CONFIG_NVT_FPGA_EMULATION) && !defined(_NVT_FPGA_) && !defined(CONFIG_EMULATION) && defined(_BSP_NS02201_)
	if (dsiphy_check_boostrap() < 0) {
		nvt_dbg(ERR, "DSI_PROT_EN BST[7] is 0, DSIPHY open fail\n");
		return;
	}
#endif
	if (id == DSIPHY_ID_1){
		dsiphy_ck1_freq = dsiphy_freq;
	} else if (id == DSIPHY_ID_2) {
		dsiphy_ck2_freq = dsiphy_freq;
	}
#if  !defined(_NVT_FPGA_) 
	// set swing driving 
	// if (dsiphy_ck1_freq >=  dsiphy_ck2_freq){
	// 	dsiphy_enable_config(id , dsiphy_ck1_freq);
	// } else {
	// 	dsiphy_enable_config(id , dsiphy_ck2_freq);
	// }
#endif
	// dsiphy_set_enable(id , TRUE);
   	//Delay_DelayUs(1000);

#if  !defined(_NVT_FPGA_) 

	dsiphy_ctrl_reg.reg		= DSIPHY_GETREG(DSIPHY_CONTROL_REG_OFS);
	dsiphy_current_reg.reg 	= DSIPHY_GETREG(DSIPHY_CURRENT_REG_OFS);
	dsiphy_current_reg.bit.DSIPHY_LP_VSEL = 1;

	// > 960 M
    if (target_freq > DSIPHY_PLL_CLK_MAX) {
		if (id == DSIPHY_ID_1){
	        target_freq = target_freq / 8;
	        dsiphy_platform_clk_set_freq(target_freq);

			dsiphy_ctrl_pll_1_reg.reg	= DSIPHY_GETREG(DSIPHY_CTRL1_PLL1_REG_OFS);
			dsiphy_clk_reg.reg			= DSIPHY_GETREG(DSIPHY_CTRL1_CLOCK_REG_OFS);

			dsiphy_ctrl_pll_1_reg.bit.REG_DSITXPLL_EN = 0x1;
			dsiphy_clk_reg.bit.PHY_HS_CLK_INV = 0x0;
			// >960 M
			// dsiphy_clk_reg.bit.DSIPHY_CKIN_SEL = 0x1;
			dsiphy_ctrl_pll_1_reg.bit.REG_DSITXPLL_RESETB = 0x1;
			dsiphy_clk_reg.bit.PHY_SEL_CKIN = 0x1;

			DSIPHY_SETREG(DSIPHY_CONTROL_REG_OFS, dsiphy_ctrl_reg.reg);
			DSIPHY_SETREG(DSIPHY_CURRENT_REG_OFS, dsiphy_current_reg.reg);
			DSIPHY_SETREG(DSIPHY_CTRL1_CLOCK_REG_OFS, dsiphy_clk_reg.reg);
			DSIPHY_SETREG(DSIPHY_CTRL1_PLL1_REG_OFS, dsiphy_ctrl_pll_1_reg.reg);
		} else if (id == DSIPHY_ID_2){ // 2C4D
		#if DSIPHY2_T
		 	target_freq = target_freq / 32;
	        dsiphy2_platform_clk_set_freq(target_freq);

			dsiphy_ctrl_pll_1_reg.reg	= DSIPHY_GETREG(DSIPHY_CTRL2_PLL1_REG_OFS);
			dsiphy_ctrl_pll_2_reg.reg	= DSIPHY_GETREG(DSIPHY_CTRL2_PLL2_REG_OFS);
			dsiphy_clk_reg.reg			= DSIPHY_GETREG(DSIPHY_CTRL2_CLOCK_REG_OFS);

			dsiphy_ctrl_reg.bit.DSIPHY_PDO = 0x1;
			dsiphy_current_reg.bit.DSIPHY_LDO_OUTPUT_BYPASS = 0x1;
			dsiphy_ctrl_pll_1_reg.bit.DSIPHY_EN_LDO = 0x1;
			dsiphy_ctrl_reg.bit.DSIPHY_CLK_SEL_C = 0x2;
			dsiphy_ctrl_reg.bit.DSIPHY_CLK_SEL_D = 0x5;
			dsiphy_clk_reg.bit.DSIPHY_HS_CLK_INV = 0x1;
			// >960 M
			dsiphy_clk_reg.bit.DSIPHY_CKIN_SEL = 0x1;
			dsiphy_ctrl_pll_1_reg.bit.DSIPHY_PLL_PDB = 0x1;
			dsiphy_ctrl_pll_2_reg.bit.DSIPHY_PLL_TEST_DIV_POST = 0x1;

			DSIPHY_SETREG(DSIPHY_CONTROL_REG_OFS, dsiphy_ctrl_reg.reg);
			DSIPHY_SETREG(DSIPHY_CURRENT_REG_OFS, dsiphy_current_reg.reg);
			DSIPHY_SETREG(DSIPHY_CTRL2_CLOCK_REG_OFS, dsiphy_clk_reg.reg);
			DSIPHY_SETREG(DSIPHY_CTRL2_PLL1_REG_OFS, dsiphy_ctrl_pll_1_reg.reg);
			DSIPHY_SETREG(DSIPHY_CTRL2_PLL2_REG_OFS, dsiphy_ctrl_pll_2_reg.reg);
		#endif
		}

	// < 960 M
    } else {
    	if (id == DSIPHY_ID_1){
	        dsiphy_platform_clk_set_freq(target_freq);

			dsiphy_clk_reg.reg			= DSIPHY_GETREG(DSIPHY_CTRL1_CLOCK_REG_OFS);
			dsiphy_ctrl_pll_1_reg.reg	= DSIPHY_GETREG(DSIPHY_CTRL1_PLL1_REG_OFS);
			
			dsiphy_ctrl_pll_1_reg.bit.REG_DSITXPLL_EN = 0x1;
			dsiphy_clk_reg.bit.PHY_HS_CLK_INV = 0x0;

			dsiphy_ctrl_pll_1_reg.bit.REG_DSITXPLL_RESETB = 0x0;
			dsiphy_clk_reg.bit.PHY_SEL_CKIN = 0x0;
			
			DSIPHY_SETREG(DSIPHY_CTRL1_CLOCK_REG_OFS, dsiphy_clk_reg.reg);
			DSIPHY_SETREG(DSIPHY_CONTROL_REG_OFS, dsiphy_ctrl_reg.reg);
			DSIPHY_SETREG(DSIPHY_CURRENT_REG_OFS, dsiphy_current_reg.reg);
			DSIPHY_SETREG(DSIPHY_CTRL1_PLL1_REG_OFS, dsiphy_ctrl_pll_1_reg.reg);
			
    	}else if (id == DSIPHY_ID_2){ // 2C4D
		#if DSIPHY2_T
	       	dsiphy2_platform_clk_set_freq(target_freq);

			dsiphy_clk_reg.reg			= DSIPHY_GETREG(DSIPHY_CTRL2_CLOCK_REG_OFS);
			dsiphy_ctrl_pll_1_reg.reg	= DSIPHY_GETREG(DSIPHY_CTRL2_PLL1_REG_OFS);
			
			dsiphy_ctrl_reg.bit.DSIPHY_PDO = 0x1;
			dsiphy_current_reg.bit.DSIPHY_LDO_OUTPUT_BYPASS = 0x1;
			dsiphy_ctrl_pll_1_reg.bit.DSIPHY_EN_LDO = 0x1;
			dsiphy_ctrl_reg.bit.DSIPHY_CLK_SEL_C = 0x2;
			dsiphy_ctrl_reg.bit.DSIPHY_CLK_SEL_D = 0x5;
			dsiphy_clk_reg.bit.DSIPHY_HS_CLK_INV = 0x1;
			
			DSIPHY_SETREG(DSIPHY_CTRL2_CLOCK_REG_OFS, dsiphy_clk_reg.reg);
			DSIPHY_SETREG(DSIPHY_CONTROL_REG_OFS, dsiphy_ctrl_reg.reg);
			DSIPHY_SETREG(DSIPHY_CURRENT_REG_OFS, dsiphy_current_reg.reg);
			DSIPHY_SETREG(DSIPHY_CTRL2_PLL1_REG_OFS, dsiphy_ctrl_pll_1_reg.reg);
		#endif
		}
    }
#endif
}
void dsiphy_set_phase(int id)
{

}


UINT32 dsiphy_get_config(DSIPHY_ID id , DSIPHY_CONFIG_ID cfg_id)
{

	switch (cfg_id) {

	case DSIPHY_CONFIG_ID_PHASE_DELAY_ENABLE: {
			return DSIPHY_GETREG(DSIPHY_ENABLE_REG_OFS);
		}
		break;

	case DSIPHY_CONFIG_ID_DAT0_PHASE_OFS: {
			return DSIPHY_GETREG(DSIPHY_DELAY_REG_OFS);
		}
		break;

	case DSIPHY_CONFIG_ID_DAT1_PHASE_OFS: {
			return DSIPHY_GETREG(DSIPHY_DELAY_REG_OFS);
		}
		break;

	case DSIPHY_CONFIG_ID_DAT2_PHASE_OFS: {
			return DSIPHY_GETREG(DSIPHY_DELAY_REG_OFS);
		}
		break;

	case DSIPHY_CONFIG_ID_DAT3_PHASE_OFS: {
			return DSIPHY_GETREG(DSIPHY_DELAY_REG_OFS);
		}
		break;

	case DSIPHY_CONFIG_ID_PHY_DRVING: {
			return DSIPHY_GETREG(DSIPHY_CURRENT_REG_OFS);
		}
		break;

	case DSIPHY_CONFIG_ID_CLK_PHASE_OFS: {
			if (id == DSIPHY_ID_1){
				return DSIPHY_GETREG(DSIPHY_CTRL1_CLOCK_REG_OFS);
			}else if (id == DSIPHY_ID_2){
				return DSIPHY_GETREG(DSIPHY_CTRL2_CLOCK_REG_OFS);
			}
		}
		break;

	case DSIPHY_CONFIG_ID_PHY_HS_CLK_INV: {
			if (id == DSIPHY_ID_1){
				return DSIPHY_GETREG(DSIPHY_CTRL1_CLOCK_REG_OFS);
			}else if (id == DSIPHY_ID_2){
				return DSIPHY_GETREG(DSIPHY_CTRL2_CLOCK_REG_OFS);
			}
		}
		break;

	case DSIPHY_CONFIG_ID_PHY_LP_RX_DAT0: {
			if (id == DSIPHY_ID_1){
				return DSIPHY_GETREG(DSIPHY_CTRL1_CONTROLLER_REG_OFS);
			}else if (id == DSIPHY_ID_2){
				return DSIPHY_GETREG(DSIPHY_CTRL2_CONTROLLER_REG_OFS);
			}
		}
		break;


	default:
		DBG_WRN("CfgID Err = %d\r\n", (int)cfg_id);
		return E_SYS;
	}


	return E_SYS;
	// return E_OK;
}

ER dsiphy_set_config(DSIPHY_ID id , DSIPHY_CONFIG_ID cfg_id, UINT32 ui_config)
{
	
	T_DSIPHY_ENABLE_REG    			dsiphy_en_reg;
	T_DSIPHY_CTRL_CLOCK_REG			dsiphy_ctrl_clk_reg;
	T_DSIPHY_DELAY_REG      		dsiphy_delay_reg;
	T_DSIPHY_CTRL_CONTROLLER_REG 	dsiphy_ctrl_controller_reg;

	if (id == DSIPHY_ID_1){
		dsiphy_en_reg.reg		= DSIPHY_GETREG(DSIPHY_ENABLE_REG_OFS);
		dsiphy_delay_reg.reg	= DSIPHY_GETREG(DSIPHY_DELAY_REG_OFS);

		dsiphy_ctrl_clk_reg.reg = DSIPHY_GETREG(DSIPHY_CTRL1_CLOCK_REG_OFS);
		dsiphy_ctrl_controller_reg.reg = DSIPHY_GETREG(DSIPHY_CTRL1_CONTROLLER_REG_OFS);
		
	}else if (id == DSIPHY_ID_2){
		dsiphy_en_reg.reg		= DSIPHY_GETREG(DSIPHY_ENABLE_REG_OFS);
		dsiphy_delay_reg.reg	= DSIPHY_GETREG(DSIPHY_DELAY_REG_OFS);

		dsiphy_ctrl_clk_reg.reg = DSIPHY_GETREG(DSIPHY_CTRL2_CLOCK_REG_OFS);
		dsiphy_ctrl_controller_reg.reg = DSIPHY_GETREG(DSIPHY_CTRL2_CONTROLLER_REG_OFS);
	}


	
	switch (cfg_id) {
		
	case DSIPHY_CONFIG_ID_PHASE_DELAY_ENABLE: {
			dsiphy_en_reg.bit.DSIPHY_DELAY_EN = ui_config;
			DSIPHY_SETREG(DSIPHY_ENABLE_REG_OFS, dsiphy_en_reg.reg);
		}
		break;

	case DSIPHY_CONFIG_ID_DAT0_PHASE_OFS: {
			dsiphy_delay_reg.bit.DSIPHY_DAT0_PHASE_OFS = ui_config;
			DSIPHY_SETREG(DSIPHY_DELAY_REG_OFS, dsiphy_delay_reg.reg);
		}
		break;

	case DSIPHY_CONFIG_ID_DAT1_PHASE_OFS: {
			dsiphy_delay_reg.bit.DSIPHY_DAT1_PHASE_OFS = ui_config;
			DSIPHY_SETREG(DSIPHY_DELAY_REG_OFS, dsiphy_delay_reg.reg);
		}
		break;

	case DSIPHY_CONFIG_ID_DAT2_PHASE_OFS: {
			dsiphy_delay_reg.bit.DSIPHY_DAT2_PHASE_OFS = ui_config;
			DSIPHY_SETREG(DSIPHY_DELAY_REG_OFS, dsiphy_delay_reg.reg);
		}
		break;

	case DSIPHY_CONFIG_ID_DAT3_PHASE_OFS: {
			dsiphy_delay_reg.bit.DSIPHY_DAT3_PHASE_OFS = ui_config;
			DSIPHY_SETREG(DSIPHY_DELAY_REG_OFS, dsiphy_delay_reg.reg);
		}
		break;

	case DSIPHY_CONFIG_ID_CLK_PHASE_OFS: {
			dsiphy_ctrl_clk_reg.bit.PHY_DLY_CK = ui_config;

			if (id == DSIPHY_ID_1){
				DSIPHY_SETREG(DSIPHY_CTRL1_CLOCK_REG_OFS, dsiphy_ctrl_clk_reg.reg);
			}else if (id == DSIPHY_ID_2){
				DSIPHY_SETREG(DSIPHY_CTRL2_CLOCK_REG_OFS, dsiphy_ctrl_clk_reg.reg);
			}
		}
		break;

	case DSIPHY_CONFIG_ID_PHY_HS_CLK_INV: {
			dsiphy_ctrl_clk_reg.bit.PHY_HS_CLK_INV = ui_config;

			if (id == DSIPHY_ID_1){
				DSIPHY_SETREG(DSIPHY_CTRL1_CLOCK_REG_OFS, dsiphy_ctrl_clk_reg.reg);
			}else if (id == DSIPHY_ID_2){
				DSIPHY_SETREG(DSIPHY_CTRL2_CLOCK_REG_OFS, dsiphy_ctrl_clk_reg.reg);
			}
		}
		break;

	case DSIPHY_CONFIG_ID_PHY_LP_RX_DAT0: {
			dsiphy_ctrl_controller_reg.bit.DSIPHY_LP_RX_DAT0_EN = ui_config;

			if (id == DSIPHY_ID_1){
				DSIPHY_SETREG(DSIPHY_CTRL1_CONTROLLER_REG_OFS, dsiphy_ctrl_controller_reg.reg);
			}else if (id == DSIPHY_ID_2){
				DSIPHY_SETREG(DSIPHY_CTRL2_CONTROLLER_REG_OFS, dsiphy_ctrl_controller_reg.reg);
			}
		}
		break;
		

	default:
		DBG_WRN("CfgID Err = %d\r\n", (int)cfg_id);
		return E_SYS;
	}

	//return E_SYS;
	return E_OK; 
}