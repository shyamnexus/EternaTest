/*
    Crypto internal header file

    Copyright Novatek Microelectronics Corp. 2021.  All rights reserved.
*/

#ifndef _CRYPTO_INT_H
#define _CRYPTO_INT_H

#include <plat/io_address.h>
#include <rcw_macro.h>

#define MKWORD(B) ((UINT32) (((UINT32)((B)[3]))<<24 | ((UINT32)((B)[2]))<<16 | ((UINT32)((B)[1]))<< 8 | ((UINT32)((B)[0]))))

#endif
