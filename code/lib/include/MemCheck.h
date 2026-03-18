/**
    Memory check

    Use check sum, CRC, or add pseudo string to ckeck memory data

    @file       MemCheck.h
    @ingroup    mIUtilMemCheck
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef _MEMCHECK_H
#define _MEMCHECK_H

/**
    @addtogroup mIUtilMemCheck
*/
//@{

typedef struct _MEMCHECK_PSEUDOSTR{
    uintptr_t uiAddr; ///< Mem Begin
    UINT32 uiLen;  ///< Data End
    UINT32 uiMemSize; ///< Mem Size, have to larger than uiLen+strlen(string)
    char*  pPseudoStr; ///< Pseudo string
}MEMCHECK_PSEUDOSTR,*PMEMCHECK_PSEUDOSTR;

/**
     Get CRC Code

     @param[in] uiAddr     starting address
     @param[in] uiLen      memory size
     @return CRC code
*/
extern UINT32 MemCheck_CalcCrcCode(uintptr_t uiAddr,UINT32 uiLen);

/**
     Get check sum code

     @param[in] uiAddr     starting address
     @param[in] uiLen      memory size
     @return sum code
*/
extern UINT32 MemCheck_CalcCheckSum16Bit(ULONG uiAddr,UINT32 uiLen);

/**
     add pseudo code into memory

     @param[in] pPseudo    refer to MEMCHECK_PSEUDOSTR
     @return total memory size of added pseudo string
*/
extern UINT32 MemCheck_AddPseudoStr(const MEMCHECK_PSEUDOSTR* pPseudo);

//@}

#endif
