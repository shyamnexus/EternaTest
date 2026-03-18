/*
    Protected header for PLL module

    PLL Configuration module protected header file.

    @file       pll_protected.h
    @ingroup    mIDrvSys_CG
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2024.  All rights reserved.
*/

#ifndef _PLL_PROTECTED_H
#define _PLL_PROTECTED_H

#include <kwrap/nvt_type.h>
#include <plat/pll.h>

#include <kwrap/nvt_type.h>
#include <plat/pll.h>
#include "nvt_pll_cfg.h"

#ifndef CG_REG_TO_BASE
#define CG_REG_TO_BASE(reg)  (((reg)/(4))*(32))
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

#define PLL_CLKSEL_R0_OFFSET        CG_REG_TO_BASE(0x10)
#define PLL_CLKSEL_R1_OFFSET        CG_REG_TO_BASE(0x14)
#define PLL_CLKSEL_R2_OFFSET        CG_REG_TO_BASE(0x18)
#define PLL_CLKSEL_R3_OFFSET        CG_REG_TO_BASE(0x1C)
#define PLL_CLKSEL_R4_OFFSET        CG_REG_TO_BASE(0x20)
#define PLL_CLKSEL_R5_OFFSET        CG_REG_TO_BASE(0x24)
#define PLL_CLKSEL_R6_OFFSET        CG_REG_TO_BASE(0x28)
#define PLL_CLKSEL_R7_OFFSET        CG_REG_TO_BASE(0x2C)
#define PLL_CLKSEL_R8_OFFSET        CG_REG_TO_BASE(0x30)
#define PLL_CLKSEL_R9_OFFSET        CG_REG_TO_BASE(0x34)
#define PLL_CLKSEL_R10_OFFSET       CG_REG_TO_BASE(0x38)
#define PLL_CLKSEL_R11_OFFSET       CG_REG_TO_BASE(0x3C)
#define PLL_CLKSEL_R12_OFFSET       CG_REG_TO_BASE(0x40)
#define PLL_CLKSEL_R13_OFFSET       CG_REG_TO_BASE(0x44)
#define PLL_CLKSEL_R14_OFFSET       CG_REG_TO_BASE(0x48)
#define PLL_CLKSEL_R15_OFFSET       CG_REG_TO_BASE(0x4C)
#define PLL_CLKSEL_R16_OFFSET       CG_REG_TO_BASE(0x50)
#define PLL_CLKSEL_R17_OFFSET       CG_REG_TO_BASE(0x54)
#define PLL_CLKSEL_R18_OFFSET       CG_REG_TO_BASE(0x58)
#define PLL_CLKSEL_R19_OFFSET       CG_REG_TO_BASE(0x5C)
#define PLL_CLKSEL_R20_OFFSET       CG_REG_TO_BASE(0x60)
#define PLL_CLKSEL_R21_OFFSET       CG_REG_TO_BASE(0x64)
#define PLL_CLKSEL_R22_OFFSET       CG_REG_TO_BASE(0x68)
#define PLL_CLKSEL_R23_OFFSET       CG_REG_TO_BASE(0x6C)
#define PLL_CLKSEL_R24_OFFSET       3968 //(0x200 - 0x10)/4*32
#define PLL_CLKSEL_R25_OFFSET       4000
#define PLL_CLKSEL_R26_OFFSET       4032
#define PLL_CLKSEL_R27_OFFSET       4064
#define PLL_CLKSEL_R28_OFFSET       6016// (0x300 - 0x10)/4*32
#define PLL_CLKSEL_R29_OFFSET       6048
#define PLL_CLKSEL_R30_OFFSET       6080
#define PLL_CLKSEL_R31_OFFSET       6112
#define PLL_CLKSEL_R32_OFFSET       6144
#define PLL_CLKSEL_R33_OFFSET       6176
#define PLL_CLKSEL_R34_OFFSET       6208
#define PLL_CLKSEL_R35_OFFSET       6240
#define PLL_CLKSEL_R36_OFFSET       6272
#define PLL_CLKSEL_R37_OFFSET       6304
#define PLL_CLKSEL_R38_OFFSET       4096


