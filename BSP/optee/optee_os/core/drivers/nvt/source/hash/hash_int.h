/*
    HASH internal header file

    Copyright Novatek Microelectronics Corp. 2021.  All rights reserved.
*/

#ifndef _HASH_INT_H
#define _HASH_INT_H

#include <plat/io_address.h>
#include <rcw_macro.h>

#define MKWORD(B)   ((UINT32) (((UINT32)((B)[3]))<<24 | ((UINT32)((B)[2]))<<16 | ((UINT32)((B)[1]))<< 8 | ((UINT32)((B)[0]))))
#define INVWORD(B)  ((UINT32) (((UINT32)((B)[0]))<<24 | ((UINT32)((B)[1]))<<16 | ((UINT32)((B)[2]))<< 8 | ((UINT32)((B)[3]))))

#endif  /* _HASH_INT_H */
