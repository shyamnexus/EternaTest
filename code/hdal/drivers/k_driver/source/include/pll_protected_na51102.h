/*
    Protected header for PLL module

    PLL Configuration module protected header file.

    @file       pll_protected.h
    @ingroup    mIDrvSys_CG
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _PLL_PROTECTED_H
#define _PLL_PROTECTED_H

#include <kwrap/nvt_type.h>
#include "plat/pll.h"

#if defined(_NVT_FPGA_)
#ifndef _FPGA_PLL_OSC_
#define _FPGA_PLL_OSC_  24000000
#endif
#endif


/*
    @addtogroup mIDrvSys_CG
*/
//@{

#define PLL_CLKSEL_R1_OFFSET        32
#define PLL_CLKSEL_R2_OFFSET        64
#define PLL_CLKSEL_R3_OFFSET        96
#define PLL_CLKSEL_R4_OFFSET        128
#define PLL_CLKSEL_R5_OFFSET        160
#define PLL_CLKSEL_R6_OFFSET        192
#define PLL_CLKSEL_R7_OFFSET        224
#define PLL_CLKSEL_R8_OFFSET        256
#define PLL_CLKSEL_R9_OFFSET        288
#define PLL_CLKSEL_R10_OFFSET       320
#define PLL_CLKSEL_R11_OFFSET       352
#define PLL_CLKSEL_R12_OFFSET       384
#define PLL_CLKSEL_R13_OFFSET       416
#define PLL_CLKSEL_R14_OFFSET       448
#define PLL_CLKSEL_R15_OFFSET       480
#define PLL_CLKSEL_R16_OFFSET       512
#define PLL_CLKSEL_R17_OFFSET       544
#define PLL_CLKSEL_R18_OFFSET       576
#define PLL_CLKSEL_R19_OFFSET       608
#define PLL_CLKSEL_R20_OFFSET       640
#define PLL_CLKSEL_R21_OFFSET       672
#define PLL_CLKSEL_R22_OFFSET       704
#define PLL_CLKSEL_R23_OFFSET       736
#define PLL_CLKSEL_R24_OFFSET       3968 //(0x200 - 0x10)/4*32
#define PLL_CLKSEL_R25_OFFSET       4000
#define PLL_CLKSEL_R26_OFFSET       4032
#define PLL_CLKSEL_R27_OFFSET       6016// (0x300 - 0x10)/4*32
#define PLL_CLKSEL_R28_OFFSET       6048
#define PLL_CLKSEL_R29_OFFSET       6080
#define PLL_CLKSEL_R30_OFFSET       6112








/*
    Clock select ID

    Clock select ID for pll_set_clock_rate() & pll_get_clock_rate().
*/
typedef enum {
	//System Clock Rate bit definition
	PLL_CLKSEL_CPU =            0,                          //< Clock Select Module ID: CPU
	PLL_CLKSEL_IFE_SRAM =       4,                          //< Clock Select Module ID: IFE SRAM source
	PLL_CLKSEL_CNN2_SRAM =      5,                          //< Clock Select Module ID: CNN2 SRAM source
	PLL_CLKSEL_APB =            8,                          //< Clock Select Module ID: APB
	PLL_CLKSEL_DMA_ARBT =       20,                         //< Clock select Module ID: DMA Arbiter
	PLL_CLKSEL_PD =             24,                         //< Clock select Module ID: PD
	PLL_CLKSEL_DDRPHY =         31,                         //< Clock Select Module ID: DDR-PHY

	//IPP Clock Rate bit definition
	PLL_CLKSEL_IVE =            PLL_CLKSEL_R1_OFFSET + 0,   //< Clock Select Module ID: IVE
	PLL_CLKSEL_TRKE =           PLL_CLKSEL_R1_OFFSET + 2,   //< Clock Select Module ID: TRKE
	PLL_CLKSEL_MDBC =           PLL_CLKSEL_R1_OFFSET + 4,   //< Clock Select Module ID: MDBC
	PLL_CLKSEL_CNN =            PLL_CLKSEL_R1_OFFSET + 6,   //< Clock Select Module ID: CNN
	PLL_CLKSEL_CNN2 =           PLL_CLKSEL_R1_OFFSET + 7,   //< Clock Select Module ID: CNN2
	PLL_CLKSEL_ISE =            PLL_CLKSEL_R1_OFFSET + 8,   //< Clock Select Module ID: ISE
	PLL_CLKSEL_DIS =            PLL_CLKSEL_R1_OFFSET + 10,  //< Clock Select Module ID: DIS
	PLL_CLKSEL_NUE =            PLL_CLKSEL_R1_OFFSET + 12,  //< Clock Select Module ID: NUE
	PLL_CLKSEL_NUE2 =           PLL_CLKSEL_R1_OFFSET + 14,  //< Clock Select Module ID: NUE2
	PLL_CLKSEL_IME =            PLL_CLKSEL_R1_OFFSET + 16,  //< Clock Select Module ID: IME
	PLL_CLKSEL_IPE =            PLL_CLKSEL_R1_OFFSET + 18,  //< Clock Select Module ID: IPE
	PLL_CLKSEL_DCE =            PLL_CLKSEL_R1_OFFSET + 20,  //< Clock Select Module ID: DCE
	PLL_CLKSEL_IFE =            PLL_CLKSEL_R1_OFFSET + 22,  //< Clock Select Module ID: IFE
	PLL_CLKSEL_VPE =            PLL_CLKSEL_R1_OFFSET + 24,  //< Clock Select Module ID: VPE
	PLL_CLKSEL_VPE_LITE =       PLL_CLKSEL_R1_OFFSET + 26,  //< Clock Select Module ID: VPE_LITE
	PLL_CLKSEL_ISE2 =           PLL_CLKSEL_R1_OFFSET + 28,  //< Clock Select Module ID: ISE2
	PLL_CLKSEL_SDE =            PLL_CLKSEL_R1_OFFSET + 30,  //< Clock Select Module ID: SDE

	//IPP Clock Rate 1 bit definition
	PLL_CLKSEL_SIE_MCLKINV =    PLL_CLKSEL_R2_OFFSET + 0,   //< Clock Select Module ID: SIE MCLK output Invert
	PLL_CLKSEL_SIE_MCLK2INV =   PLL_CLKSEL_R2_OFFSET + 1,   //< Clock Select Module ID: SIE MCLK2 output Invert
	PLL_CLKSEL_SIE_MCLK3INV =   PLL_CLKSEL_R2_OFFSET + 2,   //< Clock Select Module ID: SIE MCLK3 output Invert
	PLL_CLKSEL_SIE_MCLK4INV =   PLL_CLKSEL_R2_OFFSET + 3,   //< Clock Select Module ID: SIE MCLK4 output Invert
	PLL_CLKSEL_SIE_MCLK5INV =   PLL_CLKSEL_R2_OFFSET + 4,   //< Clock Select Module ID: SIE MCLK5 output Invert
	PLL_CLKSEL_TGE =            PLL_CLKSEL_R2_OFFSET + 8,   //< Clock Select Module ID: TGE

	//CODEC Clock Rate bit definition
	PLL_CLKSEL_JPEG =           PLL_CLKSEL_R3_OFFSET + 0,   //< Clock Select Module ID: JPEG
	PLL_CLKSEL_UVCP =           PLL_CLKSEL_R3_OFFSET + 2,   //< Clock Select Module ID: UCVP
	PLL_CLKSEL_VENC =           PLL_CLKSEL_R3_OFFSET + 4,   //< Clock Select Module ID: VENC
	PLL_CLKSEL_DRE =            PLL_CLKSEL_R3_OFFSET + 6,   //< Clock Select Module ID: DRE
	PLL_CLKSEL_JPEG2 =          PLL_CLKSEL_R3_OFFSET + 8,   //< Clock Select Module ID: JPEG2
	PLL_CLKSEL_H264D_A =        PLL_CLKSEL_R3_OFFSET + 10,  //< Clock Select Module ID: H264D_A
	PLL_CLKSEL_H265D_A =        PLL_CLKSEL_R3_OFFSET + 12,  //< Clock Select Module ID: H265D_A
	PLL_CLKSEL_H264D_M =        PLL_CLKSEL_R3_OFFSET + 14,  //< Clock Select Module ID: H264D_M
	PLL_CLKSEL_H265D_M =        PLL_CLKSEL_R3_OFFSET + 16,  //< Clock Select Module ID: H265D_M
	PLL_CLKSEL_JPEG_LITE =      PLL_CLKSEL_R3_OFFSET + 18,  //< Clock Select Module ID: JPEG_Lite
	PLL_CLKSEL_VTRC_AE =        PLL_CLKSEL_R3_OFFSET + 20,  //< Clock Select Module ID: VTRC_AE

	//Peripheral Clock Rate bit definition
	PLL_CLKSEL_SDIO =           PLL_CLKSEL_R4_OFFSET + 0,   //< Clock Select Module ID: SDIO
	PLL_CLKSEL_SDIO2 =          PLL_CLKSEL_R4_OFFSET + 2,   //< Clock Select Module ID: SDIO2
	PLL_CLKSEL_SDIO3 =          PLL_CLKSEL_R4_OFFSET + 4,   //< Clock Select Module ID: SDIO3
	PLL_CLKSEL_ETHPHY_CLKSRC =  PLL_CLKSEL_R4_OFFSET + 6,   //< Clock Select Module ID: ETH PHY Reference Clock Source
	PLL_CLKSEL_SP   =           PLL_CLKSEL_R4_OFFSET + 8,   //< Clock Select Module ID: Special
	PLL_CLKSEL_SP2  =           PLL_CLKSEL_R4_OFFSET + 10,  //< Clock Select Module ID: Special2
	PLL_CLKSEL_GPENC =          PLL_CLKSEL_R4_OFFSET + 12,  //< Clock Select Module ID: GPENC
	PLL_CLKSEL_GPENC2 =         PLL_CLKSEL_R4_OFFSET + 14,  //< Clock Select Module ID: GPENC2
	PLL_CLKSEL_UVCP2 =          PLL_CLKSEL_R4_OFFSET + 16,  //< Clock Select Module ID: UCVP2
	PLL_CLKSEL_HWCOPY =         PLL_CLKSEL_R4_OFFSET + 18,  //< Clock Select Module ID: HWCOPY
	PLL_CLKSEL_PMC =            PLL_CLKSEL_R4_OFFSET + 20,  //< Clock Select Module ID: PMC
	PLL_CLKSEL_DSI_LPSRC =      PLL_CLKSEL_R4_OFFSET + 22,  //< Clock Select Module ID: DSI LP Source
	PLL_CLKSEL_CSI_TX_LPSRC =   PLL_CLKSEL_R4_OFFSET + 24,  //< Clock Select Module ID: CSI TX LP Source
	PLL_CLKSEL_MI =             PLL_CLKSEL_R4_OFFSET + 26,  //< Clock Select Module ID: MI Source
	PLL_CLKSEL_ETH2PHY_CLKSRC = PLL_CLKSEL_R4_OFFSET + 28,  //< Clock Select Module ID: ETH2 PHY Reference Clock Source

	//Peripheral Clock Rate bit definition 1
	PLL_CLKSEL_HASH =           PLL_CLKSEL_R5_OFFSET + 8,   //< Clock Select Module ID: HASH
	PLL_CLKSEL_RSA =            PLL_CLKSEL_R5_OFFSET + 9,   //< Clock Select Module ID: RSA
	PLL_CLKSEL_CRYPTO =         PLL_CLKSEL_R5_OFFSET + 10,  //< Clock Select Module ID: CRYPTO
	PLL_CLKSEL_ADC_PD =         PLL_CLKSEL_R5_OFFSET + 11,  //< Clock Select Module ID: ADC @ PowerDown 1/2/3
	PLL_CLKSEL_TRNG =           PLL_CLKSEL_R5_OFFSET + 12,  //< Clock Select Module ID: TRNG
	PLL_CLKSEL_TRNGRO_CLKSRC =  PLL_CLKSEL_R5_OFFSET + 13,  //< Clock Select Module ID: TRNG RO CLK source
	PLL_CLKSEL_DRTC =           PLL_CLKSEL_R5_OFFSET + 14,  //< Clock Select Module ID: DRTC
	PLL_CLKSEL_REMOTE =         PLL_CLKSEL_R5_OFFSET + 16,  //< Clock Select Module ID: Remote
	PLL_CLKSEL_GRAPHIC =        PLL_CLKSEL_R5_OFFSET + 18,  //< Clock Select Module ID: GRAPHIC
	PLL_CLKSEL_IDE_CLKSRC =     PLL_CLKSEL_R5_OFFSET + 20,  //< Clock Select Module ID: IDE clock source
	PLL_CLKSEL_IDE2_CLKSRC =    PLL_CLKSEL_R5_OFFSET + 24,  //< Clock Select Module ID: IDE2 clock source
	PLL_CLKSEL_GRAPHIC2 =       PLL_CLKSEL_R5_OFFSET + 28,  //< Clock Select Module ID: GRAPHIC2
	PLL_CLKSEL_GRAPHIC3 =       PLL_CLKSEL_R5_OFFSET + 30,  //< Clock Select Module ID: GRAPHIC3

	//Peripheral Clock Rate bit definition 2
	PLL_CLKSEL_CSI2_PXCLK =     PLL_CLKSEL_R6_OFFSET + 8,   //< Clock Select Module ID: CSI2 pixel clock source select
	PLL_CLKSEL_CSI4_PXCLK =     PLL_CLKSEL_R6_OFFSET + 9,   //< Clock Select Module ID: CSI4 pixel clock source select
	PLL_CLKSEL_CSI =            PLL_CLKSEL_R6_OFFSET + 16,  //< Clock Select Module ID: CSI
	PLL_CLKSEL_CSI2 =           PLL_CLKSEL_R6_OFFSET + 18,  //< Clock Select Module ID: CSI2
	PLL_CLKSEL_CSI3 =           PLL_CLKSEL_R6_OFFSET + 20,  //< Clock Select Module ID: CSI3
	PLL_CLKSEL_CSI4 =           PLL_CLKSEL_R6_OFFSET + 22,  //< Clock Select Module ID: CSI4
	PLL_CLKSEL_CSI5 =           PLL_CLKSEL_R6_OFFSET + 24,  //< Clock Select Module ID: CSI5

	//Peripheral Clock Rate bit definition 3
	PLL_CLKSEL_CSIPHY_CSI_CK0_PHASE =    PLL_CLKSEL_R7_OFFSET + 0,  //< CSIPHY CK0 to CSI CK0 clock phase
	PLL_CLKSEL_CSIPHY2_CSI2_CK0_PHASE =  PLL_CLKSEL_R7_OFFSET + 1,  //< CSIPHY2 CK0 to CSI2 CK0 clock phase
	PLL_CLKSEL_CSIPHY2_CSI2_CK1_PHASE =  PLL_CLKSEL_R7_OFFSET + 2,  //< CSIPHY2 CK0 to CSI2 CK1 clock phase
	PLL_CLKSEL_CSIPHY2_CSI3_CK1_PHASE =  PLL_CLKSEL_R7_OFFSET + 3,  //< CSIPHY2 CK0 to CSI3 CK1 clock phase
	PLL_CLKSEL_CSIPHY3_CSI4_CK0_PHASE =  PLL_CLKSEL_R7_OFFSET + 4,  //< CSIPHY3 CK0 to CSI4 CK0 clock phase
	PLL_CLKSEL_CSIPHY3_CSI4_CK1_PHASE =  PLL_CLKSEL_R7_OFFSET + 5,  //< CSIPHY3 CK0 to CSI4 CK1 clock phase
	PLL_CLKSEL_CSIPHY3_CSI5_CK1_PHASE =  PLL_CLKSEL_R7_OFFSET + 6,  //< CSIPHY3 CK0 to CSI5 CK1 clock phase
	PLL_CLKSEL_DAI =                     PLL_CLKSEL_R7_OFFSET + 16, //< Clock Select Module ID: DAI
	PLL_CLKSEL_I2S_MCLK =                PLL_CLKSEL_R7_OFFSET + 17, //< Clock Select Module ID: I2S MCLK
	PLL_CLKSEL_I2S2_MCLK =               PLL_CLKSEL_R7_OFFSET + 18, //< Clock Select Module ID: I2S2 MCLK
	PLL_CLKSEL_TSE =                     PLL_CLKSEL_R7_OFFSET + 20, //< Clock Select Module ID: TSE

	//PWM Clock Divider bit definition 0x30
	PLL_CLKSEL_PWM0_3_CLKDIV =  PLL_CLKSEL_R8_OFFSET + 0,   //< Clock Select Module ID: PWM0_3 clock divider
	PLL_CLKSEL_PWM4_7_CLKDIV =  PLL_CLKSEL_R8_OFFSET + 16,  //< Clock Select Module ID: PWM4_7 clock divider

	//PWM Clock Divider bit definition 1
	PLL_CLKSEL_PWM8_CLKDIV =    PLL_CLKSEL_R9_OFFSET + 0,  //< Clock Select Module ID: PWM8 clock divider
	PLL_CLKSEL_PWM9_CLKDIV =    PLL_CLKSEL_R9_OFFSET + 16, //< Clock Select Module ID: PWM9 CLK divider

	//PWM Clock Divider bit definition 2
	PLL_CLKSEL_PWM10_CLKDIV =   PLL_CLKSEL_R10_OFFSET + 0,  //< Clock Select Module ID: PWM10 clock divider
	PLL_CLKSEL_PWM11_CLKDIV =   PLL_CLKSEL_R10_OFFSET + 16, //< Clock Select Module ID: PWM11 CLK divider


	//DAI Clock Divider bit definition
	PLL_CLKSEL_DAI_MCLKDIV =    PLL_CLKSEL_R11_OFFSET + 0,  //< Clock Select Module ID: DAI M clock divider
	PLL_CLKSEL_DAI2_MCLKDIV =   PLL_CLKSEL_R11_OFFSET + 8,  //< Clock Select Module ID: DAI2 M clock divider
	PLL_CLKSEL_SP_CLKDIV    =   PLL_CLKSEL_R11_OFFSET + 16, //< Clock Select Module ID: Special clock divider
	PLL_CLKSEL_SP2_CLKDIV    =  PLL_CLKSEL_R11_OFFSET + 24, //< Clock Select Module ID: Special2 clock divider

	//SDIO Clock Divider bit definition
	PLL_CLKSEL_SDIO_CLKDIV =    PLL_CLKSEL_R12_OFFSET + 0,  //< Clock Select Module ID: SDIO clock divider
	PLL_CLKSEL_SDIO2_CLKDIV =   PLL_CLKSEL_R12_OFFSET + 16, //< Clock Select Module ID: SDIO2 clock divider

	// UART Clock Divider bit definition
	PLL_CLKSEL_UART_CLKDIV =    PLL_CLKSEL_R13_OFFSET + 0,  //< Clock Select Module ID: UART clock divider
	PLL_CLKSEL_UART2_CLKDIV =   PLL_CLKSEL_R13_OFFSET + 8,  //< Clock Select Module ID: UART2 clock divider
	PLL_CLKSEL_UART3_CLKDIV =   PLL_CLKSEL_R13_OFFSET + 16, //< Clock Select Module ID: UART3 clock divider
	PLL_CLKSEL_UART4_CLKDIV =   PLL_CLKSEL_R13_OFFSET + 24, //< Clock Select Module ID: UART4 clock divider

	//Peripheral Clock Divider bit definition 0
	PLL_CLKSEL_UART5_CLKDIV =   PLL_CLKSEL_R14_OFFSET + 0,  //< Clock Select Module ID: UART5 clock divider
	PLL_CLKSEL_NAND_CLKDIV =    PLL_CLKSEL_R14_OFFSET + 16, //< Clock Select Module ID: SDIO3 clock divider
	PLL_CLKSEL_ETH_PTP_CLKDIV = PLL_CLKSEL_R14_OFFSET + 24, //< Clock Select Module ID: ETH PTP clock divider
	PLL_CLKSEL_ETH2_PTP_CLKDIV = PLL_CLKSEL_R14_OFFSET + 26, //< Clock Select Module ID: ETH2 PTP clock divider

	//Display Clock Divider bit definition
	PLL_CLKSEL_IDE_CLKDIV =     PLL_CLKSEL_R15_OFFSET + 0,  //< Clock Select Module ID: IDE clock divider
	PLL_CLKSEL_IDE_OUTIF_CLKDIV = PLL_CLKSEL_R15_OFFSET + 8, //< Clock Select Module ID: IDE Output Interface clock divider
	PLL_CLKSEL_IDE2_CLKDIV =    PLL_CLKSEL_R15_OFFSET + 16, //< Clock Select Module ID: IDE2 clock divider
	PLL_CLKSEL_IDE2_OUTIF_CLKDIV = PLL_CLKSEL_R15_OFFSET + 24, //< Clock Select Module ID: IDE2 Output Interface clock divider


	//Peripheral Clock Divider bit definition 2
	PLL_CLKSEL_TRNG_RO_CLKDIV =     PLL_CLKSEL_R16_OFFSET + 0,  //< Clock Select Module ID: TRNG RO clock divider
	PLL_CLKSEL_RO_PHY =             PLL_CLKSEL_R16_OFFSET + 8,  //< Clock Select Module ID: RO PHY
	PLL_CLKSEL_RO_PHY_FREQ =        PLL_CLKSEL_R16_OFFSET + 10, //< Clock Select Module ID: RO PHY FREQ CTL
	PLL_CLKSEL_RO32K_CLKDIV =       PLL_CLKSEL_R16_OFFSET + 12, //< Clock Select Module ID: RO 32K clock divider
	PLL_CLKSEL_RO32K_DIV_TESTEN =   PLL_CLKSEL_R16_OFFSET + 22, //< Ring oscillator range: 53 ~142MHz
	PLL_CLKSEL_TRNG_CLKDIV =        PLL_CLKSEL_R16_OFFSET + 24, //< Clock Select Module ID: TRNG clock divider

	//DSP Clock Divider bit definition 3
	PLL_CLKSEL_DSP_IOP_CLKDIV =    PLL_CLKSEL_R17_OFFSET + 0,  //< Clock Select Module ID: DSP IOP clock divider
	PLL_CLKSEL_DSP_EDP_CLKDIV =    PLL_CLKSEL_R17_OFFSET + 2,  //< Clock Select Module ID: DSP EDP clock divider
	PLL_CLKSEL_DSP_EPP_CLKDIV =    PLL_CLKSEL_R17_OFFSET + 3, //< Clock Select Module ID: DSP EPP clock divider
	PLL_CLKSEL_DSP_EDAP_CLKDIV =   PLL_CLKSEL_R17_OFFSET + 4, //< Clock Select Module ID: DSP EDAP clock divider
	PLL_CLKSEL_DSP_OCEM_CLKDIV =   PLL_CLKSEL_R17_OFFSET + 6, //< Clock Select Module ID: DSP OCEM clock divider

	//Audio Clock Divider bit definition 0
	PLL_CLKSEL_DAI_CLKDIV   =   PLL_CLKSEL_R18_OFFSET + 0, //< Clock Select Module ID: audio clock divider
	PLL_CLKSEL_DAI_OSR_CLKDIV = PLL_CLKSEL_R18_OFFSET + 8, //< Clock Select Module ID: audio OSR clock divider
	PLL_CLKSEL_DAI2_CLKDIV  =   PLL_CLKSEL_R18_OFFSET + 16, //< Clock Select Module ID: audio2 clock divider

	//SDIO Clock Divider bit definition 1
	PLL_CLKSEL_SDIO3_CLKDIV =   PLL_CLKSEL_R19_OFFSET + 0,  //< Clock Select Module ID: SDIO3 clock divider
	PLL_CLKSEL_MI_CLKDIV =      PLL_CLKSEL_R19_OFFSET + 24,  //< Clock Select Module ID: MI clock divider

	// UART Clock Divider bit definition 1
	PLL_CLKSEL_UART6_CLKDIV =   PLL_CLKSEL_R20_OFFSET + 0,  //< Clock Select Module ID: UART6 clock divider
	PLL_CLKSEL_UART7_CLKDIV =   PLL_CLKSEL_R20_OFFSET + 8,  //< Clock Select Module ID: UART7 clock divider
	PLL_CLKSEL_UART8_CLKDIV =   PLL_CLKSEL_R20_OFFSET + 16,  //< Clock Select Module ID: UART8 clock divider
	PLL_CLKSEL_UART9_CLKDIV =   PLL_CLKSEL_R20_OFFSET + 24,  //< Clock Select Module ID: UART9 clock divider

	//SPI Clock Divider bit definition
	PLL_CLKSEL_SPI_CLKDIV =     PLL_CLKSEL_R21_OFFSET + 0,  //< Clock Select Module ID: SPI clock divider
	PLL_CLKSEL_SPI2_CLKDIV =    PLL_CLKSEL_R21_OFFSET + 16, //< Clock Select Module ID: SPI2 clock divider

	//SPI Clock Divider bit definition 1
	PLL_CLKSEL_SPI3_CLKDIV =    PLL_CLKSEL_R22_OFFSET + 0,  //< Clock Select Module ID: SPI3 clock divider
	PLL_CLKSEL_SPI4_CLKDIV =    PLL_CLKSEL_R22_OFFSET + 16, //< Clock Select Module ID: SPI4 clock divider

	//SPI Clock Divider bit definition 1
	PLL_CLKSEL_SPI5_CLKDIV =    PLL_CLKSEL_R23_OFFSET + 0,  //< Clock Select Module ID: SPI5 clock divider

	// Extend IPP Clock Rate bit definition 0
	PLL_CLKSEL_SIE_CLKSRC =     PLL_CLKSEL_R24_OFFSET + 0,  //< Clock Select Module ID: SIE CLK source
	PLL_CLKSEL_SIE2_CLKSRC =     PLL_CLKSEL_R24_OFFSET + 4,  //< Clock Select Module ID: SIE2 CLK source
	PLL_CLKSEL_SIE3_CLKSRC =     PLL_CLKSEL_R24_OFFSET + 8,  //< Clock Select Module ID: SIE3 CLK source
	PLL_CLKSEL_SIE4_CLKSRC =     PLL_CLKSEL_R24_OFFSET + 12,  //< Clock Select Module ID: SIE4 CLK source
	PLL_CLKSEL_SIE5_CLKSRC =     PLL_CLKSEL_R24_OFFSET + 16,  //< Clock Select Module ID: SIE5 CLK source
	PLL_CLKSEL_SIE6_CLKSRC =     PLL_CLKSEL_R24_OFFSET + 20,  //< Clock Select Module ID: SIE6 CLK source

	PLL_CLKSEL_SIE_MCLKSRC =    PLL_CLKSEL_R24_OFFSET + 28,   //< Clock Select Module ID: SIE MCLK source

	// Extend IPP Clock Rate bit definition 1
	PLL_CLKSEL_SIE_MCLK2SRC =    PLL_CLKSEL_R25_OFFSET + 0,   //< Clock Select Module ID: SIE MCLK2 source
	PLL_CLKSEL_SIE_MCLK3SRC =    PLL_CLKSEL_R25_OFFSET + 4,   //< Clock Select Module ID: SIE MCLK3 source
	PLL_CLKSEL_SIE_MCLK4SRC =    PLL_CLKSEL_R25_OFFSET + 8,   //< Clock Select Module ID: SIE MCLK4 source
	PLL_CLKSEL_SIE_MCLK5SRC =    PLL_CLKSEL_R25_OFFSET + 12,   //< Clock Select Module ID: SIE MCLK5 source
	PLL_CLKSEL_VIE_CLKSRC =      PLL_CLKSEL_R25_OFFSET + 16,   //< Clock Select Module ID: VIE  source
	PLL_CLKSEL_VIE2_CLKSRC =      PLL_CLKSEL_R25_OFFSET + 20,   //< Clock Select Module ID: VIE2  source

	// Extend IPP Clock Rate bit definition 2
	PLL_CLKSEL_SIE3_PXCLKSRC =   PLL_CLKSEL_R26_OFFSET + 0,   //< Clock Select Module ID: SIE3 PXCLK  source
	PLL_CLKSEL_SIE4_PXCLKSRC =   PLL_CLKSEL_R26_OFFSET + 4,   //< Clock Select Module ID: SIE4 PXCLK  source
	PLL_CLKSEL_SIE5_PXCLKSRC =   PLL_CLKSEL_R26_OFFSET + 8,   //< Clock Select Module ID: SIE5 PXCLK  source
	PLL_CLKSEL_SIE6_PXCLKSRC =   PLL_CLKSEL_R26_OFFSET + 12,  //< Clock Select Module ID: SIE6 PXCLK  source


	// Extended IPP Clock Divider Register 0
	PLL_CLKSEL_SIE_CLKDIV =     PLL_CLKSEL_R27_OFFSET + 0,   //< Clock Select Module ID: SIE CLK divider
	PLL_CLKSEL_SIE2_CLKDIV =     PLL_CLKSEL_R27_OFFSET + 8,   //< Clock Select Module ID: SIE2 CLK divider
	PLL_CLKSEL_SIE3_CLKDIV =     PLL_CLKSEL_R27_OFFSET + 16,   //< Clock Select Module ID: SIE3 CLK divider
	PLL_CLKSEL_SIE4_CLKDIV =     PLL_CLKSEL_R27_OFFSET + 24,   //< Clock Select Module ID: SIE4 CLK divider

	// Extended IPP Clock Divider Register 1
	PLL_CLKSEL_SIE5_CLKDIV =     PLL_CLKSEL_R28_OFFSET + 0,   //< Clock Select Module ID: SIE5 CLK divider
	PLL_CLKSEL_SIE6_CLKDIV =     PLL_CLKSEL_R28_OFFSET + 8,   //< Clock Select Module ID: SIE6 CLK divider
	PLL_CLKSEL_SIE_MCLKDIV =     PLL_CLKSEL_R28_OFFSET + 16,   //< Clock Select Module ID: SIE MCLK divider
	PLL_CLKSEL_SIE_MCLK2DIV =     PLL_CLKSEL_R28_OFFSET + 24,   //< Clock Select Module ID: SIE MCLK2 divider

	// Extended IPP Clock Divider Register 2
	PLL_CLKSEL_SIE_MCLK3DIV =     PLL_CLKSEL_R29_OFFSET + 0,   //< Clock Select Module ID: SIE MCLK3 divider
	PLL_CLKSEL_SIE_MCLK4DIV =     PLL_CLKSEL_R29_OFFSET + 8,   //< Clock Select Module ID: SIE MCLK4 divider
	PLL_CLKSEL_SIE_MCLK5DIV =     PLL_CLKSEL_R29_OFFSET + 16,   //< Clock Select Module ID: SIE MCLK5 divider
	PLL_CLKSEL_VIE_CLKDIV =       PLL_CLKSEL_R29_OFFSET + 24,   //< Clock Select Module ID: VIE CLK divider

	// Extended IPP Clock Divider Register 3
	PLL_CLKSEL_VIE2_CLKDIV =       PLL_CLKSEL_R30_OFFSET + 0,   //< Clock Select Module ID: VIE2 CLK divider

	PLL_CLKSEL_MAX_ITEM,                                    // magic number for pll checking usage.
	ENUM_DUMMY4WORD(PLL_CLKSEL)
} PLL_CLKSEL;


