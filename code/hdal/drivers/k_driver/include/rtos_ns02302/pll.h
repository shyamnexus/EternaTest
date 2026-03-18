/**
    PLL Configuration module header

    PLL Configuration module header file.

    @file       pll.h
    @ingroup    mIDrvSys_CG
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _PLL_H
#define _PLL_H

#include <kwrap/nvt_type.h>

/**
    @addtogroup mIHALSysCG
*/
//@{

/**
    Clock Enable ID

    This is for pll_enable_clock() and pll_disable_clock().
*/
typedef enum {

//==========================================================================
//[0x70]
//==========================================================================

	Reserved22,  	    ///< Reserved22
	Reserved0,          ///< Reserved0
	SIE_MCLK,           ///< SIE MCLK
	SIE_MCLK2,          ///< SIE MCLK2
	SIE_CLK,            ///< SIE clock
	SIE2_CLK,           ///< SIE2 clock
	TGE_CLK,            ///< TGE clock
	IPE_CLK,            ///< IPE clock
	Reserved1,          ///< Reserved1
	IME_CLK,            ///< IME clock
	SIE_MCLK3,	        ///< SIE_MCLK3
	ISE_CLK,		    ///< ISE clock
	SP_CLK,  		    ///< special clock
	IFE_CLK,            ///< IFE clock
	Reserved2,          ///< Reserved12
	TRKE_CLK,           ///< TRKE Clock
	IDE_CLK,            ///< IDE clock
	SIE3_CLK,           ///< SIE3 clock
	SIE4_CLK,           ///< SIE4 clock
	SIE5_CLK,           ///< SIE5 clock
	Reserved3,          ///< Reserved3
	VPE_CLK,			///< VPE clock
	GRAPH3_CLK, 		///< Graphic3 clock
	CRYPTO_CLK, 	    ///< CRYPTO clock
	VENC_CLK, 	        ///< VENC clock
	Reserved4,          ///< Reserved4
	JPEG_CLK,			///< JPEG clock
	JPG_CLK=JPEG_CLK,
	GRAPH_CLK,			///< Graphic clock
	GRAPH2_CLK,		    ///< Graphic2 clock
	DAI_CLK,			///< DAI clock
	EAC_A_ADC_CLK,		///< EAC analog AD clock
	EAC_A_DAC_CLK,      ///< EAC analog DA clock

//==========================================================================
//[0x74]
//==========================================================================

	NAND_CLK	 = 32,	///< NAND clock
	Reserved5,			///< Reserved5
	SDIO_CLK,			///< SDIO clock
	SDIO2_CLK,			///< SDIO2 clock
	I2C_CLK,            ///< I2C clock
	I2C2_CLK,		    ///< I2C2 clock
	SPI_CLK,			///< SPI clock
	SPI2_CLK,			///< SPI2 clock
	SPI3_CLK,		    ///< SPI3 clock
	SIF_CLK,			///< SIF clock
	UART_CLK,			///< UART clock
	UART2_CLK,		    ///< UART2 clock
	REMOTE_CLK,			///< REMOTE clock
	ADC_CLK,			///< ADC clock
	SDIO3_CLK,			///< SDIO3 clock
	VIE_CLK,			///< VIE clock
	HWCOPY_CLK,			///< Hwcopy clock
	Reserved6, 			///< Reserved6
	TMR_CLK, 			///< Timer clock
	MAU_CLK, 			///< MAU clock
	EAC_D_CLK, 			///< EAC digital clock
	HVYLD_CLK,          ///< heavyload clock
	UART3_CLK,          ///< UART3 clock
	UART4_CLK,          ///< UART4 clock
	UART5_CLK,          ///< UART5 clock
	UART6_CLK,          ///< UART6 clock
	I2C4_CLK,			///< I2C4 clock
	I2C5_CLK, 			///< I2C5 clock
	EFUSE_CLK,			///< efuse clock
	ETH_CLK, 			///< ethernet  clock
	SP2_CLK,            ///< special2 clock
	I2C3_CLK,           ///< I2C3 clock

//==========================================================================
//[0x78]
//==========================================================================

	Reserved7= 64, 			///< Reserved7
	CSI_CLK,				///< CSI clock
	CSI2_CLK,				///< CSI2 clock
	Reserved8,				///< Reserved8
	SIE_PXCLK,				///< SIE PX clock
	Reserved9,				///< Reserved9
	SIE3_PXCLK,				///< SIE PX clock
	Reserved10,				///< Reserved9
	PWM_CCNT_CLK,			///< PWM CCNT clock
	PWM_CCNT0_CLK=PWM_CCNT_CLK,
	PWM_CCNT1_CLK,			///< PWM CCNT1 clock
	PWM_CCNT2_CLK,			///< PWM CCNT2 clock
	SPI4_CLK,				///< SPI4 clock
	SPI5_CLK,				///< SPI5 clock
	TSE_CLK,				///< TSE clock
	CONV_CLK,				///< CONV clock
	Reserved11,           	///< Reserved11
	Reserved12,           	///< Reserved12
	Reserved13,				///< Reserved13
	LSU_CLK,				///< LSU clock
	NUE30_CLK,				///< NUE30 clock
	PPU_CLK,				///< PPU clock
	POU_CLK,         	///< POU clock
	DRTC_CLK,				///< DRTC clock
	Reserved15,				///< Reserved15
	ETHPHY_CLK,				///< ETH EXT PHY clock
	TRNG_CLK,				///< TRNG clock
	RSA_CLK,				///< RSA clock
	HASH_CLK,				///< HASH clock
	Reserved16,				///< Reserved16
	CSI_CK0D4_CLK,         	///< MIPI CSI PHY HS-CLK0/4
	CSI_CK1D4_CLK,		    ///< MIPI CSI PHY HS-CLK1/4
	CSI2_CK1D4_CLK,			///< MIPI CSI2 PHY HS-CLK1/4
//==========================================================================
//[0x7C]
//==========================================================================

	PWM0_CLK      = 96,	///< PWM0 clock
	PWM1_CLK,           ///< PWM1 clock
	PWM2_CLK,           ///< PWM2 clock
	PWM3_CLK,           ///< PWM3 clock
	PWM4_CLK,           ///< PWM4 clock
	PWM5_CLK,           ///< PWM5 clock
	PWM6_CLK,           ///< PWM6 clock
	PWM7_CLK,           ///< PWM7 clock
	PWM8_CLK,           ///< PWM8 clock
	PWM9_CLK,           ///< PWM9 clock
	PWM10_CLK,          ///< PWM10 clock
	PWM11_CLK,          ///< PWM11 clock
	HRTMR_CLK,          ///< PWM11 clock
	HRTMR2_CLK,			///< HRTimer clock
	HRTMR3_CLK,			///< HRTimer2 clock
	HRTMR4_CLK,			///< HRTimer4 clock
	SDP_CLK,			///< sdp clock
	HRTMR5_CLK,			///< HRTimer5 clock
	HRTMR6_CLK,			///< HRTimer6 clock
	HRTMR7_CLK,			///< HRTimer7 clock
	Reserved17,			///< Reserved16
	NUE2_CLK,			///< NUE2 clock
	MDBC_CLK,			///< MDBC clock
	Reserved18,			///< Reserved17
	Reserved19,			///< Reserved18
	IVE_CLK,			///< IVE clock
	HRTMR8_CLK,			///< HRTimer8 clock
	HRTMR9_CLK,			///< HRTimer9 clock
	HRTMR10_CLK,		///< HRTimer10 clock
	MI_CLK,				///< MI clock

//==========================================================================
//[0x80]
//==========================================================================

	UVCP_CLK	= 128,	///< UVCP clock
	Reserved20,			///< Reserved18
	CSI3_CLK,			///< CSI3 clock
	CSI4_CLK,			///< CSI4 clock
	ECDSA_CLK,			///< ECDSA clock
	SIE_MCLK4,			///< SIE MCLK4
	CSI3_CK2D4_CLK,		///< MIPI CSI3 PHY HS-CLK2/4
	CSI3_CK3D4_CLK,		///< MIPI CSI3 PHY HS-CLK3/4
	CSI4_CK3D4_CLK,		///< MIPI CSI4 PHY HS-CLK3/4
	HVYLD2_CLK,			///< heavy2 clock
	HVYLD3_CLK,			///< heavy3 clock
	PRE_CLK,			///< PRE clock
	TCM_CLK,			///< TCM clock
	TCM2_CLK,			///< TCM2 clock
	TCM3_CLK,			///< TCM3 clock
	TCM4_CLK,			///< TCM4 clock
	SENPHY_LPCLK_D0,	///< SENPHY LPCLK D0
	SENPHY_LPCLK_D1,	///< SENPHY LPCLK D1
	SENPHY_LPCLK_D2,	///< SENPHY LPCLK D2
	SENPHY_LPCLK_D3,	///< SENPHY LPCLK D3
	SENPHY_LPCLK_D4,	///< SENPHY LPCLK D4
	SENPHY_LPCLK_D5,	///< SENPHY LPCLK D5
	SENPHY_LPCLK_D6,	///< SENPHY LPCLK D6
	SENPHY_LPCLK_D7,	///< SENPHY LPCLK D7
	CSI_CK2D4_CLK,		///< MIPI CSI PHY HS-CLK2/4
	CSI_CK3D4_CLK,		///< MIPI CSI PHY HS-CLK3/4
	DRE_CLK,			///< DRE clock
	SIE5_PXCLK,			///< SIE5 PX clock
	SIE5_STSEN_RX_CLK,	///< SIE5 Sensor TEMP clock
	ETH_PTP_REF_CLK,	///< ETH PTP clock
	VIE_A_PXCLK,		///< VIE_A PX clock
	VIE_B_PXCLK,		///< VIE_B PX clock


//==========================================================================
//[0xE0]
//==========================================================================

	//0xE0 start from 896
	//(0x70 + X/32 * 4) = 0xE0
	//0xE0 - 0x70 = X/8
	//X=0x380(896)

	MAU_PROG_CLKEN = 896,
	HVYLD_PROG_CLKEN,
	TIMER_PROG_CLKEN,
	HRTIMER_PROG_CLKEN,
	HRTIMER2_PROG_CLKEN,
	HRTIMER3_PROG_CLKEN,
	HRTIMER4_PROG_CLKEN,
	HRTIMER5_PROG_CLKEN,
	HRTIMER6_PROG_CLKEN,
	HRTIMER7_PROG_CLKEN,
	HRTIMER8_PROG_CLKEN,
	HRTIMER9_PROG_CLKEN,
	HRTIMER10_PROG_CLKEN,
	DRTC_PROG_CLKEN = 896 + 15,
	DAI_PROG_CLKEN,
	EAC_PROG_CLKEN,
	JPEG_PROG_CLKEN,
	VENC_PROG_CLKEN,
	IDE_PROG_CLKEN,
	MI_PROG_CLKEN = 896 + 22,
	ADC_PROG_CLKEN = 896 + 24,
	ETH_PROG_CLKEN = 896 + 26,
	I2C_PROG_CLKEN,
	I2C2_PROG_CLKEN,
	I2C3_PROG_CLKEN,
	I2C4_PROG_CLKEN,
	I2C5_PROG_CLKEN,

	PWM_PROG_CLKEN,
	REMOTE_PROG_CLKEN,
	SIF_PROG_CLKEN,
	SPI_PROG_CLKEN,
	SPI2_PROG_CLKEN,
	SPI3_PROG_CLKEN,
	SPI4_PROG_CLKEN,
	SPI5_PROG_CLKEN,
	SDP_PROG_CLKEN,
	UART_PROG_CLKEN,
	UART2_PROG_CLKEN,
	UART3_PROG_CLKEN,
	UART4_PROG_CLKEN,
	UART5_PROG_CLKEN,
	UART6_PROG_CLKEN,
	MIPI_CSI_PROG_CLKEN,
	MIPI_CSI2_PROG_CLKEN,
	MIPI_CSI3_PROG_CLKEN,
	MIPI_CSI4_PROG_CLKEN,
	NAND_PROG_CLKEN,
	SDIO_PROG_CLKEN,
	SDIO2_PROG_CLKEN,
	SDIO3_PROG_CLKEN,
	HWCP_PROG_CLKEN,
	UVCP_PROG_CLKEN,
	GRAPHIC_PROG_CLKEN = 928 + 26,
	GRAPHIC2_PROG_CLKEN,
	GRAPHIC3_PROG_CLKEN,
	TSE_PROG_CLKEN,
	CRYPTO_PROG_CLKEN,
	HASH_PROG_CLKEN,

	RSA_PROG_CLKEN,
	ECDSA_PROG_CLKEN,
	TRNG_PROG_CLKEN,
	SIE_PROG_CLKEN,
	SIE2_PROG_CLKEN,
	SIE3_PROG_CLKEN,
	SIE4_PROG_CLKEN,
	SIE5_PROG_CLKEN,
	VIE_PROG_CLKEN,
	TGE_PROG_CLKEN,
	IFE_PROG_CLKEN,
	IPE_PROG_CLKEN,
	IME_PROG_CLKEN,
	ISE_PROG_CLKEN,
	VPE_PROG_CLKEN,
	CONV_PROG_CLKEN,
	NUE2_PROG_CLKEN,
	JMISP_PROG_CLKEN,
	JM_PROG_CLKEN,
	CAL_PROG_CLKEN,
	LSU_PROG_CLKEN,
	ROU_PROG_CLKEN,
	PPU_PROG_CLKEN,
	UTIL_PROG_CLKEN,
	IVE_PROG_CLKEN,
	TRKE_PROG_CLKEN,
	DRE_PROG_CLKEN,
	MDBC_PROG_CLKEN,
	HVYLD2_PROG_CLKEN,
	HVYLD3_PROG_CLKEN,
	PRE_PROG_CLKEN,
	POU_PROG_CLKEN,

	SENPHY_IF_PROG_CLKEN = 992 + 0,
	EFUSE_PROG_CLKEN,

	//STBC
	WDT_CLK			=  17301504,	///< Gating STBC WDT clock (0x21_0070-70)/4*32
	MCU_CLK			=  17301504 + 1,	///< Gating STBC MCU clock
	USB3_CLK		=  17301504 + 2,	///< Gating STBC USB3 clock
	DSI_CLK			=  17301504 + 3,	///< Gating STBC DSI clock
	CSI_TX_CLK		=  17301504 + 4,	///< Gating STBC CSI_TX clock
	RO_PHY_CLK		=  17301504 + 5,	///< Gating STBC RO_PHY clock
	RO_32K_CLK		=  17301504 + 6,	///< Gating STBC RO 32K clock
	RO_12M_CLK		=  17301504 + 7,	///< Gating STBC RO 12M clock
	AXIC_CLK		=  17301504 + 24,	///< Gating STBC AXI  clock


	WDT_PROG_CLKEN		=  17302400,		///< Gating STBC WDT clock (0x21_00E0-70)/4*32
	CC_PROG_CLKEN		=  17302400 + 1,	///< Gating STBC CC clock
	USB3_PROG_CLKEN		=  17302400 + 2,	///< Gating STBC USB3 clock
	DSI_PROG_CLKEN		=  17302400 + 3,	///< Gating STBC DSI clock
	CSI_TX_PROG_CLKEN	=  17302400 + 4,	///< Gating STBC CSI TX clock
	DSI_PHY_PROG_CLKEN	=  17302400 + 5,	///< Gating STBC DSI PHY clock
	RTC_PROG_CLKEN		=  17302400 + 6,	///< Gating STBC RTC clock
	MCU_PROG_CLKEN		=  17302400 + 7,	///< Gating STBC MCU clock


	ENUM_DUMMY4WORD(CG_EN)
} CG_EN;

