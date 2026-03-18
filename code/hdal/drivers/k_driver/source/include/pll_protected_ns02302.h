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
#if defined(__FREERTOS)
#include <kwrap/nvt_type.h>
#include "plat/pll.h"
#else
#include <plat/cg-reg.h>
#include <linux/soc/nvt/nvt_type.h>
#endif

#if defined(_NVT_FPGA_)
#ifndef _FPGA_PLL_OSC_
#define _FPGA_PLL_OSC_  24000000
#endif
#endif


/*
    @addtogroup mIDrvSys_CG
*/
//@{
#define PLL_CLKSEL_R0_OFFSET        0
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

//STBC
#define PLL_CLKSEL_R23_OFFSET       17301504 // (0x21_0010 - 0x10)/4*32
#define PLL_CLKSEL_R24_OFFSET       17301632 // (0x21_0020 - 0x10)/4*32
#define PLL_CLKSEL_R25_OFFSET       17302144 // (0x21_0060 - 0x10)/4*32


#define STBC_PLL_CLKSEL_R0_OFFSET   17301504//  (0x23_0010 - 0x10)/4*32





/*
    Clock select ID

    Clock select ID for pll_set_clock_rate() & pll_get_clock_rate().
*/
typedef enum {

	PLL_CLKSEL_BASE = PLL_CLKSEL_R0_OFFSET,

	//System Clock Rate bit definition 0x10
	PLL_CLKSEL_CPU = 0,                                                    //< Clock Select Module ID: CPU
	PLL_CLKSEL_DDR = 20,                                                   //< Clock Select Module ID: DDR source
	PLL_CLKSEL_DDRPHY = 31,                                                //< Clock Select Module ID: DDR-PHY

	//IPP Clock Rate bit definition 0x14
	PLL_CLKSEL_IVE =                          PLL_CLKSEL_R1_OFFSET + 2,    //< Clock Select Module ID: IVE
	PLL_CLKSEL_IPE =                          PLL_CLKSEL_R1_OFFSET + 4,    //< Clock Select Module ID: IPE
	PLL_CLKSEL_IME =                          PLL_CLKSEL_R1_OFFSET + 12,   //< Clock Select Module ID: IME
	PLL_CLKSEL_MDBC =                         PLL_CLKSEL_R1_OFFSET + 20,   //< Clock Select Module ID: MDBC
	PLL_CLKSEL_VPE =                          PLL_CLKSEL_R1_OFFSET + 22,   //< Clock Select Module ID: VPE
	PLL_CLKSEL_ISE =                          PLL_CLKSEL_R1_OFFSET + 24,   //< Clock Select Module ID: ISE
	PLL_CLKSEL_TGE_MSH =                      PLL_CLKSEL_R1_OFFSET + 28,   //< Clock Select Module ID: TGE_MSH
	PLL_CLKSEL_TGE_FLASH =                    PLL_CLKSEL_R1_OFFSET + 30,   //< Clock Select Module ID: TGE_FLASH

	//IPP Clock Rate 1 bit definition 0x18
	PLL_CLKSEL_IFE =                          PLL_CLKSEL_R2_OFFSET + 0,    //< Clock Select Module ID: IFE
	PLL_CLKSEL_PRE_IFE = PLL_CLKSEL_IFE,
	PLL_CLKSEL_IFE_SRAM =                     PLL_CLKSEL_R2_OFFSET + 2,    //< Clock Select Module ID: IFE_SRAM
	PLL_CLKSEL_PRE_SRAM =                     PLL_CLKSEL_R2_OFFSET + 3,    //< Clock Select Module ID: PRE SRAM
	PLL_CLKSEL_SIE_MCLK4SRC =                 PLL_CLKSEL_R2_OFFSET + 4,    //< Clock Select Module ID: SIE MCLK4
	PLL_CLKSEL_SIE_MCLKSRC =                  PLL_CLKSEL_R2_OFFSET + 8,    //< Clock Select Module ID: SIE MCLK
	PLL_CLKSEL_SIE_MCLK2SRC =                 PLL_CLKSEL_R2_OFFSET + 10,   //< Clock Select Module ID: SIE MCLK2
	PLL_CLKSEL_SIE_MCLK3SRC =                 PLL_CLKSEL_R2_OFFSET + 12,   //< Clock Select Module ID: SIE MCLK3
	PLL_CLKSEL_SIE5_SRAM =                    PLL_CLKSEL_R2_OFFSET + 16,   //< Clock Select Module ID: SIE5 SRAM
	PLL_CLKSEL_SIE_MCLKINV =                  PLL_CLKSEL_R2_OFFSET + 19,   //< Clock Select Module ID: SIE MCLK output invert
	PLL_CLKSEL_SIE_MCLK2INV =                 PLL_CLKSEL_R2_OFFSET + 24,   //< Clock Select Module ID: SIE MLCK2 output invert
	PLL_CLKSEL_SIE_MCLK3INV =                 PLL_CLKSEL_R2_OFFSET + 25,   //< Clock Select Module ID: SIE MLCK3 output invert
	PLL_CLKSEL_SIE_MCLK4INV =                 PLL_CLKSEL_R2_OFFSET + 26,   //< Clock Select Module ID: SIE MLCK4 output invert
	PLL_CLKSEL_NUE2 =                         PLL_CLKSEL_R2_OFFSET + 30,   //< Clock Select Module ID: NUE2

	//CODEC Clock Rate bit definition 0x1C
	PLL_CLKSEL_JPEG =                         PLL_CLKSEL_R3_OFFSET + 0,    //< Clock Select Module ID: JPEG
	PLL_CLKSEL_DRE =                          PLL_CLKSEL_R3_OFFSET + 2,    //< Clock Select Module ID: DRE
	PLL_CLKSEL_VENC =                         PLL_CLKSEL_R3_OFFSET + 4,    //< Clock Select Module ID: VENC
	PLL_CLKSEL_ECDSA =                        PLL_CLKSEL_R3_OFFSET + 6,    //< Clock Select Module ID: ECDSA
	PLL_CLKSEL_GRAPHIC =                      PLL_CLKSEL_R3_OFFSET + 8,    //< Clock Select Module ID: GRAPHIC
	PLL_CLKSEL_GRAPHIC2 =                     PLL_CLKSEL_R3_OFFSET + 12,   //< Clock Select Module ID: GRAPHIC2
	PLL_CLKSEL_GRAPHIC3 =                     PLL_CLKSEL_R3_OFFSET + 14,   //< Clock Select Module ID: GRAPHIC3
	PLL_CLKSEL_SIE3_CLKSRC =                  PLL_CLKSEL_R3_OFFSET + 16,   //< Clock Select Module ID: SIE3
	PLL_CLKSEL_HWCP =                         PLL_CLKSEL_R3_OFFSET + 19,   //< Clock Select Module ID: Hwcopy
	PLL_CLKSEL_CRYPTO =                       PLL_CLKSEL_R3_OFFSET + 20,   //< Clock Select Module ID: CRYPTO
	PLL_CLKSEL_RSA =                          PLL_CLKSEL_R3_OFFSET + 22,   //< Clock Select Module ID: RSA
	PLL_CLKSEL_SIE_CLKSRC =                   PLL_CLKSEL_R3_OFFSET + 24,   //< Clock Select Module ID: SIE
	PLL_CLKSEL_SIE2_CLKSRC =                  PLL_CLKSEL_R3_OFFSET + 28,   //< Clock Select Module ID: SIE2
	PLL_CLKSEL_MI =                           PLL_CLKSEL_R3_OFFSET + 31,   //< Clock Select Module ID: MI

	//Peripheral Clock Rate bit definition 0x20
	PLL_CLKSEL_VIE_CLKSRC =                   PLL_CLKSEL_R4_OFFSET + 0,   //< Clock Select Module ID: VIE
	PLL_CLKSEL_ETH_PTP_REF =                  PLL_CLKSEL_R4_OFFSET + 3,   //< Clock Select Module ID: ETH PTP REF clock source
	PLL_CLKSEL_SDIO =                         PLL_CLKSEL_R4_OFFSET + 4,   //< Clock Select Module ID: SDIO
	PLL_CLKSEL_SDIO2 =                        PLL_CLKSEL_R4_OFFSET + 8,   //< Clock Select Module ID: SDIO2
	PLL_CLKSEL_CSI =                          PLL_CLKSEL_R4_OFFSET + 12,  //< Clock Select Module ID: CSI
	PLL_CLKSEL_CSI2 =                         PLL_CLKSEL_R4_OFFSET + 14,  //< Clock Select Module ID: CSI2
	PLL_CLKSEL_IDE_CLKSRC =                   PLL_CLKSEL_R4_OFFSET + 16,  //< Clock Select Module ID: IDE
	PLL_CLKSEL_I2S_MCLK =                     PLL_CLKSEL_R4_OFFSET + 22,  //< Clock Select Module ID: I2S
	PLL_CLKSEL_DAI =                          PLL_CLKSEL_R4_OFFSET + 23,  //< Clock Select Module ID: DAI
	PLL_CLKSEL_SENPHY_CSI2_CK1_PHASE =        PLL_CLKSEL_R4_OFFSET + 24,  //< CSIPHY CK0 to CSI2 CK1 clock phase
	PLL_CLKSEL_SENPHY_CSI3_CK2_PHASE =        PLL_CLKSEL_R4_OFFSET + 25,  //< CSIPHY CK0 to CSI3 CK2 clock phase
	PLL_CLKSEL_SENPHY_CSI3_CK3_PHASE =        PLL_CLKSEL_R4_OFFSET + 26,  //< CSIPHY CK0 to CS3I CK3 clock phase
	PLL_CLKSEL_SENPHY_CSI4_CK3_PHASE =        PLL_CLKSEL_R4_OFFSET + 27,  //< CSIPHY CK0 to CSI4 CK3 clock phase
	PLL_CLKSEL_CSI3 =                         PLL_CLKSEL_R4_OFFSET + 28,  //< Clock Select Module ID: CSI3
	PLL_CLKSEL_CSI4 =                         PLL_CLKSEL_R4_OFFSET + 30,  //< Clock Select Module ID: CSI4

	//Peripheral Clock Rate bit definition 1 0x24
	PLL_CLKSEL_SDIO3 =                        PLL_CLKSEL_R5_OFFSET + 0,   //< Clock Select Module ID: SDIO3
	PLL_CLKSEL_UVCP =                         PLL_CLKSEL_R5_OFFSET + 2,   //< Clock Select Module ID: UVCP
	PLL_CLKSEL_TSE =                          PLL_CLKSEL_R5_OFFSET + 4,   //< Clock Select Module ID: TSE
	PLL_CLKSEL_SP =                           PLL_CLKSEL_R5_OFFSET + 8,   //< Clock Select Module ID: SP
	PLL_CLKSEL_SP2 =                          PLL_CLKSEL_R5_OFFSET + 10,  //< Clock Select Module ID: SP2
	PLL_CLKSEL_HASH =                         PLL_CLKSEL_R5_OFFSET + 16,  //< Clock Select Module ID: HASH
	PLL_CLKSEL_TRNG =                         PLL_CLKSEL_R5_OFFSET + 18,  //< Clock Select Module ID: TRNG
	PLL_CLKSEL_DRTC =                         PLL_CLKSEL_R5_OFFSET + 20,  //< Clock Select Module ID: DRTC
	PLL_CLKSEL_REMOTE =                       PLL_CLKSEL_R5_OFFSET + 21,  //< Clock Select Module ID: REMOTE
	PLL_CLKSEL_ADC_PD =                       PLL_CLKSEL_R5_OFFSET + 24,  //< Clock Select Module ID: ADC @ PowerDown 1/2/3
	PLL_CLKSEL_ETHPHY_CLKSRC =                PLL_CLKSEL_R5_OFFSET + 26,  //< Clock Select Module ID: ETH PHY Reference Clock Source
	PLL_CLKSEL_NAND =                         PLL_CLKSEL_R5_OFFSET + 28,  //< Clock Select Module ID: NAND
	PLL_CLKSEL_NAND_BCH =                     PLL_CLKSEL_R5_OFFSET + 30,  //< Clock Select Module ID: NAND BCH
        PLL_CLKSEL_UART =                         PLL_CLKSEL_R5_OFFSET + 31,  //< Clock Select Module ID: UART

	//Peripheral Clock Rate bit definition 2 0x28
	PLL_CLKSEL_SENPHY_CSI_CK0_PHASE =        PLL_CLKSEL_R6_OFFSET + 0,    //< CSIPHY CK0 to CSI CK0 clock phase
	PLL_CLKSEL_SENPHY_CSI_CK1_PHASE =        PLL_CLKSEL_R6_OFFSET + 1,    //< CSIPHY CK0 to CSI CK1 clock phase
	PLL_CLKSEL_SENPHY_CSI_CK2_PHASE =        PLL_CLKSEL_R6_OFFSET + 2,    //< CSIPHY CK0 to CSI CK2 clock phase
	PLL_CLKSEL_SENPHY_CSI_CK3_PHASE =        PLL_CLKSEL_R6_OFFSET + 3,    //< CSIPHY CK0 to CSI CK3 clock phase
	PLL_CLKSEL_CSI3_PXCLK =                  PLL_CLKSEL_R6_OFFSET + 4,    //< Clock Select Module ID: CSI3 pixel clock source select
	PLL_CLKSEL_SIE2_INT_CLKSRC =             PLL_CLKSEL_R6_OFFSET + 6,    //< Clock Select Module ID: SIE2 internal CLK source
	PLL_CLKSEL_SIE4_INT_CLKSRC =             PLL_CLKSEL_R6_OFFSET + 7,    //< Clock Select Module ID: SIE2 internal CLK source
	PLL_CLKSEL_CONV =                        PLL_CLKSEL_R6_OFFSET + 16,   //< Clock Select Module ID: CONV
	PLL_CLKSEL_LSU =                         PLL_CLKSEL_R6_OFFSET + 22,   //< Clock Select Module ID: LSU
	PLL_CLKSEL_NUE30 =                       PLL_CLKSEL_R6_OFFSET + 24,   //< Clock Select Module ID: NUE30
	PLL_CLKSEL_PPU =                         PLL_CLKSEL_R6_OFFSET + 26,   //< Clock Select Module ID: PPU
	PLL_CLKSEL_POU =                         PLL_CLKSEL_R6_OFFSET + 28,   //< Clock Select Module ID: POU
	PLL_CLKSEL_TRKE =                        PLL_CLKSEL_R6_OFFSET + 30,   //< Clock Select Module ID: TRKE

	//IPP Clock Rate bit definition 2 0x2C
	PLL_CLKSEL_SIE4_CLKSRC =                 PLL_CLKSEL_R7_OFFSET + 0,    //< Clock Select Module ID: SIE4
	PLL_CLKSEL_SIE5_CLKSRC =                 PLL_CLKSEL_R7_OFFSET + 4,    //< Clock Select Module ID: SIE5
	PLL_CLKSEL_STSEN_RX_CLKSRC =             PLL_CLKSEL_R7_OFFSET + 8,    //< Clock Select Module ID: Sensor TEMP RX
	PLL_CLKSEL_VIE_A_PXCLKSRC =              PLL_CLKSEL_R7_OFFSET + 10,   //< Clock Select Module ID: VIEA PX
	PLL_CLKSEL_VIE_B_PXCLKSRC =              PLL_CLKSEL_R7_OFFSET + 11,   //< Clock Select Module ID: VIEB PX


	PLL_CLKSEL_SIE4_CLKDIV =                 PLL_CLKSEL_R7_OFFSET + 16,   //< Clock Select Module ID: SIE4 clock divider
	PLL_CLKSEL_SIE5_CLKDIV =                 PLL_CLKSEL_R7_OFFSET + 24,   //< Clock Select Module ID: SIE5 clock divider
	//IPP Clock Divider bit definition 0x30
	PLL_CLKSEL_SIE_MCLKDIV =                 PLL_CLKSEL_R8_OFFSET + 0,    //< Clock Select Module ID: SIE MCLK divider
	PLL_CLKSEL_SIE_MCLK2DIV =                PLL_CLKSEL_R8_OFFSET + 8,    //< Clock Select Module ID: SIE MCLK2 divider
	PLL_CLKSEL_SIE_CLKDIV =                  PLL_CLKSEL_R8_OFFSET + 16,   //< Clock Select Module ID: SIE divider
	PLL_CLKSEL_SIE2_CLKDIV =                 PLL_CLKSEL_R8_OFFSET + 24,   //< Clock Select Module ID: SIE2 divider

	//Video Clock Divider bit definition 0x34
	PLL_CLKSEL_IDE_CLKDIV =                 PLL_CLKSEL_R9_OFFSET + 0,     //< Clock Select Module ID: IDE divider
	PLL_CLKSEL_IDE_OUTIF_CLKDIV =           PLL_CLKSEL_R9_OFFSET + 8,     //< Clock Select Module ID: IDE Output Interface clock divider
	PLL_CLKSEL_SIE_MCLK3DIV =               PLL_CLKSEL_R9_OFFSET + 16,    //< Clock Select Module ID: SIE MCLK2 divider
	PLL_CLKSEL_TRNG_CLKDIV =                PLL_CLKSEL_R9_OFFSET + 24,    //< Clock Select Module ID: TRNG divider

	//Peripheral Clock Divider bit definition 0x38
	PLL_CLKSEL_SP_CLKDIV =                  PLL_CLKSEL_R10_OFFSET + 0,    //< Clock Select Module ID: Special clock divider
	PLL_CLKSEL_SIE3_CLKDIV =                PLL_CLKSEL_R10_OFFSET + 8,    //< Clock Select Module ID: SIE3 divider
	PLL_CLKSEL_DAI_CLKDIV =                 PLL_CLKSEL_R10_OFFSET + 16,   //< Clock Select Module ID: DAI divider
	PLL_CLKSEL_DAI_OSR_CLKDIV =             PLL_CLKSEL_R10_OFFSET + 24,   //< Clock Select Module ID: audio OSR clock divider

	//SDIO Clock Divider bit definition 0x3C
	PLL_CLKSEL_SDIO_CLKDIV =                PLL_CLKSEL_R11_OFFSET + 0,   //< Clock Select Module ID: SDIO clock divider
	PLL_CLKSEL_SDIO2_CLKDIV =               PLL_CLKSEL_R11_OFFSET + 16,   //< Clock Select Module ID: SDIO2 clock divider

	//Peripheral Clock Divider bit definition 1 0x40
	PLL_CLKSEL_SDIO3_CLKDIV =               PLL_CLKSEL_R12_OFFSET + 0,   //< Clock Select Module ID: SDIO clock divider
	PLL_CLKSEL_NAND_CLKDIV =                PLL_CLKSEL_R12_OFFSET + 12,  //< Clock Select Module ID: NAND clock divider
	PLL_CLKSEL_ETH_PTP_CLKDIV =             PLL_CLKSEL_R12_OFFSET + 20,   //< Clock Select Module ID: ETH PTP clock divider
	PLL_CLKSEL_SP2_CLKDIV =                 PLL_CLKSEL_R12_OFFSET + 24,   //< Clock Select Module ID: Special clock divider

	//SPI Clock Divider bit definition 0x44
	PLL_CLKSEL_SPI_CLKDIV =                 PLL_CLKSEL_R13_OFFSET + 0,   //< Clock Select Module ID: SPI clock divider
	PLL_CLKSEL_SPI2_CLKDIV =                PLL_CLKSEL_R13_OFFSET + 16,   //< Clock Select Module ID: SPI2 clock divider

	//SPI Clock Divider bit definition 1 0x48
	PLL_CLKSEL_SPI3_CLKDIV =                PLL_CLKSEL_R14_OFFSET + 0,   //< Clock Select Module ID: SPI3 clock divider
	PLL_CLKSEL_SPI4_CLKDIV =                PLL_CLKSEL_R14_OFFSET + 16,   //< Clock Select Module ID: SPI3 clock divider

	//UART Clock Divider bit definition 0x4C
	PLL_CLKSEL_UART2_CLKDIV =               PLL_CLKSEL_R15_OFFSET + 0,   //< Clock Select Module ID: UART2 clock divider
	PLL_CLKSEL_UART3_CLKDIV =               PLL_CLKSEL_R15_OFFSET + 8,   //< Clock Select Module ID: UART3 clock divider
	PLL_CLKSEL_UART4_CLKDIV =               PLL_CLKSEL_R15_OFFSET + 16,   //< Clock Select Module ID: UART4 clock divider
	PLL_CLKSEL_UART5_CLKDIV =               PLL_CLKSEL_R15_OFFSET + 24,   //< Clock Select Module ID: UART5 clock divider

	//PWM Clock Divider bit definition 0x50
	PLL_CLKSEL_PWM0_3_CLKDIV =              PLL_CLKSEL_R16_OFFSET + 0,   //< Clock Select Module ID: PWM0_3 clock divider
	PLL_CLKSEL_PWM4_7_CLKDIV =              PLL_CLKSEL_R16_OFFSET + 16,   //< Clock Select Module ID: PWM4_7 clock divider

	//PWM Clock Divider bit definition 1 0x54
	PLL_CLKSEL_PWM8_CLKDIV =                PLL_CLKSEL_R17_OFFSET + 0,   //< Clock Select Module ID: PWM8 clock divider
	PLL_CLKSEL_PWM9_CLKDIV =                PLL_CLKSEL_R17_OFFSET + 16,   //< Clock Select Module ID: PWM9 clock divider

	//PWM Clock Divider bit definition 2 0x58
	PLL_CLKSEL_PWM10_CLKDIV =               PLL_CLKSEL_R18_OFFSET + 0,   //< Clock Select Module ID: PWM10 clock divider
	PLL_CLKSEL_PWM11_CLKDIV =               PLL_CLKSEL_R18_OFFSET + 16,   //< Clock Select Module ID: PWM11 clock divider

	//UART Clock Divider bit definition 0x5C
	PLL_CLKSEL_UART_CLKDIV =               PLL_CLKSEL_R19_OFFSET  + 16,   //< Clock Select Module ID: UART clock divider

	//Peripheral Clock Divider bit definition 2 0x60
	PLL_CLKSEL_TRNG_RO_CLKDIV =             PLL_CLKSEL_R20_OFFSET + 0,   //< Clock Select Module ID: TRNG RO clock divider
	PLL_CLKSEL_UART6_CLKDIV =               PLL_CLKSEL_R20_OFFSET + 16,   //< Clock Select Module ID: UART6 clock divider
	PLL_CLKSEL_DAI_MCLKDIV =                PLL_CLKSEL_R20_OFFSET + 24,   //< Clock Select Module ID: audio clock divider

	//Peripheral Clock Divider bit definition 3 0x64
	PLL_CLKSEL_SPI5_CLKDIV =                PLL_CLKSEL_R21_OFFSET + 0,   //< Clock Select Module ID: SPI5 clock divider
	PLL_CLKSEL_MI_CLKDIV =                  PLL_CLKSEL_R21_OFFSET + 12,   //< Clock Select Module ID: MI clock divider
	PLL_CLKSEL_VIE_CLKDIV =                 PLL_CLKSEL_R21_OFFSET + 20,   //< Clock Select Module ID: VIE clock divider

	//Peripheral Clock Divider bit definition 3 0x68
	PLL_CLKSEL_SIE_MCLK4DIV =               PLL_CLKSEL_R22_OFFSET + 12,   //< Clock Select Module ID: SIE MCLK4 divider

	//STBC
	PLL_CLKSEL_APB      =                   STBC_PLL_CLKSEL_R0_OFFSET + 8, //STBC Clock Select Module ID: APB


	//STBC
	PLL_CLKSEL_STBC_APB      =                      PLL_CLKSEL_R23_OFFSET + 8,  //STBC Clock Select Module ID: APB
	PLL_CLKSEL_STBC_RTC_32K  =                      PLL_CLKSEL_R23_OFFSET + 24, //STBC Clock Select Module ID: RTC 32K
	PLL_CLKSEL_STBC_MCU      =                      PLL_CLKSEL_R23_OFFSET + 25, //STBC Clock Select Module ID: MCU
	PLL_CLKSEL_STBC_USB30PHY =                      PLL_CLKSEL_R23_OFFSET + 31, //STBC Clock Select Module ID: USB30PHY

	PLL_CLKSEL_STBC_DSI_LP          =                   PLL_CLKSEL_R24_OFFSET,          //STBC Clock Select Module ID: DSI LP
	PLL_CLKSEL_STBC_CSI_TX_LP       =                   PLL_CLKSEL_R24_OFFSET + 2,      //STBC Clock Select Module ID: CSI TX LP
	PLL_CLKSEL_STBC_USB3_SUSP       =                   PLL_CLKSEL_R24_OFFSET + 4,      //STBC Clock Select Module ID: USB3 SUSP
	PLL_CLKSEL_STBC_USB3_PDN        =                   PLL_CLKSEL_R24_OFFSET + 5,      //STBC Clock Select Module ID: USB3 PDN

	PLL_CLKSEL_STBC_RO_DELAY        =               PLL_CLKSEL_R25_OFFSET,          //< Clock Select Module ID: RO DELAY SEL
	PLL_CLKSEL_STBC_RO32K_CLKDIV    =               PLL_CLKSEL_R25_OFFSET + 4,      //< Clock Select Module ID: RO 32K clock divider
	PLL_CLKSEL_STBC_RO32K_DIV_TESTEN =               PLL_CLKSEL_R25_OFFSET + 16,    //< Ring oscillator range: 53 ~142MHz*/


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
#define PLL_CLKSEL_CPU_PLL8       (0x01 << PLL_CLKSEL_CPU)      //< Select CPU clock PLL8 (for CPU)
#define PLL_CLKSEL_CPU_APLL       PLL_CLKSEL_CPU_PLL8          //< Select CPU clock APLL
#define PLL_CLKSEL_CPU_480        (0x02 << PLL_CLKSEL_CPU)      //< Select CPU clock 480MHz
//@}

/*
    @name   APB clock rate value

    APB clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_APB).
*/
//@{
#define PLL_CLKSEL_APB_48        (0x00 << (PLL_CLKSEL_APB - STBC_PLL_CLKSEL_R0_OFFSET))      //< Select APB  120MHz
#define PLL_CLKSEL_APB_60        (0x01 << (PLL_CLKSEL_APB - STBC_PLL_CLKSEL_R0_OFFSET))      //< Select APB  120MHz
#define PLL_CLKSEL_APB_80        (0x02 << (PLL_CLKSEL_APB - STBC_PLL_CLKSEL_R0_OFFSET))      //< Select APB  120MHz
#define PLL_CLKSEL_APB_120       (0x03 << (PLL_CLKSEL_APB - STBC_PLL_CLKSEL_R0_OFFSET))      //< Select APB  150MHz

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


//////////////////////////////////// R1 select done.


/*
    @name   IVE clock rate value

    IVE clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_IVE).
*/
//@{
#define PLL_CLKSEL_IVE_240        (0x00 << (PLL_CLKSEL_IVE - PLL_CLKSEL_R1_OFFSET)) //< Select IVE clock as 240MHz
#define PLL_CLKSEL_IVE_320        (0x01 << (PLL_CLKSEL_IVE - PLL_CLKSEL_R1_OFFSET)) //< Select IVE clock as 320MHz
#define PLL_CLKSEL_IVE_PLL17      (0x01 << (PLL_CLKSEL_IVE - PLL_CLKSEL_R1_OFFSET)) //< Select IVE clock as PLL17
#define PLL_CLKSEL_IVE_480        (0x02 << (PLL_CLKSEL_IVE - PLL_CLKSEL_R1_OFFSET)) //< Select IVE clock as 480MHz
#define PLL_CLKSEL_IVE_PLL13      (0x03 << (PLL_CLKSEL_IVE - PLL_CLKSEL_R1_OFFSET)) //< Select IVE clock as PLL13
//@}


/*
    @name   IPE clock rate value

    IPE clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_IPE).
*/
//@{
#define PLL_CLKSEL_IPE_240        (0x00 << (PLL_CLKSEL_IPE - PLL_CLKSEL_R1_OFFSET)) //< Select IPE clock as 240MHz
#define PLL_CLKSEL_IPE_320        (0x01 << (PLL_CLKSEL_IPE - PLL_CLKSEL_R1_OFFSET)) //< Select IPE clock as 320MHz
#define PLL_CLKSEL_IPE_PLL17      (0x02 << (PLL_CLKSEL_IPE - PLL_CLKSEL_R1_OFFSET)) //< Select IPE clock as PLL17
#define PLL_CLKSEL_IPE_PLL13      (0x03 << (PLL_CLKSEL_IPE - PLL_CLKSEL_R1_OFFSET)) //< Select IPE clock as PLL13
//@}


/*
    @name   IME clock rate value

    IME clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_IME).
*/
//@{
#define PLL_CLKSEL_IME_240        (0x00 << (PLL_CLKSEL_IME - PLL_CLKSEL_R1_OFFSET)) //< Select IME clock as 240MHz
#define PLL_CLKSEL_IME_320        (0x01 << (PLL_CLKSEL_IME - PLL_CLKSEL_R1_OFFSET)) //< Select IME clock as 320MHz
#define PLL_CLKSEL_IME_PLL17      (0x02 << (PLL_CLKSEL_IME - PLL_CLKSEL_R1_OFFSET)) //< Select IME clock as PLL17
#define PLL_CLKSEL_IME_PLL13      (0x03 << (PLL_CLKSEL_IME - PLL_CLKSEL_R1_OFFSET)) //< Select IME clock as PLL13
//@}

/*
    @name   MDBC clock rate value

    TRKE clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_MDBC).
*/
//@{
#define PLL_CLKSEL_MDBC_240        (0x00 << (PLL_CLKSEL_MDBC - PLL_CLKSEL_R1_OFFSET)) //< Select MDBC clock as 240MHz
#define PLL_CLKSEL_MDBC_320        (0x01 << (PLL_CLKSEL_MDBC - PLL_CLKSEL_R1_OFFSET)) //< Select MDBC clock as 320MHz
#define PLL_CLKSEL_MDBC_PLL13      (0x03 << (PLL_CLKSEL_MDBC - PLL_CLKSEL_R1_OFFSET)) //< Select MDBC clock as PLL13

//@}

/*
    @name   VPE clock rate value

    VPE clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_VPE).
*/
//@{
#define PLL_CLKSEL_VPE_240          (0x00 << (PLL_CLKSEL_VPE - PLL_CLKSEL_R1_OFFSET)) //< Select VPE clock as 240MHz
#define PLL_CLKSEL_VPE_320          (0x01 << (PLL_CLKSEL_VPE - PLL_CLKSEL_R1_OFFSET)) //< Select VPE clock as 320MHz
#define PLL_CLKSEL_VPE_PLL17        (0x02 << (PLL_CLKSEL_VPE - PLL_CLKSEL_R1_OFFSET)) //< Select VPE clock as PLL17
#define PLL_CLKSEL_VPE_PLL13        (0x03 << (PLL_CLKSEL_VPE - PLL_CLKSEL_R1_OFFSET)) //< Select VPE clock as PLL13
#define PLL_CLKSEL_VPE_480          (0x03 << (PLL_CLKSEL_VPE - PLL_CLKSEL_R1_OFFSET)) //< Select VPE clock as 480MHz

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
#define PLL_CLKSEL_ISE_PLL17      (0x03 << (PLL_CLKSEL_ISE - PLL_CLKSEL_R1_OFFSET)) //< Select ISE clock as PLL17
#define PLL_CLKSEL_ISE_PLL13      (0x03 << (PLL_CLKSEL_ISE - PLL_CLKSEL_R1_OFFSET)) //< Select ISE clock as PLL13

//@}

/*
    @name   TGE MSH CLK clock source value

    TGE CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_TGE_MSH).
*/
//@{
#define PLL_CLKSEL_TGE_MSH_SIEMCLK           (0x00 << (PLL_CLKSEL_TGE_MSH - PLL_CLKSEL_R1_OFFSET))  //< Select TGE MSH CLK source as SIE MCLK
#define PLL_CLKSEL_TGE_MSH_SIEMCLK2          (0x01 << (PLL_CLKSEL_TGE_MSH - PLL_CLKSEL_R1_OFFSET))  //< Select TGE MSH CLK source as SIE MCLK2
#define PLL_CLKSEL_TGE_MSH_SIEMCLK3          (0x02 << (PLL_CLKSEL_TGE_MSH - PLL_CLKSEL_R1_OFFSET))  //< Select TGE MSH CLK source as SIE MCLK3
#define PLL_CLKSEL_TGE_MSH_SIEMCLK4          (0x03 << (PLL_CLKSEL_TGE_MSH - PLL_CLKSEL_R1_OFFSET))  //< Select TGE MSH CLK source as SIE MCLK4
//@}

/*
    @name   TGE FLASH CLK clock source value

    TGE CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_TGE_FLASH).
*/
//@{
#define PLL_CLKSEL_TGE_FLASH_SIEMCLK           (0x00 << (PLL_CLKSEL_TGE_FLASH - PLL_CLKSEL_R1_OFFSET))  //< Select TGE FLASH CLK source as SIE MCLK
#define PLL_CLKSEL_TGE_FLASH_SIEMCLK2          (0x01 << (PLL_CLKSEL_TGE_FLASH - PLL_CLKSEL_R1_OFFSET))  //< Select TGE FLASH CLK source as SIE MCLK2
#define PLL_CLKSEL_TGE_FLASH_SIEMCLK3          (0x02 << (PLL_CLKSEL_TGE_FLASH - PLL_CLKSEL_R1_OFFSET))  //< Select TGE FLASH CLK source as SIE MCLK3
#define PLL_CLKSEL_TGE_FLASH_SIEMCLK4          (0x03 << (PLL_CLKSEL_TGE_FLASH - PLL_CLKSEL_R1_OFFSET))  //< Select TGE FLASH CLK source as SIE MCLK4
//@}


//////////////////////////////////// R1 select done.


/*
    @name   IFE clock rate value

    IFE clock rate value

    This is for pll_set_clock_rate(PLL_CLKSEL_IFE).
*/
//@{
#define PLL_CLKSEL_IFE_240        (0x00 << (PLL_CLKSEL_IFE - PLL_CLKSEL_R2_OFFSET)) //< Select IFE clock as 240MHz
#define PLL_CLKSEL_IFE_320        (0x01 << (PLL_CLKSEL_IFE - PLL_CLKSEL_R2_OFFSET)) //< Select IFE clock as 320MHz
#define PLL_CLKSEL_IFE_PLL13      (0x03 << (PLL_CLKSEL_IFE - PLL_CLKSEL_R2_OFFSET)) //< Select IFE clock as PLL13
//@}

/*
    @name   PRE IFE clock rate value

    IFE clock rate value

    This is for pll_set_clock_rate(PLL_CLKSEL_PRE_IFE).
*/
//@{
#define PLL_CLKSEL_PRE_IFE_240        (0x00 << (PLL_CLKSEL_PRE_IFE - PLL_CLKSEL_R2_OFFSET)) //< Select PRE IFE clock as 240MHz
#define PLL_CLKSEL_PRE_IFE_320        (0x01 << (PLL_CLKSEL_PRE_IFE - PLL_CLKSEL_R2_OFFSET)) //< Select PRE IFE clock as 320MHz
#define PLL_CLKSEL_PRE_IFE_PLL17      (0x02 << (PLL_CLKSEL_PRE_IFE - PLL_CLKSEL_R2_OFFSET)) //< Select PRE IFE clock as PLL17
#define PLL_CLKSEL_PRE_IFE_PLL13      (0x03 << (PLL_CLKSEL_PRE_IFE - PLL_CLKSEL_R2_OFFSET)) //< Select PRE IFE clock as PLL13
//@}


/*
    @name   IFE SHARED SRAM clock rate value

    IFE clock rate value

    This is for pll_set_clock_rate(PLL_CLKSEL_IFE_SRAM).
*/
//@{
#define PLL_CLKSEL_IFE_SRAM_CPU        (0x00 << (PLL_CLKSEL_IFE_SRAM - PLL_CLKSEL_R2_OFFSET)) //< IFE SRAM to CPU
#define PLL_CLKSEL_IFE_SRAM_IFE        (0x01 << (PLL_CLKSEL_IFE_SRAM - PLL_CLKSEL_R2_OFFSET)) //< IFE SRAM to IFE

//@}

/*
    @name   PRE SHARED SRAM clock rate value

    PRE clock rate value

    This is for pll_set_clock_rate(PLL_CLKSEL_PRE).
*/
//@{
#define PLL_CLKSEL_PRE_SRAM_CPU        (0x00 << (PLL_CLKSEL_PRE_SRAM - PLL_CLKSEL_R2_OFFSET)) //< PRE SRAM to CPU
#define PLL_CLKSEL_PRE_SRAM_PRE        (0x01 << (PLL_CLKSEL_PRE_SRAM - PLL_CLKSEL_R2_OFFSET)) //< PRE SRAM to IFE


//@}

/*
    @name   SIE MCLK4 clock source value

    SIE MCLK4 clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK4SRC).
*/
//@{
#define PLL_CLKSEL_SIE_MCLK4SRC_480     (0x00 << (PLL_CLKSEL_SIE_MCLK4SRC - PLL_CLKSEL_R2_OFFSET))   //< Select SIE MCLK4 clock source as 480 MHz
#define PLL_CLKSEL_SIE_MCLK4SRC_PLL5    (0x01 << (PLL_CLKSEL_SIE_MCLK4SRC - PLL_CLKSEL_R2_OFFSET))   //< Select SIE MCLK4 clock source as PLL5
#define PLL_CLKSEL_SIE_MCLK4SRC_PLL6    (0x02 << (PLL_CLKSEL_SIE_MCLK4SRC - PLL_CLKSEL_R2_OFFSET))   //< Select SIE MCLK4 clock source as PLL6
#define PLL_CLKSEL_SIE_MCLK4SRC_PLL12   (0x03 << (PLL_CLKSEL_SIE_MCLK4SRC - PLL_CLKSEL_R2_OFFSET))   //< Select SIE MCLK4 clock source as PLL12
//@}

/*
    @name   SIE MCLK clock source value

    SIE MCLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLKSRC).
*/
//@{
#define PLL_CLKSEL_SIE_MCLKSRC_480      (0x00 << (PLL_CLKSEL_SIE_MCLKSRC - PLL_CLKSEL_R2_OFFSET))   //< Select SIE MCLK clock source as 480 MHz
#define PLL_CLKSEL_SIE_MCLKSRC_PLL5     (0x01 << (PLL_CLKSEL_SIE_MCLKSRC - PLL_CLKSEL_R2_OFFSET))   //< Select SIE MCLK clock source as PLL5
#define PLL_CLKSEL_SIE_MCLKSRC_PLL6     (0x02 << (PLL_CLKSEL_SIE_MCLKSRC - PLL_CLKSEL_R2_OFFSET))   //< Select SIE MCLK clock source as PLL6
#define PLL_CLKSEL_SIE_MCLKSRC_PLL12    (0x03 << (PLL_CLKSEL_SIE_MCLKSRC - PLL_CLKSEL_R2_OFFSET))   //< Select SIE MCLK clock source as PLL12
//@}

/*
    @name   SIE MCLK2 clock source value

    SIE MCLK2 clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK2SRC).
*/
//@{
#define PLL_CLKSEL_SIE_MCLK2SRC_480     (0x00 << (PLL_CLKSEL_SIE_MCLK2SRC - PLL_CLKSEL_R2_OFFSET)) //< Select SIE MCLK2 clock source as 480 MHz
#define PLL_CLKSEL_SIE_MCLK2SRC_PLL5    (0x01 << (PLL_CLKSEL_SIE_MCLK2SRC - PLL_CLKSEL_R2_OFFSET)) //< Select SIE MCLK2 clock source as PLL5
#define PLL_CLKSEL_SIE_MCLK2SRC_PLL6    (0x02 << (PLL_CLKSEL_SIE_MCLK2SRC - PLL_CLKSEL_R2_OFFSET)) //< Select SIE MCLK2 clock source as PLL6
#define PLL_CLKSEL_SIE_MCLK2SRC_PLL12   (0x03 << (PLL_CLKSEL_SIE_MCLK2SRC - PLL_CLKSEL_R2_OFFSET)) //< Select SIE MCLK2 clock source as PLL11
//@}

/*
    @name   SIE MCLK3 clock source value

    SIE MCLK2 clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK3SRC).
*/
//@{
#define PLL_CLKSEL_SIE_MCLK3SRC_480     (0x00 << (PLL_CLKSEL_SIE_MCLK3SRC - PLL_CLKSEL_R2_OFFSET)) //< Select SIE MCLK3 clock source as 480 MHz
#define PLL_CLKSEL_SIE_MCLK3SRC_PLL5    (0x01 << (PLL_CLKSEL_SIE_MCLK3SRC - PLL_CLKSEL_R2_OFFSET)) //< Select SIE MCLK3 clock source as PLL5
#define PLL_CLKSEL_SIE_MCLK3SRC_PLL6    (0x02 << (PLL_CLKSEL_SIE_MCLK3SRC - PLL_CLKSEL_R2_OFFSET)) //< Select SIE MCLK3 clock source as PLL6
#define PLL_CLKSEL_SIE_MCLK3SRC_PLL12   (0x03 << (PLL_CLKSEL_SIE_MCLK3SRC - PLL_CLKSEL_R2_OFFSET)) //< Select SIE MCLK3 clock source as PLL11

//@}

/*
    @name   SIE5 SHARED SRAM clock rate value

    SIE5 clock rate value

    This is for pll_set_clock_rate(PLL_CLKSEL_SIE5).
*/
//@{
#define PLL_CLKSEL_SIE5_SRAM_SIE5        (0x00 << (PLL_CLKSEL_SIE5_SRAM - PLL_CLKSEL_R2_OFFSET)) //< PRE SRAM to SIE5
#define PLL_CLKSEL_SIE5_SRAM_VIE         (0x01 << (PLL_CLKSEL_SIE5_SRAM - PLL_CLKSEL_R2_OFFSET)) //< PRE SRAM to VIE


//@}


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
    @name   NUE2 clock rate value

    NUE2 clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_NUE2).