#define PLL_CLKSEL_H26X         PLL_CLKSEL_H265


/*
    @name   CPU clock rate value

    CPU clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_CPU).
*/
//@{
#define PLL_CLKSEL_CPU_80         (0x00 << PLL_CLKSEL_CPU)      //< Select CPU clock 80MHz
#define PLL_CLKSEL_CPU_PLL16      (0x01 << PLL_CLKSEL_CPU)      //< Select CPU clock PLL16 (for CPU)
#define PLL_CLKSEL_CPU_APLL       PLL_CLKSEL_CPU_PLL16          //< Select CPU clock APLL
#define PLL_CLKSEL_CPU_480        (0x02 << PLL_CLKSEL_CPU)      //< Select CPU clock 480MHz
//@}

/*
    @name   APB clock rate value

    APB clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_APB).
*/
//@{
#define PLL_CLKSEL_APB_120        (0x00 << PLL_CLKSEL_APB)      //< Select APB  120MHz
#define PLL_CLKSEL_APB_150        (0x01 << PLL_CLKSEL_APB)      //< Select APB  150MHz

//@}

/*
    @name   Power Down mode source clock selection

    Power Down mode source clock selection
*/
//@{
#define PLL_CLKSEL_PD_RTC        (0x00 << PLL_CLKSEL_PD)      //< Select PD source RTC
#define PLL_CLKSEL_PD_RINGOSC    (0x01 << PLL_CLKSEL_PD)      //< Select PD source RingOSC
//@}

/*
    @name   DMA Arbiter clock rate value

    DMA Arbiter clock rate value

    @note This is for pll_setClockRate(PLL_CLKSEL_DMA_ARRT).
*/
//@{
#define PLL_CLKSEL_DMA_ARBT_DDR1  (0x00)      //< Select DMA Arbiter DDR1 PHY
#define PLL_CLKSEL_DMA_ARBT_DDR2  (0x01)      //< Select DMA Arbiter DDR2 PHY
//@}

/*
    @name   DDR-PHY clock rate value

    DDR-PHY clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_DDRPHY).
*/
//@{
#define PLL_CLKSEL_DDRPHY_PLL3    (0x00 << PLL_CLKSEL_DDRPHY)      //< Select DDRPHY clock from PLL3
#define PLL_CLKSEL_DDRPHY_ETHPHY  (0x01 << PLL_CLKSEL_DDRPHY)      //< Select DDRPHY clock from Ethernet PHY
//@}

/*
    @name   CNN clock rate value

    CNN clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_CNN).
*/
//@{

#define PLL_CLKSEL_CNN_PLL8      (0x00 << (PLL_CLKSEL_CNN - PLL_CLKSEL_R1_OFFSET)) //< Select CNN clock as PLL8
#define PLL_CLKSEL_CNN_PLL15     (0x01 << (PLL_CLKSEL_CNN - PLL_CLKSEL_R1_OFFSET)) //< Select CNN clock as PLL15
//@}


/*
    @name   CNN2 clock rate value

    CNN2 clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_CNN2).
*/
//@{

#define PLL_CLKSEL_CNN2_PLL8      (0x00 << (PLL_CLKSEL_CNN2 - PLL_CLKSEL_R1_OFFSET)) //< Select CNN2 clock as PLL8
#define PLL_CLKSEL_CNN2_PLL15     (0x01 << (PLL_CLKSEL_CNN2 - PLL_CLKSEL_R1_OFFSET)) //< Select CNN2 clock as PLL15
//@}


/*
    @name   IVE clock rate value

    IVE clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_IVE).
*/
//@{
#define PLL_CLKSEL_IVE_240        (0x02 << (PLL_CLKSEL_IVE - PLL_CLKSEL_R1_OFFSET)) //< Select IVE clock as 240MHz
#define PLL_CLKSEL_IVE_320        (0x01 << (PLL_CLKSEL_IVE - PLL_CLKSEL_R1_OFFSET)) //< Select IVE clock as 320MHz
#define PLL_CLKSEL_IVE_480        (0x00 << (PLL_CLKSEL_IVE - PLL_CLKSEL_R1_OFFSET)) //< Select IVE clock as 480MHz
//@}

/*
    @name   TRKE clock rate value

    TRKE clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_TRKE).
*/
//@{
#define PLL_CLKSEL_TRKE_240        (0x02 << (PLL_CLKSEL_TRKE - PLL_CLKSEL_R1_OFFSET)) //< Select TRKE clock as 240MHz
#define PLL_CLKSEL_TRKE_320        (0x01 << (PLL_CLKSEL_TRKE - PLL_CLKSEL_R1_OFFSET)) //< Select TRKE clock as 320MHz
#define PLL_CLKSEL_TRKE_480        (0x00 << (PLL_CLKSEL_TRKE - PLL_CLKSEL_R1_OFFSET)) //< Select TRKE clock as 480MHz
//@}

/*
    @name   MDBC clock rate value

    TRKE clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_MDBC).
*/
//@{
#define PLL_CLKSEL_MDBC_240        (0x00 << (PLL_CLKSEL_MDBC - PLL_CLKSEL_R1_OFFSET)) //< Select MDBC clock as 240MHz
#define PLL_CLKSEL_MDBC_320        (0x01 << (PLL_CLKSEL_MDBC - PLL_CLKSEL_R1_OFFSET)) //< Select MDBC clock as 320MHz
#define PLL_CLKSEL_MDBC_192        (0x02 << (PLL_CLKSEL_MDBC - PLL_CLKSEL_R1_OFFSET)) //< Select MDBC clock as 192MHz
#define PLL_CLKSEL_MDBC_96         (0x03 << (PLL_CLKSEL_MDBC - PLL_CLKSEL_R1_OFFSET)) //< Select MDBC clock as 96MHz

//@}

/*
    @name   ISE clock rate value

    ISE clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_ISE).
*/
//@{
#define PLL_CLKSEL_ISE_240        (0x00 << (PLL_CLKSEL_ISE - PLL_CLKSEL_R1_OFFSET)) //< Select ISE clock as 240MHz
#define PLL_CLKSEL_ISE_320        (0x01 << (PLL_CLKSEL_ISE - PLL_CLKSEL_R1_OFFSET)) //< Select ISE clock as 320MHz
#define PLL_CLKSEL_ISE_480        (0x02 << (PLL_CLKSEL_ISE - PLL_CLKSEL_R1_OFFSET)) //< Select ISE clock as 480MHz
#define PLL_CLKSEL_ISE_PLL15      (0x03 << (PLL_CLKSEL_ISE - PLL_CLKSEL_R1_OFFSET)) //< Select ISE clock as PLL15

//@}


/*
    @name   DIS clock rate value

    DIS clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_DIS).
*/
//@{
#define PLL_CLKSEL_DIS_240        (0x00 << (PLL_CLKSEL_DIS - PLL_CLKSEL_R1_OFFSET)) //< Select DIS clock as 240MHz
#define PLL_CLKSEL_DIS_320        (0x01 << (PLL_CLKSEL_DIS - PLL_CLKSEL_R1_OFFSET)) //< Select DIS clock as 320MHz
#define PLL_CLKSEL_DIS_480        (0x02 << (PLL_CLKSEL_DIS - PLL_CLKSEL_R1_OFFSET)) //< Select DIS clock as 480MHz
//@}


/*
    @name   NUE clock rate value

    NUE clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_NUE).
*/
//@{
#define PLL_CLKSEL_NUE_PLL8       (0x00 << (PLL_CLKSEL_NUE - PLL_CLKSEL_R1_OFFSET)) //< Select NUE2 clock as PLL8
#define PLL_CLKSEL_NUE_PLL15      (0x01 << (PLL_CLKSEL_NUE - PLL_CLKSEL_R1_OFFSET)) //< Select NUE2 clock as PLL15
#define PLL_CLKSEL_NUE_320        (0x02 << (PLL_CLKSEL_NUE - PLL_CLKSEL_R1_OFFSET)) //< Select NUE2 clock as 320MHz
#define PLL_CLKSEL_NUE_480        (0x03 << (PLL_CLKSEL_NUE - PLL_CLKSEL_R1_OFFSET)) //< Select NUE2 clock as 480MHz
//@}


/*
    @name   NUE2 clock rate value

    NUE2 clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_NUE2).
*/
//@{
#define PLL_CLKSEL_NUE2_PLL8       (0x00 << (PLL_CLKSEL_NUE2 - PLL_CLKSEL_R1_OFFSET)) //< Select NUE2 clock as PLL8
#define PLL_CLKSEL_NUE2_PLL15      (0x01 << (PLL_CLKSEL_NUE2 - PLL_CLKSEL_R1_OFFSET)) //< Select NUE2 clock as PLL15
#define PLL_CLKSEL_NUE2_320        (0x02 << (PLL_CLKSEL_NUE2 - PLL_CLKSEL_R1_OFFSET)) //< Select NUE2 clock as 320MHz
#define PLL_CLKSEL_NUE2_480        (0x03 << (PLL_CLKSEL_NUE2 - PLL_CLKSEL_R1_OFFSET)) //< Select NUE2 clock as 480MHz
//@}


/*
    @name   IME clock rate value

    IME clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_IME).
*/
//@{
#define PLL_CLKSEL_IME_240        (0x00 << (PLL_CLKSEL_IME - PLL_CLKSEL_R1_OFFSET)) //< Select IME clock as 240MHz
#define PLL_CLKSEL_IME_320        (0x01 << (PLL_CLKSEL_IME - PLL_CLKSEL_R1_OFFSET)) //< Select IME clock as 320MHz
#define PLL_CLKSEL_IME_480        (0x02 << (PLL_CLKSEL_IME - PLL_CLKSEL_R1_OFFSET)) //< Select IME clock as 480MHz
#define PLL_CLKSEL_IME_PLL15      (0x03 << (PLL_CLKSEL_IME - PLL_CLKSEL_R1_OFFSET)) //< Select IME clock as PLL15
//@}