/*backward compatible used, don't use*/
typedef enum {
	DMA_CLK       = 1,  ///< DMA(SDRAM) clock
	AFFINE_CLK    = 25, ///< AFFINE clock
	MIPI_LVDS_CLK = 65, ///< MIPI/LVDS clock
	MIPI_LVDS2_CLK,     ///< MIPI/LVDS2 clock
	MIPI_LVDS_PHYD4_CLK2 = 93,///< MIPI/LVDS PHY HS-clock/4
	MIPI_LVDS_PHYD4_CLK  = 94,///< MIPI/LVDS PHY HS-clock/4
	MIPI_LVDS2_PHYD4_CLK = 95,///< MIPI/LVDS2 PHY HS-clock/4
	IP_ACLK_CLK   = 127, ///< IP_ACLK clock


	H265D_CLK,
	H264D_CLK,
	SIE_MCLK5,
	CSI2_CK0D4_CLK,
	CSI3_CK1D4_CLK,
	CSI5_CLK,
	CSI2_CK2D4_CLK,
	CSI2_CK3D4_CLK,
	CSI4_CK2D4_CLK,
	CSI5_CK3D4_CLK,
	CSI_SIE_CLK,
	CSI_SIE2_CLK,
	CSI_SIE3_CLK,
	CSI_SIE4_CLK,
	CSI2_SIE3_CLK,
	CSI2_SIE4_CLK,
	CSI2_SIE5_CLK,
	CSI2_SIE6_CLK,
	CSI3_SIE3_CLK,
	CSI3_SIE4_CLK,
	CSI3_SIE5_CLK,
	CSI3_SIE6_CLK,
	CSI4_SIE3_CLK,
	CSI4_SIE4_CLK,
	CSI4_SIE5_CLK,
	CSI4_SIE6_CLK,
	CSI5_SIE5_CLK,
	CSI5_SIE6_CLK,
	CSI2_VIE_CLK,
	CSI4_VIE2_CLK,
	GPENC2_CLK,
	GPENC_CLK,

	IDE1_CLK,
	IDE2_CLK,
	VPEL_CLK,
	DCE_CLK,
	SDE_CLK,
	EAC_D_CLKEN,
	VIE2_CLK,
	AE_CLK,
	VTRC_CLK,
	SIE6_PXCLK,
	SIE4_PXCLK,
	VIE2_A_PXCLK,
	SIE6_CLK,
	VIE2_B_PXCLK,
	SIE2_PXCLK,
	DAI2_CLK,
	ETH2_CLK,
	ETHPHY2_CLK,


	I2C6_CLK,
	I2C7_CLK,
	I2C8_CLK,
	I2C9_CLK,
	I2C10_CLK,
	I2C11_CLK,

	UVCP2_CLK,
	DSP_CLK,

	CNN_M_GCLK,
	CNN2_M_GCLK,
	GPENC_M_GCLK,
	GPENC2_M_GCLK,
	JPEG2_M_GCLK,
	ETH2_M_GCLK,
	JPEG_LITE_M_GCLK,
	H264D_M_GCLK,
	H265D_M_GCLK,
	VTRC_M_GCLK,
	AE_M_GCLK,
	TIMER2_M_GCLK,
	TIMER3_M_GCLK,
	NUE_M_GCLK,
	DIS_M_GCLK,
	SDE_M_GCLK,
	DCE_M_GCLK,
	VPEL_M_GCLK,
	ISE2_M_GCLK,
	MSI_INTC_M_GCLK,
	I2C_M_GCLK,
	I2C2_M_GCLK,
	I2C3_M_GCLK,
	I2C4_M_GCLK,
	I2C5_M_GCLK,
	I2C6_M_GCLK,
	I2C7_M_GCLK,
	I2C8_M_GCLK,
	I2C9_M_GCLK,
	I2C10_M_GCLK,
	I2C11_M_GCLK,

	CNN_GCLK,               /*2*/   ///< Gating CNN APB clock
	CNN2_GCLK,              /*3*/   ///< Gating CNN2 APB clock
	DCE_GCLK,               /*5*/   ///< Gating DCE APB clock
	GPENC_GCLK,				/*6*/	///< Gating GPENC APB clock
	GPENC2_GCLK,			/*7*/	///< Gating GPENC2 APB clock
	JPG_GCLK,               /*8*/   ///< Gating JPEG APB clock
	MSI_INTC_GCLK,			/*9*/	///< Gating MSI INTC APB clock
	DIS_GCLK,               /*12*/  ///< Gating DIS APB clock
	VTRC_GCLK, 				/*14*/  ///< Gating VTRC APB clock
	HDMI_GCLK,				/*18*/	///< Gating HDMI APB clock
	PMC_GCLK,				/*19*/	///< Gating PMC APB clock
	RSA_GCLK,				/*20*/	///< Gating RSA APB clock
	NUE_GCLK,				/*27*/	///< Gating NUE APB clock
	TMR2_GCLK,              /*33*/  ///< Gating TMR2 APB clock
	TMR3_GCLK,              /*34*/  ///< Gating TMR3 APB clock
	SDE_GCLK,               /*35*/  ///< Gating SDE APB clock
	VPEL_GCLK,              /*39*/  ///< Gating VPE-lite APB clock
	I2C6_GCLK,              /*45*/  ///< Gating I2C6 APB clock
	I2C7_GCLK,              /*46*/  ///< Gating I2C7 APB clock
	I2C8_GCLK,              /*47*/  ///< Gating I2C8 APB clock
	I2C9_GCLK,              /*48*/  ///< Gating I2C9 APB clock
	I2C10_GCLK,             /*49*/  ///< Gating I2C10 APB clock
	I2C11_GCLK,             /*50*/  ///< Gating I2C11 APB clock
	RM_GCLK,				/*57*/	///< Gating Remote APB clock
	ADC_TSEN_GCLK,          /*59*/  ///< Gating ADC T-sensor APB clock
	DAI2_GCLK,              /*65*/  ///< Gating DAI2 APB clock
	SIE6_GCLK,				/*68*/	///< Gating SIE6 APB clock
	IDE2_GCLK,              /*71*/  ///< Gating IDE2 APB clock
	CSI5_GCLK,              /*76*/  ///< Gating CSI5 APB clock
	SENPHY2_GCLK, 			/*83*/  ///< Gating SENPHY2 APB clock
	UVCP2_GCLK, 			/*90*/  ///< Gating UVCP2 APB clock
	VIE2_GCLK,              /*91*/  ///< Gating VIE2 APB clock
	UART7_GCLK, 			/*93*/	///< Gating UART7 APB clock
	UART8_GCLK, 			/*94*/	///< Gating UART8 APB clock
	UART9_GCLK, 			/*95*/	///< Gating UART9 APB clock
	DSP_GCLK,               /*102*/ ///< Gating DSP APB clock
	JPG_LITE_GCLK,          /*104*/ ///< Gating JPEG Lite APB clock
	SENPHY3_GCLK, 			/*105*/ ///< Gating SENPHY2 APB clock
	JPG2_GCLK,              /*106*/   ///< Gating JPEG APB clock
	VDEC_GCLK,              /*107*/   ///< Gating VDEC APB clock




} CG_EN_BK;