*/
//@{
#define PLL_CLKSEL_NUE2_PLL10       (0x00 << (PLL_CLKSEL_NUE2 - PLL_CLKSEL_R2_OFFSET)) //< Select NUE2 clock as PLL10
#define PLL_CLKSEL_NUE2_240         (0x01 << (PLL_CLKSEL_NUE2 - PLL_CLKSEL_R2_OFFSET)) //< Select NUE2 clock as 240MHz
#define PLL_CLKSEL_NUE2_320         (0x02 << (PLL_CLKSEL_NUE2 - PLL_CLKSEL_R2_OFFSET)) //< Select NUE2 clock as 320MHz
#define PLL_CLKSEL_NUE2_480         (0x03 << (PLL_CLKSEL_NUE2 - PLL_CLKSEL_R2_OFFSET)) //< Select NUE2 clock as 480MHz
//@}



//////////////////////////////////// R2 select done.


/*
    @name   JPEG clock rate value

    JPEG clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_JPEG).
*/
//@{
#define PLL_CLKSEL_JPEG_240       (0x00 << (PLL_CLKSEL_JPEG - PLL_CLKSEL_R3_OFFSET)) //< Select JPEG clock as 240MHz
#define PLL_CLKSEL_JPEG_320       (0x01 << (PLL_CLKSEL_JPEG - PLL_CLKSEL_R3_OFFSET)) //< Select JPEG clock as 320MHz
#define PLL_CLKSEL_JPEG_480       (0x02 << (PLL_CLKSEL_JPEG - PLL_CLKSEL_R3_OFFSET)) //< Select JPEG clock as 480MHz
#define PLL_CLKSEL_JPEG_PLL15      (0x03 << (PLL_CLKSEL_JPEG - PLL_CLKSEL_R3_OFFSET)) //< Select JPEG clock as PLL15
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
#define PLL_CLKSEL_DRE_PLL17   (0x03 << (PLL_CLKSEL_DRE - PLL_CLKSEL_R3_OFFSET))  //< Select DRE clock as PLL17
//@}


