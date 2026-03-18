/*
    @file       display_int.h
    @ingroup

    @brief      Internal Header file for Display control object
				This file is the header file that define the API and data type
				for Display control object

    @note       Nothing.


    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.

*/

#ifndef _DISPLAY_PORT_H
#define _DISPLAY_PORT_H

#include <asm/nvt-common/rcw_macro_bit.h>


//
//0x00 PLL power enable Register
//
#define PLL_PLL_PWREN_REG_OFS           0x00
REGDEF_BEGIN(PLL_PLL_PWREN_REG)
REGDEF_BIT(DONTCARE, 32)
REGDEF_END(PLL_PLL_PWREN_REG)

//
//0x04 PLL Status Register
//
#define PLL_PLL_STATUS_REG_OFS          0x04
REGDEF_BEGIN(PLL_PLL_STATUS_REG)
REGDEF_BIT(DONTCARE, 32)
REGDEF_END(PLL_PLL_STATUS_REG)



#endif