/**
    APB Clock Gating Select ID

    This is for pll_set_pclk_auto_gating() / pll_clear_pclk_auto_gating() / pll_get_pclk_auto_gating().
*/
typedef enum {

	SIE_GCLK				= 0,		///< Gating SIE APB clock
	IPE_GCLK               	= 2,		///< Gating IPE APB clock
	IME_GCLK,				/*3*/		///< Gating IME APB clock
	IFE_GCLK               	= 7,		///< Gating IFE APB clock
	GRA_GCLK,				/*8*/		///< Gating Graphic APB clock
	GRA2_GCLK,				/*9*/		///< Gating Graphic APB clock
	IDE_GCLK,				/*10*/		///< Gating IDE APB clock
	NUE2_GCLK              	= 12,		///< Gating NUE2 APB clock
	MDBC_GCLK,              /*13*/		///< Gating MDBC APB clock
	VPE_GCLK,              	/*14*/		///< Gating VPE APB clock
	JPEG_GCLK,              /*15*/		///< Gating JPEG APB clock
	VENC_GCLK,              /*16*/		///< Gating VENC APB clock
	DAI_GCLK,    	        /*17*/		///< Gating DAI APB clock
	EAC_GCLK,    	        /*18*/		///< Gating EAC APB clock
	NAND_GCLK,    	        /*19*/		///< Gating NAND APB clock
	SDIO_GCLK,    	        /*20*/		///< Gating SDIO APB clock
	SDIO2_GCLK,    	        /*21*/		///< Gating SDIO2 APB clock
	I2C_GCLK,    	        /*22*/		///< Gating I2C APB clock
	I2C2_GCLK,    	        /*23*/		///< Gating I2C2 APB clock
	SPI_GCLK,    	        /*24*/		///< Gating SPI APB clock
	SPI2_GCLK,    	        /*25*/		///< Gating SPI2 APB clock
	SPI3_GCLK,    	        /*26*/		///< Gating SPI3 APB clock
	SIF_GCLK,    	        /*27*/		///< Gating SPI3 APB clock
	UART_GCLK,    	        /*28*/		///< Gating UART APB clock
	UART2_GCLK,    	        /*29*/		///< Gating UART2 APB clock
	REMOTE_GCLK,    	    /*30*/		///< Gating REMOTE APB clock
	ADC_GCLK,    	        /*31*/		///< Gating ADC APB clock

	TMR_GCLK               	= 32+1,		///< Gating TMR APB clock
	CSI_GCLK              	= 32+3,   		///< Gating CSI APB clock
	CSI2_GCLK,            	/*36*/   	///< Gating CSI2 APB clock
	CSI3_GCLK,            	/*37*/   	///< Gating CSI3 APB clock
	CSI4_GCLK              	= 32+7,   		///< Gating CSI4 APB clock
	ISE_GCLK,             	/*40*/  		///< Gating ISE APB clock
	SIE2_GCLK,             	/*41*/  		///< Gating SIE2 APB clock
	SIE3_GCLK,             	/*42*/  	///< Gating SIE3 APB clock
	PWM_GCLK,             	/*43*/  	///< Gating PWM APB clock
	SIE4_GCLK,             	/*44*/  	///< Gating SIE4 APB clock
	SIE5_GCLK,             	/*45*/  	///< Gating SIE5 APB clock
	JMISP_GCLK,            	/*46*/  	///< Gating JMISP APB clock
	SDIO3_GCLK,            	/*47*/  	///< Gating SDIO3 APB clock
	UART3_GCLK,            	/*48*/  	///< Gating UART3 APB clock
	UART4_GCLK,            	/*49*/  	///< Gating UART4 APB clock
	UART5_GCLK,            	/*50*/  	///< Gating UART5 APB clock
	UART6_GCLK,            	/*51*/  	///< Gating UART6 APB clock
	TGE_GCLK,            	/*52*/  	///< Gating TGE APB clock
	SDP_GCLK,            	/*53*/  	///< Gating SDP APB clock
	I2C4_GCLK,              /*54*/  	///< Gating I2C4 APB clock
	I2C5_GCLK,              /*55*/  	///< Gating I2C5 APB clock
	I2C3_GCLK,              /*56*/  	///< Gating I2C3 APB clock
	TSE_GCLK,            	/*57*/  	///< Gating TGE APB clock
	ECDSA_GCLK            	= 32+27,	///< Gating ECDSA APB clock
	IVE_GCLK,            	/*60*/  	///< Gating IVE APB clock
	SENPHY_GCLK,           	/*61*/  	///< Gating SENPHY APB clock
	SPI4_GCLK,            	/*62*/  	///< Gating SPI4 APB clock
	SPI5_GCLK,            	/*63*/  	///< Gating SPI5 APB clock

	GPIO_GCLK            	= 64,  		///< Gating GPIO APB clock
	INTC_GCLK,            	/*65*/  	///< Gating INTC APB clock
	POU_GCLK,               /*66*/
	MAU_GCLK            	= 64+3, 	///< Gating MAU APB clock
	JM_GCLK,           	 	/*68*/ 		///< Gating JM APB clock
	HRTMR_GCLK,          	/*69*/ 		///< Gating HRTIMER APB clock
	HRTMR2_GCLK,          	/*70*/ 		///< Gating HRTIMER2 APB clock
	HRTMR3_GCLK,          	/*71*/ 		///< Gating HRTIMER3 APB clock
	HRTMR4_GCLK,          	/*72*/ 		///< Gating HRTIMER4 APB clock
	HRTMR5_GCLK,          	/*73*/ 		///< Gating HRTIMER5 APB clock
	HRTMR6_GCLK,          	/*74*/ 		///< Gating HRTIMER6 APB clock
	HRTMR7_GCLK,          	/*75*/ 		///< Gating HRTIMER7 APB clock
	HRTMR8_GCLK,          	/*76*/ 		///< Gating HRTIMER8 APB clock
	HRTMR9_GCLK,          	/*77*/ 		///< Gating HRTIMER9 APB clock
	HRTMR10_GCLK,          	/*78*/ 		///< Gating HRTIMER10 APB clock
	DRTC_GCLK,          	/*79*/ 		///< Gating DRTC APB clock
	MI_GCLK,          		/*80*/ 		///< Gating MI APB clock
	HWCOPY_GCLK,          	/*81*/ 		///< Gating HWCOPY APB clock
	UVCP_GCLK,          	/*82*/ 		///< Gating UVCP APB clock
	PRE_GCLK,               /*83*/          ///< Gating PRE APB clock
	GRA3_GCLK            	= 64+20, 	///< Gating Graphic3 APB clock
	CRYPTO_GCLK,            /*85*/  	///< Gating Crypto APB clock
	HASH_GCLK,				/*86*/		///< Gating HASH APB clock
	VIE_GCLK,               /*87*/   	///< Gating VIE APB clock
	CONV_GCLK,              /*88*/   	///< Gating CONV APB clock
	CAL_GCLK,              	/*89*/   	///< Gating CAL APB clock
	LSU_GCLK,              	/*90*/   	///< Gating LSU APB clock
	ROU_GCLK,              	/*91*/   	///< Gating ROU APB clock
	PPU_GCLK,              	/*92*/   	///< Gating PPU APB clock
	UTIL_GCLK,              /*93*/   	///< Gating UTIL APB clock
	TRKE_GCLK,              /*94*/   	///< Gating TRKE APB clock
	DRE_GCLK,              	/*95*/   	///< Gating DRE APB clock


	//STBC PCLK
	WDT_GCLK		= 17301504,		///< Gating WDT APB clock (0x21_00D0-D0)/4*32
	CC_GCLK			= 17301504 + 1,	///< Gating CC APB clock
	DSI_GCLK		= 17301504 + 2,	///< Gating DSI APB clock
	CSI_TX_GCLK		= 17301504 + 3,	///< Gating CSI TX APB clock

	PCLKGAT_MAXNUM,

	ENUM_DUMMY4WORD(GATECLK)
} GATECLK;