/*
    @name VENC clock rate value

    VENC clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_H265).
*/
//@{
#define PLL_CLKSEL_VENC_240       (0x00 << (PLL_CLKSEL_VENC - PLL_CLKSEL_R3_OFFSET)) //< Select VENC clock as 240
#define PLL_CLKSEL_VENC_320       (0x01 << (PLL_CLKSEL_VENC - PLL_CLKSEL_R3_OFFSET)) //< Select VENC clock as 320
#define PLL_CLKSEL_VENC_PLL15     (0x02 << (PLL_CLKSEL_VENC - PLL_CLKSEL_R3_OFFSET)) //< Select VENC clock as PLL15
#define PLL_CLKSEL_VENC_PLL13     (0x03 << (PLL_CLKSEL_VENC - PLL_CLKSEL_R3_OFFSET)) //< Select VENC clock as PLL13
//@}

/*
    @name   Crypto clock rate value

    Crypto clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_CRYPTO).
*/
//@{
#define PLL_CLKSEL_ECDSA_240    (0x00 << (PLL_CLKSEL_ECDSA - PLL_CLKSEL_R3_OFFSET))    //< Select Crypto clock as 240Mhz
#define PLL_CLKSEL_ECDSA_320    (0x01 << (PLL_CLKSEL_ECDSA - PLL_CLKSEL_R3_OFFSET))    //< Select Crypto clock as 320Mhz
#define PLL_CLKSEL_ECDSA_PLL9   (0x03 << (PLL_CLKSEL_ECDSA - PLL_CLKSEL_R3_OFFSET))    //< Select Crypto clock as PLL9
//@}

/*
    @name   Graphic clock rate value

    Graphic clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_GRAPHIC).
*/
//@{
#define PLL_CLKSEL_GRAPHIC_240      (0x00 << (PLL_CLKSEL_GRAPHIC - PLL_CLKSEL_R3_OFFSET)) //< Select Graphic clock as 240Mhz
#define PLL_CLKSEL_GRAPHIC_320      (0x01 << (PLL_CLKSEL_GRAPHIC - PLL_CLKSEL_R3_OFFSET)) //< Select Graphic clock as 320Mhz
#define PLL_CLKSEL_GRAPHIC_480      (0x02 << (PLL_CLKSEL_GRAPHIC - PLL_CLKSEL_R3_OFFSET)) //< Select Graphic clock as 480Mhz
#define PLL_CLKSEL_GRAPHIC_PLL13    (0x03 << (PLL_CLKSEL_GRAPHIC - PLL_CLKSEL_R3_OFFSET)) //< Select Graphic clock as PLL13
#define PLL_CLKSEL_GRAPHIC_PLL17    (0x03 << (PLL_CLKSEL_GRAPHIC - PLL_CLKSEL_R3_OFFSET)) //< Select Graphic clock as PLL17
//@}


/*
    @name   Graphic2 clock rate value

    Graphic2 clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_GRAPHIC2).
*/
//@{
#define PLL_CLKSEL_GRAPHIC2_240      (0x00 << (PLL_CLKSEL_GRAPHIC2 - PLL_CLKSEL_R3_OFFSET)) //< Select Graphic2 clock as 240Mhz
#define PLL_CLKSEL_GRAPHIC2_320      (0x01 << (PLL_CLKSEL_GRAPHIC2 - PLL_CLKSEL_R3_OFFSET)) //< Select Graphic2 clock as 320Mhz
#define PLL_CLKSEL_GRAPHIC2_480      (0x02 << (PLL_CLKSEL_GRAPHIC2 - PLL_CLKSEL_R3_OFFSET)) //< Select Graphic2 clock as 480Mhz
#define PLL_CLKSEL_GRAPHIC2_PLL13    (0x03 << (PLL_CLKSEL_GRAPHIC2 - PLL_CLKSEL_R3_OFFSET)) //< Select Graphic2 clock as PLL13
#define PLL_CLKSEL_GRAPHIC2_PLL17    (0x03 << (PLL_CLKSEL_GRAPHIC2 - PLL_CLKSEL_R3_OFFSET)) //< Select Graphic2 clock as PLL17
//@}

/*
    @name   Graphic3 clock rate value

    Graphic3 clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_GRAPHIC3).
*/
//@{
#define PLL_CLKSEL_GRAPHIC3_240      (0x00 << (PLL_CLKSEL_GRAPHIC3 - PLL_CLKSEL_R3_OFFSET)) //< Select Graphic3 clock as 240Mhz
#define PLL_CLKSEL_GRAPHIC3_320      (0x01 << (PLL_CLKSEL_GRAPHIC3 - PLL_CLKSEL_R3_OFFSET)) //< Select Graphic3 clock as 320Mhz
#define PLL_CLKSEL_GRAPHIC3_480      (0x02 << (PLL_CLKSEL_GRAPHIC3 - PLL_CLKSEL_R3_OFFSET)) //< Select Graphic3 clock as 480Mhz
#define PLL_CLKSEL_GRAPHIC3_PLL13    (0x03 << (PLL_CLKSEL_GRAPHIC3 - PLL_CLKSEL_R3_OFFSET)) //< Select Graphic3 clock as PLL13
#define PLL_CLKSEL_GRAPHIC3_PLL17    (0x03 << (PLL_CLKSEL_GRAPHIC3 - PLL_CLKSEL_R3_OFFSET)) //< Select Graphic3 clock as PLL17
//@}

/*
    @name   SIE3 CLK clock source value

    SIE3 CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE3_CLKSRC).
*/
//@{
#define PLL_CLKSEL_SIE3_CLKSRC_480       (0x00 << (PLL_CLKSEL_SIE3_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE3 CLK clock source as 480 MHz
#define PLL_CLKSEL_SIE3_CLKSRC_PLL5      (0x02 << (PLL_CLKSEL_SIE3_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE3 CLK clock source as PLL5
#define PLL_CLKSEL_SIE3_CLKSRC_PLL13     (0x03 << (PLL_CLKSEL_SIE3_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE3 CLK clock source as PLL13
#define PLL_CLKSEL_SIE3_CLKSRC_PLL12     (0x04 << (PLL_CLKSEL_SIE3_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE3 CLK clock source as PLL12
#define PLL_CLKSEL_SIE3_CLKSRC_320       (0x05 << (PLL_CLKSEL_SIE3_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE3 CLK clock source as 320 MHz
#define PLL_CLKSEL_SIE3_CLKSRC_192       (0x06 << (PLL_CLKSEL_SIE3_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE3 CLK clock source as 192 MHz
#define PLL_CLKSEL_SIE3_CLKSRC_PLL10     (0x07 << (PLL_CLKSEL_SIE3_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE3 CLK clock source as PLL10
//@}

/*
    @name   HWCOPY clock rate value

    HWCOPY clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_HWCOPY).
*/
//@{
#define PLL_CLKSEL_HWCP_PLL0     (0x00 << (PLL_CLKSEL_HWCP - PLL_CLKSEL_R3_OFFSET))  //< Select HWCOPY clock as 240Mhz
#define PLL_CLKSEL_HWCP_240      (0x01 << (PLL_CLKSEL_HWCP - PLL_CLKSEL_R3_OFFSET))  //< Select HWCOPY clock as PLL2
//@}

/*
    @name   Crypto clock rate value

    Crypto clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_CRYPTO).
*/
//@{
#define PLL_CLKSEL_CRYPTO_240    (0x00 << (PLL_CLKSEL_CRYPTO - PLL_CLKSEL_R3_OFFSET))    //< Select Crypto clock as 240Mhz
#define PLL_CLKSEL_CRYPTO_320    (0x01 << (PLL_CLKSEL_CRYPTO - PLL_CLKSEL_R3_OFFSET))    //< Select Crypto clock as 320Mhz
#define PLL_CLKSEL_CRYPTO_PLL9   (0x03 << (PLL_CLKSEL_CRYPTO - PLL_CLKSEL_R3_OFFSET))    //< Select Crypto clock as PLL9
//@}


/*
    @name   RSA clock rate value

    RSA clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_RSA).
*/
//@{
#define PLL_CLKSEL_RSA_240    (0x00 << (PLL_CLKSEL_RSA - PLL_CLKSEL_R3_OFFSET))    //< Select RSA clock as 240 MHz
#define PLL_CLKSEL_RSA_320    (0x01 << (PLL_CLKSEL_RSA - PLL_CLKSEL_R3_OFFSET))    //< Select RSA clock as 320 MHz
#define PLL_CLKSEL_RSA_PLL9   (0x03 << (PLL_CLKSEL_RSA - PLL_CLKSEL_R3_OFFSET))    //< Select RSA clock as PLL9
//@}


/*
    @name   SIE CLK clock source value

    SIE CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE_CLKSRC).
*/
//@{
#define PLL_CLKSEL_SIE_CLKSRC_480       (0x00 << (PLL_CLKSEL_SIE_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE CLK clock source as 480 MHz
#define PLL_CLKSEL_SIE_CLKSRC_PLL17     (0x01 << (PLL_CLKSEL_SIE_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE CLK clock source as PLL17
#define PLL_CLKSEL_SIE_CLKSRC_PLL5      (0x02 << (PLL_CLKSEL_SIE_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE CLK clock source as PLL5
#define PLL_CLKSEL_SIE_CLKSRC_PLL13     (0x03 << (PLL_CLKSEL_SIE_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE CLK clock source as PLL13
#define PLL_CLKSEL_SIE_CLKSRC_PLL12     (0x04 << (PLL_CLKSEL_SIE_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE CLK clock source as PLL12
#define PLL_CLKSEL_SIE_CLKSRC_320       (0x05 << (PLL_CLKSEL_SIE_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE CLK clock source as 320 MHz
#define PLL_CLKSEL_SIE_CLKSRC_192       (0x06 << (PLL_CLKSEL_SIE_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE CLK clock source as 192 MHz
#define PLL_CLKSEL_SIE_CLKSRC_PLL10     (0x07 << (PLL_CLKSEL_SIE_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE CLK clock source as PLL10
//@}


/*
    @name   SIE2 CLK clock source value

    SIE2 CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE2_CLKSRC).
*/
//@{
#define PLL_CLKSEL_SIE2_CLKSRC_480       (0x00 << (PLL_CLKSEL_SIE2_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE2 CLK clock source as 480 MHz
#define PLL_CLKSEL_SIE2_CLKSRC_PLL17     (0x01 << (PLL_CLKSEL_SIE2_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE2 CLK clock source as PLL17
#define PLL_CLKSEL_SIE2_CLKSRC_PLL5      (0x02 << (PLL_CLKSEL_SIE2_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE2 CLK clock source as PLL5
#define PLL_CLKSEL_SIE2_CLKSRC_PLL13     (0x03 << (PLL_CLKSEL_SIE2_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE2 CLK clock source as PLL13
#define PLL_CLKSEL_SIE2_CLKSRC_PLL12     (0x04 << (PLL_CLKSEL_SIE2_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE2 CLK clock source as PLL12
#define PLL_CLKSEL_SIE2_CLKSRC_320       (0x05 << (PLL_CLKSEL_SIE2_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE2 CLK clock source as 320 MHz
#define PLL_CLKSEL_SIE2_CLKSRC_192       (0x06 << (PLL_CLKSEL_SIE2_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE2 CLK clock source as 192 MHz
#define PLL_CLKSEL_SIE2_CLKSRC_PLL10     (0x07 << (PLL_CLKSEL_SIE2_CLKSRC - PLL_CLKSEL_R3_OFFSET))    //< Select SIE2 CLK clock source as PLL10
//@}

