/*
    PLL configuration module internal header

    PLL configuration module internal header file

    @file       pll_int.h
    @ingroup    mIDrvSys_CG
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _PLL_INT_REG_H
#define _PLL_INT_REG_H
#if defined(__FREERTOS)
#include <rcw_macro.h>
#include <kwrap/nvt_type.h>
#include <io_address.h>
#else
#include <linux/soc/nvt/nvt_type.h>
#include <linux/soc/nvt/rcw_macro.h>
#endif

#ifndef _FPGA_EMULATION_
#ifdef _NVT_FPGA_
#define _FPGA_EMULATION_ 1
#else
#define _FPGA_EMULATION_ 0
#endif
#endif

#ifndef _EMULATION_
#ifdef _NVT_EMULATION_
#define _EMULATION_ 1
#else
#define _EMULATION_ 0
#endif
#endif

#if defined __KERNEL__
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

/**
    Clock Enable ID

    This is for pll_enable_clock() and pll_disable_clock().
*/
typedef enum {

//==========================================================================
//[0x70]
//==========================================================================

	Reserved22,         ///< Reserved22
	Reserved0,          ///< Reserved0
	SIE_MCLK,           ///< SIE MCLK
	SIE_MCLK2,          ///< SIE MCLK2
	SIE_CLK,            ///< SIE clock
	SIE2_CLK,           ///< SIE2 clock
	TGE_CLK,            ///< TGE clock
	IPE_CLK,            ///< IPE clock
	Reserved1,          ///< Reserved1
	IME_CLK,            ///< IME clock
	SIE_MCLK3,          ///< SIE_MCLK3
	ISE_CLK,            ///< ISE clock
	SP_CLK,             ///< special clock
	IFE_CLK,            ///< IFE clock
	Reserved2,          ///< Reserved12
	TRKE_CLK,           ///< TRKE Clock
	IDE_CLK,            ///< IDE clock
	SIE3_CLK,           ///< SIE3 clock
	SIE4_CLK,           ///< SIE4 clock
	SIE5_CLK,           ///< SIE5 clock
	Reserved3,          ///< Reserved3
	VPE_CLK,            ///< VPE clock
	GRAPH3_CLK,         ///< Graphic3 clock
	CRYPTO_CLK,         ///< CRYPTO clock
	VENC_CLK,           ///< VENC clock
	Reserved4,          ///< Reserved4
	JPEG_CLK,           ///< JPEG clock
	JPG_CLK = JPEG_CLK,
	GRAPH_CLK,          ///< Graphic clock
	GRAPH2_CLK,         ///< Graphic2 clock
	DAI_CLK,            ///< DAI clock
	EAC_A_ADC_CLK,      ///< EAC analog AD clock
	EAC_A_DAC_CLK,      ///< EAC analog DA clock

//==========================================================================
//[0x74]
//==========================================================================

	NAND_CLK     = 32,  ///< NAND clock
	Reserved5,          ///< Reserved5
	SDIO_CLK,           ///< SDIO clock
	SDIO2_CLK,          ///< SDIO2 clock
	I2C_CLK,            ///< I2C clock
	I2C2_CLK,           ///< I2C2 clock
	SPI_CLK,            ///< SPI clock
	SPI2_CLK,           ///< SPI2 clock
	SPI3_CLK,           ///< SPI3 clock
	SIF_CLK,            ///< SIF clock
	UART_CLK,           ///< UART clock
	UART2_CLK,          ///< UART2 clock
	REMOTE_CLK,         ///< REMOTE clock
	ADC_CLK,            ///< ADC clock
	SDIO3_CLK,          ///< SDIO3 clock
	VIE_CLK,            ///< VIE clock
	HWCOPY_CLK,         ///< Hwcopy clock
	Reserved6,          ///< Reserved6
	TMR_CLK,            ///< Timer clock
	MAU_CLK,            ///< MAU clock
	EAC_D_CLK,          ///< EAC digital clock
	HVYLD_CLK,          ///< heavyload clock
	UART3_CLK,          ///< UART3 clock
	UART4_CLK,          ///< UART4 clock
	UART5_CLK,          ///< UART5 clock
	UART6_CLK,          ///< UART6 clock
	I2C4_CLK,           ///< I2C4 clock
	I2C5_CLK,           ///< I2C5 clock
	EFUSE_CLK,          ///< efuse clock
	ETH_CLK,            ///< ethernet  clock
	SP2_CLK,            ///< special2 clock
	I2C3_CLK,           ///< I2C3 clock

//==========================================================================
//[0x78]
//==========================================================================

	Reserved7 = 64,         ///< Reserved7
	CSI_CLK,                ///< CSI clock
	CSI2_CLK,               ///< CSI2 clock
	Reserved8,              ///< Reserved8
	SIE_PXCLK,              ///< SIE PX clock
	Reserved9,              ///< Reserved9
	SIE3_PXCLK,             ///< SIE PX clock
	Reserved10,             ///< Reserved9
	PWM_CCNT_CLK,           ///< PWM CCNT clock
	PWM_CCNT0_CLK = PWM_CCNT_CLK,
	PWM_CCNT1_CLK,          ///< PWM CCNT1 clock
	PWM_CCNT2_CLK,          ///< PWM CCNT2 clock
	SPI4_CLK,               ///< SPI4 clock
	SPI5_CLK,               ///< SPI5 clock
	TSE_CLK,                ///< TSE clock
	CONV_CLK,               ///< CONV clock
	Reserved11,             ///< Reserved11
	Reserved12,             ///< Reserved12
	Reserved13,             ///< Reserved13
	LSU_CLK,                ///< LSU clock
	NUE30_CLK,              ///< NUE30 clock
	PPU_CLK,                ///< PPU clock
	POU_CLK,                ///< POU clock
	DRTC_CLK,               ///< DRTC clock
	Reserved15,             ///< Reserved15
	ETHPHY_CLK,             ///< ETH EXT PHY clock
	TRNG_CLK,               ///< TRNG clock
	RSA_CLK,                ///< RSA clock
	HASH_CLK,               ///< HASH clock
	Reserved16,             ///< Reserved16
	CSI_CK0D4_CLK,          ///< MIPI CSI PHY HS-CLK0/4
	CSI_CK1D4_CLK,          ///< MIPI CSI PHY HS-CLK1/4
	CSI2_CK1D4_CLK,         ///< MIPI CSI2 PHY HS-CLK1/4
//==========================================================================
//[0x7C]
//==========================================================================

	PWM0_CLK      = 96, ///< PWM0 clock
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
	HRTMR2_CLK,         ///< HRTimer clock
	HRTMR3_CLK,         ///< HRTimer2 clock
	HRTMR4_CLK,         ///< HRTimer4 clock
	SDP_CLK,            ///< sdp clock
	HRTMR5_CLK,         ///< HRTimer5 clock
	HRTMR6_CLK,         ///< HRTimer6 clock
	HRTMR7_CLK,         ///< HRTimer7 clock
	Reserved17,         ///< Reserved16
	NUE2_CLK,           ///< NUE2 clock
	MDBC_CLK,           ///< MDBC clock
	Reserved18,         ///< Reserved17
	Reserved19,         ///< Reserved18
	IVE_CLK,            ///< IVE clock
	HRTMR8_CLK,         ///< HRTimer8 clock
	HRTMR9_CLK,         ///< HRTimer9 clock
	HRTMR10_CLK,        ///< HRTimer10 clock
	MI_CLK,             ///< MI clock

//==========================================================================
//[0x80]
//==========================================================================

	UVCP_CLK    = 128,  ///< UVCP clock
	Reserved20,         ///< Reserved18
	CSI3_CLK,           ///< CSI3 clock
	CSI4_CLK,           ///< CSI4 clock
	ECDSA_CLK,          ///< ECDSA clock
	SIE_MCLK4,          ///< SIE MCLK4
	CSI3_CK2D4_CLK,     ///< MIPI CSI3 PHY HS-CLK2/4
	CSI3_CK3D4_CLK,     ///< MIPI CSI3 PHY HS-CLK3/4
	CSI4_CK3D4_CLK,     ///< MIPI CSI4 PHY HS-CLK3/4
	HVYLD2_CLK,         ///< heavy2 clock
	HVYLD3_CLK,         ///< heavy3 clock
	PRE_CLK,            ///< PRE clock
	TCM_CLK,            ///< TCM clock
	TCM2_CLK,           ///< TCM2 clock
	TCM3_CLK,           ///< TCM3 clock
	TCM4_CLK,           ///< TCM4 clock
	SENPHY_LPCLK_D0,    ///< SENPHY LPCLK D0
	SENPHY_LPCLK_D1,    ///< SENPHY LPCLK D1
	SENPHY_LPCLK_D2,    ///< SENPHY LPCLK D2
	SENPHY_LPCLK_D3,    ///< SENPHY LPCLK D3
	SENPHY_LPCLK_D4,    ///< SENPHY LPCLK D4
	SENPHY_LPCLK_D5,    ///< SENPHY LPCLK D5
	SENPHY_LPCLK_D6,    ///< SENPHY LPCLK D6
	SENPHY_LPCLK_D7,    ///< SENPHY LPCLK D7
	CSI_CK2D4_CLK,      ///< MIPI CSI PHY HS-CLK2/4
	CSI_CK3D4_CLK,      ///< MIPI CSI PHY HS-CLK3/4
	DRE_CLK,            ///< DRE clock
	SIE5_PXCLK,         ///< SIE5 PX clock
	SIE5_STSEN_RX_CLK,  ///< SIE5 Sensor TEMP clock
	ETH_PTP_REF_CLK,    ///< ETH PTP clock
	VIE_A_PXCLK,        ///< VIE_A PX clock
	VIE_B_PXCLK,        ///< VIE_B PX clock


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

	SENPHY_IF_PROG_CLKEN,
	EFUSE_PROG_CLKEN,

	//STBC
	WDT_CLK         =  17301504,    ///< Gating STBC WDT clock (0x21_0070-70)/4*32
	MCU_CLK         =  17301504 + 1,    ///< Gating STBC MCU clock
	USB3_CLK        =  17301504 + 2,    ///< Gating STBC USB3 clock
	DSI_CLK         =  17301504 + 3,    ///< Gating STBC DSI clock
	CSI_TX_CLK      =  17301504 + 4,    ///< Gating STBC CSI_TX clock
	RO_PHY_CLK      =  17301504 + 5,    ///< Gating STBC RO_PHY clock
	RO_32K_CLK      =  17301504 + 6,    ///< Gating STBC RO 32K clock
	RO_12M_CLK      =  17301504 + 7,    ///< Gating STBC RO 12M clock
	AXIC_CLK        =  17301504 + 24,   ///< Gating STBC AXI  clock


	WDT_PROG_CLKEN      =  17302400,        ///< Gating STBC WDT clock (0x21_00E0-70)/4*32
	CC_PROG_CLKEN       =  17302400 + 1,    ///< Gating STBC CC clock
	USB3_PROG_CLKEN     =  17302400 + 2,    ///< Gating STBC USB3 clock
	DSI_PROG_CLKEN      =  17302400 + 3,    ///< Gating STBC DSI clock
	CSI_TX_PROG_CLKEN   =  17302400 + 4,    ///< Gating STBC CSI TX clock
	DSI_PHY_PROG_CLKEN  =  17302400 + 5,    ///< Gating STBC DSI PHY clock
	RTC_PROG_CLKEN      =  17302400 + 6,    ///< Gating STBC RTC clock
	MCU_PROG_CLKEN      =  17302400 + 7,    ///< Gating STBC MCU clock


	ENUM_DUMMY4WORD(CG_EN)
} CG_EN;