/*
    @name   IPE clock rate value

    IPE clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_IPE).
*/
//@{
#define PLL_CLKSEL_IPE_240        (0x00 << (PLL_CLKSEL_IPE - PLL_CLKSEL_R1_OFFSET)) //< Select IPE clock as 240MHz
#define PLL_CLKSEL_IPE_320        (0x01 << (PLL_CLKSEL_IPE - PLL_CLKSEL_R1_OFFSET)) //< Select IPE clock as 320MHz
#define PLL_CLKSEL_IPE_480        (0x02 << (PLL_CLKSEL_IPE - PLL_CLKSEL_R1_OFFSET)) //< Select IPE clock as 480MHz
#define PLL_CLKSEL_IPE_PLL15      (0x03 << (PLL_CLKSEL_IPE - PLL_CLKSEL_R1_OFFSET)) //< Select IPE clock as PLL15
//@}


/*
    @name   DCE clock rate value

    DCE clock rate value

    This is for pll_set_clock_rate(PLL_CLKSEL_DCE).
*/
//@{
#define PLL_CLKSEL_DCE_240        (0x00 << (PLL_CLKSEL_DCE - PLL_CLKSEL_R1_OFFSET)) //< Select DCE clock as 240MHz
#define PLL_CLKSEL_DCE_320        (0x01 << (PLL_CLKSEL_DCE - PLL_CLKSEL_R1_OFFSET)) //< Select DCE clock as 320MHz
#define PLL_CLKSEL_DCE_480        (0x02 << (PLL_CLKSEL_DCE - PLL_CLKSEL_R1_OFFSET)) //< Select DCE clock as 480MHz
#define PLL_CLKSEL_DCE_PLL15      (0x03 << (PLL_CLKSEL_DCE - PLL_CLKSEL_R1_OFFSET)) //< Select DCE clock as PLL15
//@}

/*
    @name   IFE clock rate value

    IFE clock rate value

    This is for pll_set_clock_rate(PLL_CLKSEL_IFE).
*/
//@{
#define PLL_CLKSEL_IFE_240        (0x00 << (PLL_CLKSEL_IFE - PLL_CLKSEL_R1_OFFSET)) //< Select IFE clock as 240MHz
#define PLL_CLKSEL_IFE_320        (0x01 << (PLL_CLKSEL_IFE - PLL_CLKSEL_R1_OFFSET)) //< Select IFE clock as 320MHz
#define PLL_CLKSEL_IFE_480        (0x02 << (PLL_CLKSEL_IFE - PLL_CLKSEL_R1_OFFSET)) //< Select IFE clock as 480MHz
#define PLL_CLKSEL_IFE_PLL15      (0x03 << (PLL_CLKSEL_IFE - PLL_CLKSEL_R1_OFFSET)) //< Select IFE clock as PLL15
//@}


/*
    @name   VPE clock rate value

    VPE clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_VPE).
*/
//@{
#define PLL_CLKSEL_VPE_240          (0x00 << (PLL_CLKSEL_VPE - PLL_CLKSEL_R1_OFFSET)) //< Select VPE clock as 240MHz
#define PLL_CLKSEL_VPE_320          (0x01 << (PLL_CLKSEL_VPE - PLL_CLKSEL_R1_OFFSET)) //< Select VPE clock as 320MHz
#define PLL_CLKSEL_VPE_480          (0x02 << (PLL_CLKSEL_VPE - PLL_CLKSEL_R1_OFFSET)) //< Select VPE clock as 480MHz
#define PLL_CLKSEL_VPE_PLL18        (0x03 << (PLL_CLKSEL_VPE - PLL_CLKSEL_R1_OFFSET)) //< Select VPE clock as PLL18

//@}

/*
    @name   VPE LITE clock rate value

    VPE LITE clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_VPE_LITE).
*/
//@{
#define PLL_CLKSEL_VPE_LITE_240          (0x00 << (PLL_CLKSEL_VPE_LITE - PLL_CLKSEL_R1_OFFSET)) //< Select VPE LITE clock as 240MHz
#define PLL_CLKSEL_VPE_LITE_320          (0x01 << (PLL_CLKSEL_VPE_LITE - PLL_CLKSEL_R1_OFFSET)) //< Select VPE LITE clock as 320MHz
#define PLL_CLKSEL_VPE_LITE_480          (0x02 << (PLL_CLKSEL_VPE_LITE - PLL_CLKSEL_R1_OFFSET)) //< Select VPE LITE clock as 480MHz
#define PLL_CLKSEL_VPE_LITE_PLL18        (0x03 << (PLL_CLKSEL_VPE_LITE - PLL_CLKSEL_R1_OFFSET)) //< Select VPE LITE clock as PLL18

//@}


/*
    @name   ISE2 clock rate value

    ISE2 clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_ISE2).
*/
//@{
#define PLL_CLKSEL_ISE2_240        (0x00 << (PLL_CLKSEL_ISE2 - PLL_CLKSEL_R1_OFFSET)) //< Select ISE2 clock as 240MHz
#define PLL_CLKSEL_ISE2_320        (0x01 << (PLL_CLKSEL_ISE2 - PLL_CLKSEL_R1_OFFSET)) //< Select ISE2 clock as 320MHz
#define PLL_CLKSEL_ISE2_480        (0x02 << (PLL_CLKSEL_ISE2 - PLL_CLKSEL_R1_OFFSET)) //< Select ISE2 clock as 480MHz
#define PLL_CLKSEL_ISE2_PLL15      (0x03 << (PLL_CLKSEL_ISE2 - PLL_CLKSEL_R1_OFFSET)) //< Select ISE2 clock as PLL15



/*
    @name   SDE clock rate value

    SDE clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SDE).
*/
//@{
#define PLL_CLKSEL_SDE_240        (0x00 << (PLL_CLKSEL_SDE - PLL_CLKSEL_R1_OFFSET)) //< Select SDE clock as 240MHz
#define PLL_CLKSEL_SDE_320        (0x01 << (PLL_CLKSEL_SDE - PLL_CLKSEL_R1_OFFSET)) //< Select SDE clock as 320MHz
#define PLL_CLKSEL_SDE_480        (0x02 << (PLL_CLKSEL_SDE - PLL_CLKSEL_R1_OFFSET)) //< Select SDE clock as 480MHz
#define PLL_CLKSEL_SDE_PLL21      (0x03 << (PLL_CLKSEL_SDE - PLL_CLKSEL_R1_OFFSET)) //< Select SDE clock as PLL21
//@}

//////////////////////////////////// R1 select done.

/*
    @name   SIE MCLK clock Inverted Output

    Invert output phase of SIE MCLK

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLKINV).
*/
//@{
#define PLL_CLKSEL_SIE_MCLKINV_DIS      (0x00 << (PLL_CLKSEL_SIE_MCLKINV - PLL_CLKSEL_R2_OFFSET))  //< Select SIE MCLK normal output
#define PLL_CLKSEL_SIE_MCLKINV_EN       (0x01 << (PLL_CLKSEL_SIE_MCLKINV - PLL_CLKSEL_R2_OFFSET))  //< Select SIE MCLK inverted output
//@}

/*
    @name   SIE MCLK2 clock Inverted Output

    Invert output phase of SIE MCLK2

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK2INV).
*/
//@{
#define PLL_CLKSEL_SIE_MCLK2INV_DIS      (0x00 << (PLL_CLKSEL_SIE_MCLK2INV - PLL_CLKSEL_R2_OFFSET))  //< Select SIE MCLK2 normal output
#define PLL_CLKSEL_SIE_MCLK2INV_EN       (0x01 << (PLL_CLKSEL_SIE_MCLK2INV - PLL_CLKSEL_R2_OFFSET))  //< Select SIE MCLK2 inverted output
//@}

/*
    @name   SIE MCLK3 clock Inverted Output

    Invert output phase of SIE MCLK3

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK3INV).
*/
//@{
#define PLL_CLKSEL_SIE_MCLK3INV_DIS      (0x00 << (PLL_CLKSEL_SIE_MCLK3INV - PLL_CLKSEL_R2_OFFSET))  //< Select SIE MCLK3 normal output
#define PLL_CLKSEL_SIE_MCLK3INV_EN       (0x01 << (PLL_CLKSEL_SIE_MCLK3INV - PLL_CLKSEL_R2_OFFSET))  //< Select SIE MCLK3 inverted output
//@}

/*
    @name   SIE MCLK4 clock Inverted Output

    Invert output phase of SIE MCLK4

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK4INV).
*/
//@{
#define PLL_CLKSEL_SIE_MCLK4INV_DIS      (0x00 << (PLL_CLKSEL_SIE_MCLK4INV - PLL_CLKSEL_R2_OFFSET))  //< Select SIE MCLK4 normal output
#define PLL_CLKSEL_SIE_MCLK4INV_EN       (0x01 << (PLL_CLKSEL_SIE_MCLK4INV - PLL_CLKSEL_R2_OFFSET))  //< Select SIE MCLK4 inverted output
//@}

/*
    @name   SIE MCLK5 clock Inverted Output

    Invert output phase of SIE MCLK5

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK5INV).
*/
//@{
#define PLL_CLKSEL_SIE_MCLK5INV_DIS      (0x00 << (PLL_CLKSEL_SIE_MCLK5INV - PLL_CLKSEL_R2_OFFSET))  //< Select SIE MCLK5 normal output
#define PLL_CLKSEL_SIE_MCLK5INV_EN       (0x01 << (PLL_CLKSEL_SIE_MCLK5INV - PLL_CLKSEL_R2_OFFSET))  //< Select SIE MCLK5 inverted output
//@}


/*
    @name   TGE CLK clock source value

    TGE CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_TGE).
*/
//@{
#define PLL_CLKSEL_TGE_SIEMCLK           (0x00 << (PLL_CLKSEL_TGE - PLL_CLKSEL_R2_OFFSET))  //< Select TGE CLK source as SIE MCLK
#define PLL_CLKSEL_TGE_SIEMCLK2          (0x01 << (PLL_CLKSEL_TGE - PLL_CLKSEL_R2_OFFSET))  //< Select TGE CLK source as SIE MCLK2
#define PLL_CLKSEL_TGE_SIEMCLK3          (0x02 << (PLL_CLKSEL_TGE - PLL_CLKSEL_R2_OFFSET))  //< Select TGE CLK source as SIE MCLK3
#define PLL_CLKSEL_TGE_SIEMCLK4          (0x03 << (PLL_CLKSEL_TGE - PLL_CLKSEL_R2_OFFSET))  //< Select TGE CLK source as SIE MCLK4
#define PLL_CLKSEL_TGE_SIEMCLK5          (0x04 << (PLL_CLKSEL_TGE - PLL_CLKSEL_R2_OFFSET))  //< Select TGE CLK source as SIE MCLK5
//@}



//////////////////////////////////// R2 select done.


/*
    @name   JPEG clock rate value

    JPEG clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_JPEG).
*/
//@{
#define PLL_CLKSEL_JPEG_320       (0x01 << (PLL_CLKSEL_JPEG - PLL_CLKSEL_R3_OFFSET)) //< Select JPEG clock as 320MHz
#define PLL_CLKSEL_JPEG_480       (0x00 << (PLL_CLKSEL_JPEG - PLL_CLKSEL_R3_OFFSET)) //< Select JPEG clock as 480MHz
#define PLL_CLKSEL_JPEG_PLL18     (0x02 << (PLL_CLKSEL_JPEG - PLL_CLKSEL_R3_OFFSET)) //< Select JPEG clock as PLL18
//@}


/*
    @name   UVCP clock rate value

    UVCP clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_UVCP).
*/
//@{
#define PLL_CLKSEL_UVCP_240     (0x00 << (PLL_CLKSEL_UVCP - PLL_CLKSEL_R3_OFFSET))  //< Select UVCP clock as 240Mhz
#define PLL_CLKSEL_UVCP_320     (0x01 << (PLL_CLKSEL_UVCP - PLL_CLKSEL_R3_OFFSET))  //< Select UVCP clock as 320Mhz
#define PLL_CLKSEL_UVCP_120     (0x02 << (PLL_CLKSEL_UVCP - PLL_CLKSEL_R3_OFFSET))  //< Select UVCP clock as 120Mhz
//@}


/*
    @name VENC clock rate value

    VENC clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_H265).
*/
//@{
#define PLL_CLKSEL_VENC_PLL14       (0x00 << (PLL_CLKSEL_VENC - PLL_CLKSEL_R3_OFFSET)) //< Select VENC clock as PLL14
#define PLL_CLKSEL_VENC_PLL24       (0x01 << (PLL_CLKSEL_VENC - PLL_CLKSEL_R3_OFFSET)) //< Select VENC clock as PLL24
#define PLL_CLKSEL_VENC_PLL15       (0x02 << (PLL_CLKSEL_VENC - PLL_CLKSEL_R3_OFFSET)) //< Select VENC clock as PLL15
#define PLL_CLKSEL_VENC_320         (0x03 << (PLL_CLKSEL_VENC - PLL_CLKSEL_R3_OFFSET)) //< Select VENC clock as 320
//@}


/*
    @name   DRE clock rate value

    DRE clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_DRE).
*/
//@{
#define PLL_CLKSEL_DRE_240     (0x00 << (PLL_CLKSEL_DRE - PLL_CLKSEL_R3_OFFSET))  //< Select DRE clock as 240Mhz
#define PLL_CLKSEL_DRE_320     (0x01 << (PLL_CLKSEL_DRE - PLL_CLKSEL_R3_OFFSET))  //< Select DRE clock as 320Mhz
#define PLL_CLKSEL_DRE_480     (0x02 << (PLL_CLKSEL_DRE - PLL_CLKSEL_R3_OFFSET))  //< Select DRE clock as 480Mhz
//@}



/*
    @name   JPEG2 clock rate value

    JPEG2 clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_JPEG2).
*/
//@{
#define PLL_CLKSEL_JPEG2_320       (0x01 << (PLL_CLKSEL_JPEG2 - PLL_CLKSEL_R3_OFFSET)) //< Select JPEG2 clock as 320MHz
#define PLL_CLKSEL_JPEG2_480       (0x00 << (PLL_CLKSEL_JPEG2 - PLL_CLKSEL_R3_OFFSET)) //< Select JPEG2 clock as 480MHz
#define PLL_CLKSEL_JPEG2_PLL18     (0x02 << (PLL_CLKSEL_JPEG2 - PLL_CLKSEL_R3_OFFSET)) //< Select JPEG2 clock as PLL18
//@}

/*
    @name   H264D_A clock rate value

    H264D_A clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_H264D_A).
*/
//@{
#define PLL_CLKSEL_H264D_A_PLL18    (0x00 << (PLL_CLKSEL_H264D_A - PLL_CLKSEL_R3_OFFSET)) //< Select H264D_A clock as 480MHz
#define PLL_CLKSEL_H264D_A_PLL13    (0x01 << (PLL_CLKSEL_H264D_A - PLL_CLKSEL_R3_OFFSET)) //< Select H264D_A clock as 320MHz
#define PLL_CLKSEL_H264D_A_PLL0     (0x02 << (PLL_CLKSEL_H264D_A - PLL_CLKSEL_R3_OFFSET)) //< Select H264D_A clock as PLL18
#define PLL_CLKSEL_H264D_A_PLL24    (0x03 << (PLL_CLKSEL_H264D_A - PLL_CLKSEL_R3_OFFSET)) //< Select H264D_A clock as PLL18
//@}

/*
    @name   H265D_A clock rate value

    H265D_A clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_H265D_A).
*/
//@{
#define PLL_CLKSEL_H265D_A_PLL13    (0x0 << (PLL_CLKSEL_H265D_A - PLL_CLKSEL_R3_OFFSET)) //< Select H265D_A clock as 320MHz
#define PLL_CLKSEL_H265D_A_PLL2     (0x1 << (PLL_CLKSEL_H265D_A - PLL_CLKSEL_R3_OFFSET)) //< Select H265D_A clock as 480MHz
//@}

/*
    @name   H264D_M clock rate value

    H264D_M clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_H264D_A).
*/
//@{
#define PLL_CLKSEL_H264D_M_PLL18    (0x00 << (PLL_CLKSEL_H264D_M - PLL_CLKSEL_R3_OFFSET)) //< Select H264D_M clock as 480MHz
#define PLL_CLKSEL_H264D_M_PLL0     (0x01 << (PLL_CLKSEL_H264D_M - PLL_CLKSEL_R3_OFFSET)) //< Select H264D_M clock as 320MHz
#define PLL_CLKSEL_H264D_M_PLL24    (0x02 << (PLL_CLKSEL_H264D_M - PLL_CLKSEL_R3_OFFSET)) //< Select H264D_M clock as PLL18
//@}

/*
    @name   H265D_M clock rate value

    H265D_M clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_H265D_M).
*/
//@{
#define PLL_CLKSEL_H265D_M_PLL15    (0x00 << (PLL_CLKSEL_H265D_M - PLL_CLKSEL_R3_OFFSET)) //< Select H265D_M clock as 480MHz
#define PLL_CLKSEL_H265D_M_PLL0     (0x01 << (PLL_CLKSEL_H265D_M - PLL_CLKSEL_R3_OFFSET)) //< Select H265D_M clock as 320MHz
#define PLL_CLKSEL_H265D_M_PLL18    (0x02 << (PLL_CLKSEL_H265D_M - PLL_CLKSEL_R3_OFFSET)) //< Select H265D_M clock as PLL18
//@}


/*
    @name   JPEG Lite clock rate value

    JPEG Lite clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_JPEG_LITE).
*/
//@{
#define PLL_CLKSEL_JPEG_LITE_320       (0x01 << (PLL_CLKSEL_JPEG_LITE - PLL_CLKSEL_R3_OFFSET)) //< Select JPEG Lite clock as 320MHz
#define PLL_CLKSEL_JPEG_LITE_480       (0x00 << (PLL_CLKSEL_JPEG_LITE - PLL_CLKSEL_R3_OFFSET)) //< Select JPEG Lite clock as 480MHz
#define PLL_CLKSEL_JPEG_LITE_PLL18     (0x02 << (PLL_CLKSEL_JPEG_LITE - PLL_CLKSEL_R3_OFFSET)) //< Select JPEG Lite clock as PLL18
//@}

/*
    @name   VTRC AE clock rate value

    VTRC AE Lite clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_VTRC_AE).
*/
//@{
#define PLL_CLKSEL_VTRC_AE_PLL24      (0x00 << (PLL_CLKSEL_VTRC_AE - PLL_CLKSEL_R3_OFFSET)) //< Select VTRC AE clock as PLL24
#define PLL_CLKSEL_VTRC_AE_VENC_CLK   (0x01 << (PLL_CLKSEL_VTRC_AE - PLL_CLKSEL_R3_OFFSET)) //< Select VTRC AE clock as the result of PLL_CLKSEL_VENC
//@}



//////////////////////////////////// R3 select done.

/*
    @name   SDIO clock rate value

    SDIO clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SDIO).
*/
//@{
#define PLL_CLKSEL_SDIO_192          (0x00 << (PLL_CLKSEL_SDIO - PLL_CLKSEL_R4_OFFSET))  //< Select SDIO clock as 192Mhz
#define PLL_CLKSEL_SDIO_480          (0x01 << (PLL_CLKSEL_SDIO - PLL_CLKSEL_R4_OFFSET))  //< Select SDIO clock as 480Mhz
#define PLL_CLKSEL_SDIO_320          (0x02 << (PLL_CLKSEL_SDIO - PLL_CLKSEL_R4_OFFSET))  //< Select SDIO clock as 320Mhz
#define PLL_CLKSEL_SDIO_PLL6         (0x03 << (PLL_CLKSEL_SDIO - PLL_CLKSEL_R4_OFFSET))  //< Select SDIO clock as PLL6
//@}


/*
    @name   SDIO2 clock rate value

    SDIO2 clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SDIO2).
*/
//@{
#define PLL_CLKSEL_SDIO2_192         (0x00 << (PLL_CLKSEL_SDIO2 - PLL_CLKSEL_R4_OFFSET)) //< Select SDIO2 clock as 192Mhz
#define PLL_CLKSEL_SDIO2_480         (0x01 << (PLL_CLKSEL_SDIO2 - PLL_CLKSEL_R4_OFFSET)) //< Select SDIO2 clock as 480Mhz
#define PLL_CLKSEL_SDIO2_320         (0x02 << (PLL_CLKSEL_SDIO2 - PLL_CLKSEL_R4_OFFSET)) //< Select SDIO2 clock as 320Mhz
#define PLL_CLKSEL_SDIO2_PLL6        (0x03 << (PLL_CLKSEL_SDIO2 - PLL_CLKSEL_R4_OFFSET)) //< Select SDIO2 clock as PLL6
//@}