/*
    @name   MI clock rate value

    MI clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_MI).
*/
//@{
#define PLL_CLKSEL_MI_192     (0x00 << (PLL_CLKSEL_MI - PLL_CLKSEL_R3_OFFSET))  //< Select MI clock as 192Mhz
#define PLL_CLKSEL_MI_320     (0x01 << (PLL_CLKSEL_MI - PLL_CLKSEL_R3_OFFSET))  //< Select MI clock as 320Mhz
//@}



//////////////////////////////////// R3 select done.


/*
    @name   VIE CLK clock source value

    VIE CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_VIE_CLKSRC).
*/
//@{
#define PLL_CLKSEL_VIE_CLKSRC_480       (0x00 << (PLL_CLKSEL_VIE_CLKSRC - PLL_CLKSEL_R4_OFFSET))    //< Select VIE CLK clock source as 480 MHz
#define PLL_CLKSEL_VIE_CLKSRC_PLL5      (0x02 << (PLL_CLKSEL_VIE_CLKSRC - PLL_CLKSEL_R4_OFFSET))    //< Select VIE CLK clock source as PLL5
#define PLL_CLKSEL_VIE_CLKSRC_PLL13     (0x03 << (PLL_CLKSEL_VIE_CLKSRC - PLL_CLKSEL_R4_OFFSET))    //< Select VIE CLK clock source as PLL13
#define PLL_CLKSEL_VIE_CLKSRC_PLL12     (0x04 << (PLL_CLKSEL_VIE_CLKSRC - PLL_CLKSEL_R4_OFFSET))    //< Select VIE CLK clock source as PLL12
#define PLL_CLKSEL_VIE_CLKSRC_320       (0x05 << (PLL_CLKSEL_VIE_CLKSRC - PLL_CLKSEL_R4_OFFSET))    //< Select VIE CLK clock source as 320 MHz
#define PLL_CLKSEL_VIE_CLKSRC_192       (0x06 << (PLL_CLKSEL_VIE_CLKSRC - PLL_CLKSEL_R4_OFFSET))    //< Select VIE CLK clock source as 192 MHz
#define PLL_CLKSEL_VIE_CLKSRC_PLL10     (0x07 << (PLL_CLKSEL_VIE_CLKSRC - PLL_CLKSEL_R4_OFFSET))    //< Select VIE CLK clock source as PLL10
//@}

/*
    @name   Ethernet PTP reference clock rate value

    Ethernet PTP reference clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_ETH_PTP_REF).
*/
//@{
#define PLL_CLKSEL_ETH_PTP_480     (0x00 << (PLL_CLKSEL_ETH_PTP_REF - PLL_CLKSEL_R4_OFFSET))  //< Select Ethernet PTP reference clock as 480Mhz
#define PLL_CLKSEL_ETH_PTP_PLL16   (0x01 << (PLL_CLKSEL_ETH_PTP_REF - PLL_CLKSEL_R4_OFFSET))  //< Select Ethernet PTP reference clock as PLL16
//@}


/*
    @name   SDIO clock rate value

    SDIO clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SDIO).
*/
//@{
#define PLL_CLKSEL_SDIO_192          (0x00 << (PLL_CLKSEL_SDIO - PLL_CLKSEL_R4_OFFSET))  //< Select SDIO clock as 192Mhz
#define PLL_CLKSEL_SDIO_480          (0x01 << (PLL_CLKSEL_SDIO - PLL_CLKSEL_R4_OFFSET))  //< Select SDIO clock as 480Mhz
#define PLL_CLKSEL_SDIO_PLL9         (0x02 << (PLL_CLKSEL_SDIO - PLL_CLKSEL_R4_OFFSET))  //< Select SDIO clock as PLL9
#define PLL_CLKSEL_SDIO_PLL4         (0x03 << (PLL_CLKSEL_SDIO - PLL_CLKSEL_R4_OFFSET))  //< Select SDIO clock as PLL4
//@}


/*
    @name   SDIO2 clock rate value

    SDIO2 clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SDIO2).
*/
//@{
#define PLL_CLKSEL_SDIO2_192         (0x00 << (PLL_CLKSEL_SDIO2 - PLL_CLKSEL_R4_OFFSET)) //< Select SDIO2 clock as 192Mhz
#define PLL_CLKSEL_SDIO2_480         (0x01 << (PLL_CLKSEL_SDIO2 - PLL_CLKSEL_R4_OFFSET)) //< Select SDIO2 clock as 480Mhz
#define PLL_CLKSEL_SDIO2_PLL9        (0x02 << (PLL_CLKSEL_SDIO2 - PLL_CLKSEL_R4_OFFSET)) //< Select SDIO2 clock as PLL9
#define PLL_CLKSEL_SDIO2_PLL4        (0x03 << (PLL_CLKSEL_SDIO2 - PLL_CLKSEL_R4_OFFSET)) //< Select SDIO2 clock as PLL6
//@}


/*
    @name CSI clock rate value

    CSI clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_CSI).
*/
//@{
#define PLL_CLKSEL_CSI_60  (0x00 << (PLL_CLKSEL_CSI - PLL_CLKSEL_R4_OFFSET))     //< Select CSI clock as 60Mhz
#define PLL_CLKSEL_CSI_120 (0x01 << (PLL_CLKSEL_CSI - PLL_CLKSEL_R4_OFFSET))     //< Select CSI clock as 120Mhz
#define PLL_CLKSEL_CSI_240 (0x02 << (PLL_CLKSEL_CSI - PLL_CLKSEL_R4_OFFSET))     //< Select CSI clock as 240Mhz

//@}

/*
    @name CSI2 clock rate value

    CSI2 clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_CSI2).
*/
//@{
#define PLL_CLKSEL_CSI2_60  (0x00 << (PLL_CLKSEL_CSI2 - PLL_CLKSEL_R4_OFFSET))     //< Select CSI2 clock as 60Mhz
#define PLL_CLKSEL_CSI2_120 (0x01 << (PLL_CLKSEL_CSI2 - PLL_CLKSEL_R4_OFFSET))     //< Select CSI2 clock as 120Mhz
#define PLL_CLKSEL_CSI2_240 (0x02 << (PLL_CLKSEL_CSI2 - PLL_CLKSEL_R4_OFFSET))     //< Select CSI2 clock as 240Mhz
//@}



/*
    @name   IDE clock source value

    IDE clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_IDE_CLKSRC).
*/
//@{
#define PLL_CLKSEL_IDE_CLKSRC_480    (0x00 << (PLL_CLKSEL_IDE_CLKSRC - PLL_CLKSEL_R4_OFFSET))    //< Select IDE clock source as 480 MHz
#define PLL_CLKSEL_IDE_CLKSRC_PLL6   (0x01 << (PLL_CLKSEL_IDE_CLKSRC - PLL_CLKSEL_R4_OFFSET))    //< Select IDE clock source as PLL6
#define PLL_CLKSEL_IDE_CLKSRC_PLL4   (0x02 << (PLL_CLKSEL_IDE_CLKSRC - PLL_CLKSEL_R4_OFFSET))    //< Select IDE clock source as PLL4
#define PLL_CLKSEL_IDE_CLKSRC_PLL9   (0x03 << (PLL_CLKSEL_IDE_CLKSRC - PLL_CLKSEL_R4_OFFSET))    //< Select IDE clock source as PLL9
//@}

/*
    @name I2S MCLK clock rate value

    I2S MCLK  clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_I2S_MCLK).
*/
//@{
#define PLL_CLKSEL_I2S_MCLK_256FS      (0x00 << (PLL_CLKSEL_I2S_MCLK - PLL_CLKSEL_R4_OFFSET))     //< Select  I2S MCLK  clock as 256*FS
#define PLL_CLKSEL_I2S_MCLK_PLL7       (0x01 << (PLL_CLKSEL_I2S_MCLK - PLL_CLKSEL_R4_OFFSET))     //< Select  I2S MCLK  clock as PLL7
//@}

/*
    @name DAI  clock rate value

    DAI  clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_DAI).
*/
//@{
#define PLL_CLKSEL_DAI_PLL7    (0x00 << (PLL_CLKSEL_DAI - PLL_CLKSEL_R4_OFFSET))     //< Select DAI clock as PLL7
#define PLL_CLKSEL_DAI_IO      (0x01 << (PLL_CLKSEL_DAI - PLL_CLKSEL_R4_OFFSET))     //< Select DAI clock as IO PAD
//@}

/*
    @name   SENPHY Clock Phase Select

    SENPHY CK0 to CSI2 CK1 Clock Phase Select

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SENPHY_CSI2_CK1_PHASE).
*/
//@{
#define PLL_SENPHY_CSI2_CK1_PHASE_NORMAL   (0x00 << (PLL_CLKSEL_SENPHY_CSI2_CK1_PHASE - PLL_CLKSEL_R4_OFFSET))    //< Select SENPHY Clock Phase as Normal (non-invert)
#define PLL_SENPHY_CSI2_CK1_PHASE_INVERT   (0x01 << (PLL_CLKSEL_SENPHY_CSI2_CK1_PHASE - PLL_CLKSEL_R4_OFFSET))    //< Select SENPHY Clock Phase as Invert
//@}

/*
    @name   SENPHY Clock Phase Select

    SENPHY CK0 to CSI3 CK2 Clock Phase Select

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SENPHY_CSI3_CK2_PHASE).
*/
//@{
#define PLL_SENPHY_CSI3_CK2_PHASE_NORMAL   (0x00 << (PLL_CLKSEL_SENPHY_CSI3_CK2_PHASE - PLL_CLKSEL_R4_OFFSET))    //< Select SENPHY Clock Phase as Normal (non-invert)
#define PLL_SENPHY_CSI3_CK2_PHASE_INVERT   (0x01 << (PLL_CLKSEL_SENPHY_CSI3_CK2_PHASE - PLL_CLKSEL_R4_OFFSET))    //< Select SENPHY Clock Phase as Invert
//@}

/*
    @name   SENPHY Clock Phase Select

    SENPHY CK0 to CSI3 CK3 Clock Phase Select

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SENPHY_CSI3_CK3_PHASE).
*/
//@{
#define PLL_SENPHY_CSI3_CK3_PHASE_NORMAL   (0x00 << (PLL_CLKSEL_SENPHY_CSI3_CK3_PHASE - PLL_CLKSEL_R4_OFFSET))    //< Select SENPHY Clock Phase as Normal (non-invert)
#define PLL_SENPHY_CSI3_CK3_PHASE_INVERT   (0x01 << (PLL_CLKSEL_SENPHY_CSI3_CK3_PHASE - PLL_CLKSEL_R4_OFFSET))    //< Select SENPHY Clock Phase as Invert
//@}

/*
    @name   SENPHY Clock Phase Select

    SENPHY CK0 to CSI4 CK3 Clock Phase Select

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SENPHY_CSI4_CK3_PHASE).
*/
//@{
#define PLL_SENPHY_CSI4_CK3_PHASE_NORMAL   (0x00 << (PLL_CLKSEL_SENPHY_CSI4_CK3_PHASE - PLL_CLKSEL_R4_OFFSET))    //< Select SENPHY Clock Phase as Normal (non-invert)
#define PLL_SENPHY_CSI4_CK3_PHASE_INVERT   (0x01 << (PLL_CLKSEL_SENPHY_CSI4_CK3_PHASE - PLL_CLKSEL_R4_OFFSET))    //< Select SENPHY Clock Phase as Invert
//@}


/*
    @name CSI3 clock rate value

    CSI3 clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_CSI3).
*/
//@{
#define PLL_CLKSEL_CSI3_60  (0x00 << (PLL_CLKSEL_CSI3 - PLL_CLKSEL_R4_OFFSET))     //< Select CSI3 clock as 60Mhz
#define PLL_CLKSEL_CSI3_120 (0x01 << (PLL_CLKSEL_CSI3 - PLL_CLKSEL_R4_OFFSET))     //< Select CSI3 clock as 120Mhz
#define PLL_CLKSEL_CSI3_240 (0x02 << (PLL_CLKSEL_CSI3 - PLL_CLKSEL_R4_OFFSET))     //< Select CSI3 clock as 240Mhz
//@}

/*
    @name CSI4 clock rate value

    CSI4 clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_CSI4).
*/
//@{
#define PLL_CLKSEL_CSI4_60  (0x00 << (PLL_CLKSEL_CSI4 - PLL_CLKSEL_R4_OFFSET))     //< Select CSI4 clock as 60Mhz
#define PLL_CLKSEL_CSI4_120 (0x01 << (PLL_CLKSEL_CSI4 - PLL_CLKSEL_R4_OFFSET))     //< Select CSI4 clock as 120Mhz
#define PLL_CLKSEL_CSI4_240 (0x02 << (PLL_CLKSEL_CSI4 - PLL_CLKSEL_R4_OFFSET))     //< Select CSI4 clock as 240Mhz
//@}


//////////////////////////////////// R4 select done.

/*
    @name   SDIO3 clock rate value

    SDIO3 clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SDIO3).
*/
//@{
#define PLL_CLKSEL_SDIO3_192         (0x00 << (PLL_CLKSEL_SDIO3 - PLL_CLKSEL_R5_OFFSET)) //< Select SDIO3 clock as 192Mhz
#define PLL_CLKSEL_SDIO3_480         (0x01 << (PLL_CLKSEL_SDIO3 - PLL_CLKSEL_R5_OFFSET)) //< Select SDIO3 clock as 480Mhz
#define PLL_CLKSEL_SDIO3_PLL9        (0x02 << (PLL_CLKSEL_SDIO3 - PLL_CLKSEL_R5_OFFSET)) //< Select SDIO3 clock as PLL9
#define PLL_CLKSEL_SDIO3_PLL4        (0x03 << (PLL_CLKSEL_SDIO3 - PLL_CLKSEL_R5_OFFSET)) //< Select SDIO3 clock as PLL4
//@}


/*
    @name   UVCP clock rate value

    UVCP clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_UVCP).
*/
//@{
#define PLL_CLKSEL_UVCP_240     (0x00 << (PLL_CLKSEL_UVCP - PLL_CLKSEL_R5_OFFSET))  //< Select UVCP clock as 240Mhz
#define PLL_CLKSEL_UVCP_320     (0x01 << (PLL_CLKSEL_UVCP - PLL_CLKSEL_R5_OFFSET))  //< Select UVCP clock as 320Mhz
#define PLL_CLKSEL_UVCP_120     (0x02 << (PLL_CLKSEL_UVCP - PLL_CLKSEL_R5_OFFSET))  //< Select UVCP clock as 120Mhz
//@}

/*
    @name   TSE clock rate value

    TSE clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_TSMUX).
*/
//@{
#define PLL_CLKSEL_TSE_240          (0x00 << (PLL_CLKSEL_TSE - PLL_CLKSEL_R5_OFFSET))  //< Select TSE clock as 240Mhz
#define PLL_CLKSEL_TSE_320          (0x01 << (PLL_CLKSEL_TSE - PLL_CLKSEL_R5_OFFSET))  //< Select TSE clock as 320Mhz
#define PLL_CLKSEL_TSE_PLL9         (0x03 << (PLL_CLKSEL_TSE - PLL_CLKSEL_R5_OFFSET))  //< Select TSE clock as PLL9
//@}


/*
    @name   SP clock rate value

    SP clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SP).
*/
//@{
#define PLL_CLKSEL_SP_480            (0x00 << (PLL_CLKSEL_SP - PLL_CLKSEL_R5_OFFSET))  //< Select SP clock as 480Mhz
#define PLL_CLKSEL_SP_PLL4           (0x01 << (PLL_CLKSEL_SP - PLL_CLKSEL_R5_OFFSET))  //< Select SP clock as PLL4
#define PLL_CLKSEL_SP_PLL5           (0x02 << (PLL_CLKSEL_SP - PLL_CLKSEL_R5_OFFSET))  //< Select SP clock as PLL5
#define PLL_CLKSEL_SP_PLL6           (0x03 << (PLL_CLKSEL_SP - PLL_CLKSEL_R5_OFFSET))  //< Select SP clock as PLL6

//@}

/*
    @name   SP2 clock rate value

    SP2 clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SP2).
*/
//@{
#define PLL_CLKSEL_SP2_480           (0x00 << (PLL_CLKSEL_SP2 - PLL_CLKSEL_R5_OFFSET)) //< Select SP2 clock as 480Mhz
#define PLL_CLKSEL_SP2_PLL4          (0x01 << (PLL_CLKSEL_SP2 - PLL_CLKSEL_R5_OFFSET)) //< Select SP2 clock as PLL4
#define PLL_CLKSEL_SP2_PLL5          (0x02 << (PLL_CLKSEL_SP2 - PLL_CLKSEL_R5_OFFSET)) //< Select SP2 clock as PLL5
#define PLL_CLKSEL_SP2_PLL6          (0x03 << (PLL_CLKSEL_SP2 - PLL_CLKSEL_R5_OFFSET)) //< Select SP2 clock as PLL6
//@}

/*
    @name   HASH clock rate value

    HASH clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_HASH).
*/
//@{
#define PLL_CLKSEL_HASH_240    (0x00 << (PLL_CLKSEL_HASH - PLL_CLKSEL_R5_OFFSET))    //< Select Crypto clock as 240MHz
#define PLL_CLKSEL_HASH_320    (0x01 << (PLL_CLKSEL_HASH - PLL_CLKSEL_R5_OFFSET))    //< Select Crypto clock as 320Mhz
#define PLL_CLKSEL_HASH_PLL9   (0x03 << (PLL_CLKSEL_HASH - PLL_CLKSEL_R5_OFFSET))    //< Select Crypto clock as PLL9
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
    @name   ADC PowerDown clock rate value

    DC PowerDown clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_ADC_PD).
*/
//@{
#define PLL_CLKSEL_ADC_PD_RTC         (0x00 << (PLL_CLKSEL_ADC_PD - PLL_CLKSEL_R5_OFFSET)) //< Select ADC_PD clock source from RTC 32.768 KHz
#define PLL_CLKSEL_ADC_PD_OSC         (0x01 << (PLL_CLKSEL_ADC_PD - PLL_CLKSEL_R5_OFFSET)) //< Select ADC_PD clock source from OSC 12 MHz generated 32 KHz
//@}