/**
    APB Clock Gating Select ID

    This is for pll_set_pclk_auto_gating() / pll_clear_pclk_auto_gating() / pll_get_pclk_auto_gating().
*/
typedef enum {

	SIE_GCLK                = 0,        ///< Gating SIE APB clock
	IPE_GCLK                = 2,        ///< Gating IPE APB clock
	IME_GCLK,               /*3*/       ///< Gating IME APB clock
	IFE_GCLK                = 7,        ///< Gating IFE APB clock
	GRA_GCLK,               /*8*/       ///< Gating Graphic APB clock
	GRA2_GCLK,              /*9*/       ///< Gating Graphic APB clock
	IDE_GCLK,               /*10*/      ///< Gating IDE APB clock
	NUE2_GCLK               = 12,       ///< Gating NUE2 APB clock
	MDBC_GCLK,              /*13*/      ///< Gating MDBC APB clock
	VPE_GCLK,               /*14*/      ///< Gating VPE APB clock
	JPEG_GCLK,              /*15*/      ///< Gating JPEG APB clock
	VENC_GCLK,              /*16*/      ///< Gating VENC APB clock
	DAI_GCLK,               /*17*/      ///< Gating DAI APB clock
	EAC_GCLK,               /*18*/      ///< Gating EAC APB clock
	NAND_GCLK,              /*19*/      ///< Gating NAND APB clock
	SDIO_GCLK,              /*20*/      ///< Gating SDIO APB clock
	SDIO2_GCLK,             /*21*/      ///< Gating SDIO2 APB clock
	I2C_GCLK,               /*22*/      ///< Gating I2C APB clock
	I2C2_GCLK,              /*23*/      ///< Gating I2C2 APB clock
	SPI_GCLK,               /*24*/      ///< Gating SPI APB clock
	SPI2_GCLK,              /*25*/      ///< Gating SPI2 APB clock
	SPI3_GCLK,              /*26*/      ///< Gating SPI3 APB clock
	SIF_GCLK,               /*27*/      ///< Gating SPI3 APB clock
	UART_GCLK,              /*28*/      ///< Gating UART APB clock
	UART2_GCLK,             /*29*/      ///< Gating UART2 APB clock
	REMOTE_GCLK,            /*30*/      ///< Gating REMOTE APB clock
	ADC_GCLK,               /*31*/      ///< Gating ADC APB clock

	TMR_GCLK                = 32 + 1,   ///< Gating TMR APB clock
	CSI_GCLK                = 32 + 3,       ///< Gating CSI APB clock
	CSI2_GCLK,              /*36*/      ///< Gating CSI2 APB clock
	CSI3_GCLK,              /*37*/      ///< Gating CSI3 APB clock
	CSI4_GCLK               = 32 + 7,       ///< Gating CSI4 APB clock
	ISE_GCLK,               /*40*/          ///< Gating ISE APB clock
	SIE2_GCLK,              /*41*/          ///< Gating SIE2 APB clock
	SIE3_GCLK,              /*42*/      ///< Gating SIE3 APB clock
	PWM_GCLK,               /*43*/      ///< Gating PWM APB clock
	SIE4_GCLK,              /*44*/      ///< Gating SIE4 APB clock
	SIE5_GCLK,              /*45*/      ///< Gating SIE5 APB clock
	JMISP_GCLK,             /*46*/      ///< Gating JMISP APB clock
	SDIO3_GCLK,             /*47*/      ///< Gating SDIO3 APB clock
	UART3_GCLK,             /*48*/      ///< Gating UART3 APB clock
	UART4_GCLK,             /*49*/      ///< Gating UART4 APB clock
	UART5_GCLK,             /*50*/      ///< Gating UART5 APB clock
	UART6_GCLK,             /*51*/      ///< Gating UART6 APB clock
	TGE_GCLK,               /*52*/      ///< Gating TGE APB clock
	SDP_GCLK,               /*53*/      ///< Gating SDP APB clock
	I2C4_GCLK,              /*54*/      ///< Gating I2C4 APB clock
	I2C5_GCLK,              /*55*/      ///< Gating I2C5 APB clock
	I2C3_GCLK,              /*56*/      ///< Gating I2C3 APB clock
	TSE_GCLK,               /*57*/      ///< Gating TGE APB clock
	ECDSA_GCLK              = 32 + 27,  ///< Gating ECDSA APB clock
	IVE_GCLK,               /*60*/      ///< Gating IVE APB clock
	SENPHY_GCLK,            /*61*/      ///< Gating SENPHY APB clock
	SPI4_GCLK,              /*62*/      ///< Gating SPI4 APB clock
	SPI5_GCLK,              /*63*/      ///< Gating SPI5 APB clock

	GPIO_GCLK               = 64,       ///< Gating GPIO APB clock
	INTC_GCLK,              /*65*/      ///< Gating INTC APB clock
    POU_GCLK,               /*66*/
	MAU_GCLK                = 64 + 3,   ///< Gating MAU APB clock
	JM_GCLK,                /*68*/      ///< Gating JM APB clock
	HRTMR_GCLK,             /*69*/      ///< Gating HRTIMER APB clock
	HRTMR2_GCLK,            /*70*/      ///< Gating HRTIMER2 APB clock
	HRTMR3_GCLK,            /*71*/      ///< Gating HRTIMER3 APB clock
	HRTMR4_GCLK,            /*72*/      ///< Gating HRTIMER4 APB clock
	HRTMR5_GCLK,            /*73*/      ///< Gating HRTIMER5 APB clock
	HRTMR6_GCLK,            /*74*/      ///< Gating HRTIMER6 APB clock
	HRTMR7_GCLK,            /*75*/      ///< Gating HRTIMER7 APB clock
	HRTMR8_GCLK,            /*76*/      ///< Gating HRTIMER8 APB clock
	HRTMR9_GCLK,            /*77*/      ///< Gating HRTIMER9 APB clock
	HRTMR10_GCLK,           /*78*/      ///< Gating HRTIMER10 APB clock
	DRTC_GCLK,              /*79*/      ///< Gating DRTC APB clock
	MI_GCLK,                /*80*/      ///< Gating MI APB clock
	HWCOPY_GCLK,            /*81*/      ///< Gating HWCOPY APB clock
	UVCP_GCLK,              /*82*/      ///< Gating UVCP APB clock
	PRE_GCLK,               /*83*/      ///< Gating PRE APB clock
	GRA3_GCLK               = 64 + 20,  ///< Gating Graphic3 APB clock
	CRYPTO_GCLK,            /*85*/      ///< Gating Crypto APB clock
	HASH_GCLK,              /*86*/      ///< Gating HASH APB clock
	VIE_GCLK,               /*87*/      ///< Gating VIE APB clock
	CONV_GCLK,              /*88*/      ///< Gating CONV APB clock
	CAL_GCLK,               /*89*/      ///< Gating CAL APB clock
	LSU_GCLK,               /*90*/      ///< Gating LSU APB clock
	ROU_GCLK,               /*91*/      ///< Gating ROU APB clock
	PPU_GCLK,               /*92*/      ///< Gating PPU APB clock
	UTIL_GCLK,              /*93*/      ///< Gating UTIL APB clock
	TRKE_GCLK,              /*94*/      ///< Gating TRKE APB clock
	DRE_GCLK,               /*95*/      ///< Gating DRE APB clock


	//STBC PCLK
	WDT_GCLK        = 17301504,     ///< Gating WDT APB clock (0x21_00D0-D0)/4*32
	CC_GCLK         = 17301504 + 1, ///< Gating CC APB clock
	DSI_GCLK        = 17301504 + 2, ///< Gating DSI APB clock
	CSI_TX_GCLK     = 17301504 + 3, ///< Gating CSI TX APB clock

	PCLKGAT_MAXNUM,

	ENUM_DUMMY4WORD(GATECLK)
} GATECLK;