/*
    @name   SDIO3 clock rate value

    SDIO3 clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SDIO3).
*/
//@{
#define PLL_CLKSEL_SDIO3_192         (0x00 << (PLL_CLKSEL_SDIO3 - PLL_CLKSEL_R4_OFFSET)) //< Select SDIO3 clock as 192Mhz
#define PLL_CLKSEL_SDIO3_480         (0x01 << (PLL_CLKSEL_SDIO3 - PLL_CLKSEL_R4_OFFSET)) //< Select SDIO3 clock as 480Mhz
#define PLL_CLKSEL_SDIO3_320         (0x02 << (PLL_CLKSEL_SDIO3 - PLL_CLKSEL_R4_OFFSET)) //< Select SDIO3 clock as 320Mhz
#define PLL_CLKSEL_SDIO3_PLL6        (0x03 << (PLL_CLKSEL_SDIO3 - PLL_CLKSEL_R4_OFFSET)) //< Select SDIO3 clock as PLL6
//@}


/*
    @name   Ethernet PHY Reference clock rate value

    Ethernet PHY Reference clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_ETHPHY_CLKSRC).
*/
//@{
#define PLL_CLKSEL_ETHPHY_CLKSRC_PLL7          (0x00 << (PLL_CLKSEL_ETHPHY_CLKSRC - PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_ETHPHY_CLKSRC_SPCLK         (0x01 << (PLL_CLKSEL_ETHPHY_CLKSRC - PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_ETHPHY_CLKSRC_SP2CLK        (0x02 << (PLL_CLKSEL_ETHPHY_CLKSRC - PLL_CLKSEL_R4_OFFSET))
//@}


/*
    @name   SP clock rate value

    SP clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SP).
*/
//@{
#define PLL_CLKSEL_SP_480            (0x00 << (PLL_CLKSEL_SP - PLL_CLKSEL_R4_OFFSET))  //< Select SP clock as 480Mhz
#define PLL_CLKSEL_SP_PLL5           (0x01 << (PLL_CLKSEL_SP - PLL_CLKSEL_R4_OFFSET))  //< Select SP clock as PLL5
#define PLL_CLKSEL_SP_PLL6           (0x02 << (PLL_CLKSEL_SP - PLL_CLKSEL_R4_OFFSET))  //< Select SP clock as PLL6
#define PLL_CLKSEL_SP_PLL11          (0x03 << (PLL_CLKSEL_SP - PLL_CLKSEL_R4_OFFSET))  //< Select SP clock as PLL11

//@}

/*
    @name   SP2 clock rate value

    SP2 clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SP2).
*/
//@{
#define PLL_CLKSEL_SP2_480           (0x00 << (PLL_CLKSEL_SP2 - PLL_CLKSEL_R4_OFFSET)) //< Select SP2 clock as 240Mhz
#define PLL_CLKSEL_SP2_PLL5          (0x01 << (PLL_CLKSEL_SP2 - PLL_CLKSEL_R4_OFFSET)) //< Select SP2 clock as PLL5
#define PLL_CLKSEL_SP2_PLL6          (0x02 << (PLL_CLKSEL_SP2 - PLL_CLKSEL_R4_OFFSET)) //< Select SP2 clock as PLL6
#define PLL_CLKSEL_SP2_PLL11         (0x03 << (PLL_CLKSEL_SP2 - PLL_CLKSEL_R4_OFFSET)) //< Select SP2 clock as PLL11
//@}

/*
    @name   GPENC clock rate value

    GPENC clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_GPENC).
*/
//@{
#define PLL_CLKSEL_GPENC_240          (0x00 << (PLL_CLKSEL_GPENC - PLL_CLKSEL_R4_OFFSET)) //< Select GPENC clock as 240Mhz
#define PLL_CLKSEL_GPENC_320          (0x01 << (PLL_CLKSEL_GPENC - PLL_CLKSEL_R4_OFFSET)) //< Select GPENC clock as 320Mhz
#define PLL_CLKSEL_GPENC_480          (0x02 << (PLL_CLKSEL_GPENC - PLL_CLKSEL_R4_OFFSET)) //< Select GPENC clock as 480Mhz
//@}


/*
    @name   GPENC2 clock rate value

    GPENC2 clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_GPENC).
*/
//@{
#define PLL_CLKSEL_GPENC2_240          (0x00 << (PLL_CLKSEL_GPENC2 - PLL_CLKSEL_R4_OFFSET)) //< Select GPENC2 clock as 240Mhz
#define PLL_CLKSEL_GPENC2_320          (0x01 << (PLL_CLKSEL_GPENC2 - PLL_CLKSEL_R4_OFFSET)) //< Select GPENC2 clock as 320Mhz
#define PLL_CLKSEL_GPENC2_480          (0x02 << (PLL_CLKSEL_GPENC2 - PLL_CLKSEL_R4_OFFSET)) //< Select GPENC2 clock as 480Mhz
//@}


/*
    @name   UVCP2 clock rate value

    UVCP2 clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_UVCP2).
*/
//@{
#define PLL_CLKSEL_UVCP2_240     (0x00 << (PLL_CLKSEL_UVCP2 - PLL_CLKSEL_R4_OFFSET))  //< Select UVCP2 clock as 240Mhz
#define PLL_CLKSEL_UVCP2_320     (0x01 << (PLL_CLKSEL_UVCP2 - PLL_CLKSEL_R4_OFFSET))  //< Select UVCP2 clock as 320Mhz
#define PLL_CLKSEL_UVCP2_120     (0x02 << (PLL_CLKSEL_UVCP2 - PLL_CLKSEL_R4_OFFSET))  //< Select UVCP2 clock as 120Mhz
//@}


/*
    @name   HWCOPY clock rate value

    HWCOPY clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_HWCOPY).
*/
//@{
#define PLL_CLKSEL_HWCOPY_240     (0x00 << (PLL_CLKSEL_HWCOPY - PLL_CLKSEL_R4_OFFSET))  //< Select HWCOPY clock as 240Mhz
#define PLL_CLKSEL_HWCOPY_PLL2    (0x01 << (PLL_CLKSEL_HWCOPY - PLL_CLKSEL_R4_OFFSET))  //< Select HWCOPY clock as PLL2
//@}


/*
    @name   PMC clock rate value

    PMC clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_PMC).
*/
//@{
#define PLL_CLKSEL_PMC_120     (0x00 << (PLL_CLKSEL_PMC - PLL_CLKSEL_R4_OFFSET))  //< Select PMC clock as 240Mhz
#define PLL_CLKSEL_PMC_RTC     (0x01 << (PLL_CLKSEL_PMC - PLL_CLKSEL_R4_OFFSET))  //< Select PMC clock as RTC
#define PLL_CLKSEL_PMC_12      (0x02 << (PLL_CLKSEL_PMC - PLL_CLKSEL_R4_OFFSET))  //< Select PMC clock as 12Mhz
//@}

/*
    @name   DSI LP clock source value

    DSI LP clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_DSI_LPSRC).
*/
//@{
#define PLL_CLKSEL_DSI_LPSRC_60   (0x00 << (PLL_CLKSEL_DSI_LPSRC - PLL_CLKSEL_R4_OFFSET))    //< Select DSI LP clock as 60MHz
#define PLL_CLKSEL_DSI_LPSRC_80   (0x01 << (PLL_CLKSEL_DSI_LPSRC - PLL_CLKSEL_R4_OFFSET))    //< Select DSI LP clock as 80MHz
//@}


/*
    @name   CSI TX LP clock source value

    CSI TX LP clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_CSI_TX_LPSRC).
*/
//@{
#define PLL_CLKSEL_CSI_TX_LPSRC_60   (0x00 << (PLL_CLKSEL_CSI_TX_LPSRC - PLL_CLKSEL_R4_OFFSET))    //< Select CSI TX LP clock as 60MHz
#define PLL_CLKSEL_CSI_TX_LPSRC_80   (0x01 << (PLL_CLKSEL_CSI_TX_LPSRC - PLL_CLKSEL_R4_OFFSET))    //< Select CSI TX LP clock as 80MHz
//@}


/*
    @name   MI clock rate value

    MI clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_MI).
*/
//@{
#define PLL_CLKSEL_MI_192     (0x00 << (PLL_CLKSEL_MI - PLL_CLKSEL_R4_OFFSET))  //< Select MI clock as 192Mhz
#define PLL_CLKSEL_MI_240     (0x01 << (PLL_CLKSEL_MI - PLL_CLKSEL_R4_OFFSET))  //< Select MI clock as 240Mhz
//@}


/*
    @name   Ethernet2 PHY Reference clock rate value

    Ethernet2 PHY Reference clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_ETH2PHY_CLKSRC).
*/
//@{
#define PLL_CLKSEL_ETH2PHY_CLKSRC_PLL7          (0x00 << (PLL_CLKSEL_ETH2PHY_CLKSRC - PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_ETH2PHY_CLKSRC_SPCLK         (0x01 << (PLL_CLKSEL_ETH2PHY_CLKSRC - PLL_CLKSEL_R4_OFFSET))
#define PLL_CLKSEL_ETH2PHY_CLKSRC_SP2CLK        (0x02 << (PLL_CLKSEL_ETH2PHY_CLKSRC - PLL_CLKSEL_R4_OFFSET))
//@}


//////////////////////////////////// R4 select done.




/*
    @name   HASH clock rate value

    HASH clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_HASH).
*/
//@{
#define PLL_CLKSEL_HASH_PLL2   (0x00 << (PLL_CLKSEL_HASH - PLL_CLKSEL_R5_OFFSET))    //< Select Crypto clock as 400Mhz
#define PLL_CLKSEL_HASH_PLL13  (0x01 << (PLL_CLKSEL_HASH - PLL_CLKSEL_R5_OFFSET))    //< Select Crypto clock as 350Mhz
//@}

/*
    @name   RSA clock rate value

    RSA clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_RSA).
*/
//@{
#define PLL_CLKSEL_RSA_PLL2   (0x00 << (PLL_CLKSEL_RSA - PLL_CLKSEL_R5_OFFSET))    //< Select Crypto clock as 400Mhz
#define PLL_CLKSEL_RSA_PLL13  (0x01 << (PLL_CLKSEL_RSA - PLL_CLKSEL_R5_OFFSET))    //< Select Crypto clock as 350Mhz
//@}

/*
    @name   Crypto clock rate value

    Crypto clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_CRYPTO).
*/
//@{
#define PLL_CLKSEL_CRYPTO_PLL2   (0x00 << (PLL_CLKSEL_CRYPTO - PLL_CLKSEL_R5_OFFSET))    //< Select Crypto clock as 400Mhz
#define PLL_CLKSEL_CRYPTO_PLL13  (0x01 << (PLL_CLKSEL_CRYPTO - PLL_CLKSEL_R5_OFFSET))    //< Select Crypto clock as 350Mhz
//@}


/*
    @name   ADC PowerDown clock rate value

    DC PowerDown clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_ADC_PD).
*/
//@{
#define PLL_CLKSEL_ADC_PD_RTC         (0x00 << (PLL_CLKSEL_ADC_PD - PLL_CLKSEL_R5_OFFSET)) //< Select ADC_PD clock source from RTC 32.768 KHz
#define PLL_CLKSEL_ADC_PD_OSC         (0x01 << (PLL_CLKSEL_ADC_PD - PLL_CLKSEL_R5_OFFSET)) //< Select ADC_PD clock source from OSC 12 MHz generated 32 KHz
//@}


/*
    @name   TRNG clock rate value

    TRNG clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_TRNG).
*/
//@{
#define PLL_CLKSEL_TRNG_160         (0x00 << (PLL_CLKSEL_TRNG - PLL_CLKSEL_R5_OFFSET)) //< Select TRNG clock as 160Mhz
#define PLL_CLKSEL_TRNG_240         (0x01 << (PLL_CLKSEL_TRNG - PLL_CLKSEL_R5_OFFSET)) //< Select TRNG clock as 240Mhz
//@}


/*
    @name   TRNG RO clock source value

    TRNG RO clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_TRNGRO_CLKSRC).
*/
//@{
#define PLL_CLKSEL_TRNGRO_CLKSRC_ROSC     (0x00 << (PLL_CLKSEL_TRNGRO_CLKSRC - PLL_CLKSEL_R5_OFFSET)) //< Select TRNG ROSC clock from Ring OSC macro
#define PLL_CLKSEL_TRNGRO_CLKSRC_PLL11    (0x01 << (PLL_CLKSEL_TRNGRO_CLKSRC - PLL_CLKSEL_R5_OFFSET)) //< Select TRNG ROSC clock from PLL11
//@}


/*
    @name   DRTC clock rate value

    DRTC clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_DRTC).
*/
//@{
#define PLL_CLKSEL_DRTC_OSC         (0x00 << (PLL_CLKSEL_DRTC - PLL_CLKSEL_R5_OFFSET)) //< Select DRTC clock source from OSC 10/12 MHz
#define PLL_CLKSEL_DRTC_RTC         (0x01 << (PLL_CLKSEL_DRTC - PLL_CLKSEL_R5_OFFSET)) //< Select DRTC clock source from RTC 32.768 KHz
//@}


/*
    @name   REMOTE clock rate value

    REMOTE clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_REMOTE).
*/
//@{
#define PLL_CLKSEL_REMOTE_RTC         (0x00 << (PLL_CLKSEL_REMOTE - PLL_CLKSEL_R5_OFFSET)) //< Select REMOTE clock source from RTC 32.768 KHz
#define PLL_CLKSEL_REMOTE_OSC         (0x01 << (PLL_CLKSEL_REMOTE - PLL_CLKSEL_R5_OFFSET)) //< Select REMOTE clock source from OSC 12 MHz generated 32 KHz
#define PLL_CLKSEL_REMOTE_IOCLK       (0x02 << (PLL_CLKSEL_REMOTE - PLL_CLKSEL_R5_OFFSET)) //< Select REMOTE clock source from rmt_io_clk / P_GPIO_9
#define PLL_CLKSEL_REMOTE_3           (0x03 << (PLL_CLKSEL_REMOTE - PLL_CLKSEL_R5_OFFSET)) //< Select REMOTE clock source from 3 MHz
//@}


/*
    @name   Graphic clock rate value

    Graphic clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_GRAPHIC).
*/
//@{
#define PLL_CLKSEL_GRAPHIC_240      (0x00 << (PLL_CLKSEL_GRAPHIC - PLL_CLKSEL_R5_OFFSET)) //< Select Graphic clock as 240Mhz
#define PLL_CLKSEL_GRAPHIC_320      (0x01 << (PLL_CLKSEL_GRAPHIC - PLL_CLKSEL_R5_OFFSET)) //< Select Graphic clock as 320Mhz
#define PLL_CLKSEL_GRAPHIC_480      (0x02 << (PLL_CLKSEL_GRAPHIC - PLL_CLKSEL_R5_OFFSET)) //< Select Graphic clock as 480Mhz
#define PLL_CLKSEL_GRAPHIC_PLL18    (0x03 << (PLL_CLKSEL_GRAPHIC - PLL_CLKSEL_R5_OFFSET)) //< Select Graphic clock as PLL18
//@}

/*
    @name   IDE clock source value

    IDE clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_IDE_CLKSRC).
*/
//@{
#define PLL_CLKSEL_IDE_CLKSRC_480    (0x00 << (PLL_CLKSEL_IDE_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE clock source as 480 MHz
#define PLL_CLKSEL_IDE_CLKSRC_PLL9   (0x01 << (PLL_CLKSEL_IDE_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE clock source as PLL9 (for IDE)
#define PLL_CLKSEL_IDE_CLKSRC_192    (0x02 << (PLL_CLKSEL_IDE_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE clock source as 192MHz
#define PLL_CLKSEL_IDE_CLKSRC_PLL10  (0x03 << (PLL_CLKSEL_IDE_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE clock source as PLL10 (for IDE2)
#define PLL_CLKSEL_IDE_CLKSRC_PLL17  (0x04 << (PLL_CLKSEL_IDE_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE clock source as PLL17 (for DSI)
#define PLL_CLKSEL_IDE_CLKSRC_PLL11  (0x05 << (PLL_CLKSEL_IDE_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE clock source as PLL11 (for SIE_MLCK)
//@}


/*
    @name   IDE2 clock source value

    IDE2 clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_IDE2_CLKSRC).
*/
//@{
#define PLL_CLKSEL_IDE2_CLKSRC_480    (0x00 << (PLL_CLKSEL_IDE2_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE2 clock source as 480 MHz
#define PLL_CLKSEL_IDE2_CLKSRC_PLL9   (0x01 << (PLL_CLKSEL_IDE2_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE2 clock source as PLL9 (for IDE)
#define PLL_CLKSEL_IDE2_CLKSRC_192    (0x02 << (PLL_CLKSEL_IDE2_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE2 clock source as 192MHz
#define PLL_CLKSEL_IDE2_CLKSRC_PLL10  (0x03 << (PLL_CLKSEL_IDE2_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE2 clock source as PLL10 (for IDE2)
#define PLL_CLKSEL_IDE2_CLKSRC_PLL17  (0x04 << (PLL_CLKSEL_IDE2_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE2 clock source as PLL17 (for DSI)
#define PLL_CLKSEL_IDE2_CLKSRC_PLL11  (0x05 << (PLL_CLKSEL_IDE2_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE2 clock source as PLL11 (for SIE_MLCK)
//@}

/*
    @name   Graphic2 clock rate value

    Graphic2 clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_GRAPHIC2).
*/
//@{
#define PLL_CLKSEL_GRAPHIC2_240      (0x00 << (PLL_CLKSEL_GRAPHIC2 - PLL_CLKSEL_R5_OFFSET)) //< Select Graphic2 clock as 240Mhz
#define PLL_CLKSEL_GRAPHIC2_320      (0x01 << (PLL_CLKSEL_GRAPHIC2 - PLL_CLKSEL_R5_OFFSET)) //< Select Graphic2 clock as 320Mhz
#define PLL_CLKSEL_GRAPHIC2_480      (0x02 << (PLL_CLKSEL_GRAPHIC2 - PLL_CLKSEL_R5_OFFSET)) //< Select Graphic2 clock as 480Mhz
#define PLL_CLKSEL_GRAPHIC2_PLL18    (0x03 << (PLL_CLKSEL_GRAPHIC2 - PLL_CLKSEL_R5_OFFSET)) //< Select Graphic2 clock as PLL18
//@}

/*
    @name   Graphic3 clock rate value

    Graphic3 clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_GRAPHIC3).
*/
//@{
#define PLL_CLKSEL_GRAPHIC3_240      (0x00 << (PLL_CLKSEL_GRAPHIC3 - PLL_CLKSEL_R5_OFFSET)) //< Select Graphic3 clock as 240Mhz
#define PLL_CLKSEL_GRAPHIC3_320      (0x01 << (PLL_CLKSEL_GRAPHIC3 - PLL_CLKSEL_R5_OFFSET)) //< Select Graphic3 clock as 320Mhz
#define PLL_CLKSEL_GRAPHIC3_480      (0x02 << (PLL_CLKSEL_GRAPHIC3 - PLL_CLKSEL_R5_OFFSET)) //< Select Graphic3 clock as 480Mhz
#define PLL_CLKSEL_GRAPHIC3_PLL18    (0x03 << (PLL_CLKSEL_GRAPHIC3 - PLL_CLKSEL_R5_OFFSET)) //< Select Graphic3 clock as PLL18
//@}



//////////////////////////////////// R5 select done.

/*
    @name CSI clock rate value

    CSI clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_CSI).
*/
//@{
#define PLL_CLKSEL_CSI_60  (0x00 << (PLL_CLKSEL_CSI - PLL_CLKSEL_R6_OFFSET))     //< Select CSI clock as 60Mhz
#define PLL_CLKSEL_CSI_120 (0x01 << (PLL_CLKSEL_CSI - PLL_CLKSEL_R6_OFFSET))     //< Select CSI clock as 120Mhz
#define PLL_CLKSEL_CSI_240 (0x02 << (PLL_CLKSEL_CSI - PLL_CLKSEL_R6_OFFSET))     //< Select CSI clock as 240Mhz

//@}

/*
    @name CSI2 clock rate value

    CSI2 clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_CSI2).
*/
//@{
#define PLL_CLKSEL_CSI2_60  (0x00 << (PLL_CLKSEL_CSI2 - PLL_CLKSEL_R6_OFFSET))     //< Select CSI2 clock as 60Mhz
#define PLL_CLKSEL_CSI2_120 (0x01 << (PLL_CLKSEL_CSI2 - PLL_CLKSEL_R6_OFFSET))     //< Select CSI2 clock as 120Mhz
#define PLL_CLKSEL_CSI2_240 (0x02 << (PLL_CLKSEL_CSI2 - PLL_CLKSEL_R6_OFFSET))     //< Select CSI2 clock as 240Mhz
//@}

