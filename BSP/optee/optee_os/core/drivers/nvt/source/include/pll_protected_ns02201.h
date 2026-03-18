/*
    Protected header for PLL module

    PLL Configuration module protected header file.

    @file       pll_protected.h
    @ingroup    mIDrvSys_CG
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

#ifndef _PLL_PROTECTED_H
#define _PLL_PROTECTED_H

#include <kwrap/nvt_type.h>
#include <plat/pll.h>

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


/*
    Clock select ID

    Clock select ID for pll_set_clock_rate() & pll_get_clock_rate().
*/
typedef enum {
	PLL_CLKSEL_APB =            8,                          	//< Clock Select Module ID: APB

	PLL_CLKSEL_HASH = 			PLL_CLKSEL_R5_OFFSET + 8,  		//< Clock Select Module ID: HASH
	PLL_CLKSEL_RSA = 			PLL_CLKSEL_R5_OFFSET + 9,  		//< Clock Select Module ID: RSA
	PLL_CLKSEL_CRYPTO = 		PLL_CLKSEL_R5_OFFSET + 10, 		//< Clock Select Module ID: Crypto
	PLL_CLKSEL_TRNG =           PLL_CLKSEL_R5_OFFSET + 12, 		//< Clock Select Module ID: TRNG
	PLL_CLKSEL_TRNGRO_CLKSRC =  PLL_CLKSEL_R5_OFFSET + 13, 		//< Clock Select Module ID: TRNG RO CLK source

	// TRNG RO Clock bit definition
	//Peripheral Clock Divider bit definition 2
	PLL_CLKSEL_TRNG_RO_CLKDIV = 	PLL_CLKSEL_R16_OFFSET + 0,  //< Clock Select Module ID: TRNG RO clock divider
	PLL_CLKSEL_TRNG_RO_DELAY =  	PLL_CLKSEL_R16_OFFSET + 8,  //< Clock Select Module ID: TRNG RO Delay
	PLL_CLKSEL_RO32K_CLKDIV =   	PLL_CLKSEL_R16_OFFSET + 12, //< Clock Select Module ID: RO 32K clock divider
	PLL_CLKSEL_RO32K_DIV_TESTEN =   PLL_CLKSEL_R16_OFFSET + 22, //< Ring oscillator range: 53 ~142MHz
	PLL_CLKSEL_TRNG_CLKDIV =    	PLL_CLKSEL_R16_OFFSET + 24, //< Clock Select Module ID: TRNG clock divider

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
//@}


/*
    @name   HASH clock rate value

    HASH clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_HASH).
*/
//@{
#define PLL_CLKSEL_HASH_PLL13   (0x00 << (PLL_CLKSEL_HASH - PLL_CLKSEL_R5_OFFSET))    //< Select Crypto clock as PLL13
#define PLL_CLKSEL_HASH_FIX320  (0x01 << (PLL_CLKSEL_HASH - PLL_CLKSEL_R5_OFFSET))    //< Select Crypto clock as 320Mhz
//@}

/*
    @name   RSA clock rate value

    RSA clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_RSA).
*/
//@{
#define PLL_CLKSEL_RSA_PLL13   (0x00 << (PLL_CLKSEL_RSA - PLL_CLKSEL_R5_OFFSET))    //< Select Crypto clock as PLL13
#define PLL_CLKSEL_RSA_FIX320  (0x01 << (PLL_CLKSEL_RSA - PLL_CLKSEL_R5_OFFSET))    //< Select Crypto clock as 320Mhz
//@}

/*
    @name   Crypto clock rate value

    Crypto clock rate value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_CRYPTO).
*/
//@{
#define PLL_CLKSEL_CRYPTO_PLL13   (0x00 << (PLL_CLKSEL_CRYPTO - PLL_CLKSEL_R5_OFFSET))    //< Select Crypto clock as PLL13
#define PLL_CLKSEL_CRYPTO_FIX320  (0x01 << (PLL_CLKSEL_CRYPTO - PLL_CLKSEL_R5_OFFSET))    //< Select Crypto clock as 320Mhz
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
#define PLL_CLKSEL_TRNGRO_CLKSRC_ROSC   (0x00 << (PLL_CLKSEL_TRNGRO_CLKSRC - PLL_CLKSEL_R5_OFFSET)) //< Select TRNG ROSC clock from Ring OSC macro
#define PLL_CLKSEL_TRNGRO_CLKSRC_PLL2   (0x01 << (PLL_CLKSEL_TRNGRO_CLKSRC - PLL_CLKSEL_R5_OFFSET)) //< Select TRNG ROSC clock from PLL11
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
	HASH_RSTN 		= 29, 	        //< Reset HASH controller
	RSA_RSTN 		= 30, 	        //< Reset RSA controller
	CRYPTO_RSTN     = 31,   		//< Reset CRYPTO controller
	TMR_RSTN		= 64+12, 	    //< Reset TMR controller
    RTC_RSTN		= 64+15,  	    //< Reset RTC controller
	TRNG_RSTN   	= 64+29, 	    //< Reset TRNG controller
	WDT_RSTN		= 96+4,		    //< Reset WDT controller

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