/**
    Module Clock Gating Select ID

    This is for pll_set_clk_auto_gating() / pll_clear_clk_auto_gating() / pll_get_clk_auto_gating().
*/
typedef enum {
	M_GCLK_BASE             = PCLKGAT_MAXNUM,

	IPE_M_GCLK              = M_GCLK_BASE+2,       	///< Gating IPE Module clock
	IME_M_GCLK,       		/*3*/					///< Gating IME Module clock
	IFE_M_GCLK       		= M_GCLK_BASE+7,		///< Gating IFE Module clock
	GRA_M_GCLK,       		/*8*/					///< Gating Graphic Module clock
	GRA2_M_GCLK,       		/*9*/					///< Gating Graphic2 Module clock
	NUE2_M_GCLK       		= M_GCLK_BASE+12,		///< Gating NUE2 Module clock
	MDBC_M_GCLK,       		/*13*/					///< Gating MDBC Module clock
	JPEG_M_GCLK       		= M_GCLK_BASE+15,		///< Gating JPEG Module clock
	VENC_M_GCLK,       		/*16*/					///< Gating JPEG Module clock
	NAND_M_GCLK       		= M_GCLK_BASE+19,		///< Gating NAND Module clock
	SDIO_M_GCLK,       		/*20*/					///< Gating SDIO Module clock
	SDIO2_M_GCLK,       	/*21*/					///< Gating SDIO2 Module clock
	SPI4_M_GCLK,      	 	/*22*/					///< Gating SPI4 Module clock
	SPI5_M_GCLK,      	 	/*23*/					///< Gating SPI5 Module clock
	SPI_M_GCLK,      	 	/*24*/					///< Gating SPI Module clock
	SPI2_M_GCLK,      	 	/*25*/					///< Gating SPI2 Module clock
	SPI3_M_GCLK,      	 	/*26*/					///< Gating SPI3 Module clock
	SIF_M_GCLK,      	 	/*27*/					///< Gating SIF Module clock

	TIMER_M_GCLK      	 	= M_GCLK_BASE+32,		///< Gating TIMER Module clock
	ISE_M_GCLK      	 	= M_GCLK_BASE+32+8,		///< Gating ISE Module clock
	PWM_M_GCLK      	 	= M_GCLK_BASE+32+11,	///< Gating PWM Module clock
	SDIO3_M_GCLK      	 	= M_GCLK_BASE+32+15,	///< Gating SDIO3 Module clock
	TSE_M_GCLK      	 	= M_GCLK_BASE+32+25,	///< Gating TSE Module clock
	ETH_M_GCLK      	 	= M_GCLK_BASE+32+27,	///< Gating ETH Module clock
	IVE_M_GCLK,      	 	/*60*/					///< Gating IVE Module clock

	HRTMR_M_GCLK      	 	= M_GCLK_BASE+64,		///< Gating HRTIMER Module clock
	HRTMR2_M_GCLK,      	/*65*/					///< Gating HRTIMER2 Module clock
	HRTMR3_M_GCLK,      	/*66*/					///< Gating HRTIMER3 Module clock
	HRTMR4_M_GCLK,      	/*67*/					///< Gating HRTIMER4 Module clock
	HRTMR5_M_GCLK,      	/*68*/					///< Gating HRTIMER5 Module clock
	HRTMR6_M_GCLK,      	/*69*/					///< Gating HRTIMER6 Module clock
	HRTMR7_M_GCLK,      	/*70*/					///< Gating HRTIMER7 Module clock
	HRTMR8_M_GCLK,      	/*71*/					///< Gating HRTIMER8 Module clock
	HRTMR9_M_GCLK,      	/*72*/					///< Gating HRTIMER9 Module clock
	HRTMR10_M_GCLK,      	/*73*/					///< Gating HRTIMER10 Module clock
	MI_M_GCLK,      		/*74*/					///< Gating MI Module clock
	HWCOPY_M_GCLK,      		/*75*/					///< Gating HWCP Module clock
	GRA3_M_GCLK,       		/*76*/					///< Gating Graphic3 Module clock
	SIE_M_GCLK,       		/*77*/					///< Gating SIE Module clock
	SIE2_M_GCLK,       		/*78*/					///< Gating SIE2 Module clock
	SIE3_M_GCLK,       		/*79*/					///< Gating SIE3 Module clock
	SIE4_M_GCLK,       		/*80*/					///< Gating SIE4 Module clock
	SIE5_M_GCLK,       		/*81*/					///< Gating SIE5 Module clock
	VPE_M_GCLK,       		/*82*/					///< Gating VPE Module clock
	CONV_M_GCLK,       		/*83*/					///< Gating CONV Module clock
	CONV_2_M_GCLK,                  /*84*/                                  ///< Gating CONV2 Module clock
	LSU_M_GCLK       		= M_GCLK_BASE+64+21,	///< Gating LSU Module clock
	NUE30_M_GCLK,       		/*86*/			///< Gating NUE30 Module clock
	PPU_M_GCLK,       		/*87*/			///< Gating PPU Module clock
	CONV_3_M_GCLK,                  /*88*/                  ///< Gating CONV3 Module clock
	CONV_4_M_GCLK,                  /*89*/                  ///< Gating CONV4 Module clock
	TRKE_M_GCLK       		= M_GCLK_BASE+64+26,	///< Gating TRKE Module clock
	DRE_M_GCLK,       		/*91*/			///< Gating DRE Module clock
	PRE_M_GCLK,                     /*92*/                  ///< Gating PRE Moudle clock
	POU_M_GCLK,                     /*93*/                  ///< Gating POU Moudle clock

	MCLKGAT_MAXNUM,
	ENUM_DUMMY4WORD(M_GATECLK)
} M_GATECLK;