/**
    Module Clock Gating Select ID

    This is for pll_set_clk_auto_gating() / pll_clear_clk_auto_gating() / pll_get_clk_auto_gating().
*/
typedef enum {
	M_GCLK_BASE             = PCLKGAT_MAXNUM,

	IPE_M_GCLK              = M_GCLK_BASE + 2,      ///< Gating IPE Module clock
	IME_M_GCLK,             /*3*/                   ///< Gating IME Module clock
	IFE_M_GCLK              = M_GCLK_BASE + 7,      ///< Gating IFE Module clock
	GRA_M_GCLK,             /*8*/                   ///< Gating Graphic Module clock
	GRA2_M_GCLK,            /*9*/                   ///< Gating Graphic2 Module clock
	NUE2_M_GCLK             = M_GCLK_BASE + 12,     ///< Gating NUE2 Module clock
	MDBC_M_GCLK,            /*13*/                  ///< Gating MDBC Module clock
	JPEG_M_GCLK             = M_GCLK_BASE + 15,     ///< Gating JPEG Module clock
	VENC_M_GCLK,            /*16*/                  ///< Gating JPEG Module clock
	NAND_M_GCLK             = M_GCLK_BASE + 19,     ///< Gating NAND Module clock
	SDIO_M_GCLK,            /*20*/                  ///< Gating SDIO Module clock
	SDIO2_M_GCLK,           /*21*/                  ///< Gating SDIO2 Module clock
	SPI4_M_GCLK,            /*22*/                  ///< Gating SPI4 Module clock
	SPI5_M_GCLK,            /*23*/                  ///< Gating SPI5 Module clock
	SPI_M_GCLK,             /*24*/                  ///< Gating SPI Module clock
	SPI2_M_GCLK,            /*25*/                  ///< Gating SPI2 Module clock
	SPI3_M_GCLK,            /*26*/                  ///< Gating SPI3 Module clock
	SIF_M_GCLK,             /*27*/                  ///< Gating SIF Module clock

	TIMER_M_GCLK            = M_GCLK_BASE + 32,     ///< Gating TIMER Module clock
	ISE_M_GCLK              = M_GCLK_BASE + 32 + 8, ///< Gating ISE Module clock
	PWM_M_GCLK              = M_GCLK_BASE + 32 + 11, ///< Gating PWM Module clock
	SDIO3_M_GCLK            = M_GCLK_BASE + 32 + 15, ///< Gating SDIO3 Module clock
	TSE_M_GCLK              = M_GCLK_BASE + 32 + 25, ///< Gating TSE Module clock
	ETH_M_GCLK              = M_GCLK_BASE + 32 + 27, ///< Gating ETH Module clock
	IVE_M_GCLK,             /*60*/                  ///< Gating IVE Module clock

	HRTMR_M_GCLK            = M_GCLK_BASE + 64,     ///< Gating HRTIMER Module clock
	HRTMR2_M_GCLK,          /*65*/                  ///< Gating HRTIMER2 Module clock
	HRTMR3_M_GCLK,          /*66*/                  ///< Gating HRTIMER3 Module clock
	HRTMR4_M_GCLK,          /*67*/                  ///< Gating HRTIMER4 Module clock
	HRTMR5_M_GCLK,          /*68*/                  ///< Gating HRTIMER5 Module clock
	HRTMR6_M_GCLK,          /*69*/                  ///< Gating HRTIMER6 Module clock
	HRTMR7_M_GCLK,          /*70*/                  ///< Gating HRTIMER7 Module clock
	HRTMR8_M_GCLK,          /*71*/                  ///< Gating HRTIMER8 Module clock
	HRTMR9_M_GCLK,          /*72*/                  ///< Gating HRTIMER9 Module clock
	HRTMR10_M_GCLK,         /*73*/                  ///< Gating HRTIMER10 Module clock
	MI_M_GCLK,              /*74*/                  ///< Gating MI Module clock
	HWCOPY_M_GCLK,              /*75*/                  ///< Gating HWCP Module clock
	GRA3_M_GCLK,            /*76*/                  ///< Gating Graphic3 Module clock
	SIE_M_GCLK,             /*77*/                  ///< Gating SIE Module clock
	SIE2_M_GCLK,            /*78*/                  ///< Gating SIE2 Module clock
	SIE3_M_GCLK,            /*79*/                  ///< Gating SIE3 Module clock
	SIE4_M_GCLK,            /*80*/                  ///< Gating SIE4 Module clock
	SIE5_M_GCLK,            /*81*/                  ///< Gating SIE5 Module clock
	VPE_M_GCLK,             /*82*/                  ///< Gating VPE Module clock
	CONV_M_GCLK,            /*83*/                  ///< Gating CONV Module clock
	CONV_2_M_GCLK,           /*84*/                 ///< Gating CONV2 Module clock
	LSU_M_GCLK              = M_GCLK_BASE + 64 + 21, ///< Gating LSU Module clock
	NUE30_M_GCLK,           /*86*/                  ///< Gating NUE30 Module clock
	PPU_M_GCLK,             /*87*/                  ///< Gating PPU Module clock
    CONV_3_M_GCLK,          /*88*/                  ///< Gating CONV3 Module clock
    CONV_4_M_GCLK,           /*89*/                  ///< Gating CONV4 Module clock
	TRKE_M_GCLK             = M_GCLK_BASE + 64 + 26, ///< Gating TRKE Module clock
	DRE_M_GCLK,             /*91*/                  ///< Gating DRE Module clock
	PRE_M_GCLK,             /*92*/                  //< Gating PRE Module clock
	POU_M_GCLK,             /*93*/                  ///< Gating POU Moudle clock

	MCLKGAT_MAXNUM,
	ENUM_DUMMY4WORD(M_GATECLK)
} M_GATECLK;