/*
    @name   Ethernet PHY Reference clock rate value

    Ethernet PHY Reference clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_ETHPHY_CLKSRC).
*/
//@{
#define PLL_CLKSEL_ETHPHY_CLKSRC_PLL16         (0x00 << (PLL_CLKSEL_ETHPHY_CLKSRC - PLL_CLKSEL_R5_OFFSET))
#define PLL_CLKSEL_ETHPHY_CLKSRC_SPCLK         (0x01 << (PLL_CLKSEL_ETHPHY_CLKSRC - PLL_CLKSEL_R5_OFFSET))
#define PLL_CLKSEL_ETHPHY_CLKSRC_SP2CLK        (0x02 << (PLL_CLKSEL_ETHPHY_CLKSRC - PLL_CLKSEL_R5_OFFSET))
//@}

/*
    @name   NAND clock rate value

    NAND clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_NAND).
*/
//@{
#define PLL_CLKSEL_NAND_480         (0x00 << (PLL_CLKSEL_NAND - PLL_CLKSEL_R5_OFFSET)) //< Select NAND clock as 480Mhz
#define PLL_CLKSEL_NAND_PLL9        (0x01 << (PLL_CLKSEL_NAND - PLL_CLKSEL_R5_OFFSET)) //< Select NAND clock as PLL9
#define PLL_CLKSEL_NAND_320         (0x02 << (PLL_CLKSEL_NAND - PLL_CLKSEL_R5_OFFSET)) //< Select NAND clock as 320Mhz
#define PLL_CLKSEL_NAND_PLL4        (0x03 << (PLL_CLKSEL_NAND - PLL_CLKSEL_R5_OFFSET)) //< Select NAND clock as PLL4
//@}

/*
    @name   UART clock rate value

    UART clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_UART).
*/
//@{
#define PLL_CLKSEL_UART_24         (0x00 << (PLL_CLKSEL_UART - PLL_CLKSEL_R5_OFFSET)) //< Select UART clock as 24MHz
#define PLL_CLKSEL_UART_480        (0x01 << (PLL_CLKSEL_UART - PLL_CLKSEL_R5_OFFSET)) //< Select UART clock as 480MHz
//@}

//////////////////////////////////// R5 select done.

/*
    @name   SENPHY Clock Phase Select

    SENPHY CK0 to CSI CK0 Clock Phase Select

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SENPHY_CSI_CK0_PHASE).
*/
//@{
#define PLL_SENPHY_CSI_CK0_PHASE_NORMAL   (0x00 << (PLL_CLKSEL_SENPHY_CSI_CK0_PHASE - PLL_CLKSEL_R6_OFFSET))    //< Select SENPHY Clock Phase as Normal (non-invert)
#define PLL_SENPHY_CSI_CK0_PHASE_INVERT   (0x01 << (PLL_CLKSEL_SENPHY_CSI_CK0_PHASE - PLL_CLKSEL_R6_OFFSET))    //< Select SENPHY Clock Phase as Invert
//@}

/*
    @name   SENPHY Clock Phase Select

    SENPHY CK0 to CSI CK1 Clock Phase Select

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SENPHY_CSI_CK0_PHASE).
*/
//@{
#define PLL_SENPHY_CSI_CK1_PHASE_NORMAL   (0x00 << (PLL_CLKSEL_SENPHY_CSI_CK1_PHASE - PLL_CLKSEL_R6_OFFSET))    //< Select SENPHY Clock Phase as Normal (non-invert)
#define PLL_SENPHY_CSI_CK1_PHASE_INVERT   (0x01 << (PLL_CLKSEL_SENPHY_CSI_CK1_PHASE - PLL_CLKSEL_R6_OFFSET))    //< Select SENPHY Clock Phase as Invert
//@}

/*
    @name   SENPHY Clock Phase Select

    SENPHY CK0 to CSI CK2 Clock Phase Select

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SENPHY_CSI_CK2_PHASE).
*/
//@{
#define PLL_SENPHY_CSI_CK2_PHASE_NORMAL   (0x00 << (PLL_CLKSEL_SENPHY_CSI_CK2_PHASE - PLL_CLKSEL_R6_OFFSET))    //< Select SENPHY Clock Phase as Normal (non-invert)
#define PLL_SENPHY_CSI_CK2_PHASE_INVERT   (0x01 << (PLL_CLKSEL_SENPHY_CSI_CK2_PHASE - PLL_CLKSEL_R6_OFFSET))    //< Select SENPHY Clock Phase as Invert
//@}

/*
    @name   SENPHY Clock Phase Select

    SENPHY CK0 to CSI CK3 Clock Phase Select

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SENPHY_CSI_CK3_PHASE).
*/
//@{
#define PLL_SENPHY_CSI_CK3_PHASE_NORMAL   (0x00 << (PLL_CLKSEL_SENPHY_CSI_CK3_PHASE - PLL_CLKSEL_R6_OFFSET))    //< Select SENPHY Clock Phase as Normal (non-invert)
#define PLL_SENPHY_CSI_CK3_PHASE_INVERT   (0x01 << (PLL_CLKSEL_SENPHY_CSI_CK3_PHASE - PLL_CLKSEL_R6_OFFSET))    //< Select SENPHY Clock Phase as Invert
//@}

/*
    @name CSI3 PXCLK clock rate value

    CSI3 PXCLK clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_CSI3_PXCLK).
*/
//@{
#define PLL_CLKSEL_CSI3_PXCLK_SIE   (0x00 << (PLL_CLKSEL_CSI3_PXCLK - PLL_CLKSEL_R6_OFFSET))     //< Select CSI3 PXCLK clock as SIE3~5
#define PLL_CLKSEL_CSI3_PXCLK_VIE2  (0x01 << (PLL_CLKSEL_CSI3_PXCLK - PLL_CLKSEL_R6_OFFSET))     //< Select CSI3 PXCLK clock as VIE
//@}

/*
    @name   SIE2 CLK clock source value

    SIE2 CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE2_INT_CLKSRC).
*/
//@{
#define PLL_CLKSEL_SIE2_INT_CLKSRC_SIE2                                  (0x00 << (PLL_CLKSEL_SIE2_INT_CLKSRC - PLL_CLKSEL_R6_OFFSET))  //< Select SIE2 internal CLK source as SIE2 CLK
#define PLL_CLKSEL_SIE2_INT_CLKSRC_SIE1                                  (0x01 << (PLL_CLKSEL_SIE2_INT_CLKSRC - PLL_CLKSEL_R6_OFFSET))  //< Select SIE2 internal CLK source as SIE1 CLK
//@}


/*
    @name   SIE4 CLK clock source value

    SIE2 CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE4_INT_CLKSRC).
*/
//@{
#define PLL_CLKSEL_SIE4_INT_CLKSRC_SIE4                                  (0x00 << (PLL_CLKSEL_SIE4_INT_CLKSRC - PLL_CLKSEL_R6_OFFSET))  //< Select SIE2 internal CLK source as SIE2 CLK
#define PLL_CLKSEL_SIE4_INT_CLKSRC_SIE3                                  (0x01 << (PLL_CLKSEL_SIE4_INT_CLKSRC - PLL_CLKSEL_R6_OFFSET))  //< Select SIE2 internal CLK source as SIE1 CLK
//@}

//@}

/*
    @name   CONV CLK clock source value

    CONV CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_CONV).
*/
//@{
#define PLL_CLKSEL_CONV_PLL10                    (0x00 << (PLL_CLKSEL_CONV - PLL_CLKSEL_R6_OFFSET))  //< Select CONV CLK source as PLL10
#define PLL_CLKSEL_CONV_240                      (0x01 << (PLL_CLKSEL_CONV - PLL_CLKSEL_R6_OFFSET))  //< Select CONV CLK source as 240MHz
#define PLL_CLKSEL_CONV_320                      (0x02 << (PLL_CLKSEL_CONV - PLL_CLKSEL_R6_OFFSET))  //< Select CONV CLK source as 320MHz
#define PLL_CLKSEL_CONV_480                      (0x03 << (PLL_CLKSEL_CONV - PLL_CLKSEL_R6_OFFSET))  //< Select CONV CLK source as 480MHz
//@}


/*
    @name   LSU CLK clock source value

    LSU CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_LSU).
*/
//@{
#define PLL_CLKSEL_LSU_PLL10                    (0x00 << (PLL_CLKSEL_LSU - PLL_CLKSEL_R6_OFFSET))  //< Select LSU CLK source as PLL10
#define PLL_CLKSEL_LSU_240                      (0x01 << (PLL_CLKSEL_LSU - PLL_CLKSEL_R6_OFFSET))  //< Select LSU CLK source as 240MHz
#define PLL_CLKSEL_LSU_320                      (0x02 << (PLL_CLKSEL_LSU - PLL_CLKSEL_R6_OFFSET))  //< Select LSU CLK source as 320MHz
#define PLL_CLKSEL_LSU_480                      (0x03 << (PLL_CLKSEL_LSU - PLL_CLKSEL_R6_OFFSET))  //< Select LSU CLK source as 480MHz
//@}

/*
    @name   NUE30(CAL,ROU,UTIL) clock rate value

    NUE30 clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_NUE).
*/
//@{
#define PLL_CLKSEL_NUE30_PLL10      (0x00 << (PLL_CLKSEL_NUE30 - PLL_CLKSEL_R6_OFFSET)) //< Select NUE2 clock as PLL10
#define PLL_CLKSEL_NUE30_240        (0x01 << (PLL_CLKSEL_NUE30 - PLL_CLKSEL_R6_OFFSET)) //< Select NUE2 clock as 240MHz
#define PLL_CLKSEL_NUE30_320        (0x02 << (PLL_CLKSEL_NUE30 - PLL_CLKSEL_R6_OFFSET)) //< Select NUE2 clock as 320MHz
#define PLL_CLKSEL_NUE30_480        (0x03 << (PLL_CLKSEL_NUE30 - PLL_CLKSEL_R6_OFFSET)) //< Select NUE2 clock as 480MHz
//@}

/*
    @name PPU clock rate value

    PPU clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_PPU).
*/
//@{
#define PLL_CLKSEL_PPU_PLL10 (0x00 << (PLL_CLKSEL_PPU - PLL_CLKSEL_R6_OFFSET))     //< Select PPU clock as PLL10
#define PLL_CLKSEL_PPU_240   (0x01 << (PLL_CLKSEL_PPU - PLL_CLKSEL_R6_OFFSET))     //< Select PPU clock as 240MHz
#define PLL_CLKSEL_PPU_320   (0x02 << (PLL_CLKSEL_PPU - PLL_CLKSEL_R6_OFFSET))     //< Select PPU clock as 320MHz
#define PLL_CLKSEL_PPU_480   (0x03 << (PLL_CLKSEL_PPU - PLL_CLKSEL_R6_OFFSET))     //< Select PPU clock as 480MHz

//@}

/*
    @name POU clock rate value

    POU clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_POU).
*/
//@{
#define PLL_CLKSEL_POU_PLL10 (0x00 << (PLL_CLKSEL_POU - PLL_CLKSEL_R6_OFFSET))     //< Select POU clock as PLL10
#define PLL_CLKSEL_POU_240   (0x01 << (PLL_CLKSEL_POU - PLL_CLKSEL_R6_OFFSET))     //< Select POU clock as 240MHz
#define PLL_CLKSEL_POU_320   (0x02 << (PLL_CLKSEL_POU - PLL_CLKSEL_R6_OFFSET))     //< Select POU clock as 320MHz
#define PLL_CLKSEL_POU_480   (0x03 << (PLL_CLKSEL_POU - PLL_CLKSEL_R6_OFFSET))     //< Select POU clock as 480MHz

//@}


/*
    @name   TRKE clock rate value

    TRKE clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_TRKE).
*/
//@{
#define PLL_CLKSEL_TRKE_320        (0x00 << (PLL_CLKSEL_TRKE - PLL_CLKSEL_R6_OFFSET)) //< Select TRKE clock as 320MHz
#define PLL_CLKSEL_TRKE_240        (0x01 << (PLL_CLKSEL_TRKE - PLL_CLKSEL_R6_OFFSET)) //< Select TRKE clock as 240MHz
//@}

//////////////////////////////////// R6 select done.

/*
    @name   SIE4 CLK clock source value

    SIE4 CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE4_CLKSRC).
*/
//@{
#define PLL_CLKSEL_SIE4_CLKSRC_480       (0x00 << (PLL_CLKSEL_SIE4_CLKSRC - PLL_CLKSEL_R7_OFFSET))    //< Select SIE4 CLK clock source as 480 MHz
#define PLL_CLKSEL_SIE4_CLKSRC_PLL5      (0x02 << (PLL_CLKSEL_SIE4_CLKSRC - PLL_CLKSEL_R7_OFFSET))    //< Select SIE4 CLK clock source as PLL5
#define PLL_CLKSEL_SIE4_CLKSRC_PLL13     (0x03 << (PLL_CLKSEL_SIE4_CLKSRC - PLL_CLKSEL_R7_OFFSET))    //< Select SIE4 CLK clock source as PLL13
#define PLL_CLKSEL_SIE4_CLKSRC_PLL12     (0x04 << (PLL_CLKSEL_SIE4_CLKSRC - PLL_CLKSEL_R7_OFFSET))    //< Select SIE4 CLK clock source as PLL12
#define PLL_CLKSEL_SIE4_CLKSRC_320       (0x05 << (PLL_CLKSEL_SIE4_CLKSRC - PLL_CLKSEL_R7_OFFSET))    //< Select SIE4 CLK clock source as 192 MHz
#define PLL_CLKSEL_SIE4_CLKSRC_192       (0x06 << (PLL_CLKSEL_SIE4_CLKSRC - PLL_CLKSEL_R7_OFFSET))    //< Select SIE4 CLK clock source as 320Mhz
#define PLL_CLKSEL_SIE4_CLKSRC_PLL10     (0x07 << (PLL_CLKSEL_SIE4_CLKSRC - PLL_CLKSEL_R7_OFFSET))    //< Select SIE4 CLK clock source as PLL10
//@}


/*
    @name   SIE5 CLK clock source value

    SIE5 CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SIE5_CLKSRC).
*/
//@{
#define PLL_CLKSEL_SIE5_CLKSRC_480       (0x00 << (PLL_CLKSEL_SIE5_CLKSRC - PLL_CLKSEL_R7_OFFSET))    //< Select SIE5 CLK clock source as 480 MHz
#define PLL_CLKSEL_SIE5_CLKSRC_PLL5      (0x02 << (PLL_CLKSEL_SIE5_CLKSRC - PLL_CLKSEL_R7_OFFSET))    //< Select SIE5 CLK clock source as PLL5
#define PLL_CLKSEL_SIE5_CLKSRC_PLL13     (0x03 << (PLL_CLKSEL_SIE5_CLKSRC - PLL_CLKSEL_R7_OFFSET))    //< Select SIE5 CLK clock source as PLL13
#define PLL_CLKSEL_SIE5_CLKSRC_PLL12     (0x04 << (PLL_CLKSEL_SIE5_CLKSRC - PLL_CLKSEL_R7_OFFSET))    //< Select SIE5 CLK clock source as PLL12
#define PLL_CLKSEL_SIE5_CLKSRC_320       (0x05 << (PLL_CLKSEL_SIE5_CLKSRC - PLL_CLKSEL_R7_OFFSET))    //< Select SIE5 CLK clock source as 192 MHz
#define PLL_CLKSEL_SIE5_CLKSRC_192       (0x06 << (PLL_CLKSEL_SIE5_CLKSRC - PLL_CLKSEL_R7_OFFSET))    //< Select SIE5 CLK clock source as 320Mhz
#define PLL_CLKSEL_SIE5_CLKSRC_PLL10     (0x07 << (PLL_CLKSEL_SIE5_CLKSRC - PLL_CLKSEL_R7_OFFSET))    //< Select SIE5 CLK clock source as PLL10
//@}

/*
    @name   SENSOR TEMP RX CLK clock source value

    SENSOR TEMP RX clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_STSEN_RX_CLKSRC).
*/
//@{
#define PLL_CLKSEL_STSEN_RX_CLKSRC_PXCLK            (0x00 << (PLL_CLKSEL_STSEN_RX_CLKSRC - PLL_CLKSEL_R7_OFFSET))    //< Select ENSOR TEMP RX CLK clock source as PXCLK
#define PLL_CLKSEL_STSEN_RX_CLKSRC_SIE_MACLK3       (0x01 << (PLL_CLKSEL_STSEN_RX_CLKSRC - PLL_CLKSEL_R7_OFFSET))    //< Select SIE5 CLK clock source as SIE_MCLK3

//@}

/*
    @name   VIE PLL CLK clock source value

    VIE PLL CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_VIE_A_PXCLKSRC).
*/
//@{
#define PLL_CLKSEL_VIE_A_PXCLKSRC_APORT    (0x00 << (PLL_CLKSEL_VIE_A_PXCLKSRC - PLL_CLKSEL_R7_OFFSET)) //< Select VIE PX CLK source as A_PORT:S_GPIO1
#define PLL_CLKSEL_VIE_A_PXCLKSRC_BPORT    (0x01 << (PLL_CLKSEL_VIE_A_PXCLKSRC - PLL_CLKSEL_R7_OFFSET)) //< Select VIE PX CLK source as A_PORT:P_GPIO0
//@}

/*
    @name   VIE PLL CLK clock source value

    VIE PLL CLK clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_VIE_A_PXCLKSRC).
*/
//@{
#define PLL_CLKSEL_VIE_B_PXCLKSRC_APORT    (0x00 << (PLL_CLKSEL_VIE_B_PXCLKSRC - PLL_CLKSEL_R7_OFFSET)) //< Select VIE PX CLK source as B_PORT:S_GPIO7
#define PLL_CLKSEL_VIE_B_PXCLKSRC_BPORT    (0x01 << (PLL_CLKSEL_VIE_B_PXCLKSRC - PLL_CLKSEL_R7_OFFSET)) //< Select VIE PX CLK source as B_PORT:P_GPIO17
//@}



//////////////////////////////////////////////// start divider



/*
    @name   SIE4 CLK clock divider

    SIE4 CLK clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SIE4_CLKDIV).
*/
//@{
#define PLL_SIE4_CLKDIV(x)          ((x) << (PLL_CLKSEL_SIE4_CLKDIV - PLL_CLKSEL_R7_OFFSET))        //< Used for pll_set_clock_rate(PLL_CLKSEL_SIE4_CLKDIV)
//@}