/*
    Clock select ID

    Clock select ID for pll_set_clock_rate() & pll_get_clock_rate().
*/
typedef enum {
	PLL_CLKSEL_APB =            8,                          	//< Clock Select Module ID: APB

    PLL_CLKSEL_ECDSA =          PLL_CLKSEL_R3_OFFSET + 6,       //< Clock Select Module ID: ECDSA
	PLL_CLKSEL_HASH = 			PLL_CLKSEL_R5_OFFSET + 16,      //< Clock Select Module ID: HASH
	PLL_CLKSEL_RSA = 			PLL_CLKSEL_R3_OFFSET + 22,  	//< Clock Select Module ID: RSA
	PLL_CLKSEL_CRYPTO = 		PLL_CLKSEL_R3_OFFSET + 20, 		//< Clock Select Module ID: Crypto
	PLL_CLKSEL_TRNG =           PLL_CLKSEL_R5_OFFSET + 18, 	    //< Clock Select Module ID: TRNG
	PLL_CLKSEL_TRNGRO_CLKSRC =  PLL_CLKSEL_R5_OFFSET + 13, 		//< Clock Select Module ID: TRNG RO CLK source

	// TRNG RO Clock bit definition
	//Peripheral Clock Divider bit definition 2
	PLL_CLKSEL_TRNG_RO_CLKDIV = 	PLL_CLKSEL_R20_OFFSET + 0,  //< Clock Select Module ID: TRNG RO clock divider
	PLL_CLKSEL_TRNG_RO_DELAY =  	PLL_CLKSEL_R16_OFFSET + 8,  //< Clock Select Module ID: TRNG RO Delay
	PLL_CLKSEL_RO32K_CLKDIV =   	PLL_CLKSEL_R16_OFFSET + 12, //< Clock Select Module ID: RO 32K clock divider
	PLL_CLKSEL_RO32K_DIV_TESTEN =   PLL_CLKSEL_R16_OFFSET + 22, //< Ring oscillator range: 53 ~142MHz
	PLL_CLKSEL_TRNG_CLKDIV =    	PLL_CLKSEL_R9_OFFSET + 24,  //< Clock Select Module ID: TRNG clock divider

	PLL_CLKSEL_MAX_ITEM,                                    	// magic number for pll checking usage.
	ENUM_DUMMY4WORD(PLL_CLKSEL)
} PLL_CLKSEL;

/*
    @name   APB clock rate value

    APB clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_APB).
*/
//@{
#define PLL_CLKSEL_APB_120        (0x00 << PLL_CLKSEL_APB)      //< Select APB  120MHz
#define PLL_CLKSEL_APB_150        (0x01 << PLL_CLKSEL_APB)      //< Select APB  150MHz
#define PLL_CLKSEL_APB_MASK             (0x03 << (PLL_CLKSEL_APB % 32))
//@}

/*
    @name   HASH clock rate value

    HASH clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_HASH).
*/
//@{
#define PLL_CLKSEL_HASH_PLL13           (0x00 << (PLL_CLKSEL_HASH % 32)) //< Select Crypto clock as PLL13
#define PLL_CLKSEL_HASH_320             (0x01 << (PLL_CLKSEL_HASH % 32)) //< Select Crypto clock as 320Mhz
#define PLL_CLKSEL_HASH_MASK            (0x01 << (PLL_CLKSEL_HASH % 32))

/*
    @name   RSA clock rate value

    RSA clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_RSA).
*/
//@{
#define PLL_CLKSEL_RSA_PLL13            (0x00 << (PLL_CLKSEL_RSA % 32)) //< Select Crypto clock as PLL13
#define PLL_CLKSEL_RSA_320              (0x01 << (PLL_CLKSEL_RSA % 32)) //< Select Crypto clock as 320Mhz
#define PLL_CLKSEL_RSA_MASK             (0x01 << (PLL_CLKSEL_RSA % 32))

/*
    @name   Crypto clock rate value

    Crypto clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_CRYPTO).
*/
//@{
#define PLL_CLKSEL_CRYPTO_PLL13         (0x00 << (PLL_CLKSEL_CRYPTO % 32)) //< Select Crypto clock as PLL13
#define PLL_CLKSEL_CRYPTO_320           (0x01 << (PLL_CLKSEL_CRYPTO % 32)) //< Select Crypto clock as 320Mhz
#define PLL_CLKSEL_CRYPTO_MASK          (0x01 << (PLL_CLKSEL_CRYPTO % 32))

/*
    @name   Crypto clock rate value

    Crypto clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_CRYPTO).
*/
//@{
#define PLL_CLKSEL_ECDSA_PLL13          (0x00 << (PLL_CLKSEL_ECDSA % 32)) //< Select Crypto clock as PLL13
#define PLL_CLKSEL_ECDSA_320            (0x01 << (PLL_CLKSEL_ECDSA % 32)) //< Select Crypto clock as 320Mhz
#define PLL_CLKSEL_ECDSA_MASK           (0x01 << (PLL_CLKSEL_RSA % 32))