#endif
//
// Clock Rate bit definition
//
#define PLL_CLKSEL_CPU_MASK             (0x03 << PLL_CLKSEL_CPU)
#define PLL_CLKSEL_APB_MASK             (0x03 << PLL_CLKSEL_APB)
//#define PLL_CLKSEL_DMA_ARBT_MASK      (0x01 << PLL_CLKSEL_DMA_ARBT)
#define PLL_CLKSEL_DDRPHY_MASK          (0x01 << PLL_CLKSEL_DDRPHY)



/*r1*/
#define PLL_CLKSEL_IVE_MASK             (0x03 << (PLL_CLKSEL_IVE -      PLL_CLKSEL_R1_OFFSET))
#define PLL_CLKSEL_IPE_MASK             (0x03 << (PLL_CLKSEL_IPE -      PLL_CLKSEL_R1_OFFSET))
#define PLL_CLKSEL_IME_MASK             (0x03 << (PLL_CLKSEL_IME -      PLL_CLKSEL_R1_OFFSET))
#define PLL_CLKSEL_MDBC_MASK            (0x03 << (PLL_CLKSEL_MDBC -     PLL_CLKSEL_R1_OFFSET))
#define PLL_CLKSEL_VPE_MASK             (0x03 << (PLL_CLKSEL_VPE -      PLL_CLKSEL_R1_OFFSET))
#define PLL_CLKSEL_ISE_MASK             (0x03 << (PLL_CLKSEL_ISE -      PLL_CLKSEL_R1_OFFSET))
#define PLL_CLKSEL_TGE_MSH_MASK         (0x01 << (PLL_CLKSEL_TGE_MSH -  PLL_CLKSEL_R1_OFFSET))
#define PLL_CLKSEL_TGE_FLASH_MASK       (0x01 << (PLL_CLKSEL_TGE_FLASH -PLL_CLKSEL_R1_OFFSET))