/*
    @name CSI3 clock rate value

    CSI3 clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_CSI3).
*/
//@{
#define PLL_CLKSEL_CSI3_60  (0x00 << (PLL_CLKSEL_CSI3 - PLL_CLKSEL_R6_OFFSET))     //< Select CSI3 clock as 60Mhz
#define PLL_CLKSEL_CSI3_120 (0x01 << (PLL_CLKSEL_CSI3 - PLL_CLKSEL_R6_OFFSET))     //< Select CSI3 clock as 120Mhz
#define PLL_CLKSEL_CSI3_240 (0x02 << (PLL_CLKSEL_CSI3 - PLL_CLKSEL_R6_OFFSET))     //< Select CSI3 clock as 240Mhz
//@}

/*
    @name CSI4 clock rate value

    CSI4 clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_CSI4).
*/
//@{
#define PLL_CLKSEL_CSI4_60  (0x00 << (PLL_CLKSEL_CSI4 - PLL_CLKSEL_R6_OFFSET))     //< Select CSI4 clock as 60Mhz
#define PLL_CLKSEL_CSI4_120 (0x01 << (PLL_CLKSEL_CSI4 - PLL_CLKSEL_R6_OFFSET))     //< Select CSI4 clock as 120Mhz
#define PLL_CLKSEL_CSI4_240 (0x02 << (PLL_CLKSEL_CSI4 - PLL_CLKSEL_R6_OFFSET))     //< Select CSI4 clock as 240Mhz
//@}

/*
    @name CSI5 clock rate value

    CSI5 clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_CSI5).
*/
//@{
#define PLL_CLKSEL_CSI5_60  (0x00 << (PLL_CLKSEL_CSI5 - PLL_CLKSEL_R6_OFFSET))     //< Select CSI5 clock as 60Mhz
#define PLL_CLKSEL_CSI5_120 (0x01 << (PLL_CLKSEL_CSI5 - PLL_CLKSEL_R6_OFFSET))     //< Select CSI5 clock as 120Mhz
#define PLL_CLKSEL_CSI5_240 (0x02 << (PLL_CLKSEL_CSI5 - PLL_CLKSEL_R6_OFFSET))     //< Select CSI5 clock as 240Mhz
//@}


/*
    @name CSI2 PXCLK clock rate value

    CSI2 PXCLK clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_CSI2_PXCLK).
*/
//@{
#define PLL_CLKSEL_CSI2_PXCLK_SIE  (0x00 << (PLL_CLKSEL_CSI2_PXCLK - PLL_CLKSEL_R6_OFFSET))     //< Select CSI2 PXCLK clock as SIE
#define PLL_CLKSEL_CSI2_PXCLK_VIE  (0x01 << (PLL_CLKSEL_CSI2_PXCLK - PLL_CLKSEL_R6_OFFSET))     //< Select CSI2 PXCLK clock as VIE
//@}



/*
    @name CSI4 PXCLK clock rate value

    CSI4 PXCLK clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_CSI4_PXCLK).
*/
//@{
#define PLL_CLKSEL_CSI4_PXCLK_SIE   (0x00 << (PLL_CLKSEL_CSI4_PXCLK - PLL_CLKSEL_R6_OFFSET))     //< Select CSI4 PXCLK clock as SIE
#define PLL_CLKSEL_CSI4_PXCLK_VIE2  (0x01 << (PLL_CLKSEL_CSI4_PXCLK - PLL_CLKSEL_R6_OFFSET))     //< Select CSI4 PXCLK clock as VIE2
//@}


//////////////////////////////////// R6 select done.



/*
    @name   CSIPHY Clock Phase Select

    CSIPHY CK0 to CSI CK0 Clock Phase Select

    @note This is for pll_set_clock_rate(PLL_CLKSEL_CSIPHY_CSI_CK0_PHASE).
*/
//@{
#define PLL_CSIPHY_CSI_CK0_PHASE_NORMAL   (0x00 << (PLL_CLKSEL_CSIPHY_CSI_CK0_PHASE - PLL_CLKSEL_R7_OFFSET))    //< Select CSIPHY Clock Phase as Normal (non-invert)
#define PLL_CSIPHY_CSI_CK0_PHASE_INVERT   (0x01 << (PLL_CLKSEL_CSIPHY_CSI_CK0_PHASE - PLL_CLKSEL_R7_OFFSET))    //< Select CSIPHY Clock Phase as Invert
//@}


/*
    @name   CSIPHY2 Clock Phase Select

    CSIPHY2 CK0 to CSI2 CK0 Clock Phase Select

    @note This is for pll_set_clock_rate(PLL_CLKSEL_CSIPHY2_CSI2_CK0_PHASE).
*/
//@{
#define PLL_CSIPHY2_CSI2_CK0_PHASE_NORMAL   (0x00 << (PLL_CLKSEL_CSIPHY2_CSI2_CK0_PHASE - PLL_CLKSEL_R7_OFFSET))    //< Select CSIPHY2 Clock Phase as Normal (non-invert)
#define PLL_CSIPHY2_CSI2_CK0_PHASE_INVERT   (0x01 << (PLL_CLKSEL_CSIPHY2_CSI2_CK0_PHASE - PLL_CLKSEL_R7_OFFSET))    //< Select CSIPHY2 Clock Phase as Invert
//@}

/*
    @name   CSIPHY2 Clock Phase Select

    CSIPHY2 CK0 to CSI2 CK1 Clock Phase Select

    @note This is for pll_set_clock_rate(PLL_CLKSEL_CSIPHY2_CSI2_CK1_PHASE).
*/
//@{
#define PLL_CSIPHY2_CSI2_CK1_PHASE_NORMAL   (0x00 << (PLL_CLKSEL_CSIPHY2_CSI2_CK1_PHASE - PLL_CLKSEL_R7_OFFSET))    //< Select CSIPHY2 Clock Phase as Normal (non-invert)
#define PLL_CSIPHY2_CSI2_CK1_PHASE_INVERT   (0x01 << (PLL_CLKSEL_CSIPHY2_CSI2_CK1_PHASE - PLL_CLKSEL_R7_OFFSET))    //< Select CSIPHY2 Clock Phase as Invert
//@}

/*
    @name   CSIPHY2 Clock Phase Select

    CSIPHY2 CK0 to CSI3 CK1 Clock Phase Select

    @note This is for pll_set_clock_rate(PLL_CLKSEL_CSIPHY2_CSI3_CK1_PHASE).
*/
//@{
#define PLL_CSIPHY2_CSI3_CK1_PHASE_NORMAL   (0x00 << (PLL_CLKSEL_CSIPHY2_CSI3_CK1_PHASE - PLL_CLKSEL_R7_OFFSET))    //< Select CSIPHY2 Clock Phase as Normal (non-invert)
#define PLL_CSIPHY2_CSI3_CK1_PHASE_INVERT   (0x01 << (PLL_CLKSEL_CSIPHY2_CSI3_CK1_PHASE - PLL_CLKSEL_R7_OFFSET))    //< Select CSIPHY2 Clock Phase as Invert
//@}


/*
    @name   CSIPHY3 Clock Phase Select

    CSIPHY3 CK0 to CSI4 CK0 Clock Phase Select

    @note This is for pll_set_clock_rate(PLL_CLKSEL_CSIPHY3_CSI4_CK0_PHASE).
*/
//@{
#define PLL_CSIPHY3_CSI4_CK0_PHASE_NORMAL   (0x00 << (PLL_CLKSEL_CSIPHY3_CSI4_CK0_PHASE - PLL_CLKSEL_R7_OFFSET))    //< Select CSIPHY3 Clock Phase as Normal (non-invert)
#define PLL_CSIPHY3_CSI4_CK0_PHASE_INVERT   (0x01 << (PLL_CLKSEL_CSIPHY3_CSI4_CK0_PHASE - PLL_CLKSEL_R7_OFFSET))    //< Select CSIPHY3 Clock Phase as Invert
//@}

/*
    @name   CSIPHY3 Clock Phase Select

    CSIPHY3 CK0 to CSI4 CK1 Clock Phase Select

    @note This is for pll_set_clock_rate(PLL_CLKSEL_CSIPHY3_CSI4_CK1_PHASE).
*/
//@{
#define PLL_CSIPHY3_CSI4_CK1_PHASE_NORMAL   (0x00 << (PLL_CLKSEL_CSIPHY3_CSI4_CK1_PHASE - PLL_CLKSEL_R7_OFFSET))    //< Select CSIPHY3 Clock Phase as Normal (non-invert)
#define PLL_CSIPHY3_CSI4_CK1_PHASE_INVERT   (0x01 << (PLL_CLKSEL_CSIPHY3_CSI4_CK1_PHASE - PLL_CLKSEL_R7_OFFSET))    //< Select CSIPHY3 Clock Phase as Invert
//@}

/*
    @name   CSIPHY3 Clock Phase Select

    CSIPHY3 CK0 to CSI5 CK1 Clock Phase Select

    @note This is for pll_set_clock_rate(PLL_CLKSEL_CSIPHY3_CSI5_CK1_PHASE).
*/
//@{
#define PLL_CSIPHY3_CSI5_CK1_PHASE_NORMAL   (0x00 << (PLL_CLKSEL_CSIPHY3_CSI5_CK1_PHASE - PLL_CLKSEL_R7_OFFSET))    //< Select CSIPHY3 Clock Phase as Normal (non-invert)
#define PLL_CSIPHY3_CSI5_CK1_PHASE_INVERT   (0x01 << (PLL_CLKSEL_CSIPHY3_CSI5_CK1_PHASE - PLL_CLKSEL_R7_OFFSET))    //< Select CSIPHY3 Clock Phase as Invert
//@}


/*
    @name DAI  clock rate value

    DAI  clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_DAI).
*/
//@{
#define PLL_CLKSEL_DAI_PLL12   (0x00 << (PLL_CLKSEL_DAI - PLL_CLKSEL_R7_OFFSET))     //< Select DAI clock as PLL12
#define PLL_CLKSEL_DAI_IO      (0x01 << (PLL_CLKSEL_DAI - PLL_CLKSEL_R7_OFFSET))     //< Select DAI clock as IO PAD
//@}


/*
    @name I2S MCLK clock rate value

    I2S MCLK  clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_I2S_MCLK).
*/
//@{
#define PLL_CLKSEL_I2S_MCLK_256FS      (0x00 << (PLL_CLKSEL_I2S_MCLK - PLL_CLKSEL_R7_OFFSET))     //< Select  I2S MCLK  clock as 256*FS
#define PLL_CLKSEL_I2S_MCLK_PLL12      (0x01 << (PLL_CLKSEL_I2S_MCLK - PLL_CLKSEL_R7_OFFSET))     //< Select  I2S MCLK  clock as PLL12
//@}


/*
    @name I2S2 MCLK clock rate value

    I2S2 MCLK  clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_I2S2_MCLK).
*/
//@{
#define PLL_CLKSEL_I2S2_MCLK_256FS     (0x00 << (PLL_CLKSEL_I2S2_MCLK - PLL_CLKSEL_R7_OFFSET))     //< Select  I2S2 MCLK  clock as 256*FS
#define PLL_CLKSEL_I2S2_MCLK_PLL12     (0x01 << (PLL_CLKSEL_I2S2_MCLK - PLL_CLKSEL_R7_OFFSET))     //< Select  I2S2 MCLK  clock as PLL12
//@}


/*
    @name   TSE clock rate value

    TSE clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_TSMUX).
*/
//@{
#define PLL_CLKSEL_TSE_240          (0x00 << (PLL_CLKSEL_TSE - PLL_CLKSEL_R7_OFFSET))  //< Select TSE clock as 240Mhz
#define PLL_CLKSEL_TSE_320          (0x02 << (PLL_CLKSEL_TSE - PLL_CLKSEL_R7_OFFSET))  //< Select TSE clock as 320Mhz
#define PLL_CLKSEL_TSE_PLL2         (0x01 << (PLL_CLKSEL_TSE - PLL_CLKSEL_R7_OFFSET))  //< Select TSE clock as PLL2
//@}


//////////////////////////////////// R7 select done.


/*
    @name   SIE CLK clock source value

    SIE CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE_CLKSRC).
*/
//@{
#define PLL_CLKSEL_SIE_CLKSRC_480       (0x00 << (PLL_CLKSEL_SIE_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE CLK clock source as 480 MHz
#define PLL_CLKSEL_SIE_CLKSRC_192       (0x01 << (PLL_CLKSEL_SIE_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE CLK clock source as 192 MHz
#define PLL_CLKSEL_SIE_CLKSRC_320       (0x02 << (PLL_CLKSEL_SIE_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE CLK clock source as 320Mhz
#define PLL_CLKSEL_SIE_CLKSRC_PLL19     (0x03 << (PLL_CLKSEL_SIE_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE CLK clock source as PLL19 Max 500Mhz
#define PLL_CLKSEL_SIE_CLKSRC_PLL23     (0x04 << (PLL_CLKSEL_SIE_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE CLK clock source as PLL23 Max 400Mhz
//@}


/*
    @name   SIE2 CLK clock source value

    SIE2 CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE2_CLKSRC).
*/
//@{
#define PLL_CLKSEL_SIE2_CLKSRC_480       (0x00 << (PLL_CLKSEL_SIE2_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE2 CLK clock source as 480 MHz
#define PLL_CLKSEL_SIE2_CLKSRC_192       (0x01 << (PLL_CLKSEL_SIE2_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE2 CLK clock source as 192 MHz
#define PLL_CLKSEL_SIE2_CLKSRC_320       (0x02 << (PLL_CLKSEL_SIE2_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE2 CLK clock source as 320Mhz
#define PLL_CLKSEL_SIE2_CLKSRC_PLL19     (0x03 << (PLL_CLKSEL_SIE2_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE2 CLK clock source as PLL19 Max 500Mhz
#define PLL_CLKSEL_SIE2_CLKSRC_PLL23     (0x04 << (PLL_CLKSEL_SIE2_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE2 CLK clock source as PLL23 Max 400Mhz
//@}


/*
    @name   SIE3 CLK clock source value

    SIE3 CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE3_CLKSRC).
*/
//@{
#define PLL_CLKSEL_SIE3_CLKSRC_480       (0x00 << (PLL_CLKSEL_SIE3_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE3 CLK clock source as 480 MHz
#define PLL_CLKSEL_SIE3_CLKSRC_192       (0x01 << (PLL_CLKSEL_SIE3_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE3 CLK clock source as 192 MHz
#define PLL_CLKSEL_SIE3_CLKSRC_320       (0x02 << (PLL_CLKSEL_SIE3_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE3 CLK clock source as 320Mhz
#define PLL_CLKSEL_SIE3_CLKSRC_PLL19     (0x03 << (PLL_CLKSEL_SIE3_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE3 CLK clock source as PLL19 Max 500Mhz
#define PLL_CLKSEL_SIE3_CLKSRC_PLL23     (0x04 << (PLL_CLKSEL_SIE3_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE3 CLK clock source as PLL23 Max 400Mhz
//@}



/*
    @name   SIE4 CLK clock source value

    SIE4 CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE4_CLKSRC).
*/
//@{
#define PLL_CLKSEL_SIE4_CLKSRC_480       (0x00 << (PLL_CLKSEL_SIE4_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE4 CLK clock source as 480 MHz
#define PLL_CLKSEL_SIE4_CLKSRC_192       (0x01 << (PLL_CLKSEL_SIE4_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE4 CLK clock source as 192 MHz
#define PLL_CLKSEL_SIE4_CLKSRC_320       (0x02 << (PLL_CLKSEL_SIE4_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE4 CLK clock source as 320Mhz
#define PLL_CLKSEL_SIE4_CLKSRC_PLL19     (0x03 << (PLL_CLKSEL_SIE4_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE4 CLK clock source as PLL19 Max 500Mhz
#define PLL_CLKSEL_SIE4_CLKSRC_PLL23     (0x04 << (PLL_CLKSEL_SIE4_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE4 CLK clock source as PLL23 Max 400Mhz
//@}


/*
    @name   SIE5 CLK clock source value

    SIE5 CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE5_CLKSRC).
*/
//@{
#define PLL_CLKSEL_SIE5_CLKSRC_480       (0x00 << (PLL_CLKSEL_SIE5_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE5 CLK clock source as 480 MHz
#define PLL_CLKSEL_SIE5_CLKSRC_192       (0x01 << (PLL_CLKSEL_SIE5_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE5 CLK clock source as 192 MHz
#define PLL_CLKSEL_SIE5_CLKSRC_320       (0x02 << (PLL_CLKSEL_SIE5_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE5 CLK clock source as 320Mhz
#define PLL_CLKSEL_SIE5_CLKSRC_PLL19     (0x03 << (PLL_CLKSEL_SIE5_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE5 CLK clock source as PLL19 Max 500Mhz
#define PLL_CLKSEL_SIE5_CLKSRC_PLL23     (0x04 << (PLL_CLKSEL_SIE5_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE5 CLK clock source as PLL23 Max 400Mhz
//@}


/*
    @name   SIE6 CLK clock source value

    SIE6 CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE6_CLKSRC).
*/
//@{
#define PLL_CLKSEL_SIE6_CLKSRC_480       (0x00 << (PLL_CLKSEL_SIE6_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE6 CLK clock source as 480 MHz
#define PLL_CLKSEL_SIE6_CLKSRC_192       (0x01 << (PLL_CLKSEL_SIE6_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE6 CLK clock source as 192 MHz
#define PLL_CLKSEL_SIE6_CLKSRC_320       (0x02 << (PLL_CLKSEL_SIE6_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE6 CLK clock source as 320Mhz
#define PLL_CLKSEL_SIE6_CLKSRC_PLL19     (0x03 << (PLL_CLKSEL_SIE6_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE6 CLK clock source as PLL19 Max 500Mhz
#define PLL_CLKSEL_SIE6_CLKSRC_PLL23     (0x04 << (PLL_CLKSEL_SIE6_CLKSRC - PLL_CLKSEL_R24_OFFSET))    //< Select SIE6 CLK clock source as PLL23 Max 400Mhz
//@}


/*
    @name   SIE MCLK clock source value

    SIE MCLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLKSRC).
*/
//@{
#define PLL_CLKSEL_SIE_MCLKSRC_480      (0x00 << (PLL_CLKSEL_SIE_MCLKSRC - PLL_CLKSEL_R24_OFFSET))   //< Select SIE MCLK clock source as 480 MHz
#define PLL_CLKSEL_SIE_MCLKSRC_PLL5     (0x01 << (PLL_CLKSEL_SIE_MCLKSRC - PLL_CLKSEL_R24_OFFSET))   //< Select SIE MCLK clock source as PLL5
#define PLL_CLKSEL_SIE_MCLKSRC_PLL11    (0x02 << (PLL_CLKSEL_SIE_MCLKSRC - PLL_CLKSEL_R24_OFFSET))   //< Select SIE MCLK clock source as PLL11
//@}



//////////////////////////////////// R24 select done.

/*
    @name   SIE MCLK2 clock source value

    SIE MCLK2 clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK2SRC).
*/
//@{
#define PLL_CLKSEL_SIE_MCLK2SRC_480     (0x00 << (PLL_CLKSEL_SIE_MCLK2SRC - PLL_CLKSEL_R25_OFFSET)) //< Select SIE MCLK2 clock source as 480 MHz
#define PLL_CLKSEL_SIE_MCLK2SRC_PLL5    (0x01 << (PLL_CLKSEL_SIE_MCLK2SRC - PLL_CLKSEL_R25_OFFSET)) //< Select SIE MCLK2 clock source as PLL5
#define PLL_CLKSEL_SIE_MCLK2SRC_PLL11   (0x02 << (PLL_CLKSEL_SIE_MCLK2SRC - PLL_CLKSEL_R25_OFFSET)) //< Select SIE MCLK2 clock source as PLL11
//@}


/*
    @name   SIE MCLK3 clock source value

    SIE MCLK3 clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK3SRC).
*/
//@{
#define PLL_CLKSEL_SIE_MCLK3SRC_480       (0x00 << (PLL_CLKSEL_SIE_MCLK3SRC - PLL_CLKSEL_R25_OFFSET)) //< Select SIE MCLK3 clock source as 480 MHz
#define PLL_CLKSEL_SIE_MCLK3SRC_PLL5      (0x01 << (PLL_CLKSEL_SIE_MCLK3SRC - PLL_CLKSEL_R25_OFFSET)) //< Select SIE MCLK3 clock source as PLL5
#define PLL_CLKSEL_SIE_MCLK3SRC_PLL11     (0x02 << (PLL_CLKSEL_SIE_MCLK3SRC - PLL_CLKSEL_R25_OFFSET)) //< Select SIE MCLK3 clock source as PLL11
//@}