typedef enum {
	MIPI_LVDS_GCLK			= 35,	///< Gating MIPI/LVDS APB clock
	MIPI_LVDS2_GCLK,		/*36*/	///< Gating MIPI/LVDS2 APB clock

} M_GATECLK_BK;






/*
    @name Default Gating Clock Select definition

    This is for pll_config_clk_auto_gating() & pll_config_pclk_auto_gating().
*/
//@{
#define PLL_CLKSEL_DEFAULT_CLKGATE1     0x00000000
#define PLL_CLKSEL_DEFAULT_CLKGATE2     0x00000000
#define PLL_CLKSEL_DEFAULT_PCLKGATE1    0x00000000
#define PLL_CLKSEL_DEFAULT_PCLKGATE2    0x00000000
#define PLL_CLKSEL_DEFAULT_PCLKGATE3    0x00000000
//@}


/**
    PLL ID
*/
typedef enum {
	PLL_ID_0        = 0,        ///< PLL0 (for AXI0)
	PLL_ID_1        = 1,        ///< PLL1 (for internal 480 MHz)
	PLL_ID_2        = 2,        ///< PLL2 (for AXI1)
	PLL_ID_3        = 3,        ///< PLL3 (for DMA1)
	PLL_ID_4        = 4,        ///< PLL4 (for SIE SSPLL)
	PLL_ID_5        = 5,        ///< PLL5 (for SIE SENSOR)
	PLL_ID_6        = 6,        ///< PLL6 (for IDE)
	PLL_ID_7        = 7,        ///< PLL7 (for AUDIO)
	PLL_ID_8        = 8,        ///< PLL8 (for CPU)
	PLL_ID_9        = 9,        ///< PLL9 (for BACKUP)
	PLL_ID_10       = 10,       ///< PLL10 (for CNN)
	PLL_ID_11       = 11,       ///< PLL11 (for DSI)
	PLL_ID_12       = 12,       ///< PLL12 (for SENSOR2)
	PLL_ID_13       = 13,       ///< PLL13 (for IPP)
	PLL_ID_14       = 14,       ///< PLL14 (for USB3)
	PLL_ID_15       = 15,       ///< PLL15 (for VENC H264/H265)
	PLL_ID_16       = 16,       ///< PLL16 (for ETH)
	PLL_ID_17       = 17,       ///< PLL17 (for VPE)
	PLL_ID_FIXED320 = 18,       ///< Fixed 320MHz PLL

	PLL_ID_MAX,
	ENUM_DUMMY4WORD(PLL_ID)
} PLL_ID;