/*r2*/
#define PLL_CLKSEL_IFE_MASK             (0x03 << (PLL_CLKSEL_IFE -          PLL_CLKSEL_R2_OFFSET))
#define PLL_CLKSEL_IFE_SRAM_MASK        (0x01 << (PLL_CLKSEL_IFE_SRAM -     PLL_CLKSEL_R2_OFFSET))
#define PLL_CLKSEL_PRE_SRAM_MASK        (0x01 << (PLL_CLKSEL_PRE_SRAM -     PLL_CLKSEL_R2_OFFSET))
#define PLL_CLKSEL_SIE_MCLK4SRC_MASK    (0x3 << (PLL_CLKSEL_SIE_MCLK4SRC - PLL_CLKSEL_R2_OFFSET))
#define PLL_CLKSEL_SIE_MCLKSRC_MASK     (0x3 << (PLL_CLKSEL_SIE_MCLKSRC -  PLL_CLKSEL_R2_OFFSET))
#define PLL_CLKSEL_SIE_MCLK2SRC_MASK    (0x3 <<  (PLL_CLKSEL_SIE_MCLK2SRC - PLL_CLKSEL_R2_OFFSET))
#define PLL_CLKSEL_SIE_MCLK3SRC_MASK    (0x3 << (PLL_CLKSEL_SIE_MCLK3SRC - PLL_CLKSEL_R2_OFFSET))
#define PLL_CLKSEL_SIE5_SRAM_MASK       (0x01 << (PLL_CLKSEL_SIE5_SRAM -    PLL_CLKSEL_R2_OFFSET))
#define PLL_CLKSEL_SIE_MCLKINV_MASK     (0x01 << (PLL_CLKSEL_SIE_MCLKINV -  PLL_CLKSEL_R2_OFFSET))
#define PLL_CLKSEL_SIE_MCLK2INV_MASK    (0x01 << (PLL_CLKSEL_SIE_MCLK2INV - PLL_CLKSEL_R2_OFFSET))
#define PLL_CLKSEL_SIE_MCLK3INV_MASK    (0x01 << (PLL_CLKSEL_SIE_MCLK3INV - PLL_CLKSEL_R2_OFFSET))
#define PLL_CLKSEL_SIE_MCLK4INV_MASK    (0x01 << (PLL_CLKSEL_SIE_MCLK4INV - PLL_CLKSEL_R2_OFFSET))
#define PLL_CLKSEL_NUE2_MASK            (0x03 << (PLL_CLKSEL_NUE2 -         PLL_CLKSEL_R2_OFFSET))

/*r3*/
#define PLL_CLKSEL_JPEG_MASK            (0x03 << (PLL_CLKSEL_JPEG -        PLL_CLKSEL_R3_OFFSET))
#define PLL_CLKSEL_DRE_MASK             (0x03 << (PLL_CLKSEL_DRE -         PLL_CLKSEL_R3_OFFSET))
#define PLL_CLKSEL_VENC_MASK            (0x03 << (PLL_CLKSEL_VENC -        PLL_CLKSEL_R3_OFFSET))
#define PLL_CLKSEL_ECDSA_MASK           (0x03 << (PLL_CLKSEL_ECDSA -       PLL_CLKSEL_R3_OFFSET))
#define PLL_CLKSEL_GRAPHIC_MASK         (0x03 << (PLL_CLKSEL_GRAPHIC -     PLL_CLKSEL_R3_OFFSET))
#define PLL_CLKSEL_GRAPHIC2_MASK        (0x03 << (PLL_CLKSEL_GRAPHIC2 -    PLL_CLKSEL_R3_OFFSET))
#define PLL_CLKSEL_GRAPHIC3_MASK        (0x03 << (PLL_CLKSEL_GRAPHIC3 -    PLL_CLKSEL_R3_OFFSET))
#define PLL_CLKSEL_SIE3_CLKSRC_MASK     (0x0F << (PLL_CLKSEL_SIE3_CLKSRC - PLL_CLKSEL_R3_OFFSET))
#define PLL_CLKSEL_HWCOPY_MASK          (0x03 << (PLL_CLKSEL_HWCP -        PLL_CLKSEL_R3_OFFSET))
#define PLL_CLKSEL_CRYPTO_MASK          (0x01 << (PLL_CLKSEL_CRYPTO -      PLL_CLKSEL_R3_OFFSET))
#define PLL_CLKSEL_RSA_MASK             (0x01 << (PLL_CLKSEL_RSA -         PLL_CLKSEL_R3_OFFSET))
#define PLL_CLKSEL_SIE_CLKSRC_MASK      (0x0F << (PLL_CLKSEL_SIE_CLKSRC -  PLL_CLKSEL_R3_OFFSET))
#define PLL_CLKSEL_SIE2_CLKSRC_MASK     (0x0F << (PLL_CLKSEL_SIE2_CLKSRC - PLL_CLKSEL_R3_OFFSET))
#define PLL_CLKSEL_MI_MASK              (0x01 << (PLL_CLKSEL_MI -          PLL_CLKSEL_R3_OFFSET))

/*r4*/
#define PLL_CLKSEL_VIE_CLKSRC_MASK              (0x0F << (PLL_CLKSEL_VIE_CLKSRC -             PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_ETH_PTP_REF_MASK             (0x1 <<  (PLL_CLKSEL_ETH_PTP_REF -            PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_SDIO_MASK                    (0x03 << (PLL_CLKSEL_SDIO -                   PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_SDIO2_MASK                   (0x03 << (PLL_CLKSEL_SDIO2 -                  PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_CSI_MASK                     (0x03 << (PLL_CLKSEL_CSI -                    PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_CSI2_MASK                    (0x03 << (PLL_CLKSEL_CSI2 -                   PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_IDE_CLKSRC_MASK              (0x0F << (PLL_CLKSEL_IDE_CLKSRC -             PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_I2S_MCLKSEL_MASK             (0x01 << (PLL_CLKSEL_I2S_MCLK -               PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_DAI_MASK                     (0x01 << (PLL_CLKSEL_DAI -                    PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_SENPHY_CSI2_CK1_PHASE_MASK   (0x01 << (PLL_CLKSEL_SENPHY_CSI2_CK1_PHASE-   PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_SENPHY_CSI3_CK2_PHASE_MASK   (0x01 << (PLL_CLKSEL_SENPHY_CSI3_CK2_PHASE-   PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_SENPHY_CSI3_CK3_PHASE_MASK   (0x01 << (PLL_CLKSEL_SENPHY_CSI3_CK3_PHASE-   PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_SENPHY_CSI4_CK3_PHASE_MASK   (0x01 << (PLL_CLKSEL_SENPHY_CSI4_CK3_PHASE-   PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_CSI3_MASK                    (0x03 << (PLL_CLKSEL_CSI3 -                   PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_CSI4_MASK                    (0x03 << (PLL_CLKSEL_CSI4 -                   PLL_CLKSEL_R4_OFFSET))