/*
    @name   SIE MCLK4 clock source value

    SIE MCLK4 clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK4SRC).
*/
//@{
#define PLL_CLKSEL_SIE_MCLK4SRC_480     (0x00 << (PLL_CLKSEL_SIE_MCLK4SRC - PLL_CLKSEL_R25_OFFSET))   //< Select SIE MCLK4 clock source as 480 MHz
#define PLL_CLKSEL_SIE_MCLK4SRC_PLL5    (0x01 << (PLL_CLKSEL_SIE_MCLK4SRC - PLL_CLKSEL_R25_OFFSET))   //< Select SIE MCLK4 clock source as PLL5
#define PLL_CLKSEL_SIE_MCLK4SRC_PLL11   (0x02 << (PLL_CLKSEL_SIE_MCLK4SRC - PLL_CLKSEL_R25_OFFSET))   //< Select SIE MCLK4 clock source as PLL11
//@}



/*
    @name   SIE MCLK5 clock source value

    SIE MCLK5 clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK5SRC).
*/
//@{
#define PLL_CLKSEL_SIE_MCLK5SRC_480     (0x00 << (PLL_CLKSEL_SIE_MCLK5SRC - PLL_CLKSEL_R25_OFFSET))   //< Select SIE MCLK5 clock source as 480 MHz
#define PLL_CLKSEL_SIE_MCLK5SRC_PLL5    (0x01 << (PLL_CLKSEL_SIE_MCLK5SRC - PLL_CLKSEL_R25_OFFSET))   //< Select SIE MCLK5 clock source as PLL5
#define PLL_CLKSEL_SIE_MCLK5SRC_PLL11   (0x02 << (PLL_CLKSEL_SIE_MCLK5SRC - PLL_CLKSEL_R25_OFFSET))   //< Select SIE MCLK5 clock source as PLL11
//@}


/*
    @name   VIE CLK clock source value

    VIE CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_VIE_CLKSRC).
*/
//@{
#define PLL_CLKSEL_VIE_CLKSRC_480       (0x00 << (PLL_CLKSEL_VIE_CLKSRC - PLL_CLKSEL_R25_OFFSET))    //< Select VIE CLK clock source as 480 MHz
#define PLL_CLKSEL_VIE_CLKSRC_192       (0x01 << (PLL_CLKSEL_VIE_CLKSRC - PLL_CLKSEL_R25_OFFSET))    //< Select VIE CLK clock source as 192 MHz
#define PLL_CLKSEL_VIE_CLKSRC_320       (0x02 << (PLL_CLKSEL_VIE_CLKSRC - PLL_CLKSEL_R25_OFFSET))    //< Select VIE CLK clock source as 320Mhz
#define PLL_CLKSEL_VIE_CLKSRC_PLL19     (0x03 << (PLL_CLKSEL_VIE_CLKSRC - PLL_CLKSEL_R25_OFFSET))    //< Select VIE CLK clock source as PLL19 Max 500Mhz
#define PLL_CLKSEL_VIE_CLKSRC_PLL23     (0x04 << (PLL_CLKSEL_VIE_CLKSRC - PLL_CLKSEL_R25_OFFSET))    //< Select VIE CLK clock source as PLL23 Max 400Mhz
//@}


/*
    @name   VIE2 CLK clock source value

    VIE2 CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_VIE2_CLKSRC).
*/
//@{
#define PLL_CLKSEL_VIE2_CLKSRC_480       (0x00 << (PLL_CLKSEL_VIE2_CLKSRC - PLL_CLKSEL_R25_OFFSET))    //< Select VIE2 CLK clock source as 480 MHz
#define PLL_CLKSEL_VIE2_CLKSRC_192       (0x01 << (PLL_CLKSEL_VIE2_CLKSRC - PLL_CLKSEL_R25_OFFSET))    //< Select VIE2 CLK clock source as 192 MHz
#define PLL_CLKSEL_VIE2_CLKSRC_320       (0x02 << (PLL_CLKSEL_VIE2_CLKSRC - PLL_CLKSEL_R25_OFFSET))    //< Select VIE2 CLK clock source as 320Mhz
#define PLL_CLKSEL_VIE2_CLKSRC_PLL19     (0x03 << (PLL_CLKSEL_VIE2_CLKSRC - PLL_CLKSEL_R25_OFFSET))    //< Select VIE2 CLK clock source as PLL19 Max 500Mhz
#define PLL_CLKSEL_VIE2_CLKSRC_PLL23     (0x04 << (PLL_CLKSEL_VIE2_CLKSRC - PLL_CLKSEL_R25_OFFSET))    //< Select VIE2 CLK clock source as PLL23 Max 400Mhz
//@}


//////////////////////////////////// R25 select done.


/*
    @name   SIE3 PXCLK source value

    SIE3 PXCLK source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE3_PXCLKSRC).
*/
//@{
#define PLL_CLKSEL_SIE3_PXCLKSRC_CCIR2_PXCLK (0x00 << (PLL_CLKSEL_SIE3_PXCLKSRC - PLL_CLKSEL_R26_OFFSET)) //< Select SIE3 PX CLK source as PXCLK3 PAD
#define PLL_CLKSEL_SIE3_PXCLKSRC_MCLK        (0x01 << (PLL_CLKSEL_SIE3_PXCLKSRC - PLL_CLKSEL_R26_OFFSET)) //< Select SIE3 PX CLK source as PLL_CLKSEL_SIE_MCLKSRC / (PLL_SIE_MCLKDIV+1)
//@}


/*
    @name   SIE4 PLL CLK clock source value

    SIE4 PLL CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE4_PXCLKSRC).
*/
//@{
#define PLL_CLKSEL_SIE4_PXCLKSRC_PXCLKPAD   (0x00 << (PLL_CLKSEL_SIE4_PXCLKSRC - PLL_CLKSEL_R26_OFFSET)) //< Select SIE4 PX CLK source as PXCLK3 PAD
#define PLL_CLKSEL_SIE4_PXCLKSRC_MCLK       (0x01 << (PLL_CLKSEL_SIE4_PXCLKSRC - PLL_CLKSEL_R26_OFFSET)) //< Select SIE4 PX CLK source as PLL_CLKSEL_SIE_MCLKSRC / (PLL_SIE_MCLKDIV+1)
//@}


/*
    @name   SIE5 PXCLK source value

    SIE5 PXCLK source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE5_PXCLKSRC).
*/
//@{
#define PLL_CLKSEL_SIE5_PXCLKSRC_CCIR2_PXCLK (0x00 << (PLL_CLKSEL_SIE5_PXCLKSRC - PLL_CLKSEL_R26_OFFSET)) //< Select SIE5 PX CLK source as PXCLK5 PAD
#define PLL_CLKSEL_SIE5_PXCLKSRC_MCLK        (0x01 << (PLL_CLKSEL_SIE5_PXCLKSRC - PLL_CLKSEL_R26_OFFSET)) //< Select SIE5 PX CLK source as PLL_CLKSEL_SIE_MCLKSRC / (PLL_SIE_MCLKDIV+1)
//@}


/*
    @name   SIE6 PXCLK source value

    SIE6 PXCLK source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE6_PXCLKSRC).
*/
//@{
#define PLL_CLKSEL_SIE6_PXCLKSRC_CCIR2_PXCLK (0x00 << (PLL_CLKSEL_SIE6_PXCLKSRC - PLL_CLKSEL_R26_OFFSET)) //< Select SIE5 PX CLK source as PXCLK6 PAD
#define PLL_CLKSEL_SIE6_PXCLKSRC_MCLK        (0x01 << (PLL_CLKSEL_SIE6_PXCLKSRC - PLL_CLKSEL_R26_OFFSET)) //< Select SIE5 PX CLK source as PLL_CLKSEL_SIE_MCLKSRC / (PLL_SIE_MCLKDIV+1)
//@}




//////////////////////////////////////////////// start divider


/*
    @name   PWM0-3 clock divider

    PWM0-3 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_PWM0_3_CLKDIV).
*/
//@{
#define PLL_PWM0_3_CLKDIV(x)          ((x) << (PLL_CLKSEL_PWM0_3_CLKDIV - PLL_CLKSEL_R8_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_PWM0_3_CLKDIV)
//@}


/*
    @name   PWM4-7 clock divider

    PWM4-7 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_PWM4_7_CLKDIV).
*/
//@{
#define PLL_PWM4_7_CLKDIV(x)          ((x) << (PLL_CLKSEL_PWM4_7_CLKDIV - PLL_CLKSEL_R8_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_PWM4_7_CLKDIV)
//@}


//////////////////////////////////// R8 select done.

/*
    @name   PWM8 clock divider

    PWM8 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_PWM8_CLKDIV).
*/
//@{
#define PLL_PWM8_CLKDIV(x)          ((x) << (PLL_CLKSEL_PWM8_CLKDIV - PLL_CLKSEL_R9_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_PWM8_CLKDIV)
//@}


/*
    @name   PWM9 clock divider

    PWM9 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_PWM9_CLKDIV).
*/
//@{
#define PLL_PWM9_CLKDIV(x)          ((x) << (PLL_CLKSEL_PWM9_CLKDIV - PLL_CLKSEL_R9_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_PWM9_CLKDIV)
//@}


//////////////////////////////////// R9 select done.

/*
    @name   PWM10 clock divider

    PWM10 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_PWM10_CLKDIV).
*/
//@{
#define PLL_PWM10_CLKDIV(x)          ((x) << (PLL_CLKSEL_PWM10_CLKDIV - PLL_CLKSEL_R10_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_PWM10_CLKDIV)
//@}


/*
    @name   PWM11 clock divider

    PWM11 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_PWM11_CLKDIV).
*/
//@{
#define PLL_PWM11_CLKDIV(x)          ((x) << (PLL_CLKSEL_PWM11_CLKDIV - PLL_CLKSEL_R10_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_PWM11_CLKDIV)
//@}

//////////////////////////////////// R10 select done.

/*
    @name   DAI M clock divider

    DAI M clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_DAI_MCLKDIV)
*/
//@{
#define PLL_DAI_MCLKDIV(x)          ((x) << (PLL_CLKSEL_DAI_MCLKDIV - PLL_CLKSEL_R11_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_DAI_MCLKDIV)
#define PLL_ADO_MCLKDIV PLL_DAI_MCLKDIV
//@}

/*
    @name   DAI2 M clock divider

    DAI2 M clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_DAI2_MCLKDIV)
*/
//@{
#define PLL_DAI2_MCLKDIV(x)          ((x) << (PLL_CLKSEL_DAI2_MCLKDIV - PLL_CLKSEL_R11_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_DAI2_MCLKDIV)
//@}

/*
    @name   SP clock divider

    Special clock divider

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SP_CLKDIV)
*/
//@{
#define PLL_SP_CLKDIV(x)            ((x) << (PLL_CLKSEL_SP_CLKDIV - PLL_CLKSEL_R11_OFFSET))     //< Used for pll_set_clock_rate(PLL_CLKSEL_SP_CLKDIV)
//@}

/*
    @name   SP2 clock divider

    Special clock2 divider

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SP2_CLKDIV)
*/
//@{
#define PLL_SP2_CLKDIV(x)           ((x) << (PLL_CLKSEL_SP2_CLKDIV - PLL_CLKSEL_R11_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_SP2_CLKDIV)
//@}


//////////////////////////////////// R11 select done.

/*
    @name   SDIO clock divider

    SDIO clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SDIO_CLKDIV)
*/
//@{
#define PLL_SDIO_CLKDIV(x)          ((x) << (PLL_CLKSEL_SDIO_CLKDIV - PLL_CLKSEL_R12_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_SDIO_CLKDIV)
//@}

/*
    @name   SDIO2 clock divider

    SDIO2 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SDIO2_CLKDIV)
*/
//@{
#define PLL_SDIO2_CLKDIV(x)         ((x) << (PLL_CLKSEL_SDIO2_CLKDIV - PLL_CLKSEL_R12_OFFSET))  //< This if for pll_set_clock_rate(PLL_CLKSEL_SDIO2_CLKDIV)
//@}

//////////////////////////////////// R12 select done.

/*
    @name   UART clock divider

    UART clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_UART2_CLKDIV)
*/
//@{
#define PLL_UART_CLKDIV(x)          ((x) << (PLL_CLKSEL_UART_CLKDIV - PLL_CLKSEL_R13_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_UART_CLKDIV)
//@}


/*
    @name   UART2 clock divider

    UART2 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_UART2_CLKDIV)
*/
//@{
#define PLL_UART2_CLKDIV(x)          ((x) << (PLL_CLKSEL_UART2_CLKDIV - PLL_CLKSEL_R13_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_UART2_CLKDIV)
//@}

/*
    @name   UART3 clock divider

    UART3 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_UART3_CLKDIV)
*/
//@{
#define PLL_UART3_CLKDIV(x)          ((x) << (PLL_CLKSEL_UART3_CLKDIV - PLL_CLKSEL_R13_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_UART3_CLKDIV)
//@}

/*
    @name   UART4 clock divider

    UART4 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_UART4_CLKDIV)
*/
//@{
#define PLL_UART4_CLKDIV(x)          ((x) << (PLL_CLKSEL_UART4_CLKDIV - PLL_CLKSEL_R13_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_UART4_CLKDIV)
//@}


//////////////////////////////////// R13 select done.

/*
    @name   UART5 clock divider

    UART5 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_UART5_CLKDIV)
*/
//@{
#define PLL_UART5_CLKDIV(x)          ((x) << (PLL_CLKSEL_UART5_CLKDIV - PLL_CLKSEL_R14_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_UART5_CLKDIV)
//@}

/*
    @name   NAND clock divider

    NAND clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_NAND_CLKDIV)
*/
//@{
#define PLL_NAND_CLKDIV(x)          ((x) << (PLL_CLKSEL_NAND_CLKDIV - PLL_CLKSEL_R14_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_NAND_CLKDIV)
//@}


/*
    @name   ETH PTP clock divider

    ETH PTP  clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_ETH_PTP_CLKDIV)
*/
//@{
#define PLL_ETH_PTP_CLKDIV(x)          ((x) << (PLL_CLKSEL_ETH_PTP_CLKDIV - PLL_CLKSEL_R14_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_ETH_PTP_CLKDIV)
//@}

/*
    @name   ETH2 PTP clock divider

    ETH2 PTP  clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_ETH2_PTP_CLKDIV)
*/
//@{
#define PLL_ETH2_PTP_CLKDIV(x)          ((x) << (PLL_CLKSEL_ETH2_PTP_CLKDIV - PLL_CLKSEL_R14_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_ETH2_PTP_CLKDIV)
//@}


//////////////////////////////////// R14 select done.

/*
    @name   IDE clock divider

    IDE clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_IDE_CLKDIV).
*/
//@{
#define PLL_IDE_CLKDIV(x)           ((x) << (PLL_CLKSEL_IDE_CLKDIV - PLL_CLKSEL_R15_OFFSET))     //< Used for pll_set_clock_rate(PLL_CLKSEL_IDE_CLKDIV)
//@}

/*
    @name   IDE Output Interface clock divider

    IDE Output Interface clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_IDE_OUTIF_CLKDIV).
*/
//@{
#define PLL_IDE_OUTIF_CLKDIV(x)          ((x) << (PLL_CLKSEL_IDE_OUTIF_CLKDIV - PLL_CLKSEL_R15_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_IDE_OUTIF_CLKDIV)
//@}

/*
    @name   IDE2 clock divider

    IDE2 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_IDE_CLKDIV).
*/
//@{
#define PLL_IDE2_CLKDIV(x)           ((x) << (PLL_CLKSEL_IDE2_CLKDIV - PLL_CLKSEL_R15_OFFSET))     //< Used for pll_set_clock_rate(PLL_CLKSEL_IDE2_CLKDIV)
//@}

/*
    @name   IDE2 Output Interface clock divider

    IDE2 Output Interface clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_IDE2_OUTIF_CLKDIV).
*/
//@{
#define PLL_IDE2_OUTIF_CLKDIV(x)          ((x) << (PLL_CLKSEL_IDE2_OUTIF_CLKDIV - PLL_CLKSEL_R15_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_IDE2_OUTIF_CLKDIV)
//@}

//////////////////////////////////// R15 select done.

/*
    @name   TRNG RO clock divider

    TRNG RO clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_TRNG_RO_CLKDIV).
*/
//@{
#define PLL_TRNG_RO_CLKDIV(x)          ((x) << (PLL_CLKSEL_TRNG_RO_CLKDIV - PLL_CLKSEL_R16_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_TRNG_RO_CLKDIV)
//@}

/*
    @name   RO PHY clock select

    TRNG RO clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_RO_PHY).
*/
//@{
#define PLL_RO_PHY_SEL_LPRO (0x00 << (PLL_CLKSEL_RO_PHY - PLL_CLKSEL_R16_OFFSET))   //< Select RO PHY clock source as LPRO
#define PLL_RO_PHY_SEL_LNRO (0x01 << (PLL_CLKSEL_RO_PHY - PLL_CLKSEL_R16_OFFSET))   //< Select RO PHY clock source as LNRO
#define PLL_RO_PHY_SEL_PRO  (0x02 << (PLL_CLKSEL_RO_PHY - PLL_CLKSEL_R16_OFFSET))   //< Select RO PHY clock source as PRO
#define PLL_RO_PHY_SEL_NRO  (0x03 << (PLL_CLKSEL_RO_PHY - PLL_CLKSEL_R16_OFFSET))   //< Select RO PHY clock source as NRO
//@}


/*
    @name   RO PHY FREQ CTL

    RO PHY FREQ CTL

    @note This if for pll_set_clock_rate(PLL_CLKSEL_RO_PHY_FREQ).
*/
//@{
#define PLL_RO_PHY_FREQ_SEL_H   (0x00 << (PLL_CLKSEL_RO_PHY_FREQ - PLL_CLKSEL_R16_OFFSET))   //< Select RO PHY FREQ  as High
#define PLL_RO_PHY_FREQ_SEL_L   (0x01 << (PLL_CLKSEL_RO_PHY_FREQ - PLL_CLKSEL_R16_OFFSET))   //< Select RO PHY FREQ  as Low
//@}

/*
    @name   RO 32K clock divider

    RO 32K clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_RO32K_CLKDIV).
*/
//@{
#define PLL_RO32K_CLKDIV(x)          ((x) << (PLL_CLKSEL_RO32K_CLKDIV - PLL_CLKSEL_R16_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_RO32K_CLKDIV)
//@}

/*
    @name   RO 32K div test enable

    TRNG RO delay

    @note This if for pll_set_clock_rate(PLL_CLKSEL_RO32K_DIV_TESTEN).
*/
//@{
#define PLL_RO32K_DIV_TESTEN(x)          ((x) << (PLL_CLKSEL_RO32K_DIV_TESTEN - PLL_CLKSEL_R16_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_RO32K_DIV_TESTEN)
//@}

/*
    @name   TRNG clock divider

    TRNG clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_TRNG_CLKDIV).
*/
//@{
#define PLL_TRNG_CLKDIV(x)          ((x) << (PLL_CLKSEL_TRNG_CLKDIV - PLL_CLKSEL_R16_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_TRNG_CLKDIV)
//@}

//////////////////////////////////// R16 select done.

/*
    @name   DSP IOP clock divider

    DSP IOP clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_DSP_IOP_CLKDIV).
*/
//@{
#define PLL_DSP_IOP_CLKDIV(x)          ((x) << (PLL_CLKSEL_DSP_IOP_CLKDIV - PLL_CLKSEL_R17_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_DSP_IOP_CLKDIV)
//@}


/*
    @name   DSP EDP clock divider

     DSP EDP  clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_DSP_EDP_CLKDIV).
*/
//@{
#define PLL_DSP_EDP_CLKDIV(x)          ((x) << (PLL_CLKSEL_DSP_EDP_CLKDIV - PLL_CLKSEL_R17_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_DSP_EDP_CLKDIV)
//@}

/*
    @name   DSP EPP clock divider

     DSP EPP  clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_DSP_EPP_CLKDIV).
*/
//@{
#define PLL_DSP_EPP_CLKDIV(x)          ((x) << (PLL_CLKSEL_DSP_EPP_CLKDIV - PLL_CLKSEL_R17_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_DSP_EPP_CLKDIV)
//@}

/*
    @name   DSP EDAP clock divider

     DSP EDAP  clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_DSP_EDAP_CLKDIV).
*/
//@{
#define PLL_DSP_EDAP_CLKDIV(x)          ((x) << (PLL_CLKSEL_DSP_EDAP_CLKDIV - PLL_CLKSEL_R17_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_DSP_EDAP_CLKDIV)
//@}