/*
    @name   SIE5 CLK clock divider

    SIE5 CLK clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SIE5_CLKDIV).
*/
//@{
#define PLL_SIE5_CLKDIV(x)          ((x) << (PLL_CLKSEL_SIE5_CLKDIV - PLL_CLKSEL_R7_OFFSET))        //< Used for pll_set_clock_rate(PLL_CLKSEL_SIE5_CLKDIV)
//@}

//////////////////////////////////// R7 select done.

/*
    @name   SIE MCLK clock divider

    SIE MCLK clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLKDIV).
*/
//@{
#define PLL_SIE_MCLKDIV(x)          ((x) << (PLL_CLKSEL_SIE_MCLKDIV - PLL_CLKSEL_R8_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLKDIV)
//@}

/*
    @name   SIE MCLK2 clock divider

    SIE MCLK2 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK2DIV).
*/
//@{
#define PLL_SIE_MCLK2DIV(x)         ((x) << (PLL_CLKSEL_SIE_MCLK2DIV - PLL_CLKSEL_R8_OFFSET))   //< Used for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK2DIV)
//@}

/*
    @name   SIE CLK clock divider

    SIE CLK clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SIE_CLKDIV).
*/
//@{
#define PLL_SIE_CLKDIV(x)           ((x) << (PLL_CLKSEL_SIE_CLKDIV - PLL_CLKSEL_R8_OFFSET))         //< Used for pll_set_clock_rate(PLL_CLKSEL_SIE_CLKDIV)
//@}


/*
    @name   SIE2 CLK clock divider

    SIE2 CLK clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SIE2_CLKDIV).
*/
//@{
#define PLL_SIE2_CLKDIV(x)          ((x) << (PLL_CLKSEL_SIE2_CLKDIV - PLL_CLKSEL_R8_OFFSET))        //< Used for pll_set_clock_rate(PLL_CLKSEL_SIE2_CLKDIV)
//@}




//////////////////////////////////// R8 select done.

/*
    @name   IDE clock divider

    IDE clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_IDE_CLKDIV).
*/
//@{
#define PLL_IDE_CLKDIV(x)           ((x) << (PLL_CLKSEL_IDE_CLKDIV - PLL_CLKSEL_R9_OFFSET))     //< Used for pll_set_clock_rate(PLL_CLKSEL_IDE_CLKDIV)
//@}

/*
    @name   IDE Output Interface clock divider

    IDE Output Interface clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_IDE_OUTIF_CLKDIV).
*/
//@{
#define PLL_IDE_OUTIF_CLKDIV(x)          ((x) << (PLL_CLKSEL_IDE_OUTIF_CLKDIV - PLL_CLKSEL_R9_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_IDE_OUTIF_CLKDIV)
//@}

/*
    @name   SIE MCLK3 clock divider

    SIE MCLK3 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK3DIV).
*/
//@{
#define PLL_SIE_MCLK3DIV(x)         ((x) << (PLL_CLKSEL_SIE_MCLK3DIV - PLL_CLKSEL_R9_OFFSET))   //< Used for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK3DIV)
//@}

/*
    @name   TRNG clock divider

    TRNG clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_TRNG_CLKDIV).
*/
//@{
#define PLL_TRNG_CLKDIV(x)          ((x) << (PLL_CLKSEL_TRNG_CLKDIV - PLL_CLKSEL_R9_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_TRNG_CLKDIV)
//@}

//////////////////////////////////// R9 select done.

/*
    @name   SP clock divider

    Special clock divider

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SP_CLKDIV)
*/
//@{
#define PLL_SP_CLKDIV(x)            ((x) << (PLL_CLKSEL_SP_CLKDIV - PLL_CLKSEL_R10_OFFSET))     //< Used for pll_set_clock_rate(PLL_CLKSEL_SP_CLKDIV)
//@}

/*
    @name   SIE3 CLK clock divider

    SIE3 CLK clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SIE3_CLKDIV).
*/
//@{
#define PLL_SIE3_CLKDIV(x)          ((x) << (PLL_CLKSEL_SIE3_CLKDIV - PLL_CLKSEL_R10_OFFSET))        //< Used for pll_set_clock_rate(PLL_CLKSEL_SIE3_CLKDIV)
//@}

/*
    @name   DAI clock divider

    DAI clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_DAI_CLKDIV)
*/
//@{
#define PLL_DAI_CLKDIV(x)          ((x) << (PLL_CLKSEL_DAI_CLKDIV - PLL_CLKSEL_R10_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_DAI_CLKDIV)
#define PLL_ADO_CLKDIV PLL_DAI_CLKDIV
//@}

/*
    @name   DAI OSR clock divider

    DAI OSR clock divider

    @note This is for pll_set_clock_rate(PLL_CLKSEL_DAI_OSR_CLKDIV).
*/
//@{
#define PLL_DAI_OSR_CLKDIV(x)           ((x) << (PLL_CLKSEL_DAI_OSR_CLKDIV - PLL_CLKSEL_R10_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_DAI_OSR_CLKDIV)
#define PLL_ADO_OSR_CLKDIV PLL_DAI_OSR_CLKDIV
//@}


//////////////////////////////////// R10 select done.


/*
    @name   SDIO clock divider

    SDIO clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SDIO_CLKDIV)
*/
//@{
#define PLL_SDIO_CLKDIV(x)          ((x) << (PLL_CLKSEL_SDIO_CLKDIV - PLL_CLKSEL_R11_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_SDIO_CLKDIV)
//@}

/*
    @name   SDIO2 clock divider

    SDIO2 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SDIO2_CLKDIV)
*/
//@{
#define PLL_SDIO2_CLKDIV(x)         ((x) << (PLL_CLKSEL_SDIO2_CLKDIV - PLL_CLKSEL_R11_OFFSET))  //< This if for pll_set_clock_rate(PLL_CLKSEL_SDIO2_CLKDIV)
//@}

//////////////////////////////////// R11 select done.

/*
    @name   SDIO3 clock divider

    SDIO3 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SDIO3_CLKDIV)
*/
//@{
#define PLL_SDIO3_CLKDIV(x)         ((x) << (PLL_CLKSEL_SDIO3_CLKDIV - PLL_CLKSEL_R12_OFFSET))  //< This if for pll_set_clock_rate(PLL_CLKSEL_SDIO3_CLKDIV)
//@}

/*
    @name   NAND clock divider

    NAND clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_NAND_CLKDIV)
*/
//@{
#define PLL_NAND_CLKDIV(x)          ((x) << (PLL_CLKSEL_NAND_CLKDIV - PLL_CLKSEL_R12_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_NAND_CLKDIV)
//@}


/*
    @name   ETH PTP clock divider

    ETH PTP  clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_ETH_PTP_CLKDIV)
*/
//@{
#define PLL_ETH_PTP_CLKDIV(x)          ((x) << (PLL_CLKSEL_ETH_PTP_CLKDIV - PLL_CLKSEL_R12_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_ETH_PTP_CLKDIV)
//@}

/*
    @name   SP2 clock divider

    Special clock2 divider

    @note This is for pll_set_clock_rate(PLL_CLKSEL_SP2_CLKDIV)
*/
//@{
#define PLL_SP2_CLKDIV(x)           ((x) << (PLL_CLKSEL_SP2_CLKDIV - PLL_CLKSEL_R12_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_SP2_CLKDIV)
//@}

//////////////////////////////////// R12 select done.

/*
    @name   SPI clock divider

    SPI clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SPI_CLKDIV)
*/
//@{
#define PLL_SPI_CLKDIV(x)           ((x) << (PLL_CLKSEL_SPI_CLKDIV - PLL_CLKSEL_R13_OFFSET))    //< This if for pll_set_clock_rate(PLL_CLKSEL_SPI_CLKDIV)
//@}

/*
    @name   SPI2 clock divider

    SPI2 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SPI2_CLKDIV)
*/
//@{
#define PLL_SPI2_CLKDIV(x)          ((x) << (PLL_CLKSEL_SPI2_CLKDIV - PLL_CLKSEL_R13_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_SPI2_CLKDIV)
//@}

//////////////////////////////////// R13 select done.

/*
    @name   SPI3 clock divider

    SPI3 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SPI3_CLKDIV)
*/
//@{
#define PLL_SPI3_CLKDIV(x)          ((x) << (PLL_CLKSEL_SPI3_CLKDIV - PLL_CLKSEL_R14_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_SPI3_CLKDIV)
//@}

/*
    @name   SPI4 clock divider

    SPI4 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SPI4_CLKDIV)
*/
//@{
#define PLL_SPI4_CLKDIV(x)          ((x) << (PLL_CLKSEL_SPI4_CLKDIV - PLL_CLKSEL_R14_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_SPI4_CLKDIV)
//@}

//////////////////////////////////// R14 select done.


/*
    @name   UART2 clock divider

    UART2 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_UART2_CLKDIV)
*/
//@{
#define PLL_UART2_CLKDIV(x)          ((x) << (PLL_CLKSEL_UART2_CLKDIV - PLL_CLKSEL_R15_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_UART2_CLKDIV)
//@}

/*
    @name   UART3 clock divider

    UART3 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_UART3_CLKDIV)
*/
//@{
#define PLL_UART3_CLKDIV(x)          ((x) << (PLL_CLKSEL_UART3_CLKDIV - PLL_CLKSEL_R15_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_UART3_CLKDIV)
//@}

/*
    @name   UART4 clock divider

    UART4 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_UART4_CLKDIV)
*/
//@{
#define PLL_UART4_CLKDIV(x)          ((x) << (PLL_CLKSEL_UART4_CLKDIV - PLL_CLKSEL_R15_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_UART4_CLKDIV)
//@}


//////////////////////////////////// R13 select done.

/*
    @name   UART5 clock divider

    UART5 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_UART5_CLKDIV)
*/
//@{
#define PLL_UART5_CLKDIV(x)          ((x) << (PLL_CLKSEL_UART5_CLKDIV - PLL_CLKSEL_R15_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_UART5_CLKDIV)
//@}

//////////////////////////////////// R15 select done.

/*
    @name   PWM0-3 clock divider

    PWM0-3 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_PWM0_3_CLKDIV).
*/
//@{
#define PLL_PWM0_3_CLKDIV(x)          ((x) << (PLL_CLKSEL_PWM0_3_CLKDIV - PLL_CLKSEL_R16_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_PWM0_3_CLKDIV)
//@}


/*
    @name   PWM4-7 clock divider

    PWM4-7 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_PWM4_7_CLKDIV).
*/
//@{
#define PLL_PWM4_7_CLKDIV(x)          ((x) << (PLL_CLKSEL_PWM4_7_CLKDIV - PLL_CLKSEL_R16_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_PWM4_7_CLKDIV)
//@}

//////////////////////////////////// R16 select done.

/*
    @name   PWM8 clock divider

    PWM8 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_PWM8_CLKDIV).
*/
//@{
#define PLL_PWM8_CLKDIV(x)          ((x) << (PLL_CLKSEL_PWM8_CLKDIV - PLL_CLKSEL_R17_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_PWM8_CLKDIV)
//@}


/*
    @name   PWM9 clock divider

    PWM9 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_PWM9_CLKDIV).
*/
//@{
#define PLL_PWM9_CLKDIV(x)          ((x) << (PLL_CLKSEL_PWM9_CLKDIV - PLL_CLKSEL_R17_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_PWM9_CLKDIV)
//@}

//////////////////////////////////// R17 select done.

/*
    @name   PWM10 clock divider

    PWM10 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_PWM10_CLKDIV).
*/
//@{
#define PLL_PWM10_CLKDIV(x)          ((x) << (PLL_CLKSEL_PWM10_CLKDIV - PLL_CLKSEL_R18_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_PWM10_CLKDIV)
//@}


/*
    @name   PWM11 clock divider

    PWM11 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_PWM11_CLKDIV).
*/
//@{
#define PLL_PWM11_CLKDIV(x)          ((x) << (PLL_CLKSEL_PWM11_CLKDIV - PLL_CLKSEL_R18_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_PWM11_CLKDIV)
//@}

//////////////////////////////////// R18 select done.

/*
    @name   UART clock divider

    UART clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_UART_CLKDIV).
*/
//@{
#define PLL_UART_CLKDIV(x)          ((x) << (PLL_CLKSEL_UART_CLKDIV - PLL_CLKSEL_R19_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_UART_CLKDIV)
//@}

//////////////////////////////////// R19 select done.

/*
    @name   TRNG RO clock divider

    TRNG RO clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_TRNG_RO_CLKDIV).
*/
//@{
#define PLL_TRNG_RO_CLKDIV(x)          ((x) << (PLL_CLKSEL_TRNG_RO_CLKDIV - PLL_CLKSEL_R20_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_TRNG_RO_CLKDIV)
//@}


/*
    @name   UART6 clock divider

    UART6 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_UART6_CLKDIV)
*/
//@{
#define PLL_UART6_CLKDIV(x)          ((x) << (PLL_CLKSEL_UART6_CLKDIV - PLL_CLKSEL_R20_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_UART6_CLKDIV)
//@}

/*
    @name   DAI M clock divider

    DAI M clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_DAI_MCLKDIV)
*/
//@{
#define PLL_DAI_MCLKDIV(x)          ((x) << (PLL_CLKSEL_DAI_MCLKDIV - PLL_CLKSEL_R20_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_DAI_MCLKDIV)
#define PLL_ADO_MCLKDIV PLL_DAI_MCLKDIV
//@}



//////////////////////////////////// R20 select done.

/*
    @name   SPI5 clock divider

    SPI5 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SPI5_CLKDIV)
*/
//@{
#define PLL_SPI5_CLKDIV(x)          ((x) << (PLL_CLKSEL_SPI5_CLKDIV - PLL_CLKSEL_R21_OFFSET))   //< This if for pll_set_clock_rate(PLL_CLKSEL_SPI5_CLKDIV)
//@}

/*
    @name   MI clock divider

    MI clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_MI_CLKDIV)
*/
//@{
#define PLL_MI_CLKDIV(x)         ((x) << (PLL_CLKSEL_MI_CLKDIV - PLL_CLKSEL_R21_OFFSET))  //< This if for pll_set_clock_rate(PLL_CLKSEL_MI_CLKDIV)
//@}


/*
    @name   VIE  clock divider

    VIE  clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_VIE_CLKDIV).
*/
//@{
#define PLL_VIE_CLKDIV(x)         ((x) << (PLL_CLKSEL_VIE_CLKDIV - PLL_CLKSEL_R21_OFFSET))   //< Used for pll_set_clock_rate(PLL_CLKSEL_VIE_CLKDIV)
//@}

//////////////////////////////////// R21 select done.


/*
    @name   SIE MCLK4 clock divider

    SIE MCLK4 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK4DIV).
*/
//@{
#define PLL_SIE_MCLK4DIV(x)         ((x) << (PLL_CLKSEL_SIE_MCLK4DIV - PLL_CLKSEL_R22_OFFSET))   //< Used for pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK4DIV)
//@}


//////////////////////////////////// R22 select done.


/*
    @name STBC APB clock rate value

    STBC APB clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_STBC_APB).
*/
//@{
#define PLL_CLKSEL_STBC_APB_48      (0x00 << (PLL_CLKSEL_STBC_APB - PLL_CLKSEL_R23_OFFSET))     //< Select STBC APB clock as 48MHz
#define PLL_CLKSEL_STBC_APB_60      (0x01 << (PLL_CLKSEL_STBC_APB - PLL_CLKSEL_R23_OFFSET))     //< Select STBC APB clock as 60MHz
#define PLL_CLKSEL_STBC_APB_80      (0x02 << (PLL_CLKSEL_STBC_APB - PLL_CLKSEL_R23_OFFSET))     //< Select STBC APB clock as 80MHz
#define PLL_CLKSEL_STBC_APB_120     (0x03 << (PLL_CLKSEL_STBC_APB - PLL_CLKSEL_R23_OFFSET))     //< Select STBC APB clock as 120MHz

/*
    @name STBC RTC 32K clock rate value

    STBC RTC 32K clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_STBC_RTC_32K).
*/
//@{
#define PLL_CLKSEL_STBC_RTC_32K_RTC         (0x00 << (PLL_CLKSEL_STBC_RTC_32K - PLL_CLKSEL_R23_OFFSET))     //< Select STBC RTC 32K  clock as RTC
#define PLL_CLKSEL_STBC_RTC_32K_RINGOSC     (0x01 << (PLL_CLKSEL_STBC_RTC_32K - PLL_CLKSEL_R23_OFFSET))     //< Select STBC RTC 32K  clock as RING OSC


/*
    @name STBC MCU clock rate value

    STBC MCU clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_STBC_MCU).
*/
//@{
#define PLL_CLKSEL_STBC_MCU_120         (0x00 << (PLL_CLKSEL_STBC_MCU - PLL_CLKSEL_R23_OFFSET))     //< Select STBC MCU  clock as 120MHz
#define PLL_CLKSEL_STBC_MCU_32K         (0x01 << (PLL_CLKSEL_STBC_MCU - PLL_CLKSEL_R23_OFFSET))     //< Select STBC MCU  clock as 32KHZ


/*
    @name STBC USB30PHY clock rate value

    STBC USB30PHY clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_STBC_USB30PHY).
*/
//@{
#define PLL_CLKSEL_STBC_USB30PHY_PLL14      (0x00 << (PLL_CLKSEL_STBC_USB30PHY - PLL_CLKSEL_R23_OFFSET))     //< Select STBC USB30 PHY  clock as PLL14
#define PLL_CLKSEL_STBC_USB30PHY_ETH_PHY    (0x01 << (PLL_CLKSEL_STBC_USB30PHY - PLL_CLKSEL_R23_OFFSET))     //< Select STBC USB30 PHY  clock as ETH PHY



//////////////////////////////////// R23 select done.

/*
    @name STBC DSI LP clock rate value

    STBC DSI LP clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_STBC_DSI_LP).
*/
//@{
#define PLL_CLKSEL_STBC_DSI_LP_60       (0x00 << (PLL_CLKSEL_STBC_DSI_LP - PLL_CLKSEL_R24_OFFSET))     //< Select STBC DSI LP  clock as 60MHz
#define PLL_CLKSEL_STBC_DSI_LP_80       (0x01 << (PLL_CLKSEL_STBC_DSI_LP - PLL_CLKSEL_R24_OFFSET))     //< Select STBC DSI LP  clock as 80MHz
#define PLL_CLKSEL_STBC_DSI_LP_120      (0x02 << (PLL_CLKSEL_STBC_DSI_LP - PLL_CLKSEL_R24_OFFSET))     //< Select STBC DSI LP  clock as 120MHz