/*r5*/
#define PLL_CLKSEL_SDIO3_MASK                   (0x03 << (PLL_CLKSEL_SDIO3 -         PLL_CLKSEL_R5_OFFSET))
#define PLL_CLKSEL_UVCP_MASK                    (0x03 << (PLL_CLKSEL_UVCP -          PLL_CLKSEL_R5_OFFSET))
#define PLL_CLKSEL_TSE_MASK                     (0x03 << (PLL_CLKSEL_TSE -           PLL_CLKSEL_R5_OFFSET))
#define PLL_CLKSEL_SP_MASK                      (0x03 << (PLL_CLKSEL_SP -            PLL_CLKSEL_R5_OFFSET))
#define PLL_CLKSEL_SP2_MASK                     (0x03 << (PLL_CLKSEL_SP2 -           PLL_CLKSEL_R5_OFFSET))
#define PLL_CLKSEL_HASH_MASK                    (0x01 << (PLL_CLKSEL_HASH -          PLL_CLKSEL_R5_OFFSET))
#define PLL_CLKSEL_TRNG_MASK                    (0x01 << (PLL_CLKSEL_TRNG -          PLL_CLKSEL_R5_OFFSET))
#define PLL_CLKSEL_DRTC_MASK                    (0x03 << (PLL_CLKSEL_DRTC -          PLL_CLKSEL_R5_OFFSET))
#define PLL_CLKSEL_REMOTE_MASK                  (0x03 << (PLL_CLKSEL_REMOTE -        PLL_CLKSEL_R5_OFFSET))
#define PLL_CLKSEL_ADC_PD_MASK                  (0x01 << (PLL_CLKSEL_ADC_PD -        PLL_CLKSEL_R5_OFFSET))
#define PLL_CLKSEL_ETHPHY_CLKSRC_MASK           (0x03 << (PLL_CLKSEL_ETHPHY_CLKSRC - PLL_CLKSEL_R5_OFFSET))
#define PLL_CLKSEL_NAND_MASK                    (0x03 << (PLL_CLKSEL_NAND -          PLL_CLKSEL_R5_OFFSET))
#define PLL_CLKSEL_NAND_BCH_MASK                (0x01 << (PLL_CLKSEL_NAND_BCH -      PLL_CLKSEL_R5_OFFSET))
#define PLL_CLKSEL_UART_MASK                    (0x01 << (PLL_CLKSEL_UART -          PLL_CLKSEL_R5_OFFSET))

/*r6*/
#define PLL_CLKSEL_SENPHY_CSI_CK0_PHASE_MASK   (0x01 << (PLL_CLKSEL_SENPHY_CSI_CK0_PHASE-   PLL_CLKSEL_R6_OFFSET))
#define PLL_CLKSEL_SENPHY_CSI_CK1_PHASE_MASK   (0x01 << (PLL_CLKSEL_SENPHY_CSI_CK1_PHASE-   PLL_CLKSEL_R6_OFFSET))
#define PLL_CLKSEL_SENPHY_CSI_CK2_PHASE_MASK   (0x01 << (PLL_CLKSEL_SENPHY_CSI_CK2_PHASE-   PLL_CLKSEL_R6_OFFSET))
#define PLL_CLKSEL_SENPHY_CSI_CK3_PHASE_MASK   (0x01 << (PLL_CLKSEL_SENPHY_CSI_CK3_PHASE-   PLL_CLKSEL_R6_OFFSET))
#define PLL_CLKSEL_CSI3_PXCLK_MASK             (0x01 << (PLL_CLKSEL_CSI3_PXCLK -            PLL_CLKSEL_R6_OFFSET))
#define PLL_CLKSEL_SIE2_INT_CLKSRC_MASK        (0x01 << (PLL_CLKSEL_SIE2_INT_CLKSRC -       PLL_CLKSEL_R6_OFFSET))
#define PLL_CLKSEL_SIE4_INT_CLKSRC_MASK        (0x01 << (PLL_CLKSEL_SIE4_INT_CLKSRC -       PLL_CLKSEL_R6_OFFSET))
#define PLL_CLKSEL_CONV_MASK                   (0x03 << (PLL_CLKSEL_CONV -                  PLL_CLKSEL_R6_OFFSET))
#define PLL_CLKSEL_LSU_MASK                    (0x03 << (PLL_CLKSEL_LSU -                   PLL_CLKSEL_R6_OFFSET))
#define PLL_CLKSEL_NUE30_MASK                  (0x03 << (PLL_CLKSEL_NUE30 -                 PLL_CLKSEL_R6_OFFSET))
#define PLL_CLKSEL_PPU_MASK                    (0x03 << (PLL_CLKSEL_PPU -                   PLL_CLKSEL_R6_OFFSET))
#define PLL_CLKSEL_POU_MASK                    (0x03 << (PLL_CLKSEL_POU -                   PLL_CLKSEL_R6_OFFSET))
#define PLL_CLKSEL_TRKE_MASK                   (0x03 << (PLL_CLKSEL_TRKE -                  PLL_CLKSEL_R6_OFFSET))

/*r7*/
#define PLL_CLKSEL_SIE4_CLKSRC_MASK             (0x0F << (PLL_CLKSEL_SIE4_CLKSRC -    PLL_CLKSEL_R7_OFFSET))
#define PLL_CLKSEL_SIE5_CLKSRC_MASK             (0x0F << (PLL_CLKSEL_SIE5_CLKSRC -    PLL_CLKSEL_R7_OFFSET))
#define PLL_CLKSEL_STSEN_RX_CLKSRC_MASK         (0x01 << (PLL_CLKSEL_STSEN_RX_CLKSRC- PLL_CLKSEL_R7_OFFSET))
#define PLL_CLKSEL_VIE_A_PXCLKSRC_MASK          (0x01 << (PLL_CLKSEL_VIE_A_PXCLKSRC - PLL_CLKSEL_R7_OFFSET))
#define PLL_CLKSEL_VIE_B_PXCLKSRC_MASK          (0x01 << (PLL_CLKSEL_VIE_B_PXCLKSRC - PLL_CLKSEL_R7_OFFSET))

#define PLL_CLKSEL_SIE4_CLKDIV_MASK             (0xFF << (PLL_CLKSEL_SIE4_CLKDIV -    PLL_CLKSEL_R7_OFFSET))
#define PLL_CLKSEL_SIE5_CLKDIV_MASK             (0xFF << (PLL_CLKSEL_SIE5_CLKDIV -    PLL_CLKSEL_R7_OFFSET))