typedef enum {
	PLL_ID_18       = 19,       ///< nothing
	PLL_ID_19       = 19,       ///< nothing
	PLL_ID_20       = 19,		///< nothing
	PLL_ID_21       = 19,		///< nothing
	PLL_ID_22       = 19,		///< nothing
	PLL_ID_23       = 19,		///< nothing
	PLL_ID_24       = 19,		///< nothing

} PLL_BK_ID;
//#define PLL_ID_2    PLL_ID_MAX		///< Backward compatible


extern ER       pll_set_pll(PLL_ID id, UINT32 ui_setting);
extern BOOL     pll_get_pll_enable(PLL_ID id);
extern ER       pll_set_pll_enable(PLL_ID id, BOOL b_enable);
extern ER		pll_set_pll_freq(PLL_ID id, UINT32 ui_frequency);
extern UINT32   pll_get_pll_freq(PLL_ID id);
extern ER       pll_set_pll_spread_spectrum(PLL_ID id, UINT32 lower_frequency, UINT32 upper_frequency);
extern ER       pll_get_pll_spread_spectrum(PLL_ID id, UINT32 *pui_lower_freq, UINT32 *pui_upper_freq);

extern void     pll_set_pwm_clock_rate(UINT32 pwm_number, UINT32 ui_divider);
extern BOOL     pll_is_clock_enabled(CG_EN num);