/*
    @name   TRNG clock rate value

    TRNG clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_TRNG).
*/
//@{
#define PLL_CLKSEL_TRNG_160             (0x00 << (PLL_CLKSEL_TRNG % 32)) //< Select TRNG clock as 160Mhz
#define PLL_CLKSEL_TRNG_240             (0x01 << (PLL_CLKSEL_TRNG % 32)) //< Select TRNG clock as 240Mhz
#define PLL_CLKSEL_TRNG_MASK            (0x01 << (PLL_CLKSEL_TRNG % 32))
//@}

/*
    @name   TRNG RO clock source value

    TRNG RO clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_TRNGRO_CLKSRC).
*/
//@{
#define PLL_CLKSEL_TRNGRO_CLKSRC_ROSC   (0x00 << (PLL_CLKSEL_TRNGRO_CLKSRC - PLL_CLKSEL_R5_OFFSET)) //< Select TRNG ROSC clock from Ring OSC macro
#define PLL_CLKSEL_TRNGRO_CLKSRC_PLL2   (0x01 << (PLL_CLKSEL_TRNGRO_CLKSRC - PLL_CLKSEL_R5_OFFSET)) //< Select TRNG ROSC clock from PLL11
#define PLL_CLKSEL_TRNGRO_CLKSRC_MASK   (0x01 << (PLL_CLKSEL_TRNGRO_CLKSRC % 32))
//@}


/*
    @name   TRNG RO clock divider

    TRNG RO clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_TRNG_RO_CLKDIV).
*/
//@{
#define PLL_TRNG_RO_CLKDIV(x)          ((x) << (PLL_CLKSEL_TRNG_RO_CLKDIV - PLL_CLKSEL_R16_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_TRNG_RO_CLKDIV)
//@}

/*
    @name   TRNG RO delay

    TRNG RO delay

    @note This if for pll_set_clock_rate(PLL_CLKSEL_TRNG_RO_DELAY).
*/
//@{
#define PLL_TRNG_RO_DELAY(x)          ((x) << (PLL_CLKSEL_TRNG_RO_DELAY - PLL_CLKSEL_R16_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_TRNG_RO_DELAY)
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

    @note This if for pll_set_clock_rate(PLL_CLKSEL_TRNG_RO_DELAY).
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


/*
    System Reset ID

    This is for pll_enable_system_reset() and pll_disable_system_reset().
*/
typedef enum {
	HASH_RSTN 		= 32+26, 	    //< Reset HASH controller
	RSA_RSTN 		= 32+27, 	    //< Reset RSA controller
	CRYPTO_RSTN     = 32+25,   		//< Reset CRYPTO controller
    ECDSA_RSTN		= 32+28,		//< Reset ECDSA controller
	TMR_RSTN		= 64+12, 	    //< Reset TMR controller
    RTC_RSTN		= 32+23,  	    //< Reset RTC controller
	TRNG_RSTN   	= 32+29, 	    //< Reset TRNG controller
	WDT_RSTN		= 2,		    //< Reset WDT controller

	ENUM_DUMMY4WORD(CG_RSTN)
} CG_RSTN;



/**
    Clock frequency select ID

    @note This is for pll_set_clock_freq().
*/
typedef enum {
	APBCLK_FREQ,			///< APB    CLK Select ID
	TRNGCLK_FREQ,           ///< TRNG   CLK Select ID
	PLL_CLKFREQ_MAXNUM,

	ENUM_DUMMY4WORD(PLL_CLKFREQ)
} PLL_CLKFREQ;



//
//	Exporting APIs
//

extern void     pll_set_clock_rate(PLL_CLKSEL clk_sel, uint32_t uiValue);
extern uint32_t pll_get_clock_rate(PLL_CLKSEL clk_sel);

extern void     pll_disable_clock(CG_EN Num);
extern ER       pll_set_clock_freq(PLL_CLKFREQ ClkID, uint32_t uiFreq);
extern ER       pll_get_clock_freq(PLL_CLKFREQ ClkID, uint32_t *pFreq);
extern void     pll_enable_system_reset(CG_RSTN Num);
extern void     pll_disable_system_reset(CG_RSTN Num);

extern uint32_t   pll_get_osc_freq(void);

//@}

#endif