/*r8*/
#define PLL_CLKSEL_SIE_MCLKDIV_MASK             (0xFF << (PLL_CLKSEL_SIE_MCLKDIV -  PLL_CLKSEL_R8_OFFSET))
#define PLL_CLKSEL_SIE_MCLK2DIV_MASK            (0xFF << (PLL_CLKSEL_SIE_MCLK2DIV - PLL_CLKSEL_R8_OFFSET))
#define PLL_CLKSEL_SIE_CLKDIV_MASK              (0xFF << (PLL_CLKSEL_SIE_CLKDIV -   PLL_CLKSEL_R8_OFFSET))
#define PLL_CLKSEL_SIE2_CLKDIV_MASK             (0xFF << (PLL_CLKSEL_SIE2_CLKDIV -  PLL_CLKSEL_R8_OFFSET))

/*r9*/
#define PLL_CLKSEL_IDE_CLKDIV_MASK              (0xFF << (PLL_CLKSEL_IDE_CLKDIV -       PLL_CLKSEL_R9_OFFSET))
#define PLL_CLKSEL_IDE_OUTIF_CLKDIV_MASK        (0xFF << (PLL_CLKSEL_IDE_OUTIF_CLKDIV - PLL_CLKSEL_R9_OFFSET))
#define PLL_CLKSEL_SIE_MCLK3DIV_MASK            (0xFF << (PLL_CLKSEL_SIE_MCLK3DIV -     PLL_CLKSEL_R9_OFFSET))
#define PLL_CLKSEL_TRNG_CLKDIV_MASK             (0xFF << (PLL_CLKSEL_TRNG_CLKDIV -      PLL_CLKSEL_R9_OFFSET))

/*r10*/
#define PLL_CLKSEL_SP_CLKDIV_MASK        (0xFF << (PLL_CLKSEL_SP_CLKDIV -      PLL_CLKSEL_R10_OFFSET))
#define PLL_CLKSEL_SIE3_CLKDIV_MASK      (0xFF << (PLL_CLKSEL_SIE3_CLKDIV -    PLL_CLKSEL_R10_OFFSET))
#define PLL_CLKSEL_DAI_CLKDIV_MASK       (0xFF << (PLL_CLKSEL_DAI_CLKDIV -     PLL_CLKSEL_R10_OFFSET))
#define PLL_CLKSEL_DAI_OSR_CLKDIV_MASK   (0xFF << (PLL_CLKSEL_DAI_OSR_CLKDIV - PLL_CLKSEL_R10_OFFSET))

/*r11*/
#define PLL_CLKSEL_SDIO_CLKDIV_MASK      (0x7FF<< (PLL_CLKSEL_SDIO_CLKDIV -    PLL_CLKSEL_R11_OFFSET))
#define PLL_CLKSEL_SDIO2_CLKDIV_MASK     (0x7FF<< (PLL_CLKSEL_SDIO2_CLKDIV -   PLL_CLKSEL_R11_OFFSET))

/*r12*/
#define PLL_CLKSEL_SDIO3_CLKDIV_MASK    (0x7FF<< (PLL_CLKSEL_SDIO3_CLKDIV -  PLL_CLKSEL_R12_OFFSET))
#define PLL_CLKSEL_NAND_CLKDIV_MASK     (0x3F << (PLL_CLKSEL_NAND_CLKDIV -   PLL_CLKSEL_R12_OFFSET))
#define PLL_CLKSEL_ETH_PTP_CLKDIV_MASK  (0x3 <<  (PLL_CLKSEL_ETH_PTP_CLKDIV -PLL_CLKSEL_R12_OFFSET))
#define PLL_CLKSEL_SP2_CLKDIV_MASK      (0xFF << (PLL_CLKSEL_SP2_CLKDIV -    PLL_CLKSEL_R12_OFFSET))

/*r13*/
#define PLL_CLKSEL_SPI_CLKDIV_MASK      (0x7FF << (PLL_CLKSEL_SPI_CLKDIV -  PLL_CLKSEL_R13_OFFSET))
#define PLL_CLKSEL_SPI2_CLKDIV_MASK     (0x7FF << (PLL_CLKSEL_SPI2_CLKDIV - PLL_CLKSEL_R13_OFFSET))

/*r14*/
#define PLL_CLKSEL_SPI3_CLKDIV_MASK     (0x7FF << (PLL_CLKSEL_SPI3_CLKDIV - PLL_CLKSEL_R14_OFFSET))
#define PLL_CLKSEL_SPI4_CLKDIV_MASK     (0x7FF << (PLL_CLKSEL_SPI4_CLKDIV - PLL_CLKSEL_R14_OFFSET))

/*r15*/
#define PLL_CLKSEL_UART2_CLKDIV_MASK    (0xFF << (PLL_CLKSEL_UART2_CLKDIV - PLL_CLKSEL_R15_OFFSET))
#define PLL_CLKSEL_UART3_CLKDIV_MASK    (0xFF << (PLL_CLKSEL_UART3_CLKDIV - PLL_CLKSEL_R15_OFFSET))
#define PLL_CLKSEL_UART4_CLKDIV_MASK    (0xFF << (PLL_CLKSEL_UART4_CLKDIV - PLL_CLKSEL_R15_OFFSET))
#define PLL_CLKSEL_UART5_CLKDIV_MASK    (0xFF << (PLL_CLKSEL_UART5_CLKDIV - PLL_CLKSEL_R15_OFFSET))

/*r16*/
#define PLL_CLKSEL_PWM0_3_CLKDIV_MASK   (0x3FFF << (PLL_CLKSEL_PWM0_3_CLKDIV - PLL_CLKSEL_R16_OFFSET))
#define PLL_CLKSEL_PWM4_7_CLKDIV_MASK   (0x3FFF << (PLL_CLKSEL_PWM4_7_CLKDIV - PLL_CLKSEL_R16_OFFSET))

/*r17*/
#define PLL_CLKSEL_PWM8_CLKDIV_MASK     (0x3FFF << (PLL_CLKSEL_PWM8_CLKDIV - PLL_CLKSEL_R17_OFFSET))
#define PLL_CLKSEL_PWM9_CLKDIV_MASK     (0x3FFF << (PLL_CLKSEL_PWM9_CLKDIV - PLL_CLKSEL_R17_OFFSET))

/*r18*/
#define PLL_CLKSEL_PWM10_CLKDIV_MASK     (0x3FFF << (PLL_CLKSEL_PWM10_CLKDIV - PLL_CLKSEL_R18_OFFSET))
#define PLL_CLKSEL_PWM11_CLKDIV_MASK     (0x3FFF << (PLL_CLKSEL_PWM11_CLKDIV - PLL_CLKSEL_R18_OFFSET))

/*r19*/
#define PLL_CLKSEL_UART_CLKDV_MASK        (0xFF << (PLL_CLKSEL_UART_CLKDIV - PLL_CLKSEL_R19_OFFSET))