/*
    @name STBC CSI TX LP clock rate value

    STBC CSI TX LP clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_STBC_CSI_TX_LP).
*/
//@{
#define PLL_CLKSEL_STBC_CSI_TX_LP_60        (0x00 << (PLL_CLKSEL_STBC_CSI_TX_LP - PLL_CLKSEL_R24_OFFSET))     //< Select STBC CSI TX LP  clock as 60MHz
#define PLL_CLKSEL_STBC_CSI_TX_LP_80        (0x01 << (PLL_CLKSEL_STBC_CSI_TX_LP - PLL_CLKSEL_R24_OFFSET))     //< Select STBC CSI TX LP  clock as 80MHz
#define PLL_CLKSEL_STBC_CSI_TX_LP_120       (0x02 << (PLL_CLKSEL_STBC_CSI_TX_LP - PLL_CLKSEL_R24_OFFSET))     //< Select STBC CSI TX LP  clock as 120MHz


/*
    @name STBC USB3 SUSP clock rate value

    STBC USB3 SUSP clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_STBC_USB3_SUSP).
*/
//@{
#define PLL_CLKSEL_STBC_USB3_SUSP_OSC       (0x00 << (PLL_CLKSEL_STBC_USB3_SUSP - PLL_CLKSEL_R24_OFFSET))     //< Select STBC USB3 SUSP  clock as OSC
#define PLL_CLKSEL_STBC_USB3_SUSP_12M       (0x01 << (PLL_CLKSEL_STBC_USB3_SUSP - PLL_CLKSEL_R24_OFFSET))     //< Select STBC USB3 SUSP  clock as 12M

/*
    @name STBC USB3 PDN CLK12M clock rate value

    STBC USB3 PDN CLK12M clock rate value

    @note This is used for pll_set_clock_rate(PLL_CLKSEL_STBC_USB3_PDN).
*/
//@{
#define PLL_CLKSEL_STBC_USB3_PDN_RO12M      (0x00 << (PLL_CLKSEL_STBC_USB3_PDN - PLL_CLKSEL_R24_OFFSET))     //< Select STBC USB3 PDN CLK12M  clock as ro_clk 12m
#define PLL_CLKSEL_STBC_USB3_PDN_RTC        (0x01 << (PLL_CLKSEL_STBC_USB3_PDN - PLL_CLKSEL_R24_OFFSET))     //< Select STBC USB3 PDN CLK12M  clock as RTC




//////////////////////////////////// R24 select done.


/*
    @name  STBC RO 32K DIV  clock divider

    STBC RO 32K DIV  clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_STBC_RO32K_CLKDIV).
*/
//@{
#define PLL_STBC_RO32K_CLKDIV(x)         ((x) << (PLL_CLKSEL_STBC_RO32K_CLKDIV - PLL_CLKSEL_R25_OFFSET))   //< Used for pll_set_clock_rate(PLL_STBC_RO32K_CLKDIV)
//@}

/*
    @name  STBC RO 32K DIV TESTEN clock divider

    STBC RO 32K DIV TESTEN  clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_STBC_RO32K_DIV_TESTEN).
*/
//@{
#define PLL_STBC_RO32K_DIV_TESTEN(x)         ((x) << (PLL_CLKSEL_STBC_RO32K_DIV_TESTEN - PLL_CLKSEL_R25_OFFSET))   //< Used for pll_set_clock_rate(PLL_STBC_RO32K_DIV_TESTEN)
//@}

//////////////////////////////////// R25 select done.

//////////////////////////////////////////////////////////////////////////////////////////////////////////


/*
    System Reset ID

    This is for pll_enable_system_reset() and pll_disable_system_reset().
*/
typedef enum {

	SIE_RSTN                = 4,            //< Reset SIE controller
	SIE2_RSTN,              /*5*/           //< Reset SIE2 controller
	TGE_RSTN,               /*6*/           //< Reset TGE controller
	IPE_RSTN,               /*7*/           //< Reset IPE controller
	IME_RSTN                = 9,            //< Reset IME controller
	ISE_RSTN                = 11,           //< Reset ISE controller
	IVE_RSTN,               /*12*/          //< Reset IVE controller
	IFE_RSTN,               /*13*/          //< Reset IFE controller
	IDE_RSTN                = 16,           //< Reset IDE controller
	SIE4_RSTN               = 18,           //< Reset SIE4 controller
	SIE5_RSTN,              /*19*/          //< Reset SIE5 controller
	VPE_RSTN                = 21,           //< Reset VPE controller
	PRE_RSTN,               /*22*/      	//< Reset Reset PRE controller
	CRYPTO_RSTN             = 23,           //< Reset CRYPTO controller
	VENC_RSTN,              /*24*/          //< Reset VENC controller
	JPEG_RSTN               = 26,           //< Reset JPEG controller
	GRAPH_RSTN,             /*27*/          //< Reset Graphic controller
	GRAPH2_RSTN,            /*28*/          //< Reset Graphic2 controller
	DAI_RSTN,               /*29*/          //< Reset DAI controller
	EAC_RSTN,               /*29*/          //< Reset EAC controller

	NAND_RSTN               = 32,           //< Reset NAND controller
	MAU_RSTN,               /*32+1*/        //< Reset MAU controller
	SDIO_RSTN,              /*32+2*/        //< Reset SDIO controller
	SDIO2_RSTN,             /*32+3*/        //< Reset SDIO2 controller
	I2C_RSTN,               /*32+4*/        //< Reset I2C controller
	I2C2_RSTN,              /*32+5*/        //< Reset I2C2 controller
	SPI_RSTN,               /*32+6*/        //< Reset SPI controller
	SPI2_RSTN,              /*32+7*/        //< Reset SPI2 controller
	SPI3_RSTN,              /*32+8*/        //< Reset SPI3 controller
	SIF_RSTN,               /*32+9*/        //< Reset SIF controller
	UART_RSTN,              /*32+10*/       //< Reset UART controller
	UART2_RSTN,             /*32+11*/       //< Reset UART2 controller
	REMOTE_RSTN,            /*32+12*/       //< Reset REMOTE controller
	ADC_RSTN,               /*32+13*/       //< Reset ADC controller
	SDIO3_RSTN,             /*32+14*/       //< Reset SDIO3 controller
	PWBC_RSTN,              /*32+15*/       //< Reset PWBC controller
	TMR_RSTN                = 32 + 18,      //< Reset TIMER controller
	I2C4_RSTN               = 32 + 20,      //< Reset I2C4 controller
	I2C5_RSTN,              /*32+21*/       //< Reset I2C5 controller
	UART3_RSTN,             /*32+22*/       //< Reset UART3 controller
	INTC_RSTN               = 32 + 24,      //< Reset INTC controller
	GPIO_RSTN,              /*32+25*/       //< Reset GPIO controller
	PAD_RSTN,               /*32+26*/       //< Reset PAD controller
	TOP_RSTN,               /*32+27*/       //< Reset TOP controller
	EFUSE_RSTN,             /*32+28*/       //< Reset EFUSE controller
	ETH_RSTN,               /*32+29*/       //< Reset ETHERNET controller
	ETH_GLUE_RSTN,          /*32+30*/       //< Reset ETH Glue
	I2C3_RSTN,              /*32+31*/       //< Reset I2C3 controller

	CSI_RSTN                = 64 + 1,       //< Reset CSI controller
	CSI2_RSTN,              /*64+2*/        //< Reset CSI2 controller
	UART4_RSTN              = 64 + 4,       //< Reset UART4 controller
	UART5_RSTN,             /*64+5*/        //< Reset UART5 controller
	UART6_RSTN,             /*64+6*/        //< Reset UART6 controller
	PWM_RSTN                = 64 + 8,       //< Reset PWM controller
	SIE3_RSTN,              /*64+9*/        //< Reset SIE3 controller
	SPI4_RSTN               = 64 + 11,      //< Reset SPI4 controller
	SPI5_RSTN,              /*64+12*/       //< Reset SPI5 controller
	TSE_RSTN,               /*64+13*/       //< Reset TSE controller
	NUE2_RSTN               = 64 + 15,      //< Reset NUE2 controller
	MDBC_RSTN,              /*64+16*/       //< Reset MDBC controller
	SDP_RSTN                = 64 + 20,      //< Reset SDP controller
	DRTC_RSTN               = 64 + 22,      //< Reset DRTC controller
	SENPHY_RSTN             = 64 + 24,      //< Reset SENPHY controller
	TRNG_RSTN,              /*64+25*/       //< Reset TRNG controller
	RSA_RSTN,               /*64+26*/       //< Reset RSA controller
	HASH_RSTN,              /*64+27*/       //< Reset HASH controller
	DDRPHY_RSTN,            /*64+28*/       //< Reset DDRPHY
	ETH_PHY_RSTN            = 64 + 30,      //< Reset ETH PHY module
	ETH_PHY_HI_RSTN,        /*64+31*/       //< Reset ETH PHY reg

	MAU_RING_RSTN           = 96 + 1,       //< Reset MAU RING
	HRTMR_RSTN              = 96 + 3,       //< Reset HRTIMER controller
	HRTMR2_RSTN,            /*96+4*/        //< Reset HRTIMER2 controller
	HRTMR3_RSTN,            /*96+5*/        //< Reset HRTIMER3 controller
	HRTMR4_RSTN,            /*96+6*/        //< Reset HRTIMER4 controller
	HRTMR5_RSTN,            /*96+7*/        //< Reset HRTIMER5 controller
	HRTMR6_RSTN,            /*96+8*/        //< Reset HRTIMER6 controller
	HRTMR7_RSTN,            /*96+9*/        //< Reset HRTIMER7 controller
	HRTMR8_RSTN,            /*96+10*/       //< Reset HRTIMER8 controller
	HRTMR9_RSTN,            /*96+11*/       //< Reset HRTIMER9 controller
	HRTMR10_RSTN,           /*96+12*/       //< Reset HRTIMER10 controller
	DSI_PROT_RSTN,          /*96+13*/       //< Reset DSI Protect controller
	MI_RSTN,                /*96+14*/       //< Reset MI controller
	HWCOPY_RSTN             = 96 + 19,      //< Reset HWCOPY controller
	GRAPH3_RSTN,            /*96+20*/       //< Reset Graphic3 controller
	VIE_RSTN,               /*96+21*/       //< Reset VIE controller
	CONV_RSTN,              /*96+22*/       //< Reset CONV controller
	JMISP_RSTN,             /*96+23*/       //< Reset JMISP controller
	JM_RSTN,                /*96+24*/       //< Reset JM controller
	LSU_RSTN                = 96 + 26,      //< Reset LSU controller
	NUE30_RSTN,             /*96+27*/       //< Reset NUE30 controller
	PPU_RSTN,               /*96+28*/       //< Reset PPU controller
	POU_RSTN,               /*96+29*/       //< Reset ROU controller
	TRKE_RSTN               = 96 + 30,      //< Reset TRKE controller

	UVCP_RSTN               = 128,          //< Reset UVCP controller
	HVYLD_RSTN              = 128 + 2,      //< Reset Heavyload controller
	CSI3_RSTN,              /*128+3*/       //< Reset CSI3 controller
	CSI4_RSTN,              /*128+4*/       //< Reset CSI4 controller
	ECDSA_RSTN,             /*128+5*/       //< Reset ECDSA controller
	HVYLD2_RSTN,            /*128+6*/       //< Reset Heavyload2 controller
	HVYLD3_RSTN             = 128 + 8,      //< Reset Heavyload3 controller
	TCM_RSTN                = 128 + 11,     //< Reset TCM controller
	TCM2_RSTN,              /*128+12*/      //< Reset TCM2 controller
	SN1_IO_SMP_RSTN         = 128 + 15,     //< Reset Reset SN1 IO sample controller
	SN2_IO_SMP_RSTN,        /*128+16*/      //< Reset Reset SN2 IO sample controller
	JPEG_WRAP_RSTN,         /*128+17*/      //< Reset JPEG wraper controller
	DRE_RSTN,               /*128+18*/      //< Reset DRE controller
	SN3_IO_SMP_RSTN,        /*128+19*/      //< Reset Reset SN3 IO sample controller
	STSEN_IO_IN_SMP_RSTN,   /*128+20*/      //< Reset Reset SIE Temp IO IN sample
	STSEN_IO_OUT_SMP_RSTN,  /*128+21*/      //< Reset Reset SIE Temp IO OUT sample

	//STBC RESET
	TOP_LVD_RSTN            = 17301504,//< Reset Reset TOP LVD(0x21_0090-90)/4*32
	MCU_RSTN                = 17301504 + 4, //< Reset Reset MCU
	CC_RSTN                 = 17301504 + 5, //< Reset Reset CC
	RTC_RSTN                = 17301504 + 8, //< Reset Reset RTC
	WDT_RSTN                = 17301504 + 9, //< Reset Reset WDT
	DSI_RSTN                = 17301504 + 12, //< Reset Reset DSI
	DSI_PHY_RSTN            = 17301504 + 13, //< Reset Reset DSI PHY
	CSI_TX_RSTN             = 17301504 + 14, //< Reset Reset CSI TX
	USB3_RSTN               = 17301504 + 15, //< Reset Reset USB3
	USB3_A_RSTN             = 17301504 + 16, //< Reset Reset USB3 A
	USB3_GLUE_RSTN          = 17301504 + 17, //< Reset Reset USB3 GLUE
	USB3_PHY_HI_RSTN        = 17301504 + 18, //< Reset Reset USB3 PHY HI
	USB2_PHY_PORN_RSTN      = 17301504 + 19, //< Reset Reset USB2 PHY PORN
	USB2_PHY_SIDQ_RSTN      = 17301504 + 20, //< Reset Reset USB2 SIDQ

	ENUM_DUMMY4WORD(CG_RSTN)
} CG_RSTN;