extern void     pll_set_clk_auto_gating(M_GATECLK clock_select);
extern void     pll_clear_clk_auto_gating(M_GATECLK clock_select);
extern BOOL     pll_get_clk_auto_gating(M_GATECLK clock_select);

extern void     pll_set_pclk_auto_gating(GATECLK clock_select);
extern void     pll_clear_pclk_auto_gating(GATECLK clock_select);
extern BOOL     pll_get_pclk_auto_gating(GATECLK clock_select);

extern void     pll_set_trng_ro_sel(UINT32 trng_ro_select, UINT32 ui_divider);
extern void     pll_get_trng_ro_sel(UINT32 *pui_trng_ro_sel, UINT32 *pui_divider);
extern ER       pll_init(void);
extern UINT32   pll_get_apb_freq(void);

extern void     pll_dump_info(void);




#define pll_setPLL					pll_set_pll
#define pll_getPLLEn				pll_get_pll_enable
#define pll_setPLLEn				pll_set_pll_enable
#define pll_getPLLFreq				pll_get_pll_freq
#define pll_setPLLSpreadSpectrum	pll_set_pll_spread_spectrum
#define pll_getPLLSpreadSpectrum	pll_get_pll_spread_spectrum
#define pll_isClockEnabled			pll_is_clock_enabled
#define pll_setClkAutoGating		pll_set_clk_auto_gating
#define pll_clearClkAutoGating		pll_clear_clk_auto_gating
#define pll_getClkAutoGating		pll_get_clk_auto_gating
#define pll_setPclkAutoGating		pll_set_pclk_auto_gating
#define pll_clearPclkAutoGating		pll_clear_pclk_auto_gating
#define pll_getPclkAutoGating		pll_get_pclk_auto_gating


//@}

#endif



