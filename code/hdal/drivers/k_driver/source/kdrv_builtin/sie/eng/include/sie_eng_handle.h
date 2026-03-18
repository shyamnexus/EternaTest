/*
    Public header file for SIE module.

    Copyright Novatek Microelectronics Corp. 2023.  All rights reserved.
*/

#ifndef _SIE_ENG_HANDLE_H_
#define _SIE_ENG_HANDLE_H_

#include "kwrap/type.h"

/*************************************************************************************
 *  SIE Engine Handle Definition for External
 *************************************************************************************/
typedef struct sie_eng_handle_t SIE_ENG_HANDLE;

/*************************************************************************************
 *  Public Function Prototype
 *************************************************************************************/
SIE_ENG_HANDLE *sie_eng_get_handle(UINT32 chip_id, UINT32 eng_id);

#endif  /* _SIE_ENG_HANDLE_H_ */