typedef enum {
	MIPI_LVDS_RSTN,       //< Reset MIPI_LVDS controller
	MIPI_LVDS2_RSTN,        /*64+ 2*/       //< Reset MIPI_LVDS2 controller

	AXI_BUS_RSTN,                            //< Reset AXI BUS
	AXI_BUS1_RSTN,          /*1*/           //< Reset AXI BUS1
	AXI_BUS2_RSTN,          /*2*/           //< Reset AXI BUS2
	AXI_BUS3_RSTN,          /*3*/           //< Reset AXI BUS3
	AXI_BUS4_RSTN,          /*4*/           //< Reset AXI BUS4
	AXI_BUS5_RSTN,          /*5*/           //< Reset AXI BUS5
	AXI_BUS6_RSTN,          /*6*/           //< Reset AXI BUS6
	AXI_BUS7_RSTN,          /*7*/           //< Reset AXI BUS7
	DDRPHY_HI_RSTN,         /*9*/           //< Reset DDRPHY HI
	JPGE2_WRAP_RSTN,        /*11*/          //< Reset JPEG2 wraper controller
	SIE6_RSTN,              /*19*/          //< Reset SIE6 controller
	GPENC_RSTN,             /*20*/          //< Reset GPENC controller
	GPENC2_RSTN,            /*21*/          //< Reset GPENC2 controller
	VIE2_RSTN,              /*25*/          //< Reset VIE2 controller
	DIS_RSTN,               /*26*/          //< Reset DIS controller
	VTRC_RSTN,              /*32+1*/        //< Reset VTRC controller
	DCE_RSTN,               /*32+3*/        //< Reset DCE controller
	IDE2_RSTN,              /*32+6*/        //< Reset IDE2 controller
	UVCP2_RSTN,             /*32+7*/        //< Reset UVCP2 controller
	HDMITX_RSTN,            /*32+14*/       //< Reset HDMITX controller
	NUE_RSTN,               /*32+15*/       //< Reset NUE controller
	SATA_RSTN,              /*32+19*/       //< Reset SATA controller
	JPEG_LITE_RSTN,         /*32+21*/       //< Reset JPEG Lite controller
	JPEG2_RSTN,             /*32+22*/       //< Reset JPEG2 controller
	DSP_RSTN,               /*32+23*/       //< Reset DSP controller
	MAU_APB_RSTN,           /*32+24*/       //< Reset MAU APB controller
	USB2_RSTN,              /*32+25*/       //< Reset USB2 controller
	CNN_RSTN,               /*32+28*/       //< Reset CNN controller
	CNN2_RSTN,              /*32+29*/       //< Reset CNN2 controller
	VPEL_RSTN,              /*32+31*/       //< Reset VPE-Lite controller
	I2C6_RSTN,              /*64+5*/        //< Reset I2C6 controller
	I2C7_RSTN,              /*64+6*/        //< Reset I2C7 controller
	I2C8_RSTN,              /*64+7*/        //< Reset I2C8 controller
	I2C9_RSTN,              /*64+8*/        //< Reset I2C9 controller
	I2C10_RSTN,             /*64+9*/        //< Reset I2C10 controller
	I2C11_RSTN,             /*64+10*/       //< Reset I2C11 controller
	TMR2_RSTN,              /*64+13*/       //< Reset Timer2 controller
	TMR3_RSTN,              /*64+14*/       //< Reset Timer3 controller
	ADC_TSEN_RSTN,          /*64+28*/       //< Reset ADC T-sensor controller
	USB3_PHY_HI_RSTB,       /*96+5*/        //< Reset USB3 PHY
	USB2_PHY_SIDDQ,         /*96+6*/        //< Reset USB2 PHY SIDDQ
	USB2_PHY_POR,           /*96+7*/        //< Reset USB2 PHY POR
	MSI_INTC_RSTN           = 96 + 11,      //< Reset MSI_INTC
	DAI2_RSTN,              /*96+17*/       //< Reset DAI2 controller
	PMC_RSTN                = 96 + 20,      //< Reset PMC controller
	UART7_RSTN,             /*96+22*/       //< Reset UART7 controller
	UART8_RSTN,             /*96+23*/       //< Reset UART8 controller
	UART9_RSTN,             /*96+24*/       //< Reset UART9 controller
	CSI5_RSTN,              /*96+29*/       //< Reset CSI5 controller
	SDE_RSTN,               /*96+30*/       //< Reset SDE controller
	VDEC_RSTN                = 128,         //< Reset VDEC controller
	JPEG_LITE_WRAP_RSTN,    /*128+1*/       //< Reset JPEG Lite wraper controller
	SENPHY_IF_RSTN           = 128 + 8,     //< Reset senphy interface
	SENPHY2_IF_RSTN,        /*128+9*/       //< Reset senphy2 interface
	SENPHY3_IF_RSTN,        /*128+10*/      //< Reset senphy3 interface



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

	SIECLK_FREQ,            ///< SIE    CLK freq Select ID
	SIE2CLK_FREQ,           ///< SIE2   CLK freq Select ID
	SIE3CLK_FREQ,           ///< SIE3   CLK freq Select ID
	SIE4CLK_FREQ,           ///< SIE4   CLK freq Select ID

	SIE5CLK_FREQ,           ///< SIE5   CLK freq Select ID
	VIECLK_FREQ,            ///< VIE    CLK freq Select ID
	IDECLK_FREQ,            ///< IDE    CLK freq Select ID
	SPCLK_FREQ,             ///< SP     CLK Select ID

	SPCLK2_FREQ,            ///< SP2    CLK Select ID
	ADOCLK_FREQ,            ///< ADO    CLK Select ID
	ADOOSRCLK_FREQ,         ///< ADO OSRCLK Select ID
	SDIOCLK_FREQ,           ///< SDIO   CLK Select ID

	SDIO2CLK_FREQ,          ///< SDIO2  CLK Select ID
	SDIO3CLK_FREQ,          ///< SDIO3  CLK Select ID
	NANDCLK_FREQ,
	SPICLK_FREQ,            ///< SPI    CLK Select ID
	SPI2CLK_FREQ,           ///< SPI2   CLK Select ID

	SPI3CLK_FREQ,           ///< SPI3   CLK Select ID
	SPI4CLK_FREQ,           ///< SPI4   CLK Select ID
	SPI5CLK_FREQ,           ///< SPI5   CLK Select ID
	MICLK_FREQ,

	IDEOUTIFCLK_FREQ,       ///< IDE    output I/F CLK freq Select ID
	CPUCLK_FREQ,            ///< CPU1   CLK Select ID
	APBCLK_FREQ,            ///< APB    CLK Select ID
	TRNGCLK_FREQ,           ///< TRNG   CLK Select ID

	SIEMCLK_12SYNC_FREQ,    ///< SIE    MCLK-1/2 SYNC freq Select ID
//	DSICLK_FREQ,            ///< DSI    CLK Select ID

	CONVCLK_FREQ,
	LSUCLK_FREQ,
	PPUCLK_FREQ,

	NUE30CLK_FREQ,
	NUE2CLK_FREQ,


	IMECLK_FREQ,
	IVECLK_FREQ,            ///< IVE    CLK freq Select ID (V)
	IPECLK_FREQ,
	MDBCCLK_FREQ,           ///< MDBC   CLK freq select ID (V)

	IFECLK_FREQ,
	VPECLK_FREQ,
	ISECLK_FREQ,
	TRKECLK_FREQ,

	JPEGCLK_FREQ,
	DRECLK_FREQ,
	UVCPCLK_FREQ,
	GRPHCLK_FREQ,

	GRPH2CLK_FREQ,
	GRPH3CLK_FREQ,
	VENCCLK_FREQ,
	ECDSACLK_FREQ,

	CRYPTOCLK_FREQ,
	HASHCLK_FREQ,
	RSACLK_FREQ,
	HWCPYCLK_FREQ,

	ETH_PTP_CLK_FREQ,
	TSECLK_FREQ,

	CSICLK_FREQ,
	CSI2CLK_FREQ,
	CSI3CLK_FREQ,
	CSI4CLK_FREQ,


	PLL_CLKFREQ_MAXNUM,

	HDMIADOCLK_FREQ,        ///< Backward compatible
	SIEMCLK5_FREQ,          ///< Backward compatible
	DSPCLK_FREQ,            ///< Backward compatible
	H264D_ACLK_FREQ,        ///< Backward compatible
	H265D_ACLK_FREQ,        ///< Backward compatible
	H264M_ACLK_FREQ,        ///< Backward compatible
	H265M_ACLK_FREQ,        ///< Backward compatible
	JPEG2CLK_FREQ,      ///< Backward compatible
	JPEGLITECLK_FREQ,       ///< Backward compatible
	SDECLK_FREQ,            ///< Backward compatible
	CNNCLK_FREQ,            ///< Backward compatible
	CNN2CLK_FREQ,           ///< Backward compatible
	DCECLK_FREQ,            ///< Backward compatible
	VPELCLK_FREQ,           ///< Backward compatible
	IDE2OUTIFCLK_FREQ,      ///< Backward compatible
	IDE2CLK_FREQ,           ///< Backward compatible
	SIE6CLK_FREQ,           ///< Backward compatible
	VIE2CLK_FREQ,           ///< Backward compatible
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
#define PLL_CLKSEL_SIE_MCLK2SRC_PLL10  PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_SIE_MCLKSRC_PLL10   PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_SIE_MCLK3SRC_PLL10  PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_SIE_MCLKSRC_PLL18   PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_SIE_MCLK3SRC_PLL18  PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_TRNGRO_CLKSRC_PLL4  PLL_CLKSEL_MAX_ITEM + 5 //< Backward compatible
#define PLL_CLKSEL_MIPI_LVDS           PLL_CLKSEL_MAX_ITEM
#define PLL_CLKSEL_LVDS_CLKPHASE       PLL_CLKSEL_MAX_ITEM
#define PLL_CLKSEL_SIE_IO_PXCLKSRC_PXCLKPAD    PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_SIE_IO_PXCLKSRC_PXCLKPAD2   PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_SIE2_IO_PXCLKSRC_PXCLKPAD2  PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_SIE2_IO_PXCLKSRC_PXCLKPAD   PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_SIE_PXCLKSRC_PXCLKPAD    PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_SIE_PXCLKSRC_MCLK        PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_SIE2_PXCLKSRC_PXCLKPAD   PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_SIE2_PXCLKSRC_MCLK       PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_SIE4_IO_PXCLKSRC_PXCLKPAD   PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_SIE4_IO_PXCLKSRC_PXCLKPAD2  PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_TGE_PXCLKSRC_PXCLKPAD    PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_TGE_PXCLKSRC_MCLK        PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_TGE2_PXCLKSRC_PXCLKPAD    PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_TGE2_PXCLKSRC_MCLK2      PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_AFFINE_240      PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_AFFINE_320      PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_AFFINE_480      PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_MIPI_LVDS_60  PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_MIPI_LVDS_120 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_LVDS_CLK_PHASE_NORMAL   PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_LVDS_CLK_PHASE_INVERT   PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_LVDS2_CLK_PHASE_NORMAL   PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_LVDS2_CLK_PHASE_INVERT   PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_MIPI_LVDS2 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_MIPI_LVDS2_60  PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_MIPI_LVDS2_120 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_ADO_MCLKSEL_256FS   PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_ADO_MCLKSEL_PLL7    PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_ETH_PLL6   PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_ETH_PLL9   PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_ETH_REFCLK_INV_DIS   PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_ETH_REFCLK_INV_EN    PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_IPACLK_CLKSRC_240     PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_IPACLK_CLKSRC_120       PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D0_SRC_HSI0 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D0_SRC_HSI1 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D0_SRC_HSI2 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D0_SRC_HSI3 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D1_SRC_HSI0 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D1_SRC_HSI1 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D1_SRC_HSI2 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D1_SRC_HSI3 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D2_SRC_HSI0 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D2_SRC_HSI1 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D2_SRC_HSI2 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D2_SRC_HSI3 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D3_SRC_HSI0 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D3_SRC_HSI1 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D3_SRC_HSI2 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CSILPCLK_D3_SRC_HSI3 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_TRNG_RO_DELAY PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_TGE2 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_DCE_192 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_DCE_96 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_DCE_PLL12 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CNN_SRAM PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_IPE_192 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_IPE_96 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_IPE_PLL12 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_IME_192 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_IME_96 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_IME_PLL12 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_SIE_MCLKSRC_320 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_SIE_MCLK2SRC_320 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_SIE_MCLK3SRC_320 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CNN_240  PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CNN_320  PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CNN_PLL12 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CNN_PLL9 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CNN2_240 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_CNN2_320 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_IFE_192 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_IFE_396 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_IFE_PLL12 PLL_CLKSEL_MAX_ITEM //< Backward compatible
#define PLL_CLKSEL_IFE_96 PLL_CLKSEL_MAX_ITEM //< Backward compatible


#define PLL_CLKSEL_RSA_400 PLL_CLKSEL_MAX_ITEM + 20 //< Not Backward compatible
#define PLL_CLKSEL_RSA_350 PLL_CLKSEL_MAX_ITEM + 21 //< Not Backward compatible

#define PLL_CLKSEL_HASH_400 PLL_CLKSEL_MAX_ITEM + 20 //< Not Backward compatible
#define PLL_CLKSEL_HASH_350 PLL_CLKSEL_MAX_ITEM + 21 //< Not Backward compatible

#define PLL_CLKSEL_CRYPTO_400 PLL_CLKSEL_MAX_ITEM + 20 //< Not Backward compatible
#define PLL_CLKSEL_CRYPTO_350 PLL_CLKSEL_MAX_ITEM + 21 //< Not Backward compatible

#define PLL_CLKSEL_SIE_MCLK5SRC PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_TGE PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE_MCLKSRC_PLL11 PLL_CLKSEL_MAX_ITEM + 22 //< Not Backward compatible
#define PLL_CLKSEL_TGE_SIEMCLK PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_TGE_SIEMCLK2 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE_MCLK3SRC_PLL11 PLL_CLKSEL_MAX_ITEM + 26 //< Not Backward compatible
#define PLL_CLKSEL_TGE_SIEMCLK3 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE_MCLK4SRC_PLL11 PLL_CLKSEL_MAX_ITEM + 12 //< Not Backward compatible
#define PLL_CLKSEL_TGE_SIEMCLK4 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE_MCLK5SRC_480 PLL_CLKSEL_MAX_ITEM + 16 //< Not Backward compatible
#define PLL_CLKSEL_SIE_MCLK5SRC_PLL5 PLL_CLKSEL_MAX_ITEM + 27 //< Not Backward compatible
#define PLL_CLKSEL_TGE_SIEMCLK5 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE_MCLK2SRC_PLL11 PLL_CLKSEL_MAX_ITEM + 100 //< Not Backward compatible
#define PLL_CLKSEL_SIE_MCLK5SRC_PLL11 PLL_CLKSEL_MAX_ITEM + 10 //< Not Backward compatible
#define PLL_CLKSEL_CSI2_PXCLK PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_CSI4_PXCLK_SIE PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_CSI4_PXCLK_VIE2 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_CSI4_PXCLK PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_CSI2_PXCLK_SIE PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_CSI5_120 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_CSI5_60 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_CSI2_PXCLK_VIE PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_CSI5 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_CSI5_240 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_CSI5 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_TSE_PLL2 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_MI_240 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_DSI_LPSRC PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_IDE_CLKSRC_192 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_IDE_CLKSRC_PLL10 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_IDE2_CLKSRC PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_IDE2_CLKSRC_192 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_IDE2_CLKSRC_PLL10 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_GRAPHIC_PLL18 PLL_CLKSEL_MAX_ITEM + 11 //< Not Backward compatible
#define PLL_CLKSEL_GRAPHIC2_PLL18 PLL_CLKSEL_MAX_ITEM + 12 //< Not Backward compatible
#define PLL_CLKSEL_GRAPHIC3_PLL18 PLL_CLKSEL_MAX_ITEM + 13 //< Not Backward compatible
#define PLL_CLKSEL_ISE_PLL15 PLL_CLKSEL_MAX_ITEM+ 17 //< Not Backward compatible
#define PLL_CLKSEL_VPE_PLL18 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_DCE_240 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_IFE_480 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SDE_PLL21 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_IFE_PLL15 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_DCE_320 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SDE_480 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible


#define PLL_CLKSEL_SDE_320 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SDE_240 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SDE PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_DCE_480 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_DCE_PLL15 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_DCE PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_IME_480 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_IPE_480 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_IME_PLL15 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_IPE_PLL15 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_CNN_SRAM_DCE PLL_CLKSEL_MAX_ITEM //< Not Backward compatible

#define PLL_CLKSEL_VENC_PLL14 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_VENC_PLL14 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_VIE_CLKSRC_PLL19 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_VENC_PLL24 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_VIE2_CLKSRC PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE_CLKSRC_PLL19 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_VIE2_CLKSRC_PLL19 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE2_CLKSRC_PLL19 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible

#define PLL_CLKSEL_SIE3_CLKSRC_PLL19 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE4_CLKSRC_PLL19 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE5_CLKSRC_PLL19 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE6_CLKSRC PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_JPEG2 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE6_CLKSRC_PLL19 PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE3_PXCLKSRC PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_JPEG_LITE PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE3_PXCLKSRC_CCIR2_PXCLK PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE3_PXCLKSRC_MCLK PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE4_PXCLKSRC PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE4_PXCLKSRC_PXCLKPAD PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE4_PXCLKSRC_MCLK PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE5_PXCLKSRC PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE5_PXCLKSRC_CCIR2_PXCLK PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE5_PXCLKSRC_MCLK PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE6_PXCLKSRC PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE6_PXCLKSRC_CCIR2_PXCLK PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SIE6_PXCLKSRC_MCLK PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_SP_PLL11 PLL_CLKSEL_MAX_ITEM//< Not Backward compatible
#define PLL_CLKSEL_SP2_PLL11 PLL_CLKSEL_MAX_ITEM//< Not Backward compatible

#define PLL_CLKSEL_DSP_IOP_CLKDIV PLL_CLKSEL_MAX_ITEM//< Not Backward compatible
#define PLL_DSP_IOP_CLKDIV(x) (x)//< Not Backward compatible
#define PLL_CLKSEL_DSP_EDP_CLKDIV PLL_CLKSEL_MAX_ITEM//< Not Backward compatible
#define PLL_DSP_EDP_CLKDIV(x) (x)//< Not Backward compatible
#define PLL_CLKSEL_DSP_EPP_CLKDIV PLL_CLKSEL_MAX_ITEM//< Not Backward compatible
#define PLL_DSP_EPP_CLKDIV(x) (x)//< Not Backward compatible
#define PLL_CLKSEL_DSP_EDAP_CLKDIV PLL_CLKSEL_MAX_ITEM//< Not Backward compatible
#define PLL_DSP_EDAP_CLKDIV(x) (x)//< Not Backward compatible

#define DIS_CLK PLL_CLKSEL_MAX_ITEM//< Not Backward compatible
#define PLL_CLKSEL_DIS_480 PLL_CLKSEL_MAX_ITEM//< Not Backward compatible
#define PLL_CLKSEL_DIS_320 PLL_CLKSEL_MAX_ITEM//< Not Backward compatible
#define PLL_CLKSEL_DIS_240 PLL_CLKSEL_MAX_ITEM//< Not Backward compatible
#define PLL_CLKSEL_DIS PLL_CLKSEL_MAX_ITEM//< Not Backward compatible
#define PLL_CLKSEL_MDBC_192 PLL_CLKSEL_MAX_ITEM//< Not Backward compatible
#define PLL_CLKSEL_MDBC_96 PLL_CLKSEL_MAX_ITEM//< Not Backward compatible
#define PLL_CLKSEL_TRKE_480 PLL_CLKSEL_MAX_ITEM+1//< Not Backward compatible

#define PLL_CLKSEL_CNN_SRAM_CNN PLL_CLKSEL_MAX_ITEM+1//< Not Backward compatible
#define NUE_CLK 1//< Not Backward compatible
#define PLL_CLKSEL_NUE2_PLL8 PLL_CLKSEL_MAX_ITEM+1//< Not Backward compatible
#define PLL_CLKSEL_NUE2_PLL15 PLL_CLKSEL_MAX_ITEM+1//< Not Backward compatible
#define PLL_CLKSEL_NUE_480 PLL_CLKSEL_MAX_ITEM+1//< Not Backward compatible
#define PLL_CLKSEL_NUE_PLL15 PLL_CLKSEL_MAX_ITEM+//< Not Backward compatible
#define CNN_CLK 1//< Not Backward compatible
#define PLL_CLKSEL_NUE_320 PLL_CLKSEL_MAX_ITEM+1//< Not Backward compatible
#define CNN2_CLK 1//< Not Backward compatible
#define CNN_CLK 1//< Not Backward compatible
#define PLL_CLKSEL_NUE PLL_CLKSEL_MAX_ITEM+1//< Not Backward compatible
#define PLL_CLKSEL_NUE_PLL8 PLL_CLKSEL_MAX_ITEM+1//< Not Backward compatible
#define PLL_CLKSEL_CNN_PLL15 PLL_CLKSEL_MAX_ITEM+1//< Not Backward compatible
#define PLL_CLKSEL_CNN2_PLL15 PLL_CLKSEL_MAX_ITEM+1//< Not Backward compatible
#define PLL_CLKSEL_CNN PLL_CLKSEL_MAX_ITEM+1//< Not Backward compatible
#define PLL_CLKSEL_CNN2 PLL_CLKSEL_MAX_ITEM+1//< Not Backward compatible
#define PLL_CLKSEL_CNN_PLL8 PLL_CLKSEL_MAX_ITEM+1//< Not Backward compatible
#define PLL_CLKSEL_CNN2_PLL8 PLL_CLKSEL_MAX_ITEM+1//< Not Backward compatible

#define PLL_CLKSEL_SDIO_320 PLL_CLKSEL_MAX_ITEM + 2//< Not Backward compatible
#define PLL_CLKSEL_SDIO_PLL6 PLL_CLKSEL_MAX_ITEM + 3//< Not Backward compatible
#define PLL_CLKSEL_SDIO2_320 PLL_CLKSEL_MAX_ITEM + 2//< Not Backward compatible
#define PLL_CLKSEL_SDIO2_PLL6 PLL_CLKSEL_MAX_ITEM+ 3//< Not Backward compatible
#define PLL_CLKSEL_SDIO3_320 PLL_CLKSEL_MAX_ITEM+2//< Not Backward compatible
#define PLL_CLKSEL_SDIO3_PLL6 PLL_CLKSEL_MAX_ITEM+3//< Not Backward compatible

#define ADC_TSEN_CLK 0//< Not Backward compatible
#define PLL_CLKSEL_TRNGRO_CLKSRC_ROSC PLL_CLKSEL_MAX_ITEM//< Not Backward compatible
#define TRNG_RO_CLK 0 //< Not Backward compatible
#define UART7_CLK 0 //< Not Backward compatible
#define UART8_CLK 0 //< Not Backward compatible
#define UART9_CLK 0 //< Not Backward compatible
#define PLL_CLKSEL_TRNGRO_CLKSRC PLL_CLKSEL_MAX_ITEM //< Not Backward compatible
#define PLL_CLKSEL_JPEG_PLL9  PLL_CLKSEL_MAX_ITEM //< Not Backward compatible


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


