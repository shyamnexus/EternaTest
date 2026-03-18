/***************************************************************************
* Copyright  Novatek Microelectronics Corp. 2022.  All rights reserved.    *
*--------------------------------------------------------------------------*
* Name: IVE Library Module                                                  *
* Description:                                                             *
* Author: Ming Yang                                                         *
****************************************************************************/

/**
    IVE lib

    Sample module detailed description.

    @file       nvt_ive_ccl_cpuproc.h
    @ingroup    mhdal
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

#ifndef _NVT_IVE_CCL_CPUPROC_H_
#define _NVT_IVE_CCL_CPUPROC_H_

#include "hd_type.h"

/********************************************************************
MACRO CONSTANT DEFINITIONS
********************************************************************/

/********************************************************************
TYPE DEFINITION - ENUM
********************************************************************/

/********************************************************************
TYPE DEFINITION - Structure
********************************************************************/

/********************************************************************
EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
extern INT32 NVT_IVE_CCL_CPUPROC(UINT32 blk_w, UINT32 blk_h, UINT32 blk_h_num, UINT32 blk_v_num, UINT8* output_map, UINT16* r_table_global, UINT16* temp_label_num, 
				UINT32 is_four_connect, UINT32 *valid_rect_num, UINT32 *area_threshold);

#endif  /* _NVT_IVE_CCL_CPUPROC_H_ */