/*r20*/
#define PLL_CLKSEL_TRNG_RO_CLKDIV_MASK   (0xFF << (PLL_CLKSEL_TRNG_RO_CLKDIV - PLL_CLKSEL_R20_OFFSET))
#define PLL_CLKSEL_UART6_CLKDIV_MASK     (0xFF << (PLL_CLKSEL_UART6_CLKDIV -   PLL_CLKSEL_R20_OFFSET))
#define PLL_CLKSEL_DAI_MCLKDIV_MASK      (0xFF << (PLL_CLKSEL_DAI_MCLKDIV -    PLL_CLKSEL_R20_OFFSET))

/*r21*/
#define PLL_CLKSEL_SPI5_CLKDIV_MASK     (0x7FF <<(PLL_CLKSEL_SPI5_CLKDIV -   PLL_CLKSEL_R21_OFFSET))
#define PLL_CLKSEL_MI_CLKDIV_MASK       (0xFF<<  (PLL_CLKSEL_MI_CLKDIV -     PLL_CLKSEL_R21_OFFSET))
#define PLL_CLKSEL_VIE_CLKDIV_MASK      (0xFF << (PLL_CLKSEL_VIE_CLKDIV -    PLL_CLKSEL_R21_OFFSET))


/*r22*/
#define PLL_CLKSEL_SIE_MCLK4DIV_MASK    (0xFF << (PLL_CLKSEL_SIE_MCLK4DIV - PLL_CLKSEL_R22_OFFSET))



/*r23*/
#define PLL_CLKSEL_STBC_APB_MASK            (0x03 << (PLL_CLKSEL_STBC_APB -             PLL_CLKSEL_R23_OFFSET))
#define PLL_CLKSEL_STBC_RTC_32K_MASK        (0x01 << (PLL_CLKSEL_STBC_RTC_32K -         PLL_CLKSEL_R23_OFFSET))
#define PLL_CLKSEL_STBC_MCU_MASK            (0x01 << (PLL_CLKSEL_STBC_MCU -             PLL_CLKSEL_R23_OFFSET))
#define PLL_CLKSEL_STBC_USB30PHY_MASK       (0x01 << (PLL_CLKSEL_STBC_USB30PHY -        PLL_CLKSEL_R23_OFFSET))

/*r24*/
#define PLL_CLKSEL_STBC_DSI_LP_MASK         (0x03 << (PLL_CLKSEL_STBC_DSI_LP -          PLL_CLKSEL_R24_OFFSET))
#define PLL_CLKSEL_STBC_CSI_TX_LP_MASK      (0x03 << (PLL_CLKSEL_STBC_CSI_TX_LP -       PLL_CLKSEL_R24_OFFSET))
#define PLL_CLKSEL_STBC_USB3_SUSP_MASK      (0x01 << (PLL_CLKSEL_STBC_USB3_SUSP -       PLL_CLKSEL_R24_OFFSET))
#define PLL_CLKSEL_STBC_USB3_PDN_MASK       (0x01 << (PLL_CLKSEL_STBC_USB3_PDN -        PLL_CLKSEL_R24_OFFSET))


/*r25*/
#define PLL_CLKSEL_STBC_RO_DELAY_MASK           (0xF <<(PLL_CLKSEL_STBC_RO_DELAY -              PLL_CLKSEL_R25_OFFSET))
#define PLL_CLKSEL_STBC_RO32K_CLKDIV_MASK       (0x3FF<<  (PLL_CLKSEL_STBC_RO32K_CLKDIV -       PLL_CLKSEL_R25_OFFSET))
#define PLL_CLKSEL_STBC_RO32K_DIV_TESTEN_MASK   (0x1<<  (PLL_CLKSEL_STBC_RO32K_DIV_TESTEN - PLL_CLKSEL_R25_OFFSET))

/////////////////////////////






#if 0
#define PLL_CLKSEL_SIE_IO_PXCLKSRC_MASK 0
#define PLL_CLKSEL_SIE2_IO_PXCLKSRC_MASK 0
#define PLL_CLKSEL_SIE_PXCLKSRC_MASK    0
#define PLL_CLKSEL_SIE2_PXCLKSRC_MASK   0
#define PLL_CLKSEL_SIE4_IO_PXCLKSRC_MASK 0
#define PLL_CLKSEL_TGE2_MASK            0
#define PLL_CLKSEL_CNN_SRAM_MASK        (0x03 << (PLL_CLKSEL_CNN_SRAM - PLL_CLKSEL_R2_OFFSET))
#define PLL_CLKSEL_AFFINE_MASK          0
#define PLL_CLKSEL_MIPI_LVDS_MASK       (0x01 << (PLL_CLKSEL_MIPI_LVDS - PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_LVDS_CLKPHASE_MASK   (0x01 << (PLL_CLKSEL_LVDS_CLKPHASE - PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_LVDS2_CLKPHASE_MASK  0
#define PLL_CLKSEL_MIPI_LVDS2_MASK      (0x01 << (PLL_CLKSEL_MIPI_LVDS2 - PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_ETH_MASK             0
#define PLL_CLKSEL_ETH_REFCLK_INV_MASK  0
#define PLL_CLKSEL_CSILPCLK_D0_MASK     0
#define PLL_CLKSEL_CSILPCLK_D1_MASK     0
#define PLL_CLKSEL_CSILPCLK_D2_MASK     0
#define PLL_CLKSEL_CSILPCLK_D3_MASK     0
#endif




#if defined(__FREERTOS)
//
//  PLL Register access definition
//
#define     PLL_SETREG(ofs, value)       OUTW((IOADDR_CG_REG_BASE+(ofs)), (value))
#define     PLL_GETREG(ofs)             INW(IOADDR_CG_REG_BASE+(ofs))

#define     PLL_STBC_SETREG(ofs, value)       OUTW((IOADDR_STBC_CG_REG_BASE+(ofs)), (value))
#define     PLL_STBC_GETREG(ofs)             INW(IOADDR_STBC_CG_REG_BASE+(ofs))
#endif

#define PLL_FPGA_480SRC                 (24000000)
#define PLL_FPGA_320SRC                 (24000000)
#define PLL_FPGA_PLL1SRC                (24000000)
#define PLL_FPGA_PLL2SRC                (12000000)
#define PLL_FPGA_PLL2RC                 (12000000)
#define PLL_FPGA_PLL3SRC                (24000000)
#define PLL_FPGA_PLL7SRC                (12288000)
#define PLL_FPGA_PLL12SRC               (25000000)

typedef struct {
	UINT32          source_enabled;
	UINT32          source_module;
	UINT32          check_item;
	CHAR           *module_name;
} PLLCLKCHECK, *PPLLCLKCHECK;

typedef struct {
	UINT32          clk_src;
	CHAR           *clk_name;
} PLLCLKINFO, *PPLLCLKINFO;


typedef struct {
	UINT32          clk_num;
	CHAR           *clk_name;
} MCLKINFO, *PMCLKINFO;

#if defined __KERNEL__
void pll_dump_info(void);
#endif

#endif