/*
    @name   DSP OCEM clock divider

     DSP OCEM  clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_DSP_OCEM_CLKDIV).
*/
//@{
#define PLL_DSP_OCEM_CLKDIV(x)          ((x) << (PLL_CLKSEL_DSP_OCEM_CLKDIV - PLL_CLKSEL_R17_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_DSP_OCEM_CLKDIV)
//@}


//////////////////////////////////// R17 select done.


/*
    @name   DAI clock divider

    DAI clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_DAI_CLKDIV)
*/
//@{
#define PLL_DAI_CLKDIV(x)          ((x) << (PLL_CLKSEL_DAI_CLKDIV - PLL_CLKSEL_R18_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_DAI_CLKDIV)
#define PLL_ADO_CLKDIV PLL_DAI_CLKDIV
//@}

/*
    @name   DAI2 clock divider

    DAI2 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_DAI2_CLKDIV)
*/
//@{
#define PLL_DAI2_CLKDIV(x)          ((x) << (PLL_CLKSEL_DAI2_CLKDIV - PLL_CLKSEL_R18_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_DAI2_CLKDIV)
//@}



/*
    @name   DAI OSR clock divider

    DAI OSR clock divider

    @note This is for pll_set_clock_rate(PLL_CLKSEL_DAI_OSR_CLKDIV).
*/
//@{
#define PLL_DAI_OSR_CLKDIV(x)           ((x) << (PLL_CLKSEL_DAI_OSR_CLKDIV - PLL_CLKSEL_R18_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_DAI_OSR_CLKDIV)
#define PLL_ADO_OSR_CLKDIV PLL_DAI_OSR_CLKDIV
//@}

//////////////////////////////////// R18 select done.

/*
    @name   SDIO3 clock divider

    SDIO3 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SDIO3_CLKDIV)
*/
//@{
#define PLL_SDIO3_CLKDIV(x)         ((x) << (PLL_CLKSEL_SDIO3_CLKDIV - PLL_CLKSEL_R19_OFFSET))  //< This if for pll_set_clock_rate(PLL_CLKSEL_SDIO3_CLKDIV)
//@}

/*
    @name   MI clock divider

    MI clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_MI_CLKDIV)
*/
//@{
#define PLL_MI_CLKDIV(x)         ((x) << (PLL_CLKSEL_MI_CLKDIV - PLL_CLKSEL_R19_OFFSET))  //< This if for pll_set_clock_rate(PLL_CLKSEL_MI_CLKDIV)
//@}


//////////////////////////////////// R19 select done.

/*
    @name   UART6 clock divider

    UART6 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_UART6_CLKDIV)
*/
//@{
#define PLL_UART6_CLKDIV(x)          ((x) << (PLL_CLKSEL_UART6_CLKDIV - PLL_CLKSEL_R20_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_UART6_CLKDIV)
//@}

/*
    @name   UART7 clock divider

    UART7 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_UART7_CLKDIV)
*/
//@{
#define PLL_UART7_CLKDIV(x)          ((x) << (PLL_CLKSEL_UART7_CLKDIV - PLL_CLKSEL_R20_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_UART7_CLKDIV)
//@}

/*
    @name   UART8 clock divider

    UART8 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_UART8_CLKDIV)
*/
//@{
#define PLL_UART8_CLKDIV(x)          ((x) << (PLL_CLKSEL_UART8_CLKDIV - PLL_CLKSEL_R20_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_UART8_CLKDIV)
//@}


/*
    @name   UART9 clock divider

    UART9 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_UART9_CLKDIV)
*/
//@{
#define PLL_UART9_CLKDIV(x)          ((x) << (PLL_CLKSEL_UART9_CLKDIV - PLL_CLKSEL_R20_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_UART9_CLKDIV)
//@}


//////////////////////////////////// R20 select done.

/*
    @name   SPI clock divider

    SPI clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SPI_CLKDIV)
*/
//@{
#define PLL_SPI_CLKDIV(x)           ((x) << (PLL_CLKSEL_SPI_CLKDIV - PLL_CLKSEL_R21_OFFSET))    //< This if for pll_set_clock_rate(PLL_CLKSEL_SPI_CLKDIV)
//@}

/*
    @name   SPI2 clock divider

    SPI2 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SPI2_CLKDIV)
*/
//@{
#define PLL_SPI2_CLKDIV(x)          ((x) << (PLL_CLKSEL_SPI2_CLKDIV - PLL_CLKSEL_R21_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_SPI2_CLKDIV)
//@}

//////////////////////////////////// R21 select done.


/*
    @name   SPI3 clock divider

    SPI3 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SPI3_CLKDIV)
*/
//@{
#define PLL_SPI3_CLKDIV(x)          ((x) << (PLL_CLKSEL_SPI3_CLKDIV - PLL_CLKSEL_R22_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_SPI3_CLKDIV)
//@}

/*
    @name   SPI4 clock divider

    SPI4 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SPI4_CLKDIV)
*/
//@{
#define PLL_SPI4_CLKDIV(x)          ((x) << (PLL_CLKSEL_SPI4_CLKDIV - PLL_CLKSEL_R22_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_SPI4_CLKDIV)
//@}


//////////////////////////////////// R22 select done.

/*
    @name   SPI5 clock divider

    SPI5 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SPI5_CLKDIV)
*/
//@{
#define PLL_SPI5_CLKDIV(x)          ((x) << (PLL_CLKSEL_SPI5_CLKDIV - PLL_CLKSEL_R23_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_SPI5_CLKDIV)
//@}

//////////////////////////////////// R23 select done.

/*
    @name   SIE CLK clock divider

    SIE CLK clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SIE_CLKDIV).
*/
//@{
#define PLL_SIE_CLKDIV(x)           ((x) << (PLL_CLKSEL_SIE_CLKDIV - PLL_CLKSEL_R27_OFFSET))         //< Used for pll_set_clock_rate(PLL_CLKSEL_SIE_CLKDIV)
//@}


/*
    @name   SIE2 CLK clock divider

    SIE2 CLK clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SIE2_CLKDIV).
*/
//@{
#define PLL_SIE2_CLKDIV(x)          ((x) << (PLL_CLKSEL_SIE2_CLKDIV - PLL_CLKSEL_R27_OFFSET))        //< Used for pll_set_clock_rate(PLL_CLKSEL_SIE2_CLKDIV)
//@}


/*
    @name   SIE3 CLK clock divider

    SIE3 CLK clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SIE3_CLKDIV).
*/
//@{
#define PLL_SIE3_CLKDIV(x)          ((x) << (PLL_CLKSEL_SIE3_CLKDIV - PLL_CLKSEL_R27_OFFSET))        //< Used for pll_set_clock_rate(PLL_CLKSEL_SIE3_CLKDIV)
//@}


/*
    @name   SIE4 CLK clock divider

    SIE4 CLK clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SIE4_CLKDIV).
*/
//@{
#define PLL_SIE4_CLKDIV(x)          ((x) << (PLL_CLKSEL_SIE4_CLKDIV - PLL_CLKSEL_R27_OFFSET))        //< Used for pll_set_clock_rate(PLL_CLKSEL_SIE4_CLKDIV)
//@}

//////////////////////////////////// R27 select done.

/*
    @name   SIE5 CLK clock divider

    SIE5 CLK clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SIE5_CLKDIV).
*/
//@{
#define PLL_SIE5_CLKDIV(x)          ((x) << (PLL_CLKSEL_SIE5_CLKDIV - PLL_CLKSEL_R28_OFFSET))        //< Used for pll_set_clock_rate(PLL_CLKSEL_SIE5_CLKDIV)
//@}


/*
    @name   SIE6 CLK clock divider

    SIE6 CLK clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SIE6_CLKDIV).
*/
//@{
#define PLL_SIE6_CLKDIV(x)          ((x) << (PLL_CLKSEL_SIE6_CLKDIV - PLL_CLKSEL_R28_OFFSET))        //< Used for pll_set_clock_rate(PLL_CLKSEL_SIE6_CLKDIV)
//@}

/*
    @name   SIE MCLK clock divider

    SIE MCLK clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLKDIV).
*/
//@{
#define PLL_SIE_MCLKDIV(x)          ((x) << (PLL_CLKSEL_SIE_MCLKDIV - PLL_CLKSEL_R28_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLKDIV)
//@}

/*
    @name   SIE MCLK2 clock divider

    SIE MCLK2 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK2DIV).
*/
//@{
#define PLL_SIE_MCLK2DIV(x)         ((x) << (PLL_CLKSEL_SIE_MCLK2DIV - PLL_CLKSEL_R28_OFFSET))   //< Used for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK2DIV)
//@}


//////////////////////////////////// R28 select done.

/*
    @name   SIE MCLK3 clock divider

    SIE MCLK3 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK3DIV).
*/
//@{
#define PLL_SIE_MCLK3DIV(x)         ((x) << (PLL_CLKSEL_SIE_MCLK3DIV - PLL_CLKSEL_R29_OFFSET))   //< Used for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK3DIV)
//@}


/*
    @name   SIE MCLK4 clock divider

    SIE MCLK4 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK4DIV).
*/
//@{
#define PLL_SIE_MCLK4DIV(x)         ((x) << (PLL_CLKSEL_SIE_MCLK4DIV - PLL_CLKSEL_R29_OFFSET))   //< Used for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK4DIV)
//@}


/*
    @name   SIE MCLK5 clock divider

    SIE MCLK5 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK5DIV).
*/
//@{
#define PLL_SIE_MCLK5DIV(x)         ((x) << (PLL_CLKSEL_SIE_MCLK5DIV - PLL_CLKSEL_R29_OFFSET))   //< Used for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK5DIV)
//@}


/*
    @name   VIE  clock divider

    VIE  clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_VIE_CLKDIV).
*/
//@{
#define PLL_VIE_CLKDIV(x)         ((x) << (PLL_CLKSEL_VIE_CLKDIV - PLL_CLKSEL_R29_OFFSET))   //< Used for pll_set_clock_rate(PLL_CLKSEL_VIE_CLKDIV)
//@}

//////////////////////////////////// R29 select done.

/*
    @name   VIE2  clock divider

    VIE2  clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_VIE2_CLKDIV).
*/
//@{
#define PLL_VIE2_CLKDIV(x)         ((x) << (PLL_CLKSEL_VIE2_CLKDIV - PLL_CLKSEL_R30_OFFSET))   //< Used for pll_set_clock_rate(PLL_CLKSEL_VIE2_CLKDIV)
//@}



//////////////////////////////////// R30 select done.



















//////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PLL_CLKSEL_IFE_SRAM_CPU         (0x00 << (PLL_CLKSEL_IFE_SRAM)) //< Select IFE SRAM clock from CPU
#define PLL_CLKSEL_IFE_SRAM_IFE         (0x01 << (PLL_CLKSEL_IFE_SRAM)) //< Select IFE SRAM clock from IFE
#define PLL_CLKSEL_CNN_SRAM_CNN         (0x00 << (PLL_CLKSEL_CNN2_SRAM)) //< Select CNN SRAM clock from CNN
#define PLL_CLKSEL_CNN_SRAM_DCE         (0x01 << (PLL_CLKSEL_CNN2_SRAM)) //< Select CNN SRAM clock from DCE



/*
    System Reset ID

    This is for pll_enable_system_reset() and pll_disable_system_reset().
*/
typedef enum {
	AXI_BUS_RSTN            = 0,            //< Reset AXI BUS
	AXI_BUS1_RSTN,          /*1*/           //< Reset AXI BUS1
	AXI_BUS2_RSTN,          /*2*/           //< Reset AXI BUS2
	AXI_BUS3_RSTN,          /*3*/           //< Reset AXI BUS3
	AXI_BUS4_RSTN,          /*4*/           //< Reset AXI BUS4
	AXI_BUS5_RSTN,          /*5*/           //< Reset AXI BUS5
	AXI_BUS6_RSTN,          /*6*/           //< Reset AXI BUS6
	AXI_BUS7_RSTN,          /*7*/           //< Reset AXI BUS7
	DDRPHY_RSTN,            /*8*/           //< Reset DDRPHY
	DDRPHY_HI_RSTN,         /*9*/           //< Reset DDRPHY HI
	JPEG_WRAP_RSTN,         /*10*/          //< Reset JPEG wraper controller
	JPGE2_WRAP_RSTN,        /*11*/          //< Reset JPEG2 wraper controller
	MAU_RSTN,               /*12*/          //< Reset MAU controller
	IME_RSTN,               /*13*/          //< Reset IME controller
	SIE_RSTN,               /*14*/          //< Reset SIE controller
	SIE2_RSTN,              /*15*/          //< Reset SIE2 controller
	SIE3_RSTN,              /*16*/          //< Reset SIE3 controller
	SIE4_RSTN,              /*17*/          //< Reset SIE4 controller
	SIE5_RSTN,              /*18*/          //< Reset SIE5 controller
	SIE6_RSTN,              /*19*/          //< Reset SIE6 controller
	GPENC_RSTN,             /*20*/          //< Reset GPENC controller
	GPENC2_RSTN,            /*21*/          //< Reset GPENC2 controller
	JPEG_RSTN,              /*22*/          //< Reset JPEG controller
	UVCP_RSTN,              /*23*/          //< Reset UVCP controller
	VIE_RSTN,               /*24*/          //< Reset VIE controller
	VIE2_RSTN,              /*25*/          //< Reset VIE2 controller
	DIS_RSTN,               /*26*/          //< Reset DIS controller
	ETH_RSTN,               /*27*/          //< Reset ETH controller
	ISE_RSTN,               /*28*/          //< Reset ISE controller
	HASH_RSTN,              /*29*/          //< Reset HASH controller
	RSA_RSTN,               /*30*/          //< Reset RSA controller
	CRYPTO_RSTN,            /*31*/          //< Reset CRYPTO controller

	VENC_RSTN               = 32,           //< Reset VENC controller
	VTRC_RSTN,              /*32+1*/        //< Reset VTRC controller
	IPE_RSTN,               /*32+2*/        //< Reset IPE controller
	DCE_RSTN,               /*32+3*/        //< Reset DCE controller
	IFE_RSTN,               /*32+4*/        //< Reset IFE controller
	IDE_RSTN,               /*32+5*/        //< Reset IDE controller
	IDE2_RSTN,              /*32+6*/        //< Reset IDE2 controller
	UVCP2_RSTN,             /*32+7*/        //< Reset UVCP2 controller
	MDBC_RSTN,              /*32+8*/        //< Reset MDBC controller
	HWCOPY_RSTN,            /*32+9*/        //< Reset HWCOPY controller
	NAND_RSTN,              /*32+10*/       //< Reset NAND controller
	SDIO_RSTN,              /*32+11*/       //< Reset SDIO controller
	SDIO2_RSTN,             /*32+12*/       //< Reset SDIO2 controller
	SDIO3_RSTN,             /*32+13*/       //< Reset SDIO3 controller
	HDMITX_RSTN,            /*32+14*/       //< Reset HDMITX controller
	NUE_RSTN,               /*32+15*/       //< Reset NUE controller
	NUE2_RSTN,              /*32+16*/       //< Reset NUE2 controller
	TRKE_RSTN,              /*32+17*/       //< Reset TRKE controller
	IVE_RSTN,               /*32+18*/       //< Reset IVE controller
	SATA_RSTN,              /*32+19*/       //< Reset SATA controller
	MI_RSTN,                /*32+20*/       //< Reset MI controller
	JPEG_LITE_RSTN,         /*32+21*/       //< Reset JPEG Lite controller
	JPEG2_RSTN,             /*32+22*/       //< Reset JPEG2 controller
	DSP_RSTN,               /*32+23*/       //< Reset DSP controller
	MAU_APB_RSTN,           /*32+24*/       //< Reset MAU APB controller
	USB2_RSTN,              /*32+25*/       //< Reset USB2 controller
	USB3_RSTN,              /*32+26*/       //< Reset USB3 controller
	TSE_RSTN,               /*32+27*/       //< Reset TSMUX controller
	CNN_RSTN,               /*32+28*/       //< Reset CNN controller
	CNN2_RSTN,              /*32+29*/       //< Reset CNN2 controller
	VPE_RSTN,               /*32+30*/       //< Reset VPE controller
	VPEL_RSTN,              /*32+31*/       //< Reset VPE-Lite controller

	I2C_RSTN                = 64,           //< Reset I2C controller
	I2C2_RSTN,              /*64+1*/        //< Reset I2C2 controller
	I2C3_RSTN,              /*64+2*/        //< Reset I2C3 controller
	I2C4_RSTN,              /*64+3*/        //< Reset I2C4 controller
	I2C5_RSTN,              /*64+4*/        //< Reset I2C5 controller
	I2C6_RSTN,              /*64+5*/        //< Reset I2C6 controller
	I2C7_RSTN,              /*64+6*/        //< Reset I2C7 controller
	I2C8_RSTN,              /*64+7*/        //< Reset I2C8 controller
	I2C9_RSTN,              /*64+8*/        //< Reset I2C9 controller
	I2C10_RSTN,             /*64+9*/        //< Reset I2C10 controller
	I2C11_RSTN,             /*64+10*/       //< Reset I2C11 controller
	TMR_RSTN                = 64 + 12,      //< Reset Timer controller
	TMR2_RSTN,              /*64+13*/       //< Reset Timer2 controller
	TMR3_RSTN,              /*64+14*/       //< Reset Timer3 controller
	RTC_RSTN,               /*64+15*/       //< Reset RTC controller
	SIF_RSTN,               /*64+16*/       //< Reset SIF controller
	GRAPH_RSTN,             /*64+17*/       //< Reset Graphic controller
	GRAPH2_RSTN,            /*64+18*/       //< Reset Graphic2 controller
	GRAPH3_RSTN,            /*64+19*/       //< Reset Graphic3 controller
	UART_RSTN,              /*64+20*/       //< Reset UART controller
	UART2_RSTN,             /*64+21*/       //< Reset UART2 controller
	UART3_RSTN,             /*64+22*/       //< Reset UART3 controller
	UART4_RSTN,             /*64+23*/       //< Reset UART4 controller
	UART5_RSTN,             /*64+24*/       //< Reset UART5 controller
	REMOTE_RSTN,            /*64+25*/       //< Reset Remote controller
	DRTC_RSTN,              /*64+26*/       //< Reset DRTC controller
	ADC_RSTN,               /*64+27*/       //< Reset ADC controller
	ADC_TSEN_RSTN,          /*64+28*/       //< Reset ADC T-sensor controller
	TRNG_RSTN,              /*64+29*/       //< Reset TRNG controller
	CC_RSTN,                /*64+30*/       //< Reset CC controller
	PWBC_RSTN,              /*64+31*/       //< Reset PWBC controller

	GPIO_RSTN               = 96,           //< Reset GPIO controller
	PAD_RSTN,               /*96+1*/        //< Reset PAD controller
	TOP_RSTN,               /*96+2*/        //< Reset TOP controller
	EFUSE_RSTN,             /*96+3*/        //< Reset EFUSE controller
	WDT_RSTN,               /*96+4*/        //< Reset WDT controller
	USB3_PHY_HI_RSTB,       /*96+5*/        //< Reset USB3 PHY
	USB2_PHY_SIDDQ,         /*96+6*/        //< Reset USB2 PHY SIDDQ
	USB2_PHY_POR,           /*96+7*/        //< Reset USB2 PHY POR
	ETH_GLUE_RSTN           = 96 + 9,       //< Reset ETH Glue
	MSI_INTC_RSTN           = 96 + 11,      //< Reset MSI_INTC
	HVYLD_RSTN,             /*96+12*/       //< Reset heavyload controller
	HVYLD2_RSTN,            /*96+13*/       //< Reset heavyload2 controller
	HVYLD3_RSTN,            /*96+14*/       //< Reset heavyload3 controller
	DRE_RSTN,               /*96+15*/       //< Reset DRE controller
	DAI_RSTN,               /*96+16*/       //< Reset DAI controller
	DAI2_RSTN,              /*96+17*/       //< Reset DAI2 controller
	EAC_RSTN,               /*96+18*/       //< Reset EAC controller
	PMC_RSTN                = 96 + 20,      //< Reset PMC controller
	UART6_RSTN,             /*96+21*/       //< Reset UART6 controller
	UART7_RSTN,             /*96+22*/       //< Reset UART7 controller
	UART8_RSTN,             /*96+23*/       //< Reset UART8 controller
	UART9_RSTN,             /*96+24*/       //< Reset UART9 controller
	CSI_RSTN,               /*96+25*/       //< Reset CSI controller
	CSI2_RSTN,              /*96+26*/       //< Reset CSI2 controller
	CSI3_RSTN,              /*96+27*/       //< Reset CSI3 controller
	CSI4_RSTN,              /*96+28*/       //< Reset CSI4 controller
	CSI5_RSTN,              /*96+29*/       //< Reset CSI5 controller
	SDE_RSTN,               /*96+30*/       //< Reset SDE controller
	TGE_RSTN,               /*96+31*/       //< Reset TGE controller

	VDEC_RSTN                = 128,         //< Reset VDEC controller
	JPEG_LITE_WRAP_RSTN,    /*128+1*/       //< Reset JPEG Lite wraper controller
	SENPHY_IF_RSTN           = 128 + 8,     //< Reset senphy interface
	SENPHY2_IF_RSTN,        /*128+9*/       //< Reset senphy2 interface
	SENPHY3_IF_RSTN,        /*128+10*/      //< Reset senphy3 interface
	DSI_RSTN                 = 128 + 16,    //< Reset DSI controller
	CSI_TX_RSTN,            /*128+17*/      //< Reset CSI TX controller
	DSI_PROT_RSTN,          /*128+18*/      //< Reset DSI Protect controller
	SPI_RSTN                 = 128 + 20,    //< Reset SPI controller
	SPI2_RSTN,              /*128+21*/      //< Reset SPI2 controller
	SPI3_RSTN,              /*128+22*/      //< Reset SPI3 controller
	SPI4_RSTN,              /*128+23*/      //< Reset SPI4 controller
	SPI5_RSTN,              /*128+24*/      //< Reset SPI5 controller

	PWM_RSTN                 = 160,         //< Reset PWM controller
	SDP_RSTN                 = 160 + 16,    //< Reset SDP controller


	ENUM_DUMMY4WORD(CG_RSTN)
} CG_RSTN;

