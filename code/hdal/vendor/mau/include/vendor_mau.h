/**
    Public header file for dal_ive

    This file is the header file that define the API and data type for vendor_ive.

    @file       vendor_ive.h
    @ingroup    mILibIPLCom
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2018.    All rights reserved.
*/
#ifndef _VENDOR_MAU_H_
#define _VENDOR_MAU_H_

#include "hd_type.h"


/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
//ch: ch: 0: CPU_NS, 1: JM, 2: JMISP, 3: CONV, 4: NUE, 5: LSU, 6:PPU 7:NUE2 8:CONV+JM+NUE+LSU+POU 9:POU
//rw: 0: write, 1: read, 2: both
//dram: 0: DRAM1, 1: DRAM2(no use)
extern INT32 vendor_mau_ch_mon_start(int ch, int rw, int dram);
extern UINT64 vendor_mau_ch_mon_stop(int ch, int dram);

//dram: 0: DRAM1, 1: DRAM2(no use)
extern INT32 vendor_dma_reset_data_count(int dram);
extern UINT64 vendor_dma_get_data_count(int dram);


#endif //_VENDOR_MAU_H_