typedef enum {
	MIPI_LVDS_RSTN          = 64 + 1,       //< Reset MIPI_LVDS controller
	MIPI_LVDS2_RSTN,        /*64+ 2*/       //< Reset MIPI_LVDS2 controller
} CG_RSTN_BK;

/*
    Power Domain Reset ID

    This is for pll_enable_system_reset() and pll_disable_system_reset().
*/
typedef enum {
	PD3_RSTN                = 3,            //< Reset PD3 (including SIE)
	PD4_RSTN                = 4,            //< Reset PD4 (including CV/CNN)
	PD5_RSTN                = 5,            //< Reset PD5 (including IME/IPE)
	PD6_RSTN                = 6,            //< Reset PD6 (including VENC)
	PD7_RSTN                = 7,            //< Reset PD7 (including DSP)
//	PD8_RSTN             = 8,            //< Reset PD8 (including NONE)
	PD9_RSTN                = 9,            //< Reset PD9 (including VDEC)
	PDx_RSTN_NUM,


	ENUM_DUMMY4WORD(PD_RSTN)
} PD_RSTN;

/**
    Clock frequency select ID

    @note This is for pll_set_clock_freq().
*/
typedef enum {
	SIEMCLK_FREQ,           ///< SIE    MCLK freq Select ID
	SIEMCLK2_FREQ,          ///< SIE    MCLK2 freq Select ID
	SIEMCLK3_FREQ,          ///< SIE    MCLK3 freq Select ID
	SIEMCLK4_FREQ,          ///< SIE    MCLK4 freq Select ID
	SIEMCLK5_FREQ,          ///< SIE    MCLK5 freq Select ID

	SIECLK_FREQ,            ///< SIE    CLK freq Select ID
	SIE2CLK_FREQ,           ///< SIE2   CLK freq Select ID
	SIE3CLK_FREQ,           ///< SIE3   CLK freq Select ID
	SIE4CLK_FREQ,           ///< SIE4   CLK freq Select ID
	SIE5CLK_FREQ,           ///< SIE5   CLK freq Select ID
	SIE6CLK_FREQ,           ///< SIE6   CLK freq Select ID

	VIECLK_FREQ,            ///< VIE    CLK freq Select ID
	VIE2CLK_FREQ,           ///< VIE2   CLK freq Select ID

	IDECLK_FREQ,            ///< IDE    CLK freq Select ID
	IDE2CLK_FREQ,           ///< IDE2    CLK freq Select ID

	SPCLK_FREQ,             ///< SP     CLK Select ID
	SPCLK2_FREQ,            ///< SP2    CLK Select ID

	ADOCLK_FREQ,            ///< ADO    CLK Select ID
	ADOOSRCLK_FREQ,         ///< ADO OSRCLK Select ID

	SDIOCLK_FREQ,           ///< SDIO   CLK Select ID
	SDIO2CLK_FREQ,          ///< SDIO2  CLK Select ID
	SDIO3CLK_FREQ,          ///< SDIO3  CLK Select ID

	SPICLK_FREQ,            ///< SPI    CLK Select ID
	SPI2CLK_FREQ,           ///< SPI2   CLK Select ID
	SPI3CLK_FREQ,           ///< SPI3   CLK Select ID
	SPI4CLK_FREQ,           ///< SPI4   CLK Select ID
	SPI5CLK_FREQ,           ///< SPI5   CLK Select ID

	IDEOUTIFCLK_FREQ,       ///< IDE    output I/F CLK freq Select ID
	IDE2OUTIFCLK_FREQ,      ///< IDE2    output I/F CLK freq Select ID

	CPUCLK_FREQ,            ///< CPU1   CLK Select ID
	APBCLK_FREQ,            ///< APB    CLK Select ID
	TRNGCLK_FREQ,           ///< TRNG   CLK Select ID
	SIEMCLK_12SYNC_FREQ,    ///< SIE    MCLK-1/2 SYNC freq Select ID

	DSICLK_FREQ,            ///< DSI    CLK Select ID

	CNNCLK_FREQ,
	CNN2CLK_FREQ,
	NUECLK_FREQ,
	NUE2CLK_FREQ,

	IMECLK_FREQ,

	IPECLK_FREQ,
	DCECLK_FREQ,
	IFECLK_FREQ,
	VPECLK_FREQ,
	VPELCLK_FREQ,

	SDECLK_FREQ,
	ISECLK_FREQ,
	TRKECLK_FREQ,

	JPEGCLK_FREQ,
	UVCPCLK_FREQ,
	JPEG2CLK_FREQ,
	JPEGLITECLK_FREQ,
	GRPHCLK_FREQ,
	GRPH2CLK_FREQ,
	VENCCLK_FREQ,
	DSPCLK_FREQ,
	H264D_ACLK_FREQ,
	H265D_ACLK_FREQ,
	H264M_ACLK_FREQ,
	H265M_ACLK_FREQ,
	PLL_CLKFREQ_MAXNUM,

	HDMIADOCLK_FREQ,        ///< Backward compatible

	ENUM_DUMMY4WORD(PLL_CLKFREQ)
} PLL_CLKFREQ;


//
//	Exporting APIs
//

extern void     pll_set_clock_rate(PLL_CLKSEL clk_sel, UINT32 uiValue);
extern UINT32   pll_get_clock_rate(PLL_CLKSEL clk_sel);

extern void     pll_enable_clock(CG_EN Num);
extern void     pll_disable_clock(CG_EN Num);
extern ER       pll_set_clock_freq(PLL_CLKFREQ ClkID, UINT32 uiFreq);
extern ER       pll_get_clock_freq(PLL_CLKFREQ ClkID, UINT32 *pFreq);
extern void     pll_enable_system_reset(CG_RSTN Num);
extern void     pll_disable_system_reset(CG_RSTN Num);
/**
    Module (hardware) reset off.

    This fulction will enable module.

    @param[in] num  Reset bit number of type CG_RSTN, only one at a time

    @return void
*/
extern void     pll_enable_power_domain_reset(PD_RSTN num);

/**
    PD (Power domain) reset off.

    This fulction will enable module.

    @param[in] num  Reset bit number of type PD_RSTN, only one at a time

    @return void
*/
extern void     pll_disable_power_domain_reset(PD_RSTN num);
extern UINT32   pll_get_osc_freq(void);
extern ER       pll_set_driver_pll(PLL_ID id, UINT32 ui_setting);

#define pll_setDrvPLL           pll_set_driver_pll
#define pll_setClockRate        pll_set_clock_rate
#define pll_getClockRate        pll_get_clock_rate
#define pll_enableClock         pll_enable_clock
#define pll_disableClock        pll_disable_clock
#define pll_setClockFreq        pll_set_clock_freq
#define pll_getClockFreq        pll_get_clock_freq
#define pll_enableSystemReset   pll_enable_system_reset
#define pll_disableSystemReset  pll_disable_system_reset

//
// For Backward Compatible
//
#define PLL_CLKSEL_SIE_MCLK2SRC_PLL10  0 //< Backward compatible
#define PLL_CLKSEL_SIE_MCLKSRC_PLL10   0 //< Backward compatible
#define PLL_CLKSEL_SIE_MCLK3SRC_PLL10  0 //< Backward compatible
#define PLL_CLKSEL_SIE_MCLKSRC_PLL18   0 //< Backward compatible
#define PLL_CLKSEL_SIE_MCLK3SRC_PLL18  0 //< Backward compatible
#define PLL_CLKSEL_SDIO_PLL4           5 //< Backward compatible
#define PLL_CLKSEL_SDIO2_PLL4          5 //< Backward compatible
#define PLL_CLKSEL_SDIO3_PLL4          5 //< Backward compatible
#define PLL_CLKSEL_TRNGRO_CLKSRC_PLL4  5 //< Backward compatible
#define PLL_CLKSEL_MIPI_LVDS           0
#define PLL_CLKSEL_LVDS_CLKPHASE       0
#define PLL_CLKSEL_SIE_IO_PXCLKSRC_PXCLKPAD    0 //< Backward compatible
#define PLL_CLKSEL_SIE_IO_PXCLKSRC_PXCLKPAD2   0 //< Backward compatible
#define PLL_CLKSEL_SIE2_IO_PXCLKSRC_PXCLKPAD2  0 //< Backward compatible
#define PLL_CLKSEL_SIE2_IO_PXCLKSRC_PXCLKPAD   0 //< Backward compatible
#define PLL_CLKSEL_SIE_PXCLKSRC_PXCLKPAD    0 //< Backward compatible
#define PLL_CLKSEL_SIE_PXCLKSRC_MCLK        0 //< Backward compatible
#define PLL_CLKSEL_SIE2_PXCLKSRC_PXCLKPAD   0 //< Backward compatible
#define PLL_CLKSEL_SIE2_PXCLKSRC_MCLK       0 //< Backward compatible
#define PLL_CLKSEL_SIE4_IO_PXCLKSRC_PXCLKPAD   0 //< Backward compatible
#define PLL_CLKSEL_SIE4_IO_PXCLKSRC_PXCLKPAD2  0 //< Backward compatible
#define PLL_CLKSEL_TGE_PXCLKSRC_PXCLKPAD    0 //< Backward compatible
#define PLL_CLKSEL_TGE_PXCLKSRC_MCLK        0 //< Backward compatible
#define PLL_CLKSEL_TGE2_PXCLKSRC_PXCLKPAD    0 //< Backward compatible
#define PLL_CLKSEL_TGE2_PXCLKSRC_MCLK2      0 //< Backward compatible
#define PLL_CLKSEL_AFFINE_240      0 //< Backward compatible
#define PLL_CLKSEL_AFFINE_320      0 //< Backward compatible
#define PLL_CLKSEL_AFFINE_480      0 //< Backward compatible
#define PLL_CLKSEL_MIPI_LVDS_60  0 //< Backward compatible
#define PLL_CLKSEL_MIPI_LVDS_120 0 //< Backward compatible
#define PLL_LVDS_CLK_PHASE_NORMAL   0 //< Backward compatible
#define PLL_LVDS_CLK_PHASE_INVERT   0 //< Backward compatible
#define PLL_LVDS2_CLK_PHASE_NORMAL   0 //< Backward compatible
#define PLL_LVDS2_CLK_PHASE_INVERT   0 //< Backward compatible
#define PLL_CLKSEL_MIPI_LVDS2 0 //< Backward compatible
#define PLL_CLKSEL_MIPI_LVDS2_60  0 //< Backward compatible
#define PLL_CLKSEL_MIPI_LVDS2_120 0 //< Backward compatible
#define PLL_CLKSEL_ADO_MCLKSEL_256FS   0 //< Backward compatible
#define PLL_CLKSEL_ADO_MCLKSEL_PLL7    0 //< Backward compatible
#define PLL_CLKSEL_ETH_PLL6   0 //< Backward compatible
#define PLL_CLKSEL_ETH_PLL9   0 //< Backward compatible
#define PLL_CLKSEL_ETH_REFCLK_INV_DIS   0 //< Backward compatible
#define PLL_CLKSEL_ETH_REFCLK_INV_EN    0 //< Backward compatible
#define PLL_CLKSEL_IPACLK_CLKSRC_240     0 //< Backward compatible
#define PLL_CLKSEL_IPACLK_CLKSRC_120       0 //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D0_SRC_HSI0 0 //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D0_SRC_HSI1 0 //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D0_SRC_HSI2 0 //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D0_SRC_HSI3 0 //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D1_SRC_HSI0 0 //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D1_SRC_HSI1 0 //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D1_SRC_HSI2 0 //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D1_SRC_HSI3 0 //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D2_SRC_HSI0 0 //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D2_SRC_HSI1 0 //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D2_SRC_HSI2 0 //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D2_SRC_HSI3 0 //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D3_SRC_HSI0 0 //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D3_SRC_HSI1 0 //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D3_SRC_HSI2 0 //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D3_SRC_HSI3 0 //< Backward compatible
#define PLL_TRNG_RO_DELAY 0 //< Backward compatible
#define PLL_CLKSEL_SIE_MCLKSRC_PLL12 0 //< Backward compatible
#define PLL_CLKSEL_SIE_MCLK2SRC_PLL12 0 //< Backward compatible
#define PLL_CLKSEL_SIE_MCLK3SRC_PLL12 0 //< Backward compatible
#define PLL_CLKSEL_TGE2 0 //< Backward compatible
#define PLL_CLKSEL_DCE_192 0 //< Backward compatible
#define PLL_CLKSEL_DCE_96 0 //< Backward compatible
#define PLL_CLKSEL_DCE_PLL12 0 //< Backward compatible
#define PLL_CLKSEL_CNN_SRAM 0 //< Backward compatible
#define PLL_CLKSEL_IPE_192 0 //< Backward compatible
#define PLL_CLKSEL_IPE_96 0 //< Backward compatible
#define PLL_CLKSEL_IPE_PLL12 0 //< Backward compatible
#define PLL_CLKSEL_IME_192 0 //< Backward compatible
#define PLL_CLKSEL_IME_96 0 //< Backward compatible
#define PLL_CLKSEL_IME_PLL12 0 //< Backward compatible
#define PLL_CLKSEL_SIE_MCLKSRC_320 0 //< Backward compatible
#define PLL_CLKSEL_SIE_MCLK2SRC_320 0 //< Backward compatible
#define PLL_CLKSEL_SIE_MCLK3SRC_320 0 //< Backward compatible
#define PLL_CLKSEL_NUE2_240 0 //< Backward compatible
#define PLL_CLKSEL_CNN_240  0 //< Backward compatible
#define PLL_CLKSEL_CNN_320 0 //< Backward compatible
#define PLL_CLKSEL_CNN_PLL12 0 //< Backward compatible
#define PLL_CLKSEL_CNN_PLL9 0 //< Backward compatible
#define PLL_CLKSEL_CNN2_240 0 //< Backward compatible
#define PLL_CLKSEL_CNN2_320 0 //< Backward compatible
#define PLL_CLKSEL_IFE_192 0 //< Backward compatible
#define PLL_CLKSEL_IFE_396 0 //< Backward compatible
#define PLL_CLKSEL_IFE_PLL12 0 //< Backward compatible
#define PLL_CLKSEL_IFE_96 0 //< Backward compatible
#define PLL_CLKSEL_SDIO_PLL9 20 //< Backward compatible
#define PLL_CLKSEL_SDIO2_PLL9 20 //< Backward compatible
#define PLL_CLKSEL_SDIO3_PLL9 20 //< Backward compatible

#define PLL_CLKSEL_RSA_400 20 //< Not Backward compatible
#define PLL_CLKSEL_RSA_350 21 //< Not Backward compatible

#define PLL_CLKSEL_HASH_400 20 //< Not Backward compatible
#define PLL_CLKSEL_HASH_350 21 //< Not Backward compatible

#define PLL_CLKSEL_CRYPTO_400 20 //< Not Backward compatible
#define PLL_CLKSEL_CRYPTO_350 21 //< Not Backward compatible




#if 0
#define PLL_CLKSEL_NAND_60             0 //< Backward compatible
#define H265_CLK                       0 //< Backward compatible
#define SIE4_CLK                       0 //< Backward compatible
#define SIE5_CLK                       0 //< Backward compatible
#define PLL_CLKSEL_SIE_CLKSRC_PLL10    0 //< Backward compatible
#define PLL_CLKSEL_SIE2_CLKSRC_PLL10   0 //< Backward compatible
#define PLL_CLKSEL_SIE3_CLKSRC_PLL10   0 //< Backward compatible
#define PLL_CLKSEL_SIE4_CLKSRC_PLL10   0 //< Backward compatible
#define PLL_CLKSEL_SIE5_CLKSRC_PLL10   0 //< Backward compatible
#define SIE4CLK_FREQ                   0 //< Backward compatible
#define SIE5CLK_FREQ                   0 //< Backward compatible
#define SIE4_PXCLK                     0 //< Backward compatible
#define H265_M_GCLK                    0 //< Backward compatible
#define PLL_CLKSEL_H265                0 //< Backward compatible
#define PLL_CLKSEL_H265_240            0 //< Backward compatible
#define PLL_CLKSEL_H265_320            0 //< Backward compatible
#define PLL_CLKSEL_H265_PLL15          0 //< Backward compatible
#define PLL_CLKSEL_AFFINE_PLL13        5 //< Backward compatible
#define PLL_CLKSEL_ISE_PLL13           0 //< Backward compatible
#define PLL_CLKSEL_GRAPHIC_PLL6        6 //< Backward compatible
#define PLL_CLKSEL_GRAPHIC_PLL13       5 //< Backward compatible
#define PLL_CLKSEL_GRAPHIC2_PLL6       6 //< Backward compatible
#define PLL_CLKSEL_GRAPHIC2_PLL13      5 //< Backward compatible
#define CNN2_M_GCLK                    0 //< Backward compatible
#define SPI4_CLK                       0 //< Backward compatible
#define SPI5_CLK                       0 //< Backward compatible
#define SPI4CLK_FREQ                   0 //< Backward compatible
#define SPI5CLK_FREQ                   0 //< Backward compatible
#define SDE_CLK                        0 //< Backward compatible
#define PLL_CLKSEL_SDE_480             0 //< Backward compatible
#define PLL_CLKSEL_SDE_PLL13           0 //< Backward compatible
#define PLL_CLKSEL_SDE_320             0 //< Backward compatible
#define PLL_CLKSEL_SDE_240             0 //< Backward compatible
#define IFE2_M_GCLK                    0 //< Backward compatible
#define IFE2_GCLK                      0 //< Backward compatible
#define IFE2_CLK                       0 //< Backward compatible
#define IFE2_RSTN                      0 //< Backward compatible
#define PLL_CLKSEL_IFE2_240            0 //< Backward compatible
#define PLL_CLKSEL_IDE_CLKSRC_PLL4     0 //< Backward compatible
#define PLL_CLKSEL_MDBC_PLL13          0 //< Backward compatible
#define PLL_CLKSEL_MDBC_320            0 //< Backward compatible
#define PLL_CLKSEL_DIS_PLL13           0 //< Backward compatible
#define PLL_CLKSEL_DIS_480             0 //< Backward compatible
#define PLL_CLKSEL_DIS_320             0 //< Backward compatible
#define PLL_CLKSEL_IVE_PLL13           5 //< Backward compatible
#define CNN2_CLK                       0 //< Backward compatible
#define PLL_CLKSEL_NUE                 0 //< Backward compatible
#define PLL_CLKSEL_NUE_240             0 //< Backward compatible
#define PLL_CLKSEL_NUE_PLL10           0 //< Backward compatible
#define PLL_CLKSEL_NUE_480             0 //< Backward compatible
#define PLL_CLKSEL_NUE_320             0 //< Backward compatible
#define PLL_CLKSEL_NUE2_PLL13          0 //< Backward compatible
#define PLL_CLKSEL_CNN_PLL10           0 //< Backward compatible
#define PLL_CLKSEL_CNN_480             0 //< Backward compatible
#define NUE_CLK                        0 //< Backward compatible
#define UART4_CLK                      0 //< Backward compatible
#define UART5_CLK                      0 //< Backward compatible
#define UART6_CLK                      0 //< Backward compatible
//@}
#endif


#endif


